#!/usr/bin/env python3

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
EFFECT_TOOL_CPP = REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
EFFECT_TOOL_HEADER = REPOSITORY_ROOT / "Client/Public/Effect_Tool.h"
VALTAN_PATTERN_TREE_CPP = REPOSITORY_ROOT / "Client/Private/ValtanPatternTree.cpp"
VALTAN_PATTERN_TREE_HEADER = REPOSITORY_ROOT / "Client/Public/ValtanPatternTree.h"
ENCOUNTER_PATH = (
    REPOSITORY_ROOT / "Data/Encounters/Valtan/ValtanEncounter.json"
)
COMBAT_OBJECT_PATH = (
    REPOSITORY_ROOT / "Data/Encounters/Valtan/ValtanCombatObjects.json"
)
CUE_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
PATTERN_BINDING_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
)
REFERENCE_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patterneffects.json"
)
BOSS_CATALOG_PATH = REPOSITORY_ROOT / "Data/Actors/BossCatalog.json"
AUTHORED_ROOT = REPOSITORY_ROOT / "Data/Effects/Authored"
SOURCE_CATALOG_PATH = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
RUNTIME_CATALOG_PATH = (
    REPOSITORY_ROOT / "Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json"
)
V1_ALIAS_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json"
)
RETIRED_CANARY_LIVE_FILES = (
    REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp",
    REPOSITORY_ROOT / "Client/Public/Effect_Tool.h",
    REPOSITORY_ROOT / "Client/Private/Effect_Object.cpp",
    REPOSITORY_ROOT / "Client/Public/Effect_Object.h",
    REPOSITORY_ROOT / "Client/Private/Effect_DocumentRenderer.cpp",
    REPOSITORY_ROOT / "Client/Public/Effect_DocumentRenderer.h",
    REPOSITORY_ROOT / "Client/Default/Client.vcxproj",
    REPOSITORY_ROOT / "Client/Default/Client.vcxproj.filters",
    REPOSITORY_ROOT
    / "Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj",
    REPOSITORY_ROOT
    / "Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj.filters",
)
RETIRED_CANARY_DEDICATED_PATHS = (
    REPOSITORY_ROOT / "Client/Public/Effect_ValtanTranslatedCanaryRuntime.h",
    REPOSITORY_ROOT / "Client/Private/Effect_ValtanTranslatedCanaryRuntime.cpp",
    REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_EffectExactLocalMeshBridge.hlsl",
    REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_EffectExactSpriteBridge.hlsl",
    REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_VtxEffectGlasshole02.hlsl",
    REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_VtxEffectUe3ValtanCrack01.hlsl",
    REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_VtxEffectUe3ValtanDissolve01.hlsl",
    REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_VtxEffectUe3ValtanGround04.hlsl",
    REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_Ue3ValtanCrack01.hlsli",
    REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_Ue3ValtanDissolve01.hlsli",
    REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_Ue3ValtanGround04.hlsli",
    REPOSITORY_ROOT
    / "Data/Effects/Imported/DimensionMaster/Materials/skill.2050120.clip3.glasshole02-runtime-canary.contract.receipt.json",
    REPOSITORY_ROOT
    / "Data/Effects/Imported/Valtan/FrontBackFrontFamilyRestoration/Valtan.front-back-front-runtime-canary-contract.receipt.v1.json",
    REPOSITORY_ROOT
    / "Data/Effects/Imported/Valtan/FrontBackFrontFamilyRestoration/Valtan.front-back-front-runtime-canary-contract.targets.v1.json",
    REPOSITORY_ROOT
    / "Tools/EffectPipeline/materialize_ue3_glasshole02_runtime_canary_contract.py",
    REPOSITORY_ROOT
    / "Tools/EffectPipeline/materialize_valtan_front_back_front_runtime_canary_contract.py",
    REPOSITORY_ROOT
    / "Tools/EffectPipeline/replay_ue3_glasshole02_runtime_rt0.py",
    REPOSITORY_ROOT
    / "Tools/EffectPipeline/test_materialize_ue3_glasshole02_runtime_canary_contract.py",
    REPOSITORY_ROOT
    / "Tools/EffectPipeline/test_materialize_valtan_front_back_front_runtime_canary_contract.py",
    REPOSITORY_ROOT
    / "Tools/EffectPipeline/test_replay_ue3_glasshole02_runtime_rt0.py",
)


def function_slice(text: str, signature: str, next_signature: str) -> str:
    start = text.index(signature)
    end = text.index(next_signature, start + len(signature))
    return text[start:end]


def validate_source_contract(cpp_text: str, header_text: str) -> None:
    tree = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
        "void Client::CEffect_Tool::Render_AllEffectsWindow(",
    )
    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )

    forbidden = (
        'ImGui::BeginCombo("Phase"',
        "Repeat rotation in every phase",
        "Only stages with an Effect",
        "m_iValtanPhaseFilter",
        "m_bValtanRepeatRotationPerPhase",
        "m_bValtanOnlyStagesWithEffect",
        "Refresh_ValtanBossPatternEffects",
        "Unmapped Product Canary",
        "Unmapped Valtan Authoring",
    )
    for token in forbidden:
        if token in cpp_text or token in header_text:
            raise AssertionError(f"legacy Valtan phase UI survived: {token}")

    required_tree = (
        "m_ValtanPatternTree.iIntroRotationIndex",
        "m_ValtanPatternTree.Gimmicks",
        "m_ValtanPatternTree.Rotation",
        'Render_ValtanPatternNode(Pattern, "Gimmick", strSearch)',
    )
    for token in required_tree:
        if token not in tree:
            raise AssertionError(f"flat Valtan pattern tree lost: {token}")
    if "m_ValtanPatternTree.Phases" in tree:
        raise AssertionError("All Effects must not repeat patterns through phase bands")

    ordered_tokens = (
        "Clip.ProductCues",
        "Stage.Effects",
        "Stage.CombatObjectEffects",
        'ImGui::SeparatorText("Saved Unified Effects")',
        'ImGui::SeparatorText("Animations / Semantic Stages")',
        "Render_ValtanStageRow(Stage)",
    )
    for token in ordered_tokens:
        if token not in pattern:
            raise AssertionError(f"Valtan Saved projection lost: {token}")
    positions = [pattern.index(token) for token in ordered_tokens]
    if positions != sorted(positions):
        raise AssertionError(
            "Saved Effect projection must precede animation/stage diagnostics"
        )
    required_pattern = (
        "SavedRowIndices",
        "ProductSources",
        "ReferenceStages",
        "CombatObjectStages",
        "std::erase_if(SavedRows",
        '"[PRODUCT] "',
        'ImGui::SmallButton("Open Saved Effect")',
        'ImGui::SmallButton("Play Saved Effect")',
        "Try_OpenValtanAuthoredEffect",
        "Try_PlayValtanSavedUnifiedEffect",
        "Try_OpenValtanSavedReferenceEffect",
        "pNonProductStage->ClipOccurrences",
    )
    for token in required_pattern:
        if token not in pattern:
            raise AssertionError(f"Valtan Saved row contract lost: {token}")
    if '"[REFERENCE] "' in pattern:
        raise AssertionError("reference-only rows must stay out of Saved Unified Effects")
    if "Try_PlaySavedUnifiedEffect(" in pattern:
        raise AssertionError("Valtan must not use the Player-only saved play path")


def validate_v1_alias_projection_contract(
    cpp_text: str, tree_cpp_text: str, tree_header_text: str
) -> None:
    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )
    required_tree = (
        "std::string strV1EffectAssetId;",
        "Cue.strV1EffectAssetId = SourceCue.strV1EffectAssetId;",
        "CValtanPatternEffectCueDocument::Load_Source(CueDocument, Error)",
    )
    combined_tree = tree_header_text + "\n" + tree_cpp_text
    for token in required_tree:
        if token not in combined_tree:
            raise AssertionError(f"typed V1 alias propagation lost: {token}")

    required_pattern = (
        "Cue.strV1EffectAssetId",
        "ResolveRow(Cue.strV1EffectAssetId)",
        "V1Row.bV1Alias = true",
        '"[PRODUCT] "',
        '"[V1] "',
        "PlaybackCue.strEffectAssetId = Row.strEffectAssetId",
    )
    for token in required_pattern:
        if token not in pattern:
            raise AssertionError(f"paired V0/V1 Saved row contract lost: {token}")
    if re.search(r"strEffectAssetId\s*\+\s*[^;]*v1\.unified", pattern):
        raise AssertionError("V1 Saved rows must consume the typed alias, not infer a suffix")


