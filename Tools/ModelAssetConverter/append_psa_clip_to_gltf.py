#!/usr/bin/env python3
"""Append one PSA sequence to a staged glTF as an extra animation.

Why this shape
--------------
The staged `<Class>.gltf` already carries the five clips the customizing screen
uses, at the scale the cooker expects (inject.json records the 100x it applied
to positions, inverse bind matrices and node translations). Rebuilding that from
`rig.gltf` would mean re-deriving the same scaling, so this starts from the
staged file and only adds to it: the existing animations, accessors and buffer
are copied through untouched and the new clip's data is appended to the end of
the buffer.

PSA layout used here
--------------------
  BONENAMES  120 bytes each, name in the first 64
  ANIMINFO   168 bytes each: name[64], group[64], TotalBones, RootInclude,
             KeyCompressionStyle, KeyQuotum, KeyReduction, TrackTime, AnimRate,
             StartBone, FirstRawFrame, NumRawFrames
  ANIMKEYS   32 bytes each: position float3, orientation float4, time float,
             indexed [FirstRawFrame * TotalBones + frame * TotalBones + bone]

A PSA bone is matched to a glTF node by name. Bones the glTF does not have are
skipped rather than guessed at, and the tool reports how many matched.

Usage:
  python append_psa_clip_to_gltf.py --gltf <staged.gltf> --psa <source.psa>
                                    --clip sc_hurray_1 --out <out.gltf>
                                    [--scale 100]
"""

from __future__ import annotations

import argparse
import json
import os
import struct
import sys
from pathlib import Path

BONE_SIZE = 120
ANIM_INFO_SIZE = 168
ANIM_KEY = struct.Struct("<3f4ff")          # position, quaternion, time
CHUNK = struct.Struct("<20siii")


def read_chunks(path: Path):
    """(id, entrySize, count, fileOffset) for every chunk, without reading bodies."""
    out = []
    with path.open("rb") as handle:
        while True:
            header = handle.read(32)
            if len(header) < 32:
                break
            name = header[:20].split(b"\0")[0].decode("ascii", "ignore")
            _flag, size, count = struct.unpack_from("<iii", header, 20)
            if size < 0 or count < 0:
                break
            out.append((name, size, count, handle.tell()))
            handle.seek(size * count, os.SEEK_CUR)
    return out


def load_psa_clip(path: Path, clip: str):
    chunks = {name: (size, count, at) for name, size, count, at in read_chunks(path)}
    if "BONENAMES" not in chunks or "ANIMINFO" not in chunks or "ANIMKEYS" not in chunks:
        raise SystemExit("%s is missing one of BONENAMES/ANIMINFO/ANIMKEYS" % path)

    with path.open("rb") as handle:
        size, count, at = chunks["BONENAMES"]
        handle.seek(at)
        raw = handle.read(size * count)
        bones = [raw[i * size:i * size + 64].split(b"\0")[0].decode("ascii", "ignore")
                 for i in range(count)]

        size, count, at = chunks["ANIMINFO"]
        handle.seek(at)
        raw = handle.read(size * count)
        info = None
        for i in range(count):
            entry = raw[i * size:(i + 1) * size]
            if entry[:64].split(b"\0")[0].decode("ascii", "ignore") != clip:
                continue
            total_bones, _root, _style, _quotum = struct.unpack_from("<iiii", entry, 128)
            _reduction, track_time, anim_rate = struct.unpack_from("<fff", entry, 144)
            _start_bone, first_frame, frames = struct.unpack_from("<iii", entry, 156)
            info = {"totalBones": total_bones, "trackTime": track_time,
                    "rate": anim_rate, "firstFrame": first_frame, "frames": frames}
            break
        if info is None:
            raise SystemExit("%s has no sequence named %s" % (path, clip))

        size, count, at = chunks["ANIMKEYS"]
        start = info["firstFrame"] * info["totalBones"]
        span = info["frames"] * info["totalBones"]
        if start + span > count:
            raise SystemExit("clip key range runs past ANIMKEYS")
        handle.seek(at + start * size)
        keys = handle.read(span * size)

    return bones, info, keys


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--gltf", type=Path, required=True)
    parser.add_argument("--psa", type=Path, required=True)
    parser.add_argument("--clip", required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--scale", type=float, default=100.0)
    args = parser.parse_args()

    document = json.loads(args.gltf.read_text(encoding="utf-8"))
    buffer_uri = document["buffers"][0]["uri"]
    buffer_path = args.gltf.parent / buffer_uri
    blob = bytearray(buffer_path.read_bytes())

    bones, info, keys = load_psa_clip(args.psa, args.clip)
    node_index = {node.get("name"): i for i, node in enumerate(document["nodes"])}

    times = [frame / info["rate"] for frame in range(info["frames"])]
    time_blob = struct.pack("<%df" % len(times), *times)

    def add_accessor(payload: bytes, component_count: int, kind: str,
                     minimum=None, maximum=None) -> int:
        while len(blob) % 4:
            blob.append(0)
        offset = len(blob)
        blob.extend(payload)
        document["bufferViews"].append(
            {"buffer": 0, "byteOffset": offset, "byteLength": len(payload)})
        accessor = {"bufferView": len(document["bufferViews"]) - 1,
                    "componentType": 5126, "count": component_count, "type": kind}
        if minimum is not None:
            accessor["min"], accessor["max"] = minimum, maximum
        document["accessors"].append(accessor)
        return len(document["accessors"]) - 1

    time_accessor = add_accessor(time_blob, len(times), "SCALAR",
                                 [times[0]], [times[-1]])

    channels, samplers = [], []
    matched = 0
    for bone_index, bone in enumerate(bones):
        node = node_index.get(bone)
        if node is None:
            continue
        matched += 1

        translations, rotations = [], []
        for frame in range(info["frames"]):
            at = (frame * info["totalBones"] + bone_index) * ANIM_KEY.size
            px, py, pz, qx, qy, qz, qw, _t = ANIM_KEY.unpack_from(keys, at)
            translations += [px * args.scale, py * args.scale, pz * args.scale]
            rotations += [qx, qy, qz, qw]

        t_accessor = add_accessor(
            struct.pack("<%df" % len(translations), *translations),
            info["frames"], "VEC3")
        r_accessor = add_accessor(
            struct.pack("<%df" % len(rotations), *rotations),
            info["frames"], "VEC4")

        for accessor, path_name in ((t_accessor, "translation"),
                                    (r_accessor, "rotation")):
            samplers.append({"input": time_accessor, "interpolation": "LINEAR",
                             "output": accessor})
            channels.append({"sampler": len(samplers) - 1,
                             "target": {"node": node, "path": path_name}})

    document.setdefault("animations", []).append(
        {"name": args.clip, "channels": channels, "samplers": samplers})
    document["buffers"][0]["byteLength"] = len(blob)

    out_buffer = args.out.parent / (args.out.stem + ".bin")
    document["buffers"][0]["uri"] = out_buffer.name
    args.out.parent.mkdir(parents=True, exist_ok=True)
    out_buffer.write_bytes(bytes(blob))
    args.out.write_text(json.dumps(document), encoding="utf-8")

    print("%s: %s  %d frames @ %.2f fps, %d/%d bones matched, %d channels"
          % (args.out.name, args.clip, info["frames"], info["rate"],
             matched, len(bones), len(channels)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
