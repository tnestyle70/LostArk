#!/usr/bin/env python3
"""Build exact UE3 StaticMesh/material variants without adding a new runtime.

The source boundary is a placement schema v1/v2 file set.  A v1 placement is
the base ``[]`` material signature; v2 preserves ordered overrides and authored
null slots.  Runtime variants still cook through ModelAssetConverter and are
consumed by the existing CModel -> CMaterial path.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import sys
import tempfile
import uuid
from collections import Counter
from pathlib import Path
from typing import Any, Iterable, Sequence


HERE = Path(__file__).resolve().parent
BERN_TOOL_DIR = HERE.parent / "BernCastlePipeline"
if str(BERN_TOOL_DIR) not in sys.path:
    sys.path.insert(0, str(BERN_TOOL_DIR))

import build_bern_castle_assets as base  # noqa: E402
import build_maptool_scene as scene  # noqa: E402


DEFAULT_AREA_ID = "LV_LUT_MIDNIGHTC_ED"
DEFAULT_LEVEL_PREFIX = "LV_LUT_MIDNIGHTC_ED_"
INSTALL_RECEIPT_NAME = ".lostark-area-install.receipt.json"
MATERIAL_CLASSES = frozenset({"material", "materialinstanceconstant"})


class VariantError(RuntimeError):
    pass


def area_token(value: str) -> str:
    if (
        not value
        or len(value) > 128
        or not re.fullmatch(r"[A-Za-z0-9_.-]+", value)
        or value in (".", "..")
    ):
        raise VariantError(f"invalid area ID: {value!r}")
    return value


def load_json_snapshot(path: Path) -> tuple[dict[str, Any], str]:
    try:
        payload = path.read_bytes()
        value = json.loads(payload.decode("utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise VariantError(f"could not read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise VariantError(f"JSON root must be an object: {path}")
    return value, hashlib.sha256(payload).hexdigest().lower()


def load_json(path: Path) -> dict[str, Any]:
    return load_json_snapshot(path)[0]


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().lower()


def safe_relative_path(value: str) -> Path:
    normalized = value.replace("\\", "/")
    path = Path(normalized)
    if (
        not normalized
        or path.is_absolute()
        or path.drive
        or any(part in ("", ".", "..") for part in path.parts)
    ):
        raise VariantError(f"unsafe relative path: {value!r}")
    return path


def validate_runtime_material_coverage(
    coverage: Any, unsupported: Any, context: str
) -> tuple[bool, bool, bool]:
    if (
        not isinstance(coverage, dict)
        or not isinstance(coverage.get("textureDependencyClosureComplete"), bool)
        or not isinstance(coverage.get("textureSlotsComplete"), bool)
        or not isinstance(coverage.get("materialComplete"), bool)
        or not isinstance(unsupported, list)
    ):
        raise VariantError(f"runtime manifest material coverage gate failed: {context}")
    texture_dependency_closure_complete = coverage[
        "textureDependencyClosureComplete"
    ]
    texture_slots_complete = coverage["textureSlotsComplete"]
    material_complete = coverage["materialComplete"]
    if not texture_dependency_closure_complete:
        raise VariantError(
            "runtime texture dependency closure invariant failed: "
            f"{context}; exact source texture closure is incomplete"
        )
    expected_material_complete = texture_slots_complete and not unsupported
    if material_complete != expected_material_complete:
        raise VariantError(
            "runtime material coverage invariant failed: "
            f"{context}; materialComplete={material_complete}, "
            f"textureSlotsComplete={texture_slots_complete}, "
            f"sourceOnlyUnsupportedEmpty={not unsupported}"
        )
    return (
        texture_dependency_closure_complete,
        texture_slots_complete,
        material_complete,
    )


def variant_asset_id(base_asset_id: str, signature: str) -> str:
    if signature == scene.EMPTY_MATERIAL_SIGNATURE:
        return base_asset_id
    if not re.fullmatch(r"[0-9a-f]{64}", signature):
        raise VariantError(f"invalid material signature: {signature!r}")
    return f"{base_asset_id}_OVR_{signature[:12].upper()}"


def validate_variant_asset(asset: dict[str, Any]) -> None:
    full_path = str(asset.get("fullPath", ""))
    object_name = str(asset.get("objectName", ""))
    if not full_path or not object_name or full_path.split(".")[-1].casefold() != object_name.casefold():
        raise VariantError(f"invalid variant StaticMesh identity: {full_path!r}")
    try:
        computed_signature = scene.material_signature_from_slots(
            asset.get("materialSlots")
        )
    except ValueError as error:
        raise VariantError(str(error)) from error
    signature = str(asset.get("materialSignatureSha256", ""))
    if signature != computed_signature:
        raise VariantError(
            f"variant material signature mismatch: {signature!r} != {computed_signature}"
        )
    source_asset_id = base.stable_asset_id(full_path, object_name)
    expected_asset_id = variant_asset_id(source_asset_id, signature)
    if asset.get("sourceAssetId") != source_asset_id or asset.get("assetId") != expected_asset_id:
        raise VariantError(
            f"variant stable asset ID mismatch: {asset.get('assetId')!r} != {expected_asset_id}"
        )


def normalized_material_slots(
    placement: dict[str, Any], schema_version: int
) -> tuple[str, list[dict[str, Any]], bool]:
    try:
        signature = scene.placement_material_signature(placement, schema_version)
    except ValueError as error:
        raise VariantError(str(error)) from error
    if schema_version == 1:
        return signature, [], False
    material_overrides = placement["materialOverrides"]
    slots = [
        {
            "slot": int(slot["slot"]),
            "class": slot.get("class"),
            "objectPath": slot.get("objectPath"),
        }
        for slot in material_overrides["slots"]
    ]
    return signature, slots, bool(material_overrides["propertyPresent"])


def build_inventory(
    placement_directories: Sequence[Path],
    *,
    area_id: str = DEFAULT_AREA_ID,
    level_prefix: str = DEFAULT_LEVEL_PREFIX,
    expect_packages: int | None = 6,
    expect_source_meshes: int | None = 164,
    expect_variants: int | None = 292,
    expect_placements: int | None = 2951,
    expect_override_placements: int | None = 1649,
) -> dict[str, Any]:
    area_id = area_token(area_id)
    if not level_prefix:
        raise VariantError("level prefix cannot be empty")
    try:
        placement_paths = base.placement_files(placement_directories)
    except base.PipelineError as error:
        raise VariantError(str(error)) from error

    meshes: dict[str, dict[str, Any]] = {}
    variants: dict[tuple[str, str], dict[str, Any]] = {}
    source_rows: list[dict[str, Any]] = []
    level_counts: Counter[str] = Counter()
    package_names: set[str] = set()
    raw_override_signatures: set[str] = set()
    placement_count = 0
    override_placement_count = 0
    material_slot_count = 0
    non_null_slot_count = 0
    null_slot_count = 0
    any_negative_count = 0
    reflected_count = 0

    for path in placement_paths:
        document, placement_sha256 = load_json_snapshot(path)
        schema_version = document.get("schemaVersion")
        if (
            schema_version not in (1, 2)
            or document.get("propertyErrors")
            or document.get("unresolvedPlacements")
        ):
            raise VariantError(f"invalid placement source: {path}")
        selected_count = 0
        for placement in document.get("placements", []):
            if not isinstance(placement, dict):
                raise VariantError(f"placement row is not an object: {path}")
            level = str(placement.get("levelPackage", ""))
            if not level.casefold().startswith(level_prefix.casefold()):
                continue
            asset = placement.get("asset")
            if not isinstance(asset, dict):
                raise VariantError(f"placement asset is missing: {path}")
            full_path = str(asset.get("objectPath", "")).strip()
            object_name = str(asset.get("objectName", "")).strip()
            path_parts = full_path.split(".")
            if (
                len(path_parts) < 2
                or not object_name
                or path_parts[-1].casefold() != object_name.casefold()
            ):
                raise VariantError(f"invalid full StaticMesh path: {asset!r}")
            mesh_key = full_path.casefold()
            logical_package = path_parts[0]
            base_asset_id = base.stable_asset_id(full_path, object_name)
            mesh_row = {
                "assetId": base_asset_id,
                "sourceAssetId": base_asset_id,
                "fullPath": full_path,
                "logicalPackage": logical_package,
                "objectGroup": "/".join(path_parts[1:-1]),
                "objectName": object_name,
                "sourceCategory": "staticmesh",
                "rootImport": logical_package,
            }
            previous_mesh = meshes.get(mesh_key)
            if previous_mesh is not None:
                comparable = dict(previous_mesh)
                comparable["fullPath"] = full_path
                comparable["logicalPackage"] = logical_package
                comparable["objectName"] = object_name
                comparable["rootImport"] = logical_package
                if comparable != mesh_row:
                    raise VariantError(f"conflicting StaticMesh identity: {full_path}")
            else:
                meshes[mesh_key] = mesh_row

            signature, slots, property_present = normalized_material_slots(
                placement, int(schema_version)
            )
            variant_id = variant_asset_id(base_asset_id, signature)
            variant_key = (mesh_key, signature)
            variant_row = {
                **mesh_row,
                "assetId": variant_id,
                "materialSignatureSha256": signature,
                "materialSlots": slots,
            }
            previous_variant = variants.get(variant_key)
            if previous_variant is not None and previous_variant != variant_row:
                raise VariantError(
                    f"conflicting material variant identity: {full_path} / {signature}"
                )
            variants[variant_key] = variant_row
            raw_override_signatures.add(signature)

            if property_present:
                override_placement_count += 1
                material_slot_count += len(slots)
                non_null_slot_count += sum(
                    slot["objectPath"] is not None for slot in slots
                )
                null_slot_count += sum(
                    slot["objectPath"] is None for slot in slots
                )
            scale = placement.get("transform", {}).get("scale3D", {})
            try:
                signed_scale = tuple(float(scale[axis]) for axis in ("x", "y", "z"))
            except (KeyError, TypeError, ValueError) as error:
                raise VariantError(f"invalid placement scale: {path}") from error
            if any(value < 0.0 for value in signed_scale):
                any_negative_count += 1
            if signed_scale[0] * signed_scale[1] * signed_scale[2] < 0.0:
                reflected_count += 1
            placement_count += 1
            selected_count += 1
            level_counts[level] += 1
            package_names.add(level)
        if selected_count:
            source_rows.append(
                {
                    "path": str(path.resolve()),
                    "sha256": placement_sha256,
                    "schemaVersion": schema_version,
                    "selectedPlacementCount": selected_count,
                }
            )

    source_meshes = sorted(meshes.values(), key=lambda row: row["fullPath"].casefold())
    assets = sorted(
        variants.values(),
        key=lambda row: (
            row["fullPath"].casefold(),
            row["materialSignatureSha256"],
        ),
    )
    if len({row["assetId"] for row in source_meshes}) != len(source_meshes):
        raise VariantError("stable source mesh asset ID collision")
    variant_ids: dict[str, tuple[str, str]] = {}
    for key, row in variants.items():
        validate_variant_asset(row)
        previous_key = variant_ids.get(row["assetId"])
        if previous_key is not None and previous_key != key:
            raise VariantError(
                f"truncated material variant asset ID collision: {row['assetId']}"
            )
        variant_ids[row["assetId"]] = key

    actual_gates = {
        "packages": len(package_names),
        "sourceMeshes": len(source_meshes),
        "variants": len(assets),
        "placements": placement_count,
        "overridePlacements": override_placement_count,
    }
    expected_gates = {
        "packages": expect_packages,
        "sourceMeshes": expect_source_meshes,
        "variants": expect_variants,
        "placements": expect_placements,
        "overridePlacements": expect_override_placements,
    }
    for name, expected in expected_gates.items():
        if expected is not None and actual_gates[name] != expected:
            raise VariantError(
                f"{name} count gate failed: {actual_gates[name]} != {expected}"
            )

    return {
        "schemaVersion": 1,
        "areaId": area_id,
        "levelPrefix": level_prefix,
        "sourceMeshCount": len(source_meshes),
        "assetCount": len(assets),
        "placementCount": placement_count,
        "summary": {
            **actual_gates,
            "noOverridePlacements": placement_count - override_placement_count,
            "uniqueRawOverrideSignatures": len(raw_override_signatures),
            "materialSlots": material_slot_count,
            "nonNullMaterialSlots": non_null_slot_count,
            "nullMaterialSlots": null_slot_count,
            "anyNegativeScalePlacements": any_negative_count,
            "reflectedPlacements": reflected_count,
            "levelCounts": dict(sorted(level_counts.items())),
        },
        "sources": source_rows,
        "sourceMeshes": source_meshes,
        "assets": assets,
    }


def exact_candidate(
    candidates: Sequence[dict[str, Any]],
    object_path: str,
    *,
    expected_class: str | None = None,
    kind: str = "material",
) -> dict[str, Any]:
    key = object_path.casefold()
    rows = [
        row
        for row in candidates
        if str(row.get("objectPath", "")).casefold() == key
    ]
    if len(rows) != 1:
        raise VariantError(
            f"expected exactly one {kind} candidate for {object_path}, found {len(rows)}"
        )
    row = rows[0]
    if expected_class is not None and str(row.get("class", "")).casefold() != expected_class.casefold():
        raise VariantError(
            f"{kind} class mismatch for {object_path}: "
            f"{row.get('class')!r} != {expected_class!r}"
        )
    return row


def contextual_candidate(
    candidates: Sequence[dict[str, Any]],
    object_path: str,
    *,
    context_object_path: str | None = None,
    preferred_source_pack: str | None = None,
    expected_class: str | None = None,
    kind: str = "material",
) -> dict[str, Any]:
    key = object_path.casefold()
    rows = [
        row
        for row in candidates
        if str(row.get("objectPath", "")).casefold() == key
    ]
    if not rows and context_object_path is not None:
        package = context_object_path.split(".", 1)[0]
        contextual_path = f"{package}.{object_path}"
        rows = [
            row
            for row in candidates
            if str(row.get("objectPath", "")).casefold()
            == contextual_path.casefold()
        ]
    if not rows:
        suffix = f".{key}"
        rows = [
            row
            for row in candidates
            if str(row.get("objectPath", "")).casefold().endswith(suffix)
        ]
    if len(rows) > 1 and preferred_source_pack is not None:
        preferred_key = preferred_source_pack.casefold()
        preferred = [
            row
            for row in rows
            if preferred_key
            in {
                str(value).casefold()
                for value in row.get(
                    "sourcePacks", [row.get("sourcePack", "")]
                )
            }
        ]
        if preferred:
            rows = preferred
    if len(rows) != 1:
        raise VariantError(
            f"expected exactly one {kind} candidate for {object_path}, "
            f"found {len(rows)} (context={context_object_path!r}, "
            f"sourcePack={preferred_source_pack!r})"
        )
    row = rows[0]
    if expected_class is not None and str(row.get("class", "")).casefold() != expected_class.casefold():
        raise VariantError(
            f"{kind} class mismatch for {object_path}: "
            f"{row.get('class')!r} != {expected_class!r}"
        )
    return row


def parse_exact_material_document(path: Path) -> dict[str, Any]:
    document = base.parse_material_document(path)
    text = path.read_text(encoding="utf-8", errors="replace")
    exact_textures: dict[str, str] = {}
    for block in re.finditer(
        r"ParameterValue\s*=\s*Texture2D'([^']+)'(?:(?!ParameterValue).)*?"
        r"ParameterName\s*=\s*([A-Za-z0-9_]+)",
        text,
        re.DOTALL,
    ):
        exact_textures[block.group(2).casefold()] = block.group(1).strip()
    for block in re.finditer(
        r"Texture\s*=\s*Texture2D'([^']+)'\s*[\r\n]\s*Name\s*=\s*"
        r"([A-Za-z0-9_]+)",
        text,
    ):
        exact_textures.setdefault(block.group(2).casefold(), block.group(1).strip())
    document["textures"] = exact_textures
    referenced_textures: list[str] = []
    for block in re.finditer(
        r"ReferencedTextures\[[0-9]+\]\s*=\s*\{([^}]*)\}",
        text,
        re.DOTALL,
    ):
        referenced_textures.extend(
            match.group(1).strip()
            for match in re.finditer(r"Texture2D'([^']+)'", block.group(1))
        )
    document["referencedTextures"] = referenced_textures
    return document


def resolve_material_contracts(
    required: Sequence[tuple[str, str | None]],
    material_candidates: Sequence[dict[str, Any]],
    texture_candidates: Sequence[dict[str, Any]],
) -> dict[str, dict[str, Any]]:
    resolved: dict[str, dict[str, Any]] = {}
    visiting: set[str] = set()

    def empty_contract() -> dict[str, Any]:
        return {
            "textures": {},
            "textureContexts": {},
            "textureSourcePacks": {},
            "referencedTextures": [],
            "scalars": {},
            "vectors": {},
            "flags": {},
            "parentChain": [],
        }

    def visit(
        object_path: str,
        expected_class: str | None,
        context_object_path: str | None = None,
    ) -> dict[str, Any]:
        candidate = contextual_candidate(
            material_candidates,
            object_path,
            context_object_path=context_object_path,
            expected_class=expected_class,
            kind="material",
        )
        canonical_object_path = str(candidate["objectPath"])
        key = canonical_object_path.casefold()
        existing = resolved.get(key)
        if existing is not None:
            if expected_class is not None and existing["class"].casefold() != expected_class.casefold():
                raise VariantError(f"material class mismatch for {object_path}")
            return existing
        if key in visiting:
            raise VariantError(f"material parent cycle: {canonical_object_path}")
        class_name = str(candidate.get("class", ""))
        if class_name.casefold() not in MATERIAL_CLASSES:
            raise VariantError(f"unsupported material class for {object_path}: {class_name}")
        source_only_reason = candidate.get("sourceOnlyReason")
        if source_only_reason:
            contract = empty_contract()
            contract["parentChain"].append(str(candidate["objectPath"]))
            contract["resolvedTextures"] = {}
            contract["resolvedReferencedTextures"] = []
            contract["objectPath"] = str(candidate["objectPath"])
            contract["class"] = class_name
            contract["sourcePack"] = candidate.get("sourcePack")
            contract["sourceOnlyReason"] = str(source_only_reason)
            resolved[key] = contract
            return contract
        props_path = Path(str(candidate.get("props", "")))
        if not props_path.is_file():
            raise VariantError(f"material props are missing: {props_path}")
        visiting.add(key)
        document = parse_exact_material_document(props_path)
        parent = document.get("parent")
        if parent:
            if not isinstance(parent, str) or "." not in parent:
                raise VariantError(
                    f"material parent is not a full canonical objectPath: {parent!r}"
                )
            inherited = visit(parent, None, canonical_object_path)
            contract = {
                "textures": dict(inherited["textures"]),
                "textureContexts": dict(inherited["textureContexts"]),
                "textureSourcePacks": dict(inherited["textureSourcePacks"]),
                "referencedTextures": list(inherited["referencedTextures"]),
                "scalars": dict(inherited["scalars"]),
                "vectors": dict(inherited["vectors"]),
                "flags": dict(inherited["flags"]),
                "parentChain": list(inherited["parentChain"]),
            }
        else:
            contract = empty_contract()
        contract["textures"].update(document["textures"])
        contract["textureContexts"].update(
            {parameter: canonical_object_path for parameter in document["textures"]}
        )
        contract["textureSourcePacks"].update(
            {
                parameter: str(candidate.get("sourcePack", ""))
                for parameter in document["textures"]
            }
        )
        contract["referencedTextures"].extend(
            {
                "declaredObjectPath": object_path,
                "declaredByMaterial": canonical_object_path,
                "sourcePack": str(candidate.get("sourcePack", "")),
            }
            for object_path in document["referencedTextures"]
        )
        contract["scalars"].update(document["scalars"])
        contract["vectors"].update(document["vectors"])
        contract["flags"].update(document["flags"])
        contract["parentChain"].append(str(candidate["objectPath"]))
        contract["objectPath"] = str(candidate["objectPath"])
        contract["class"] = class_name
        contract["sourcePack"] = candidate.get("sourcePack")
        resolved[key] = contract
        visiting.remove(key)
        return contract

    required_contracts: dict[str, dict[str, Any]] = {}
    for object_path, class_name in required:
        contract = visit(object_path, class_name)
        required_contracts[object_path.casefold()] = contract
        required_contracts[str(contract["objectPath"]).casefold()] = contract
    for contract in {id(row): row for row in required_contracts.values()}.values():
        texture_rows: dict[str, dict[str, Any]] = {}
        for parameter, texture_path in sorted(contract["textures"].items()):
            texture = contextual_candidate(
                texture_candidates,
                texture_path,
                context_object_path=contract["textureContexts"].get(
                    parameter, contract["objectPath"]
                ),
                preferred_source_pack=contract["textureSourcePacks"].get(
                    parameter
                ),
                kind="texture",
            )
            source = Path(str(texture.get("source", "")))
            if not source.is_file():
                raise VariantError(f"texture source is missing: {source}")
            texture_rows[parameter] = {
                "objectPath": str(texture["objectPath"]),
                "source": str(source),
                "sha256": sha256_file(source),
            }
        contract["resolvedTextures"] = texture_rows
        referenced_rows: list[dict[str, Any]] = []
        seen_references: set[tuple[str, str]] = set()
        for reference in contract["referencedTextures"]:
            declared_object_path = str(reference["declaredObjectPath"])
            declared_by_material = str(reference["declaredByMaterial"])
            texture = contextual_candidate(
                texture_candidates,
                declared_object_path,
                context_object_path=declared_by_material,
                preferred_source_pack=str(reference.get("sourcePack", "")),
                kind="referenced texture",
            )
            source = Path(str(texture.get("source", "")))
            if not source.is_file():
                raise VariantError(f"referenced texture source is missing: {source}")
            key = (
                str(texture["objectPath"]).casefold(),
                declared_by_material.casefold(),
            )
            if key in seen_references:
                continue
            seen_references.add(key)
            referenced_rows.append(
                {
                    "declaredObjectPath": declared_object_path,
                    "declaredByMaterial": declared_by_material,
                    "objectPath": str(texture["objectPath"]),
                    "source": str(source),
                    "sha256": sha256_file(source),
                }
            )
        contract["resolvedReferencedTextures"] = referenced_rows
    return required_contracts


def make_slot_unique_gltf(source: Path, destination: Path) -> list[dict[str, Any]]:
    document = load_json(source)
    materials = document.get("materials")
    if not isinstance(materials, list) or not materials:
        raise VariantError(f"glTF has no material slots: {source}")
    rows: list[dict[str, Any]] = []
    for slot, material in enumerate(materials):
        if not isinstance(material, dict):
            raise VariantError(f"invalid glTF material row at slot {slot}")
        original = str(material.get("name", "")).strip()
        if not original:
            raise VariantError(f"unnamed glTF material at slot {slot}")
        label = re.sub(r"[^A-Za-z0-9_]+", "_", original).strip("_") or "MATERIAL"
        unique = f"SLOT_{slot:03d}_{label}"
        material["name"] = unique
        rows.append({"slot": slot, "sourceName": original, "runtimeName": unique})
    for mesh in document.get("meshes", []):
        if not isinstance(mesh, dict):
            raise VariantError("invalid glTF mesh row")
        for primitive in mesh.get("primitives", []):
            if not isinstance(primitive, dict):
                raise VariantError("invalid glTF primitive row")
            material_index = primitive.get("material")
            if material_index is not None and (
                not isinstance(material_index, int)
                or not 0 <= material_index < len(materials)
            ):
                raise VariantError(f"glTF primitive material index is invalid: {material_index}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    base.atomic_write_json(destination, document)
    return rows


UMODEL_EXPORT_PATTERN = re.compile(
    r"^Exporting\s+(MaterialInstanceConstant|Material3|Material|Texture2D)\s+"
    r"([^\s]+)\s+to\s+(.+?)/umodel/(.+)$",
    re.IGNORECASE,
)
UMODEL_IGNORED_EMPTY_MATERIAL_PATTERN = re.compile(
    r"Ignoring\s+Material3'([^']+)'\s+due\s+to\s+empty\s+parameters",
    re.IGNORECASE,
)
UMODEL_LOADED_TEXTURE_PATTERN = re.compile(
    r"^Loading\s+Texture2D\s+([^\s]+)\s+from\s+package\s+([^\s]+\.upk)$",
    re.IGNORECASE,
)


def _canonical_export_path(relative_path: str) -> str:
    path = relative_path.replace("\\", "/")
    suffixes = (".props.txt", ".mat", ".dds", ".tga", ".png")
    for suffix in suffixes:
        if path.casefold().endswith(suffix):
            path = path[: -len(suffix)]
            break
    parts = [part for part in path.split("/") if part]
    if parts and parts[0].casefold().endswith("_prologue"):
        parts.pop(0)
    if len(parts) < 2:
        raise VariantError(f"UModel export path is not canonicalizable: {relative_path}")
    return ".".join(parts)


def parse_umodel_export_log(path: Path) -> list[dict[str, str]]:
    records: list[dict[str, str]] = []
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = UMODEL_EXPORT_PATTERN.match(line.strip())
        if match is None:
            continue
        source_class, object_name, _prefix, relative = match.groups()
        class_name = (
            "MaterialInstanceConstant"
            if source_class.casefold() == "materialinstanceconstant"
            else "Material"
            if source_class.casefold() in {"material", "material3"}
            else "Texture2D"
        )
        records.append(
            {
                "class": class_name,
                "sourceClass": source_class,
                "objectName": object_name,
                "relativePath": relative.replace("\\", "/"),
                "objectPath": _canonical_export_path(relative),
            }
        )
    return records


def parse_umodel_ignored_empty_materials(path: Path) -> set[str]:
    return {
        match.group(1).casefold()
        for match in UMODEL_IGNORED_EMPTY_MATERIAL_PATTERN.finditer(
            path.read_text(encoding="utf-8", errors="replace")
        )
    }


def missing_loaded_texture_sources(source_roots: Sequence[Path]) -> list[dict[str, str]]:
    missing: dict[tuple[str, str], dict[str, str]] = {}
    for source_root in source_roots:
        if not source_root.is_dir():
            continue
        for pack in sorted(
            (path for path in source_root.iterdir() if path.is_dir()),
            key=lambda path: path.name.casefold(),
        ):
            log_path = pack / "umodel.log.txt"
            if not log_path.is_file():
                continue
            text = log_path.read_text(encoding="utf-8", errors="replace")
            exported = {
                row["objectName"].casefold()
                for row in parse_umodel_export_log(log_path)
                if row["class"] == "Texture2D"
            }
            for line in text.splitlines():
                match = UMODEL_LOADED_TEXTURE_PATTERN.match(line.strip())
                if match is None:
                    continue
                object_name, physical_package = match.groups()
                if object_name.casefold() in exported:
                    continue
                key = (physical_package.casefold(), object_name.casefold())
                missing.setdefault(
                    key,
                    {
                        "physicalPackage": physical_package,
                        "objectName": object_name,
                    },
                )
    return [missing[key] for key in sorted(missing)]


def _select_props_for_record(
    pack: Path, record: dict[str, str]
) -> tuple[Path | None, str | None]:
    material_root = pack / "materials"
    rows = sorted(
        material_root.glob(f"*__{record['objectName']}.props.txt"),
        key=lambda path: path.name.casefold(),
    )
    if not rows:
        return None, "missing-props"
    classified: list[tuple[Path, bool, str]] = []
    for row in rows:
        document = parse_exact_material_document(row)
        classified.append((row, bool(document.get("parent")), sha256_file(row)))
    if record["class"] == "Material":
        preferred = [entry for entry in classified if not entry[1]]
    else:
        preferred = [entry for entry in classified if entry[1]]
    candidates = preferred or classified
    hashes = {entry[2] for entry in candidates}
    if len(hashes) != 1:
        return None, "ambiguous-props"
    return sorted(
        (entry[0] for entry in candidates), key=lambda path: str(path).casefold()
    )[0], None


def _source_texture_for_record(
    pack: Path, record: dict[str, str]
) -> tuple[Path | None, str | None]:
    rows = [
        path
        for path in (pack / "textures").glob("*")
        if path.is_file()
        and path.stem.casefold() == record["objectName"].casefold()
        and path.suffix.casefold() in {".dds", ".tga", ".png"}
    ]
    if not rows:
        return None, "missing-texture"
    hashes = {sha256_file(path) for path in rows}
    if len(hashes) != 1:
        return None, "ambiguous-texture"
    return sorted(rows, key=lambda path: str(path).casefold())[0], None


def _deduplicate_material_candidates(
    candidates: Sequence[dict[str, Any]], gaps: list[dict[str, Any]]
) -> list[dict[str, Any]]:
    grouped: dict[tuple[str, str], list[dict[str, Any]]] = {}
    for row in candidates:
        key = (str(row["objectPath"]).casefold(), str(row["class"]).casefold())
        grouped.setdefault(key, []).append(row)
    result: list[dict[str, Any]] = []
    for key, rows in sorted(grouped.items()):
        exported_rows = [row for row in rows if row.get("propsSha256")]
        if not exported_rows:
            reasons = {str(row.get("sourceOnlyReason", "")) for row in rows}
            if reasons != {"umodel-empty-material3"}:
                gaps.append(
                    {
                        "kind": "material-source-conflict",
                        "objectPath": rows[0]["objectPath"],
                        "class": rows[0]["class"],
                        "candidateCount": len(rows),
                        "sourceOnlyReasons": sorted(reasons),
                    }
                )
                continue
            selected = sorted(
                rows, key=lambda row: str(row.get("evidence", "")).casefold()
            )[0]
            result.append(
                {
                    "objectPath": selected["objectPath"],
                    "class": selected["class"],
                    "sourceOnlyReason": "umodel-empty-material3",
                    "evidence": selected.get("evidence"),
                    "sourcePack": selected.get("sourcePack"),
                }
            )
            continue
        hashes = {str(row["propsSha256"]) for row in exported_rows}
        if len(hashes) != 1:
            gaps.append(
                {
                    "kind": "material-source-conflict",
                    "objectPath": rows[0]["objectPath"],
                    "class": rows[0]["class"],
                    "candidateCount": len(exported_rows),
                    "hashes": sorted(hashes),
                }
            )
            continue
        selected = sorted(
            exported_rows, key=lambda row: str(row["props"]).casefold()
        )[0]
        result.append(
            {
                "objectPath": selected["objectPath"],
                "class": selected["class"],
                "props": selected["props"],
                "propsSha256": selected["propsSha256"],
                "sourcePack": selected.get("sourcePack"),
            }
        )
    return result


def _deduplicate_texture_candidates(
    candidates: Sequence[dict[str, Any]], gaps: list[dict[str, Any]]
) -> list[dict[str, Any]]:
    grouped: dict[str, list[dict[str, Any]]] = {}
    for row in candidates:
        grouped.setdefault(str(row["objectPath"]).casefold(), []).append(row)
    result: list[dict[str, Any]] = []
    for key, rows in sorted(grouped.items()):
        hashes = {str(row["sha256"]) for row in rows}
        if len(hashes) != 1:
            gaps.append(
                {
                    "kind": "texture-source-conflict",
                    "objectPath": rows[0]["objectPath"],
                    "candidateCount": len(rows),
                    "hashes": sorted(hashes),
                }
            )
            continue
        selected = sorted(rows, key=lambda row: str(row["source"]).casefold())[0]
        result.append(
            {
                "objectPath": selected["objectPath"],
                "source": selected["source"],
                "sha256": selected["sha256"],
                "sourcePacks": sorted(
                    {
                        str(row.get("sourcePack", ""))
                        for row in rows
                        if row.get("sourcePack")
                    },
                    key=str.casefold,
                ),
            }
        )
    return result


def build_observed_material_catalog(
    inventory: dict[str, Any], source_root: Path,
    material_source_root: Path | None = None,
    texture_source_root: Path | None = None,
) -> dict[str, Any]:
    source_meshes = inventory.get("sourceMeshes")
    if not isinstance(source_meshes, list) or len(source_meshes) != inventory.get("sourceMeshCount"):
        raise VariantError("source mesh inventory is incomplete")
    required_overrides = {
        (str(slot["objectPath"]).casefold(), str(slot["class"]).casefold()): {
            "objectPath": str(slot["objectPath"]),
            "class": str(slot["class"]),
        }
        for asset in inventory.get("assets", [])
        for slot in asset.get("materialSlots", [])
        if slot.get("objectPath") is not None
    }
    gaps: list[dict[str, Any]] = []
    material_candidates: list[dict[str, Any]] = []
    texture_candidates: list[dict[str, Any]] = []
    mesh_defaults: list[dict[str, Any]] = []

    def collect_pack_dependencies(
        pack: Path, source_id: str
    ) -> list[dict[str, str]]:
        log_path = pack / "umodel.log.txt"
        if not log_path.is_file():
            gaps.append({"kind": "missing-umodel-log", "sourceId": source_id})
            return []
        records = parse_umodel_export_log(log_path)
        for record in records:
            if record["class"].casefold() in MATERIAL_CLASSES:
                props, reason = _select_props_for_record(pack, record)
                if props is None:
                    gaps.append(
                        {
                            "kind": reason,
                            "objectPath": record["objectPath"],
                            "class": record["class"],
                            "sourceId": source_id,
                        }
                    )
                    continue
                material_candidates.append(
                    {
                        "objectPath": record["objectPath"],
                        "class": record["class"],
                        "props": str(props),
                        "propsSha256": sha256_file(props),
                        "sourcePack": str(pack),
                    }
                )
            elif record["class"] == "Texture2D":
                source, _reason = _source_texture_for_record(pack, record)
                if source is None:
                    # UModel reports inherited engine defaults that it elects
                    # not to export. Only a resolved material contract can turn
                    # one of these into a required dependency gap.
                    continue
                texture_candidates.append(
                    {
                        "objectPath": record["objectPath"],
                        "source": str(source),
                        "sha256": sha256_file(source),
                        "sourcePack": str(pack),
                    }
                )
        return records

    for mesh in source_meshes:
        pack = source_root / str(mesh["sourceAssetId"])
        receipt_path = pack / "source.receipt.json"
        log_path = pack / "umodel.log.txt"
        if not receipt_path.is_file() or not log_path.is_file():
            gaps.append(
                {
                    "kind": "missing-source-pack",
                    "meshObjectPath": mesh["fullPath"],
                    "sourceAssetId": mesh["sourceAssetId"],
                }
            )
            continue
        receipt = load_json(receipt_path)
        if str(receipt.get("fullPath", "")).casefold() != str(mesh["fullPath"]).casefold():
            raise VariantError(f"source receipt mismatch: {receipt_path}")
        records = collect_pack_dependencies(pack, str(mesh["sourceAssetId"]))
        ignored_empty_materials = parse_umodel_ignored_empty_materials(log_path)
        material_records = [
            row
            for row in records
            if row["class"].casefold() in MATERIAL_CLASSES
        ]
        texture_records = [row for row in records if row["class"] == "Texture2D"]
        defaults: list[dict[str, Any]] = []
        logical_package = str(mesh["logicalPackage"]).casefold()
        for required in required_overrides.values():
            parts = str(required["objectPath"]).split(".")
            if (
                len(parts) >= 2
                and parts[0].casefold() == logical_package
                and parts[-1].casefold() in ignored_empty_materials
            ):
                material_candidates.append(
                    {
                        **required,
                        "sourceOnlyReason": "umodel-empty-material3",
                        "evidence": str(log_path),
                        "sourcePack": str(pack),
                    }
                )
        for slot, material in enumerate(receipt.get("materials", [])):
            material_name = str(material.get("name", ""))
            candidates = [
                row
                for row in material_records
                if row["objectName"].casefold() == material_name.casefold()
            ]
            same_package = [
                row
                for row in candidates
                if row["objectPath"].split(".")[0].casefold() == logical_package
            ]
            mic_candidates = [
                row for row in (same_package or candidates)
                if row["class"] == "MaterialInstanceConstant"
            ]
            selected_rows = mic_candidates or same_package or candidates
            identities = {
                (row["objectPath"].casefold(), row["class"].casefold())
                for row in selected_rows
            }
            if len(identities) != 1:
                if not selected_rows and material_name.casefold().startswith(
                    "dummy_material_"
                ):
                    defaults.append(
                        {
                            "slot": slot,
                            "class": None,
                            "objectPath": None,
                            "sourceOnlyReason": "umodel-dummy-material",
                            "sourceMaterialName": material_name,
                        }
                    )
                    continue
                if (
                    not selected_rows
                    and material_name.casefold() in ignored_empty_materials
                ):
                    defaults.append(
                        {
                            "slot": slot,
                            "class": None,
                            "objectPath": None,
                            "sourceOnlyReason": "umodel-empty-material3",
                            "sourceMaterialName": material_name,
                        }
                    )
                    continue
                gaps.append(
                    {
                        "kind": "mesh-default-not-exact-one",
                        "meshObjectPath": mesh["fullPath"],
                        "slot": slot,
                        "materialName": material_name,
                        "candidates": sorted(
                            f"{row['class']}|{row['objectPath']}"
                            for row in selected_rows
                        ),
                    }
                )
                continue
            selected = selected_rows[0]
            defaults.append(
                {
                    "slot": slot,
                    "class": selected["class"],
                    "objectPath": selected["objectPath"],
                }
            )
        if len(defaults) == len(receipt.get("materials", [])) and defaults:
            mesh_defaults.append(
                {"meshObjectPath": mesh["fullPath"], "slots": defaults}
            )

    if material_source_root is not None:
        if not material_source_root.is_dir():
            raise VariantError(
                f"material source root is missing: {material_source_root}"
            )
        for pack in sorted(
            (path for path in material_source_root.iterdir() if path.is_dir()),
            key=lambda path: path.name.casefold(),
        ):
            collect_pack_dependencies(pack, pack.name)
    if texture_source_root is not None:
        if not texture_source_root.is_dir():
            raise VariantError(
                f"texture source root is missing: {texture_source_root}"
            )
        for pack in sorted(
            (path for path in texture_source_root.iterdir() if path.is_dir()),
            key=lambda path: path.name.casefold(),
        ):
            collect_pack_dependencies(pack, pack.name)

    materials = _deduplicate_material_candidates(material_candidates, gaps)
    textures = _deduplicate_texture_candidates(texture_candidates, gaps)
    material_keys = {
        (str(row["objectPath"]).casefold(), str(row["class"]).casefold())
        for row in materials
    }
    for key, row in sorted(required_overrides.items()):
        if key not in material_keys:
            gaps.append({"kind": "missing-override-material", **row})

    gap_counts = Counter(str(row["kind"]) for row in gaps)
    return {
        "schemaVersion": 1,
        "areaId": inventory["areaId"],
        "sourceMeshCount": len(source_meshes),
        "materialCount": len(materials),
        "textureCount": len(textures),
        "meshDefaultCount": len(mesh_defaults),
        "admissionReady": not gaps and len(mesh_defaults) == len(source_meshes),
        "summary": {
            "requiredOverrideMaterialCount": len(required_overrides),
            "materialCount": len(materials),
            "textureCount": len(textures),
            "meshDefaultCount": len(mesh_defaults),
            "sourceOnlyMaterialCount": sum(
                1 for row in materials if row.get("sourceOnlyReason")
            ),
            "gapCount": len(gaps),
            "gapCounts": dict(sorted(gap_counts.items())),
        },
        "materials": materials,
        "textures": textures,
        "meshDefaults": mesh_defaults,
        "gaps": gaps,
    }


def _material_source_id(object_path: str) -> str:
    digest = hashlib.sha1(object_path.casefold().encode("utf-8")).hexdigest()
    return f"MAT_{digest[:16].upper()}"


def _material_source_receipt_valid(
    destination: Path, object_path: str, class_name: str
) -> bool:
    try:
        receipt = load_json(destination / "source.receipt.json")
        if (
            receipt.get("objectPath") != object_path
            or str(receipt.get("class", "")).casefold() != class_name.casefold()
        ):
            return False
        for output in receipt.get("outputs", []):
            path = destination / safe_relative_path(str(output["path"]))
            if not path.is_file() or sha256_file(path) != str(output["sha256"]):
                return False
        return True
    except (VariantError, OSError, KeyError, TypeError, ValueError):
        return False


def export_exact_material_source(
    material: dict[str, Any],
    *,
    output_root: Path,
    umodel: Path,
    package_root: Path,
    region: str,
    timeout: float,
    force: bool,
) -> dict[str, Any]:
    object_path = str(material["objectPath"])
    class_name = str(material["class"])
    source_id = _material_source_id(object_path)
    destination = output_root / source_id
    if not force and _material_source_receipt_valid(
        destination, object_path, class_name
    ):
        return {"assetId": source_id, "fullPath": object_path, "status": "resumed"}
    parts = object_path.split(".")
    if len(parts) < 2:
        raise VariantError(f"material objectPath is not canonical: {object_path}")
    logical_package = parts[0]
    object_name = parts[-1]
    staging_root = output_root.parent / ".staging" / "material-extract"
    staging_root.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix=f"{source_id}.", dir=staging_root))
    export_root = work / "umodel"
    pack = work / "pack"
    try:
        command = [
            str(umodel),
            "-export",
            "-game=lostark",
            f"-{region}",
            "-nameresolve",
            "-dds",
            "-uncook",
            "-groups",
            f"-path={package_root}",
            f"-out={export_root}",
            f"-obj={object_name}",
            logical_package,
        ]
        try:
            completed = base.run(
                command, umodel.parent, timeout, f"UModel material export {object_path}"
            )
        except base.PipelineError as error:
            raise VariantError(str(error)) from error
        console = completed.stdout + "\n" + completed.stderr
        temporary_log = work / "umodel.log.txt"
        temporary_log.write_text(console, encoding="utf-8", newline="\n")
        records = parse_umodel_export_log(temporary_log)
        target_rows = [
            row
            for row in records
            if row["objectPath"].casefold() == object_path.casefold()
            and row["class"].casefold() == class_name.casefold()
        ]
        identities = {
            (row["objectPath"].casefold(), row["class"].casefold())
            for row in target_rows
        }
        if len(identities) != 1 or not target_rows:
            raise VariantError(
                f"UModel did not export exact material identity {class_name}|{object_path}"
            )
        pack.mkdir(parents=True)
        (pack / "umodel.log.txt").write_text(
            console, encoding="utf-8", newline="\n"
        )
        material_directory = pack / "materials"
        texture_directory = pack / "textures"
        copied_props: dict[str, Path] = {}
        copied_textures: dict[str, tuple[str, Path]] = {}
        for record in records:
            relative = safe_relative_path(record["relativePath"])
            if record["class"].casefold() in MATERIAL_CLASSES:
                props_relative = relative.with_suffix(".props.txt")
                props = export_root / props_relative
                if not props.is_file():
                    continue
                props_hash = sha256_file(props)
                target = material_directory / f"{props_hash[:12].upper()}__{props.name}"
                target.parent.mkdir(parents=True, exist_ok=True)
                if not target.exists():
                    shutil.copy2(props, target)
                copied_props[props_hash] = target
            elif record["class"] == "Texture2D":
                source = export_root / relative
                if not source.is_file():
                    continue
                source_hash = sha256_file(source)
                key = source.name.casefold()
                previous = copied_textures.get(key)
                if previous is not None and previous[0] != source_hash:
                    raise VariantError(
                        f"material source texture filename collision: {source.name}"
                    )
                target = texture_directory / source.name
                target.parent.mkdir(parents=True, exist_ok=True)
                if not target.exists():
                    shutil.copy2(source, target)
                copied_textures[key] = (source_hash, target)
        exact_target = [
            row
            for row in target_rows
            if (
                export_root
                / safe_relative_path(row["relativePath"]).with_suffix(".props.txt")
            ).is_file()
        ]
        if not exact_target:
            raise VariantError(f"exact material props were not exported: {object_path}")
        outputs = [
            {
                "path": path.relative_to(pack).as_posix(),
                "sha256": sha256_file(path),
            }
            for path in sorted(path for path in pack.rglob("*") if path.is_file())
        ]
        receipt = {
            "schemaVersion": 1,
            "sourceId": source_id,
            "objectPath": object_path,
            "class": class_name,
            "logicalPackage": logical_package,
            "objectName": object_name,
            "materialDependencyCount": len(copied_props),
            "textureDependencyCount": len(copied_textures),
            "outputs": outputs,
        }
        base.atomic_write_json(pack / "source.receipt.json", receipt)
        try:
            base.commit_directory(pack, destination, output_root, force=True)
        except base.PipelineError as error:
            raise VariantError(str(error)) from error
        return {
            "assetId": source_id,
            "fullPath": object_path,
            "status": "exported",
        }
    finally:
        if work.exists():
            try:
                base.bounded_rmtree(work, output_root.parent)
            except base.PipelineError as error:
                raise VariantError(str(error)) from error


def _texture_source_id(physical_package: str, object_name: str) -> str:
    identity = f"{physical_package.casefold()}|{object_name.casefold()}"
    digest = hashlib.sha1(identity.encode("utf-8")).hexdigest()
    return f"TEX_{digest[:16].upper()}"


def _texture_source_receipt_valid(
    destination: Path, physical_package: str, object_name: str
) -> bool:
    try:
        receipt = load_json(destination / "source.receipt.json")
        if (
            str(receipt.get("physicalPackage", "")).casefold()
            != physical_package.casefold()
            or str(receipt.get("objectName", "")).casefold()
            != object_name.casefold()
        ):
            return False
        for output in receipt.get("outputs", []):
            path = destination / safe_relative_path(str(output["path"]))
            if not path.is_file() or sha256_file(path) != str(output["sha256"]):
                return False
        return True
    except (VariantError, OSError, KeyError, TypeError, ValueError):
        return False


def export_exact_texture_source(
    texture: dict[str, Any],
    *,
    output_root: Path,
    umodel: Path,
    package_root: Path,
    region: str,
    timeout: float,
    force: bool,
) -> dict[str, Any]:
    physical_package = str(texture["physicalPackage"])
    object_name = str(texture["objectName"])
    package_relative = safe_relative_path(physical_package)
    if len(package_relative.parts) != 1 or package_relative.suffix.casefold() != ".upk":
        raise VariantError(f"invalid physical texture package: {physical_package}")
    physical_path = package_root / package_relative
    if not physical_path.is_file():
        raise VariantError(f"physical texture package is missing: {physical_path}")
    source_id = _texture_source_id(physical_package, object_name)
    destination = output_root / source_id
    if not force and _texture_source_receipt_valid(
        destination, physical_package, object_name
    ):
        return {
            "assetId": source_id,
            "fullPath": f"{physical_package}|{object_name}",
            "status": "resumed",
        }
    staging_root = output_root.parent / ".staging" / "texture-extract"
    staging_root.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix=f"{source_id}.", dir=staging_root))
    export_root = work / "umodel"
    pack = work / "pack"
    try:
        command = [
            str(umodel),
            "-export",
            "-game=lostark",
            f"-{region}",
            "-nameresolve",
            "-dds",
            "-uncook",
            "-groups",
            f"-path={package_root}",
            f"-out={export_root}",
            f"-obj={object_name}",
            str(physical_path),
        ]
        try:
            completed = base.run(
                command, umodel.parent, timeout,
                f"UModel texture export {physical_package}|{object_name}",
            )
        except base.PipelineError as error:
            raise VariantError(str(error)) from error
        console = completed.stdout + "\n" + completed.stderr
        temporary_log = work / "umodel.log.txt"
        temporary_log.write_text(console, encoding="utf-8", newline="\n")
        rows = [
            row
            for row in parse_umodel_export_log(temporary_log)
            if row["class"] == "Texture2D"
            and row["objectName"].casefold() == object_name.casefold()
        ]
        identities = {row["objectPath"].casefold() for row in rows}
        if len(identities) != 1 or not rows:
            raise VariantError(
                f"UModel did not export one exact texture identity: "
                f"{physical_package}|{object_name}"
            )
        row = rows[0]
        source = export_root / safe_relative_path(row["relativePath"])
        if not source.is_file():
            raise VariantError(f"exact texture payload is missing: {source}")
        pack.mkdir(parents=True)
        (pack / "umodel.log.txt").write_text(
            console, encoding="utf-8", newline="\n"
        )
        target = pack / "textures" / source.name
        target.parent.mkdir(parents=True)
        shutil.copy2(source, target)
        outputs = [
            {
                "path": path.relative_to(pack).as_posix(),
                "sha256": sha256_file(path),
            }
            for path in sorted(path for path in pack.rglob("*") if path.is_file())
        ]
        receipt = {
            "schemaVersion": 1,
            "sourceId": source_id,
            "physicalPackage": physical_package,
            "objectName": object_name,
            "objectPath": row["objectPath"],
            "outputs": outputs,
        }
        base.atomic_write_json(pack / "source.receipt.json", receipt)
        try:
            base.commit_directory(pack, destination, output_root, force=True)
        except base.PipelineError as error:
            raise VariantError(str(error)) from error
        return {
            "assetId": source_id,
            "fullPath": row["objectPath"],
            "status": "exported",
        }
    finally:
        if work.exists():
            try:
                base.bounded_rmtree(work, output_root.parent)
            except base.PipelineError as error:
                raise VariantError(str(error)) from error


def missing_override_materials(catalog: dict[str, Any]) -> list[dict[str, str]]:
    rows = [
        {"objectPath": str(row["objectPath"]), "class": str(row["class"])}
        for row in catalog.get("gaps", [])
        if row.get("kind") == "missing-override-material"
    ]
    return sorted(rows, key=lambda row: row["objectPath"].casefold())


def exact_mesh_defaults(
    mesh_defaults: Sequence[dict[str, Any]], mesh_object_path: str
) -> list[dict[str, Any]]:
    rows = [
        row
        for row in mesh_defaults
        if str(row.get("meshObjectPath", "")).casefold()
        == mesh_object_path.casefold()
    ]
    if len(rows) != 1:
        raise VariantError(
            f"expected exactly one default material row for {mesh_object_path}, "
            f"found {len(rows)}"
        )
    slots = rows[0].get("slots")
    if not isinstance(slots, list) or not slots:
        raise VariantError(f"default material slots are missing: {mesh_object_path}")
    result: list[dict[str, Any]] = []
    for slot_index, slot in enumerate(slots):
        if not isinstance(slot, dict) or slot.get("slot", slot_index) != slot_index:
            raise VariantError(f"default material slots are not ordered: {mesh_object_path}")
        class_name = str(slot.get("class", ""))
        object_path = str(slot.get("objectPath", ""))
        if slot.get("class") is None and slot.get("objectPath") is None:
            result.append(
                {
                    "class": None,
                    "objectPath": None,
                    "sourceOnlyReason": str(
                        slot.get("sourceOnlyReason", "authored-null-default")
                    ),
                    "sourceMaterialName": slot.get("sourceMaterialName"),
                }
            )
            continue
        if class_name.casefold() not in MATERIAL_CLASSES or "." not in object_path:
            raise VariantError(
                f"default material is not a full exact identity: {mesh_object_path} slot {slot_index}"
            )
        result.append({"class": class_name, "objectPath": object_path})
    return result


def prepare_material_catalog(
    inventory: dict[str, Any], catalog: dict[str, Any]
) -> tuple[dict[str, list[dict[str, Any]]], dict[str, dict[str, Any]]]:
    material_candidates = catalog.get("materials")
    texture_candidates = catalog.get("textures")
    mesh_defaults = catalog.get("meshDefaults")
    if not all(isinstance(rows, list) for rows in (material_candidates, texture_candidates, mesh_defaults)):
        raise VariantError("material catalog requires materials/textures/meshDefaults arrays")
    defaults_by_mesh: dict[str, list[dict[str, Any]]] = {}
    required: dict[str, tuple[str, str]] = {}

    def require(object_path: str, class_name: str) -> None:
        key = object_path.casefold()
        previous = required.get(key)
        if previous is not None and previous[1].casefold() != class_name.casefold():
            raise VariantError(
                f"conflicting material classes for {object_path}: "
                f"{previous[1]} / {class_name}"
            )
        required.setdefault(key, (object_path, class_name))

    for mesh in inventory.get("sourceMeshes", []):
        full_path = str(mesh["fullPath"])
        defaults = exact_mesh_defaults(mesh_defaults, full_path)
        defaults_by_mesh[full_path.casefold()] = defaults
        for slot in defaults:
            if slot["objectPath"] is not None:
                require(str(slot["objectPath"]), str(slot["class"]))
    for asset in inventory.get("assets", []):
        validate_variant_asset(asset)
        for slot in asset.get("materialSlots", []):
            object_path = slot.get("objectPath")
            class_name = slot.get("class")
            if object_path is not None:
                require(str(object_path), str(class_name))
    contracts = resolve_material_contracts(
        list(required.values()), material_candidates, texture_candidates
    )
    return defaults_by_mesh, contracts


def _copy_gltf_buffers(source_gltf: Path, staged_gltf: Path) -> None:
    document = load_json(source_gltf)
    for buffer in document.get("buffers", []):
        if not isinstance(buffer, dict):
            raise VariantError("invalid glTF buffer row")
        relative = safe_relative_path(str(buffer.get("uri", "")))
        source = source_gltf.parent / relative
        destination = staged_gltf.parent / relative
        if not source.is_file():
            raise VariantError(f"glTF buffer is missing: {source}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)


def _copy_runtime_texture(
    texture: dict[str, Any],
    pack: Path,
    copied: dict[str, str],
) -> Path:
    source = Path(str(texture["source"]))
    expected_hash = str(texture["sha256"])
    if not source.is_file() or sha256_file(source) != expected_hash:
        raise VariantError(f"material texture changed after resolution: {source}")
    filename = f"{expected_hash[:12]}_{source.name}"
    key = filename.casefold()
    previous_hash = copied.get(key)
    if previous_hash is not None and previous_hash != expected_hash:
        raise VariantError(f"runtime texture filename collision: {filename}")
    destination = pack / "textures" / filename
    destination.parent.mkdir(parents=True, exist_ok=True)
    if not destination.exists():
        shutil.copy2(source, destination)
    copied[key] = expected_hash
    return destination


def _runtime_roles(contract: dict[str, Any]) -> dict[str, tuple[str, dict[str, Any]]]:
    parameters = {
        parameter: str(texture["objectPath"])
        for parameter, texture in contract["resolvedTextures"].items()
    }
    selected = base.select_supported_texture_parameters(parameters)
    return {
        switch: (parameter, contract["resolvedTextures"][parameter])
        for switch, (parameter, _object_path) in selected.items()
    }


def cook_variant(
    asset: dict[str, Any],
    *,
    source_root: Path,
    output_root: Path,
    converter: Path,
    defaults_by_mesh: dict[str, list[dict[str, Any]]],
    contracts: dict[str, dict[str, Any]],
    timeout: float,
    force: bool,
) -> dict[str, Any]:
    validate_variant_asset(asset)
    asset_id = str(asset["assetId"])
    source_asset_id = str(asset["sourceAssetId"])
    source_pack = source_root / source_asset_id
    source_receipt_path = source_pack / "source.receipt.json"
    source_receipt = load_json(source_receipt_path)
    if str(source_receipt.get("fullPath", "")).casefold() != str(asset["fullPath"]).casefold():
        raise VariantError(f"source mesh receipt mismatch: {asset_id}")
    source_gltf = source_pack / str(source_receipt.get("gltf", ""))
    if not source_gltf.is_file():
        raise VariantError(f"source mesh glTF is missing: {source_gltf}")
    if not converter.is_file():
        raise VariantError(f"ModelAssetConverter is missing: {converter}")

    staging_parent = output_root / ".staging" / "variant-cook"
    staging_parent.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix=f"{asset_id}.", dir=staging_parent))
    pack = work / "pack"
    destination = output_root / "runtime" / asset_id
    try:
        pack.mkdir(parents=True)
        staged_gltf = pack / source_gltf.name
        _copy_gltf_buffers(source_gltf, staged_gltf)
        slot_names = make_slot_unique_gltf(source_gltf, staged_gltf)
        defaults = defaults_by_mesh.get(str(asset["fullPath"]).casefold())
        if defaults is None or len(defaults) != len(slot_names):
            raise VariantError(
                f"default material/glTF slot count mismatch for {asset['fullPath']}: "
                f"{0 if defaults is None else len(defaults)} != {len(slot_names)}"
            )
        overrides = asset.get("materialSlots", [])
        if len(overrides) > len(defaults):
            raise VariantError(
                f"override array exceeds glTF material slots: {asset_id}"
            )

        effective = [dict(row) for row in defaults]
        for slot in overrides:
            slot_index = int(slot["slot"])
            if slot_index >= len(effective):
                raise VariantError(f"override slot is outside glTF materials: {asset_id}")
            if slot.get("objectPath") is None:
                effective[slot_index] = {
                    "class": None,
                    "objectPath": None,
                    "sourceOnlyReason": "authored-null-override",
                }
            else:
                effective[slot_index] = {
                    "class": str(slot["class"]),
                    "objectPath": str(slot["objectPath"]),
                }

        command_remaps: list[str] = []
        copied_textures: dict[str, str] = {}
        material_rows: list[dict[str, Any]] = []
        source_only_unsupported: list[dict[str, Any]] = []
        texture_dependency_closure_complete = True
        texture_slots_complete = True
        for slot_name, material in zip(slot_names, effective):
            if material.get("objectPath") is None:
                reason = str(
                    material.get("sourceOnlyReason", "authored-null-material")
                )
                texture_slots_complete = False
                source_only_unsupported.append(
                    {
                        "slot": slot_name["slot"],
                        "fields": [f"material:{reason}"],
                    }
                )
                material_rows.append(
                    {
                        **slot_name,
                        "class": None,
                        "objectPath": None,
                        "parentChain": [],
                        "roles": {},
                        "textureDependencies": [],
                        "sourceOnlyReason": reason,
                        "sourceMaterialName": material.get("sourceMaterialName"),
                    }
                )
                continue
            contract = contracts.get(material["objectPath"].casefold())
            if contract is None or contract["class"].casefold() != material["class"].casefold():
                raise VariantError(
                    f"resolved material contract is missing/mismatched: {material}"
                )
            roles = _runtime_roles(contract)
            has_visible_color = (
                "--material-remap" in roles or "--emissive-remap" in roles
            )
            texture_slots_complete = texture_slots_complete and has_visible_color
            role_receipt: dict[str, Any] = {}
            selected_parameters: set[str] = set()
            binding_by_parameter = {
                parameter: switch
                for switch, (parameter, _texture) in roles.items()
            }
            dependency_receipt: list[dict[str, Any]] = []
            dependency_paths: dict[str, Path] = {}
            for parameter, texture in sorted(contract["resolvedTextures"].items()):
                destination_texture = _copy_runtime_texture(
                    texture, pack, copied_textures
                )
                dependency_paths[parameter] = destination_texture
                dependency_receipt.append(
                    {
                        "kind": "parameter",
                        "parameter": parameter,
                        "runtimeBinding": binding_by_parameter.get(parameter),
                        "objectPath": texture["objectPath"],
                        "path": destination_texture.relative_to(pack).as_posix(),
                        "sha256": texture["sha256"],
                    }
                )
            for texture in contract.get("resolvedReferencedTextures", []):
                destination_texture = _copy_runtime_texture(
                    texture, pack, copied_textures
                )
                dependency_receipt.append(
                    {
                        "kind": "materialReferencedTexture",
                        "parameter": None,
                        "runtimeBinding": None,
                        "declaredObjectPath": texture["declaredObjectPath"],
                        "declaredByMaterial": texture["declaredByMaterial"],
                        "objectPath": texture["objectPath"],
                        "path": destination_texture.relative_to(pack).as_posix(),
                        "sha256": texture["sha256"],
                    }
                )
            for switch, (parameter, texture) in sorted(roles.items()):
                destination_texture = dependency_paths[parameter]
                command_remaps.extend(
                    [switch, f"{slot_name['runtimeName']}={destination_texture}"]
                )
                selected_parameters.add(parameter)
                role_receipt[switch] = {
                    "parameter": parameter,
                    "objectPath": texture["objectPath"],
                    "path": destination_texture.relative_to(pack).as_posix(),
                    "sha256": texture["sha256"],
                }
            unsupported = []
            if contract.get("sourceOnlyReason"):
                unsupported.append(
                    f"material:{contract['sourceOnlyReason']}"
                )
            unsupported.extend(
                f"texture:{parameter}"
                for parameter in sorted(
                    set(contract["resolvedTextures"]) - selected_parameters
                )
            )
            unsupported.extend(
                f"scalar:{parameter}" for parameter in sorted(contract["scalars"])
            )
            unsupported.extend(
                f"vector:{parameter}" for parameter in sorted(contract["vectors"])
            )
            unsupported.extend(
                f"renderFlag:{parameter}" for parameter in sorted(contract["flags"])
            )
            if unsupported:
                source_only_unsupported.append(
                    {"slot": slot_name["slot"], "fields": unsupported}
                )
            material_rows.append(
                {
                    **slot_name,
                    "class": material["class"],
                    "objectPath": material["objectPath"],
                    "parentChain": contract["parentChain"],
                    "roles": role_receipt,
                    "textureDependencies": dependency_receipt,
                    "sourceOnlyReason": contract.get("sourceOnlyReason"),
                }
            )

        model = pack / f"{asset_id}.wmodel"
        command = [
            str(converter),
            str(staged_gltf),
            "-o",
            str(model),
            "--pretransform",
            "--no-auto-textures",
            "--scale",
            "100",
            *command_remaps,
        ]
        try:
            completed = base.run(
                command, pack, timeout, f"ModelAssetConverter cook {asset_id}"
            )
        except base.PipelineError as error:
            raise VariantError(str(error)) from error
        if not model.is_file() or model.read_bytes()[:4] not in base.REQUIRED_WMODEL_MAGICS:
            raise VariantError(f"invalid cooked WModel: {model}")
        try:
            info = base.run(
                [str(converter), "info", str(model)],
                pack,
                timeout,
                f"ModelAssetConverter info {asset_id}",
            )
        except base.PipelineError as error:
            raise VariantError(str(error)) from error
        (pack / "converter.log.txt").write_text(
            completed.stdout + "\n" + completed.stderr,
            encoding="utf-8",
            newline="\n",
        )
        (pack / "converter.info.txt").write_text(
            info.stdout + "\n" + info.stderr,
            encoding="utf-8",
            newline="\n",
        )
        outputs = [
            {
                "path": path.relative_to(pack).as_posix(),
                "sha256": sha256_file(path),
            }
            for path in sorted(path for path in pack.rglob("*") if path.is_file())
        ]
        receipt = {
            "schemaVersion": 1,
            "assetId": asset_id,
            "sourceAssetId": source_asset_id,
            "fullPath": asset["fullPath"],
            "materialSignatureSha256": asset["materialSignatureSha256"],
            "model": f"{asset_id}/{asset_id}.wmodel",
            "sourceReceiptSha256": sha256_file(source_receipt_path),
            "materials": material_rows,
            "sourceOnlyUnsupported": source_only_unsupported,
            "runtimeCoverage": {
                "textureDependencyClosureComplete": (
                    texture_dependency_closure_complete
                ),
                "textureSlotsComplete": texture_slots_complete,
                "materialComplete": (
                    texture_slots_complete and not source_only_unsupported
                ),
            },
            "outputs": outputs,
        }
        base.atomic_write_json(pack / "runtime.receipt.json", receipt)
        try:
            base.commit_directory(pack, destination, output_root, force)
        except base.PipelineError as error:
            raise VariantError(str(error)) from error
        return receipt
    finally:
        if work.exists():
            try:
                base.bounded_rmtree(work, output_root)
            except base.PipelineError as error:
                raise VariantError(str(error)) from error


def build_runtime_manifest(
    inventory: dict[str, Any], output_root: Path, expect_variants: int
) -> dict[str, Any]:
    assets = inventory.get("assets")
    if not isinstance(assets, list) or len(assets) != expect_variants:
        raise VariantError(
            f"inventory variant count mismatch: {0 if not isinstance(assets, list) else len(assets)}"
        )
    rows: list[dict[str, Any]] = []
    for asset in assets:
        validate_variant_asset(asset)
        asset_id = str(asset["assetId"])
        pack = output_root / "runtime" / asset_id
        receipt = load_json(pack / "runtime.receipt.json")
        if (
            receipt.get("assetId") != asset_id
            or receipt.get("fullPath") != asset["fullPath"]
            or receipt.get("materialSignatureSha256")
            != asset["materialSignatureSha256"]
        ):
            raise VariantError(f"runtime receipt identity mismatch: {asset_id}")
        install_files: list[dict[str, str]] = []
        for output in receipt.get("outputs", []):
            relative_in_pack = safe_relative_path(str(output["path"]))
            if relative_in_pack.suffix.casefold() != ".wmodel" and (
                not relative_in_pack.parts
                or relative_in_pack.parts[0].casefold() != "textures"
            ):
                continue
            source = pack / relative_in_pack
            expected_hash = str(output["sha256"]).casefold()
            if not source.is_file() or sha256_file(source) != expected_hash:
                raise VariantError(f"runtime output drift: {source}")
            install_files.append(
                {
                    "source": (Path(asset_id) / relative_in_pack).as_posix(),
                    "path": (Path(asset_id) / relative_in_pack).as_posix(),
                    "sha256": expected_hash,
                }
            )
        model_relative = safe_relative_path(str(receipt["model"]))
        if not any(row["path"] == model_relative.as_posix() for row in install_files):
            raise VariantError(f"runtime model is not in install closure: {asset_id}")
        runtime_coverage = receipt.get("runtimeCoverage")
        source_only_unsupported = receipt.get("sourceOnlyUnsupported")
        validate_runtime_material_coverage(
            runtime_coverage, source_only_unsupported, asset_id
        )
        rows.append(
            {
                "assetId": asset_id,
                "sourceAssetId": asset["sourceAssetId"],
                "fullPath": asset["fullPath"],
                "materialSignatureSha256": asset["materialSignatureSha256"],
                "model": model_relative.as_posix(),
                "runtimeCoverage": runtime_coverage,
                "sourceOnlyUnsupported": source_only_unsupported,
                "files": sorted(install_files, key=lambda row: row["path"]),
            }
        )
    manifest = {
        "schemaVersion": 1,
        "areaId": inventory["areaId"],
        "assetCount": len(rows),
        "assets": rows,
    }
    base.atomic_write_json(
        output_root / "manifests" / "map_material_runtime_assets.json", manifest
    )
    return manifest


def _area_actual_files(area: Path) -> dict[str, str]:
    rows: dict[str, str] = {}
    for path in sorted(area.rglob("*")):
        if path.is_symlink():
            raise VariantError(f"runtime area contains a symlink: {path}")
        if not path.is_file() or path.name == INSTALL_RECEIPT_NAME:
            continue
        relative = path.relative_to(area).as_posix()
        rows[relative] = sha256_file(path)
    return rows


def validate_owned_area(
    area: Path, expected_area_id: str | None = None
) -> dict[str, Any]:
    receipt_path = area / INSTALL_RECEIPT_NAME
    if not receipt_path.is_file():
        raise VariantError(f"existing runtime area has no ownership receipt: {area}")
    receipt = load_json(receipt_path)
    area_id = area.name if expected_area_id is None else expected_area_id
    if receipt.get("schemaVersion") != 1 or receipt.get("areaId") != area_id:
        raise VariantError(f"runtime area ownership receipt identity mismatch: {area}")
    owned_rows = receipt.get("ownedFiles")
    if not isinstance(owned_rows, list):
        raise VariantError("runtime area receipt has no ownedFiles array")
    declared: dict[str, str] = {}
    for row in owned_rows:
        if not isinstance(row, dict):
            raise VariantError("invalid runtime area owned file row")
        relative = safe_relative_path(str(row.get("path", ""))).as_posix()
        file_hash = str(row.get("sha256", "")).casefold()
        if relative in declared or not re.fullmatch(r"[0-9a-f]{64}", file_hash):
            raise VariantError(f"duplicate/invalid owned file row: {relative}")
        declared[relative] = file_hash
    actual = _area_actual_files(area)
    if actual != declared:
        unknown = sorted(set(actual) - set(declared))
        missing = sorted(set(declared) - set(actual))
        changed = sorted(
            path
            for path in set(actual) & set(declared)
            if actual[path] != declared[path]
        )
        raise VariantError(
            "runtime area ownership CAS mismatch: "
            f"unknown={unknown}, missing={missing}, changed={changed}"
        )
    return receipt


def install_runtime_area(
    runtime_manifest_path: Path,
    runtime_root: Path,
    resources_root: Path,
    *,
    area_id: str = DEFAULT_AREA_ID,
    expect_variants: int = 292,
    allow_partial_material_preview: bool = False,
    fail_after: str | None = None,
) -> dict[str, Any]:
    area_id = area_token(area_id)
    manifest_hash = sha256_file(runtime_manifest_path)
    manifest = load_json(runtime_manifest_path)
    assets = manifest.get("assets")
    if (
        manifest.get("schemaVersion") != 1
        or manifest.get("areaId") != area_id
        or not isinstance(assets, list)
        or manifest.get("assetCount") != len(assets)
        or len(assets) != expect_variants
    ):
        raise VariantError("runtime manifest schema/area/count gate failed")
    texture_complete_count = 0
    texture_dependency_closure_complete_count = 0
    material_complete_count = 0
    unsupported_asset_count = 0
    for asset in assets:
        if not isinstance(asset, dict):
            raise VariantError("invalid runtime manifest asset row")
        coverage = asset.get("runtimeCoverage")
        unsupported = asset.get("sourceOnlyUnsupported")
        (
            texture_dependency_closure_complete,
            texture_slots_complete,
            material_complete,
        ) = validate_runtime_material_coverage(
            coverage, unsupported, str(asset.get("assetId", "<missing>"))
        )
        texture_dependency_closure_complete_count += int(
            texture_dependency_closure_complete
        )
        texture_complete_count += int(texture_slots_complete)
        material_complete_count += int(material_complete)
        unsupported_asset_count += int(bool(unsupported))
    partial_material = material_complete_count != len(assets)
    if partial_material and not allow_partial_material_preview:
        raise VariantError(
            "runtime material coverage is partial; "
            "--allow-partial-material-preview is required for geometry preview install"
        )
    if resources_root.name.casefold() != "resources":
        raise VariantError(f"resource root must end in Resources: {resources_root}")
    map_root = resources_root / "Map"
    map_root.mkdir(parents=True, exist_ok=True)
    destination = map_root / area_id
    # Keep transaction names shorter than the final area directory. Long
    # stable asset IDs otherwise cross legacy Win32 MAX_PATH only while staged.
    transaction = uuid.uuid4().hex[:12]
    stage = map_root / f".li-{transaction}"
    backup = map_root / f".lr-{transaction}"
    if stage.exists() or backup.exists():
        raise VariantError("install transaction path already exists")

    previous_receipt: dict[str, Any] | None = None
    previous_fingerprint: str | None = None
    if destination.exists():
        if not destination.is_dir() or destination.is_symlink():
            raise VariantError(f"runtime area destination is not a directory: {destination}")
        previous_receipt = validate_owned_area(destination)
        previous_fingerprint = sha256_file(destination / INSTALL_RECEIPT_NAME)

    promoted = False
    backed_up = False
    committed = False
    try:
        stage.mkdir()
        staged_files: dict[str, str] = {}
        seen_assets: set[str] = set()
        for asset in assets:
            if not isinstance(asset, dict):
                raise VariantError("invalid runtime manifest asset row")
            asset_id = str(asset.get("assetId", ""))
            if not asset_id or asset_id in seen_assets:
                raise VariantError(f"duplicate/empty runtime asset ID: {asset_id!r}")
            seen_assets.add(asset_id)
            files = asset.get("files")
            if not isinstance(files, list) or not files:
                raise VariantError(f"runtime asset has no install files: {asset_id}")
            for row in files:
                if not isinstance(row, dict):
                    raise VariantError(f"invalid install file row: {asset_id}")
                source_relative = safe_relative_path(str(row.get("source", "")))
                destination_relative = safe_relative_path(str(row.get("path", "")))
                expected_hash = str(row.get("sha256", "")).casefold()
                destination_key = destination_relative.as_posix()
                if destination_key in staged_files:
                    raise VariantError(f"duplicate install destination: {destination_key}")
                source = runtime_root / source_relative
                if (
                    not re.fullmatch(r"[0-9a-f]{64}", expected_hash)
                    or not source.is_file()
                    or source.is_symlink()
                    or sha256_file(source) != expected_hash
                ):
                    raise VariantError(f"runtime install source validation failed: {source}")
                target = stage / destination_relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target)
                if sha256_file(target) != expected_hash:
                    raise VariantError(f"staged runtime file hash mismatch: {target}")
                staged_files[destination_key] = expected_hash

        if sha256_file(runtime_manifest_path) != manifest_hash:
            raise VariantError("runtime manifest changed during install staging")
        receipt = {
            "schemaVersion": 1,
            "areaId": area_id,
            "runtimeManifestSha256": manifest_hash,
            "assetCount": len(assets),
            "admissionState": (
                "geometry-preview-partial-material"
                if partial_material
                else "runtime-material-complete"
            ),
            "runtimeCoverage": {
                "textureDependencyClosureComplete": (
                    texture_dependency_closure_complete_count
                ),
                "textureDependencyClosureIncomplete": (
                    len(assets) - texture_dependency_closure_complete_count
                ),
                "textureSlotsComplete": texture_complete_count,
                "textureSlotsIncomplete": len(assets) - texture_complete_count,
                "materialComplete": material_complete_count,
                "materialIncomplete": len(assets) - material_complete_count,
                "assetsWithSourceOnlyUnsupported": unsupported_asset_count,
            },
            "ownedFiles": [
                {"path": path, "sha256": file_hash}
                for path, file_hash in sorted(staged_files.items())
            ],
        }
        base.atomic_write_json(stage / INSTALL_RECEIPT_NAME, receipt)
        validate_owned_area(stage, area_id)

        if destination.exists():
            current = validate_owned_area(destination)
            if (
                current != previous_receipt
                or sha256_file(destination / INSTALL_RECEIPT_NAME)
                != previous_fingerprint
            ):
                raise VariantError("runtime area changed during install staging")
            os.replace(destination, backup)
            backed_up = True
            if fail_after == "backup":
                raise VariantError("injected install failure after backup")
        os.replace(stage, destination)
        promoted = True
        if fail_after == "promote":
            raise VariantError("injected install failure after promote")
        installed = validate_owned_area(destination)
        if installed != receipt:
            raise VariantError("installed runtime area receipt changed during promotion")
        committed = True
        if backup.exists():
            try:
                base.bounded_rmtree(backup, map_root)
            except base.PipelineError as error:
                raise VariantError(str(error)) from error
        return receipt
    except BaseException:
        rollback_errors: list[str] = []
        if not committed and promoted and destination.exists():
            try:
                base.bounded_rmtree(destination, map_root)
            except (base.PipelineError, OSError) as error:
                rollback_errors.append(f"remove promoted area: {error}")
        if not committed and backed_up and backup.exists():
            try:
                os.replace(backup, destination)
            except OSError as error:
                rollback_errors.append(f"restore previous area: {error}")
        if stage.exists():
            try:
                base.bounded_rmtree(stage, map_root)
            except (base.PipelineError, OSError) as error:
                rollback_errors.append(f"remove staging area: {error}")
        if rollback_errors:
            raise VariantError("install rollback failed: " + "; ".join(rollback_errors))
        raise


def _inventory_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--placements-dir", type=Path, action="append", required=True)
    parser.add_argument("--area-id", default=DEFAULT_AREA_ID)
    parser.add_argument("--level-prefix", default=DEFAULT_LEVEL_PREFIX)
    parser.add_argument("--expect-packages", type=int, default=6)
    parser.add_argument("--expect-source-meshes", type=int, default=164)
    parser.add_argument("--expect-variants", type=int, default=292)
    parser.add_argument("--expect-placements", type=int, default=2951)
    parser.add_argument("--expect-override-placements", type=int, default=1649)


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)

    inventory = commands.add_parser("inventory")
    _inventory_args(inventory)
    inventory.add_argument("--output", type=Path, required=True)
    inventory.add_argument("--dry-run", action="store_true")

    extract = commands.add_parser("extract")
    extract.add_argument("--inventory", type=Path, required=True)
    extract.add_argument("--output-root", type=Path, required=True)
    extract.add_argument("--umodel", type=Path, required=True)
    extract.add_argument("--package-root", type=Path, required=True)
    extract.add_argument("--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn"))
    extract.add_argument("--umodel-timeout", type=float, default=180.0)
    extract.add_argument("--workers", type=int, default=2)
    extract.add_argument("--force", action="store_true")

    catalog = commands.add_parser("catalog")
    catalog.add_argument("--inventory", type=Path, required=True)
    catalog.add_argument("--source-root", type=Path, required=True)
    catalog.add_argument("--output", type=Path, required=True)
    catalog.add_argument("--material-source-root", type=Path)
    catalog.add_argument("--texture-source-root", type=Path)
    catalog.add_argument("--allow-incomplete", action="store_true")

    hydrate = commands.add_parser("hydrate-catalog")
    hydrate.add_argument("--inventory", type=Path, required=True)
    hydrate.add_argument("--source-root", type=Path, required=True)
    hydrate.add_argument("--material-source-root", type=Path, required=True)
    hydrate.add_argument("--texture-source-root", type=Path, required=True)
    hydrate.add_argument("--output", type=Path, required=True)
    hydrate.add_argument("--umodel", type=Path, required=True)
    hydrate.add_argument("--package-root", type=Path, required=True)
    hydrate.add_argument("--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn"))
    hydrate.add_argument("--umodel-timeout", type=float, default=180.0)
    hydrate.add_argument("--workers", type=int, default=2)
    hydrate.add_argument("--force", action="store_true")
    hydrate.add_argument("--allow-incomplete", action="store_true")

    cook = commands.add_parser("cook")
    cook.add_argument("--inventory", type=Path, required=True)
    cook.add_argument("--source-root", type=Path, required=True)
    cook.add_argument("--material-catalog", type=Path, required=True)
    cook.add_argument("--output-root", type=Path, required=True)
    cook.add_argument("--converter", type=Path, required=True)
    cook.add_argument("--converter-timeout", type=float, default=180.0)
    cook.add_argument("--expect-variants", type=int, default=292)
    cook.add_argument("--asset-id", action="append")
    cook.add_argument("--force", action="store_true")

    install = commands.add_parser("install")
    install.add_argument("--runtime-manifest", type=Path, required=True)
    install.add_argument("--runtime-root", type=Path, required=True)
    install.add_argument("--resources-root", type=Path, required=True)
    install.add_argument("--area-id", default=DEFAULT_AREA_ID)
    install.add_argument("--expect-variants", type=int, default=292)
    install.add_argument("--allow-partial-material-preview", action="store_true")
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        if args.command == "inventory":
            inventory = build_inventory(
                args.placements_dir,
                area_id=args.area_id,
                level_prefix=args.level_prefix,
                expect_packages=args.expect_packages,
                expect_source_meshes=args.expect_source_meshes,
                expect_variants=args.expect_variants,
                expect_placements=args.expect_placements,
                expect_override_placements=args.expect_override_placements,
            )
            if not args.dry_run:
                base.atomic_write_json(args.output, inventory)
            print(json.dumps(inventory["summary"], ensure_ascii=False, indent=2))
            return 0

        if args.command == "extract":
            inventory = load_json(args.inventory)
            meshes = inventory.get("sourceMeshes")
            if (
                not isinstance(meshes, list)
                or inventory.get("sourceMeshCount") != len(meshes)
            ):
                raise VariantError("source mesh inventory count mismatch")
            if not 1 <= args.workers <= 8:
                raise VariantError("workers must be in [1, 8]")
            base.validate_tools(args.umodel, args.package_root)
            output_root = base.validate_output_root(args.output_root)
            try:
                base.parallel_assets(
                    "extract-source-mesh",
                    meshes,
                    args.workers,
                    lambda asset: base.export_one(
                        asset,
                        output_root,
                        args.umodel,
                        args.package_root,
                        args.region,
                        args.umodel_timeout,
                        args.force,
                    ),
                )
            except base.PipelineError as error:
                raise VariantError(str(error)) from error
            return 0

        if args.command == "catalog":
            inventory = load_json(args.inventory)
            catalog = build_observed_material_catalog(
                inventory,
                args.source_root,
                args.material_source_root,
                args.texture_source_root,
            )
            base.atomic_write_json(args.output, catalog)
            print(json.dumps(catalog["summary"], ensure_ascii=False, indent=2))
            if not catalog["admissionReady"] and not args.allow_incomplete:
                raise VariantError(
                    f"material catalog has {catalog['summary']['gapCount']} gaps"
                )
            return 0

        if args.command == "hydrate-catalog":
            if not 1 <= args.workers <= 8:
                raise VariantError("workers must be in [1, 8]")
            base.validate_tools(args.umodel, args.package_root)
            inventory = load_json(args.inventory)
            initial = build_observed_material_catalog(
                inventory,
                args.source_root,
                args.material_source_root
                if args.material_source_root.is_dir()
                else None,
                args.texture_source_root
                if args.texture_source_root.is_dir()
                else None,
            )
            missing = missing_override_materials(initial)
            args.material_source_root.mkdir(parents=True, exist_ok=True)
            if missing:
                try:
                    base.parallel_assets(
                        "extract-material",
                        [
                            {
                                "assetId": _material_source_id(row["objectPath"]),
                                "fullPath": row["objectPath"],
                                **row,
                            }
                            for row in missing
                        ],
                        args.workers,
                        lambda material: export_exact_material_source(
                            material,
                            output_root=args.material_source_root,
                            umodel=args.umodel,
                            package_root=args.package_root,
                            region=args.region,
                            timeout=args.umodel_timeout,
                            force=args.force,
                        ),
                    )
                except base.PipelineError as error:
                    if not args.allow_incomplete:
                        raise VariantError(str(error)) from error
                    print(
                        f"WARNING: exact material hydration remained incomplete: {error}",
                        file=sys.stderr,
                    )
            missing_textures = missing_loaded_texture_sources(
                [args.source_root, args.material_source_root]
            )
            args.texture_source_root.mkdir(parents=True, exist_ok=True)
            if missing_textures:
                try:
                    base.parallel_assets(
                        "extract-texture",
                        [
                            {
                                "assetId": _texture_source_id(
                                    row["physicalPackage"], row["objectName"]
                                ),
                                "fullPath": (
                                    f"{row['physicalPackage']}|{row['objectName']}"
                                ),
                                **row,
                            }
                            for row in missing_textures
                        ],
                        args.workers,
                        lambda texture: export_exact_texture_source(
                            texture,
                            output_root=args.texture_source_root,
                            umodel=args.umodel,
                            package_root=args.package_root,
                            region=args.region,
                            timeout=args.umodel_timeout,
                            force=args.force,
                        ),
                    )
                except base.PipelineError as error:
                    if not args.allow_incomplete:
                        raise VariantError(str(error)) from error
                    print(
                        f"WARNING: exact texture hydration remained incomplete: {error}",
                        file=sys.stderr,
                    )
            catalog = build_observed_material_catalog(
                inventory,
                args.source_root,
                args.material_source_root,
                args.texture_source_root,
            )
            base.atomic_write_json(args.output, catalog)
            print(json.dumps(catalog["summary"], ensure_ascii=False, indent=2))
            if not catalog["admissionReady"] and not args.allow_incomplete:
                raise VariantError(
                    f"material catalog has {catalog['summary']['gapCount']} gaps"
                )
            return 0

        if args.command == "cook":
            inventory = load_json(args.inventory)
            if inventory.get("assetCount") != args.expect_variants:
                raise VariantError("inventory variant count gate failed")
            catalog = load_json(args.material_catalog)
            defaults, contracts = prepare_material_catalog(inventory, catalog)
            output_root = base.validate_output_root(args.output_root)
            assets = inventory.get("assets")
            if not isinstance(assets, list):
                raise VariantError("inventory assets array is missing")
            if args.asset_id:
                try:
                    selected = base.select_assets(assets, args.asset_id)
                except base.PipelineError as error:
                    raise VariantError(str(error)) from error
            else:
                selected = assets
            for index, asset in enumerate(selected, 1):
                receipt = cook_variant(
                    asset,
                    source_root=args.source_root,
                    output_root=output_root,
                    converter=args.converter,
                    defaults_by_mesh=defaults,
                    contracts=contracts,
                    timeout=args.converter_timeout,
                    force=args.force,
                )
                print(
                    json.dumps(
                        {
                            "phase": "cook-variant",
                            "done": index,
                            "total": len(selected),
                            "assetId": receipt["assetId"],
                            "textureSlotsComplete": receipt["runtimeCoverage"][
                                "textureSlotsComplete"
                            ],
                        },
                        ensure_ascii=False,
                    ),
                    flush=True,
                )
            if len(selected) == len(assets):
                build_runtime_manifest(inventory, output_root, args.expect_variants)
            return 0

        receipt = install_runtime_area(
            args.runtime_manifest,
            args.runtime_root,
            args.resources_root,
            area_id=args.area_id,
            expect_variants=args.expect_variants,
            allow_partial_material_preview=args.allow_partial_material_preview,
        )
        print(json.dumps(receipt, ensure_ascii=False, indent=2))
        return 0
    except (VariantError, base.PipelineError, OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
