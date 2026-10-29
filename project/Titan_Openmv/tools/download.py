"""Download Titan OpenMV with J-Link, then restart through the reset pin."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def studio_jlink():
    """Find the BSP's J-Link alongside Studio's configured ARM toolchain."""
    settings = ROOT / 'build/make/settings.json'
    if not settings.is_file():
        return None
    compiler = json.loads(settings.read_text(encoding='utf-8')).get('compiler')
    if not compiler:
        return None
    # Native builds may store Windows 8.3 directory names in settings.json.
    for parent in Path(compiler).parent.resolve().parents:
        if parent.name == 'ToolChain_Support_Packages':
            # v9.84 loses AP access on a second RA8P1 reset in our setup.
            # Keep the v8.48 version specified by this BSP.
            executable = parent.parent / 'Debugger_Support_Packages/SEGGER/J-Link/v8.48/JLink.exe'
            if executable.is_file():
                return str(executable)
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--jlink', default=os.environ.get('JLINK_EXE'),
                        help='JLink.exe path (or set JLINK_EXE / PATH)')
    parser.add_argument('--serial', help='J-Link probe serial number')
    parser.add_argument('--hex', type=Path, default=ROOT / 'firmware/rtthread.hex')
    parser.add_argument('--speed', type=int, default=1000, help='SWD clock in kHz')
    args = parser.parse_args()
    executable = args.jlink or studio_jlink() or shutil.which('JLink.exe') or shutil.which('JLinkExe')
    if not executable:
        parser.error('J-Link Commander not found; pass --jlink or set JLINK_EXE')
    firmware = args.hex.resolve()
    if not firmware.is_file() or firmware.suffix.lower() != '.hex':
        parser.error('A packaged rtthread.hex file is required: ' + str(firmware))
    if args.speed <= 0:
        parser.error('--speed must be positive')
    if any(c in str(firmware) for c in '\r\n"'):
        parser.error('Unsupported character in firmware path')

    # A normal LoadFile also invokes ResetTarget implicitly. On this board,
    # that SYSRESETREQ path can lose AP access before programming starts.
    # Halt explicitly, skip that reset, and boot via the physical reset pin.
    commands = ('h\nloadfile "' + firmware.as_posix() + '" 0 noreset\n'
                'r0\nsleep 50\nr1\nsleep 300\nq\n')
    command = [executable, '-Device', 'R7KA8P1KF_CPU0', '-If', 'SWD',
               '-Speed', str(args.speed), '-AutoConnect', '1', '-NoGui', '1',
               '-ExitOnError', '1']
    if args.serial:
        command += ['-SelectEmuBySN', args.serial]
    log = ROOT / 'build/jlink-download.log'
    log.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='titan-jlink-') as directory:
        script = Path(directory) / 'download.jlink'
        script.write_text(commands, encoding='utf-8')
        command += ['-CommanderScript', str(script)]
        print('J-Link: ' + str(executable), flush=True)
        print('Downloading ' + str(firmware), flush=True)
        with log.open('w', encoding='utf-8') as output:
            result = subprocess.run(command, stdout=output, stderr=subprocess.STDOUT)
    output = log.read_text(encoding='utf-8', errors='replace')
    print(output)
    download_output = output.partition('J-Link>loadfile ')[2]
    if result.returncode or 'O.K.' not in download_output or 'Error:' in output:
        raise SystemExit('Download failed; see ' + str(log))
    print('Download verified; reset pin released. Log: ' + str(log))


if __name__ == '__main__':
    main()