def validate_hit_schedule_parse_contract(tree_cpp_text: str) -> None:
    fields = {
        "durationMs": "Stage.iDurationMs",
        "hitCount": "Stage.iHitCount",
        "hitIntervalMs": "Stage.iHitIntervalMs",
        "hitDelayMs": "Stage.iHitDelayMs",
    }
    for field, target in fields.items():
        required = re.compile(
            rf'Read_RequiredUInt32\(\s*StageValue,\s*"{field}",\s*'
            rf'{re.escape(target)}\s*\)'
        )
        if required.search(tree_cpp_text) is None:
            raise AssertionError(
                f"Valtan pattern tree lost required uint32 parsing for {field}"
            )
        if re.search(rf'Read_Number\(\s*StageValue,\s*"{field}"\s*\)', tree_cpp_text):
            raise AssertionError(
                f"Valtan pattern tree silently defaults required timing field {field}"
            )
    if "Valtan encounter stage numeric field is missing or invalid" not in tree_cpp_text:
        raise AssertionError("Valtan pattern tree lost the required numeric failure status")
    float_fields = {
        "hitOuterRadius": "Stage.fHitOuterRadius",
        "hitInnerRadius": "Stage.fHitInnerRadius",
        "hitAngleDegrees": "Stage.fHitAngleDegrees",
        "hitLength": "Stage.fHitLength",
        "hitHalfWidth": "Stage.fHitHalfWidth",
    }
    for field, target in float_fields.items():
        required = re.compile(
            rf'Read_RequiredFiniteFloat\(\s*StageValue,\s*"{field}",\s*'
            rf'{re.escape(target)}\s*\)'
        )
        if required.search(tree_cpp_text) is None:
            raise AssertionError(
                f"Valtan pattern tree lost required finite parsing for {field}"
            )
        if re.search(rf'Read_Number\(\s*StageValue,\s*"{field}"\s*\)', tree_cpp_text):
            raise AssertionError(
                f"Valtan pattern tree silently defaults required hit-shape field {field}"
            )
    required_identity_tokens = (
        "Stage.strStageId.empty()",
        "Stage.strActionId.empty()",
        "!bValidStageKind",
        "!bValidHitShape",
        "Valtan encounter stage identity is missing or invalid",
    )
    for token in required_identity_tokens:
        if token not in tree_cpp_text:
            raise AssertionError(
                f"Valtan pattern tree lost required stage identity validation: {token}"
            )
    if re.search(
        r"if\s*\(Stage\.strActionId\.empty\(\)\)\s*continue\s*;",
        tree_cpp_text,
    ):
        raise AssertionError("Valtan pattern tree silently drops an invalid stage action")


def validate_drawable_preflight_contract(cpp_text: str) -> None:
    play_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_PlayValtanSavedUnifiedEffect(",
        "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
    )
    reference_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
        "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
    )
    product_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
        "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree(",
    )

    for name, helper, commit_token in (
        ("play", play_helper, "Is_UnifiedEffectActive(Cache)"),
        ("reference", reference_helper, "Try_LoadDocumentPath("),
    ):
        refresh = helper.find("Refresh_UnifiedEffectCache(")
        valid = helper.find("!Cache.bValid")
        drawable = helper.find("!Cache.bDrawable")
        commit = helper.find(commit_token)
        if min(refresh, valid, drawable, commit) < 0:
            raise AssertionError(f"{name} helper lost the full drawable preflight")
        if not (refresh < valid < commit and refresh < drawable < commit):
            raise AssertionError(
                f"{name} helper must reject invalid/non-drawable data before preview mutation"
            )

    refresh = product_helper.find("Refresh_UnifiedEffectCache(")
    valid = product_helper.find("!Cache.bValid")
    commit = product_helper.find("Try_LoadDocumentPath(")
    drawable = product_helper.find("!Cache.bDrawable")
    if min(refresh, valid, commit) < 0 or not (refresh < valid < commit):
        raise AssertionError("Product Open lost structural validation before load")
    if 0 <= drawable < commit:
        raise AssertionError(
            "Product Open must admit a structurally valid empty authoring document"
        )

    refresh_tree = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree(",
        "bool_t Client::CEffect_Tool::Matches_ValtanPatternSearch(",
    )
    if "m_ValtanUnifiedEffectCaches.clear();" not in refresh_tree:
        raise AssertionError("explicit Valtan Refresh must reopen repaired documents")

    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )
    open_button = pattern.find('ImGui::SmallButton("Open Saved Effect")')
    play_button = pattern.find('ImGui::SmallButton("Play Saved Effect")')
    refreshed_lookup = pattern.find("const auto RefreshedCache =", play_button)
    if min(open_button, play_button, refreshed_lookup) < 0 or not (
        open_button < play_button < refreshed_lookup
    ):
        raise AssertionError(
            "Saved row must reacquire its unordered cache iterator after button actions"
        )
    if "ObservedCache->" in pattern[open_button:]:
        raise AssertionError(
            "Saved row retained a possibly invalidated cache iterator across Open/Play"
        )
    open_guard = pattern[pattern.rfind("ImGui::BeginDisabled(", 0, open_button):open_button]
    play_guard = pattern[pattern.rfind("ImGui::BeginDisabled(", 0, play_button):play_button]
    if "bKnownNonDrawable" in open_guard:
        raise AssertionError("empty Product drafts must remain openable for authoring")
    if "bKnownInvalid" not in open_guard:
        raise AssertionError("known-invalid Product documents must remain open-locked")
    if not all(
        token in play_guard for token in ("bKnownInvalid", "bKnownNonDrawable")
    ):
        raise AssertionError(
            "invalid and non-drawable Product documents must remain play-locked"
        )

    save_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_SaveDocument()",
        "bool_t Client::CEffect_Tool::Try_SaveDocumentAs(",
    )
    cache_lookup = save_helper.find("m_ValtanUnifiedEffectCaches.find(")
    cache_reset = save_helper.find("ValtanCache->second = {};", cache_lookup)
    cache_refresh = save_helper.find("Refresh_UnifiedEffectCache(", cache_lookup)
    hot_reload = save_helper.find("Try_HotReloadSavedProduct()", cache_refresh)
    if min(cache_lookup, cache_reset, cache_refresh, hot_reload) < 0 or not (
        cache_lookup < cache_reset < cache_refresh < hot_reload
    ):
        raise AssertionError(
            "Authored save must refresh an observed Valtan Product cache before hot reload"
        )


def validate_animation_stage_metadata_contract(cpp_text: str) -> None:
    sections = (
        (
            "Render_ValtanStageRow",
            function_slice(
                cpp_text,
                "void Client::CEffect_Tool::Render_ValtanStageRow(",
                "void Client::CEffect_Tool::Render_ValtanClipOccurrence(",
            ),
        ),
        (
            "Render_ValtanClipOccurrence",
            function_slice(
                cpp_text,
                "void Client::CEffect_Tool::Render_ValtanClipOccurrence(",
                "void Client::CEffect_Tool::Render_ValtanProductCue(",
            ),
        ),
        (
            "Render_ValtanProductCue",
            function_slice(
                cpp_text,
                "void Client::CEffect_Tool::Render_ValtanProductCue(",
                "void Client::CEffect_Tool::Render_ValtanPatternNode(",
            ),
        ),
    )
    forbidden = (
        "Refresh_UnifiedEffectCache(",
        "Render_UnifiedEffectTree(",
        "Render_ValtanAuthoringOpenButton(",
    )
    for section_name, section in sections:
        for token in forbidden:
            if token in section:
                raise AssertionError(
                    f"{section_name} must remain animation/metadata-only: {token}"
                )


