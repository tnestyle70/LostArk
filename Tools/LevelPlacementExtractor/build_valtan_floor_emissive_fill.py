"""Fill the Valtan arena emissive mask over the stone joint UV footprint.

The authored reconstruction mask draws thin crack lines over the loose rubble
that lies on the arena, so the teal glow lands on scattered debris instead of
the joints between the paving slabs where it belongs.

The joints are geometry, not texture: submesh 0 models each slab with real side
walls, and those walls carry no emissive of their own. Two halves answer that.
The deferred emissive overlay pass draws both submeshes and keeps only surfaces
that turn away from up, so slab tops stay dark and joint walls light. This tool
answers the other half, coverage, by flooding the joint UV footprint with the
mask's own brightest authored colour so every joint wall has something to emit.

Face direction, not the texture, decides what lights, so it does not matter that
the slab tops share UV space with their own walls.

The operation is idempotent: the fill colour is the brightest texel of the input,
and an already-filled mask reports the same brightest texel.

Usage:
    python build_valtan_floor_emissive_fill.py            # write the masks
    python build_valtan_floor_emissive_fill.py --check    # report only
"""

from __future__ import annotations

import argparse
import math
import struct
import sys
import zlib
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
RESOURCE_ROOT = REPO_ROOT / "Client" / "Bin" / "Resources"
MODEL_NAMES = ("BG_RAD_VALTAN_FLOOR01A_SM", "BG_RAD_VALTAN_FLOOR01B_SM")
MASK_NAME = "bg_rad_valtan_crack_floor01_em_reconstruction.png"

# Submesh 0 is the arena stone, submesh 1 the loose crack rubble lying on it.
# The seams the glow belongs in are the stone's own joints, so the fill follows
# submesh 0. The rubble keeps whatever the authored mask already gave it.
ARENA_STONE_SUBMESH_INDEX = 0
CRACK_RUBBLE_SUBMESH_INDEX = 1
FILL_SUBMESH_INDEX = ARENA_STONE_SUBMESH_INDEX
# Same split the overlay shader uses: a face is a plate top when its geometric
# normal points up. Everything else is the inside of the gap.
UP_FACING_NORMAL_Y = 0.7
# Bilinear taps reach one texel past a triangle edge, so grow the footprint by
# two texels to keep the seam from sampling black.
FOOTPRINT_DILATE_TEXELS = 2


class BuildError(RuntimeError):
    pass


def read_mesh(path: Path):
    data = path.read_bytes()
    base = 16
    magic, submesh_count, _bones, _flags, stride, vertex_total, index_total, index_stride, _bounds = (
        struct.unpack_from("<4sIIIIIIIB", data, base)
    )
    if magic != b"WMSH":
        raise BuildError(f"{path.name} is not a WMSH mesh")
    offset = base + 36
    submeshes = []
    for _ in range(submesh_count):
        vertex_offset, vertex_count, idx_offset, idx_count, material_index, _hash = (
            struct.unpack_from("<IIIIIQ", data, offset)
        )
        offset += 48
        submeshes.append(
            dict(
                vertex_offset=vertex_offset,
                vertex_count=vertex_count,
                index_offset=idx_offset,
                index_count=idx_count,
                material_index=material_index,
            )
        )
    vertex_base = offset
    index_base = vertex_base + vertex_total * stride
    return data, submeshes, vertex_base, stride, index_base, index_stride


