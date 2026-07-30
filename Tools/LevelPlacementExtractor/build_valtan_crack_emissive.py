#!/usr/bin/env python3
"""Build a restrained green emissive mask from the Valtan crack diffuse atlas.

The source UE3 material has diffuse and normal slots but no authored emissive
slot.  The reference footage nevertheless shows green light leaking through
dark creases.  This tool keeps that reconstruction deterministic: it detects
pixels darker than their local neighbourhood, writes only those valleys into
an RGB emissive texture, and leaves the source diffuse untouched.

Only non-interlaced, 8-bit RGB/RGBA PNG input is accepted.  The implementation
uses the Python standard library so the map extraction pipeline has no Pillow
or ImageMagick dependency.
"""

from __future__ import annotations

import argparse
import binascii
import struct
import zlib
from pathlib import Path


PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def paeth(left: int, up: int, upper_left: int) -> int:
    prediction = left + up - upper_left
    left_distance = abs(prediction - left)
    up_distance = abs(prediction - up)
    upper_left_distance = abs(prediction - upper_left)
    if left_distance <= up_distance and left_distance <= upper_left_distance:
        return left
    if up_distance <= upper_left_distance:
        return up
    return upper_left


def read_png(path: Path) -> tuple[int, int, int, bytes]:
    data = path.read_bytes()
    if not data.startswith(PNG_SIGNATURE):
        raise ValueError(f"not a PNG: {path}")

    offset = len(PNG_SIGNATURE)
    width = height = color_type = bit_depth = interlace = -1
    compressed = bytearray()
    while offset < len(data):
        length = struct.unpack_from(">I", data, offset)[0]
        chunk_type = data[offset + 4 : offset + 8]
        payload = data[offset + 8 : offset + 8 + length]
        offset += 12 + length
        if chunk_type == b"IHDR":
            width, height, bit_depth, color_type, _, _, interlace = struct.unpack(
                ">IIBBBBB", payload
            )
        elif chunk_type == b"IDAT":
            compressed.extend(payload)
        elif chunk_type == b"IEND":
            break

    if bit_depth != 8 or color_type not in (2, 6) or interlace != 0:
        raise ValueError(
            "expected non-interlaced 8-bit RGB/RGBA PNG; "
            f"got bitDepth={bit_depth}, colorType={color_type}, interlace={interlace}"
        )

    channels = 3 if color_type == 2 else 4
    stride = width * channels
    raw = zlib.decompress(bytes(compressed))
    if len(raw) != height * (stride + 1):
        raise ValueError("unexpected PNG scanline size")

    decoded = bytearray(height * stride)
    source_offset = 0
    for y in range(height):
        filter_type = raw[source_offset]
        source_offset += 1
        row_offset = y * stride
        previous_offset = (y - 1) * stride
        for x in range(stride):
            value = raw[source_offset + x]
            left = decoded[row_offset + x - channels] if x >= channels else 0
            up = decoded[previous_offset + x] if y > 0 else 0
            upper_left = (
                decoded[previous_offset + x - channels]
                if y > 0 and x >= channels
                else 0
            )
            if filter_type == 1:
                value += left
            elif filter_type == 2:
                value += up
            elif filter_type == 3:
                value += (left + up) // 2
            elif filter_type == 4:
                value += paeth(left, up, upper_left)
            elif filter_type != 0:
                raise ValueError(f"unsupported PNG filter: {filter_type}")
            decoded[row_offset + x] = value & 0xFF
        source_offset += stride
    return width, height, channels, bytes(decoded)


def box_blur(values: list[int], width: int, height: int, radius: int) -> list[int]:
    diameter = radius * 2 + 1
    horizontal = [0] * len(values)
    for y in range(height):
        row = y * width
        running = values[row] * radius
        for x in range(radius + 1):
            running += values[row + min(width - 1, x)]
        for x in range(width):
            horizontal[row + x] = running
            running += values[row + min(width - 1, x + radius + 1)]
            running -= values[row + max(0, x - radius)]

    blurred = [0] * len(values)
    for x in range(width):
        running = horizontal[x] * radius
        for y in range(radius + 1):
            running += horizontal[min(height - 1, y) * width + x]
        for y in range(height):
            blurred[y * width + x] = running // (diameter * diameter)
            running += horizontal[min(height - 1, y + radius + 1) * width + x]
            running -= horizontal[max(0, y - radius) * width + x]
    return blurred


def make_emissive(
    pixels: bytes,
    width: int,
    height: int,
    channels: int,
    radius: int,
    threshold: int,
    softness: int,
    color: tuple[int, int, int],
) -> bytes:
    luminance = [0] * (width * height)
    for index in range(width * height):
        source = index * channels
        red, green, blue = pixels[source : source + 3]
        luminance[index] = (54 * red + 183 * green + 19 * blue) >> 8

    local_average = box_blur(luminance, width, height, radius)
    output = bytearray(width * height * 3)
    for index, value in enumerate(luminance):
        contrast = local_average[index] - value - threshold
        mask = 0 if contrast <= 0 else min(255, contrast * 255 // softness)
        target = index * 3
        output[target] = color[0] * mask // 255
        output[target + 1] = color[1] * mask // 255
        output[target + 2] = color[2] * mask // 255
    return bytes(output)


def png_chunk(chunk_type: bytes, payload: bytes) -> bytes:
    checksum = binascii.crc32(chunk_type)
    checksum = binascii.crc32(payload, checksum) & 0xFFFFFFFF
    return struct.pack(">I", len(payload)) + chunk_type + payload + struct.pack(">I", checksum)


def write_rgb_png(path: Path, width: int, height: int, pixels: bytes) -> None:
    stride = width * 3
    scanlines = b"".join(
        b"\x00" + pixels[y * stride : (y + 1) * stride] for y in range(height)
    )
    header = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    encoded = (
        PNG_SIGNATURE
        + png_chunk(b"IHDR", header)
        + png_chunk(b"IDAT", zlib.compress(scanlines, 9))
        + png_chunk(b"IEND", b"")
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_bytes(encoded)
    temporary.replace(path)


def parse_color(text: str) -> tuple[int, int, int]:
    values = tuple(int(part) for part in text.split(","))
    if len(values) != 3 or any(value < 0 or value > 255 for value in values):
        raise argparse.ArgumentTypeError("color must be R,G,B with values from 0 to 255")
    return values


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--radius", type=int, default=4)
    parser.add_argument("--threshold", type=int, default=6)
    parser.add_argument("--softness", type=int, default=22)
    parser.add_argument("--color", type=parse_color, default=(12, 180, 105))
    args = parser.parse_args()
    if args.radius < 1 or args.softness < 1:
        parser.error("radius and softness must be positive")

    width, height, channels, pixels = read_png(args.input)
    emissive = make_emissive(
        pixels,
        width,
        height,
        channels,
        args.radius,
        args.threshold,
        args.softness,
        args.color,
    )
    write_rgb_png(args.output, width, height, emissive)
    print(f"wrote {args.output} ({width}x{height}, RGB8)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