def validate_stage_reference_sequence_contract(cpp_text: str) -> None:
    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )
    reference_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
        "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
    )

    vector_parameter = re.search(
        r"const\s+std::vector\s*<\s*VALTAN_CLIP_OCCURRENCE_VIEW\s*>\s*&\s*"
        r"([A-Za-z_][A-Za-z0-9_]*)",
        reference_helper,
    )
    if vector_parameter is None:
        raise AssertionError(
            "saved stage-reference helper must receive the complete clip vector"
        )
    clips_parameter = vector_parameter.group(1)
    if re.search(
        rf"Play_ValtanStageSequence\s*\(\s*{re.escape(clips_parameter)}\s*\)",
        reference_helper,
    ) is None:
        raise AssertionError(
            "saved stage-reference helper must play the complete clip vector"
        )

    forbidden_pattern_tokens = (
        "1u == Stage.ClipOccurrences.size()",
        "Stage.ClipOccurrences.front()",
        "bAmbiguousReferenceAnimation",
        "choose a clip below",
    )
    for token in forbidden_pattern_tokens:
        if token in pattern:
            raise AssertionError(
                f"saved stage-reference rows must not select/disable by clip count: {token}"
            )
    if "Play_ValtanClipOccurrence(" in reference_helper or ".front()" in reference_helper:
        raise AssertionError(
            "saved stage-reference helper must not collapse a sequence to its first clip"
        )

    reference_calls = re.findall(
        r"Try_OpenValtanSavedReferenceEffect\s*\((.*?)\);",
        pattern,
        flags=re.DOTALL,
    )
    if not reference_calls:
        raise AssertionError("saved stage-reference Open/Play actions are missing")
    for call in reference_calls:
        if re.search(
            r"(?:ClipOccurrences|Row\.[A-Za-z_][A-Za-z0-9_]*Clips[A-Za-z0-9_]*)",
            call,
        ) is None:
            raise AssertionError(
                "saved stage-reference Open/Play must pass all stage clip occurrences"
            )


def validate_saved_row_aggregation_contract(cpp_text: str) -> None:
    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )
    if "SeenEffectAssetIds" in pattern:
        raise AssertionError(
            "saved rows must aggregate duplicate provenance, not use a first-wins set"
        )
    if re.search(
        r"\.insert\s*\([^;]*strEffectAssetId[^;]*\.second\s*\)\s*continue\s*;",
        pattern,
        flags=re.DOTALL,
    ) is not None:
        raise AssertionError(
            "saved rows must not discard later provenance after duplicate insertion"
        )
    if re.search(
        r"std::(?:unordered_)?map\s*<\s*std::string\s*,",
        pattern,
    ) is None:
        raise AssertionError(
            "saved rows need an effectAssetId index that resolves to an aggregate row"
        )

    row_start = pattern.find("struct SAVED_VALTAN_EFFECT_ROW")
    row_end = pattern.find("};", row_start)
    if row_start < 0 or row_end < 0:
        raise AssertionError("saved Valtan row structure is missing")
    row_structure = pattern[row_start:row_end]
    provenance_members = re.findall(
        r"std::vector\s*<[^;]+>\s+([A-Za-z_][A-Za-z0-9_]*)\s*;",
        row_structure,
    )
    provenance_members = [
        member
        for member in provenance_members
        if re.search(
            r"(?:provenance|source|origin|link|occurrence|cue|reference|combat)",
            member,
            flags=re.IGNORECASE,
        )
    ]
    if not provenance_members:
        raise AssertionError(
            "each saved row must retain aggregated cue/reference/combat provenance"
        )
    if not any(
        re.search(rf"\.{re.escape(member)}\.push_back\s*\(", pattern)
        for member in provenance_members
    ):
        raise AssertionError("saved row provenance is declared but never aggregated")

    has_path_fallback = False
    for empty_check in re.finditer(
        r"\b([A-Za-z_][A-Za-z0-9_]*)\.Path\.empty\s*\(\s*\)",
        pattern,
    ):
        owner = empty_check.group(1)
        fallback_window = pattern[empty_check.end() : empty_check.end() + 500]
        if re.search(rf"\b{re.escape(owner)}\.Path\s*=", fallback_window):
            has_path_fallback = True
            break
    if not has_path_fallback:
        raise AssertionError(
            "later provenance must fill an aggregate row whose saved path is still empty"
        )
    if (
        "The catalog path is authoritative" not in pattern
        or "Row.Path = Editable->second.Path" not in pattern
    ):
        raise AssertionError(
            "EffectCatalog must override a legacy reference path after provenance merge"
        )


def validate_saved_reference_load_guard_contract(cpp_text: str) -> None:
    reference_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
        "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
    )
    load_guard = reference_helper.find("Try_LoadDocumentPath(")
    if load_guard < 0:
        raise AssertionError("saved reference helper lost its unsaved-document load guard")
    target_selection = re.search(
        r"m_pCharacterPreviewPanel->Select_TargetAsset\s*\(",
        reference_helper,
    )
    if target_selection is not None and target_selection.start() < load_guard:
        raise AssertionError(
            "saved reference helper must not switch target before the load guard commits"
        )


def validate_pending_reference_preview_contract(cpp_text: str) -> None:
    execute = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
        "bool_t Client::CEffect_Tool::Refresh_AllEffects(",
    )
    required = (
        "Pending.ValtanReferenceClips.has_value()",
        "Play_ValtanStageSequence(Clips)",
        "Try_PlayActiveUnifiedEffect()",
        "CompleteValtanPreviewPartial",
    )
    for token in required:
        if token not in execute:
            raise AssertionError(
                f"pending saved-reference preview contract lost: {token}"
            )


def validate_manual_authoring_detail_contract(cpp_text: str) -> None:
    size = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_SizeDetail(",
        "void Client::CEffect_Tool::Render_AuthoringMaterialParameters(",
    )
    kind = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_KindDetail(",
        "void Client::CEffect_Tool::Render_SourceRecipeDetail(",
    )

    required_size = (
        '"Source Playback Tuning###SizeDetail"',
        "Tuning.fCount",
        "Tuning.fSize",
        "Tuning.fLifeTime",
        "Tuning.fSpeed",
        "Tuning.fRotation",
        "Tuning.fAlpha",
        'DragFloat2("Start Size"',
        'DragFloat2("End Size"',
        'DragFloat2("Decal Size"',
        'ImGui::DragFloat("Projection Depth"',
        'ImGui::DragFloat("Start Width"',
        'ImGui::DragFloat("End Width"',
    )
    for token in required_size:
        if token not in size:
            raise AssertionError(f"manual authoring Size surface lost: {token}")

    if "Is_SourceParticleCarrier(Element)" not in size:
        raise AssertionError(
            "Source Playback Tuning must use the mesh/sprite/decal carrier predicate"
        )
    source_start = size.index("if (bSourceParticleCarrier)")
    source_end = size.index("\n\t\treturn;", source_start)
    source_branch = size[source_start:source_end]
    if "Element.eKind == EFFECT_ELEMENT_KIND::DECAL" not in source_branch:
        raise AssertionError("source decal lost its working Projection Depth overlay")
    if 'ImGui::DragFloat("Projection Depth"' not in source_branch:
        raise AssertionError("source decal Projection Depth is not editable")
    if 'DragFloat2("Decal Size"' in source_branch:
        raise AssertionError(
            "source decal must use Source Size x instead of ignored Detail.Decal.vSize"
        )
    if size.index('DragFloat2("Decal Size"') < source_end:
        raise AssertionError("manual Decal Size must remain outside the source-owned branch")
    if "SourceScale.fSpawnDelay" in size or '"Spawn Delay x"' in size:
        raise AssertionError(
            "Source Playback Tuning must not expose the non-scheduled Spawn Delay axis"
        )

    preview_end = function_slice(
        cpp_text,
        "\tf32_t Element_PreviewEndSeconds(",
        "    bool Slot_Allowed(",
    )
    for token in (
        "Is_PreviewParticleSimulationElement(Element)",
        "SourceScale.fLifeTime",
        "SourceRecipe.fEmitterDelaySeconds",
        "SourceRecipe.fEmitterDurationSeconds",
        "SourceRecipe.iEmitterLoopCount",
    ):
        if token not in preview_end:
            raise AssertionError(f"Tool preview duration lost source schedule parity: {token}")
    recalculate = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Recalculate_PreviewDuration(\n    const EFFECT_DOCUMENT_DESC& Document)",
        "bool_t Client::CEffect_Tool::Has_UnsavedWork() const",
    )
    if "Element_PreviewEndSeconds(Element)" not in recalculate:
        raise AssertionError(
            "document preview duration must consume the shared Element end calculation"
        )

    duplicate_fields = (
        "Detail.Mesh.fModelPreScale",
        "Detail.Particle.vStartSize",
        "Detail.Particle.vEndSize",
        "Detail.Decal.vSize",
        "Detail.Decal.fDepth",
        "Detail.Trail.fStartWidth",
        "Detail.Trail.fEndWidth",
    )
    for token in duplicate_fields:
        if token in kind:
            raise AssertionError(f"Type Detail duplicates the Size owner: {token}")

    required_kind = (
        "Detail.Particle.iMaxParticles",
        "Detail.Particle.iRandomSeed",
        "bCompilerOwnedSourceParticle",
        "if (bCompilerOwnedSourceParticle)",
        "Detail.Particle.fSpawnRatePerSecond",
        "Detail.Particle.vLifeTimeSeconds",
        "Detail.Particle.vInitialPositionMin",
        "Detail.Particle.SpawnShape",
        "Detail.Particle.InitialVelocity",
        "Detail.Particle.vDynamicParameterStart",
        "Detail.Particle.bLocalSpace",
        "Detail.Particle.TargetAttractor",
    )
    for token in required_kind:
        if token not in kind:
            raise AssertionError(f"working Particle overlay was hidden: {token}")

    retired_labels = (
        "Source Trim",
        "Exact Cooked Canary",
        "Translated Glasshole02 Canary",
        "Translated Valtan Core-Three Canary",
    )
    for token in retired_labels:
        if token in cpp_text:
            raise AssertionError(f"retired Effect Detail surface survived: {token}")


