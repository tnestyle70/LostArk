#!/usr/bin/env python3
"""Materialize admitted DimensionMaster Effect documents for runtime.

Imported UE3 documents may include extraction candidates that are not current
gameplay skills.  Only the intersection admitted by PlayerSkills and the
DimensionMaster skillbindings can update canonical Authored documents.
"""

from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path
from typing import Any

from build_dimensionmaster_base_effects import dimensionmaster_admitted_skills
from build_effect_source_material_contract import (
    build_contract as build_source_material_contract,
    upgrade_effect_document,
)
from build_imported_effect_documents import read_json, write_json_atomic


def canonicalize(document: dict[str, Any], effect_id: str) -> dict[str, Any]:
    result = copy.deepcopy(document)
    result["effectAssetId"] = effect_id
    return result


def restore_mesh_material_override_contract(
    document: dict[str, Any],
) -> int:
    corrected = 0
    for element in document.get("elements", []):
        if str(element.get("kind") or "").casefold() != "particle":
            continue
        source_recipe = element.get("sourceRecipe") or {}
        if str(source_recipe.get("rendererShape") or "").casefold() != "mesh":
            continue
        mesh_modules = [
            module for module in source_recipe.get("modules", [])
            if str(module.get("className") or "").casefold()
            == "particlemoduletypedatamesh"
        ]
        if len(mesh_modules) != 1:
            raise ValueError(
                "Cascade mesh Element must preserve exactly one TypeDataMesh: "
                f"{element.get('id')}"
            )
        override_values = []
        for literal in mesh_modules[0].get("literals", []):
            if str(literal.get("propertyPath") or "").casefold() != (
                "boverridematerial"
            ):
                continue
            value = literal.get("value")
            if not isinstance(value, bool):
                raise ValueError(
                    "TypeDataMesh.bOverrideMaterial must be boolean: "
                    f"{element.get('id')}"
                )
            override_values.append(value)
        if any(value != override_values[0] for value in override_values[1:]):
            raise ValueError(
                "conflicting TypeDataMesh.bOverrideMaterial values: "
                f"{element.get('id')}"
            )
        override_material = override_values[0] if override_values else False
        use_model_material = not override_material
        detail = element.get("detail") or {}
        mesh_detail = detail.get("mesh")
        if not isinstance(mesh_detail, dict):
            raise ValueError(
                f"Cascade mesh Element has no mesh Detail: {element.get('id')}"
            )
        if bool(mesh_detail.get("useModelMaterial")) != use_model_material:
            mesh_detail["useModelMaterial"] = use_model_material
            corrected += 1
    return corrected


def build_combo_stage_document(
    aggregate: dict[str, Any],
    effect_id: str,
    display_name: str,
    clip_offset: float,
    next_clip_offset: float | None,
) -> dict[str, Any]:
    result = canonicalize(aggregate, effect_id)
    result["displayName"] = display_name
    selected = []
    for source in aggregate.get("elements", []):
        start = float(source["detail"]["timing"]["startDelaySeconds"])
        if start + 1.0e-5 < clip_offset:
            continue
        if next_clip_offset is not None and start >= next_clip_offset - 1.0e-5:
            continue
        element = copy.deepcopy(source)
        element["detail"]["timing"]["startDelaySeconds"] = max(
            0.0, start - clip_offset
        )
        selected.append(element)
    if not selected:
        raise ValueError(f"{effect_id} has no source-derived elements")
    result["elements"] = selected
    result["modelCues"] = []
    return result


