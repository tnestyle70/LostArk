#!/usr/bin/env python3
"""Promote reviewed Valtan animation chains into managed audition patterns.

The animator-owned debug document remains an intake library.  This projector
freezes its native clip lengths against the cooked Valtan model and produces
the same split gameplay/presentation contract used by phase one.  Every source
occurrence becomes one Server stage, so an explicitly extended clip can loop
for exactly the authored wall time without changing the next occurrence.

Promotion is intentionally non-destructive to live combat: every generated
pattern is owned by ``decisionModel.manualAuditions`` and compiles to
``AUDITION_ONLY``.  Pattern/effect authors can therefore play it through the
existing stable-ID Server audition command while normal rotation ignores it.
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
import tempfile
from pathlib import Path
from typing import Any, Mapping


MANIFEST_REL = "Data/Valtan/Valtan.animation-chain-promotions.json"
DEBUG_REL = "Data/Valtan/Valtan.presentation.debug.json"
GAMEPLAY_REL = "Data/Valtan/Valtan.gameplay.json"
PRESENTATION_REL = "Data/Valtan/Valtan.presentation.json"
RECEIPT_REL = "Data/Valtan/Valtan.animation-chain-promotion.receipt.json"
ANIM_NOTIFY_REL = "Data/Animation/Reference/Valtan/Valtan.animnotify"

STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]{1,160}$")
SHA256 = re.compile(r"^[0-9a-f]{64}$")
ANIMATION_INTAKE_ONLY = "ANIMATION_INTAKE_ONLY"


class PromotionError(RuntimeError):
    pass


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise PromotionError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(
            path.read_text(encoding="utf-8"),
            object_pairs_hook=_reject_duplicate_pairs,
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise PromotionError(f"cannot read strict JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise PromotionError(f"JSON root must be an object: {path}")
    return value


def _exact(value: Mapping[str, Any], fields: tuple[str, ...], context: str) -> None:
    if not isinstance(value, dict) or set(value) != set(fields):
        actual = sorted(value) if isinstance(value, dict) else type(value).__name__
        raise PromotionError(
            f"{context} properties mismatch: expected={list(fields)} actual={actual}"
        )


def _required_with_optional(
    value: Mapping[str, Any],
    required: tuple[str, ...],
    optional: tuple[str, ...],
    context: str,
) -> None:
    if not isinstance(value, dict):
        raise PromotionError(
            f"{context} properties mismatch: expected object actual={type(value).__name__}"
        )
    actual = set(value)
    required_set = set(required)
    allowed = required_set | set(optional)
    if not required_set.issubset(actual) or not actual.issubset(allowed):
        raise PromotionError(
            f"{context} properties mismatch: required={list(required)} "
            f"optional={list(optional)} actual={sorted(actual)}"
        )


def _stable(value: Any, context: str) -> str:
    if not isinstance(value, str) or STABLE_ID.fullmatch(value) is None:
        raise PromotionError(f"{context} is not a stable ID: {value!r}")
    return value


def _integer(value: Any, context: str, minimum: int = 0) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        raise PromotionError(f"{context} must be an integer >= {minimum}")
    return value


def _number(value: Any, context: str, minimum: float = 0.0) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise PromotionError(f"{context} must be a finite number")
    result = float(value)
    if not math.isfinite(result) or result < minimum:
        raise PromotionError(f"{context} must be a finite number >= {minimum}")
    return result


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as source:
            for block in iter(lambda: source.read(1024 * 1024), b""):
                digest.update(block)
    except OSError as exc:
        raise PromotionError(f"cannot hash {path}: {exc}") from exc
    return digest.hexdigest()


def _lround_positive(value: float) -> int:
    if not math.isfinite(value) or value < 0.0:
        raise PromotionError(f"cannot round invalid duration: {value}")
    return int(math.floor(value + 0.5))


def _json_text(value: Any) -> str:
    return json.dumps(value, ensure_ascii=False, indent=2) + "\n"


def _reviewed_closure_counts(
    promotions: list[dict[str, Any]], chains: list[dict[str, Any]]
) -> tuple[int, int]:
    """Return the closure reviewed by the manifest/debug exact-order join."""

    return len(promotions), sum(
        len(chain["animation"]["occurrences"]) for chain in chains
    )


def _load_clip_skills(path: Path) -> dict[str, int]:
    rows: dict[str, int] = {}
    pattern = re.compile(r'^"([^"]+)" skill=([0-9]+) ')
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except (OSError, UnicodeError) as exc:
        raise PromotionError(f"cannot read animation provenance {path}: {exc}") from exc
    for line in lines:
        match = pattern.match(line)
        if match is None:
            continue
        clip = match.group(1)
        skill = int(match.group(2))
        if clip in rows and rows[clip] != skill:
            raise PromotionError(f"clip has conflicting source skills: {clip}")
        rows[clip] = skill
    if not rows:
        raise PromotionError("Valtan.animnotify contains no clip skill rows")
    return rows


def _load_root_curves(repo_root: Path, model_path: Path) -> dict[str, tuple[float, float, list]]:
    root_text = str(repo_root)
    if root_text not in sys.path:
        sys.path.insert(0, root_text)
    try:
        from Tools.ValtanActionExtractor.build_valtan_rootmotion import read_root_curves
    except ImportError as exc:
        raise PromotionError(f"cannot import WModel duration reader: {exc}") from exc
    try:
        curves = read_root_curves(model_path)
    except (OSError, SystemExit, ValueError) as exc:
        raise PromotionError(f"cannot read Valtan WModel animation table: {exc}") from exc
    if not curves:
        raise PromotionError("Valtan WModel animation table is empty")
    return curves


def _validate_header(manifest: dict[str, Any], debug: dict[str, Any]) -> None:
    _exact(
        manifest,
        (
            "schema",
            "formatVersion",
            "bossArchetypeId",
            "encounterId",
            "sourceDocument",
            "presentationProfile",
            "clipAliases",
            "animationIntakeOnly",
            "patterns",
        ),
        "promotion manifest",
    )
    if (
        manifest["schema"] != "lostark.valtan-animation-chain-promotions"
        or manifest["formatVersion"] != 2
        or manifest["bossArchetypeId"] != "BOSS_VALTAN"
        or manifest["encounterId"] != "ENCOUNTER_VALTAN"
    ):
        raise PromotionError("promotion manifest header mismatch")
    _exact(
        debug,
        ("schema", "formatVersion", "bossArchetypeId", "encounterId", "chains"),
        "debug source",
    )
    if (
        debug["schema"] != "lostark.valtan-pattern-presentation-debug"
        or debug["formatVersion"] != 1
        or debug["bossArchetypeId"] != manifest["bossArchetypeId"]
        or debug["encounterId"] != manifest["encounterId"]
    ):
        raise PromotionError("debug source header mismatch")


def _validate_manifest_paths(repo_root: Path, manifest: dict[str, Any]) -> tuple[Path, Path]:
    source = manifest["sourceDocument"]
    _exact(source, ("path", "sha256"), "sourceDocument")
    if source["path"] != DEBUG_REL or not SHA256.fullmatch(source["sha256"]):
        raise PromotionError("sourceDocument identity is invalid")
    source_path = repo_root / source["path"]
    if _sha256(source_path) != source["sha256"]:
        raise PromotionError(
            "animation chain source changed; review its rows and refresh the manifest hash"
        )

    profile = manifest["presentationProfile"]
    _exact(
        profile,
        ("profileId", "modelAssetId", "modelPath", "modelSha256"),
        "presentationProfile",
    )
    _stable(profile["profileId"], "presentationProfile.profileId")
    model_asset = profile["modelAssetId"]
    if (
        not isinstance(model_asset, str)
        or "\\" in model_asset
        or model_asset.startswith("/")
        or ".." in model_asset.split("/")
        or not model_asset.startswith("Character/Valtan/")
        or not model_asset.endswith(".wmodel")
    ):
        raise PromotionError("presentationProfile.modelAssetId is outside Valtan Resources")
    model_relative = profile["modelPath"]
    if (
        not isinstance(model_relative, str)
        or "\\" in model_relative
        or model_relative.startswith("/")
        or ".." in model_relative.split("/")
        or not SHA256.fullmatch(profile["modelSha256"])
    ):
        raise PromotionError("presentationProfile model identity is invalid")
    model_path = repo_root / model_relative
    if _sha256(model_path) != profile["modelSha256"]:
        raise PromotionError("Valtan WModel changed; refresh durations and review before promotion")
    return source_path, model_path


def _manual_gameplay_pattern(
    promotion: dict[str, Any],
    source_action_ids: list[int],
    stages: list[dict[str, Any]],
) -> dict[str, Any]:
    chain_id = promotion["sourceChainId"]
    return {
        "patternId": promotion["patternId"],
        "displayName": promotion["displayName"],
        "category": "NORMAL",
        "compatibilitySelectionWeight": 0,
        "actionId": f"valtan.sequence.{chain_id}",
        "entryActionId": stages[0]["actionId"],
        "targetPolicy": promotion.get("targetPolicy", "NONE"),
        "aimPolicy": promotion.get("aimPolicy", "NONE"),
        "eligibility": {
            "armorRequirement": "ANY",
            "phaseRequirement": "ANY",
            "minimumGameplayPhase": 1,
            "maximumGameplayPhase": 3,
            "minimumHealthBarInclusive": 0,
            "maximumHealthBarInclusive": 0,
            "minimumRangeM": 0.0,
            "maximumRangeM": 1.0,
            "cooldownPolicy": "DERIVED_SOURCE_ACTION",
            "selectionCooldownMs": None,
            "cooldownGroupId": None,
            "repeatPolicy": {
                "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE",
                "limit": 0,
            },
        },
        "invulnerableWhileRunning": False,
        "sourceActionIds": source_action_ids,
        "serverMotion": None,
        "reactions": [],
        "stages": [
            {
                "stageId": stage["stageId"],
                "actionId": stage["actionId"],
                "stageKind": "ACTIVE",
                "durationMs": stage["durationMs"],
                "defaultNextActionId": (
                    stages[index + 1]["actionId"]
                    if index + 1 < len(stages)
                    else None
                ),
                "hit": {"shape": {"kind": "NONE"}},
                "motion": None,
                "events": [],
                "branches": [],
            }
            for index, stage in enumerate(stages)
        ],
    }


def _manual_presentation_pattern(
    promotion: dict[str, Any],
    source_action_ids: list[int],
    stages: list[dict[str, Any]],
) -> dict[str, Any]:
    sources = [
        {
            "sourceActionId": source_action_id,
            "sequenceIndex": 1,
            "role": "PRIMARY" if index == 0 else "REFERENCE",
        }
        for index, source_action_id in enumerate(source_action_ids)
    ]
    return {
        "patternId": promotion["patternId"],
        "sourceSequenceIndex": 1,
        "presentationSources": sources,
        "stages": [
            {
                "stageId": stage["stageId"],
                "actionId": stage["actionId"],
                "sequenceRole": "STEP",
                "animation": {
                    "endPolicy": stage["endPolicy"],
                    "repeatCount": 1,
                    "occurrences": [stage["occurrence"]],
                },
                "effectCues": [],
                "cameraInvocations": [],
            }
            for stage in stages
        ],
    }


def _expand_trash_capture_promotion(
    generated_gameplay: dict[str, Any],
    generated_presentation: dict[str, Any],
    existing_gameplay: dict[str, Any] | None,
    existing_presentation: dict[str, Any] | None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    """Join the reviewed eight-clip intake to the authored capture graph.

    The six outcome stages belong to the phase-two author, not the animation
    intake. Reuse that author before the ordinary enrichment identity checks;
    do not accept arbitrary extra stages or quietly repair their fixed clocks.
    """
    # The author imports this module's readers, so defer the reverse dependency.
    from author_valtan_phase_two_mechanics import author_trash_capture_flow

    intake_ids = [f"STEP_{index:02d}" for index in range(1, 9)]
    for generated in (generated_gameplay, generated_presentation):
        if generated.get("patternId") != "VALTAN_TRASH" or [
            row.get("stageId") for row in generated.get("stages", [])
        ] != intake_ids:
            raise PromotionError("Trash capture promotion requires the reviewed eight-stage intake")
    if (existing_gameplay is None) != (existing_presentation is None):
        raise PromotionError("Trash capture promotion requires paired split source rows")

    # Keep the receipt's sourceActionIds/occurrences limited to the intake.
    gameplay = copy.deepcopy(generated_gameplay)
    presentation = copy.deepcopy(generated_presentation)
    author_trash_capture_flow({"patterns": [gameplay]}, {"patterns": [presentation]})
    expected_ids = [row["stageId"] for row in gameplay["stages"]]
    for domain, generated, existing in (
        ("gameplay", gameplay, existing_gameplay),
        ("presentation", presentation, existing_presentation),
    ):
        if existing is None:
            continue
        rows = existing.get("stages")
        if not isinstance(rows, list) or any(not isinstance(row, dict) for row in rows) or [
            row.get("stageId") for row in rows
        ] != expected_ids:
            raise PromotionError(f"Trash capture {domain} stage closure/order drift")
        for expected, current in zip(generated["stages"], rows):
            stage_id = expected["stageId"]
            if current.get("actionId") != expected["actionId"]:
                raise PromotionError(f"Trash capture {domain} action identity drift: {stage_id}")
            if stage_id in intake_ids:
                continue
            if domain == "gameplay":
                duration = _integer(current.get("durationMs"), f"Trash/{stage_id}.durationMs", 1)
                if duration != expected["durationMs"]:
                    raise PromotionError(f"Trash capture branch duration drift: {stage_id}")
                if "counterProxy" in expected and "counterProxy" not in current:
                    raise PromotionError(f"Trash capture counter proxy is missing: {stage_id}")
            elif current.get("animation") != expected["animation"]:
                raise PromotionError(f"Trash capture branch source slice drift: {stage_id}")
    return gameplay, presentation


def _preserve_manual_gameplay_enrichment(
    generated: dict[str, Any], existing: dict[str, Any] | None
) -> dict[str, Any]:
    """Keep reviewed gameplay semantics while rebuilding animation lineage.

    The promotion tool owns clip identity and stage walls. Pattern authors own
    target/aim, hit, motion, event and branch semantics after that promotion.
    Re-running the animation resolver must not silently turn a working Server
    pattern back into an animation-only audition.
    """

    if existing is None:
        return generated
    generated_stages = {
        stage["stageId"]: stage for stage in generated["stages"]
    }
    existing_stages = {
        stage.get("stageId"): stage
        for stage in existing.get("stages", [])
        if isinstance(stage, dict)
    }
    if set(generated_stages) != set(existing_stages):
        raise PromotionError(
            f"manual gameplay enrichment stage closure drift: {generated['patternId']}"
        )
    for field in (
        "targetPolicy",
        "aimPolicy",
        "eligibility",
        "invulnerableWhileRunning",
        "serverMotion",
        "reactions",
    ):
        if field in existing:
            generated[field] = copy.deepcopy(existing[field])
    generated_action_ids = {
        stage["actionId"] for stage in generated_stages.values()
    }
    for stage_id, generated_stage in generated_stages.items():
        existing_stage = existing_stages[stage_id]
        if existing_stage.get("actionId") != generated_stage["actionId"]:
            raise PromotionError(
                "manual gameplay enrichment action identity drift: "
                f"{generated['patternId']}/{stage_id}"
            )
        for field in (
            "stageKind",
            "defaultNextActionId",
            "hit",
            "motion",
            "events",
            "branches",
            "counterProxy",
        ):
            if field in existing_stage:
                generated_stage[field] = copy.deepcopy(existing_stage[field])
        next_action = generated_stage["defaultNextActionId"]
        if next_action is not None and next_action not in generated_action_ids:
            raise PromotionError(
                "manual gameplay enrichment default edge leaves its pattern: "
                f"{generated['patternId']}/{stage_id}"
            )
        for branch in generated_stage["branches"]:
            next_action = branch.get("nextActionId")
            if next_action is not None and next_action not in generated_action_ids:
                raise PromotionError(
                    "manual gameplay enrichment branch leaves its pattern: "
                    f"{generated['patternId']}/{stage_id}"
                )
    return generated


def _preserve_manual_presentation_enrichment(
    generated: dict[str, Any], existing: dict[str, Any] | None
) -> dict[str, Any]:
    """Keep exact-occurrence Effect/Camera joins across animation refreshes."""

    if existing is None:
        return generated
    generated_stages = {
        stage["stageId"]: stage for stage in generated["stages"]
    }
    existing_stages = {
        stage.get("stageId"): stage
        for stage in existing.get("stages", [])
        if isinstance(stage, dict)
    }
    if set(generated_stages) != set(existing_stages):
        raise PromotionError(
            f"manual presentation enrichment stage closure drift: {generated['patternId']}"
        )
    for stage_id, generated_stage in generated_stages.items():
        existing_stage = existing_stages[stage_id]
        if existing_stage.get("actionId") != generated_stage["actionId"]:
            raise PromotionError(
                "manual presentation enrichment action identity drift: "
                f"{generated['patternId']}/{stage_id}"
            )
        for field in ("sequenceRole", "effectCues", "cameraInvocations"):
            if field in existing_stage:
                generated_stage[field] = copy.deepcopy(existing_stage[field])
    return generated


def build_candidates(repo_root: Path) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    manifest_path = repo_root / MANIFEST_REL
    manifest = _read_json(manifest_path)
    source_path = repo_root / DEBUG_REL
    debug = _read_json(source_path)
    _validate_header(manifest, debug)
    checked_source_path, model_path = _validate_manifest_paths(repo_root, manifest)
    if checked_source_path.resolve() != source_path.resolve():
        raise PromotionError("manifest source path does not resolve to the debug document")

    aliases = manifest["clipAliases"]
    if not isinstance(aliases, dict):
        raise PromotionError("clipAliases must be an object")
    for source_clip, target_clip in aliases.items():
        _stable(source_clip, "clipAliases source")
        _stable(target_clip, f"clipAliases[{source_clip}]")

    intake_only = manifest["animationIntakeOnly"]
    promotions = manifest["patterns"]
    chains = debug["chains"]
    if (
        not isinstance(intake_only, list)
        or not isinstance(promotions, list)
        or not isinstance(chains, list)
        or not promotions
        or not chains
    ):
        raise PromotionError(
            "manifest promotions, animation intake, and debug chains must be arrays"
        )
    promotion_ids: set[str] = set()
    pattern_ids: set[str] = set()
    for ordinal, promotion in enumerate(promotions):
        _required_with_optional(
            promotion,
            (
                "sourceChainId",
                "patternId",
                "displayName",
                "authoringPhase",
                "admissionState",
            ),
            ("targetPolicy", "aimPolicy"),
            f"promotion[{ordinal}]",
        )
        chain_id = _stable(promotion["sourceChainId"], f"promotion[{ordinal}].sourceChainId")
        pattern_id = _stable(promotion["patternId"], f"promotion[{ordinal}].patternId")
        if chain_id in promotion_ids or pattern_id in pattern_ids:
            raise PromotionError(f"duplicate promotion identity: {chain_id}/{pattern_id}")
        promotion_ids.add(chain_id)
        pattern_ids.add(pattern_id)
        if (
            not isinstance(promotion["displayName"], str)
            or not promotion["displayName"].strip()
            or len(promotion["displayName"]) > 64
            or _integer(promotion["authoringPhase"], "authoringPhase", 1) > 3
            or promotion["admissionState"] != "MANUAL_SERVER_AUDITION"
        ):
            raise PromotionError(f"promotion metadata is invalid: {chain_id}")
        target_policy = promotion.get("targetPolicy", "NONE")
        aim_policy = promotion.get("aimPolicy", "NONE")
        if target_policy not in {
            "NONE",
            "LOCK_NEAREST_ON_START",
            "LOCK_RANDOM_ALIVE_ON_START",
        } or aim_policy not in {
            "NONE",
            "LOCK_FACING_ON_START",
        }:
            raise PromotionError(f"promotion targeting is invalid: {chain_id}")
        if (target_policy == "NONE") != (aim_policy == "NONE"):
            raise PromotionError(
                f"promotion target and aim policies must be authored together: {chain_id}"
            )

    intake_chain_ids: list[str] = []
    for ordinal, row in enumerate(intake_only):
        _exact(
            row,
            ("sourceChainId", "displayName", "authoringPhase", "admissionState"),
            f"animationIntakeOnly[{ordinal}]",
        )
        chain_id = _stable(
            row["sourceChainId"],
            f"animationIntakeOnly[{ordinal}].sourceChainId",
        )
        if chain_id in promotion_ids or chain_id in intake_chain_ids:
            raise PromotionError(f"duplicate animation intake identity: {chain_id}")
        if (
            not isinstance(row["displayName"], str)
            or not row["displayName"].strip()
            or len(row["displayName"]) > 64
            or _integer(row["authoringPhase"], "authoringPhase", 1) > 3
            or row["admissionState"] != ANIMATION_INTAKE_ONLY
        ):
            raise PromotionError(f"animation intake metadata is invalid: {chain_id}")
        intake_chain_ids.append(chain_id)

    chain_ids = [
        _stable(chain.get("chainId"), f"debug chain[{ordinal}].chainId")
        for ordinal, chain in enumerate(chains)
        if isinstance(chain, dict)
    ]
    if len(chain_ids) != len(chains) or len(chain_ids) != len(set(chain_ids)):
        raise PromotionError("debug chain IDs are missing or duplicated")
    promoted_chain_ids = [row["sourceChainId"] for row in promotions]
    declared_chain_ids = promoted_chain_ids + intake_chain_ids
    if (
        set(declared_chain_ids) != set(chain_ids)
        or promoted_chain_ids != [value for value in chain_ids if value in promotion_ids]
        or intake_chain_ids != [value for value in chain_ids if value in intake_chain_ids]
    ):
        raise PromotionError(
            "promotion plus intake-only manifest order/closure must exactly match debug chains"
        )
    # Retiring a promoted pattern must not reorder the user's animation intake.
    # Each admission partition keeps its original relative order instead.
    chains_by_id = {chain["chainId"]: chain for chain in chains}
    promoted_chains = [chains_by_id[value] for value in promoted_chain_ids]

    curves = _load_root_curves(repo_root, model_path)
    clip_skills = _load_clip_skills(repo_root / ANIM_NOTIFY_REL)

    intake_occurrence_ids: set[str] = set()
    used_aliases: set[str] = set()
    for declaration in intake_only:
        chain_id = declaration["sourceChainId"]
        chain = chains_by_id[chain_id]
        _exact(
            chain,
            ("chainId", "targetPatternId", "targetStageId", "animation"),
            f"intake-only chain {chain_id}",
        )
        if chain["chainId"] != chain_id or chain["targetPatternId"] or chain["targetStageId"]:
            raise PromotionError(
                f"intake-only debug chain target fields must remain empty: {chain_id}"
            )
        animation = chain["animation"]
        _exact(
            animation,
            ("endPolicy", "repeatCount", "occurrences"),
            f"intake-only chain {chain_id}.animation",
        )
        occurrences = animation["occurrences"]
        if (
            animation["endPolicy"] != "NATIVE_CLIP_LENGTHS"
            or not isinstance(occurrences, list)
            or not occurrences
            or _integer(animation["repeatCount"], f"chain {chain_id}.repeatCount", 1)
            != len(occurrences)
            or len(occurrences) > 64
        ):
            raise PromotionError(f"intake-only animation header is invalid: {chain_id}")
        for ordinal, source in enumerate(occurrences, start=1):
            _exact(
                source,
                (
                    "clipOccurrenceId",
                    "clip",
                    "mappingBasis",
                    "sourceStartMs",
                    "playMs",
                    "playRate",
                    "repeatUntilStageEnd",
                ),
                f"intake-only chain {chain_id} occurrence[{ordinal}]",
            )
            occurrence_id = _stable(
                source["clipOccurrenceId"],
                f"intake-only chain {chain_id} source occurrence",
            )
            if occurrence_id in intake_occurrence_ids:
                raise PromotionError(f"duplicate intake-only occurrence: {occurrence_id}")
            intake_occurrence_ids.add(occurrence_id)
            source_clip = _stable(
                source["clip"], f"intake-only chain {chain_id} source clip"
            )
            resolved_clip = aliases.get(source_clip, source_clip)
            if resolved_clip != source_clip:
                used_aliases.add(source_clip)
            curve = curves.get(resolved_clip)
            if curve is None:
                raise PromotionError(
                    f"intake-only clip is absent from the reviewed WModel: {source_clip}"
                )
            if source["mappingBasis"] != "PROJECT_AUTHORED":
                raise PromotionError(
                    f"intake-only clip must remain PROJECT_AUTHORED: {occurrence_id}"
                )
            source_start_ms = _integer(
                source["sourceStartMs"], f"{occurrence_id}.sourceStartMs"
            )
            _integer(source["playMs"], f"{occurrence_id}.playMs")
            _number(source["playRate"], f"{occurrence_id}.playRate", 0.000001)
            if not isinstance(source["repeatUntilStageEnd"], bool):
                raise PromotionError(
                    f"{occurrence_id}.repeatUntilStageEnd must be Boolean"
                )
            native_duration_ms, _ticks_per_second, _keys = curve
            if source_start_ms >= native_duration_ms:
                raise PromotionError(
                    f"intake-only sourceStartMs escapes native clip: {occurrence_id}"
                )

    gameplay = _read_json(repo_root / GAMEPLAY_REL)
    presentation = _read_json(repo_root / PRESENTATION_REL)
    retired_collisions = pattern_ids & set(gameplay.get("retiredPatternIds", []))
    if retired_collisions:
        raise PromotionError(
            f"retired patterns cannot be promoted: {sorted(retired_collisions)}"
        )

    current_gameplay_ids = {row.get("patternId") for row in gameplay.get("patterns", [])}
    current_presentation_ids = {
        row.get("patternId") for row in presentation.get("patterns", [])
    }
    live_collisions = pattern_ids & (
        current_gameplay_ids ^ current_presentation_ids
    )
    if live_collisions:
        raise PromotionError(f"split source contains partial promotion rows: {sorted(live_collisions)}")

    existing_manual = gameplay.get("decisionModel", {}).get("manualAuditions", [])
    existing_manual_by_id = {
        row.get("patternId"): row for row in existing_manual if isinstance(row, dict)
    }
    existing_gameplay_by_id = {
        row.get("patternId"): row
        for row in gameplay.get("patterns", [])
        if isinstance(row, dict) and row.get("patternId") in pattern_ids
    }
    existing_presentation_by_id = {
        row.get("patternId"): row
        for row in presentation.get("patterns", [])
        if isinstance(row, dict) and row.get("patternId") in pattern_ids
    }
    for promotion in promotions:
        pattern_id = promotion["patternId"]
        if pattern_id in current_gameplay_ids:
            owner = existing_manual_by_id.get(pattern_id)
            if owner is None or owner.get("sourceChainId") != promotion["sourceChainId"]:
                raise PromotionError(f"pattern ID collides with a non-matching owner: {pattern_id}")

    gameplay["patterns"] = [
        row for row in gameplay["patterns"] if row.get("patternId") not in pattern_ids
    ]
    presentation["patterns"] = [
        row for row in presentation["patterns"] if row.get("patternId") not in pattern_ids
    ]
    if "decisionModel" not in gameplay or not isinstance(gameplay["decisionModel"], dict):
        raise PromotionError("gameplay decisionModel is missing")
    gameplay["decisionModel"]["manualAuditions"] = [
        row for row in existing_manual if row.get("patternId") not in pattern_ids
    ]

    receipt_patterns: list[dict[str, Any]] = []
    seen_source_occurrences: set[str] = set()
    seen_target_actions: set[str] = set()
    seen_target_occurrences: set[str] = set()
    for promotion, chain in zip(promotions, promoted_chains):
        chain_id = promotion["sourceChainId"]
        _exact(
            chain,
            ("chainId", "targetPatternId", "targetStageId", "animation"),
            f"chain {chain_id}",
        )
        if chain["chainId"] != chain_id or chain["targetPatternId"] or chain["targetStageId"]:
            raise PromotionError(f"debug chain target fields must remain empty: {chain_id}")
        animation = chain["animation"]
        _exact(animation, ("endPolicy", "repeatCount", "occurrences"), f"chain {chain_id}.animation")
        occurrences = animation["occurrences"]
        if (
            animation["endPolicy"] != "NATIVE_CLIP_LENGTHS"
            or not isinstance(occurrences, list)
            or not occurrences
            or _integer(animation["repeatCount"], f"chain {chain_id}.repeatCount", 1)
            != len(occurrences)
            or len(occurrences) > 64
        ):
            raise PromotionError(f"debug chain animation header is invalid: {chain_id}")

        stages: list[dict[str, Any]] = []
        source_action_ids: list[int] = []
        receipt_occurrences: list[dict[str, Any]] = []
        for occurrence_ordinal, source in enumerate(occurrences, start=1):
            _exact(
                source,
                (
                    "clipOccurrenceId",
                    "clip",
                    "mappingBasis",
                    "sourceStartMs",
                    "playMs",
                    "playRate",
                    "repeatUntilStageEnd",
                ),
                f"chain {chain_id} occurrence[{occurrence_ordinal}]",
            )
            source_occurrence_id = _stable(
                source["clipOccurrenceId"],
                f"chain {chain_id} source occurrence",
            )
            if source_occurrence_id in seen_source_occurrences:
                raise PromotionError(f"duplicate source occurrence: {source_occurrence_id}")
            seen_source_occurrences.add(source_occurrence_id)
            source_clip = _stable(source["clip"], f"chain {chain_id} source clip")
            resolved_clip = aliases.get(source_clip, source_clip)
            if source_clip in aliases:
                used_aliases.add(source_clip)
                if source_clip in curves:
                    raise PromotionError(f"unnecessary clip alias shadows a real clip: {source_clip}")
            curve = curves.get(resolved_clip)
            if curve is None:
                raise PromotionError(f"clip is absent from the reviewed WModel: {source_clip}")
            source_action_id = clip_skills.get(resolved_clip)
            if source_action_id is None:
                raise PromotionError(f"clip has no Valtan.animnotify source action: {resolved_clip}")
            if source_action_id not in source_action_ids:
                source_action_ids.append(source_action_id)

            source_start_ms = _integer(
                source["sourceStartMs"], f"{source_occurrence_id}.sourceStartMs"
            )
            authored_wall_ms = _integer(source["playMs"], f"{source_occurrence_id}.playMs")
            play_rate = _number(source["playRate"], f"{source_occurrence_id}.playRate", 0.000001)
            if not isinstance(source["repeatUntilStageEnd"], bool):
                raise PromotionError(f"{source_occurrence_id}.repeatUntilStageEnd must be Boolean")
            native_duration_ms, ticks_per_second, _keys = curve
            available_source_ms = native_duration_ms - source_start_ms
            if available_source_ms <= 0.0:
                raise PromotionError(f"sourceStartMs escapes native clip: {source_occurrence_id}")
            native_source_ms = _lround_positive(available_source_ms)
            native_wall_ms = _lround_positive(native_source_ms / play_rate)
            if native_source_ms <= 0 or native_wall_ms <= 0:
                raise PromotionError(f"resolved native duration is empty: {source_occurrence_id}")

            if authored_wall_ms == 0:
                if source["repeatUntilStageEnd"]:
                    raise PromotionError(
                        f"native occurrence cannot request an unbounded loop: {source_occurrence_id}"
                    )
                duration_ms = native_wall_ms
                product_play_ms = native_source_ms
                end_policy = "EXACT"
                repeat_until_stage_end = False
                resolution = "NATIVE_WMODEL"
            elif source["repeatUntilStageEnd"] or authored_wall_ms > native_wall_ms:
                duration_ms = authored_wall_ms
                product_play_ms = 0
                end_policy = "LOOP_TO_STAGE_END"
                repeat_until_stage_end = True
                resolution = "EXPLICIT_WALL_LOOP"
            else:
                duration_ms = authored_wall_ms
                product_play_ms = _lround_positive(authored_wall_ms * play_rate)
                if product_play_ms <= 0 or product_play_ms > native_source_ms:
                    raise PromotionError(f"explicit source slice is invalid: {source_occurrence_id}")
                end_policy = "EXACT"
                repeat_until_stage_end = False
                resolution = "EXPLICIT_WALL_EXACT"

            stage_id = f"STEP_{occurrence_ordinal:02d}"
            action_id = f"valtan.sequence.{chain_id}.step-{occurrence_ordinal:02d}"
            target_occurrence_id = f"{action_id}.clip-01"
            _stable(action_id, f"chain {chain_id} actionId")
            _stable(target_occurrence_id, f"chain {chain_id} target occurrence")
            if action_id in seen_target_actions or target_occurrence_id in seen_target_occurrences:
                raise PromotionError(f"generated identity collided: {action_id}")
            seen_target_actions.add(action_id)
            seen_target_occurrences.add(target_occurrence_id)
            target_occurrence = {
                "clipOccurrenceId": target_occurrence_id,
                "clip": resolved_clip,
                "mappingBasis": "PROJECT_AUTHORED",
                "sourceStartMs": source_start_ms,
                "playMs": product_play_ms,
                "playRate": play_rate,
                "repeatUntilStageEnd": repeat_until_stage_end,
            }
            stages.append(
                {
                    "stageId": stage_id,
                    "actionId": action_id,
                    "durationMs": duration_ms,
                    "endPolicy": end_policy,
                    "occurrence": target_occurrence,
                }
            )
            receipt_occurrences.append(
                {
                    "sourceClipOccurrenceId": source_occurrence_id,
                    "targetStageId": stage_id,
                    "targetActionId": action_id,
                    "targetClipOccurrenceId": target_occurrence_id,
                    "sourceClip": source_clip,
                    "resolvedClip": resolved_clip,
                    "aliasApplied": source_clip != resolved_clip,
                    "sourceActionId": source_action_id,
                    "sourceStartMs": source_start_ms,
                    "authoredWallMs": authored_wall_ms,
                    "playRate": play_rate,
                    "nativeDurationTicks": round(
                        native_duration_ms * ticks_per_second / 1000.0, 6
                    ),
                    "ticksPerSecond": ticks_per_second,
                    "nativeSourceMs": native_source_ms,
                    "stageDurationMs": duration_ms,
                    "productPlayMs": product_play_ms,
                    "endPolicy": end_policy,
                    "resolution": resolution,
                }
            )

        gameplay_pattern = _manual_gameplay_pattern(
            promotion, source_action_ids, stages
        )
        presentation_pattern = _manual_presentation_pattern(
            promotion, source_action_ids, stages
        )
        if promotion["patternId"] == "VALTAN_TRASH":
            gameplay_pattern, presentation_pattern = _expand_trash_capture_promotion(
                gameplay_pattern,
                presentation_pattern,
                existing_gameplay_by_id.get(promotion["patternId"]),
                existing_presentation_by_id.get(promotion["patternId"]),
            )
        gameplay_pattern = _preserve_manual_gameplay_enrichment(
            gameplay_pattern,
            existing_gameplay_by_id.get(promotion["patternId"]),
        )
        presentation_pattern = _preserve_manual_presentation_enrichment(
            presentation_pattern,
            existing_presentation_by_id.get(promotion["patternId"]),
        )
        gameplay["patterns"].append(gameplay_pattern)
        presentation["patterns"].append(presentation_pattern)
        gameplay["decisionModel"]["manualAuditions"].append(
            {
                "patternId": promotion["patternId"],
                "sourceChainId": chain_id,
                "authoringPhase": promotion["authoringPhase"],
                "admissionState": promotion["admissionState"],
            }
        )
        receipt_patterns.append(
            {
                "sourceChainId": chain_id,
                "patternId": promotion["patternId"],
                "authoringPhase": promotion["authoringPhase"],
                "admissionState": promotion["admissionState"],
                "sourceActionIds": source_action_ids,
                "stageCount": len(stages),
                "occurrences": receipt_occurrences,
            }
        )

    if used_aliases != set(aliases):
        raise PromotionError(
            f"clip alias coverage drift: used={sorted(used_aliases)} declared={sorted(aliases)}"
        )
    reviewed_pattern_count, reviewed_stage_count = _reviewed_closure_counts(
        promotions, promoted_chains
    )
    generated_stage_count = sum(row["stageCount"] for row in receipt_patterns)
    if (
        len(receipt_patterns) != reviewed_pattern_count
        or generated_stage_count != reviewed_stage_count
    ):
        raise PromotionError(
            "generated promotion closure left the reviewed manifest/debug closure"
        )
    receipt = {
        "schema": "lostark.valtan-animation-chain-promotion-receipt",
        "formatVersion": 1,
        "bossArchetypeId": manifest["bossArchetypeId"],
        "encounterId": manifest["encounterId"],
        "resolverVersion": 1,
        "roundingPolicy": "POSITIVE_HALF_AWAY_FROM_ZERO",
        "sourceDocument": copy.deepcopy(manifest["sourceDocument"]),
        "presentationProfile": copy.deepcopy(manifest["presentationProfile"]),
        "patternCount": reviewed_pattern_count,
        "stageCount": reviewed_stage_count,
        "patterns": receipt_patterns,
    }
    return gameplay, presentation, receipt


def validate_and_project(
    repo_root: Path,
    gameplay: dict[str, Any],
    presentation: dict[str, Any],
) -> dict[str, str]:
    root_text = str(repo_root)
    if root_text not in sys.path:
        sys.path.insert(0, root_text)
    try:
        from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline
    except ImportError as exc:
        raise PromotionError(f"cannot import Valtan V2 pipeline: {exc}") from exc

    try:
        docs = pipeline.load_pipeline_documents(repo_root)
        docs[pipeline.GAMEPLAY_AUTHORING_REL] = gameplay
        docs[pipeline.PRESENTATION_AUTHORING_REL] = presentation
        joined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            docs[pipeline.WORLD_SET_REL],
            docs[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_legacy_manifest(
            docs[pipeline.LEGACY_REL],
            {row["patternId"] for row in joined["patterns"]},
        )
        outputs = pipeline.project_v2_products(repo_root, docs, joined)
        balance_outputs = pipeline.project_balance_products(
            repo_root,
            docs[pipeline.BOSS_PROFILES_REL],
            docs[pipeline.DAMAGE_REL],
        )
        outputs[pipeline.PROVENANCE_REL] = pipeline.project_provenance_receipt(
            repo_root, {**outputs, **balance_outputs}
        )
    except (KeyError, TypeError, ValueError, pipeline.PipelineError) as exc:
        raise PromotionError(f"promoted split/Product validation failed: {exc}") from exc
    for relative, text in outputs.items():
        try:
            json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
        except (json.JSONDecodeError, PromotionError) as exc:
            raise PromotionError(f"projected Product is not strict JSON: {relative}: {exc}") from exc
    return outputs


def _atomic_commit(
    targets: Mapping[Path, bytes],
    *,
    inject_failure_after: int | None = None,
) -> None:
    baselines: dict[Path, bytes | None] = {
        path: path.read_bytes() if path.exists() else None for path in targets
    }
    staged: dict[Path, Path] = {}
    committed: list[Path] = []
    try:
        for path, payload in targets.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            handle, temporary_name = tempfile.mkstemp(
                prefix=f".{path.name}.promotion-", suffix=".tmp", dir=path.parent
            )
            temporary = Path(temporary_name)
            staged[path] = temporary
            try:
                with os.fdopen(handle, "wb") as output:
                    output.write(payload)
                    output.flush()
                    os.fsync(output.fileno())
            except BaseException:
                temporary.unlink(missing_ok=True)
                raise

        for path, baseline in baselines.items():
            current = path.read_bytes() if path.exists() else None
            if current != baseline:
                raise PromotionError(f"target changed during promotion: {path}")

        for path, temporary in staged.items():
            os.replace(temporary, path)
            committed.append(path)
            if inject_failure_after is not None and len(committed) >= inject_failure_after:
                raise PromotionError("injected promotion commit failure")
    except BaseException as exc:
        rollback_failures: list[str] = []
        for path in reversed(committed):
            baseline = baselines[path]
            try:
                if baseline is None:
                    path.unlink(missing_ok=True)
                else:
                    handle, rollback_name = tempfile.mkstemp(
                        prefix=f".{path.name}.rollback-", suffix=".tmp", dir=path.parent
                    )
                    rollback = Path(rollback_name)
                    with os.fdopen(handle, "wb") as output:
                        output.write(baseline)
                        output.flush()
                        os.fsync(output.fileno())
                    os.replace(rollback, path)
            except BaseException as rollback_exc:
                rollback_failures.append(f"{path}: {rollback_exc}")
        if rollback_failures:
            raise PromotionError(
                f"promotion failed ({exc}); rollback also failed: {rollback_failures}"
            ) from exc
        raise
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def run(
    repo_root: Path,
    mode: str,
    *,
    inject_failure_after: int | None = None,
) -> dict[str, Any]:
    repo_root = repo_root.resolve()
    gameplay, presentation, receipt = build_candidates(repo_root)
    outputs = validate_and_project(repo_root, gameplay, presentation)
    if mode == "Apply":
        targets: dict[Path, bytes] = {
            repo_root / GAMEPLAY_REL: _json_text(gameplay).encode("utf-8"),
            repo_root / PRESENTATION_REL: _json_text(presentation).encode("utf-8"),
            repo_root / RECEIPT_REL: _json_text(receipt).encode("utf-8"),
        }
        for relative, text in outputs.items():
            targets[repo_root / relative] = text.encode("utf-8")
        _atomic_commit(targets, inject_failure_after=inject_failure_after)
    return {
        "mode": mode,
        "patternCount": receipt["patternCount"],
        "stageCount": receipt["stageCount"],
        "sourceOccurrenceCount": sum(
            len(pattern["occurrences"]) for pattern in receipt["patterns"]
        ),
        "projectedArtifactCount": len(outputs),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
    )
    parser.add_argument("--mode", choices=("Validate", "Apply"), default="Validate")
    parser.add_argument("--inject-failure-after", type=int, default=None, help=argparse.SUPPRESS)
    arguments = parser.parse_args()
    try:
        result = run(
            arguments.repo_root,
            arguments.mode,
            inject_failure_after=arguments.inject_failure_after,
        )
    except PromotionError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    print(json.dumps(result, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
