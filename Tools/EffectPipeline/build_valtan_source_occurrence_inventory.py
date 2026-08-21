#!/usr/bin/env python3
"""Build the non-destructive Valtan source-occurrence inventory.

This is deliberately not an authored Effect writer.  It factors the source
into three immutable layers:

* source action branches and their animation/effect notify occurrences,
* exact ParticleSystem -> emitter -> selected LOD -> ordered module closure,
* the full occurrence x carrier keys that a reviewed product cue may consume.

No source sequence is admitted merely because its clip name matches.  The
read-only ``Valtan.clipseq`` sequenceIndex/full clip path owns sequence identity,
including paths that cross a misleading ``Main`` stage label.  Every sequence
candidate remains visible, and an unreviewed sequence is
``UNRESOLVED_BRANCH_SELECTION``.  Consequently the sequence-before-emitter
cartesian upper bound is never reported as the completion denominator.

The Cascade conversion primitives are shared with character Effects through
``build_imported_effect_documents.py``.  Light carriers and ParticleSystems
whose source identity explicitly names a generic Dust package/system are kept
as deferred evidence.  A selected core ParticleSystem keeps every emitter and
the selected LOD's complete ordered module occurrence stream.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


ROOT = Path(__file__).resolve().parents[2]
PIPELINE_TOOLS = ROOT / "Tools" / "EffectPipeline"
if str(PIPELINE_TOOLS) not in sys.path:
    sys.path.insert(0, str(PIPELINE_TOOLS))
LEVEL_TOOLS = ROOT / "Tools" / "LevelPlacementExtractor"
if str(LEVEL_TOOLS) not in sys.path:
    sys.path.insert(0, str(LEVEL_TOOLS))

import build_imported_effect_documents as imported_effects  # noqa: E402
import build_skill_effect_source_receipt as source_receipts  # noqa: E402
from materialize_artist_31470_portable_particle_carriers import (  # noqa: E402
    MaterializeError,
    portable_recipe,
)


ENCOUNTER_PATH = ROOT / "Data/Encounters/Valtan/ValtanEncounter.json"
PATTERN_BINDINGS_PATH = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
)
ACTION_BINDINGS_PATH = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.actionbindings.json"
)
CUE_PATH = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
SOURCE_CATALOG_PATH = (
    ROOT / "Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json"
)
CLIPSEQ_PATH = ROOT / "Data/Animation/Reference/Valtan/Valtan.clipseq"
PATTERN_PREVIEW_PATH = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.patternpreview.json"
)
OUTPUT_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/Valtan.source-occurrence-inventory.v1.json"
)
AUTHORED_ROOT = ROOT / "Data/Effects/Authored"

EXPECTED_PATTERN_COUNT = 33
MISSING_ACTION_BINDING_POLICIES = {
    "VALTAN_ENTRANCE_WHIRLWIND": (
        "MISSING_ACTION_BINDING_PROJECT_REUSE_REVIEW"
    ),
    "VALTAN_ARENA_BREAK_84": "MISSING_ACTION_BINDING_NO_SOURCE_ACTION",
}
ARENA84_BINDING_GAP_PROPOSALS = [
    {
        "patternId": "VALTAN_ARENA_BREAK_84",
        "stageId": "WINDUP",
        "actionId": "valtan.mechanic.arena-floor-84.windup",
        "sourceActionId": 420629,
        "sourceStageIndex": 0,
        "status": "PROPOSED_SOURCE_REVIEWED_DELTA_NOT_ACCEPTED",
        "proposedClip": {
            "clipOccurrenceId": "valtan.mechanic.arena-floor-84.windup.clip.01",
            "clip": "mesh_att_battle_12_01",
            "mappingBasis": "SOURCE_REVIEWED_DELTA",
            "sourceStartMs": 0,
            "playMs": 0,
            "playRate": 1.0,
            "loop": True,
        },
    },
    {
        "patternId": "VALTAN_ARENA_BREAK_84",
        "stageId": "IMPACT",
        "actionId": "valtan.mechanic.arena-floor-84.impact",
        "sourceActionId": 420629,
        "sourceStageIndex": 1,
        "status": "PROPOSED_SOURCE_REVIEWED_DELTA_NOT_ACCEPTED",
        "proposedClip": {
            "clipOccurrenceId": "valtan.mechanic.arena-floor-84.impact.clip.01",
            "clip": "mesh_att_battle_12_02",
            "mappingBasis": "SOURCE_REVIEWED_DELTA",
            "sourceStartMs": 0,
            "playMs": 0,
            "playRate": 1.0,
            "loop": True,
        },
    },
    {
        "patternId": "VALTAN_ARENA_BREAK_84",
        "stageId": "RECOVERY",
        "actionId": "valtan.mechanic.arena-floor-84.recovery",
        "sourceActionId": 420629,
        "sourceStageIndex": 2,
        "status": "PROPOSED_SOURCE_REVIEWED_DELTA_NOT_ACCEPTED",
        "proposedClip": {
            "clipOccurrenceId": "valtan.mechanic.arena-floor-84.recovery.clip.01",
            "clip": "mesh_att_battle_12_03",
            "mappingBasis": "SOURCE_REVIEWED_DELTA",
            "sourceStartMs": 0,
            "playMs": 0,
            "playRate": 1.0,
            "loop": True,
        },
    },
]
SOURCE_VISUAL_SIGNATURE_EQUIVALENCE_REVIEWS = {
    "VALTAN_SWING": [(420601, 0), (420660, 0)],
    "VALTAN_IMPRISON_ROAR": [(420603, 0), (420603, 2), (420603, 3)],
    "VALTAN_PARRY": [(420606, 0), (420606, 2), (420606, 3)],
    "VALTAN_STOMP": [(420611, 0), (420611, 2), (420611, 3)],
}
SOURCE_VISUAL_SIGNATURE_AMBIGUITIES = {
    "VALTAN_BIND_CHARGE_SMASH",
}
ADDITIONAL_SOURCE_SELECTION_REVIEW_ORDER = [
    "VALTAN_SWING",
    "VALTAN_IMPRISON_ROAR",
    "VALTAN_DASH_CHARGE",
    "VALTAN_PARRY",
    "VALTAN_STOMP",
    "VALTAN_BIND_CHARGE_SMASH",
    "VALTAN_FOUR_PILLARS_105",
    "VALTAN_ENTRANCE_WHIRLWIND",
    "VALTAN_ARENA_BREAK_84",
    "VALTAN_MAGIC_ORB_STAGGER_76",
    "VALTAN_CENTER_GRAB_COUNTER_64",
    "VALTAN_ARENA_BREAK_33",
]
SAFE_ADDITIONAL_SOURCE_SELECTIONS = {
    "VALTAN_SWING": {
        "branchId": (
            "valtan_swing.source-420601.mn_rpbf_00."
            "sequence-000.stages-000-001"
        ),
        "reviewBasis": (
            "420601/420660 seq=0 have byte-equivalent source visual "
            "families; 420601 seq=0 is the exact ordered Animation PR #127 "
            "two-clip occurrence path"
        ),
    },
    "VALTAN_IMPRISON_ROAR": {
        "branchId": (
            "valtan_imprison_roar.source-420603.mn_rpbf_00."
            "sequence-002.stages-005-007"
        ),
        "reviewBasis": (
            "420603 seq=0/2/3 have byte-equivalent source visual families; "
            "seq=2 is the exact ordered Animation PR #127 start/loop/end "
            "occurrence path without extra source stages"
        ),
    },
    "VALTAN_PARRY": {
        "branchId": (
            "valtan_parry.source-420606.mn_rpbf_00."
            "sequence-000.stages-000-002"
        ),
        "reviewBasis": (
            "420606 seq=0/2/3 have byte-equivalent source visual families; "
            "seq=0 exactly joins the ordered Animation PR #127 "
            "stance/slash/recovery occurrences"
        ),
    },
}
UNRESOLVED_ADDITIONAL_SOURCE_SELECTION_REASONS = {
    "VALTAN_DASH_CHARGE": (
        "canonical 420604 branches cannot join the PR #127 GROGGY and "
        "PART_BREAK clips; 400424 remains an unaccepted evidence-only "
        "source-action delta"
    ),
    "VALTAN_STOMP": (
        "byte-equivalent visual families do not resolve occurrence timing: "
        "seq=0 has eight Att_Battle_11_01 stages while seq=2/3 have two, "
        "but PR #127 owns three same-name product occurrences"
    ),
    "VALTAN_BIND_CHARGE_SMASH": (
        "candidate source sequences have non-equivalent visual signatures "
        "and the multi-action source paths contain clips absent from the "
        "four PR #127 occurrences"
    ),
    "VALTAN_FOUR_PILLARS_105": (
        "seq=2 and seq=3 both match the four PR #127 clip names but their "
        "source visual family signatures differ, so no source payload is "
        "exact-equivalent"
    ),
    "VALTAN_ENTRANCE_WHIRLWIND": (
        "the encounter pattern has no action binding; reuse of the protected "
        "Whirlwind family remains review-only"
    ),
    "VALTAN_ARENA_BREAK_84": (
        "all three encounter actions lack canonical animation bindings; the "
        "420629 mappings remain unaccepted source-reviewed proposals"
    ),
    "VALTAN_MAGIC_ORB_STAGGER_76": (
        "420618 seq=0 matches the three PR #127 groggy clips but its source "
        "visual payload differs from seq=2/3; 420617 does not join those "
        "product clip occurrences"
    ),
    "VALTAN_CENTER_GRAB_COUNTER_64": (
        "420623 sequence candidates have different visual payloads and extra "
        "source stages; 420631 groggy clips are absent from this pattern's "
        "five PR #127 occurrences"
    ),
    "VALTAN_ARENA_BREAK_33": (
        "420629 owns one Att_Battle_12_02 source occurrence while PR #127 "
        "owns separate LANDING and SPIN occurrences, and the sequence visual "
        "payloads are not all equivalent"
    ),
}
VISUAL_CATEGORIES = {
    "particle",
    "trail",
    "decal",
    "unresolved_effect",
}
ID_SAFE_RE = re.compile(r"[^a-z0-9_.:-]+")
LEGACY_ELEMENT_RE = re.compile(r"(?:^|\.)em\d+$", re.IGNORECASE)


class InventoryError(RuntimeError):
    """Raised when a source identity cannot be preserved deterministically."""


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def canonical_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(
            value,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
            allow_nan=False,
        )
        + "\n"
    ).encode("utf-8")


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def canonical_sha256(value: Any) -> str:
    return sha256_bytes(canonical_json_bytes(value))


def stable_slug(value: Any, maximum: int = 120) -> str:
    result = ID_SAFE_RE.sub("-", str(value or "").casefold()).strip("-._:")
    return (result or "unnamed")[:maximum]


def normalize_clip(value: Any) -> str:
    clip = str(value or "").casefold()
    return clip[5:] if clip.startswith("mesh_") else clip


def visual_cue_signature_payload(cue: dict[str, Any]) -> dict[str, Any]:
    """Return only byte-stable presentation fields, never source coordinates.

    Notify IDs, source offsets, action IDs, stage indices, and labels identify an
    occurrence, not its visual payload.  Asset reference order remains exact.
    """
    return {
        "sourceType": str(cue.get("sourceType") or ""),
        "category": str(cue.get("category") or ""),
        "resolutionStatus": str(cue.get("resolutionStatus") or ""),
        "localTimeSeconds": finite_number(
            cue.get("localTimeSeconds", 0.0), "visual cue local time"
        ),
        "durationSeconds": finite_number(
            cue.get("durationSeconds", 0.0), "visual cue duration"
        ),
        "assetReferences": copy.deepcopy(cue.get("assetReferences", [])),
    }


def source_visual_family_signature(
    branch: dict[str, Any],
) -> tuple[str, list[str]]:
    """Hash exact unique clip-local visual families without deduping branches.

    A source sequence may repeat the exact same clip-local notify family several
    times.  Equivalence review is allowed to notice that byte identity, while
    the occurrence inventory still preserves every repeated stage separately.
    """
    ordered_clips_by_stage: dict[int, list[str]] = defaultdict(list)
    for row in branch["orderedClips"]:
        ordered_clips_by_stage[int(row["sourceStageIndex"])].append(
            str(row["normalizedClip"])
        )

    family_payloads = []
    family_shas = []
    seen: set[str] = set()
    for stage in branch["stages"]:
        cues = [
            visual_cue_signature_payload(cue)
            for cue in stage.get("effectCues", [])
            if str(cue.get("category") or "") in VISUAL_CATEGORIES
        ]
        if not cues:
            continue
        payload = {
            "normalizedClips": ordered_clips_by_stage[
                int(stage["stageIndex"])
            ],
            "visualCues": cues,
        }
        digest = canonical_sha256(payload)
        if digest in seen:
            continue
        seen.add(digest)
        family_shas.append(digest)
        family_payloads.append(payload)
    return canonical_sha256(family_payloads), family_shas


def is_idle_clip(value: Any) -> bool:
    clip = normalize_clip(value)
    return clip.startswith("idle_") or clip.startswith("wait_")


def finite_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise InventoryError(f"{label} must be a finite number")
    result = float(value)
    if not math.isfinite(result):
        raise InventoryError(f"{label} must be a finite number")
    return result


def flatten_binding_clips(row: dict[str, Any]) -> list[dict[str, Any]]:
    """Read both the checked-in v1 binding and the clip-occurrence v2 shape."""
    action_id = str(row.get("actionId") or "")
    if not action_id:
        raise InventoryError("pattern binding has no actionId")

    raw_clips = row.get("clips")
    if raw_clips is None:
        raw_clips = row.get("clip")
    if isinstance(raw_clips, (str, dict)):
        values: list[Any] = [raw_clips]
    elif isinstance(raw_clips, list):
        values = raw_clips
    else:
        raise InventoryError(f"{action_id} has no clip binding")

    clips: list[dict[str, Any]] = []
    for ordinal, raw in enumerate(values):
        if isinstance(raw, str):
            clip = raw
            occurrence_id = f"{action_id}.clip-{ordinal + 1:03d}"
            mapping_basis = "LEGACY_V1_ORDER"
            source_start_ms = 0
            play_ms = None
            play_rate = 1.0
            loop = False
        elif isinstance(raw, dict):
            clip = str(raw.get("clip") or "")
            occurrence_id = str(raw.get("clipOccurrenceId") or "")
            mapping_basis = str(raw.get("mappingBasis") or "EXPLICIT_V2")
            source_start_ms = int(raw.get("sourceStartMs") or 0)
            play_ms = raw.get("playMs")
            play_rate = finite_number(raw.get("playRate", 1.0), "playRate")
            loop = bool(raw.get("loop", False))
        else:
            raise InventoryError(f"{action_id} has an invalid clip entry")
        if not clip or not occurrence_id:
            raise InventoryError(f"{action_id} has an incomplete clip occurrence")
        if play_ms is not None:
            play_ms = int(play_ms)
            if play_ms < 0:
                raise InventoryError(
                    f"{occurrence_id} playMs must be zero (natural) or positive"
                )
        if source_start_ms < 0 or play_rate <= 0:
            raise InventoryError(f"{occurrence_id} has invalid source timing")
        clips.append(
            {
                "clipOccurrenceId": occurrence_id,
                "clip": clip,
                "normalizedClip": normalize_clip(clip),
                "mappingBasis": mapping_basis,
                "sourceStartMs": source_start_ms,
                "playMs": play_ms,
                "playRate": play_rate,
                "loop": loop,
            }
        )
    return clips


def product_clip_occurrences(
    encounter: dict[str, Any], pattern_bindings: dict[str, Any]
) -> dict[str, list[dict[str, Any]]]:
    by_action: dict[str, list[dict[str, Any]]] = {}
    seen_occurrence_ids: set[str] = set()
    for row in pattern_bindings.get("bindings", []):
        action_id = str(row.get("actionId") or "")
        if action_id in by_action:
            raise InventoryError(f"duplicate pattern binding actionId: {action_id}")
        clips = flatten_binding_clips(row)
        for clip in clips:
            occurrence_id = clip["clipOccurrenceId"]
            if occurrence_id in seen_occurrence_ids:
                raise InventoryError(
                    f"duplicate clipOccurrenceId: {occurrence_id}"
                )
            seen_occurrence_ids.add(occurrence_id)
        by_action[action_id] = clips

    result: dict[str, list[dict[str, Any]]] = {}
    for pattern in encounter.get("patterns", []):
        pattern_id = str(pattern.get("patternId") or "")
        ordered: list[dict[str, Any]] = []
        for stage_ordinal, stage in enumerate(pattern.get("stages", [])):
            action_id = str(stage.get("actionId") or "")
            for clip_ordinal, clip in enumerate(by_action.get(action_id, [])):
                ordered.append(
                    {
                        **copy.deepcopy(clip),
                        "patternId": pattern_id,
                        "semanticStageId": str(stage.get("stageId") or ""),
                        "gameplayActionId": action_id,
                        "productStageOrdinal": stage_ordinal,
                        "productClipOrdinal": clip_ordinal,
                    }
                )
        result[pattern_id] = ordered
    return result


def load_source_clip_sequences(path: Path = CLIPSEQ_PATH) -> dict[int, list[dict[str, Any]]]:
    rows: dict[int, list[dict[str, Any]]] = defaultdict(list)
    pattern = re.compile(
        r'^([0-9]+)\s+"[^"]*"\s+seq=([0-9]+)\s+'
        r'mode=([A-Z]+)\s+clips="([^"]*)"$'
    )
    seen: set[tuple[int, int]] = set()
    for line_number, raw_line in enumerate(
        path.read_text(encoding="utf-8-sig").splitlines(), start=1
    ):
        line = raw_line.strip()
        if not line or line.startswith("LOSTARK_CLIP_SEQ"):
            continue
        match = pattern.fullmatch(line)
        if match is None:
            raise InventoryError(
                f"Valtan clip sequence row is malformed at line {line_number}"
            )
        action_id = int(match.group(1))
        sequence_index = int(match.group(2))
        identity = (action_id, sequence_index)
        if identity in seen:
            raise InventoryError(
                f"Valtan clip sequence identity is duplicated: {identity}"
            )
        seen.add(identity)
        clips = [
            value.strip()
            for value in match.group(4).split(",")
            if value.strip()
        ]
        if not clips:
            raise InventoryError(
                f"Valtan clip sequence has no clips: {identity}"
            )
        rows[action_id].append(
            {
                "sequenceIndex": sequence_index,
                "sequenceMode": match.group(3),
                "clips": clips,
                "sourceLineNumber": line_number,
            }
        )
    return dict(rows)


def split_source_sequences(
    pattern_id: str,
    source_action: dict[str, Any],
    sequences: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    """Rebuild exact clipseq sequence paths without using ``Main`` resets."""
    stages = sorted(
        source_action.get("stages", []), key=lambda row: int(row["stageIndex"])
    )
    flattened = []
    for stage in stages:
        clips = stage.get("animationClips", [])
        for clip_ordinal, clip in enumerate(clips):
            flattened.append(
                {
                    "sourceStageIndex": int(stage["stageIndex"]),
                    "sourceStageName": str(stage.get("stageName") or ""),
                    "sourceClipOrdinal": clip_ordinal,
                    "sourceStageClipCount": len(clips),
                    "clip": str(clip),
                    "normalizedClip": normalize_clip(clip),
                }
            )

    action_id = int(source_action["sourceActionId"])
    profile_id = str(source_action.get("profileId") or "unknown")
    branches: list[dict[str, Any]] = []
    cursor = 0
    stage_by_index = {int(row["stageIndex"]): row for row in stages}
    for ordinal, sequence in enumerate(sequences):
        sequence_clips = sequence["clips"]
        selected = flattened[cursor : cursor + len(sequence_clips)]
        if len(selected) != len(sequence_clips) or [
            row["normalizedClip"] for row in selected
        ] != [normalize_clip(row) for row in sequence_clips]:
            raise InventoryError(
                "Valtan action stages do not match clipseq sequence path: "
                f"{pattern_id}/{action_id}/seq-{sequence['sequenceIndex']}"
            )
        if (
            selected[0]["sourceClipOrdinal"] != 0
            or selected[-1]["sourceClipOrdinal"] + 1
            != selected[-1]["sourceStageClipCount"]
        ):
            raise InventoryError(
                "Valtan clipseq sequence boundary splits one source stage: "
                f"{pattern_id}/{action_id}/seq-{sequence['sequenceIndex']}"
            )
        cursor += len(sequence_clips)
        source_stage_path = list(
            dict.fromkeys(row["sourceStageIndex"] for row in selected)
        )
        start = source_stage_path[0]
        end = source_stage_path[-1]
        sequence_path_sha = canonical_sha256(
            [
                {
                    "sourceStageIndex": row["sourceStageIndex"],
                    "sourceClipOrdinal": row["sourceClipOrdinal"],
                    "clip": row["clip"],
                }
                for row in selected
            ]
        )
        branch_id = (
            f"{pattern_id.casefold()}.source-{action_id}."
            f"{stable_slug(profile_id)}.sequence-"
            f"{sequence['sequenceIndex']:03d}."
            f"stages-{start:03d}-{end:03d}"
        )
        branch = {
                "branchId": branch_id,
                "patternId": pattern_id,
                "sourceActionId": action_id,
                "profileId": profile_id,
                "branchOrdinal": ordinal,
                "sequenceIndex": sequence["sequenceIndex"],
                "sequenceMode": sequence["sequenceMode"],
                "sourceSequencePathSha256": sequence_path_sha,
                "sourceStageStartIndex": start,
                "sourceStageEndIndex": end,
                "sourceStagePath": source_stage_path,
                "orderedClips": [
                    {
                        key: value
                        for key, value in row.items()
                        if key != "sourceStageClipCount"
                    }
                    for row in selected
                ],
                "stages": [stage_by_index[index] for index in source_stage_path],
            }
        (
            branch["sourceVisualFamilySignatureSha256"],
            branch["sourceVisualFamilyMemberSha256s"],
        ) = source_visual_family_signature(branch)
        branches.append(branch)
    if cursor != len(flattened):
        raise InventoryError(
            f"Valtan clipseq does not cover every source stage clip: "
            f"{pattern_id}/{action_id} ({cursor}/{len(flattened)})"
        )
    return branches


def source_visual_signature_candidate(branch: dict[str, Any]) -> dict[str, Any]:
    return {
        "branchId": branch["branchId"],
        "sourceActionId": branch["sourceActionId"],
        "profileId": branch["profileId"],
        "sequenceIndex": branch["sequenceIndex"],
        "sourceSequencePathSha256": branch["sourceSequencePathSha256"],
        "sourceVisualFamilySignatureSha256": branch[
            "sourceVisualFamilySignatureSha256"
        ],
        "sourceVisualFamilyMemberSha256s": copy.deepcopy(
            branch["sourceVisualFamilyMemberSha256s"]
        ),
    }


def build_source_visual_signature_reviews(
    branches: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    by_identity: dict[tuple[str, int, int], dict[str, Any]] = {}
    by_pattern: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for branch in branches:
        identity = (
            branch["patternId"],
            branch["sourceActionId"],
            branch["sequenceIndex"],
        )
        if identity in by_identity:
            raise InventoryError(
                f"duplicate source visual sequence identity: {identity}"
            )
        by_identity[identity] = branch
        by_pattern[branch["patternId"]].append(branch)

    reviews = []
    for pattern_id, identities in sorted(
        SOURCE_VISUAL_SIGNATURE_EQUIVALENCE_REVIEWS.items()
    ):
        candidates = []
        for source_action_id, sequence_index in identities:
            branch = by_identity.get(
                (pattern_id, source_action_id, sequence_index)
            )
            if branch is None:
                raise InventoryError(
                    "source visual equivalence review candidate disappeared: "
                    f"{pattern_id}/{source_action_id}/{sequence_index}"
                )
            candidates.append(source_visual_signature_candidate(branch))
        candidates.sort(
            key=lambda row: (
                row["sourceActionId"],
                row["sequenceIndex"],
                row["profileId"],
            )
        )
        signatures = {
            row["sourceVisualFamilySignatureSha256"] for row in candidates
        }
        if len(signatures) != 1:
            raise InventoryError(
                "reviewed source visual signatures are not byte-equivalent: "
                + pattern_id
            )
        reviews.append(
            {
                "patternId": pattern_id,
                "status": "SOURCE_VISUAL_SIGNATURE_EQUIVALENT",
                "reviewBasis": (
                    "exact unique clip-local visual cue payloads are "
                    "byte-equivalent; repeated source stages remain occurrences"
                ),
                "canonicalCandidate": copy.deepcopy(candidates[0]),
                "equivalentCandidates": candidates,
                "admissionDisposition": (
                    "SOURCE_TIMING_REVIEW_REQUIRED"
                    if pattern_id == "VALTAN_STOMP"
                    else "SELECTION_MANIFEST_STAGE_MAPPING_REQUIRED"
                ),
            }
        )

    for pattern_id in sorted(SOURCE_VISUAL_SIGNATURE_AMBIGUITIES):
        candidates = sorted(
            (
                source_visual_signature_candidate(branch)
                for branch in by_pattern.get(pattern_id, [])
            ),
            key=lambda row: (
                row["sourceActionId"],
                row["sequenceIndex"],
                row["profileId"],
            ),
        )
        signatures = sorted(
            {
                row["sourceVisualFamilySignatureSha256"]
                for row in candidates
            }
        )
        if len(signatures) < 2:
            raise InventoryError(
                "source visual ambiguity unexpectedly became equivalent: "
                + pattern_id
            )
        reviews.append(
            {
                "patternId": pattern_id,
                "status": "AMBIGUOUS_SOURCE_VISUAL_SIGNATURE",
                "reviewBasis": (
                    "candidate visual payloads or timing splits are not "
                    "byte-equivalent"
                ),
                "canonicalCandidate": None,
                "equivalentCandidates": candidates,
                "candidateVisualSignatureSha256s": signatures,
                "admissionDisposition": "UNRESOLVED_BRANCH_SELECTION",
            }
        )
    reviews.sort(key=lambda row: row["patternId"])
    return reviews


def build_additional_source_selection_audit(
    branches: list[dict[str, Any]],
    source_visual_signature_reviews: list[dict[str, Any]],
    reviewed_selections: list[dict[str, Any]],
    reviewed_mappings: dict[
        str, dict[tuple[int, int], dict[str, Any]]
    ],
) -> list[dict[str, Any]]:
    """Keep the original twelve-pattern follow-up review explicit.

    A safe admission must be one of the byte-equivalent visual-family
    candidates and must map every source clip coordinate in the selected full
    sequence to one exact PR #127 clip occurrence.  A same-name clip or a best
    LCS score is never enough.
    """
    branch_by_id = {str(row["branchId"]): row for row in branches}
    review_by_pattern = {
        str(row["patternId"]): row
        for row in source_visual_signature_reviews
    }
    selected_by_pattern: dict[str, dict[str, Any]] = {}
    for selection in reviewed_selections:
        if selection.get("status") != "REVIEWED_SELECTED":
            continue
        pattern_id = str(selection.get("patternId") or "")
        if pattern_id not in ADDITIONAL_SOURCE_SELECTION_REVIEW_ORDER:
            continue
        if pattern_id in selected_by_pattern:
            raise InventoryError(
                "additional source selection review has two selected branches: "
                + pattern_id
            )
        selected_by_pattern[pattern_id] = selection

    rows = []
    for pattern_id in ADDITIONAL_SOURCE_SELECTION_REVIEW_ORDER:
        safe = SAFE_ADDITIONAL_SOURCE_SELECTIONS.get(pattern_id)
        selected = selected_by_pattern.get(pattern_id)
        if safe is not None:
            candidate_branch_id = str(safe["branchId"])
            branch = branch_by_id.get(candidate_branch_id)
            if branch is None:
                raise InventoryError(
                    "safe additional source branch disappeared: "
                    + candidate_branch_id
                )
            review = review_by_pattern.get(pattern_id)
            equivalent_branch_ids = {
                str(row.get("branchId") or "")
                for row in (review or {}).get("equivalentCandidates", [])
            }
            if (
                (review or {}).get("status")
                != "SOURCE_VISUAL_SIGNATURE_EQUIVALENT"
                or candidate_branch_id not in equivalent_branch_ids
            ):
                raise InventoryError(
                    "safe additional source branch lost byte-equivalence proof: "
                    + pattern_id
                )

            selected_branch_id = None
            selection_status = "UNRESOLVED_BRANCH_SELECTION"
            unresolved_reason = (
                "the exact-equivalent safe branch is reviewed but is not "
                "present in the source selection manifest"
            )
            if selected is not None:
                if selected.get("branchId") != candidate_branch_id:
                    raise InventoryError(
                        "additional source selection chose a non-safe branch: "
                        + pattern_id
                    )
                mappings = reviewed_mappings.get(candidate_branch_id, {})
                source_coordinates = {
                    (
                        int(row["sourceStageIndex"]),
                        int(row["sourceClipOrdinal"]),
                    )
                    for row in branch.get("orderedClips", [])
                }
                if set(mappings) != source_coordinates:
                    raise InventoryError(
                        "safe additional source selection does not map the "
                        "complete source sequence: "
                        + pattern_id
                    )
                for mapping in mappings.values():
                    if (
                        mapping.get("timingDisposition") != "REACHABLE"
                        or not str(mapping.get("clipOccurrenceId") or "")
                        or not str(mapping.get("reviewBasis") or "")
                    ):
                        raise InventoryError(
                            "safe additional source mapping lost exact timing "
                            "evidence: "
                            + pattern_id
                        )
                selection_status = "REVIEWED_SELECTED"
                selected_branch_id = candidate_branch_id
                unresolved_reason = None
            rows.append(
                {
                    "patternId": pattern_id,
                    "eligibilityDisposition": (
                        "SAFE_EXACT_EQUIVALENT_FULL_SEQUENCE_JOIN"
                    ),
                    "selectionStatus": selection_status,
                    "candidateBranchId": candidate_branch_id,
                    "selectedBranchId": selected_branch_id,
                    "reviewBasis": str(safe["reviewBasis"]),
                    "unresolvedReason": unresolved_reason,
                }
            )
            continue

        if selected is not None:
            raise InventoryError(
                "unresolved additional source pattern was selected without "
                "exact-equivalence proof: "
                + pattern_id
            )
        missing_status = MISSING_ACTION_BINDING_POLICIES.get(pattern_id)
        rows.append(
            {
                "patternId": pattern_id,
                "eligibilityDisposition": (
                    "UNRESOLVED_ACTION_BINDING_GAP"
                    if missing_status is not None
                    else "UNRESOLVED_SOURCE_VISUAL_OR_TIMING_CONFLICT"
                ),
                "selectionStatus": (
                    missing_status or "UNRESOLVED_BRANCH_SELECTION"
                ),
                "candidateBranchId": None,
                "selectedBranchId": None,
                "reviewBasis": (
                    "Animation PR #127 bindings and complete Valtan.clipseq "
                    "paths were compared without changing product clips"
                ),
                "unresolvedReason": (
                    UNRESOLVED_ADDITIONAL_SOURCE_SELECTION_REASONS[pattern_id]
                ),
            }
        )
    return rows


def lcs_length(left: list[str], right: list[str]) -> int:
    previous = [0] * (len(right) + 1)
    for lhs in left:
        current = [0]
        for index, rhs in enumerate(right, start=1):
            if lhs == rhs:
                current.append(previous[index - 1] + 1)
            else:
                current.append(max(previous[index], current[-1]))
        previous = current
    return previous[-1]


def score_branch(
    branch: dict[str, Any], product_clips: list[dict[str, Any]]
) -> dict[str, Any]:
    product = [
        row["normalizedClip"]
        for row in product_clips
        if not is_idle_clip(row["normalizedClip"])
    ]
    source = [
        row["normalizedClip"]
        for row in branch["orderedClips"]
        if not is_idle_clip(row["normalizedClip"])
    ]
    matched = lcs_length(product, source)
    return {
        "productNonIdleClipCount": len(product),
        "sourceNonIdleClipCount": len(source),
        "orderedMatchCount": matched,
        "missingProductClipCount": len(product) - matched,
        "extraSourceClipCount": len(source) - matched,
        "productIsSubsequence": matched == len(product),
    }


def choose_branch_recommendations(
    branches: list[dict[str, Any]], product_clips: list[dict[str, Any]]
) -> None:
    grouped: dict[tuple[str, int, str], list[dict[str, Any]]] = defaultdict(list)
    for branch in branches:
        branch["matchScore"] = score_branch(branch, product_clips)
        grouped[
            (
                branch["patternId"],
                branch["sourceActionId"],
                branch["profileId"],
            )
        ].append(branch)

    for candidates in grouped.values():
        best_value = min(
            (
                row["matchScore"]["missingProductClipCount"],
                row["matchScore"]["extraSourceClipCount"],
            )
            for row in candidates
        )
        best = [
            row
            for row in candidates
            if (
                row["matchScore"]["missingProductClipCount"],
                row["matchScore"]["extraSourceClipCount"],
            )
            == best_value
        ]
        for row in candidates:
            if row in best and len(best) == 1:
                row["recommendationStatus"] = "UNIQUE_BEST_CANDIDATE"
            elif row in best:
                row["recommendationStatus"] = "TIED_BEST_CANDIDATE"
            else:
                row["recommendationStatus"] = "ALTERNATE_BRANCH_CANDIDATE"
            row["selectionStatus"] = "UNRESOLVED_BRANCH_SELECTION"


def reviewed_selection_index(
    previous: dict[str, Any] | None,
) -> tuple[
    list[dict[str, Any]],
    dict[str, dict[str, Any]],
    dict[str, dict[tuple[int, int], dict[str, Any]]],
]:
    selections = copy.deepcopy(
        (previous or {}).get("reviewedBranchSelections", [])
    )
    result: dict[str, dict[str, Any]] = {}
    mappings: dict[str, dict[tuple[int, int], dict[str, Any]]] = {}
    selected_groups: set[tuple[str, int, str]] = set()
    for row in selections:
        branch_id = str(row.get("branchId") or "")
        status = str(row.get("status") or "")
        if not branch_id or status not in {
            "REVIEWED_SELECTED",
            "REVIEWED_REJECTED",
        }:
            raise InventoryError("reviewed branch selection is malformed")
        if branch_id in result:
            raise InventoryError(f"duplicate reviewed branch: {branch_id}")
        result[branch_id] = copy.deepcopy(row)
        branch_mappings: dict[tuple[int, int], dict[str, Any]] = {}
        for mapping in row.get("stageMappings", []):
            if not isinstance(mapping, dict):
                raise InventoryError("reviewed source stage mapping is malformed")
            try:
                source_stage = int(mapping["sourceStageIndex"])
                source_clip = int(mapping["sourceClipOrdinal"])
            except (KeyError, TypeError, ValueError) as error:
                raise InventoryError(
                    "reviewed source stage mapping coordinate is invalid"
                ) from error
            timing = str(mapping.get("timingDisposition") or "")
            clip_occurrence_id = mapping.get("clipOccurrenceId")
            if timing not in {
                "REACHABLE",
                "SOURCE_TIMING_REVIEW_REQUIRED",
                "UNREACHABLE",
            }:
                raise InventoryError(
                    "reviewed source stage mapping timing disposition is invalid"
                )
            if timing == "REACHABLE" and not str(clip_occurrence_id or ""):
                raise InventoryError(
                    "reachable source stage mapping has no clipOccurrenceId"
                )
            if clip_occurrence_id is not None and not isinstance(
                clip_occurrence_id, str
            ):
                raise InventoryError(
                    "reviewed source stage mapping clipOccurrenceId is invalid"
                )
            if timing == "UNREACHABLE" and clip_occurrence_id is not None:
                raise InventoryError(
                    "unreachable source stage mapping must not name a "
                    "clipOccurrenceId"
                )
            coordinate = (source_stage, source_clip)
            if coordinate in branch_mappings:
                raise InventoryError(
                    "reviewed source stage mapping coordinate is duplicated"
                )
            branch_mappings[coordinate] = copy.deepcopy(mapping)
        mappings[branch_id] = branch_mappings
        if status == "REVIEWED_SELECTED":
            group = (
                str(row.get("patternId") or ""),
                int(row.get("sourceActionId")),
                str(row.get("profileId") or ""),
            )
            if group in selected_groups:
                raise InventoryError(
                    "one source action/profile has two selected branches"
                )
            selected_groups.add(group)
    return selections, result, mappings


def load_selection_manifest(path: Path) -> dict[str, Any]:
    document = read_json(path)
    if (
        document.get("schema")
        != "lostark.valtan-source-branch-selections"
        or document.get("formatVersion") != 1
        or document.get("bossArchetypeId") != "BOSS_VALTAN"
        or not isinstance(document.get("selections"), list)
    ):
        raise InventoryError(f"branch selection manifest header is invalid: {path}")
    for row in document["selections"]:
        if not isinstance(row, dict):
            raise InventoryError("branch selection row is not an object")
        required = {
            "patternId",
            "sourceActionId",
            "profileId",
            "sequenceIndex",
            "sourceSequencePathSha256",
            "branchId",
            "status",
            "reviewBasis",
            "stageMappings",
        }
        if not required.issubset(row) or not str(row.get("reviewBasis") or ""):
            raise InventoryError("branch selection row has no review evidence")
        if not isinstance(row.get("stageMappings"), list):
            raise InventoryError("branch selection row stageMappings is not a list")
        if (
            isinstance(row.get("sequenceIndex"), bool)
            or not isinstance(row.get("sequenceIndex"), int)
            or row["sequenceIndex"] < 0
            or not re.fullmatch(
                r"[0-9a-f]{64}",
                str(row.get("sourceSequencePathSha256") or ""),
            )
        ):
            raise InventoryError("branch selection sequence identity is invalid")
    return document


def is_explicit_generic_dust(source_asset: str) -> bool:
    parts = [part.casefold() for part in source_asset.split(".")]
    if any(part == "dust" or part.startswith("dust_") for part in parts[:-1]):
        return True
    object_name = parts[-1] if parts else ""
    return bool(
        re.search(r"(?:^|_)dust(?:_|$)", object_name)
        or re.search(r"^par_[a-z0-9]+_dust", object_name)
    )


def graph_specs(catalog: dict[str, Any]) -> list[tuple[str, Path]]:
    specs: list[tuple[str, Path]] = []
    seen: set[str] = set()
    for row in catalog.get("sourcePackageGraphs", []):
        package = str(row.get("logicalPackage") or "")
        path = Path(str(row.get("graphFile") or ""))
        if not package or not path.is_file():
            raise InventoryError(f"source graph is missing: {package}={path}")
        key = package.casefold()
        if key in seen:
            raise InventoryError(f"duplicate source graph package: {package}")
        seen.add(key)
        specs.append((package, path))
    if not specs:
        raise InventoryError("Valtan source catalog has no sourcePackageGraphs")
    return specs


def graph_source_rows(specs: list[tuple[str, Path]]) -> list[dict[str, Any]]:
    common = Path(os.path.commonpath([str(path.parent) for _, path in specs]))
    return [
        {
            "logicalPackage": package,
            "logicalPath": path.relative_to(common).as_posix(),
            "sha256": sha256_file(path),
        }
        for package, path in sorted(specs, key=lambda row: row[0].casefold())
    ]


def runtime_cook_receipt(
    specs: list[tuple[str, Path]],
) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    artifact_roots = {path.parent.parent.resolve() for _, path in specs}
    if len(artifact_roots) != 1:
        raise InventoryError(
            "Valtan source graphs do not share one runtime cook artifact root"
        )
    artifact_root = next(iter(artifact_roots))
    path = artifact_root / "runtime-cook-receipt.json"
    if not path.is_file():
        raise InventoryError(f"Valtan runtime cook receipt is missing: {path}")
    document = read_json(path)
    if (
        document.get("schema")
        != "lostark.effect-runtime-resource-cook-receipt"
        or document.get("formatVersion") != 1
        or document.get("characterClass") != "VALTAN"
        or document.get("failures") != []
        or not isinstance(document.get("assets"), list)
    ):
        raise InventoryError("Valtan runtime cook receipt header is invalid")

    successful_statuses = {
        "COPIED",
        "COOKED",
        "CONVERTED_TGA_TO_DDS",
    }
    bindings: list[dict[str, Any]] = []
    seen_sources: set[str] = set()
    for asset in document["assets"]:
        source_path = str(asset.get("sourceAssetPath") or "")
        runtime_asset_id = str(asset.get("runtimeAssetId") or "")
        status = str(asset.get("status") or "")
        digest = str(asset.get("sha256") or "")
        runtime_file = Path(str(asset.get("runtimeFile") or ""))
        relative = PurePosixPath(runtime_asset_id)
        if (
            not source_path
            or not runtime_asset_id.startswith("Effect/Valtan/")
            or "\\" in runtime_asset_id
            or ":" in runtime_asset_id
            or relative.is_absolute()
            or any(part in {"", ".", ".."} for part in relative.parts)
            or status not in successful_statuses
            or not re.fullmatch(r"[0-9a-f]{64}", digest)
            or not runtime_file.is_file()
            or int(asset.get("byteSize", -1)) != runtime_file.stat().st_size
            or sha256_file(runtime_file) != digest
        ):
            raise InventoryError(
                f"Valtan runtime cook asset is invalid: {source_path}"
            )
        key = source_path.casefold()
        if key in seen_sources:
            raise InventoryError(
                f"Valtan runtime cook source asset is duplicated: {source_path}"
            )
        seen_sources.add(key)
        bindings.append(
            {
                "role": str(asset.get("role") or ""),
                "sourceObjectPath": source_path,
                "resolutionStatus": "RESOLVED_RUNTIME_ASSET",
                "assetId": runtime_asset_id,
                "candidateCount": 1,
                "sha256": digest,
                "cookStatus": status,
            }
        )
    bindings.sort(key=lambda row: row["sourceObjectPath"].casefold())
    return bindings, {
        "logicalPath": path.relative_to(artifact_root).as_posix(),
        "sha256": sha256_file(path),
        "assetCount": len(bindings),
        "verifiedRuntimeFileCount": len(bindings),
        "failureCount": 0,
    }


def material_rows_for_system(
    catalog: dict[str, Any], graph: dict[str, Any]
) -> list[dict[str, Any]]:
    wanted = {
        str(row.get("objectPath") or "").casefold()
        for row in graph.get("resourceBindings", [])
        if row.get("role") == "material" and row.get("objectPath")
    }
    indexed = {
        str(row.get("sourceMaterialPath") or "").casefold(): row
        for row in catalog.get("materialParameterBindings", [])
        if row.get("sourceMaterialPath")
    }
    return [copy.deepcopy(indexed[key]) for key in sorted(wanted) if key in indexed]


def root_emitter_rows(
    graph: dict[str, Any], normalized_graph: dict[str, Any]
) -> list[dict[str, Any]]:
    root = str(graph["rootNodeId"])
    nodes = {str(row["nodeId"]): row for row in normalized_graph["nodes"]}
    rows = []
    for edge in normalized_graph["edges"]:
        if str(edge.get("sourceNodeId")) != root:
            continue
        if imported_effects.base_property_name(str(edge.get("property") or "")) != "emitters":
            continue
        target = nodes.get(str(edge.get("targetNodeId") or ""))
        if target is not None:
            rows.append(
                {
                    "referenceIndex": int(edge.get("referenceIndex", 0)),
                    "sourceEmitterNodeId": str(target["nodeId"]),
                    "sourceEmitterPath": str(target.get("objectPath") or ""),
                }
            )
    rows.sort(key=lambda row: row["referenceIndex"])
    return rows


def module_occurrences(modules: Iterable[Any]) -> list[dict[str, Any]]:
    rows = []
    for ordinal, module in enumerate(modules):
        stable_id, separator, raw_reference = str(module.source_id).partition(
            "@ref:"
        )
        rows.append(
            {
                "ordinal": ordinal,
                "sourceNodeId": stable_id,
                "referenceIndex": int(raw_reference) if separator else ordinal,
                "className": str(module.class_name).casefold(),
                "objectPath": str(module.object_path),
            }
        )
    return rows


def source_resource_closure(
    system: dict[str, Any], module_ids: set[str], catalog: dict[str, Any]
) -> list[dict[str, Any]]:
    materials = {
        str(row.get("sourceMaterialPath") or "").casefold(): row
        for row in catalog.get("materialParameterBindings", [])
    }
    result = []
    for binding in system.get("resourceBindings", []):
        if str(binding.get("sourceNodeId")) not in module_ids:
            continue
        row: dict[str, Any] = {
            "sourceNodeId": str(binding.get("sourceNodeId") or ""),
            "referenceIndex": int(binding.get("referenceIndex", 0)),
            "role": str(binding.get("role") or ""),
            "objectPath": str(binding.get("objectPath") or ""),
        }
        material = materials.get(row["objectPath"].casefold())
        if material is not None:
            row["materialResolutionStatus"] = str(
                material.get("resolutionStatus") or ""
            )
            row["textures"] = copy.deepcopy(material.get("textures", []))
        result.append(row)
    return sorted(
        result,
        key=lambda row: (
            row["sourceNodeId"],
            row["referenceIndex"],
            row["role"],
            row["objectPath"].casefold(),
        ),
    )


def carrier_element_seed(
    source_system_id: str,
    carrier_key: str,
    kind: str,
    renderer_shape: str,
    emitter_path: str,
    detail: dict[str, Any],
    source_recipe: dict[str, Any],
    resources: list[dict[str, Any]],
    material_rows: list[dict[str, Any]],
) -> dict[str, Any]:
    source_material = (
        str(material_rows[0].get("sourceMaterialPath") or "")
        if material_rows
        else ""
    )
    render_profile = (
        "additive_two_sided_depth_read"
        if any(token in source_material.casefold() for token in ("_ad", "add"))
        else "alpha_two_sided_depth_read"
    )
    return {
        "id": f"source.{sha256_bytes(carrier_key.encode('utf-8'))[:20]}",
        "displayName": emitter_path.rsplit(".", 1)[-1][:64],
        "groupId": stable_slug(source_system_id, 120),
        "sourceNode": carrier_key,
        "visible": True,
        "kind": kind,
        "resources": copy.deepcopy(resources),
        "material": {
            "templateId": "effect.standard",
            "sourceMaterialPath": source_material,
            "renderProfile": render_profile,
            "sourceProfile": {"enabled": False},
        },
        "detail": copy.deepcopy(detail),
        "sourceRecipe": copy.deepcopy(source_recipe),
        "sourcePresentation": imported_effects.default_source_presentation(),
        "inventoryRendererShape": renderer_shape,
    }


def build_system_inventory(
    catalog: dict[str, Any],
    graph_index: dict[str, Any],
    catalog_system: dict[str, Any],
    runtime_resource_bindings: list[dict[str, Any]],
    *,
    include_payloads: bool = False,
) -> dict[str, Any]:
    source_asset = str(catalog_system.get("sourceAsset") or "")
    parts = source_receipts.source_asset_parts(source_asset)
    if parts is None:
        raise InventoryError(f"invalid source system identity: {source_asset}")
    root = source_receipts.find_particle_system(graph_index, *parts)
    if root is None:
        raise InventoryError(f"source graph has no exact system: {source_asset}")
    package_key, root_row = root
    graph = source_receipts.collect_system_graph(graph_index, package_key, root_row)
    catalog_root = str((catalog_system.get("graph") or {}).get("rootNodeId") or "")
    if catalog_root and catalog_root.casefold() != graph["rootNodeId"].casefold():
        raise InventoryError(f"source graph root drifted: {source_asset}")

    system_id = source_asset.casefold()
    system = {
        "sourceSystemId": system_id,
        "sourceAsset": source_asset,
        "logicalPackage": graph_index["packages"][package_key]["package"],
        "objectName": str(root_row.get("objectName") or ""),
        "objectPath": str(root_row.get("objectPath") or ""),
        "rootNodeId": graph["rootNodeId"],
        "nodeIds": graph["nodeIds"],
        "resourceBindings": graph["resourceBindings"],
        "unresolvedExternalReferences": graph["unresolvedExternalReferences"],
        "summary": graph["summary"],
    }
    normalized_graph = {
        "schema": "lostark.normalized-effect-source-graph",
        "schemaVersion": 1,
        "characterClass": "VALTAN",
        "skillId": 0,
        "sourceSystems": [system],
        "nodes": [graph["nodes"][key] for key in sorted(graph["nodes"])],
        "edges": graph["edges"],
        "materialParameterBindings": material_rows_for_system(catalog, graph),
        "runtimeResourceBindings": runtime_resource_bindings,
    }
    index = imported_effects.SourceIndex(normalized_graph, {"packages": []})
    partitions = imported_effects.selected_lod_partitions(
        normalized_graph, index, {"packages": []}
    )
    root_emitters = root_emitter_rows(graph, normalized_graph)
    emitter_occurrences: Counter[str] = Counter()
    carriers: list[dict[str, Any]] = []
    selected_emitters: Counter[str] = Counter()
    generic_dust = is_explicit_generic_dust(source_asset)

    for partition_ordinal, (selected_system, emitter, lod, modules) in enumerate(
        partitions
    ):
        occurrence_ordinal = emitter_occurrences[emitter.source_id]
        emitter_occurrences[emitter.source_id] += 1
        selected_emitters[emitter.source_id] += 1
        module_rows = module_occurrences(modules)
        module_order_hash = canonical_sha256(module_rows)
        carrier_key = (
            f"system={system_id}|emitter={emitter.source_id}"
            f"|emitterOccurrence={occurrence_ordinal}"
            f"|lod={lod.source_id}|moduleOrder={module_order_hash}"
        )
        kind, unsupported_reason, renderer_shape = imported_effects.classify(
            selected_system, modules
        )
        conversion_status = "SOURCE_PRIMITIVES_READY"
        conversion_blockers: list[str] = []
        detail: dict[str, Any] | None = None
        source_recipe: dict[str, Any] | None = None
        runtime_resources: list[dict[str, Any]] = []
        resource_receipt: list[dict[str, Any]] = []
        material_rows: list[dict[str, Any]] = []
        portable_source_recipe: dict[str, Any] | None = None
        if unsupported_reason:
            conversion_status = "UNRESOLVED_IMPORT_PRIMITIVE"
            conversion_blockers.append(str(unsupported_reason))
        else:
            try:
                detail, detail_mappings, bursts = imported_effects.emitter_detail(
                    index,
                    lod,
                    modules,
                    0.0,
                    0.0,
                    partition_ordinal + 1,
                )
                source_recipe = imported_effects.build_source_recipe(
                    index, modules, str(renderer_shape), bursts
                )
                module_ids = {
                    lod.source_id,
                    emitter.source_id,
                    *(module.source_id.split("@ref:", 1)[0] for module in modules),
                }
                runtime_resources, resource_receipt, material_rows = (
                    imported_effects.choose_resources(
                        selected_system, module_ids, normalized_graph
                    )
                )
                imported_effects.material_detail(
                    material_rows, detail, detail_mappings
                )
                if renderer_shape == "mesh":
                    # Follow the ordinary character Cascade importer. UE3
                    # bOverrideMaterial is the inverse of the runtime
                    # useModelMaterial flag; treating every mesh as embedded-
                    # material made valid Valtan carriers prepare/draw zero.
                    detail["mesh"]["useModelMaterial"] = (
                        imported_effects.mesh_uses_model_material(
                            modules, detail_mappings
                        )
                    )
                    detail["particle"]["billboard"] = False
                    has_runtime_override_texture = any(
                        row.get("slotId") != "meshModel"
                        for row in runtime_resources
                    )
                    if (
                        not detail["mesh"]["useModelMaterial"]
                        and not has_runtime_override_texture
                    ):
                        # A texture-free procedural override is not executable
                        # in the ordinary Effect renderer. Keep the exact model
                        # carrier and use its embedded material instead.
                        detail["mesh"]["useModelMaterial"] = True
                    if (
                        not detail["mesh"]["useModelMaterial"]
                        and not any(
                            row.get("slotId") == "base"
                            for row in runtime_resources
                        )
                    ):
                        # Some UE3 override materials expose their only color
                        # carrier through mask/emissive/noise. Rebind that exact
                        # source texture to the required runtime base lane.
                        color_carrier = next(
                            (
                                row
                                for preferred_slot in (
                                    "emissive",
                                    "mask",
                                    "dissolve",
                                    "noise",
                                )
                                for row in runtime_resources
                                if row.get("slotId") == preferred_slot
                            ),
                            None,
                        )
                        if color_carrier is not None:
                            runtime_resources.append(
                                {
                                    "slotId": "base",
                                    "assetId": color_carrier["assetId"],
                                }
                            )
                            resource_receipt.append(
                                {
                                    "slotId": "base",
                                    "sourceObjectPath": "",
                                    "assetId": color_carrier["assetId"],
                                    "status": (
                                        "EXACT_SOURCE_TEXTURE_RUNTIME_BASE_ADAPTER_"
                                        + str(color_carrier["slotId"]).upper()
                                    ),
                                    "sourceSlotId": color_carrier["slotId"],
                                }
                            )
                portable_source_recipe = portable_recipe(source_recipe)
            except MaterializeError as error:
                conversion_status = "UNRESOLVED_RUNTIME_ADAPTER"
                conversion_blockers.append(
                    "PORTABLE_RUNTIME_CARRIER_VALIDATION_FAILED: "
                    + str(error)
                )
            except (KeyError, TypeError, ValueError) as error:
                conversion_status = "UNRESOLVED_IMPORT_PRIMITIVE"
                conversion_blockers.append(type(error).__name__)

        module_ids = {
            lod.source_id,
            emitter.source_id,
            *(module.source_id.split("@ref:", 1)[0] for module in modules),
        }
        ribbon_adapter_required = any(
            row["className"] == "particlemoduletypedataribbon"
            for row in module_rows
        )
        if generic_dust:
            disposition = "DEFERRED_GENERIC_DUST"
        elif renderer_shape == "light":
            disposition = "DEFERRED_LIGHT"
        elif ribbon_adapter_required:
            disposition = "UNRESOLVED_RUNTIME_ADAPTER"
            conversion_status = "UNRESOLVED_RUNTIME_ADAPTER"
            conversion_blockers.append("RIBBON_RUNTIME_ADAPTER_UNAVAILABLE")
        elif conversion_status == "UNRESOLVED_RUNTIME_ADAPTER":
            disposition = "UNRESOLVED_RUNTIME_ADAPTER"
        elif conversion_status != "SOURCE_PRIMITIVES_READY":
            disposition = "UNRESOLVED_IMPORT_PRIMITIVE"
        elif (
            renderer_shape == "mesh"
            and (
                not any(
                    row.get("slotId") == "meshModel"
                    for row in runtime_resources
                )
                or (
                    detail is not None
                    and not detail["mesh"]["useModelMaterial"]
                    and not any(
                        row.get("slotId") == "base"
                        for row in runtime_resources
                    )
                )
            )
        ) or (
            renderer_shape != "mesh"
            and not any(
                row.get("slotId") == "base"
                for row in runtime_resources
            )
        ):
            disposition = "MISSING_RUNTIME_RESOURCE"
            conversion_status = "MISSING_RUNTIME_RESOURCE"
            conversion_blockers.append(
                "DRAWABLE_BASE_OR_MESH_RUNTIME_BINDING_MISSING"
            )
        elif (
            detail is not None
            and renderer_shape in ("sprite", "mesh")
            and float(detail["particle"]["spawnRatePerSecond"]) <= 0.0
            and portable_source_recipe is not None
            and not portable_source_recipe.get("bursts")
        ):
            disposition = "UNRESOLVED_RUNTIME_ADAPTER"
            conversion_status = "UNRESOLVED_RUNTIME_ADAPTER"
            conversion_blockers.append("NO_EXECUTABLE_PARTICLE_EMISSION")
        else:
            disposition = "EXECUTABLE_CORE"
            conversion_status = "PORTABLE_RUNTIME_CARRIER_READY"

        element_seed = None
        if (
            disposition == "EXECUTABLE_CORE"
            and detail is not None
            and portable_source_recipe is not None
            and kind is not None
        ):
            element_seed = carrier_element_seed(
                system_id,
                carrier_key,
                str(kind),
                str(renderer_shape),
                emitter.object_path,
                detail,
                portable_source_recipe,
                runtime_resources,
                material_rows,
            )
        recipe_summary = None
        if source_recipe is not None:
            recipe_summary = {
                "moduleCount": len(source_recipe.get("modules", [])),
                "distributionCount": sum(
                    len(module.get("distributions", []))
                    for module in source_recipe.get("modules", [])
                ),
                "burstCount": len(source_recipe.get("bursts", [])),
            }
        resource_closure = source_resource_closure(
            selected_system, module_ids, catalog
        )
        carrier_row = {
                "carrierKey": carrier_key,
                "sourceOrder": partition_ordinal,
                "sourceEmitterNodeId": emitter.source_id,
                "sourceEmitterOccurrence": occurrence_ordinal,
                "sourceEmitterPath": emitter.object_path,
                "selectedLodNodeId": lod.source_id,
                "selectedLodPath": lod.object_path,
                "orderedModuleOccurrences": module_rows,
                "moduleOrderSha256": module_order_hash,
                "rendererShape": renderer_shape,
                "kind": kind,
                "disposition": disposition,
                "conversionStatus": conversion_status,
                "conversionBlockers": sorted(set(conversion_blockers)),
                "runtimeAdapterType": (
                    "RIBBON" if ribbon_adapter_required else "DIRECT_EFFECT"
                ),
                "sourceRecipeSha256": (
                    canonical_sha256(source_recipe)
                    if source_recipe is not None
                    else None
                ),
                "portableSourceRecipeSha256": (
                    canonical_sha256(portable_source_recipe)
                    if portable_source_recipe is not None
                    else None
                ),
                "sourceRecipeSummary": recipe_summary,
                "sourceRecipe": (
                    portable_source_recipe if include_payloads else None
                ),
                "runtimeResourceBindingCount": len(runtime_resources),
                "runtimeResourceBindingSha256": canonical_sha256(
                    runtime_resources
                ),
                "runtimeResources": runtime_resources,
                "resourceReceipt": resource_receipt,
                "sourceResourceClosureCount": len(resource_closure),
                "sourceResourceClosureSha256": canonical_sha256(
                    resource_closure
                ),
                "sourceResourceClosure": resource_closure,
                "elementSeed": element_seed if include_payloads else None,
            }
        if not include_payloads:
            for payload_field in (
                "sourceRecipe",
                "runtimeResources",
                "resourceReceipt",
                "sourceResourceClosure",
                "elementSeed",
            ):
                carrier_row.pop(payload_field)
        carriers.append(carrier_row)

    # A root emitter that has no selected LOD must remain explicit evidence;
    # omitting it would make dropped=0 meaningless.
    unresolved_ordinal = len(carriers)
    for emitter in root_emitters:
        node_id = emitter["sourceEmitterNodeId"]
        if selected_emitters[node_id] > 0:
            selected_emitters[node_id] -= 1
            continue
        carrier_key = (
            f"system={system_id}|emitter={node_id}"
            f"|emitterOccurrence=0|lod=UNRESOLVED|moduleOrder=UNRESOLVED"
        )
        carrier_row = {
                "carrierKey": carrier_key,
                "sourceOrder": unresolved_ordinal,
                "sourceEmitterNodeId": node_id,
                "sourceEmitterOccurrence": 0,
                "sourceEmitterPath": emitter["sourceEmitterPath"],
                "selectedLodNodeId": None,
                "selectedLodPath": None,
                "orderedModuleOccurrences": [],
                "moduleOrderSha256": canonical_sha256([]),
                "rendererShape": None,
                "kind": None,
                "disposition": "UNRESOLVED_SELECTED_LOD",
                "conversionStatus": "UNRESOLVED_SELECTED_LOD",
                "conversionBlockers": ["SELECTED_LOD_NOT_RESOLVED"],
                "runtimeAdapterType": "UNRESOLVED",
                "sourceRecipeSha256": None,
                "portableSourceRecipeSha256": None,
                "sourceRecipeSummary": None,
                "sourceRecipe": None,
                "runtimeResourceBindingCount": 0,
                "runtimeResourceBindingSha256": canonical_sha256([]),
                "runtimeResources": [],
                "resourceReceipt": [],
                "sourceResourceClosureCount": 0,
                "sourceResourceClosureSha256": canonical_sha256([]),
                "sourceResourceClosure": [],
                "elementSeed": None,
            }
        if not include_payloads:
            for payload_field in (
                "sourceRecipe",
                "runtimeResources",
                "resourceReceipt",
                "sourceResourceClosure",
                "elementSeed",
            ):
                carrier_row.pop(payload_field)
        carriers.append(carrier_row)
        unresolved_ordinal += 1

    carrier_keys = [row["carrierKey"] for row in carriers]
    if len(carrier_keys) != len(set(carrier_keys)):
        raise InventoryError(f"duplicate carrier full key: {source_asset}")
    return {
        "sourceSystemId": system_id,
        "catalogSourceAsset": source_asset,
        "logicalPackage": system["logicalPackage"],
        "graphRootNodeId": system["rootNodeId"],
        "explicitGenericDust": generic_dust,
        "rootEmitterDenominator": len(root_emitters),
        "selectedLodCarrierCount": len(partitions),
        "preservedUnresolvedEmitterCount": len(carriers) - len(partitions),
        "droppedCarrierCount": 0,
        "duplicateCarrierCount": 0,
        "carriers": carriers,
    }


def asset_reference_rows(cue: dict[str, Any]) -> list[dict[str, Any] | None]:
    references = cue.get("assetReferences", [])
    if not references:
        return [None]
    return [copy.deepcopy(row) for row in references]


def source_stage_product_matches(
    stage: dict[str, Any],
    product_clips: list[dict[str, Any]],
    branch_selection_status: str,
    reviewed_mappings: dict[tuple[int, int], dict[str, Any]],
) -> list[dict[str, Any]]:
    """Keep one row per source clip; product occurrences stay candidates.

    A repeated product clip name is not enough evidence to choose a product
    occurrence.  Only a reviewed source-stage/clip-ordinal mapping may attach
    one exact ``clipOccurrenceId`` and make the source occurrence reachable.
    """
    source_stage_index = int(stage["stageIndex"])
    product_by_id = {
        row["clipOccurrenceId"]: row for row in product_clips
    }
    matches = []
    for source_ordinal, source_clip in enumerate(stage.get("animationClips", [])):
        normalized = normalize_clip(source_clip)
        product_candidates = [
            row for row in product_clips if row["normalizedClip"] == normalized
        ]
        mapping = reviewed_mappings.get((source_stage_index, source_ordinal))
        mapped_product = None
        timing_disposition = "SOURCE_TIMING_REVIEW_REQUIRED"
        mapping_review_basis = None
        if branch_selection_status == "REVIEWED_REJECTED":
            timing_disposition = "UNREACHABLE"
        elif mapping is not None:
            timing_disposition = str(mapping["timingDisposition"])
            mapping_review_basis = mapping.get("reviewBasis")
            clip_occurrence_id = mapping.get("clipOccurrenceId")
            if clip_occurrence_id is not None:
                mapped_product = product_by_id[str(clip_occurrence_id)]
        matches.append(
            {
                "sourceClipOrdinal": source_ordinal,
                "sourceClip": str(source_clip),
                "candidateClipOccurrenceIds": [
                    row["clipOccurrenceId"] for row in product_candidates
                ],
                "clipOccurrenceId": (
                    mapped_product["clipOccurrenceId"]
                    if mapped_product is not None
                    else None
                ),
                "semanticStageId": (
                    mapped_product["semanticStageId"]
                    if mapped_product is not None
                    else None
                ),
                "gameplayActionId": (
                    mapped_product["gameplayActionId"]
                    if mapped_product is not None
                    else None
                ),
                "timingDisposition": timing_disposition,
                "mappingReviewBasis": mapping_review_basis,
                "hasExplicitReviewedMapping": mapping is not None,
            }
        )
    if not matches:
        matches.append(
            {
                "sourceClipOrdinal": None,
                "sourceClip": None,
                "candidateClipOccurrenceIds": [],
                "clipOccurrenceId": None,
                "semanticStageId": None,
                "gameplayActionId": None,
                "timingDisposition": (
                    "UNREACHABLE"
                    if branch_selection_status == "REVIEWED_REJECTED"
                    else "SOURCE_TIMING_REVIEW_REQUIRED"
                ),
                "mappingReviewBasis": None,
                "hasExplicitReviewedMapping": False,
            }
        )
    return matches


def occurrence_disposition(
    source_type: str,
    category: str,
    asset_reference: dict[str, Any] | None,
    source_system: dict[str, Any] | None,
) -> str:
    class_name = str(
        (asset_reference or {}).get("className") or ""
    ).casefold()
    if (
        source_type.casefold() == "trailghosteffect"
        or class_name == "efdata_animnotify_trails"
    ):
        return "UNRESOLVED_RUNTIME_ADAPTER"
    if asset_reference is None:
        return "UNRESOLVED_SOURCE_PAYLOAD"
    if class_name != "particlesystem":
        return "PRESERVED_NON_PARTICLE_SOURCE_ASSET"
    if source_system is None:
        return "UNRESOLVED_SOURCE_SYSTEM"
    dispositions = {row["disposition"] for row in source_system["carriers"]}
    if dispositions == {"DEFERRED_LIGHT"}:
        return "DEFERRED_LIGHT"
    if source_system["explicitGenericDust"]:
        return "DEFERRED_GENERIC_DUST"
    if "EXECUTABLE_CORE" in dispositions:
        return "EXECUTABLE_CORE"
    if "MISSING_RUNTIME_RESOURCE" in dispositions:
        return "MISSING_RUNTIME_RESOURCE"
    if "UNRESOLVED_RUNTIME_ADAPTER" in dispositions:
        return "UNRESOLVED_RUNTIME_ADAPTER"
    if category in {"decal", "unresolved_effect"}:
        return "UNRESOLVED_SOURCE_PAYLOAD"
    return "UNRESOLVED_SOURCE_CARRIER"


def build_occurrences(
    patterns: list[dict[str, Any]],
    branches: list[dict[str, Any]],
    product_clips: dict[str, list[dict[str, Any]]],
    source_systems: dict[str, dict[str, Any]],
    reviewed_mappings: dict[
        str, dict[tuple[int, int], dict[str, Any]]
    ],
) -> list[dict[str, Any]]:
    pattern_source_actions: dict[
        tuple[str, int, str], dict[str, Any]
    ] = {}
    for pattern in patterns:
        for action in pattern.get("sourceActions", []):
            key = (
                str(pattern["patternId"]),
                int(action["sourceActionId"]),
                str(action.get("profileId") or ""),
            )
            if key in pattern_source_actions:
                raise InventoryError(f"duplicate source action/profile: {key}")
            pattern_source_actions[key] = action

    occurrences: list[dict[str, Any]] = []
    full_keys: set[str] = set()
    for branch in branches:
        pattern_id = branch["patternId"]
        action = pattern_source_actions[
            (pattern_id, branch["sourceActionId"], branch["profileId"])
        ]
        stage_by_index = {
            int(row["stageIndex"]): row for row in action.get("stages", [])
        }
        for source_stage_index in branch["sourceStagePath"]:
            stage = stage_by_index[source_stage_index]
            product_matches = source_stage_product_matches(
                stage,
                product_clips.get(pattern_id, []),
                branch["selectionStatus"],
                reviewed_mappings.get(branch["branchId"], {}),
            )
            for cue_ordinal, cue in enumerate(stage.get("effectCues", [])):
                category = str(cue.get("category") or "")
                if category not in VISUAL_CATEGORIES:
                    raise InventoryError(
                        f"unknown visual category: {pattern_id}/{category}"
                    )
                for product_match in product_matches:
                    for asset_ordinal, asset_reference in enumerate(
                        asset_reference_rows(cue)
                    ):
                        object_path = (
                            str(asset_reference.get("objectPath") or "")
                            if asset_reference is not None
                            else ""
                        )
                        class_name = (
                            str(asset_reference.get("className") or "")
                            if asset_reference is not None
                            else ""
                        )
                        source_system_id = (
                            object_path.casefold()
                            if class_name.casefold() == "particlesystem"
                            else None
                        )
                        source_system = source_systems.get(
                            source_system_id or ""
                        )
                        occurrence_key = (
                            f"pattern={pattern_id}"
                            f"|sourceAction={branch['sourceActionId']}"
                            f"|profile={branch['profileId']}"
                            f"|branch={branch['branchId']}"
                            f"|sourceStage={source_stage_index}"
                            f"|sourceClipOrdinal={product_match['sourceClipOrdinal']}"
                            f"|sourceClip={product_match['sourceClip']}"
                            f"|notify={cue.get('notifyId')}"
                            f"|time={float(cue.get('localTimeSeconds') or 0.0):.9f}"
                            f"|duration={float(cue.get('durationSeconds') or 0.0):.9f}"
                            f"|sourceType={cue.get('sourceType')}"
                            f"|category={category}|assetOrdinal={asset_ordinal}"
                            f"|assetClass={class_name}|asset={object_path}"
                        )
                        if occurrence_key in full_keys:
                            raise InventoryError(
                                f"duplicate source occurrence full key: {occurrence_key}"
                            )
                        full_keys.add(occurrence_key)
                        full_key_sha = sha256_bytes(
                            occurrence_key.encode("utf-8")
                        )
                        carrier_keys = (
                            [row["carrierKey"] for row in source_system["carriers"]]
                            if source_system is not None
                            else []
                        )
                        expanded = [
                            f"occurrence-key.{full_key_sha}|{carrier_key}"
                            for carrier_key in carrier_keys
                        ]
                        occurrences.append(
                            {
                                "occurrenceId": (
                                    "occurrence."
                                    + full_key_sha[:24]
                                ),
                                "fullKey": "occurrence-key." + full_key_sha,
                                "patternId": pattern_id,
                                "semanticStageId": product_match[
                                    "semanticStageId"
                                ],
                                "gameplayActionId": product_match[
                                    "gameplayActionId"
                                ],
                                "clipOccurrenceId": product_match[
                                    "clipOccurrenceId"
                                ],
                                "candidateClipOccurrenceIds": product_match[
                                    "candidateClipOccurrenceIds"
                                ],
                                "timingDisposition": product_match[
                                    "timingDisposition"
                                ],
                                "mappingReviewBasis": product_match[
                                    "mappingReviewBasis"
                                ],
                                "sourceActionId": branch["sourceActionId"],
                                "profileId": branch["profileId"],
                                "branchId": branch["branchId"],
                                "branchSelectionStatus": branch[
                                    "selectionStatus"
                                ],
                                "sourceStageIndex": source_stage_index,
                                "sourceStagePath": (
                                    f"action-{branch['sourceActionId']}/"
                                    f"{branch['profileId']}/"
                                    f"sequence-{branch['sequenceIndex']:03d}/"
                                    f"stage-{source_stage_index:03d}"
                                ),
                                "sourceClipOrdinal": product_match[
                                    "sourceClipOrdinal"
                                ],
                                "sourceClip": product_match["sourceClip"],
                                "notifyOrdinal": cue_ordinal,
                                "notifyId": str(cue.get("notifyId") or ""),
                                "sourceTimeSeconds": finite_number(
                                    cue.get("localTimeSeconds", 0.0),
                                    "source notify time",
                                ),
                                "sourceDurationSeconds": finite_number(
                                    cue.get("durationSeconds", 0.0),
                                    "source notify duration",
                                ),
                                "sourceType": str(cue.get("sourceType") or ""),
                                "category": category,
                                "sourceResolutionStatus": str(
                                    cue.get("resolutionStatus") or ""
                                ),
                                "sourceAssetOrdinal": asset_ordinal,
                                "assetReference": asset_reference,
                                "sourceSystemId": source_system_id,
                                "disposition": occurrence_disposition(
                                    str(cue.get("sourceType") or ""),
                                    category,
                                    asset_reference,
                                    source_system,
                                ),
                                "reachabilityDisposition": (
                                    "UNREACHABLE_SOURCE_OCCURRENCE"
                                    if product_match["timingDisposition"]
                                    == "UNREACHABLE"
                                    else "REACHABLE_REVIEWED"
                                    if branch["selectionStatus"]
                                    == "REVIEWED_SELECTED"
                                    and product_match[
                                        "hasExplicitReviewedMapping"
                                    ]
                                    and product_match["timingDisposition"]
                                    == "REACHABLE"
                                    else "SOURCE_TIMING_REVIEW_REQUIRED"
                                    if branch["selectionStatus"]
                                    == "REVIEWED_SELECTED"
                                    and product_match[
                                        "hasExplicitReviewedMapping"
                                    ]
                                    else "UNRESOLVED_BRANCH_SELECTION"
                                    if branch["selectionStatus"]
                                    == "UNRESOLVED_BRANCH_SELECTION"
                                    else "UNRESOLVED_CLIP_OCCURRENCE_MAPPING"
                                ),
                                "expandedCarrierCount": len(expanded),
                                "expandedCarrierFullKeysSha256": canonical_sha256(
                                    expanded
                                ),
                            }
                        )
    occurrences.sort(key=lambda row: row["fullKey"])
    return occurrences


def apply_reviewed_selections(
    branches: list[dict[str, Any]],
    product_clips_by_pattern: dict[str, list[dict[str, Any]]],
    selection_index: dict[str, dict[str, Any]],
    mapping_index: dict[str, dict[tuple[int, int], dict[str, Any]]],
) -> None:
    known = {row["branchId"] for row in branches}
    unknown = sorted(set(selection_index) - known)
    if unknown:
        raise InventoryError(
            "reviewed branch selection references missing branch: " + unknown[0]
        )
    for branch in branches:
        selection = selection_index.get(branch["branchId"])
        if selection is None:
            branch["selectionStatus"] = "UNRESOLVED_BRANCH_SELECTION"
            branch["reviewedStageMappings"] = []
            continue
        for field in (
            "patternId",
            "sourceActionId",
            "profileId",
            "sequenceIndex",
            "sourceSequencePathSha256",
        ):
            if selection.get(field) != branch[field]:
                raise InventoryError(
                    f"reviewed branch selection {field} does not match branch: "
                    f"{branch['branchId']}"
                )
        branch["selectionStatus"] = selection["status"]
        mappings = mapping_index.get(branch["branchId"], {})
        if selection["status"] == "REVIEWED_REJECTED" and mappings:
            raise InventoryError(
                "rejected branch must not contain source stage mappings: "
                + branch["branchId"]
            )
        source_coordinates = {
            (row["sourceStageIndex"], row["sourceClipOrdinal"]): row
            for row in branch["orderedClips"]
        }
        product_rows = product_clips_by_pattern.get(branch["patternId"], [])
        products_by_id = {
            row["clipOccurrenceId"]: row for row in product_rows
        }
        for coordinate, mapping in mappings.items():
            source_clip = source_coordinates.get(coordinate)
            if source_clip is None:
                raise InventoryError(
                    "reviewed source stage mapping is outside selected branch: "
                    f"{branch['branchId']}/{coordinate}"
                )
            clip_occurrence_id = mapping.get("clipOccurrenceId")
            if clip_occurrence_id is None:
                continue
            product_clip = products_by_id.get(clip_occurrence_id)
            if product_clip is None:
                raise InventoryError(
                    "reviewed source stage mapping references a foreign product "
                    f"clip occurrence: {clip_occurrence_id}"
                )
            if source_clip["normalizedClip"] != product_clip["normalizedClip"]:
                raise InventoryError(
                    "reviewed source/product clip names do not match: "
                    f"{branch['branchId']}/{coordinate}/{clip_occurrence_id}"
                )
        branch["reviewedStageMappings"] = [
            copy.deepcopy(mappings[key]) for key in sorted(mappings)
        ]


def pattern_coverage(
    encounter: dict[str, Any], action_bindings: dict[str, Any]
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    action_by_pattern = {
        str(row.get("patternId") or ""): row
        for row in action_bindings.get("patterns", [])
    }
    if len(action_by_pattern) != len(action_bindings.get("patterns", [])):
        raise InventoryError("actionbindings has duplicate patternId")
    rows = []
    missing = []
    for pattern in encounter.get("patterns", []):
        pattern_id = str(pattern.get("patternId") or "")
        action = action_by_pattern.get(pattern_id)
        if action is None:
            status = MISSING_ACTION_BINDING_POLICIES.get(
                pattern_id, "MISSING_ACTION_BINDING_UNREVIEWED"
            )
            missing_row = {
                "patternId": pattern_id,
                "status": status,
                "sourceActionIds": [
                    int(value) for value in pattern.get("sourceActionIds", [])
                ],
            }
            missing.append(missing_row)
            rows.append({**missing_row, "sourceActionBindingCount": 0})
        else:
            rows.append(
                {
                    "patternId": pattern_id,
                    "status": "ACTION_BINDING_PRESENT",
                    "sourceActionIds": [
                        int(value) for value in action.get("sourceActionIds", [])
                    ],
                    "sourceActionBindingCount": len(
                        action.get("sourceActions", [])
                    ),
                }
            )
    return rows, missing


def source_system_ids_from_patterns(patterns: list[dict[str, Any]]) -> set[str]:
    result = set()
    for pattern in patterns:
        for action in pattern.get("sourceActions", []):
            for stage in action.get("stages", []):
                for cue in stage.get("effectCues", []):
                    for reference in cue.get("assetReferences", []):
                        if str(reference.get("className") or "").casefold() == (
                            "particlesystem"
                        ):
                            result.add(str(reference.get("objectPath") or "").casefold())
    return result


def repository_source_rows(paths: Iterable[Path]) -> list[dict[str, Any]]:
    return [
        {
            "path": path.relative_to(ROOT).as_posix(),
            "sha256": sha256_file(path),
        }
        for path in paths
    ]


def build_dash_source_reviewed_delta_proposal(
    action_bindings: dict[str, Any],
    catalog: dict[str, Any],
    source_sequences: dict[int, list[dict[str, Any]]],
) -> dict[str, Any]:
    """Intake 400424 as evidence without replacing authored 420604."""
    descriptors = [
        row
        for row in catalog.get("sourceActionDocuments", [])
        if str(row.get("profileId") or "") == "MN_RPBF_00"
    ]
    if len(descriptors) != 1:
        raise InventoryError(
            "Dash evidence requires one MN_RPBF_00 source action document"
        )
    descriptor = descriptors[0]
    source_path = Path(str(descriptor.get("path") or ""))
    if not source_path.is_absolute():
        source_path = (ROOT / source_path).resolve()
    if not source_path.is_file():
        raise InventoryError(f"Dash source action evidence is missing: {source_path}")
    actual_sha = sha256_file(source_path)
    if actual_sha != str(descriptor.get("sha256") or ""):
        raise InventoryError("Dash source action evidence drifted from catalog")
    source_document = read_json(source_path)
    if str(source_document.get("profileId") or "") != "MN_RPBF_00":
        raise InventoryError("Dash source action evidence profile is invalid")
    actions = [
        row
        for row in source_document.get("actions", [])
        if int(row.get("actionId", -1)) == 400424
    ]
    if len(actions) != 1:
        raise InventoryError("Dash source action 400424 is not unique")
    action = actions[0]
    sequence_rows = [
        row
        for row in source_sequences.get(400424, [])
        if row["sequenceIndex"] == 0
    ]
    if len(sequence_rows) != 1:
        raise InventoryError("Dash source action 400424 sequence 0 is missing")
    sequence = sequence_rows[0]

    proposed_stages = []
    flattened_clips = []
    for stage in action.get("stages", []):
        stage_index = int(stage.get("stageIndex", -1))
        clips = [
            str(row.get("clipName") or row.get("clip") or "")
            for row in stage.get("animationClips", [])
            if isinstance(row, dict)
        ]
        clips = [row for row in clips if row]
        flattened_clips.extend(clips)
        cues = [
            {
                "notifyId": str(row.get("notifyId") or ""),
                **visual_cue_signature_payload(row),
            }
            for row in stage.get("notifies", [])
            if str(row.get("category") or "") in VISUAL_CATEGORIES
        ]
        if clips or cues:
            proposed_stages.append(
                {
                    "stageIndex": stage_index,
                    "stageName": str(stage.get("stageName") or ""),
                    "animationClips": clips,
                    "effectCues": cues,
                    "sourceVisualCuePayloadSha256": canonical_sha256(
                        [
                            {
                                key: value
                                for key, value in cue.items()
                                if key != "notifyId"
                            }
                            for cue in cues
                        ]
                    ),
                }
            )
    if [normalize_clip(row) for row in flattened_clips] != [
        normalize_clip(row) for row in sequence["clips"]
    ]:
        raise InventoryError(
            "Dash 400424 clipseq and source action evidence disagree"
        )

    preview = read_json(PATTERN_PREVIEW_PATH)
    preview_rows = []
    for pattern in preview.get("patterns", []):
        for source_sequence in pattern.get("sequences", []):
            if (
                int(source_sequence.get("sourceActionId", -1)) == 400424
                and int(source_sequence.get("sequenceIndex", -1)) == 0
            ):
                preview_rows.append(
                    {
                        "patternNumber": int(pattern["number"]),
                        "confidence": str(pattern.get("confidence") or ""),
                        "repeat": int(source_sequence.get("repeat", 1)),
                    }
                )
    if not preview_rows or {
        row["confidence"] for row in preview_rows
    } != {"USER_CONFIRMED_FAMILY"}:
        raise InventoryError(
            "Dash 400424 is not user-confirmed by Valtan patternpreview"
        )

    dash_bindings = [
        row
        for row in action_bindings.get("patterns", [])
        if row.get("patternId") == "VALTAN_DASH_CHARGE"
    ]
    if len(dash_bindings) != 1:
        raise InventoryError("Dash action binding evidence is missing")
    current_source_ids = [
        int(row) for row in dash_bindings[0].get("sourceActionIds", [])
    ]
    if current_source_ids != [420604]:
        raise InventoryError("Dash current source action conflict changed")

    source_sequence_path = [
        {
            "sourceStageIndex": int(stage["stageIndex"]),
            "sourceClipOrdinal": clip_ordinal,
            "clip": clip,
        }
        for stage in proposed_stages
        for clip_ordinal, clip in enumerate(stage["animationClips"])
    ]
    return {
        "patternId": "VALTAN_DASH_CHARGE",
        "status": "PROPOSED_SOURCE_REVIEWED_DELTA_NOT_ACCEPTED",
        "reviewBasis": "PATTERNPREVIEW_USER_CONFIRMED_FAMILY_SEQUENCE_0",
        "currentSourceActionIds": current_source_ids,
        "proposedSourceActionId": 400424,
        "profileId": "MN_RPBF_00",
        "sequenceIndex": 0,
        "sourceSequencePathSha256": canonical_sha256(source_sequence_path),
        "mappingBasis": "SOURCE_REVIEWED_DELTA",
        "conflictDisposition": (
            "EVIDENCE_ONLY_DO_NOT_AUTO_REPLACE_420604_OR_SPLIT_PRODUCT_TIMING"
        ),
        "patternPreviewReferences": sorted(
            preview_rows, key=lambda row: row["patternNumber"]
        ),
        "sourceActionDocument": {
            "profileId": "MN_RPBF_00",
            "catalogPath": source_path.as_posix(),
            "sha256": actual_sha,
        },
        "proposedSourceAction": {
            "sourceActionId": 400424,
            "profileId": "MN_RPBF_00",
            "sourceDisplayName": str(action.get("displayName") or ""),
            "stages": proposed_stages,
        },
    }


def build_inventory(
    previous: dict[str, Any] | None = None,
    *,
    encounter: dict[str, Any] | None = None,
    pattern_bindings: dict[str, Any] | None = None,
    action_bindings: dict[str, Any] | None = None,
    catalog: dict[str, Any] | None = None,
    loaded_graph_index: dict[str, Any] | None = None,
    graph_source_descriptors: list[dict[str, Any]] | None = None,
    runtime_resource_bindings: list[dict[str, Any]] | None = None,
    runtime_cook_descriptor: dict[str, Any] | None = None,
    additional_repository_sources: list[Path] | None = None,
    include_payloads: bool = False,
) -> dict[str, Any]:
    encounter = encounter or read_json(ENCOUNTER_PATH)
    pattern_bindings = pattern_bindings or read_json(PATTERN_BINDINGS_PATH)
    action_bindings = action_bindings or read_json(ACTION_BINDINGS_PATH)
    catalog = catalog or read_json(SOURCE_CATALOG_PATH)

    patterns = action_bindings.get("patterns", [])
    coverage, missing = pattern_coverage(encounter, action_bindings)
    if len(encounter.get("patterns", [])) != EXPECTED_PATTERN_COUNT:
        raise InventoryError(
            f"Valtan encounter pattern count is not {EXPECTED_PATTERN_COUNT}"
        )
    if {row["patternId"] for row in missing} != set(
        MISSING_ACTION_BINDING_POLICIES
    ):
        raise InventoryError(
            "first inventory must explicitly retain Entrance/Arena84 gaps"
        )

    product_clips = product_clip_occurrences(encounter, pattern_bindings)
    source_sequences = load_source_clip_sequences()
    branches = []
    for pattern in patterns:
        pattern_id = str(pattern.get("patternId") or "")
        pattern_branches = []
        for source_action in pattern.get("sourceActions", []):
            source_action_id = int(source_action["sourceActionId"])
            sequences = source_sequences.get(source_action_id)
            if not sequences:
                raise InventoryError(
                    f"source action has no Valtan clipseq rows: {source_action_id}"
                )
            pattern_branches.extend(
                split_source_sequences(pattern_id, source_action, sequences)
            )
        choose_branch_recommendations(
            pattern_branches, product_clips.get(pattern_id, [])
        )
        branches.extend(pattern_branches)
    source_visual_signature_reviews = build_source_visual_signature_reviews(
        branches
    )
    source_action_evidence_proposals = [
        build_dash_source_reviewed_delta_proposal(
            action_bindings,
            catalog,
            source_sequences,
        )
    ]

    (
        reviewed_selections,
        selection_index,
        reviewed_mappings,
    ) = reviewed_selection_index(previous)
    apply_reviewed_selections(
        branches,
        product_clips,
        selection_index,
        reviewed_mappings,
    )
    additional_source_selection_audit = (
        build_additional_source_selection_audit(
            branches,
            source_visual_signature_reviews,
            reviewed_selections,
            reviewed_mappings,
        )
    )

    if loaded_graph_index is None:
        specs = graph_specs(catalog)
        loaded_graph_index = source_receipts.load_graphs(specs)
        graph_source_descriptors = graph_source_rows(specs)
        (
            runtime_resource_bindings,
            runtime_cook_descriptor,
        ) = runtime_cook_receipt(specs)
    elif graph_source_descriptors is None:
        graph_source_descriptors = []
    if runtime_resource_bindings is None:
        runtime_resource_bindings = []
    if runtime_cook_descriptor is None:
        runtime_cook_descriptor = {
            "logicalPath": "IN_MEMORY",
            "sha256": canonical_sha256(runtime_resource_bindings),
            "assetCount": len(runtime_resource_bindings),
            "verifiedRuntimeFileCount": 0,
            "failureCount": 0,
        }

    catalog_systems = {
        str(row.get("sourceAsset") or "").casefold(): row
        for row in catalog.get("sourceSystems", [])
    }
    if len(catalog_systems) != len(catalog.get("sourceSystems", [])):
        raise InventoryError("source catalog has duplicate sourceAsset")
    requested_system_ids = source_system_ids_from_patterns(patterns)
    missing_systems = sorted(requested_system_ids - set(catalog_systems))
    if missing_systems:
        raise InventoryError(
            "action-bound source system is missing from catalog: "
            + missing_systems[0]
        )
    source_systems = {
        system_id: build_system_inventory(
            catalog,
            loaded_graph_index,
            catalog_systems[system_id],
            runtime_resource_bindings,
            include_payloads=include_payloads,
        )
        for system_id in sorted(requested_system_ids)
    }

    occurrences = build_occurrences(
        patterns,
        branches,
        product_clips,
        source_systems,
        reviewed_mappings,
    )
    encounter_action_ids = {
        str(stage.get("actionId") or "")
        for pattern in encounter.get("patterns", [])
        for stage in pattern.get("stages", [])
    }
    bound_action_ids = {
        str(row.get("actionId") or "")
        for row in pattern_bindings.get("bindings", [])
    }
    binding_gaps = sorted(encounter_action_ids - bound_action_ids)
    proposed_gap_ids = sorted(
        row["actionId"] for row in ARENA84_BINDING_GAP_PROPOSALS
    )
    if binding_gaps != proposed_gap_ids:
        raise InventoryError(
            "canonical pattern binding gaps changed: " + repr(binding_gaps)
        )
    all_expanded = [
        f"{occurrence['fullKey']}|{carrier['carrierKey']}"
        for occurrence in occurrences
        for carrier in source_systems.get(
            occurrence.get("sourceSystemId") or "", {}
        ).get("carriers", [])
    ]
    if len(all_expanded) != len(set(all_expanded)):
        raise InventoryError("duplicate expanded occurrence/carrier full key")
    carrier_count = sum(
        len(system["carriers"]) for system in source_systems.values()
    )
    selected_occurrences = [
        row
        for row in occurrences
        if row["reachabilityDisposition"] == "REACHABLE_REVIEWED"
    ]
    selected_core_keys = [
        f"{occurrence['fullKey']}|{carrier['carrierKey']}"
        for occurrence in selected_occurrences
        for carrier in source_systems.get(
            occurrence.get("sourceSystemId") or "", {}
        ).get("carriers", [])
        if carrier["disposition"] == "EXECUTABLE_CORE"
    ]

    # Branch-local stage payload is already expanded into occurrences.  Avoid
    # duplicating that large input tree in the immutable receipt.
    branch_rows = []
    for branch in branches:
        staged = {
            key: copy.deepcopy(value)
            for key, value in branch.items()
            if key != "stages"
        }
        staged["candidateOccurrenceCount"] = sum(
            1 for row in occurrences if row["branchId"] == branch["branchId"]
        )
        branch_rows.append(staged)
    branch_rows.sort(key=lambda row: row["branchId"])

    category_counts = Counter(row["category"] for row in occurrences)
    disposition_counts = Counter(row["disposition"] for row in occurrences)
    reachability_counts = Counter(
        row["reachabilityDisposition"] for row in occurrences
    )
    carrier_dispositions = Counter(
        carrier["disposition"]
        for system in source_systems.values()
        for carrier in system["carriers"]
    )
    ribbon_system_ids = {
        system["sourceSystemId"]
        for system in source_systems.values()
        if any(
            carrier["runtimeAdapterType"] == "RIBBON"
            for carrier in system["carriers"]
        )
    }
    document = {
        "schema": "lostark.valtan-source-occurrence-inventory",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "completionPolicy": (
            "REVIEWED_SELECTED_SOURCE_SEQUENCE_EXACT_STAGE_CLIP_MAPPING_"
            "REACHABLE_CORE_CARRIERS_ONLY; "
            "sequence candidate x emitter upper bound is evidence, not completion"
        ),
        "sources": {
            "repository": repository_source_rows(
                [
                    ENCOUNTER_PATH,
                    PATTERN_BINDINGS_PATH,
                    ACTION_BINDINGS_PATH,
                    SOURCE_CATALOG_PATH,
                    CLIPSEQ_PATH,
                    PATTERN_PREVIEW_PATH,
                    *(additional_repository_sources or []),
                ]
            ),
            "sourceGraphs": graph_source_descriptors,
            "runtimeCookReceipt": runtime_cook_descriptor,
        },
        "coverage": {
            "patterns": coverage,
            "missingActionBindingPatterns": missing,
        },
        "bindingGapProposals": copy.deepcopy(
            ARENA84_BINDING_GAP_PROPOSALS
        ),
        "sourceActionEvidenceProposals": source_action_evidence_proposals,
        "sourceVisualSignatureReviews": source_visual_signature_reviews,
        "additionalSourceSelectionAudit": (
            additional_source_selection_audit
        ),
        "reviewedBranchSelections": reviewed_selections,
        "branches": branch_rows,
        "sourceSystems": [source_systems[key] for key in sorted(source_systems)],
        "occurrences": occurrences,
        "summary": {
            "encounterPatternCount": len(encounter.get("patterns", [])),
            "actionBindingPatternCount": len(patterns),
            "missingActionBindingPatternCount": len(missing),
            "branchCandidateCount": len(branches),
            "sourceSequenceCandidateCount": len(branches),
            "reviewedSelectedBranchCount": sum(
                row["selectionStatus"] == "REVIEWED_SELECTED"
                for row in branches
            ),
            "reviewedSelectedSequenceCount": sum(
                row["selectionStatus"] == "REVIEWED_SELECTED"
                for row in branches
            ),
            "sourceOccurrenceCandidateCount": len(occurrences),
            "sourceOccurrenceCategoryCounts": dict(sorted(category_counts.items())),
            "sourceOccurrenceDispositionCounts": dict(
                sorted(disposition_counts.items())
            ),
            "sourceOccurrenceReachabilityCounts": dict(
                sorted(reachability_counts.items())
            ),
            "sourceSystemCount": len(source_systems),
            "sourceCarrierCount": carrier_count,
            "encounterStageActionCount": len(encounter_action_ids),
            "patternBindingActionCount": len(bound_action_ids),
            "patternBindingGapCount": len(binding_gaps),
            "patternBindingGapProposalCount": len(
                ARENA84_BINDING_GAP_PROPOSALS
            ),
            "sourceActionEvidenceProposalCount": len(
                source_action_evidence_proposals
            ),
            "sourceVisualSignatureEquivalentReviewCount": sum(
                row["status"] == "SOURCE_VISUAL_SIGNATURE_EQUIVALENT"
                for row in source_visual_signature_reviews
            ),
            "sourceVisualSignatureAmbiguousReviewCount": sum(
                row["status"] == "AMBIGUOUS_SOURCE_VISUAL_SIGNATURE"
                for row in source_visual_signature_reviews
            ),
            "sourceCarrierDispositionCounts": dict(
                sorted(carrier_dispositions.items())
            ),
            "sourcePrimitiveDecodedCarrierCount": sum(
                carrier.get("sourceRecipeSha256") is not None
                for system in source_systems.values()
                for carrier in system["carriers"]
            ),
            "portableModuleReadyCarrierCount": sum(
                carrier.get("portableSourceRecipeSha256") is not None
                for system in source_systems.values()
                for carrier in system["carriers"]
            ),
            "drawableRuntimeReadyCarrierCount": carrier_dispositions[
                "EXECUTABLE_CORE"
            ],
            "missingRuntimeResourceCarrierCount": carrier_dispositions[
                "MISSING_RUNTIME_RESOURCE"
            ],
            "runtimeResourceBoundCarrierCount": sum(
                carrier.get("runtimeResourceBindingCount", 0) > 0
                for system in source_systems.values()
                for carrier in system["carriers"]
            ),
            "runtimeResourceBindingCount": sum(
                carrier.get("runtimeResourceBindingCount", 0)
                for system in source_systems.values()
                for carrier in system["carriers"]
            ),
            "animationTrailOccurrenceCount": sum(
                str((row.get("assetReference") or {}).get("className") or "")
                .casefold()
                == "efdata_animnotify_trails"
                for row in occurrences
            ),
            "trailGhostOccurrenceCount": sum(
                row["sourceType"].casefold() == "trailghosteffect"
                for row in occurrences
            ),
            "ribbonCarrierCount": sum(
                carrier["runtimeAdapterType"] == "RIBBON"
                for system in source_systems.values()
                for carrier in system["carriers"]
            ),
            "ribbonSourceSystemCount": len(ribbon_system_ids),
            "ribbonReferencedOccurrenceCount": sum(
                row.get("sourceSystemId") in ribbon_system_ids
                for row in occurrences
            ),
            "ribbonBlockedOccurrenceCount": sum(
                row.get("sourceSystemId") in ribbon_system_ids
                and row["disposition"] == "UNRESOLVED_RUNTIME_ADAPTER"
                for row in occurrences
            ),
            "unresolvedRuntimeAdapterCarrierCount": carrier_dispositions[
                "UNRESOLVED_RUNTIME_ADAPTER"
            ],
            "portableRuntimeAdapterBlockedCarrierCount": sum(
                carrier["disposition"] == "UNRESOLVED_RUNTIME_ADAPTER"
                and carrier.get("runtimeAdapterType") != "RIBBON"
                for system in source_systems.values()
                for carrier in system["carriers"]
            ),
            "unresolvedRuntimeAdapterOccurrenceCount": disposition_counts[
                "UNRESOLVED_RUNTIME_ADAPTER"
            ],
            "branchCarrierUpperBound": len(all_expanded),
            "completionCarrierDenominator": len(selected_core_keys),
            "unresolvedDecalOrEffectCount": sum(
                row["category"] in {"decal", "unresolved_effect"}
                and row["disposition"] == "UNRESOLVED_SOURCE_PAYLOAD"
                for row in occurrences
            ),
            "droppedOccurrenceCount": 0,
            "duplicateOccurrenceCount": 0,
            "droppedCarrierCount": 0,
            "duplicateCarrierCount": 0,
        },
    }
    validate_inventory(document)
    return document


def validate_inventory(document: dict[str, Any]) -> None:
    if document.get("schema") != "lostark.valtan-source-occurrence-inventory":
        raise InventoryError("inventory schema is invalid")
    if document.get("formatVersion") != 1:
        raise InventoryError("inventory formatVersion is invalid")
    summary = document.get("summary") or {}
    if summary.get("encounterPatternCount") != EXPECTED_PATTERN_COUNT:
        raise InventoryError("inventory does not cover 33 patterns")
    if summary.get("actionBindingPatternCount") != 31:
        raise InventoryError("first inventory must expose 31/33 actionbindings")
    if summary.get("missingActionBindingPatternCount") != 2:
        raise InventoryError("first inventory must expose two actionbinding gaps")
    if summary.get("sourceActionEvidenceProposalCount") != 1:
        raise InventoryError("Dash 400424 evidence-only proposal is missing")
    if summary.get("sourceVisualSignatureEquivalentReviewCount") != 4:
        raise InventoryError("source visual equivalence reviews changed")
    if summary.get("sourceVisualSignatureAmbiguousReviewCount") != 1:
        raise InventoryError("source visual ambiguity reviews changed")
    if (
        summary.get("sourceSequenceCandidateCount")
        != summary.get("branchCandidateCount")
        or summary.get("reviewedSelectedSequenceCount")
        != summary.get("reviewedSelectedBranchCount")
    ):
        raise InventoryError("source sequence compatibility summary changed")
    for field in (
        "droppedOccurrenceCount",
        "duplicateOccurrenceCount",
        "droppedCarrierCount",
        "duplicateCarrierCount",
    ):
        if summary.get(field) != 0:
            raise InventoryError(f"inventory {field} is not zero")
    if (
        not document.get("reviewedBranchSelections")
        and summary.get("completionCarrierDenominator") != 0
    ):
        raise InventoryError(
            "unreviewed branch upper bound leaked into completion denominator"
        )
    missing = {
        row["patternId"]: row["status"]
        for row in document["coverage"]["missingActionBindingPatterns"]
    }
    if missing != MISSING_ACTION_BINDING_POLICIES:
        raise InventoryError("Entrance/Arena84 missing policies changed")
    dash_proposals = document.get("sourceActionEvidenceProposals", [])
    if (
        len(dash_proposals) != 1
        or dash_proposals[0].get("patternId") != "VALTAN_DASH_CHARGE"
        or dash_proposals[0].get("proposedSourceActionId") != 400424
        or dash_proposals[0].get("currentSourceActionIds") != [420604]
        or dash_proposals[0].get("status")
        != "PROPOSED_SOURCE_REVIEWED_DELTA_NOT_ACCEPTED"
    ):
        raise InventoryError("Dash source reviewed delta proposal changed")
    branch_by_id = {row["branchId"]: row for row in document["branches"]}
    for branch in document["branches"]:
        if not re.fullmatch(
            r"[0-9a-f]{64}",
            str(branch.get("sourceVisualFamilySignatureSha256") or ""),
        ):
            raise InventoryError("source visual family signature is invalid")
        for digest in branch.get("sourceVisualFamilyMemberSha256s", []):
            if not re.fullmatch(r"[0-9a-f]{64}", str(digest)):
                raise InventoryError(
                    "source visual family member signature is invalid"
                )
    for review in document.get("sourceVisualSignatureReviews", []):
        candidates = review.get("equivalentCandidates", [])
        if not candidates:
            raise InventoryError("source visual signature review has no candidates")
        for candidate in candidates:
            branch = branch_by_id.get(candidate.get("branchId"))
            if branch is None:
                raise InventoryError(
                    "source visual signature review references missing branch"
                )
            if candidate != source_visual_signature_candidate(branch):
                raise InventoryError(
                    "source visual signature review candidate drifted"
                )
        if review["status"] == "SOURCE_VISUAL_SIGNATURE_EQUIVALENT":
            if (
                review.get("canonicalCandidate") != candidates[0]
                or len(
                    {
                        row["sourceVisualFamilySignatureSha256"]
                        for row in candidates
                    }
                )
                != 1
            ):
                raise InventoryError(
                    "source visual equivalence canonical candidate changed"
                )
        elif review["status"] == "AMBIGUOUS_SOURCE_VISUAL_SIGNATURE":
            if review.get("canonicalCandidate") is not None:
                raise InventoryError(
                    "ambiguous source visual review selected a candidate"
                )
        else:
            raise InventoryError("source visual signature review status is invalid")
    additional_audit = document.get("additionalSourceSelectionAudit", [])
    if [row.get("patternId") for row in additional_audit] != (
        ADDITIONAL_SOURCE_SELECTION_REVIEW_ORDER
    ):
        raise InventoryError(
            "additional source selection audit does not preserve the twelve "
            "reviewed patterns"
        )
    for row in additional_audit:
        selected = row.get("selectionStatus") == "REVIEWED_SELECTED"
        if selected != bool(row.get("selectedBranchId")):
            raise InventoryError(
                "additional source selection audit selected identity is invalid"
            )
        if selected != (row.get("unresolvedReason") is None):
            raise InventoryError(
                "additional source selection audit unresolved reason is invalid"
            )
        if selected and row.get("eligibilityDisposition") != (
            "SAFE_EXACT_EQUIVALENT_FULL_SEQUENCE_JOIN"
        ):
            raise InventoryError(
                "additional source selection audit admitted an unsafe branch"
            )
    occurrence_keys = [row["fullKey"] for row in document["occurrences"]]
    if len(occurrence_keys) != len(set(occurrence_keys)):
        raise InventoryError("inventory has duplicate occurrence full keys")
    carrier_keys = [
        row["carrierKey"]
        for system in document["sourceSystems"]
        for row in system["carriers"]
    ]
    if len(carrier_keys) != len(set(carrier_keys)):
        raise InventoryError("inventory has duplicate carrier keys")
    systems = {
        row["sourceSystemId"]: row for row in document["sourceSystems"]
    }
    expanded = []
    for occurrence in document["occurrences"]:
        reachability = occurrence["reachabilityDisposition"]
        if reachability == "REACHABLE_REVIEWED":
            if (
                occurrence["branchSelectionStatus"] != "REVIEWED_SELECTED"
                or occurrence["timingDisposition"] != "REACHABLE"
                or not occurrence.get("clipOccurrenceId")
                or occurrence.get("clipOccurrenceId")
                not in occurrence.get("candidateClipOccurrenceIds", [])
            ):
                raise InventoryError(
                    "reachable occurrence has no exact reviewed source/product "
                    "clip mapping"
                )
        elif (
            occurrence["branchSelectionStatus"] == "REVIEWED_SELECTED"
            and not occurrence.get("clipOccurrenceId")
            and reachability == "REACHABLE_REVIEWED"
        ):
            raise InventoryError(
                "unmapped selected source occurrence leaked into completion"
            )
        carriers = systems.get(
            occurrence.get("sourceSystemId") or "", {}
        ).get("carriers", [])
        occurrence_expanded = [
            f"{occurrence['fullKey']}|{carrier['carrierKey']}"
            for carrier in carriers
        ]
        if occurrence["expandedCarrierCount"] != len(occurrence_expanded):
            raise InventoryError("inventory expanded carrier count changed")
        if occurrence["expandedCarrierFullKeysSha256"] != canonical_sha256(
            occurrence_expanded
        ):
            raise InventoryError("inventory expanded carrier closure changed")
        expanded.extend(occurrence_expanded)
    if len(expanded) != len(set(expanded)):
        raise InventoryError("inventory has duplicate expanded full keys")
    completion_count = sum(
        1
        for occurrence in document["occurrences"]
        if occurrence["reachabilityDisposition"] == "REACHABLE_REVIEWED"
        for carrier in systems.get(
            occurrence.get("sourceSystemId") or "", {}
        ).get("carriers", [])
        if carrier["disposition"] == "EXECUTABLE_CORE"
    )
    if summary.get("completionCarrierDenominator") != completion_count:
        raise InventoryError(
            "completion denominator is not the exact reviewed reachable core "
            "carrier closure"
        )


def cue_effect_index(cue_document: dict[str, Any]) -> dict[tuple[str, str], str]:
    result: dict[tuple[str, str], str] = {}
    for cue in cue_document.get("cues", []):
        key = (
            str(cue.get("patternId") or ""),
            str(cue.get("actionId") or ""),
        )
        effect_id = str(cue.get("effectAssetId") or "")
        if not all(key) or not effect_id:
            continue
        if key in result and result[key] != effect_id:
            raise InventoryError(f"two Effect assets own cue action: {key}")
        result[key] = effect_id
    return result


def is_legacy_generic_default(element: dict[str, Any]) -> bool:
    detail = element.get("detail") or {}
    particle = detail.get("particle") or {}
    transform = detail.get("transform") or {}
    recipe = element.get("sourceRecipe") or {}
    return bool(
        LEGACY_ELEMENT_RE.search(str(element.get("id") or ""))
        and recipe.get("enabled") is False
        and transform.get("scale") == [1.0, 1.0, 1.0]
        and particle.get("maxParticles") == 8
        and particle.get("burstCount") == 8
        and particle.get("lifeTimeSeconds") == [0.6, 1.0]
    )


def occurrence_element_seed(
    occurrence: dict[str, Any], carrier: dict[str, Any]
) -> dict[str, Any]:
    seed = copy.deepcopy(carrier["elementSeed"])
    full_key = (
        f"{occurrence['fullKey']}|{carrier['carrierKey']}"
    )
    seed["id"] = "source." + sha256_bytes(full_key.encode("utf-8"))[:20]
    seed["sourceNode"] = full_key
    detail = seed.setdefault("detail", {})
    timing = detail.setdefault("timing", {})
    timing["startDelaySeconds"] = occurrence["sourceTimeSeconds"]
    return seed


def reconcile_effect_document(
    existing: dict[str, Any], candidates: list[dict[str, Any]]
) -> dict[str, Any]:
    elements = existing.get("elements", [])
    by_source = {
        str(row.get("sourceNode") or ""): row
        for row in elements
        if row.get("sourceNode")
    }
    if len(by_source) != sum(bool(row.get("sourceNode")) for row in elements):
        raise InventoryError("existing document has duplicate sourceNode")
    ids = {str(row.get("id") or "") for row in elements}
    additions = []
    source_rebase = []
    for candidate in candidates:
        source_key = str(candidate["sourceNode"])
        current = by_source.get(source_key)
        if current is not None:
            current_recipe = current.get("sourceRecipe")
            candidate_recipe = candidate.get("sourceRecipe")
            current_material = str(
                (current.get("material") or {}).get("sourceMaterialPath") or ""
            )
            candidate_material = str(
                (candidate.get("material") or {}).get("sourceMaterialPath") or ""
            )
            if (
                canonical_sha256(current_recipe) != canonical_sha256(candidate_recipe)
                or current_material.casefold() != candidate_material.casefold()
            ):
                source_rebase.append(source_key)
            continue
        if candidate["id"] in ids:
            raise InventoryError(
                f"candidate element ID collides with authored row: {candidate['id']}"
            )
        ids.add(candidate["id"])
        additions.append(copy.deepcopy(candidate))
    return {
        "effectAssetId": str(existing.get("effectAssetId") or ""),
        "existingElementCount": len(elements),
        "addElements": additions,
        "preservedExistingElementCount": len(elements),
        "sourceRebaseRequired": sorted(source_rebase),
        "legacyGenericRetireCandidates": sorted(
            str(row.get("id") or "")
            for row in elements
            if is_legacy_generic_default(row)
        ),
        "deleteElements": [],
    }


def build_reconcile_plan(
    inventory: dict[str, Any], authored_root: Path = AUTHORED_ROOT
) -> dict[str, Any]:
    cues = cue_effect_index(read_json(CUE_PATH))
    systems = {
        row["sourceSystemId"]: {
            carrier["carrierKey"]: carrier for carrier in row["carriers"]
        }
        for row in inventory["sourceSystems"]
    }
    candidates: dict[str, list[dict[str, Any]]] = defaultdict(list)
    unresolved = []
    for occurrence in inventory["occurrences"]:
        if occurrence["reachabilityDisposition"] != "REACHABLE_REVIEWED":
            continue
        effect_id = cues.get(
            (
                occurrence["patternId"],
                str(occurrence.get("gameplayActionId") or ""),
            )
        )
        if not effect_id:
            unresolved.append(
                {
                    "occurrenceId": occurrence["occurrenceId"],
                    "reason": "NO_PRODUCT_CUE_FOR_REVIEWED_OCCURRENCE",
                }
            )
            continue
        system = systems.get(occurrence.get("sourceSystemId") or "", {})
        for carrier in system.values():
            if (
                carrier["disposition"] != "EXECUTABLE_CORE"
                or carrier.get("elementSeed") is None
            ):
                continue
            candidates[effect_id].append(
                occurrence_element_seed(occurrence, carrier)
            )

    documents = []
    for effect_id in sorted(candidates):
        path = authored_root / f"{effect_id}.effect.json"
        if not path.is_file():
            unresolved.append(
                {
                    "effectAssetId": effect_id,
                    "reason": "AUTHORED_DOCUMENT_MISSING",
                }
            )
            continue
        plan = reconcile_effect_document(read_json(path), candidates[effect_id])
        plan["authoringPath"] = path.relative_to(ROOT).as_posix()
        documents.append(plan)
    return {
        "schema": "lostark.valtan-effect-reconcile-plan",
        "formatVersion": 1,
        "mode": "REPORT_ONLY_MISSING_SOURCE_ELEMENTS",
        "documents": documents,
        "unresolved": unresolved,
        "summary": {
            "documentCount": len(documents),
            "missingElementCount": sum(
                len(row["addElements"]) for row in documents
            ),
            "preservedElementCount": sum(
                row["preservedExistingElementCount"] for row in documents
            ),
            "sourceRebaseRequiredCount": sum(
                len(row["sourceRebaseRequired"]) for row in documents
            ),
            "legacyGenericRetireCandidateCount": sum(
                len(row["legacyGenericRetireCandidates"]) for row in documents
            ),
            "deletedElementCount": 0,
            "unresolvedCount": len(unresolved),
        },
    }


def write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".staging")
    temporary.write_bytes(payload)
    temporary.replace(path)


def check_exact(path: Path, payload: bytes) -> None:
    if not path.is_file():
        raise InventoryError(f"checked inventory is missing: {path}")
    actual = path.read_bytes()
    if actual != payload:
        raise InventoryError(
            f"checked inventory drifted: {path}\n"
            f"  expected {sha256_bytes(payload)}\n"
            f"  actual   {sha256_bytes(actual)}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument("--output", type=Path, default=OUTPUT_PATH)
    parser.add_argument(
        "--selection-manifest",
        type=Path,
        help=(
            "authoritative reviewed branch selections; recommendations alone "
            "never admit a branch"
        ),
    )
    parser.add_argument(
        "--reconcile-plan",
        type=Path,
        help=(
            "write a report-only missing-element plan; never writes authored "
            "Effect documents"
        ),
    )
    args = parser.parse_args()

    selection_sources: list[Path] = []
    if args.selection_manifest is not None:
        manifest_path = args.selection_manifest.resolve()
        manifest = load_selection_manifest(manifest_path)
        previous = {
            "reviewedBranchSelections": copy.deepcopy(
                manifest["selections"]
            )
        }
        selection_sources.append(manifest_path)
    else:
        previous = read_json(args.output) if args.output.is_file() else None
    inventory = build_inventory(
        previous,
        include_payloads=False,
        additional_repository_sources=selection_sources,
    )
    payload = pretty_json_bytes(inventory)
    if args.check:
        if args.reconcile_plan is not None:
            parser.error("--check cannot write a reconcile plan")
        check_exact(args.output, payload)
        label = "checked"
    elif args.write:
        write_atomic(args.output, payload)
        label = "written"
    else:
        label = "dry-run"

    if args.reconcile_plan is not None:
        authored = AUTHORED_ROOT.resolve()
        target = args.reconcile_plan.resolve()
        if target == authored or authored in target.parents:
            parser.error("reconcile plan cannot target Data/Effects/Authored")
        plan_inventory = build_inventory(
            previous,
            include_payloads=True,
            additional_repository_sources=selection_sources,
        )
        plan = build_reconcile_plan(plan_inventory)
        write_atomic(args.reconcile_plan, pretty_json_bytes(plan))

    print(
        "Valtan source occurrence inventory "
        f"{label}: {json.dumps(inventory['summary'], ensure_ascii=False, sort_keys=True)}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except InventoryError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
