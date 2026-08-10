#!/usr/bin/env python3

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Any, Iterable


SCHEMA = "lostark.effect-source-semantic-closure"
FORMAT_VERSION = 1

EXECUTION_CONSUMED = "EXECUTION_CONSUMED"
VERIFIED_IRRELEVANT = "VERIFIED_IRRELEVANT"
UNRESOLVED = "UNRESOLVED"
DECISIONS = {EXECUTION_CONSUMED, VERIFIED_IRRELEVANT, UNRESOLVED}

SOURCE_EXACT = "SOURCE_EXACT"
CURRENT_REVISION_EVIDENCE = "CURRENT_REVISION_EVIDENCE"
RECONSTRUCTED_NUMERICALLY_VERIFIED = "RECONSTRUCTED_NUMERICALLY_VERIFIED"
RECONSTRUCTED_GRAPH = "RECONSTRUCTED_GRAPH"
SOURCE_UNRESOLVED = "UNRESOLVED"
SOURCE_FIDELITIES = {
    SOURCE_EXACT,
    CURRENT_REVISION_EVIDENCE,
    RECONSTRUCTED_NUMERICALLY_VERIFIED,
    RECONSTRUCTED_GRAPH,
    SOURCE_UNRESOLVED,
}

EXPECTED_DENOMINATORS = {
    "activeOccurrenceCount": 35,
    "selectedLodFieldCount": 70,
    "orderedModuleReferenceCount": 399,
    "topLevelTaggedPropertyCount": 1434,
    "primitiveLeafCount": 1572,
    "distributionCount": 629,
    "defaultDependentDistributionCount": 137,
    "operationReconstructedDistributionCount": 409,
    "lookupChunkReconstructedDistributionCount": 257,
    "lookupCountReconstructedDistributionCount": 257,
    "explicitRandomOperationDistributionCount": 82,
    "unapprovedClassAliasOccurrenceCount": 26,
    "localDistributionDefinitionCount": 15,
    "localDistributionOccurrenceCount": 17,
    "pointLightDefinitionCount": 1,
    "pointLightOccurrenceCount": 1,
    "externalNativeTailCount": 248,
    "seededModuleCount": 14,
    "requiredLocalSpaceDefaultCount": 8,
    "decalDefaultCount": 3,
    "ribbonDefaultCount": 1,
    "screenPostDefaultCount": 1,
    "lightDefaultCount": 1,
}

# These are canonical JSON hashes, not generated-output hashes.  Updating one is
# an explicit source-evidence review action; a self-signed mutation of an input
# artifact cannot silently redefine the Artist F semantic denominator.
FROZEN_INPUT_CANONICAL_SHA256 = {
    "activeInventory": "a1397ff82a2d3e1981e318aab1e36b3f6ff3cc08c3df14606fcfd777d36f4827",
    "normalizedGraph": "7daa2519dc943fe4b61341dcd1fd794c6db9d48a5ccaf015213f89de2fbc2b1f",
    "externalModuleClosure": "768a9a3c927913ba8a3c7d4ad02b2cfaa491271409fa94ae3c31c2f90e3a7b4a",
    "sourceEvidence": "a45e7548b442c7ee796eee088de660fb9804eb4ba03b960cd5e71cfc85a9276d",
    "localReferenceClosure": "9b0adf6efc2c0e16fd15b367f34c8b96027a41289df431f4fef56ae5012e77d3",
}

# No class alias is approved at this checkpoint.  In particular, neither the
# EF prefix nor the _seeded suffix is a normalization rule.  A future entry
# must carry a stable alias ID and an independently reviewed evidence receipt.
APPROVED_CLASS_ALIASES: dict[str, dict[str, str]] = {}

LOCAL_EVALUATOR_IDS = {
    "distributionfloatparticleparameter":
        "ue3.distribution.float-particle-parameter.v1",
    "distributionvectorparticleparameter":
        "ue3.distribution.vector-particle-parameter.v1",
    "distributionfloatconstantcurve":
        "ue3.distribution.float-constant-curve.v1",
    "efdistributionvectormultiplyparticleparameter":
        "lostark.ef.distribution.vector-multiply-particle-parameter.v1",
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, sort_keys=False) + "\n"
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(
            value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
        ).encode("utf-8")
    ).hexdigest()


def validate_self_hash(value: dict[str, Any], field: str) -> None:
    unsigned = copy.deepcopy(value)
    expected = str(unsigned.pop(field, ""))
    require(expected != "", f"source evidence has no {field}")
    require(
        canonical_sha256(unsigned) == expected,
        f"source evidence self hash is invalid: {field}",
    )


def validate_frozen_input(name: str, value: dict[str, Any]) -> None:
    expected = FROZEN_INPUT_CANONICAL_SHA256[name]
    require(
        canonical_sha256(value) == expected,
        f"Artist 31470 frozen semantic input changed: {name}",
    )


def sanitized_id(value: str) -> str:
    result = re.sub(r"[^a-z0-9]+", ".", value.casefold()).strip(".")
    require(result != "", f"stable semantic ID component is empty: {value}")
    return result


def legacy_normalized_class(exact_source_class: str) -> str:
    result = exact_source_class.casefold()
    if result.startswith("efparticlemodule"):
        result = "particlemodule" + result[len("efparticlemodule") :]
    if result.endswith("_seeded"):
        result = result[: -len("_seeded")]
    return result


def resolve_class_identity(exact_source_class: str) -> dict[str, str]:
    exact = exact_source_class.casefold()
    alias = APPROVED_CLASS_ALIASES.get(exact)
    if alias is None:
        return {
            "exactSourceClass": exact_source_class,
            "normalizedClass": exact,
            "aliasId": "",
            "aliasEvidenceId": "",
        }
    require(alias.get("aliasId", "") != "", f"approved alias has no ID: {exact}")
    require(
        alias.get("evidenceId", "") != "",
        f"approved alias has no evidence ID: {exact}",
    )
    return {
        "exactSourceClass": exact_source_class,
        "normalizedClass": alias["normalizedClass"],
        "aliasId": alias["aliasId"],
        "aliasEvidenceId": alias["evidenceId"],
    }


def source_fidelity_for_document(source_document: str) -> str:
    if source_document == "normalizedGraph":
        return SOURCE_EXACT
    if source_document == "externalModuleClosure":
        return CURRENT_REVISION_EVIDENCE
    raise ValueError(f"unknown source document: {source_document}")


def source_fidelity_for_local_definition(definition: dict[str, Any]) -> str:
    fidelity = str(definition.get("fidelity") or "")
    if fidelity == "SOURCE_EXACT_PHYSICAL_PACKAGE":
        return SOURCE_EXACT
    if fidelity in {
        "RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED",
        "PINNED_SOURCE_RECORD_PHYSICAL_ABSENT",
    }:
        return CURRENT_REVISION_EVIDENCE
    return SOURCE_UNRESOLVED


def source_fidelity_for_selected_field(selected: dict[str, Any] | None) -> str:
    if not isinstance(selected, dict):
        return SOURCE_UNRESOLVED
    status = str(selected.get("evidenceStatus") or "")
    if status.startswith("SOURCE_EXACT"):
        return SOURCE_EXACT
    if status.startswith("CURRENT_"):
        return CURRENT_REVISION_EVIDENCE
    return SOURCE_UNRESOLVED


