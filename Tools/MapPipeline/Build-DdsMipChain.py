from __future__ import annotations

import argparse
import hashlib
import io
import os
from pathlib import Path
import re
import struct
import sys

from PIL import Image


DDS_HEADER_BYTES = 128
DDSD_MIPMAPCOUNT = 0x00020000
DDSCAPS_COMPLEX = 0x00000008
DDSCAPS_MIPMAP = 0x00400000


def read_u32(data: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def write_u32(data: bytearray, offset: int, value: int) -> None:
    struct.pack_into("<I", data, offset, value)


def dxt1_payload_size(width: int, height: int) -> int:
    return max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * 8


def dimensions(width: int, height: int) -> list[tuple[int, int]]:
    result = [(width, height)]
    while width > 1 or height > 1:
        width = max(1, width // 2)
        height = max(1, height // 2)
        result.append((width, height))
    return result


def validate_source(path: Path, source: bytes) -> tuple[int, int, int]:
    if len(source) < DDS_HEADER_BYTES or source[:4] != b"DDS ":
        raise ValueError(f"{path}: not a legacy DDS file")
    if read_u32(source, 4) != 124 or read_u32(source, 76) != 32:
        raise ValueError(f"{path}: unsupported DDS header")
    if source[84:88] != b"DXT1":
        raise ValueError(f"{path}: only DXT1 is supported")

    width = read_u32(source, 16)
    height = read_u32(source, 12)
    mip_count = read_u32(source, 28)
    if width == 0 or height == 0:
        raise ValueError(f"{path}: invalid dimensions {width}x{height}")
    if mip_count > 1:
        levels = dimensions(width, height)
        if mip_count != len(levels):
            raise ValueError(
                f"{path}: partial mip chain {mip_count}, expected {len(levels)}"
            )
        flags = read_u32(source, 8)
        caps = read_u32(source, 108)
        if 0 == flags & DDSD_MIPMAPCOUNT or (
            caps & (DDSCAPS_COMPLEX | DDSCAPS_MIPMAP)
        ) != (DDSCAPS_COMPLEX | DDSCAPS_MIPMAP):
            raise ValueError(f"{path}: mip count and DDS header flags disagree")
        expected = DDS_HEADER_BYTES + sum(
            dxt1_payload_size(mip_width, mip_height)
            for mip_width, mip_height in levels
        )
        if len(source) != expected:
            raise ValueError(
                f"{path}: complete mip payload should be {expected} bytes, got {len(source)}"
            )
        return width, height, mip_count

    expected = DDS_HEADER_BYTES + dxt1_payload_size(width, height)
    if len(source) != expected:
        raise ValueError(
            f"{path}: expected one DXT1 base payload ({expected} bytes), got {len(source)}"
        )
    return width, height, mip_count


def encode_dxt1(image: Image.Image) -> bytes:
    output = io.BytesIO()
    image.save(output, format="DDS", pixel_format="DXT1")
    encoded = output.getvalue()
    if len(encoded) < DDS_HEADER_BYTES or encoded[84:88] != b"DXT1":
        raise ValueError("Pillow did not produce a legacy DXT1 DDS")
    return encoded[DDS_HEADER_BYTES:]


def build_mipped_dds(path: Path, source: bytes) -> tuple[bytes, int, str]:
    width, height, existing_mips = validate_source(path, source)
    if existing_mips > 1:
        return source, existing_mips, "already-mipped"

    with Image.open(io.BytesIO(source)) as opened:
        base_image = opened.convert("RGB")
        base_image.load()

    levels = dimensions(width, height)
    payloads = [source[DDS_HEADER_BYTES:]]
    for mip_width, mip_height in levels[1:]:
        resized = base_image.resize(
            (mip_width, mip_height),
            Image.Resampling.LANCZOS,
            reducing_gap=3.0,
        )
        payload = encode_dxt1(resized)
        expected = dxt1_payload_size(mip_width, mip_height)
        if len(payload) != expected:
            raise ValueError(
                f"{path}: {mip_width}x{mip_height} mip is {len(payload)} bytes, expected {expected}"
            )
        payloads.append(payload)

    header = bytearray(source[:DDS_HEADER_BYTES])
    write_u32(header, 8, read_u32(header, 8) | DDSD_MIPMAPCOUNT)
    write_u32(header, 28, len(levels))
    write_u32(
        header,
        108,
        read_u32(header, 108) | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP,
    )
    result = bytes(header) + b"".join(payloads)
    expected_total = DDS_HEADER_BYTES + sum(
        dxt1_payload_size(mip_width, mip_height)
        for mip_width, mip_height in levels
    )
    if len(result) != expected_total:
        raise ValueError(
            f"{path}: output is {len(result)} bytes, expected {expected_total}"
        )
    return result, len(levels), "generated"


def replace_file(path: Path, content: bytes) -> None:
    temporary = path.with_name(path.name + ".miptmp")
    try:
        temporary.write_bytes(content)
        with Image.open(temporary) as verification:
            verification.load()
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build a complete mip chain for matching legacy DXT1 DDS files."
    )
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--include-regex", required=True)
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()

    root = args.root.resolve()
    if not root.is_dir():
        parser.error(f"root does not exist: {root}")
    pattern = re.compile(args.include_regex, re.IGNORECASE)
    paths = sorted(
        path for path in root.rglob("*.dds")
        if pattern.search(path.name)
    )
    if not paths:
        parser.error("no matching DDS files")

    generated = 0
    skipped = 0
    for path in paths:
        source = path.read_bytes()
        width, height, _ = validate_source(path, source)
        base_bytes = dxt1_payload_size(width, height)
        base_hash = hashlib.sha256(
            source[DDS_HEADER_BYTES:DDS_HEADER_BYTES + base_bytes]
        ).hexdigest()
        result, mip_count, state = build_mipped_dds(path, source)
        if state == "already-mipped":
            skipped += 1
        else:
            generated += 1
            if args.write:
                replace_file(path, result)
                written = path.read_bytes()
                written_hash = hashlib.sha256(
                    written[DDS_HEADER_BYTES:DDS_HEADER_BYTES + base_bytes]
                ).hexdigest()
                if written_hash != base_hash or read_u32(written, 28) != mip_count:
                    raise ValueError(f"{path}: post-write verification failed")
        print(
            f"{state}|mips={mip_count}|base_sha256={base_hash}|"
            f"bytes={len(result)}|{path.relative_to(root)}"
        )

    mode = "write" if args.write else "dry-run"
    print(f"summary|mode={mode}|matched={len(paths)}|generated={generated}|skipped={skipped}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, re.error, struct.error) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
