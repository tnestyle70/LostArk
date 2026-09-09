"""Put the retail body's own skin weights back on a cooked body's ocular submeshes.

Why this exists: ``transplant_face_skin_weights.py`` copies the retail *face* SkeletalMesh's
facial rig onto the cooked body's head, and it paired wedges to runtime vertices by position.
The eyeball and eye-AO shells sit within a few hundredths of a unit of the eyelid skin, so the
0.05 distance filter accepted them as face-skin pairs and overwrote them.  Measured on the two
bodies that were run:

    Warlord  submesh 5 (pc_wr_eye_mi)   b_fc_l_eye_ani 93 / b_fc_r_eye_ani 92
                                        -> b_fc_l_eye_up_cm 89 / b_fc_l_eye_dn_cm 88 / ...
    Artist   submesh 2 (pc_sp_eye_mi)   bip001-head 226
                                        -> b_fc_mouse_ut_cm 113 / b_fc_mouse_dt_cm 113
             submesh 1 (pc_sp_eyeao_mi) bip001-head 322 -> eyelid and cheek bones

Both are wrong, and in opposite directions: Warlord lost a real eye rig, Artist gained mouth
bones on its eyeballs.  Neither is recoverable from the retail face mesh -- measured, none of
``pc_ft_00_face_sk``, ``pc_wr_00_face_sk``, ``pc_sp_00_face_sk`` carries a ``b_fc_*_eye_ani``
bone in its bone map at all.  The eyeballs exist only in the body, so the body extract is the
authority:

    pc_wr_00_sk    prim6 162 verts   b_fc_l_eye_ani 81 / b_fc_r_eye_ani 81, all weight 1.0
    pc_sp_00_sk    prim2 226 verts   bip001-head 226, all weight 1.0
                   prim1 322 verts   bip001-head 322, all weight 1.0
    pc_sp_m_00_sk  prim7 242 verts   b_fc_l_eye_ani 121 / b_fc_r_eye_ani 121, all weight 1.0

So Artist's eyeballs are not rigged in retail either; in game its rigged head is the separate
face mesh this project does not load.  Restoring means writing what the body extract says, not
inventing an eye rig for it.

## Pairing

The cook neither keeps the primitive split nor the vertex count: it duplicates wedges on UV
seams and merges primitives that share a material, so a cooked ocular submesh can hold more
vertices than the primitive it came from (Warlord 185 from 162, LanceMaster 265 from two
primitives; Artist and DimensionMaster happen to be 1:1).  Rather than guess the grouping,
every cooked vertex is matched to its nearest vertex across the whole retail body.

The axis convention is fitted rather than assumed, and it is decided on bone names rather than
on distance.  A body in bind pose is nearly mirror symmetric, so the reflected candidate fits
the positions just as well: measured on DimensionMaster it reproduced every weight value to the
digit and put each one on the opposite eye.  See ``fit_axis_transform``.

Running this on a body that was never transplanted must leave the file byte-identical; that is
the check that the pairing and the bone-name resolution are right.

usage:
  python restore_body_eye_weights.py --class-id Warlord
      --body-gltf <extract>/pc_wr_00_sk.gltf [--dry-run]
"""
from __future__ import annotations

import argparse
import collections
import itertools
import json
import struct
import sys
from pathlib import Path

import numpy

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build_face_morph_vertex_map as _map  # noqa: E402
import transplant_face_skin_weights as _tp  # noqa: E402

GLTF_COMPONENT = {5120: ("b", 1), 5121: ("B", 1), 5122: ("h", 2),
                  5123: ("H", 2), 5125: ("I", 4), 5126: ("f", 4)}
GLTF_COMPONENT_DIVISOR = {5120: 127.0, 5121: 255.0, 5122: 32767.0, 5123: 65535.0}
GLTF_TYPE_COUNT = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4, "MAT4": 16}


