#!/usr/bin/env python3
"""Propose reviewed source-time compression without mutating animation data.

The first positive row compresses Portal Rush FINISH's full 1666.7 ms source
clip into its 600 ms Server stage.  The proposal preserves clip identity and
order, changes the mapping basis to ``SOURCE_REVIEWED_DELTA``, disables the
legacy final loop, and expands the v2 cue's source-local end so its wall-clock
projection covers the selected source segment.  Dash 400424 remains an
explicit cross-stage review item and is never auto-split by this tool.
"""

from __future__ import annotations

import argparse
import copy
import json
import math
import re
from pathlib import Path
from typing import Any

import build_valtan_source_occurrence_inventory as source_inventory


ROOT = Path(__file__).resolve().parents[2]
OUTPUT_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/"
    "Valtan.source-timing-delta-proposals.v1.json"
)
SELECTION_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/"
    "Valtan.priority-source-sequence-selections.v1.json"
)
ANIMNOTIFY_PATH = ROOT / "Data/Animation/Reference/Valtan/Valtan.animnotify"
PATTERN_BINDINGS_PATH = source_inventory.PATTERN_BINDINGS_PATH
ENCOUNTER_PATH = source_inventory.ENCOUNTER_PATH
CUE_PATH = source_inventory.CUE_PATH

PORTAL_PATTERN_ID = "VALTAN_PORTAL_RUSH"
PORTAL_ACTION_ID = "valtan.attack.portal-rush.finish"
PORTAL_CLIP_OCCURRENCE_ID = "valtan.attack.portal-rush.finish.clip.01"
PORTAL_CLIP = "mesh_att_battle_18_03-1"
PORTAL_SOURCE_ACTION_ID = 420622
PORTAL_SOURCE_STAGE_INDEX = 5
PORTAL_SOURCE_CLIP_ORDINAL = 0
EXPECTED_SOURCE_DURATION_MS = 1666.7
EXPECTED_STAGE_DURATION_MS = 600
SOURCE_NOTIFY_SAMPLES_MS = [732.0, 733.0]


class TimingProposalError(RuntimeError):
    """Raised when timing evidence or a proposed transform is unsafe."""


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def binding_index(document: dict[str, Any]) -> dict[str, dict[str, Any]]:
    result = {}
    for row in document.get("bindings", []):
        action_id = str(row.get("actionId") or "")
        if not action_id or action_id in result:
            raise TimingProposalError("pattern binding action identity is invalid")
        result[action_id] = row
    return result


def cue_index(document: dict[str, Any]) -> dict[str, list[dict[str, Any]]]:
    if document.get("formatVersion") != 2:
        raise TimingProposalError("timing proposals require v2 cue rows")
    result: dict[str, list[dict[str, Any]]] = {}
    for row in document.get("cues", []):
        occurrence = str(row.get("clipOccurrenceId") or "")
        if not occurrence:
            raise TimingProposalError("cue clip occurrence identity is invalid")
        result.setdefault(occurrence, []).append(row)
    return result


def source_clip_duration_ms(path: Path, clip: str) -> float:
    expression = re.compile(
        rf'^"{re.escape(clip)}"\s+skill=\d+\s+len=([0-9]+(?:\.[0-9]+)?)\b',
        re.IGNORECASE,
    )
    matches = []
    for line in path.read_text(encoding="utf-8-sig").splitlines():
        match = expression.match(line.strip())
        if match:
            matches.append(float(match.group(1)) * 1000.0)
    if len(matches) != 1 or not math.isfinite(matches[0]):
        raise TimingProposalError(f"source clip duration is not unique: {clip}")
    return matches[0]


def encounter_stage_duration_ms(
    encounter: dict[str, Any], pattern_id: str, action_id: str
) -> int:
    matches = [
        int(stage["durationMs"])
        for pattern in encounter.get("patterns", [])
        if pattern.get("patternId") == pattern_id
        for stage in pattern.get("stages", [])
        if stage.get("actionId") == action_id
    ]
    if len(matches) != 1:
        raise TimingProposalError("Server stage duration evidence is not unique")
    return matches[0]


