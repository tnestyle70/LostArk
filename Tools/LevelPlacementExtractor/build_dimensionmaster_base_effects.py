#!/usr/bin/env python3
"""Build the base DimensionMaster skill Effect recipes and one fail-closed audit."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
from typing import Any

from build_action_cue_recipe import build_action_cue_recipe
from build_imported_effect_documents import build_document, read_json, write_json_atomic


def apply_base_variant_contract(
    source_receipt: dict[str, Any], action_recipe: dict[str, Any]
) -> tuple[dict[str, Any], dict[str, int]]:
    result = copy.deepcopy(source_receipt)
    events = list(result.get("timeline", {}).get("events", []))
    disabled_indices = {
        int(cue["sourceReceiptEventIndex"])
        for cue in action_recipe.get("cues", [])
        if cue.get("sourceType", "").casefold() == "playparticleeffect"
        and not bool(cue.get("executionEnabled", True))
        and cue.get("sourceReceiptEventIndex") is not None
    }
    enabled_indices = {
        int(cue["sourceReceiptEventIndex"])
        for cue in action_recipe.get("cues", [])
        if cue.get("sourceType", "").casefold() == "playparticleeffect"
        and bool(cue.get("executionEnabled", True))
        and cue.get("sourceReceiptEventIndex") is not None
    }
    if disabled_indices & enabled_indices:
        raise ValueError("Action cue enabled/disabled reference join overlaps")
    expected_particle_indices = {
        index
        for index, event in enumerate(events)
        if str(event.get("sourceType") or "").casefold()
        == "playparticleeffect"
    }
    if expected_particle_indices != disabled_indices | enabled_indices:
        missing = sorted(
            expected_particle_indices - disabled_indices - enabled_indices
        )
        raise ValueError(
            "PlayParticleEffect variant join is incomplete: "
            f"missing source event indices {missing}"
        )
    enabled_cues_by_index = {
        int(cue["sourceReceiptEventIndex"]): cue
        for cue in action_recipe.get("cues", [])
        if cue.get("sourceType", "").casefold() == "playparticleeffect"
        and bool(cue.get("executionEnabled", True))
        and cue.get("sourceReceiptEventIndex") is not None
    }
    filtered_events = []
    for index, event in enumerate(events):
        if index in disabled_indices:
            continue
        staged_event = copy.deepcopy(event)
        action_cue = enabled_cues_by_index.get(index)
        if action_cue is not None:
            staged_event["sourceActionCueId"] = action_cue["cueId"]
            staged_event["actionCuePayload"] = copy.deepcopy(
                action_cue["typedPayload"]
            )
        filtered_events.append(staged_event)
    result["timeline"]["events"] = filtered_events
    return result, {
        "sourcePlayParticleEffectCount": len(expected_particle_indices),
        "enabledPlayParticleEffectCount": len(enabled_indices),
        "disabledPlayParticleEffectCount": len(disabled_indices),
        "disabledNotifyExecutionCount": 0,
    }


# These classes have a real execution path in CEffectPlayback, but the path is
# still incomplete (for example duplicate module order, world-space flags,
# camera basis, and UE3 seeded distributions).  They must never be reported as
# EXACT until the module contract harness proves source/runtime parity.
PARTIAL_RUNTIME_MODULE_CLASSES = {
    "particlemoduleacceleration",
    "particlemodulecameraoffset",
    "particlemodulecolor",
    "particlemodulecolor_seeded",
    "particlemodulecoloroverlife",
    "particlemodulecolorscaleoverlife",
    "particlemodulelifetime",
    "particlemodulelifetime_seeded",
    "particlemodulelocation",
    "particlemodulelocation_seeded",
    "particlemodulelocationdirect",
    "particlemodulemeshrotation",
    "particlemodulemeshrotation_seeded",
    "particlemodulemeshrotationrate",
    "particlemodulemeshrotationrate_seeded",
    "particlemodulemeshrotationratemultiplylife",
    "particlemodulerotation",
    "particlemodulerotation_seeded",
    "particlemodulerotationrate",
    "particlemodulesize",
    "particlemodulesize_seeded",
    "particlemodulesizemultiplylife",
    "particlemodulespawn",
    "particlemodulevelocity",
    "particlemodulevelocity_seeded",
    "particlemodulevelocityoverlifetime",
    "efparticlemoduleacceleration",
    "efparticlemodulevelocityoverlifetime",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def presentation_slot(value: Any) -> str | None:
    slot = str(value or "").upper()
    if slot == "LMB":
        return "BA"
    return slot or None


def dimensionmaster_admitted_skills(
    player_skills_path: Path,
    skill_bindings_path: Path,
) -> list[dict[str, Any]]:
    """Return canonical Effect rows admitted by both gameplay and animation.

    Extracted UE3 graphs are evidence candidates.  They do not become product
    skills merely because their numeric IDs resemble a previous quick-slot
    roster.  Product admission requires the current PlayerSkills effectId and
    a current skillbinding with at least one clip.
    """
    bindings_root = read_json(skill_bindings_path)
    if (
        str(bindings_root.get("characterClass") or "").casefold()
        != "dimensionmaster"
    ):
        raise ValueError("DimensionMaster skillbindings class mismatch")
    clips_by_skill: dict[int, tuple[list[str], list[Any]]] = {}
    for row in bindings_root.get("bindings", []):
        skill_id = int(row["skillId"])
        if skill_id in clips_by_skill:
            raise ValueError(f"duplicate DimensionMaster skillbinding: {skill_id}")
        raw_clips = row.get("clips", [])
        if not isinstance(raw_clips, list):
            raise ValueError(f"invalid DimensionMaster skillbinding: {skill_id}")
        clips = []
        for value in raw_clips:
            if isinstance(value, str):
                clip = value
            elif isinstance(value, dict):
                clip = str(value.get("clip") or "")
            else:
                clip = ""
            if not clip:
                raise ValueError(f"invalid DimensionMaster binding clip: {skill_id}")
            clips.append(clip)
        if clips:
            clips_by_skill[skill_id] = (clips, copy.deepcopy(raw_clips))

    result: list[dict[str, Any]] = []
    effect_ids: set[str] = set()
    skill_ids: set[int] = set()
    for row in read_json(player_skills_path).get("skills", []):
        if str(row.get("characterClass", "")).casefold() != "dimensionmaster":
            continue
        effect_id = str(row.get("effectId") or "")
        if not effect_id:
            continue
        skill_id = int(row["skillId"])
        expected_effect_id = f"effect.dimensionmaster.skill.{skill_id}"
        if effect_id != expected_effect_id:
            raise ValueError(
                "DimensionMaster canonical Effect identity mismatch: "
                f"{effect_id} != {expected_effect_id}"
            )
        if skill_id in skill_ids or effect_id in effect_ids:
            raise ValueError(f"duplicate DimensionMaster canonical Effect: {effect_id}")
        binding_clips = clips_by_skill.get(skill_id)
        if not binding_clips:
            raise ValueError(
                f"DimensionMaster canonical Effect has no skillbinding: {effect_id}"
            )
        clips, raw_clips = binding_clips
        slot = presentation_slot(row.get("inputSlot"))
        result.append({
            "skillId": skill_id,
            "effectAssetId": effect_id,
            "inputSlot": slot,
            "displayName": str(row.get("displayName") or effect_id),
            "skillKind": str(row.get("skillKind") or ""),
            "comboStages": copy.deepcopy(row.get("comboStages", [])),
            "clips": clips,
            "clipBindings": raw_clips,
        })
        skill_ids.add(skill_id)
        effect_ids.add(effect_id)
    return result


def graph_index(manifest: dict[str, Any]) -> dict[int, Path]:
    result: dict[int, Path] = {}
    for value in manifest.get("sourceGraphs", []):
        path = Path(value)
        marker = "skill_"
        parent = path.parent.name
        if marker not in parent:
            continue
        result[int(parent.split(marker, 1)[1])] = path
    return result


def build_all(
    data_root: Path,
    manifest_path: Path,
    output_root: Path,
    action_source_path: Path,
    semantic_overlay_path: Path | None = None,
    socket_contract_path: Path | None = None,
) -> dict[str, Any]:
    skill_catalog_path = data_root / "Balance" / "PlayerSkills.json"
    skill_bindings_path = (
        data_root / "Animation" / "Authored" / "DimensionMaster" /
        "DimensionMaster.skillbindings.json"
    )
    admitted_skills = dimensionmaster_admitted_skills(
        skill_catalog_path, skill_bindings_path
    )
    admitted_by_skill = {
        int(row["skillId"]): row for row in admitted_skills
    }

    manifest = read_json(manifest_path)
    action_source = read_json(action_source_path)
    socket_contract = (
        read_json(socket_contract_path)
        if socket_contract_path is not None and socket_contract_path.is_file()
        else None
    )
    semantic_overlay = (
        read_json(semantic_overlay_path)
        if semantic_overlay_path is not None and semantic_overlay_path.is_file()
        else None
    )
    if str(action_source.get("source", {}).get("sha256") or "") == "":
        raise ValueError("DimensionMaster Action source has no LOA hash")
    graphs = graph_index(manifest)
    skill_rows = []
    totals = {
        "sourceEmitterPartitionCount": 0,
        "emittedElementCount": 0,
        "particleBudget": 0,
        "sourceModuleCount": 0,
        "sourceDistributionCount": 0,
        "sourceLiteralCount": 0,
        "unsupportedEmitterCount": 0,
        "missingResourceEmitterCount": 0,
        "sourceMaterialPendingEmitterCount": 0,
        "sourceActionCueCount": 0,
        "sourceActionPayloadBytes": 0,
        "partialRuntimeModuleCount": 0,
        "unsupportedRuntimeModuleCount": 0,
    }
    for skill_id, graph_path in graphs.items():
        admitted = admitted_by_skill.get(skill_id)
        slot = admitted.get("inputSlot") if admitted is not None else None
        receipt_path = output_root / f"skill.{skill_id}.source-receipt.json"
        closure_path = output_root / "Modules" / (
            f"skill.{skill_id}.external-module-closure.json"
        )
        required = (graph_path, receipt_path, closure_path)
        if any(path is None or not Path(path).is_file() for path in required):
            raise FileNotFoundError(
                f"DimensionMaster source {skill_id} contract is incomplete"
            )
        source_receipt = read_json(receipt_path)
        action_cue_recipe = build_action_cue_recipe(
            action_source, source_receipt, socket_contract
        )
        variant_receipt, variant_summary = apply_base_variant_contract(
            source_receipt, action_cue_recipe
        )
        document, receipt = build_document(
            variant_receipt,
            read_json(Path(graph_path)),
            read_json(closure_path),
            semantic_overlay,
        )
        document_path = output_root / "Converted" / (
            f"effect.dimensionmaster.skill.{skill_id}.imported.effect.json"
        )
        conversion_path = output_root / "Converted" / (
            f"skill.{skill_id}.element-conversion-receipt.json"
        )
        write_json_atomic(document_path, document)
        write_json_atomic(conversion_path, receipt)
        action_cue_path = output_root / "Converted" / (
            f"skill.{skill_id}.action-cue-recipe.json"
        )
        write_json_atomic(action_cue_path, action_cue_recipe)

        source_modules = [
            module
            for element in document["elements"]
            for module in element["sourceRecipe"]["modules"]
        ]
        source_module_count = len(source_modules)
        partial_runtime_module_count = sum(
            module["className"] in PARTIAL_RUNTIME_MODULE_CLASSES
            for module in source_modules
        )
        unsupported_runtime_module_count = (
            source_module_count - partial_runtime_module_count
        )
        source_module_classes = sorted(
            {module["className"] for module in source_modules}
        )
        unsupported_runtime_module_classes = sorted(
            set(source_module_classes) - PARTIAL_RUNTIME_MODULE_CLASSES
        )
        source_distribution_count = sum(
            len(module["distributions"])
            for element in document["elements"]
            for module in element["sourceRecipe"]["modules"]
        )
        source_literal_count = sum(
            len(module["literals"])
            for element in document["elements"]
            for module in element["sourceRecipe"]["modules"]
        )
        summary = receipt["summary"]
        row = {
            "slot": slot,
            "skillId": skill_id,
            "effectAssetId": f"effect.dimensionmaster.skill.{skill_id}",
            "admittedCanonical": admitted is not None,
            "canonicalEffectAssetId": (
                admitted["effectAssetId"] if admitted is not None else None
            ),
            "sourceGraph": str(graph_path),
            "sourceReceipt": str(receipt_path),
            "externalModuleClosure": str(closure_path),
            "importedDocument": str(document_path),
            "conversionReceipt": str(conversion_path),
            "actionCueRecipe": str(action_cue_path),
            "sourceEmitterPartitionCount": summary[
                "sourceEmitterPartitionCount"
            ],
            "emittedElementCount": summary["emittedElementCount"],
            "particleBudget": summary["particleBudget"],
            "sourceModuleCount": source_module_count,
            "sourceDistributionCount": source_distribution_count,
            "sourceLiteralCount": source_literal_count,
            "externalRequestCount": summary["externalRequestCount"],
            "externalUnresolvedRequestCount": summary[
                "externalUnresolvedRequestCount"
            ],
            "unsupportedEmitterCount": summary["unsupportedEmitterCount"],
            "missingResourceEmitterCount": summary[
                "missingResourceEmitterCount"
            ],
            "sourceMaterialPendingEmitterCount": summary[
                "sourceMaterialPendingEmitterCount"
            ],
            "sourceActionCueCount": action_cue_recipe["summary"][
                "presentationCueCount"
            ],
            "sourceActionPayloadBytes": action_cue_recipe["summary"][
                "serializedPayloadBytes"
            ],
            **variant_summary,
            "partialRuntimeModuleCount": partial_runtime_module_count,
            "unsupportedRuntimeModuleCount": unsupported_runtime_module_count,
            "sourceModuleClasses": source_module_classes,
            "unsupportedRuntimeModuleClasses": (
                unsupported_runtime_module_classes
            ),
            "cascadeRecipeCaptureComplete": (
                summary["unsupportedEmitterCount"] == 0
                and summary["externalUnresolvedRequestCount"] == 0
            ),
            "actionPayloadCaptureComplete": action_cue_recipe[
                "sourceExtractionComplete"
            ],
            "sourceExtractionStatus": "SEMANTIC_DECODERS_REQUIRED",
            "runtimeExecutionStatus": (
                "MISSING_RESOURCE"
                if summary["missingResourceEmitterCount"]
                else "UNSUPPORTED_SOURCE_MATERIAL_EXECUTION"
                if summary["sourceMaterialPendingEmitterCount"]
                else "UNSUPPORTED_MODULE_EXECUTION"
                if unsupported_runtime_module_count
                else "PARTIAL_MODULE_EXECUTION"
                if partial_runtime_module_count
                else "UNSUPPORTED_ACTION_CUE_EXECUTION"
            ),
            "documentSha256": sha256(document_path),
            "actionCueSha256": sha256(action_cue_path),
        }
        skill_rows.append(row)
        for name in totals:
            totals[name] += int(row[name])

    extracted_ids = {int(row["skillId"]) for row in skill_rows}
    missing_canonical_sources = [
        {
            "skillId": int(row["skillId"]),
            "effectAssetId": str(row["effectAssetId"]),
            "inputSlot": row.get("inputSlot"),
            "status": "MISSING_EXTRACTED_SOURCE",
        }
        for row in admitted_skills
        if int(row["skillId"]) not in extracted_ids
    ]
    admitted_rows = [row for row in skill_rows if row["admittedCanonical"]]
    candidate_rows = [row for row in skill_rows if not row["admittedCanonical"]]
    return {
        "schema": "lostark.dimensionmaster-base-effect-extraction-receipt",
        "formatVersion": 1,
        "characterClass": "DIMENSIONMASTER",
        "variantContract": "BASE_NO_TIME_OR_SPACE_AXIS",
        "slotContract": [row.get("inputSlot") for row in admitted_skills],
        "canonicalSkillContract": admitted_skills,
        "admittedCanonicalEffectCount": len(admitted_rows),
        "extractedCandidateEffectCount": len(candidate_rows),
        "missingCanonicalSources": missing_canonical_sources,
        "skills": skill_rows,
        "totals": totals,
        "sourceExtractionComplete": all(
            row["sourceExtractionStatus"] == "EXACT"
            for row in skill_rows
        ),
        "sourcePayloadCaptureComplete": all(
            row["cascadeRecipeCaptureComplete"]
            and row["actionPayloadCaptureComplete"]
            for row in skill_rows
        ),
        "runtimeExecutionComplete": all(
            row["runtimeExecutionStatus"] == "EXACT"
            for row in skill_rows
        ),
        "inputs": {
            "playerSkills": str(skill_catalog_path),
            "playerSkillsSha256": sha256(skill_catalog_path),
            "skillBindings": str(skill_bindings_path),
            "skillBindingsSha256": sha256(skill_bindings_path),
            "resourceSourceManifest": str(manifest_path),
            "resourceSourceManifestSha256": sha256(manifest_path),
            "actionSource": str(action_source_path),
            "actionSourceSha256": sha256(action_source_path),
            "loaSha256": str(action_source["source"]["sha256"]),
            "semanticOverlay": (
                str(semantic_overlay_path) if semantic_overlay_path else None
            ),
            "semanticOverlaySha256": (
                sha256(semantic_overlay_path)
                if semantic_overlay_path is not None
                and semantic_overlay_path.is_file()
                else None
            ),
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-root", type=Path, default=Path("Data"))
    parser.add_argument(
        "--manifest",
        type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.resource-source-manifest.json"
        ),
    )
    parser.add_argument(
        "--output-root",
        type=Path,
        default=Path("Data/Effects/Imported/DimensionMaster"),
    )
    parser.add_argument(
        "--action-source",
        type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/ActionSource/"
            "DimensionMasterBase.action-effects.json"
        ),
    )
    parser.add_argument(
        "--receipt",
        type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.base-effect-extraction.receipt.json"
        ),
    )
    parser.add_argument(
        "--semantic-overlay",
        type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.source-semantic-overlay.json"
        ),
    )
    parser.add_argument(
        "--socket-contract",
        type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/ActionSource/"
            "DimensionMaster.skeletal-mesh-sockets.json"
        ),
    )
    parser.add_argument("--require-exact-execution", action="store_true")
    args = parser.parse_args()

    receipt = build_all(
        args.data_root,
        args.manifest,
        args.output_root,
        args.action_source,
        args.semantic_overlay,
        args.socket_contract,
    )
    write_json_atomic(args.receipt, receipt)
    print(json.dumps({
        **receipt["totals"],
        "sourceExtractionComplete": receipt["sourceExtractionComplete"],
        "sourcePayloadCaptureComplete": receipt[
            "sourcePayloadCaptureComplete"
        ],
        "runtimeExecutionComplete": receipt["runtimeExecutionComplete"],
        "receipt": str(args.receipt),
    }, ensure_ascii=False, sort_keys=True))
    if not receipt["sourceExtractionComplete"]:
        return 1
    if args.require_exact_execution and not receipt["runtimeExecutionComplete"]:
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
