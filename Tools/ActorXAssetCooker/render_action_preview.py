"""Render deterministic action-frame previews from an ActorX cook ``.blend``.

Run this script through Blender, not through the system Python::

    blender.exe --background --factory-startup \
        --python render_action_preview.py -- \
        --blend Model.blend --action-suffix ao_off \
        --frames 0 55 57 61 --output-dir preview

The camera is fitted once to the union of the evaluated mesh and armature
bounds for every requested frame.  Keeping one camera for the whole sequence
makes movement, break-up and scale-to-zero changes directly comparable.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import sys
import traceback


bpy = None
Vector = None


def script_arguments():
    """Return only arguments passed after Blender's ``--`` delimiter."""
    if "--" in sys.argv:
        return sys.argv[sys.argv.index("--") + 1 :]
    return sys.argv[1:]


def build_parser():
    parser = argparse.ArgumentParser(
        description=(
            "Open a Blender cook snapshot, select one Action, and render the "
            "requested frames using a bounds-fitted comparison camera."
        )
    )
    parser.add_argument("--blend", required=True, help="Input .blend snapshot")
    parser.add_argument(
        "--action-name",
        help="Exact, case-sensitive Action name",
    )
    parser.add_argument(
        "--action-suffix",
        help=(
            "Case-sensitive Action-name suffix. The suffix must resolve to "
            "exactly one Action."
        ),
    )
    parser.add_argument(
        "--frames",
        required=True,
        nargs="+",
        type=int,
        help="Integer action frames to render, for example: 0 55 57 61",
    )
    parser.add_argument(
        "--output-dir",
        required=True,
        help="Directory for PNG images and the default JSON report",
    )
    parser.add_argument(
        "--report",
        help="Optional JSON report path (default: <output-dir>/preview.json)",
    )
    parser.add_argument(
        "--armature",
        help="Exact Armature object name; required only for ambiguous scenes",
    )
    parser.add_argument(
        "--resolution-x",
        type=int,
        default=900,
        help="PNG width in pixels (default: 900)",
    )
    parser.add_argument(
        "--resolution-y",
        type=int,
        default=900,
        help="PNG height in pixels (default: 900)",
    )
    parser.add_argument(
        "--margin",
        type=float,
        default=1.18,
        help="Orthographic framing margin multiplier (default: 1.18)",
    )
    parser.add_argument(
        "--view-direction",
        nargs=3,
        type=float,
        metavar=("X", "Y", "Z"),
        default=(1.15, -1.35, 0.8),
        help="Direction from target to camera (default: 1.15 -1.35 0.8)",
    )
    parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Explicitly allow replacing preview PNG/report outputs",
    )
    return parser


def validate_arguments(args):
    if not args.action_name and not args.action_suffix:
        raise RuntimeError("Pass --action-name or --action-suffix")
    if args.resolution_x < 64 or args.resolution_y < 64:
        raise RuntimeError("Preview resolution must be at least 64x64")
    if args.margin <= 1.0:
        raise RuntimeError("--margin must be greater than 1.0")
    if not args.frames:
        raise RuntimeError("At least one --frames value is required")


def resolve_input_blend(value):
    path = Path(value).expanduser().resolve()
    if not path.is_file():
        raise RuntimeError("Input .blend does not exist: {}".format(path))
    if path.suffix.lower() != ".blend":
        raise RuntimeError("--blend must reference a .blend file: {}".format(path))
    return path


def resolve_outputs(args):
    output_dir = Path(args.output_dir).expanduser().resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = (
        Path(args.report).expanduser().resolve()
        if args.report
        else output_dir / "preview.json"
    )
    report_path.parent.mkdir(parents=True, exist_ok=True)
    return output_dir, report_path


def resolve_action(action_name, action_suffix):
    actions = list(bpy.data.actions)
    if not actions:
        raise RuntimeError("The .blend contains no Actions")

    if action_name:
        action = bpy.data.actions.get(action_name)
        if action is None:
            available = ", ".join(sorted(item.name for item in actions))
            raise RuntimeError(
                "Exact Action was not found: {}. Available: {}".format(
                    action_name, available
                )
            )
        if action_suffix and not action.name.endswith(action_suffix):
            raise RuntimeError(
                "Exact Action '{}' does not end with suffix '{}'".format(
                    action.name, action_suffix
                )
            )
        return action, "exact"

    matches = [item for item in actions if item.name.endswith(action_suffix)]
    if len(matches) != 1:
        matched = ", ".join(sorted(item.name for item in matches)) or "<none>"
        available = ", ".join(sorted(item.name for item in actions))
        raise RuntimeError(
            "Action suffix '{}' matched {} Actions: {}. Available: {}".format(
                action_suffix, len(matches), matched, available
            )
        )
    return matches[0], "suffix"


