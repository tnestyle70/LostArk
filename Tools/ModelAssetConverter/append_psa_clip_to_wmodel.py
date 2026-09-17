#!/usr/bin/env python3
"""Add one PSA sequence to an existing WModel animation set as a new clip.

Why this is not a cook
----------------------
`ModelAssetConverter.exe` does not read glTF animations -- it reports
`animations=0` even for the staged glTF that produced the shipped
`<Class>_CustomizingAnimSet.wmodel` -- so the tool that built those sets was
something else and is not in the repo. Rather than rebuild that, this writes the
WANM section straight into a copy of the set, which is the smaller and safer
change: the skeleton, the bind pose and the five existing clips are copied
through byte for byte.

WModel layout this relies on (all of it decoded by
verify_dimensionmaster_summon_bind_pose.py, which is what validates the result)

  FILE_HEADER   <4sHHII>   'WINT', major, minor, flags, size = filesize - 16
  MODEL_HEADER  <4sIII4I>  'WMOD', sectionCount, animationCount, ...
  SECTION_DESC  <IIQQ40s>  type, index, offset (from content start), size, name
  section       its own WINT wrapper, then the payload

  WANM          <4sIffIIB7s>  magic, channelCount, durationTicks,
                              ticksPerSecond, totalKeyCount, eventCount, ...
  channel       <QIIIIIIiI>   boneNameHash, posCount, posOffset, rotCount,
                              rotOffset, sclCount, sclOffset, -1, 0
  vector key    <4f>          tick, x, y, z
  quaternion    <5f>          tick, x, y, z, w

Key times are tick indices (0, 1, 2, ...) and ticksPerSecond carries the rate.
Every animation section ends with the same 8-byte constant, copied from the
section already in the file rather than guessed.

PSA -> cooked transform (measured, not assumed)
-----------------------------------------------
The shipped sets already contain four of these social actions (sc_charming_1,
sc_dance_1, sc_greet_3, sc_groupdance_3) and the same clips sit in the PSA, so
the original cooker's transform can be read off instead of guessed. Comparing
every bone of every frame of those four clips -- about 256,000 keys -- gives, to
float precision:

  position           (px, py, -pz)
  rotation, root     (-qx, -qy,  qz, qw)     PSA bone index 0
  rotation, other    ( qx,  qy, -qz, qw)

The two rotation rules differ because PSA stores non-root rotations conjugated,
the usual UE convention. One mirror through the Z plane applied to a conjugated
value negates only z; applied to the root's unconjugated value it negates x and
y. Writing the PSA quaternion straight through lays the character flat on the
floor -- which is exactly what happens if this measurement is skipped.

The cooked clips also carry channels for bones the PSA does not animate (hair,
tail and skirt -- 14 of them on the Lance Master rig), each constant at its rest
pose. Those are copied from a clip already in the file so a new clip drives the
same bone set as its neighbours.

Usage:
  python append_psa_clip_to_wmodel.py --wmodel <in.wmodel> --psa <src.psa>
                                      --clip sc_hurray_1 --out <out.wmodel>
"""

from __future__ import annotations

import argparse
import os
import struct
import sys
from pathlib import Path

FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
SKELETON_HEADER = struct.Struct("<4sII5I")
SKELETON_BONE = struct.Struct("<Q64si16fII27I")
ANIMATION_HEADER = struct.Struct("<4sIffIIB7s")
ANIMATION_CHANNEL = struct.Struct("<QIIIIIIiI")
VECTOR_KEY = struct.Struct("<4f")
QUATERNION_KEY = struct.Struct("<5f")

PSA_ANIM_KEY = struct.Struct("<3f4ff")
PSA_BONE_SIZE = 120
PSA_INFO_SIZE = 168


def psa_chunks(path: Path):
    with path.open("rb") as handle:
        while True:
            header = handle.read(32)
            if len(header) < 32:
                break
            name = header[:20].split(b"\0")[0].decode("ascii", "ignore")
            _flag, size, count = struct.unpack_from("<iii", header, 20)
            if size < 0 or count < 0:
                break
            yield name, size, count, handle.tell()
            handle.seek(size * count, os.SEEK_CUR)


def load_clip(path: Path, clip: str):
    chunks = {n: (s, c, a) for n, s, c, a in psa_chunks(path)}
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
            total_bones = struct.unpack_from("<i", entry, 128)[0]
            rate = struct.unpack_from("<f", entry, 152)[0]
            first_frame, frames = struct.unpack_from("<ii", entry, 160)
            info = {"bones": total_bones, "rate": rate,
                    "first": first_frame, "frames": frames}
            break
        if info is None:
            raise SystemExit("%s has no sequence %s" % (path, clip))

        size, count, at = chunks["ANIMKEYS"]
        start = info["first"] * info["bones"]
        span = info["frames"] * info["bones"]
        if start + span > count:
            raise SystemExit("clip key range runs past ANIMKEYS")
        handle.seek(at + start * size)
        keys = handle.read(span * size)
    return bones, info, keys


