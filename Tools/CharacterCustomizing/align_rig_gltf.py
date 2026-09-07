"""Reshape a UModel rig glTF so its cooked skeleton matches a body WModel's.

``CModel::Attach_AnimationSet`` accepts an animation set only when its skeleton
hash and bone count equal the body's, and that hash is an FNV-1a walk over the
bone names in order.  A body cooked through the FBX path carries three wrapper
nodes the raw UModel glTF does not have: Assimp's ``RootNode``, an armature node
and a mesh node.  Their names and their order differ per class.

This step reads those wrapper names straight out of the body WModel skeleton and
rebuilds the glTF scene graph to reproduce them, so the converted animation set
lands on the same bone table.  It changes no joint, no transform and no mesh
data; it only wraps what UModel exported.

``--carrier-mesh`` swaps the rig's geometry for a single minimal skinned
triangle.  An animation set only lends clips to a body that already has its own
geometry, so shipping the rig's mesh is dead weight -- and UModel's export of
some rigs carries blend indices outside the palette the converter builds, which
the runtime rejects outright ("A skinned vertex contains invalid bone data").
The existing Esther animation sets are built the same way, one submesh each.
The skin is left alone, so the skeleton and the clips are unaffected.

Run this on the raw rig glTF, before ``build_umodel_gltf_psa.py`` injects clips.

Usage:
  python align_rig_gltf.py --gltf <rig.gltf> --body <body.wmodel>
                           --output <aligned.gltf> [--carrier-mesh]
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import struct
import sys

FILE_HEADER_SIZE = 16
MODEL_META_SIZE = 32
SECTION_DESC_SIZE = 64
SKELETON_META_SIZE = 32
SKELETON_BONE_NODE_SIZE = 256
SECTION_SKELETON = 3

COMPONENT_FLOAT = 5126
COMPONENT_UNSIGNED_SHORT = 5123
CARRIER_EDGE_METRES = 0.01


def append_view(document: dict, payload: bytearray, blob: bytes, target: int | None) -> int:
    payload.extend(b"\0" * ((-len(payload)) & 3))
    view = {"buffer": 0, "byteOffset": len(payload), "byteLength": len(blob)}
    if target is not None:
        view["target"] = target
    document.setdefault("bufferViews", []).append(view)
    payload.extend(blob)
    return len(document["bufferViews"]) - 1


def append_accessor(
    document: dict,
    view: int,
    component_type: int,
    count: int,
    accessor_type: str,
    minimum: list | None = None,
    maximum: list | None = None,
) -> int:
    accessor = {
        "bufferView": view,
        "componentType": component_type,
        "count": count,
        "type": accessor_type,
    }
    if minimum is not None:
        accessor["min"] = minimum
        accessor["max"] = maximum
    document.setdefault("accessors", []).append(accessor)
    return len(document["accessors"]) - 1


def build_carrier_mesh(document: dict, payload: bytearray) -> int:
    """One triangle weighted entirely to the first joint, replacing every mesh."""
    edge = CARRIER_EDGE_METRES
    positions = [(0.0, 0.0, 0.0), (edge, 0.0, 0.0), (0.0, edge, 0.0)]
    normals = [(0.0, 0.0, 1.0)] * 3
    texcoords = [(0.0, 0.0), (1.0, 0.0), (0.0, 1.0)]

    position_view = append_view(
        document, payload, b"".join(struct.pack("<3f", *p) for p in positions), 34962
    )
    normal_view = append_view(
        document, payload, b"".join(struct.pack("<3f", *n) for n in normals), 34962
    )
    texcoord_view = append_view(
        document, payload, b"".join(struct.pack("<2f", *t) for t in texcoords), 34962
    )
    joint_view = append_view(
        document, payload, struct.pack("<4H", 0, 0, 0, 0) * 3, 34962
    )
    weight_view = append_view(
        document, payload, struct.pack("<4f", 1.0, 0.0, 0.0, 0.0) * 3, 34962
    )
    index_view = append_view(document, payload, struct.pack("<3H", 0, 1, 2), 34963)

    attributes = {
        "POSITION": append_accessor(
            document,
            position_view,
            COMPONENT_FLOAT,
            3,
            "VEC3",
            [0.0, 0.0, 0.0],
            [edge, edge, 0.0],
        ),
        "NORMAL": append_accessor(document, normal_view, COMPONENT_FLOAT, 3, "VEC3"),
        "TEXCOORD_0": append_accessor(
            document, texcoord_view, COMPONENT_FLOAT, 3, "VEC2"
        ),
        "JOINTS_0": append_accessor(
            document, joint_view, COMPONENT_UNSIGNED_SHORT, 3, "VEC4"
        ),
        "WEIGHTS_0": append_accessor(document, weight_view, COMPONENT_FLOAT, 3, "VEC4"),
    }
    indices = append_accessor(
        document, index_view, COMPONENT_UNSIGNED_SHORT, 3, "SCALAR"
    )
    document["meshes"] = [
        {"name": "carrier", "primitives": [{"attributes": attributes, "indices": indices}]}
    ]
    return 0


def read_body_skeleton(path: Path) -> list[tuple[str, int]]:
    data = path.read_bytes()
    magic, _major, _minor, _flags, _size = struct.unpack_from("<4sHHII", data, 0)
    if magic != b"WINT":
        raise SystemExit(f"{path} is not a WModel package")
    model_at = FILE_HEADER_SIZE
    model_magic, section_count, _animations, _model_flags = struct.unpack_from(
        "<4sIII", data, model_at
    )
    if model_magic != b"WMOD":
        raise SystemExit(f"{path} has no WMOD metadata")
    base = None
    table = model_at + MODEL_META_SIZE
    for index in range(section_count):
        desc_at = table + index * SECTION_DESC_SIZE
        section_type, _index, offset, _size = struct.unpack_from(
            "<IIQQ", data, desc_at
        )
        if section_type == SECTION_SKELETON:
            base = FILE_HEADER_SIZE + offset + FILE_HEADER_SIZE
            break
    if base is None:
        raise SystemExit(f"{path} has no skeleton section")
    magic, bone_count, _sockets = struct.unpack_from("<4sII", data, base)
    if magic != b"WSKL":
        raise SystemExit(f"{path} skeleton section is not WSKL")
    bones = []
    for index in range(bone_count):
        at = base + SKELETON_META_SIZE + index * SKELETON_BONE_NODE_SIZE
        _hash, raw_name = struct.unpack_from("<Q64s", data, at)
        parent = struct.unpack_from("<i", data, at + 72)[0]
        bones.append((raw_name.split(b"\0")[0].decode("ascii", "replace"), parent))
    return bones


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--gltf", required=True, type=Path)
    parser.add_argument("--body", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument(
        "--carrier-mesh",
        action="store_true",
        help="Replace the rig geometry with one minimal skinned triangle.",
    )
    parser.add_argument("--overwrite", action="store_true")
    args = parser.parse_args()

    if args.output.exists() and not args.overwrite:
        raise SystemExit(f"Output already exists: {args.output}")

    document = json.loads(args.gltf.read_text(encoding="utf-8"))
    nodes = document.get("nodes")
    skins = document.get("skins")
    scenes = document.get("scenes")
    if not isinstance(nodes, list) or not isinstance(skins, list) or len(skins) != 1:
        raise SystemExit("Expected a UModel glTF with exactly one skin")
    if document.get("animations"):
        raise SystemExit("Run this before clips are injected")

    joints = skins[0]["joints"]
    joint_names = [nodes[i].get("name") for i in joints]

    parented = {child for node in nodes for child in node.get("children", [])}
    roots = [i for i in range(len(nodes)) if i not in parented]
    if len(roots) != 1:
        raise SystemExit(f"Expected one glTF root node, found {len(roots)}")
    root_index = roots[0]
    root = nodes[root_index]
    if "mesh" not in root:
        raise SystemExit("Expected the UModel root node to carry the mesh")

    body = read_body_skeleton(args.body)
    body_names = [name for name, _ in body]
    wrappers = [
        (index, name) for index, name in enumerate(body_names) if name not in set(joint_names)
    ]
    if len(wrappers) != 3 or wrappers[0][1] != "RootNode" or wrappers[0][0] != 0:
        raise SystemExit(
            "Body skeleton does not have the expected RootNode plus two wrapper "
            f"nodes: {[name for _, name in wrappers]}"
        )
    if len(body_names) != len(joint_names) + 3:
        raise SystemExit(
            f"Body has {len(body_names)} bones but the rig has {len(joint_names)} joints"
        )

    # Whichever wrapper is the parent of the joint subtree is the armature; the
    # other one is the mesh node.  Their order under RootNode is the order the
    # body's own bone table lists them in.
    joint_root_index = body_names.index(joint_names[0])
    armature_parent = body[joint_root_index][1]
    armature_name = body_names[armature_parent]
    mesh_name = next(
        name for index, name in wrappers[1:] if name != armature_name
    )
    ordered = [name for _, name in wrappers[1:]]

    buffer_uri = document["buffers"][0]["uri"]
    payload = bytearray((args.gltf.parent / buffer_uri).read_bytes())
    mesh_index = root.pop("mesh")
    if args.carrier_mesh:
        mesh_index = build_carrier_mesh(document, payload)
    mesh_node = {"name": mesh_name, "mesh": mesh_index}
    if "skin" in root:
        mesh_node["skin"] = root.pop("skin")
    else:
        mesh_node["skin"] = 0
    nodes.append(mesh_node)
    mesh_index = len(nodes) - 1

    root["name"] = armature_name
    child_index = {armature_name: root_index, mesh_name: mesh_index}
    nodes.append({"name": "RootNode", "children": [child_index[n] for n in ordered]})
    new_root = len(nodes) - 1
    scenes[document.get("scene", 0)]["nodes"] = [new_root]

    document["buffers"][0]["byteLength"] = len(payload)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(document), encoding="utf-8")
    (args.output.parent / buffer_uri).write_bytes(bytes(payload))

    print(
        f"{args.output}: root=RootNode children={ordered} "
        f"armature={armature_name} mesh={mesh_name} joints={len(joint_names)} "
        f"carrier={args.carrier_mesh}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