def action_bone_names(action):
    names = set()
    pattern = re.compile(r'^pose\.bones\["((?:\\.|[^"\\])+)"\]')
    for curve in action.fcurves:
        match = pattern.match(curve.data_path)
        if match:
            names.add(match.group(1).replace(r'\"', '"').replace(r'\\', '\\'))
    return names


def resolve_armature(explicit_name, action):
    armatures = [
        item for item in bpy.context.scene.objects if item.type == "ARMATURE"
    ]
    if explicit_name:
        armature = bpy.context.scene.objects.get(explicit_name)
        if armature is None or armature.type != "ARMATURE":
            raise RuntimeError(
                "Armature object was not found: {}".format(explicit_name)
            )
        return armature

    if len(armatures) == 1:
        return armatures[0]
    if not armatures:
        raise RuntimeError("The .blend contains no Armature object")

    animated_bones = action_bone_names(action)
    scored = []
    for armature in armatures:
        score = len(animated_bones.intersection(armature.pose.bones.keys()))
        scored.append((score, armature))
    scored.sort(key=lambda item: item[0], reverse=True)
    if scored[0][0] <= 0 or (
        len(scored) > 1 and scored[0][0] == scored[1][0]
    ):
        details = ", ".join(
            "{}={}".format(item.name, score) for score, item in scored
        )
        raise RuntimeError(
            "Multiple Armatures are ambiguous; pass --armature. Scores: {}".format(
                details
            )
        )
    return scored[0][1]


def validate_action_armature(action, armature):
    animated_bones = action_bone_names(action)
    if not animated_bones:
        raise RuntimeError(
            "Action '{}' contains no PoseBone animation curves".format(action.name)
        )
    missing = sorted(animated_bones.difference(armature.pose.bones.keys()))
    if missing:
        raise RuntimeError(
            "Action '{}' targets bones missing from Armature '{}': {}".format(
                action.name, armature.name, ", ".join(missing)
            )
        )


def linked_meshes(armature):
    all_meshes = [item for item in bpy.context.scene.objects if item.type == "MESH"]
    linked = []
    for mesh in all_meshes:
        if mesh.parent == armature:
            linked.append(mesh)
            continue
        for modifier in mesh.modifiers:
            if modifier.type == "ARMATURE" and modifier.object == armature:
                linked.append(mesh)
                break

    if linked:
        return linked, "armature-link"
    raise RuntimeError(
        "No Mesh is linked to Armature '{}'; linked bounds cannot be computed".format(
            armature.name
        )
    )


def set_action(armature, action):
    armature.animation_data_create()
    armature.animation_data.action = action
    for track in armature.animation_data.nla_tracks:
        track.mute = True


def validate_frames(action, frames):
    unique_frames = []
    seen = set()
    for frame in frames:
        if frame not in seen:
            seen.add(frame)
            unique_frames.append(frame)

    frame_start = float(action.frame_range[0])
    frame_end = float(action.frame_range[1])
    outside = [
        frame
        for frame in unique_frames
        if float(frame) < frame_start - 1.0e-4
        or float(frame) > frame_end + 1.0e-4
    ]
    if outside:
        raise RuntimeError(
            "Requested frames {} are outside Action '{}' range [{}, {}]".format(
                outside, action.name, frame_start, frame_end
            )
        )
    return unique_frames


def empty_bounds():
    return {
        "minimum": Vector((math.inf, math.inf, math.inf)),
        "maximum": Vector((-math.inf, -math.inf, -math.inf)),
        "point_count": 0,
    }


def include_point(bounds, point):
    for axis in range(3):
        bounds["minimum"][axis] = min(bounds["minimum"][axis], point[axis])
        bounds["maximum"][axis] = max(bounds["maximum"][axis], point[axis])
    bounds["point_count"] += 1


def evaluated_mesh_bounds(meshes, dependency_graph):
    bounds = empty_bounds()
    for mesh_object in meshes:
        evaluated = mesh_object.evaluated_get(dependency_graph)
        evaluated_mesh = evaluated.to_mesh()
        if evaluated_mesh is None:
            continue
        try:
            matrix_world = evaluated.matrix_world
            for vertex in evaluated_mesh.vertices:
                include_point(bounds, matrix_world @ vertex.co)
        finally:
            evaluated.to_mesh_clear()
    return bounds


