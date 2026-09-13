#!/usr/bin/env python3
"""List the length-prefixed strings inside a ClientData `.loa` record file.

`.loa` files (IconInfo, UISoundTheme, Sequence_UI, ActionCategory ...) share one
shape: a float 1.0, a few int32 counts, then records built out of
`int32 length` + NUL-terminated bytes, with fixed-width numeric fields between
them. A full typed parser differs per file, but the string stream alone already
answers "what is in here and what does it reference", which is what a coverage
sweep needs before deciding whether a file matters.

Strings are CP949 in these tables (GameMsg is the UTF-8 exception).

Usage:
  python dump_loa_strings.py <file.loa> [--grep mvp] [--context 6] [--offsets]
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path


def strings(data: bytes):
    """(offset, text) for every plausible length-prefixed string."""
    out, at, size = [], 0, len(data)
    while at + 4 <= size:
        (length,) = struct.unpack_from("<i", data, at)
        if 1 <= length <= 512 and at + 4 + length <= size:
            raw = data[at + 4: at + 4 + length]
            if raw.endswith(b"\0") and b"\0" not in raw[:-1]:
                body = raw[:-1]
                if all(32 <= b or b in (9,) for b in body):
                    try:
                        out.append((at, body.decode("cp949")))
                    except UnicodeDecodeError:
                        out.append((at, body.decode("latin-1")))
                    at += 4 + length
                    continue
        at += 1
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("path", type=Path)
    parser.add_argument("--grep", default="",
                        help="only show matches (case-insensitive) and neighbours")
    parser.add_argument("--context", type=int, default=6)
    parser.add_argument("--offsets", action="store_true")
    args = parser.parse_args()

    found = strings(args.path.read_bytes())
    print("%s : %d strings" % (args.path.name, len(found)))

    if not args.grep:
        for offset, text in found:
            print(("%8d  " % offset if args.offsets else "") + text)
        return 0

    needle = args.grep.lower()
    shown = set()
    for i, (_offset, text) in enumerate(found):
        if needle not in text.lower():
            continue
        lo = max(0, i - args.context)
        hi = min(len(found), i + args.context + 1)
        if lo not in shown:
            print("---")
        for j in range(lo, hi):
            if j in shown:
                continue
            shown.add(j)
            mark = " <==" if needle in found[j][1].lower() else ""
            print(("%8d  " % found[j][0] if args.offsets else "")
                  + found[j][1] + mark)
    return 0


if __name__ == "__main__":
    sys.exit(main())
