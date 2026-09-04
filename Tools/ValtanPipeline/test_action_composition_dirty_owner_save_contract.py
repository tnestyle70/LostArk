"""Focused source oracles for owner-scoped Composition Save participation."""

from __future__ import annotations

import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


def _function_body(source: str, signature: str, next_signature: str) -> str:
    return source.split(signature, 1)[1].split(next_signature, 1)[0]


class ActionCompositionDirtyOwnerSaveContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.workbench = (
            REPOSITORY_ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
        ).read_text(encoding="utf-8")
        cls.save = _function_body(
            cls.workbench,
            "bool_t Client::CActionCompositionWorkbench::Save_Reload()",
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar",
        )
        cls.accept = _function_body(
            cls.workbench,
            "bool_t Client::CActionCompositionWorkbench::Accept_PendingSaveOwners(",
            "bool_t Client::CActionCompositionWorkbench::Reload_AfterPendingSave(",
        )

    def test_dirty_snapshot_selects_the_only_save_participants(self) -> None:
        for token in (
            "const bool_t bSavePattern = bCurrentAuthoringDirty",
            "const bool_t bSaveSound = Is_PatternSoundDraftDirty(SoundStatus)",
            "const bool_t bSaveEffectV2 =\n"
            "\t\tCEffectV2Catalog::Get().Has_BossValtanBindingDraft()",
            "if (!bSavePattern && !bSaveSound && !bSaveEffectV2)",
        ):
            self.assertIn(token, self.save)

        no_change = self.save.index(
            "if (!bSavePattern && !bSaveSound && !bSaveEffectV2)"
        )
        pipeline_owner = self.save.index("if (nullptr == m_pBossTool)")
        self.assertLess(no_change, pipeline_owner)

    def test_pattern_and_sound_validators_are_owner_scoped(self) -> None:
        for token in (
            "if (bSavePattern && nullptr == m_pAnimationTool)",
            "if (bSaveSound && nullptr == m_pAnimationTool)",
            "if (bSaveSound && !m_pAnimationTool->\n"
            "\t\t\tCan_CommitValtanCompositionPatternSoundGeneration(SoundStatus))",
            "if (bSavePattern || bSaveSound)",
            "if (bSavePattern && !m_pAnimationTool->\n"
            "\t\t\tValidate_ValtanCompositionAnimationGraphMutations(",
            "if (bSaveSound && !m_pAnimationTool->\n"
            "\t\t\tValidate_ValtanCompositionPatternSoundGraphDependencies(",
        ):
            self.assertIn(token, self.save)
        self.assertNotIn("\n\tif (nullptr == m_pAnimationTool)\n", self.save)

    def test_sidecar_stage_and_reopen_run_only_for_dirty_owners(self) -> None:
        sound_stage = self.save.index("\tif (bSaveSound)\n\t{")
        sound_prepare = self.save.index(
            "Prepare_ValtanCompositionPatternSoundSave(", sound_stage
        )
        sound_consistency = self.save.index(
            "if (bPreparedPatternSoundDirty != bSaveSound)", sound_prepare
        )
        self.assertLess(sound_stage, sound_prepare)
        self.assertLess(sound_prepare, sound_consistency)

        effect_stage = self.save.index("\tif (bSaveEffectV2)\n\t{")
        effect_prepare = self.save.index(
            "Prepare_BossValtanBindingDraftSave(", effect_stage
        )
        effect_consistency = self.save.index(
            "if (bEffectV2Dirty != bSaveEffectV2)", effect_prepare
        )
        self.assertLess(effect_stage, effect_prepare)
        self.assertLess(effect_prepare, effect_consistency)

        self.assertIn(
            "m_bPendingPatternSoundOwner = bPreparedPatternSoundDirty", self.save
        )
        self.assertIn("m_bPendingEffectV2Owner = bEffectV2Dirty", self.save)
        self.assertIn("if (m_bPendingPatternSoundOwner)", self.accept)
        self.assertIn(
            "Accept_ValtanCompositionPatternSoundSave(", self.accept
        )
        self.assertIn("if (m_bPendingEffectV2Owner)", self.accept)
        self.assertIn("Accept_BossValtanBindingDraftSave(", self.accept)

    def test_clean_sound_owner_is_not_polled_or_reported_as_pinned(self) -> None:
        timeline = _function_body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Render_Timeline(",
            "void Client::CActionCompositionWorkbench::Render_TimelineGrid(",
        )
        self.assertIn(
            "bool_t bSoundOwnerCommitAdmitted = !m_bPatternSoundDependencyDirty",
            timeline,
        )
        self.assertIn(
            "if (m_bPatternSoundDependencyDirty && bSoundOwnerReady)", timeline
        )
        self.assertIn(
            '"Sound: %s", !m_bPatternSoundDependencyDirty ? "clean"', timeline
        )
        sound_poll = timeline.index(
            "Can_CommitValtanCompositionPatternSoundGeneration("
        )
        dirty_gate = timeline.rfind(
            "if (m_bPatternSoundDependencyDirty && bSoundOwnerReady)",
            0,
            sound_poll,
        )
        self.assertGreaterEqual(dirty_gate, 0)

    def test_unchanged_sidecars_stay_physical_read_set_dependencies(self) -> None:
        pipeline = (
            REPOSITORY_ROOT
            / "Tools/ValtanPipeline/promote_valtan_animation_chains.py"
        ).read_text(encoding="utf-8")
        commit = pipeline.split("def commit_typed_authoring_patch(", 1)[1].split(
            "def create_pattern_from_request(", 1
        )[0]
        for token in (
            "if pattern_sound_pair is None",
            "else pattern_sound_pair[1]",
            "effect_v2_physical_baseline",
            "if effect_v2_pair is None",
            "else effect_v2_pair[1]",
            "_validate_pattern_sound_dependencies_against_candidate_products(",
            "_validate_effect_v2_bindings_against_candidate_products(",
        ):
            self.assertIn(token, commit)

    def test_camera_remains_read_only_and_outside_this_save_contract(self) -> None:
        self.assertNotIn("Camera", self.save)
        self.assertIn(
            "Camera stays in its typed read-only/deep-link boundary",
            self.workbench,
        )
        animation_tool = (
            REPOSITORY_ROOT / "Client/Private/Animation_Tool.cpp"
        ).read_text(encoding="utf-8")
        self.assertIn("Camera Owner (read-only here)", animation_tool)
        self.assertIn('if (ImGui::SmallButton("Open Camera Tool"))', animation_tool)


if __name__ == "__main__":
    unittest.main()
