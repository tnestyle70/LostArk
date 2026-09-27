#!/usr/bin/env python3
"""Export a Texture2D out of a retail LostArk UE3 `.upk` package as PNG.

`umodel` cannot read these packages: it serialises `FCompressedChunk` as the standard 16 bytes
while LostArk writes 20 (one extra int32), so every chunk offset past the first comes out as
garbage and the export data appears to sit beyond the end of the file. This reader uses the real
20-byte stride, AES-256-ECB decrypts the head of each block with the shipped key and LZ4-raw
decompresses it, which is what the retail loader does for `CompressionFlags == 0x44`.

    python export_upk_texture.py <package.upk> <object name> [--out DIR] [--list] [--props]

`--list` prints the package's export table instead of exporting, `--props` also prints the
texture's tagged properties. The UI atlas pages this is used for are the `DefineExternalImage2`
targets of a `.gfx` in the same package, so the page name is usually `<movie>_i<N>`.

Two serialisation quirks this engine adds, both required to walk the properties at all:
export data starts 4 bytes after `SerialOffset`, and an `IntProperty` writes three int32s
(marker, 0, value) for a declared size of 4, so the real value is the third and the cursor
advances by size + 8.
"""
from __future__ import annotations

import argparse
import io
import struct
import sys
from pathlib import Path

try:
    from Crypto.Cipher import AES
except ImportError:  # pragma: no cover - dependency hint
    sys.exit('pycryptodome is required: python -m pip install pycryptodome')
try:
    import lz4.block
except ImportError:  # pragma: no cover - dependency hint
    sys.exit('lz4 is required: python -m pip install lz4')

PACKAGE_KEY = b'V1ZEG1PL34V77SQW39A9I4VUW34T6L15'
COMPRESSED_BLOCK_MAGIC = 0x9E2A83C1
ENCRYPTED_FLAGS = 0x44
# The engine keeps one extra int32 per chunk after the standard four offsets/sizes.
COMPRESSED_CHUNK_SIZE = 20
# Export data begins this far past the header's SerialOffset.
EXPORT_DATA_SKEW = 4


class Reader:
    def __init__(self, data: bytes, offset: int = 0):
        self.data = data
        self.offset = offset

    def u32(self) -> int:
        value, = struct.unpack_from('<I', self.data, self.offset)
        self.offset += 4
        return value

    def i32(self) -> int:
        value, = struct.unpack_from('<i', self.data, self.offset)
        self.offset += 4
        return value

    def u16(self) -> int:
        value, = struct.unpack_from('<H', self.data, self.offset)
        self.offset += 2
        return value

    def u64(self) -> int:
        value, = struct.unpack_from('<Q', self.data, self.offset)
        self.offset += 8
        return value

    def u8(self) -> int:
        value = self.data[self.offset]
        self.offset += 1
        return value

    def raw(self, count: int) -> bytes:
        value = self.data[self.offset:self.offset + count]
        self.offset += count
        return value

    def string(self) -> str:
        count = self.i32()
        if count == 0:
            return ''
        if count < 0:
            return self.raw(-count * 2).decode('utf-16-le').rstrip('\0')
        return self.raw(count).decode('cp949', 'replace').rstrip('\0')


def decompress_block_stream(data: bytes, offset: int, compression_flags: int) -> bytes:
    """Expand one `0x9E2A83C1` block stream, the form both package chunks and bulk data use."""
    _, block_size, _, total_uncompressed = struct.unpack_from('<4I', data, offset)
    block_count = (total_uncompressed + block_size - 1) // block_size
    cursor = offset + 16 + 8 * block_count
    out = bytearray()
    for index in range(block_count):
        block_compressed, block_uncompressed = struct.unpack_from(
            '<II', data, offset + 16 + 8 * index)
        block = bytearray(data[cursor:cursor + block_compressed])
        cursor += block_compressed
        if compression_flags == ENCRYPTED_FLAGS:
            # Only the leading whole AES blocks are encrypted, capped at one page.
            encrypted = min(block_compressed & ~15, 4096)
            if encrypted > 0:
                block[0:encrypted] = AES.new(PACKAGE_KEY, AES.MODE_ECB).decrypt(
                    bytes(block[0:encrypted]))
        out += lz4.block.decompress(bytes(block), uncompressed_size=block_uncompressed)
    return bytes(out)


