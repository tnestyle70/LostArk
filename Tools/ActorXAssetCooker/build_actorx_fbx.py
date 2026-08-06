"""Build an animation-bearing FBX from an ActorX PSK and one or more PSA files.

Run this script through Blender, not through the system Python::

    blender.exe --background --factory-startup --python build_actorx_fbx.py -- \
        --psk Model.psk --psa Anim.psa --output-fbx Model.fbx \
        --report Model.actorx.json

The script intentionally does not know any game asset names.  It uses the same
direct PSK/PSA importer API and FBX export settings as the previously verified
Backrooms asset build, while making every input and output explicit.
"""

import argparse
import hashlib
import importlib.util
import json
import math
import os
from pathlib import Path
import struct
import sys
import traceback


bpy = None


def script_arguments():
    """Return only arguments intended for this script."""
    if "--" in sys.argv:
        return sys.argv[sys.argv.index("--") + 1 :]
    return sys.argv[1:]


def build_parser():
    parser = argparse.ArgumentParser(
        description=(
            "Import an ActorX PSK and PSA files in Blender and export one FBX "
            "containing the skeleton and every imported Action."
        )
    )
    parser.add_argument("--psk", required=True, help="Input skeletal mesh PSK file")
    parser.add_argument(
        "--psa",
        action="append",
        required=True,
        help="Input PSA file; repeat this option for multiple files",
    )
    parser.add_argument("--output-fbx", required=True, help="Output FBX path")
    parser.add_argument("--report", required=True, help="UTF-8 JSON report path")
    parser.add_argument(
        "--output-blend",
        help="Optional .blend snapshot for visual inspection",
    )
    parser.add_argument(
        "--addon",
        help=(
            "PSK/PSA importer .py or package path. If omitted, the script checks "
            "ACTORX_BLENDER_ADDON and Blender user addon folders."
        ),
    )
    parser.add_argument(
        "--preferred-action",
        help="Exact Action name to leave assigned to the Armature before export",
    )
    parser.add_argument(
        "--prefix-actions-with-source",
        action="store_true",
        help="Ask the importer to prefix Action names with the PSA file name",
    )
    parser.add_argument(
        "--no-scale-down",
        action="store_false",
        dest="scale_down",
        help="Disable the proven ActorX importer scale-down setting",
    )
    parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Explicitly allow replacing direct-script output files",
    )
    parser.set_defaults(scale_down=True)
    return parser


def resolve_input_file(value, label):
    path = Path(value).expanduser().resolve()
    if not path.is_file():
        raise RuntimeError("{} does not exist: {}".format(label, path))
    return path


def resolve_output_file(value, label, overwrite):
    path = Path(value).expanduser().resolve()
    if path.exists() and not overwrite:
        raise RuntimeError(
            "{} already exists; use --overwrite only for a staging output: {}".format(
                label, path
            )
        )
    path.parent.mkdir(parents=True, exist_ok=True)
    return path


def decode_actorx_name(value):
    return value.split(b"\0", 1)[0].decode("utf-8", errors="replace")


