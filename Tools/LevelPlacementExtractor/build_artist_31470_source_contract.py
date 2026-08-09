#!/usr/bin/env python3

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

from build_imported_effect_documents import build_document


PROFILE_ID = "ue3CascadeSourceContractV1"
REPO_ROOT = Path(__file__).resolve().parents[2]
EXPECTED_RENDERER_COUNTS = {
    "MeshParticle": 13,
    "SpriteParticle": 16,
    "DecalParticle": 3,
    "CascadeRibbon": 1,
    "LightParticle": 1,
    "ScreenPost": 1,
}
RENDERERS = {
    "MeshParticle": ("meshParticle", "particle", "mesh", "ue3CascadeV1"),
    "SpriteParticle": ("spriteParticle", "particle", "sprite", "ue3CascadeV1"),
    "DecalParticle": ("decalParticle", "decal", "decal", "ue3CascadeV1"),
    "CascadeRibbon": ("cascadeRibbon", "trail", "ribbon", "ue3CascadeV1"),
    "LightParticle": ("lightParticle", "light", "light", "ue3CascadeV1"),
    "ScreenPost": ("screenPost", "screenPost", "screenPost", "screenSpaceV1"),
}
COVERAGE_SEVERITY = {
    "source_decoded": 0,
    "deterministic_conversion": 1,
    "metadata_only": 2,
    "unresolved": 3,
}


def load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        value = json.load(stream)
    if not isinstance(value, dict):
        raise ValueError(f"JSON root must be an object: {path}")
    return value


def json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, sort_keys=False) + "\n"
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def file_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def repository_path(path: Path) -> str:
    resolved = path.resolve()
    try:
        return resolved.relative_to(REPO_ROOT).as_posix()
    except ValueError:
        return resolved.as_posix()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def normalize_module_class(value: str) -> str:
    result = value.casefold()
    if result.startswith("efparticlemodule"):
        result = "particlemodule" + result[len("efparticlemodule") :]
    if result.endswith("_seeded"):
        result = result[: -len("_seeded")]
    return result


def source_peak(row: dict[str, Any]) -> int:
    for mapping in row.get("detailMappings", []):
        if mapping.get("target") != "particle.maxParticles":
            continue
        note = str(mapping.get("note") or "")
        match = re.search(r"(?:^|\s)source=(\d+)(?:\s|$)", note)
        if match:
            return int(match.group(1))
        value = mapping.get("value")
        if isinstance(value, int) and value >= 0:
            return value
    raise ValueError(
        "active source element has no source PeakActiveParticles evidence: "
        + str(row.get("selectedLegacyElementId"))
    )


def attachment_from_cue(cue: dict[str, Any]) -> dict[str, Any]:
    typed = cue.get("typedPayload") or {}
    attachment = typed.get("attachment") or {}
    mode = str(attachment.get("mode") or "")
    if mode == "FOLLOW_NAMED_ANCHORS":
        source_names = attachment.get("sourceAnchorNames") or []
        source_name = str(source_names[0]) if source_names else ""
        runtime_anchor = str(attachment.get("runtimeAnchorSlotId") or "")
        runtime_bone = str(attachment.get("runtimeBoneName") or "")
        transform = attachment.get("socketLocalTransform") or {}
        require(source_name != "", f"follow cue has no source anchor: {cue['cueId']}")
        require(
            runtime_anchor != "" and runtime_bone != "",
            f"follow cue has no exact runtime socket mapping: {cue['cueId']}",
        )
        require(
            attachment.get("runtimeResolutionStatus") == "EXACT_SOURCE_SOCKET",
            f"follow cue socket mapping is not source exact: {cue['cueId']}",
        )
        return {
            "enabled": True,
            "follow": True,
            "sourceAnchorSlotId": source_name,
            "runtimeAnchorSlotId": runtime_anchor,
            "runtimeBoneName": runtime_bone,
            "socketLocalTransform": {
                "position": list(transform.get("position") or [0.0, 0.0, 0.0]),
                "rotationDegrees": list(
                    transform.get("rotationDegrees") or [0.0, 0.0, 0.0]
                ),
                "scale": list(transform.get("scale") or [1.0, 1.0, 1.0]),
            },
        }
    require(
        mode in {"SNAPSHOT_ROOT", "ROOT_SNAPSHOT"},
        f"unsupported source attachment mode: {cue['cueId']} ({mode})",
    )
    return {
        "enabled": True,
        "follow": False,
        "sourceAnchorSlotId": "root",
        "runtimeAnchorSlotId": "root",
        "runtimeBoneName": "",
        "socketLocalTransform": {
            "position": [0.0, 0.0, 0.0],
            "rotationDegrees": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
        },
    }


