#!/usr/bin/env python3
"""Close exact UE3 effect Material/MIC identities for enabled occurrences.

The source receipt may contain ambiguous basename matches because the same
object path can exist in several physical packages.  Cascade imports still
name the logical package, so this extractor joins that logical package through
the package inventory, parses the selected UPK directly, and records the exact
Material Instance parameters and surviving parent Material graph.  A cooked
partial graph is evidence, not runtime-exact shader admission.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from extract_ue3_material_graph import extract_material_contract
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    decompress_package,
    package_ref_name,
    package_ref_path,
    parse_export_table,
    parse_import_table,
    parse_name_table,
    parse_summary,
    parse_tagged_properties,
)


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"expected JSON object: {path}")
    return value


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def folded(value: Any) -> str:
    return str(value or "").casefold()


def tagged_value(properties: dict[str, Any], name: str) -> Any:
    wanted = name.casefold()
    for key, item in properties.items():
        if key.casefold() == wanted:
            return item.get("value") if isinstance(item, dict) else item
    return None


def struct_linear_color(value: Any) -> list[float] | None:
    if not isinstance(value, dict) or folded(value.get("structType")) != "linearcolor":
        return None
    payload = value.get("value")
    if not isinstance(payload, dict):
        return None
    encoded = payload.get("hex")
    if not isinstance(encoded, str) or len(encoded) != 32:
        return None
    return list(struct.unpack("<4f", bytes.fromhex(encoded)))


def parameter_field(row: dict[str, Any], name: str) -> Any:
    value = row.get(name)
    return value.get("value") if isinstance(value, dict) else None


@dataclass(frozen=True)
class PackageData:
    path: Path
    sha256: str
    summary: Any
    logical: bytes
    names: list[str]
    imports: list[Any]
    exports: list[Any]


def load_package(path: Path, aes_key: str) -> PackageData:
    physical = path.read_bytes()
    summary = parse_summary(physical)
    logical = decompress_package(physical, summary, aes_key)
    names = parse_name_table(logical, summary)
    imports = parse_import_table(logical, summary, names)
    exports = parse_export_table(logical, summary, names)
    return PackageData(
        path=path,
        sha256=hashlib.sha256(physical).hexdigest(),
        summary=summary,
        logical=logical,
        names=names,
        imports=imports,
        exports=exports,
    )


def find_export(package: PackageData, relative_path: str) -> Any:
    wanted = relative_path.casefold()
    for entry in package.exports:
        if package_ref_path(
            entry.index + 1, package.imports, package.exports
        ).casefold() == wanted:
            return entry
    raise ValueError(
        f"object is missing from {package.path.name}: {relative_path}"
    )


def decode_material_instance(package: PackageData, relative_path: str) -> dict[str, Any]:
    entry = find_export(package, relative_path)
    class_name = package_ref_name(
        entry.class_index, package.imports, package.exports
    ) or ""
    if class_name.casefold() != "materialinstanceconstant":
        raise ValueError(f"source object is not a MaterialInstanceConstant: {relative_path}")
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    properties, property_end = parse_tagged_properties(
        serial, package.names, package.summary.version
    )

    scalar_parameters = []
    for row in tagged_value(properties, "scalarparametervalues") or []:
        if not isinstance(row, dict):
            continue
        name = parameter_field(row, "parametername")
        value = parameter_field(row, "parametervalue")
        if isinstance(name, str) and isinstance(value, (int, float)):
            scalar_parameters.append({"name": name, "value": float(value)})

    texture_parameters = []
    for row in tagged_value(properties, "textureparametervalues") or []:
        if not isinstance(row, dict):
            continue
        name = parameter_field(row, "parametername")
        reference = parameter_field(row, "parametervalue")
        if isinstance(name, str) and isinstance(reference, int):
            texture_parameters.append({
                "name": name,
                "sourceObjectPath": package_ref_path(
                    reference, package.imports, package.exports
                ) if reference else None,
                "packageIndex": reference,
            })

    vector_parameters = []
    for row in tagged_value(properties, "vectorparametervalues") or []:
        if not isinstance(row, dict):
            continue
        name = parameter_field(row, "parametername")
        value = struct_linear_color(row.get("parametervalue"))
        if isinstance(name, str) and value is not None:
            vector_parameters.append({"name": name, "value": value})

    parent_reference = tagged_value(properties, "parent")
    parent_path = (
        package_ref_path(parent_reference, package.imports, package.exports)
        if isinstance(parent_reference, int) and parent_reference
        else None
    )
    object_path = package_ref_path(
        entry.index + 1, package.imports, package.exports
    )
    return {
        "objectPath": object_path,
        "className": class_name,
        "exportIndex": entry.index,
        "serialSize": entry.serial_size,
        "propertyStreamEnd": property_end,
        "trailingByteCount": len(serial) - property_end,
        "parent": parent_path,
        "scalarParameters": scalar_parameters,
        "textureParameters": texture_parameters,
        "vectorParameters": vector_parameters,
        "overrideTwoSided": tagged_value(properties, "overridedtwosided"),
        "hasStaticPermutationResource": tagged_value(
            properties, "bhasstaticpermutationresource"
        ),
    }


def load_inventory(path: Path) -> dict[str, str]:
    import csv

    with path.open("r", encoding="utf-8-sig", newline="") as source:
        rows = list(csv.DictReader(source))
    result: dict[str, str] = {}
    for row in rows:
        logical = str(row.get("logical_name") or "").casefold()
        physical = str(row.get("physical_file") or "")
        if not logical or not physical:
            continue
        if logical in result and result[logical].casefold() != physical.casefold():
            raise ValueError(f"duplicate physical package mapping: {logical}")
        result[logical] = physical
    return result


def enabled_system_ids(action_recipe: dict[str, Any]) -> set[str]:
    result: set[str] = set()
    for cue in action_recipe.get("cues", []):
        if folded(cue.get("sourceType")) != "playparticleeffect":
            continue
        if not bool(cue.get("executionEnabled")):
            continue
        references = cue.get("assetReferences", [])
        if references and references[0].get("objectPath"):
            result.add(folded(references[0]["objectPath"]))
    return result


def enabled_material_paths(
    conversion_receipt: dict[str, Any], active_systems: set[str]
) -> list[str]:
    result: set[str] = set()
    for row in conversion_receipt.get("elementConversions", []):
        if folded(row.get("sourceSystemId")) not in active_systems:
            continue
        for evidence in row.get("materialParameterEvidence", []):
            path = str(evidence.get("sourceMaterialPath") or "")
            if path:
                result.add(path)
    return sorted(result, key=str.casefold)


def enabled_material_renderer_shapes(
    conversion_receipt: dict[str, Any], active_systems: set[str]
) -> dict[str, list[str]]:
    result: dict[str, set[str]] = {}
    for row in conversion_receipt.get("elementConversions", []):
        if folded(row.get("sourceSystemId")) not in active_systems:
            continue
        renderer_shape = folded(row.get("rendererShape")) or "unknown"
        for evidence in row.get("materialParameterEvidence", []):
            path = str(evidence.get("sourceMaterialPath") or "")
            if path:
                result.setdefault(path.casefold(), set()).add(renderer_shape)
    return {
        key: sorted(values)
        for key, values in sorted(result.items())
    }


def source_material_bindings(
    source_receipt: dict[str, Any]
) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in source_receipt.get("materialParameterBindings", []):
        if not isinstance(row, dict):
            continue
        source_path = folded(row.get("sourceMaterialPath"))
        if source_path:
            result[source_path] = row
    return result


def relative_object_path(source_path: str) -> tuple[str, str]:
    parts = source_path.split(".")
    if len(parts) < 2:
        raise ValueError(f"material path has no logical package: {source_path}")
    return parts[0], ".".join(parts[1:])


def build_closure(
    source_receipt_path: Path,
    conversion_receipt_path: Path,
    action_recipe_path: Path,
    inventory_path: Path,
    package_root: Path,
    aes_key: str,
) -> dict[str, Any]:
    source_receipt = read_json(source_receipt_path)
    conversion_receipt = read_json(conversion_receipt_path)
    action_recipe = read_json(action_recipe_path)
    inventory = load_inventory(inventory_path)
    active_systems = enabled_system_ids(action_recipe)
    material_paths = enabled_material_paths(conversion_receipt, active_systems)
    renderer_shapes = enabled_material_renderer_shapes(
        conversion_receipt, active_systems
    )
    receipt_bindings = source_material_bindings(source_receipt)
    package_cache: dict[str, PackageData] = {}
    parent_graph_cache: dict[str, dict[str, Any]] = {}
    rows = []

    for source_path in material_paths:
        logical_package, relative_path = relative_object_path(source_path)
        physical_name = inventory.get(logical_package.casefold())
        resolution_evidence = "LOGICAL_PACKAGE_INVENTORY"
        receipt_binding = receipt_bindings.get(source_path.casefold())
        if physical_name is None and receipt_binding is not None:
            if folded(receipt_binding.get("resolutionStatus")) == \
                    "resolved_unique_path":
                physical_name = str(
                    receipt_binding.get("sourcePhysicalPackage") or ""
                ) or None
                relative_path = str(
                    receipt_binding.get("materialPath") or relative_path
                )
                resolution_evidence = "SOURCE_RECEIPT_UNIQUE_PATH"
        if physical_name is None:
            shapes = renderer_shapes.get(source_path.casefold(), [])
            if (source_path.casefold() == "enginematerials.defaultparticle"
                    and shapes == ["light"]):
                rows.append({
                    "sourceMaterialPath": source_path,
                    "sourceLogicalPackage": logical_package,
                    "rendererShapes": shapes,
                    "status": "SOURCE_BUILTIN_NON_RENDER_MATERIAL",
                    "runtimeExactEligible": True,
                })
                continue
            rows.append({
                "sourceMaterialPath": source_path,
                "sourceLogicalPackage": logical_package,
                "rendererShapes": shapes,
                "status": "MISSING_LOGICAL_PACKAGE_MAPPING",
            })
            continue
        package_path = package_root / physical_name
        package = package_cache.get(physical_name.casefold())
        if package is None:
            package = load_package(package_path, aes_key)
            package_cache[physical_name.casefold()] = package
        try:
            entry = find_export(package, relative_path)
            class_name = package_ref_name(
                entry.class_index, package.imports, package.exports
            ) or ""
            if class_name.casefold() == "materialinstanceconstant":
                material = decode_material_instance(package, relative_path)
            elif class_name.casefold() == "material":
                material = {
                    "objectPath": relative_path,
                    "className": class_name,
                    "parent": None,
                    "scalarParameters": [],
                    "textureParameters": [],
                    "vectorParameters": [],
                }
            else:
                raise ValueError(f"source object has invalid class: {class_name}")
        except ValueError as error:
            rows.append({
                "sourceMaterialPath": source_path,
                "sourceLogicalPackage": logical_package,
                "sourcePhysicalPackage": physical_name,
                "sourcePhysicalPackageSha256": package.sha256,
                "status": "MISSING_EXACT_OBJECT",
                "error": str(error),
            })
            continue

        material_graph = None
        if material["className"].casefold() == "material":
            cache_key = f"{physical_name.casefold()}::{relative_path.casefold()}"
            if cache_key not in parent_graph_cache:
                try:
                    parent_graph_cache[cache_key] = extract_material_contract(
                        package_path, relative_path, aes_key
                    )
                except ValueError as error:
                    parent_graph_cache[cache_key] = {
                        "materialPath": relative_path,
                        "summary": {
                            "topologyStatus": "MATERIAL_GRAPH_UNRESOLVED",
                            "runtimeExactEligible": False,
                        },
                        "error": str(error),
                    }
            material_graph = {
                "physicalPackage": physical_name,
                "physicalPackageSha256": package.sha256,
                "resolutionEvidence": resolution_evidence,
                "graph": parent_graph_cache[cache_key],
            }

        parent_graph = None
        parent = material.get("parent")
        if parent:
            parent_logical, parent_relative = relative_object_path(str(parent))
            parent_physical = None
            parent_resolution = None
            try:
                parent_entry = find_export(package, str(parent))
                parent_class = package_ref_name(
                    parent_entry.class_index, package.imports, package.exports
                ) or ""
                if parent_class.casefold() == "material":
                    parent_physical = physical_name
                    parent_relative = str(parent)
                    parent_resolution = "SAME_PHYSICAL_PACKAGE_EXPORT"
            except ValueError:
                pass
            if parent_physical is None:
                parent_physical = inventory.get(parent_logical.casefold())
                parent_resolution = "LOGICAL_PACKAGE_INVENTORY"
            if parent_physical:
                cache_key = f"{parent_physical.casefold()}::{parent_relative.casefold()}"
                if cache_key not in parent_graph_cache:
                    try:
                        parent_graph_cache[cache_key] = extract_material_contract(
                            package_root / parent_physical,
                            parent_relative,
                            aes_key,
                        )
                    except ValueError as error:
                        parent_graph_cache[cache_key] = {
                            "materialPath": parent_relative,
                            "summary": {
                                "topologyStatus": "PARENT_GRAPH_UNRESOLVED",
                                "runtimeExactEligible": False,
                            },
                            "error": str(error),
                        }
                parent_graph = {
                    "logicalPackage": parent_logical,
                    "physicalPackage": parent_physical,
                    "resolutionEvidence": parent_resolution,
                    "physicalPackageSha256": sha256_file(
                        package_root / parent_physical
                    ),
                    "graph": parent_graph_cache[cache_key],
                }

        shader_graph = material_graph or parent_graph
        topology = (
            shader_graph["graph"].get("summary", {}).get("topologyStatus")
            if shader_graph else "PARENT_MATERIAL_MISSING"
        )
        rows.append({
            "sourceMaterialPath": source_path,
            "sourceLogicalPackage": logical_package,
            "rendererShapes": renderer_shapes.get(source_path.casefold(), []),
            "resolutionEvidence": resolution_evidence,
            "sourcePhysicalPackage": physical_name,
            "sourcePhysicalPackageSha256": package.sha256,
            "material": material,
            "materialGraph": material_graph,
            "parentGraph": parent_graph,
            "status": (
                "SOURCE_IDENTITY_CLOSED_SHADER_TOPOLOGY_PENDING"
                if topology != "SURVIVING_GRAPH_CAPTURED"
                else "SOURCE_IDENTITY_AND_SURVIVING_GRAPH_CAPTURED"
            ),
        })

    status_counts: dict[str, int] = {}
    for row in rows:
        status = str(row["status"])
        status_counts[status] = status_counts.get(status, 0) + 1
    return {
        "schema": "lostark.ue3-effect-material-closure",
        "formatVersion": 1,
        "characterClass": source_receipt.get("characterClass"),
        "skillId": source_receipt.get("skillId"),
        "inputSlot": source_receipt.get("inputSlot"),
        "source": {
            "sourceReceiptSha256": sha256_file(source_receipt_path),
            "conversionReceiptSha256": sha256_file(conversion_receipt_path),
            "actionCueRecipeSha256": sha256_file(action_recipe_path),
            "packageInventorySha256": sha256_file(inventory_path),
        },
        "activeSourceSystemIds": sorted(active_systems),
        "materials": rows,
        "summary": {
            "activeSourceSystemCount": len(active_systems),
            "materialCount": len(rows),
            "sourceIdentityClosedCount": sum(
                row["status"].startswith("SOURCE_IDENTITY")
                or row["status"] == "SOURCE_BUILTIN_NON_RENDER_MATERIAL"
                for row in rows
            ),
            "runtimeExactEligibleCount": sum(
                row["status"] ==
                "SOURCE_IDENTITY_AND_SURVIVING_GRAPH_CAPTURED"
                or bool(row.get("runtimeExactEligible"))
                for row in rows
            ),
            "statusCounts": dict(sorted(status_counts.items())),
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-receipt", required=True, type=Path)
    parser.add_argument("--conversion-receipt", required=True, type=Path)
    parser.add_argument("--action-cue-recipe", required=True, type=Path)
    parser.add_argument("--package-inventory", required=True, type=Path)
    parser.add_argument("--package-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    args = parser.parse_args()
    result = build_closure(
        args.source_receipt,
        args.conversion_receipt,
        args.action_cue_recipe,
        args.package_inventory,
        args.package_root,
        args.aes_key,
    )
    write_json_atomic(args.output, result)
    print(json.dumps(result["summary"], ensure_ascii=False, sort_keys=True))
    return 0 if result["summary"]["sourceIdentityClosedCount"] == result[
        "summary"
    ]["materialCount"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
