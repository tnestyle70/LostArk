#!/usr/bin/env python3
"""Build the fail-closed Artist F source execution adapter receipt.

This artifact is the boundary between decoded Cascade evidence and the runtime
compiler.  It materializes every source row as typed input, records current
revision/default reconstructions separately from source-exact evidence, and
keeps exact custom/seeded class handlers blocked until a numeric oracle exists.
It never grants Product admission.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import re
import struct
import sys
from collections import Counter, defaultdict
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

TRACKED_INPUTS = {
    "semanticClosure": (
        "Data/Effects/Imported/Artist/Candidates/"
        "skill.31470.source-semantic-closure.json"
    ),
    "candidate": (
        "Data/Effects/Imported/Artist/Candidates/"
        "effect.artist.skill.31470.native-v14.source-contract-candidate.effect.json"
    ),
    "candidateReceipt": (
        "Data/Effects/Imported/Artist/Candidates/"
        "skill.31470.native-v14.source-contract-candidate.receipt.json"
    ),
    "localReferenceClosure": (
        "Data/Effects/Imported/Artist/Graphs/"
        "skill.31470.local-reference-closure.json"
    ),
    "externalModuleClosure": (
        "Data/Effects/Imported/Artist/Modules/"
        "skill.31470.external-module-closure.json"
    ),
    "actionCueRecipe": (
        "Data/Effects/Imported/Artist/skill.31470.action-cue-recipe.json"
    ),
}

DESCRIPTOR_FIELDS = (
    "propertyPath",
    "sourceClass",
    "sourceObjectPath",
    "componentCount",
    "operation",
    "randomLockAxes",
    "lookupTableChunkSize",
    "lookupTableNumElements",
    "lookupTableTimeScale",
    "lookupTableStartTime",
    "defaultMinimum",
    "defaultMaximum",
    "lookupTable",
    "keys",
    "referenceId",
    "occurrenceId",
    "payloadStatus",
    "fidelity",
)

IRRELEVANT_PROPERTIES = {
    "b3ddrawmode": "ue3.editor-only-3d-draw-mode.v1",
    "lodvalidity": "artist-f.first-lod-compiled-mask.v1",
}

SAMPLE_INPUTS = (
    (0.0, (0.0, 0.25, 0.5, 0.75)),
    (0.25, (0.25, 0.5, 0.75, 1.0)),
    (1.0, (1.0, 0.75, 0.5, 0.25)),
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


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


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def canonical_text_sha256(path: Path) -> str:
    text = path.read_text(encoding="utf-8-sig")
    return hashlib.sha256(text.replace("\r\n", "\n").encode("utf-8")).hexdigest()


def sanitized_id(value: str) -> str:
    result = re.sub(r"[^a-z0-9]+", ".", value.casefold()).strip(".")
    require(result != "", f"empty stable ID component: {value}")
    return result


def legacy_normalized_class(value: str) -> str:
    result = value.casefold()
    if result.startswith("efparticlemodule"):
        result = "particlemodule" + result[len("efparticlemodule") :]
    if result.endswith("_seeded"):
        result = result[: -len("_seeded")]
    return result


def exact_handler_needs_oracle(value: str) -> bool:
    return legacy_normalized_class(value) != value.casefold()


def finite_number(value: Any) -> float:
    require(
        isinstance(value, (int, float))
        and not isinstance(value, bool)
        and math.isfinite(float(value)),
        f"non-finite numeric payload: {value}",
    )
    return float(value)


def vector4(value: Any) -> list[float]:
    if isinstance(value, dict):
        result = [value.get(axis, 0.0) for axis in ("x", "y", "z", "w")]
    elif isinstance(value, list):
        result = list(value[:4])
    elif isinstance(value, (int, float)) and not isinstance(value, bool):
        result = [value, value, value, value]
    else:
        result = [0.0, 0.0, 0.0, 0.0]
    while len(result) < 4:
        result.append(0.0)
    return [finite_number(row) for row in result]


def lerp(a: list[float], b: list[float], ratio: float) -> list[float]:
    return [a[index] + (b[index] - a[index]) * ratio for index in range(4)]


def hermite(
    start: list[float],
    start_tangent: list[float],
    end: list[float],
    end_tangent: list[float],
    ratio: float,
    duration: float,
) -> list[float]:
    ratio2 = ratio * ratio
    ratio3 = ratio2 * ratio
    h00 = 2.0 * ratio3 - 3.0 * ratio2 + 1.0
    h10 = ratio3 - 2.0 * ratio2 + ratio
    h01 = -2.0 * ratio3 + 3.0 * ratio2
    h11 = ratio3 - ratio2
    return [
        start[index] * h00
        + start_tangent[index] * h10 * duration
        + end[index] * h01
        + end_tangent[index] * h11 * duration
        for index in range(4)
    ]


def select_range(
    minimum: list[float],
    maximum: list[float],
    operation: int,
    random_units: Iterable[float],
) -> list[float]:
    random_value = [min(1.0, max(0.0, float(row))) for row in random_units]
    if operation == 2:
        return [
            minimum[index]
            + (maximum[index] - minimum[index]) * random_value[index]
            for index in range(4)
        ]
    if operation == 3:
        return list(minimum if random_value[0] < 0.5 else maximum)
    return list(minimum)


def evaluate_descriptor(
    descriptor: dict[str, Any], time: float, random_units: Iterable[float]
) -> list[float]:
    components = int(descriptor["componentCount"])
    operation = int(descriptor["operation"])
    lookup = [finite_number(row) for row in descriptor["lookupTable"]]
    if lookup:
        chunk = int(descriptor["lookupTableChunkSize"])
        if chunk == 0:
            chunk = components * (2 if operation >= 2 else 1)
        payload_count = len(lookup) - 2
        require(chunk > 0 and payload_count >= chunk and payload_count % chunk == 0,
                "invalid cooked lookup payload")
        entry_count = payload_count // chunk
        scale = finite_number(descriptor["lookupTableTimeScale"])
        start_time = finite_number(descriptor["lookupTableStartTime"])
        lookup_time = (time - start_time) * scale if scale > 0.0 else 0.0
        clamped = min(float(entry_count - 1), max(0.0, lookup_time))
        start = int(math.floor(clamped))
        end = min(start + 1, entry_count - 1)
        ratio = clamped - start

        def table_value(entry: int, maximum: bool) -> list[float]:
            offset = 2 + entry * chunk
            if maximum and chunk >= components * 2:
                offset += components
            result = [0.0, 0.0, 0.0, 0.0]
            for index in range(components):
                result[index] = lookup[offset + index]
            return result

        minimum = lerp(table_value(start, False), table_value(end, False), ratio)
        maximum = lerp(table_value(start, True), table_value(end, True), ratio)
        return select_range(minimum, maximum, operation, random_units)

    keys = descriptor["keys"]
    if keys:
        first = keys[0]
        last = keys[-1]
        if time <= finite_number(first["time"]):
            return select_range(
                vector4(first["minimum"]), vector4(first["maximum"]),
                operation, random_units,
            )
        if time >= finite_number(last["time"]):
            return select_range(
                vector4(last["minimum"]), vector4(last["maximum"]),
                operation, random_units,
            )
        for index in range(len(keys) - 1):
            begin = keys[index]
            end = keys[index + 1]
            end_time = finite_number(end["time"])
            if time > end_time:
                continue
            begin_time = finite_number(begin["time"])
            duration = end_time - begin_time
            ratio = 0.0 if duration <= 0.0 else (time - begin_time) / duration
            if str(begin["interpolation"]).casefold() == "constant":
                minimum = vector4(begin["minimum"])
                maximum = vector4(begin["maximum"])
            elif str(begin["interpolation"]).casefold() == "cubic":
                minimum = hermite(
                    vector4(begin["minimum"]),
                    vector4(begin["leaveTangentMinimum"]),
                    vector4(end["minimum"]),
                    vector4(end["arriveTangentMinimum"]), ratio, duration,
                )
                maximum = hermite(
                    vector4(begin["maximum"]),
                    vector4(begin["leaveTangentMaximum"]),
                    vector4(end["maximum"]),
                    vector4(end["arriveTangentMaximum"]), ratio, duration,
                )
            else:
                minimum = lerp(
                    vector4(begin["minimum"]), vector4(end["minimum"]), ratio
                )
                maximum = lerp(
                    vector4(begin["maximum"]), vector4(end["maximum"]), ratio
                )
            return select_range(minimum, maximum, operation, random_units)
    return select_range(
        vector4(descriptor["defaultMinimum"]),
        vector4(descriptor["defaultMaximum"]),
        operation,
        random_units,
    )


def descriptor_payload(value: dict[str, Any]) -> dict[str, Any]:
    result = {name: copy.deepcopy(value.get(name)) for name in DESCRIPTOR_FIELDS}
    require(1 <= int(result["componentCount"]) <= 4, "invalid component count")
    require(0 <= int(result["operation"]) <= 3, "invalid distribution operation")
    for name in ("defaultMinimum", "defaultMaximum"):
        result[name] = vector4(result[name])
    return result


def numeric_samples(descriptor: dict[str, Any]) -> list[dict[str, Any]]:
    return [
        {
            "time": time,
            "randomUnits": list(random_units),
            "value": evaluate_descriptor(descriptor, time, random_units),
        }
        for time, random_units in SAMPLE_INPUTS
    ]


def is_zero_vector(value: Any) -> bool:
    return vector4(value) == [0.0, 0.0, 0.0, 0.0]


def require_null_raw_distribution_projection(
    semantic_distribution: dict[str, Any], descriptor: dict[str, Any],
) -> None:
    evidence = semantic_distribution.get("fieldEvidence") or {}
    require(evidence.get("defaultDependent") is True,
            "null RawDistribution lost default-dependent evidence")
    raw_fields = evidence.get("rawFieldNames") or []
    require(raw_fields and raw_fields[0] == "distribution"
            and set(raw_fields).issubset({
                "distribution", "lookuptabletimescale", "lookuptablestarttime",
            }) and evidence.get("hasLookupPayload") is False
            and evidence.get("explicitOperation") is None,
            "null RawDistribution source field shape changed")
    require(DEFAULT_DEPENDENT_BLOCKER in semantic_distribution.get(
        "executionBlockers", []
    ), "null RawDistribution class-default blocker is missing")
    require(
        str(descriptor.get("sourceClass") or "") == ""
        and str(descriptor.get("sourceObjectPath") or "") == ""
        and 1 <= int(descriptor.get("componentCount", 0)) <= 4
        and int(descriptor.get("operation", -1)) == 1
        and int(descriptor.get("lookupTableChunkSize", -1)) == 0
        and int(descriptor.get("lookupTableNumElements", -1)) == 0
        and math.isfinite(float(descriptor.get("lookupTableTimeScale", math.nan)))
        and math.isfinite(float(descriptor.get("lookupTableStartTime", math.nan)))
        and is_zero_vector(descriptor.get("defaultMinimum"))
        and is_zero_vector(descriptor.get("defaultMaximum"))
        and descriptor.get("lookupTable") == []
        and descriptor.get("keys") == []
        and str(descriptor.get("referenceId") or "") == ""
        and str(descriptor.get("occurrenceId") or "") == "",
        "null RawDistribution was not the zero-byte numeric projection",
    )


def require_spawn_ratescale_raw_distribution_float(
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
    spawn_rate_default_occurrences: list[int] = []
    explicit_zero_rate_occurrences: list[int] = []
    explicit_nonzero_rates: dict[int, float] = {}
    parameter_rate_occurrences: list[int] = []
    spawn_module_count = 0
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
            candidate_by_path: dict[str, list[dict[str, Any]]] = defaultdict(list)
            for row in candidate_distributions:
                candidate_by_path[str(row.get("propertyPath") or "").casefold()].append(row)
            exact_class = str(semantic_module.get("exactSourceClass") or "").casefold()
            if exact_class == "particlemodulespawn":
                spawn_module_count += 1
            for semantic_distribution in semantic_distributions:
                path = str(semantic_distribution.get("propertyPath") or "").casefold()
                candidates = candidate_by_path.get(path) or []
                require(candidates, "Full35 default gate payload path changed")
                descriptor = descriptor_payload(candidates.pop(0))
                default_dependent = bool(
                    (semantic_distribution.get("fieldEvidence") or {}).get(
                        "defaultDependent"
                    )
                )
                if default_dependent:
                    default_count += 1
                    require_null_raw_distribution_projection(
                        semantic_distribution, descriptor
                    )
                if exact_class != "particlemodulespawn":
                    continue
                require(path in {"rate", "ratescale"},
                        "ParticleModuleSpawn distribution set changed")
                if default_dependent:
                    spawn_default_count += 1
                    if path == "ratescale":
                        require_spawn_ratescale_raw_distribution_float(
                            semantic_distribution, descriptor
                        )
                        spawn_ratescale_default_count += 1
                    else:
                        spawn_rate_default_occurrences.append(occurrence_index)
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
                    parameter_rate_occurrences.append(occurrence_index)
                    continue
                table = [finite_number(row) for row in descriptor.get(
                    "lookupTable", []
                )]
                require(
                    str(descriptor.get("sourceClass") or "") == ""
                    and int(descriptor.get("operation", -1)) == 1
                    and int(descriptor.get("lookupTableChunkSize", -1)) == 1
                    and int(descriptor.get("lookupTableNumElements", -1)) == 1
                    and len(table) == 4
                    and len(set(table)) == 1,
                    "Full35 explicit Spawn Rate shape changed",
                )
                value = table[0]
                if value == 0.0:
                    explicit_zero_rate_occurrences.append(occurrence_index)
                else:
                    explicit_nonzero_rates[occurrence_index] = value
            require(not any(candidate_by_path.values()),
                    "Full35 default gate left unmatched payload distributions")
    require(spawn_module_count == EXPECTED["occurrenceCount"],
            "ParticleModuleSpawn occurrence denominator changed")
    require(default_count == EXPECTED_DEFAULT_DEPENDENT_COUNT,
            f"default-dependent denominator changed: {default_count}")
    require(spawn_default_count == EXPECTED_SPAWN_DEFAULT_DEPENDENT_COUNT,
            f"Spawn default-dependent denominator changed: {spawn_default_count}")
    require(spawn_ratescale_default_count
            == EXPECTED_SPAWN_RATESCALE_DEFAULT_DEPENDENT_COUNT,
            "Spawn RateScale default-dependent denominator changed")
    require(spawn_rate_default_occurrences
            == [EXPECTED_SPAWN_RATE_DEFAULT_DEPENDENT_OCCURRENCE],
            "Spawn Rate default-dependent occurrence changed")
    require(len(explicit_zero_rate_occurrences) == 29,
            "explicit zero Spawn Rate denominator changed")
    require(explicit_nonzero_rates == EXPECTED_NONZERO_SPAWN_RATES,
            f"explicit nonzero Spawn Rates changed: {explicit_nonzero_rates}")
    require(parameter_rate_occurrences
            == [EXPECTED_SPAWN_PARAMETER_RATE_OCCURRENCE],
            "Spawn parameter Rate occurrence changed")
    return {
        "defaultDependentCount": default_count,
        "spawnDefaultDependentCount": spawn_default_count,
        "spawnRateScaleDefaultDependentCount": spawn_ratescale_default_count,
        "spawnRateScaleSourceDistributionKind": "RawDistributionFloat",
        "spawnRateDefaultDependentOccurrences": spawn_rate_default_occurrences,
        "spawnExplicitZeroRateCount": len(explicit_zero_rate_occurrences),
        "spawnExplicitNonzeroRates": {
            str(index): value for index, value in sorted(explicit_nonzero_rates.items())
        },
        "spawnParameterRateOccurrences": parameter_rate_occurrences,
    }


def action_parameters(action_recipe: dict[str, Any]) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for cue in action_recipe.get("cues", []):
        values: dict[str, Any] = {}
        for row in (cue.get("typedPayload") or {}).get("parameterOverrides", []):
            kind = str(row.get("type") or "").casefold()
            if kind == "scalar":
                value = finite_number(row.get("scalarValue"))
            elif kind == "vector":
                value = vector4(row.get("vectorValue"))[:3]
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


def selected_value(row: dict[str, Any] | None) -> tuple[Any, str, str]:
    if not isinstance(row, dict):
        return None, "", ""
    selected = row.get("selected")
    if not isinstance(selected, dict):
        return None, "", ""
    tagged = selected.get("value")
    value = tagged.get("value") if isinstance(tagged, dict) else tagged
    return value, str(selected.get("tier") or ""), str(
        selected.get("evidenceStatus") or ""
    )


def current_local_fields(definition: dict[str, Any]) -> list[dict[str, Any]]:
    coverage = definition.get("semanticCoverage") or {}
    fields: dict[str, dict[str, Any]] = {}
    for name, row in (coverage.get("resolvedFields") or {}).items():
        value, tier, evidence = selected_value(row)
        if value is not None:
            fields[str(name).casefold()] = {
                "fieldPath": str(name).casefold(),
                "value": copy.deepcopy(value),
                "provenanceTier": tier,
                "evidenceStatus": evidence,
            }

    current_object = (
        (definition.get("currentRevisionTargetEvidence") or {}).get("object") or {}
    )
    for name, tagged in (current_object.get("properties") or {}).items():
        if not isinstance(tagged, dict):
            continue
        fields[str(name).casefold()] = {
            "fieldPath": str(name).casefold(),
            "value": copy.deepcopy(tagged.get("value")),
            "provenanceTier": "CURRENT_REVISION_INSTANCE_EXPLICIT",
            "evidenceStatus": "CURRENT_REVISION_TARGET_EVIDENCE",
        }

    source_class = str(definition.get("sourceClass") or "").casefold()
    if "particleparameter" in source_class and "parametername" not in fields:
        fields["parametername"] = {
            "fieldPath": "parametername",
            "value": None,
            "provenanceTier": "CURRENT_NATIVE_EVALUATOR_DEFAULT",
            "evidenceStatus": "CURRENT_NATIVE_EVALUATOR_SHAPE_ONLY",
        }
    return [fields[name] for name in sorted(fields)]


def field_map(rows: Iterable[dict[str, Any]]) -> dict[str, Any]:
    return {str(row["fieldPath"]).casefold(): row.get("value") for row in rows}


def local_parameter_oracle(
    definition: dict[str, Any], occurrence: dict[str, Any],
    parameters: dict[str, dict[str, Any]], fields: list[dict[str, Any]],
) -> dict[str, Any]:
    coverage = definition.get("semanticCoverage") or {}
    source_class = str(definition.get("sourceClass") or "").casefold()
    if source_class == "distributionfloatconstantcurve":
        return {
            "branch": "STANDARD_CONSTANT_CURVE",
            "parameterInput": None,
            "value": None,
            "blocked": False,
        }

    values = field_map(fields)
    expected_type = str(coverage.get("expectedOverrideType") or "").casefold()
    parameter_name = values.get("parametername")
    parameter = (
        parameters.get(str(parameter_name).casefold())
        if parameter_name not in (None, "", "none")
        else None
    )
    if parameter is not None and parameter.get("kind") != expected_type:
        parameter = None
    constant = vector4(values.get("constant"))
    component_count = 3 if expected_type == "vector" else 1
    if parameter is None:
        result = constant
        branch = "CONSTANT_FALLBACK"
    else:
        input_value = vector4(parameter["value"])
        result = [0.0, 0.0, 0.0, 0.0]
        for index in range(component_count):
            mode_name = "parammode" if component_count == 1 else (
                "parammodes" if index == 0 else f"parammodes[{index}]"
            )
            mode = str(values.get(mode_name, "dpm_normal")).casefold()
            if mode == "dpm_direct":
                result[index] = input_value[index]
                continue
            source = abs(input_value[index]) if mode == "dpm_abs" else input_value[index]
            min_input = vector4(values.get("mininput"))[index]
            max_input = vector4(values.get("maxinput"))[index]
            min_output = vector4(values.get("minoutput"))[index]
            max_output = vector4(values.get("maxoutput"))[index]
            ratio = 0.0 if max_input == min_input else (
                (source - min_input) / (max_input - min_input)
            )
            ratio = min(1.0, max(0.0, ratio))
            result[index] = min_output + (max_output - min_output) * ratio
        branch = "PARAMETER_INPUT"

    custom = source_class == "efdistributionvectormultiplyparticleparameter"
    return {
        "branch": "UNRESOLVED_CUSTOM_EVALUATOR" if custom else branch,
        "parameterInput": copy.deepcopy(parameter),
        "value": None if custom else result[:component_count],
        "diagnosticStandardBaseValue": result[:component_count] if custom else None,
        "blocked": custom,
    }


def normalized_tagged(value: Any) -> Any:
    if isinstance(value, list):
        return [normalized_tagged(row) for row in value]
    if isinstance(value, dict):
        result = {}
        for name, row in value.items():
            key = str(name).casefold()
            if key in {"type", "structtype"} and isinstance(row, str):
                result[key] = row.casefold()
            else:
                result[key] = normalized_tagged(row)
        return result
    return value


def external_native_tail_evidence(
    semantic_closure: dict[str, Any], external_closure: dict[str, Any],
    release_root: Path,
) -> dict[str, Any]:
    objects: dict[str, dict[str, Any]] = {}
    packages: dict[str, dict[str, Any]] = {}
    for package in external_closure.get("packages", []):
        logical = str(package.get("logicalPackage") or "").casefold()
        require(logical and logical not in packages, f"duplicate external package: {logical}")
        packages[logical] = package
        for row in package.get("objects", []):
            object_id = str(row.get("objectId") or "")
            require(object_id and object_id not in objects, f"duplicate external object: {object_id}")
            objects[object_id] = row

    required: dict[str, str] = {}
    for occurrence in semantic_closure.get("occurrences", []):
        for module in occurrence.get("modules", []):
            if module.get("sourceDocument") == "externalModuleClosure":
                object_id = str(module["sourceObjectId"])
                previous = required.setdefault(object_id, str(module["sourceRecordSha256"]))
                require(previous == module["sourceRecordSha256"],
                        f"external object hash split: {object_id}")
    require(len(required) == 178, "external unique module denominator changed")

    cache: dict[str, Any] = {}
    package_rows: dict[str, dict[str, Any]] = {}
    object_rows: list[dict[str, Any]] = []
    for object_id in sorted(required):
        logical = object_id.partition(":")[0].casefold()
        package_row = packages.get(logical)
        require(package_row is not None, f"external package mapping missing: {logical}")
        physical = str(package_row.get("physicalPackage") or "")
        path = release_root / "Packages" / physical
        require(path.is_file(), f"installed current package missing: {path}")
        package = cache.get(physical.casefold())
        if package is None:
            package = load_package(path, LOSTARK_KR_AES_KEY)
            cache[physical.casefold()] = package
            package_rows[logical] = {
                "logicalPackage": package_row["logicalPackage"],
                "physicalPackage": physical,
                "bytes": path.stat().st_size,
                "sha256": sha256_file(path),
                "provenance": "LATE_PINNED_CURRENT_REVISION_OBSERVATION",
            }
        source = objects.get(object_id)
        require(source is not None, f"external object missing: {object_id}")
        path_in_package = str(source.get("objectPath") or "")
        entry = find_export(package, path_in_package)
        class_name = package_ref_name(entry.class_index, package.imports, package.exports) or ""
        actual_path = package_ref_path(entry.index + 1, package.imports, package.exports)
        serial = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        properties, property_end = parse_tagged_properties(
            serial, package.names, package.summary.version
        )
        require(entry.index == int(source["exportIndex"]), f"external export changed: {object_id}")
        require(class_name.casefold() == str(source["className"]).casefold(),
                f"external class changed: {object_id}")
        require(actual_path.casefold() == path_in_package.casefold(),
                f"external path changed: {object_id}")
        require(
            normalized_tagged(properties) == normalized_tagged(source["properties"]),
            f"external tagged properties changed: {object_id}",
        )
        require(property_end == int(source["propertyStreamEnd"]),
                f"external property end changed: {object_id}")
        require(property_end == entry.serial_size,
                f"current external native tail is nonempty: {object_id}")
        require(canonical_sha256(source) == required[object_id],
                f"external source record hash changed: {object_id}")
        object_rows.append({
            "sourceObjectId": object_id,
            "logicalPackage": package_row["logicalPackage"],
            "physicalPackage": physical,
            "exportIndex": entry.index,
            "objectPath": actual_path,
            "className": class_name.casefold(),
            "sourceRecordSha256": required[object_id],
            "serialSha256": hashlib.sha256(serial).hexdigest(),
            "serialSize": entry.serial_size,
            "propertyStreamEnd": property_end,
            "decision": "VERIFIED_IRRELEVANT",
            "oracleId": "ue3.current-export.serial-equals-property-end.v1",
            "sourceFidelity": "CURRENT_REVISION_EVIDENCE_NOT_SOURCE_EXACT",
        })
    return {
        "packages": [package_rows[name] for name in sorted(package_rows)],
        "objects": object_rows,
        "uniqueObjectCount": len(object_rows),
    }


def decode_cdo(package: Any, object_path: str) -> dict[str, Any]:
    entry = find_export(package, object_path)
    serial = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
    properties, property_end = parse_tagged_properties(
        serial, package.names, package.summary.version
    )
    require(property_end == entry.serial_size, f"CDO native tail changed: {object_path}")
    return {
        "objectPath": package_ref_path(entry.index + 1, package.imports, package.exports),
        "className": (
            package_ref_name(entry.class_index, package.imports, package.exports) or ""
        ).casefold(),
        "exportIndex": entry.index,
        "recordSha256": canonical_sha256(normalized_tagged(properties)),
        "properties": normalized_tagged(properties),
    }


def current_default_evidence(
    local_closure: dict[str, Any], release_root: Path,
) -> dict[str, Any]:
    evidence = local_closure.get("currentRevisionClassDefaultEvidence") or {}
    script_rows = evidence.get("scriptPackages") or []
    require(len(script_rows) == 2, "current script package denominator changed")
    packages: dict[str, Any] = {}
    identities: list[dict[str, Any]] = []
    for row in script_rows:
        logical = str(row.get("logicalPackage") or "")
        path = release_root / str(row.get("physicalPackage") or "")
        require(path.is_file(), f"current script package missing: {path}")
        require(path.stat().st_size == int(row["bytes"]), f"script bytes changed: {logical}")
        require(sha256_file(path) == row["sha256"], f"script SHA changed: {logical}")
        packages[logical.casefold()] = load_package(path, LOSTARK_KR_AES_KEY)
        identities.append({
            **copy.deepcopy(row),
            "provenance": "LATE_PINNED_CURRENT_REVISION_ENVIRONMENT_RECEIPT",
            "sourceEraIdentityPinned": False,
        })

    engine = packages["engine"]
    efgame = packages["efgame"]
    cdo_rows = {
        "particleLodLevel": decode_cdo(engine, "Default__ParticleLODLevel"),
        "particleModuleRequired": decode_cdo(engine, "Default__ParticleModuleRequired"),
        "particleModuleSpawn": decode_cdo(engine, "Default__ParticleModuleSpawn"),
        "particleModuleTypeDataRibbon": decode_cdo(
            engine, "Default__ParticleModuleTypeDataRibbon"
        ),
        "efParticleModuleTypeDataDecal": decode_cdo(
            efgame, "Default__EFParticleModuleTypeDataDecal"
        ),
    }
    seeded_classes = (
        "efparticlemodulelocationprimitivecylinderspin_seeded",
        "particlemodulecolor_seeded",
        "particlemodulelifetime_seeded",
        "particlemodulelocation_seeded",
        "particlemodulelocationprimitivecylinder_seeded",
        "particlemodulemeshrotation_seeded",
        "particlemodulesize_seeded",
        "particlemodulevelocity_seeded",
    )
    seed_policy = {
        "parameterName": None,
        "getSeedFromInstance": False,
        "instanceSeedIsIndex": False,
        "resetSeedOnEmitterLooping": True,
        "randomlySelectSeedArray": False,
        "emptyArrayUsesOccurrenceRandomStream": True,
    }
    for class_name in seeded_classes:
        key = "seed::" + class_name
        cdo = decode_cdo(
            efgame if class_name.startswith("ef") else engine,
            "Default__" + class_name,
        )
        random_seed = cdo["properties"].get("randomseedinfo") or {}
        nested = (random_seed.get("value") or {}).get("properties") or {}
        require(
            nested.get("parametername", {}).get("value") == "None"
            and nested.get("bgetseedfrominstance", {}).get("value") is False
            and nested.get("binstanceseedisindex", {}).get("value") is False
            and nested.get("bresetseedonemitterlooping", {}).get("value") is True
            and nested.get("brandomlyselectseedarray", {}).get("value") is False
            and nested.get("randomseeds", {}).get("value") == [],
            f"current seeded CDO policy changed: {class_name}",
        )
        cdo_rows[key] = cdo
    lod_props = cdo_rows["particleLodLevel"]["properties"]
    require(lod_props["benabled"]["value"] is True, "current LOD enabled default changed")
    required_props = cdo_rows["particleModuleRequired"]["properties"]
    require("buselocalspace" not in required_props,
            "current required local-space serialization changed")
    decal = cdo_rows["efParticleModuleTypeDataDecal"]["properties"]
    ribbon = cdo_rows["particleModuleTypeDataRibbon"]["properties"]
    require(decal["defaultsize"]["value"] == {"x": 50.0, "y": 50.0},
            "current decal size default changed")
    require(ribbon["sheetspertrail"]["value"] == 1,
            "current ribbon sheet default changed")
    return {
        "status": "CURRENT_REVISION_DEFAULTS_NOT_SOURCE_EXACT",
        "scriptPackages": identities,
        "classDefaultObjects": cdo_rows,
        "selectedLodProjection": {
            "level": 0,
            "enabled": True,
            "decision": "VERIFIED_IRRELEVANT",
            "reason": "FIRST_LOD_ONLY_SOURCE_IDENTITY_COMPILED_AT_ADAPTER_BUILD",
        },
        "requiredLocalSpace": {
            "value": False,
            "provenance": "CURRENT_INHERITED_LANGUAGE_DEFAULT",
        },
        "decal": {
            "values": {
                name: copy.deepcopy(row["value"]) for name, row in decal.items()
            },
            "provenance": "CURRENT_EFGAME_CDO",
        },
        "ribbon": {
            "values": {
                name: copy.deepcopy(row["value"]) for name, row in ribbon.items()
            },
            "provenance": "CURRENT_ENGINE_CDO",
        },
        "randomSeedPolicy": {
            **seed_policy,
            "provenance": "CURRENT_ENGINE_OR_EFGAME_SEEDED_CLASS_CDO_QUORUM",
            "sourceEraIdentityPinned": False,
            "classCount": len(seeded_classes),
        },
    }


def current_engine_spawn_ratescale_reconstruction(
    defaults: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    cdo = defaults["classDefaultObjects"]["particleModuleSpawn"]
    require(cdo["recordSha256"] == canonical_sha256(cdo["properties"]),
            "current ParticleModuleSpawn CDO record hash changed")
    tagged = cdo["properties"].get("ratescale") or {}
    require(tagged.get("type") == "structproperty"
            and tagged.get("structtype") == "rawdistributionfloat",
            "current ParticleModuleSpawn RateScale tag changed")
    raw_value = tagged.get("value") or {}
    fields = raw_value.get("properties") or {}
    require(
        int(raw_value.get("size", -1)) == 268
        and (fields.get("distribution") or {}).get("type") == "objectproperty"
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
    engine_identity = next(
        row for row in defaults["scriptPackages"]
        if str(row.get("logicalPackage") or "").casefold() == "engine"
    )
    descriptor = {
        "propertyPath": "ratescale",
        "sourceClass": "",
        "sourceObjectPath": cdo["objectPath"],
        "componentCount": 1,
        "operation": 1,
        "randomLockAxes": 0,
        "lookupTableChunkSize": 1,
        "lookupTableNumElements": 1,
        "lookupTableTimeScale": 0.0,
        "lookupTableStartTime": 0.0,
        "defaultMinimum": [0.0, 0.0, 0.0, 0.0],
        "defaultMaximum": [0.0, 0.0, 0.0, 0.0],
        "lookupTable": [1.0, 1.0, 1.0, 1.0],
        "keys": [],
        "referenceId": "",
        "occurrenceId": "",
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
        "logicalPackage": engine_identity["logicalPackage"],
        "physicalPackage": engine_identity["physicalPackage"],
        "packageBytes": engine_identity["bytes"],
        "packageSha256": engine_identity["sha256"],
        "classDefaultObject": cdo["objectPath"],
        "classDefaultExportIndex": cdo["exportIndex"],
        "classDefaultRecordSha256": cdo["recordSha256"],
        "propertyPath": "ratescale",
        "value": 1.0,
        "reconstructedDescriptorSha256": canonical_sha256(descriptor),
    }
    return descriptor, evidence


def default_dependent_policy_receipt(
    policy_id: str, defaults: dict[str, Any],
) -> dict[str, Any]:
    require(policy_id in ALLOWED_DEFAULT_POLICIES,
            f"unknown default-dependent policy: {policy_id}")
    row = {
        "policyId": policy_id,
        "sourceExact": False,
        "sourceExactUpgradeAllowed": False,
        "nullRawDistributionNumericPayloadByteCount": 0,
        "numericPayloadDefinition": "LOOKUP_TABLE_OR_KEY_VALUE_BYTES",
    }
    if policy_id == FAIL_CLOSED_DEFAULT_POLICY:
        row.update({
            "mode": "FAIL_CLOSED",
            "resolvedScope": [],
        })
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


def parse_seed(
    module_payload: dict[str, Any], exact_class: str, defaults: dict[str, Any]
) -> dict[str, Any]:
    literals = module_payload["literals"]
    rows = {
        str(row["propertyPath"]).casefold(): row.get("value")
        for row in literals
        if str(row.get("propertyPath") or "").casefold().startswith("randomseedinfo")
    }
    if not rows:
        seeds: list[int] = []
        source = "CURRENT_CLASS_DEFAULT_EMPTY_ARRAY"
    elif "randomseedinfo.properties.randomseeds[0]" in rows:
        seeds = [int(rows["randomseedinfo.properties.randomseeds[0]"])]
        source = "SOURCE_DECODED_SEED_ARRAY"
    else:
        encoded = str(rows.get("randomseedinfo.hex") or "")
        raw = bytes.fromhex(encoded)
        # The tagged StructProperty reports 40 bytes including its UE3 tag-side
        # framing; the sourceRecipe hex is the 32-byte struct body.  The final
        # int32 is the single RandomSeeds array element.
        require(len(raw) == 32, "seed metadata payload size changed")
        seeds = [struct.unpack_from("<i", raw, 28)[0]]
        source = "SOURCE_OPAQUE_STRUCT_ARRAY_DECODED"
    return {
        "decision": "READY_FOR_HANDLER",
        "evaluatorId": "ue3.particle-random-seed-info.current-default.v1",
        "randomSeeds": seeds,
        "policy": {
            name: copy.deepcopy(value)
            for name, value in defaults["randomSeedPolicy"].items()
            if name not in {"provenance", "sourceEraIdentityPinned", "classCount"}
        },
        "currentCdoEvidenceKey": "seed::" + exact_class,
        "source": source,
        "sourceFidelity": (
            "CURRENT_REVISION_DEFAULT_RECONSTRUCTION"
            if not rows else "MODULE_SOURCE_EVIDENCE"
        ),
    }


def default_adapter(
    default: dict[str, Any], defaults: dict[str, Any]
) -> dict[str, Any]:
    family = str(default["family"])
    if family == "RequiredLocalSpace":
        return {
            "defaultId": default["defaultId"],
            "family": family,
            "fieldPath": default["fieldPath"],
            "decision": "READY_FOR_HANDLER",
            **copy.deepcopy(defaults["requiredLocalSpace"]),
        }
    if family == "Decal":
        value = defaults["decal"]
        decision = "READY_FOR_HANDLER"
    elif family == "Ribbon":
        value = defaults["ribbon"]
        decision = "READY_FOR_HANDLER"
    elif family == "Light":
        value = {
            "provenance": "CURRENT_DEFAULT_CHAIN_IN_LOCAL_REFERENCE_CLOSURE",
            "valuesOwnedBy": "pointLightAdapter",
        }
        decision = "READY_FOR_HANDLER"
    elif family == "ScreenPost":
        value = {
            "provenance": "NO_IMPLICIT_SOURCE_FIELD",
            "reason": "SCREEN_POST_IS_RENDERER_PROJECTION_OF_TYPED_MODULE_PAYLOAD",
        }
        decision = "VERIFIED_IRRELEVANT"
    else:
        raise ValueError(f"unknown default family: {family}")
    return {
        "defaultId": default["defaultId"],
        "family": family,
        "fieldPath": default["fieldPath"],
        "decision": decision,
        **copy.deepcopy(value),
    }


def point_light_adapter(local_closure: dict[str, Any]) -> dict[str, Any]:
    definitions = local_closure.get("componentDefinitions") or []
    occurrences = local_closure.get("componentOccurrences") or []
    require(len(definitions) == 1 and len(occurrences) == 1,
            "PointLight denominator changed")
    definition = definitions[0]
    fields = []
    for name, row in (definition.get("semanticCoverage") or {}).get(
        "resolvedFields", {}
    ).items():
        value, tier, evidence = selected_value(row)
        require(value is not None, f"PointLight field unresolved: {name}")
        irrelevant = str(name).casefold() in {"lightguid", "lightmapguid"}
        fields.append({
            "fieldId": "point-light::" + str(name).casefold(),
            "fieldPath": str(name).casefold(),
            "value": copy.deepcopy(value),
            "sourceTier": tier,
            "sourceFidelity": evidence,
            "decision": "VERIFIED_IRRELEVANT" if irrelevant else "READY_FOR_HANDLER",
            "oracleId": "ue3.runtime-light-guid-irrelevance.v1" if irrelevant else "",
        })
    return {
        "definitionId": definition["definitionId"],
        "occurrenceId": occurrences[0]["occurrenceId"],
        "exactSourceClass": "pointlightcomponent",
        "handlerCapabilityId": "source.component.pointlightcomponent.v1",
        "handlerDecision": "READY_FOR_HANDLER",
        "fields": fields,
        "sourceEraDefaultIdentityPinned": False,
    }


def build_receipt(
    *, root: Path, release_root: Path, inputs: dict[str, dict[str, Any]],
    default_dependent_policy: str = FAIL_CLOSED_DEFAULT_POLICY,
) -> dict[str, Any]:
    semantic = inputs["semanticClosure"]
    candidate = inputs["candidate"]
    candidate_receipt = inputs["candidateReceipt"]
    local = inputs["localReferenceClosure"]
    external = inputs["externalModuleClosure"]
    action_recipe = inputs["actionCueRecipe"]
    require(semantic.get("schema") == "lostark.effect-source-semantic-closure",
            "semantic closure schema changed")
    require(candidate.get("schema") == "lostark.effect-authoring",
            "candidate schema changed")
    require(candidate_receipt.get("skillId") == 31470, "candidate receipt skill changed")
    require(local.get("skillId") == 31470, "local closure skill changed")
    require(action_recipe.get("inputSlot") == "F", "ActionCue input slot changed")
    require(default_dependent_policy in ALLOWED_DEFAULT_POLICIES,
            f"unknown default-dependent policy: {default_dependent_policy}")
    full35_default_gate = full35_default_dependent_gate(semantic, candidate)

    input_rows = {}
    for name, relative in TRACKED_INPUTS.items():
        path = root / relative
        input_rows[name] = {
            "path": relative,
            "canonicalJsonSha256": canonical_sha256(inputs[name]),
            "bytes": path.stat().st_size,
        }

    defaults = current_default_evidence(local, release_root)
    default_policy = default_dependent_policy_receipt(
        default_dependent_policy, defaults
    )
    native_evidence = external_native_tail_evidence(semantic, external, release_root)
    parameters_by_cue = action_parameters(action_recipe)
    local_definitions = {
        str(row["definitionId"]): row for row in local.get("distributionDefinitions", [])
    }
    local_occurrences = {
        str(row["occurrenceId"]): row for row in local.get("distributionOccurrences", [])
    }
    candidate_elements = candidate.get("elements") or []
    semantic_occurrences = semantic.get("occurrences") or []
    require(len(candidate_elements) == len(semantic_occurrences) == EXPECTED["occurrenceCount"],
            "occurrence denominator changed")

    handler_classes: Counter[str] = Counter()
    occurrence_rows: list[dict[str, Any]] = []
    local_rows: list[dict[str, Any]] = []
    consumed_local_occurrences: set[str] = set()
    payload_literal_ids: set[str] = set()
    consumed_literal_ids: set[str] = set()
    payload_distribution_ids: set[str] = set()
    consumed_distribution_ids: set[str] = set()
    module_decisions: Counter[str] = Counter()
    property_decisions: Counter[str] = Counter()
    leaf_decisions: Counter[str] = Counter()
    distribution_decisions: Counter[str] = Counter()
    default_decisions: Counter[str] = Counter()
    seed_count = 0
    external_native_occurrences = 0

    for semantic_occurrence, element in zip(semantic_occurrences, candidate_elements):
        emitter = str(element.get("sourceNode") or "").partition("|")[2]
        require(emitter.casefold() == str(semantic_occurrence["sourceEmitterPath"]).casefold(),
                "candidate occurrence/source identity changed")
        recipe = element.get("sourceRecipe") or {}
        candidate_modules = recipe.get("modules") or []
        coverage_modules = recipe.get("moduleCoverage") or []
        semantic_modules = semantic_occurrence.get("modules") or []
        require(len(candidate_modules) == len(coverage_modules) == len(semantic_modules),
                f"module denominator changed: {emitter}")
        module_rows = []
        for module, payload, coverage in zip(
            semantic_modules, candidate_modules, coverage_modules
        ):
            module_id = str(module["moduleOccurrenceId"])
            stable_id = str(payload.get("stableId") or "")
            require(stable_id.partition("@ref:")[0].casefold()
                    == str(module["sourceObjectId"]).casefold(),
                    f"module source object changed: {module_id}")
            require(int(stable_id.rpartition("@ref:")[2]) == int(module["order"]),
                    f"module order changed: {module_id}")
            exact_class = str(module["exactSourceClass"]).casefold()
            require(str(coverage.get("exactSourceClass") or "").casefold() == exact_class,
                    f"coverage exact class changed: {module_id}")
            handler_classes[exact_class] += 1

            literals = []
            for index, literal in enumerate(payload.get("literals", [])):
                literal_id = f"{module_id}::literal:{index:03d}"
                require(literal_id not in payload_literal_ids, f"duplicate literal: {literal_id}")
                payload_literal_ids.add(literal_id)
                literals.append({"literalId": literal_id, **copy.deepcopy(literal)})
            distributions = []
            for index, descriptor in enumerate(payload.get("distributions", [])):
                distribution_id = f"{module_id}::payload-distribution:{index:03d}"
                require(distribution_id not in payload_distribution_ids,
                        f"duplicate payload distribution: {distribution_id}")
                payload_distribution_ids.add(distribution_id)
                distributions.append({
                    "payloadDistributionId": distribution_id,
                    "descriptor": descriptor_payload(descriptor),
                })

            properties = []
            property_by_path = {}
            for row in module.get("properties", []):
                path = str(row["propertyPath"]).casefold()
                matching_literals = [
                    item["literalId"] for item in literals
                    if str(item["propertyPath"]).casefold() == path
                    or str(item["propertyPath"]).casefold().startswith(path + ".")
                    or str(item["propertyPath"]).casefold().startswith(path + "[")
                ]
                matching_distributions = [
                    item["payloadDistributionId"] for item in distributions
                    if str(item["descriptor"]["propertyPath"]).casefold() == path
                    or str(item["descriptor"]["propertyPath"]).casefold().startswith(path + ".")
                    or str(item["descriptor"]["propertyPath"]).casefold().startswith(path + "[")
                ]
                require(matching_literals or matching_distributions,
                        f"property has no typed payload: {row['propertyId']}")
                semantic_distributions = [
                    item for item in module.get("distributions", [])
                    if str(item["topLevelPropertyPath"]).casefold() == path
                ]
                custom_distribution = any(
                    str(item.get("exactSourceClass") or "").casefold()
                    == "efdistributionvectormultiplyparticleparameter"
                    for item in semantic_distributions
                )
                if path in IRRELEVANT_PROPERTIES:
                    decision = "VERIFIED_IRRELEVANT"
                    oracle_id = IRRELEVANT_PROPERTIES[path]
                    blockers: list[str] = []
                elif custom_distribution:
                    decision = "BLOCKED"
                    oracle_id = ""
                    blockers = [CUSTOM_DISTRIBUTION_BLOCKER]
                else:
                    decision = "READY_FOR_HANDLER"
                    oracle_id = ""
                    blockers = []
                consumed_literal_ids.update(matching_literals)
                consumed_distribution_ids.update(matching_distributions)
                prop = {
                    "propertyId": row["propertyId"],
                    "propertyPath": path,
                    "decision": decision,
                    "handlerCapabilityId": (
                        f"source.property.{sanitized_id(exact_class)}."
                        f"{sanitized_id(path)}.v1"
                    ),
                    "irrelevanceOracleId": oracle_id,
                    "payloadLiteralIds": matching_literals,
                    "payloadDistributionIds": matching_distributions,
                    "semanticDistributionIds": [
                        item["distributionId"] for item in semantic_distributions
                    ],
                    "sourceFidelity": row["sourceFidelity"],
                    "blockers": blockers,
                }
                properties.append(prop)
                property_by_path[path] = prop
                property_decisions[decision] += 1

            leaves = []
            for row in module.get("primitiveLeaves", []):
                path = str(row["propertyPath"]).casefold()
                literal = next(
                    (item for item in literals
                     if str(item["propertyPath"]).casefold() == path), None
                )
                require(literal is not None, f"leaf has no typed literal: {row['leafId']}")
                parent = property_by_path[str(row["topLevelPropertyPath"]).casefold()]
                leaves.append({
                    "leafId": row["leafId"],
                    "propertyPath": path,
                    "topLevelPropertyPath": str(row["topLevelPropertyPath"]).casefold(),
                    "kind": row["valueKind"],
                    "payloadLiteralId": literal["literalId"],
                    "decision": parent["decision"],
                    "handlerCapabilityId": parent["handlerCapabilityId"],
                    "blockers": copy.deepcopy(parent["blockers"]),
                })
                consumed_literal_ids.add(literal["literalId"])
                leaf_decisions[parent["decision"]] += 1

            semantic_distribution_rows = []
            require(len(distributions) == len(module.get("distributions", [])),
                    f"distribution payload count changed: {module_id}")
            payload_by_path: dict[str, list[dict[str, Any]]] = defaultdict(list)
            for payload_distribution in distributions:
                payload_by_path[str(
                    payload_distribution["descriptor"]["propertyPath"]
                ).casefold()].append(payload_distribution)
            for semantic_distribution in module.get("distributions", []):
                semantic_path = str(
                    semantic_distribution["propertyPath"]
                ).casefold()
                candidates = payload_by_path.get(semantic_path, [])
                require(candidates,
                        f"distribution payload path changed: {module_id}:{semantic_path}")
                payload_distribution = candidates.pop(0)
                descriptor = payload_distribution["descriptor"]
                field_evidence = semantic_distribution["fieldEvidence"]
                default_dependent = bool(field_evidence.get("defaultDependent"))
                semantic_execution_blockers = sorted({
                    str(blocker) for blocker in semantic_distribution.get(
                        "executionBlockers", []
                    )
                })
                local_occurrence_id = str(descriptor.get("occurrenceId") or "")
                if local_occurrence_id:
                    require(not default_dependent,
                            "local distribution became class-default dependent")
                    occurrence = local_occurrences.get(local_occurrence_id)
                    require(occurrence is not None,
                            f"local distribution occurrence missing: {local_occurrence_id}")
                    definition = local_definitions[str(occurrence["definitionId"])]
                    fields = current_local_fields(definition)
                    parameter_rows = parameters_by_cue.get(str(occurrence["sourceCueId"]), {})
                    oracle = local_parameter_oracle(
                        definition, occurrence, parameter_rows, fields
                    )
                    source_class = str(definition["sourceClass"]).casefold()
                    if source_class == "distributionfloatconstantcurve":
                        reconstructed_descriptor = copy.deepcopy(descriptor)
                        samples = numeric_samples(reconstructed_descriptor)
                        decision = "READY_FOR_HANDLER"
                        blockers = []
                    elif oracle["blocked"]:
                        reconstructed_descriptor = None
                        samples = []
                        decision = "BLOCKED"
                        blockers = [CUSTOM_DISTRIBUTION_BLOCKER]
                    else:
                        reconstructed_descriptor = {
                            "sourceClass": source_class,
                            "sourceObjectPath": (
                                (definition.get("currentRevisionTargetEvidence") or {})
                                .get("object", {}).get("objectPath")
                                or definition.get("targetPackageLocalPath") or ""
                            ),
                            "componentCount": 3 if (
                                (definition.get("semanticCoverage") or {}).get(
                                    "expectedOverrideType"
                                ) == "vector"
                            ) else 1,
                            "fields": copy.deepcopy(fields),
                        }
                        samples = [{
                            "sourceCueId": occurrence["sourceCueId"],
                            **copy.deepcopy(oracle),
                        }]
                        decision = "READY_FOR_HANDLER"
                        blockers = []
                    local_row = {
                        "distributionId": semantic_distribution["distributionId"],
                        "propertyPath": semantic_path,
                        "topLevelPropertyPath": str(
                            semantic_distribution["topLevelPropertyPath"]
                        ).casefold(),
                        "referenceId": occurrence["referenceId"],
                        "definitionId": occurrence["definitionId"],
                        "legacyOccurrenceId": local_occurrence_id,
                        "payloadDistributionId": payload_distribution[
                            "payloadDistributionId"
                        ],
                        "exactSourceClass": source_class,
                        "evaluatorCapabilityId": (
                            "source.distribution.exact."
                            + sanitized_id(source_class) + ".v1"
                        ),
                        "decision": decision,
                        "currentRevisionFields": fields,
                        "reconstructedDescriptor": reconstructed_descriptor,
                        "numericOracleSamples": samples,
                        "sourceFidelity": definition["fidelity"],
                        "sourceEraIdentityPinned": bool(
                            definition.get("exactPhysicalSourcePackagePresent")
                        ),
                        "defaultDependent": False,
                        "sourceNumericPayloadByteCount": None,
                        "sourceNumericPayloadDefinition": None,
                        "semanticClosureExecutionBlockers": (
                            semantic_execution_blockers
                        ),
                        "defaultResolution": None,
                        "blockers": blockers,
                    }
                    local_rows.append(local_row)
                    consumed_local_occurrences.add(local_occurrence_id)
                    semantic_distribution_rows.append(copy.deepcopy(local_row))
                else:
                    reconstructed = list(field_evidence["reconstructedFieldNames"])
                    reconstructed_descriptor = None
                    default_resolution = None
                    if default_dependent:
                        require_null_raw_distribution_projection(
                            semantic_distribution, descriptor
                        )
                        if (exact_class == "particlemodulespawn"
                                and semantic_path == "ratescale"):
                            require_spawn_ratescale_raw_distribution_float(
                                semantic_distribution, descriptor
                            )
                        can_use_current_cdo = (
                            default_dependent_policy == CURRENT_ENGINE_CDO_POLICY
                            and exact_class == "particlemodulespawn"
                            and semantic_path == "ratescale"
                        )
                        if can_use_current_cdo:
                            reconstructed_descriptor, default_resolution = (
                                current_engine_spawn_ratescale_reconstruction(defaults)
                            )
                            samples = numeric_samples(reconstructed_descriptor)
                            decision = "READY_FOR_HANDLER"
                            blockers = []
                            reconstruction_basis = CURRENT_ENGINE_CDO_POLICY
                        else:
                            samples = []
                            decision = "BLOCKED"
                            blockers = sorted({
                                *semantic_execution_blockers,
                                DEFAULT_POLICY_BLOCKER,
                            })
                            default_resolution = {
                                "policyId": default_dependent_policy,
                                "status": "BLOCKED",
                                "provenanceTier": "SOURCE_NULL_RAW_DISTRIBUTION",
                                "sourceExact": False,
                                "sourceExactUpgradeAllowed": False,
                                "numericPayloadByteCount": 0,
                                "numericPayloadDefinition": (
                                    "LOOKUP_TABLE_OR_KEY_VALUE_BYTES"
                                ),
                            }
                            reconstruction_basis = (
                                "NULL_RAW_DISTRIBUTION_FAIL_CLOSED"
                            )
                    else:
                        expected_operation = (
                            1 if "operation" in reconstructed else int(
                                field_evidence["explicitOperation"]
                            )
                        )
                        require(int(descriptor["operation"]) == expected_operation,
                                "distribution operation reconstruction changed")
                        if descriptor["lookupTable"]:
                            expected_chunk = int(descriptor["componentCount"]) * (
                                2 if int(descriptor["operation"]) >= 2 else 1
                            )
                            require(
                                int(descriptor["lookupTableChunkSize"])
                                == expected_chunk,
                                "distribution chunk reconstruction changed",
                            )
                            require(int(descriptor["lookupTableNumElements"]) == (
                                2 if int(descriptor["operation"]) >= 2 else 1
                            ), "distribution element reconstruction changed")
                        samples = numeric_samples(descriptor)
                        decision = "READY_FOR_HANDLER"
                        blockers = []
                        reconstruction_basis = (
                            "CURRENT_UE3_RAW_DISTRIBUTION_DEFAULT_AND_PAYLOAD_SHAPE"
                            if reconstructed else "SOURCE_TAGGED_FIELDS"
                        )
                    inline = {
                        "distributionId": semantic_distribution["distributionId"],
                        "propertyPath": semantic_path,
                        "topLevelPropertyPath": str(
                            semantic_distribution["topLevelPropertyPath"]
                        ).casefold(),
                        "payloadDistributionId": payload_distribution[
                            "payloadDistributionId"
                        ],
                        "exactSourceClass": str(
                            semantic_distribution.get("exactSourceClass") or ""
                        ).casefold(),
                        "evaluatorCapabilityId": "source.distribution.ue3-cooked.v1",
                        "decision": decision,
                        "fieldProvenance": {
                            "rawFieldSourceFidelity": field_evidence[
                                "rawFieldSourceFidelity"
                            ],
                            "reconstructedFieldNames": reconstructed,
                            "reconstructionBasis": reconstruction_basis,
                            "sourceExactUpgradeAllowed": False,
                        },
                        "defaultDependent": default_dependent,
                        "sourceNumericPayloadByteCount": (
                            0 if default_dependent else None
                        ),
                        "sourceNumericPayloadDefinition": (
                            "LOOKUP_TABLE_OR_KEY_VALUE_BYTES"
                            if default_dependent else None
                        ),
                        "semanticClosureExecutionBlockers": (
                            semantic_execution_blockers
                        ),
                        "reconstructedDescriptor": reconstructed_descriptor,
                        "defaultResolution": default_resolution,
                        "numericOracleSamples": samples,
                        "blockers": blockers,
                    }
                    semantic_distribution_rows.append(inline)
                distribution_decisions[
                    semantic_distribution_rows[-1]["decision"]
                ] += 1
                consumed_distribution_ids.add(
                    payload_distribution["payloadDistributionId"]
                )
            require(not any(payload_by_path.values()),
                    f"unmatched distribution payload remains: {module_id}")

            for prop in properties:
                nested_property_blockers = sorted({
                    blocker
                    for row in semantic_distribution_rows
                    if row["topLevelPropertyPath"] == prop["propertyPath"]
                    for blocker in row["blockers"]
                })
                if prop["decision"] != "VERIFIED_IRRELEVANT":
                    prop["blockers"] = sorted({
                        *prop["blockers"], *nested_property_blockers,
                    })
                    if prop["blockers"]:
                        prop["decision"] = "BLOCKED"
            for leaf in leaves:
                parent = property_by_path[leaf["topLevelPropertyPath"]]
                leaf["decision"] = parent["decision"]
                leaf["blockers"] = copy.deepcopy(parent["blockers"])

            native = module["nativeTail"]
            if module["sourceDocument"] == "externalModuleClosure":
                external_native_occurrences += 1
                native_adapter = {
                    "decision": "VERIFIED_IRRELEVANT",
                    "oracleId": "ue3.current-export.serial-equals-property-end.v1",
                    "evidenceObjectId": module["sourceObjectId"],
                    "sourceFidelity": "CURRENT_REVISION_EVIDENCE_NOT_SOURCE_EXACT",
                }
            else:
                require(native["classification"] == "VERIFIED_IRRELEVANT",
                        f"internal native tail changed: {module_id}")
                native_adapter = {
                    "decision": "VERIFIED_IRRELEVANT",
                    "oracleId": native["oracleId"],
                    "sourceFidelity": "SOURCE_EXACT",
                }

            seed = None
            if module.get("seed") is not None:
                seed = parse_seed(payload, exact_class, defaults)
                seed_count += 1
            implicit_defaults = [
                default_adapter(row, defaults)
                for row in module.get("implicitDefaults", [])
            ]
            default_decisions.update(row["decision"] for row in implicit_defaults)

            class_blocked = exact_handler_needs_oracle(exact_class)
            nested_blockers = sorted({
                blocker
                for row in semantic_distribution_rows
                for blocker in row["blockers"]
            })
            blockers = sorted({
                *( [CUSTOM_HANDLER_BLOCKER] if class_blocked else [] ),
                *nested_blockers,
            })
            module_decision = "BLOCKED" if blockers else "READY_FOR_HANDLER"
            module_decisions[module_decision] += 1
            transport_literals = [
                row["literalId"] for row in literals
                if row["literalId"] not in consumed_literal_ids
            ]
            for literal_id in transport_literals:
                literal = next(row for row in literals if row["literalId"] == literal_id)
                require(str(literal["propertyPath"]).casefold().endswith(".sourceobjectpath"),
                        f"unclassified typed literal: {literal_id}")
                consumed_literal_ids.add(literal_id)
            module_rows.append({
                "moduleOccurrenceId": module_id,
                "order": module["order"],
                "sourceObjectId": module["sourceObjectId"],
                "sourceRecordSha256": module["sourceRecordSha256"],
                "exactSourceClass": exact_class,
                "normalizedAliasAllowed": False,
                "handlerCapabilityId": (
                    "source.module.exact." + sanitized_id(exact_class) + ".v1"
                ),
                "decision": module_decision,
                "blockers": blockers,
                "typedPayload": {
                    "stableId": stable_id,
                    "className": payload["className"],
                    "objectPath": payload["objectPath"],
                    "literals": literals,
                    "distributions": distributions,
                    "payloadSha256": canonical_sha256({
                        "stableId": stable_id,
                        "className": payload["className"],
                        "objectPath": payload["objectPath"],
                        "literals": literals,
                        "distributions": distributions,
                    }),
                },
                "properties": properties,
                "primitiveLeaves": leaves,
                "distributionAdapters": semantic_distribution_rows,
                "transportLiteralIds": transport_literals,
                "nativeTail": native_adapter,
                "seed": seed,
                "implicitDefaults": implicit_defaults,
            })
        occurrence_rows.append({
            "occurrenceCompositeId": semantic_occurrence["occurrenceCompositeId"],
            "evidenceId": semantic_occurrence["evidenceId"],
            "sourceOccurrenceId": semantic_occurrence["sourceOccurrenceId"],
            "sourceSystemId": semantic_occurrence["sourceSystemId"],
            "sourceEmitterPath": semantic_occurrence["sourceEmitterPath"],
            "rendererType": semantic_occurrence["rendererType"],
            "selectedLod": {
                "sourceLodPath": semantic_occurrence["selectedLodSemantics"][
                    "sourceLodPath"
                ],
                "sourceLodNodeId": semantic_occurrence["selectedLodSemantics"][
                    "sourceLodNodeId"
                ],
                "sourceLodRecordSha256": semantic_occurrence[
                    "selectedLodSemantics"
                ]["sourceLodRecordSha256"],
                "fields": [
                    {
                        "fieldId": row["fieldId"],
                        "fieldName": row["fieldName"],
                        "decision": "VERIFIED_IRRELEVANT",
                        "oracleId": "artist-f.first-lod-selection-compiled.v1",
                    }
                    for row in semantic_occurrence["selectedLodSemantics"]["fields"]
                ],
            },
            "actionCueParameterInputs": [
                parameters_by_cue.get(
                    next((row["sourceCueId"] for row in local.get(
                        "distributionOccurrences", []
                    ) if row["activeElementId"] == semantic_occurrence["evidenceId"]), ""),
                    {},
                )[name]
                for name in sorted(parameters_by_cue.get(
                    next((row["sourceCueId"] for row in local.get(
                        "distributionOccurrences", []
                    ) if row["activeElementId"] == semantic_occurrence["evidenceId"]), ""),
                    {},
                ))
            ],
            "modules": module_rows,
        })

    expected_local = {str(row["occurrenceId"]) for row in local.get(
        "distributionOccurrences", []
    )}
    require(consumed_local_occurrences == expected_local,
            "local distribution occurrence consumption changed")
    require(payload_literal_ids == consumed_literal_ids,
            "typed literal consumption is incomplete")
    require(payload_distribution_ids == consumed_distribution_ids,
            "typed distribution consumption is incomplete")

    point_light = point_light_adapter(local)
    handler_catalog = []
    for class_name in sorted(handler_classes):
        blocked = exact_handler_needs_oracle(class_name)
        handler_catalog.append({
            "exactSourceClass": class_name,
            "occurrenceCount": handler_classes[class_name],
            "handlerCapabilityId": (
                "source.module.exact." + sanitized_id(class_name) + ".v1"
            ),
            "normalizedAliasAllowed": False,
            "decision": "BLOCKED" if blocked else "READY_FOR_HANDLER",
            "blockers": [CUSTOM_HANDLER_BLOCKER] if blocked else [],
        })

    modules = [row for occurrence in occurrence_rows for row in occurrence["modules"]]
    properties = [row for module in modules for row in module["properties"]]
    leaves = [row for module in modules for row in module["primitiveLeaves"]]
    distributions = [
        row for module in modules for row in module["distributionAdapters"]
    ]
    defaults_rows = [row for module in modules for row in module["implicitDefaults"]]
    selected_lod = [
        row for occurrence in occurrence_rows for row in occurrence["selectedLod"]["fields"]
    ]
    module_decisions = Counter(row["decision"] for row in modules)
    property_decisions = Counter(row["decision"] for row in properties)
    leaf_decisions = Counter(row["decision"] for row in leaves)
    distribution_decisions = Counter(row["decision"] for row in distributions)
    default_decisions = Counter(row["decision"] for row in defaults_rows)
    default_dependent_rows = [
        row for row in distributions if row.get("defaultDependent") is True
    ]
    default_dependent_blocked = [
        row for row in default_dependent_rows if row["decision"] == "BLOCKED"
    ]
    default_dependent_reconstructed = [
        row for row in default_dependent_rows
        if row["decision"] == "READY_FOR_HANDLER"
    ]
    require(len(default_dependent_rows) == EXPECTED_DEFAULT_DEPENDENT_COUNT,
            "built default-dependent denominator changed")
    expected_reconstructed_count = (
        EXPECTED_SPAWN_RATESCALE_DEFAULT_DEPENDENT_COUNT
        if default_dependent_policy == CURRENT_ENGINE_CDO_POLICY else 0
    )
    require(len(default_dependent_reconstructed) == expected_reconstructed_count
            and len(default_dependent_blocked)
            == EXPECTED_DEFAULT_DEPENDENT_COUNT - expected_reconstructed_count,
            "default-dependent policy result denominator changed")
    parameter_spawn_module = next(
        row for row in occurrence_rows[EXPECTED_SPAWN_PARAMETER_RATE_OCCURRENCE][
            "modules"
        ] if row["exactSourceClass"] == "particlemodulespawn"
    )
    parameter_spawn_rate = next(
        row for row in parameter_spawn_module["distributionAdapters"]
        if row["propertyPath"] == "rate"
    )
    require(
        parameter_spawn_rate.get("exactSourceClass")
        == "distributionfloatparticleparameter"
        and parameter_spawn_rate["decision"] == "READY_FOR_HANDLER"
        and len(parameter_spawn_rate["numericOracleSamples"]) == 1
        and parameter_spawn_rate["numericOracleSamples"][0]["branch"]
        == "PARAMETER_INPUT"
        and parameter_spawn_rate["numericOracleSamples"][0]["parameterInput"]
        == {
            "name": "Spawn", "kind": "scalar", "value": 0.0,
            "sourceIndex": 0, "sourceValueByteOffset": 494,
        }
        and parameter_spawn_rate["numericOracleSamples"][0]["value"] == [0.0],
        "Full35 occurrence #34 Spawn parameter Rate branch changed",
    )
    custom_occurrences = sum(
        handler_classes[row["exactSourceClass"]]
        for row in handler_catalog if row["decision"] == "BLOCKED"
    )
    measured = {
        "occurrenceCount": len(occurrence_rows),
        "selectedLodFieldCount": len(selected_lod),
        "moduleCount": len(modules),
        "propertyCount": len(properties),
        "primitiveLeafCount": len(leaves),
        "distributionCount": len(distributions),
        "inlineDistributionCount": sum(
            not row.get("legacyOccurrenceId") for row in distributions
        ),
        "localDistributionDefinitionCount": len(local_definitions),
        "localDistributionOccurrenceCount": len(local_rows),
        "pointLightOccurrenceCount": 1,
        "nativeTailCount": len(modules),
        "externalNativeTailOccurrenceCount": external_native_occurrences,
        "seedCount": seed_count,
        "implicitDefaultCount": len(defaults_rows),
        "transportLiteralCount": len(payload_literal_ids) - len(leaves),
        "exactClassCount": len(handler_catalog),
        "customOrSeededClassOccurrenceCount": custom_occurrences,
    }
    require(measured == EXPECTED, f"source execution denominator changed: {measured}")

    blocker_union = sorted({
        PRODUCT_OWNER_BLOCKER,
        *(blocker for row in handler_catalog for blocker in row["blockers"]),
        *(blocker for row in local_rows for blocker in row["blockers"]),
        *(blocker for row in modules for blocker in row["blockers"]),
    })
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": "ACTIVE_OCCURRENCES_FIRST_LOD_ONLY",
        "provenancePolicy": {
            "sourceExactNeverInferredFromCurrentRevision": True,
            "currentRevisionDefaultsAreExecutionReconstructionOnly": True,
            "customClassAliasRequiresNumericOracle": True,
            "unknownAndUnconsumedRowsFailClosed": True,
        },
        "defaultDependentPolicy": default_policy,
        "full35DefaultDependentGate": full35_default_gate,
        "inputs": input_rows,
        "toolIdentities": {
            "generator": {
                "path": "Tools/LevelPlacementExtractor/"
                        "build_artist_31470_source_execution_semantics.py",
                "canonicalTextSha256": canonical_text_sha256(Path(__file__)),
            },
            "sourceContractIo": {
                "path": "Tools/LevelPlacementExtractor/effect_source_contract_io.py",
                "canonicalTextSha256": canonical_text_sha256(
                    Path(__file__).with_name("effect_source_contract_io.py")
                ),
            },
        },
        "currentRevisionDefaultEvidence": defaults,
        "externalNativeTailEvidence": native_evidence,
        "handlerCapabilities": handler_catalog,
        "pointLightAdapter": point_light,
        "localDistributionAdapters": local_rows,
        "occurrences": occurrence_rows,
        "summary": {
            "denominators": measured,
            "moduleDecisionCounts": dict(sorted(module_decisions.items())),
            "propertyDecisionCounts": dict(sorted(property_decisions.items())),
            "primitiveLeafDecisionCounts": dict(sorted(leaf_decisions.items())),
            "distributionDecisionCounts": dict(sorted(distribution_decisions.items())),
            "defaultDependentDistributionCount": len(default_dependent_rows),
            "defaultDependentBlockedCount": len(default_dependent_blocked),
            "defaultDependentReconstructedCount": len(
                default_dependent_reconstructed
            ),
            "selectedLodDecisionCounts": {"VERIFIED_IRRELEVANT": len(selected_lod)},
            "nativeTailDecisionCounts": {"VERIFIED_IRRELEVANT": len(modules)},
            "seedDecisionCounts": {"READY_FOR_HANDLER": seed_count},
            "implicitDefaultDecisionCounts": dict(sorted(default_decisions.items())),
            "externalCurrentUniqueObjectCount": native_evidence["uniqueObjectCount"],
            "typedPayloadLiteralCount": len(payload_literal_ids),
            "typedPayloadDistributionCount": len(payload_distribution_ids),
            "allRowsClassifiedAndBound": True,
            "allRowsConsumedOrIrrelevant": False,
            "unclassifiedRowCount": 0,
            "silentFallbackCount": 0,
            "sourceAdapterExecutionReady": not any(
                row["decision"] == "BLOCKED" for row in modules
            ),
        },
        "blockerUnion": list(blocker_union),
        "productAdmission": {
            "allowed": False,
            "blockers": list(blocker_union),
        },
    }
    receipt["receiptSha256"] = canonical_sha256(receipt)
    validate_receipt(receipt)
    return receipt


def validate_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "execution receipt schema changed")
    require(type(receipt.get("formatVersion")) is int
            and receipt["formatVersion"] == FORMAT_VERSION,
            "execution receipt version changed")
    unsigned = copy.deepcopy(receipt)
    expected_hash = str(unsigned.pop("receiptSha256", ""))
    require(expected_hash and canonical_sha256(unsigned) == expected_hash,
            "execution receipt self hash changed")
    require(receipt.get("productAdmission", {}).get("allowed") is False,
            "execution receipt granted Product admission")
    require(receipt.get("summary", {}).get("denominators") == EXPECTED,
            "execution receipt denominator changed")
    require(receipt.get("summary", {}).get("allRowsClassifiedAndBound") is True
            and receipt.get("summary", {}).get("unclassifiedRowCount") == 0,
            "execution receipt has unclassified rows")
    require(receipt.get("summary", {}).get("allRowsConsumedOrIrrelevant") is False,
            "blocked custom rows were reported as consumed")
    require(receipt.get("summary", {}).get("silentFallbackCount") == 0,
            "execution receipt has silent fallback")
    require(PRODUCT_OWNER_BLOCKER in receipt.get("blockerUnion", []),
            "final Product owner blocker is missing")
    policy = receipt.get("defaultDependentPolicy") or {}
    policy_id = str(policy.get("policyId") or "")
    require(policy_id in ALLOWED_DEFAULT_POLICIES,
            "default-dependent policy is missing or unknown")
    require(policy == default_dependent_policy_receipt(
        policy_id, receipt["currentRevisionDefaultEvidence"]
    ), "default-dependent typed policy evidence changed")
    require(receipt.get("full35DefaultDependentGate") == {
        "defaultDependentCount": EXPECTED_DEFAULT_DEPENDENT_COUNT,
        "spawnDefaultDependentCount": EXPECTED_SPAWN_DEFAULT_DEPENDENT_COUNT,
        "spawnRateScaleDefaultDependentCount": (
            EXPECTED_SPAWN_RATESCALE_DEFAULT_DEPENDENT_COUNT
        ),
        "spawnRateScaleSourceDistributionKind": "RawDistributionFloat",
        "spawnRateDefaultDependentOccurrences": [
            EXPECTED_SPAWN_RATE_DEFAULT_DEPENDENT_OCCURRENCE
        ],
        "spawnExplicitZeroRateCount": 29,
        "spawnExplicitNonzeroRates": {
            str(index): value
            for index, value in sorted(EXPECTED_NONZERO_SPAWN_RATES.items())
        },
        "spawnParameterRateOccurrences": [
            EXPECTED_SPAWN_PARAMETER_RATE_OCCURRENCE
        ],
    }, "Full35 default-dependent gate changed")

    modules = [
        module for occurrence in receipt.get("occurrences", [])
        for module in occurrence.get("modules", [])
    ]
    distributions = [
        row for module in modules for row in module.get("distributionAdapters", [])
    ]
    default_rows: list[tuple[dict[str, Any], dict[str, Any]]] = []
    for module in modules:
        payload_by_id = {
            row["payloadDistributionId"]: row["descriptor"]
            for row in module["typedPayload"]["distributions"]
        }
        for row in module["distributionAdapters"]:
            require(isinstance(row.get("semanticClosureExecutionBlockers"), list)
                    and row.get("defaultDependent") in {True, False},
                    "semantic closure distribution evidence was discarded")
            if row["defaultDependent"] is not True:
                continue
            default_rows.append((module, row))
            require(not row.get("legacyOccurrenceId"),
                    "local distribution became default-dependent")
            require(row.get("sourceNumericPayloadByteCount") == 0
                    and row.get("sourceNumericPayloadDefinition")
                    == "LOOKUP_TABLE_OR_KEY_VALUE_BYTES"
                    and DEFAULT_DEPENDENT_BLOCKER
                    in row["semanticClosureExecutionBlockers"],
                    "null RawDistribution numeric evidence changed")
            descriptor = payload_by_id[row["payloadDistributionId"]]
            require(
                descriptor.get("lookupTable") == []
                and descriptor.get("keys") == []
                and int(descriptor.get("lookupTableChunkSize", -1)) == 0
                and int(descriptor.get("lookupTableNumElements", -1)) == 0
                and is_zero_vector(descriptor.get("defaultMinimum"))
                and is_zero_vector(descriptor.get("defaultMaximum")),
                "null RawDistribution was replaced by numeric payload",
            )
            use_current_cdo = (
                policy_id == CURRENT_ENGINE_CDO_POLICY
                and module["exactSourceClass"] == "particlemodulespawn"
                and row["propertyPath"] == "ratescale"
            )
            if use_current_cdo:
                expected_descriptor, expected_evidence = (
                    current_engine_spawn_ratescale_reconstruction(
                        receipt["currentRevisionDefaultEvidence"]
                    )
                )
                require(
                    row["decision"] == "READY_FOR_HANDLER"
                    and row["blockers"] == []
                    and row["reconstructedDescriptor"] == expected_descriptor
                    and row["defaultResolution"] == expected_evidence
                    and row["numericOracleSamples"]
                    == numeric_samples(expected_descriptor)
                    and row["fieldProvenance"]["reconstructionBasis"]
                    == CURRENT_ENGINE_CDO_POLICY,
                    "current Engine CDO reconstruction evidence changed",
                )
            else:
                expected_blockers = sorted({
                    *row["semanticClosureExecutionBlockers"],
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
                        "provenanceTier": "SOURCE_NULL_RAW_DISTRIBUTION",
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

        adapter_rows = module["distributionAdapters"]
        for prop in module["properties"]:
            nested = sorted({
                blocker for row in adapter_rows
                if row["topLevelPropertyPath"] == prop["propertyPath"]
                for blocker in row["blockers"]
            })
            if nested and prop["decision"] != "VERIFIED_IRRELEVANT":
                require(prop["decision"] == "BLOCKED"
                        and set(nested).issubset(prop["blockers"]),
                        "default-dependent property blocker propagation changed")
        property_by_path = {
            row["propertyPath"]: row for row in module["properties"]
        }
        for leaf in module["primitiveLeaves"]:
            parent = property_by_path[leaf["topLevelPropertyPath"]]
            require(leaf["decision"] == parent["decision"]
                    and leaf["blockers"] == parent["blockers"],
                    "property blocker did not reach primitive leaf")
        expected_module_blockers = sorted({
            *([CUSTOM_HANDLER_BLOCKER] if exact_handler_needs_oracle(
                module["exactSourceClass"]
            ) else []),
            *(blocker for row in adapter_rows for blocker in row["blockers"]),
        })
        require(module["blockers"] == expected_module_blockers
                and module["decision"] == (
                    "BLOCKED" if expected_module_blockers
                    else "READY_FOR_HANDLER"
                ), "default-dependent module blocker propagation changed")

    require(len(default_rows) == EXPECTED_DEFAULT_DEPENDENT_COUNT,
            "default-dependent receipt denominator changed")
    reconstructed = [
        row for _, row in default_rows if row["decision"] == "READY_FOR_HANDLER"
    ]
    expected_reconstructed = (
        EXPECTED_SPAWN_RATESCALE_DEFAULT_DEPENDENT_COUNT
        if policy_id == CURRENT_ENGINE_CDO_POLICY else 0
    )
    require(len(reconstructed) == expected_reconstructed,
            "default-dependent reconstructed denominator changed")
    require(receipt["summary"]["defaultDependentDistributionCount"]
            == EXPECTED_DEFAULT_DEPENDENT_COUNT
            and receipt["summary"]["defaultDependentReconstructedCount"]
            == expected_reconstructed
            and receipt["summary"]["defaultDependentBlockedCount"]
            == EXPECTED_DEFAULT_DEPENDENT_COUNT - expected_reconstructed,
            "default-dependent summary changed")
    require(receipt["summary"]["moduleDecisionCounts"] == dict(sorted(
        Counter(row["decision"] for row in modules).items()
    )) and receipt["summary"]["distributionDecisionCounts"] == dict(sorted(
        Counter(row["decision"] for row in distributions).items()
    )), "default-dependent decision summary changed")
    expected_union = sorted({
        PRODUCT_OWNER_BLOCKER,
        *(blocker for row in receipt["handlerCapabilities"]
          for blocker in row["blockers"]),
        *(blocker for row in receipt["localDistributionAdapters"]
          for blocker in row["blockers"]),
        *(blocker for row in modules for blocker in row["blockers"]),
    })
    require(receipt["blockerUnion"] == expected_union
            and receipt["productAdmission"]["blockers"] == expected_union,
            "default-dependent blocker union changed")


def default_paths(root: Path) -> dict[str, Path]:
    return {name: root / relative for name, relative in TRACKED_INPUTS.items()}


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    paths = default_paths(root)
    parser = argparse.ArgumentParser()
    for name, path in paths.items():
        parser.add_argument("--" + name.replace("_", "-"), type=Path, default=path)
    parser.add_argument(
        "--release-root", type=Path,
        default=Path(r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC"),
    )
    parser.add_argument(
        "--output", type=Path,
        default=root / "Data/Effects/Imported/Artist/Candidates/"
        "skill.31470.source-execution-semantics.receipt.json",
    )
    parser.add_argument(
        "--default-dependent-policy",
        choices=sorted(ALLOWED_DEFAULT_POLICIES),
        default=FAIL_CLOSED_DEFAULT_POLICY,
    )
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    inputs = {
        name: load_strict_json_object(getattr(args, name)) for name in paths
    }
    result = build_receipt(
        root=root, release_root=args.release_root, inputs=inputs,
        default_dependent_policy=args.default_dependent_policy,
    )
    if args.check:
        current = load_strict_json_object(args.output)
        validate_receipt(current)
        if json_bytes(current) != json_bytes(result):
            raise ValueError("source execution semantics receipt is stale")
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        temporary = args.output.with_suffix(args.output.suffix + ".tmp")
        temporary.write_bytes(json_bytes(result))
        temporary.replace(args.output)
    print(
        "Artist F source execution semantics: "
        f"modules={len([m for o in result['occurrences'] for m in o['modules']])} "
        f"ready={result['summary']['moduleDecisionCounts'].get('READY_FOR_HANDLER', 0)} "
        f"blocked={result['summary']['moduleDecisionCounts'].get('BLOCKED', 0)} "
        f"distributions={result['summary']['denominators']['distributionCount']} "
        f"product={str(result['productAdmission']['allowed']).lower()}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"Artist F source execution semantics failed: {error}", file=sys.stderr)
        raise SystemExit(1)
