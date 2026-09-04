#!/usr/bin/env python3
"""Focused source contracts for the native Composition descriptor facade."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from Tools.CompositionPipeline import composition_pipeline as pipeline  # noqa: E402


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for cursor in range(opening, len(source)):
        if source[cursor] == "{":
            depth += 1
        elif source[cursor] == "}":
            depth -= 1
            if depth == 0:
                return source[start : cursor + 1]
    raise AssertionError(f"unterminated function: {signature}")


class NativeCompositionFacadeContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.workbench_h = read("Client/Public/ActionCompositionWorkbench.h")
        cls.sequencer_h = read("Client/Public/SequencerTool.h")
        cls.sequencer_cpp = read("Client/Private/SequencerTool.cpp")
        cls.main_cpp = read("Client/Private/MainApp.cpp")
        cls.document_h = read("Client/Public/BossCompositionDocument.h")
        cls.document_cpp = read("Client/Private/BossCompositionDocument.cpp")
        cls.harness_project = read(
            "Tools/ValtanPatternAuditionServiceHarness/Default/"
            "ValtanPatternAuditionServiceHarness.vcxproj"
        )
        cls.harness_main = read(
            "Tools/ValtanPatternAuditionServiceHarness/Private/"
            "ValtanPatternAuditionServiceHarness.cpp"
        )
        cls.harness_contract = read(
            "Tools/ValtanPatternAuditionServiceHarness/Private/"
            "BossCompositionDocumentContractTests.cpp"
        )

    def test_workbench_generation_invalidates_only_the_read_only_facade(self) -> None:
        self.assertIn(
            "Get_CanonicalDisplayGeneration() const noexcept", self.workbench_h
        )
        self.assertIn("return m_iCanonicalDisplayGeneration;", self.workbench_h)

        synchronize = body(
            self.sequencer_cpp,
            "Synchronize_SourceDocumentsWithCanonicalGeneration()",
        )
        for token in (
            "Get_CanonicalDisplayGeneration()",
            "generation != m_iObservedCanonicalDisplayGeneration",
            "Reload_SourceDocuments();",
        ):
            self.assertIn(token, synchronize)
        self.assertIn(
            "Synchronize_SourceDocumentsWithCanonicalGeneration();",
            body(
                self.sequencer_cpp,
                "void Client::CSequencerTool::Render_SourceDocumentHeader()",
            ),
        )

    def test_selected_pair_is_staged_without_loading_the_unrelated_boss(self) -> None:
        self.assertIn("bool_t Load_Pair(", self.document_h)
        self.assertNotIn("Load_All", self.document_h)
        load_pair = body(
            self.document_cpp,
            "bool_t Client::CCompositionDocumentCatalog::Load_Pair(",
        )
        for token in (
            '"boss.composition.valtan" == compositionId',
            '"boss.composition.kakulsaydon" == compositionId',
            "stagedBoss.Load(bossPath, status)",
            "stagedArena.Load(arenaPath, status)",
            "m_BossDocuments = std::move(stagedBosses)",
            "m_ArenaDocuments = std::move(stagedArenas)",
        ):
            self.assertIn(token, load_pair)
        self.assertLess(
            load_pair.index("stagedArena.Load(arenaPath, status)"),
            load_pair.index("m_BossDocuments = std::move(stagedBosses)"),
        )
        reload_facade = body(
            self.sequencer_cpp,
            "void Client::CSequencerTool::Reload_SourceDocuments()",
        )
        self.assertIn("stagedCatalog.Load_Pair(bossId, arenaId", reload_facade)

    def test_uint_fields_and_actor_pattern_references_fail_closed(self) -> None:
        read_u32 = body(self.document_cpp, "bool_t Read_U32(")
        self.assertIn("value->Was_FloatingPointToken()", read_u32)
        for token in (
            "ARENA_WORLD_SEQUENCE_REFERENCE",
            "ARENA_CAMERA_SHOT_REFERENCE",
            "ARENA_ACTOR_PATTERN_REFERENCE",
            "ARENA_SEQUENCER_TRACK_REFERENCE",
        ):
            self.assertIn(token, self.document_h)
        load_pair = body(
            self.document_cpp,
            "bool_t Client::CCompositionDocumentCatalog::Load_Pair(",
        )
        self.assertIn("std::get_if<ARENA_ACTOR_PATTERN_REFERENCE>", load_pair)
        self.assertIn("stagedBoss.Find_Pattern(actor->patternId)", load_pair)

    def test_ui_does_not_claim_cross_owner_revision_admission(self) -> None:
        for token in (
            "descriptor revision",
            "This view only parsed source descriptors.",
            "Cross-owner exact revision/reference admission belongs to the Composition Publisher receipt.",
            "WORLD_SEQUENCE and CAMERA_SHOT owner resolution is Publisher validation",
        ):
            self.assertIn(token, self.sequencer_cpp)

    def test_source_details_live_only_in_the_collapsed_advanced_inspector(
        self,
    ) -> None:
        self.assertIn(
            "void Render_AdvancedSourceInspector();", self.sequencer_h
        )
        header = body(
            self.sequencer_cpp,
            "void Client::CSequencerTool::Render_SourceDocumentHeader()",
        )
        advanced = body(
            self.sequencer_cpp,
            "void Client::CSequencerTool::Render_AdvancedSourceInspector()",
        )
        render = body(
            self.sequencer_cpp,
            "void Client::CSequencerTool::Render()",
        )

        self.assertIn(
            '"Advanced Source Inspector##UnifiedCompositionAdvanced"',
            advanced,
        )
        collapsed_at = advanced.index("ImGui::CollapsingHeader(")
        early_return_at = advanced.index("return;", collapsed_at)
        reload_at = advanced.index(
            'ImGui::SmallButton("Reload Sources##UnifiedComposition")'
        )
        self.assertLess(collapsed_at, early_return_at)
        self.assertLess(early_return_at, reload_at)
        for advanced_only in (
            'ImGui::TextWrapped("%s", m_strSourceDocumentStatus.c_str())',
            "descriptor revision",
            'ImGui::TreeNode("Source References##UnifiedComposition")',
            'ImGui::SeparatorText("Boss Composition Projection")',
            "Render_SourcePatternSummary();",
            "Render_ArenaSequencerSummary();",
        ):
            self.assertIn(advanced_only, advanced)
            self.assertNotIn(advanced_only, header)

        self.assertIn(
            "Source descriptor unavailable; last-good owner view preserved. "
            "See Advanced Source Inspector.",
            header,
        )
        self.assertIn(
            "ImGui::SetNextWindowSize(ImVec2(960.f, 460.f), "
            "ImGuiCond_FirstUseEver)",
            render,
        )
        self.assertNotIn("Render_SourcePatternSummary();", render)
        self.assertNotIn("Render_ArenaSequencerSummary();", render)
        self.assertEqual(
            7,
            render.count("Render_AdvancedSourceInspector();"),
            "every normal and early-return surface must keep the inspector reachable",
        )

    def test_kakul_profiles_deep_link_to_the_single_animation_writer(self) -> None:
        for token in (
            "Consume_KakulAnimationOpenRequest",
            '"Kakul"',
            '"Saydon"',
            '"Large Saydon"',
            '"Kakul + Saydon"',
            "Open Create / Append / Preview",
            "not a scaled MN_RPCT_05",
        ):
            self.assertIn(token, self.sequencer_cpp)
        for token in (
            "Consume_KakulAnimationOpenRequest",
            "EnsureDebugTool(DEBUG_TOOL::ANIMATION)",
            "m_pAnimationTool->Open_KakulProfile(kakulProfileId)",
            "m_eDebugWindowFocusPending = DEBUG_TOOL::ANIMATION",
        ):
            self.assertIn(token, self.main_cpp)

    def test_fixed_source_closure_has_an_executable_native_contract(self) -> None:
        for token in ("Has_ExactSourceClosure", "Has_KakulSourceClosure"):
            self.assertIn(token, self.document_cpp)
        for source_contract in (
            pipeline.VALTAN_SOURCE_DOCUMENTS,
            pipeline.VALTAN_ARENA_SOURCE_DOCUMENTS,
            pipeline.KAKUL_ARENA_SOURCE_DOCUMENTS,
        ):
            for role, path in source_contract.items():
                self.assertIn(f'{{ "{role}", "{path}" }}', self.document_cpp)
        for token in (
            "BossCompositionDocumentContractTests.cpp",
            r"Client\Private\BossCompositionDocument.cpp",
        ):
            self.assertIn(token, self.harness_project)
        self.assertIn(
            "Run_BossCompositionDocumentContractTests();", self.harness_main
        )
        for token in (
            "LOSTARK_PROJECT_DATA_ROOT",
            "Data/Items/ItemCatalog.json",
            "ACTION_REFERENCE_WRONG",
            "source role/path closure",
            "RequirePreserved",
        ):
            self.assertIn(token, self.harness_contract)


if __name__ == "__main__":
    unittest.main()
