#!/usr/bin/env python3
"""Cook exact UModel effect exports into LostArk runtime Meshes/Textures."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def select_export(
    row: dict[str, Any], source_root: Path, suffixes: tuple[str, ...]
) -> Path | None:
    candidates = []
    for output in row.get("outputs", []):
        path = source_root / output["relativePath"]
        if path.suffix.casefold() in suffixes and path.is_file():
            candidates.append(path)
    return sorted(candidates, key=lambda path: path.as_posix().casefold())[0] \
        if candidates else None


def validate_unique_names(
    assets: list[dict[str, Any]], package_scoped_roles: set[str] | None = None
) -> None:
    package_scoped_roles = package_scoped_roles or set()
    owners: dict[tuple[str, ...], str] = {}
    for row in assets:
        source = str(row.get("sourceAssetPath", ""))
        name = str(row.get("objectName", "")).casefold()
        for role in sorted(set(row.get("roles", [])) & {"mesh", "texture"}):
            key = (
                (role, str(row.get("logicalPackage", "")).casefold(), name)
                if role in package_scoped_roles else (role, name)
            )
            previous = owners.get(key)
            if previous is not None and role in package_scoped_roles:
                continue
            if previous is not None and previous.casefold() != source.casefold():
                raise ValueError(
                    f"runtime name collision for {role}/{name}: "
                    f"{previous} and {source}"
                )
            owners[key] = source


def convert_tga_to_dds(source: Path, destination: Path) -> None:
    try:
        from PIL import Image
    except ImportError as error:
        raise RuntimeError("Pillow is required to convert TGA exports to DDS") \
            from error
    with Image.open(source) as image:
        image.convert("RGBA").save(destination, format="DDS")


def run_converter(
    converter: Path, source: Path, destination: Path, scale: float
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [
            str(converter),
            str(source),
            "-o",
            str(destination),
            "--pretransform",
            "--scale",
            str(scale),
            "--no-auto-textures",
        ],
        cwd=converter.parent,
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
        check=False,
        creationflags=subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0,
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--export-receipt", required=True, type=Path)
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--converter", required=True, type=Path)
    parser.add_argument("--runtime-root", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    parser.add_argument("--scale", type=float, default=100.0)
    parser.add_argument("--mesh-package-directories", action="store_true")
    parser.add_argument("--texture-package-directories", action="store_true")
    args = parser.parse_args()

    export_receipt = json.loads(
        args.export_receipt.read_text(encoding="utf-8-sig")
    )
    assets = export_receipt.get("assets", [])
    package_scoped_roles = set()
    if args.mesh_package_directories:
        package_scoped_roles.add("mesh")
    if args.texture_package_directories:
        package_scoped_roles.add("texture")
    validate_unique_names(assets, package_scoped_roles)

    mesh_root = args.runtime_root / "Meshes"
    texture_root = args.runtime_root / "Textures"
    logs_root = args.receipt.parent / (args.receipt.stem + "-logs")
    mesh_root.mkdir(parents=True, exist_ok=True)
    texture_root.mkdir(parents=True, exist_ok=True)
    logs_root.mkdir(parents=True, exist_ok=True)

    results: list[dict[str, Any]] = []
    failures: list[dict[str, Any]] = []
    for row in assets:
        roles = set(row.get("roles", []))
        name = str(row.get("objectName", ""))
        source_asset = str(row.get("sourceAssetPath", ""))
        logical_package = str(row.get("logicalPackage", ""))

        if "mesh" in roles:
            source = select_export(row, args.source_root, (".gltf", ".fbx"))
            package_directory = (
                logical_package.upper() if args.mesh_package_directories else ""
            )
            destination = mesh_root / package_directory / f"{name}.wmodel"
            destination.parent.mkdir(parents=True, exist_ok=True)
            result: dict[str, Any] = {
                "sourceAssetPath": source_asset,
                "role": "mesh",
                "sourceFile": (
                    source.relative_to(args.source_root).as_posix() if source else None
                ),
                "runtimeAssetId": "/".join(
                    part for part in (
                        "Effect",
                        args.runtime_root.name,
                        "Meshes",
                        package_directory,
                        f"{name}.wmodel",
                    ) if part
                ),
            }
            if source is None:
                result["status"] = "MISSING_SOURCE_EXPORT"
            else:
                completed = run_converter(
                    args.converter, source, destination, args.scale
                )
                log = logs_root / f"mesh.{logical_package}.{name}.log"
                log.write_text(
                    completed.stdout + "\n" + completed.stderr, encoding="utf-8"
                )
                result["converterExitCode"] = completed.returncode
                result["logFile"] = log.relative_to(args.receipt.parent).as_posix()
                result["status"] = (
                    "COOKED"
                    if completed.returncode == 0 and destination.is_file()
                    else "CONVERTER_FAILED"
                )
            if destination.is_file():
                result["runtimeFile"] = destination.as_posix()
                result["byteSize"] = destination.stat().st_size
                result["sha256"] = sha256_file(destination)
            results.append(result)
            if result["status"] != "COOKED":
                failures.append(result)

        if "texture" in roles:
            source = select_export(row, args.source_root, (".dds", ".tga"))
            package_directory = (
                str(row.get("logicalPackage", "")).upper()
                if args.texture_package_directories else ""
            )
            destination = texture_root / package_directory / f"{name}.dds"
            destination.parent.mkdir(parents=True, exist_ok=True)
            result = {
                "sourceAssetPath": source_asset,
                "role": "texture",
                "sourceFile": (
                    source.relative_to(args.source_root).as_posix() if source else None
                ),
                "runtimeAssetId": "/".join(
                    part for part in (
                        "Effect",
                        args.runtime_root.name,
                        "Textures",
                        package_directory,
                        f"{name}.dds",
                    ) if part
                ),
            }
            try:
                if source is None:
                    result["status"] = "MISSING_SOURCE_EXPORT"
                elif source.suffix.casefold() == ".dds":
                    shutil.copyfile(source, destination)
                    result["status"] = "COPIED"
                else:
                    convert_tga_to_dds(source, destination)
                    result["status"] = "CONVERTED_TGA_TO_DDS"
            except Exception as error:  # Preserve the concrete per-asset failure.
                result["status"] = "TEXTURE_COOK_FAILED"
                result["error"] = str(error)
            if destination.is_file():
                result["runtimeFile"] = destination.as_posix()
                result["byteSize"] = destination.stat().st_size
                result["sha256"] = sha256_file(destination)
            results.append(result)
            if result["status"] not in {"COPIED", "CONVERTED_TGA_TO_DDS"}:
                failures.append(result)

    receipt = {
        "schema": "lostark.effect-runtime-resource-cook-receipt",
        "formatVersion": 1,
        "characterClass": export_receipt.get("characterClass"),
        "sourceExportReceipt": args.export_receipt.as_posix(),
        "sourceExportReceiptSha256": sha256_file(args.export_receipt),
        "runtimeRoot": args.runtime_root.as_posix(),
        "scale": args.scale,
        "assets": results,
        "failures": failures,
        "summary": {
            "meshCount": sum(row["role"] == "mesh" for row in results),
            "textureCount": sum(row["role"] == "texture" for row in results),
            "cookedAssetCount": len(results) - len(failures),
            "failureCount": len(failures),
        },
    }
    args.receipt.parent.mkdir(parents=True, exist_ok=True)
    args.receipt.write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(receipt["summary"], ensure_ascii=False, sort_keys=True))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