def armature_bounds(armature):
    bounds = empty_bounds()
    matrix_world = armature.matrix_world
    for bone in armature.pose.bones:
        include_point(bounds, matrix_world @ bone.head)
        include_point(bounds, matrix_world @ bone.tail)
    return bounds


def merge_bounds(destination, source):
    if source["point_count"] == 0:
        return
    include_point(destination, source["minimum"])
    include_point(destination, source["maximum"])
    destination["point_count"] += max(0, source["point_count"] - 2)


def bounds_payload(bounds):
    if bounds["point_count"] == 0:
        return None
    minimum = bounds["minimum"]
    maximum = bounds["maximum"]
    center = (minimum + maximum) * 0.5
    size = maximum - minimum
    return {
        "minimum": [round(float(value), 8) for value in minimum],
        "maximum": [round(float(value), 8) for value in maximum],
        "center": [round(float(value), 8) for value in center],
        "size": [round(float(value), 8) for value in size],
        "point_count": int(bounds["point_count"]),
    }


def collect_frame_bounds(scene, armature, meshes, frames):
    union = empty_bounds()
    records = []
    for frame in frames:
        scene.frame_set(frame, subframe=0.0)
        bpy.context.view_layer.update()
        dependency_graph = bpy.context.evaluated_depsgraph_get()
        mesh = evaluated_mesh_bounds(meshes, dependency_graph)
        bones = armature_bounds(armature)
        combined = empty_bounds()
        merge_bounds(combined, mesh)
        merge_bounds(combined, bones)
        if combined["point_count"] == 0:
            raise RuntimeError("No bounds points were found at frame {}".format(frame))
        merge_bounds(union, combined)
        records.append(
            {
                "frame": frame,
                "mesh_bounds": bounds_payload(mesh),
                "armature_bounds": bounds_payload(bones),
                "combined_bounds": bounds_payload(combined),
            }
        )
    return union, records


def bounds_corners(bounds):
    minimum = bounds["minimum"]
    maximum = bounds["maximum"]
    return [
        Vector((x, y, z))
        for x in (minimum.x, maximum.x)
        for y in (minimum.y, maximum.y)
        for z in (minimum.z, maximum.z)
    ]


def hide_unrelated_render_objects(meshes):
    target_pointers = {item.as_pointer() for item in meshes}
    for item in bpy.context.scene.objects:
        if item.type in {"LIGHT", "CAMERA"}:
            item.hide_render = True
        elif item.type == "MESH":
            item.hide_render = item.as_pointer() not in target_pointers
    for mesh in meshes:
        mesh.hide_render = False
        mesh.hide_set(False)


def preview_collection(scene):
    name = "__ActorXActionPreview"
    existing = bpy.data.collections.get(name)
    if existing is not None:
        for item in list(existing.objects):
            bpy.data.objects.remove(item, do_unlink=True)
        bpy.data.collections.remove(existing)
    collection = bpy.data.collections.new(name)
    scene.collection.children.link(collection)
    return collection


def create_preview_material(view_layer):
    material = bpy.data.materials.new("__ActorXPreviewMaterial")
    material.use_nodes = True
    material.diffuse_color = (0.34, 0.56, 0.82, 1.0)
    principled = material.node_tree.nodes.get("Principled BSDF")
    if principled is not None:
        principled.inputs["Base Color"].default_value = (0.24, 0.48, 0.78, 1.0)
        principled.inputs["Roughness"].default_value = 0.62
        principled.inputs["Metallic"].default_value = 0.05
    view_layer.material_override = material
    return material


