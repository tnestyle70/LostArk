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
DIRECT_AUTHORED_INDEX_CPP = (
    REPOSITORY_ROOT / "Client/Private/Effect_DirectAuthoredSourceIndex.cpp"
)
DIRECT_AUTHORED_INDEX_HEADER = (
    REPOSITORY_ROOT / "Client/Public/Effect_DirectAuthoredSourceIndex.h"
)
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
VALTAN_MASTER_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.pattern.json"
VALTAN_PRESENTATION_PATH = (
    REPOSITORY_ROOT / "Data/Valtan/Valtan.presentation.json"
)
AUTHORED_ROOT = REPOSITORY_ROOT / "Data/Effects/Authored"
SOURCE_CATALOG_PATH = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
INTENTIONAL_EMPTY_PRODUCT_SHELLS = frozenset()
RECOVERED_VALTAN_EFFECT_ELEMENT_IDS = {
    "effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01": frozenset({
        "donut.telegraph.outer.red",
        "sprite_particle_6",
        "donut.telegraph.inner.grow",
        "donut.impact.wave.black",
        "authored.copy.donut.impact.wave.black.1",
    }),
    "effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01": frozenset({
        "source.f4617c98d44349eec51d",
    }),
    "effect.valtan.carrier-v1.attack.high-jump.land.clip-01": frozenset({
        "source.7b7b7c81b12e9dd59483",
        "high-jump-landing-wave",
    }),
    "effect.valtan.sky-axe.active": frozenset({
        "mesh.valtan.sky-axe.descent",
        "particle.valtan.sky-axe.impact",
        "sky-axe-target-inner-fill",
        "authored.copy.sky-axe-target-inner-fill.1",
        "sky-axe-flight-line",
        "authored.copy.mesh_particle_6.1",
        "sprite_particle_7",
    }),
    "effect.valtan.floor-wipe-130": frozenset({
        "authored.copy.authored.copy.sprite_particle_8.1.1",
        "authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.1",
        "authored.copy.authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.1.1",
        "authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.1.1.1",
        "authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.1.1.1.1",
        "authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.1.1.1..1",
        "authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.2",
        "authored.copy.authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.2.1",
        "authored.copy.authored.copy.donut.impact.wave.black.1.1",
        "authored.copy.authored.copy.authored.copy.donut.impact.wave.black.1.1.1",
        "authored.copy.authored.copy.authored.copy.authored.copy.donut.impact.wave.black.1.1.1.1",
        "authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.donut.impact.wave.black.1.1.1.1.1",
        "authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.donut.impact.wave.black.1.1.1.1.1.1",
        "authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.donut.impact.wave.black.1.1..1",
        "authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.authored.copy.donut.impact.w.1",
        "authored.copy.authored.copy.authored.copy.donut.impact.wave.black.1.1.2",
        "sprite_particle_5",
    }),
}
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
        "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
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
        "m_ValtanPatternTree.IndependentEffects",
        "Render_ValtanIndependentEffectNode(Effect, strSearch)",
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
        'ImGui::SeparatorText("Independent Effect References")',
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
        'ImGui::SmallButton("Open Editor")',
        'ImGui::SmallButton("Play Saved Effect")',
        "Try_OpenValtanAuthoredEffect",
        "Try_PlayValtanSavedUnifiedEffect",
        "Try_OpenValtanSavedReferenceEffect",
        "pNonProductStage->ClipOccurrences",
        "Effect.strOwnerPatternId == Pattern.strPatternId",
        "Effect.strEffectAssetId == Row.strEffectAssetId",
        '"[REFERENCE] %s | stages %s | %s"',
    )
    for token in required_pattern:
        if token not in pattern:
            raise AssertionError(f"Valtan Saved row contract lost: {token}")
    if "Try_PlaySavedUnifiedEffect(" in pattern:
        raise AssertionError("Valtan must not use the Player-only saved play path")


def validate_editor_admission_isolation_contract(
    effect_tool_cpp: str, index_cpp: str, index_header: str
) -> None:
    refresh = function_slice(
        effect_tool_cpp,
        "bool_t Client::CEffect_Tool::Refresh_DirectAuthoredEditableIndex(",
        "const std::filesystem::path*\nClient::CEffect_Tool::Resolve_DirectAuthoredEditablePath(",
    )
    required_refresh = (
        "Player Product joins were isolated while exact authored documents remained editor-eligible",
        "Valtan saved rows were isolated while Player saved rows remained available",
        "SourceIndex.iOwnerJoinUnavailableCount",
        "without removing Open Editor",
        "m_DirectAuthoredEditableEntries = std::move(StagedEntries)",
        "stable player source identity is sufficient to keep the",
        "Binding.bProductOwnerJoined",
        "Saved 0",
    )
    for token in required_refresh:
        if token not in refresh:
            raise AssertionError(f"editor admission isolation lost: {token}")
    if re.search(
        r"if \(!Ensure_PlayerSkillCatalog\([^)]*\)\)\s*\{\s*return PreservePrevious",
        refresh,
        flags=re.DOTALL,
    ):
        raise AssertionError(
            "Player Product-owner loading must not globally reject exact authored sources"
        )
    boss_failure = refresh.index(
        "Valtan saved rows were isolated while Player saved rows remained available"
    )
    if "return PreservePrevious" in refresh[boss_failure : boss_failure + 300]:
        raise AssertionError(
            "Valtan Product-owner loading must isolate its rows instead of replacing the editor index"
        )

    resolve = function_slice(
        effect_tool_cpp,
        "const std::filesystem::path*\nClient::CEffect_Tool::Resolve_DirectAuthoredEditablePath(",
        "bool_t Client::CEffect_Tool::Is_UnifiedEffectActive(",
    )
    for token in (
        "CEffectDocumentCodec::Load(",
        "Document.strEffectAssetId == strEffectAssetId",
        "DIRECT_AUTHORED_DOCUMENT source path",
        "Validated the writable Data/Effects/Authored document",
    ):
        if token not in resolve:
            raise AssertionError(
                f"version-neutral direct-authored Play gate lost: {token}"
            )
    for token in (
        "DIRECT_AUTHORED_DOCUMENT_V13",
        "Document.iLoadedFormatVersion",
        "version 13 document",
    ):
        if token in resolve:
            raise AssertionError(
                f"All Effects duplicated the codec version contract: {token}"
            )

    play = function_slice(
        effect_tool_cpp,
        "bool_t Client::CEffect_Tool::Try_PlayUnifiedEffect(",
        "bool_t Client::CEffect_Tool::Try_PlaySavedUnifiedEffect(",
    )
    for token in (
        "Resolve_DirectAuthoredEditablePath(",
        "if (nullptr == pEditablePath)",
    ):
        if token not in play:
            raise AssertionError(
                f"unified Play lost its direct-authored identity gate: {token}"
            )

    valtan_play = function_slice(
        effect_tool_cpp,
        "bool_t Client::CEffect_Tool::Try_PlayValtanSavedUnifiedEffect(",
        "bool_t Client::CEffect_Tool::Try_SnapshotValtanWorldPreviewRoot(",
    )
    if "return bTargetReady && Try_PlayUnifiedEffect(Cache);" not in valtan_play:
        raise AssertionError(
            "Valtan Play no longer reaches the version-neutral unified Play gate"
        )

    for token in (
        "iOwnerJoinUnavailableCount",
        "strFirstOwnerJoinUnavailable",
        "owner failure leaves eOwnerKind == END",
    ):
        if token not in index_header:
            raise AssertionError(f"typed editor/owner split contract lost: {token}")
    for message in (
        "direct authored Effect ID has no PlayerSkills owner",
        "direct authored Effect ID has ambiguous boss-pattern and boss-combat-object owners",
        "direct authored Effect ID has no stable player-skill, boss-pattern, or boss-combat-object owner",
        "registry-bound audition source owner mismatch",
    ):
        position = index_cpp.index(message)
        prefix = index_cpp[max(0, position - 80) : position]
        if "RecordOwnerJoinUnavailable(" not in prefix:
            raise AssertionError(
                f"Product-owner failure was promoted back into editor admission: {message}"
            )

    catalog_rows = index_cpp[
        index_cpp.index(
            "for (const DATA_JSON_VALUE& CatalogEntry : pEffects->Get_Array())",
            index_cpp.index("RecordOwnerJoinUnavailable"),
        ) : index_cpp.index("std::sort(Staged.Entries.begin()")
    ]
    if "return false;" in catalog_rows:
        raise AssertionError(
            "one malformed direct-authored catalog row must not reject every editor-ready document"
        )
    for token in (
        'RecordUnavailable("EffectCatalog.json contains a non-object row.")',
        "EffectCatalog.json contains a malformed direct-authored row.",
        "DuplicateAssetIds.emplace(strAssetId)",
        "std::erase_if(Staged.Entries",
    ):
        if token not in index_cpp:
            raise AssertionError(
                f"direct-authored row isolation contract lost: {token}"
            )