def read_donor_scales(data, content, donor):
    """Frame-0 scale of every donor channel, keyed by bone name hash.

    The cooked body clips do not leave scale at one: the armature bone (`flm`,
    `wgl`, ...) carries a constant 100 in all 224 shipped clips, which is the
    cm -> m the rest of the rig is authored in. Writing 1 there shrinks the
    whole character to a hundredth of its size for as long as the clip plays --
    it reads on screen as the character vanishing. Everything else is 1 to float
    jitter, so sampling the donor reproduces both cases without a special case.
    """
    at = content + donor[2] + FILE_HEADER.size
    head = ANIMATION_HEADER.unpack_from(data, at)
    channel_at = at + ANIMATION_HEADER.size
    key_at = channel_at + head[1] * ANIMATION_CHANNEL.size

    out = {}
    for i in range(head[1]):
        row = ANIMATION_CHANNEL.unpack_from(
            data, channel_at + i * ANIMATION_CHANNEL.size)
        if row[0] in out or 0 == row[5]:
            continue
        out[row[0]] = VECTOR_KEY.unpack_from(data, key_at + row[6])[1:]
    return out


def read_constant_channels(data, content, donor, psa_bone_names, name_by_hash):
    """Donor channels for bones the PSA does not animate: (hash, position, rotation).

    Each of these is constant in the shipped clips -- the bone's rest pose -- so
    one sampled key reproduces it. A donor channel that is not constant is
    skipped rather than frozen at some arbitrary frame.
    """
    at = content + donor[2] + FILE_HEADER.size
    head = ANIMATION_HEADER.unpack_from(data, at)
    channel_at = at + ANIMATION_HEADER.size
    key_at = channel_at + head[1] * ANIMATION_CHANNEL.size

    out, seen = [], set()
    for i in range(head[1]):
        row = ANIMATION_CHANNEL.unpack_from(
            data, channel_at + i * ANIMATION_CHANNEL.size)
        if row[0] in seen or name_by_hash.get(row[0]) in psa_bone_names:
            continue
        seen.add(row[0])
        positions = [VECTOR_KEY.unpack_from(
            data, key_at + row[2] + k * VECTOR_KEY.size)[1:] for k in range(row[1])]
        rotations = [QUATERNION_KEY.unpack_from(
            data, key_at + row[4] + k * QUATERNION_KEY.size)[1:] for k in range(row[3])]
        if not positions or not rotations:
            continue
        if any(p != positions[0] for p in positions):
            continue
        if any(q != rotations[0] for q in rotations):
            continue
        out.append((row[0], positions[0], rotations[0]))
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--wmodel", type=Path, required=True)
    parser.add_argument("--psa", type=Path, required=True)
    parser.add_argument("--clip", required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--name", help="section name written into the set (default: the PSA clip "
                                       "name); the body cooks name clips <armature>_<action>")
    parser.add_argument("--scale", type=float, default=1.0,
                        help="applied to PSA translations; the shipped sets are "
                             "already in the cooked unit, so default is 1")
    args = parser.parse_args()
    section_name = args.name or args.clip

    data = bytearray(args.wmodel.read_bytes())
    content = FILE_HEADER.size
    # The outer header's version is the geometry format version, and the decoder
    # rejects the file when it stops matching the WMSH section's own version
    # ("WMOD and WMSH geometry format versions do not match"). Body cooks are
    # 1.3 (skinned extra UV), so it has to be carried through, not rewritten.
    source_header = FILE_HEADER.unpack_from(data, 0)
    version_major, version_minor, header_flags = source_header[1:4]
    model = list(MODEL_HEADER.unpack_from(data, content))
    table = content + MODEL_HEADER.size
    section_count = model[1]
    sections = [list(SECTION_DESC.unpack_from(data, table + i * SECTION_DESC.size))
                for i in range(section_count)]

    if any(s[4].split(b"\0")[0].decode("utf-8", "ignore") == section_name
           for s in sections):
        raise SystemExit("%s already contains %s" % (args.wmodel.name, section_name))

    # Bone name -> hash, from the skeleton the clips are authored against.
    skeleton = next(s for s in sections if s[0] == 3)
    at = content + skeleton[2] + FILE_HEADER.size
    head = SKELETON_HEADER.unpack_from(data, at)
    at += SKELETON_HEADER.size
    hash_by_name, name_by_hash = {}, {}
    for i in range(head[1]):
        row = SKELETON_BONE.unpack_from(data, at + i * SKELETON_BONE.size)
        name = row[1].split(b"\0")[0].decode("utf-8", "ignore")
        hash_by_name[name] = row[0]
        name_by_hash[row[0]] = name

    bones, info, keys = load_clip(args.psa, args.clip)

    # Every animation section in the file ends with the same 8 bytes; take them
    # from one that is already there instead of inventing a value.
    donor = next(s for s in sections if s[0] == 4)
    trailer = bytes(data[content + donor[2] + donor[3] - 8:content + donor[2] + donor[3]])

    carried = read_constant_channels(data, content, donor, set(bones), name_by_hash)
    scale_by_hash = read_donor_scales(data, content, donor)

    channels, key_blob = bytearray(), bytearray()
    matched = 0
    for bone_index, bone in enumerate(bones):
        name_hash = hash_by_name.get(bone)
        if name_hash is None:
            continue
        matched += 1
        pos_at, rot_at, scl_at = len(key_blob), 0, 0

        for frame in range(info["frames"]):
            px, py, pz, _qx, _qy, _qz, _qw, _t = PSA_ANIM_KEY.unpack_from(
                keys, (frame * info["bones"] + bone_index) * PSA_ANIM_KEY.size)
            key_blob += VECTOR_KEY.pack(float(frame), px * args.scale,
                                        py * args.scale, -pz * args.scale)
        rot_at = len(key_blob)
        for frame in range(info["frames"]):
            _px, _py, _pz, qx, qy, qz, qw, _t = PSA_ANIM_KEY.unpack_from(
                keys, (frame * info["bones"] + bone_index) * PSA_ANIM_KEY.size)
            if 0 == bone_index:
                key_blob += QUATERNION_KEY.pack(float(frame), -qx, -qy, qz, qw)
            else:
                key_blob += QUATERNION_KEY.pack(float(frame), qx, qy, -qz, qw)
        scl_at = len(key_blob)
        scale = scale_by_hash.get(name_hash, (1.0, 1.0, 1.0))
        for frame in range(info["frames"]):
            key_blob += VECTOR_KEY.pack(float(frame), *scale)

        channels += ANIMATION_CHANNEL.pack(
            name_hash, info["frames"], pos_at, info["frames"], rot_at,
            info["frames"], scl_at, -1, 0)

    if matched == 0:
        raise SystemExit("no PSA bone matched the skeleton")

    for name_hash, position, rotation in carried:
        pos_at = len(key_blob)
        for frame in range(info["frames"]):
            key_blob += VECTOR_KEY.pack(float(frame), *position)
        rot_at = len(key_blob)
        for frame in range(info["frames"]):
            key_blob += QUATERNION_KEY.pack(float(frame), *rotation)
        scl_at = len(key_blob)
        scale = scale_by_hash.get(name_hash, (1.0, 1.0, 1.0))
        for frame in range(info["frames"]):
            key_blob += VECTOR_KEY.pack(float(frame), *scale)
        channels += ANIMATION_CHANNEL.pack(
            name_hash, info["frames"], pos_at, info["frames"], rot_at,
            info["frames"], scl_at, -1, 0)

    channel_count = matched + len(carried)
    payload = bytearray()
    payload += ANIMATION_HEADER.pack(
        b"WANM", channel_count, float(info["frames"] - 1), float(info["rate"]),
        channel_count * info["frames"] * 3, 0, 0, b"\0" * 7)
    payload += channels
    payload += key_blob
    payload += trailer
    donor_wrapper = FILE_HEADER.unpack_from(data, content + donor[2])
    section = FILE_HEADER.pack(b"WINT", donor_wrapper[1], donor_wrapper[2],
                               donor_wrapper[3], len(payload)) + bytes(payload)

    # One more descriptor row pushes every existing section forward by its size.
    # Section offsets are measured from the start of the content block (the
    # MODEL_HEADER), not from the start of the section data, so the appended
    # section sits after the header, the grown table and the existing body.
    shift = SECTION_DESC.size
    body = bytes(data[table + section_count * SECTION_DESC.size:])
    new_offset = (MODEL_HEADER.size
                  + (section_count + 1) * SECTION_DESC.size
                  + len(body))

    rebuilt = bytearray()
    rebuilt += FILE_HEADER.pack(b"WINT", version_major, version_minor,
                                header_flags, 0)              # size patched below
    model[1] = section_count + 1
    model[2] = model[2] + 1
    rebuilt += MODEL_HEADER.pack(*model)
    for s in sections:
        rebuilt += SECTION_DESC.pack(s[0], s[1], s[2] + shift, s[3], s[4])
    animation_index = max(s[1] for s in sections if s[0] == 4) + 1
    rebuilt += SECTION_DESC.pack(4, animation_index, new_offset, len(section),
                                 section_name.encode("utf-8")[:39].ljust(40, b"\0"))
    rebuilt += body
    rebuilt += section
    struct.pack_into("<I", rebuilt, 12, len(rebuilt) - FILE_HEADER.size)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(bytes(rebuilt))
    print("%s: +%s  %d frames @ %.2f fps, %d/%d PSA bones + %d carried "
          "= %d channels, file %d -> %d"
          % (args.out.name, section_name, info["frames"], info["rate"], matched,
             len(bones), len(carried), channel_count, len(data), len(rebuilt)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
