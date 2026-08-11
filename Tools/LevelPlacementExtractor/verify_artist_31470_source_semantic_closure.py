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
from typing import Any

from effect_source_contract_io import load_strict_json_object


EXECUTION_CONSUMED = "EXECUTION_CONSUMED"
VERIFIED_IRRELEVANT = "VERIFIED_IRRELEVANT"
UNRESOLVED = "UNRESOLVED"
DECISIONS = {EXECUTION_CONSUMED, VERIFIED_IRRELEVANT, UNRESOLVED}
FINAL_PRODUCT_BLOCKER = "PRODUCT_ADMISSION_OWNED_BY_FINAL_INTEGRATION_GATE"

RECONSTRUCTION_BLOCKERS = {
    "operation": "DISTRIBUTION_OPERATION_RECONSTRUCTION_UNVERIFIED",
    "lookupTableChunkSize":
        "DISTRIBUTION_LOOKUP_CHUNK_RECONSTRUCTION_UNVERIFIED",
    "lookupTableNumElements":
        "DISTRIBUTION_LOOKUP_COUNT_RECONSTRUCTION_UNVERIFIED",
}

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

FROZEN_INPUT_CANONICAL_SHA256 = {
    "activeInventory": "a1397ff82a2d3e1981e318aab1e36b3f6ff3cc08c3df14606fcfd777d36f4827",
    "normalizedGraph": "7daa2519dc943fe4b61341dcd1fd794c6db9d48a5ccaf015213f89de2fbc2b1f",
    "externalModuleClosure": "768a9a3c927913ba8a3c7d4ad02b2cfaa491271409fa94ae3c31c2f90e3a7b4a",
    "sourceEvidence": "a45e7548b442c7ee796eee088de660fb9804eb4ba03b960cd5e71cfc85a9276d",
    "localReferenceClosure": "9b0adf6efc2c0e16fd15b367f34c8b96027a41289df431f4fef56ae5012e77d3",
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


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def load_json(path: Path) -> dict[str, Any]:
    return load_strict_json_object(path)


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(
            value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
        ).encode("utf-8")
    ).hexdigest()


def sanitized_id(value: str) -> str:
    result = re.sub(r"[^a-z0-9]+", ".", value.casefold()).strip(".")
    require(result != "", f"stable semantic ID component is empty: {value}")
    return result


def primitive_leaf_paths(properties: dict[str, Any]) -> list[tuple[str, str, str]]:
    rows: list[tuple[str, str, str]] = []

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
                (
                    path,
                    top_property,
                    "boolean"
                    if isinstance(value, bool)
                    else "number"
                    if isinstance(value, (int, float))
                    else "string",
                )
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


def distribution_paths(properties: dict[str, Any]) -> list[tuple[str, str, int]]:
    rows: list[tuple[str, str, int]] = []

    def visit(value: Any, path: str, top_property: str) -> None:
        if (
            isinstance(value, dict)
            and value.get("type") == "structproperty"
            and str(value.get("structType", "")).casefold()
            in {"rawdistributionfloat", "rawdistributionvector"}
        ):
            raw = value.get("value")
            nested = raw.get("properties") if isinstance(raw, dict) else None
            distribution = (
                nested.get("distribution") if isinstance(nested, dict) else None
            )
            package_index = (
                distribution.get("value")
                if isinstance(distribution, dict)
                and isinstance(distribution.get("value"), int)
                else 0
            )
            rows.append((path, top_property, package_index))
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


def distribution_field_evidence_map(
    properties: dict[str, Any], raw_source_fidelity: str
) -> dict[str, dict[str, Any]]:
    rows: dict[str, dict[str, Any]] = {}

    def visit(value: Any, path: str) -> None:
        if (
            isinstance(value, dict)
            and value.get("type") == "structproperty"
            and str(value.get("structType", "")).casefold()
            in {"rawdistributionfloat", "rawdistributionvector"}
        ):
            raw = value.get("value")
            fields = raw.get("properties") if isinstance(raw, dict) else None
            fields = fields if isinstance(fields, dict) else {}
            lookup = fields.get("lookuptable")
            lookup_value = lookup.get("value") if isinstance(lookup, dict) else None
            has_lookup = isinstance(lookup_value, list) and bool(lookup_value)
            reconstructed = []
            if "op" not in fields:
                reconstructed.append("operation")
            if has_lookup and "lookuptablechunksize" not in fields:
                reconstructed.append("lookupTableChunkSize")
            if has_lookup and "lookuptablenumelements" not in fields:
                reconstructed.append("lookupTableNumElements")
            operation = fields.get("op")
            operation_value = (
                operation.get("value") if isinstance(operation, dict) else None
            )
            distribution = fields.get("distribution")
            package_index = (
                distribution.get("value")
                if isinstance(distribution, dict)
                and isinstance(distribution.get("value"), int)
                else 0
            )
            rows[path] = {
                "rawFieldNames": sorted(str(name).casefold() for name in fields),
                "reconstructedFieldNames": reconstructed,
                "hasLookupPayload": has_lookup,
                "defaultDependent": package_index == 0 and not has_lookup,
                "explicitOperation": (
                    operation_value if isinstance(operation_value, int) else None
                ),
                "rawFieldSourceFidelity": raw_source_fidelity,
                "reconstructedFieldSourceFidelity": (
                    "RECONSTRUCTED_GRAPH" if reconstructed else ""
                ),
                "numericExecutionEvidenceStatus": "UNVERIFIED",
            }
            return
        if isinstance(value, dict) and "type" in value and "value" in value:
            visit(value.get("value"), path)
        elif isinstance(value, dict):
            for name, child in value.items():
                child_path = f"{path}.{str(name).casefold()}" if path else str(name).casefold()
                visit(child, child_path)
        elif isinstance(value, list):
            for index, child in enumerate(value):
                visit(child, f"{path}[{index}]")

    for name, value in properties.items():
        visit(value, str(name).casefold())
    return rows


