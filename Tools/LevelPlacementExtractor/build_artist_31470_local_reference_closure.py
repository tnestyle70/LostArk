#!/usr/bin/env python3

from __future__ import annotations

import argparse
import collections
import copy
import hashlib
import json
import sys
from pathlib import Path
from typing import Any, Iterable

from extract_ue3_particle_graph import extract_package
from extract_ue3_placements import LOSTARK_KR_AES_KEY


POINT_LIGHT_PACKAGE = "FX_CM_02"
POINT_LIGHT_SOURCE_NODE = "FX_CM_02:export:268"
POINT_LIGHT_PROPERTY = "pointlightcomponent"
POINT_LIGHT_PATH = (
    "light.par_mp_light_01.efparticlemoduletypedatalight_0."
    "pointlightcomponent_1335"
)
MESH_ROTATION_PACKAGE = "FX_PC_SDM_07"
MESH_ROTATION_SOURCE_NODE = "FX_PC_SDM_07:export:812"
MESH_ROTATION_PATH = (
    "par_v_smd_onestroke_swing_01.particlemodulemeshrotation_12."
    "efdistributionvectormultiplyparticleparameter_0"
)
COLOR_SCALE_PACKAGE = "FX_FS_AV_10"
COLOR_SCALE_SOURCE_NODE = "FX_FS_AV_10:export:165"
COLOR_SCALE_PATH = (
    "chunjie25.par_l_chunjie25_move_01."
    "particlemodulecolorscaleoverlife_22."
    "efdistributionvectormultiplyparticleparameter_0"
)
EXTERNAL_QUORUM_FIELDS = (
    "className",
    "objectName",
    "objectPath",
    "properties",
    "references",
    "propertyStreamEnd",
)


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


def source_package(receipt: dict[str, Any], logical_package: str) -> dict[str, Any]:
    matches = [
        row
        for row in receipt.get("sourcePackages", [])
        if str(row.get("logicalPackage", "")).casefold()
        == logical_package.casefold()
    ]
    require(len(matches) == 1, f"source package receipt is missing: {logical_package}")
    return matches[0]


def find_object(graph: dict[str, Any], object_path: str) -> dict[str, Any]:
    matches = [
        row
        for row in graph.get("objects", [])
        if str(row.get("objectPath", "")).casefold() == object_path.casefold()
    ]
    require(len(matches) == 1, f"decoded graph object is missing: {object_path}")
    return matches[0]


def record_sha256(value: dict[str, Any]) -> str:
    return canonical_sha256(value)


