"""Focused contract for Effect V2 Stage/clip-occurrence runtime clocks."""

from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


class EffectV2OccurrenceRuntimeContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runtime_h = (ROOT / "Client/Public/EffectV2_Runtime.h").read_text(
            encoding="utf-8"
        )
        cls.runtime_cpp = (ROOT / "Client/Private/EffectV2_Runtime.cpp").read_text(
            encoding="utf-8"
        )
        cls.valtan_cpp = (ROOT / "Client/Private/Valtan.cpp").read_text(
            encoding="utf-8"
        )

    def test_external_group_clock_rewinds_without_double_advancing_or_losing_tails(self) -> None:
        self.assertIn("bool_t bExternalClock = false", self.runtime_h)
        self.assertIn("Group.Playback.bExternalClock && iOnlyHandle == 0u", self.runtime_cpp)
        self.assertIn("bAllSpawned && Group.Spawned.empty() && !Group.Playback.bExternalClock", self.runtime_cpp)
        sample = self.runtime_cpp.split("bool_t Client::CEffectV2Runtime::Sample_Group(", 1)[1].split(
            "bool_t Client::CEffectV2Runtime::Seek_Group(", 1)[0]
        self.assertIn("fGroupAgeSeconds + 0.000001f < Lane.fSeconds", sample)
        self.assertIn("Pending.iNextLoopEpoch = 0u", sample)
        self.assertIn("Lane.Playback.PivotSampler(fGroupAgeSeconds", sample)
        self.assertIn("DrainAtStart ? StopAge : fInitialElapsedSeconds", self.runtime_cpp)
        self.assertIn("pObject->Stop_Emission()", self.runtime_cpp)
        self.assertIn("fInitialElapsedSeconds > 0.f || (pPlayback && pPlayback->bExternalClock)", self.runtime_cpp)
        self.assertIn("Object->Set_PlaybackPaused(true)", self.runtime_cpp)

    def test_mesh_particle_birth_samples_history_and_keeps_an_independent_body_clock(self) -> None:
        source = (ROOT / "Client/Private/EffectV2_Object.cpp").read_text(encoding="utf-8")
        spawn = source.split("void Client::CEffectV2Object::Spawn_Particle(", 1)[1].split(
            "void Client::CEffectV2Object::Advance_ParticleBodies(", 1)[0]
        self.assertIn("m_PivotSampler(fBirthElapsedSeconds, Pivot, m_strStatus)", spawn)
        self.assertIn("if (!P.bLocalSpace)", spawn)
        self.assertIn("Particle birth pivot unavailable", spawn)
        clock = source.split("void Client::CEffectV2Object::Advance_ParticleClock(", 1)[1].split(
            "HRESULT Client::CEffectV2Object::Build_ParticleInstances(", 1)[0]
        self.assertIn("constexpr double FixedStep = 1.0 / 60.0", clock)
        self.assertIn("UntilEnd <= Remaining", clock)
        self.assertIn("Update_Particles(static_cast<f32_t>(Chunk))", clock)
        self.assertIn("m_bEmissionStopped) { Advance_ParticleBodies(fStep); return; }", clock)
        self.assertIn("m_dParticleSeconds + Birth", clock)
        self.assertIn("Advance_ParticleClock(0.f, true)", source)
        self.assertIn("Particle.dAge + m_dParticleRemainder", source)
        self.assertIn("m_iRandomState = (std::max)(1u, m_Params.Particle.iRandomSeed)", source)
        self.assertIn("Requested root time has not been recorded", self.runtime_cpp)
        self.assertIn("if (Right->bDiscontinuity)", self.runtime_cpp)

    def test_kouku_product_resolves_selected_effect_after_save_and_uses_one_clock(self) -> None:
        source = (ROOT / "Client/Private/KoukuSaydonPresentationPlayer.cpp").read_text(encoding="utf-8")
        selected = source.split("bool Client::CKoukuSaydonPresentationPlayer::Ensure_EffectResource(", 1)[1].split(
            "void Client::CKoukuSaydonPresentationPlayer::Stop_Session(", 1)[0]
        self.assertIn("CEffectV2Runtime::Cache_Generation()", selected)
        self.assertIn("Load_ResourceSnapshot(resourceKind, asset, staged, m_strStatus)", selected)
        self.assertIn("m_EffectResourceFailures.emplace(key, m_strStatus)", selected)
        self.assertNotIn("Reload_BossValtan", source)
        self.assertIn("playback.bExternalClock = true", source)
        self.assertIn("Sample_Group(row.effectHandle, age, paused", source)
        self.assertIn("history->Sample(resolved ? seconds : box.iStartMs / 1000.f + seconds", source)
        self.assertIn("leaf.eType == EFFECT_V2_TYPE::PARTICLE", source)
        self.assertIn("++g_iCacheGeneration", self.runtime_cpp)

    def test_runtime_consumes_a_typed_occurrence_wall_map(self) -> None:
        for token in (
            "EFFECT_V2_CLIP_OCCURRENCE_CLOCK",
            "fStageWallStartSeconds",
            "fSourceStartSeconds",
            "fSourceDurationSeconds",
            "fLoopWallDurationSeconds",
            "fPlaybackRate",
            "bLoop",
        ):
            self.assertIn(token, self.runtime_h)
        self.assertIn("Find_OccurrenceClock", self.runtime_cpp)
        self.assertIn("Resolve_StageSpawnClock", self.runtime_cpp)
        self.assertIn("Pending.Binding.strClipOccurrenceId", self.runtime_cpp)

    def test_shutdown_releases_retained_lanes_before_gpu_caches_after_loader_join(self) -> None:
        source = (ROOT / "Client/Private/MainApp.cpp").read_text(encoding="utf-8")
        shutdown = source.split("void CMainApp::Free()", 1)[1]
        release = self.runtime_cpp.split(
            "void Client::CEffectV2Runtime::Release_Resources()", 1
        )[1].split("uint64_t Client::CEffectV2Runtime::Cache_Generation()", 1)[0]
        self.assertLess(
            shutdown.index("CGameInstance::Get().Release_Engine()"),
            shutdown.index("CEffectV2Runtime::Release_Resources()"),
        )
        for owner in (
            "g_TargetStates", "g_IgnoredTargets", "g_FreeGroups",
            "g_FreeGroupTerminalFailures",
        ):
            self.assertLess(
                release.index(f"{owner}.clear()"),
                release.index("CEffectV2Object::Clear_ResourceCache()"),
            )
        self.assertIn("Invalidate_Caches()", release)
        self.assertIn("g_bPrototypeRegistered = false", release)

    def test_each_loop_is_epoch_driven_and_bounded(self) -> None:
        for token in (
            "iNextLoopEpoch",
            "EFFECT_V2_REPEAT_POLICY::EACH_LOOP",
            "MAX_EPOCHS_PER_SYNC = 256u",
            "Resolve_LastDueLoopEpoch",
            "iLastDueEpoch",
        ):
            self.assertIn(token, self.runtime_cpp)
        self.assertNotIn("bool_t bSpawned", self.runtime_cpp)
        self.assertIn("EACH_LOOP requires a looping clip occurrence", self.runtime_cpp)

    def test_occurrence_stop_and_late_snapshot_are_explicit(self) -> None:
        self.assertIn(
            "EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END", self.runtime_cpp
        )
        self.assertIn("fOccurrenceEnd", self.runtime_cpp)
        self.assertIn("fAgeSeconds - Clock.fStartSeconds", self.runtime_cpp)
        self.assertIn("Reset_StageLane(State)", self.runtime_cpp)

    def test_valtan_builds_clocks_from_the_same_animation_chain(self) -> None:
        for token in (
            "Build_EffectV2OccurrenceClocks(",
            "Build_PatternTimeline(pModel, Clips, Timings)",
            "Clips[iClip].strClipOccurrenceId",
            "Build_EffectV2OccurrenceClocksForAction(",
            "m_LocalPreviewClipByActionId : m_PatternClipByActionId",
            "m_PatternClipByActionId,",
            "EffectV2Clocks,",
        ):
            self.assertIn(token, self.valtan_cpp)

    def test_every_each_loop_binding_joins_one_looping_occurrence(self) -> None:
        bindings = json.loads(
            (
                ROOT
                / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"
            ).read_text(encoding="utf-8")
        )["bindings"]
        animation = json.loads(
            (
                ROOT
                / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
            ).read_text(encoding="utf-8")
        )["bindings"]
        by_action = {row["actionId"]: row for row in animation}
        each_loop = [
            row
            for row in bindings
            if row["clock"]["repeatPolicy"] == "EACH_LOOP"
        ]
        self.assertGreater(len(each_loop), 0)
        for row in each_loop:
            action = by_action[row["scope"]["actionId"]]
            matches = [
                clip
                for clip in action["clips"]
                if clip["clipOccurrenceId"] == row["clock"]["clipOccurrenceId"]
            ]
            self.assertEqual(1, len(matches), row["bindingId"])
            self.assertTrue(matches[0]["loop"], row["bindingId"])


if __name__ == "__main__":
    unittest.main()