def read_accessor(gltf, buffers, index):
    accessor = gltf["accessors"][index]
    view = gltf["bufferViews"][accessor["bufferView"]]
    fmt, size = GLTF_COMPONENT[accessor["componentType"]]
    components = GLTF_TYPE_COUNT[accessor["type"]]
    stride = view.get("byteStride") or size * components
    at = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
    blob = buffers[view["buffer"]]
    divisor = (GLTF_COMPONENT_DIVISOR.get(accessor["componentType"])
               if accessor.get("normalized") else None)
    rows = []
    for row in range(accessor["count"]):
        values = struct.unpack_from("<%d%s" % (components, fmt), blob, at + row * stride)
        rows.append(tuple(v / divisor for v in values) if divisor else values)
    return rows


def read_body_gltf(path: Path):
    """Return (positions, influences) for the whole retail body, primitives concatenated.

    The cook merges and re-splits primitives, so keeping them apart here would only invite a
    guess about which one a cooked submesh came from."""
    gltf = json.loads(path.read_text(encoding="utf-8"))
    buffers = [(path.parent / buffer["uri"]).read_bytes() for buffer in gltf["buffers"]]
    joints = [gltf["nodes"][node].get("name", "").lower() for node in gltf["skins"][0]["joints"]]
    positions, influences = [], []
    for primitive in gltf["meshes"][0]["primitives"]:
        attributes = primitive["attributes"]
        positions.extend(read_accessor(gltf, buffers, attributes["POSITION"]))
        bones = read_accessor(gltf, buffers, attributes["JOINTS_0"])
        weights = read_accessor(gltf, buffers, attributes["WEIGHTS_0"])
        for bone, weight in zip(bones, weights):
            entries = sorted(((joints[bone[k]], float(weight[k]))
                              for k in range(len(weight)) if weight[k] > 0),
                             key=lambda pair: -pair[1])
            total = sum(w for _name, w in entries)
            if not entries or abs(total - 1.0) > 1e-3:
                raise SystemExit("%s: a vertex' weights sum to %.4f, not 1" % (path, total))
            if len(entries) > 4:
                raise SystemExit("%s: a vertex has %d influences, the cook holds 4"
                                 % (path, len(entries)))
            influences.append(entries)
    return numpy.asarray(positions, dtype=numpy.float64), influences


def bbox_centre(points):
    return (points.min(axis=0) + points.max(axis=0)) / 2.0


def bbox_span(points):
    return float((points.max(axis=0) - points.min(axis=0)).max())


def place(points, frame):
    permutation, signs, scale, source_centre, target_centre = frame
    moved = points[:, list(permutation)] - source_centre[list(permutation)]
    return moved * numpy.asarray(signs, dtype=numpy.float64) * scale + target_centre


def nearest(cooked_points, retail_points, frame, tolerance=None):
    """Return (index, distance, coincident) for every cooked vertex.

    ``coincident`` is every retail vertex within ``tolerance`` of the cooked one, and it is not
    a formality: the eyeball, the eyelash sheet and the eyelid skin share vertex positions
    exactly.  Measured on LanceMaster, four cooked vertices sit on three retail vertices at once
    -- one weighted to `b_fc_l_eye_ani`, two to the eyelid -- so the closest of them is decided
    by floating-point noise and the caller has to break the tie some other way."""
    moved = place(retail_points, frame)
    index = numpy.empty(len(cooked_points), dtype=numpy.int64)
    distance = numpy.empty(len(cooked_points), dtype=numpy.float64)
    coincident = []
    for start in range(0, len(cooked_points), 128):
        block = cooked_points[start:start + 128]
        spread = numpy.linalg.norm(block[:, None, :] - moved[None, :, :], axis=2)
        index[start:start + 128] = spread.argmin(axis=1)
        distance[start:start + 128] = spread.min(axis=1)
        if tolerance is not None:
            coincident.extend(numpy.flatnonzero(row <= tolerance) for row in spread)
    return index, distance, coincident