def read_psa_metadata(psa_path):
    """Read and validate the PSA skeleton, sequence table, and scale tracks."""
    chunk_header = struct.Struct("<20siii")
    bone_record_size = 120
    anim_info = struct.Struct("<64s64s4i3f3i")
    scale_key = struct.Struct("<4f")
    bones = None
    bone_parents = None
    sequences = []
    animation_key_count = None
    scale_keys = None

    with psa_path.open("rb") as stream:
        while True:
            header_bytes = stream.read(chunk_header.size)
            if not header_bytes:
                break
            if len(header_bytes) != chunk_header.size:
                raise RuntimeError("Truncated PSA chunk header: {}".format(psa_path))

            chunk_id_bytes, _type_flag, data_size, data_count = chunk_header.unpack(
                header_bytes
            )
            chunk_id = decode_actorx_name(chunk_id_bytes)
            if data_size < 0 or data_count < 0:
                raise RuntimeError(
                    "Invalid PSA chunk dimensions: {} ({})".format(psa_path, chunk_id)
                )
            payload_size = data_size * data_count
            payload_start = stream.tell()
            stream.seek(0, os.SEEK_END)
            remaining = stream.tell() - payload_start
            stream.seek(payload_start, os.SEEK_SET)
            if payload_size > remaining:
                raise RuntimeError(
                    "Truncated PSA chunk payload: {} ({})".format(psa_path, chunk_id)
                )

            payload = stream.read(payload_size)
            if len(payload) != payload_size:
                raise RuntimeError(
                    "Truncated PSA chunk payload: {} ({})".format(psa_path, chunk_id)
                )

            if chunk_id == "BONENAMES":
                if bones is not None:
                    raise RuntimeError("PSA contains duplicate BONENAMES chunks: {}".format(psa_path))
                if data_size != bone_record_size:
                    raise RuntimeError(
                        "Unsupported PSA BONENAMES record size: {} (expected {}, got {})".format(
                            psa_path, bone_record_size, data_size
                        )
                    )
                bones = [
                    decode_actorx_name(payload[index * data_size : index * data_size + 64])
                    for index in range(data_count)
                ]
                bone_parents = [
                    struct.unpack_from("<i", payload, index * data_size + 72)[0]
                    for index in range(data_count)
                ]
            elif chunk_id in ("ANIMINFO", "ANIMINFO_BINARY"):
                if data_size != anim_info.size:
                    raise RuntimeError(
                        "Unsupported PSA ANIMINFO record size: {} (expected {}, got {})".format(
                            psa_path, anim_info.size, data_size
                        )
                    )
                for _ in range(data_count):
                    record_offset = _ * data_size
                    record = payload[record_offset : record_offset + data_size]
                    (
                        name_bytes,
                        group_bytes,
                        total_bones,
                        root_include,
                        key_compression_style,
                        key_quotum,
                        key_reduction,
                        track_time,
                        animation_rate,
                        start_bone,
                        first_raw_frame,
                        raw_frame_count,
                    ) = anim_info.unpack(record)
                    if (
                        not math.isfinite(animation_rate)
                        or not (animation_rate > 0.0)
                        or not math.isfinite(track_time)
                        or not (track_time > 0.0)
                        or raw_frame_count <= 0
                    ):
                        raise RuntimeError(
                            "PSA sequence has invalid timing: {} ({})".format(
                                psa_path, decode_actorx_name(name_bytes)
                            )
                        )
                    sequences.append(
                        {
                            "name": decode_actorx_name(name_bytes),
                            "group": decode_actorx_name(group_bytes),
                            "total_bones": total_bones,
                            "root_include": root_include,
                            "key_compression_style": key_compression_style,
                            "key_quotum": key_quotum,
                            "key_reduction": float(key_reduction),
                            "track_time": float(track_time),
                            "animation_rate": float(animation_rate),
                            "start_bone": start_bone,
                            "first_raw_frame": first_raw_frame,
                            "raw_frame_count": raw_frame_count,
                        }
                    )
            elif chunk_id == "ANIMKEYS":
                if animation_key_count is not None:
                    raise RuntimeError("PSA contains duplicate ANIMKEYS chunks: {}".format(psa_path))
                animation_key_count = data_count
            elif chunk_id == "SCALEKEYS":
                if scale_keys is not None:
                    raise RuntimeError("PSA contains duplicate SCALEKEYS chunks: {}".format(psa_path))
                if data_size != scale_key.size:
                    raise RuntimeError(
                        "Unsupported PSA SCALEKEYS record size: {} (expected {}, got {})".format(
                            psa_path, scale_key.size, data_size
                        )
                    )
                scale_keys = [
                    scale_key.unpack_from(payload, index * data_size)
                    for index in range(data_count)
                ]

    if bones is None or not bones:
        raise RuntimeError("PSA contains no BONENAMES skeleton: {}".format(psa_path))
    if any(not name for name in bones) or len(set(bones)) != len(bones):
        raise RuntimeError("PSA bone names must be non-empty and unique: {}".format(psa_path))
    for index, parent_index in enumerate(bone_parents):
        if parent_index < -1 or parent_index >= index:
            raise RuntimeError(
                "PSA bone hierarchy is not parent-before-child: {} (bone {})".format(
                    psa_path, bones[index]
                )
            )
    if not sequences:
        raise RuntimeError("PSA contains no ANIMINFO sequences: {}".format(psa_path))

    maximum_frame = 0
    expected_first_frame = 0
    for sequence in sequences:
        if sequence["total_bones"] != len(bones) or sequence["start_bone"] != 0:
            raise RuntimeError(
                "Partial or mismatched PSA skeleton tracks are not supported: {} ({})".format(
                    psa_path, sequence["name"]
                )
            )
        if not math.isclose(
            sequence["track_time"],
            float(sequence["raw_frame_count"]),
            abs_tol=0.0001,
        ):
            raise RuntimeError(
                "Unsupported PSA TrackTime convention: {} ({}, track {}, frames {})".format(
                    psa_path,
                    sequence["name"],
                    sequence["track_time"],
                    sequence["raw_frame_count"],
                )
            )
        if sequence["first_raw_frame"] < 0:
            raise RuntimeError(
                "PSA sequence has a negative FirstRawFrame: {} ({})".format(
                    psa_path, sequence["name"]
                )
            )
        if sequence["first_raw_frame"] != expected_first_frame:
            raise RuntimeError(
                "PSA sequence raw-frame ranges must be contiguous and ordered: {} ({}, expected {}, got {})".format(
                    psa_path,
                    sequence["name"],
                    expected_first_frame,
                    sequence["first_raw_frame"],
                )
            )
        expected_first_frame += sequence["raw_frame_count"]
        maximum_frame = max(
            maximum_frame,
            sequence["first_raw_frame"] + sequence["raw_frame_count"],
        )

    expected_key_count = maximum_frame * len(bones)
    if animation_key_count != expected_key_count:
        raise RuntimeError(
            "PSA ANIMKEYS count does not cover every sequence and bone: {} (expected {}, got {})".format(
                psa_path, expected_key_count, animation_key_count
            )
        )
    if scale_keys is not None and len(scale_keys) != expected_key_count:
        raise RuntimeError(
            "PSA SCALEKEYS count does not cover every sequence and bone: {} (expected {}, got {})".format(
                psa_path, expected_key_count, len(scale_keys)
            )
        )
    if scale_keys is not None:
        for values in scale_keys:
            if not all(math.isfinite(value) for value in values):
                raise RuntimeError("PSA contains a non-finite scale key: {}".format(psa_path))
            if not math.isclose(values[3], 1.0, abs_tol=0.0001):
                raise RuntimeError(
                    "Unsupported PSA SCALEKEYS time convention: {} ({})".format(
                        psa_path, values[3]
                    )
                )

    for sequence in sequences:
        sequence["source_has_scale_keys"] = scale_keys is not None
        sequence["source_scale_key_count"] = (
            sequence["raw_frame_count"] * len(bones) if scale_keys is not None else 0
        )
        if scale_keys is not None:
            values = []
            for frame_index in range(sequence["raw_frame_count"]):
                source_frame = sequence["first_raw_frame"] + frame_index
                frame_start = source_frame * len(bones)
                values.extend(scale_keys[frame_start : frame_start + len(bones)])
            sequence["source_scale_minimum"] = [
                min(value[axis] for value in values) for axis in range(3)
            ]
            sequence["source_scale_maximum"] = [
                max(value[axis] for value in values) for axis in range(3)
            ]
            sequence["source_scale_bones"] = []
            for bone_index, bone_name in enumerate(bones):
                bone_values = [
                    scale_keys[
                        (sequence["first_raw_frame"] + frame_index) * len(bones)
                        + bone_index
                    ]
                    for frame_index in range(sequence["raw_frame_count"])
                ]
                sequence["source_scale_bones"].append(
                    {
                        "name": bone_name,
                        "minimum": [
                            min(value[axis] for value in bone_values)
                            for axis in range(3)
                        ],
                        "maximum": [
                            max(value[axis] for value in bone_values)
                            for axis in range(3)
                        ],
                    }
                )
        else:
            sequence["source_scale_minimum"] = None
            sequence["source_scale_maximum"] = None
            sequence["source_scale_bones"] = []

    return {
        "bones": bones,
        "bone_parents": bone_parents,
        "bone_count": len(bones),
        "animation_key_count": animation_key_count,
        "scale_key_count": len(scale_keys) if scale_keys is not None else 0,
        "scale_keys": scale_keys,
        "sequences": sequences,
    }


