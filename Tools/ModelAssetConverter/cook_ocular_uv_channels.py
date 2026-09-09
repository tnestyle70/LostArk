"""Put the retail face mesh's extra UV channels on a cooked body's ocular submesh.

Why this exists: the retail eye material compiles to source-character program 5, and
`CModel` refuses that program on a submesh without native UV1 and UV2 --
`Model.cpp`, "source character requires native extra UV channels". Those channels only
exist in a WModel cooked at 1.3 (`WINT_SKINNED_UV_VERSION_MINOR`). Measured on the four
playable bodies, only DimensionMaster is 1.3; LanceMaster, Warlord and Artist are 1.0, so
adding their eye rows to `CharacterCatalog.json` failed the whole body load.

The correspondence is not guessed. `<Class>.facemorphmap` was built by matching every wedge
of the retail face SkeletalMesh to the runtime vertices it became, and the glTF export of
that same mesh carries TEXCOORD_1 and TEXCOORD_2. Wedge indices run over the mesh's
primitives concatenated in order, which is exactly how the map numbers them -- verified
per class by the wedge range each cooked submesh occupies:

    LanceMaster  face 0..1375   eyelash 1376..1733  eye 1734..1883   (1376 + 358 + 150)
    Warlord      face 0..1199   eye 1200..1361                       (1200 + 162)
    Artist       face 0..2350   eyeAO   2351..2672  eye 2673..2898   (2351 + 322 + 226)

So a cooked vertex takes the UVs of the wedge it was made from. Every vertex of the chosen
submesh has to be covered; a partial submesh is refused rather than filled with invented
values, because `cook_skinned_uv_contract` exists precisely so that no UV is manufactured.

The write itself is `cook_skinned_uv_contract`, which appends the WUVS block and bumps the
file to 1.3 while proving the legacy vertex stream, the skinning and every non-mesh section
came through unchanged. Weight work already done on these bodies therefore survives.

usage:
  python cook_ocular_uv_channels.py --class-id Warlord
      --face-gltf <extract>/races/WR/PC_WR_00_FACE/SkeletalMesh3/pc_wr_00_face_sk.gltf
      --material pc_wr_eye_mi [--dry-run]
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
import transplant_face_skin_weights as _transplant  # noqa: E402

CHANNELS = ("TEXCOORD_1", "TEXCOORD_2")
COMPONENT_FLOAT = 5126
TYPE_VEC2 = "VEC2"


def read_wedge_uvs(path: Path):
    """Return {channel: [float2 per wedge]} for the mesh, primitives concatenated in order.

    A channel absent from any primitive is absent from the result: a mesh either carries the
    set throughout or this cannot supply it, and half a channel is not a source.
    """
    gltf = json.loads(path.read_text(encoding="utf-8"))
    buffers = [(path.parent / buffer["uri"]).read_bytes() for buffer in gltf["buffers"]]
    primitives = gltf["meshes"][0]["primitives"]
    out = {}
    for channel in CHANNELS:
        if any(channel not in primitive["attributes"] for primitive in primitives):
            continue
        values = []
        for primitive in primitives:
            accessor = gltf["accessors"][primitive["attributes"][channel]]
            if accessor["componentType"] != COMPONENT_FLOAT or accessor["type"] != TYPE_VEC2:
                raise SystemExit("%s: %s is not a float VEC2 accessor" % (path.name, channel))
            view = gltf["bufferViews"][accessor["bufferView"]]
            start = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
            stride = view.get("byteStride") or 8
            blob = buffers[view["buffer"]]
            for index in range(accessor["count"]):
                values.append(struct.unpack_from("<2f", blob, start + index * stride))
        out[channel] = values
    return out


def main() -> int:
    repo = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--class-id", required=True)
    parser.add_argument("--face-gltf", type=Path, required=True)
    parser.add_argument("--material", required=True,
                        help="the submesh's material name, e.g. pc_wr_eye_mi")
    parser.add_argument("--wmodel", type=Path)
    parser.add_argument("--resources", type=Path, default=repo / "Client" / "Bin" / "Resources")
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
    if parsed["versionMinor"] != 0:
        raise SystemExit("%s: already at 1.%d, this cook only upgrades a 1.0 file"
                         % (model_path.name, parsed["versionMinor"]))
    vertex_count = parsed["submeshes"][submesh][1]

    uvs = read_wedge_uvs(args.face_gltf)
    for channel in CHANNELS:
        if channel not in uvs:
            raise SystemExit("%s: no %s -- program 5 needs both extra channels"
                             % (args.face_gltf.name, channel))
    wedge_count = len(uvs[CHANNELS[0]])

    targets = _transplant.read_map(class_dir / "FaceMorphs" / (args.class_id + ".facemorphmap"))
    if len(targets) != wedge_count:
        raise SystemExit("map has %d wedges, %s has %d"
                         % (len(targets), args.face_gltf.name, wedge_count))

    wedge_of = {}
    for wedge, pairs in enumerate(targets):
        for mesh_index, local_index in pairs:
            if mesh_index != submesh:
                continue
            if local_index >= vertex_count:
                raise SystemExit("map points at vertex %d of a %d-vertex submesh"
                                 % (local_index, vertex_count))
            if wedge_of.setdefault(local_index, wedge) != wedge:
                raise SystemExit("vertex %d of submesh %d is claimed by wedges %d and %d"
                                 % (local_index, submesh, wedge_of[local_index], wedge))
    missing = [index for index in range(vertex_count) if index not in wedge_of]
    if missing:
        raise SystemExit(
            "%s submesh %d (%s): %d of %d vertices have no wedge, so their UVs have no source. "
            "Vertices %d..%d are unmapped." % (args.class_id, submesh, args.material,
                                               len(missing), vertex_count, missing[0], missing[-1]))

    channels = {submesh: {channel: [uvs[channel][wedge_of[index]] for index in range(vertex_count)]
                          for channel in CHANNELS}}
    cooked, receipt = _cook.cook_skinned_uv_contract(data, channels)

    print("%s: submesh %d (%s), %d vertices from %d wedges of %s"
          % (model_path.name, submesh, args.material, vertex_count,
             len(set(wedge_of.values())), args.face_gltf.name))
    print("  %s" % json.dumps(receipt, sort_keys=True)[:400])
    if args.dry_run:
        print("  dry run, nothing written")
        return 0
    model_path.write_bytes(cooked)
    print("  written, %d -> %d bytes" % (len(data), len(cooked)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
