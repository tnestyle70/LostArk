#!/usr/bin/env python3
"""Audit admitted DimensionMaster Effects by their actual renderer family.

Cascade calls both mesh and sprite renderers particles.  This audit preserves
that source provenance while separating skeletal model cues, static mesh
instances, sprite/procedural layers, and typed presentation channels.  It does
not infer that a static mesh is a complete visual merely because a WModel is
present.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

from build_dimensionmaster_base_effects import dimensionmaster_admitted_skills


MESH_MODEL_SLOT = "meshmodel"
MATERIAL_TEXTURE_SLOTS = {
    "base", "noise", "mask", "emissive", "dissolve",
}
BASE11_SLOTS = {
    "BA", "LMB", "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V",
}


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def resources_by_slot(element: dict[str, Any]) -> dict[str, str]:
    resources: dict[str, str] = {}
    for binding in element.get("resources", []):
        if not isinstance(binding, dict):
            raise ValueError("Effect resources must contain objects")
        slot = str(binding.get("slotId") or "").casefold()
        asset = str(binding.get("assetId") or "")
        if not slot or not asset:
            raise ValueError("Effect resource binding requires slotId and assetId")
        if slot in resources:
            raise ValueError(f"duplicate Effect resource slot: {slot}")
        resources[slot] = asset
    return resources


def model_override_literal(element: dict[str, Any]) -> bool | None:
    values: list[bool] = []
    source_recipe = element.get("sourceRecipe") or {}
    for module in source_recipe.get("modules", []):
        if str(module.get("className") or "").casefold() != (
            "particlemoduletypedatamesh"
        ):
            continue
        for literal in module.get("literals", []):
            if str(literal.get("propertyPath") or "").casefold() != (
                "boverridematerial"
            ):
                continue
            value = literal.get("value")
            if not isinstance(value, bool):
                raise ValueError("bOverrideMaterial must be boolean")
            values.append(value)
    if not values:
        return None
    if any(value != values[0] for value in values[1:]):
        raise ValueError("conflicting bOverrideMaterial values")
    return values[0]


def source_mesh_rotations(element: dict[str, Any]) -> list[dict[str, Any]]:
    source_recipe = element.get("sourceRecipe") or {}
    matches: list[dict[str, Any]] = []
    for module in source_recipe.get("modules", []):
        if str(module.get("className") or "").casefold() != (
            "particlemodulemeshrotation"
        ):
            continue
        for distribution in module.get("distributions", []):
            if str(distribution.get("propertyPath") or "").casefold() != (
                "startrotation"
            ):
                continue
            matches.append({
                "moduleStableId": str(module.get("stableId") or ""),
                "moduleObjectPath": str(module.get("objectPath") or ""),
                "componentCount": int(distribution.get("componentCount", 0)),
                "operation": int(distribution.get("operation", 0)),
                "randomLockAxes": int(distribution.get("randomLockAxes", 0)),
                "lookupTable": list(distribution.get("lookupTable", [])),
                "keyCount": len(distribution.get("keys", [])),
            })
    return matches


def runtime_shader_profile(element: dict[str, Any]) -> str:
    material = element.get("material") or {}
    source_profile = material.get("sourceProfile") or {}
    return str(source_profile.get("runtimeShaderProfileId") or "")


def classify_cascade_layer(element: dict[str, Any]) -> str:
    if str(element.get("kind") or "").casefold() != "particle":
        raise ValueError("renderer-family classifier only accepts Cascade layers")
    resources = resources_by_slot(element)
    shape = str(
        (element.get("sourceRecipe") or {}).get("rendererShape") or ""
    ).casefold()
    has_mesh = MESH_MODEL_SLOT in resources
    if (shape == "mesh") != has_mesh:
        raise ValueError(
            f"renderer shape and meshModel binding disagree: {element.get('id')}"
        )
    if has_mesh:
        use_model_material = bool(
            (((element.get("detail") or {}).get("mesh") or {})
             .get("useModelMaterial"))
        )
        override = model_override_literal(element)
        if override is True and use_model_material:
            return "STATIC_MESH_CONTRACT_CONTRADICTION"
        if use_model_material and override is not True:
            return "STATIC_MESH_EMBEDDED_MATERIAL_CANDIDATE"
        return "STATIC_MESH_MATERIAL_OVERRIDE_CARRIER"

    profile = runtime_shader_profile(element).casefold()
    slots = set(resources)
    if "fallback-blocked" in profile:
        return "SPRITE_UNRESOLVED_MATERIAL"
    if slots.intersection(MATERIAL_TEXTURE_SLOTS):
        return "SPRITE_MATERIAL_CARRIER"
    if profile:
        return "SPRITE_PROCEDURAL_PROFILE"
    return "SPRITE_UNRESOLVED_MATERIAL"


def event_base_element_id(element_id: str) -> str:
    return element_id.split(".event_", 1)[0]


def compact_transform(element: dict[str, Any]) -> dict[str, Any]:
    detail = element.get("detail") or {}
    transform = detail.get("transform") or {}
    timing = detail.get("timing") or {}
    return {
        "elementId": str(element.get("id") or ""),
        "startDelaySeconds": float(timing.get("startDelaySeconds", 0.0)),
        "position": list(transform.get("position", [])),
        "rotationDegrees": list(transform.get("rotationDegrees", [])),
        "scale": list(transform.get("scale", [])),
    }


def build_mesh_groups(
    mesh_elements: list[tuple[dict[str, Any], str]],
) -> list[dict[str, Any]]:
    groups: dict[tuple[str, str], list[dict[str, Any]]] = defaultdict(list)
    for element, classification in mesh_elements:
        mesh = resources_by_slot(element)[MESH_MODEL_SLOT]
        groups[(mesh, classification)].append(element)
    result = []
    for (asset_id, classification), elements in sorted(
        groups.items(), key=lambda row: (row[0][0].casefold(), row[0][1])
    ):
        profile_counts = Counter(runtime_shader_profile(row) for row in elements)
        result.append({
            "modelAssetId": asset_id,
            "classification": classification,
            "occurrenceCount": len(elements),
            "runtimeShaderProfiles": [
                {"profileId": profile, "occurrenceCount": count}
                for profile, count in sorted(profile_counts.items())
            ],
            "elementIds": sorted(
                (str(row.get("id") or "") for row in elements),
                key=str.casefold,
            ),
        })
    return result


def build_repeated_mesh_groups(
    mesh_elements: list[tuple[dict[str, Any], str]],
) -> list[dict[str, Any]]:
    groups: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for element, _classification in mesh_elements:
        groups[event_base_element_id(str(element.get("id") or ""))].append(element)
    result = []
    for base_id, elements in sorted(groups.items(), key=lambda row: row[0].casefold()):
        if len(elements) <= 1:
            continue
        ordered = sorted(
            elements,
            key=lambda row: (
                float(((row.get("detail") or {}).get("timing") or {})
                      .get("startDelaySeconds", 0.0)),
                str(row.get("id") or "").casefold(),
            ),
        )
        rotations = [source_mesh_rotations(row) for row in ordered]
        if any(value != rotations[0] for value in rotations[1:]):
            raise ValueError(
                f"event copies changed source mesh rotation contract: {base_id}"
            )
        result.append({
            "baseElementId": base_id,
            "instanceCount": len(ordered),
            "sourceMeshRotations": rotations[0],
            "instances": [compact_transform(row) for row in ordered],
        })
    return result


def validate_document_identity(
    document: dict[str, Any], skill_id: int, effect_asset_id: str,
) -> None:
    if document.get("schema") != "lostark.effect-authoring":
        raise ValueError(f"invalid Effect authoring schema for skill {skill_id}")
    if str(document.get("effectAssetId") or "") != effect_asset_id:
        raise ValueError(f"Effect asset identity mismatch for skill {skill_id}")
    ids: set[str] = set()
    for element in document.get("elements", []):
        element_id = str(element.get("id") or "")
        if not element_id or element_id.casefold() in ids:
            raise ValueError(f"missing or duplicate Element ID for skill {skill_id}")
        ids.add(element_id.casefold())


def source_skeletal_cues(recipe: dict[str, Any]) -> list[dict[str, Any]]:
    result = []
    for cue in recipe.get("cues", []):
        if cue.get("sourceType") != "PlaySkeletalMesh":
            continue
        typed = cue.get("typedPayload") or {}
        result.append({
            "sourceCueId": str(cue.get("cueId") or ""),
            "sourceTimeSeconds": float(cue.get("globalTimeSeconds", 0.0)),
            "sourceCueName": str(typed.get("sourceCueName") or ""),
            "sourceSkeletalMesh": str(typed.get("sourceSkeletalMesh") or ""),
            "sourceAnimSet": str(typed.get("sourceAnimSet") or ""),
            "sourceMaterialInstances": list(
                typed.get("sourceMaterialInstances", [])
            ),
            "sourceExecutionStatus": str(
                cue.get("sourceExecutionStatus") or ""
            ),
        })
    return result


def build_skeletal_cue_rows(
    skill_id: int,
    recipe: dict[str, Any],
    authored_model_cues: list[dict[str, Any]],
    bindings: list[dict[str, Any]],
    resource_root: Path,
) -> list[dict[str, Any]]:
    rows = []
    bindings_for_skill = [
        row for row in bindings if int(row.get("skillId", -1)) == skill_id
    ]
    sources = source_skeletal_cues(recipe)
    for source in sources:
        matches = []
        for binding in bindings_for_skill:
            identity = binding.get("source") or {}
            if (
                str(identity.get("cueName") or "").casefold()
                == source["sourceCueName"].casefold()
                and str(identity.get("skeletalMesh") or "").casefold()
                == source["sourceSkeletalMesh"].casefold()
                and str(identity.get("animSet") or "").casefold()
                == source["sourceAnimSet"].casefold()
            ):
                matches.append(binding)
        if len(matches) > 1:
            raise ValueError(f"duplicate Model Cue runtime binding for skill {skill_id}")
        if not matches:
            rows.append({
                **source,
                "classification": "UNBOUND_SOURCE_MODEL_CUE",
                "runtime": None,
            })
            continue
        runtime = matches[0].get("runtime") or {}
        authored_matches = [
            cue for cue in authored_model_cues
            if (
                str(cue.get("cueId") or "").casefold()
                == str(runtime.get("cueId") or "").casefold()
                and str(cue.get("modelAssetId") or "").casefold()
                == str(runtime.get("modelAssetId") or "").casefold()
                and str(cue.get("clipName") or "").casefold()
                == str(runtime.get("clipName") or "").casefold()
            )
        ]
        if len(authored_matches) != 1:
            raise ValueError(
                f"bound source Model Cue is missing from Authored skill {skill_id}"
            )
        asset_id = str(runtime.get("modelAssetId") or "")
        asset_path = resource_root / Path(asset_id)
        if not asset_path.is_file():
            raise ValueError(f"bound Model Cue asset is missing: {asset_id}")
        rows.append({
            **source,
            "classification": "BOUND_SKELETAL_MODEL_CUE",
            "runtime": {
                "cueId": str(runtime.get("cueId") or ""),
                "modelAssetId": asset_id,
                "clipName": str(runtime.get("clipName") or ""),
                "assetPreTransform": runtime.get("assetPreTransform") or {},
                "assetExists": True,
            },
        })
    if authored_model_cues and len(rows) != len(authored_model_cues):
        raise ValueError(
            f"Authored Model Cue has no exact source cue for skill {skill_id}"
        )
    return rows


def build_renderer_family_audit(
    admitted_skills: list[dict[str, Any]],
    authored_root: Path,
    converted_root: Path,
    model_bindings_path: Path,
    resource_root: Path,
) -> dict[str, Any]:
    bindings_document = read_json(model_bindings_path)
    if bindings_document.get("schema") != (
        "lostark.effect-model-cue-runtime-bindings"
    ):
        raise ValueError("invalid Model Cue runtime bindings schema")
    bindings = list(bindings_document.get("bindings", []))
    result_skills = []
    sources = [{
        "path": model_bindings_path.as_posix(),
        "sha256": sha256_file(model_bindings_path),
    }]
    total = Counter()
    for skill in admitted_skills:
        skill_id = int(skill["skillId"])
        input_slot = str(skill.get("inputSlot") or "").upper()
        effect_asset_id = str(skill["effectAssetId"])
        authored_path = authored_root / f"{effect_asset_id}.effect.json"
        recipe_path = converted_root / f"skill.{skill_id}.action-cue-recipe.json"
        document = read_json(authored_path)
        recipe = read_json(recipe_path)
        validate_document_identity(document, skill_id, effect_asset_id)
        if (
            recipe.get("schema") != "lostark.effect-action-cue-recipe"
            or int(recipe.get("skillId", -1)) != skill_id
        ):
            raise ValueError(f"Action Cue recipe identity mismatch for skill {skill_id}")
        sources.extend([
            {"path": authored_path.as_posix(), "sha256": sha256_file(authored_path)},
            {"path": recipe_path.as_posix(), "sha256": sha256_file(recipe_path)},
        ])

        family_counts = Counter()
        mesh_elements: list[tuple[dict[str, Any], str]] = []
        for element in document.get("elements", []):
            kind = str(element.get("kind") or "").casefold()
            if kind == "particle":
                classification = classify_cascade_layer(element)
                family_counts[classification] += 1
                if classification.startswith("STATIC_MESH_"):
                    mesh_elements.append((element, classification))
                    asset_id = resources_by_slot(element)[MESH_MODEL_SLOT]
                    if not (resource_root / Path(asset_id)).is_file():
                        raise ValueError(f"static Effect Mesh asset is missing: {asset_id}")
                continue
            typed_name = {
                "light": "LIGHT",
                "screenpost": "SCREEN_POST",
                "decal": "DECAL",
            }.get(kind)
            if typed_name is None:
                raise ValueError(f"unsupported Effect Element kind: {kind}")
            family_counts[typed_name] += 1

        skeletal_rows = build_skeletal_cue_rows(
            skill_id,
            recipe,
            list(document.get("modelCues", [])),
            bindings,
            resource_root,
        )
        family_counts.update(row["classification"] for row in skeletal_rows)
        model_material_cue_count = sum(
            1 for cue in recipe.get("cues", [])
            if cue.get("sourceType") == "PlaySkeletalMeshMaterialParam"
        )
        repeated_groups = build_repeated_mesh_groups(mesh_elements)
        result_skills.append({
            "skillId": skill_id,
            "inputSlot": input_slot,
            "effectAssetId": effect_asset_id,
            "familyCounts": dict(sorted(family_counts.items())),
            "staticMeshGroups": build_mesh_groups(mesh_elements),
            "repeatedStaticMeshGroups": repeated_groups,
            "skeletalModelCues": skeletal_rows,
            "sourceModelMaterialCueCount": model_material_cue_count,
            "summary": {
                "elementCount": len(document.get("elements", [])),
                "staticMeshLayerCount": sum(
                    value for key, value in family_counts.items()
                    if key.startswith("STATIC_MESH_")
                ),
                "spriteLayerCount": sum(
                    value for key, value in family_counts.items()
                    if key.startswith("SPRITE_")
                ),
                "uniqueStaticMeshAssetCount": len({
                    resources_by_slot(element)[MESH_MODEL_SLOT]
                    for element, _classification in mesh_elements
                }),
                "repeatedStaticMeshGroupCount": len(repeated_groups),
            },
        })
        total.update(family_counts)

    static_count = sum(
        value for key, value in total.items() if key.startswith("STATIC_MESH_")
    )
    sprite_count = sum(
        value for key, value in total.items() if key.startswith("SPRITE_")
    )
    return {
        "schema": "lostark.dimensionmaster-renderer-family-audit",
        "formatVersion": 1,
        "characterClass": "DIMENSIONMASTER",
        "scope": "BASE11",
        "rendererFamilyContract": {
            "skeletalModelCue": (
                "Bone-deformed WModel clip admitted by an exact PlaySkeletalMesh "
                "source cue and runtime binding."
            ),
            "staticMeshEffectInstance": (
                "Static WModel geometry whose instance transform, lifetime, and "
                "Material parameters are evaluated by the Cascade executor."
            ),
            "spriteProceduralEffectLayer": (
                "Camera-facing or oriented card whose silhouette is produced by "
                "texture/SubUV or a finite procedural Material profile."
            ),
        },
        "summary": {
            "skillCount": len(result_skills),
            "cascadeLayerCount": static_count + sprite_count,
            "staticMeshLayerCount": static_count,
            "spriteLayerCount": sprite_count,
            "staticMeshMaterialOverrideCarrierCount": total[
                "STATIC_MESH_MATERIAL_OVERRIDE_CARRIER"
            ],
            "staticMeshEmbeddedMaterialCandidateCount": total[
                "STATIC_MESH_EMBEDDED_MATERIAL_CANDIDATE"
            ],
            "staticMeshContractContradictionCount": total[
                "STATIC_MESH_CONTRACT_CONTRADICTION"
            ],
            "boundSkeletalModelCueCount": total["BOUND_SKELETAL_MODEL_CUE"],
            "unboundSourceModelCueCount": total["UNBOUND_SOURCE_MODEL_CUE"],
            "lightCount": total["LIGHT"],
            "screenPostCount": total["SCREEN_POST"],
            "decalCount": total["DECAL"],
        },
        "skills": result_skills,
        "sources": sources,
    }


def selected_base11_skills(
    player_skills_path: Path, skill_bindings_path: Path,
) -> list[dict[str, Any]]:
    admitted = dimensionmaster_admitted_skills(
        player_skills_path, skill_bindings_path
    )
    selected = [
        row for row in admitted
        if str(row.get("inputSlot") or "").upper() in BASE11_SLOTS
        and str(row.get("inputSlot") or "").upper() != "ALT_V"
    ]
    if len(selected) != 11:
        raise ValueError(
            f"DimensionMaster BASE11 admission must contain 11 skills, got {len(selected)}"
        )
    return selected


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--player-skills", type=Path,
        default=Path("Data/Balance/PlayerSkills.json"),
    )
    parser.add_argument(
        "--skill-bindings", type=Path,
        default=Path(
            "Data/Animation/Authored/DimensionMaster/"
            "DimensionMaster.skillbindings.json"
        ),
    )
    parser.add_argument(
        "--authored-root", type=Path,
        default=Path("Data/Effects/Authored"),
    )
    parser.add_argument(
        "--converted-root", type=Path,
        default=Path("Data/Effects/Imported/DimensionMaster/Converted"),
    )
    parser.add_argument(
        "--model-bindings", type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.model-cue-runtime-bindings.json"
        ),
    )
    parser.add_argument(
        "--resource-root", type=Path,
        default=Path("Client/Bin/Resources"),
    )
    parser.add_argument(
        "--output", type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.renderer-family-audit.json"
        ),
    )
    args = parser.parse_args()
    audit = build_renderer_family_audit(
        selected_base11_skills(args.player_skills, args.skill_bindings),
        args.authored_root,
        args.converted_root,
        args.model_bindings,
        args.resource_root,
    )
    write_json_atomic(args.output, audit)
    print(json.dumps(audit["summary"], ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