def configure_scene_frame_rate(psa_metadata):
    rates = {
        round(sequence["animation_rate"], 6)
        for source in psa_metadata
        for sequence in source["sequences"]
    }
    if len(rates) != 1:
        raise RuntimeError(
            "All PSA sequences in one FBX must share one animation rate; got {}".format(
                ", ".join(str(value) for value in sorted(rates))
            )
        )

    source_rate = next(iter(rates))
    # Blender stores an integer FPS and a floating base. Keep the numerator
    # small and choose the base so fps / fps_base equals the PSA AnimRate.
    fps = max(1, int(round(source_rate)))
    fps_base = float(fps) / float(source_rate)

    if fps <= 0 or fps > 32767:
        raise RuntimeError("PSA animation rate is outside Blender FPS limits: {}".format(source_rate))
    bpy.context.scene.render.fps = fps
    bpy.context.scene.render.fps_base = fps_base
    return {
        "source_fps": source_rate,
        "scene_fps": fps,
        "scene_fps_base": fps_base,
        "effective_fps": float(fps) / float(fps_base),
    }


def match_action_sequence(action, sequences):
    candidates = []
    for sequence in sequences:
        labels = [sequence["name"]]
        if sequence["group"]:
            labels.append("({}) {}".format(sequence["group"], sequence["name"]))
        matched_lengths = [
            len(label)
            for label in labels
            if action.name == label
            or (
                action.name.endswith(label)
                and len(action.name) > len(label)
                and action.name[-len(label) - 1] in " ._|-"
            )
        ]
        if matched_lengths:
            candidates.append((max(matched_lengths), sequence))

    if not candidates:
        raise RuntimeError(
            "Imported Action does not match a PSA sequence: {}".format(action.name)
        )
    best_length = max(length for length, _sequence in candidates)
    best = [sequence for length, sequence in candidates if length == best_length]
    if len(best) != 1:
        raise RuntimeError(
            "Imported Action ambiguously matches PSA sequences: {}".format(action.name)
        )
    sequence = best[0]

    imported_frame_span = float(action.frame_range[1] - action.frame_range[0])
    expected_frame_span = float(sequence["raw_frame_count"] - 1)
    if not math.isclose(imported_frame_span, expected_frame_span, abs_tol=0.0001):
        raise RuntimeError(
            "Action frame span differs from PSA. action={} imported={} source={}".format(
                action.name, imported_frame_span, expected_frame_span
            )
        )
    return sequence


