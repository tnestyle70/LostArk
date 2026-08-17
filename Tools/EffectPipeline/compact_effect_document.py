"""Strip insignificant JSON whitespace from a sealed Effect document.

The publisher seals authored Effect documents into
Client/Bin/DataFiles/Effect/Authored/ and the runtime reparses them on the
main-thread prewarm seam. The authored documents are pretty-printed for git
review, which is more than half of their bytes: across the 259 authored
documents 113.4 MB of 210.9 MB is indentation.

This is a lexical transform, not a JSON round trip. Number tokens are copied
byte for byte, so 0.20000000298023224 stays exactly that, and key order is
preserved because nothing is reordered. A parse/serialize round trip through
either PowerShell or json.dumps would reformat floats and could reorder keys,
and the runtime codec compares keys with exact order.

Usage: compact_effect_document.py <sourcePath> <destinationPath>
"""

import sys


def compact(payload: bytes) -> bytes:
    out = bytearray()
    in_string = False
    escaped = False
    for byte in payload:
        if in_string:
            out.append(byte)
            if escaped:
                escaped = False
            elif byte == 0x5C:  # backslash
                escaped = True
            elif byte == 0x22:  # quote
                in_string = False
            continue
        if byte == 0x22:
            in_string = True
            out.append(byte)
            continue
        # space, tab, carriage return, line feed between tokens
        if byte in (0x20, 0x09, 0x0D, 0x0A):
            continue
        out.append(byte)
    if in_string:
        raise ValueError("document ended inside a JSON string literal")
    return bytes(out)


def main() -> int:
    if len(sys.argv) != 3:
        sys.stderr.write(
            "usage: compact_effect_document.py <source> <destination>\n")
        return 2
    with open(sys.argv[1], "rb") as handle:
        payload = handle.read()
    if not payload:
        sys.stderr.write("compact_effect_document: empty input\n")
        return 1
    try:
        compacted = compact(payload)
    except ValueError as error:
        sys.stderr.write("compact_effect_document: %s\n" % error)
        return 1
    # A compacted document must still parse and must carry the same value
    # tree as the input. This is a self-check on the transform, not the
    # publisher's schema validation.
    import json
    try:
        if json.loads(payload.decode("utf-8")) != json.loads(
                compacted.decode("utf-8")):
            sys.stderr.write(
                "compact_effect_document: value tree changed\n")
            return 1
    except Exception as error:  # noqa: BLE001 - report any decode/parse failure
        sys.stderr.write("compact_effect_document: %s\n" % error)
        return 1
    with open(sys.argv[2], "wb") as handle:
        handle.write(compacted)
    return 0


if __name__ == "__main__":
    sys.exit(main())
