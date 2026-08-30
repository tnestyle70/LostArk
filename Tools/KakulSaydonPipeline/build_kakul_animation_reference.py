#!/usr/bin/env python3
"""Build reference-only KakulSaydon action/animation documents.

The extracted Action reports are authoring evidence, not Server Product data.
This generator joins their explicit animation clip occurrences to the clips
that physically exist in the admitted WModel.  Missing clips remain visible as
holdouts and never become authored overrides.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
import math
from pathlib import Path, PurePosixPath, PureWindowsPath
import re
import subprocess
import sys
from typing import Any, Iterable


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE_ROOT = Path(
    "C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/"
    "RemainingCharacterExtraction-20260829/ActionNameSources"
)
DEFAULT_RUNTIME_ROOT = (
    REPO_ROOT / "Client/Bin/Resources/Character/KoukuSaton"
)
DEFAULT_OUTPUT_ROOT = REPO_ROOT / "Data/Animation"
DEFAULT_MODEL_INFO_TOOL = (
    REPO_ROOT / "Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe"
)

REFERENCE_SCHEMA = "lostark.kakul-animation-action-reference"
AUTHORED_SCHEMA = "lostark.kakul-animation-action-bindings"
FORMAT_VERSION = 1
AUTHORITY = "REFERENCE_ONLY"
REFERENCE_BASIS = "EXTRACTED_REFERENCE"
AUTHORED_BASIS = "PROJECT_AUTHORED"
REVIEW_CANDIDATE = "REVIEW_CANDIDATE"
HOLDOUT = "HOLDOUT"

SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
STAGE_ID_RE = re.compile(r"^stage-[0-9]{3}$")
SLOT_ID_RE = re.compile(r"^animation-[0-9]{3}$")
RUNTIME_CLIP_RE = re.compile(r"^[A-Za-z0-9_.\-']{1,128}$")
MODEL_SECTION_RE = re.compile(r"section type=4 .* name=(\S+)\s*$")
MODEL_SUMMARY_RE = re.compile(r"\[Model\].*animations=([0-9]+)")
HANGUL_RE = re.compile(r"[\uac00-\ud7a3]")
MAX_TIMELINE_MS = 600_000
MIN_PLAY_RATE = 0.01
MAX_PLAY_RATE = 16.0

COMMON_DISPLAY_NAMES = {
    "STAND": "대기",
    "MOVE": "이동",
    "FALLING": "낙하",
    "LAND": "착지",
    "BEHIT": "피격",
    "DIE": "사망",
    "SPAWN": "생성",
    "RESURRECT": "부활",
    "DESPAWN": "소멸",
}

REFERENCE_ROOT_KEYS = {
    "schema",
    "formatVersion",
    "authority",
    "profileId",
    "modelAssetId",
    "sourceEvidenceSha256",
    "referenceRevision",
    "actions",
}
ACTION_KEYS = {
    "sourceActionId",
    "displayName",
    "reviewStatus",
    "authority",
    "stages",
}
STAGE_KEYS = {"stageId", "stageOrdinal", "holdoutClipNames", "slots"}
SLOT_KEYS = {
    "slotId",
    "extractedClip",
    "runtimeClip",
    "sourceStartMs",
    "playMs",
    "playRate",
    "loop",
    "mappingBasis",
    "authority",
}
AUTHORED_ROOT_KEYS = {
    "schema",
    "formatVersion",
    "authority",
    "profileId",
    "referenceRevision",
    "bindings",
}
BINDING_KEYS = {
    "sourceActionId",
    "stageId",
    "slotId",
    "runtimeClip",
    "sourceStartMs",
    "playMs",
    "playRate",
    "loop",
    "mappingBasis",
    "authority",
}


class BuildError(ValueError):
    """Raised when extracted evidence cannot be joined without guessing."""


@dataclass(frozen=True)
class ProfileSpec:
    profile_id: str
    physical_model: str
    model_asset_id: str
    runtime_prefix: str | None = None
    uses_rpct05_retime: bool = False
    uses_rpcz_structure: bool = False


PROFILE_SPECS = (
    ProfileSpec(
        "MN_RPCT_05",
        "MN_RPCT_05",
        "Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05",
        uses_rpct05_retime=True,
    ),
    ProfileSpec(
        "MN_RPCT_06",
        "MN_RPCT_06",
        "Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06",
        runtime_prefix="mn_rpct_06_sk.ao_",
    ),
    ProfileSpec(
        "MN_RPCT_07",
        "MN_RPCT_05",
        "Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05",
        uses_rpct05_retime=True,
    ),
    ProfileSpec(
        "MN_RPCZ_00",
        "MN_RPCZ_00",
        "Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00",
        runtime_prefix="rpcz00_",
        uses_rpcz_structure=True,
    ),
)


def _read_json(path: Path) -> Any:
    if not path.is_file():
        raise BuildError(f"required JSON does not exist: {path}")
    try:
        return json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise BuildError(f"failed to read JSON {path}: {error}") from error


def _file_sha256(path: Path) -> str:
    if not path.is_file():
        raise BuildError(f"required evidence does not exist: {path}")
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(block)
    except OSError as error:
        raise BuildError(f"failed to hash evidence {path}: {error}") from error
    return digest.hexdigest()


def aggregate_evidence_sha256(rows: Iterable[tuple[str, str]]) -> str:
    normalized = sorted(rows, key=lambda row: row[0])
    if not normalized:
        raise BuildError("source evidence set must not be empty")
    payload = "".join(f"{label}\t{sha}\n" for label, sha in normalized)
    if any(not label or not SHA256_RE.fullmatch(sha) for label, sha in normalized):
        raise BuildError("source evidence row is invalid")
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


def parse_model_info(text: str) -> dict[str, str]:
    declared_count: int | None = None
    clips: dict[str, str] = {}
    for line in text.splitlines():
        summary = MODEL_SUMMARY_RE.search(line)
        if summary:
            declared_count = int(summary.group(1))
        match = MODEL_SECTION_RE.search(line)
        if not match:
            continue
        clip = match.group(1)
        key = clip.casefold()
        if key in clips:
            raise BuildError(f"duplicate runtime clip in model info: {clip}")
        clips[key] = clip
    if declared_count is None:
        raise BuildError("ModelAssetConverter info summary is missing")
    if declared_count != len(clips):
        raise BuildError(
            "ModelAssetConverter animation count mismatch: "
            f"declared={declared_count} parsed={len(clips)}"
        )
    return clips


def read_model_info(tool: Path, wmodel: Path) -> dict[str, str]:
    if not tool.is_file():
        raise BuildError(f"ModelAssetConverter does not exist: {tool}")
    if not wmodel.is_file():
        raise BuildError(f"physical WModel does not exist: {wmodel}")
    try:
        completed = subprocess.run(
            [str(tool), "info", str(wmodel)],
            check=False,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
    except OSError as error:
        raise BuildError(f"failed to run ModelAssetConverter: {error}") from error
    if completed.returncode != 0:
        detail = (completed.stderr or completed.stdout).strip()
        raise BuildError(
            f"ModelAssetConverter info failed for {wmodel}: "
            f"exit={completed.returncode} {detail}"
        )
    return parse_model_info(completed.stdout)


def _strict_object(value: Any, keys: set[str], context: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise BuildError(f"{context} must be an object")
    actual = set(value)
    if actual != keys:
        raise BuildError(
            f"{context} property mismatch: "
            f"missing={sorted(keys - actual)} extra={sorted(actual - keys)}"
        )
    return value


def _safe_model_asset_id(value: str) -> bool:
    if not value or "\\" in value or value.endswith(".wmodel"):
        return False
    posix = PurePosixPath(value)
    windows = PureWindowsPath(value)
    return (
        not posix.is_absolute()
        and not windows.is_absolute()
        and not windows.drive
        and "." not in posix.parts
        and ".." not in posix.parts
        and len(posix.parts) >= 3
    )


def _finite_number(value: Any, context: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise BuildError(f"{context} must be numeric")
    result = float(value)
    if not math.isfinite(result):
        raise BuildError(f"{context} must be finite")
    return result


def _milliseconds(value: Any, context: str, *, allow_zero: bool) -> int:
    seconds = _finite_number(value, context)
    if seconds < 0.0 or (not allow_zero and seconds <= 0.0):
        relation = "non-negative" if allow_zero else "positive"
        raise BuildError(f"{context} must be {relation}")
    return int(round(seconds * 1000.0))


def _localized_display_name(source_name: Any, context: str) -> str:
    if not isinstance(source_name, str) or not source_name.strip():
        raise BuildError(f"{context} displayName must be a non-empty string")
    value = COMMON_DISPLAY_NAMES.get(source_name, source_name)
    if not HANGUL_RE.search(value):
        raise BuildError(f"{context} has no Korean display label: {source_name}")
    return value


def load_rpct05_mapping(
    receipt_path: Path, runtime_clips: dict[str, str]
) -> dict[str, str]:
    document = _read_json(receipt_path)
    if not isinstance(document, dict) or document.get("schema") != (
        "lostark.wmodel-animation-retime-receipt"
    ) or document.get("formatVersion") != 1:
        raise BuildError("MN_RPCT_05 retime receipt header is invalid")
    rows = document.get("clips")
    if not isinstance(rows, list) or not rows:
        raise BuildError("MN_RPCT_05 retime receipt clips must be non-empty")
    mapping: dict[str, str] = {}
    runtime_seen: set[str] = set()
    for index, row in enumerate(rows):
        if not isinstance(row, dict):
            raise BuildError(f"retime clips[{index}] must be an object")
        source = row.get("sourceClip")
        runtime = row.get("runtimeClip")
        if not isinstance(source, str) or not source or not isinstance(runtime, str) or not runtime:
            raise BuildError(f"retime clips[{index}] clip identity is invalid")
        source_key = source.casefold()
        runtime_key = runtime.casefold()
        if source_key in mapping or runtime_key in runtime_seen:
            raise BuildError(f"retime clips[{index}] duplicates clip identity")
        if runtime_clips.get(runtime_key) != runtime:
            raise BuildError(
                f"retime runtime clip is absent from physical WModel: {runtime}"
            )
        mapping[source_key] = runtime
        runtime_seen.add(runtime_key)
    if len(mapping) != len(runtime_clips):
        raise BuildError(
            "retime/physical runtime clip count mismatch: "
            f"retime={len(mapping)} runtime={len(runtime_clips)}"
        )
    return mapping


def build_prefix_mapping(
    prefix: str, runtime_clips: dict[str, str]
) -> dict[str, str]:
    prefix_folded = prefix.casefold()
    mapping: dict[str, str] = {}
    for runtime in runtime_clips.values():
        if not runtime.casefold().startswith(prefix_folded):
            raise BuildError(
                f"runtime clip does not use required prefix {prefix}: {runtime}"
            )
        source = runtime[len(prefix) :]
        if not source:
            raise BuildError(f"runtime clip has empty source identity: {runtime}")
        key = source.casefold()
        if key in mapping:
            raise BuildError(f"runtime prefix mapping is ambiguous: {source}")
        mapping[key] = runtime
    return mapping


def load_rpcz_structure(path: Path) -> dict[tuple[int, int], list[str]]:
    document = _read_json(path)
    if not isinstance(document, dict) or document.get("schemaVersion") != 2:
        raise BuildError("MN_RPCZ_00 action structure header is invalid")
    actions = document.get("actions")
    if not isinstance(actions, list):
        raise BuildError("MN_RPCZ_00 action structure actions must be an array")
    result: dict[tuple[int, int], list[str]] = {}
    action_indices: set[int] = set()
    for action in actions:
        if not isinstance(action, dict) or not isinstance(action.get("index"), int):
            raise BuildError("action structure action index is invalid")
        action_index = action["index"]
        if action_index in action_indices:
            raise BuildError(f"duplicate action structure index: {action_index}")
        action_indices.add(action_index)
        stages = action.get("stages")
        if not isinstance(stages, list):
            raise BuildError(f"action structure {action_index} stages must be an array")
        for expected, stage in enumerate(stages, start=1):
            if not isinstance(stage, dict) or stage.get("index") != expected:
                raise BuildError(
                    f"action structure {action_index} stage order is not contiguous"
                )
            clips = stage.get("clips")
            if not isinstance(clips, list) or any(
                not isinstance(clip, str) or not clip for clip in clips
            ):
                raise BuildError(
                    f"action structure {action_index} stage {expected} clips are invalid"
                )
            if len({clip.casefold() for clip in clips}) != len(clips):
                raise BuildError(
                    f"action structure {action_index} stage {expected} clips duplicate"
                )
            result[(action_index, expected - 1)] = clips
    return result


def _action_clip_rows(
    action: dict[str, Any],
    stage: dict[str, Any],
    structure: dict[tuple[int, int], list[str]] | None,
) -> list[tuple[str, dict[str, Any]]]:
    raw_rows = stage.get("animationClips")
    if not isinstance(raw_rows, list):
        raise BuildError("source stage animationClips must be an array")
    for row in raw_rows:
        if not isinstance(row, dict):
            raise BuildError("source animation clip row must be an object")
    if structure is None:
        return [
            (row["clipName"], row)
            for row in raw_rows
            if isinstance(row.get("clipName"), str)
            and row["clipName"] not in {"0", "Anim"}
        ]

    source_index = action.get("sourceActionIndex")
    stage_index = stage.get("stageIndex")
    if not isinstance(source_index, int) or not isinstance(stage_index, int):
        raise BuildError("RPCZ source action/stage identity is invalid")
    structure_clips = structure.get((source_index, stage_index))
    if structure_clips is None:
        raise BuildError(
            f"RPCZ structure stage is missing: action={source_index} stage={stage_index}"
        )
    if structure_clips:
        if len(structure_clips) != len(raw_rows):
            raise BuildError(
                "RPCZ structure/timing occurrence count mismatch: "
                f"action={source_index} stage={stage_index} "
                f"structure={len(structure_clips)} timing={len(raw_rows)}"
            )
        result: list[tuple[str, dict[str, Any]]] = []
        for exact, timing in zip(structure_clips, raw_rows):
            reported = timing.get("clipName")
            if reported not in {"0", "Anim", exact} and (
                not isinstance(reported, str) or reported.casefold() != exact.casefold()
            ):
                raise BuildError(
                    "RPCZ structure/action clip disagreement: "
                    f"action={source_index} stage={stage_index} "
                    f"structure={exact} action={reported}"
                )
            result.append((exact, timing))
        return result
    return [
        (row["clipName"], row)
        for row in raw_rows
        if isinstance(row.get("clipName"), str)
        and row["clipName"] not in {"0", "Anim"}
    ]


def build_reference_document(
    spec: ProfileSpec,
    source_document: dict[str, Any],
    runtime_mapping: dict[str, str],
    source_evidence_sha256: str,
    rpcz_structure: dict[tuple[int, int], list[str]] | None = None,
) -> dict[str, Any]:
    if source_document.get("schema") != "lostark.ue3-action-effect-source" or (
        source_document.get("formatVersion") != 1
    ) or source_document.get("profileId") != spec.profile_id:
        raise BuildError(f"{spec.profile_id} Action source header is invalid")
    actions = source_document.get("actions")
    if not isinstance(actions, list) or not actions:
        raise BuildError(f"{spec.profile_id} Action source actions must be non-empty")

    action_ids: set[int] = set()
    output_actions: list[dict[str, Any]] = []
    for action_ordinal, action in enumerate(actions):
        if not isinstance(action, dict):
            raise BuildError(f"actions[{action_ordinal}] must be an object")
        action_id = action.get("actionId")
        if isinstance(action_id, bool) or not isinstance(action_id, int) or action_id < 0:
            raise BuildError(f"actions[{action_ordinal}].actionId is invalid")
        if action_id in action_ids:
            raise BuildError(f"duplicate source action ID: {action_id}")
        action_ids.add(action_id)
        display_name = _localized_display_name(
            action.get("displayName"), f"actions[{action_ordinal}]"
        )
        stages = action.get("stages")
        if not isinstance(stages, list):
            raise BuildError(f"action {action_id} stages must be an array")

        output_stages: list[dict[str, Any]] = []
        action_has_holdout = False
        for expected_stage, stage in enumerate(stages):
            if not isinstance(stage, dict) or stage.get("stageIndex") != expected_stage:
                raise BuildError(
                    f"action {action_id} source stage order is not contiguous"
                )
            clip_rows = _action_clip_rows(action, stage, rpcz_structure)
            slots: list[dict[str, Any]] = []
            holdouts: list[str] = []
            for clip_ordinal, (extracted, timing) in enumerate(clip_rows):
                if not extracted or not isinstance(extracted, str):
                    raise BuildError(
                        f"action {action_id} stage {expected_stage} clip is invalid"
                    )
                runtime = runtime_mapping.get(extracted.casefold())
                if runtime is None:
                    holdouts.append(extracted)
                    continue
                notify_id = timing.get("notifyId")
                notifies = stage.get("notifies")
                if not isinstance(notifies, list) or not isinstance(notify_id, str):
                    raise BuildError(
                        f"action {action_id} stage {expected_stage} timing link is invalid"
                    )
                matches = [
                    notify
                    for notify in notifies
                    if isinstance(notify, dict) and notify.get("notifyId") == notify_id
                ]
                if len(matches) != 1 or matches[0].get("sourceType") != "Anim":
                    raise BuildError(
                        f"action {action_id} stage {expected_stage} Anim notify link is invalid"
                    )
                slots.append(
                    {
                        "slotId": f"animation-{clip_ordinal:03d}",
                        "extractedClip": extracted,
                        "runtimeClip": runtime,
                        "sourceStartMs": _milliseconds(
                            matches[0].get("localTimeSeconds"),
                            "animation localTimeSeconds",
                            allow_zero=True,
                        ),
                        "playMs": _milliseconds(
                            timing.get("lengthSeconds"),
                            "animation lengthSeconds",
                            allow_zero=False,
                        ),
                        "playRate": 1.0,
                        "loop": "_loop" in extracted.casefold(),
                        "mappingBasis": REFERENCE_BASIS,
                        "authority": AUTHORITY,
                    }
                )
            if len({name.casefold() for name in holdouts}) != len(holdouts):
                raise BuildError(
                    f"action {action_id} stage {expected_stage} holdouts duplicate"
                )
            if holdouts:
                action_has_holdout = True
            output_stages.append(
                {
                    "stageId": f"stage-{expected_stage:03d}",
                    "stageOrdinal": expected_stage,
                    "holdoutClipNames": holdouts,
                    "slots": slots,
                }
            )
        output_actions.append(
            {
                "sourceActionId": action_id,
                "displayName": display_name,
                "reviewStatus": HOLDOUT if action_has_holdout else REVIEW_CANDIDATE,
                "authority": AUTHORITY,
                "stages": output_stages,
            }
        )

    document: dict[str, Any] = {
        "schema": REFERENCE_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "authority": AUTHORITY,
        "profileId": spec.profile_id,
        "modelAssetId": spec.model_asset_id,
        "sourceEvidenceSha256": source_evidence_sha256,
        "referenceRevision": "",
        "actions": output_actions,
    }
    document["referenceRevision"] = calculate_reference_revision(document)
    validate_reference_document(document)
    return document


def calculate_reference_revision(document: dict[str, Any]) -> str:
    semantic = {key: value for key, value in document.items() if key != "referenceRevision"}
    payload = json.dumps(
        semantic,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def validate_reference_document(document: Any) -> None:
    root = _strict_object(document, REFERENCE_ROOT_KEYS, "reference")
    if root["schema"] != REFERENCE_SCHEMA or root["formatVersion"] != FORMAT_VERSION:
        raise BuildError("reference header is invalid")
    if root["authority"] != AUTHORITY:
        raise BuildError("reference authority must be REFERENCE_ONLY")
    if not isinstance(root["profileId"], str) or not root["profileId"]:
        raise BuildError("reference profileId is invalid")
    if not isinstance(root["modelAssetId"], str) or not _safe_model_asset_id(
        root["modelAssetId"]
    ):
        raise BuildError("reference modelAssetId is invalid")
    if not isinstance(root["sourceEvidenceSha256"], str) or not SHA256_RE.fullmatch(
        root["sourceEvidenceSha256"]
    ):
        raise BuildError("reference sourceEvidenceSha256 is invalid")
    if not isinstance(root["referenceRevision"], str) or not SHA256_RE.fullmatch(
        root["referenceRevision"]
    ) or root["referenceRevision"] != calculate_reference_revision(root):
        raise BuildError("referenceRevision does not match the semantic payload")
    if not isinstance(root["actions"], list):
        raise BuildError("reference actions must be an array")

    action_ids: set[int] = set()
    for action_index, raw_action in enumerate(root["actions"]):
        action = _strict_object(raw_action, ACTION_KEYS, f"actions[{action_index}]")
        action_id = action["sourceActionId"]
        if isinstance(action_id, bool) or not isinstance(action_id, int) or action_id < 0:
            raise BuildError(f"actions[{action_index}].sourceActionId is invalid")
        if action_id in action_ids:
            raise BuildError(f"duplicate reference sourceActionId: {action_id}")
        action_ids.add(action_id)
        if not isinstance(action["displayName"], str) or not HANGUL_RE.search(
            action["displayName"]
        ):
            raise BuildError(f"action {action_id} displayName must contain Korean text")
        if action["reviewStatus"] not in {REVIEW_CANDIDATE, HOLDOUT}:
            raise BuildError(f"action {action_id} reviewStatus is invalid")
        if action["authority"] != AUTHORITY:
            raise BuildError(f"action {action_id} authority is invalid")
        if not isinstance(action["stages"], list):
            raise BuildError(f"action {action_id} stages must be an array")
        has_holdout = False
        for stage_ordinal, raw_stage in enumerate(action["stages"]):
            stage = _strict_object(
                raw_stage, STAGE_KEYS, f"action {action_id} stage {stage_ordinal}"
            )
            if stage["stageOrdinal"] != stage_ordinal or stage["stageId"] != (
                f"stage-{stage_ordinal:03d}"
            ) or not STAGE_ID_RE.fullmatch(stage["stageId"]):
                raise BuildError(f"action {action_id} stage identity/order is invalid")
            holdouts = stage["holdoutClipNames"]
            if not isinstance(holdouts, list) or any(
                not isinstance(name, str) or not name or name in {"0", "Anim"}
                for name in holdouts
            ) or len({name.casefold() for name in holdouts}) != len(holdouts):
                raise BuildError(f"action {action_id} stage holdouts are invalid")
            has_holdout = has_holdout or bool(holdouts)
            if not isinstance(stage["slots"], list):
                raise BuildError(f"action {action_id} stage slots must be an array")
            slot_ids: set[str] = set()
            for slot_index, raw_slot in enumerate(stage["slots"]):
                slot = _strict_object(
                    raw_slot,
                    SLOT_KEYS,
                    f"action {action_id} stage {stage_ordinal} slot {slot_index}",
                )
                slot_id = slot["slotId"]
                if not isinstance(slot_id, str) or not SLOT_ID_RE.fullmatch(slot_id):
                    raise BuildError("reference slotId is invalid")
                if slot_id in slot_ids:
                    raise BuildError("reference slotId duplicates within a stage")
                slot_ids.add(slot_id)
                if any(
                    not isinstance(slot[key], str) or not slot[key]
                    for key in ("extractedClip", "runtimeClip")
                ):
                    raise BuildError("reference clip identity is invalid")
                if slot["extractedClip"].casefold() in {
                    name.casefold() for name in holdouts
                }:
                    raise BuildError("holdout clip must not also become a slot")
                if isinstance(slot["sourceStartMs"], bool) or not isinstance(
                    slot["sourceStartMs"], int
                ) or slot["sourceStartMs"] < 0:
                    raise BuildError("reference sourceStartMs is invalid")
                if isinstance(slot["playMs"], bool) or not isinstance(
                    slot["playMs"], int
                ) or slot["playMs"] <= 0:
                    raise BuildError("reference playMs is invalid")
                if _finite_number(slot["playRate"], "reference playRate") <= 0.0:
                    raise BuildError("reference playRate must be positive")
                if not isinstance(slot["loop"], bool):
                    raise BuildError("reference loop must be boolean")
                if slot["mappingBasis"] != REFERENCE_BASIS or slot["authority"] != AUTHORITY:
                    raise BuildError("reference slot contract is invalid")
        expected_review = HOLDOUT if has_holdout else REVIEW_CANDIDATE
        if action["reviewStatus"] != expected_review:
            raise BuildError(f"action {action_id} holdout reviewStatus disagrees with stages")


def build_empty_authored_document(reference: dict[str, Any]) -> dict[str, Any]:
    document = {
        "schema": AUTHORED_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "authority": AUTHORITY,
        "profileId": reference["profileId"],
        "referenceRevision": reference["referenceRevision"],
        "bindings": [],
    }
    validate_authored_document(document, reference)
    return document


def validate_authored_document(document: Any, reference: dict[str, Any]) -> None:
    root = _strict_object(document, AUTHORED_ROOT_KEYS, "authored")
    if root["schema"] != AUTHORED_SCHEMA or root["formatVersion"] != FORMAT_VERSION:
        raise BuildError("authored header is invalid")
    if root["authority"] != AUTHORITY:
        raise BuildError("authored authority must be REFERENCE_ONLY")
    if root["profileId"] != reference["profileId"] or root["referenceRevision"] != (
        reference["referenceRevision"]
    ):
        raise BuildError("authored/reference identity is stale")
    if not isinstance(root["bindings"], list):
        raise BuildError("authored bindings must be an array")

    available: dict[tuple[int, str, str], dict[str, Any]] = {}
    for action in reference["actions"]:
        for stage in action["stages"]:
            for slot in stage["slots"]:
                available[
                    (action["sourceActionId"], stage["stageId"], slot["slotId"])
                ] = slot

    identities: set[tuple[int, str, str]] = set()
    for index, raw_binding in enumerate(root["bindings"]):
        binding = _strict_object(raw_binding, BINDING_KEYS, f"bindings[{index}]")
        source_action_id = binding["sourceActionId"]
        stage_id = binding["stageId"]
        slot_id = binding["slotId"]
        if (
            isinstance(source_action_id, bool)
            or not isinstance(source_action_id, int)
            or source_action_id < 0
        ):
            raise BuildError(f"authored binding sourceActionId is invalid: {index}")
        if not isinstance(stage_id, str) or not STAGE_ID_RE.fullmatch(stage_id):
            raise BuildError(f"authored binding stageId is invalid: {index}")
        if not isinstance(slot_id, str) or not SLOT_ID_RE.fullmatch(slot_id):
            raise BuildError(f"authored binding slotId is invalid: {index}")
        identity = (
            source_action_id,
            stage_id,
            slot_id,
        )
        if identity in identities:
            raise BuildError(f"duplicate authored binding: {identity}")
        identities.add(identity)
        slot = available.get(identity)
        if slot is None:
            raise BuildError(f"authored binding does not resolve to reference: {identity}")
        if binding["mappingBasis"] != AUTHORED_BASIS or binding["authority"] != AUTHORITY:
            raise BuildError(f"authored binding contract is invalid: {identity}")
        runtime_clip = binding["runtimeClip"]
        if not isinstance(runtime_clip, str) or not RUNTIME_CLIP_RE.fullmatch(runtime_clip):
            raise BuildError(f"authored runtimeClip is not a stable token: {identity}")
        source_start_ms = binding["sourceStartMs"]
        if (
            isinstance(source_start_ms, bool)
            or not isinstance(source_start_ms, int)
            or source_start_ms < 0
            or source_start_ms > MAX_TIMELINE_MS
        ):
            raise BuildError(f"authored sourceStartMs is out of range: {identity}")
        play_ms = binding["playMs"]
        if (
            isinstance(play_ms, bool)
            or not isinstance(play_ms, int)
            or play_ms <= 0
            or play_ms > MAX_TIMELINE_MS
        ):
            raise BuildError(f"authored playMs is out of range: {identity}")
        play_rate = _finite_number(binding["playRate"], "authored playRate")
        if play_rate < MIN_PLAY_RATE or play_rate > MAX_PLAY_RATE:
            raise BuildError(f"authored playRate is out of range: {identity}")
        if not isinstance(binding["loop"], bool):
            raise BuildError(f"authored loop must be boolean: {identity}")


def _json_bytes(document: dict[str, Any]) -> bytes:
    return (
        json.dumps(document, ensure_ascii=False, indent=2) + "\n"
    ).encode("utf-8")


def _write_or_check(path: Path, payload: bytes, check: bool) -> None:
    if check:
        if not path.is_file() or path.read_bytes() != payload:
            raise BuildError(f"generated document is stale or missing: {path}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.tmp")
    try:
        temporary.write_bytes(payload)
        temporary.replace(path)
    except OSError as error:
        raise BuildError(f"failed to write {path}: {error}") from error
    finally:
        if temporary.exists():
            temporary.unlink()


def generate(
    source_root: Path,
    runtime_root: Path,
    output_root: Path,
    model_info_tool: Path,
    *,
    check: bool,
) -> list[dict[str, Any]]:
    rpct05_receipt = source_root.parent / "MN_RPCT_05/MN_RPCT_05.retime.receipt.json"
    rpcz_structure_path = source_root.parent / "MN_RPCZ_00.action-structure.json"
    runtime_cache: dict[Path, dict[str, str]] = {}
    wmodel_sha_cache: dict[Path, str] = {}
    results: list[dict[str, Any]] = []

    for spec in PROFILE_SPECS:
        source_path = source_root / f"{spec.profile_id}.action-effects.json"
        wmodel_path = runtime_root / spec.physical_model / f"{spec.physical_model}.wmodel"
        if wmodel_path not in runtime_cache:
            runtime_cache[wmodel_path] = read_model_info(model_info_tool, wmodel_path)
            wmodel_sha_cache[wmodel_path] = _file_sha256(wmodel_path)
        runtime_clips = runtime_cache[wmodel_path]
        evidence = [
            ("action-effects", _file_sha256(source_path)),
            ("runtime-wmodel", wmodel_sha_cache[wmodel_path]),
        ]
        rpcz_structure = None
        if spec.uses_rpct05_retime:
            runtime_mapping = load_rpct05_mapping(rpct05_receipt, runtime_clips)
            evidence.append(("retime-receipt", _file_sha256(rpct05_receipt)))
        elif spec.runtime_prefix is not None:
            runtime_mapping = build_prefix_mapping(spec.runtime_prefix, runtime_clips)
        else:
            raise BuildError(f"profile mapping strategy is missing: {spec.profile_id}")
        if spec.uses_rpcz_structure:
            rpcz_structure = load_rpcz_structure(rpcz_structure_path)
            evidence.append(("action-structure", _file_sha256(rpcz_structure_path)))

        reference = build_reference_document(
            spec,
            _read_json(source_path),
            runtime_mapping,
            aggregate_evidence_sha256(evidence),
            rpcz_structure,
        )
        authored = build_empty_authored_document(reference)
        reference_path = (
            output_root
            / "Reference/KakulSaydon"
            / f"{spec.profile_id}.actionreference.json"
        )
        authored_path = (
            output_root
            / "Authored/KakulSaydon"
            / f"{spec.profile_id}.actionbindings.json"
        )
        _write_or_check(reference_path, _json_bytes(reference), check)
        _write_or_check(authored_path, _json_bytes(authored), check)

        stages = [stage for action in reference["actions"] for stage in action["stages"]]
        results.append(
            {
                "profileId": spec.profile_id,
                "actions": len(reference["actions"]),
                "stages": len(stages),
                "slots": sum(len(stage["slots"]) for stage in stages),
                "holdoutNames": len(
                    {
                        name.casefold()
                        for stage in stages
                        for name in stage["holdoutClipNames"]
                    }
                ),
                "referenceRevision": reference["referenceRevision"],
            }
        )
    return results


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, default=DEFAULT_SOURCE_ROOT)
    parser.add_argument("--runtime-root", type=Path, default=DEFAULT_RUNTIME_ROOT)
    parser.add_argument("--output-root", type=Path, default=DEFAULT_OUTPUT_ROOT)
    parser.add_argument("--model-info-tool", type=Path, default=DEFAULT_MODEL_INFO_TOOL)
    parser.add_argument(
        "--check",
        action="store_true",
        help="fail if committed generated documents differ; do not write",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        results = generate(
            args.source_root.resolve(),
            args.runtime_root.resolve(),
            args.output_root.resolve(),
            args.model_info_tool.resolve(),
            check=args.check,
        )
    except BuildError as error:
        print(f"Kakul animation reference generation failed: {error}", file=sys.stderr)
        return 1
    mode = "checked" if args.check else "generated"
    for result in results:
        print(
            f"{mode} {result['profileId']}: actions={result['actions']} "
            f"stages={result['stages']} slots={result['slots']} "
            f"holdoutNames={result['holdoutNames']} "
            f"revision={result['referenceRevision']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