def stage_effect_asset_id(pattern: dict[str, object], stage: dict[str, object]) -> str:
    pattern_action = str(pattern["actionId"])
    stage_action = str(stage["actionId"])
    pieces = pattern_action.split(".", 2)
    pattern_slug = pieces[2] if len(pieces) == 3 else pattern_action
    prefix = pattern_action + "."
    if stage_action.startswith(prefix) and len(stage_action) > len(prefix):
        stage_slug = stage_action[len(prefix) :]
    else:
        stage_slug = str(stage["stageId"]).lower().replace("_", "-")
    return f"effect.valtan.{pattern_slug}.{stage_slug}"


def project_saved_rows(
    include_v1_aliases: bool = True,
    include_reference_only: bool = False,
) -> tuple[dict[str, list[str]], int]:
    encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))
    cue_document = json.loads(CUE_PATH.read_text(encoding="utf-8"))
    reference_document = json.loads(REFERENCE_PATH.read_text(encoding="utf-8"))
    boss_catalog = json.loads(BOSS_CATALOG_PATH.read_text(encoding="utf-8"))
    alias_document = json.loads(V1_ALIAS_PATH.read_text(encoding="utf-8"))
    v1_aliases = {
        row["effectAssetId"]: row["v1EffectAssetId"]
        for row in alias_document["aliases"]
    }

    cues_by_stage: dict[tuple[str, str, str], list[str]] = {}
    for cue in cue_document["cues"]:
        key = (cue["patternId"], cue["stageId"], cue["actionId"])
        cues_by_stage.setdefault(key, []).append(cue["effectAssetId"])
    references_by_action = {
        binding["actionId"]: binding["effectAssetId"]
        for binding in reference_document["bindings"]
    }
    valtan = next(
        boss
        for boss in boss_catalog["bosses"]
        if boss["archetypeId"] == "BOSS_VALTAN"
    )
    combat_visuals = {
        visual["combatObjectArchetypeId"]: visual["effectAssetId"]
        for visual in valtan["combatObjectVisuals"]
    }

    projected: dict[str, list[str]] = {}
    raw_links = 0
    for pattern in encounter["patterns"]:
        ordered: list[str] = []
        seen: set[str] = set()

        def add(effect_id: str) -> None:
            nonlocal raw_links
            raw_links += 1
            if effect_id not in seen:
                seen.add(effect_id)
                ordered.append(effect_id)

        for stage in pattern["stages"]:
            key = (pattern["patternId"], stage["stageId"], stage["actionId"])
            product_ids = cues_by_stage.get(key, [])
            for effect_id in product_ids:
                add(effect_id)
                if include_v1_aliases and effect_id in v1_aliases:
                    add(v1_aliases[effect_id])

            explicit = references_by_action.get(stage["actionId"])
            if include_reference_only and explicit:
                add(explicit)

            combat_ids: list[str] = []
            for action in stage.get("actions", []):
                if action.get("kind") != "SPAWN_COMBAT_OBJECT":
                    continue
                effect_id = combat_visuals.get(action.get("targetId"))
                if effect_id:
                    combat_ids.append(effect_id)

            if include_reference_only and not product_ids and not combat_ids:
                candidate = stage_effect_asset_id(pattern, stage)
                if (AUTHORED_ROOT / f"{candidate}.effect.json").is_file():
                    add(candidate)
            for effect_id in combat_ids:
                add(effect_id)

        projected[pattern["patternId"]] = ordered
    return projected, raw_links


