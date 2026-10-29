"""Generate MicroPython metadata and frozen modules for SCons and Make."""
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


def generate(root, settings):
    ROOT = Path(root).resolve()
    UPSTREAM = ROOT / 'ThirdParty'
    MICROPYTHON = UPSTREAM / 'micropython'
    LOCK_FILE = UPSTREAM / 'components.lock.json'
    SCRIPT = Path(__file__).resolve()
    GENERATED = ROOT / 'build/generated'
    MODULES = ROOT / 'platform/micropython/modules'
    ML_PACKAGES = ('ml-core', 'ml-darknet', 'ml-edgeimpulse', 'ml-mediapipe', 'ml-ultralytics')
    ML_OVERRIDES = {'ml/model.py', 'ml/utils.py', 'ml/postprocessing/mediapipe.py'}



    def write_if_changed(path, data):
        path = Path(path)
        data = data.encode('utf-8') if isinstance(data, str) else data
        path.parent.mkdir(parents=True, exist_ok=True)
        if not path.is_file() or path.read_bytes() != data:
            path.write_bytes(data)


    def absolute_path(path):
        text = str(path)
        if text.startswith('#'):
            return (ROOT / text[1:]).resolve()
        path = Path(text)
        return path.resolve() if path.is_absolute() else (ROOT / path).resolve()


    def header_inputs(include_dirs):
        """Cover upstream/configuration headers without fingerprinting build output."""
        # Shared RASC headers are covered by the actual compiler include paths.
        roots = [ROOT / 'platform', UPSTREAM]
        roots += include_dirs
        files = set()
        visited = set()
        excluded = {'build', '.git', '.venv', '__pycache__'}
        generated_roots = (ROOT / 'build',)
        for root in sorted(set(roots)):
            for directory, subdirs, names in os.walk(root):
                directory = Path(directory).resolve()
                if any(directory == path or path in directory.parents for path in generated_roots):
                    subdirs[:] = []
                    continue
                if directory in visited:
                    subdirs[:] = []
                    continue
                visited.add(directory)
                subdirs[:] = [name for name in subdirs if name not in excluded]
                for name in names:
                    if Path(name).suffix.lower() in ('.h', '.hpp', '.inc'):
                        files.add(directory / name)
        return files


    def metadata_signature(sources, local_flags, include_dirs, common_flags, compiler):
        inputs = header_inputs(include_dirs)
        inputs.update(Path(source) for source in sources)
        # These source fragments are included by compiled modules rather than
        # appearing as independent entries in the SCons source list.
        inputs.update((ROOT / 'platform/micropython').glob('*_port.c'))
        inputs.update((MICROPYTHON / 'extmod').glob('machine_*.c'))
        inputs.update((MICROPYTHON / 'py').glob('*.py'))
        inputs.update((ROOT / 'rtconfig.h', ROOT / 'rtconfig_project.h',
                       ROOT / 'rtconfig.py', LOCK_FILE, SCRIPT,
                       ROOT / 'platform/port/imlib_titan.h'))
        digest = hashlib.sha256()
        fingerprint_settings = {
            'root': str(ROOT), 'compiler': str(compiler), 'python': sys.version,
            'common_flags': common_flags, 'cflags': settings['cflags'],
            'cxxflags': settings['cxxflags'], 'local_flags': local_flags,
        }
        compiler_file = compiler if compiler.is_file() else compiler.with_suffix('.exe')
        if compiler_file.is_file():
            stat = compiler_file.stat()
            fingerprint_settings['compiler_stat'] = [stat.st_size, stat.st_mtime_ns]
        digest.update(json.dumps(fingerprint_settings, sort_keys=True).encode('utf-8'))
        for path in sorted(inputs):
            digest.update(path.as_posix().encode('utf-8') + b'\0')
            digest.update(hashlib.sha256(path.read_bytes()).digest())
        return digest.hexdigest()


    def generate_metadata():
        mp = MICROPYTHON / 'py'
        output = GENERATED / 'genhdr'
        write_if_changed(output / 'mpversion.h',
            '#define MICROPY_GIT_TAG "v1.28.0-openmv-dd984559"\n'
            '#define MICROPY_GIT_HASH "dd984559"\n'
            '#define MICROPY_BUILD_DATE "2026-09-07"\n')
        local_flags = {str(absolute_path(name)): flags
                       for name, flags in settings['metadata_sources'].items()}
        sources = sorted(local_flags)
        include_dirs = [absolute_path(name) for name in settings['include_dirs']]
        common_flags = ' '.join('-I"' + path.as_posix() + '"' for path in include_dirs)
        for define in settings['defines']:
            if isinstance(define, (tuple, list)):
                define = str(define[0]) + '=' + str(define[1])
            common_flags += ' -D' + str(define).replace('"', '\\"')
        compiler = Path(settings['compiler'])
        signature = metadata_signature(sources, local_flags, include_dirs, common_flags, compiler)
        signature_file = output / 'signature'
        required = ('registrations.h', 'qstrdefs.input.h', 'qstrdefs.collected.h',
                    'qstrdefs.preprocessed.h', 'qstrdefs.generated.h',
                    'moduledefs.h', 'root_pointers.h')
        if (signature_file.is_file() and signature_file.read_text() == signature and
                all((output / name).is_file() for name in required)):
            return

        print('Generating OpenMV QSTR, module and root-pointer headers...')
        cache = ROOT / 'build' / ('metadata-gcc')
        cache.mkdir(parents=True, exist_ok=True)
        errors = []

        def preprocess(source):
            flags = settings['cxxflags'] if Path(source).suffix in ('.cc', '.cpp') else settings['cflags']
            flags = (flags + ' ' + local_flags[source]).replace(' -c', '')
            response = cache / (hashlib.sha256(source.encode()).hexdigest()[:16] + '.rsp')
            write_if_changed(response, flags + ' -E -P -DNO_QSTR ' + common_flags +
                             ' "' + Path(source).as_posix() + '"')
            result = subprocess.run([str(compiler), '@' + str(response)], cwd=ROOT,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if result.returncode:
                write_if_changed(response.with_suffix('.stderr'), result.stderr)
                diagnostics = result.stderr.decode(errors='replace')
                error_lines = [line for line in diagnostics.splitlines() if 'error:' in line]
                errors.append(source + '\n' + ('\n'.join(error_lines[:12]) or diagnostics[-1100:]))
                return []
            text = result.stdout.decode(errors='replace')
            matches = re.findall(r'MP_QSTR_[_a-zA-Z0-9]+', text)
            matches += re.findall(
                r'MP_REGISTER_(?:MODULE|EXTENSIBLE_MODULE|MODULE_DELEGATION|ROOT_POINTER)\(.*?\);', text)
            return matches

        matches = set()
        with concurrent.futures.ThreadPoolExecutor(max_workers=8) as workers:
            for result in workers.map(preprocess, sources):
                matches.update(result)
        if errors:
            raise RuntimeError('\n'.join(errors))
        if not any(re.fullmatch(r'MP_REGISTER_MODULE\(MP_QSTR_csi,\s*csi_module\);', item)
                   for item in matches):
            raise RuntimeError('OpenMV csi module is disabled: enable MICROPY_PY_CSI_NG in mpconfigport.h')
        registrations = output / 'registrations.h'
        write_if_changed(registrations, '\n'.join(sorted(matches)) + '\n')
        qstr = output / 'qstrdefs.collected.h'
        write_if_changed(qstr, '\n'.join('Q(' + item[8:] + ')' for item in sorted(matches)
                                        if item.startswith('MP_QSTR_')) + '\n')
        # Keep literal QSTRs such as /lib and obtain QCFG from the target compiler.
        definitions = (mp / 'qstrdefs.h').read_text() + '\nQ(/lib)\n'
        quoted = '\n'.join(json.dumps(line) if line.startswith('Q(') else line
                           for line in definitions.splitlines())
        qinput = output / 'qstrdefs.input.h'
        write_if_changed(qinput, quoted)
        response = cache / 'qstrdefs.rsp'
        write_if_changed(response, settings['cflags'].replace(' -c', '') +
                         ' -E -P -x c -DNO_QSTR ' + common_flags + ' "' + qinput.as_posix() + '"')
        result = subprocess.run([str(compiler), '@' + str(response)], cwd=ROOT,
                                capture_output=True, check=True)
        qdefinitions = output / 'qstrdefs.preprocessed.h'
        write_if_changed(qdefinitions, '\n'.join(
            json.loads(line) if line.startswith('"Q(') else line
            for line in result.stdout.decode().splitlines()))
        for script, input_path, filename in (
                ('makeqstrdata.py', qstr, 'qstrdefs.generated.h'),
                ('makemoduledefs.py', registrations, 'moduledefs.h'),
                ('make_root_pointers.py', registrations, 'root_pointers.h')):
            command = [sys.executable, str(mp / script)]
            if script == 'makeqstrdata.py':
                command.append(str(qdefinitions))
            write_if_changed(output / filename,
                             subprocess.check_output(command + [str(input_path)], cwd=ROOT))
        write_if_changed(signature_file, signature)


    def python_modules():
        """Use checked-in board adaptations and the other pinned upstream modules."""
        modules = {}
        if settings['ml_enabled']:
            base = UPSTREAM / 'openmv/scripts/libraries/ml'
            for package in ML_PACKAGES:
                package_root = base / package
                paths = sorted((package_root / 'ml').rglob('*.py'))
                if not paths:
                    raise RuntimeError('Missing OpenMV Python package: ' + str(package_root))
                for upstream_path in paths:
                    name = upstream_path.relative_to(package_root).as_posix()
                    source = MODULES / name if name in ML_OVERRIDES else upstream_path
                    data = source.read_bytes()
                    if name in modules and modules[name][1] != data:
                        raise RuntimeError('Conflicting frozen package entry: ' + name)
                    modules[name] = (source, data)
        base = MICROPYTHON / 'extmod/asyncio'
        for name in ('__init__.py', 'core.py', 'event.py', 'funcs.py', 'lock.py', 'stream.py'):
            source = base / name
            modules['asyncio/' + name] = (source, source.read_bytes())
        source = base / 'uasyncio.py'
        modules['uasyncio.py'] = (source, source.read_bytes())
        source = UPSTREAM / 'openmv/scripts/libraries/sensor.py'
        modules['sensor.py'] = (source, source.read_bytes())
        source = MODULES / 'lcd.py'
        modules['lcd.py'] = (source, source.read_bytes())
        return dict(sorted(modules.items()))


    def ensure_mpy_cross():
        """Build the matching host compiler; make owns its source dependencies."""
        output = ROOT / 'build/mpy-cross'
        output.mkdir(parents=True, exist_ok=True)
        executable = output / ('mpy-cross.exe' if os.name == 'nt' else 'mpy-cross')
        make = shutil.which('mingw32-make') or shutil.which('make')
        cc_name = os.environ.get('MPY_HOST_CC', 'gcc')
        cc = shutil.which(cc_name) or (cc_name if Path(cc_name).is_file() else None)
        if not make or not cc:
            missing = ', '.join(name for name, tool in (('GNU make', make), ('host GCC', cc))
                                if not tool)
            raise RuntimeError('Building mpy-cross requires native GNU make and GCC; missing: ' +
                               missing + '. Add MinGW-w64 bin to PATH, or set OPENMV_HOST_TOOLS '
                               'to a directory containing gcc.exe, make.exe and sh.exe. '
                               'MPY_HOST_CC may select a host compiler; ARM GCC cannot build '
                               'this Windows/Linux executable.')
        cc = Path(cc).resolve()
        process_env = os.environ.copy()
        process_env['PYTHONDONTWRITEBYTECODE'] = '1'
        paths = [str(cc.parent)]
        if os.name == 'nt':
            from tools.host_tools import find_posix_shell
            shell = find_posix_shell(process_env)
            paths.append(str(Path(shell).parent))
        process_env['PATH'] = os.pathsep.join(paths + [process_env.get('PATH', '')])
        python = sys.executable.replace('\\', '/')
        if ' ' in python:
            python = '"' + python + '"'
        command = [make, '-j' + os.environ.get('MPY_HOST_JOBS', '2'), 'PYTHON=' + python,
                   'CC=' + cc.name, 'BUILD=' + output.as_posix()]
        if os.name == 'nt':
            command.append('SHELL=sh.exe')
        signature = hashlib.sha256((str(MICROPYTHON.resolve()) + '\n' + str(cc) + '\n' +
                                    hashlib.sha256(LOCK_FILE.read_bytes()).hexdigest()).encode()).hexdigest()
        signature_file = output / 'source.signature'
        if not signature_file.is_file() or signature_file.read_text() != signature:
            command.append('-B')
        result = subprocess.run(command, cwd=MICROPYTHON / 'mpy-cross', env=process_env,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        write_if_changed(output / 'build.log', result.stdout)
        if result.returncode or not executable.is_file():
            raise RuntimeError('mpy-cross build failed; see %s\n%s' %
                               (output / 'build.log', result.stdout.decode(errors='replace')[-3000:]))
        write_if_changed(signature_file, signature)
        return executable


    def generate_frozen():
        # Frozen C adds a separate QSTR pool and is intentionally excluded from
        # target metadata discovery. Freeze only after that target pool is complete.
        modules = python_modules()
        cross = ensure_mpy_cross()
        output = GENERATED / 'frozen_content.c'
        cache = ROOT / 'build/frozen'
        qstr = cache / 'qstrdefs.complete.h'
        write_if_changed(qstr, b'\n'.join((GENERATED / 'genhdr' / name).read_bytes()
                                         for name in ('qstrdefs.preprocessed.h', 'qstrdefs.collected.h')))
        fingerprints = {
            name: {'source': source.relative_to(ROOT).as_posix(), 'size': len(data),
                   'sha256': hashlib.sha256(data).hexdigest(),
                   'platform_adapted': name in ML_OVERRIDES}
            for name, (source, data) in modules.items()
        }
        mpy_tool = MICROPYTHON / 'tools/mpy-tool.py'
        digest = hashlib.sha256(json.dumps(fingerprints, sort_keys=True).encode())
        digest.update(str(ROOT).encode())
        for path in (qstr, cross, LOCK_FILE, mpy_tool, SCRIPT):
            digest.update(path.read_bytes())
        # mpy-tool imports the matching QSTR implementation.
        for path in sorted((MICROPYTHON / 'py').glob('*.py')):
            digest.update(path.name.encode() + path.read_bytes())
        signature = digest.hexdigest()
        signature_file = cache / 'frozen.signature'
        if (output.is_file() and output.with_suffix('.json').is_file() and
                signature_file.is_file() and signature_file.read_text() == signature):
            return
        mpy_files = []
        for name, (source, data) in modules.items():
            destination = cache / Path(name).with_suffix('.mpy')
            destination.parent.mkdir(parents=True, exist_ok=True)
            compile_source = cache / 'sources' / name
            write_if_changed(compile_source, data)
            subprocess.run([str(cross), '-O2', '-msmall-int-bits=31', '-s', name,
                            '-o', str(destination), str(compile_source)], cwd=ROOT, check=True)
            mpy_files.append(destination)
        command = [sys.executable, str(mpy_tool), '-f', '-q', str(qstr),
                   '-mlongint-impl', 'mpz', '-mmpz-dig-size', '16']
        process_env = os.environ.copy()
        process_env['PYTHONDONTWRITEBYTECODE'] = '1'
        result = subprocess.run(command + [str(path) for path in mpy_files], cwd=ROOT,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                check=True, env=process_env)
        write_if_changed(output, result.stdout)
        report = {
            'format': 'MicroPython frozen MPY', 'lock_file': str(LOCK_FILE),
            'lock_sha256': hashlib.sha256(LOCK_FILE.read_bytes()).hexdigest(),
            'source_bytes': sum(value['size'] for value in fingerprints.values()),
            'mpy_bytes': sum(path.stat().st_size for path in mpy_files),
            'module_count': len(modules), 'files': fingerprints,
            'mpy_cross_sha256': hashlib.sha256(cross.read_bytes()).hexdigest(),
            'generated_sha256': hashlib.sha256(result.stdout).hexdigest(),
        }
        write_if_changed(output.with_suffix('.json'), json.dumps(report, indent=2) + '\n')
        write_if_changed(signature_file, signature)
        print('OpenMV frozen Python: %d modules, %d MPY bytes' %
              (report['module_count'], report['mpy_bytes']))



    generate_metadata()
    generate_frozen()