def fit_axis_transform(cooked_points, cooked_bones, retail_points, retail_influences,
                       sample=512):
    """Find the axis convention that maps the retail body extract onto the cook.

    Centre and scale come from the two whole bodies, which is the only pair of extents that
    describes the same thing on both sides -- a cooked submesh and the retail body do not share
    a bounding box.

    Position alone cannot pick the winner.  A body in bind pose is very nearly mirror symmetric,
    so the reflected candidate fits the vertices just as well and quietly swaps left for right;
    measured on DimensionMaster, it reproduced every weight value exactly but moved each one to
    the opposite eye.  Nor can a reflection simply be excluded the way
    ``build_face_morph_vertex_map.find_axis_transform`` excludes it -- there both sides are
    retail data in the same handedness, whereas this reads a right-handed y-up glTF export of a
    left-handed source, so the true mapping may itself be improper.

    The tie is broken on bone names instead: under the true transform a cooked vertex' nearest
    retail vertex carries the same dominant bone.  Most of the body was never touched by the
    transplant, so a whole-body sample is honest ground truth, and a mirrored candidate scores
    about half because every `_l_`/`_r_` pair disagrees."""
    source_centre, target_centre = bbox_centre(retail_points), bbox_centre(cooked_points)
    scale = bbox_span(cooked_points) / bbox_span(retail_points)
    step = max(1, len(cooked_points) // sample)
    probe, probe_bones = cooked_points[::step], cooked_bones[::step]
    retail_dominant = [entries[0][0] for entries in retail_influences]
    scored = []
    for permutation in itertools.permutations(range(3)):
        for signs in itertools.product((1, -1), repeat=3):
            frame = (permutation, signs, scale, source_centre, target_centre)
            rows, spread, _coincident = nearest(probe, retail_points, frame)
            agreement = sum(retail_dominant[int(row)] == bone
                            for row, bone in zip(rows, probe_bones)) / len(probe_bones)
            scored.append((float(numpy.median(spread)), float(spread.max()), agreement, frame))
    aligned = sorted((row for row in scored if row[0] <= 1e-3), key=lambda row: -row[2])
    if not aligned:
        raise SystemExit("no axis convention lines the retail body up with the cook "
                         "(best median residual %.6f)" % min(row[0] for row in scored))
    median, worst, agreement, frame = aligned[0]
    runner_up = aligned[1][2] if len(aligned) > 1 else 0.0
    # A body whose head the transplant rewrote cannot reach 100%: measured, Warlord agrees on
    # 84.7%, which is every vertex outside the 1508 the transplant touched.  What has to hold is
    # the margin -- a mirrored candidate disagrees on every left/right pair, so it lands far
    # below (0.284 for the same Warlord).
    if agreement < 0.6 or agreement - runner_up < 0.25:
        raise SystemExit("the axis convention is ambiguous: best bone agreement %.3f, next "
                         "%.3f" % (agreement, runner_up))
    return frame, median, worst, agreement, runner_up


def matches(current, cooked_bones, entries):
    """Is a cooked vertex' influence set already the one this retail vertex carries?"""
    resolved = {cooked_bones.index(name): weight for name, weight in entries
                if name in cooked_bones}
    return (current.keys() == resolved.keys()
            and all(abs(current[bone] - weight) <= 1e-4 for bone, weight in resolved.items()))


def main() -> int:
    repo = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--class-id", required=True)
    parser.add_argument("--body-gltf", type=Path, required=True,
                        help="the retail body extract the cook was made from")
    parser.add_argument("--wmodel", type=Path)
    parser.add_argument("--resources", type=Path, default=repo / "Client" / "Bin" / "Resources")
    parser.add_argument("--max-distance", type=float, default=1e-3)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    class_dir = args.resources / "Character" / args.class_id
    model_path = args.wmodel or (class_dir / (args.class_id + ".wmodel"))
    original = model_path.read_bytes()
    data, vertex_base, stride, submeshes, cooked_bones = _tp.read_cooked(model_path)
    names = _map.read_material_names(model_path)
    material_of = {index: names.get(material, "")
                   for index, material in enumerate(_map.read_submesh_materials(model_path))}
    retail_points, retail_influences = read_body_gltf(args.body_gltf)

    def submesh_points(index):
        offset, count = submeshes[index]
        return numpy.asarray(
            [struct.unpack_from("<3f", data, vertex_base + (offset + v) * stride)
             for v in range(count)], dtype=numpy.float64)

    ocular = sorted(i for i, name in material_of.items() if "eye" in name)
    if not ocular:
        raise SystemExit("%s: no ocular material among %s"
                         % (args.class_id, sorted(set(material_of.values()))))

    total = sum(count for _offset, count in submeshes)
    cooked_points = numpy.empty((total, 3), dtype=numpy.float64)
    cooked_dominant = []
    for vertex in range(total):
        at = vertex_base + vertex * stride
        cooked_points[vertex] = struct.unpack_from("<3f", data, at)
        indices = struct.unpack_from("<4I", data, at + _tp.COOKED_INDEX_OFFSET)
        weights = struct.unpack_from("<4f", data, at + _tp.COOKED_WEIGHT_OFFSET)
        cooked_dominant.append(cooked_bones[indices[max(range(4), key=lambda k: weights[k])]])
    frame, median, worst, agreement, runner_up = fit_axis_transform(
        cooked_points, cooked_dominant, retail_points, retail_influences)
    print("%s: the retail body lines up under axes %s signs %s scale %.4f (median %.7f, "
          "max %.7f, dominant bone agrees on %.1f%% of the body against %.1f%% for the next "
          "candidate)" % (args.class_id, frame[0], frame[1], frame[2], median, worst,
                          agreement * 100.0, runner_up * 100.0))

    written = 0
    for index in ocular:
        points = submesh_points(index)
        rows, distances, coincident = nearest(points, retail_points, frame,
                                              tolerance=args.max_distance)
        far = int(numpy.argmax(distances))
        if distances[far] > args.max_distance:
            raise SystemExit("%s submesh %d vertex %d is %.6f from its nearest retail vertex"
                             % (args.class_id, index, far, distances[far]))
        before, after = collections.Counter(), collections.Counter()
        offset, count = submeshes[index]
        already = 0
        for vertex in range(count):
            at = vertex_base + (offset + vertex) * stride
            indices = struct.unpack_from("<4I", data, at + _tp.COOKED_INDEX_OFFSET)
            weights = struct.unpack_from("<4f", data, at + _tp.COOKED_WEIGHT_OFFSET)
            before[cooked_bones[indices[max(range(4), key=lambda k: weights[k])]]] += 1

            entries = retail_influences[int(rows[vertex])]
            missing = sorted({name for name, _w in entries if name not in cooked_bones})
            if missing:
                raise SystemExit("%s: retail bones absent from the cooked palette: %s"
                                 % (args.class_id, missing))
            resolved = [(cooked_bones.index(name), weight) for name, weight in entries]
            after[cooked_bones[max(resolved, key=lambda e: e[1])[0]]] += 1

            # Leave a vertex the transplant never reached exactly as the cook wrote it, and
            # settle a coincident-vertex tie the cook's way.  Two reasons: the cook's float32 of
            # `byte / 255` is up to one ULP from the same quotient taken in double, so rewriting
            # an already-correct vertex churns its bytes for nothing; and where several retail
            # vertices share a position, whichever one the cook picked is as good an answer as
            # the one that came out closest here.  Weights are 1/255 apart at the finest, so
            # 1e-4 separates "the same value" from "a different value".
            current = {indices[k]: weights[k] for k in range(4) if weights[k] > 0.0}
            if any(matches(current, cooked_bones, retail_influences[int(candidate)])
                   for candidate in coincident[vertex]):
                already += 1
                continue

            # The cook parks every unused influence slot on bone 0 -- measured, that is the
            # only index any zero-weight slot holds in LanceMaster and DimensionMaster.
            padded = resolved + [(0, 0.0)] * (4 - len(resolved))
            struct.pack_into("<4I", data, at + _tp.COOKED_INDEX_OFFSET, *[e[0] for e in padded])
            struct.pack_into("<4f", data, at + _tp.COOKED_WEIGHT_OFFSET, *[e[1] for e in padded])
            written += 1
        print("   submesh %d %-26s %4d verts, max pair distance %.7f, %d already correct: "
              "%s -> %s" % (index, repr(material_of[index]), count, distances.max(), already,
                            dict(before.most_common(3)), dict(after.most_common(3))))

    identical = bytes(data) == original
    if identical != (written == 0):
        raise SystemExit("%d vertices were rewritten but the file %s"
                         % (written, "did not change" if identical else "changed anyway"))
    if args.dry_run:
        print("   dry run, %d vertices would be rewritten" % written)
        return 0
    if identical:
        print("   nothing to repair, %s left byte-identical" % model_path)
        return 0
    model_path.write_bytes(bytes(data))
    print("   %d vertices rewritten -> %s" % (written, model_path))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
