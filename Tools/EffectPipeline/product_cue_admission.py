#!/usr/bin/env python3
"""Validate and publish explicit cue-scoped Product Effect admissions.

The policy is deliberately narrower than an Effect document.  A document that
contains AUTHORING_APPROXIMATE carriers is publishable only when the exact
animation cue is present in this policy.  Hard fail-closed carriers remain
suppressed and cannot be converted into execution targets by an approval.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import stat
import tempfile
from typing import Any


POLICY_SCHEMA = "lostark.effect-product-cue-approval-policy"
RUNTIME_SCHEMA = "lostark.effect-product-cue-admissions"
FORMAT_VERSION = 1
ADMISSION = "PRODUCT_APPROVED_APPROXIMATE"
FULL_ADMISSION = "PRODUCT_APPROVED_FULL"
STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]+$")
HEX_SHA256 = re.compile(r"^[0-9a-f]{64}$")
DIRECT_RUNTIME_KEYS = (
    "payloadKind",
    "effectAssetId",
    "authoringFormatVersion",
    "authoredDocumentPath",
    "contentSha256",
    "dependencies",
)


class AdmissionError(ValueError):
    pass


def _reject_duplicate_json_properties(
    pairs: list[tuple[str, Any]],
) -> dict[str, Any]:
    value: dict[str, Any] = {}
    for key, item in pairs:
        if key in value:
            raise AdmissionError(f"duplicate JSON property: {key}")
        value[key] = item
    return value


def _read_bytes(path: Path, label: str) -> bytes:
    try:
        payload = path.read_bytes()
    except OSError as error:
        raise AdmissionError(f"{label} could not be read: {path}: {error}") from error
    if payload.startswith(b"\xef\xbb\xbf"):
        raise AdmissionError(f"{label} must be UTF-8 without BOM: {path}")
    return payload


def _read_json(path: Path, label: str) -> tuple[dict[str, Any], bytes]:
    payload = _read_bytes(path, label)
    try:
        value = json.loads(
            payload.decode("utf-8"),
            object_pairs_hook=_reject_duplicate_json_properties,
        )
    except AdmissionError:
        raise
    except (UnicodeError, json.JSONDecodeError) as error:
        raise AdmissionError(f"{label} is not strict UTF-8 JSON: {path}: {error}") from error
    if not isinstance(value, dict):
        raise AdmissionError(f"{label} must be a JSON object: {path}")
    return value, payload


def _require_exact_keys(value: Any, keys: tuple[str, ...], label: str) -> None:
    if not isinstance(value, dict) or tuple(value.keys()) != keys:
        raise AdmissionError(f"{label} fields or field order are invalid")


def _require_string(value: Any, label: str, *, stable: bool = False) -> str:
    if not isinstance(value, str) or not value or len(value.encode("utf-8")) > 512:
        raise AdmissionError(f"{label} must be a non-empty bounded string")
    if stable and STABLE_ID.fullmatch(value) is None:
        raise AdmissionError(f"{label} is not a stable ID: {value}")
    return value


def _require_uint(value: Any, label: str, maximum: int = 0xFFFFFFFF) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < 0 or value > maximum:
        raise AdmissionError(f"{label} must be an unsigned integer")
    return value


def _sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _policy_path(data_root: Path) -> Path:
    return data_root / "Effects/ProductCueApprovals.json"


def _catalog_path(data_root: Path) -> Path:
    return data_root / "Effects/EffectCatalog.json"


def _load_policy(data_root: Path) -> tuple[dict[str, Any], bytes]:
    policy, payload = _read_json(_policy_path(data_root), "Product cue policy")
    _require_exact_keys(
        policy,
        ("schema", "formatVersion", "decisionSetId", "defaultAdmission", "approvals"),
        "Product cue policy",
    )
    if (
        policy["schema"] != POLICY_SCHEMA
        or policy["formatVersion"] != FORMAT_VERSION
        or policy["defaultAdmission"] != "DENY"
    ):
        raise AdmissionError("Product cue policy header is invalid")
    _require_string(policy["decisionSetId"], "decisionSetId", stable=True)
    approvals = policy["approvals"]
    if not isinstance(approvals, list) or not approvals:
        raise AdmissionError("Product cue policy approvals must be a non-empty array")
    return policy, payload


def _catalog_entries(data_root: Path) -> dict[str, dict[str, Any]]:
    catalog, _ = _read_json(_catalog_path(data_root), "Effect source catalog")
    if catalog.get("formatVersion") != 1 or not isinstance(catalog.get("effects"), list):
        raise AdmissionError("Effect source catalog header is invalid")
    result: dict[str, dict[str, Any]] = {}
    for entry in catalog["effects"]:
        if not isinstance(entry, dict):
            raise AdmissionError("Effect source catalog contains a non-object entry")
        effect_id = _require_string(entry.get("effectAssetId"), "catalog effectAssetId", stable=True)
        if effect_id in result:
            raise AdmissionError(f"Duplicate Effect source catalog entry: {effect_id}")
        result[effect_id] = entry
    return result


def _runtime_entries(runtime_catalog: dict[str, Any]) -> dict[str, dict[str, Any]]:
    if (
        runtime_catalog.get("schema") != "lostark.effect-runtime-catalog"
        or runtime_catalog.get("formatVersion") != 3
        or not isinstance(runtime_catalog.get("effects"), list)
    ):
        raise AdmissionError("Runtime Effect catalog must be typed formatVersion 3")
    result: dict[str, dict[str, Any]] = {}
    for entry in runtime_catalog["effects"]:
        if not isinstance(entry, dict):
            raise AdmissionError("Runtime Effect catalog contains a non-object entry")
        effect_id = _require_string(entry.get("effectAssetId"), "runtime effectAssetId", stable=True)
        if effect_id in result:
            raise AdmissionError(f"Duplicate runtime Effect entry: {effect_id}")
        result[effect_id] = entry
    return result


def _load_sealed_runtime_document(
    runtime_catalog_path: Path,
    runtime_entry: dict[str, Any],
    effect_id: str,
) -> tuple[dict[str, Any], bytes]:
    _require_exact_keys(
        runtime_entry, DIRECT_RUNTIME_KEYS, "Direct authored runtime entry"
    )
    if (
        runtime_entry["payloadKind"] != "DIRECT_AUTHORED_DOCUMENT_V13"
        or runtime_entry["effectAssetId"] != effect_id
        or type(runtime_entry["authoringFormatVersion"]) is not int
        or runtime_entry["authoringFormatVersion"] != 13
    ):
        raise AdmissionError(
            f"Approved Effect runtime identity/version is invalid: {effect_id}"
        )
    content_sha = _require_string(
        runtime_entry["contentSha256"], "runtime contentSha256"
    )
    if HEX_SHA256.fullmatch(content_sha) is None:
        raise AdmissionError(
            f"Approved Effect runtime content hash is invalid: {effect_id}"
        )
    expected_path = f"Authored/{effect_id}.{content_sha}.effect.json"
    if runtime_entry["authoredDocumentPath"] != expected_path:
        raise AdmissionError(
            f"Approved Effect runtime document path is not canonical: {effect_id}"
        )

    catalog_root = runtime_catalog_path.resolve(strict=False).parent
    candidate = catalog_root.joinpath(*expected_path.split("/"))
    try:
        resolved = candidate.resolve(strict=True)
    except OSError as error:
        raise AdmissionError(
            f"Approved Effect sealed runtime document could not be resolved: "
            f"{effect_id}: {error}"
        ) from error
    try:
        resolved.relative_to(catalog_root)
    except ValueError as error:
        raise AdmissionError(
            f"Approved Effect sealed runtime document escapes the catalog: "
            f"{effect_id}"
        ) from error
    try:
        candidate_stat = candidate.lstat()
    except OSError as error:
        raise AdmissionError(
            f"Approved Effect sealed runtime document could not be inspected: "
            f"{effect_id}: {error}"
        ) from error
    reparse_flag = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0)
    file_attributes = getattr(candidate_stat, "st_file_attributes", 0)
    if (
        not stat.S_ISREG(candidate_stat.st_mode)
        or (reparse_flag and file_attributes & reparse_flag)
        or not resolved.is_file()
    ):
        raise AdmissionError(
            f"Approved Effect sealed runtime document is not a regular file: "
            f"{effect_id}"
        )
    payload = _read_bytes(resolved, f"Approved Effect sealed runtime document {effect_id}")
    if _sha256(payload) != content_sha:
        raise AdmissionError(
            f"Approved Effect sealed runtime document hash drifted: {effect_id}"
        )
    try:
        document = json.loads(
            payload.decode("utf-8"),
            object_pairs_hook=_reject_duplicate_json_properties,
        )
    except AdmissionError:
        raise
    except (UnicodeError, json.JSONDecodeError) as error:
        raise AdmissionError(
            f"Approved Effect sealed runtime document is not strict UTF-8 JSON: "
            f"{effect_id}: {error}"
        ) from error
    if (
        not isinstance(document, dict)
        or document.get("schema") != "lostark.effect-authoring"
        or type(document.get("version")) is not int
        or document.get("version") != 13
        or document.get("effectAssetId") != effect_id
        or not isinstance(document.get("elements"), list)
    ):
        raise AdmissionError(
            f"Approved Effect sealed runtime document identity is invalid: "
            f"{effect_id}"
        )
    return document, payload


def _load_player_skills(data_root: Path) -> dict[tuple[str, int], dict[str, Any]]:
    document, _ = _read_json(data_root / "Balance/PlayerSkills.json", "PlayerSkills")
    skills = document.get("skills")
    if not isinstance(skills, list):
        raise AdmissionError("PlayerSkills.skills must be an array")
    result: dict[tuple[str, int], dict[str, Any]] = {}
    for skill in skills:
        if not isinstance(skill, dict):
            raise AdmissionError("PlayerSkills contains a non-object row")
        key = (
            _require_string(skill.get("characterClass"), "skill characterClass", stable=True),
            _require_uint(skill.get("skillId"), "skillId"),
        )
        if key in result:
            raise AdmissionError(f"Duplicate PlayerSkills row: {key}")
        result[key] = skill
    return result


def _binding_clip(
    data_root: Path,
    animation_asset_id: str,
    character_class: str,
    skill_id: int,
    stage_index: int,
) -> str:
    path = data_root / "Animation/Authored" / animation_asset_id / (
        f"{animation_asset_id}.skillbindings.json"
    )
    document, _ = _read_json(path, f"{animation_asset_id} skill bindings")
    if (
        document.get("animationAssetId") != animation_asset_id
        or document.get("characterClass") != character_class
        or not isinstance(document.get("bindings"), list)
    ):
        raise AdmissionError(f"Skill binding owner mismatch: {animation_asset_id}")
    matches = [row for row in document["bindings"] if isinstance(row, dict) and row.get("skillId") == skill_id]
    if len(matches) != 1:
        raise AdmissionError(f"Skill binding must contain skill exactly once: {character_class}/{skill_id}")
    clips = matches[0].get("clips")
    if not isinstance(clips, list) or not clips:
        raise AdmissionError(f"Skill binding clips are invalid: {character_class}/{skill_id}")
    if all(isinstance(value, str) for value in clips):
        if stage_index != 0 or len(clips) != 1:
            raise AdmissionError(f"Active cue approval must identify its sole stage: {character_class}/{skill_id}")
        return _require_string(clips[0], "binding clip")
    if stage_index >= len(clips) or not isinstance(clips[stage_index], list):
        raise AdmissionError(f"Combo stage is outside skill binding: {character_class}/{skill_id}/{stage_index}")
    stage_clips = clips[stage_index]
    if len(stage_clips) != 1 or not isinstance(stage_clips[0], str):
        raise AdmissionError("Canary approval requires exactly one clip in the selected stage")
    return _require_string(stage_clips[0], "binding stage clip")


def _parse_animevent_payload(
    path: Path, payload: bytes, animation_asset_id: str
) -> tuple[list[str], list[dict[str, Any]]]:
    try:
        lines = payload.decode("utf-8").splitlines()
    except UnicodeError as error:
        raise AdmissionError(f"Animevents is not UTF-8: {path}: {error}") from error
    if not lines:
        raise AdmissionError(f"Animevents is empty: {path}")
    try:
        header = shlex.split(lines[0], posix=True)
    except ValueError as error:
        raise AdmissionError(f"Animevents header is invalid: {path}") from error
    event_lines = [line for line in lines[1:] if line]
    try:
        version = int(header[1]) if len(header) == 4 else -1
        declared_rows = int(header[3]) if len(header) == 4 else -1
    except ValueError as error:
        raise AdmissionError(
            f"Animevents owner/version/count is invalid: {path}"
        ) from error
    if (
        len(header) != 4
        or header[0] != "LOSTARK_ANIM_EVENTS"
        or version < 3
        or version > 5
        or header[2] != animation_asset_id
        or declared_rows != len(event_lines)
    ):
        raise AdmissionError(f"Animevents owner/version/count is invalid: {path}")
    rows: list[dict[str, Any]] = []
    for index, line in enumerate(lines[1:], start=1):
        if not line:
            continue
        try:
            tokens = shlex.split(line, posix=True)
        except ValueError as error:
            raise AdmissionError(f"Animevents row is invalid: {path}:{index + 1}") from error
        if len(tokens) < 3:
            raise AdmissionError(f"Animevents row is invalid: {path}:{index + 1}")
        if tokens[1] != "EFFECT":
            continue
        fields: dict[str, str] = {}
        for token in tokens[2:]:
            if "=" not in token:
                raise AdmissionError(
                    f"Animevents EFFECT field is invalid: {path}:{index + 1}"
                )
            key, value = token.split("=", 1)
            if key in fields:
                raise AdmissionError(f"Duplicate animevent field: {path}:{index + 1}")
            fields[key] = value
        if fields.get("effectref") != "asset":
            continue
        try:
            start_token = fields["startms"]
            if re.fullmatch(r"[0-9]+", start_token) is None:
                raise ValueError(start_token)
            start_ms = int(start_token)
            if start_ms > 0xFFFFFFFF:
                raise ValueError(start_token)
        except (KeyError, ValueError) as error:
            raise AdmissionError(f"Product animevent startms is invalid: {path}:{index + 1}") from error
        rows.append(
            {
                "lineIndex": index,
                "clipName": tokens[0],
                "startMs": start_ms,
                "effectAssetId": fields.get("payload", ""),
            }
        )
    return lines, rows


def _animevent_rows(data_root: Path, animation_asset_id: str) -> tuple[Path, list[str], list[dict[str, Any]]]:
    path = data_root / "Animation/Authored" / animation_asset_id / (
        f"{animation_asset_id}.animevents"
    )
    payload = _read_bytes(path, f"{animation_asset_id} animevents")
    lines, rows = _parse_animevent_payload(path, payload, animation_asset_id)
    return path, lines, rows


def _document_counts(document: dict[str, Any], effect_id: str) -> tuple[int, int]:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != effect_id
        or not isinstance(document.get("elements"), list)
    ):
        raise AdmissionError(f"Approved Effect document identity is invalid: {effect_id}")
    approximate_count = 0
    hard_count = 0
    for element in document["elements"]:
        if not isinstance(element, dict) or not isinstance(element.get("material"), dict):
            raise AdmissionError(f"Approved Effect element is malformed: {effect_id}")
        execution = element["material"].get("execution")
        if execution is None:
            continue
        if not isinstance(execution, dict):
            raise AdmissionError(f"Approved Effect execution is malformed: {effect_id}")
        approximate = execution.get("authoringApproximate") is True
        fail_closed = execution.get("failClosed") is True
        if approximate:
            if not fail_closed:
                raise AdmissionError(f"Approximate carrier is not fail-closed tagged: {effect_id}")
            approximate_count += 1
        elif fail_closed:
            if execution.get("enabled") is not False or element.get("visible") is not False:
                raise AdmissionError(f"Hard carrier is not suppressed: {effect_id}")
            hard_count += 1
    return approximate_count, hard_count


def _validate_approvals(
    data_root: Path,
    runtime_catalog: dict[str, Any],
    runtime_catalog_path: Path,
) -> tuple[dict[str, Any], bytes, list[dict[str, Any]]]:
    policy, policy_payload = _load_policy(data_root)
    if (
        tuple(runtime_catalog.keys())
        != (
            "schema",
            "formatVersion",
            "productCueAdmissionsRequired",
            "productCuePolicySha256",
            "components",
            "effects",
        )
        or runtime_catalog.get("schema") != "lostark.effect-runtime-catalog"
        or runtime_catalog.get("formatVersion") != 3
        or runtime_catalog.get("productCueAdmissionsRequired") is not True
        or runtime_catalog.get("productCuePolicySha256") != _sha256(policy_payload)
    ):
        raise AdmissionError(
            "Runtime catalog does not pin the exact Product cue source policy"
        )
    source_entries = _catalog_entries(data_root)
    runtime_entries = _runtime_entries(runtime_catalog)
    player_skills = _load_player_skills(data_root)
    exact_keys = (
        "cueId", "approvalCeiling", "animationAssetId", "characterClass", "inputSlot",
        "skillId", "stageIndex", "clipName", "startMs", "effectAssetId",
        "rollbackEffectAssetId", "effectContentSha256", "provenance",
    )
    provenance_keys = ("decision", "approvedAtKst", "sourceThreadId", "scope")
    cue_ids: set[str] = set()
    cue_keys: set[tuple[str, str, int, str]] = set()
    cue_owners: set[tuple[str, str, int]] = set()
    target_owners: dict[str, tuple[str, str, int]] = {}
    candidate_ids: set[str] = set()
    rollback_ids: set[str] = set()
    receipt_rows: list[dict[str, Any]] = []
    animevent_cache: dict[str, list[dict[str, Any]]] = {}
    for row in policy["approvals"]:
        _require_exact_keys(row, exact_keys, "Product cue approval")
        _require_exact_keys(row["provenance"], provenance_keys, "Product cue provenance")
        cue_id = _require_string(row["cueId"], "cueId", stable=True)
        if cue_id in cue_ids:
            raise AdmissionError(f"Duplicate Product cue approval ID: {cue_id}")
        cue_ids.add(cue_id)
        if row["approvalCeiling"] != ADMISSION:
            raise AdmissionError(f"Unsupported Product cue admission: {cue_id}")
        animation_asset_id = _require_string(row["animationAssetId"], "animationAssetId", stable=True)
        character_class = _require_string(row["characterClass"], "characterClass", stable=True)
        input_slot = _require_string(row["inputSlot"], "inputSlot", stable=True)
        skill_id = _require_uint(row["skillId"], "skillId")
        stage_index = _require_uint(row["stageIndex"], "stageIndex", 255)
        clip_name = _require_string(row["clipName"], "clipName")
        start_ms = _require_uint(row["startMs"], "startMs")
        effect_id = _require_string(row["effectAssetId"], "effectAssetId", stable=True)
        rollback_id = _require_string(row["rollbackEffectAssetId"], "rollbackEffectAssetId", stable=True)
        content_sha = _require_string(row["effectContentSha256"], "effectContentSha256")
        if HEX_SHA256.fullmatch(content_sha) is None or effect_id == rollback_id:
            raise AdmissionError(f"Product cue target/hash is invalid: {cue_id}")
        candidate_ids.add(effect_id)
        rollback_ids.add(rollback_id)
        cue_key = (animation_asset_id, clip_name, start_ms, effect_id)
        if cue_key in cue_keys:
            raise AdmissionError(f"Duplicate Product cue approval tuple: {cue_id}")
        cue_keys.add(cue_key)
        owner = (animation_asset_id, clip_name, start_ms)
        if owner in cue_owners:
            raise AdmissionError(f"Physical Product cue has multiple approvals: {cue_id}")
        cue_owners.add(owner)
        previous_owner = target_owners.setdefault(effect_id, owner)
        if previous_owner != owner:
            raise AdmissionError(f"Approval-managed Effect has multiple cue owners: {effect_id}")
        skill = player_skills.get((character_class, skill_id))
        if skill is None or skill.get("inputSlot") != input_slot:
            raise AdmissionError(f"Product cue PlayerSkills identity mismatch: {cue_id}")
        if _binding_clip(
            data_root, animation_asset_id, character_class, skill_id, stage_index
        ) != clip_name:
            raise AdmissionError(f"Product cue skillbinding clip mismatch: {cue_id}")
        if animation_asset_id not in animevent_cache:
            _, _, animevent_cache[animation_asset_id] = _animevent_rows(
                data_root, animation_asset_id
            )
        cue_rows = [
            cue for cue in animevent_cache[animation_asset_id]
            if cue["clipName"] == clip_name and cue["startMs"] == start_ms
        ]
        if len(cue_rows) != 1 or cue_rows[0]["effectAssetId"] not in (effect_id, rollback_id):
            raise AdmissionError(f"Product cue is neither candidate nor rollback target: {cue_id}")
        source_entry = source_entries.get(effect_id)
        rollback_entry = source_entries.get(rollback_id)
        runtime_entry = runtime_entries.get(effect_id)
        if (
            source_entry is None
            or source_entry.get("payloadKind") != "DIRECT_AUTHORED_DOCUMENT_V13"
            or rollback_entry is None
            or runtime_entry is None
            or runtime_entry.get("payloadKind") != "DIRECT_AUTHORED_DOCUMENT_V13"
            or runtime_entries.get(rollback_id) is None
        ):
            raise AdmissionError(f"Product cue catalog candidate/rollback is missing: {cue_id}")
        authored_path = source_entry.get("authoringPath")
        if authored_path != f"Effects/Authored/{effect_id}.effect.json":
            raise AdmissionError(f"Approved authoringPath is not canonical: {cue_id}")
        document, document_payload = _read_json(
            data_root / authored_path, f"Approved Effect {effect_id}"
        )
        _, runtime_payload = _load_sealed_runtime_document(
            runtime_catalog_path, runtime_entry, effect_id
        )
        if _sha256(document_payload) != content_sha:
            raise AdmissionError(f"Approved Effect source hash drifted: {cue_id}")
        if runtime_entry.get("contentSha256") != content_sha:
            raise AdmissionError(f"Approved Effect runtime hash drifted: {cue_id}")
        if runtime_payload != document_payload:
            raise AdmissionError(f"Approved Effect runtime payload is not the pinned source: {cue_id}")
        approximate_count, hard_count = _document_counts(document, effect_id)
        provenance = row["provenance"]
        if provenance["decision"] != "EXPLICIT_USER_OPT_IN":
            raise AdmissionError(f"Product cue provenance is not explicit opt-in: {cue_id}")
        for name in provenance_keys[1:]:
            _require_string(provenance[name], f"{cue_id} provenance.{name}")
        receipt_rows.append(
            {
                "cueId": cue_id,
                "admission": ADMISSION if approximate_count else FULL_ADMISSION,
                "approvalCeiling": ADMISSION,
                "observedExactness": (
                    "AUTHORING_APPROXIMATE" if approximate_count else "FULL"
                ),
                "characterClass": character_class,
                "inputSlot": input_slot,
                "skillId": skill_id,
                "stageIndex": stage_index,
                "animationAssetId": animation_asset_id,
                "clipName": clip_name,
                "startMs": start_ms,
                "effectAssetId": effect_id,
                "rollbackEffectAssetId": rollback_id,
                "effectContentSha256": content_sha,
                "approximateElementCount": approximate_count,
                "hardSuppressedElementCount": hard_count,
                "provenance": dict(provenance),
            }
        )
    overlap = sorted(candidate_ids & rollback_ids)
    if overlap:
        raise AdmissionError(
            "Product cue candidate and rollback sets overlap: " + ", ".join(overlap)
        )
    for effect_id, source_entry in source_entries.items():
        if source_entry.get("payloadKind") != "DIRECT_AUTHORED_DOCUMENT_V13":
            continue
        authored_path = source_entry.get("authoringPath")
        if authored_path != f"Effects/Authored/{effect_id}.effect.json":
            raise AdmissionError(f"Direct authored authoringPath is not canonical: {effect_id}")
        document, _ = _read_json(
            data_root / authored_path, f"Direct authored Effect {effect_id}"
        )
        approximate_count, _ = _document_counts(document, effect_id)
        if approximate_count and effect_id not in target_owners:
            raise AdmissionError(
                f"Approximate Product Effect has no explicit cue approval: {effect_id}"
            )
    return policy, policy_payload, receipt_rows


def _runtime_receipt_payload(data_root: Path, runtime_catalog_path: Path) -> bytes:
    runtime_catalog, runtime_payload = _read_json(runtime_catalog_path, "Runtime Effect catalog")
    policy, policy_payload, approvals = _validate_approvals(
        data_root, runtime_catalog, runtime_catalog_path
    )
    receipt = {
        "schema": RUNTIME_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "runtimeCatalogSha256": _sha256(runtime_payload),
        "sourcePolicySha256": _sha256(policy_payload),
        "sourcePolicyUtf8Json": policy_payload.decode("utf-8"),
        "decisionSetId": policy["decisionSetId"],
        "admissionMode": "EXPLICIT_CUE_OPT_IN_ONLY",
        "approvals": sorted(approvals, key=lambda row: row["cueId"]),
    }
    return (json.dumps(receipt, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def build_runtime_receipt(data_root: Path, runtime_catalog_path: Path, output: Path) -> None:
    payload = _runtime_receipt_payload(data_root, runtime_catalog_path)
    output.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{output.name}.", suffix=".tmp", dir=output.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, output)
    finally:
        if temporary.exists():
            temporary.unlink()


def set_cue(
    data_root: Path,
    cue_id: str,
    target: str,
    check: bool,
    runtime_catalog_path: Path,
    runtime_receipt_path: Path,
) -> None:
    expected_receipt = _runtime_receipt_payload(data_root, runtime_catalog_path)
    installed_receipt = _read_bytes(
        runtime_receipt_path, "Installed Product cue admission receipt"
    )
    if installed_receipt != expected_receipt:
        raise AdmissionError(
            "Installed Product cue admission receipt is missing or stale"
        )
    policy, _ = _load_policy(data_root)
    matches = [row for row in policy["approvals"] if isinstance(row, dict) and row.get("cueId") == cue_id]
    if len(matches) != 1:
        raise AdmissionError(f"Unknown or duplicate Product cue approval ID: {cue_id}")
    row = matches[0]
    desired = row["effectAssetId"] if target == "candidate" else row["rollbackEffectAssetId"]
    path, lines, rows = _animevent_rows(data_root, row["animationAssetId"])
    matches = [
        cue for cue in rows
        if cue["clipName"] == row["clipName"] and cue["startMs"] == row["startMs"]
    ]
    if len(matches) != 1:
        raise AdmissionError(f"Product cue source row is missing or ambiguous: {cue_id}")
    current = matches[0]
    allowed = (row["effectAssetId"], row["rollbackEffectAssetId"])
    if current["effectAssetId"] not in allowed:
        raise AdmissionError(f"Product cue source row escaped candidate/rollback pair: {cue_id}")
    if current["effectAssetId"] == desired:
        return
    if check:
        raise AdmissionError(f"Product cue target is stale: {cue_id}: {current['effectAssetId']} != {desired}")
    line_index = current["lineIndex"]
    old_line = lines[line_index]
    pattern = re.compile(r'(?<![A-Za-z0-9_])payload="[^"]*"')
    replacement = f'payload="{desired}"'
    new_line, count = pattern.subn(replacement, old_line)
    if count != 1:
        raise AdmissionError(f"Product cue payload field is missing or ambiguous: {cue_id}")
    original_payload = _read_bytes(path, f"{row['animationAssetId']} animevents")
    old_line_payload = old_line.encode("utf-8")
    if original_payload.count(old_line_payload) != 1:
        raise AdmissionError(f"Product cue source bytes are ambiguous: {cue_id}")
    payload = original_payload.replace(
        old_line_payload, new_line.encode("utf-8"), 1
    )
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    backup = path.with_name(f".{path.name}.{os.getpid()}.bak")
    staged = path.with_name(f".{path.name}.{os.getpid()}.stage")
    backup_owned = False
    preserve_backup = False
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        # Strictly reparse the staged document before replacing the source.
        os.replace(temporary, staged)
        staged_payload = _read_bytes(staged, "staged animevents")
        _, staged_rows = _parse_animevent_payload(
            staged, staged_payload, row["animationAssetId"]
        )
        staged_matches = [
            cue for cue in staged_rows
            if cue["clipName"] == row["clipName"]
            and cue["startMs"] == row["startMs"]
            and cue["effectAssetId"] == desired
        ]
        if len(staged_matches) != 1:
            raise AdmissionError(f"Staged Product cue failed exact validation: {cue_id}")
        os.replace(path, backup)
        backup_owned = True
        try:
            os.replace(staged, path)
        except BaseException:
            try:
                os.replace(backup, path)
                backup_owned = False
            except BaseException:
                preserve_backup = True
                raise
            raise
        backup.unlink(missing_ok=True)
        backup_owned = False
    finally:
        temporary.unlink(missing_ok=True)
        if backup_owned and not preserve_backup:
            backup.unlink(missing_ok=True)
        staged.unlink(missing_ok=True)


def _main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, required=True)
    parser.add_argument("--data-root", type=Path)
    subparsers = parser.add_subparsers(dest="command", required=True)
    build = subparsers.add_parser("build-runtime-receipt")
    build.add_argument("--runtime-catalog", type=Path, required=True)
    build.add_argument("--output", type=Path, required=True)
    set_parser = subparsers.add_parser("set-cue")
    set_parser.add_argument("--cue-id", required=True)
    set_parser.add_argument("--target", choices=("candidate", "rollback"), required=True)
    set_parser.add_argument("--check", action="store_true")
    arguments = parser.parse_args()
    repository_root = arguments.repository_root.resolve()
    data_root = arguments.data_root.resolve() if arguments.data_root else repository_root / "Data"
    try:
        if arguments.command == "build-runtime-receipt":
            build_runtime_receipt(data_root, arguments.runtime_catalog.resolve(), arguments.output.resolve())
        else:
            runtime_root = repository_root / "Client/Bin/DataFiles/Effect"
            set_cue(
                data_root,
                arguments.cue_id,
                arguments.target,
                arguments.check,
                runtime_root / "EffectCatalog.runtime.json",
                runtime_root / "EffectProductCueAdmissions.runtime.json",
            )
    except AdmissionError as error:
        print(f"FAIL: {error}")
        return 1
    print(f"PASS: Product cue admission {arguments.command}")
    return 0


if __name__ == "__main__":
    raise SystemExit(_main())
