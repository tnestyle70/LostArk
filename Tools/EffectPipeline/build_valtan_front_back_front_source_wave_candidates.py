#!/usr/bin/env python3
"""Build four immutable FRONT_BACK_FRONT 19_01 source-wave candidates.

The reviewed aggregate source document intentionally remains untouched.  Its
four exact visual timing groups are split into four direct-authored v13
candidate documents, one cue occurrence per source wave.  The first three are
presentation waves and the fourth is explicitly an auxiliary source wave; no
row in this artifact changes Server hit authority.
"""

from __future__ import annotations

import argparse
from copy import deepcopy
import hashlib
import json
import math
from pathlib import Path, PurePosixPath
import sys
from typing import Any, Mapping


SCHEMA = "lostark.valtan-front-back-front-source-wave-candidates"
FORMAT_VERSION = 1
BOSS_ARCHETYPE_ID = "BOSS_VALTAN"
PATTERN_ID = "VALTAN_FRONT_BACK_FRONT"
STAGE_ID = "SMASHES"
ACTION_ID = "valtan.attack.front-back-front.active"
CLIP_OCCURRENCE_ID = "valtan.attack.front-back-front.active.clip.01"
CLIP_NAME = "mesh_att_battle_19_01"
AGGREGATE_EFFECT_ID = "effect.valtan.front-back-front.active"
WHIRLWIND_EFFECT_ID = "effect.valtan.pattern.420633.active"
EXPECTED_WAVE_COUNT = 4
EXPECTED_ELEMENTS_PER_WAVE = 25
EXPECTED_ELEMENT_COUNT = 100
EXPECTED_NOTIFY_SYSTEM_GROUP_COUNT = 12

