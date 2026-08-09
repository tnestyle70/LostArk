#!/usr/bin/env python3

from __future__ import annotations

import argparse
import collections
import copy
import hashlib
import json
import sys
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"JSON root must be an object: {path}")
    return value


def file_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(
            value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
        ).encode("utf-8")
    ).hexdigest()


def json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, sort_keys=False) + "\n"
    ).encode("utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def repository_path(path: Path) -> str:
    resolved = path.resolve()
    try:
        return resolved.relative_to(REPO_ROOT).as_posix()
    except ValueError:
        return resolved.as_posix()


def base_property(value: str) -> str:
    return value.casefold().split("[", 1)[0].split(".", 1)[0]


def raw_distribution_wrappers(properties: dict[str, Any]) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []

    def visit(value: Any, path: str) -> None:
        if (
            isinstance(value, dict)
            and value.get("type") == "structproperty"
            and str(value.get("structType", "")).casefold()
            in {"rawdistributionfloat", "rawdistributionvector"}
        ):
            result.append(
                {
                    "propertyPath": path,
                    "structType": str(value.get("structType", "")).casefold(),
                    "raw": copy.deepcopy(value.get("value") or {}),
                }
            )
            return
        if isinstance(value, dict) and "type" in value and "value" in value:
            visit(value.get("value"), path)
            return
        if isinstance(value, dict):
            for name, child in value.items():
                child_path = f"{path}.{str(name).casefold()}" if path else str(name).casefold()
                visit(child, child_path)
            return
        if isinstance(value, list):
            for index, child in enumerate(value):
                visit(child, f"{path}[{index}]")

    for name, value in properties.items():
        visit(value, str(name).casefold())
    return result


def primitive_leaf_count(properties: dict[str, Any]) -> int:
    count = 0

    def visit(value: Any) -> None:
        nonlocal count
        if (
            isinstance(value, dict)
            and value.get("type") == "structproperty"
            and str(value.get("structType", "")).casefold()
            in {"rawdistributionfloat", "rawdistributionvector"}
        ):
            return
        if isinstance(value, dict) and "type" in value and "value" in value:
            visit(value.get("value"))
            return
        if isinstance(value, (bool, int, float, str)):
            count += 1
            return
        if isinstance(value, list):
            for child in value:
                visit(child)
            return
        if isinstance(value, dict):
            for child in value.values():
                visit(child)

    for value in properties.values():
        visit(value)
    return count


def raw_distribution_inference(wrapper: dict[str, Any]) -> dict[str, bool]:
    raw = wrapper.get("raw") or {}
    properties = raw.get("properties") if isinstance(raw, dict) else None
    properties = properties if isinstance(properties, dict) else {}

    def value(name: str) -> Any:
        row = properties.get(name)
        return row.get("value") if isinstance(row, dict) else None

    lookup = value("lookuptable")
    lookup_present = isinstance(lookup, list) and bool(lookup)
    operation = value("op")
    chunk_size = value("lookuptablechunksize")
    num_elements = value("lookuptablenumelements")
    return {
        "operationDeterministicConversion": operation in {None, 0},
        "chunkSizeDeterministicConversion": lookup_present
        and chunk_size in {None, 0},
        "numElementsDeterministicConversion": lookup_present
        and num_elements in {None, 0},
    }


def source_object_indexes(
    graph: dict[str, Any], external: dict[str, Any]
) -> tuple[dict[str, dict[str, Any]], dict[str, dict[str, Any]]]:
    graph_objects = {str(row["nodeId"]): row for row in graph["nodes"]}
    external_objects = {
        str(row["objectId"]): row
        for package in external["packages"]
        for row in package["objects"]
    }
    return graph_objects, external_objects


def external_path_index(external: dict[str, Any]) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for package in external["packages"]:
        logical = str(package["logicalPackage"])
        for row in package["objects"]:
            relative = str(row["objectPath"])
            result[relative.casefold()] = row
            result[f"{logical}.{relative}".casefold()] = row
    return result


def load_pinned_source_graphs(
    source_receipt: dict[str, Any], source_graph_root: Path
) -> tuple[dict[str, dict[str, Any]], list[dict[str, Any]]]:
    objects: dict[str, dict[str, Any]] = {}
    pins = []
    for package in source_receipt["sourcePackages"]:
        logical = str(package["logicalPackage"])
        path = source_graph_root / f"{logical}.particle-graph.json"
        require(path.is_file(), f"pinned source graph is missing: {path}")
        require(
            file_sha256(path) == package["graphSha256"],
            f"pinned source graph hash changed: {path}",
        )
        graph = load_json(path)
        for row in graph["objects"]:
            objects[str(row["objectId"])] = row
        pins.append(
            {
                "logicalPackage": logical,
                "graphAsset": path.name,
                "bytes": path.stat().st_size,
                "sha256": file_sha256(path),
            }
        )
    return objects, pins


def build_reference_order(
    element: dict[str, Any],
    graph: dict[str, Any],
    graph_by_id: dict[str, dict[str, Any]],
    external_by_path: dict[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    lod_id = str(element["sourceLodNode"]["nodeId"])
    system = next(
        row
        for row in graph["sourceSystems"]
        if row["sourceSystemId"] == element["sourceSystemId"]
    )
    references = [
        row
        for row in graph["edges"]
        if row["sourceNodeId"] == lod_id and row.get("targetNodeId")
    ]
    references.extend(
        row
        for row in system.get("unresolvedExternalReferences", [])
        if row["sourceNodeId"] == lod_id
    )
    references.sort(key=lambda row: int(row.get("referenceIndex", 0)))
    evidence = element["moduleEvidence"]
    require(
        len(references) == len(evidence),
        f"selected LOD module-order denominator changed: {element['activeElementId']}",
    )
    result = []
    for order, (reference, module) in enumerate(zip(references, evidence)):
        target = graph_by_id.get(str(reference.get("targetNodeId") or ""))
        if target is None:
            target = external_by_path.get(str(reference.get("objectPath", "")).casefold())
        require(target is not None, f"selected module target is missing: {reference}")
        module_identity = str(module.get("nodeId") or module.get("objectId") or "")
        target_identity = str(target.get("nodeId") or target.get("objectId") or "")
        require(
            module_identity == target_identity,
            f"selected module order differs from active inventory: {module_identity}",
        )
        role = {
            "requiredmodule": "REQUIRED",
            "spawnmodule": "SPAWN",
            "typedatamodule": "TYPE_DATA",
            "modules": "MODULE",
        }.get(base_property(str(reference.get("property", ""))), "UNRESOLVED")
        require(role != "UNRESOLVED", f"unknown selected module role: {reference}")
        result.append(
            {
                "order": order,
                "sourceReferenceIndex": int(reference.get("referenceIndex", 0)),
                "role": role,
                "sourceDocument": module["sourceDocument"],
                "sourceObjectId": module_identity,
                "sourceClass": module["className"],
                "sourceObjectPath": module["resolvedObjectPath"],
                "sourceRecordSha256": module["recordSha256"],
            }
        )
    return result


def explicit_value(properties: dict[str, Any], name: str) -> tuple[Any, str]:
    row = properties.get(name)
    if isinstance(row, dict) and "value" in row:
        return copy.deepcopy(row["value"]), "SOURCE_EXPLICIT"
    return None, "UNRESOLVED_CLASS_DEFAULT"


def build_evidence(
    source_receipt_path: Path,
    action_recipe_path: Path,
    active_inventory_path: Path,
    normalized_graph_path: Path,
    external_closure_path: Path,
    local_reference_closure_path: Path,
    geometry_parity_path: Path,
    source_graph_root: Path,
) -> dict[str, Any]:
    source_receipt = load_json(source_receipt_path)
    action_recipe = load_json(action_recipe_path)
    inventory = load_json(active_inventory_path)
    graph = load_json(normalized_graph_path)
    external = load_json(external_closure_path)
    local_references = load_json(local_reference_closure_path)
    geometry = load_json(geometry_parity_path)
    require(source_receipt.get("skillId") == 31470, "source receipt skill mismatch")
    require(len(inventory.get("activeElements", [])) == 35, "active element count changed")
    graph_by_id, external_by_id = source_object_indexes(graph, external)
    external_by_path = external_path_index(external)
    source_graph_objects, source_graph_pins = load_pinned_source_graphs(
        source_receipt, source_graph_root
    )
    cue_by_id = {row["cueId"]: row for row in action_recipe["cues"]}
    graph_edges: dict[str, list[dict[str, Any]]] = collections.defaultdict(list)
    for row in graph["edges"]:
        graph_edges[str(row["sourceNodeId"])].append(row)
    system_by_id = {row["sourceSystemId"]: row for row in graph["sourceSystems"]}

    module_occurrence_count = 0
    top_property_count = 0
    primitive_leaf_total = 0
    distribution_wrapper_count = 0
    operation_conversion_count = 0
    chunk_conversion_count = 0
    num_elements_conversion_count = 0
    internal_tail_proven = 0
    external_tail_unresolved = 0
    seeded_hex_count = 0
    seeded_hex_bytes = 0
    seeded_array_count = 0
    seeded_default_count = 0
    active_two_lod_count = 0
    rows = []

    for element in inventory["activeElements"]:
        cue = cue_by_id[element["cueId"]]
        typed = cue.get("typedPayload") or {}
        occurrence = cue.get("sourceOccurrence") or {}
        require(
            typed.get("particleDataDecoded") is True
            and typed.get("parameterOverridesDecoded") is True,
            f"active cue payload is incomplete: {cue['cueId']}",
        )
        emitter_id = str(element["sourceEmitterNode"]["nodeId"])
        lod_id = str(element["sourceLodNode"]["nodeId"])
        emitter = graph_by_id[emitter_id]
        lod = graph_by_id[lod_id]
        root = graph_by_id[system_by_id[element["sourceSystemId"]]["rootNodeId"]]
        lod_edges = sorted(
            (
                row
                for row in graph_edges[emitter_id]
                if base_property(str(row.get("property", ""))) == "lodlevels"
                and row.get("targetNodeId")
            ),
            key=lambda row: int(row.get("referenceIndex", 0)),
        )
        selected_indices = [
            index
            for index, edge in enumerate(lod_edges)
            if edge["targetNodeId"] == lod_id
        ]
        require(selected_indices == [0], "FIRST_LOD_ONLY selection changed")
        if len(lod_edges) == 2:
            active_two_lod_count += 1
        nonselected_lods = []
        system = system_by_id[element["sourceSystemId"]]
        for edge in lod_edges[1:]:
            nonselected_id = str(edge["targetNodeId"])
            external_count = sum(
                row["sourceNodeId"] == nonselected_id
                for row in system.get("unresolvedExternalReferences", [])
            )
            nonselected_lods.append(
                {
                    "sourceLodNodeId": nonselected_id,
                    "sourceLodPath": edge["objectPath"],
                    "directInternalReferenceCount": len(graph_edges[nonselected_id]),
                    "directExternalReferenceCount": external_count,
                    "status": "PRESERVED_NOT_SELECTED_COMPILED_EXECUTION_BLOCKED",
                }
            )
        reference_order = build_reference_order(
            element, graph, graph_by_id, external_by_path
        )
        module_occurrence_count += len(reference_order)
        module_rows = []
        for module_order, module in enumerate(element["moduleEvidence"]):
            source_document = str(module["sourceDocument"])
            object_id = str(module.get("nodeId") or module.get("objectId") or "")
            raw = (
                graph_by_id[object_id]
                if source_document == "normalizedGraph"
                else external_by_id[object_id]
            )
            properties = raw.get("properties") or {}
            top_property_count += len(properties)
            leaves = primitive_leaf_count(properties)
            primitive_leaf_total += leaves
            wrappers = raw_distribution_wrappers(properties)
            distribution_wrapper_count += len(wrappers)
            inferred = collections.Counter()
            for wrapper in wrappers:
                inference = raw_distribution_inference(wrapper)
                operation_conversion_count += int(
                    inference["operationDeterministicConversion"]
                )
                chunk_conversion_count += int(
                    inference["chunkSizeDeterministicConversion"]
                )
                num_elements_conversion_count += int(
                    inference["numElementsDeterministicConversion"]
                )
                inferred.update(name for name, enabled in inference.items() if enabled)

            native_tail_status = "UNRESOLVED_EXTERNAL_CLOSURE_SERIAL_SIZE_MISSING"
            if source_document == "normalizedGraph":
                original = source_graph_objects.get(object_id)
                require(original is not None, f"pinned source object is missing: {object_id}")
                require(
                    int(original["serialSize"])
                    == int(original["propertyStreamEnd"]),
                    f"pinned source object has an unparsed native tail: {object_id}",
                )
                native_tail_status = "SOURCE_DECODED_NO_NATIVE_TAIL"
                internal_tail_proven += 1
            else:
                require(
                    "serialSize" not in raw and "propertyStreamEnd" in raw,
                    "external closure native-tail contract changed",
                )
                external_tail_unresolved += 1

            seed_status = "NOT_SEEDED"
            if str(raw["className"]).casefold().endswith("_seeded"):
                random_seed = properties.get("randomseedinfo")
                payload = random_seed.get("value") if isinstance(random_seed, dict) else None
                if isinstance(payload, dict) and isinstance(payload.get("hex"), str):
                    seed_status = "OPAQUE_HEX_METADATA_ONLY"
                    seeded_hex_count += 1
                    seeded_hex_bytes += len(bytes.fromhex(payload["hex"]))
                elif isinstance(payload, dict):
                    random_properties = payload.get("properties")
                    seeds = (
                        random_properties.get("randomseeds")
                        if isinstance(random_properties, dict)
                        else None
                    )
                    if isinstance(seeds, dict) and isinstance(seeds.get("value"), list):
                        seed_status = "SEED_ARRAY_SOURCE_DECODED_CONSUMPTION_UNRESOLVED"
                        seeded_array_count += 1
                    else:
                        seed_status = "CLASS_DEFAULT_UNRESOLVED"
                        seeded_default_count += 1
                else:
                    seed_status = "CLASS_DEFAULT_UNRESOLVED"
                    seeded_default_count += 1
            module_rows.append(
                {
                    **reference_order[module_order],
                    "topLevelTaggedPropertyCount": len(properties),
                    "primitiveLeafCount": leaves,
                    "distributionWrapperCount": len(wrappers),
                    "deterministicDistributionFieldConversions": dict(inferred),
                    "randomSeedStatus": seed_status,
                    "nativeTailStatus": native_tail_status,
                }
            )

        level, level_status = explicit_value(lod.get("properties") or {}, "level")
        enabled, enabled_status = explicit_value(
            lod.get("properties") or {}, "benabled"
        )
        rows.append(
            {
                "evidenceId": element["activeElementId"],
                "sourceCueId": element["cueId"],
                "sourceOccurrenceId": occurrence["notifyId"],
                "sourceSystemId": element["sourceSystemId"],
                "sourceSystemNodeId": system["rootNodeId"],
                "sourceSystemRecordSha256": element[
                    "sourceSystemGraphRecordSha256"
                ],
                "sourceEmitterPath": element["sourceEmitter"],
                "sourceEmitterNodeId": emitter_id,
                "sourceEmitterRecordSha256": element["sourceEmitterNode"][
                    "recordSha256"
                ],
                "lodSelectionPolicy": "FIRST_LOD_ONLY",
                "selectedLod": {
                    "sourceLodPath": element["sourceLod"],
                    "sourceLodNodeId": lod_id,
                    "sourceLodRecordSha256": element["sourceLodNode"][
                        "recordSha256"
                    ],
                    "emitterArrayIndex": 0,
                    "level": level,
                    "levelProvenance": level_status,
                    "enabled": enabled,
                    "enabledProvenance": enabled_status,
                    "topLevelTaggedPropertyCount": len(lod.get("properties") or {}),
                },
                "nonSelectedLods": nonselected_lods,
                "sourceContainerCoverage": {
                    "systemRootTaggedPropertyCount": len(root.get("properties") or {}),
                    "emitterTaggedPropertyCount": len(emitter.get("properties") or {}),
                    "selectedLodTaggedPropertyCount": len(lod.get("properties") or {}),
                },
                "moduleReferenceOrder": module_rows,
                "cueLocalTransform": copy.deepcopy(typed["localTransform"]),
                "parameterOverrides": copy.deepcopy(typed["parameterOverrides"]),
                "attachment": copy.deepcopy(typed["attachment"]),
                "compositionOrder": [
                    "carrierGeometryPreScale",
                    "signedParticleScaleRotationLocation",
                    "emitterElementTransform",
                    "cueLocalTransform",
                    "attachmentSocketOrRoot",
                    "actorWorld",
                ],
                "compiledExecutionAdmission": {
                    "allowed": False,
                    "blockers": [
                        "SOURCE_ONLY_NO_COMPILED_EXECUTION",
                        "MATERIAL_RECIPE_AND_RENDER_STATE_NOT_COMPILED",
                        *(
                            ["SELECTED_LOD_CLASS_DEFAULTS_UNRESOLVED"]
                            if level_status.startswith("UNRESOLVED")
                            or enabled_status.startswith("UNRESOLVED")
                            else []
                        ),
                    ],
                },
            }
        )

    active_system_ids = {row["sourceSystemId"] for row in inventory["activeElements"]}
    system_root_ids = {
        system_by_id[source_system_id]["rootNodeId"]
        for source_system_id in active_system_ids
    }
    emitter_ids = {
        row["sourceEmitterNode"]["nodeId"] for row in inventory["activeElements"]
    }
    selected_lod_ids = {
        row["sourceLodNode"]["nodeId"] for row in inventory["activeElements"]
    }
    system_property_count = sum(
        len(graph_by_id[node_id].get("properties") or {}) for node_id in system_root_ids
    )
    emitter_property_count = sum(
        len(graph_by_id[node_id].get("properties") or {}) for node_id in emitter_ids
    )
    lod_property_count = sum(
        len(graph_by_id[node_id].get("properties") or {}) for node_id in selected_lod_ids
    )

    all_emitter_ids = {
        str(edge["targetNodeId"])
        for system in graph["sourceSystems"]
        for edge in graph_edges[str(system["rootNodeId"])]
        if base_property(str(edge.get("property", ""))) == "emitters"
        and edge.get("targetNodeId")
    }
    full_selected_lod_ids: set[str] = set()
    full_nonselected_lod_ids: set[str] = set()
    for emitter_id in all_emitter_ids:
        lods = sorted(
            (
                str(edge["targetNodeId"])
                for edge in graph_edges[emitter_id]
                if base_property(str(edge.get("property", ""))) == "lodlevels"
                and edge.get("targetNodeId")
            )
        )
        # Reference index, not lexical object id, owns selection.  Rebuild in
        # that order after using the set above only for denominator checks.
        ordered_lods = [
            str(edge["targetNodeId"])
            for edge in sorted(
                graph_edges[emitter_id],
                key=lambda edge: int(edge.get("referenceIndex", 0)),
            )
            if base_property(str(edge.get("property", ""))) == "lodlevels"
            and edge.get("targetNodeId")
        ]
        if not ordered_lods:
            continue
        full_selected_lod_ids.add(ordered_lods[0])
        full_nonselected_lod_ids.update(ordered_lods[1:])
    all_unresolved = [
        reference
        for system in graph["sourceSystems"]
        for reference in system.get("unresolvedExternalReferences", [])
    ]
    full_selected_external = sum(
        row["sourceNodeId"] in full_selected_lod_ids for row in all_unresolved
    )
    full_nonselected_external = sum(
        row["sourceNodeId"] in full_nonselected_lod_ids for row in all_unresolved
    )
    filtered_local = len(all_unresolved) - full_selected_external - full_nonselected_external

    summary = {
        "activeCueCount": len(inventory["activeCues"]),
        "activeOccurrenceCount": len(rows),
        "activeSelectedModuleReferenceOrderCount": module_occurrence_count,
        "activeModuleTopLevelTaggedPropertyCount": top_property_count,
        "activeModulePrimitiveLeafCount": primitive_leaf_total,
        "activeDistributionWrapperCount": distribution_wrapper_count,
        "distributionOperationDeterministicConversionCount": operation_conversion_count,
        "distributionChunkSizeDeterministicConversionCount": chunk_conversion_count,
        "distributionNumElementsDeterministicConversionCount": num_elements_conversion_count,
        "activeSystemRootTaggedPropertyCount": system_property_count,
        "activeEmitterTaggedPropertyCount": emitter_property_count,
        "activeSelectedLodTaggedPropertyCount": lod_property_count,
        "activeInternalModuleNativeTailProvenOccurrenceCount": internal_tail_proven,
        "activeExternalModuleNativeTailUnresolvedOccurrenceCount": external_tail_unresolved,
        "activeSeededModuleOccurrenceCount": seeded_hex_count
        + seeded_array_count
        + seeded_default_count,
        "activeOpaqueSeedHexOccurrenceCount": seeded_hex_count,
        "activeOpaqueSeedHexBytes": seeded_hex_bytes,
        "activeSeedArrayOnlyOccurrenceCount": seeded_array_count,
        "activeSeedClassDefaultOccurrenceCount": seeded_default_count,
        "activeTwoLodEmitterOccurrenceCount": active_two_lod_count,
        "fullSourceSelectedLodCount": len(full_selected_lod_ids),
        "fullSourceSelectedLodExternalReferenceCount": full_selected_external,
        "fullSourceNonSelectedLodCount": len(full_nonselected_lod_ids),
        "fullSourceNonSelectedLodExternalReferenceCount": full_nonselected_external,
        "fullSourceFilteredLocalReferenceCount": filtered_local,
        "fullSourceNonSelectedOrFilteredReferenceBlockedCount": full_nonselected_external
        + filtered_local,
        "sourceEvidenceStatus": "SOURCE_EVIDENCE_PARTIAL",
        "compiledExecutionAllowedOccurrenceCount": 0,
    }
    expected = {
        "activeCueCount": 7,
        "activeOccurrenceCount": 35,
        "activeSelectedModuleReferenceOrderCount": 399,
        "activeModuleTopLevelTaggedPropertyCount": 1434,
        "activeModulePrimitiveLeafCount": 1572,
        "activeDistributionWrapperCount": 629,
        "distributionOperationDeterministicConversionCount": 409,
        "distributionChunkSizeDeterministicConversionCount": 257,
        "distributionNumElementsDeterministicConversionCount": 257,
        "activeSystemRootTaggedPropertyCount": 61,
        "activeEmitterTaggedPropertyCount": 53,
        "activeSelectedLodTaggedPropertyCount": 158,
        "activeInternalModuleNativeTailProvenOccurrenceCount": 151,
        "activeExternalModuleNativeTailUnresolvedOccurrenceCount": 248,
        "activeSeededModuleOccurrenceCount": 14,
        "activeOpaqueSeedHexOccurrenceCount": 8,
        "activeOpaqueSeedHexBytes": 256,
        "activeSeedArrayOnlyOccurrenceCount": 5,
        "activeSeedClassDefaultOccurrenceCount": 1,
        "activeTwoLodEmitterOccurrenceCount": 30,
        "fullSourceSelectedLodCount": 125,
        "fullSourceSelectedLodExternalReferenceCount": 913,
        "fullSourceNonSelectedLodExternalReferenceCount": 875,
        "fullSourceFilteredLocalReferenceCount": 5,
        "fullSourceNonSelectedOrFilteredReferenceBlockedCount": 880,
    }
    for name, value in expected.items():
        require(summary[name] == value, f"source evidence denominator changed: {name}")

    evidence: dict[str, Any] = {
        "schema": "lostark.effect-source-evidence-envelope",
        "formatVersion": 1,
        "characterClass": "Artist",
        "skillId": 31470,
        "scope": "ACTIVE_OCCURRENCES_FIRST_LOD_ONLY",
        "status": "SOURCE_EVIDENCE_PARTIAL",
        "inputs": {
            "sourceReceipt": {
                "path": repository_path(source_receipt_path),
                "sha256": file_sha256(source_receipt_path),
            },
            "actionCueRecipe": {
                "path": repository_path(action_recipe_path),
                "sha256": file_sha256(action_recipe_path),
            },
            "activeInventory": {
                "path": repository_path(active_inventory_path),
                "sha256": file_sha256(active_inventory_path),
            },
            "normalizedGraph": {
                "path": repository_path(normalized_graph_path),
                "sha256": file_sha256(normalized_graph_path),
            },
            "externalModuleClosure": {
                "path": repository_path(external_closure_path),
                "sha256": file_sha256(external_closure_path),
            },
            "localReferenceClosure": {
                "path": repository_path(local_reference_closure_path),
                "sha256": file_sha256(local_reference_closure_path),
                "selfSha256": local_references["closureSha256"],
            },
            "wmodelGeometryParity": {
                "path": repository_path(geometry_parity_path),
                "sha256": file_sha256(geometry_parity_path),
                "selfSha256": geometry["receiptSha256"],
            },
            "pinnedSourceGraphs": source_graph_pins,
        },
        "selectionPolicy": {
            "lodPolicy": "FIRST_LOD_ONLY",
            "selectedLodCompiled": False,
            "nonSelectedLodsPreserved": True,
            "nonSelectedLodCompiled": False,
        },
        "compositionContract": {
            "matrixOrder": [
                "carrierGeometryPreScale",
                "signedParticleScaleRotationLocation",
                "emitterElementTransform",
                "cueLocalTransform",
                "attachmentSocketOrRoot",
                "actorWorld",
            ],
            "legacyDetailTransformIsCompilerInput": False,
        },
        "occurrences": rows,
        "summary": summary,
        "evidenceSha256": "",
    }
    unsigned = copy.deepcopy(evidence)
    unsigned.pop("evidenceSha256")
    evidence["evidenceSha256"] = canonical_sha256(unsigned)
    return evidence


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build immutable Artist F occurrence/LOD/module-order evidence."
    )
    parser.add_argument("--source-receipt", required=True, type=Path)
    parser.add_argument("--action-cue-recipe", required=True, type=Path)
    parser.add_argument("--active-inventory", required=True, type=Path)
    parser.add_argument("--normalized-graph", required=True, type=Path)
    parser.add_argument("--external-module-closure", required=True, type=Path)
    parser.add_argument("--local-reference-closure", required=True, type=Path)
    parser.add_argument("--geometry-parity", required=True, type=Path)
    parser.add_argument("--source-graph-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    evidence = build_evidence(
        args.source_receipt,
        args.action_cue_recipe,
        args.active_inventory,
        args.normalized_graph,
        args.external_module_closure,
        args.local_reference_closure,
        args.geometry_parity,
        args.source_graph_root,
    )
    content = json_bytes(evidence)
    if args.check:
        require(
            args.output.is_file() and args.output.read_bytes() == content,
            f"generated output is stale: {args.output}",
        )
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(content)
    summary = evidence["summary"]
    print(
        "Artist F 31470 source evidence "
        f"{'check' if args.check else 'write'}: "
        f"occurrences={summary['activeOccurrenceCount']} "
        f"modules={summary['activeSelectedModuleReferenceOrderCount']} "
        f"properties={summary['activeModuleTopLevelTaggedPropertyCount']}/"
        f"{summary['activeModulePrimitiveLeafCount']} "
        f"distributions={summary['activeDistributionWrapperCount']} "
        "status=SOURCE_EVIDENCE_PARTIAL"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