def decompress_package(path: Path) -> bytes:
    """Return the package with every compressed chunk expanded in place."""
    data = path.read_bytes()
    reader = Reader(data)
    reader.u32()                     # tag
    reader.u16(); reader.u16()       # file / licensee version
    header_size = reader.i32()
    reader.string()                  # folder name
    reader.u32()                     # package flags
    for _ in range(7):               # name/export/import counts and offsets, depends offset
        reader.u32()
    reader.raw(16)                   # import/export guid counts and thumbnail table
    reader.raw(16)                   # package guid
    generation_count = reader.u32()
    reader.raw(generation_count * 12)
    reader.u32(); reader.u32()       # engine / cooker version
    compression_flags = reader.u32()
    chunk_count = reader.u32()

    chunks = []
    for _ in range(chunk_count):
        fields = struct.unpack_from('<5I', data, reader.offset)
        reader.offset += COMPRESSED_CHUNK_SIZE
        chunks.append(fields)
    if not chunks:
        return data

    total = max(uncompressed_offset + uncompressed_size
                for uncompressed_offset, uncompressed_size, _, _, _ in chunks)
    out = bytearray(max(total, header_size))
    out[0:chunks[0][0]] = data[0:chunks[0][0]]

    for uncompressed_offset, uncompressed_size, compressed_offset, compressed_size, _ in chunks:
        magic, = struct.unpack_from('<I', data, compressed_offset)
        if magic != COMPRESSED_BLOCK_MAGIC:
            out[uncompressed_offset:uncompressed_offset + uncompressed_size] = \
                data[compressed_offset:compressed_offset + compressed_size]
            continue
        _, block_size, _, total_uncompressed = struct.unpack_from('<4I', data, compressed_offset)
        block_count = (total_uncompressed + block_size - 1) // block_size
        cursor = compressed_offset + 16 + 8 * block_count
        destination = uncompressed_offset
        for index in range(block_count):
            block_compressed, block_uncompressed = struct.unpack_from(
                '<II', data, compressed_offset + 16 + 8 * index)
            block = bytearray(data[cursor:cursor + block_compressed])
            cursor += block_compressed
            if compression_flags == ENCRYPTED_FLAGS:
                # Only the leading whole AES blocks are encrypted, capped at one page.
                encrypted = min(block_compressed & ~15, 4096)
                if encrypted > 0:
                    block[0:encrypted] = AES.new(PACKAGE_KEY, AES.MODE_ECB).decrypt(
                        bytes(block[0:encrypted]))
            expanded = lz4.block.decompress(bytes(block), uncompressed_size=block_uncompressed)
            out[destination:destination + block_uncompressed] = expanded
            destination += block_uncompressed
    return bytes(out)


