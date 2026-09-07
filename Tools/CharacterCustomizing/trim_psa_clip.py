"""Cut one ActorX PSA sequence out of a family AnimSet and re-base it onto a rig.

A retail family AnimSet holds a thousand or more sequences and drives only the
bones that family shares.  A class rig carries extra attachment bones -- hair
strands, a tail, skirt panels -- that the shared clip never touches, and
``build_umodel_gltf_psa.py`` requires the PSA bone list to match the glTF joint
list exactly, in order.

This step writes a new PSA holding one sequence, with its bone list rewritten to
the rig's joint order.  A bone the source clip does not animate is emitted as a
constant key holding the rig's own bind pose, which is where an unanimated bone
sits anyway.  No key value is invented: animated bones keep their source bytes.

This is an offline intake step.  It writes a staging PSA for
``build_umodel_gltf_psa.py``; it does not touch runtime resources.

An animation set that will be attached to a body cooked through the ActorX/FBX
path needs one more thing.  That body's bone-local keys are raw ActorX values --
centimetres, ActorX axes -- while ``build_umodel_gltf_psa.py`` deliberately
rewrites keys into glTF's Y-up metres.  Cloned animations are evaluated against
the body's own skeleton, so the two have to agree.  ``--target-space actorx``
pre-applies the inverse of that rewrite, so the clip comes out of the converter
holding exactly the values it had in the retail PSA.  Pair it with
``build_umodel_gltf_psa.py --scale 100`` so the unit cancels too.

The same option undoes ActorX's quaternion storage convention.  A PSA keeps the
root bone's rotation as written and every other bone's conjugated; the FBX
importers un-conjugate on the way in, the glTF injector does not.  Left alone
that plays every bone's rotation backwards, which reads on screen as a character
lying flat in a T-pose.

Usage:
  python trim_psa_clip.py --psa <family.psa> --gltf <rig.gltf>
                          --clip <sequence name> --output <out.psa>
                          [--target-space gltf|actorx] [--report <out.json>]
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
import struct
import sys

CHUNK_HEADER = struct.Struct("<20sIii")
BONE_RECORD = struct.Struct("<64siii4f3ff3f")
ANIM_INFO = struct.Struct("<64s64s4i3f3i")
ANIM_KEY = struct.Struct("<3f4ff")
SCALE_KEY = struct.Struct("<4f")

# build_umodel_gltf_psa.py reads a PSA key as glTF (x, z, -y) at 0.01 scale and
# a PSA quaternion as glTF (x, z, -y, w).  A synthesised key has to survive that
# same conversion and land back on the bind pose the rig node already carries,
# so these two helpers are the exact inverses of it.
POSITION_UNIT = 100.0


def decode_name(raw: bytes) -> str:
    return raw.split(b"\0", 1)[0].decode("windows-1252").rstrip(" ")


def encode_name(name: str, width: int) -> bytes:
    raw = name.encode("windows-1252")
    if len(raw) >= width:
        raise SystemExit(f"Name does not fit in {width} bytes: {name}")
    return raw + b"\0" * (width - len(raw))


def gltf_position_to_psa(translation: list[float]) -> tuple[float, float, float]:
    x, y, z = translation
    return (x * POSITION_UNIT, -z * POSITION_UNIT, y * POSITION_UNIT)


def gltf_rotation_to_psa(rotation: list[float]) -> tuple[float, float, float, float]:
    g0, g1, g2, g3 = rotation
    return (g0, -g2, g1, g3)


# The injector maps a PSA key to glTF as (x, z, -y) and a quaternion the same
# way, and swaps the two trailing scale components.  Pre-applying these inverses
# makes that mapping a round trip, which is what an ActorX-cooked body needs.
def cancel_position(x: float, y: float, z: float) -> tuple[float, float, float]:
    return (x, -z, y)


def cancel_rotation(x: float, y: float, z: float, w: float) -> tuple[float, float, float, float]:
    return (x, -z, y, w)


def unconjugate(x: float, y: float, z: float, w: float) -> tuple[float, float, float, float]:
    """ActorX stores every non-root bone's rotation conjugated."""
    return (-x, -y, -z, w)


def cancel_scale(x: float, y: float, z: float) -> tuple[float, float, float]:
    return (x, z, y)


