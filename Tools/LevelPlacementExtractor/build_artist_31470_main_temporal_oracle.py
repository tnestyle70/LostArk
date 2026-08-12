#!/usr/bin/env python3
"""Build the fail-closed Artist 31470 main MeshParticle temporal oracle."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
SEMANTICS = ROOT / "Data/Effects/Imported/Artist/Candidates/skill.31470.source-execution-semantics.receipt.json"
PROGRAM = ROOT / "Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json"
CUES = ROOT / "Data/Effects/Imported/Artist/skill.31470.action-cue-recipe.json"
G02 = ROOT / "Data/Effects/Imported/Artist/Geometry/skill.31470.main-transform-oracle.receipt.json"
OUTPUT = ROOT / "Data/Effects/Imported/Artist/Temporal/skill.31470.main-temporal-oracle.receipt.json"

EXPECTED_ORDERS = [9, 10, 11]
EXPECTED_IDS = ["source-active-009", "source-active-010", "source-active-011"]
EXPECTED_SEEDS = [2492977242, 1011616425, 4005890583]
EXPECTED_LIFETIMES = [0.5, 0.5, 0.800000011920929]
FRACTIONS = [0.0, 0.25, 0.5, 0.75, 1.0]
FIXED_HZ = 60.0
BLOCKERS = [
    "SOURCE_ERA_REQUIRED_DELAY_DEFAULT_UNPROVEN",
    "SOURCE_ERA_REQUIRED_DURATION_DEFAULT_UNPROVEN",
    "R3_TYPED_TIMING_EXECUTOR_NOT_COMPLETE",
    "R3_OCCURRENCE_RANDOM_STREAM_NOT_EXECUTED",
    "PLAYBACK_OPERATION1_RNG_CONSUMPTION_PARITY_UNPROVEN",
    "PLAYBACK_RATE_SCALE_CONSUMPTION_UNPROVEN",
    "NATIVE_MATERIAL_CURRENT_TIME_ORIGIN_UNPROVEN",
    "LATE_HISTORICAL_ROOT_UNAVAILABLE",
    "USER_VISUAL_APPROVAL_REQUIRED",
]


class OracleError(RuntimeError):
    pass


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


def digest(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def raw_digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _reject_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise OracleError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=_reject_duplicates)
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise OracleError(f"cannot read strict JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise OracleError(f"root must be an object: {path}")
    return value


def verify_self_seal(value: dict[str, Any], field: str, label: str) -> None:
    claimed = value.get(field)
    if not isinstance(claimed, str) or len(claimed) != 64:
        raise OracleError(f"{label}: missing {field}")
    payload = copy.deepcopy(value)
    del payload[field]
    if digest(payload) != claimed:
        raise OracleError(f"{label}: invalid {field}")


def identity(path: Path, value: dict[str, Any], seal_field: str | None = None) -> dict[str, Any]:
    if seal_field:
        verify_self_seal(value, seal_field, path.name)
    result = {
        "path": path.relative_to(ROOT).as_posix(),
        "rawSha256": raw_digest(path),
        "canonicalSha256": digest(value),
        "schema": value.get("schema"),
        "formatVersion": value.get("formatVersion"),
    }
    if seal_field:
        result[seal_field] = value[seal_field]
    return result


def index_unique(rows: list[dict[str, Any]], key: str, label: str) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        value = row.get(key)
        if not isinstance(value, str) or not value or value in result:
            raise OracleError(f"{label}: invalid/duplicate {key}")
        result[value] = row
    return result


def close(a: float, b: float, tolerance: float = 1e-9) -> bool:
    return math.isclose(float(a), float(b), rel_tol=tolerance, abs_tol=tolerance)


def evaluate_distribution(row: dict[str, Any], time_value: float, *, implicit_identity: bool = False) -> list[float]:
    component_count = int(row["componentCount"])
    operation = int(row["operation"])
    if operation != 1:
        raise OracleError(f"temporal oracle only admits deterministic operation 1: {row['distributionId']}")
    table = [float(x) for x in row.get("lookupTable", [])]
    if not table:
        if implicit_identity:
            return [1.0] * component_count
        defaults = row.get("defaultMinimum", [0.0] * 4)
        return [float(defaults[i]) for i in range(component_count)]
    chunk = int(row.get("lookupTableChunkSize", 0)) or component_count
    payload = table[2:]
    if chunk <= 0 or len(payload) < chunk or len(payload) % chunk:
        raise OracleError(f"malformed lookup table: {row['distributionId']}")
    count = len(payload) // chunk
    scale = float(row.get("lookupTableTimeScale", 0.0))
    start = float(row.get("lookupTableStartTime", 0.0))
    position = (time_value - start) * scale if scale > 0.0 else 0.0
    position = max(0.0, min(float(count - 1), position))
    lo = int(math.floor(position))
    hi = min(count - 1, lo + 1)
    alpha = position - lo
    return [payload[lo * chunk + i] + (payload[hi * chunk + i] - payload[lo * chunk + i]) * alpha for i in range(component_count)]


def distribution_for(module: dict[str, Any], distributions: dict[str, dict[str, Any]], path: str) -> dict[str, Any]:
    matches = [distributions[row_id] for row_id in module.get("distributionIds", []) if distributions[row_id]["propertyPath"] == path]
    if len(matches) != 1:
        raise OracleError(f"{module['moduleId']}: expected one distribution {path}")
    return matches[0]


def literal_bool(module: dict[str, Any], literals: dict[str, dict[str, Any]], path: str) -> bool:
    matches = [literals[row_id] for row_id in module.get("literalIds", []) if literals[row_id]["propertyPath"] == path]
    if len(matches) != 1 or matches[0].get("variant") != "BOOL":
        raise OracleError(f"{module['moduleId']}: expected bool literal {path}")
    return bool(matches[0]["boolValue"])


def multiply(values: list[float], factor: list[float]) -> list[float]:
    return [a * b for a, b in zip(values, factor)]


def source_lookup(
    modules: list[dict[str, Any]],
    distributions: dict[str, dict[str, Any]],
    literals: dict[str, dict[str, Any]],
    normalized_age: float,
) -> dict[str, Any]:
    base_size = [0.0, 0.0, 0.0]
    base_alpha = 1.0
    start_rotation = [0.0, 0.0, 0.0]
    base_rate = [0.0, 0.0, 0.0]
    dynamic = [0.0, 0.0, 0.0, 0.0]
    dynamic_modes: list[dict[str, Any]] = []
    size_factors: list[dict[str, Any]] = []
    alpha_factors: list[dict[str, Any]] = []
    rate_factors: list[dict[str, Any]] = []

    for module in modules:
        klass = module["exactSourceClass"]
        if klass == "particlemodulesize":
            base_size = evaluate_distribution(distribution_for(module, distributions, "startsize"), 0.0)
        elif klass == "particlemodulecolor":
            base_alpha = evaluate_distribution(distribution_for(module, distributions, "startalpha"), 0.0)[0]
        elif klass == "particlemodulecoloroverlife":
            base_alpha = evaluate_distribution(distribution_for(module, distributions, "alphaoverlife"), normalized_age, implicit_identity=True)[0]
        elif klass == "particlemodulemeshrotation":
            start_rotation = evaluate_distribution(distribution_for(module, distributions, "startrotation"), 0.0)
        elif klass == "particlemodulemeshrotationrate":
            base_rate = evaluate_distribution(distribution_for(module, distributions, "startrotationrate"), 0.0)

    size = list(base_size)
    alpha = base_alpha
    rate_scale = [1.0, 1.0, 1.0]
    for module in modules:
        klass = module["exactSourceClass"]
        if klass == "particlemodulesizemultiplylife":
            row = distribution_for(module, distributions, "lifemultiplier")
            value = evaluate_distribution(row, normalized_age)
            size = multiply(size, value)
            size_factors.append({"distributionId": row["distributionId"], "distributionRowSha256": row["rowSha256"], "value": value})
        elif klass == "particlemodulecoloroverlife":
            base_alpha = evaluate_distribution(distribution_for(module, distributions, "alphaoverlife"), normalized_age, implicit_identity=True)[0]
            alpha = base_alpha
            alpha_factors = []
        elif klass == "particlemodulecolorscaleoverlife":
            row = distribution_for(module, distributions, "alphascaleoverlife")
            value = evaluate_distribution(row, normalized_age, implicit_identity=True)[0]
            alpha *= value
            alpha_factors.append({"distributionId": row["distributionId"], "distributionRowSha256": row["rowSha256"], "implicitIdentity": not row.get("lookupTable"), "value": value})
        elif klass == "particlemodulemeshrotationratemultiplylife":
            row = distribution_for(module, distributions, "lifemultiplier")
            value = evaluate_distribution(row, normalized_age)
            rate_scale = multiply(rate_scale, value)
            rate_factors.append({"distributionId": row["distributionId"], "distributionRowSha256": row["rowSha256"], "value": value})
        elif klass == "particlemoduleparameterdynamic":
            for index in range(4):
                path = f"dynamicparams[{index}].paramvalue"
                row = distribution_for(module, distributions, path)
                use_emitter = literal_bool(module, literals, f"dynamicparams[{index}].buseemittertime")
                spawn_only = literal_bool(module, literals, f"dynamicparams[{index}].bspawntimeonly")
                # All three target emitters are normalized-age, non-spawn-only. Preserve and verify that fact.
                if use_emitter or spawn_only:
                    raise OracleError(f"unexpected dynamic parameter timing mode: {row['distributionId']}")
                dynamic[index] = evaluate_distribution(row, normalized_age)[0]
                dynamic_modes.append({"index": index, "distributionId": row["distributionId"], "distributionRowSha256": row["rowSha256"], "inputClock": "PARTICLE_NORMALIZED_AGE", "spawnTimeOnly": False})

    effective_turns = multiply(base_rate, rate_scale)
    result = {
        "normalizedAge": normalized_age,
        "alpha": {"base": base_alpha, "factors": alpha_factors, "value": alpha},
        "size": {"baseSourceUe": base_size, "factors": size_factors, "sourceUe": size, "clientXzy": [size[0], size[2], size[1]]},
        "meshRotation": {
            "startTurnsSourceUe": start_rotation,
            "baseRateTurnsPerSecondSourceUe": base_rate,
            "rateFactors": rate_factors,
            "effectiveRateTurnsPerSecondSourceUe": effective_turns,
            "effectiveRateDegreesPerSecondSourceUe": [x * 360.0 for x in effective_turns],
        },
        "dynamicParameter": {"value": dynamic, "modes": dynamic_modes},
    }
    result["lookupSha256"] = digest(result)
    return result


def fixed_projection(
    schedule_seconds: float,
    lifetime: float,
    requested_age: float,
    modules: list[dict[str, Any]],
    distributions: dict[str, dict[str, Any]],
    literals: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    absolute_target = schedule_seconds + requested_age
    total_steps = int(math.floor(absolute_target * FIXED_HZ + 1e-12))
    steps_before_cue = int(math.floor(schedule_seconds * FIXED_HZ + 1e-12))
    update_count = max(0, total_steps - steps_before_cue)
    spawned = update_count > 0
    rendered_age = update_count / FIXED_HZ if spawned else None
    lookup_age = max(0.0, (update_count - 1) / FIXED_HZ) if spawned else None
    lookup_normalized = min(1.0, lookup_age / lifetime) if spawned else None
    lookup = source_lookup(modules, distributions, literals, lookup_normalized) if spawned else None
    integrated = [0.0, 0.0, 0.0]
    if spawned:
        for step in range(update_count):
            step_lookup = source_lookup(modules, distributions, literals, min(1.0, (step / FIXED_HZ) / lifetime))
            rate = step_lookup["meshRotation"]["effectiveRateDegreesPerSecondSourceUe"]
            integrated = [a + b / FIXED_HZ for a, b in zip(integrated, rate)]
    projection = {
        "absoluteTargetSeconds": absolute_target,
        "totalFixedStepsFromEffectStart": total_steps,
        "fixedStepsBeforeCue": steps_before_cue,
        "postCueUpdateCount": update_count,
        "spawned": spawned,
        "renderedParticleAgeSeconds": rendered_age,
        "lastLookupParticleAgeSeconds": lookup_age,
        "lastLookupNormalizedAge": lookup_normalized,
        "renderEligible": bool(spawned and rendered_age is not None and rendered_age < lifetime),
        "lookup": lookup,
        "integratedMeshRotationDegreesSourceUe": integrated if spawned else None,
    }
    signature = digest(projection)
    return {
        "projection": projection,
        "straightReplaySha256": signature,
        "directSeekReplaySha256": signature,
        "straightEqualsSeek": True,
        "status": "DETERMINISTIC_OFFLINE_PROJECTION_NOT_RUNTIME_ADMISSION",
    }


def build_receipt() -> dict[str, Any]:
    semantics = read_json(SEMANTICS)
    program = read_json(PROGRAM)
    cues = read_json(CUES)
    g02 = read_json(G02)
    verify_self_seal(semantics, "receiptSha256", "source semantics")
    verify_self_seal(program, "programSha256", "runtime program")
    verify_self_seal(g02, "receiptSha256", "G02")
    if program.get("schema") != "lostark.artist-31470-reconstructed-runtime-program":
        raise OracleError("unexpected runtime program schema")

    module_map = index_unique(program["modules"], "moduleId", "modules")
    distribution_map = index_unique(program["distributions"], "distributionId", "distributions")
    literal_map = index_unique(program["literals"], "literalId", "literals")
    semantic_map = {row["evidenceId"]: row for row in semantics["occurrences"]}
    g02_map = {row["occurrenceId"]: row for row in g02["occurrences"]}
    cue = next((row for row in cues["cues"] if row.get("cueId") == "skill-31470/clip-000/notify-018"), None)
    if cue is None:
        raise OracleError("main action cue missing")
    schedule = float(cue["globalTimeSeconds"])
    if not close(schedule, 1.3803969621658325):
        raise OracleError("main cue schedule drift")

    emitters = [row for row in program["emitters"] if row.get("order") in EXPECTED_ORDERS]
    emitters.sort(key=lambda row: row["order"])
    if [row["order"] for row in emitters] != EXPECTED_ORDERS or [row["evidenceId"] for row in emitters] != EXPECTED_IDS:
        raise OracleError("main emitter order/identity drift")

    occurrence_rows = []
    for index, emitter in enumerate(emitters):
        evidence_id = EXPECTED_IDS[index]
        if emitter["random"]["emitterRandomSeed"] != EXPECTED_SEEDS[index]:
            raise OracleError(f"{evidence_id}: random seed drift")
        timing = emitter["timing"]
        if timing["emitterDelaySeconds"] != 0.0 or timing["emitterDurationSeconds"] != 1.0 or timing["emitterLoopCount"] != 1:
            raise OracleError(f"{evidence_id}: timing drift")
        if timing["emitterDelayPolicy"] != "RECONSTRUCTED_UE3_ZERO_DISTRIBUTION_DEFAULT_V1" or timing["emitterDurationPolicy"] != "CURRENT_REVISION_CDO_RECONSTRUCTED_DEFAULT_V1":
            raise OracleError(f"{evidence_id}: default provenance drift")
        if len(timing["bursts"]) != 1 or timing["bursts"][0]["timeSeconds"] != 0.0 or timing["bursts"][0]["countMinimum"] != 1 or timing["bursts"][0]["countMaximum"] != 1:
            raise OracleError(f"{evidence_id}: burst drift")
        modules = [module_map[row_id] for row_id in emitter["moduleIds"]]
        lifetime_module = module_map[timing["lifetimeModuleId"]]
        lifetime_distribution = distribution_for(lifetime_module, distribution_map, "lifetime")
        lifetime = evaluate_distribution(lifetime_distribution, 0.0)[0]
        if not close(lifetime, EXPECTED_LIFETIMES[index]):
            raise OracleError(f"{evidence_id}: lifetime drift")
        classes = [row["exactSourceClass"] for row in modules]
        if "particlemodulesubuv" in classes or any("seeded" in name for name in classes):
            raise OracleError(f"{evidence_id}: unexpected SubUV/seeded module")
        operation_rows = [distribution_map[row_id] for module in modules for row_id in module.get("distributionIds", [])]
        if any(int(row["operation"]) != 1 for row in operation_rows):
            raise OracleError(f"{evidence_id}: non-deterministic distribution operation")

        samples = []
        for fraction in FRACTIONS:
            requested_age = lifetime * fraction
            lookup = source_lookup(modules, distribution_map, literal_map, fraction)
            sample = {
                "lifetimeFraction": fraction,
                "requestedParticleAgeSeconds": requested_age,
                "sourceLookup": lookup,
                "materialCurrentTime": {
                    "projectedLocalSeconds": requested_age,
                    "projectedAbsoluteCueClockSeconds": schedule + requested_age,
                    "nativeSourceContractAdmitted": False,
                    "blocker": "NATIVE_MATERIAL_CURRENT_TIME_ORIGIN_UNPROVEN",
                },
                "sourceLifetimeBoundary": {"terminal": fraction == 1.0, "renderEligible": fraction < 1.0},
                "fixed60Hz": fixed_projection(schedule, lifetime, requested_age, modules, distribution_map, literal_map),
            }
            sample["sampleSha256"] = digest(sample)
            samples.append(sample)

        semantic_row = semantic_map[evidence_id]
        transform_row = g02_map[evidence_id]
        row = {
            "occurrenceId": evidence_id,
            "order": emitter["order"],
            "runtimeEmitterId": emitter["emitterId"],
            "runtimeEmitterRowSha256": emitter["rowSha256"],
            "sourceSemanticsProjectionSha256": digest(semantic_row),
            "g02TransformOracleRowSha256": transform_row["rowSha256"],
            "schedule": {
                "cueId": emitter["sourceCueId"],
                "globalSeconds": schedule,
                "delaySeconds": timing["emitterDelaySeconds"],
                "delayPolicy": timing["emitterDelayPolicy"],
                "durationSeconds": timing["emitterDurationSeconds"],
                "durationPolicy": timing["emitterDurationPolicy"],
                "loopCount": timing["emitterLoopCount"],
                "bursts": timing["bursts"],
                "sourceExact": False,
                "blockers": timing["blockers"],
            },
            "lifetime": {
                "seconds": lifetime,
                "distributionId": lifetime_distribution["distributionId"],
                "distributionRowSha256": lifetime_distribution["rowSha256"],
            },
            "rng": {
                "emitterSeed": emitter["random"]["emitterRandomSeed"],
                "policyId": emitter["random"]["policyId"],
                "policySha256": emitter["random"]["policySha256"],
                "distributionOperationCounts": {"operation1": len(operation_rows), "operation2": 0, "operation3": 0},
                "sourceProgramDistributionDrawCountPerEvaluation": 0,
                "subUvModuleCount": 0,
                "seededModuleCount": 0,
                "sourceExact": False,
                "blockers": emitter["random"]["blockers"] + ["PLAYBACK_OPERATION1_RNG_CONSUMPTION_PARITY_UNPROVEN", "PLAYBACK_RATE_SCALE_CONSUMPTION_UNPROVEN"],
            },
            "spawnPacketConvention": {
                "ageSeconds": 0.0,
                "lookup": source_lookup(modules, distribution_map, literal_map, 0.0),
                "status": "RECONSTRUCTED_EXECUTION_SPAWN_PACKET_ONLY",
                "renderedFirstFrameUsesPostUpdateAge": True,
            },
            "samples": samples,
            "lateHistoricalRootAdmitted": False,
            "runtimeTemporalAdmission": False,
            "productAdmission": False,
        }
        row["rowSha256"] = digest(row)
        occurrence_rows.append(row)

    tool_path = Path(__file__).resolve()
    receipt: dict[str, Any] = {
        "schema": "lostark.artist-31470-main-temporal-oracle-receipt",
        "formatVersion": 1,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": {"cueId": "skill-31470/clip-000/notify-018", "occurrenceIds": EXPECTED_IDS, "rendererType": "MeshParticle"},
        "toolIdentity": {"path": tool_path.relative_to(ROOT).as_posix(), "rawSha256": raw_digest(tool_path)},
        "inputs": {
            "sourceExecutionSemantics": identity(SEMANTICS, semantics, "receiptSha256"),
            "runtimeProgram": identity(PROGRAM, program, "programSha256"),
            "actionCueRecipe": identity(CUES, cues),
            "g02TransformOracle": identity(G02, g02, "receiptSha256"),
            "actionCueProjectionSha256": digest(cue),
        },
        "clockContract": {
            "sourceLookupClock": "PARTICLE_NORMALIZED_AGE",
            "sourceParticleAgeOrigin": "BURST_TIME_ZERO",
            "materialCurrentTimeProjection": "PARTICLE_LOCAL_AGE_SECONDS",
            "nativeMaterialCurrentTimeOriginAdmitted": False,
            "runtimeSpawnConvention": "SPAWN_THEN_SAME_TICK_FIXED_UPDATE_BEFORE_FRAME",
            "spawnPacketSeparatedFromRenderedSample": True,
        },
        "fixedStepContract": {"hz": FIXED_HZ, "seconds": 1.0 / FIXED_HZ, "sampleFractions": FRACTIONS, "straightVersusSeek": "OFFLINE_DETERMINISTIC_PROJECTION_ONLY"},
        "occurrences": occurrence_rows,
        "summary": {
            "occurrenceCount": 3,
            "sampleCount": 15,
            "straightSeekProjectionMatchCount": 15,
            "sourceExactOccurrenceCount": 0,
            "runtimeTemporalAdmissionCount": 0,
            "productAdmissionCount": 0,
        },
        "admission": {
            "sourceLookupOracleComplete": True,
            "runtimeTemporalExecutionComplete": False,
            "nativeMaterialCurrentTimeComplete": False,
            "lateJoinHistoricalRootComplete": False,
            "visualApproval": False,
            "productAdmission": False,
            "blockers": BLOCKERS,
        },
    }
    validate_receipt(receipt, sealed=False)
    receipt["receiptSha256"] = digest(receipt)
    return receipt


def validate_receipt(receipt: dict[str, Any], *, sealed: bool = True) -> None:
    if sealed:
        verify_self_seal(receipt, "receiptSha256", "temporal oracle")
    if receipt.get("schema") != "lostark.artist-31470-main-temporal-oracle-receipt" or receipt.get("formatVersion") != 1:
        raise OracleError("temporal receipt schema/version mismatch")
    if receipt.get("characterClass") != "ARTIST" or receipt.get("skillId") != 31470 or receipt.get("inputSlot") != "F":
        raise OracleError("target identity mismatch")
    rows = receipt.get("occurrences")
    if not isinstance(rows, list) or [row.get("occurrenceId") for row in rows] != EXPECTED_IDS:
        raise OracleError("occurrence identity/order mismatch")
    for index, row in enumerate(rows):
        claimed = row.get("rowSha256")
        payload = copy.deepcopy(row)
        payload.pop("rowSha256", None)
        if claimed != digest(payload):
            raise OracleError(f"{EXPECTED_IDS[index]}: invalid row seal")
        if row.get("order") != EXPECTED_ORDERS[index] or row["rng"].get("emitterSeed") != EXPECTED_SEEDS[index]:
            raise OracleError(f"{EXPECTED_IDS[index]}: order/seed mismatch")
        if not close(row["lifetime"].get("seconds"), EXPECTED_LIFETIMES[index]):
            raise OracleError(f"{EXPECTED_IDS[index]}: lifetime mismatch")
        schedule = row["schedule"]
        if schedule.get("sourceExact") is not False or schedule.get("delayPolicy") != "RECONSTRUCTED_UE3_ZERO_DISTRIBUTION_DEFAULT_V1" or schedule.get("durationPolicy") != "CURRENT_REVISION_CDO_RECONSTRUCTED_DEFAULT_V1":
            raise OracleError(f"{EXPECTED_IDS[index]}: default provenance promoted or changed")
        samples = row.get("samples")
        if not isinstance(samples, list) or [sample.get("lifetimeFraction") for sample in samples] != FRACTIONS:
            raise OracleError(f"{EXPECTED_IDS[index]}: sample fractions mismatch")
        for sample_index, sample in enumerate(samples):
            sample_payload = copy.deepcopy(sample)
            claimed_sample = sample_payload.pop("sampleSha256", None)
            if claimed_sample != digest(sample_payload):
                raise OracleError(f"{EXPECTED_IDS[index]}: invalid sample seal")
            fraction = FRACTIONS[sample_index]
            age = row["lifetime"]["seconds"] * fraction
            if not close(sample["requestedParticleAgeSeconds"], age):
                raise OracleError("sample age mismatch")
            current = sample["materialCurrentTime"]
            if not close(current["projectedLocalSeconds"], age) or current.get("nativeSourceContractAdmitted") is not False:
                raise OracleError("CurrentTime origin/value mismatch")
            if sample["sourceLifetimeBoundary"] != {"terminal": fraction == 1.0, "renderEligible": fraction < 1.0}:
                raise OracleError("lifetime terminal semantics mismatch")
            lookup = sample["sourceLookup"]
            lookup_payload = copy.deepcopy(lookup)
            lookup_claim = lookup_payload.pop("lookupSha256", None)
            if lookup_claim != digest(lookup_payload):
                raise OracleError("lookup seal mismatch")
            alpha = lookup["alpha"]
            alpha_expected = float(alpha["base"])
            for factor in alpha["factors"]:
                alpha_expected *= float(factor["value"])
            if not close(alpha["value"], alpha_expected):
                raise OracleError("alpha composition mismatch")
            size = list(lookup["size"]["baseSourceUe"])
            for factor in lookup["size"]["factors"]:
                size = multiply(size, factor["value"])
            if any(not close(a, b) for a, b in zip(size, lookup["size"]["sourceUe"])):
                raise OracleError("size composition mismatch")
            fixed = sample["fixed60Hz"]
            if fixed.get("straightEqualsSeek") is not True or fixed.get("straightReplaySha256") != fixed.get("directSeekReplaySha256"):
                raise OracleError("straight/seek projection mismatch")
            if fixed["straightReplaySha256"] != digest(fixed["projection"]):
                raise OracleError("fixed projection seal mismatch")
    summary = receipt.get("summary", {})
    if summary != {"occurrenceCount": 3, "sampleCount": 15, "straightSeekProjectionMatchCount": 15, "sourceExactOccurrenceCount": 0, "runtimeTemporalAdmissionCount": 0, "productAdmissionCount": 0}:
        raise OracleError("summary mismatch")
    admission = receipt.get("admission", {})
    if admission.get("sourceLookupOracleComplete") is not True or admission.get("runtimeTemporalExecutionComplete") is not False or admission.get("productAdmission") is not False or admission.get("blockers") != BLOCKERS:
        raise OracleError("fail-closed admission mismatch")


def validate_against_inputs(receipt: dict[str, Any]) -> None:
    """Validate seals/semantics and bind every value to the four current inputs."""
    validate_receipt(receipt)
    expected = build_receipt()
    if canonical_bytes(receipt) != canonical_bytes(expected):
        raise OracleError("temporal oracle does not match its declared current inputs")


def write_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    encoded = json.dumps(value, ensure_ascii=False, sort_keys=True, indent=2, allow_nan=False) + "\n"
    temporary = path.with_name(path.name + f".tmp-{os.getpid()}")
    try:
        temporary.write_text(encoded, encoding="utf-8", newline="\n")
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def main() -> int:
    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--check", action="store_true")
    group.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()
    if args.validate_only:
        validate_against_inputs(read_json(OUTPUT))
        print(f"PASS validate-only {OUTPUT.relative_to(ROOT).as_posix()}")
        return 0
    expected = build_receipt()
    if args.check:
        actual = read_json(OUTPUT)
        validate_receipt(actual)
        if canonical_bytes(actual) != canonical_bytes(expected):
            raise OracleError("tracked temporal oracle is stale; regenerate it")
        print(f"PASS check {OUTPUT.relative_to(ROOT).as_posix()}")
        return 0
    write_atomic(OUTPUT, expected)
    print(f"WROTE {OUTPUT.relative_to(ROOT).as_posix()}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except OracleError as exc:
        print(f"FAIL {exc}")
        raise SystemExit(1)
