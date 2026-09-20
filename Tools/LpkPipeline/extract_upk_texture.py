"""Export a Texture2D out of a Lost Ark .upk without UModel.

UModel cannot read these packages any more: its Lost Ark reader assumes four int32 per
compressed-chunk entry, while the shipped packages write five (an extra flag of 1 after each
entry), so every offset after the first chunk lands somewhere else and the read runs past the end
of the file. The tagged-property stream has the same shape problem -- Lost Ark writes an extra
FName after Size/ArrayIndex that stock UE3 does not -- so even a patched chunk table only gets as
far as "unknown type" while parsing the texture.

This reads the format directly:

  header            plain up to the name-table offset.
  chunk table       flags, count, then per chunk (UncompOffset, UncompSize, CompOffset,
                    CompSize, 1). Flags 0x44 means each block is AES-256-ECB over its first
                    min(4096, size & ~15) bytes, then LZ4. Bulk payloads reuse the same chunk
                    header but are plain LZ4.
  property tag      Name(8) Type(8) Size(4) ArrayIndex(4), then an extra FName(8) and the value,
                    except BoolProperty which stores one byte in place of both.
  mip chain         after the properties: three zeroes, a self-pointer, the mip count, then per
                    mip a FByteBulkData (flags, element count, size on disk, offset) whose
                    payload is another compressed chunk when the flags say so.

The mip comes out as the cooked DXT surface, which is written as a .dds (Pillow decodes those)
and, with --png, also as a decoded .png. Nothing is resampled or recompressed.

  python extract_upk_texture.py --package <path.upk> --object masterkey_i4 --out <dir>
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

import lz4.block
from Crypto.Cipher import AES

AES_KEY = b"V1ZEG1PL34V77SQW39A9I4VUW34T6L15"
COMPRESSED_TAG = 0x9E2A83C1
# The whole package and every bulk payload use this one scheme.
LZ4_AES_FLAGS = 0x44


def _decrypt_lz4(block: bytes, uncompressed_size: int) -> bytes:
    """The package's own chunks are encrypted over their first 4 KiB; the bulk payloads inside
    them are plain LZ4. Both carry the same chunk header, so which one this is only shows when
    the block is decoded."""
    try:
        return lz4.block.decompress(block, uncompressed_size=uncompressed_size)
    except lz4.block.LZ4BlockError:
        prefix = min(4096, len(block) & ~15)
        body = AES.new(AES_KEY, AES.MODE_ECB).decrypt(block[:prefix]) + block[prefix:]
        return lz4.block.decompress(body, uncompressed_size=uncompressed_size)


def _read_compressed_chunk(data: bytes, offset: int) -> bytes:
    """One FCompressedChunkHeader plus its blocks, at an absolute file offset."""
    tag, block_size, _comp_total, uncomp_total = struct.unpack_from("<IIii", data, offset)
    if tag != COMPRESSED_TAG:
        raise ValueError("not a compressed chunk at 0x%X" % offset)
    count = (uncomp_total + block_size - 1) // block_size
    sizes = [struct.unpack_from("<ii", data, offset + 16 + i * 8) for i in range(count)]
    payload = offset + 16 + count * 8
    out = bytearray()
    for comp_size, uncomp_size in sizes:
        out += _decrypt_lz4(data[payload:payload + comp_size], uncomp_size)
        payload += comp_size
    return bytes(out)


def read_header(raw: bytes) -> dict:
    """Field positions depend on the folder name's length, so the header is walked, not indexed."""
    offset = 12
    length, = struct.unpack_from("<i", raw, offset)
    offset += 4 + length + 4  # folder name and the package flags
    name_count, name_offset, export_count, export_offset = struct.unpack_from("<4i", raw, offset)
    return {"names": (name_count, name_offset), "exports": (export_count, export_offset),
            "chunks": offset + 24 + 4 + 16 + 4 + 12 + 8}


def decompress_package(raw: bytes) -> bytes:
    """The package as it would be on disk uncompressed; offsets in it are the package's own."""
    header = read_header(raw)
    name_offset = header["names"][1]
    flags, count = struct.unpack_from("<ii", raw, 0x6D)
    if flags != LZ4_AES_FLAGS:
        raise ValueError("unexpected compression flags 0x%X" % flags)
    chunks = []
    cursor = 0x75
    for _ in range(count):
        chunks.append(struct.unpack_from("<4i", raw, cursor))
        cursor += 20  # four fields and the trailing 1
    total = max(u_off + u_size for u_off, u_size, _, _ in chunks)
    out = bytearray(total)
    out[:name_offset] = raw[:name_offset]
    for u_off, u_size, c_off, _c_size in chunks:
        block = _read_compressed_chunk(raw, c_off)
        if len(block) != u_size:
            raise ValueError("chunk at 0x%X gave %d bytes, expected %d" % (c_off, len(block), u_size))
        out[u_off:u_off + u_size] = block
    return bytes(out)


def read_names(pkg: bytes) -> list[str]:
    count, offset = read_header(pkg)["names"]
    names = []
    for _ in range(count):
        length, = struct.unpack_from("<i", pkg, offset)
        offset += 4
        names.append(pkg[offset:offset + length - 1].decode("latin1"))
        offset += length + 8  # the name's own flags
    return names


