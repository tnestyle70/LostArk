#!/usr/bin/env python3
"""Materialize the fail-closed Glasshole02 authoring runtime canary contract.

This receipt joins already-sealed source recovery evidence to the focused
runtime adapter.  It intentionally admits one authored occurrence only; it
does not turn unresolved UE3 sampler axes into exact facts and it never admits
the adapter to Product or visual fidelity.
"""

from __future__ import annotations

import argparse
import copy
import ctypes
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
EFFECT_TOOLS = REPOSITORY_ROOT / "Tools" / "EffectPipeline"
if str(EFFECT_TOOLS) not in sys.path:
    sys.path.insert(0, str(EFFECT_TOOLS))

import evaluate_ue3_material_uniform_expressions as uniform_evaluator  # noqa: E402
import replay_ue3_glasshole02_hlsl_translation as translation  # noqa: E402


SCHEMA = "lostark.effect-ue3-glasshole02-runtime-canary-contract"
FORMAT_VERSION = 1
TARGET_ID = "dimensionmaster-w-glasshole-02"
EFFECT_ASSET_ID = "effect.dimensionmaster.skill.2050120.clip3.unified"
OCCURRENCE_ID = "authored.source-particle.40e1b48e2f0f88dcfeff1549"
FAMILY_ID = "ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2"
RUNTIME_PROFILE_ID = "effect.ue3.glasshole-02.v1"
SOURCE_MATERIAL = "fx_m_mi_k_00.fx_mi.fx_k_pa_glasshole_02_01_tr"
PARENT_MATERIAL = "fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr"
SOURCE_VF = "fparticleoffsetcenterdynamicparametervertexfactory"
SOURCE_VS_TYPE = "tbasepassvertexshaderfnolightmappolicyfnodensitypolicy"
SOURCE_VS_ID = "2dd6d96a7e6c974fac82106409a5b9b8"
SOURCE_VS_SHA256 = "defae822f429760d30e0bfd31cef8c1217af8d7e43f794be6b42e85e6552995b"
REQUIRED_MASK = 0x7F
RT1_SENTINEL = -99.0
GLASSHOLE_SCALAR_LANES = [
    ["alpha_tile_x", "alpha_tile_y", "alpha_offsetx", "alpha_offsety"],
    ["aura_str", "aura_pow", "curve_power", "twist_str"],
    ["main_ucoord", "main_v_coord", "main_tex_upanner", "main_v_panner"],
    ["uvnoise_utile", "uvnoise_vtile", "uvnoise_pan", "in_hole_crackuv"],
    ["in_hole_panx", "in_hole_pany", "in_hole_pow", "in_hole_str"],
    ["in_hole_desaturation", "distortionpower", "distortionscale", "scale"],
    ["cracknormal_tile_x", "cracknormal_tile_y", "cracknormal_str", "edge_crack_desaturation"],
    ["edge_line", "edge_size", "time", "in_hole_height"],
]

