#!/usr/bin/env python3
"""Build exact per-ParticleSystem material, texture, and mesh intake closure."""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path

from convert_recipe_to_effect import Converter, load_material_map


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--recipe-dir", type=Path, required=True)
    parser.add_argument("--material-map", type=Path, required=True)
    parser.add_argument("--inventory-csv", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    return parser.parse_args()


def iter_modules(asset: dict):
    for emitter_index, emitter in enumerate(asset.get("emitters", [])):
        for lod_index, lod in enumerate(emitter.get("lod_levels", [])):
            for module in lod.get("modules", []):
                yield emitter_index, lod_index, module


def package_object(path: str) -> tuple[str, str] | None:
    pieces = [piece for piece in str(path or "").split(".") if piece]
    if len(pieces) < 2:
        return None
    return pieces[0], pieces[-1]


def main() -> int:
    args = parse_args()
    with args.inventory_csv.open("r", encoding="utf-8-sig", newline="") as source:
        inventory = {
            row["logical_name"].casefold(): row for row in csv.DictReader(source)
        }
    logical_by_physical = {
        row["physical_file"].casefold(): row["logical_name"]
        for row in inventory.values()
    }

    material_map = load_material_map(args.material_map)
    converter = Converter(0.01, "", False, material_map)
    systems: list[dict] = []
    all_direct_materials: set[str] = set()
    all_meshes: set[str] = set()
    for recipe_path in sorted(args.recipe_dir.glob("*.recipe.json")):
        document = json.loads(recipe_path.read_text(encoding="utf-8"))
        for asset in document.get("assets", []):
            direct_materials: set[str] = set()
            meshes: set[str] = set()
            material_occurrences: list[str] = []
            module_count = 0
            for _, _, module in iter_modules(asset):
                module_count += 1
                params = module.get("params") or {}
                material = params.get("material")
                if isinstance(material, dict) and material.get("path"):
                    path = str(material["path"])
                    direct_materials.add(path)
                    material_occurrences.append(path)
                mesh = params.get("mesh")
                if isinstance(mesh, dict) and mesh.get("path"):
                    meshes.add(str(mesh["path"]))
            name = str(asset.get("object_name", ""))
            emitters = asset.get("emitters", [])
            unresolved_module_refs = [
                reference
                for emitter in emitters
                for lod in emitter.get("lod_levels", [])
                for reference in lod.get("unresolved_module_refs", [])
            ]
            systems.append(
                {
                    "recipeFile": recipe_path.name,
                    "objectName": name,
                    "isOld": name.casefold().startswith("old_"),
                    "isTest": "test" in name.casefold(),
                    "isCamera": "_cam" in name.casefold() or "camera" in name.casefold(),
                    "emitterCount": len(emitters),
                    "emptyLodEmitterCount": sum(not emitter.get("lod_levels") for emitter in emitters),
                    "unresolvedLodRefCount": sum(
                        len(emitter.get("unresolved_lod_refs", []))
                        for emitter in emitters
                    ),
                    "moduleCount": module_count,
                    "unresolvedModuleRefCount": len(unresolved_module_refs),
                    "externalImportModuleRefCount": sum(
                        int(reference.get("ref", 0)) < 0
                        for reference in unresolved_module_refs
                    ),
                    "directMaterials": sorted(direct_materials, key=str.casefold),
                    "materialOccurrences": material_occurrences,
                    "meshObjectPaths": sorted(meshes, key=str.casefold),
                }
            )
            all_direct_materials.update(direct_materials)
            all_meshes.update(meshes)

    material_layers_by_request: dict[str, list[dict]] = {}
    texture_refs_by_material: dict[str, set[tuple[str, str]]] = {}
    missing_materials: list[str] = []
    for path in sorted(all_direct_materials, key=str.casefold):
        layers = converter.material_layers(path)
        material_layers_by_request[path.casefold()] = layers
        if not layers:
            missing_materials.append(path)
            texture_refs_by_material[path.casefold()] = set()
            continue
        textures: set[tuple[str, str]] = set()
        for layer in layers:
            for item in layer.get("textures", []):
                if item.get("texture"):
                    textures.add((
                        str(item["texture"]),
                        str(layer.get("source_file") or ""),
                    ))
        texture_refs_by_material[path.casefold()] = textures

    missing_keys = {path.casefold() for path in missing_materials}
    for system in systems:
        texture_refs: set[tuple[str, str]] = set()
        for material in system["directMaterials"]:
            texture_refs.update(texture_refs_by_material[material.casefold()])
        system["textureReferences"] = [
            {"objectPath": path, "sourceFile": source_file}
            for path, source_file in sorted(
                texture_refs,
                key=lambda item: (item[0].casefold(), item[1].casefold()),
            )
        ]
        system["missingMaterials"] = sorted(
            [path for path in system["directMaterials"] if path.casefold() in missing_keys],
            key=str.casefold,
        )
        system["missingMaterialOccurrenceCount"] = sum(
            path.casefold() in missing_keys for path in system.pop("materialOccurrences")
        )

    def resource_row(path: str, kind: str, source_file: str = "") -> dict:
        parsed = package_object(path)
        if parsed is None:
            return {"objectPath": path, "kind": kind, "resolved": False}
        logical_package, object_name = parsed
        inventory_row = inventory.get(logical_package.casefold())
        if inventory_row is None and source_file:
            owner = logical_by_physical.get(source_file.casefold())
            if owner:
                logical_package = owner
                inventory_row = inventory.get(owner.casefold())
        return {
            "objectPath": path,
            "logicalPackage": logical_package,
            "objectName": object_name,
            "kind": kind,
            "sourceFile": source_file or None,
            "resolved": inventory_row is not None,
            "physicalPackage": inventory_row.get("physical_file") if inventory_row else None,
        }

    unique_textures: dict[tuple[str, str], dict] = {}
    for system in systems:
        resolved_rows = []
        for reference in system.pop("textureReferences"):
            row = resource_row(
                reference["objectPath"], "Texture2D", reference["sourceFile"]
            )
            key = (
                str(row.get("logicalPackage") or "").casefold(),
                str(row.get("objectName") or "").casefold(),
            )
            unique_textures.setdefault(key, row)
            resolved_rows.append({
                "objectPath": row["objectPath"],
                "logicalPackage": row.get("logicalPackage"),
                "objectName": row.get("objectName"),
                "physicalPackage": row.get("physicalPackage"),
                "resolved": row["resolved"],
            })
        system["textures"] = sorted(
            {json.dumps(row, sort_keys=True): row for row in resolved_rows}.values(),
            key=lambda row: (
                str(row.get("logicalPackage") or "").casefold(),
                str(row.get("objectName") or "").casefold(),
            ),
        )
    texture_rows = sorted(
        unique_textures.values(),
        key=lambda row: (
            str(row.get("logicalPackage") or "").casefold(),
            str(row.get("objectName") or "").casefold(),
        ),
    )
    mesh_rows = [
        resource_row(path, "StaticMesh")
        for path in sorted(all_meshes, key=str.casefold)
    ]
    unresolved_resources = [
        row for row in [*texture_rows, *mesh_rows] if not row["resolved"]
    ]
    affected_systems = [system for system in systems if system["missingMaterials"]]
    missing_occurrences = sum(
        system["missingMaterialOccurrenceCount"] for system in systems
    )
    result = {
        "schemaVersion": 2,
        "summary": {
            "particleSystemCandidateCount": len(systems),
            "emitterCount": sum(system["emitterCount"] for system in systems),
            "emptyLodEmitterCount": sum(system["emptyLodEmitterCount"] for system in systems),
            "unresolvedModuleRefAffectedParticleSystemCount": sum(
                system["unresolvedModuleRefCount"] > 0 for system in systems
            ),
            "unresolvedLodRefAffectedParticleSystemCount": sum(
                system["unresolvedLodRefCount"] > 0 for system in systems
            ),
            "unresolvedLodRefCount": sum(
                system["unresolvedLodRefCount"] for system in systems
            ),
            "unresolvedModuleRefCount": sum(
                system["unresolvedModuleRefCount"] for system in systems
            ),
            "externalImportModuleRefCount": sum(
                system["externalImportModuleRefCount"] for system in systems
            ),
            "directMaterialCount": len(all_direct_materials),
            "resolvedMaterialLayerCount": sum(len(layers) for layers in material_layers_by_request.values()),
            "missingMaterialCount": len(missing_materials),
            "missingMaterialAffectedParticleSystemCount": len(affected_systems),
            "missingMaterialOccurrenceCount": missing_occurrences,
            "uniqueTextureObjectCount": len(texture_rows),
            "uniqueMeshObjectCount": len(mesh_rows),
            "unresolvedResourceCount": len(unresolved_resources),
            "admissionIssueCount": (
                len(missing_materials)
                + len(unresolved_resources)
                + sum(system["unresolvedLodRefCount"] for system in systems)
                + sum(system["unresolvedModuleRefCount"] for system in systems)
            ),
        },
        "particleSystems": systems,
        "directMaterials": sorted(all_direct_materials, key=str.casefold),
        "missingMaterials": missing_materials,
        "textures": texture_rows,
        "meshes": mesh_rows,
        "unresolvedResources": unresolved_resources,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps({"output": str(args.output), **result["summary"]}))
    return 0 if not result["summary"]["admissionIssueCount"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
