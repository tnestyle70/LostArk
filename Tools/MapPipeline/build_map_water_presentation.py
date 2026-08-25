"""Project the source water material contract into a map water presentation document.

The cooked .wmodel carries five material slots and none of them means
"reflection" or "second scrolling normal", so a water surface loses most of its
contract at cook time. This tool reads the source receipts that
build_bern_castle_assets.py writes, keeps only what the original materials
actually declare, and emits the authoring document the runtime binds when it
draws a water asset.

Nothing here is inferred from a file name. An asset becomes water because the
master Material3 at the root of its instance chain declares
BlendMode = BLEND_Translucent, and an asset whose materials disagree is
recorded with its reason instead of being guessed either way.
"""

from __future__ import annotations

import argparse
import json
import os
import shlex
import sys
import tempfile
from pathlib import Path
from typing import Any

CATALOG_MAGIC = "LOSTARK_MAP_ASSET_CATALOG"
CATALOG_VERSION = 4
CATALOG_FIELD_COUNT = 26
RENDER_MODE_INDEX = 11
CULL_MODE_INDEX = 12
SCHEMA = "lostark.map-water-presentation"
FORMAT_VERSION = 1
TRANSLUCENT_BLEND_MODE = "BLEND_Translucent"
SOURCE_RECEIPT_SCHEMA_VERSION = 2

# Auxiliary role in the source receipt -> field name in the water document.
AUXILIARY_ROLE_TO_FIELD = {
    "detailNormal": "detailNormalTexture",
    "reflection": "reflectionTexture",
    "foam": "foamTexture",
}
# Source scalar -> field name. Only scalars the water pass actually consumes are
# projected; the rest stay in the receipt as evidence.
SCALAR_TO_FIELD = {
    "opacity": "opacity",
    "opacity_power": "opacityPower",
    "fresnel_intensity": "fresnelIntensity",
    "fresnel_power": "fresnelPower",
    "screen_distortion_intensity": "screenDistortionIntensity",
    "normal_intensity": "normalIntensity",
    "detail_normal_intensity": "detailNormalIntensity",
    "normal_distortion_intensity": "normalDistortionIntensity",
    "reflection_intensity": "reflectionIntensity",
    "reflection_uv": "reflectionUv",
    "depth_bias": "depthBias",
    "diffuse_tiling": "diffuseTiling",
}
VECTOR_TO_FIELD = {
    "diffuse_color": "diffuseColor",
    "reflection_color": "reflectionColor",
    "normal_tiling_panning": "normalTilingPanning",
    "detail_normal_tiling_panning": "detailNormalTilingPanning",
    "reflection_tiling_panning": "reflectionTilingPanning",
}
# Applied only when the source chain declares nothing. Identity values that keep
# a missing term from changing the pixel, never a look someone liked.
SCALAR_DEFAULTS = {
    "opacity": 1.0,
    "opacityPower": 1.0,
    "fresnelIntensity": 0.0,
    "fresnelPower": 1.0,
    "screenDistortionIntensity": 0.0,
    "normalIntensity": 0.0,
    "detailNormalIntensity": 0.0,
    "normalDistortionIntensity": 0.0,
    "reflectionIntensity": 0.0,
    "reflectionUv": 1.0,
    "depthBias": 0.0,
    "diffuseTiling": 1.0,
}
VECTOR_DEFAULTS = {
    "diffuseColor": [1.0, 1.0, 1.0, 1.0],
    "reflectionColor": [1.0, 1.0, 1.0, 1.0],
    "normalTilingPanning": [1.0, 1.0, 0.0, 0.0],
    "detailNormalTilingPanning": [1.0, 1.0, 0.0, 0.0],
    "reflectionTilingPanning": [1.0, 1.0, 0.0, 0.0],
}


class WaterPresentationError(RuntimeError):
    pass


