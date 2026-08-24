#!/usr/bin/env python3
"""Seal the read-only Valtan legacy-V0 carrier migration denominator.

The retired V0 builder joined every ParticleSystem sharing a clip and then
copied a system mesh onto every emitter row.  Those rows are not source-exact
Product occurrences.  This inventory therefore does not mutate Effect
documents, cues, or the catalog.  It records three separate facts:

* the Product documents that still contain ``*.emNN``/empty-sourceNode rows,
* the exact carrier shapes in each referenced source ParticleSystem, and
* the clip-aggregate candidate expansion, explicitly blocked until a reviewed
  source occurrence selects a carrier full key.

Only ``source element -> exact carrier -> typed RT0 family`` rows may later be
materialized.  A legacy emitter ordinal is never treated as a carrier key.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
TOOLS_ROOT = ROOT / "Tools" / "EffectPipeline"
if str(TOOLS_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOLS_ROOT))

import build_valtan_source_occurrence_inventory as source_inventory  # noqa: E402


CUE_PATH = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
PATTERN_BINDINGS_PATH = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
)
BOSS_CATALOG_PATH = ROOT / "Data/Actors/BossCatalog.json"
COMBAT_OBJECTS_PATH = (
    ROOT / "Data/Encounters/Valtan/ValtanCombatObjects.json"
)
EFFECT_CATALOG_PATH = ROOT / "Data/Effects/EffectCatalog.json"
SOURCE_CATALOG_PATH = (
    ROOT / "Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json"
)
SOURCE_OCCURRENCE_INVENTORY_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/Valtan.source-occurrence-inventory.v1.json"
)
OUTPUT_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/CarrierV1"
    / "Valtan.legacy-v0-carrier-migration-inventory.v1.json"
)
MATERIALIZATION_RECEIPT_PATH = (
    ROOT
    / "Data/Effects/Imported/Valtan/CarrierV1"
    / "Valtan.carrier-v1-materialization-receipt.v1.json"
)
SCHEMA_PATH = (
    ROOT
    / "Tools/EffectPipeline/Schemas"
    / "lostark.valtan-legacy-v0-carrier-migration-inventory.schema.json"
)

SCHEMA_ID = "lostark.valtan-legacy-v0-carrier-migration-inventory"
DENOMINATOR_LABEL = "LEGACY_CLIP_AGGREGATE_CANDIDATE_DENOMINATOR"
BLOCKED_DISPOSITION = "BLOCKED_SOURCE_BRANCH"
IDENTITY_DRIFT_DISPOSITION = "BLOCKED_LEGACY_IDENTITY_DRIFT"
PRESERVED_DISPOSITION = "PRESERVE_EXISTING_NONLEGACY"
LEGACY_ID_PATTERN = re.compile(
    r"^(?P<system>[A-Za-z0-9_-]+)\.em(?P<ordinal>[0-9]+)$"
)

EXPECTED = {
    "productOwnerDocumentCount": 97,
    "bossPatternCueOwnerDocumentCount": 96,
    "combatObjectOwnerDocumentCount": 1,
    "productElementCount": 3343,
    "legacyAggregateElementCount": 3032,
    "preservedNonLegacyElementCount": 311,
    "pureLegacyDocumentCount": 54,
    "mixedLegacyDocumentCount": 43,
    "rawAggregateCarrierCandidateCount": 3403,
    "rawAggregateRendererShapeCounts": {
        "sprite": 2914,
        "mesh": 394,
        "decal": 89,
        "light": 6,
    },
    "uniqueSourceSystemCount": 115,
    "uniqueSourceCarrierCount": 934,
    "uniqueSourceCarrierRendererShapeCounts": {
        "sprite": 741,
        "mesh": 162,
        "decal": 29,
        "light": 2,
    },
    "unknownSourceSystemJoinCount": 0,
    "duplicateSourceSystemJoinCount": 0,
    "exactMaterialOrdinalJoinCount": 3032,
    "blockedLegacyIdentityDriftCount": 0,
}


class InventoryError(RuntimeError):
    """A source or denominator contract drifted."""


def read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8-sig") as handle:
        value = json.load(handle)
    if not isinstance(value, dict):
        raise InventoryError(f"JSON root is not an object: {path}")
    return value


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def repository_path(path: Path) -> str:
    return path.resolve().relative_to(ROOT.resolve()).as_posix()


def source_artifact(path: Path) -> dict[str, Any]:
    if not path.is_file():
        raise InventoryError(f"source artifact is missing: {path}")
    return {"path": repository_path(path), "sha256": sha256_file(path)}


def effect_document_path(catalog_entry: dict[str, Any]) -> Path:
    relative = str(catalog_entry.get("authoringPath") or "")
    if not relative or "\\" in relative or ":" in relative:
        raise InventoryError("Valtan Product catalog authoringPath is invalid")
    path = (ROOT / "Data" / Path(relative)).resolve()
    data_root = (ROOT / "Data").resolve()
    if data_root not in path.parents or not path.is_file():
        raise InventoryError(f"Valtan Product document is missing: {relative}")
    return path


def legacy_identity(element: dict[str, Any]) -> tuple[str, int] | None:
    if str(element.get("sourceNode") or "").strip():
        return None
    match = LEGACY_ID_PATTERN.fullmatch(str(element.get("id") or ""))
    if match is None:
        return None
    return match.group("system").casefold(), int(match.group("ordinal"))


def product_cue_fields(cue: dict[str, Any]) -> dict[str, Any]:
    required = (
        "bindingId",
        "occurrenceId",
        "patternId",
        "stageId",
        "actionId",
        "clipOccurrenceId",
        "effectAssetId",
    )
    if any(not str(cue.get(field) or "") for field in required):
        raise InventoryError(
            "Valtan Product cue lacks an occurrence identity: "
            + str(cue.get("bindingId") or cue.get("effectAssetId") or "")
        )
    return {field: str(cue[field]) for field in required}


def pattern_clip_index(
    pattern_bindings: dict[str, Any],
) -> dict[str, list[dict[str, Any]]]:
    rows = pattern_bindings.get("bindings")
    if not isinstance(rows, list):
        raise InventoryError("Valtan pattern bindings have no bindings")
    result: dict[str, list[dict[str, Any]]] = {}
    for row in rows:
        action_id = str(row.get("actionId") or "")
        clips = row.get("clips")
        if not action_id or action_id in result or not isinstance(clips, list):
            raise InventoryError(
                f"Valtan pattern binding is duplicate/invalid: {action_id}"
            )
        result[action_id] = clips
    return result


def resolve_owner_clip(
    clips_by_action: dict[str, list[dict[str, Any]]],
    action_id: str,
    clip_occurrence_id: str | None = None,
) -> dict[str, str]:
    candidates = clips_by_action.get(action_id, [])
    if clip_occurrence_id is not None:
        candidates = [
            row
            for row in candidates
            if str(row.get("clipOccurrenceId") or "") == clip_occurrence_id
        ]
    if len(candidates) != 1:
        raise InventoryError(
            "Product owner action does not select one clip occurrence: "
            f"{action_id} / {clip_occurrence_id} / matches={len(candidates)}"
        )
    row = candidates[0]
    occurrence_id = str(row.get("clipOccurrenceId") or "")
    clip = str(row.get("clip") or "")
    if not occurrence_id or not clip:
        raise InventoryError(f"Product owner clip is incomplete: {action_id}")
    return {"clipOccurrenceId": occurrence_id, "clip": clip}


def build_product_owners(
    cue_document: dict[str, Any],
    pattern_bindings: dict[str, Any],
    boss_catalog: dict[str, Any],
    combat_objects: dict[str, Any],
) -> list[dict[str, Any]]:
    cues = cue_document.get("cues")
    if not isinstance(cues, list):
        raise InventoryError("Valtan Product cue document has no cues")
    clips_by_action = pattern_clip_index(pattern_bindings)
    owners: list[dict[str, Any]] = []
    for cue in cues:
        cue_fields = product_cue_fields(cue)
        clip = resolve_owner_clip(
            clips_by_action,
            cue_fields["actionId"],
            cue_fields["clipOccurrenceId"],
        )
        owners.append(
            {
                "ownerKind": "BOSS_PATTERN_CUE",
                "ownerRoot": "BOSS_ROOT",
                "ownerId": cue_fields["bindingId"],
                "ownerPatternId": cue_fields["patternId"],
                "ownerStageId": cue_fields["stageId"],
                "ownerStageActionId": cue_fields["actionId"],
                "ownerClipOccurrenceId": clip["clipOccurrenceId"],
                "ownerClip": clip["clip"],
                "effectAssetId": cue_fields["effectAssetId"],
                "occurrenceId": cue_fields["occurrenceId"],
                "bindingId": cue_fields["bindingId"],
                "combatObjectArchetypeId": None,
                "clientVisualId": None,
            }
        )

    bosses = boss_catalog.get("bosses")
    objects = combat_objects.get("objects")
    if not isinstance(bosses, list) or not isinstance(objects, list):
        raise InventoryError("Valtan combat-object Product owner inputs are invalid")
    valtan_bosses = [
        row for row in bosses if row.get("archetypeId") == "BOSS_VALTAN"
    ]
    if len(valtan_bosses) != 1:
        raise InventoryError("BossCatalog does not contain exactly one BOSS_VALTAN")
    object_by_archetype: dict[str, dict[str, Any]] = {}
    for row in objects:
        archetype_id = str(row.get("combatObjectArchetypeId") or "")
        if not archetype_id or archetype_id in object_by_archetype:
            raise InventoryError(
                f"Valtan combat object is duplicate/invalid: {archetype_id}"
            )
        object_by_archetype[archetype_id] = row

    visuals = valtan_bosses[0].get("combatObjectVisuals")
    if not isinstance(visuals, list):
        raise InventoryError("BOSS_VALTAN has no combatObjectVisuals")
    for visual in visuals:
        archetype_id = str(visual.get("combatObjectArchetypeId") or "")
        client_visual_id = str(visual.get("clientVisualId") or "")
        effect_asset_id = str(visual.get("effectAssetId") or "")
        combat_object = object_by_archetype.get(archetype_id)
        if (
            combat_object is None
            or combat_object.get("clientVisualId") != client_visual_id
            or not effect_asset_id
        ):
            raise InventoryError(
                f"BossCatalog combat-object visual join failed: {archetype_id}"
            )
        action_id = str(combat_object.get("ownerStageActionId") or "")
        pattern_id = str(combat_object.get("ownerPatternId") or "")
        clip = resolve_owner_clip(clips_by_action, action_id)
        owners.append(
            {
                "ownerKind": "COMBAT_OBJECT",
                "ownerRoot": "WORLD_ROOT",
                "ownerId": client_visual_id,
                "ownerPatternId": pattern_id,
                "ownerStageId": action_id.rsplit(".", 1)[-1].upper(),
                "ownerStageActionId": action_id,
                "ownerClipOccurrenceId": clip["clipOccurrenceId"],
                "ownerClip": clip["clip"],
                "effectAssetId": effect_asset_id,
                "occurrenceId": None,
                "bindingId": None,
                "combatObjectArchetypeId": archetype_id,
                "clientVisualId": client_visual_id,
            }
        )

    owner_ids = [str(row["ownerId"]) for row in owners]
    if len(owner_ids) != len(set(owner_ids)):
        raise InventoryError("Valtan Product owner ids are duplicated")
    effect_ids = [str(row["effectAssetId"]) for row in owners]
    if len(effect_ids) != len(set(effect_ids)):
        raise InventoryError("Valtan Product effect owners are duplicated")
    return owners


def resolve_source_system(
    system_stem: str,
    systems_by_object_name: dict[str, list[dict[str, Any]]],
) -> tuple[dict[str, Any] | None, str]:
    candidates = systems_by_object_name.get(system_stem.casefold(), [])
    if not candidates:
        return None, "UNKNOWN_SOURCE_SYSTEM"
    if len(candidates) != 1:
        return None, "DUPLICATE_SOURCE_SYSTEM_JOIN"
    return candidates[0], "UNIQUE_SOURCE_SYSTEM_CANDIDATE_ONLY"


def material_ordinal_join(
    element: dict[str, Any],
    catalog_system: dict[str, Any],
    emitter_ordinal: int,
) -> dict[str, Any]:
    graph = catalog_system.get("graph")
    bindings = graph.get("resourceBindings") if isinstance(graph, dict) else None
    if not isinstance(bindings, list):
        raise InventoryError(
            "Valtan candidate source system has no graph resourceBindings: "
            + str(catalog_system.get("sourceAsset") or "")
        )
    material_bindings = [
        row
        for row in bindings
        if isinstance(row, dict) and row.get("role") == "material"
    ]
    material = element.get("material")
    legacy_source_material_path = (
        str(material.get("sourceMaterialPath") or "")
        if isinstance(material, dict)
        else ""
    )
    if emitter_ordinal < 0 or emitter_ordinal >= len(material_bindings):
        return {
            "legacySourceMaterialPath": legacy_source_material_path,
            "candidateMaterialObjectPath": None,
            "candidateMaterialBindingCount": len(material_bindings),
            "materialOrdinalJoinStatus": "MATERIAL_ORDINAL_OUT_OF_RANGE",
            "migrationDisposition": IDENTITY_DRIFT_DISPOSITION,
            "blockingReason": "LEGACY_MATERIAL_ORDINAL_OUT_OF_RANGE",
        }
    candidate_path = str(
        material_bindings[emitter_ordinal].get("objectPath") or ""
    )
    if candidate_path.casefold() != legacy_source_material_path.casefold():
        return {
            "legacySourceMaterialPath": legacy_source_material_path,
            "candidateMaterialObjectPath": candidate_path,
            "candidateMaterialBindingCount": len(material_bindings),
            "materialOrdinalJoinStatus": "SOURCE_MATERIAL_PATH_MISMATCH",
            "migrationDisposition": IDENTITY_DRIFT_DISPOSITION,
            "blockingReason": "LEGACY_SOURCE_MATERIAL_PATH_MISMATCH",
        }
    return {
        "legacySourceMaterialPath": legacy_source_material_path,
        "candidateMaterialObjectPath": candidate_path,
        "candidateMaterialBindingCount": len(material_bindings),
        "materialOrdinalJoinStatus": "EXACT_MATERIAL_ORDINAL_JOIN",
        "migrationDisposition": BLOCKED_DISPOSITION,
        "blockingReason": None,
    }


def carrier_contract_row(carrier: dict[str, Any]) -> dict[str, Any]:
    renderer_shape = str(carrier.get("rendererShape") or "")
    if renderer_shape not in {"sprite", "mesh", "decal", "light"}:
        raise InventoryError(
            "legacy V0 source system has an unresolved renderer shape: "
            + str(carrier.get("carrierKey") or "")
        )
    return {
        "carrierKey": str(carrier["carrierKey"]),
        "sourceOrder": int(carrier["sourceOrder"]),
        "sourceEmitterNodeId": str(carrier["sourceEmitterNodeId"]),
        "sourceEmitterOccurrence": int(carrier["sourceEmitterOccurrence"]),
        "sourceEmitterPath": str(carrier["sourceEmitterPath"]),
        "selectedLodNodeId": str(carrier["selectedLodNodeId"]),
        "selectedLodPath": str(carrier["selectedLodPath"]),
        "rendererShape": renderer_shape,
        "kind": str(carrier.get("kind") or ""),
        "runtimeReadinessDisposition": str(carrier.get("disposition") or ""),
        "conversionStatus": str(carrier.get("conversionStatus") or ""),
        "sourceRecipeSha256": carrier.get("sourceRecipeSha256"),
        "sourceResourceClosureSha256": str(
            carrier.get("sourceResourceClosureSha256") or ""
        ),
        "aggregateMigrationDisposition": BLOCKED_DISPOSITION,
        "blockingReason": "EXACT_REVIEWED_SOURCE_OCCURRENCE_NOT_JOINED",
    }


def _validate_expected(summary: dict[str, Any]) -> None:
    for key, expected in EXPECTED.items():
        actual = summary.get(key)
        if actual != expected:
            raise InventoryError(
                f"Valtan legacy V0 carrier denominator drifted: "
                f"{key} expected={expected!r} actual={actual!r}"
            )


def build_inventory() -> dict[str, Any]:
    cue_document = read_json(CUE_PATH)
    pattern_bindings = read_json(PATTERN_BINDINGS_PATH)
    boss_catalog = read_json(BOSS_CATALOG_PATH)
    combat_objects = read_json(COMBAT_OBJECTS_PATH)
    effect_catalog = read_json(EFFECT_CATALOG_PATH)
    source_catalog = read_json(SOURCE_CATALOG_PATH)

    product_owners = build_product_owners(
        cue_document,
        pattern_bindings,
        boss_catalog,
        combat_objects,
    )
    catalog_entries = effect_catalog.get("effects")
    catalog_systems = source_catalog.get("sourceSystems")
    if not isinstance(catalog_entries, list):
        raise InventoryError("Valtan Product catalog shape is invalid")
    if not isinstance(catalog_systems, list):
        raise InventoryError("Valtan source catalog has no sourceSystems")

    catalog_by_asset: dict[str, dict[str, Any]] = {}
    for entry in catalog_entries:
        asset_id = str(entry.get("effectAssetId") or "")
        if not asset_id or asset_id in catalog_by_asset:
            raise InventoryError(f"duplicate/empty Effect catalog id: {asset_id}")
        catalog_by_asset[asset_id] = entry

    systems_by_object_name: dict[str, list[dict[str, Any]]] = {}
    for system in catalog_systems:
        name = str(system.get("objectName") or "").casefold()
        if not name:
            raise InventoryError("Valtan source system has no objectName")
        systems_by_object_name.setdefault(name, []).append(system)

    document_contexts: list[dict[str, Any]] = []
    unknown_join_count = 0
    duplicate_join_count = 0
    required_catalog_systems: dict[str, dict[str, Any]] = {}

    for product_owner in product_owners:
        effect_asset_id = str(product_owner["effectAssetId"])
        catalog_entry = catalog_by_asset.get(effect_asset_id)
        if catalog_entry is None:
            raise InventoryError(
                f"Valtan Product owner is absent from EffectCatalog: {effect_asset_id}"
            )
        path = effect_document_path(catalog_entry)
        effect_document = read_json(path)
        if effect_document.get("effectAssetId") != effect_asset_id:
            raise InventoryError(
                f"Valtan Product document asset id drifted: {effect_asset_id}"
            )
        elements = effect_document.get("elements")
        if not isinstance(elements, list):
            raise InventoryError(f"Effect document has no elements: {effect_asset_id}")

        legacy_rows: list[dict[str, Any]] = []
        preserved_rows: list[dict[str, Any]] = []
        deduplicated_system_ids: list[str] = []
        for element_index, element in enumerate(elements):
            identity = legacy_identity(element)
            if identity is None:
                preserved_rows.append(
                    {
                        "elementIndex": element_index,
                        "elementId": str(element.get("id") or ""),
                        "sourceNode": str(element.get("sourceNode") or ""),
                        "elementSha256": canonical_sha256(element),
                        "preservationDisposition": PRESERVED_DISPOSITION,
                    }
                )
                continue

            system_stem, emitter_ordinal = identity
            catalog_system, join_status = resolve_source_system(
                system_stem, systems_by_object_name
            )
            if join_status == "UNKNOWN_SOURCE_SYSTEM":
                unknown_join_count += 1
            elif join_status == "DUPLICATE_SOURCE_SYSTEM_JOIN":
                duplicate_join_count += 1
            if catalog_system is None:
                source_system_id = None
                catalog_source_asset = None
                ordinal_proof = {
                    "legacySourceMaterialPath": str(
                        (element.get("material") or {}).get(
                            "sourceMaterialPath", ""
                        )
                    ),
                    "candidateMaterialObjectPath": None,
                    "candidateMaterialBindingCount": 0,
                    "materialOrdinalJoinStatus": "SOURCE_SYSTEM_JOIN_UNAVAILABLE",
                    "migrationDisposition": IDENTITY_DRIFT_DISPOSITION,
                    "blockingReason": "LEGACY_SOURCE_SYSTEM_JOIN_UNAVAILABLE",
                }
            else:
                catalog_source_asset = str(catalog_system["sourceAsset"])
                source_system_id = catalog_source_asset.casefold()
                required_catalog_systems[source_system_id] = catalog_system
                if source_system_id not in deduplicated_system_ids:
                    deduplicated_system_ids.append(source_system_id)
                ordinal_proof = material_ordinal_join(
                    element, catalog_system, emitter_ordinal
                )

            mesh_binding_count = sum(
                1
                for binding in element.get("resources", [])
                if isinstance(binding, dict)
                and binding.get("slotId") == "meshModel"
            )
            legacy_rows.append(
                {
                    "auditId": "legacy-row."
                    + canonical_sha256(
                        [effect_asset_id, element_index, element.get("id")]
                    )[:24],
                    "effectAssetId": effect_asset_id,
                    "ownerKind": product_owner["ownerKind"],
                    "elementIndex": element_index,
                    "elementId": str(element.get("id") or ""),
                    "sourceNode": "",
                    "legacySourceSystemStem": system_stem,
                    "legacyEmitterOrdinal": emitter_ordinal,
                    "legacyKind": str(element.get("kind") or ""),
                    "legacyMeshModelBindingCount": mesh_binding_count,
                    "legacyElementSha256": canonical_sha256(element),
                    "legacySourceMaterialPath": ordinal_proof[
                        "legacySourceMaterialPath"
                    ],
                    "candidateSourceSystemId": source_system_id,
                    "candidateCatalogSourceAsset": catalog_source_asset,
                    "sourceSystemJoinStatus": join_status,
                    "candidateMaterialObjectPath": ordinal_proof[
                        "candidateMaterialObjectPath"
                    ],
                    "candidateMaterialBindingCount": ordinal_proof[
                        "candidateMaterialBindingCount"
                    ],
                    "materialOrdinalJoinStatus": ordinal_proof[
                        "materialOrdinalJoinStatus"
                    ],
                    "exactCarrierKey": None,
                    "exactReviewedSourceOccurrenceId": None,
                    "migrationDisposition": ordinal_proof[
                        "migrationDisposition"
                    ],
                    "blockingReasons": [
                        reason
                        for reason in (
                            "LEGACY_EMITTER_ORDINAL_IS_NOT_A_CARRIER_KEY",
                            "EXACT_REVIEWED_SOURCE_OCCURRENCE_NOT_JOINED",
                            ordinal_proof["blockingReason"],
                        )
                        if reason is not None
                    ],
                }
            )

        if not legacy_rows:
            continue
        document_contexts.append(
            {
                "owner": product_owner,
                "path": path,
                "document": effect_document,
                "legacyRows": legacy_rows,
                "preservedRows": preserved_rows,
                "deduplicatedSourceSystemIds": deduplicated_system_ids,
            }
        )

    if unknown_join_count or duplicate_join_count:
        raise InventoryError(
            "legacy V0 source system joins are not one-to-one: "
            f"unknown={unknown_join_count} duplicate={duplicate_join_count}"
        )

    graph_specs = source_inventory.graph_specs(source_catalog)
    graph_index = source_inventory.source_receipts.load_graphs(graph_specs)
    runtime_bindings, runtime_cook_descriptor = (
        source_inventory.runtime_cook_receipt(graph_specs)
    )
    built_systems = {
        system_id: source_inventory.build_system_inventory(
            source_catalog,
            graph_index,
            required_catalog_systems[system_id],
            runtime_bindings,
            include_payloads=False,
        )
        for system_id in sorted(required_catalog_systems)
    }

    source_system_rows: list[dict[str, Any]] = []
    unique_carrier_keys: set[str] = set()
    unique_shape_counts: Counter[str] = Counter()
    for system_id in sorted(built_systems):
        source_system = built_systems[system_id]
        carrier_rows = [
            carrier_contract_row(row) for row in source_system["carriers"]
        ]
        for carrier in carrier_rows:
            key = carrier["carrierKey"]
            if key in unique_carrier_keys:
                raise InventoryError(f"duplicate global carrier key: {key}")
            unique_carrier_keys.add(key)
            unique_shape_counts[carrier["rendererShape"]] += 1
        source_system_rows.append(
            {
                "sourceSystemId": system_id,
                "catalogSourceAsset": str(source_system["catalogSourceAsset"]),
                "logicalPackage": str(source_system["logicalPackage"]),
                "graphRootNodeId": str(source_system["graphRootNodeId"]),
                "rootEmitterDenominator": int(
                    source_system["rootEmitterDenominator"]
                ),
                "selectedLodCarrierCount": int(
                    source_system["selectedLodCarrierCount"]
                ),
                "preservedUnresolvedEmitterCount": int(
                    source_system["preservedUnresolvedEmitterCount"]
                ),
                "droppedCarrierCount": int(source_system["droppedCarrierCount"]),
                "duplicateCarrierCount": int(
                    source_system["duplicateCarrierCount"]
                ),
                "rendererShapeCounts": dict(
                    sorted(Counter(
                        row["rendererShape"] for row in carrier_rows
                    ).items())
                ),
                "carriers": carrier_rows,
            }
        )

    documents: list[dict[str, Any]] = []
    legacy_row_audit: list[dict[str, Any]] = []
    aggregate_candidates: list[dict[str, Any]] = []
    aggregate_candidate_ids: set[str] = set()
    aggregate_shape_counts: Counter[str] = Counter()
    pure_count = 0
    mixed_count = 0

    for context in sorted(
        document_contexts, key=lambda row: row["owner"]["effectAssetId"]
    ):
        product_owner = context["owner"]
        legacy_rows = context["legacyRows"]
        preserved_rows = context["preservedRows"]
        document_classification = (
            "PURE_LEGACY_V0"
            if not preserved_rows
            else "MIXED_LEGACY_V0_AND_PRESERVED_NONLEGACY"
        )
        if preserved_rows:
            mixed_count += 1
        else:
            pure_count += 1

        document_shape_counts: Counter[str] = Counter()
        document_candidate_count = 0
        for source_system_id in context["deduplicatedSourceSystemIds"]:
            source_system = built_systems[source_system_id]
            for carrier in source_system["carriers"]:
                renderer_shape = str(carrier["rendererShape"])
                candidate_id = "aggregate-candidate." + canonical_sha256(
                    [product_owner["effectAssetId"], carrier["carrierKey"]]
                )[:24]
                if candidate_id in aggregate_candidate_ids:
                    raise InventoryError(
                        f"duplicate clip-aggregate candidate: {candidate_id}"
                    )
                aggregate_candidate_ids.add(candidate_id)
                document_candidate_count += 1
                document_shape_counts[renderer_shape] += 1
                aggregate_shape_counts[renderer_shape] += 1
                aggregate_candidates.append(
                    {
                        "candidateId": candidate_id,
                        "denominatorLabel": DENOMINATOR_LABEL,
                        "effectAssetId": product_owner["effectAssetId"],
                        "ownerKind": product_owner["ownerKind"],
                        "ownerRoot": product_owner["ownerRoot"],
                        "ownerId": product_owner["ownerId"],
                        "ownerStageActionId": product_owner[
                            "ownerStageActionId"
                        ],
                        "ownerClipOccurrenceId": product_owner[
                            "ownerClipOccurrenceId"
                        ],
                        "ownerClip": product_owner["ownerClip"],
                        "sourceSystemId": source_system_id,
                        "carrierKey": str(carrier["carrierKey"]),
                        "sourceOrder": int(carrier["sourceOrder"]),
                        "rendererShape": renderer_shape,
                        "productOccurrenceId": None,
                        "exactProductOccurrenceJoinStatus": (
                            "NOT_ESTABLISHED_LEGACY_CLIP_AGGREGATE"
                        ),
                        "migrationDisposition": BLOCKED_DISPOSITION,
                    }
                )

        documents.append(
            {
                "effectAssetId": product_owner["effectAssetId"],
                "authoringPath": repository_path(context["path"]),
                "documentSha256": sha256_file(context["path"]),
                "productOwner": product_owner,
                "documentClassification": document_classification,
                "productElementCount": len(context["document"]["elements"]),
                "legacyAggregateElementCount": len(legacy_rows),
                "preservedNonLegacyElementCount": len(preserved_rows),
                "deduplicatedSourceSystemCount": len(
                    context["deduplicatedSourceSystemIds"]
                ),
                "deduplicatedSourceSystemIds": context[
                    "deduplicatedSourceSystemIds"
                ],
                "rawAggregateDenominatorLabel": DENOMINATOR_LABEL,
                "rawAggregateCarrierCandidateCount": document_candidate_count,
                "rawAggregateRendererShapeCounts": dict(
                    sorted(document_shape_counts.items())
                ),
                "exactReviewedSourceOccurrenceJoinCount": 0,
                "migrationDisposition": BLOCKED_DISPOSITION,
                "preservedNonLegacyElements": preserved_rows,
            }
        )
        legacy_row_audit.extend(legacy_rows)

    repository_sources = [
        source_artifact(CUE_PATH),
        source_artifact(PATTERN_BINDINGS_PATH),
        source_artifact(BOSS_CATALOG_PATH),
        source_artifact(COMBAT_OBJECTS_PATH),
        source_artifact(EFFECT_CATALOG_PATH),
        source_artifact(SOURCE_CATALOG_PATH),
        source_artifact(SOURCE_OCCURRENCE_INVENTORY_PATH),
        source_artifact(
            ROOT / "Tools/EffectPipeline/build_valtan_source_occurrence_inventory.py"
        ),
        source_artifact(
            ROOT / "Tools/LevelPlacementExtractor/build_imported_effect_documents.py"
        ),
        source_artifact(
            ROOT / "Tools/LevelPlacementExtractor/build_skill_effect_source_receipt.py"
        ),
    ]
    product_effect_sources = [
        {
            "effectAssetId": row["effectAssetId"],
            "path": row["authoringPath"],
            "sha256": row["documentSha256"],
        }
        for row in documents
    ]

    summary = {
        "productOwnerDocumentCount": len(documents),
        "bossPatternCueOwnerDocumentCount": sum(
            row["productOwner"]["ownerKind"] == "BOSS_PATTERN_CUE"
            for row in documents
        ),
        "combatObjectOwnerDocumentCount": sum(
            row["productOwner"]["ownerKind"] == "COMBAT_OBJECT"
            for row in documents
        ),
        "productElementCount": sum(row["productElementCount"] for row in documents),
        "legacyAggregateElementCount": len(legacy_row_audit),
        "legacyRowsWithMeshModelBindingCount": sum(
            row["legacyMeshModelBindingCount"] > 0 for row in legacy_row_audit
        ),
        "legacyRowsWithoutMeshModelBindingCount": sum(
            row["legacyMeshModelBindingCount"] == 0 for row in legacy_row_audit
        ),
        "preservedNonLegacyElementCount": sum(
            row["preservedNonLegacyElementCount"] for row in documents
        ),
        "pureLegacyDocumentCount": pure_count,
        "mixedLegacyDocumentCount": mixed_count,
        "rawAggregateDenominatorLabel": DENOMINATOR_LABEL,
        "rawAggregateCarrierCandidateCount": len(aggregate_candidates),
        "rawAggregateRendererShapeCounts": dict(
            sorted(aggregate_shape_counts.items())
        ),
        "uniqueSourceSystemCount": len(source_system_rows),
        "uniqueSourceCarrierCount": len(unique_carrier_keys),
        "uniqueSourceCarrierRendererShapeCounts": dict(
            sorted(unique_shape_counts.items())
        ),
        "unknownSourceSystemJoinCount": unknown_join_count,
        "duplicateSourceSystemJoinCount": duplicate_join_count,
        "exactMaterialOrdinalJoinCount": sum(
            row["materialOrdinalJoinStatus"] == "EXACT_MATERIAL_ORDINAL_JOIN"
            for row in legacy_row_audit
        ),
        "blockedLegacyIdentityDriftCount": sum(
            row["migrationDisposition"] == IDENTITY_DRIFT_DISPOSITION
            for row in legacy_row_audit
        ),
        "exactReviewedSourceOccurrenceJoinCount": 0,
        "migrationDispositionCounts": {
            BLOCKED_DISPOSITION: sum(
                row["migrationDisposition"] == BLOCKED_DISPOSITION
                for row in legacy_row_audit
            ),
            IDENTITY_DRIFT_DISPOSITION: sum(
                row["migrationDisposition"] == IDENTITY_DRIFT_DISPOSITION
                for row in legacy_row_audit
            ),
        },
        "failedButRemovedElementCount": 0,
    }
    _validate_expected(summary)

    document = {
        "schema": SCHEMA_ID,
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "completionPolicy": (
            "READ_ONLY_V0_AUDIT; LEGACY_CLIP_AGGREGATE_IS_NOT_AN_EXACT_"
            "PRODUCT_OCCURRENCE; MATERIALIZE_ONLY_REVIEWED_SOURCE_OCCURRENCE_"
            "CARRIER_KEYS"
        ),
        "sources": {
            "repository": repository_sources,
            "sourceGraphs": source_inventory.graph_source_rows(graph_specs),
            "runtimeCookReceipt": runtime_cook_descriptor,
            "productEffectDocuments": product_effect_sources,
        },
        "summary": summary,
        "documents": documents,
        "legacyRowAudit": legacy_row_audit,
        "sourceSystems": source_system_rows,
        "legacyClipAggregateCandidates": aggregate_candidates,
    }
    validate_inventory(document)
    return document


def validate_inventory(document: dict[str, Any]) -> None:
    if document.get("schema") != SCHEMA_ID or document.get("formatVersion") != 1:
        raise InventoryError("legacy V0 carrier inventory header is invalid")
    summary = document.get("summary")
    if not isinstance(summary, dict):
        raise InventoryError("legacy V0 carrier inventory has no summary")
    _validate_expected(summary)

    documents = document.get("documents")
    legacy_rows = document.get("legacyRowAudit")
    systems = document.get("sourceSystems")
    candidates = document.get("legacyClipAggregateCandidates")
    if not all(isinstance(rows, list) for rows in (
        documents,
        legacy_rows,
        systems,
        candidates,
    )):
        raise InventoryError("legacy V0 carrier inventory arrays are invalid")
    if len(documents) != summary["productOwnerDocumentCount"]:
        raise InventoryError("document denominator does not match summary")
    if len(legacy_rows) != summary["legacyAggregateElementCount"]:
        raise InventoryError("legacy row denominator does not match summary")
    if len(systems) != summary["uniqueSourceSystemCount"]:
        raise InventoryError("source system denominator does not match summary")
    if len(candidates) != summary["rawAggregateCarrierCandidateCount"]:
        raise InventoryError("raw aggregate denominator does not match summary")
    if any(
        row.get("migrationDisposition")
        not in {BLOCKED_DISPOSITION, IDENTITY_DRIFT_DISPOSITION}
        or row.get("exactCarrierKey") is not None
        or row.get("exactReviewedSourceOccurrenceId") is not None
        for row in legacy_rows
    ):
        raise InventoryError("legacy V0 row was admitted without an exact occurrence")
    for row in legacy_rows:
        exact_material = (
            row.get("materialOrdinalJoinStatus")
            == "EXACT_MATERIAL_ORDINAL_JOIN"
        )
        if exact_material:
            if (
                row.get("migrationDisposition") != BLOCKED_DISPOSITION
                or str(row.get("candidateMaterialObjectPath") or "").casefold()
                != str(row.get("legacySourceMaterialPath") or "").casefold()
            ):
                raise InventoryError("exact material ordinal proof is inconsistent")
        elif row.get("migrationDisposition") != IDENTITY_DRIFT_DISPOSITION:
            raise InventoryError("legacy material identity drift is not fail-closed")
    if any(
        row.get("denominatorLabel") != DENOMINATOR_LABEL
        or row.get("productOccurrenceId") is not None
        or row.get("migrationDisposition") != BLOCKED_DISPOSITION
        for row in candidates
    ):
        raise InventoryError("clip aggregate was mislabeled as a Product occurrence")


def serialized(document: dict[str, Any]) -> bytes:
    return (
        json.dumps(document, ensure_ascii=False, indent=2) + "\n"
    ).encode("utf-8")


def write_output(document: dict[str, Any], output_path: Path = OUTPUT_PATH) -> bool:
    payload = serialized(document)
    if output_path.is_file() and output_path.read_bytes() == payload:
        return False
    output_path.parent.mkdir(parents=True, exist_ok=True)
    staging = output_path.with_name(output_path.name + ".staging")
    try:
        staging.write_bytes(payload)
        os.replace(staging, output_path)
    finally:
        if staging.exists():
            staging.unlink()
    return True


def check_output(document: dict[str, Any], output_path: Path = OUTPUT_PATH) -> None:
    if not output_path.is_file():
        raise InventoryError(f"inventory output is missing: {output_path}")
    if output_path.read_bytes() != serialized(document):
        raise InventoryError(
            "legacy V0 carrier inventory is stale; rerun with --write"
        )


def validate_sealed_historical_preimage(
    artifact: dict[str, Any], receipt: dict[str, Any]
) -> None:
    """Validate the post-migration ledger without rebuilding deleted rows."""

    validate_inventory(artifact)
    summary = receipt.get("summary") or {}
    if (
        receipt.get("schema")
        != "lostark.valtan-carrier-v1-materialization-receipt"
        or receipt.get("formatVersion") != 1
        or summary.get("baselineStrictLegacyRowCount")
        != EXPECTED["legacyAggregateElementCount"]
    ):
        raise InventoryError("carrier V1 historical receipt header is invalid")
    sealed = (receipt.get("legacyMigration") or {}).get("sealedInventory") or {}
    if (
        sealed.get("path") != repository_path(OUTPUT_PATH)
        or sealed.get("canonicalSha256") != canonical_sha256(artifact)
        or sealed.get("authorizedRowCount")
        != EXPECTED["legacyAggregateElementCount"]
        or sealed.get("requiredJoinStatus") != "EXACT_MATERIAL_ORDINAL_JOIN"
    ):
        raise InventoryError("historical legacy inventory seal drifted")
    schema = read_json(SCHEMA_PATH)
    if schema.get("$id") != (
        "lostark.valtan-legacy-v0-carrier-migration-inventory.schema.json"
    ):
        raise InventoryError("historical legacy inventory schema drifted")

    migration = receipt.get("legacyMigration") or {}
    migration_rows = {
        str(row.get("effectAssetId") or ""): row
        for row in migration.get("documents", [])
    }
    artifact_documents = {
        str(row.get("effectAssetId") or ""): row
        for row in artifact["documents"]
    }
    if not set(artifact_documents).issubset(migration_rows):
        raise InventoryError("historical receipt omits a legacy Product preimage")
    for effect_id, source in artifact_documents.items():
        row = migration_rows[effect_id]
        if (
            row.get("preimageByteSha256") != source.get("documentSha256")
            or row.get("beforeElementCount") != source.get("productElementCount")
            or row.get("strictLegacyRemovedCount")
            != source.get("legacyAggregateElementCount")
            or row.get("exactSourceMovedCount")
            + row.get("nonExactExistingRemovedCount")
            != source.get("preservedNonLegacyElementCount")
        ):
            raise InventoryError(
                f"historical Product preimage hash/count drifted: {effect_id}"
            )
    if any(
        not re.fullmatch(r"[0-9a-f]{64}", str(row.get(field) or ""))
        for row in migration_rows.values()
        for field in ("preimageByteSha256", "preimageCanonicalSha256")
    ):
        raise InventoryError("historical Product preimage hash is missing")

    retired = list(migration.get("retiredEffectAssetIds") or [])
    if len(retired) != 105:
        raise InventoryError("historical retired document denominator drifted")
    for effect_id in retired:
        row = migration_rows.get(effect_id)
        path = ROOT / str((row or {}).get("path") or "")
        if row is None or not path.is_file():
            raise InventoryError(f"historical evidence shell is missing: {effect_id}")
        shell = read_json(path)
        if shell.get("effectAssetId") != effect_id or shell.get("elements") != []:
            raise InventoryError(
                f"historical evidence shell regained rows: {effect_id}"
            )

    # ``outputs`` is the immutable Carrier V1 materialization preimage, not a
    # seal over every later Valtan catalog/cue successor.  Current Product
    # closure is owned by the active publisher/master pipeline; keep proving
    # the exact historical output identity here without comparing it to live
    # append-only successors.
    outputs = receipt.get("outputs") or {}
    catalog_output = outputs.get("catalog") or {}
    cue_output = outputs.get("cues") or {}
    if (
        catalog_output.get("path") != repository_path(EFFECT_CATALOG_PATH)
        or catalog_output.get("scope") != "EFFECT_ASSET_ID_PREFIX"
        or catalog_output.get("effectAssetIdPrefix") != "effect.valtan."
        or catalog_output.get("effectCount")
        != summary.get("finalValtanCatalogCount")
        or catalog_output.get("canonicalSha256")
        != "123c070157e743ef467294607f104a9e5f1d90c3c99f73b6cf9c48033da093da"
        or cue_output.get("path") != repository_path(CUE_PATH)
        or cue_output.get("cueCount") != summary.get("finalBossRootCueCount")
        or cue_output.get("canonicalSha256")
        != "4ff3c88cffdbe84abb99aaee22aad86c92f1b1797dfd8706058ded489b738dc9"
    ):
        raise InventoryError("historical receipt Product output seal drifted")


def sealed_historical_preimage_is_applied() -> bool:
    if not MATERIALIZATION_RECEIPT_PATH.is_file():
        return False
    artifact = read_json(OUTPUT_PATH)
    receipt = read_json(MATERIALIZATION_RECEIPT_PATH)
    validate_sealed_historical_preimage(artifact, receipt)
    return True


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    args = parser.parse_args(argv)
    try:
        historical = sealed_historical_preimage_is_applied()
        if historical:
            if args.write:
                raise InventoryError(
                    "legacy V0 preimage is historical after carrier V1 migration"
                )
            document = read_json(OUTPUT_PATH)
            action = "checked-sealed-historical-preimage"
        else:
            document = build_inventory()
        if args.write and not historical:
            changed = write_output(document)
            action = "wrote" if changed else "already-current"
        elif not args.write and not historical:
            check_output(document)
            action = "checked"
    except (InventoryError, OSError, ValueError, KeyError, TypeError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    summary = document["summary"]
    print(
        "PASS: Valtan legacy V0 carrier migration inventory "
        f"{action}; documents={summary['productOwnerDocumentCount']} "
        f"legacy={summary['legacyAggregateElementCount']} "
        f"rawCandidates={summary['rawAggregateCarrierCandidateCount']} "
        f"uniqueCarriers={summary['uniqueSourceCarrierCount']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
