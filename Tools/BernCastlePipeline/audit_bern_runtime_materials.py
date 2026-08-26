from __future__ import annotations

"""Fail when a visible Bern placement would use the opaque gray material fallback."""

import argparse
import json
import math
import shlex
import struct
from collections import defaultdict
from pathlib import Path
from typing import Iterable


FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
SUBMESH_DESC = struct.Struct("<IIIIIQ20s")
BOUNDS = struct.Struct("<10f")


def fixed_wstring(blob: bytes) -> str:
    end = blob.find(b"\0\0")
    if end < 0:
        end = len(blob)
    elif end & 1:
        end += 1
    return blob[:end].decode("utf-16-le", errors="ignore").rstrip("\0")


def model_sections(data: bytes) -> dict[int, bytes]:
    outer = FILE_HEADER.unpack_from(data, 0)
    base = FILE_HEADER.size
    model = MODEL_HEADER.unpack_from(data, base)
    if outer[0] != b"WINT" or model[0] != b"WMOD":
        raise ValueError("not a WModel container")
    table = base + MODEL_HEADER.size
    result: dict[int, bytes] = {}
    for index in range(model[1]):
        kind, _, offset, size, _ = SECTION_DESC.unpack_from(
            data, table + index * SECTION_DESC.size
        )
        result[kind] = data[base + offset : base + offset + size]
    return result


def inspect_model(path: Path) -> tuple[bool, float, list[str]]:
    owned = model_sections(path.read_bytes())
    mesh = owned[1]
    material = owned[2]

    mesh_header = MESH_HEADER.unpack_from(mesh, FILE_HEADER.size)
    submesh_count = mesh_header[1]
    bounds_offset = (
        FILE_HEADER.size
        + MESH_HEADER.size
        + submesh_count * SUBMESH_DESC.size
        + mesh_header[5] * mesh_header[4]
        + mesh_header[6] * mesh_header[7]
        + mesh_header[2] * 128
    )
    radius = 0.0
    if mesh_header[8]:
        for row in range(submesh_count):
            values = BOUNDS.unpack_from(mesh, bounds_offset + row * BOUNDS.size)
            radius = max(radius, values[9])

    magic, material_count = struct.unpack_from("<4sI", material, FILE_HEADER.size)
    entry_size = {b"WMAT": 596, b"WMA2": 4756, b"WMA3": 5376}.get(magic)
    if entry_size is None:
        raise ValueError(f"unsupported material section: {magic!r}")
    missing_visible_color = True
    names: list[str] = []
    for row in range(material_count):
        begin = FILE_HEADER.size + 8 + row * entry_size
        names.append(
            material[begin + 12 : begin + 76]
            .split(b"\0", 1)[0]
            .decode("utf-8", errors="ignore")
        )
        base_color = fixed_wstring(material[begin + 76 : begin + 596])
        emissive_begin = begin + 76 + 3 * 520
        emissive = fixed_wstring(material[emissive_begin : emissive_begin + 520])
        if base_color or emissive:
            missing_visible_color = False
    return missing_visible_color, radius, names


def audit(imported_root: Path, resource_root: Path) -> list[dict[str, object]]:
    assets: dict[str, list[str]] = {}
    for path in imported_root.glob("*.mapassets"):
        for line in path.read_text(encoding="utf-8").splitlines()[1:]:
            values = shlex.split(line, posix=True)
            assets.setdefault(values[0], values)

    placements: dict[str, list[tuple[float, float, float]]] = defaultdict(list)
    for path in imported_root.glob("*.mapplacements"):
        for line in path.read_text(encoding="utf-8").splitlines()[1:]:
            values = shlex.split(line, posix=True)
            if values[-1] == "1":
                placements[values[4]].append(tuple(map(float, values[12:15])))

    failures: list[dict[str, object]] = []
    for asset_id, scales in sorted(placements.items()):
        catalog = assets.get(asset_id)
        if catalog is None:
            raise ValueError(f"visible placement references missing catalog asset: {asset_id}")
        model_path = resource_root / catalog[2]
        missing_color, local_radius, material_names = inspect_model(model_path)
        if not missing_color:
            continue
        world_radius = max(
            local_radius * max(abs(component) for component in scale)
            for scale in scales
        )
        if not math.isfinite(world_radius):
            raise ValueError(f"non-finite world radius: {asset_id}")
        failures.append(
            {
                "assetId": asset_id,
                "placementCount": len(scales),
                "maximumWorldRadius": round(world_radius, 6),
                "materials": material_names,
            }
        )
    return failures


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--imported-root",
        type=Path,
        default=root / "Data/Maps/Imported/LV_BER_BERNCASTLE",
    )
    parser.add_argument(
        "--resource-root",
        type=Path,
        default=root / "Client/Bin/Resources",
    )
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    failures = audit(args.imported_root, args.resource_root)
    result = {
        "status": "passed" if not failures else "failed",
        "visibleGrayFallbackAssetCount": len(failures),
        "failures": failures,
    }
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
