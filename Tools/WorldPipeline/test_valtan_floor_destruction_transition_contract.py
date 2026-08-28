from __future__ import annotations

import copy
import json
import math
import unittest
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROJECTION_PATH = (
    ROOT
    / "Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction.json"
)
PRESENTATION_PATH = (
    ROOT
    / "Client/Bin/DataFiles/World/"
    "LV_LUT_HEARTRB_ED.worlddestructionpresentation.json"
)
GAMEPLAY_AUTHORING_PATH = ROOT / "Data/Valtan/Valtan.gameplay.json"
PRESENTATION_AUTHORING_PATH = ROOT / "Data/Valtan/Valtan.presentation.json"
ENCOUNTER_PATH = ROOT / "Data/Encounters/Valtan/ValtanEncounter.json"
ROTATIONS_PATH = ROOT / "Data/Encounters/Valtan/ValtanPatternRotations.json"
FLOWS_PATH = ROOT / "Data/Encounters/Valtan/ValtanBossAuditionFlows.json"
WORLD_EVENTS_PATH = ROOT / "Data/Encounters/Valtan/ValtanWorldEvents.json"
WORLD_EVENT_SETS_PATH = ROOT / "Data/Valtan/Valtan.worldeventsets.json"
DEBUG_AUDITION_PATH = ROOT / "Data/Encounters/Valtan/ValtanDebugAudition.json"
BOSS_CATALOG_PATH = ROOT / "Data/Actors/BossCatalog.json"
UINT32_MASK = (1 << 32) - 1
UINT32_HALF_RANGE = 1 << 31


def tick_span(end_tick: int, start_tick: int) -> int:
    return (end_tick - start_tick) & UINT32_MASK


def sample_breaking_age(
    *,
    state_start_tick: int,
    commit_tick: int,
    server_tick: int,
    fixed_tick_hz: int,
    local_interpolation_seconds: float,
) -> tuple[int, int]:
    duration = tick_span(commit_tick, state_start_tick)
    if not (0 < duration < UINT32_HALF_RANGE):
        raise ValueError("invalid BREAKING window")
    anchor_age = tick_span(server_tick, state_start_tick)
    if anchor_age >= UINT32_HALF_RANGE:
        anchor_age = 0
    interpolation_ticks = min(
        math.floor(local_interpolation_seconds * fixed_tick_hz), duration
    )
    return min(anchor_age + interpolation_ticks, duration - 1), duration


@dataclass(frozen=True)
class SurfacePacket:
    emissive_intensity: float = 1.0
    emissive_color: tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0)
    emissive_mask_power: float = 1.0
    transition_multiplier: float = 1.0
    root_offset: tuple[float, float, float] = (0.0, 0.0, 0.0)
    root_rotation: tuple[float, float, float, float] = (0.0, 0.0, 0.0, 1.0)
    opacity: float = 1.0


def identity_transition(packet: SurfacePacket) -> SurfacePacket:
    return SurfacePacket(
        emissive_intensity=packet.emissive_intensity,
        emissive_color=packet.emissive_color,
        emissive_mask_power=packet.emissive_mask_power,
    )


def stage_group_transaction(
    group_ids: list[str],
    state_ids: list[str],
    placement_ids: list[int],
    packets: dict[int, SurfacePacket],
) -> dict[int, SurfacePacket]:
    if len(group_ids) != len(state_ids):
        raise ValueError("state count")
    if any(group_id != state_id for group_id, state_id in zip(group_ids, state_ids)):
        raise ValueError("state order")
    if len(set(placement_ids)) != len(placement_ids):
        raise ValueError("duplicate placement")
    if any(placement_id not in packets for placement_id in placement_ids):
        raise ValueError("missing placement")
    staged = copy.deepcopy(packets)
    for placement_id in placement_ids:
        base = packets[placement_id]
        staged[placement_id] = SurfacePacket(
            emissive_intensity=base.emissive_intensity,
            emissive_color=base.emissive_color,
            emissive_mask_power=base.emissive_mask_power,
            transition_multiplier=0.5,
            root_offset=(0.0, -2.0, 0.0),
            opacity=0.5,
        )
    return staged


class ValtanFloorDestructionTransitionContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.projection = json.loads(PROJECTION_PATH.read_text(encoding="utf-8-sig"))
        cls.presentation = json.loads(
            PRESENTATION_PATH.read_text(encoding="utf-8-sig")
        )
        cls.gameplay_authoring = json.loads(
            GAMEPLAY_AUTHORING_PATH.read_text(encoding="utf-8-sig")
        )
        cls.presentation_authoring = json.loads(
            PRESENTATION_AUTHORING_PATH.read_text(encoding="utf-8-sig")
        )
        cls.encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8-sig"))
        cls.rotations = json.loads(ROTATIONS_PATH.read_text(encoding="utf-8-sig"))
        cls.flows = json.loads(FLOWS_PATH.read_text(encoding="utf-8-sig"))
        cls.world_events = json.loads(
            WORLD_EVENTS_PATH.read_text(encoding="utf-8-sig")
        )
        cls.world_event_sets = json.loads(
            WORLD_EVENT_SETS_PATH.read_text(encoding="utf-8-sig")
        )
        cls.debug_audition = json.loads(
            DEBUG_AUDITION_PATH.read_text(encoding="utf-8-sig")
        )
        cls.boss_catalog = json.loads(
            BOSS_CATALOG_PATH.read_text(encoding="utf-8-sig")
        )

    def test_canonical_three_and_nine_oclock_server_audition_closure(self) -> None:
        pattern_ids = (
            "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
            "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
        )
        set_ids = (
            "worldeventset.valtan.terrain-destruction-3.floor84",
            "worldeventset.valtan.terrain-destruction-9.floor30",
        )
        sequence_reference = self.gameplay_authoring["decisionModel"]["scriptedSequence"]
        source_flow = next(
            row for row in self.flows["flows"]
            if row["flowId"] == sequence_reference["flowId"]
        )
        source_sequence = [row["patternId"] for row in source_flow["slots"]]
        product_sequence = self.rotations["scriptedSequence"]["patternIds"]
        self.assertEqual(source_sequence, product_sequence)
        self.assertEqual(
            self.rotations["scriptedSequence"]["interStepPursuitMs"],
            source_flow["interStepPursuitMs"],
        )

        gameplay_by_id = {
            row["patternId"]: row for row in self.gameplay_authoring["patterns"]
        }
        presentation_by_id = {
            row["patternId"]: row
            for row in self.presentation_authoring["patterns"]
        }
        product_by_id = {
            row["patternId"]: row for row in self.encounter["patterns"]
        }
        set_by_id = {
            row["worldEventSetId"]: row
            for row in self.world_event_sets["sets"]
        }
        audition_pattern_ids = {
            pattern["patternId"]
            for row in self.debug_audition["rows"]
            for pattern in row["patterns"]
        }
        for pattern_id, set_id in zip(pattern_ids, set_ids):
            source = gameplay_by_id[pattern_id]
            source_presentation = presentation_by_id[pattern_id]
            product = product_by_id[pattern_id]
            self.assertEqual(len(source["stages"]), 4)
            self.assertEqual(source["stages"][1]["stageId"], "AIRBORNE")
            self.assertEqual(source["stages"][1]["durationMs"], 2000)
            self.assertEqual(source["stages"][3]["stageId"], "IMPACT")
            self.assertEqual(product["stages"][3]["stageId"], "IMPACT")
            self.assertEqual(
                source["stages"][3]["actionId"],
                product["stages"][3]["actionId"],
            )
            self.assertEqual(
                [
                    event["worldEventSetId"]
                    for event in source["stages"][3]["events"]
                    if event["kind"] == "TRIGGER_WORLD_EVENT_SET"
                ],
                [set_id],
            )
            self.assertEqual(
                source_presentation["stages"][3]["actionId"],
                source["stages"][3]["actionId"],
            )
            self.assertTrue(source["serverMotion"]["moveToAnchorBeforeTakeoff"])
            takeoff_cues = source_presentation["stages"][0]["effectCues"]
            self.assertTrue(takeoff_cues)
            for cue in takeoff_cues:
                self.assertEqual(cue["anchorSlotId"], "arena.center")
                self.assertEqual(cue["followPolicy"], "snapshot")
            self.assertIn(pattern_id, audition_pattern_ids)

            members = set_by_id[set_id]["members"]
            self.assertEqual(len(members), 3)
            member_binding_ids = {member["bindingId"] for member in members}
            product_bindings = [
                row
                for row in self.world_events["bindings"]
                if row["bindingId"] in member_binding_ids
            ]
            self.assertEqual(len(product_bindings), 3)
            self.assertTrue(
                all(
                    row["patternId"] == pattern_id
                    and row["stageId"] == "IMPACT"
                    and row["triggerKind"] == "STAGE_ENTER"
                    and row["enabled"] is True
                    for row in product_bindings
                )
            )

        valtan = next(
            row
            for row in self.boss_catalog["bosses"]
            if row["archetypeId"] == "BOSS_VALTAN"
        )
        self.assertEqual(valtan["presentationScale"], 1.0)

    def test_published_ground_groups_join_the_existing_debris_seam(self) -> None:
        self.assertEqual(
            self.projection["schema"], "lostark.world-destruction-client-projection"
        )
        self.assertEqual(self.projection["formatVersion"], 3)
        self.assertEqual(
            self.projection["combatRuntimeRevision"],
            self.presentation["combatRuntimeRevision"],
        )
        floor_groups = [
            group for group in self.projection["groups"] if group["removesGround"]
        ]
        self.assertEqual(len(floor_groups), 6)
        self.assertTrue(
            all(group["groupId"].startswith("destroyable.group.valtan.floor") for group in floor_groups)
        )
        profiles = {
            profile["groupId"]: profile for profile in self.presentation["profiles"]
        }
        for group in floor_groups:
            profile = profiles[group["groupId"]]
            self.assertEqual(profile["mutationId"], group["mutationId"])
            emitter_placements = {
                int(emitter["sourceRuntimePlacementId"])
                for emitter in profile["emitters"]
            }
            self.assertTrue(emitter_placements.issubset(set(map(int, group["memberPlacementIds"]))))
            for emitter in profile["emitters"]:
                self.assertLessEqual(float(emitter["direction"][1]), -0.5)
                self.assertGreater(float(emitter["speedMetersPerSecond"]), 0.0)
                self.assertGreaterEqual(float(emitter["gravityScale"]), 0.0)

    def test_late_join_uses_full_sync_server_tick_and_never_commits_locally(self) -> None:
        age, duration = sample_breaking_age(
            state_start_tick=100,
            commit_tick=108,
            server_tick=104,
            fixed_tick_hz=30,
            local_interpolation_seconds=0.0,
        )
        self.assertEqual((age, duration), (4, 8))
        self.assertEqual(age / duration, 0.5)

        age, duration = sample_breaking_age(
            state_start_tick=100,
            commit_tick=108,
            server_tick=104,
            fixed_tick_hz=30,
            local_interpolation_seconds=0.25,
        )
        self.assertEqual(age, duration - 1)
        self.assertLess(age / duration, 1.0)

    def test_valid_transition_longer_than_one_second_does_not_freeze(self) -> None:
        age, duration = sample_breaking_age(
            state_start_tick=500,
            commit_tick=575,
            server_tick=500,
            fixed_tick_hz=30,
            local_interpolation_seconds=1.5,
        )
        self.assertEqual(duration, 75)
        self.assertEqual(age, 45)
        self.assertGreater(age, 30)

        clamped_age, _ = sample_breaking_age(
            state_start_tick=500,
            commit_tick=575,
            server_tick=500,
            fixed_tick_hz=30,
            local_interpolation_seconds=10.0,
        )
        self.assertEqual(clamped_age, duration - 1)

    def test_generation_reset_preserves_map_surface_fields(self) -> None:
        breaking = SurfacePacket(
            emissive_intensity=3.5,
            emissive_color=(0.2, 0.4, 0.8, 1.0),
            emissive_mask_power=2.25,
            transition_multiplier=0.15,
            root_offset=(0.0, -4.0, 0.0),
            opacity=0.1,
        )
        restored = identity_transition(breaking)
        self.assertEqual(restored.emissive_intensity, breaking.emissive_intensity)
        self.assertEqual(restored.emissive_color, breaking.emissive_color)
        self.assertEqual(restored.emissive_mask_power, breaking.emissive_mask_power)
        self.assertEqual(restored.transition_multiplier, 1.0)
        self.assertEqual(restored.root_offset, (0.0, 0.0, 0.0))
        self.assertEqual(restored.opacity, 1.0)

    def test_final_despawned_restores_transition_lane_only(self) -> None:
        late_curve = SurfacePacket(
            emissive_intensity=6.0,
            emissive_mask_power=3.0,
            transition_multiplier=0.0,
            root_offset=(0.0, -8.0, 0.0),
            opacity=0.0,
        )
        final_packet = identity_transition(late_curve)
        self.assertEqual(final_packet.transition_multiplier, 1.0)
        self.assertEqual(final_packet.root_rotation, (0.0, 0.0, 0.0, 1.0))
        self.assertEqual(final_packet.opacity, 1.0)
        self.assertEqual(final_packet.emissive_intensity, 6.0)
        self.assertEqual(final_packet.emissive_mask_power, 3.0)

    def test_bad_order_or_missing_placement_cannot_partially_commit(self) -> None:
        packets = {
            11: SurfacePacket(emissive_intensity=2.0),
            12: SurfacePacket(emissive_intensity=4.0),
        }
        before = copy.deepcopy(packets)
        with self.assertRaisesRegex(ValueError, "state order"):
            stage_group_transaction(["floor.a", "floor.b"], ["floor.b", "floor.a"], [11, 12], packets)
        self.assertEqual(packets, before)
        with self.assertRaisesRegex(ValueError, "missing placement"):
            stage_group_transaction(["floor.a"], ["floor.a"], [11, 99], packets)
        self.assertEqual(packets, before)

    def test_cpp_runtime_keeps_one_owner_and_server_authority(self) -> None:
        runtime = (
            ROOT / "Client/Private/WorldDestructionDebrisPresentationRuntime.cpp"
        ).read_text(encoding="utf-8-sig")
        header = (
            ROOT / "Client/Public/WorldDestructionDebrisPresentationRuntime.h"
        ).read_text(encoding="utf-8-sig")
        deploy_header = (ROOT / "Client/Public/DeployPropObject.h").read_text(
            encoding="utf-8-sig"
        )
        level = (ROOT / "Client/Private/Level_ValtanArena.cpp").read_text(
            encoding="utf-8-sig"
        )
        replication = (ROOT / "Client/Public/ClientReplication.h").read_text(
            encoding="utf-8-sig"
        )
        deploy_runtime = (ROOT / "Client/Private/DeployPropRuntime.cpp").read_text(
            encoding="utf-8-sig"
        )
        projection_runtime = (
            ROOT / "Client/Private/WorldDestructionProjectionRuntime.cpp"
        ).read_text(encoding="utf-8-sig")

        self.assertIn("DEPLOY_SURFACE_PRESENTATION_PACKET", deploy_header)
        self.assertIn("maximumActiveBreakingTicks", runtime)
        self.assertIn("maximumBreakingAge", runtime)
        self.assertIn("Reset_SourceTransitions", runtime)
        self.assertIn("m_SourceTransitionPlacementIds", runtime)
        self.assertIn("packet.fTransitionMultiplier = 1.f", runtime)
        self.assertIn("packet.fOpacity = 1.f", runtime)
        self.assertIn("emitterCue.suppressSource = !projectionGroup->bRemovesGround", level)
        self.assertIn("if (!m_WorldDestructionDebrisPresentationRuntime.Reset_Presentation())", level)
        self.assertIn("Get_WorldDestructionServerTick", replication)
        self.assertIn("Set_SurfacePresentations", deploy_runtime)
        self.assertIn("surface presentation transaction rolled back", deploy_runtime)
        self.assertIn("case WORLD_DESTRUCTION_RUNTIME_STATE::DESPAWNED", projection_runtime)
        self.assertIn("outState = DEPLOY_PROP_STATE::DESPAWNED", projection_runtime)
        self.assertNotIn("fx_a_atypical_013_cl", runtime.lower())
        self.assertNotIn("effect.valtan.sky", runtime.lower())


if __name__ == "__main__":
    unittest.main()
