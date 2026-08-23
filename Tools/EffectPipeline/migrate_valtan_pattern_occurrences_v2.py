#!/usr/bin/env python3
"""Migrate Valtan animation bindings and Effect cues to occurrence format v2.

The migration is deliberately mechanical.  It preserves the merged clip order,
records row-level Git provenance, and attaches every legacy stage cue to the
first persisted clip occurrence.  A legacy cue end that extends beyond that
clip's natural source duration becomes ``natural``: Valtan already stops every
owned occurrence at the Server stage edge, while an out-of-segment v2 cue end
would be an invalid source-time claim.

This tool never derives a product animation sequence from the source extraction.
Reviewed source-driven changes use a separate SOURCE_REVIEWED_DELTA edit.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import re
import struct
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
VALTAN_ROOT = REPOSITORY_ROOT / "Data/Animation/Authored/Valtan"
BINDINGS_PATH = VALTAN_ROOT / "Valtan.patternbindings.json"
CUES_PATH = VALTAN_ROOT / "Valtan.patterneffectcues.json"
ANIM_NOTIFY_PATH = (
    REPOSITORY_ROOT / "Data/Animation/Reference/Valtan/Valtan.animnotify"
)
RECEIPT_PATH = VALTAN_ROOT / "Valtan.pattern-occurrence-v2-migration.receipt.json"
CARRIER_V1_RECEIPT_PATH = (
    REPOSITORY_ROOT
    / "Data/Effects/Imported/Valtan/CarrierV1"
    / "Valtan.carrier-v1-materialization-receipt.v1.json"
)
CARRIER_V1_RECEIPT_REPOSITORY_PATH = (
    "Data/Effects/Imported/Valtan/CarrierV1/"
    "Valtan.carrier-v1-materialization-receipt.v1.json"
)

ANIMATION_PR_127_ACTIONS = frozenset(
    {
        "valtan.attack.down-smash.active",
        "valtan.attack.down-smash.recovery",
        "valtan.attack.down-smash.windup",
        "valtan.attack.earthquake-smash.delayed",
        "valtan.attack.earthquake-smash.impact",
        "valtan.attack.earthquake-smash.recovery",
        "valtan.attack.earthquake-smash.windup",
        "valtan.attack.fist-in-out.outer",
        "valtan.attack.fist-in-out.recovery",
        "valtan.attack.four-slash.active",
        "valtan.attack.four-slash.recovery",
        "valtan.attack.four-slash.windup",
        "valtan.attack.front-back-front.active",
        "valtan.attack.front-back-front.recovery",
        "valtan.attack.front-back-front.windup",
        "valtan.attack.ground-wave-smash.active",
        "valtan.attack.ground-wave-smash.windup",
        "valtan.attack.high-jump.recovery",
        "valtan.attack.swing.active",
        "valtan.attack.swing.recovery",
        "valtan.attack.swing.windup",
    }
)

PATTERN_PR_REFERENCE_ACTIONS = frozenset(
    {
        "valtan.attack.dash-charge.groggy",
        "valtan.attack.dash-charge.part-break",
        "valtan.mechanic.armor-break-opening.part-break",
    }
)

EXPECTED_BINDING_COUNT = 124
EXPECTED_CLIP_OCCURRENCE_COUNT = 128
EXPECTED_CUE_COUNT = 99
EXPECTED_CARRIER_V1_CUE_COUNT = 44
EXPECTED_CARRIER_V1_RETIRED_CUE_COUNT = 105
EXPECTED_CARRIER_V1_REPLACEMENT_MAPPING_COUNT = 48
EXPECTED_CARRIER_V1_BLOCKED_MAPPING_COUNT = 57
EXPECTED_CARRIER_V1_CLIP_CUE_COUNT = 43
FOUR_SLASH_OWNER_RESEAL_SCHEMA = (
    "lostark.valtan-carrier-v1-four-slash-owner-reseal-proof"
)
FOUR_SLASH_SOURCE_ACTION_ID = 420609
FOUR_SLASH_SOURCE_BRANCH_ID = (
    "valtan_four_slash.source-420609.mn_rpbf_00.sequence-003.stages-008-010"
)
FOUR_SLASH_SOURCE_SEQUENCE_CANONICAL_SHA256 = (
    "c615170e4e095d2e53c8b22799e88847d244a60ab6f0140ebe0c8c671afef220"
)
FOUR_SLASH_OLD_CUE_CANONICAL_SHA256 = (
    "6b0ce162e27eab4f8839c1aaec1e88e202afb314addde04864fa18252fde9b83"
)
FOUR_SLASH_NEW_CUE_CANONICAL_SHA256 = (
    "4ff3c88cffdbe84abb99aaee22aad86c92f1b1797dfd8706058ded489b738dc9"
)
FOUR_SLASH_CLIP01_SCREEN_POST_OVERLAY_SCHEMA = (
    "lostark.valtan-carrier-v1-clip01-screen-post-successor-overlay"
)
FOUR_SLASH_CLIP01_EFFECT_ID = (
    "effect.valtan.carrier-v1.attack.four-slash.active.clip-01"
)
FOUR_SLASH_CLIP01_DOCUMENT_REPOSITORY_PATH = (
    "Data/Effects/Authored/"
    "effect.valtan.carrier-v1.attack.four-slash.active.clip-01.effect.json"
)
FOUR_SLASH_CLIP01_DOCUMENT_PATH = (
    REPOSITORY_ROOT / FOUR_SLASH_CLIP01_DOCUMENT_REPOSITORY_PATH
)
FOUR_SLASH_CLIP01_BASE_ELEMENT_COUNT = 12
FOUR_SLASH_CLIP01_BASE_CANONICAL_SHA256 = (
    "5fe8332a267686b77224bb8311de7a79af4da6ca032c8429775ea579aa4baf10"
)
FOUR_SLASH_CLIP01_SCREEN_POST_ELEMENT_COUNT = 2
FOUR_SLASH_CLIP01_FINAL_ELEMENT_COUNT = 14
FOUR_SLASH_CLIP01_FINAL_CANONICAL_SHA256 = (
    "9ef084e2ef13cccbbde5febf5f9a5e4f4c4099e93dd066f026675dca7801ab6c"
)
FOUR_SLASH_CLIP02_SCREEN_POST_OCCURRENCE_ID = (
    "occurrence.0b1f192b7838db7bbb5ca80a"
)
FOUR_SLASH_CLIP02_SCREEN_POST_FULL_KEY = (
    "occurrence-key.0b1f192b7838db7bbb5ca80a"
    "12913fd8a36f4475074c8bc6cf66172e1fb3333d"
)
FOUR_SLASH_CLIP01_SCREEN_POST_ELEMENTS = (
    {
        "elementId": "occurrence.c627ba06ba0a8d5d48086907",
        "displayName": "FilmNoise [PROJECT_TUNED_APPROX]",
        "profileId": "screen.film-noise.reconstructed.v1",
        "fidelity": "PROJECT_TUNED_APPROX",
        "sourceTimeSeconds": 3.144474983215332,
        "lifeTimeSeconds": 0.35,
    },
    {
        "elementId": "occurrence.14794cdb89c73ee33f1dead3",
        "displayName": "ZoomBlur [BOUNDED_RECONSTRUCTED]",
        "profileId": "screen.zoom-blur.reconstructed.v1",
        "fidelity": "BOUNDED_RECONSTRUCTED",
        "sourceTimeSeconds": 3.144474983215332,
        "lifeTimeSeconds": 0.35,
    },
)
FOUR_SLASH_CLIP01_SCREEN_POST_RUNTIME_CONTRACTS = (
    {
        "elementId": "occurrence.c627ba06ba0a8d5d48086907",
        "groupId": "fx_post.fx_par.par_c_filmnoise_01",
        "sourceNode": (
            "fx_post.fx_par.par_c_filmnoise_01|"
            "FX_POST.fx_par.par_c_filmnoise_01.particlespriteemitter_0|"
            "occurrence.c627ba06ba0a8d5d48086907"
        ),
        "sourceMaterialPath": "fx_mi.fx_c_pa_filmnoise_01_tr",
        "sourceObjectPath": (
            "FX_POST.fx_par.par_c_filmnoise_01.particlespriteemitter_0"
        ),
        "sourceActionCueId": "action-420609/stage-008/notify-025",
        "sourceOccurrenceFullKey": (
            "occurrence-key.c627ba06ba0a8d5d48086907"
            "319ea4861dfd4521c7f532dff8eeb8befdc17957"
        ),
        "sourceNotifyOrdinal": 15,
        "intensity": 0.08,
        "secondaryIntensity": 0.02,
        "frequency": 1.0,
        "tint": (1.0, 1.0, 1.0, 1.0),
        "randomSeed": 42060925,
    },
    {
        "elementId": "occurrence.14794cdb89c73ee33f1dead3",
        "groupId": "fx_post.fx_par.par_c_zoomblur_03",
        "sourceNode": (
            "fx_post.fx_par.par_c_zoomblur_03|"
            "FX_POST.fx_par.par_c_zoomblur_03.particlespriteemitter_0|"
            "occurrence.14794cdb89c73ee33f1dead3"
        ),
        "sourceMaterialPath": "fx_mi.fx_c_pa_zoomblur_01_tr",
        "sourceObjectPath": (
            "FX_POST.fx_par.par_c_zoomblur_03.particlespriteemitter_0"
        ),
        "sourceActionCueId": "action-420609/stage-008/notify-026",
        "sourceOccurrenceFullKey": (
            "occurrence-key.14794cdb89c73ee33f1dead3"
            "a920092369cde4326f3d0b29a41c484ff99e6cb6"
        ),
        "sourceNotifyOrdinal": 16,
        "intensity": 0.0,
        "secondaryIntensity": 0.0,
        "frequency": 0.0,
        "tint": (1.0, 1.0, 1.0, 1.0),
        "randomSeed": 42060926,
    },
)
FOUR_SLASH_OWNER_TRANSFERS = (
    {
        "sourceStageOrdinal": 8,
        "retiredBindingId": "cue.valtan.four-slash.active",
        "replacementBindingId": (
            "cue.valtan.carrier-v1.attack.four-slash.active.clip-01"
        ),
        "occurrenceId": (
            "cue.valtan.carrier-v1.attack.four-slash.active.clip-01.occurrence.01"
        ),
        "effectAssetId": (
            "effect.valtan.carrier-v1.attack.four-slash.active.clip-01"
        ),
        "clipOccurrenceId": "valtan.attack.four-slash.active.clip.01",
        "oldOwner": {
            "patternId": "VALTAN_FOUR_SLASH",
            "stageId": "SLASHES",
            "actionId": "valtan.attack.four-slash.active",
        },
        "newOwner": {
            "patternId": "VALTAN_TRIPLE_SLASH",
            "stageId": "SLASHES",
            "actionId": "valtan.attack.triple-slash.active",
        },
    },
    {
        "sourceStageOrdinal": 9,
        "retiredBindingId": "cue.valtan.four-slash.active.clip-02",
        "replacementBindingId": (
            "cue.valtan.carrier-v1.attack.four-slash.active.clip-02"
        ),
        "occurrenceId": (
            "cue.valtan.carrier-v1.attack.four-slash.active.clip-02.occurrence.01"
        ),
        "effectAssetId": (
            "effect.valtan.carrier-v1.attack.four-slash.active.clip-02"
        ),
        "clipOccurrenceId": "valtan.attack.four-slash.active.clip.02",
        "oldOwner": {
            "patternId": "VALTAN_FOUR_SLASH",
            "stageId": "SLASHES",
            "actionId": "valtan.attack.four-slash.active",
        },
        "newOwner": {
            "patternId": "VALTAN_ROTATION_SLASH",
            "stageId": "SPIN",
            "actionId": "valtan.attack.rotation-slash.active",
        },
    },
    {
        "sourceStageOrdinal": 10,
        "retiredBindingId": "cue.valtan.four-slash.recovery",
        "replacementBindingId": (
            "cue.valtan.carrier-v1.attack.four-slash.recovery.clip-01"
        ),
        "occurrenceId": (
            "cue.valtan.carrier-v1.attack.four-slash.recovery.clip-01.occurrence.01"
        ),
        "effectAssetId": (
            "effect.valtan.carrier-v1.attack.four-slash.recovery.clip-01"
        ),
        "clipOccurrenceId": "valtan.attack.four-slash.recovery.clip.01",
        "oldOwner": {
            "patternId": "VALTAN_FOUR_SLASH",
            "stageId": "RECOVERY",
            "actionId": "valtan.attack.four-slash.recovery",
        },
        "newOwner": {
            "patternId": "VALTAN_ROTATION_SLASH",
            "stageId": "RECOVERY",
            "actionId": "valtan.attack.four-slash.recovery",
        },
    },
)
RETIRED_BASELINE_CUES = (
    {
        "bindingId": "cue.valtan.front-back-front.windup",
        "occurrenceId": (
            "cue.valtan.front-back-front.windup.occurrence.01"
        ),
        "reason": "LEGACY_CLIP_AGGREGATE_CARRIER_COLLAPSE",
        "replacementOwnerKind": "ANIMATION_EFFECT_CUE",
        "replacementBindingId": (
            "cue.valtan.front-back-front.v1-watertrail-audition"
        ),
        "replacementOccurrenceId": (
            "cue.valtan.front-back-front.v1-watertrail-audition.occurrence.01"
        ),
        "replacementEffectAssetId": (
            "effect.valtan.front-back-front.v1-watertrail-audition"
        ),
        "effectAssetId": "effect.valtan.front-back-front.windup",
    },
    {
        "bindingId": "cue.valtan.red-blade-wave.active",
        "occurrenceId": "cue.valtan.red-blade-wave.active.occurrence.01",
        "reason": "COMBAT_OBJECT_OWNERSHIP_TRANSFER",
        "replacementOwnerKind": "BOSS_COMBAT_OBJECT",
        "combatObjectArchetypeId": (
            "combatobject.valtan.red-blade-wave.projectile"
        ),
        "clientVisualId": (
            "combatobject.visual.valtan.red-blade-wave.projectile.v1"
        ),
        "effectAssetId": "effect.valtan.red-blade-wave.active",
    },
)
STABLE_TOKEN = re.compile(r"^[A-Za-z0-9_.-]+$")
ANIM_NOTIFY_ROW = re.compile(
    r'^"(?P<clip>[^"]+)".*\slen=(?P<seconds>[0-9]+(?:\.[0-9]+)?)'
)


class MigrationError(RuntimeError):
    pass


def read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8-sig") as handle:
        value = json.load(handle)
    if not isinstance(value, dict):
        raise MigrationError(f"Expected an object document: {path}")
    return value


def pretty_bytes(value: dict[str, Any]) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2) + "\n"
    ).encode("utf-8")


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def canonical_sha256(value: Any) -> str:
    """Match the Carrier V1 publisher's canonical JSON hash contract."""

    return sha256_bytes(
        (
            json.dumps(
                value,
                ensure_ascii=False,
                sort_keys=True,
                separators=(",", ":"),
                allow_nan=False,
            )
            + "\n"
        ).encode("utf-8")
    )


