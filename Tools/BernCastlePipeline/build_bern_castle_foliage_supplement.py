#!/usr/bin/env python3
"""Extract and cook the exact Bern Castle foliage meshes absent from the 950 set.

The authoritative input is ``bern_castle_nonstatic.json``.  Only rows carrying
``missingRuntimeStaticMesh`` are accepted; package/object names are never
inferred from filenames.  Source and runtime packs are kept in a separate root,
so the 950-asset manifests and the Bern mapset are not modified.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
import time
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Iterable, Sequence

import build_bern_castle_assets as asset_pipeline


DEFAULT_BERN_ROOT = Path(r"C:\LostArkExtract\bern_full")
DEFAULT_OUTPUT_ROOT = DEFAULT_BERN_ROOT / "foliage"
DEFAULT_NONSTATIC = DEFAULT_BERN_ROOT / "manifests" / "bern_castle_nonstatic.json"
DEFAULT_BASE_STATIC = DEFAULT_BERN_ROOT / "manifests" / "bern_castle_assets.json"
ASSET_MANIFEST_NAME = "bern_castle_foliage_supplement_assets.json"
RUNTIME_MANIFEST_NAME = "bern_castle_foliage_supplement_runtime_assets.json"
EXPECTED_ASSETS = 11
EXPECTED_USAGES = 1407


class SupplementError(RuntimeError):
    pass


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise SupplementError(f"could not read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise SupplementError(f"JSON root must be an object: {path}")
    return value


def _exact_static_reference(item: dict[str, Any], full_path: str) -> dict[str, Any]:
    references = item.get("references")
    if not isinstance(references, list):
        raise SupplementError(f"foliage item has no reference array: {item.get('id')}")
    matches = [
        row
        for row in references
        if isinstance(row, dict)
        and row.get("role") == "staticMesh"
        and str(row.get("objectPath", "")).casefold() == full_path.casefold()
    ]
    if len(matches) != 1:
        raise SupplementError(
            f"expected one exact StaticMesh reference for {item.get('id')}: {full_path}"
        )
    reference = matches[0]
    if str(reference.get("runtimeAvailability")) != "missing":
        raise SupplementError(
            f"supplemental reference is not marked missing: {item.get('id')}"
        )
    if str(reference.get("class", "")).casefold() != "staticmesh":
        raise SupplementError(
            f"reference class is not StaticMesh: {item.get('id')} {reference.get('class')}"
        )
    return reference


def build_inventory(
    nonstatic_path: Path,
    base_static_path: Path,
    expect_assets: int = EXPECTED_ASSETS,
    expect_usages: int = EXPECTED_USAGES,
) -> dict[str, Any]:
    nonstatic = load_json(nonstatic_path)
    if nonstatic.get("schemaVersion") != 1 or nonstatic.get("areaId") != "LV_BER_BERNCASTLE":
        raise SupplementError("non-static manifest identity mismatch")
    base_static = load_json(base_static_path)
    base_assets = base_static.get("assets")
    if not isinstance(base_assets, list) or base_static.get("assetCount") != len(base_assets):
        raise SupplementError("base StaticMesh manifest is invalid")
    base_paths = {str(row.get("fullPath", "")).casefold() for row in base_assets}
    base_ids = {str(row.get("assetId", "")) for row in base_assets}

    uses: defaultdict[str, list[dict[str, Any]]] = defaultdict(list)
    items = nonstatic.get("items")
    if not isinstance(items, list):
        raise SupplementError("non-static manifest has no items array")
    for item in items:
        if not isinstance(item, dict) or item.get("type") != "foliage":
            continue
        missing = item.get("missingReferences")
        if not isinstance(missing, list):
            raise SupplementError(f"foliage item has invalid missingReferences: {item.get('id')}")
        rows = [
            row
            for row in missing
            if isinstance(row, dict) and row.get("kind") == "missingRuntimeStaticMesh"
        ]
        if not rows:
            continue
        if len(rows) != 1:
            raise SupplementError(f"foliage item has multiple missing meshes: {item.get('id')}")
        full_path = str(rows[0].get("objectPath", "")).strip().casefold()
        if not full_path:
            raise SupplementError(f"foliage missing record has no objectPath: {item.get('id')}")
        reference = _exact_static_reference(item, full_path)
        source = item.get("source")
        if not isinstance(source, dict):
            raise SupplementError(f"foliage item has no source: {item.get('id')}")
        component = source.get("component")
        if not isinstance(component, dict) or component.get("exportIndex") is None:
            raise SupplementError(f"foliage item has no source component: {item.get('id')}")
        uses[full_path].append(
            {
                "itemId": str(item.get("id", "")),
                "level": str(source.get("level", "")),
                "componentExportIndex": int(component["exportIndex"]),
                "componentPath": str(component.get("objectPath", "")),
                "packageReferenceIndex": int(reference["index"]),
            }
        )

    assets: list[dict[str, Any]] = []
    for full_path, evidence in sorted(uses.items()):
        parts = full_path.split(".")
        if len(parts) < 2 or not all(parts):
            raise SupplementError(f"invalid exact object path: {full_path!r}")
        object_name = parts[-1]
        asset_id = asset_pipeline.stable_asset_id(full_path, object_name)
        if full_path in base_paths:
            raise SupplementError(f"supplemental path already exists in base 950: {full_path}")
        if asset_id in base_ids:
            raise SupplementError(f"supplemental asset ID collides with base 950: {asset_id}")
        level_counts = Counter(row["level"] for row in evidence)
        assets.append(
            {
                "assetId": asset_id,
                "fullPath": full_path,
                "logicalPackage": parts[0].upper(),
                "objectGroup": "/".join(parts[1:-1]),
                "objectName": object_name,
                "sourceCategory": "foliage-staticmesh-supplement",
                "usageCount": len(evidence),
                "sourceLevelCounts": dict(sorted(level_counts.items())),
                "evidence": sorted(
                    evidence,
                    key=lambda row: (
                        row["level"].casefold(),
                        row["componentExportIndex"],
                        row["itemId"],
                    ),
                ),
            }
        )

    inventory = {
        "schemaVersion": 1,
        "areaId": "LV_BER_BERNCASTLE",
        "kind": "foliage-staticmesh-supplement",
        "assetCount": len(assets),
        "usageCount": sum(int(row["usageCount"]) for row in assets),
        "sourceNonstaticManifest": {
            "path": str(nonstatic_path.resolve()),
            "sha256": asset_pipeline.sha256(nonstatic_path),
        },
        "baseStaticManifest": {
            "path": str(base_static_path.resolve()),
            "sha256": asset_pipeline.sha256(base_static_path),
            "assetCount": len(base_assets),
        },
        "assets": assets,
    }
    validate_inventory(inventory, expect_assets, expect_usages)
    return inventory


def validate_inventory(
    inventory: dict[str, Any], expect_assets: int, expect_usages: int
) -> None:
    assets = inventory.get("assets")
    if not isinstance(assets, list):
        raise SupplementError("supplement inventory assets must be an array")
    if inventory.get("assetCount") != len(assets) or len(assets) != expect_assets:
        raise SupplementError(
            f"supplement asset count mismatch: {len(assets)} != {expect_assets}"
        )
    usage_count = sum(int(row.get("usageCount", -1)) for row in assets)
    if inventory.get("usageCount") != usage_count or usage_count != expect_usages:
        raise SupplementError(
            f"supplement usage count mismatch: {usage_count} != {expect_usages}"
        )
    ids = {str(row.get("assetId", "")) for row in assets}
    paths = {str(row.get("fullPath", "")).casefold() for row in assets}
    if "" in ids or "" in paths or len(ids) != len(assets) or len(paths) != len(assets):
        raise SupplementError("empty/duplicate supplemental asset ID or path")
    evidence_ids: set[str] = set()
    for asset in assets:
        evidence = asset.get("evidence")
        if not isinstance(evidence, list) or len(evidence) != int(asset["usageCount"]):
            raise SupplementError(f"supplement evidence count mismatch: {asset['assetId']}")
        for row in evidence:
            item_id = str(row.get("itemId", ""))
            if not item_id or item_id in evidence_ids:
                raise SupplementError(f"empty/duplicate supplemental evidence ID: {item_id!r}")
            evidence_ids.add(item_id)


def write_inventory(output_root: Path, inventory: dict[str, Any]) -> Path:
    path = output_root / "manifests" / ASSET_MANIFEST_NAME
    asset_pipeline.atomic_write_json(path, inventory)
    staged = load_json(path)
    validate_inventory(staged, int(inventory["assetCount"]), int(inventory["usageCount"]))
    return path


def _validate_source_pack(
    output_root: Path, asset: dict[str, Any]
) -> tuple[dict[str, Any], int, int, int]:
    asset_id = str(asset["assetId"])
    pack = output_root / "source" / asset_id
    if not asset_pipeline.receipt_is_valid(pack, "source.receipt.json", asset):
        raise SupplementError(f"invalid source receipt/output hashes: {asset_id}")
    receipt = load_json(pack / "source.receipt.json")
    if receipt.get("assetId") != asset_id or receipt.get("logicalPackage") != asset["logicalPackage"]:
        raise SupplementError(f"source receipt identity mismatch: {asset_id}")
    physical = str(receipt.get("physicalPackage", ""))
    if not physical.casefold().endswith(".upk"):
        raise SupplementError(f"source receipt has no exact physical UPK: {asset_id}")
    materials = receipt.get("materials")
    if not isinstance(materials, list) or not materials:
        raise SupplementError(f"source pack has no material receipt: {asset_id}")
    props_count = sum(
        1 for row in receipt.get("outputs", [])
        if str(row.get("path", "")).casefold().endswith(".props.txt")
    )
    texture_outputs = [
        row for row in receipt.get("outputs", [])
        if Path(str(row.get("path", ""))).suffix.casefold() in {".dds", ".tga", ".png"}
        and str(row.get("path", "")).casefold().startswith("textures/")
    ]
    if props_count == 0 or not texture_outputs:
        raise SupplementError(
            f"source material/texture evidence is incomplete: {asset_id} "
            f"props={props_count} textures={len(texture_outputs)}"
        )
    umodel_log = pack / "umodel.log.txt"
    log_text = umodel_log.read_text(encoding="utf-8", errors="replace")
    if re.search(r"error\s+creating\s+file|failed\s+to\s+create", log_text, re.IGNORECASE):
        raise SupplementError(f"UModel reported an output creation failure: {asset_id}")
    role_count = 0
    for material in materials:
        roles = material.get("roles") if isinstance(material, dict) else None
        if not isinstance(roles, dict):
            raise SupplementError(f"invalid material role receipt: {asset_id}")
        for role in roles.values():
            if not isinstance(role, dict):
                raise SupplementError(f"invalid texture role receipt: {asset_id}")
            texture = pack / str(role.get("texture", ""))
            if not texture.is_file() or asset_pipeline.sha256(texture) != role.get("sha256"):
                raise SupplementError(f"material texture role hash mismatch: {asset_id} {texture}")
            role_count += 1
    if role_count == 0:
        raise SupplementError(f"source pack has no recognized texture role: {asset_id}")
    return receipt, len(materials), len(texture_outputs), role_count


def _validate_runtime_pack(
    output_root: Path, asset: dict[str, Any], source_receipt: dict[str, Any]
) -> dict[str, Any]:
    asset_id = str(asset["assetId"])
    pack = output_root / "runtime" / asset_id
    if not asset_pipeline.receipt_is_valid(pack, "runtime.receipt.json", asset):
        raise SupplementError(f"invalid runtime receipt/output hashes: {asset_id}")
    receipt = load_json(pack / "runtime.receipt.json")
    source_receipt_path = output_root / "source" / asset_id / "source.receipt.json"
    if receipt.get("sourceReceiptSha256") != asset_pipeline.sha256(source_receipt_path):
        raise SupplementError(f"runtime/source receipt join mismatch: {asset_id}")
    model = output_root / "runtime" / str(receipt.get("model", ""))
    if not model.is_file():
        raise SupplementError(f"supplemental WModel is missing: {asset_id}")
    magic = model.read_bytes()[:4]
    if magic not in asset_pipeline.REQUIRED_WMODEL_MAGICS:
        raise SupplementError(f"supplemental WModel magic is invalid: {asset_id} {magic!r}")
    info = pack / "converter.info.txt"
    info_text = info.read_text(encoding="utf-8", errors="replace") if info.is_file() else ""
    if not info_text.strip():
        raise SupplementError(f"converter info is missing/empty: {asset_id}")
    material_match = re.search(r"\bfirst=(\S+)\s+base=(\S+)", info_text)
    if material_match is None:
        raise SupplementError(f"converter info has no first material/base texture: {asset_id}")
    base_relative = Path(material_match.group(2))
    if base_relative.is_absolute() or ".." in base_relative.parts:
        raise SupplementError(f"converter info has an unsafe base texture path: {asset_id}")
    base_texture = pack / base_relative
    if not base_texture.is_file():
        raise SupplementError(
            f"converter info base texture does not exist in runtime pack: {asset_id} {base_relative}"
        )
    texture_count = sum(
        1
        for path in (pack / "textures").glob("*")
        if path.is_file() and path.suffix.casefold() in {".dds", ".tga", ".png"}
    )
    if texture_count == 0:
        raise SupplementError(f"runtime pack has no texture: {asset_id}")
    return {
        "assetId": asset_id,
        "fullPath": asset["fullPath"],
        "model": str(receipt["model"]).replace("\\", "/"),
        "wmodelSha256": asset_pipeline.sha256(model),
        "wmodelBytes": model.stat().st_size,
        "wmodelMagic": magic.decode("ascii"),
        "sourcePhysicalPackage": source_receipt["physicalPackage"],
        "sourceReceiptSha256": asset_pipeline.sha256(source_receipt_path),
        "runtimeReceiptSha256": asset_pipeline.sha256(pack / "runtime.receipt.json"),
        "converterInfoSha256": asset_pipeline.sha256(info),
        "converterFirstMaterial": material_match.group(1),
        "converterBaseTexture": base_relative.as_posix(),
        "runtimeTextureCount": texture_count,
    }


def build_runtime_manifest(
    output_root: Path,
    inventory: dict[str, Any],
    converter: Path,
    expect_assets: int,
    expect_usages: int,
) -> dict[str, Any]:
    validate_inventory(inventory, expect_assets, expect_usages)
    rows: list[dict[str, Any]] = []
    material_slots = 0
    source_textures = 0
    material_roles = 0
    for asset in inventory["assets"]:
        source_receipt, material_count, texture_count, role_count = _validate_source_pack(
            output_root, asset
        )
        row = _validate_runtime_pack(output_root, asset, source_receipt)
        row["usageCount"] = int(asset["usageCount"])
        row["sourceMaterialCount"] = material_count
        row["sourceTextureCount"] = texture_count
        row["sourceTextureRoleCount"] = role_count
        rows.append(row)
        material_slots += material_count
        source_textures += texture_count
        material_roles += role_count
    rows.sort(key=lambda row: row["fullPath"])
    magic_counts = Counter(row["wmodelMagic"] for row in rows)
    manifest = {
        "schemaVersion": 1,
        "areaId": "LV_BER_BERNCASTLE",
        "kind": "foliage-staticmesh-supplement-runtime",
        "assetCount": len(rows),
        "usageCount": sum(int(row["usageCount"]) for row in rows),
        "sourceMaterialCount": material_slots,
        "sourceTextureCount": source_textures,
        "sourceTextureRoleCount": material_roles,
        "wmodelMagicCounts": dict(sorted(magic_counts.items())),
        "converter": {
            "path": str(converter.resolve()),
            "sha256": asset_pipeline.sha256(converter),
            "bytes": converter.stat().st_size,
            "cookContract": ["--pretransform", "--no-auto-textures", "--scale", "100"],
        },
        "sourceInventory": {
            "path": str((output_root / "manifests" / ASSET_MANIFEST_NAME).resolve()),
            "sha256": asset_pipeline.sha256(output_root / "manifests" / ASSET_MANIFEST_NAME),
        },
        "assets": rows,
    }
    if manifest["assetCount"] != expect_assets or manifest["usageCount"] != expect_usages:
        raise SupplementError("supplement runtime manifest count gate failed")
    output = output_root / "manifests" / RUNTIME_MANIFEST_NAME
    asset_pipeline.atomic_write_json(output, manifest)
    staged = load_json(output)
    if staged.get("assetCount") != expect_assets or staged.get("usageCount") != expect_usages:
        raise SupplementError("committed supplement runtime manifest failed revalidation")
    return manifest


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("inventory", "extract", "cook", "all", "verify"))
    parser.add_argument("--nonstatic", type=Path, default=DEFAULT_NONSTATIC)
    parser.add_argument("--base-static", type=Path, default=DEFAULT_BASE_STATIC)
    parser.add_argument("--output-root", type=Path, default=DEFAULT_OUTPUT_ROOT)
    parser.add_argument("--expect-assets", type=int, default=EXPECTED_ASSETS)
    parser.add_argument("--expect-usages", type=int, default=EXPECTED_USAGES)
    parser.add_argument("--umodel", type=Path)
    parser.add_argument("--package-root", type=Path)
    parser.add_argument("--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn"))
    parser.add_argument("--converter", type=Path)
    parser.add_argument("--workers", type=int, default=2)
    parser.add_argument("--umodel-timeout", type=float, default=180.0)
    parser.add_argument("--converter-timeout", type=float, default=180.0)
    parser.add_argument("--force", action="store_true")
    return parser.parse_args(argv)


def require_path(path: Path | None, label: str) -> Path:
    if path is None:
        raise SupplementError(f"{label} is required for this command")
    if not path.exists():
        raise SupplementError(f"{label} is missing: {path}")
    return path.resolve()


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    started = time.monotonic()
    if not 1 <= args.workers <= 8:
        raise SupplementError("workers must be in [1, 8]")
    output_root = asset_pipeline.validate_output_root(args.output_root)
    inventory_path = output_root / "manifests" / ASSET_MANIFEST_NAME
    if args.command in ("inventory", "extract", "all"):
        inventory = build_inventory(
            args.nonstatic, args.base_static, args.expect_assets, args.expect_usages
        )
        write_inventory(output_root, inventory)
    else:
        inventory = load_json(inventory_path)
        validate_inventory(inventory, args.expect_assets, args.expect_usages)

    if args.command in ("extract", "all"):
        umodel = require_path(args.umodel, "UModel")
        package_root = require_path(args.package_root, "package root")
        asset_pipeline.validate_tools(umodel, package_root)
        asset_pipeline.parallel_assets(
            "foliage-extract",
            inventory["assets"],
            args.workers,
            lambda asset: asset_pipeline.export_one(
                asset,
                output_root,
                umodel,
                package_root,
                args.region,
                args.umodel_timeout,
                args.force,
            ),
        )
    if args.command in ("cook", "all"):
        converter = require_path(args.converter, "ModelAssetConverter")
        asset_pipeline.parallel_assets(
            "foliage-cook",
            inventory["assets"],
            args.workers,
            lambda asset: asset_pipeline.cook_one(
                asset,
                output_root,
                converter,
                args.converter_timeout,
                args.force,
            ),
        )
        manifest = build_runtime_manifest(
            output_root,
            inventory,
            converter,
            args.expect_assets,
            args.expect_usages,
        )
        print(json.dumps({"phase": "runtime-manifest", **manifest["wmodelMagicCounts"]}))
    if args.command == "verify":
        converter = require_path(args.converter, "ModelAssetConverter")
        manifest = build_runtime_manifest(
            output_root,
            inventory,
            converter,
            args.expect_assets,
            args.expect_usages,
        )
        print(json.dumps({"phase": "verified", "assetCount": manifest["assetCount"]}))
    print(
        json.dumps(
            {
                "phase": "complete",
                "command": args.command,
                "assetCount": inventory["assetCount"],
                "usageCount": inventory["usageCount"],
                "elapsedSeconds": round(time.monotonic() - started, 3),
            }
        ),
        flush=True,
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (SupplementError, asset_pipeline.PipelineError, OSError, ValueError) as error:
        print(json.dumps({"status": "failed", "error": str(error)}), file=sys.stderr)
        raise SystemExit(1)
