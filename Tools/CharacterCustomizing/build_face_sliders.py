"""Build Data/Customizing/FaceSliders/<race>.facesliders.json from a retail face AnimSet PSA.

The retail character customizing drives its face sliders with additive animation
sequences named ``add_<slider>_ui`` inside each race's face AnimSet
(``pc_<race>_00_face_ani``).  Every sequence has three frames: frame 0 is one
extreme, frame 1 is the neutral identity pose and frame 2 is the other extreme.
A slider value of 0.5 therefore leaves the face untouched.

This tool reads the PSA exported by UModel, keeps only the bones a sequence
actually moves and writes the three per-bone keys (position in cm, rotation as
the PSA quaternion) so the Client can apply them through
``CModel::Set_BoneLocalMatrix`` without any engine-side animation cooking.

usage:
  python build_face_sliders.py <face.psa> <race> [-o <out.json>] [--include-expressions]
"""
from __future__ import annotations

import argparse
import json
import math
import struct
from pathlib import Path

CHUNK_HEADER = struct.Struct("<20siii")
BONE = struct.Struct("<64siii7f")
ANIM_INFO = struct.Struct("<64s64s4i3f3i")
ANIM_KEY = struct.Struct("<3f4ff")
POSITION_EPSILON_CM = 0.01
ROTATION_EPSILON_DEGREES = 0.2
NEUTRAL_FRAME = 1


def read_chunks(data: bytes) -> dict[str, tuple[int, int, bytes]]:
    chunks = {}
    at = 0
    while at + CHUNK_HEADER.size <= len(data):
        raw_id, _type_flag, size, count = CHUNK_HEADER.unpack_from(data, at)
        at += CHUNK_HEADER.size
        chunks[raw_id.split(b"\0", 1)[0].decode("ascii")] = (size, count, data[at:at + size * count])
        at += size * count
    return chunks


def decode_name(raw: bytes) -> str:
    return raw.split(b"\0", 1)[0].decode("windows-1252").rstrip(" ")


def quaternion_angle_degrees(a, b) -> float:
    dot = min(1.0, abs(sum(x * y for x, y in zip(a, b))))
    return math.degrees(2.0 * math.acos(dot))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("psa", type=Path)
    parser.add_argument("race")
    parser.add_argument("-o", "--output", type=Path)
    parser.add_argument("--include-expressions", action="store_true",
                        help="also emit add_fc_* expression poses as sliders")
    args = parser.parse_args()

    data = args.psa.read_bytes()
    chunks = read_chunks(data)
    for required in ("BONENAMES", "ANIMINFO", "ANIMKEYS"):
        if required not in chunks:
            raise SystemExit(f"PSA is missing {required}")

    size, count, raw = chunks["BONENAMES"]
    bones = []
    for index in range(count):
        fields = BONE.unpack_from(raw, index * size)
        bones.append(decode_name(fields[0]))
    bone_count = len(bones)

    size, count, raw = chunks["ANIMINFO"]
    sequences = []
    for index in range(count):
        fields = ANIM_INFO.unpack_from(raw, index * size)
        name = decode_name(fields[0])
        total_bones, _root, _compression, _quotum, _reduction, _track_time, rate, _start_bone, first_frame, frame_count = fields[2:]
        if total_bones != bone_count:
            raise SystemExit(f"sequence {name} has {total_bones} bones, PSA has {bone_count}")
        sequences.append((name, first_frame, frame_count, rate))

    key_size, key_count, key_raw = chunks["ANIMKEYS"]

    def key(frame: int, bone: int):
        fields = ANIM_KEY.unpack_from(key_raw, (frame * bone_count + bone) * key_size)
        return fields[0:3], fields[3:7]

    sliders = []
    for name, first_frame, frame_count, _rate in sequences:
        is_slider = name.startswith("add_") and name.endswith("_ui")
        is_expression = name.startswith("add_fc_")
        if not (is_slider or (args.include_expressions and is_expression)):
            continue
        if frame_count != 3 and is_slider:
            raise SystemExit(f"{name}: expected 3 frames, got {frame_count}")
        moved = []
        for bone in range(bone_count):
            keys = [key(first_frame + frame, bone) for frame in range(frame_count)]
            neutral_position, neutral_rotation = keys[min(NEUTRAL_FRAME, frame_count - 1)]
            significant = False
            for position, rotation in keys:
                if math.dist(position, neutral_position) > POSITION_EPSILON_CM:
                    significant = True
                if quaternion_angle_degrees(rotation, neutral_rotation) > ROTATION_EPSILON_DEGREES:
                    significant = True
            if not significant:
                continue
            moved.append({
                "bone": bones[bone],
                "keys": [
                    {"position": [round(v, 5) for v in position],
                     "rotation": [round(v, 6) for v in rotation]}
                    for position, rotation in keys
                ],
            })
        if not moved:
            continue
        slider_id = name[len("add_"):]
        if slider_id.endswith("_ui"):
            slider_id = slider_id[:-3]
        sliders.append({"id": slider_id, "sequence": name, "bones": moved})

    document = {
        "schema": "lostark.face-sliders",
        "formatVersion": 1,
        "race": args.race,
        "source": args.psa.name,
        "neutralFrame": NEUTRAL_FRAME,
        "positionUnit": "cm",
        "sliders": sliders,
    }
    output = args.output or Path(__file__).resolve().parents[2] / "Data" / "Customizing" / "FaceSliders" / f"{args.race}.facesliders.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(document, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    print(f"{output}: {len(sliders)} sliders, bones per slider: {[len(s['bones']) for s in sliders]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
