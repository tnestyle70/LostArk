"""Bind a cooked body's eyeballs to its eye bones so the eye-position slider moves them.

This is the one deliberate deviation from the retail body extract in the customizing path, so
it says why in full.

The creation screen's eye position, angle and width sliders drive `b_fc_earall_l/r_cm`.  In the
cooked skeleton that bone parents the whole eye and brow cluster -- eight children on each side,
including `b_fc_l_eye_ani`, the eyeball's own bone -- so moving it carries the eyeball with the
lids.  Measured, three of the four playable bodies bind their eyeballs to that bone and the
slider works on them:

    Warlord          93 / 92    b_fc_l/r_eye_ani
    DimensionMaster  121 / 121  b_fc_l/r_eye_ani
    LanceMaster      79 / 78    b_fc_l/r_eye_ani  (plus 108 verts of another piece on the head)
    Artist           0          all 226 on bip001-head

So on Artist the pupil cannot follow any facial bone, and the eye sliders slide the lids off the
eyes.  The retail body really is authored that way -- `pc_sp_00_sk` prim2 is 226 vertices at
`bip001-head` weight 1.0 -- and none of the three retail *face* meshes carries an eye bone
either.  What retail has and this project does not load is `PC_SP_AV_BASEBODY`, which ships its
own eye materials (`pc_sp_av_eye_mi`, `_left`, `_right`); those avatar eye meshes are what the
game draws, and the body eyeball underneath is never posed.  Loading them is a separate slice.

Until then this binds the body eyeball the way the other three classes already are.  That is a
project decision, not extracted data, and it is confined to a submesh that is entirely
unrigged: the tool refuses anything else, so it cannot touch a class whose eyeballs retail
already rigged, nor the mixed submesh LanceMaster has.

usage:
  python bind_eyeball_to_eye_bones.py --class-id Artist [--dry-run]
"""
from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build_face_morph_vertex_map as _map  # noqa: E402
import transplant_face_skin_weights as _tp  # noqa: E402

LEFT_BONE = "b_fc_l_eye_ani"
RIGHT_BONE = "b_fc_r_eye_ani"
# The two eyeballs sit either side of the head's centre line with a real gap between them.
# Anything narrower than this is not two eyeballs and the split would be a guess.
MINIMUM_CLUSTER_GAP = 0.5


def read_bone_bind_lateral(path: Path, wanted):
    """Return {boneName: lateral component of its inverse bind translation}.

    Which side is which is read off the model rather than assumed: measured on all four bodies
    `b_fc_l_eye_ani` has a positive value here and its skinned vertices have positive y, so the
    sign of this entry is what says which cluster is the left eye."""
    data = path.read_bytes()
    base = _map.WMODEL_FILE_HEADER.size
    _magic, section_count = _map.WMODEL_MODEL_HEADER.unpack_from(data, base)[:2]
    for index in range(section_count):
        section_type, _i, offset, _size, _name = _map.WMODEL_SECTION_DESC.unpack_from(
            data, base + _map.WMODEL_MODEL_HEADER.size + index * _map.WMODEL_SECTION_DESC.size)
        if section_type != _map.WMODEL_SECTION_MESH:
            continue
        start = next(c for c in (base + offset, offset) if data[c:c + 4] == b"WINT")
        payload = start + _map.WMODEL_FILE_HEADER.size
        header = _map.WMODEL_MESH_HEADER.unpack_from(data, payload)
        _magic2, submesh_count, bone_count, _flags, stride, vertices, indices, index_stride = header[:8]
        bone_base = (payload + _map.WMODEL_MESH_HEADER.size
                     + submesh_count * _map.WMODEL_SUBMESH_DESC.size
                     + vertices * stride + indices * index_stride)
        found = {}
        for bone in range(bone_count):
            fields = _tp.BONE_ENTRY.unpack_from(data, bone_base + bone * _tp.BONE_ENTRY.size)
            name = fields[1].split(b"\0")[0].decode("ascii", "replace").lower()
            if name in wanted:
                found[name] = fields[3:19][13]
        return found
    raise SystemExit("%s: no mesh section" % path)