# class, super, outer, FName(2), archetype, flags(2), size, offset, then seven unused ints.
EXPORT_ENTRY_BYTES = 68


def read_exports(pkg: bytes, names: list[str]) -> dict:
    count, offset = read_header(pkg)["exports"]
    exports = {}
    for _ in range(count):
        name_index, _number = struct.unpack_from("<ii", pkg, offset + 12)
        size, serial = struct.unpack_from("<ii", pkg, offset + 32)
        if not 0 <= name_index < len(names) or not 0 < serial < len(pkg):
            raise ValueError("export table does not line up at 0x%X" % offset)
        exports[names[name_index]] = (serial, size)
        offset += EXPORT_ENTRY_BYTES
    return exports


def read_properties(pkg: bytes, base: int, names: list[str]) -> tuple[dict, int]:
    """Lost Ark tagged properties. Returns the values and where the stream ended."""
    name_of = lambda i: names[i] if 0 <= i < len(names) else "?%d" % i
    offset = base + 4  # the export's leading int
    values = {}
    while True:
        name_index, _ = struct.unpack_from("<ii", pkg, offset)
        if name_of(name_index) == "none":
            return values, offset + 8
        type_index, _ = struct.unpack_from("<ii", pkg, offset + 8)
        size, _array_index = struct.unpack_from("<ii", pkg, offset + 16)
        kind = name_of(type_index)
        if kind == "boolproperty":
            values[name_of(name_index)] = bool(pkg[offset + 24])
            offset += 25
            continue
        data = pkg[offset + 32:offset + 32 + size]
        if kind == "intproperty":
            values[name_of(name_index)] = struct.unpack_from("<i", data, 0)[0]
        elif kind == "byteproperty":
            values[name_of(name_index)] = (
                name_of(struct.unpack_from("<i", data, 0)[0]) if size == 8 else data[0])
        else:
            values[name_of(name_index)] = data
        offset += 32 + size


# Cooked block formats, by the EPixelFormat name the package stores.
DDS_FOURCC = {"pf_dxt1": b"DXT1", "pf_dxt3": b"DXT3", "pf_dxt5": b"DXT5"}


def write_dds(path: Path, width: int, height: int, fourcc: bytes, surface: bytes) -> None:
    header = bytearray(128)
    header[0:4] = b"DDS "
    struct.pack_into("<i", header, 4, 124)
    struct.pack_into("<i", header, 8, 0x1 | 0x2 | 0x4 | 0x1000 | 0x80000)  # caps/size/px/linear
    struct.pack_into("<i", header, 12, height)
    struct.pack_into("<i", header, 16, width)
    struct.pack_into("<i", header, 20, len(surface))
    struct.pack_into("<i", header, 76, 32)      # pixel format size
    struct.pack_into("<i", header, 80, 0x4)     # DDPF_FOURCC
    header[84:88] = fourcc
    struct.pack_into("<i", header, 108, 0x1000)  # DDSCAPS_TEXTURE
    path.write_bytes(bytes(header) + surface)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--package", type=Path, required=True)
    parser.add_argument("--object", required=True, help="Texture2D export name, lower case")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--png", action="store_true", help="also decode the surface to .png")
    args = parser.parse_args()

    pkg = decompress_package(args.package.read_bytes())
    names = read_names(pkg)
    exports = read_exports(pkg, names)
    if args.object not in exports:
        print("no such export: %s (have %s)" % (args.object, ", ".join(sorted(exports))))
        return 1
    serial, _size = exports[args.object]
    props, end = read_properties(pkg, serial, names)
    width = props.get("sizex")
    height = props.get("sizey")
    pixel_format = props.get("format")
    if not width or not height or pixel_format not in DDS_FOURCC:
        print("unsupported texture: %dx%s %s" % (width or 0, height, pixel_format))
        return 1

    # three zeroes, the self-pointer, then the mip count
    mip_count, = struct.unpack_from("<i", pkg, end + 16)
    cursor = end + 20
    flags, element_count, size_on_disk, data_offset = struct.unpack_from("<4i", pkg, cursor)
    surface = (_read_compressed_chunk(pkg, data_offset)
               if struct.unpack_from("<I", pkg, data_offset)[0] == COMPRESSED_TAG
               else pkg[data_offset:data_offset + size_on_disk])
    if len(surface) != element_count:
        print("mip0 gave %d bytes, expected %d" % (len(surface), element_count))
        return 1

    args.out.mkdir(parents=True, exist_ok=True)
    dds_path = args.out / (args.object + ".dds")
    write_dds(dds_path, width, height, DDS_FOURCC[pixel_format], surface)
    print("%s %dx%d %s, %d mip(s), flags 0x%X -> %s"
          % (args.object, width, height, pixel_format, mip_count, flags, dds_path))
    if args.png:
        from PIL import Image
        png_path = args.out / (args.object + ".png")
        Image.open(dds_path).convert("RGBA").save(png_path)
        print("  -> %s" % png_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