def validate_axes(row: dict[str, Any], identity: str) -> None:
    axes = (
        "evidenceBlockers",
        "artifactBindingBlockers",
        "executionBlockers",
        "productBlockers",
    )
    for name in axes:
        values = row.get(name)
        require(isinstance(values, list), f"{identity} has no {name}")
        require(
            values == sorted(set(values)),
            f"{identity} {name} is not a canonical set",
        )
    artifact_blockers = row["artifactBindingBlockers"]
    artifact_integrity = row.get("artifactBindingIntegrity")
    require(
        isinstance(artifact_integrity, dict)
        and artifact_integrity.get("verified") == (not artifact_blockers)
        and artifact_integrity.get("blockers") == artifact_blockers,
        f"{identity} artifact binding integrity does not match blockers",
    )
    execution_blockers = sorted(
        {*artifact_blockers, *row["executionBlockers"]}
    )
    admission = row.get("executionAdmission")
    require(
        isinstance(admission, dict)
        and admission.get("allowed") == (not execution_blockers)
        and admission.get("blockers") == execution_blockers,
        f"{identity} execution admission does not match blocker axes",
    )
    product_blockers = sorted(
        {
            *artifact_blockers,
            *row["executionBlockers"],
            *row["productBlockers"],
        }
    )
    product_admission = row.get("productAdmission")
    require(
        FINAL_PRODUCT_BLOCKER in row["productBlockers"]
        and isinstance(product_admission, dict)
        and product_admission.get("allowed") == (not product_blockers)
        and product_admission.get("blockers") == product_blockers,
        f"{identity} Product admission does not match blocker axes",
    )
    decision = str(row.get("classification") or "")
    require(decision in DECISIONS, f"{identity} decision is unknown: {decision}")
    if decision == EXECUTION_CONSUMED:
        require(
            bool(
                row.get("consumerId")
                or row.get("evaluatorId")
                or row.get("componentBinderId")
            ),
            f"{identity} consumed row has no typed consumer",
        )
    elif decision == VERIFIED_IRRELEVANT:
        require(
            bool(row.get("irrelevanceOracleId") or row.get("oracleId")),
            f"{identity} irrelevant row has no oracle",
        )
    else:
        require(
            bool(execution_blockers) and admission.get("allowed") is False,
            f"{identity} unresolved row has no execution-closing blocker",
        )


def expected_axes(
    *,
    evidence: list[str] | tuple[str, ...] | set[str] = (),
    artifact: list[str] | tuple[str, ...] | set[str] = (),
    execution: list[str] | tuple[str, ...] | set[str] = (),
    product: list[str] | tuple[str, ...] | set[str] = (),
) -> dict[str, list[str]]:
    return {
        "evidenceBlockers": sorted({str(value) for value in evidence}),
        "artifactBindingBlockers": sorted({str(value) for value in artifact}),
        "executionBlockers": sorted({str(value) for value in execution}),
        "productBlockers": sorted(
            {FINAL_PRODUCT_BLOCKER, *(str(value) for value in product)}
        ),
    }


def expected_axes_from_source_blockers(
    blockers: list[str] | set[str],
) -> dict[str, list[str]]:
    axes = expected_axes()
    for blocker in blockers:
        token = str(blocker)
        if token.startswith(("COMPILED_", "CONSTANT_CURVE_COMPILER_")) or token in {
            "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN",
            "LIGHT_RENDERER_NOT_COMPILED",
            "RUNTIME_PARAMETER_SOURCE_CLOSURE_UNPROVEN",
        }:
            axes["executionBlockers"].append(token)
        elif token.startswith("PRODUCT_"):
            axes["productBlockers"].append(token)
        elif "PAYLOAD_HASH" in token or "ARTIFACT_BINDING" in token:
            axes["artifactBindingBlockers"].append(token)
        else:
            axes["evidenceBlockers"].append(token)
    return {name: sorted(set(values)) for name, values in axes.items()}


def require_exact_axes(
    row: dict[str, Any], expected: dict[str, list[str]], identity: str
) -> None:
    for name in (
        "evidenceBlockers",
        "artifactBindingBlockers",
        "executionBlockers",
        "productBlockers",
    ):
        require(
            row.get(name) == expected[name],
            f"{identity} blocker lineage changed: {name}",
        )
    validate_axes(row, identity)


def build_local_occurrence_index(
    local_references: dict[str, Any], field: str
) -> dict[tuple[str, str, str, str], dict[str, Any]]:
    result: dict[tuple[str, str, str, str], dict[str, Any]] = {}
    for row in local_references.get(field, []):
        identity = row.get("propertyIdentity") or {}
        key = (
            str(row.get("activeElementId") or ""),
            str(identity.get("logicalPackage") or "").casefold(),
            str(identity.get("packageLocalPath") or "").casefold(),
            str(identity.get("propertyPath") or "").casefold(),
        )
        require(all(key) and key not in result, f"duplicate local occurrence: {key}")
        result[key] = row
    return result


def expected_local_source_fidelity(definition: dict[str, Any]) -> str:
    fidelity = str(definition.get("fidelity") or "")
    if fidelity == "SOURCE_EXACT_PHYSICAL_PACKAGE":
        return "SOURCE_EXACT"
    if fidelity in {
        "RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED",
        "PINNED_SOURCE_RECORD_PHYSICAL_ABSENT",
    }:
        return "CURRENT_REVISION_EVIDENCE"
    return "UNRESOLVED"


