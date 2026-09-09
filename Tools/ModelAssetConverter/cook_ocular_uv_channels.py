"""Put the retail body's extra UV channels on a cooked body's ocular submesh.

Why this exists: the retail eye material compiles to source-character program 5, and
`CModel` refuses that program on a submesh without native UV1 and UV2 -- `Model.cpp`,
"source character requires native extra UV channels". Those channels only exist in a WModel
cooked at 1.3 (`WINT_SKINNED_UV_VERSION_MINOR`). Measured on the four playable bodies, only
DimensionMaster arrived at 1.3; LanceMaster, Warlord and Artist were 1.0, so adding their eye
rows to `CharacterCatalog.json` failed the whole body load.

## Where each slot comes from

Read off the retail meshes and off the DimensionMaster body this project already ships,
rather than assumed.

* Slot 1 is the model's own UV0. That is what the shipped DimensionMaster cook holds -- its
  UV1 equals its UV0 on all 242 eye vertices -- and it is how every retail face SkeletalMesh
  ships its second set as well, a byte-identical copy of the first.
* Slot 2 is the source mesh's extra channel: the eye's own UV layout, which program 5 reads
  as the iris coordinate and as the odd-eye selector. On a three-set mesh that is TEXCOORD_2;
  the FT body carries only two sets and its second one is that same channel -- real values on
  the head sections (face 694, eyelash 306, eye 75 distinct) and a constant near-zero on the
  sections that do not use it.

## Reaching a vertex

The submesh records the mesh it was cooked from, and that mesh is matched vertex by vertex on
exact position and UV0, so a vertex the cook split at a seam still resolves to one source
vertex. Only the source the cook actually used reaches every vertex: LanceMaster's eye
submesh is 265 vertices, 155 from the body's eye section and 110 from a second section that
shares the eye material and has no counterpart in the separate face SkeletalMesh.

The exporter flips handedness, so the cook's lateral axis is mirrored. That sign is fitted
once per model over every submesh rather than on the eye alone: an eyeball is symmetric in
position, UV0 and normal, so on its own it matches under either sign. Whole-model margins are
decisive -- Warlord 9576/3381, Artist 14186/5520, LanceMaster 8978/1227,
DimensionMaster 34516/8986 -- and a fit without a clear winner is refused.

Cross-checked against a cook this tool did not produce: run against the shipped
DimensionMaster body it reproduces the stored TEXCOORD_2 on every eye vertex it resolves and
the stored TEXCOORD_1 on all 242.

Nothing is invented: a vertex with no source vertex is a hard error.

The write is `cook_skinned_uv_contract`, which appends the WUVS block and bumps the file to
1.3 while proving the legacy vertex stream, the skinning and every non-mesh section came
through unchanged, so weight work already done on these bodies survives.

usage:
  python cook_ocular_uv_channels.py --class-id Warlord --material pc_wr_eye_mi
      --body-gltf <extract>/body2/PC_WR_00/mesh/pc_wr_00_sk.gltf [--verify] [--dry-run]
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "CharacterCustomizing"))
import cook_wmodel_geometry_contract as _cook  # noqa: E402
import build_face_morph_vertex_map as _map  # noqa: E402

COMPONENT_FLOAT = 5126
COOKED_TEXCOORD_OFFSET = 24
# The cook stores centimetres with the exporter's y-up axes swapped away.
SOURCE_SCALE = 100.0
# Coarse enough that a coordinate sitting on a rounding boundary still shares a bucket with
# its match; the candidates a bucket gathers are then compared properly.
BUCKET = 1
BUCKET_STEP = 0.1
POSITION_TOLERANCE = 1e-3
TEXCOORD_TOLERANCE = 1e-4


def read_source(path: Path):
    """Return [(position, uv0, extra)] for every vertex, primitives concatenated in order."""
    gltf = json.loads(path.read_text(encoding="utf-8"))
    buffers = [(path.parent / buffer["uri"]).read_bytes() for buffer in gltf["buffers"]]

    def accessor(index, components):
        record = gltf["accessors"][index]
        if record["componentType"] != COMPONENT_FLOAT:
            raise SystemExit("%s: accessor %d is not float" % (path.name, index))
        view = gltf["bufferViews"][record["bufferView"]]
        start = view.get("byteOffset", 0) + record.get("byteOffset", 0)
        stride = view.get("byteStride") or 4 * components
        blob = buffers[view["buffer"]]
        return [struct.unpack_from("<%df" % components, blob, start + i * stride)
                for i in range(record["count"])]

    rows = []
    for primitive in gltf["meshes"][0]["primitives"]:
        attributes = primitive["attributes"]
        if "TEXCOORD_1" not in attributes:
            raise SystemExit("%s: a primitive has no extra UV channel" % path.name)
        extra = "TEXCOORD_2" if "TEXCOORD_2" in attributes else "TEXCOORD_1"
        rows += list(zip(accessor(attributes["POSITION"], 3),
                         accessor(attributes["TEXCOORD_0"], 2),
                         accessor(attributes[extra], 2)))
    return rows


def place(position, sign):
    return (position[0] * SOURCE_SCALE, sign * position[2] * SOURCE_SCALE,
            -position[1] * SOURCE_SCALE)


def read_cooked_vertices(parsed, submesh):
    offset, count = parsed["submeshes"][submesh][:2]
    stride = parsed["meshHeader"][4]
    base = parsed["vertexStart"] + offset
    mesh = parsed["mesh"]
    return [(struct.unpack_from("<3f", mesh, base + i * stride),
             struct.unpack_from("<2f", mesh, base + i * stride + COOKED_TEXCOORD_OFFSET))
            for i in range(count)]


def fit_lateral_sign(parsed, source):
    """Decide the exporter's lateral flip over the whole model, not over one submesh.

    Scored the same way the transfer itself resolves a vertex -- position and UV0 together --
    because position alone is nearly sign-blind on a body that is mostly symmetric.
    """
    everything = [vertex for submesh in range(len(parsed["submeshes"]))
                  for vertex in read_cooked_vertices(parsed, submesh)]
    scored = {}
    for sign in (1, -1):
        _extra, missing, _ambiguous = resolve_extra(source, sign, everything)
        scored[sign] = len(everything) - len(missing)
    best, other = (1, -1) if scored[1] >= scored[-1] else (-1, 1)
    if scored[best] == 0 or scored[best] < scored[other] * 2:
        raise SystemExit("the lateral flip is not decided by the model: %+d scores %d, %+d "
                         "scores %d" % (best, scored[best], other, scored[other]))
    return best, scored


def resolve_extra(source, sign, cooked):
    """One source vertex per cooked vertex, on exact position and UV0."""
    table = {}
    for position, uv, extra in source:
        p = place(position, sign)
        table.setdefault((round(p[0], BUCKET), round(p[1], BUCKET), round(p[2], BUCKET)),
                         []).append((p, uv, extra))
    out, missing, ambiguous = [], [], []
    for index, (position, uv) in enumerate(cooked):
        found = []
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                for dz in (-1, 0, 1):
                    key = (round(position[0] + dx * BUCKET_STEP, BUCKET),
                           round(position[1] + dy * BUCKET_STEP, BUCKET),
                           round(position[2] + dz * BUCKET_STEP, BUCKET))
                    for candidate_position, candidate_uv, extra in table.get(key, ()):
                        if all(abs(candidate_position[k] - position[k]) <= POSITION_TOLERANCE
                               for k in range(3)) and \
                                abs(candidate_uv[0] - uv[0]) <= TEXCOORD_TOLERANCE and \
                                abs(candidate_uv[1] - uv[1]) <= TEXCOORD_TOLERANCE:
                            found.append(extra)
        if not found:
            missing.append(index)
            out.append((0.0, 0.0))
        elif len({(round(v[0], 5), round(v[1], 5)) for v in found}) > 1:
            ambiguous.append(index)
            out.append(found[0])
        else:
            out.append(found[0])
    return out, missing, ambiguous


def restore_legacy(data, parsed):
    """Undo a previous WUVS append so the same file can be cooked again.

    `cook_skinned_uv_contract` proves it changed nothing before the block it added, so
    dropping that block and the flags it set returns the file to its 1.0 bytes.
    """
    content = bytearray(parsed["mesh"][_cook.FILE_HEADER.size:parsed["legacyEnd"]])
    flags = parsed["meshHeader"][3] & ~(_cook.VF_TEXCOORD1 | _cook.VF_TEXCOORD2)
    struct.pack_into("<I", content, 12, flags)
    mesh = _cook.FILE_HEADER.pack(b"WINT", 1, 0, 0, len(content)) + bytes(content)
    return _cook.rebuild_wmodel(parsed["modelHeader"], parsed["sections"], mesh)


def main() -> int:
    repo = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--class-id", required=True)
    parser.add_argument("--body-gltf", type=Path, required=True)
    parser.add_argument("--material", required=True,
                        help="the submesh's material name, e.g. pc_wr_eye_mi")
    parser.add_argument("--wmodel", type=Path)
    parser.add_argument("--resources", type=Path, default=repo / "Client" / "Bin" / "Resources")
    parser.add_argument("--verify", action="store_true",
                        help="compare against the file's stored channels instead of writing")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    class_dir = args.resources / "Character" / args.class_id
    model_path = args.wmodel or (class_dir / (args.class_id + ".wmodel"))
    data = model_path.read_bytes()

    material_names = _map.read_material_names(model_path)
    submesh_materials = _map.read_submesh_materials(model_path)
    selected = [index for index, material in enumerate(submesh_materials)
                if material_names.get(material) == args.material]
    if len(selected) != 1:
        raise SystemExit("%s: material %r names %d submeshes, expected exactly one (have %s)"
                         % (model_path.name, args.material, len(selected),
                            sorted(set(material_names.values()))))
    submesh = selected[0]

    parsed = _cook.parse_skinned_uv_wmodel(data)
    source = read_source(args.body_gltf)
    sign, scored = fit_lateral_sign(parsed, source)
    cooked = read_cooked_vertices(parsed, submesh)
    extra, missing, ambiguous = resolve_extra(source, sign, cooked)

    print("%s: submesh %d (%s), %d vertices from %s, lateral %+d (%d vs %d)"
          % (model_path.name, submesh, args.material, len(cooked), args.body_gltf.name,
             sign, scored[sign], scored[-sign]))
    if missing:
        raise SystemExit("  %d of %d vertices are not in the source, so their UVs have no "
                         "origin (vertices %d..%d)"
                         % (len(missing), len(cooked), missing[0], missing[-1]))
    if ambiguous:
        raise SystemExit("  %d vertices resolve to source vertices that disagree on the extra "
                         "channel" % len(ambiguous))

    slot1 = [tuple(uv) for _position, uv in cooked]
    if args.verify:
        row = parsed["uvRows"][submesh]
        for name, values in (("TEXCOORD_1", slot1), ("TEXCOORD_2", extra)):
            stored = row.get(name)
            if stored is None:
                print("  %s: absent in the file" % name)
                continue
            same = sum(1 for a, b in zip(stored, values)
                       if abs(a[0] - b[0]) < 1e-6 and abs(a[1] - b[1]) < 1e-6)
            print("  %s: %d of %d identical to what is stored" % (name, same, len(cooked)))
        return 0

    if parsed["versionMinor"] != 0:
        data = restore_legacy(data, parsed)
        print("  dropped the previous 1.%d UV block first" % parsed["versionMinor"])
    written, receipt = _cook.cook_skinned_uv_contract(
        data, {submesh: {"TEXCOORD_1": slot1, "TEXCOORD_2": extra}})
    print("  %s" % json.dumps(receipt, sort_keys=True)[:240])
    if args.dry_run:
        print("  dry run, nothing written")
        return 0
    model_path.write_bytes(written)
    print("  written, %d bytes" % len(written))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