def read_chunks(path: Path) -> dict[str, tuple[int, int, int]]:
    """Returns name -> (record size, record count, payload offset)."""
    layout: dict[str, tuple[int, int, int]] = {}
    with path.open("rb") as stream:
        while True:
            header = stream.read(CHUNK_HEADER.size)
            if not header:
                break
            if len(header) != CHUNK_HEADER.size:
                raise SystemExit("Truncated PSA chunk header")
            raw_name, _flags, record_size, record_count = CHUNK_HEADER.unpack(header)
            name = decode_name(raw_name)
            if record_size < 0 or record_count < 0:
                raise SystemExit(f"Invalid PSA chunk dimensions: {name}")
            if name in layout:
                raise SystemExit(f"Duplicate PSA chunk: {name}")
            layout[name] = (record_size, record_count, stream.tell())
            stream.seek(record_size * record_count, 1)
    for required in ("BONENAMES", "ANIMINFO", "ANIMKEYS"):
        if required not in layout:
            raise SystemExit(f"PSA is missing the {required} chunk")
    return layout


def read_records(path: Path, layout: tuple[int, int, int]) -> bytes:
    record_size, record_count, offset = layout
    with path.open("rb") as stream:
        stream.seek(offset)
        payload = stream.read(record_size * record_count)
    if len(payload) != record_size * record_count:
        raise SystemExit("Truncated PSA chunk payload")
    return payload


def read_key_slice(
    path: Path,
    layout: tuple[int, int, int],
    first_record: int,
    record_total: int,
) -> bytes:
    """Reads only the frames this clip needs; a family AnimSet key chunk is
    hundreds of megabytes and must never be loaded whole."""
    record_size, record_count, offset = layout
    if first_record < 0 or first_record + record_total > record_count:
        raise SystemExit("Clip key range falls outside the PSA chunk")
    with path.open("rb") as stream:
        stream.seek(offset + first_record * record_size)
        payload = stream.read(record_total * record_size)
    if len(payload) != record_total * record_size:
        raise SystemExit("Truncated PSA key slice")
    return payload


