from __future__ import annotations

import json
import re
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
EFFECT_TOOL_CPP = REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
EFFECT_TOOL_HEADER = REPOSITORY_ROOT / "Client/Public/Effect_Tool.h"
PATTERN_TREE_CPP = REPOSITORY_ROOT / "Client/Private/ValtanPatternTree.cpp"
CHARACTER_PREVIEW_CPP = (
    REPOSITORY_ROOT / "Client/Private/CharacterPreviewPanel.cpp"
)
OWNERSHIP_CPP = (
    REPOSITORY_ROOT
    / "Client/Private/ValtanPatternAuthoringEffectDocument.cpp"
)
OWNERSHIP_HEADER = (
    REPOSITORY_ROOT
    / "Client/Public/ValtanPatternAuthoringEffectDocument.h"
)
OWNERSHIP_JSON = (
    REPOSITORY_ROOT / "Data/Effects/ValtanPatternAuthoringEffects.json"
)
GAMEPLAY_JSON = REPOSITORY_ROOT / "Data/Valtan/Valtan.gameplay.json"
PATTERN_PRODUCT_JSON = REPOSITORY_ROOT / "Data/Valtan/Valtan.pattern.json"
PRESENTATION_JSON = REPOSITORY_ROOT / "Data/Valtan/Valtan.presentation.json"
PRODUCT_CUES_JSON = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
EFFECT_CATALOG_JSON = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
CLIENT_PROJECT = REPOSITORY_ROOT / "Client/Default/Client.vcxproj"
CLIENT_FILTERS = REPOSITORY_ROOT / "Client/Default/Client.vcxproj.filters"

EXPECTED_INDEPENDENT_IDS = [
    "valtan.independent-effect.donut-in-out",
    "valtan.independent-effect.target-axe",
]
EXPECTED_CORE_PATTERN_IDS = [
    "VALTAN_WHIRLWIND",
    "VALTAN_FOUR_SLASH",
    "VALTAN_HIGH_JUMP",
    "VALTAN_DASH_CHARGE",
    "VALTAN_FLOOR_WIPE_130",
    "VALTAN_ARENA_BREAK_109",
    "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
    "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
]


def source_section(source: str, start: str, end: str) -> str:
    start_index = source.index(start)
    end_index = source.index(end, start_index)
    return source[start_index:end_index]


def string_array(source: str, name: str) -> list[str]:
    match = re.search(
        rf"{re.escape(name)}\s*=\s*\{{(?P<body>.*?)\}};",
        source,
        flags=re.DOTALL,
    )
    if match is None:
        raise AssertionError(f"array not found: {name}")
    return re.findall(r'"([^"]+)"', match.group("body"))


class EffectToolValtanAllEffectsContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cpp = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        cls.header = EFFECT_TOOL_HEADER.read_text(encoding="utf-8")
        cls.pattern_tree_cpp = PATTERN_TREE_CPP.read_text(encoding="utf-8")
        cls.character_preview = CHARACTER_PREVIEW_CPP.read_text(
            encoding="utf-8"
        )
        cls.ownership_cpp = OWNERSHIP_CPP.read_text(encoding="utf-8")
        cls.ownership_header = OWNERSHIP_HEADER.read_text(encoding="utf-8")
        cls.gameplay = json.loads(GAMEPLAY_JSON.read_text(encoding="utf-8"))
        cls.pattern_product = json.loads(
            PATTERN_PRODUCT_JSON.read_text(encoding="utf-8")
        )
        cls.presentation = json.loads(
            PRESENTATION_JSON.read_text(encoding="utf-8")
        )
        cls.product_cues = json.loads(
            PRODUCT_CUES_JSON.read_text(encoding="utf-8")
        )
        cls.effect_catalog = json.loads(
            EFFECT_CATALOG_JSON.read_text(encoding="utf-8")
        )

    def test_exact_two_plus_twenty_eight_inventory_is_explicit(self) -> None:
        self.assertEqual(
            EXPECTED_INDEPENDENT_IDS,
            string_array(
                self.cpp, "VALTAN_ALL_EFFECTS_INDEPENDENT_EFFECT_IDS"
            ),
        )
        self.assertEqual(
            EXPECTED_CORE_PATTERN_IDS,
            string_array(
                self.pattern_tree_cpp,
                "VALTAN_TOOL_AUDITION_CORE_PATTERN_IDS",
            ),
        )
        manual = self.gameplay["decisionModel"]["manualAuditions"]
        self.assertEqual(20, len(manual))
        manual_ids = [row["patternId"] for row in manual]
        self.assertEqual(20, len(set(manual_ids)))
        self.assertTrue(set(EXPECTED_CORE_PATTERN_IDS).isdisjoint(manual_ids))

        independent = {
            row["independentEffectId"]: row
            for row in self.pattern_product["independentEffects"]
        }
        self.assertEqual(set(EXPECTED_INDEPENDENT_IDS), set(independent))
        self.assertEqual(
            "effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01",
            independent[EXPECTED_INDEPENDENT_IDS[0]]["effectAssetId"],
        )
        self.assertEqual(
            "effect.valtan.sky-axe.active",
            independent[EXPECTED_INDEPENDENT_IDS[1]]["effectAssetId"],
        )
        self.assertNotIn("VALTAN_FIST_IN_OUT", EXPECTED_CORE_PATTERN_IDS)

    def test_tree_uses_authored_manual_order_and_no_legacy_fallback(self) -> None:
        tree = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
            "void Client::CEffect_Tool::Render_AllEffectsWindow()",
        )
        for token in (
            "VALTAN_ALL_EFFECTS_INDEPENDENT_EFFECT_IDS",
            "m_ValtanToolAuditionInventory.CorePatternIds",
            "m_ValtanToolAuditionInventory.AnimatorPatternIds",
            "CORE SERVER PATTERNS (8)",
            "ANIMATOR PATTERNS (20)",
            "INDEPENDENT EFFECT (2)",
            "TOTAL_PATTERN_COUNT !=",
            "no legacy or replacement row was substituted",
        ):
            self.assertIn(token, tree)
        self.assertIn(
            "CValtanPatternTree::Build_ToolAuditionInventory",
            self.cpp,
        )
        for retired_surface in (
            "CounterReactionLayers",
            "iIntroRotationIndex",
            "LegacyRotations",
            "Render_ValtanStageRow",
            'Render_ValtanPatternNode(Pattern, "Gimmick"',
        ):
            self.assertNotIn(retired_surface, tree)

    def test_published_pattern_effects_are_editable_and_keep_server_play(self) -> None:
        pattern = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
        )
        for token in (
            "m_strSelectedValtanPatternId = Pattern.strPatternId",
            'ImGui::SmallButton("Play Server")',
            "Try_PlayValtanServerPattern(Pattern)",
            "Pattern.Stages",
            "Stage.ProductCues",
            "RUNTIME_VALTAN_EFFECT_ROW",
            "Runtime Product Effects",
            'ImGui::SmallButton("Open Editor")',
            "Build_ValtanProductPreview",
            "Try_OpenValtanAuthoredEffect",
            "Try_PlayValtanSavedUnifiedEffect",
            "Try_OpenValtanSavedReferenceEffect",
            "iReferenceEffectStartMs",
            "Unpublished Pattern Draft",
            "Play Effect + Pattern",
            "Try_OpenValtanPatternDraftEffect",
        ):
            self.assertIn(token, pattern)
        self.assertNotIn("Play Draft Effect Only", pattern)
        self.assertIn(
            "return Row.ProductSources.empty() &&",
            pattern,
            "an independent Effect must still appear below the pattern that consumes it",
        )
        for retired_projection in (
            'Label += " | Effect 1"',
            "Effect | (not created)",
            "DRAFT_ATTACHED##aggregate-effect",
        ):
            self.assertNotIn(retired_projection, pattern)

    def test_pattern_draft_keeps_pattern_identity_and_timeline_through_play(self) -> None:
        create = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(",
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        )
        prepare = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Prepare_ActiveValtanPatternDraftTimeline(",
            "bool_t Client::CEffect_Tool::Try_OpenValtanPatternDraftEffect(",
        )
        open_draft = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenValtanPatternDraftEffect(",
            "bool_t Client::CEffect_Tool::Prepare_ValtanStandaloneEffectTarget(",
        )
        pending = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
            "bool_t Client::CEffect_Tool::Refresh_AllEffects(",
        )
        restart = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()",
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
        )
        synchronize = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
            "bool_t Client::CEffect_Tool::Start_SynchronizedAnimationClip(",
        )

        for token in (
            "VALTAN_PATTERN_DRAFT",
            "strValtanPatternId",
            "eValtanPatternPreviewPath",
            "m_strActiveValtanPatternDraftId",
            "m_eActiveValtanPatternDraftPreviewPath",
        ):
            self.assertIn(token, self.header)
        for token in (
            "Build_ValtanAuthoringTimeline",
            "Play_ValtanStageSequence",
            "m_iValtanWorldOwnerStageDurationMs = iTimelineDurationMs",
            "m_iValtanReferenceEffectStartMs = 0u",
            "Recalculate_PreviewDuration()",
            "Set_SynchronizedAnimationPaused(bPaused)",
        ):
            self.assertIn(token, prepare)
        for forbidden in (
            "m_ePreviewPivotKind =",
            "m_strPreviewAnchorSlotId.clear()",
        ):
            self.assertNotIn(
                forbidden,
                prepare,
                "Play/re-sync must preserve the Model View authoring pivot",
            )
        for token in (
            "EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT",
            "bPreserveAuthoringPivot",
            "m_PendingDocumentLoad->strValtanPatternId = Pattern.strPatternId",
            "m_PendingDocumentLoad->eValtanPatternPreviewPath",
            "m_PendingDocumentLoad->bPlayCompleteAfterLoad = bPlayAfterOpen",
            "m_strActiveValtanPatternDraftId = Pattern.strPatternId",
            "if (!bPreserveAuthoringPivot)",
            "m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT",
            "m_strPreviewAnchorSlotId.clear()",
            "Prepare_ActiveValtanPatternDraftTimeline(true)",
            "Try_PlayActiveUnifiedEffect()",
        ):
            self.assertIn(token, open_draft)
        self.assertLess(
            open_draft.index("if (!bAlreadyActive)"),
            open_draft.index("Refresh_UnifiedEffectCache"),
            "an active in-memory draft must not be rejected by the empty disk shell",
        )
        self.assertIn(
            "else if (bPlayAfterOpen && !m_bActiveDocumentDrawable)",
            open_draft,
        )
        for token in (
            "Pending.strValtanPatternId",
            "Pending.eValtanPatternPreviewPath",
            "m_strActiveValtanPatternDraftId = Pending.strValtanPatternId",
            "m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT",
            "m_strPreviewAnchorSlotId.clear()",
            "Prepare_ActiveValtanPatternDraftTimeline(true)",
            "Pending.bPlayCompleteAfterLoad",
            "Try_PlayActiveUnifiedEffect()",
        ):
            self.assertIn(token, pending)
        self.assertLess(
            pending.index(
                "EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT"
            ),
            pending.index("EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT"),
        )
        self.assertIn(
            "Prepare_ActiveValtanPatternDraftTimeline(false)", restart
        )
        self.assertIn(
            "Prepare_ActiveValtanPatternDraftTimeline(true)", synchronize
        )
        self.assertIn("Try_OpenValtanPatternDraftEffect", create)
        self.assertNotIn("Try_OpenValtanStandaloneEffect", create)

    def test_server_play_is_arena_only_and_reuses_typed_audition(self) -> None:
        resolver = source_section(
            self.cpp,
            "const char_t* Resolve_ValtanServerPatternBossPlacement(",
            "const char_t* Tool_PlayerStanceLabel(",
        )
        can_play = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Can_PlayValtanServerPattern(",
            "bool_t Client::CEffect_Tool::Try_PlayValtanServerPattern(",
        )
        play = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_PlayValtanServerPattern(",
            "void Client::CEffect_Tool::Update_ValtanServerPatternAudition()",
        )
        update = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Update_ValtanServerPatternAudition()",
            "bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(",
        )
        self.assertIn("LEVEL::VALTAN_ARENA", resolver)
        self.assertIn("boss.valtan.center", self.cpp)
        self.assertNotIn("CHARACTER_SELECT", resolver + can_play + play)
        self.assertIn("CNetworkManager::Get().Is_Connected()", can_play)
        for gate in (
            "CCombatHUDViewModel::Get().Get_Boss()",
            "CCombatHUDViewModel::Get().Get_Player()",
            "Boss.iCurrentHp",
            "Player.iCurrentHp",
            "Player.isCombatReady",
        ):
            self.assertIn(gate, can_play)
        self.assertIn("CValtanPatternAuditionService::Get().Submit", play)
        self.assertIn("VALTAN_EFFECT_TOOL_AUDITION_CONSUMER_ID", play)
        self.assertIn(
            "VALTAN_EFFECT_TOOL_AUDITION_CONSUMER_ID !=",
            update,
            "Effect Tool must not display another consumer's lifecycle as its own",
        )
        for local_preview in (
            "Play_ValtanAuthoringTimeline",
            "Play_ValtanStageSequence",
            "Start_Animation",
            "Clear_ProductCuePreview",
            "Reset_SynchronizedAnimationSequence",
            "Release_WorldPreview",
            "Send_ValtanPatternAuditionById",
            "Try_Consume_ValtanPatternAuditionByIdResult",
            "Request_RevivePlayer",
        ):
            self.assertNotIn(local_preview, play + update)
        self.assertNotIn(
            "CValtanPatternAuditionService::Get().Update();",
            play + update,
        )

    def test_all_twenty_eight_visible_pattern_names_are_canonical_korean(self) -> None:
        patterns = {
            row["patternId"]: row for row in self.gameplay["patterns"]
        }
        visible_pattern_ids = EXPECTED_CORE_PATTERN_IDS + [
            row["patternId"]
            for row in self.gameplay["decisionModel"]["manualAuditions"]
        ]
        self.assertEqual(28, len(visible_pattern_ids))
        self.assertEqual(28, len(set(visible_pattern_ids)))
        for pattern_id in visible_pattern_ids:
            display_name = patterns[pattern_id]["displayName"]
            with self.subTest(pattern_id=pattern_id):
                self.assertNotIn("[P2 Animation]", display_name)
                self.assertTrue(
                    any("가" <= character <= "힣" for character in display_name),
                    display_name,
                )

        pattern_node = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
        )
        self.assertIn(
            "Pattern.strPatternId : Pattern.strDisplayName",
            pattern_node,
            "the Korean display name must be the primary visible Pattern label",
        )

    def test_independent_rows_split_local_owner_preview_from_server_logic(self) -> None:
        independent = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
        )
        standalone_open = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenValtanStandaloneEffect(",
            "bool_t Client::CEffect_Tool::Try_PlayValtanStandaloneEffect(",
        )
        standalone_target = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Prepare_ValtanStandaloneEffectTarget(",
            "bool_t Client::CEffect_Tool::Try_OpenValtanStandaloneEffect(",
        )
        synchronize = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
            "bool_t Client::CEffect_Tool::Start_SynchronizedAnimationClip(",
        )
        update = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Update(const f32_t fTimeDelta)",
            "void Client::CEffect_Tool::Render()",
        )
        restart = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()",
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
        )
        pending = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
            "bool_t Client::CEffect_Tool::Refresh_AllEffects(",
        )
        for token in (
            "Build_ValtanAuthoringTimeline",
            "Build_ValtanProductPreview",
            "PatternStagePreview",
            "pStageClockCue",
            "Try_OpenValtanAuthoredEffect",
            "Try_OpenValtanSavedReferenceEffect",
            "Try_PlayValtanSavedUnifiedEffect",
            "Try_OpenValtanStandaloneEffect",
            "Try_PlayValtanStandaloneEffect",
            "iEffectStartMs",
            'ImGui::SmallButton("Play Server Owner")',
            "Try_PlayValtanServerPattern(*pOwnerPattern)",
            "actual target tracking and hit timing require Play Server",
        ):
            self.assertIn(token, independent)
        self.assertIn(
            "iOwnerTimelineDurationMs, true, iEffectStartMs",
            independent,
            "combat-object local playback must wait for its Server owner stage",
        )
        for token in (
            "EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT",
            "Clear_ProductCuePreview()",
            "Reset_SynchronizedAnimationSequence()",
            "pModel->Set_AnimPaused(true)",
            "CAnimationTargetService::Resolve_Boss()",
        ):
            self.assertIn(token, standalone_open + standalone_target)
        self.assertLess(
            standalone_target.index("pModel->Set_AnimPaused(true)"),
            standalone_target.index("Reset_SynchronizedAnimationSequence()"),
        )
        self.assertIn("m_eActiveDocumentPreviewIntent", synchronize)
        self.assertIn("Prepare_ValtanStandaloneEffectTarget()", synchronize)
        for invariant in (
            "bStandaloneValtanEffectActive",
            "(!bStaticAreaEffectActive && Has_UnsavedWork()) ||",
            "CAnimationTargetService::Resolve_Boss()",
            "Prepare_ValtanStandaloneEffectTarget()",
            "Release_WorldPreview(true)",
        ):
            self.assertIn(invariant, update)
        self.assertIn(
            "EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT", restart
        )
        self.assertIn("Prepare_ValtanStandaloneEffectTarget()", restart)
        self.assertIn("LEVEL::VALTAN_ARENA", self.character_preview)
        self.assertIn("bValtanArenaBossPreview", self.character_preview)
        self.assertIn("Pending.ePreviewIntent", pending)
        self.assertIn("Try_PlayActiveUnifiedEffect()", pending)

    def test_open_editor_stages_owner_timeline_paused_at_zero(self) -> None:
        saved_reference = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
            "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
        )
        authored = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(\n\tconst std::filesystem::path& Path,\n\tconst std::string& strEffectAssetId,\n\tconst VALTAN_PRODUCT_PREVIEW& Preview,",
            "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree()",
        )
        pending = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
            "bool_t Client::CEffect_Tool::Refresh_AllEffects(",
        )
        self.assertIn("if (!bQueuePlayCompleteAfterLoad)", saved_reference)
        self.assertIn(
            "bAnimationReady && !bQueuePlayCompleteAfterLoad", authored
        )
        for section in (saved_reference, authored):
            self.assertIn("Set_SynchronizedAnimationPaused(true)", section)
        self.assertGreaterEqual(
            pending.count("Set_SynchronizedAnimationPaused(true)"),
            2,
            "Save/Discard + Open must preserve the same paused owner timeline",
        )

    def test_authoritative_valtan_effect_inventory_and_timing_are_intact(self) -> None:
        cues = self.product_cues["cues"]
        cue_asset_ids = [row["effectAssetId"] for row in cues]
        self.assertEqual(81, len(cues))
        self.assertEqual(66, len(set(cue_asset_ids)))

        valtan_catalog = {
            row["effectAssetId"]: row
            for row in self.effect_catalog["effects"]
            if row["effectAssetId"].startswith("effect.valtan.")
        }
        self.assertGreaterEqual(len(valtan_catalog), 58)
        for effect_asset_id in set(cue_asset_ids) | {
            "effect.valtan.sky-axe.active"
        }:
            with self.subTest(effect_asset_id=effect_asset_id):
                self.assertIn(effect_asset_id, valtan_catalog)
                authoring_path = REPOSITORY_ROOT / "Data" / valtan_catalog[
                    effect_asset_id
                ]["authoringPath"]
                self.assertTrue(authoring_path.is_file(), authoring_path)
                document = json.loads(authoring_path.read_text(encoding="utf-8"))
                self.assertEqual(effect_asset_id, document["effectAssetId"])
                self.assertGreater(len(document["elements"]), 0)

        fist = next(
            row for row in self.presentation["patterns"]
            if row["patternId"] == "VALTAN_FIST_IN_OUT"
        )
        self.assertEqual(1, len(fist["stages"]))
        fist_stage = fist["stages"][0]
        self.assertEqual("NONE", fist_stage["animation"]["mode"])
        self.assertEqual("STAGE_CLOCK", fist_stage["effectCues"][0]["timingBasis"])
        product_fist = next(
            row for row in cues if row["patternId"] == "VALTAN_FIST_IN_OUT"
        )
        self.assertEqual("STAGE_CLOCK", product_fist["timingBasis"])
        self.assertNotIn("clipOccurrenceId", product_fist)

    def test_new_effect_is_a_two_document_cas_transaction(self) -> None:
        create = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(",
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        )
        for token in (
            "EFFECT_AUTHORING_FORMAT_VERSION",
            "Build_ValtanPatternAggregateEffectAssetId",
            "DRAFT_ATTACHED",
            "CEffectDocumentCodec::Save_AtomicIfUnchanged",
            "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged",
            "m_strValtanPatternAuthoringEffectsBaseline",
            "CValtanPatternAuthoringEffectTransaction Transaction",
            "CurrentOwnershipBaseline",
            "CAuthoritativeProductSourceReadLocks",
            "Try_LockAndInspectAuthoritativeProductOwnership",
            "CEffectCatalog::Contains",
            "Remove_EffectDocumentIfCanonical",
            "Refresh_ValtanPatternAuthoringEffects()",
        ):
            self.assertIn(token, create)
        self.assertLess(
            create.index("CEffectDocumentCodec::Save_AtomicIfUnchanged"),
            create.index(
                "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged"
            ),
        )
        for forbidden in (
            "EffectCatalog.json",
            "Valtan.gameplay.json",
            "Valtan.presentation.json",
            "Valtan.patternbindings.json",
            "CValtanPatternAuditionService",
        ):
            self.assertNotIn(forbidden, create)

    def test_pattern_draft_recovers_after_model_view_target_replacement(self) -> None:
        update = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Update(const f32_t fTimeDelta)",
            "void Client::CEffect_Tool::Render()",
        )
        for token in (
            "bValtanPatternDraftActive",
            "bValtanPatternDraftTimelineRebound",
            "bValtanPatternDraftTargetInvalid",
            "m_iSynchronizedAnimationTargetGeneration !=",
            "CAnimationTargetService::Resolve_TargetGeneration()",
            "Prepare_ActiveValtanPatternDraftTimeline(!m_bPreviewPlaying)",
            "bool_t bSeekAfterLoop = bValtanPatternDraftTimelineRebound",
            "bSeekAfterLoop = bSeekAfterLoop ||",
            "Seek_WorldPreviewWithSourceAnchorHistory",
            "pObject->Set_SampleTime(fEffectSampleSeconds)",
        ):
            self.assertIn(token, update)
        self.assertLess(
            update.index("m_iSynchronizedAnimationTargetGeneration !="),
            update.index("Update_SynchronizedAnimationSequence()"),
            "target replacement must rebuild the Pattern timeline before generic update resets it",
        )
        self.assertLess(
            update.index("bool_t bSeekAfterLoop = bValtanPatternDraftTimelineRebound"),
            update.index("pObject->Advance_Preview(fSequentialAdvance, Root)"),
            "target replacement must force a zero-time full seek before sequential Effect advance",
        )

    def test_new_effect_reports_auto_open_failure_without_rolling_back_commit(self) -> None:
        create = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(",
            "bool_t Client::CEffect_Tool::Can_DeleteSelectedValtanPatternEffect(",
        )
        self.assertNotIn("(void)Try_OpenValtanPatternDraftEffect", create)
        self.assertIn("if (!Try_OpenValtanPatternDraftEffect", create)
        self.assertIn("files remain committed", create)
        self.assertLess(
            create.rindex(
                "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged"
            ),
            create.index("if (!Try_OpenValtanPatternDraftEffect"),
        )

    def test_delete_effect_is_selected_row_scoped_and_confirmed(self) -> None:
        pattern = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
        )
        tree = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
            "void Client::CEffect_Tool::Render_AllEffectsWindow()",
        )
        self.assertIn("m_SelectedValtanPatternEffect", pattern)
        self.assertIn("Selection.CueIds", pattern)
        self.assertIn("VALTAN_PATTERN_EFFECT_SELECTION_KIND::PRODUCT_CUE_LINK", pattern)
        self.assertIn("VALTAN_PATTERN_EFFECT_SELECTION_KIND::DRAFT_ATTACHED", pattern)
        self.assertIn('ImGui::Button("Delete Effect")', tree)
        self.assertIn('ImGui::Button("Confirm Delete")', tree)
        self.assertIn("m_PendingValtanPatternEffectDeletion", tree)
        self.assertIn("bDeleteAttemptedThisFrame", tree)
        delete = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_DeleteSelectedValtanPatternEffect()",
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        )
        for token in (
            "Pending.eKind != Current.eKind",
            "Pending.strPatternId != Current.strPatternId",
            "Pending.strEffectAssetId != Current.strEffectAssetId",
            "Pending.CueIds != Current.CueIds",
            "nothing was deleted",
        ):
            self.assertIn(token, delete)
        self.assertLess(
            tree.index('ImGui::Button("Create Effect")'),
            tree.index('ImGui::Button("Delete Effect")'),
        )

    def test_draft_delete_uses_sidecar_cas_and_exact_file_delete(self) -> None:
        draft_delete = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_DeleteValtanPatternDraftEffect(",
            "bool_t Client::CEffect_Tool::Try_UnlinkValtanPatternProductEffect(",
        )
        for token in (
            "Build_ValtanPatternAggregateEffectAssetId",
            "Build_AuthoringPath",
            "CEffectDocumentCodec::Load",
            "CEffectCatalog::Contains",
            "Try_LockAndInspectAuthoritativeProductOwnership",
            "Count_ProductCueMappings",
            "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged",
            "Remove_EffectDocumentIfCanonical",
            "StagedOwnershipCanonical",
            "Discard_ActiveDocument",
        ):
            self.assertIn(token, draft_delete)
        self.assertLess(
            draft_delete.index(
                "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged"
            ),
            draft_delete.index("Remove_EffectDocumentIfCanonical"),
        )
        self.assertGreaterEqual(
            draft_delete.count(
                "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged"
            ),
            2,
            "file-delete failure must CAS-restore the sidecar ownership",
        )
        for token in (
            "CAuthoritativeProductSourceReadLocks",
            "FILE_SHARE_READ",
            "Try_ContainsSourceRegistrationFresh",
            "CValtanPatternTree::Load",
            "Count_ValtanProductReferences",
            "Valtan.gameplay.json",
            "Valtan.presentation.json",
            "ValtanEncounter.json",
            "ValtanPatternRotations.json",
            "Valtan.patternbindings.json",
            "Valtan.patterneffectcues.json",
            "Valtan.patterneffects.json",
            "BossCatalog.json",
            "ValtanCombatObjects.json",
            "complete authoritative Product read set",
        ):
            self.assertIn(token, self.cpp)

    def test_product_delete_unlinks_only_selected_pattern_and_preserves_asset(self) -> None:
        unlink = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_UnlinkValtanPatternProductEffect(",
            "bool_t Client::CEffect_Tool::Try_DeleteSelectedValtanPatternEffect(",
        )
        for token in (
            "Remove-ValtanPatternEffectLink.ps1",
            "-Mode Apply",
            "-PatternId",
            "-EffectAssetId",
            "-CueIds",
            "Start_OwnedToolProcess",
            "m_ValtanPatternProductUnlinkOperation",
            "the process will not be killed on timeout",
        ):
            self.assertIn(token, unlink)
        for forbidden in (
            "EffectCatalog.json",
            "std::filesystem::remove",
            "DeleteFileW",
            "Remove_EffectDocumentIfCanonical",
            "Run_OwnedToolProcess",
            "TerminateProcess",
        ):
            self.assertNotIn(forbidden, unlink)
        poll = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Update_ValtanPatternProductEffectUnlink()",
            "bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(",
        )
        for token in (
            "WaitForSingleObject(Operation.hProcess, 0u)",
            "WAIT_OBJECT_0 != iWait",
            "still running after 180 seconds",
            "It was not terminated",
            "All Effects remains locked",
            "Refresh_ValtanPatternTree()",
            "Refresh_DataFiles()",
            "does not assume that source or Product was preserved",
            "shared Effect asset, authored file, and other Pattern links were preserved",
            "Restart the Server and re-enter Valtan Arena",
        ):
            self.assertIn(token, poll)
        self.assertNotIn("TerminateProcess", poll)
        destructor = source_section(
            self.cpp,
            "Client::CEffect_Tool::~CEffect_Tool()",
            "void Client::CEffect_Tool::Update(const f32_t fTimeDelta)",
        )
        self.assertIn("CloseHandle", destructor)
        self.assertNotIn("TerminateProcess", destructor)

    def test_sidecar_is_effect_domain_only_and_registered_in_client(self) -> None:
        document = json.loads(OWNERSHIP_JSON.read_text(encoding="utf-8"))
        self.assertEqual(
            {"schema", "formatVersion", "bossArchetypeId", "bindings"},
            set(document),
        )
        self.assertEqual(
            "lostark.valtan-pattern-authoring-effects", document["schema"]
        )
        self.assertEqual(1, document["formatVersion"])
        self.assertEqual("BOSS_VALTAN", document["bossArchetypeId"])
        self.assertIsInstance(document["bindings"], list)
        self.assertTrue(document["bindings"])
        seen_patterns: set[str] = set()
        seen_effects: set[str] = set()
        for binding in document["bindings"]:
            self.assertEqual(
                {"patternId", "effectAssetId", "authoringPath", "state"},
                set(binding),
            )
            pattern_id = binding["patternId"]
            effect_id = binding["effectAssetId"]
            self.assertRegex(pattern_id, r"^[A-Z0-9_]+$")
            self.assertRegex(effect_id, r"^[a-z0-9._-]+$")
            self.assertEqual(
                f"Effects/Authored/{effect_id}.effect.json",
                binding["authoringPath"],
            )
            self.assertEqual("DRAFT_ATTACHED", binding["state"])
            self.assertNotIn(pattern_id, seen_patterns)
            self.assertNotIn(effect_id, seen_effects)
            seen_patterns.add(pattern_id)
            seen_effects.add(effect_id)
        self.assertIn("VALTAN_SEQUENCE_RUSH", seen_patterns)
        combined = self.ownership_header + self.ownership_cpp
        for token in (
            "Effects/ValtanPatternAuthoringEffects.json",
            "Effects/Authored/",
            "DRAFT_ATTACHED",
            "Save_AtomicIfUnchanged",
            "changed on disk; refresh before creating",
            "TRANSACTION_MUTEX_NAME",
            "WaitForSingleObject",
            "BeforeReplaceCanonical",
            "MoveFileExW",
        ):
            self.assertIn(token, combined)
        for forbidden in (
            "strStageId",
            "strClipName",
            "iDurationMs",
            "strAnimation",
            "EffectCatalog",
        ):
            self.assertNotIn(forbidden, self.ownership_header)

        project = CLIENT_PROJECT.read_text(encoding="utf-8")
        filters = CLIENT_FILTERS.read_text(encoding="utf-8")
        for text in (project, filters):
            self.assertIn("ValtanPatternAuthoringEffectDocument.h", text)
            self.assertIn("ValtanPatternAuthoringEffectDocument.cpp", text)
        self.assertIn("ValtanPatternAuthoringEffects.json", project)


if __name__ == "__main__":
    unittest.main(verbosity=2)
