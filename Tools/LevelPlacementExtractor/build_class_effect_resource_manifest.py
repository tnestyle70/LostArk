#!/usr/bin/env python3
"""Merge per-skill normalized graphs into an exact class resource manifest."""

from __future__ import annotations

import argparse
import csv
import json
from collections import Counter
from pathlib import Path
from typing import Any


def load_package_inventory(path: Path) -> dict[str, dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as source:
        rows = list(csv.DictReader(source))
    return {
        str(row["logical_name"]).casefold(): row
        for row in rows
        if row.get("logical_name") and row.get("physical_file")
    }


def asset_package(asset_path: str) -> str | None:
    package, separator, _ = asset_path.partition(".")
    return package if separator and package else None


def build_manifest(
    graph_paths: list[Path],
    inventory_path: Path,
    character_class: str,
    material_map_path: Path | None = None,
) -> dict[str, Any]:
    inventory = load_package_inventory(inventory_path)
    material_sources: dict[str, set[str]] = {}
    identical_material_sources: dict[str, str] = {}
    if material_map_path is not None:
        material_map = json.loads(material_map_path.read_text(encoding="utf-8-sig"))
        for key, candidates in material_map.get("materials", {}).items():
            signatures: set[str] = set()
            for candidate in candidates:
                source_file = candidate.get("source_file")
                material_path = candidate.get("material_path") or key
                if source_file and material_path:
                    material_sources.setdefault(
                        str(material_path).casefold(), set()
                    ).add(str(source_file))
                    signatures.add(
                        json.dumps(
                            {
                                field: candidate.get(field)
                                for field in (
                                    "material_path",
                                    "object_name",
                                    "class",
                                    "parent",
                                    "textures",
                                    "scalars",
                                    "vectors",
                                )
                            },
                            ensure_ascii=False,
                            sort_keys=True,
                            separators=(",", ":"),
                        )
                    )
            folded_key = str(key).casefold()
            if len(signatures) == 1 and material_sources.get(folded_key):
                identical_material_sources[folded_key] = sorted(
                    material_sources[folded_key], key=str.casefold
                )[0]
    assets: dict[str, dict[str, Any]] = {}
    unresolved_materials: dict[tuple[int, str], dict[str, Any]] = {}

    def add_asset(
        asset_path: str | None,
        role: str,
        skill_id: int,
        source_system: str | None = None,
        preferred_physical: str | None = None,
    ) -> None:
        if not asset_path:
            return
        key = asset_path.casefold()
        row = assets.setdefault(
            key,
            {
                "sourceAssetPath": asset_path,
                "roles": set(),
                "skillIds": set(),
                "sourceSystems": set(),
                "preferredPhysicalPackages": set(),
            },
        )
        row["roles"].add(role)
        row["skillIds"].add(skill_id)
        if source_system:
            row["sourceSystems"].add(source_system)
        if preferred_physical:
            row["preferredPhysicalPackages"].add(preferred_physical)

    for graph_path in graph_paths:
        graph = json.loads(graph_path.read_text(encoding="utf-8-sig"))
        skill_id = int(graph["skillId"])
        if str(graph.get("characterClass")) != character_class:
            raise ValueError(
                f"class mismatch in {graph_path}: {graph.get('characterClass')}"
            )
        for system in graph.get("sourceSystems", []):
            source_system = system.get("sourceAsset")
            for binding in system.get("resourceBindings", []):
                add_asset(
                    binding.get("objectPath"),
                    str(binding.get("role", "unknown")),
                    skill_id,
                    source_system,
                )

        for material in graph.get("materialParameterBindings", []):
            status = str(material.get("resolutionStatus", ""))
            source_material = material.get("sourceMaterialPath")
            add_asset(
                source_material,
                "material",
                skill_id,
                preferred_physical=material.get("sourcePhysicalPackage"),
            )
            if not status.startswith("RESOLVED_"):
                unresolved_materials[(skill_id, str(source_material))] = {
                    "skillId": skill_id,
                    "sourceMaterialPath": source_material,
                    "resolutionStatus": status,
                    "candidateCount": material.get("candidateCount", 0),
                }
                continue
            add_asset(
                material.get("parent"),
                "material_parent",
                skill_id,
                preferred_physical=(
                    identical_material_sources.get(
                        str(material.get("parent")).casefold()
                    )
                    if material.get("parent")
                    else None
                ),
            )
            for texture in material.get("textures", []):
                add_asset(texture.get("texture"), "texture", skill_id)

    serialized_assets = []
    unresolved_packages = set()
    role_counts: Counter[str] = Counter()
    for row in sorted(assets.values(), key=lambda item: item["sourceAssetPath"].casefold()):
        package = asset_package(row["sourceAssetPath"])
        package_row = inventory.get(package.casefold()) if package else None
        preferred = sorted(row["preferredPhysicalPackages"], key=str.casefold)
        physical_package = (
            preferred[0]
            if len(preferred) == 1
            else package_row.get("physical_file")
            if package_row is not None
            else None
        )
        if physical_package is None:
            unresolved_packages.add(package or "<invalid>")
        roles = sorted(row["roles"])
        role_counts.update(roles)
        serialized_assets.append(
            {
                "sourceAssetPath": row["sourceAssetPath"],
                "logicalPackage": package,
                "physicalPackage": physical_package,
                "roles": roles,
                "skillIds": sorted(row["skillIds"]),
                "sourceSystems": sorted(row["sourceSystems"], key=str.casefold),
                "resolutionStatus": (
                    "RESOLVED_SOURCE_PACKAGE"
                    if physical_package is not None
                    else "UNRESOLVED_SOURCE_PACKAGE"
                ),
            }
        )

    return {
        "schema": "lostark.class-effect-resource-source-manifest",
        "formatVersion": 1,
        "characterClass": character_class,
        "sourceGraphs": [path.as_posix() for path in sorted(graph_paths)],
        "assets": serialized_assets,
        "unresolvedMaterialBindings": sorted(
            unresolved_materials.values(),
            key=lambda row: (row["skillId"], str(row["sourceMaterialPath"]).casefold()),
        ),
        "unresolvedLogicalPackages": sorted(unresolved_packages, key=str.casefold),
        "summary": {
            "skillGraphCount": len(graph_paths),
            "assetCount": len(serialized_assets),
            "resolvedSourcePackageCount": sum(
                row["resolutionStatus"] == "RESOLVED_SOURCE_PACKAGE"
                for row in serialized_assets
            ),
            "unresolvedSourcePackageCount": sum(
                row["resolutionStatus"] == "UNRESOLVED_SOURCE_PACKAGE"
                for row in serialized_assets
            ),
            "unresolvedMaterialBindingCount": len(unresolved_materials),
            "roleCounts": dict(sorted(role_counts.items())),
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--graphs-root", required=True, type=Path)
    parser.add_argument("--inventory-csv", required=True, type=Path)
    parser.add_argument("--character-class", required=True)
    parser.add_argument("--material-map", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    graph_paths = sorted(args.graphs_root.rglob("*.normalized-effect-graph.json"))
    if not graph_paths:
        raise ValueError(f"no normalized graphs below {args.graphs_root}")
    document = build_manifest(
        graph_paths, args.inventory_csv, args.character_class, args.material_map
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(document["summary"], ensure_ascii=False, sort_keys=True))
    return 0 if not document["summary"]["unresolvedSourcePackageCount"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
