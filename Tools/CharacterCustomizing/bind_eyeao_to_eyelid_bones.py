"""Bind a cooked body's eye-AO shell to the eyelid bones so it closes with the lids.

This is a deliberate deviation from the retail body extract, the same kind
``bind_eyeball_to_eye_bones.py`` makes and for the same class, so it says why in full.

The shell drawn with the ``eyeao``/``eyelashes`` material sits over the eye within
hundredths of a unit of the eyelid skin.  Measured across the four playable bodies, three
of them already skin it to the lid cluster and one does not:

    LanceMaster      pc_ft_eyelashes_mi  362 verts  12 bones  b_fc_l/r_eye_up_cm, _r_cm, ...
    DimensionMaster  pc_sp_m_eyeao_mi    512 verts  14 bones  b_fc_l/r_eye_r_cm, _up_cm, ...
    Artist           pc_sp_eyeao_mi      322 verts   1 bone   bip001-head at weight 1.0
    Warlord          -- no such submesh --

So on Artist the lids close and the shell stays where it was, which reads as the eyelashes
being left behind on a shut eye.  The retail body really is authored that way -- the same
``restore_body_eye_weights.py`` note that keeps the eyeballs on ``bip001-head`` covers this
submesh -- because in game the head is the separate face SkeletalMesh this project does not
load, and the body's own ocular shells are never drawn.  Until that is loaded, this binds the
shell the way the other two bodies already are.

Weights are not invented.  Each shell vertex takes the influences of the nearest vertex of the
body's own face submesh, which already carries the eight lid bones
(``b_fc_l/r_eye_{up,dn,l,r}_cm``, about 120 vertices each on Artist).  Where the shell lies
over skin that is rigid the nearest face vertex is rigid too and the shell stays rigid there,
so the rule needs no list of which part of the shell is lid and which is not.

Like its sibling the tool refuses anything but a submesh that is entirely unrigged, so it
cannot touch a body whose shell retail already rigged -- and, for the same reason, a second
run on a body it has already bound refuses instead of binding again.

The struct layouts are repeated here rather than imported from
``build_face_morph_vertex_map``/``transplant_face_skin_weights``: those pull in the LPK reader
and pycryptodome, which this task has no use for.

usage:
  python bind_eyeao_to_eyelid_bones.py --class-id Artist [--dry-run]
"""
from __future__ import annotations

import argparse
import collections
import struct
from pathlib import Path

import numpy

FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
SUBMESH_DESC = struct.Struct("<IIIIIQ20s")
MATERIAL_META = struct.Struct("<4sI")
MATERIAL_ENTRY = {
    b"WMAT": struct.Struct("<IQ64s520s"),
    b"WMA2": struct.Struct("<IQ64s" + "520s" * 9),
    b"WMA3": struct.Struct("<IQ64s" + "520s" * 10 + "16f"),
}
BONE_ENTRY = struct.Struct("<Q32si16fI16s")
SECTION_MESH = 1
SECTION_MATERIAL = 2
SUBMESH_MATERIAL_FIELD = 4
COOKED_INDEX_OFFSET = 44
COOKED_WEIGHT_OFFSET = 60

LID_BONES = (
    "b_fc_l_eye_up_cm", "b_fc_l_eye_dn_cm", "b_fc_l_eye_l_cm", "b_fc_l_eye_r_cm",
    "b_fc_r_eye_up_cm", "b_fc_r_eye_dn_cm", "b_fc_r_eye_l_cm", "b_fc_r_eye_r_cm",
)
# The shell and the lid skin are cooked from the same head, so a pairing is only ever a step
# across the face mesh's own vertex spacing.  A shell vertex whose nearest face vertex is
# farther than this share of the face's bounding diagonal is not the pairing this tool assumes,
# and it refuses rather than skinning that vertex to something across the head.  Cooked units
# differ per model, so the bound is a fraction rather than a distance: measured on Artist the
# face diagonal is 21.3, the median pairing 0.126 and the furthest 0.457, i.e. 2.1%.
MAXIMUM_PAIR_FRACTION = 0.05


