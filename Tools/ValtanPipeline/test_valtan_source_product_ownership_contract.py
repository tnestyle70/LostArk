#!/usr/bin/env python3
"""Executable ownership contract for Valtan presentation Products.

The split presentation document is the only writable animation/Effect
invocation owner.  The flat binding/cue documents are deterministic projector
outputs, and Product runtime presentation must not reopen split authoring.
"""

from __future__ import annotations

import copy
import json
from pathlib import Path
import sys
import unittest


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools" / "ValtanPipeline"))

import valtan_tuning_pipeline as pipeline  # noqa: E402


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


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
    raise AssertionError(f"unterminated function body: {signature}")


class ValtanSourceProductOwnershipContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.docs, cls.joined, cls.outputs = (
            pipeline.build_repository_product_projection(ROOT)
        )

    def test_checked_in_flat_products_equal_current_source_projection(self) -> None:
        for relative in (pipeline.BINDINGS_REL, pipeline.CUES_REL):
            self.assertEqual(
                json.loads(read(relative)),
                json.loads(self.outputs[relative]),
                f"generated Product is stale against {pipeline.PRESENTATION_AUTHORING_REL}: {relative}",
            )

    def test_presentation_edit_projects_to_cue_without_mutating_product(self) -> None:
        edited = copy.deepcopy(self.joined)
        cue = next(
            cue
            for pattern in edited["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
        )
        cue_id = cue["cueId"]
        before_bytes = (ROOT / pipeline.CUES_REL).read_bytes()
        cue["localTransform"]["rotationDegrees"][1] = 17.0

        projected = pipeline.project_v2_products(ROOT, self.docs, edited)
        projected_cues = json.loads(projected[pipeline.CUES_REL])["cues"]
        projected_cue = next(
            row for row in projected_cues if row["bindingId"] == cue_id
        )
        self.assertEqual(
            17.0, projected_cue["localTransform"]["rotationDegrees"][1]
        )
        self.assertEqual(before_bytes, (ROOT / pipeline.CUES_REL).read_bytes())

    def test_generated_documents_expose_read_only_loaders(self) -> None:
        binding_cpp = read("Client/Private/AnimationSkillBindingDocument.cpp")
        save = function_body(
            binding_cpp,
            "bool_t Client::CValtanPatternAnimationBindingDocument::Save_Atomic(",
        ).split("#if 0", 1)[0]
        self.assertIn("outCommittedSourceBytes.clear()", save)
        self.assertIn("read-only generated Product", save)
        self.assertIn("Data/Valtan/Valtan.presentation.json", save)
        for forbidden in (
            "Read_BinaryText(",
            "Make_BossPatternTemporaryPath(",
            "Commit_BossPatternTemporary(",
        ):
            self.assertNotIn(forbidden, save)

        effect_h = read("Client/Public/ValtanPatternEffectCueDocument.h")
        effect_cpp = read("Client/Private/ValtanPatternEffectCueDocument.cpp")
        self.assertIn("Load_ReadOnlyProduct", effect_h)
        self.assertIn("Load_ReadOnlyProduct", effect_cpp)
        self.assertNotIn("static bool_t Save", effect_h)

    def test_product_valtan_runtime_does_not_load_split_authoring_tree(self) -> None:
        valtan_cpp = read("Client/Private/Valtan.cpp")
        reload_effects = function_body(
            valtan_cpp,
            "bool_t CValtan::Reload_PatternEffectCues_WhileAdmitted(",
        )
        self.assertIn("CEncounterPatternReference", reload_effects)
        self.assertIn("pattern.serverMotion", reload_effects)
        for forbidden in (
            "CValtanPatternTree::Load(",
            "Valtan.gameplay.json",
            "Valtan.presentation.json",
        ):
            self.assertNotIn(forbidden, reload_effects)

    def test_stale_preserved_workbench_is_diagnostic_only(self) -> None:
        tool = read("Client/Private/Animation_Tool.cpp")
        admission = read("Client/Public/ValtanViewAdmission.h")
        for token in (
            "enum class VALTAN_VIEW_ADMISSION",
            "Can_DisplayValtanView(",
            "Can_MutateValtanView(",
        ):
            self.assertIn(token, admission)
        master = function_body(
            tool, "void Client::CAnimation_Tool::Render_ValtanPatternMaster("
        )
        for token in (
            "bMutationAdmitted",
            "Can_MutateValtanView(m_eValtanPatternMasterAdmission)",
            "READ-ONLY: Pattern data is %s",
            "nullptr == m_pBossTool || !bMutationAdmitted",
            "nullptr == m_pBalanceTool || !bMutationAdmitted",
            'ImGui::Button("Complete Play (Server/Arena)")',
            "ImGui::BeginDisabled(!bMutationAdmitted)",
        ):
            self.assertIn(token, master)

        create = function_body(
            tool,
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternCreateCommand(",
        )
        self.assertIn(
            "!Can_MutateValtanView(m_eValtanPatternMasterAdmission)", create
        )
        self.assertIn("Create stopped before writing", create)

    def test_create_apply_reports_incomplete_reload_closure_as_failure(self) -> None:
        poll = function_body(
            read("Client/Private/Animation_Tool.cpp"),
            "void Client::CAnimation_Tool::Poll_ValtanPatternCreateCommand()",
        )
        for token in (
            "bReloadClosureAdmitted",
            "bAnimationAdmitted",
            "local Product/canonical reload is INCOMPLETE",
            "Boss canonical graph/inventory reload",
        ):
            self.assertIn(token, poll)


if __name__ == "__main__":
    unittest.main()