def local_reference_index(
    closure: dict[str, Any],
) -> dict[tuple[str, int, str], dict[str, Any]]:
    result: dict[tuple[str, int, str], dict[str, Any]] = {}
    for reference in closure.get("references", []):
        if reference.get("referenceKind") != "DISTRIBUTION_TARGET":
            continue
        property_path = str(reference["propertyPath"])
        require(
            property_path.casefold().endswith(".distribution"),
            f"local distribution reference path is malformed: {property_path}",
        )
        recipe_property = property_path[: -len(".distribution")]
        for occurrence in reference["activeOccurrences"]:
            key = (
                str(occurrence["activeElementId"]),
                int(occurrence["moduleOrder"]),
                recipe_property.casefold(),
            )
            require(key not in result, f"duplicate local-reference occurrence: {key}")
            result[key] = reference
    return result


def aggregate_coverage_status(
    properties: list[dict[str, Any]], blockers: list[str]
) -> str:
    if blockers:
        return "unresolved"
    return max(
        (str(row["status"]) for row in properties),
        key=lambda status: COVERAGE_SEVERITY[status],
        default="source_decoded",
    )


def module_coverage(
    recipe: dict[str, Any],
    evidence: dict[str, Any],
    local_references: dict[tuple[str, int, str], dict[str, Any]],
    renderer_name: str,
) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    modules = recipe.get("modules", [])
    source_modules = evidence["moduleReferenceOrder"]
    require(
        len(modules) == len(source_modules),
        f"source module order changed: {evidence['evidenceId']}",
    )
    for module_order, (module, source_module) in enumerate(
        zip(modules, source_modules)
    ):
        require(
            str(module["className"]).casefold()
            == str(source_module["sourceClass"]).casefold()
            and str(module["stableId"]).casefold()
            == (
                f"{source_module['sourceObjectId']}@ref:"
                f"{source_module['sourceReferenceIndex']}"
            ).casefold(),
            f"source module identity changed: {evidence['evidenceId']}[{module_order}]",
        )
        properties = []
        blockers: list[str] = []
        seed_status = str(source_module["randomSeedStatus"])
        if seed_status != "NOT_SEEDED":
            blockers.append("RANDOM_SEED_CONSUMPTION_SEMANTICS_UNRESOLVED")
        if str(source_module["nativeTailStatus"]).startswith("UNRESOLVED"):
            blockers.append("EXTERNAL_MODULE_NATIVE_TAIL_NOT_PROVEN")
        normalized = normalize_module_class(str(module.get("className") or ""))
        literal_paths = {
            str(literal["propertyPath"]).casefold()
            for literal in module.get("literals", [])
        }
        if normalized == "particlemodulerequired" and "buselocalspace" not in literal_paths:
            blockers.append("REQUIRED_BUSELOCALSPACE_CLASS_DEFAULT_UNRESOLVED")
        if "typedatadecal" in normalized:
            blockers.append("DECAL_CLASS_DEFAULT_REGISTRY_MISSING")
        if "typedataribbon" in normalized:
            blockers.append("RIBBON_CLASS_DEFAULT_REGISTRY_MISSING")
        if "typedatalight" in normalized:
            blockers.append("POINT_LIGHT_CLASS_DEFAULTS_UNRESOLVED")
        if renderer_name == "ScreenPost" and normalized == "particlemodulerequired":
            blockers.append("SCREEN_POST_CLASS_DEFAULT_REGISTRY_MISSING")
        for literal in module.get("literals", []):
            path = str(literal["propertyPath"])
            folded_path = path.casefold()
            status = "source_decoded"
            provenance = "SOURCE_TAGGED_PRIMITIVE"
            if folded_path.endswith(".objectpath") or folded_path.endswith(".assetid"):
                status = "metadata_only"
                provenance = "DETERMINISTIC_REFERENCE_METADATA_JOIN"
            elif folded_path.startswith("randomseedinfo."):
                status = (
                    "unresolved"
                    if seed_status == "CLASS_DEFAULT_UNRESOLVED"
                    else "metadata_only"
                )
                provenance = seed_status
            properties.append(
                {
                    "propertyPath": path,
                    "storage": "literal",
                    "status": status,
                    "provenance": provenance,
                }
            )
        for distribution in module.get("distributions", []):
            path = str(distribution["propertyPath"])
            reference = local_references.get(
                (evidence["evidenceId"], module_order, path.casefold())
            )
            status = "deterministic_conversion"
            provenance = "RAW_DISTRIBUTION_TO_TYPED_SOURCE_RECIPE"
            if reference is not None and (
                str(reference["targetPayloadStatus"]).startswith("UNRESOLVED")
                or str(reference["targetSemanticCoverage"]["status"]).startswith(
                    "UNRESOLVED"
                )
            ):
                status = "unresolved"
                provenance = "OBJECT_REFERENCE_TARGET_OR_CLASS_DEFAULT_UNRESOLVED"
                blockers.append(
                    "DISTRIBUTION_TARGET_OR_PARTICLE_PARAMETER_SEMANTICS_UNRESOLVED"
                )
            properties.append(
                {
                    "propertyPath": path,
                    "storage": "distribution",
                    "status": status,
                    "provenance": provenance,
                }
            )
        properties.sort(key=lambda item: (item["propertyPath"], item["storage"]))
        blockers = sorted(set(blockers))
        rows.append(
            {
                "moduleStableId": module["stableId"],
                "normalizedClass": normalized,
                "status": aggregate_coverage_status(properties, blockers),
                "blockers": blockers,
                "properties": properties,
            }
        )
    return rows