def float32_bits(value: Any) -> str | None:
    """Return the runtime ABI float bits, independent of JSON spelling."""

    if isinstance(value, bool) or not isinstance(value, (int, float)):
        return None
    try:
        runtime_value = struct.unpack("<f", struct.pack("<f", float(value)))[0]
    except (OverflowError, struct.error, ValueError):
        return None
    if not math.isfinite(runtime_value):
        return None
    return struct.pack("<f", runtime_value).hex()


def same_float32(actual: Any, expected: Any) -> bool:
    expected_bits = float32_bits(expected)
    return expected_bits is not None and float32_bits(actual) == expected_bits


def same_float32_sequence(actual: Any, expected: Any) -> bool:
    return (
        isinstance(actual, list)
        and len(actual) == len(expected)
        and all(
            same_float32(actual_value, expected_value)
            for actual_value, expected_value in zip(
                actual, expected, strict=True
            )
        )
    )


def source_parameter(
    source: dict[str, Any], name: str
) -> dict[str, Any] | None:
    parameters = source.get("parameters")
    if not isinstance(parameters, list):
        return None
    matches = [
        row
        for row in parameters
        if isinstance(row, dict) and row.get("name") == name
    ]
    return matches[0] if len(matches) == 1 else None


def expected_four_slash_owner_reseal_proof() -> dict[str, Any]:
    return {
        "schema": FOUR_SLASH_OWNER_RESEAL_SCHEMA,
        "formatVersion": 1,
        "sourceActionId": FOUR_SLASH_SOURCE_ACTION_ID,
        "sourceBranchId": FOUR_SLASH_SOURCE_BRANCH_ID,
        "sourceSequenceCanonicalSha256": (
            FOUR_SLASH_SOURCE_SEQUENCE_CANONICAL_SHA256
        ),
        "oldCueCanonicalSha256": FOUR_SLASH_OLD_CUE_CANONICAL_SHA256,
        "newCueCanonicalSha256": FOUR_SLASH_NEW_CUE_CANONICAL_SHA256,
        "ownerTransfers": copy.deepcopy(list(FOUR_SLASH_OWNER_TRANSFERS)),
    }


