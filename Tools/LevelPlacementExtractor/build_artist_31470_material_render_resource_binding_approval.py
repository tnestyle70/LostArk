#!/usr/bin/env python3
"""Build and validate the Artist 31470 reconstructed render-resource approval.

This artifact is an explicitly reviewed reconstruction policy.  It binds the
frozen reconstructed runtime program to exact texture-field decisions and
standard D3D11 descriptor expectations without claiming source fidelity or
opening Product/runtime execution.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import sys
from pathlib import Path
from typing import Any, Iterable


def discover_repository_root() -> Path:
    current = Path.cwd().resolve()
    for candidate in (current, *current.parents):
        if (
            (candidate / ".git").exists()
            and (candidate / "Tools/EffectPipeline").is_dir()
            and (candidate / "Data/Effects/Imported/Artist/Materials").is_dir()
        ):
            return candidate
    raise RuntimeError("cannot locate canonical LostArk repository root")


ROOT = discover_repository_root()
SCRIPT_DIR = ROOT / "Tools/LevelPlacementExtractor"
EFFECT_PIPELINE_DIR = ROOT / "Tools/EffectPipeline"
for search_path in (SCRIPT_DIR, EFFECT_PIPELINE_DIR):
    if str(search_path) not in sys.path:
        sys.path.insert(0, str(search_path))

import artist_31470_material_render_resource_binding_approval as approval_module
import build_artist_31470_reconstructed_runtime_program as program_module
import effect_source_contract_io as strict_io


SCHEMA = "lostark.artist-31470-material-render-resource-binding-approved-receipt"
FORMAT_VERSION = 1
APPROVAL_ID = "ARTIST_31470_MATERIAL_RENDER_RESOURCE_BINDING_APPROVED_V1"
DEFAULT_PROGRAM = ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-runtime-program.candidate.json"
)
DEFAULT_OUTPUT = ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-render-resource-binding-approved-v1.receipt.json"
)
DEFAULT_TOOL = Path(__file__).resolve()
EFFECT_SHADER = ROOT / "Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli"
ENGINE_SHADER_DEFINES = ROOT / "Engine/Bin/ShaderFiles/Engine_Shader_Defines.hlsli"

PROGRAM_BYTE_COUNT = 15_121_873
PROGRAM_RAW_SHA256 = "430ed1aa42a34e23d1f216a69c6f51e81a8cbcdbb03318930894e0dfe16cd6c6"
PROGRAM_SHA256 = "0666164bce946fd3b7e72dd92422b21a13e58d3388a3e3264ab30b8065e9c802"
EFFECT_SHADER_TRACKED_TEXT_SHA256 = (
    "34ca3d68ea1e2d8714e975c2dc4c0ad245a399c9f4f7aee759bc993485675baf"
)
ENGINE_SHADER_DEFINES_TRACKED_TEXT_SHA256 = (
    "869b1aec6f1a5839937b82f19cf88e4b38fe6890a4b299818709b483ed0a80f8"
)

EXPECTED_RECIPES = 27
EXPECTED_RENDERER_BINDINGS = 57
EXPECTED_DESCRIPTOR_DECISIONS = 46
EXPECTED_BLEND_DECISIONS = 27
EXPECTED_RASTER_DECISIONS = 18
EXPECTED_DEPTH_DECISIONS = 1
SECOND_TEXTURE_BIT = 1
DISTORTION_BIT = 256

BLOCKERS = [
    "AUTOMATED_WARP_DESCRIPTOR_AND_BINDING_PROBE_REQUIRED",
    "MANUAL_ARTIST_F_EYE_VALIDATION_REQUIRED",
    "PRODUCT_RUNTIME_RENDER_RESOURCE_CONSUMER_NOT_ADMITTED",
    "RECONSTRUCTED_POLICY_IS_NOT_SOURCE_EVIDENCE",
]

NEUTRAL_BASE_WHITE = "RECONSTRUCTED_UNIT_BASE_WHITE_RGBA"
NEUTRAL_SECOND_WHITE = "RECONSTRUCTED_SECOND_TEXTURE_MULTIPLY_IDENTITY_RGBA"
NEUTRAL_DISTORTION_HALF = "RECONSTRUCTED_SIGNED_DISTORTION_ZERO_RGBA"
NEUTRAL_UNUSED_ZERO = "RECONSTRUCTED_UNUSED_ZERO_RGBA"

ROOT_KEYS = (
    "schema", "formatVersion", "approvalId", "characterClass", "skillId",
    "inputSlot", "approvalContract", "sourceEvidence", "neutralProviders",
    "recipeTextureBindings", "rendererSlotBindings", "renderStateDescriptors",
    "blockerProjection", "admission", "summary", "receiptSha256",
)
_APPROVED_PROGRAM_CACHE: dict[str, Any] | None = None


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def seal_row(row: dict[str, Any]) -> dict[str, Any]:
    require("rowSha256" not in row, "row was already sealed")
    row["rowSha256"] = canonical_sha256(row)
    return row


def strict_ordered_equal(actual: Any, expected: Any, label: str = "root") -> None:
    require(type(actual) is type(expected), f"{label}: strict type changed")
    if isinstance(expected, dict):
        require(tuple(actual.keys()) == tuple(expected.keys()), f"{label}: key/order changed")
        for key in expected:
            strict_ordered_equal(actual[key], expected[key], f"{label}.{key}")
    elif isinstance(expected, list):
        require(len(actual) == len(expected), f"{label}: list length changed")
        for index, (left, right) in enumerate(zip(actual, expected, strict=True)):
            strict_ordered_equal(left, right, f"{label}[{index}]")
    else:
        require(actual == expected, f"{label}: value changed")


def validate_recursive_types(value: Any, label: str = "root") -> None:
    if value is None or isinstance(value, (str, bool)):
        return
    if isinstance(value, int):
        return
    if isinstance(value, float):
        require(math.isfinite(value), f"{label}: non-finite float")
        return
    if isinstance(value, list):
        for index, item in enumerate(value):
            validate_recursive_types(item, f"{label}[{index}]")
        return
    if isinstance(value, dict):
        for key, item in value.items():
            require(isinstance(key, str), f"{label}: non-string object key")
            validate_recursive_types(item, f"{label}.{key}")
        return
    raise ValueError(f"{label}: unsupported recursive type {type(value).__name__}")


def _read_program_bytes(path: Path) -> tuple[bytes, dict[str, Any]]:
    raw = path.read_bytes()
    require(not raw.startswith(b"\xef\xbb\xbf"), "program must be UTF-8 without BOM")
    require(len(raw) == PROGRAM_BYTE_COUNT, "frozen program byte-count mismatch")
    require(hashlib.sha256(raw).hexdigest() == PROGRAM_RAW_SHA256,
            "frozen program raw SHA-256 mismatch")
    require(b"\r" not in raw, "frozen program must remain canonical LF")
    program = program_module.load_json_bytes(raw, str(path))
    program_module.validate_program(program)
    require(program["programSha256"] == PROGRAM_SHA256, "frozen program identity mismatch")
    return raw, program


def _validate_current_artifact(authority: dict[str, Any]) -> None:
    path = ROOT / authority["path"]
    require(path.is_file(), f"current authority is missing: {authority['path']}")
    require(
        strict_io.tracked_text_sha256(path) == authority["trackedTextSha256"],
        f"current authority tracked-text identity changed: {authority['artifactId']}",
    )
    parsed = strict_io.load_strict_json_object(path)
    require(
        canonical_sha256(parsed) == authority["canonicalJsonSha256"],
        f"current authority canonical identity changed: {authority['artifactId']}",
    )
    self_field = authority["selfHashField"]
    if self_field:
        require(parsed.get(self_field) == authority["selfSha256"],
                f"current authority self identity changed: {authority['artifactId']}")
        unsigned = copy.deepcopy(parsed)
        unsigned.pop(self_field)
        require(canonical_sha256(unsigned) == authority["selfSha256"],
                f"current authority self seal is invalid: {authority['artifactId']}")


def validate_program_authorities(program: dict[str, Any]) -> None:
    required = {"materialRuntime", "materialPolicy", "materialTextureBinding"}
    rows = {row["artifactId"]: row for row in program["inputArtifacts"]}
    require(required <= set(rows), "program lacks required Material authorities")
    for artifact_id in sorted(required):
        _validate_current_artifact(rows[artifact_id])
    require(
        strict_io.tracked_text_sha256(EFFECT_SHADER) == EFFECT_SHADER_TRACKED_TEXT_SHA256,
        "Effect shader implementation mapping reference changed",
    )
    require(
        strict_io.tracked_text_sha256(ENGINE_SHADER_DEFINES)
        == ENGINE_SHADER_DEFINES_TRACKED_TEXT_SHA256,
        "Engine shader-state mapping reference changed",
    )


def load_approved_program(path: Path = DEFAULT_PROGRAM) -> dict[str, Any]:
    global _APPROVED_PROGRAM_CACHE
    raw = path.read_bytes()
    require(not raw.startswith(b"\xef\xbb\xbf"), "program must be UTF-8 without BOM")
    require(len(raw) == PROGRAM_BYTE_COUNT, "frozen program byte-count mismatch")
    require(hashlib.sha256(raw).hexdigest() == PROGRAM_RAW_SHA256,
            "frozen program raw SHA-256 mismatch")
    require(b"\r" not in raw, "frozen program must remain canonical LF")
    use_cache = path.resolve() == DEFAULT_PROGRAM.resolve()
    if not use_cache or _APPROVED_PROGRAM_CACHE is None:
        _, program = _read_program_bytes(path)
        if use_cache:
            _APPROVED_PROGRAM_CACHE = program
    else:
        program = _APPROVED_PROGRAM_CACHE
    validate_program_authorities(program)
    return program


def _texture_provider(
    binding: dict[str, Any],
    input_row: dict[str, Any],
    policy_row: dict[str, Any],
    basis: str,
) -> dict[str, Any]:
    return {
        "providerKind": "MATERIAL_TEXTURE_BINDING",
        "neutralProviderId": "",
        "materialInputFieldId": input_row["fieldId"],
        "materialInputRowSha256": input_row["rowSha256"],
        "textureBindingId": binding["bindingId"],
        "textureBindingRowSha256": binding["rowSha256"],
        "samplerPolicyRowId": binding["samplerPolicyRowId"],
        "samplerPolicyRowSha256": policy_row["rowSha256"],
        "runtimeAssetId": binding["runtimeAssetId"],
        "selectionBasis": basis,
    }


def _neutral_provider(provider_id: str, basis: str) -> dict[str, Any]:
    return {
        "providerKind": "NEUTRAL_CONSTANT",
        "neutralProviderId": provider_id,
        "materialInputFieldId": "",
        "materialInputRowSha256": "",
        "textureBindingId": "",
        "textureBindingRowSha256": "",
        "samplerPolicyRowId": "",
        "samplerPolicyRowSha256": "",
        "runtimeAssetId": "",
        "selectionBasis": basis,
    }


def build_neutral_providers() -> list[dict[str, Any]]:
    specifications = [
        (
            NEUTRAL_BASE_WHITE,
            [1.0, 1.0, 1.0, 1.0],
            "TEXTURE0_UNIT_SAMPLE_FOR_EXPLICITLY_ABSENT_TEXTURE_FIELDS",
            "The evaluator begins with texture0; unit white preserves a neutral base sample.",
            1.0,
            0.0,
        ),
        (
            NEUTRAL_SECOND_WHITE,
            [1.0, 1.0, 1.0, 1.0],
            "SECOND_TEXTURE_MULTIPLY_IDENTITY",
            "The reconstructed factor 0.5 + 0.5 * texture1 is exactly one.",
            1.0,
            1.0,
        ),
        (
            NEUTRAL_DISTORTION_HALF,
            [0.5, 0.5, 0.5, 1.0],
            "SIGNED_DISTORTION_ZERO_OFFSET",
            "The reconstructed signed offset texture1 * 2 - 1 is exactly zero; "
            "a simultaneous second-texture factor is explicitly 0.75 and requires eye validation.",
            0.75,
            0.0,
        ),
        (
            NEUTRAL_UNUSED_ZERO,
            [0.0, 0.0, 0.0, 0.0],
            "EXPLICIT_UNUSED_TEXTURE_REGISTER",
            "The feature mask consumes neither second texture nor distortion input.",
            0.5,
            -1.0,
        ),
    ]
    rows: list[dict[str, Any]] = []
    for order, (provider_id, rgba, semantic, rationale, factor, signed_offset) in enumerate(specifications):
        rows.append(seal_row({
            "neutralProviderId": provider_id,
            "order": order,
            "rgbaF32": rgba,
            "evaluatorSemantic": semantic,
            "secondaryMultiplyFactor": factor,
            "signedDistortionOffset": signed_offset,
            "rationale": rationale,
            "sourceExact": False,
            "requiresAutomatedWARPProbe": True,
            "requiresManualEyeValidation": True,
            "runtimeExecutionAdmission": False,
            "product": False,
        }))
    return rows


def _selected_renderer_bindings(program: dict[str, Any]) -> list[dict[str, Any]]:
    inputs = {row["fieldId"]: row for row in program["materialInputs"]}
    bindings = {row["bindingId"]: row for row in program["materialTextureBindings"]}
    policies = {row["policyRowId"]: row for row in program["materialPolicyRows"]}
    occurrences = {row["occurrenceId"]: row for row in program["materialOccurrences"]}
    result: list[dict[str, Any]] = []
    ambiguous_seen: set[str] = set()

    for order, resource in enumerate(program["rendererTextureResources"]):
        occurrence = occurrences[resource["materialOccurrenceId"]]
        candidates = [
            row for row in program["materialTextureBindings"]
            if resource["materialOccurrenceId"] in row["materialOccurrenceIds"]
            and row["runtimeAssetId"] == resource["assetId"]
        ]
        require(len(candidates) in (1, 2),
                f"renderer row has unsupported candidate count: {resource['textureResourceId']}")
        approved_field = approval_module.APPROVED_AMBIGUOUS_RENDERER_BINDINGS.get(
            resource["textureResourceId"]
        )
        if len(candidates) == 1:
            require(approved_field is None,
                    f"unique renderer row unexpectedly has an ambiguity override: {resource['textureResourceId']}")
            selected = candidates[0]
            decision_basis = "UNIQUE_OCCURRENCE_AND_EXACT_ASSET_RECONSTRUCTION"
            rationale = (
                "The frozen occurrence and full Resources-relative asset ID produce one exact "
                "Material texture-field candidate."
            )
        else:
            require(approved_field is not None,
                    f"ambiguous renderer row lacks explicit approval: {resource['textureResourceId']}")
            selected_rows = [
                row for row in candidates if row["materialInputFieldId"] == approved_field
            ]
            require(len(selected_rows) == 1,
                    f"approved ambiguity choice is not a candidate: {resource['textureResourceId']}")
            selected = selected_rows[0]
            ambiguous_seen.add(resource["textureResourceId"])
            decision_basis = "EXPLICIT_INDEPENDENT_AMBIGUITY_APPROVAL"
            rationale = (
                "An independent review explicitly selected this field from the two exact "
                "occurrence/asset candidates; no basename or semantic-role guess is executable."
            )

        input_row = inputs[selected["materialInputFieldId"]]
        policy_row = policies[selected["samplerPolicyRowId"]]
        candidate_projection = [
            {
                "materialInputFieldId": row["materialInputFieldId"],
                "materialInputRowSha256": inputs[row["materialInputFieldId"]]["rowSha256"],
                "textureBindingId": row["bindingId"],
                "textureBindingRowSha256": row["rowSha256"],
                "normalizedParameterName": inputs[row["materialInputFieldId"]]["normalizedParameterName"],
            }
            for row in candidates
        ]
        result.append(seal_row({
            "rendererBindingDecisionId": f"renderer-material-input-binding-{order:03d}",
            "order": order,
            "textureResourceId": resource["textureResourceId"],
            "rendererResourceRowSha256": resource["rowSha256"],
            "materialOccurrenceId": occurrence["occurrenceId"],
            "materialOccurrenceRowSha256": occurrence["rowSha256"],
            "recipeId": occurrence["recipeId"],
            "slotId": resource["slotId"],
            "runtimeAssetId": resource["assetId"],
            "candidateCount": len(candidate_projection),
            "candidates": candidate_projection,
            "selectedMaterialInputFieldId": input_row["fieldId"],
            "selectedMaterialInputRowSha256": input_row["rowSha256"],
            "selectedNormalizedParameterName": input_row["normalizedParameterName"],
            "selectedTextureBindingId": selected["bindingId"],
            "selectedTextureBindingRowSha256": selected["rowSha256"],
            "selectedSamplerPolicyRowId": selected["samplerPolicyRowId"],
            "selectedSamplerPolicyRowSha256": policy_row["rowSha256"],
            "decisionBasis": decision_basis,
            "rationale": rationale,
            "sourceExact": False,
            "requiresAutomatedWARPProbe": True,
            "requiresManualEyeValidation": True,
            "runtimeExecutionAdmission": False,
            "product": False,
        }))

    require(ambiguous_seen == set(approval_module.APPROVED_AMBIGUOUS_RENDERER_BINDINGS),
            "the exact three-row ambiguity approval projection changed")
    require(len(result) == EXPECTED_RENDERER_BINDINGS, "renderer decision denominator changed")
    return result


def _distortion_strength(recipe: dict[str, Any], feature_mask: int) -> float:
    samples = recipe["numericBindingSamples"]
    require(samples, f"recipe lacks numeric binding samples: {recipe['recipeId']}")
    values = [sample["params1"][2] for sample in samples]
    require(all(type(value) is float and math.isfinite(value) for value in values),
            f"recipe distortion-strength ABI is not exact finite F64: {recipe['recipeId']}")
    require(all(value == values[0] for value in values),
            f"recipe distortion strength changes across frozen samples: {recipe['recipeId']}")
    if not feature_mask & DISTORTION_BIT:
        return 0.0
    return values[0]


def _secondary_neutral_id(feature_mask: int, distortion_strength: float) -> str:
    second = bool(feature_mask & SECOND_TEXTURE_BIT)
    distortion = bool(feature_mask & DISTORTION_BIT)
    if distortion and distortion_strength != 0.0:
        return NEUTRAL_DISTORTION_HALF
    if second:
        return NEUTRAL_SECOND_WHITE
    return NEUTRAL_UNUSED_ZERO


def _first_renderer_choice(
    rows: list[dict[str, Any]],
    slot_priority: Iterable[str],
    excluded_binding_id: str = "",
    excluded_logical_texture_path: str = "",
    bindings: dict[str, dict[str, Any]] | None = None,
) -> dict[str, Any] | None:
    for slot in slot_priority:
        for row in rows:
            binding_id = row["selectedTextureBindingId"]
            if row["slotId"] != slot or binding_id == excluded_binding_id:
                continue
            if excluded_logical_texture_path:
                require(bindings is not None and binding_id in bindings,
                        "renderer texture choice lacks a binding identity")
                if bindings[binding_id]["logicalTexturePath"].casefold() == (
                    excluded_logical_texture_path.casefold()
                ):
                    continue
            return row
    return None


def _provider_from_binding_id(
    binding_id: str,
    basis: str,
    bindings: dict[str, dict[str, Any]],
    inputs: dict[str, dict[str, Any]],
    policies: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    binding = bindings[binding_id]
    return _texture_provider(
        binding,
        inputs[binding["materialInputFieldId"]],
        policies[binding["samplerPolicyRowId"]],
        basis,
    )


def build_recipe_decisions(
    program: dict[str, Any], renderer_rows: list[dict[str, Any]]
) -> list[dict[str, Any]]:
    families = {row["familyId"]: row for row in program["materialFamilies"]}
    inputs = {row["fieldId"]: row for row in program["materialInputs"]}
    bindings = {row["bindingId"]: row for row in program["materialTextureBindings"]}
    policies = {row["policyRowId"]: row for row in program["materialPolicyRows"]}
    result: list[dict[str, Any]] = []

    for order, recipe in enumerate(program["materialRecipes"]):
        family = families[recipe["familyId"]]
        feature_mask = family["featureMask"]
        recipe_texture_inputs = [
            row for row in program["materialInputs"]
            if row["recipeId"] == recipe["recipeId"]
            and row["fieldKind"] == "texture"
        ]
        runtime_required_texture_inputs = [
            row for row in recipe_texture_inputs
            if row["bindingOrigin"] == "SELF_DEFAULT"
        ]
        recipe_bindings = [
            row for row in program["materialTextureBindings"]
            if row["recipeId"] == recipe["recipeId"]
        ]
        renderer_choices = [
            row for row in renderer_rows if row["recipeId"] == recipe["recipeId"]
        ]
        distortion_strength = _distortion_strength(recipe, feature_mask)
        secondary_neutral = _secondary_neutral_id(feature_mask, distortion_strength)

        texture0_renderer = _first_renderer_choice(
            renderer_choices, ("base", "emissive", "mask", "dissolve", "noise")
        )
        if texture0_renderer is not None:
            texture0 = _provider_from_binding_id(
                texture0_renderer["selectedTextureBindingId"],
                "REVIEW_CANDIDATE_RENDERER_SLOT_PRIORITY_BASE_EMISSIVE_MASK_DISSOLVE_NOISE",
                bindings, inputs, policies,
            )
        elif recipe_bindings:
            texture0 = _provider_from_binding_id(
                recipe_bindings[0]["bindingId"],
                "REVIEW_CANDIDATE_FROZEN_PROGRAM_TEXTURE_BINDING_ORDER",
                bindings, inputs, policies,
            )
        else:
            require(not runtime_required_texture_inputs,
                    "SELF_DEFAULT texture inputs have no runtime bindings: "
                    f"{recipe['recipeId']}")
            texture0 = _neutral_provider(
                NEUTRAL_BASE_WHITE,
                "EXPLICIT_EVALUATOR_BASE_NEUTRAL_FOR_RECIPE_WITH_NO_TEXTURE_FIELDS",
            )

        needs_texture1 = bool(feature_mask & (SECOND_TEXTURE_BIT | DISTORTION_BIT))
        texture0_binding_id = texture0["textureBindingId"]
        texture0_logical_path = (
            bindings[texture0_binding_id]["logicalTexturePath"]
            if texture0_binding_id else ""
        )
        if needs_texture1:
            texture1_renderer = _first_renderer_choice(
                renderer_choices,
                ("noise", "dissolve", "mask", "emissive", "base"),
                texture0_binding_id,
                texture0_logical_path,
                bindings,
            )
            if texture1_renderer is not None:
                texture1 = _provider_from_binding_id(
                    texture1_renderer["selectedTextureBindingId"],
                    "REVIEW_CANDIDATE_RENDERER_SLOT_PRIORITY_NOISE_DISSOLVE_MASK_EMISSIVE_BASE",
                    bindings, inputs, policies,
                )
            else:
                distinct = [
                    row for row in recipe_bindings
                    if row["bindingId"] != texture0_binding_id
                    and row["logicalTexturePath"].casefold()
                    != texture0_logical_path.casefold()
                ]
                if distinct:
                    texture1 = _provider_from_binding_id(
                        distinct[0]["bindingId"],
                        "REVIEW_CANDIDATE_DISTINCT_FROZEN_PROGRAM_TEXTURE_BINDING_ORDER",
                        bindings, inputs, policies,
                    )
                else:
                    texture1 = _neutral_provider(
                        secondary_neutral,
                        "EXPLICIT_EVALUATOR_SECONDARY_NEUTRAL_FOR_MISSING_DISTINCT_TEXTURE_FIELD",
                    )
        else:
            texture1 = _neutral_provider(
                NEUTRAL_UNUSED_ZERO,
                "EXPLICIT_UNUSED_PROVIDER_FEATURE_MASK_CONSUMES_NO_SECONDARY_TEXTURE",
            )

        if texture0["providerKind"] == "NEUTRAL_CONSTANT":
            require(not recipe_bindings and not runtime_required_texture_inputs,
                    "texture0 neutral chosen despite runtime-bindable texture fields: "
                    f"{recipe['recipeId']}")
            require(texture0["neutralProviderId"] == NEUTRAL_BASE_WHITE,
                    f"invalid base neutral: {recipe['recipeId']}")
        if texture1["providerKind"] == "NEUTRAL_CONSTANT":
            require(texture1["neutralProviderId"] == secondary_neutral,
                    f"secondary neutral does not match evaluator semantics: {recipe['recipeId']}")
        elif needs_texture1 and texture0["providerKind"] == "MATERIAL_TEXTURE_BINDING":
            require(
                bindings[texture1["textureBindingId"]]["logicalTexturePath"].casefold()
                != texture0_logical_path.casefold(),
                f"secondary texture aliases the primary resource: {recipe['recipeId']}",
            )

        result.append(seal_row({
            "recipeTextureDecisionId": f"recipe-texture-binding-{order:02d}",
            "order": order,
            "recipeId": recipe["recipeId"],
            "recipeRowSha256": recipe["rowSha256"],
            "familyId": family["familyId"],
            "familyRowSha256": family["rowSha256"],
            "featureMask": feature_mask,
            "secondTextureOperationEnabled": bool(feature_mask & SECOND_TEXTURE_BIT),
            "distortionOperationEnabled": bool(feature_mask & DISTORTION_BIT),
            "distortionStrengthF32": distortion_strength,
            "candidateTextureBindingIds": [row["bindingId"] for row in recipe_bindings],
            "candidateTextureBindingRowSha256": [row["rowSha256"] for row in recipe_bindings],
            "texture0Provider": texture0,
            "texture1Provider": texture1,
            "neutralFallbackDecision": {
                "texture0NeutralProviderId": NEUTRAL_BASE_WHITE,
                "texture1NeutralProviderId": secondary_neutral,
                "neutralProviderApplicationPolicy": "ONLY_WHEN_THIS_APPROVAL_EXPLICITLY_SELECTS_NEUTRAL",
                "materialBindingFailurePolicy": "FAIL_CLOSED_TRANSACTION_ROLLBACK",
                "rationale": (
                    "Neutral constants model evaluator semantics only; a selected Material binding "
                    "that fails staging is never silently replaced."
                ),
            },
            "decisionBasis": "EXPLICITLY_ENUMERATED_RECONSTRUCTED_POLICY_APPROVAL",
            "sourceExact": False,
            "requiresAutomatedWARPProbe": True,
            "requiresManualEyeValidation": True,
            "runtimeExecutionAdmission": False,
            "product": False,
        }))

    require(len(result) == EXPECTED_RECIPES, "recipe decision denominator changed")
    return result


def _default_render_target() -> dict[str, Any]:
    return {
        "BlendEnable": False,
        "SrcBlend": 2,
        "DestBlend": 1,
        "BlendOp": 1,
        "SrcBlendAlpha": 2,
        "DestBlendAlpha": 1,
        "BlendOpAlpha": 1,
        "RenderTargetWriteMask": 15,
    }


def _effect_blend_descriptor(blend_mode: str) -> tuple[str, dict[str, Any]]:
    require(blend_mode in {"blend_translucent", "blend_additive", "blend_masked"},
            f"unsupported reconstructed blend mode: {blend_mode}")
    targets = [_default_render_target() for _ in range(8)]
    targets[1] = {
        "BlendEnable": True,
        "SrcBlend": 2,
        "DestBlend": 2,
        "BlendOp": 1,
        "SrcBlendAlpha": 2,
        "DestBlendAlpha": 2,
        "BlendOpAlpha": 1,
        "RenderTargetWriteMask": 3,
    }
    if blend_mode == "blend_translucent":
        state_name = "BS_EffectAlpha"
        targets[0] = {
            "BlendEnable": True,
            "SrcBlend": 5,
            "DestBlend": 6,
            "BlendOp": 1,
            "SrcBlendAlpha": 2,
            "DestBlendAlpha": 6,
            "BlendOpAlpha": 1,
            "RenderTargetWriteMask": 15,
        }
    elif blend_mode == "blend_additive":
        state_name = "BS_EffectAdditive"
        targets[0] = {
            "BlendEnable": True,
            "SrcBlend": 5,
            "DestBlend": 2,
            "BlendOp": 1,
            "SrcBlendAlpha": 2,
            "DestBlendAlpha": 2,
            "BlendOpAlpha": 1,
            "RenderTargetWriteMask": 15,
        }
    else:
        state_name = "BS_EffectOpaque"
    return state_name, {
        "AlphaToCoverageEnable": False,
        "IndependentBlendEnable": True,
        "RenderTarget": targets,
    }


def _raster_descriptor() -> dict[str, Any]:
    return {
        "FillMode": 3,
        "CullMode": 1,
        "FrontCounterClockwise": False,
        "DepthBias": 0,
        "DepthBiasClamp": 0.0,
        "SlopeScaledDepthBias": 0.0,
        "DepthClipEnable": True,
        "ScissorEnable": False,
        "MultisampleEnable": False,
        "AntialiasedLineEnable": False,
    }


def _depth_disabled_descriptor() -> dict[str, Any]:
    face = {
        "StencilFailOp": 1,
        "StencilDepthFailOp": 1,
        "StencilPassOp": 1,
        "StencilFunc": 8,
    }
    return {
        "DepthEnable": False,
        "DepthWriteMask": 0,
        "DepthFunc": 2,
        "StencilEnable": False,
        "StencilReadMask": 255,
        "StencilWriteMask": 255,
        "FrontFace": copy.deepcopy(face),
        "BackFace": copy.deepcopy(face),
    }


def build_descriptor_decisions(program: dict[str, Any]) -> list[dict[str, Any]]:
    recipes = {row["recipeId"]: row for row in program["materialRecipes"]}
    selected: list[tuple[dict[str, Any], str, str, dict[str, Any], str, str]] = []
    for binding in program["materialRenderBindings"]:
        field = binding["fieldName"]
        if field == "blendmode":
            state_name, descriptor = _effect_blend_descriptor(binding["enumValue"])
            selected.append((
                binding, "D3D11_BLEND_DESC", state_name, descriptor,
                "Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli",
                EFFECT_SHADER_TRACKED_TEXT_SHA256,
            ))
        elif field == "twosided" and binding["boolValue"] is True:
            selected.append((
                binding, "D3D11_RASTERIZER_DESC", "RS_Cull_None", _raster_descriptor(),
                "Engine/Bin/ShaderFiles/Engine_Shader_Defines.hlsli",
                ENGINE_SHADER_DEFINES_TRACKED_TEXT_SHA256,
            ))
        elif field == "bdisabledepthtest" and binding["boolValue"] is True:
            selected.append((
                binding, "D3D11_DEPTH_STENCIL_DESC", "DSS_ZNone",
                _depth_disabled_descriptor(),
                "Engine/Bin/ShaderFiles/Engine_Shader_Defines.hlsli",
                ENGINE_SHADER_DEFINES_TRACKED_TEXT_SHA256,
            ))

    rows: list[dict[str, Any]] = []
    for order, (binding, kind, state_name, descriptor, path, tracked_sha) in enumerate(selected):
        recipe = recipes[binding["recipeId"]]
        source_value: bool | str
        source_value = binding["enumValue"] if binding["valueVariant"] == "ENUM_STRING" else binding["boolValue"]
        rows.append(seal_row({
            "renderStateDecisionId": f"render-state-descriptor-{order:02d}",
            "order": order,
            "renderBindingId": binding["renderBindingId"],
            "renderBindingRowSha256": binding["rowSha256"],
            "recipeId": recipe["recipeId"],
            "recipeRowSha256": recipe["rowSha256"],
            "fieldName": binding["fieldName"],
            "approvedFieldValue": source_value,
            "descriptorKind": kind,
            "standardMappingId": f"{path}::{state_name}",
            "implementationReferencePath": path,
            "implementationReferenceTrackedTextSha256": tracked_sha,
            "implementationStateName": state_name,
            "expectedDescriptor": descriptor,
            "decisionBasis": (
                "EXPLICIT_RECONSTRUCTED_STANDARD_D3D11_MAPPING; implementation text is a "
                "mapping reference, not source-revision Material evidence"
            ),
            "sourceExact": False,
            "requiresAutomatedWARPProbe": True,
            "requiresManualEyeValidation": True,
            "runtimeExecutionAdmission": False,
            "product": False,
        }))

    counts = {
        "D3D11_BLEND_DESC": sum(row["descriptorKind"] == "D3D11_BLEND_DESC" for row in rows),
        "D3D11_RASTERIZER_DESC": sum(row["descriptorKind"] == "D3D11_RASTERIZER_DESC" for row in rows),
        "D3D11_DEPTH_STENCIL_DESC": sum(row["descriptorKind"] == "D3D11_DEPTH_STENCIL_DESC" for row in rows),
    }
    require(len(rows) == EXPECTED_DESCRIPTOR_DECISIONS, "descriptor denominator changed")
    require(counts["D3D11_BLEND_DESC"] == EXPECTED_BLEND_DECISIONS, "blend denominator changed")
    require(counts["D3D11_RASTERIZER_DESC"] == EXPECTED_RASTER_DECISIONS, "raster denominator changed")
    require(counts["D3D11_DEPTH_STENCIL_DESC"] == EXPECTED_DEPTH_DECISIONS, "depth denominator changed")
    return rows


def build_receipt(
    program: dict[str, Any], *, _program_already_validated: bool = False
) -> dict[str, Any]:
    if not _program_already_validated:
        program_module.validate_program(program)
    require(program["programSha256"] == PROGRAM_SHA256, "program SHA mismatch")
    validate_program_authorities(program)

    renderer_rows = _selected_renderer_bindings(program)
    recipe_rows = build_recipe_decisions(program, renderer_rows)
    descriptor_rows = build_descriptor_decisions(program)
    neutral_rows = build_neutral_providers()
    source_evidence = {
        "program": {
            "path": "Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json",
            "rawByteCount": PROGRAM_BYTE_COUNT,
            "rawSha256": PROGRAM_RAW_SHA256,
            "programSha256": PROGRAM_SHA256,
            "programRole": program["programRole"],
            "sourceExact": False,
        },
        "programInputArtifacts": copy.deepcopy(program["inputArtifacts"]),
        "programInputArtifactsSha256": canonical_sha256(program["inputArtifacts"]),
        "implementationMappingReferences": [
            {
                "path": "Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli",
                "trackedTextSha256": EFFECT_SHADER_TRACKED_TEXT_SHA256,
                "stateNames": ["BS_EffectOpaque", "BS_EffectAlpha", "BS_EffectAdditive"],
                "claimFidelity": "CURRENT_IMPLEMENTATION_MAPPING_REFERENCE_NOT_SOURCE_EVIDENCE",
            },
            {
                "path": "Engine/Bin/ShaderFiles/Engine_Shader_Defines.hlsli",
                "trackedTextSha256": ENGINE_SHADER_DEFINES_TRACKED_TEXT_SHA256,
                "stateNames": ["RS_Cull_None", "DSS_ZNone"],
                "claimFidelity": "CURRENT_IMPLEMENTATION_MAPPING_REFERENCE_NOT_SOURCE_EVIDENCE",
            },
        ],
        "generatorAndValidator": {
            "path": "Tools/LevelPlacementExtractor/build_artist_31470_material_render_resource_binding_approval.py",
            "trackedTextSha256": strict_io.tracked_text_sha256(DEFAULT_TOOL),
            "role": "DETERMINISTIC_REVIEW_CANDIDATE_GENERATOR_AND_STRICT_VALIDATOR",
        },
        "decisionFidelity": "EXPLICITLY_RECONSTRUCTED_POLICY_APPROVAL_NOT_SOURCE_EVIDENCE",
        "sourceExact": False,
    }
    receipt: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "approvalId": APPROVAL_ID,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "approvalContract": {
            "decisionKind": "RECONSTRUCTED_MATERIAL_RENDER_RESOURCE_POLICY_APPROVAL",
            "reviewCandidateDerivation": "DETERMINISTIC_OFFLINE_ONLY",
            "runtimeNameOrRoleHeuristicsAllowed": False,
            "actionTimeIoAllowed": False,
            "transactionPolicy": "PARSE_VALIDATE_STAGE_COMMIT_OR_ROLLBACK",
            "sourceExact": False,
            "requiresAutomatedWARPProbe": True,
            "requiresManualEyeValidation": True,
            "runtimeExecutionAdmission": False,
            "product": False,
        },
        "sourceEvidence": source_evidence,
        "neutralProviders": neutral_rows,
        "recipeTextureBindings": recipe_rows,
        "rendererSlotBindings": renderer_rows,
        "renderStateDescriptors": descriptor_rows,
        "blockerProjection": {
            "blockers": BLOCKERS,
            "actionTimeIoAllowed": False,
            "bindingFailureBehavior": "ROLLBACK_PRESERVE_PREVIOUS_RESOURCE_SET",
            "partialCommitAllowed": False,
            "sourceExact": False,
            "runtimeExecutionAdmission": False,
            "product": False,
        },
        "admission": {
            "sourceExact": False,
            "requiresAutomatedWARPProbe": True,
            "requiresManualEyeValidation": True,
            "runtimeExecutionAdmission": False,
            "product": False,
        },
        "summary": {
            "neutralProviderCount": len(neutral_rows),
            "recipeTextureBindingCount": len(recipe_rows),
            "rendererSlotBindingCount": len(renderer_rows),
            "ambiguousRendererDecisionCount": sum(
                row["candidateCount"] == 2 for row in renderer_rows
            ),
            "renderStateDescriptorCount": len(descriptor_rows),
            "blendDescriptorCount": sum(
                row["descriptorKind"] == "D3D11_BLEND_DESC" for row in descriptor_rows
            ),
            "twoSidedRasterDescriptorCount": sum(
                row["descriptorKind"] == "D3D11_RASTERIZER_DESC" for row in descriptor_rows
            ),
            "disableDepthDescriptorCount": sum(
                row["descriptorKind"] == "D3D11_DEPTH_STENCIL_DESC" for row in descriptor_rows
            ),
            "recipesWithNeutralTexture0Count": sum(
                row["texture0Provider"]["providerKind"] == "NEUTRAL_CONSTANT"
                for row in recipe_rows
            ),
            "recipesWithNeutralTexture1Count": sum(
                row["texture1Provider"]["providerKind"] == "NEUTRAL_CONSTANT"
                for row in recipe_rows
            ),
            "sourceExact": False,
            "runtimeExecutionAdmission": False,
            "product": False,
        },
    }
    receipt["receiptSha256"] = canonical_sha256(receipt)
    validate_recursive_types(receipt)
    return receipt


def _validate_sealed_rows(receipt: dict[str, Any]) -> None:
    for section in (
        "neutralProviders", "recipeTextureBindings", "rendererSlotBindings",
        "renderStateDescriptors",
    ):
        for index, row in enumerate(receipt[section]):
            require(tuple(row.keys())[-1] == "rowSha256",
                    f"{section}[{index}]: rowSha256 must be last")
            unsigned = copy.deepcopy(row)
            digest = unsigned.pop("rowSha256")
            require(type(digest) is str and canonical_sha256(unsigned) == digest,
                    f"{section}[{index}]: row seal mismatch")


def validate_receipt(
    receipt: dict[str, Any],
    program: dict[str, Any] | None = None,
    *,
    program_path: Path = DEFAULT_PROGRAM,
    _program_already_validated: bool = False,
    require_approval: bool = True,
) -> None:
    validate_recursive_types(receipt)
    require(tuple(receipt.keys()) == ROOT_KEYS, "receipt root key/order changed")
    require(receipt["schema"] == SCHEMA and receipt["formatVersion"] == FORMAT_VERSION,
            "receipt schema/version changed")
    require(tuple(receipt.keys())[-1] == "receiptSha256", "receiptSha256 must be last")
    unsigned = copy.deepcopy(receipt)
    receipt_sha = unsigned.pop("receiptSha256")
    require(type(receipt_sha) is str and canonical_sha256(unsigned) == receipt_sha,
            "receipt self seal mismatch")
    _validate_sealed_rows(receipt)

    if _program_already_validated:
        require(program is not None,
                "validated supplied Program is required")
        program_module.validate_program(program)
        validate_program_authorities(program)
        approved_program = program
    else:
        approved_program = load_approved_program(program_path)
    if program is not None:
        program_module.validate_program(program)
        strict_ordered_equal(program, approved_program, "suppliedProgram")
    expected = build_receipt(approved_program, _program_already_validated=True)
    strict_ordered_equal(receipt, expected)
    if require_approval:
        approval_module.require_approved_receipt(receipt)


def serialized_receipt(receipt: dict[str, Any]) -> bytes:
    return (
        json.dumps(receipt, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--program", type=Path, default=DEFAULT_PROGRAM)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument(
        "--allow-unapproved",
        action="store_true",
        help="bootstrap only: build/check before independent projection pins are frozen",
    )
    args = parser.parse_args()

    raw, program = _read_program_bytes(args.program)
    require(hashlib.sha256(raw).hexdigest() == PROGRAM_RAW_SHA256,
            "program command-line input is not the frozen candidate")
    validate_program_authorities(program)
    receipt = build_receipt(program)
    validate_receipt(
        receipt,
        program,
        program_path=args.program,
        _program_already_validated=True,
        require_approval=not args.allow_unapproved,
    )
    expected = serialized_receipt(receipt)

    if args.check:
        require(args.output.is_file(), f"generated receipt is missing: {args.output}")
        require(args.output.read_bytes() == expected,
                "generated receipt is stale or not canonical LF UTF-8")
        print(
            "PASS Artist 31470 Material render-resource approval "
            f"recipes={len(receipt['recipeTextureBindings'])} "
            f"renderer={len(receipt['rendererSlotBindings'])} "
            f"descriptors={len(receipt['renderStateDescriptors'])} "
            f"receipt={receipt['receiptSha256']}"
        )
        return 0

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(expected)
    print(f"WROTE {args.output}")
    print(f"receiptSha256={receipt['receiptSha256']}")
    print(f"decisionProjectionSha256={approval_module.decision_projection_sha256(receipt)}")
    print(f"receiptProjectionSha256={approval_module.receipt_projection_sha256(receipt)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)
