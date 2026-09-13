"""Give a single-UV-set vehicle submesh the UV1 channel its retail material reads.

The Golden Terpeion mane (`mn_pmstg_01-3_mi`) compiles to source-character program 18, and
`CModel` refuses program 18 on a submesh without native UV1. The retail skeletal mesh carries
one UV set (the exported PSK has no EXTRAUVS chunk), and UE3 resolves a material's TexCoord[N]
past the last set by clamping to that set, so UV1 is the submesh's own UV0.

The source PSK is read to prove that single set before anything is written. The write goes
through `cook_skinned_uv_contract`, which bumps the file to 1.3 and proves the legacy stream,
skinning and every non-mesh section are unchanged.

usage:
  python cook_single_set_uv1.py --wmodel <Terpeion.wmodel> --material mn_pmstg_01-3_mi
      --source-psk <mn_pmstg_01_sk.psk> [--dry-run]
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "ModelAssetConverter"))
import cook_wmodel_geometry_contract as _cook  # noqa: E402

COOKED_TEXCOORD_OFFSET = 24
WMODEL_FILE_HEADER = struct.Struct("<4sHHII")
WMODEL_MODEL_HEADER = struct.Struct("<4sIII4I")
WMODEL_SECTION_DESC = struct.Struct("<IIQQ40s")
WMODEL_MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
WMODEL_SUBMESH_DESC = struct.Struct("<IIIIIQ20s")
WMODEL_MATERIAL_META = struct.Struct("<4sI")
WMODEL_MATERIAL_ENTRY = {
    b"WMAT": struct.Struct("<IQ64s520s"),
    b"WMA2": struct.Struct("<IQ64s" + "520s" * 9),
    b"WMA3": struct.Struct("<IQ64s" + "520s" * 10 + "16f"),
}


def sections(data: bytes, section_type: int):
    base = WMODEL_FILE_HEADER.size
    count = WMODEL_MODEL_HEADER.unpack_from(data, base)[1]
    for index in range(count):
        kind, _i, offset, _size, _name = WMODEL_SECTION_DESC.unpack_from(
            data, base + WMODEL_MODEL_HEADER.size + index * WMODEL_SECTION_DESC.size)
        if kind == section_type:
            yield base + offset + WMODEL_FILE_HEADER.size


def read_material_names(data: bytes) -> dict[int, str]:
    at = next(sections(data, 2))
    magic, count = WMODEL_MATERIAL_META.unpack_from(data, at)
    if magic not in WMODEL_MATERIAL_ENTRY:
        raise SystemExit("unknown material container %r" % magic)
    entry = WMODEL_MATERIAL_ENTRY[magic]
    at += WMODEL_MATERIAL_META.size
    names = {}
    for row in range(count):
        fields = entry.unpack_from(data, at + row * entry.size)
        names[fields[0]] = fields[2].split(b"\0")[0].decode("ascii", "replace").lower()
    return names


def read_submesh_materials(data: bytes) -> list[int]:
    payload = next(sections(data, 1))
    count = WMODEL_MESH_HEADER.unpack_from(data, payload)[1]
    base = payload + WMODEL_MESH_HEADER.size
    return [WMODEL_SUBMESH_DESC.unpack_from(data, base + row * WMODEL_SUBMESH_DESC.size)[4]
            for row in range(count)]


def psk_chunks(path: Path) -> list[str]:
    data = path.read_bytes()
    names, offset = [], 0
    while offset + 32 <= len(data):
        name = data[offset:offset + 20].split(b"\0")[0].decode("ascii", "replace")
        size, count = struct.unpack_from("<ii", data, offset + 24)
        names.append(name)
        offset += 32 + size * count
    if offset != len(data):
        raise SystemExit("%s: chunk table does not end at the file end" % path.name)
    return names


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--wmodel", type=Path, required=True)
    parser.add_argument("--material", required=True)
    parser.add_argument("--source-psk", type=Path, required=True)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    chunks = psk_chunks(args.source_psk)
    if "VTXW0000" not in chunks or any(name.startswith("EXTRAUV") for name in chunks):
        raise SystemExit("%s: source is not a single-UV-set skeletal mesh (%s)"
                         % (args.source_psk.name, ", ".join(chunks)))

    data = args.wmodel.read_bytes()
    material_names = read_material_names(data)
    submesh_materials = read_submesh_materials(data)
    selected = [index for index, material in enumerate(submesh_materials)
                if material_names.get(material) == args.material.lower()]
    if len(selected) != 1:
        raise SystemExit("%s: material %r names %d submeshes, expected exactly one (have %s)"
                         % (args.wmodel.name, args.material, len(selected),
                            sorted(set(material_names.values()))))
    submesh = selected[0]

    parsed = _cook.parse_skinned_uv_wmodel(data)
    if parsed["versionMinor"] != 0:
        raise SystemExit("%s: already 1.%d; cook from the legacy 1.0 file"
                         % (args.wmodel.name, parsed["versionMinor"]))
    offset, count = parsed["submeshes"][submesh][:2]
    stride = parsed["meshHeader"][4]
    base = parsed["vertexStart"] + offset
    uv0 = [struct.unpack_from("<2f", parsed["mesh"], base + i * stride + COOKED_TEXCOORD_OFFSET)
           for i in range(count)]

    written, receipt = _cook.cook_skinned_uv_contract(data, {submesh: {"TEXCOORD_1": uv0}})
    print("%s: submesh %d (%s), %d vertices, UV1 = UV0" % (args.wmodel.name, submesh,
                                                          args.material, count))
    print("  %s" % json.dumps(receipt, sort_keys=True)[:240])
    if args.dry_run:
        print("  dry run, nothing written")
        return 0
    args.wmodel.write_bytes(written)
    print("  written, %d bytes" % len(written))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
