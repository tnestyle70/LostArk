#!/usr/bin/env python3
"""List the animation sequences inside one or more PSA files.

This is the lookup step in front of append_psa_clip_to_wmodel.py: it names every
clip a PSA carries, with its frame count and rate, so a clip can be picked by
name instead of by index.

The parser quirk worth keeping
------------------------------
PSA is a chunk stream: 20-byte chunk id, then int32 typeFlag, int32 dataSize,
int32 dataCount, then dataCount * dataSize bytes. The leading `ANIMHEAD` chunk
has dataSize = 0, so a reader that stops on a zero size reports "0 sequences"
and hides the whole file. Only a negative size or count is a real end.

ANIMINFO entries are 168 bytes:
  name[64], group[64], TotalBones@128, RootInclude, KeyCompressionStyle,
  KeyQuotum, KeyReduction@144, TrackTime@148, AnimRate@152, StartBone@156,
  FirstRawFrame@160, NumRawFrames@164

Usage:
  python list_psa_sequences.py <file-or-directory> [...] [--filter hurray]
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

ANIM_INFO_SIZE = 168


def sequences(path: Path):
    raw = path.read_bytes()
    at, out = 0, []
    while at + 32 <= len(raw):
        chunk_id = raw[at:at + 20].split(b"\0")[0].decode("ascii", "ignore")
        _flag, size, count = struct.unpack_from("<iii", raw, at + 20)
        if size < 0 or count < 0:
            break
        at += 32
        body = raw[at:at + size * count]
        at += size * count
        if chunk_id.upper().startswith("ANIMINFO") and size == ANIM_INFO_SIZE:
            for i in range(count):
                entry = body[i * size:(i + 1) * size]
                name = entry[:64].split(b"\0")[0].decode("ascii", "ignore")
                rate = struct.unpack_from("<f", entry, 152)[0]
                first, frames = struct.unpack_from("<ii", entry, 160)
                out.append((name, frames, rate, first))
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("paths", nargs="+", type=Path)
    parser.add_argument("--filter", default="",
                        help="only list clips whose name contains this")
    args = parser.parse_args()

    files = []
    for path in args.paths:
        files.extend(sorted(path.rglob("*.psa")) if path.is_dir() else [path])

    for psa in files:
        rows = [r for r in sequences(psa)
                if args.filter.lower() in r[0].lower()]
        print("\n=== %s : %d clips ===" % (psa, len(rows)))
        for name, frames, rate, first in rows:
            print("   %-44s %5d frames @ %6.2f fps  first %d"
                  % (name, frames, rate, first))
    return 0


if __name__ == "__main__":
    sys.exit(main())
