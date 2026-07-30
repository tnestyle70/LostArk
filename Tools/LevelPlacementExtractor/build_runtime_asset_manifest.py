#!/usr/bin/env python3
"""Build the exact assetId -> cooked WModel manifest consumed by MapTool."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import tempfile
from pathlib import Path


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def atomic_write(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, name = tempfile.mkstemp(prefix=path.name + ".", suffix=".tmp", dir=path.parent)
    temporary = Path(name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as output:
            json.dump(value, output, ensure_ascii=False, indent=2)
            output.write("\n")
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--asset-manifest", required=True, type=Path)
    parser.add_argument("--pack-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    source = json.loads(args.asset_manifest.read_text(encoding="utf-8"))
    assets = source.get("assets", [])
    rows: list[dict[str, object]] = []
    seen: set[str] = set()
    for asset in sorted(assets, key=lambda row: str(row["assetId"])):
        asset_id = str(asset["assetId"])
        if asset_id in seen:
            raise ValueError(f"duplicate assetId: {asset_id}")
        seen.add(asset_id)
        relative = Path(asset_id) / f"{asset_id}.wmodel"
        model = args.pack_root / relative
        if not model.is_file():
            raise FileNotFoundError(model)
        if model.read_bytes()[:4] not in (b"WINT", b"WMOD"):
            raise ValueError(f"invalid WModel header: {model}")
        rows.append(
            {
                "assetId": asset_id,
                "model": relative.as_posix(),
                "bytes": model.stat().st_size,
                "sha256": sha256(model),
            }
        )

    result = {
        "schemaVersion": 1,
        "areaId": source["areaId"],
        "assetCount": len(rows),
        "assets": rows,
    }
    atomic_write(args.output, result)
    print(json.dumps({"assetCount": len(rows), "output": str(args.output)}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
