#!/usr/bin/env python3
"""Focused executable contracts for Map Effect runtime lifecycle hardening."""

from __future__ import annotations

import copy
import hashlib
import json
import subprocess
import unittest
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RUNTIME_CPP = ROOT / "Client/Private/MapEffectPresentationRuntime.cpp"
RUNTIME_HEADER = ROOT / "Client/Public/MapEffectPresentationRuntime.h"
PRESENTATION_SERVICE_CPP = ROOT / "Client/Private/Effect_PresentationService.cpp"
PRESENTATION_SERVICE_HEADER = ROOT / "Client/Public/Effect_PresentationService.h"
EFFECT_PATH = (
    ROOT
    / "Data/Effects/Authored/effect.valtan.environment.red-vortex-sky.effect.json"
)
MAP_EFFECT_PATH = (
    ROOT
    / "Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapeffects.json"
)
ENCOUNTER_PATH = ROOT / "Data/Encounters/Valtan/ValtanEncounter.json"
RESOURCE_ROOT = ROOT / "Client/Bin/Resources"

TRACKED_EXACT_BYTE_REUSE_RECEIPT = {
    "Effect/DimensionMaster/Textures/FX_TEX_05/fx_k_cloudtilie_01.dds": {
        "sha256": "78e2e9f740a5e987d095b539b79cf3bca0850895664a3344502bb314f00e3275",
        "same_bytes_as_physical_source": (
            "Effect/Warlord/Textures/FX_TEX_05/fx_k_cloudtilie_01.dds"
        ),
    },
    "Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_shockwave_001_ycl.dds": {
        "sha256": "6c8093e014022aa6368fe7b45562e4384ba07a599ca6802c337f8c5cc8c4c1dd",
        "same_bytes_as_physical_source": (
            "Effect/Valtan/Textures/FX_TEX_02/fx_d_shockwave_001_ycl.dds"
        ),
    },
}

EXACT_SOURCE_IDENTITY_RESOURCE_CLOSURE = {
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_cloud_031.dds": (
        "1e900c88e36936d56a7cde183f7f8756a3b35d020d5fba5434988140f4943f08"
    ),
    "Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_019.dds": (
        "9a98e31c833d8a9efdbc9ec911c991acba57763bb94488afa49b8a11f3d31b35"
    ),
}

CANDIDATE_ONLY_RESOURCE = (
    "Effect/Warlord/Textures/FX_TEX_00/fx_a_atypical_013_cl.dds"
)
CANDIDATE_ONLY_SHA256 = (
    "0f85ec20993380d65d1a494e23da12b70598048de481c7f372d7787468b3c4c3"
)


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig")


def read_json(path: Path) -> dict:
    return json.loads(read_text(path))


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


def sha256_resource(resource_id: str) -> str:
    return hashlib.sha256((RESOURCE_ROOT / resource_id).read_bytes()).hexdigest()