def addon_candidates(explicit_path):
    if explicit_path:
        yield Path(explicit_path).expanduser()

    environment_path = os.environ.get("ACTORX_BLENDER_ADDON")
    if environment_path:
        yield Path(environment_path).expanduser()

    appdata = os.environ.get("APPDATA")
    if appdata:
        blender_root = Path(appdata) / "Blender Foundation" / "Blender"
        version = "{}.{}".format(bpy.app.version[0], bpy.app.version[1])
        version_roots = [blender_root / version]
        try:
            if blender_root.is_dir():
                version_roots.extend(
                    sorted(
                        (item for item in blender_root.iterdir() if item.is_dir()),
                        key=lambda item: item.name,
                        reverse=True,
                    )
                )
        except OSError:
            # A managed/sandboxed Blender process may be unable to enumerate
            # APPDATA even when an explicit addon path is normally valid. The
            # caller still receives the full checked-path list below.
            pass

        seen = set()
        for version_root in version_roots:
            resolved = str(version_root.resolve())
            if resolved in seen:
                continue
            seen.add(resolved)
            addon_root = version_root / "scripts" / "addons"
            yield addon_root / "io_import_scene_unreal_psa_psk_280.py"
            yield addon_root / "io_scene_psk_psa" / "__init__.py"

    try:
        user_addons = Path(bpy.utils.user_resource("SCRIPTS", path="addons"))
        yield user_addons / "io_import_scene_unreal_psa_psk_280.py"
        yield user_addons / "io_scene_psk_psa" / "__init__.py"
    except (TypeError, ValueError):
        pass


def normalize_addon_path(candidate):
    path = candidate.resolve()
    if path.is_file():
        return path
    if path.is_dir():
        for child_name in (
            "io_import_scene_unreal_psa_psk_280.py",
            "__init__.py",
        ):
            child = path / child_name
            if child.is_file():
                return child
    return None


def locate_addon(explicit_path):
    checked = []
    for candidate in addon_candidates(explicit_path):
        checked.append(str(candidate))
        try:
            normalized = normalize_addon_path(candidate)
        except OSError:
            normalized = None
        if normalized is not None:
            return normalized

    raise RuntimeError(
        "PSK/PSA importer was not found. Pass --addon or set "
        "ACTORX_BLENDER_ADDON. Checked:\n  " + "\n  ".join(checked)
    )


