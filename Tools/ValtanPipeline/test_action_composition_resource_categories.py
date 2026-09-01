#!/usr/bin/env python3
"""Focused catalog/browser contract for Composition animation resources."""

from __future__ import annotations

import json
import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
ANIMATION_CPP = ROOT / "Client/Private/Animation_Tool.cpp"
WORKBENCH_CPP = ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
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
        return "Large Kakul"
    if profile in ("MN_RPCZ_00", "MN_RPCT_07"):
        return "Kakul"
    return "Saydon"


def effective_rows(document: dict) -> list[dict]:
    return [
        action
        for action in document["actions"]
        if action["reviewStatus"] == "REVIEW_CANDIDATE"
        and any(stage["slots"] for stage in action["stages"])
    ]


class ActionCompositionResourceCategoryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.animation = ANIMATION_CPP.read_text(encoding="utf-8")
        cls.workbench = WORKBENCH_CPP.read_text(encoding="utf-8")
        cls.documents: dict[str, dict] = {}
        cls.authored_documents: dict[str, dict] = {}
        for profile in PROFILES:
            folder = (
                ROOT
                / "Data/Animation/Reference/KakulSaydon"
            )
            cls.documents[profile] = json.loads(
                (folder / f"{profile}.actionreference.json").read_text(
                    encoding="utf-8"
                )
            )
            authored_path = (
                ROOT
                / "Data/Animation/Authored/KakulSaydon"
                / f"{profile}.actionbindings.json"
            )
            cls.authored_documents[profile] = json.loads(
                authored_path.read_text(encoding="utf-8")
            )

    def test_fixed_typed_documents_supply_meaningful_designer_rows(self) -> None:
        names: set[str] = set()
        counts = {"Kakul": 0, "Large Kakul": 0, "Saydon": 0}
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

        for expected in ("Kakul", "Large Kakul", "Saydon"):
            self.assertGreater(counts[expected], 0, expected)
        for expected_name in (
            "세이튼_불뿜기 쇼",
            "대형 세이튼_내려찍기",
            "쿠크세이튼_슬라이딩 태클 후 내려찍기",
            "대형 쿠크_순간이동",
        ):
            self.assertIn(expected_name, names)

    def test_catalog_reads_only_fixed_sources_on_explicit_reload(self) -> None:
        loader = function_body(
            self.animation,
            "bool_t Load_KakulSaydonCompositionSequenceLibrary(",
        )
        reload_sequences = function_body(
            self.workbench,
            "bool_t Client::CActionCompositionWorkbench::Reload_AnimationSequences()",
        )
        render_browser = function_body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Render_SequenceBrowser(",
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
        self.assertIn("Get_ActionCompositionSequenceCatalog(", reload_sequences)
        self.assertNotIn("Parse_ReferenceText", render_browser)
        self.assertNotIn("Resolve_ReferencePath", render_browser)

    def test_category_selection_preserves_valtan_slots_and_routes_profiles(self) -> None:
        browser = function_body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Render_SequenceBrowser(",
        )
        apply = function_body(
            self.workbench,
            "bool_t Client::CActionCompositionWorkbench::Apply_SelectedSequenceToStage(",
        )
        for label in ("Valtan", "Kakul", "Large Kakul", "Saydon"):
            self.assertIn(f'"{label}"', self.workbench)
        for token in (
            "m_iAnimationSequenceCategory",
            '"All Categories"',
            "m_iAnimationSequenceCategory < 0 || Sequence.strCategory ==",
            "Sequence.strProfileId",
            "m_strSelectedSequenceStableId",
            "Open_KakulAction(",
            "Apply_SelectedSequenceToStage(*pPattern, *pStage, false)",
            "Apply_SelectedSequenceToStage(*pPattern, *pStage, true)",
            "Stage_ValtanCompositionIntakeSequence(",
        ):
            self.assertIn(token, browser)
        self.assertIn(
            'int32_t m_iAnimationSequenceCategory = -1;',
            (ROOT / "Client/Public/ActionCompositionWorkbench.h").read_text(
                encoding="utf-8"
            ),
        )
        category_segment = browser.index(
            'Sequence.strCategory.empty() ? "UNCATEGORIZED"'
        )
        profile_segment = browser.index(
            'Sequence.strProfileId.empty() ? "UNKNOWN PROFILE"'
        )
        self.assertLess(category_segment, profile_segment)
        self.assertIn("!Selected->bValtanPatternCompatible", apply)


if __name__ == "__main__":
    unittest.main()