def expected_four_slash_clip01_screen_post_overlay() -> dict[str, Any]:
    return {
        "schema": FOUR_SLASH_CLIP01_SCREEN_POST_OVERLAY_SCHEMA,
        "formatVersion": 1,
        "effectAssetId": FOUR_SLASH_CLIP01_EFFECT_ID,
        "path": FOUR_SLASH_CLIP01_DOCUMENT_REPOSITORY_PATH,
        "policy": (
            "APPEND_ONLY_TYPED_SCREEN_POST_SUCCESSOR; "
            "MATERIALIZER_TARGET_REMAINS_HISTORICAL_PREIMAGE"
        ),
        "materializerPreimage": {
            "elementCount": FOUR_SLASH_CLIP01_BASE_ELEMENT_COUNT,
            "canonicalSha256": FOUR_SLASH_CLIP01_BASE_CANONICAL_SHA256,
        },
        "appendedElements": copy.deepcopy(
            list(FOUR_SLASH_CLIP01_SCREEN_POST_ELEMENTS)
        ),
        "finalDocument": {
            "elementCount": FOUR_SLASH_CLIP01_FINAL_ELEMENT_COUNT,
            "canonicalSha256": FOUR_SLASH_CLIP01_FINAL_CANONICAL_SHA256,
        },
    }


def validate_four_slash_clip01_screen_post_overlay(
    carrier_receipt: dict[str, Any],
    document: dict[str, Any] | None = None,
) -> dict[str, Any]:
    proof = carrier_receipt.get("clip01ScreenPostSuccessorOverlay")
    if proof != expected_four_slash_clip01_screen_post_overlay():
        raise MigrationError(
            "Carrier V1 clip-01 ScreenPost successor proof drifted."
        )

    target_rows = (carrier_receipt.get("outputs") or {}).get(
        "targetDocuments"
    )
    if not isinstance(target_rows, list):
        raise MigrationError("Carrier V1 target document ledger is invalid.")
    matching_targets = [
        row
        for row in target_rows
        if isinstance(row, dict)
        and row.get("effectAssetId") == FOUR_SLASH_CLIP01_EFFECT_ID
    ]
    expected_preimage = {
        "effectAssetId": FOUR_SLASH_CLIP01_EFFECT_ID,
        "path": FOUR_SLASH_CLIP01_DOCUMENT_REPOSITORY_PATH,
        "elementCount": FOUR_SLASH_CLIP01_BASE_ELEMENT_COUNT,
        "canonicalSha256": FOUR_SLASH_CLIP01_BASE_CANONICAL_SHA256,
    }
    if matching_targets != [expected_preimage]:
        raise MigrationError(
            "Carrier V1 clip-01 materializer preimage seal drifted."
        )

    if document is None:
        document = read_json(FOUR_SLASH_CLIP01_DOCUMENT_PATH)
    if (
        document.get("effectAssetId") != FOUR_SLASH_CLIP01_EFFECT_ID
        or not isinstance(document.get("elements"), list)
    ):
        raise MigrationError("Carrier V1 clip-01 authored document is invalid.")
    # The receipt's finalDocument seal records the exact 12-row materializer
    # preimage plus the two rows at the moment this successor was created.  It
    # is historical provenance, not a lock on later Effect Tool authoring.
    # Users may hide, delete, tune or add non-ScreenPost rows.  Seal only the
    # two typed ScreenPost rows so those edits cannot silently remove or mutate
    # the Valtan presentation contract.
    elements = document["elements"]
    protected_ids = {
        row["elementId"] for row in FOUR_SLASH_CLIP01_SCREEN_POST_ELEMENTS
    }
    screen_post_projection = [
        element
        for element in elements
        if isinstance(element, dict) and element.get("id") in protected_ids
    ]
    try:
        serialized_elements = json.dumps(
            elements, sort_keys=True, allow_nan=False
        )
    except (TypeError, ValueError) as error:
        raise MigrationError(
            "Carrier V1 clip-01 authored elements are not canonical JSON."
        ) from error
    if (
        FOUR_SLASH_CLIP02_SCREEN_POST_OCCURRENCE_ID in serialized_elements
        or FOUR_SLASH_CLIP02_SCREEN_POST_FULL_KEY in serialized_elements
    ):
        raise MigrationError(
            "Carrier V1 clip-02 ScreenPost occurrence contaminated clip-01."
        )
    expected_ids = tuple(
        row["elementId"] for row in FOUR_SLASH_CLIP01_SCREEN_POST_ELEMENTS
    )
    if tuple(row.get("id") for row in screen_post_projection) != expected_ids:
        raise MigrationError(
            "Carrier V1 clip-01 ScreenPost protected identity/order drifted."
        )
    for element, expected, runtime in zip(
        screen_post_projection,
        FOUR_SLASH_CLIP01_SCREEN_POST_ELEMENTS,
        FOUR_SLASH_CLIP01_SCREEN_POST_RUNTIME_CONTRACTS,
        strict=True,
    ):
        detail = element.get("detail") or {}
        timing = detail.get("timing") or {}
        screen_post = detail.get("screenPost") or {}
        source = element.get("sourcePresentation") or {}
        material = element.get("material") or {}
        source_system = source_parameter(source, "sourceSystemId") or {}
        source_occurrence = source_parameter(source, "sourceOccurrenceId") or {}
        source_full_key = source_parameter(
            source, "sourceOccurrenceFullKey"
        ) or {}
        source_clip = source_parameter(
            source, "sourceClipOccurrenceId"
        ) or {}
        source_notify = source_parameter(source, "sourceNotifyOrdinal") or {}
        if (
            element.get("id") != expected["elementId"]
            or element.get("id") != runtime["elementId"]
            or element.get("displayName") != expected["displayName"]
            or element.get("groupId") != runtime["groupId"]
            or element.get("sourceNode") != runtime["sourceNode"]
            or element.get("visible") is not True
            or element.get("kind") != "screenPost"
            or element.get("resources") != []
            or material.get("templateId") != "effect.standard"
            or material.get("sourceMaterialPath")
            != runtime["sourceMaterialPath"]
            or material.get("renderProfile")
            != "alpha_two_sided_depth_read"
            or source.get("enabled") is not True
            or source.get("schema")
            != "lostark.effect-source-presentation"
            or source.get("version") != 1
            or source.get("status") != "reconstructed"
            or source.get("sourceEventId") != expected["elementId"]
            or source.get("profileId") != expected["profileId"]
            or source.get("sourceObjectPath")
            != runtime["sourceObjectPath"]
            or source.get("sourceActionCueId")
            != runtime["sourceActionCueId"]
            or source.get("sourceOccurrenceIndex") != 0
            or not same_float32(
                source.get("sourceTimeSeconds"),
                expected["sourceTimeSeconds"],
            )
            or source_system.get("type") != "string"
            or source_system.get("status") != "source_explicit"
            or source_system.get("stringValue") != runtime["groupId"]
            or source_occurrence.get("type") != "string"
            or source_occurrence.get("status") != "source_explicit"
            or source_occurrence.get("stringValue")
            != expected["elementId"]
            or source_full_key.get("type") != "string"
            or source_full_key.get("status") != "source_explicit"
            or source_full_key.get("stringValue")
            != runtime["sourceOccurrenceFullKey"]
            or source_clip.get("type") != "string"
            or source_clip.get("status") != "source_explicit"
            or source_clip.get("stringValue")
            != "valtan.attack.four-slash.active.clip.01"
            or source_notify.get("type") != "number"
            or source_notify.get("status") != "source_explicit"
            or not same_float32(
                source_notify.get("numberValue"),
                runtime["sourceNotifyOrdinal"],
            )
            or not same_float32(
                timing.get("startDelaySeconds"),
                expected["sourceTimeSeconds"],
            )
            or not same_float32(
                timing.get("lifeTimeSeconds"),
                expected["lifeTimeSeconds"],
            )
            or not same_float32(timing.get("afterImageSeconds"), 0.0)
            or not same_float32(
                timing.get("dissolveStartNormalized"), 1.0
            )
            or screen_post.get("enabled") is not True
            or screen_post.get("profileId") != expected["profileId"]
            or screen_post.get("status") != "reconstructed_profile"
            or not same_float32(
                screen_post.get("intensity"), runtime["intensity"]
            )
            or not same_float32(
                screen_post.get("secondaryIntensity"),
                runtime["secondaryIntensity"],
            )
            or not same_float32(
                screen_post.get("frequency"), runtime["frequency"]
            )
            or not same_float32_sequence(
                screen_post.get("tint"), runtime["tint"]
            )
            or type(screen_post.get("randomSeed")) is not int
            or screen_post.get("randomSeed") != runtime["randomSeed"]
        ):
            raise MigrationError(
                "Carrier V1 clip-01 ScreenPost runtime/identity drifted: "
                f"{expected['elementId']}"
            )
    return proof