def main() -> int:
    repo = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--class-id", required=True)
    parser.add_argument("--wmodel", type=Path)
    parser.add_argument("--resources", type=Path, default=repo / "Client" / "Bin" / "Resources")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    class_dir = args.resources / "Character" / args.class_id
    model_path = args.wmodel or (class_dir / (args.class_id + ".wmodel"))
    original = model_path.read_bytes()
    data, vertex_base, stride, submeshes, cooked_bones = _tp.read_cooked(model_path)

    names = _map.read_material_names(model_path)
    material_of = {index: names.get(material, "")
                   for index, material in enumerate(_map.read_submesh_materials(model_path))}
    # The eyeball's own material, not the eye-AO shell or the eyelashes that also say "eye".
    eyeball = [index for index, name in material_of.items()
               if "eye" in name and "eyeao" not in name and "eyelash" not in name]
    if len(eyeball) != 1:
        raise SystemExit("%s: expected one eyeball material, found %s"
                         % (args.class_id, [material_of[i] for i in eyeball]))
    submesh = eyeball[0]

    for bone in (LEFT_BONE, RIGHT_BONE):
        if bone not in cooked_bones:
            raise SystemExit("%s: the palette has no %s" % (args.class_id, bone))
    lateral = read_bone_bind_lateral(model_path, {LEFT_BONE, RIGHT_BONE})
    if lateral.get(LEFT_BONE, 0.0) * lateral.get(RIGHT_BONE, 0.0) >= 0.0:
        raise SystemExit("%s: the two eye bones do not sit on opposite sides" % args.class_id)
    left_is_positive = lateral[LEFT_BONE] > 0.0

    offset, count = submeshes[submesh]
    current, lateral_values = set(), []
    for vertex in range(count):
        at = vertex_base + (offset + vertex) * stride
        indices = struct.unpack_from("<4I", data, at + _tp.COOKED_INDEX_OFFSET)
        weights = struct.unpack_from("<4f", data, at + _tp.COOKED_WEIGHT_OFFSET)
        entries = {indices[k] for k in range(4) if weights[k] > 0.0}
        if len(entries) != 1:
            raise SystemExit("%s: submesh %d vertex %d has %d influences; this tool only binds a "
                             "submesh that is entirely unrigged"
                             % (args.class_id, submesh, vertex, len(entries)))
        current.add(cooked_bones[entries.pop()])
        lateral_values.append(struct.unpack_from("<3f", data, at)[1])
    if len(current) != 1 or current & {LEFT_BONE, RIGHT_BONE}:
        raise SystemExit("%s: submesh %d is already rigged to %s; nothing to bind"
                         % (args.class_id, submesh, sorted(current)))

    left = [v for v in lateral_values if v > 0.0]
    right = [v for v in lateral_values if v <= 0.0]
    if not left or not right:
        raise SystemExit("%s: submesh %d is all on one side" % (args.class_id, submesh))
    gap = min(left) - max(right)
    if gap < MINIMUM_CLUSTER_GAP:
        raise SystemExit("%s: the two eyeballs are only %.4f apart; the split would be a guess"
                         % (args.class_id, gap))

    print("%s: %r %d verts, currently all on %s; %d left / %d right, gap %.4f"
          % (args.class_id, material_of[submesh], count, sorted(current)[0],
             len(left), len(right), gap))

    left_index = cooked_bones.index(LEFT_BONE)
    right_index = cooked_bones.index(RIGHT_BONE)
    for vertex in range(count):
        at = vertex_base + (offset + vertex) * stride
        positive = struct.unpack_from("<3f", data, at)[1] > 0.0
        bone = left_index if positive == left_is_positive else right_index
        # Single influence at 1.0, which is how the three rigged bodies carry their eyeballs;
        # unused slots park on bone 0 the way the cook writes them.
        struct.pack_into("<4I", data, at + _tp.COOKED_INDEX_OFFSET, bone, 0, 0, 0)
        struct.pack_into("<4f", data, at + _tp.COOKED_WEIGHT_OFFSET, 1.0, 0.0, 0.0, 0.0)

    if args.dry_run:
        print("   dry run, %d vertices would be bound" % count)
        return 0
    if bytes(data) == original:
        print("   already bound, %s left byte-identical" % model_path)
        return 0
    model_path.write_bytes(bytes(data))
    print("   %d vertices bound -> %s" % (count, model_path))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