def load_addon(addon_path):
    module_key = hashlib.sha1(str(addon_path).encode("utf-8")).hexdigest()[:12]
    module_name = "actorx_importer_{}".format(module_key)
    kwargs = {}
    if addon_path.name == "__init__.py":
        kwargs["submodule_search_locations"] = [str(addon_path.parent)]
    spec = importlib.util.spec_from_file_location(
        module_name, str(addon_path), **kwargs
    )
    if spec is None or spec.loader is None:
        raise RuntimeError("Unable to load importer module: {}".format(addon_path))
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)

    if not callable(getattr(module, "pskimport", None)) or not callable(
        getattr(module, "psaimport", None)
    ):
        raise RuntimeError(
            "Importer does not expose the proven pskimport/psaimport API: {}".format(
                addon_path
            )
        )
    return module


def clear_scene():
    active = bpy.context.object
    if active is not None and active.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for action in list(bpy.data.actions):
        bpy.data.actions.remove(action)


def find_single_armature():
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    if len(armatures) != 1:
        raise RuntimeError(
            "Expected exactly one Armature after PSK import, found {}".format(
                len(armatures)
            )
        )
    return armatures[0]


def find_meshes():
    return [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]


def import_psk(addon, psk_path, scale_down):
    result = addon.pskimport(
        filepath=str(psk_path),
        context=bpy.context,
        bImportmesh=True,
        bImportbone=True,
        bSpltiUVdata=False,
        fBonesize=5.0,
        fBonesizeRatio=0.6,
        bDontInvertRoot=True,
        bReorientBones=False,
        bReorientDirectly=False,
        bScaleDown=scale_down,
        bToSRGB=True,
        error_callback=print,
    )
    if result is False:
        raise RuntimeError("PSK importer reported failure: {}".format(psk_path))


def ensure_skin_links(armature, meshes):
    total_weighted_vertices = 0
    for mesh in meshes:
        weighted_vertices = sum(1 for vertex in mesh.data.vertices if vertex.groups)
        total_weighted_vertices += weighted_vertices

        armature_modifiers = [
            modifier for modifier in mesh.modifiers if modifier.type == "ARMATURE"
        ]
        if not armature_modifiers:
            modifier = mesh.modifiers.new(name="Armature", type="ARMATURE")
            modifier.object = armature
        elif all(modifier.object is None for modifier in armature_modifiers):
            armature_modifiers[0].object = armature

    if total_weighted_vertices == 0:
        raise RuntimeError("PSK mesh has no weighted vertices; skeletal FBX would be invalid")


def validate_psa_skeleton(armature, psa_path, source_metadata):
    imported_bones = [bone.name for bone in armature.data.bones]
    source_bones = source_metadata["bones"]
    if imported_bones != source_bones:
        mismatch = min(len(imported_bones), len(source_bones))
        for index, (imported, source) in enumerate(zip(imported_bones, source_bones)):
            if imported != source:
                mismatch = index
                break
        imported_name = imported_bones[mismatch] if mismatch < len(imported_bones) else "<missing>"
        source_name = source_bones[mismatch] if mismatch < len(source_bones) else "<missing>"
        raise RuntimeError(
            "PSK Armature and PSA BONENAMES differ at index {}: {} (PSK={!r}, PSA={!r}, counts={}/{})".format(
                mismatch,
                psa_path,
                imported_name,
                source_name,
                len(imported_bones),
                len(source_bones),
            )
        )