def validate_transform(
    source_duration_ms: float,
    stage_duration_ms: int,
    proposed_binding: dict[str, Any],
    proposed_cue: dict[str, Any],
) -> dict[str, Any]:
    rate = proposed_binding.get("playRate")
    if (
        isinstance(rate, bool)
        or not isinstance(rate, (int, float))
        or not math.isfinite(float(rate))
        or not 0.05 <= float(rate) <= 16.0
    ):
        raise TimingProposalError("proposed playRate is outside v2 bounds")
    if proposed_binding.get("loop") is not False:
        raise TimingProposalError("finite timing delta may not retain a loop")
    effective_wall_ms = source_duration_ms / float(rate)
    if effective_wall_ms > stage_duration_ms + 1e-6:
        raise TimingProposalError("full source segment exceeds Server stage")
    cue_start = proposed_cue.get("sourceStartMs")
    cue_end = proposed_cue.get("sourceEndMs")
    if (
        isinstance(cue_start, bool)
        or isinstance(cue_end, bool)
        or not isinstance(cue_start, int)
        or not isinstance(cue_end, int)
        or cue_start < 0
        or cue_end <= cue_start
        or cue_end > math.floor(source_duration_ms)
    ):
        raise TimingProposalError("proposed cue source segment is invalid")
    cue_wall_end_ms = cue_end / float(rate)
    notify_rows = [
        {
            "sourceTimeMs": source_ms,
            "projectedStageWallTimeMs": source_ms / float(rate),
        }
        for source_ms in SOURCE_NOTIFY_SAMPLES_MS
    ]
    if any(
        row["projectedStageWallTimeMs"] >= stage_duration_ms
        for row in notify_rows
    ):
        raise TimingProposalError("source notify still exceeds Server stage")
    return {
        "effectiveFullSegmentWallDurationMs": effective_wall_ms,
        "cueWallStartMs": cue_start / float(rate),
        "cueWallEndMs": cue_wall_end_ms,
        "sourceNotifyWallProjections": notify_rows,
        "fullSourceSegmentFitsStage": True,
        "allReviewedNotifySamplesFitStage": True,
    }


