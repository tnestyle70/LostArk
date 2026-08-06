#!/usr/bin/env python3
"""Materialize source-derived DimensionMaster Effect documents for runtime.

The aggregate BA document remains the authoring/audition view.  Product combo
playback uses four stage documents whose element clocks are rebased to the
corresponding animation clip, so starting BA2 cannot replay BA1 particles.
"""

from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path
from typing import Any

from build_dimensionmaster_base_effects import EXPECTED_SLOTS
from build_imported_effect_documents import read_json, write_json_atomic


def canonicalize(document: dict[str, Any], effect_id: str) -> dict[str, Any]:
    result = copy.deepcopy(document)
    result["effectAssetId"] = effect_id
    return result


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
    existing_authored: dict[str, Any] | None,
    action_recipe: dict[str, Any],
    summon_retime: dict[str, Any],
) -> list[dict[str, Any]]:
    source_model_cues = [
        cue for cue in action_recipe.get("cues", [])
        if cue.get("runtimeChannel") == "MODEL_CUE"
    ]
    if not source_model_cues or existing_authored is None:
        return []
    # Only F currently has a converted runtime WModel with an exact source clip
    # join.  V's source mesh remains captured in the cue recipe but has no
    # admitted WModel and therefore must remain fail-closed.
    if skill_id != 2050500:
        return []
    duration_by_clip = {
        str(row["runtimeClip"]): float(row["durationSeconds"])
        for row in summon_retime.get("clips", [])
    }
    result = []
    for cue in existing_authored.get("modelCues", []):
        clip = str(cue.get("clipName") or "")
        if clip not in duration_by_clip:
            continue
        value = copy.deepcopy(cue)
        value["durationSeconds"] = duration_by_clip[clip]
        result.append(value)
    if len(result) != 1:
        raise ValueError("F source Model Cue did not resolve to exactly one WModel clip")
    return result


def materialize(
    imported_root: Path,
    authored_root: Path,
    extraction_receipt_path: Path,
    summon_retime_path: Path,
) -> dict[str, Any]:
    extraction = read_json(extraction_receipt_path)
    if not extraction.get("sourcePayloadCaptureComplete"):
        raise ValueError("byte-lossless Action source capture is incomplete")
    rows_by_skill = {
        int(row["skillId"]): row for row in extraction.get("skills", [])
    }
    summon_retime = read_json(summon_retime_path)
    if not summon_retime.get("sourceRateRestorationComplete"):
        raise ValueError("summon source animation rate restoration is incomplete")

    authored_root.mkdir(parents=True, exist_ok=True)
    written = []
    aggregates: dict[int, dict[str, Any]] = {}
    for slot, skill_id in EXPECTED_SLOTS.items():
        if skill_id not in rows_by_skill:
            raise ValueError(f"missing extraction receipt row for {slot} {skill_id}")
        imported_path = imported_root / "Converted" / (
            f"effect.dimensionmaster.skill.{skill_id}.imported.effect.json"
        )
        action_path = imported_root / "Converted" / (
            f"skill.{skill_id}.action-cue-recipe.json"
        )
        imported = read_json(imported_path)
        action = read_json(action_path)
        canonical_id = f"effect.dimensionmaster.skill.{skill_id}"
        authored_path = authored_root / f"{canonical_id}.effect.json"
        existing = read_json(authored_path) if authored_path.is_file() else None
        aggregate = canonicalize(imported, canonical_id)
        aggregate["modelCues"] = source_confirmed_model_cues(
            skill_id, existing, action, summon_retime
        )
        write_json_atomic(authored_path, aggregate)
        aggregates[skill_id] = aggregate
        written.append({
            "effectAssetId": canonical_id,
            "path": str(authored_path),
            "elementCount": len(aggregate.get("elements", [])),
            "modelCueCount": len(aggregate.get("modelCues", [])),
            "role": "AGGREGATE",
        })

    ba_action = read_json(
        imported_root / "Converted" / "skill.2050010.action-cue-recipe.json"
    )
    stages = ba_action.get("selectedStages", [])
    if len(stages) != 4:
        raise ValueError(f"BA stage contract requires 4 stages, got {len(stages)}")
    for index, stage in enumerate(stages):
        offset = float(stage["clipOffsetSeconds"])
        next_offset = (
            float(stages[index + 1]["clipOffsetSeconds"])
            if index + 1 < len(stages) else None
        )
        effect_id = f"effect.dimensionmaster.skill.2050010.ba{index + 1}"
        document = build_combo_stage_document(
            aggregates[2050010], effect_id, f"기본 공격 BA{index + 1}",
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

    return {
        "schema": "lostark.dimensionmaster-base-effect-materialization-receipt",
        "formatVersion": 1,
        "variantContract": "BASE_NO_TIME_OR_SPACE_AXIS",
        "aggregateSkillCount": len(EXPECTED_SLOTS),
        "comboStageDocumentCount": 4,
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
        default=Path("Data/Effects/Imported/DimensionMaster/DimensionMaster.base-effect-extraction.receipt.json")
    )
    parser.add_argument(
        "--summon-retime", type=Path,
        default=Path("Data/Effects/Imported/DimensionMaster/DimensionMaster.summon-animation-retime.receipt.json")
    )
    parser.add_argument(
        "--receipt", type=Path,
        default=Path("Data/Effects/Imported/DimensionMaster/DimensionMaster.base-effect-materialization.receipt.json")
    )
    args = parser.parse_args()
    receipt = materialize(
        args.imported_root, args.authored_root,
        args.extraction_receipt, args.summon_retime
    )
    write_json_atomic(args.receipt, receipt)
    print(json.dumps({
        "aggregateSkillCount": receipt["aggregateSkillCount"],
        "comboStageDocumentCount": receipt["comboStageDocumentCount"],
        "runtimeExecutionComplete": receipt["runtimeExecutionComplete"],
        "receipt": str(args.receipt),
    }, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