def gap_interior_triangles(mesh_path: Path, submesh_index: int):
    data, submeshes, vertex_base, stride, index_base, index_stride = read_mesh(mesh_path)
    if len(submeshes) <= submesh_index:
        raise BuildError(f"{mesh_path.name} has no submesh {submesh_index}")
    submesh = submeshes[submesh_index]
    if index_stride != 2:
        raise BuildError(f"{mesh_path.name} uses an unsupported index stride")

    def vertex(local_index: int):
        at = vertex_base + submesh["vertex_offset"] + local_index * stride
        position = struct.unpack_from("<3f", data, at)
        texcoord = struct.unpack_from("<2f", data, at + 24)
        return position, texcoord

    gap = []
    top = []
    for triangle in range(submesh["index_count"] // 3):
        at = index_base + submesh["index_offset"] + triangle * 3 * index_stride
        i0, i1, i2 = struct.unpack_from("<3H", data, at)
        (a, ta), (b, tb), (c, tc) = vertex(i0), vertex(i1), vertex(i2)
        ux, uy, uz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
        vx, vy, vz = c[0] - a[0], c[1] - a[1], c[2] - a[2]
        nx = uy * vz - uz * vy
        ny = uz * vx - ux * vz
        nz = ux * vy - uy * vx
        length = math.sqrt(nx * nx + ny * ny + nz * nz)
        if length <= 1e-12:
            continue
        if ny / length > UP_FACING_NORMAL_Y:
            top.append((ta, tb, tc))
        else:
            gap.append((ta, tb, tc))
    return gap, top


def rasterize(triangles, size: int) -> bytearray:
    coverage = bytearray(size * size)
    for (u0, v0), (u1, v1), (u2, v2) in triangles:
        xs = [(u0 % 1.0) * size, (u1 % 1.0) * size, (u2 % 1.0) * size]
        ys = [(v0 % 1.0) * size, (v1 % 1.0) * size, (v2 % 1.0) * size]
        # A triangle whose wrapped corners straddle the atlas is a seam artefact,
        # not a real face; filling its bounding box would flood the whole mask.
        if max(xs) - min(xs) > size * 0.5 or max(ys) - min(ys) > size * 0.5:
            continue
        x0 = max(0, int(min(xs)) - 1)
        x1 = min(size - 1, int(max(xs)) + 1)
        y0 = max(0, int(min(ys)) - 1)
        y1 = min(size - 1, int(max(ys)) + 1)
        denominator = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2])
        if abs(denominator) < 1e-9:
            continue
        for py in range(y0, y1 + 1):
            cy = py + 0.5
            row = py * size
            for px in range(x0, x1 + 1):
                cx = px + 0.5
                w0 = ((ys[1] - ys[2]) * (cx - xs[2]) + (xs[2] - xs[1]) * (cy - ys[2])) / denominator
                w1 = ((ys[2] - ys[0]) * (cx - xs[2]) + (xs[0] - xs[2]) * (cy - ys[2])) / denominator
                w2 = 1.0 - w0 - w1
                if w0 >= -0.002 and w1 >= -0.002 and w2 >= -0.002:
                    coverage[row + px] = 1
    return coverage


def dilate(coverage: bytearray, size: int, radius: int) -> bytearray:
    result = coverage
    for _ in range(radius):
        grown = bytearray(result)
        for y in range(size):
            row = y * size
            up = row - size if y > 0 else None
            down = row + size if y + 1 < size else None
            for x in range(size):
                if result[row + x]:
                    continue
                if x > 0 and result[row + x - 1]:
                    grown[row + x] = 1
                elif x + 1 < size and result[row + x + 1]:
                    grown[row + x] = 1
                elif up is not None and result[up + x]:
                    grown[row + x] = 1
                elif down is not None and result[down + x]:
                    grown[row + x] = 1
        result = grown
    return result


def read_png(path: Path):
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise BuildError(f"{path.name} is not a PNG")
    offset = 8
    idat = bytearray()
    width = height = bit_depth = color_type = interlace = 0
    while offset < len(data):
        length = struct.unpack_from(">I", data, offset)[0]
        chunk = data[offset + 4 : offset + 8]
        if chunk == b"IHDR":
            width, height, bit_depth, color_type, _c, _f, interlace = struct.unpack_from(
                ">IIBBBBB", data, offset + 8
            )
        elif chunk == b"IDAT":
            idat += data[offset + 8 : offset + 8 + length]
        offset += 12 + length
    if bit_depth != 8 or color_type != 2 or interlace != 0:
        raise BuildError(f"{path.name} must be 8-bit non-interlaced RGB")
    raw = zlib.decompress(bytes(idat))
    stride = width * 3
    out = bytearray()
    previous = bytearray(stride)
    at = 0
    for _ in range(height):
        filter_type = raw[at]
        at += 1
        line = bytearray(raw[at : at + stride])
        at += stride
        if filter_type == 1:
            for x in range(3, stride):
                line[x] = (line[x] + line[x - 3]) & 255
        elif filter_type == 2:
            for x in range(stride):
                line[x] = (line[x] + previous[x]) & 255
        elif filter_type == 3:
            for x in range(stride):
                left = line[x - 3] if x >= 3 else 0
                line[x] = (line[x] + ((left + previous[x]) >> 1)) & 255
        elif filter_type == 4:
            for x in range(stride):
                left = line[x - 3] if x >= 3 else 0
                upper = previous[x]
                upper_left = previous[x - 3] if x >= 3 else 0
                predictor = left + upper - upper_left
                da, db, dc = (
                    abs(predictor - left),
                    abs(predictor - upper),
                    abs(predictor - upper_left),
                )
                if da <= db and da <= dc:
                    chosen = left
                elif db <= dc:
                    chosen = upper
                else:
                    chosen = upper_left
                line[x] = (line[x] + chosen) & 255
        elif filter_type != 0:
            raise BuildError(f"{path.name} uses an unknown PNG filter")
        out += line
        previous = line
    return width, height, out


