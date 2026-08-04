#!/usr/bin/env python3
"""Attach Resources-relative texture asset IDs to an extracted material map."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--material-map", type=Path, required=True)
    parser.add_argument("--export-receipt", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    return parser.parse_args()


def canonical_object_path(value: str) -> str:
    parts = [part for part in str(value or "").split(".") if part]
    if len(parts) < 2:
        return str(value or "").casefold()
    return f"{parts[0]}.{parts[-1]}".casefold()


def main() -> int:
    args = parse_args()
    material_map = json.loads(args.material_map.read_text(encoding="utf-8"))
    receipt = json.loads(args.export_receipt.read_text(encoding="utf-8"))
    runtime_paths: dict[str, str] = {}
    for row in receipt.get("rows", []):
        if not row.get("success") or row.get("kind") != "Texture2D":
            continue
        key = canonical_object_path(
            f"{row.get('logicalPackage', '')}.{row.get('objectName', '')}"
        )
        asset_id = str(row.get("assetId") or "")
        if not asset_id:
            continue
        previous = runtime_paths.get(key)
        if previous and previous.casefold() != asset_id.casefold():
            raise SystemExit(f"conflicting runtime paths for {key}: {previous} / {asset_id}")
        runtime_paths[key] = asset_id

    matched_entries = 0
    unresolved_paths: set[str] = set()
    for layers in material_map.get("materials", {}).values():
        for layer in layers:
            for texture in layer.get("textures", []):
                source_path = str(texture.get("texture") or "")
                asset_id = runtime_paths.get(canonical_object_path(source_path))
                if asset_id:
                    texture["dds_path"] = asset_id
                    matched_entries += 1
                elif source_path:
                    unresolved_paths.add(source_path)

    material_map["runtime_path_summary"] = {
        "receipt": str(args.export_receipt),
        "availableTextureCount": len(runtime_paths),
        "matchedMaterialTextureEntryCount": matched_entries,
        "unresolvedMaterialTexturePathCount": len(unresolved_paths),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(material_map, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    print(json.dumps(material_map["runtime_path_summary"]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
