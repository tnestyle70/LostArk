#!/usr/bin/env python3
"""Focused contract for Boss Verification canonical RING authoring and live reveal."""

from __future__ import annotations

import copy
import json
import pathlib
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ValtanPipeline"))
import valtan_tuning_pipeline as pipeline  # noqa: E402


BOSS_CPP = ROOT / "Client/Private/ValtanBossTool.cpp"
BOSS_H = ROOT / "Client/Public/ValtanBossTool.h"
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
TREE_CPP = ROOT / "Client/Private/ValtanPatternTree.cpp"
TREE_H = ROOT / "Client/Public/ValtanPatternTree.h"
PROMOTER = ROOT / "Tools/ValtanPipeline/promote_valtan_animation_chains.py"


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


class ValtanValtanBossToolRingAuthoringContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.docs = pipeline.load_pipeline_documents(ROOT)
        cls.source_revision = pipeline.source_manifest(ROOT)["sourceManifestId"]
        cls.master = pipeline.join_v2_authoring(
            cls.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            cls.docs[pipeline.PRESENTATION_AUTHORING_REL],
            cls.docs[pipeline.WORLD_SET_REL],
            cls.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        cls.boss_cpp = BOSS_CPP.read_text(encoding="utf-8")
        cls.boss_h = BOSS_H.read_text(encoding="utf-8")
        cls.balance_cpp = BALANCE_CPP.read_text(encoding="utf-8")
        cls.balance_h = BALANCE_H.read_text(encoding="utf-8")
        cls.tree_cpp = TREE_CPP.read_text(encoding="utf-8")
        cls.tree_h = TREE_H.read_text(encoding="utf-8")
        cls.promoter = PROMOTER.read_text(encoding="utf-8")

    def patch(self, **overrides: object) -> dict[str, object]:
        operation: dict[str, object] = {
            "op": "SET_COMBAT_OBJECT_RING_HIT",
            "patternId": "VALTAN_FIST_IN_OUT",
            "stageId": "INNER",
            "combatObjectArchetypeId":
                "combatobject.valtan.fist-in-out.donut",
            "hitId": "hit.valtan.fist-in-out.donut.01",
            "innerRadiusM": 7.5,
            "outerRadiusM": 15.5,
        }
        operation.update(overrides)
        return {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.source_revision,
            "operations": [operation],
        }

    def apply(self, patch: dict[str, object]):
        return pipeline.apply_draft_patch(
            self.master,
            self.docs[pipeline.BOSS_PROFILES_REL],
            self.docs[pipeline.DAMAGE_REL],
            patch,
            self.source_revision,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
            repository_root=ROOT,
            effect_catalog=self.docs[pipeline.EFFECT_CATALOG_REL],
            include_combat_authoring=True,
        )

    @staticmethod
    def ring_shape(combat: dict[str, object]) -> dict[str, object]:
        objects = combat["objects"]
        assert isinstance(objects, list)
        donut = next(
            row for row in objects
            if row["combatObjectArchetypeId"] ==
            "combatobject.valtan.fist-in-out.donut"
        )
        return donut["hits"][0]["shape"]

    def test_typed_patch_changes_only_the_copied_canonical_combat_owner(self) -> None:
        original_combat = copy.deepcopy(self.docs[pipeline.COMBAT_AUTHORING_REL])
        master, bosses, damage, combat, count = self.apply(self.patch())
        self.assertEqual(1, count)
        self.assertEqual(self.master, master)
        self.assertEqual(self.docs[pipeline.BOSS_PROFILES_REL], bosses)
        self.assertEqual(self.docs[pipeline.DAMAGE_REL], damage)
        self.assertEqual(original_combat, self.docs[pipeline.COMBAT_AUTHORING_REL])
        self.assertEqual(7.5, self.ring_shape(combat)["innerRadiusM"])
        self.assertEqual(15.5, self.ring_shape(combat)["outerRadiusM"])

        candidate_docs = dict(self.docs)
        candidate_docs[pipeline.COMBAT_AUTHORING_REL] = combat
        products = pipeline.project_v2_products(ROOT, candidate_docs, master)
        product = json.loads(products[pipeline.COMBAT_PRODUCT_REL])
        product_donut = next(
            row for row in product["objects"]
            if row["combatObjectArchetypeId"] ==
            "combatobject.valtan.fist-in-out.donut"
        )
        self.assertEqual(7.5, product_donut["hits"][0]["hitInnerRadius"])
        self.assertEqual(15.5, product_donut["hits"][0]["hitOuterRadius"])

    def test_wrong_owner_hit_and_inverted_radii_fail_closed(self) -> None:
        for overrides in (
            {"patternId": "VALTAN_HIGH_JUMP"},
            {"stageId": "MISSING"},
            {"hitId": "hit.valtan.missing"},
            {"innerRadiusM": 16.0, "outerRadiusM": 8.0},
        ):
            with self.subTest(overrides=overrides):
                with self.assertRaises(pipeline.DraftPatchError):
                    self.apply(self.patch(**overrides))

    def test_immutable_authoring_save_contains_the_changed_combat_owner(self) -> None:
        intermediate = ROOT / "Intermediate"
        intermediate.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(
            prefix="boss-ring-authoring-", dir=intermediate
        ) as directory:
            authoring_root = pathlib.Path(directory)
            result = pipeline.save_authoring(
                ROOT, authoring_root, self.patch()
            )
            revision = result["revisionId"]
            saved = pipeline.read_json(
                authoring_root
                / "revisions"
                / revision
                / pipeline.COMBAT_AUTHORING_REL
            )
            self.assertEqual(7.5, self.ring_shape(saved)["innerRadiusM"])
            self.assertEqual(15.5, self.ring_shape(saved)["outerRadiusM"])

    def test_boss_slots_use_shared_typed_draft_and_canonical_save(self) -> None:
        render = function_body(
            self.boss_cpp,
            "bool_t Client::CValtanBossTool::Render_SelectedPatternRingAuthoring(",
        )
        for marker in (
            "ImGui::InputDouble",
            "Get_ValtanStageDraft",
            "Set_ValtanStageDraft",
            "Get_ValtanCombatObjectRingHitDraft",
            "Set_ValtanCombatObjectRingHitDraft",
            'ImGui::Button("Save Canonical Ring Geometry")',
        ):
            self.assertIn(marker, render)
        self.assertNotIn("static double", render)
        self.assertNotRegex(self.boss_h, r"m_\w*(?:Inner|Outer)Radius")
        self.assertIn(
            "every Flow occurrence of %s uses these same radii", render
        )
        self.assertIn("Different radii require a different stable Pattern definition", render)

        selected = function_body(
            self.boss_cpp,
            "void Client::CValtanBossTool::Render_SelectedPattern()",
        )
        self.assertRegex(
            selected,
            r"if \(Render_SelectedPatternRingAuthoring\(\*pPattern\)\)\s*"
            r"return;\s*Render_ConnectionSummary\(\*pPattern, \*pStage\)",
        )
        save_button = render.index(
            'ImGui::Button("Save Canonical Ring Geometry")'
        )
        end_disabled = render.index("ImGui::EndDisabled();", save_button)
        early_return = render.index("return true;", save_button)
        self.assertLess(save_button, end_disabled)
        self.assertLess(end_disabled, early_return)

        save = function_body(
            self.boss_cpp,
            "bool_t Client::CValtanBossTool::Save_SelectedPatternRingAuthoring()",
        )
        for marker in (
            "Can_MutateCanonicalGraph",
            "Save_ValtanCanonicalProduct",
            "Save_ValtanProduct",
            "Reload_Graph",
        ):
            self.assertIn(marker, save)

    def test_combat_geometry_is_joined_and_committed_in_the_existing_cas(self) -> None:
        for marker in (
            "VALTAN_COMBAT_OBJECT_HIT_VIEW",
            "std::vector<VALTAN_COMBAT_OBJECT_HIT_VIEW> Hits",
        ):
            self.assertIn(marker, self.tree_h)
        for marker in (
            'Read_RequiredFiniteFloat(\n\t\t\t\t\tHit, "hitInnerRadius"',
            "Reference->second.Hits.push_back",
            "View.Hits = Reference->second.Hits",
        ):
            self.assertIn(marker, self.tree_cpp)
        self.assertIn("VALTAN_COMBAT_OBJECT_RING_HIT_EDIT", self.balance_h)
        self.assertIn("SET_COMBAT_OBJECT_RING_HIT", self.balance_cpp)
        apply_start = self.promoter.index("def commit_typed_authoring_patch(")
        apply_end = self.promoter.index("\ndef ", apply_start + 1)
        apply_body = self.promoter[apply_start:apply_end]
        self.assertIn("resolve_authoring_combat_base", apply_body)
        self.assertIn("include_combat_authoring=True", apply_body)
        self.assertIn("repo_root / pipeline.COMBAT_AUTHORING_REL", apply_body)
        self.assertIn("combat_authoring=committed_combat", apply_body)

    def test_live_reveal_is_exact_and_never_overwrites_manual_selection(self) -> None:
        sync = function_body(
            self.boss_cpp,
            "void Client::CValtanBossTool::Synchronize_LiveSelection()",
        )
        self.assertIn("Find_AuditionPattern(Boss.strPatternId)", sync)
        self.assertIn("Find_LiveStage(*pPattern)", sync)
        self.assertIn("m_strLivePatternId = pPattern->strPatternId", sync)
        self.assertNotIn("m_strSelectedPatternId =", sync)
        self.assertNotIn("m_strSelectedStageId =", sync)
        listing = function_body(
            self.boss_cpp, "void Client::CValtanBossTool::Render_PatternList()"
        )
        self.assertIn('Label += "  [LIVE]"', listing)
        self.assertIn("ImGui::SetScrollHereY(0.5f)", listing)
        self.assertIn("m_bFollowLive", listing)


if __name__ == "__main__":
    unittest.main()