def source_confirmed_model_cues(
    skill_id: int,
    action_recipe: dict[str, Any],
    summon_retime: dict[str, Any],
    model_cue_bindings: dict[str, Any],
) -> list[dict[str, Any]]:
    source_model_cues = [
        cue for cue in action_recipe.get("cues", [])
        if cue.get("runtimeChannel") == "MODEL_CUE"
        and cue.get("executionEnabled", False)
    ]
    bindings = [
        row for row in model_cue_bindings.get("bindings", [])
        if int(row.get("skillId", -1)) == skill_id
    ]
    if not bindings:
        return []
    if not source_model_cues:
        raise ValueError(
            f"skill {skill_id} has a runtime Model binding but no source cue"
        )
    retime_by_clip = {
        (str(row["sourceClip"]), str(row["runtimeClip"])): row
        for row in summon_retime.get("clips", [])
    }
    result = []
    consumed_cues: set[str] = set()
    for binding in bindings:
        source = binding.get("source", {})
        runtime = binding.get("runtime", {})
        source_materials = list(source.get("materialInstances", []))
        matched = []
        for cue in source_model_cues:
            typed = cue.get("typedPayload", {})
            if (
                str(typed.get("sourceCueName") or "")
                == str(source.get("cueName") or "")
                and str(typed.get("sourceSkeletalMesh") or "")
                == str(source.get("skeletalMesh") or "")
                and str(typed.get("sourceAnimSet") or "")
                == str(source.get("animSet") or "")
                and list(typed.get("sourceMaterialInstances", []))
                == source_materials
            ):
                matched.append(cue)
        if len(matched) != 1:
            raise ValueError(
                f"skill {skill_id} Model binding matched {len(matched)} cues"
            )
        cue = matched[0]
        cue_id = str(cue.get("cueId") or "")
        if cue_id in consumed_cues:
            raise ValueError(f"skill {skill_id} Model cue was bound twice: {cue_id}")
        consumed_cues.add(cue_id)
        typed = cue.get("typedPayload", {})
        if not typed.get("transformDecoded", False):
            raise ValueError(
                f"skill {skill_id} Model cue transform is not decoded: {cue_id}"
            )
        source_clip = str(source.get("animationClip") or "")
        runtime_clip = str(runtime.get("clipName") or "")
        retime = retime_by_clip.get((source_clip, runtime_clip))
        if retime is None:
            raise ValueError(
                f"skill {skill_id} Model clip has no source-rate receipt: "
                f"{source_clip}/{runtime_clip}"
            )
        result.append({
            "cueId": str(runtime.get("cueId") or ""),
            "modelAssetId": str(runtime.get("modelAssetId") or ""),
            "clipName": runtime_clip,
            "startDelaySeconds": float(cue.get("globalTimeSeconds", 0.0)),
            "durationSeconds": float(retime["durationSeconds"]),
            "visible": True,
            "localTransform": copy.deepcopy(typed["localTransform"]),
            "assetPreTransform": copy.deepcopy(
                runtime.get("assetPreTransform", {})
            ),
        })
    if len(consumed_cues) != len(source_model_cues):
        raise ValueError(
            f"skill {skill_id} has unbound source Model cues: "
            f"{len(source_model_cues) - len(consumed_cues)}"
        )
    return result


def unsupported_model_material_cues(
    action_recipe: dict[str, Any],
) -> list[str]:
    return [
        str(cue.get("cueId") or "")
        for cue in action_recipe.get("cues", [])
        if cue.get("runtimeChannel") == "MODEL_MATERIAL"
        and cue.get("executionEnabled", False)
        and not cue.get("typedPayload", {}).get("semanticDecoded", False)
    ]


def materializable_extraction_rows(
    extraction: dict[str, Any],
    admitted_skills: list[dict[str, Any]],
) -> tuple[list[tuple[dict[str, Any], dict[str, Any]]], list[dict[str, Any]]]:
    rows_by_skill = {
        int(row["skillId"]): row for row in extraction.get("skills", [])
    }
    selected = []
    missing = []
    for skill in admitted_skills:
        skill_id = int(skill["skillId"])
        source = rows_by_skill.get(skill_id)
        if source is None:
            missing.append({
                "skillId": skill_id,
                "effectAssetId": str(skill["effectAssetId"]),
                "inputSlot": skill.get("inputSlot"),
                "status": "MISSING_EXTRACTED_SOURCE",
            })
            continue
        if str(source.get("effectAssetId") or "") != str(skill["effectAssetId"]):
            raise ValueError(
                "extracted/canonical Effect identity mismatch: "
                f"{source.get('effectAssetId')} != {skill['effectAssetId']}"
            )
        selected.append((skill, source))
    return selected, missing


def select_admitted_skills(
    admitted_skills: list[dict[str, Any]],
    requested_skill_ids: set[int] | None,
) -> list[dict[str, Any]]:
    """Restrict a materialization staging run without inventing new skills."""
    if not requested_skill_ids:
        return admitted_skills
    admitted_ids = {int(row["skillId"]) for row in admitted_skills}
    unknown = sorted(requested_skill_ids - admitted_ids)
    if unknown:
        raise ValueError(
            "--skill-id is not an admitted DimensionMaster skill: "
            f"{unknown}"
        )
    return [
        row for row in admitted_skills
        if int(row["skillId"]) in requested_skill_ids
    ]


