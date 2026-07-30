#!/usr/bin/env python3
"""Synchronize a cooked WModel pack manifest with its on-disk deliverables.

The asset pack is the reproducible source of truth for runtime installation.
When material-v2 slots are added after the first cook, stale output hashes must
not remain in asset.manifest.json or .complete.json.  This tool updates only
material semantics and deliverable receipts; mesh/source provenance is kept.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import tempfile
from pathlib import Path, PurePosixPath
from typing import Any


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def atomic_write_json(path: Path, value: Any) -> None:
    handle, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as output:
            json.dump(value, output, ensure_ascii=False, separators=(",", ":"))
            output.write("\n")
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def receipt(root: Path, path: Path) -> dict[str, Any]:
    return {
        "bytes": path.stat().st_size,
        "path": path.relative_to(root).as_posix(),
        "sha256": sha256(path),
    }


def runtime_texture_path(diffuse: str, filename: str) -> str:
    return str(PurePosixPath(diffuse).parent / filename)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--pack", required=True, type=Path)
    parser.add_argument("--material-index", required=True, type=int)
    parser.add_argument("--normal", required=True)
    parser.add_argument("--emissive", required=True)
    parser.add_argument("--emissive-proof", required=True)
    args = parser.parse_args()

    manifest_path = args.pack / "asset.manifest.json"
    complete_path = args.pack / ".complete.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    materials = manifest["cookedSemantics"]["material"]["materials"]
    remaps = manifest["options"]["materialRemaps"]
    if not (0 <= args.material_index < len(materials)):
        parser.error("material-index is outside cookedSemantics.material.materials")
    if args.material_index >= len(remaps):
        parser.error("material-index is outside options.materialRemaps")

    normal_file = args.pack / "textures" / args.normal
    emissive_file = args.pack / "textures" / args.emissive
    if not normal_file.is_file() or not emissive_file.is_file():
        parser.error("normal/emissive texture is missing from the pack textures directory")

    diffuse = materials[args.material_index].get("diffuse")
    if not diffuse:
        parser.error("target material has no diffuse path from which to derive runtime root")
    normal_runtime = runtime_texture_path(diffuse, args.normal)
    emissive_runtime = runtime_texture_path(diffuse, args.emissive)
    for target in (materials[args.material_index], remaps[args.material_index]):
        target["normal"] = normal_runtime
        target["emissive"] = emissive_runtime

    deliverables = sorted((args.pack / "cooked").glob("*.wmat"))
    deliverables += sorted((args.pack / "cooked").glob("*.wmesh"))
    deliverables += sorted((args.pack / "textures").glob("*"))
    if not deliverables or any(not path.is_file() for path in deliverables):
        parser.error("pack has no complete cooked/texture deliverable set")
    manifest["outputs"] = [receipt(args.pack, path) for path in deliverables]
    manifest["materialAugmentation"] = {
        "normal": "exact UE3 texture converted without semantic synthesis",
        "emissive": args.emissive_proof,
        "emissiveTexture": emissive_runtime,
    }
    atomic_write_json(manifest_path, manifest)

    complete = {
        "files": manifest["outputs"],
        "manifestSha256": sha256(manifest_path),
        "schemaVersion": 1,
    }
    atomic_write_json(complete_path, complete)
    print(
        f"synchronized {args.pack.name}: "
        f"{len(deliverables)} deliverables, manifest={complete['manifestSha256']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
