"""Split a cooked submesh's constant-extra-UV tail into a submesh of its own.

Why this exists: the cook merges source primitives that share a material, so a piece that
is not what the material is for can end up inside another primitive's submesh.  Measured on
the LanceMaster body, ``pc_ft_eye_mi`` holds 265 vertices: 157 are the eyeballs and the
trailing 108 are a head-wide cap rigged to ``bip001-head`` plus the ``b_hair_fl/fr_01``
bangs, spanning x -5.19..6.45 and z -126.73..-120.08 while the eyeballs sit at x 4.33..5.62.
That cap carries a single extra-UV value, (3e-05, 3e-05), where the eyeballs carry 75
distinct ones, because that is what the source section has.  Source-character program 5
reads those channels as the eye base and iris coordinates, so the cap samples one texel and
draws as a flat eye-coloured shell across the head.

Hiding it is not possible while it shares a submesh, because ``CHARACTER_SPEC``'s hidden-mesh
masks are per submesh and the eyeballs would go with it.  This splits the tail into its own
submesh so an existing mask bit can take it, and keeps every vertex, index, weight and UV.

The split is refused unless the file says it is clean:

* the constant-extra-UV vertices form one contiguous run at the end of the submesh
* their triangles form one contiguous run at the end of its index range
* no triangle mixes the two groups

The new submesh is inserted directly after the one it came from, not appended, because
``parse_skinned_uv_wmodel`` requires descriptor order to match blob order.  Submeshes after
it shift up by one, so the owning class's mask bits must shift with them -- for LanceMaster
``BAKED_HAIR`` moves from 1<<6 to 1<<7.

usage:
  python split_constant_uv_submesh_tail.py --wmodel <body.wmodel> --material pc_ft_eye_mi
      [--dry-run]
"""
from __future__ import annotations

import argparse
import math
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cook_wmodel_geometry_contract as _c  # noqa: E402

MATERIAL_META = struct.Struct("<4sI")
MATERIAL_ENTRY = {
    b"WMAT": struct.Struct("<IQ64s520s"),
    b"WMA2": struct.Struct("<IQ64s" + "520s" * 9),
    b"WMA3": struct.Struct("<IQ64s" + "520s" * 10 + "16f"),
}
CONSTANT_UV_EPSILON = 1e-4


def material_names(sections):
    for section in sections:
        if section.type_id != 2:
            continue
        payload = section.payload[_c.FILE_HEADER.size:]
        magic, count = MATERIAL_META.unpack_from(payload, 0)
        entry = MATERIAL_ENTRY[magic]
        at = MATERIAL_META.size
        return {entry.unpack_from(payload, at + row * entry.size)[0]:
                entry.unpack_from(payload, at + row * entry.size)[2].split(b"\0")[0]
                .decode("ascii", "replace").lower() for row in range(count)}
    raise SystemExit("no material section")