def load_gltf_rig(path: Path) -> tuple[list[str], list[int], list[dict]]:
    document = json.loads(path.read_text(encoding="utf-8"))
    skins = document.get("skins")
    nodes = document.get("nodes")
    if not isinstance(skins, list) or len(skins) != 1:
        raise SystemExit("Exactly one glTF skin is required")
    if not isinstance(nodes, list) or not nodes:
        raise SystemExit("glTF nodes are missing")
    joints = skins[0].get("joints")
    if not isinstance(joints, list) or not joints:
        raise SystemExit("glTF skin has no joints")

    names: list[str] = []
    for node_index in joints:
        if not isinstance(node_index, int) or not 0 <= node_index < len(nodes):
            raise SystemExit("glTF skin contains an invalid joint index")
        name = nodes[node_index].get("name")
        if not isinstance(name, str) or not name:
            raise SystemExit("glTF joint node has no stable name")
        names.append(name)
    if len(set(names)) != len(names):
        raise SystemExit("glTF joint names are not unique")

    slot_of_node = {node_index: slot for slot, node_index in enumerate(joints)}
    parents = [-1] * len(joints)
    for slot, node_index in enumerate(joints):
        for child in nodes[node_index].get("children", []):
            child_slot = slot_of_node.get(child)
            if child_slot is None:
                continue
            if parents[child_slot] != -1:
                raise SystemExit(f"glTF joint has two parents: {names[child_slot]}")
            parents[child_slot] = slot
    for slot, parent in enumerate(parents):
        if parent >= slot:
            raise SystemExit(
                "glTF joints are not ordered parent before child: " + names[slot]
            )
    return names, parents, [nodes[i] for i in joints]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--psa", required=True, type=Path)
    parser.add_argument("--gltf", required=True, type=Path)
    parser.add_argument("--clip", required=True)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--report", type=Path)
    parser.add_argument(
        "--target-space",
        choices=("gltf", "actorx"),
        default="gltf",
        help="actorx pre-cancels the glTF intake's Y-up rewrite; use it when the "
        "animation set attaches to a body cooked from FBX/ActorX.",
    )
    parser.add_argument("--overwrite", action="store_true")
    args = parser.parse_args()
    cancel = args.target_space == "actorx"

    if args.output.exists() and not args.overwrite:
        raise SystemExit(f"Output already exists: {args.output}")

    layout = read_chunks(args.psa)
    bone_size, source_bone_count, _ = layout["BONENAMES"]
    if bone_size != BONE_RECORD.size:
        raise SystemExit("Unsupported PSA BONENAMES layout")
    bone_payload = read_records(args.psa, layout["BONENAMES"])
    source_bones = [
        decode_name(bone_payload[i * bone_size : i * bone_size + 64])
        for i in range(source_bone_count)
    ]

    info_size, info_count, _ = layout["ANIMINFO"]
    if info_size != ANIM_INFO.size:
        raise SystemExit("Unsupported PSA ANIMINFO layout")
    info_payload = read_records(args.psa, layout["ANIMINFO"])
    selected = None
    for index in range(info_count):
        values = ANIM_INFO.unpack_from(info_payload, index * info_size)
        if decode_name(values[0]) == args.clip:
            selected = values
            break
    if selected is None:
        raise SystemExit(f"PSA has no sequence named {args.clip}")

    total_bones = selected[2]
    track_time = float(selected[7])
    rate = float(selected[8])
    start_bone = selected[9]
    first_frame = selected[10]
    frame_count = selected[11]
    if total_bones != source_bone_count or start_bone != 0 or frame_count <= 0:
        raise SystemExit(f"Unsupported PSA sequence contract: {args.clip}")
    if not math.isfinite(rate) or rate <= 0.0:
        raise SystemExit(f"Sequence has no usable rate: {args.clip}")
    if not math.isclose(track_time, float(frame_count), abs_tol=0.0001):
        raise SystemExit(
            f"Sequence track time does not match its frame count: {args.clip}"
        )

    joint_names, joint_parents, joint_nodes = load_gltf_rig(args.gltf)
    source_slot = {name: index for index, name in enumerate(source_bones)}
    unknown = [name for name in source_bones if name not in set(joint_names)]
    if unknown:
        raise SystemExit(
            "PSA animates bones the rig does not have: " + ", ".join(unknown[:8])
        )
    padded = [name for name in joint_names if name not in source_slot]

    key_size, key_count, _ = layout["ANIMKEYS"]
    if key_size != ANIM_KEY.size or key_count != info_total_frames(info_payload, info_size, info_count) * source_bone_count:
        raise SystemExit("PSA ANIMKEYS count does not match its sequence table")
    keys = read_key_slice(
        args.psa,
        layout["ANIMKEYS"],
        first_frame * source_bone_count,
        frame_count * source_bone_count,
    )
    scales = None
    if "SCALEKEYS" in layout:
        scale_size, scale_count, _ = layout["SCALEKEYS"]
        if scale_size != SCALE_KEY.size or scale_count != key_count:
            raise SystemExit("PSA SCALEKEYS count/layout mismatch")
        scales = read_key_slice(
            args.psa,
            layout["SCALEKEYS"],
            first_frame * source_bone_count,
            frame_count * source_bone_count,
        )

    # Bind pose fallbacks for the bones this clip never drives.
    bind_key: dict[int, bytes] = {}
    bind_scale: dict[int, bytes] = {}
    for slot, node in enumerate(joint_nodes):
        if joint_names[slot] in source_slot:
            continue
        position = gltf_position_to_psa(node.get("translation", [0.0, 0.0, 0.0]))
        # A rig node already holds a true rotation, so it never needs
        # un-conjugating the way a stored PSA key does.
        rotation = gltf_rotation_to_psa(node.get("rotation", [0.0, 0.0, 0.0, 1.0]))
        scale = node.get("scale", [1.0, 1.0, 1.0])
        if cancel:
            position = cancel_position(*position)
            rotation = cancel_rotation(*rotation)
            scale = cancel_scale(*scale)
        bind_key[slot] = ANIM_KEY.pack(*position, *rotation, 1.0)
        bind_scale[slot] = SCALE_KEY.pack(scale[0], scale[1], scale[2], 1.0)

    out_bone_count = len(joint_names)
    out_keys = bytearray()
    out_scales = bytearray() if scales is not None else None
    for frame in range(frame_count):
        row = frame * source_bone_count
        for slot, name in enumerate(joint_names):
            source = source_slot.get(name)
            if source is None:
                out_keys += bind_key[slot]
                if out_scales is not None:
                    out_scales += bind_scale[slot]
                continue
            at = (row + source) * key_size
            scale_at = (row + source) * SCALE_KEY.size
            if not cancel:
                out_keys += keys[at : at + key_size]
                if out_scales is not None:
                    out_scales += scales[scale_at : scale_at + SCALE_KEY.size]
                continue
            px, py, pz, qx, qy, qz, qw, time = ANIM_KEY.unpack_from(keys, at)
            if joint_parents[slot] >= 0:
                qx, qy, qz, qw = unconjugate(qx, qy, qz, qw)
            out_keys += ANIM_KEY.pack(
                *cancel_position(px, py, pz), *cancel_rotation(qx, qy, qz, qw), time
            )
            if out_scales is not None:
                sx, sy, sz, stime = SCALE_KEY.unpack_from(scales, scale_at)
                out_scales += SCALE_KEY.pack(*cancel_scale(sx, sy, sz), stime)

    out_bones = bytearray()
    child_count = [0] * out_bone_count
    for parent in joint_parents:
        if parent >= 0:
            child_count[parent] += 1
    for slot, name in enumerate(joint_names):
        source = source_slot.get(name)
        if source is not None:
            record = bytearray(
                bone_payload[source * bone_size : (source + 1) * bone_size]
            )
        else:
            node = joint_nodes[slot]
            position = gltf_position_to_psa(node.get("translation", [0.0, 0.0, 0.0]))
            rotation = gltf_rotation_to_psa(node.get("rotation", [0.0, 0.0, 0.0, 1.0]))
            record = bytearray(
                BONE_RECORD.pack(
                    encode_name(name, 64),
                    0,
                    0,
                    0,
                    *rotation,
                    *position,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                )
            )
        struct.pack_into("<64s", record, 0, encode_name(name, 64))
        struct.pack_into("<ii", record, 64, 0, child_count[slot])
        struct.pack_into("<i", record, 72, joint_parents[slot])
        out_bones += record

    out_info = bytearray(
        ANIM_INFO.pack(
            encode_name(args.clip, 64),
            encode_name("trimmed", 64),
            out_bone_count,
            selected[3],
            selected[4],
            selected[5],
            selected[6],
            float(frame_count),
            rate,
            0,
            0,
            frame_count,
        )
    )

    def chunk(name: str, record_size: int, record_count: int, payload: bytes) -> bytes:
        return CHUNK_HEADER.pack(
            encode_name(name, 20), 0, record_size, record_count
        ) + payload

    blob = bytearray()
    blob += chunk("ANIMHEAD", 0, 0, b"")
    blob += chunk("BONENAMES", bone_size, out_bone_count, bytes(out_bones))
    blob += chunk("ANIMINFO", ANIM_INFO.size, 1, bytes(out_info))
    blob += chunk(
        "ANIMKEYS", ANIM_KEY.size, frame_count * out_bone_count, bytes(out_keys)
    )
    if out_scales is not None:
        blob += chunk(
            "SCALEKEYS", SCALE_KEY.size, frame_count * out_bone_count, bytes(out_scales)
        )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(bytes(blob))

    report = {
        "sourcePsa": str(args.psa),
        "rigGltf": str(args.gltf),
        "clip": args.clip,
        "frameCount": frame_count,
        "rate": rate,
        "sourceBoneCount": source_bone_count,
        "outputBoneCount": out_bone_count,
        "paddedBones": padded,
        "hasScaleKeys": scales is not None,
        "targetSpace": args.target_space,
        "output": str(args.output),
    }
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(
            json.dumps(report, ensure_ascii=False, indent=1) + "\n", encoding="utf-8"
        )
    print(
        f"{args.output}: clip={args.clip} frames={frame_count} rate={rate} "
        f"bones={source_bone_count}->{out_bone_count} padded={len(padded)}"
    )
    return 0


def info_total_frames(payload: bytes, record_size: int, record_count: int) -> int:
    total = 0
    for index in range(record_count):
        values = ANIM_INFO.unpack_from(payload, index * record_size)
        total += values[11]
    return total


if __name__ == "__main__":
    sys.exit(main())
