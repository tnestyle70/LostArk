#!/usr/bin/env python3
"""Move a single-submesh WModel onto material slot 0.

cook_npc.py --exclude drops the other slots' submeshes through the converter's
--exclude-material, but the surviving submesh keeps its original slot number.
Effect model cues reject that: Effect_DocumentRenderer_ResourceStaging.cpp
requires every mesh of a recovered skeletal material to sit on slot 0.

These WModels carry no WGEOMETRY or WUVS payload digest, so the descriptor is
rewritten in place. Files that already sit on slot 0 are left untouched.

usage:
  python normalize_model_cue_material_slot.py <wmodel> [...] [--check]
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII")
SECTION_DESC = struct.Struct("<IIQQ40s")
MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
SUBMESH = struct.Struct("<IIIIIQ20s")
SUBMESH_MATERIAL_OFFSET = 16


def mesh_sections(data: bytes) -> list[int]:
    magic, major, _minor, flags, content_size = FILE_HEADER.unpack_from(data, 0)
    if magic != b"WINT" or major != 1 or flags != 0:
        raise SystemExit("outer WINT header is invalid")
    if content_size != len(data) - FILE_HEADER.size:
        raise SystemExit("outer WINT content size does not match the file")
    model_at = FILE_HEADER.size
    model_magic, section_count, _animations, _model_flags = MODEL_HEADER.unpack_from(data, model_at)
    if model_magic != b"WMOD":
        raise SystemExit("WMOD metadata is missing")
    table = model_at + MODEL_HEADER.size + 16
    payloads = []
    for index in range(section_count):
        _type, _index, offset, _size, _name = SECTION_DESC.unpack_from(
            data, table + index * SECTION_DESC.size)
        payload = FILE_HEADER.size + offset + FILE_HEADER.size
        if data[payload:payload + 4] == b"WMSH":
            payloads.append(payload)
    return payloads


def normalize(path: Path, check: bool) -> bool:
    data = bytearray(path.read_bytes())
    payloads = mesh_sections(data)
    if len(payloads) != 1:
        raise SystemExit("%s: expected one WMSH section, found %d" % (path, len(payloads)))
    payload = payloads[0]
    _magic, submesh_count, _bones, _vflags, _stride, _vertices, *_rest = \
        MESH_HEADER.unpack_from(data, payload)
    if submesh_count != 1:
        raise SystemExit("%s: expected one submesh, found %d" % (path, submesh_count))
    at = payload + MESH_HEADER.size + SUBMESH_MATERIAL_OFFSET
    material_index = struct.unpack_from("<I", data, at)[0]
    if material_index == 0:
        print("%s: already on slot 0" % path.name)
        return False
    print("%s: slot %d -> 0%s" % (path.name, material_index, " (check only)" if check else ""))
    if check:
        return True
    backup = path.with_suffix(path.suffix + ".slot%d" % material_index)
    if not backup.exists():
        backup.write_bytes(bytes(data))
    struct.pack_into("<I", data, at, 0)
    path.write_bytes(bytes(data))
    return True


def main() -> int:
    argv = sys.argv[1:]
    check = "--check" in argv
    paths = [Path(a) for a in argv if a != "--check"]
    if not paths:
        print(__doc__)
        return 2
    changed = sum(normalize(path, check) for path in paths)
    print("changed=%d of %d" % (changed, len(paths)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