def inject_psa_scale_curves(action, armature, source_metadata, sequence):
    scale_keys = source_metadata["scale_keys"]
    if scale_keys is None:
        return {
            "scale_fcurve_count": 0,
            "scale_keyframe_count": 0,
            "action_scale_minimum": None,
            "action_scale_maximum": None,
        }

    bones = source_metadata["bones"]
    frame_count = sequence["raw_frame_count"]
    action_start = float(action.frame_range[0])
    scale_paths = {
        armature.pose.bones[bone_name].path_from_id("scale") for bone_name in bones
    }
    for curve in list(action.fcurves):
        if curve.data_path in scale_paths:
            action.fcurves.remove(curve)

    imported_values = [[], [], []]
    for bone_index, bone_name in enumerate(bones):
        data_path = armature.pose.bones[bone_name].path_from_id("scale")
        for axis in range(3):
            curve = action.fcurves.new(
                data_path=data_path,
                index=axis,
                action_group=bone_name,
            )
            curve.keyframe_points.add(frame_count)
            for frame_index, point in enumerate(curve.keyframe_points):
                source_frame = sequence["first_raw_frame"] + frame_index
                source_index = source_frame * len(bones) + bone_index
                value = float(scale_keys[source_index][axis])
                point.co = (action_start + float(frame_index), value)
                point.interpolation = "LINEAR"
                imported_values[axis].append(value)
            curve.update()

    scale_curves = [curve for curve in action.fcurves if curve.data_path in scale_paths]
    expected_curve_count = len(bones) * 3
    expected_keyframe_count = expected_curve_count * frame_count
    actual_keyframe_count = sum(len(curve.keyframe_points) for curve in scale_curves)
    if len(scale_curves) != expected_curve_count or actual_keyframe_count != expected_keyframe_count:
        raise RuntimeError(
            "PSA scale curve injection is incomplete: {} (curves {}/{}, keys {}/{})".format(
                action.name,
                len(scale_curves),
                expected_curve_count,
                actual_keyframe_count,
                expected_keyframe_count,
            )
        )

    minimum = [min(values) for values in imported_values]
    maximum = [max(values) for values in imported_values]
    for axis in range(3):
        if not math.isclose(
            minimum[axis], sequence["source_scale_minimum"][axis], abs_tol=1e-7
        ) or not math.isclose(
            maximum[axis], sequence["source_scale_maximum"][axis], abs_tol=1e-7
        ):
            raise RuntimeError(
                "Injected scale extrema differ from PSA: {} axis={}".format(
                    action.name, axis
                )
            )

    action.update_tag()
    return {
        "scale_fcurve_count": len(scale_curves),
        "scale_keyframe_count": actual_keyframe_count,
        "action_scale_minimum": minimum,
        "action_scale_maximum": maximum,
    }


def import_psa(addon, armature, psa_path, scale_down, prefix_actions):
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature

    before = {action.as_pointer() for action in bpy.data.actions}
    result = addon.psaimport(
        filepath=str(psa_path),
        context=bpy.context,
        oArmature=armature,
        bFilenameAsPrefix=prefix_actions,
        bActionsToTrack=False,
        first_frames=0,
        bDontInvertRoot=True,
        bUpdateTimelineRange=False,
        bRotationOnly=False,
        bScaleDown=scale_down,
        fcurve_interpolation="LINEAR",
        bBoneNameCaseSensitiveCmp=True,
        error_callback=print,
    )
    if result is False:
        raise RuntimeError("PSA importer reported failure: {}".format(psa_path))

    created = [
        action for action in bpy.data.actions if action.as_pointer() not in before
    ]
    if not created:
        raise RuntimeError("No Action was imported from PSA: {}".format(psa_path))
    for action in created:
        if len(action.fcurves) == 0:
            raise RuntimeError(
                "Imported Action has no animation curves: {} ({})".format(
                    action.name, psa_path
                )
            )
        action.use_fake_user = True
    return created


def assign_action(armature, imported_actions, preferred_name):
    if preferred_name:
        selected = next(
            (action for action in imported_actions if action.name == preferred_name),
            None,
        )
        if selected is None:
            available = ", ".join(action.name for action in imported_actions)
            raise RuntimeError(
                "Preferred Action was not imported: {}. Available: {}".format(
                    preferred_name, available
                )
            )
    else:
        selected = imported_actions[0]

    armature.animation_data_create()
    armature.animation_data.action = selected
    bpy.context.scene.frame_start = int(selected.frame_range[0])
    bpy.context.scene.frame_end = max(
        int(selected.frame_range[0]), int(selected.frame_range[1])
    )
    bpy.context.scene.frame_set(bpy.context.scene.frame_start)
    return selected


def select_export_objects(armature, meshes):
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature
    for mesh in meshes:
        mesh.select_set(True)


def export_fbx(output_fbx):
    result = bpy.ops.export_scene.fbx(
        filepath=str(output_fbx),
        check_existing=False,
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        apply_unit_scale=True,
        bake_space_transform=False,
        add_leaf_bones=False,
        primary_bone_axis="Y",
        secondary_bone_axis="X",
        use_armature_deform_only=False,
        bake_anim=True,
        bake_anim_use_all_actions=True,
        bake_anim_use_nla_strips=False,
        bake_anim_force_startend_keying=True,
        bake_anim_step=1.0,
        bake_anim_simplify_factor=0.0,
        path_mode="COPY",
        embed_textures=False,
        axis_forward="-Z",
        axis_up="Y",
    )
    if "FINISHED" not in result or not output_fbx.is_file():
        raise RuntimeError("Blender did not create the requested FBX: {}".format(output_fbx))
    if output_fbx.stat().st_size == 0:
        raise RuntimeError("Blender created an empty FBX: {}".format(output_fbx))