def validate_four_slash_owner_reseal_proof(
    carrier_receipt: dict[str, Any],
) -> dict[str, dict[str, Any]]:
    proof = carrier_receipt.get("fourSlashPatternSplitOwnerReseal")
    if proof != expected_four_slash_owner_reseal_proof():
        raise MigrationError(
            "Carrier V1 four-slash owner reseal proof drifted."
        )
    transfers = proof["ownerTransfers"]
    by_replacement = {
        row["replacementBindingId"]: row for row in transfers
    }
    if len(by_replacement) != len(FOUR_SLASH_OWNER_TRANSFERS):
        raise MigrationError(
            "Carrier V1 four-slash owner transfer identity is duplicated."
        )
    return by_replacement


def require_token(value: Any, label: str) -> str:
    if not isinstance(value, str) or not STABLE_TOKEN.fullmatch(value):
        raise MigrationError(f"Invalid stable token for {label}: {value!r}")
    return value


def mapping_basis(action_id: str) -> str:
    if action_id in ANIMATION_PR_127_ACTIONS:
        return "ANIMATION_PR_127"
    if action_id in PATTERN_PR_REFERENCE_ACTIONS:
        return "PATTERN_PR_REFERENCE"
    return "CURRENT_PRODUCT_BASELINE"


def clip_occurrence_id(action_id: str, ordinal: int) -> str:
    return f"{action_id}.clip.{ordinal:02d}"


def normalize_legacy_clips(value: Any, action_id: str) -> list[str]:
    if isinstance(value, str):
        clips = [value]
    elif isinstance(value, list):
        clips = value
    else:
        raise MigrationError(f"Invalid legacy clip chain: {action_id}")
    if not clips or len(clips) > 16:
        raise MigrationError(f"Invalid legacy clip count: {action_id}")
    return [require_token(clip, f"{action_id} clip") for clip in clips]


def migrate_bindings(document: dict[str, Any]) -> dict[str, Any]:
    if document.get("schema") != "lostark.valtan-pattern-bindings":
        raise MigrationError("Unexpected Valtan pattern binding schema.")
    if document.get("formatVersion") != 1:
        raise MigrationError("Binding migration requires formatVersion 1 input.")
    rows = document.get("bindings")
    if not isinstance(rows, list) or len(rows) != EXPECTED_BINDING_COUNT:
        raise MigrationError("Unexpected Valtan pattern binding denominator.")

    migrated: list[dict[str, Any]] = []
    claimed_actions: set[str] = set()
    claimed_occurrences: set[str] = set()
    for row in rows:
        if not isinstance(row, dict) or set(row) != {"actionId", "clip"}:
            raise MigrationError("Legacy pattern binding has unexpected fields.")
        action_id = require_token(row.get("actionId"), "actionId")
        if action_id in claimed_actions:
            raise MigrationError(f"Duplicate actionId: {action_id}")
        claimed_actions.add(action_id)
        clips = normalize_legacy_clips(row.get("clip"), action_id)
        migrated_clips: list[dict[str, Any]] = []
        for index, clip in enumerate(clips, start=1):
            occurrence_id = clip_occurrence_id(action_id, index)
            if occurrence_id in claimed_occurrences:
                raise MigrationError(f"Duplicate clip occurrence: {occurrence_id}")
            claimed_occurrences.add(occurrence_id)
            migrated_clips.append(
                {
                    "clipOccurrenceId": occurrence_id,
                    "clip": clip,
                    "mappingBasis": mapping_basis(action_id),
                    "sourceStartMs": 0,
                    "playMs": 0,
                    "playRate": 1.0,
                    # This explicitly preserves the v1 runtime's last-clip loop.
                    "loop": index == len(clips),
                }
            )
        migrated.append({"actionId": action_id, "clips": migrated_clips})

    if len(claimed_occurrences) != EXPECTED_CLIP_OCCURRENCE_COUNT:
        raise MigrationError("Unexpected Valtan clip occurrence denominator.")
    return {
        "schema": document["schema"],
        "formatVersion": 2,
        "bossArchetypeId": document.get("bossArchetypeId"),
        "bindings": migrated,
    }


def load_clip_durations_ms() -> dict[str, int]:
    durations: dict[str, int] = {}
    for line in ANIM_NOTIFY_PATH.read_text(encoding="utf-8-sig").splitlines():
        match = ANIM_NOTIFY_ROW.match(line)
        if match is None:
            continue
        # Cue times are integer milliseconds.  Flooring prevents a 1.4667 s
        # source clip from admitting a 1467 ms cue end that the runtime model
        # timeline must reject as out of segment.
        milliseconds = int(float(match.group("seconds")) * 1000.0 + 1.0e-9)
        clip = match.group("clip")
        durations[clip] = max(milliseconds, durations.get(clip, 0))
    if not durations:
        raise MigrationError("Valtan animnotify yielded no clip durations.")
    return durations


def migrate_cues(
    document: dict[str, Any],
    bindings: dict[str, Any],
) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    if document.get("schema") != "lostark.valtan-pattern-effect-cues":
        raise MigrationError("Unexpected Valtan cue schema.")
    if document.get("formatVersion") != 1:
        raise MigrationError("Cue migration requires formatVersion 1 input.")
    rows = document.get("cues")
    if not isinstance(rows, list) or len(rows) != EXPECTED_CUE_COUNT:
        raise MigrationError("Unexpected Valtan cue denominator.")

    binding_by_action = {
        row["actionId"]: row for row in bindings["bindings"]
    }
    clip_durations = load_clip_durations_ms()
    migrated: list[dict[str, Any]] = []
    promoted_natural: list[dict[str, Any]] = []
    occurrence_ids: set[str] = set()

    legacy_fields = {
        "bindingId",
        "patternId",
        "stageId",
        "actionId",
        "effectAssetId",
        "anchorSlotId",
        "followPolicy",
        "stopPolicy",
        "startMs",
        "endMs",
        "localTransform",
    }
    for row in rows:
        if not isinstance(row, dict) or set(row) != legacy_fields:
            raise MigrationError("Legacy Valtan cue has unexpected fields.")
        binding_id = require_token(row.get("bindingId"), "bindingId")
        action_id = require_token(row.get("actionId"), "cue actionId")
        action_binding = binding_by_action.get(action_id)
        if action_binding is None:
            raise MigrationError(f"Cue action has no animation binding: {action_id}")
        first_clip = action_binding["clips"][0]
        source_start_ms = row.get("startMs")
        if not isinstance(source_start_ms, int) or source_start_ms < 0:
            raise MigrationError(f"Invalid cue start: {binding_id}")
        duration_ms = clip_durations.get(first_clip["clip"])
        if duration_ms is None or source_start_ms >= duration_ms:
            raise MigrationError(
                f"Cue start is outside the first persisted clip: {binding_id}"
            )

        stop_policy = row.get("stopPolicy")
        source_end_ms = row.get("endMs")
        if stop_policy == "natural":
            if source_end_ms is not None:
                raise MigrationError(f"Natural legacy cue has an end: {binding_id}")
        elif stop_policy == "cue_end":
            if (
                not isinstance(source_end_ms, int)
                or source_end_ms <= source_start_ms
            ):
                raise MigrationError(f"Invalid legacy cue end: {binding_id}")
            if source_end_ms > duration_ms:
                promoted_natural.append(
                    {
                        "bindingId": binding_id,
                        "legacyEndMs": source_end_ms,
                        "clipDurationMs": duration_ms,
                        "reason": "LEGACY_STAGE_WALL_END_OUTSIDE_SOURCE_CLIP",
                    }
                )
                stop_policy = "natural"
                source_end_ms = None
        else:
            raise MigrationError(f"Invalid legacy stop policy: {binding_id}")

        occurrence_id = f"{binding_id}.occurrence.01"
        if occurrence_id in occurrence_ids:
            raise MigrationError(f"Duplicate cue occurrence: {occurrence_id}")
        occurrence_ids.add(occurrence_id)
        migrated.append(
            {
                "bindingId": binding_id,
                "occurrenceId": occurrence_id,
                "patternId": row["patternId"],
                "stageId": row["stageId"],
                "actionId": action_id,
                "clipOccurrenceId": first_clip["clipOccurrenceId"],
                "effectAssetId": row["effectAssetId"],
                "anchorSlotId": row["anchorSlotId"],
                "followPolicy": row["followPolicy"],
                "stopPolicy": stop_policy,
                "repeatPolicy": "once",
                "sourceStartMs": source_start_ms,
                "sourceEndMs": source_end_ms,
                "localTransform": row["localTransform"],
            }
        )

    return (
        {
            "schema": document["schema"],
            "formatVersion": 2,
            "ownerArchetypeId": document.get("ownerArchetypeId"),
            "cues": migrated,
        },
        promoted_natural,
    )


