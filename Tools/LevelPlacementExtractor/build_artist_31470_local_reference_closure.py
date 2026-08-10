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

from effect_source_contract_io import (
    generated_text_matches,
    raw_file_sha256,
    tracked_text_sha256,
)
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
CURRENT_EFGAME_SCRIPT = (
    "NU1V7NCQ4YAE9ZPJVNOQS.u",
    893410,
    "620a21b9ca6800b6179e2fda07dc28491747d8a44eafdd666e133bed2a5c81cc",
)
CURRENT_ENGINE_SCRIPT = (
    "NE1FENCQ4UNE9ZPRENOQS.u",
    1396327,
    "cee4257abe9a60730d48bab16e742f12123c71dd7f13faf7807c14647e989434",
)
CURRENT_DEFAULT_RECOVERY_PACKAGES = (
    (
        "BFX_HIGH_00",
        "5XFB2CBI4B2N00FTC5IS5EH0.upk",
        1896589,
        "8b6a79f9519fdbe7f3db69e39e6ea69149f391f66437954347743d673c8130dc",
        "explosion.par_q_isbc_jump_02.efparticlemodulelocationonground_4."
        "distributionfloatparticleparameter_0",
    ),
    (
        "FX_BS_02",
        "XFH2R5G2R0EF0Z4E90QX0TSQ.upk",
        506836,
        "0247bf1a4e4c24412aadaa0eb41b02ddac2f97bdaf49e9fb20782f578ce2ccbb",
        "item.par_g_gadgetskill_001.particlemodulespawn_16."
        "distributionfloatparticleparameter_33",
    ),
    (
        "FX_PC_FLM_01",
        "YGI3SWD3SY4B3D18G9KMHNT6M.upk",
        338425,
        "8565cb6832be4f5421900adb43dd9a6cbb3e740df0b803f1cbe15f33f596d469",
        "par_m_flm_ribbon_02.particlemodulelifetime_0."
        "distributionfloatparticleparameter_0",
    ),
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


def optional_source_package(
    receipt: dict[str, Any], logical_package: str
) -> dict[str, Any] | None:
    matches = [
        row
        for row in receipt.get("sourcePackages", [])
        if str(row.get("logicalPackage", "")).casefold()
        == logical_package.casefold()
    ]
    require(len(matches) <= 1, f"duplicate source package receipt: {logical_package}")
    return matches[0] if matches else None


def external_package(
    closure: dict[str, Any], logical_package: str
) -> dict[str, Any] | None:
    matches = [
        row
        for row in closure.get("packages", [])
        if str(row.get("logicalPackage", "")).casefold()
        == logical_package.casefold()
    ]
    require(len(matches) <= 1, f"duplicate external package row: {logical_package}")
    return matches[0] if matches else None


def find_object(graph: dict[str, Any], object_path: str) -> dict[str, Any]:
    matches = [
        row
        for row in graph.get("objects", [])
        if str(row.get("objectPath", "")).casefold() == object_path.casefold()
    ]
    require(len(matches) == 1, f"decoded graph object is missing: {object_path}")
    return matches[0]


def find_normalized_node(graph: dict[str, Any], node_id: str) -> dict[str, Any]:
    matches = [
        row for row in graph.get("nodes", []) if str(row.get("nodeId")) == node_id
    ]
    require(len(matches) == 1, f"normalized graph node is missing: {node_id}")
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


def module_reference_projection(value: dict[str, Any]) -> dict[str, Any]:
    return {
        "className": copy.deepcopy(value.get("className")),
        "objectName": copy.deepcopy(value.get("objectName")),
        "objectPath": copy.deepcopy(value.get("objectPath")),
        "properties": copy.deepcopy(value.get("properties")),
        "references": [
            {
                "property": str(reference.get("property") or "").casefold(),
                "packageIndex": int(reference.get("packageIndex") or 0),
                "objectPath": str(reference.get("objectPath") or "").casefold(),
            }
            for reference in value.get("references") or []
        ],
    }


def canonical_source_node_references(
    graph: dict[str, Any], source_node_id: str
) -> list[dict[str, Any]]:
    """Deduplicate occurrence-level unresolved rows into one module reference set."""
    references: dict[tuple[str, int, str], dict[str, Any]] = {}
    for system in graph.get("sourceSystems") or []:
        for reference in system.get("unresolvedExternalReferences") or []:
            if str(reference.get("sourceNodeId") or "") != source_node_id:
                continue
            normalized = {
                "property": str(reference.get("property") or "").casefold(),
                "packageIndex": int(reference.get("packageIndex") or 0),
                "objectPath": str(reference.get("objectPath") or ""),
            }
            identity = (
                normalized["property"],
                normalized["packageIndex"],
                normalized["objectPath"].casefold(),
            )
            references[identity] = normalized
    return [references[key] for key in sorted(references)]


def admission_from_blockers(blockers: Iterable[str]) -> dict[str, Any]:
    tokens = sorted({str(blocker) for blocker in blockers if str(blocker)})
    return {"allowed": not tokens, "blockers": tokens}


def tagged_value(value: Any, value_type: str) -> dict[str, Any]:
    return {"type": value_type, "structType": None, "value": copy.deepcopy(value)}


def property_casefold_index(properties: dict[str, Any]) -> dict[str, tuple[str, Any]]:
    return {str(name).casefold(): (str(name), value) for name, value in properties.items()}


def resolve_inheritance_fields(
    sources: list[dict[str, Any]],
    field_names: Iterable[str],
) -> dict[str, Any]:
    """Resolve one UE3 property chain in explicit -> archetype -> CDO -> evaluator order."""
    result: dict[str, Any] = {}
    source_indexes = [
        property_casefold_index(source.get("properties") or {}) for source in sources
    ]
    for requested_name in field_names:
        folded_name = requested_name.casefold()
        checked = []
        resolved = None
        for source, index in zip(sources, source_indexes):
            checked.append(
                {
                    "tier": source["tier"],
                    "sourceId": source["sourceId"],
                    "evidenceStatus": source["evidenceStatus"],
                    "hasField": folded_name in index,
                }
            )
            if resolved is None and folded_name in index:
                source_name, value = index[folded_name]
                resolved = {
                    "requestedField": requested_name,
                    "sourceField": source_name,
                    "value": copy.deepcopy(value),
                    "tier": source["tier"],
                    "sourceId": source["sourceId"],
                    "evidenceStatus": source["evidenceStatus"],
                }
        result[folded_name] = {
            "status": "RESOLVED" if resolved is not None else "UNRESOLVED",
            "selected": resolved,
            "checkedSources": checked,
        }
    return result


def resolved_property_value(fields: dict[str, Any], name: str) -> Any:
    selected = fields.get(name.casefold(), {}).get("selected")
    if not isinstance(selected, dict):
        return None
    value = selected.get("value")
    return value.get("value") if isinstance(value, dict) else None


def standard_particle_parameter_class(class_name: str) -> bool:
    return class_name.casefold() in {
        "distributionfloatparticleparameter",
        "distributionvectorparticleparameter",
    }


def custom_particle_parameter_class(class_name: str) -> bool:
    folded = class_name.casefold()
    return "particleparameter" in folded and not standard_particle_parameter_class(folded)


def expected_override_type(class_name: str) -> str:
    return "vector" if "vector" in class_name.casefold() else "scalar"


def qualified_object_identity(
    logical_package: str, package_local_path: str
) -> tuple[str, str]:
    logical = logical_package.strip()
    local_path = package_local_path.strip()
    prefix = logical + "."
    if local_path.casefold().startswith(prefix.casefold()):
        local_path = local_path[len(prefix) :]
    return logical.casefold(), local_path.casefold()


def cross_revision_module_quorum(
    pinned_module: dict[str, Any],
    recovery_module: dict[str, Any],
    recovery_child: dict[str, Any] | None = None,
) -> dict[str, Any]:
    pinned_projection = module_reference_projection(pinned_module)
    recovery_projection = module_reference_projection(recovery_module)
    semantic_equal = pinned_projection == recovery_projection
    return {
        "pinnedRecordSha256": record_sha256(pinned_projection),
        "recoveryRecordSha256": record_sha256(recovery_projection),
        "semanticEqual": semantic_equal,
        "provesChildTargetPayloadEqual": False,
        "sourceExact": False,
        "recoveryChildRecordSha256": (
            record_sha256(recovery_child) if recovery_child is not None else ""
        ),
        "recoveryChildEvidenceStatus": (
            "CURRENT_REVISION_RECOVERY_LEAD_ONLY"
            if recovery_child is not None
            else "ABSENT"
        ),
        "blockers": (
            ["CROSS_REVISION_TARGET_PAYLOAD_NOT_SOURCE_EXACT"]
            if semantic_equal
            else [
                "CROSS_REVISION_SOURCE_MODULE_QUORUM_FAILED",
                "CROSS_REVISION_TARGET_PAYLOAD_NOT_SOURCE_EXACT",
            ]
        ),
    }


def resolve_point_light_fields(
    instance_properties: dict[str, Any],
    nested_archetype_properties: dict[str, Any],
    class_cdo_properties: dict[str, Any],
    parent_cdo_properties: dict[str, Any],
) -> dict[str, Any]:
    return resolve_inheritance_fields(
        [
            {
                "tier": "INSTANCE_EXPLICIT",
                "sourceId": POINT_LIGHT_PATH,
                "evidenceStatus": "SOURCE_EXACT_PHYSICAL_PACKAGE",
                "properties": instance_properties,
            },
            {
                "tier": "NESTED_ARCHETYPE_TEMPLATE",
                "sourceId": (
                    "EFGame.Default__EFParticleModuleTypeDataLight."
                    "PointLightComponent0"
                ),
                "evidenceStatus": "CURRENT_REVISION_ARCHETYPE_EVIDENCE",
                "properties": nested_archetype_properties,
            },
            {
                "tier": "CLASS_CDO",
                "sourceId": "Engine.Default__PointLightComponent",
                "evidenceStatus": "CURRENT_REVISION_SCRIPT_EVIDENCE",
                "properties": class_cdo_properties,
            },
            {
                "tier": "PARENT_CDO_HIERARCHY",
                "sourceId": "Engine.Default__LightComponent",
                "evidenceStatus": "CURRENT_REVISION_SCRIPT_EVIDENCE",
                "properties": parent_cdo_properties,
            },
        ],
        (
            "brightness",
            "bcastcompositeshadow",
            "baffectcompositeshadowdirection",
            "radius",
            "falloffexponent",
            "lightcolor",
        ),
    )


def evaluate_particle_parameter_occurrence(
    definition_semantics: dict[str, Any],
    parameter_overrides: list[dict[str, Any]],
) -> dict[str, Any]:
    """Choose the UE3 parameter branch without reading an unresolved fallback payload."""
    blockers: list[str] = []
    evaluator_kind = definition_semantics.get("evaluatorKind")
    if evaluator_kind == "STANDARD_CONSTANT_CURVE":
        blockers.append("CONSTANT_CURVE_COMPILER_NOT_IMPLEMENTED")
        return {
            "branch": "STANDARD_CONSTANT_CURVE_SOURCE_PAYLOAD",
            **admission_from_blockers(blockers),
        }
    if evaluator_kind == "STANDARD_DISTRIBUTION":
        return {
            "branch": "STANDARD_SOURCE_PAYLOAD",
            **admission_from_blockers(blockers),
        }
    if evaluator_kind != "STANDARD_PARTICLE_PARAMETER":
        blockers.append("CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN")
        return {
            "branch": "UNRESOLVED_CUSTOM_EVALUATOR",
            **admission_from_blockers(blockers),
        }
    fields = definition_semantics["resolvedFields"]
    parameter_name = resolved_property_value(fields, "parametername")
    modes = [
        resolved_property_value(fields, name)
        for name in definition_semantics["requiredModeFields"]
    ]
    if not isinstance(parameter_name, str) or not parameter_name:
        blockers.append("PARAMETER_NAME_PROVENANCE_UNRESOLVED")
    if any(not isinstance(mode, str) for mode in modes):
        blockers.append("PARAMETER_MODE_PROVENANCE_UNRESOLVED")
    direct = bool(modes) and all(
        isinstance(mode, str) and mode.casefold() == "dpm_direct" for mode in modes
    )
    expected_type = definition_semantics["expectedOverrideType"]
    matches = [
        row
        for row in parameter_overrides
        if isinstance(parameter_name, str)
        and str(row.get("name", "")).casefold() == parameter_name.casefold()
    ]
    typed_matches = [row for row in matches if row.get("type") == expected_type]
    if direct and len(typed_matches) == 1:
        return {
            "branch": "DIRECT_INPUT",
            "ignoredRangeFields": ["mininput", "maxinput", "minoutput", "maxoutput"],
            **admission_from_blockers(blockers),
        }
    direct_input_blocker = ""
    if direct and matches and not typed_matches:
        direct_input_blocker = "CUE_PARAMETER_TYPE_MISMATCH"
    elif direct and len(typed_matches) > 1:
        direct_input_blocker = "CUE_PARAMETER_BINDING_AMBIGUOUS"
    elif direct:
        direct_input_blocker = "CUE_PARAMETER_BINDING_MISSING"
    constant = fields.get("constant", {}).get("selected")
    if constant is None:
        if direct_input_blocker:
            blockers.append(direct_input_blocker)
        blockers.append("PARAMETER_FALLBACK_CONSTANT_UNRESOLVED")
    elif str(constant.get("evidenceStatus", "")).startswith("CURRENT_"):
        if direct_input_blocker:
            blockers.append(direct_input_blocker)
        blockers.append(
            "PARAMETER_FALLBACK_SOURCE_ERA_DEFAULT_PROVENANCE_UNRESOLVED"
        )
    if not direct:
        for name in ("mininput", "maxinput", "minoutput", "maxoutput"):
            if fields.get(name, {}).get("selected") is None:
                blockers.append("PARAMETER_RANGE_FOUR_FIELDS_INCOMPLETE")
                break
    return {
        "branch": "CONSTANT_FALLBACK" if not blockers else "UNRESOLVED_FALLBACK",
        **admission_from_blockers(blockers),
    }


def require_raw_identity(
    path: Path, expected_name: str, expected_size: int, expected_sha256: str
) -> None:
    require(path.name.casefold() == expected_name.casefold(), f"package name changed: {path}")
    require(path.stat().st_size == expected_size, f"package size changed: {path}")
    require(raw_file_sha256(path) == expected_sha256, f"package hash changed: {path}")


def current_revision_class_default_evidence(
    current_package_root: Path,
    efgame_script_path: Path,
    engine_script_path: Path,
) -> dict[str, Any]:
    require_raw_identity(efgame_script_path, *CURRENT_EFGAME_SCRIPT)
    require_raw_identity(engine_script_path, *CURRENT_ENGINE_SCRIPT)
    efgame_graph = extract_package(
        efgame_script_path, "EFGame", LOSTARK_KR_AES_KEY
    )
    engine_graph = extract_package(
        engine_script_path, "Engine", LOSTARK_KR_AES_KEY
    )
    custom_vector_cdo = find_object(
        efgame_graph, "Default__EFDistributionVectorMultiplyParticleParameter"
    )
    location_skip_archetype = find_object(
        efgame_graph,
        "Default__EFParticleModuleLocationOnGround.DistributionLocationSkip",
    )
    light_archetype = find_object(
        efgame_graph,
        "Default__EFParticleModuleTypeDataLight.PointLightComponent0",
    )
    float_parameter_cdo = find_object(
        engine_graph, "Default__DistributionFloatParameterBase"
    )
    vector_parameter_cdo = find_object(
        engine_graph, "Default__DistributionVectorParameterBase"
    )
    point_light_cdo = find_object(engine_graph, "Default__PointLightComponent")
    light_cdo = find_object(engine_graph, "Default__LightComponent")
    require(
        custom_vector_cdo["properties"]["MaxInput"]["value"]
        == {"x": 100.0, "y": 100.0, "z": 100.0}
        and custom_vector_cdo["properties"]["MaxOutput"]["value"]
        == {"x": 100.0, "y": 100.0, "z": 100.0}
        and location_skip_archetype["properties"]["Constant"]["value"] == 1.0
        and float_parameter_cdo["properties"]["MaxInput"]["value"] == 1.0
        and float_parameter_cdo["properties"]["MaxOutput"]["value"] == 1.0
        and vector_parameter_cdo["properties"]["MaxInput"]["value"]
        == {"x": 1.0, "y": 1.0, "z": 1.0}
        and vector_parameter_cdo["properties"]["MaxOutput"]["value"]
        == {"x": 1.0, "y": 1.0, "z": 1.0}
        and light_archetype["properties"]["Radius"]["value"] == 200.0
        and point_light_cdo["properties"]["FalloffExponent"]["value"] == 2.0
        and light_cdo["properties"]["LightColor"]["value"]
        == {"r": 255, "g": 255, "b": 255, "a": 0},
        "current script class-default evidence changed",
    )

    recovery_targets = []
    for logical, physical, size, sha256, object_path in (
        CURRENT_DEFAULT_RECOVERY_PACKAGES
    ):
        package_path = current_package_root / physical
        require_raw_identity(package_path, physical, size, sha256)
        graph = extract_package(package_path, logical, LOSTARK_KR_AES_KEY)
        target = find_object(graph, object_path)
        recovery_targets.append(
            {
                "logicalPackage": logical,
                "physicalPackage": physical,
                "bytes": size,
                "sha256": sha256,
                "sourceEraPackageIdentityPinned": False,
                "object": {
                    "className": target["className"],
                    "classPath": target["classPath"],
                    "objectPath": target["objectPath"],
                    "archetypeIndex": target["archetypeIndex"],
                    "archetypePath": target["archetypePath"],
                    "properties": copy.deepcopy(target["properties"]),
                    "recordSha256": record_sha256(target),
                },
            }
        )
    require(
        recovery_targets[0]["object"]["archetypePath"].casefold()
        == (
            "efgame.default__efparticlemodulelocationonground."
            "distributionlocationskip"
        )
        and recovery_targets[1]["object"]["archetypeIndex"] == 0
        and recovery_targets[2]["object"]["archetypeIndex"] == 0,
        "current recovery target archetype evidence changed",
    )

    def cdo_record(value: dict[str, Any]) -> dict[str, Any]:
        return {
            "className": value["className"],
            "classPath": value["classPath"],
            "objectPath": value["objectPath"],
            "archetypeIndex": value["archetypeIndex"],
            "archetypePath": value["archetypePath"],
            "properties": copy.deepcopy(value["properties"]),
            "recordSha256": record_sha256(value),
        }

    return {
        "status": "CURRENT_REVISION_CLASS_DEFAULT_EVIDENCE_ONLY",
        "sourceEraScriptIdentityPinned": False,
        "admissionAllowed": False,
        "blockers": [
            "SOURCE_ERA_SCRIPT_PACKAGE_IDENTITY_NOT_PINNED",
            "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN",
        ],
        "scriptPackages": [
            {
                "logicalPackage": "EFGame",
                "physicalPackage": CURRENT_EFGAME_SCRIPT[0],
                "bytes": CURRENT_EFGAME_SCRIPT[1],
                "sha256": CURRENT_EFGAME_SCRIPT[2],
            },
            {
                "logicalPackage": "Engine",
                "physicalPackage": CURRENT_ENGINE_SCRIPT[0],
                "bytes": CURRENT_ENGINE_SCRIPT[1],
                "sha256": CURRENT_ENGINE_SCRIPT[2],
            },
        ],
        "classDefaults": {
            "efVectorMultiply": cdo_record(custom_vector_cdo),
            "locationSkipArchetype": cdo_record(location_skip_archetype),
            "floatParticleParameter": {
                "status": "CURRENT_NATIVE_CLASS_CDO_NOT_SERIALIZED",
                "classPath": "Engine.DistributionFloatParticleParameter",
                "properties": {},
            },
            "vectorParticleParameter": {
                "status": "CURRENT_NATIVE_CLASS_CDO_NOT_SERIALIZED",
                "classPath": "Engine.DistributionVectorParticleParameter",
                "properties": {},
            },
            "floatParameterBase": cdo_record(float_parameter_cdo),
            "vectorParameterBase": cdo_record(vector_parameter_cdo),
            "effectPointLightArchetype": cdo_record(light_archetype),
            "pointLightComponent": cdo_record(point_light_cdo),
            "lightComponent": cdo_record(light_cdo),
        },
        "evaluatorDefaults": {
            "floatParticleParameter": {
                "status": "CURRENT_NATIVE_EVALUATOR_SHAPE_ONLY",
                "properties": {
                    "ParamMode": tagged_value("dpm_normal", "ByteProperty"),
                    "MinInput": tagged_value(0.0, "FloatProperty"),
                    "MinOutput": tagged_value(0.0, "FloatProperty"),
                    "Constant": tagged_value(0.0, "FloatProperty"),
                },
            },
            "vectorParticleParameter": {
                "status": "CURRENT_NATIVE_EVALUATOR_SHAPE_ONLY",
                "properties": {
                    "ParamModes": tagged_value("dpm_normal", "ByteProperty"),
                    "ParamModes[1]": tagged_value("dpm_normal", "ByteProperty"),
                    "ParamModes[2]": tagged_value("dpm_normal", "ByteProperty"),
                    "MinInput": tagged_value(
                        {"x": 0.0, "y": 0.0, "z": 0.0}, "StructProperty"
                    ),
                    "MinOutput": tagged_value(
                        {"x": 0.0, "y": 0.0, "z": 0.0}, "StructProperty"
                    ),
                    "Constant": tagged_value(
                        {"x": 0.0, "y": 0.0, "z": 0.0}, "StructProperty"
                    ),
                },
            },
        },
        "recoveryTargets": recovery_targets,
    }


def particle_parameter_semantics(
    target: dict[str, Any] | None,
    current_target: dict[str, Any],
    current_defaults: dict[str, Any],
    payload_fidelity: str,
) -> dict[str, Any]:
    class_name = str((target or current_target).get("className", "")).casefold()
    if "particleparameter" not in class_name:
        curve = "constantcurve" in class_name
        return {
            "isParticleParameter": False,
            "evaluatorKind": (
                "STANDARD_CONSTANT_CURVE" if curve else "STANDARD_DISTRIBUTION"
            ),
            "resolvedFields": {},
            "requiredModeFields": [],
            "expectedOverrideType": "",
            "status": "SEMANTIC_SOURCE_READY",
            "blockers": [],
            "compilerBlockers": (
                ["CONSTANT_CURVE_COMPILER_NOT_IMPLEMENTED"] if curve else []
            ),
        }

    vector = "vector" in class_name
    custom = custom_particle_parameter_class(class_name)
    class_defaults = current_defaults["classDefaults"]
    evaluator_defaults = current_defaults["evaluatorDefaults"]
    sources = [
        {
            "tier": "INSTANCE_EXPLICIT",
            "sourceId": str((target or {}).get("objectPath", "")),
            "evidenceStatus": payload_fidelity,
            "properties": copy.deepcopy((target or {}).get("properties") or {}),
        }
    ]
    archetype_path = str(current_target.get("archetypePath") or "")
    if archetype_path.casefold() == (
        "efgame.default__efparticlemodulelocationonground."
        "distributionlocationskip"
    ):
        archetype = class_defaults["locationSkipArchetype"]
        sources.append(
            {
                "tier": "NESTED_ARCHETYPE_TEMPLATE",
                "sourceId": archetype["objectPath"],
                "evidenceStatus": "CURRENT_REVISION_ARCHETYPE_EVIDENCE",
                "properties": copy.deepcopy(archetype["properties"]),
            }
        )

    if custom:
        custom_cdo = class_defaults["efVectorMultiply"]
        sources.append(
            {
                "tier": "CLASS_CDO",
                "sourceId": custom_cdo["objectPath"],
                "evidenceStatus": "CURRENT_REVISION_SCRIPT_EVIDENCE",
                "properties": copy.deepcopy(custom_cdo["properties"]),
            }
        )
    else:
        class_key = "vectorParticleParameter" if vector else "floatParticleParameter"
        class_cdo = class_defaults[class_key]
        sources.append(
            {
                "tier": "CLASS_CDO",
                "sourceId": class_cdo["classPath"],
                "evidenceStatus": class_cdo["status"],
                "properties": copy.deepcopy(class_cdo["properties"]),
            }
        )

    parent_key = "vectorParameterBase" if vector else "floatParameterBase"
    parent_cdo = class_defaults[parent_key]
    sources.append(
        {
            "tier": "PARENT_CDO_HIERARCHY",
            "sourceId": parent_cdo["objectPath"],
            "evidenceStatus": "CURRENT_REVISION_SCRIPT_EVIDENCE",
            "properties": copy.deepcopy(parent_cdo["properties"]),
        }
    )
    evaluator_key = "vectorParticleParameter" if vector else "floatParticleParameter"
    evaluator = evaluator_defaults[evaluator_key]
    sources.append(
        {
            "tier": "EVALUATOR_DEFAULT",
            "sourceId": evaluator_key,
            "evidenceStatus": evaluator["status"],
            "properties": copy.deepcopy(evaluator["properties"]),
        }
    )

    mode_fields = (
        ["parammodes", "parammodes[1]", "parammodes[2]"]
        if vector
        else ["parammode"]
    )
    range_fields = ["mininput", "maxinput", "minoutput", "maxoutput"]
    resolved_fields = resolve_inheritance_fields(
        sources, ["parametername", *mode_fields, *range_fields, "constant"]
    )
    modes = [resolved_property_value(resolved_fields, name) for name in mode_fields]
    direct = bool(modes) and all(
        isinstance(mode, str) and mode.casefold() == "dpm_direct"
        for mode in modes
    )
    blockers: list[str] = []
    if resolved_fields["parametername"]["selected"] is None:
        blockers.append("PARAMETER_NAME_PROVENANCE_UNRESOLVED")
    if any(resolved_fields[name]["selected"] is None for name in mode_fields):
        blockers.append("PARAMETER_MODE_PROVENANCE_UNRESOLVED")
    if not direct and any(
        resolved_fields[name]["selected"] is None for name in range_fields
    ):
        blockers.append("PARAMETER_RANGE_FOUR_FIELDS_INCOMPLETE")
    if resolved_fields["constant"]["selected"] is None:
        blockers.append("PARAMETER_FALLBACK_CONSTANT_UNRESOLVED")

    current_only_required: list[str] = []
    required_for_totality = ["parametername", *mode_fields, "constant"]
    if not direct:
        required_for_totality.extend(range_fields)
    for name in required_for_totality:
        selected = resolved_fields[name]["selected"]
        if selected is not None and str(selected["evidenceStatus"]).startswith(
            "CURRENT_"
        ):
            current_only_required.append(name)
    if current_only_required:
        blockers.append("SOURCE_ERA_ENGINE_CDO_PROVENANCE_UNRESOLVED")
        if any(name in mode_fields for name in current_only_required):
            blockers.append(
                "PARAMETER_MODE_SOURCE_ERA_DEFAULT_PROVENANCE_UNRESOLVED"
            )
        if any(name in range_fields for name in current_only_required):
            blockers.append(
                "PARAMETER_RANGE_SOURCE_ERA_DEFAULT_PROVENANCE_UNRESOLVED"
            )
        if "constant" in current_only_required:
            blockers.append(
                "PARAMETER_FALLBACK_SOURCE_ERA_DEFAULT_PROVENANCE_UNRESOLVED"
            )

    selected_constant = resolved_fields["constant"]["selected"]
    if (
        selected_constant is not None
        and selected_constant["tier"] == "NESTED_ARCHETYPE_TEMPLATE"
    ):
        blockers.extend(
            ["CURRENT_REVISION_ARCHETYPE_EVIDENCE", "CLASS_DEFAULT_ARCHETYPE_UNPROVEN"]
        )
    if custom:
        blockers.extend(
            [
                "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN",
                "RUNTIME_PARAMETER_SOURCE_CLOSURE_UNPROVEN",
                "SOURCE_ERA_SCRIPT_PACKAGE_IDENTITY_NOT_PINNED",
            ]
        )

    decoded_properties = copy.deepcopy((target or {}).get("properties") or {})
    return {
        "isParticleParameter": True,
        "isCustomEvaluator": custom,
        "evaluatorKind": (
            "CUSTOM_EF_DISTRIBUTION_UNPROVEN"
            if custom
            else "STANDARD_PARTICLE_PARAMETER"
        ),
        "decodedFields": sorted(decoded_properties),
        "decodedValues": decoded_properties,
        "resolvedFields": resolved_fields,
        "resolutionOrder": [
            "INSTANCE_EXPLICIT",
            "NESTED_ARCHETYPE_TEMPLATE",
            "CLASS_CDO",
            "PARENT_CDO_HIERARCHY",
            "EVALUATOR_DEFAULT",
        ],
        "requiredModeFields": mode_fields,
        "expectedOverrideType": expected_override_type(class_name),
        "parameterEvaluationMode": (
            "CUSTOM_EVALUATOR_UNPROVEN"
            if custom
            else ("DIRECT_PER_COMPONENT" if direct else "NORMAL_OR_ABS_RANGE_MAPPING")
        ),
        "executionIrrelevantFields": range_fields if direct else [],
        "currentOnlyRequiredFields": sorted(current_only_required),
        "status": "SEMANTIC_SOURCE_READY" if not blockers else "SEMANTIC_BLOCKED",
        "blockers": sorted(set(blockers)),
        "compilerBlockers": [],
    }

def build_indexes(
    graph: dict[str, Any], external: dict[str, Any]
) -> tuple[dict[str, dict[str, Any]], dict[str, dict[str, Any]], dict[str, dict[str, Any]], dict[str, dict[str, Any]]]:
    graph_by_id = {str(row["nodeId"]): row for row in graph.get("nodes", [])}
    graph_by_identity: dict[tuple[str, str], dict[str, Any]] = {}
    for row in graph.get("nodes", []):
        logical_package = str(row.get("nodeId", "")).split(":", 1)[0]
        identity = qualified_object_identity(
            logical_package, str(row.get("objectPath", ""))
        )
        require(identity not in graph_by_identity, f"duplicate graph identity: {identity}")
        graph_by_identity[identity] = row
    external_by_id: dict[str, dict[str, Any]] = {}
    external_by_identity: dict[tuple[str, str], dict[str, Any]] = {}
    for package in external.get("packages", []):
        logical_package = str(package.get("logicalPackage", ""))
        for row in package.get("objects", []):
            external_by_id[str(row["objectId"])] = row
            identity = qualified_object_identity(
                logical_package, str(row.get("objectPath", ""))
            )
            require(
                identity not in external_by_identity,
                f"duplicate external graph identity: {identity}",
            )
            external_by_identity[identity] = row
    return graph_by_id, graph_by_identity, external_by_id, external_by_identity


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


def resolve_typed_component_reference(
    source: dict[str, Any],
    candidates: list[dict[str, Any]],
    property_path: str,
    expected_package_index: int,
    expected_object_path: str,
) -> dict[str, Any]:
    properties = property_casefold_index(source.get("properties") or {})
    property_row = properties.get(property_path.casefold())
    require(property_row is not None, f"typed component property is missing: {property_path}")
    _source_name, tagged = property_row
    require(
        isinstance(tagged, dict)
        and str(tagged.get("type") or "").casefold() == "objectproperty"
        and isinstance(tagged.get("value"), int)
        and int(tagged["value"]) != 0,
        f"typed component property is not a non-zero object reference: {property_path}",
    )
    identities = {
        (
            int(candidate.get("packageIndex") or 0),
            str(candidate.get("objectPath") or "").casefold(),
        )
        for candidate in candidates
    }
    require(
        len(identities) == 1,
        f"typed component reference evidence is missing or ambiguous: {property_path}",
    )
    package_index, object_path = next(iter(identities))
    require(
        package_index == int(tagged["value"])
        and package_index == expected_package_index
        and object_path == expected_object_path.casefold(),
        f"typed component reference identity changed: {property_path}",
    )
    return {
        "propertyPath": property_path,
        "sourcePackageIndex": package_index,
        "targetObjectPath": str(candidates[0].get("objectPath") or ""),
    }


def active_distribution_reference_occurrences(
    inventory: dict[str, Any],
    graph: dict[str, Any],
    external: dict[str, Any],
) -> list[dict[str, Any]]:
    graph_by_id, _graph_by_identity, external_by_id, _external_by_identity = (
        build_indexes(graph, external)
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
                        int(candidate.get("packageIndex", 0)),
                        str(candidate.get("objectPath", "")).casefold(),
                        str(candidate.get("targetNodeId", "")),
                    )
                    for candidate in candidates
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
    grouped: dict[
        tuple[str, str, int, str], list[dict[str, Any]]
    ] = collections.defaultdict(list)
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
                    name: occurrence[name]
                    for name in (
                        "activeElementId",
                        "sourceCueId",
                        "rendererType",
                        "moduleOrder",
                    )
                }
                for occurrence in sorted(
                    rows_for_key,
                    key=lambda value: (
                        str(value["activeElementId"]),
                        int(value["moduleOrder"]),
                    ),
                )
            ],
        }
        for key, rows_for_key in sorted(grouped.items())
    ]