def validate_v1_alias_projection_contract(
    cpp_text: str, tree_cpp_text: str, tree_header_text: str
) -> None:
    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
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
    for token in (
        '"effect.valtan.pattern.420633.active.v1.unified"',
        "Is_ValtanExactHistoryPreviewEffectAssetId(",
        "Matches_ValtanExactHistoryBinding(",
        "Seek_ValtanBossPatternTransformHistory(",
    ):
        if token not in cpp_text:
            raise AssertionError(
                f"V1 exact-history preview contract lost: {token}"
            )
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

    reference_refresh = reference_helper.find("Refresh_UnifiedEffectCache(")
    reference_valid = reference_helper.find("!Cache.bValid")
    reference_play_drawable = reference_helper.find(
        "bQueuePlayCompleteAfterLoad && !Cache.bDrawable"
    )
    reference_commit = reference_helper.find("Try_LoadDocumentPath(")
    if min(
        reference_refresh,
        reference_valid,
        reference_play_drawable,
        reference_commit,
    ) < 0 or not (
        reference_refresh
        < reference_valid
        < reference_play_drawable
        < reference_commit
    ):
        raise AssertionError(
            "reference Open must admit structural partial documents while Play alone requires drawable readiness"
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
        "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
    )
    open_button = pattern.find('ImGui::SmallButton("Open Editor")')
    play_button = pattern.find('ImGui::SmallButton(pRuntimePlayLabel)')
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
    forbidden_open_gates = (
        "bKnownNonDrawable",
        "bKnownInvalid",
        "bHasExactPlaybackOwner",
        "bAmbiguousOccurrence",
    )
    if any(token in open_guard for token in forbidden_open_gates):
        raise AssertionError(
            "Open Editor must depend only on the exact source path, not Product or preview readiness"
        )
    open_action = pattern[open_button:play_button]
    if not all(
        token in open_action
        for token in ("ProductPlaybackPreview.has_value()", "Try_LoadDocumentPath(")
    ):
        raise AssertionError(
            "Open Editor must preserve exact Product context when available and fall back to source-only editing"
        )
    if not all(
        token in play_guard for token in ("bKnownInvalid", "bKnownNonDrawable")
    ):
        raise AssertionError(
            "invalid and non-drawable Product documents must remain play-locked"
        )

    independent = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )
    independent_open = independent.find('ImGui::SmallButton("Open Editor")')
    independent_play = independent.find(
        "ImGui::SmallButton(pIndependentPlayLabel)"
    )
    if min(independent_open, independent_play) < 0 or not (
        independent_open < independent_play
    ):
        raise AssertionError("independent exact source lost its Open Editor entry")
    independent_guard = independent[
        independent.rfind("ImGui::BeginDisabled(", 0, independent_open):
        independent_open
    ]
    for forbidden in ("bIndependentTimelineReady", "bKnownInvalid", "bKnownNonDrawable"):
        if forbidden in independent_guard:
            raise AssertionError(
                f"independent Open Editor was coupled back to Play state: {forbidden}"
            )
    independent_action = independent[independent_open:independent_play]
    if "Try_LoadDocumentPath(" not in independent_action:
        raise AssertionError(
            "independent Open Editor must fall back to the exact source without an owner timeline"
        )
    for token in (
        '"Play Effect" : "Play Effect + Owner Animation"',
        'ImGui::SmallButton("Play Server Owner")',
        "bCanPlayServerOwner",
        "Try_PlayValtanServerPattern(*pOwnerPattern)",
    ):
        if token not in independent:
            raise AssertionError(
                f"independent local/server playback distinction lost: {token}"
            )

    all_effects = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_AllEffectsWindow()",
        "void Client::CEffect_Tool::Render_LoadedEffectContents()",
    )
    product_identity = all_effects.find(
        "const std::string strUnifiedCandidateId ="
    )
    product_resolve = all_effects.find(
        "Resolve_DirectAuthoredEditablePath(", product_identity
    )
    product_open = all_effects.find(
        'ImGui::SmallButton("Open Editor")', product_resolve
    )
    product_preview = all_effects.find(
        "Refresh_UnifiedEffectCache(", product_open
    )
    if min(product_identity, product_resolve, product_open, product_preview) < 0 or not (
        product_identity < product_resolve < product_open < product_preview
    ):
        raise AssertionError(
            "Product cue Open Editor must resolve the exact source before any Product cache/preview gate"
        )
    for token in (
        "EDITOR-ONLY EXACT SOURCES",
        "PlayerSkills Product ownership is unavailable",
        "Product preview tree unavailable",
    ):
        if token not in all_effects:
            raise AssertionError(
                f"owner-less exact source discoverability contract lost: {token}"
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
        "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
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
        "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
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


def validate_full_valtan_product_timeline_contract(
    cpp_text: str, header_text: str
) -> None:
    for token in (
        "TimelineClips",
        "iTimelineDurationMs",
        "iOwningStageTimelineOffsetMs",
        "iOwningClipTimelineOffsetMs",
        "ValtanProductPreview",
    ):
        if token not in header_text:
            raise AssertionError(f"full Valtan Product preview state lost: {token}")

    play = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Play_ValtanProductCue(",
        "bool_t Client::CEffect_Tool::Play_ValtanStageSequence(",
    )
    for token in (
        "Play_ValtanStageSequence(SourcePreview.TimelineClips)",
        "SourcePreview.iOwningStageTimelineOffsetMs",
        "SourcePreview.iOwningClipTimelineOffsetMs",
        "SourcePreview.iTimelineDurationMs",
        "non-unique or incomplete owner timeline",
    ):
        if token not in play:
            raise AssertionError(f"full Valtan Product playback lost: {token}")
    if "Play_ValtanClipOccurrence(Clip)" in play:
        raise AssertionError("managed Product playback regressed to its owner clip only")

    sample = function_slice(
        cpp_text,
        "f32_t Client::CEffect_Tool::Resolve_EffectSampleTime(",
        "f32_t Client::CEffect_Tool::Resolve_EffectTimelineTime(",
    )
    inverse = function_slice(
        cpp_text,
        "f32_t Client::CEffect_Tool::Resolve_EffectTimelineTime(",
        "bool_t Client::CEffect_Tool::Seek_WorldPreviewWithSourceAnchorHistory(",
    )
    visible = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Is_ProductCueVisible(",
        "bool_t Client::CEffect_Tool::Restore_ValtanProductPreviewPlayback(",
    )
    if "fTimelineSeconds - fClipTimelineOffsetSeconds" not in sample:
        raise AssertionError("Valtan cue sample clock lost its global clip offset")
    if "fClipTimelineOffsetSeconds + Sample.fCueWallStartSeconds" not in inverse:
        raise AssertionError("Valtan effect-to-timeline inverse lost its global offset")
    for token in (
        "iOwningStageTimelineOffsetMs",
        "iOwningClipTimelineOffsetMs",
        "fTimelineSeconds - fClipStartMs * 0.001f",
    ):
        if token not in visible:
            raise AssertionError(f"global cue visibility window lost: {token}")

    authoring = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Play_ValtanAuthoringTimeline(",
        "void Client::CEffect_Tool::Render_ValtanStageRow(",
    )
    active_branch = authoring.index("if (1u == ActiveCueOwners.size())")
    explicit_clear = authoring.index("Clear_ProductCuePreview();")
    if explicit_clear < active_branch:
        raise AssertionError(
            "Play Authoring Timeline must try the active exact cue before clearing it"
        )
    for token in (
        "Build_ValtanProductPreview",
        "Try_PlayActiveUnifiedEffect()",
        "no exact cue for the current authored Effect",
        "maps to multiple cues; select one saved occurrence",
    ):
        if token not in authoring:
            raise AssertionError(f"active authoring cue contract lost: {token}")

    pattern_node = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
    )
    ambiguity_contract = re.compile(
        r"const bool_t bAmbiguousOccurrence\s*=\s*"
        r"1u < Row\.ProductSources\.size\(\)\s*\|\|\s*"
        r"\(!Row\.ProductSources\.empty\(\)\s*&&\s*"
        r"!NonProductStages\.empty\(\)\)\s*\|\|\s*"
        r"\(Row\.ProductSources\.empty\(\)\s*&&\s*"
        r"1u < NonProductStages\.size\(\)\)\s*;",
        flags=re.DOTALL,
    )
    if ambiguity_contract.search(pattern_node) is None:
        raise AssertionError(
            "saved Valtan owner ambiguity must cover Product duplicates, "
            "mixed Product/non-Product provenance, and non-Product duplicates"
        )
    if pattern_node.count("bAmbiguousOccurrence ||") != 1:
        raise AssertionError(
            "only Product Play may reject ambiguous owners; Open Editor is source-owned"
        )

    legacy_start = pattern_node.index(
        "if (nullptr != pProductSource)"
    )
    legacy_end = pattern_node.index(
        "const VALTAN_STAGE_VIEW* pNonProductStage", legacy_start
    )
    legacy_product = pattern_node[legacy_start:legacy_end]
    for token in (
        "VALTAN_PRODUCT_EFFECT_CUE_VIEW PlaybackCue =",
        "*pProductSource->pCue",
        "if (Pattern.bAuthoringMasterManaged &&",
        "Build_ValtanProductPreview(Pattern, eProductPath",
        "else if (!Pattern.bAuthoringMasterManaged)",
        "SourceCue.strEffectAssetId == Row.strEffectAssetId",
        "SourceCue.strV1EffectAssetId == Row.strEffectAssetId",
        "OwnerStage.iDurationMs != SourceCue.iStageDurationMs",
        "Preview.Cue = std::move(PlaybackCue)",
        "TimelineClip.iAuthoringWallMs =",
        "Preview.Cue.iStageDurationMs",
        "Preview.iTimelineDurationMs =",
        "ProductPlaybackPreview = std::move(Preview)",
    ):
        if token not in legacy_product:
            raise AssertionError(
                f"legacy exact stage-local Product preview lost: {token}"
            )
    mismatch = legacy_product.index("if (!bExactEffectIdentity")
    fallback_commit = legacy_product.index(
        "ProductPlaybackPreview = std::move(Preview)", mismatch
    )
    success_else = legacy_product.index("else", mismatch)
    if fallback_commit < success_else:
        raise AssertionError(
            "legacy Product mismatch must leave the optional preview unset"
        )

    independent = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )
    for token in (
        '"SERVER_PATTERN_STAGE" == Effect.strOwnership',
        '"SERVER_COMBAT_OBJECT" == Effect.strOwnership',
        "Effect.bHasCueProjection",
        "Effect.strEffectCueBindingId",
        "Effect.strCueClipOccurrenceId",
        "Effect.strCueMappingBasis",
        "PatternStagePreview",
        "Try_PlayValtanSavedUnifiedEffect",
        "Try_OpenValtanSavedReferenceEffect",
    ):
        if token not in independent:
            raise AssertionError(f"independent owner playback split lost: {token}")


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


