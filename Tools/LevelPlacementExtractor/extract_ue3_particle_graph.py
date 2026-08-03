#!/usr/bin/env python3
"""Decode Lost Ark UE3 ParticleSystem graphs and their tagged properties."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

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
    resolve_physical_package,
)


def is_particle_graph_class(class_name: str) -> bool:
    folded = class_name.casefold()
    return (
        folded == "particlesystem"
        or folded == "particlelodlevel"
        or (folded.startswith("particle") and folded.endswith("emitter"))
        or folded.startswith("particlemodule")
        or folded.startswith("efparticlemodule")
        or folded.startswith("distributionfloat")
        or folded.startswith("distributionvector")
    )


def resolve_reference(value: int, imports: list[object], exports: list[object]) -> dict:
    return {
        "packageIndex": value,
        "objectName": package_ref_name(value, imports, exports),
        "objectPath": package_ref_path(value, imports, exports),
    }


def property_references(
    properties: dict, imports: list[object], exports: list[object]
) -> list[dict]:
    rows = []
    for name, item in properties.items():
        kind = str(item.get("type", "")).casefold()
        value = item.get("value")
        values: list[int] = []
        if kind in {"objectproperty", "componentproperty", "interfaceproperty"}:
            if isinstance(value, int):
                values.append(value)
        elif kind == "arrayproperty" and isinstance(value, list):
            values.extend(entry for entry in value if isinstance(entry, int))
        for package_index in values:
            if package_index == 0:
                continue
            reference = resolve_reference(package_index, imports, exports)
            if reference["objectPath"] is not None:
                rows.append({"property": name, **reference})
    return rows


def extract_package(package_path: Path, logical_name: str, aes_key: str) -> dict:
    physical = package_path.read_bytes()
    summary = parse_summary(physical)
    logical = decompress_package(physical, summary, aes_key)
    names = parse_name_table(logical, summary)
    imports = parse_import_table(logical, summary, names)
    exports = parse_export_table(logical, summary, names)

    objects = []
    errors = []
    for entry in exports:
        class_name = package_ref_name(entry.class_index, imports, exports)
        if not is_particle_graph_class(class_name):
            continue
        serial_data = logical[entry.serial_offset : entry.serial_offset + entry.serial_size]
        try:
            properties, property_end = parse_tagged_properties(
                serial_data, names, summary.version
            )
            references = property_references(properties, imports, exports)
        except Exception as error:
            errors.append(
                {
                    "exportIndex": entry.index,
                    "className": class_name,
                    "objectName": entry.object_name,
                    "error": str(error),
                }
            )
            continue
        objects.append(
            {
                "objectId": f"{logical_name}:export:{entry.index}",
                "exportIndex": entry.index,
                "className": class_name,
                "objectName": entry.object_name,
                "objectPath": package_ref_path(entry.index + 1, imports, exports),
                "serialSize": entry.serial_size,
                "propertyStreamEnd": property_end,
                "properties": properties,
                "references": references,
            }
        )

    graph_export_indexes = {row["exportIndex"] for row in objects}
    particle_emitter_reference_count = 0
    missing_emitter_targets = []
    for row in objects:
        if row["className"].casefold() != "particlesystem":
            continue
        for reference in row["references"]:
            if not reference["property"].casefold().startswith("emitters"):
                continue
            package_index = int(reference["packageIndex"])
            if package_index <= 0:
                continue
            particle_emitter_reference_count += 1
            target_export_index = package_index - 1
            if target_export_index not in graph_export_indexes:
                missing_emitter_targets.append(
                    {
                        "particleSystem": row["objectName"],
                        "property": reference["property"],
                        "targetExportIndex": target_export_index,
                        "targetObjectPath": reference["objectPath"],
                    }
                )

    return {
        "schemaVersion": 1,
        "package": logical_name,
        "physicalPackage": str(package_path),
        "source": "decrypted UE3 export payload tagged properties",
        "summary": {
            "graphObjectCount": len(objects),
            "particleSystemCount": sum(
                row["className"].casefold() == "particlesystem" for row in objects
            ),
            "propertyErrorCount": len(errors),
            "particleEmitterReferenceCount": particle_emitter_reference_count,
            "missingParticleEmitterTargetCount": len(missing_emitter_targets),
        },
        "objects": objects,
        "propertyErrors": errors,
        "graphInvariantErrors": missing_emitter_targets,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--umodel", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn"))
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    parser.add_argument("packages", nargs="+")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    rows = []
    for logical_name in args.packages:
        physical_path = resolve_physical_package(
            args.umodel, args.package_root, logical_name, args.region
        )
        result = extract_package(physical_path, logical_name, args.aes_key)
        output_path = args.output / f"{logical_name}.particle-graph.json"
        output_path.write_text(
            json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
        )
        row = {
            "logicalPackage": logical_name,
            "physicalPackage": physical_path.name,
            "graphFile": output_path.name,
            **result["summary"],
        }
        rows.append(row)
        print(json.dumps(row, ensure_ascii=False))

    manifest = {"schemaVersion": 1, "packages": rows}
    manifest_path = args.output / "particle_graph_manifest.json"
    manifest_path.write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps({"manifest": str(manifest_path), "packageCount": len(rows)}))
    return 0 if not any(
        row["propertyErrorCount"] or row["missingParticleEmitterTargetCount"]
        for row in rows
    ) else 1


if __name__ == "__main__":
    raise SystemExit(main())
