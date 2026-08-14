#!/usr/bin/env python3
"""Build the first four-class Track A authored-import admission batch.

The artifact produced here is an offline, hash-pinned materializer input.  It
does not write authored Effects and it never mutates EffectCatalog or animation
events.  The closed scope is the twelve executable three-class BA visual
programs plus the Warlord 17000 BA1 fail-closed extension canary.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
import re
import tempfile
from collections import Counter
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent

SCHEMA_RELATIVE_PATH = (
    "Tools/EffectPipeline/Schemas/"
    "lostark.effect-authored-import-batch.schema.json"
)
VISUAL_RUNTIME_RELATIVE_PATH = (
    "Data/Effects/VisualPrograms/effect-visual-program-runtime.v1.json"
)
SOURCE_ADMISSION_RELATIVE_PATH = (
    "Data/Effects/Imported/CombatBA/lmb-combo-3class.v1.runtime-admission.json"
)
DEFAULT_OUTPUT_RELATIVE_PATH = (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.track-a-authored-import-batch.json"
)
WARLORD_SELECTION_RECEIPT_RELATIVE_PATH = (
    "Data/Effects/AuthoredCorrections/Generated/Warlord/"
    "effect.warlord.skill.17000.ba1.approximation-receipt.json"
)
FOUR_CLASS_ROLLOUT_RELATIVE_PATH = (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.authored-product-rollout.json"
)
EFFECT_CATALOG_RELATIVE_PATH = "Data/Effects/EffectCatalog.json"
CLASS_ANIMEVENTS = {
    "ARTIST": "Data/Animation/Authored/Artist/Artist.animevents",
    "DIMENSIONMASTER": (
        "Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents"
    ),
    "LANCE_MASTER": (
        "Data/Animation/Authored/LanceMaster/LanceMaster.animevents"
    ),
    "WARLORD": "Data/Animation/Authored/Warlord/Warlord.animevents",
}

BATCH_SCHEMA = "lostark.effect-authored-import-batch"
BATCH_VERSION = 1
BATCH_ID = "effect.authored-import.four-class.track-a.v1"
CONTRACT_ROLE = "OFFLINE_AUTHORING_STAGE_INPUT_NOT_PRODUCT_MAPPING"

VISUAL_RUNTIME_SCHEMA = "lostark.effect-visual-program-runtime"
SOURCE_ADMISSION_SCHEMA = "lostark.effect-source-runtime-admission-index"
SOURCE_MANIFEST_SCHEMA = "lostark.combat-effect-source-stage-manifest"
SOURCE_RECEIPT_SCHEMA = "lostark.combat-effect-product-source-receipt"
CONVERSION_RECEIPT_SCHEMA = (
    "lostark.imported-effect-element-conversion-receipt"
)
AUTHORING_SCHEMA = "lostark.effect-authoring"
FOUR_CLASS_ROLLOUT_SCHEMA = "lostark.four-class-authored-product-rollout"

TRACK_A_PROJECTION_KIND = "SOURCE_RECIPE_OVERLAY_V1"
TRACK_A_SELECTION_KIND = "TRACK_A_VISUAL_PROGRAM"
WARLORD_SELECTION_KIND = "WARLORD_FAIL_CLOSED_CANARY"
WARLORD_CANARY_ID = "extension-canary.warlord.generic-selector.v1"
WARLORD_EFFECT_ASSET_ID = "effect.warlord.skill.17000.ba1"
WARLORD_SOURCE_EVENT_ID = "source-event-003"
WARLORD_SKILL_ID = 17000
WARLORD_STAGE_INDEX = 0

CLASS_MANIFESTS = {
    "ARTIST": "Data/Effects/Imported/Artist/Artist.combat-source-stage-manifest.json",
    "DIMENSIONMASTER": (
        "Data/Effects/Imported/DimensionMaster/"
        "DimensionMaster.combat-source-stage-manifest.json"
    ),
    "LANCE_MASTER": (
        "Data/Effects/Imported/LanceMaster/"
        "LanceMaster.combat-source-stage-manifest.json"
    ),
    "WARLORD": (
        "Data/Effects/Imported/Warlord/"
        "Warlord.combat-source-stage-manifest.json"
    ),
}
CLASS_ORDER = {
    "ARTIST": 0,
    "DIMENSIONMASTER": 1,
    "LANCE_MASTER": 2,
    "WARLORD": 3,
}

EXPECTED_DENOMINATORS = {
    "stageCount": 13,
    "trackAProgramStageCount": 12,
    "failClosedCanaryStageCount": 1,
    "trackASelectedSourceRowCount": 70,
    "trackAExcludedSourceRowCount": 63,
    "supplementalElementCount": 4,
    "canaryElementCount": 5,
    "elementPlanCount": 79,
    "genericParticleCandidateCount": 71,
    "familyAdapterRequiredCount": 4,
    "supplementalAdapterPreserveCount": 4,
    "typedExecutionMaterialCount": 0,
    "admittedSourceProfileMaterialCount": 16,
    "failClosedMaterialCount": 63,
    "productMutationCount": 0,
}
EXPECTED_MATERIAL_COUNTS = {
    "ADMITTED_SOURCE_PROFILE": 16,
    "FAIL_CLOSED": 63,
    "TYPED_EXECUTION": 0,
}
EXPECTED_CARRIER_COUNTS = {
    "FAMILY_ADAPTER_REQUIRED": 4,
    "GENERIC_PARTICLE_IMPORT_CANDIDATE": 71,
    "SUPPLEMENTAL_ADAPTER_PRESERVE": 4,
}

EXPECTED_FULL_SCOPE_DENOMINATORS = {
    "fullSkillCount": 51,
    "fullStageCount": 74,
    "fullClipOccurrenceCount": 113,
    "fullVisualClipOccurrenceCount": 102,
    "fullSilentOrNoCarrierClipOccurrenceCount": 11,
    "fullUniqueCandidateDocumentCount": 101,
    "trackASeamStageCount": 13,
    "trackASeamCandidateDocumentCount": 13,
    "legacyStarterSkillIdentityCount": 48,
    "legacyStarterStageCount": 61,
    "legacyStarterEffectBearingStageCount": 60,
    "legacyStarterIntentionallySilentStageCount": 1,
    "legacyStarterClipOccurrenceCount": 100,
    "legacyStarterVisualClipOccurrenceCount": 89,
    "legacyStarterSilentOrNoCarrierClipOccurrenceCount": 11,
    "legacyStarterCandidateDocumentCount": 88,
    "legacyStarterActiveProductReferenceCount": 85,
    "legacyStarterOrphanedCatalogReferenceCount": 3,
    "productMutationCount": 0,
}

KNOWN_BINDING_MANIFEST_DRIFT = {
    ("ARTIST", 31210),
    ("WARLORD", 17080),
    ("WARLORD", 17240),
    ("WARLORD", 17820),
}
EXPECTED_ORPHANED_PRODUCT_EFFECT_IDS = {
    "effect.warlord.skill.17820.authored-baseline.clip3",
    "effect.warlord.skill.17820.authored-baseline.clip4",
    "effect.warlord.skill.17820.authored-baseline.clip8",
}

SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
STABLE_ID_RE = re.compile(r"^[A-Za-z0-9_.:-]+$")
PRODUCT_EFFECT_RE = re.compile(
    r"^effect\.(?P<class_id>[a-z0-9]+)\.skill\."
    r"(?P<skill_id>[1-9][0-9]*)\.ba(?P<stage>[1-9][0-9]*)$"
)


class ContractError(ValueError):
    """Raised when a batch input is stale, ambiguous, or unsafe."""


def _reject_non_finite(value: str) -> None:
    raise ContractError(f"non-finite JSON number is forbidden: {value}")


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(
            path.read_text(encoding="utf-8-sig"),
            parse_constant=_reject_non_finite,
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ContractError(f"cannot parse JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise ContractError(f"JSON root must be an object: {path}")
    return value


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        allow_nan=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def canonical_json_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2)
        + "\n"
    ).encode("utf-8")


def raw_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as source:
            for block in iter(lambda: source.read(1024 * 1024), b""):
                digest.update(block)
    except OSError as error:
        raise ContractError(f"cannot hash {path}: {error}") from error
    return digest.hexdigest()


def _unified_candidate_identity(product_effect_id: str) -> tuple[str, str]:
    _require_stable_id(product_effect_id, "candidate source Effect ID")
    _require(
        product_effect_id.startswith("effect.")
        and ".skill." in product_effect_id
        and not product_effect_id.endswith(".unified")
        and product_effect_id.count(".authored-baseline") <= 1,
        f"candidate source is not a bounded product Effect ID: {product_effect_id}",
    )
    candidate_base = product_effect_id.replace(".authored-baseline", "")
    candidate_id = f"{candidate_base}.unified"
    return (
        candidate_id,
        f"Data/Effects/Authored/{candidate_id}.effect.json",
    )


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise ContractError(message)


def _require_dict(value: Any, label: str) -> dict[str, Any]:
    _require(isinstance(value, dict), f"{label} must be an object")
    return value


def _require_list(value: Any, label: str) -> list[Any]:
    _require(isinstance(value, list), f"{label} must be an array")
    return value


def _require_string(value: Any, label: str) -> str:
    _require(isinstance(value, str) and bool(value), f"{label} must be a string")
    return value


def _require_int(value: Any, label: str, minimum: int = 0) -> int:
    _require(
        isinstance(value, int) and not isinstance(value, bool) and value >= minimum,
        f"{label} must be an integer >= {minimum}",
    )
    return value


def _require_sha256(value: Any, label: str) -> str:
    text = _require_string(value, label)
    _require(bool(SHA256_RE.fullmatch(text)), f"{label} must be lowercase SHA-256")
    return text


def _require_stable_id(value: Any, label: str) -> str:
    text = _require_string(value, label)
    _require(bool(STABLE_ID_RE.fullmatch(text)), f"{label} is not a stable ID")
    return text


def _require_exact_keys(
    value: dict[str, Any], expected: Iterable[str], label: str
) -> None:
    expected_set = set(expected)
    actual = set(value)
    _require(
        actual == expected_set,
        f"{label} fields mismatch; missing={sorted(expected_set - actual)}, "
        f"extra={sorted(actual - expected_set)}",
    )


def _safe_relative_path(value: Any, label: str) -> str:
    text = _require_string(value, label)
    _require(
        "\\" not in text and ":" not in text and not text.startswith("/"),
        f"{label} must be repository-relative POSIX: {text}",
    )
    path = PurePosixPath(text)
    _require(
        all(part not in ("", ".", "..") for part in path.parts),
        f"{label} contains an unsafe segment: {text}",
    )
    return text


def _repository_file(repository_root: Path, relative: str, label: str) -> Path:
    safe = _safe_relative_path(relative, label)
    resolved_root = repository_root.resolve()
    resolved = (resolved_root / Path(*PurePosixPath(safe).parts)).resolve()
    try:
        resolved.relative_to(resolved_root)
    except ValueError as error:
        raise ContractError(f"{label} escaped repository root: {safe}") from error
    _require(resolved.is_file(), f"{label} does not exist: {safe}")
    return resolved


def _repository_path(repository_root: Path, relative: str, label: str) -> Path:
    safe = _safe_relative_path(relative, label)
    resolved_root = repository_root.resolve()
    resolved = (resolved_root / Path(*PurePosixPath(safe).parts)).resolve()
    try:
        resolved.relative_to(resolved_root)
    except ValueError as error:
        raise ContractError(f"{label} escaped repository root: {safe}") from error
    return resolved


def _artifact_reference(
    repository_root: Path,
    relative: str,
    *,
    expected_raw_sha256: str | None = None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    path = _repository_file(repository_root, relative, "artifact path")
    document = load_json(path)
    actual_raw = raw_sha256(path)
    if expected_raw_sha256 is not None:
        _require_sha256(expected_raw_sha256, f"{relative} expected SHA")
        _require(
            actual_raw == expected_raw_sha256,
            f"stale source/receipt SHA for {relative}: "
            f"expected {expected_raw_sha256}, got {actual_raw}",
        )
    return (
        {
            "path": relative,
            "rawSha256": actual_raw,
            "canonicalJsonSha256": canonical_json_sha256(document),
        },
        document,
    )


def _validate_seal(document: dict[str, Any], field: str, label: str) -> None:
    expected = _require_sha256(document.get(field), f"{label}.{field}")
    staged = copy.deepcopy(document)
    staged.pop(field, None)
    _require(
        canonical_json_sha256(staged) == expected,
        f"{label} self seal is stale",
    )


def _validate_record_seal(
    record: dict[str, Any], field: str, label: str
) -> None:
    _validate_seal(record, field, label)


@dataclass
class ArtifactRegistry:
    repository_root: Path

    def __post_init__(self) -> None:
        self._items: dict[str, dict[str, Any]] = {}

    def add(
        self,
        relative: str,
        roles: Iterable[str],
        *,
        expected_raw_sha256: str | None = None,
    ) -> tuple[dict[str, Any], dict[str, Any]]:
        reference, document = _artifact_reference(
            self.repository_root,
            relative,
            expected_raw_sha256=expected_raw_sha256,
        )
        role_set = {_require_stable_id(role, "artifact role") for role in roles}
        _require(role_set, f"artifact roles are empty: {relative}")
        existing = self._items.get(relative)
        if existing is None:
            self._items[relative] = {
                **reference,
                "roles": sorted(role_set),
            }
        else:
            _require(
                existing["rawSha256"] == reference["rawSha256"]
                and existing["canonicalJsonSha256"]
                == reference["canonicalJsonSha256"],
                f"artifact identity changed during build: {relative}",
            )
            existing["roles"] = sorted(set(existing["roles"]) | role_set)
        return copy.deepcopy(reference), document

    def items(self) -> list[dict[str, Any]]:
        return [copy.deepcopy(self._items[key]) for key in sorted(self._items)]


def _source_artifact_bundle(
    repository_root: Path,
    registry: ArtifactRegistry,
    source_artifact: dict[str, Any],
    stage_label: str,
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    source_artifact = _require_dict(source_artifact, f"{stage_label}.sourceArtifact")
    bundle: dict[str, Any] = {}
    loaded: dict[str, dict[str, Any]] = {}
    for key, role in (
        ("sourceReceipt", "SOURCE_RECEIPT"),
        ("importedDocument", "IMPORTED_SOURCE_DOCUMENT"),
        ("conversionReceipt", "ELEMENT_CONVERSION_RECEIPT"),
    ):
        pinned = _require_dict(source_artifact.get(key), f"{stage_label}.{key}")
        relative = _safe_relative_path(
            pinned.get("path"), f"{stage_label}.{key}.path"
        )
        expected = _require_sha256(
            pinned.get("sha256"), f"{stage_label}.{key}.sha256"
        )
        reference, document = registry.add(
            relative, (role,), expected_raw_sha256=expected
        )
        bundle[key] = reference
        loaded[key] = document

    source_receipt = loaded["sourceReceipt"]
    imported_document = loaded["importedDocument"]
    conversion_receipt = loaded["conversionReceipt"]
    _require(
        source_receipt.get("schema") == SOURCE_RECEIPT_SCHEMA
        and source_receipt.get("version") == 1,
        f"{stage_label} source receipt schema/version changed",
    )
    _require(
        imported_document.get("schema") == AUTHORING_SCHEMA
        and isinstance(imported_document.get("version"), int),
        f"{stage_label} imported document schema/version changed",
    )
    imported_effect_id = _require_stable_id(
        imported_document.get("effectAssetId"),
        f"{stage_label}.importedDocument.effectAssetId",
    )
    pinned_imported_effect_id = source_artifact["importedDocument"].get(
        "effectAssetId"
    )
    _require(
        imported_effect_id == pinned_imported_effect_id,
        f"{stage_label} imported Effect identity changed",
    )
    bundle["importedDocument"]["effectAssetId"] = imported_effect_id
    _require(
        conversion_receipt.get("schema") == CONVERSION_RECEIPT_SCHEMA
        and conversion_receipt.get("schemaVersion") == 1,
        f"{stage_label} conversion receipt schema/version changed",
    )
    _require(
        conversion_receipt.get("documentEffectAssetId") == imported_effect_id,
        f"{stage_label} conversion receipt targets another imported Effect",
    )
    conversion_ids: set[str] = set()
    for index, conversion in enumerate(
        _require_list(
            conversion_receipt.get("elementConversions"),
            f"{stage_label}.elementConversions",
        )
    ):
        conversion = _require_dict(
            conversion, f"{stage_label}.elementConversions[{index}]"
        )
        for element_id in _require_list(
            conversion.get("elementIds"),
            f"{stage_label}.elementConversions[{index}].elementIds",
        ):
            stable = _require_stable_id(
                element_id,
                f"{stage_label}.elementConversions[{index}].elementId",
            )
            _require(
                stable not in conversion_ids,
                f"duplicate converted source Element ID: {stable}",
            )
            conversion_ids.add(stable)
    return bundle, loaded, {"elementIds": conversion_ids}


def _material_disposition(
    target_element: dict[str, Any], *, canary: bool
) -> dict[str, Any]:
    material = _require_dict(target_element.get("material"), "target material")
    execution = material.get("execution")
    if isinstance(execution, dict) and execution.get("enabled") is True:
        return {
            "kind": "TYPED_EXECUTION",
            "executionSnapshotSha256": canonical_json_sha256(execution),
        }

    source_profile = material.get("sourceProfile")
    blocked_profile_status: str | None = None
    if isinstance(source_profile, dict) and source_profile.get("enabled") is True:
        profile_id = _require_stable_id(
            source_profile.get("profileId"), "source material profileId"
        )
        runtime_profile = _require_stable_id(
            source_profile.get("runtimeShaderProfileId"),
            "source material runtimeShaderProfileId",
        )
        admission = _require_string(
            source_profile.get("productAdmissionStatus"),
            "source material productAdmissionStatus",
        )
        if admission.startswith("ADMITTED_"):
            return {
                "kind": "ADMITTED_SOURCE_PROFILE",
                "sourceProfileSha256": canonical_json_sha256(source_profile),
                "profileId": profile_id,
                "runtimeShaderProfileId": runtime_profile,
            }
        blocked_profile_status = admission

    blockers = [
        "SOURCE_MATERIAL_PROFILE_NOT_ADMITTED",
        "TYPED_MATERIAL_EXECUTION_NOT_ADMITTED",
    ]
    if blocked_profile_status is not None:
        blockers.append("SOURCE_MATERIAL_PROFILE_EXPLICITLY_BLOCKED")
    if canary:
        blockers.append("WARLORD_CANARY_MATERIAL_FAIL_CLOSED")
    return {"kind": "FAIL_CLOSED", "blockers": sorted(blockers)}


def _carrier_disposition(family: str, *, supplemental: bool) -> dict[str, Any]:
    if supplemental:
        return {
            "kind": "SUPPLEMENTAL_ADAPTER_PRESERVE",
            "blockers": ["GENERIC_PARTICLE_IMPORT_NOT_APPLICABLE"],
        }
    if family in {"MESH_PARTICLE", "SPRITE_PARTICLE"}:
        return {
            "kind": "GENERIC_PARTICLE_IMPORT_CANDIDATE",
            "blockers": ["CXX_GENERIC_CODEC_ELEMENT_VALIDATION_REQUIRED"],
        }
    _require(family == "CASCADE_RIBBON", f"unsupported selected family: {family}")
    return {
        "kind": "FAMILY_ADAPTER_REQUIRED",
        "blockers": ["GENERIC_PARTICLE_IMPORT_UNSUPPORTED_RIBBON"],
    }


def _element_plan_id(
    stage_key: str, target_element_id: str, source_record_id: str
) -> str:
    identity = canonical_json_sha256(
        [stage_key, target_element_id, source_record_id]
    )
    return f"element-import.{identity}"


def _make_target_identity(
    stage_target: dict[str, Any],
    target_element: dict[str, Any],
    baseline_by_id: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    element_id = _require_stable_id(target_element.get("id"), "target Element ID")
    baseline = baseline_by_id.get(element_id)
    return {
        "documentPath": stage_target["path"],
        "effectAssetId": stage_target["effectAssetId"],
        "elementId": element_id,
        "groupId": _require_stable_id(
            target_element.get("groupId"), f"{element_id}.groupId"
        ),
        "displayName": _require_string(
            target_element.get("displayName"), f"{element_id}.displayName"
        ),
        "blueprintElementCanonicalSha256": canonical_json_sha256(target_element),
        "baselineElementCanonicalSha256": (
            canonical_json_sha256(baseline) if baseline is not None else None
        ),
        "targetDetailCanonicalSha256": canonical_json_sha256(
            _require_dict(target_element.get("detail"), f"{element_id}.detail")
        ),
    }


def _make_imported_element_plan(
    *,
    stage_key: str,
    stage_target: dict[str, Any],
    source_bundle: dict[str, Any],
    source_document: dict[str, Any],
    source_element: dict[str, Any],
    target_element: dict[str, Any],
    baseline_by_id: dict[str, dict[str, Any]],
    family: str,
    selection_record_id: str,
    selection_record_sha256: str,
    source_event_id: str | None,
    carrier: dict[str, Any],
    canary: bool,
) -> dict[str, Any]:
    source_element_id = _require_stable_id(
        source_element.get("id"), "source Element ID"
    )
    source_recipe = _require_dict(
        source_element.get("sourceRecipe"), f"{source_element_id}.sourceRecipe"
    )
    _require(
        source_element.get("kind") == "particle"
        and source_recipe.get("enabled") is True,
        f"imported Element is not a source Particle recipe: {source_element_id}",
    )
    target_identity = _make_target_identity(
        stage_target, target_element, baseline_by_id
    )
    plan_id = _element_plan_id(
        stage_key, target_identity["elementId"], source_element_id
    )
    return {
        "planId": plan_id,
        "stageKey": stage_key,
        "source": {
            "kind": "IMPORTED_ELEMENT",
            "documentPath": source_bundle["importedDocument"]["path"],
            "documentRawSha256": source_bundle["importedDocument"]["rawSha256"],
            "effectAssetId": source_document["effectAssetId"],
            "elementId": source_element_id,
            "elementCanonicalSha256": canonical_json_sha256(source_element),
            "sourceRecipeCanonicalSha256": canonical_json_sha256(source_recipe),
            "sourceDetailCanonicalSha256": canonical_json_sha256(
                _require_dict(
                    source_element.get("detail"), f"{source_element_id}.detail"
                )
            ),
            "sourceAttachmentCanonicalSha256": canonical_json_sha256(
                _require_dict(
                    source_element.get("actionCueAttachment"),
                    f"{source_element_id}.actionCueAttachment",
                )
            ),
            "sourceEventId": source_event_id,
        },
        "target": target_identity,
        "selectionIdentity": {
            "recordId": selection_record_id,
            "recordSha256": selection_record_sha256,
        },
        "family": family,
        "carrierDisposition": carrier,
        "materialDisposition": _material_disposition(
            target_element, canary=canary
        ),
        "productMutation": False,
        "visualApproval": False,
    }


def _make_supplemental_plan(
    *,
    stage_key: str,
    stage_target: dict[str, Any],
    supplemental: dict[str, Any],
    target_element: dict[str, Any],
    baseline_by_id: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    selector = _require_dict(
        supplemental.get("selector"), "supplemental.selector"
    )
    record_id = _require_stable_id(
        selector.get("occurrenceId"), "supplemental occurrenceId"
    )
    record_sha = _require_sha256(
        supplemental.get("rowSha256"), "supplemental rowSha256"
    )
    target_identity = _make_target_identity(
        stage_target, target_element, baseline_by_id
    )
    plan_id = _element_plan_id(
        stage_key, target_identity["elementId"], record_id
    )
    source_identity = _require_dict(
        supplemental.get("sourceIdentity"), "supplemental.sourceIdentity"
    )
    return {
        "planId": plan_id,
        "stageKey": stage_key,
        "source": {
            "kind": "VISUAL_PROGRAM_SUPPLEMENTAL",
            "recordId": _require_stable_id(
                source_identity.get("sourceRecordId"),
                "supplemental sourceRecordId",
            ),
            "recordCanonicalSha256": _require_sha256(
                source_identity.get("sourceRecordSha256"),
                "supplemental sourceRecordSha256",
            ),
            "sourceEventId": _require_stable_id(
                _require_dict(
                    supplemental.get("schedule"), "supplemental.schedule"
                ).get("sourceEventId"),
                "supplemental sourceEventId",
            ),
        },
        "target": target_identity,
        "selectionIdentity": {
            "recordId": record_id,
            "recordSha256": record_sha,
        },
        "family": _require_stable_id(
            supplemental.get("family"), "supplemental family"
        ),
        "carrierDisposition": _carrier_disposition(
            _require_string(supplemental.get("family"), "supplemental family"),
            supplemental=True,
        ),
        "materialDisposition": _material_disposition(
            target_element, canary=False
        ),
        "productMutation": False,
        "visualApproval": False,
    }


def _load_manifests(
    repository_root: Path, registry: ArtifactRegistry
) -> dict[str, tuple[dict[str, Any], dict[str, Any]]]:
    result: dict[str, tuple[dict[str, Any], dict[str, Any]]] = {}
    for character_class, relative in CLASS_MANIFESTS.items():
        reference, manifest = registry.add(relative, ("SOURCE_STAGE_MANIFEST",))
        _require(
            manifest.get("schema") == SOURCE_MANIFEST_SCHEMA
            and manifest.get("version") == 1
            and manifest.get("characterClass") == character_class,
            f"{character_class} source manifest schema/class changed",
        )
        result[character_class] = (reference, manifest)
    return result


def _find_manifest_stage(
    manifest: dict[str, Any], skill_id: int, stage_index: int
) -> tuple[dict[str, Any], dict[str, Any]]:
    skills = [
        item
        for item in _require_list(manifest.get("skills"), "manifest.skills")
        if isinstance(item, dict) and item.get("productSkillId") == skill_id
    ]
    _require(len(skills) == 1, f"manifest requires exactly one skill {skill_id}")
    stages = [
        item
        for item in _require_list(skills[0].get("stages"), "manifest skill.stages")
        if isinstance(item, dict) and item.get("stageIndex") == stage_index
    ]
    _require(
        len(stages) == 1,
        f"manifest requires exactly one skill {skill_id} stage {stage_index}",
    )
    return skills[0], stages[0]


def _target_baseline(
    repository_root: Path,
    registry: ArtifactRegistry,
    *,
    relative: str,
    expected_effect_id: str,
    expected_raw_sha256: str | None,
    expected_canonical_sha256: str | None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    reference, document = registry.add(
        relative,
        ("TARGET_AUTHORED_BASELINE",),
        expected_raw_sha256=expected_raw_sha256,
    )
    _require(
        document.get("schema") == AUTHORING_SCHEMA
        and document.get("effectAssetId") == expected_effect_id,
        f"target baseline identity changed: {relative}",
    )
    if expected_canonical_sha256 is not None:
        _require(
            reference["canonicalJsonSha256"] == expected_canonical_sha256,
            f"target baseline canonical SHA changed: {relative}",
        )
    elements = _require_list(document.get("elements"), f"{relative}.elements")
    element_ids = [
        _require_stable_id(item.get("id"), f"{relative}.element.id")
        for item in elements
        if isinstance(item, dict)
    ]
    _require(
        len(element_ids) == len(elements) == len(set(element_ids)),
        f"target baseline contains duplicate/malformed Element IDs: {relative}",
    )

    candidate_effect_id, candidate_relative = _unified_candidate_identity(
        expected_effect_id
    )
    candidate_path = _repository_path(
        repository_root, candidate_relative, "unified candidate path"
    )
    if candidate_path.exists():
        candidate_reference, candidate_document = registry.add(
            candidate_relative,
            ("UNIFIED_CANDIDATE_BASELINE",),
        )
        _require(
            candidate_document.get("schema") == AUTHORING_SCHEMA
            and candidate_document.get("version") == 13
            and candidate_document.get("effectAssetId") == candidate_effect_id,
            f"unified candidate baseline identity changed: {candidate_relative}",
        )
        candidate_baseline = {
            "policy": "EXPECTED_EXACT_OR_REFUSE",
            "expectedRawSha256": candidate_reference["rawSha256"],
            "expectedCanonicalJsonSha256": candidate_reference[
                "canonicalJsonSha256"
            ],
            "authoringVersion": 13,
        }
    else:
        candidate_baseline = {
            "policy": "MUST_NOT_EXIST",
            "expectedRawSha256": None,
            "expectedCanonicalJsonSha256": None,
            "authoringVersion": None,
        }
    return (
        {
            "path": candidate_relative,
            "effectAssetId": candidate_effect_id,
            "requiredOutputVersion": 13,
            "candidateBaseline": candidate_baseline,
            "legacyRollbackBaseline": {
                "path": relative,
                "effectAssetId": expected_effect_id,
                "rawSha256": reference["rawSha256"],
                "canonicalJsonSha256": reference["canonicalJsonSha256"],
                "authoringVersion": _require_int(
                    document.get("version"), f"{relative}.version", 1
                ),
                "policy": "IMMUTABLE_LEGACY_ROLLBACK_EXACT",
            },
        },
        document,
    )


def _build_track_a_stages(
    repository_root: Path,
    registry: ArtifactRegistry,
    visual_reference: dict[str, Any],
    visual_runtime: dict[str, Any],
    source_admission: dict[str, Any],
    manifests: dict[str, tuple[dict[str, Any], dict[str, Any]]],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], int]:
    admission_stages = {
        item["productEffectAssetId"]: item
        for item in _require_list(source_admission.get("stages"), "admission.stages")
        if isinstance(item, dict)
    }
    source_rows = {
        item["sourceRowId"]: item
        for item in _require_list(
            source_admission.get("sourceRows"), "admission.sourceRows"
        )
        if isinstance(item, dict)
    }
    _require(
        len(admission_stages) == 12 and len(source_rows) == 133,
        "three-class BA source-admission denominator changed",
    )
    programs = [
        item
        for item in _require_list(visual_runtime.get("programs"), "runtime.programs")
        if isinstance(item, dict)
        and item.get("projectionKind") == TRACK_A_PROJECTION_KIND
    ]
    _require(len(programs) == 12, "Track A executable program denominator changed")
    _require(
        {item.get("effectAssetId") for item in programs} == set(admission_stages),
        "Track A programs no longer match the twelve BA admission stages",
    )

    stages: list[dict[str, Any]] = []
    plans: list[dict[str, Any]] = []
    excluded_count = 0
    ordered = sorted(
        programs,
        key=lambda item: (
            CLASS_ORDER[admission_stages[item["effectAssetId"]]["characterClass"]],
            admission_stages[item["effectAssetId"]]["stageIndex"],
        ),
    )
    for program in ordered:
        effect_id = _require_stable_id(
            program.get("effectAssetId"), "visual program effectAssetId"
        )
        _validate_record_seal(program, "programSha256", effect_id)
        admission_stage = admission_stages[effect_id]
        character_class = _require_stable_id(
            admission_stage.get("characterClass"), f"{effect_id}.characterClass"
        )
        _require(
            character_class in {"ARTIST", "DIMENSIONMASTER", "LANCE_MASTER"},
            f"unexpected Track A class: {character_class}",
        )
        skill_id = _require_int(admission_stage.get("skillId"), f"{effect_id}.skillId", 1)
        stage_index = _require_int(
            admission_stage.get("stageIndex"), f"{effect_id}.stageIndex"
        )
        match = PRODUCT_EFFECT_RE.fullmatch(effect_id)
        _require(
            match is not None
            and int(match.group("skill_id")) == skill_id
            and int(match.group("stage")) == stage_index + 1,
            f"Track A program/stage identity mismatch: {effect_id}",
        )
        manifest_reference, manifest = manifests[character_class]
        manifest_skill, manifest_stage = _find_manifest_stage(
            manifest, skill_id, stage_index
        )
        clips = _require_list(manifest_stage.get("clips"), f"{effect_id}.clips")
        _require(len(clips) == 1, f"{effect_id} requires one BA clip")
        clip = _require_stable_id(clips[0].get("clip"), f"{effect_id}.clip")
        _require(
            clip == admission_stage.get("clip")
            and manifest_stage.get("stageId") == admission_stage.get("stageId"),
            f"{effect_id} Track A program-to-skill/stage mapping changed",
        )
        artifacts = _require_list(
            manifest_stage.get("sourceArtifacts"), f"{effect_id}.sourceArtifacts"
        )
        _require(len(artifacts) == 1, f"{effect_id} source artifact is ambiguous")
        source_bundle, loaded, conversion = _source_artifact_bundle(
            repository_root, registry, artifacts[0], effect_id
        )
        source_document = loaded["importedDocument"]
        source_by_id = {
            item["id"]: item
            for item in _require_list(
                source_document.get("elements"), f"{effect_id}.source.elements"
            )
            if isinstance(item, dict)
        }
        current_product = _require_dict(
            admission_stage.get("currentProduct"), f"{effect_id}.currentProduct"
        )
        base_identity = _require_dict(
            program.get("baseDocumentIdentity"), f"{effect_id}.baseDocumentIdentity"
        )
        target, baseline_document = _target_baseline(
            repository_root,
            registry,
            relative=_safe_relative_path(
                current_product.get("authoringPath"),
                f"{effect_id}.currentProduct.authoringPath",
            ),
            expected_effect_id=effect_id,
            expected_raw_sha256=_require_sha256(
                current_product.get("authoringRawSha256"),
                f"{effect_id}.authoringRawSha256",
            ),
            expected_canonical_sha256=_require_sha256(
                base_identity.get("canonicalSha256"),
                f"{effect_id}.baseDocumentIdentity.canonicalSha256",
            ),
        )
        _require(
            target["legacyRollbackBaseline"]["rawSha256"]
            == _require_sha256(
                base_identity.get("rawSha256"),
                f"{effect_id}.baseDocumentIdentity.rawSha256",
            ),
            f"{effect_id} visual program baseline raw SHA changed",
        )
        projected = _require_dict(
            program.get("projectedDocument"), f"{effect_id}.projectedDocument"
        )
        _require(
            projected.get("schema") == AUTHORING_SCHEMA
            and projected.get("effectAssetId") == effect_id,
            f"{effect_id} projected authoring identity changed",
        )
        projected_sha = canonical_json_sha256(projected)
        _require(
            projected_sha
            == _require_sha256(
                program.get("projectedDocumentSha256"),
                f"{effect_id}.projectedDocumentSha256",
            ),
            f"{effect_id} projected document SHA changed",
        )
        target["blueprint"] = {
            "kind": "VISUAL_PROGRAM_PROJECTED_DOCUMENT",
            "authoringVersion": _require_int(
                projected.get("version"), f"{effect_id}.projected.version", 1
            ),
            "canonicalJsonSha256": projected_sha,
            "typedCodecSha256": _require_sha256(
                program.get("projectedDocumentTypedCodecSha256"),
                f"{effect_id}.projectedDocumentTypedCodecSha256",
            ),
        }
        baseline_by_id = {
            item["id"]: item for item in baseline_document["elements"]
        }
        projected_by_id = {
            item["id"]: item
            for item in _require_list(
                projected.get("elements"), f"{effect_id}.projected.elements"
            )
            if isinstance(item, dict)
        }
        _require(
            len(projected_by_id) == len(projected["elements"]),
            f"{effect_id} projected document has duplicate Element IDs",
        )

        stage_key = (
            f"{character_class.lower()}.skill.{skill_id}.stage.{stage_index}"
        )
        stage_plan_ids: list[str] = []
        selected_target_ids: set[str] = set()
        for row in _require_list(program.get("visualRows"), f"{effect_id}.visualRows"):
            row = _require_dict(row, f"{effect_id}.visualRow")
            _validate_record_seal(row, "rowSha256", f"{effect_id}.visualRow")
            disposition = _require_string(
                row.get("disposition"), f"{effect_id}.visualRow.disposition"
            )
            if disposition != "ADMITTED_BOUNDED":
                _require(
                    disposition == "FAIL_CLOSED"
                    and row.get("targetIdentity") is None,
                    f"{effect_id} has an unknown visual row disposition",
                )
                excluded_count += 1
                continue
            source_identity = _require_dict(
                row.get("sourceIdentity"), f"{effect_id}.sourceIdentity"
            )
            source_row_id = _require_stable_id(
                source_identity.get("sourceRowId"), f"{effect_id}.sourceRowId"
            )
            source_row = source_rows.get(source_row_id)
            _require(source_row is not None, f"missing source admission row: {source_row_id}")
            _require(
                source_row.get("characterClass") == character_class
                and source_row.get("skillId") == skill_id
                and source_row.get("stageIndex") == stage_index
                and source_row.get("productEffectAssetId") == effect_id,
                f"cross-stage source admission join: {source_row_id}",
            )
            source_element_id = _require_stable_id(
                source_row.get("sourceElementId"), f"{source_row_id}.sourceElementId"
            )
            source_element = source_by_id.get(source_element_id)
            _require(source_element is not None, f"missing imported source Element: {source_element_id}")
            _require(
                source_element_id in conversion["elementIds"],
                f"conversion receipt omitted source Element: {source_element_id}",
            )
            source_element_sha = canonical_json_sha256(source_element)
            _require(
                source_element_sha
                == _require_sha256(
                    source_row.get("sourceElementSha256"),
                    f"{source_row_id}.sourceElementSha256",
                )
                == _require_sha256(
                    source_identity.get("sourceRecordSha256"),
                    f"{source_row_id}.sourceRecordSha256",
                ),
                f"stale imported source Element SHA: {source_element_id}",
            )
            row_family = _require_stable_id(
                row.get("family"), f"{effect_id}.family"
            )
            recipe_sha = canonical_json_sha256(source_element["sourceRecipe"])
            _require(
                recipe_sha
                == _require_sha256(
                    source_row["sourceRecipe"].get("sourceRecipeSha256"),
                    f"{source_row_id}.admissionSourceRecipeSha256",
                ),
                f"stale sourceRecipe SHA: {source_element_id}",
            )
            visual_recipe_sha = _require_sha256(
                source_identity.get("sourceRecipeSha256"),
                f"{source_row_id}.sourceRecipeSha256",
            )
            if row_family != "CASCADE_RIBBON":
                _require(
                    visual_recipe_sha == recipe_sha,
                    f"visual program sourceRecipe SHA changed: {source_element_id}",
                )
            target_identity = _require_dict(
                row.get("targetIdentity"), f"{effect_id}.targetIdentity"
            )
            target_element_id = _require_stable_id(
                target_identity.get("targetElementId"), f"{effect_id}.targetElementId"
            )
            _require(
                target_element_id not in selected_target_ids,
                f"duplicate selected target Element: {effect_id}/{target_element_id}",
            )
            selected_target_ids.add(target_element_id)
            target_element = projected_by_id.get(target_element_id)
            _require(target_element is not None, f"missing projected target Element: {target_element_id}")
            if row_family == "CASCADE_RIBBON":
                _require(
                    canonical_json_sha256(
                        _require_dict(
                            target_element.get("sourceRecipe"),
                            f"{target_element_id}.sourceRecipe",
                        )
                    )
                    == visual_recipe_sha,
                    f"ribbon adapter sourceRecipe SHA changed: {target_element_id}",
                )
            _require_sha256(
                target_identity.get("targetRecordSha256"),
                f"{effect_id}.{target_element_id}.targetRecordSha256",
            )
            selector = _require_dict(row.get("selector"), f"{effect_id}.selector")
            plan = _make_imported_element_plan(
                stage_key=stage_key,
                stage_target=target,
                source_bundle=source_bundle,
                source_document=source_document,
                source_element=source_element,
                target_element=target_element,
                baseline_by_id=baseline_by_id,
                family=row_family,
                selection_record_id=_require_stable_id(
                    selector.get("occurrenceId"), f"{effect_id}.occurrenceId"
                ),
                selection_record_sha256=_require_sha256(
                    row.get("rowSha256"), f"{effect_id}.rowSha256"
                ),
                source_event_id=source_row.get("sourceEventId"),
                carrier=_carrier_disposition(
                    row_family,
                    supplemental=False,
                ),
                canary=False,
            )
            stage_plan_ids.append(plan["planId"])
            plans.append(plan)

        for supplemental in _require_list(
            program.get("supplementalElements"), f"{effect_id}.supplementalElements"
        ):
            supplemental = _require_dict(supplemental, f"{effect_id}.supplemental")
            _validate_record_seal(
                supplemental, "rowSha256", f"{effect_id}.supplemental"
            )
            _require(
                supplemental.get("disposition") == "ADMITTED_BOUNDED",
                f"{effect_id} supplemental Element is not admitted",
            )
            target_identity = _require_dict(
                supplemental.get("targetIdentity"),
                f"{effect_id}.supplemental.targetIdentity",
            )
            target_element_id = _require_stable_id(
                target_identity.get("targetElementId"),
                f"{effect_id}.supplemental.targetElementId",
            )
            _require(
                target_element_id not in selected_target_ids,
                f"duplicate supplemental target Element: {effect_id}/{target_element_id}",
            )
            selected_target_ids.add(target_element_id)
            target_element = projected_by_id.get(target_element_id)
            _require(target_element is not None, f"missing supplemental target: {target_element_id}")
            _require_sha256(
                target_identity.get("targetRecordSha256"),
                f"{effect_id}.{target_element_id}.targetRecordSha256",
            )
            plan = _make_supplemental_plan(
                stage_key=stage_key,
                stage_target=target,
                supplemental=supplemental,
                target_element=target_element,
                baseline_by_id=baseline_by_id,
            )
            stage_plan_ids.append(plan["planId"])
            plans.append(plan)

        _require(
            set(projected_by_id) == selected_target_ids,
            f"{effect_id} projected document contains unplanned Elements",
        )
        stage_plans = [item for item in plans if item["stageKey"] == stage_key]
        material_counts = Counter(
            item["materialDisposition"]["kind"] for item in stage_plans
        )
        carrier_counts = Counter(
            item["carrierDisposition"]["kind"] for item in stage_plans
        )
        stages.append(
            {
                "stageKey": stage_key,
                "mode": TRACK_A_SELECTION_KIND,
                "characterClass": character_class,
                "animationAssetId": _require_stable_id(
                    admission_stage.get("animationAssetId"),
                    f"{effect_id}.animationAssetId",
                ),
                "skillId": skill_id,
                "inputSlot": "LMB",
                "skillKind": "COMBO",
                "stageIndex": stage_index,
                "stageId": _require_stable_id(
                    admission_stage.get("stageId"), f"{effect_id}.stageId"
                ),
                "clip": clip,
                "sourceArtifacts": {
                    "manifest": manifest_reference,
                    **source_bundle,
                },
                "selection": {
                    "kind": TRACK_A_SELECTION_KIND,
                    "artifact": visual_reference,
                    "recordId": effect_id,
                    "recordSha256": _require_sha256(
                        program.get("programSha256"), f"{effect_id}.programSha256"
                    ),
                    "sourceSelectionReceipt": None,
                },
                "target": target,
                "elementPlanIds": stage_plan_ids,
                "elementPlanCount": len(stage_plan_ids),
                "materialDispositionCounts": dict(sorted(material_counts.items())),
                "carrierDispositionCounts": dict(sorted(carrier_counts.items())),
                "productMutation": False,
                "visualApproval": False,
            }
        )
    return stages, plans, excluded_count


def _build_warlord_canary_stage(
    repository_root: Path,
    registry: ArtifactRegistry,
    visual_reference: dict[str, Any],
    visual_runtime: dict[str, Any],
    manifests: dict[str, tuple[dict[str, Any], dict[str, Any]]],
) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    canaries = [
        item
        for item in _require_list(
            visual_runtime.get("extensionCanaries"), "runtime.extensionCanaries"
        )
        if isinstance(item, dict) and item.get("canaryId") == WARLORD_CANARY_ID
    ]
    _require(len(canaries) == 1, "Warlord extension canary identity changed")
    canary = canaries[0]
    _validate_record_seal(canary, "canarySha256", WARLORD_CANARY_ID)
    selector = _require_dict(canary.get("selector"), "Warlord canary selector")
    _require(
        selector.get("effectAssetId") == WARLORD_EFFECT_ASSET_ID
        and canary.get("disposition") == "FAIL_CLOSED"
        and canary.get("productCountContribution") is False,
        "Warlord canary is no longer fail-closed/non-product",
    )

    manifest_reference, manifest = manifests["WARLORD"]
    manifest_skill, manifest_stage = _find_manifest_stage(
        manifest, WARLORD_SKILL_ID, WARLORD_STAGE_INDEX
    )
    clips = _require_list(manifest_stage.get("clips"), "Warlord canary clips")
    _require(len(clips) == 1, "Warlord canary requires one BA clip")
    clip = _require_stable_id(clips[0].get("clip"), "Warlord canary clip")
    artifacts = _require_list(
        manifest_stage.get("sourceArtifacts"), "Warlord canary sourceArtifacts"
    )
    _require(len(artifacts) == 1, "Warlord canary source artifact is ambiguous")
    source_bundle, loaded, conversion = _source_artifact_bundle(
        repository_root, registry, artifacts[0], WARLORD_EFFECT_ASSET_ID
    )
    source_document = loaded["importedDocument"]
    source_by_id = {
        item["id"]: item
        for item in _require_list(
            source_document.get("elements"), "Warlord imported elements"
        )
        if isinstance(item, dict)
    }

    selection_reference, selection_receipt = registry.add(
        WARLORD_SELECTION_RECEIPT_RELATIVE_PATH,
        ("WARLORD_CANARY_SELECTION_RECEIPT",),
    )
    _require(
        selection_receipt.get("schema")
        == "lostark.effect-authored-approximation-receipt"
        and selection_receipt.get("version") == 1
        and selection_receipt.get("targetEffectAssetId")
        == WARLORD_EFFECT_ASSET_ID
        and selection_receipt.get("productSkillId") == WARLORD_SKILL_ID
        and selection_receipt.get("stageIndex") == WARLORD_STAGE_INDEX,
        "Warlord canary selection receipt identity changed",
    )
    target_relative = (
        "Data/Effects/Authored/"
        f"{WARLORD_EFFECT_ASSET_ID}.effect.json"
    )
    expected_receipt_target = target_relative.removeprefix("Data/")
    _require(
        selection_receipt.get("targetAuthoringPath") == expected_receipt_target,
        "Warlord selection receipt target path changed",
    )
    output = _require_dict(selection_receipt.get("output"), "Warlord receipt.output")
    target, baseline_document = _target_baseline(
        repository_root,
        registry,
        relative=target_relative,
        expected_effect_id=WARLORD_EFFECT_ASSET_ID,
        expected_raw_sha256=None,
        expected_canonical_sha256=_require_sha256(
            output.get("documentSha256"), "Warlord output.documentSha256"
        ),
    )
    target["blueprint"] = {
        "kind": "CURRENT_AUTHORED_BASELINE",
        "authoringVersion": target["legacyRollbackBaseline"]["authoringVersion"],
        "canonicalJsonSha256": target["legacyRollbackBaseline"][
            "canonicalJsonSha256"
        ],
        "typedCodecSha256": None,
    }
    baseline_by_id = {
        item["id"]: item for item in baseline_document["elements"]
    }
    occurrences = _require_list(
        selection_receipt.get("occurrences"), "Warlord receipt.occurrences"
    )
    _require(len(occurrences) == 1, "Warlord canary occurrence denominator changed")
    occurrence = _require_dict(occurrences[0], "Warlord receipt.occurrence")
    _require(
        occurrence.get("sourceEventId") == WARLORD_SOURCE_EVENT_ID
        and occurrence.get("status") == "selected",
        "Warlord canary source event/selection changed",
    )
    selected_ids = [
        _require_stable_id(item, "Warlord selected source Element ID")
        for item in _require_list(
            occurrence.get("selectedSourceElementIds"),
            "Warlord selectedSourceElementIds",
        )
    ]
    _require(
        len(selected_ids) == 5 and len(set(selected_ids)) == 5,
        "Warlord canary must select exactly five unique source carriers",
    )
    candidate_by_id = {
        item["sourceElementId"]: item
        for item in _require_list(
            occurrence.get("candidates"), "Warlord occurrence.candidates"
        )
        if isinstance(item, dict)
    }
    target_by_source: dict[str, dict[str, Any]] = {}
    for target_element in baseline_document["elements"]:
        source_node = _require_string(
            target_element.get("sourceNode"), "Warlord target sourceNode"
        )
        marker = "|element:"
        _require(marker in source_node, "Warlord target lacks source Element identity")
        source_id = source_node.rsplit(marker, 1)[1]
        _require(
            source_id not in target_by_source,
            f"duplicate Warlord target source identity: {source_id}",
        )
        target_by_source[source_id] = target_element
    _require(
        set(target_by_source) == set(selected_ids),
        "Warlord target baseline no longer matches selected five carriers",
    )

    stage_key = "warlord.skill.17000.stage.0"
    plans: list[dict[str, Any]] = []
    for source_id in selected_ids:
        source_element = source_by_id.get(source_id)
        candidate = candidate_by_id.get(source_id)
        _require(
            source_element is not None and candidate is not None,
            f"missing Warlord selected source evidence: {source_id}",
        )
        _require(
            source_id in conversion["elementIds"],
            f"Warlord conversion receipt omitted source Element: {source_id}",
        )
        source_sha = canonical_json_sha256(source_element)
        _require(
            source_sha
            == _require_sha256(
                candidate.get("sourceElementSha256"),
                f"Warlord {source_id}.sourceElementSha256",
            ),
            f"stale Warlord source Element SHA: {source_id}",
        )
        renderer_shape = _require_string(
            source_element["sourceRecipe"].get("rendererShape"),
            f"Warlord {source_id}.rendererShape",
        )
        family = {
            "mesh": "MESH_PARTICLE",
            "sprite": "SPRITE_PARTICLE",
        }.get(renderer_shape)
        _require(family is not None, f"Warlord canary selected non-particle family: {source_id}")
        plan = _make_imported_element_plan(
            stage_key=stage_key,
            stage_target=target,
            source_bundle=source_bundle,
            source_document=source_document,
            source_element=source_element,
            target_element=target_by_source[source_id],
            baseline_by_id=baseline_by_id,
            family=family,
            selection_record_id=source_id,
            selection_record_sha256=source_sha,
            source_event_id=WARLORD_SOURCE_EVENT_ID,
            carrier=_carrier_disposition(family, supplemental=False),
            canary=True,
        )
        _require(
            plan["materialDisposition"]["kind"] == "FAIL_CLOSED",
            "Warlord canary material unexpectedly became admitted",
        )
        plans.append(plan)

    material_counts = Counter(
        item["materialDisposition"]["kind"] for item in plans
    )
    carrier_counts = Counter(
        item["carrierDisposition"]["kind"] for item in plans
    )
    stage = {
        "stageKey": stage_key,
        "mode": WARLORD_SELECTION_KIND,
        "characterClass": "WARLORD",
        "animationAssetId": _require_stable_id(
            manifest.get("animationAssetId"), "Warlord animationAssetId"
        ),
        "skillId": WARLORD_SKILL_ID,
        "inputSlot": _require_stable_id(
            manifest_skill.get("inputSlot"), "Warlord inputSlot"
        ),
        "skillKind": _require_stable_id(
            manifest_skill.get("skillKind"), "Warlord skillKind"
        ),
        "stageIndex": WARLORD_STAGE_INDEX,
        "stageId": _require_stable_id(
            manifest_stage.get("stageId"), "Warlord stageId"
        ),
        "clip": clip,
        "sourceArtifacts": {
            "manifest": manifest_reference,
            **source_bundle,
        },
        "selection": {
            "kind": WARLORD_SELECTION_KIND,
            "artifact": visual_reference,
            "recordId": WARLORD_CANARY_ID,
            "recordSha256": _require_sha256(
                canary.get("canarySha256"), "Warlord canarySha256"
            ),
            "sourceSelectionReceipt": selection_reference,
        },
        "target": target,
        "elementPlanIds": [item["planId"] for item in plans],
        "elementPlanCount": len(plans),
        "materialDispositionCounts": dict(sorted(material_counts.items())),
        "carrierDispositionCounts": dict(sorted(carrier_counts.items())),
        "productMutation": False,
        "visualApproval": False,
    }
    return stage, plans


def _raw_file_identity(
    repository_root: Path, relative: str, label: str
) -> tuple[dict[str, Any], bytes]:
    path = _repository_file(repository_root, relative, label)
    try:
        payload = path.read_bytes()
    except OSError as error:
        raise ContractError(f"cannot read {relative}: {error}") from error
    return (
        {
            "path": relative,
            "rawSha256": hashlib.sha256(payload).hexdigest(),
        },
        payload,
    )


def _normalized_lf_sha256(payload: bytes) -> str:
    normalized = payload.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return hashlib.sha256(normalized).hexdigest()


def _active_effect_reference_count(payload: bytes, effect_id: str) -> int:
    try:
        text = payload.decode("utf-8-sig")
    except UnicodeError as error:
        raise ContractError(f"animevent is not UTF-8: {error}") from error
    expression = re.compile(
        r'payload="' + re.escape(effect_id) + r'"\s+effectref=asset(?:\s|$)'
    )
    return len(expression.findall(text))


def _extended_document_reference(
    reference: dict[str, Any], document: dict[str, Any], *, kind: str | None = None
) -> dict[str, Any]:
    result = {
        **reference,
        "effectAssetId": _require_stable_id(
            document.get("effectAssetId"), "authored Effect identity"
        ),
        "authoringVersion": _require_int(
            document.get("version"), "authored Effect version", 1
        ),
    }
    if kind is not None:
        result = {"kind": kind, **result}
    return result


def _build_legacy_starter_scope(
    repository_root: Path,
    registry: ArtifactRegistry,
    manifests: dict[str, tuple[dict[str, Any], dict[str, Any]]],
    track_a_stages: list[dict[str, Any]],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], dict[str, int]]:
    rollout_reference, rollout = registry.add(
        FOUR_CLASS_ROLLOUT_RELATIVE_PATH,
        ("FOUR_CLASS_PRODUCT_ROLLOUT",),
    )
    _require(
        rollout.get("schema") == FOUR_CLASS_ROLLOUT_SCHEMA
        and rollout.get("version") == 2,
        "four-class rollout schema/version changed",
    )
    catalog_reference, catalog = registry.add(
        EFFECT_CATALOG_RELATIVE_PATH,
        ("EFFECT_CATALOG_MAPPING_EVIDENCE",),
    )
    catalog_rows = _require_list(catalog.get("effects"), "EffectCatalog.effects")
    catalog_by_id: dict[str, list[dict[str, Any]]] = {}
    for index, row in enumerate(catalog_rows):
        row = _require_dict(row, f"EffectCatalog.effects[{index}]")
        effect_id = _require_stable_id(
            row.get("effectAssetId"), f"EffectCatalog.effects[{index}].effectAssetId"
        )
        catalog_by_id.setdefault(effect_id, []).append(row)

    animevent_identity: dict[str, dict[str, Any]] = {}
    animevent_payload: dict[str, bytes] = {}
    for character_class, relative in CLASS_ANIMEVENTS.items():
        identity, payload = _raw_file_identity(
            repository_root, relative, f"{character_class} animevent"
        )
        animevent_identity[character_class] = identity
        animevent_payload[character_class] = payload

    track_a_stage_identities = {
        (row["characterClass"], row["skillId"], row["stageIndex"])
        for row in track_a_stages
    }
    track_a_legacy_effect_ids = {
        row["target"]["legacyRollbackBaseline"]["effectAssetId"]
        for row in track_a_stages
    }
    _require(
        len(track_a_stage_identities) == 13
        and len(track_a_legacy_effect_ids) == 13,
        "Track A seam stage/target identity denominator changed",
    )

    rollout_targets = [
        _require_dict(row, f"rollout.productTargets[{index}]")
        for index, row in enumerate(
            _require_list(rollout.get("productTargets"), "rollout.productTargets")
        )
    ]
    _require(len(rollout_targets) == 101, "rollout product target denominator changed")
    rollout_target_by_id: dict[str, dict[str, Any]] = {}
    for row in rollout_targets:
        effect_id = _require_stable_id(
            row.get("effectAssetId"), "rollout product target Effect ID"
        )
        _require(
            effect_id not in rollout_target_by_id,
            f"duplicate rollout product target: {effect_id}",
        )
        rollout_target_by_id[effect_id] = row
    _require(
        track_a_legacy_effect_ids <= set(rollout_target_by_id),
        "Track A seam target is absent from the four-class rollout",
    )

    legacy_candidates: list[dict[str, Any]] = []
    candidate_by_legacy_id: dict[str, dict[str, Any]] = {}
    for row in rollout_targets:
        legacy_effect_id = row["effectAssetId"]
        if legacy_effect_id in track_a_legacy_effect_ids:
            continue
        character_class = _require_stable_id(
            row.get("characterClass"), f"{legacy_effect_id}.characterClass"
        )
        skill_id = _require_int(
            row.get("productSkillId"), f"{legacy_effect_id}.productSkillId", 1
        )
        stage_index = _require_int(
            row.get("stageIndex"), f"{legacy_effect_id}.stageIndex"
        )
        stage_clip_index = _require_int(
            row.get("stageClipIndex"), f"{legacy_effect_id}.stageClipIndex"
        )
        clip = _require_stable_id(row.get("clip"), f"{legacy_effect_id}.clip")
        _require(character_class in manifests, f"unknown rollout class: {character_class}")
        _, manifest = manifests[character_class]
        _, manifest_stage = _find_manifest_stage(manifest, skill_id, stage_index)
        manifest_clips = _require_list(
            manifest_stage.get("clips"), f"{legacy_effect_id}.manifest.clips"
        )
        _require(
            stage_clip_index < len(manifest_clips)
            and manifest_clips[stage_clip_index].get("clip") == clip,
            f"rollout product target no longer joins its source manifest clip: {legacy_effect_id}",
        )

        authoring_path = _safe_relative_path(
            row.get("authoringPath"), f"{legacy_effect_id}.authoringPath"
        )
        legacy_relative = f"Data/{authoring_path}"
        source_reference, source_document = registry.add(
            legacy_relative,
            ("IMMUTABLE_LEGACY_ROLLBACK", "LEGACY_STARTER_SOURCE"),
        )
        _require(
            source_document.get("schema") == AUTHORING_SCHEMA
            and source_document.get("version") == 12
            and source_document.get("effectAssetId") == legacy_effect_id,
            f"legacy starter source identity changed: {legacy_relative}",
        )
        rollout_file_sha = _require_sha256(
            row.get("documentFileSha256"),
            f"{legacy_effect_id}.rollout.documentFileSha256",
        )
        if source_reference["rawSha256"] == rollout_file_sha:
            rollout_hash_disposition = "EXACT_RAW"
        else:
            source_payload = _repository_file(
                repository_root, legacy_relative, "legacy starter source"
            ).read_bytes()
            _require(
                _normalized_lf_sha256(source_payload) == rollout_file_sha,
                f"legacy starter source drift is not EOL-only: {legacy_relative}",
            )
            rollout_hash_disposition = "EOL_NORMALIZED_MATCH"

        catalog_matches = catalog_by_id.get(legacy_effect_id, [])
        _require(
            len(catalog_matches) == 1
            and catalog_matches[0].get("authoringPath") == authoring_path,
            f"legacy starter catalog mapping is missing/ambiguous: {legacy_effect_id}",
        )
        catalog_entry_sha = canonical_json_sha256(catalog_matches[0])
        animation_identity = animevent_identity[character_class]
        active_count = _active_effect_reference_count(
            animevent_payload[character_class], legacy_effect_id
        )
        orphaned = legacy_effect_id in EXPECTED_ORPHANED_PRODUCT_EFFECT_IDS
        _require(
            active_count == (0 if orphaned else 1),
            f"legacy starter animevent reference count changed: {legacy_effect_id}",
        )

        candidate_effect_id, candidate_relative = _unified_candidate_identity(
            legacy_effect_id
        )
        _require(
            _active_effect_reference_count(
                animevent_payload[character_class], candidate_effect_id
            )
            == 0,
            f"unapproved candidate is already active in animevents: {candidate_effect_id}",
        )
        candidate_path = _repository_path(
            repository_root, candidate_relative, "legacy unified candidate path"
        )
        if candidate_path.exists():
            candidate_reference, candidate_document = registry.add(
                candidate_relative,
                ("UNIFIED_CANDIDATE_BASELINE", "LEGACY_STARTER_SELECTED_SOURCE"),
            )
            candidate_version = _require_int(
                candidate_document.get("version"),
                f"{candidate_effect_id}.candidate.version",
                1,
            )
            _require(
                candidate_document.get("schema") == AUTHORING_SCHEMA
                and candidate_version in {12, 13}
                and candidate_document.get("effectAssetId") == candidate_effect_id,
                f"existing unified candidate identity changed: {candidate_relative}",
            )
            candidate_baseline = {
                "policy": "EXPECTED_EXACT_OR_REFUSE",
                "expectedRawSha256": candidate_reference["rawSha256"],
                "expectedCanonicalJsonSha256": candidate_reference[
                    "canonicalJsonSha256"
                ],
                "authoringVersion": candidate_version,
            }
            starter_source = _extended_document_reference(
                candidate_reference,
                candidate_document,
                kind="EXISTING_UNIFIED_CANDIDATE",
            )
        else:
            candidate_baseline = {
                "policy": "MUST_NOT_EXIST",
                "expectedRawSha256": None,
                "expectedCanonicalJsonSha256": None,
                "authoringVersion": None,
            }
            starter_source = _extended_document_reference(
                source_reference,
                source_document,
                kind="IMMUTABLE_LEGACY_ROLLBACK",
            )

        blockers = {
            "LEGACY_TUNING_STARTER_NOT_TRACK_A_ADMISSION",
            "USER_VISUAL_APPROVAL_REQUIRED_BEFORE_PRODUCT_MAPPING",
        }
        if (character_class, skill_id) in KNOWN_BINDING_MANIFEST_DRIFT:
            blockers.add("SKILL_BINDING_MANIFEST_RECONCILIATION_REQUIRED")
        if orphaned:
            blockers.add("PRODUCT_ANIMEVENT_REFERENCE_MISSING")
        candidate = {
            "candidateKey": candidate_effect_id,
            "characterClass": character_class,
            "animationAssetId": _require_stable_id(
                row.get("animationAssetId"), f"{legacy_effect_id}.animationAssetId"
            ),
            "skillId": skill_id,
            "stageIndex": stage_index,
            "stageClipIndex": stage_clip_index,
            "clip": clip,
            "legacyRollbackBaseline": {
                **_extended_document_reference(source_reference, source_document),
                "policy": "IMMUTABLE_LEGACY_ROLLBACK_EXACT",
                "rolloutDocumentFileSha256": rollout_file_sha,
                "rolloutHashDisposition": rollout_hash_disposition,
            },
            "starterSource": starter_source,
            "target": {
                "path": candidate_relative,
                "effectAssetId": candidate_effect_id,
                "requiredOutputVersion": 13,
                "candidateBaseline": candidate_baseline,
            },
            "productReference": {
                "catalogPath": catalog_reference["path"],
                "catalogEntryCanonicalSha256": catalog_entry_sha,
                "animeventPath": animation_identity["path"],
                "animeventRawSha256": animation_identity["rawSha256"],
                "activeReferenceCount": active_count,
                "orphanedCatalogReference": orphaned,
            },
            "rolloutRecordCanonicalSha256": canonical_json_sha256(row),
            "disposition": {
                "kind": "LEGACY_TUNING_STARTER",
                "blockers": sorted(blockers),
            },
            "trackAAdmission": False,
            "productMutation": False,
            "visualApproval": False,
        }
        _require(
            candidate_effect_id not in {
                item["target"]["effectAssetId"] for item in legacy_candidates
            },
            f"duplicate normalized unified candidate: {candidate_effect_id}",
        )
        legacy_candidates.append(candidate)
        candidate_by_legacy_id[legacy_effect_id] = candidate

    rollout_stages = [
        _require_dict(row, f"rollout.stages[{index}]")
        for index, row in enumerate(
            _require_list(rollout.get("stages"), "rollout.stages")
        )
    ]
    _require(len(rollout_stages) == 74, "rollout stage denominator changed")
    legacy_stages: list[dict[str, Any]] = []
    for row in rollout_stages:
        character_class = _require_stable_id(
            row.get("characterClass"), "rollout stage characterClass"
        )
        skill_id = _require_int(row.get("productSkillId"), "rollout stage skillId", 1)
        stage_index = _require_int(row.get("stageIndex"), "rollout stage stageIndex")
        if (character_class, skill_id, stage_index) in track_a_stage_identities:
            continue
        manifest_reference, manifest = manifests[character_class]
        manifest_skill, manifest_stage = _find_manifest_stage(
            manifest, skill_id, stage_index
        )
        stage_id = _require_stable_id(
            row.get("stageId"), f"{character_class}/{skill_id}/{stage_index}.stageId"
        )
        _require(
            stage_id == manifest_stage.get("stageId"),
            f"rollout stage no longer joins manifest: {stage_id}",
        )
        clip_rows: list[dict[str, Any]] = []
        candidate_ids: list[str] = []
        for clip_row_value in _require_list(
            row.get("clipProducts"), f"{stage_id}.clipProducts"
        ):
            clip_row = _require_dict(clip_row_value, f"{stage_id}.clipProduct")
            status = _require_stable_id(
                clip_row.get("status"), f"{stage_id}.clip.status"
            )
            _require(
                status in {
                    "visualBearing",
                    "noSelectedCarrier",
                    "sourceIntentionallySilent",
                },
                f"unknown rollout clip disposition: {stage_id}/{status}",
            )
            legacy_effect_id = clip_row.get("productTargetEffectAssetId")
            if status == "visualBearing":
                legacy_effect_id = _require_stable_id(
                    legacy_effect_id, f"{stage_id}.clip.productTargetEffectAssetId"
                )
                candidate = candidate_by_legacy_id.get(legacy_effect_id)
                _require(
                    candidate is not None,
                    f"remaining stage references no legacy starter candidate: {legacy_effect_id}",
                )
                candidate_effect_id = candidate["target"]["effectAssetId"]
                if candidate_effect_id not in candidate_ids:
                    candidate_ids.append(candidate_effect_id)
            else:
                _require(
                    legacy_effect_id is None,
                    f"silent/no-carrier clip unexpectedly has a product target: {stage_id}",
                )
                candidate_effect_id = None
            clip_rows.append(
                {
                    "clip": _require_stable_id(
                        clip_row.get("clip"), f"{stage_id}.clip"
                    ),
                    "stageClipIndex": _require_int(
                        clip_row.get("stageClipIndex"),
                        f"{stage_id}.stageClipIndex",
                    ),
                    "status": status,
                    "legacyEffectAssetId": legacy_effect_id,
                    "candidateEffectAssetId": candidate_effect_id,
                }
            )
        stage_status = _require_stable_id(row.get("status"), f"{stage_id}.status")
        _require(
            stage_status in {"effectBearing", "sourceIntentionallySilent"},
            f"unknown remaining stage status: {stage_status}",
        )
        blockers = {
            "SOURCE_INTENTIONALLY_SILENT"
            if stage_status == "sourceIntentionallySilent"
            else "LEGACY_TUNING_STARTER_NOT_TRACK_A_ADMISSION"
        }
        if (character_class, skill_id) in KNOWN_BINDING_MANIFEST_DRIFT:
            blockers.add("SKILL_BINDING_MANIFEST_RECONCILIATION_REQUIRED")
        legacy_stages.append(
            {
                "stageKey": f"{character_class.lower()}.skill.{skill_id}.stage.{stage_index}",
                "mode": (
                    "INTENTIONALLY_SILENT"
                    if stage_status == "sourceIntentionallySilent"
                    else "LEGACY_STARTER_STAGE"
                ),
                "characterClass": character_class,
                "animationAssetId": _require_stable_id(
                    row.get("animationAssetId"), f"{stage_id}.animationAssetId"
                ),
                "skillId": skill_id,
                "inputSlot": _require_stable_id(
                    manifest_skill.get("inputSlot"), f"{stage_id}.inputSlot"
                ),
                "skillKind": _require_stable_id(
                    manifest_skill.get("skillKind"), f"{stage_id}.skillKind"
                ),
                "stageIndex": stage_index,
                "stageId": stage_id,
                "status": stage_status,
                "sourceManifest": manifest_reference,
                "rolloutStageCanonicalSha256": canonical_json_sha256(row),
                "clips": clip_rows,
                "candidateEffectAssetIds": candidate_ids,
                "blockers": sorted(blockers),
                "productMutation": False,
                "visualApproval": False,
            }
        )

    full_denominators = {
        "fullSkillCount": len(
            {
                (row["characterClass"], row["productSkillId"])
                for row in rollout_stages
            }
        ),
        "fullStageCount": len(rollout_stages),
        "fullClipOccurrenceCount": sum(
            len(_require_list(row.get("clipProducts"), "rollout stage.clipProducts"))
            for row in rollout_stages
        ),
        "fullVisualClipOccurrenceCount": sum(
            clip.get("status") == "visualBearing"
            for row in rollout_stages
            for clip in _require_list(row.get("clipProducts"), "rollout clipProducts")
            if isinstance(clip, dict)
        ),
        "fullSilentOrNoCarrierClipOccurrenceCount": sum(
            clip.get("status") != "visualBearing"
            for row in rollout_stages
            for clip in _require_list(row.get("clipProducts"), "rollout clipProducts")
            if isinstance(clip, dict)
        ),
        "fullUniqueCandidateDocumentCount": len(rollout_targets),
        "trackASeamStageCount": len(track_a_stages),
        "trackASeamCandidateDocumentCount": len(track_a_legacy_effect_ids),
        "legacyStarterSkillIdentityCount": len(
            {(row["characterClass"], row["skillId"]) for row in legacy_stages}
        ),
        "legacyStarterStageCount": len(legacy_stages),
        "legacyStarterEffectBearingStageCount": sum(
            row["status"] == "effectBearing" for row in legacy_stages
        ),
        "legacyStarterIntentionallySilentStageCount": sum(
            row["status"] == "sourceIntentionallySilent" for row in legacy_stages
        ),
        "legacyStarterClipOccurrenceCount": sum(
            len(row["clips"]) for row in legacy_stages
        ),
        "legacyStarterVisualClipOccurrenceCount": sum(
            clip["status"] == "visualBearing"
            for row in legacy_stages
            for clip in row["clips"]
        ),
        "legacyStarterSilentOrNoCarrierClipOccurrenceCount": sum(
            clip["status"] != "visualBearing"
            for row in legacy_stages
            for clip in row["clips"]
        ),
        "legacyStarterCandidateDocumentCount": len(legacy_candidates),
        "legacyStarterActiveProductReferenceCount": sum(
            row["productReference"]["activeReferenceCount"]
            for row in legacy_candidates
        ),
        "legacyStarterOrphanedCatalogReferenceCount": sum(
            row["productReference"]["orphanedCatalogReference"]
            for row in legacy_candidates
        ),
        "productMutationCount": 0,
    }
    _require(
        full_denominators == EXPECTED_FULL_SCOPE_DENOMINATORS,
        f"full four-class denominator changed: {full_denominators}",
    )
    _require(
        {
            row["legacyRollbackBaseline"]["effectAssetId"]
            for row in legacy_candidates
            if row["productReference"]["orphanedCatalogReference"]
        }
        == EXPECTED_ORPHANED_PRODUCT_EFFECT_IDS,
        "orphaned Warlord product target denominator changed",
    )
    return legacy_stages, legacy_candidates, full_denominators


def build_batch(repository_root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    repository_root = repository_root.resolve()
    registry = ArtifactRegistry(repository_root)
    schema_reference, schema_document = _artifact_reference(
        repository_root, SCHEMA_RELATIVE_PATH
    )
    _require(
        schema_document.get("$id")
        == "https://lostark.local/schemas/effect-authored-import-batch-v1.json",
        "authored import batch schema identity changed",
    )
    builder_relative = SCRIPT_PATH.resolve().relative_to(repository_root).as_posix()
    builder_path = _repository_file(
        repository_root, builder_relative, "builder path"
    )
    builder_identity = {
        "path": builder_relative,
        "rawSha256": raw_sha256(builder_path),
    }

    visual_reference, visual_runtime = registry.add(
        VISUAL_RUNTIME_RELATIVE_PATH, ("TRACK_A_VISUAL_PROGRAM_RUNTIME",)
    )
    _require(
        visual_runtime.get("schema") == VISUAL_RUNTIME_SCHEMA
        and visual_runtime.get("formatVersion") == 1,
        "visual program runtime schema/version changed",
    )
    _validate_seal(visual_runtime, "artifactSha256", "visual program runtime")
    source_reference, source_admission = registry.add(
        SOURCE_ADMISSION_RELATIVE_PATH, ("THREE_CLASS_BA_SOURCE_ADMISSION",)
    )
    _require(
        source_admission.get("schema") == SOURCE_ADMISSION_SCHEMA
        and source_admission.get("formatVersion") == 1,
        "source runtime admission schema/version changed",
    )
    _validate_seal(source_admission, "artifactSha256", "source runtime admission")
    _require(
        source_admission.get("admission", {}).get("currentProductMutation")
        is False,
        "source admission unexpectedly permits current product mutation",
    )
    manifests = _load_manifests(repository_root, registry)

    stages, plans, excluded_count = _build_track_a_stages(
        repository_root,
        registry,
        visual_reference,
        visual_runtime,
        source_admission,
        manifests,
    )
    canary_stage, canary_plans = _build_warlord_canary_stage(
        repository_root,
        registry,
        visual_reference,
        visual_runtime,
        manifests,
    )
    stages.append(canary_stage)
    plans.extend(canary_plans)
    legacy_starter_stages, legacy_starter_candidates, full_denominators = (
        _build_legacy_starter_scope(
            repository_root,
            registry,
            manifests,
            stages,
        )
    )

    plan_ids = [item["planId"] for item in plans]
    target_keys = [
        (item["target"]["effectAssetId"], item["target"]["elementId"])
        for item in plans
    ]
    _require(
        len(plan_ids) == len(set(plan_ids)),
        "duplicate authored import plan ID",
    )
    _require(
        len(target_keys) == len(set(target_keys)),
        "duplicate target Effect/Element import",
    )
    _require(
        all("effect.artist.skill.31470" not in str(item) for item in stages),
        "Artist F must stay outside the four-class import batch",
    )

    material_counts = Counter(
        item["materialDisposition"]["kind"] for item in plans
    )
    carrier_counts = Counter(
        item["carrierDisposition"]["kind"] for item in plans
    )
    track_a_stage_count = sum(
        item["mode"] == TRACK_A_SELECTION_KIND for item in stages
    )
    canary_stage_count = sum(
        item["mode"] == WARLORD_SELECTION_KIND for item in stages
    )
    track_a_selected = sum(
        item["source"]["kind"] == "IMPORTED_ELEMENT"
        and item["stageKey"] != "warlord.skill.17000.stage.0"
        for item in plans
    )
    supplemental_count = sum(
        item["source"]["kind"] == "VISUAL_PROGRAM_SUPPLEMENTAL"
        for item in plans
    )
    canary_count = len(canary_plans)
    denominators = {
        "stageCount": len(stages),
        "trackAProgramStageCount": track_a_stage_count,
        "failClosedCanaryStageCount": canary_stage_count,
        "trackASelectedSourceRowCount": track_a_selected,
        "trackAExcludedSourceRowCount": excluded_count,
        "supplementalElementCount": supplemental_count,
        "canaryElementCount": canary_count,
        "elementPlanCount": len(plans),
        "genericParticleCandidateCount": carrier_counts[
            "GENERIC_PARTICLE_IMPORT_CANDIDATE"
        ],
        "familyAdapterRequiredCount": carrier_counts["FAMILY_ADAPTER_REQUIRED"],
        "supplementalAdapterPreserveCount": carrier_counts[
            "SUPPLEMENTAL_ADAPTER_PRESERVE"
        ],
        "typedExecutionMaterialCount": material_counts["TYPED_EXECUTION"],
        "admittedSourceProfileMaterialCount": material_counts[
            "ADMITTED_SOURCE_PROFILE"
        ],
        "failClosedMaterialCount": material_counts["FAIL_CLOSED"],
        "productMutationCount": 0,
    }
    _require(
        denominators == EXPECTED_DENOMINATORS,
        f"authored import denominator changed: {denominators}",
    )
    normalized_material_counts = {
        key: material_counts[key] for key in EXPECTED_MATERIAL_COUNTS
    }
    _require(
        normalized_material_counts == EXPECTED_MATERIAL_COUNTS,
        f"material disposition denominator changed: {material_counts}",
    )
    _require(
        dict(sorted(carrier_counts.items())) == EXPECTED_CARRIER_COUNTS,
        f"carrier disposition denominator changed: {carrier_counts}",
    )

    batch: dict[str, Any] = {
        "schema": BATCH_SCHEMA,
        "formatVersion": BATCH_VERSION,
        "batchId": BATCH_ID,
        "contractRole": CONTRACT_ROLE,
        "scope": {
            "characterClasses": [
                "ARTIST",
                "DIMENSIONMASTER",
                "LANCE_MASTER",
                "WARLORD",
            ],
            "trackAProgramEffectAssetIds": [
                item["selection"]["recordId"]
                for item in stages
                if item["mode"] == TRACK_A_SELECTION_KIND
            ],
            "extensionCanaryEffectAssetId": WARLORD_EFFECT_ASSET_ID,
            "trackAUnifiedCandidateEffectAssetIds": [
                item["target"]["effectAssetId"] for item in stages
            ],
            "legacyStarterUnifiedCandidateEffectAssetIds": [
                item["target"]["effectAssetId"]
                for item in legacy_starter_candidates
            ],
            "unifiedCandidateEffectAssetIds": [
                item["target"]["effectAssetId"] for item in stages
            ]
            + [
                item["target"]["effectAssetId"]
                for item in legacy_starter_candidates
            ],
            "artistFDirectSliceEffectAssetId": "effect.artist.skill.31470.unified",
            "artistFExcluded": True,
            "productCatalogMutation": False,
            "animationEventMutation": False,
        },
        "schemaIdentity": schema_reference,
        "builderIdentity": builder_identity,
        "inputArtifacts": registry.items(),
        "stages": stages,
        "elementPlans": plans,
        "legacyStarterStages": legacy_starter_stages,
        "legacyStarterCandidates": legacy_starter_candidates,
        "denominators": denominators,
        "fullScopeDenominators": full_denominators,
        "materialDispositionCounts": EXPECTED_MATERIAL_COUNTS,
        "carrierDispositionCounts": EXPECTED_CARRIER_COUNTS,
        "admission": {
            "offlineBatchPlanning": True,
            "allElementsMaterialAdmitted": False,
            "allElementsGenericCarrierAdmitted": False,
            "productMappingMutation": False,
            "visualApproval": False,
            "blockers": [
                "CXX_GENERIC_CODEC_BATCH_VALIDATION_REQUIRED",
                "FAIL_CLOSED_MATERIAL_ROWS_PRESENT",
                "FAMILY_ADAPTER_ROWS_PRESENT",
                "USER_VISUAL_APPROVAL_REQUIRED_BEFORE_PRODUCT_MAPPING",
            ],
        },
        "transactionPolicy": {
            "loadOrder": [
                "schema",
                "visual-program-runtime",
                "source-admission",
                "source-manifests",
                "source-receipts-and-documents",
                "selection-receipts",
                "legacy-rollback-baselines",
                "four-class-product-rollout",
                "effect-catalog-and-animevent-mapping-evidence",
                "legacy-starter-sources",
                "unified-candidate-baselines-if-present",
            ],
            "commitMode": "PARSE_VALIDATE_STAGE_THEN_CREATE_OR_EXACT_REPLACE_CANDIDATES_ONLY",
            "failureAction": "PRESERVE_PREVIOUS_BATCH_LEGACY_BASELINES_AND_CANDIDATES",
            "candidateBaselinePolicy": "MUST_NOT_EXIST_OR_EXPECTED_EXACT_OR_REFUSE",
            "authoredDocumentMutation": False,
            "catalogMutation": False,
            "animationEventMutation": False,
        },
    }
    batch["artifactSha256"] = canonical_json_sha256(batch)
    validate_batch(batch, repository_root)
    return batch


def _validate_material_disposition(value: Any, label: str) -> str:
    material = _require_dict(value, label)
    kind = _require_string(material.get("kind"), f"{label}.kind")
    if kind == "TYPED_EXECUTION":
        _require_exact_keys(
            material, ("kind", "executionSnapshotSha256"), label
        )
        _require_sha256(
            material["executionSnapshotSha256"], f"{label}.executionSnapshotSha256"
        )
    elif kind == "ADMITTED_SOURCE_PROFILE":
        _require_exact_keys(
            material,
            (
                "kind",
                "sourceProfileSha256",
                "profileId",
                "runtimeShaderProfileId",
            ),
            label,
        )
        _require_sha256(material["sourceProfileSha256"], f"{label}.sourceProfileSha256")
        _require_stable_id(material["profileId"], f"{label}.profileId")
        _require_stable_id(
            material["runtimeShaderProfileId"], f"{label}.runtimeShaderProfileId"
        )
    elif kind == "FAIL_CLOSED":
        _require_exact_keys(material, ("kind", "blockers"), label)
        blockers = _require_list(material["blockers"], f"{label}.blockers")
        _require(
            blockers
            and blockers == sorted(set(blockers))
            and all(isinstance(item, str) and item for item in blockers),
            f"{label}.blockers must be non-empty, sorted, and unique",
        )
    else:
        raise ContractError(f"{label} has unknown material disposition: {kind}")
    return kind


def _validate_legacy_starter_contract(
    batch: dict[str, Any],
    artifact_by_path: dict[str, dict[str, Any]],
    repository_root: Path,
) -> None:
    stages = _require_list(
        batch.get("legacyStarterStages"), "batch.legacyStarterStages"
    )
    candidates = _require_list(
        batch.get("legacyStarterCandidates"), "batch.legacyStarterCandidates"
    )
    _require(len(stages) == 61, "legacy starter stage denominator changed")
    _require(len(candidates) == 88, "legacy starter candidate denominator changed")

    rollout_artifact = artifact_by_path.get(FOUR_CLASS_ROLLOUT_RELATIVE_PATH)
    catalog_artifact = artifact_by_path.get(EFFECT_CATALOG_RELATIVE_PATH)
    _require(
        rollout_artifact is not None and catalog_artifact is not None,
        "legacy starter rollout/catalog evidence is not registered",
    )
    rollout = load_json(
        _repository_file(
            repository_root,
            FOUR_CLASS_ROLLOUT_RELATIVE_PATH,
            "legacy starter rollout",
        )
    )
    catalog = load_json(
        _repository_file(
            repository_root, EFFECT_CATALOG_RELATIVE_PATH, "legacy starter catalog"
        )
    )
    rollout_targets = {
        row["effectAssetId"]: row
        for row in _require_list(rollout.get("productTargets"), "rollout.productTargets")
        if isinstance(row, dict) and isinstance(row.get("effectAssetId"), str)
    }
    rollout_stages = {
        (row.get("characterClass"), row.get("productSkillId"), row.get("stageIndex")): row
        for row in _require_list(rollout.get("stages"), "rollout.stages")
        if isinstance(row, dict)
    }
    catalog_rows: dict[str, list[dict[str, Any]]] = {}
    for row in _require_list(catalog.get("effects"), "EffectCatalog.effects"):
        if isinstance(row, dict) and isinstance(row.get("effectAssetId"), str):
            catalog_rows.setdefault(row["effectAssetId"], []).append(row)

    def validate_registered_reference(
        value: Any,
        label: str,
        *,
        expected_keys: Iterable[str],
    ) -> tuple[dict[str, Any], dict[str, Any]]:
        reference = _require_dict(value, label)
        _require_exact_keys(reference, expected_keys, label)
        relative = _safe_relative_path(reference.get("path"), f"{label}.path")
        registered = artifact_by_path.get(relative)
        _require(registered is not None, f"unregistered legacy starter artifact: {relative}")
        _require(
            registered["rawSha256"]
            == _require_sha256(reference.get("rawSha256"), f"{label}.rawSha256")
            and registered["canonicalJsonSha256"]
            == _require_sha256(
                reference.get("canonicalJsonSha256"),
                f"{label}.canonicalJsonSha256",
            ),
            f"legacy starter artifact identity mismatch: {relative}",
        )
        document = load_json(
            _repository_file(repository_root, relative, f"{label}.path")
        )
        return reference, document

    candidate_by_id: dict[str, dict[str, Any]] = {}
    target_paths: set[str] = set()
    orphaned_legacy_ids: set[str] = set()
    active_reference_count = 0
    for index, value in enumerate(candidates):
        candidate = _require_dict(value, f"legacyStarterCandidates[{index}]")
        _require_exact_keys(
            candidate,
            (
                "candidateKey",
                "characterClass",
                "animationAssetId",
                "skillId",
                "stageIndex",
                "stageClipIndex",
                "clip",
                "legacyRollbackBaseline",
                "starterSource",
                "target",
                "productReference",
                "rolloutRecordCanonicalSha256",
                "disposition",
                "trackAAdmission",
                "productMutation",
                "visualApproval",
            ),
            f"legacyStarterCandidates[{index}]",
        )
        candidate_key = _require_stable_id(
            candidate.get("candidateKey"), f"legacyStarterCandidates[{index}].candidateKey"
        )
        _require(
            candidate_key not in candidate_by_id,
            f"duplicate legacy starter candidate ID: {candidate_key}",
        )
        character_class = _require_stable_id(
            candidate.get("characterClass"), f"{candidate_key}.characterClass"
        )
        _require(character_class in CLASS_ANIMEVENTS, f"unknown candidate class: {character_class}")
        skill_id = _require_int(candidate.get("skillId"), f"{candidate_key}.skillId", 1)
        stage_index = _require_int(candidate.get("stageIndex"), f"{candidate_key}.stageIndex")
        stage_clip_index = _require_int(
            candidate.get("stageClipIndex"), f"{candidate_key}.stageClipIndex"
        )
        clip = _require_stable_id(candidate.get("clip"), f"{candidate_key}.clip")

        legacy, legacy_document = validate_registered_reference(
            candidate.get("legacyRollbackBaseline"),
            f"{candidate_key}.legacyRollbackBaseline",
            expected_keys=(
                "path",
                "rawSha256",
                "canonicalJsonSha256",
                "effectAssetId",
                "authoringVersion",
                "policy",
                "rolloutDocumentFileSha256",
                "rolloutHashDisposition",
            ),
        )
        legacy_effect_id = _require_stable_id(
            legacy.get("effectAssetId"), f"{candidate_key}.legacy.effectAssetId"
        )
        _require(
            legacy.get("policy") == "IMMUTABLE_LEGACY_ROLLBACK_EXACT"
            and legacy.get("authoringVersion") == 12
            and legacy_document.get("schema") == AUTHORING_SCHEMA
            and legacy_document.get("version") == 12
            and legacy_document.get("effectAssetId") == legacy_effect_id
            and legacy.get("rolloutHashDisposition")
            in {"EXACT_RAW", "EOL_NORMALIZED_MATCH"},
            f"legacy starter rollback contract changed: {candidate_key}",
        )
        rollout_row = rollout_targets.get(legacy_effect_id)
        _require(
            rollout_row is not None
            and canonical_json_sha256(rollout_row)
            == _require_sha256(
                candidate.get("rolloutRecordCanonicalSha256"),
                f"{candidate_key}.rolloutRecordCanonicalSha256",
            )
            and rollout_row.get("characterClass") == character_class
            and rollout_row.get("productSkillId") == skill_id
            and rollout_row.get("stageIndex") == stage_index
            and rollout_row.get("stageClipIndex") == stage_clip_index
            and rollout_row.get("clip") == clip,
            f"legacy starter rollout record changed: {candidate_key}",
        )
        rollout_sha = _require_sha256(
            legacy.get("rolloutDocumentFileSha256"),
            f"{candidate_key}.rolloutDocumentFileSha256",
        )
        _require(
            rollout_row.get("documentFileSha256") == rollout_sha,
            f"legacy starter rollout file SHA record changed: {candidate_key}",
        )
        source_path = _repository_file(
            repository_root, legacy["path"], f"{candidate_key}.legacy.path"
        )
        source_payload = source_path.read_bytes()
        if legacy["rolloutHashDisposition"] == "EXACT_RAW":
            _require(
                legacy["rawSha256"] == rollout_sha,
                f"legacy starter exact rollout SHA changed: {candidate_key}",
            )
        else:
            _require(
                legacy["rawSha256"] != rollout_sha
                and _normalized_lf_sha256(source_payload) == rollout_sha,
                f"legacy starter EOL-only rollout evidence changed: {candidate_key}",
            )

        starter, starter_document = validate_registered_reference(
            candidate.get("starterSource"),
            f"{candidate_key}.starterSource",
            expected_keys=(
                "kind",
                "path",
                "rawSha256",
                "canonicalJsonSha256",
                "effectAssetId",
                "authoringVersion",
            ),
        )
        _require(
            starter.get("kind")
            in {"IMMUTABLE_LEGACY_ROLLBACK", "EXISTING_UNIFIED_CANDIDATE"}
            and starter.get("authoringVersion") in {12, 13}
            and starter_document.get("schema") == AUTHORING_SCHEMA
            and starter_document.get("version") == starter.get("authoringVersion")
            and starter_document.get("effectAssetId") == starter.get("effectAssetId"),
            f"legacy starter selected source changed: {candidate_key}",
        )

        target = _require_dict(candidate.get("target"), f"{candidate_key}.target")
        _require_exact_keys(
            target,
            ("path", "effectAssetId", "requiredOutputVersion", "candidateBaseline"),
            f"{candidate_key}.target",
        )
        target_path = _safe_relative_path(target.get("path"), f"{candidate_key}.target.path")
        target_effect_id = _require_stable_id(
            target.get("effectAssetId"), f"{candidate_key}.target.effectAssetId"
        )
        expected_effect_id, expected_path = _unified_candidate_identity(legacy_effect_id)
        _require(
            target_effect_id == candidate_key == expected_effect_id
            and target_path == expected_path
            and target.get("requiredOutputVersion") == 13
            and target_path not in target_paths,
            f"legacy starter target identity changed: {candidate_key}",
        )
        target_paths.add(target_path)
        baseline = _require_dict(
            target.get("candidateBaseline"), f"{candidate_key}.candidateBaseline"
        )
        _require_exact_keys(
            baseline,
            (
                "policy",
                "expectedRawSha256",
                "expectedCanonicalJsonSha256",
                "authoringVersion",
            ),
            f"{candidate_key}.candidateBaseline",
        )
        target_file = _repository_path(
            repository_root, target_path, f"{candidate_key}.target.path"
        )
        if baseline.get("policy") == "MUST_NOT_EXIST":
            _require(
                baseline.get("expectedRawSha256") is None
                and baseline.get("expectedCanonicalJsonSha256") is None
                and baseline.get("authoringVersion") is None
                and not target_file.exists()
                and target_path not in artifact_by_path
                and starter.get("kind") == "IMMUTABLE_LEGACY_ROLLBACK",
                f"legacy starter MUST_NOT_EXIST contract changed: {candidate_key}",
            )
        elif baseline.get("policy") == "EXPECTED_EXACT_OR_REFUSE":
            registered_candidate = artifact_by_path.get(target_path)
            _require(
                baseline.get("authoringVersion") in {12, 13}
                and registered_candidate is not None
                and registered_candidate["rawSha256"]
                == _require_sha256(
                    baseline.get("expectedRawSha256"),
                    f"{candidate_key}.candidate.expectedRawSha256",
                )
                and registered_candidate["canonicalJsonSha256"]
                == _require_sha256(
                    baseline.get("expectedCanonicalJsonSha256"),
                    f"{candidate_key}.candidate.expectedCanonicalJsonSha256",
                )
                and starter.get("kind") == "EXISTING_UNIFIED_CANDIDATE"
                and starter.get("path") == target_path
                and starter.get("effectAssetId") == target_effect_id,
                f"legacy starter exact candidate baseline changed: {candidate_key}",
            )
        else:
            raise ContractError(
                f"unknown legacy starter candidate baseline policy: {candidate_key}"
            )

        product = _require_dict(
            candidate.get("productReference"), f"{candidate_key}.productReference"
        )
        _require_exact_keys(
            product,
            (
                "catalogPath",
                "catalogEntryCanonicalSha256",
                "animeventPath",
                "animeventRawSha256",
                "activeReferenceCount",
                "orphanedCatalogReference",
            ),
            f"{candidate_key}.productReference",
        )
        matches = catalog_rows.get(legacy_effect_id, [])
        animevent_path = _safe_relative_path(
            product.get("animeventPath"), f"{candidate_key}.animeventPath"
        )
        animevent_file = _repository_file(
            repository_root, animevent_path, f"{candidate_key}.animeventPath"
        )
        animevent_payload = animevent_file.read_bytes()
        active_count = _require_int(
            product.get("activeReferenceCount"), f"{candidate_key}.activeReferenceCount"
        )
        orphaned = product.get("orphanedCatalogReference")
        _require(
            product.get("catalogPath") == EFFECT_CATALOG_RELATIVE_PATH
            and len(matches) == 1
            and canonical_json_sha256(matches[0])
            == _require_sha256(
                product.get("catalogEntryCanonicalSha256"),
                f"{candidate_key}.catalogEntryCanonicalSha256",
            )
            and animevent_path == CLASS_ANIMEVENTS[character_class]
            and raw_sha256(animevent_file)
            == _require_sha256(
                product.get("animeventRawSha256"),
                f"{candidate_key}.animeventRawSha256",
            )
            and _active_effect_reference_count(animevent_payload, legacy_effect_id)
            == active_count
            and _active_effect_reference_count(animevent_payload, target_effect_id) == 0
            and isinstance(orphaned, bool)
            and orphaned == (active_count == 0)
            and active_count in {0, 1},
            f"legacy starter product reference changed: {candidate_key}",
        )
        active_reference_count += active_count
        if orphaned:
            orphaned_legacy_ids.add(legacy_effect_id)

        disposition = _require_dict(
            candidate.get("disposition"), f"{candidate_key}.disposition"
        )
        _require_exact_keys(
            disposition, ("kind", "blockers"), f"{candidate_key}.disposition"
        )
        blockers = _require_list(
            disposition.get("blockers"), f"{candidate_key}.disposition.blockers"
        )
        _require(
            disposition.get("kind") == "LEGACY_TUNING_STARTER"
            and blockers == sorted(set(blockers))
            and "LEGACY_TUNING_STARTER_NOT_TRACK_A_ADMISSION" in blockers
            and candidate.get("trackAAdmission") is False
            and candidate.get("productMutation") is False
            and candidate.get("visualApproval") is False,
            f"legacy starter disposition overclaims admission: {candidate_key}",
        )
        candidate_by_id[candidate_key] = candidate

    stage_keys: set[str] = set()
    referenced_candidates: set[str] = set()
    visual_occurrences = 0
    silent_occurrences = 0
    effect_bearing = 0
    intentionally_silent = 0
    for index, value in enumerate(stages):
        stage = _require_dict(value, f"legacyStarterStages[{index}]")
        _require_exact_keys(
            stage,
            (
                "stageKey",
                "mode",
                "characterClass",
                "animationAssetId",
                "skillId",
                "inputSlot",
                "skillKind",
                "stageIndex",
                "stageId",
                "status",
                "sourceManifest",
                "rolloutStageCanonicalSha256",
                "clips",
                "candidateEffectAssetIds",
                "blockers",
                "productMutation",
                "visualApproval",
            ),
            f"legacyStarterStages[{index}]",
        )
        stage_key = _require_stable_id(
            stage.get("stageKey"), f"legacyStarterStages[{index}].stageKey"
        )
        _require(stage_key not in stage_keys, f"duplicate legacy starter stage: {stage_key}")
        stage_keys.add(stage_key)
        character_class = _require_stable_id(
            stage.get("characterClass"), f"{stage_key}.characterClass"
        )
        skill_id = _require_int(stage.get("skillId"), f"{stage_key}.skillId", 1)
        stage_index = _require_int(stage.get("stageIndex"), f"{stage_key}.stageIndex")
        rollout_row = rollout_stages.get((character_class, skill_id, stage_index))
        _require(
            rollout_row is not None
            and canonical_json_sha256(rollout_row)
            == _require_sha256(
                stage.get("rolloutStageCanonicalSha256"),
                f"{stage_key}.rolloutStageCanonicalSha256",
            ),
            f"legacy starter rollout stage changed: {stage_key}",
        )
        manifest_reference, _ = validate_registered_reference(
            stage.get("sourceManifest"),
            f"{stage_key}.sourceManifest",
            expected_keys=("path", "rawSha256", "canonicalJsonSha256"),
        )
        _require(
            manifest_reference["path"] == CLASS_MANIFESTS[character_class],
            f"legacy starter stage references another class manifest: {stage_key}",
        )
        status = _require_stable_id(stage.get("status"), f"{stage_key}.status")
        mode = _require_stable_id(stage.get("mode"), f"{stage_key}.mode")
        _require(
            (status, mode)
            in {
                ("effectBearing", "LEGACY_STARTER_STAGE"),
                ("sourceIntentionallySilent", "INTENTIONALLY_SILENT"),
            }
            and stage.get("stageId") == rollout_row.get("stageId")
            and stage.get("animationAssetId") == rollout_row.get("animationAssetId")
            and stage.get("productMutation") is False
            and stage.get("visualApproval") is False,
            f"legacy starter stage identity changed: {stage_key}",
        )
        effect_bearing += status == "effectBearing"
        intentionally_silent += status == "sourceIntentionallySilent"
        clips = _require_list(stage.get("clips"), f"{stage_key}.clips")
        rollout_clips = _require_list(rollout_row.get("clipProducts"), f"{stage_key}.rollout.clips")
        _require(len(clips) == len(rollout_clips), f"legacy starter clip count changed: {stage_key}")
        actual_candidate_ids: list[str] = []
        for clip_index, (clip_value, rollout_clip) in enumerate(zip(clips, rollout_clips)):
            clip_row = _require_dict(clip_value, f"{stage_key}.clips[{clip_index}]")
            _require_exact_keys(
                clip_row,
                (
                    "clip",
                    "stageClipIndex",
                    "status",
                    "legacyEffectAssetId",
                    "candidateEffectAssetId",
                ),
                f"{stage_key}.clips[{clip_index}]",
            )
            _require(
                clip_row.get("clip") == rollout_clip.get("clip")
                and clip_row.get("stageClipIndex") == rollout_clip.get("stageClipIndex")
                and clip_row.get("status") == rollout_clip.get("status"),
                f"legacy starter clip identity changed: {stage_key}/{clip_index}",
            )
            if clip_row.get("status") == "visualBearing":
                visual_occurrences += 1
                legacy_effect_id = _require_stable_id(
                    clip_row.get("legacyEffectAssetId"), f"{stage_key}.clip.legacyEffectAssetId"
                )
                candidate_effect_id = _require_stable_id(
                    clip_row.get("candidateEffectAssetId"), f"{stage_key}.clip.candidateEffectAssetId"
                )
                _require(
                    rollout_clip.get("productTargetEffectAssetId") == legacy_effect_id
                    and candidate_effect_id in candidate_by_id
                    and candidate_by_id[candidate_effect_id]["legacyRollbackBaseline"]["effectAssetId"]
                    == legacy_effect_id,
                    f"legacy starter clip/candidate join changed: {stage_key}/{clip_index}",
                )
                referenced_candidates.add(candidate_effect_id)
                if candidate_effect_id not in actual_candidate_ids:
                    actual_candidate_ids.append(candidate_effect_id)
            else:
                silent_occurrences += 1
                _require(
                    clip_row.get("legacyEffectAssetId") is None
                    and clip_row.get("candidateEffectAssetId") is None,
                    f"silent/no-carrier clip gained a candidate: {stage_key}/{clip_index}",
                )
        declared_candidates = _require_list(
            stage.get("candidateEffectAssetIds"), f"{stage_key}.candidateEffectAssetIds"
        )
        _require(
            declared_candidates == actual_candidate_ids
            and len(declared_candidates) == len(set(declared_candidates)),
            f"legacy starter stage candidate membership changed: {stage_key}",
        )
        blockers = _require_list(stage.get("blockers"), f"{stage_key}.blockers")
        _require(
            blockers == sorted(set(blockers)) and bool(blockers),
            f"legacy starter stage blockers are invalid: {stage_key}",
        )

    _require(
        referenced_candidates == set(candidate_by_id)
        and visual_occurrences == 89
        and silent_occurrences == 11
        and effect_bearing == 60
        and intentionally_silent == 1
        and active_reference_count == 85
        and orphaned_legacy_ids == EXPECTED_ORPHANED_PRODUCT_EFFECT_IDS,
        "legacy starter stage/candidate denominator or join changed",
    )
    _require(
        batch.get("fullScopeDenominators") == EXPECTED_FULL_SCOPE_DENOMINATORS,
        "full-scope denominators changed",
    )


def validate_batch(
    batch: dict[str, Any], repository_root: Path = REPOSITORY_ROOT
) -> None:
    repository_root = repository_root.resolve()
    _require_exact_keys(
        batch,
        (
            "schema",
            "formatVersion",
            "batchId",
            "contractRole",
            "scope",
            "schemaIdentity",
            "builderIdentity",
            "inputArtifacts",
            "stages",
            "elementPlans",
            "legacyStarterStages",
            "legacyStarterCandidates",
            "denominators",
            "fullScopeDenominators",
            "materialDispositionCounts",
            "carrierDispositionCounts",
            "admission",
            "transactionPolicy",
            "artifactSha256",
        ),
        "batch",
    )
    _require(
        batch["schema"] == BATCH_SCHEMA
        and batch["formatVersion"] == BATCH_VERSION
        and batch["batchId"] == BATCH_ID
        and batch["contractRole"] == CONTRACT_ROLE,
        "batch identity changed",
    )
    expected_artifact_sha = _require_sha256(
        batch["artifactSha256"], "batch.artifactSha256"
    )
    unsealed = copy.deepcopy(batch)
    unsealed.pop("artifactSha256")
    _require(
        canonical_json_sha256(unsealed) == expected_artifact_sha,
        "batch artifact SHA is stale",
    )

    schema_identity = _require_dict(batch["schemaIdentity"], "batch.schemaIdentity")
    _require_exact_keys(
        schema_identity,
        ("path", "rawSha256", "canonicalJsonSha256"),
        "batch.schemaIdentity",
    )
    schema_relative = _safe_relative_path(
        schema_identity["path"], "batch.schemaIdentity.path"
    )
    _require(
        schema_relative == SCHEMA_RELATIVE_PATH,
        "batch schema path changed",
    )
    schema_path = _repository_file(repository_root, schema_relative, "batch schema path")
    schema_document = load_json(schema_path)
    _require(
        raw_sha256(schema_path)
        == _require_sha256(
            schema_identity["rawSha256"], "batch.schemaIdentity.rawSha256"
        )
        and canonical_json_sha256(schema_document)
        == _require_sha256(
            schema_identity["canonicalJsonSha256"],
            "batch.schemaIdentity.canonicalJsonSha256",
        ),
        "batch schema identity is stale",
    )
    builder_identity = _require_dict(
        batch["builderIdentity"], "batch.builderIdentity"
    )
    _require_exact_keys(
        builder_identity, ("path", "rawSha256"), "batch.builderIdentity"
    )
    builder_relative = _safe_relative_path(
        builder_identity["path"], "batch.builderIdentity.path"
    )
    _require(
        builder_relative
        == SCRIPT_PATH.resolve().relative_to(repository_root).as_posix(),
        "batch builder path changed",
    )
    builder_path = _repository_file(repository_root, builder_relative, "batch builder path")
    _require(
        raw_sha256(builder_path)
        == _require_sha256(
            builder_identity["rawSha256"], "batch.builderIdentity.rawSha256"
        ),
        "batch builder identity is stale",
    )

    artifact_by_path: dict[str, dict[str, Any]] = {}
    for index, artifact in enumerate(
        _require_list(batch["inputArtifacts"], "batch.inputArtifacts")
    ):
        artifact = _require_dict(artifact, f"inputArtifacts[{index}]")
        _require_exact_keys(
            artifact,
            ("path", "rawSha256", "canonicalJsonSha256", "roles"),
            f"inputArtifacts[{index}]",
        )
        relative = _safe_relative_path(
            artifact["path"], f"inputArtifacts[{index}].path"
        )
        _require(relative not in artifact_by_path, f"duplicate input artifact path: {relative}")
        path = _repository_file(repository_root, relative, "input artifact path")
        _require(
            raw_sha256(path)
            == _require_sha256(
                artifact["rawSha256"], f"inputArtifacts[{index}].rawSha256"
            ),
            f"stale input artifact SHA: {relative}",
        )
        document = load_json(path)
        _require(
            canonical_json_sha256(document)
            == _require_sha256(
                artifact["canonicalJsonSha256"],
                f"inputArtifacts[{index}].canonicalJsonSha256",
            ),
            f"stale input artifact canonical SHA: {relative}",
        )
        roles = _require_list(artifact["roles"], f"inputArtifacts[{index}].roles")
        _require(
            roles
            and roles == sorted(set(roles))
            and all(STABLE_ID_RE.fullmatch(str(item)) for item in roles),
            f"inputArtifacts[{index}].roles are invalid",
        )
        artifact_by_path[relative] = artifact

    stage_by_key: dict[str, dict[str, Any]] = {}
    target_effects: set[str] = set()
    for index, stage in enumerate(_require_list(batch["stages"], "batch.stages")):
        stage = _require_dict(stage, f"stages[{index}]")
        stage_key = _require_stable_id(stage.get("stageKey"), f"stages[{index}].stageKey")
        _require(stage_key not in stage_by_key, f"duplicate stage key: {stage_key}")
        stage_by_key[stage_key] = stage
        mode = _require_string(stage.get("mode"), f"{stage_key}.mode")
        _require(
            mode in {TRACK_A_SELECTION_KIND, WARLORD_SELECTION_KIND},
            f"{stage_key} has unknown mode",
        )
        target = _require_dict(stage.get("target"), f"{stage_key}.target")
        _require_exact_keys(
            target,
            (
                "path",
                "effectAssetId",
                "requiredOutputVersion",
                "candidateBaseline",
                "legacyRollbackBaseline",
                "blueprint",
            ),
            f"{stage_key}.target",
        )
        target_path = _safe_relative_path(target.get("path"), f"{stage_key}.target.path")
        effect_id = _require_stable_id(
            target.get("effectAssetId"), f"{stage_key}.target.effectAssetId"
        )
        _require(effect_id not in target_effects, f"duplicate target Effect stage: {effect_id}")
        target_effects.add(effect_id)
        _require(
            target.get("requiredOutputVersion") == 13
            and effect_id.endswith(".unified")
            and target_path
            == f"Data/Effects/Authored/{effect_id}.effect.json",
            f"{stage_key} unified target identity changed",
        )
        rollback = _require_dict(
            target.get("legacyRollbackBaseline"),
            f"{stage_key}.target.legacyRollbackBaseline",
        )
        _require_exact_keys(
            rollback,
            (
                "path",
                "effectAssetId",
                "rawSha256",
                "canonicalJsonSha256",
                "authoringVersion",
                "policy",
            ),
            f"{stage_key}.target.legacyRollbackBaseline",
        )
        rollback_path = _safe_relative_path(
            rollback.get("path"), f"{stage_key}.rollback.path"
        )
        rollback_effect_id = _require_stable_id(
            rollback.get("effectAssetId"), f"{stage_key}.rollback.effectAssetId"
        )
        _require(
            rollback.get("policy") == "IMMUTABLE_LEGACY_ROLLBACK_EXACT"
            and rollback_effect_id == effect_id.removesuffix(".unified")
            and rollback_path
            == f"Data/Effects/Authored/{rollback_effect_id}.effect.json",
            f"{stage_key} legacy rollback identity changed",
        )
        rollback_file = _repository_file(
            repository_root, rollback_path, f"{stage_key}.rollback.path"
        )
        rollback_document = load_json(rollback_file)
        _require(
            raw_sha256(rollback_file)
            == _require_sha256(
                rollback.get("rawSha256"), f"{stage_key}.rollback.rawSha256"
            )
            and canonical_json_sha256(rollback_document)
            == _require_sha256(
                rollback.get("canonicalJsonSha256"),
                f"{stage_key}.rollback.canonicalJsonSha256",
            )
            and rollback_document.get("effectAssetId") == rollback_effect_id,
            f"legacy rollback baseline changed: {rollback_path}",
        )
        registered_rollback = artifact_by_path.get(rollback_path)
        _require(
            registered_rollback is not None
            and registered_rollback["rawSha256"] == rollback["rawSha256"]
            and registered_rollback["canonicalJsonSha256"]
            == rollback["canonicalJsonSha256"],
            f"legacy rollback baseline is not registered exactly once: {rollback_path}",
        )

        candidate = _require_dict(
            target.get("candidateBaseline"),
            f"{stage_key}.target.candidateBaseline",
        )
        _require_exact_keys(
            candidate,
            (
                "policy",
                "expectedRawSha256",
                "expectedCanonicalJsonSha256",
                "authoringVersion",
            ),
            f"{stage_key}.target.candidateBaseline",
        )
        candidate_path = _repository_path(
            repository_root, target_path, f"{stage_key}.target.path"
        )
        candidate_policy = _require_string(
            candidate.get("policy"), f"{stage_key}.candidate.policy"
        )
        if candidate_policy == "MUST_NOT_EXIST":
            _require(
                candidate.get("expectedRawSha256") is None
                and candidate.get("expectedCanonicalJsonSha256") is None
                and candidate.get("authoringVersion") is None
                and not candidate_path.exists()
                and target_path not in artifact_by_path,
                f"unified candidate must not exist: {target_path}",
            )
        elif candidate_policy == "EXPECTED_EXACT_OR_REFUSE":
            candidate_document = load_json(
                _repository_file(
                    repository_root, target_path, f"{stage_key}.target.path"
                )
            )
            registered_candidate = artifact_by_path.get(target_path)
            _require(
                candidate.get("authoringVersion") == 13
                and candidate_document.get("effectAssetId") == effect_id
                and registered_candidate is not None
                and registered_candidate["rawSha256"]
                == _require_sha256(
                    candidate.get("expectedRawSha256"),
                    f"{stage_key}.candidate.expectedRawSha256",
                )
                and registered_candidate["canonicalJsonSha256"]
                == _require_sha256(
                    candidate.get("expectedCanonicalJsonSha256"),
                    f"{stage_key}.candidate.expectedCanonicalJsonSha256",
                ),
                f"unified candidate baseline changed: {target_path}",
            )
        else:
            raise ContractError(
                f"{stage_key} has unknown candidate baseline policy: {candidate_policy}"
            )
        _require(
            stage.get("productMutation") is False
            and stage.get("visualApproval") is False,
            f"{stage_key} cannot claim product/visual approval",
        )
        source_artifacts = _require_dict(
            stage.get("sourceArtifacts"), f"{stage_key}.sourceArtifacts"
        )
        for artifact_name in (
            "manifest",
            "sourceReceipt",
            "importedDocument",
            "conversionReceipt",
        ):
            reference = _require_dict(
                source_artifacts.get(artifact_name),
                f"{stage_key}.sourceArtifacts.{artifact_name}",
            )
            relative = _safe_relative_path(
                reference.get("path"),
                f"{stage_key}.sourceArtifacts.{artifact_name}.path",
            )
            registered = artifact_by_path.get(relative)
            _require(registered is not None, f"unregistered stage artifact: {relative}")
            _require(
                registered["rawSha256"] == reference.get("rawSha256")
                and registered["canonicalJsonSha256"]
                == reference.get("canonicalJsonSha256"),
                f"stage artifact identity mismatch: {relative}",
            )
        selection = _require_dict(stage.get("selection"), f"{stage_key}.selection")
        selection_artifact = _require_dict(
            selection.get("artifact"), f"{stage_key}.selection.artifact"
        )
        selection_path = _safe_relative_path(
            selection_artifact.get("path"),
            f"{stage_key}.selection.artifact.path",
        )
        registered_selection = artifact_by_path.get(selection_path)
        _require(
            registered_selection is not None
            and registered_selection["rawSha256"]
            == selection_artifact.get("rawSha256")
            and registered_selection["canonicalJsonSha256"]
            == selection_artifact.get("canonicalJsonSha256"),
            f"stage selection artifact identity mismatch: {selection_path}",
        )
        source_selection_receipt = selection.get("sourceSelectionReceipt")
        if source_selection_receipt is not None:
            source_selection_receipt = _require_dict(
                source_selection_receipt,
                f"{stage_key}.selection.sourceSelectionReceipt",
            )
            receipt_path = _safe_relative_path(
                source_selection_receipt.get("path"),
                f"{stage_key}.selection.sourceSelectionReceipt.path",
            )
            registered_receipt = artifact_by_path.get(receipt_path)
            _require(
                registered_receipt is not None
                and registered_receipt["rawSha256"]
                == source_selection_receipt.get("rawSha256")
                and registered_receipt["canonicalJsonSha256"]
                == source_selection_receipt.get("canonicalJsonSha256"),
                f"stage selection receipt identity mismatch: {receipt_path}",
            )

    plan_ids: set[str] = set()
    target_elements: set[tuple[str, str]] = set()
    plan_ids_by_stage: dict[str, list[str]] = {key: [] for key in stage_by_key}
    material_counts: Counter[str] = Counter()
    carrier_counts: Counter[str] = Counter()
    source_document_cache: dict[str, dict[str, Any]] = {}
    for index, plan in enumerate(
        _require_list(batch["elementPlans"], "batch.elementPlans")
    ):
        plan = _require_dict(plan, f"elementPlans[{index}]")
        plan_id = _require_stable_id(plan.get("planId"), f"elementPlans[{index}].planId")
        _require(plan_id not in plan_ids, f"duplicate element plan ID: {plan_id}")
        plan_ids.add(plan_id)
        stage_key = _require_stable_id(
            plan.get("stageKey"), f"elementPlans[{index}].stageKey"
        )
        _require(stage_key in stage_by_key, f"element plan references unknown stage: {stage_key}")
        plan_ids_by_stage[stage_key].append(plan_id)
        target = _require_dict(plan.get("target"), f"{plan_id}.target")
        target_path = _safe_relative_path(target.get("documentPath"), f"{plan_id}.target.documentPath")
        stage_target = stage_by_key[stage_key]["target"]
        _require(
            target_path == stage_target["path"]
            and target.get("effectAssetId") == stage_target["effectAssetId"],
            f"{plan_id} targets another stage document",
        )
        target_key = (
            _require_stable_id(target.get("effectAssetId"), f"{plan_id}.target.effectAssetId"),
            _require_stable_id(target.get("elementId"), f"{plan_id}.target.elementId"),
        )
        _require(target_key not in target_elements, f"duplicate target Element import: {target_key}")
        target_elements.add(target_key)
        material_kind = _validate_material_disposition(
            plan.get("materialDisposition"), f"{plan_id}.materialDisposition"
        )
        material_counts[material_kind] += 1
        carrier = _require_dict(
            plan.get("carrierDisposition"), f"{plan_id}.carrierDisposition"
        )
        _require_exact_keys(carrier, ("kind", "blockers"), f"{plan_id}.carrierDisposition")
        carrier_kind = _require_string(carrier["kind"], f"{plan_id}.carrierDisposition.kind")
        _require(
            carrier_kind in EXPECTED_CARRIER_COUNTS,
            f"{plan_id} has unknown carrier disposition",
        )
        blockers = _require_list(carrier["blockers"], f"{plan_id}.carrierDisposition.blockers")
        _require(
            blockers and blockers == sorted(set(blockers)),
            f"{plan_id} carrier blockers must be sorted/unique",
        )
        carrier_counts[carrier_kind] += 1
        source = _require_dict(plan.get("source"), f"{plan_id}.source")
        source_kind = _require_string(source.get("kind"), f"{plan_id}.source.kind")
        _require(
            source_kind in {"IMPORTED_ELEMENT", "VISUAL_PROGRAM_SUPPLEMENTAL"},
            f"{plan_id} has unknown source kind",
        )
        if source_kind == "IMPORTED_ELEMENT":
            source_path = _safe_relative_path(
                source.get("documentPath"), f"{plan_id}.source.documentPath"
            )
            _require(source_path in artifact_by_path, f"{plan_id} source document is unregistered")
            _require(
                artifact_by_path[source_path]["rawSha256"]
                == _require_sha256(
                    source.get("documentRawSha256"),
                    f"{plan_id}.source.documentRawSha256",
                ),
                f"{plan_id} source document SHA changed",
            )
            source_document = source_document_cache.get(source_path)
            if source_document is None:
                source_document = load_json(
                    _repository_file(
                        repository_root, source_path, f"{plan_id}.source.documentPath"
                    )
                )
                source_document_cache[source_path] = source_document
            _require(
                source_document.get("effectAssetId")
                == source.get("effectAssetId"),
                f"{plan_id} source Effect identity changed",
            )
            source_element_id = _require_stable_id(
                source.get("elementId"), f"{plan_id}.source.elementId"
            )
            source_matches = [
                item
                for item in _require_list(
                    source_document.get("elements"),
                    f"{plan_id}.source.document.elements",
                )
                if isinstance(item, dict) and item.get("id") == source_element_id
            ]
            _require(
                len(source_matches) == 1,
                f"{plan_id} source Element identity is missing/duplicate",
            )
            source_element = source_matches[0]
            _require(
                canonical_json_sha256(source_element)
                == _require_sha256(
                    source.get("elementCanonicalSha256"),
                    f"{plan_id}.source.elementCanonicalSha256",
                )
                and canonical_json_sha256(source_element["sourceRecipe"])
                == _require_sha256(
                    source.get("sourceRecipeCanonicalSha256"),
                    f"{plan_id}.source.sourceRecipeCanonicalSha256",
                )
                and canonical_json_sha256(source_element["detail"])
                == _require_sha256(
                    source.get("sourceDetailCanonicalSha256"),
                    f"{plan_id}.source.sourceDetailCanonicalSha256",
                )
                and canonical_json_sha256(source_element["actionCueAttachment"])
                == _require_sha256(
                    source.get("sourceAttachmentCanonicalSha256"),
                    f"{plan_id}.source.sourceAttachmentCanonicalSha256",
                ),
                f"{plan_id} source Element snapshot is stale",
            )
        _require(
            plan.get("productMutation") is False
            and plan.get("visualApproval") is False,
            f"{plan_id} cannot claim product/visual approval",
        )

    for stage_key, stage in stage_by_key.items():
        declared = _require_list(stage.get("elementPlanIds"), f"{stage_key}.elementPlanIds")
        _require(
            len(declared) == stage.get("elementPlanCount")
            and declared == plan_ids_by_stage[stage_key]
            and len(declared) == len(set(declared)),
            f"{stage_key} element plan membership changed",
        )

    _require(batch["denominators"] == EXPECTED_DENOMINATORS, "batch denominator changed")
    normalized_material_counts = {
        key: material_counts[key] for key in EXPECTED_MATERIAL_COUNTS
    }
    _require(
        normalized_material_counts == EXPECTED_MATERIAL_COUNTS
        == batch["materialDispositionCounts"],
        "material disposition count changed",
    )
    _require(
        dict(sorted(carrier_counts.items())) == EXPECTED_CARRIER_COUNTS
        == batch["carrierDispositionCounts"],
        "carrier disposition count changed",
    )
    _require(
        len(stage_by_key) == EXPECTED_DENOMINATORS["stageCount"]
        and len(plan_ids) == EXPECTED_DENOMINATORS["elementPlanCount"],
        "stage/element plan denominator changed",
    )
    scope = _require_dict(batch["scope"], "batch.scope")
    _require_exact_keys(
        scope,
        (
            "characterClasses",
            "trackAProgramEffectAssetIds",
            "extensionCanaryEffectAssetId",
            "trackAUnifiedCandidateEffectAssetIds",
            "legacyStarterUnifiedCandidateEffectAssetIds",
            "unifiedCandidateEffectAssetIds",
            "artistFDirectSliceEffectAssetId",
            "artistFExcluded",
            "productCatalogMutation",
            "animationEventMutation",
        ),
        "batch.scope",
    )
    _require(
        scope.get("artistFExcluded") is True
        and scope.get("productCatalogMutation") is False
        and scope.get("animationEventMutation") is False
        and "effect.artist.skill.31470"
        not in scope.get("trackAProgramEffectAssetIds", []),
        "scope crossed Artist F or product mutation boundary",
    )
    track_a_candidates = _require_list(
        scope.get("trackAUnifiedCandidateEffectAssetIds"),
        "batch.scope.trackAUnifiedCandidateEffectAssetIds",
    )
    legacy_candidates = _require_list(
        scope.get("legacyStarterUnifiedCandidateEffectAssetIds"),
        "batch.scope.legacyStarterUnifiedCandidateEffectAssetIds",
    )
    all_candidates = _require_list(
        scope.get("unifiedCandidateEffectAssetIds"),
        "batch.scope.unifiedCandidateEffectAssetIds",
    )
    _require(
        track_a_candidates
        == [stage["target"]["effectAssetId"] for stage in batch["stages"]]
        and legacy_candidates
        == [
            row["target"]["effectAssetId"]
            for row in batch["legacyStarterCandidates"]
        ]
        and all_candidates == track_a_candidates + legacy_candidates
        and len(all_candidates) == len(set(all_candidates)) == 101
        and all(
            item.endswith(".unified")
            for item in all_candidates
        ),
        "scope unified candidate identities changed",
    )
    _require(
        scope.get("artistFDirectSliceEffectAssetId")
        == "effect.artist.skill.31470.unified",
        "Artist F direct slice boundary changed",
    )
    _validate_legacy_starter_contract(
        batch, artifact_by_path, repository_root
    )
    admission = _require_dict(batch["admission"], "batch.admission")
    _require(
        admission.get("productMappingMutation") is False
        and admission.get("visualApproval") is False,
        "batch admission overclaims product or visual approval",
    )
    transaction = _require_dict(
        batch["transactionPolicy"], "batch.transactionPolicy"
    )
    _require(
        transaction.get("commitMode")
        == "PARSE_VALIDATE_STAGE_THEN_CREATE_OR_EXACT_REPLACE_CANDIDATES_ONLY"
        and transaction.get("failureAction")
        == "PRESERVE_PREVIOUS_BATCH_LEGACY_BASELINES_AND_CANDIDATES"
        and transaction.get("candidateBaselinePolicy")
        == "MUST_NOT_EXIST_OR_EXPECTED_EXACT_OR_REFUSE"
        and transaction.get("authoredDocumentMutation") is False
        and transaction.get("catalogMutation") is False
        and transaction.get("animationEventMutation") is False,
        "batch transaction crossed unified candidate safety boundary",
    )


def atomic_write(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb",
            dir=path.parent,
            prefix=f".{path.name}.",
            suffix=".tmp",
            delete=False,
        ) as output:
            temporary_path = Path(output.name)
            output.write(payload)
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary_path, path)
        temporary_path = None
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def build_and_write(
    repository_root: Path = REPOSITORY_ROOT,
    output_path: Path | None = None,
) -> dict[str, Any]:
    repository_root = repository_root.resolve()
    destination = (
        output_path
        if output_path is not None
        else repository_root / DEFAULT_OUTPUT_RELATIVE_PATH
    )
    batch = build_batch(repository_root)
    validate_batch(batch, repository_root)
    payload = pretty_json_bytes(batch)

    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb",
            dir=destination.parent,
            prefix=f".{destination.name}.",
            suffix=".tmp",
            delete=False,
        ) as output:
            temporary_path = Path(output.name)
            output.write(payload)
            output.flush()
            os.fsync(output.fileno())
        staged = load_json(temporary_path)
        validate_batch(staged, repository_root)
        _require(
            pretty_json_bytes(staged) == payload,
            "staged batch changed after JSON round trip",
        )
        os.replace(temporary_path, destination)
        temporary_path = None
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)
    return batch


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=REPOSITORY_ROOT,
        help="LostArk repository root",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="Override generated batch path",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Validate that the tracked output is the current deterministic batch",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repository_root = args.repository_root.resolve()
    output = (
        args.output.resolve()
        if args.output is not None
        else repository_root / DEFAULT_OUTPUT_RELATIVE_PATH
    )
    batch = build_batch(repository_root)
    expected = pretty_json_bytes(batch)
    if args.check:
        _require(output.is_file(), f"batch output is missing: {output}")
        actual = output.read_bytes()
        _require(actual == expected, f"batch output is stale: {output}")
        validate_batch(load_json(output), repository_root)
        print(
            "Four-class Track A authored import batch check PASS: "
            f"{batch['denominators']}"
        )
        return 0
    build_and_write(repository_root, output)
    print(
        "Four-class Track A authored import batch generated: "
        f"{batch['denominators']}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ContractError as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