def validate_v2(
    bindings: dict[str, Any],
    cues: dict[str, Any],
    *,
    require_migration_denominator: bool,
) -> tuple[int, int, int]:
    if bindings.get("formatVersion") != 2 or cues.get("formatVersion") != 2:
        raise MigrationError("Expected v2 Valtan documents.")
    binding_rows = bindings.get("bindings")
    cue_rows = cues.get("cues")
    if not isinstance(binding_rows, list) or not isinstance(cue_rows, list):
        raise MigrationError("Expected Valtan v2 row arrays.")
    occurrence_count = sum(len(row.get("clips", [])) for row in binding_rows)
    counts = (len(binding_rows), occurrence_count, len(cue_rows))
    migration_expected = (
        EXPECTED_BINDING_COUNT,
        EXPECTED_CLIP_OCCURRENCE_COUNT,
        EXPECTED_CUE_COUNT,
    )
    current_expected = (
        len(binding_rows),
        occurrence_count,
        EXPECTED_CARRIER_V1_CUE_COUNT,
    )
    if (
        require_migration_denominator
        and counts != migration_expected
    ) or (
        not require_migration_denominator
        and (
            counts[0] < EXPECTED_BINDING_COUNT
            or counts[1] < EXPECTED_CLIP_OCCURRENCE_COUNT
            or counts != current_expected
        )
    ):
        raise MigrationError(f"Unexpected v2 denominators: {counts}")
    return counts


def write_atomic(path: Path, payload: bytes) -> None:
    staging = path.with_suffix(path.suffix + ".staging")
    staging.write_bytes(payload)
    staging.replace(path)


def build_receipt(
    legacy_binding_sha: str,
    legacy_cue_sha: str,
    bindings: dict[str, Any],
    cues: dict[str, Any],
    binding_payload: bytes,
    cue_payload: bytes,
    promoted_natural: list[dict[str, Any]],
) -> dict[str, Any]:
    binding_identity = [
        {
            "actionId": row["actionId"],
            "clips": [
                {
                    "clipOccurrenceId": clip["clipOccurrenceId"],
                    "clip": clip["clip"],
                }
                for clip in row["clips"]
            ],
        }
        for row in bindings["bindings"]
    ]
    cue_identity_fields = (
        "bindingId",
        "occurrenceId",
        "patternId",
        "stageId",
        "actionId",
        "clipOccurrenceId",
        "effectAssetId",
    )
    cue_identity = [
        {field: row[field] for field in cue_identity_fields}
        for row in cues["cues"]
    ]
    return {
        "schema": "lostark.valtan-pattern-occurrence-v2-migration-receipt",
        "formatVersion": 2,
        "basis": {
            "animationSequenceMerge": "459da808",
            "patternReferenceMerge": "0b70f06e",
            "implementationBaseline": "dd382bf1c817eca3314067fbfac3628b47b617b2",
        },
        "inputs": {
            "legacyPatternBindingsSha256": legacy_binding_sha,
            "legacyPatternEffectCuesSha256": legacy_cue_sha,
        },
        "outputs": {
            "migrationBaselinePatternBindingsSha256": sha256_bytes(
                binding_payload
            ),
            "migrationBaselinePatternEffectCuesSha256": sha256_bytes(
                cue_payload
            ),
            "bindingCount": EXPECTED_BINDING_COUNT,
            "clipOccurrenceCount": EXPECTED_CLIP_OCCURRENCE_COUNT,
            "cueOccurrenceCount": EXPECTED_CUE_COUNT,
        },
        "baselineIdentity": {
            "bindings": binding_identity,
            "cues": cue_identity,
        },
        "legacyCueEndPromotions": promoted_natural,
        "policies": {
            "sourceSequenceInsertion": "FORBIDDEN_WITHOUT_SOURCE_REVIEWED_DELTA",
            "lastClipLoop": "EXPLICIT_V1_RUNTIME_PRESERVATION",
            "legacyCueRepeat": "once",
            "outOfSegmentCueEnd": "natural_until_server_stage_edge",
        },
    }