def verify_semantic_closure(
    closure: dict[str, Any],
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
        require(
            canonical_sha256(value) == FROZEN_INPUT_CANONICAL_SHA256[name],
            f"oracle input changed: {name}",
        )
    require(
        closure.get("schema") == "lostark.effect-source-semantic-closure"
        and closure.get("formatVersion") == 1,
        "semantic closure schema changed",
    )
    require(
        closure.get("characterClass") == "Artist"
        and closure.get("skillId") == 31470
        and closure.get("scope") == "ACTIVE_OCCURRENCES_FIRST_LOD_ONLY",
        "semantic closure subject identity changed",
    )
    unsigned = copy.deepcopy(closure)
    expected_self = str(unsigned.pop("closureSha256", ""))
    require(
        expected_self and canonical_sha256(unsigned) == expected_self,
        "semantic closure self hash is invalid",
    )
    policy = closure.get("sourceFidelityPolicy") or {}
    require(
        policy
        == {
            "allowedTokens": [
                "CURRENT_REVISION_EVIDENCE",
                "RECONSTRUCTED_GRAPH",
                "RECONSTRUCTED_NUMERICALLY_VERIFIED",
                "SOURCE_EXACT",
                "UNRESOLVED",
            ],
            "sourceExactIsNeverDerivedFromReconstruction": True,
            "externalModuleClosureFidelity": "CURRENT_REVISION_EVIDENCE",
            "currentDefaultsOpenExecution": False,
        },
        "source fidelity and reconstruction were merged",
    )
    require(
        closure.get("inputs")
        == {
            name: {"canonicalSha256": digest}
            for name, digest in FROZEN_INPUT_CANONICAL_SHA256.items()
        },
        "semantic closure input manifest changed",
    )
    alias_policy = closure.get("classAliasPolicy") or {}
    require(
        alias_policy
        == {
            "automaticEfPrefixAliasAllowed": False,
            "automaticSeededSuffixAliasAllowed": False,
            "approvedAliasCount": 0,
        },
        "class alias policy is not fail-closed",
    )

    graph_rows = {str(row["nodeId"]): row for row in normalized_graph["nodes"]}
    external_rows = {
        str(row["objectId"]): row
        for package in external_module_closure["packages"]
        for row in package["objects"]
    }
    renderer_by_element = {
        str(row["activeElementId"]): str(row["rendererType"])
        for row in active_inventory["activeElements"]
    }
    report_occurrence_rows = closure.get("occurrences", [])
    require(isinstance(report_occurrence_rows, list), "semantic occurrences are invalid")
    report_occurrences = {
        str(row.get("evidenceId") or ""): row
        for row in report_occurrence_rows
    }
    require(
        len(report_occurrence_rows) == len(report_occurrences) == 35,
        "semantic occurrence rows are duplicated or incomplete",
    )
    distribution_occurrences = build_local_occurrence_index(
        local_reference_closure, "distributionOccurrences"
    )
    component_occurrences = build_local_occurrence_index(
        local_reference_closure, "componentOccurrences"
    )
    local_definition_rows = local_reference_closure.get(
        "distributionDefinitions", []
    )
    local_definitions = {
        str(row.get("definitionId") or ""): row
        for row in local_definition_rows
    }
    component_definition_rows = local_reference_closure.get(
        "componentDefinitions", []
    )
    component_definitions = {
        str(row.get("definitionId") or ""): row
        for row in component_definition_rows
    }
    require(
        len(local_definition_rows) == len(local_definitions) == 15
        and "" not in local_definitions,
        "local distribution definition denominator changed",
    )
    require(
        len(component_definition_rows) == len(component_definitions) == 1
        and "" not in component_definitions,
        "PointLight definition denominator changed",
    )
    local_binding_rows = closure.get("localDistributionBindings", [])
    point_light_binding_rows = closure.get("pointLightBindings", [])
    require(
        isinstance(local_binding_rows, list)
        and isinstance(point_light_binding_rows, list),
        "semantic local binding rows are invalid",
    )
    local_binding_by_legacy_occurrence = {
        str(row.get("legacyOccurrenceId") or ""): row
        for row in local_binding_rows
    }
    point_light_by_legacy_occurrence = {
        str(row.get("legacyOccurrenceId") or ""): row
        for row in point_light_binding_rows
    }
    require(
        len(local_binding_rows) == len(local_binding_by_legacy_occurrence) == 17,
        "local distribution binding denominator changed",
    )
    require(
        len(point_light_binding_rows) == len(point_light_by_legacy_occurrence) == 1,
        "PointLight binding denominator changed",
    )

    module_counts: Counter[str] = Counter()
    property_counts: Counter[str] = Counter()
    leaf_counts: Counter[str] = Counter()
    distribution_counts: Counter[str] = Counter()
    native_tail_counts: Counter[str] = Counter()
    seed_counts: Counter[str] = Counter()
    default_counts: Counter[str] = Counter()
    selected_lod_field_counts: Counter[str] = Counter()
    distribution_field_evidence_counts: Counter[str] = Counter()
    unapproved_class_alias_occurrence_count = 0
    source_fidelity_counts: Counter[str] = Counter()
    seen_local_occurrences: set[str] = set()
    seen_point_light_occurrences: set[str] = set()

    for evidence in source_evidence["occurrences"]:
        evidence_id = str(evidence["evidenceId"])
        report = report_occurrences.get(evidence_id)
        require(report is not None, f"semantic occurrence is missing: {evidence_id}")
        source_occurrence_id = str(evidence["sourceOccurrenceId"])
        source_system_id = str(evidence["sourceSystemId"])
        source_emitter_path = str(evidence["sourceEmitterPath"])
        expected_occurrence_id = (
            f"{source_system_id}::{source_occurrence_id}::{source_emitter_path}"
        )
        require(
            report.get("occurrenceCompositeId") == expected_occurrence_id
            and report.get("sourceOccurrenceId") == source_occurrence_id
            and report.get("sourceSystemId") == source_system_id
            and report.get("sourceEmitterPath") == source_emitter_path
            and report.get("rendererType") == renderer_by_element[evidence_id],
            f"semantic occurrence identity changed: {evidence_id}",
        )
        expected_lod = evidence.get("selectedLod") or {}
        report_lod = report.get("selectedLodSemantics") or {}
        require(
            evidence.get("lodSelectionPolicy") == "FIRST_LOD_ONLY"
            and report_lod.get("sourceLodPath") == expected_lod.get("sourceLodPath")
            and report_lod.get("sourceLodNodeId") == expected_lod.get("sourceLodNodeId")
            and report_lod.get("sourceLodRecordSha256")
            == expected_lod.get("sourceLodRecordSha256")
            and report_lod.get("emitterArrayIndex")
            == expected_lod.get("emitterArrayIndex"),
            f"selected LOD identity changed: {evidence_id}",
        )
        lod_fields = {
            str(row.get("fieldName") or ""): row
            for row in report_lod.get("fields", [])
        }
        require(
            len(report_lod.get("fields", [])) == len(lod_fields) == 2
            and set(lod_fields) == {"level", "enabled"},
            f"selected LOD field set changed: {evidence_id}",
        )
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
            field = lod_fields[field_name]
            require(
                expected_lod.get(field_name) is None
                and expected_lod.get(provenance_name) == "UNRESOLVED_CLASS_DEFAULT"
                and field.get("fieldId")
                == f"{expected_occurrence_id}::selected-lod:{field_name}"
                and field.get("encodedValue") is None
                and field.get("provenance") == "UNRESOLVED_CLASS_DEFAULT"
                and field.get("sourceFidelity") == "UNRESOLVED"
                and field.get("classification") == UNRESOLVED
                and field.get("evaluatorId") == ""
                and field.get("oracleStatus") == "UNVERIFIED_FAIL_CLOSED",
                f"selected LOD default was laundered: {evidence_id}:{field_name}",
            )
            require_exact_axes(
                field,
                expected_axes(
                    evidence=[
                        "SOURCE_ERA_LOD_CLASS_DEFAULT_EVIDENCE_UNAVAILABLE"
                    ],
                    execution=[blocker],
                ),
                str(field["fieldId"]),
            )
            selected_lod_field_counts[UNRESOLVED] += 1
        modules = report.get("modules") or []
        require(
            len(modules) == len(evidence["moduleReferenceOrder"]),
            f"semantic module count changed: {evidence_id}",
        )
        for order, (source_module, module) in enumerate(
            zip(evidence["moduleReferenceOrder"], modules)
        ):
            source_document = str(source_module["sourceDocument"])
            object_id = str(source_module["sourceObjectId"])
            raw = (
                graph_rows.get(object_id)
                if source_document == "normalizedGraph"
                else external_rows.get(object_id)
            )
            require(raw is not None, f"oracle module record is missing: {object_id}")
            expected_module_id = f"{expected_occurrence_id}::module:{order:03d}"
            exact_class = str(raw.get("className") or "")
            expected_fidelity = (
                "SOURCE_EXACT"
                if source_document == "normalizedGraph"
                else "CURRENT_REVISION_EVIDENCE"
            )
            require(
                int(module.get("order", -1)) == order
                and module.get("moduleOccurrenceId") == expected_module_id
                and module.get("sourceReferenceIndex")
                == source_module.get("sourceReferenceIndex")
                and module.get("role") == source_module.get("role")
                and module.get("sourceDocument") == source_document
                and module.get("sourceObjectId") == object_id
                and module.get("sourceObjectPath")
                == source_module.get("sourceObjectPath")
                and module.get("sourceRecordSha256") == canonical_sha256(raw)
                and module.get("exactSourceClass") == exact_class
                and module.get("sourceFidelity") == expected_fidelity,
                f"module evidence binding changed: {expected_module_id}",
            )
            source_fidelity_counts[expected_fidelity] += 1
            require(
                module.get("aliasId") == ""
                and module.get("aliasEvidenceId") == ""
                and module.get("normalizedClass") == exact_class.casefold(),
                f"unapproved source class alias was introduced: {expected_module_id}",
            )
            if source_document == "externalModuleClosure":
                require(
                    module.get("sourceFidelity") != "SOURCE_EXACT"
                    and "SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"
                    in module.get("evidenceBlockers", []),
                    f"external module was laundered to SOURCE_EXACT: {expected_module_id}",
                )

            properties = raw.get("properties") or {}
            report_property_rows = module.get("properties", [])
            report_properties = {
                str(row.get("propertyPath") or ""): row
                for row in report_property_rows
            }
            require(
                len(report_property_rows) == len(report_properties)
                and set(report_properties)
                == {str(name).casefold() for name in properties},
                f"top-level property set changed: {expected_module_id}",
            )
            expected_leaves = primitive_leaf_paths(properties)
            report_leaf_rows = module.get("primitiveLeaves", [])
            report_leaves = {
                str(row.get("propertyPath") or ""): row
                for row in report_leaf_rows
            }
            require(
                len(report_leaf_rows) == len(report_leaves)
                and set(report_leaves)
                == {path for path, _top, _kind in expected_leaves}
                and len(report_leaves) == len(expected_leaves),
                f"primitive leaf set changed: {expected_module_id}",
            )
            expected_distributions = distribution_paths(properties)
            expected_distribution_field_evidence = (
                distribution_field_evidence_map(properties, expected_fidelity)
            )
            report_distribution_rows = module.get("distributions", [])
            report_distributions = {
                str(row.get("propertyPath") or ""): row
                for row in report_distribution_rows
            }
            require(
                len(report_distribution_rows) == len(report_distributions)
                and set(report_distributions)
                == {path for path, _, _ in expected_distributions}
                and len(report_distributions) == len(expected_distributions),
                f"distribution set changed: {expected_module_id}",
            )

            logical_package = object_id.partition(":")[0].casefold()
            package_local_path = str(raw.get("objectPath") or "").casefold()
            expected_distribution_ids_by_top: dict[str, list[str]] = {}
            for property_path, top_property, package_index in expected_distributions:
                row = report_distributions[property_path]
                identity = f"{expected_module_id}::distribution:{property_path}"
                expected_distribution_ids_by_top.setdefault(
                    top_property, []
                ).append(identity)
                require(
                    row.get("distributionId") == identity
                    and row.get("occurrenceId") == identity,
                    f"distribution occurrence ID changed: {identity}",
                )
                require(
                    row.get("topLevelPropertyPath") == top_property,
                    f"distribution top-level property changed: {identity}",
                )
                local = distribution_occurrences.get(
                    (
                        evidence_id,
                        logical_package,
                        package_local_path,
                        (property_path + ".distribution").casefold(),
                    )
                )
                if local is None:
                    require(
                        package_index == 0
                        and row.get("definitionId")
                        == (
                            "distribution-definition::"
                            f"{object_id.casefold()}::{property_path.casefold()}"
                        )
                        and row.get("referenceId")
                        == (
                            "distribution-reference::"
                            f"{object_id.casefold()}::{property_path.casefold()}"
                        )
                        and row.get("classification") == UNRESOLVED
                        and row.get("sourceFidelity") == expected_fidelity
                        and row.get("evaluatorId")
                        == "ue3.raw-distribution.cooked-lookup.v1"
                        and row.get("evaluatorVersion") == 1
                        and row.get("oracleStatus")
                        == "UNVERIFIED_FAIL_CLOSED",
                        f"inline distribution identity changed: {identity}",
                    )
                    require(
                        row.get("fieldEvidence")
                        == expected_distribution_field_evidence[property_path]
                        ,
                        f"inline distribution evidence was laundered: {identity}",
                    )
                    field_evidence = row["fieldEvidence"]
                    inline_execution_blockers = {
                        "DOWNSTREAM_EVALUATOR_RECEIPT_REQUIRED",
                        "INDEPENDENT_NUMERIC_ORACLE_REQUIRED",
                        *(
                            RECONSTRUCTION_BLOCKERS[name]
                            for name in field_evidence["reconstructedFieldNames"]
                        ),
                    }
                    if field_evidence["defaultDependent"]:
                        inline_execution_blockers.add(
                            "DISTRIBUTION_CLASS_DEFAULT_VALUE_UNRESOLVED"
                        )
                    if field_evidence["explicitOperation"] in {2, 3}:
                        inline_execution_blockers.add(
                            "DISTRIBUTION_RANDOM_STREAM_SAMPLE_ORDER_UNVERIFIED"
                        )
                    row_expected_axes = expected_axes(
                        evidence=(
                            ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                            if expected_fidelity != "SOURCE_EXACT"
                            else []
                        ),
                        execution=inline_execution_blockers,
                    )
                else:
                    legacy_occurrence_id = str(local["occurrenceId"])
                    require(
                        package_index != 0
                        and row.get("legacyOccurrenceId") == legacy_occurrence_id
                        and local_binding_by_legacy_occurrence.get(
                            legacy_occurrence_id
                        )
                        == row,
                        f"local distribution binding changed: {identity}",
                    )
                    definition_id = str(local["definitionId"])
                    definition = local_definitions.get(definition_id)
                    require(
                        definition is not None,
                        f"local distribution definition is missing: {definition_id}",
                    )
                    local_identity = local.get("propertyIdentity") or {}
                    source_class = str(definition.get("sourceClass") or "")
                    expected_evaluator = LOCAL_EVALUATOR_IDS.get(
                        source_class.casefold(), ""
                    )
                    definition_identity = (
                        f"{str(local_identity.get('logicalPackage') or '').casefold()}::"
                        f"{str(definition.get('targetPackageLocalPath') or '').casefold()}"
                    )
                    semantic_status = str(
                        (definition.get("semanticCoverage") or {}).get("status")
                        or ""
                    )
                    expected_oracle_status = (
                        "STRUCTURAL_BRANCH_BOUND_NUMERIC_ORACLE_PENDING"
                        if semantic_status == "SEMANTIC_SOURCE_READY"
                        else "UNVERIFIED_FAIL_CLOSED"
                    )
                    require(
                        expected_evaluator != ""
                        and row.get("legacyReferenceId")
                        == definition.get("referenceId")
                        and row.get("legacyDefinitionId") == definition_id
                        and row.get("legacyOccurrenceId")
                        == legacy_occurrence_id
                        and row.get("moduleOccurrenceId") == expected_module_id
                        and row.get("distributionId") == identity
                        and row.get("definitionId")
                        == "distribution-definition::" + definition_identity
                        and row.get("referenceId")
                        == "distribution-reference::" + definition_identity
                        and row.get("exactSourceClass") == source_class
                        and row.get("evaluatorId") == expected_evaluator
                        and row.get("evaluatorVersion") == 1
                        and row.get("oracleStatus") == expected_oracle_status
                        and row.get("sourceFidelity")
                        == expected_local_source_fidelity(definition)
                        and row.get("classification") == UNRESOLVED
                        and str(
                            row.get("sourceReferencePropertyPath") or ""
                        ).casefold()
                        == (property_path + ".distribution").casefold()
                        and row.get("fieldEvidence")
                        == expected_distribution_field_evidence[property_path]
                        ,
                        f"local distribution decision changed: {identity}",
                    )
                    source_blockers = {
                        *list(
                            (definition.get("executionAdmission") or {}).get(
                                "blockers"
                            )
                            or []
                        ),
                        *list(
                            (local.get("executionAdmission") or {}).get(
                                "blockers"
                            )
                            or []
                        ),
                    }
                    row_expected_axes = expected_axes_from_source_blockers(
                        source_blockers
                    )
                    row_expected_axes["executionBlockers"] = sorted(
                        {
                            *row_expected_axes["executionBlockers"],
                            "DOWNSTREAM_EVALUATOR_RECEIPT_REQUIRED",
                            "INDEPENDENT_NUMERIC_ORACLE_REQUIRED",
                            *(
                                RECONSTRUCTION_BLOCKERS[name]
                                for name in row["fieldEvidence"][
                                    "reconstructedFieldNames"
                                ]
                            ),
                            *(
                                [
                                    "DISTRIBUTION_RANDOM_STREAM_SAMPLE_ORDER_UNVERIFIED"
                                ]
                                if row["fieldEvidence"]["explicitOperation"]
                                in {2, 3}
                                else []
                            ),
                        }
                    )
                    seen_local_occurrences.add(legacy_occurrence_id)
                require_exact_axes(row, row_expected_axes, identity)
                field_evidence = row["fieldEvidence"]
                if field_evidence["defaultDependent"]:
                    distribution_field_evidence_counts["defaultDependent"] += 1
                for name in field_evidence["reconstructedFieldNames"]:
                    distribution_field_evidence_counts[
                        "reconstructed:" + name
                    ] += 1
                if field_evidence["explicitOperation"] in {2, 3}:
                    require(
                        "DISTRIBUTION_RANDOM_STREAM_SAMPLE_ORDER_UNVERIFIED"
                        in row.get("executionBlockers", []),
                        f"distribution random sample order was opened: {identity}",
                    )
                    distribution_field_evidence_counts[
                        "explicitRandomOperation"
                    ] += 1
                distribution_counts[str(row["classification"])] += 1

            for property_path, prop in report_properties.items():
                nested_distributions = [
                    report_distributions[distribution_path]
                    for distribution_path, top_path, _package_index
                    in expected_distributions
                    if top_path == property_path
                ]
                if property_path == "randomseedinfo":
                    property_expected_axes = expected_axes(
                        evidence=(
                            ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                            if expected_fidelity != "SOURCE_EXACT"
                            else []
                        ),
                        execution=[
                            "RANDOM_SEED_CONSUMPTION_SEMANTICS_UNRESOLVED"
                        ],
                    )
                    expected_consumer_id = ""
                elif nested_distributions:
                    property_expected_axes = expected_axes(
                        evidence={
                            blocker
                            for distribution in nested_distributions
                            for blocker in distribution["evidenceBlockers"]
                        },
                        artifact={
                            blocker
                            for distribution in nested_distributions
                            for blocker in distribution[
                                "artifactBindingBlockers"
                            ]
                        },
                        execution={
                            "DOWNSTREAM_PROPERTY_HANDLER_RECEIPT_REQUIRED",
                            *(
                                blocker
                                for distribution in nested_distributions
                                for blocker in distribution[
                                    "executionBlockers"
                                ]
                            ),
                        },
                    )
                    expected_consumer_id = (
                        "source.property."
                        + sanitized_id(exact_class)
                        + "."
                        + sanitized_id(property_path)
                        + ".v1"
                    )
                elif property_path == "b3ddrawmode":
                    property_expected_axes = expected_axes(
                        evidence=(
                            ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                            if expected_fidelity != "SOURCE_EXACT"
                            else []
                        ),
                        execution=["EDITOR_ONLY_IRRELEVANCE_PROOF_REQUIRED"],
                    )
                    expected_consumer_id = ""
                else:
                    property_expected_axes = expected_axes(
                        evidence=(
                            ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                            if expected_fidelity != "SOURCE_EXACT"
                            else []
                        ),
                        execution=[
                            "DOWNSTREAM_PROPERTY_HANDLER_RECEIPT_REQUIRED"
                        ],
                    )
                    expected_consumer_id = (
                        "source.property."
                        + sanitized_id(exact_class)
                        + "."
                        + sanitized_id(property_path)
                        + ".v1"
                    )
                require(
                    prop.get("propertyId")
                    == f"{expected_module_id}::property:{property_path}"
                    and prop.get("sourceFidelity") == expected_fidelity
                    and prop.get("classification") == UNRESOLVED
                    and prop.get("distributionIds")
                    == expected_distribution_ids_by_top.get(property_path, [])
                    and prop.get("consumerId") == expected_consumer_id
                    and prop.get("irrelevanceOracleId") == "",
                    f"top-level property decision changed: {expected_module_id}:{property_path}",
                )
                require_exact_axes(
                    prop,
                    property_expected_axes,
                    str(prop["propertyId"]),
                )
                property_counts[UNRESOLVED] += 1

            top_by_leaf = {
                path: (top, kind) for path, top, kind in expected_leaves
            }
            for leaf_path, leaf in report_leaves.items():
                expected_top, expected_kind = top_by_leaf[leaf_path]
                parent = report_properties[expected_top]
                require(
                    leaf.get("leafId")
                    == f"{expected_module_id}::leaf:{leaf_path}"
                    and leaf.get("topLevelPropertyPath") == expected_top
                    and leaf.get("valueKind") == expected_kind
                    and leaf.get("classification") == parent["classification"]
                    and leaf.get("sourceFidelity") == parent["sourceFidelity"]
                    and leaf.get("consumerId") == parent.get("consumerId")
                    and leaf.get("irrelevanceOracleId")
                    == parent.get("irrelevanceOracleId")
                    and all(
                        leaf.get(axis) == parent.get(axis)
                        for axis in (
                            "evidenceBlockers",
                            "artifactBindingBlockers",
                            "executionBlockers",
                            "productBlockers",
                            "artifactBindingIntegrity",
                            "executionAdmission",
                            "productAdmission",
                        )
                    ),
                    f"primitive leaf decision changed: {expected_module_id}:{leaf_path}",
                )
                validate_axes(leaf, str(leaf["leafId"]))
                leaf_counts[str(leaf["classification"])] += 1

            native = module.get("nativeTail") or {}
            if source_module["nativeTailStatus"] == "SOURCE_DECODED_NO_NATIVE_TAIL":
                require(
                    native.get("status") == source_module["nativeTailStatus"]
                    and native.get("classification") == VERIFIED_IRRELEVANT
                    and native.get("oracleId")
                    == "source.record.serial-size-equals-property-stream-end.v1",
                    f"proven native-tail decision changed: {expected_module_id}",
                )
                native_expected_axes = expected_axes()
            else:
                require(
                    native.get("status") == source_module["nativeTailStatus"]
                    and native.get("classification") == UNRESOLVED
                    and "EXTERNAL_MODULE_NATIVE_TAIL_NOT_PROVEN"
                    in native.get("executionBlockers", []),
                    f"external native tail was silently ignored: {expected_module_id}",
                )
                native_expected_axes = expected_axes(
                    evidence=["EXTERNAL_MODULE_NATIVE_TAIL_BYTES_UNAVAILABLE"],
                    execution=["EXTERNAL_MODULE_NATIVE_TAIL_NOT_PROVEN"],
                )
            require_exact_axes(
                native,
                native_expected_axes,
                expected_module_id + ":native-tail",
            )
            native_tail_counts[str(native["classification"])] += 1

            seed = module.get("seed")
            seed_status = str(source_module["randomSeedStatus"])
            if seed_status == "NOT_SEEDED":
                require(seed is None, f"non-seeded module gained seed semantics: {expected_module_id}")
            else:
                expected_seed_specific_blocker = (
                    "RANDOM_SEED_OPAQUE_PAYLOAD_UNRESOLVED"
                    if seed_status == "OPAQUE_HEX_METADATA_ONLY"
                    else "RANDOM_SEED_SELECTION_POLICY_UNRESOLVED"
                    if seed_status
                    == "SEED_ARRAY_SOURCE_DECODED_CONSUMPTION_UNRESOLVED"
                    else "RANDOM_SEED_CLASS_DEFAULT_UNRESOLVED"
                )
                require(
                    isinstance(seed, dict)
                    and seed.get("classification") == UNRESOLVED
                    and seed.get("status") == seed_status
                    and seed.get("evaluatorId")
                    == "ue3.particle-random-seed-info.v1"
                    and seed.get("evaluatorVersion") == 1
                    and seed.get("oracleStatus") == "UNVERIFIED_FAIL_CLOSED"
                    and seed.get("sourceFidelity")
                    == (
                        "UNRESOLVED"
                        if seed_status == "CLASS_DEFAULT_UNRESOLVED"
                        else expected_fidelity
                    )
                    and "RANDOM_SEED_CONSUMPTION_SEMANTICS_UNRESOLVED"
                    in seed.get("executionBlockers", [])
                    and expected_seed_specific_blocker
                    in seed.get("executionBlockers", []),
                    f"seed semantics were laundered: {expected_module_id}",
                )
                require_exact_axes(
                    seed,
                    expected_axes(
                        evidence=(
                            ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                            if expected_fidelity != "SOURCE_EXACT"
                            else []
                        ),
                        execution=[
                            "RANDOM_SEED_CONSUMPTION_SEMANTICS_UNRESOLVED",
                            expected_seed_specific_blocker,
                        ],
                    ),
                    expected_module_id + ":seed",
                )
                seed_counts[UNRESOLVED] += 1

            expected_default_specs: list[tuple[str, str, str, str]] = []
            exact_folded = exact_class.casefold()
            if exact_folded == "particlemodulerequired" and "buselocalspace" not in {
                str(name).casefold() for name in properties
            }:
                expected_default_specs.append(
                    (
                        "RequiredLocalSpace",
                        "buselocalspace",
                        f"{expected_module_id}::default:buselocalspace",
                        "REQUIRED_BUSELOCALSPACE_CLASS_DEFAULT_UNRESOLVED",
                    )
                )
            if exact_folded == "efparticlemoduletypedatadecal":
                expected_default_specs.append(
                    (
                        "Decal",
                        "typedata.decal.class-default-set",
                        f"{expected_module_id}::default:decal",
                        "DECAL_CLASS_DEFAULT_REGISTRY_MISSING",
                    )
                )
            if exact_folded == "particlemoduletypedataribbon":
                expected_default_specs.append(
                    (
                        "Ribbon",
                        "typedata.ribbon.class-default-set",
                        f"{expected_module_id}::default:ribbon",
                        "RIBBON_CLASS_DEFAULT_REGISTRY_MISSING",
                    )
                )
            if exact_folded == "efparticlemoduletypedatalight":
                expected_default_specs.append(
                    (
                        "Light",
                        "typedata.light.source-era-default-set",
                        f"{expected_module_id}::default:light",
                        "POINT_LIGHT_CLASS_DEFAULTS_UNRESOLVED",
                    )
                )
            if (
                renderer_by_element[evidence_id] == "ScreenPost"
                and source_module["role"] == "REQUIRED"
            ):
                expected_default_specs.append(
                    (
                        "ScreenPost",
                        "screenpost.class-default-set",
                        f"{expected_module_id}::default:screen-post",
                        "SCREEN_POST_CLASS_DEFAULT_REGISTRY_MISSING",
                    )
                )
            expected_defaults = Counter(
                family for family, _path, _identity, _blocker in expected_default_specs
            )
            actual_defaults = Counter(
                str(row.get("family") or "")
                for row in module.get("implicitDefaults", [])
            )
            require(
                actual_defaults == expected_defaults,
                f"implicit default rows changed: {expected_module_id}",
            )
            expected_default_by_family = {
                family: (field_path, default_id, blocker)
                for family, field_path, default_id, blocker in expected_default_specs
            }
            for default in module.get("implicitDefaults", []):
                family = str(default.get("family") or "")
                expected_default = expected_default_by_family.get(family)
                require(
                    expected_default is not None
                    and default.get("fieldPath") == expected_default[0]
                    and default.get("defaultId") == expected_default[1]
                    and expected_default[2]
                    in default.get("executionBlockers", [])
                    and "SOURCE_ERA_CLASS_DEFAULT_EVIDENCE_UNAVAILABLE"
                    in default.get("evidenceBlockers", [])
                    and default.get("evaluatorId") == ""
                    and default.get("oracleStatus") == "UNVERIFIED_FAIL_CLOSED"
                    and default.get("classification") == UNRESOLVED
                    and default.get("sourceFidelity") == "UNRESOLVED",
                    f"implicit default was promoted: {default.get('defaultId')}",
                )
                require_exact_axes(
                    default,
                    expected_axes(
                        evidence=[
                            "SOURCE_ERA_CLASS_DEFAULT_EVIDENCE_UNAVAILABLE"
                        ],
                        execution=[expected_default[2]],
                    ),
                    str(default.get("defaultId")),
                )
                default_counts[family] += 1

            point_light_module_execution_blockers: set[str] = set()
            component = component_occurrences.get(
                (
                    evidence_id,
                    logical_package,
                    package_local_path,
                    "pointlightcomponent",
                )
            )
            if component is not None:
                legacy_occurrence_id = str(component["occurrenceId"])
                binding = point_light_by_legacy_occurrence.get(legacy_occurrence_id)
                component_definition_id = str(component.get("definitionId") or "")
                component_definition = component_definitions.get(
                    component_definition_id
                )
                require(
                    component_definition is not None,
                    f"PointLight definition is missing: {component_definition_id}",
                )
                component_identity = component.get("propertyIdentity") or {}
                component_definition_identity = (
                    f"{str(component_identity.get('logicalPackage') or '').casefold()}::"
                    f"{str(component_definition.get('targetPackageLocalPath') or '').casefold()}"
                )
                component_exact_class = str(
                    (component_definition.get("exactPayload") or {}).get(
                        "className"
                    )
                    or ""
                )
                require(
                    binding is not None
                    and binding.get("legacyReferenceId")
                    == component_definition.get("referenceId")
                    and binding.get("legacyDefinitionId")
                    == component_definition_id
                    and binding.get("legacyOccurrenceId")
                    == legacy_occurrence_id
                    and binding.get("moduleOccurrenceId") == expected_module_id
                    and binding.get("definitionId")
                    == "component-definition::" + component_definition_identity
                    and binding.get("referenceId")
                    == "component-reference::" + component_definition_identity
                    and binding.get("occurrenceId")
                    == f"{expected_module_id}::component:pointlightcomponent"
                    and component_exact_class == "pointlightcomponent"
                    and binding.get("exactSourceClass") == component_exact_class
                    and binding.get("componentBinderId")
                    == "ue3.typedata.point-light-component.v1"
                    and binding.get("sourceFidelity") == "SOURCE_EXACT"
                    and binding.get("classification") == UNRESOLVED,
                    f"PointLight binding changed: {expected_module_id}",
                )
                component_source_blockers = {
                    *list(
                        (component_definition.get("executionAdmission") or {}).get(
                            "blockers"
                        )
                        or []
                    ),
                    *list(
                        (component.get("executionAdmission") or {}).get(
                            "blockers"
                        )
                        or []
                    ),
                }
                require_exact_axes(
                    binding,
                    expected_axes_from_source_blockers(
                        component_source_blockers
                    ),
                    expected_module_id + ":point-light",
                )
                exact_field_names = {
                    "brightness",
                    "bcastcompositeshadow",
                    "baffectcompositeshadowdirection",
                    "lightguid",
                    "lightmapguid",
                }
                current_field_names = {
                    "radius",
                    "falloffexponent",
                    "lightcolor",
                }
                expected_field_names = exact_field_names | current_field_names
                actual_fields = {
                    str(row.get("fieldName") or ""): row
                    for row in binding.get("fields", [])
                }
                require(
                    len(binding.get("fields", [])) == len(actual_fields) == 8
                    and set(actual_fields) == expected_field_names,
                    "PointLight semantic field denominator changed",
                )
                for name in expected_field_names:
                    field = actual_fields[name]
                    if name in {"lightguid", "lightmapguid"}:
                        expected_selected_tier = "INSTANCE_EXPLICIT"
                        expected_evidence_status = (
                            "SOURCE_EXACT_PHYSICAL_PACKAGE"
                        )
                    else:
                        resolved_field = (
                            (component_definition.get("semanticCoverage") or {})
                            .get("resolvedFields", {})
                            .get(name, {})
                        )
                        selected_field = resolved_field.get("selected") or {}
                        expected_selected_tier = str(
                            selected_field.get("tier") or ""
                        )
                        expected_evidence_status = str(
                            selected_field.get("evidenceStatus") or ""
                        )
                    require(
                        field.get("fieldId")
                        == f"typedata-point-light-component-000/field:{name}"
                        and field.get("selectedTier") == expected_selected_tier
                        and field.get("selectedEvidenceStatus")
                        == expected_evidence_status
                        and field.get("classification") == UNRESOLVED,
                        f"PointLight field decision changed: {name}",
                    )
                    if name in current_field_names:
                        require(
                            field.get("sourceFidelity")
                            == "CURRENT_REVISION_EVIDENCE"
                            and field.get("consumerId")
                            == "source.point-light." + sanitized_id(name) + ".v1",
                            f"PointLight current default was laundered: {name}",
                        )
                        point_light_field_axes = expected_axes(
                            evidence=[
                                "POINT_LIGHT_SOURCE_ERA_DEFAULT_PROVENANCE_UNRESOLVED",
                                "SOURCE_ERA_SCRIPT_PACKAGE_IDENTITY_NOT_PINNED",
                            ],
                            execution=[
                                "POINT_LIGHT_DEFAULT_FIELD_EXECUTION_UNRESOLVED"
                            ],
                        )
                    else:
                        require(
                            field.get("sourceFidelity") == "SOURCE_EXACT"
                            and field.get("consumerId")
                            == (
                                ""
                                if name in {"lightguid", "lightmapguid"}
                                else "source.point-light."
                                + sanitized_id(name)
                                + ".v1"
                            ),
                            f"PointLight exact child field changed: {name}",
                        )
                        point_light_field_axes = expected_axes(
                            execution=(
                                [
                                    "POINT_LIGHT_GUID_RUNTIME_SEMANTICS_OR_IRRELEVANCE_UNPROVEN"
                                ]
                                if name in {"lightguid", "lightmapguid"}
                                else [
                                    "DOWNSTREAM_LIGHT_HANDLER_RECEIPT_REQUIRED",
                                    "INDEPENDENT_LIGHT_NUMERIC_OR_SEMANTIC_ORACLE_REQUIRED",
                                ]
                            )
                        )
                    require_exact_axes(
                        field,
                        point_light_field_axes,
                        str(field.get("fieldId")),
                    )
                point_light_module_execution_blockers.update(
                    binding["executionBlockers"]
                )
                point_light_module_execution_blockers.update(
                    blocker
                    for field in actual_fields.values()
                    for blocker in field["executionBlockers"]
                )
                seen_point_light_occurrences.add(legacy_occurrence_id)

            alias_risk = (
                exact_folded.startswith("efparticlemodule")
                or exact_folded.endswith("_seeded")
            )
            expected_module_decision = UNRESOLVED
            require(
                module.get("classification") == expected_module_decision
                and module.get("consumerId")
                == "source.module." + sanitized_id(exact_class) + ".v1",
                f"module decision changed: {expected_module_id}",
            )
            expected_module_execution_blockers = {
                "DOWNSTREAM_MODULE_HANDLER_RECEIPT_REQUIRED",
                *(
                    blocker
                    for field in lod_fields.values()
                    for blocker in field["executionBlockers"]
                ),
                *native["executionBlockers"],
                *(
                    seed["executionBlockers"]
                    if isinstance(seed, dict)
                    else []
                ),
                *(
                    blocker
                    for default in module.get("implicitDefaults", [])
                    for blocker in default["executionBlockers"]
                ),
                *(
                    blocker
                    for prop in report_property_rows
                    for blocker in prop["executionBlockers"]
                ),
                *point_light_module_execution_blockers,
            }
            if alias_risk:
                unapproved_class_alias_occurrence_count += 1
                expected_module_execution_blockers.add(
                    "SOURCE_CLASS_ALIAS_OR_CUSTOM_HANDLER_UNVERIFIED"
                )
            require_exact_axes(
                module,
                expected_axes(
                    evidence=(
                        ["SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"]
                        if expected_fidelity != "SOURCE_EXACT"
                        else []
                    ),
                    execution=expected_module_execution_blockers,
                ),
                expected_module_id,
            )
            module_counts[expected_module_decision] += 1

    require(
        seen_local_occurrences == set(local_binding_by_legacy_occurrence),
        "local distribution occurrences were not consumed exactly once",
    )
    require(
        seen_point_light_occurrences == set(point_light_by_legacy_occurrence),
        "PointLight occurrence was not consumed exactly once",
    )
    measured = {
        "activeOccurrenceCount": len(report_occurrences),
        "selectedLodFieldCount": sum(selected_lod_field_counts.values()),
        "orderedModuleReferenceCount": sum(module_counts.values()),
        "topLevelTaggedPropertyCount": sum(property_counts.values()),
        "primitiveLeafCount": sum(leaf_counts.values()),
        "distributionCount": sum(distribution_counts.values()),
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
        "localDistributionDefinitionCount": len(
            local_reference_closure["distributionDefinitions"]
        ),
        "localDistributionOccurrenceCount": len(local_binding_by_legacy_occurrence),
        "pointLightDefinitionCount": len(
            local_reference_closure["componentDefinitions"]
        ),
        "pointLightOccurrenceCount": len(point_light_by_legacy_occurrence),
        "externalNativeTailCount": native_tail_counts[UNRESOLVED],
        "seededModuleCount": sum(seed_counts.values()),
        "requiredLocalSpaceDefaultCount": default_counts["RequiredLocalSpace"],
        "decalDefaultCount": default_counts["Decal"],
        "ribbonDefaultCount": default_counts["Ribbon"],
        "screenPostDefaultCount": default_counts["ScreenPost"],
        "lightDefaultCount": default_counts["Light"],
    }
    require(measured == EXPECTED_DENOMINATORS, f"oracle denominator changed: {measured}")
    summary = closure.get("summary") or {}
    require(
        summary.get("denominators") == measured
        and summary.get("moduleDecisionCounts") == dict(sorted(module_counts.items()))
        and summary.get("propertyDecisionCounts")
        == dict(sorted(property_counts.items()))
        and summary.get("primitiveLeafDecisionCounts")
        == dict(sorted(leaf_counts.items()))
        and summary.get("distributionDecisionCounts")
        == dict(sorted(distribution_counts.items()))
        and summary.get("distributionFieldEvidenceCounts")
        == dict(sorted(distribution_field_evidence_counts.items()))
        and summary.get("nativeTailDecisionCounts")
        == dict(sorted(native_tail_counts.items()))
        and summary.get("seedDecisionCounts") == dict(sorted(seed_counts.items()))
        and summary.get("implicitDefaultFamilyCounts")
        == dict(sorted(default_counts.items()))
        and summary.get("moduleSourceFidelityCounts")
        == dict(sorted(source_fidelity_counts.items()))
        and summary.get("selectedLodFieldDecisionCounts")
        == dict(sorted(selected_lod_field_counts.items()))
        and summary.get("unknownDecisionCount") == 0
        and summary.get("unconsumedRowCount") == 0
        and summary.get("silentIgnoredRowCount") == 0
        and summary.get("allRowsClassified") is True
        and summary.get("semanticExecutionAdmission") is False,
        "semantic closure summary is not an independent projection of raw rows",
    )
    product = closure.get("productAdmission") or {}
    require(
        product.get("allowed") is False
        and product.get("admittedOccurrenceCount") == 0
        and product.get("totalOccurrenceCount") == 35
        and product.get("blockers")
        == [
            "DOWNSTREAM_COMPILER_HANDLER_RECEIPTS_REQUIRED",
            "SOURCE_SEMANTIC_UNRESOLVED_ROWS_REMAIN",
            "PRODUCT_ADMISSION_OWNED_BY_FINAL_INTEGRATION_GATE",
        ],
        "Product admission changed before final integration gate",
    )
    return {
        "denominators": measured,
        "moduleDecisionCounts": dict(sorted(module_counts.items())),
        "propertyDecisionCounts": dict(sorted(property_counts.items())),
        "primitiveLeafDecisionCounts": dict(sorted(leaf_counts.items())),
        "distributionDecisionCounts": dict(sorted(distribution_counts.items())),
        "productAdmission": "0/35",
    }


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Independently verify the Artist 31470 code-only semantic closure."
        )
    )
    parser.add_argument("--semantic-closure", required=True, type=Path)
    parser.add_argument("--active-inventory", required=True, type=Path)
    parser.add_argument("--normalized-graph", required=True, type=Path)
    parser.add_argument("--external-module-closure", required=True, type=Path)
    parser.add_argument("--source-evidence", required=True, type=Path)
    parser.add_argument("--local-reference-closure", required=True, type=Path)
    args = parser.parse_args()
    result = verify_semantic_closure(
        load_json(args.semantic_closure),
        load_json(args.active_inventory),
        load_json(args.normalized_graph),
        load_json(args.external_module_closure),
        load_json(args.source_evidence),
        load_json(args.local_reference_closure),
    )
    print(
        "Artist F 31470 semantic closure oracle PASS: "
        f"modules={result['denominators']['orderedModuleReferenceCount']} "
        f"properties={result['denominators']['topLevelTaggedPropertyCount']}/"
        f"{result['denominators']['primitiveLeafCount']} "
        f"distributions={result['denominators']['distributionCount']} "
        f"product={result['productAdmission']}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, StopIteration) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
