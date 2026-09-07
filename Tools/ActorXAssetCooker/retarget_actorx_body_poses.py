"""Bake explicitly selected humanoid ActorX poses onto a different PSK skeleton.

This is an offline authoring step before Cook-ActorXWModel, never a runtime rig
attachment. It transfers common bip001 body rotations in skeleton space, with
both bind poses removed/applied, and preserves the target's local bone offsets.
Target-only face, cloth, twist, weapon and effect bones retain their bind pose.
It does not claim to reproduce the original target character's animation.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path
import struct

from build_actorx_fbx import read_psa_metadata


HEADER = struct.Struct("<20s3i")
INFO = struct.Struct("<64s64s4i3f3i")
KEY = struct.Struct("<8f")


def read_chunks(path):
    data = path.read_bytes()
    result = {}
    offset = 0
    while offset < len(data):
        name, flag, size, count = HEADER.unpack_from(data, offset)
        offset += HEADER.size
        name = name.split(b"\0", 1)[0].decode("ascii")
        if size < 0 or count < 0 or offset + size * count > len(data):
            raise ValueError(f"Invalid chunk in {path}: {name}")
        if name in result:
            raise ValueError(f"Duplicate chunk in {path}: {name}")
        result[name] = (flag, size, count, data[offset:offset + size * count])
        offset += size * count
    return result


def read_bones(chunk):
    _, size, count, data = chunk
    if size != 120 or count < 1:
        raise ValueError("Expected a nonempty ActorX 120-byte skeleton")
    result = []
    for index in range(count):
        offset = index * size
        name = data[offset:offset + 64].split(b"\0", 1)[0].decode("ascii")
        parent = struct.unpack_from("<i", data, offset + 72)[0]
        if (index == 0 and parent != 0) or (index > 0 and not 0 <= parent < index):
            raise ValueError(f"Skeleton is not in parent-first order: {name}")
        result.append({"name": name, "parent": parent,
                       "rotation": normalize(struct.unpack_from("<4f", data, offset + 76)),
                       "position": struct.unpack_from("<3f", data, offset + 92)})
    if len({bone["name"] for bone in result}) != len(result):
        raise ValueError("Duplicate skeleton bone names")
    return result


def normalize(value):
    length = math.sqrt(sum(x * x for x in value))
    if not math.isfinite(length) or length < 1e-8:
        raise ValueError("Nonfinite or zero quaternion")
    return tuple(x / length for x in value)


def inverse(q):
    return (-q[0], -q[1], -q[2], q[3])


def multiply(a, b):
    x, y, z, w = a
    u, v, s, t = b
    return normalize((w*u + x*t + y*s - z*v,
                      w*v - x*s + y*t + z*u,
                      w*s + x*v - y*u + z*t,
                      w*t - x*u - y*v - z*s))


def world_rotations(bones, raw_rotations):
    # Matches the ActorX importer's bDontInvertRoot=True convention.
    result = []
    for index, (bone, raw) in enumerate(zip(bones, raw_rotations)):
        local = normalize(raw) if index == 0 else inverse(normalize(raw))
        result.append(local if index == 0 else multiply(result[bone["parent"]], local))
    return result


def retarget_rotations(source_world, source_rest, target_rest, target, mapping):
    result, target_world = [], []
    for index, bone in enumerate(target):
        source_index = mapping.get(index)
        if source_index is None:
            raw = bone["rotation"]
            local = raw if index == 0 else inverse(raw)
            world = local if index == 0 else multiply(target_world[bone["parent"]], local)
        else:
            delta = multiply(source_world[source_index], inverse(source_rest[source_index]))
            world = multiply(delta, target_rest[index])
            local = world if index == 0 else multiply(inverse(target_world[bone["parent"]]), world)
            raw = local if index == 0 else inverse(local)
        result.append(raw)
        target_world.append(world)
    return result


def run(args):
    source_path, target_path, animation_path = map(Path, (args.source_psk, args.target_psk, args.source_psa))
    output, receipt = Path(args.output_psa), Path(args.receipt)
    if output.exists() or receipt.exists():
        raise ValueError("Use new staging output paths; existing outputs are not overwritten")
    source = read_bones(read_chunks(source_path)["REFSKELT"])
    target_chunk = read_chunks(target_path)["REFSKELT"]
    target = read_bones(target_chunk)
    source_by_name = {bone["name"]: index for index, bone in enumerate(source)}
    metadata = read_psa_metadata(animation_path)
    if metadata["bones"] != [bone["name"] for bone in source]:
        raise ValueError("Source PSA/PSK skeleton order differs")
    expected_parents = [-1] + [bone["parent"] for bone in source[1:]]
    # UModel's PSA export may contain a flat placeholder hierarchy. In that
    # format animation tracks still follow the exact PSK bone order; the PSK
    # remains the only usable hierarchy/rest-pose source.
    placeholder_parents = [-1] + [0] * (len(source) - 1)
    if metadata["bone_parents"] not in (expected_parents, placeholder_parents):
        raise ValueError("Source PSA/PSK bone parents differ")
    mapping = {i: source_by_name[b["name"]] for i, b in enumerate(target)
               if b["name"] in source_by_name and (b["name"] == "b_root" or b["name"].startswith("bip001"))}
    source_body_ancestors = set(mapping.values())
    for index in list(source_body_ancestors):
        while index > 0:
            index = source[index]["parent"]
            source_body_ancestors.add(index)
    required = ("b_root", "bip001", "bip001-pelvis", "bip001-head", "bip001-neck",
                "bip001-l-upperarm", "bip001-r-upperarm", "bip001-l-hand", "bip001-r-hand",
                "bip001-l-thigh", "bip001-r-thigh", "bip001-l-foot", "bip001-r-foot")
    mapped_names = {target[index]["name"] for index in mapping}
    if any(name not in mapped_names for name in required):
        raise ValueError("Required humanoid body bones are missing")
    source_rest = world_rotations(source, [b["rotation"] for b in source])
    target_rest = world_rotations(target, [b["rotation"] for b in target])
    rest_roundtrip = retarget_rotations(source_rest, source_rest, target_rest, target, mapping)
    rest_error = max(1 - abs(sum(x*y for x, y in zip(q, b["rotation"])))
                     for q, b in zip(rest_roundtrip, target))
    if rest_error > 1e-6:
        raise ValueError("Rest-pose transfer did not reproduce the target bind pose")
    source_pelvis = source_by_name["bip001"]
    target_pelvis = next(i for i, b in enumerate(target) if b["name"] == "bip001")
    height_ratio = math.dist(target[target_pelvis]["position"], (0, 0, 0)) / math.dist(source[source_pelvis]["position"], (0, 0, 0))
    sequence_map = {s["name"]: s for s in metadata["sequences"]}
    source_keys = read_chunks(animation_path)["ANIMKEYS"][3]
    animation_info, animation_keys, clips = bytearray(), bytearray(), []
    frame_offset, output_names = 0, set()
    bindings = [(binding, False) for binding in args.clip] + [(binding, True) for binding in args.pose]
    if not bindings:
        raise ValueError("At least one explicit clip or pose binding is required")
    for binding, compose_pose in bindings:
        donor_name, target_name = binding.split("=", 1)
        if not target_name.startswith("project_tuned_") or len(target_name.encode("ascii")) > 63:
            raise ValueError("Output clip must identify project_tuned_ provenance")
        if target_name in output_names:
            raise ValueError("Duplicate target clip name")
        output_names.add(target_name)
        sequences = ([sequence_map[donor_name + suffix] for suffix in ("_start", "_loop", "_end")]
                     if compose_pose else [sequence_map[donor_name]])
        for sequence in sequences:
            if sequence["source_scale_minimum"] != [1.0, 1.0, 1.0] or sequence["source_scale_maximum"] != [1.0, 1.0, 1.0]:
                raise ValueError(f"Nonunit source scale needs a separate retarget policy: {sequence['name']}")
        rate = sequences[0]["animation_rate"]
        if any(s["animation_rate"] != rate for s in sequences):
            raise ValueError("Pose phases must have the same sample rate")
        if compose_pose:
            start, loop, end = sequences
            loop_frames = round(args.loop_seconds * rate)
            if loop_frames < 1 or loop["raw_frame_count"] < 2:
                raise ValueError("Pose loop interval must contain at least one frame")
            frame_references = ([(start, i) for i in range(start["raw_frame_count"] - 1)] +
                                [(loop, i % (loop["raw_frame_count"] - 1)) for i in range(loop_frames)] +
                                [(end, i) for i in range(end["raw_frame_count"])])
        else:
            frame_references = [(sequences[0], i) for i in range(sequences[0]["raw_frame_count"])]
        frames = len(frame_references)
        if frames < 2 or not rate > 0:
            raise ValueError("Invalid source clip timing")
        animation_info.extend(INFO.pack(target_name.encode(), b"None", len(target), 0, 0,
                                        frames * len(target), 0., float(frames), rate, 0, frame_offset, frames))
        previous = None
        for sequence, frame in frame_references:
            first = (sequence["first_raw_frame"] + frame) * len(source)
            keys = [KEY.unpack_from(source_keys, (first + i) * KEY.size) for i in range(len(source))]
            # Unmapped donor weapon/effect branches can contain deliberately
            # invalid hidden transforms. They are not input to a body retarget.
            source_world = world_rotations(source, [k[3:7] if i in source_body_ancestors else source[i]["rotation"]
                                                    for i, k in enumerate(keys)])
            rotations = retarget_rotations(source_world, source_rest, target_rest, target, mapping)
            for index, (bone, q) in enumerate(zip(target, rotations)):
                if previous is not None and sum(a*b for a, b in zip(previous[index], q)) < 0:
                    q = tuple(-x for x in q)
                    rotations[index] = q
                position = bone["position"]
                if index == target_pelvis:
                    position = tuple(position[a] + height_ratio * (keys[source_pelvis][a] - source[source_pelvis]["position"][a]) for a in range(3))
                if not all(math.isfinite(v) for v in (*position, *q)):
                    raise ValueError("Nonfinite baked transform")
                animation_keys.extend(KEY.pack(*position, *q, 1 / rate))
            previous = rotations
        clips.append({"sourceClips": [s["name"] for s in sequences], "targetClip": target_name,
                      "middleLoopSeconds": args.loop_seconds if compose_pose else None,
                      "frames": frames, "fps": rate, "durationSeconds": (frames - 1) / rate})
        frame_offset += frames
    target_bone_records = bytearray(target_chunk[3])
    struct.pack_into("<i", target_bone_records, 72, -1)
    result = bytearray(HEADER.pack(b"ANIMHEAD", 1999801, 0, 0))
    for name, size, count, payload in ((b"BONENAMES", 120, len(target), target_bone_records),
                                      (b"ANIMINFO", INFO.size, len(clips), animation_info),
                                      (b"ANIMKEYS", KEY.size, frame_offset * len(target), animation_keys)):
        result.extend(HEADER.pack(name, 1999801, size, count))
        result.extend(payload)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(result)
    checked = read_psa_metadata(output)
    if checked["bones"] != [b["name"] for b in target] or len(checked["sequences"]) != len(clips):
        raise ValueError("Baked target skeleton/clip validation failed")
    report = {"provenance": "PROJECT_TUNED_OFFLINE_RETARGET", "visualApproval": "PENDING_USER",
              "sourcePsk": str(source_path), "targetPsk": str(target_path), "sourcePsa": str(animation_path),
              "sourcePsaSha256": hashlib.sha256(animation_path.read_bytes()).hexdigest(),
              "outputPsa": str(output), "outputSha256": hashlib.sha256(result).hexdigest(),
              "sourceBoneCount": len(source), "targetBoneCount": len(target),
              "mappedBodyBones": sorted(mapped_names), "mappedBodyBoneCount": len(mapping),
              "targetLocalOffsetsPreserved": True, "pelvisTranslationScale": height_ratio,
              "targetRestRoundtripQuaternionError": rest_error,
              "targetOnlyBones": "bind local transform; no donor face, cloth, weapon or effect tracks",
              "rotationPolicy": "source animated world * inverse(source bind world) * target bind world; solve against animated target parent",
              "clips": clips}
    receipt.parent.mkdir(parents=True, exist_ok=True)
    receipt.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({key: report[key] for key in ("targetBoneCount", "mappedBodyBoneCount", "targetRestRoundtripQuaternionError", "clips")}))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("source-psk", "target-psk", "source-psa", "output-psa", "receipt"):
        parser.add_argument("--" + name, required=True)
    parser.add_argument("--clip", action="append", default=[], help="source_clip=project_tuned_name")
    parser.add_argument("--pose", action="append", default=[], help="source_phase_prefix=project_tuned_pose_name; joins _start, repeated _loop, _end")
    parser.add_argument("--loop-seconds", type=float, default=1.0)
    run(parser.parse_args())
