#!/usr/bin/env python3
"""Build a reproducible source/dependency manifest for decoded UE3 particle graphs."""

from __future__ import annotations

import argparse
import csv
import json
from collections import Counter
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--graphs-dir", type=Path, required=True)
    parser.add_argument("--inventory-csv", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--source-package", action="append", required=True)
    parser.add_argument(
        "--dependency-report",
        type=Path,
        action="append",
        default=[],
        help="add direct ImportTable package roots from an extractor dependency report",
    )
    parser.add_argument(
        "--allow-unresolved",
        action="store_true",
        help="succeed while preserving unresolved external package names in the manifest",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    with args.inventory_csv.open("r", encoding="utf-8-sig", newline="") as source:
        inventory = {row["logical_name"].casefold(): row for row in csv.DictReader(source)}

    class_counts: Counter[str] = Counter()
    property_type_counts: Counter[str] = Counter()
    particle_systems = []
    referenced_roots: set[str] = set()
    reference_count = 0
    tagged_property_count = 0
    raw_property_value_count = 0
    summarized_array_value_count = 0
    graph_rows = []
    for logical_name in args.source_package:
        path = args.graphs_dir / f"{logical_name}.particle-graph.json"
        graph = json.loads(path.read_text(encoding="utf-8"))
        graph_rows.append(
            {
                "logicalPackage": logical_name,
                "graphFile": path.name,
                **graph["summary"],
            }
        )
        for entry in graph["objects"]:
            class_counts[entry["className"]] += 1
            for property_entry in entry["properties"].values():
                tagged_property_count += 1
                property_type_counts[str(property_entry.get("type", "<unknown>"))] += 1
                value = property_entry.get("value")
                if isinstance(value, dict) and "hex" in value:
                    raw_property_value_count += 1
                elif (
                    isinstance(value, dict)
                    and "count" in value
                    and "size" in value
                ):
                    summarized_array_value_count += 1
            if entry["className"].casefold() == "particlesystem":
                particle_systems.append(
                    {
                        "objectId": entry["objectId"],
                        "logicalPackage": logical_name,
                        "objectName": entry["objectName"],
                    }
                )
            for reference in entry["references"]:
                object_path = reference.get("objectPath")
                if not object_path or int(reference.get("packageIndex", 0)) >= 0:
                    continue
                reference_count += 1
                root = object_path.split(".", 1)[0]
                if root.casefold() not in {"core", "engine", "efgame"}:
                    referenced_roots.add(root)

    dependency_reports = []
    direct_import_reference_count = 0
    for path in args.dependency_report:
        report = json.loads(path.read_text(encoding="utf-8"))
        packages = list(report.get("dependencyPackages", []))
        dependency_reports.append(
            {
                "logicalPackage": report.get("package", path.stem),
                "reportFile": path.name,
                "directDependencyPackageCount": len(packages),
            }
        )
        for logical_name in packages:
            direct_import_reference_count += 1
            if logical_name.casefold() not in {"core", "engine", "efgame"}:
                referenced_roots.add(logical_name)

    def inventory_row(logical_name: str) -> dict:
        row = inventory.get(logical_name.casefold())
        if row is None:
            return {"logicalPackage": logical_name, "resolved": False}
        return {
            "logicalPackage": row["logical_name"],
            "physicalPackage": row["physical_file"],
            "byteSize": int(row["byte_size"]),
            "resolved": True,
        }

    source_packages = [inventory_row(name) for name in args.source_package]
    dependencies = [
        inventory_row(name) for name in sorted(referenced_roots, key=str.casefold)
    ]
    unresolved = [row for row in dependencies if not row["resolved"]]
    feature_classes = {
        "decal": sorted(name for name in class_counts if "decal" in name.casefold()),
        "mesh": sorted(name for name in class_counts if "typedatamesh" in name.casefold()),
        "ribbon": sorted(name for name in class_counts if "ribbon" in name.casefold()),
        "subUV": sorted(name for name in class_counts if "subuv" in name.casefold()),
        "vectorField": sorted(name for name in class_counts if "vectorfield" in name.casefold()),
        "cameraOffset": sorted(name for name in class_counts if "cameraoffset" in name.casefold()),
    }
    result = {
        "schemaVersion": 1,
        "sourcePackages": source_packages,
        "graphs": graph_rows,
        "dependencyReports": dependency_reports,
        "summary": {
            "sourcePackageCount": len(source_packages),
            "graphObjectCount": sum(row["graphObjectCount"] for row in graph_rows),
            "particleSystemCount": len(particle_systems),
            "propertyErrorCount": sum(row["propertyErrorCount"] for row in graph_rows),
            "referenceCount": reference_count,
            "directImportReferenceCount": direct_import_reference_count,
            "taggedPropertyCount": tagged_property_count,
            "rawPropertyValueCount": raw_property_value_count,
            "summarizedArrayValueCount": summarized_array_value_count,
            "dependencyPackageCount": len(dependencies),
            "resolvedDependencyPackageCount": len(dependencies) - len(unresolved),
            "dependencyBytes": sum(row.get("byteSize", 0) for row in dependencies),
        },
        "featureClasses": feature_classes,
        "classCounts": dict(sorted(class_counts.items())),
        "propertyTypeCounts": dict(sorted(property_type_counts.items())),
        "particleSystems": sorted(
            particle_systems,
            key=lambda row: (row["logicalPackage"].casefold(), row["objectName"].casefold()),
        ),
        "dependencies": dependencies,
        "unresolvedDependencies": unresolved,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps({"output": str(args.output), **result["summary"]}, ensure_ascii=False))
    has_blocking_unresolved = bool(unresolved) and not args.allow_unresolved
    return 0 if not has_blocking_unresolved and not result["summary"]["propertyErrorCount"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
