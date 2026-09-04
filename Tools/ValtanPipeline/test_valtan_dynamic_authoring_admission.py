from __future__ import annotations

import copy
import json
import unittest
from pathlib import Path

from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class ValtanDynamicAuthoringAdmissionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = REPOSITORY_ROOT.resolve()
        cls.docs = pipeline.load_pipeline_documents(cls.root)

    def sources(self) -> tuple[dict, dict, dict]:
        return (
            copy.deepcopy(self.docs[pipeline.GAMEPLAY_AUTHORING_REL]),
            copy.deepcopy(self.docs[pipeline.PRESENTATION_AUTHORING_REL]),
            copy.deepcopy(self.docs),
        )

    @staticmethod
    def pattern(document: dict, pattern_id: str) -> dict:
        return next(
            row for row in document["patterns"] if row["patternId"] == pattern_id
        )

    @staticmethod
    def stage(pattern: dict, stage_id: str) -> dict:
        return next(row for row in pattern["stages"] if row["stageId"] == stage_id)

    def join(self, gameplay: dict, presentation: dict, docs: dict) -> dict:
        return pipeline.join_v2_authoring(
            gameplay,
            presentation,
            docs[pipeline.WORLD_SET_REL],
            docs[pipeline.COMBAT_AUTHORING_REL],
        )

    def test_portal_stage_id_and_each_leg_timing_are_data_driven(self) -> None:
        gameplay, presentation, docs = self.sources()
        gameplay_warp = self.pattern(gameplay, "VALTAN_WARP")
        presentation_warp = self.pattern(presentation, "VALTAN_WARP")
        gameplay_leg = self.stage(gameplay_warp, "STEP_02")
        presentation_leg = self.stage(presentation_warp, "STEP_02")

        old_action_id = gameplay_leg["actionId"]
        gameplay_warp["stages"][0]["defaultNextActionId"] = (
            "valtan.sequence.warp.portal-leg-alpha"
        )
        gameplay_leg["stageId"] = "PORTAL_LEG_ALPHA"
        gameplay_leg["actionId"] = "valtan.sequence.warp.portal-leg-alpha"
        gameplay_leg["durationMs"] = 2600
        gameplay_leg["motion"] = {
            "kind": "PORTAL_TARGET_RUSH",
            "retargetDelayMs": 500,
            "speedMps": 20.0,
            "distanceM": 20.0,
        }
        gameplay_leg["hit"]["schedule"]["offsetsMs"] = list(
            range(500, 1500, 50)
        )
        presentation_leg["stageId"] = "PORTAL_LEG_ALPHA"
        presentation_leg["actionId"] = "valtan.sequence.warp.portal-leg-alpha"

        master = self.join(gameplay, presentation, docs)
        projected = pipeline.project_v2_products(self.root, docs, master)
        product = next(
            row
            for row in json.loads(projected[pipeline.ENCOUNTER_REL])["patterns"]
            if row["patternId"] == "VALTAN_WARP"
        )
        product_leg = self.stage(product, "PORTAL_LEG_ALPHA")
        self.assertNotEqual(old_action_id, product_leg["actionId"])
        self.assertEqual(20.0, product_leg["motion"]["distanceM"])
        self.assertEqual(2600, product_leg["durationMs"])

    def test_ghost_finale_primary_children_are_fixed_while_capacity_and_portal_are_data_driven(
        self,
    ) -> None:
        gameplay, presentation, docs = self.sources()
        gameplay_finale = self.pattern(gameplay, "VALTAN_GHOST_FINALE")
        gameplay_finale["finale"]["ghostPatternIds"] = [
            "VALTAN_FOUR_SLASH",
            "VALTAN_WHIRLWIND",
        ]
        with self.assertRaisesRegex(pipeline.PipelineError, "primary-loop order drifted"):
            self.join(gameplay, presentation, docs)

        gameplay, presentation, docs = self.sources()
        gameplay_finale = self.pattern(gameplay, "VALTAN_GHOST_FINALE")
        presentation_finale = self.pattern(presentation, "VALTAN_GHOST_FINALE")
        finale = gameplay_finale["finale"]
        finale["maximumActiveGhosts"] = 2

        gameplay_leg = self.stage(gameplay_finale, "STEP_02")
        presentation_leg = self.stage(presentation_finale, "STEP_02")
        gameplay_finale["stages"][0]["defaultNextActionId"] = (
            "valtan.sequence.ghost-finale.portal-leg-alpha"
        )
        gameplay_leg["stageId"] = "PORTAL_LEG_ALPHA"
        gameplay_leg["actionId"] = "valtan.sequence.ghost-finale.portal-leg-alpha"
        gameplay_leg["motion"] = {
            "kind": "PORTAL_CROSS_ARENA",
            "cornerIndex": 3,
            "halfExtentsM": [18.0, 26.0],
        }
        presentation_leg["stageId"] = "PORTAL_LEG_ALPHA"
        presentation_leg["actionId"] = "valtan.sequence.ghost-finale.portal-leg-alpha"

        master = self.join(gameplay, presentation, docs)
        projected = pipeline.project_v2_products(self.root, docs, master)
        product = next(
            row
            for row in json.loads(projected[pipeline.ENCOUNTER_REL])["patterns"]
            if row["patternId"] == "VALTAN_GHOST_FINALE"
        )
        self.assertEqual(finale, product["finale"])
        self.assertEqual(
            [18.0, 26.0],
            self.stage(product, "PORTAL_LEG_ALPHA")["motion"]["halfExtentsM"],
        )

    def test_phase_edges_are_fixed_while_volley_events_may_move_by_stable_id(
        self,
    ) -> None:
        gameplay, presentation, docs = self.sources()
        arena_break = self.pattern(gameplay, "VALTAN_ARENA_BREAK_109")
        impact = self.stage(arena_break, "IMPACT")
        phase_event = next(
            event for event in impact["events"] if event["kind"] == "SET_GAMEPLAY_PHASE"
        )
        impact["events"].remove(phase_event)
        self.stage(arena_break, "RECOVERY")["events"].append(phase_event)
        with self.assertRaisesRegex(
            pipeline.PipelineError, "exact arena-break and ghost-respawn transitions"
        ):
            self.join(gameplay, presentation, docs)

        gameplay, presentation, docs = self.sources()
        high_jump = self.pattern(gameplay, "VALTAN_HIGH_JUMP")
        airborne = self.stage(high_jump, "AIRBORNE")
        volley = next(
            event
            for event in airborne["events"]
            if event["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY"
        )
        airborne["events"].remove(volley)
        self.stage(high_jump, "LAND")["events"].append(volley)

        master = self.join(gameplay, presentation, docs)
        projected = pipeline.project_v2_products(self.root, docs, master)
        product = {
            row["patternId"]: row
            for row in json.loads(projected[pipeline.ENCOUNTER_REL])["patterns"]
        }
        combat_product = json.loads(projected[pipeline.COMBAT_PRODUCT_REL])
        target_axe = next(
            row
            for row in combat_product["objects"]
            if row["combatObjectArchetypeId"]
            == "combatobject.valtan.high-jump.target-axe"
        )
        self.assertEqual(
            self.stage(product["VALTAN_HIGH_JUMP"], "LAND")["actionId"],
            target_axe["ownerStageActionId"],
        )

    def test_new_valtan_cue_namespace_is_not_a_historical_id_whitelist(self) -> None:
        gameplay, presentation, docs = self.sources()
        presentation_pattern = self.pattern(presentation, "VALTAN_WARP")
        presentation_stage = self.stage(presentation_pattern, "STEP_02")
        cue = copy.deepcopy(next(
            cue
            for pattern in presentation["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
            if cue["anchorSlotId"] == "root"
            and cue["followPolicy"] == "follow"
            and "clipOccurrenceId" in cue
        ))
        cue["cueId"] = "cue.valtan.user-authored.warp.portal-leg"
        cue["occurrenceId"] = cue["cueId"] + ".occurrence.01"
        cue["clipOccurrenceId"] = presentation_stage["animation"]["occurrences"][0][
            "clipOccurrenceId"
        ]
        cue["sourceStartMs"] = 0
        cue["sourceEndMs"] = None
        cue["scalePolicy"] = {
            "kind": "GAMEPLAY_FOOTPRINT",
            "worldScale": [1.25, 1.25, 1.25],
        }
        presentation_stage["effectCues"].append(cue)

        master = self.join(gameplay, presentation, docs)
        projected = pipeline.project_v2_products(self.root, docs, master)
        cue_ids = {
            row["bindingId"]
            for row in json.loads(projected[pipeline.CUES_REL])["cues"]
        }
        self.assertIn(cue["cueId"], cue_ids)

    def test_dynamic_admission_keeps_structural_identity_and_reference_checks(self) -> None:
        gameplay, presentation, docs = self.sources()
        warp = self.pattern(gameplay, "VALTAN_WARP")
        warp["stages"][2]["actionId"] = warp["stages"][1]["actionId"]
        with self.assertRaisesRegex(pipeline.PipelineError, "duplicate"):
            self.join(gameplay, presentation, docs)

        gameplay, presentation, docs = self.sources()
        high_jump = self.pattern(gameplay, "VALTAN_HIGH_JUMP")
        volley = next(
            event
            for stage in high_jump["stages"]
            for event in stage["events"]
            if event["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY"
        )
        volley["combatObjectArchetypeId"] = "combatobject.valtan.missing"
        with self.assertRaisesRegex(
            pipeline.PipelineError, "unresolved/unsupported archetype"
        ):
            self.join(gameplay, presentation, docs)

        gameplay, presentation, docs = self.sources()
        self.stage(self.pattern(gameplay, "VALTAN_WARP"), "STEP_02")["motion"] = {
            "kind": "UNSUPPORTED_MOTION"
        }
        with self.assertRaisesRegex(pipeline.PipelineError, "kind is unsupported"):
            self.join(gameplay, presentation, docs)

    def test_rejected_draft_keeps_input_documents_byte_equivalent(self) -> None:
        gameplay, presentation, docs = self.sources()
        master = self.join(gameplay, presentation, docs)
        before = copy.deepcopy(master)
        source_revision = "0" * 64
        draft = {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": source_revision,
            "operations": [
                {
                    "op": "SET_STAGE_PORTAL_RUSH_MOTION",
                    "patternId": "VALTAN_WARP",
                    "stageId": "STEP_02",
                    "retargetDelayMs": 500,
                    "speedMps": 20.0,
                    "distanceM": 1000.0,
                }
            ],
        }
        with self.assertRaises(pipeline.DraftPatchError):
            pipeline.apply_draft_patch(
                master,
                docs[pipeline.BOSS_PROFILES_REL],
                docs[pipeline.DAMAGE_REL],
                draft,
                source_revision,
                docs[pipeline.WORLD_SET_REL],
                docs[pipeline.COMBAT_AUTHORING_REL],
            )
        self.assertEqual(before, master)


if __name__ == "__main__":
    unittest.main()