def bounds_row(positions):
    """min(3), max(3), centre(3), radius -- the layout cook_wmodel_geometry_contract checks."""
    minimum = tuple(min(p[axis] for p in positions) for axis in range(3))
    maximum = tuple(max(p[axis] for p in positions) for axis in range(3))
    centre = tuple(0.5 * minimum[axis] + 0.5 * maximum[axis] for axis in range(3))
    radius = max(math.sqrt(sum((p[axis] - centre[axis]) ** 2 for axis in range(3)))
                 for p in positions)
    return tuple(minimum) + tuple(maximum) + tuple(centre) + (radius,)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wmodel", type=Path, required=True)
    parser.add_argument("--material", required=True)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    original = args.wmodel.read_bytes()
    parsed = _c.parse_skinned_uv_wmodel(original)
    mesh = parsed["mesh"]
    header = parsed["meshHeader"]
    _magic, subcount, bones, vertex_flags, stride, vcount, icount, istride, has_bounds, reserved = header
    submeshes = parsed["submeshes"]
    uv_rows = parsed["uvRows"]
    names = material_names(parsed["sections"])

    target = [i for i, desc in enumerate(submeshes)
              if names.get(desc[4], "") == args.material.lower()]
    if len(target) != 1:
        raise SystemExit("expected exactly one %r submesh, found %d" % (args.material, len(target)))
    target = target[0]
    desc = submeshes[target]
    count = desc[1]

    row = uv_rows[target]
    if "TEXCOORD_2" not in row:
        raise SystemExit("submesh %d has no TEXCOORD_2 to judge by" % target)
    uv2 = row["TEXCOORD_2"]
    last = uv2[-1]
    split = count
    while split > 0 and all(abs(uv2[split - 1][axis] - last[axis]) <= CONSTANT_UV_EPSILON
                            for axis in range(2)):
        split -= 1
    tail = count - split
    if tail == 0 or split == 0:
        raise SystemExit("submesh %d has no constant-UV tail to split (tail=%d of %d)"
                         % (target, tail, count))
    if any(all(abs(uv2[v][axis] - last[axis]) <= CONSTANT_UV_EPSILON for axis in range(2))
           for v in range(split)):
        raise SystemExit("submesh %d carries the constant UV outside its tail run" % target)

    index_fmt = "<H" if istride == 2 else "<I"
    index_base = parsed["indexStart"] + desc[2]
    triangles = desc[3] // 3
    first_tail_triangle = None
    for triangle in range(triangles):
        vertices = [struct.unpack_from(index_fmt, mesh, index_base + (triangle * 3 + k) * istride)[0]
                    for k in range(3)]
        inside = sum(1 for v in vertices if v >= split)
        if inside == 3:
            if first_tail_triangle is None:
                first_tail_triangle = triangle
        elif inside:
            raise SystemExit("triangle %d of submesh %d mixes the two groups" % (triangle, target))
        elif first_tail_triangle is not None:
            raise SystemExit("submesh %d tail triangles are not one contiguous run" % target)
    if first_tail_triangle is None:
        raise SystemExit("submesh %d has no triangle made only of tail vertices" % target)
    tail_triangles = triangles - first_tail_triangle
    head_indices = first_tail_triangle * 3
    tail_indices = tail_triangles * 3

    print("%s: %r submesh %d, %d verts / %d triangles" %
          (args.wmodel.name, args.material, target, count, triangles))
    print("   constant extra UV %s on the trailing %d verts and %d triangles"
          % (tuple(round(v, 6) for v in last), tail, tail_triangles))

    vertex_start = parsed["vertexStart"]
    positions = [struct.unpack_from("<3f", mesh, vertex_start + (desc[0] // stride + v) * stride)
                 for v in range(count)]

    descriptors = []
    for index, entry in enumerate(submeshes):
        if index != target:
            descriptors.append(entry)
            continue
        descriptors.append((entry[0], split, entry[2], head_indices, entry[4], entry[5], entry[6]))
        descriptors.append((entry[0] + split * stride, tail,
                            entry[2] + head_indices * istride, tail_indices,
                            entry[4], entry[5], entry[6]))

    indices = bytearray(mesh[parsed["indexStart"]:parsed["indexStart"] + icount * istride])
    rebase_at = desc[2] + head_indices * istride
    for slot in range(tail_indices):
        at = rebase_at + slot * istride
        value = struct.unpack_from(index_fmt, indices, at)[0]
        struct.pack_into(index_fmt, indices, at, value - split)

    bounds_start = parsed["indexStart"] + icount * istride + bones * _c.MESH_BONE_SIZE
    bounds = []
    for index in range(subcount):
        if index != target:
            bounds.append(_c.BOUNDS_V1.unpack_from(mesh, bounds_start + index * _c.BOUNDS_V1.size))
            continue
        bounds.append(bounds_row(positions[:split]))
        bounds.append(bounds_row(positions[split:]))

    blocks = []
    aggregate = 0
    for index, entry in enumerate(descriptors):
        source = uv_rows[index if index <= target else index - 1]
        if index == target:
            source = {k: v[:split] for k, v in uv_rows[target].items()}
        elif index == target + 1:
            source = {k: v[split:] for k, v in uv_rows[target].items()}
        mask = ((_c.VF_TEXCOORD1 if "TEXCOORD_1" in source else 0)
                | (_c.VF_TEXCOORD2 if "TEXCOORD_2" in source else 0))
        aggregate |= mask
        blocks.append(struct.pack("<II", entry[1], mask))
        for name in ("TEXCOORD_1", "TEXCOORD_2"):
            if name in source:
                blocks.append(b"".join(struct.pack("<2f", *uv) for uv in source[name]))
    uv_payload = b"".join(blocks)

    content = bytearray()
    content += _c.MESH_HEADER.pack(b"WMSH", subcount + 1, bones, vertex_flags, stride,
                                   vcount, icount, istride, has_bounds, reserved)
    content += b"".join(_c.SUBMESH_DESC.pack(*entry) for entry in descriptors)
    content += mesh[vertex_start:vertex_start + vcount * stride]
    content += bytes(indices)
    content += mesh[parsed["indexStart"] + icount * istride:bounds_start]
    content += b"".join(_c.BOUNDS_V1.pack(*entry) for entry in bounds)
    content += _c.SKINNED_UV_HEADER.pack(b"WUVS", 1, len(descriptors), len(uv_payload),
                                         _c.sha256_bytes(uv_payload)) + uv_payload
    new_mesh = _c.FILE_HEADER.pack(b"WINT", 1, parsed["versionMinor"], 0, len(content)) + bytes(content)
    result = _c.rebuild_wmodel(parsed["modelHeader"], parsed["sections"], new_mesh)

    readback = _c.parse_skinned_uv_wmodel(result)
    if len(readback["submeshes"]) != subcount + 1:
        raise SystemExit("readback submesh count is wrong")
    if (readback["mesh"][readback["vertexStart"]:readback["vertexStart"] + vcount * stride]
            != mesh[vertex_start:vertex_start + vcount * stride]):
        raise SystemExit("the split changed the vertex stream")
    for old, new in zip(parsed["sections"], readback["sections"]):
        if old.type_id != 1 and old.payload != new.payload:
            raise SystemExit("the split changed a non-mesh section")
    print("   -> submesh %d keeps %d verts / %d triangles, new submesh %d takes %d / %d"
          % (target, split, first_tail_triangle, target + 1, tail, tail_triangles))
    print("   submeshes after it shift up by one; move the owning class's mask bits with them")

    if args.dry_run:
        print("   dry run, nothing written")
        return 0
    args.wmodel.write_bytes(result)
    print("   written -> %s" % args.wmodel)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