def create_camera(scene, collection, bounds, args):
    direction = Vector(tuple(args.view_direction))
    if direction.length_squared < 1.0e-10:
        raise RuntimeError("--view-direction must not be the zero vector")
    direction.normalize()

    center = (bounds["minimum"] + bounds["maximum"]) * 0.5
    size = bounds["maximum"] - bounds["minimum"]
    diagonal = max(float(size.length), 1.0e-4)
    distance = max(diagonal * 2.5, 1.0)

    camera_data = bpy.data.cameras.new("__ActorXPreviewCamera")
    camera_data.type = "ORTHO"
    camera = bpy.data.objects.new("__ActorXPreviewCamera", camera_data)
    collection.objects.link(camera)
    camera.location = center + direction * distance
    camera.rotation_euler = (center - camera.location).to_track_quat(
        "-Z", "Y"
    ).to_euler()
    camera_data.clip_start = max(0.001, distance * 0.005)
    camera_data.clip_end = max(distance + diagonal * 10.0, 100.0)
    scene.camera = camera
    bpy.context.view_layer.update()

    rotation = camera.matrix_world.to_quaternion()
    right = rotation @ Vector((1.0, 0.0, 0.0))
    up = rotation @ Vector((0.0, 1.0, 0.0))
    projected_x = [(point - center).dot(right) for point in bounds_corners(bounds)]
    projected_y = [(point - center).dot(up) for point in bounds_corners(bounds)]
    projected_width = max(projected_x) - min(projected_x)
    projected_height = max(projected_y) - min(projected_y)
    aspect = float(args.resolution_x) / float(args.resolution_y)
    camera_data.ortho_scale = max(
        projected_height,
        projected_width / aspect,
        diagonal * 0.01,
        0.01,
    ) * args.margin

    return camera, {
        "type": "ORTHO",
        "location": [round(float(value), 8) for value in camera.location],
        "target": [round(float(value), 8) for value in center],
        "view_direction": [round(float(value), 8) for value in direction],
        "ortho_scale": round(float(camera_data.ortho_scale), 8),
        "clip_start": round(float(camera_data.clip_start), 8),
        "clip_end": round(float(camera_data.clip_end), 8),
        "margin": float(args.margin),
    }


def add_sun(collection, name, energy, direction, angle):
    light_data = bpy.data.lights.new(name=name, type="SUN")
    light_data.energy = energy
    light_data.angle = angle
    light = bpy.data.objects.new(name, light_data)
    collection.objects.link(light)
    direction_vector = Vector(direction).normalized()
    light.rotation_euler = direction_vector.to_track_quat("-Z", "Y").to_euler()
    return light


def create_basic_lighting(collection):
    lights = [
        add_sun(collection, "__ActorXKey", 2.2, (-0.8, 0.5, -1.0), 0.22),
        add_sun(collection, "__ActorXFill", 0.8, (0.7, 0.8, -0.35), 0.45),
        add_sun(collection, "__ActorXRim", 1.15, (0.1, -1.0, -0.5), 0.3),
    ]
    return [
        {
            "name": item.name,
            "type": item.data.type,
            "energy": float(item.data.energy),
            "angle": float(item.data.angle),
        }
        for item in lights
    ]


def configure_render(scene, args):
    scene.render.engine = "BLENDER_EEVEE"
    scene.eevee.use_gtao = True
    scene.eevee.gtao_distance = 3.0
    scene.eevee.gtao_factor = 1.25
    scene.render.resolution_x = args.resolution_x
    scene.render.resolution_y = args.resolution_y
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGB"
    scene.render.film_transparent = False
    scene.render.use_file_extension = True
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "Medium High Contrast"
    scene.view_settings.exposure = 0.0
    scene.view_settings.gamma = 1.0

    if scene.world is None:
        scene.world = bpy.data.worlds.new("__ActorXPreviewWorld")
    scene.world.use_nodes = True
    background = scene.world.node_tree.nodes.get("Background")
    if background is not None:
        background.inputs["Color"].default_value = (0.025, 0.032, 0.045, 1.0)
        background.inputs["Strength"].default_value = 0.32


def safe_file_stem(value):
    sanitized = re.sub(r'[<>:"/\\|?*\x00-\x1f]+', "_", value).strip(" .")
    return sanitized or "action"


def frame_label(frame):
    if frame < 0:
        return "m{:04d}".format(abs(frame))
    return "{:04d}".format(frame)


def ensure_outputs_available(paths, overwrite):
    existing = [path for path in paths if path.exists()]
    if existing and not overwrite:
        raise RuntimeError(
            "Preview output already exists; pass --overwrite only for staging: {}".format(
                ", ".join(str(path) for path in existing)
            )
        )


def file_sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def render_frames(scene, action, frames, output_paths, frame_records):
    images = []
    records_by_frame = {record["frame"]: record for record in frame_records}
    for frame, output_path in zip(frames, output_paths):
        scene.frame_set(frame, subframe=0.0)
        bpy.context.view_layer.update()
        scene.render.filepath = str(output_path)
        result = bpy.ops.render.render(write_still=True)
        if "FINISHED" not in result or not output_path.is_file():
            raise RuntimeError(
                "Blender did not render Action '{}' frame {} to {}".format(
                    action.name, frame, output_path
                )
            )
        if output_path.stat().st_size == 0:
            raise RuntimeError("Blender created an empty PNG: {}".format(output_path))
        images.append(
            {
                "frame": frame,
                "path": str(output_path),
                "size": output_path.stat().st_size,
                "sha256": file_sha256(output_path),
                "bounds": records_by_frame[frame],
            }
        )
    return images