class Package:
    def __init__(self, data: bytes):
        self.data = data
        reader = Reader(data)
        reader.u32()
        self.version = reader.u16()
        self.licensee_version = reader.u16()
        reader.i32()
        reader.string()
        reader.u32()
        name_count = reader.u32()
        name_offset = reader.u32()
        export_count = reader.u32()
        export_offset = reader.u32()
        import_count = reader.u32()
        import_offset = reader.u32()
        reader.u32()

        self.names: list[str] = []
        cursor = Reader(data, name_offset)
        for _ in range(name_count):
            self.names.append(cursor.string())
            cursor.u64()

        self.imports: list[str] = []
        cursor = Reader(data, import_offset)
        for _ in range(import_count):
            cursor.i32(); cursor.i32()          # package name
            cursor.i32(); cursor.i32()          # class name
            cursor.i32()                        # outer
            object_name = self.name(cursor.i32())
            cursor.i32()
            self.imports.append(object_name)

        # The table runs from export_offset up to the first export's data, which is where the
        # header ends. Entries are fixed width, so the stride follows from the count.
        first_data_offset = None
        stride = (self._probe_stride(export_offset, export_count)
                  if export_count else 0)
        self.exports: list[dict] = []
        for index in range(export_count):
            entry = Reader(data, export_offset + index * stride)
            class_index = entry.i32()
            entry.i32()                         # super
            entry.i32()                         # outer
            name = self.name(entry.i32())
            entry.i32()
            entry.i32()                         # archetype
            entry.u64()                         # object flags
            size = entry.i32()
            offset = entry.i32()
            self.exports.append({
                'name': name,
                'class': self._class_name(class_index),
                'size': size,
                'offset': offset,
            })
            if first_data_offset is None or offset < first_data_offset:
                first_data_offset = offset
        self.export_stride = stride

    def _probe_stride(self, export_offset: int, export_count: int) -> int:
        """Derive the export entry width from the gap to the first export's data."""
        # The first entry's SerialOffset is the end of the header, so the table fills the space
        # between export_offset and that offset.
        probe = Reader(self.data, export_offset)
        for _ in range(8):
            probe.i32()
        probe.u64()
        probe.i32()
        first_offset = probe.i32()
        span = first_offset - export_offset
        if export_count <= 0 or span <= 0 or span % export_count:
            return 68
        return span // export_count

    def name(self, index: int) -> str:
        return self.names[index] if 0 <= index < len(self.names) else '<name%d>' % index

    def _class_name(self, class_index: int) -> str:
        if class_index < 0:
            position = -class_index - 1
            if 0 <= position < len(self.imports):
                return self.imports[position]
        if class_index == 0:
            return 'class'
        return '<class%d>' % class_index

    def property_name(self, offset: int) -> tuple[str, int]:
        index, number = struct.unpack_from('<ii', self.data, offset)
        text = self.name(index)
        return (text if number == 0 else '%s_%d' % (text, number - 1)), offset + 8

    def read_properties(self, offset: int, end: int) -> tuple[dict, int]:
        """Read UE3 tagged properties, applying this engine's IntProperty padding."""
        values: dict[str, object] = {}
        cursor = offset
        while cursor < end:
            name, cursor = self.property_name(cursor)
            if name.lower() == 'none':
                break
            kind, cursor = self.property_name(cursor)
            kind = kind.lower()
            size, = struct.unpack_from('<i', self.data, cursor); cursor += 4
            struct.unpack_from('<i', self.data, cursor); cursor += 4   # array index
            boolean = None
            if kind == 'structproperty':
                _, cursor = self.property_name(cursor)
            elif kind == 'boolproperty':
                boolean = self.data[cursor] != 0
                cursor += 1
            elif kind == 'byteproperty':
                _, cursor = self.property_name(cursor)
            body = cursor
            if kind == 'boolproperty':
                values[name] = boolean
            elif kind == 'intproperty':
                _, _, value = struct.unpack_from('<3i', self.data, body)
                values[name] = value
            elif kind == 'byteproperty':
                enum_value, _ = self.property_name(body)
                values[name] = enum_value
            elif kind == 'strproperty':
                length, = struct.unpack_from('<i', self.data, body)
                values[name] = self.data[body + 4:body + 4 + length].decode(
                    'cp949', 'replace').rstrip('\0') if length > 0 else ''
            else:
                values[name] = self.data[body:body + size]
            cursor = body + size + (8 if kind == 'intproperty' else 0)
        return values, cursor


DDS_FOURCC = {'pf_dxt1': b'DXT1', 'pf_dxt3': b'DXT3', 'pf_dxt5': b'DXT5'}
BLOCK_BYTES = {b'DXT1': 8, b'DXT3': 16, b'DXT5': 16}
# Uncompressed pages store one 32-bit pixel per texel in this channel order.
UNCOMPRESSED_FORMATS = {'pf_a8r8g8b8': 'BGRA'}


def dds_bytes(width: int, height: int, fourcc: bytes, payload: bytes) -> bytes:
    header = bytearray(128)
    header[0:4] = b'DDS '
    struct.pack_into('<I', header, 4, 124)
    struct.pack_into('<I', header, 8, 0x1 | 0x2 | 0x4 | 0x1000 | 0x80000)
    struct.pack_into('<I', header, 12, height)
    struct.pack_into('<I', header, 16, width)
    struct.pack_into('<I', header, 20, len(payload))
    struct.pack_into('<I', header, 28, 1)
    struct.pack_into('<I', header, 76, 32)
    struct.pack_into('<I', header, 80, 0x4)
    header[84:88] = fourcc
    struct.pack_into('<I', header, 108, 0x1000)
    return bytes(header) + payload