def build_proposals() -> dict[str, Any]:
    source_duration_ms = source_clip_duration_ms(
        ANIMNOTIFY_PATH, PORTAL_CLIP
    )
    stage_duration_ms = encounter_stage_duration_ms(
        read_json(ENCOUNTER_PATH), PORTAL_PATTERN_ID, PORTAL_ACTION_ID
    )
    if (
        abs(source_duration_ms - EXPECTED_SOURCE_DURATION_MS) > 1e-6
        or stage_duration_ms != EXPECTED_STAGE_DURATION_MS
    ):
        raise TimingProposalError("Portal FINISH timing evidence drifted")

    bindings = binding_index(read_json(PATTERN_BINDINGS_PATH))
    current_binding_row = bindings.get(PORTAL_ACTION_ID)
    if current_binding_row is None or len(current_binding_row.get("clips", [])) != 1:
        raise TimingProposalError("Portal FINISH binding is not one clip")
    current_clip = copy.deepcopy(current_binding_row["clips"][0])
    if (
        current_clip.get("clipOccurrenceId") != PORTAL_CLIP_OCCURRENCE_ID
        or str(current_clip.get("clip") or "").casefold()
        != PORTAL_CLIP.casefold()
    ):
        raise TimingProposalError("Portal FINISH clip identity drifted")

    selections = source_inventory.load_selection_manifest(SELECTION_PATH)
    selection_rows = [
        row
        for row in selections["selections"]
        if row.get("patternId") == PORTAL_PATTERN_ID
    ]
    if len(selection_rows) != 1:
        raise TimingProposalError("Portal source selection is not unique")
    selected_mapping = [
        row
        for row in selection_rows[0].get("stageMappings", [])
        if row.get("sourceStageIndex") == PORTAL_SOURCE_STAGE_INDEX
        and row.get("sourceClipOrdinal") == PORTAL_SOURCE_CLIP_ORDINAL
    ]
    if (
        len(selected_mapping) != 1
        or selected_mapping[0].get("clipOccurrenceId")
        != PORTAL_CLIP_OCCURRENCE_ID
        or selected_mapping[0].get("timingDisposition")
        != "SOURCE_TIMING_REVIEW_REQUIRED"
    ):
        raise TimingProposalError("Portal FINISH is not awaiting timing review")

    cues = cue_index(read_json(CUE_PATH))
    portal_cues = cues.get(PORTAL_CLIP_OCCURRENCE_ID, [])
    if len(portal_cues) != 1:
        raise TimingProposalError("Portal FINISH v2 cue identity is not unique")
    current_cue = copy.deepcopy(portal_cues[0])
    if not current_cue or current_cue.get("repeatPolicy") != "once":
        raise TimingProposalError("Portal FINISH v2 cue identity drifted")

    play_rate = source_duration_ms / stage_duration_ms
    proposed_binding = copy.deepcopy(current_clip)
    proposed_binding.update(
        {
            "mappingBasis": "SOURCE_REVIEWED_DELTA",
            "sourceStartMs": 0,
            "playMs": 0,
            "playRate": play_rate,
            "loop": False,
        }
    )
    proposed_cue = copy.deepcopy(current_cue)
    proposed_cue["sourceStartMs"] = 0
    proposed_cue["sourceEndMs"] = math.floor(source_duration_ms)
    transform = validate_transform(
        source_duration_ms,
        stage_duration_ms,
        proposed_binding,
        proposed_cue,
    )

    return {
        "schema": "lostark.valtan-source-timing-delta-proposals",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "mode": "REVIEW_ONLY_NO_CANONICAL_DATA_MUTATION",
        "sources": source_inventory.repository_source_rows(
            [
                ENCOUNTER_PATH,
                PATTERN_BINDINGS_PATH,
                CUE_PATH,
                ANIMNOTIFY_PATH,
                SELECTION_PATH,
            ]
        ),
        "proposals": [
            {
                "proposalId": "valtan.portal-rush.finish.full-segment-compression.v1",
                "patternId": PORTAL_PATTERN_ID,
                "actionId": PORTAL_ACTION_ID,
                "clipOccurrenceId": PORTAL_CLIP_OCCURRENCE_ID,
                "sourceActionId": PORTAL_SOURCE_ACTION_ID,
                "sourceStageIndex": PORTAL_SOURCE_STAGE_INDEX,
                "sourceClipOrdinal": PORTAL_SOURCE_CLIP_ORDINAL,
                "status": "PROPOSED_SOURCE_REVIEWED_DELTA_NOT_ACCEPTED",
                "sourceSegment": {
                    "clip": PORTAL_CLIP,
                    "sourceStartMs": 0,
                    "sourceDurationMs": source_duration_ms,
                    "evidencePath": ANIMNOTIFY_PATH.relative_to(ROOT).as_posix(),
                },
                "serverStageDurationMs": stage_duration_ms,
                "currentBindingClip": current_clip,
                "proposedBindingClip": proposed_binding,
                "currentCue": current_cue,
                "proposedCue": proposed_cue,
                "wallProjection": transform,
            }
        ],
        "deferredExplicitReviews": [
            {
                "patternId": "VALTAN_DASH_CHARGE",
                "proposedSourceActionId": 400424,
                "status": "CROSS_STAGE_SOURCE_TIMING_SPLIT_REVIEW_REQUIRED",
                "disposition": "DO_NOT_AUTO_REPLACE_420604_OR_SPLIT_PRODUCT_TIMING",
            }
        ],
        "summary": {
            "proposalCount": 1,
            "fullSegmentFitsStageCount": 1,
            "canonicalDataMutationCount": 0,
            "deferredCrossStageReviewCount": 1,
        },
    }


def check_exact(path: Path, payload: bytes) -> None:
    if not path.is_file() or path.read_bytes() != payload:
        raise TimingProposalError(f"timing delta proposal drifted: {path}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument("--output", type=Path, default=OUTPUT_PATH)
    args = parser.parse_args()

    document = build_proposals()
    payload = source_inventory.pretty_json_bytes(document)
    output = args.output.resolve()
    authored = source_inventory.AUTHORED_ROOT.resolve()
    if output == authored or authored in output.parents:
        parser.error("timing proposal cannot target Data/Effects/Authored")
    if args.write:
        source_inventory.write_atomic(output, payload)
        label = "written"
    elif args.check:
        check_exact(output, payload)
        label = "checked"
    else:
        label = "dry-run"
    print(
        "Valtan source timing delta proposals "
        f"{label}: {json.dumps(document['summary'], sort_keys=True)}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (TimingProposalError, source_inventory.InventoryError) as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
