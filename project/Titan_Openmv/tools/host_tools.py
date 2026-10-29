"""Resolve installed host and ARM compilers; never launch a build."""
import os
import subprocess


def studio_directories(environment, project_dir=None):
    origins = [project_dir or os.path.dirname(os.path.dirname(os.path.abspath(__file__)))]
    if environment.get('ENV_ROOT'):
        origins.append(environment['ENV_ROOT'])
    seen = set()
    for origin in origins:
        directory = os.path.abspath(origin)
        while directory not in seen:
            seen.add(directory)
            yield directory
            parent = os.path.dirname(directory)
            if parent == directory:
                break
            directory = parent


def registered_windows_paths():
    """Read installed tool paths even when Eclipse replaces the process PATH."""
    try:
        import winreg
    except ImportError:  # The launcher also supports Studio's Python 2.7.
        import _winreg as winreg
    for hive, key in (
            (winreg.HKEY_LOCAL_MACHINE,
             r'SYSTEM\CurrentControlSet\Control\Session Manager\Environment'),
            (winreg.HKEY_CURRENT_USER, r'Environment')):
        try:
            with winreg.OpenKey(hive, key) as handle:
                value, _ = winreg.QueryValueEx(handle, 'Path')
        except OSError:
            continue
        for directory in value.split(os.pathsep):
            directory = os.path.expandvars(directory.strip().strip('"'))
            if directory:
                yield directory


def msys2_host_paths(environment):
    """Find native GCC and its POSIX tools when Studio hides MSYS2 from PATH."""
    roots = []
    if environment.get('MSYS2_ROOT'):
        roots.append(environment['MSYS2_ROOT'])
    # An active MSYS2 terminal may identify a non-default installation.
    if environment.get('MSYSTEM_PREFIX'):
        roots.append(os.path.dirname(environment['MSYSTEM_PREFIX'].rstrip('/\\')))
    drive = environment.get('SYSTEMDRIVE', environment.get('SystemDrive', 'C:'))
    roots.extend(os.path.join(drive + os.sep, name) for name in ('msys64', 'msys32'))
    for root in roots:
        root = os.path.abspath(root.strip('"'))
        native_bins = [os.path.join(root, prefix, 'bin')
                       for prefix in ('ucrt64', 'mingw64', 'mingw32')]
        native_bins = [directory for directory in native_bins
                       if os.path.isfile(os.path.join(directory, 'gcc.exe'))]
        if native_bins:
            for directory in native_bins:
                yield directory
            # MSYS2 installs sh/cat/sed separately from the native compiler.
            yield os.path.join(root, 'usr', 'bin')


def configure_host_tools(environment, project_dir=None):
    """Expose an installed portable Windows GCC/make/sh to mpy-cross."""
    if os.name != 'nt':
        return
    explicit = environment.get('OPENMV_HOST_TOOLS')
    candidates = [explicit] if explicit else [
        os.path.join(directory, 'platform', 'openmv-host-tools', 'w64devkit', 'bin')
        for directory in studio_directories(environment, project_dir)]
    for candidate in candidates:
        directory = os.path.abspath(candidate.strip('"'))
        if all(os.path.isfile(os.path.join(directory, name))
               for name in ('gcc.exe', 'make.exe', 'sh.exe')):
            environment['PATH'] = directory + os.pathsep + environment.get('PATH', '')
            environment.setdefault('MPY_HOST_CC', os.path.join(directory, 'gcc.exe'))
            print('OpenMV host tools: ' + directory)
            return
    if explicit:
        raise RuntimeError('OPENMV_HOST_TOOLS must contain gcc.exe, make.exe and sh.exe')

    # Studio may replace PATH with its ARM/build tools, hiding installed MinGW.
    # MSYS2 normally does not register its compiler directories in Windows PATH.
    # Append only existing tool directories, preserving caller/registry priority.
    directories = environment.get('PATH', '').split(os.pathsep)
    seen = set(os.path.normcase(os.path.abspath(item.strip('"')))
               for item in directories if item)
    added = []
    candidates = list(registered_windows_paths()) + list(msys2_host_paths(environment))
    for candidate in candidates:
        directory = os.path.abspath(candidate)
        key = os.path.normcase(directory)
        if key in seen:
            continue
        seen.add(key)
        if any(os.path.isfile(os.path.join(directory, name))
               for name in ('gcc.exe', 'mingw32-make.exe', 'make.exe', 'sh.exe')):
            added.append(directory)
    if added:
        environment['PATH'] = os.pathsep.join(directories + added)
        print('OpenMV host PATH: ' + os.pathsep.join(added))