def file_sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def mesh_report(mesh, armature):
    weighted_vertices = sum(1 for vertex in mesh.data.vertices if vertex.groups)
    armature_links = [
        modifier.object.name if modifier.object is not None else None
        for modifier in mesh.modifiers
        if modifier.type == "ARMATURE"
    ]
    return {
        "name": mesh.name,
        "vertex_count": len(mesh.data.vertices),
        "polygon_count": len(mesh.data.polygons),
        "weighted_vertex_count": weighted_vertices,
        "materials": [
            material.name if material is not None else None
            for material in mesh.data.materials
        ],
        "parent": mesh.parent.name if mesh.parent is not None else None,
        "armature_modifiers": armature_links,
        "linked_to_expected_armature": (
            mesh.parent == armature or armature.name in armature_links
        ),
        "dimensions": [round(float(value), 8) for value in mesh.dimensions],
    }


def write_report(
    report_path,
    psk_path,
    psa_paths,
    addon_path,
    output_fbx,
    output_blend,
    armature,
    meshes,
    imported_records,
    psa_metadata,
    frame_rate,
    selected_action,
    scale_down,
    prefix_actions,
):
    actions = []
    for record in imported_records:
        action = record["action"]
        scale_result = record["scale_result"]
        actions.append(
            {
                "name": action.name,
                "source_psa": str(record["source_psa"]),
                "frame_start": float(action.frame_range[0]),
                "frame_end": float(action.frame_range[1]),
                "fcurve_count": len(action.fcurves),
                "source_animation_rate": record["sequence"]["animation_rate"],
                "source_raw_frame_count": record["sequence"]["raw_frame_count"],
                "source_has_scale_keys": record["sequence"]["source_has_scale_keys"],
                "source_scale_key_count": record["sequence"]["source_scale_key_count"],
                "source_scale_minimum": record["sequence"]["source_scale_minimum"],
                "source_scale_maximum": record["sequence"]["source_scale_maximum"],
                "source_scale_bones": record["sequence"]["source_scale_bones"],
                "scale_fcurve_count": scale_result["scale_fcurve_count"],
                "scale_keyframe_count": scale_result["scale_keyframe_count"],
                "action_scale_minimum": scale_result["action_scale_minimum"],
                "action_scale_maximum": scale_result["action_scale_maximum"],
            }
        )

    report_psa_metadata = []
    for source in psa_metadata:
        report_psa_metadata.append(
            {
                "path": source["path"],
                "bone_count": source["bone_count"],
                "bones": source["bones"],
                "bone_parents": source["bone_parents"],
                "animation_key_count": source["animation_key_count"],
                "scale_key_count": source["scale_key_count"],
                "sequences": source["sequences"],
            }
        )

    payload = {
        "schema": "LOSTARK_ACTORX_FBX_REPORT",
        "version": 2,
        "blender_version": bpy.app.version_string,
        "psk": str(psk_path),
        "psa": [str(path) for path in psa_paths],
        "psa_metadata": report_psa_metadata,
        "addon": str(addon_path),
        "output_fbx": str(output_fbx),
        "output_blend": str(output_blend) if output_blend is not None else None,
        "fbx_size": output_fbx.stat().st_size,
        "fbx_sha256": file_sha256(output_fbx),
        "armature": armature.name,
        "bone_count": len(armature.data.bones),
        "bones": [bone.name for bone in armature.data.bones],
        "mesh_count": len(meshes),
        "meshes": [mesh_report(mesh, armature) for mesh in meshes],
        "action_count": len(actions),
        "actions": actions,
        "selected_action": selected_action.name,
        "settings": {
            "scale_down": scale_down,
            "prefix_actions_with_source": prefix_actions,
            "add_leaf_bones": False,
            "bake_anim_use_all_actions": True,
            "bake_anim_use_nla_strips": False,
            "axis_forward": "-Z",
            "axis_up": "Y",
            "source_fps": frame_rate["source_fps"],
            "scene_fps": frame_rate["scene_fps"],
            "scene_fps_base": frame_rate["scene_fps_base"],
            "effective_fps": frame_rate["effective_fps"],
        },
    }
    report_path.write_text(
        json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8"
    )


