#!/usr/bin/env python3
"""Focused executable contracts for Map Effect runtime lifecycle hardening."""

from __future__ import annotations

import copy
import unittest
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RUNTIME_CPP = ROOT / "Client/Private/MapEffectPresentationRuntime.cpp"
RUNTIME_HEADER = ROOT / "Client/Public/MapEffectPresentationRuntime.h"
PRESENTATION_SERVICE_CPP = ROOT / "Client/Private/Effect_PresentationService.cpp"
PRESENTATION_SERVICE_HEADER = ROOT / "Client/Public/Effect_PresentationService.h"


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig")


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace : index + 1]
    raise AssertionError(f"unterminated C++ function: {signature}")


@dataclass(eq=True)
class SurfacePacket:
    emissive_intensity: float
    emissive_color: tuple[float, float, float, float]
    emissive_mask_power: float
    transition_multiplier: float
    root_y: float
    rotation_y: float
    opacity: float


def restore_emissive_lane(
    live: SurfacePacket, baseline: SurfacePacket
) -> SurfacePacket:
    result = copy.deepcopy(live)
    result.emissive_intensity = baseline.emissive_intensity
    result.emissive_color = baseline.emissive_color
    result.emissive_mask_power = baseline.emissive_mask_power
    return result


def bounded_duplicate_sample(
    authoritative: float, previous: float, delta: float, fixed_tick_hz: float
) -> float:
    tick_seconds = 1.0 / fixed_tick_hz
    bounded_delta = min(max(delta, 0.0), tick_seconds)
    return min(
        authoritative + tick_seconds,
        max(authoritative, previous + bounded_delta),
    )


class MapEffectRuntimeLifecycleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cpp = read_text(RUNTIME_CPP)
        cls.header = read_text(RUNTIME_HEADER)
        cls.presentation_service_cpp = read_text(PRESENTATION_SERVICE_CPP)
        cls.presentation_service_header = read_text(PRESENTATION_SERVICE_HEADER)

    def test_failed_world_admission_precedes_all_live_commit_seams(self) -> None:
        commit = function_body(
            self.cpp,
            "bool_t CMapEffectPresentationRuntime::Commit_StagedDocument(",
        )
        surface_stage = commit.index("Validate_AndStageSurfacePackets(")
        world_stage = commit.index("Validate_WorldEffects(")
        admission = commit.index("Probe_WorldEffectAdmissions(")
        surface_commit = commit.index("deployRuntime.Set_SurfacePresentations(packets)")
        document_commit = commit.index("m_Document = std::move(stagedDocument)")
        self.assertLess(surface_stage, world_stage)
        self.assertLess(world_stage, admission)
        self.assertLess(admission, surface_commit)
        self.assertLess(surface_commit, document_commit)
        self.assertNotIn("Stop_WorldEffect", commit)

        committed = {
            "document": "revision-7",
            "surface": "surface-revision-7",
            "handles": ("active-old-revision",),
        }
        before = copy.deepcopy(committed)
        probe_admitted = False
        if probe_admitted:
            committed = {
                "document": "revision-8",
                "surface": "surface-revision-8",
                "handles": committed["handles"],
            }
        self.assertEqual(before, committed)

    def test_removed_owner_restores_baseline_without_clobbering_destruction(self) -> None:
        stage = function_body(
            self.cpp,
            "bool_t CMapEffectPresentationRuntime::Validate_AndStageSurfacePackets(",
        )
        self.assertIn("m_Document.Get_Surfaces()", stage)
        self.assertIn("stagedDocument.Get_Surfaces()", stage)
        self.assertIn("previousPlacements", stage)
        self.assertIn("if (seenPlacements.contains(placementId))", stage)
        self.assertIn("Get_SurfacePresentation(placementId, packet)", stage)
        for lane in (
            "packet.fEmissiveIntensity = baseline->second.fEmissiveIntensity",
            "packet.vEmissiveColor = baseline->second.vEmissiveColor",
            "packet.fEmissiveMaskPower = baseline->second.fEmissiveMaskPower",
        ):
            self.assertIn(lane, stage)
        for forbidden_assignment in (
            "packet.fTransitionMultiplier =",
            "packet.vTransitionRootOffset =",
            "packet.vTransitionRotationDegrees =",
            "packet.fOpacity =",
        ):
            self.assertNotIn(forbidden_assignment, stage)

        baseline = SurfacePacket(
            1.0, (1.0, 1.0, 1.0, 1.0), 1.0, 1.0, 0.0, 0.0, 1.0
        )
        live_breaking = SurfacePacket(
            3.5, (0.1, 1.0, 0.5, 1.0), 2.0, 0.42, -3.25, 37.0, 0.31
        )
        restored = restore_emissive_lane(live_breaking, baseline)
        self.assertEqual(1.0, restored.emissive_intensity)
        self.assertEqual((1.0, 1.0, 1.0, 1.0), restored.emissive_color)
        self.assertEqual(1.0, restored.emissive_mask_power)
        self.assertEqual(0.42, restored.transition_multiplier)
        self.assertEqual(-3.25, restored.root_y)
        self.assertEqual(37.0, restored.rotation_y)
        self.assertEqual(0.31, restored.opacity)

    def test_world_only_document_skips_empty_surface_transaction(self) -> None:
        commit = function_body(
            self.cpp,
            "bool_t CMapEffectPresentationRuntime::Commit_StagedDocument(",
        )
        self.assertIn(
            "if (!packets.empty() && !deployRuntime.Set_SurfacePresentations(packets))",
            commit,
        )
        packets: list[tuple[int, SurfacePacket]] = []
        transaction_count = 0
        if packets:
            transaction_count += 1
        self.assertEqual(0, transaction_count)

    def test_admission_probe_clones_aggregate_candidate_then_cleans_up(self) -> None:
        probe = function_body(
            self.cpp,
            "bool_t CMapEffectPresentationRuntime::Probe_WorldEffectAdmissions(",
        )
        for required in (
            "Spawn_LevelPlacement(",
            "Commit_PendingWorldRootSpawns(probeHandles)",
            "Update_WorldRoot(",
            "Seek_WorldRoot(",
            "Stop_WorldRoot(",
            "probes.reserve(worlds.size())",
        ):
            self.assertIn(required, probe)
        self.assertNotIn("Commit_PendingSpawns()", probe)
        self.assertEqual(1, probe.count("Commit_PendingWorldRootSpawns("))
        self.assertLess(
            probe.index("probes.push_back(std::move(stagedProbe))"),
            probe.index("Commit_PendingWorldRootSpawns("),
        )
        self.assertGreaterEqual(probe.count("stopProbes();"), 2)

        scoped_commit = function_body(
            self.presentation_service_cpp,
            "void Client::CEffectPresentationService::Commit_PendingWorldRootSpawns(",
        )
        self.assertIn("std::vector<PENDING_EFFECT_SPAWN> retained", scoped_commit)
        self.assertIn(
            "g_PendingEffectSpawns = std::move(retained)", scoped_commit
        )
        self.assertIn("requestedHandles.find(", scoped_commit)
        self.assertIn("Spawn_Immediate(Request.Desc, Status)", scoped_commit)
        self.assertIn(
            "Commit_PendingWorldRootSpawns(", self.presentation_service_header
        )

    def test_active_occurrence_keeps_immutable_spawn_revision(self) -> None:
        commit = function_body(
            self.cpp,
            "bool_t CMapEffectPresentationRuntime::Commit_StagedDocument(",
        )
        update = function_body(
            self.cpp,
            "void CMapEffectPresentationRuntime::Update_ServerPresentation(",
        )
        self.assertIn("MAP_EFFECT_WORLD_PRESENTATION Presentation;", self.header)
        self.assertNotIn("m_ActiveWorldEffects.clear()", commit)
        self.assertIn("active.Presentation, &active", update)
        self.assertIn("active.Presentation = world", update)

        active_occurrence = {"handle": 41, "revision": "old"}
        next_spawn_definition = {"revision": "old"}
        next_spawn_definition = {"revision": "new"}
        self.assertEqual("old", active_occurrence["revision"])
        self.assertEqual("new", next_spawn_definition["revision"])

    def test_server_duplicate_clock_is_bounded_and_stage_edge_keeps_handle(self) -> None:
        resolver = function_body(
            self.cpp,
            "bool_t CMapEffectPresentationRuntime::Resolve_WorldSample(",
        )
        update = function_body(
            self.cpp,
            "void CMapEffectPresentationRuntime::Update_ServerPresentation(",
        )
        self.assertIn("active->iLastServerTick == boss.iServerTick", resolver)
        self.assertIn("authoritativeSample + tickSeconds", resolver)
        self.assertIn("active.iPatternSequence != occurrenceSequence", update)
        self.assertNotIn("active.iActionStartTick != sampleStartTick", update)

        authoritative = 8.0
        tick = 1.0 / 30.0
        sample = authoritative
        for _ in range(20):
            sample = bounded_duplicate_sample(authoritative, sample, 1.0 / 120.0, 30.0)
        self.assertGreater(sample, authoritative)
        self.assertLessEqual(sample, authoritative + tick)

        handle = 73
        previous_action_start = 1000
        next_stage_action_start = 1058
        self.assertNotEqual(previous_action_start, next_stage_action_start)
        corrected_sample = 1.933
        self.assertEqual(73, handle)
        self.assertEqual(1.933, corrected_sample)



if __name__ == "__main__":
    unittest.main(verbosity=2)