def find_posix_shell(environment):
    """mpy-cross also needs cat/sed; Studio's minimal sh is insufficient."""
    directories = environment.get('PATH', '').split(os.pathsep)
    candidates = list(directories)
    for directory in directories + list(registered_windows_paths()):
        if os.path.isfile(os.path.join(directory, 'git.exe')):
            candidates.append(os.path.join(directory, '..', 'usr', 'bin'))
    candidates.append(os.path.join(environment.get('ProgramFiles', 'C:/Program Files'),
                                   'Git', 'usr', 'bin'))
    if environment.get('ENV_ROOT'):
        import glob
        candidates += glob.glob(os.path.join(environment['ENV_ROOT'], 'tools', 'git*', 'usr', 'bin'))
    for candidate in candidates:
        directory = os.path.abspath(candidate.strip('"'))
        if all(os.path.isfile(os.path.join(directory, name))
               for name in ('sh.exe', 'cat.exe', 'sed.exe')):
            return os.path.join(directory, 'sh.exe')
    raise RuntimeError('mpy-cross requires sh, cat and sed. Install Git for Windows '
                       'or supply the complete portable host tool directory.')


def native_tool_path(directory):
    """Keep GCC's relocated multilib paths below the Windows MAX_PATH limit."""
    directory = os.path.abspath(directory)
    if os.name == 'nt':
        import ctypes
        buffer = ctypes.create_unicode_buffer(32768)
        length = ctypes.windll.kernel32.GetShortPathNameW(directory, buffer, len(buffer))
        if 0 < length < len(buffer):
            directory = buffer.value
    return directory


def resolve_toolchain(environment, project_dir=None):
    """Select the required GCC 13.3, independently of Studio's legacy PATH."""
    explicit = environment.get('RTT_EXEC_PATH')
    candidates = [explicit] if explicit else []
    # Studio's project generator exports its default GCC 5.4.1 as RTT_EXEC_PATH.
    # Prefer a compatible explicit path, but keep searching when it is unsuitable.
    for directory in studio_directories(environment, project_dir):
        candidates.append(os.path.join(directory, 'repo', 'Extract',
            'ToolChain_Support_Packages', 'ARM',
            'GNU_Tools_for_ARM_Embedded_Processors', '13.3', 'bin'))
    candidates.extend(environment.get('PATH', '').split(os.pathsep))

    seen = set()
    rejected = []
    compiler = 'arm-none-eabi-gcc.exe' if os.name == 'nt' else 'arm-none-eabi-gcc'
    for candidate in candidates:
        directory = native_tool_path(candidate.strip('"'))
        if directory in seen:
            continue
        seen.add(directory)
        executable = os.path.join(directory, compiler)
        if not os.path.isfile(executable):
            continue
        try:
            version = subprocess.check_output([executable, '-dumpversion'],
                stderr=subprocess.STDOUT, env=environment).decode('ascii').strip()
        except (OSError, subprocess.CalledProcessError, UnicodeError):
            rejected.append(executable + ' (version check failed)')
            continue
        if version.split('.')[:2] == ['13', '3']:
            return directory
        rejected.append(executable + ' (' + version + ')')
    detail = ' Found: ' + '; '.join(rejected) if rejected else ''
    raise RuntimeError('OpenMV requires ARM GCC 13.3.Rel1. Set RTT_EXEC_PATH '
                       'to its bin directory.' + detail)