def distribution_references(properties: dict[str, Any]) -> list[dict[str, Any]]:
    references: list[dict[str, Any]] = []

    def visit(value: Any, path: str) -> None:
        if (
            isinstance(value, dict)
            and value.get("type") == "structproperty"
            and str(value.get("structType", "")).casefold()
            in {"rawdistributionfloat", "rawdistributionvector"}
        ):
            raw = value.get("value") or {}
            nested = raw.get("properties") if isinstance(raw, dict) else None
            distribution = (
                nested.get("distribution") if isinstance(nested, dict) else None
            )
            package_index = (
                distribution.get("value")
                if isinstance(distribution, dict)
                and distribution.get("type") == "objectproperty"
                else 0
            )
            if isinstance(package_index, int) and package_index != 0:
                references.append(
                    {
                        "propertyPath": path + ".distribution",
                        "sourcePackageIndex": package_index,
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
    return references


def semantic_projection(value: dict[str, Any]) -> dict[str, Any]:
    return {field: copy.deepcopy(value.get(field)) for field in EXTERNAL_QUORUM_FIELDS}


def particle_parameter_semantics(target: dict[str, Any] | None) -> dict[str, Any]:
    if target is None:
        return {
            "isParticleParameter": True,
            "decodedFields": [],
            "unresolvedRequiredFields": [
                "parametername",
                "parammodes",
                "parammodes[1]",
                "parammodes[2]",
                "maxinput",
                "maxoutput",
            ],
            "status": "UNRESOLVED_TARGET_PAYLOAD",
        }
    class_name = str(target.get("className", "")).casefold()
    is_parameter = "particleparameter" in class_name
    if not is_parameter:
        return {
            "isParticleParameter": False,
            "decodedFields": [],
            "unresolvedRequiredFields": [],
            "status": "NOT_APPLICABLE",
        }
    properties = target.get("properties") or {}
    vector = "vector" in class_name
    expected_modes = (
        ["parammodes", "parammodes[1]", "parammodes[2]"]
        if vector
        else ["parammode"]
    )
    required = ["parametername", *expected_modes, "maxinput", "maxoutput"]
    decoded = [name for name in required if name in properties]
    unresolved = [name for name in required if name not in properties]
    copied = {
        name: copy.deepcopy(properties[name])
        for name in (
            "parametername",
            "parammode",
            "parammodes",
            "parammodes[1]",
            "parammodes[2]",
            "maxinput",
            "maxoutput",
            "constant",
            "bisdirty",
        )
        if name in properties
    }
    return {
        "isParticleParameter": True,
        "decodedFields": decoded,
        "decodedValues": copied,
        "unresolvedRequiredFields": unresolved,
        "status": (
            "SOURCE_DECODED"
            if not unresolved
            else "UNRESOLVED_CLASS_DEFAULT_SEMANTICS"
        ),
    }


def build_indexes(
    graph: dict[str, Any], external: dict[str, Any]
) -> tuple[dict[str, dict[str, Any]], dict[str, dict[str, Any]], dict[str, dict[str, Any]], dict[str, dict[str, Any]]]:
    graph_by_id = {str(row["nodeId"]): row for row in graph.get("nodes", [])}
    graph_by_path = {
        str(row.get("objectPath", "")).casefold(): row
        for row in graph.get("nodes", [])
    }
    external_by_id: dict[str, dict[str, Any]] = {}
    external_by_path: dict[str, dict[str, Any]] = {}
    for package in external.get("packages", []):
        for row in package.get("objects", []):
            external_by_id[str(row["objectId"])] = row
            external_by_path[str(row.get("objectPath", "")).casefold()] = row
    return graph_by_id, graph_by_path, external_by_id, external_by_path


def reference_candidates(
    graph: dict[str, Any],
    external_by_id: dict[str, dict[str, Any]],
    source_system_id: str,
    source_document: str,
    source_id: str,
    property_path: str,
) -> list[dict[str, Any]]:
    if source_document == "normalizedGraph":
        result = [
            row
            for row in graph.get("edges", [])
            if row.get("sourceNodeId") == source_id
            and str(row.get("property", "")).casefold()
            == property_path.casefold()
        ]
        system = next(
            row
            for row in graph.get("sourceSystems", [])
            if row.get("sourceSystemId") == source_system_id
        )
        result.extend(
            row
            for row in system.get("unresolvedExternalReferences", [])
            if row.get("sourceNodeId") == source_id
            and str(row.get("property", "")).casefold()
            == property_path.casefold()
        )
        return result
    source = external_by_id[source_id]
    return [
        row
        for row in source.get("references", [])
        if str(row.get("property", "")).casefold() == property_path.casefold()
    ]


def active_distribution_reference_occurrences(
    inventory: dict[str, Any],
    graph: dict[str, Any],
    external: dict[str, Any],
) -> list[dict[str, Any]]:
    graph_by_id, _graph_by_path, external_by_id, _external_by_path = build_indexes(
        graph, external
    )
    result: list[dict[str, Any]] = []
    for element in inventory.get("activeElements", []):
        for module_order, evidence in enumerate(element.get("moduleEvidence", [])):
            source_document = str(evidence["sourceDocument"])
            source_id = str(evidence.get("nodeId") or evidence.get("objectId") or "")
            source = (
                graph_by_id.get(source_id)
                if source_document == "normalizedGraph"
                else external_by_id.get(source_id)
            )
            require(source is not None, f"active module source is missing: {source_id}")
            for reference in distribution_references(source.get("properties") or {}):
                candidates = reference_candidates(
                    graph,
                    external_by_id,
                    str(element["sourceSystemId"]),
                    source_document,
                    source_id,
                    str(reference["propertyPath"]),
                )
                require(
                    candidates,
                    "non-zero distribution target has no source reference evidence: "
                    f"{source_id}.{reference['propertyPath']}",
                )
                identities = {
                    (
                        int(row.get("packageIndex", 0)),
                        str(row.get("objectPath", "")).casefold(),
                        str(row.get("targetNodeId", "")),
                    )
                    for row in candidates
                }
                require(
                    len(identities) == 1,
                    f"distribution target evidence conflicts: {source_id}",
                )
                candidate = candidates[0]
                result.append(
                    {
                        "activeElementId": element["activeElementId"],
                        "sourceCueId": element["cueId"],
                        "rendererType": element["rendererType"],
                        "moduleOrder": module_order,
                        "sourceDocument": source_document,
                        "sourceNodeId": source_id,
                        "sourceModuleClass": source["className"],
                        "sourceModulePath": source["objectPath"],
                        "propertyPath": reference["propertyPath"],
                        "sourcePackageIndex": int(reference["sourcePackageIndex"]),
                        "targetNodeId": str(candidate.get("targetNodeId") or ""),
                        "targetObjectPath": str(candidate.get("objectPath") or ""),
                    }
                )
    return result


def group_occurrences(rows: Iterable[dict[str, Any]]) -> list[dict[str, Any]]:
    grouped: dict[tuple[str, str, int, str], list[dict[str, Any]]] = collections.defaultdict(list)
    for row in rows:
        key = (
            str(row["sourceNodeId"]),
            str(row["propertyPath"]).casefold(),
            int(row["sourcePackageIndex"]),
            str(row["targetObjectPath"]).casefold(),
        )
        grouped[key].append(row)
    return [
        {
            "sourceNodeId": key[0],
            "propertyPath": rows_for_key[0]["propertyPath"],
            "sourcePackageIndex": key[2],
            "targetNodeId": rows_for_key[0]["targetNodeId"],
            "targetObjectPath": rows_for_key[0]["targetObjectPath"],
            "sourceDocument": rows_for_key[0]["sourceDocument"],
            "sourceModuleClass": rows_for_key[0]["sourceModuleClass"],
            "sourceModulePath": rows_for_key[0]["sourceModulePath"],
            "activeOccurrences": [
                {
                    name: row[name]
                    for name in (
                        "activeElementId",
                        "sourceCueId",
                        "rendererType",
                        "moduleOrder",
                    )
                }
                for row in rows_for_key
            ],
        }
        for key, rows_for_key in sorted(grouped.items())
    ]


def build_closure(
    source_receipt_path: Path,
    active_inventory_path: Path,
    normalized_graph_path: Path,
    external_closure_path: Path,
    point_light_package_path: Path,
    mesh_rotation_recovery_package_path: Path,
    color_scale_package_path: Path,
) -> dict[str, Any]:
    source_receipt = load_json(source_receipt_path)
    inventory = load_json(active_inventory_path)
    graph = load_json(normalized_graph_path)
    external = load_json(external_closure_path)
    require(source_receipt.get("skillId") == 31470, "source receipt skill mismatch")
    require(inventory.get("skillId") == 31470, "active inventory skill mismatch")
    point_source = source_package(source_receipt, POINT_LIGHT_PACKAGE)
    rotation_source = source_package(source_receipt, MESH_ROTATION_PACKAGE)

    point_hash = file_sha256(point_light_package_path)
    point_bytes = point_light_package_path.stat().st_size
    require(
        point_light_package_path.name.casefold()
        == str(point_source["physicalPackage"]).casefold()
        and point_hash == point_source["sourcePackageSha256"]
        and point_bytes == int(point_source["sourcePackageBytes"]),
        "PointLight package does not match the immutable source receipt",
    )
    point_graph = extract_package(
        point_light_package_path, POINT_LIGHT_PACKAGE, LOSTARK_KR_AES_KEY
    )
    point_node = find_object(point_graph, POINT_LIGHT_PATH)
    point_properties = point_node["properties"]
    require(
        str(point_node["className"]).casefold() == "pointlightcomponent"
        and int(point_node["serialSize"]) == int(point_node["propertyStreamEnd"])
        and float(point_properties["brightness"]["value"]) == 10.0
        and point_properties["bcastcompositeshadow"]["value"] is False
        and point_properties["baffectcompositeshadowdirection"]["value"] is False,
        "PointLight exact payload changed",
    )

    rotation_hash = file_sha256(mesh_rotation_recovery_package_path)
    rotation_bytes = mesh_rotation_recovery_package_path.stat().st_size
    require(
        mesh_rotation_recovery_package_path.name.casefold()
        == str(rotation_source["physicalPackage"]).casefold()
        and (
            rotation_hash != rotation_source["sourcePackageSha256"]
            or rotation_bytes != int(rotation_source["sourcePackageBytes"])
        ),
        "mesh-rotation recovery package must remain a revision-mismatch lead",
    )
    rotation_graph = extract_package(
        mesh_rotation_recovery_package_path,
        MESH_ROTATION_PACKAGE,
        LOSTARK_KR_AES_KEY,
    )
    rotation_node = find_object(rotation_graph, MESH_ROTATION_PATH)
    require(
        str(rotation_node["className"]).casefold()
        == "efdistributionvectormultiplyparticleparameter"
        and int(rotation_node["serialSize"])
        == int(rotation_node["propertyStreamEnd"]),
        "mesh-rotation recovery payload is incomplete",
    )

    color_hash = file_sha256(color_scale_package_path)
    color_bytes = color_scale_package_path.stat().st_size
    require(
        color_scale_package_path.name.casefold()
        == "ygi3syh3sz23s81g1cmhufmhl.upk"
        and color_bytes == 23741
        and color_hash
        == "c163a79c1b679c76e2e4ac2607098a4ae24be6d4757e9465fe64b265fb4423c8",
        "ColorScale external package identity changed",
    )
    color_graph = extract_package(
        color_scale_package_path, COLOR_SCALE_PACKAGE, LOSTARK_KR_AES_KEY
    )
    color_node = find_object(color_graph, COLOR_SCALE_PATH)
    external_color_package = next(
        row
        for row in external["packages"]
        if row["logicalPackage"] == COLOR_SCALE_PACKAGE
    )
    current_color_by_path = {
        str(row["objectPath"]).casefold(): row for row in color_graph["objects"]
    }
    quorum = []
    for old in external_color_package["objects"]:
        current = current_color_by_path.get(str(old["objectPath"]).casefold())
        require(current is not None, f"ColorScale quorum object is missing: {old['objectPath']}")
        equal = semantic_projection(old) == semantic_projection(current)
        require(equal, f"ColorScale quorum object changed: {old['objectPath']}")
        quorum.append(
            {
                "objectId": old["objectId"],
                "objectPath": old["objectPath"],
                "priorClosureRecordSha256": record_sha256(semantic_projection(old)),
                "currentPackageRecordSha256": record_sha256(semantic_projection(current)),
                "semanticEqual": True,
            }
        )
    require(
        len(quorum) == 5
        and int(color_node["serialSize"]) == int(color_node["propertyStreamEnd"]),
        "ColorScale external package quorum is incomplete",
    )

    graph_by_id, graph_by_path, external_by_id, external_by_path = build_indexes(
        graph, external
    )
    occurrences = active_distribution_reference_occurrences(inventory, graph, external)
    grouped = group_occurrences(occurrences)
    references: list[dict[str, Any]] = []
    for index, row in enumerate(grouped):
        target = (
            graph_by_id.get(row["targetNodeId"])
            or graph_by_path.get(str(row["targetObjectPath"]).casefold())
            or external_by_path.get(str(row["targetObjectPath"]).casefold())
        )
        payload_status = "SOURCE_DECODED"
        source_revision: dict[str, Any] = {
            "basis": "PINNED_NORMALIZED_OR_EXTERNAL_CLOSURE",
        }
        if row["sourceNodeId"] == MESH_ROTATION_SOURCE_NODE:
            target = rotation_node
            payload_status = "UNRESOLVED_SOURCE_REVISION_MISMATCH"
            source_revision = {
                "basis": "RECOVERY_LEAD_ONLY",
                "expected": {
                    "physicalPackage": rotation_source["physicalPackage"],
                    "bytes": rotation_source["sourcePackageBytes"],
                    "sha256": rotation_source["sourcePackageSha256"],
                },
                "recoveryLead": {
                    "physicalPackage": mesh_rotation_recovery_package_path.name,
                    "bytes": rotation_bytes,
                    "sha256": rotation_hash,
                    "matchesSourceReceipt": False,
                    "objectPathMatchesOldGraphReference": True,
                },
            }
        elif row["sourceNodeId"] == COLOR_SCALE_SOURCE_NODE:
            target = color_node
            payload_status = "SOURCE_DECODED_EXTERNAL_PACKAGE_SEMANTIC_QUORUM"
            source_revision = {
                "basis": "CURRENT_PACKAGE_PIN_AND_PRIOR_CLOSURE_SEMANTIC_QUORUM",
                "physicalPackage": color_scale_package_path.name,
                "bytes": color_bytes,
                "sha256": color_hash,
                "priorRequestedObjectQuorum": quorum,
            }
        require(target is not None, f"distribution target is missing: {row}")
        semantics = particle_parameter_semantics(target)
        blockers: list[str] = []
        if payload_status.startswith("UNRESOLVED"):
            blockers.append("DISTRIBUTION_TARGET_SOURCE_REVISION_UNRESOLVED")
        if semantics["status"].startswith("UNRESOLVED"):
            blockers.append("PARTICLE_PARAMETER_CLASS_DEFAULT_SEMANTICS_UNRESOLVED")
        blockers.append("COMPILED_DISTRIBUTION_EXECUTION_NOT_IMPLEMENTED")
        references.append(
            {
                "referenceId": f"distribution-target-{index:03d}",
                "referenceKind": "DISTRIBUTION_TARGET",
                **row,
                "activeOccurrenceCount": len(row["activeOccurrences"]),
                "targetPayloadStatus": payload_status,
                "targetSourceRevision": source_revision,
                "target": {
                    "className": target["className"],
                    "objectPath": target["objectPath"],
                    "recordSha256": record_sha256(target),
                    "serialSize": target.get("serialSize"),
                    "propertyStreamEnd": target.get("propertyStreamEnd"),
                },
                "targetSemanticCoverage": semantics,
                "compiledExecutionAdmission": {
                    "allowed": False,
                    "blockers": sorted(set(blockers)),
                },
            }
        )

    light_occurrences = [
        {
            "activeElementId": row["activeElementId"],
            "sourceCueId": row["cueId"],
            "rendererType": row["rendererType"],
            "moduleOrder": module_order,
        }
        for row in inventory["activeElements"]
        for module_order, module in enumerate(row["moduleEvidence"])
        if (module.get("nodeId") or module.get("objectId"))
        == POINT_LIGHT_SOURCE_NODE
    ]
    require(len(light_occurrences) == 1, "active PointLight reference count changed")
    references.append(
        {
            "referenceId": "typedata-point-light-component-000",
            "referenceKind": "TYPEDATA_COMPONENT",
            "sourceNodeId": POINT_LIGHT_SOURCE_NODE,
            "propertyPath": POINT_LIGHT_PROPERTY,
            "sourcePackageIndex": 7055,
            "targetObjectPath": POINT_LIGHT_PATH,
            "activeOccurrences": light_occurrences,
            "activeOccurrenceCount": 1,
            "targetPayloadStatus": "SOURCE_DECODED",
            "targetSourceRevision": {
                "basis": "IMMUTABLE_SOURCE_RECEIPT_PACKAGE_MATCH",
                "physicalPackage": point_light_package_path.name,
                "bytes": point_bytes,
                "sha256": point_hash,
            },
            "target": {
                "className": point_node["className"],
                "objectPath": point_node["objectPath"],
                "recordSha256": record_sha256(point_node),
                "serialSize": point_node["serialSize"],
                "propertyStreamEnd": point_node["propertyStreamEnd"],
                "explicitProperties": {
                    name: copy.deepcopy(point_properties[name])
                    for name in (
                        "brightness",
                        "bcastcompositeshadow",
                        "baffectcompositeshadowdirection",
                        "lightguid",
                        "lightmapguid",
                    )
                },
            },
            "targetSemanticCoverage": {
                "decodedFields": sorted(point_properties),
                "unresolvedRequiredFields": [
                    "radius",
                    "falloffexponent",
                    "lightcolor",
                ],
                "status": "UNRESOLVED_CLASS_DEFAULT_SEMANTICS",
            },
            "compiledExecutionAdmission": {
                "allowed": False,
                "blockers": [
                    "POINT_LIGHT_CLASS_DEFAULTS_UNRESOLVED",
                    "LIGHT_RENDERER_NOT_COMPILED",
                ],
            },
        }
    )

    distribution_rows = [
        row for row in references if row["referenceKind"] == "DISTRIBUTION_TARGET"
    ]
    unresolved_distribution_rows = [
        row
        for row in distribution_rows
        if row["targetPayloadStatus"].startswith("UNRESOLVED")
    ]
    decoded_semantic_fields = collections.Counter()
    for row in distribution_rows:
        for field in row["targetSemanticCoverage"].get("decodedFields", []):
            if field == "maxinput":
                decoded_semantic_fields["MaxInput"] += 1
            elif field == "maxoutput":
                decoded_semantic_fields["MaxOutput"] += 1
            elif field.startswith("parammode"):
                decoded_semantic_fields["ParamMode"] += 1

    closure: dict[str, Any] = {
        "schema": "lostark.effect-local-reference-closure",
        "formatVersion": 2,
        "characterClass": "Artist",
        "skillId": 31470,
        "scope": "ACTIVE_FIRST_LOD_OBJECT_REFERENCES",
        "inputs": {
            "sourceReceiptSha256": file_sha256(source_receipt_path),
            "activeInventorySha256": file_sha256(active_inventory_path),
            "normalizedGraphSha256": file_sha256(normalized_graph_path),
            "externalModuleClosureSha256": file_sha256(external_closure_path),
        },
        "parserCapability": {
            "pointLightComponentClassIncluded": True,
            "efDistributionFloatClassIncluded": True,
            "efDistributionVectorClassIncluded": True,
            "normalizedGraphStatus": "OLDER_THAN_CURRENT_PARSER_CAPABILITY",
            "enumeration": "ALL_NONZERO_OBJECT_REFERENCES_NESTED_IN_ACTIVE_SELECTED_RAW_DISTRIBUTION_WRAPPERS",
        },
        "references": references,
        "summary": {
            "distributionTargetUniqueCount": len(distribution_rows),
            "distributionTargetOccurrenceCount": sum(
                row["activeOccurrenceCount"] for row in distribution_rows
            ),
            "distributionTargetPayloadDecodedUniqueCount": len(distribution_rows)
            - len(unresolved_distribution_rows),
            "distributionTargetPayloadUnresolvedUniqueCount": len(
                unresolved_distribution_rows
            ),
            "distributionTargetPayloadUnresolvedOccurrenceCount": sum(
                row["activeOccurrenceCount"] for row in unresolved_distribution_rows
            ),
            "pointLightTargetUniqueCount": 1,
            "pointLightTargetOccurrenceCount": 1,
            "decodedParticleParameterSemanticFieldCounts": dict(
                sorted(decoded_semantic_fields.items())
            ),
            "compiledExecutionAllowedReferenceCount": 0,
        },
        "closureSha256": "",
    }
    require(
        closure["summary"]["distributionTargetUniqueCount"] == 15
        and closure["summary"]["distributionTargetOccurrenceCount"] == 17
        and closure["summary"]["distributionTargetPayloadUnresolvedUniqueCount"]
        == 1
        and closure["summary"]["distributionTargetPayloadUnresolvedOccurrenceCount"]
        == 2,
        "active distribution-reference denominator changed",
    )
    # The 33 fields are the semantics that were already present in the pinned
    # graph/closure before the two supplemental target recoveries.  New
    # supplement fields stay in the per-reference rows and are not laundered
    # into this historical denominator.
    require(
        decoded_semantic_fields["MaxInput"] >= 7
        and decoded_semantic_fields["MaxOutput"] >= 7
        and decoded_semantic_fields["ParamMode"] >= 19,
        "ParticleParameter semantic-field coverage regressed",
    )
    unsigned = copy.deepcopy(closure)
    unsigned.pop("closureSha256")
    closure["closureSha256"] = canonical_sha256(unsigned)
    return closure


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Close or fail-close every Artist F active selected object reference."
    )
    parser.add_argument("--source-receipt", required=True, type=Path)
    parser.add_argument("--active-inventory", required=True, type=Path)
    parser.add_argument("--normalized-graph", required=True, type=Path)
    parser.add_argument("--external-module-closure", required=True, type=Path)
    parser.add_argument("--point-light-package", required=True, type=Path)
    parser.add_argument("--mesh-rotation-recovery-package", required=True, type=Path)
    parser.add_argument("--color-scale-package", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    closure = build_closure(
        args.source_receipt,
        args.active_inventory,
        args.normalized_graph,
        args.external_module_closure,
        args.point_light_package,
        args.mesh_rotation_recovery_package,
        args.color_scale_package,
    )
    content = json_bytes(closure)
    if args.check:
        require(
            args.output.is_file() and args.output.read_bytes() == content,
            f"generated output is stale: {args.output}",
        )
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(content)
    summary = closure["summary"]
    print(
        "Artist F 31470 local reference closure "
        f"{'check' if args.check else 'write'}: "
        f"distribution={summary['distributionTargetUniqueCount']}/"
        f"{summary['distributionTargetOccurrenceCount']} "
        f"unresolved={summary['distributionTargetPayloadUnresolvedUniqueCount']}/"
        f"{summary['distributionTargetPayloadUnresolvedOccurrenceCount']} "
        "pointLight=1/1 execution=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
