"""Give a cooked body's face vertices the facial rig weights the retail face mesh carries.

Why this exists: the character-creation face sliders rotate `b_fc_*` bones, and the face only
follows if its vertices are weighted to them. Two of the four bodies are not -- measured on the
cooked models, the face region is skinned to:

    LanceMaster  42 bones, 41 of them b_fc_*      DimensionMaster  41 bones, 39 b_fc_*
    Warlord       2 bones,  0 of them b_fc_*      Artist            2 bones,  0 b_fc_*

and the same holds in the retail bodies, so the cook did not drop it. For those two races the
rigged head is the *separate* face SkeletalMesh (`pc_wr_00_face_sk`, `pc_sp_00_face_sk`), which
this project does not load; the body carries a plain head bound to `bip001-head`.

Rather than load a second head, this copies the weights across. The correspondence already
exists: `<Class>.facemorphmap` was built by matching every face-mesh wedge to the runtime
vertices it became, and that is exactly the pairing needed here.

## Resolving a bone

This is where the first attempt went wrong, and it is the whole of the difficulty. A retail
vertex stores a *chunk-local* bone index; the chunk's `BoneMap` turns that into an index into
the retail `RefSkeleton`. That skeleton index is **not** an index into the cooked model's bone
palette -- the two orders are similar enough that 40 of 42 entries still land on some `b_fc_*`
bone, which is exactly why the mistake survived review, but not the *right* facial bone. The
result was nose weights driving the temple, and a head deformed into a cone.

So every index is resolved through the bone's **name**, and a name that is not present in the
cooked palette is a hard error rather than a skipped influence. Measured: all 42 (FT, WR) and
40 (SP) names exist in each of the three cooked palettes.

## What is read where

* retail `RefSkeleton` -- `[count][FMeshBone x 52]`; within a record the FName index is at +0,
  NumChildren at +40, ParentIndex at +44.
* retail `BoneMap` -- the last `[count][uint16 x count]` before the GPU vertex buffer whose
  entries are unique, in range, and numerous enough to cover every index the weights use.
  Verified: 42/42/40 entries, 40/40/38 of them `b_fc_*`, one chunk per mesh (the highest bone
  index any vertex uses is 41/41/39).
* retail GPU vertex, stride 40 -- packed normals at 0, bone indices at +8, bone weights at +12
  (bytes, summing to 255 on every vertex: 1884/1362/2899 of them), position at +16.
* cooked VTXANIMMESH, stride 76 -- position 0, blend indices 44 (4x uint32), weights 60 (4x float).

usage:
  python transplant_face_skin_weights.py --class-id Warlord --upk <face.upk>
      --object-name pc_wr_00_face_sk --expect-verts 1362 [--dry-run]
"""
from __future__ import annotations

import argparse
import collections
import itertools
import math
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build_face_morphs as _bfm  # noqa: E402
import build_face_morph_vertex_map as _map  # noqa: E402
import read_skeletalmesh_vertices as _rv  # noqa: E402

BONE_ENTRY = struct.Struct("<Q32si16fI16s")
MESHBONE_SIZE = 52
RETAIL_INDEX_OFFSET = 8
RETAIL_WEIGHT_OFFSET = 12
RETAIL_POSITION_OFFSET = 16
COOKED_INDEX_OFFSET = 44
COOKED_WEIGHT_OFFSET = 60


def read_retail_face(upk: Path, object_name: str, expect_verts: int):
    """Return (skeletonNames, boneMap, influences, positions) for the retail face mesh."""
    buffer, info = _bfm.decompress_package(upk)
    names = _bfm.read_names(buffer, info)
    export = _rv._find_export(buffer, names, _bfm.read_exports(buffer, info, names), object_name)
    start, end = export["offset"], export["offset"] + export["size"]
    vertex_base, stride = _rv._find_vertex_buffer(buffer, start, end, expect_verts)[0]

    skeleton = None
    for at in range(start, start + 4000):
        count, = struct.unpack_from("<i", buffer, at)
        if not (40 <= count <= 400) or at + 4 + count * MESHBONE_SIZE > vertex_base:
            continue
        bones, ok = [], True
        for index in range(count):
            record = at + 4 + index * MESHBONE_SIZE
            name_index, = struct.unpack_from("<i", buffer, record)
            parent, = struct.unpack_from("<i", buffer, record + 44)
            if not (0 <= name_index < len(names)) or not (0 <= parent < count):
                ok = False
                break
            bones.append(names[name_index].lower())
        if ok and "b_root" in bones:
            skeleton = bones
            break
    if skeleton is None:
        raise SystemExit("%s: RefSkeleton not found" % object_name)

    influences, positions, used = [], [], collections.Counter()
    for i in range(expect_verts):
        at = vertex_base + i * stride
        indices = struct.unpack_from("<4B", buffer, at + RETAIL_INDEX_OFFSET)
        weights = struct.unpack_from("<4B", buffer, at + RETAIL_WEIGHT_OFFSET)
        if sum(weights) != 255:
            raise SystemExit("%s: vertex %d weights sum to %d, not 255"
                             % (object_name, i, sum(weights)))
        influences.append((indices, weights))
        positions.append(struct.unpack_from("<3f", buffer, at + RETAIL_POSITION_OFFSET))
        for k in range(4):
            if weights[k]:
                used[indices[k]] += 1

    needed = max(used) + 1
    bone_map = None
    for at in range(start, vertex_base - 4):
        count, = struct.unpack_from("<i", buffer, at)
        if not (needed <= count <= 200) or at + 4 + count * 2 > vertex_base:
            continue
        values = struct.unpack_from("<%dH" % count, buffer, at + 4)
        if len(set(values)) == count and all(v < len(skeleton) for v in values):
            bone_map = values           # the last match is the one next to the vertex buffer
    if bone_map is None:
        raise SystemExit("%s: BoneMap not found (needs >= %d entries)" % (object_name, needed))
    return skeleton, bone_map, influences, positions


