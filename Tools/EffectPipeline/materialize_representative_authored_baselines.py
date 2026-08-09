#!/usr/bin/env python3
"""Materialize source-backed Authored Effect baselines.

The materializer is deliberately one-way and fail-closed.  Imported evidence
and canonical source Effect documents are read-only.  A ready skill manifest
may copy admitted source carriers into separate Authored stage documents.
Mesh Particle carriers are never product output.  A fixed singleton Cascade
carrier may become a standalone Mesh/Sprite, while a particle-dependent Sprite
keeps its Sprite Particle renderer recipe.  Blocked manifests are validated and
reported without inventing replacement assets.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shlex
from typing import Any, Iterable


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DATA_ROOT = (REPOSITORY_ROOT / "Data").resolve()
AUTHORED_ROOT = (DATA_ROOT / "Effects/Authored").resolve()
CORRECTION_ROOT = (DATA_ROOT / "Effects/AuthoredCorrections").resolve()
RESOURCES_ROOT = (REPOSITORY_ROOT / "Client/Bin/Resources").resolve()
DEFAULT_SET_MANIFEST = CORRECTION_ROOT / (
    "representative-four.authored-materialization-set.json"
)

SET_SCHEMA = "lostark.effect-authored-materialization-set"
SET_VERSION = 1
SKILL_SCHEMA = "lostark.effect-authored-materialization"
SKILL_VERSION = 1
STATUS_SCHEMA = "lostark.effect-authored-materialization-status"
STATUS_VERSION = 1
PRODUCT_GATE_SCHEMA = "lostark.effect-authored-product-gate"
PRODUCT_GATE_VERSION = 1
EXTERNAL_APPROXIMATION_SCHEMA = "lostark.effect-authored-approximation-receipt"
EXTERNAL_APPROXIMATION_VERSION = 1
EXTERNAL_APPROXIMATION_CLASS_DIRECTORIES = {
    "LANCE_MASTER": "LanceMaster",
    "ARTIST": "Artist",
    "DIMENSIONMASTER": "DimensionMaster",
    "WARLORD": "Warlord",
}

STANDALONE_MESH = "standaloneMesh"
MESH_PARTICLE = "meshParticle"
STANDALONE_SPRITE = "standaloneSprite"
SPRITE_PARTICLE = "spriteParticle"
SOURCE_CLASSIFICATIONS = frozenset(
    {STANDALONE_MESH, MESH_PARTICLE, STANDALONE_SPRITE, SPRITE_PARTICLE}
)
MESH_CLASSIFICATIONS = frozenset({STANDALONE_MESH, MESH_PARTICLE})
SPRITE_CLASSIFICATIONS = frozenset({STANDALONE_SPRITE, SPRITE_PARTICLE})
SOURCE_CARRIER_POLICY = {
    STANDALONE_MESH: "materialize",
    MESH_PARTICLE: "convertCertifiedSingletonToStandaloneOnly",
    STANDALONE_SPRITE: "materialize",
    SPRITE_PARTICLE: "convertCertifiedSingletonOrPreserveRenderer",
}

CERTIFIED = "CERTIFIED"
PARTICLE_REQUIRED = "PARTICLE_REQUIRED"
UNKNOWN = "UNKNOWN"
CONVERSION_ELIGIBILITIES = frozenset(
    {CERTIFIED, PARTICLE_REQUIRED, UNKNOWN}
)
MODULES_NOT_APPLICABLE = "notApplicable"
MODULES_REMOVED_AFTER_CERTIFIED_CONVERSION = (
    "reviewedPerModuleRemoval"
)
MODULES_PRESERVED_FOR_PARTICLE_RENDERER = "preservedInSourceRecipe"
MODULES_PRODUCT_EXCLUDED = "productExcluded"

READY = "ready"
BLOCKED = "blocked"
PRESERVE_EXISTING = "preserveExisting"
SKILL_STATUSES = frozenset({READY, BLOCKED, PRESERVE_EXISTING})

RESOURCE_SLOT_ORDER = (
    "meshModel",
    "base",
    "noise",
    "mask",
    "emissive",
    "dissolve",
)

STABLE_TOKEN = re.compile(r"^[a-z0-9][a-z0-9._-]{0,255}$")
EFFECT_ASSET_ID = re.compile(r"^effect\.[a-z0-9][a-z0-9._-]{0,248}$")
SHA256 = re.compile(r"^[0-9a-f]{64}$")
STAGE_TIME_EPSILON = 1.0e-6


def _require_object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ValueError(f"{label} must be an object.")
    return value


def _require_array(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise ValueError(f"{label} must be an array.")
    return value


def _require_string(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value or len(value) > 512:
        raise ValueError(f"{label} must be a non-empty bounded string.")
    return value


def _require_stable_token(value: Any, label: str) -> str:
    token = _require_string(value, label)
    if STABLE_TOKEN.fullmatch(token) is None:
        raise ValueError(f"{label} is not a stable lowercase token.")
    return token


def _require_effect_id(value: Any, label: str) -> str:
    effect_id = _require_string(value, label)
    if EFFECT_ASSET_ID.fullmatch(effect_id) is None:
        raise ValueError(f"{label} is not a stable Effect asset ID.")
    return effect_id


def _require_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{label} must be a number.")
    result = float(value)
    if not math.isfinite(result):
        raise ValueError(f"{label} must be finite.")
    return result


def _require_non_negative_number(value: Any, label: str) -> float:
    result = _require_number(value, label)
    if result < 0.0:
        raise ValueError(f"{label} must not be negative.")
    return result


def _require_vector(
    value: Any,
    count: int,
    label: str,
    *,
    positive: bool = False,
) -> list[float]:
    if not isinstance(value, list) or len(value) != count:
        raise ValueError(f"{label} must contain exactly {count} numbers.")
    result = [_require_number(component, label) for component in value]
    if positive and any(component <= 0.0 for component in result):
        raise ValueError(f"{label} components must be positive.")
    return result


def _load_json(path: Path, label: str) -> dict[str, Any]:
    try:
        with path.open("r", encoding="utf-8") as stream:
            return _require_object(json.load(stream), label)
    except (OSError, json.JSONDecodeError) as error:
        raise ValueError(f"{label} could not be parsed: {error}") from error


def _canonical_json_sha256(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(
            value,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
        ).encode("utf-8")
    ).hexdigest()


def _validate_external_authored_approximation(
    *,
    character_class: str,
    skill_id: int,
    stage_index: int,
    target_effect_id: str,
    target_path: Path,
) -> dict[str, Any]:
    class_directory = EXTERNAL_APPROXIMATION_CLASS_DIRECTORIES.get(character_class)
    if class_directory is None:
        raise ValueError(
            "Blocked stage unexpectedly has an Authored output for an unsupported class."
        )
    receipt_path = (
        CORRECTION_ROOT
        / "Generated"
        / class_directory
        / f"{target_effect_id}.approximation-receipt.json"
    )
    if not receipt_path.is_file():
        raise ValueError(
            "Blocked stage Authored output has no matching external approximation receipt: "
            f"{target_path}"
        )
    document = _load_json(target_path, "external Authored approximation")
    receipt = _load_json(receipt_path, "external approximation receipt")
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 12
        or document.get("effectAssetId") != target_effect_id
    ):
        raise ValueError("External Authored approximation identity/schema drifted.")
    if (
        receipt.get("schema") != EXTERNAL_APPROXIMATION_SCHEMA
        or receipt.get("version") != EXTERNAL_APPROXIMATION_VERSION
        or receipt.get("targetEffectAssetId") != target_effect_id
        or receipt.get("characterClass") != character_class
        or receipt.get("productSkillId") != skill_id
        or receipt.get("stageIndex") != stage_index
    ):
        raise ValueError("External approximation receipt identity/schema drifted.")
    expected_authoring_path = target_path.relative_to(DATA_ROOT).as_posix()
    if receipt.get("targetAuthoringPath") != expected_authoring_path:
        raise ValueError("External approximation receipt target path drifted.")
    output = _require_object(receipt.get("output"), "external approximation output")
    document_hash = _canonical_json_sha256(document)
    if (
        output.get("documentSha256") != document_hash
        or output.get("particleCount") != 0
    ):
        raise ValueError("External approximation receipt/document hash drifted.")
    elements = _require_array(document.get("elements"), "external elements")
    if not elements or output.get("elementCount") != len(elements):
        raise ValueError("External approximation output Element count drifted.")
    for raw_element in elements:
        element = _require_object(raw_element, "external approximation Element")
        if element.get("kind") not in {"mesh", "sprite"}:
            raise ValueError("External approximation contains a non-standalone carrier.")
        source_recipe = _require_object(
            element.get("sourceRecipe"), "external approximation sourceRecipe"
        )
        if source_recipe.get("enabled") is not False or source_recipe.get("modules") != []:
            raise ValueError("External approximation retained an executable source recipe.")
    selection_policy = _require_object(
        receipt.get("selectionPolicy"), "external approximation selectionPolicy"
    )
    if (
        selection_policy.get("particleOutputAllowed") is not False
        or selection_policy.get("genericPlaceholderAllowed") is not False
        or selection_policy.get("crossSkillBorrowingAllowed") is not False
    ):
        raise ValueError("External approximation selection boundary drifted.")
    return {
        "stageIndex": stage_index,
        "targetEffectAssetId": target_effect_id,
        "status": "externalApproximationPresent",
        "receiptPath": receipt_path.relative_to(DATA_ROOT).as_posix(),
        "documentSha256": document_hash,
        "strictAdmissionStatus": BLOCKED,
    }


def _sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _resolve_data_path(value: Any, label: str, *, require_file: bool) -> Path:
    relative = _require_string(value, label).replace("\\", "/")
    if relative.startswith("/") or ":" in relative or ".." in relative.split("/"):
        raise ValueError(f"{label} escaped the Data root.")
    resolved = (DATA_ROOT / relative).resolve()
    try:
        resolved.relative_to(DATA_ROOT)
    except ValueError as error:
        raise ValueError(f"{label} escaped the Data root.") from error
    if require_file and not resolved.is_file():
        raise ValueError(f"{label} does not exist: {resolved}")
    return resolved


def _resolve_target_path(value: Any, effect_id: str, label: str) -> Path:
    resolved = _resolve_data_path(value, label, require_file=False)
    if resolved.parent != AUTHORED_ROOT:
        raise ValueError(f"{label} must be a direct Authored document path.")
    expected_name = f"{effect_id}.effect.json"
    if resolved.name != expected_name:
        raise ValueError(f"{label} does not match targetEffectAssetId.")
    return resolved


def _resolve_json_pointer(document: Any, pointer: str) -> Any:
    if pointer == "":
        return document
    if not isinstance(pointer, str) or not pointer.startswith("/"):
        raise ValueError("Evidence assertion pointer must be a JSON pointer.")
    current = document
    for raw_token in pointer[1:].split("/"):
        token = raw_token.replace("~1", "/").replace("~0", "~")
        if isinstance(current, dict) and token in current:
            current = current[token]
        elif isinstance(current, list) and token.isdigit():
            index = int(token)
            if index >= len(current):
                raise ValueError(f"Evidence assertion pointer is out of range: {pointer}")
            current = current[index]
        else:
            raise ValueError(f"Evidence assertion pointer is missing: {pointer}")
    return current


def _validate_evidence(manifest: dict[str, Any]) -> None:
    for index, raw_evidence in enumerate(
        _require_array(manifest.get("evidence", []), "evidence")
    ):
        evidence = _require_object(raw_evidence, f"evidence[{index}]")
        path = _resolve_data_path(
            evidence.get("path"), f"evidence[{index}].path", require_file=True
        )
        expected_hash = _require_string(
            evidence.get("sha256"), f"evidence[{index}].sha256"
        )
        if SHA256.fullmatch(expected_hash) is None:
            raise ValueError(f"evidence[{index}].sha256 is invalid.")
        if _sha256_file(path) != expected_hash:
            raise ValueError(f"Evidence changed after manifest review: {path}")
        assertions = _require_array(
            evidence.get("assertions", []), f"evidence[{index}].assertions"
        )
        if not assertions:
            continue
        document = _load_json(path, f"evidence[{index}]")
        for assertion_index, raw_assertion in enumerate(assertions):
            assertion = _require_object(
                raw_assertion,
                f"evidence[{index}].assertions[{assertion_index}]",
            )
            pointer = _require_string(assertion.get("pointer"), "assertion pointer")
            if "equals" not in assertion:
                raise ValueError("Evidence assertion requires equals.")
            actual = _resolve_json_pointer(document, pointer)
            if actual != assertion["equals"]:
                raise ValueError(
                    f"Evidence assertion changed at {path}:{pointer}; "
                    f"expected={assertion['equals']!r} actual={actual!r}"
                )


FORBIDDEN_STANDALONE_MODULE_FRAGMENTS = (
    "acceleration",
    "attractor",
    "beam",
    "circlesurface",
    "collision",
    "locationbone",
    "locationdirect",
    "locationemitter",
    "locationonground",
    "locationprimitive",
    "orbit",
    "rotationrate",
    "seeded",
    "spawnperunit",
    "trail",
    "velocity",
)
CERTIFIABLE_STANDALONE_MODULE_CLASSES = frozenset(
    {
        "particlemodulecameraoffset",
        "particlemodulecolor",
        "particlemodulecoloroverlife",
        "particlemodulecolorscaleoverlife",
        "particlemoduleinitialrotation",
        "particlemoduleinitialsize",
        "particlemodulelifetime",
        "particlemodulelocation",
        "particlemodulemeshrotation",
        "particlemoduleorientationaxislock",
        "particlemoduleparameterdynamic",
        "particlemodulerequired",
        "particlemodulerotation",
        "particlemodulesize",
        "particlemodulesizemultiplylife",
        "particlemodulespawn",
        "particlemodulesubuv",
        "particlemoduletypedatamesh",
    }
)
MODULE_CLASSES_REQUIRING_FIDELITY_REVIEW = frozenset(
    CERTIFIABLE_STANDALONE_MODULE_CLASSES
    - {
        "particlemodulerequired",
        "particlemodulespawn",
    }
)


def _is_zero_number(value: Any) -> bool:
    return (
        not isinstance(value, bool)
        and isinstance(value, (int, float))
        and math.isfinite(float(value))
        and abs(float(value)) <= 1.0e-8
    )


def _fixed_distribution(distribution: dict[str, Any]) -> bool:
    if distribution.get("operation") != 1:
        return False
    source_class = str(distribution.get("sourceClass", "")).casefold()
    if "particleparameter" in source_class:
        return False
    minimum = distribution.get("defaultMinimum")
    maximum = distribution.get("defaultMaximum")
    if (
        not isinstance(minimum, list)
        or not isinstance(maximum, list)
        or len(minimum) != len(maximum)
        or not minimum
    ):
        return False
    for lower, upper in zip(minimum, maximum, strict=True):
        if (
            isinstance(lower, bool)
            or isinstance(upper, bool)
            or not isinstance(lower, (int, float))
            or not isinstance(upper, (int, float))
            or not math.isfinite(float(lower))
            or not math.isfinite(float(upper))
            or float(lower) != float(upper)
        ):
            return False
    return True


def _distribution_numeric_values(
    distribution: dict[str, Any],
) -> list[float] | None:
    result: list[float] = []
    for field in ("defaultMinimum", "defaultMaximum", "lookupTable"):
        values = distribution.get(field)
        if not isinstance(values, list):
            return None
        for value in values:
            if (
                isinstance(value, bool)
                or not isinstance(value, (int, float))
                or not math.isfinite(float(value))
            ):
                return None
            result.append(float(value))
    return result


def _is_rate_property(property_path: str) -> bool:
    tokens = [
        token
        for token in re.split(r"[^a-z0-9]+", property_path.casefold())
        if token
    ]
    if not tokens:
        return False
    leaf = tokens[-1]
    return leaf == "rate" or leaf.endswith("rate") or leaf.endswith("ratescale")


def _particle_recipe_analysis(
    recipe: dict[str, Any],
) -> tuple[str, list[str], list[dict[str, Any]]]:
    unknown_reasons: set[str] = set()
    particle_reasons: set[str] = set()
    fidelity_warnings_by_module: list[dict[str, Any]] = []

    emitter_delay = recipe.get("emitterDelaySeconds")
    if (
        isinstance(emitter_delay, bool)
        or not isinstance(emitter_delay, (int, float))
        or not math.isfinite(float(emitter_delay))
        or float(emitter_delay) < 0.0
    ):
        unknown_reasons.add("emitter-delay-unresolved")
    elif not _is_zero_number(emitter_delay):
        particle_reasons.add("emitter-delay-particle-dependent")

    loop_count = recipe.get("emitterLoopCount")
    if isinstance(loop_count, bool) or not isinstance(loop_count, int):
        unknown_reasons.add("emitter-loop-count-unresolved")
    elif loop_count != 1:
        particle_reasons.add("emitter-loop-count-particle-dependent")

    bursts = recipe.get("bursts")
    if not isinstance(bursts, list):
        unknown_reasons.add("burst-contract-unresolved")
    elif len(bursts) != 1:
        particle_reasons.add("burst-cardinality-particle-dependent")
    else:
        burst = bursts[0]
        if not isinstance(burst, dict):
            unknown_reasons.add("burst-contract-unresolved")
        else:
            if not _is_zero_number(burst.get("timeSeconds")):
                particle_reasons.add("burst-time-particle-dependent")
            counts = (burst.get("countMinimum"), burst.get("countMaximum"))
            if any(isinstance(count, bool) or not isinstance(count, int) for count in counts):
                unknown_reasons.add("burst-count-unresolved")
            elif counts != (1, 1):
                particle_reasons.add("burst-count-particle-dependent")

    modules = recipe.get("modules")
    if not isinstance(modules, list) or not modules:
        unknown_reasons.add("source-modules-unresolved")
        modules = []
    spawn_rate_found = False
    module_stable_ids: set[str] = set()
    for raw_module in modules:
        if not isinstance(raw_module, dict):
            unknown_reasons.add("source-module-contract-unresolved")
            continue
        stable_id = raw_module.get("stableId")
        stable_id_is_usable = (
            isinstance(stable_id, str)
            and bool(stable_id)
            and stable_id not in module_stable_ids
        )
        if (
            not isinstance(stable_id, str)
            or not stable_id
            or stable_id in module_stable_ids
        ):
            unknown_reasons.add("source-module-stable-id-unresolved")
        else:
            module_stable_ids.add(stable_id)
        class_name = str(raw_module.get("className", "")).casefold()
        module_warning_codes: set[str] = set()
        if not class_name:
            unknown_reasons.add("source-module-class-unresolved")
        elif class_name not in CERTIFIABLE_STANDALONE_MODULE_CLASSES and not any(
            fragment in class_name
            for fragment in FORBIDDEN_STANDALONE_MODULE_FRAGMENTS
        ):
            unknown_reasons.add("source-module-class-unsupported")
        for fragment in FORBIDDEN_STANDALONE_MODULE_FRAGMENTS:
            if fragment in class_name:
                particle_reasons.add(f"source-module-{fragment}-particle-dependent")
        if class_name in MODULE_CLASSES_REQUIRING_FIDELITY_REVIEW:
            module_warning_codes.add("deterministic-module-fidelity-review-required")
        literals = raw_module.get("literals", [])
        if not isinstance(literals, list):
            unknown_reasons.add("source-module-literal-contract-unresolved")
        elif literals:
            module_warning_codes.add("source-module-literals-review-required")
        distributions = raw_module.get("distributions", [])
        if not isinstance(distributions, list):
            unknown_reasons.add("source-distribution-contract-unresolved")
            continue
        for raw_distribution in distributions:
            if not isinstance(raw_distribution, dict):
                unknown_reasons.add("source-distribution-contract-unresolved")
                continue
            property_path = str(
                raw_distribution.get("propertyPath", "")
            ).casefold()
            if not property_path:
                unknown_reasons.add("source-distribution-path-unresolved")
                continue
            if not _fixed_distribution(raw_distribution):
                particle_reasons.add("source-distribution-particle-dependent")
            numeric_values = _distribution_numeric_values(raw_distribution)
            if numeric_values is None:
                unknown_reasons.add("source-distribution-values-unresolved")
            lookup_values = raw_distribution.get("lookupTable", [])
            keys = raw_distribution.get("keys", [])
            if not isinstance(keys, list):
                unknown_reasons.add("source-distribution-keys-unresolved")
            elif keys:
                module_warning_codes.add(
                    "deterministic-distribution-keys-review-required"
                )
            if (
                isinstance(lookup_values, list)
                and lookup_values
                and numeric_values is not None
                and any(
                    not _is_zero_number(value - float(lookup_values[0]))
                    for value in lookup_values
                )
            ):
                module_warning_codes.add(
                    "deterministic-distribution-curve-review-required"
                )
            if (
                _is_rate_property(property_path)
                and numeric_values is not None
                and any(not _is_zero_number(value) for value in numeric_values)
            ):
                particle_reasons.add("rate-property-particle-dependent")
            normalized_property_path = re.sub(r"[^a-z0-9]+", "", property_path)
            if normalized_property_path.endswith("spawnrate"):
                spawn_rate_found = True
                if numeric_values is None or not all(
                    _is_zero_number(value) for value in numeric_values
                ):
                    particle_reasons.add("spawn-rate-particle-dependent")
        if stable_id_is_usable and module_warning_codes:
            fidelity_warnings_by_module.append(
                {
                    "sourceModuleStableId": stable_id,
                    "warningCodes": sorted(module_warning_codes),
                }
            )
    if not spawn_rate_found:
        unknown_reasons.add("spawn-rate-unresolved")

    if unknown_reasons:
        return (
            UNKNOWN,
            sorted(unknown_reasons | particle_reasons),
            fidelity_warnings_by_module,
        )
    if particle_reasons:
        return PARTICLE_REQUIRED, sorted(particle_reasons), fidelity_warnings_by_module
    return (
        CERTIFIED,
        ["fixed-singleton-cardinality-and-motion"],
        fidelity_warnings_by_module,
    )


def _singleton_fixed_recipe(recipe: dict[str, Any]) -> bool:
    eligibility, _, _ = _particle_recipe_analysis(recipe)
    return eligibility == CERTIFIED


def source_element_sha256(element: dict[str, Any]) -> str:
    serialized = json.dumps(
        element,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(serialized).hexdigest()


def _source_analysis_result(
    *,
    source_document_kind: str,
    renderer_shape: str,
    source_kind: str,
    eligibility: str,
    reason_codes: list[str],
    target_kind_candidates: list[str],
    removed_module_disposition: str,
    fidelity_warnings: list[dict[str, Any]] | None = None,
) -> dict[str, Any]:
    warnings = fidelity_warnings or []
    return {
        "sourceDocumentKind": source_document_kind,
        "sourceKind": source_kind,
        "rendererShape": renderer_shape,
        # Compatibility alias for manifests written before sourceKind became
        # the explicit four-way carrier type.
        "sourceClassification": source_kind,
        "conversionEligibility": eligibility,
        "reasonCodes": reason_codes,
        "fidelityWarningCodes": sorted(
            {
                code
                for warning in warnings
                for code in warning["warningCodes"]
            }
        ),
        "fidelityWarnings": warnings,
        "targetKindCandidates": target_kind_candidates,
        "requiredTargetAnchorPolicy": (
            "PRESERVE_SOURCE_ATTACHMENT"
            if target_kind_candidates == ["particle"]
            else (
                "AUTHORED_ROOT_SNAPSHOT"
                if target_kind_candidates
                else "UNRESOLVED"
            )
        ),
        "requiredRemovedModuleDisposition": removed_module_disposition,
    }


def analyze_source_element(element: dict[str, Any]) -> dict[str, Any]:
    """Return reviewable source/eligibility/target evidence for one Element."""

    element = _require_object(element, "source element")
    source_kind_value = element.get("kind")
    source_document_kind = (
        source_kind_value if isinstance(source_kind_value, str) else "unknown"
    )
    recipe_value = element.get("sourceRecipe", {})
    if not isinstance(recipe_value, dict):
        return _source_analysis_result(
            source_document_kind=source_document_kind,
            renderer_shape="unknown",
            source_kind="unsupported",
            eligibility=UNKNOWN,
            reason_codes=["source-recipe-contract-unresolved"],
            target_kind_candidates=[],
            removed_module_disposition=MODULES_NOT_APPLICABLE,
        )

    recipe_enabled = recipe_value.get("enabled") is True
    renderer_shape_value = recipe_value.get("rendererShape", "")
    renderer_shape = (
        renderer_shape_value if isinstance(renderer_shape_value, str) else "unknown"
    )
    if source_document_kind == "mesh" and not recipe_enabled:
        return _source_analysis_result(
            source_document_kind=source_document_kind,
            renderer_shape=renderer_shape,
            source_kind=STANDALONE_MESH,
            eligibility=CERTIFIED,
            reason_codes=["source-native-standalone"],
            target_kind_candidates=["mesh"],
            removed_module_disposition=MODULES_NOT_APPLICABLE,
        )
    if source_document_kind == "sprite" and not recipe_enabled:
        return _source_analysis_result(
            source_document_kind=source_document_kind,
            renderer_shape=renderer_shape,
            source_kind=STANDALONE_SPRITE,
            eligibility=CERTIFIED,
            reason_codes=["source-native-standalone"],
            target_kind_candidates=["sprite"],
            removed_module_disposition=MODULES_NOT_APPLICABLE,
        )
    if source_document_kind == "particle" and recipe_enabled and renderer_shape in {
        "mesh",
        "sprite",
    }:
        classification = MESH_PARTICLE if renderer_shape == "mesh" else SPRITE_PARTICLE
        eligibility, reasons, fidelity_warnings = _particle_recipe_analysis(
            recipe_value
        )
        if eligibility == CERTIFIED:
            return _source_analysis_result(
                source_document_kind=source_document_kind,
                renderer_shape=renderer_shape,
                source_kind=classification,
                eligibility=CERTIFIED,
                reason_codes=reasons,
                fidelity_warnings=fidelity_warnings,
                target_kind_candidates=[renderer_shape],
                removed_module_disposition=(
                    MODULES_REMOVED_AFTER_CERTIFIED_CONVERSION
                ),
            )
        return _source_analysis_result(
            source_document_kind=source_document_kind,
            renderer_shape=renderer_shape,
            source_kind=classification,
            eligibility=eligibility,
            reason_codes=reasons,
            fidelity_warnings=fidelity_warnings,
            target_kind_candidates=(
                ["particle"]
                if renderer_shape == "sprite" and eligibility == PARTICLE_REQUIRED
                else []
            ),
            removed_module_disposition=(
                MODULES_NOT_APPLICABLE
                if eligibility == UNKNOWN
                else (
                    MODULES_PRODUCT_EXCLUDED
                    if renderer_shape == "mesh"
                    else MODULES_PRESERVED_FOR_PARTICLE_RENDERER
                )
            ),
        )
    return _source_analysis_result(
        source_document_kind=source_document_kind,
        renderer_shape=renderer_shape,
        source_kind="unsupported",
        eligibility=UNKNOWN,
        reason_codes=["source-carrier-contract-unsupported"],
        target_kind_candidates=[],
        removed_module_disposition=MODULES_NOT_APPLICABLE,
    )


def classify_source_element(element: dict[str, Any]) -> str | None:
    """Classify the source carrier without laundering it into a target kind."""

    classification = analyze_source_element(element)["sourceClassification"]
    return classification if classification != "unsupported" else None


def _resolve_authored_target_field(document: dict[str, Any], path: str) -> Any:
    if not path.startswith("detail."):
        raise ValueError("Removed module target fields must stay inside detail.")
    current: Any = document
    for token in path.split("."):
        if not isinstance(current, dict) or token not in current:
            raise ValueError(f"Removed module target field is missing: {path}")
        current = current[token]
    return current


def _validate_removed_module_dispositions(
    layer: dict[str, Any],
    source_element: dict[str, Any],
    source_element_id: str,
    target_kind: str,
) -> None:
    rows = _require_array(
        layer.get("removedModuleDispositions"),
        "layer removedModuleDispositions",
    )
    source_recipe = _require_object(
        source_element.get("sourceRecipe"), "sourceRecipe"
    )
    source_modules = _require_array(source_recipe.get("modules"), "sourceRecipe.modules")
    conversion_removes_modules = (
        source_element.get("kind") == "particle" and target_kind in {"mesh", "sprite"}
    )
    if not conversion_removes_modules:
        if rows:
            raise ValueError(
                f"Layer claims removed modules that are preserved: {source_element_id}"
            )
        return
    if len(rows) != len(source_modules):
        raise ValueError(
            f"Layer must disposition every removed source module: {source_element_id}"
        )
    seen_ids: set[str] = set()
    for index, (raw_row, raw_module) in enumerate(zip(rows, source_modules, strict=True)):
        row = _require_object(raw_row, f"removedModuleDispositions[{index}]")
        module = _require_object(raw_module, f"sourceRecipe.modules[{index}]")
        stable_id = _require_string(
            row.get("sourceModuleStableId"),
            f"removedModuleDispositions[{index}].sourceModuleStableId",
        )
        if stable_id != module.get("stableId") or stable_id in seen_ids:
            raise ValueError(
                f"Removed module stable ID drifted for {source_element_id}."
            )
        seen_ids.add(stable_id)
        disposition = _require_string(
            row.get("disposition"),
            f"removedModuleDispositions[{index}].disposition",
        )
        target_fields = [
            _require_string(field, "removed module target field")
            for field in _require_array(
                row.get("targetFields", []),
                f"removedModuleDispositions[{index}].targetFields",
            )
        ]
        if len(target_fields) != len(set(target_fields)):
            raise ValueError("Removed module target fields are duplicated.")
        if disposition == "bakedIntoAuthoredDetail":
            if not target_fields:
                raise ValueError("Baked module disposition requires target fields.")
            for target_field in target_fields:
                _resolve_authored_target_field(source_element, target_field)
        elif disposition == "acceptedApproximation":
            _require_string(
                row.get("rationale"),
                f"removedModuleDispositions[{index}].rationale",
            )
            for target_field in target_fields:
                _resolve_authored_target_field(source_element, target_field)
        else:
            raise ValueError(
                f"Removed module disposition is unsupported: {disposition}"
            )


def _validate_fidelity_warnings(
    value: Any, label: str
) -> list[dict[str, Any]]:
    warnings: list[dict[str, Any]] = []
    stable_ids: set[str] = set()
    for index, raw_warning in enumerate(_require_array(value, label)):
        warning = _require_object(raw_warning, f"{label}[{index}]")
        stable_id = _require_string(
            warning.get("sourceModuleStableId"),
            f"{label}[{index}].sourceModuleStableId",
        )
        if stable_id in stable_ids:
            raise ValueError(f"{label} contains a duplicate source module.")
        stable_ids.add(stable_id)
        warning_codes = [
            _require_stable_token(code, f"{label}[{index}].warningCodes")
            for code in _require_array(
                warning.get("warningCodes"), f"{label}[{index}].warningCodes"
            )
        ]
        if not warning_codes or warning_codes != sorted(set(warning_codes)):
            raise ValueError(f"{label} warning codes must be unique and sorted.")
        warnings.append(
            {
                "sourceModuleStableId": stable_id,
                "warningCodes": warning_codes,
            }
        )
    return warnings


def _validate_reviewed_layer_contract(
    layer: dict[str, Any],
    source_element: dict[str, Any],
    source_element_id: str,
    analysis: dict[str, Any],
) -> str:
    declared_document_kind = _require_string(
        layer.get("sourceDocumentKind"), "layer sourceDocumentKind"
    )
    if declared_document_kind != analysis["sourceDocumentKind"]:
        raise ValueError(
            f"Layer sourceDocumentKind drifted for {source_element_id}: "
            f"declared={declared_document_kind} "
            f"actual={analysis['sourceDocumentKind']}"
        )

    declared_source_kind = _require_string(
        layer.get("sourceKind"), "layer sourceKind"
    )
    if declared_source_kind != analysis["sourceKind"]:
        raise ValueError(
            f"Layer sourceKind drifted for {source_element_id}: "
            f"declared={declared_source_kind} actual={analysis['sourceKind']}"
        )

    declared_eligibility = _require_string(
        layer.get("conversionEligibility"), "layer conversionEligibility"
    )
    if declared_eligibility not in CONVERSION_ELIGIBILITIES:
        raise ValueError("Layer conversionEligibility is invalid.")
    if declared_eligibility != analysis["conversionEligibility"]:
        raise ValueError(
            f"Layer conversionEligibility drifted for {source_element_id}."
        )

    declared_reasons = [
        _require_stable_token(reason, "layer conversionReasonCodes")
        for reason in _require_array(
            layer.get("conversionReasonCodes"), "layer conversionReasonCodes"
        )
    ]
    if declared_reasons != analysis["reasonCodes"]:
        raise ValueError(
            f"Layer conversion reason evidence drifted for {source_element_id}."
        )

    declared_warning_codes = [
        _require_stable_token(code, "layer fidelityWarningCodes")
        for code in _require_array(
            layer.get("fidelityWarningCodes"), "layer fidelityWarningCodes"
        )
    ]
    if declared_warning_codes != analysis["fidelityWarningCodes"]:
        raise ValueError(
            f"Layer fidelity warning evidence drifted for {source_element_id}."
        )
    declared_warnings = _validate_fidelity_warnings(
        layer.get("fidelityWarnings"), "layer fidelityWarnings"
    )
    if declared_warnings != analysis["fidelityWarnings"]:
        raise ValueError(
            f"Layer per-module fidelity evidence drifted for {source_element_id}."
        )

    expected_hash = _require_string(
        layer.get("sourceElementSha256"), "layer sourceElementSha256"
    )
    if SHA256.fullmatch(expected_hash) is None or expected_hash != source_element_sha256(
        source_element
    ):
        raise ValueError(f"Layer source Element evidence drifted: {source_element_id}")

    removed_module_disposition = _require_string(
        layer.get("removedModuleDisposition"), "layer removedModuleDisposition"
    )
    if removed_module_disposition != analysis["requiredRemovedModuleDisposition"]:
        raise ValueError(
            f"Layer removed-module disposition drifted for {source_element_id}."
        )

    target_kind = _require_string(layer.get("targetKind"), "layer targetKind")
    if target_kind not in analysis["targetKindCandidates"]:
        raise ValueError(
            f"Layer targetKind is not certified for {source_element_id}: {target_kind}"
        )
    target_anchor_policy = _require_string(
        layer.get("targetAnchorPolicy"), "layer targetAnchorPolicy"
    )
    expected_anchor_policy = (
        "PRESERVE_SOURCE_ATTACHMENT"
        if target_kind == "particle"
        else "AUTHORED_ROOT_SNAPSHOT"
    )
    if expected_anchor_policy != analysis["requiredTargetAnchorPolicy"]:
        raise ValueError(
            f"Source target anchor policy is unresolved for {source_element_id}."
        )
    if target_anchor_policy != expected_anchor_policy:
        raise ValueError(
            f"Layer target anchor policy drifted for {source_element_id}."
        )
    _validate_removed_module_dispositions(
        layer,
        source_element,
        source_element_id,
        target_kind,
    )
    return target_kind


def _validate_authored_root_snapshot_policy(
    occurrence: dict[str, Any], occurrence_index: int
) -> dict[str, Any]:
    policy = _require_object(
        occurrence.get("authoredAnchorPolicy"),
        f"occurrences[{occurrence_index}].authoredAnchorPolicy",
    )
    if policy.get("provenance") != "AUTHORED_POLICY":
        raise ValueError("Root snapshot must be declared as AUTHORED_POLICY.")
    if policy.get("runtimeAnchorSlotId") != "root" or policy.get("follow") is not False:
        raise ValueError("Authored occurrence anchor must be a root snapshot.")
    socket_transform = _require_object(
        policy.get("socketLocalTransform"), "authoredAnchorPolicy.socketLocalTransform"
    )
    _require_vector(
        socket_transform.get("position"),
        3,
        "authoredAnchorPolicy.socketLocalTransform.position",
    )
    _require_vector(
        socket_transform.get("rotationDegrees"),
        3,
        "authoredAnchorPolicy.socketLocalTransform.rotationDegrees",
    )
    _require_vector(
        socket_transform.get("scale"),
        3,
        "authoredAnchorPolicy.socketLocalTransform.scale",
        positive=True,
    )
    return policy


def _reset_source_recipe() -> dict[str, Any]:
    return {
        "enabled": False,
        "rendererShape": "",
        "emitterDelaySeconds": 0.0,
        "emitterDurationSeconds": 0.0,
        "emitterLoopCount": 0,
        "bursts": [],
        "modules": [],
    }


def _reset_source_presentation() -> dict[str, Any]:
    return {
        "enabled": False,
        "schema": "lostark.effect-source-presentation",
        "version": 1,
        "profileId": "",
        "status": "unresolved",
        "sourceObjectPath": "",
        "sourceActionCueId": "",
        "sourceEventId": "",
        "sourceOccurrenceIndex": 0,
        "sourceTimeSeconds": 0.0,
        "parameters": [],
    }


def _validate_resource_id(value: Any, label: str) -> str:
    asset_id = _require_string(value, label)
    normalized = asset_id.replace("\\", "/")
    if (
        normalized != asset_id
        or not asset_id.startswith("Effect/")
        or ".." in asset_id.split("/")
        or ":" in asset_id
    ):
        raise ValueError(f"{label} is not a Resources-relative Effect asset ID.")
    resource_path = (RESOURCES_ROOT / asset_id).resolve()
    try:
        resource_path.relative_to(RESOURCES_ROOT)
    except ValueError as error:
        raise ValueError(f"{label} escaped the Resources root.") from error
    if not resource_path.is_file():
        raise ValueError(f"{label} is unresolved: {asset_id}")
    return asset_id


def _validate_resources(
    element: dict[str, Any], classification: str, source_element_id: str
) -> None:
    resources = _require_array(element.get("resources"), "source resources")
    slots: dict[str, str] = {}
    for raw_resource in resources:
        resource = _require_object(raw_resource, "source resource")
        slot = _require_string(resource.get("slotId"), "source resource slotId")
        if slot not in RESOURCE_SLOT_ORDER or slot in slots:
            raise ValueError(f"Source Element has an invalid resource slot: {source_element_id}")
        slots[slot] = _validate_resource_id(
            resource.get("assetId"), f"source resource {slot}"
        )
    if classification in MESH_CLASSIFICATIONS and "meshModel" not in slots:
        raise ValueError(f"Mesh source has no meshModel: {source_element_id}")
    if classification in SPRITE_CLASSIFICATIONS and (
        "meshModel" in slots or not ({"base", "mask", "emissive"} & set(slots))
    ):
        raise ValueError(f"Sprite source has no admitted texture carrier: {source_element_id}")


def _validate_source_document(
    source: dict[str, Any], expected_effect_id: str
) -> dict[str, dict[str, Any]]:
    if (
        source.get("schema") != "lostark.effect-authoring"
        or source.get("version") != 12
        or source.get("effectAssetId") != expected_effect_id
    ):
        raise ValueError("Canonical source Effect document contract is invalid.")
    _require_object(source.get("particleSystem"), "source particleSystem")
    elements = _require_array(source.get("elements"), "source elements")
    by_id: dict[str, dict[str, Any]] = {}
    for raw_element in elements:
        element = _require_object(raw_element, "source element")
        element_id = _require_string(element.get("id"), "source element ID")
        if element_id in by_id:
            raise ValueError(f"Canonical source has a duplicate Element ID: {element_id}")
        by_id[element_id] = element
    return by_id


def _validate_material(element: dict[str, Any], source_element_id: str) -> None:
    material = _require_object(element.get("material"), "source material")
    _require_string(material.get("templateId"), "source material templateId")
    _require_string(material.get("sourceMaterialPath"), "source material path")
    _require_string(material.get("renderProfile"), "source material renderProfile")
    source_profile = _require_object(
        material.get("sourceProfile"), "source material sourceProfile"
    )
    if source_profile.get("enabled") is not True:
        raise ValueError(f"Source material profile is unresolved: {source_element_id}")
    _require_string(source_profile.get("profileId"), "source material profileId")
    runtime_shader_profile = _require_string(
        source_profile.get("runtimeShaderProfileId"),
        "source material runtimeShaderProfileId",
    )
    if runtime_shader_profile == "effect.ue3.fallback-blocked.v1":
        raise ValueError(
            f"Source material runtime profile is fallback-blocked: {source_element_id}"
        )
    semantic_status = _require_string(
        source_profile.get("semanticStatus"),
        "source material semanticStatus",
    )
    if semantic_status.casefold() in {
        "unresolved",
        "unsupported",
        "missing_resource",
    }:
        raise ValueError(
            f"Source material semantic status is unresolved: {source_element_id}"
        )
    for texture_index, raw_texture in enumerate(
        _require_array(source_profile.get("textures"), "source material textures")
    ):
        texture = _require_object(
            raw_texture, f"source material textures[{texture_index}]"
        )
        _require_string(
            texture.get("sourceObjectPath"),
            f"source material textures[{texture_index}].sourceObjectPath",
        )
        _validate_resource_id(
            texture.get("assetId"),
            f"source material textures[{texture_index}].assetId",
        )


def _validate_anchor(element: dict[str, Any], source_element_id: str) -> None:
    attachment = _require_object(
        element.get("actionCueAttachment"), "source actionCueAttachment"
    )
    if attachment.get("enabled") is not True:
        raise ValueError(f"Source anchor is not enabled: {source_element_id}")
    if not isinstance(attachment.get("follow"), bool):
        raise ValueError(f"Source anchor follow policy is unresolved: {source_element_id}")
    _require_string(
        attachment.get("sourceAnchorSlotId"), "source anchor slot ID"
    )
    _require_string(
        attachment.get("runtimeAnchorSlotId"), "runtime anchor slot ID"
    )
    runtime_bone_name = attachment.get("runtimeBoneName", "")
    if not isinstance(runtime_bone_name, str) or len(runtime_bone_name) > 512:
        raise ValueError(f"Source anchor runtime bone is invalid: {source_element_id}")
    socket_transform = _require_object(
        attachment.get("socketLocalTransform"), "socketLocalTransform"
    )
    _require_vector(
        socket_transform.get("position"), 3, "socketLocalTransform.position"
    )
    _require_vector(
        socket_transform.get("rotationDegrees"),
        3,
        "socketLocalTransform.rotationDegrees",
    )
    _require_vector(
        socket_transform.get("scale"),
        3,
        "socketLocalTransform.scale",
        positive=True,
    )


def _copy_source_element(
    source_element: dict[str, Any],
    *,
    source_effect_id: str,
    source_element_id: str,
    group_id: str,
    role: str,
    source_timeline_offset: float,
    default_sprite_roll: float,
    layer: dict[str, Any],
    target_kind: str,
    authored_anchor_policy: dict[str, Any],
) -> dict[str, Any]:
    source_analysis = analyze_source_element(source_element)
    classification = source_analysis["sourceClassification"]
    if classification not in SOURCE_CLASSIFICATIONS:
        raise ValueError(f"Source Element has no admitted carrier: {source_element_id}")
    _validate_resources(source_element, classification, source_element_id)
    _validate_material(source_element, source_element_id)
    if target_kind == "particle":
        _validate_anchor(source_element, source_element_id)

    element = copy.deepcopy(source_element)
    element["id"] = f"{group_id}.{role}"
    element["displayName"] = element["id"]
    element["groupId"] = group_id
    element["sourceNode"] = f"authored-source:{source_effect_id}|element:{source_element_id}"
    element["visible"] = True
    if target_kind == "particle":
        element["sourceNode"] += "|anchor-policy:source-preserved"
    else:
        element["sourceNode"] += "|anchor-policy:authored-root-snapshot"
        element["actionCueAttachment"] = {
            "enabled": True,
            "follow": False,
            "sourceAnchorSlotId": "root",
            "runtimeAnchorSlotId": "root",
            "runtimeBoneName": "",
            "socketLocalTransform": copy.deepcopy(
                authored_anchor_policy["socketLocalTransform"]
            ),
        }

    detail = _require_object(element.get("detail"), "source detail")
    transform = _require_object(detail.get("transform"), "source transform")
    _require_vector(transform.get("position"), 3, "source transform position")
    _require_vector(
        transform.get("rotationDegrees"), 3, "source transform rotationDegrees"
    )
    _require_vector(
        transform.get("scale"), 3, "source transform scale", positive=True
    )
    timing = _require_object(detail.get("timing"), "source timing")
    source_delay = _require_non_negative_number(
        timing.get("startDelaySeconds"), "source startDelaySeconds"
    )
    target_delay = source_delay - source_timeline_offset
    if target_delay < -1.0e-6:
        raise ValueError(
            f"Source Element precedes its stage offset: {source_element_id}"
        )
    timing["startDelaySeconds"] = max(0.0, target_delay)
    if _require_number(timing.get("lifeTimeSeconds"), "source lifeTimeSeconds") <= 0.0:
        raise ValueError(f"Source lifetime is invalid: {source_element_id}")

    sprite = _require_object(detail.get("sprite"), "source sprite detail")
    if target_kind == "mesh":
        if classification not in {STANDALONE_MESH, MESH_PARTICLE} or (
            classification == MESH_PARTICLE
            and source_analysis["conversionEligibility"] != CERTIFIED
        ):
            raise ValueError("Only a certified Mesh carrier may target mesh.")
        element["kind"] = "mesh"
        element["sourceRecipe"] = _reset_source_recipe()
        element["sourcePresentation"] = _reset_source_presentation()
        sprite["billboard"] = False
        sprite["billboardRollDegrees"] = 0.0
        mesh = _require_object(detail.get("mesh"), "source mesh detail")
        mesh["useModelMaterial"] = False
    elif target_kind == "sprite":
        if classification not in {STANDALONE_SPRITE, SPRITE_PARTICLE} or (
            classification == SPRITE_PARTICLE
            and source_analysis["conversionEligibility"] != CERTIFIED
        ):
            raise ValueError("Only a certified Sprite carrier may target sprite.")
        element["kind"] = "sprite"
        element["sourceRecipe"] = _reset_source_recipe()
        element["sourcePresentation"] = _reset_source_presentation()
        override = layer.get("spriteBillboardRollDegrees", default_sprite_roll)
        sprite["billboard"] = True
        sprite["billboardRollDegrees"] = _require_number(
            override, "spriteBillboardRollDegrees"
        )
    elif target_kind == "particle":
        if (
            classification != SPRITE_PARTICLE
            or source_analysis["conversionEligibility"] != PARTICLE_REQUIRED
        ):
            raise ValueError(
                "Only an executable particle-dependent Sprite may target particle."
            )
        if (
            element.get("kind") != "particle"
            or element["sourceRecipe"].get("enabled") is not True
            or element["sourceRecipe"].get("rendererShape") != "sprite"
        ):
            raise ValueError(
                f"Sprite Particle renderer recipe is unresolved: {source_element_id}"
            )
        particle = _require_object(
            detail.get("particle"), "source Sprite Particle detail"
        )
        if not isinstance(particle.get("billboard"), bool):
            raise ValueError(
                f"Sprite Particle billboard policy is unresolved: {source_element_id}"
            )
        override = layer.get("spriteBillboardRollDegrees", default_sprite_roll)
        sprite["billboardRollDegrees"] = _require_number(
            override, "spriteBillboardRollDegrees"
        )
    else:
        raise ValueError(f"Unsupported Authored target kind: {target_kind}")
    return element


def build_stage_document(
    source: dict[str, Any],
    manifest: dict[str, Any],
    stage: dict[str, Any],
    default_sprite_roll: float,
    *,
    source_timeline_end: float | None = None,
    claimed_source_ids: set[str] | None = None,
) -> dict[str, Any]:
    source_contract = _require_object(manifest.get("source"), "source")
    source_effect_id = _require_effect_id(
        source_contract.get("effectAssetId"), "source.effectAssetId"
    )
    source_by_id = _validate_source_document(source, source_effect_id)
    target_effect_id = _require_effect_id(
        stage.get("targetEffectAssetId"), "stage targetEffectAssetId"
    )
    source_offset = _require_non_negative_number(
        stage.get("sourceTimelineOffsetSeconds", 0.0),
        "stage sourceTimelineOffsetSeconds",
    )
    if source_timeline_end is not None:
        source_timeline_end = _require_non_negative_number(
            source_timeline_end, "stage source timeline end"
        )
        if source_timeline_end <= source_offset:
            raise ValueError("Stage source timeline window is invalid.")
    occurrences = _require_array(stage.get("occurrences"), "stage occurrences")
    if not occurrences:
        raise ValueError("Ready stage requires at least one occurrence.")

    staged_elements: list[dict[str, Any]] = []
    source_ids: set[str] = set()
    group_ids: set[str] = set()
    for occurrence_index, raw_occurrence in enumerate(occurrences):
        occurrence = _require_object(
            raw_occurrence, f"occurrences[{occurrence_index}]"
        )
        _require_stable_token(
            occurrence.get("occurrenceId"),
            f"occurrences[{occurrence_index}].occurrenceId",
        )
        group_id = _require_stable_token(
            occurrence.get("groupId"),
            f"occurrences[{occurrence_index}].groupId",
        )
        if group_id in group_ids:
            raise ValueError(f"Ready stage has a duplicate groupId: {group_id}")
        group_ids.add(group_id)
        authored_anchor_policy = _validate_authored_root_snapshot_policy(
            occurrence, occurrence_index
        )
        layers = _require_array(
            occurrence.get("layers"), f"occurrences[{occurrence_index}].layers"
        )
        if not layers:
            raise ValueError("Occurrence must contain at least one admitted layer.")
        roles: list[str] = []
        for layer_index, raw_layer in enumerate(layers):
            layer = _require_object(
                raw_layer,
                f"occurrences[{occurrence_index}].layers[{layer_index}]",
            )
            role = _require_stable_token(layer.get("role"), "layer role")
            if role in roles:
                raise ValueError(f"Occurrence has a duplicate role: {role}")
            roles.append(role)
        for layer, role in zip(layers, roles, strict=True):
            source_element_id = _require_string(
                layer.get("sourceElementId"), "layer sourceElementId"
            )
            if source_element_id in source_ids:
                raise ValueError(
                    f"Ready stage reuses a source Element: {source_element_id}"
                )
            if (
                claimed_source_ids is not None
                and source_element_id in claimed_source_ids
            ):
                raise ValueError(
                    "Ready stages reuse a source Element without explicit reuse "
                    f"provenance: {source_element_id}"
                )
            source_element = source_by_id.get(source_element_id)
            if source_element is None:
                raise ValueError(
                    f"Ready stage source Element is missing: {source_element_id}"
                )
            source_timing = _require_object(
                _require_object(
                    source_element.get("detail"), "source Element detail"
                ).get("timing"),
                "source Element timing",
            )
            source_delay = _require_non_negative_number(
                source_timing.get("startDelaySeconds"),
                "source Element startDelaySeconds",
            )
            if source_delay + STAGE_TIME_EPSILON < source_offset:
                raise ValueError(
                    f"Source Element precedes its stage offset: {source_element_id}"
                )
            if (
                source_timeline_end is not None
                and source_delay + STAGE_TIME_EPSILON >= source_timeline_end
            ):
                raise ValueError(
                    "Source Element is outside its stage timeline window: "
                    f"{source_element_id}"
                )
            declared_classification = _require_string(
                layer.get("sourceClassification"), "layer sourceClassification"
            )
            source_analysis = analyze_source_element(source_element)
            actual_classification = source_analysis["sourceClassification"]
            if declared_classification != actual_classification:
                raise ValueError(
                    "Layer classification is not proven by the source recipe: "
                    f"{source_element_id}; declared={declared_classification} "
                    f"actual={actual_classification}"
                )
            if source_analysis["conversionEligibility"] == UNKNOWN:
                raise ValueError(
                    f"Source Element conversion eligibility is unknown: {source_element_id}"
                )
            if (
                actual_classification == MESH_PARTICLE
                and source_analysis["conversionEligibility"] != CERTIFIED
            ):
                raise ValueError(
                    "Particle-dependent Mesh has no admitted product target: "
                    f"{source_element_id}"
                )
            target_kind = _validate_reviewed_layer_contract(
                layer,
                source_element,
                source_element_id,
                source_analysis,
            )
            source_ids.add(source_element_id)
            if claimed_source_ids is not None:
                claimed_source_ids.add(source_element_id)
            staged_elements.append(
                _copy_source_element(
                    source_element,
                    source_effect_id=source_effect_id,
                    source_element_id=source_element_id,
                    group_id=group_id,
                    role=role,
                    source_timeline_offset=source_offset,
                    default_sprite_roll=default_sprite_roll,
                    layer=layer,
                    target_kind=target_kind,
                    authored_anchor_policy=authored_anchor_policy,
                )
            )

    for element in staged_elements:
        if element["kind"] != "particle":
            continue
        recipe = _require_object(
            element.get("sourceRecipe"), "Authored Sprite Particle sourceRecipe"
        )
        if recipe.get("enabled") is not True or recipe.get("rendererShape") != "sprite":
            raise ValueError(
                "Authored stage may preserve only a Sprite Particle renderer."
            )
    return {
        "schema": "lostark.effect-authoring",
        "version": 12,
        "effectAssetId": target_effect_id,
        "displayName": _require_string(stage.get("displayName"), "stage displayName"),
        "particleSystem": copy.deepcopy(source["particleSystem"]),
        "modelCues": [],
        "elements": staged_elements,
    }


def _validate_blockers(manifest: dict[str, Any], status: str) -> list[dict[str, str]]:
    raw_blockers = _require_array(manifest.get("blockers", []), "blockers")
    if status == BLOCKED and not raw_blockers:
        raise ValueError("Blocked manifest requires at least one blocker.")
    if status != BLOCKED and raw_blockers:
        raise ValueError("Only blocked manifests may contain blockers.")
    blockers: list[dict[str, str]] = []
    codes: set[str] = set()
    for index, raw_blocker in enumerate(raw_blockers):
        blocker = _require_object(raw_blocker, f"blockers[{index}]")
        code = _require_stable_token(
            blocker.get("code"), f"blockers[{index}].code"
        )
        message = _require_string(
            blocker.get("message"), f"blockers[{index}].message"
        )
        if code in codes:
            raise ValueError(f"Blocked manifest has a duplicate blocker: {code}")
        codes.add(code)
        blockers.append({"code": code, "message": message})
    return blockers


def _validate_stage_common(
    stage: dict[str, Any], manifest_status: str
) -> tuple[int, str, Path]:
    stage_index = stage.get("stageIndex")
    if isinstance(stage_index, bool) or not isinstance(stage_index, int) or stage_index < 0:
        raise ValueError("stageIndex must be a non-negative integer.")
    _require_string(stage.get("clip"), "stage clip")
    status = _require_string(stage.get("status"), "stage status")
    if status != manifest_status:
        raise ValueError("Stage status must match its skill manifest status.")
    target_effect_id = _require_effect_id(
        stage.get("targetEffectAssetId"), "stage targetEffectAssetId"
    )
    target_path = _resolve_target_path(
        stage.get("targetAuthoringPath"),
        target_effect_id,
        "stage targetAuthoringPath",
    )
    return stage_index, target_effect_id, target_path


def _validate_file_hash(path: Path, value: Any, label: str) -> None:
    expected_hash = _require_string(value, label)
    if SHA256.fullmatch(expected_hash) is None or _sha256_file(path) != expected_hash:
        raise ValueError(f"{label} drifted after product-gate review: {path}")


def _read_effect_cue_rows(path: Path) -> list[tuple[str, dict[str, str]]]:
    rows: list[tuple[str, dict[str, str]]] = []
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except OSError as error:
        raise ValueError(f"Animation cue document could not be read: {error}") from error
    for line_number, line in enumerate(lines, start=1):
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        try:
            tokens = shlex.split(stripped, posix=True)
        except ValueError as error:
            raise ValueError(
                f"Animation cue syntax is invalid at {path}:{line_number}: {error}"
            ) from error
        if len(tokens) < 2 or tokens[1] != "EFFECT":
            continue
        attributes: dict[str, str] = {}
        for token in tokens[2:]:
            if "=" not in token:
                continue
            key, value = token.split("=", 1)
            if key in attributes:
                raise ValueError(
                    f"Animation cue has a duplicate attribute at {path}:{line_number}."
                )
            attributes[key] = value
        rows.append((tokens[0], attributes))
    return rows


def _validate_preserved_product_gate(
    stage: dict[str, Any],
    target_effect_id: str,
    target_path: Path,
    protected: dict[str, Any],
) -> dict[str, Any]:
    gate = _require_object(stage.get("productGate"), "preserved stage productGate")
    if (
        gate.get("schema") != PRODUCT_GATE_SCHEMA
        or gate.get("version") != PRODUCT_GATE_VERSION
    ):
        raise ValueError("Preserved Authored product gate schema/version is invalid.")
    _validate_file_hash(
        target_path, gate.get("documentSha256"), "productGate.documentSha256"
    )

    expected_element_count = gate.get("expectedElementCount")
    expected_occurrence_count = gate.get("expectedOccurrenceCount")
    if (
        isinstance(expected_element_count, bool)
        or not isinstance(expected_element_count, int)
        or expected_element_count <= 0
        or isinstance(expected_occurrence_count, bool)
        or not isinstance(expected_occurrence_count, int)
        or expected_occurrence_count <= 0
    ):
        raise ValueError("Preserved product count gate is invalid.")
    elements = _require_array(protected.get("elements"), "protected elements")
    if len(elements) != expected_element_count:
        raise ValueError("Protected Authored Element count failed its product gate.")

    expected_kind_counts = _require_object(
        gate.get("expectedKindCounts"), "productGate.expectedKindCounts"
    )
    if set(expected_kind_counts) != {"mesh", "sprite", "particle"} or any(
        isinstance(count, bool) or not isinstance(count, int) or count < 0
        for count in expected_kind_counts.values()
    ):
        raise ValueError("Preserved product kind-count gate is invalid.")
    actual_kind_counts = {"mesh": 0, "sprite": 0, "particle": 0}
    group_ids: set[str] = set()
    for raw_element in elements:
        element = _require_object(raw_element, "protected Element")
        kind = _require_string(element.get("kind"), "protected Element kind")
        if kind not in actual_kind_counts:
            raise ValueError(f"Protected product contains an unapproved kind: {kind}")
        actual_kind_counts[kind] += 1
        group_ids.add(_require_stable_token(element.get("groupId"), "protected groupId"))
    if actual_kind_counts != expected_kind_counts:
        raise ValueError("Protected Authored kind counts failed the product gate.")
    if len(group_ids) != expected_occurrence_count:
        raise ValueError("Protected Authored occurrence count failed the product gate.")

    expected_sprite_roll = _require_number(
        gate.get("spriteBillboardRollDegrees"),
        "productGate.spriteBillboardRollDegrees",
    )
    for raw_element in elements:
        element = _require_object(raw_element, "protected Element")
        if element.get("kind") != "sprite":
            continue
        sprite = _require_object(
            _require_object(element.get("detail"), "protected Element detail").get(
                "sprite"
            ),
            "protected Sprite detail",
        )
        if sprite.get("billboard") is not True or abs(
            _require_number(
                sprite.get("billboardRollDegrees"),
                "protected Sprite billboardRollDegrees",
            )
            - expected_sprite_roll
        ) > 1.0e-6:
            raise ValueError("Protected Sprite roll failed the product gate.")

    inner_anchor = _require_object(
        gate.get("innerAnchorPolicy"), "productGate.innerAnchorPolicy"
    )
    if (
        inner_anchor.get("provenance") != "AUTHORED_POLICY"
        or inner_anchor.get("runtimeAnchorSlotId") != "root"
        or inner_anchor.get("follow") is not False
    ):
        raise ValueError("Protected inner anchor must be Authored root snapshot policy.")
    for raw_element in elements:
        attachment = _require_object(
            _require_object(raw_element, "protected Element").get(
                "actionCueAttachment"
            ),
            "protected actionCueAttachment",
        )
        if (
            attachment.get("enabled") is not True
            or attachment.get("runtimeAnchorSlotId") != "root"
            or attachment.get("follow") is not False
        ):
            raise ValueError("Protected inner root snapshot contract drifted.")

    catalog_gate = _require_object(gate.get("catalog"), "productGate.catalog")
    catalog_path = _resolve_data_path(
        catalog_gate.get("path"), "productGate.catalog.path", require_file=True
    )
    _validate_file_hash(
        catalog_path, catalog_gate.get("sha256"), "productGate.catalog.sha256"
    )
    catalog = _load_json(catalog_path, "product Effect catalog")
    catalog_rows = [
        row
        for row in _require_array(catalog.get("effects"), "product catalog effects")
        if isinstance(row, dict) and row.get("effectAssetId") == target_effect_id
    ]
    expected_authoring_path = target_path.relative_to(DATA_ROOT).as_posix()
    if len(catalog_rows) != 1 or catalog_rows[0].get(
        "authoringPath"
    ) != expected_authoring_path:
        raise ValueError("Protected Authored Effect is not exactly registered in catalog.")

    cue_gate = _require_object(
        gate.get("animationCue"), "productGate.animationCue"
    )
    cue_path = _resolve_data_path(
        cue_gate.get("path"), "productGate.animationCue.path", require_file=True
    )
    _validate_file_hash(
        cue_path, cue_gate.get("sha256"), "productGate.animationCue.sha256"
    )
    cue_clip = _require_string(cue_gate.get("clip"), "productGate.animationCue.clip")
    cue_start_ms = cue_gate.get("startMilliseconds")
    if isinstance(cue_start_ms, bool) or not isinstance(cue_start_ms, int) or cue_start_ms < 0:
        raise ValueError("Product animation cue startMilliseconds is invalid.")
    cue_stop = _require_string(cue_gate.get("stop"), "productGate.animationCue.stop")
    outer_anchor = _require_object(
        cue_gate.get("outerAnchorPolicy"),
        "productGate.animationCue.outerAnchorPolicy",
    )
    if (
        outer_anchor.get("provenance") != "AUTHORED_POLICY"
        or outer_anchor.get("runtimeAnchorSlotId") != "root"
        or outer_anchor.get("follow") is not True
    ):
        raise ValueError("Product cue outer anchor must be Authored root follow policy.")
    matching_cues = []
    for clip, attributes in _read_effect_cue_rows(cue_path):
        if clip != cue_clip or attributes.get("payload") != target_effect_id:
            continue
        matching_cues.append(attributes)
    if len(matching_cues) != 1:
        raise ValueError("Protected Authored Effect requires exactly one product cue.")
    cue = matching_cues[0]
    if (
        cue.get("effectref") != "asset"
        or cue.get("anchor") != "root"
        or cue.get("follow") != "follow"
        or cue.get("startms") != str(cue_start_ms)
        or cue.get("stop") != cue_stop
    ):
        raise ValueError("Protected Authored product cue contract drifted.")

    return {
        "status": "passed",
        "effectAssetId": target_effect_id,
        "elementCount": len(elements),
        "occurrenceCount": len(group_ids),
        "kindCounts": actual_kind_counts,
        "spriteBillboardRollDegrees": expected_sprite_roll,
        "innerAnchorProvenance": "AUTHORED_POLICY",
        "outerAnchorProvenance": "AUTHORED_POLICY",
        "catalogRegistered": True,
        "exactCueRegistered": True,
    }


def _diagnose_source_document(
    document: dict[str, Any],
    diagnostic_contract: dict[str, Any],
    stages: list[dict[str, Any]],
) -> dict[str, Any]:
    if document.get("schema") != "lostark.effect-authoring" or document.get(
        "version"
    ) != 12:
        raise ValueError("Diagnostic source Effect contract is invalid.")
    expected_effect_id = _require_effect_id(
        diagnostic_contract.get("effectAssetId"), "diagnosticSource.effectAssetId"
    )
    if document.get("effectAssetId") != expected_effect_id:
        raise ValueError("Diagnostic source Effect asset ID changed.")
    elements = _require_array(document.get("elements"), "diagnostic source elements")

    offsets: list[tuple[int, float]] = []
    for stage in stages:
        stage_index = stage.get("stageIndex")
        offset = _require_non_negative_number(
            stage.get("sourceTimelineOffsetSeconds"),
            "diagnostic stage sourceTimelineOffsetSeconds",
        )
        offsets.append((stage_index, offset))
    if offsets != sorted(offsets) or any(
        current[1] >= following[1]
        for current, following in zip(offsets, offsets[1:])
    ):
        raise ValueError("Diagnostic stage timeline offsets are not strictly ordered.")

    classification_keys = [
        STANDALONE_MESH,
        MESH_PARTICLE,
        STANDALONE_SPRITE,
        SPRITE_PARTICLE,
        "unsupported",
    ]

    def empty_counts() -> dict[str, int]:
        return {key: 0 for key in classification_keys}

    aggregate_counts = empty_counts()
    stage_rows = [
        {
            "stageIndex": stage_index,
            "sourceTimelineOffsetSeconds": offset,
            "elementCount": 0,
            "classificationCounts": empty_counts(),
            "fullyAdmittedStandaloneCount": 0,
            "fullyAdmittedSpriteParticleCount": 0,
            "fullyAdmittedProductCarrierCount": 0,
        }
        for stage_index, offset in offsets
    ]
    fully_admitted_standalone = 0
    fully_admitted_sprite_particle = 0
    unassigned = 0
    source_element_rows: list[dict[str, Any]] = []
    for raw_element in elements:
        element = _require_object(raw_element, "diagnostic source element")
        element_id = _require_string(element.get("id"), "diagnostic source Element ID")
        source_analysis = analyze_source_element(element)
        classification = source_analysis["sourceClassification"]
        classification_key = classification or "unsupported"
        aggregate_counts[classification_key] += 1

        detail = _require_object(element.get("detail"), "diagnostic source detail")
        timing = _require_object(detail.get("timing"), "diagnostic source timing")
        start_delay = _require_non_negative_number(
            timing.get("startDelaySeconds"), "diagnostic source startDelaySeconds"
        )
        selected_stage: dict[str, Any] | None = None
        for row in reversed(stage_rows):
            if start_delay + 1.0e-6 >= row["sourceTimelineOffsetSeconds"]:
                selected_stage = row
                break
        if selected_stage is None:
            unassigned += 1
        else:
            selected_stage["elementCount"] += 1
            selected_stage["classificationCounts"][classification_key] += 1

        admission_reasons: list[str] = []
        target_candidates = source_analysis["targetKindCandidates"]
        admitted_target_kind = (
            target_candidates[0] if len(target_candidates) == 1 else None
        )
        if admitted_target_kind is not None and source_analysis[
            "conversionEligibility"
        ] != UNKNOWN:
            try:
                _validate_resources(element, classification, element_id)
            except ValueError:
                admission_reasons.append("resource-contract-unresolved")
            try:
                _validate_material(element, element_id)
            except ValueError:
                admission_reasons.append("material-contract-unresolved")
            if admitted_target_kind == "particle":
                try:
                    _validate_anchor(element, element_id)
                except ValueError:
                    admission_reasons.append("source-anchor-contract-unresolved")
                try:
                    recipe = _require_object(
                        element.get("sourceRecipe"),
                        "diagnostic Sprite Particle sourceRecipe",
                    )
                    detail_particle = _require_object(
                        _require_object(
                            element.get("detail"), "diagnostic Sprite Particle detail"
                        ).get("particle"),
                        "diagnostic Sprite Particle particle detail",
                    )
                    if (
                        recipe.get("enabled") is not True
                        or recipe.get("rendererShape") != "sprite"
                        or not isinstance(detail_particle.get("billboard"), bool)
                    ):
                        raise ValueError("Sprite Particle renderer is unresolved.")
                except ValueError:
                    admission_reasons.append("sprite-particle-renderer-unresolved")
        elif (
            classification == MESH_PARTICLE
            and source_analysis["conversionEligibility"] == PARTICLE_REQUIRED
        ):
            admission_reasons.append("mesh-particle-product-excluded")
        elif source_analysis["conversionEligibility"] == UNKNOWN:
            admission_reasons.append("conversion-eligibility-unknown")
        else:
            admission_reasons.append("source-carrier-unsupported")

        admitted = not admission_reasons
        source_element_rows.append(
            {
                "sourceElementId": element_id,
                "sourceElementSha256": source_element_sha256(element),
                "sourceDocumentKind": source_analysis["sourceDocumentKind"],
                "sourceKind": source_analysis["sourceKind"],
                "rendererShape": source_analysis["rendererShape"],
                "sourceClassification": classification_key,
                "conversionEligibility": {
                    "status": source_analysis["conversionEligibility"],
                    "reasonCodes": source_analysis["reasonCodes"],
                },
                "fidelityWarningCodes": source_analysis[
                    "fidelityWarningCodes"
                ],
                "fidelityWarnings": source_analysis["fidelityWarnings"],
                "targetKindCandidates": source_analysis["targetKindCandidates"],
                "admittedTargetKind": (
                    admitted_target_kind if not admission_reasons else None
                ),
                "requiredRemovedModuleDisposition": source_analysis[
                    "requiredRemovedModuleDisposition"
                ],
                "requiredTargetAnchorPolicy": source_analysis[
                    "requiredTargetAnchorPolicy"
                ],
                "productAdmission": "ADMITTED" if admitted else "BLOCKED",
                "productAdmissionReasonCodes": admission_reasons,
            }
        )
        if admitted:
            if admitted_target_kind == "particle":
                fully_admitted_sprite_particle += 1
            else:
                fully_admitted_standalone += 1
            if selected_stage is not None:
                if admitted_target_kind == "particle":
                    selected_stage["fullyAdmittedSpriteParticleCount"] += 1
                else:
                    selected_stage["fullyAdmittedStandaloneCount"] += 1
                selected_stage["fullyAdmittedProductCarrierCount"] += 1

    return {
        "effectAssetId": expected_effect_id,
        "elementCount": len(elements),
        "classificationCounts": aggregate_counts,
        "fullyAdmittedStandaloneCount": fully_admitted_standalone,
        "fullyAdmittedSpriteParticleCount": fully_admitted_sprite_particle,
        "fullyAdmittedProductCarrierCount": (
            fully_admitted_standalone + fully_admitted_sprite_particle
        ),
        "unassignedElementCount": unassigned,
        "sourceElements": source_element_rows,
        "stages": stage_rows,
    }


def evaluate_skill_manifest(
    manifest: dict[str, Any], default_sprite_roll: float
) -> tuple[dict[str, Any], list[tuple[Path, dict[str, Any]]]]:
    if manifest.get("schema") != SKILL_SCHEMA or manifest.get("version") != SKILL_VERSION:
        raise ValueError("Skill materialization manifest schema/version is invalid.")
    materialization_id = _require_stable_token(
        manifest.get("materializationId"), "materializationId"
    )
    character_class = _require_string(
        manifest.get("characterClass"), "characterClass"
    )
    skill_id = manifest.get("skillId")
    if isinstance(skill_id, bool) or not isinstance(skill_id, int) or skill_id <= 0:
        raise ValueError("skillId must be a positive integer.")
    _require_string(manifest.get("inputSlot"), "inputSlot")
    status = _require_string(manifest.get("status"), "status")
    if status not in SKILL_STATUSES:
        raise ValueError(f"Unknown skill materialization status: {status}")
    blockers = _validate_blockers(manifest, status)
    _validate_evidence(manifest)

    source_contract = _require_object(manifest.get("source"), "source")
    source_effect_id = _require_effect_id(
        source_contract.get("effectAssetId"), "source.effectAssetId"
    )
    source_path = _resolve_data_path(
        source_contract.get("authoringPath"),
        "source.authoringPath",
        require_file=False,
    )
    expected_state = _require_string(
        source_contract.get("expectedState"), "source.expectedState"
    )
    if expected_state not in {"present", "missing"}:
        raise ValueError("source.expectedState must be present or missing.")
    if expected_state == "present" and not source_path.is_file():
        raise ValueError(f"Expected canonical source Effect is missing: {source_path}")
    if expected_state == "missing" and source_path.exists():
        raise ValueError(
            f"Blocked source state is stale; review the new canonical Effect: {source_path}"
        )
    if status in {READY, PRESERVE_EXISTING} and expected_state != "present":
        raise ValueError(f"{status} manifest requires a present canonical source.")
    if status == READY:
        expected_hash = _require_string(
            source_contract.get("sha256"), "source.sha256"
        )
        if SHA256.fullmatch(expected_hash) is None or _sha256_file(source_path) != expected_hash:
            raise ValueError("Canonical source changed after ready manifest approval.")

    sprite_roll = _require_number(
        manifest.get("spriteBillboardRollDegrees", default_sprite_roll),
        "spriteBillboardRollDegrees",
    )
    stages = _require_array(manifest.get("stages"), "stages")
    if not stages:
        raise ValueError("Skill materialization manifest has no stages.")
    stage_objects = [_require_object(raw_stage, "stage") for raw_stage in stages]
    preliminary_stage_order: list[int] = []
    stage_offsets: list[float] = []
    for stage in stage_objects:
        stage_index = stage.get("stageIndex")
        if (
            isinstance(stage_index, bool)
            or not isinstance(stage_index, int)
            or stage_index < 0
        ):
            raise ValueError("stageIndex must be a non-negative integer.")
        preliminary_stage_order.append(stage_index)
        if status == READY:
            stage_offsets.append(
                _require_non_negative_number(
                    stage.get("sourceTimelineOffsetSeconds"),
                    "stage sourceTimelineOffsetSeconds",
                )
            )
    if preliminary_stage_order != sorted(preliminary_stage_order):
        raise ValueError("Stage order is not stable.")
    if status == READY and any(
        current >= following
        for current, following in zip(stage_offsets, stage_offsets[1:])
    ):
        raise ValueError("Stage source timeline offsets are not strictly ordered.")
    seen_stage_indices: set[int] = set()
    stage_order: list[int] = []
    seen_targets: set[str] = set()
    outputs: list[tuple[Path, dict[str, Any]]] = []
    product_gate_results: list[dict[str, Any]] = []
    external_approximations: list[dict[str, Any]] = []
    source_document = (
        _load_json(source_path, "canonical source Effect") if status == READY else None
    )
    claimed_source_ids: set[str] = set()
    for stage_position, stage in enumerate(stage_objects):
        stage_index, target_effect_id, target_path = _validate_stage_common(stage, status)
        if stage_index in seen_stage_indices:
            raise ValueError(f"Duplicate stageIndex: {stage_index}")
        if target_effect_id in seen_targets:
            raise ValueError(f"Duplicate targetEffectAssetId: {target_effect_id}")
        seen_stage_indices.add(stage_index)
        stage_order.append(stage_index)
        seen_targets.add(target_effect_id)
        if status == BLOCKED:
            if stage.get("occurrences", []) not in ([], None):
                raise ValueError("Blocked stage must not claim source-backed occurrences.")
            if target_path.exists():
                external_approximations.append(
                    _validate_external_authored_approximation(
                        character_class=character_class,
                        skill_id=skill_id,
                        stage_index=stage_index,
                        target_effect_id=target_effect_id,
                        target_path=target_path,
                    )
                )
        elif status == PRESERVE_EXISTING:
            if not target_path.is_file():
                raise ValueError(f"Protected Authored document is missing: {target_path}")
            protected = _load_json(target_path, "protected Authored Effect")
            if protected.get("effectAssetId") != target_effect_id:
                raise ValueError("Protected Authored document has the wrong Effect asset ID.")
            product_gate_results.append(
                _validate_preserved_product_gate(
                    stage,
                    target_effect_id,
                    target_path,
                    protected,
                )
            )
        else:
            assert source_document is not None
            outputs.append(
                (
                    target_path,
                    build_stage_document(
                        source_document,
                        manifest,
                        stage,
                        sprite_roll,
                        source_timeline_end=(
                            stage_offsets[stage_position + 1]
                            if stage_position + 1 < len(stage_offsets)
                            else None
                        ),
                        claimed_source_ids=claimed_source_ids,
                    ),
                )
            )

    if stage_order != sorted(stage_order):
        raise ValueError("Stage order is not stable.")
    source_diagnostics = None
    if status == READY:
        assert source_document is not None
        source_diagnostics = _diagnose_source_document(
            source_document,
            {"effectAssetId": source_effect_id},
            stage_objects,
        )
    raw_diagnostic_contract = manifest.get("diagnosticSource")
    if raw_diagnostic_contract is not None:
        if status != BLOCKED:
            raise ValueError("Only blocked manifests may carry a diagnostic source.")
        diagnostic_contract = _require_object(
            raw_diagnostic_contract, "diagnosticSource"
        )
        diagnostic_path = _resolve_data_path(
            diagnostic_contract.get("authoringPath"),
            "diagnosticSource.authoringPath",
            require_file=True,
        )
        expected_hash = _require_string(
            diagnostic_contract.get("sha256"), "diagnosticSource.sha256"
        )
        if (
            SHA256.fullmatch(expected_hash) is None
            or _sha256_file(diagnostic_path) != expected_hash
        ):
            raise ValueError("Diagnostic source changed after manifest review.")
        source_diagnostics = _diagnose_source_document(
            _load_json(diagnostic_path, "diagnostic source Effect"),
            diagnostic_contract,
            stage_objects,
        )

    result = {
        "materializationId": materialization_id,
        "characterClass": character_class,
        "skillId": skill_id,
        "sourceEffectAssetId": source_effect_id,
        "status": status,
        "targetCount": len(stages),
        "materializedTargetCount": len(outputs),
        "blockers": blockers,
        "productGates": product_gate_results,
    }
    if source_diagnostics is not None:
        result["sourceDiagnostics"] = source_diagnostics
    if external_approximations:
        result["externalApproximations"] = external_approximations
    return result, outputs


def materialize_set(
    set_manifest_path: Path = DEFAULT_SET_MANIFEST,
) -> tuple[dict[str, Any], list[tuple[Path, dict[str, Any]]]]:
    set_manifest_path = set_manifest_path.resolve()
    try:
        set_manifest_path.relative_to(CORRECTION_ROOT)
    except ValueError as error:
        raise ValueError("Materialization set escaped AuthoredCorrections.") from error
    set_manifest = _load_json(set_manifest_path, "materialization set")
    if (
        set_manifest.get("schema") != SET_SCHEMA
        or set_manifest.get("version") != SET_VERSION
    ):
        raise ValueError("Materialization set schema/version is invalid.")
    set_id = _require_stable_token(set_manifest.get("setId"), "setId")
    default_sprite_roll = _require_number(
        set_manifest.get("defaultSpriteBillboardRollDegrees"),
        "defaultSpriteBillboardRollDegrees",
    )
    carrier_policy = _require_object(
        set_manifest.get("sourceCarrierPolicy"), "sourceCarrierPolicy"
    )
    if carrier_policy != SOURCE_CARRIER_POLICY:
        raise ValueError(
            "sourceCarrierPolicy must keep all four source kinds distinct, convert "
            "only certified singleton Cascade carriers, exclude Mesh Particle "
            "product output, and preserve an executable Sprite Particle renderer."
        )
    manifest_paths = _require_array(
        set_manifest.get("skillManifests"), "skillManifests"
    )
    if not manifest_paths:
        raise ValueError("Materialization set has no skill manifests.")
    expected_skill_count = set_manifest.get("expectedSkillCount")
    if (
        isinstance(expected_skill_count, bool)
        or not isinstance(expected_skill_count, int)
        or expected_skill_count <= 0
        or expected_skill_count != len(manifest_paths)
    ):
        raise ValueError("expectedSkillCount does not match skillManifests.")

    results: list[dict[str, Any]] = []
    outputs: list[tuple[Path, dict[str, Any]]] = []
    skill_keys: set[tuple[str, int]] = set()
    target_paths: set[Path] = set()
    for index, manifest_value in enumerate(manifest_paths):
        manifest_path = _resolve_data_path(
            manifest_value, f"skillManifests[{index}]", require_file=True
        )
        try:
            manifest_path.relative_to(CORRECTION_ROOT)
        except ValueError as error:
            raise ValueError("Skill manifest escaped AuthoredCorrections.") from error
        manifest = _load_json(manifest_path, f"skill manifest {index}")
        result, skill_outputs = evaluate_skill_manifest(
            manifest, default_sprite_roll
        )
        key = (result["characterClass"], result["skillId"])
        if key in skill_keys:
            raise ValueError(f"Materialization set has a duplicate skill: {key}")
        skill_keys.add(key)
        for target_path, document in skill_outputs:
            if target_path in target_paths:
                raise ValueError(f"Materialization set has a duplicate target: {target_path}")
            target_paths.add(target_path)
            outputs.append((target_path, document))
        result["manifestPath"] = manifest_path.relative_to(DATA_ROOT).as_posix()
        results.append(result)

    status = {
        "schema": STATUS_SCHEMA,
        "version": STATUS_VERSION,
        "setId": set_id,
        "skills": results,
        "summary": {
            "skillCount": len(results),
            "readyCount": sum(result["status"] == READY for result in results),
            "blockedCount": sum(result["status"] == BLOCKED for result in results),
            "preservedCount": sum(
                result["status"] == PRESERVE_EXISTING
                and result["targetCount"] == len(result["productGates"])
                and all(gate["status"] == "passed" for gate in result["productGates"])
                for result in results
            ),
            "pendingOutputCount": len(outputs),
        },
    }
    return status, outputs


def write_documents(outputs: Iterable[tuple[Path, dict[str, Any]]]) -> None:
    staged = list(outputs)
    for path, _ in staged:
        if path.exists():
            raise FileExistsError(
                f"Authored document already exists; tune it with the Effect Tool: {path}"
            )
    created: list[Path] = []
    try:
        for path, document in staged:
            path.parent.mkdir(parents=True, exist_ok=True)
            descriptor = os.open(
                path,
                os.O_WRONLY | os.O_CREAT | os.O_EXCL,
                0o644,
            )
            created.append(path)
            with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as stream:
                json.dump(document, stream, ensure_ascii=False, indent=2)
                stream.write("\n")
                stream.flush()
                os.fsync(stream.fileno())
    except Exception:
        for path in created:
            try:
                path.unlink()
            except FileNotFoundError:
                pass
        raise


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest-set", type=Path, default=DEFAULT_SET_MANIFEST)
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()
    status, outputs = materialize_set(args.manifest_set)
    if args.write:
        write_documents(outputs)
    print(json.dumps(status, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
