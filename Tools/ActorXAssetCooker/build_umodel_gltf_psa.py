"""Attach ActorX PSA clips to a UModel skinned glTF without a Blender addon.

This is an offline intake step.  It does not create a second runtime format:
the generated glTF is staging input for the repository ModelAssetConverter,
which remains the only writer of CModel WModel assets.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import sys
from typing import BinaryIO


CHUNK_HEADER = struct.Struct("<20siii")
ANIM_INFO = struct.Struct("<64s64s4i3f3i")
ANIM_KEY = struct.Struct("<3f4ff")
SCALE_KEY = struct.Struct("<4f")
MAX_SOURCE_BYTES = 512 * 1024 * 1024
FLOAT_COMPONENT_TYPE = 5126


def decode_name(raw: bytes) -> str:
    return raw.split(b"\0", 1)[0].decode("windows-1252").rstrip(" ")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def bounded_file(path: Path, label: str) -> Path:
    resolved = path.resolve(strict=True)
    if not resolved.is_file():
        raise RuntimeError(f"{label} is not a regular file: {resolved}")
    size = resolved.stat().st_size
    if size <= 0 or size > MAX_SOURCE_BYTES:
        raise RuntimeError(f"{label} size is outside the intake limit: {size}")
    return resolved


def read_exact(stream: BinaryIO, count: int, label: str) -> bytes:
    value = stream.read(count)
    if len(value) != count:
        raise RuntimeError(f"Truncated PSA {label}")
    return value


def read_psa(path: Path) -> dict:
    chunks: dict[str, tuple[int, int, bytes]] = {}
    with path.open("rb") as stream:
        while True:
            header = stream.read(CHUNK_HEADER.size)
            if not header:
                break
            if len(header) != CHUNK_HEADER.size:
                raise RuntimeError("Truncated PSA chunk header")
            raw_name, _flags, record_size, record_count = CHUNK_HEADER.unpack(header)
            name = decode_name(raw_name)
            if record_size < 0 or record_count < 0:
                raise RuntimeError(f"Invalid PSA chunk dimensions: {name}")
            if name in chunks:
                raise RuntimeError(f"Duplicate PSA chunk: {name}")
            payload_size = record_size * record_count
            if payload_size > MAX_SOURCE_BYTES:
                raise RuntimeError(f"PSA chunk exceeds the intake limit: {name}")
            chunks[name] = (
                record_size,
                record_count,
                read_exact(stream, payload_size, name),
            )

    required = {"BONENAMES", "ANIMINFO", "ANIMKEYS"}
    missing = sorted(required.difference(chunks))
    if missing:
        raise RuntimeError("PSA is missing chunks: " + ", ".join(missing))

    bone_size, bone_count, bone_payload = chunks["BONENAMES"]
    if bone_size != 120 or bone_count <= 0:
        raise RuntimeError("Unsupported PSA BONENAMES layout")
    bones = []
    parents = []
    for index in range(bone_count):
        offset = index * bone_size
        name = decode_name(bone_payload[offset : offset + 64])
        parent = struct.unpack_from("<i", bone_payload, offset + 72)[0]
        if not name or name in bones:
            raise RuntimeError("PSA bone names must be non-empty and unique")
        if parent < -1 or parent >= index:
            raise RuntimeError(f"PSA bone parent is invalid: {name} -> {parent}")
        bones.append(name)
        parents.append(parent)

    info_size, info_count, info_payload = chunks["ANIMINFO"]
    if info_size != ANIM_INFO.size or info_count <= 0:
        raise RuntimeError("Unsupported PSA ANIMINFO layout")
    sequences = []
    expected_first_frame = 0
    for index in range(info_count):
        values = ANIM_INFO.unpack_from(info_payload, index * info_size)
        name = decode_name(values[0])
        total_bones = values[2]
        track_time = float(values[7])
        rate = float(values[8])
        start_bone = values[9]
        first_frame = values[10]
        frame_count = values[11]
        if (
            not name
            or total_bones != bone_count
            or start_bone != 0
            or first_frame != expected_first_frame
            or frame_count <= 0
            or not math.isfinite(track_time)
            or not math.isclose(track_time, float(frame_count), abs_tol=0.0001)
            or not math.isfinite(rate)
            or rate <= 0.0
        ):
            raise RuntimeError(f"Unsupported PSA sequence contract: {name}")
        sequences.append(
            {
                "name": name,
                "first_frame": first_frame,
                "frame_count": frame_count,
                "rate": rate,
            }
        )
        expected_first_frame += frame_count

    key_size, key_count, key_payload = chunks["ANIMKEYS"]
    expected_key_count = expected_first_frame * bone_count
    if key_size != ANIM_KEY.size or key_count != expected_key_count:
        raise RuntimeError(
            f"PSA ANIMKEYS count mismatch: expected {expected_key_count}, got {key_count}"
        )
    animation_keys = [
        ANIM_KEY.unpack_from(key_payload, index * key_size)
        for index in range(key_count)
    ]
    for key in animation_keys:
        if not all(math.isfinite(value) for value in key):
            raise RuntimeError("PSA contains a non-finite animation key")

    scale_keys = None
    if "SCALEKEYS" in chunks:
        scale_size, scale_count, scale_payload = chunks["SCALEKEYS"]
        if scale_size != SCALE_KEY.size or scale_count != expected_key_count:
            raise RuntimeError("PSA SCALEKEYS count/layout mismatch")
        scale_keys = [
            SCALE_KEY.unpack_from(scale_payload, index * scale_size)
            for index in range(scale_count)
        ]
        for key in scale_keys:
            if not all(math.isfinite(value) for value in key):
                raise RuntimeError("PSA contains a non-finite scale key")
            if not math.isclose(key[3], 1.0, abs_tol=0.0001):
                raise RuntimeError("Unsupported PSA SCALEKEYS time convention")

    return {
        "bones": bones,
        "parents": parents,
        "sequences": sequences,
        "animation_keys": animation_keys,
        "scale_keys": scale_keys,
    }


def validate_gltf(document: dict, gltf_path: Path, psa: dict) -> tuple[Path, list[int]]:
    if document.get("asset", {}).get("version") != "2.0":
        raise RuntimeError("Source is not a glTF 2.0 document")
    buffers = document.get("buffers")
    skins = document.get("skins")
    nodes = document.get("nodes")
    if not isinstance(buffers, list) or len(buffers) != 1:
        raise RuntimeError("Exactly one external glTF buffer is required")
    if not isinstance(skins, list) or len(skins) != 1:
        raise RuntimeError("Exactly one glTF skin is required")
    if not isinstance(nodes, list) or not nodes:
        raise RuntimeError("glTF nodes are missing")
    if document.get("animations"):
        raise RuntimeError("Source glTF already contains animations")
    uri = buffers[0].get("uri")
    if not isinstance(uri, str) or not uri or uri.startswith("data:"):
        raise RuntimeError("glTF buffer must use one relative external URI")
    if Path(uri).is_absolute() or ".." in Path(uri).parts:
        raise RuntimeError("glTF buffer URI is unsafe")
    source_buffer = bounded_file(gltf_path.parent / uri, "glTF buffer")
    if buffers[0].get("byteLength") != source_buffer.stat().st_size:
        raise RuntimeError("glTF buffer byteLength differs from the source file")

    joints = skins[0].get("joints")
    if not isinstance(joints, list) or len(joints) != len(psa["bones"]):
        raise RuntimeError("glTF joint count differs from PSA BONENAMES")
    joint_names = []
    for node_index in joints:
        if not isinstance(node_index, int) or not 0 <= node_index < len(nodes):
            raise RuntimeError("glTF skin contains an invalid joint index")
        name = nodes[node_index].get("name")
        if not isinstance(name, str) or not name:
            raise RuntimeError("glTF joint node has no stable name")
        joint_names.append(name)
    if joint_names != psa["bones"]:
        raise RuntimeError(
            "glTF joint order/names differ from PSA BONENAMES: "
            f"{joint_names} vs {psa['bones']}"
        )
    return source_buffer, joints


def align4(payload: bytearray) -> None:
    payload.extend(b"\0" * ((-len(payload)) & 3))


def append_accessor(
    document: dict,
    payload: bytearray,
    values: list[tuple[float, ...]] | list[float],
    accessor_type: str,
    component_count: int,
    minimum: list[float] | None = None,
    maximum: list[float] | None = None,
) -> int:
    align4(payload)
    byte_offset = len(payload)
    if component_count == 1:
        for value in values:
            payload.extend(struct.pack("<f", float(value)))
    else:
        packer = struct.Struct("<" + "f" * component_count)
        for value in values:
            if len(value) != component_count:
                raise RuntimeError("Generated animation accessor has the wrong width")
            payload.extend(packer.pack(*value))
    byte_length = len(payload) - byte_offset
    views = document.setdefault("bufferViews", [])
    view_index = len(views)
    views.append({"buffer": 0, "byteOffset": byte_offset, "byteLength": byte_length})
    accessors = document.setdefault("accessors", [])
    accessor_index = len(accessors)
    accessor = {
        "bufferView": view_index,
        "componentType": FLOAT_COMPONENT_TYPE,
        "count": len(values),
        "type": accessor_type,
    }
    if minimum is not None:
        accessor["min"] = minimum
    if maximum is not None:
        accessor["max"] = maximum
    accessors.append(accessor)
    return accessor_index


def read_accessor_floats(
    document: dict, payload: bytearray, accessor_index: int, width: int
) -> tuple[int, int]:
    """Return the payload offset and element count of a tightly packed accessor."""
    accessor = document["accessors"][accessor_index]
    if accessor.get("componentType") != 5126:
        raise RuntimeError("Only float accessors can be scaled")
    view_index = accessor.get("bufferView")
    if not isinstance(view_index, int):
        raise RuntimeError("Scaled accessor must reference a bufferView")
    view = document["bufferViews"][view_index]
    if view.get("byteStride") not in (None, width * 4):
        raise RuntimeError("Interleaved accessors are not supported")
    offset = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    count = int(accessor["count"])
    if offset + count * width * 4 > len(payload):
        raise RuntimeError("Accessor range is outside the glTF buffer")
    return offset, count


def scale_bind_geometry(document: dict, payload: bytearray, scale: float) -> dict:
    """Scale every length-carrying value of the bind pose by the same factor.

    The converter's own --scale only multiplies mesh vertices, which silently
    desynchronises a skinned asset from its skeleton. Positions, inverse bind
    translations, node translations and PSA translation keys must all move
    together, so the whole conversion is scaled here instead.
    """
    if scale == 1.0:
        return {"positionAccessors": 0, "inverseBindMatrices": 0, "nodeTranslations": 0}

    position_accessors = set()
    for mesh in document.get("meshes", []):
        for primitive in mesh.get("primitives", []):
            index = primitive.get("attributes", {}).get("POSITION")
            if index is None:
                raise RuntimeError("A glTF primitive has no POSITION attribute")
            position_accessors.add(int(index))
    for index in sorted(position_accessors):
        accessor = document["accessors"][index]
        if accessor.get("type") != "VEC3":
            raise RuntimeError("POSITION accessor must be VEC3")
        offset, count = read_accessor_floats(document, payload, index, 3)
        for element in range(count * 3):
            at = offset + element * 4
            value = struct.unpack_from("<f", payload, at)[0]
            struct.pack_into("<f", payload, at, value * scale)
        for bound in ("min", "max"):
            if bound in accessor:
                accessor[bound] = [component * scale for component in accessor[bound]]

    inverse_bind = document["skins"][0].get("inverseBindMatrices")
    matrices = 0
    if inverse_bind is not None:
        accessor = document["accessors"][int(inverse_bind)]
        if accessor.get("type") != "MAT4":
            raise RuntimeError("inverseBindMatrices accessor must be MAT4")
        offset, matrices = read_accessor_floats(document, payload, int(inverse_bind), 16)
        # glTF matrices are column-major, so 12..14 hold the translation.
        for matrix in range(matrices):
            base = offset + matrix * 64
            for component in (12, 13, 14):
                at = base + component * 4
                value = struct.unpack_from("<f", payload, at)[0]
                struct.pack_into("<f", payload, at, value * scale)

    translated_nodes = 0
    for node in document.get("nodes", []):
        translation = node.get("translation")
        if translation is None:
            continue
        if len(translation) != 3:
            raise RuntimeError("A glTF node translation is not a VEC3")
        node["translation"] = [component * scale for component in translation]
        translated_nodes += 1
        if "matrix" in node:
            raise RuntimeError("A glTF node mixes matrix and TRS transforms")
    for node in document.get("nodes", []):
        if "matrix" in node:
            raise RuntimeError("Matrix-form glTF nodes are not supported")

    return {
        "positionAccessors": len(position_accessors),
        "inverseBindMatrices": matrices,
        "nodeTranslations": translated_nodes,
    }


def normalize_quaternion(value: tuple[float, float, float, float]) -> tuple[float, ...]:
    length = math.sqrt(sum(component * component for component in value))
    if not math.isfinite(length) or length <= 1e-8:
        raise RuntimeError("PSA contains a zero-length quaternion")
    return tuple(component / length for component in value)


def build_animations(
    document: dict,
    payload: bytearray,
    psa: dict,
    joints: list[int],
    translation_scale: float = 1.0,
) -> list[dict]:
    animations = []
    bone_count = len(joints)
    keys = psa["animation_keys"]
    scale_keys = psa["scale_keys"]
    receipts = []
    for sequence in psa["sequences"]:
        frame_count = sequence["frame_count"]
        first_frame = sequence["first_frame"]
        rate = sequence["rate"]
        times = [frame / rate for frame in range(frame_count)]
        time_accessor = append_accessor(
            document,
            payload,
            times,
            "SCALAR",
            1,
            [0.0],
            [times[-1]],
        )
        samplers = []
        channels = []
        for bone_index, node_index in enumerate(joints):
            translations = []
            rotations = []
            scales = []
            previous_rotation = None
            for local_frame in range(frame_count):
                source_frame = first_frame + local_frame
                key_index = source_frame * bone_count + bone_index
                key = keys[key_index]
                # UModel glTF uses metres and maps UE3 (X,Y,Z) to (X,Z,-Y).
                # scale keeps translation keys in the same unit as the bind pose.
                unit = 0.01 * translation_scale
                translations.append((key[0] * unit, key[2] * unit, -key[1] * unit))
                rotation = normalize_quaternion((key[3], key[5], -key[4], key[6]))
                if previous_rotation is not None and sum(
                    a * b for a, b in zip(previous_rotation, rotation)
                ) < 0.0:
                    rotation = tuple(-value for value in rotation)
                rotations.append(rotation)
                previous_rotation = rotation
                if scale_keys is not None:
                    scale = scale_keys[key_index]
                    scales.append((scale[0], scale[2], scale[1]))

            outputs = [
                ("translation", "VEC3", 3, translations),
                ("rotation", "VEC4", 4, rotations),
            ]
            if scales:
                outputs.append(("scale", "VEC3", 3, scales))
            for path, accessor_type, width, values in outputs:
                output_accessor = append_accessor(
                    document, payload, values, accessor_type, width
                )
                sampler_index = len(samplers)
                samplers.append(
                    {
                        "input": time_accessor,
                        "interpolation": "LINEAR",
                        "output": output_accessor,
                    }
                )
                channels.append(
                    {
                        "sampler": sampler_index,
                        "target": {"node": node_index, "path": path},
                    }
                )
        animations.append(
            {"name": sequence["name"], "samplers": samplers, "channels": channels}
        )
        receipts.append(
            {
                "name": sequence["name"],
                "frameCount": frame_count,
                "framesPerSecond": rate,
                "durationSeconds": times[-1],
                "channelCount": len(channels),
            }
        )
    document["animations"] = animations
    return receipts


def ensure_output(path: Path, overwrite: bool) -> Path:
    resolved = path.resolve()
    if resolved.exists() and not overwrite:
        raise RuntimeError(f"Output already exists: {resolved}")
    resolved.parent.mkdir(parents=True, exist_ok=True)
    return resolved


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--gltf", required=True, type=Path)
    parser.add_argument("--psa", required=True, type=Path)
    parser.add_argument("--output-gltf", required=True, type=Path)
    parser.add_argument("--output-bin", required=True, type=Path)
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--overwrite", action="store_true")
    parser.add_argument(
        "--scale",
        type=float,
        default=1.0,
        help="Uniform factor applied to positions, bind poses and translation keys.",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    source_gltf = bounded_file(args.gltf, "glTF")
    source_psa = bounded_file(args.psa, "PSA")
    output_gltf = ensure_output(args.output_gltf, args.overwrite)
    output_bin = ensure_output(args.output_bin, args.overwrite)
    report_path = ensure_output(args.report, args.overwrite)
    if output_gltf.parent != output_bin.parent:
        raise RuntimeError("Output glTF and buffer must use the same directory")
    output_paths = {str(path).casefold() for path in (output_gltf, output_bin, report_path)}
    if len(output_paths) != 3 or str(source_gltf).casefold() in output_paths or str(source_psa).casefold() in output_paths:
        raise RuntimeError("Input and output files must all be different")

    try:
        document = json.loads(source_gltf.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise RuntimeError(f"Could not parse source glTF: {error}") from error
    if not math.isfinite(args.scale) or args.scale <= 0.0:
        raise RuntimeError("--scale must be a finite positive number")
    psa = read_psa(source_psa)
    source_buffer, joints = validate_gltf(document, source_gltf, psa)
    payload = bytearray(source_buffer.read_bytes())
    scaled = scale_bind_geometry(document, payload, args.scale)
    receipts = build_animations(document, payload, psa, joints, args.scale)
    align4(payload)
    document["buffers"][0]["uri"] = output_bin.name
    document["buffers"][0]["byteLength"] = len(payload)

    output_bin.write_bytes(payload)
    output_gltf.write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    report = {
        "schema": "lostark.umodel-gltf-psa-stage",
        "formatVersion": 1,
        "source": {
            "gltf": str(source_gltf),
            "gltfSha256": sha256(source_gltf),
            "buffer": str(source_buffer),
            "bufferSha256": sha256(source_buffer),
            "psa": str(source_psa),
            "psaSha256": sha256(source_psa),
        },
        "jointCount": len(joints),
        "scale": args.scale,
        "scaled": scaled,
        "clips": receipts,
        "output": {
            "gltf": str(output_gltf),
            "gltfSha256": sha256(output_gltf),
            "buffer": str(output_bin),
            "bufferSha256": sha256(output_bin),
        },
    }
    report_path.write_text(
        json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(
        "UMODEL_GLTF_PSA_OK "
        f"joints={len(joints)} clips={len(receipts)} scale={args.scale} "
        f"output={output_gltf}"
    )


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"UMODEL_GLTF_PSA_ERROR {error}", file=sys.stderr)
        sys.exit(1)