def derive_authoritative_saved_link_closure(
    include_v1_aliases: bool = True,
    include_reference_only: bool = False,
) -> tuple[dict[str, list[str]], int]:
    """Build the expected UI closure directly from the canonical documents.

    This deliberately does not call ``project_saved_rows``.  Besides deriving
    the expected count from the current Product graph, it rejects orphaned or
    duplicate identities that a stale aggregate count could not explain.
    """
    encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))
    cue_document = json.loads(CUE_PATH.read_text(encoding="utf-8"))
    reference_document = json.loads(REFERENCE_PATH.read_text(encoding="utf-8"))
    boss_catalog = json.loads(BOSS_CATALOG_PATH.read_text(encoding="utf-8"))
    alias_document = json.loads(V1_ALIAS_PATH.read_text(encoding="utf-8"))

    patterns = encounter["patterns"]
    pattern_ids = [pattern["patternId"] for pattern in patterns]
    if len(pattern_ids) != len(set(pattern_ids)):
        raise AssertionError("Valtan encounter has duplicate patternId rows")

    stage_owner_by_key: dict[tuple[str, str, str], str] = {}
    for pattern in patterns:
        for stage in pattern["stages"]:
            key = (pattern["patternId"], stage["stageId"], stage["actionId"])
            if key in stage_owner_by_key:
                raise AssertionError(f"duplicate Valtan stage identity: {key}")
            stage_owner_by_key[key] = pattern["patternId"]

    cues = cue_document["cues"]
    cue_binding_ids = [cue["bindingId"] for cue in cues]
    if len(cue_binding_ids) != len(set(cue_binding_ids)):
        raise AssertionError("Valtan Product cues have duplicate bindingId rows")
    cues_by_stage: dict[tuple[str, str, str], list[str]] = {}
    for cue in cues:
        key = (cue["patternId"], cue["stageId"], cue["actionId"])
        if key not in stage_owner_by_key:
            raise AssertionError(f"orphaned Valtan Product cue: {cue['bindingId']}")
        cues_by_stage.setdefault(key, []).append(cue["effectAssetId"])

    alias_rows = alias_document["aliases"]
    alias_source_ids = [row["effectAssetId"] for row in alias_rows]
    alias_target_ids = [row["v1EffectAssetId"] for row in alias_rows]
    if len(alias_source_ids) != len(set(alias_source_ids)):
        raise AssertionError("Valtan v1 aliases have duplicate source identities")
    if len(alias_target_ids) != len(set(alias_target_ids)):
        raise AssertionError("Valtan v1 aliases have duplicate target identities")
    v1_aliases = {
        row["effectAssetId"]: row["v1EffectAssetId"]
        for row in alias_rows
    }

    reference_rows = reference_document["bindings"]
    reference_actions = [row["actionId"] for row in reference_rows]
    if len(reference_actions) != len(set(reference_actions)):
        raise AssertionError("Valtan reference bindings have duplicate actions")
    references_by_action = {
        row["actionId"]: row["effectAssetId"]
        for row in reference_rows
    }

    valtan = next(
        boss
        for boss in boss_catalog["bosses"]
        if boss["archetypeId"] == "BOSS_VALTAN"
    )
    combat_rows = valtan["combatObjectVisuals"]
    combat_archetype_ids = [
        row["combatObjectArchetypeId"] for row in combat_rows
    ]
    if len(combat_archetype_ids) != len(set(combat_archetype_ids)):
        raise AssertionError("Valtan combat visuals have duplicate archetypes")
    combat_visuals = {
        row["combatObjectArchetypeId"]: row["effectAssetId"]
        for row in combat_rows
    }

    expected: dict[str, list[str]] = {}
    raw_link_count = 0
    for pattern in patterns:
        occurrences: list[str] = []
        for stage in pattern["stages"]:
            key = (pattern["patternId"], stage["stageId"], stage["actionId"])
            product_ids = cues_by_stage.get(key, [])
            for effect_id in product_ids:
                occurrences.append(effect_id)
                if include_v1_aliases and effect_id in v1_aliases:
                    occurrences.append(v1_aliases[effect_id])

            if include_reference_only and stage["actionId"] in references_by_action:
                occurrences.append(references_by_action[stage["actionId"]])

            combat_ids = [
                combat_visuals[action["targetId"]]
                for action in stage.get("actions", [])
                if action.get("kind") in {
                    "SPAWN_COMBAT_OBJECT",
                    "SPAWN_COMBAT_OBJECT_VOLLEY",
                }
                and action.get("targetId") in combat_visuals
            ]
            if include_reference_only and not product_ids and not combat_ids:
                candidate = stage_effect_asset_id(pattern, stage)
                if (AUTHORED_ROOT / f"{candidate}.effect.json").is_file():
                    occurrences.append(candidate)
            occurrences.extend(combat_ids)

        raw_link_count += len(occurrences)
        expected[pattern["patternId"]] = list(dict.fromkeys(occurrences))
    return expected, raw_link_count


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
                if action.get("kind") not in {
                    "SPAWN_COMBAT_OBJECT",
                    "SPAWN_COMBAT_OBJECT_VOLLEY",
                }:
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
    @unittest.skip(
        "Superseded by the exact 2+26 aggregate All Effects contract test."
    )
    def test_source_contract_is_flat_lazy_and_valtan_specific(self) -> None:
        validate_source_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8"),
            EFFECT_TOOL_HEADER.read_text(encoding="utf-8"),
        )

    def test_editor_admission_is_independent_from_product_and_preview_joins(self) -> None:
        validate_editor_admission_isolation_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8"),
            DIRECT_AUTHORED_INDEX_CPP.read_text(encoding="utf-8"),
            DIRECT_AUTHORED_INDEX_HEADER.read_text(encoding="utf-8"),
        )

        catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        effect_id = "effect.valtan.pattern.420633.active"
        row = catalog[effect_id]
        self.assertEqual("DIRECT_AUTHORED_DOCUMENT", row["payloadKind"])
        document = json.loads(
            (REPOSITORY_ROOT / "Data" / row["authoringPath"]).read_text(
                encoding="utf-8"
            )
        )
        self.assertEqual(15, document["version"])
        self.assertEqual(effect_id, document["effectAssetId"])

    def test_failed_valtan_tree_cold_load_retries_only_on_explicit_refresh(self) -> None:
        cpp_text = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        header_text = EFFECT_TOOL_HEADER.read_text(encoding="utf-8")
        refresh = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree(",
            "bool_t Client::CEffect_Tool::Matches_ValtanPatternSearch(",
        )
        render = function_slice(
            cpp_text,
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
            "void Client::CEffect_Tool::Render_AllEffectsWindow(",
        )

        self.assertIn(
            "bool_t m_bValtanPatternTreeLoadAttempted = false;",
            header_text,
        )
        self.assertIn("m_bValtanPatternTreeLoadAttempted = true;", refresh)
        self.assertIn(
            "if (!m_bValtanPatternTreeLoaded && "
            "!m_bValtanPatternTreeLoadAttempted)",
            render,
        )

    def test_pattern_master_joins_two_independent_effects_once_at_root(self) -> None:
        master = json.loads(VALTAN_MASTER_PATH.read_text(encoding="utf-8"))
        self.assertEqual("lostark.valtan-pattern-master", master["schema"])
        self.assertEqual(1, master["formatVersion"])
        independent = master["independentEffects"]
        self.assertEqual(
            {
                "valtan.independent-effect.target-axe":
                    "effect.valtan.sky-axe.active",
                "valtan.independent-effect.donut-in-out":
                    "effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01",
            },
            {
                row["independentEffectId"]: row["effectAssetId"]
                for row in independent
            },
        )
        patterns = {row["patternId"]: row for row in master["patterns"]}
        self.assertEqual(7, len(patterns))
        for row in independent:
            if row["ownership"] == "SERVER_COMBAT_OBJECT":
                self.assertIsNone(row["cueProjection"])
            else:
                self.assertEqual(
                    {
                        "clipOccurrenceId",
                        "sourceStartMs",
                        "sourceEndMs",
                        "mappingBasis",
                    },
                    set(row["cueProjection"]),
                )
            owner = patterns[row["ownerPatternId"]]
            self.assertIn(
                row["ownerStageId"],
                {stage["stageId"] for stage in owner["stages"]},
            )
            references = [
                (owner["patternId"], stage["stageId"])
                for stage in owner["stages"]
                for ref in stage["effectRefs"]
                if ref == {
                    "refType": "INDEPENDENT_EFFECT",
                    "refId": row["independentEffectId"],
                }
            ]
            self.assertEqual(
                [(row["ownerPatternId"], row["ownerStageId"])],
                references,
                row["independentEffectId"],
            )

        tree_cpp = VALTAN_PATTERN_TREE_CPP.read_text(encoding="utf-8")
        tree_header = VALTAN_PATTERN_TREE_HEADER.read_text(encoding="utf-8")
        tool_cpp = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        for token in (
            "Parse_MasterDocument",
            "Apply_MasterDocument",
            '"lostark.valtan-pattern-master"',
            "View.IndependentEffects = Master.IndependentEffects",
            "IndependentEffectIds.push_back",
            '"effectCueBindingId", "cueProjection"',
            "Independent.bHasCueProjection",
        ):
            self.assertIn(token, tree_cpp)
        self.assertIn(
            "std::vector<VALTAN_INDEPENDENT_EFFECT_VIEW> IndependentEffects;",
            tree_header,
        )
        independent_render = function_slice(
            tool_cpp,
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
        )
        for token in (
            'ImGui::SmallButton("Open Editor")',
            '"Play Effect" : "Play Effect + Owner Animation"',
            'ImGui::SmallButton("Play Server Owner")',
            "Try_OpenValtanStandaloneEffect",
            "Try_PlayValtanStandaloneEffect",
            "Try_PlayValtanSavedUnifiedEffect",
            "Try_OpenValtanSavedReferenceEffect",
            "Try_PlayValtanServerPattern(*pOwnerPattern)",
        ):
            self.assertIn(token, independent_render)
        for token in (
            '"INDEPENDENT EFFECT (PATTERN 2 + AREA "',
            "m_ValtanAreaMapEffectDocument.Get_Surfaces().size() +",
            "m_ValtanAreaMapEffectDocument.Get_WorldEffects().size()",
            '"PATTERN-OWNED INDEPENDENT EFFECT (2)"',
        ):
            self.assertIn(token, tool_cpp)

    @unittest.skip(
        "All Effects no longer exposes local stage/clip authoring timelines."
    )
    def test_split_animation_occurrences_project_and_dash_paths_are_explicit(self) -> None:
        presentation = json.loads(
            VALTAN_PRESENTATION_PATH.read_text(encoding="utf-8")
        )
        bindings = {
            row["actionId"]: row["clips"]
            for row in json.loads(
                PATTERN_BINDING_PATH.read_text(encoding="utf-8")
            )["bindings"]
        }
        for pattern in presentation["patterns"]:
            for stage in pattern["stages"]:
                expected = [
                    {
                        "clipOccurrenceId": row["clipOccurrenceId"],
                        "clip": row["clip"],
                        "mappingBasis": row["mappingBasis"],
                        "sourceStartMs": row["sourceStartMs"],
                        "playMs": row["playMs"],
                        "playRate": row["playRate"],
                        "loop": row["repeatUntilStageEnd"],
                    }
                    for row in stage["animation"]["occurrences"]
                ]
                self.assertEqual(expected, bindings[stage["actionId"]])

        dash = next(
            row for row in presentation["patterns"]
            if row["patternId"] == "VALTAN_DASH_CHARGE"
        )
        windup = next(
            row for row in dash["stages"] if row["stageId"] == "WINDUP"
        )
        self.assertEqual(3, windup["animation"]["repeatCount"])
        self.assertEqual(3, len(windup["animation"]["occurrences"]))
        tool_cpp = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        for token in (
            "CValtanPatternTree::Build_PreviewStagePath(",
            "VALTAN_PATTERN_PREVIEW_PATH::NORMAL",
            "VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY",
            "VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK",
            'ImGui::SmallButton("Play Authoring Timeline")',
            "pStage->iAuthoringRepeatCount",
            "iPlayableOccurrenceCount",
            "Pattern.PresentationSources",
            "Source.iSourceActionId",
            "Source.iSequenceIndex",
            "StagedClips.push_back(Clip)",
        ):
            self.assertIn(token, tool_cpp)

    @unittest.skip(
        "All Effects Product timeline preview was replaced by Play Server."
    )
    def test_full_product_cue_uses_the_complete_pattern_timeline(self) -> None:
        validate_full_valtan_product_timeline_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8"),
            EFFECT_TOOL_HEADER.read_text(encoding="utf-8"),
        )

        master = json.loads(VALTAN_MASTER_PATH.read_text(encoding="utf-8"))
        patterns = {row["patternId"]: row for row in master["patterns"]}
        # The donut cue belongs to INNER, which begins after WINDUP. This is
        # the concrete regression case: its effect must not start at pattern 0.
        donut = next(
            row
            for row in master["independentEffects"]
            if row["independentEffectId"]
            == "valtan.independent-effect.donut-in-out"
        )
        owner = patterns[donut["ownerPatternId"]]
        owner_stage_index = next(
            index
            for index, stage in enumerate(owner["stages"])
            if stage["stageId"] == donut["ownerStageId"]
        )
        self.assertEqual(
            1500,
            sum(
                stage["durationMs"]
                for stage in owner["stages"][:owner_stage_index]
            ),
        )
        self.assertEqual("SERVER_PATTERN_STAGE", donut["ownership"])
        self.assertIsNotNone(donut["cueProjection"])
        axe = next(
            row
            for row in master["independentEffects"]
            if row["independentEffectId"]
            == "valtan.independent-effect.target-axe"
        )
        self.assertEqual("SERVER_COMBAT_OBJECT", axe["ownership"])
        self.assertIsNone(axe["cueProjection"])

    def test_master_wall_budget_caps_loop_and_keeps_full_timeline_seekable(self) -> None:
        tree_header = VALTAN_PATTERN_TREE_HEADER.read_text(encoding="utf-8")
        tool_header = EFFECT_TOOL_HEADER.read_text(encoding="utf-8")
        tool_cpp = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        self.assertIn("uint32_t iAuthoringWallMs = 0u;", tree_header)
        self.assertIn("uint32_t iAuthoringWallMs = 0u;", tool_header)
        for token in (
            "Assign_MasterWallBudgets",
            "Clip.iAuthoringWallMs = Source.iAuthoringWallMs",
            "fTimelineClipWallDurationSeconds",
            "fTimelineClipWallSeconds",
            "m_iSynchronizedAnimationLoopEpoch",
            "Seek_SynchronizedAnimationSequence",
            "m_iValtanReferenceEffectStartMs",
            "iEffectStartMs += Stage.iDurationMs",
            "Preserve the final source pose",
        ):
            self.assertIn(
                token,
                VALTAN_PATTERN_TREE_CPP.read_text(encoding="utf-8") + tool_cpp,
            )

    @unittest.skip(
        "Pattern rows now own one aggregate authoring Effect child."
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

    @unittest.skip(
        "Stage rows were intentionally removed from All Effects."
    )
    def test_animation_stage_rows_do_not_decode_or_open_saved_documents(self) -> None:
        validate_animation_stage_metadata_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    @unittest.skip(
        "Stage-reference local playback was intentionally removed."
    )
    def test_stage_reference_play_uses_the_complete_clip_sequence(self) -> None:
        validate_stage_reference_sequence_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    @unittest.skip(
        "Legacy saved-row aggregation was replaced by one aggregate sidecar slot."
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
        self.assertEqual(4000, stages["AIRBORNE"]["durationMs"])
        self.assertEqual(3200, stages["LAND"]["durationMs"])
        self.assertEqual(400, stages["RECOVERY"]["durationMs"])
        self.assertEqual("LAND", high_jump["serverMotion"]["travelStageId"])
        self.assertEqual(
            [
                {
                    "trigger": "ENTER",
                    "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
                    "targetId": "combatobject.valtan.high-jump.target-axe",
                    "targetingPolicy": "PER_ALIVE_PLAYER",
                    "layout": "SINGLE",
                    "countPerResolvedTarget": 1,
                    "radiusM": 0,
                    "startAngleDegrees": 0,
                    "angleStepDegrees": 0,
                    "allowOverlap": False,
                    "maximumTotalObjects": 36,
                    "spawnCount": 3,
                    "spawnIntervalMs": 1333,
                    "arenaRandomCount": 4,
                    "arenaRandomRadiusM": 14.0,
                    "arenaHeightToleranceM": 1.0,
                    "arenaAnchorPolicy": "BOSS_SPAWN_POSITION",
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
        self.assertEqual(4000, target_axe["lifeMs"])
        self.assertEqual(1, len(target_axe["hits"]))
        self.assertEqual(1200, target_axe["hits"][0]["atMs"])
        self.assertEqual(1, target_axe["hits"][0]["repeatCount"])

        cpp_text = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        header_text = EFFECT_TOOL_HEADER.read_text(encoding="utf-8")
        pattern = function_slice(
            cpp_text,
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
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
        create_element = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Try_CreateMeshEffect(",
            "bool_t Client::CEffect_Tool::Try_UseSelectedElementAsAuthoringPreset()",
        )
        snapshot_root = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Try_SnapshotValtanWorldPreviewRoot()",
            "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
        )
        independent = function_slice(
            cpp_text,
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
        )
        pending_load = function_slice(
            cpp_text,
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
            "bool_t Client::CEffect_Tool::Refresh_AllEffects(",
        )
        for token in (
            "iValtanWorldOwnerStageDurationMs",
            "m_iValtanWorldOwnerStageDurationMs",
        ):
            self.assertIn(token, header_text)
        for token in (
            "Build_ValtanAuthoringTimeline(",
            "iReferenceTimelineDurationMs",
            "iReferenceEffectStartMs += Stage.iDurationMs",
        ):
            self.assertIn(token, pattern)
        self.assertIn("iWorldOwnerStageDurationMs", reference_open)
        self.assertIn("Recalculate_PreviewDuration();", reference_open)
        self.assertIn(
            "m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;",
            reference_open,
        )
        self.assertIn("m_iValtanWorldOwnerStageDurationMs", duration)
        self.assertIn("(std::max)(", duration)
        self.assertIn(
            "iPreviousValtanWorldOwnerStageDurationMs",
            create_element,
        )
        self.assertGreaterEqual(
            create_element.count(
                "m_iValtanWorldOwnerStageDurationMs ="
            ),
            2,
        )
        self.assertIn(
            "CAnimationTargetService::Resolve_RootTransform(&TargetRoot)",
            snapshot_root,
        )
        self.assertIn("XMMatrixRotationQuaternion(Rotation) *", snapshot_root)
        self.assertIn(
            "XMMatrixTranslationFromVector(Translation)", snapshot_root
        )
        self.assertIn("Try_SnapshotValtanWorldPreviewRoot()", reference_open)
        self.assertIn("Try_SnapshotValtanWorldPreviewRoot()", pending_load)
        self.assertIn(
            "iOwnerTimelineDurationMs, true, iEffectStartMs", independent
        )
        self.assertIn('ImGui::SmallButton("Play Server Owner")', independent)
        self.assertIn("World preview root:", independent)

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
        self.assertEqual(1200, stages["SPIN"]["durationMs"])
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
                    "playMs": 533,
                    "playRate": 0.4441666667,
                    "loop": False,
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
        source_duration = 1.2 * 0.888888889
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
            if element["kind"] == "trail":
                self.assertAlmostEqual(
                    1.4, preview_end / 0.761904762, places=5
                )
            else:
                self.assertAlmostEqual(
                    0.9166666435, preview_end, places=7
                )

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
        self.assertNotIn("fStageEndMs", visible)
        self.assertIn(
            "CUE_END source window remains bounded by "
            "Resolve_CuePreviewSample",
            visible,
        )
        duration = function_slice(
            cpp_text,
            "void Client::CEffect_Tool::Recalculate_PreviewDuration(",
            "bool_t Client::CEffect_Tool::Has_UnsavedWork() const",
        )
        self.assertIn(
            "m_ValtanProductPreview->Cue.iStageDurationMs", duration
        )

    def test_optional_authored_slot_delete_and_hit_resource_contract(self) -> None:
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
                "emissive": "Effect/Warlord/Textures/FX_TEX_00/fx_a_hit_007.dds",
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

        self.assertEqual(
            "DIRECT_AUTHORED_DOCUMENT",
            source_catalog[effect_id]["payloadKind"],
        )

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
        self.assertEqual(1500, stages["CHARGE"]["durationMs"])
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
                    "playRate": 0.6,
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
            self.assertEqual(
                "DIRECT_AUTHORED_DOCUMENT",
                catalog[effect_id]["payloadKind"],
            )
            self.assertEqual(
                f"Effects/Authored/{effect_id}.effect.json",
                catalog[effect_id]["authoringPath"],
            )
            self.assertEqual(effect_id, source_document["effectAssetId"])

    def test_live_data_projects_every_owned_saved_document_once_per_pattern(self) -> None:
        projected, raw_links = project_saved_rows()
        expected_projected, expected_raw_links = (
            derive_authoritative_saved_link_closure()
        )
        self.assertEqual(expected_projected, projected)
        self.assertEqual(expected_raw_links, raw_links)
        self.assertEqual(len(expected_projected), len(projected))
        self.assertEqual(
            sum(len(rows) for rows in expected_projected.values()),
            sum(len(rows) for rows in projected.values()),
        )
        self.assertEqual(2, len(projected["VALTAN_DASH_CHARGE"]))
        self.assertEqual(1, len(projected["VALTAN_FRONT_BACK_FRONT"]))
        self.assertNotIn("VALTAN_TRIPLE_SLASH", projected)
        self.assertNotIn("VALTAN_ROTATION_SLASH", projected)
        self.assertEqual(3, len(projected["VALTAN_FOUR_SLASH"]))
        self.assertEqual(
            [
                "effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01",
                "effect.valtan.sky-axe.active",
                "effect.valtan.carrier-v1.attack.high-jump.land.clip-01",
            ],
            projected["VALTAN_HIGH_JUMP"],
        )
        self.assertEqual(
            [
                "effect.valtan.carrier-v1.mechanic.four-pillars-105.takeoff.clip-01",
                "effect.valtan.carrier-v1.mechanic.four-pillars-105.target-cone.clip-01",
            ],
            projected["VALTAN_FOUR_PILLARS_105"],
        )
        self.assertEqual(
            "effect.valtan.carrier-v1.attack.four-slash.active.clip-01",
            projected["VALTAN_FOUR_SLASH"][0],
        )
        self.assertEqual(
            "effect.valtan.carrier-v1.attack.four-slash.active.clip-02",
            projected["VALTAN_FOUR_SLASH"][1],
        )
        self.assertEqual(4, len(projected["VALTAN_WHIRLWIND"]))
        self.assertEqual(
            len(projected["VALTAN_WHIRLWIND"]),
            len(set(projected["VALTAN_WHIRLWIND"])),
        )

    def test_entrance_whirlwind_has_two_exact_product_cues(self) -> None:
        cue_document = json.loads(CUE_PATH.read_text(encoding="utf-8"))
        entrance_cues = [
            cue for cue in cue_document["cues"]
            if cue["patternId"] == "VALTAN_ENTRANCE_WHIRLWIND"
        ]
        self.assertEqual(
            [
                (
                    "cue.valtan.entrance-whirlwind.sweep.carrier-v1",
                    "effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01",
                ),
                (
                    "cue.valtan.entrance-whirlwind.sweep.active",
                    "effect.valtan.pattern.420633.active",
                ),
            ],
            [
                (cue["bindingId"], cue["effectAssetId"])
                for cue in entrance_cues
            ],
        )
        for cue in entrance_cues:
            self.assertEqual("SWEEP", cue["stageId"])
            self.assertEqual(
                "valtan.mechanic.entrance-whirlwind.sweep",
                cue["actionId"],
            )
            self.assertEqual(
                "valtan.mechanic.entrance-whirlwind.sweep.clip.01",
                cue["clipOccurrenceId"],
            )
            self.assertEqual("root", cue["anchorSlotId"])
            self.assertEqual("follow", cue["followPolicy"])
            self.assertEqual("natural", cue["stopPolicy"])
            self.assertEqual("once", cue["repeatPolicy"])
            self.assertEqual(0, cue["sourceStartMs"])
            self.assertIsNone(cue["sourceEndMs"])
            self.assertEqual(
                {
                    "kind": "GAMEPLAY_FOOTPRINT",
                    "worldScale": [1.5, 1.5, 1.5],
                },
                cue["scalePolicy"],
            )

    def test_base_saved_projection_inventory_requires_explicit_migration(self) -> None:
        # FIST_IN_OUT is intentionally one animation-free INNER stage now. The
        # old OUTER/RECOVERY generated reference documents remain inert empty
        # tombstones on disk, but are no longer runtime rows in the pattern.
        retired_fist_reference_tombstones = {
            "effect.valtan.fist-in-out.outer",
            "effect.valtan.fist-in-out.recovery",
        }
        encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))
        fist_pattern = next(
            row
            for row in encounter["patterns"]
            if row["patternId"] == "VALTAN_FIST_IN_OUT"
        )
        self.assertEqual(
            [("INNER", "valtan.attack.fist-in-out.inner")],
            [
                (stage["stageId"], stage["actionId"])
                for stage in fist_pattern["stages"]
            ],
        )
        fist_bindings = {
            row["actionId"]: row
            for row in json.loads(
                PATTERN_BINDING_PATH.read_text(encoding="utf-8")
            )["bindings"]
            if row["actionId"].startswith("valtan.attack.fist-in-out.")
        }
        self.assertEqual(
            {
                "valtan.attack.fist-in-out.windup",
                "valtan.attack.fist-in-out.inner",
                "valtan.attack.fist-in-out.outer",
                "valtan.attack.fist-in-out.recovery",
            },
            set(fist_bindings),
        )
        for binding in fist_bindings.values():
            self.assertEqual("NONE", binding["playbackMode"])
            self.assertEqual([], binding["clips"])

        projected, raw_links = project_saved_rows(
            include_v1_aliases=False,
            include_reference_only=True,
        )
        expected_projected, expected_raw_links = (
            derive_authoritative_saved_link_closure(
                include_v1_aliases=False,
                include_reference_only=True,
            )
        )
        self.assertEqual(expected_projected, projected)
        self.assertEqual(expected_raw_links, raw_links)
        self.assertEqual(
            ["effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01"],
            projected["VALTAN_FIST_IN_OUT"],
        )
        flattened_ids = {
            effect_id
            for effect_ids in projected.values()
            for effect_id in effect_ids
        }
        self.assertTrue(retired_fist_reference_tombstones.isdisjoint(flattened_ids))
        catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        for effect_id in retired_fist_reference_tombstones:
            path = AUTHORED_ROOT / f"{effect_id}.effect.json"
            document = json.loads(path.read_text(encoding="utf-8"))
            self.assertEqual([], document.get("elements"), effect_id)
            self.assertEqual([], document.get("modelCues"), effect_id)
        nonempty_candidates = 0
        empty_shells = 0
        empty_effect_ids: set[str] = set()
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
                    empty_effect_ids.add(effect_id)
        self.assertEqual(
            sum(len(rows) for rows in expected_projected.values()),
            nonempty_candidates + empty_shells,
        )
        product_effect_ids = {
            cue["effectAssetId"]
            for cue in json.loads(CUE_PATH.read_text(encoding="utf-8"))["cues"]
        }
        self.assertTrue(empty_effect_ids)
        self.assertTrue(empty_effect_ids.isdisjoint(product_effect_ids))

    def test_all_declared_v0_product_cues_remain_canonical_and_nonempty(self) -> None:
        cues = json.loads(CUE_PATH.read_text(encoding="utf-8"))["cues"]
        source_catalog = json.loads(
            SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
        )["effects"]
        project_tuned_ids = {
            "effect.valtan.project-tuned.dash-charge.active-shield",
            "effect.valtan.project-tuned.dash-charge.windup-telegraph",
            "effect.valtan.project-tuned.sequence.attack-whirlwind",
            "effect.valtan.project-tuned.sequence.catch-breath",
            "effect.valtan.project-tuned.sequence.counter",
            "effect.valtan.project-tuned.sequence.six-pizza-106",
            "effect.valtan.project-tuned.sequence.three",
            "effect.valtan.project-tuned.sequence.trash",
            "effect.valtan.project-tuned.sequence.trash-catch-fail",
            "effect.valtan.project-tuned.sequence.trash-catch-if",
            "effect.valtan.project-tuned.sequence.trash-catch-success",
            "effect.valtan.project-tuned.sequence.warp.portal",
            "effect.valtan.project-tuned.terrain-destruction-3.semicircle",
            "effect.valtan.project-tuned.terrain-destruction-9.semicircle",
        }
        self.assertEqual(
            project_tuned_ids,
            {
                cue["effectAssetId"]
                for cue in cues
                if cue["effectAssetId"].startswith("effect.valtan.project-tuned.")
            },
        )
        floor_wipe_cues = [
            cue
            for cue in cues
            if cue["bindingId"] ==
            "cue.valtan.carrier-v1.mechanic.floor-wipe-130.windup.clip-01"
        ]
        self.assertEqual(1, len(floor_wipe_cues))
        self.assertEqual(
            "effect.valtan.floor-wipe-130",
            floor_wipe_cues[0]["effectAssetId"],
        )
        self.assertEqual("WINDUP", floor_wipe_cues[0]["stageId"])
        self.assertEqual("natural", floor_wipe_cues[0]["stopPolicy"])
        self.assertEqual("once", floor_wipe_cues[0]["repeatPolicy"])
        sealed_cues = [
            cue for cue in cues if cue["effectAssetId"] not in project_tuned_ids
        ]
        effect_ids = [cue["effectAssetId"] for cue in sealed_cues]
        sealed_binding_ids = [cue["bindingId"] for cue in sealed_cues]
        self.assertEqual(len(sealed_binding_ids), len(set(sealed_binding_ids)))
        encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))
        canonical_stage_keys = {
            (pattern["patternId"], stage["stageId"], stage["actionId"])
            for pattern in encounter["patterns"]
            for stage in pattern["stages"]
        }
        self.assertTrue(
            all(
                (cue["patternId"], cue["stageId"], cue["actionId"])
                in canonical_stage_keys
                for cue in sealed_cues
            )
        )
        # Stable cue identities are the closure. Multiple occurrences may
        # intentionally reuse one reviewed authored Effect document.
        self.assertLess(len(set(effect_ids)), len(effect_ids))
        drawable_effect_ids = set()
        model_cues = 0
        observed_empty_shells = set()
        for effect_id in effect_ids:
            source_matches = [
                row for row in source_catalog if row["effectAssetId"] == effect_id
            ]
            self.assertEqual(1, len(source_matches), effect_id)
            source_entry = source_matches[0]
            self.assertEqual(
                "DIRECT_AUTHORED_DOCUMENT",
                source_entry.get("payloadKind"),
                effect_id,
            )
            self.assertEqual(
                f"Effects/Authored/{effect_id}.effect.json",
                source_entry["authoringPath"],
                effect_id,
            )
            source_path = REPOSITORY_ROOT / "Data" / source_entry["authoringPath"]
            source_document = json.loads(source_path.read_text(encoding="utf-8"))
            self.assertEqual(effect_id, source_document["effectAssetId"])
            if effect_id in INTENTIONAL_EMPTY_PRODUCT_SHELLS:
                self.assertEqual([], source_document.get("elements"), effect_id)
                self.assertEqual([], source_document.get("modelCues"), effect_id)
                observed_empty_shells.add(effect_id)
            else:
                self.assertTrue(
                    source_document.get("elements") or
                    source_document.get("modelCues"),
                    effect_id,
                )
            model_cues += len(source_document.get("modelCues", []))
            if effect_id not in INTENTIONAL_EMPTY_PRODUCT_SHELLS:
                is_drawable = (
                    any(
                        element.get("visible", True)
                        for element in source_document.get("elements", [])
                    ) or bool(source_document.get("modelCues"))
                )
                self.assertTrue(is_drawable, effect_id)
                if is_drawable:
                    drawable_effect_ids.add(effect_id)
        # The reviewed set includes the authored FLOOR_WIPE carrier rows.
        # Sky Axe is combat-object owned, so its exact element set is covered
        # separately below instead of contributing to this cue inventory.
        self.assertEqual(INTENTIONAL_EMPTY_PRODUCT_SHELLS, observed_empty_shells)
        self.assertEqual(
            set(effect_ids) - INTENTIONAL_EMPTY_PRODUCT_SHELLS,
            drawable_effect_ids,
        )
        self.assertEqual(0, model_cues)

    def test_recovered_valtan_effects_have_exact_reviewed_tuning_set(self) -> None:
        source_catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        for effect_id, expected_ids in RECOVERED_VALTAN_EFFECT_ELEMENT_IDS.items():
            with self.subTest(effect_id=effect_id):
                source_entry = source_catalog[effect_id]
                source_path = REPOSITORY_ROOT / "Data" / source_entry["authoringPath"]
                source_document = json.loads(source_path.read_text(encoding="utf-8"))
                element_ids = [row["id"] for row in source_document["elements"]]
                self.assertEqual(len(expected_ids), len(element_ids))
                self.assertEqual(len(element_ids), len(set(element_ids)))
                self.assertEqual(expected_ids, frozenset(element_ids))

                self.assertEqual(
                    "DIRECT_AUTHORED_DOCUMENT",
                    source_entry["payloadKind"],
                )

    def test_sky_axe_does_not_reuse_donut_impact_rows(self) -> None:
        sky_axe_path = AUTHORED_ROOT / "effect.valtan.sky-axe.active.effect.json"
        sky_axe = json.loads(sky_axe_path.read_text(encoding="utf-8"))
        self.assertFalse(
            any(
                "donut.impact.wave.black" in row["id"]
                for row in sky_axe["elements"]
            )
        )

    def test_recovered_floor_wipe_and_center_landing_sources_are_catalogued(
        self,
    ) -> None:
        source_catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        floor_id = "effect.valtan.floor-wipe-130"
        center_id = "effect.valtan.high-jump.center-landing.active"
        self.assertEqual(
            "Effects/Authored/effect.valtan.floor-wipe-130.effect.json",
            source_catalog[floor_id]["authoringPath"],
        )
        self.assertEqual(
            "Effects/Authored/effect.valtan.high-jump.center-landing.active.effect.json",
            source_catalog[center_id]["authoringPath"],
        )
        floor = json.loads(
            (
                REPOSITORY_ROOT / "Data" /
                source_catalog[floor_id]["authoringPath"]
            ).read_text(encoding="utf-8")
        )
        self.assertEqual(
            RECOVERED_VALTAN_EFFECT_ELEMENT_IDS[floor_id],
            frozenset(row["id"] for row in floor["elements"]),
        )
        self.assertTrue(all(row["visible"] for row in floor["elements"]))
        self.assertEqual([], floor["modelCues"])
        center = json.loads(
            (
                REPOSITORY_ROOT / "Data" /
                source_catalog[center_id]["authoringPath"]
            ).read_text(encoding="utf-8")
        )
        self.assertEqual(
            {
                "sprite_particle_2",
                "authored.copy.authored.copy.donut.impact.wave.black.1.1",
            },
            {row["id"] for row in center["elements"]},
        )
        self.assertTrue(all(row["visible"] for row in center["elements"]))

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
        direct_ids = {
            effect_id
            for effect_id, row in source_catalog.items()
            if row.get("payloadKind") == "DIRECT_AUTHORED_DOCUMENT"
        }
        self.assertEqual(6, len(aliases))
        self.assertEqual(6, len({row["effectAssetId"] for row in aliases}))
        self.assertEqual(6, len({row["v1EffectAssetId"] for row in aliases}))
        for row in aliases:
            self.assertIn(row["effectAssetId"], cue_ids)
            self.assertIn(row["effectAssetId"], source_catalog)
            self.assertIn(row["v1EffectAssetId"], source_catalog)
            self.assertIn(row["v1EffectAssetId"], direct_ids)
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
            cpp_text.replace("Open Editor", "Open Effect"),
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
            cpp_text.replace(
                '"effect.valtan.pattern.420633.active.v1.unified"',
                '"effect.valtan.pattern.420633.active.v1.disabled"',
                1,
            ),
            cpp_text.replace(
                "Matches_ValtanExactHistoryBinding(",
                "Matches_DisabledValtanExactHistoryBinding(",
            ),
            cpp_text.replace(
                "Seek_ValtanBossPatternTransformHistory(",
                "Seek_DisabledValtanBossPatternTransformHistory(",
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
