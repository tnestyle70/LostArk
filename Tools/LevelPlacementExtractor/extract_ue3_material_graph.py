#!/usr/bin/env python3
"""Extract the surviving UE3 cooked Material graph contract from a package."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

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


MATERIAL_OUTPUTS = (
    "diffusecolor",
    "emissivecolor",
    "opacity",
    "opacitymask",
    "distortion",
    "normal",
    "worldpositionoffset",
)

EFFECT_MATERIAL_CLASSES = frozenset(("material", "decalmaterial"))


def folded(value: Any) -> str:
    return str(value or "").casefold()


def is_effect_material_class(value: Any) -> bool:
    return folded(value) in EFFECT_MATERIAL_CLASSES


def property_value(properties: dict[str, Any], name: str) -> Any:
    wanted = folded(name)
    for key, item in properties.items():
        if folded(key) != wanted:
            continue
        return item.get("value") if isinstance(item, dict) else item
    return None


def expression_input(value: Any) -> dict[str, Any] | None:
    if not isinstance(value, dict):
        return None
    properties = value.get("properties")
    if not isinstance(properties, dict):
        return None
    expression = property_value(properties, "expression")
    if not isinstance(expression, int):
        return None
    result: dict[str, Any] = {"packageIndex": expression}
    for name in ("outputindex", "mask", "maskr", "maskg", "maskb", "maska"):
        candidate = property_value(properties, name)
        if isinstance(candidate, (bool, int, float)):
            result[name] = candidate
    return result


def expression_inputs(properties: dict[str, Any]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for name, item in properties.items():
        if not isinstance(item, dict):
            continue
        decoded = expression_input(item.get("value"))
        if decoded is not None:
            rows.append({"input": str(name), **decoded})
    return rows


def decode_parameter_value(properties: dict[str, Any]) -> Any:
    for name in ("defaultvalue", "defaultValue"):
        value = property_value(properties, name)
        if value is not None:
            return value
    return None


def classify_topology(
    expression_count: int,
    non_null_count: int,
    outputs: dict[str, dict[str, Any]],
    unresolved_edge_count: int,
) -> str:
    if non_null_count < expression_count:
        return "COOKED_PARTIAL"
    if unresolved_edge_count:
        return "COOKED_PARTIAL"
    if not outputs or all(row["packageIndex"] == 0 for row in outputs.values()):
        return "COOKED_PARTIAL"
    return "SURVIVING_GRAPH_CAPTURED"


def extract_material_contract(
    package_path: Path,
    material_path: str,
    aes_key: str = LOSTARK_KR_AES_KEY,
) -> dict[str, Any]:
    physical = package_path.read_bytes()
    summary = parse_summary(physical)
    logical = decompress_package(physical, summary, aes_key)
    names = parse_name_table(logical, summary)
    imports = parse_import_table(logical, summary, names)
    exports = parse_export_table(logical, summary, names)

    wanted = folded(material_path)
    material = None
    for entry in exports:
        path = package_ref_path(entry.index + 1, imports, exports)
        if folded(path) == wanted:
            material = entry
            break
    if material is None:
        raise ValueError(f"Material export is missing: {material_path}")
    material_class = package_ref_name(
        material.class_index, imports, exports
    ) or ""
    if not is_effect_material_class(material_class):
        raise ValueError(
            f"Source object is not an effect Material: {material_path}"
        )

    serial = logical[
        material.serial_offset : material.serial_offset + material.serial_size
    ]
    properties, property_end = parse_tagged_properties(
        serial, names, summary.version
    )
    raw_expression_refs = property_value(properties, "expressions")
    if not isinstance(raw_expression_refs, list):
        raise ValueError(f"Material Expressions array is missing: {material_path}")

    outputs: dict[str, dict[str, Any]] = {}
    for output_name in MATERIAL_OUTPUTS:
        decoded = expression_input(property_value(properties, output_name))
        if decoded is None:
            continue
        package_index = int(decoded["packageIndex"])
        decoded["objectPath"] = (
            package_ref_path(package_index, imports, exports)
            if package_index else None
        )
        outputs[output_name] = decoded

    expressions: list[dict[str, Any]] = []
    unresolved_edge_count = 0
    for source_order, package_index in enumerate(raw_expression_refs):
        if not isinstance(package_index, int) or package_index == 0:
            continue
        if package_index < 1 or package_index > len(exports):
            raise ValueError(
                f"Material expression reference is invalid: {package_index}"
            )
        entry = exports[package_index - 1]
        class_name = package_ref_name(entry.class_index, imports, exports) or ""
        if not folded(class_name).startswith("materialexpression"):
            raise ValueError(
                f"Material expression target has invalid class: {class_name}"
            )
        expression_serial = logical[
            entry.serial_offset : entry.serial_offset + entry.serial_size
        ]
        expression_properties, expression_end = parse_tagged_properties(
            expression_serial, names, summary.version
        )
        inputs = expression_inputs(expression_properties)
        for row in inputs:
            reference = int(row["packageIndex"])
            row["objectPath"] = (
                package_ref_path(reference, imports, exports)
                if reference else None
            )
            if reference == 0:
                unresolved_edge_count += 1
        texture_reference = property_value(expression_properties, "texture")
        expressions.append({
            "sourceOrder": source_order,
            "exportIndex": entry.index,
            "className": class_name,
            "objectPath": package_ref_path(
                entry.index + 1, imports, exports
            ),
            "parameterName": property_value(
                expression_properties, "parametername"
            ),
            "group": property_value(expression_properties, "group"),
            "defaultValue": decode_parameter_value(expression_properties),
            "textureObjectPath": (
                package_ref_path(texture_reference, imports, exports)
                if isinstance(texture_reference, int) and texture_reference
                else None
            ),
            "inputs": inputs,
            "serialSize": entry.serial_size,
            "propertyStreamEnd": expression_end,
        })

    topology_status = classify_topology(
        len(raw_expression_refs), len(expressions), outputs,
        unresolved_edge_count,
    )
    named_textures = [
        {
            "name": row["parameterName"],
            "group": row["group"] or "",
            "sourceObjectPath": row["textureObjectPath"],
            "expressionObjectPath": row["objectPath"],
        }
        for row in expressions
        if row["parameterName"] and row["textureObjectPath"]
    ]
    return {
        "schema": "lostark.ue3-cooked-material-graph-evidence",
        "formatVersion": 1,
        "sourcePackage": package_path.name,
        "sourcePackageVersion": summary.version,
        "materialClassName": material_class,
        "materialPath": package_ref_path(
            material.index + 1, imports, exports
        ),
        "materialExportIndex": material.index,
        "materialSerialSize": material.serial_size,
        "materialPropertyStreamEnd": property_end,
        "materialTrailingByteCount": len(serial) - property_end,
        "outputs": outputs,
        "expressions": expressions,
        "namedTextures": named_textures,
        "summary": {
            "expressionEntryCount": len(raw_expression_refs),
            "nonNullExpressionCount": len(expressions),
            "nullExpressionCount": len(raw_expression_refs) - len(expressions),
            "namedTextureCount": len(named_textures),
            "unresolvedInputEdgeCount": unresolved_edge_count,
            "topologyStatus": topology_status,
            "runtimeExactEligible": topology_status == "SURVIVING_GRAPH_CAPTURED",
        },
    }


def write_json_atomic(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    staged = path.with_suffix(path.suffix + ".tmp")
    staged.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    staged.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package", required=True, type=Path)
    parser.add_argument("--material", required=True, action="append")
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    args = parser.parse_args()
    documents = [
        extract_material_contract(args.package, path, args.aes_key)
        for path in args.material
    ]
    write_json_atomic(args.output, {
        "schema": "lostark.ue3-cooked-material-graph-evidence-set",
        "formatVersion": 1,
        "materials": documents,
    })
    print(json.dumps({
        "materialCount": len(documents),
        "runtimeExactEligibleCount": sum(
            row["summary"]["runtimeExactEligible"] for row in documents
        ),
        "output": str(args.output),
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
