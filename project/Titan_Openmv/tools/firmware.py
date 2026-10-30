"""Package the linked vision image and publish the matching firmware pair."""
import hashlib
import struct
import zlib

FLASH = (0x02000000, 0x02100000)
VISION = (0x70C00000, 0x71000000)
HEADER = struct.Struct('<8sIIII32s8s')
DEFLATE_WBITS = -13


class ELF:
    """Read the ELF32 sections and symbols needed to bind the vision image."""

    def __init__(self, path):
        self.data = path.read_bytes()
        if self.data[:7] != b'\x7fELF\x01\x01\x01':
            raise ValueError('Expected little-endian ELF32')
        header = self.unpack('<16sHHIIIIIHHHHHH', 0)
        _, typ, machine, _, _, _, shoff, _, _, phsize, _, shsize, shnum, shstr = header
        if typ != 2 or machine != 40:
            raise ValueError('Expected final ET_EXEC / ARM firmware')
        if phsize != 32 or shsize != 40 or not shnum or shstr >= shnum:
            raise ValueError('Unsupported/truncated ELF table layout')
        self.sections = []
        for n in range(shnum):
            values = self.unpack('<IIIIIIIIII', shoff + n * shsize)
            self.sections.append(dict(zip(
                ('nameidx', 'type', 'flags', 'addr', 'offset', 'size',
                 'link', 'info', 'align', 'entsize'), values)))
        strings = self.section_bytes(self.sections[shstr])
        for section in self.sections:
            section['name'] = self.string(strings, section['nameidx'])
        self.symbols = {}
        for section in self.sections:
            if section['type'] != 2:
                continue
            if section['entsize'] != 16:
                raise ValueError('Unsupported ELF symbol size')
            names = self.section_bytes(self.sections[section['link']])
            for offset in range(section['offset'], section['offset'] + section['size'], 16):
                index, value, size, info, _, shndx = self.unpack('<IIIBBH', offset)
                if not index or not shndx or info & 15 == 4:
                    continue
                name = self.string(names, index)
                symbol = dict(value=value, size=size, section=shndx)
                self.symbols.setdefault(name, []).append(symbol)

    def unpack(self, fmt, offset):
        if offset < 0 or offset + struct.calcsize(fmt) > len(self.data):
            raise ValueError('Truncated ELF data')
        return struct.unpack_from(fmt, self.data, offset)

    @staticmethod
    def string(data, offset):
        if not 0 <= offset < len(data):
            raise ValueError('Invalid ELF string offset')
        return data[offset:data.index(b'\0', offset)].decode('utf-8', 'replace')

    def section_bytes(self, section):
        start, end = section['offset'], section['offset'] + section['size']
        if section['type'] == 8 or end > len(self.data):
            raise ValueError('Missing or truncated ELF section data')
        return self.data[start:end]

    def symbol(self, name):
        matches = self.symbols.get(name, [])
        if not matches:
            raise ValueError('Required firmware symbol missing: ' + name)
        if len({(s['value'], s['size']) for s in matches}) > 1:
            raise ValueError('Ambiguous firmware symbol: ' + name)
        return matches[0]


def vision_image(elf):
    section = next((s for s in elf.sections if s['name'] == '.openmv_vision'), None)
    if (section is None or section['type'] != 1 or not section['flags'] & 2
            or not section['flags'] & 4 or section['addr'] != VISION[0]
            or not section['size'] or section['size'] % 32
            or section['addr'] + section['size'] > VISION[1]):
        raise ValueError('Invalid vision image region')
    if (elf.symbol('__openmv_vision_start')['value'] != section['addr']
            or elf.symbol('__openmv_vision_end')['value'] != section['addr'] + section['size']):
        raise ValueError('Vision image linker bounds differ')
    payload = elf.section_bytes(section)
    digest = hashlib.sha256(payload).digest()
    compressor = zlib.compressobj(level=9, wbits=DEFLATE_WBITS)
    encoded = compressor.compress(payload) + compressor.flush()
    if len(encoded) < len(payload):
        if zlib.decompress(encoded, DEFLATE_WBITS) != payload:
            raise ValueError('Vision image compression failed')
        version, stored = 2, encoded
        reserved = struct.pack('<II', 1, len(stored))
        encoding = 'deflate'
    else:
        version, stored, reserved, encoding = 1, payload, bytes(8), 'raw'
    blob = HEADER.pack(b'OMVVISN1', version, VISION[0], len(payload),
                       zlib.crc32(payload), digest, reserved) + stored

    symbol = elf.symbol('titan_vision_build_id')
    if (symbol['size'] != 32 or not FLASH[0] <= symbol['value']
            or symbol['value'] + 32 > FLASH[1]
            or symbol['section'] >= len(elf.sections)):
        raise ValueError('Vision binding must be a 32-byte internal flash object')
    containing = elf.sections[symbol['section']]
    delta = symbol['value'] - containing['addr']
    if containing['type'] != 1 or delta < 0 or delta + 32 > containing['size']:
        raise ValueError('Vision binding is not initialized data')
    # Validate the containing file extent before replacing its fixed-size hash.
    elf.section_bytes(containing)
    offset = containing['offset'] + delta
    patched = bytearray(elf.data)
    patched[offset:offset + 32] = digest
    print('Vision image: %d bytes at 0x%08x; %s %d bytes + %d-byte header' %
          (len(payload), section['addr'], encoding, len(stored), HEADER.size))
    return patched, blob
