#!/usr/bin/env python3
"""Focused contracts for the remaining native Composition descriptor reader.

The removed read-only Sequencer facade and automatic Animation-window deep link
have no UI oracles here; the shared Workbench has a separate editor contract.
"""

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
            'L"Compositions/Bosses/KoukuSaydonGate1.bosscomposition.json"',
            'L"Compositions/Sequences/KoukuSaydonArena.sequencer.json"',
            "stagedBoss.Load(bossPath, status)",
            "stagedArena.Load(arenaPath, status)",
            "m_BossDocuments = std::move(stagedBosses)",
            "m_ArenaDocuments = std::move(stagedArenas)",
        ):
            self.assertIn(token, load_pair)
        self.assertNotIn("KakulSaydon", load_pair)
        self.assertLess(
            load_pair.index("stagedArena.Load(arenaPath, status)"),
            load_pair.index("m_BossDocuments = std::move(stagedBosses)"),
        )

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

    def test_fixed_source_closure_has_an_executable_native_contract(self) -> None:
        for token in (
            "Has_ExactSourceClosure",
            "Has_KoukuSaydonSourceClosure",
        ):
            self.assertIn(token, self.document_cpp)
        for source_contract in (
            pipeline.VALTAN_SOURCE_DOCUMENTS,
            pipeline.VALTAN_ARENA_SOURCE_DOCUMENTS,
            pipeline.KOUKU_SAYDON_ARENA_SOURCE_DOCUMENTS,
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