DEFAULT_AUTHORED = REPOSITORY_ROOT / "Data/Effects/Authored/effect.dimensionmaster.skill.2050120.clip3.unified.effect.json"
DEFAULT_MATERIAL_MAPS = REPOSITORY_ROOT / "Data/Effects/Imported/DimensionMaster/Materials/skill.2050120.clip3.exact-material-maps.receipt.json"
DEFAULT_TEXTURE_SAMPLERS = REPOSITORY_ROOT / "Data/Effects/Imported/DimensionMaster/Materials/skill.2050120.clip3.exact-texture-sampler-closure.receipt.json"
DEFAULT_TRANSLATION = REPOSITORY_ROOT / "Data/Effects/Imported/DimensionMaster/Materials/skill.2050120.clip3.glasshole02-hlsl-translation.receipt.json"
DEFAULT_UNIFORM_VALUES = REPOSITORY_ROOT / "Data/Effects/Imported/DimensionMaster/Materials/skill.2050120.clip3.source-value-uniform-evaluation.receipt.json"
DEFAULT_RUNTIME_SHADER = REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_VtxEffectGlasshole02.hlsl"
DEFAULT_RUNTIME_INCLUDE = REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_Ue3Glasshole02.hlsli"
DEFAULT_RENDERER = REPOSITORY_ROOT / "Client/Private/Effect_DocumentRenderer.cpp"
DEFAULT_RENDERER_HEADER = REPOSITORY_ROOT / "Client/Public/Effect_DocumentRenderer.h"
DEFAULT_EFFECT_TOOL = REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
DEFAULT_EFFECT_TOOL_HEADER = REPOSITORY_ROOT / "Client/Public/Effect_Tool.h"
DEFAULT_OUTPUT = REPOSITORY_ROOT / "Data/Effects/Imported/DimensionMaster/Materials/skill.2050120.clip3.glasshole02-runtime-canary.contract.receipt.json"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(8 * 1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def canonical_json_sha256(value: Any) -> str:
    return sha256_bytes(json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8"))


def read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8-sig") as source:
        value = json.load(source)
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def seal(value: dict[str, Any]) -> None:
    value.pop("receiptSha256", None)
    value["receiptSha256"] = canonical_json_sha256(value)


def descriptor(path: Path, role: str) -> dict[str, Any]:
    require(path.is_file(), f"required input is missing: {path}")
    resolved = path.resolve()
    try:
        display = resolved.relative_to(REPOSITORY_ROOT.resolve()).as_posix()
    except ValueError:
        display = resolved.as_posix()
    return {"path": display, "byteSize": resolved.stat().st_size, "sha256": sha256_file(resolved), "role": role}


def target(document: dict[str, Any], target_id: str) -> dict[str, Any]:
    rows = [row for row in document.get("targets", []) if row.get("targetId") == target_id]
    require(len(rows) == 1, f"expected exactly one target {target_id}, got {len(rows)}")
    return rows[0]


def authored_occurrence(document: dict[str, Any]) -> dict[str, Any]:
    require(document.get("effectAssetId") == EFFECT_ASSET_ID, "authored effect identity changed")
    rows = [row for row in document.get("elements", []) if row.get("id") == OCCURRENCE_ID]
    require(len(rows) == 1, "exact Glasshole02 authored occurrence is absent or duplicated")
    row = rows[0]
    material = row.get("material", {})
    profile = material.get("sourceProfile", {})
    require(row.get("visible") is True and row.get("kind") == "particle", "canary occurrence is not a visible particle")
    require(material.get("sourceMaterialPath") == SOURCE_MATERIAL, "canary source material changed")
    require(profile.get("enabled") is True, "canary source profile is disabled")
    require(profile.get("profileId") == FAMILY_ID, "canary family changed")
    require(profile.get("runtimeShaderProfileId") == RUNTIME_PROFILE_ID, "canary runtime profile changed")
    require(profile.get("parentMaterialPath") == PARENT_MATERIAL, "canary parent material changed")
    return row


def module(module_rows: list[dict[str, Any]], class_name: str) -> dict[str, Any]:
    rows = [row for row in module_rows if str(row.get("className", "")).casefold() == class_name]
    require(len(rows) == 1, f"source module {class_name} is absent or duplicated")
    return rows[0]


def literal(module_row: dict[str, Any], property_path: str) -> Any:
    rows = [row for row in module_row.get("literals", []) if row.get("propertyPath") == property_path]
    require(len(rows) == 1, f"source literal {property_path} is absent or duplicated")
    return rows[0].get("value")


def expression_contains_time(value: Any) -> bool:
    if isinstance(value, dict):
        if str(value.get("typeName", "")).casefold() in (
            "fmaterialuniformexpressiontime",
            "fmaterialuniformexpressionrealtime",
        ):
            return True
        return any(expression_contains_time(child) for child in value.values())
    if isinstance(value, list):
        return any(expression_contains_time(child) for child in value)
    return False


def collect_parameter_defaults(
    value: Any,
    scalar_defaults: dict[str, float],
    vector_defaults: dict[str, list[float]],
) -> None:
    if isinstance(value, dict):
        type_name = str(value.get("typeName", "")).casefold()
        name = str(value.get("parameterName", "")).casefold()
        if type_name == "fmaterialuniformexpressionscalarparameter":
            default = float(value["defaultValue"])
            require(
                name not in scalar_defaults or scalar_defaults[name] == default,
                f"conflicting scalar AST default: {name}",
            )
            scalar_defaults[name] = default
        elif type_name == "fmaterialuniformexpressionvectorparameter":
            default_vector = [float(lane) for lane in value["defaultValue"]]
            require(
                name not in vector_defaults
                or vector_defaults[name] == default_vector,
                f"conflicting vector AST default: {name}",
            )
            vector_defaults[name] = default_vector
        for child in value.values():
            collect_parameter_defaults(child, scalar_defaults, vector_defaults)
    elif isinstance(value, list):
        for child in value:
            collect_parameter_defaults(child, scalar_defaults, vector_defaults)


def strip_cpp_comments(text: str) -> str:
    return re.sub(r"//[^\n]*|/\*.*?\*/", "", text, flags=re.DOTALL)


def cpp_function_body(text: str, marker: str) -> str:
    code = strip_cpp_comments(text)
    marker_offset = code.find(marker)
    require(marker_offset >= 0, f"renderer function seam is absent: {marker}")
    open_offset = code.find("{", marker_offset)
    require(open_offset >= 0, f"renderer function body is absent: {marker}")
    depth = 0
    for offset in range(open_offset, len(code)):
        if code[offset] == "{":
            depth += 1
        elif code[offset] == "}":
            depth -= 1
            if depth == 0:
                return code[open_offset + 1 : offset]
    raise ValueError(f"renderer function body is unterminated: {marker}")


def parse_cpp_float4_array22(function_body: str, name: str) -> list[list[float]]:
    array_match = re.search(
        rf"const\s+std::array<float4_t\s*,\s*22u>\s+{name}\s*=\s*"
        r"\{\{(?P<body>.*?)\}\}\s*;",
        function_body,
        re.DOTALL,
    )
    require(array_match is not None, f"C++ CB0 self-test array is absent: {name}")
    number = r"[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?f?"
    rows = [
        [float(component.rstrip("fF")) for component in match.groups()]
        for match in re.finditer(
            rf"\{{\s*({number})\s*,\s*({number})\s*,\s*"
            rf"({number})\s*,\s*({number})\s*\}}",
            array_match.group("body"),
        )
    ]
    require(len(rows) == 22, f"C++ CB0 self-test row denominator changed: {name}")
    return rows


def rows_close(left: list[list[float]], right: list[list[float]]) -> bool:
    return len(left) == len(right) and all(
        abs(left[row][lane] - right[row][lane]) <= 1.0e-6
        for row in range(len(left))
        for lane in range(4)
    )


def validate_renderer_runtime_seams(
    renderer_text: str, renderer_header_text: str
) -> None:
    renderer_code = strip_cpp_comments(renderer_text)
    renderer_contract_text = renderer_header_text + "\n" + renderer_text
    for anchor in (EFFECT_ASSET_ID, RUNTIME_PROFILE_ID, FAMILY_ID, OCCURRENCE_ID):
        require(
            anchor in renderer_contract_text,
            f"renderer canary anchor is absent: {anchor}",
        )
    require(
        "Shader_VtxEffectGlasshole02.hlsl" in renderer_code,
        "renderer shader path anchor is absent",
    )
    require(
        "0x7f" in strip_cpp_comments(renderer_contract_text).casefold(),
        "renderer 0x7f source texture mask anchor is absent",
    )
    require(
        re.search(
            r"Bind_RawValue\s*\(\s*"
            r'"g_Glasshole02LocalTimeSeconds"\s*,\s*'
            r"&fLocalTimeSeconds\s*,\s*"
            r"sizeof\s*\(\s*fLocalTimeSeconds\s*\)\s*\)",
            renderer_code,
        )
        is not None,
        "renderer occurrence-local-time Bind_RawValue call is absent",
    )
    require(
        re.search(
            r"Build_Glasshole02TranslatedCanaryCB0\s*\("
            r"[^;]*fLocalTimeSeconds\s*,\s*MaterialCB0\s*\)",
            renderer_code,
        )
        is not None
        and re.search(
            r"Bind_RawValue\s*\(\s*"
            r'"g_Ue3Glasshole02CB0"\s*,\s*MaterialCB0\.data\(\)\s*,\s*'
            r"sizeof\s*\(\s*MaterialCB0\s*\)\s*\)",
            renderer_code,
        )
        is not None,
        "renderer same-time exact CB0 upload seam is absent",
    )
    for name, expected in (
        ("GLASSHOLE02_TRANSLATED_CANARY_DEFAULT_ENABLED", "false"),
        ("GLASSHOLE02_TRANSLATED_CANARY_PRODUCT_ENABLED", "false"),
        ("GLASSHOLE02_TRANSLATED_CANARY_FAIL_CLOSED", "true"),
    ):
        require(
            re.search(rf"{name}\s*=\s*{expected}\s*;", renderer_header_text)
            is not None,
            f"renderer activation policy anchor changed: {name}",
        )
    for assertion in (
        r"static_assert\s*\(\s*!\s*GLASSHOLE02_TRANSLATED_CANARY_DEFAULT_ENABLED\s*\)",
        r"static_assert\s*\(\s*!\s*GLASSHOLE02_TRANSLATED_CANARY_PRODUCT_ENABLED\s*\)",
        r"static_assert\s*\(\s*GLASSHOLE02_TRANSLATED_CANARY_FAIL_CLOSED\s*\)",
    ):
        require(
            re.search(assertion, renderer_code) is not None,
            "renderer compile-time closed activation assertion is absent",
        )


def validate_renderer_source_join(
    renderer_text: str,
    material_slots: list[dict[str, Any]],
    expected_runtime_cb0_by_time: dict[float, list[list[float]]],
) -> None:
    code = strip_cpp_comments(renderer_text)
    stage = cpp_function_body(renderer_text, "Stage_Glasshole02TranslatedCanaryPacket(")
    render = cpp_function_body(renderer_text, "Render_Glasshole02TranslatedCanaryParticles(")
    render_particles = cpp_function_body(renderer_text, "Render_Particles(")
    validator = cpp_function_body(
        renderer_text, "Validate_Glasshole02TranslatedCanaryCB0Evaluator()"
    )
    constants = cpp_function_body(renderer_text, "Build_Glasshole02Constants(")
    source_profile = cpp_function_body(renderer_text, "uint32_t SourceMaterialProfileIndex(")
    effective_profile = cpp_function_body(renderer_text, "uint32_t EffectiveSourceMaterialProfileIndex(")
    stage_element = cpp_function_body(
        renderer_text, "HRESULT Client::CEffectDocumentRenderer::Stage_ElementResource("
    )

    for anchor in (
        "GLASSHOLE02_TRANSLATED_CANARY_EFFECT_ASSET_ID",
        "GLASSHOLE02_TRANSLATED_CANARY_OCCURRENCE_ID",
        "GLASSHOLE02_TRANSLATED_SOURCE_MATERIAL",
        "GLASSHOLE02_TRANSLATED_CANARY_FAMILY_ID",
        "GLASSHOLE02_TRANSLATED_CANARY_PROFILE_ID",
        "GLASSHOLE02_TRANSLATED_PARENT_MATERIAL",
        '"FX_PC_SWP_05:export:2282@ref:0"',
        '"particlemodulerequired"',
        '"FX_PC_SWP_05:export:1895@ref:4"',
        '"particlemoduleparameterdynamic"',
        'HasBooleanLiteral(pRequired, "boffsetcenter", true)',
        'HasStringLiteral(pRequired, "screenalignment", "psa_rectangle")',
        'HasNumberLiteral(pDynamic, "updateflags", 15.0)',
    ):
        require(anchor in stage, f"renderer exact occurrence/carrier gate changed: {anchor}")
    for exact_identity in (SOURCE_MATERIAL, PARENT_MATERIAL):
        require(
            exact_identity in code,
            f"renderer exact material identity changed: {exact_identity}",
        )
    for parameter_index, expected_names in enumerate(GLASSHOLE_SCALAR_LANES):
        assignment = re.search(
            rf"Parameters\s*\[\s*{parameter_index}\s*\]\s*=\s*"
            r"\{(?P<body>.*?)\}\s*;",
            constants,
            re.DOTALL,
        )
        require(
            assignment is not None
            and re.findall(r'S\(\s*"([^"]+)"', assignment.group("body"))
            == expected_names,
            f"renderer named scalar-to-P mapping changed: P[{parameter_index}]",
        )
    require(
        re.search(
            r"vAuraColor\s*=\s*SourceVector\s*\(\s*Source\s*,\s*"
            r'"aura_color"',
            constants,
        )
        is not None
        and re.search(
            r"vInHoleColor\s*=\s*SourceVector\s*\(\s*Source\s*,\s*"
            r'"in_hole_color"',
            constants,
        )
        is not None,
        "renderer named vector-to-Aura/InHole mapping changed",
    )
    require(
        re.search(
            r"Build_Glasshole02Constants\s*\(\s*SourceMaterial\s*,\s*"
            r"Staged\.TypedTrailParameters\s*,\s*"
            r"Staged\.vSourceVector0\s*,\s*Staged\.vSourceVector1\s*\)",
            code,
        )
        is not None,
        "renderer Glasshole02 P/Aura/InHole staging join changed",
    )
    source_profile_anchors = (
        RUNTIME_PROFILE_ID,
        FAMILY_ID,
        PARENT_MATERIAL,
        "Has_EffectGlasshole02NamedTextureContract(Source)",
        "29u : UINT32_MAX",
    )
    require(
        all(anchor in source_profile for anchor in source_profile_anchors),
        "Glasshole02 SourceMaterial profile-29 resolver changed",
    )
    require(
        "const uint32_t iStoredProfile = SourceMaterialProfileIndex(Source);"
        in effective_profile
        and "iStoredProfile >= 29u && iStoredProfile <= 33u" in effective_profile
        and "Is_FamilyProfileCarrierContractSatisfied(" in effective_profile
        and "? iStoredProfile : UINT32_MAX" in effective_profile,
        "Glasshole02 effective profile carrier gate changed",
    )
    require(
        "Staged.iSourceMaterialProfile = EffectiveSourceMaterialProfileIndex(Element);"
        in stage_element
        and "else if (29u == Staged.iSourceMaterialProfile)" in stage_element
        and "Build_Glasshole02Constants(" in stage_element,
        "Glasshole02 profile-29 staging preservation changed",
    )

    for row in material_slots:
        srgb = "true" if row["sourceColorSpace"] == "srgb" else "false"
        pin_pattern = (
            re.escape(row["runtimeDdsAssetId"])
            + r'"\s*,\s*'
            + str(row["runtimeDdsByteSize"])
            + r"u\s*,\s*\""
            + row["runtimeDdsSha256"]
            + r'"\s*,\s*'
            + srgb
        )
        require(
            re.search(pin_pattern, code) is not None,
            f"renderer DDS pin changed: {row['runtimeDdsAssetId']}",
        )
    texture_variables = [
        "g_Ue3Glasshole02CrackNormal",
        "g_Ue3Glasshole02AtypicalMask",
        "g_Ue3Glasshole02SceneDepth",
        "g_Ue3Glasshole02InnerHole",
        "g_Ue3Glasshole02Dust",
        "g_Ue3Glasshole02Environment",
        "g_Ue3Glasshole02Aura",
        "g_Ue3Glasshole02EnvironmentOverlay",
    ]
    variable_block = re.search(
        r"GLASSHOLE02_TRANSLATED_TEXTURE_VARIABLES\s*=\s*\{\{(?P<body>.*?)\}\}\s*;",
        code,
        re.DOTALL,
    )
    require(variable_block is not None, "renderer texture-variable permutation is absent")
    require(
        re.findall(r'"([A-Za-z0-9_]+)"', variable_block.group("body"))
        == texture_variables,
        "renderer material-lane/scene-depth shader permutation changed",
    )
    for anchor in (
        "const size_t iVariable = iLane < 2u ? iLane : iLane + 1u;",
        'TEXT("Target_Depth")',
        "GLASSHOLE02_TRANSLATED_TEXTURE_VARIABLES[2u]",
        "CExactPreviewPipelineStateGuard StateGuard(m_pContext.Get());",
        "SceneCB2[0] = { 0.5f, -0.5f, 0.5f, 0.5f };",
        "1.f / pProjection->_43",
        "pProjection->_33 / pProjection->_43",
        "PSSetSamplers(",
        "0u, static_cast<uint32_t>(Samplers.size()), Samplers.data()",
    ):
        require(anchor in render, f"renderer runtime binding seam changed: {anchor}")
    for anchor in (
        "D3D11_FILTER_MIN_MAG_MIP_POINT",
        "D3D11_FILTER_MIN_MAG_MIP_LINEAR",
        "D3D11_TEXTURE_ADDRESS_CLAMP",
        "D3D11_TEXTURE_ADDRESS_WRAP",
        "Packet->Samplers.size()",
    ):
        require(anchor in stage, f"renderer sampler candidate seam changed: {anchor}")

    canary_branch = render_particles.find("if (bGlasshole02TranslatedCanaryCandidate)")
    fail_return = render_particles.find("return Fail_RenderOperation(", canary_branch)
    ordinary = render_particles.find("Render_AuthoringExactPreviewParticles(")
    require(
        0 <= canary_branch < fail_return < ordinary,
        "staged translated failure can enter the ordinary shader path",
    )

    expected_names = {
        0.0: "TimeZero",
        0.25: "TimeQuarter",
        0.6: "TimePointSix",
    }
    for seconds, name in expected_names.items():
        parsed = parse_cpp_float4_array22(validator, name)
        require(
            rows_close(parsed, expected_runtime_cb0_by_time[seconds]),
            f"C++ full CB0 self-test differs from exact AST at t={seconds}",
        )
        require(
            f"AllRowsEqual(Rows, {name})" in validator,
            f"C++ full-row CB0 comparator is absent at t={seconds}",
        )
        for slot in (3, 10, 11, 13):
            require(
                parsed[slot][2:] == [0.0, 0.0],
                f"C++ CB0 zero-tail changed at t={seconds}, c{slot}",
            )
    for time_literal in ("0.f", "0.25f", "0.6f"):
        require(
            re.search(
                r"Build_Glasshole02TranslatedCanaryCB0\s*\("
                rf"[^;]*{re.escape(time_literal)}\s*,\s*Rows\s*\)",
                validator,
            )
            is not None,
            f"C++ CB0 evaluator self-test time is absent: {time_literal}",
        )

    reset_block = (
        "m_bAuthoringGlasshole02TranslatedCanaryEnabled = false;",
        "m_pGlasshole02TranslatedCanaryShader.reset();",
        "m_pGlasshole02TranslatedCanaryBlendState.Reset();",
    )
    clear = cpp_function_body(renderer_text, "void Client::CEffectDocumentRenderer::Clear()")
    require(all(anchor in clear for anchor in reset_block), "Clear does not reset the canary")
    require(
        all(code.count(anchor) >= 6 for anchor in reset_block),
        "Product/prepared attachment paths do not consistently reset the canary",
    )


def validate_tool_frontend_seams(tool_text: str, tool_header_text: str) -> None:
    code = strip_cpp_comments(tool_text)
    header_code = strip_cpp_comments(tool_header_text)
    session = cpp_function_body(tool_text, "Render_AuthoringSessionBar()")
    toggle = cpp_function_body(
        tool_text,
        "bool_t Client::CEffect_Tool::Try_SetGlasshole02TranslatedCanaryEnabled(",
    )
    occurrence = cpp_function_body(
        tool_text,
        "bool_t Client::CEffect_Tool::Has_Glasshole02TranslatedCanaryOccurrence(",
    )
    stage = cpp_function_body(
        tool_text, "const bool_t bAllowReadOnlySourceProjection)"
    )
    reset_exact = cpp_function_body(
        tool_text,
        "void Client::CEffect_Tool::Reset_ExactCookedCanarySelection(",
    )

    require(
        re.search(
            r"bool_t\s+m_bGlasshole02TranslatedCanaryEnabled\s*=\s*false\s*;",
            header_code,
        )
        is not None
        and "Try_SetGlasshole02TranslatedCanaryEnabled(bool_t bEnabled)"
        in header_code,
        "Tool translated-canary flag/declaration changed",
    )
    for anchor in (
        'ImGui::Checkbox("Enable Translated Glasshole02 Canary"',
        "m_bGlasshole02TranslatedCanaryEnabled",
        "Has_ProductCuePreview() ||",
        "m_bExactCookedCanaryEnabled",
        "Try_SetGlasshole02TranslatedCanaryEnabled(",
        "Product/read-only OFF",
    ):
        require(anchor in session, f"Tool checkbox scope changed: {anchor}")

    for anchor in (
        "GLASSHOLE02_TRANSLATED_CANARY_EFFECT_ASSET_ID",
        "1u == static_cast<size_t>(std::count_if(",
        "GLASSHOLE02_TRANSLATED_CANARY_OCCURRENCE_ID",
        "GLASSHOLE02_TRANSLATED_CANARY_FAMILY_ID",
        "GLASSHOLE02_TRANSLATED_CANARY_PROFILE_ID",
    ):
        require(anchor in occurrence, f"Tool exact occurrence gate changed: {anchor}")

    for anchor in (
        "EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource",
        "Has_ProductCuePreview()",
        "m_bExactCookedCanaryEnabled",
        "!Has_Glasshole02TranslatedCanaryOccurrence(*m_ActiveDocument)",
        "m_bGlasshole02TranslatedCanaryEnabled = true;",
        "Release_WorldPreview(true);",
        "Stage_WorldPreview(*m_ActiveDocument)",
        "m_bGlasshole02TranslatedCanaryEnabled = false;",
        "const bool_t bOrdinaryPreviewRestored =",
    ):
        require(anchor in toggle, f"Tool explicit enable/rollback seam changed: {anchor}")
    enable_offset = toggle.find("m_bGlasshole02TranslatedCanaryEnabled = true;")
    release_offset = toggle.find("Release_WorldPreview(true);", enable_offset)
    stage_offset = toggle.find("Stage_WorldPreview(*m_ActiveDocument)", release_offset)
    failure_off_offset = toggle.find(
        "m_bGlasshole02TranslatedCanaryEnabled = false;", stage_offset
    )
    rollback_release_offset = toggle.find(
        "Release_WorldPreview(true);", failure_off_offset
    )
    ordinary_stage_offset = toggle.find(
        "Stage_WorldPreview(*m_ActiveDocument)", rollback_release_offset
    )
    require(
        0
        <= enable_offset
        < release_offset
        < stage_offset
        < failure_off_offset
        < rollback_release_offset
        < ordinary_stage_offset,
        "Tool initial-stage rollback ordering changed",
    )

    for anchor in (
        "m_bGlasshole02TranslatedCanaryEnabled &&",
        "EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource",
        "!bAllowReadOnlySourceProjection",
        "!Has_ProductCuePreview()",
        "Has_Glasshole02TranslatedCanaryOccurrence(PreviewDocument)",
        "!bGlasshole02TranslatedCanaryStage",
        "else if (!bGlasshole02TranslatedCanaryStage &&",
        "Set_AuthoringGlasshole02TranslatedCanaryEnabled(",
        "true, Error",
        "false, DisableError",
        "Release_WorldPreview(true);",
        "pObject->Stage_Document(PreviewDocument, Error)",
    ):
        require(anchor in stage, f"Tool pre-stage/Product-off seam changed: {anchor}")
    require(
        re.search(
            r"const\s+bool_t\s+bGlasshole02TranslatedCanaryStage\s*=\s*"
            r"m_bGlasshole02TranslatedCanaryEnabled\s*&&.*?"
            r"EFFECT_DOCUMENT_SOURCE::AUTHORED\s*==\s*m_eActiveDocumentSource\s*&&.*?"
            r"!bAllowReadOnlySourceProjection\s*&&\s*!Has_ProductCuePreview\(\)\s*&&.*?"
            r"Has_Glasshole02TranslatedCanaryOccurrence\(PreviewDocument\)\s*;",
            stage,
            re.DOTALL,
        )
        is not None,
        "Tool translated pre-stage Authored/Product/read-only gate changed",
    )
    require(
        stage.find("Set_AuthoringGlasshole02TranslatedCanaryEnabled(")
        < stage.find("pObject->Stage_Document(PreviewDocument, Error)"),
        "Tool translated canary is not enabled before document stage",
    )
    translated_disable_offset = stage.find(
        "!bGlasshole02TranslatedCanaryStage"
    )
    disable_call_offset = stage.find(
        "Set_AuthoringGlasshole02TranslatedCanaryEnabled(",
        translated_disable_offset,
    )
    disable_false_offset = stage.find("false, DisableError", disable_call_offset)
    release_after_disable_offset = stage.find(
        "Release_WorldPreview(true);", disable_false_offset
    )
    require(
        0 <= translated_disable_offset < disable_call_offset
        < disable_false_offset < release_after_disable_offset,
        "Tool Product/read-only stage no longer disables translated execution",
    )

    require(
        "Reset_Glasshole02TranslatedCanarySelection(strReason);"
        in reset_exact,
        "Tool document-change reset no longer cascades to translated canary",
    )
    for reason in (
        '"A new document was created."',
        '"The active document changed."',
        '"The active document was discarded."',
    ):
        require(
            f"Reset_ExactCookedCanarySelection({reason})" in code,
            f"Tool document-change reset reason changed: {reason}",
        )


def compile_shader(path: Path, compiler: Path, entry: str, profile: str) -> bytes:
    translation.external_binary_identity(compiler)
    dll = ctypes.WinDLL(str(compiler))
    function = dll.D3DCompileFromFile
    function.restype = translation.HRESULT
    function.argtypes = [ctypes.c_wchar_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, translation.UINT, translation.UINT, ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_void_p)]
    code = ctypes.c_void_p()
    errors = ctypes.c_void_p()
    try:
        result = function(str(path.resolve()), None, ctypes.c_void_p(1), entry.encode("ascii"), profile.encode("ascii"), 0, 0, ctypes.byref(code), ctypes.byref(errors))
        error_text = translation.blob_bytes(errors).decode("utf-8", "replace") if errors.value else ""
        require(result >= 0 and code.value, f"runtime shader {entry}/{profile} compile failed 0x{result & 0xFFFFFFFF:08X}: {error_text}")
        return translation.blob_bytes(code)
    finally:
        translation.release(errors)
        translation.release(code)


def public_signature(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    return [{key: row[key] for key in ("semanticName", "semanticIndex", "systemValueType", "componentType", "register", "mask", "readWriteMask", "stream")} for row in rows]


def build_receipt(
    authored_path: Path = DEFAULT_AUTHORED,
    material_maps_path: Path = DEFAULT_MATERIAL_MAPS,
    texture_samplers_path: Path = DEFAULT_TEXTURE_SAMPLERS,
    translation_path: Path = DEFAULT_TRANSLATION,
    uniform_values_path: Path = DEFAULT_UNIFORM_VALUES,
    runtime_shader_path: Path = DEFAULT_RUNTIME_SHADER,
    runtime_include_path: Path = DEFAULT_RUNTIME_INCLUDE,
    renderer_path: Path = DEFAULT_RENDERER,
    renderer_header_path: Path = DEFAULT_RENDERER_HEADER,
    effect_tool_path: Path = DEFAULT_EFFECT_TOOL,
    effect_tool_header_path: Path = DEFAULT_EFFECT_TOOL_HEADER,
    compiler: Path = translation.DEFAULT_D3DCOMPILER,
) -> dict[str, Any]:
    authored = read_json(authored_path)
    occurrence = authored_occurrence(authored)
    maps = read_json(material_maps_path)
    maps_target = target(maps, TARGET_ID)
    textures = read_json(texture_samplers_path)
    texture_target = target(textures, TARGET_ID)
    translated = read_json(translation_path)
    uniform_values = read_json(uniform_values_path)

    require(maps_target.get("familyId") == FAMILY_ID, "exact-map family changed")
    emitter = maps_target.get("sourceEmitterVertexFactoryPass", {})
    require(emitter.get("occurrenceId") == OCCURRENCE_ID, "exact-map emitter occurrence changed")
    selection = emitter.get("sourceEmitterVertexFactorySelection", {})
    require(selection.get("vertexFactoryType") == SOURCE_VF, "exact source emitter VF changed")
    vertex = emitter.get("exactVertexShader", {})
    require(vertex.get("shaderType") == SOURCE_VS_TYPE and vertex.get("shaderIdHex") == SOURCE_VS_ID, "exact NoDensity VS identity changed")
    require(vertex.get("dxbc", {}).get("sha256") == SOURCE_VS_SHA256, "exact NoDensity VS blob changed")
    require(emitter.get("vertexPixelSignatureClosure", {}).get("pass") is True, "exact source VS/PS signature closure is open")

    recipe = occurrence.get("sourceRecipe", {})
    require(recipe.get("enabled") is True and recipe.get("rendererShape") == "sprite", "canary carrier is not source sprite")
    required = module(recipe.get("modules", []), "particlemodulerequired")
    dynamic = module(recipe.get("modules", []), "particlemoduleparameterdynamic")
    require(required.get("stableId") == "FX_PC_SWP_05:export:2282@ref:0", "required-module identity changed")
    require(literal(required, "boffsetcenter") is True, "source offset-center carrier condition changed")
    require(literal(required, "screenalignment") == "psa_rectangle", "source rectangle alignment changed")
    require(dynamic.get("stableId") == "FX_PC_SWP_05:export:1895@ref:4", "dynamic-module identity changed")
    require(literal(dynamic, "updateflags") == 15, "source dynamic-parameter update flags changed")

    native = maps_target.get("nativeShaderObjectBinding", {})
    closure = native.get("textureSampleClosure", {})
    require(closure.get("materialSamplePairs") == ["t0/s1", "t1/s2", "t3/s3", "t4/s4", "t5/s5", "t6/s6", "t7/s7"], "material texture/sample wire changed")
    require(closure.get("unownedEngineSamplePairs") == ["t2/s0"], "scene-depth texture/sample wire changed")
    require(texture_target.get("sourceExactSamplerAdmission") is False, "full source-exact sampler was falsely admitted")
    bindings = texture_target.get("uniformTextureBindings", [])
    require(len(bindings) == 7, "Glasshole02 material DDS denominator changed")
    material_slots = []
    for runtime_slot, binding in enumerate(bindings):
        sampling = binding["sourceTexture2D"]["samplerAndColorSpace"]
        dds = binding["ddsIdentity"]["runtimeDimensionMaster"]
        require(dds.get("sourceExactParity") is True, f"runtime DDS parity is open at slot {runtime_slot}")
        require(sampling.get("sourceExactColorSpace") is True, f"source color space is open at slot {runtime_slot}")
        require(sampling.get("sourceExactFilterSelector") is True, f"source filter selector is open at slot {runtime_slot}")
        require(sampling.get("sourceExactHardwareFilter") is False, f"hardware filter unexpectedly exact at slot {runtime_slot}")
        material_slots.append({
            "runtimeSourceTextureSlot": runtime_slot,
            "textureRegister": binding["textureRegister"],
            "samplerRegister": binding["samplerRegister"],
            "sourceObjectPath": binding["effectiveSourceObjectPath"],
            "runtimeDdsAssetId": dds["relativePath"],
            "runtimeDdsByteSize": dds["byteSize"],
            "runtimeDdsSha256": dds["sha256"],
            "runtimeDdsSourceExactParity": True,
            "sourceColorSpace": sampling["colorSpace"]["value"],
            "sourceExactColorSpace": True,
            "sourceFilterSelector": sampling["filterSelector"]["value"],
            "sourceExactFilterSelector": True,
            "sourceAddressU": sampling["addressU"]["value"],
            "sourceExactAddressU": sampling["addressU"]["sourceExact"],
            "sourceAddressV": sampling["addressV"]["value"],
            "sourceExactAddressV": sampling["addressV"]["sourceExact"],
            "sourceHardwareFilter": sampling["hardwareFilter"]["value"],
            "sourceExactHardwareFilter": False,
            "fullSourceExactSampler": False,
            "blockers": sampling["blockers"],
        })
    require([row["runtimeSourceTextureSlot"] for row in material_slots] == list(range(7)), "runtime material slot order is not dense")
    preview_sampler_candidates = [
        {
            "samplerRegister": "s0",
            "owner": "ENGINE_SCENE_DEPTH",
            "filter": "MIN_MAG_MIP_POINT",
            "addressU": "CLAMP",
            "addressV": "CLAMP",
            "addressW": "CLAMP",
            "fidelity": "EXPLICIT_AUTHORING_CANARY_CANDIDATE_NOT_SOURCE_MATERIAL_SAMPLER",
            "sourceExact": False,
        }
    ]
    for sampler_index in range(1, 8):
        preview_sampler_candidates.append(
            {
                "samplerRegister": f"s{sampler_index}",
                "owner": "MATERIAL_PREVIEW_CANDIDATE",
                "filter": "MIN_MAG_MIP_LINEAR",
                "addressU": "WRAP",
                "addressV": "CLAMP" if sampler_index == 2 else "WRAP",
                "addressW": "WRAP",
                "fidelity": "EXPLICIT_AUTHORING_CANARY_CANDIDATE_NOT_SOURCE_EXACT_HARDWARE_SAMPLER",
                "sourceExact": False,
            }
        )

    uniform_target = target(uniform_values, TARGET_ID)
    require(uniform_target.get("familyId") == FAMILY_ID, "source-value uniform family changed")
    time_evaluations = []
    evaluated_rows_by_time: list[dict[int, list[float]]] = []
    runtime_cb0_by_time: dict[float, list[list[float]]] = {}
    for seconds in (0.0, 0.25, 0.6):
        context = {
            "gameTimeSeconds": seconds,
            "realTimeSeconds": seconds,
            "floatSemantics": "IEEE754_FLOAT32_ROUND_AFTER_EACH_UNIFORM_EXPRESSION_OPERATION",
        }
        evaluated = uniform_evaluator.evaluate_uniform_set_into_cb0(
            maps_target["materialMap"]["uniformExpressionSet"],
            maps_target["nativeShaderObjectBinding"],
            uniform_target["effectiveScalarOverrides"],
            uniform_target["effectiveVectorOverrides"],
            context,
        )
        if seconds == 0.0:
            uniform_evaluator.validate_target_native_scalar_group_packing(
                TARGET_ID, evaluated
            )
        material_rows = evaluated["nativeCb0"]["materialRows"]
        material_rows_by_slot = {
            int(row["slot"]): row["value"] for row in material_rows
        }
        require(
            sorted(material_rows_by_slot) == list(range(1, 22)),
            "Glasshole02 exact AST material rows are not c1..c21",
        )
        evaluated_rows_by_time.append(material_rows_by_slot)
        runtime_rows = [[1.0, 0.0, 0.0, 0.0]] + [
            material_rows_by_slot[slot] for slot in range(1, 22)
        ]
        runtime_cb0_by_time[seconds] = runtime_rows
        time_evaluations.append(
            {
                "gameTimeSeconds": seconds,
                "realTimeSeconds": seconds,
                "nativeCb0MaterialRowsSemanticSha256": evaluated["nativeCb0"][
                    "materialRowsSemanticSha256"
                ],
                "pixelVectorValuesSemanticSha256": evaluated[
                    "pixelVectorValuesSemanticSha256"
                ],
                "pixelScalarValuesSemanticSha256": evaluated[
                    "pixelScalarValuesSemanticSha256"
                ],
                "runtimeCb0WithRendererC0SemanticSha256": canonical_json_sha256(
                    runtime_rows
                ),
                "runtimeCb0Rows": runtime_rows,
            }
        )
    require(
        len(
            {
                row["nativeCb0MaterialRowsSemanticSha256"]
                for row in time_evaluations
            }
        )
        == 3,
        "Glasshole02 CB0 packet is static across canary times",
    )
    value_changing_slots = sorted(
        slot
        for slot in evaluated_rows_by_time[0]
        if len(
            {
                canonical_json_sha256(rows[slot])
                for rows in evaluated_rows_by_time
            }
        )
        > 1
    )
    require(
        value_changing_slots == [2, 3, 10, 11, 13, 17, 19, 20, 21],
        "Glasshole02 value-changing CB0 slot set changed",
    )
    uniform_set = maps_target["materialMap"]["uniformExpressionSet"]
    scalar_defaults: dict[str, float] = {}
    vector_defaults: dict[str, list[float]] = {}
    collect_parameter_defaults(uniform_set, scalar_defaults, vector_defaults)
    scalar_overrides = {
        row["parameterKey"]["nameCasefold"]: float(row["value"])
        for row in uniform_target["effectiveScalarOverrides"]
        if row["parameterKey"]["number"] == 0
    }
    vector_overrides = {
        row["parameterKey"]["nameCasefold"]: [
            float(lane) for lane in row["value"]
        ]
        for row in uniform_target["effectiveVectorOverrides"]
        if row["parameterKey"]["number"] == 0
    }
    authored_profile = occurrence["material"]["sourceProfile"]
    authored_scalars = {
        row["name"].casefold(): float(row["value"])
        for row in authored_profile["scalars"]
    }
    runtime_parameter_rows = []
    for parameter_index, lane_names in enumerate(GLASSHOLE_SCALAR_LANES):
        values = []
        lanes = []
        for lane_index, name in enumerate(lane_names):
            require(
                name in scalar_defaults or name in authored_scalars,
                f"Glasshole scalar evidence absent: {name}",
            )
            value = scalar_overrides.get(
                name, scalar_defaults.get(name, authored_scalars[name])
            )
            values.append(value)
            lanes.append(
                {
                    "lane": "xyzw"[lane_index],
                    "parameterName": name,
                    "value": value,
                    "valueSource": (
                        "EFFECTIVE_MIC_OVERRIDE"
                        if name in scalar_overrides
                        else (
                            "EXACT_AST_PARAMETER_DEFAULT"
                            if name in scalar_defaults
                            else "AUTHORED_SOURCE_PROFILE_DEAD_AST_INPUT"
                        )
                    ),
                }
            )
        runtime_parameter_rows.append(
            {"parameterRow": parameter_index, "lanes": lanes, "value": values}
        )
    runtime_vectors = []
    for runtime_field, name in (
        ("vSourceVector0", "aura_color"),
        ("vSourceVector1", "in_hole_color"),
    ):
        require(name in vector_defaults, f"Glasshole vector AST default absent: {name}")
        value = vector_overrides.get(name, vector_defaults[name])
        runtime_vectors.append(
            {
                "runtimeField": runtime_field,
                "parameterName": name,
                "value": value,
                "valueSource": (
                    "EFFECTIVE_MIC_OVERRIDE"
                    if name in vector_overrides
                    else "EXACT_AST_PARAMETER_DEFAULT"
                ),
            }
        )
    require(
        sorted(authored_scalars) == sorted(sum(GLASSHOLE_SCALAR_LANES, [])),
        "authored Glasshole scalar denominator/mapping names changed",
    )
    for row in runtime_parameter_rows:
        for lane in row["lanes"]:
            require(
                abs(authored_scalars[lane["parameterName"]] - lane["value"])
                <= 1.0e-6,
                f"authored/effective scalar value mismatch: {lane['parameterName']}",
            )
    authored_vectors = {
        row["name"].casefold(): [float(lane) for lane in row["value"]]
        for row in authored_profile["vectors"]
    }
    for row in runtime_vectors:
        require(
            rows_close([authored_vectors[row["parameterName"]]], [row["value"]]),
            f"authored/effective vector mismatch: {row['parameterName']}",
        )
    time_vector_indices = {
        index
        for index, expression in enumerate(uniform_set["pixelVectorExpressions"])
        if expression_contains_time(expression)
    }
    time_scalar_indices = {
        index
        for index, expression in enumerate(uniform_set["pixelScalarExpressions"])
        if expression_contains_time(expression)
    }
    ast_time_dependent_slots = {
        int(wire["baseIndex"]) // 16
        for wire in native["vectors"]
        if int(wire["expressionIndexOrGroup"]) in time_vector_indices
    }
    for group in evaluated["nativeCb0"]["nativeScalarGroupPackingEvidence"][
        "groupsInAscendingCb0SlotOrder"
    ]:
        if time_scalar_indices.intersection(group["projectedExpressionIndices"]):
            ast_time_dependent_slots.add(int(group["slot"]))
    ast_time_dependent_slots = sorted(ast_time_dependent_slots)
    require(
        ast_time_dependent_slots
        == [2, 3, 7, 8, 10, 11, 13, 15, 17, 19, 20, 21],
        "Glasshole02 AST time-dependent CB0 slot set changed",
    )
    cpp_self_test_scalars = copy.deepcopy(
        uniform_target["effectiveScalarOverrides"]
    )
    time_override_rows = [
        row
        for row in cpp_self_test_scalars
        if row.get("parameterKey") == {"nameCasefold": "time", "number": 0}
    ]
    require(len(time_override_rows) == 1, "Glasshole02 self-test time scalar changed")
    time_override_rows[0]["value"] = 0.4
    cpp_self_test_cb0_by_time: dict[float, list[list[float]]] = {}
    cpp_self_test_hashes = []
    for seconds in (0.0, 0.25, 0.6):
        evaluated_self_test = uniform_evaluator.evaluate_uniform_set_into_cb0(
            uniform_set,
            maps_target["nativeShaderObjectBinding"],
            cpp_self_test_scalars,
            uniform_target["effectiveVectorOverrides"],
            {
                "gameTimeSeconds": seconds,
                "realTimeSeconds": seconds,
                "floatSemantics": "IEEE754_FLOAT32_ROUND_AFTER_EACH_UNIFORM_EXPRESSION_OPERATION",
            },
        )
        self_test_rows = {
            int(row["slot"]): row["value"]
            for row in evaluated_self_test["nativeCb0"]["materialRows"]
        }
        runtime_self_test_rows = [[1.0, 0.0, 0.0, 0.0]] + [
            self_test_rows[slot] for slot in range(1, 22)
        ]
        cpp_self_test_cb0_by_time[seconds] = runtime_self_test_rows
        cpp_self_test_hashes.append(
            {
                "gameTimeSeconds": seconds,
                "exactAstC1ThroughC21SemanticSha256": evaluated_self_test[
                    "nativeCb0"
                ]["materialRowsSemanticSha256"],
                "runtimeC0PlusExactAstC1ThroughC21SemanticSha256": canonical_json_sha256(
                    runtime_self_test_rows
                ),
                "runtimeCb0Rows": runtime_self_test_rows,
            }
        )

    shader_text = runtime_shader_path.read_text(encoding="utf-8-sig")
    renderer_text = renderer_path.read_text(encoding="utf-8-sig")
    renderer_header_text = renderer_header_path.read_text(encoding="utf-8-sig")
    effect_tool_text = effect_tool_path.read_text(encoding="utf-8-sig")
    effect_tool_header_text = effect_tool_header_path.read_text(encoding="utf-8-sig")
    for anchor in ("Shader_Ue3Glasshole02.hlsli", "VS_MAIN", "PS_MAIN", "technique11 DefaultTechnique", "pass Glasshole02AlphaTwoSidedDepthRead"):
        require(anchor in shader_text, f"runtime shader anchor is absent: {anchor}")
    require(
        "g_Glasshole02LocalTimeSeconds" in shader_text,
        "runtime shader local-time seam is absent",
    )
    require(
        re.search(
            r"clip\s*\(\s*g_Glasshole02LocalTimeSeconds\s*\)",
            shader_text,
        )
        is not None,
        "runtime shader does not consume the nonnegative local-time seam",
    )
    include_text = runtime_include_path.read_text(encoding="utf-8-sig")
    require(
        re.search(r"g_Ue3Glasshole02CB0\s*\[\s*22\s*\]", include_text)
        is not None,
        "runtime shader exact CB0[22] seam changed",
    )
    validate_renderer_runtime_seams(renderer_text, renderer_header_text)
    validate_renderer_source_join(
        renderer_text, material_slots, cpp_self_test_cb0_by_time
    )
    validate_tool_frontend_seams(effect_tool_text, effect_tool_header_text)

    vs_dxbc = compile_shader(runtime_shader_path, compiler, "VS_MAIN", "vs_5_0")
    ps_dxbc = compile_shader(runtime_shader_path, compiler, "PS_MAIN", "ps_5_0")
    vs_chunks = translation.dxbc_chunks(vs_dxbc)
    ps_chunks = translation.dxbc_chunks(ps_dxbc)
    vs_input = translation.parse_signature(vs_chunks["ISGN"])
    vs_output = translation.parse_signature(vs_chunks["OSGN"])
    ps_input = translation.parse_signature(ps_chunks["ISGN"])
    ps_output = translation.parse_signature(ps_chunks["OSGN"])
    runtime_link = translation.close_carrier_signature(ps_input, vs_output)
    require(translation.output_registers(ps_output) == [0], "runtime canary pixel shader must declare RT0 only")
    disassembler = translation.D3DDisassembler(compiler)
    declarations = translation.parse_runtime_declarations(disassembler.disassemble(ps_dxbc))
    require(declarations["textureRegisters"] == list(range(8)), "runtime canary texture register denominator changed")
    require(declarations["samplerRegisters"] == list(range(8)), "runtime canary sampler register denominator changed")

    translation.validate_receipt(translated)
    require(translated["target"]["occurrenceId"] == OCCURRENCE_ID, "translation receipt occurrence changed")
    require(translated["decision"]["sourceExactSamplerAdmission"] is False, "translation receipt overclaims sampler exactness")
    cases = translated["warpComparison"]["cases"]
    require(cases and all(row["rawDxbcMrt"][1] == [RT1_SENTINEL] * 4 and row["translatedHlslMrt"][1] == [RT1_SENTINEL] * 4 for row in cases), "offline RT1 sentinel evidence changed")

    receipt: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "canaryId": "dimensionmaster.skill.2050120.clip3.glasshole02.first-runtime-canary",
        "inputs": [
            descriptor(authored_path, "exact authored effect and occurrence"),
            descriptor(material_maps_path, "exact material map, source emitter VF, and NoDensity VS evidence"),
            descriptor(texture_samplers_path, "exact texture identity and sampler-axis provenance"),
            descriptor(translation_path, "fixed-fixture HLSL equation and RT sentinel evidence"),
            descriptor(uniform_values_path, "effective MIC values for multi-time exact-AST evaluation"),
            descriptor(runtime_include_path, "shared Glasshole02 translated equation ABI"),
            descriptor(runtime_shader_path, "focused runtime canary shader adapter"),
            descriptor(renderer_path, "focused authoring renderer canary gate and bindings"),
            descriptor(renderer_header_path, "focused canary identity and closed activation constants"),
            descriptor(effect_tool_path, "explicit authoring Tool canary toggle, staging, and rollback seam"),
            descriptor(effect_tool_header_path, "separate default-off Tool canary state"),
        ],
        "subject": {
            "effectAssetId": EFFECT_ASSET_ID,
            "occurrenceId": OCCURRENCE_ID,
            "occurrenceCount": 1,
            "targetId": TARGET_ID,
            "familyId": FAMILY_ID,
            "runtimeShaderProfileId": RUNTIME_PROFILE_ID,
            "sourceMaterialPath": SOURCE_MATERIAL,
            "parentMaterialPath": PARENT_MATERIAL,
        },
        "carrierAndVertexFactory": {
            "rendererShape": "sprite",
            "requiredModuleStableId": required["stableId"],
            "offsetCenter": True,
            "screenAlignment": "psa_rectangle",
            "dynamicModuleStableId": dynamic["stableId"],
            "dynamicParameterUpdateFlags": 15,
            "sourceEmitterVertexFactoryType": SOURCE_VF,
            "sourceNoDensityVertexShaderType": SOURCE_VS_TYPE,
            "sourceNoDensityVertexShaderIdHex": SOURCE_VS_ID,
            "sourceNoDensityVertexShaderSha256": SOURCE_VS_SHA256,
            "exactSourceVertexPixelSignatureClosure": True,
            "runtimeAdapterVertexPixelSignatureClosure": runtime_link,
            "rawSourceVertexShaderExecution": False,
            "sourceExactVertexConstantBuffer": False,
        },
        "resourceBindings": {
            "materialDdsSlotCount": 7,
            "materialDdsSlots": material_slots,
            "engineSceneDepth": {"textureRegister": "t2", "samplerRegister": "s0", "ownership": "ENGINE_SCENE_DEPTH", "ddsSlot": None},
            "declaredTextureRegisters": declarations["textureRegisters"],
            "declaredSamplerRegisters": declarations["samplerRegisters"],
            "textureRegisterCount": 8,
            "samplerRegisterCount": 8,
            "requiredSourceTextureMask": REQUIRED_MASK,
            "requiredSourceTextureMaskHex": "0x7f",
            "sourceExactColorSpaceBindingCount": 7,
            "sourceExactFilterSelectorBindingCount": 7,
            "sourceExactAddressAxisCount": texture_target["sourceExactAddressAxisCount"],
            "sourceExactAddressAxisDenominator": texture_target["sourceExactAddressAxisDenominator"],
            "sourceExactHardwareFilterBindingCount": 0,
            "fullSourceExactSampler": False,
            "previewSamplerCandidateCount": 8,
            "previewSamplerCandidates": preview_sampler_candidates,
        },
        "timeVaryingCb0": {
            "source": "EXACT_MATERIAL_MAP_UNIFORM_EXPRESSION_AST_PLUS_EFFECTIVE_MIC_VALUES_FOR_C1_THROUGH_C21",
            "clock": "OCCURRENCE_LOCAL_GAME_TIME_SECONDS",
            "runtimeSeamName": "g_Glasshole02LocalTimeSeconds",
            "evaluationTimesSeconds": [0.0, 0.25, 0.6],
            "evaluations": time_evaluations,
            "astTimeDependentCb0Slots": ast_time_dependent_slots,
            "valueChangingCb0SlotsAtSealedTimes": value_changing_slots,
            "distinctMaterialRowsSemanticHashCount": 3,
            "staticCb0PacketAllowed": False,
            "runtimeShaderConsumesLocalTime": True,
            "rendererUploadsOccurrenceLocalTime": True,
        },
        "runtimeCb0Contract": {
            "float4Count": 22,
            "rendererOwnedC0": {
                "slot": 0,
                "value": [1.0, 0.0, 0.0, 0.0],
                "role": "RUNTIME_PARTICLE_ALPHA_ALREADY_CARRIES_LIFE_ENVELOPE_EXTERNAL_OPACITY_EXCEPTION",
                "sourceExactMaterialAstParity": False,
            },
            "exactMaterialAstParitySlots": list(range(1, 22)),
            "exactMaterialAstParityFloat4Count": 21,
            "cppSelfTestTimesSeconds": [0.0, 0.25, 0.6],
            "cppSelfTestFixture": {
                "basis": "EXACT_AST_WITH_SYNTHETIC_TIME_SCALAR_OVERRIDE_FOR_NONZERO_DEPENDENCY_COVERAGE",
                "scalarParameter": "time",
                "sourceAuthoredValue": 0.0,
                "selfTestValue": 0.4,
            },
            "cppSelfTestSemanticHashes": cpp_self_test_hashes,
            "cppSelfTestFullRuntimeRowCount": 22,
            "cppZeroTailSlots": [3, 10, 11, 13],
            "fullCb0SourceExact": False,
            "materialRowsC1ThroughC21ExactAstParity": True,
        },
        "runtimeInputMapping": {
            "basis": "EXACT_AST_PARAMETER_DEFAULT_THEN_EFFECTIVE_MIC_OVERRIDE_CROSS_CHECKED_WITH_AUTHORED_SOURCE_PROFILE",
            "scalarParameterCount": 32,
            "parameterRowCount": 8,
            "parameterRows": runtime_parameter_rows,
            "parameterRowsSemanticSha256": canonical_json_sha256(
                runtime_parameter_rows
            ),
            "vectorParameterCount": 2,
            "vectors": runtime_vectors,
            "vectorsSemanticSha256": canonical_json_sha256(runtime_vectors),
            "cppStagingFields": {
                "parameterRows": "TypedTrailParameters",
                "auraColor": "vSourceVector0",
                "inHoleColor": "vSourceVector1",
            },
            "profileResolution": {
                "storedProfileIndex": 29,
                "resolver": "SourceMaterialProfileIndex",
                "effectiveCarrierGate": "EffectiveSourceMaterialProfileIndex",
                "resourceField": "iSourceMaterialProfile",
                "buildDispatchProfileIndex": 29,
                "failClosedValue": "UINT32_MAX",
            },
        },
        "runtimeShader": {
            "path": descriptor(runtime_shader_path, "runtime shader")["path"],
            "vertexEntry": "VS_MAIN",
            "pixelEntry": "PS_MAIN",
            "technique": "DefaultTechnique",
            "pass": "Glasshole02AlphaTwoSidedDepthRead",
            "compiledVertexDxbcSha256": sha256_bytes(vs_dxbc),
            "compiledPixelDxbcSha256": sha256_bytes(ps_dxbc),
            "vertexInputSignature": public_signature(vs_input),
            "vertexOutputSignature": public_signature(vs_output),
            "pixelInputSignature": public_signature(ps_input),
            "pixelOutputSignature": public_signature(ps_output),
            "pixelOutputRegisters": [0],
        },
        "rtContract": {
            "runtimeDeclaredRenderTargets": ["RT0"],
            "rt1BoundSentinel": RT1_SENTINEL,
            "offlineCaseCount": len(cases),
            "offlineRawAndTranslationLeaveRt1SentinelUntouched": True,
            "runtimePixelShaderCannotWriteRt1": True,
            "rt1SentinelRequiredAfterCanaryDraw": True,
        },
        "activation": {
            "scope": "AUTHORING_DEBUG_EXACT_OCCURRENCE_ONLY",
            "defaultEnabled": False,
            "productEnabled": False,
            "failClosed": True,
            "requiresAllSevenDds": True,
            "requiresSourceTextureMask": REQUIRED_MASK,
            "requiresSceneDepthT2": True,
            "requiresAllEightSamplers": True,
            "fallbackToApproximationOnCanaryFailure": False,
        },
        "toolFrontendContract": {
            "separateFlagDefaultOff": True,
            "separateCheckbox": True,
            "rawCookedCanaryMutuallyExclusive": True,
            "authoredExactOccurrenceOnly": True,
            "productAndReadOnlyOffBeforeStage": True,
            "translatedExecutionEnabledBeforeDocumentStage": True,
            "initialStageFailureFlagOffReleaseOrdinaryRestage": True,
            "productReadOnlyStageDisablesTranslatedExecution": True,
            "documentChangeReset": True,
            "frontendRuntimeAdmission": False,
        },
        "admission": {
            "exactAuthoredOccurrence": True,
            "exactCarrierAndSourceEmitterVf": True,
            "exactSevenDdsIdentity": True,
            "exactColorSpaceAndFilterSelectors": True,
            "sourceExactFullSampler": False,
            "focusedAuthoringCanaryContract": True,
            "rendererContractStatus": "CONTRACT_STAGED_AWAITING_CPP_RUNTIME_HARNESS",
            "rendererRuntimeAdmission": False,
            "defaultRuntime": False,
            "product": False,
            "rawSourceVertexShaderExecution": False,
            "visual": False,
        },
    }
    seal(receipt)
    validate_receipt(receipt)
    return receipt


def validate_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA and receipt.get("formatVersion") == FORMAT_VERSION, "runtime canary receipt schema/version changed")
    expected_hash = receipt.get("receiptSha256")
    require(isinstance(expected_hash, str) and len(expected_hash) == 64, "runtime canary receipt seal is absent")
    unsealed = dict(receipt)
    unsealed.pop("receiptSha256", None)
    require(canonical_json_sha256(unsealed) == expected_hash, "runtime canary receipt seal mismatch")
    subject = receipt.get("subject", {})
    require(subject.get("effectAssetId") == EFFECT_ASSET_ID and subject.get("occurrenceId") == OCCURRENCE_ID and subject.get("occurrenceCount") == 1, "runtime canary subject is not the exact single occurrence")
    require(subject.get("familyId") == FAMILY_ID and subject.get("runtimeShaderProfileId") == RUNTIME_PROFILE_ID, "runtime canary family/profile changed")
    resources = receipt.get("resourceBindings", {})
    require(resources.get("materialDdsSlotCount") == 7 and resources.get("requiredSourceTextureMask") == REQUIRED_MASK, "runtime canary seven-DDS/mask contract changed")
    require(resources.get("declaredTextureRegisters") == list(range(8)) and resources.get("declaredSamplerRegisters") == list(range(8)), "runtime canary t/s denominator changed")
    require(resources.get("fullSourceExactSampler") is False and resources.get("sourceExactHardwareFilterBindingCount") == 0, "runtime canary overclaims source-exact sampler state")
    preview_candidates = resources.get("previewSamplerCandidates", [])
    require(resources.get("previewSamplerCandidateCount") == 8 and [row.get("samplerRegister") for row in preview_candidates] == [f"s{index}" for index in range(8)], "runtime preview sampler candidate denominator changed")
    require(all(row.get("sourceExact") is False and "CANDIDATE" in str(row.get("fidelity")) for row in preview_candidates), "runtime preview sampler candidate was promoted to source exact")
    slots = resources.get("materialDdsSlots", [])
    require(len(slots) == 7 and [row.get("runtimeSourceTextureSlot") for row in slots] == list(range(7)), "runtime material DDS slots changed")
    require(all(row.get("sourceExactColorSpace") is True and row.get("sourceExactFilterSelector") is True and row.get("sourceExactHardwareFilter") is False and row.get("fullSourceExactSampler") is False for row in slots), "runtime sampler axes were conflated")
    require(resources.get("engineSceneDepth") == {"textureRegister": "t2", "samplerRegister": "s0", "ownership": "ENGINE_SCENE_DEPTH", "ddsSlot": None}, "runtime scene-depth slot changed")
    time_cb0 = receipt.get("timeVaryingCb0", {})
    require(time_cb0.get("runtimeSeamName") == "g_Glasshole02LocalTimeSeconds", "runtime canary local-time seam changed")
    require(time_cb0.get("evaluationTimesSeconds") == [0.0, 0.25, 0.6], "runtime canary CB0 time denominator changed")
    require(time_cb0.get("astTimeDependentCb0Slots") == [2, 3, 7, 8, 10, 11, 13, 15, 17, 19, 20, 21], "runtime canary AST time-dependent slot set changed")
    require(time_cb0.get("valueChangingCb0SlotsAtSealedTimes") == [2, 3, 10, 11, 13, 17, 19, 20, 21], "runtime canary value-changing slot set changed")
    evaluations = time_cb0.get("evaluations", [])
    require(len(evaluations) == 3 and len({row.get("nativeCb0MaterialRowsSemanticSha256") for row in evaluations}) == 3, "runtime canary permits a static CB0 packet")
    require(all(len(row.get("runtimeCb0Rows", [])) == 22 for row in evaluations), "runtime canary full CB0 self-test denominator changed")
    require(time_cb0.get("distinctMaterialRowsSemanticHashCount") == 3 and time_cb0.get("staticCb0PacketAllowed") is False and time_cb0.get("runtimeShaderConsumesLocalTime") is True and time_cb0.get("rendererUploadsOccurrenceLocalTime") is True, "runtime local-time seam is open")
    cb0 = receipt.get("runtimeCb0Contract", {})
    require(cb0.get("rendererOwnedC0", {}).get("slot") == 0 and cb0.get("rendererOwnedC0", {}).get("value") == [1.0, 0.0, 0.0, 0.0] and cb0.get("rendererOwnedC0", {}).get("sourceExactMaterialAstParity") is False, "renderer-owned c0 exception changed")
    require(cb0.get("exactMaterialAstParitySlots") == list(range(1, 22)) and cb0.get("exactMaterialAstParityFloat4Count") == 21 and cb0.get("materialRowsC1ThroughC21ExactAstParity") is True, "material AST parity scope is not c1..c21")
    require(cb0.get("cppSelfTestTimesSeconds") == [0.0, 0.25, 0.6] and cb0.get("cppSelfTestFullRuntimeRowCount") == 22 and cb0.get("cppZeroTailSlots") == [3, 10, 11, 13] and cb0.get("fullCb0SourceExact") is False, "C++ CB0 self-test/claim boundary changed")
    require(cb0.get("cppSelfTestFixture") == {"basis": "EXACT_AST_WITH_SYNTHETIC_TIME_SCALAR_OVERRIDE_FOR_NONZERO_DEPENDENCY_COVERAGE", "scalarParameter": "time", "sourceAuthoredValue": 0.0, "selfTestValue": 0.4}, "C++ CB0 self-test fixture provenance changed")
    require(len(cb0.get("cppSelfTestSemanticHashes", [])) == 3 and all(len(row.get("runtimeCb0Rows", [])) == 22 for row in cb0.get("cppSelfTestSemanticHashes", [])), "C++ CB0 self-test semantic hash/row denominator changed")
    mapping = receipt.get("runtimeInputMapping", {})
    require(mapping.get("scalarParameterCount") == 32 and mapping.get("parameterRowCount") == 8 and len(mapping.get("parameterRows", [])) == 8, "runtime named scalar mapping denominator changed")
    require([[lane.get("parameterName") for lane in row.get("lanes", [])] for row in mapping.get("parameterRows", [])] == GLASSHOLE_SCALAR_LANES, "runtime named scalar mapping order changed")
    require(mapping.get("parameterRowsSemanticSha256") == canonical_json_sha256(mapping.get("parameterRows")), "runtime named scalar mapping seal changed")
    require(mapping.get("vectorParameterCount") == 2 and [(row.get("runtimeField"), row.get("parameterName")) for row in mapping.get("vectors", [])] == [("vSourceVector0", "aura_color"), ("vSourceVector1", "in_hole_color")], "runtime named vector mapping changed")
    require(mapping.get("vectorsSemanticSha256") == canonical_json_sha256(mapping.get("vectors")), "runtime named vector mapping seal changed")
    require(mapping.get("profileResolution") == {"storedProfileIndex": 29, "resolver": "SourceMaterialProfileIndex", "effectiveCarrierGate": "EffectiveSourceMaterialProfileIndex", "resourceField": "iSourceMaterialProfile", "buildDispatchProfileIndex": 29, "failClosedValue": "UINT32_MAX"}, "runtime profile-29 mapping chain changed")
    shader = receipt.get("runtimeShader", {})
    require(shader.get("pixelOutputRegisters") == [0], "runtime shader is not RT0-only")
    rt = receipt.get("rtContract", {})
    require(rt.get("rt1BoundSentinel") == RT1_SENTINEL and rt.get("offlineRawAndTranslationLeaveRt1SentinelUntouched") is True and rt.get("runtimePixelShaderCannotWriteRt1") is True and rt.get("rt1SentinelRequiredAfterCanaryDraw") is True, "RT1 sentinel contract changed")
    activation = receipt.get("activation", {})
    require(activation.get("defaultEnabled") is False and activation.get("productEnabled") is False and activation.get("failClosed") is True and activation.get("fallbackToApproximationOnCanaryFailure") is False, "runtime canary activation became open")
    frontend = receipt.get("toolFrontendContract", {})
    for field in (
        "separateFlagDefaultOff",
        "separateCheckbox",
        "rawCookedCanaryMutuallyExclusive",
        "authoredExactOccurrenceOnly",
        "productAndReadOnlyOffBeforeStage",
        "translatedExecutionEnabledBeforeDocumentStage",
        "initialStageFailureFlagOffReleaseOrdinaryRestage",
        "productReadOnlyStageDisablesTranslatedExecution",
        "documentChangeReset",
    ):
        require(frontend.get(field) is True, f"Tool frontend contract is open: {field}")
    require(frontend.get("frontendRuntimeAdmission") is False, "Tool frontend contract overclaims runtime admission")
    admission = receipt.get("admission", {})
    require(admission.get("rendererContractStatus") == "CONTRACT_STAGED_AWAITING_CPP_RUNTIME_HARNESS", "renderer contract status overclaimed admission")
    for field in ("sourceExactFullSampler", "rendererRuntimeAdmission", "defaultRuntime", "product", "rawSourceVertexShaderExecution", "visual"):
        require(admission.get(field) is False, f"runtime canary overclaims {field}")


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n")
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    generated = build_receipt()
    if args.check:
        tracked = read_json(args.output)
        validate_receipt(tracked)
        require(tracked == generated, f"tracked runtime canary receipt is stale: {args.output}")
        print(f"PASS: Glasshole02 runtime canary contract is current ({generated['receiptSha256']})")
        return 0
    write_json_atomic(args.output, generated)
    print(f"wrote {args.output} ({generated['receiptSha256']})")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
