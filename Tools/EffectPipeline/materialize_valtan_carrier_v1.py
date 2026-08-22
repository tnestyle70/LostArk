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


CATALOG_PATH = ROOT / "Data/Effects/EffectCatalog.json"
CUE_PATH = ROOT / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
BOSS_CATALOG_PATH = ROOT / "Data/Actors/BossCatalog.json"
AUTHORED_ROOT = ROOT / "Data/Effects/Authored"
RECEIPT_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/CarrierV1"
    / "Valtan.carrier-v1-materialization-receipt.v1.json"
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
) -> dict[str, Any]:
    """Seal only the catalog contract owned by this Valtan materializer."""

    try:
        return source_inventory.effect_catalog_prefix_projection(
            catalog, "effect.valtan."
        )
    except source_inventory.InventoryError as error:
        raise MaterializeError(str(error)) from error


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


def _load_inventory() -> dict[str, Any]:
    selection = source_inventory.load_selection_manifest(
        reviewed_candidates.SELECTION_PATH
    )
    inventory = source_inventory.build_inventory(
        previous={
            "reviewedBranchSelections": copy.deepcopy(
                selection["selections"]
            )
        },
        include_payloads=True,
    )
    source_inventory.validate_inventory(inventory)
    return inventory


def _build_projections(
    inventory: dict[str, Any],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], dict[str, Any]]:
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
        if occurrence.get("reachabilityDisposition") != "REACHABLE_REVIEWED":
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
            if carrier.get("elementSeed") is None:
                raise MaterializeError("executable carrier has no elementSeed")
            full_key = (
                f"{occurrence['fullKey']}|{carrier['carrierKey']}"
            )
            element = source_inventory.occurrence_element_seed(
                occurrence, carrier
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
        if actual[key] != EXPECTED[key]:
            raise MaterializeError(
                f"reviewed carrier denominator drifted: {key}: "
                f"expected {EXPECTED[key]}, actual {actual[key]}"
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
) -> dict[str, Any]:
    valtan_catalog_projection = _valtan_catalog_receipt_projection(
        target_catalog
    )
    catalog_by_effect = _catalog_index(target_catalog)
    cues_by_id = _cue_index(target_cues)
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
                "cueCount": len(target_cues["cues"]),
                "canonicalSha256": canonical_sha256(target_cues),
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
        target_catalog
    )
    if (
        catalog_output.get("scope") != "EFFECT_ASSET_ID_PREFIX"
        or catalog_output.get("effectAssetIdPrefix") != "effect.valtan."
        or catalog_output.get("effectCount")
        != len(valtan_catalog_projection["effects"])
        or catalog_output.get("canonicalSha256")
        != canonical_sha256(valtan_catalog_projection)
        or (outputs.get("cues") or {}).get("canonicalSha256")
        != canonical_sha256(target_cues)
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
    if expected_docs != actual_docs:
        raise MaterializeError("applied carrier V1 target documents drifted")
    if (
        canonical_sha256(receipt.get("reviewedProjectionLedger") or [])
        != canonical_sha256(blockers["reviewedProjectionLedger"])
        or canonical_sha256(
            receipt.get("reviewedSourceOnlyOccurrences") or []
        )
        != canonical_sha256(blockers["reviewedSourceOnlyOccurrences"])
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
        for row in _cue_index(target_cues).values()
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
        or sky.get("documentCanonicalSha256") != canonical_sha256(sky_document)
        or sky.get("documentByteSha256") != byte_sha256(sky_path.read_bytes())
        or sky.get("catalogRowCanonicalSha256")
        != canonical_sha256(catalog_by_effect["effect.valtan.sky-axe.active"])
    ):
        raise MaterializeError("applied Sky Axe exception drifted")


def build_outputs() -> tuple[str, dict[Path, bytes], dict[str, Any]]:
    inventory = _load_inventory()
    core, materialized, blockers = _build_projections(inventory)
    legacy_inventory, legacy_authorizations = _load_legacy_authorizations()
    all_exact = {row["sourceNode"]: row for row in core}
    materialized_exact = {row["sourceNode"]: row for row in materialized}

    catalog_source = read_json(CATALOG_PATH)
    cues_source = read_json(CUE_PATH)
    catalog_rows = _catalog_index(catalog_source)
    cue_rows = _cue_index(cues_source)
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
        path = authored_path(catalog_row)
        document = read_json(path)
        _validate_document(document, effect_id)
        document_paths[effect_id] = path
        documents[effect_id] = copy.deepcopy(document)
        valtan_product_row_count += len(document["elements"])
        counts = {"legacy": 0, "exact": 0, "other": 0}
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

    valtan_effect_ids = {
        effect_id
        for effect_id in catalog_rows
        if effect_id.startswith("effect.valtan.")
    }
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
    if len(sky_document["elements"]) != 3:
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
            exact_count != EXPECTED["materializedProjectionCount"]
            or valtan_effect_ids != final_valtan_effect_ids
            or set(cue_rows) != final_binding_ids
        ):
            raise MaterializeError(
                "carrier V1 repository is partial after boss-root reset"
            )
        for effect_id, document in documents.items():
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
    if sum(len(row["elements"]) for row in target_documents.values()) != EXPECTED[
        "materializedProjectionCount"
    ]:
        raise MaterializeError("target exact carrier row count drifted")
    for effect_id, document in {
        **target_documents,
        PROTECTED_EFFECT_ID: target_protected_document,
    }.items():
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
    if (
        sum(len(row["elements"]) for row in target_documents.values())
        + len(target_protected_document["elements"])
        + len(sky_document["elements"])
        != 669
    ):
        raise MaterializeError("final Valtan Product row closure is not 669")

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
    if sum(
        row["effectAssetId"].startswith("effect.valtan.")
        for row in target_catalog["effects"]
    ) != EXPECTED["finalValtanCatalogCount"]:
        raise MaterializeError("final Valtan Product catalog closure drifted")

    target_cue_rows = {
        str(protected_cues[0]["bindingId"]): copy.deepcopy(protected_cues[0])
    }
    for clip_id in sorted(groups):
        if clip_id == RED_BLADE_CLIP_ID:
            continue
        row = _cue_row(groups[clip_id])
        binding_id = str(row["bindingId"])
        current = target_cue_rows.get(binding_id)
        if current is not None and current != row:
            raise MaterializeError(f"carrier V1 cue is rebound: {binding_id}")
        target_cue_rows[binding_id] = row
    if any(
        row.get("effectAssetId") == RED_BLADE_EFFECT_ID
        for row in target_cue_rows.values()
    ):
        raise MaterializeError("Red Blade combat object regained a boss-root cue")
    target_cues = copy.deepcopy(cues_source)
    target_cues["cues"] = sorted(
        target_cue_rows.values(), key=lambda row: str(row["bindingId"])
    )
    _cue_index(target_cues)
    if len(target_cues["cues"]) != EXPECTED["finalBossRootCueCount"]:
        raise MaterializeError("final Valtan boss-root cue closure drifted")
    cue_clip_owners = Counter(
        str(row["clipOccurrenceId"]) for row in target_cues["cues"]
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
        )
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
    return {
        path: payload
        for path, payload in writes.items()
        if not path.is_file() or path.read_bytes() != payload
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
    print(
        "Valtan carrier V1 "
        f"{args.mode}: state={state} changed={len(changed)} "
        f"exact={summary['reviewedCoreProjectionCount']} "
        f"materialized={summary['materializedProjectionCount']} "
        f"legacyRetired={summary['baselineStrictLegacyRowCount']}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except MaterializeError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