class EffectToolValtanSavedRowsTests(unittest.TestCase):
    def test_source_contract_is_flat_lazy_and_valtan_specific(self) -> None:
        validate_source_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8"),
            EFFECT_TOOL_HEADER.read_text(encoding="utf-8"),
        )

    def test_typed_v1_aliases_project_as_paired_saved_rows(self) -> None:
        validate_v1_alias_projection_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8"),
            VALTAN_PATTERN_TREE_CPP.read_text(encoding="utf-8"),
            VALTAN_PATTERN_TREE_HEADER.read_text(encoding="utf-8"),
        )

    def test_pattern_tree_requires_all_hit_schedule_timing_fields(self) -> None:
        validate_hit_schedule_parse_contract(
            VALTAN_PATTERN_TREE_CPP.read_text(encoding="utf-8")
        )

    def test_saved_open_and_play_fail_closed_before_preview_mutation(self) -> None:
        validate_drawable_preflight_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_animation_stage_rows_do_not_decode_or_open_saved_documents(self) -> None:
        validate_animation_stage_metadata_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_stage_reference_play_uses_the_complete_clip_sequence(self) -> None:
        validate_stage_reference_sequence_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_saved_rows_aggregate_all_provenance_and_path_fallbacks(self) -> None:
        validate_saved_row_aggregation_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_saved_reference_load_guard_precedes_target_selection(self) -> None:
        validate_saved_reference_load_guard_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_pending_saved_reference_replays_animation_and_effect(self) -> None:
        validate_pending_reference_preview_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_high_jump_world_owner_and_target_axe_timing_are_exact(self) -> None:
        encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))
        high_jump = next(
            pattern
            for pattern in encounter["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
        )
        stages = {stage["stageId"]: stage for stage in high_jump["stages"]}
        self.assertEqual(1933, stages["TAKEOFF"]["durationMs"])
        self.assertEqual(8000, stages["AIRBORNE"]["durationMs"])
        self.assertEqual(3200, stages["LAND"]["durationMs"])
        self.assertEqual(400, stages["RECOVERY"]["durationMs"])
        self.assertEqual(
            [
                {
                    "trigger": "ENTER",
                    "kind": "SPAWN_COMBAT_OBJECT",
                    "targetId": "combatobject.valtan.high-jump.target-axe",
                    "value": 1,
                    "durationMs": 0,
                }
            ],
            stages["AIRBORNE"]["actions"],
        )

        combat_objects = json.loads(
            COMBAT_OBJECT_PATH.read_text(encoding="utf-8")
        )
        target_axe = next(
            row
            for row in combat_objects["objects"]
            if row["combatObjectArchetypeId"]
            == "combatobject.valtan.high-jump.target-axe"
        )
        self.assertEqual(4200, target_axe["lifeMs"])
        self.assertEqual(1, len(target_axe["hits"]))
        self.assertEqual(1200, target_axe["hits"][0]["atMs"])
        self.assertEqual(1, target_axe["hits"][0]["repeatCount"])

        cpp_text = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        header_text = EFFECT_TOOL_HEADER.read_text(encoding="utf-8")
        pattern = function_slice(
            cpp_text,
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
        )
        reference_open = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
            "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
        )
        duration = function_slice(
            cpp_text,
            "void Client::CEffect_Tool::Recalculate_PreviewDuration(",
            "bool_t Client::CEffect_Tool::Has_UnsavedWork() const",
        )
        for token in (
            "iValtanWorldOwnerStageDurationMs",
            "m_iValtanWorldOwnerStageDurationMs",
        ):
            self.assertIn(token, header_text)
        self.assertIn("pNonProductStage->iDurationMs", pattern)
        self.assertIn("iWorldOwnerStageDurationMs", reference_open)
        self.assertIn("Recalculate_PreviewDuration();", reference_open)
        self.assertIn("m_iValtanWorldOwnerStageDurationMs", duration)
        self.assertIn("(std::max)(", duration)

    def test_effect_detail_has_one_working_owner_per_manual_tuning_axis(self) -> None:
        validate_manual_authoring_detail_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_retired_authoring_canaries_have_no_live_execution_path(self) -> None:
        live_text = "\n".join(
            path.read_text(encoding="utf-8") for path in RETIRED_CANARY_LIVE_FILES
        )
        forbidden = (
            "Set_AuthoringExactPreviewExecutionEnabled",
            "Set_AuthoringGlasshole02TranslatedCanaryEnabled",
            "Set_AuthoringValtanTranslatedCanaryEnabled",
            "Effect_ValtanTranslatedCanaryRuntime",
            "Shader_EffectExactLocalMeshBridge",
            "Shader_EffectExactSpriteBridge",
            "Shader_VtxEffectGlasshole02",
            "Shader_VtxEffectUe3Valtan",
        )
        for token in forbidden:
            self.assertNotIn(token, live_text)
        for path in RETIRED_CANARY_DEDICATED_PATHS:
            self.assertFalse(path.exists(), str(path))

    def test_whirlwind_carrier_v1_plays_on_spin_clip(self) -> None:
        encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))
        pattern = next(
            row
            for row in encounter["patterns"]
            if row["patternId"] == "VALTAN_WHIRLWIND"
        )
        self.assertEqual(
            ["WINDUP", "SPIN", "RECOVERY"],
            [row["stageId"] for row in pattern["stages"]],
        )
        stages = {row["stageId"]: row for row in pattern["stages"]}
        self.assertEqual(1333, stages["WINDUP"]["durationMs"])
        self.assertEqual(1400, stages["SPIN"]["durationMs"])
        self.assertEqual(4, stages["SPIN"]["hitCount"])
        self.assertEqual(350, stages["SPIN"]["hitIntervalMs"])
        self.assertEqual(1467, stages["RECOVERY"]["durationMs"])

        cues = json.loads(CUE_PATH.read_text(encoding="utf-8"))["cues"]
        matches = [
            row
            for row in cues
            if row["bindingId"]
            == "cue.valtan.carrier-v1.attack.whirlwind.recovery.clip-01"
        ]
        self.assertEqual(1, len(matches))
        cue = matches[0]
        self.assertEqual("VALTAN_WHIRLWIND", cue["patternId"])
        self.assertEqual("SPIN", cue["stageId"])
        self.assertEqual("valtan.attack.whirlwind.active", cue["actionId"])
        self.assertEqual(
            "valtan.attack.whirlwind.active.clip.01",
            cue["clipOccurrenceId"],
        )
        self.assertEqual(
            "effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01",
            cue["effectAssetId"],
        )
        self.assertEqual("natural", cue["stopPolicy"])
        self.assertEqual("once", cue["repeatPolicy"])
        self.assertEqual(0, cue["sourceStartMs"])
        self.assertIsNone(cue["sourceEndMs"])

        bindings = {
            row["actionId"]: row["clips"]
            for row in json.loads(
                PATTERN_BINDING_PATH.read_text(encoding="utf-8")
            )["bindings"]
        }
        self.assertEqual(
            [
                {
                    "clipOccurrenceId": "valtan.attack.whirlwind.active.clip.01",
                    "clip": "mesh_att_battle_20_03",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 0,
                    "playRate": 0.761904762,
                    "loop": True,
                }
            ],
            bindings["valtan.attack.whirlwind.active"],
        )
        self.assertEqual(
            "mesh_att_battle_20_04",
            bindings["valtan.attack.whirlwind.recovery"][0]["clip"],
        )

        effect_id = cue["effectAssetId"]
        source_catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        source_path = REPOSITORY_ROOT / "Data" / source_catalog[effect_id][
            "authoringPath"
        ]
        source_document = json.loads(source_path.read_text(encoding="utf-8"))
        self.assertEqual(
            "VALTAN_WHIRLWIND / SPIN / carrier V1",
            source_document["displayName"],
        )
        source_duration = 1.4 * 0.761904762
        for element in source_document["elements"]:
            if element["kind"] == "trail":
                preview_end = (
                    element["detail"]["timing"]["lifeTimeSeconds"]
                    + element["detail"]["trail"]["pointLifeTimeSeconds"]
                )
            else:
                preview_end = (
                    element["detail"]["timing"]["lifeTimeSeconds"]
                    + element["detail"]["particle"]["lifeTimeSeconds"][1]
                )
            self.assertLessEqual(preview_end, source_duration + 0.000001)
            self.assertAlmostEqual(1.4, preview_end / 0.761904762, places=5)

        v1_document = json.loads(
            (
                AUTHORED_ROOT
                / "effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.v1.unified.effect.json"
            ).read_text(encoding="utf-8")
        )
        self.assertEqual(
            "VALTAN_WHIRLWIND / SPIN / carrier V1 [V1]",
            v1_document["displayName"],
        )
        for element in v1_document["elements"]:
            timing = element["detail"]["timing"]
            preview_end = (
                timing["startDelaySeconds"]
                + timing["lifeTimeSeconds"]
                + element["detail"]["particle"]["lifeTimeSeconds"][1]
            )
            self.assertLessEqual(preview_end, source_duration + 0.000001)

        trail = next(
            row
            for row in source_document["elements"]
            if row["id"] == "whirlwind.trail.20.axe.main"
        )
        self.assertEqual(256, trail["detail"]["trail"]["maxPoints"])
        self.assertAlmostEqual(
            1.0 / 120.0,
            trail["detail"]["trail"]["sampleIntervalSeconds"],
            places=7,
        )
        self.assertGreater(
            trail["detail"]["trail"]["tilingDistanceWorldUnits"], 0
        )
        self.assertGreater(
            trail["detail"]["trail"]["distanceTessellationStepWorldUnits"],
            0,
        )

        header_text = EFFECT_TOOL_HEADER.read_text(encoding="utf-8")
        tree_header_text = VALTAN_PATTERN_TREE_HEADER.read_text(encoding="utf-8")
        tree_cpp_text = VALTAN_PATTERN_TREE_CPP.read_text(encoding="utf-8")
        cpp_text = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        self.assertIn("uint32_t iStageDurationMs = 0u;", tree_header_text)
        self.assertIn(
            "Cue.iStageDurationMs = SourceCue.iStageDurationMs;", tree_cpp_text
        )
        self.assertIn("VALTAN_PRODUCT_PREVIEW", header_text)
        play_cue = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Play_ValtanProductCue(",
            "bool_t Client::CEffect_Tool::Play_ValtanStageSequence(",
        )
        self.assertIn("0u == Cue.iStageDurationMs", play_cue)
        visible = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Is_ProductCueVisible(",
            "bool_t Client::CEffect_Tool::Restore_ValtanProductPreviewPlayback(",
        )
        self.assertIn(
            "m_ValtanProductPreview->Cue.iStageDurationMs", visible
        )
        duration = function_slice(
            cpp_text,
            "void Client::CEffect_Tool::Recalculate_PreviewDuration(",
            "bool_t Client::CEffect_Tool::Has_UnsavedWork() const",
        )
        self.assertIn(
            "m_ValtanProductPreview->Cue.iStageDurationMs", duration
        )

    def test_optional_authored_slot_delete_and_hit_base_only_contract(self) -> None:
        hit_path = (
            AUTHORED_ROOT
            / "effect.valtan.carrier-v1.attack.four-slash.active.clip-01.effect.json"
        )
        document = json.loads(hit_path.read_text(encoding="utf-8"))
        elements = {row["id"]: row for row in document["elements"]}
        self.assertEqual(
            {
                "base": "Effect/Valtan/Textures/FX_TEX_04/fx_h_hit_01.dds",
            },
            {
                row["slotId"]: row["assetId"]
                for row in elements["valtan.clip01.hit-spark.01"]["resources"]
            },
        )
        self.assertEqual(
            {
                "base": "Effect/Valtan/Textures/FX_TEX_00/fx_a_hit_007.dds",
            },
            {
                row["slotId"]: row["assetId"]
                for row in elements["impact.fragments.hit_007"]["resources"]
            },
        )

        cpp_text = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        for token in (
            "bool Is_DirectHandAuthoredElement(",
            "bool Is_OptionalHandAuthoredResourceSlot(",
            "Element.strSourceNode.empty()",
            'Element.strSourceNode.starts_with("authored-copy:")',
            'strSlotId == "base"',
            "Element.SourceRecipe.bEnabled",
            "Element.SourcePresentation.bEnabled",
            "Element.Material.SourceMaterial.bEnabled",
            "Element.Material.strSourceMaterialPath.empty()",
            "Element.Material.Execution.bEnabled",
            '"Delete Selected Slot"',
        ):
            self.assertIn(token, cpp_text)
        bind_slot = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Try_BindResource(",
            "bool_t Client::CEffect_Tool::Try_ClearSelectedSlot()",
        )
        authored_template = bind_slot[
            bind_slot.index("const bool_t bAuthoredTemplateSlot =") :
            bind_slot.index("if (bUnlockMissingBaseSourceDecal")
        ]
        self.assertIn(
            "Is_DirectHandAuthoredElement(*pElement)", authored_template
        )
        self.assertLess(
            authored_template.index("Is_DirectHandAuthoredElement(*pElement)"),
            authored_template.index("nullptr == pMaterialLane"),
        )
        clear_slot = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Try_ClearSelectedSlot()",
            "bool_t Client::CEffect_Tool::Try_SetSelectedTrailFollowAnchor(",
        )
        ordered_tokens = (
            "!bHasAuthoringOverride",
            "nullptr == Find_MaterialExecutionLane(",
            "nullptr == Find_SourceMaterialTexture(",
            "Is_OptionalHandAuthoredResourceSlot(",
            "Element.ResourceBindings.erase(Binding);",
            "Try_CommitDocument(std::move(Staged))",
            "m_strSelectedResourceAssetId.clear();",
            "Save Changes to persist it.",
        )
        for token in ordered_tokens:
            self.assertIn(token, clear_slot)
        self.assertEqual(
            sorted(clear_slot.index(token) for token in ordered_tokens),
            [clear_slot.index(token) for token in ordered_tokens],
        )

    def test_whirlwind_trail_uses_element_local_axe_follow(self) -> None:
        effect_id = "effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01"
        source_catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        source_path = (
            REPOSITORY_ROOT / "Data" / source_catalog[effect_id]["authoringPath"]
        )
        source_document = json.loads(source_path.read_text(encoding="utf-8"))
        trail = next(
            row
            for row in source_document["elements"]
            if row["id"] == "whirlwind.trail.20.axe.main"
        )
        self.assertEqual("trail", trail["kind"])
        self.assertEqual([0, 0, 0], trail["detail"]["transform"]["velocityPerSecond"])
        self.assertGreaterEqual(trail["detail"]["trail"]["minimumDistance"], 0)
        self.assertEqual(
            {
                "enabled": True,
                "follow": True,
                "sourceAnchorSlotId": "b_wp_r_01",
                "runtimeAnchorSlotId": "whirlwind.trail.20.axe.main",
                "runtimeBoneName": "b_wp_r_01",
                "snapshotRootSourceBasisYawDegrees": 0,
                "socketLocalTransform": {
                    "position": [0, 0, 0],
                    "rotationDegrees": [0, 0, 0],
                    "scale": [1, 1, 1],
                },
            },
            trail["actionCueAttachment"],
        )

        runtime_catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                RUNTIME_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        runtime_path = (
            REPOSITORY_ROOT
            / "Client/Bin/DataFiles/Effect"
            / runtime_catalog[effect_id]["authoredDocumentPath"]
        )
        runtime_bytes = runtime_path.read_bytes()
        hash_match = re.search(
            r"\.([0-9a-f]{64})\.effect\.json$", runtime_path.name
        )
        self.assertIsNotNone(hash_match)
        self.assertEqual(hash_match.group(1), hashlib.sha256(runtime_bytes).hexdigest())
        self.assertEqual(source_document, json.loads(runtime_bytes.decode("utf-8")))

        cpp_text = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        model_view = function_slice(
            cpp_text,
            "void Client::CEffect_Tool::Render_ModelViewWindow()",
            "void Client::CEffect_Tool::Render_AnimationControls(",
        )
        self.assertLess(
            model_view.index('ImGui::InputText("Socket / Bone"'),
            model_view.index("ImGui::BeginDisabled(Has_ProductCuePreview());"),
        )
        for token in (
            'ImGui::SeparatorText("Selected Trail Follow")',
            'ImGui::Button("Attach Selected Trail to Bone")',
            'ImGui::Button("Clear Selected Trail Follow")',
            "EFFECT_ELEMENT_KIND::TRAIL == pSelectedTrail->eKind",
        ):
            self.assertIn(token, model_view)

        setter = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Try_SetSelectedTrailFollowAnchor(",
            "bool_t Client::CEffect_Tool::Try_ClearSelectedTrailFollowAnchor()",
        )
        ordered_tokens = (
            "Has_UnappliedDetailDraft()",
            "EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource",
            "CAnimationTargetService::Resolve_AnchorTransform(",
            "EFFECT_ELEMENT_KIND::TRAIL != Selected->eKind",
            "Attachment.bEnabled = true;",
            "Attachment.bFollow = true;",
            "Attachment.strRuntimeAnchorSlotId = Selected->strElementId;",
            "Try_CommitDocument(std::move(Staged))",
            "Start_WorldPreviewFromBeginning();",
        )
        for token in ordered_tokens:
            self.assertIn(token, setter)
        self.assertEqual(
            sorted(setter.index(token) for token in ordered_tokens),
            [setter.index(token) for token in ordered_tokens],
        )

        history_seek = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Seek_WorldPreviewWithSourceAnchorHistory(",
            "bool_t Client::CEffect_Tool::Is_ProductCueVisible(",
        )
        for token in (
            "Collect_ToolSourceAnchorRequests(Document)",
            "CAnimationTargetService::Prepare_HistoricalPoseBinding(",
            "Resolve_EffectTimelineTime(fHistoryEffectSeconds)",
            "CAnimationTargetService::Sample_HistoricalPose(",
            "pObject->Set_SampleTimeWithTransformHistory(",
        ):
            self.assertIn(token, history_seek)
        self.assertGreaterEqual(
            cpp_text.count("Seek_WorldPreviewWithSourceAnchorHistory("), 3
        )

        renderer_text = (
            REPOSITORY_ROOT / "Client/Private/Effect_DocumentRenderer.cpp"
        ).read_text(encoding="utf-8")
        self.assertIn(
            "const bool_t bDistanceTessellated = bTypedSourceRibbon ||",
            renderer_text,
        )
        self.assertIn(
            "const f32_t U = fTilingDistance > 0.f ?",
            renderer_text,
        )
        kind_detail = function_slice(
            cpp_text,
            "void Client::CEffect_Tool::Render_KindDetail(",
            "void Client::CEffect_Tool::Render_SourceRecipeDetail(",
        )
        self.assertIn('"Trail UV Repeat Distance"', kind_detail)
        self.assertIn('"Trail Curve Step"', kind_detail)

    def test_dash_charge_project_tuned_segments_and_cues_are_exact(self) -> None:
        encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))
        pattern = next(
            row
            for row in encounter["patterns"]
            if row["patternId"] == "VALTAN_DASH_CHARGE"
        )
        self.assertEqual([420604], pattern["sourceActionIds"])
        stages = {row["stageId"]: row for row in pattern["stages"]}
        self.assertEqual(3650, stages["WINDUP"]["durationMs"])
        self.assertEqual("NONE", stages["WINDUP"]["hitShape"])
        self.assertEqual(0, stages["WINDUP"]["hitCount"])
        self.assertEqual(500, stages["CHARGE"]["durationMs"])
        self.assertEqual("BOX", stages["CHARGE"]["hitShape"])
        self.assertEqual(1, stages["CHARGE"]["hitCount"])
        self.assertEqual(
            {"kind": "FORWARD", "distance": 20.0},
            stages["CHARGE"]["motion"],
        )
        self.assertEqual(900, stages["RECOVERY"]["durationMs"])
        self.assertEqual("NONE", stages["RECOVERY"]["hitShape"])

        bindings = {
            row["actionId"]: row["clips"]
            for row in json.loads(
                PATTERN_BINDING_PATH.read_text(encoding="utf-8")
            )["bindings"]
        }
        expected_clips = {
            "valtan.attack.dash-charge.windup": [
                {
                    "clipOccurrenceId": (
                        "valtan.attack.dash-charge.windup.project-tuned."
                        "prep-repeat.clip.01"
                    ),
                    "clip": "mesh_att_battle_4_01",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 600,
                    "playRate": 1.0,
                    "loop": False,
                },
                {
                    "clipOccurrenceId": (
                        "valtan.attack.dash-charge.windup.project-tuned."
                        "prep-repeat.clip.02"
                    ),
                    "clip": "mesh_att_battle_4_01",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 600,
                    "playRate": 1.0,
                    "loop": False,
                },
                {
                    "clipOccurrenceId": "valtan.attack.dash-charge.windup.clip.01",
                    "clip": "mesh_att_battle_4_01",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 2450,
                    "playRate": 1.0,
                    "loop": False,
                },
            ],
            "valtan.attack.dash-charge.active": [
                {
                    "clipOccurrenceId": "valtan.attack.dash-charge.active.clip.01",
                    "clip": "mesh_att_battle_4_01",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 2450,
                    "playMs": 900,
                    "playRate": 1.8,
                    "loop": False,
                }
            ],
            "valtan.attack.dash-charge.recovery": [
                {
                    "clipOccurrenceId": (
                        "valtan.attack.dash-charge.recovery.project-tuned.clip.01"
                    ),
                    "clip": "mesh_att_battle_4_01",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 3350,
                    "playMs": 1383,
                    "playRate": 1.5366667,
                    "loop": False,
                }
            ],
        }
        for action_id, expected in expected_clips.items():
            self.assertEqual(expected, bindings[action_id])

        cues = {
            row["bindingId"]: row
            for row in json.loads(CUE_PATH.read_text(encoding="utf-8"))["cues"]
        }
        cue_contracts = {
            "cue.valtan.project-tuned.attack.dash-charge.windup-telegraph": {
                "stageId": "WINDUP",
                "actionId": "valtan.attack.dash-charge.windup",
                "clipOccurrenceId": "valtan.attack.dash-charge.windup.clip.01",
                "effectAssetId": (
                    "effect.valtan.project-tuned.dash-charge.windup-telegraph"
                ),
                "followPolicy": "snapshot",
                "sourceStartMs": 559,
                "sourceEndMs": 2364,
            },
            "cue.valtan.project-tuned.attack.dash-charge.active-shield": {
                "stageId": "CHARGE",
                "actionId": "valtan.attack.dash-charge.active",
                "clipOccurrenceId": "valtan.attack.dash-charge.active.clip.01",
                "effectAssetId": (
                    "effect.valtan.project-tuned.dash-charge.active-shield"
                ),
                "followPolicy": "follow",
                "sourceStartMs": 2450,
                "sourceEndMs": 3350,
            },
        }
        for binding_id, expected in cue_contracts.items():
            cue = cues[binding_id]
            self.assertEqual("VALTAN_DASH_CHARGE", cue["patternId"])
            for field, value in expected.items():
                self.assertEqual(value, cue[field], (binding_id, field))
            self.assertEqual("root", cue["anchorSlotId"])
            self.assertEqual("cue_end", cue["stopPolicy"])
            self.assertEqual("once", cue["repeatPolicy"])

        catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        runtime_catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                RUNTIME_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        windup_id = "effect.valtan.project-tuned.dash-charge.windup-telegraph"
        active_id = "effect.valtan.project-tuned.dash-charge.active-shield"
        windup = json.loads(
            (REPOSITORY_ROOT / "Data" / catalog[windup_id]["authoringPath"])
            .read_text(encoding="utf-8")
        )
        active = json.loads(
            (REPOSITORY_ROOT / "Data" / catalog[active_id]["authoringPath"])
            .read_text(encoding="utf-8")
        )
        self.assertEqual(windup_id, windup["effectAssetId"])
        self.assertEqual([], windup["modelCues"])
        self.assertEqual(1, len(windup["elements"]))
        self.assertEqual("dash-charge-red-floor", windup["elements"][0]["id"])
        self.assertEqual("particle", windup["elements"][0]["kind"])
        self.assertEqual(
            "Effect/Valtan/Textures/EFMASTER_MATERIAL_PROLOGUE/diffuse.dds",
            next(
                row["assetId"]
                for row in windup["elements"][0]["resources"]
                if row["slotId"] == "base"
            ),
        )
        self.assertEqual(
            "VALTAN_DASH_CHARGE / WINDUP / Red Telegraph",
            windup["displayName"],
        )

        self.assertEqual(active_id, active["effectAssetId"])
        self.assertEqual([], active["modelCues"])
        self.assertEqual(1, len(active["elements"]))
        self.assertEqual("dash-charge-front-aura", active["elements"][0]["id"])
        self.assertEqual("particle", active["elements"][0]["kind"])
        self.assertEqual(
            "Effect/Valtan/Textures/FX_TEX_05/fx_m_trail_001_cl.dds",
            next(
                row["assetId"]
                for row in active["elements"][0]["resources"]
                if row["slotId"] == "base"
            ),
        )
        self.assertEqual(
            "VALTAN_DASH_CHARGE / CHARGE / Front Shield",
            active["displayName"],
        )

        for effect_id, source_document in (
            (windup_id, windup),
            (active_id, active),
        ):
            runtime_entry = runtime_catalog[effect_id]
            self.assertEqual(
                "DIRECT_AUTHORED_DOCUMENT_V13",
                runtime_entry["payloadKind"],
            )
            runtime_path = (
                REPOSITORY_ROOT
                / "Client/Bin/DataFiles/Effect"
                / runtime_entry["authoredDocumentPath"]
            )
            runtime_bytes = runtime_path.read_bytes()
            hash_match = re.search(
                r"\.([0-9a-f]{64})\.effect\.json$", runtime_path.name
            )
            self.assertIsNotNone(hash_match, effect_id)
            self.assertEqual(
                hash_match.group(1), hashlib.sha256(runtime_bytes).hexdigest()
            )
            self.assertEqual(
                source_document,
                json.loads(runtime_bytes.decode("utf-8")),
                effect_id,
            )

    def test_live_data_projects_every_owned_saved_document_once_per_pattern(self) -> None:
        projected, raw_links = project_saved_rows()
        self.assertEqual(34, len(projected))
        self.assertEqual(54, raw_links)
        self.assertEqual(54, sum(len(rows) for rows in projected.values()))
        self.assertEqual(2, len(projected["VALTAN_DASH_CHARGE"]))
        self.assertEqual(1, len(projected["VALTAN_FRONT_BACK_FRONT"]))
        self.assertNotIn("VALTAN_FOUR_SLASH", projected)
        self.assertEqual(1, len(projected["VALTAN_TRIPLE_SLASH"]))
        self.assertEqual(2, len(projected["VALTAN_ROTATION_SLASH"]))
        self.assertEqual(
            "effect.valtan.carrier-v1.attack.four-slash.active.clip-01",
            projected["VALTAN_TRIPLE_SLASH"][0],
        )
        self.assertEqual(
            "effect.valtan.carrier-v1.attack.four-slash.active.clip-02",
            projected["VALTAN_ROTATION_SLASH"][0],
        )
        self.assertEqual(4, len(projected["VALTAN_WHIRLWIND"]))
        self.assertEqual(
            len(projected["VALTAN_WHIRLWIND"]),
            len(set(projected["VALTAN_WHIRLWIND"])),
        )

    def test_base_saved_projection_inventory_requires_explicit_migration(self) -> None:
        # Reference-only shells remain a source inventory even though the active
        # Saved Unified Effects surface intentionally hides them.
        projected, raw_links = project_saved_rows(
            include_v1_aliases=False,
            include_reference_only=True,
        )
        self.assertEqual(108, raw_links)
        self.assertEqual(107, sum(len(rows) for rows in projected.values()))
        catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        nonempty_candidates = 0
        empty_shells = 0
        for effect_ids in projected.values():
            for effect_id in effect_ids:
                entry = catalog.get(effect_id)
                path = (
                    REPOSITORY_ROOT / "Data" / entry["authoringPath"]
                    if entry is not None
                    else AUTHORED_ROOT / f"{effect_id}.effect.json"
                )
                document = json.loads(path.read_text(encoding="utf-8"))
                if document.get("elements") or document.get("modelCues"):
                    nonempty_candidates += 1
                else:
                    empty_shells += 1
        self.assertEqual(51, nonempty_candidates)
        self.assertEqual(56, empty_shells)

    def test_all_declared_v0_product_cues_remain_published_and_nonempty(self) -> None:
        cues = json.loads(CUE_PATH.read_text(encoding="utf-8"))["cues"]
        source_catalog = json.loads(
            SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
        )["effects"]
        runtime_catalog = json.loads(
            RUNTIME_CATALOG_PATH.read_text(encoding="utf-8")
        )["effects"]
        project_tuned_ids = {
            "effect.valtan.project-tuned.dash-charge.active-shield",
            "effect.valtan.project-tuned.dash-charge.windup-telegraph",
        }
        self.assertEqual(
            project_tuned_ids,
            {
                cue["effectAssetId"]
                for cue in cues
                if cue["effectAssetId"].startswith("effect.valtan.project-tuned.")
            },
        )
        sealed_cues = [
            cue for cue in cues if cue["effectAssetId"] not in project_tuned_ids
        ]
        effect_ids = [cue["effectAssetId"] for cue in sealed_cues]
        self.assertEqual(44, len(effect_ids))
        self.assertEqual(44, len(set(effect_ids)))
        visible_elements = 0
        hidden_elements = 0
        model_cues = 0
        for effect_id in effect_ids:
            source_matches = [
                row for row in source_catalog if row["effectAssetId"] == effect_id
            ]
            runtime_matches = [
                row for row in runtime_catalog if row["effectAssetId"] == effect_id
            ]
            self.assertEqual(1, len(source_matches), effect_id)
            self.assertEqual(1, len(runtime_matches), effect_id)
            source_entry = source_matches[0]
            source_path = REPOSITORY_ROOT / "Data" / source_entry["authoringPath"]
            source_document = json.loads(source_path.read_text(encoding="utf-8"))
            self.assertEqual(effect_id, source_document["effectAssetId"])
            self.assertTrue(
                source_document.get("elements") or source_document.get("modelCues"),
                effect_id,
            )
            visible_elements += sum(
                1 for element in source_document.get("elements", [])
                if element.get("visible", True)
            )
            hidden_elements += sum(
                1 for element in source_document.get("elements", [])
                if not element.get("visible", True)
            )
            model_cues += len(source_document.get("modelCues", []))
            self.assertTrue(
                any(
                    element.get("visible", True)
                    for element in source_document.get("elements", [])
                ) or source_document.get("modelCues"),
                effect_id,
            )
            runtime_entry = runtime_matches[0]
            runtime_path = (
                REPOSITORY_ROOT
                / "Client/Bin/DataFiles/Effect"
                / runtime_entry["authoredDocumentPath"]
            )
            self.assertTrue(runtime_path.is_file(), effect_id)
            runtime_bytes = runtime_path.read_bytes()
            hash_match = re.search(
                r"\.([0-9a-f]{64})\.effect\.json$", runtime_path.name
            )
            self.assertIsNotNone(hash_match, effect_id)
            self.assertEqual(
                hash_match.group(1), hashlib.sha256(runtime_bytes).hexdigest()
            )
            runtime_document = json.loads(runtime_bytes.decode("utf-8"))
            self.assertEqual(source_document, runtime_document, effect_id)
        # clip-01 intentionally preserves the latest Effect Tool authoring
        # snapshot (six user-visible rows) instead of restoring the retired
        # twelve-row materializer preimage.
        self.assertEqual(634, visible_elements)
        self.assertEqual(5, hidden_elements)
        self.assertEqual(0, model_cues)

    def test_v1_alias_sidecar_is_exactly_six_valid_pairs(self) -> None:
        aliases = json.loads(V1_ALIAS_PATH.read_text(encoding="utf-8"))["aliases"]
        cue_ids = {
            cue["effectAssetId"]
            for cue in json.loads(CUE_PATH.read_text(encoding="utf-8"))["cues"]
        }
        source_catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        runtime_ids = {
            row["effectAssetId"]
            for row in json.loads(
                RUNTIME_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        self.assertEqual(6, len(aliases))
        self.assertEqual(6, len({row["effectAssetId"] for row in aliases}))
        self.assertEqual(6, len({row["v1EffectAssetId"] for row in aliases}))
        for row in aliases:
            self.assertIn(row["effectAssetId"], cue_ids)
            self.assertIn(row["effectAssetId"], source_catalog)
            self.assertIn(row["v1EffectAssetId"], source_catalog)
            self.assertIn(row["v1EffectAssetId"], runtime_ids)
            self.assertTrue(row["v1EffectAssetId"].endswith(".v1.unified"))
            v1_path = (
                REPOSITORY_ROOT
                / "Data"
                / source_catalog[row["v1EffectAssetId"]]["authoringPath"]
            )
            v1_document = json.loads(v1_path.read_text(encoding="utf-8"))
            self.assertTrue(
                v1_document.get("elements") or v1_document.get("modelCues"),
                row["v1EffectAssetId"],
            )

    def test_ui_contract_mutations_fail_closed(self) -> None:
        cpp_text = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        header_text = EFFECT_TOOL_HEADER.read_text(encoding="utf-8")
        mutations = (
            cpp_text.replace("Clip.ProductCues", "Clip.RemovedCues"),
            cpp_text.replace("Stage.Effects", "Stage.RemovedEffects"),
            cpp_text.replace("Open Saved Effect", "Open Effect"),
            cpp_text.replace(
                "Try_PlayValtanSavedUnifiedEffect",
                "Try_PlaySavedUnifiedEffect",
            ),
        )
        for mutation in mutations:
            with self.subTest(mutation=mutations.index(mutation)):
                with self.assertRaises(AssertionError):
                    validate_source_contract(mutation, header_text)

    def test_v1_and_drawable_gate_mutations_fail_closed(self) -> None:
        cpp_text = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        tree_cpp_text = VALTAN_PATTERN_TREE_CPP.read_text(encoding="utf-8")
        tree_header_text = VALTAN_PATTERN_TREE_HEADER.read_text(encoding="utf-8")
        v1_mutations = (
            cpp_text.replace(
                "ResolveRow(Cue.strV1EffectAssetId)",
                "ResolveRow(Cue.strEffectAssetId)",
            ),
            cpp_text.replace(
                "PlaybackCue.strEffectAssetId = Row.strEffectAssetId",
                "PlaybackCue.strEffectAssetId = pProductSource->pCue->strEffectAssetId",
            ),
        )
        for mutation in v1_mutations:
            with self.assertRaises(AssertionError):
                validate_v1_alias_projection_contract(
                    mutation, tree_cpp_text, tree_header_text
                )
        with self.assertRaises(AssertionError):
            validate_drawable_preflight_contract(
                cpp_text.replace("!Cache.bDrawable", "false", 1)
            )
        with self.assertRaises(AssertionError):
            validate_drawable_preflight_contract(
                cpp_text.replace("RefreshedCache", "ObservedCache")
            )


if __name__ == "__main__":
    unittest.main()
