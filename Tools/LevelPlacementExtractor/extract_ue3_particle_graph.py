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
        or folded == "lightcomponent"
        or folded == "pointlightcomponent"
        or (folded.startswith("particle") and folded.endswith("emitter"))
        or folded.startswith("particlemodule")
        or folded.startswith("efparticlemodule")
        or folded.startswith("distributionfloat")
        or folded.startswith("distributionvector")
        or folded.startswith("efdistributionfloat")
        or folded.startswith("efdistributionvector")
    )


def parse_physical_package_overrides(values: list[str]) -> dict[str, Path]:
    overrides: dict[str, Path] = {}
    for value in values:
        logical_name, separator, physical_path = value.partition("=")
        if not separator or not logical_name.strip() or not physical_path.strip():
            raise ValueError(
                "--physical-package must use LOGICAL_NAME=ABSOLUTE_PATH"
            )
        path = Path(physical_path)
        if not path.is_file():
            raise ValueError(f"physical package does not exist: {path}")
        key = logical_name.strip().casefold()
        if key in overrides:
            raise ValueError(f"duplicate physical package override: {logical_name}")
        overrides[key] = path
    return overrides


def resolve_reference(value: int, imports: list[object], exports: list[object]) -> dict:
    return {
        "packageIndex": value,
        "objectName": package_ref_name(value, imports, exports),
        "objectPath": package_ref_path(value, imports, exports),
    }


def iter_property_reference_values(
    properties: dict, prefix: str = ""
):
    object_reference_arrays = {
        "defaultmaterials",
        "emitters",
        "lodlevels",
        "meshmaterials",
        "modules",
    }
    for name, item in properties.items():
        if not isinstance(item, dict):
            continue
        path = f"{prefix}.{name}" if prefix else str(name)
        kind = str(item.get("type", "")).casefold()
        value = item.get("value")
        if kind in {"objectproperty", "componentproperty", "interfaceproperty"}:
            if isinstance(value, int):
                yield path, value
        elif (
            kind == "arrayproperty"
            and str(name).split("[", 1)[0].casefold() in object_reference_arrays
            and isinstance(value, list)
        ):
            for entry in value:
                if isinstance(entry, int):
                    yield path, entry

        if isinstance(value, dict):
            nested = value.get("properties")
            if isinstance(nested, dict):
                yield from iter_property_reference_values(nested, path)
        elif isinstance(value, list):
            for index, entry in enumerate(value):
                if isinstance(entry, dict):
                    yield from iter_property_reference_values(
                        entry, f"{path}[{index}]"
                    )


def property_references(
    properties: dict, imports: list[object], exports: list[object]
) -> list[dict]:
    rows = []
    for property_path, package_index in iter_property_reference_values(properties):
        if package_index == 0:
            continue
        reference = resolve_reference(package_index, imports, exports)
        if reference["objectPath"] is not None:
            rows.append({"property": property_path, **reference})
    return rows


def iter_property_items(properties: dict):
    for item in properties.values():
        if not isinstance(item, dict):
            continue
        yield item
        value = item.get("value")
        if isinstance(value, dict):
            nested = value.get("properties")
            if isinstance(nested, dict):
                yield from iter_property_items(nested)
        elif isinstance(value, list):
            for entry in value:
                if isinstance(entry, dict):
                    yield from iter_property_items(entry)