def build_carrier_v1_successor_contract(
    carrier_receipt: dict[str, Any],
    cues: dict[str, Any],
    baseline_cues: list[dict[str, Any]],
) -> dict[str, Any]:
    """Validate and summarize the Carrier V1 cue-owner replacement closure."""

    if (
        carrier_receipt.get("schema")
        != "lostark.valtan-carrier-v1-materialization-receipt"
        or carrier_receipt.get("formatVersion") != 1
        or carrier_receipt.get("bossArchetypeId") != "BOSS_VALTAN"
    ):
        raise MigrationError("Carrier V1 materialization receipt is invalid.")
    screen_post_overlay = validate_four_slash_clip01_screen_post_overlay(
        carrier_receipt
    )
    if (
        cues.get("schema") != "lostark.valtan-pattern-effect-cues"
        or cues.get("formatVersion") != 2
        or cues.get("ownerArchetypeId") != "BOSS_VALTAN"
    ):
        raise MigrationError("Current Valtan cue document header is invalid.")

    cue_rows = cues.get("cues")
    if (
        not isinstance(cue_rows, list)
        or len(cue_rows) != EXPECTED_CARRIER_V1_CUE_COUNT
        or any(not isinstance(row, dict) for row in cue_rows)
    ):
        raise MigrationError("Current Carrier V1 cue denominator drifted.")
    current_by_binding = {
        row.get("bindingId"): row for row in cue_rows
    }
    current_by_occurrence = {
        row.get("occurrenceId"): row for row in cue_rows
    }
    if (
        None in current_by_binding
        or None in current_by_occurrence
        or len(current_by_binding) != len(cue_rows)
        or len(current_by_occurrence) != len(cue_rows)
    ):
        raise MigrationError("Current Carrier V1 cue identity is duplicated.")
    for binding_id, row in current_by_binding.items():
        require_token(binding_id, "current cue bindingId")
        occurrence_id = require_token(
            row.get("occurrenceId"), "current cue occurrenceId"
        )
        if occurrence_id != f"{binding_id}.occurrence.01":
            raise MigrationError(
                f"Current Carrier V1 cue occurrence is rebound: {binding_id}"
            )

    outputs = carrier_receipt.get("outputs")
    cue_output = (outputs or {}).get("cues")
    current_cue_sha256 = canonical_sha256(cues)
    transfers_by_replacement = validate_four_slash_owner_reseal_proof(
        carrier_receipt
    )
    owner_reseal_proof = carrier_receipt[
        "fourSlashPatternSplitOwnerReseal"
    ]
    if (
        not isinstance(cue_output, dict)
        or cue_output.get("path")
        != "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        or cue_output.get("cueCount") != EXPECTED_CARRIER_V1_CUE_COUNT
        or cue_output.get("canonicalSha256") != current_cue_sha256
        or current_cue_sha256 != FOUR_SLASH_NEW_CUE_CANONICAL_SHA256
    ):
        raise MigrationError("Carrier V1 current cue canonical hash drifted.")
    if (
        (carrier_receipt.get("summary") or {}).get(
            "finalBossRootCueCount"
        )
        != EXPECTED_CARRIER_V1_CUE_COUNT
        or (carrier_receipt.get("productReset") or {}).get(
            "finalBossRootCueCount"
        )
        != EXPECTED_CARRIER_V1_CUE_COUNT
    ):
        raise MigrationError("Carrier V1 final cue summary drifted.")

    mappings = carrier_receipt.get("retiredOwnerSuccessorMappings")
    mapping_fields = {
        "retiredBindingId",
        "retiredEffectAssetId",
        "patternId",
        "stageId",
        "actionId",
        "clipOccurrenceId",
        "disposition",
        "replacementBindingId",
        "replacementEffectAssetId",
    }
    if (
        not isinstance(mappings, list)
        or len(mappings) != EXPECTED_CARRIER_V1_RETIRED_CUE_COUNT
        or any(
            not isinstance(row, dict) or set(row) != mapping_fields
            for row in mappings
        )
    ):
        raise MigrationError("Carrier V1 successor mapping denominator drifted.")
    mappings_by_retired = {
        row.get("retiredBindingId"): row for row in mappings
    }
    if (
        None in mappings_by_retired
        or len(mappings_by_retired) != len(mappings)
    ):
        raise MigrationError("Carrier V1 retired cue identity is duplicated.")
    retired_ids = set(mappings_by_retired)
    legacy_migration = carrier_receipt.get("legacyMigration") or {}
    sealed_retired_ids = legacy_migration.get("retiredCueBindingIds")
    if (
        not isinstance(sealed_retired_ids, list)
        or len(sealed_retired_ids) != EXPECTED_CARRIER_V1_RETIRED_CUE_COUNT
        or set(sealed_retired_ids) != retired_ids
        or (carrier_receipt.get("productReset") or {}).get(
            "retiredBossRootCueCount"
        )
        != EXPECTED_CARRIER_V1_RETIRED_CUE_COUNT
    ):
        raise MigrationError("Carrier V1 retired cue seal drifted.")
    if retired_ids & set(current_by_binding):
        raise MigrationError("A Carrier V1 retired cue was restored.")

    replacement_mappings = []
    blocked_mappings = []
    for retired_id, row in mappings_by_retired.items():
        require_token(retired_id, "retired cue bindingId")
        for field in (
            "retiredEffectAssetId",
            "patternId",
            "stageId",
            "actionId",
            "clipOccurrenceId",
        ):
            require_token(row.get(field), f"{retired_id} {field}")
        disposition = row.get("disposition")
        replacement_id = row.get("replacementBindingId")
        replacement_effect = row.get("replacementEffectAssetId")
        if disposition == "REPLACED_BY_EXACT_CARRIER_V1_CLIP_OWNER":
            replacement_id = require_token(
                replacement_id, f"{retired_id} replacementBindingId"
            )
            replacement_effect = require_token(
                replacement_effect, f"{retired_id} replacementEffectAssetId"
            )
            current = current_by_binding.get(replacement_id)
            if not isinstance(current, dict) or any(
                current.get(field) != row.get(source_field)
                for field, source_field in (
                    ("patternId", "patternId"),
                    ("stageId", "stageId"),
                    ("actionId", "actionId"),
                    ("clipOccurrenceId", "clipOccurrenceId"),
                )
            ) or current.get("effectAssetId") != replacement_effect:
                raise MigrationError(
                    "Carrier V1 successor cue is missing or rebound: "
                    f"{replacement_id}"
                )
            replacement_mappings.append(row)
        elif disposition == "RETIRED_NO_EXACT_REVIEWED_CARRIER_OWNER":
            if replacement_id is not None or replacement_effect is not None:
                raise MigrationError(
                    f"Blocked Carrier V1 retirement gained a successor: {retired_id}"
                )
            blocked_mappings.append(row)
        else:
            raise MigrationError(
                f"Unknown Carrier V1 successor disposition: {disposition}"
            )
    if (
        len(replacement_mappings)
        != EXPECTED_CARRIER_V1_REPLACEMENT_MAPPING_COUNT
        or len(blocked_mappings) != EXPECTED_CARRIER_V1_BLOCKED_MAPPING_COUNT
    ):
        raise MigrationError("Carrier V1 successor disposition counts drifted.")

    transfers_by_retired = {
        row["retiredBindingId"]: row
        for row in transfers_by_replacement.values()
    }
    if len(transfers_by_retired) != len(FOUR_SLASH_OWNER_TRANSFERS):
        raise MigrationError(
            "Carrier V1 four-slash retired owner identity is duplicated."
        )
    owner_fields = ("patternId", "stageId", "actionId")
    for replacement_id, transfer in transfers_by_replacement.items():
        retired_id = transfer["retiredBindingId"]
        mapping = mappings_by_retired.get(retired_id)
        current = current_by_binding.get(replacement_id)
        if (
            not isinstance(mapping, dict)
            or not isinstance(current, dict)
            or mapping.get("disposition")
            != "REPLACED_BY_EXACT_CARRIER_V1_CLIP_OWNER"
            or mapping.get("replacementBindingId") != replacement_id
            or mapping.get("replacementEffectAssetId")
            != transfer["effectAssetId"]
            or mapping.get("clipOccurrenceId")
            != transfer["clipOccurrenceId"]
            or current.get("occurrenceId") != transfer["occurrenceId"]
            or current.get("effectAssetId") != transfer["effectAssetId"]
            or current.get("clipOccurrenceId")
            != transfer["clipOccurrenceId"]
            or any(
                mapping.get(field) != transfer["newOwner"][field]
                or current.get(field) != transfer["newOwner"][field]
                for field in owner_fields
            )
        ):
            raise MigrationError(
                "Carrier V1 four-slash successor proof is not joined: "
                f"{replacement_id}"
            )

    clip_groups = carrier_receipt.get("clipGroups")
    if not isinstance(clip_groups, list) or len(clip_groups) != 44:
        raise MigrationError("Carrier V1 clip owner denominator drifted.")
    cue_groups = [
        row
        for row in clip_groups
        if isinstance(row, dict)
        and row.get("owner") == "VALTAN_PATTERN_EFFECT_CUE"
    ]
    combat_groups = [
        row
        for row in clip_groups
        if isinstance(row, dict)
        and row.get("owner") == "BOSS_CATALOG_COMBAT_OBJECT"
    ]
    if (
        len(cue_groups) != EXPECTED_CARRIER_V1_CLIP_CUE_COUNT
        or len(combat_groups) != 1
        or combat_groups[0].get("cueBindingId") is not None
    ):
        raise MigrationError("Carrier V1 clip owner kinds drifted.")
    clip_cue_ids: set[str] = set()
    for group in cue_groups:
        binding_id = require_token(
            group.get("cueBindingId"), "Carrier V1 clip cue bindingId"
        )
        current = current_by_binding.get(binding_id)
        if binding_id in clip_cue_ids or not isinstance(current, dict):
            raise MigrationError(
                f"Carrier V1 clip cue owner is duplicated or missing: {binding_id}"
            )
        clip_cue_ids.add(binding_id)
        if any(
            current.get(field) != group.get(source_field)
            for field, source_field in (
                ("patternId", "patternId"),
                ("stageId", "semanticStageId"),
                ("actionId", "gameplayActionId"),
                ("clipOccurrenceId", "clipOccurrenceId"),
                ("effectAssetId", "effectAssetId"),
            )
        ):
            raise MigrationError(
                f"Carrier V1 clip cue owner is rebound: {binding_id}"
            )

    exceptions = carrier_receipt.get("ownershipExceptions")
    protected = next(
        (
            row
            for row in exceptions
            if isinstance(row, dict)
            and row.get("disposition")
            == "PRESERVE_9_ROW_CANARY_RESEAL_2_WMODEL_SCALE_FIELDS"
        ),
        None,
    ) if isinstance(exceptions, list) else None
    if not isinstance(protected, dict):
        raise MigrationError("Carrier V1 protected cue exception is missing.")
    protected_rows = [
        row
        for row in cue_rows
        if row.get("effectAssetId") == protected.get("effectAssetId")
        and row.get("clipOccurrenceId") == protected.get("clipOccurrenceId")
    ]
    if (
        len(protected_rows) != 1
        or canonical_sha256(protected_rows[0])
        != protected.get("cueRowCanonicalSha256")
    ):
        raise MigrationError("Carrier V1 protected cue owner seal drifted.")
    protected_binding_id = protected_rows[0]["bindingId"]
    if set(current_by_binding) != clip_cue_ids | {protected_binding_id}:
        raise MigrationError("Carrier V1 current cue owner set drifted.")

    baseline_by_binding = {
        row.get("bindingId"): row for row in baseline_cues
    }
    if (
        None in baseline_by_binding
        or len(baseline_by_binding) != EXPECTED_CUE_COUNT
    ):
        raise MigrationError("Migration baseline cue identity is duplicated.")
    legacy_retired_ids = {
        row["bindingId"] for row in RETIRED_BASELINE_CUES
    }
    carrier_retired_baseline_ids = retired_ids & set(baseline_by_binding)
    if carrier_retired_baseline_ids & legacy_retired_ids:
        raise MigrationError("Baseline cue was retired by two authorities.")
    for binding_id in carrier_retired_baseline_ids:
        baseline = baseline_by_binding[binding_id]
        mapping = mappings_by_retired[binding_id]
        transfer = transfers_by_retired.get(binding_id)
        retired_owner = transfer["oldOwner"] if transfer else mapping
        if (
            any(
                retired_owner.get(field) != baseline.get(field)
                for field in owner_fields
            )
            or mapping.get("clipOccurrenceId")
            != baseline.get("clipOccurrenceId")
            or mapping.get("retiredEffectAssetId")
            != baseline.get("effectAssetId")
            or mapping.get("retiredBindingId") != binding_id
            or (
                transfer is not None
                and transfer.get("clipOccurrenceId")
                != baseline.get("clipOccurrenceId")
            )
        ):
            raise MigrationError(
                f"Carrier V1 retired baseline cue is rebound: {binding_id}"
            )

    surviving_baseline_ids = (
        set(baseline_by_binding)
        - legacy_retired_ids
        - carrier_retired_baseline_ids
    )
    if (
        len(carrier_retired_baseline_ids) != 96
        or len(legacy_retired_ids) != 2
        or len(surviving_baseline_ids) != 1
    ):
        raise MigrationError("Carrier V1 baseline successor denominator drifted.")
    identity_fields = (
        "bindingId",
        "occurrenceId",
        "patternId",
        "stageId",
        "actionId",
        "clipOccurrenceId",
        "effectAssetId",
    )
    for binding_id in surviving_baseline_ids:
        baseline = baseline_by_binding[binding_id]
        current = current_by_binding.get(binding_id)
        if not isinstance(current, dict) or any(
            current.get(field) != baseline.get(field)
            for field in identity_fields
        ):
            raise MigrationError(
                f"Surviving baseline cue was removed or rebound: {binding_id}"
            )

    for retired in RETIRED_BASELINE_CUES:
        replacement_id = retired.get("replacementBindingId")
        if replacement_id is None:
            continue
        replacement_retirement = mappings_by_retired.get(replacement_id)
        if replacement_id not in current_by_binding and not isinstance(
            replacement_retirement, dict
        ):
            raise MigrationError(
                "Legacy retired cue successor chain is broken: "
                f"{replacement_id}"
            )
        if isinstance(replacement_retirement, dict) and (
            replacement_retirement.get("retiredEffectAssetId")
            != retired.get("replacementEffectAssetId")
        ):
            raise MigrationError(
                "Legacy retired cue successor chain is rebound: "
                f"{replacement_id}"
            )

    replacement_ids = {
        row["replacementBindingId"] for row in replacement_mappings
    }
    new_clip_cues = clip_cue_ids - replacement_ids
    if len(replacement_ids) != 42 or len(new_clip_cues) != 1:
        raise MigrationError("Carrier V1 successor owner coverage drifted.")

    return {
        "carrierReceiptPath": CARRIER_V1_RECEIPT_REPOSITORY_PATH,
        "carrierReceiptCanonicalSha256": canonical_sha256(carrier_receipt),
        "clip01ScreenPostOverlayProofCanonicalSha256": canonical_sha256(
            screen_post_overlay
        ),
        "clip01ScreenPostFinalDocumentCanonicalSha256": (
            FOUR_SLASH_CLIP01_FINAL_CANONICAL_SHA256
        ),
        "currentCueCanonicalSha256": current_cue_sha256,
        "fourSlashOwnerResealProofCanonicalSha256": canonical_sha256(
            owner_reseal_proof
        ),
        "fourSlashOwnerTransferCount": len(transfers_by_replacement),
        "fourSlashSourceBranchId": FOUR_SLASH_SOURCE_BRANCH_ID,
        "fourSlashSourceSequenceCanonicalSha256": (
            FOUR_SLASH_SOURCE_SEQUENCE_CANONICAL_SHA256
        ),
        "fourSlashOldCueCanonicalSha256": (
            FOUR_SLASH_OLD_CUE_CANONICAL_SHA256
        ),
        "fourSlashNewCueCanonicalSha256": (
            FOUR_SLASH_NEW_CUE_CANONICAL_SHA256
        ),
        "currentCueCount": len(cue_rows),
        "retiredCueCount": len(mappings),
        "replacementMappingCount": len(replacement_mappings),
        "retiredWithoutSuccessorCount": len(blocked_mappings),
        "uniqueReplacementBindingCount": len(replacement_ids),
        "carrierClipCueCount": len(clip_cue_ids),
        "newCarrierCueWithoutRetiredPredecessorCount": len(new_clip_cues),
        "carrierRetiredBaselineCueCount": len(
            carrier_retired_baseline_ids
        ),
        "legacyRetiredBaselineCueCount": len(legacy_retired_ids),
        "survivingBaselineCueCount": len(surviving_baseline_ids),
        "postBaselineRetiredCueCount": len(retired_ids - set(baseline_by_binding)),
    }


