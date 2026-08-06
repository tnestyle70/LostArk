#!/usr/bin/env python3
"""Export exact Mesh/Texture objects named by a class resource manifest."""

from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def export_requests(document: dict[str, Any]) -> dict[str, list[dict[str, Any]]]:
    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in document.get("assets", []):
        roles = set(row.get("roles", []))
        export_roles = sorted(roles & {"mesh", "texture"})
        if not export_roles:
            continue
        asset_path = str(row.get("sourceAssetPath", ""))
        package = row.get("logicalPackage")
        object_name = asset_path.rsplit(".", 1)[-1] if "." in asset_path else None
        if not package or not object_name:
            continue
        grouped[str(package)].append(
            {
                "sourceAssetPath": asset_path,
                "objectName": object_name,
                "roles": export_roles,
                "skillIds": row.get("skillIds", []),
            }
        )
    for rows in grouped.values():
        rows.sort(key=lambda row: row["sourceAssetPath"].casefold())
    return dict(sorted(grouped.items(), key=lambda item: item[0].casefold()))


def matching_outputs(
    root: Path, object_name: str, logical_package: str | None = None
) -> list[Path]:
    folded = object_name.casefold()
    return sorted(
        (
            path
            for path in root.rglob("*")
            if path.is_file()
            and (
                logical_package is None
                or (
                    path.relative_to(root).parts
                    and path.relative_to(root).parts[0].casefold()
                    == logical_package.casefold()
                )
            )
            and (
                path.stem.casefold() == folded
                or path.stem.casefold().startswith(folded + "_")
            )
        ),
        key=lambda path: path.as_posix().casefold(),
    )


def chunks(rows: list[dict[str, Any]], size: int) -> list[list[dict[str, Any]]]:
    return [rows[index : index + size] for index in range(0, len(rows), size)]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--umodel", required=True, type=Path)
    parser.add_argument("--package-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    parser.add_argument("--region", default="kr")
    parser.add_argument("--chunk-size", type=int, default=32)
    args = parser.parse_args()

    document = json.loads(args.manifest.read_text(encoding="utf-8-sig"))
    requests = export_requests(document)
    args.output.mkdir(parents=True, exist_ok=True)
    logs_root = args.output / "_logs"
    logs_root.mkdir(parents=True, exist_ok=True)

    invocations = []
    process_failures = []
    for package, rows in requests.items():
        for chunk_index, batch in enumerate(chunks(rows, args.chunk_size), start=1):
            command = [
                str(args.umodel),
                "-export",
                "-game=lostark",
                f"-{args.region}",
                "-nameresolve",
                f"-path={args.package_root}",
                f"-out={args.output}",
                "-dds",
                "-gltf",
                "-nooverwrite",
            ]
            command.extend(f"-obj={row['objectName']}" for row in batch)
            command.append(package)
            completed = subprocess.run(
                command,
                cwd=args.umodel.parent,
                text=True,
                encoding="utf-8",
                errors="replace",
                capture_output=True,
                check=False,
                creationflags=(
                    subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0
                ),
            )
            log_path = logs_root / f"{package}.{chunk_index:03d}.log"
            log_path.write_text(
                completed.stdout + "\n" + completed.stderr, encoding="utf-8"
            )
            invocation = {
                "logicalPackage": package,
                "chunkIndex": chunk_index,
                "requestedObjectCount": len(batch),
                "exitCode": completed.returncode,
                "logFile": log_path.relative_to(args.output).as_posix(),
            }
            invocations.append(invocation)
            if completed.returncode != 0:
                process_failures.append(invocation)

    assets = []
    missing = []
    for package, rows in requests.items():
        for row in rows:
            outputs = matching_outputs(
                args.output, row["objectName"], package
            )
            serialized_outputs = [
                {
                    "relativePath": path.relative_to(args.output).as_posix(),
                    "byteSize": path.stat().st_size,
                    "sha256": sha256_file(path),
                }
                for path in outputs
                if "_logs" not in path.parts
            ]
            result = {
                **row,
                "logicalPackage": package,
                "outputs": serialized_outputs,
                "resolutionStatus": (
                    "EXPORTED" if serialized_outputs else "MISSING_EXPORTED_OBJECT"
                ),
            }
            assets.append(result)
            if not serialized_outputs:
                missing.append(result)

    receipt = {
        "schema": "lostark.effect-resource-export-receipt",
        "formatVersion": 1,
        "characterClass": document.get("characterClass"),
        "sourceManifest": args.manifest.as_posix(),
        "sourceManifestSha256": sha256_file(args.manifest),
        "assets": assets,
        "invocations": invocations,
        "processFailures": process_failures,
        "missingAssets": missing,
        "summary": {
            "requestedAssetCount": len(assets),
            "exportedAssetCount": len(assets) - len(missing),
            "missingAssetCount": len(missing),
            "packageCount": len(requests),
            "processFailureCount": len(process_failures),
            "outputFileCount": sum(len(row["outputs"]) for row in assets),
        },
    }
    args.receipt.parent.mkdir(parents=True, exist_ok=True)
    args.receipt.write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(receipt["summary"], ensure_ascii=False, sort_keys=True))
    return 0 if not missing and not process_failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
