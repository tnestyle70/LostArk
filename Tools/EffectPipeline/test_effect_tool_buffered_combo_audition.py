import json
import unittest
from pathlib import Path


class EffectToolBufferedComboAuditionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = Path(__file__).resolve().parents[2]
        cls.header = (
            cls.repository_root / "Client/Public/Effect_Tool.h"
        ).read_text(encoding="utf-8")
        cls.source = (
            cls.repository_root / "Client/Private/Effect_Tool.cpp"
        ).read_text(encoding="utf-8")
        cls.player_skills = json.loads(
            (cls.repository_root / "Data/Balance/PlayerSkills.json").read_text(
                encoding="utf-8"
            )
        )["skills"]
        cls.artist_bindings = json.loads(
            (
                cls.repository_root
                / "Data/Animation/Authored/Artist/Artist.skillbindings.json"
            ).read_text(encoding="utf-8")
        )["bindings"]
        cls.dimensionmaster_bindings = json.loads(
            (
                cls.repository_root
                / "Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json"
            ).read_text(encoding="utf-8")
        )["bindings"]
        cls.artist_r_ba4_effect = json.loads(
            (
                cls.repository_root
                / "Data/Effects/Authored/effect.artist.skill.31210.ba4.unified.effect.json"
            ).read_text(encoding="utf-8")
        )

    def test_combo_skill_root_exposes_explicit_authoring_audition(self) -> None:
        self.assertIn('ImGui::Button("Play Buffered Combo Audition")', self.source)
        self.assertIn("Try_PlayBufferedComboAudition(*pProductEntry)", self.source)
        self.assertIn("PLAYER_SKILL_KIND::COMBO", self.source)
        self.assertIn("m_bBufferedComboAuditionActive", self.header)

    def test_server_buffer_boundaries_are_not_inferred_from_effect_lifetime(self) -> None:
        builder_start = self.source.index(
            "bool_t Client::CEffect_Tool::Try_BuildBufferedComboAnimationClips("
        )
        builder = self.source[
            builder_start : self.source.index(
                "bool_t Client::CEffect_Tool::Try_PlayBufferedComboAudition(",
                builder_start,
            )
        ]
        self.assertIn("ServerTiming.iActionDurationMs", builder)
        self.assertIn("ServerTiming.iComboAdvanceMs", builder)
        self.assertIn("bFinalStage ?", builder)
        self.assertIn("Clip.iSourceStartMs", builder)
        self.assertIn("Clip.fPlayRate", builder)
        self.assertIn("Trimmed.iPlayMs", builder)

    def test_repeated_clip_names_remain_ordered_occurrences(self) -> None:
        builder_start = self.source.index(
            "bool_t Client::CEffect_Tool::Try_BuildBufferedComboAnimationClips("
        )
        builder = self.source[
            builder_start : self.source.index(
                "bool_t Client::CEffect_Tool::Try_PlayBufferedComboAudition(",
                builder_start,
            )
        ]
        self.assertIn("StagedClips.emplace_back(Clip);", builder)
        self.assertIn("StagedClips.push_back(std::move(Trimmed));", builder)
        self.assertNotIn("std::unique", builder)
        self.assertNotIn("unordered_set", builder)

    def test_server_stage_can_hold_the_authored_final_pose(self) -> None:
        self.assertIn("fHoldAfterSeconds", self.header)
        self.assertIn(
            "StagedClips.back().fHoldAfterSeconds += fRemainingWallSeconds",
            self.source,
        )
        self.assertIn(
            "fWallDurationSeconds + Clip.fHoldAfterSeconds", self.source
        )
        self.assertIn(
            "Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds)",
            self.source,
        )

    def test_exact_product_play_remains_a_separate_mode(self) -> None:
        product_play = self.source[
            self.source.index('ImGui::Button("Play Full Effect")') :
            self.source.index(
                "Render_VisualProgramAuthoring(*pProductEntry, iCue)",
                self.source.index('ImGui::Button("Play Full Effect")'),
            )
        ]
        self.assertIn("Try_SelectProductCue(*pProductEntry, iCue)", product_play)
        select_product = self.source[
            self.source.index("bool_t Client::CEffect_Tool::Try_SelectProductCue(") :
            self.source.index("bool_t Client::CEffect_Tool::Try_PlayVisualProgramFamily(")
        ]
        self.assertIn("Reset_BufferedComboAudition();", select_product)
        self.assertIn("exact Product cue playback was restored", self.source)

    def test_failed_buffered_audition_restores_the_previous_preview_transaction(self) -> None:
        audition_start = self.source.index(
            "bool_t Client::CEffect_Tool::Try_PlayBufferedComboAudition("
        )
        audition = self.source[
            audition_start : self.source.index(
                "bool_t Client::CEffect_Tool::Try_SelectProductCue(",
                audition_start,
            )
        ]
        snapshot = audition.index("strPreviousTargetAsset")
        build = audition.index("Try_BuildBufferedComboAnimationClips(")
        self.assertLess(snapshot, build)
        self.assertIn("RestoreBufferedComboRollback", audition)
        self.assertGreaterEqual(
            audition.count("RestoreBufferedComboRollback("), 3
        )
        self.assertIn(
            "RestoreBufferedComboRollback(false, RollbackError)", audition
        )
        self.assertGreaterEqual(
            audition.count("RestoreBufferedComboRollback(true, RollbackError)"), 2
        )
        for preserved_state in (
            "PreviousProductPreview",
            "fPreviousPreviewTimeSeconds",
            "bPreviousPreviewPlaying",
            "Select_TargetAsset(\n\t\t\t\t\tstrPreviousTargetAsset)",
            "Stage_WorldPreview(",
            "Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds)",
            "bPreviousReconstructedSourceRuntimeActive",
            "Try_StartArtist31470FullPreview()",
            "Seek_ReconstructedSourceRuntimeTimeline(",
            "PreviousPreviewSubmissionIsolation",
            "fPreviousProductCueActionFacingYawDegrees",
            "ePreviousAllEffectsClass",
            "strPreviousAuthoringDomainId",
        ):
            self.assertIn(preserved_state, audition)

    def test_selected_product_occurrence_uses_its_server_chain_offset(self) -> None:
        dimensionmaster = next(
            skill for skill in self.player_skills if skill["skillId"] == 2050010
        )
        artist = next(
            skill for skill in self.player_skills if skill["skillId"] == 31210
        )
        dimensionmaster_binding = next(
            binding
            for binding in self.dimensionmaster_bindings
            if binding["skillId"] == 2050010
        )
        self.assertEqual(
            dimensionmaster_binding["clips"][2],
            ["pc_sp_m_00_sk_att_battle_1_03"],
        )
        self.assertEqual(
            sum(
                stage["comboAdvanceMs"]
                for stage in dimensionmaster["comboStages"][:2]
            ),
            545,
        )
        self.assertEqual(
            sum(stage["comboAdvanceMs"] for stage in artist["comboStages"][:2]),
            1200,
        )
        self.assertIn(
            "m_fBufferedComboAuditionOccurrenceOffsetSeconds", self.header
        )
        for function_name in (
            "f32_t Client::CEffect_Tool::Resolve_EffectSampleTime(",
            "f32_t Client::CEffect_Tool::Resolve_EffectTimelineTime(",
            "bool_t Client::CEffect_Tool::Is_ProductCueVisible(",
            "void Client::CEffect_Tool::Recalculate_PreviewDuration(\n    const EFFECT_DOCUMENT_DESC& Document)",
        ):
            function_start = self.source.index(function_name)
            function_body = self.source[function_start : function_start + 5000]
            self.assertIn(
                "m_fBufferedComboAuditionOccurrenceOffsetSeconds", function_body
            )

    def test_buffered_final_pose_releases_the_clock_for_the_effect_tail(self) -> None:
        artist = next(
            skill for skill in self.player_skills if skill["skillId"] == 31210
        )
        combo_seconds = sum(
            stage["comboAdvanceMs"] for stage in artist["comboStages"][:-1]
        ) / 1000.0 + artist["comboStages"][-1]["actionDurationMs"] / 1000.0
        visible_ends = [
            element["detail"]["timing"]["startDelaySeconds"]
            + element["detail"]["timing"]["lifeTimeSeconds"]
            for element in self.artist_r_ba4_effect["elements"]
            if element["visible"]
        ]
        self.assertAlmostEqual(combo_seconds, 1.733, places=3)
        self.assertGreater(max(visible_ends), combo_seconds)
        self.assertIn(
            "m_bBufferedComboAuditionActive && bLastClip", self.source
        )
        self.assertIn("bBufferedFinalClip", self.source)

    def test_action_facing_is_captured_only_when_the_occurrence_is_visible(self) -> None:
        root_start = self.source.index(
            "bool_t Client::CEffect_Tool::Resolve_PreviewRoot("
        )
        root_body = self.source[
            root_start : self.source.index(
                "bool_t Client::CEffect_Tool::Has_ProductCuePreview() const",
                root_start,
            )
        ]
        self.assertIn("const bool_t bCueVisible", root_body)
        self.assertIn("if (bCueVisible)", root_body)
        self.assertLess(
            root_body.index("if (bCueVisible)"),
            root_body.index("m_bProductCueActionFacingCaptured = true"),
        )

    def test_artist_r_is_three_server_owned_input_stages(self) -> None:
        skill = next(skill for skill in self.player_skills if skill["skillId"] == 31210)
        self.assertEqual(skill["characterClass"], "ARTIST")
        self.assertEqual(skill["inputSlot"], "R")
        self.assertEqual(skill["skillKind"], "COMBO")
        self.assertEqual(
            [stage["comboAdvanceMs"] for stage in skill["comboStages"]],
            [600, 600, 533],
        )

    def test_artist_r_buffered_chain_keeps_recovery_only_on_stage_one(self) -> None:
        binding = next(
            binding for binding in self.artist_bindings if binding["skillId"] == 31210
        )
        self.assertEqual(
            binding["clips"],
            [
                ["sdm_sk_skykongkong_01", "sdm_sk_skykongkong_03"],
                ["sdm_sk_skykongkong_01"],
                ["sdm_sk_skykongkong_02"],
            ],
        )
        # The explicit buffered audition cuts the first stage at 600 ms, before
        # its no-next-input recovery clip, so R/R/R yields three attack rows.
        self.assertIn("ServerTiming.iComboAdvanceMs", self.source)


if __name__ == "__main__":
    unittest.main()