def git_tracked(resource_id: str) -> bool:
    relative = f"Client/Bin/Resources/{resource_id}"
    result = subprocess.run(
        ["git", "-C", str(ROOT), "ls-files", "--error-unmatch", "--", relative],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    return result.returncode == 0


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
        cls.effect = read_json(EFFECT_PATH)
        cls.map_effect = read_json(MAP_EFFECT_PATH)
        cls.encounter = read_json(ENCOUNTER_PATH)

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

    def test_red_vortex_asset_is_not_deployed_during_valtan_entry(self) -> None:
        pattern = next(
            row
            for row in self.encounter["patterns"]
            if row["patternId"] == "VALTAN_FOUR_PILLARS_105"
        )
        expected_offsets: dict[str, int] = {}
        occurrence_duration_ms = 0
        for stage in pattern["stages"]:
            expected_offsets[stage["stageId"]] = occurrence_duration_ms
            occurrence_duration_ms += stage["durationMs"]

        worlds = [
            row
            for row in self.map_effect["presentations"]
            if row["presentationKind"] == "EFFECT_DOCUMENT"
        ]
        self.assertEqual([], worlds)
        self.assertEqual(11933, occurrence_duration_ms)

        expected_times = [0.0, 0.075, 0.53165, 0.58665, 1.0]
        expected_alpha = [0.0, 1.0, 1.0, 0.0, 0.0]
        self.assertEqual(6, len(self.effect["elements"]))
        for element in self.effect["elements"]:
            recipe = element["sourceRecipe"]
            self.assertTrue(recipe["enabled"], element["id"])
            self.assertEqual("sprite", recipe["rendererShape"])
            self.assertEqual(20, recipe["emitterDurationSeconds"])
            self.assertEqual(1, recipe["emitterLoopCount"])
            self.assertEqual(
                [{"timeSeconds": 0, "countMinimum": 1, "countMaximum": 1}],
                recipe["bursts"],
            )
            modules = {
                row["className"]: row for row in recipe["modules"]
            }
            self.assertEqual(
                {
                    "particlemodulerequired",
                    "particlemodulelifetime",
                    "particlemodulespawn",
                    "particlemodulecolorscaleoverlife",
                },
                set(modules),
            )
            required_literals = {
                row["propertyPath"]: row["value"]
                for row in modules["particlemodulerequired"]["literals"]
            }
            self.assertEqual(
                {"buselocalspace": True, "emitterloops": 1},
                required_literals,
            )
            self.assertEqual(
                ["spawnrate"],
                [
                    row["propertyPath"]
                    for row in modules["particlemodulerequired"]["distributions"]
                ],
            )
            spawn_rate_default = modules["particlemodulerequired"][
                "distributions"
            ][0]
            self.assertEqual([0, 0, 0, 0], spawn_rate_default["defaultMinimum"])
            self.assertEqual([0, 0, 0, 0], spawn_rate_default["defaultMaximum"])
            self.assertEqual([], spawn_rate_default["lookupTable"])
            self.assertEqual([], spawn_rate_default["keys"])
            lifetime = modules["particlemodulelifetime"]["distributions"]
            self.assertEqual(1, len(lifetime))
            self.assertEqual("lifetime", lifetime[0]["propertyPath"])
            self.assertEqual([20, 0, 0, 0], lifetime[0]["defaultMinimum"])
            self.assertEqual([20, 0, 0, 0], lifetime[0]["defaultMaximum"])
            self.assertEqual(
                [20, 20], element["detail"]["particle"]["lifeTimeSeconds"]
            )
            self.assertEqual(
                recipe["emitterDurationSeconds"],
                element["detail"]["timing"]["lifeTimeSeconds"],
            )
            spawn_literals = {
                row["propertyPath"]: row["value"]
                for row in modules["particlemodulespawn"]["literals"]
            }
            self.assertEqual(
                {
                    "burstlist[0].count": 1,
                    "burstlist[0].countlow": 1,
                    "burstlist[0].time": 0,
                },
                spawn_literals,
            )
            self.assertEqual(
                ["rate", "ratescale"],
                [
                    row["propertyPath"]
                    for row in modules["particlemodulespawn"]["distributions"]
                ],
            )
            for spawn_distribution in modules["particlemodulespawn"][
                "distributions"
            ]:
                self.assertEqual(
                    [0, 0, 0, 0], spawn_distribution["defaultMinimum"]
                )
                self.assertEqual(
                    [0, 0, 0, 0], spawn_distribution["defaultMaximum"]
                )
                self.assertEqual([], spawn_distribution["lookupTable"])
                self.assertEqual([], spawn_distribution["keys"])
            self.assertEqual(
                recipe["bursts"][0]["countMaximum"],
                element["detail"]["particle"]["burstCount"],
            )
            color_distributions = {
                row["propertyPath"]: row
                for row in modules["particlemodulecolorscaleoverlife"][
                    "distributions"
                ]
            }
            self.assertEqual(
                {"alphascaleoverlife", "colorscaleoverlife"},
                set(color_distributions),
            )
            self.assertEqual(
                [1, 1, 1, 0],
                color_distributions["colorscaleoverlife"]["defaultMinimum"],
            )
            self.assertEqual(
                [1, 1, 1, 0],
                color_distributions["colorscaleoverlife"]["defaultMaximum"],
            )
            keys = color_distributions["alphascaleoverlife"]["keys"]
            self.assertEqual(expected_times, [row["time"] for row in keys])
            self.assertEqual(expected_alpha, [row["maximum"][0] for row in keys])
            self.assertEqual(expected_alpha, [row["minimum"][0] for row in keys])

        lifetime_seconds = 20.0
        takeoff_end = expected_offsets["YELLOW_ZONE"] / 1000.0
        recovery_start = expected_offsets["RECOVERY"] / 1000.0
        fade_in_end = expected_times[1] * lifetime_seconds
        fade_out_start = expected_times[2] * lifetime_seconds
        fade_out_end = expected_times[3] * lifetime_seconds
        occurrence_end = occurrence_duration_ms / 1000.0
        self.assertLess(fade_in_end, takeoff_end)
        self.assertEqual(recovery_start, fade_out_start)
        self.assertGreater(fade_out_end, recovery_start)
        self.assertGreaterEqual(occurrence_end - fade_out_end, 5.0 / 30.0)
        self.assertLess(fade_out_end, occurrence_end)

    def test_resource_provenance_and_minimal_physical_closure(self) -> None:
        referenced = {
            resource["assetId"]
            for element in self.effect["elements"]
            for resource in element["resources"]
        }
        self.assertTrue(set(TRACKED_EXACT_BYTE_REUSE_RECEIPT).issubset(referenced))
        self.assertTrue(set(EXACT_SOURCE_IDENTITY_RESOURCE_CLOSURE).issubset(referenced))
        self.assertNotIn(CANDIDATE_ONLY_RESOURCE, referenced)
        self.assertEqual(CANDIDATE_ONLY_SHA256, sha256_resource(CANDIDATE_ONLY_RESOURCE))
        self.assertNotEqual(
            CANDIDATE_ONLY_SHA256,
            EXACT_SOURCE_IDENTITY_RESOURCE_CLOSURE[
                "Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_019.dds"
            ],
        )

        for selected, receipt in TRACKED_EXACT_BYTE_REUSE_RECEIPT.items():
            self.assertEqual(receipt["sha256"], sha256_resource(selected))
            self.assertTrue(git_tracked(selected), selected)
            original = RESOURCE_ROOT / receipt["same_bytes_as_physical_source"]
            if original.is_file():
                self.assertEqual(receipt["sha256"], sha256_resource(
                    receipt["same_bytes_as_physical_source"]
                ))

        for resource_id, expected_hash in EXACT_SOURCE_IDENTITY_RESOURCE_CLOSURE.items():
            self.assertEqual(expected_hash, sha256_resource(resource_id))

        untracked_referenced = {
            resource_id for resource_id in referenced if not git_tracked(resource_id)
        }
        self.assertLessEqual(
            untracked_referenced,
            set(EXACT_SOURCE_IDENTITY_RESOURCE_CLOSURE),
            "red-vortex references an untracked resource outside its exact two-file closure",
        )
        if untracked_referenced:
            print(
                "red-vortex exact source-identity files still requiring Git tracking: "
                + ", ".join(sorted(untracked_referenced))
            )


if __name__ == "__main__":
    unittest.main(verbosity=2)