def planned_phase(normalized_class: str) -> str:
    if normalized_class == "particlemodulespawn":
        return "spawn"
    if "typedata" in normalized_class or normalized_class == "particlemodulerequired":
        return "type_data"
    if any(
        token in normalized_class
        for token in (
            "overlife",
            "multiplylife",
            "rotationrate",
            "orbit",
            "attractor",
            "acceleration",
        )
    ):
        return "update"
    return "spawn"


def unit_rule_ids(
    normalized_class: str, property_path: str, renderer_types: list[str]
) -> list[str]:
    folded_path = property_path.casefold()
    if "acceleration" in normalized_class:
        return ["ue3.acceleration_cm_per_s2_to_client_m_per_s2"]
    if "velocity" in normalized_class:
        return ["ue3.velocity_cm_per_s_to_client_m_per_s"]
    if "location" in normalized_class:
        return ["ue3.position_cm_to_client_m"]
    if "size" in normalized_class and "size" in folded_path:
        rules = []
        for renderer in renderer_types:
            rules.append(
                {
                    "MeshParticle": "mesh_particle.dimensionless_axis_reorder",
                    "SpriteParticle": "sprite_particle.size_cm_to_m",
                    "DecalParticle": "decal_particle.size_cm_to_m",
                    "CascadeRibbon": "cascade_ribbon.width_cm_to_m_semantics_pending",
                    "LightParticle": "light_particle.size_parameter_semantics_unresolved",
                    "ScreenPost": "screen_post.size_semantics_unresolved",
                }[renderer]
            )
        return sorted(set(rules))
    if "rotation" in normalized_class:
        return ["ue3.rotation_turns_axis_mapping_pending"]
    return ["dimensionless_or_metadata"]


