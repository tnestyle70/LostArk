#!/usr/bin/env python3
"""Cook exact Valtan DeployData visuals and build runtime catalog/placements."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Any


AREA_ID = "LV_LUT_HEARTRB_ED"
MODEL_PATTERN = re.compile(r"EFDLProp_(ITR_[0-9]+)\.", re.IGNORECASE)


def quoted(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, name = tempfile.mkstemp(prefix=path.name + ".", suffix=".tmp", dir=path.parent)
    temporary = Path(name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as output:
            output.write(text)
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def atomic_copy(source: Path, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary = destination.with_name(destination.name + ".installing")
    shutil.copy2(source, temporary)
    os.replace(temporary, destination)


def install_tree(source: Path, destination: Path) -> list[dict[str, Any]]:
    receipts: list[dict[str, Any]] = []
    for path in sorted(source.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(source)
        target = destination / relative
        atomic_copy(path, target)
        receipts.append(
            {
                "path": relative.as_posix(),
                "bytes": target.stat().st_size,
                "sha256": sha256(target),
            }
        )
    return receipts


def finite_vector(value: list[Any], length: int, name: str) -> tuple[float, ...]:
    if len(value) != length:
        raise ValueError(f"{name} has invalid length")
    result = tuple(float(component) for component in value)
    if not all(math.isfinite(component) for component in result):
        raise ValueError(f"{name} contains non-finite values")
    return result


def cook(
    converter: Path,
    source: Path,
    output: Path,
    texture_root: Path,
    pretransform: bool,
    expect_skeleton: bool,
) -> dict[str, Any]:
    output.parent.mkdir(parents=True, exist_ok=True)
    command = [
        str(converter),
        str(source),
        "-o",
        str(output),
        "--scale",
        "100",
        "--texture-root",
        str(texture_root),
    ]
    if pretransform:
        command.append("--pretransform")
    result = subprocess.run(command, check=True, capture_output=True, text=True)
    info = subprocess.run(
        [str(converter), "info", str(output)],
        check=True,
        capture_output=True,
        text=True,
    ).stdout
    if "material-version=2" not in info or "[Model]" not in info:
        raise RuntimeError(f"invalid cooked model {output}:\n{info}")
    skeleton_marker = "skeleton=yes" if expect_skeleton else "skeleton=no"
    if skeleton_marker not in info:
        raise RuntimeError(
            f"unexpected skeleton contract for {output}; expected {skeleton_marker}:\n{info}"
        )
    return {
        "source": str(source),
        "sourceSha256": sha256(source),
        "converterOutput": result.stdout.strip(),
        "info": info.strip().splitlines(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--deploy-json", required=True, type=Path)
    parser.add_argument("--converter", required=True, type=Path)
    parser.add_argument("--texture-root", required=True, type=Path)
    parser.add_argument("--runtime-root", required=True, type=Path)
    parser.add_argument("--catalog-output", required=True, type=Path)
    parser.add_argument("--placement-output", required=True, type=Path)
    parser.add_argument("--receipt-output", required=True, type=Path)
    args = parser.parse_args()

    for path in (args.deploy_json, args.converter, args.texture_root, args.runtime_root):
        if not path.exists():
            parser.error(f"required path is missing: {path}")

    document = json.loads(args.deploy_json.read_text(encoding="utf-8"))
    if document.get("zoneId") != 37051:
        raise ValueError("unexpected DeployData zone")
    records = document.get("records", [])
    visual_records = [
        row for row in records
        if row.get("lookInfo") and row["lookInfo"].get("meshReferences")
    ]
    if len(visual_records) != 85:
        raise ValueError(f"expected 85 visual arena records, got {len(visual_records)}")

    definitions: dict[str, dict[str, Any]] = {}
    for row in visual_records:
        model_name = str(row["prop"]["Model"])
        match = MODEL_PATTERN.match(model_name)
        if match is None:
            raise ValueError(f"unsupported prop model: {model_name}")
        package = match.group(1).upper()
        references = row["lookInfo"]["meshReferences"]
        if len(references) != 1:
            raise ValueError(f"expected one mesh reference for {package}")
        reference = references[0]
        kind = str(reference["kind"])
        asset_id = f"DEPLOY_{package}"
        definition = {
            "assetId": asset_id,
            "package": package,
            "label": model_name,
            "kind": "ANIM" if kind == "SkeletalMesh" else "STATIC",
            "referenceKind": kind,
            "referenceObjectPath": str(reference["objectPath"]),
            "intactSource": reference.get("extractedIntactSiblingGltf"),
            "fracturedSource": reference.get("extractedStaticMeshGltf"),
            "evidence": (
                "LookInfo SkeletalMesh direct reference"
                if kind == "SkeletalMesh"
                else "LookInfo FracturedStaticMesh direct reference; intact same-package sibling"
            ),
        }
        if kind == "SkeletalMesh":
            candidates = list((args.texture_root / package / "SkeletalMesh3").glob("*.gltf"))
            if len(candidates) != 1:
                raise ValueError(f"cannot resolve skeletal source for {package}: {candidates}")
            definition["intactSource"] = str(candidates[0].resolve())
            definition["fracturedSource"] = None
        existing = definitions.get(asset_id)
        if existing is not None and existing != definition:
            raise ValueError(f"conflicting definition for {asset_id}")
        definitions[asset_id] = definition

    if len(definitions) != 9:
        raise ValueError(f"expected 9 visual definitions, got {len(definitions)}")

    catalog_rows: list[str] = []
    cook_receipts: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="valtan-deploy-runtime-") as temporary_name:
        stage = Path(temporary_name)
        for asset_id, definition in sorted(definitions.items()):
            intact_source = Path(str(definition["intactSource"]))
            if not intact_source.is_file():
                raise FileNotFoundError(intact_source)
            asset_stage = stage / asset_id
            intact_name = f"{asset_id}_INTACT.wmodel"
            intact_model = asset_stage / "intact" / intact_name
            cooked = {
                "assetId": asset_id,
                "intact": cook(
                    args.converter,
                    intact_source,
                    intact_model,
                    args.texture_root,
                    definition["kind"] == "STATIC",
                    definition["kind"] == "ANIM",
                ),
            }
            intact_relative = (
                Path("Deploy") / AREA_ID / asset_id / "intact" / intact_name
            ).as_posix()
            intact_prototype = f"Prototype_Component_Model_{asset_id}_INTACT"

            fractured_relative = ""
            fractured_prototype = ""
            fractured_source_value = definition["fracturedSource"]
            if fractured_source_value:
                fractured_source = Path(str(fractured_source_value))
                if not fractured_source.is_file():
                    raise FileNotFoundError(fractured_source)
                fractured_name = f"{asset_id}_FRACTURED.wmodel"
                fractured_model = asset_stage / "fractured" / fractured_name
                cooked["fractured"] = cook(
                    args.converter,
                    fractured_source,
                    fractured_model,
                    args.texture_root,
                    True,
                    False,
                )
                fractured_relative = (
                    Path("Deploy") / AREA_ID / asset_id / "fractured" / fractured_name
                ).as_posix()
                fractured_prototype = (
                    f"Prototype_Component_Model_{asset_id}_FRACTURED"
                )

            installed = install_tree(
                asset_stage,
                args.runtime_root / "Deploy" / AREA_ID / asset_id,
            )
            cooked["files"] = installed
            cook_receipts.append(cooked)
            catalog_rows.append(
                " ".join(
                    (
                        quoted(asset_id),
                        definition["kind"],
                        quoted(str(definition["label"])),
                        quoted(intact_relative),
                        quoted(intact_prototype),
                        quoted(fractured_relative),
                        quoted(fractured_prototype),
                        "1",
                        "0",
                        quoted(str(definition["evidence"])),
                    )
                )
            )

    placement_rows: list[str] = []
    runtime_ids: set[int] = set()
    for row in sorted(visual_records, key=lambda value: int(value["runtimePlacementId"])):
        runtime_id = int(row["runtimePlacementId"])
        if runtime_id == 0 or runtime_id in runtime_ids:
            raise ValueError(f"invalid/duplicate runtime placement id: {runtime_id}")
        runtime_ids.add(runtime_id)
        match = MODEL_PATTERN.match(str(row["prop"]["Model"]))
        assert match is not None
        asset_id = f"DEPLOY_{match.group(1).upper()}"
        position = finite_vector(row["clientTransform"]["positionMeters"], 3, "position")
        quaternion = finite_vector(row["clientTransform"]["quaternion"], 4, "quaternion")
        scale = float(row["clientTransform"]["uniformScale"])
        if not math.isfinite(scale) or scale <= 0.0:
            raise ValueError("invalid uniform scale")
        destructible = row["classification"] == "destructible-or-stateful"
        state_off_action = int(row["prop"].get("StateOffActionId", 0))
        occurrence_count = int(row.get("triggerBinaryOccurrenceCount", 0))
        placement_rows.append(
            " ".join(
                (
                    str(runtime_id),
                    str(int(row["deployActorId"])),
                    str(int(row["propDefinitionId"])),
                    quoted(str(row["sourcePlacementId"])),
                    quoted(asset_id),
                    *(format(value, ".9g") for value in position),
                    *(format(value, ".9g") for value in quaternion),
                    format(scale, ".9g"),
                    "1" if destructible else "0",
                    str(state_off_action),
                    str(occurrence_count),
                )
            )
        )

    catalog_text = (
        f"LOSTARK_DEPLOY_PROP_CATALOG 2 {quoted(AREA_ID)} {len(catalog_rows)}\n"
        + "\n".join(catalog_rows)
        + "\n"
    )
    placement_text = (
        f"LOSTARK_DEPLOY_PROP_PLACEMENTS 1 {quoted(AREA_ID)} {len(placement_rows)}\n"
        + "\n".join(placement_rows)
        + "\n"
    )
    atomic_write(args.catalog_output, catalog_text)
    atomic_write(args.placement_output, placement_text)
    receipt = {
        "schemaVersion": 1,
        "areaId": AREA_ID,
        "catalogAssetCount": len(catalog_rows),
        "visualPlacementCount": len(placement_rows),
        "staticPlacementCount": sum(
            1 for row in visual_records
            if row["lookInfo"]["meshReferences"][0]["kind"] == "FracturedStaticMesh"
        ),
        "skeletalPlacementCount": sum(
            1 for row in visual_records
            if row["lookInfo"]["meshReferences"][0]["kind"] == "SkeletalMesh"
        ),
        "triggerEvidence": "raw little-endian deploy actor ID binary occurrence; node/field not parsed",
        "assets": cook_receipts,
        "outputs": {
            "catalog": sha256(args.catalog_output),
            "placements": sha256(args.placement_output),
        },
    }
    atomic_write(
        args.receipt_output,
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n",
    )
    print(
        json.dumps(
            {
                "assets": len(catalog_rows),
                "placements": len(placement_rows),
                "static": receipt["staticPlacementCount"],
                "skeletal": receipt["skeletalPlacementCount"],
            }
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