def write_png(path: Path, width: int, height: int, pixels: bytes) -> None:
    raw = bytearray()
    stride = width * 3
    for y in range(height):
        raw.append(0)
        raw += pixels[y * stride : (y + 1) * stride]

    def chunk(tag: bytes, payload: bytes) -> bytes:
        body = tag + payload
        return struct.pack(">I", len(payload)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    header = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    blob = (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", header)
        + chunk(b"IDAT", zlib.compress(bytes(raw), 9))
        + chunk(b"IEND", b"")
    )
    path.write_bytes(blob)


def brightest_texel(pixels: bytes) -> tuple[int, int, int]:
    best = (0, 0, 0)
    best_level = -1
    for at in range(0, len(pixels), 3):
        r, g, b = pixels[at], pixels[at + 1], pixels[at + 2]
        level = max(r, g, b)
        if level > best_level:
            best_level = level
            best = (r, g, b)
    return best


def process(model_name: str, write: bool) -> None:
    model_root = RESOURCE_ROOT / "Map" / "BG_RAD_VALTAN_A" / model_name
    mesh_path = model_root / f"{model_name}.wmesh"
    mask_path = model_root / "textures" / MASK_NAME
    if not mesh_path.is_file() or not mask_path.is_file():
        raise BuildError(f"{model_name} is missing its mesh or emissive mask")

    gap, top = gap_interior_triangles(mesh_path, FILL_SUBMESH_INDEX)
    width, height, pixels = read_png(mask_path)
    if width != height:
        raise BuildError(f"{mask_path.name} must be square")

    footprint = dilate(rasterize(gap, width), width, FOOTPRINT_DILATE_TEXELS)
    fill = brightest_texel(pixels)
    covered = sum(footprint)

    filled = bytearray(pixels)
    changed = 0
    for index in range(width * height):
        if not footprint[index]:
            continue
        at = index * 3
        before = (filled[at], filled[at + 1], filled[at + 2])
        after = (max(before[0], fill[0]), max(before[1], fill[1]), max(before[2], fill[2]))
        if after != before:
            changed += 1
        filled[at], filled[at + 1], filled[at + 2] = after

    lit_before = sum(1 for at in range(0, len(pixels), 3) if max(pixels[at : at + 3]) > 0)
    lit_after = sum(1 for at in range(0, len(filled), 3) if max(filled[at : at + 3]) > 0)
    total = width * height
    report = [
        f"{model_name}:",
        f"  seam triangles {len(gap)}  slab-top triangles {len(top)}",
        f"  fill colour sRGB {fill}",
        f"  seam footprint {covered} texels ({covered / total * 100:.2f}%)",
        f"  lit texels {lit_before} ({lit_before / total * 100:.2f}%)"
        f" -> {lit_after} ({lit_after / total * 100:.2f}%)",
        f"  texels raised {changed}",
    ]
    sys.stdout.buffer.write(("\n".join(report) + "\n").encode("utf-8"))

    if write:
        write_png(mask_path, width, height, bytes(filled))
        sys.stdout.buffer.write(f"  wrote {mask_path}\n".encode("utf-8"))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="report without writing")
    arguments = parser.parse_args()
    try:
        for model_name in MODEL_NAMES:
            process(model_name, write=not arguments.check)
    except BuildError as error:
        sys.stdout.buffer.write(f"build_valtan_floor_emissive_fill: {error}\n".encode("utf-8"))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