def blocker_axes(
    *,
    evidence: Iterable[str] = (),
    artifact: Iterable[str] = (),
    execution: Iterable[str] = (),
    product: Iterable[str] = (),
) -> dict[str, list[str]]:
    return {
        "evidenceBlockers": sorted({str(row) for row in evidence if str(row)}),
        "artifactBindingBlockers": sorted(
            {str(row) for row in artifact if str(row)}
        ),
        "executionBlockers": sorted(
            {str(row) for row in execution if str(row)}
        ),
        "productBlockers": sorted(
            {
                "PRODUCT_ADMISSION_OWNED_BY_FINAL_INTEGRATION_GATE",
                *{str(row) for row in product if str(row)},
            }
        ),
    }


def classify_local_blockers(blockers: Iterable[str]) -> dict[str, list[str]]:
    evidence: list[str] = []
    artifact: list[str] = []
    execution: list[str] = []
    product: list[str] = []
    for blocker in blockers:
        token = str(blocker)
        if token.startswith(("COMPILED_", "CONSTANT_CURVE_COMPILER_")) or token in {
            "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN",
            "LIGHT_RENDERER_NOT_COMPILED",
            "RUNTIME_PARAMETER_SOURCE_CLOSURE_UNPROVEN",
        }:
            execution.append(token)
        elif token.startswith("PRODUCT_"):
            product.append(token)
        elif "PAYLOAD_HASH" in token or "ARTIFACT_BINDING" in token:
            artifact.append(token)
        else:
            evidence.append(token)
    return blocker_axes(
        evidence=evidence,
        artifact=artifact,
        execution=execution,
        product=product,
    )


def row_admission(axes: dict[str, list[str]]) -> dict[str, Any]:
    blockers = sorted(
        {
            *axes["artifactBindingBlockers"],
            *axes["executionBlockers"],
        }
    )
    return {"allowed": not blockers, "blockers": blockers}


def row_axis_statuses(axes: dict[str, list[str]]) -> dict[str, Any]:
    artifact_blockers = list(axes["artifactBindingBlockers"])
    product_blockers = sorted(
        {
            *artifact_blockers,
            *axes["executionBlockers"],
            *axes["productBlockers"],
        }
    )
    return {
        "artifactBindingIntegrity": {
            "verified": not artifact_blockers,
            "blockers": artifact_blockers,
        },
        "executionAdmission": row_admission(axes),
        "productAdmission": {
            "allowed": not product_blockers,
            "blockers": product_blockers,
        },
    }


def object_indexes(
    graph: dict[str, Any], external: dict[str, Any]
) -> tuple[dict[str, dict[str, Any]], dict[str, dict[str, Any]]]:
    graph_rows: dict[str, dict[str, Any]] = {}
    for row in graph.get("nodes", []):
        identity = str(row.get("nodeId") or "")
        require(identity and identity not in graph_rows, f"duplicate graph node: {identity}")
        graph_rows[identity] = row
    external_rows: dict[str, dict[str, Any]] = {}
    for package in external.get("packages", []):
        for row in package.get("objects", []):
            identity = str(row.get("objectId") or "")
            require(
                identity and identity not in external_rows,
                f"duplicate external object: {identity}",
            )
            external_rows[identity] = row
    return graph_rows, external_rows