def cue_parameter_index(action_recipe: dict[str, Any]) -> dict[str, list[dict[str, Any]]]:
    result: dict[str, list[dict[str, Any]]] = {}
    for cue in action_recipe.get("cues", []):
        cue_id = str(cue.get("cueId", ""))
        require(cue_id and cue_id not in result, f"duplicate action cue: {cue_id}")
        typed = cue.get("typedPayload") or {}
        result[cue_id] = (
            copy.deepcopy(typed.get("parameterOverrides") or [])
            if typed.get("parameterOverridesDecoded") is True
            else []
        )
    return result


def build_closure(
    source_receipt_path: Path,
    action_recipe_path: Path,
    active_inventory_path: Path,
    normalized_graph_path: Path,
    external_closure_path: Path,
    current_package_root: Path,
    current_efgame_script_path: Path,
    current_engine_script_path: Path,
    point_light_package_path: Path,
    mesh_rotation_recovery_package_path: Path,
    color_scale_package_path: Path,
) -> dict[str, Any]:
    source_receipt = load_json(source_receipt_path)
    action_recipe = load_json(action_recipe_path)
    inventory = load_json(active_inventory_path)
    graph = load_json(normalized_graph_path)
    external = load_json(external_closure_path)
    require(source_receipt.get("skillId") == 31470, "source receipt skill mismatch")
    require(action_recipe.get("skillId") == 31470, "action recipe skill mismatch")
    require(inventory.get("skillId") == 31470, "active inventory skill mismatch")
    cue_parameters = cue_parameter_index(action_recipe)
    current_defaults = current_revision_class_default_evidence(
        current_package_root,
        current_efgame_script_path,
        current_engine_script_path,
    )
    point_source = source_package(source_receipt, POINT_LIGHT_PACKAGE)
    rotation_source = source_package(source_receipt, MESH_ROTATION_PACKAGE)

    point_hash = raw_file_sha256(point_light_package_path)
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
        and str(point_node["archetypePath"]).casefold()
        == "efgame.default__efparticlemoduletypedatalight.pointlightcomponent0"
        and int(point_node["serialSize"]) == int(point_node["propertyStreamEnd"])
        and float(point_properties["brightness"]["value"]) == 10.0
        and point_properties["bcastcompositeshadow"]["value"] is False
        and point_properties["baffectcompositeshadowdirection"]["value"] is False,
        "PointLight exact payload changed",
    )

    rotation_hash = raw_file_sha256(mesh_rotation_recovery_package_path)
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
    pinned_rotation_module = copy.deepcopy(
        find_normalized_node(graph, MESH_ROTATION_SOURCE_NODE)
    )
    pinned_rotation_module["references"] = canonical_source_node_references(
        graph, MESH_ROTATION_SOURCE_NODE
    )
    require(
        len(pinned_rotation_module["references"]) == 1,
        "mesh-rotation pinned module reference evidence is incomplete",
    )
    recovery_rotation_module = find_object(
        rotation_graph,
        "par_v_smd_onestroke_swing_01.particlemodulemeshrotation_12",
    )
    rotation_quorum = cross_revision_module_quorum(
        pinned_rotation_module, recovery_rotation_module, rotation_node
    )
    require(
        str(rotation_node["className"]).casefold()
        == "efdistributionvectormultiplyparticleparameter"
        and int(rotation_node["serialSize"])
        == int(rotation_node["propertyStreamEnd"]),
        "mesh-rotation recovery payload is incomplete",
    )
    require(
        rotation_quorum["semanticEqual"],
        "mesh-rotation source module changed across package revisions",
    )
    recovery_rotation_references = recovery_rotation_module.get("references", [])
    require(
        len(recovery_rotation_references) == 1
        and int(recovery_rotation_references[0].get("packageIndex", 0)) == 12
        and str(recovery_rotation_references[0].get("objectPath", "")).casefold()
        == MESH_ROTATION_PATH.casefold(),
        "mesh-rotation distribution target reference changed across revisions",
    )

    color_hash = raw_file_sha256(color_scale_package_path)
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
    current_color_by_identity = {
        qualified_object_identity(COLOR_SCALE_PACKAGE, str(row["objectPath"])): row
        for row in color_graph["objects"]
    }
    quorum = []
    for old in external_color_package["objects"]:
        identity = qualified_object_identity(
            COLOR_SCALE_PACKAGE, str(old["objectPath"])
        )
        current = current_color_by_identity.get(identity)
        require(current is not None, f"ColorScale quorum object is missing: {identity}")
        equal = semantic_projection(old) == semantic_projection(current)
        require(equal, f"ColorScale quorum object changed: {identity}")
        quorum.append(
            {
                "objectId": old["objectId"],
                "logicalPackage": COLOR_SCALE_PACKAGE,
                "packageLocalPath": old["objectPath"],
                "priorClosureRecordSha256": record_sha256(
                    semantic_projection(old)
                ),
                "currentPackageRecordSha256": record_sha256(
                    semantic_projection(current)
                ),
                "semanticEqual": True,
                "provesChildTargetPayloadEqual": False,
            }
        )
    require(
        len(quorum) == 5
        and int(color_node["serialSize"]) == int(color_node["propertyStreamEnd"]),
        "ColorScale external package quorum is incomplete",
    )

    (
        graph_by_id,
        graph_by_identity,
        external_by_id,
        external_by_identity,
    ) = build_indexes(graph, external)
    occurrences = active_distribution_reference_occurrences(inventory, graph, external)
    grouped = group_occurrences(occurrences)
    current_packages: dict[str, dict[str, Any]] = {}
    for logical_package in sorted(
        {str(row["sourceNodeId"]).split(":", 1)[0] for row in grouped}
    ):
        receipt_package = optional_source_package(source_receipt, logical_package)
        closure_package = external_package(external, logical_package)
        package_record = receipt_package or closure_package
        require(
            package_record is not None,
            f"current package identity is missing: {logical_package}",
        )
        physical = str(package_record["physicalPackage"])
        package_path = current_package_root / physical
        require(package_path.is_file(), f"current package is missing: {package_path}")
        current_hash = raw_file_sha256(package_path)
        current_bytes = package_path.stat().st_size
        exact_physical_source_package_present = bool(
            receipt_package is not None
            and current_hash == str(receipt_package["sourcePackageSha256"])
            and current_bytes == int(receipt_package["sourcePackageBytes"])
        )
        current_packages[logical_package] = {
            "logicalPackage": logical_package,
            "physicalPackage": physical,
            "bytes": current_bytes,
            "sha256": current_hash,
            "receiptPackageIdentityPinned": receipt_package is not None,
            "exactPhysicalSourcePackagePresent": (
                exact_physical_source_package_present
            ),
            "graph": extract_package(
                package_path, logical_package, LOSTARK_KR_AES_KEY
            ),
        }
    distribution_definitions: list[dict[str, Any]] = []
    distribution_occurrences: list[dict[str, Any]] = []
    for index, row in enumerate(grouped):
        reference_id = f"distribution-target-{index:03d}"
        definition_id = f"distribution-definition-{index:03d}"
        logical_package = str(row["sourceNodeId"]).split(":", 1)[0]
        target_identity = qualified_object_identity(
            logical_package, str(row["targetObjectPath"])
        )
        pinned_target = (
            graph_by_id.get(row["targetNodeId"])
            or graph_by_identity.get(target_identity)
            or external_by_identity.get(target_identity)
        )
        source_target = pinned_target
        is_mesh_rotation_target = row["sourceNodeId"] == MESH_ROTATION_SOURCE_NODE
        receipt_package = optional_source_package(source_receipt, logical_package)
        current_package = current_packages[logical_package]
        payload_status = "DECODED_SOURCE_RECORD"
        fidelity = "PINNED_SOURCE_RECORD_PHYSICAL_ABSENT"
        source_revision: dict[str, Any] = {
            "basis": "PINNED_NORMALIZED_GRAPH_SOURCE_RECORD",
        }
        if is_mesh_rotation_target:
            source_target = None
            payload_status = "UNRESOLVED_CROSS_REVISION_TARGET_PAYLOAD"
            fidelity = "UNRESOLVED_CROSS_REVISION"
            source_revision = {
                "basis": "PINNED_SOURCE_MODULE_REFERENCE_QUORUM_ONLY",
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
                "sourceModuleQuorum": {
                    **rotation_quorum,
                    "sourceNodeId": MESH_ROTATION_SOURCE_NODE,
                    "sourceObjectPath": recovery_rotation_module["objectPath"],
                    "targetPackageIndex": 12,
                    "targetObjectPath": MESH_ROTATION_PATH,
                },
            }
        elif row["sourceNodeId"] == COLOR_SCALE_SOURCE_NODE:
            source_target = color_node
            payload_status = "DECODED_EXTERNAL_RECORD_WITHOUT_PACKAGE_IDENTITY"
            fidelity = "RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED"
            source_revision = {
                "basis": "CURRENT_RECORD_AND_PRIOR_CLOSURE_SEMANTIC_QUORUM",
                "physicalPackage": color_scale_package_path.name,
                "bytes": color_bytes,
                "sha256": color_hash,
                "priorRequestedObjectQuorum": quorum,
                "provesChildTargetPayloadEqual": False,
            }
        elif receipt_package is None:
            payload_status = "DECODED_EXTERNAL_RECORD_WITHOUT_PACKAGE_IDENTITY"
            fidelity = "RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED"
            source_revision = {
                "basis": "EXTERNAL_RECORD_WITHOUT_RECEIPT_PACKAGE_IDENTITY",
                "sourceExact": False,
            }
        elif current_package["exactPhysicalSourcePackagePresent"]:
            fidelity = "SOURCE_EXACT_PHYSICAL_PACKAGE"
        require(
            source_target is not None or is_mesh_rotation_target,
            f"distribution target is missing: {row}",
        )
        current_target = find_object(
            current_package["graph"], str(row["targetObjectPath"])
        )
        current_target_evidence = {
            name: current_package[name]
            for name in (
                "logicalPackage",
                "physicalPackage",
                "bytes",
                "sha256",
                "receiptPackageIdentityPinned",
                "exactPhysicalSourcePackagePresent",
            )
        }
        current_target_evidence["object"] = {
            "className": current_target["className"],
            "classPath": current_target["classPath"],
            "objectPath": current_target["objectPath"],
            "archetypeIndex": current_target["archetypeIndex"],
            "archetypePath": current_target["archetypePath"],
            "properties": copy.deepcopy(current_target["properties"]),
            "recordSha256": record_sha256(current_target),
        }
        current_target_evidence["semanticEqualToPinnedTarget"] = bool(
            source_target is not None
            and semantic_projection(source_target)
            == semantic_projection(current_target)
        )
        semantics = particle_parameter_semantics(
            source_target, current_target, current_defaults, fidelity
        )
        semantics["currentRevisionTargetEvidence"] = current_target_evidence
        semantics["receiptPackageIdentityPinned"] = receipt_package is not None
        semantics["pinnedPayloadDecoded"] = bool(
            receipt_package is not None and source_target is not None
        )
        semantics["exactPhysicalSourcePackagePresent"] = current_package[
            "exactPhysicalSourcePackagePresent"
        ]
        blockers: list[str] = list(semantics.get("blockers", []))
        blockers.extend(semantics.get("compilerBlockers", []))
        if payload_status.startswith("UNRESOLVED"):
            blockers.append("CROSS_REVISION_TARGET_PAYLOAD_NOT_SOURCE_EXACT")
        if receipt_package is None:
            blockers.append("SOURCE_ERA_TARGET_PACKAGE_IDENTITY_NOT_PINNED")
        elif not current_package["exactPhysicalSourcePackagePresent"]:
            blockers.append("SOURCE_ERA_TARGET_PHYSICAL_PACKAGE_NOT_AVAILABLE")
        if is_mesh_rotation_target:
            blockers.extend(
                [
                    "OLD_SOURCE_ABSENT_BINDING_FALLBACK_UNRESOLVED",
                    "SELECTED_LOD_CLASS_DEFAULTS_UNRESOLVED",
                ]
            )
        blockers.append("COMPILED_DISTRIBUTION_EXECUTION_NOT_IMPLEMENTED")
        definition_admission = admission_from_blockers(blockers)
        source_payload = (
            {
                "variant": "DECODED_EVIDENCE",
                "className": source_target["className"],
                "classPath": source_target.get("classPath", ""),
                "packageLocalPath": source_target["objectPath"],
                "archetypeIndex": source_target.get("archetypeIndex", 0),
                "archetypePath": source_target.get("archetypePath", ""),
                "properties": copy.deepcopy(source_target.get("properties") or {}),
                "recordSha256": record_sha256(source_target),
                "serialSize": source_target.get("serialSize"),
                "propertyStreamEnd": source_target.get("propertyStreamEnd"),
            }
            if source_target is not None
            else {
                "variant": "UNRESOLVED",
                "className": current_target["className"],
                "classPath": current_target.get("classPath", ""),
                "packageLocalPath": row["targetObjectPath"],
                "properties": None,
                "blockers": sorted(set(blockers)),
            }
        )
        occurrence_ids: list[str] = []
        for occurrence_index, occurrence in enumerate(row["activeOccurrences"]):
            occurrence_id = (
                f"distribution-occurrence-{index:03d}-{occurrence_index:02d}"
            )
            occurrence_ids.append(occurrence_id)
            cue_id = str(occurrence["sourceCueId"])
            require(cue_id in cue_parameters, f"action cue is missing: {cue_id}")
            evaluation = evaluate_particle_parameter_occurrence(
                semantics, cue_parameters[cue_id]
            )
            occurrence_blockers = [
                *definition_admission["blockers"],
                *evaluation["blockers"],
            ]
            distribution_occurrences.append(
                {
                    "occurrenceId": occurrence_id,
                    "referenceKind": "DISTRIBUTION_TARGET",
                    "referenceId": reference_id,
                    "definitionId": definition_id,
                    "activeElementId": occurrence["activeElementId"],
                    "sourceCueId": cue_id,
                    "rendererType": occurrence["rendererType"],
                    "referenceOrderIndex": occurrence["moduleOrder"],
                    "propertyIdentity": {
                        "logicalPackage": logical_package,
                        "packageLocalPath": row["sourceModulePath"],
                        "propertyPath": row["propertyPath"],
                    },
                    "parameterEvaluation": evaluation,
                    "executionAdmission": admission_from_blockers(
                        occurrence_blockers
                    ),
                }
            )
        distribution_definitions.append(
            {
                "definitionId": definition_id,
                "referenceKind": "DISTRIBUTION_TARGET",
                "referenceId": reference_id,
                "logicalPackage": logical_package,
                "sourceDocument": row["sourceDocument"],
                "sourceNodeId": row["sourceNodeId"],
                "sourceModuleClass": row["sourceModuleClass"],
                "sourceModulePath": row["sourceModulePath"],
                "propertyPath": row["propertyPath"],
                "sourcePackageIndex": row["sourcePackageIndex"],
                "targetNodeId": row["targetNodeId"],
                "targetPackageLocalPath": row["targetObjectPath"],
                "sourceClass": str(current_target["className"]),
                "occurrenceIds": occurrence_ids,
                "payloadStatus": payload_status,
                "fidelity": fidelity,
                "receiptPackageIdentityPinned": receipt_package is not None,
                "pinnedPayloadDecoded": bool(
                    receipt_package is not None and source_target is not None
                ),
                "exactPhysicalSourcePackagePresent": current_package[
                    "exactPhysicalSourcePackagePresent"
                ],
                "sourceRevisionEvidence": source_revision,
                "sourcePayload": source_payload,
                "currentRevisionTargetEvidence": current_target_evidence,
                "semanticCoverage": semantics,
                "executionAdmission": definition_admission,
            }
        )

    light_source_module = graph_by_id.get(POINT_LIGHT_SOURCE_NODE) or external_by_id.get(
        POINT_LIGHT_SOURCE_NODE
    )
    require(light_source_module is not None, "PointLight source module is missing")
    light_occurrences = []
    light_reference_identity: dict[str, Any] | None = None
    for element in inventory["activeElements"]:
        for module_order, module in enumerate(element["moduleEvidence"]):
            source_id = str(module.get("nodeId") or module.get("objectId") or "")
            if source_id != POINT_LIGHT_SOURCE_NODE:
                continue
            source_document = str(module["sourceDocument"])
            candidates = reference_candidates(
                graph,
                external_by_id,
                str(element["sourceSystemId"]),
                source_document,
                source_id,
                POINT_LIGHT_PROPERTY,
            )
            resolved_reference = resolve_typed_component_reference(
                light_source_module,
                candidates,
                POINT_LIGHT_PROPERTY,
                7055,
                POINT_LIGHT_PATH,
            )
            if light_reference_identity is None:
                light_reference_identity = resolved_reference
            else:
                require(
                    light_reference_identity == resolved_reference,
                    "active PointLight references disagree",
                )
            light_occurrences.append(
                {
                    "activeElementId": element["activeElementId"],
                    "sourceCueId": element["cueId"],
                    "rendererType": element["rendererType"],
                    "referenceOrderIndex": module_order,
                }
            )
    require(
        len(light_occurrences) == 1 and light_reference_identity is not None,
        "active PointLight reference count changed",
    )
    light_fields = resolve_point_light_fields(
        point_properties,
        current_defaults["classDefaults"]["effectPointLightArchetype"]["properties"],
        current_defaults["classDefaults"]["pointLightComponent"]["properties"],
        current_defaults["classDefaults"]["lightComponent"]["properties"],
    )
    require(
        resolved_property_value(light_fields, "brightness") == 10.0
        and resolved_property_value(light_fields, "bcastcompositeshadow") is False
        and resolved_property_value(
            light_fields, "baffectcompositeshadowdirection"
        )
        is False
        and resolved_property_value(light_fields, "radius") == 200.0
        and resolved_property_value(light_fields, "falloffexponent") == 2.0
        and resolved_property_value(light_fields, "lightcolor")
        == {"r": 255, "g": 255, "b": 255, "a": 0},
        "PointLight inherited field resolution changed",
    )
    light_blockers = [
        "POINT_LIGHT_CLASS_DEFAULTS_UNRESOLVED",
        "SOURCE_ERA_SCRIPT_PACKAGE_IDENTITY_NOT_PINNED",
        "LIGHT_RENDERER_NOT_COMPILED",
    ]
    component_definitions = [
        {
            "definitionId": "component-definition-000",
            "referenceId": "typedata-point-light-component-000",
            "referenceKind": "TYPEDATA_COMPONENT",
            "logicalPackage": POINT_LIGHT_PACKAGE,
            "sourceNodeId": POINT_LIGHT_SOURCE_NODE,
            "sourceModuleClass": light_source_module["className"],
            "sourceModulePath": light_source_module["objectPath"],
            "propertyPath": light_reference_identity["propertyPath"],
            "sourcePackageIndex": light_reference_identity["sourcePackageIndex"],
            "targetPackageLocalPath": light_reference_identity["targetObjectPath"],
            "occurrenceIds": ["component-occurrence-000-00"],
            "payloadStatus": "DECODED_SOURCE_RECORD",
            "fidelity": "SOURCE_EXACT_PHYSICAL_PACKAGE",
            "sourceRevisionEvidence": {
                "basis": "IMMUTABLE_SOURCE_RECEIPT_PACKAGE_MATCH",
                "physicalPackage": point_light_package_path.name,
                "bytes": point_bytes,
                "sha256": point_hash,
            },
            "exactPayload": {
                "variant": "DECODED_EVIDENCE",
                "className": point_node["className"],
                "classPath": point_node["classPath"],
                "packageLocalPath": point_node["objectPath"],
                "archetypeIndex": point_node["archetypeIndex"],
                "archetypePath": point_node["archetypePath"],
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
            "semanticCoverage": {
                "decodedFields": sorted(point_properties),
                "resolvedFields": light_fields,
                "currentRevisionClassDefaultEvidenceRefs": [
                    "currentRevisionClassDefaultEvidence.classDefaults."
                    "effectPointLightArchetype",
                    "currentRevisionClassDefaultEvidence.classDefaults."
                    "pointLightComponent",
                    "currentRevisionClassDefaultEvidence.classDefaults."
                    "lightComponent",
                ],
                "status": "SEMANTIC_BLOCKED_SOURCE_ERA_DEFAULT_CHAIN",
            },
            "executionAdmission": admission_from_blockers(light_blockers),
        }
    ]
    component_occurrences = [
        {
            "occurrenceId": "component-occurrence-000-00",
            "referenceKind": "TYPEDATA_COMPONENT",
            "referenceId": "typedata-point-light-component-000",
            "definitionId": "component-definition-000",
            **light_occurrences[0],
            "propertyIdentity": {
                "logicalPackage": POINT_LIGHT_PACKAGE,
                "packageLocalPath": light_source_module["objectPath"],
                "propertyPath": POINT_LIGHT_PROPERTY,
            },
            "executionAdmission": admission_from_blockers(light_blockers),
        }
    ]

    unresolved_distribution_definitions = [
        row
        for row in distribution_definitions
        if row["payloadStatus"].startswith("UNRESOLVED")
    ]
    blocked_semantic_distribution_definitions = [
        row
        for row in distribution_definitions
        if row["semanticCoverage"]["status"] == "SEMANTIC_BLOCKED"
    ]
    current_nonzero_archetype_rows = [
        row
        for row in distribution_definitions
        if int(
            row["currentRevisionTargetEvidence"]["object"]["archetypeIndex"]
            or 0
        )
        != 0
    ]
    decoded_semantic_fields = collections.Counter()
    for row in distribution_definitions:
        for field in row["semanticCoverage"].get("decodedFields", []):
            if field == "mininput":
                decoded_semantic_fields["MinInput"] += 1
            elif field == "maxinput":
                decoded_semantic_fields["MaxInput"] += 1
            elif field == "minoutput":
                decoded_semantic_fields["MinOutput"] += 1
            elif field == "maxoutput":
                decoded_semantic_fields["MaxOutput"] += 1
            elif field.startswith("parammode"):
                decoded_semantic_fields["ParamMode"] += 1

    closure: dict[str, Any] = {
        "schema": "lostark.effect-local-reference-closure",
        "formatVersion": 5,
        "characterClass": "Artist",
        "skillId": 31470,
        "scope": "ACTIVE_FIRST_LOD_OBJECT_REFERENCES",
        "inputs": {
            "sourceReceiptSha256": tracked_text_sha256(source_receipt_path),
            "actionCueRecipeSha256": tracked_text_sha256(action_recipe_path),
            "activeInventorySha256": tracked_text_sha256(active_inventory_path),
            "normalizedGraphSha256": tracked_text_sha256(normalized_graph_path),
            "externalModuleClosureSha256": tracked_text_sha256(external_closure_path),
        },
        "parserCapability": {
            "pointLightComponentClassIncluded": True,
            "efDistributionFloatClassIncluded": True,
            "efDistributionVectorClassIncluded": True,
            "normalizedGraphStatus": "OLDER_THAN_CURRENT_PARSER_CAPABILITY",
            "enumeration": "ALL_NONZERO_OBJECT_REFERENCES_NESTED_IN_ACTIVE_SELECTED_RAW_DISTRIBUTION_WRAPPERS",
        },
        "currentRevisionClassDefaultEvidence": current_defaults,
        "distributionDefinitions": distribution_definitions,
        "distributionOccurrences": distribution_occurrences,
        "componentDefinitions": component_definitions,
        "componentOccurrences": component_occurrences,
        "activeReferenceKinds": ["DISTRIBUTION_TARGET", "TYPEDATA_COMPONENT"],
        "summary": {
            "distributionTargetUniqueCount": len(distribution_definitions),
            "distributionTargetOccurrenceCount": len(distribution_occurrences),
            "receiptPackageIdentityPinnedUniqueCount": sum(
                bool(row["receiptPackageIdentityPinned"])
                for row in distribution_definitions
            ),
            "pinnedPayloadDecodedUniqueCount": sum(
                bool(row["pinnedPayloadDecoded"])
                for row in distribution_definitions
            ),
            "exactPhysicalSourcePackagePresentUniqueCount": sum(
                bool(row["exactPhysicalSourcePackagePresent"])
                for row in distribution_definitions
            ),
            "distributionTargetPayloadDecodedUniqueCount": (
                len(distribution_definitions)
                - len(unresolved_distribution_definitions)
            ),
            "distributionTargetPayloadUnresolvedUniqueCount": len(
                unresolved_distribution_definitions
            ),
            "distributionTargetPayloadUnresolvedOccurrenceCount": sum(
                len(row["occurrenceIds"])
                for row in unresolved_distribution_definitions
            ),
            "distributionTargetSemanticReadyUniqueCount": (
                len(distribution_definitions)
                - len(blocked_semantic_distribution_definitions)
            ),
            "distributionTargetSemanticReadyOccurrenceCount": (
                len(distribution_occurrences)
                - sum(
                    len(row["occurrenceIds"])
                    for row in blocked_semantic_distribution_definitions
                )
            ),
            "distributionTargetSemanticBlockedUniqueCount": len(
                blocked_semantic_distribution_definitions
            ),
            "distributionTargetSemanticBlockedOccurrenceCount": sum(
                len(row["occurrenceIds"])
                for row in blocked_semantic_distribution_definitions
            ),
            "pointLightTargetUniqueCount": 1,
            "pointLightTargetOccurrenceCount": 1,
            "activeReferenceUniqueCount": (
                len(distribution_definitions) + len(component_definitions)
            ),
            "activeReferenceOccurrenceCount": (
                len(distribution_occurrences) + len(component_occurrences)
            ),
            "decodedParticleParameterSemanticFieldCounts": dict(
                sorted(
                    {
                        name: decoded_semantic_fields[name]
                        for name in (
                            "MinInput",
                            "MaxInput",
                            "MinOutput",
                            "MaxOutput",
                            "ParamMode",
                        )
                    }.items()
                )
            ),
            "compiledExecutionAllowedReferenceCount": sum(
                bool(row["executionAdmission"]["allowed"])
                for row in distribution_definitions
            ),
            "compiledExecutionAllowedOccurrenceCount": sum(
                bool(row["executionAdmission"]["allowed"])
                for row in distribution_occurrences
            ),
            "currentRevisionClassDefaultRecoveryTargetCount": len(
                current_defaults["recoveryTargets"]
            ),
            "currentRevisionClassDefaultAdmissionAllowed": False,
            "currentRevisionTargetNonzeroArchetypeUniqueCount": len(
                current_nonzero_archetype_rows
            ),
        },
        "closureSha256": "",
    }
    require(
        closure["summary"]["distributionTargetUniqueCount"] == 15
        and closure["summary"]["distributionTargetOccurrenceCount"] == 17
        and closure["summary"]["receiptPackageIdentityPinnedUniqueCount"] == 8
        and closure["summary"]["pinnedPayloadDecodedUniqueCount"] == 7
        and closure["summary"]["exactPhysicalSourcePackagePresentUniqueCount"]
        == 3
        and closure["summary"]["distributionTargetPayloadDecodedUniqueCount"]
        == 14
        and closure["summary"]["distributionTargetPayloadUnresolvedUniqueCount"]
        == 1
        and closure["summary"]["distributionTargetPayloadUnresolvedOccurrenceCount"]
        == 2
        and closure["summary"]["distributionTargetSemanticReadyUniqueCount"] == 9
        and closure["summary"]["distributionTargetSemanticReadyOccurrenceCount"]
        == 9
        and closure["summary"]["distributionTargetSemanticBlockedUniqueCount"]
        == 6
        and closure["summary"]["distributionTargetSemanticBlockedOccurrenceCount"]
        == 8
        and closure["summary"]["compiledExecutionAllowedOccurrenceCount"] == 0,
        "active distribution-reference denominator changed",
    )
    require(
        len(current_nonzero_archetype_rows) == 1
        and current_nonzero_archetype_rows[0]["sourceNodeId"]
        == "BFX_HIGH_00:export:1730",
        "active distribution target archetype denominator changed",
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
    parser.add_argument("--action-cue-recipe", required=True, type=Path)
    parser.add_argument("--active-inventory", required=True, type=Path)
    parser.add_argument("--normalized-graph", required=True, type=Path)
    parser.add_argument("--external-module-closure", required=True, type=Path)
    parser.add_argument("--current-package-root", required=True, type=Path)
    parser.add_argument("--current-efgame-script-package", required=True, type=Path)
    parser.add_argument("--current-engine-script-package", required=True, type=Path)
    parser.add_argument("--point-light-package", required=True, type=Path)
    parser.add_argument("--mesh-rotation-recovery-package", required=True, type=Path)
    parser.add_argument("--color-scale-package", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    closure = build_closure(
        args.source_receipt,
        args.action_cue_recipe,
        args.active_inventory,
        args.normalized_graph,
        args.external_module_closure,
        args.current_package_root,
        args.current_efgame_script_package,
        args.current_engine_script_package,
        args.point_light_package,
        args.mesh_rotation_recovery_package,
        args.color_scale_package,
    )
    content = json_bytes(closure)
    if args.check:
        require(
            generated_text_matches(args.output, content),
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