def write_report(
    report_path,
    blend_path,
    action,
    selection_mode,
    armature,
    meshes,
    mesh_selection_mode,
    frames,
    union_bounds,
    camera_report,
    light_report,
    images,
    args,
):
    payload = {
        "schema": "LOSTARK_ACTORX_ACTION_PREVIEW_REPORT",
        "version": 1,
        "blender_version": bpy.app.version_string,
        "input_blend": str(blend_path),
        "input_blend_size": blend_path.stat().st_size,
        "input_blend_sha256": file_sha256(blend_path),
        "action": {
            "name": action.name,
            "selection_mode": selection_mode,
            "requested_exact_name": args.action_name,
            "requested_suffix": args.action_suffix,
            "frame_start": float(action.frame_range[0]),
            "frame_end": float(action.frame_range[1]),
            "fcurve_count": len(action.fcurves),
        },
        "armature": armature.name,
        "bone_count": len(armature.data.bones),
        "mesh_selection_mode": mesh_selection_mode,
        "meshes": [
            {
                "name": mesh.name,
                "vertex_count": len(mesh.data.vertices),
                "polygon_count": len(mesh.data.polygons),
            }
            for mesh in meshes
        ],
        "requested_frames": frames,
        "union_bounds": bounds_payload(union_bounds),
        "camera": camera_report,
        "lights": light_report,
        "render": {
            "engine": "BLENDER_EEVEE",
            "resolution_x": args.resolution_x,
            "resolution_y": args.resolution_y,
            "material_override": "__ActorXPreviewMaterial",
            "fixed_camera_for_all_frames": True,
        },
        "images": images,
    }
    report_path.write_text(
        json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8"
    )


def run(args):
    global bpy
    global Vector
    try:
        import bpy as blender_python
        from mathutils import Vector as BlenderVector
    except ImportError as error:
        raise RuntimeError(
            "This script must be run by Blender. Use blender.exe --background "
            "--python <script> -- <arguments>."
        ) from error
    bpy = blender_python
    Vector = BlenderVector

    validate_arguments(args)
    blend_path = resolve_input_blend(args.blend)
    output_dir, report_path = resolve_outputs(args)

    result = bpy.ops.wm.open_mainfile(filepath=str(blend_path))
    if "FINISHED" not in result:
        raise RuntimeError("Blender could not open: {}".format(blend_path))

    action, selection_mode = resolve_action(args.action_name, args.action_suffix)
    armature = resolve_armature(args.armature, action)
    validate_action_armature(action, armature)
    meshes, mesh_selection_mode = linked_meshes(armature)
    set_action(armature, action)
    frames = validate_frames(action, args.frames)

    action_stem = safe_file_stem(action.name)
    output_paths = [
        output_dir / "{}_frame_{}.png".format(action_stem, frame_label(frame))
        for frame in frames
    ]
    distinct_paths = [blend_path, report_path] + output_paths
    normalized_paths = [str(path).casefold() for path in distinct_paths]
    if len(normalized_paths) != len(set(normalized_paths)):
        raise RuntimeError(
            "Input .blend, report, and every PNG output path must be different"
        )
    ensure_outputs_available(output_paths + [report_path], args.overwrite)

    scene = bpy.context.scene
    union_bounds, frame_records = collect_frame_bounds(
        scene, armature, meshes, frames
    )
    hide_unrelated_render_objects(meshes)
    collection = preview_collection(scene)
    create_preview_material(bpy.context.view_layer)
    camera, camera_report = create_camera(scene, collection, union_bounds, args)
    light_report = create_basic_lighting(collection)
    configure_render(scene, args)
    images = render_frames(
        scene, action, frames, output_paths, frame_records
    )
    write_report(
        report_path,
        blend_path,
        action,
        selection_mode,
        armature,
        meshes,
        mesh_selection_mode,
        frames,
        union_bounds,
        camera_report,
        light_report,
        images,
        args,
    )

    print("ActorX action preview rendered: {}".format(action.name))
    print("Frames: {}".format(", ".join(str(frame) for frame in frames)))
    print("Camera: {} ({})".format(camera.name, camera.data.type))
    print("Images: {}".format(len(images)))
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