def primitive_leaves(properties: dict[str, Any]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []

    def visit(value: Any, path: str, top_property: str) -> None:
        if (
            isinstance(value, dict)
            and value.get("type") == "structproperty"
            and str(value.get("structType", "")).casefold()
            in {"rawdistributionfloat", "rawdistributionvector"}
        ):
            return
        if isinstance(value, dict) and "type" in value and "value" in value:
            visit(value.get("value"), path, top_property)
            return
        if isinstance(value, (bool, int, float, str)):
            rows.append(
                {
                    "propertyPath": path,
                    "topLevelPropertyPath": top_property,
                    "valueKind": (
                        "boolean"
                        if isinstance(value, bool)
                        else "number"
                        if isinstance(value, (int, float))
                        else "string"
                    ),
                }
            )
            return
        if isinstance(value, list):
            for index, child in enumerate(value):
                visit(child, f"{path}[{index}]", top_property)
            return
        if isinstance(value, dict):
            for name, child in value.items():
                visit(child, f"{path}.{str(name).casefold()}", top_property)

    for name, value in properties.items():
        folded = str(name).casefold()
        visit(value, folded, folded)
    return rows


def raw_distribution_wrappers(properties: dict[str, Any]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []

    def visit(value: Any, path: str, top_property: str) -> None:
        if (
            isinstance(value, dict)
            and value.get("type") == "structproperty"
            and str(value.get("structType", "")).casefold()
            in {"rawdistributionfloat", "rawdistributionvector"}
        ):
            raw = value.get("value")
            rows.append(
                {
                    "propertyPath": path,
                    "topLevelPropertyPath": top_property,
                    "structType": str(value.get("structType") or "").casefold(),
                    "raw": copy.deepcopy(raw if isinstance(raw, dict) else {}),
                }
            )
            return
        if isinstance(value, dict) and "type" in value and "value" in value:
            visit(value.get("value"), path, top_property)
            return
        if isinstance(value, dict):
            for name, child in value.items():
                child_path = f"{path}.{str(name).casefold()}" if path else str(name).casefold()
                visit(child, child_path, top_property)
            return
        if isinstance(value, list):
            for index, child in enumerate(value):
                visit(child, f"{path}[{index}]", top_property)

    for name, value in properties.items():
        folded = str(name).casefold()
        visit(value, folded, folded)
    return rows


def raw_distribution_reference_index(wrapper: dict[str, Any]) -> int:
    raw = wrapper.get("raw")
    properties = raw.get("properties") if isinstance(raw, dict) else None
    distribution = (
        properties.get("distribution") if isinstance(properties, dict) else None
    )
    value = distribution.get("value") if isinstance(distribution, dict) else 0
    return value if isinstance(value, int) else 0


def raw_distribution_field_evidence(
    wrapper: dict[str, Any], raw_source_fidelity: str
) -> dict[str, Any]:
    require(
        raw_source_fidelity in SOURCE_FIDELITIES,
        "raw distribution source fidelity is invalid",
    )
    raw = wrapper.get("raw")
    properties = raw.get("properties") if isinstance(raw, dict) else None
    fields = properties if isinstance(properties, dict) else {}
    present = sorted(str(name).casefold() for name in fields)
    lookup = fields.get("lookuptable")
    lookup_value = lookup.get("value") if isinstance(lookup, dict) else None
    has_lookup_payload = isinstance(lookup_value, list) and bool(lookup_value)
    reconstructed = []
    if "op" not in fields:
        reconstructed.append("operation")
    if has_lookup_payload and "lookuptablechunksize" not in fields:
        reconstructed.append("lookupTableChunkSize")
    if has_lookup_payload and "lookuptablenumelements" not in fields:
        reconstructed.append("lookupTableNumElements")
    operation = fields.get("op")
    operation_value = (
        operation.get("value") if isinstance(operation, dict) else None
    )
    return {
        "rawFieldNames": present,
        "reconstructedFieldNames": reconstructed,
        "hasLookupPayload": has_lookup_payload,
        "defaultDependent": (
            raw_distribution_reference_index(wrapper) == 0
            and not has_lookup_payload
        ),
        "explicitOperation": (
            operation_value if isinstance(operation_value, int) else None
        ),
        "rawFieldSourceFidelity": raw_source_fidelity,
        "reconstructedFieldSourceFidelity": (
            RECONSTRUCTED_GRAPH if reconstructed else ""
        ),
        "numericExecutionEvidenceStatus": "UNVERIFIED",
    }


def qualified_object_identity(value: str) -> tuple[str, str]:
    logical, separator, local_path = value.partition(".")
    require(
        bool(separator and logical and local_path),
        f"module object path is not package-qualified: {value}",
    )
    return logical.casefold(), local_path.casefold()


def property_identity_key(
    active_element_id: str,
    logical_package: str,
    package_local_path: str,
    property_path: str,
) -> tuple[str, str, str, str]:
    return (
        active_element_id,
        logical_package.casefold(),
        package_local_path.casefold(),
        property_path.casefold(),
    )


def local_reference_indexes(
    local_references: dict[str, Any],
) -> tuple[
    dict[str, dict[str, Any]],
    dict[tuple[str, str, str, str], dict[str, Any]],
    dict[str, dict[str, Any]],
    dict[tuple[str, str, str, str], dict[str, Any]],
]:
    def definitions(name: str) -> dict[str, dict[str, Any]]:
        result: dict[str, dict[str, Any]] = {}
        for row in local_references.get(name, []):
            identity = str(row.get("definitionId") or "")
            require(identity and identity not in result, f"duplicate {name}: {identity}")
            result[identity] = row
        return result

    def occurrences(name: str) -> dict[tuple[str, str, str, str], dict[str, Any]]:
        result: dict[tuple[str, str, str, str], dict[str, Any]] = {}
        for row in local_references.get(name, []):
            identity = row.get("propertyIdentity") or {}
            key = property_identity_key(
                str(row.get("activeElementId") or ""),
                str(identity.get("logicalPackage") or ""),
                str(identity.get("packageLocalPath") or ""),
                str(identity.get("propertyPath") or ""),
            )
            require(all(key) and key not in result, f"duplicate {name}: {key}")
            result[key] = row
        return result

    return (
        definitions("distributionDefinitions"),
        occurrences("distributionOccurrences"),
        definitions("componentDefinitions"),
        occurrences("componentOccurrences"),
    )


def build_local_distribution_binding(
    *,
    definition: dict[str, Any],
    occurrence: dict[str, Any],
    module_occurrence_id: str,
    distribution_id: str,
    raw_property_path: str,
    field_evidence: dict[str, Any],
) -> dict[str, Any]:
    semantic = definition.get("semanticCoverage") or {}
    status = str(semantic.get("status") or "")
    require(
        status in {"SEMANTIC_SOURCE_READY", "SEMANTIC_BLOCKED"},
        f"local distribution semantic status is invalid: {definition.get('definitionId')}",
    )
    source_class = str(definition.get("sourceClass") or "")
    require(source_class != "", "local distribution source class is empty")
    evaluator_id = LOCAL_EVALUATOR_IDS.get(source_class.casefold(), "")
    require(evaluator_id != "", f"local distribution evaluator ID is unknown: {source_class}")
    source_blockers = sorted(
        {
            *list((definition.get("executionAdmission") or {}).get("blockers") or []),
            *list((occurrence.get("executionAdmission") or {}).get("blockers") or []),
        }
    )
    axes = classify_local_blockers(source_blockers)
    decision = UNRESOLVED
    axes["executionBlockers"] = sorted(
        {
            *axes["executionBlockers"],
            "DOWNSTREAM_EVALUATOR_RECEIPT_REQUIRED",
            "INDEPENDENT_NUMERIC_ORACLE_REQUIRED",
        }
    )
    reconstructed_blockers = {
        "operation": "DISTRIBUTION_OPERATION_RECONSTRUCTION_UNVERIFIED",
        "lookupTableChunkSize":
            "DISTRIBUTION_LOOKUP_CHUNK_RECONSTRUCTION_UNVERIFIED",
        "lookupTableNumElements":
            "DISTRIBUTION_LOOKUP_COUNT_RECONSTRUCTION_UNVERIFIED",
    }
    axes["executionBlockers"] = sorted(
        {
            *axes["executionBlockers"],
            *(
                reconstructed_blockers[name]
                for name in field_evidence["reconstructedFieldNames"]
            ),
        }
    )
    if field_evidence["explicitOperation"] in {2, 3}:
        axes["executionBlockers"] = sorted(
            {
                *axes["executionBlockers"],
                "DISTRIBUTION_RANDOM_STREAM_SAMPLE_ORDER_UNVERIFIED",
            }
        )
    oracle_status = (
        "STRUCTURAL_BRANCH_BOUND_NUMERIC_ORACLE_PENDING"
        if status == "SEMANTIC_SOURCE_READY"
        else "UNVERIFIED_FAIL_CLOSED"
    )
    row = {
        "legacyReferenceId": str(definition.get("referenceId") or ""),
        "legacyDefinitionId": str(definition.get("definitionId") or ""),
        "legacyOccurrenceId": str(occurrence.get("occurrenceId") or ""),
        "moduleOccurrenceId": module_occurrence_id,
        "distributionId": distribution_id,
        "propertyPath": raw_property_path,
        "sourceReferencePropertyPath": str(
            (occurrence.get("propertyIdentity") or {}).get("propertyPath") or ""
        ),
        "exactSourceClass": source_class,
        "evaluatorId": evaluator_id,
        "evaluatorVersion": 1,
        "oracleStatus": oracle_status,
        "fieldEvidence": copy.deepcopy(field_evidence),
        "sourceFidelity": source_fidelity_for_local_definition(definition),
        "classification": decision,
        **axes,
    }
    identity = occurrence.get("propertyIdentity") or {}
    definition_identity = (
        f"{str(identity.get('logicalPackage') or '').casefold()}::"
        f"{str(definition.get('targetPackageLocalPath') or '').casefold()}"
    )
    row["definitionId"] = "distribution-definition::" + definition_identity
    row["referenceId"] = "distribution-reference::" + definition_identity
    row["occurrenceId"] = distribution_id
    row.update(row_axis_statuses(axes))
    return row


def build_point_light_binding(
    *,
    definition: dict[str, Any],
    occurrence: dict[str, Any],
    module_occurrence_id: str,
) -> dict[str, Any]:
    exact_payload = definition.get("exactPayload") or {}
    exact_class = str(exact_payload.get("className") or "")
    require(exact_class.casefold() == "pointlightcomponent", "PointLight class changed")
    resolved = (definition.get("semanticCoverage") or {}).get("resolvedFields") or {}
    fields = []
    for name in (
        "brightness",
        "bcastcompositeshadow",
        "baffectcompositeshadowdirection",
        "radius",
        "falloffexponent",
        "lightcolor",
    ):
        row = resolved.get(name)
        require(isinstance(row, dict), f"PointLight field is missing: {name}")
        selected = row.get("selected")
        fidelity = source_fidelity_for_selected_field(selected)
        if fidelity == SOURCE_EXACT:
            axes = blocker_axes(
                execution=[
                    "DOWNSTREAM_LIGHT_HANDLER_RECEIPT_REQUIRED",
                    "INDEPENDENT_LIGHT_NUMERIC_OR_SEMANTIC_ORACLE_REQUIRED",
                ]
            )
        else:
            axes = blocker_axes(
                evidence=[
                    "POINT_LIGHT_SOURCE_ERA_DEFAULT_PROVENANCE_UNRESOLVED",
                    "SOURCE_ERA_SCRIPT_PACKAGE_IDENTITY_NOT_PINNED",
                ],
                execution=["POINT_LIGHT_DEFAULT_FIELD_EXECUTION_UNRESOLVED"],
            )
        field = {
            "fieldId": f"typedata-point-light-component-000/field:{name}",
            "fieldName": name,
            "selectedTier": str((selected or {}).get("tier") or ""),
            "selectedEvidenceStatus": str(
                (selected or {}).get("evidenceStatus") or ""
            ),
            "sourceFidelity": fidelity,
            "classification": UNRESOLVED,
            "consumerId": "source.point-light." + sanitized_id(name) + ".v1",
            **axes,
        }
        field.update(row_axis_statuses(axes))
        fields.append(field)
    explicit_properties = exact_payload.get("explicitProperties") or {}
    for name in ("lightguid", "lightmapguid"):
        require(name in explicit_properties, f"PointLight GUID field is missing: {name}")
        guid_axes = blocker_axes(
            execution=[
                "POINT_LIGHT_GUID_RUNTIME_SEMANTICS_OR_IRRELEVANCE_UNPROVEN"
            ]
        )
        field = {
            "fieldId": f"typedata-point-light-component-000/field:{name}",
            "fieldName": name,
            "selectedTier": "INSTANCE_EXPLICIT",
            "selectedEvidenceStatus": "SOURCE_EXACT_PHYSICAL_PACKAGE",
            "sourceFidelity": SOURCE_EXACT,
            "classification": UNRESOLVED,
            "consumerId": "",
            **guid_axes,
        }
        field.update(row_axis_statuses(guid_axes))
        fields.append(field)
    axes = classify_local_blockers(
        {
            *list((definition.get("executionAdmission") or {}).get("blockers") or []),
            *list((occurrence.get("executionAdmission") or {}).get("blockers") or []),
        }
    )
    result = {
        "legacyReferenceId": str(definition.get("referenceId") or ""),
        "legacyDefinitionId": str(definition.get("definitionId") or ""),
        "legacyOccurrenceId": str(occurrence.get("occurrenceId") or ""),
        "moduleOccurrenceId": module_occurrence_id,
        "exactSourceClass": exact_class,
        "componentBinderId": "ue3.typedata.point-light-component.v1",
        "sourceFidelity": SOURCE_EXACT,
        "classification": UNRESOLVED,
        "fields": fields,
        **axes,
    }
    identity = occurrence.get("propertyIdentity") or {}
    definition_identity = (
        f"{str(identity.get('logicalPackage') or '').casefold()}::"
        f"{str(definition.get('targetPackageLocalPath') or '').casefold()}"
    )
    result["definitionId"] = "component-definition::" + definition_identity
    result["referenceId"] = "component-reference::" + definition_identity
    result["occurrenceId"] = (
        f"{module_occurrence_id}::component:pointlightcomponent"
    )
    result.update(row_axis_statuses(axes))
    return result


def build_selected_lod_semantics(
    evidence: dict[str, Any], occurrence_id: str
) -> dict[str, Any]:
    require(
        evidence.get("lodSelectionPolicy") == "FIRST_LOD_ONLY",
        f"selected LOD policy changed: {occurrence_id}",
    )
    selected = evidence.get("selectedLod") or {}
    require(
        selected.get("sourceLodPath")
        and selected.get("sourceLodNodeId")
        and selected.get("sourceLodRecordSha256"),
        f"selected LOD identity is incomplete: {occurrence_id}",
    )
    fields = []
    for field_name, provenance_name, blocker in (
        (
            "level",
            "levelProvenance",
            "SELECTED_LOD_LEVEL_CLASS_DEFAULT_UNRESOLVED",
        ),
        (
            "enabled",
            "enabledProvenance",
            "SELECTED_LOD_ENABLED_CLASS_DEFAULT_UNRESOLVED",
        ),
    ):
        require(
            selected.get(field_name) is None
            and selected.get(provenance_name) == "UNRESOLVED_CLASS_DEFAULT",
            f"selected LOD default evidence changed: {occurrence_id}:{field_name}",
        )
        axes = blocker_axes(
            evidence=["SOURCE_ERA_LOD_CLASS_DEFAULT_EVIDENCE_UNAVAILABLE"],
            execution=[blocker],
        )
        row = {
            "fieldId": f"{occurrence_id}::selected-lod:{field_name}",
            "fieldName": field_name,
            "encodedValue": None,
            "provenance": "UNRESOLVED_CLASS_DEFAULT",
            "sourceFidelity": SOURCE_UNRESOLVED,
            "classification": UNRESOLVED,
            "evaluatorId": "",
            "oracleStatus": "UNVERIFIED_FAIL_CLOSED",
            **axes,
        }
        row.update(row_axis_statuses(axes))
        fields.append(row)
    return {
        "sourceLodPath": str(selected["sourceLodPath"]),
        "sourceLodNodeId": str(selected["sourceLodNodeId"]),
        "sourceLodRecordSha256": str(selected["sourceLodRecordSha256"]),
        "emitterArrayIndex": int(selected.get("emitterArrayIndex", -1)),
        "fields": fields,
    }


def build_semantic_closure(
    active_inventory: dict[str, Any],
    normalized_graph: dict[str, Any],
    external_module_closure: dict[str, Any],
    source_evidence: dict[str, Any],
    local_reference_closure: dict[str, Any],
) -> dict[str, Any]:
    inputs = {
        "activeInventory": active_inventory,
        "normalizedGraph": normalized_graph,
        "externalModuleClosure": external_module_closure,
        "sourceEvidence": source_evidence,
        "localReferenceClosure": local_reference_closure,
    }
    for name, value in inputs.items():
        validate_frozen_input(name, value)
    validate_self_hash(source_evidence, "evidenceSha256")
    validate_self_hash(local_reference_closure, "closureSha256")
    require(source_evidence.get("skillId") == 31470, "source evidence skill changed")
    require(
        source_evidence.get("scope") == "ACTIVE_OCCURRENCES_FIRST_LOD_ONLY",
        "source evidence scope changed",
    )

    graph_rows, external_rows = object_indexes(
        normalized_graph, external_module_closure
    )
    (
        distribution_definitions,
        distribution_occurrences,
        component_definitions,
        component_occurrences,
    ) = local_reference_indexes(local_reference_closure)
    renderer_by_element = {
        str(row["activeElementId"]): str(row["rendererType"])
        for row in active_inventory.get("activeElements", [])
    }
    require(
        len(renderer_by_element) == EXPECTED_DENOMINATORS["activeOccurrenceCount"],
        "active inventory occurrence identity changed",
    )

    occurrences: list[dict[str, Any]] = []
    local_distribution_bindings: list[dict[str, Any]] = []
    point_light_bindings: list[dict[str, Any]] = []
    consumed_local_occurrences: set[str] = set()
    module_counts: Counter[str] = Counter()
    property_counts: Counter[str] = Counter()
    leaf_counts: Counter[str] = Counter()
    distribution_counts: Counter[str] = Counter()
    native_tail_counts: Counter[str] = Counter()
    seed_counts: Counter[str] = Counter()
    default_counts: Counter[str] = Counter()
    source_fidelity_counts: Counter[str] = Counter()
    selected_lod_field_counts: Counter[str] = Counter()
    distribution_field_evidence_counts: Counter[str] = Counter()
    unapproved_class_alias_occurrence_count = 0

    for evidence in source_evidence.get("occurrences", []):
        evidence_id = str(evidence.get("evidenceId") or "")
        source_occurrence_id = str(evidence.get("sourceOccurrenceId") or "")
        source_system_id = str(evidence.get("sourceSystemId") or "")
        renderer_type = renderer_by_element.get(evidence_id, "")
        require(
            evidence_id and source_occurrence_id and source_system_id and renderer_type,
            f"source occurrence composite identity is incomplete: {evidence_id}",
        )
        source_emitter_path = str(evidence.get("sourceEmitterPath") or "")
        require(source_emitter_path != "", f"source emitter path is empty: {evidence_id}")
        occurrence_id = (
            f"{source_system_id}::{source_occurrence_id}::{source_emitter_path}"
        )
        selected_lod_semantics = build_selected_lod_semantics(
            evidence, occurrence_id
        )
        selected_lod_module_execution_blockers = {
            blocker
            for field in selected_lod_semantics["fields"]
            for blocker in field["executionBlockers"]
        }
        selected_lod_field_counts.update(
            row["classification"] for row in selected_lod_semantics["fields"]
        )
        occurrence_modules: list[dict[str, Any]] = []
        for expected_order, source_module in enumerate(
            evidence.get("moduleReferenceOrder", [])
        ):
            order = int(source_module.get("order", -1))
            require(order == expected_order, f"module order changed: {occurrence_id}")
            source_document = str(source_module.get("sourceDocument") or "")
            object_id = str(source_module.get("sourceObjectId") or "")
            if source_document == "normalizedGraph":
                raw = graph_rows.get(object_id)
            elif source_document == "externalModuleClosure":
                raw = external_rows.get(object_id)
            else:
                raw = None
            require(raw is not None, f"source module record is missing: {object_id}")
            require(
                canonical_sha256(raw) == source_module.get("sourceRecordSha256"),
                f"source module record hash changed: {object_id}",
            )
            exact_source_class = str(raw.get("className") or "")
            require(
                exact_source_class == str(source_module.get("sourceClass") or ""),
                f"exact source class changed: {object_id}",
            )
            class_identity = resolve_class_identity(exact_source_class)
            module_occurrence_id = f"{occurrence_id}::module:{order:03d}"
            properties = raw.get("properties") or {}
            require(isinstance(properties, dict), f"module properties changed: {object_id}")
            leaves = primitive_leaves(properties)
            wrappers = raw_distribution_wrappers(properties)
            require(
                len(properties)
                == int(source_module.get("topLevelTaggedPropertyCount", -1)),
                f"top property denominator changed: {module_occurrence_id}",
            )
            require(
                len(leaves) == int(source_module.get("primitiveLeafCount", -1)),
                f"primitive leaf denominator changed: {module_occurrence_id}",
            )
            require(
                len(wrappers) == int(source_module.get("distributionWrapperCount", -1)),
                f"distribution denominator changed: {module_occurrence_id}",
            )
            logical_package = object_id.partition(":")[0].casefold()
            package_local_path = str(raw.get("objectPath") or "").casefold()
            require(
                logical_package and package_local_path,
                f"source module package identity is incomplete: {object_id}",
            )
            module_fidelity = source_fidelity_for_document(source_document)
            source_fidelity_counts[module_fidelity] += 1

            distribution_rows: list[dict[str, Any]] = []
            distributions_by_top: dict[str, list[dict[str, Any]]] = {}
            for wrapper in wrappers:
                property_path = str(wrapper["propertyPath"])
                distribution_id = (
                    f"{module_occurrence_id}::distribution:{property_path}"
                )
                occurrence = distribution_occurrences.get(
                    property_identity_key(
                        evidence_id,
                        logical_package,
                        package_local_path,
                        property_path + ".distribution",
                    )
                )
                package_index = raw_distribution_reference_index(wrapper)
                if occurrence is None:
                    require(
                        package_index == 0,
                        f"nonzero distribution reference is not bound: {distribution_id}",
                    )
                    field_evidence = raw_distribution_field_evidence(
                        wrapper, module_fidelity
                    )
                    execution_blockers = {
                        "DOWNSTREAM_EVALUATOR_RECEIPT_REQUIRED",
                        "INDEPENDENT_NUMERIC_ORACLE_REQUIRED",
                    }
                    if field_evidence["defaultDependent"]:
                        execution_blockers.add(
                            "DISTRIBUTION_CLASS_DEFAULT_VALUE_UNRESOLVED"
                        )
                    reconstructed_blockers = {
                        "operation":
                            "DISTRIBUTION_OPERATION_RECONSTRUCTION_UNVERIFIED",
                        "lookupTableChunkSize":
                            "DISTRIBUTION_LOOKUP_CHUNK_RECONSTRUCTION_UNVERIFIED",
                        "lookupTableNumElements":
                            "DISTRIBUTION_LOOKUP_COUNT_RECONSTRUCTION_UNVERIFIED",
                    }
                    execution_blockers.update(
                        reconstructed_blockers[name]
                        for name in field_evidence["reconstructedFieldNames"]
                    )
                    if field_evidence["explicitOperation"] in {2, 3}:
                        execution_blockers.add(
                            "DISTRIBUTION_RANDOM_STREAM_SAMPLE_ORDER_UNVERIFIED"
                        )
                    axes = blocker_axes(
                        evidence=(
                            ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                            if module_fidelity != SOURCE_EXACT
                            else []
                        ),
                        execution=execution_blockers,
                    )
                    distribution = {
                        "distributionId": distribution_id,
                        "definitionId": (
                            "distribution-definition::"
                            f"{object_id.casefold()}::{property_path.casefold()}"
                        ),
                        "referenceId": (
                            "distribution-reference::"
                            f"{object_id.casefold()}::{property_path.casefold()}"
                        ),
                        "occurrenceId": distribution_id,
                        "propertyPath": property_path,
                        "topLevelPropertyPath": wrapper["topLevelPropertyPath"],
                        "exactSourceClass": "",
                        "evaluatorId": "ue3.raw-distribution.cooked-lookup.v1",
                        "evaluatorVersion": 1,
                        "oracleStatus": "UNVERIFIED_FAIL_CLOSED",
                        "fieldEvidence": field_evidence,
                        "sourceFidelity": module_fidelity,
                        "classification": UNRESOLVED,
                        **axes,
                    }
                    distribution.update(row_axis_statuses(axes))
                else:
                    definition_id = str(occurrence.get("definitionId") or "")
                    definition = distribution_definitions.get(definition_id)
                    require(
                        definition is not None,
                        f"local distribution definition is missing: {definition_id}",
                    )
                    distribution = build_local_distribution_binding(
                        definition=definition,
                        occurrence=occurrence,
                        module_occurrence_id=module_occurrence_id,
                        distribution_id=distribution_id,
                        raw_property_path=property_path,
                        field_evidence=raw_distribution_field_evidence(
                            wrapper, module_fidelity
                        ),
                    )
                    distribution["topLevelPropertyPath"] = str(
                        wrapper["topLevelPropertyPath"]
                    )
                    require(
                        distribution["sourceReferencePropertyPath"].casefold()
                        == (property_path + ".distribution").casefold(),
                        f"local distribution property changed: {distribution_id}",
                    )
                    consumed_local_occurrences.add(
                        distribution["legacyOccurrenceId"]
                    )
                    local_distribution_bindings.append(copy.deepcopy(distribution))
                distribution_rows.append(distribution)
                field_evidence = distribution["fieldEvidence"]
                if field_evidence["defaultDependent"]:
                    distribution_field_evidence_counts["defaultDependent"] += 1
                for name in field_evidence["reconstructedFieldNames"]:
                    distribution_field_evidence_counts[
                        "reconstructed:" + name
                    ] += 1
                if field_evidence["explicitOperation"] in {2, 3}:
                    distribution_field_evidence_counts[
                        "explicitRandomOperation"
                    ] += 1
                distributions_by_top.setdefault(
                    str(wrapper["topLevelPropertyPath"]), []
                ).append(distribution)
                distribution_counts[distribution["classification"]] += 1

            property_rows: list[dict[str, Any]] = []
            property_by_path: dict[str, dict[str, Any]] = {}
            for name in properties:
                property_path = str(name).casefold()
                nested_distributions = distributions_by_top.get(property_path, [])
                if property_path == "randomseedinfo":
                    axes = blocker_axes(
                        evidence=(
                            ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                            if module_fidelity != SOURCE_EXACT
                            else []
                        ),
                        execution=["RANDOM_SEED_CONSUMPTION_SEMANTICS_UNRESOLVED"],
                    )
                    consumer_id = ""
                    oracle_id = ""
                elif nested_distributions:
                    axes = blocker_axes(
                        evidence=(
                            blocker
                            for row in nested_distributions
                            for blocker in row["evidenceBlockers"]
                        ),
                        artifact=(
                            blocker
                            for row in nested_distributions
                            for blocker in row["artifactBindingBlockers"]
                        ),
                        execution=(
                            blocker
                            for row in nested_distributions
                            for blocker in row["executionBlockers"]
                        ),
                    )
                    axes["executionBlockers"] = sorted(
                        {
                            *axes["executionBlockers"],
                            "DOWNSTREAM_PROPERTY_HANDLER_RECEIPT_REQUIRED",
                        }
                    )
                    consumer_id = (
                        "source.property."
                        + sanitized_id(exact_source_class)
                        + "."
                        + sanitized_id(property_path)
                        + ".v1"
                    )
                    oracle_id = ""
                elif property_path == "b3ddrawmode":
                    axes = blocker_axes(
                        evidence=(
                            ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                            if module_fidelity != SOURCE_EXACT
                            else []
                        ),
                        execution=["EDITOR_ONLY_IRRELEVANCE_PROOF_REQUIRED"],
                    )
                    consumer_id = ""
                    oracle_id = ""
                else:
                    axes = blocker_axes(
                        evidence=(
                            ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                            if module_fidelity != SOURCE_EXACT
                            else []
                        ),
                        execution=["DOWNSTREAM_PROPERTY_HANDLER_RECEIPT_REQUIRED"],
                    )
                    consumer_id = (
                        "source.property."
                        + sanitized_id(exact_source_class)
                        + "."
                        + sanitized_id(property_path)
                        + ".v1"
                    )
                    oracle_id = ""
                decision = UNRESOLVED
                prop = {
                    "propertyId": f"{module_occurrence_id}::property:{property_path}",
                    "propertyPath": property_path,
                    "sourceFidelity": module_fidelity,
                    "classification": decision,
                    "consumerId": consumer_id,
                    "irrelevanceOracleId": oracle_id,
                    "distributionIds": [
                        row["distributionId"] for row in nested_distributions
                    ],
                    **axes,
                }
                prop.update(row_axis_statuses(axes))
                property_rows.append(prop)
                property_by_path[property_path] = prop
                property_counts[decision] += 1

            leaf_rows: list[dict[str, Any]] = []
            for leaf in leaves:
                parent = property_by_path[leaf["topLevelPropertyPath"]]
                row = {
                    "leafId": (
                        f"{module_occurrence_id}::leaf:{leaf['propertyPath']}"
                    ),
                    **leaf,
                    "sourceFidelity": parent["sourceFidelity"],
                    "classification": parent["classification"],
                    "consumerId": parent["consumerId"],
                    "irrelevanceOracleId": parent["irrelevanceOracleId"],
                    "evidenceBlockers": copy.deepcopy(parent["evidenceBlockers"]),
                    "artifactBindingBlockers": copy.deepcopy(
                        parent["artifactBindingBlockers"]
                    ),
                    "executionBlockers": copy.deepcopy(parent["executionBlockers"]),
                    "productBlockers": copy.deepcopy(parent["productBlockers"]),
                    "executionAdmission": copy.deepcopy(
                        parent["executionAdmission"]
                    ),
                    "artifactBindingIntegrity": copy.deepcopy(
                        parent["artifactBindingIntegrity"]
                    ),
                    "productAdmission": copy.deepcopy(
                        parent["productAdmission"]
                    ),
                }
                leaf_rows.append(row)
                leaf_counts[row["classification"]] += 1

            native_tail_status = str(source_module.get("nativeTailStatus") or "")
            if native_tail_status == "SOURCE_DECODED_NO_NATIVE_TAIL":
                native_axes = blocker_axes()
                native_tail = {
                    "classification": VERIFIED_IRRELEVANT,
                    "oracleId": "source.record.serial-size-equals-property-stream-end.v1",
                    "status": native_tail_status,
                    **native_axes,
                }
            else:
                require(
                    native_tail_status.startswith("UNRESOLVED"),
                    f"native-tail status changed: {module_occurrence_id}",
                )
                native_axes = blocker_axes(
                    evidence=["EXTERNAL_MODULE_NATIVE_TAIL_BYTES_UNAVAILABLE"],
                    execution=["EXTERNAL_MODULE_NATIVE_TAIL_NOT_PROVEN"],
                )
                native_tail = {
                    "classification": UNRESOLVED,
                    "oracleId": "",
                    "status": native_tail_status,
                    **native_axes,
                }
            native_tail.update(row_axis_statuses(native_axes))
            native_tail_counts[native_tail["classification"]] += 1

            seed_status = str(source_module.get("randomSeedStatus") or "")
            seed = None
            if seed_status != "NOT_SEEDED":
                require(seed_status != "", f"seed status is empty: {module_occurrence_id}")
                seed_axes = blocker_axes(
                    evidence=(
                        ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                        if module_fidelity != SOURCE_EXACT
                        else []
                    ),
                    execution=[
                        "RANDOM_SEED_CONSUMPTION_SEMANTICS_UNRESOLVED",
                        (
                            "RANDOM_SEED_OPAQUE_PAYLOAD_UNRESOLVED"
                            if seed_status == "OPAQUE_HEX_METADATA_ONLY"
                            else "RANDOM_SEED_SELECTION_POLICY_UNRESOLVED"
                            if seed_status
                            == "SEED_ARRAY_SOURCE_DECODED_CONSUMPTION_UNRESOLVED"
                            else "RANDOM_SEED_CLASS_DEFAULT_UNRESOLVED"
                        ),
                    ],
                )
                seed = {
                    "classification": UNRESOLVED,
                    "evaluatorId": "ue3.particle-random-seed-info.v1",
                    "evaluatorVersion": 1,
                    "oracleStatus": "UNVERIFIED_FAIL_CLOSED",
                    "sourceFidelity": (
                        SOURCE_UNRESOLVED
                        if seed_status == "CLASS_DEFAULT_UNRESOLVED"
                        else module_fidelity
                    ),
                    "status": seed_status,
                    **seed_axes,
                }
                seed.update(row_axis_statuses(seed_axes))
                seed_counts[UNRESOLVED] += 1

            default_rows: list[dict[str, Any]] = []
            exact_folded = exact_source_class.casefold()
            if (
                exact_folded == "particlemodulerequired"
                and "buselocalspace" not in {
                    str(name).casefold() for name in properties
                }
            ):
                default_rows.append(
                    {
                        "defaultId": f"{module_occurrence_id}::default:buselocalspace",
                        "fieldPath": "buselocalspace",
                        "family": "RequiredLocalSpace",
                        "blocker": "REQUIRED_BUSELOCALSPACE_CLASS_DEFAULT_UNRESOLVED",
                    }
                )
            if exact_folded == "efparticlemoduletypedatadecal":
                default_rows.append(
                    {
                        "defaultId": f"{module_occurrence_id}::default:decal",
                        "fieldPath": "typedata.decal.class-default-set",
                        "family": "Decal",
                        "blocker": "DECAL_CLASS_DEFAULT_REGISTRY_MISSING",
                    }
                )
            if exact_folded == "particlemoduletypedataribbon":
                default_rows.append(
                    {
                        "defaultId": f"{module_occurrence_id}::default:ribbon",
                        "fieldPath": "typedata.ribbon.class-default-set",
                        "family": "Ribbon",
                        "blocker": "RIBBON_CLASS_DEFAULT_REGISTRY_MISSING",
                    }
                )
            if exact_folded == "efparticlemoduletypedatalight":
                default_rows.append(
                    {
                        "defaultId": f"{module_occurrence_id}::default:light",
                        "fieldPath": "typedata.light.source-era-default-set",
                        "family": "Light",
                        "blocker": "POINT_LIGHT_CLASS_DEFAULTS_UNRESOLVED",
                    }
                )
            if renderer_type == "ScreenPost" and str(source_module.get("role")) == "REQUIRED":
                default_rows.append(
                    {
                        "defaultId": f"{module_occurrence_id}::default:screen-post",
                        "fieldPath": "screenpost.class-default-set",
                        "family": "ScreenPost",
                        "blocker": "SCREEN_POST_CLASS_DEFAULT_REGISTRY_MISSING",
                    }
                )
            completed_defaults = []
            for default in default_rows:
                default_axes = blocker_axes(
                    evidence=["SOURCE_ERA_CLASS_DEFAULT_EVIDENCE_UNAVAILABLE"],
                    execution=[default.pop("blocker")],
                )
                completed = {
                    **default,
                    "sourceFidelity": SOURCE_UNRESOLVED,
                    "classification": UNRESOLVED,
                    "evaluatorId": "",
                    "oracleStatus": "UNVERIFIED_FAIL_CLOSED",
                    **default_axes,
                }
                completed.update(row_axis_statuses(default_axes))
                completed_defaults.append(completed)
                default_counts[completed["family"]] += 1

            point_light = None
            component_occurrence = component_occurrences.get(
                property_identity_key(
                    evidence_id,
                    logical_package,
                    package_local_path,
                    "pointlightcomponent",
                )
            )
            if component_occurrence is not None:
                definition_id = str(component_occurrence.get("definitionId") or "")
                definition = component_definitions.get(definition_id)
                require(
                    definition is not None,
                    f"PointLight definition is missing: {definition_id}",
                )
                point_light = build_point_light_binding(
                    definition=definition,
                    occurrence=component_occurrence,
                    module_occurrence_id=module_occurrence_id,
                )
                point_light_bindings.append(point_light)
                consumed_local_occurrences.add(point_light["legacyOccurrenceId"])

            specific_module_blockers = {
                *selected_lod_module_execution_blockers,
                *native_tail["executionBlockers"],
                *(
                    seed["executionBlockers"]
                    if isinstance(seed, dict)
                    else []
                ),
                *(
                    blocker
                    for default in completed_defaults
                    for blocker in default["executionBlockers"]
                ),
                *(
                    blocker
                    for prop in property_rows
                    if prop["classification"] == UNRESOLVED
                    for blocker in prop["executionBlockers"]
                ),
                *(
                    point_light["executionBlockers"]
                    if isinstance(point_light, dict)
                    else []
                ),
                *(
                    blocker
                    for field in (
                        point_light["fields"]
                        if isinstance(point_light, dict)
                        else []
                    )
                    for blocker in field["executionBlockers"]
                ),
            }
            if (
                legacy_normalized_class(exact_source_class)
                != class_identity["normalizedClass"]
                and not class_identity["aliasId"]
            ):
                unapproved_class_alias_occurrence_count += 1
                specific_module_blockers.add(
                    "SOURCE_CLASS_ALIAS_OR_CUSTOM_HANDLER_UNVERIFIED"
                )
            module_decision = UNRESOLVED
            module_axes = blocker_axes(
                evidence=(
                    ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                    if module_fidelity != SOURCE_EXACT
                    else []
                ),
                execution={
                    "DOWNSTREAM_MODULE_HANDLER_RECEIPT_REQUIRED",
                    *specific_module_blockers,
                },
            )
            module = {
                "moduleOccurrenceId": module_occurrence_id,
                "order": order,
                "sourceReferenceIndex": int(
                    source_module.get("sourceReferenceIndex", -1)
                ),
                "role": str(source_module.get("role") or ""),
                "sourceDocument": source_document,
                "sourceObjectId": object_id,
                "sourceObjectPath": str(source_module.get("sourceObjectPath") or ""),
                "sourceRecordSha256": str(
                    source_module.get("sourceRecordSha256") or ""
                ),
                **class_identity,
                "sourceFidelity": module_fidelity,
                "classification": module_decision,
                "consumerId": (
                    "source.module."
                    + sanitized_id(class_identity["normalizedClass"])
                    + ".v1"
                ),
                "nativeTail": native_tail,
                "seed": seed,
                "implicitDefaults": completed_defaults,
                "properties": property_rows,
                "primitiveLeaves": leaf_rows,
                "distributions": distribution_rows,
                **module_axes,
            }
            module.update(row_axis_statuses(module_axes))
            occurrence_modules.append(module)
            module_counts[module_decision] += 1

        occurrences.append(
            {
                "occurrenceCompositeId": occurrence_id,
                "evidenceId": evidence_id,
                "sourceOccurrenceId": source_occurrence_id,
                "sourceSystemId": source_system_id,
                "sourceEmitterPath": source_emitter_path,
                "rendererType": renderer_type,
                "selectedLodSemantics": selected_lod_semantics,
                "modules": occurrence_modules,
            }
        )

    expected_local_occurrences = {
        str(row.get("occurrenceId") or "")
        for row in (
            list(local_reference_closure.get("distributionOccurrences", []))
            + list(local_reference_closure.get("componentOccurrences", []))
        )
    }
    require(
        consumed_local_occurrences == expected_local_occurrences,
        "semantic local-reference binding is incomplete: "
        f"missing={sorted(expected_local_occurrences - consumed_local_occurrences)} "
        f"extra={sorted(consumed_local_occurrences - expected_local_occurrences)}",
    )

    measured = {
        "activeOccurrenceCount": len(occurrences),
        "selectedLodFieldCount": sum(selected_lod_field_counts.values()),
        "orderedModuleReferenceCount": sum(
            len(row["modules"]) for row in occurrences
        ),
        "topLevelTaggedPropertyCount": sum(
            len(module["properties"])
            for occurrence in occurrences
            for module in occurrence["modules"]
        ),
        "primitiveLeafCount": sum(
            len(module["primitiveLeaves"])
            for occurrence in occurrences
            for module in occurrence["modules"]
        ),
        "distributionCount": sum(
            len(module["distributions"])
            for occurrence in occurrences
            for module in occurrence["modules"]
        ),
        "defaultDependentDistributionCount": distribution_field_evidence_counts[
            "defaultDependent"
        ],
        "operationReconstructedDistributionCount": distribution_field_evidence_counts[
            "reconstructed:operation"
        ],
        "lookupChunkReconstructedDistributionCount": distribution_field_evidence_counts[
            "reconstructed:lookupTableChunkSize"
        ],
        "lookupCountReconstructedDistributionCount": distribution_field_evidence_counts[
            "reconstructed:lookupTableNumElements"
        ],
        "explicitRandomOperationDistributionCount": distribution_field_evidence_counts[
            "explicitRandomOperation"
        ],
        "unapprovedClassAliasOccurrenceCount": (
            unapproved_class_alias_occurrence_count
        ),
        "localDistributionDefinitionCount": len(distribution_definitions),
        "localDistributionOccurrenceCount": len(local_distribution_bindings),
        "pointLightDefinitionCount": len(component_definitions),
        "pointLightOccurrenceCount": len(point_light_bindings),
        "externalNativeTailCount": native_tail_counts[UNRESOLVED],
        "seededModuleCount": sum(seed_counts.values()),
        "requiredLocalSpaceDefaultCount": default_counts["RequiredLocalSpace"],
        "decalDefaultCount": default_counts["Decal"],
        "ribbonDefaultCount": default_counts["Ribbon"],
        "screenPostDefaultCount": default_counts["ScreenPost"],
        "lightDefaultCount": default_counts["Light"],
    }
    require(measured == EXPECTED_DENOMINATORS, f"semantic denominator changed: {measured}")
    for counter_name, counter, denominator in (
        (
            "selected LOD field",
            selected_lod_field_counts,
            measured["selectedLodFieldCount"],
        ),
        ("module", module_counts, measured["orderedModuleReferenceCount"]),
        ("property", property_counts, measured["topLevelTaggedPropertyCount"]),
        ("primitive leaf", leaf_counts, measured["primitiveLeafCount"]),
        ("distribution", distribution_counts, measured["distributionCount"]),
    ):
        require(
            set(counter).issubset(DECISIONS) and sum(counter.values()) == denominator,
            f"{counter_name} semantic decision coverage is incomplete",
        )

    closure: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "Artist",
        "skillId": 31470,
        "scope": "ACTIVE_OCCURRENCES_FIRST_LOD_ONLY",
        "sourceFidelityPolicy": {
            "allowedTokens": sorted(SOURCE_FIDELITIES),
            "sourceExactIsNeverDerivedFromReconstruction": True,
            "externalModuleClosureFidelity": CURRENT_REVISION_EVIDENCE,
            "currentDefaultsOpenExecution": False,
        },
        "classAliasPolicy": {
            "automaticEfPrefixAliasAllowed": False,
            "automaticSeededSuffixAliasAllowed": False,
            "approvedAliasCount": len(APPROVED_CLASS_ALIASES),
        },
        "inputs": {
            name: {"canonicalSha256": FROZEN_INPUT_CANONICAL_SHA256[name]}
            for name in FROZEN_INPUT_CANONICAL_SHA256
        },
        "occurrences": occurrences,
        "localDistributionBindings": local_distribution_bindings,
        "pointLightBindings": point_light_bindings,
        "summary": {
            "denominators": measured,
            "moduleDecisionCounts": dict(sorted(module_counts.items())),
            "propertyDecisionCounts": dict(sorted(property_counts.items())),
            "primitiveLeafDecisionCounts": dict(sorted(leaf_counts.items())),
            "distributionDecisionCounts": dict(
                sorted(distribution_counts.items())
            ),
            "distributionFieldEvidenceCounts": dict(
                sorted(distribution_field_evidence_counts.items())
            ),
            "nativeTailDecisionCounts": dict(sorted(native_tail_counts.items())),
            "seedDecisionCounts": dict(sorted(seed_counts.items())),
            "implicitDefaultFamilyCounts": dict(sorted(default_counts.items())),
            "selectedLodFieldDecisionCounts": dict(
                sorted(selected_lod_field_counts.items())
            ),
            "moduleSourceFidelityCounts": dict(
                sorted(source_fidelity_counts.items())
            ),
            "unknownDecisionCount": 0,
            "unconsumedRowCount": 0,
            "silentIgnoredRowCount": 0,
            "allRowsClassified": True,
            "semanticExecutionAdmission": False,
        },
        "productAdmission": {
            "allowed": False,
            "admittedOccurrenceCount": 0,
            "totalOccurrenceCount": 35,
            "blockers": [
                "DOWNSTREAM_COMPILER_HANDLER_RECEIPTS_REQUIRED",
                "SOURCE_SEMANTIC_UNRESOLVED_ROWS_REMAIN",
                "PRODUCT_ADMISSION_OWNED_BY_FINAL_INTEGRATION_GATE",
            ],
        },
        "closureSha256": "",
    }
    unsigned = copy.deepcopy(closure)
    unsigned.pop("closureSha256")
    closure["closureSha256"] = canonical_sha256(unsigned)
    return closure


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Build the code-only Artist 31470 source semantic closure checkpoint."
        )
    )
    parser.add_argument("--active-inventory", required=True, type=Path)
    parser.add_argument("--normalized-graph", required=True, type=Path)
    parser.add_argument("--external-module-closure", required=True, type=Path)
    parser.add_argument("--source-evidence", required=True, type=Path)
    parser.add_argument("--local-reference-closure", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    closure = build_semantic_closure(
        load_json(args.active_inventory),
        load_json(args.normalized_graph),
        load_json(args.external_module_closure),
        load_json(args.source_evidence),
        load_json(args.local_reference_closure),
    )
    content = json_bytes(closure)
    if args.check:
        require(args.output.is_file(), f"semantic closure output is missing: {args.output}")
        require(
            args.output.read_bytes().replace(b"\r\n", b"\n")
            == content.replace(b"\r\n", b"\n"),
            f"semantic closure output is stale: {args.output}",
        )
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(content)
    summary = closure["summary"]
    print(
        "Artist F 31470 semantic closure "
        f"{'check' if args.check else 'write'}: "
        f"modules={summary['denominators']['orderedModuleReferenceCount']} "
        f"properties={summary['denominators']['topLevelTaggedPropertyCount']}/"
        f"{summary['denominators']['primitiveLeafCount']} "
        f"distributions={summary['denominators']['distributionCount']} "
        "product=0/35"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