def check_receipt(
    receipt: dict[str, Any],
    bindings: dict[str, Any],
    cues: dict[str, Any],
    carrier_receipt: dict[str, Any] | None = None,
) -> None:
    if receipt.get("schema") != (
        "lostark.valtan-pattern-occurrence-v2-migration-receipt"
    ) or receipt.get("formatVersion") != 2:
        raise MigrationError("Unexpected Valtan occurrence migration receipt.")
    outputs = receipt.get("outputs")
    if not isinstance(outputs, dict):
        raise MigrationError("Occurrence migration receipt has no outputs.")
    baseline_identity = receipt.get("baselineIdentity")
    if not isinstance(baseline_identity, dict):
        raise MigrationError("Occurrence migration receipt has no identity set.")
    baseline_bindings = baseline_identity.get("bindings")
    baseline_cues = baseline_identity.get("cues")
    retired_baseline_cues = receipt.get("retiredBaselineCues")
    if (
        not isinstance(baseline_bindings, list)
        or not isinstance(baseline_cues, list)
        or len(baseline_bindings) != outputs.get("bindingCount")
        or len(baseline_cues) != outputs.get("cueOccurrenceCount")
    ):
        raise MigrationError("Occurrence migration identity denominator drifted.")
    if retired_baseline_cues != list(RETIRED_BASELINE_CUES):
        raise MigrationError("Occurrence migration retirement ledger drifted.")
    if carrier_receipt is None:
        carrier_receipt = read_json(CARRIER_V1_RECEIPT_PATH)
    expected_carrier_contract = build_carrier_v1_successor_contract(
        carrier_receipt,
        cues,
        baseline_cues,
    )
    if receipt.get("carrierV1SuccessorContract") != expected_carrier_contract:
        raise MigrationError("Carrier V1 successor contract seal drifted.")

    current_bindings = {
        row.get("actionId"): row
        for row in bindings.get("bindings", [])
        if isinstance(row, dict)
    }
    moved_active_transfers = sorted(
        (
            row
            for row in FOUR_SLASH_OWNER_TRANSFERS
            if row["oldOwner"]["actionId"]
            == "valtan.attack.four-slash.active"
            and row["newOwner"]["actionId"]
            != row["oldOwner"]["actionId"]
        ),
        key=lambda row: row["sourceStageOrdinal"],
    )
    if len(moved_active_transfers) != 2:
        raise MigrationError("Four-slash active split proof denominator drifted.")
    baseline_clip_count = 0
    for baseline in baseline_bindings:
        if not isinstance(baseline, dict):
            raise MigrationError("Invalid baseline binding identity row.")
        action_id = baseline.get("actionId")
        current = current_bindings.get(action_id)
        baseline_clips = baseline.get("clips")
        if not isinstance(baseline_clips, list):
            raise MigrationError(
                f"Migrated binding identity was removed: {action_id}"
            )
        if action_id == "valtan.attack.four-slash.active":
            expected_occurrences = [
                row["clipOccurrenceId"] for row in moved_active_transfers
            ]
            if current is not None or [
                row.get("clipOccurrenceId")
                for row in baseline_clips
                if isinstance(row, dict)
            ] != expected_occurrences:
                raise MigrationError(
                    "Four-slash active baseline did not split by exact proof."
                )
            for baseline_clip, transfer in zip(
                baseline_clips, moved_active_transfers, strict=True
            ):
                target_action = transfer["newOwner"]["actionId"]
                target = current_bindings.get(target_action)
                target_clips = (
                    target.get("clips") if isinstance(target, dict) else None
                )
                if (
                    not isinstance(baseline_clip, dict)
                    or not isinstance(target_clips, list)
                    or len(target_clips) != 1
                    or target_clips[0].get("clipOccurrenceId")
                    != transfer["clipOccurrenceId"]
                    or target_clips[0].get("clip")
                    != baseline_clip.get("clip")
                    or target_clips[0].get("loop") is not False
                ):
                    raise MigrationError(
                        "Four-slash active clip split is missing or rebound: "
                        f"{transfer['clipOccurrenceId']}"
                    )
                baseline_clip_count += 1
            continue
        if not isinstance(current, dict):
            raise MigrationError(
                f"Migrated binding identity was removed: {action_id}"
            )
        current_clips = current.get("clips")
        if not isinstance(current_clips, list):
            raise MigrationError(f"Migrated binding clips are invalid: {action_id}")
        current_by_occurrence = {
            clip.get("clipOccurrenceId"): clip
            for clip in current_clips
            if isinstance(clip, dict)
        }
        current_order = [
            clip.get("clipOccurrenceId")
            for clip in current_clips
            if isinstance(clip, dict)
        ]
        baseline_order: list[str] = []
        for baseline_clip in baseline_clips:
            if not isinstance(baseline_clip, dict):
                raise MigrationError("Invalid baseline clip identity row.")
            occurrence_id = baseline_clip.get("clipOccurrenceId")
            current_clip = current_by_occurrence.get(occurrence_id)
            if (
                not isinstance(current_clip, dict)
                or current_clip.get("clip") != baseline_clip.get("clip")
            ):
                raise MigrationError(
                    "Migrated clip identity was removed or rebound: "
                    f"{occurrence_id}"
                )
            baseline_order.append(occurrence_id)
            baseline_clip_count += 1
        projected_order = [
            occurrence_id
            for occurrence_id in current_order
            if occurrence_id in set(baseline_order)
        ]
        if projected_order != baseline_order:
            raise MigrationError(
                f"Migrated clip order changed: {action_id}"
            )
    if baseline_clip_count != outputs.get("clipOccurrenceCount"):
        raise MigrationError("Occurrence migration clip denominator drifted.")

    cue_identity_fields = (
        "bindingId",
        "occurrenceId",
        "patternId",
        "stageId",
        "actionId",
        "clipOccurrenceId",
        "effectAssetId",
    )
    current_cues = {
        row.get("occurrenceId"): row
        for row in cues.get("cues", [])
        if isinstance(row, dict)
    }
    retired_by_occurrence = {
        row["occurrenceId"]: row for row in RETIRED_BASELINE_CUES
    }
    carrier_retired_by_binding = {
        row["retiredBindingId"]: row
        for row in carrier_receipt["retiredOwnerSuccessorMappings"]
    }
    for baseline in baseline_cues:
        if not isinstance(baseline, dict):
            raise MigrationError("Invalid baseline cue identity row.")
        occurrence_id = baseline.get("occurrenceId")
        current = current_cues.get(occurrence_id)
        retired = retired_by_occurrence.get(occurrence_id)
        if retired is not None:
            if (
                baseline.get("bindingId") != retired["bindingId"]
                or baseline.get("effectAssetId") != retired["effectAssetId"]
                or current is not None
                or any(
                    row.get("bindingId") == retired["bindingId"]
                    for row in cues.get("cues", [])
                    if isinstance(row, dict)
                )
            ):
                raise MigrationError(
                    "Retired migrated cue identity was restored or rebound: "
                    f"{occurrence_id}"
                )
            continue
        carrier_retired = carrier_retired_by_binding.get(
            baseline.get("bindingId")
        )
        if carrier_retired is not None:
            if (
                current is not None
                or any(
                    row.get("bindingId") == baseline.get("bindingId")
                    for row in cues.get("cues", [])
                    if isinstance(row, dict)
                )
            ):
                raise MigrationError(
                    "Carrier V1 retired migrated cue was restored: "
                    f"{occurrence_id}"
                )
            continue
        if not isinstance(current, dict) or any(
            current.get(field) != baseline.get(field)
            for field in cue_identity_fields
        ):
            raise MigrationError(
                "Migrated cue identity was removed or rebound: "
                f"{occurrence_id}"
            )