def sections(data):
    base = FILE_HEADER.size
    _magic, section_count = MODEL_HEADER.unpack_from(data, base)[:2]
    for index in range(section_count):
        yield SECTION_DESC.unpack_from(data, base + MODEL_HEADER.size + index * SECTION_DESC.size)


def payload_at(data, offset):
    base = FILE_HEADER.size
    start = next(c for c in (base + offset, offset) if data[c:c + 4] == b"WINT")
    return start + FILE_HEADER.size


def read_material_names(data):
    """Return {materialIndex: lowercase name} as a submesh descriptor indexes them."""
    for section_type, _i, offset, _size, _name in sections(data):
        if section_type != SECTION_MATERIAL:
            continue
        at = payload_at(data, offset)
        magic, count = MATERIAL_META.unpack_from(data, at)
        if magic not in MATERIAL_ENTRY:
            raise SystemExit("unknown material container %r" % magic)
        entry = MATERIAL_ENTRY[magic]
        at += MATERIAL_META.size
        names = {}
        for row in range(count):
            fields = entry.unpack_from(data, at + row * entry.size)
            names[fields[0]] = fields[2].split(b"\0")[0].decode("ascii", "replace").lower()
        return names
    raise SystemExit("no material section")


def read_cooked(path: Path):
    """Return (bytes, vertexBase, stride, submeshes, materialIndices, paletteNames)."""
    data = bytearray(path.read_bytes())
    for section_type, _i, offset, _size, _name in sections(data):
        if section_type != SECTION_MESH:
            continue
        payload = payload_at(data, offset)
        header = MESH_HEADER.unpack_from(data, payload)
        magic, submesh_count, bone_count, _vflags, stride, vertex_count, index_count, index_stride = header[:8]
        if magic != b"WMSH":
            raise SystemExit("%s: expected WMSH" % path)
        submesh_base = payload + MESH_HEADER.size
        submeshes, materials = [], []
        for i in range(submesh_count):
            fields = SUBMESH_DESC.unpack_from(data, submesh_base + i * SUBMESH_DESC.size)
            if fields[0] % stride:
                raise SystemExit("%s: submesh %d vertexOffset is not a multiple of stride" % (path, i))
            submeshes.append((fields[0] // stride, fields[1]))
            materials.append(fields[SUBMESH_MATERIAL_FIELD])
        vertex_base = submesh_base + submesh_count * SUBMESH_DESC.size
        bone_base = vertex_base + vertex_count * stride + index_count * index_stride
        palette = [BONE_ENTRY.unpack_from(data, bone_base + b * BONE_ENTRY.size)[1]
                   .split(b"\0")[0].decode("ascii", "replace").lower()
                   for b in range(bone_count)]
        return data, vertex_base, stride, submeshes, materials, palette
    raise SystemExit("%s: no mesh section" % path)


def submesh_arrays(data, vertex_base, stride, submesh):
    offset, count = submesh
    positions = numpy.empty((count, 3), dtype=numpy.float64)
    indices = numpy.empty((count, 4), dtype=numpy.uint32)
    weights = numpy.empty((count, 4), dtype=numpy.float32)
    for vertex in range(count):
        at = vertex_base + (offset + vertex) * stride
        positions[vertex] = struct.unpack_from("<3f", data, at)
        indices[vertex] = struct.unpack_from("<4I", data, at + COOKED_INDEX_OFFSET)
        weights[vertex] = struct.unpack_from("<4f", data, at + COOKED_WEIGHT_OFFSET)
    return positions, indices, weights


def main() -> int:
    repo = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--class-id", required=True)
    parser.add_argument("--wmodel", type=Path)
    parser.add_argument("--resources", type=Path, default=repo / "Client" / "Bin" / "Resources")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    model_path = args.wmodel or (args.resources / "Character" / args.class_id /
                                 (args.class_id + ".wmodel"))
    original = model_path.read_bytes()
    data, vertex_base, stride, submeshes, materials, palette = read_cooked(model_path)
    names = read_material_names(data)
    material_of = {index: names.get(material, "") for index, material in enumerate(materials)}

    shell = [index for index, name in material_of.items()
             if "eyeao" in name or "eye_ao" in name or "eyelash" in name]
    if len(shell) != 1:
        raise SystemExit("%s: expected one eye-AO/eyelash material, found %s"
                         % (args.class_id, [material_of[i] for i in shell]))
    shell = shell[0]

    face = [index for index, name in material_of.items() if "face" in name]
    if len(face) != 1:
        raise SystemExit("%s: expected one face material, found %s"
                         % (args.class_id, [material_of[i] for i in face]))
    face = face[0]

    missing = [bone for bone in LID_BONES if bone not in palette]
    if missing:
        raise SystemExit("%s: the mesh palette has no %s" % (args.class_id, missing))

    shell_positions, shell_indices, shell_weights = submesh_arrays(
        data, vertex_base, stride, submeshes[shell])
    face_positions, face_indices, face_weights = submesh_arrays(
        data, vertex_base, stride, submeshes[face])

    current = set()
    for vertex in range(len(shell_positions)):
        live = {int(shell_indices[vertex][k]) for k in range(4) if shell_weights[vertex][k] > 0.0}
        if len(live) != 1:
            raise SystemExit("%s: submesh %d vertex %d has %d influences; this tool only binds a "
                             "submesh that is entirely unrigged"
                             % (args.class_id, shell, vertex, len(live)))
        current.add(palette[live.pop()])
    if len(current) != 1 or current & set(LID_BONES):
        raise SystemExit("%s: submesh %d is already rigged to %s; nothing to bind"
                         % (args.class_id, shell, sorted(current)))

    lid_face_vertices = sum(
        1 for vertex in range(len(face_positions))
        if any(face_weights[vertex][k] > 0.0 and palette[int(face_indices[vertex][k])] in LID_BONES
               for k in range(4)))
    if 0 == lid_face_vertices:
        raise SystemExit("%s: submesh %d carries no lid bone; there is nothing to copy"
                         % (args.class_id, face))

    deltas = shell_positions[:, None, :] - face_positions[None, :, :]
    distances = numpy.sqrt(numpy.einsum("ijk,ijk->ij", deltas, deltas))
    nearest = numpy.argmin(distances, axis=1)
    furthest = float(distances[numpy.arange(len(shell_positions)), nearest].max())
    diagonal = float(numpy.linalg.norm(face_positions.max(0) - face_positions.min(0)))
    bound = diagonal * MAXIMUM_PAIR_FRACTION
    if furthest > bound:
        raise SystemExit("%s: a shell vertex is %.4f from the nearest face vertex, past the %.4f "
                         "this pairing assumes (%.1f%% of the %.3f face diagonal)"
                         % (args.class_id, furthest, bound, MAXIMUM_PAIR_FRACTION * 100.0, diagonal))

    print("%s: %r %d verts, currently all on %s" %
          (args.class_id, material_of[shell], len(shell_positions), sorted(current)[0]))
    print("   source %r %d verts, %d of them on a lid bone; furthest pairing %.5f of %.3f allowed"
          % (material_of[face], len(face_positions), lid_face_vertices, furthest, bound))

    offset = submeshes[shell][0]
    histogram = collections.Counter()
    for vertex in range(len(shell_positions)):
        source = int(nearest[vertex])
        at = vertex_base + (offset + vertex) * stride
        struct.pack_into("<4I", data, at + COOKED_INDEX_OFFSET, *(int(v) for v in face_indices[source]))
        struct.pack_into("<4f", data, at + COOKED_WEIGHT_OFFSET, *(float(v) for v in face_weights[source]))
        for k in range(4):
            if face_weights[source][k] > 0.0:
                histogram[palette[int(face_indices[source][k])]] += 1
    print("   bones after: " + ", ".join("%s:%d" % row for row in histogram.most_common(8)))

    if args.dry_run:
        print("   dry run, %d vertices would be bound" % len(shell_positions))
        return 0
    if bytes(data) == original:
        print("   already bound, %s left byte-identical" % model_path)
        return 0
    model_path.write_bytes(bytes(data))
    print("   %d vertices bound -> %s" % (len(shell_positions), model_path))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
