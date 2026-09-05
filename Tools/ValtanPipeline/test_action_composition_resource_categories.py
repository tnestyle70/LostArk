#!/usr/bin/env python3
"""Focused catalog and Valtan workbench animation-resource isolation contract."""

from __future__ import annotations

import json
import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
ANIMATION_CPP = ROOT / "Client/Private/Animation_Tool.cpp"
ANIMATION_HEADER = ROOT / "Client/Public/Animation_Tool.h"
WORKBENCH_CPP = ROOT / "Client/Private/ValtanActionWorkbench.cpp"
WORKBENCH_HEADER = ROOT / "Client/Public/ValtanActionWorkbench.h"
PROFILES = (
    "MN_RPCT_05",
    "MN_RPCT_06",
    "MN_RPCT_07",
    "MN_RPCZ_00",
)


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for cursor in range(opening, len(source)):
        if source[cursor] == "{":
            depth += 1
        elif source[cursor] == "}":
            depth -= 1
            if depth == 0:
                return source[opening : cursor + 1]
    raise AssertionError(f"unterminated function: {signature}")


def category(profile: str, display_name: str) -> str:
    if profile == "MN_RPCZ_00" and display_name.startswith("대형 쿠크_"):
        return "Large Kouku"
    if profile in ("MN_RPCZ_00", "MN_RPCT_07"):
        return "Kouku"
    return "Saydon"


def effective_rows(document: dict) -> list[dict]:
    return [
        action
        for action in document["actions"]
        if action["reviewStatus"] == "REVIEW_CANDIDATE"
        and any(stage["slots"] for stage in action["stages"])
    ]


class ValtanActionWorkbenchResourceIsolationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.animation = ANIMATION_CPP.read_text(encoding="utf-8")
        cls.animation_header = ANIMATION_HEADER.read_text(encoding="utf-8")
        cls.workbench = WORKBENCH_CPP.read_text(encoding="utf-8")
        cls.workbench_header = WORKBENCH_HEADER.read_text(encoding="utf-8")
        cls.documents: dict[str, dict] = {}
        cls.authored_documents: dict[str, dict] = {}
        for profile in PROFILES:
            folder = (
                ROOT
                / "Data/Animation/Reference/KoukuSaydon"
            )
            cls.documents[profile] = json.loads(
                (folder / f"{profile}.actionreference.json").read_text(
                    encoding="utf-8"
                )
            )
            authored_path = (
                ROOT
                / "Data/Animation/Authored/KoukuSaydon"
                / f"{profile}.actionbindings.json"
            )
            cls.authored_documents[profile] = json.loads(
                authored_path.read_text(encoding="utf-8")
            )

    def test_fixed_typed_documents_supply_meaningful_designer_rows(self) -> None:
        names: set[str] = set()
        counts = {"Kouku": 0, "Large Kouku": 0, "Saydon": 0}
        for profile in PROFILES:
            document = self.documents[profile]
            authored = self.authored_documents[profile]
            self.assertEqual(profile, document["profileId"])
            self.assertEqual(profile, authored["profileId"])
            self.assertEqual(
                document["referenceRevision"], authored["referenceRevision"]
            )
            rows = effective_rows(document)
            self.assertTrue(rows, profile)
            for action in rows:
                display_name = action["displayName"]
                self.assertTrue(display_name.strip())
                self.assertTrue(
                    any("가" <= character <= "힣" for character in display_name),
                    display_name,
                )
                names.add(display_name)
                counts[category(profile, display_name)] += 1

        for expected in ("Kouku", "Large Kouku", "Saydon"):
            self.assertGreater(counts[expected], 0, expected)
        for expected_name in (
            "세이튼_불뿜기 쇼",
            "대형 세이튼_내려찍기",
            "쿠크세이튼_슬라이딩 태클 후 내려찍기",
            "대형 쿠크_순간이동",
        ):
            self.assertIn(expected_name, names)

    def test_generic_catalog_retains_fixed_kouku_sources(self) -> None:
        loader = function_body(
            self.animation,
            "bool_t Load_KoukuSaydonCompositionSequenceLibrary(",
        )
        generic_catalog = function_body(
            self.animation,
            "bool_t Client::CAnimation_Tool::Get_ActionCompositionSequenceCatalog(",
        )
        for profile in PROFILES:
            self.assertIn(f'"{profile}"', self.animation)
        for token in (
            "Resolve_ReferencePath",
            "Resolve_AuthoredPath",
            "Parse_ReferenceText",
            "Parse_AuthoredText",
            '"REVIEW_CANDIDATE" != Action.strReviewStatus',
            "if (!Sequence.Clips.empty())",
        ):
            self.assertIn(token, loader)
        self.assertNotIn("recursive_directory_iterator", loader)
        self.assertNotIn("directory_iterator", loader)
        self.assertIn("Load_KoukuSaydonCompositionSequenceLibrary(", generic_catalog)
        for token in (
            "Open_KoukuSaydonProfile(",
            "Open_KoukuSaydonAction(",
            "Get_ActionCompositionSequenceCatalog(",
        ):
            self.assertIn(token, self.animation_header)

    def test_valtan_workbench_uses_only_valtan_sequence_catalog(self) -> None:
        reload_sequences = function_body(
            self.workbench,
            "bool_t Client::CValtanActionWorkbench::Reload_AnimationSequences()",
        )
        browser = function_body(
            self.workbench,
            "void Client::CValtanActionWorkbench::Render_SequenceBrowser(",
        )
        apply = function_body(
            self.workbench,
            "bool_t Client::CValtanActionWorkbench::Apply_SelectedSequenceToStage(",
        )
        self.assertIn("Get_ValtanCompositionSequences(", reload_sequences)
        self.assertNotIn("Get_ActionCompositionSequenceCatalog(", reload_sequences)
        self.assertNotIn("Parse_ReferenceText", browser)
        self.assertNotIn("Resolve_ReferencePath", browser)
        for token in (
            "Sequence.strProfileId",
            "m_strSelectedSequenceStableId",
            "Apply_SelectedSequenceToStage(*pPattern, *pStage, false)",
            "Apply_SelectedSequenceToStage(*pPattern, *pStage, true)",
            "Stage_ValtanCompositionIntakeSequence(",
        ):
            self.assertIn(token, browser)
        for forbidden in (
            "Kouku",
            "Kakul",
            "Saydon",
            "Data/Animation/Reference/KoukuSaydon",
            "Data/Animation/Authored/KoukuSaydon",
            "Character/KoukuSaton",
            "ACTION_COMPOSITION_SEQUENCE_CATEGORIES",
            "m_iAnimationSequenceCategory",
            '"All Categories"',
            "Get_ActionCompositionSequenceCatalog(",
            "Open_KoukuSaydonAction(",
            "bValtanPatternCompatible",
        ):
            self.assertNotIn(forbidden, self.workbench)
            self.assertNotIn(forbidden, self.workbench_header)
        self.assertNotIn("bValtanPatternCompatible", apply)


if __name__ == "__main__":
    unittest.main()