def decoded_distribution_summary(objects: list[dict]) -> dict[str, int]:
    raw_distribution_count = 0
    interp_curve_count = 0
    interp_curve_point_count = 0
    opaque_supported_struct_count = 0
    for row in objects:
        for item in iter_property_items(row.get("properties", {})):
            structure = str(item.get("structType") or "").casefold()
            if structure in {"rawdistributionfloat", "rawdistributionvector"}:
                raw_distribution_count += 1
            elif structure in {"interpcurvefloat", "interpcurvevector"}:
                interp_curve_count += 1
                value = item.get("value")
                properties = value.get("properties") if isinstance(value, dict) else None
                points = properties.get("points") if isinstance(properties, dict) else None
                point_values = points.get("value") if isinstance(points, dict) else None
                if isinstance(point_values, list):
                    interp_curve_point_count += len(point_values)
            else:
                continue
            value = item.get("value")
            if not isinstance(value, dict) or not isinstance(
                value.get("properties"), dict
            ):
                opaque_supported_struct_count += 1
    return {
        "decodedRawDistributionCount": raw_distribution_count,
        "decodedInterpCurveCount": interp_curve_count,
        "decodedInterpCurvePointCount": interp_curve_point_count,
        "opaqueSupportedDistributionStructCount": opaque_supported_struct_count,
    }


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
                "classPath": package_ref_path(entry.class_index, imports, exports),
                "objectName": entry.object_name,
                "objectPath": package_ref_path(entry.index + 1, imports, exports),
                "archetypeIndex": entry.archetype_index,
                "archetypePath": package_ref_path(
                    entry.archetype_index, imports, exports
                ),
                "serialSize": entry.serial_size,
                "propertyStreamEnd": property_end,
                "properties": properties,
                "references": references,
            }
        )

    graph_export_indexes = {row["exportIndex"] for row in objects}
    graph_exports_by_index = {
        int(row["exportIndex"]): row for row in objects
    }
    particle_emitter_reference_count = 0
    missing_emitter_targets = []
    point_light_component_reference_count = 0
    invalid_point_light_component_targets = []
    for row in objects:
        class_name = row["className"].casefold()
        if class_name == "particlesystem":
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
        if "typedatalight" in class_name:
            references = [
                reference
                for reference in row["references"]
                if reference["property"].casefold() == "pointlightcomponent"
            ]
            point_light_component_reference_count += len(references)
            if len(references) != 1:
                invalid_point_light_component_targets.append(
                    {
                        "sourceModule": row["objectPath"],
                        "reason": "POINT_LIGHT_COMPONENT_REFERENCE_COUNT",
                        "referenceCount": len(references),
                    }
                )
                continue
            reference = references[0]
            package_index = int(reference["packageIndex"])
            target = (
                graph_exports_by_index.get(package_index - 1)
                if package_index > 0 else None
            )
            if target is None or target["className"].casefold() != "pointlightcomponent":
                invalid_point_light_component_targets.append(
                    {
                        "sourceModule": row["objectPath"],
                        "reason": "POINT_LIGHT_COMPONENT_TARGET_MISSING_OR_WRONG_CLASS",
                        "packageIndex": package_index,
                        "targetObjectPath": reference["objectPath"],
                        "targetClassName": (
                            target["className"] if target is not None else None
                        ),
                    }
                )

    graph_invariant_errors = [
        *missing_emitter_targets,
        *invalid_point_light_component_targets,
    ]

    return {
        "schemaVersion": 2,
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
            "pointLightComponentCount": sum(
                row["className"].casefold() == "pointlightcomponent"
                for row in objects
            ),
            "pointLightComponentReferenceCount": (
                point_light_component_reference_count
            ),
            "invalidPointLightComponentTargetCount": len(
                invalid_point_light_component_targets
            ),
            **decoded_distribution_summary(objects),
        },
        "objects": objects,
        "propertyErrors": errors,
        "graphInvariantErrors": graph_invariant_errors,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--umodel", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn"))
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    parser.add_argument(
        "--physical-package",
        action="append",
        default=[],
        metavar="LOGICAL_NAME=ABSOLUTE_PATH",
        help="Bypass UModel name resolution for an already identified package.",
    )
    parser.add_argument("packages", nargs="+")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    physical_overrides = parse_physical_package_overrides(args.physical_package)
    rows = []
    for logical_name in args.packages:
        physical_path = physical_overrides.get(
            logical_name.casefold()
        ) or resolve_physical_package(
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

    manifest = {"schemaVersion": 2, "packages": rows}
    manifest_path = args.output / "particle_graph_manifest.json"
    manifest_path.write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps({"manifest": str(manifest_path), "packageCount": len(rows)}))
    return 0 if not any(
        row["propertyErrorCount"]
        or row["missingParticleEmitterTargetCount"]
        or row["invalidPointLightComponentTargetCount"]
        for row in rows
    ) else 1


if __name__ == "__main__":
    raise SystemExit(main())
