"""Convert one Unreal ActorX PSK/PSA pair into an animation-bearing FBX.

Run this script through Blender, not the system Python::

    blender.exe --background --python build_actorx_fbx.py -- \
        --psk input.psk --psa input.psa --fbx output.fbx \
        --report output.json

The io_scene_psk_psa extension must be enabled in the selected Blender profile.
The script deliberately imports every PSA sequence and asks Blender's FBX exporter
to write every action.  ModelAssetConverter then cooks those FBX takes into the
single CModel/CMaterial WModel runtime path.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys

import bpy

from psk_psa_py.psa.reader import PsaReader
from psk_psa_py.psk.reader import read_psk_from_file
from bl_ext.user_default.io_scene_psk_psa.psa.importer import (
    PsaImportOptions,
    import_psa,
)
from bl_ext.user_default.io_scene_psk_psa.psk.importer import (
    PskImportOptions,
    import_psk,
)


def parse_args() -> argparse.Namespace:
    try:
        separator = sys.argv.index("--")
    except ValueError as error:
        raise SystemExit("Blender arguments must follow '--'.") from error

    parser = argparse.ArgumentParser()
    parser.add_argument("--psk", required=True, type=Path)
    parser.add_argument("--psa", required=True, type=Path)
    parser.add_argument("--fbx", required=True, type=Path)
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--blend", type=Path)
    parser.add_argument(
        "--scale",
        type=float,
        default=1.0,
        help="ActorX mesh and translation scale applied before FBX export.",
    )
    return parser.parse_args(sys.argv[separator + 1 :])


def decode_name(value: bytes | str) -> str:
    if isinstance(value, bytes):
        return value.decode("windows-1252").rstrip("\0 ")
    return str(value)


def make_runtime_action_names(
    sequence_names: list[str], section_prefix: str
) -> list[str]:
    """Fit stable action IDs into MODEL_SECTION_DESC.name[40]."""
    runtime_names: list[str] = []
    used: set[str] = set()
    prefix_size = len((section_prefix + "_").encode("utf-8"))
    action_budget = 39 - prefix_size
    if action_budget < 10:
        raise RuntimeError(
            "The FBX node name leaves no room for a stable animation ID: "
            + section_prefix
        )
    for sequence_name in sequence_names:
        encoded = sequence_name.encode("utf-8")
        if len(encoded) <= action_budget and sequence_name not in used:
            runtime_name = sequence_name
        else:
            suffix = "~" + hashlib.sha1(encoded).hexdigest()[:8]
            prefix_budget = action_budget - len(suffix)
            prefix = encoded[:prefix_budget]
            while True:
                try:
                    runtime_name = prefix.decode("utf-8") + suffix
                    break
                except UnicodeDecodeError:
                    prefix = prefix[:-1]
            if runtime_name in used:
                raise RuntimeError(
                    "Animation runtime-name collision: " + sequence_name
                )
        used.add(runtime_name)
        runtime_names.append(runtime_name)
    return runtime_names


def clear_scene() -> None:
    bpy.ops.object.mode_set.poll() and bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)


def main() -> None:
    args = parse_args()
    psk_path = args.psk.resolve(strict=True)
    psa_path = args.psa.resolve(strict=True)
    fbx_path = args.fbx.resolve()
    report_path = args.report.resolve()
    blend_path = args.blend.resolve() if args.blend else None

    fbx_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.parent.mkdir(parents=True, exist_ok=True)
    if blend_path:
        blend_path.parent.mkdir(parents=True, exist_ok=True)

    clear_scene()

    psk = read_psk_from_file(str(psk_path))
    psk_options = PskImportOptions()
    psk_options.scale = args.scale
    psk_options.should_import_materials = True
    psk_options.should_reuse_materials = False
    psk_result = import_psk(psk, bpy.context, psk_path.stem, psk_options)
    armature = psk_result.armature_object
    mesh = psk_result.mesh_object
    if armature is None or mesh is None:
        raise RuntimeError("The PSK did not produce both an armature and a mesh.")

    psa_stream = psa_path.open("rb")
    psa_reader = PsaReader(psa_stream)
    sequence_names = list(psa_reader.sequences.keys())
    if not sequence_names:
        raise RuntimeError("The PSA contains no animation sequences.")

    psk_bones = [decode_name(bone.name) for bone in psk.bones]
    psa_bones = [decode_name(bone.name) for bone in psa_reader.bones]
    missing_from_mesh = sorted(set(psa_bones).difference(psk_bones))
    if missing_from_mesh:
        raise RuntimeError(
            "PSA bones are missing from the PSK: " + ", ".join(missing_from_mesh)
        )

    psa_options = PsaImportOptions(
        sequence_names=sequence_names,
        fps_source="SEQUENCE",
        should_stash=False,
        should_use_config_file=False,
        should_use_fake_user=True,
        should_write_scale_keys=True,
        translation_scale=args.scale,
    )
    psa_result = import_psa(bpy.context, psa_reader, armature, psa_options)

    imported_actions = [bpy.data.actions[name] for name in sequence_names]
    if len(imported_actions) != len(sequence_names):
        raise RuntimeError("Not every PSA sequence produced a Blender action.")

    runtime_action_names = make_runtime_action_names(
        sequence_names, armature.name
    )
    for action, runtime_name in zip(imported_actions, runtime_action_names):
        action.name = runtime_name

    animation_data = armature.animation_data_create()
    animation_data.action = imported_actions[0]

    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    mesh.select_set(True)
    bpy.context.view_layer.objects.active = armature

    bpy.ops.export_scene.fbx(
        filepath=str(fbx_path),
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        apply_unit_scale=True,
        apply_scale_options="FBX_SCALE_UNITS",
        axis_forward="-Z",
        axis_up="Y",
        add_leaf_bones=False,
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=True,
        bake_anim_force_startend_keying=True,
        bake_anim_simplify_factor=0.0,
        path_mode="AUTO",
        embed_textures=False,
    )

    if blend_path:
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))

    sequences = []
    for name, runtime_name in zip(sequence_names, runtime_action_names):
        sequence = psa_reader.sequences[name]
        sequences.append(
            {
                "name": name,
                "runtimeName": runtime_name,
                "frames": int(sequence.frame_count),
                "fps": float(sequence.fps),
            }
        )

    report = {
        "schemaVersion": 1,
        "source": {
            "psk": str(psk_path),
            "psa": str(psa_path),
        },
        "output": {
            "fbx": str(fbx_path),
            "blend": str(blend_path) if blend_path else None,
        },
        "scale": args.scale,
        "mesh": {
            "name": mesh.name,
            "vertices": len(mesh.data.vertices),
            "polygons": len(mesh.data.polygons),
            "materials": [slot.material.name for slot in mesh.material_slots],
        },
        "skeleton": {
            "pskBoneCount": len(psk_bones),
            "psaBoneCount": len(psa_bones),
            "missingPsaBones": missing_from_mesh,
        },
        "animations": {
            "sequenceCount": len(sequences),
            "sequences": sequences,
        },
        "warnings": list(psk_result.warnings) + list(psa_result.warnings),
    }
    report_path.write_text(
        json.dumps(report, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    psa_stream.close()
    print(
        "ACTORX_FBX_OK "
        f"mesh={mesh.name} bones={len(psk_bones)} actions={len(sequences)} "
        f"fbx={fbx_path}"
    )


if __name__ == "__main__":
    main()