def find_bulk_payload(data: bytes, start: int, end: int) -> tuple[int, int] | None:
    """Return the first mip's payload from the `FByteBulkData` that follows the properties.

    The mip array is `mipCount` then, per mip, a bulk-data header of flags, element count, size
    on disk and offset in file, with the bytes stored inline directly after that header. Rather
    than assume where the array begins this looks for the header whose own offset field points
    at the 16 bytes past itself, which is specific enough that a stray match is not possible.
    """
    for offset in range(start, max(start, end - 16) + 1):
        flags, count, size_on_disk, offset_in_file = struct.unpack_from('<4i', data, offset)
        if flags < 0 or count <= 0 or size_on_disk <= 0:
            continue
        if offset_in_file != offset + 16 or offset_in_file + size_on_disk > end:
            continue
        return offset_in_file, size_on_disk
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('package', type=Path)
    parser.add_argument('object', nargs='?', default=None)
    parser.add_argument('--out', type=Path, default=Path('.'))
    parser.add_argument('--list', action='store_true', dest='list_exports')
    parser.add_argument('--props', action='store_true')
    parser.add_argument('--raw', type=Path, default=None,
                        help='also write the decompressed package to this path')
    args = parser.parse_args()

    data = decompress_package(args.package)
    if args.raw:
        args.raw.write_bytes(data)
        print('decompressed %d bytes -> %s' % (len(data), args.raw))
    package = Package(data)

    if args.list_exports or not args.object:
        print('%s  version %d/%d  %d names  %d exports (stride %d)'
              % (args.package.name, package.version, package.licensee_version,
                 len(package.names), len(package.exports), package.export_stride))
        for export in package.exports:
            print('  %-24s %-16s size=%-10d offset=0x%X'
                  % (export['name'], export['class'], export['size'], export['offset']))
        return 0

    wanted = args.object.lower()
    match = next((e for e in package.exports if e['name'].lower() == wanted), None)
    if match is None:
        print('no export named %r; use --list' % args.object, file=sys.stderr)
        return 1

    start = match['offset'] + EXPORT_DATA_SKEW
    end = match['offset'] + match['size']
    properties, after = package.read_properties(start, end)
    if args.props:
        for key, value in properties.items():
            shown = value if not isinstance(value, bytes) else value[:24].hex()
            print('  %-28s %s' % (key, shown))
        print('  properties end at 0x%X, export ends at 0x%X' % (after, end))

    crunched = bool(properties.get('bUseCrunchCompression')
                    or properties.get('busecrunchcompression'))

    width = properties.get('SizeX') or properties.get('sizex')
    height = properties.get('SizeY') or properties.get('sizey')
    pixel_format = str(properties.get('Format') or properties.get('format') or '').lower()
    if not width or not height:
        print('no SizeX/SizeY on %s' % match['name'], file=sys.stderr)
        return 2
    fourcc = DDS_FOURCC.get(pixel_format)
    channels = UNCOMPRESSED_FORMATS.get(pixel_format)
    if fourcc is None and channels is None:
        print('unsupported pixel format %r on %s' % (pixel_format, match['name']), file=sys.stderr)
        return 2

    located = find_bulk_payload(data, after, end)
    if located is None:
        print('could not locate the mip bulk data in %s' % match['name'], file=sys.stderr)
        return 2
    payload_offset, payload_size = located
    payload = data[payload_offset:payload_offset + payload_size]
    if payload[:4] == struct.pack('<I', COMPRESSED_BLOCK_MAGIC):
        # Larger pages store the mip as the same block stream the package chunks use. The object
        # still sets bUseCrunchCompression, but that flag does not describe these bytes.
        payload = decompress_block_stream(data, payload_offset, 0)
    elif crunched:
        # The mip is a crunch stream. Both decoders accept it and both return the right number
        # of bytes, but only the Unity-flavoured one produces valid blocks -- stock crnlib
        # decodes these to noise, which is what makes a bad export look like a successful one.
        import texture2ddecoder
        payload = texture2ddecoder.unpack_unity_crunch(payload)
        if not payload:
            print('crunch unpack produced nothing for %s' % match['name'], file=sys.stderr)
            return 2

    from PIL import Image
    if channels is not None:
        expected = width * height * 4
        image = Image.frombytes('RGBA', (width, height), payload[:expected], 'raw', channels)
    else:
        expected = max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * BLOCK_BYTES[fourcc]
        image = Image.open(io.BytesIO(dds_bytes(width, height, fourcc, payload[:expected])))
        image = image.convert('RGBA')
    args.out.mkdir(parents=True, exist_ok=True)
    destination = args.out / ('%s.png' % match['name'])
    image.save(destination)
    print('%s  %dx%d  %s%s  -> %s'
          % (match['name'], width, height, pixel_format,
             ' (crunch)' if crunched else '', destination))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