def reconstruct_source_material_profiles(
    skill_id: int,
    imported_root: Path,
    imported_document: dict[str, Any],
    resource_manifest: dict[str, Any],
    material_evidence_path: Path | None = None,
    material_graph_evidence_path: Path | None = None,
    texture_sampling_evidence_path: Path | None = None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    """Rebuild v12 profiles from source evidence, never an old Authored file."""
    source_receipt = read_json(
        imported_root / f"skill.{skill_id}.source-receipt.json"
    )
    if (
        source_receipt.get("schema") != "lostark.effect-source-receipt"
        or int(source_receipt.get("skillId", -1)) != skill_id
        or str(source_receipt.get("characterClass") or "").upper()
        != "DIMENSIONMASTER"
    ):
        raise ValueError(
            f"skill {skill_id} source material receipt identity is invalid"
        )
    conversion_receipt = read_json(
        imported_root / "Converted" /
        f"skill.{skill_id}.element-conversion-receipt.json"
    )
    # The checked-in evidence map contains only UModel-exported parent props
    # with hashes. It may refine a source receipt, while host-local catalogs
    # and old Authored documents remain forbidden as fallback sources.
    evidence_path = material_evidence_path or (
        imported_root / "ActionSource" /
        "DimensionMaster.source-material-evidence.json"
    )
    material_evidence = (
        read_json(evidence_path) if evidence_path.is_file()
        else {"materials": {}}
    )
    if material_evidence.get("schema") and (
        material_evidence.get("schema")
        != "lostark.effect-source-material-evidence"
    ):
        raise ValueError("DimensionMaster source Material evidence is invalid")
    if material_evidence.get("schema") and str(
        material_evidence.get("characterClass") or ""
    ).upper() not in {"", "DIMENSIONMASTER"}:
        raise ValueError("source Material evidence character class is invalid")
    graph_evidence_path = material_graph_evidence_path or (
        imported_root / "ActionSource" /
        "DimensionMaster.parent-material-graph-evidence.json"
    )
    material_graph_evidence = (
        read_json(graph_evidence_path) if graph_evidence_path.is_file() else {}
    )
    if material_graph_evidence and (
        material_graph_evidence.get("schema")
        != "lostark.ue3-cooked-material-graph-evidence-set"
        or int(material_graph_evidence.get("formatVersion", 0)) != 1
    ):
        raise ValueError("DimensionMaster parent Material graph evidence is invalid")
    sampling_evidence_path = texture_sampling_evidence_path or (
        imported_root / "ActionSource" /
        "DimensionMaster.texture-sampling-evidence.json"
    )
    texture_sampling_evidence = (
        read_json(sampling_evidence_path)
        if sampling_evidence_path.is_file() else {}
    )
    if texture_sampling_evidence and (
        texture_sampling_evidence.get("schema")
        != "lostark.ue3-texture-sampling-evidence"
        or int(texture_sampling_evidence.get("formatVersion", 0)) != 1
    ):
        raise ValueError("DimensionMaster texture sampling evidence is invalid")
    contract, receipt = build_source_material_contract(
        imported_document,
        source_receipt,
        conversion_receipt,
        resource_manifest,
        material_evidence,
        Path(__file__).resolve().parents[2] / "Client" / "Bin" / "Resources",
        material_graph_evidence,
        texture_sampling_evidence,
    )
    if receipt.get("failures"):
        raise ValueError(
            f"skill {skill_id} source material reconstruction failed: "
            f"{receipt['failures']}"
        )
    upgraded = upgrade_effect_document(imported_document, contract)
    particle_elements = [
        row for row in upgraded.get("elements", [])
        if str(row.get("kind") or "").casefold() == "particle"
    ]
    enabled_profiles = [
        row for row in particle_elements
        if bool(
            (row.get("material") or {})
            .get("sourceProfile", {})
            .get("enabled")
        )
    ]
    if (
        int(upgraded.get("version", 0)) != 12
        or len(enabled_profiles) != len(particle_elements)
    ):
        raise ValueError(
            f"skill {skill_id} source material v12 coverage is incomplete"
        )
    return upgraded, receipt


def synchronize_effect_catalog(
    catalog_path: Path,
    documents: list[dict[str, Any]],
) -> dict[str, int]:
    """Replace only the admitted DimensionMaster rows in the product catalog."""
    catalog = read_json(catalog_path)
    if (
        catalog.get("formatVersion") != 1
        or not isinstance(catalog.get("effects"), list)
    ):
        raise ValueError("Effect catalog contract is invalid")
    prefix = "effect.dimensionmaster.skill."
    preserved = [
        copy.deepcopy(row)
        for row in catalog["effects"]
        if not str(row.get("effectAssetId") or "").startswith(prefix)
    ]
    canonical = []
    seen: set[str] = set()
    for row in documents:
        effect_id = str(row.get("effectAssetId") or "")
        if not effect_id.startswith(prefix) or effect_id in seen:
            raise ValueError(f"invalid materialized Effect identity: {effect_id}")
        seen.add(effect_id)
        canonical.append({
            "effectAssetId": effect_id,
            "authoringPath": f"Effects/Authored/{effect_id}.effect.json",
        })

    def order(row: dict[str, str]) -> tuple[int, int]:
        identity = row["effectAssetId"].removeprefix(prefix)
        base, separator, stage = identity.partition(".ba")
        return int(base), int(stage) if separator else 0

    canonical.sort(key=order)
    catalog["effects"] = preserved + canonical
    write_json_atomic(catalog_path, catalog)
    return {
        "preservedEffectCount": len(preserved),
        "dimensionMasterEffectCount": len(canonical),
    }


def materialize(
    imported_root: Path,
    authored_root: Path,
    extraction_receipt_path: Path,
    summon_retime_path: Path,
    model_cue_bindings_path: Path,
    player_skills_path: Path = Path("Data/Balance/PlayerSkills.json"),
    skill_bindings_path: Path = Path(
        "Data/Animation/Authored/DimensionMaster/"
        "DimensionMaster.skillbindings.json"
    ),
    catalog_path: Path | None = None,
    resource_manifest_path: Path | None = None,
    material_evidence_path: Path | None = None,
    material_graph_evidence_path: Path | None = None,
    requested_skill_ids: set[int] | None = None,
    texture_sampling_evidence_path: Path | None = None,
) -> dict[str, Any]:
    extraction = read_json(extraction_receipt_path)
    all_admitted_skills = dimensionmaster_admitted_skills(
        player_skills_path, skill_bindings_path
    )
    admitted_skills = select_admitted_skills(
        all_admitted_skills, requested_skill_ids
    )
    selected_rows, missing_sources = materializable_extraction_rows(
        extraction, admitted_skills
    )
    incomplete_payloads = [
        int(source["skillId"])
        for _skill, source in selected_rows
        if not source.get("actionPayloadCaptureComplete")
    ]
    if incomplete_payloads:
        raise ValueError(
            "admitted Action source capture is incomplete: "
            f"{incomplete_payloads}"
        )
    summon_retime = read_json(summon_retime_path)
    if not summon_retime.get("sourceRateRestorationComplete"):
        raise ValueError("summon source animation rate restoration is incomplete")
    model_cue_bindings = read_json(model_cue_bindings_path)
    if (
        model_cue_bindings.get("schema")
        != "lostark.effect-model-cue-runtime-bindings"
        or int(model_cue_bindings.get("formatVersion", 0)) != 1
        or str(model_cue_bindings.get("characterClass") or "")
        != "DIMENSIONMASTER"
    ):
        raise ValueError("DimensionMaster Model Cue binding contract is invalid")
    if resource_manifest_path is None:
        resource_manifest_path = (
            imported_root / "DimensionMaster.resource-source-manifest.json"
        )
    resource_manifest = read_json(resource_manifest_path)
    if (
        resource_manifest.get("schema")
        != "lostark.class-effect-resource-source-manifest"
        or str(resource_manifest.get("characterClass") or "").upper()
        != "DIMENSIONMASTER"
    ):
        raise ValueError("DimensionMaster resource source manifest is invalid")

    authored_root.mkdir(parents=True, exist_ok=True)
    written = []
    aggregates: dict[int, dict[str, Any]] = {}
    for skill, _source_row in selected_rows:
        skill_id = int(skill["skillId"])
        imported_path = imported_root / "Converted" / (
            f"effect.dimensionmaster.skill.{skill_id}.imported.effect.json"
        )
        action_path = imported_root / "Converted" / (
            f"skill.{skill_id}.action-cue-recipe.json"
        )
        imported = read_json(imported_path)
        imported, source_material_receipt = reconstruct_source_material_profiles(
            skill_id, imported_root, imported, resource_manifest,
            material_evidence_path, material_graph_evidence_path,
            texture_sampling_evidence_path,
        )
        mesh_material_contract_correction_count = (
            restore_mesh_material_override_contract(imported)
        )
        action = read_json(action_path)
        canonical_id = str(skill["effectAssetId"])
        authored_path = authored_root / f"{canonical_id}.effect.json"
        aggregate = canonicalize(imported, canonical_id)
        aggregate["modelCues"] = source_confirmed_model_cues(
            skill_id, action, summon_retime, model_cue_bindings
        )
        unsupported_model_material = unsupported_model_material_cues(action)
        write_json_atomic(authored_path, aggregate)
        aggregates[skill_id] = aggregate
        written.append({
            "effectAssetId": canonical_id,
            "path": str(authored_path),
            "elementCount": len(aggregate.get("elements", [])),
            "modelCueCount": len(aggregate.get("modelCues", [])),
            "unsupportedModelMaterialCueCount": len(
                unsupported_model_material
            ),
            "unsupportedModelMaterialCueIds": unsupported_model_material,
            "sourceMaterialProfileSummary": copy.deepcopy(
                source_material_receipt["summary"]
            ),
            "meshMaterialContractCorrectionCount": (
                mesh_material_contract_correction_count
            ),
            "role": "AGGREGATE",
        })

    combo_skills = [
        row for row in admitted_skills
        if str(row.get("skillKind") or "").upper() == "COMBO"
    ]
    combo_stage_count = 0
    if len(combo_skills) > 1:
        raise ValueError("DimensionMaster has multiple admitted combo Effects")
    if combo_skills:
        combo = combo_skills[0]
        combo_skill_id = int(combo["skillId"])
        if combo_skill_id in aggregates:
            action = read_json(
                imported_root / "Converted" /
                f"skill.{combo_skill_id}.action-cue-recipe.json"
            )
            stages = action.get("selectedStages", [])
            expected_count = len(combo.get("comboStages", []))
            if not expected_count:
                raise ValueError("admitted combo Effect has no gameplay stages")
            if len(combo.get("clips", [])) != expected_count:
                raise ValueError(
                    "combo skillbinding/stage count mismatch: "
                    f"{len(combo.get('clips', []))} != {expected_count}"
                )
            if len(stages) != expected_count:
                raise ValueError(
                    f"BA Action/stage contract mismatch: "
                    f"{len(stages)} != {expected_count}"
                )
            for index, stage in enumerate(stages):
                offset = float(stage["clipOffsetSeconds"])
                next_offset = (
                    float(stages[index + 1]["clipOffsetSeconds"])
                    if index + 1 < len(stages) else None
                )
                effect_id = f"{combo['effectAssetId']}.ba{index + 1}"
                document = build_combo_stage_document(
                    aggregates[combo_skill_id], effect_id,
                    f"{combo['displayName']} BA{index + 1}",
                    offset, next_offset
                )
                path = authored_root / f"{effect_id}.effect.json"
                write_json_atomic(path, document)
                written.append({
                    "effectAssetId": effect_id,
                    "path": str(path),
                    "elementCount": len(document["elements"]),
                    "modelCueCount": 0,
                    "role": "COMBO_STAGE",
                    "clip": stage["clip"],
                    "sourceOffsetSeconds": offset,
                })
            combo_stage_count = len(stages)

    unsupported_model_material_count = sum(
        int(row.get("unsupportedModelMaterialCueCount", 0))
        for row in written
    )
    catalog_result = (
        synchronize_effect_catalog(catalog_path, written)
        if catalog_path is not None and not requested_skill_ids else None
    )
    return {
        "schema": "lostark.dimensionmaster-base-effect-materialization-receipt",
        "formatVersion": 1,
        "variantContract": "BASE_NO_TIME_OR_SPACE_AXIS",
        "canonicalSkillCount": len(admitted_skills),
        "admittedCanonicalSkillCount": len(all_admitted_skills),
        "selectedSkillIds": sorted(requested_skill_ids or []),
        "aggregateSkillCount": len(aggregates),
        "comboStageDocumentCount": combo_stage_count,
        "missingCanonicalSources": missing_sources,
        "unsupportedModelMaterialCueCount": unsupported_model_material_count,
        "runtimeModelMaterialExecutionComplete": (
            unsupported_model_material_count == 0
        ),
        "sourceMaterialProfileReconstructionComplete": all(
            int(row.get("sourceMaterialProfileSummary", {}).get(
                "failureCount", -1
            )) == 0
            for row in written if row.get("role") == "AGGREGATE"
        ),
        "catalogSynchronization": catalog_result,
        "catalogSynchronizationSkippedForSelection": bool(
            catalog_path is not None and requested_skill_ids
        ),
        "runtimeExecutionComplete": bool(extraction.get("runtimeExecutionComplete")),
        "documents": written,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--imported-root", type=Path,
        default=Path("Data/Effects/Imported/DimensionMaster")
    )
    parser.add_argument(
        "--authored-root", type=Path,
        default=Path("Data/Effects/Authored")
    )
    parser.add_argument(
        "--extraction-receipt", type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.base-effect-extraction.receipt.json"
        )
    )
    parser.add_argument(
        "--summon-retime", type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.summon-animation-retime.receipt.json"
        )
    )
    parser.add_argument(
        "--model-cue-bindings", type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.model-cue-runtime-bindings.json"
        )
    )
    parser.add_argument(
        "--player-skills", type=Path,
        default=Path("Data/Balance/PlayerSkills.json")
    )
    parser.add_argument(
        "--skill-bindings", type=Path,
        default=Path(
            "Data/Animation/Authored/DimensionMaster/"
            "DimensionMaster.skillbindings.json"
        )
    )
    parser.add_argument(
        "--receipt", type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.base-effect-materialization.receipt.json"
        )
    )
    parser.add_argument(
        "--catalog", type=Path,
        default=Path("Data/Effects/EffectCatalog.json")
    )
    parser.add_argument(
        "--resource-manifest", type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.resource-source-manifest.json"
        )
    )
    parser.add_argument(
        "--material-evidence", type=Path,
        help=(
            "Optional UModel parent-props evidence map. Use a staged path "
            "with --skill-id; default is the checked-in ActionSource map."
        ),
    )
    parser.add_argument(
        "--material-graph-evidence", type=Path,
        help=(
            "Optional direct cooked-UPK parent graph evidence. The default "
            "is the checked-in ActionSource evidence set."
        ),
    )
    parser.add_argument(
        "--texture-sampling-evidence", type=Path,
        help=(
            "Optional direct cooked-UPK texture address/sRGB evidence. The "
            "default is the checked-in ActionSource evidence set."
        ),
    )
    parser.add_argument(
        "--skill-id", type=int, action="append", default=[],
        help=(
            "Materialize only admitted skill IDs for staging. Catalog "
            "synchronization is skipped for a selected run."
        ),
    )
    args = parser.parse_args()
    if args.skill_id:
        if args.authored_root == Path("Data/Effects/Authored"):
            parser.error("--skill-id staging requires a non-default --authored-root")
        if args.receipt == Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.base-effect-materialization.receipt.json"
        ):
            parser.error("--skill-id staging requires a non-default --receipt")
    receipt = materialize(
        args.imported_root, args.authored_root,
        args.extraction_receipt, args.summon_retime,
        args.model_cue_bindings,
        args.player_skills, args.skill_bindings,
        args.catalog, args.resource_manifest,
        args.material_evidence, args.material_graph_evidence,
        set(args.skill_id) or None,
        args.texture_sampling_evidence,
    )
    write_json_atomic(args.receipt, receipt)
    print(json.dumps({
        "canonicalSkillCount": receipt["canonicalSkillCount"],
        "aggregateSkillCount": receipt["aggregateSkillCount"],
        "comboStageDocumentCount": receipt["comboStageDocumentCount"],
        "runtimeExecutionComplete": receipt["runtimeExecutionComplete"],
        "receipt": str(args.receipt),
    }, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
