#!/usr/bin/env python3
"""Independent oracle for the Artist F source execution semantics receipt."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Any, Iterable

from effect_source_contract_io import load_strict_json_object
from extract_ue3_effect_material_closure import load_package, find_export
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    package_ref_path,
    parse_tagged_properties,
)


SCHEMA = "lostark.effect-source-execution-semantics"
FORMAT_VERSION = 1
PRODUCT_OWNER_BLOCKER = "FINAL_INTEGRATION_PRODUCT_ADMISSION_REQUIRED"
CUSTOM_HANDLER_BLOCKER = "EXACT_SOURCE_CLASS_HANDLER_NUMERIC_ORACLE_REQUIRED"
CUSTOM_DISTRIBUTION_BLOCKER = "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN"
DEFAULT_DEPENDENT_BLOCKER = "DISTRIBUTION_CLASS_DEFAULT_VALUE_UNRESOLVED"
DEFAULT_POLICY_BLOCKER = "DEFAULT_DEPENDENT_DISTRIBUTION_REQUIRES_TYPED_DEFAULT_POLICY"
FAIL_CLOSED_DEFAULT_POLICY = "FAIL_CLOSED_NULL_RAW_DISTRIBUTION"
CURRENT_ENGINE_CDO_POLICY = "CURRENT_ENGINE_CDO_RECONSTRUCTED"
ALLOWED_DEFAULT_POLICIES = {
    FAIL_CLOSED_DEFAULT_POLICY,
    CURRENT_ENGINE_CDO_POLICY,
}
EXPECTED_DEFAULT_DEPENDENT_COUNT = 137
EXPECTED_SPAWN_DEFAULT_DEPENDENT_COUNT = 36
EXPECTED_SPAWN_RATESCALE_DEFAULT_DEPENDENT_COUNT = 35
EXPECTED_SPAWN_RATE_DEFAULT_DEPENDENT_OCCURRENCE = 2
EXPECTED_SPAWN_PARAMETER_RATE_OCCURRENCE = 34
EXPECTED_NONZERO_SPAWN_RATES = {
    0: 7.0,
    3: 40.0,
    24: 80.0,
    26: 50.0,
}

EXPECTED = {
    "occurrenceCount": 35,
    "selectedLodFieldCount": 70,
    "moduleCount": 399,
    "propertyCount": 1434,
    "primitiveLeafCount": 1572,
    "distributionCount": 629,
    "inlineDistributionCount": 612,
    "localDistributionDefinitionCount": 15,
    "localDistributionOccurrenceCount": 17,
    "pointLightOccurrenceCount": 1,
    "nativeTailCount": 399,
    "externalNativeTailOccurrenceCount": 248,
    "seedCount": 14,
    "implicitDefaultCount": 14,
    "transportLiteralCount": 18,
    "exactClassCount": 40,
    "customOrSeededClassOccurrenceCount": 26,
}

DESCRIPTOR_FIELDS = (
    "propertyPath", "sourceClass", "sourceObjectPath", "componentCount",
    "operation", "randomLockAxes", "lookupTableChunkSize",
    "lookupTableNumElements", "lookupTableTimeScale", "lookupTableStartTime",
    "defaultMinimum", "defaultMaximum", "lookupTable", "keys", "referenceId",
    "occurrenceId", "payloadStatus", "fidelity",
)

IRRELEVANT_PROPERTIES = {"b3ddrawmode", "lodvalidity"}
ALLOWED_DECISIONS = {"READY_FOR_HANDLER", "VERIFIED_IRRELEVANT", "BLOCKED"}
SAMPLES = (
    (0.0, [0.0, 0.25, 0.5, 0.75]),
    (0.25, [0.25, 0.5, 0.75, 1.0]),
    (1.0, [1.0, 0.75, 0.5, 0.25]),
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(value, ensure_ascii=False, sort_keys=True,
                   separators=(",", ":")).encode("utf-8")
    ).hexdigest()


def canonical_text_sha256(path: Path) -> str:
    text = path.read_text(encoding="utf-8-sig").replace("\r\n", "\n")
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def legacy_normalized(value: str) -> str:
    result = value.casefold()
    if result.startswith("efparticlemodule"):
        result = "particlemodule" + result[len("efparticlemodule") :]
    if result.endswith("_seeded"):
        result = result[: -len("_seeded")]
    return result


def custom_handler(value: str) -> bool:
    return legacy_normalized(value) != value.casefold()


def as_vector(value: Any) -> list[float]:
    if isinstance(value, dict):
        rows = [value.get(name, 0.0) for name in ("x", "y", "z", "w")]
    elif isinstance(value, list):
        rows = list(value[:4])
    elif isinstance(value, (int, float)) and not isinstance(value, bool):
        rows = [value] * 4
    else:
        rows = [0.0] * 4
    rows += [0.0] * (4 - len(rows))
    result = [float(row) for row in rows]
    require(all(math.isfinite(row) for row in result), "oracle input is not finite")
    return result


def mix(a: list[float], b: list[float], t: float) -> list[float]:
    return [(1.0 - t) * left + t * right for left, right in zip(a, b)]


def range_value(
    lower: list[float], upper: list[float], operation: int,
    random_values: list[float],
) -> list[float]:
    if operation == 2:
        return [
            lower[index] + (upper[index] - lower[index])
            * min(1.0, max(0.0, random_values[index]))
            for index in range(4)
        ]
    if operation == 3:
        return list(lower if random_values[0] < 0.5 else upper)
    return list(lower)


def cubic(
    p0: list[float], m0: list[float], p1: list[float], m1: list[float],
    t: float, span: float,
) -> list[float]:
    a = 2.0 * t ** 3 - 3.0 * t ** 2 + 1.0
    b = t ** 3 - 2.0 * t ** 2 + t
    c = -2.0 * t ** 3 + 3.0 * t ** 2
    d = t ** 3 - t ** 2
    return [
        p0[index] * a + m0[index] * b * span
        + p1[index] * c + m1[index] * d * span
        for index in range(4)
    ]


def evaluate(desc: dict[str, Any], time: float, random_values: list[float]) -> list[float]:
    count = int(desc["componentCount"])
    operation = int(desc["operation"])
    table = [float(row) for row in desc["lookupTable"]]
    if table:
        chunk = int(desc["lookupTableChunkSize"]) or count * (
            2 if operation >= 2 else 1
        )
        entries, remainder = divmod(len(table) - 2, chunk)
        require(entries > 0 and remainder == 0, "lookup shape is invalid")
        time_scale = float(desc["lookupTableTimeScale"])
        cursor = ((time - float(desc["lookupTableStartTime"])) * time_scale
                  if time_scale > 0.0 else 0.0)
        cursor = max(0.0, min(float(entries - 1), cursor))
        left = int(cursor)
        right = min(left + 1, entries - 1)
        ratio = cursor - left

        def read(entry: int, high: bool) -> list[float]:
            base = 2 + entry * chunk + (count if high and chunk >= count * 2 else 0)
            result = [0.0] * 4
            for component in range(count):
                result[component] = table[base + component]
            return result

        return range_value(
            mix(read(left, False), read(right, False), ratio),
            mix(read(left, True), read(right, True), ratio),
            operation, random_values,
        )

    keys = desc["keys"]
    if not keys:
        return range_value(
            as_vector(desc["defaultMinimum"]), as_vector(desc["defaultMaximum"]),
            operation, random_values,
        )
    if time <= float(keys[0]["time"]):
        return range_value(as_vector(keys[0]["minimum"]),
                           as_vector(keys[0]["maximum"]), operation, random_values)
    if time >= float(keys[-1]["time"]):
        return range_value(as_vector(keys[-1]["minimum"]),
                           as_vector(keys[-1]["maximum"]), operation, random_values)
    for left, right in zip(keys, keys[1:]):
        if time > float(right["time"]):
            continue
        begin = float(left["time"])
        span = float(right["time"]) - begin
        ratio = 0.0 if span <= 0.0 else (time - begin) / span
        interpolation = str(left["interpolation"]).casefold()
        if interpolation == "constant":
            low, high = as_vector(left["minimum"]), as_vector(left["maximum"])
        elif interpolation == "cubic":
            low = cubic(as_vector(left["minimum"]),
                        as_vector(left["leaveTangentMinimum"]),
                        as_vector(right["minimum"]),
                        as_vector(right["arriveTangentMinimum"]), ratio, span)
            high = cubic(as_vector(left["maximum"]),
                         as_vector(left["leaveTangentMaximum"]),
                         as_vector(right["maximum"]),
                         as_vector(right["arriveTangentMaximum"]), ratio, span)
        else:
            low = mix(as_vector(left["minimum"]), as_vector(right["minimum"]), ratio)
            high = mix(as_vector(left["maximum"]), as_vector(right["maximum"]), ratio)
        return range_value(low, high, operation, random_values)
    raise ValueError("curve interval was not selected")


def close_enough(left: Any, right: Any) -> bool:
    if isinstance(left, list) and isinstance(right, list):
        return len(left) == len(right) and all(
            close_enough(a, b) for a, b in zip(left, right)
        )
    if isinstance(left, (int, float)) and isinstance(right, (int, float)):
        return math.isclose(float(left), float(right), rel_tol=1e-6, abs_tol=1e-7)
    return left == right


def is_zero_vector(value: Any) -> bool:
    return as_vector(value) == [0.0, 0.0, 0.0, 0.0]


def verify_null_raw_distribution_projection(
    semantic_distribution: dict[str, Any], descriptor: dict[str, Any],
) -> None:
    evidence = semantic_distribution.get("fieldEvidence") or {}
    raw_fields = evidence.get("rawFieldNames") or []
    require(
        evidence.get("defaultDependent") is True
        and raw_fields and raw_fields[0] == "distribution"
        and set(raw_fields).issubset({
            "distribution", "lookuptabletimescale", "lookuptablestarttime",
        })
        and evidence.get("hasLookupPayload") is False
        and evidence.get("explicitOperation") is None
        and DEFAULT_DEPENDENT_BLOCKER
        in semantic_distribution.get("executionBlockers", []),
        "null RawDistribution semantic evidence changed",
    )
    require(
        str(descriptor.get("sourceClass") or "") == ""
        and str(descriptor.get("sourceObjectPath") or "") == ""
        and 1 <= int(descriptor.get("componentCount", 0)) <= 4
        and int(descriptor.get("operation", -1)) == 1
        and int(descriptor.get("lookupTableChunkSize", -1)) == 0
        and int(descriptor.get("lookupTableNumElements", -1)) == 0
        and is_zero_vector(descriptor.get("defaultMinimum"))
        and is_zero_vector(descriptor.get("defaultMaximum"))
        and descriptor.get("lookupTable") == []
        and descriptor.get("keys") == []
        and str(descriptor.get("referenceId") or "") == ""
        and str(descriptor.get("occurrenceId") or "") == "",
        "null RawDistribution numeric payload shape changed",
    )


def verify_spawn_ratescale_raw_distribution_float(
    semantic_distribution: dict[str, Any], descriptor: dict[str, Any],
) -> None:
    field_evidence = semantic_distribution.get("fieldEvidence") or {}
    require(
        str(semantic_distribution.get("propertyPath") or "").casefold()
        == "ratescale"
        and str(semantic_distribution.get("topLevelPropertyPath") or "").casefold()
        == "ratescale"
        and str(semantic_distribution.get("exactSourceClass") or "") == ""
        and semantic_distribution.get("evaluatorId")
        == "ue3.raw-distribution.cooked-lookup.v1"
        and field_evidence.get("rawFieldNames") == ["distribution"]
        and field_evidence.get("reconstructedFieldNames") == ["operation"]
        and int(descriptor.get("componentCount", 0)) == 1
        and str(descriptor.get("sourceClass") or "") == "",
        "Spawn RateScale RawDistributionFloat source shape changed",
    )


def full35_default_dependent_gate(
    semantic: dict[str, Any], candidate: dict[str, Any],
) -> dict[str, Any]:
    semantic_occurrences = semantic.get("occurrences") or []
    candidate_elements = candidate.get("elements") or []
    require(len(semantic_occurrences) == len(candidate_elements)
            == EXPECTED["occurrenceCount"],
            "Full35 default gate occurrence denominator changed")
    default_count = 0
    spawn_default_count = 0
    spawn_ratescale_default_count = 0
    rate_default_occurrences: list[int] = []
    zero_rate_occurrences: list[int] = []
    nonzero_rates: dict[int, float] = {}
    parameter_occurrences: list[int] = []
    spawn_count = 0
    for occurrence_index, (semantic_occurrence, element) in enumerate(zip(
        semantic_occurrences, candidate_elements
    )):
        semantic_modules = semantic_occurrence.get("modules") or []
        candidate_modules = (element.get("sourceRecipe") or {}).get("modules") or []
        require(len(semantic_modules) == len(candidate_modules),
                "Full35 default gate module denominator changed")
        for semantic_module, candidate_module in zip(
            semantic_modules, candidate_modules
        ):
            semantic_distributions = semantic_module.get("distributions") or []
            candidate_distributions = candidate_module.get("distributions") or []
            require(len(semantic_distributions) == len(candidate_distributions),
                    "Full35 default gate distribution denominator changed")
            by_path: dict[str, list[dict[str, Any]]] = {}
            for descriptor in candidate_distributions:
                by_path.setdefault(
                    str(descriptor.get("propertyPath") or "").casefold(), []
                ).append(descriptor)
            exact_class = str(semantic_module.get("exactSourceClass") or "").casefold()
            if exact_class == "particlemodulespawn":
                spawn_count += 1
            for semantic_distribution in semantic_distributions:
                path = str(semantic_distribution.get("propertyPath") or "").casefold()
                candidates = by_path.get(path) or []
                require(candidates, "Full35 default gate payload path changed")
                descriptor = candidates.pop(0)
                default_dependent = bool(
                    (semantic_distribution.get("fieldEvidence") or {}).get(
                        "defaultDependent"
                    )
                )
                if default_dependent:
                    default_count += 1
                    verify_null_raw_distribution_projection(
                        semantic_distribution, descriptor
                    )
                if exact_class != "particlemodulespawn":
                    continue
                require(path in {"rate", "ratescale"},
                        "ParticleModuleSpawn distribution set changed")
                if default_dependent:
                    spawn_default_count += 1
                    if path == "ratescale":
                        verify_spawn_ratescale_raw_distribution_float(
                            semantic_distribution, descriptor
                        )
                        spawn_ratescale_default_count += 1
                    else:
                        rate_default_occurrences.append(occurrence_index)
                    continue
                if path == "ratescale":
                    raise ValueError("ParticleModuleSpawn RateScale became explicit")
                source_class = str(
                    semantic_distribution.get("exactSourceClass") or ""
                ).casefold()
                if source_class == "distributionfloatparticleparameter":
                    require(
                        occurrence_index == EXPECTED_SPAWN_PARAMETER_RATE_OCCURRENCE
                        and str(descriptor.get("sourceClass") or "").casefold()
                        == source_class
                        and descriptor.get("lookupTable") == []
                        and str(descriptor.get("referenceId") or "") != ""
                        and str(descriptor.get("occurrenceId") or "") != "",
                        "Full35 Spawn parameter Rate branch changed",
                    )
                    parameter_occurrences.append(occurrence_index)
                    continue
                table = [float(value) for value in descriptor.get("lookupTable", [])]
                require(
                    all(math.isfinite(value) for value in table)
                    and str(descriptor.get("sourceClass") or "") == ""
                    and int(descriptor.get("operation", -1)) == 1
                    and int(descriptor.get("lookupTableChunkSize", -1)) == 1
                    and int(descriptor.get("lookupTableNumElements", -1)) == 1
                    and len(table) == 4 and len(set(table)) == 1,
                    "Full35 explicit Spawn Rate shape changed",
                )
                if table[0] == 0.0:
                    zero_rate_occurrences.append(occurrence_index)
                else:
                    nonzero_rates[occurrence_index] = table[0]
            require(not any(by_path.values()),
                    "Full35 default gate left unmatched payload distributions")
    require(spawn_count == 35
            and default_count == EXPECTED_DEFAULT_DEPENDENT_COUNT
            and spawn_default_count == EXPECTED_SPAWN_DEFAULT_DEPENDENT_COUNT
            and spawn_ratescale_default_count
            == EXPECTED_SPAWN_RATESCALE_DEFAULT_DEPENDENT_COUNT
            and rate_default_occurrences
            == [EXPECTED_SPAWN_RATE_DEFAULT_DEPENDENT_OCCURRENCE]
            and len(zero_rate_occurrences) == 29
            and nonzero_rates == EXPECTED_NONZERO_SPAWN_RATES
            and parameter_occurrences
            == [EXPECTED_SPAWN_PARAMETER_RATE_OCCURRENCE],
            "Full35 Spawn/default-dependent exact gate changed")
    return {
        "defaultDependentCount": default_count,
        "spawnDefaultDependentCount": spawn_default_count,
        "spawnRateScaleDefaultDependentCount": spawn_ratescale_default_count,
        "spawnRateScaleSourceDistributionKind": "RawDistributionFloat",
        "spawnRateDefaultDependentOccurrences": rate_default_occurrences,
        "spawnExplicitZeroRateCount": len(zero_rate_occurrences),
        "spawnExplicitNonzeroRates": {
            str(index): value for index, value in sorted(nonzero_rates.items())
        },
        "spawnParameterRateOccurrences": parameter_occurrences,
    }


def current_engine_spawn_ratescale_reconstruction(
    defaults: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    cdo = defaults["classDefaultObjects"]["particleModuleSpawn"]
    require(cdo["recordSha256"] == canonical_sha256(cdo["properties"]),
            "current ParticleModuleSpawn CDO record hash changed")
    tagged = cdo["properties"].get("ratescale") or {}
    raw_value = tagged.get("value") or {}
    fields = raw_value.get("properties") or {}
    require(
        tagged.get("type") == "structproperty"
        and tagged.get("structtype") == "rawdistributionfloat"
        and int(raw_value.get("size", -1)) == 268
        and int((fields.get("distribution") or {}).get("value", 0)) != 0
        and (fields.get("op") or {}).get("value") == 1
        and (fields.get("lookuptablechunksize") or {}).get("value") == 1
        and (fields.get("lookuptablenumelements") or {}).get("value") == 1
        and (fields.get("lookuptable") or {}).get("value")
        == [1.0, 1.0, 1.0, 1.0]
        and (fields.get("lookuptabletimescale") or {}).get("value") == 0.0
        and (fields.get("lookuptablestarttime") or {}).get("value") == 0.0,
        "current ParticleModuleSpawn RateScale CDO value changed",
    )
    engine = next(
        row for row in defaults["scriptPackages"]
        if str(row.get("logicalPackage") or "").casefold() == "engine"
    )
    descriptor = {
        "propertyPath": "ratescale", "sourceClass": "",
        "sourceObjectPath": cdo["objectPath"], "componentCount": 1,
        "operation": 1, "randomLockAxes": 0,
        "lookupTableChunkSize": 1, "lookupTableNumElements": 1,
        "lookupTableTimeScale": 0.0, "lookupTableStartTime": 0.0,
        "defaultMinimum": [0.0, 0.0, 0.0, 0.0],
        "defaultMaximum": [0.0, 0.0, 0.0, 0.0],
        "lookupTable": [1.0, 1.0, 1.0, 1.0], "keys": [],
        "referenceId": "", "occurrenceId": "",
        "payloadStatus": CURRENT_ENGINE_CDO_POLICY,
        "fidelity": CURRENT_ENGINE_CDO_POLICY,
    }
    evidence = {
        "policyId": CURRENT_ENGINE_CDO_POLICY,
        "status": "RESOLVED_FOR_RECONSTRUCTED_HANDLER",
        "provenanceTier": CURRENT_ENGINE_CDO_POLICY,
        "sourceExact": False,
        "sourceExactUpgradeAllowed": False,
        "sourceEraIdentityPinned": False,
        "logicalPackage": engine["logicalPackage"],
        "physicalPackage": engine["physicalPackage"],
        "packageBytes": engine["bytes"],
        "packageSha256": engine["sha256"],
        "classDefaultObject": cdo["objectPath"],
        "classDefaultExportIndex": cdo["exportIndex"],
        "classDefaultRecordSha256": cdo["recordSha256"],
        "propertyPath": "ratescale", "value": 1.0,
        "reconstructedDescriptorSha256": canonical_sha256(descriptor),
    }
    return descriptor, evidence


def expected_default_policy(
    policy_id: str, defaults: dict[str, Any],
) -> dict[str, Any]:
    require(policy_id in ALLOWED_DEFAULT_POLICIES,
            "default-dependent policy is missing or unknown")
    row = {
        "policyId": policy_id, "sourceExact": False,
        "sourceExactUpgradeAllowed": False,
        "nullRawDistributionNumericPayloadByteCount": 0,
        "numericPayloadDefinition": "LOOKUP_TABLE_OR_KEY_VALUE_BYTES",
    }
    if policy_id == FAIL_CLOSED_DEFAULT_POLICY:
        row.update({"mode": "FAIL_CLOSED", "resolvedScope": []})
    else:
        _, evidence = current_engine_spawn_ratescale_reconstruction(defaults)
        row.update({
            "mode": CURRENT_ENGINE_CDO_POLICY,
            "resolvedScope": [{
                "exactSourceClass": "particlemodulespawn",
                "propertyPath": "ratescale",
                "sourceDistributionKind": "RawDistributionFloat",
                "componentCount": 1,
                "occurrenceCount": EXPECTED_SPAWN_RATESCALE_DEFAULT_DEPENDENT_COUNT,
            }],
            "typedEvidence": evidence,
        })
    return row


def normalized_tagged(value: Any) -> Any:
    if isinstance(value, list):
        return [normalized_tagged(row) for row in value]
    if isinstance(value, dict):
        result = {}
        for name, row in value.items():
            key = str(name).casefold()
            result[key] = (
                row.casefold() if key in {"type", "structtype"}
                and isinstance(row, str) else normalized_tagged(row)
            )
        return result
    return value


def local_oracle(row: dict[str, Any]) -> tuple[str, Any]:
    fields = {
        str(field["fieldPath"]).casefold(): field.get("value")
        for field in row["currentRevisionFields"]
    }
    source_class = str(row["exactSourceClass"]).casefold()
    if source_class == "distributionfloatconstantcurve":
        return "STANDARD_CONSTANT_CURVE", None
    samples = row["numericOracleSamples"]
    if source_class == "efdistributionvectormultiplyparticleparameter":
        require(row["decision"] == "BLOCKED" and not samples,
                "custom local evaluator became executable")
        return "UNRESOLVED_CUSTOM_EVALUATOR", None
    require(len(samples) == 1, "standard local oracle sample count changed")
    sample = samples[0]
    parameter = sample.get("parameterInput")
    constant = as_vector(fields.get("constant"))
    component_count = int(row["reconstructedDescriptor"]["componentCount"])
    if parameter is None:
        return "CONSTANT_FALLBACK", constant[:component_count]
    input_value = as_vector(parameter["value"])
    output = [0.0] * 4
    for index in range(component_count):
        mode_key = "parammode" if component_count == 1 else (
            "parammodes" if index == 0 else f"parammodes[{index}]"
        )
        mode = str(fields.get(mode_key, "dpm_normal")).casefold()
        if mode == "dpm_direct":
            output[index] = input_value[index]
            continue
        current = abs(input_value[index]) if mode == "dpm_abs" else input_value[index]
        input_min = as_vector(fields.get("mininput"))[index]
        input_max = as_vector(fields.get("maxinput"))[index]
        output_min = as_vector(fields.get("minoutput"))[index]
        output_max = as_vector(fields.get("maxoutput"))[index]
        ratio = 0.0 if input_min == input_max else (
            (current - input_min) / (input_max - input_min)
        )
        ratio = max(0.0, min(1.0, ratio))
        output[index] = output_min + (output_max - output_min) * ratio
    return "PARAMETER_INPUT", output[:component_count]


def expected_local_fields(definition: dict[str, Any]) -> list[dict[str, Any]]:
    fields: dict[str, dict[str, Any]] = {}
    coverage = definition.get("semanticCoverage") or {}
    for name, resolution in (coverage.get("resolvedFields") or {}).items():
        selected = resolution.get("selected") if isinstance(resolution, dict) else None
        if not isinstance(selected, dict):
            continue
        tagged = selected.get("value")
        value = tagged.get("value") if isinstance(tagged, dict) else tagged
        if value is None:
            continue
        fields[str(name).casefold()] = {
            "fieldPath": str(name).casefold(),
            "value": copy.deepcopy(value),
            "provenanceTier": str(selected.get("tier") or ""),
            "evidenceStatus": str(selected.get("evidenceStatus") or ""),
        }
    current_object = (
        (definition.get("currentRevisionTargetEvidence") or {}).get("object") or {}
    )
    for name, tagged in (current_object.get("properties") or {}).items():
        if isinstance(tagged, dict):
            fields[str(name).casefold()] = {
                "fieldPath": str(name).casefold(),
                "value": copy.deepcopy(tagged.get("value")),
                "provenanceTier": "CURRENT_REVISION_INSTANCE_EXPLICIT",
                "evidenceStatus": "CURRENT_REVISION_TARGET_EVIDENCE",
            }
    if "particleparameter" in str(definition.get("sourceClass") or "").casefold() \
            and "parametername" not in fields:
        fields["parametername"] = {
            "fieldPath": "parametername",
            "value": None,
            "provenanceTier": "CURRENT_NATIVE_EVALUATOR_DEFAULT",
            "evidenceStatus": "CURRENT_NATIVE_EVALUATOR_SHAPE_ONLY",
        }
    return [fields[name] for name in sorted(fields)]


def action_parameter_index(action_recipe: dict[str, Any]) -> dict[str, dict[str, Any]]:
    result = {}
    for cue in action_recipe.get("cues", []):
        values = {}
        for row in (cue.get("typedPayload") or {}).get("parameterOverrides", []):
            kind = str(row.get("type") or "").casefold()
            if kind == "scalar":
                value = float(row["scalarValue"])
            elif kind == "vector":
                value = as_vector(row["vectorValue"])[:3]
            else:
                continue
            values[str(row.get("name") or "").casefold()] = {
                "name": str(row.get("name") or ""),
                "kind": kind,
                "value": value,
                "sourceIndex": int(row.get("sourceIndex", -1)),
                "sourceValueByteOffset": int(row.get("sourceValueByteOffset", -1)),
            }
        result[str(cue.get("cueId") or "")] = values
    return result


def verify_deep_native(
    receipt: dict[str, Any], external: dict[str, Any], release_root: Path,
) -> None:
    package_sources = {
        str(row["logicalPackage"]).casefold(): row
        for row in external.get("packages", [])
    }
    source_objects = {
        str(obj["objectId"]): obj
        for package in external.get("packages", [])
        for obj in package.get("objects", [])
    }
    evidence = receipt["externalNativeTailEvidence"]
    package_rows = {
        str(row["logicalPackage"]).casefold(): row for row in evidence["packages"]
    }
    cache = {}
    for logical, row in package_rows.items():
        path = release_root / "Packages" / row["physicalPackage"]
        require(path.stat().st_size == row["bytes"] and sha256_file(path) == row["sha256"],
                f"external current package identity changed: {logical}")
        source_package = package_sources[logical]
        require(source_package["physicalPackage"] == row["physicalPackage"],
                f"external physical mapping changed: {logical}")
        cache[logical] = load_package(path, LOSTARK_KR_AES_KEY)
    for row in evidence["objects"]:
        source = source_objects.get(row["sourceObjectId"])
        require(source is not None, f"native source object is missing: {row['sourceObjectId']}")
        package = cache[str(row["logicalPackage"]).casefold()]
        entry = find_export(package, row["objectPath"])
        serial = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        properties, end = parse_tagged_properties(serial, package.names, package.summary.version)
        require(entry.index == row["exportIndex"] and entry.serial_size == row["serialSize"],
                f"native export identity changed: {row['sourceObjectId']}")
        require(hashlib.sha256(serial).hexdigest() == row["serialSha256"],
                f"native serial changed: {row['sourceObjectId']}")
        require(end == row["propertyStreamEnd"] == row["serialSize"],
                f"native tail became executable: {row['sourceObjectId']}")
        require(normalized_tagged(properties) == normalized_tagged(source["properties"]),
                f"native decoded properties changed: {row['sourceObjectId']}")
        require((package_ref_name(entry.class_index, package.imports, package.exports) or "").casefold()
                == row["className"], f"native class changed: {row['sourceObjectId']}")
        require(package_ref_path(entry.index + 1, package.imports, package.exports).casefold()
                == str(row["objectPath"]).casefold(),
                f"native object path changed: {row['sourceObjectId']}")

    for row in receipt["currentRevisionDefaultEvidence"]["scriptPackages"]:
        path = release_root / row["physicalPackage"]
        require(path.stat().st_size == row["bytes"] and sha256_file(path) == row["sha256"],
                f"current script identity changed: {row['logicalPackage']}")
        require(row["sourceEraIdentityPinned"] is False,
                "current script was upgraded to source-exact")
    script_by_name = {
        str(row["logicalPackage"]).casefold(): load_package(
            release_root / row["physicalPackage"], LOSTARK_KR_AES_KEY
        )
        for row in receipt["currentRevisionDefaultEvidence"]["scriptPackages"]
    }
    cdo_specs = {
        "particleLodLevel": ("engine", "Default__ParticleLODLevel"),
        "particleModuleRequired": ("engine", "Default__ParticleModuleRequired"),
        "particleModuleSpawn": ("engine", "Default__ParticleModuleSpawn"),
        "particleModuleTypeDataRibbon": (
            "engine", "Default__ParticleModuleTypeDataRibbon"
        ),
        "efParticleModuleTypeDataDecal": (
            "efgame", "Default__EFParticleModuleTypeDataDecal"
        ),
    }
    stored_cdos = receipt["currentRevisionDefaultEvidence"]["classDefaultObjects"]
    for key, stored in stored_cdos.items():
        if not str(key).startswith("seed::"):
            continue
        class_name = str(key).partition("seed::")[2]
        cdo_specs[key] = (
            "efgame" if class_name.startswith("ef") else "engine",
            str(stored["objectPath"]),
        )
    for key, (logical, object_path) in cdo_specs.items():
        package = script_by_name[logical]
        entry = find_export(package, object_path)
        serial = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        properties, end = parse_tagged_properties(
            serial, package.names, package.summary.version
        )
        stored = stored_cdos[key]
        require(end == entry.serial_size
                and stored["exportIndex"] == entry.index
                and stored["objectPath"].casefold()
                == package_ref_path(
                    entry.index + 1, package.imports, package.exports
                ).casefold()
                and stored["className"]
                == (package_ref_name(
                    entry.class_index, package.imports, package.exports
                ) or "").casefold()
                and stored["properties"] == normalized_tagged(properties)
                and stored["recordSha256"]
                == canonical_sha256(normalized_tagged(properties)),
                f"current CDO evidence changed: {key}")


def verify_receipt(
    receipt: dict[str, Any], *, root: Path, inputs: dict[str, dict[str, Any]],
    release_root: Path | None = None,
) -> dict[str, int]:
    require(receipt.get("schema") == SCHEMA, "receipt schema changed")
    require(type(receipt.get("formatVersion")) is int
            and receipt["formatVersion"] == FORMAT_VERSION,
            "receipt version changed")
    unsigned = copy.deepcopy(receipt)
    stored_hash = str(unsigned.pop("receiptSha256", ""))
    require(stored_hash and canonical_sha256(unsigned) == stored_hash,
            "receipt self hash changed")
    require(receipt.get("characterClass") == "ARTIST"
            and receipt.get("skillId") == 31470
            and receipt.get("inputSlot") == "F",
            "receipt root identity changed")
    require(receipt.get("productAdmission", {}).get("allowed") is False,
            "receipt granted Product admission")
    require(PRODUCT_OWNER_BLOCKER in receipt.get("blockerUnion", []),
            "Product owner blocker is missing")
    require(receipt["summary"]["denominators"] == EXPECTED,
            "receipt denominators changed")
    require(receipt["summary"]["silentFallbackCount"] == 0,
            "silent fallback count changed")
    defaults = receipt["currentRevisionDefaultEvidence"]
    require(defaults["status"] == "CURRENT_REVISION_DEFAULTS_NOT_SOURCE_EXACT",
            "current default fidelity changed")
    require(defaults["selectedLodProjection"] == {
        "level": 0,
        "enabled": True,
        "decision": "VERIFIED_IRRELEVANT",
        "reason": "FIRST_LOD_ONLY_SOURCE_IDENTITY_COMPILED_AT_ADAPTER_BUILD",
    }, "selected LOD default projection changed")
    require(defaults["requiredLocalSpace"] == {
        "value": False,
        "provenance": "CURRENT_INHERITED_LANGUAGE_DEFAULT",
    }, "Required local-space current default changed")
    require(defaults["decal"]["values"]["defaultsize"] == {
        "x": 50.0, "y": 50.0
    } and defaults["decal"]["values"]["farplane"] == 300.0,
            "Decal current defaults changed")
    require(defaults["ribbon"]["values"]["sheetspertrail"] == 1
            and defaults["ribbon"]["values"]["maxparticleintrailcount"] == 500,
            "Ribbon current defaults changed")
    require(defaults["randomSeedPolicy"] == {
        "parameterName": None,
        "getSeedFromInstance": False,
        "instanceSeedIsIndex": False,
        "resetSeedOnEmitterLooping": True,
        "randomlySelectSeedArray": False,
        "emptyArrayUsesOccurrenceRandomStream": True,
        "provenance": "CURRENT_ENGINE_OR_EFGAME_SEEDED_CLASS_CDO_QUORUM",
        "sourceEraIdentityPinned": False,
        "classCount": 8,
    }, "random seed current CDO quorum changed")
    policy_id = str((receipt.get("defaultDependentPolicy") or {}).get(
        "policyId"
    ) or "")
    require(receipt.get("defaultDependentPolicy")
            == expected_default_policy(policy_id, defaults),
            "default-dependent typed policy evidence changed")

    for name, row in receipt["inputs"].items():
        source = inputs.get(name)
        require(source is not None, f"tracked input is missing: {name}")
        require(canonical_sha256(source) == row["canonicalJsonSha256"],
                f"tracked input semantic hash changed: {name}")
    for row in receipt["toolIdentities"].values():
        path = root / row["path"]
        require(canonical_text_sha256(path) == row["canonicalTextSha256"],
                f"tracked tool identity changed: {row['path']}")

    semantic = inputs["semanticClosure"]
    candidate = inputs["candidate"]
    require(receipt.get("full35DefaultDependentGate")
            == full35_default_dependent_gate(semantic, candidate),
            "Full35 default-dependent gate changed")
    local_closure = inputs["localReferenceClosure"]
    expected_script_packages = [
        {
            **copy.deepcopy(row),
            "provenance": "LATE_PINNED_CURRENT_REVISION_ENVIRONMENT_RECEIPT",
            "sourceEraIdentityPinned": False,
        }
        for row in (
            (local_closure.get("currentRevisionClassDefaultEvidence") or {}).get(
                "scriptPackages", []
            )
        )
    ]
    require(defaults["scriptPackages"] == expected_script_packages,
            "current script package pin changed")
    require(all(
        row["recordSha256"] == canonical_sha256(row["properties"])
        for row in defaults["classDefaultObjects"].values()
    ), "current CDO record hash changed")
    local_definitions = {
        str(row["definitionId"]): row
        for row in local_closure.get("distributionDefinitions", [])
    }
    local_occurrences = {
        str(row["occurrenceId"]): row
        for row in local_closure.get("distributionOccurrences", [])
    }
    cue_parameters = action_parameter_index(inputs["actionCueRecipe"])
    semantic_occurrences = semantic["occurrences"]
    candidate_elements = candidate["elements"]
    receipt_occurrences = receipt["occurrences"]
    require(len(semantic_occurrences) == len(candidate_elements)
            == len(receipt_occurrences) == EXPECTED["occurrenceCount"],
            "occurrence coverage changed")

    all_module_ids: set[str] = set()
    all_property_ids: set[str] = set()
    all_leaf_ids: set[str] = set()
    all_distribution_ids: set[str] = set()
    all_payload_literals: set[str] = set()
    property_literal_refs: set[str] = set()
    leaf_literal_refs: set[str] = set()
    all_payload_distributions: set[str] = set()
    distribution_payload_refs: set[str] = set()
    class_counts: Counter[str] = Counter()
    module_decisions: Counter[str] = Counter()
    property_decisions: Counter[str] = Counter()
    leaf_decisions: Counter[str] = Counter()
    distribution_decisions: Counter[str] = Counter()
    seed_count = 0
    default_count = 0
    default_dependent_count = 0
    default_dependent_reconstructed_count = 0
    selected_lod_count = 0
    external_native_count = 0

    for occurrence_index, (
        semantic_occurrence, element, occurrence
    ) in enumerate(zip(semantic_occurrences, candidate_elements, receipt_occurrences)):
        for name in ("occurrenceCompositeId", "evidenceId", "sourceOccurrenceId",
                     "sourceSystemId", "sourceEmitterPath", "rendererType"):
            require(occurrence[name] == semantic_occurrence[name],
                    f"occurrence identity changed: {name}")
        lod = occurrence["selectedLod"]
        semantic_lod = semantic_occurrence["selectedLodSemantics"]
        require(lod["sourceLodNodeId"] == semantic_lod["sourceLodNodeId"]
                and lod["sourceLodRecordSha256"] == semantic_lod["sourceLodRecordSha256"],
                "selected LOD identity changed")
        require(len(lod["fields"]) == 2 and all(
            row["decision"] == "VERIFIED_IRRELEVANT" for row in lod["fields"]
        ), "selected LOD default was not compile-time irrelevant")
        selected_lod_count += len(lod["fields"])

        recipe = element["sourceRecipe"]
        require(len(occurrence["modules"]) == len(semantic_occurrence["modules"])
                == len(recipe["modules"]), "module ordering changed")
        for semantic_module, candidate_module, module in zip(
            semantic_occurrence["modules"], recipe["modules"], occurrence["modules"]
        ):
            module_id = module["moduleOccurrenceId"]
            require(module_id == semantic_module["moduleOccurrenceId"]
                    and module_id not in all_module_ids,
                    f"module stable identity changed: {module_id}")
            all_module_ids.add(module_id)
            require(module["order"] == semantic_module["order"],
                    f"module order changed: {module_id}")
            require(module["sourceObjectId"] == semantic_module["sourceObjectId"]
                    and module["sourceRecordSha256"] == semantic_module["sourceRecordSha256"],
                    f"module evidence binding changed: {module_id}")
            exact_class = str(semantic_module["exactSourceClass"]).casefold()
            require(module["exactSourceClass"] == exact_class
                    and module["normalizedAliasAllowed"] is False,
                    f"module class was normalized: {module_id}")
            class_counts[exact_class] += 1

            typed = module["typedPayload"]
            unsigned_payload = {name: copy.deepcopy(typed[name]) for name in (
                "stableId", "className", "objectPath", "literals", "distributions"
            )}
            require(canonical_sha256(unsigned_payload) == typed["payloadSha256"],
                    f"typed module payload hash changed: {module_id}")
            require(typed["stableId"] == candidate_module["stableId"]
                    and typed["className"] == candidate_module["className"]
                    and typed["objectPath"] == candidate_module["objectPath"],
                    f"typed module header changed: {module_id}")
            require([
                {name: row[name] for name in ("propertyPath", "kind", "value")}
                for row in typed["literals"]
            ] == candidate_module["literals"], f"typed literals changed: {module_id}")
            candidate_descriptors = [
                {name: copy.deepcopy(row.get(name)) for name in DESCRIPTOR_FIELDS}
                for row in candidate_module["distributions"]
            ]
            typed_descriptors = [row["descriptor"] for row in typed["distributions"]]
            require(typed_descriptors == candidate_descriptors,
                    f"typed distributions changed: {module_id}")

            module_literal_ids = {row["literalId"] for row in typed["literals"]}
            require(len(module_literal_ids) == len(typed["literals"])
                    and not module_literal_ids.intersection(all_payload_literals),
                    f"literal identity collision: {module_id}")
            all_payload_literals.update(module_literal_ids)
            module_distribution_ids = {
                row["payloadDistributionId"] for row in typed["distributions"]
            }
            require(len(module_distribution_ids) == len(typed["distributions"])
                    and not module_distribution_ids.intersection(all_payload_distributions),
                    f"payload distribution collision: {module_id}")
            all_payload_distributions.update(module_distribution_ids)
            require(len(module["distributionAdapters"])
                    == len(semantic_module["distributions"]),
                    f"distribution adapter coverage changed: {module_id}")
            adapter_by_id = {
                row["distributionId"]: row
                for row in module["distributionAdapters"]
            }
            require(len(adapter_by_id) == len(module["distributionAdapters"]),
                    f"distribution adapter identity collision: {module_id}")

            require(len(module["properties"]) == len(semantic_module["properties"]),
                    f"property coverage changed: {module_id}")
            for semantic_property, prop in zip(
                semantic_module["properties"], module["properties"]
            ):
                require(prop["propertyId"] == semantic_property["propertyId"]
                        and prop["propertyId"] not in all_property_ids,
                        f"property identity changed: {prop['propertyId']}")
                all_property_ids.add(prop["propertyId"])
                require(prop["propertyPath"] == semantic_property["propertyPath"],
                        f"property path changed: {prop['propertyId']}")
                for literal_id in prop["payloadLiteralIds"]:
                    require(literal_id in module_literal_ids,
                            f"property literal escaped module: {prop['propertyId']}")
                for distribution_id in prop["payloadDistributionIds"]:
                    require(distribution_id in module_distribution_ids,
                            f"property distribution escaped module: {prop['propertyId']}")
                property_literal_refs.update(prop["payloadLiteralIds"])
                require(prop["decision"] in ALLOWED_DECISIONS,
                        f"unknown property decision: {prop['propertyId']}")
                expected_irrelevant = prop["propertyPath"] in IRRELEVANT_PROPERTIES
                expected_custom = any(
                    str(row.get("exactSourceClass") or "").casefold()
                    == "efdistributionvectormultiplyparticleparameter"
                    for row in semantic_module["distributions"]
                    if row["topLevelPropertyPath"] == prop["propertyPath"]
                )
                expected_semantic_ids = [
                    row["distributionId"]
                    for row in semantic_module["distributions"]
                    if row["topLevelPropertyPath"] == prop["propertyPath"]
                ]
                require(prop["semanticDistributionIds"] == expected_semantic_ids,
                        f"property semantic distribution join changed: {prop['propertyId']}")
                expected_blockers_set: set[str] = set()
                for semantic_distribution in semantic_module["distributions"]:
                    if semantic_distribution["distributionId"] \
                            not in expected_semantic_ids:
                        continue
                    field_evidence = semantic_distribution.get("fieldEvidence") or {}
                    default_dependent = bool(field_evidence.get("defaultDependent"))
                    resolved_by_current_cdo = (
                        default_dependent
                        and policy_id == CURRENT_ENGINE_CDO_POLICY
                        and exact_class == "particlemodulespawn"
                        and str(semantic_distribution.get("propertyPath") or "").casefold()
                        == "ratescale"
                    )
                    if default_dependent and not resolved_by_current_cdo:
                        expected_blockers_set.update(
                            str(blocker) for blocker
                            in semantic_distribution.get("executionBlockers", [])
                        )
                        expected_blockers_set.add(DEFAULT_POLICY_BLOCKER)
                    if str(semantic_distribution.get(
                        "exactSourceClass"
                    ) or "").casefold() == \
                            "efdistributionvectormultiplyparticleparameter":
                        expected_blockers_set.add(CUSTOM_DISTRIBUTION_BLOCKER)
                expected_blockers = sorted(expected_blockers_set)
                expected_decision = (
                    "VERIFIED_IRRELEVANT" if expected_irrelevant else
                    "BLOCKED" if expected_blockers or expected_custom
                    else "READY_FOR_HANDLER"
                )
                require(prop["decision"] == expected_decision
                        and prop["blockers"] == (
                            [] if expected_irrelevant else expected_blockers
                        ),
                        f"property decision changed: {prop['propertyId']}")
                property_decisions[prop["decision"]] += 1

            literal_by_path = {
                str(row["propertyPath"]).casefold(): row["literalId"]
                for row in typed["literals"]
            }
            require(len(module["primitiveLeaves"])
                    == len(semantic_module["primitiveLeaves"]),
                    f"leaf coverage changed: {module_id}")
            property_by_path = {
                row["propertyPath"]: row for row in module["properties"]
            }
            for semantic_leaf, leaf in zip(
                semantic_module["primitiveLeaves"], module["primitiveLeaves"]
            ):
                require(leaf["leafId"] == semantic_leaf["leafId"]
                        and leaf["leafId"] not in all_leaf_ids,
                        f"leaf identity changed: {leaf['leafId']}")
                all_leaf_ids.add(leaf["leafId"])
                require(leaf["payloadLiteralId"]
                        == literal_by_path[leaf["propertyPath"]],
                        f"leaf literal binding changed: {leaf['leafId']}")
                parent = property_by_path[leaf["topLevelPropertyPath"]]
                require(leaf["decision"] == parent["decision"]
                        and leaf["blockers"] == parent["blockers"],
                        f"leaf decision diverged: {leaf['leafId']}")
                leaf_literal_refs.add(leaf["payloadLiteralId"])
                leaf_decisions[leaf["decision"]] += 1

            require(len(module["distributionAdapters"])
                    == len(semantic_module["distributions"]),
                    f"distribution adapter coverage changed: {module_id}")
            adapter_by_id = {
                row["distributionId"]: row for row in module["distributionAdapters"]
            }
            require(len(adapter_by_id) == len(module["distributionAdapters"]),
                    f"distribution adapter identity collision: {module_id}")
            typed_descriptor_by_id = {
                item["payloadDistributionId"]: item["descriptor"]
                for item in typed["distributions"]
            }
            for semantic_distribution in semantic_module["distributions"]:
                row = adapter_by_id.get(semantic_distribution["distributionId"])
                require(row is not None and row["distributionId"] not in all_distribution_ids,
                        f"distribution identity changed: {semantic_distribution['distributionId']}")
                all_distribution_ids.add(row["distributionId"])
                require(row["payloadDistributionId"] in module_distribution_ids,
                        f"distribution payload escaped module: {row['distributionId']}")
                distribution_payload_refs.add(row["payloadDistributionId"])
                field_evidence = semantic_distribution["fieldEvidence"]
                expected_default_dependent = bool(
                    field_evidence.get("defaultDependent")
                )
                expected_semantic_blockers = sorted({
                    str(blocker) for blocker in semantic_distribution.get(
                        "executionBlockers", []
                    )
                })
                require(
                    row["propertyPath"]
                    == str(semantic_distribution["propertyPath"]).casefold()
                    and row["topLevelPropertyPath"]
                    == str(semantic_distribution["topLevelPropertyPath"]).casefold()
                    and row["defaultDependent"] is expected_default_dependent
                    and row["semanticClosureExecutionBlockers"]
                    == expected_semantic_blockers,
                    f"semantic closure distribution evidence changed: {row['distributionId']}",
                )
                if row.get("legacyOccurrenceId"):
                    require(not expected_default_dependent
                            and row["sourceNumericPayloadByteCount"] is None
                            and row["sourceNumericPayloadDefinition"] is None
                            and row["defaultResolution"] is None,
                            f"local distribution default seam changed: {row['distributionId']}")
                    legacy_occurrence = local_occurrences.get(
                        str(row["legacyOccurrenceId"])
                    )
                    definition = local_definitions.get(str(row["definitionId"]))
                    require(legacy_occurrence is not None and definition is not None
                            and legacy_occurrence["definitionId"] == row["definitionId"],
                            f"local definition binding changed: {row['distributionId']}")
                    require(row["currentRevisionFields"]
                            == expected_local_fields(definition),
                            f"local current/default fields changed: {row['distributionId']}")
                    branch, value = local_oracle(row)
                    samples = row["numericOracleSamples"]
                    if str(row["exactSourceClass"]).casefold() == \
                            "distributionfloatconstantcurve":
                        desc = next(item["descriptor"] for item in typed["distributions"]
                                    if item["payloadDistributionId"]
                                    == row["payloadDistributionId"])
                        require(len(samples) == len(SAMPLES),
                                f"curve oracle count changed: {row['distributionId']}")
                        for stored, (time, random_values) in zip(samples, SAMPLES):
                            require(close_enough(stored["value"],
                                                 evaluate(desc, time, random_values)),
                                    f"curve oracle changed: {row['distributionId']}")
                    elif row["decision"] == "BLOCKED":
                        require(CUSTOM_DISTRIBUTION_BLOCKER in row["blockers"],
                                f"custom evaluator blocker lost: {row['distributionId']}")
                    else:
                        parameter_name = {
                            str(field["fieldPath"]).casefold(): field.get("value")
                            for field in row["currentRevisionFields"]
                        }.get("parametername")
                        expected_type = str(
                            (definition.get("semanticCoverage") or {}).get(
                                "expectedOverrideType"
                            ) or ""
                        ).casefold()
                        expected_parameter = cue_parameters.get(
                            str(legacy_occurrence["sourceCueId"]), {}
                        ).get(str(parameter_name).casefold()) \
                            if parameter_name not in (None, "", "none") else None
                        if expected_parameter is not None \
                                and expected_parameter["kind"] != expected_type:
                            expected_parameter = None
                        require(samples[0]["parameterInput"] == expected_parameter,
                                f"local ActionCue input binding changed: {row['distributionId']}")
                        require(samples[0]["branch"] == branch
                                and close_enough(samples[0]["value"], value),
                                f"local parameter oracle changed: {row['distributionId']}")
                else:
                    desc = typed_descriptor_by_id[row["payloadDistributionId"]]
                    reconstructed_fields = list(
                        field_evidence["reconstructedFieldNames"]
                    )
                    require(
                        row["exactSourceClass"] == str(
                            semantic_distribution.get("exactSourceClass") or ""
                        ).casefold()
                        and row["fieldProvenance"]["rawFieldSourceFidelity"]
                        == field_evidence["rawFieldSourceFidelity"]
                        and row["fieldProvenance"]["reconstructedFieldNames"]
                        == reconstructed_fields
                        and row["fieldProvenance"]["sourceExactUpgradeAllowed"]
                        is False,
                        f"inline field provenance changed: {row['distributionId']}",
                    )
                    if expected_default_dependent:
                        default_dependent_count += 1
                        verify_null_raw_distribution_projection(
                            semantic_distribution, desc
                        )
                        if (exact_class == "particlemodulespawn"
                                and row["propertyPath"] == "ratescale"):
                            verify_spawn_ratescale_raw_distribution_float(
                                semantic_distribution, desc
                            )
                        require(row["sourceNumericPayloadByteCount"] == 0,
                                f"null numeric payload bytes changed: {row['distributionId']}")
                        require(row["sourceNumericPayloadDefinition"]
                                == "LOOKUP_TABLE_OR_KEY_VALUE_BYTES",
                                f"null numeric payload definition changed: {row['distributionId']}")
                        use_current_cdo = (
                            policy_id == CURRENT_ENGINE_CDO_POLICY
                            and exact_class == "particlemodulespawn"
                            and row["propertyPath"] == "ratescale"
                        )
                        if use_current_cdo:
                            expected_descriptor, expected_evidence = (
                                current_engine_spawn_ratescale_reconstruction(defaults)
                            )
                            expected_samples = [
                                {
                                    "time": time,
                                    "randomUnits": list(random_values),
                                    "value": evaluate(
                                        expected_descriptor, time, random_values
                                    ),
                                }
                                for time, random_values in SAMPLES
                            ]
                            require(
                                row["decision"] == "READY_FOR_HANDLER"
                                and row["blockers"] == []
                                and row["reconstructedDescriptor"]
                                == expected_descriptor
                                and row["defaultResolution"] == expected_evidence
                                and row["numericOracleSamples"] == expected_samples
                                and row["fieldProvenance"]["reconstructionBasis"]
                                == CURRENT_ENGINE_CDO_POLICY,
                                "current Engine CDO reconstruction evidence changed",
                            )
                            default_dependent_reconstructed_count += 1
                        else:
                            expected_blockers = sorted({
                                *expected_semantic_blockers,
                                DEFAULT_POLICY_BLOCKER,
                            })
                            require(
                                row["decision"] == "BLOCKED"
                                and row["blockers"] == expected_blockers
                                and row["numericOracleSamples"] == []
                                and row["reconstructedDescriptor"] is None
                                and row["defaultResolution"] == {
                                    "policyId": policy_id,
                                    "status": "BLOCKED",
                                    "provenanceTier": (
                                        "SOURCE_NULL_RAW_DISTRIBUTION"
                                    ),
                                    "sourceExact": False,
                                    "sourceExactUpgradeAllowed": False,
                                    "numericPayloadByteCount": 0,
                                    "numericPayloadDefinition": (
                                        "LOOKUP_TABLE_OR_KEY_VALUE_BYTES"
                                    ),
                                }
                                and row["fieldProvenance"]["reconstructionBasis"]
                                == "NULL_RAW_DISTRIBUTION_FAIL_CLOSED",
                                "default-dependent zero fabrication was admitted",
                            )
                    else:
                        require(row["sourceNumericPayloadByteCount"] is None
                                and row["sourceNumericPayloadDefinition"] is None
                                and row["defaultResolution"] is None,
                                f"non-default distribution gained a default seam: {row['distributionId']}")
                        expected_basis = (
                            "CURRENT_UE3_RAW_DISTRIBUTION_DEFAULT_AND_PAYLOAD_SHAPE"
                            if reconstructed_fields else "SOURCE_TAGGED_FIELDS"
                        )
                        require(
                            row["decision"] == "READY_FOR_HANDLER"
                            and row["blockers"] == []
                            and row["reconstructedDescriptor"] is None
                            and row["fieldProvenance"]["reconstructionBasis"]
                            == expected_basis
                            and len(row["numericOracleSamples"]) == len(SAMPLES),
                            f"inline oracle policy changed: {row['distributionId']}",
                        )
                        for stored, (time, random_values) in zip(
                            row["numericOracleSamples"], SAMPLES
                        ):
                            require(close_enough(stored["value"], evaluate(
                                desc, time, random_values
                            )), f"inline oracle changed: {row['distributionId']}")
                distribution_decisions[row["decision"]] += 1

            if (occurrence_index == EXPECTED_SPAWN_PARAMETER_RATE_OCCURRENCE
                    and exact_class == "particlemodulespawn"):
                parameter_rate = next(
                    row for row in module["distributionAdapters"]
                    if row["propertyPath"] == "rate"
                )
                require(
                    parameter_rate["exactSourceClass"]
                    == "distributionfloatparticleparameter"
                    and parameter_rate["decision"] == "READY_FOR_HANDLER"
                    and len(parameter_rate["numericOracleSamples"]) == 1
                    and parameter_rate["numericOracleSamples"][0]["branch"]
                    == "PARAMETER_INPUT"
                    and parameter_rate["numericOracleSamples"][0][
                        "parameterInput"
                    ] == {
                        "name": "Spawn", "kind": "scalar", "value": 0.0,
                        "sourceIndex": 0, "sourceValueByteOffset": 494,
                    }
                    and parameter_rate["numericOracleSamples"][0]["value"]
                    == [0.0],
                    "Full35 occurrence #34 Spawn parameter Rate branch changed",
                )

            if module["seed"] is not None:
                seed_count += 1
                require(module["seed"]["decision"] == "READY_FOR_HANDLER",
                        f"seed decision changed: {module_id}")
                expected_seed_policy = {
                    name: value for name, value in defaults["randomSeedPolicy"].items()
                    if name not in {"provenance", "sourceEraIdentityPinned", "classCount"}
                }
                require(module["seed"]["policy"] == expected_seed_policy
                        and module["seed"]["currentCdoEvidenceKey"]
                        == "seed::" + exact_class
                        and module["seed"]["currentCdoEvidenceKey"]
                        in defaults["classDefaultObjects"],
                        f"seed CDO binding changed: {module_id}")
                require(module["seed"]["policy"]["randomlySelectSeedArray"] is False
                        and module["seed"]["policy"]["resetSeedOnEmitterLooping"] is True,
                        f"seed policy changed: {module_id}")
            default_count += len(module["implicitDefaults"])
            require(all(row["decision"] in {
                "READY_FOR_HANDLER", "VERIFIED_IRRELEVANT"
            } for row in module["implicitDefaults"]),
                    f"implicit default decision changed: {module_id}")
            require(module["nativeTail"]["decision"] == "VERIFIED_IRRELEVANT",
                    f"native tail decision changed: {module_id}")
            if semantic_module["sourceDocument"] == "externalModuleClosure":
                external_native_count += 1

            expected_module_blockers = sorted({
                *([CUSTOM_HANDLER_BLOCKER] if custom_handler(exact_class) else []),
                *(blocker for row in module["distributionAdapters"]
                  for blocker in row["blockers"]),
            })
            require(module["blockers"] == expected_module_blockers
                    and module["decision"] == (
                        "BLOCKED" if expected_module_blockers
                        else "READY_FOR_HANDLER"
                    ), f"module decision changed: {module_id}")
            module_decisions[module["decision"]] += 1

    require(all_payload_literals == property_literal_refs,
            "typed literal property consumption changed")
    require(leaf_literal_refs.issubset(all_payload_literals),
            "leaf literal consumption changed")
    require(all_payload_distributions == distribution_payload_refs,
            "typed distribution consumption changed")

    catalog = {row["exactSourceClass"]: row for row in receipt["handlerCapabilities"]}
    require(len(catalog) == len(class_counts) == EXPECTED["exactClassCount"],
            "handler catalog class coverage changed")
    for name, count in class_counts.items():
        row = catalog.get(name)
        require(row is not None and row["occurrenceCount"] == count,
                f"handler catalog count changed: {name}")
        require(row["decision"] == (
            "BLOCKED" if custom_handler(name) else "READY_FOR_HANDLER"
        ) and row["normalizedAliasAllowed"] is False,
                f"handler catalog decision changed: {name}")

    local_rows = receipt["localDistributionAdapters"]
    nested_local = [
        row for occurrence in receipt_occurrences for module in occurrence["modules"]
        for row in module["distributionAdapters"] if row.get("legacyOccurrenceId")
    ]
    require(len(local_rows) == len(nested_local) == EXPECTED["localDistributionOccurrenceCount"],
            "local adapter denominator changed")
    require({row["legacyOccurrenceId"]: canonical_sha256(row) for row in local_rows}
            == {row["legacyOccurrenceId"]: canonical_sha256(row) for row in nested_local},
            "local adapter top-level join changed")

    point = receipt["pointLightAdapter"]
    component_definitions = local_closure.get("componentDefinitions", [])
    component_occurrences = local_closure.get("componentOccurrences", [])
    require(len(component_definitions) == len(component_occurrences) == 1,
            "PointLight source denominator changed")
    point_definition = component_definitions[0]
    require(point["definitionId"] == point_definition["definitionId"]
            and point["occurrenceId"] == component_occurrences[0]["occurrenceId"],
            "PointLight identity binding changed")
    expected_point_fields = {}
    for name, resolution in (point_definition.get("semanticCoverage") or {}).get(
        "resolvedFields", {}
    ).items():
        selected = resolution.get("selected") if isinstance(resolution, dict) else None
        require(isinstance(selected, dict), f"PointLight source field unresolved: {name}")
        tagged = selected.get("value")
        expected_point_fields[str(name).casefold()] = (
            tagged.get("value") if isinstance(tagged, dict) else tagged
        )
    require({row["fieldPath"]: row["value"] for row in point["fields"]}
            == expected_point_fields, "PointLight field evidence changed")
    require(point["handlerDecision"] == "READY_FOR_HANDLER"
            and point["sourceEraDefaultIdentityPinned"] is False,
            "PointLight evidence boundary changed")
    require({row["fieldPath"] for row in point["fields"]}
            >= {"brightness", "radius", "falloffexponent", "lightcolor"},
            "PointLight executable fields changed")
    require(all(row["decision"] == "VERIFIED_IRRELEVANT"
                for row in point["fields"]
                if row["fieldPath"] in {"lightguid", "lightmapguid"}),
            "PointLight GUID irrelevance changed")

    measured = {
        "occurrenceCount": len(receipt_occurrences),
        "selectedLodFieldCount": selected_lod_count,
        "moduleCount": len(all_module_ids),
        "propertyCount": len(all_property_ids),
        "primitiveLeafCount": len(all_leaf_ids),
        "distributionCount": len(all_distribution_ids),
        "inlineDistributionCount": len(all_distribution_ids) - len(local_rows),
        "localDistributionDefinitionCount": len({row["definitionId"] for row in local_rows}),
        "localDistributionOccurrenceCount": len(local_rows),
        "pointLightOccurrenceCount": 1,
        "nativeTailCount": len(all_module_ids),
        "externalNativeTailOccurrenceCount": external_native_count,
        "seedCount": seed_count,
        "implicitDefaultCount": default_count,
        "transportLiteralCount": len(all_payload_literals) - len(all_leaf_ids),
        "exactClassCount": len(class_counts),
        "customOrSeededClassOccurrenceCount": sum(
            count for name, count in class_counts.items() if custom_handler(name)
        ),
    }
    require(measured == EXPECTED, f"independent denominator changed: {measured}")
    require(dict(sorted(module_decisions.items()))
            == receipt["summary"]["moduleDecisionCounts"],
            "module decision summary changed")
    require(dict(sorted(property_decisions.items()))
            == receipt["summary"]["propertyDecisionCounts"],
            "property decision summary changed")
    require(dict(sorted(leaf_decisions.items()))
            == receipt["summary"]["primitiveLeafDecisionCounts"],
            "leaf decision summary changed")
    require(dict(sorted(distribution_decisions.items()))
            == receipt["summary"]["distributionDecisionCounts"],
            "distribution decision summary changed")
    expected_reconstructed = (
        EXPECTED_SPAWN_RATESCALE_DEFAULT_DEPENDENT_COUNT
        if policy_id == CURRENT_ENGINE_CDO_POLICY else 0
    )
    require(
        default_dependent_count == EXPECTED_DEFAULT_DEPENDENT_COUNT
        and default_dependent_reconstructed_count == expected_reconstructed
        and receipt["summary"]["defaultDependentDistributionCount"]
        == EXPECTED_DEFAULT_DEPENDENT_COUNT
        and receipt["summary"]["defaultDependentReconstructedCount"]
        == expected_reconstructed
        and receipt["summary"]["defaultDependentBlockedCount"]
        == EXPECTED_DEFAULT_DEPENDENT_COUNT - expected_reconstructed,
        "default-dependent decision summary changed",
    )
    require(receipt["summary"]["typedPayloadLiteralCount"]
            == len(all_payload_literals), "typed literal summary changed")
    require(receipt["summary"]["typedPayloadDistributionCount"]
            == len(all_payload_distributions), "typed distribution summary changed")
    require(receipt["summary"]["allRowsClassifiedAndBound"] is True
            and receipt["summary"]["unclassifiedRowCount"] == 0,
            "all-row classification changed")
    require(receipt["summary"]["allRowsConsumedOrIrrelevant"] is False,
            "blocked custom rows were reported as consumed")
    expected_union = sorted({
        PRODUCT_OWNER_BLOCKER,
        *(blocker for row in receipt["handlerCapabilities"] for blocker in row["blockers"]),
        *(blocker for row in local_rows for blocker in row["blockers"]),
        *(blocker for occurrence in receipt_occurrences
          for module in occurrence["modules"] for blocker in module["blockers"]),
    })
    require(receipt["blockerUnion"] == expected_union
            and receipt["productAdmission"]["blockers"] == expected_union,
            "blocker union propagation changed")

    if release_root is not None:
        verify_deep_native(receipt, inputs["externalModuleClosure"], release_root)
    return {
        "modules": len(all_module_ids),
        "readyModules": module_decisions["READY_FOR_HANDLER"],
        "blockedModules": module_decisions["BLOCKED"],
        "distributions": len(all_distribution_ids),
        "readyDistributions": distribution_decisions["READY_FOR_HANDLER"],
        "blockedDistributions": distribution_decisions["BLOCKED"],
    }


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--receipt", type=Path,
        default=root / "Data/Effects/Imported/Artist/Candidates/"
        "skill.31470.source-execution-semantics.receipt.json",
    )
    parser.add_argument(
        "--release-root", type=Path,
        default=Path(r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC"),
    )
    parser.add_argument("--shallow", action="store_true")
    args = parser.parse_args()
    receipt = load_strict_json_object(args.receipt)
    inputs = {
        name: load_strict_json_object(root / row["path"])
        for name, row in receipt.get("inputs", {}).items()
    }
    result = verify_receipt(
        receipt, root=root, inputs=inputs,
        release_root=None if args.shallow else args.release_root,
    )
    print(
        "Artist F source execution independent oracle: "
        f"modules={result['modules']} ready={result['readyModules']} "
        f"blocked={result['blockedModules']} distributions={result['distributions']} "
        f"distributionBlocked={result['blockedDistributions']} product=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"Artist F source execution oracle failed: {error}", file=sys.stderr)
        raise SystemExit(1)
