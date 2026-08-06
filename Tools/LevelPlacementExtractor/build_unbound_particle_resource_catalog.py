#!/usr/bin/env python3
"""Catalog every ParticleSystem in class packages before skill notify mapping exists."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import sys
from collections import Counter
from pathlib import Path
from typing import Any

SCRIPT_ROOT = Path(__file__).resolve().parent
if str(SCRIPT_ROOT) not in sys.path:
    sys.path.insert(0, str(SCRIPT_ROOT))

from build_skill_effect_source_receipt import (  # noqa: E402
    collect_system_graph,
    load_graphs,
    resolve_material_parameters,
)


def load_inventory(path: Path) -> dict[str, dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as source:
        return {
            str(row["logical_name"]).casefold(): row
            for row in csv.DictReader(source)
            if row.get("logical_name") and row.get("physical_file")
        }


def asset_package(asset_path: str) -> str | None:
    package, separator, _ = asset_path.partition(".")
    return package if separator and package else None


def source_asset(package: str, object_path: str) -> str:
    if object_path.casefold().startswith(package.casefold() + "."):
        return object_path
    return f"{package}.{object_path}"


def build_catalog(
    graph_paths: list[Path],
    inventory_path: Path,
    character_class: str,
    material_map_paths: list[Path] | None,
    source_system_metadata: dict[str, dict[str, Any]] | None = None,
    source_direct_material_metadata: dict[str, dict[str, Any]] | None = None,
    source_direct_resource_metadata: dict[str, dict[str, Any]] | None = None,
) -> dict[str, Any]:
    graph_specs = []
    for path in graph_paths:
        graph = json.loads(path.read_text(encoding="utf-8-sig"))
        graph_specs.append((str(graph["package"]), path))
    graph_index = load_graphs(graph_specs)
    inventory = load_inventory(inventory_path)
    package_manifest = {
        key: str(row["physical_file"]) for key, row in inventory.items()
    }

    systems = []
    found_source_systems: set[str] = set()
    for package_key, package_graph in sorted(
        graph_index["packages"].items(),
        key=lambda item: item[1]["package"].casefold(),
    ):
        package = package_graph["package"]
        roots = sorted(
            (
                row for row in package_graph["graph"].get("objects", [])
                if str(row.get("className", "")).casefold() == "particlesystem"
            ),
            key=lambda row: str(row.get("objectPath", "")).casefold(),
        )
        for root in roots:
            system_asset = source_asset(package, str(root["objectPath"]))
            metadata = (
                source_system_metadata.get(system_asset.casefold())
                if source_system_metadata is not None else None
            )
            if source_system_metadata is not None and metadata is None:
                continue
            graph = collect_system_graph(graph_index, package_key, root)
            found_source_systems.add(system_asset.casefold())
            systems.append(
                {
                    "sourceAsset": system_asset,
                    "logicalPackage": package,
                    "objectName": root["objectName"],
                    "resolutionStatus": (
                        "ACTION_BOUND_SOURCE_SYSTEM"
                        if source_system_metadata is not None
                        else "UNBOUND_SOURCE_SYSTEM"
                    ),
                    **(
                        {
                            "profileIds": sorted(metadata["profileIds"], key=str.casefold),
                            "actionIds": sorted(metadata["actionIds"]),
                            "actionNames": sorted(metadata["actionNames"], key=str.casefold),
                            "clipNames": sorted(metadata["clipNames"], key=str.casefold),
                            "occurrenceCount": metadata["occurrenceCount"],
                        }
                        if metadata is not None else {}
                    ),
                    "graph": {
                        "rootNodeId": graph["rootNodeId"],
                        "resourceBindings": graph["resourceBindings"],
                        "unresolvedExternalReferences": graph[
                            "unresolvedExternalReferences"
                        ],
                        "summary": graph["summary"],
                    },
                }
            )

    material_catalog: dict[str, list[dict[str, Any]]] = {}
    identical_material_sources: dict[str, str] = {}
    if material_map_paths:
        for material_map_path in material_map_paths:
            document = json.loads(
                material_map_path.read_text(encoding="utf-8-sig")
            )
            for key, value in document.get("materials", {}).items():
                material_catalog.setdefault(str(key).casefold(), []).extend(value)
        for key, candidates in material_catalog.items():
            signatures = {
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
                for candidate in candidates
                if candidate.get("source_file")
            }
            sources = sorted(
                {
                    str(candidate["source_file"])
                    for candidate in candidates
                    if candidate.get("source_file")
                },
                key=str.casefold,
            )
            if len(signatures) == 1 and sources:
                identical_material_sources[key] = sources[0]
    material_paths = sorted(
        {
            str(binding["objectPath"])
            for system in systems
            for binding in system["graph"]["resourceBindings"]
            if binding.get("role") == "material" and binding.get("objectPath")
        } | {
            str(row["sourceAsset"])
            for row in (source_direct_material_metadata or {}).values()
        },
        key=str.casefold,
    )
    material_bindings = [
        resolve_material_parameters(path, material_catalog, package_manifest)
        for path in material_paths
    ] if material_catalog else []

    material_by_path = {
        str(row["sourceMaterialPath"]).casefold(): row
        for row in material_bindings
    }
    assets: dict[str, dict[str, Any]] = {}

    def add_asset(
        path: str | None,
        role: str,
        source_system_name: str,
        preferred_physical: str | None = None,
    ) -> None:
        if not path:
            return
        row = assets.setdefault(
            path.casefold(),
            {
                "sourceAssetPath": path,
                "roles": set(),
                "sourceSystems": set(),
                "preferredPhysicalPackages": set(),
            },
        )
        row["roles"].add(role)
        row["sourceSystems"].add(source_system_name)
        if preferred_physical:
            row["preferredPhysicalPackages"].add(preferred_physical)

    for system in systems:
        system_name = system["sourceAsset"]
        for binding in system["graph"]["resourceBindings"]:
            path = binding.get("objectPath")
            role = str(binding.get("role", "unknown"))
            material = material_by_path.get(str(path).casefold())
            add_asset(
                path,
                role,
                system_name,
                material.get("sourcePhysicalPackage") if material else None,
            )
            if not material or not str(
                material.get("resolutionStatus", "")
            ).startswith("RESOLVED_"):
                continue
            add_asset(
                material.get("parent"),
                "material_parent",
                system_name,
                material.get("parentSourcePhysicalPackage") or
                identical_material_sources.get(
                    str(material.get("parent", "")).casefold()
                ),
            )
            for texture in material.get("textures", []):
                add_asset(texture.get("texture"), "texture", system_name)

    serialized_direct_materials = []
    for metadata in sorted(
        (source_direct_material_metadata or {}).values(),
        key=lambda row: str(row["sourceAsset"]).casefold(),
    ):
        material_path = str(metadata["sourceAsset"])
        source_labels = sorted(
            {
                f"{profile_id}:{source_name}"
                for profile_id in metadata["profileIds"]
                for source_name in metadata["actionNames"]
            },
            key=str.casefold,
        ) or sorted(metadata["profileIds"], key=str.casefold)
        material = material_by_path.get(material_path.casefold())
        for source_label in source_labels:
            add_asset(
                material_path,
                "material",
                source_label,
                material.get("sourcePhysicalPackage") if material else None,
            )
            if not material or not str(
                material.get("resolutionStatus", "")
            ).startswith("RESOLVED_"):
                continue
            add_asset(
                material.get("parent"),
                "material_parent",
                source_label,
                material.get("parentSourcePhysicalPackage") or
                identical_material_sources.get(
                    str(material.get("parent", "")).casefold()
                ),
            )
            for texture in material.get("textures", []):
                add_asset(texture.get("texture"), "texture", source_label)
        serialized_direct_materials.append(
            {
                "sourceAsset": material_path,
                "profileIds": sorted(metadata["profileIds"], key=str.casefold),
                "sourceNames": sorted(metadata["actionNames"], key=str.casefold),
                "occurrenceCount": metadata["occurrenceCount"],
                "resolutionStatus": (
                    material.get("resolutionStatus")
                    if material else "UNRESOLVED_MATERIAL_BINDING"
                ),
            }
        )

    serialized_direct_resources = []
    for metadata in sorted(
        (source_direct_resource_metadata or {}).values(),
        key=lambda row: (str(row["role"]), str(row["sourceAsset"]).casefold()),
    ):
        source_path = str(metadata["sourceAsset"])
        source_labels = sorted(
            {
                f"{profile_id}:{source_name}"
                for profile_id in metadata["profileIds"]
                for source_name in metadata["actionNames"]
            },
            key=str.casefold,
        ) or sorted(metadata["profileIds"], key=str.casefold)
        for source_label in source_labels:
            add_asset(source_path, str(metadata["role"]), source_label)
        serialized_direct_resources.append(
            {
                "sourceAsset": source_path,
                "role": metadata["role"],
                "classNames": sorted(metadata["classNames"], key=str.casefold),
                "profileIds": sorted(metadata["profileIds"], key=str.casefold),
                "actionIds": sorted(metadata["actionIds"]),
                "sourceNames": sorted(metadata["actionNames"], key=str.casefold),
                "occurrenceCount": metadata["occurrenceCount"],
            }
        )

    serialized_assets = []
    unresolved_packages = set()
    role_counts: Counter[str] = Counter()
    for row in sorted(
        assets.values(), key=lambda item: item["sourceAssetPath"].casefold()
    ):
        package = asset_package(row["sourceAssetPath"])
        package_row = inventory.get(package.casefold()) if package else None
        preferred = sorted(row["preferredPhysicalPackages"], key=str.casefold)
        physical = (
            preferred[0]
            if len(preferred) == 1
            else package_row.get("physical_file") if package_row else None
        )
        is_engine_fallback = (
            str(row["sourceAssetPath"]).casefold()
            == "enginematerials.defaultparticle"
        )
        if physical is None and not is_engine_fallback:
            unresolved_packages.add(package or "<invalid>")
        roles = sorted(row["roles"])
        role_counts.update(roles)
        serialized_assets.append(
            {
                "sourceAssetPath": row["sourceAssetPath"],
                "logicalPackage": package,
                "physicalPackage": physical,
                "roles": roles,
                "skillIds": [],
                "actionIds": sorted(
                    {
                        action_id
                        for system_name in row["sourceSystems"]
                        for action_id in (
                            source_system_metadata.get(system_name.casefold(), {}).get(
                                "actionIds", set()
                            ) if source_system_metadata is not None else ()
                        )
                    }
                ),
                "sourceSystems": sorted(row["sourceSystems"], key=str.casefold),
                "resolutionStatus": (
                    "ENGINE_DEFAULT_PARTICLE_FALLBACK"
                    if is_engine_fallback else
                    "RESOLVED_SOURCE_PACKAGE"
                    if physical else "UNRESOLVED_SOURCE_PACKAGE"
                ),
            }
        )

    unresolved_materials = [
        row for row in material_bindings
        if not str(row.get("resolutionStatus", "")).startswith("RESOLVED_")
    ]
    graph_summaries = [
        {
            "logicalPackage": package_graph["package"],
            "graphFile": package_graph["path"].as_posix(),
            "summary": package_graph["graph"].get("summary", {}),
        }
        for package_graph in graph_index["packages"].values()
    ]
    return {
        "schema": "lostark.unbound-class-particle-resource-catalog",
        "formatVersion": 1,
        "characterClass": character_class,
        "bindingStatus": (
            "ACTION_NOTIFY_BOUND"
            if source_system_metadata is not None
            else "SKILL_NOTIFY_SOURCE_NOT_AVAILABLE"
        ),
        "sourcePackageGraphs": sorted(
            graph_summaries, key=lambda row: row["logicalPackage"].casefold()
        ),
        "sourceSystems": systems,
        "sourceDirectMaterials": serialized_direct_materials,
        "sourceDirectResources": serialized_direct_resources,
        "materialParameterBindings": material_bindings,
        "assets": serialized_assets,
        "unresolvedMaterialBindings": unresolved_materials,
        "unresolvedLogicalPackages": sorted(unresolved_packages, key=str.casefold),
        "missingActionSourceSystems": sorted(
            (
                set(source_system_metadata) - found_source_systems
                if source_system_metadata is not None else set()
            ),
            key=str.casefold,
        ),
        "summary": {
            "sourcePackageCount": len(graph_paths),
            "sourceSystemCount": len(systems),
            "sourceDirectMaterialCount": len(serialized_direct_materials),
            "sourceDirectResourceCount": len(serialized_direct_resources),
            "assetCount": len(serialized_assets),
            "resolvedSourcePackageCount": sum(
                row["resolutionStatus"] == "RESOLVED_SOURCE_PACKAGE"
                for row in serialized_assets
            ),
            "unresolvedSourcePackageCount": sum(
                row["resolutionStatus"] == "UNRESOLVED_SOURCE_PACKAGE"
                for row in serialized_assets
            ),
            "engineFallbackAssetCount": sum(
                row["resolutionStatus"] == "ENGINE_DEFAULT_PARTICLE_FALLBACK"
                for row in serialized_assets
            ),
            "unresolvedMaterialBindingCount": len(unresolved_materials),
            "requestedActionSourceSystemCount": (
                len(source_system_metadata)
                if source_system_metadata is not None else 0
            ),
            "missingActionSourceSystemCount": (
                len(set(source_system_metadata) - found_source_systems)
                if source_system_metadata is not None else 0
            ),
            "roleCounts": dict(sorted(role_counts.items())),
        },
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def load_action_source_metadata(
    paths: list[Path],
) -> tuple[
    dict[str, dict[str, Any]],
    dict[str, dict[str, Any]],
    dict[str, dict[str, Any]],
    list[dict[str, Any]],
]:
    metadata: dict[str, dict[str, Any]] = {}
    direct_material_metadata: dict[str, dict[str, Any]] = {}
    direct_resource_metadata: dict[str, dict[str, Any]] = {}
    receipts = []

    def merge_source(
        target: dict[str, dict[str, Any]],
        source: dict[str, Any],
        profile_id: str,
    ) -> None:
        source_asset_path = str(source["sourceAsset"])
        row = target.setdefault(
            source_asset_path.casefold(),
            {
                "sourceAsset": source_asset_path,
                "profileIds": set(),
                "actionIds": set(),
                "actionNames": set(),
                "clipNames": set(),
                "occurrenceCount": 0,
            },
        )
        row["profileIds"].add(profile_id)
        row["actionIds"].update(int(value) for value in source.get("actionIds", []))
        row["actionNames"].update(str(value) for value in source.get("actionNames", []))
        row["clipNames"].update(str(value) for value in source.get("clipNames", []))
        row["occurrenceCount"] += int(source.get("occurrenceCount", 0))

    for path in paths:
        document = json.loads(path.read_text(encoding="utf-8-sig"))
        profile_id = str(document.get("profileId", path.stem))
        for source in document.get("particleSystems", []):
            merge_source(metadata, source, profile_id)
        for source in document.get("materials", []):
            merge_source(direct_material_metadata, source, profile_id)
        for role, field in (("mesh", "meshes"), ("texture", "textures")):
            for source in document.get(field, []):
                source_asset_path = str(source["sourceAsset"])
                key = f"{role}:{source_asset_path.casefold()}"
                row = direct_resource_metadata.setdefault(
                    key,
                    {
                        "sourceAsset": source_asset_path,
                        "role": role,
                        "classNames": set(),
                        "profileIds": set(),
                        "actionIds": set(),
                        "actionNames": set(),
                        "occurrenceCount": 0,
                    },
                )
                row["classNames"].update(
                    str(value) for value in source.get("classNames", [])
                )
                row["profileIds"].add(profile_id)
                row["actionIds"].update(
                    int(value) for value in source.get("actionIds", [])
                )
                row["actionNames"].update(
                    str(value) for value in source.get("actionNames", [])
                )
                row["occurrenceCount"] += int(source.get("occurrenceCount", 0))
        receipts.append(
            {
                "profileId": profile_id,
                "path": path.as_posix(),
                "sha256": sha256_file(path),
                "summary": document.get("summary", {}),
            }
        )
    return metadata, direct_material_metadata, direct_resource_metadata, receipts


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--graphs-root", required=True, type=Path)
    parser.add_argument("--inventory-csv", required=True, type=Path)
    parser.add_argument("--character-class", required=True)
    parser.add_argument("--material-map", action="append", type=Path)
    parser.add_argument("--action-source", action="append", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    graph_paths = sorted(args.graphs_root.glob("*.particle-graph.json"))
    if not graph_paths:
        raise ValueError(f"no package particle graphs below {args.graphs_root}")
    source_metadata = None
    source_direct_material_metadata = None
    source_direct_resource_metadata = None
    action_source_receipts = []
    if args.action_source:
        (
            source_metadata,
            source_direct_material_metadata,
            source_direct_resource_metadata,
            action_source_receipts,
        ) = load_action_source_metadata(args.action_source)
    document = build_catalog(
        graph_paths,
        args.inventory_csv,
        args.character_class,
        args.material_map,
        source_metadata,
        source_direct_material_metadata,
        source_direct_resource_metadata,
    )
    if action_source_receipts:
        document["sourceActionDocuments"] = action_source_receipts
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    print(json.dumps(document["summary"], ensure_ascii=False, sort_keys=True))
    return 0 if not (
        document["summary"]["unresolvedSourcePackageCount"]
        or document["summary"]["missingActionSourceSystemCount"]
    ) else 1


if __name__ == "__main__":
    raise SystemExit(main())
