#!/usr/bin/env python3
"""Materialize the fail-closed Valtan FrontBackFront Tool canary contract.

The contract joins three source-exact material families to nine exact authored
element identities.  It deliberately admits only an RT0 bounded carrier
adapter in Effect Tool.  It does not admit the source vertex-factory/pass,
Product playback, or visual fidelity.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
RESTORATION_ROOT = (
    REPOSITORY_ROOT
    / "Data"
    / "Effects"
    / "Imported"
    / "Valtan"
    / "FrontBackFrontFamilyRestoration"
)

SCHEMA = "lostark.valtan-front-back-front-runtime-canary-contract"
TARGET_SCHEMA = "lostark.valtan-front-back-front-runtime-canary-contract-targets"
FORMAT_VERSION = 1
EFFECT_ASSET_ID = "effect.valtan.front-back-front.windup"

DEFAULT_TARGETS = RESTORATION_ROOT / "Valtan.front-back-front-runtime-canary-contract.targets.v1.json"
DEFAULT_AUTHORED = REPOSITORY_ROOT / "Data/Effects/Authored/effect.valtan.front-back-front.windup.effect.json"
DEFAULT_SOURCE_TARGETS = RESTORATION_ROOT / "Valtan.front-back-front-source-exact-family-targets.v1.json"
DEFAULT_SOURCE_RECEIPT = RESTORATION_ROOT / "Valtan.front-back-front-source-exact-family-receipt.v1.json"
DEFAULT_EXACT_PROGRAMS = RESTORATION_ROOT / "Valtan.front-back-front-exact-pixel-programs.v1.json"
DEFAULT_OUTPUT = RESTORATION_ROOT / "Valtan.front-back-front-runtime-canary-contract.receipt.v1.json"

EXPECTED_IMPLEMENTATION_FILES = {
    "Client/Public/Effect_ValtanTranslatedCanaryRuntime.h":
        "Tool-only three-family runtime adapter public contract",
    "Client/Private/Effect_ValtanTranslatedCanaryRuntime.cpp":
        "exact translated shader compile, live ABI binding, and fail-closed draw implementation",
    "Client/Public/Effect_DocumentRenderer.h":
        "renderer default-off Valtan canary packet and state contract",
    "Client/Private/Effect_DocumentRenderer.cpp":
        "exact occurrence staging and bounded RT0 renderer dispatch integration",
    "Client/Public/Effect_Tool.h":
        "Effect Tool default-off Valtan canary selection state contract",
    "Client/Private/Effect_Tool.cpp":
        "explicit Tool toggle, exact occurrence gate, rollback, and preview staging integration",
    "Client/Bin/ShaderFiles/Shader_Ue3ValtanDissolve01.hlsli":
        "masked-dissolve source-exact translated pixel equation ABI",
    "Client/Bin/ShaderFiles/Shader_VtxEffectUe3ValtanDissolve01.hlsl":
        "masked-dissolve bounded RT0 Tool carrier shader",
    "Client/Bin/ShaderFiles/Shader_Ue3ValtanGround04.hlsli":
        "ground-decal source-exact translated pixel equation ABI",
    "Client/Bin/ShaderFiles/Shader_VtxEffectUe3ValtanGround04.hlsl":
        "ground-decal bounded RT0 Tool carrier shader",
    "Client/Bin/ShaderFiles/Shader_Ue3ValtanCrack01.hlsli":
        "crack-translucent source-exact translated pixel equation ABI",
    "Client/Bin/ShaderFiles/Shader_VtxEffectUe3ValtanCrack01.hlsl":
        "crack-translucent bounded RT0 Tool carrier shader",
}

EXPECTED_RUNTIME_IDS = {
    "valtan-front-back-front-masked-dissolve-stone": [
        "par_n_rpbf_atk_01_02.em07",
    ],
    "valtan-front-back-front-ground-decal": [
        "par_n_rpbf_atk_01_02.em14",
    ],
    "valtan-front-back-front-crack-translucent": [
        "par_n_rpbf_atk_04_11.em00",
        "par_n_rpbf_atk_04_11.em01",
        "par_n_rpbf_atk_04_12.em00",
        "par_n_rpbf_atk_04_12.em01",
        "par_n_rpbf_atk_04_12.em02",
        "par_n_rpbf_atk_04_13.em00",
        "par_n_rpbf_atk_04_13.em01",
    ],
}

EXPECTED_SOURCE_IDS = {
    "valtan-front-back-front-masked-dissolve-stone": [
        "valtan.front-back-front.atk-01-02.source-order-07",
    ],
    "valtan-front-back-front-ground-decal": [
        "valtan.front-back-front.atk-01-02.source-order-14",
    ],
    "valtan-front-back-front-crack-translucent": [
        "valtan.front-back-front.atk-04-11.em00",
        "valtan.front-back-front.atk-04-11.em01",
        "valtan.front-back-front.atk-04-12.em00",
        "valtan.front-back-front.atk-04-12.em01",
        "valtan.front-back-front.atk-04-12.em02",
        "valtan.front-back-front.atk-04-13.em00",
        "valtan.front-back-front.atk-04-13.em01",
    ],
}

EXPECTED_FAMILY_IDENTITIES = {
    "valtan-front-back-front-masked-dissolve-stone": {
        "familyId": "ue3.material.fx.m.mi.n.00.fx.n.me.dissolve.04.011.ma.748e33633b4a",
        "rendererType": "MeshParticle",
        "sourceVertexFactoryType": "flocalvertexfactory",
        "sourceMaterialPath": "fx_m_mi_n_00.fx_n_me_dissolve_04_011_ma",
        "parentMaterialPath": "fx_m_mi_00.fx_m.fx_d_pa_dissolve_01_ma",
        "boundedAdapterKind": "TOOL_LOCAL_MESH_RT0",
    },
    "valtan-front-back-front-ground-decal": {
        "familyId": "ue3.material.fx.m.mi.n.00.fx.n.de.ground.04.30.tr.9ad14c432bdb",
        "rendererType": "LocalDecal",
        "sourceVertexFactoryType": "flocaldecalvertexfactory",
        "sourceMaterialPath": "fx_m_mi_n_00.fx_mi.fx_n_de_ground_04_30_tr",
        "parentMaterialPath": "fx_m_mi_04.fx_m.fx_d_de_ground_04_tr",
        "boundedAdapterKind": "TOOL_LOCAL_DECAL_RT0",
    },
    "valtan-front-back-front-crack-translucent": {
        "familyId": "ue3.material.fx.m.mi.o.00.fx.o.me.crack.01.01.tr.c9f38136093d",
        "rendererType": "MeshParticle",
        "sourceVertexFactoryType": "flocalvertexfactory",
        "sourceMaterialPath": "fx_m_mi_o_00.fx_mi.fx_o_me_crack_01_01_tr",
        "parentMaterialPath": "fx_m_mi_00.fx_m.fx_d_me_crack_01_tr",
        "boundedAdapterKind": "TOOL_LOCAL_MESH_RT0",
    },
}

EXPECTED_PIXEL_SHADERS = {
    "valtan-front-back-front-masked-dissolve-stone": "19bba55d0aaaffa320e175d3fa3bd71602fd469f3f90cdcc6e90dde636978c5b",
    "valtan-front-back-front-ground-decal": "88199e51d1d2faf27c6fd58e6690712e3a0eb0561f3d1829100cb34ced7f7c4e",
    "valtan-front-back-front-crack-translucent": "39503bb820e24101792acce4c87d161b7f12fb65dea1d836cc5ce4f3fcb93a94",
}

EXPECTED_ACTIVATION = {
    "toolOnly": True,
    "defaultOff": True,
    "failClosed": True,
    "fallbackToApproximationOnFailure": False,
    "productAdmission": False,
    "visualAdmission": False,
    "actualVfPassAdmission": False,
}

EXPECTED_CARRIER_ADAPTER = {
    "fidelity": "RT0_ONLY_BOUNDED_CARRIER_ADAPTER",
    "pixelOutputRegisters": [0],
    "rawSourceVertexShaderExecution": False,
    "rawSourceMrtExecution": False,
    "sourceVertexFactoryPassAdmission": False,
    "engineSceneCbExact": False,
}

EXPECTED_RUNTIME_REGRESSION_SEAMS = {
    "compileTimeAdmissionBoundaryCrossAsserted": True,
    "groundNeutralSceneCb2Lane": True,
    "groundRectResourcesBoundBeforeDraw": True,
    "ordinaryFailClosedExcludesValtan": True,
    "rendererAcceptsNonEmptyExactSubset": True,
    "toolFullNineAuthoredPrecheck": True,
    "previewProjectionRequiresExactAuthoredStage": True,
    "toolLabelsEngineSceneCbBoundedNotExact": True,
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8-sig") as source:
        value = json.load(source)
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def read_singleton_json_object(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8-sig") as source:
        value = json.load(source)
    if isinstance(value, list):
        require(len(value) == 1, f"expected one JSON object in array: {path}")
        value = value[0]
    require(isinstance(value, dict), f"JSON root must contain one object: {path}")
    return value


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(8 * 1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def canonical_json_sha256(value: Any) -> str:
    encoded = json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    return sha256_bytes(encoded)


def seal(value: dict[str, Any]) -> None:
    value.pop("receiptSha256", None)
    value["receiptSha256"] = canonical_json_sha256(value)


def display_path(path: Path) -> str:
    resolved = path.resolve()
    try:
        return resolved.relative_to(REPOSITORY_ROOT.resolve()).as_posix()
    except ValueError:
        return resolved.as_posix()


def descriptor(path: Path, role: str) -> dict[str, Any]:
    require(path.is_file(), f"required input is missing: {path}")
    return {
        "path": display_path(path),
        "byteSize": path.stat().st_size,
        "sha256": sha256_file(path),
        "role": role,
    }


def strip_cpp_comments(text: str) -> str:
    return re.sub(r"//[^\n]*|/\*.*?\*/", "", text, flags=re.DOTALL)


def cpp_function_body(text: str, marker: str) -> str:
    code = strip_cpp_comments(text)
    marker_offset = code.find(marker)
    require(marker_offset >= 0, f"runtime regression function seam is absent: {marker}")
    open_offset = code.find("{", marker_offset)
    require(open_offset >= 0, f"runtime regression function body is absent: {marker}")
    depth = 0
    for offset in range(open_offset, len(code)):
        if code[offset] == "{":
            depth += 1
        elif code[offset] == "}":
            depth -= 1
            if depth == 0:
                return code[open_offset + 1 : offset]
    raise ValueError(f"runtime regression function body is unterminated: {marker}")


def validate_runtime_regression_seams(
    runtime_header_text: str,
    runtime_cpp_text: str,
    renderer_cpp_text: str,
    tool_cpp_text: str,
) -> dict[str, bool]:
    runtime_header = strip_cpp_comments(runtime_header_text)
    for anchor in (
        "static constexpr bool_t DEFAULT_ENABLED = false;",
        "static constexpr bool_t FAIL_CLOSED = true;",
        "static constexpr bool_t PRODUCT_ENABLED = false;",
        "static constexpr bool_t VISUAL_ADMISSION = false;",
        "static constexpr bool_t ACTUAL_VERTEX_FACTORY_PASS = false;",
        "static constexpr bool_t ENGINE_SCENE_CB_EXACT = false;",
        "static constexpr bool_t SAMPLER_EXACT = false;",
        "static constexpr bool_t RT0_ONLY = true;",
    ):
        require(anchor in runtime_header, f"runtime compile-time admission boundary changed: {anchor}")
    renderer_set = cpp_function_body(
        renderer_cpp_text,
        "Set_AuthoringValtanTranslatedCanaryEnabled(",
    )
    for anchor in (
        "static_assert(!CValtanTranslatedCanaryRuntime::DEFAULT_ENABLED);",
        "static_assert(!CValtanTranslatedCanaryRuntime::PRODUCT_ENABLED);",
        "static_assert(CValtanTranslatedCanaryRuntime::FAIL_CLOSED);",
        "static_assert(!CValtanTranslatedCanaryRuntime::ENGINE_SCENE_CB_EXACT);",
    ):
        require(anchor in renderer_set, f"renderer/runtime admission cross-assert changed: {anchor}")

    ground = cpp_function_body(
        runtime_cpp_text, "CValtanTranslatedCanaryRuntime::Draw_Ground(")
    require(
        re.search(
            r"CB2\s*\[\s*3\s*\]\s*=\s*\{\s*0\.f\s*,\s*0\.f\s*,\s*0\.f\s*,\s*1\.f\s*\}\s*;",
            ground,
        ) is not None
        and re.search(
            r'Bind_RawValue\s*\(\s*"g_Ue3ValtanGround04CB2"\s*,\s*CB2\.data\(\)\s*,\s*sizeof\(CB2\)\s*\)',
            ground,
        ) is not None,
        "Ground bounded neutral cb2[3] binding seam changed",
    )
    bind_resources_offset = ground.find("pRect->Bind_Resources()")
    render_offset = ground.find("pRect->Render()")
    require(
        0 <= bind_resources_offset < render_offset,
        "Ground rectangle resources are not bound before draw",
    )

    prepared = cpp_function_body(
        renderer_cpp_text, "CEffectDocumentRenderer::Build_PreparedDocument(")
    require(
        re.search(
            r"const\s+bool_t\s+bOrdinaryFailClosed\s*=.*?"
            r"!m_bAuthoringValtanTranslatedCanaryEnabled\s*&&.*?"
            r"Element\.Material\.Execution\.bFailClosed",
            prepared,
            re.DOTALL,
        ) is not None,
        "ordinary fail-closed staging no longer excludes the armed Valtan canary",
    )

    stage_document = cpp_function_body(
        renderer_cpp_text, "CEffectDocumentRenderer::Stage_Document(")
    valtan_offset = stage_document.find(
        "if (m_bAuthoringValtanTranslatedCanaryEnabled)")
    codec_offset = stage_document.find(
        "if (!CEffectDocumentCodec::Validate_Drawable", valtan_offset)
    require(
        0 <= valtan_offset < codec_offset,
        "Valtan Stage_Document admission block changed",
    )
    valtan_stage = stage_document[valtan_offset:codec_offset]
    require(
        "if (iCount > 1u)" in valtan_stage
        and "iStagedOccurrenceCount += iCount;" in valtan_stage
        and "if (0u == iStagedOccurrenceCount)" in valtan_stage
        and "1u != iCount" not in valtan_stage,
        "Valtan Stage_Document no longer accepts a non-empty unique exact subset",
    )

    full_nine = cpp_function_body(
        tool_cpp_text, "CEffect_Tool::Has_ValtanTranslatedCanaryOccurrences(")
    for anchor in (
        "VALTAN_TRANSLATED_CANARY_OCCURRENCE_IDS",
        "if (nullptr != pFound)",
        "if (nullptr == pFound ||",
        "pFound->Detail.Mesh.fModelPreScale - 0.01f",
        "pFound->Material.SourceMaterial.bEnabled",
    ):
        require(anchor in full_nine, f"Tool full-nine Authored precheck changed: {anchor}")

    world_preview = cpp_function_body(
        tool_cpp_text, "const bool_t bAllowReadOnlySourceProjection)")
    stage_offset = world_preview.find(
        "const bool_t bValtanTranslatedCanaryStage =")
    projection_offset = world_preview.find(
        "Build_PreviewDocument(Document, bValtanTranslatedCanaryStage)")
    require(
        0 <= stage_offset < projection_offset
        and "EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource" in world_preview[stage_offset:projection_offset]
        and "!bAllowReadOnlySourceProjection" in world_preview[stage_offset:projection_offset]
        and "!Has_ProductCuePreview()" in world_preview[stage_offset:projection_offset]
        and "Has_ValtanTranslatedCanaryOccurrences(Document)" in world_preview[stage_offset:projection_offset],
        "Valtan preview carrier projection is no longer gated by exact Authored eligibility",
    )
    preview_projection = cpp_function_body(
        tool_cpp_text, "CEffect_Tool::Build_PreviewDocument(")
    require(
        "if (bAllowValtanTranslatedCanaryProjection &&" in preview_projection
        and "m_bValtanTranslatedCanaryEnabled" in preview_projection
        and "VALTAN_TRANSLATED_CANARY_EFFECT_ASSET_ID" in preview_projection,
        "Valtan bounded carrier projection guard changed",
    )

    require(
        "bounded engine scene CBs" in tool_cpp_text
        and "bounded neutral engine scene-CB lanes" in tool_cpp_text
        and re.search(r"exact\s+engine\s+scene[- ]CB", tool_cpp_text, re.IGNORECASE) is None,
        "Effect Tool no longer labels the engine scene-CB boundary truthfully",
    )
    return dict(EXPECTED_RUNTIME_REGRESSION_SEAMS)


def exactly_one(rows: Any, field: str, value: str, label: str) -> dict[str, Any]:
    require(isinstance(rows, list), f"{label} rows are absent")
    matches = [row for row in rows if isinstance(row, dict) and row.get(field) == value]
    require(len(matches) == 1, f"expected exactly one {label} {value}, got {len(matches)}")
    return matches[0]


def validate_targets_document(document: dict[str, Any]) -> None:
    require(document.get("schema") == TARGET_SCHEMA, "runtime canary target schema changed")
    require(document.get("formatVersion") == FORMAT_VERSION, "runtime canary target version changed")
    identity = document.get("identity", {})
    require(identity.get("encounter") == "VALTAN", "runtime canary encounter changed")
    require(identity.get("pattern") == "FRONT_BACK_FRONT", "runtime canary pattern changed")
    require(identity.get("effectAssetId") == EFFECT_ASSET_ID, "runtime canary effect identity changed")
    require(identity.get("scope") == "THREE_FAMILY_TOOL_RUNTIME_CANARY", "runtime canary scope changed")
    require(document.get("activation") == EXPECTED_ACTIVATION, "runtime canary activation policy changed")
    require(document.get("carrierAdapter") == EXPECTED_CARRIER_ADAPTER, "runtime canary carrier adapter changed")

    implementation_files = document.get("runtimeImplementationFiles", [])
    require(
        isinstance(implementation_files, list)
        and len(implementation_files) == len(EXPECTED_IMPLEMENTATION_FILES),
        "runtime canary implementation file denominator changed",
    )
    require(
        [row.get("path") for row in implementation_files if isinstance(row, dict)]
        == list(EXPECTED_IMPLEMENTATION_FILES),
        "runtime canary implementation file identity or order changed",
    )
    for row in implementation_files:
        path = row["path"]
        require(
            row.get("role") == EXPECTED_IMPLEMENTATION_FILES[path],
            f"runtime canary implementation role changed: {path}",
        )
        sha256 = row.get("sha256")
        require(
            isinstance(sha256, str)
            and len(sha256) == 64
            and sha256 == sha256.lower()
            and all(character in "0123456789abcdef" for character in sha256),
            f"runtime canary implementation SHA-256 is malformed: {path}",
        )

    families = document.get("families", [])
    require(isinstance(families, list) and len(families) == 3, "runtime canary family denominator changed")
    require(
        [row.get("targetId") for row in families] == list(EXPECTED_RUNTIME_IDS),
        "runtime canary family order or identity changed",
    )
    all_runtime_ids: list[str] = []
    all_source_ids: list[str] = []
    for row in families:
        target_id = row["targetId"]
        runtime_ids = row.get("runtimeElementIds")
        source_ids = row.get("sourceOccurrenceIds")
        require(runtime_ids == EXPECTED_RUNTIME_IDS[target_id], f"runtime element set changed: {target_id}")
        require(source_ids == EXPECTED_SOURCE_IDS[target_id], f"source occurrence set changed: {target_id}")
        require(len(source_ids) == len(runtime_ids), f"source/runtime occurrence mapping changed: {target_id}")
        require(row.get("exactPixelShaderSha256") == EXPECTED_PIXEL_SHADERS[target_id], f"exact PS identity changed: {target_id}")
        require(row.get("expectedAuthoredKind") == "particle", f"authored carrier kind changed: {target_id}")
        for field, expected in EXPECTED_FAMILY_IDENTITIES[target_id].items():
            require(row.get(field) == expected, f"family {field} identity changed: {target_id}")
        all_runtime_ids.extend(runtime_ids)
        all_source_ids.extend(source_ids)
    require(len(all_runtime_ids) == len(set(all_runtime_ids)) == 9, "runtime canary element IDs are duplicated")
    require(len(all_source_ids) == len(set(all_source_ids)) == 9, "source occurrence IDs are duplicated")
    require(
        document.get("summary")
        == {"familyCount": 3, "runtimeElementCount": 9, "exactPixelShaderCount": 3},
        "runtime canary target summary changed",
    )


def validate_source_target(config: dict[str, Any], row: dict[str, Any]) -> None:
    target_id = config["targetId"]
    for field in ("familyId", "rendererType", "sourceMaterialPath", "parentMaterialPath"):
        require(row.get(field) == config.get(field), f"source target {field} changed: {target_id}")
    require(row.get("occurrenceIds") == config.get("sourceOccurrenceIds"), f"source occurrence set changed: {target_id}")
    selection = row.get("expectedStructuralSelection", {})
    require(selection.get("vertexFactoryCandidateCount") == 1, f"source VF candidate is not unique: {target_id}")
    require(selection.get("expectedDxbc", {}).get("sha256") == config["exactPixelShaderSha256"], f"source target DXBC changed: {target_id}")


def validate_source_receipt(config: dict[str, Any], row: dict[str, Any]) -> None:
    target_id = config["targetId"]
    for field in ("familyId", "rendererType", "sourceMaterialPath", "parentMaterialPath"):
        require(row.get(field) == config.get(field), f"source receipt {field} changed: {target_id}")
    require(row.get("occurrenceIds") == config.get("sourceOccurrenceIds"), f"source receipt occurrence set changed: {target_id}")
    require(row.get("status") == "EXACT_MATERIAL_SHADER_MAP", f"source shader map is not exact: {target_id}")
    require(row.get("parentDefaultFallbackApplied") is False, f"source receipt used parent fallback: {target_id}")
    cooked = row.get("cookedPixelShader", {})
    require(cooked.get("exactOneDxbcContainer") is True, f"source receipt DXBC is not unique: {target_id}")
    require(cooked.get("dxbc", {}).get("sha256") == config["exactPixelShaderSha256"], f"source receipt DXBC changed: {target_id}")
    require(cooked.get("actualVfPassAdmission") is False, f"source receipt overclaims VF/pass admission: {target_id}")
    native = row.get("nativeShaderObjectBinding", {})
    require(native.get("status") == "EXACT_NATIVE_SHADER_OBJECT_BINDING", f"native source binding is not exact: {target_id}")
    require(native.get("runtimeAdmission") is False, f"source receipt overclaims runtime admission: {target_id}")
    require(native.get("actualVfPassAdmission") is False, f"native source binding overclaims VF/pass: {target_id}")


def validate_exact_program(config: dict[str, Any], row: dict[str, Any]) -> None:
    target_id = config["targetId"]
    for field in ("familyId", "rendererType", "sourceMaterialPath"):
        require(row.get(field) == config.get(field), f"exact program {field} changed: {target_id}")
    require(row.get("selectedVertexFactoryTypes") == [config["sourceVertexFactoryType"]], f"exact program source VF changed: {target_id}")
    require(row.get("dxbc", {}).get("sha256") == config["exactPixelShaderSha256"], f"exact program DXBC changed: {target_id}")
    require(row.get("pixelEquationEvidence") == "EXACT_PACKED_DXBC", f"exact pixel equation evidence changed: {target_id}")
    for field in ("actualVfPassAdmission", "runtimeAdmission", "productAdmission"):
        require(row.get(field) is False, f"exact program overclaims {field}: {target_id}")


def validate_translation(config: dict[str, Any], row: dict[str, Any]) -> None:
    target_id = config["targetId"]
    require(row.get("status") == "TRANSLATED", f"HLSL translation is not sealed: {target_id}")
    require(row.get("dxbcSha256") == config["exactPixelShaderSha256"], f"translation DXBC changed: {target_id}")
    require(row.get("functionName") == config["translatedFunctionName"], f"translation function changed: {target_id}")
    require(row.get("hlslSha256") == config["translatedHlslSha256"], f"translated HLSL changed: {target_id}")
    outputs = row.get("declarations", {}).get("outputs", [])
    require("o0" in outputs, f"translation has no RT0 output: {target_id}")


def authored_projection(
    authored: dict[str, Any], config: dict[str, Any]
) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for source_id, runtime_id in zip(
        config["sourceOccurrenceIds"], config["runtimeElementIds"], strict=True
    ):
        element = exactly_one(authored.get("elements"), "id", runtime_id, "authored element")
        material = element.get("material", {})
        source_profile = material.get("sourceProfile", {})
        require(element.get("visible") is True, f"authored element is hidden: {runtime_id}")
        require(element.get("kind") == config["expectedAuthoredKind"], f"authored element kind changed: {runtime_id}")
        require(material.get("sourceMaterialPath") == config["sourceMaterialPath"], f"authored source material changed: {runtime_id}")
        require(source_profile.get("enabled") is False, f"Product source profile was unexpectedly admitted: {runtime_id}")
        model_pre_scale = element.get("detail", {}).get("mesh", {}).get("modelPreScale")
        require(model_pre_scale == 0.01, f"authored modelPreScale must remain 0.01: {runtime_id}")
        resources = element.get("resources", [])
        require(isinstance(resources, list), f"authored resources are malformed: {runtime_id}")
        rows.append(
            {
                "sourceOccurrenceId": source_id,
                "runtimeElementId": runtime_id,
                "visible": True,
                "authoredKind": element["kind"],
                "sourceMaterialPath": material["sourceMaterialPath"],
                "sourceProfileEnabled": False,
                "modelPreScale": model_pre_scale,
                "resourceBindingsSemanticSha256": canonical_json_sha256(resources),
            }
        )
    return rows


def build_receipt(
    *,
    targets_path: Path = DEFAULT_TARGETS,
    authored_path: Path = DEFAULT_AUTHORED,
    source_targets_path: Path = DEFAULT_SOURCE_TARGETS,
    source_receipt_path: Path = DEFAULT_SOURCE_RECEIPT,
    exact_programs_path: Path = DEFAULT_EXACT_PROGRAMS,
) -> dict[str, Any]:
    targets = read_json(targets_path)
    validate_targets_document(targets)
    authored = read_json(authored_path)
    require(authored.get("effectAssetId") == EFFECT_ASSET_ID, "authored effect identity changed")
    source_targets = read_json(source_targets_path)
    source_receipt = read_json(source_receipt_path)
    exact_programs = read_json(exact_programs_path)

    runtime_implementation_inputs: list[dict[str, Any]] = []
    for implementation in targets["runtimeImplementationFiles"]:
        implementation_path = REPOSITORY_ROOT / implementation["path"]
        implementation_input = descriptor(
            implementation_path,
            implementation["role"],
        )
        require(
            implementation_input["sha256"] == implementation["sha256"],
            f"runtime canary implementation hash drifted: {implementation['path']}",
        )
        runtime_implementation_inputs.append(implementation_input)
    runtime_regression_seams = validate_runtime_regression_seams(
        (REPOSITORY_ROOT / "Client/Public/Effect_ValtanTranslatedCanaryRuntime.h").read_text(
            encoding="utf-8-sig"),
        (REPOSITORY_ROOT / "Client/Private/Effect_ValtanTranslatedCanaryRuntime.cpp").read_text(
            encoding="utf-8-sig"),
        (REPOSITORY_ROOT / "Client/Private/Effect_DocumentRenderer.cpp").read_text(
            encoding="utf-8-sig"),
        (REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp").read_text(
            encoding="utf-8-sig"),
    )

    families: list[dict[str, Any]] = []
    all_authored_projection: list[dict[str, Any]] = []
    translation_inputs: list[dict[str, Any]] = []
    for config in targets["families"]:
        target_id = config["targetId"]
        source_target = exactly_one(source_targets.get("targets"), "targetId", target_id, "source target")
        source_result = exactly_one(source_receipt.get("targets"), "targetId", target_id, "source receipt target")
        exact_program = exactly_one(exact_programs.get("programs"), "targetId", target_id, "exact pixel program")
        validate_source_target(config, source_target)
        validate_source_receipt(config, source_result)
        validate_exact_program(config, exact_program)

        translation_path = REPOSITORY_ROOT / config["translationReceiptPath"]
        translation = read_singleton_json_object(translation_path)
        validate_translation(config, translation)
        translation_inputs.append(descriptor(translation_path, f"{target_id} translated HLSL evidence"))

        occurrences = authored_projection(authored, config)
        all_authored_projection.extend(occurrences)
        source_outputs = [
            int(register[1:])
            for register in translation["declarations"]["outputs"]
            if isinstance(register, str) and register.startswith("o") and register[1:].isdigit()
        ]
        families.append(
            {
                "targetId": target_id,
                "familyId": config["familyId"],
                "rendererType": config["rendererType"],
                "sourceVertexFactoryType": config["sourceVertexFactoryType"],
                "sourceMaterialPath": config["sourceMaterialPath"],
                "parentMaterialPath": config["parentMaterialPath"],
                "exactPixelShaderSha256": config["exactPixelShaderSha256"],
                "translatedFunctionName": config["translatedFunctionName"],
                "translatedHlslSha256": config["translatedHlslSha256"],
                "sourcePixelOutputRegisters": source_outputs,
                "boundedAdapterKind": config["boundedAdapterKind"],
                "boundedAdapterPixelOutputRegisters": [0],
                "authoredOccurrences": occurrences,
                "evidence": {
                    "shaderMap": "EXACT_MATERIAL_SHADER_MAP",
                    "nativeShaderObjectBinding": "EXACT_NATIVE_SHADER_OBJECT_BINDING",
                    "pixelEquation": "EXACT_PACKED_DXBC",
                    "hlslTranslation": "TRANSLATED",
                },
                "admission": {
                    "toolCanaryContract": True,
                    "rendererRuntime": True,
                    "samplerExact": False,
                    "engineSceneCbExact": False,
                    "actualVfPass": False,
                    "product": False,
                    "visual": False,
                },
            }
        )

    authored_identity = {
        "path": display_path(authored_path),
        "effectAssetId": EFFECT_ASSET_ID,
        "selectedOccurrenceCount": len(all_authored_projection),
        "selectedOccurrenceIdentitySemanticSha256": canonical_json_sha256(all_authored_projection),
        "role": "exact authored identity projection; tuning-only fields outside the projection remain editable",
    }
    receipt: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "subject": {
            "encounter": "VALTAN",
            "pattern": "FRONT_BACK_FRONT",
            "effectAssetId": EFFECT_ASSET_ID,
            "familyCount": 3,
            "runtimeElementCount": 9,
        },
        "inputs": {
            "targets": descriptor(targets_path, "three-family Tool canary targets"),
            "authoredIdentity": authored_identity,
            "sourceExactTargets": descriptor(source_targets_path, "source-exact family targets"),
            "sourceExactReceipt": descriptor(source_receipt_path, "source-exact shader-map/native-binding receipt"),
            "exactPixelPrograms": descriptor(exact_programs_path, "exact packed DXBC programs"),
            "translations": translation_inputs,
            "runtimeImplementation": runtime_implementation_inputs,
        },
        "activation": targets["activation"],
        "carrierAdapter": targets["carrierAdapter"],
        "runtimeRegressionSeams": runtime_regression_seams,
        "families": families,
        "summary": {
            "exactShaderMapFamilyCount": 3,
            "exactPixelEquationCount": 3,
            "translatedHlslCount": 3,
            "toolCanaryFamilyCount": 3,
            "runtimeElementCount": 9,
            "boundedRt0AdapterCount": 3,
            "runtimeImplementationFileCount": len(EXPECTED_IMPLEMENTATION_FILES),
            "rendererRuntimeFamilyCount": 3,
            "runtimeRegressionSeamCount": len(EXPECTED_RUNTIME_REGRESSION_SEAMS),
        },
        "admission": {
            "contractStatus": "TOOL_RENDERER_RUNTIME_ADMITTED_BOUNDED_RT0",
            "toolCanaryContract": True,
            "rendererRuntimeAdmission": True,
            "defaultRuntime": False,
            "samplerExact": False,
            "engineSceneCbExact": False,
            "actualVfPass": False,
            "product": False,
            "visual": False,
        },
    }
    seal(receipt)
    validate_receipt(receipt)
    return receipt


def validate_receipt(receipt: dict[str, Any]) -> None:
    claimed_hash = receipt.get("receiptSha256")
    unsigned = dict(receipt)
    unsigned.pop("receiptSha256", None)
    require(claimed_hash == canonical_json_sha256(unsigned), "runtime canary receipt seal changed")
    require(receipt.get("schema") == SCHEMA, "runtime canary receipt schema changed")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "runtime canary receipt version changed")
    subject = receipt.get("subject", {})
    require(subject == {"encounter": "VALTAN", "pattern": "FRONT_BACK_FRONT", "effectAssetId": EFFECT_ASSET_ID, "familyCount": 3, "runtimeElementCount": 9}, "runtime canary subject changed")
    require(receipt.get("activation") == EXPECTED_ACTIVATION, "runtime canary activation became open")
    require(receipt.get("carrierAdapter") == EXPECTED_CARRIER_ADAPTER, "runtime canary is not an RT0-only bounded carrier adapter")
    require(
        receipt.get("runtimeRegressionSeams") == EXPECTED_RUNTIME_REGRESSION_SEAMS,
        "runtime regression seam evidence changed",
    )

    runtime_implementation = receipt.get("inputs", {}).get("runtimeImplementation", [])
    require(
        isinstance(runtime_implementation, list)
        and len(runtime_implementation) == len(EXPECTED_IMPLEMENTATION_FILES),
        "runtime canary implementation receipt denominator changed",
    )
    require(
        [row.get("path") for row in runtime_implementation if isinstance(row, dict)]
        == list(EXPECTED_IMPLEMENTATION_FILES),
        "runtime canary implementation receipt identity or order changed",
    )
    for row in runtime_implementation:
        path = row["path"]
        require(
            row.get("role") == EXPECTED_IMPLEMENTATION_FILES[path],
            f"runtime canary implementation receipt role changed: {path}",
        )
        actual = descriptor(REPOSITORY_ROOT / path, row["role"])
        require(
            row == actual,
            f"runtime canary implementation receipt is stale: {path}",
        )

    families = receipt.get("families", [])
    require(isinstance(families, list) and len(families) == 3, "runtime canary receipt family denominator changed")
    require([row.get("targetId") for row in families] == list(EXPECTED_RUNTIME_IDS), "runtime canary receipt family identity changed")
    all_runtime_ids: list[str] = []
    for family in families:
        target_id = family["targetId"]
        require(family.get("exactPixelShaderSha256") == EXPECTED_PIXEL_SHADERS[target_id], f"receipt exact PS changed: {target_id}")
        for field, expected in EXPECTED_FAMILY_IDENTITIES[target_id].items():
            require(family.get(field) == expected, f"receipt family {field} identity changed: {target_id}")
        require(family.get("boundedAdapterPixelOutputRegisters") == [0], f"family adapter is not RT0-only: {target_id}")
        require(family.get("evidence") == {"shaderMap": "EXACT_MATERIAL_SHADER_MAP", "nativeShaderObjectBinding": "EXACT_NATIVE_SHADER_OBJECT_BINDING", "pixelEquation": "EXACT_PACKED_DXBC", "hlslTranslation": "TRANSLATED"}, f"family evidence changed: {target_id}")
        require(family.get("admission") == {"toolCanaryContract": True, "rendererRuntime": True, "samplerExact": False, "engineSceneCbExact": False, "actualVfPass": False, "product": False, "visual": False}, f"family admission boundary changed: {target_id}")
        occurrences = family.get("authoredOccurrences", [])
        require([row.get("runtimeElementId") for row in occurrences] == EXPECTED_RUNTIME_IDS[target_id], f"receipt runtime occurrence set changed: {target_id}")
        require([row.get("sourceOccurrenceId") for row in occurrences] == EXPECTED_SOURCE_IDS[target_id], f"receipt source occurrence set changed: {target_id}")
        for occurrence in occurrences:
            require(occurrence.get("visible") is True, f"receipt admits hidden occurrence: {target_id}")
            require(occurrence.get("authoredKind") == "particle", f"receipt authored kind changed: {target_id}")
            require(occurrence.get("sourceMaterialPath") == family.get("sourceMaterialPath"), f"receipt material join changed: {target_id}")
            require(occurrence.get("sourceProfileEnabled") is False, f"receipt Product-admitted source profile: {target_id}")
            require(occurrence.get("modelPreScale") == 0.01, f"receipt lost 0.01 modelPreScale: {target_id}")
        all_runtime_ids.extend(EXPECTED_RUNTIME_IDS[target_id])
    require(len(all_runtime_ids) == len(set(all_runtime_ids)) == 9, "receipt runtime element IDs are duplicated")
    require(receipt.get("summary") == {"exactShaderMapFamilyCount": 3, "exactPixelEquationCount": 3, "translatedHlslCount": 3, "toolCanaryFamilyCount": 3, "runtimeElementCount": 9, "boundedRt0AdapterCount": 3, "runtimeImplementationFileCount": 12, "rendererRuntimeFamilyCount": 3, "runtimeRegressionSeamCount": 8}, "runtime canary receipt summary changed")
    require(receipt.get("admission") == {"contractStatus": "TOOL_RENDERER_RUNTIME_ADMITTED_BOUNDED_RT0", "toolCanaryContract": True, "rendererRuntimeAdmission": True, "defaultRuntime": False, "samplerExact": False, "engineSceneCbExact": False, "actualVfPass": False, "product": False, "visual": False}, "runtime canary receipt admission boundary changed")


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--targets", type=Path, default=DEFAULT_TARGETS)
    parser.add_argument("--authored", type=Path, default=DEFAULT_AUTHORED)
    parser.add_argument("--source-targets", type=Path, default=DEFAULT_SOURCE_TARGETS)
    parser.add_argument("--source-receipt", type=Path, default=DEFAULT_SOURCE_RECEIPT)
    parser.add_argument("--exact-programs", type=Path, default=DEFAULT_EXACT_PROGRAMS)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    generated = build_receipt(
        targets_path=args.targets,
        authored_path=args.authored,
        source_targets_path=args.source_targets,
        source_receipt_path=args.source_receipt,
        exact_programs_path=args.exact_programs,
    )
    if args.check:
        tracked = read_json(args.output)
        validate_receipt(tracked)
        require(tracked == generated, f"tracked runtime canary receipt is stale: {args.output}")
        print(f"PASS: Valtan FrontBackFront runtime canary contract is current ({generated['receiptSha256']})")
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
