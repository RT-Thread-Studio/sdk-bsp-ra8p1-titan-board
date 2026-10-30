"""Generate MicroPython inputs and package firmware for Studio's GNU Make."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
sys.dont_write_bytecode = True
os.environ['PYTHONDONTWRITEBYTECODE'] = '1'


def prepare():
    from tools.host_tools import configure_host_tools, resolve_toolchain
    from tools.micropython_gen import generate
    from tools.native_project import write_build_tools, write_changed
    settings = json.loads((ROOT / 'build/make/settings.json').read_text())
    environment = os.environ.copy()
    environment['PATH'] = environment.get('PATH', '') + os.pathsep + str(Path(settings['compiler']).parent)
    compiler_dir = resolve_toolchain(environment, str(ROOT))
    settings['compiler'] = str(Path(compiler_dir) / 'arm-none-eabi-gcc')
    os.environ['PATH'] = compiler_dir + os.pathsep + os.environ.get('PATH', '')
    write_build_tools(ROOT, settings['compiler'])
    write_changed(ROOT / 'build/make/settings.json', json.dumps(settings, indent=2) + '\n')
    print('OpenMV ARM GCC: ' + compiler_dir)
    configure_host_tools(os.environ, str(ROOT))
    os.chdir(ROOT)
    generate(ROOT, settings)
    selected = [name for name in settings.get('native_sources', settings['sources']) if any(part in '/' + name for part in
                ('/openmv/lib/imlib/', '/openmv/lib/cmsis/src/dsp/', '/apriltag/'))]
    if not selected:
        raise RuntimeError('No vision sources in the generated Studio project')
    content = '/* Generated for the Studio managed Make object paths. */\n'
    content += ''.join('        *' + Path(name).with_suffix('.o').as_posix() +
                       '(.text* .rodata*)\n' for name in selected)
    output = ROOT / 'build/generated/vision_sections.ld'
    if not output.exists() or output.read_text() != content:
        output.write_text(content)


def package(build_dir):
    from tools.firmware import ELF, vision_image
    settings = json.loads((ROOT / 'build/make/settings.json').read_text())
    compiler_dir = Path(settings['compiler']).parent
    source = build_dir / 'rtthread.elf'
    patched, vision = vision_image(ELF(source))
    source.write_bytes(patched)
    output = ROOT / 'firmware'
    output.mkdir(exist_ok=True)
    (output / 'openmv-vision.bin').write_bytes(vision)
    subprocess.run([str(compiler_dir / 'arm-none-eabi-objcopy'), '-R', '.openmv_vision',
                    '-O', 'ihex', str(source), str(build_dir / 'rtthread.hex')], check=True)
    shutil.copyfile(build_dir / 'rtthread.hex', output / 'rtthread.hex')
    print('OpenMV firmware: ' + str(output))


def clean(build_dir):
    # Only the current project's direct Debug/Release directory is eligible.
    if build_dir.parent != ROOT or build_dir.name not in ('Debug', 'Release'):
        raise ValueError('Refusing to clean outside this project Debug/Release directory')
    for path in build_dir.rglob('*'):
        if path.is_file() and path.suffix in ('.o', '.d', '.elf', '.hex', '.bin', '.map', '.siz'):
            path.unlink()
    for name in ('rtthread.hex', 'openmv-vision.bin'):
        (ROOT / 'firmware' / name).unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=('prepare', 'package', 'clean'))
    parser.add_argument('--build-dir', default='.')
    args = parser.parse_args()
    build_dir = Path(args.build_dir).resolve()
    if args.action == 'prepare':
        prepare()
    elif args.action == 'package':
        package(build_dir)
    else:
        clean(build_dir)


if __name__ == '__main__':
    main()
