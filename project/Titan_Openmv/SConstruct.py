"""Build the Titan Board OpenMV release with a single scons command."""
import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"
import rtconfig
from tools.host_tools import configure_host_tools

ROOT = Path(Dir('#').abspath)
configure_host_tools(os.environ, str(ROOT))
# Studio copies shared SDK sources into the imported project directory.
SDK_ROOT = ROOT if (ROOT / 'rt-thread').is_dir() else ROOT.parents[1]
RTT_ROOT = str(SDK_ROOT / 'rt-thread')
SDK_LIB = str(SDK_ROOT / 'libraries')
FSP_ROOT = str(ROOT / 'ra')
# Keep Kconfig usable in both the SDK checkout and an exported Studio project.
os.environ['RTT_ROOT'] = RTT_ROOT
os.environ['TITAN_OPENMV_SDK_LIB_ROOT'] = SDK_LIB
# Prefer RT-Thread's helpers over same-named modules in site-packages.
# Kconfiglib also installs menuconfig.py, but it has no guiconfig_silent API
# and imports curses even when Studio only needs to regenerate rtconfig.h.
sys.path.insert(0, str(Path(RTT_ROOT) / 'tools'))
sys.path.append(str(ROOT / 'ThirdParty'))
import building

AddOption('--prepare-only', dest='prepare_only', action='store_true', default=False,
          help='Generate MicroPython build metadata without compiling firmware')


def backend_selected():
    source = (ROOT / 'rtconfig.h').read_text(encoding='utf-8-sig')
    source = re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)
    names = set(re.findall(r'(?m)^\s*#\s*define\s+(\w+)\b', source))
    selected = [name for name in ('sd', 'flash')
                if 'BSP_OPENMV_STORAGE_' + name.upper() in names]
    if len(selected) != 1:
        raise ValueError('Select exactly one OpenMV storage medium: SD or SPI Flash')
    return selected[0]

SOURCE_OVERRIDES = {
    "/HAL_Drivers/drv_common.c": "platform/port/drv_common_openmv.c",
    "/openmv/modules/py_display.c": "platform/port/titan_py_display.c",
    "/openmv/protocol/omv_protocol_channel_stdio.c": "platform/port/omv_protocol_channel_stdio_titan.c",
    "/micropython/extmod/moddeflate.c": "platform/port/titan_moddeflate.c",
    "/libcpu/arm/cortex-m85/context_gcc.S": "platform/os/context_gcc.S",
    "/fsp/src/r_mipi_csi/r_mipi_csi.c": "platform/port/r_mipi_csi.c",
    "/dfs_v1/src/dfs_file.c": "platform/port/dfs_file_titan.c",
    "/dfs_v1/src/dfs_fs.c": "platform/port/dfs_fs_titan.c",
    "/dfs_v1/src/dfs_posix.c": "platform/port/dfs_posix_titan.c",
    "/dfs_v1/filesystems/elmfat/dfs_elm.c": "platform/port/dfs_elm_titan.c",
    "/ra_gen/common_data.c": "platform/port/common_data_openmv.c",
    "/ra_gen/vector_data.c": "platform/port/vector_data_openmv.c",
}
SOURCE_EXCLUDES = ("/HAL_Drivers/drv_sdram.c", "/HAL_Drivers/drv_i2c.c", "/HAL_Drivers/drv_sdhi.c")
SOURCE_EXTENSIONS = (".c", ".cc", ".cpp", ".s", ".S")


def create_environment(config):
    DefaultEnvironment(tools=[])
    compiler_tools = ["mingw"] if os.name == "nt" else ["gcc", "g++", "gnulink", "ar", "gas"]
    env = Environment(tools=compiler_tools, ENV=os.environ.copy(),
        AS=config.AS, ASFLAGS=config.AFLAGS,
        CC=config.CC, CFLAGS=config.CFLAGS,
        CXX=config.CXX, CXXFLAGS=config.CXXFLAGS,
        AR=config.AR, ARFLAGS="-rc", LINK=config.LINK, LINKFLAGS=config.LFLAGS)
    env.PrependENVPath("PATH", config.EXEC_PATH)
    install_gcc_spawn(env)
    return env


def install_gcc_spawn(env):
    # RT-Thread replaces SPAWN during PrepareBuilding; restore only this hook
    # afterwards, retaining its includes/defines, LINK and project state.
    if os.name == "nt":
        env["SPAWN"] = _gcc_spawn


def _gcc_spawn(sh, escape, cmd, args, environment):
    os.makedirs('build/response', exist_ok=True)
    response = None
    cmd = shutil.which(cmd, path=environment.get('PATH')) or cmd
    try:
        # SCons arguments already carry command-line quotes. Let GCC parse
        # a response file for both short and long commands; list2cmdline would
        # otherwise preserve those quotes inside short source-file arguments.
        fd, response = tempfile.mkstemp(suffix='.rsp', dir='build/response')
        with os.fdopen(fd, 'w', encoding='utf-8') as out:
            response_args = []
            define_value = False
            for argument in args[1:]:
                argument = str(argument)
                # SCons adds outer quotes to arguments containing spaces.
                token = argument[1:-1] if argument.startswith('"') and argument.endswith('"') else argument
                if define_value:
                    response_args.append(argument)
                    define_value = False
                elif token.startswith('-D'):
                    # Preserve C/string escaping, including a separate -D value.
                    response_args.append(argument)
                    define_value = token == '-D'
                else:
                    response_args.append(argument.replace(chr(92), '/'))
            out.write(' '.join(response_args))
        command = [cmd, '@' + response]
        result = subprocess.run(command, env=environment, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)
        sys.stdout.buffer.write(result.stdout)
        return result.returncode
    finally:
        if response:
            os.remove(response)