def build_registry(
    elements: list[dict[str, Any]], evidence_links: dict[str, Any]
) -> dict[str, Any]:
    classes: dict[str, dict[str, Any]] = {}
    for element in elements:
        for module in element["sourceRecipe"].get("modules", []):
            original_class = str(module.get("className") or "")
            normalized = normalize_module_class(original_class)
            require(normalized != "", "source module class is empty")
            row = classes.setdefault(
                normalized,
                {
                    "normalizedClass": normalized,
                    "sourceClasses": set(),
                    "rendererTypes": set(),
                    "literalProperties": defaultdict(set),
                    "distributionProperties": defaultdict(
                        lambda: {"componentCounts": set(), "sourceClasses": set()}
                    ),
                },
            )
            row["sourceClasses"].add(original_class)
            renderer_name = next(
                name
                for name, values in RENDERERS.items()
                if values[0] == element["renderer"]["type"]
            )
            row["rendererTypes"].add(renderer_name)
            for literal in module.get("literals", []):
                row["literalProperties"][literal["propertyPath"]].add(
                    literal["kind"]
                )
            for distribution in module.get("distributions", []):
                prop = row["distributionProperties"][distribution["propertyPath"]]
                prop["componentCounts"].add(int(distribution["componentCount"]))
                source_class = str(distribution.get("sourceClass") or "")
                if source_class:
                    prop["sourceClasses"].add(source_class)

    module_rows = []
    for normalized, row in sorted(classes.items()):
        literal_rows = [
            {"propertyPath": path, "kinds": sorted(kinds)}
            for path, kinds in sorted(row["literalProperties"].items())
        ]
        distribution_rows = [
            {
                "propertyPath": path,
                "componentCounts": sorted(values["componentCounts"]),
                "sourceClasses": sorted(values["sourceClasses"]),
            }
            for path, values in sorted(row["distributionProperties"].items())
        ]
        module_rows.append(
            {
                "normalizedClass": normalized,
                "sourceClasses": sorted(row["sourceClasses"]),
                "rendererTypes": sorted(row["rendererTypes"]),
                "literalProperties": literal_rows,
                "distributionProperties": distribution_rows,
                "runtimeImplemented": False,
            }
        )

    field_rules = []
    for row in module_rows:
        normalized = row["normalizedClass"]
        renderer_types = row["rendererTypes"]
        for storage_name, properties in (
            ("literal", row["literalProperties"]),
            ("distribution", row["distributionProperties"]),
        ):
            for property_row in properties:
                property_path = property_row["propertyPath"]
                stable_path = re.sub(
                    r"[^a-z0-9]+", ".", property_path.casefold()
                ).strip(".")
                phase = planned_phase(normalized)
                field_rules.append(
                    {
                        "ruleId": f"field.{normalized}.{stable_path}.{storage_name}",
                        "normalizedClass": normalized,
                        "propertyPath": property_path,
                        "storage": storage_name,
                        "rendererApplicability": renderer_types,
                        "plannedPhase": phase,
                        "plannedHandlerId": f"source.{normalized}.{stable_path}",
                        "plannedOpcodeId": "UNASSIGNED_TRACK_A_COMPILED_EXECUTION",
                        "unitRuleIds": unit_rule_ids(
                            normalized, property_path, renderer_types
                        ),
                        "timeDomain": (
                            "particle_normalized_life"
                            if phase == "update"
                            else "emitter_or_spawn_time"
                            if phase == "spawn"
                            else "metadata"
                        ),
                        "classDefaultPolicy": "EXPLICIT_ONLY_UNRESOLVED_IF_ABSENT",
                        "seedConsumption": (
                            "UNRESOLVED"
                            if normalized.endswith("_seeded")
                            or property_path.casefold().startswith("randomseedinfo")
                            else "NOT_APPLICABLE"
                        ),
                        "runtimeImplemented": False,
                    }
                )

    registry: dict[str, Any] = {
        "schema": "lostark.effect-source-contract-registry",
        "formatVersion": 1,
        "profileId": PROFILE_ID,
        "effectDocumentVersion": 14,
        "runtimeAdmission": False,
        "sourceEvidenceStatus": "SOURCE_EVIDENCE_PARTIAL",
        "evidenceLinks": copy.deepcopy(evidence_links),
        "coverageSeed": {
            "characterClass": "Artist",
            "skillId": 31470,
            "activeElementCount": len(elements),
        },
        "rendererCategories": [
            {
                "rendererType": runtime_type,
                "sourceSpace": source_space,
                "compatibilityKind": kind,
                "runtimeImplemented": False,
            }
            for _, (runtime_type, kind, _, source_space) in RENDERERS.items()
        ],
        "coverageStatusTokens": [
            "source_decoded",
            "deterministic_conversion",
            "metadata_only",
            "unresolved",
        ],
        "coordinateAndScaleRules": [
            {
                "ruleId": "ue3.position_cm_to_client_m",
                "sourceUnit": "centimeter",
                "targetUnit": "meter",
                "scale": 0.01,
                "axisMapping": "(X,Y,Z)->(X,Z,-Y)",
            },
            {
                "ruleId": "ue3.velocity_cm_per_s_to_client_m_per_s",
                "sourceUnit": "centimeter_per_second",
                "targetUnit": "meter_per_second",
                "scale": 0.01,
                "axisMapping": "(X,Y,Z)->(X,Z,-Y)",
            },
            {
                "ruleId": "ue3.acceleration_cm_per_s2_to_client_m_per_s2",
                "sourceUnit": "centimeter_per_second_squared",
                "targetUnit": "meter_per_second_squared",
                "scale": 0.01,
                "axisMapping": "(X,Y,Z)->(X,Z,-Y)",
            },
            {
                "ruleId": "sprite_particle.size_cm_to_m",
                "sourceUnit": "centimeter",
                "targetUnit": "meter",
                "scale": 0.01,
                "axisMapping": "(X,Y,Z)->(X,Z,Y)",
            },
            {
                "ruleId": "decal_particle.size_cm_to_m",
                "sourceUnit": "centimeter",
                "targetUnit": "meter",
                "scale": 0.01,
                "axisMapping": "(X,Y,Z)->(X,Z,Y)",
            },
            {
                "ruleId": "mesh_particle.dimensionless_axis_reorder",
                "sourceUnit": "dimensionless_multiplier",
                "targetUnit": "dimensionless_multiplier",
                "scale": 1.0,
                "axisMapping": "(X,Y,Z)->(X,Z,Y)",
                "separateCarrierRuleId": "wmodel.carrier_geometry_prescale",
            },
            {
                "ruleId": "wmodel.carrier_geometry_prescale",
                "sourceGeometryRelation": "glTF*diag(100,100,-100)",
                "carrierGeometryPreScale": 0.01,
                "particleScaleIncluded": False,
            },
        ],
        "compositionOrder": [
            "carrierGeometryPreScale",
            "signedParticleScaleRotationLocation",
            "emitterElementTransform",
            "cueLocalTransform",
            "attachmentSocketOrRoot",
            "actorWorld",
        ],
        "legacyProjection": {
            "detailTransformIsCompilerInput": False,
            "legacyImplicitMeshScale01IsCompilerInput": False,
        },
        "moduleClasses": module_rows,
        "fieldRules": field_rules,
        "contractSha256": "",
    }
    unsigned = copy.deepcopy(registry)
    unsigned.pop("contractSha256")
    registry["contractSha256"] = canonical_sha256(unsigned)
    return registry