def upgrade_v1_receipt(
    receipt: dict[str, Any],
    bindings: dict[str, Any],
    cues: dict[str, Any],
) -> dict[str, Any]:
    if receipt.get("schema") != (
        "lostark.valtan-pattern-occurrence-v2-migration-receipt"
    ) or receipt.get("formatVersion") != 1:
        raise MigrationError("Receipt refresh requires the v1 migration receipt.")
    validate_v2(bindings, cues, require_migration_denominator=True)
    refreshed = build_receipt(
        receipt.get("inputs", {}).get("legacyPatternBindingsSha256", ""),
        receipt.get("inputs", {}).get("legacyPatternEffectCuesSha256", ""),
        bindings,
        cues,
        pretty_bytes(bindings),
        pretty_bytes(cues),
        receipt.get("legacyCueEndPromotions", []),
    )
    refreshed["basis"] = receipt.get("basis", refreshed["basis"])
    refreshed["policies"] = receipt.get("policies", refreshed["policies"])
    return refreshed


def main() -> int:
    parser = argparse.ArgumentParser()
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--refresh-receipt", action="store_true")
    args = parser.parse_args()

    bindings = read_json(BINDINGS_PATH)
    cues = read_json(CUES_PATH)
    if args.write:
        legacy_binding_payload = BINDINGS_PATH.read_bytes()
        legacy_cue_payload = CUES_PATH.read_bytes()
        migrated_bindings = migrate_bindings(bindings)
        migrated_cues, promoted = migrate_cues(cues, migrated_bindings)
        validate_v2(
            migrated_bindings,
            migrated_cues,
            require_migration_denominator=True,
        )
        binding_payload = pretty_bytes(migrated_bindings)
        cue_payload = pretty_bytes(migrated_cues)
        receipt = build_receipt(
            sha256_bytes(legacy_binding_payload),
            sha256_bytes(legacy_cue_payload),
            migrated_bindings,
            migrated_cues,
            binding_payload,
            cue_payload,
            promoted,
        )
        write_atomic(BINDINGS_PATH, binding_payload)
        write_atomic(CUES_PATH, cue_payload)
        write_atomic(RECEIPT_PATH, pretty_bytes(receipt))
        print(
            "Migrated Valtan occurrence v2: "
            f"{EXPECTED_BINDING_COUNT} bindings / "
            f"{EXPECTED_CLIP_OCCURRENCE_COUNT} clips / "
            f"{EXPECTED_CUE_COUNT} cues / "
            f"{len(promoted)} legacy cue ends promoted to natural."
        )
        return 0

    if not RECEIPT_PATH.is_file():
        raise MigrationError(f"Missing migration receipt: {RECEIPT_PATH}")
    if args.refresh_receipt:
        refreshed = upgrade_v1_receipt(
            read_json(RECEIPT_PATH), bindings, cues
        )
        write_atomic(RECEIPT_PATH, pretty_bytes(refreshed))
        print(
            "Refreshed Valtan occurrence migration receipt to identity-subset "
            "format v2."
        )
        return 0

    counts = validate_v2(
        bindings,
        cues,
        require_migration_denominator=False,
    )
    check_receipt(
        read_json(RECEIPT_PATH),
        bindings,
        cues,
    )
    print(
        "Valtan occurrence v2 check passed: "
        f"{counts[0]} bindings / {counts[1]} clips / {counts[2]} cues."
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except MigrationError as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