def run(args):
    global bpy
    try:
        import bpy as blender_python
    except ImportError as error:
        raise RuntimeError(
            "This script must be run by Blender. Use blender.exe --background "
            "--python <script> -- <arguments>."
        ) from error
    bpy = blender_python

    psk_path = resolve_input_file(args.psk, "PSK")
    psa_paths = [resolve_input_file(value, "PSA") for value in args.psa]
    psa_metadata = []
    for path in psa_paths:
        source_metadata = read_psa_metadata(path)
        source_metadata["path"] = str(path)
        psa_metadata.append(source_metadata)
    output_fbx = resolve_output_file(
        args.output_fbx, "Output FBX", args.overwrite
    )
    report_path = resolve_output_file(args.report, "Report", args.overwrite)
    output_blend = None
    if args.output_blend:
        output_blend = resolve_output_file(
            args.output_blend, "Output Blend", args.overwrite
        )

    all_paths = [psk_path] + psa_paths + [output_fbx, report_path]
    if output_blend is not None:
        all_paths.append(output_blend)
    if len({str(path).casefold() for path in all_paths}) != len(all_paths):
        raise RuntimeError("Input and output paths must all be different")

    addon_path = locate_addon(args.addon)
    clear_scene()
    frame_rate = configure_scene_frame_rate(psa_metadata)
    addon = load_addon(addon_path)

    import_psk(addon, psk_path, args.scale_down)
    armature = find_single_armature()
    meshes = find_meshes()
    if not meshes:
        raise RuntimeError("No Mesh was created by the PSK importer")
    if len(armature.data.bones) == 0:
        raise RuntimeError("PSK Armature has no bones")
    ensure_skin_links(armature, meshes)
    for psa_path, source_metadata in zip(psa_paths, psa_metadata):
        validate_psa_skeleton(armature, psa_path, source_metadata)

    imported_records = []
    imported_actions = []
    for psa_path, source_metadata in zip(psa_paths, psa_metadata):
        created = import_psa(
            addon,
            armature,
            psa_path,
            args.scale_down,
            args.prefix_actions_with_source,
        )
        if len(created) != len(source_metadata["sequences"]):
            raise RuntimeError(
                "Imported Action count differs from PSA sequence count: {} ({} vs {})".format(
                    psa_path, len(created), len(source_metadata["sequences"])
                )
            )
        consumed_sequences = set()
        for action in created:
            sequence = match_action_sequence(action, source_metadata["sequences"])
            sequence_key = (sequence["group"], sequence["name"], sequence["first_raw_frame"])
            if sequence_key in consumed_sequences:
                raise RuntimeError(
                    "Multiple Actions consumed the same PSA sequence: {}".format(
                        sequence["name"]
                    )
                )
            consumed_sequences.add(sequence_key)
            scale_result = inject_psa_scale_curves(
                action, armature, source_metadata, sequence
            )
            imported_actions.append(action)
            imported_records.append(
                {
                    "action": action,
                    "source_psa": psa_path,
                    "sequence": sequence,
                    "scale_result": scale_result,
                }
            )
        if len(consumed_sequences) != len(source_metadata["sequences"]):
            raise RuntimeError("Not every PSA sequence was consumed: {}".format(psa_path))

    selected_action = assign_action(
        armature, imported_actions, args.preferred_action
    )
    select_export_objects(armature, meshes)

    if output_blend is not None:
        bpy.ops.wm.save_as_mainfile(filepath=str(output_blend), check_existing=False)
    export_fbx(output_fbx)
    write_report(
        report_path,
        psk_path,
        psa_paths,
        addon_path,
        output_fbx,
        output_blend,
        armature,
        meshes,
        imported_records,
        psa_metadata,
        frame_rate,
        selected_action,
        args.scale_down,
        args.prefix_actions_with_source,
    )

    print("ActorX FBX created: {}".format(output_fbx))
    print("Actions exported: {}".format(len(imported_actions)))
    print("Assigned Action: {}".format(selected_action.name))
    print("Report: {}".format(report_path))


def main():
    parser = build_parser()
    args = parser.parse_args(script_arguments())
    run(args)


if __name__ == "__main__":
    try:
        main()
    except SystemExit:
        raise
    except Exception:
        traceback.print_exc()
        sys.exit(1)