REVIEWED_RECEIPT = PurePosixPath(
    "Data/Effects/Imported/Valtan/ReviewedSourceFamilies/"
    "Valtan.reviewed-source-family-candidates.v1.json"
)
PATTERN_BINDINGS = PurePosixPath(
    "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
)
ENCOUNTER = PurePosixPath("Data/Encounters/Valtan/ValtanEncounter.json")
CATALOG = PurePosixPath("Data/Effects/EffectCatalog.json")
CUES = PurePosixPath(
    "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
PROJECT_PLAN = PurePosixPath(
    "Data/Effects/Imported/Valtan/ProjectAuthoredPriority/"
    "Valtan.project-authored-priority.patch-plan.v1.json"
)
OUTPUT_DIRECTORY = PurePosixPath(
    "Data/Effects/Imported/Valtan/FrontBackFrontSourceWaves"
)
OUTPUT_RECEIPT = OUTPUT_DIRECTORY / PurePosixPath(
    "Valtan.front-back-front-source-wave-candidates.v1.json"
)


class CandidateError(RuntimeError):
    """The source closure cannot produce deterministic safe candidates."""


def _json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def _sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _json_sha(value: Any) -> str:
    return _sha256(_json_bytes(value))


def _repository_path(root: Path, relative: PurePosixPath) -> Path:
    path = root.joinpath(*relative.parts)
    try:
        path.resolve(strict=False).relative_to(root.resolve())
    except ValueError as exc:
        raise CandidateError(f"path escaped repository: {relative}") from exc
    return path


def _load_json(root: Path, relative: PurePosixPath) -> tuple[dict[str, Any], bytes]:
    path = _repository_path(root, relative)
    try:
        payload = path.read_bytes()
    except OSError as exc:
        raise CandidateError(f"cannot read {relative}: {exc}") from exc
    if payload.startswith(b"\xef\xbb\xbf"):
        raise CandidateError(f"JSON must be UTF-8 without BOM: {relative}")

    def no_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
        result: dict[str, Any] = {}
        for key, value in pairs:
            if key in result:
                raise CandidateError(f"duplicate property {key!r}: {relative}")
            result[key] = value
        return result

    try:
        value = json.loads(payload.decode("utf-8"), object_pairs_hook=no_duplicates)
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise CandidateError(f"invalid JSON {relative}: {exc}") from exc
    if not isinstance(value, dict):
        raise CandidateError(f"JSON root must be an object: {relative}")
    return value, payload


def _require_list(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise CandidateError(f"{label} must be an array")
    return value


def _require_object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise CandidateError(f"{label} must be an object")
    return value


def _relative(value: Any, label: str) -> PurePosixPath:
    if not isinstance(value, str) or not value or "\\" in value:
        raise CandidateError(f"{label} must be a repository-relative POSIX path")
    path = PurePosixPath(value)
    if path.is_absolute() or "." in path.parts or ".." in path.parts:
        raise CandidateError(f"{label} escaped repository")
    return path


def _find_unique(rows: list[Any], predicate: Any, label: str) -> dict[str, Any]:
    matches = [row for row in rows if isinstance(row, dict) and predicate(row)]
    if len(matches) != 1:
        raise CandidateError(f"expected exactly one {label}; found {len(matches)}")
    return matches[0]


def _wave_identity(ordinal: int) -> dict[str, str]:
    if ordinal < 4:
        suffix = f"source-wave-{ordinal:02d}"
        return {
            "waveId": suffix,
            "effectAssetId": f"effect.valtan.front-back-front.{suffix}",
            "displayLabel": f"Source Wave {ordinal:02d}",
            "documentDisplayName": (
                f"FRONT_BACK_FRONT 19_01 / Source Wave {ordinal:02d}"
            ),
            "presentationRole": "source-wave",
            "gameplayHitDisposition": "PRESENTATION_ONLY_SERVER_HIT_UNCHANGED",
        }
    return {
        "waveId": "auxiliary-source-wave",
        "effectAssetId": (
            "effect.valtan.front-back-front.auxiliary-source-wave"
        ),
        "displayLabel": "Source Wave Aux",
        "documentDisplayName": (
            "FRONT_BACK_FRONT 19_01 / Auxiliary Source Wave"
        ),
        "presentationRole": "auxiliary-source-wave",
        "gameplayHitDisposition": "FORBIDDEN_AUXILIARY_NOT_GAMEPLAY_HIT",
    }


def _cue_row(identity: Mapping[str, str], source_start_ms: int) -> dict[str, Any]:
    suffix = identity["waveId"]
    return {
        "bindingId": f"cue.valtan.front-back-front.{suffix}",
        "occurrenceId": (
            f"cue.valtan.front-back-front.{suffix}.occurrence.01"
        ),
        "patternId": PATTERN_ID,
        "stageId": STAGE_ID,
        "actionId": ACTION_ID,
        "clipOccurrenceId": CLIP_OCCURRENCE_ID,
        "effectAssetId": identity["effectAssetId"],
        "anchorSlotId": "root",
        "followPolicy": "follow",
        "stopPolicy": "natural",
        "repeatPolicy": "once",
        "sourceStartMs": source_start_ms,
        "sourceEndMs": None,
        "localTransform": {
            "position": [0, 0, 0],
            "rotationDegrees": [0, 0, 0],
            "scale": [1, 1, 1],
        },
    }


def _load_sealed_outputs(
    root: Path,
) -> tuple[dict[PurePosixPath, bytes], dict[str, Any]] | None:
    """Load the already-proven immutable candidates after aggregate cleanup."""
    receipt_path = _repository_path(root, OUTPUT_RECEIPT)
    if not receipt_path.is_file():
        return None
    receipt, receipt_payload = _load_json(root, OUTPUT_RECEIPT)
    if (
        receipt.get("schema") != SCHEMA
        or receipt.get("formatVersion") != FORMAT_VERSION
        or receipt.get("bossArchetypeId") != BOSS_ARCHETYPE_ID
        or receipt.get("mode")
        != "IMMUTABLE_FOUR_WAVE_CANDIDATES_NO_CANONICAL_MUTATION"
    ):
        raise CandidateError("sealed source-wave receipt identity is invalid")
    summary = _require_object(receipt.get("summary"), "sealed summary")
    expected_summary = {
        "candidateDocumentCount": 4,
        "candidateElementCount": 100,
        "elementsPerCandidateDocument": 25,
        "visualTimingGroupCount": 4,
        "notifySystemTimingGroupCount": 12,
        "catalogAppendRowCount": 4,
        "cueAppendRowCount": 4,
        "targetAuthoredDocumentCount": 4,
        "aggregateSourceElementAppendCount": 0,
        "duplicateSourceElementCount": 0,
        "auxiliarySourceWaveCount": 1,
        "canonicalMutationCount": 0,
    }
    if summary != expected_summary:
        raise CandidateError("sealed source-wave summary drift")
    candidates = [
        _require_object(row, "sealed candidate")
        for row in _require_list(receipt.get("candidates"), "sealed candidates")
    ]
    if (
        len(candidates) != EXPECTED_WAVE_COUNT
        or [row.get("waveOrdinal") for row in candidates] != [1, 2, 3, 4]
    ):
        raise CandidateError("sealed source-wave candidate order drift")
    outputs: dict[PurePosixPath, bytes] = {}
    all_pairs: set[tuple[str, str]] = set()
    for candidate in candidates:
        effect_id = candidate.get("effectAssetId")
        relative = _relative(
            candidate.get("candidateDocumentPath"),
            f"{effect_id} sealed candidate path",
        )
        if relative.parent != OUTPUT_DIRECTORY:
            raise CandidateError("sealed source-wave candidate escaped output directory")
        document, payload = _load_json(root, relative)
        if (
            _sha256(payload) != candidate.get("candidateDocumentSha256")
            or document.get("schema") != "lostark.effect-authoring"
            or document.get("version") != 13
            or document.get("effectAssetId") != effect_id
        ):
            raise CandidateError(f"sealed candidate SHA/identity drift: {effect_id}")
        elements = [
            _require_object(row, f"{effect_id} sealed element")
            for row in _require_list(document.get("elements"), f"{effect_id}.elements")
        ]
        expected_pairs = [
            (row.get("elementId"), row.get("sourceNode"))
            for row in _require_list(
                candidate.get("candidateElements"),
                f"{effect_id}.candidateElements",
            )
            if isinstance(row, dict)
        ]
        actual_pairs = [(row.get("id"), row.get("sourceNode")) for row in elements]
        if (
            len(elements) != EXPECTED_ELEMENTS_PER_WAVE
            or actual_pairs != expected_pairs
            or len(set(actual_pairs)) != EXPECTED_ELEMENTS_PER_WAVE
            or all_pairs.intersection(actual_pairs)
        ):
            raise CandidateError(f"sealed candidate element closure drift: {effect_id}")
        all_pairs.update(actual_pairs)
        outputs[relative] = payload
    if len(all_pairs) != EXPECTED_ELEMENT_COUNT:
        raise CandidateError("sealed source-wave denominator must remain 100")
    outputs[OUTPUT_RECEIPT] = receipt_payload
    return outputs, receipt


def _catalog_row(effect_id: str) -> dict[str, str]:
    return {
        "effectAssetId": effect_id,
        "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
        "authoringPath": f"Effects/Authored/{effect_id}.effect.json",
    }


def build_outputs(repository_root: Path) -> tuple[dict[PurePosixPath, bytes], dict[str, Any]]:
    root = repository_root.resolve()
    reviewed, reviewed_payload = _load_json(root, REVIEWED_RECEIPT)
    if (
        reviewed.get("schema") != "lostark.valtan-reviewed-source-family-candidates"
        or reviewed.get("formatVersion") != 1
        or reviewed.get("bossArchetypeId") != BOSS_ARCHETYPE_ID
    ):
        raise CandidateError("reviewed source-family receipt identity is invalid")
    reviewed_documents = _require_list(
        reviewed.get("documents"), "reviewed documents"
    )
    aggregate_rows = [
        row
        for row in reviewed_documents
        if isinstance(row, dict)
        and row.get("effectAssetId") == AGGREGATE_EFFECT_ID
    ]
    if not aggregate_rows:
        sealed = _load_sealed_outputs(root)
        if sealed is not None:
            return sealed
        raise CandidateError(
            "reviewed FRONT_BACK_FRONT aggregate is absent and no sealed candidates exist"
        )
    if len(aggregate_rows) != 1:
        raise CandidateError(
            "expected exactly one FRONT_BACK_FRONT aggregate candidate row; "
            f"found {len(aggregate_rows)}"
        )
    aggregate_row = aggregate_rows[0]
    if aggregate_row.get("candidateElementCount") != EXPECTED_ELEMENT_COUNT:
        raise CandidateError("FRONT_BACK_FRONT aggregate must remain 100 elements")
    source_candidate_relative = _relative(
        aggregate_row.get("candidateDocumentPath"), "aggregate candidate path"
    )
    source_candidate, source_candidate_payload = _load_json(
        root, source_candidate_relative
    )
    if (
        _sha256(source_candidate_payload)
        != aggregate_row.get("candidateDocumentSha256")
        or source_candidate.get("effectAssetId") != AGGREGATE_EFFECT_ID
        or source_candidate.get("version") != 13
    ):
        raise CandidateError("aggregate source candidate SHA/identity drift")
    source_elements = [
        _require_object(row, "aggregate source element")
        for row in _require_list(source_candidate.get("elements"), "source elements")
    ]
    by_element_id = {row.get("id"): row for row in source_elements}
    if (
        len(source_elements) != EXPECTED_ELEMENT_COUNT
        or len(by_element_id) != EXPECTED_ELEMENT_COUNT
        or None in by_element_id
    ):
        raise CandidateError("aggregate source element identities are invalid")

    visual_groups = [
        _require_object(row, "visual timing group")
        for row in _require_list(
            aggregate_row.get("visualTimingGroups"), "visual timing groups"
        )
    ]
    visual_groups.sort(key=lambda row: float(row.get("sourceTimeSeconds", -1)))
    notify_groups = [
        _require_object(row, "notify/system timing group")
        for row in _require_list(
            aggregate_row.get("notifySystemTimingGroups"),
            "notify/system timing groups",
        )
    ]
    if (
        len(visual_groups) != EXPECTED_WAVE_COUNT
        or len(notify_groups) != EXPECTED_NOTIFY_SYSTEM_GROUP_COUNT
        or any(row.get("elementCount") != EXPECTED_ELEMENTS_PER_WAVE for row in visual_groups)
    ):
        raise CandidateError("FRONT_BACK_FRONT timing groups are not 4 x 25 / 12")
    grouped_ids = [
        element_id
        for group in visual_groups
        for element_id in _require_list(group.get("elementIds"), "group element IDs")
    ]
    if len(grouped_ids) != EXPECTED_ELEMENT_COUNT or set(grouped_ids) != set(by_element_id):
        raise CandidateError("visual timing groups do not partition the 100 source elements")
    if len(set(grouped_ids)) != EXPECTED_ELEMENT_COUNT:
        raise CandidateError("a source element occurs in more than one visual timing group")

    bindings, bindings_payload = _load_json(root, PATTERN_BINDINGS)
    binding = _find_unique(
        _require_list(bindings.get("bindings"), "pattern bindings"),
        lambda row: row.get("actionId") == ACTION_ID,
        "FRONT_BACK_FRONT active animation binding",
    )
    clips = _require_list(binding.get("clips"), "FRONT_BACK_FRONT clips")
    clip = _find_unique(
        clips,
        lambda row: row.get("clipOccurrenceId") == CLIP_OCCURRENCE_ID,
        "FRONT_BACK_FRONT 19_01 clip occurrence",
    )
    if (
        len(clips) != 1
        or clip.get("clip") != CLIP_NAME
        or clip.get("sourceStartMs") != 0
        or clip.get("playMs") != 0
        or clip.get("playRate") != 1.0
        or clip.get("loop") is not True
    ):
        raise CandidateError("FRONT_BACK_FRONT animation binding/clip drift")

    encounter, encounter_payload = _load_json(root, ENCOUNTER)
    pattern = _find_unique(
        _require_list(encounter.get("patterns"), "encounter patterns"),
        lambda row: row.get("patternId") == PATTERN_ID,
        "FRONT_BACK_FRONT encounter pattern",
    )
    stage = _find_unique(
        _require_list(pattern.get("stages"), "FRONT_BACK_FRONT stages"),
        lambda row: row.get("stageId") == STAGE_ID and row.get("actionId") == ACTION_ID,
        "FRONT_BACK_FRONT SMASHES stage",
    )
    if stage.get("durationMs") != 5000 or stage.get("hitCount") != 3:
        raise CandidateError("FRONT_BACK_FRONT Server stage authority drift")

    catalog, catalog_payload = _load_json(root, CATALOG)
    aggregate_catalog_row = _find_unique(
        _require_list(catalog.get("effects"), "EffectCatalog rows"),
        lambda row: row.get("effectAssetId") == AGGREGATE_EFFECT_ID,
        "aggregate catalog row",
    )
    cues, cues_payload = _load_json(root, CUES)
    aggregate_cue_row = _find_unique(
        _require_list(cues.get("cues"), "Valtan cues"),
        lambda row: row.get("effectAssetId") == AGGREGATE_EFFECT_ID
        and row.get("clipOccurrenceId") == CLIP_OCCURRENCE_ID,
        "aggregate FRONT_BACK_FRONT cue",
    )
    if aggregate_cue_row != aggregate_row["clipOccurrences"][0]["cueRow"]:
        raise CandidateError("aggregate cue no longer matches reviewed source receipt")

    aggregate_authored_relative = _relative(
        aggregate_row.get("authoredDocumentPath"), "aggregate authored path"
    )
    aggregate_authored, aggregate_authored_payload = _load_json(
        root, aggregate_authored_relative
    )
    if aggregate_authored.get("effectAssetId") != AGGREGATE_EFFECT_ID:
        raise CandidateError("aggregate authored document identity drift")

    project_plan, project_plan_payload = _load_json(root, PROJECT_PLAN)
    project_target = _find_unique(
        _require_list(project_plan.get("targets"), "project overlay targets"),
        lambda row: row.get("targetEffectAssetId") == AGGREGATE_EFFECT_ID,
        "FRONT_BACK_FRONT project aggregate overlay target",
    )
    project_overlay_relative = _relative(
        project_target.get("overlayDocumentPath"), "project overlay path"
    )
    project_overlay, project_overlay_payload = _load_json(
        root, project_overlay_relative
    )
    if (
        project_overlay.get("effectAssetId") != AGGREGATE_EFFECT_ID
        or _sha256(project_overlay_payload) != project_target.get("overlayDocumentSha256")
    ):
        raise CandidateError("FRONT_BACK_FRONT project overlay canary drift")

    whirlwind_row = _find_unique(
        _require_list(reviewed.get("protectedCanaries"), "protected canaries"),
        lambda row: row.get("effectAssetId") == WHIRLWIND_EFFECT_ID,
        "Whirlwind active protected canary",
    )
    whirlwind_relative = _relative(
        whirlwind_row.get("authoredDocumentPath"), "Whirlwind canary path"
    )
    _, whirlwind_payload = _load_json(root, whirlwind_relative)
    if _sha256(whirlwind_payload) != whirlwind_row.get("authoredDocumentSha256"):
        raise CandidateError("Whirlwind active byte canary drift")

    source_key_by_id = {
        row["id"]: row
        for row in _require_list(aggregate_row.get("sourceElementKeys"), "source keys")
    }
    outputs: dict[PurePosixPath, bytes] = {}
    candidates: list[dict[str, Any]] = []
    all_candidate_ids: list[str] = []
    for ordinal, group in enumerate(visual_groups, start=1):
        identity = _wave_identity(ordinal)
        source_time = float(group["sourceTimeSeconds"])
        source_start_ms = math.floor(source_time * 1000.0)
        local_delay = source_time - source_start_ms / 1000.0
        if (
            source_start_ms < int(clip["sourceStartMs"])
            or source_start_ms >= int(stage["durationMs"])
            or local_delay < 0.0
            or local_delay >= 0.001000001
        ):
            raise CandidateError(f"wave {ordinal} integer-floor cue window is unsafe")
        group_ids = list(group["elementIds"])
        wave_elements: list[dict[str, Any]] = []
        wave_source_keys: list[dict[str, Any]] = []
        for element_id in group_ids:
            if element_id not in by_element_id or element_id not in source_key_by_id:
                raise CandidateError(f"wave {ordinal} references an unknown source element")
            source_key = source_key_by_id[element_id]
            if (
                source_key.get("visualTimingGroupId") != group["visualTimingGroupId"]
                or source_key.get("elementStartDelaySeconds")
                != group["elementStartDelaySeconds"]
            ):
                raise CandidateError(f"wave {ordinal} source timing identity drift")
            element = deepcopy(by_element_id[element_id])
            timing = _require_object(
                _require_object(element.get("detail"), "candidate detail").get("timing"),
                "candidate timing",
            )
            timing["startDelaySeconds"] = local_delay
            wave_elements.append(element)
            wave_source_keys.append(deepcopy(source_key))
        candidate_document = {
            "schema": "lostark.effect-authoring",
            "version": 13,
            "effectAssetId": identity["effectAssetId"],
            "displayName": identity["documentDisplayName"],
            "particleSystem": deepcopy(source_candidate["particleSystem"]),
            "modelCues": [],
            "elements": wave_elements,
        }
        candidate_relative = OUTPUT_DIRECTORY / PurePosixPath(
            f"{identity['effectAssetId']}.source-wave-candidate.effect.json"
        )
        candidate_payload = _json_bytes(candidate_document)
        outputs[candidate_relative] = candidate_payload
        cue = _cue_row(identity, source_start_ms)
        target_relative = PurePosixPath(
            f"Data/Effects/Authored/{identity['effectAssetId']}.effect.json"
        )
        group_notify_rows = sorted(
            [
                deepcopy(row)
                for row in notify_groups
                if row.get("visualTimingGroupId") == group["visualTimingGroupId"]
            ],
            key=lambda row: row["notifySystemTimingGroupId"],
        )
        if len(group_notify_rows) != 3:
            raise CandidateError(f"wave {ordinal} must retain exactly 3 notify/system groups")
        candidates.append(
            {
                "waveOrdinal": ordinal,
                "waveId": identity["waveId"],
                "displayLabel": identity["displayLabel"],
                "documentDisplayName": identity["documentDisplayName"],
                "presentationRole": identity["presentationRole"],
                "gameplayHitDisposition": identity["gameplayHitDisposition"],
                "visualTimingGroup": deepcopy(group),
                "notifySystemTimingGroups": group_notify_rows,
                "sourceTimeSeconds": source_time,
                "cueSourceStartRounding": "SAFE_INTEGER_FLOOR",
                "cueSourceStartMs": source_start_ms,
                "cueLocalStartDelaySeconds": local_delay,
                "effectAssetId": identity["effectAssetId"],
                "candidateDocumentPath": candidate_relative.as_posix(),
                "candidateDocumentSha256": _sha256(candidate_payload),
                "targetAuthoredDocumentPath": target_relative.as_posix(),
                "candidateElementCount": len(wave_elements),
                "candidateElements": [
                    {"elementId": row["id"], "sourceNode": row["sourceNode"]}
                    for row in wave_elements
                ],
                "sourceElementKeys": wave_source_keys,
                "catalogRow": _catalog_row(identity["effectAssetId"]),
                "cueRow": cue,
            }
        )
        all_candidate_ids.extend(group_ids)
    if len(all_candidate_ids) != EXPECTED_ELEMENT_COUNT or len(set(all_candidate_ids)) != EXPECTED_ELEMENT_COUNT:
        raise CandidateError("wave candidates duplicate or omit exact source elements")
    if (
        candidates[-1]["presentationRole"] != "auxiliary-source-wave"
        or candidates[-1]["gameplayHitDisposition"]
        != "FORBIDDEN_AUXILIARY_NOT_GAMEPLAY_HIT"
    ):
        raise CandidateError("fourth wave is not explicitly non-gameplay auxiliary")

    source_guards = [
        {"path": REVIEWED_RECEIPT.as_posix(), "sha256": _sha256(reviewed_payload)},
        {"path": source_candidate_relative.as_posix(), "sha256": _sha256(source_candidate_payload)},
        {"path": PATTERN_BINDINGS.as_posix(), "sha256": _sha256(bindings_payload)},
        {"path": ENCOUNTER.as_posix(), "sha256": _sha256(encounter_payload)},
        {"path": CATALOG.as_posix(), "sha256": _sha256(catalog_payload)},
        {"path": CUES.as_posix(), "sha256": _sha256(cues_payload)},
        {"path": PROJECT_PLAN.as_posix(), "sha256": _sha256(project_plan_payload)},
        {"path": project_overlay_relative.as_posix(), "sha256": _sha256(project_overlay_payload)},
    ]
    source_guards.sort(key=lambda row: row["path"])
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": BOSS_ARCHETYPE_ID,
        "mode": "IMMUTABLE_FOUR_WAVE_CANDIDATES_NO_CANONICAL_MUTATION",
        "sourceGuards": source_guards,
        "clipIdentity": {
            "patternId": PATTERN_ID,
            "stageId": STAGE_ID,
            "actionId": ACTION_ID,
            "clipOccurrenceId": CLIP_OCCURRENCE_ID,
            "clipName": CLIP_NAME,
            "clipSourceStartMs": clip["sourceStartMs"],
            "clipPlayMs": clip["playMs"],
            "clipPlayRate": clip["playRate"],
            "clipLoop": clip["loop"],
            "stageDurationMs": stage["durationMs"],
            "serverHitCount": stage["hitCount"],
            "animationBindingMutationDisposition": "FORBIDDEN",
        },
        "aggregateCanary": {
            "effectAssetId": AGGREGATE_EFFECT_ID,
            "displayLabel": "Project Tuned Aggregate",
            "authoredDocumentPath": aggregate_authored_relative.as_posix(),
            "authoredDocumentSha256": _sha256(aggregate_authored_payload),
            "catalogRow": deepcopy(aggregate_catalog_row),
            "catalogRowSha256": _json_sha(aggregate_catalog_row),
            "cueRow": deepcopy(aggregate_cue_row),
            "cueRowSha256": _json_sha(aggregate_cue_row),
            "projectPatchPlanPath": PROJECT_PLAN.as_posix(),
            "projectPatchPlanSha256": _sha256(project_plan_payload),
            "projectOverlayPath": project_overlay_relative.as_posix(),
            "projectOverlaySha256": _sha256(project_overlay_payload),
            "sourceElementAppendCount": 0,
            "disposition": "PRESERVE_EXISTING_PROJECT_TUNED_AGGREGATE",
        },
        "whirlwindCanary": {
            "effectAssetId": WHIRLWIND_EFFECT_ID,
            "authoredDocumentPath": whirlwind_relative.as_posix(),
            "authoredDocumentSha256": _sha256(whirlwind_payload),
            "disposition": "PROTECTED_BYTE_CANARY_NO_MUTATION",
        },
        "allEffectsClipDisplay": {
            "expectedCueCountAfterApply": 5,
            "cueDisplayOrder": [
                {
                    "displayLabel": "Project Tuned Aggregate",
                    "effectAssetId": AGGREGATE_EFFECT_ID,
                    "occurrenceId": aggregate_cue_row["occurrenceId"],
                    "sourceStartMs": aggregate_cue_row["sourceStartMs"],
                    "disposition": "PRESERVE_EXISTING",
                },
                *[
                    {
                        "displayLabel": row["displayLabel"],
                        "effectAssetId": row["effectAssetId"],
                        "occurrenceId": row["cueRow"]["occurrenceId"],
                        "sourceStartMs": row["cueSourceStartMs"],
                        "disposition": "APPEND_SEPARATE_SOURCE_CUE",
                    }
                    for row in candidates
                ],
            ],
        },
        "candidates": candidates,
        "summary": {
            "candidateDocumentCount": 4,
            "candidateElementCount": 100,
            "elementsPerCandidateDocument": 25,
            "visualTimingGroupCount": 4,
            "notifySystemTimingGroupCount": 12,
            "catalogAppendRowCount": 4,
            "cueAppendRowCount": 4,
            "targetAuthoredDocumentCount": 4,
            "aggregateSourceElementAppendCount": 0,
            "duplicateSourceElementCount": 0,
            "auxiliarySourceWaveCount": 1,
            "canonicalMutationCount": 0,
        },
    }
    outputs[OUTPUT_RECEIPT] = _json_bytes(receipt)
    return outputs, receipt


def write_outputs(root: Path, outputs: Mapping[PurePosixPath, bytes]) -> None:
    for relative, payload in sorted(outputs.items(), key=lambda item: item[0].as_posix()):
        path = _repository_path(root, relative)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(payload)


def check_outputs(root: Path, outputs: Mapping[PurePosixPath, bytes]) -> None:
    stale: list[str] = []
    for relative, payload in outputs.items():
        path = _repository_path(root, relative)
        actual = path.read_bytes() if path.is_file() else None
        if actual != payload:
            stale.append(relative.as_posix())
    if stale:
        raise CandidateError("candidate outputs are missing or stale: " + ", ".join(sorted(stale)))


def _make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parents[2])
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _make_parser().parse_args(argv)
    try:
        outputs, receipt = build_outputs(args.repo_root)
        if args.write:
            write_outputs(args.repo_root.resolve(), outputs)
            mode = "written"
        else:
            check_outputs(args.repo_root.resolve(), outputs)
            mode = "checked"
    except CandidateError as exc:
        print(f"[FAILURE] {exc}", file=sys.stderr)
        return 1
    summary = receipt["summary"]
    print(
        "Valtan FRONT_BACK_FRONT source-wave candidates " + mode + ": "
        + json.dumps(summary, sort_keys=True)
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
