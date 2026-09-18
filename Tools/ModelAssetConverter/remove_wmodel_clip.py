#!/usr/bin/env python3
"""Drop one animation clip (WANM section) from a WModel animation set.

`append_psa_clip_to_wmodel.py` writes a new clip into a copy of an existing set, so the copy
still carries the donor's clips. `CModel::Attach_AnimationSet` rejects a set whose clip name the
body already has (the donor set is attached too), so a set built that way must be reduced to its
new clips. This rewrites the section table without the named section and renumbers the remaining
animation indices; every other section is copied byte for byte.

  python remove_wmodel_clip.py --wmodel <set.wmodel> --clip <clip name> --out <set.wmodel>
"""
from __future__ import annotations

import argparse
import struct
from pathlib import Path

FILE_HEADER = struct.Struct("<4sHHII")          # 'WINT', major, minor, flags, contentSize
MODEL_HEADER = struct.Struct("<4sIII4I")        # 'WMOD', sectionCount, animationCount, flags, reserved[4]
SECTION_DESC = struct.Struct("<IIQQ40s")        # type, index, offset (from content start), size, name
SECTION_ANIMATION = 4


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--wmodel", type=Path, required=True)
    ap.add_argument("--clip", required=True)
    ap.add_argument("--out", type=Path, required=True)
    a = ap.parse_args()

    data = a.wmodel.read_bytes()
    content = FILE_HEADER.size
    model = list(MODEL_HEADER.unpack_from(data, content))
    table = content + MODEL_HEADER.size
    count = model[1]
    sections = [list(SECTION_DESC.unpack_from(data, table + i * SECTION_DESC.size)) for i in range(count)]
    keep, dropped = [], []
    for s in sections:
        name = s[4].split(b"\0")[0].decode("utf-8", "ignore")
        (dropped if s[0] == SECTION_ANIMATION and name == a.clip else keep).append(s)
    if len(dropped) != 1:
        raise SystemExit("%s: expected one animation section named %s, found %d" % (a.wmodel.name, a.clip, len(dropped)))
    if not any(s[0] == SECTION_ANIMATION for s in keep):
        raise SystemExit("%s: removing %s would leave no animation clip" % (a.wmodel.name, a.clip))

    # Offsets are measured from the content start (the MODEL_HEADER); rebuild the body in the
    # kept order so the table shrinks by one row and the payloads stay contiguous.
    new_table_size = MODEL_HEADER.size + len(keep) * SECTION_DESC.size
    body = bytearray()
    rows = []
    animation_index = 0
    for s in keep:
        payload = data[content + s[2]:content + s[2] + s[3]]
        index = s[1]
        if s[0] == SECTION_ANIMATION:
            index = animation_index
            animation_index += 1
        rows.append(SECTION_DESC.pack(s[0], index, new_table_size + len(body), len(payload), s[4]))
        body += payload
    model[1] = len(keep)
    model[2] = animation_index
    header = FILE_HEADER.unpack_from(data, 0)
    rebuilt = bytearray(FILE_HEADER.pack(header[0], header[1], header[2], header[3], 0))
    rebuilt += MODEL_HEADER.pack(*model)
    for row in rows:
        rebuilt += row
    rebuilt += body
    struct.pack_into("<I", rebuilt, 12, len(rebuilt) - FILE_HEADER.size)
    a.out.write_bytes(rebuilt)
    print("%s: -%s, %d section(s), %d clip(s), %d -> %d bytes" % (
        a.out.name, a.clip, len(keep), animation_index, len(data), len(rebuilt)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