def isolate_sources(nodes, projects):
    """Route platform replacements into project-owned object directories."""
    directories = {}
    overrides = dict(SOURCE_OVERRIDES)
    if backend_selected() == "sd":
        overrides["/sdio/mmcsd_core.c"] = "platform/port/titan_mmcsd_core.c"

    def isolate(items):
        result = []
        for node in items:
            if isinstance(node, list):
                result.extend(isolate(node))
                continue
            source = node.srcnode().abspath
            normalized = source.replace("\\", "/")
            if normalized.endswith(SOURCE_EXCLUDES):
                continue
            for suffix, replacement in overrides.items():
                if normalized.endswith(suffix):
                    source = str(ROOT / replacement)
                    break
            if os.path.splitext(source)[1] not in SOURCE_EXTENSIONS:
                result.append(node)
                continue
            directory = os.path.dirname(source)
            if directory not in directories:
                variant = "build/objects/" + hashlib.sha256(directory.encode()).hexdigest()[:12]
                VariantDir(variant, directory, duplicate=0)
                directories[directory] = variant
            result.append(File(os.path.join(directories[directory], os.path.basename(source))))
        return result

    result = isolate(nodes)
    # RT-Thread rebuilds groups with LOCAL flags from Projects, so both lists
    # must use the same routing and VariantDir registry.
    for project in projects:
        project["src"] = isolate(project["src"])
    return result


def generate_vision_linker(projects):
    """Select only image algorithms, DSP and dictionaries for late loading.

    Object paths use the same directory hash as isolate_sources. Boot, USB,
    filesystem and the loader remain executable without the vision image.
    """
    selected = set()
    for project in projects:
        for node in project['src']:
            source = Path(node.srcnode().abspath)
            normalized = source.as_posix()
            if any(part in normalized for part in (
                    '/openmv/lib/imlib/', '/openmv/lib/cmsis/src/dsp/', '/apriltag/')):
                directory = hashlib.sha256(str(source.parent).encode()).hexdigest()[:12]
                selected.add(f'        *{directory}/{source.stem}.o(.text* .rodata*)')
    if not selected:
        raise RuntimeError('No vision objects selected for HyperRAM')
    path = ROOT / 'build/generated/vision_sections.ld'
    content = '/* Generated from the actual SCons source graph. */\n' + '\n'.join(sorted(selected)) + '\n'
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_text() != content:
        path.write_text(content)


env = create_environment(rtconfig)
Export(RTT_ROOT=RTT_ROOT, SDK_LIB=SDK_LIB, FSP_ROOT=FSP_ROOT, rtconfig=rtconfig)
rtconfig.BSP_LIBRARY_TYPE = None
config_paths = [str(ROOT / 'platform/port/machine_cfg')]
if backend_selected() == 'flash':
    config_paths.insert(0, str(ROOT / 'platform/port/flash_cfg'))
env.PrependUnique(CPPPATH=config_paths)

# RT-Thread maps the BSP to build/ by default. Keep that source mapping away
# from build/generated/, whose files are actual compiler inputs, not aliases.
sdk_sconscript = building.SConscript
def bsp_sconscript(*args, **kwargs):
    # Restore before dispatch so later SDK calls resolve named exports in
    # their original caller's scope rather than through this wrapper.
    building.SConscript = sdk_sconscript
    if kwargs.get('variant_dir') == 'build':
        kwargs['variant_dir'] = 'build/bsp'
    return sdk_sconscript(*args, **kwargs)

building.SConscript = bsp_sconscript
try:
    objs = building.PrepareBuilding(env, RTT_ROOT, has_libcpu=False)
finally:
    building.SConscript = sdk_sconscript
objs += SConscript(os.path.join(SDK_LIB, 'HAL_Drivers/SConscript'))
env.PrependUnique(CPPPATH=config_paths)
install_gcc_spawn(env)
objs = isolate_sources(objs, building.Projects)
projects = building.Projects

if GetOption('target') == 'eclipse':
    from tools.native_project import export_project
    env.AppendUnique(LIBS=['stdc++', 'm', 'c'])
    def export_native(program=None):
        # RT-Thread defaults this option to "project", even when Studio only
        # passes --target=eclipse. Preserve the imported project's actual name.
        named = any(argument == '--project-name' or argument.startswith('--project-name=')
                    for argument in sys.argv[1:])
        export_project(ROOT, env, projects, rtconfig,
                       GetOption('reset-project-config'), GetOption('project-name') if named else None)
    building.GenTargetProject = export_native
    building.DoBuilding('Debug/rtthread.elf', objs)
    Exit(0)

# Updating Studio's index does not need the generated VM sources or host GCC.
if not GetOption('clean') and GetOption('target') != 'eclipse':
    generate_vision_linker(projects)
    SConscript('platform/micropython/SConscript', exports={'env': env, 'projects': projects, 'rtconfig': rtconfig})
if GetOption('prepare_only'):
    Exit(0)

env.AppendUnique(LIBS=['stdc++', 'm', 'c'])
link_target = 'build/firmware/rtthread-unpacked.elf'
env.Depends(link_target, ['board/linker_scripts/fsp.ld', 'build/generated/vision_sections.ld'])
env.Clean(link_target, [Dir('build/generated'), 'build/firmware/rtthread.map'])
building.DoBuilding(link_target, objs)
package = SConscript('board/firmware/SConscript', exports={'env': env, 'rtconfig': rtconfig, 'link_target': link_target})
Default(package)