def atomic_write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        dir=str(path.parent), prefix=path.name + ".", suffix=".tmp")
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as stream:
            json.dump(value, stream, ensure_ascii=False, indent=2)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        temporary.replace(path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def atomic_write_text(path: Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        dir=str(path.parent), prefix=path.name + ".", suffix=".tmp")
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as stream:
            stream.write(value)
            stream.flush()
            os.fsync(stream.fileno())
        temporary.replace(path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def catalog_files(catalog_dir: Path) -> list[Path]:
    rows = sorted(catalog_dir.glob("*.mapassets"))
    if not rows:
        raise WaterPresentationError(f"no .mapassets under {catalog_dir}")
    return rows


def parse_catalog(path: Path) -> tuple[str, list[list[str]]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines:
        raise WaterPresentationError(f"empty catalog: {path}")
    header = shlex.split(lines[0], posix=True)
    if len(header) != 4 or header[0] != CATALOG_MAGIC:
        raise WaterPresentationError(f"unsupported catalog header: {path}")
    if int(header[1]) != CATALOG_VERSION:
        raise WaterPresentationError(
            f"catalog version {header[1]} is not {CATALOG_VERSION}: {path}")
    declared = int(header[3])
    rows = [shlex.split(line, posix=True) for line in lines[1:] if line.strip()]
    if len(rows) != declared:
        raise WaterPresentationError(
            f"catalog declares {declared} rows but carries {len(rows)}: {path}")
    for index, fields in enumerate(rows, 2):
        if len(fields) != CATALOG_FIELD_COUNT:
            raise WaterPresentationError(
                f"catalog row {index} must contain {CATALOG_FIELD_COUNT} fields: {path}")
    return header[2], rows


def load_receipt(source_root: Path, asset_id: str) -> tuple[dict[str, Any] | None, str]:
    """Returns the receipt and why it is or is not usable.

    A receipt written before the material contract was preserved carries no
    BlendMode at all, so it cannot say whether its asset is water. That is
    reported as unclassified rather than silently read as "not water", because
    the two look identical in the output and only one of them is true.
    """
    path = source_root / asset_id / "source.receipt.json"
    if not path.is_file():
        return None, "no-receipt"
    receipt = json.loads(path.read_text(encoding="utf-8"))
    if receipt.get("schemaVersion") != SOURCE_RECEIPT_SCHEMA_VERSION:
        return None, "stale-receipt-schema"
    return receipt, "ok"


def blend_modes(receipt: dict[str, Any]) -> list[tuple[str, str | None]]:
    return [
        (str(material["name"]), (material.get("renderFlags") or {}).get("blendMode"))
        for material in receipt.get("materials", [])
    ]


def texture_asset_id(area_id: str, asset_id: str, receipt_texture: str) -> str:
    name = Path(receipt_texture).name
    return f"Map/{area_id}/{asset_id}/textures/{name}"


def build_row(
    area_id: str, asset_id: str, material: dict[str, Any], resource_root: Path | None,
) -> dict[str, Any]:
    row: dict[str, Any] = {
        "assetId": asset_id,
        "materialName": str(material["name"]),
        "provenance": "SOURCE_MATERIAL_EXACT",
        "sourceParentChain": list(material.get("parentChain", [])),
    }
    flags = material.get("renderFlags") or {}
    row["sourceBlendMode"] = flags.get("blendMode")
    row["twoSided"] = bool(flags.get("twoSided", False))

    textures: dict[str, str] = {}
    for role, entry in (material.get("auxiliaryTextures") or {}).items():
        field = AUXILIARY_ROLE_TO_FIELD.get(role)
        if field is None:
            continue
        textures[field] = texture_asset_id(area_id, asset_id, str(entry["texture"]))
    for field in AUXILIARY_ROLE_TO_FIELD.values():
        row[field] = textures.get(field, "")

    if resource_root is not None:
        for field in AUXILIARY_ROLE_TO_FIELD.values():
            value = row[field]
            if value and not (resource_root / value).is_file():
                raise WaterPresentationError(
                    f"{asset_id}: {field} is not deployed: {value}")

    scalars = material.get("scalars") or {}
    for source_name, field in SCALAR_TO_FIELD.items():
        row[field] = float(scalars.get(source_name, SCALAR_DEFAULTS[field]))
    vectors = material.get("vectors") or {}
    for source_name, field in VECTOR_TO_FIELD.items():
        value = vectors.get(source_name)
        row[field] = [float(component) for component in value] if value else list(
            VECTOR_DEFAULTS[field])

    missing = [entry["parameter"] for entry in material.get("missingTextures", [])]
    if missing:
        row["missingSourceTextures"] = sorted(missing)
    return row


def build_document(
    area_id: str, catalog_dir: Path, source_root: Path, resource_root: Path | None,
) -> tuple[dict[str, Any], dict[str, str]]:
    seen: dict[str, list[str]] = {}
    for path in catalog_files(catalog_dir):
        catalog_area, rows = parse_catalog(path)
        if catalog_area != area_id:
            raise WaterPresentationError(
                f"catalog area {catalog_area} is not {area_id}: {path}")
        for fields in rows:
            seen.setdefault(fields[0], []).append(fields[RENDER_MODE_INDEX])

    waters: list[dict[str, Any]] = []
    deferred: list[dict[str, Any]] = []
    unclassified: list[str] = []
    admitted_render_mode: dict[str, str] = {}
    for asset_id in sorted(seen):
        receipt, status = load_receipt(source_root, asset_id)
        if receipt is None:
            if status == "stale-receipt-schema":
                unclassified.append(asset_id)
            continue
        modes = blend_modes(receipt)
        translucent = [name for name, mode in modes if mode == TRANSLUCENT_BLEND_MODE]
        if not translucent:
            continue
        if len(translucent) != len(modes):
            deferred.append({
                "assetId": asset_id,
                "reason": "MIXED_MATERIAL_BLEND_MODES",
                "detail": (
                    "The render group and the shader pass are chosen once per "
                    "object, so an asset whose materials disagree cannot be "
                    "drawn correctly without splitting its meshes into two "
                    "submissions. Left at its authored render mode."
                ),
                "materials": [
                    {"name": name, "blendMode": mode} for name, mode in modes
                ],
            })
            continue
        for material in receipt["materials"]:
            waters.append(build_row(area_id, asset_id, material, resource_root))
        admitted_render_mode[asset_id] = "Water"

    document = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "areaId": area_id,
        "revision": 1,
        "unclassifiedAssetCount": len(unclassified),
        "waters": waters,
        "deferred": deferred,
    }
    return document, admitted_render_mode


def apply_render_mode(
    catalog_dir: Path, admitted: dict[str, str],
) -> list[dict[str, str]]:
    changes: list[dict[str, str]] = []
    for path in catalog_files(catalog_dir):
        lines = path.read_text(encoding="utf-8").splitlines()
        rewritten = list(lines)
        touched = False
        for index in range(1, len(lines)):
            line = lines[index]
            if not line.strip():
                continue
            fields = shlex.split(line, posix=True)
            wanted = admitted.get(fields[0])
            if wanted is None or fields[RENDER_MODE_INDEX] == wanted:
                continue
            changes.append({
                "file": path.name,
                "assetId": fields[0],
                "from": fields[RENDER_MODE_INDEX],
                "to": wanted,
            })
            # Only the trailing render profile is rebuilt. Re-quoting the whole
            # row would rewrite every string field's quoting style and produce
            # a diff far larger than the one token that actually changed.
            trailing = CATALOG_FIELD_COUNT - RENDER_MODE_INDEX
            parts = line.rsplit(None, trailing)
            if len(parts) != trailing + 1 or parts[1] != fields[RENDER_MODE_INDEX]:
                raise WaterPresentationError(
                    f"unexpected render profile layout in {path.name}: {fields[0]}")
            parts[1] = wanted
            rewritten[index] = parts[0] + " " + " ".join(parts[1:])
            touched = True
        if touched:
            atomic_write_text(path, "\n".join(rewritten) + "\n")
    return changes


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--area-id", required=True)
    parser.add_argument("--catalog-dir", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--resource-root", type=Path)
    parser.add_argument("--apply-render-mode", action="store_true")
    parser.add_argument("--check", action="store_true")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    document, admitted = build_document(
        args.area_id, args.catalog_dir, args.source_root, args.resource_root)
    if args.check:
        if not args.output.is_file():
            raise WaterPresentationError(f"missing water document: {args.output}")
        current = json.loads(args.output.read_text(encoding="utf-8"))
        if current != document:
            raise WaterPresentationError(
                f"water document is stale: {args.output}")
    else:
        atomic_write_json(args.output, document)
    changes = apply_render_mode(args.catalog_dir, admitted) if (
        args.apply_render_mode and not args.check) else []
    print(json.dumps({
        "areaId": args.area_id,
        "waterMaterialCount": len(document["waters"]),
        "admittedAssetCount": len(admitted),
        "deferredAssetCount": len(document["deferred"]),
        "renderModeChanges": changes,
        "mode": "check" if args.check else "write",
    }, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except WaterPresentationError as error:
        print(json.dumps({"status": "failed", "error": str(error)},
                         ensure_ascii=False), file=sys.stderr)
        raise SystemExit(1)
