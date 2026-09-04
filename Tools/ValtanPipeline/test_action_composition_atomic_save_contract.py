"""Focused oracles for the Action Composition single-Save transaction."""

from __future__ import annotations

import sys
import tempfile
import unittest
import json
from pathlib import Path


TOOLS_ROOT = Path(__file__).resolve().parent
REPOSITORY_ROOT = TOOLS_ROOT.parents[1]
sys.path.insert(0, str(TOOLS_ROOT))

import promote_valtan_animation_chains as promotion  # noqa: E402


class ActionCompositionAtomicSaveContractTests(unittest.TestCase):
    def test_workbench_uses_one_save_edge_and_keeps_v2_edits_draft(self) -> None:
        source = (
            REPOSITORY_ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
        ).read_text(encoding="utf-8")
        save = source.split(
            "bool_t Client::CActionCompositionWorkbench::Save_Reload()", 1
        )[1].split(
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar", 1
        )[0]
        accept = source.split(
            "bool_t Client::CActionCompositionWorkbench::Accept_PendingSaveOwners(",
            1,
        )[1].split(
            "bool_t Client::CActionCompositionWorkbench::Reload_AfterPendingSave(",
            1,
        )[0]
        self.assertEqual(save.count("Begin_ValtanCompositionSave("), 1)
        self.assertNotIn("Save_ValtanProduct", save)
        self.assertNotIn("Save_ValtanCompositionProduct", save)
        self.assertNotIn("Save_ValtanCompositionPatternSounds", save)
        self.assertIn("Prepare_ValtanCompositionPatternSoundSave", save)
        self.assertIn("Prepare_BossValtanBindingDraftSave", save)
        self.assertIn("EffectV2ReadSetBytes", save)
        self.assertIn("OwnerDrafts.effectV2ReadSetBytes", save)
        self.assertIn("Accept_ValtanCompositionPatternSoundSave", accept)
        self.assertIn("Accept_BossValtanBindingDraftSave", accept)
        self.assertIn("Accept_PendingSaveOwners(LocalOwnerStatus)", source)

        for staged_api in (
            "Stage_AppendBossValtanStageBinding",
            "Stage_RemoveBossValtanStageBinding",
            "Stage_DuplicateBossValtanStageBinding",
            "Stage_UpdateBossValtanStageBindingStart",
        ):
            self.assertIn(staged_api, source)

    def test_save_participants_are_selected_by_owner_dirty_state(self) -> None:
        source = (
            REPOSITORY_ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
        ).read_text(encoding="utf-8")
        save = source.split(
            "bool_t Client::CActionCompositionWorkbench::Save_Reload()", 1
        )[1].split(
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar", 1
        )[0]
        accept = source.split(
            "bool_t Client::CActionCompositionWorkbench::Accept_PendingSaveOwners(",
            1,
        )[1].split(
            "bool_t Client::CActionCompositionWorkbench::Reload_AfterPendingSave(",
            1,
        )[0]

        self.assertIn(
            "const bool_t bSaveSound = Is_PatternSoundDraftDirty(SoundStatus)",
            save,
        )
        self.assertIn(
            "const bool_t bSaveEffectV2 =\n"
            "\t\tCEffectV2Catalog::Get().Has_BossValtanBindingDraft()",
            save,
        )
        self.assertIn(
            "if (bSaveSound && !m_pAnimationTool->\n"
            "\t\t\tCan_CommitValtanCompositionPatternSoundGeneration(SoundStatus))",
            save,
        )

        sound_stage_start = save.index(
            "\tif (bSaveSound)\n\t{",
            save.index("std::string PatternSoundBaselineBytes"),
        )
        sound_stage_end = save.index(
            "\tif (bPreparedPatternSoundDirty != bSaveSound)", sound_stage_start
        )
        self.assertIn(
            "Prepare_ValtanCompositionPatternSoundSave(",
            save[sound_stage_start:sound_stage_end],
        )

        effect_stage_start = save.index(
            "\tif (bSaveEffectV2)\n\t{",
            save.index("std::string EffectV2BaselineBytes"),
        )
        effect_stage_end = save.index(
            "\tif (bEffectV2Dirty != bSaveEffectV2)", effect_stage_start
        )
        self.assertIn(
            "Prepare_BossValtanBindingDraftSave(",
            save[effect_stage_start:effect_stage_end],
        )
        self.assertIn(
            "m_bPendingPatternSoundOwner = bPreparedPatternSoundDirty", save
        )
        self.assertIn("m_bPendingEffectV2Owner = bEffectV2Dirty", save)
        self.assertIn("if (m_bPendingPatternSoundOwner)", accept)
        self.assertIn("Accept_ValtanCompositionPatternSoundSave(", accept)
        self.assertIn("if (m_bPendingEffectV2Owner)", accept)
        self.assertIn("Accept_BossValtanBindingDraftSave(", accept)
        self.assertNotIn("Camera", save)

    def test_all_owner_sidecars_reach_the_shared_generation_commit(self) -> None:
        balance = (REPOSITORY_ROOT / "Client/Private/BalanceTool.cpp").read_text(
            encoding="utf-8"
        )
        balance_header = (
            REPOSITORY_ROOT / "Client/Public/BalanceTool.h"
        ).read_text(encoding="utf-8")
        catalog = (
            REPOSITORY_ROOT / "Client/Private/EffectV2_Catalog.cpp"
        ).read_text(encoding="utf-8")
        save_job = (
            REPOSITORY_ROOT
            / "Tools/ValtanPipeline/Run-ValtanAuthoringSaveJob.ps1"
        ).read_text(encoding="utf-8")
        wrapper = (
            REPOSITORY_ROOT
            / "Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1"
        ).read_text(encoding="utf-8")
        cli = (
            REPOSITORY_ROOT / "Tools/ValtanPipeline/valtan_tuning_pipeline.py"
        ).read_text(encoding="utf-8")
        promote = (
            REPOSITORY_ROOT
            / "Tools/ValtanPipeline/promote_valtan_animation_chains.py"
        ).read_text(encoding="utf-8")
        for token in (
            "PatternSoundBaselinePath",
            "PatternSoundCandidatePath",
            "EffectV2BaselinePath",
            "EffectV2CandidatePath",
            "EffectV2ReadSetPath",
        ):
            self.assertIn(token, balance)
            self.assertIn(token, wrapper)
            self.assertIn(token, save_job)
        for token in (
            "--pattern-sound-baseline",
            "--pattern-sound-candidate",
            "--effect-v2-baseline",
            "--effect-v2-candidate",
        ):
            self.assertIn(token, wrapper)
            self.assertIn(token, cli)
        self.assertIn("EffectV2ReadSetPath", wrapper)
        self.assertIn("--effect-v2-read-set", wrapper)
        self.assertIn("--effect-v2-read-set", cli)
        self.assertIn("effectV2ReadSetBytes", balance_header)
        self.assertIn("effectV2ReadSetBytes", balance)
        self.assertIn("m_ResourceReadRows", catalog)
        self.assertIn("ReadSetDocument", catalog)
        self.assertIn("Reload before Save", catalog)
        self.assertNotIn("effect_v2_binding_pipeline.py", save_job)
        self.assertNotIn("'snapshot'", save_job)
        self.assertIn("provided_baselines", promote)
        self.assertIn("target_payloads[pattern_sound_target]", promote)
        self.assertIn("target_payloads[effect_v2_target]", promote)
        self.assertIn(
            "_validate_effect_v2_bindings_against_candidate_products", promote
        )
        typed_commit = promote.split("def commit_typed_authoring_patch(", 1)[1].split(
            "def create_pattern_from_request(", 1
        )[0]
        ordered_read_set_cas = (
            "with _exclusive_transaction_lock(",
            "_validate_effect_v2_bindings_against_candidate_products(",
            "assert_resource_read_set_current(",
            "target_payloads: dict[Path, bytes]",
            "_atomic_commit(",
        )
        positions = [typed_commit.index(token) for token in ordered_read_set_cas]
        self.assertEqual(positions, sorted(positions))
        self.assertIn("Reload before Save", typed_commit)
        self.assertIn("effect_v2_effective_bytes", typed_commit)
        self.assertIn(
            "_read_bytes_or_none(effect_v2_target) != effect_v2_physical_baseline",
            typed_commit,
        )
        self.assertLess(
            typed_commit.index("effect_v2_effective_bytes"),
            typed_commit.index("target_payloads: dict[Path, bytes]"),
        )

    def test_effect_v2_draft_cas_preserves_the_physical_owner_bytes(self) -> None:
        source = (
            REPOSITORY_ROOT / "Client/Private/EffectV2_Catalog.cpp"
        ).read_text(encoding="utf-8")
        stage = source.split("bool_t Stage_BossValtanBindings(", 1)[1].split(
            "bool_t Cross_Validate(", 1
        )[0]
        commit = source.split(
            "bool_t Client::CEffectV2Catalog::Commit_BossValtanBindingsLocked(",
            1,
        )[1].split(
            "bool_t Client::CEffectV2Catalog::Mutate_BossValtanStageBinding(",
            1,
        )[0]
        prepare = source.split(
            "bool_t Client::CEffectV2Catalog::Prepare_BossValtanBindingDraftSave(",
            1,
        )[1].split(
            "bool_t Client::CEffectV2Catalog::Accept_BossValtanBindingDraftSave(",
            1,
        )[0]

        for token in (
            "std::string* const pOutSourceBytes = nullptr",
            "*pOutSourceBytes = std::move(strText)",
        ):
            self.assertIn(token, stage)
        self.assertIn("&strDiskSourceBytes", commit)
        self.assertIn(
            "strDiskSourceBytes == m_strBossValtanBindingDraftBaselineBytes",
            commit,
        )
        self.assertIn("std::move(strDiskSourceBytes)", commit)
        self.assertIn("&strDiskSourceBytes", prepare)
        self.assertIn(
            "strDiskSourceBytes != m_strBossValtanBindingDraftBaselineBytes",
            prepare,
        )
        self.assertNotIn(
            "Serialize_Bindings(\n\t\t\tBOSS_VALTAN_ARCHETYPE_ID, DiskBindings) !=\n"
            "\t\t\tm_strBossValtanBindingDraftBaselineBytes",
            prepare,
        )

    def test_canonical_save_uses_one_bounded_lock_wait_contract(self) -> None:
        balance = (REPOSITORY_ROOT / "Client/Private/BalanceTool.cpp").read_text(
            encoding="utf-8"
        )
        wrapper = (
            REPOSITORY_ROOT
            / "Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1"
        ).read_text(encoding="utf-8")
        pipeline = (
            REPOSITORY_ROOT / "Tools/ValtanPipeline/valtan_tuning_pipeline.py"
        ).read_text(encoding="utf-8")
        promote = (
            REPOSITORY_ROOT
            / "Tools/ValtanPipeline/promote_valtan_animation_chains.py"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "VALTAN_CANONICAL_SAVE_LOCK_TIMEOUT_SECONDS = 30u", balance
        )
        self.assertIn('L" -LockTimeoutSeconds "', balance)
        self.assertIn("[double]$LockTimeoutSeconds = 30.0", wrapper)
        self.assertIn("'--lock-timeout-seconds'", wrapper)

        typed_commit_signature = promote.split(
            "def commit_typed_authoring_patch(", 1
        )[1].split(") -> dict[str, Any]:", 1)[0]
        self.assertIn("lock_timeout_seconds: float = 0.0", typed_commit_signature)
        self.assertIn(
            'canonical_parser.add_argument("--lock-timeout-seconds", '
            "type=float, default=0.0)",
            pipeline,
        )

    def test_shared_commit_rolls_pattern_sound_and_v2_back_byte_exactly(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary).resolve()
            targets = {
                root / "Data/Valtan/Valtan.presentation.json": b"pattern-baseline\n",
                root
                / "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json": b"sound-baseline\n",
                root
                / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json": b"v2-baseline\n",
            }
            for path, baseline in targets.items():
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(baseline)
            candidates = {
                path: baseline.replace(b"baseline", b"candidate")
                for path, baseline in targets.items()
            }
            with self.assertRaises(promotion.PromotionError):
                promotion._atomic_commit(
                    candidates,
                    inject_failure_after=2,
                    expected_baselines=targets,
                    repository_root=root,
                )
            for path, baseline in targets.items():
                self.assertEqual(baseline, path.read_bytes())
            self.assertFalse(
                (root / promotion.TRANSACTION_ACTIVE_REL).exists(),
                "rolled-back transaction must not leave an active generation pointer",
            )

    def test_current_sound_and_v2_candidates_join_the_projected_closure(self) -> None:
        outputs = promotion.validate_and_project(
            REPOSITORY_ROOT,
            promotion._read_json(REPOSITORY_ROOT / promotion.GAMEPLAY_REL),
            promotion._read_json(REPOSITORY_ROOT / promotion.PRESENTATION_REL),
        )
        promotion._validate_pattern_sound_dependencies_against_candidate_products(
            REPOSITORY_ROOT,
            outputs,
            sound_source_bytes=(
                REPOSITORY_ROOT / promotion.PATTERN_SOUND_REL
            ).read_bytes(),
        )
        gameplay = promotion._read_json(REPOSITORY_ROOT / promotion.GAMEPLAY_REL)
        promotion._validate_effect_v2_bindings_against_candidate_products(
            REPOSITORY_ROOT,
            outputs,
            (
                REPOSITORY_ROOT / promotion.EFFECT_V2_BINDINGS_REL
            ).read_bytes(),
            gameplay,
        )

        legacy_v2 = promotion._read_json(
            REPOSITORY_ROOT / promotion.EFFECT_V2_BINDINGS_REL
        )
        legacy_v2["formatVersion"] = 1
        with self.assertRaisesRegex(
            promotion.PromotionError, "formatVersion 2.*explicit legacy migration"
        ):
            promotion._validate_effect_v2_bindings_against_candidate_products(
                REPOSITORY_ROOT,
                outputs,
                json.dumps(legacy_v2, ensure_ascii=False).encode("utf-8"),
                gameplay,
            )

        sound_path = REPOSITORY_ROOT / promotion.PATTERN_SOUND_REL
        sound_baseline = sound_path.read_bytes()
        malformed_sound = json.loads(sound_baseline)
        malformed_sound["cues"][0]["patternId"] = "VALTAN_MISSING_PATTERN"
        with self.assertRaises(promotion.PromotionError):
            promotion._validate_pattern_sound_dependencies_against_candidate_products(
                REPOSITORY_ROOT,
                outputs,
                sound_source_bytes=(
                    json.dumps(malformed_sound, ensure_ascii=False).encode("utf-8")
                ),
            )
        self.assertEqual(sound_baseline, sound_path.read_bytes())

        v2_path = REPOSITORY_ROOT / promotion.EFFECT_V2_BINDINGS_REL
        v2_baseline = v2_path.read_bytes()
        malformed_v2 = json.loads(v2_baseline)
        malformed_v2["bindings"][0]["startMs"] = 600001
        with self.assertRaises(promotion.PromotionError):
            promotion._validate_effect_v2_bindings_against_candidate_products(
                REPOSITORY_ROOT,
                outputs,
                json.dumps(malformed_v2, ensure_ascii=False).encode("utf-8"),
                gameplay,
            )
        self.assertEqual(v2_baseline, v2_path.read_bytes())


if __name__ == "__main__":
    unittest.main()
