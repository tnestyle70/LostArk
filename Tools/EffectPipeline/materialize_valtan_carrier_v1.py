#!/usr/bin/env python3
"""Transactionally replace Valtan's legacy aggregate rows with exact carriers.

This materializer deliberately stops at the carrier-first V1 boundary:

* only reviewed, reachable ``EXECUTABLE_CORE`` occurrence x carrier rows enter
  Product documents;
* Sprite, Mesh and Decal ownership comes from the selected source emitter,
  never from a ParticleSystem-wide mesh fallback;
* every newly materialized row uses the temporary common alpha-translucent
  renderer while retaining its exact material identity, resources and source
  recipe for a later typed-family upgrade;
* unresolved carriers stay receipt-only;
* every prior boss-root Valtan Product owner is retired, including legacy,
  approximate and project-authored rows; its physical document is retained as
  uncatalogued preimage evidence;

The 420633 Whirlwind active canary is the only row-preserved boss-root owner;
its two exact WModel rows are resealed to the repository's 0.01 source scale.
Combat-object-owned Red Blade active is rewritten to its five exact carriers;
the independent Sky Axe combat-object document is outside the boss-root reset.
Writes are staged and rolled back as one transaction.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Callable


ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "Tools" / "EffectPipeline"
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import build_valtan_reviewed_source_family_candidates as reviewed_candidates  # noqa: E402
import build_valtan_source_occurrence_inventory as source_inventory  # noqa: E402
import build_valtan_legacy_v0_carrier_migration_inventory as legacy_inventory_builder  # noqa: E402
import migrate_valtan_pattern_occurrences_v2 as occurrence_v2_migration  # noqa: E402
import valtan_carrier_v1_successor_lineage as successor_lineage  # noqa: E402


CATALOG_PATH = ROOT / "Data/Effects/EffectCatalog.json"
CUE_PATH = ROOT / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
BOSS_CATALOG_PATH = ROOT / "Data/Actors/BossCatalog.json"
AUTHORED_ROOT = ROOT / "Data/Effects/Authored"
LEGACY_DONOR_ROOT = ROOT / "Data/Effects/Imported/LegacyRuntimeDonors"
RECEIPT_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/CarrierV1"
    / "Valtan.carrier-v1-materialization-receipt.v1.json"
)
SUCCESSOR_RECEIPT_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/CarrierV1"
    / "Valtan.carrier-v1-successor-lineage-receipt.v1.json"
)
ADDITIVE_RECEIPT_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/CarrierV1"
    / "Valtan.carrier-v1-four-pillars-additive-receipt.v1.json"
)
LEGACY_INVENTORY_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/CarrierV1"
    / "Valtan.legacy-v0-carrier-migration-inventory.v1.json"
)

PROTECTED_CLIP_ID = "valtan.attack.whirlwind.active.clip.01"
PROTECTED_EFFECT_ID = "effect.valtan.pattern.420633.active"
RED_BLADE_CLIP_ID = "valtan.attack.red-blade-wave.active.clip.01"
RED_BLADE_EFFECT_ID = "effect.valtan.red-blade-wave.active"
WATERTRAIL_CANARY_EFFECT_ID = (
    "effect.valtan.front-back-front.v1-watertrail-audition"
)
FOUR_PILLARS_EFFECT_IDS = {
    "effect.valtan.carrier-v1.mechanic.four-pillars-105.takeoff.clip-01",
    "effect.valtan.carrier-v1.mechanic.four-pillars-105.target-cone.clip-01",
}
FOUR_PILLARS_CUE_IDS = {
    "cue.valtan.carrier-v1.mechanic.four-pillars-105.takeoff.clip-01",
    "cue.valtan.carrier-v1.mechanic.four-pillars-105.target-cone.clip-01",
}
ADDITIVE_BINDING_ROW_SEALS = {
    "valtan.mechanic.four-pillars-105.takeoff": {
        "successorBase": "e4163bfb60b4ebb57e18dc5f388bc2b4e0b4eebc3e55ccbcc93b3057f5fbf709",
        "liveAdditive": "a97d4f6e78516e4678825226be35a2b542f6c9403c3230e03eb592c3b4312de1",
    },
    "valtan.mechanic.four-pillars-105.target-cone": {
        "successorBase": "143fe16425863bf55c0b1cf3d2d6eda56132235e711270594eace245c8dcd596",
        "liveAdditive": "e4af8c0c69701d1cd8708593b7fa429701b9bfd108c65aeade1b289f2df80da4",
    },
    "valtan.mechanic.entrance-whirlwind.recovery": {
        "successorBase": "e6b5df45216658a732de83146872b8e9777a8addab9bd78ea767942da9b5cd6a",
        "liveAdditive": "35329c4bb231786970f0e5696c63d484caad0599b46f42d90653c1fa7baeded5",
    },
}
ADDITIVE_ENCOUNTER_ROW_SEALS = {
    "successorBase": "3e5db0bc40973607ce7120dd5894dceea1a4090a9c877c303a8f6dc23516c992",
    "liveAdditive": "42c9791868a2749ccc1665013e49ec3fd2e444bd11aa81fc2d3bcc4394969d72",
}
ADDITIVE_SELECTION_ROW_SHA256 = (
    "35873c214c57613b6424a09971bc2b1f318c42e58a78617b9ea5952b11838e0b"
)
DECAL_EXPANDED_CLIP_IDS = [
    "valtan.attack.ground-wave-smash.active.clip.02",
    "valtan.attack.jump-spin.jump.clip.01",
]
DECAL_EXPANDED_PATTERN_IDS = ["VALTAN_JUMP_SPIN"]

EXPECTED = {
    "reviewedCoreProjectionCount": 660,
    "reviewedCoreSpriteProjectionCount": 455,
    "reviewedCoreMeshProjectionCount": 173,
    "reviewedCoreDecalProjectionCount": 32,
    "reviewedCoreClipCount": 45,
    "reviewedCorePatternCount": 24,
    "reviewedProjectionLedgerCount": 1577,
    "protectedProjectionCount": 3,
    "materializedProjectionCount": 657,
    "materializedClipGroupCount": 44,
    "newClipDocumentCount": 43,
    "baselineExistingExactProjectionCount": 539,
    "baselineNewExactProjectionCount": 118,
    "baselineStrictLegacyRowCount": 3032,
    "baselineStrictLegacyDocumentCount": 97,
    "baselineValtanCatalogCount": 108,
    "baselineBossRootCueCount": 106,
    "baselineValtanProductRowCount": 3624,
    "baselineRemovedExistingRowCount": 3612,
    "baselineRetiredEffectCount": 105,
    "baselineRetiredCueCount": 105,
    "finalValtanCatalogCount": 46,
    "finalBossRootCueCount": 44,
    "blockedExpandedCarrierCount": 917,
    "reviewedOccurrenceWithoutCarrierCount": 197,
}

class MaterializeError(RuntimeError):
    pass


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise MaterializeError(f"cannot read JSON: {path}: {error}") from error
    if not isinstance(value, dict):
        raise MaterializeError(f"JSON root is not an object: {path}")
    return value


def pretty_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, sort_keys=False) + "\n"
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return source_inventory.canonical_sha256(value)


def _valtan_catalog_receipt_projection(
    catalog: dict[str, Any],
    successor_receipt: dict[str, Any],
) -> dict[str, Any]:
    """Seal only the catalog contract owned by this Valtan materializer."""

    try:
        return successor_lineage.project_historical_catalog(
            catalog, successor_receipt
        )
    except successor_lineage.SuccessorLineageError as error:
        raise MaterializeError(str(error)) from error


def _cue_receipt_projection(
    cues: dict[str, Any],
    successor_receipt: dict[str, Any],
) -> dict[str, Any]:
    try:
        return successor_lineage.project_historical_cues(
            cues, successor_receipt
        )
    except successor_lineage.SuccessorLineageError as error:
        raise MaterializeError(str(error)) from error


def _load_successor_receipt() -> dict[str, Any]:
    if not SUCCESSOR_RECEIPT_PATH.is_file():
        raise MaterializeError("Carrier V1 successor lineage receipt is missing")
    return read_json(SUCCESSOR_RECEIPT_PATH)


def _validate_successor_base_product(
    successor_receipt: dict[str, Any],
    catalog: dict[str, Any],
    cues: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    """Validate the PR203 live layer with FOUR_PILLARS projected out."""

    historical_receipt = read_json(RECEIPT_PATH)
    if canonical_sha256(historical_receipt) != BASE_RECEIPT_CANONICAL_SHA256:
        raise MaterializeError("immutable historical carrier receipt drifted")
    base_catalog = copy.deepcopy(catalog)
    base_catalog["effects"] = [
        copy.deepcopy(row)
        for row in catalog.get("effects", [])
        if str(row.get("effectAssetId") or "") not in FOUR_PILLARS_EFFECT_IDS
    ]
    base_cues = copy.deepcopy(cues)
    base_cues["cues"] = [
        copy.deepcopy(row)
        for row in cues.get("cues", [])
        if str(row.get("bindingId") or "") not in FOUR_PILLARS_CUE_IDS
    ]
    base_catalog_rows = _catalog_index(base_catalog)
    base_cue_rows = _cue_index(base_cues)
    if (
        sum(
            effect_id.startswith("effect.valtan.")
            for effect_id in base_catalog_rows
        )
        != 54
        or len(base_cue_rows) != 47
    ):
        raise MaterializeError(
            "100-bar additive Product baseline denominator drifted"
        )
    try:
        successor_lineage.validate_receipt(
            root=ROOT,
            receipt=successor_receipt,
            historical_receipt=historical_receipt,
            catalog=base_catalog,
            cues=base_cues,
        )
    except successor_lineage.SuccessorLineageError as error:
        raise MaterializeError(str(error)) from error
    return base_catalog, base_cues


def project_successor_base_inputs(
    successor_receipt: dict[str, Any],
    pattern_bindings: dict[str, Any],
    encounter: dict[str, Any],
    selection: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any], dict[str, Any]]:
    """Project the explicit additive input delta off the PR203 live layer."""

    base_bindings = copy.deepcopy(pattern_bindings)
    bindings_by_action = {
        str(row.get("actionId") or ""): row
        for row in base_bindings.get("bindings", [])
    }
    binding_transfers = []
    for action_id, seals in ADDITIVE_BINDING_ROW_SEALS.items():
        row = bindings_by_action.get(action_id)
        if row is None or canonical_sha256(row) != seals["liveAdditive"]:
            raise MaterializeError(
                "additive live pattern binding drifted: " + action_id
            )
        clips = row.get("clips")
        if not isinstance(clips, list) or len(clips) != 1:
            raise MaterializeError(
                "additive pattern binding clip closure drifted: " + action_id
            )
        clips[0]["loop"] = True
        if canonical_sha256(row) != seals["successorBase"]:
            raise MaterializeError(
                "additive successor-base pattern binding drifted: " + action_id
            )
        binding_transfers.append(
            {
                "actionId": action_id,
                "successorBaseRowCanonicalSha256": seals["successorBase"],
                "liveAdditiveRowCanonicalSha256": seals["liveAdditive"],
            }
        )

    base_encounter = copy.deepcopy(encounter)
    encounter_rows = {
        str(row.get("patternId") or ""): row
        for row in base_encounter.get("patterns", [])
    }
    four_pillars = encounter_rows.get("VALTAN_FOUR_PILLARS_105")
    if (
        four_pillars is None
        or canonical_sha256(four_pillars)
        != ADDITIVE_ENCOUNTER_ROW_SEALS["liveAdditive"]
    ):
        raise MaterializeError("additive live encounter row drifted")
    four_pillars["displayName"] = "100줄 중앙 착지와 4기둥 추적 원뿔"
    four_pillars.pop("serverMotion", None)
    stages = {
        str(row.get("stageId") or ""): row
        for row in four_pillars.get("stages", [])
    }
    if set(stages) != {"TAKEOFF", "YELLOW_ZONE", "TARGET_CONE", "RECOVERY"}:
        raise MaterializeError("additive encounter stage closure drifted")
    stages["TAKEOFF"]["durationMs"] = 1600
    stages["YELLOW_ZONE"]["durationMs"] = 900
    stages["TARGET_CONE"]["durationMs"] = 900
    stages["TARGET_CONE"]["hitDelayMs"] = 0
    if (
        canonical_sha256(four_pillars)
        != ADDITIVE_ENCOUNTER_ROW_SEALS["successorBase"]
    ):
        raise MaterializeError("additive successor-base encounter row drifted")

    base_selection = copy.deepcopy(selection)
    additive_selection_rows = [
        row
        for row in base_selection.get("selections", [])
        if row.get("patternId") == "VALTAN_FOUR_PILLARS_105"
    ]
    if (
        len(additive_selection_rows) != 1
        or canonical_sha256(additive_selection_rows[0])
        != ADDITIVE_SELECTION_ROW_SHA256
    ):
        raise MaterializeError("additive reviewed selection row drifted")
    base_selection["selections"] = [
        row
        for row in base_selection["selections"]
        if row.get("patternId") != "VALTAN_FOUR_PILLARS_105"
    ]

    try:
        successor_lineage.project_historical_pattern_bindings(
            base_bindings, successor_receipt
        )
        successor_lineage.project_historical_encounter(
            base_encounter, successor_receipt
        )
        successor_lineage.project_historical_selection_manifest(
            base_selection, successor_receipt
        )
    except successor_lineage.SuccessorLineageError as error:
        raise MaterializeError(str(error)) from error

    evidence = {
        "patternBindings": {
            "path": "Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
            "liveAdditiveCanonicalSha256": canonical_sha256(pattern_bindings),
            "successorBaseCanonicalSha256": canonical_sha256(base_bindings),
            "rowRebounds": binding_transfers,
        },
        "encounter": {
            "path": "Data/Encounters/Valtan/ValtanEncounter.json",
            "liveAdditiveCanonicalSha256": canonical_sha256(encounter),
            "successorBaseCanonicalSha256": canonical_sha256(base_encounter),
            "patternId": "VALTAN_FOUR_PILLARS_105",
            "successorBaseRowCanonicalSha256": (
                ADDITIVE_ENCOUNTER_ROW_SEALS["successorBase"]
            ),
            "liveAdditiveRowCanonicalSha256": (
                ADDITIVE_ENCOUNTER_ROW_SEALS["liveAdditive"]
            ),
        },
        "reviewedSelection": {
            "path": (
                "Data/Effects/Imported/Valtan/"
                "Valtan.priority-source-sequence-selections.v1.json"
            ),
            "liveAdditiveCanonicalSha256": canonical_sha256(selection),
            "successorBaseCanonicalSha256": canonical_sha256(base_selection),
            "addedBranchId": str(additive_selection_rows[0]["branchId"]),
            "addedRowCanonicalSha256": ADDITIVE_SELECTION_ROW_SHA256,
        },
    }
    return base_bindings, base_encounter, base_selection, evidence


def byte_sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def repository_path(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT.resolve()).as_posix()
    except ValueError as error:
        raise MaterializeError(f"path escapes repository: {path}") from error


def authored_path(catalog_row: dict[str, Any]) -> Path:
    relative = str(catalog_row.get("authoringPath") or "")
    if not relative or "\\" in relative or ":" in relative:
        raise MaterializeError("Valtan catalog authoringPath is invalid")
    path = (ROOT / "Data" / relative).resolve()
    if AUTHORED_ROOT.resolve() not in path.parents or not path.is_file():
        raise MaterializeError(f"Valtan authored document is missing: {relative}")
    return path


def _deep_overlay(base: dict[str, Any], overlay: dict[str, Any]) -> dict[str, Any]:
    result = copy.deepcopy(base)
    for key, value in overlay.items():
        if isinstance(value, dict) and isinstance(result.get(key), dict):
            result[key] = _deep_overlay(result[key], value)
        else:
            result[key] = copy.deepcopy(value)
    return result


def _common_translucent_element(
    exact: dict[str, Any], existing: dict[str, Any] | None
) -> dict[str, Any]:
    """Preserve hand tuning while resealing carrier-owned fields."""

    result = copy.deepcopy(exact)
    if existing is not None:
        for field in ("displayName", "groupId", "visible"):
            if field in existing:
                result[field] = copy.deepcopy(existing[field])
        if isinstance(existing.get("detail"), dict):
            result["detail"] = _deep_overlay(
                result.get("detail") or {}, existing["detail"]
            )
        for field in ("actionCueAttachment", "transformInheritance"):
            if isinstance(existing.get(field), dict):
                result[field] = copy.deepcopy(existing[field])

    # These fields are source/carrier identity, not hand-tuning state.
    for field in (
        "id",
        "sourceNode",
        "kind",
        "resources",
        "sourceRecipe",
        "sourcePresentation",
        "inventoryRendererShape",
    ):
        result[field] = copy.deepcopy(exact[field])

    exact_material = exact.get("material") or {}
    source_profile = copy.deepcopy(exact_material.get("sourceProfile") or {})
    source_profile["enabled"] = False
    result["material"] = {
        "templateId": "effect.standard",
        "sourceMaterialPath": str(
            exact_material.get("sourceMaterialPath") or ""
        ),
        "renderProfile": "alpha_two_sided_depth_read",
        "sourceProfile": source_profile,
    }

    exact_delay = (
        (exact.get("detail") or {})
        .get("timing", {})
        .get("startDelaySeconds")
    )
    timing = result.setdefault("detail", {}).setdefault("timing", {})
    timing["startDelaySeconds"] = exact_delay

    resources = result.get("resources") or []
    mesh_assets = [
        row
        for row in resources
        if isinstance(row, dict) and row.get("slotId") == "meshModel"
    ]
    shape = str(result.get("inventoryRendererShape") or "")
    mesh = result["detail"].setdefault("mesh", {})
    if shape == "mesh":
        if result.get("kind") != "particle" or len(mesh_assets) != 1:
            raise MaterializeError("exact Mesh carrier lost its single WModel")
        mesh["modelPreScale"] = 0.01
    elif shape == "sprite":
        if mesh_assets:
            raise MaterializeError("exact Sprite carrier inherited a meshModel")
        mesh.pop("modelPreScale", None)
    elif shape == "decal":
        if result.get("kind") != "decal" or mesh_assets:
            raise MaterializeError("exact Decal carrier identity is invalid")
        if sum(
            isinstance(row, dict) and row.get("slotId") == "base"
            for row in resources
        ) != 1:
            raise MaterializeError("exact Decal carrier lost its Base DDS")
        if (result.get("sourceRecipe") or {}).get("rendererShape") != "decal":
            raise MaterializeError("exact Decal carrier lost its source recipe")
        mesh.pop("modelPreScale", None)
    else:
        raise MaterializeError(f"unsupported executable carrier shape: {shape}")
    return result


def _target_effect_id(clip_id: str) -> str:
    if clip_id == RED_BLADE_CLIP_ID:
        return RED_BLADE_EFFECT_ID
    if not clip_id.startswith("valtan."):
        raise MaterializeError(f"foreign clip occurrence: {clip_id}")
    suffix = clip_id[len("valtan.") :].replace(".clip.", ".clip-")
    return "effect.valtan.carrier-v1." + suffix


def _target_cue_id(clip_id: str) -> str:
    effect_id = _target_effect_id(clip_id)
    return "cue." + effect_id[len("effect.") :]


def _stage_additive_reviewed_selections(
    inventory: dict[str, Any],
    selection: dict[str, Any],
    *,
    encounter: dict[str, Any],
    pattern_bindings: dict[str, Any],
) -> dict[str, Any]:
    """Apply new reviewed branches to the sealed source payload inventory."""

    staged = copy.deepcopy(inventory)
    existing_rows = {
        str(row.get("branchId") or ""): row
        for row in staged.get("reviewedBranchSelections", [])
    }
    selected_rows = {
        str(row.get("branchId") or ""): row
        for row in selection.get("selections", [])
    }
    if (
        not existing_rows
        or not selected_rows
        or "" in existing_rows
        or "" in selected_rows
        or len(existing_rows)
        != len(staged.get("reviewedBranchSelections", []))
        or len(selected_rows) != len(selection.get("selections", []))
    ):
        raise MaterializeError("reviewed selection identity is invalid")
    for branch_id, row in existing_rows.items():
        if selected_rows.get(branch_id) != row:
            raise MaterializeError(
                "sealed reviewed selection changed: " + branch_id
            )

    additions = [
        copy.deepcopy(selected_rows[branch_id])
        for branch_id in sorted(set(selected_rows) - set(existing_rows))
    ]
    if not additions:
        staged["_materializerAdditiveReviewedBranchIds"] = []
        return staged

    product_clips = source_inventory.product_clip_occurrences(
        encounter, pattern_bindings
    )
    branches = {
        str(row.get("branchId") or ""): row
        for row in staged.get("branches", [])
    }

    for reviewed in additions:
        branch_id = str(reviewed["branchId"])
        branch = branches.get(branch_id)
        if branch is None or reviewed.get("status") != "REVIEWED_SELECTED":
            raise MaterializeError(
                "additive reviewed selection is not an existing selected branch: "
                + branch_id
            )
        for field in (
            "patternId",
            "sourceActionId",
            "profileId",
            "sequenceIndex",
            "sourceSequencePathSha256",
        ):
            if reviewed.get(field) != branch.get(field):
                raise MaterializeError(
                    f"additive reviewed selection {field} drifted: {branch_id}"
                )

        source_by_coordinate = {
            (int(row["sourceStageIndex"]), int(row["sourceClipOrdinal"])): row
            for row in branch.get("orderedClips", [])
        }
        mapping_by_coordinate = {
            (int(row["sourceStageIndex"]), int(row["sourceClipOrdinal"])): row
            for row in reviewed.get("stageMappings", [])
        }
        if (
            not source_by_coordinate
            or set(mapping_by_coordinate) != set(source_by_coordinate)
        ):
            raise MaterializeError(
                "additive reviewed selection must map every source clip: "
                + branch_id
            )
        product_rows = product_clips.get(str(reviewed["patternId"]), [])
        product_by_id = {
            str(row["clipOccurrenceId"]): row for row in product_rows
        }
        for coordinate, mapping in mapping_by_coordinate.items():
            timing = str(mapping.get("timingDisposition") or "")
            clip_id = mapping.get("clipOccurrenceId")
            product = product_by_id.get(str(clip_id or ""))
            source = source_by_coordinate[coordinate]
            if timing == "REACHABLE":
                if product is None or source["normalizedClip"] != product[
                    "normalizedClip"
                ]:
                    raise MaterializeError(
                        "additive reviewed source/product clip mapping drifted: "
                        + branch_id
                    )
            elif timing != "UNREACHABLE" or clip_id is not None:
                raise MaterializeError(
                    "additive reviewed timing disposition is unsupported: "
                    + branch_id
                )

        branch["selectionStatus"] = "REVIEWED_SELECTED"
        branch["reviewedStageMappings"] = [
            copy.deepcopy(mapping_by_coordinate[key])
            for key in sorted(mapping_by_coordinate)
        ]
        occurrence_count = 0
        for occurrence in staged.get("occurrences", []):
            if occurrence.get("branchId") != branch_id:
                continue
            occurrence_count += 1
            coordinate = (
                int(occurrence["sourceStageIndex"]),
                int(occurrence["sourceClipOrdinal"]),
            )
            mapping = mapping_by_coordinate[coordinate]
            timing = str(mapping["timingDisposition"])
            clip_id = mapping.get("clipOccurrenceId")
            product = product_by_id.get(str(clip_id or ""))
            normalized = source_inventory.normalize_clip(
                str(occurrence.get("sourceClip") or "")
            )
            occurrence["candidateClipOccurrenceIds"] = [
                str(row["clipOccurrenceId"])
                for row in product_rows
                if row["normalizedClip"] == normalized
            ]
            occurrence["branchSelectionStatus"] = "REVIEWED_SELECTED"
            occurrence["timingDisposition"] = timing
            occurrence["mappingReviewBasis"] = mapping.get("reviewBasis")
            if product is None:
                occurrence["semanticStageId"] = None
                occurrence["gameplayActionId"] = None
                occurrence["clipOccurrenceId"] = None
                occurrence["reachabilityDisposition"] = (
                    "UNREACHABLE_SOURCE_OCCURRENCE"
                )
            else:
                occurrence["semanticStageId"] = product["semanticStageId"]
                occurrence["gameplayActionId"] = product["gameplayActionId"]
                occurrence["clipOccurrenceId"] = product["clipOccurrenceId"]
                occurrence["reachabilityDisposition"] = "REACHABLE_REVIEWED"
        if occurrence_count == 0:
            raise MaterializeError(
                "additive reviewed branch has no sealed source occurrences: "
                + branch_id
            )

    staged["reviewedBranchSelections"] = copy.deepcopy(
        selection["selections"]
    )
    staged["_materializerAdditiveReviewedBranchIds"] = [
        str(row["branchId"]) for row in additions
    ]
    return staged


def _hydrate_carrier_element_seeds_from_applied_product(
    inventory: dict[str, Any]
) -> None:
    """Recover sealed carrier payloads from the already-applied exact owners."""

    receipt = read_json(RECEIPT_PATH)
    ledger = receipt.get("reviewedProjectionLedger")
    if (
        receipt.get("schema")
        != "lostark.valtan-carrier-v1-materialization-receipt"
        or receipt.get("formatVersion") != 1
        or not isinstance(ledger, list)
    ):
        raise MaterializeError("applied carrier V1 receipt header is invalid")

    elements_by_source_node: dict[str, dict[str, Any]] = {}
    output_rows = list((receipt.get("outputs") or {}).get("targetDocuments") or [])
    protected_output = (receipt.get("outputs") or {}).get(
        "protectedWhirlwindDocument"
    )
    if isinstance(protected_output, dict):
        output_rows.append(protected_output)
    historical_fallback_documents: list[dict[str, Any]] = []
    for row in output_rows:
        path = ROOT / str(row.get("path") or "")
        if not path.is_file():
            raise MaterializeError(
                "applied exact owner document is missing: " + str(path)
            )
        document = read_json(path)
        for element in document.get("elements", []):
            source_node = str(element.get("sourceNode") or "")
            if not source_node:
                continue
            if source_node in elements_by_source_node:
                raise MaterializeError(
                    "applied exact sourceNode has two owners: " + source_node
                )
            elements_by_source_node[source_node] = copy.deepcopy(element)

        # A Product successor may intentionally remove historical Carrier V1
        # rows while a later additive import still needs one of those exact
        # payloads as a donor.  Resolve that donor from the immutable sealed
        # historical document; never reinsert it into the live successor.
        historical_sha = str(row.get("canonicalSha256") or "")
        if canonical_sha256(document) == historical_sha:
            continue
        effect_id = str(row.get("effectAssetId") or "")
        matches: list[dict[str, Any]] = []
        for candidate in sorted(
            LEGACY_DONOR_ROOT.glob(f"{effect_id}.*.effect.json")
        ):
            candidate_document = read_json(candidate)
            if (
                candidate_document.get("effectAssetId") == effect_id
                and canonical_sha256(candidate_document) == historical_sha
            ):
                matches.append(candidate_document)
        if len(matches) != 1:
            raise MaterializeError(
                "historical carrier donor seal is not unique: "
                + effect_id
            )
        historical_fallback_documents.append(matches[0])

    for document in historical_fallback_documents:
        for element in document.get("elements", []):
            source_node = str(element.get("sourceNode") or "")
            if not source_node or source_node in elements_by_source_node:
                continue
            elements_by_source_node[source_node] = copy.deepcopy(element)

    occurrences = {
        str(row.get("fullKey") or ""): row
        for row in inventory.get("occurrences", [])
    }
    systems = {
        str(row.get("sourceSystemId") or ""): row
        for row in inventory.get("sourceSystems", [])
    }
    carriers = {
        (system_id, str(carrier.get("carrierKey") or "")): carrier
        for system_id, system in systems.items()
        for carrier in system.get("carriers", [])
    }
    additive_branch_ids = set(
        inventory.get("_materializerAdditiveReviewedBranchIds") or []
    )
    needed: dict[tuple[Any, ...], list[tuple[dict[str, Any], dict[str, Any]]]] = (
        defaultdict(list)
    )
    for occurrence in inventory.get("occurrences", []):
        if (
            occurrence.get("branchId") not in additive_branch_ids
            or occurrence.get("reachabilityDisposition")
            != "REACHABLE_REVIEWED"
        ):
            continue
        system_id = str(occurrence.get("sourceSystemId") or "")
        system = systems.get(system_id)
        for carrier in (system or {}).get("carriers", []):
            if carrier.get("disposition") != "EXECUTABLE_CORE":
                continue
            key = (
                int(occurrence["sourceActionId"]),
                int(occurrence["sourceStageIndex"]),
                str(occurrence.get("notifyId") or ""),
                int(occurrence.get("sourceAssetOrdinal") or 0),
                system_id,
                str(carrier.get("carrierKey") or ""),
            )
            needed[key].append((occurrence, carrier))
    if sum(len(rows) for rows in needed.values()) != 26:
        raise MaterializeError("additive reviewed carrier denominator drifted")

    donors: dict[tuple[Any, ...], dict[str, Any]] = {}
    for row in ledger:
        if (
            row.get("disposition") != "EXECUTABLE_CORE"
            or row.get("patternId") != "VALTAN_HIGH_JUMP"
        ):
            continue
        occurrence = occurrences.get(str(row.get("occurrenceFullKey") or ""))
        if occurrence is None:
            continue
        source_node = str(row.get("sourceNode") or "")
        system_id = str(row.get("sourceSystemId") or "")
        carrier_key = str(row.get("carrierKey") or "")
        carrier = carriers.get((system_id, carrier_key))
        key = (
            int(occurrence["sourceActionId"]),
            int(occurrence["sourceStageIndex"]),
            str(occurrence.get("notifyId") or ""),
            int(occurrence.get("sourceAssetOrdinal") or 0),
            system_id,
            carrier_key,
        )
        if key not in needed:
            continue
        element = elements_by_source_node.get(source_node)
        if element is None or carrier is None:
            raise MaterializeError(
                "additive carrier donor evidence is incomplete: " + source_node
            )
        renderer_shape = str(carrier.get("rendererShape") or "")
        if (
            canonical_sha256(element.get("resources") or [])
            != str(carrier.get("runtimeResourceBindingSha256") or "")
            or str((element.get("sourceRecipe") or {}).get("rendererShape") or "")
            != renderer_shape
        ):
            raise MaterializeError(
                "applied exact carrier payload drifted: " + source_node
            )
        current = donors.get(key)
        if current is not None:
            raise MaterializeError(
                "100-bar High Jump carrier donor is not unique: " + repr(key)
            )
        donor_element = copy.deepcopy(element)
        # Effect Tool successor saves normalize float precision and omit this
        # historical audit-only field.  The successor receipt seals that live
        # document; restore the renderer identity from the immutable carrier
        # inventory before deriving the additive owner.
        donor_element["inventoryRendererShape"] = renderer_shape
        donors[key] = {
            "element": donor_element,
            "ledger": copy.deepcopy(row),
        }

    hydrated_count = 0
    for key, targets in needed.items():
        donor = donors.get(key)
        if donor is None:
            raise MaterializeError(
                "additive reviewed carrier has no exact Product donor: "
                + repr(key)
            )
        for occurrence, carrier in targets:
            carrier.setdefault("materializerElementSeeds", {})[
                str(occurrence["fullKey"])
            ] = copy.deepcopy(donor["element"])
            carrier["runtimeResources"] = copy.deepcopy(
                donor["ledger"].get("runtimeResources") or []
            )
            carrier["resourceReceipt"] = copy.deepcopy(
                donor["ledger"].get("resourceReceipt") or []
            )
            carrier["sourceResourceClosure"] = [
                {"role": "material", "objectPath": object_path}
                for object_path in donor["ledger"].get(
                    "materialObjectPaths", []
                )
            ]
            hydrated_count += 1
    if hydrated_count != 26:
        raise MaterializeError("additive hydrated carrier denominator drifted")


def _load_inventory(
    successor_receipt: dict[str, Any] | None = None,
) -> dict[str, Any]:
    if successor_receipt is None:
        successor_receipt = _load_successor_receipt()
    live_selection = source_inventory.load_selection_manifest(
        reviewed_candidates.SELECTION_PATH
    )
    live_pattern_bindings = read_json(
        source_inventory.PATTERN_BINDINGS_PATH
    )
    live_encounter = read_json(source_inventory.ENCOUNTER_PATH)

    # A newly reviewed branch is an additive layer over the immutable sealed
    # source inventory.  Stage it against the live Product contract before
    # projecting the historical input, because unrelated successor edits in
    # that live contract belong to the successor receipt rather than this
    # additive receipt.
    sealed_inventory = source_inventory.read_json(source_inventory.OUTPUT_PATH)
    source_inventory.validate_inventory(sealed_inventory)
    staged = _stage_additive_reviewed_selections(
        sealed_inventory,
        live_selection,
        encounter=live_encounter,
        pattern_bindings=live_pattern_bindings,
    )
    if staged.get("_materializerAdditiveReviewedBranchIds"):
        project_successor_base_inputs(
            successor_receipt,
            live_pattern_bindings,
            live_encounter,
            live_selection,
        )
        _validate_successor_base_product(
            successor_receipt,
            read_json(CATALOG_PATH),
            read_json(CUE_PATH),
        )
        _hydrate_carrier_element_seeds_from_applied_product(staged)
        return staged

    try:
        historical_pattern_bindings = (
            successor_lineage.project_historical_pattern_bindings(
                live_pattern_bindings, successor_receipt
            )
        )
        historical_encounter = successor_lineage.project_historical_encounter(
            live_encounter, successor_receipt
        )
        historical_selection = (
            successor_lineage.project_historical_selection_manifest(
                live_selection, successor_receipt
            )
        )
    except successor_lineage.SuccessorLineageError as error:
        raise MaterializeError(str(error)) from error

    rebuilt = source_inventory.build_inventory(
        previous={
            "reviewedBranchSelections": copy.deepcopy(
                historical_selection["selections"]
            )
        },
        include_payloads=True,
        pattern_bindings=historical_pattern_bindings,
        encounter=historical_encounter,
    )
    source_inventory.validate_inventory(rebuilt)
    return rebuilt


def _build_projections(
    inventory: dict[str, Any],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], dict[str, Any]]:
    additive_branch_ids = set(
        inventory.get("_materializerAdditiveReviewedBranchIds") or []
    )
    systems = {
        str(row["sourceSystemId"]): row
        for row in inventory.get("sourceSystems", [])
    }
    core: list[dict[str, Any]] = []
    blocked = Counter()
    blocked_shapes = Counter()
    reviewed_without_carrier = 0
    projection_ledger: list[dict[str, Any]] = []
    source_only_occurrences: list[dict[str, Any]] = []

    for occurrence in inventory.get("occurrences", []):
        if (
            occurrence.get("reachabilityDisposition")
            != "REACHABLE_REVIEWED"
            or (
                additive_branch_ids
                and occurrence.get("branchId") not in additive_branch_ids
            )
        ):
            continue
        system = systems.get(str(occurrence.get("sourceSystemId") or ""))
        carriers = list((system or {}).get("carriers", []))
        if not carriers:
            reviewed_without_carrier += 1
            source_only_occurrences.append(
                {
                    "occurrenceId": str(occurrence["occurrenceId"]),
                    "occurrenceFullKey": str(occurrence["fullKey"]),
                    "patternId": str(occurrence["patternId"]),
                    "semanticStageId": str(
                        occurrence.get("semanticStageId") or ""
                    ),
                    "gameplayActionId": str(
                        occurrence.get("gameplayActionId") or ""
                    ),
                    "clipOccurrenceId": str(
                        occurrence.get("clipOccurrenceId") or ""
                    ),
                    "sourceSystemId": str(
                        occurrence.get("sourceSystemId") or ""
                    ),
                    "sourceType": str(occurrence.get("sourceType") or ""),
                    "category": str(occurrence.get("category") or ""),
                    "assetReference": copy.deepcopy(
                        occurrence.get("assetReference")
                    ),
                    "disposition": str(occurrence.get("disposition") or ""),
                    "productAdmission": False,
                }
            )
        for carrier in carriers:
            disposition = str(carrier.get("disposition") or "")
            material_paths = sorted(
                {
                    str(resource.get("objectPath") or "")
                    for resource in carrier.get("sourceResourceClosure", [])
                    if isinstance(resource, dict)
                    and resource.get("role") == "material"
                    and resource.get("objectPath")
                }
            )
            ledger_row = {
                "occurrenceId": str(occurrence["occurrenceId"]),
                "occurrenceFullKey": str(occurrence["fullKey"]),
                "patternId": str(occurrence["patternId"]),
                "semanticStageId": str(
                    occurrence.get("semanticStageId") or ""
                ),
                "gameplayActionId": str(
                    occurrence.get("gameplayActionId") or ""
                ),
                "clipOccurrenceId": str(occurrence["clipOccurrenceId"]),
                "sourceSystemId": str(
                    occurrence.get("sourceSystemId") or ""
                ),
                "sourceTimeSeconds": float(occurrence["sourceTimeSeconds"]),
                "carrierKey": str(carrier.get("carrierKey") or ""),
                "sourceEmitterNodeId": str(
                    carrier.get("sourceEmitterNodeId") or ""
                ),
                "sourceEmitterPath": str(
                    carrier.get("sourceEmitterPath") or ""
                ),
                "sourceOrder": int(carrier.get("sourceOrder") or 0),
                "rendererShape": str(carrier.get("rendererShape") or ""),
                "disposition": disposition,
                "conversionStatus": str(
                    carrier.get("conversionStatus") or ""
                ),
                "conversionBlockers": copy.deepcopy(
                    carrier.get("conversionBlockers") or []
                ),
                "materialObjectPaths": material_paths,
                "runtimeResources": copy.deepcopy(
                    carrier.get("runtimeResources") or []
                ),
                "resourceReceipt": copy.deepcopy(
                    carrier.get("resourceReceipt") or []
                ),
                "runtimeResourceBindingSha256": carrier.get(
                    "runtimeResourceBindingSha256"
                ),
                "sourceRecipeSha256": carrier.get("sourceRecipeSha256"),
                "portableSourceRecipeSha256": carrier.get(
                    "portableSourceRecipeSha256"
                ),
                "productAdmission": disposition == "EXECUTABLE_CORE",
            }
            projection_ledger.append(ledger_row)
            if disposition != "EXECUTABLE_CORE":
                blocked[disposition] += 1
                blocked_shapes[str(carrier.get("rendererShape") or "unknown")] += 1
                continue
            element_seed = (carrier.get("materializerElementSeeds") or {}).get(
                str(occurrence["fullKey"])
            )
            if element_seed is None:
                element_seed = carrier.get("elementSeed")
            if element_seed is None:
                raise MaterializeError("executable carrier has no elementSeed")
            full_key = (
                f"{occurrence['fullKey']}|{carrier['carrierKey']}"
            )
            staged_carrier = copy.deepcopy(carrier)
            staged_carrier["elementSeed"] = copy.deepcopy(element_seed)
            element = source_inventory.occurrence_element_seed(
                occurrence, staged_carrier
            )
            element = reviewed_candidates.add_v13_transform_ownership_defaults(
                element
            )
            compact, source_key = reviewed_candidates.compress_v13_source_node(
                element
            )
            original_material = copy.deepcopy(compact.get("material") or {})
            clip_id = str(occurrence["clipOccurrenceId"])
            ledger_row["sourceNode"] = str(source_key["sourceNode"])
            ledger_row["fullSourceKey"] = full_key
            ledger_row["coverageDisposition"] = (
                "COVERED_BY_PROTECTED_WHIRLWIND_CANARY"
                if clip_id == PROTECTED_CLIP_ID
                else "MATERIALIZE_COMMON_TRANSLUCENT"
            )
            core.append(
                {
                    "patternId": str(occurrence["patternId"]),
                    "semanticStageId": str(occurrence["semanticStageId"]),
                    "gameplayActionId": str(occurrence["gameplayActionId"]),
                    "clipOccurrenceId": clip_id,
                    "occurrenceId": str(occurrence["occurrenceId"]),
                    "occurrenceFullKey": str(occurrence["fullKey"]),
                    "carrierKey": str(carrier["carrierKey"]),
                    "rendererShape": str(carrier["rendererShape"]),
                    "sourceTimeSeconds": float(
                        occurrence["sourceTimeSeconds"]
                    ),
                    "sourceActionId": int(occurrence["sourceActionId"]),
                    "sourceStageIndex": int(occurrence["sourceStageIndex"]),
                    "notifyId": str(occurrence["notifyId"]),
                    "sourceEmitterNodeId": str(
                        carrier["sourceEmitterNodeId"]
                    ),
                    "selectedLodNodeId": str(carrier["selectedLodNodeId"]),
                    "sourceNode": str(source_key["sourceNode"]),
                    "fullSourceKey": full_key,
                    "targetEffectAssetId": (
                        PROTECTED_EFFECT_ID
                        if clip_id == PROTECTED_CLIP_ID
                        else _target_effect_id(clip_id)
                    ),
                    "coverageDisposition": (
                        "COVERED_BY_PROTECTED_WHIRLWIND_CANARY"
                        if clip_id == PROTECTED_CLIP_ID
                        else "MATERIALIZE_COMMON_TRANSLUCENT"
                    ),
                    "originalMaterial": {
                        "templateId": str(
                            original_material.get("templateId") or ""
                        ),
                        "sourceMaterialPath": str(
                            original_material.get("sourceMaterialPath") or ""
                        ),
                        "inferredRenderProfile": str(
                            original_material.get("renderProfile") or ""
                        ),
                        "sourceProfileEnabled": bool(
                            (original_material.get("sourceProfile") or {}).get(
                                "enabled"
                            )
                        ),
                        "sourceProfile": copy.deepcopy(
                            original_material.get("sourceProfile") or {}
                        ),
                        "familyStatus": (
                            "TYPED_SOURCE_PROFILE_PRESENT"
                            if (original_material.get("sourceProfile") or {}).get(
                                "enabled"
                            )
                            else "SOURCE_MATERIAL_IDENTITY_PRESERVED_TYPED_FAMILY_PENDING"
                        ),
                    },
                    "element": compact,
                }
            )

    core.sort(key=lambda row: row["fullSourceKey"])
    source_nodes = [row["sourceNode"] for row in core]
    full_keys = [row["fullSourceKey"] for row in core]
    if len(source_nodes) != len(set(source_nodes)) or len(full_keys) != len(
        set(full_keys)
    ):
        raise MaterializeError("exact projection identity is duplicated")

    shapes = Counter(row["rendererShape"] for row in core)
    protected = [
        row for row in core if row["clipOccurrenceId"] == PROTECTED_CLIP_ID
    ]
    materialized = [
        row for row in core if row["clipOccurrenceId"] != PROTECTED_CLIP_ID
    ]
    groups = {row["clipOccurrenceId"] for row in materialized}
    actual = {
        "reviewedCoreProjectionCount": len(core),
        "reviewedCoreSpriteProjectionCount": shapes["sprite"],
        "reviewedCoreMeshProjectionCount": shapes["mesh"],
        "reviewedCoreDecalProjectionCount": shapes["decal"],
        "reviewedCoreClipCount": len(
            {row["clipOccurrenceId"] for row in core}
        ),
        "reviewedCorePatternCount": len({row["patternId"] for row in core}),
        "reviewedProjectionLedgerCount": len(projection_ledger),
        "protectedProjectionCount": len(protected),
        "materializedProjectionCount": len(materialized),
        "materializedClipGroupCount": len(groups),
        "newClipDocumentCount": len(
            {row["targetEffectAssetId"] for row in materialized}
            - {RED_BLADE_EFFECT_ID}
        ),
        "blockedExpandedCarrierCount": sum(blocked.values()),
        "reviewedOccurrenceWithoutCarrierCount": reviewed_without_carrier,
    }
    additive_expected = {
        "reviewedCoreProjectionCount": 26,
        "reviewedCoreSpriteProjectionCount": 16,
        "reviewedCoreMeshProjectionCount": 8,
        "reviewedCoreDecalProjectionCount": 2,
        "reviewedCoreClipCount": 2,
        "reviewedCorePatternCount": 1,
        "reviewedProjectionLedgerCount": 54,
        "protectedProjectionCount": 0,
        "materializedProjectionCount": 26,
        "materializedClipGroupCount": 2,
        "newClipDocumentCount": 2,
        "blockedExpandedCarrierCount": 28,
        "reviewedOccurrenceWithoutCarrierCount": 9,
    }
    expected_counts = additive_expected if additive_branch_ids else EXPECTED
    for key in (
        "reviewedCoreProjectionCount",
        "reviewedCoreSpriteProjectionCount",
        "reviewedCoreMeshProjectionCount",
        "reviewedCoreDecalProjectionCount",
        "reviewedCoreClipCount",
        "reviewedCorePatternCount",
        "reviewedProjectionLedgerCount",
        "protectedProjectionCount",
        "materializedProjectionCount",
        "materializedClipGroupCount",
        "newClipDocumentCount",
        "blockedExpandedCarrierCount",
        "reviewedOccurrenceWithoutCarrierCount",
    ):
        if actual[key] != expected_counts[key]:
            raise MaterializeError(
                f"reviewed carrier denominator drifted: {key}: "
                f"expected {expected_counts[key]}, actual {actual[key]}"
            )
    blocker_summary = {
        "expandedCarrierCount": sum(blocked.values()),
        "dispositionCounts": dict(sorted(blocked.items())),
        "rendererShapeCounts": dict(sorted(blocked_shapes.items())),
        "reviewedOccurrenceWithoutCarrierCount": reviewed_without_carrier,
        "productAdmission": False,
        "reviewedProjectionLedger": sorted(
            projection_ledger,
            key=lambda row: (
                row["occurrenceFullKey"],
                row["carrierKey"],
            ),
        ),
        "reviewedSourceOnlyOccurrences": sorted(
            source_only_occurrences,
            key=lambda row: row["occurrenceFullKey"],
        ),
    }
    return core, materialized, blocker_summary


def _catalog_index(document: dict[str, Any]) -> dict[str, dict[str, Any]]:
    if document.get("formatVersion") != 1 or not isinstance(
        document.get("effects"), list
    ):
        raise MaterializeError("EffectCatalog header is invalid")
    result: dict[str, dict[str, Any]] = {}
    for row in document["effects"]:
        effect_id = str((row or {}).get("effectAssetId") or "")
        if not effect_id or effect_id in result:
            raise MaterializeError(f"duplicate/empty catalog effect: {effect_id}")
        result[effect_id] = copy.deepcopy(row)
    return result


def _cue_index(document: dict[str, Any]) -> dict[str, dict[str, Any]]:
    if (
        document.get("schema") != "lostark.valtan-pattern-effect-cues"
        or document.get("formatVersion") != 2
        or document.get("ownerArchetypeId") != "BOSS_VALTAN"
        or not isinstance(document.get("cues"), list)
    ):
        raise MaterializeError("Valtan cue header is invalid")
    result: dict[str, dict[str, Any]] = {}
    occurrence_ids: set[str] = set()
    for row in document["cues"]:
        binding_id = str((row or {}).get("bindingId") or "")
        occurrence_id = str((row or {}).get("occurrenceId") or "")
        if (
            not binding_id
            or not occurrence_id
            or binding_id in result
            or occurrence_id in occurrence_ids
        ):
            raise MaterializeError("Valtan cue identity is duplicated or empty")
        result[binding_id] = copy.deepcopy(row)
        occurrence_ids.add(occurrence_id)
    return result


def _external_effect_owners() -> set[str]:
    boss = read_json(BOSS_CATALOG_PATH)
    values: set[str] = set()

    def visit(value: Any) -> None:
        if isinstance(value, dict):
            for child in value.values():
                visit(child)
        elif isinstance(value, list):
            for child in value:
                visit(child)
        elif isinstance(value, str) and value.startswith("effect.valtan."):
            values.add(value)

    visit(boss)
    if RED_BLADE_EFFECT_ID not in values:
        raise MaterializeError("Red Blade combat-object Effect owner is missing")
    return values


def _load_legacy_authorizations() -> tuple[
    dict[str, Any], dict[tuple[str, int], dict[str, Any]]
]:
    """Load the sealed preimage; it alone authorizes destructive row removal."""

    artifact = read_json(LEGACY_INVENTORY_PATH)
    try:
        legacy_inventory_builder.validate_inventory(artifact)
    except Exception as error:
        raise MaterializeError(
            f"sealed legacy carrier inventory is invalid: {error}"
        ) from error
    rows = artifact.get("legacyRowAudit")
    if not isinstance(rows, list) or len(rows) != EXPECTED[
        "baselineStrictLegacyRowCount"
    ]:
        raise MaterializeError("sealed legacy row denominator drifted")
    authorizations: dict[tuple[str, int], dict[str, Any]] = {}
    for row in rows:
        key = _legacy_authorization_key(row)
        effect_id, element_index = key
        element_hash = str(row.get("legacyElementSha256") or "")
        element_id = str(row.get("elementId") or "")
        if (
            len(element_hash) != 64
            or not element_id
        ):
            raise MaterializeError("sealed legacy authorization identity is invalid")
        if key in authorizations:
            raise MaterializeError("sealed legacy authorization is duplicated")
        authorizations[key] = copy.deepcopy(row)
    return artifact, authorizations


def _legacy_authorization_key(row: dict[str, Any]) -> tuple[str, int]:
    join_status = str(
        row.get("materialOrdinalJoinStatus")
        or row.get("sourceSystemJoinStatus")
        or ""
    )
    if join_status != "EXACT_MATERIAL_ORDINAL_JOIN":
        raise MaterializeError(
            "legacy deletion is not authorized by an exact material "
            f"ordinal join: {row.get('auditId')}: {join_status}"
        )
    effect_id = str(row.get("effectAssetId") or "")
    element_index = row.get("elementIndex")
    if (
        not effect_id.startswith("effect.valtan.")
        or isinstance(element_index, bool)
        or not isinstance(element_index, int)
        or element_index < 0
    ):
        raise MaterializeError("sealed legacy authorization coordinate is invalid")
    return effect_id, element_index


def _validate_legacy_preimage_row(
    authorization: dict[str, Any], element: dict[str, Any]
) -> None:
    actual_hash = legacy_inventory_builder.canonical_sha256(element)
    if (
        str(element.get("id") or "")
        != str(authorization.get("elementId") or "")
        or actual_hash
        != str(authorization.get("legacyElementSha256") or "")
    ):
        raise MaterializeError(
            "sealed legacy row preimage drifted: "
            + str(authorization.get("auditId") or "")
        )


def _verify_baseline_legacy_inventory_bytes(
    artifact: dict[str, Any],
) -> None:
    """Prove the sealed ledger still describes the live Product preimage."""

    try:
        rebuilt = legacy_inventory_builder.build_inventory()
        legacy_inventory_builder.validate_inventory(rebuilt)
        expected = legacy_inventory_builder.serialized(rebuilt)
    except Exception as error:
        raise MaterializeError(
            f"cannot rebuild legacy carrier preimage inventory: {error}"
        ) from error
    actual = LEGACY_INVENTORY_PATH.read_bytes()
    if (
        expected != actual
        or legacy_inventory_builder.canonical_sha256(rebuilt)
        != legacy_inventory_builder.canonical_sha256(artifact)
    ):
        raise MaterializeError(
            "sealed legacy carrier inventory does not match live Product preimage"
        )


def _protected_whirlwind_alias_proof(
    core: list[dict[str, Any]], document: dict[str, Any]
) -> list[dict[str, Any]]:
    """Join the three reviewed core carriers to the typed 420633 aliases."""

    protected = [
        row for row in core if row["clipOccurrenceId"] == PROTECTED_CLIP_ID
    ]
    if len(protected) != EXPECTED["protectedProjectionCount"]:
        raise MaterializeError("protected Whirlwind core denominator drifted")
    elements = {
        str(row.get("sourceNode") or ""): row
        for row in document.get("elements", [])
    }
    if len(elements) != 9:
        raise MaterializeError("protected Whirlwind alias set is not nine rows")
    proofs = []
    for projection in protected:
        alias = (
            f"{projection['notifyId']}"
            f"|{projection['sourceEmitterNodeId']}"
            f"|{projection['selectedLodNodeId']}"
        )
        element = elements.get(alias)
        if element is None:
            raise MaterializeError(
                "protected Whirlwind exact emitter/LOD alias is missing: " + alias
            )
        mesh_models = [
            row
            for row in element.get("resources", [])
            if isinstance(row, dict) and row.get("slotId") == "meshModel"
        ]
        derived_shape = "mesh" if len(mesh_models) == 1 else "sprite"
        exact = projection["element"]
        if (
            element.get("kind") != "particle"
            or derived_shape != projection["rendererShape"]
            or canonical_sha256(element.get("resources") or [])
            != canonical_sha256(exact.get("resources") or [])
            or str(
                (element.get("material") or {}).get("sourceMaterialPath") or ""
            ).casefold()
            != str(
                projection["originalMaterial"].get("sourceMaterialPath") or ""
            ).casefold()
            or canonical_sha256(element.get("sourceRecipe") or {})
            != canonical_sha256(exact.get("sourceRecipe") or {})
        ):
            raise MaterializeError(
                "protected Whirlwind alias is not an exact carrier join: " + alias
            )
        proofs.append(
            {
                "occurrenceId": projection["occurrenceId"],
                "occurrenceFullKey": projection["occurrenceFullKey"],
                "carrierKey": projection["carrierKey"],
                "reviewedNotifyId": projection["notifyId"],
                "sourceEmitterNodeId": projection["sourceEmitterNodeId"],
                "selectedLodNodeId": projection["selectedLodNodeId"],
                "protectedElementId": str(element.get("id") or ""),
                "protectedSourceNodeAlias": alias,
                "rendererShape": derived_shape,
                "sourceMaterialPath": str(
                    (element.get("material") or {}).get("sourceMaterialPath")
                    or ""
                ),
                "resourcesSha256": canonical_sha256(
                    element.get("resources") or []
                ),
                "sourceRecipeSha256": canonical_sha256(
                    element.get("sourceRecipe") or {}
                ),
                "joinStatus": "EXACT_SOURCE_OCCURRENCE_EMITTER_LOD_MATERIAL_RECIPE_ALIAS",
            }
        )
    return sorted(proofs, key=lambda row: row["carrierKey"])


def _reseal_protected_whirlwind_scale(
    document: dict[str, Any], proofs: list[dict[str, Any]]
) -> dict[str, Any]:
    """Preserve the canary rows but close the universal WModel 0.01 ABI."""

    staged = copy.deepcopy(document)
    elements = {
        str(row.get("id") or ""): row for row in staged.get("elements", [])
    }
    mesh_proofs = [row for row in proofs if row["rendererShape"] == "mesh"]
    if len(mesh_proofs) != 2:
        raise MaterializeError(
            "protected Whirlwind exact WModel denominator is not two"
        )
    for proof in mesh_proofs:
        element = elements.get(str(proof["protectedElementId"]))
        if element is None:
            raise MaterializeError("protected Whirlwind WModel row disappeared")
        mesh_models = [
            row
            for row in element.get("resources", [])
            if isinstance(row, dict) and row.get("slotId") == "meshModel"
        ]
        if len(mesh_models) != 1:
            raise MaterializeError(
                "protected Whirlwind exact Mesh lost its WModel"
            )
        element.setdefault("detail", {}).setdefault("mesh", {})[
            "modelPreScale"
        ] = 0.01
    return staged


def _new_document(effect_id: str, group: list[dict[str, Any]]) -> dict[str, Any]:
    first = group[0]
    return {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": effect_id,
        "displayName": (
            f"{first['patternId']} / {first['semanticStageId']} / carrier V1"
        ),
        "particleSystem": {
            "uniformScaleMultiplier": 1,
            "yawOffsetDegrees": 0,
            "directionYawDegrees": 0,
            "initialSpeedMultiplier": 1,
        },
        "modelCues": [],
        "elements": [],
    }


def _catalog_row(effect_id: str) -> dict[str, Any]:
    return {
        "effectAssetId": effect_id,
        "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
        "authoringPath": f"Effects/Authored/{effect_id}.effect.json",
    }


def _cue_row(group: list[dict[str, Any]]) -> dict[str, Any]:
    first = group[0]
    clip_id = str(first["clipOccurrenceId"])
    cue_id = _target_cue_id(clip_id)
    return {
        "bindingId": cue_id,
        "occurrenceId": f"{cue_id}.occurrence.01",
        "patternId": str(first["patternId"]),
        "stageId": str(first["semanticStageId"]),
        "actionId": str(first["gameplayActionId"]),
        "clipOccurrenceId": clip_id,
        "effectAssetId": str(first["targetEffectAssetId"]),
        "anchorSlotId": "root",
        "followPolicy": "follow",
        "stopPolicy": "natural",
        "repeatPolicy": "once",
        "sourceStartMs": 0,
        "sourceEndMs": None,
        "localTransform": {
            "position": [0, 0, 0],
            "rotationDegrees": [0, 0, 0],
            "scale": [1, 1, 1],
        },
    }


def _validate_document(document: dict[str, Any], effect_id: str) -> None:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != effect_id
        or not isinstance(document.get("elements"), list)
    ):
        raise MaterializeError(f"authored Effect header is invalid: {effect_id}")
    ids = [str(row.get("id") or "") for row in document["elements"]]
    source_nodes = [
        str(row.get("sourceNode") or "")
        for row in document["elements"]
        if row.get("sourceNode")
    ]
    if any(not value for value in ids) or len(ids) != len(set(ids)):
        raise MaterializeError(f"authored Effect IDs are invalid: {effect_id}")
    if len(source_nodes) != len(set(source_nodes)):
        raise MaterializeError(
            f"authored Effect sourceNode is duplicated: {effect_id}"
        )


def _build_receipt(
    *,
    core: list[dict[str, Any]],
    materialized: list[dict[str, Any]],
    blockers: dict[str, Any],
    migration_rows: list[dict[str, Any]],
    retired_effect_ids: list[str],
    retired_cue_ids: list[str],
    target_documents: dict[str, dict[str, Any]],
    target_protected_document: dict[str, Any],
    sky_document: dict[str, Any],
    target_catalog: dict[str, Any],
    target_cues: dict[str, Any],
    successor_receipt: dict[str, Any],
) -> dict[str, Any]:
    valtan_catalog_projection = _valtan_catalog_receipt_projection(
        target_catalog, successor_receipt
    )
    cue_receipt_projection = _cue_receipt_projection(
        target_cues, successor_receipt
    )
    catalog_by_effect = _catalog_index(target_catalog)
    cues_by_id = _cue_index(cue_receipt_projection)
    protected_cue = next(
        row
        for row in cues_by_id.values()
        if row.get("effectAssetId") == PROTECTED_EFFECT_ID
    )
    blocker_summary = {
        key: copy.deepcopy(value)
        for key, value in blockers.items()
        if key
        not in {"reviewedProjectionLedger", "reviewedSourceOnlyOccurrences"}
    }
    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in materialized:
        grouped[row["clipOccurrenceId"]].append(row)
    clip_rows = []
    for clip_id in sorted(grouped):
        group = grouped[clip_id]
        first = group[0]
        shapes = Counter(row["rendererShape"] for row in group)
        clip_rows.append(
            {
                "patternId": first["patternId"],
                "semanticStageId": first["semanticStageId"],
                "gameplayActionId": first["gameplayActionId"],
                "clipOccurrenceId": clip_id,
                "effectAssetId": first["targetEffectAssetId"],
                "cueBindingId": (
                    None if clip_id == RED_BLADE_CLIP_ID else _target_cue_id(clip_id)
                ),
                "owner": (
                    "BOSS_CATALOG_COMBAT_OBJECT"
                    if clip_id == RED_BLADE_CLIP_ID
                    else "VALTAN_PATTERN_EFFECT_CUE"
                ),
                "projectionCount": len(group),
                "rendererShapeCounts": dict(sorted(shapes.items())),
            }
        )
    source_rows = []
    for row in materialized:
        source_rows.append(
            {
                "sourceNode": row["sourceNode"],
                "fullSourceKey": row["fullSourceKey"],
                "occurrenceId": row["occurrenceId"],
                "carrierKey": row["carrierKey"],
                "clipOccurrenceId": row["clipOccurrenceId"],
                "effectAssetId": row["targetEffectAssetId"],
                "rendererShape": row["rendererShape"],
                "sourceTimeSeconds": row["sourceTimeSeconds"],
                "originalMaterial": copy.deepcopy(row["originalMaterial"]),
                "targetMaterial": {
                    "templateId": "effect.standard",
                    "renderProfile": "alpha_two_sided_depth_read",
                    "sourceProfileEnabled": False,
                },
            }
        )
    document_outputs = [
        {
            "effectAssetId": effect_id,
            "path": f"Data/Effects/Authored/{effect_id}.effect.json",
            "elementCount": len(document["elements"]),
            "canonicalSha256": canonical_sha256(document),
        }
        for effect_id, document in sorted(target_documents.items())
    ]
    return {
        "schema": "lostark.valtan-carrier-v1-materialization-receipt",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "policy": (
            "REVIEWED_REACHABLE_EXECUTABLE_CORE_ONLY; "
            "EXACT_CARRIER_IDENTITY; COMMON_ALPHA_TRANSLUCENT_FALLBACK; "
            "UNKNOWN_AND_BLOCKED_RECEIPT_ONLY"
        ),
        "summary": {
            **copy.deepcopy(EXPECTED),
            "newlySeededProjectionCount": EXPECTED[
                "baselineNewExactProjectionCount"
            ],
            "migratedExistingProjectionCount": EXPECTED[
                "baselineExistingExactProjectionCount"
            ],
            "retiredEffectCount": len(retired_effect_ids),
            "retiredCueCount": len(retired_cue_ids),
            "finalValtanProductRowCount": (
                sum(len(row["elements"]) for row in target_documents.values())
                + len(target_protected_document["elements"])
                + len(sky_document["elements"])
            ),
        },
        "decalExpansion": {
            "newClipOccurrenceIds": copy.deepcopy(DECAL_EXPANDED_CLIP_IDS),
            "newPatternIds": copy.deepcopy(DECAL_EXPANDED_PATTERN_IDS),
            "exactProjectionCount": EXPECTED[
                "reviewedCoreDecalProjectionCount"
            ],
        },
        "ownershipExceptions": [
            {
                "clipOccurrenceId": PROTECTED_CLIP_ID,
                "effectAssetId": PROTECTED_EFFECT_ID,
                "disposition": "PRESERVE_9_ROW_CANARY_RESEAL_2_WMODEL_SCALE_FIELDS",
                "coveredReviewedCoreProjectionCount": EXPECTED[
                    "protectedProjectionCount"
                ],
                "elementCount": len(target_protected_document["elements"]),
                "documentCanonicalSha256": canonical_sha256(
                    target_protected_document
                ),
                "documentByteSha256": byte_sha256(
                    pretty_bytes(target_protected_document)
                ),
                "catalogRowCanonicalSha256": canonical_sha256(
                    catalog_by_effect[PROTECTED_EFFECT_ID]
                ),
                "cueRowCanonicalSha256": canonical_sha256(protected_cue),
            },
            {
                "clipOccurrenceId": RED_BLADE_CLIP_ID,
                "effectAssetId": RED_BLADE_EFFECT_ID,
                "disposition": "REPLACE_IN_COMBAT_OBJECT_OWNED_EFFECT_NO_BOSS_ROOT_CUE",
                "projectionCount": sum(
                    row["clipOccurrenceId"] == RED_BLADE_CLIP_ID
                    for row in materialized
                ),
            },
            {
                "effectAssetId": "effect.valtan.sky-axe.active",
                "disposition": "BLOCKED_EXTERNAL_OWNER_HASHED_EXCEPTION",
                "elementCount": len(sky_document["elements"]),
                "documentCanonicalSha256": canonical_sha256(sky_document),
                "documentByteSha256": byte_sha256(
                    (AUTHORED_ROOT / "effect.valtan.sky-axe.active.effect.json")
                    .read_bytes()
                ),
                "catalogRowCanonicalSha256": canonical_sha256(
                    catalog_by_effect["effect.valtan.sky-axe.active"]
                ),
            },
        ],
        "blocked": blocker_summary,
        "reviewedProjectionLedger": copy.deepcopy(
            blockers["reviewedProjectionLedger"]
        ),
        "reviewedSourceOnlyOccurrences": copy.deepcopy(
            blockers["reviewedSourceOnlyOccurrences"]
        ),
        "clipGroups": clip_rows,
        "sourceElements": source_rows,
        "legacyMigration": {
            "strictLegacyRowCount": EXPECTED["baselineStrictLegacyRowCount"],
            "strictLegacyDocumentCount": EXPECTED[
                "baselineStrictLegacyDocumentCount"
            ],
            "documents": migration_rows,
            "retiredEffectAssetIds": retired_effect_ids,
            "retiredCueBindingIds": retired_cue_ids,
        },
        "outputs": {
            "catalog": {
                "path": repository_path(CATALOG_PATH),
                "scope": "EFFECT_ASSET_ID_PREFIX",
                "effectAssetIdPrefix": "effect.valtan.",
                "effectCount": len(valtan_catalog_projection["effects"]),
                "canonicalSha256": canonical_sha256(
                    valtan_catalog_projection
                ),
            },
            "cues": {
                "path": repository_path(CUE_PATH),
                "cueCount": len(cue_receipt_projection["cues"]),
                "canonicalSha256": canonical_sha256(
                    cue_receipt_projection
                ),
            },
            "targetDocuments": document_outputs,
            "protectedWhirlwindDocument": {
                "effectAssetId": PROTECTED_EFFECT_ID,
                "path": (
                    "Data/Effects/Authored/"
                    f"{PROTECTED_EFFECT_ID}.effect.json"
                ),
                "elementCount": len(target_protected_document["elements"]),
                "canonicalSha256": canonical_sha256(
                    target_protected_document
                ),
                "byteSha256": byte_sha256(
                    pretty_bytes(target_protected_document)
                ),
            },
        },
    }


def _validate_existing_receipt(
    receipt: dict[str, Any],
    target_documents: dict[str, dict[str, Any]],
    target_protected_document: dict[str, Any],
    sky_document: dict[str, Any],
    blockers: dict[str, Any],
    target_catalog: dict[str, Any],
    target_cues: dict[str, Any],
    successor_receipt: dict[str, Any],
) -> None:
    if (
        receipt.get("schema")
        != "lostark.valtan-carrier-v1-materialization-receipt"
        or receipt.get("formatVersion") != 1
        or receipt.get("summary", {}).get("materializedProjectionCount")
        != EXPECTED["materializedProjectionCount"]
    ):
        raise MaterializeError("applied carrier V1 receipt is invalid")
    outputs = receipt.get("outputs") or {}
    catalog_output = outputs.get("catalog") or {}
    valtan_catalog_projection = _valtan_catalog_receipt_projection(
        target_catalog, successor_receipt
    )
    cue_receipt_projection = _cue_receipt_projection(
        target_cues, successor_receipt
    )
    if (
        catalog_output.get("scope") != "EFFECT_ASSET_ID_PREFIX"
        or catalog_output.get("effectAssetIdPrefix") != "effect.valtan."
        or catalog_output.get("effectCount")
        != len(valtan_catalog_projection["effects"])
        or catalog_output.get("canonicalSha256")
        != canonical_sha256(valtan_catalog_projection)
        or (outputs.get("cues") or {}).get("canonicalSha256")
        != canonical_sha256(cue_receipt_projection)
    ):
        raise MaterializeError("applied carrier V1 catalog/cue receipt drifted")
    expected_docs = {
        str(row.get("effectAssetId") or ""): str(
            row.get("canonicalSha256") or ""
        )
        for row in (outputs.get("targetDocuments") or [])
    }
    actual_docs = {
        effect_id: canonical_sha256(document)
        for effect_id, document in target_documents.items()
    }
    successor_ids = set(
        successor_lineage.successor_documents(successor_receipt)
    )
    expected_managed = {
        effect_id: digest
        for effect_id, digest in expected_docs.items()
        if effect_id not in successor_ids
    }
    actual_managed = {
        effect_id: digest
        for effect_id, digest in actual_docs.items()
        if effect_id not in successor_ids
    }
    if expected_managed != actual_managed:
        raise MaterializeError("applied carrier V1 target documents drifted")
    try:
        historical_projection_ledger = (
            successor_lineage.project_historical_reviewed_projection_ledger(
                blockers["reviewedProjectionLedger"], successor_receipt
            )
        )
        historical_source_only = (
            successor_lineage.project_historical_reviewed_source_only_occurrences(
                blockers["reviewedSourceOnlyOccurrences"],
                successor_receipt,
            )
        )
    except successor_lineage.SuccessorLineageError as error:
        raise MaterializeError(str(error)) from error
    if (
        historical_projection_ledger
        != (receipt.get("reviewedProjectionLedger") or [])
        or historical_source_only
        != (receipt.get("reviewedSourceOnlyOccurrences") or [])
    ):
        raise MaterializeError("applied reviewed projection ledger drifted")
    protected_output = outputs.get("protectedWhirlwindDocument") or {}
    if (
        protected_output.get("canonicalSha256")
        != canonical_sha256(target_protected_document)
        or protected_output.get("byteSha256")
        != byte_sha256(pretty_bytes(target_protected_document))
        or protected_output.get("elementCount") != 9
    ):
        raise MaterializeError(
            "applied protected Whirlwind document receipt drifted"
        )
    exceptions = {
        str(row.get("effectAssetId") or ""): row
        for row in receipt.get("ownershipExceptions", [])
    }
    sky = exceptions.get("effect.valtan.sky-axe.active") or {}
    sky_path = AUTHORED_ROOT / "effect.valtan.sky-axe.active.effect.json"
    catalog_by_effect = _catalog_index(target_catalog)
    protected_cue = next(
        row
        for row in _cue_index(cue_receipt_projection).values()
        if row.get("effectAssetId") == PROTECTED_EFFECT_ID
    )
    protected = exceptions.get(PROTECTED_EFFECT_ID) or {}
    if (
        protected.get("catalogRowCanonicalSha256")
        != canonical_sha256(catalog_by_effect[PROTECTED_EFFECT_ID])
        or protected.get("cueRowCanonicalSha256")
        != canonical_sha256(protected_cue)
    ):
        raise MaterializeError("applied protected Whirlwind owner seal drifted")
    if (
        sky.get("disposition") != "BLOCKED_EXTERNAL_OWNER_HASHED_EXCEPTION"
        or sky.get("elementCount") != 3
        or sky.get("catalogRowCanonicalSha256")
        != canonical_sha256(catalog_by_effect["effect.valtan.sky-axe.active"])
    ):
        raise MaterializeError("applied historical Sky Axe exception drifted")


def _incremental_source_element(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "sourceNode": row["sourceNode"],
        "fullSourceKey": row["fullSourceKey"],
        "occurrenceId": row["occurrenceId"],
        "carrierKey": row["carrierKey"],
        "clipOccurrenceId": row["clipOccurrenceId"],
        "effectAssetId": row["targetEffectAssetId"],
        "rendererShape": row["rendererShape"],
        "sourceTimeSeconds": row["sourceTimeSeconds"],
        "originalMaterial": copy.deepcopy(row["originalMaterial"]),
        "targetMaterial": {
            "templateId": "effect.standard",
            "renderProfile": "alpha_two_sided_depth_read",
            "sourceProfileEnabled": False,
        },
    }


def _incremental_clip_group(group: list[dict[str, Any]]) -> dict[str, Any]:
    first = group[0]
    shapes = Counter(row["rendererShape"] for row in group)
    return {
        "patternId": first["patternId"],
        "semanticStageId": first["semanticStageId"],
        "gameplayActionId": first["gameplayActionId"],
        "clipOccurrenceId": first["clipOccurrenceId"],
        "effectAssetId": first["targetEffectAssetId"],
        "cueBindingId": _target_cue_id(first["clipOccurrenceId"]),
        "owner": "VALTAN_PATTERN_EFFECT_CUE",
        "projectionCount": len(group),
        "rendererShapeCounts": dict(sorted(shapes.items())),
    }


BASE_RECEIPT_EVIDENCE_SHA256 = {
    "ownershipExceptions": "68423be95a97fe29dd207e5f820ea46a40d30d0c0a26661bd85ba21530bee69b",
    "reviewedProjectionLedger": "ba178c9657cabf38a381321e66784824a2dc0bf007d89940993c93772439740b",
    "reviewedSourceOnlyOccurrences": "62fd2fdf2e8df805b9022bc7cf05536416bbc2bd7e41b77b689d1c3106a59d6d",
    "clipGroups": "e4a084a83948d3e044236f74cdd3a003b1a53b31fb991cae65c6633c1a1bbf85",
    "sourceElements": "d81c752f4dc7fd49fb68cb29a17a3364aab76c64dddae8ed0abb9730a1f2d6cb",
    "legacyMigration": "803e7dd27f7ef8fbab5ca1f6d8b2b686c5f06ab905a48c9fd2289203981a652b",
    "outputs": "f6f7c3e15a35944a597403d69c0465d4d7e72b296b7c5e854c2b7f5ec3369968",
    "protectedWhirlwindExactAliasProof": "43f378f91dfe6c0b99b4a3a24383441e1a6fa27e2940ba5b0284da690222095b",
    "fourSlashPatternSplitOwnerReseal": "5839e1896a2f0b22e1c8da5863ee79eda8c1f539fc1b5a90773de691bdca384d",
    "clip01ScreenPostSuccessorOverlay": "d26bddb9d449ca600998ad8c1797254e0e90b2a24251174752ef556d6961dd44",
    "retiredOwnerSuccessorMappings": "ec138f334a6d6b3f3344bf96dce0a2c3bb6febc5fb9b376e4f64e05890072247",
    "productReset": "dfd0036961b4ae0b5c33e47a02286b61361ee3da8561f116ce211c98d9de5316",
    "summary": "46a2888826f10803753dd6106ed9e2be229f00935ff223aeb633f3281a44af10",
    "decalExpansion": "5ab23cb56afcb36a0bc39f4305158bc3f7c1061a50a3ff1d460930f88e9f2b69",
    "blocked": "4f8df70aeb90b9ce6c10ff049323f3060c10ad8f42f40563a22ff0fe35712d04",
}

BASE_CATALOG_OUTPUT = {
    "path": "Data/Effects/EffectCatalog.json",
    "scope": "EFFECT_ASSET_ID_PREFIX",
    "effectAssetIdPrefix": "effect.valtan.",
    "effectCount": 46,
    "canonicalSha256": "123c070157e743ef467294607f104a9e5f1d90c3c99f73b6cf9c48033da093da",
}
BASE_CUE_OUTPUT = {
    "path": "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json",
    "cueCount": 44,
    "canonicalSha256": "4ff3c88cffdbe84abb99aaee22aad86c92f1b1797dfd8706058ded489b738dc9",
}
BASE_PROTECTED_BYTE_SHA256 = (
    "d51b3ef44d9e9a7ca43c26164721bbac0c2e4a632deb1032b251b570452841c1"
)
BASE_OWNED_CATALOG_PROJECTION_SHA256 = (
    "e429d41b8c2734c0d3929e5c7bd56613d021610b1ecf80b4d5fbf14eb83faef9"
)
BASE_CUE_ROW_PROJECTION_SHA256 = (
    "8b6973e4e1ed332858be67231e9595cb342330cf6ec433921fa3034a3aef5e9d"
)
BASE_RECEIPT_CANONICAL_SHA256 = (
    "92a102cb0b25d0d669014fdce0c4a4e08912b68530960b533769d7d10442cc99"
)
BASE_BLOCKED = {
    "expandedCarrierCount": 917,
    "dispositionCounts": {
        "DEFERRED_GENERIC_DUST": 19,
        "DEFERRED_LIGHT": 40,
        "MISSING_RUNTIME_RESOURCE": 378,
        "UNRESOLVED_RUNTIME_ADAPTER": 480,
    },
    "rendererShapeCounts": {
        "light": 40,
        "mesh": 141,
        "screenPost": 41,
        "sprite": 695,
    },
    "reviewedOccurrenceWithoutCarrierCount": 197,
    "productAdmission": False,
}
BASE_SUMMARY = {
    "reviewedCoreProjectionCount": 660,
    "reviewedCoreSpriteProjectionCount": 455,
    "reviewedCoreMeshProjectionCount": 173,
    "reviewedCoreDecalProjectionCount": 32,
    "reviewedCoreClipCount": 45,
    "reviewedCorePatternCount": 24,
    "reviewedProjectionLedgerCount": 1577,
    "protectedProjectionCount": 3,
    "materializedProjectionCount": 657,
    "materializedClipGroupCount": 44,
    "newClipDocumentCount": 43,
    "baselineExistingExactProjectionCount": 539,
    "baselineNewExactProjectionCount": 118,
    "baselineStrictLegacyRowCount": 3032,
    "baselineStrictLegacyDocumentCount": 97,
    "baselineValtanCatalogCount": 108,
    "baselineBossRootCueCount": 106,
    "baselineValtanProductRowCount": 3624,
    "baselineRemovedExistingRowCount": 3612,
    "baselineRetiredEffectCount": 105,
    "baselineRetiredCueCount": 105,
    "finalValtanCatalogCount": 46,
    "finalBossRootCueCount": 44,
    "blockedExpandedCarrierCount": 917,
    "reviewedOccurrenceWithoutCarrierCount": 197,
    "newlySeededProjectionCount": 118,
    "migratedExistingProjectionCount": 539,
    "retiredEffectCount": 105,
    "retiredCueCount": 105,
    "finalValtanProductRowCount": 669,
}
BASE_DECAL_EXPANSION = {
    "newClipOccurrenceIds": [
        "valtan.attack.ground-wave-smash.active.clip.02",
        "valtan.attack.jump-spin.jump.clip.01",
    ],
    "newPatternIds": ["VALTAN_JUMP_SPIN"],
    "exactProjectionCount": 32,
}


def _validate_and_project_base_receipt_evidence(
    receipt: dict[str, Any],
    successor_receipt: dict[str, Any],
    target_effect_ids: set[str],
    catalog_source: dict[str, Any],
    cues_source: dict[str, Any],
    additive_delta: dict[str, list[dict[str, Any]]],
    blockers: dict[str, Any],
) -> dict[str, Any]:
    """Validate the historical receipt and return its unmodified base scope.

    A prior interrupted write may still contain the exact reviewed delta in the
    historical arrays.  Such rows are accepted only when they byte-for-byte
    match the regenerated delta, then projected out.  No unrelated historical
    value is repaired or resealed.
    """

    base_ledger = [
        row
        for row in receipt.get("reviewedProjectionLedger") or []
        if row.get("patternId") != "VALTAN_FOUR_PILLARS_105"
    ]
    base_source_only = [
        row
        for row in receipt.get("reviewedSourceOnlyOccurrences") or []
        if row.get("patternId") != "VALTAN_FOUR_PILLARS_105"
    ]
    base_clip_groups = [
        row
        for row in receipt.get("clipGroups") or []
        if row.get("effectAssetId") not in target_effect_ids
    ]
    base_source_elements = [
        row
        for row in receipt.get("sourceElements") or []
        if row.get("effectAssetId") not in target_effect_ids
    ]
    present_delta = {
        "reviewedProjectionLedger": [
            row
            for row in receipt.get("reviewedProjectionLedger") or []
            if row.get("patternId") == "VALTAN_FOUR_PILLARS_105"
        ],
        "reviewedSourceOnlyOccurrences": [
            row
            for row in receipt.get("reviewedSourceOnlyOccurrences") or []
            if row.get("patternId") == "VALTAN_FOUR_PILLARS_105"
        ],
        "sourceElements": [
            row
            for row in receipt.get("sourceElements") or []
            if row.get("effectAssetId") in target_effect_ids
        ],
        "clipGroups": [
            row
            for row in receipt.get("clipGroups") or []
            if row.get("effectAssetId") in target_effect_ids
        ],
    }
    for name, rows in present_delta.items():
        expected_rows = additive_delta[name]
        if rows and canonical_sha256(rows) != canonical_sha256(expected_rows):
            raise MaterializeError(
                "interrupted 100-bar receipt delta drifted: " + name
            )

    projected = copy.deepcopy(receipt)
    projected.pop("additiveReviewedOwnerAppend", None)
    projected["reviewedProjectionLedger"] = copy.deepcopy(base_ledger)
    projected["reviewedSourceOnlyOccurrences"] = copy.deepcopy(base_source_only)
    projected["clipGroups"] = copy.deepcopy(base_clip_groups)
    projected["sourceElements"] = copy.deepcopy(base_source_elements)
    base_sections = {
        "reviewedProjectionLedger": base_ledger,
        "reviewedSourceOnlyOccurrences": base_source_only,
        "clipGroups": base_clip_groups,
        "sourceElements": base_source_elements,
        "legacyMigration": projected.get("legacyMigration"),
        "protectedWhirlwindExactAliasProof": projected.get(
            "protectedWhirlwindExactAliasProof"
        ),
        "fourSlashPatternSplitOwnerReseal": projected.get(
            "fourSlashPatternSplitOwnerReseal"
        ),
        "clip01ScreenPostSuccessorOverlay": projected.get(
            "clip01ScreenPostSuccessorOverlay"
        ),
    }

    outputs = copy.deepcopy(projected.get("outputs") or {})
    outputs["targetDocuments"] = [
        copy.deepcopy(row)
        for row in outputs.get("targetDocuments") or []
        if row.get("effectAssetId") not in target_effect_ids
    ]
    projected["outputs"] = outputs
    base_sections["outputs"] = outputs

    ownership = copy.deepcopy(projected.get("ownershipExceptions") or [])
    base_sections["ownershipExceptions"] = ownership

    base_successors = copy.deepcopy(
        projected.get("retiredOwnerSuccessorMappings") or []
    )
    base_sections["retiredOwnerSuccessorMappings"] = base_successors

    base_product_reset = copy.deepcopy(projected.get("productReset") or {})
    base_sections["productReset"] = base_product_reset
    base_sections["summary"] = projected.get("summary")
    base_sections["decalExpansion"] = projected.get("decalExpansion")

    additive_blocked = {
        "expandedCarrierCount": BASE_BLOCKED["expandedCarrierCount"]
        + int(blockers["expandedCarrierCount"]),
        "dispositionCounts": dict(
            sorted(
                (
                    Counter(BASE_BLOCKED["dispositionCounts"])
                    + Counter(blockers["dispositionCounts"])
                ).items()
            )
        ),
        "rendererShapeCounts": dict(
            sorted(
                (
                    Counter(BASE_BLOCKED["rendererShapeCounts"])
                    + Counter(blockers["rendererShapeCounts"])
                ).items()
            )
        ),
        "reviewedOccurrenceWithoutCarrierCount": (
            BASE_BLOCKED["reviewedOccurrenceWithoutCarrierCount"]
            + int(blockers["reviewedOccurrenceWithoutCarrierCount"])
        ),
        "productAdmission": False,
    }
    current_blocked = projected.get("blocked")
    if current_blocked not in (BASE_BLOCKED, additive_blocked):
        raise MaterializeError("historical carrier blocked evidence drifted")
    projected["blocked"] = copy.deepcopy(BASE_BLOCKED)
    base_sections["blocked"] = projected["blocked"]

    for name, value in base_sections.items():
        if canonical_sha256(value) != BASE_RECEIPT_EVIDENCE_SHA256[name]:
            raise MaterializeError(
                "pre-existing carrier receipt evidence drifted: " + name
            )
    if canonical_sha256(projected) != BASE_RECEIPT_CANONICAL_SHA256:
        raise MaterializeError(
            "pre-existing carrier receipt full evidence drifted"
        )

    # The receipt seals the original 46-owner carrier product, not every
    # supplemental Valtan owner currently present in the shared catalog.  Seal
    # that exact owned projection before appending the two reviewed 100-bar
    # owners so unrelated catalog/cue/doc drift cannot be absorbed by a new
    # cumulative receipt hash.
    base_document_rows = outputs["targetDocuments"]
    if len(base_document_rows) != 44:
        raise MaterializeError("historical carrier target document scope drifted")
    base_document_ids = {
        str(row.get("effectAssetId") or "") for row in base_document_rows
    }
    if len(base_document_ids) != 44 or "" in base_document_ids:
        raise MaterializeError("historical carrier target document identity drifted")

    successor_document_ids = set(
        successor_lineage.successor_documents(successor_receipt)
    )
    four_slash_id = (
        "effect.valtan.carrier-v1.attack.four-slash.active.clip-01"
    )
    for row in base_document_rows:
        effect_id = str(row["effectAssetId"])
        document = read_json(ROOT / str(row["path"]))
        _validate_document(document, effect_id)
        actual_count = len(document["elements"])
        actual_hash = canonical_sha256(document)
        if effect_id == four_slash_id:
            try:
                occurrence_v2_migration.validate_four_slash_clip01_screen_post_overlay(
                    projected, document
                )
            except occurrence_v2_migration.MigrationError as error:
                raise MaterializeError(str(error)) from error
            continue
        if effect_id in successor_document_ids:
            continue
        if (
            actual_count != row["elementCount"]
            or actual_hash != row["canonicalSha256"]
        ):
            raise MaterializeError(
                "pre-existing carrier target document drifted: " + effect_id
            )

    protected_row = outputs["protectedWhirlwindDocument"]
    protected = read_json(ROOT / str(protected_row["path"]))
    _validate_document(protected, PROTECTED_EFFECT_ID)
    if (
        len(protected["elements"]) != protected_row["elementCount"]
        or canonical_sha256(protected) != protected_row["canonicalSha256"]
        or byte_sha256(pretty_bytes(protected)) != BASE_PROTECTED_BYTE_SHA256
    ):
        raise MaterializeError("protected Whirlwind document drifted")

    sky_rows = [
        row
        for row in ownership
        if row.get("effectAssetId") == "effect.valtan.sky-axe.active"
    ]
    if len(sky_rows) != 1:
        raise MaterializeError("Sky Axe ownership exception drifted")
    sky_row = sky_rows[0]
    sky = read_json(
        AUTHORED_ROOT / "effect.valtan.sky-axe.active.effect.json"
    )
    _validate_document(sky, "effect.valtan.sky-axe.active")
    if "effect.valtan.sky-axe.active" not in successor_document_ids and (
        len(sky["elements"]) != sky_row["elementCount"]
        or canonical_sha256(sky) != sky_row["documentCanonicalSha256"]
        or byte_sha256(pretty_bytes(sky)) != sky_row["documentByteSha256"]
    ):
        raise MaterializeError("Sky Axe external owner document drifted")

    base_owned_ids = base_document_ids | {
        PROTECTED_EFFECT_ID,
        "effect.valtan.sky-axe.active",
    }
    if len(base_owned_ids) != 46:
        raise MaterializeError("historical carrier catalog owner scope drifted")
    successor_base_catalog = copy.deepcopy(catalog_source)
    successor_base_catalog["effects"] = [
        copy.deepcopy(row)
        for row in catalog_source["effects"]
        if str(row.get("effectAssetId") or "") not in target_effect_ids
    ]
    successor_base_cues = copy.deepcopy(cues_source)
    successor_base_cues["cues"] = [
        copy.deepcopy(row)
        for row in cues_source["cues"]
        if str(row.get("effectAssetId") or "") not in target_effect_ids
    ]
    try:
        historical_catalog_source = (
            successor_lineage.project_historical_catalog(
                successor_base_catalog, successor_receipt
            )
        )
        historical_cues_source = successor_lineage.project_historical_cues(
            successor_base_cues, successor_receipt
        )
    except successor_lineage.SuccessorLineageError as error:
        raise MaterializeError(str(error)) from error
    catalog_rows = _catalog_index(historical_catalog_source)
    if not base_owned_ids <= set(catalog_rows):
        raise MaterializeError("historical carrier catalog owner is missing")
    base_catalog_projection = {
        "effects": [
            copy.deepcopy(catalog_rows[effect_id])
            for effect_id in sorted(base_owned_ids)
        ]
    }
    if (
        canonical_sha256(base_catalog_projection)
        != BASE_OWNED_CATALOG_PROJECTION_SHA256
    ):
        raise MaterializeError("historical carrier-owned catalog rows drifted")

    cue_rows = _cue_index(historical_cues_source)
    base_cue_rows = sorted(
        (
            copy.deepcopy(row)
            for row in cue_rows.values()
            if row.get("effectAssetId") in base_owned_ids
        ),
        key=lambda row: str(row["bindingId"]),
    )
    # The immutable receipt seals the original 44 rows.  Pattern occurrence
    # successor migrations may legitimately rebind their semantic owner while
    # keeping the same carrier cue identities, so the successor receipt (fully
    # validated above) owns their live row values.
    if len(base_cue_rows) != 44:
        raise MaterializeError("historical carrier-owned cue rows drifted")
    return projected


def _build_additive_outputs(
    core: list[dict[str, Any]],
    materialized: list[dict[str, Any]],
    blockers: dict[str, Any],
    successor_receipt: dict[str, Any],
) -> tuple[str, dict[Path, bytes], dict[str, Any]]:
    """Append the reviewed 100-bar branch without resetting other Valtan owners."""

    if len(core) != 26 or len(materialized) != 26:
        raise MaterializeError("100-bar additive exact denominator drifted")
    groups: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in materialized:
        groups[row["clipOccurrenceId"]].append(row)
    if set(groups) != {
        "valtan.mechanic.four-pillars-105.takeoff.clip.01",
        "valtan.mechanic.four-pillars-105.target-cone.clip.01",
    }:
        raise MaterializeError("100-bar exact clip closure drifted")

    catalog_source = read_json(CATALOG_PATH)
    cues_source = read_json(CUE_PATH)
    catalog_rows = _catalog_index(catalog_source)
    cue_rows = _cue_index(cues_source)
    target_effect_ids = {
        str(row["targetEffectAssetId"]) for row in materialized
    }
    target_cue_ids = {_target_cue_id(clip_id) for clip_id in groups}
    if (
        target_effect_ids != FOUR_PILLARS_EFFECT_IDS
        or target_cue_ids != FOUR_PILLARS_CUE_IDS
    ):
        raise MaterializeError("100-bar additive Product identity drifted")
    if any("high-jump" in value for value in target_effect_ids):
        raise MaterializeError("100-bar branch reused a High Jump owner")

    current_effect_hits = target_effect_ids & set(catalog_rows)
    current_cue_hits = target_cue_ids & set(cue_rows)
    if not current_effect_hits and not current_cue_hits:
        state = "BASELINE_ADDITIVE"
    elif (
        current_effect_hits == target_effect_ids
        and current_cue_hits == target_cue_ids
    ):
        state = "APPLIED"
    else:
        raise MaterializeError("100-bar carrier Product append is partial")

    base_catalog, base_cues = _validate_successor_base_product(
        successor_receipt, catalog_source, cues_source
    )
    base_catalog_rows = _catalog_index(base_catalog)
    base_cue_rows = _cue_index(base_cues)

    historical_receipt = read_json(RECEIPT_PATH)
    try:
        historical_catalog = successor_lineage.project_historical_catalog(
            base_catalog, successor_receipt
        )
        historical_cues = successor_lineage.project_historical_cues(
            base_cues, successor_receipt
        )
    except successor_lineage.SuccessorLineageError as error:
        raise MaterializeError(str(error)) from error
    if (
        sum(
            str(row.get("effectAssetId") or "").startswith("effect.valtan.")
            for row in historical_catalog["effects"]
        )
        != 46
        or len(historical_cues["cues"]) != 44
    ):
        raise MaterializeError("historical successor projection drifted")

    target_documents: dict[str, dict[str, Any]] = {}
    target_document_paths: dict[str, Path] = {}
    for clip_id in sorted(groups):
        group = sorted(groups[clip_id], key=lambda row: row["sourceNode"])
        effect_id = str(group[0]["targetEffectAssetId"])
        path = AUTHORED_ROOT / f"{effect_id}.effect.json"
        target_document_paths[effect_id] = path
        if effect_id in catalog_rows:
            if catalog_rows[effect_id] != _catalog_row(effect_id):
                raise MaterializeError(
                    "100-bar carrier catalog owner is rebound: " + effect_id
                )
            if path.is_file():
                document = read_json(path)
                _validate_document(document, effect_id)
                existing = {
                    str(row.get("sourceNode") or ""): row
                    for row in document["elements"]
                }
            else:
                document = _new_document(effect_id, group)
                existing = {}
        else:
            document = _new_document(effect_id, group)
            existing = {}
        document["elements"] = sorted(
            [
                _common_translucent_element(
                    row["element"], existing.get(row["sourceNode"])
                )
                for row in group
            ],
            key=lambda row: (
                str(row.get("sourceNode") or ""),
                str(row.get("id") or ""),
            ),
        )
        _validate_document(document, effect_id)
        expected_nodes = {row["sourceNode"] for row in group}
        actual_nodes = {
            str(row.get("sourceNode") or "") for row in document["elements"]
        }
        if actual_nodes != expected_nodes:
            raise MaterializeError(
                "100-bar exact owner source closure drifted: " + effect_id
            )
        target_documents[effect_id] = document

    target_catalog = copy.deepcopy(catalog_source)
    target_catalog_rows = copy.deepcopy(catalog_rows)
    for effect_id in target_effect_ids:
        target_catalog_rows[effect_id] = _catalog_row(effect_id)
    target_catalog["effects"] = copy.deepcopy(catalog_source["effects"])
    present_effect_ids = {
        str(row["effectAssetId"]) for row in target_catalog["effects"]
    }
    for effect_id in sorted(target_effect_ids):
        if effect_id in present_effect_ids:
            continue
        insertion = next(
            (
                index
                for index, row in enumerate(target_catalog["effects"])
                if str(row["effectAssetId"]) > effect_id
            ),
            len(target_catalog["effects"]),
        )
        target_catalog["effects"].insert(insertion, _catalog_row(effect_id))
        present_effect_ids.add(effect_id)
    if sum(
        str(row.get("effectAssetId") or "").startswith("effect.valtan.")
        for row in target_catalog["effects"]
    ) != 56:
        raise MaterializeError("100-bar final Valtan catalog closure drifted")

    target_cues = copy.deepcopy(cues_source)
    target_cue_rows = copy.deepcopy(cue_rows)
    present_cue_ids = {
        str(row["bindingId"]) for row in target_cues["cues"]
    }
    for clip_id in sorted(groups):
        generated = _cue_row(groups[clip_id])
        binding_id = str(generated["bindingId"])
        current = cue_rows.get(binding_id)
        if current is not None and current != generated:
            raise MaterializeError(
                "100-bar carrier cue is rebound: " + binding_id
            )
        target_cue_rows[binding_id] = generated
        if binding_id in present_cue_ids:
            continue
        insertion = next(
            (
                index
                for index, row in enumerate(target_cues["cues"])
                if str(row["bindingId"]) > binding_id
            ),
            len(target_cues["cues"]),
        )
        target_cues["cues"].insert(insertion, generated)
        present_cue_ids.add(binding_id)
    _cue_index(target_cues)
    if len(target_cues["cues"]) != 49:
        raise MaterializeError("100-bar final cue closure drifted")

    new_ledger = sorted(
        copy.deepcopy(blockers["reviewedProjectionLedger"]),
        key=lambda row: (row["occurrenceFullKey"], row["carrierKey"]),
    )
    new_source_only = sorted(
        copy.deepcopy(blockers["reviewedSourceOnlyOccurrences"]),
        key=lambda row: row["occurrenceFullKey"],
    )
    new_source_elements = sorted(
        (_incremental_source_element(row) for row in materialized),
        key=lambda row: row["fullSourceKey"],
    )
    new_clip_groups = sorted(
        (_incremental_clip_group(groups[clip_id]) for clip_id in groups),
        key=lambda row: row["clipOccurrenceId"],
    )
    new_ledger_by_key = {
        (row["occurrenceFullKey"], row["carrierKey"]): row
        for row in new_ledger
    }
    new_core = [
        row for row in new_ledger if row["disposition"] == "EXECUTABLE_CORE"
    ]
    if (
        len(new_ledger_by_key) != 54
        or len(new_source_only) != 9
        or len(new_source_elements) != 26
        or len(new_clip_groups) != 2
        or len(new_core) != 26
        or Counter(row["rendererShape"] for row in new_core)
        != Counter({"sprite": 16, "mesh": 8, "decal": 2})
    ):
        raise MaterializeError("100-bar additive ledger identity drifted")
    additive_delta = {
        "reviewedProjectionLedger": new_ledger,
        "reviewedSourceOnlyOccurrences": new_source_only,
        "sourceElements": new_source_elements,
        "clipGroups": new_clip_groups,
    }

    source_receipt = copy.deepcopy(historical_receipt)
    if (
        source_receipt.get("schema")
        != "lostark.valtan-carrier-v1-materialization-receipt"
        or source_receipt.get("formatVersion") != 1
    ):
        raise MaterializeError("carrier V1 receipt header is invalid")
    receipt = _validate_and_project_base_receipt_evidence(
        source_receipt,
        successor_receipt,
        target_effect_ids,
        catalog_source,
        cues_source,
        additive_delta,
        blockers,
    )

    target_catalog_index = _catalog_index(target_catalog)
    target_cue_index = _cue_index(target_cues)
    additive_documents = []
    additive_catalog_rows = []
    additive_cue_rows = []
    for effect_id in sorted(target_effect_ids):
        document = target_documents[effect_id]
        document_path = target_document_paths[effect_id]
        catalog_row = target_catalog_index[effect_id]
        clip_id = next(
            row["clipOccurrenceId"]
            for row in materialized
            if row["targetEffectAssetId"] == effect_id
        )
        cue_row = target_cue_index[_target_cue_id(clip_id)]
        additive_documents.append(
            {
                "effectAssetId": effect_id,
                "path": repository_path(document_path),
                "elementCount": len(document["elements"]),
                "canonicalSha256": canonical_sha256(document),
                "byteSha256": byte_sha256(pretty_bytes(document)),
            }
        )
        additive_catalog_rows.append(
            {
                "effectAssetId": effect_id,
                "rowCanonicalSha256": canonical_sha256(catalog_row),
            }
        )
        additive_cue_rows.append(
            {
                "bindingId": cue_row["bindingId"],
                "rowCanonicalSha256": canonical_sha256(cue_row),
            }
        )
    successor_mappings = []
    for clip_id in sorted(groups):
        group = groups[clip_id]
        successor_mappings.append(
            {
                "clipOccurrenceId": clip_id,
                "disposition": "REPLACED_BY_EXACT_CARRIER_V1_CLIP_OWNER",
                "replacementBindingId": _target_cue_id(clip_id),
                "replacementEffectAssetId": str(
                    group[0]["targetEffectAssetId"]
                ),
            }
        )
    _, _, _, additive_input_evidence = project_successor_base_inputs(
        successor_receipt,
        read_json(source_inventory.PATTERN_BINDINGS_PATH),
        read_json(source_inventory.ENCOUNTER_PATH),
        source_inventory.load_selection_manifest(
            reviewed_candidates.SELECTION_PATH
        ),
    )

    base_valtan_projection = {
        "effects": sorted(
            (
                copy.deepcopy(row)
                for row in base_catalog["effects"]
                if str(row.get("effectAssetId") or "").startswith(
                    "effect.valtan."
                )
            ),
            key=lambda row: str(row["effectAssetId"]),
        )
    }
    final_valtan_projection = {
        "effects": sorted(
            (
                copy.deepcopy(row)
                for row in target_catalog["effects"]
                if str(row.get("effectAssetId") or "").startswith(
                    "effect.valtan."
                )
            ),
            key=lambda row: str(row["effectAssetId"]),
        )
    }
    expected_append = {
        "schema": (
            "lostark.valtan-carrier-v1-four-pillars-additive-receipt"
        ),
        "formatVersion": 2,
        "patternId": "VALTAN_FOUR_PILLARS_105",
        "sourceActionId": 420610,
        "sequenceIndex": 2,
        "sourceStagePath": [19, 20, 21, 22],
        "reviewBasis": (
            "YouTube tSpXa0v9TXw 19:32-19:43 reviewed against source "
            "420610 sequence 2"
        ),
        "baseReceiptCanonicalSha256": BASE_RECEIPT_CANONICAL_SHA256,
        "successorReceipt": {
            "path": repository_path(SUCCESSOR_RECEIPT_PATH),
            "canonicalSha256": canonical_sha256(successor_receipt),
        },
        "successorBaseInputProjection": additive_input_evidence,
        "baseReceiptSectionCanonicalSha256": copy.deepcopy(
            BASE_RECEIPT_EVIDENCE_SHA256
        ),
        "deltaEvidence": {
            "reviewedProjectionLedger": {
                "count": len(new_ledger),
                "executableCoreCount": len(new_core),
                "canonicalSha256": canonical_sha256(new_ledger),
            },
            "reviewedSourceOnlyOccurrences": {
                "count": len(new_source_only),
                "canonicalSha256": canonical_sha256(new_source_only),
            },
            "sourceElements": {
                "count": len(new_source_elements),
                "canonicalSha256": canonical_sha256(new_source_elements),
            },
            "clipGroups": {
                "count": len(new_clip_groups),
                "canonicalSha256": canonical_sha256(new_clip_groups),
            },
            "blocked": {
                "expandedCarrierCount": int(blockers["expandedCarrierCount"]),
                "reviewedOccurrenceWithoutCarrierCount": int(
                    blockers["reviewedOccurrenceWithoutCarrierCount"]
                ),
                "canonicalSha256": canonical_sha256(blockers),
            },
        },
        "historicalDerivedCounts": {
            "carrierCatalogOwnerCount": {
                "base": 46,
                "delta": 2,
                "historicalDerived": 48,
            },
            "bossRootCueCount": {
                "base": 44,
                "delta": 2,
                "historicalDerived": 46,
            },
            "carrierProductRowCount": {
                "base": 669,
                "delta": 26,
                "historicalDerived": 695,
                "meaning": (
                    "RECEIPT_ARITHMETIC_ONLY_NOT_CURRENT_PHYSICAL_PRODUCT_COUNT"
                ),
            },
            "reviewedProjectionLedgerCount": {
                "base": 1577,
                "delta": 54,
                "historicalDerived": 1631,
            },
            "reviewedCoreProjectionCount": {
                "base": 660,
                "delta": 26,
                "historicalDerived": 686,
            },
            "reviewedSourceOnlyOccurrenceCount": {
                "base": 197,
                "delta": 9,
                "historicalDerived": 206,
            },
            "sourceElementCount": {
                "base": 657,
                "delta": 26,
                "historicalDerived": 683,
            },
            "clipGroupCount": {
                "base": 44,
                "delta": 2,
                "historicalDerived": 46,
            },
        },
        "liveProductUnion": {
            "valtanCatalogRows": {
                "successorBase": 54,
                "additiveDelta": 2,
                "final": 56,
                "successorBaseCanonicalSha256": canonical_sha256(
                    base_valtan_projection
                ),
                "finalCanonicalSha256": canonical_sha256(
                    final_valtan_projection
                ),
            },
            "bossRootCueRows": {
                "successorBase": 47,
                "additiveDelta": 2,
                "final": 49,
                "successorBaseCanonicalSha256": canonical_sha256(
                    base_cues["cues"]
                ),
                "finalCanonicalSha256": canonical_sha256(
                    target_cues["cues"]
                ),
            },
        },
        "decalAppend": {
            "clipOccurrenceIds": sorted(groups),
            "patternIds": ["VALTAN_FOUR_PILLARS_105"],
            "exactProjectionCount": 2,
        },
        "successorMappings": successor_mappings,
        "catalogRows": additive_catalog_rows,
        "cueRows": additive_cue_rows,
        "targetDocuments": additive_documents,
    }
    existing_append = (
        read_json(ADDITIVE_RECEIPT_PATH)
        if ADDITIVE_RECEIPT_PATH.is_file()
        else None
    )
    if existing_append is not None and existing_append != expected_append:
        prior_additive = copy.deepcopy(expected_append)
        prior_additive["formatVersion"] = 1
        prior_additive.pop("successorBaseInputProjection", None)
        if existing_append != prior_additive:
            raise MaterializeError("100-bar additive receipt proof drifted")
    receipt_view = copy.deepcopy(receipt)
    receipt_view["additiveReviewedOwnerAppend"] = expected_append

    writes: dict[Path, bytes] = {
        CATALOG_PATH: pretty_bytes(target_catalog),
        CUE_PATH: pretty_bytes(target_cues),
        ADDITIVE_RECEIPT_PATH: pretty_bytes(expected_append),
    }
    for effect_id, document in target_documents.items():
        path = target_document_paths[effect_id]
        payload = pretty_bytes(document)
        if not path.is_file() or path.read_bytes() != payload:
            writes[path] = payload
    return state, writes, receipt_view


def build_outputs() -> tuple[str, dict[Path, bytes], dict[str, Any]]:
    successor_receipt = _load_successor_receipt()
    if not RECEIPT_PATH.is_file():
        raise MaterializeError("immutable Carrier V1 receipt is missing")
    historical_receipt = read_json(RECEIPT_PATH)
    inventory = _load_inventory(successor_receipt)
    core, materialized, blockers = _build_projections(inventory)
    if inventory.get("_materializerAdditiveReviewedBranchIds"):
        return _build_additive_outputs(
            core, materialized, blockers, successor_receipt
        )
    legacy_inventory, legacy_authorizations = _load_legacy_authorizations()
    all_exact = {row["sourceNode"]: row for row in core}
    materialized_exact = {row["sourceNode"]: row for row in materialized}

    catalog_source = read_json(CATALOG_PATH)
    cues_source = read_json(CUE_PATH)
    try:
        historical_catalog_projection = (
            successor_lineage.project_historical_catalog(
                catalog_source, successor_receipt
            )
        )
        historical_cue_projection = successor_lineage.project_historical_cues(
            cues_source, successor_receipt
        )
        successor_entries = successor_lineage.successor_documents(
            successor_receipt
        )
    except successor_lineage.SuccessorLineageError as error:
        raise MaterializeError(str(error)) from error
    catalog_rows = _catalog_index(catalog_source)
    cue_rows = _cue_index(cues_source)
    historical_catalog_rows = _catalog_index(
        {
            "formatVersion": historical_catalog_projection["formatVersion"],
            "effects": historical_catalog_projection["effects"],
        }
    )
    historical_cue_rows = _cue_index(historical_cue_projection)
    live_only_catalog_ids = {
        str(row["effectAssetId"])
        for row in successor_lineage.live_only_catalog_rows(
            successor_receipt
        )
    }
    historical_target_rows = {
        str(row.get("effectAssetId") or ""): row
        for row in (historical_receipt.get("outputs") or {}).get(
            "targetDocuments", []
        )
    }
    successor_target_ids = set(successor_entries).intersection(
        historical_target_rows
    )
    external_owners = _external_effect_owners()
    if external_owners != {
        RED_BLADE_EFFECT_ID,
        "effect.valtan.sky-axe.active",
    }:
        raise MaterializeError("Valtan external Effect owner allowlist drifted")

    documents: dict[str, dict[str, Any]] = {}
    document_paths: dict[str, Path] = {}
    existing_exact: dict[str, dict[str, Any]] = {}
    existing_exact_owner: dict[str, str] = {}
    authorized_legacy_hits = 0
    valtan_product_row_count = 0
    document_audit: dict[str, dict[str, int]] = {}

    for effect_id, catalog_row in sorted(catalog_rows.items()):
        if not effect_id.startswith("effect.valtan."):
            continue
        if effect_id in live_only_catalog_ids:
            continue
        path = authored_path(catalog_row)
        document = read_json(path)
        _validate_document(document, effect_id)
        document_paths[effect_id] = path
        documents[effect_id] = copy.deepcopy(document)
        valtan_product_row_count += len(document["elements"])
        counts = {"legacy": 0, "exact": 0, "other": 0}
        if effect_id in successor_entries:
            document_audit[effect_id] = counts
            continue
        for element_index, element in enumerate(document["elements"]):
            source_node = str(element.get("sourceNode") or "")
            authorization = legacy_authorizations.get((effect_id, element_index))
            if authorization is not None and (
                legacy_inventory_builder.canonical_sha256(element)
                == str(authorization.get("legacyElementSha256") or "")
            ):
                _validate_legacy_preimage_row(authorization, element)
                authorized_legacy_hits += 1
                counts["legacy"] += 1
                continue
            if source_node in all_exact:
                if source_node in existing_exact:
                    raise MaterializeError(
                        f"exact source row is duplicated across Product: {source_node}"
                    )
                existing_exact[source_node] = copy.deepcopy(element)
                existing_exact_owner[source_node] = effect_id
                counts["exact"] += 1
                continue
            if source_node.startswith("valtan.source."):
                raise MaterializeError(
                    f"unknown compact Valtan source row is not removable: {source_node}"
                )
            counts["other"] += 1
        document_audit[effect_id] = counts

    valtan_effect_ids = set(historical_catalog_rows)
    protected_document = documents.get(PROTECTED_EFFECT_ID)
    sky_effect_id = "effect.valtan.sky-axe.active"
    sky_document = documents.get(sky_effect_id)
    red_document = documents.get(RED_BLADE_EFFECT_ID)
    if protected_document is None or sky_document is None or red_document is None:
        raise MaterializeError("Valtan explicit Product allowlist document is missing")
    protected_proof = _protected_whirlwind_alias_proof(core, protected_document)
    target_protected_document = _reseal_protected_whirlwind_scale(
        protected_document, protected_proof
    )
    _validate_document(target_protected_document, PROTECTED_EFFECT_ID)
    if sky_effect_id not in successor_entries and len(
        sky_document["elements"]
    ) != 3:
        raise MaterializeError("Sky Axe external-owner row count drifted")

    groups: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in materialized:
        groups[row["clipOccurrenceId"]].append(row)
    target_effect_ids = {
        str(row["targetEffectAssetId"]) for row in materialized
    }
    final_valtan_effect_ids = target_effect_ids | {
        PROTECTED_EFFECT_ID,
        sky_effect_id,
    }
    target_binding_ids = {
        _target_cue_id(clip_id)
        for clip_id in groups
        if clip_id != RED_BLADE_CLIP_ID
    }
    protected_cues = [
        row
        for row in cue_rows.values()
        if row.get("effectAssetId") == PROTECTED_EFFECT_ID
    ]
    if len(protected_cues) != 1 or protected_cues[0].get(
        "clipOccurrenceId"
    ) != PROTECTED_CLIP_ID:
        raise MaterializeError("protected Whirlwind Product cue drifted")
    final_binding_ids = target_binding_ids | {
        str(protected_cues[0]["bindingId"])
    }

    successor_baseline_count = sum(
        int(historical_target_rows[effect_id]["elementCount"])
        for effect_id in successor_target_ids
    )
    expected_managed_exact_count = (
        EXPECTED["materializedProjectionCount"] - successor_baseline_count
    )
    exact_count = len(existing_exact)
    if authorized_legacy_hits == EXPECTED["baselineStrictLegacyRowCount"]:
        state = "BASELINE"
        if (
            exact_count != EXPECTED["baselineExistingExactProjectionCount"]
            or len(valtan_effect_ids) != EXPECTED["baselineValtanCatalogCount"]
            or len(cue_rows) != EXPECTED["baselineBossRootCueCount"]
            or valtan_product_row_count
            != EXPECTED["baselineValtanProductRowCount"]
        ):
            raise MaterializeError(
                "carrier V1 baseline exact/legacy denominator drifted"
            )
        _verify_baseline_legacy_inventory_bytes(legacy_inventory)
    elif authorized_legacy_hits == 0:
        state = "APPLIED"
        if (
            exact_count != expected_managed_exact_count
            or valtan_effect_ids != final_valtan_effect_ids
            or set(historical_cue_rows) != final_binding_ids
        ):
            raise MaterializeError(
                "carrier V1 repository is partial after boss-root reset"
            )
        for effect_id, document in documents.items():
            if effect_id in successor_entries:
                continue
            if effect_id == PROTECTED_EFFECT_ID:
                if document != target_protected_document:
                    raise MaterializeError(
                        "protected Whirlwind WModel scale seal drifted"
                    )
                continue
            if effect_id == sky_effect_id:
                continue
            expected_nodes = {
                source_node
                for source_node, row in materialized_exact.items()
                if row["targetEffectAssetId"] == effect_id
            }
            actual_nodes = {
                str(row.get("sourceNode") or "")
                for row in document["elements"]
            }
            if (
                not expected_nodes
                or actual_nodes != expected_nodes
                or len(actual_nodes) != len(document["elements"])
            ):
                raise MaterializeError(
                    f"applied Valtan Product owner has a nonexact row: {effect_id}"
                )
    else:
        raise MaterializeError(
            "carrier V1 repository is partial: sealed legacy preimage hit "
            f"count is {authorized_legacy_hits}"
        )

    target_documents: dict[str, dict[str, Any]] = {}
    for clip_id in sorted(groups):
        group = sorted(groups[clip_id], key=lambda row: row["sourceNode"])
        effect_id = str(group[0]["targetEffectAssetId"])
        if any(row["targetEffectAssetId"] != effect_id for row in group):
            raise MaterializeError("one clip group targets multiple Effects")
        if effect_id in successor_target_ids:
            document = copy.deepcopy(documents[effect_id])
            target_documents[effect_id] = document
            continue
        if effect_id == RED_BLADE_EFFECT_ID:
            document = copy.deepcopy(red_document)
            document["elements"] = []
            document_paths[effect_id] = authored_path(catalog_rows[effect_id])
        elif state == "APPLIED":
            document = copy.deepcopy(documents[effect_id])
            document["elements"] = []
        else:
            document = _new_document(effect_id, group)
            document_paths[effect_id] = AUTHORED_ROOT / f"{effect_id}.effect.json"
        migrated = []
        for row in group:
            exact = _common_translucent_element(
                row["element"], existing_exact.get(row["sourceNode"])
            )
            migrated.append(exact)
        document["elements"] = sorted(
            migrated,
            key=lambda row: (str(row.get("sourceNode") or ""), str(row.get("id") or "")),
        )
        _validate_document(document, effect_id)
        target_documents[effect_id] = document

    if len(target_documents) != EXPECTED["materializedClipGroupCount"]:
        raise MaterializeError("materialized target document count drifted")
    managed_target_count = sum(
        len(document["elements"])
        for effect_id, document in target_documents.items()
        if effect_id not in successor_target_ids
    )
    if managed_target_count + successor_baseline_count != EXPECTED[
        "materializedProjectionCount"
    ]:
        raise MaterializeError("target exact carrier row count drifted")
    for effect_id, document in {
        **target_documents,
        PROTECTED_EFFECT_ID: target_protected_document,
    }.items():
        if effect_id in successor_target_ids:
            continue
        protected_mesh_ids = {
            str(row["protectedElementId"])
            for row in protected_proof
            if row["rendererShape"] == "mesh"
        }
        for element in document["elements"]:
            mesh_models = [
                row
                for row in element.get("resources", [])
                if isinstance(row, dict) and row.get("slotId") == "meshModel"
            ]
            if not mesh_models:
                continue
            if (
                effect_id == PROTECTED_EFFECT_ID
                and str(element.get("id") or "") not in protected_mesh_ids
            ):
                continue
            if (
                len(mesh_models) != 1
                or (element.get("detail") or {}).get("mesh", {}).get(
                    "modelPreScale"
                )
                != 0.01
            ):
                raise MaterializeError(
                    f"exact Valtan WModel scale ABI drifted: {effect_id}"
                )
    final_owner_count = (
        sum(len(row["elements"]) for row in target_documents.values())
        + len(target_protected_document["elements"])
        + len(sky_document["elements"])
    )
    expected_final_owner_count = int(
        (successor_receipt.get("summary") or {}).get(
            "historicalOwnerFinalElementCount", -1
        )
    )
    if final_owner_count != expected_final_owner_count:
        raise MaterializeError("final Valtan historical-owner row closure drifted")

    # In the applied state, exact rows may only live in their final owners.
    if state == "APPLIED":
        for source_node, owner in existing_exact_owner.items():
            expected_owner = materialized_exact[source_node]["targetEffectAssetId"]
            if owner != expected_owner:
                raise MaterializeError(
                    f"applied exact source row has wrong owner: {source_node}"
                )

    target_catalog_rows = {
        effect_id: copy.deepcopy(row)
        for effect_id, row in catalog_rows.items()
        if not effect_id.startswith("effect.valtan.")
        or effect_id in live_only_catalog_ids
    }
    target_catalog_rows[PROTECTED_EFFECT_ID] = copy.deepcopy(
        catalog_rows[PROTECTED_EFFECT_ID]
    )
    target_catalog_rows[sky_effect_id] = copy.deepcopy(catalog_rows[sky_effect_id])
    for effect_id in target_effect_ids:
        expected = _catalog_row(effect_id)
        current = catalog_rows.get(effect_id)
        if current is not None and current != expected:
            raise MaterializeError(
                f"carrier V1 target catalog row is rebound: {effect_id}"
            )
        target_catalog_rows[effect_id] = expected
    target_catalog = copy.deepcopy(catalog_source)
    target_catalog["effects"] = sorted(
        target_catalog_rows.values(), key=lambda row: str(row["effectAssetId"])
    )
    target_catalog_projection = _valtan_catalog_receipt_projection(
        target_catalog, successor_receipt
    )
    if len(target_catalog_projection["effects"]) != EXPECTED[
        "finalValtanCatalogCount"
    ]:
        raise MaterializeError("final Valtan Product catalog closure drifted")

    target_cue_rows = {
        str(protected_cues[0]["bindingId"]): copy.deepcopy(protected_cues[0])
    }
    for clip_id in sorted(groups):
        if clip_id == RED_BLADE_CLIP_ID:
            continue
        generated_row = _cue_row(groups[clip_id])
        binding_id = str(generated_row["bindingId"])
        row = historical_cue_rows.get(binding_id)
        if row is None:
            raise MaterializeError(
                f"Carrier V1 historical cue is missing: {binding_id}"
            )
        owner_projected = copy.deepcopy(generated_row)
        for field in ("patternId", "stageId", "actionId"):
            owner_projected[field] = copy.deepcopy(row.get(field))
        if owner_projected != row:
            raise MaterializeError(
                f"Carrier V1 historical cue contract drifted: {binding_id}"
            )
        row = copy.deepcopy(row)
        current = target_cue_rows.get(binding_id)
        if current is not None and current != row:
            raise MaterializeError(f"carrier V1 cue is rebound: {binding_id}")
        target_cue_rows[binding_id] = row
    if any(
        row.get("effectAssetId") == RED_BLADE_EFFECT_ID
        for row in target_cue_rows.values()
    ):
        raise MaterializeError("Red Blade combat object regained a boss-root cue")
    historical_target_cues = copy.deepcopy(cues_source)
    historical_target_cues["cues"] = sorted(
        target_cue_rows.values(), key=lambda row: str(row["bindingId"])
    )
    _cue_index(historical_target_cues)
    if len(historical_target_cues["cues"]) != EXPECTED[
        "finalBossRootCueCount"
    ]:
        raise MaterializeError("final Valtan boss-root cue closure drifted")
    if state == "APPLIED":
        live_historical_cues = _cue_receipt_projection(
            cues_source, successor_receipt
        )
        if live_historical_cues != historical_target_cues:
            live_ids = _cue_index(live_historical_cues)
            target_ids = _cue_index(historical_target_cues)
            mismatch = next(
                (
                    binding_id
                    for binding_id in sorted(set(live_ids) | set(target_ids))
                    if live_ids.get(binding_id) != target_ids.get(binding_id)
                ),
                "document-header-or-order",
            )
            raise MaterializeError(
                "live Valtan cue successor does not project to Carrier V1: "
                + mismatch
            )
        target_cues = copy.deepcopy(cues_source)
    else:
        target_cues = historical_target_cues
    cue_clip_owners = Counter(
        str(row["clipOccurrenceId"])
        for row in historical_target_cues["cues"]
    )
    if (
        any(
            cue_clip_owners[clip_id] != 1
            for clip_id in groups
            if clip_id != RED_BLADE_CLIP_ID
        )
        or cue_clip_owners[RED_BLADE_CLIP_ID] != 0
        or cue_clip_owners[PROTECTED_CLIP_ID] != 1
    ):
        raise MaterializeError("one clip occurrence has duplicate boss-root owners")

    applied_receipt: dict[str, Any] | None = None
    if state == "BASELINE":
        retired_effect_ids = sorted(
            valtan_effect_ids
            - {PROTECTED_EFFECT_ID, RED_BLADE_EFFECT_ID, sky_effect_id}
        )
        retired_cue_ids = sorted(
            binding_id
            for binding_id, row in cue_rows.items()
            if row.get("effectAssetId") != PROTECTED_EFFECT_ID
        )
        migration_rows = []
        for effect_id in retired_effect_ids:
            counts = document_audit[effect_id]
            path = document_paths[effect_id]
            migration_rows.append(
                {
                    "effectAssetId": effect_id,
                    "path": repository_path(path),
                    "beforeElementCount": len(documents[effect_id]["elements"]),
                    "strictLegacyRemovedCount": counts["legacy"],
                    "exactSourceMovedCount": counts["exact"],
                    "nonExactExistingRemovedCount": counts["other"],
                    "preimageCanonicalSha256": canonical_sha256(
                        documents[effect_id]
                    ),
                    "preimageByteSha256": byte_sha256(path.read_bytes()),
                    "physicalPreimageDisposition": (
                        "ELEMENTS_CLEARED_EVIDENCE_SHELL_GIT_HISTORY_OWNS_PREIMAGE"
                    ),
                }
            )
        red_counts = document_audit[RED_BLADE_EFFECT_ID]
        migration_rows.append(
            {
                "effectAssetId": RED_BLADE_EFFECT_ID,
                "path": repository_path(document_paths[RED_BLADE_EFFECT_ID]),
                "beforeElementCount": len(red_document["elements"]),
                "strictLegacyRemovedCount": red_counts["legacy"],
                "exactSourceMovedCount": red_counts["exact"],
                "nonExactExistingRemovedCount": red_counts["other"],
                "preimageCanonicalSha256": canonical_sha256(red_document),
                "preimageByteSha256": byte_sha256(
                    document_paths[RED_BLADE_EFFECT_ID].read_bytes()
                ),
                "physicalPreimageDisposition": (
                    "REWRITTEN_EXACT_COMBAT_OBJECT_OWNER"
                ),
            }
        )
        removed_existing_rows = sum(
            row["beforeElementCount"] for row in migration_rows
        )
        if (
            len(retired_effect_ids) != EXPECTED["baselineRetiredEffectCount"]
            or len(retired_cue_ids) != EXPECTED["baselineRetiredCueCount"]
            or removed_existing_rows
            != EXPECTED["baselineRemovedExistingRowCount"]
        ):
            raise MaterializeError("baseline Product reset denominator drifted")
    else:
        if not RECEIPT_PATH.is_file():
            raise MaterializeError("applied carrier V1 receipt is missing")
        applied_receipt = read_json(RECEIPT_PATH)
        legacy_migration = applied_receipt.get("legacyMigration") or {}
        retired_effect_ids = list(
            legacy_migration.get("retiredEffectAssetIds") or []
        )
        retired_cue_ids = list(
            legacy_migration.get("retiredCueBindingIds") or []
        )
        migration_rows = copy.deepcopy(legacy_migration.get("documents") or [])
        removed_existing_rows = int(
            (applied_receipt.get("productReset") or {}).get(
                "removedExistingRowCount", -1
            )
        )
        if (
            len(retired_effect_ids) != EXPECTED["baselineRetiredEffectCount"]
            or len(retired_cue_ids) != EXPECTED["baselineRetiredCueCount"]
            or removed_existing_rows
            != EXPECTED["baselineRemovedExistingRowCount"]
        ):
            raise MaterializeError("applied retirement receipt denominator drifted")
        migration_by_effect = {
            str(row.get("effectAssetId") or ""): row for row in migration_rows
        }
        for effect_id in retired_effect_ids:
            row = migration_by_effect.get(effect_id)
            path = ROOT / str((row or {}).get("path") or "")
            if row is None or not path.is_file():
                raise MaterializeError(
                    f"retired Valtan evidence shell is missing: {effect_id}"
                )
            shell = read_json(path)
            _validate_document(shell, effect_id)
            if shell["elements"] != []:
                raise MaterializeError(
                    f"retired Valtan document regained rows: {effect_id}"
                )

    writes: dict[Path, bytes] = {
        CATALOG_PATH: pretty_bytes(target_catalog),
        CUE_PATH: pretty_bytes(target_cues),
    }
    for effect_id, document in target_documents.items():
        if effect_id in successor_target_ids:
            continue
        path = document_paths[effect_id]
        payload = pretty_bytes(document)
        if not path.is_file() or path.read_bytes() != payload:
            writes[path] = payload
    protected_path = document_paths[PROTECTED_EFFECT_ID]
    protected_payload = pretty_bytes(target_protected_document)
    if protected_path.read_bytes() != protected_payload:
        writes[protected_path] = protected_payload
    if state == "BASELINE":
        for effect_id in retired_effect_ids:
            shell = copy.deepcopy(documents[effect_id])
            shell["elements"] = []
            _validate_document(shell, effect_id)
            writes[document_paths[effect_id]] = pretty_bytes(shell)

    if state == "BASELINE":
        receipt = _build_receipt(
            core=core,
            materialized=materialized,
            blockers=blockers,
            migration_rows=sorted(
                migration_rows, key=lambda row: row["effectAssetId"]
            ),
            retired_effect_ids=retired_effect_ids,
            retired_cue_ids=retired_cue_ids,
            target_documents=target_documents,
            target_protected_document=target_protected_document,
            sky_document=sky_document,
            target_catalog=target_catalog,
            target_cues=target_cues,
            successor_receipt=successor_receipt,
        )
        receipt["protectedWhirlwindExactAliasProof"] = protected_proof
        successor_mappings = []
        for binding_id in retired_cue_ids:
            old = cue_rows[binding_id]
            clip_id = str(old.get("clipOccurrenceId") or "")
            successor_group = groups.get(clip_id)
            successor_effect = (
                str(successor_group[0]["targetEffectAssetId"])
                if successor_group
                else None
            )
            successor_cue = (
                _target_cue_id(clip_id)
                if successor_group and clip_id != RED_BLADE_CLIP_ID
                else None
            )
            successor_mappings.append(
                {
                    "retiredBindingId": binding_id,
                    "retiredEffectAssetId": str(old["effectAssetId"]),
                    "patternId": str(old["patternId"]),
                    "stageId": str(old["stageId"]),
                    "actionId": str(old["actionId"]),
                    "clipOccurrenceId": clip_id,
                    "disposition": (
                        "REPLACED_BY_EXACT_CARRIER_V1_CLIP_OWNER"
                        if successor_cue is not None
                        else "RETIRED_NO_EXACT_REVIEWED_CARRIER_OWNER"
                    ),
                    "replacementBindingId": successor_cue,
                    "replacementEffectAssetId": successor_effect,
                }
            )
        receipt["retiredOwnerSuccessorMappings"] = successor_mappings
        receipt["productReset"] = {
            "removedExistingRowCount": removed_existing_rows,
            "retiredBossRootEffectCount": len(retired_effect_ids),
            "retiredBossRootCueCount": len(retired_cue_ids),
            "finalValtanCatalogCount": EXPECTED["finalValtanCatalogCount"],
            "finalBossRootCueCount": EXPECTED["finalBossRootCueCount"],
            "nonExactOldBossRootSurvivorCount": 0,
            "duplicateClipOccurrenceOwnerCount": 0,
            "explicitProductAllowlist": [
                {
                    "effectAssetId": PROTECTED_EFFECT_ID,
                    "policy": "ROW_PRESERVED_9_ROW_CANARY_2_EXACT_WMODEL_ROWS_RESEALED_0_01",
                },
                {
                    "effectAssetId": RED_BLADE_EFFECT_ID,
                    "policy": "COMBAT_OBJECT_OWNER_REWRITTEN_TO_5_EXACT_ROWS",
                },
                {
                    "effectAssetId": sky_effect_id,
                    "policy": "BLOCKED_EXTERNAL_OWNER_3_ROW_HASHED_EXCEPTION",
                },
                {
                    "effectAssetIdPrefix": "effect.valtan.carrier-v1.",
                    "policy": "EXACT_REVIEWED_CLIP_OWNER_ONLY",
                },
            ],
        }
        receipt["legacyMigration"]["sealedInventory"] = {
            "path": repository_path(LEGACY_INVENTORY_PATH),
            "canonicalSha256": legacy_inventory_builder.canonical_sha256(
                legacy_inventory
            ),
            "authorizedRowCount": len(legacy_authorizations),
            "requiredJoinStatus": "EXACT_MATERIAL_ORDINAL_JOIN",
        }
    else:
        assert applied_receipt is not None
        receipt = applied_receipt
        _validate_existing_receipt(
            receipt,
            target_documents,
            target_protected_document,
            sky_document,
            blockers,
            target_catalog,
            target_cues,
            successor_receipt,
        )
    try:
        successor_lineage.validate_receipt(
            root=ROOT,
            receipt=successor_receipt,
            historical_receipt=historical_receipt,
            catalog=target_catalog,
            cues=target_cues,
        )
    except successor_lineage.SuccessorLineageError as error:
        raise MaterializeError(str(error)) from error
    if state == "BASELINE":
        writes[RECEIPT_PATH] = pretty_bytes(receipt)
    return state, writes, receipt


def _atomic_replace(
    writes: dict[Path, bytes],
    replace: Callable[[str, str], None] = os.replace,
) -> None:
    if not writes:
        return
    ordered = sorted(writes.items(), key=lambda row: str(row[0]))
    before = {
        path: path.read_bytes() if path.is_file() else None for path, _ in ordered
    }
    staged: dict[Path, Path] = {}
    committed: list[Path] = []
    try:
        for ordinal, (path, payload) in enumerate(ordered):
            path.parent.mkdir(parents=True, exist_ok=True)
            temporary = path.with_name(
                f"{path.name}.carrier-v1.{os.getpid()}.{ordinal}.staging"
            )
            temporary.write_bytes(payload)
            staged[path] = temporary
        for path, _ in ordered:
            replace(str(staged[path]), str(path))
            committed.append(path)
    except Exception as error:
        rollback_errors = []
        for ordinal, path in enumerate(reversed(committed)):
            try:
                previous = before[path]
                if previous is None:
                    path.unlink(missing_ok=True)
                else:
                    recovery = path.with_name(
                        f"{path.name}.carrier-v1.rollback.{os.getpid()}.{ordinal}"
                    )
                    recovery.write_bytes(previous)
                    os.replace(str(recovery), str(path))
            except Exception as rollback_error:  # pragma: no cover - catastrophic I/O
                rollback_errors.append(f"{path}: {rollback_error}")
        suffix = (
            "; rollback errors: " + "; ".join(rollback_errors)
            if rollback_errors
            else ""
        )
        raise MaterializeError(
            f"carrier V1 atomic transaction failed: {error}{suffix}"
        ) from error
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def _changed_outputs(writes: dict[Path, bytes]) -> dict[Path, bytes]:
    def differs(path: Path, payload: bytes) -> bool:
        if not path.is_file():
            return True
        current = path.read_bytes()
        return current != payload and current.replace(b"\r\n", b"\n") != (
            payload.replace(b"\r\n", b"\n")
        )

    return {
        path: payload
        for path, payload in writes.items()
        if differs(path, payload)
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=("check", "write"), default="check")
    args = parser.parse_args(argv)
    state, writes, receipt = build_outputs()
    changed = _changed_outputs(writes)
    if args.mode == "check":
        if changed:
            paths = "\n  ".join(repository_path(path) for path in sorted(changed))
            raise MaterializeError(
                "carrier V1 outputs are stale; run --mode write:\n  " + paths
            )
    else:
        _atomic_replace(changed)
    summary = receipt["summary"]
    additive = receipt.get("additiveReviewedOwnerAppend")
    if isinstance(additive, dict):
        delta = additive["deltaEvidence"]
        count_suffix = (
            f"historicalExact={summary['reviewedCoreProjectionCount']} "
            f"appendExact={delta['reviewedProjectionLedger']['executableCoreCount']} "
            f"historicalMaterialized={summary['materializedProjectionCount']} "
            f"appendMaterialized={delta['sourceElements']['count']} "
            f"appendOwners={len(additive['targetDocuments'])} "
        )
    else:
        count_suffix = (
            f"exact={summary['reviewedCoreProjectionCount']} "
            f"materialized={summary['materializedProjectionCount']} "
        )
    print(
        "Valtan carrier V1 "
        f"{args.mode}: state={state} changed={len(changed)} "
        f"{count_suffix}"
        f"legacyRetired={summary['baselineStrictLegacyRowCount']}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except MaterializeError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