def read_cooked(path: Path):
    """Return (bytes, vertexBase, stride, submeshes, boneNames) for the model's mesh."""
    data = bytearray(path.read_bytes())
    base = _map.WMODEL_FILE_HEADER.size
    _magic, section_count, _anim, _flags = _map.WMODEL_MODEL_HEADER.unpack_from(data, base)[:4]
    for index in range(section_count):
        section_type, _i, offset, _size, _name = _map.WMODEL_SECTION_DESC.unpack_from(
            data, base + _map.WMODEL_MODEL_HEADER.size + index * _map.WMODEL_SECTION_DESC.size)
        if section_type != _map.WMODEL_SECTION_MESH:
            continue
        start = next(c for c in (base + offset, offset) if data[c:c + 4] == b"WINT")
        payload = start + _map.WMODEL_FILE_HEADER.size
        header = _map.WMODEL_MESH_HEADER.unpack_from(data, payload)
        magic, submesh_count, bone_count, _vflags, stride, vertex_count, index_count, index_stride = header[:8]
        if magic != b"WMSH":
            raise SystemExit("%s: expected WMSH" % path)
        submesh_base = payload + _map.WMODEL_MESH_HEADER.size
        submeshes = []
        for i in range(submesh_count):
            fields = _map.WMODEL_SUBMESH_DESC.unpack_from(
                data, submesh_base + i * _map.WMODEL_SUBMESH_DESC.size)
            if fields[0] % stride:
                raise SystemExit("%s: submesh %d vertexOffset is not a multiple of stride" % (path, i))
            submeshes.append((fields[0] // stride, fields[1]))
        vertex_base = submesh_base + submesh_count * _map.WMODEL_SUBMESH_DESC.size
        bone_base = vertex_base + vertex_count * stride + index_count * index_stride
        bones = [BONE_ENTRY.unpack_from(data, bone_base + b * BONE_ENTRY.size)[1]
                 .split(b"\0")[0].decode("ascii", "replace").lower()
                 for b in range(bone_count)]
        return data, vertex_base, stride, submeshes, bones
    raise SystemExit("%s: no mesh section" % path)


def read_map(path: Path):
    data = path.read_bytes()
    magic, version, wedge_count, mesh_count = struct.unpack_from("<8sIII", data, 0)
    if magic != b"LAFMVMAP" or version != 2:
        raise SystemExit("%s: not a v2 .facemorphmap" % path)
    at, targets = 20 + mesh_count * 4, []
    for _ in range(wedge_count):
        count, = struct.unpack_from("<I", data, at)
        at += 4
        targets.append([struct.unpack_from("<2I", data, at + i * 8) for i in range(count)])
        at += count * 8
    if at != len(data):
        raise SystemExit("%s: trailing bytes" % path)
    return targets


def transform(point, permutation, signs):
    return (point[permutation[0]] * signs[0],
            point[permutation[1]] * signs[1],
            point[permutation[2]] * signs[2])


def find_axis_transform(positions, cooked_positions, pairs):
    """Work out which axis convention this class' cook used, from the map's own pairings.

    It is not the same for every class -- measured over each map's whole pairing list, the
    residual is zero for LanceMaster and Warlord at (x, -y, -z) but for Artist at (x, y, -z),
    and `build_face_morph_vertex_map.py` brute-forces it per class for the same reason. A
    hardcoded convention silently mismatches the odd one out: it made every Artist pairing look
    3.28 units apart, which read exactly like a corrupt map.

    Every candidate is scored against the pairs the map already commits to, so the winner is
    the transform the map was built with rather than a guess, and a class whose map does not
    line up under any of them fails here instead of being written wrong."""
    best = []
    for permutation in itertools.permutations(range(3)):
        # Only the 24 proper rotations are candidates -- a reflection would put b_fc_l_*
        # weights on the model's right. See build_face_morph_vertex_map.find_axis_transform.
        parity = 1 if ((permutation[1] - permutation[0]) * (permutation[2] - permutation[0])
                       * (permutation[2] - permutation[1])) > 0 else -1
        for signs in itertools.product((1, -1), repeat=3):
            if parity * signs[0] * signs[1] * signs[2] < 0:
                continue
            residuals = sorted(math.dist(cooked, transform(positions[wedge], permutation, signs))
                               for (wedge, _at, _m), cooked in zip(pairs, cooked_positions))
            best.append((residuals[len(residuals) // 2], permutation, signs))
    best.sort(key=lambda entry: entry[0])
    median, permutation, signs = best[0]
    if median > 1e-3:
        raise SystemExit("no axis/sign combination lines the map up with the model "
                         "(best median residual %.4f)" % median)
    return permutation, signs


def main() -> int:
    repo = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--class-id", required=True)
    parser.add_argument("--upk", type=Path, required=True)
    parser.add_argument("--object-name", required=True)
    parser.add_argument("--expect-verts", type=int, required=True)
    parser.add_argument("--wmodel", type=Path)
    parser.add_argument("--resources", type=Path, default=repo / "Client" / "Bin" / "Resources")
    # The map matched wedges to runtime vertices by position, so the pairing can be re-checked
    # here. A target further than this from its wedge is a stray match onto neighbouring body
    # geometry -- writing facial weights there would make a patch of the body follow the jaw.
    parser.add_argument("--max-distance", type=float, default=0.05)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    class_dir = args.resources / "Character" / args.class_id
    model_path = args.wmodel or (class_dir / (args.class_id + ".wmodel"))
    data, vertex_base, stride, submeshes, cooked_bones = read_cooked(model_path)
    skeleton, bone_map, influences, positions = read_retail_face(
        args.upk, args.object_name, args.expect_verts)
    targets = read_map(class_dir / "FaceMorphs" / (args.class_id + ".facemorphmap"))
    if len(targets) != args.expect_verts:
        raise SystemExit("map has %d wedges, the face mesh has %d"
                         % (len(targets), args.expect_verts))

    resolved_names = [skeleton[b] for b in bone_map]
    missing = sorted({n for n in resolved_names if n not in cooked_bones})
    if missing:
        raise SystemExit("%s: retail bones absent from the cooked palette: %s"
                         % (args.class_id, missing))
    palette = [cooked_bones.index(n) for n in resolved_names]
    facial = sum(1 for n in resolved_names if n.startswith("b_fc_"))

    pairs = []
    for wedge in range(len(influences)):
        for mesh_index, local_index in targets[wedge]:
            offset, count = submeshes[mesh_index]
            if local_index < count:
                pairs.append((wedge, vertex_base + (offset + local_index) * stride, mesh_index))
    cooked_positions = [struct.unpack_from("<3f", data, at) for _w, at, _m in pairs]
    permutation, signs = find_axis_transform(positions, cooked_positions, pairs)

    written = skipped = 0
    distances = []
    touched = collections.Counter()
    dominant = collections.Counter()
    entries_by_wedge = {}
    for wedge, (indices, weights) in enumerate(influences):
        entries = [(palette[indices[k]], weights[k] / 255.0) for k in range(4) if weights[k]]
        while len(entries) < 4:
            entries.append((entries[0][0], 0.0))
        entries_by_wedge[wedge] = entries
    for (wedge, at, mesh_index), cooked in zip(pairs, cooked_positions):
        entries = entries_by_wedge[wedge]
        distance = math.dist(cooked, transform(positions[wedge], permutation, signs))
        distances.append(distance)
        if distance > args.max_distance:
            skipped += 1
            continue
        struct.pack_into("<4I", data, at + COOKED_INDEX_OFFSET, *[e[0] for e in entries])
        struct.pack_into("<4f", data, at + COOKED_WEIGHT_OFFSET, *[e[1] for e in entries])
        written += 1
        touched[mesh_index] += 1
        dominant[cooked_bones[max(entries, key=lambda e: e[1])[0]]] += 1

    distances.sort()
    print("%s: axes %s signs %s; %d/%d bone-map entries are facial; %d vertices written %s"
          % (args.class_id, permutation, signs, facial, len(bone_map), written,
             dict(sorted(touched.items()))))
    print("   wedge-to-vertex distance: median %.4f, 99th %.4f, max %.4f (%d skipped over %.3f)"
          % (distances[len(distances) // 2], distances[int(len(distances) * 0.99)],
             distances[-1], skipped, args.max_distance))
    print("   dominant bones after the write: %s" % dict(dominant.most_common(6)))
    if args.dry_run:
        print("   dry run, %s not written" % model_path)
        return 0
    model_path.write_bytes(bytes(data))
    print("   written -> %s" % model_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