def build_header(registry: dict[str, Any]) -> bytes:
    text = (
        "#pragma once\n\n"
        "#include <string_view>\n\n"
        "namespace Client\n"
        "{\n"
        "inline constexpr std::string_view EFFECT_SOURCE_CONTRACT_PROFILE_ID =\n"
        f'\t"{registry["profileId"]}";\n'
        "inline constexpr std::string_view EFFECT_SOURCE_CONTRACT_SHA256 =\n"
        f'\t"{registry["contractSha256"]}";\n'
        "}\n"
    )
    return text.encode("utf-8")


def build_source_contract(
    source_receipt_path: Path,
    action_cue_recipe_path: Path,
    active_inventory_path: Path,
    normalized_graph_path: Path,
    module_closure_path: Path,
    material_closure_path: Path,
    source_evidence_path: Path,
    local_reference_closure_path: Path,
    geometry_parity_path: Path,
    candidate_artifact_name: str,
    registry_artifact_name: str,
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    source_receipt = load_json(source_receipt_path)
    action_recipe = load_json(action_cue_recipe_path)
    inventory = load_json(active_inventory_path)
    graph = load_json(normalized_graph_path)
    module_closure = load_json(module_closure_path)
    material_closure = load_json(material_closure_path)
    source_evidence = load_json(source_evidence_path)
    local_reference_closure = load_json(local_reference_closure_path)
    geometry_parity = load_json(geometry_parity_path)

    require(source_receipt.get("skillId") == 31470, "source receipt skill mismatch")
    require(source_receipt.get("inputSlot") == "R", "Imported receipt was modified")
    require(action_recipe.get("inputSlot") == "F", "Derived action recipe is not F")
    require(inventory.get("inputSlot") == "F", "Derived inventory is not F")
    require(inventory.get("productEmissionAllowed") is False, "inventory is not fail-closed")
    require(
        source_evidence.get("status") == "SOURCE_EVIDENCE_PARTIAL"
        and source_evidence.get("skillId") == 31470,
        "source evidence envelope is not fail-closed",
    )
    require(
        local_reference_closure.get("skillId") == 31470
        and geometry_parity.get("skillId") == 31470,
        "linked source evidence skill mismatch",
    )
    for artifact, self_field in (
        (source_evidence, "evidenceSha256"),
        (local_reference_closure, "closureSha256"),
        (geometry_parity, "receiptSha256"),
    ):
        unsigned = copy.deepcopy(artifact)
        expected = str(unsigned.pop(self_field))
        require(
            expected == canonical_sha256(unsigned),
            f"linked source evidence self hash is invalid: {self_field}",
        )

    legacy, conversion = build_document(
        source_receipt,
        graph,
        module_closure,
        include_source_contract_bindings=True,
    )
    legacy_by_id = {element["id"]: element for element in legacy["elements"]}
    cue_by_id = {cue["cueId"]: cue for cue in action_recipe["cues"]}
    evidence_by_id = {
        row["evidenceId"]: row for row in source_evidence["occurrences"]
    }
    local_references = local_reference_index(local_reference_closure)
    geometry_by_asset = {
        row["assetId"]: row for row in geometry_parity["assets"]
    }
    active_rows = inventory["activeElements"]
    require(len(active_rows) == 35, "active inventory count changed")
    require(
        Counter(row["rendererType"] for row in active_rows)
        == Counter(EXPECTED_RENDERER_COUNTS),
        "active renderer counts changed",
    )

    elements = []
    receipt_elements = []
    for row in active_rows:
        element_id = row["selectedLegacyElementId"]
        renderer_name = row["rendererType"]
        require(element_id in legacy_by_id, f"legacy element is missing: {element_id}")
        require(renderer_name in RENDERERS, f"unknown renderer: {renderer_name}")
        runtime_type, kind, shape, source_space = RENDERERS[renderer_name]
        element = copy.deepcopy(legacy_by_id[element_id])
        cue = cue_by_id[row["cueId"]]
        evidence = evidence_by_id.get(row["activeElementId"])
        require(evidence is not None, f"source evidence row is missing: {row['activeElementId']}")
        require(
            evidence["sourceCueId"] == row["cueId"]
            and evidence["sourceEmitterPath"] == row["sourceEmitter"],
            f"source evidence occurrence join changed: {row['activeElementId']}",
        )

        element["kind"] = kind
        element["renderer"] = {"type": runtime_type, "sourceSpace": source_space}
        element["actionCueAttachment"] = attachment_from_cue(cue)
        element["transformInheritance"] = {"enabled": False, "masterElementId": ""}
        element["material"]["sourceProfile"] = {"enabled": False}

        recipe = element["sourceRecipe"]
        recipe["rendererShape"] = shape
        recipe["sourceContractProfileId"] = PROFILE_ID
        recipe["sourceContractSha256"] = ""
        recipe["sourceGraphSha256"] = file_sha256(normalized_graph_path)
        recipe["sourceClosureSha256"] = file_sha256(module_closure_path)
        recipe["sourceMaterialClosureSha256"] = file_sha256(material_closure_path)
        recipe["sourcePeakActiveParticles"] = source_peak(row)
        recipe["moduleCoverage"] = module_coverage(
            recipe,
            evidence,
            local_references,
            renderer_name,
        )
        recipe["compilerEvidence"] = {
            "artifactFileSha256": file_sha256(source_evidence_path),
            "artifactSelfSha256": source_evidence["evidenceSha256"],
            "evidenceId": evidence["evidenceId"],
            "sourceEvidenceStatus": source_evidence["status"],
            "sourceCueId": evidence["sourceCueId"],
            "sourceOccurrenceId": evidence["sourceOccurrenceId"],
            "sourceSystemId": evidence["sourceSystemId"],
            "sourceEmitterPath": evidence["sourceEmitterPath"],
            "sourceEmitterNodeId": evidence["sourceEmitterNodeId"],
            "lodSelectionPolicy": evidence["lodSelectionPolicy"],
            "selectedLodPath": evidence["selectedLod"]["sourceLodPath"],
            "selectedLodNodeId": evidence["selectedLod"]["sourceLodNodeId"],
            "selectedLodArrayIndex": evidence["selectedLod"]["emitterArrayIndex"],
            "selectedLodLevelProvenance": evidence["selectedLod"][
                "levelProvenance"
            ],
            "selectedLodEnabledProvenance": evidence["selectedLod"][
                "enabledProvenance"
            ],
            "nonSelectedLodCount": len(evidence["nonSelectedLods"]),
            "moduleReferenceOrder": [
                {
                    name: module[name]
                    for name in (
                        "order",
                        "sourceReferenceIndex",
                        "role",
                        "sourceObjectId",
                        "sourceRecordSha256",
                    )
                }
                for module in evidence["moduleReferenceOrder"]
            ],
            "cueLocalTransform": copy.deepcopy(evidence["cueLocalTransform"]),
            "parameterOverrides": copy.deepcopy(evidence["parameterOverrides"]),
            "compositionOrder": copy.deepcopy(evidence["compositionOrder"]),
            "localReferenceClosureFileSha256": file_sha256(
                local_reference_closure_path
            ),
            "localReferenceClosureSelfSha256": local_reference_closure[
                "closureSha256"
            ],
            "geometryParityFileSha256": file_sha256(geometry_parity_path),
            "geometryParitySelfSha256": geometry_parity["receiptSha256"],
        }

        module_blockers = sorted(
            {
                blocker
                for coverage in recipe["moduleCoverage"]
                for blocker in coverage["blockers"]
            }
        )
        recipe["compiledExecutionAdmission"] = {
            "allowed": False,
            "blockers": sorted(
                {
                    "SOURCE_EVIDENCE_PARTIAL",
                    "TRACK_A_COMPILED_EXECUTION_NOT_IMPLEMENTED",
                    *module_blockers,
                }
            ),
        }

        source_materials = copy.deepcopy(row.get("sourceMaterials", []))
        non_render_builtin = (
            renderer_name == "LightParticle"
            and source_materials == ["enginematerials.defaultparticle"]
        )
        recipe["materialAdmission"] = {
            "status": (
                "NON_RENDER_BUILTIN_NOT_APPLICABLE"
                if non_render_builtin
                else "BLOCKED_COOKED_PARTIAL_NO_TYPED_MATERIAL_RECIPE"
            ),
            "sourceMaterialPaths": source_materials,
            "materialRecipeId": "",
            "renderStateRecipeId": "",
            "blockers": (
                []
                if non_render_builtin
                else [
                    "TYPED_MATERIAL_RECIPE_MISSING",
                    "RENDER_STATE_RECIPE_MISSING",
                    "COOKED_PARTIAL_IS_NOT_SHADER_EXACT",
                ]
            ),
        }

        mesh_assets = [
            resource["assetId"]
            for resource in element.get("resources", [])
            if resource["slotId"] == "meshModel"
        ]
        require(len(mesh_assets) <= 1, f"element has duplicate mesh carriers: {element_id}")
        if mesh_assets:
            geometry = geometry_by_asset.get(mesh_assets[0])
            require(geometry is not None, f"geometry parity row is missing: {mesh_assets[0]}")
            recipe["geometryBinding"] = {
                "enabled": True,
                "assetId": mesh_assets[0],
                "receiptFileSha256": file_sha256(geometry_parity_path),
                "receiptSelfSha256": geometry_parity["receiptSha256"],
                "carrierGeometryPreScale": geometry["scaleContract"][
                    "carrierGeometryPreScale"
                ],
                "particleScaleSemantics": geometry["scaleContract"][
                    "particleMeshStartSizeSemantics"
                ],
                "status": "GLTF_TO_WMODEL_PARITY_PROVEN_UPK_TO_GLTF_UNRESOLVED",
                "blockers": copy.deepcopy(geometry["admission"]["blockers"]),
            }
        else:
            recipe["geometryBinding"] = {
                "enabled": False,
                "assetId": "",
                "receiptFileSha256": file_sha256(geometry_parity_path),
                "receiptSelfSha256": geometry_parity["receiptSha256"],
                "carrierGeometryPreScale": 1.0,
                "particleScaleSemantics": "NOT_APPLICABLE",
                "status": "NOT_APPLICABLE",
                "blockers": [],
            }

        elements.append(element)
        receipt_elements.append(
            {
                "elementId": element_id,
                "cueId": row["cueId"],
                "rendererType": renderer_name,
                "sourceSystemId": row["sourceSystemId"],
                "sourceEmitter": row["sourceEmitter"],
                "sourceLod": row["sourceLod"],
                "sourcePeakActiveParticles": recipe["sourcePeakActiveParticles"],
                "sourceMaterials": copy.deepcopy(row.get("sourceMaterials", [])),
                "attachment": copy.deepcopy(element["actionCueAttachment"]),
                "sourceOccurrenceId": evidence["sourceOccurrenceId"],
                "cueLocalTransform": copy.deepcopy(evidence["cueLocalTransform"]),
                "parameterOverrides": copy.deepcopy(evidence["parameterOverrides"]),
                "coverageStatus": (
                    "UNRESOLVED"
                    if any(
                        coverage["status"] == "unresolved"
                        for coverage in recipe["moduleCoverage"]
                    )
                    else "PARTIAL"
                ),
                "assemblyStatus": "MANUAL_MASTER_ASSEMBLY_PENDING",
            }
        )

    evidence_links = {
        "sourceEvidence": {
            "path": repository_path(source_evidence_path),
            "fileSha256": file_sha256(source_evidence_path),
            "selfSha256": source_evidence["evidenceSha256"],
        },
        "localReferenceClosure": {
            "path": repository_path(local_reference_closure_path),
            "fileSha256": file_sha256(local_reference_closure_path),
            "selfSha256": local_reference_closure["closureSha256"],
        },
        "wmodelGeometryParity": {
            "path": repository_path(geometry_parity_path),
            "fileSha256": file_sha256(geometry_parity_path),
            "selfSha256": geometry_parity["receiptSha256"],
        },
    }
    registry = build_registry(elements, evidence_links)
    for element in elements:
        element["sourceRecipe"]["sourceContractSha256"] = registry[
            "contractSha256"
        ]

    candidate = {
        "schema": "lostark.effect-authoring",
        "version": 14,
        "purpose": "source_contract",
        "effectAssetId": "effect.artist.skill.31470.native-v14.source-contract-candidate",
        "displayName": "Artist F 31470 Source Contract Candidate",
        "particleSystem": copy.deepcopy(legacy.get("particleSystem", {})),
        "modelCues": copy.deepcopy(legacy.get("modelCues", [])),
        "elements": elements,
    }

    material_status_counts = Counter(
        str((row.get("shaderGraph") or {}).get("topologyStatus") or "NON_RENDER_BUILTIN")
        for row in inventory["materialEvidence"]
    )
    receipt = {
        "schema": "lostark.effect-source-contract-candidate-receipt",
        "formatVersion": 1,
        "characterClass": "Artist",
        "skillId": 31470,
        "derivedInputSlot": "F",
        "importedReceiptInputSlot": source_receipt["inputSlot"],
        "status": "SOURCE_EXTRACTED",
        "aggregateSourceEvidenceStatus": "SOURCE_EVIDENCE_PARTIAL",
        "manualAssemblyStatus": "MANUAL_MASTER_ASSEMBLY_PENDING",
        "visualApprovalStatus": "NOT_VISUAL_APPROVED",
        "runtimeAdmission": "NOT_IMPLEMENTED_IN_THIS_SLICE",
        "productAdmission": {
            "allowed": False,
            "reason": "NATIVE_V14_SOURCE_CONTRACT_IS_NOT_RUNTIME_EXECUTABLE",
            "publisherStillAcceptsVersions": [5, 6, 7, 8, 9, 10, 11, 12],
        },
        "source": {
            "sourceReceipt": {
                "path": repository_path(source_receipt_path),
                "sha256": file_sha256(source_receipt_path),
            },
            "actionCueRecipe": {
                "path": repository_path(action_cue_recipe_path),
                "sha256": file_sha256(action_cue_recipe_path),
            },
            "activeInventory": {
                "path": repository_path(active_inventory_path),
                "sha256": file_sha256(active_inventory_path),
            },
            "normalizedGraph": {
                "path": repository_path(normalized_graph_path),
                "sha256": file_sha256(normalized_graph_path),
            },
            "moduleClosure": {
                "path": repository_path(module_closure_path),
                "sha256": file_sha256(module_closure_path),
            },
            "materialClosure": {
                "path": repository_path(material_closure_path),
                "sha256": file_sha256(material_closure_path),
            },
            "sourceEvidence": copy.deepcopy(evidence_links["sourceEvidence"]),
            "localReferenceClosure": copy.deepcopy(
                evidence_links["localReferenceClosure"]
            ),
            "wmodelGeometryParity": copy.deepcopy(
                evidence_links["wmodelGeometryParity"]
            ),
        },
        "candidate": {
            "artifactName": candidate_artifact_name,
            "sha256": hashlib.sha256(json_bytes(candidate)).hexdigest(),
            "registryArtifactName": registry_artifact_name,
            "registrySha256": hashlib.sha256(json_bytes(registry)).hexdigest(),
            "contractSha256": registry["contractSha256"],
        },
        "elements": receipt_elements,
        "boundaries": {
            "wmodel": "GEOMETRY_ONLY",
            "wmodelParityScope": "SELECTED_GLTF_BIN_TO_CURRENT_WMODEL_ONLY",
            "upkToUmodelGltf": "UNRESOLVED_AND_COMPILED_ADMISSION_BLOCKED",
            "cascadePlacement": "SOURCE_EVIDENCE_NOT_FINAL_ASSEMBLY",
            "materialRuntime": "COOKED_PARTIAL_NOT_SHADER_EXACT",
            "playbackRendererShader": "OWNED_BY_TRACK_A_AFTER_SOURCE_CONTRACT_MERGE",
            "productCatalog": "UNCHANGED_AND_BLOCKED",
        },
        "summary": {
            "activeCueCount": len(inventory["activeCues"]),
            "activeElementCount": len(elements),
            "rendererCounts": dict(Counter(row["rendererType"] for row in active_rows)),
            "excludedExecutionDisabledElementCount": inventory["summary"][
                "excludedExecutionDisabledLegacyElementCount"
            ],
            "materialCount": len(inventory["materialEvidence"]),
            "materialShaderTopologyStatusCounts": dict(material_status_counts),
            "sourceContractRuntimeAdmission": False,
            "visualApprovalComplete": False,
            "aggregateSourceEvidenceStatus": "SOURCE_EVIDENCE_PARTIAL",
            "coverageDenominators": copy.deepcopy(source_evidence["summary"]),
            "localReferenceDenominators": copy.deepcopy(
                local_reference_closure["summary"]
            ),
            "geometryParityDenominators": copy.deepcopy(
                geometry_parity["summary"]
            ),
        },
        "legacyProjectionReceiptSummary": copy.deepcopy(conversion.get("summary", {})),
    }
    return candidate, receipt, registry


def check_or_write(path: Path, content: bytes, check: bool) -> None:
    if check:
        if not path.is_file() or path.read_bytes() != content:
            raise ValueError(f"generated output is stale: {path}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(content)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build the Artist F 31470 native-v14 source contract candidate."
    )
    parser.add_argument("--source-receipt", required=True, type=Path)
    parser.add_argument("--action-cue-recipe", required=True, type=Path)
    parser.add_argument("--active-inventory", required=True, type=Path)
    parser.add_argument("--normalized-graph", required=True, type=Path)
    parser.add_argument("--module-closure", required=True, type=Path)
    parser.add_argument("--material-closure", required=True, type=Path)
    parser.add_argument("--source-evidence", required=True, type=Path)
    parser.add_argument("--local-reference-closure", required=True, type=Path)
    parser.add_argument("--geometry-parity", required=True, type=Path)
    parser.add_argument("--output-candidate", required=True, type=Path)
    parser.add_argument("--output-receipt", required=True, type=Path)
    parser.add_argument("--output-registry", required=True, type=Path)
    parser.add_argument("--output-header", required=True, type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    candidate, receipt, registry = build_source_contract(
        args.source_receipt,
        args.action_cue_recipe,
        args.active_inventory,
        args.normalized_graph,
        args.module_closure,
        args.material_closure,
        args.source_evidence,
        args.local_reference_closure,
        args.geometry_parity,
        args.output_candidate.name,
        args.output_registry.name,
    )
    outputs = (
        (args.output_candidate, json_bytes(candidate)),
        (args.output_receipt, json_bytes(receipt)),
        (args.output_registry, json_bytes(registry)),
        (args.output_header, build_header(registry)),
    )
    for path, content in outputs:
        check_or_write(path, content, args.check)
    print(
        "Artist F 31470 source contract "
        f"{'check' if args.check else 'write'}: "
        f"elements={receipt['summary']['activeElementCount']} "
        f"renderers={receipt['summary']['rendererCounts']} "
        "sourceEvidence=partial runtimeAdmission=false productAdmission=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ValueError as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
