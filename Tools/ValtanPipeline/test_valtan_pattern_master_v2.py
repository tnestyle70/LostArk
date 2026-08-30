#!/usr/bin/env python3
from __future__ import annotations

import argparse
import copy
import contextlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from collections import Counter
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]

EXPECTED_SCRIPTED_SEQUENCE = {
    "sequenceId": "sequence.valtan.server-authored.v1",
    "mode": "ORDERED_ONCE_THEN_IDLE",
    "interStepPursuitMs": 1000,
    "patternIds": [
        "VALTAN_WHIRLWIND",
        "VALTAN_FOUR_SLASH",
        "VALTAN_HIGH_JUMP",
        "VALTAN_DASH_CHARGE",
        "VALTAN_FLOOR_WIPE_130",
        "VALTAN_FIST_IN_OUT",
        "VALTAN_WHIRLWIND",
        "VALTAN_ARENA_BREAK_109",
        "VALTAN_WHIRLWIND",
        "VALTAN_FOUR_SLASH",
        "VALTAN_SIX_PIZZA_106",
        "VALTAN_CHARGE",
        "VALTAN_SEQUENCE_FOUR",
        "VALTAN_HIGH_JUMP",
        "VALTAN_COUNTER",
        "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
        "VALTAN_THREE",
        "VALTAN_SEQUENCE_FOUR",
        "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
        "VALTAN_WARP",
        "VALTAN_TRASH",
        "VALTAN_CATCH_BREATH",
        "VALTAN_CHARGE_2",
        "VALTAN_STRUGGLING",
        "VALTAN_GHOST_RESPAWN_AUDITION",
        "VALTAN_GHOST_FINALE",
    ],
}


class ValtanPatternMasterV2Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = REPOSITORY_ROOT.resolve()
        cls.docs = pipeline.load_pipeline_documents(cls.root)
        cls.migration_docs = pipeline.load_pipeline_documents(
            cls.root,
            include_split_authoring=False,
            include_migration_fixture=True,
        )
        cls.source_manifest = pipeline.source_manifest(cls.root)
        cls.debug_presentation = pipeline.read_json(
            cls.root / pipeline.DEBUG_PRESENTATION_REL
        )
        cls.animation_promotion_manifest = pipeline.read_json(
            cls.root / pipeline.ANIMATION_PROMOTION_MANIFEST_REL
        )

    def migrate(self):
        return pipeline.migrate_v1_to_v2(
            self.root, copy.deepcopy(self.migration_docs)
        )

    def with_manual_audition(
        self,
        master: dict | None = None,
        *,
        pattern_id: str = "VALTAN_ANIMATION_PHASE2_PIPELINE_TEST",
        source_chain_id: str = "pipeline-test-chain",
    ) -> dict:
        staged = copy.deepcopy(
            master
            if master is not None
            else pipeline.join_v2_authoring(
                self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
                self.docs[pipeline.PRESENTATION_AUTHORING_REL],
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )
        )
        action_root = "valtan.animation.phase2." + source_chain_id
        action_id = action_root + ".step-01"
        staged["patterns"].append(
            {
                "patternId": pattern_id,
                "displayName": "[P2 Animation] " + source_chain_id,
                "category": "NORMAL",
                "compatibilitySelectionWeight": 0,
                "actionId": action_root,
                "entryActionId": action_id,
                "targetPolicy": "NONE",
                "aimPolicy": "NONE",
                "eligibility": {
                    "armorRequirement": "ANY",
                    "phaseRequirement": "ANY",
                    "minimumGameplayPhase": 1,
                    "maximumGameplayPhase": 3,
                    "minimumHealthBarInclusive": 1,
                    "maximumHealthBarInclusive": 160,
                    "minimumRangeM": 0.0,
                    "maximumRangeM": 1.0,
                    "cooldownPolicy": "DERIVED_SOURCE_ACTION",
                    "selectionCooldownMs": None,
                    "cooldownGroupId": None,
                    "repeatPolicy": {
                        "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE",
                        "limit": 4,
                    },
                },
                "invulnerableWhileRunning": False,
                "sourceActionIds": [420633],
                "sourceSequenceIndex": 0,
                "presentationSources": [
                    {
                        "sourceActionId": 420633,
                        "sequenceIndex": 0,
                        "role": "PRIMARY",
                    }
                ],
                "serverMotion": None,
                "reactions": [],
                "stages": [
                    {
                        "stageId": "STEP_01",
                        "sequenceRole": "ACTIVE",
                        "actionId": action_id,
                        "stageKind": "ACTIVE",
                        "durationMs": 1000,
                        "defaultNextActionId": None,
                        "hit": {"shape": {"kind": "NONE"}},
                        "motion": None,
                        "events": [],
                        "branches": [],
                        "animation": {
                            "endPolicy": "EXACT",
                            "repeatCount": 1,
                            "occurrences": [
                                {
                                    "clipOccurrenceId": action_id + ".clip.01",
                                    "clip": "mesh_idle_battle_1",
                                    "mappingBasis": "PROJECT_AUTHORED",
                                    "sourceStartMs": 0,
                                    "playMs": 1000,
                                    "playRate": 1.0,
                                    "repeatUntilStageEnd": False,
                                }
                            ],
                        },
                        "effectCues": [],
                        "cameraInvocations": [],
                    }
                ],
            }
        )
        staged["decisionModel"]["manualAuditions"].append(
            {
                "patternId": pattern_id,
                "sourceChainId": source_chain_id,
                "authoringPhase": 2,
                "admissionState": pipeline.MANUAL_SERVER_AUDITION,
            }
        )
        return staged

    def create_directory_reparse(self, link: Path, target: Path) -> None:
        if os.name == "nt":
            completed = subprocess.run(
                [
                    "powershell",
                    "-NoProfile",
                    "-Command",
                    "New-Item",
                    "-ItemType",
                    "Junction",
                    "-Path",
                    str(link),
                    "-Target",
                    str(target),
                ],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=False,
            )
            self.assertEqual(0, completed.returncode, "failed to create test junction")
        else:
            link.symlink_to(target, target_is_directory=True)
        self.assertTrue(pipeline._is_reparse_point(link))

    @staticmethod
    def remove_directory_reparse(link: Path) -> None:
        if not (link.exists() or pipeline._is_reparse_point(link)):
            return
        if os.name == "nt":
            os.rmdir(link)
        else:
            link.unlink()

    def draft_patch(self):
        migrated = self.migrate()
        spin = next(
            stage
            for pattern in migrated["patterns"]
            if pattern["patternId"] == "VALTAN_WHIRLWIND"
            for stage in pattern["stages"]
            if stage["stageId"] == "SPIN"
        )
        edited_hit = copy.deepcopy(spin["hit"])
        edited_hit["shape"]["outerRadiusM"] = 10.5
        edited_hit["schedule"] = {
            "kind": "EXPLICIT_OFFSETS",
            "offsetsMs": [0, 300, 600, 900],
        }
        edited_hit["serverDamageProfileId"] = "damage.valtan.circular-spin"
        edited_hit["pushRangeM"] = 0.5
        edited_hit["pushMs"] = 75
        edited_hit["knockdown"] = True
        edited_hit["downMs"] = 500
        return {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.source_manifest["sourceManifestId"],
            "operations": [
                {
                    "op": "SET_PATTERN_WEIGHT",
                    "selectionSetId": "selectionset.valtan.160.130",
                    "patternId": "VALTAN_WHIRLWIND",
                    "value": 25,
                },
                {
                    "op": "SET_PATTERN_WEIGHT",
                    "selectionSetId": "selectionset.valtan.130.109",
                    "patternId": "VALTAN_WHIRLWIND",
                    "value": 25,
                },
                {
                    "op": "SET_PATTERN_REPEAT_LIMIT",
                    "patternId": "VALTAN_WHIRLWIND",
                    "value": 3,
                },
                {
                    "op": "SET_PATTERN_RANGE",
                    "patternId": "VALTAN_WHIRLWIND",
                    "minimumRangeM": 1.0,
                    "maximumRangeM": 13.0,
                },
                {
                    "op": "SET_STAGE_DURATION",
                    "patternId": "VALTAN_WHIRLWIND",
                    "stageId": "WINDUP",
                    "durationMs": 1400,
                },
                {
                    "op": "SET_STAGE_HIT",
                    "patternId": "VALTAN_WHIRLWIND",
                    "stageId": "SPIN",
                    "hit": edited_hit,
                },
                {
                    "op": "SET_AXE_VOLLEY",
                    "patternId": "VALTAN_HIGH_JUMP",
                    "stageId": "AIRBORNE",
                    "eventId": "event.valtan.high-jump.airborne.spawn-target-axe",
                    "countPerResolvedTarget": 1,
                    "layout": {"kind": "TARGET_CENTER"},
                    "spawnSchedule": {
                        "kind": "INTERVAL",
                        "count": 3,
                        "firstOffsetMs": 0,
                        "intervalMs": 1333,
                    },
                    "arenaRandom": {
                        "kind": "RANDOM_NAVIGABLE_CIRCLE",
                        "anchor": "BOSS_SPAWN_POSITION",
                        "count": 4,
                        "radiusM": 14.0,
                        "heightToleranceM": 1.0,
                    },
                    "allowOverlap": False,
                    "maximumTotalObjects": 36,
                },
                {
                    "op": "SET_BOSS_BASE_FIELD",
                    "bossArchetypeId": "BOSS_VALTAN",
                    "field": "maximumHp",
                    "value": 61000,
                },
                {
                    "op": "SET_DAMAGE_RATE",
                    "damageProfileId": "damage.valtan.circular-spin",
                    "value": 325,
                },
            ],
        }

    def test_v1_migration_is_staged_and_excluded_from_current_split_authority(self) -> None:
        source_path = self.root / pipeline.MASTER_REL
        before = pipeline.sha256_file(source_path)
        migrated = self.migrate()
        self.assertEqual(
            1, self.migration_docs[pipeline.MASTER_REL]["formatVersion"]
        )
        self.assertEqual(2, migrated["formatVersion"])
        self.assertEqual(7, len(migrated["patterns"]))
        self.assertIsNone(migrated["decisionModel"]["scriptedSequence"])
        unlinked_cue = "cue.valtan.carrier-v1.attack.four-slash.recovery.clip-01"
        for document in (migrated, self.docs[pipeline.PRESENTATION_AUTHORING_REL]):
            self.assertNotIn(
                unlinked_cue,
                {
                    cue["cueId"]
                    for pattern in document["patterns"]
                    for stage in pattern["stages"]
                    for cue in stage["effectCues"]
                },
                "V1 migration must not restore an intentionally unlinked Product cue",
            )
        self.assertEqual(before, pipeline.sha256_file(source_path))
        phase_events = [
            (pattern["patternId"], stage["stageId"], event)
            for pattern in migrated["patterns"]
            for stage in pattern["stages"]
            for event in stage["events"]
            if event["kind"] == "SET_GAMEPLAY_PHASE"
        ]
        self.assertEqual(
            [
                (
                    "VALTAN_ARENA_BREAK_109",
                    "IMPACT",
                    {
                        "eventId": "event.valtan.arena-break-109.impact.set-gameplay-phase-2",
                        "trigger": "ENTER",
                        "kind": "SET_GAMEPLAY_PHASE",
                        "gameplayPhase": 2,
                    },
                )
            ],
            phase_events,
        )
        projected = pipeline.project_v2_products(
            self.root, self.docs, migrated, migration_fixture=True
        )
        self.assertEqual(
            self.docs[pipeline.ROTATIONS_REL],
            json.loads(
                projected[pipeline.ROTATIONS_REL],
                object_pairs_hook=pipeline._reject_duplicate_pairs,
            ),
            "the frozen v1 migration fixture must preserve the current rotation "
            "Product instead of projecting its scriptedSequence=null placeholder",
        )
        for relative, text in projected.items():
            if relative in (pipeline.ENCOUNTER_REL, pipeline.BINDINGS_REL):
                continue
            self.assertEqual(
                self.docs[relative],
                json.loads(text, object_pairs_hook=pipeline._reject_duplicate_pairs),
                relative,
            )
        migrated_high_jump = next(
            pattern
            for pattern in migrated["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
        )
        current_high_jump = next(
            pattern
            for pattern in self.docs[pipeline.GAMEPLAY_AUTHORING_REL]["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
        )
        self.assertEqual("LAND", migrated_high_jump["serverMotion"]["travelStageId"])
        self.assertEqual("LAND", current_high_jump["serverMotion"]["travelStageId"])
        self.assertNotEqual(
            self.docs[pipeline.ENCOUNTER_REL],
            json.loads(
                projected[pipeline.ENCOUNTER_REL],
                object_pairs_hook=pipeline._reject_duplicate_pairs,
            ),
            "the frozen v1 migration fixture must not impersonate current split authoring",
        )
        self.assertNotEqual(
            self.docs[pipeline.BINDINGS_REL],
            json.loads(
                projected[pipeline.BINDINGS_REL],
                object_pairs_hook=pipeline._reject_duplicate_pairs,
            ),
        )

    def test_split_authoring_round_trips_canonically_and_preserves_products(self) -> None:
        self.assertTrue(self.source_manifest["splitJoinValidated"])
        source_paths = {row["path"] for row in self.source_manifest["files"]}
        self.assertIn(pipeline.GAMEPLAY_AUTHORING_REL, source_paths)
        self.assertIn(pipeline.PRESENTATION_AUTHORING_REL, source_paths)
        self.assertIn(pipeline.ANIMATION_PROMOTION_MANIFEST_REL, source_paths)
        self.assertIn(pipeline.SAVED_FLOW_REL, source_paths)
        self.assertNotIn(pipeline.MASTER_REL, source_paths)
        self.assertEqual(1, self.source_manifest["gameplaySourceVersion"])
        self.assertEqual(1, self.source_manifest["presentationSourceVersion"])
        self.assertEqual(2, self.source_manifest["joinedSourceVersion"])
        repository_join = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        gameplay, presentation = pipeline.split_v2_authoring(
            repository_join,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(self.docs[pipeline.GAMEPLAY_AUTHORING_REL], gameplay)
        self.assertEqual(self.docs[pipeline.PRESENTATION_AUTHORING_REL], presentation)
        joined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(
            pipeline.canonical_bytes(repository_join),
            pipeline.canonical_bytes(joined),
        )
        repository_products = pipeline.project_v2_products(
            self.root, self.docs, repository_join
        )
        joined_products = pipeline.project_v2_products(self.root, self.docs, joined)
        self.assertEqual(
            {path: text.encode("utf-8") for path, text in repository_products.items()},
            {path: text.encode("utf-8") for path, text in joined_products.items()},
        )
        pipeline._preserve_byte_identical_client_products(
            self.root, joined_products
        )
        for relative in (
            pipeline.BINDINGS_REL,
            pipeline.CUES_REL,
            pipeline.COMBAT_PRODUCT_REL,
            pipeline.WORLD_PRODUCT_REL,
        ):
            self.assertEqual(
                (self.root / relative).read_bytes(),
                joined_products[relative].encode("utf-8"),
                relative,
            )
        self.assert_v1_migration_fixture_is_excluded_from_normal_revision_and_load()

    def test_fist_in_out_projects_an_independent_timed_donut_without_a_foreground_hold(
        self,
    ) -> None:
        joined = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        fist = next(
            row for row in joined["patterns"]
            if row["patternId"] == "VALTAN_FIST_IN_OUT"
        )
        self.assertEqual("valtan.attack.fist-in-out.inner", fist["entryActionId"])
        self.assertEqual(1, len(fist["stages"]))
        stage = fist["stages"][0]
        self.assertEqual(
            {
                "stageId": "INNER",
                "actionId": "valtan.attack.fist-in-out.inner",
                "stageKind": "ACTIVE",
                "durationMs": 100,
                "defaultNextActionId": None,
            },
            {
                key: stage[key]
                for key in (
                    "stageId", "actionId", "stageKind", "durationMs",
                    "defaultNextActionId",
                )
            },
        )
        self.assertEqual({"kind": "NONE"}, stage["hit"]["shape"])
        self.assertEqual({"mode": pipeline.ANIMATION_MODE_NONE}, stage["animation"])
        self.assertEqual([], stage["effectCues"])
        self.assertEqual([
            {"eventId": "event.valtan.fist-in-out.spawn-donut",
             "trigger": "ENTER", "kind": "SPAWN_COMBAT_OBJECT",
             "combatObjectArchetypeId": "combatobject.valtan.fist-in-out.donut",
             "count": 1},
        ], stage["events"])
        donut = next(row for row in self.docs[pipeline.COMBAT_AUTHORING_REL]["objects"]
                     if row["combatObjectArchetypeId"] == "combatobject.valtan.fist-in-out.donut")
        self.assertEqual(("FIXED_AREA", 2600, {"kind": "STATIC"}),
                         (donut["kind"], donut["lifetimeMs"], donut["movement"]))
        self.assertEqual({"kind": "BOSS_POSITION", "forwardOffsetM": 0.0,
                          "rightOffsetM": 0.0}, donut["spawn"]["origin"])
        self.assertEqual(1, len(donut["hits"]))
        hit = donut["hits"][0]
        self.assertEqual({"kind": "RING", "innerRadiusM": 8.0, "outerRadiusM": 16.0},
                         hit["shape"])
        self.assertEqual({"kind": "TIMED", "atMs": 1600}, hit["trigger"])
        self.assertGreater(hit["trigger"]["atMs"], stage["durationMs"],
                           "the hit must remain live after the foreground stage has finished")
        self.assertLess(hit["trigger"]["atMs"], donut["lifetimeMs"])

        projected = pipeline.project_v2_products(self.root, self.docs, joined)
        encounter = json.loads(projected[pipeline.ENCOUNTER_REL])
        bindings = json.loads(projected[pipeline.BINDINGS_REL])
        cues = json.loads(projected[pipeline.CUES_REL])
        product_fist = next(
            row for row in encounter["patterns"]
            if row["patternId"] == "VALTAN_FIST_IN_OUT"
        )
        self.assertEqual(["INNER"], [row["stageId"] for row in product_fist["stages"]])
        self.assertEqual(3, bindings["formatVersion"])
        fist_bindings = [
            row for row in bindings["bindings"]
            if row["actionId"].startswith("valtan.attack.fist-in-out.")
        ]
        self.assertEqual(
            [
                "valtan.attack.fist-in-out.windup",
                "valtan.attack.fist-in-out.inner",
                "valtan.attack.fist-in-out.outer",
                "valtan.attack.fist-in-out.recovery",
            ],
            [row["actionId"] for row in fist_bindings],
            "retired stage bindings remain inert NONE tombstones so sealed legacy ordinals do not shift",
        )
        self.assertTrue(all(
            row.get("playbackMode") == pipeline.ANIMATION_MODE_NONE and
            row["clips"] == []
            for row in fist_bindings
        ))
        self.assertEqual(4, cues["formatVersion"])
        self.assertEqual([], [row for row in cues["cues"]
                              if row["patternId"] == "VALTAN_FIST_IN_OUT"])
        product_stage = product_fist["stages"][0]
        self.assertEqual((100, "NONE", 0),
                         (product_stage["durationMs"], product_stage["hitShape"],
                          product_stage["hitCount"]))
        self.assertEqual([{"trigger": "ENTER", "kind": "SPAWN_COMBAT_OBJECT",
                           "targetId": "combatobject.valtan.fist-in-out.donut",
                           "value": 1, "durationMs": 0}], product_stage["actions"])
        product_donut = next(row for row in json.loads(projected[pipeline.COMBAT_PRODUCT_REL])["objects"]
                             if row["combatObjectArchetypeId"] == donut["combatObjectArchetypeId"])
        self.assertEqual(("VALTAN_FIST_IN_OUT", stage["actionId"], 2600, "NONE", 0.0),
                         (product_donut["ownerPatternId"], product_donut["ownerStageActionId"],
                          product_donut["lifeMs"], product_donut["directionPolicy"],
                          product_donut["speedMps"]))
        self.assertEqual(("TIMED", 1600, "RING", 8.0, 16.0),
                         tuple(product_donut["hits"][0][key] for key in
                               ("trigger", "atMs", "hitShape", "hitInnerRadius", "hitOuterRadius")))

    def test_animation_none_and_stage_clock_cue_tagged_unions_fail_closed(
        self,
    ) -> None:
        base = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )

        def fist_stage(document: dict) -> dict:
            return next(
                stage
                for pattern in document["patterns"]
                if pattern["patternId"] == "VALTAN_FIST_IN_OUT"
                for stage in pattern["stages"]
            )

        # The live donut no longer owns a cue or a 2.6-second stage. Preserve
        # the generic NONE/STAGE_CLOCK union coverage with a local valid fixture.
        fixture = fist_stage(base)
        fixture["durationMs"] = 2600
        fixture["effectCues"] = [{
            "cueId": "cue.valtan.carrier-v1.attack.fist-in-out.inner.clip-01",
            "occurrenceId": "cue.valtan.carrier-v1.attack.fist-in-out.inner.clip-01.occurrence.01",
            "effectAssetId": "effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01",
            "timingBasis": pipeline.CUE_TIMING_BASIS_STAGE_CLOCK,
            "stageOffsetMs": 0,
            "anchorSlotId": "root", "followPolicy": "snapshot",
            "stopPolicy": "natural", "repeatPolicy": "once",
            "localTransform": {"position": [0.0, 0.0, 0.0],
                               "rotationDegrees": [0.0, 0.0, 0.0],
                               "scale": [1.0, 1.0, 1.0]},
            "scalePolicy": {"kind": "GAMEPLAY_FOOTPRINT", "worldScale": [1.5, 1.5, 1.5]},
        }]
        pipeline.validate_v2_master(base, self.docs[pipeline.WORLD_SET_REL],
                                    self.docs[pipeline.COMBAT_AUTHORING_REL])
        invalid_documents: list[tuple[str, dict, str]] = []

        none_with_occurrences = copy.deepcopy(base)
        fist_stage(none_with_occurrences)["animation"]["occurrences"] = []
        invalid_documents.append(
            ("NONE with occurrences", none_with_occurrences,
             "animation fields mismatch")
        )

        empty_clip_sequence = copy.deepcopy(base)
        fist_stage(empty_clip_sequence)["animation"] = {
            "mode": pipeline.ANIMATION_MODE_CLIP_SEQUENCE,
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [],
        }
        invalid_documents.append(
            ("empty CLIP_SEQUENCE", empty_clip_sequence,
             "stage has no animation occurrence")
        )

        stage_clock_with_clip = copy.deepcopy(base)
        fist_stage(stage_clock_with_clip)["effectCues"][0][
            "clipOccurrenceId"
        ] = "valtan.attack.fist-in-out.inner.clip.01"
        invalid_documents.append(
            ("STAGE_CLOCK with clip field", stage_clock_with_clip,
             "effectCue fields mismatch")
        )

        stage_clock_with_animation = copy.deepcopy(base)
        fist_stage(stage_clock_with_animation)["animation"] = {
            "mode": pipeline.ANIMATION_MODE_CLIP_SEQUENCE,
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [{
                "clipOccurrenceId": "valtan.attack.fist-in-out.inner.clip.01",
                "clip": "mesh_att_battle_19_02",
                "mappingBasis": "CURRENT_PRODUCT_BASELINE",
                "sourceStartMs": 0,
                "playMs": 2600,
                "playRate": 1.0,
                "repeatUntilStageEnd": False,
            }],
        }
        invalid_documents.append(
            ("STAGE_CLOCK with animation", stage_clock_with_animation,
             "stage-clock cue requires NONE animation mode")
        )

        escaped_stage_offset = copy.deepcopy(base)
        fist_stage(escaped_stage_offset)["effectCues"][0][
            "stageOffsetMs"
        ] = 2600
        invalid_documents.append(
            ("STAGE_CLOCK offset at stage end", escaped_stage_offset,
             "stageOffsetMs out of range")
        )

        looping_stage_clock = copy.deepcopy(base)
        fist_stage(looping_stage_clock)["effectCues"][0][
            "repeatPolicy"
        ] = "each_loop"
        invalid_documents.append(
            ("STAGE_CLOCK each_loop", looping_stage_clock,
             "stage-clock cue must use once")
        )

        for name, document, expected_error in invalid_documents:
            with self.subTest(case=name), self.assertRaisesRegex(
                pipeline.PipelineError, expected_error
            ):
                pipeline.validate_v2_master(
                    document,
                    self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL],
                )

    def test_source_manifest_is_stable_across_platform_line_endings(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            repository_root = Path(temporary) / "repository"
            for row in self.source_manifest["files"]:
                source = self.root / row["path"]
                destination = repository_root / row["path"]
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, destination)

            for row in self.source_manifest["files"]:
                if row["path"] == pipeline.SAVED_FLOW_REL:
                    continue
                path = repository_root / row["path"]
                text = (
                    pipeline.read_text(path)
                    .replace("\r\n", "\n")
                    .replace("\r", "\n")
                )
                path.write_bytes(text.encode("utf-8"))
            lf_manifest = pipeline.source_manifest(repository_root)
            gameplay_path = repository_root / pipeline.GAMEPLAY_AUTHORING_REL
            raw_lf = pipeline.sha256_file(gameplay_path)

            for row in self.source_manifest["files"]:
                if row["path"] == pipeline.SAVED_FLOW_REL:
                    continue
                path = repository_root / row["path"]
                text = pipeline.read_text(path).replace("\n", "\r\n")
                path.write_bytes(text.encode("utf-8"))
            crlf_manifest = pipeline.source_manifest(repository_root)
            raw_crlf = pipeline.sha256_file(gameplay_path)

            for row in self.source_manifest["files"]:
                if row["path"] == pipeline.SAVED_FLOW_REL:
                    continue
                path = repository_root / row["path"]
                text = pipeline.read_text(path).replace("\r\n", "\r")
                path.write_bytes(text.encode("utf-8"))
            cr_manifest = pipeline.source_manifest(repository_root)

            self.assertNotEqual(raw_lf, raw_crlf)
            self.assertEqual(lf_manifest, crlf_manifest)
            self.assertEqual(lf_manifest, cr_manifest)
            flow_path = repository_root / pipeline.SAVED_FLOW_REL
            flow_path.write_bytes(flow_path.read_bytes() + b"\n")
            flow_changed_manifest = pipeline.source_manifest(repository_root)
            self.assertNotEqual(
                cr_manifest["sourceManifestId"],
                flow_changed_manifest["sourceManifestId"],
                "Flow revisions use the exact raw bytes hashed by the Client",
            )

    def test_split_authoring_strict_join_rejects_drift_and_role_leaks(self) -> None:
        gameplay = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        presentation = self.docs[pipeline.PRESENTATION_AUTHORING_REL]

        cases = []

        wrong_pattern = copy.deepcopy(presentation)
        wrong_pattern["patterns"][0]["patternId"] += ".drift"
        cases.append(("patternId", copy.deepcopy(gameplay), wrong_pattern))

        wrong_stage = copy.deepcopy(presentation)
        wrong_stage["patterns"][0]["stages"][0]["stageId"] += ".drift"
        cases.append(("stageId", copy.deepcopy(gameplay), wrong_stage))

        wrong_action = copy.deepcopy(presentation)
        wrong_action["patterns"][0]["stages"][0]["actionId"] += ".drift"
        cases.append(("actionId", copy.deepcopy(gameplay), wrong_action))

        gameplay_role_leak = copy.deepcopy(gameplay)
        gameplay_role_leak["patterns"][0]["sourceSequenceIndex"] = 1
        cases.append(
            ("gameplay role leak", gameplay_role_leak, copy.deepcopy(presentation))
        )

        presentation_role_leak = copy.deepcopy(presentation)
        presentation_role_leak["patterns"][0]["stages"][0]["durationMs"] = 1
        cases.append(
            (
                "presentation role leak",
                copy.deepcopy(gameplay),
                presentation_role_leak,
            )
        )

        wall_budget = copy.deepcopy(gameplay)
        spin = next(
            stage
            for pattern in wall_budget["patterns"]
            if pattern["patternId"] == "VALTAN_WHIRLWIND"
            for stage in pattern["stages"]
            if stage["stageId"] == "SPIN"
        )
        spin["durationMs"] += 100
        cases.append(("stage wall budget", wall_budget, copy.deepcopy(presentation)))

        cue_occurrence = copy.deepcopy(presentation)
        cue = next(
            cue
            for pattern in cue_occurrence["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
        )
        cue["clipOccurrenceId"] = "missing.clip.occurrence"
        cases.append(
            ("cue occurrence", copy.deepcopy(gameplay), cue_occurrence)
        )

        for label, gameplay_case, presentation_case in cases:
            with self.subTest(label=label), self.assertRaises(pipeline.PipelineError):
                pipeline.join_v2_authoring(
                    gameplay_case,
                    presentation_case,
                    self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL],
                )

        missing_volley = copy.deepcopy(gameplay)
        airborne = next(
            stage
            for pattern in missing_volley["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
        )
        airborne["events"] = []
        with self.assertRaisesRegex(pipeline.PipelineError, "HIGH_JUMP/AIRBORNE"):
            pipeline.join_v2_authoring(
                missing_volley,
                copy.deepcopy(presentation),
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        wrong_schedule = copy.deepcopy(gameplay)
        wrong_event = next(
            event
            for pattern in wrong_schedule["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
            for event in stage["events"]
            if event["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY"
        )
        wrong_event["spawnSchedule"]["firstOffsetMs"] = 1
        with self.assertRaisesRegex(
            pipeline.PipelineError, "must start at stage ENTER"
        ):
            pipeline.join_v2_authoring(
                wrong_schedule,
                copy.deepcopy(presentation),
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        single_schedule = copy.deepcopy(gameplay)
        single_event = next(
            event
            for pattern in single_schedule["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
            for event in stage["events"]
            if event["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY"
        )
        single_event["spawnSchedule"]["count"] = 1
        with self.assertRaisesRegex(
            pipeline.PipelineError, "single volley spawn schedule has non-zero interval"
        ):
            pipeline.join_v2_authoring(
                single_schedule,
                copy.deepcopy(presentation),
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        per_wave_capacity = copy.deepcopy(gameplay)
        capacity_event = next(
            event
            for pattern in per_wave_capacity["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
            for event in stage["events"]
            if event["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY"
        )
        capacity_event["maximumTotalObjects"] = 5
        pipeline.join_v2_authoring(
            per_wave_capacity,
            copy.deepcopy(presentation),
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        capacity_event["maximumTotalObjects"] = 4
        with self.assertRaisesRegex(
            pipeline.PipelineError, "cannot admit one wave"
        ):
            pipeline.join_v2_authoring(
                per_wave_capacity,
                copy.deepcopy(presentation),
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        wrong_arena_random = copy.deepcopy(gameplay)
        wrong_event = next(
            event
            for pattern in wrong_arena_random["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
            for event in stage["events"]
            if event["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY"
        )
        wrong_event["arenaRandom"]["kind"] = "RANDOM_CIRCLE"
        with self.assertRaisesRegex(
            pipeline.PipelineError, "unsupported volley arena-random policy"
        ):
            pipeline.join_v2_authoring(
                wrong_arena_random,
                copy.deepcopy(presentation),
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        overflowing_schedule = pipeline.join_v2_authoring(
            copy.deepcopy(gameplay),
            copy.deepcopy(presentation),
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        overflowing_event = next(
            event
            for pattern in overflowing_schedule["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
            for event in stage["events"]
            if event["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY"
        )
        overflowing_stage = next(
            stage for pattern in overflowing_schedule["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"] if stage["stageId"] == "AIRBORNE"
        )
        overflowing_event["spawnSchedule"]["intervalMs"] = (
            overflowing_stage["durationMs"] + 1
        ) // 2
        with self.assertRaisesRegex(
            pipeline.PipelineError, "schedule exceeds its stage duration"
        ):
            pipeline.validate_v2_master(
                overflowing_schedule,
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        missing_phase = copy.deepcopy(gameplay)
        impact = next(
            stage
            for pattern in missing_phase["patterns"]
            if pattern["patternId"] == "VALTAN_ARENA_BREAK_109"
            for stage in pattern["stages"]
            if stage["stageId"] == "IMPACT"
        )
        impact["events"] = [
            event
            for event in impact["events"]
            if event["kind"] != "SET_GAMEPLAY_PHASE"
        ]
        with self.assertRaisesRegex(pipeline.PipelineError, "ARENA_BREAK_109/IMPACT"):
            pipeline.join_v2_authoring(
                missing_phase,
                copy.deepcopy(presentation),
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

    def assert_v1_migration_fixture_is_excluded_from_normal_revision_and_load(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            repository_root = Path(temporary) / "repository"
            for row in self.source_manifest["files"]:
                source = self.root / row["path"]
                destination = repository_root / row["path"]
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, destination)
            fixture = repository_root / pipeline.MASTER_REL
            fixture.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(self.root / pipeline.MASTER_REL, fixture)

            before = pipeline.source_manifest(repository_root)
            fixture.write_bytes(b"not migration JSON\n")
            after = pipeline.source_manifest(repository_root)
            self.assertEqual(before, after)
            normal_docs = pipeline.load_pipeline_documents(repository_root)
            self.assertNotIn(pipeline.MASTER_REL, normal_docs)
            validation = pipeline.validate_repository(repository_root)
            self.assertEqual(1, validation["gameplaySourceVersion"])
            self.assertEqual(1, validation["presentationSourceVersion"])
            self.assertEqual(2, validation["joinedSourceVersion"])

            patch = self.draft_patch()
            patch["sourceRevision"] = before["sourceManifestId"]
            saved = pipeline.save_authoring(
                repository_root,
                Path(temporary) / "authoring",
                patch,
            )
            self.assertRegex(saved["revisionId"], r"^[0-9a-f]{64}$")

            with self.assertRaises(pipeline.PipelineError):
                pipeline.load_pipeline_documents(
                    repository_root,
                    include_split_authoring=False,
                    include_migration_fixture=True,
                )

    def test_world_set_exact_contract_and_pass_through(self) -> None:
        companion = self.docs[pipeline.WORLD_SET_REL]
        self.assertEqual(
            {"schema", "formatVersion", "areaId", "encounterId", "sets"},
            set(companion),
        )
        self.assertEqual(3, len(companion["sets"]))
        event_sets = {
            row["worldEventSetId"]: row for row in companion["sets"]
        }
        self.assertEqual(set(pipeline.WORLD_SET_OWNERS), set(event_sets))
        self.assertEqual(97, len(event_sets[pipeline.WORLD_SET_ID]["members"]))
        groups = {
            group["groupId"]: group
            for group in self.docs[pipeline.WORLD_PRODUCT_REL]["groups"]
        }
        placement_ids = [
            placement_id
            for member in event_sets[pipeline.WORLD_SET_ID]["members"]
            for placement_id in groups[member["groupId"]]["memberPlacementIds"]
        ]
        self.assertEqual(135, len(placement_ids))
        self.assertEqual(135, len(set(placement_ids)))
        self.assertEqual(
            3,
            len(
                event_sets[
                    "worldeventset.valtan.terrain-destruction-3.floor84"
                ]["members"]
            ),
        )
        self.assertEqual(
            3,
            len(
                event_sets[
                    "worldeventset.valtan.terrain-destruction-9.floor30"
                ]["members"]
            ),
        )
        member_fields = {
            "memberId", "bindingId", "groupId", "mutationId", "offsetMs",
            "receiverCollisionId", "enabled",
        }
        self.assertTrue(
            all(
                set(member) == member_fields
                for event_set in event_sets.values()
                for member in event_set["members"]
            )
        )
        pipeline.validate_world_event_sets(
            companion,
            self.docs[pipeline.WORLD_PRODUCT_REL],
            migration_fixture=False,
        )
        migrated = self.migrate()
        projected_text = pipeline.project_v2_products(
            self.root, self.docs, migrated, migration_fixture=True
        )[
            pipeline.WORLD_PRODUCT_REL
        ]
        source_text = pipeline.read_text(self.root / pipeline.WORLD_PRODUCT_REL)
        source = self.docs[pipeline.WORLD_PRODUCT_REL]
        projected = json.loads(projected_text)
        self.assertEqual((105, 105, 224), (len(projected["groups"]), len(projected["mutations"]), len(projected["bindings"])))
        managed_ids = {
            member["bindingId"]
            for member in event_sets[pipeline.WORLD_SET_ID]["members"]
        }
        self.assertEqual(97, len(managed_ids))
        source_other = [row for row in source["bindings"] if row["bindingId"] not in managed_ids]
        projected_other = [row for row in projected["bindings"] if row["bindingId"] not in managed_ids]
        self.assertEqual(127, len(source_other))
        self.assertEqual(source_other, projected_other)
        self.assertEqual(source["groups"], projected["groups"])
        self.assertEqual(source["mutations"], projected["mutations"])
        self.assertEqual(source["provenance"], projected["provenance"])
        pipeline._assert_unmanaged_raw_rows_preserved(
            source_text, projected_text, "bindings", "bindingId", managed_ids
        )

    def test_combat_companion_has_only_object_owned_fields(self) -> None:
        companion = self.docs[pipeline.COMBAT_AUTHORING_REL]
        pipeline.validate_combat_authoring(companion)
        self.assertEqual({"combatobject.valtan.high-jump.target-axe",
                          "combatobject.valtan.red-blade-wave.projectile",
                          "combatobject.valtan.fist-in-out.donut"},
                         {row["combatObjectArchetypeId"] for row in companion["objects"]})
        axe = next(
            row for row in companion["objects"]
            if row["combatObjectArchetypeId"] == "combatobject.valtan.high-jump.target-axe"
        )
        self.assertEqual({"kind": "RESOLVED_VOLLEY_POSITION"}, axe["spawn"]["origin"])
        forbidden = {"ownerPatternId", "ownerStageActionId", "lifeMs", "clientVisualId"}
        for row in companion["objects"]:
            self.assertFalse(forbidden & set(row), row["combatObjectArchetypeId"])

    def test_legacy_manifest_seals_unmanaged_closure(self) -> None:
        legacy = self.docs[pipeline.LEGACY_REL]
        pipeline.validate_legacy_manifest(legacy, set(pipeline.MANAGED_PATTERN_IDS))
        self.assertEqual(26, len(legacy["patternEntries"]))
        red_blade = next(
            row for row in legacy["legacyCombatObjectOwners"]
            if row["combatObjectArchetypeId"] == "combatobject.valtan.red-blade-wave.projectile"
        )
        self.assertEqual("VALTAN_RED_BLADE_WAVE", red_blade["ownerPatternId"])
        mixed = next(
            row for row in legacy["sharedRotationRows"]
            if row["rotationId"] == "rotation.valtan.100.84"
        )
        self.assertEqual(
            ["LEGACY", "LEGACY", "MANAGED", "LEGACY", "MANAGED"],
            [ref["ownership"] for ref in mixed["patternRefs"]],
        )

    def test_legacy_effect_cue_validation_uses_stable_binding_identity(self) -> None:
        drifted_docs = copy.deepcopy(self.docs)
        sealed_cue = next(
            cue
            for entry in drifted_docs[pipeline.LEGACY_REL]["patternEntries"]
            for cue in entry["effectCues"]
        )
        sealed_binding_id = sealed_cue["runtimeCue"]["bindingId"]
        cue_rows = drifted_docs[pipeline.CUES_REL]["cues"]
        sealed_ordinal = next(
            ordinal
            for ordinal, cue in enumerate(cue_rows)
            if cue["bindingId"] == sealed_binding_id
        )
        managed_pattern_ids = {
            pattern["patternId"]
            for pattern in pipeline.join_v2_authoring(
                self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
                self.docs[pipeline.PRESENTATION_AUTHORING_REL],
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )["patterns"]
        }
        managed_ordinal = next(
            ordinal
            for ordinal, cue in enumerate(cue_rows[:sealed_ordinal])
            if cue["patternId"] in managed_pattern_ids
        )
        del cue_rows[managed_ordinal]
        pipeline.validate_legacy_products(
            drifted_docs[pipeline.LEGACY_REL],
            drifted_docs,
            managed_pattern_ids,
        )

        sealed_row = next(
            cue for cue in cue_rows if cue["bindingId"] == sealed_binding_id
        )
        sealed_row["effectAssetId"] += ".drift"
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "sealed legacy effect cue drift.*" + sealed_binding_id,
        ):
            pipeline.validate_legacy_products(
                drifted_docs[pipeline.LEGACY_REL],
                drifted_docs,
                managed_pattern_ids,
            )

    def test_strict_negative_fixtures(self) -> None:
        world_sets = copy.deepcopy(self.docs[pipeline.WORLD_SET_REL])
        world_sets["sets"][0]["members"][0]["patternId"] = pipeline.WORLD_PATTERN_ID
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_world_event_sets(
                world_sets, self.docs[pipeline.WORLD_PRODUCT_REL], migration_fixture=True
            )
        world_sets = copy.deepcopy(self.docs[pipeline.WORLD_SET_REL])
        world_sets["sets"][0]["members"][1]["memberId"] = world_sets["sets"][0]["members"][0]["memberId"]
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_world_event_sets(
                world_sets, self.docs[pipeline.WORLD_PRODUCT_REL], migration_fixture=True
            )
        world_sets = copy.deepcopy(self.docs[pipeline.WORLD_SET_REL])
        world_sets["sets"][0]["members"][0]["groupId"] = "destroyable.group.invalid"
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_world_event_sets(
                world_sets, self.docs[pipeline.WORLD_PRODUCT_REL], migration_fixture=True
            )
        migrated = self.migrate()
        cue = next(
            cue
            for pattern in migrated["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
        )
        cue["stageOffsetMs"] = 10
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_v2_master(
                migrated,
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )
        drifted_docs = copy.deepcopy(self.docs)
        legacy_ordinal = self.docs[pipeline.LEGACY_REL]["patternEntries"][0]["encounterOrdinal"]
        drifted_docs[pipeline.ENCOUNTER_REL]["patterns"][legacy_ordinal]["displayName"] += " drift"
        with self.assertRaisesRegex(pipeline.PipelineError, "sealed legacy encounter row drift"):
            pipeline.validate_legacy_products(
                self.docs[pipeline.LEGACY_REL],
                drifted_docs,
                {
                    row["patternId"]
                    for row in pipeline.join_v2_authoring(
                        self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
                        self.docs[pipeline.PRESENTATION_AUTHORING_REL],
                        self.docs[pipeline.WORLD_SET_REL],
                        self.docs[pipeline.COMBAT_AUTHORING_REL],
                    )["patterns"]
                },
            )

    def test_mapping_basis_vocabulary_rejects_occurrence_and_cue_typos(self) -> None:
        self.assertEqual(
            frozenset(
                {
                    "CURRENT_PRODUCT_BASELINE",
                    "PATTERN_PR_REFERENCE",
                    "ANIMATION_PR_127",
                    "SOURCE_REVIEWED_DELTA",
                    "PROJECT_AUTHORED",
                    "LEGACY_V1_MIGRATION",
                }
            ),
            pipeline.ALLOWED_MAPPING_BASES,
        )
        invalid_occurrence = self.migrate()
        occurrence = next(
            occurrence
            for pattern in invalid_occurrence["patterns"]
            for stage in pattern["stages"]
            for occurrence in stage["animation"]["occurrences"]
        )
        occurrence["mappingBasis"] = "PROJECT_AUTHORDE"
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "mappingBasis vocabulary",
        ):
            pipeline.validate_v2_master(
                invalid_occurrence,
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        invalid_cue = self.migrate()
        cue = next(
            cue
            for pattern in invalid_cue["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
        )
        cue["mappingBasis"] = "PROJECT_AUTHORDE"
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "mappingBasis vocabulary",
        ):
            pipeline.validate_v2_master(
                invalid_cue,
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

    def test_server_motion_rejects_stage_order_and_window_drift(self) -> None:
        def high_jump(document: dict) -> dict:
            return next(
                pattern
                for pattern in document["patterns"]
                if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            )

        travel_takeoff = copy.deepcopy(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        )
        high_jump(travel_takeoff)["serverMotion"]["travelStageId"] = "TAKEOFF"
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "travel stage must follow the entry stage",
        ):
            pipeline.validate_gameplay_authoring(travel_takeoff)

        takeoff_overrun = copy.deepcopy(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        )
        takeoff_pattern = high_jump(takeoff_overrun)
        takeoff_pattern["serverMotion"]["takeoffEndMs"] = (
            takeoff_pattern["stages"][0]["durationMs"] + 1
        )
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "travel window is outside its stage",
        ):
            pipeline.validate_gameplay_authoring(takeoff_overrun)

        travel_overrun = copy.deepcopy(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        )
        travel_pattern = high_jump(travel_overrun)
        travel_stage_id = travel_pattern["serverMotion"]["travelStageId"]
        travel_stage = next(
            stage
            for stage in travel_pattern["stages"]
            if stage["stageId"] == travel_stage_id
        )
        travel_pattern["serverMotion"]["travelEndMs"] = (
            travel_stage["durationMs"] + 1
        )
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "travel window is outside its stage",
        ):
            pipeline.validate_gameplay_authoring(travel_overrun)

        missing_window = copy.deepcopy(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        )
        del high_jump(missing_window)["serverMotion"]["travelEndMs"]
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "serverMotion fields mismatch",
        ):
            pipeline.validate_gameplay_authoring(missing_window)

    def test_trash_capture_deadlines_and_atomic_impacts_project_losslessly(self) -> None:
        master = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            pipeline.build_world_event_sets(self.docs[pipeline.WORLD_PRODUCT_REL]),
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        trash = next(row for row in master["patterns"] if row["patternId"] == "VALTAN_TRASH")
        stages = {row["stageId"]: row for row in trash["stages"]}
        self.assertNotIn("counterProxy", stages["STEP_06"])
        self.assertEqual([], stages["STEP_06"]["events"])
        self.assertEqual(stages["STEP_07"]["actionId"], stages["STEP_06"]["defaultNextActionId"])
        self.assertEqual(["ANY_PLAYER_GRABBED", "NAVIGATION_BLOCKED", "TIMEOUT"],
                         [row["outcome"] for row in stages["STEP_08"]["branches"]])
        self.assertLess(max(stages["STEP_08"]["hit"]["schedule"]["offsetsMs"]),
                        stages["STEP_08"]["durationMs"])
        for stage_id, source_start, duration in (
            ("CATCH_COUNTER", 0, 200), ("CATCH_PRE_IMPACT", 200, 1300),
            ("CATCH_SLAM", 1500, 1500), ("EXECUTE_TAIL", 1500, 1500),
        ):
            with self.subTest(stage=stage_id):
                row = stages[stage_id]
                occurrence = row["animation"]["occurrences"][0]
                self.assertEqual(duration, row["durationMs"])
                self.assertEqual(("mesh_att_battle_13_05-1", source_start, duration),
                    (occurrence["clip"], occurrence["sourceStartMs"], occurrence["playMs"]))
        self.assertEqual(["ALL_PLAYERS_GRABBED", "TIMEOUT"],
                         [row["outcome"] for row in stages["CATCH_PRE_IMPACT"]["branches"]])
        self.assertEqual("DAMAGE_GRABBED_PLAYERS", stages["CATCH_SLAM"]["events"][0]["kind"])
        self.assertEqual("EXECUTE_GRABBED_PLAYERS", stages["EXECUTE_TAIL"]["events"][0]["kind"])
        for terminal in ("CATCH_SLAM", "EXECUTE_TAIL", "RUSH_MISS", "GROGGY"):
            self.assertIsNone(stages[terminal]["defaultNextActionId"])
            self.assertEqual([{"outcome": "TIMEOUT", "nextActionId": None}],
                             stages[terminal]["branches"])
        self.assertEqual(4433, sum(row["playMs"] for row in stages["GROGGY"]["animation"]["occurrences"]))
        self.assertIn(420631, trash["sourceActionIds"])

        _, _, outputs = pipeline.build_repository_product_projection(self.root)
        product = next(row for row in json.loads(outputs[pipeline.ENCOUNTER_REL])["patterns"]
                       if row["patternId"] == "VALTAN_TRASH")
        projected = {row["stageId"]: row for row in product["stages"]}
        for stage_id, kind, target in (
            ("CATCH_SLAM", "DAMAGE_GRABBED_PLAYERS", "damage.valtan.charge-grab-roar"),
            ("EXECUTE_TAIL", "EXECUTE_GRABBED_PLAYERS", "boss.attachment.left-hand"),
        ):
            self.assertEqual([{"trigger": "ENTER", "kind": kind, "targetId": target,
                               "value": 0, "durationMs": 0}], projected[stage_id]["actions"])
        for mutate in ("exit", "extra", "mixed", "bad_profile"):
            invalid = copy.deepcopy(master)
            bad = next(row for row in invalid["patterns"] if row["patternId"] == "VALTAN_TRASH")
            bad_stage = next(row for row in bad["stages"] if row["stageId"] == "CATCH_SLAM")
            if mutate == "exit": bad_stage["events"][0]["trigger"] = "EXIT"
            if mutate == "extra": bad_stage["events"][0]["value"] = 100
            if mutate == "mixed": bad_stage["events"].append(copy.deepcopy(stages["STEP_07"]["events"][0]))
            if mutate == "bad_profile": bad_stage["events"][0]["damageProfileId"] = "boss.invalid"
            with self.subTest(mutation=mutate), self.assertRaises(pipeline.PipelineError):
                pipeline.validate_v2_master(invalid,
                    pipeline.build_world_event_sets(self.docs[pipeline.WORLD_PRODUCT_REL]),
                    self.docs[pipeline.COMBAT_AUTHORING_REL])

    def test_grab_hit_and_release_actions_project_losslessly(self) -> None:
        gameplay = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]

        def pattern(pattern_id: str) -> dict:
            return next(
                row for row in gameplay["patterns"]
                if row["patternId"] == pattern_id
            )

        def stage(owner: dict, stage_id: str) -> dict:
            return next(
                row for row in owner["stages"]
                if row["stageId"] == stage_id
            )

        trash = pattern("VALTAN_TRASH")
        trash_counter = stage(trash, "STEP_07")
        trash_release = stage(trash, "GROGGY")
        trash_rush = stage(trash, "STEP_08")
        catch = pattern("VALTAN_CATCH_BREATH")
        catch_grab = stage(catch, "STEP_02")
        catch_release = stage(catch, "STEP_04")
        self.assertEqual(
            ("LOCK_RANDOM_ALIVE_ON_START", "LOCK_FACING_ON_START"),
            (trash["targetPolicy"], trash["aimPolicy"]),
        )
        self.assertEqual(
            {
                "space": "BOSS_LOCAL",
                "forwardOffsetM": 1.0,
                "rightOffsetM": 0.0,
                "radiusM": 2.25,
            },
            trash_counter["counterProxy"],
        )
        self.assertEqual(
            ("LOCK_RANDOM_ALIVE_BEHIND_ON_START", "LOCK_FACING_ON_START"),
            (catch["targetPolicy"], catch["aimPolicy"]),
        )
        self.assertEqual(
            ("CAPTURE", "BOSS_LEFT_HAND", "BOX"),
            (
                trash_rush["hit"]["playerResponse"],
                trash_rush["hit"]["attachmentSlot"],
                trash_rush["hit"]["shape"]["kind"],
            ),
        )
        self.assertEqual(
            ["COUNTER_HIT", "TIMEOUT"],
            [branch["outcome"] for branch in trash_counter["branches"]],
        )
        self.assertEqual(
            ("HOLD", 0.0, 0),
            tuple(
                next(
                    event for event in trash_release["events"]
                    if event["kind"] == "RELEASE_GRABBED_PLAYERS"
                )[key]
                for key in ("releaseMode", "speedMps", "durationMs")
            ),
        )
        self.assertEqual(
            ("CAPTURE", "BOSS_LEFT_HAND", "CONE"),
            (
                catch_grab["hit"]["playerResponse"],
                catch_grab["hit"]["attachmentSlot"],
                catch_grab["hit"]["shape"]["kind"],
            ),
        )
        self.assertEqual(
            ("ARENA_EJECTION", 24.0, 500),
            tuple(
                next(
                    event for event in catch_release["events"]
                    if event["kind"] == "RELEASE_GRABBED_PLAYERS"
                )[key]
                for key in ("releaseMode", "speedMps", "durationMs")
            ),
        )

        _, _, outputs = pipeline.build_repository_product_projection(self.root)
        encounter = json.loads(outputs[pipeline.ENCOUNTER_REL])
        projected = {
            row["patternId"]: row for row in encounter["patterns"]
        }
        projected_trash = {
            row["stageId"]: row
            for row in projected["VALTAN_TRASH"]["stages"]
        }
        projected_catch = {
            row["stageId"]: row
            for row in projected["VALTAN_CATCH_BREATH"]["stages"]
        }
        self.assertEqual(
            ("CAPTURE", "BOSS_LEFT_HAND"),
            (
                projected_trash["STEP_08"]["playerResponse"],
                projected_trash["STEP_08"]["attachmentSlot"],
            ),
        )
        self.assertEqual(
            trash_counter["counterProxy"],
            projected_trash["STEP_07"]["counterProxy"],
        )
        self.assertIn(
            {
                "trigger": "ENTER",
                "kind": "RELEASE_GRABBED_PLAYERS",
                "targetId": "boss.attachment.left-hand",
                "releaseMode": "HOLD",
                "speedMps": 0.0,
                "durationMs": 0,
            },
            projected_trash["GROGGY"]["actions"],
        )
        self.assertEqual(
            ("CAPTURE", "BOSS_LEFT_HAND"),
            (
                projected_catch["STEP_02"]["playerResponse"],
                projected_catch["STEP_02"]["attachmentSlot"],
            ),
        )
        self.assertIn(
            {
                "trigger": "ENTER",
                "kind": "RELEASE_GRABBED_PLAYERS",
                "targetId": "boss.attachment.left-hand",
                "releaseMode": "ARENA_EJECTION",
                "speedMps": 24.0,
                "durationMs": 500,
            },
            projected_catch["STEP_04"]["actions"],
        )

        partial = copy.deepcopy(gameplay)
        partial_trash = next(
            row for row in partial["patterns"]
            if row["patternId"] == "VALTAN_TRASH"
        )
        del next(
            row for row in partial_trash["stages"]
            if row["stageId"] == "STEP_08"
        )["hit"]["attachmentSlot"]
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_gameplay_authoring(partial)

        exit_release = pipeline._compile_event(
            {
                "eventId": "event.valtan.test.release-on-exit",
                "trigger": "EXIT",
                "kind": "RELEASE_GRABBED_PLAYERS",
                "releaseMode": "HOLD",
                "speedMps": 0.0,
                "durationMs": 0,
            },
            "VALTAN_TRASH",
            "STEP_07",
        )
        self.assertEqual("EXIT", exit_release["trigger"])

    def test_target_rush_and_finale_corner_motion_project_losslessly(self) -> None:
        master = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL])
        outputs = pipeline.project_v2_products(self.root, self.docs, master)
        product = {row["patternId"]: row for row in json.loads(outputs[pipeline.ENCOUNTER_REL])["patterns"]}
        owners = {row["patternId"]: row for row in master["patterns"]}
        warp_legs = [stage for stage in owners["VALTAN_WARP"]["stages"]
                     if stage["motion"] is not None]
        finale_legs = [stage for stage in owners["VALTAN_GHOST_FINALE"]["stages"]
                       if stage["motion"] is not None]
        self.assertEqual([{"kind": "PORTAL_TARGET_RUSH"}] * 8,
                         [stage["motion"] for stage in warp_legs])
        self.assertEqual([
            {"kind": "PORTAL_CROSS_ARENA", "cornerIndex": index % 4,
             "halfExtentsM": [22.0, 22.0]}
            for index in range(8)
        ], [stage["motion"] for stage in finale_legs])
        presentation = {
            row["patternId"]: row
            for row in self.docs[pipeline.PRESENTATION_AUTHORING_REL]["patterns"]
        }
        for stage in presentation["VALTAN_WARP"]["stages"][1:9]:
            cue = stage["effectCues"][0]
            self.assertEqual("follow", cue["followPolicy"])
            self.assertEqual("root", cue["anchorSlotId"])
            self.assertEqual([0.0, 0.0, 3.0], cue["localTransform"]["position"])
        for stage in presentation["VALTAN_GHOST_FINALE"]["stages"][1:9]:
            cue = stage["effectCues"][0]
            self.assertEqual("snapshot", cue["followPolicy"])
            self.assertEqual("root", cue["anchorSlotId"])
            self.assertEqual([0.0, 0.0, 0.0], cue["localTransform"]["position"])
        for pattern_id in ("VALTAN_WARP", "VALTAN_GHOST_FINALE"):
            with self.subTest(pattern=pattern_id):
                owner = owners[pattern_id]
                legs = [stage for stage in owner["stages"] if stage["motion"] is not None]
                self.assertEqual([2000, *([900] * 8), 1667],
                                 [stage["durationMs"] for stage in owner["stages"]])
                self.assertIsNone(owner["stages"][-1]["defaultNextActionId"])
                self.assertEqual("RETURN_TO_ARENA_CENTER", owner["stages"][-1]["events"][0]["kind"])
                projected_stages = {row["stageId"]: row for row in product[pattern_id]["stages"]}
                for stage in legs:
                    self.assertEqual(stage["motion"], projected_stages[stage["stageId"]]["motion"])
                    self.assertEqual("RETARGET_RANDOM_ALIVE", projected_stages[stage["stageId"]]["actions"][0]["kind"])
                self.assertEqual([{"trigger": "ENTER", "kind": "RETURN_TO_ARENA_CENTER",
                                   "targetId": "boss.arena.center", "value": 1, "durationMs": 0}],
                                 projected_stages[owner["stages"][-1]["stageId"]]["actions"])

    def test_portal_motion_rejects_invalid_owner_shape_and_kind(self) -> None:
        base = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        finale_mutations = [
            ("negative corner", lambda motion: motion.update(cornerIndex=-1)),
            ("fifth corner", lambda motion: motion.update(cornerIndex=4)),
            ("boolean corner", lambda motion: motion.update(cornerIndex=True)),
            ("missing corner", lambda motion: motion.pop("cornerIndex")),
            ("wrong extent dimensions", lambda motion: motion.update(halfExtentsM=[22.0])),
            ("zero extent", lambda motion: motion.update(halfExtentsM=[0.0, 22.0])),
            ("nonfinite extent", lambda motion: motion.update(halfExtentsM=[float("nan"), 22.0])),
            ("unknown motion kind", lambda motion: motion.update(kind="PORTAL_NAV_FALLBACK")),
            ("mixed forward fields", lambda motion: motion.update(distance=44.0)),
        ]
        for name, mutate in finale_mutations:
            invalid = copy.deepcopy(base)
            portal = next(row for row in invalid["patterns"]
                          if row["patternId"] == "VALTAN_GHOST_FINALE")
            mutate(portal["stages"][1]["motion"])
            with self.subTest(case=name), self.assertRaises(pipeline.PipelineError):
                pipeline.validate_gameplay_authoring(invalid)
        for name, mutate in (
            ("target rush extra field", lambda motion: motion.update(distance=7.2654)),
            ("target rush unknown kind", lambda motion: motion.update(kind="PORTAL_NAV_FALLBACK")),
            ("warp changed to corner", lambda motion: motion.update(
                kind="PORTAL_CROSS_ARENA", cornerIndex=0, halfExtentsM=[22.0, 22.0])),
        ):
            invalid = copy.deepcopy(base)
            portal = next(row for row in invalid["patterns"]
                          if row["patternId"] == "VALTAN_WARP")
            mutate(portal["stages"][1]["motion"])
            with self.subTest(case=name), self.assertRaises(pipeline.PipelineError):
                pipeline.validate_gameplay_authoring(invalid)

    def test_ghost_finale_projects_a_finite_cycle_and_three_scripted_attacks(self) -> None:
        master = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL])
        patterns = {row["patternId"]: row for row in master["patterns"]}
        finale = patterns["VALTAN_GHOST_FINALE"]
        self.assertEqual({
            "kind": "GHOST_PORTAL_LOOP", "ghostArchetypeId": "BOSS_VALTAN_GHOST",
            "ghostPatternIds": ["VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH", "VALTAN_SEQUENCE_FOUR"],
            "spawnHalfExtentsM": [10.0, 10.0], "maximumActiveGhosts": 1}, finale["finale"])
        self.assertIsNone(finale["stages"][-1]["defaultNextActionId"],
                          "the persistent outer run must not be a cyclic stage graph")
        for pattern_id in ["VALTAN_GHOST_FINALE", *finale["finale"]["ghostPatternIds"]]:
            pipeline._validate_finite_pattern_graph(patterns[pattern_id])
        self.assertEqual("CROSS", next(stage for stage in patterns["VALTAN_SEQUENCE_FOUR"]["stages"]
                                       if stage["hit"]["shape"]["kind"] != "NONE")["hit"]["shape"]["kind"])
        outputs = pipeline.project_v2_products(self.root, self.docs, master)
        product = next(row for row in json.loads(outputs[pipeline.ENCOUNTER_REL])["patterns"]
                       if row["patternId"] == "VALTAN_GHOST_FINALE")
        self.assertEqual(finale["finale"], product["finale"])
        self.assertEqual("AUDITION_ONLY", product["selectionMode"])
        self.assertIn("VALTAN_GHOST_FINALE", {
            row["patternId"] for row in master["decisionModel"]["manualAuditions"]})
        self.assertEqual(
            [row["patternId"]
             for row in self.docs[pipeline.SAVED_FLOW_REL]["flows"][0]["slots"]],
            master["decisionModel"]["scriptedSequence"]["patternIds"],
            "joining derived patterns must preserve the user's saved Flow slots exactly",
        )

    def test_ghost_finale_rejects_invalid_owner_shape_and_recursive_graphs(self) -> None:
        base = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        def owner(document):
            return next(row for row in document["patterns"] if row["patternId"] == "VALTAN_GHOST_FINALE")
        mutations = [
            ("unknown kind", lambda row: row["finale"].update(kind="INFINITE_STAGE")),
            ("invulnerable owner cannot finish by death", lambda row: row.update(invulnerableWhileRunning=True)),
            ("wrong entity role", lambda row: row["finale"].update(ghostArchetypeId="BOSS_VALTAN")),
            ("two children", lambda row: row["finale"].update(ghostPatternIds=["VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH"])),
            ("duplicate child", lambda row: row["finale"].update(ghostPatternIds=["VALTAN_WHIRLWIND"] * 3)),
            ("missing child", lambda row: row["finale"]["ghostPatternIds"].__setitem__(0, "VALTAN_UNKNOWN")),
            ("self child", lambda row: row["finale"]["ghostPatternIds"].__setitem__(0, row["patternId"])),
            ("terrain child", lambda row: row["finale"]["ghostPatternIds"].__setitem__(0, "VALTAN_ARENA_BREAK_109")),
            ("unbounded child count", lambda row: row["finale"].update(maximumActiveGhosts=2)),
            ("wrong extent dimensions", lambda row: row["finale"].update(spawnHalfExtentsM=[10.0, 10.0, 10.0])),
            ("zero extent", lambda row: row["finale"].update(spawnHalfExtentsM=[0.0, 10.0])),
            ("nonfinite extent", lambda row: row["finale"].update(spawnHalfExtentsM=[float("inf"), 10.0])),
            ("missing field", lambda row: row["finale"].pop("maximumActiveGhosts")),
            ("cyclic main graph", lambda row: row["stages"][-1].update(defaultNextActionId=row["entryActionId"])),
        ]
        for name, mutate in mutations:
            invalid = copy.deepcopy(base)
            mutate(owner(invalid))
            with self.subTest(case=name), self.assertRaises(pipeline.PipelineError):
                pipeline.validate_gameplay_authoring(invalid)
        for recursive in (False, True):
            invalid = copy.deepcopy(base)
            child = next(row for row in invalid["patterns"] if row["patternId"] == "VALTAN_WHIRLWIND")
            if recursive:
                child["finale"] = copy.deepcopy(owner(invalid)["finale"])
            else:
                child["stages"][-1]["defaultNextActionId"] = child["entryActionId"]
            with self.subTest(recursive=recursive), self.assertRaises(pipeline.PipelineError):
                pipeline.validate_gameplay_authoring(invalid)

    def test_all_four_trash_variants_reject_stage_cycles_and_non_capture_nav_branches(self) -> None:
        base = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        ids = ("VALTAN_TRASH", "VALTAN_TRASH_CATCH_IF",
               "VALTAN_TRASH_CATCH_SUCCESS", "VALTAN_TRASH_CATCH_FAIL")
        for pattern_id in ids:
            invalid = copy.deepcopy(base)
            pattern = next(row for row in invalid["patterns"] if row["patternId"] == pattern_id)
            pipeline._validate_finite_pattern_graph(pattern)
            pattern["stages"][-1]["defaultNextActionId"] = pattern["entryActionId"]
            with self.subTest(pattern=pattern_id), self.assertRaisesRegex(pipeline.PipelineError, "finite stage graph contains a cycle"):
                pipeline.validate_gameplay_authoring(invalid)
        invalid = copy.deepcopy(base)
        trash = next(row for row in invalid["patterns"] if row["patternId"] == "VALTAN_TRASH")
        rush = next(row for row in trash["stages"] if row["stageId"] == "STEP_08")
        del rush["hit"]["playerResponse"]
        del rush["hit"]["attachmentSlot"]
        with self.assertRaisesRegex(pipeline.PipelineError, "navigation-blocked outcome requires a capture rush"):
            pipeline.validate_gameplay_authoring(invalid)

    def test_center_jump_and_pizza_facing_anchor_project_without_world_rotation_loss(self) -> None:
        master = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL])
        expected = {"VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK": "arena.center",
                    "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK": "arena.center",
                    "VALTAN_SIX_PIZZA_106": "arena.center.facing"}
        for row in master["patterns"]:
            if row["patternId"] not in expected:
                continue
            self.assertEqual("LEAP_TO_ANCHOR", row["serverMotion"]["kind"])
            self.assertTrue(row["serverMotion"]["moveToAnchorBeforeTakeoff"])
            self.assertGreater(row["serverMotion"]["takeoffStartMs"], 0)
            self.assertEqual([156.03, 22.99751, -122.06], row["serverMotion"]["landingPosition"])
            cues = [cue for stage in row["stages"] for cue in stage["effectCues"]]
            self.assertTrue(cues)
            for cue in cues:
                self.assertEqual(expected[row["patternId"]], cue["anchorSlotId"])
                self.assertEqual("snapshot", cue["followPolicy"])
                cue["localTransform"]["rotationDegrees"][1] = 37.0
            if row["patternId"] == "VALTAN_SIX_PIZZA_106":
                self.assertEqual(("LOCK_RANDOM_ALIVE_ON_START", "LOCK_FACING_ON_START"),
                                 (row["targetPolicy"], row["aimPolicy"]))
        outputs = pipeline.project_v2_products(self.root, self.docs, master)
        product = {row["patternId"]: row for row in json.loads(outputs[pipeline.ENCOUNTER_REL])["patterns"]}
        cues = json.loads(outputs[pipeline.CUES_REL])["cues"]
        for pattern_id, anchor in expected.items():
            authored = next(row for row in master["patterns"] if row["patternId"] == pattern_id)
            self.assertEqual(authored["serverMotion"], product[pattern_id]["serverMotion"])
            projected = [cue for cue in cues if cue["patternId"] == pattern_id]
            self.assertTrue(projected)
            self.assertTrue(all(cue["anchorSlotId"] == anchor and
                                cue["localTransform"]["rotationDegrees"][1] == 37.0
                                for cue in projected))

    def test_center_anchor_rejects_missing_motion_moving_roots_and_unlocked_facing(self) -> None:
        base = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL])
        mutations = [
            ("missing landing authority", lambda row, cue: row.update(serverMotion=None)),
            ("no preposition", lambda row, cue: row["serverMotion"].update(moveToAnchorBeforeTakeoff=False)),
            ("moving effect root", lambda row, cue: cue.update(followPolicy="follow")),
            ("unknown reserved anchor", lambda row, cue: cue.update(anchorSlotId="arena.center.unknown")),
            ("unlocked facing", lambda row, cue: row.update(targetPolicy="NONE", aimPolicy="NONE")),
        ]
        for name, mutate in mutations:
            invalid = copy.deepcopy(base)
            pizza = next(row for row in invalid["patterns"] if row["patternId"] == "VALTAN_SIX_PIZZA_106")
            cue = next(cue for stage in pizza["stages"] for cue in stage["effectCues"])
            mutate(pizza, cue)
            with self.subTest(case=name), self.assertRaises(pipeline.PipelineError):
                pipeline.validate_v2_master(invalid, self.docs[pipeline.WORLD_SET_REL],
                                            self.docs[pipeline.COMBAT_AUTHORING_REL])

    def test_high_jump_clock_and_motion_project_losslessly(self) -> None:
        gameplay = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        presentation = self.docs[pipeline.PRESENTATION_AUTHORING_REL]
        gameplay_pattern = next(
            pattern
            for pattern in gameplay["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
        )
        presentation_pattern = next(
            pattern
            for pattern in presentation["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
        )
        self.assertEqual("LAND", gameplay_pattern["serverMotion"]["travelStageId"])
        gameplay_airborne = next(
            stage
            for stage in gameplay_pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
        )
        self.assertEqual(6500, gameplay_airborne["durationMs"])
        self.assertEqual(
            {
                "eventId": "event.valtan.high-jump.airborne.spawn-target-axe",
                "trigger": "ENTER",
                "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
                "combatObjectArchetypeId": "combatobject.valtan.high-jump.target-axe",
                "volleyPolicy": "PER_ALIVE_PLAYER",
                "countPerResolvedTarget": 1,
                "layout": {"kind": "TARGET_CENTER"},
                "spawnSchedule": {
                    "kind": "INTERVAL",
                    "count": 3,
                    "firstOffsetMs": 0,
                    "intervalMs": 1333,
                },
                "arenaRandom": {
                    "kind": "RANDOM_NAVIGABLE_CIRCLE",
                    "anchor": "BOSS_SPAWN_POSITION",
                    "count": 4,
                    "radiusM": 14.0,
                    "heightToleranceM": 1.0,
                },
                "allowOverlap": False,
                "maximumTotalObjects": 36,
            },
            gameplay_airborne["events"][0],
        )
        self.assertEqual(
            [
                ("TAKEOFF", "EXACT", "mesh_att_battle_8_01_start", 1933, False),
                ("AIRBORNE", "LOOP_TO_STAGE_END", "mesh_att_battle_8_01_loop", 0, True),
                ("LAND", "EXACT", "mesh_att_battle_8_01_end", 3200, False),
                ("RECOVERY", "EXACT", "mesh_idle_battle_1", 400, False),
            ],
            [
                (
                    stage["stageId"],
                    stage["animation"]["endPolicy"],
                    stage["animation"]["occurrences"][0]["clip"],
                    stage["animation"]["occurrences"][0]["playMs"],
                    stage["animation"]["occurrences"][0]["repeatUntilStageEnd"],
                )
                for stage in presentation_pattern["stages"]
            ],
        )
        _, joined, outputs = pipeline.build_repository_product_projection(self.root)
        joined_pattern = next(
            pattern
            for pattern in joined["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
        )
        encounter = json.loads(outputs[pipeline.ENCOUNTER_REL])
        product_pattern = next(
            pattern
            for pattern in encounter["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
        )
        self.assertEqual("LAND", joined_pattern["serverMotion"]["travelStageId"])
        self.assertEqual("LAND", product_pattern["serverMotion"]["travelStageId"])
        product_airborne = next(
            stage
            for stage in product_pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
        )
        self.assertEqual(6500, product_airborne["durationMs"])
        self.assertEqual(
            {
                "spawnCount": 3,
                "spawnIntervalMs": 1333,
                "arenaRandomCount": 4,
                "arenaRandomRadiusM": 14.0,
                "arenaHeightToleranceM": 1.0,
                "arenaAnchorPolicy": "BOSS_SPAWN_POSITION",
            },
            {
                field: product_airborne["actions"][0][field]
                for field in (
                    "spawnCount",
                    "spawnIntervalMs",
                    "arenaRandomCount",
                    "arenaRandomRadiusM",
                    "arenaHeightToleranceM",
                    "arenaAnchorPolicy",
                )
            },
        )
        bindings = json.loads(outputs[pipeline.BINDINGS_REL])
        binding_by_action = {row["actionId"]: row for row in bindings["bindings"]}
        for stage in presentation_pattern["stages"]:
            occurrence = stage["animation"]["occurrences"][0]
            clip = binding_by_action[stage["actionId"]]["clips"][0]
            self.assertEqual(occurrence["clip"], clip["clip"])
            self.assertEqual(occurrence["playMs"], clip["playMs"])
            self.assertEqual(occurrence["repeatUntilStageEnd"], clip["loop"])

    def test_managed_cue_scale_policy_is_typed_and_projects_losslessly(self) -> None:
        presentation = self.docs[pipeline.PRESENTATION_AUTHORING_REL]
        managed_cues = [
            cue
            for pattern in presentation["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
        ]
        managed_cues_by_id = {cue["cueId"]: cue for cue in managed_cues}
        self.assertEqual(
            len(managed_cues), len(managed_cues_by_id)
        )
        self.assertTrue(
            set(managed_cues_by_id).issubset(pipeline.MANAGED_CUE_SCALE_POLICIES)
        )
        self.assertEqual(
            dict(
                Counter(
                    pipeline.MANAGED_CUE_SCALE_POLICIES[cue_id]
                    for cue_id in managed_cues_by_id
                )
            ),
            dict(Counter(cue["scalePolicy"]["kind"] for cue in managed_cues)),
        )
        self.assertEqual(
            {
                cue_id: pipeline.MANAGED_CUE_SCALE_POLICIES[cue_id]
                for cue_id in managed_cues_by_id
            },
            {cue["cueId"]: cue["scalePolicy"]["kind"] for cue in managed_cues},
        )
        for cue in managed_cues:
            policy = cue["scalePolicy"]
            if policy["kind"] == pipeline.OWNER_RELATIVE:
                self.assertEqual({"kind": pipeline.OWNER_RELATIVE}, policy)
            else:
                self.assertEqual([1.5, 1.5, 1.5], policy["worldScale"])

        _, joined, outputs = pipeline.build_repository_product_projection(self.root)
        projected = json.loads(outputs[pipeline.CUES_REL])
        self.assertEqual(4, projected["formatVersion"])
        projected_by_id = {cue["bindingId"]: cue for cue in projected["cues"]}
        for cue in managed_cues:
            self.assertEqual(
                cue["scalePolicy"],
                projected_by_id[cue["cueId"]]["scalePolicy"],
            )
        self.assertEqual(
            len(managed_cues) + 2,
            sum(1 for cue in projected["cues"] if "scalePolicy" in cue),
            "managed cues plus the two sealed Entrance Whirlwind cues",
        )
        self.assertEqual(
            len(managed_cues),
            sum(
                len(stage["effectCues"])
                for pattern in joined["patterns"]
                for stage in pattern["stages"]
            ),
        )

    def test_managed_cue_scale_policy_drift_fails_closed(self) -> None:
        gameplay = copy.deepcopy(self.docs[pipeline.GAMEPLAY_AUTHORING_REL])
        source = self.docs[pipeline.PRESENTATION_AUTHORING_REL]

        def mutate(cue_id, edit):
            presentation = copy.deepcopy(source)
            cue = next(
                cue
                for pattern in presentation["patterns"]
                for stage in pattern["stages"]
                for cue in stage["effectCues"]
                if cue["cueId"] == cue_id
            )
            edit(cue)
            return presentation

        cases = [
            mutate(
                "cue.valtan.project-tuned.attack.dash-charge.active-shield",
                lambda cue: cue["scalePolicy"].update(
                    {"worldScale": [1.5, 1.5, 1.5]}
                ),
            ),
            mutate(
                "cue.valtan.project-tuned.attack.dash-charge.windup-telegraph",
                lambda cue: cue["scalePolicy"].pop("worldScale"),
            ),
            mutate(
                "cue.valtan.whirlwind.active",
                lambda cue: cue["scalePolicy"].update({"kind": "UNKNOWN"}),
            ),
            mutate(
                "cue.valtan.carrier-v1.attack.high-jump.land.clip-01",
                lambda cue: cue["scalePolicy"].update(
                    {"worldScale": [1.5, 0.0, 1.5]}
                ),
            ),
            mutate(
                "cue.valtan.project-tuned.attack.dash-charge.active-shield",
                lambda cue: cue["scalePolicy"].update(
                    {
                        "kind": "GAMEPLAY_FOOTPRINT",
                        "worldScale": [1.5, 1.5, 1.5],
                    }
                ),
            ),
        ]
        for presentation in cases:
            with self.subTest(), self.assertRaises(pipeline.PipelineError):
                pipeline.join_v2_authoring(
                    copy.deepcopy(gameplay),
                    presentation,
                    self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL],
                )

    def test_dash_charge_uses_one_visible_server_and_presentation_clock(self) -> None:
        gameplay = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        presentation = self.docs[pipeline.PRESENTATION_AUTHORING_REL]
        dash_gameplay = next(
            pattern
            for pattern in gameplay["patterns"]
            if pattern["patternId"] == "VALTAN_DASH_CHARGE"
        )
        charge_gameplay = next(
            stage
            for stage in dash_gameplay["stages"]
            if stage["stageId"] == "CHARGE"
        )
        recovery_gameplay = next(
            stage
            for stage in dash_gameplay["stages"]
            if stage["stageId"] == "RECOVERY"
        )
        groggy_gameplay = next(
            stage
            for stage in dash_gameplay["stages"]
            if stage["stageId"] == "GROGGY"
        )
        dash_presentation = next(
            pattern
            for pattern in presentation["patterns"]
            if pattern["patternId"] == "VALTAN_DASH_CHARGE"
        )
        charge_presentation = next(
            stage
            for stage in dash_presentation["stages"]
            if stage["stageId"] == "CHARGE"
        )
        groggy_presentation = next(
            stage
            for stage in dash_presentation["stages"]
            if stage["stageId"] == "GROGGY"
        )
        recovery_presentation = next(
            stage
            for stage in dash_presentation["stages"]
            if stage["stageId"] == "RECOVERY"
        )
        occurrences = charge_presentation["animation"]["occurrences"]
        presentation_wall_ms = sum(
            occurrence["playMs"] / occurrence["playRate"]
            for occurrence in occurrences
        )

        self.assertEqual(1500, charge_gameplay["durationMs"])
        self.assertEqual(
            {"kind": "FORWARD", "distance": 20.0},
            charge_gameplay["motion"],
        )
        self.assertEqual(1, len(occurrences))
        self.assertEqual(2450, occurrences[0]["sourceStartMs"])
        self.assertEqual(900, occurrences[0]["playMs"])
        self.assertEqual(0.6, occurrences[0]["playRate"])
        self.assertAlmostEqual(
            charge_gameplay["durationMs"], presentation_wall_ms, delta=2.0
        )
        self.assertEqual(
            {
                "WALL_CONTACT": "valtan.attack.dash-charge.groggy",
                "TIMEOUT": "valtan.attack.dash-charge.recovery",
            },
            {
                branch["outcome"]: branch["nextActionId"]
                for branch in charge_gameplay["branches"]
            },
        )
        wall_contact_target = next(
            branch["nextActionId"]
            for branch in charge_gameplay["branches"]
            if branch["outcome"] == "WALL_CONTACT"
        )
        self.assertEqual(groggy_presentation["actionId"], wall_contact_target)
        self.assertNotEqual(recovery_presentation["actionId"], wall_contact_target)
        self.assertEqual(
            "LOOP_TO_STAGE_END",
            groggy_presentation["animation"]["endPolicy"],
        )
        self.assertEqual(
            [
                {
                    "clipOccurrenceId": "valtan.attack.dash-charge.groggy.clip.01",
                    "clip": "mesh_dmg_parts_loop_1",
                    "mappingBasis": "PATTERN_PR_REFERENCE",
                    "sourceStartMs": 0,
                    "playMs": 0,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": True,
                }
            ],
            groggy_presentation["animation"]["occurrences"],
        )
        self.assertEqual(
            "DESTROY_FIRST_ELIGIBLE",
            groggy_gameplay["partDamagePolicy"],
        )
        self.assertEqual(
            {
                "PART_DESTROYED": "valtan.attack.dash-charge.part-break",
                "TIMEOUT": "valtan.attack.dash-charge.recovery",
            },
            {
                branch["outcome"]: branch["nextActionId"]
                for branch in groggy_gameplay["branches"]
            },
        )
        self.assertEqual(
            {
                "TIMEOUT": None,
            },
            {
                branch["outcome"]: branch["nextActionId"]
                for branch in recovery_gameplay["branches"]
            },
        )
        self.assertTrue(
            pipeline._has_closed_stage_flag(
                groggy_gameplay, "boss.flag.groggy"
            )
        )
        _, _, outputs = pipeline.build_repository_product_projection(self.root)
        encounter = json.loads(outputs[pipeline.ENCOUNTER_REL])
        bindings = json.loads(outputs[pipeline.BINDINGS_REL])
        projected_dash = next(
            pattern
            for pattern in encounter["patterns"]
            if pattern["patternId"] == "VALTAN_DASH_CHARGE"
        )
        projected_groggy = next(
            stage
            for stage in projected_dash["stages"]
            if stage["stageId"] == "GROGGY"
        )
        self.assertEqual(
            "DESTROY_FIRST_ELIGIBLE",
            projected_groggy["partDamagePolicy"],
        )
        projected_groggy_binding = next(
            binding
            for binding in bindings["bindings"]
            if binding["actionId"] == "valtan.attack.dash-charge.groggy"
        )
        self.assertEqual(
            [
                {
                    "clipOccurrenceId": "valtan.attack.dash-charge.groggy.clip.01",
                    "clip": "mesh_dmg_parts_loop_1",
                    "mappingBasis": "PATTERN_PR_REFERENCE",
                    "sourceStartMs": 0,
                    "playMs": 0,
                    "playRate": 1.0,
                    "loop": True,
                }
            ],
            projected_groggy_binding["clips"],
        )

    def test_charge_lock_and_counter_terminal_graph_are_explicit(self) -> None:
        gameplay = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        patterns = {row["patternId"]: row for row in gameplay["patterns"]}
        charge = patterns["VALTAN_CHARGE"]
        self.assertEqual(
            ("LOCK_NEAREST_ON_START", "TRACK_TARGET_EACH_TICK"),
            (charge["targetPolicy"], charge["aimPolicy"]),
        )
        counter_slam = next(
            stage
            for stage in patterns["VALTAN_COUNTER"]["stages"]
            if stage["stageId"] == "STEP_03"
        )
        self.assertIsNone(counter_slam["defaultNextActionId"])
        self.assertEqual(
            [{"outcome": "TIMEOUT", "nextActionId": None}],
            counter_slam["branches"],
        )

    def test_part_damage_and_counter_proxy_extensions_fail_closed(self) -> None:
        gameplay = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]

        def stage(document: dict, pattern_id: str, stage_id: str) -> dict:
            owner = next(
                row for row in document["patterns"]
                if row["patternId"] == pattern_id
            )
            return next(
                row for row in owner["stages"]
                if row["stageId"] == stage_id
            )

        bad_policy = copy.deepcopy(gameplay)
        stage(
            bad_policy, "VALTAN_DASH_CHARGE", "GROGGY"
        )["partDamagePolicy"] = "DESTROY_ALL"
        with self.assertRaisesRegex(
            pipeline.PipelineError, "partDamagePolicy is unsupported"
        ):
            pipeline.validate_gameplay_authoring(bad_policy)

        open_part_window = copy.deepcopy(gameplay)
        groggy = stage(
            open_part_window, "VALTAN_DASH_CHARGE", "GROGGY"
        )
        groggy["events"] = [
            event for event in groggy["events"]
            if event["trigger"] != "EXIT"
        ]
        with self.assertRaisesRegex(
            pipeline.PipelineError, "closed groggy window"
        ):
            pipeline.validate_gameplay_authoring(open_part_window)

        invalid_proxy = copy.deepcopy(gameplay)
        stage(
            invalid_proxy, "VALTAN_TRASH", "STEP_07"
        )["counterProxy"]["space"] = "WORLD"
        with self.assertRaisesRegex(
            pipeline.PipelineError, "counterProxy space is unsupported"
        ):
            pipeline.validate_gameplay_authoring(invalid_proxy)

        unowned_proxy = copy.deepcopy(gameplay)
        trash_counter = stage(unowned_proxy, "VALTAN_TRASH", "STEP_07")
        trash_counter["branches"] = [
            branch for branch in trash_counter["branches"]
            if branch["outcome"] != "COUNTER_HIT"
        ]
        with self.assertRaisesRegex(
            pipeline.PipelineError, "COUNTER_HIT branch"
        ):
            pipeline.validate_gameplay_authoring(unowned_proxy)

        incoherent_target = copy.deepcopy(gameplay)
        catch = next(
            row for row in incoherent_target["patterns"]
            if row["patternId"] == "VALTAN_CATCH_BREATH"
        )
        catch["aimPolicy"] = "NONE"
        with self.assertRaisesRegex(
            pipeline.PipelineError, "target/aim policy is unsupported"
        ):
            pipeline.validate_gameplay_authoring(incoherent_target)

    def test_source_hash_precondition_and_revision_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            candidate_root = Path(temporary) / "candidates"
            pointer = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
            )
            self.assertFalse(pointer["activeRuntimeChanged"])
            revision = pointer["revisionId"]
            manifest = pipeline.read_json(
                candidate_root / "revisions" / revision / "revision-manifest.json"
            )
            identity_payload = copy.deepcopy(manifest)
            identity_payload["revisionId"] = ""
            self.assertEqual(revision, pipeline.canonical_hash(identity_payload))
            self.assertEqual("PARENT_MANIFEST", manifest["revisionIdentity"]["kind"])
            identity_path = (
                candidate_root
                / "revisions"
                / revision
                / manifest["revisionIdentity"]["identityPayloadPath"]
            )
            self.assertEqual(revision, pipeline.sha256_file(identity_path))
            self.assertEqual(identity_payload, pipeline.read_json(identity_path))
            self.assertNotIn(
                "revision-identity.json", {row["path"] for row in manifest["artifacts"]}
            )
            pipeline.validate_candidate_revision_manifest(
                candidate_root / "revisions" / revision,
                manifest,
            )
            invalid_activation = copy.deepcopy(manifest)
            invalid_activation["runtimeActivation"] = "NOT_IMPLEMENTED"
            with self.assertRaisesRegex(
                pipeline.PipelineError, "activation contract"
            ):
                pipeline.validate_candidate_revision_manifest(
                    candidate_root / "revisions" / revision,
                    invalid_activation,
                )
            self.assertEqual("SERVER_2PC_TICK_BOUNDARY", manifest["runtimeActivation"])
            self.assertEqual("HOT_RELOAD", manifest["applyClass"])
            self.assertEqual(["VALTAN_BOSS"], manifest["allowedDomains"])
            expected_lanes = {
                "ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA", "WORLD_EVENT_SET"
            }
            self.assertEqual(expected_lanes, set(manifest["requiredPresentationLanes"]))
            compatibility = manifest["clientPresentationCompatibility"]
            self.assertEqual("BYTE_IDENTICAL_TO_ACTIVE", compatibility["mode"])
            self.assertEqual(expected_lanes, set(compatibility["requiredLanes"]))
            self.assertEqual(expected_lanes, {row["lane"] for row in compatibility["artifacts"]})
            revision_root = candidate_root / "revisions" / revision
            for artifact in compatibility["artifacts"]:
                staged = revision_root / artifact["path"]
                source = self.root / artifact["path"]
                self.assertEqual(source.read_bytes(), staged.read_bytes())
                self.assertEqual(artifact["sha256"], artifact["repositorySourceSha256"])
                self.assertEqual(artifact["bytes"], staged.stat().st_size)
            bootstrap = revision_root / pipeline.GAMEPLAY_BOOTSTRAP_REL
            self.assertTrue(bootstrap.is_file())
            bootstrap_contract = manifest["serverGameplayBootstrap"]
            self.assertEqual(pipeline.GAMEPLAY_BOOTSTRAP_REL, bootstrap_contract["path"])
            self.assertEqual(
                pipeline.sha256_file(bootstrap), bootstrap_contract["candidateSha256"]
            )
            self.assertEqual(
                bootstrap_contract["candidateSha256"],
                manifest["revisionIdentity"]["serverBootstrapContentRevision"],
            )
            server_submanifest = pipeline.read_json(revision_root / "_manifest/server.json")
            self.assertIn(
                pipeline.GAMEPLAY_BOOTSTRAP_REL,
                {row["path"] for row in server_submanifest["artifacts"]},
            )
            second = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
            )
            self.assertEqual(pointer, second)
            self.assertEqual(self.source_manifest, pipeline.source_manifest(self.root))
            wrong = copy.deepcopy(self.source_manifest)
            wrong["sourceManifestId"] = "0" * 64
            with self.assertRaises(pipeline.PipelineError):
                pipeline.publish_candidate(
                    self.root,
                    candidate_root,
                    expected_source_manifest=wrong,
                )

    def test_gameplay_bootstrap_diff_is_valtan_only_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            baseline = root / "baseline.bootstrap"
            candidate = root / "candidate.bootstrap"

            def write_bootstrap(path: Path, rows: list[str]) -> None:
                path.write_text(
                    f"LOSTARK_GAMEPLAY_BOOTSTRAP\t{pipeline.GAMEPLAY_BOOTSTRAP_VERSION}\t"
                    + str(len(rows))
                    + "\n"
                    + "\n".join(rows)
                    + "\n",
                    encoding="utf-8",
                )

            write_bootstrap(
                baseline,
                ["PLAYER\tLANCE_MASTER\t100", "DAMAGE\tdamage.valtan.swing\t220"],
            )
            write_bootstrap(
                candidate,
                ["PLAYER\tLANCE_MASTER\t100", "DAMAGE\tdamage.valtan.swing\t221"],
            )
            result = pipeline.validate_valtan_only_bootstrap_diff(baseline, candidate)
            self.assertEqual((1, 1), (result["removedValtanRows"], result["addedValtanRows"]))

            write_bootstrap(
                candidate,
                ["PLAYER\tLANCE_MASTER\t101", "DAMAGE\tdamage.valtan.swing\t221"],
            )
            with self.assertRaisesRegex(
                pipeline.PipelineError, "outside VALTAN_BOSS"
            ):
                pipeline.validate_valtan_only_bootstrap_diff(baseline, candidate)

    def test_candidate_apply_class_uses_final_saved_boss_state(self) -> None:
        runtime_bosses = self.docs[pipeline.BOSS_PROFILES_REL]
        unchanged_candidate = copy.deepcopy(runtime_bosses)
        self.assertEqual(
            "HOT_RELOAD",
            pipeline.classify_candidate_apply_class(
                runtime_bosses, unchanged_candidate
            ),
        )

        changed_candidate = copy.deepcopy(runtime_bosses)
        runtime_valtan = next(
            row
            for row in runtime_bosses["bosses"]
            if row["archetypeId"] == "BOSS_VALTAN"
        )
        candidate_valtan = next(
            row
            for row in changed_candidate["bosses"]
            if row["archetypeId"] == "BOSS_VALTAN"
        )
        candidate_valtan["maximumHp"] = runtime_valtan["maximumHp"] + 1
        self.assertEqual(
            "ENCOUNTER_RESET",
            pipeline.classify_candidate_apply_class(
                runtime_bosses, changed_candidate
            ),
        )

        patch = {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.source_manifest["sourceManifestId"],
            "operations": [
                {
                    "op": "SET_BOSS_BASE_FIELD",
                    "bossArchetypeId": "BOSS_VALTAN",
                    "field": "maximumHp",
                    "value": runtime_valtan["maximumHp"] + 1,
                }
            ],
        }
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            saved = pipeline.save_authoring(self.root, authoring_root, patch)
            candidate_root = temporary_root / "candidates"
            published = pipeline.publish_candidate(
                self.root,
                candidate_root,
                authoring_root=authoring_root,
                authoring_revision=saved["revisionId"],
            )
            revision_root = (
                candidate_root / "revisions" / published["revisionId"]
            )
            manifest = pipeline.read_json(
                revision_root / "revision-manifest.json"
            )
            self.assertEqual(0, manifest["draftPatchOperationCount"])
            self.assertEqual("ENCOUNTER_RESET", manifest["applyClass"])

            invalid = copy.deepcopy(manifest)
            invalid["applyClass"] = "UNRECOGNIZED_APPLY_CLASS"
            with self.assertRaisesRegex(
                pipeline.PipelineError, "activation contract"
            ):
                pipeline.validate_candidate_revision_manifest(
                    revision_root, invalid
                )

    def test_stable_id_draft_patch_projects_balance_and_pattern_candidate(self) -> None:
        draft = self.draft_patch()
        migrated = self.migrate()
        candidate, bosses, damage, count = pipeline.apply_draft_patch(
            migrated,
            self.docs[pipeline.BOSS_PROFILES_REL],
            self.docs[pipeline.DAMAGE_REL],
            draft,
            self.source_manifest["sourceManifestId"],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(9, count)
        whirlwind = next(row for row in candidate["patterns"] if row["patternId"] == "VALTAN_WHIRLWIND")
        self.assertEqual(3, whirlwind["eligibility"]["repeatPolicy"]["limit"])
        self.assertEqual((1.0, 13.0), (whirlwind["eligibility"]["minimumRangeM"], whirlwind["eligibility"]["maximumRangeM"]))
        spin = next(row for row in whirlwind["stages"] if row["stageId"] == "SPIN")
        self.assertEqual([0, 300, 600, 900], spin["hit"]["schedule"]["offsetsMs"])
        self.assertEqual(61000, bosses["bosses"][0]["maximumHp"])
        damage_row = next(
            row for row in damage["profiles"]
            if row["damageProfileId"] == "damage.valtan.circular-spin"
        )
        self.assertEqual(325, damage_row["damageRatePercent"])

        with tempfile.TemporaryDirectory() as temporary:
            candidate_root = Path(temporary) / "candidates"
            pointer = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
                draft_patch=draft,
            )
            revision_root = candidate_root / "revisions" / pointer["revisionId"]
            encounter = pipeline.read_json(revision_root / pipeline.ENCOUNTER_REL)
            projected = next(row for row in encounter["patterns"] if row["patternId"] == "VALTAN_WHIRLWIND")
            self.assertEqual(
                20, projected["selectionWeight"],
                "per-set edits must not rewrite the explicit global fallback",
            )
            self.assertEqual(1400, projected["stages"][0]["durationMs"])
            projected_bosses = pipeline.read_json(revision_root / pipeline.BOSS_PROFILES_REL)
            self.assertEqual(61000, projected_bosses["bosses"][0]["maximumHp"])
            projected_damage = pipeline.read_json(revision_root / pipeline.DAMAGE_REL)
            self.assertEqual(
                325,
                next(
                    row["damageRatePercent"]
                    for row in projected_damage["profiles"]
                    if row["damageProfileId"] == "damage.valtan.circular-spin"
                ),
            )
            manifest = pipeline.read_json(revision_root / "revision-manifest.json")
            self.assertEqual(9, manifest["draftPatchOperationCount"])
            self.assertEqual("ENCOUNTER_RESET", manifest["applyClass"])
            self.assertEqual(self.source_manifest["sourceManifestId"], manifest["sourceManifestId"])
            receipt = pipeline.read_json(revision_root / pipeline.PROVENANCE_REL)
            provenance = {
                (row["targetId"], row["targetField"]): row
                for row in receipt["entries"]
            }
            self.assertEqual(
                61000,
                provenance[("boss:BOSS_VALTAN", "maximumHp")]["resultValue"],
            )
            self.assertEqual(
                325,
                provenance[
                    ("damage:damage.valtan.circular-spin", "damageRatePercent")
                ]["resultValue"],
            )
            weight_entry = provenance[
                ("pattern:VALTAN_WHIRLWIND", "patterns[16].selectionWeight")
            ]
            self.assertEqual(20, weight_entry["resultValue"])

    def test_per_set_weight_mechanic_and_scripted_sequence_round_trip_to_v4_bootstrap(
        self,
    ) -> None:
        patch = {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.source_manifest["sourceManifestId"],
            "operations": [
                {
                    "op": "SET_PATTERN_WEIGHT",
                    "selectionSetId": "selectionset.valtan.160.130",
                    "patternId": "VALTAN_DASH_CHARGE",
                    "value": 31,
                },
                {
                    "op": "SET_PATTERN_ENABLED",
                    "selectionSetId": "selectionset.valtan.130.109",
                    "patternId": "VALTAN_DASH_CHARGE",
                    "value": False,
                },
                {
                    "op": "SET_MECHANIC_TRIGGER",
                    "mechanicId": "mechanic.valtan-arena-break-109",
                    "patternId": "VALTAN_ARENA_BREAK_109",
                    "healthBar": 109,
                    "triggerOrder": 2,
                },
            ],
        }
        master = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        staged, _, _, count = pipeline.apply_draft_patch(
            master,
            self.docs[pipeline.BOSS_PROFILES_REL],
            self.docs[pipeline.DAMAGE_REL],
            patch,
            self.source_manifest["sourceManifestId"],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(3, count)
        first, second = staged["decisionModel"]["selectionSets"]
        self.assertEqual(31, first["candidates"][1]["weight"])
        self.assertFalse(first["candidates"][1]["enabled"])
        self.assertEqual(30, second["candidates"][1]["weight"])
        self.assertFalse(second["candidates"][1]["enabled"])
        mechanic = staged["decisionModel"]["mechanics"][1]
        self.assertEqual((109, 2), (
            mechanic["trigger"]["healthBar"], mechanic["triggerOrder"]
        ))
        self.assertEqual(
            EXPECTED_SCRIPTED_SEQUENCE,
            staged["decisionModel"]["scriptedSequence"],
        )
        phase_two_ids = [
            row["patternId"]
            for row in staged["decisionModel"]["manualAuditions"]
            if row["authoringPhase"] == 2
        ]
        self.assertEqual(19, len(phase_two_ids))
        self.assertEqual(
            [
                row["patternId"]
                for row in self.docs[pipeline.SAVED_FLOW_REL]["flows"][0]["slots"]
            ],
            staged["decisionModel"]["scriptedSequence"]["patternIds"],
            "Product order must preserve the saved Flow including repetitions and explicit Phase-3 rows",
        )

        projected = pipeline.project_v2_products(self.root, self.docs, staged)
        rotations = json.loads(projected[pipeline.ROTATIONS_REL])
        self.assertEqual(4, rotations["formatVersion"])
        self.assertEqual(
            EXPECTED_SCRIPTED_SEQUENCE,
            rotations["scriptedSequence"],
        )
        self.assertEqual(31, rotations["rotations"][0]["candidates"][1]["weight"])
        self.assertFalse(rotations["rotations"][0]["candidates"][1]["enabled"])
        self.assertEqual(30, rotations["rotations"][1]["candidates"][1]["weight"])
        self.assertFalse(rotations["rotations"][1]["candidates"][1]["enabled"])
        self.assertEqual(
            self.docs[pipeline.ROTATIONS_REL]["rotations"][2:],
            rotations["rotations"][2:],
            "all six post-109 legacy rows must preserve order and duplicates",
        )
        encounter = json.loads(projected[pipeline.ENCOUNTER_REL])
        dash = next(row for row in encounter["patterns"]
                    if row["patternId"] == "VALTAN_DASH_CHARGE")
        arena = next(row for row in encounter["patterns"]
                     if row["patternId"] == "VALTAN_ARENA_BREAK_109")
        self.assertEqual(30, dash["selectionWeight"],
                         "phase-1 set edits must not mutate global fallback")
        self.assertEqual((109, 2), (arena["triggerHealthBar"], arena["triggerOrder"]))

        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            saved = pipeline.save_authoring(self.root, authoring_root, patch)
            resumed, _, _ = pipeline.load_authoring_revision(
                self.root,
                authoring_root,
                saved["revisionId"],
                self.source_manifest,
                self.docs,
            )
            self.assertEqual(
                staged["decisionModel"], resumed["decisionModel"],
                "a new Tool must resume the exact saved typed decision model",
            )
            candidate_root = temporary_root / "candidates"
            published = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
                authoring_root=authoring_root,
                authoring_revision=saved["revisionId"],
            )
            revision_root = candidate_root / "revisions" / published["revisionId"]
            candidate_rotations = pipeline.read_json(
                revision_root / pipeline.ROTATIONS_REL
            )
            self.assertEqual(rotations, candidate_rotations)
            manifest = pipeline.read_json(revision_root / "revision-manifest.json")
            self.assertEqual("HOT_RELOAD", manifest["applyClass"])
            candidate_flow_path = revision_root / pipeline.SAVED_FLOW_REL
            candidate_flow_bytes = candidate_flow_path.read_bytes()
            self.assertEqual(
                (self.root / pipeline.SAVED_FLOW_REL).read_bytes(),
                candidate_flow_bytes,
            )
            self.assertEqual(
                pipeline.read_json(self.root / pipeline.GAMEPLAY_AUTHORING_REL)["decisionModel"]["scriptedSequence"],
                pipeline.read_json(revision_root / "Authoring/Valtan.gameplay.json")["decisionModel"]["scriptedSequence"],
            )
            self.assertIn(
                pipeline.SAVED_FLOW_REL,
                {
                    row["path"]
                    for row in pipeline.read_json(revision_root / "_manifest/authoring.json")["artifacts"]
                },
            )
            self.assertEqual(
                pipeline.GAMEPLAY_BOOTSTRAP_VERSION,
                manifest["serverGameplayBootstrap"]["formatVersion"],
            )
            bootstrap_lines = (
                revision_root / pipeline.GAMEPLAY_BOOTSTRAP_REL
            ).read_text(encoding="utf-8").splitlines()
            self.assertTrue(bootstrap_lines[0].startswith(
                f"LOSTARK_GAMEPLAY_BOOTSTRAP\t{pipeline.GAMEPLAY_BOOTSTRAP_VERSION}\t"
            ))
            managed_rows = [
                line for line in bootstrap_lines
                if line.startswith("PATTERNAUTHORINGMANAGED\t")
            ]
            self.assertEqual(
                {
                    f"PATTERNAUTHORINGMANAGED\tENCOUNTER_VALTAN\t{pattern['patternId']}"
                    for pattern in self.docs[pipeline.GAMEPLAY_AUTHORING_REL]["patterns"]
                },
                set(managed_rows),
            )
            self.assertEqual(len(managed_rows), len(set(managed_rows)))
            self.assertIn(
                "PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\t"
                "rotation.valtan.160.130\t1\tVALTAN_DASH_CHARGE\t31\t0",
                bootstrap_lines,
            )
            self.assertIn(
                "PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\t"
                "rotation.valtan.130.109\t1\tVALTAN_DASH_CHARGE\t30\t0",
                bootstrap_lines,
            )
            self.assertIn(
                "PATTERNROTATIONWINDOW\tENCOUNTER_VALTAN\t"
                "rotation.valtan.160.130\twindow.valtan.phase1.160.130\t1\t"
                "selectionset.valtan.160.130\t160\t130\t5",
                bootstrap_lines,
            )
            self.assertFalse(any(
                line.startswith("PATTERNROTATIONSTEP\tENCOUNTER_VALTAN\t"
                                "rotation.valtan.160.130\t")
                for line in bootstrap_lines
            ))
            self.assertTrue(any(
                line.startswith("PATTERNROTATIONSTEP\tENCOUNTER_VALTAN\t"
                                "rotation.valtan.109.100\t")
                for line in bootstrap_lines
            ))

            sequence_id = EXPECTED_SCRIPTED_SEQUENCE["sequenceId"]
            self.assertIn(
                "PATTERNSEQUENCE\tENCOUNTER_VALTAN\t"
                f"{sequence_id}\tORDERED_ONCE_THEN_IDLE\t1000\t"
                f"{len(EXPECTED_SCRIPTED_SEQUENCE['patternIds'])}",
                bootstrap_lines,
            )
            self.assertEqual(
                [
                    "PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\t"
                    f"{sequence_id}\t{ordinal}\t{pattern_id}"
                    for ordinal, pattern_id in enumerate(
                        EXPECTED_SCRIPTED_SEQUENCE["patternIds"]
                    )
                ],
                [
                    line
                    for line in bootstrap_lines
                    if line.startswith(
                        "PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\t"
                        f"{sequence_id}\t"
                    )
                ],
            )
            candidate_pointer_before = (candidate_root / "current-candidate.json").read_bytes()
            changed_flow = pipeline.read_saved_flow_document(revision_root)
            changed_flow["flows"][0]["slots"][0:2] = reversed(changed_flow["flows"][0]["slots"][0:2])
            candidate_flow_path.write_bytes(pipeline.json_text(changed_flow).encode("utf-8"))
            changed_manifest = copy.deepcopy(manifest)
            flow_artifact = next(
                row for row in changed_manifest["artifacts"]
                if row["path"] == pipeline.SAVED_FLOW_REL
            )
            flow_artifact["sha256"] = pipeline.sha256_file(candidate_flow_path)
            flow_artifact["bytes"] = candidate_flow_path.stat().st_size
            changed_manifest["artifactSetId"] = pipeline._manifest_hash(changed_manifest["artifacts"])
            with self.assertRaisesRegex(pipeline.PipelineError, "saved Flow order does not match"):
                pipeline.validate_candidate_revision_manifest(revision_root, changed_manifest)
            candidate_flow_path.write_bytes(candidate_flow_bytes)
            pipeline.validate_candidate_revision_manifest(revision_root, manifest)
            self.assertEqual(
                candidate_pointer_before,
                (candidate_root / "current-candidate.json").read_bytes(),
            )

    def test_saved_flow_reference_resolves_exact_repeated_slot_order(self) -> None:
        physical = pipeline.read_json(self.root / pipeline.GAMEPLAY_AUTHORING_REL)
        before = copy.deepcopy(physical)
        flow_document = copy.deepcopy(self.docs[pipeline.SAVED_FLOW_REL])
        flow = flow_document["flows"][0]
        original = copy.deepcopy(flow["slots"])
        flow["slots"] = [original[2], original[0], original[1], original[6]]
        flow["interStepPursuitMs"] = 4321
        joined = pipeline.join_v2_authoring(
            physical, self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL],
            flow_document,
        )
        expected = ["VALTAN_HIGH_JUMP", "VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH", "VALTAN_WHIRLWIND"]
        sequence = joined["decisionModel"]["scriptedSequence"]
        self.assertEqual(expected, sequence["patternIds"])
        self.assertEqual(4321, sequence["interStepPursuitMs"])
        projected = pipeline.project_v2_products(self.root, self.docs, joined)
        self.assertEqual(sequence, json.loads(projected[pipeline.ROTATIONS_REL])["scriptedSequence"])
        self.assertEqual(before, physical, "resolving must not rewrite the physical Flow reference")

    def make_saved_flow(self, pattern_ids: list[str]) -> dict:
        document = copy.deepcopy(self.docs[pipeline.SAVED_FLOW_REL])
        flow = document["flows"][0]
        flow["slots"] = [
            {"slotId": f"{flow['flowId']}.slot.{ordinal:06d}", "patternId": pattern_id}
            for ordinal, pattern_id in enumerate(pattern_ids, start=1)
        ]
        flow["nextSlotOrdinal"] = len(pattern_ids) + 1
        return document

    def test_saved_flow_admits_every_authored_pattern_through_product_projection(self) -> None:
        physical = pipeline.read_json(self.root / pipeline.GAMEPLAY_AUTHORING_REL)
        before = copy.deepcopy(physical)
        for pattern in physical["patterns"]:
            pattern_id = pattern["patternId"]
            with self.subTest(pattern=pattern_id):
                expected = [pattern_id, "VALTAN_WHIRLWIND"]
                if pattern_id != pipeline.OPTIONAL_ENTRY_PATTERN_ID:
                    expected.append(pattern_id)
                document = self.make_saved_flow(expected)
                joined = pipeline.join_v2_authoring(
                    physical, self.docs[pipeline.PRESENTATION_AUTHORING_REL],
                    self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL],
                    document,
                )
                products = pipeline.project_v2_products(self.root, self.docs, joined)
                self.assertEqual(expected, joined["decisionModel"]["scriptedSequence"]["patternIds"])
                self.assertEqual(expected, json.loads(products[pipeline.ROTATIONS_REL])["scriptedSequence"]["patternIds"])
        self.assertEqual(before, physical)

    def test_saved_flow_new_core_manual_and_derived_patterns_need_no_allowlist(self) -> None:
        staged = self.with_manual_audition(
            pattern_id="VALTAN_EXPANSION_CORE", source_chain_id="expansion-core")
        staged["decisionModel"]["manualAuditions"].pop()
        staged["patterns"][-1]["compatibilitySelectionWeight"] = 1
        for selection_set in staged["decisionModel"]["selectionSets"]:
            selection_set["candidates"].append({
                "patternId": "VALTAN_EXPANSION_CORE", "weight": 1, "enabled": True})
        staged = self.with_manual_audition(
            staged, pattern_id="VALTAN_EXPANSION_MANUAL", source_chain_id="expansion-manual")
        staged = self.with_manual_audition(
            staged, pattern_id="VALTAN_EXPANSION_DERIVED", source_chain_id="expansion-derived")
        staged["decisionModel"]["manualAuditions"][-1]["admissionState"] = pipeline.DERIVED_SERVER_PATTERN
        gameplay, presentation = pipeline.split_v2_authoring(
            staged, self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL])
        gameplay["decisionModel"]["scriptedSequence"] = copy.deepcopy(
            pipeline.read_json(self.root / pipeline.GAMEPLAY_AUTHORING_REL)["decisionModel"]["scriptedSequence"])
        expected = ["VALTAN_EXPANSION_DERIVED", "VALTAN_EXPANSION_CORE", "VALTAN_EXPANSION_MANUAL", "VALTAN_EXPANSION_DERIVED"]
        document = self.make_saved_flow(expected)
        joined = pipeline.join_v2_authoring(
            gameplay, presentation, self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL], document)
        products = pipeline.project_v2_products(self.root, self.docs, joined)
        self.assertEqual(expected, json.loads(products[pipeline.ROTATIONS_REL])["scriptedSequence"]["patternIds"])
        product_patterns = {row["patternId"]: row for row in json.loads(products[pipeline.ENCOUNTER_REL])["patterns"]}
        for pattern_id in expected:
            self.assertEqual(pattern_id, product_patterns[pattern_id]["patternId"])
            self.assertTrue(product_patterns[pattern_id]["stages"])

    def test_saved_flow_inventory_can_exceed_slot_capacity_and_remove_unused_definitions(self) -> None:
        # Registered definitions and selected Flow slots have independent sizes.
        for extra_count in (1, 7, 36):
            with self.subTest(extra_patterns=extra_count):
                staged = None
                for ordinal in range(extra_count):
                    staged = self.with_manual_audition(
                        staged, pattern_id=f"VALTAN_EXPANSION_{ordinal}",
                        source_chain_id=f"expansion-{ordinal}")
                    staged["decisionModel"]["manualAuditions"][-1]["admissionState"] = pipeline.DERIVED_SERVER_PATTERN
                gameplay, presentation = pipeline.split_v2_authoring(
                    staged, self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL])
                gameplay["decisionModel"]["scriptedSequence"] = copy.deepcopy(
                    pipeline.read_json(self.root / pipeline.GAMEPLAY_AUTHORING_REL)["decisionModel"]["scriptedSequence"])
                expected = [f"VALTAN_EXPANSION_{extra_count - 1}", "VALTAN_WHIRLWIND"]
                document = self.make_saved_flow(expected)
                joined = pipeline.join_v2_authoring(
                    gameplay, presentation, self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL], document)
                self.assertEqual(expected, joined["decisionModel"]["scriptedSequence"]["patternIds"])
                # Retire the added definitions without changing the remaining saved slots.
                gameplay["patterns"] = [r for r in gameplay["patterns"] if not r["patternId"].startswith("VALTAN_EXPANSION_")]
                presentation["patterns"] = [r for r in presentation["patterns"] if not r["patternId"].startswith("VALTAN_EXPANSION_")]
                gameplay["decisionModel"]["manualAuditions"] = [r for r in gameplay["decisionModel"]["manualAuditions"] if not r["patternId"].startswith("VALTAN_EXPANSION_")]
                remaining = self.make_saved_flow(["VALTAN_WHIRLWIND"])
                joined = pipeline.join_v2_authoring(
                    gameplay, presentation, self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL], remaining)
                self.assertEqual(["VALTAN_WHIRLWIND"], joined["decisionModel"]["scriptedSequence"]["patternIds"])
                with self.assertRaisesRegex(pipeline.PipelineError, "not in the Boss Tool inventory"):
                    pipeline.resolve_gameplay_flow_reference(gameplay, document)

    def test_saved_flow_reference_rejects_invalid_documents_and_mixed_shapes(self) -> None:
        physical = pipeline.read_json(self.root / pipeline.GAMEPLAY_AUTHORING_REL)
        baseline = self.docs[pipeline.SAVED_FLOW_REL]
        cases = []
        def invalid(label, change):
            document = copy.deepcopy(baseline)
            change(document)
            cases.append((label, document))
        invalid("schema", lambda doc: doc.update(schema="wrong"))
        invalid("float version", lambda doc: doc.update(formatVersion=1.0))
        invalid("boolean version", lambda doc: doc.update(formatVersion=True))
        invalid("two flows", lambda doc: doc["flows"].append(copy.deepcopy(doc["flows"][0])))
        invalid("unknown flow", lambda doc: doc["flows"][0].update(flowId="flow.other"))
        invalid("float pursuit", lambda doc: doc["flows"][0].update(interStepPursuitMs=1000.0))
        invalid("pursuit zero", lambda doc: doc["flows"][0].update(interStepPursuitMs=0))
        invalid("empty", lambda doc: doc["flows"][0].update(slots=[]))
        invalid("unknown pattern", lambda doc: doc["flows"][0]["slots"][0].update(patternId="VALTAN_UNKNOWN"))
        invalid("entry after first slot", lambda doc: doc["flows"][0]["slots"][1].update(patternId="VALTAN_ENTRANCE_CINEMATIC"))
        invalid("legacy-only pattern", lambda doc: doc["flows"][0]["slots"][0].update(patternId="VALTAN_SWING"))
        invalid("duplicate slot", lambda doc: doc["flows"][0]["slots"][1].update(slotId=doc["flows"][0]["slots"][0]["slotId"]))
        invalid("foreign slot", lambda doc: doc["flows"][0]["slots"][0].update(slotId="flow.other.slot.000001"))
        invalid("non-numeric slot", lambda doc: doc["flows"][0]["slots"][0].update(slotId=pipeline.DEFAULT_SAVED_FLOW_ID + ".slot.00000x"))
        invalid("counter reuse", lambda doc: doc["flows"][0].update(nextSlotOrdinal=30))
        invalid("float counter", lambda doc: doc["flows"][0].update(nextSlotOrdinal=31.0))
        for label, document in cases:
            with self.subTest(case=label), self.assertRaises(pipeline.PipelineError):
                pipeline.resolve_gameplay_flow_reference(physical, document)
        overflow = copy.deepcopy(baseline)
        overflow_flow = overflow["flows"][0]
        overflow_flow["slots"] = [
            {
                "slotId": (
                    f"{pipeline.DEFAULT_SAVED_FLOW_ID}.slot.{ordinal:06d}"
                ),
                "patternId": "VALTAN_WHIRLWIND",
            }
            for ordinal in range(1, pipeline.SAVED_FLOW_MAX_SLOTS + 2)
        ]
        overflow_flow["nextSlotOrdinal"] = pipeline.SAVED_FLOW_MAX_SLOTS + 2
        with self.assertRaisesRegex(pipeline.PipelineError, "1..255 slots"):
            pipeline.resolve_gameplay_flow_reference(physical, overflow)
        for field, value in (("patternIds", ["VALTAN_WHIRLWIND"]), ("path", "../outside.json"), ("flowId", "flow.other")):
            mixed = copy.deepcopy(physical)
            mixed["decisionModel"]["scriptedSequence"][field] = value
            with self.subTest(field=field), self.assertRaises(pipeline.PipelineError):
                pipeline.resolve_gameplay_flow_reference(mixed, baseline)
        with self.assertRaisesRegex(pipeline.PipelineError, "snapshot-local Flow"):
            pipeline.join_v2_authoring(
                physical, self.docs[pipeline.PRESENTATION_AUTHORING_REL],
                self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL],
            )
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / pipeline.SAVED_FLOW_REL
            path.parent.mkdir(parents=True)
            for raw in (b'{"schema":"a","schema":"b"}', b'{"formatVersion":NaN}', b'x' * (pipeline.SAVED_FLOW_MAX_BYTES + 1)):
                path.write_bytes(raw)
                with self.assertRaises(pipeline.PipelineError):
                    pipeline.read_saved_flow_document(Path(temporary))

    def test_saved_flow_projects_33_and_255_ordered_occurrences(self) -> None:
        physical = pipeline.read_json(self.root / pipeline.GAMEPLAY_AUTHORING_REL)
        for count in (33, pipeline.SAVED_FLOW_MAX_SLOTS):
            with self.subTest(slot_count=count):
                expected = ["VALTAN_WHIRLWIND"] * count
                joined = pipeline.join_v2_authoring(
                    physical,
                    self.docs[pipeline.PRESENTATION_AUTHORING_REL],
                    self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL],
                    self.make_saved_flow(expected),
                )
                self.assertEqual(
                    expected,
                    joined["decisionModel"]["scriptedSequence"]["patternIds"],
                )
                products = pipeline.project_v2_products(
                    self.root, self.docs, joined
                )
                self.assertEqual(
                    expected,
                    json.loads(products[pipeline.ROTATIONS_REL])[
                        "scriptedSequence"
                    ]["patternIds"],
                )

    def test_saved_flow_source_manifest_and_immutable_authoring_cas(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "repository"
            for entry in self.source_manifest["files"]:
                destination = root / entry["path"]
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(self.root / entry["path"], destination)
            sources = pipeline.source_manifest(root)
            authoring_root = Path(temporary) / "authoring"
            patch = {"schema": pipeline.DRAFT_PATCH_SCHEMA, "formatVersion": 1,
                     "sourceRevision": sources["sourceManifestId"], "operations": []}
            pointer = pipeline.save_authoring(root, authoring_root, patch)
            revision_root = authoring_root / "revisions" / pointer["revisionId"]
            self.assertEqual((root / pipeline.SAVED_FLOW_REL).read_bytes(),
                             (revision_root / pipeline.SAVED_FLOW_REL).read_bytes())
            self.assertEqual(
                pipeline.read_json(root / pipeline.GAMEPLAY_AUTHORING_REL)["decisionModel"]["scriptedSequence"],
                pipeline.read_json(revision_root / pipeline.GAMEPLAY_AUTHORING_REL)["decisionModel"]["scriptedSequence"],
            )
            docs = pipeline.load_pipeline_documents(root)
            loaded, _, _ = pipeline.load_authoring_revision(root, authoring_root, pointer["revisionId"], sources, docs)
            self.assertEqual(EXPECTED_SCRIPTED_SEQUENCE, loaded["decisionModel"]["scriptedSequence"])
            changed = pipeline.read_saved_flow_document(root)
            changed["flows"][0]["slots"][0:2] = reversed(changed["flows"][0]["slots"][0:2])
            (root / pipeline.SAVED_FLOW_REL).write_text(pipeline.json_text(changed), encoding="utf-8")
            updated = pipeline.source_manifest(root)
            self.assertNotEqual(sources["sourceManifestId"], updated["sourceManifestId"])
            self.assertEqual(pipeline.sha256_file(root / pipeline.SAVED_FLOW_REL),
                             next(row["sha256"] for row in updated["files"] if row["path"] == pipeline.SAVED_FLOW_REL))
            pointer_before = (authoring_root / "current-authoring.json").read_bytes()
            with self.assertRaisesRegex(pipeline.PipelineError, "repository source changed"):
                pipeline.load_authoring_revision(root, authoring_root, pointer["revisionId"], updated, docs)
            self.assertEqual(pointer_before, (authoring_root / "current-authoring.json").read_bytes())
            (revision_root / pipeline.SAVED_FLOW_REL).unlink()
            with self.assertRaises(pipeline.PipelineError):
                pipeline._validate_authoring_artifact(revision_root, pointer["revisionId"])

    def test_saved_flow_publish_cas_and_publisher_failure_do_not_start_a_candidate(self) -> None:
        current_revision = pipeline.sha256_file(self.root / pipeline.SAVED_FLOW_REL)
        with tempfile.TemporaryDirectory() as temporary, mock.patch.object(pipeline.subprocess, "run") as run, mock.patch.object(pipeline, "publish_candidate") as publish:
            with self.assertRaisesRegex(pipeline.PipelineError, "revision changed"):
                pipeline.publish_saved_flow(self.root, Path(temporary), "0" * 64)
            run.assert_not_called()
            publish.assert_not_called()
            run.return_value = subprocess.CompletedProcess([], 1, "", "injected Product failure")
            with self.assertRaisesRegex(pipeline.PipelineError, "injected Product failure"):
                pipeline.publish_saved_flow(self.root, Path(temporary), current_revision)
            # A timeout on this publisher seam would kill its rollback owner.
            self.assertNotIn("timeout", run.call_args.kwargs)
            publish.assert_not_called()
            run.return_value = subprocess.CompletedProcess([], 0, "Project complete", "")
            with mock.patch.object(pipeline, "_require_saved_flow_revision", side_effect=[None, pipeline.PipelineError("concurrent Flow save")]):
                with self.assertRaisesRegex(pipeline.PipelineError, "concurrent Flow save"):
                    pipeline.publish_saved_flow(self.root, Path(temporary), current_revision)
            publish.assert_not_called()

    def test_inline_legacy_authoring_remains_closed_without_a_flow_document(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "repository"
            for entry in self.source_manifest["files"]:
                if entry["path"] == pipeline.SAVED_FLOW_REL:
                    continue
                destination = root / entry["path"]
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(self.root / entry["path"], destination)
            (root / pipeline.GAMEPLAY_AUTHORING_REL).write_bytes(
                pipeline.json_text(self.docs[pipeline.GAMEPLAY_AUTHORING_REL]).encode("utf-8")
            )
            sources = pipeline.source_manifest(root)
            self.assertNotIn(pipeline.SAVED_FLOW_REL, {row["path"] for row in sources["files"]})
            authoring_root = Path(temporary) / "authoring"
            patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA, "formatVersion": 1,
                "sourceRevision": sources["sourceManifestId"], "operations": [],
            }
            pointer = pipeline.save_authoring(root, authoring_root, patch)
            revision_root = authoring_root / "revisions" / pointer["revisionId"]
            self.assertFalse((revision_root / pipeline.SAVED_FLOW_REL).exists())
            manifest = pipeline._validate_authoring_artifact(revision_root, pointer["revisionId"])
            self.assertEqual(
                set(pipeline.LEGACY_AUTHORING_ARTIFACTS),
                {row["path"] for row in manifest["artifacts"]},
            )
            loaded, _, _ = pipeline.load_authoring_revision(
                root, authoring_root, pointer["revisionId"], sources,
                pipeline.load_pipeline_documents(root),
            )
            self.assertEqual(EXPECTED_SCRIPTED_SEQUENCE, loaded["decisionModel"]["scriptedSequence"])

    def test_saved_flow_publish_command_returns_one_structured_identity(self) -> None:
        result = {"sourceRevision": "a" * 64, "candidateRevision": "b" * 64,
                  "flowRevision": "c" * 64, "applyClass": "HOT_RELOAD",
                  "splitJoinValidated": True, "pointer": {"revisionId": "b" * 64}}
        stdout = io.StringIO()
        with mock.patch.object(pipeline, "publish_saved_flow", return_value=result), contextlib.redirect_stdout(stdout):
            exit_code = pipeline.main([
                "--repository-root", str(self.root), "publish-saved-flow",
                "--candidate-root", "Intermediate/TestFlowCandidates",
                "--expected-flow-revision", "c" * 64,
            ])
        response = json.loads(stdout.getvalue())
        self.assertEqual(0, exit_code)
        self.assertEqual("PUBLISH_SAVED_FLOW", response["command"])
        self.assertEqual("b" * 64, response["candidateRevision"])
        self.assertEqual("c" * 64, response["payload"]["flowRevision"])
        self.assertTrue(response["payload"]["splitJoinValidated"])
        stdout = io.StringIO()
        stderr = io.StringIO()
        with mock.patch.object(
            pipeline, "publish_saved_flow",
            side_effect=pipeline.PipelineError("injected Flow candidate rejection"),
        ), contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
            exit_code = pipeline.main([
                "--repository-root", str(self.root), "publish-saved-flow",
                "--candidate-root", "Intermediate/TestFlowCandidates",
                "--expected-flow-revision", "c" * 64,
            ])
        response = json.loads(stderr.getvalue())
        self.assertEqual(1, exit_code)
        self.assertEqual("", stdout.getvalue())
        self.assertFalse(response["ok"])
        self.assertEqual("PUBLISH_SAVED_FLOW", response["command"])
        self.assertIsNone(response["candidateRevision"])
        self.assertEqual(
            "injected Flow candidate rejection", response["errors"][0]["message"]
        )

    def test_scripted_sequence_invalid_inputs_fail_closed(self) -> None:
        base = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )

        unknown = copy.deepcopy(base)
        unknown["decisionModel"]["scriptedSequence"]["patternIds"][-1] = (
            "VALTAN_UNKNOWN_SCRIPTED_PATTERN"
        )

        late_entry = copy.deepcopy(base)
        late_entry["decisionModel"]["scriptedSequence"]["patternIds"][-1] = (
            pipeline.OPTIONAL_ENTRY_PATTERN_ID
        )

        wrong_mode = copy.deepcopy(base)
        wrong_mode["decisionModel"]["scriptedSequence"]["mode"] = "WEIGHTED_POOL"

        invalid_pursuit = copy.deepcopy(base)
        invalid_pursuit["decisionModel"]["scriptedSequence"][
            "interStepPursuitMs"
        ] = 0

        retired_suppression_field = copy.deepcopy(base)
        retired_suppression_field["decisionModel"]["scriptedSequence"][
            "suppressHealthBarMechanics"
        ] = False

        missing_contract_field = copy.deepcopy(base)
        del missing_contract_field["decisionModel"]["scriptedSequence"]["mode"]

        invalid_cases = (
            ("unknown", unknown, "names no managed pattern"),
            ("late entry", late_entry, "optional entry cinematic"),
            ("wrong mode", wrong_mode, "mode must be ORDERED_ONCE_THEN_IDLE"),
            (
                "invalid inter-step pursuit",
                invalid_pursuit,
                "scriptedSequence.interStepPursuitMs",
            ),
            (
                "retired suppression field",
                retired_suppression_field,
                "decisionModel.scriptedSequence fields mismatch",
            ),
            (
                "missing sequence contract field",
                missing_contract_field,
                "decisionModel.scriptedSequence fields mismatch",
            ),
        )
        for name, staged, message in invalid_cases:
            with self.subTest(case=name), self.assertRaisesRegex(
                pipeline.PipelineError, message
            ):
                pipeline.project_v2_products(self.root, self.docs, staged)

    def test_decision_draft_and_closure_invalid_inputs_fail_closed(self) -> None:
        base = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        def apply(operation: dict) -> None:
            staged, _, _, _ = pipeline.apply_draft_patch(
                base,
                self.docs[pipeline.BOSS_PROFILES_REL],
                self.docs[pipeline.DAMAGE_REL],
                {
                    "schema": pipeline.DRAFT_PATCH_SCHEMA,
                    "formatVersion": 1,
                    "sourceRevision": self.source_manifest["sourceManifestId"],
                    "operations": [operation],
                },
                self.source_manifest["sourceManifestId"],
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )
            pipeline.project_v2_products(self.root, self.docs, staged)

        invalid_operations = (
            {
                "op": "SET_PATTERN_WEIGHT", "selectionSetId": "missing.set",
                "patternId": "VALTAN_DASH_CHARGE", "value": 31,
            },
            {
                "op": "SET_PATTERN_WEIGHT",
                "selectionSetId": "selectionset.valtan.160.130",
                "patternId": "VALTAN_DASH_CHARGE", "value": 0,
            },
            {
                "op": "SET_MECHANIC_TRIGGER", "mechanicId": "missing.mechanic",
                "patternId": "VALTAN_ARENA_BREAK_109", "healthBar": 100,
                "triggerOrder": 2,
            },
            {
                "op": "SET_MECHANIC_TRIGGER",
                "mechanicId": "mechanic.valtan-arena-break-109",
                "patternId": "VALTAN_ARENA_BREAK_109", "healthBar": 0,
                "triggerOrder": 1,
            },
            {
                "op": "SET_MECHANIC_TRIGGER",
                "mechanicId": "mechanic.valtan-arena-break-109",
                "patternId": "VALTAN_ARENA_BREAK_109", "healthBar": 161,
                "triggerOrder": 1,
            },
            {
                "op": "SET_MECHANIC_TRIGGER",
                "mechanicId": "mechanic.valtan-arena-break-109",
                "patternId": "VALTAN_ARENA_BREAK_109", "healthBar": 100,
                "triggerOrder": 1,
            },
            {
                "op": "SET_MECHANIC_TRIGGER",
                "mechanicId": "mechanic.valtan-arena-break-109",
                "patternId": "VALTAN_ARENA_BREAK_109", "healthBar": 100,
                "triggerOrder": 2,
            },
        )
        for operation in invalid_operations:
            with self.subTest(operation=operation), self.assertRaises(pipeline.PipelineError):
                apply(operation)

        invalid_masters = []
        duplicate_candidate = copy.deepcopy(base)
        duplicate_candidate["decisionModel"]["selectionSets"][0]["candidates"].append(
            copy.deepcopy(duplicate_candidate["decisionModel"]["selectionSets"][0]["candidates"][0])
        )
        invalid_masters.append(duplicate_candidate)
        zero_weight = copy.deepcopy(base)
        zero_weight["decisionModel"]["selectionSets"][0]["candidates"][0]["weight"] = 0
        invalid_masters.append(zero_weight)
        all_disabled = copy.deepcopy(base)
        for row in all_disabled["decisionModel"]["selectionSets"][0]["candidates"]:
            row["enabled"] = False
        invalid_masters.append(all_disabled)
        overlap = copy.deepcopy(base)
        overlap["decisionModel"]["selectionWindows"][1]["maximumHealthBarInclusive"] = 129
        invalid_masters.append(overlap)
        unknown_set = copy.deepcopy(base)
        unknown_set["decisionModel"]["selectionWindows"][0]["selectionSetId"] = "missing.set"
        invalid_masters.append(unknown_set)
        phase_boundary_drift = copy.deepcopy(base)
        phase_boundary_drift["decisionModel"]["mechanics"][1]["trigger"]["healthBar"] = 100
        invalid_masters.append(phase_boundary_drift)
        for invalid in invalid_masters:
            with self.assertRaises(pipeline.PipelineError):
                pipeline.validate_v2_master(
                    invalid,
                    self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL],
                )

        coordinated_but_unpromoted = copy.deepcopy(base)
        coordinated_but_unpromoted["decisionModel"]["selectionWindows"][1][
            "minimumHealthBarExclusive"
        ] = 100
        coordinated_but_unpromoted["decisionModel"]["mechanics"][1]["trigger"][
            "healthBar"
        ] = 100
        with self.assertRaisesRegex(
            pipeline.PipelineError, "first legacy rotation boundary"
        ):
            pipeline.project_v2_products(
                self.root, self.docs, coordinated_but_unpromoted
            )

    def test_manual_audition_rows_are_strict_disjoint_and_compile_fail_closed(self) -> None:
        staged = self.with_manual_audition()
        pipeline.validate_v2_master(
            staged,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        manual_pattern = staged["patterns"][-1]
        product = pipeline.compile_pattern_product(staged, manual_pattern)
        self.assertEqual(pipeline.AUDITION_ONLY, product["selectionMode"])
        self.assertEqual("ANY", product["armorRequirement"])
        self.assertEqual("ANY", product["phaseRequirement"])
        for field in (
            "minimumHealthBar",
            "maximumHealthBar",
            "triggerHealthBar",
            "triggerOrder",
            "selectionWeight",
            "maximumConsecutiveUses",
        ):
            self.assertEqual(0, product[field], field)
        self.assertEqual(1, product["minimumPhase"])
        self.assertEqual(3, product["maximumPhase"])
        self.assertEqual(0.0, product["minimumRange"])
        self.assertEqual(1.0, product["maximumRange"])

        invalid_masters = []

        unknown_field = copy.deepcopy(staged)
        unknown_field["decisionModel"]["manualAuditions"][0]["note"] = "draft"
        invalid_masters.append(unknown_field)

        missing_pattern = copy.deepcopy(staged)
        missing_pattern["decisionModel"]["manualAuditions"][0][
            "patternId"
        ] = "VALTAN_ANIMATION_PHASE2_MISSING"
        invalid_masters.append(missing_pattern)

        invalid_phase = copy.deepcopy(staged)
        invalid_phase["decisionModel"]["manualAuditions"][0]["authoringPhase"] = 0
        invalid_masters.append(invalid_phase)

        invalid_admission = copy.deepcopy(staged)
        invalid_admission["decisionModel"]["manualAuditions"][0][
            "admissionState"
        ] = "PRODUCT_ROTATION"
        invalid_masters.append(invalid_admission)

        overlap = copy.deepcopy(staged)
        overlap["decisionModel"]["selectionSets"][0]["candidates"].append(
            {
                "patternId": manual_pattern["patternId"],
                "weight": 1,
                "enabled": True,
            }
        )
        invalid_masters.append(overlap)

        duplicate = copy.deepcopy(staged)
        duplicate["decisionModel"]["manualAuditions"].append(
            copy.deepcopy(duplicate["decisionModel"]["manualAuditions"][0])
        )
        invalid_masters.append(duplicate)

        duplicate_source_chain = self.with_manual_audition(
            staged,
            pattern_id="VALTAN_ANIMATION_PHASE2_PIPELINE_TEST_SECOND",
            source_chain_id="pipeline-test-chain-second",
        )
        duplicate_source_chain["decisionModel"]["manualAuditions"][1][
            "sourceChainId"
        ] = "pipeline-test-chain"
        invalid_masters.append(duplicate_source_chain)

        for invalid in invalid_masters:
            with self.subTest(invalid=invalid["decisionModel"]["manualAuditions"]), self.assertRaises(
                pipeline.PipelineError
            ):
                pipeline.validate_v2_master(
                    invalid,
                    self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL],
                )

    def test_only_known_cinematic_may_remain_dormant_without_a_decision_owner(self) -> None:
        joined = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        sequence = joined["decisionModel"]["scriptedSequence"]["patternIds"]
        self.assertEqual("VALTAN_WHIRLWIND", sequence[0])
        self.assertNotIn("VALTAN_ENTRANCE_CINEMATIC", sequence)
        owned = {
            candidate["patternId"]
            for selection_set in joined["decisionModel"]["selectionSets"]
            for candidate in selection_set["candidates"]
        } | {
            mechanic["patternId"]
            for mechanic in joined["decisionModel"]["mechanics"]
        } | {
            audition["patternId"]
            for audition in joined["decisionModel"]["manualAuditions"]
        }
        self.assertNotIn("VALTAN_ENTRANCE_CINEMATIC", owned)
        pipeline.validate_v2_master(
            joined,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        explicit_entry = copy.deepcopy(joined)
        explicit_entry["decisionModel"]["scriptedSequence"]["patternIds"].insert(
            0, pipeline.OPTIONAL_ENTRY_PATTERN_ID
        )
        pipeline.validate_v2_master(
            explicit_entry, self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        repeated_entry = copy.deepcopy(explicit_entry)
        repeated_entry["decisionModel"]["scriptedSequence"]["patternIds"][1] = (
            pipeline.OPTIONAL_ENTRY_PATTERN_ID
        )
        with self.assertRaisesRegex(pipeline.PipelineError, "optional entry cinematic"):
            pipeline.validate_v2_master(
                repeated_entry, self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        invalid = copy.deepcopy(joined)
        for selection_set in invalid["decisionModel"]["selectionSets"]:
            selection_set["candidates"] = [
                candidate
                for candidate in selection_set["candidates"]
                if candidate["patternId"] != "VALTAN_WHIRLWIND"
            ]
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "only VALTAN_ENTRANCE_CINEMATIC may be a dormant entry-only definition",
        ):
            pipeline.validate_v2_master(
                invalid,
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

    def test_phase_two_and_three_animation_intake_exact_joins_manual_product(self) -> None:
        joined = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_manual_audition_animation_lineage(
            joined,
            self.debug_presentation,
            self.animation_promotion_manifest,
        )

        intake_chain_ids = [
            row["sourceChainId"]
            for row in self.animation_promotion_manifest["animationIntakeOnly"]
        ]
        manual_chain_ids = [
            row["sourceChainId"]
            for row in joined["decisionModel"]["manualAuditions"]
            if row["admissionState"] == pipeline.MANUAL_SERVER_AUDITION
        ]
        self.assertEqual(["front-back-front"], intake_chain_ids)
        self.assertTrue(set(intake_chain_ids).isdisjoint(manual_chain_ids))
        self.assertTrue({"respawn", "dead"}.issubset(set(manual_chain_ids)))
        ghost_wrappers = {
            row["patternId"]: row
            for row in joined["patterns"]
            if row["patternId"] in {
                "VALTAN_GHOST_RESPAWN_AUDITION",
                "VALTAN_GHOST_DEATH_AUDITION",
            }
        }
        self.assertEqual(
            {
                "VALTAN_GHOST_RESPAWN_AUDITION",
                "VALTAN_GHOST_DEATH_AUDITION",
            },
            set(ghost_wrappers),
        )
        self.assertEqual(
            ["mesh_respawn_1"],
            [
                occurrence["clip"]
                for stage in ghost_wrappers["VALTAN_GHOST_RESPAWN_AUDITION"]["stages"]
                for occurrence in stage["animation"]["occurrences"]
            ],
        )
        self.assertEqual(
            ["mesh_dead_1"],
            [
                occurrence["clip"]
                for stage in ghost_wrappers["VALTAN_GHOST_DEATH_AUDITION"]["stages"]
                for occurrence in stage["animation"]["occurrences"]
            ],
        )

        wrong_clip = copy.deepcopy(self.debug_presentation)
        wrong_clip["chains"][0]["animation"]["occurrences"][0]["clip"] = (
            "mesh_att_battle_invalid"
        )
        with self.assertRaisesRegex(
            pipeline.PipelineError, "no longer matches joined Product"
        ):
            pipeline.validate_manual_audition_animation_lineage(
                joined,
                wrong_clip,
                self.animation_promotion_manifest,
            )

        half_targeted = copy.deepcopy(self.debug_presentation)
        half_targeted["chains"][0]["targetPatternId"] = (
            joined["decisionModel"]["manualAuditions"][0]["patternId"]
        )
        with self.assertRaisesRegex(pipeline.PipelineError, "empty or paired"):
            pipeline.validate_manual_audition_animation_lineage(
                joined,
                half_targeted,
                self.animation_promotion_manifest,
            )

        unmapped = copy.deepcopy(self.debug_presentation)
        unmapped["chains"].append(copy.deepcopy(unmapped["chains"][0]))
        unmapped["chains"][-1]["chainId"] = "new-phase-three-intake"
        with self.assertRaisesRegex(
            pipeline.PipelineError, "must exact-join promotion plus"
        ):
            pipeline.validate_manual_audition_animation_lineage(
                joined,
                unmapped,
                self.animation_promotion_manifest,
            )

        targeted_intake = copy.deepcopy(self.debug_presentation)
        intake_chain = next(
            row for row in targeted_intake["chains"]
            if row["chainId"] == "front-back-front"
        )
        intake_chain["targetPatternId"] = (
            joined["decisionModel"]["manualAuditions"][0]["patternId"]
        )
        intake_chain["targetStageId"] = (
            joined["patterns"][0]["stages"][0]["stageId"]
        )
        with self.assertRaisesRegex(
            pipeline.PipelineError, "intake-only chain targets must remain empty"
        ):
            pipeline.validate_manual_audition_animation_lineage(
                joined,
                targeted_intake,
                self.animation_promotion_manifest,
            )

    def test_manual_audition_projection_appends_after_existing_product_ordinals(self) -> None:
        staged = self.with_manual_audition()
        managed_pattern_ids = {row["patternId"] for row in staged["patterns"]}
        pipeline.validate_legacy_products(
            self.docs[pipeline.LEGACY_REL], self.docs, managed_pattern_ids
        )

        encounter_before_text = pipeline.read_text(
            self.root / pipeline.ENCOUNTER_REL
        )
        bindings_before_text = pipeline.read_text(self.root / pipeline.BINDINGS_REL)
        encounter_before = self.docs[pipeline.ENCOUNTER_REL]
        bindings_before = self.docs[pipeline.BINDINGS_REL]

        first = pipeline.project_v2_products(self.root, self.docs, staged)
        second = pipeline.project_v2_products(self.root, self.docs, staged)
        self.assertEqual(first, second)

        encounter_after = json.loads(
            first[pipeline.ENCOUNTER_REL],
            object_pairs_hook=pipeline._reject_duplicate_pairs,
        )
        bindings_after = json.loads(
            first[pipeline.BINDINGS_REL],
            object_pairs_hook=pipeline._reject_duplicate_pairs,
        )
        self.assertEqual(
            [row["patternId"] for row in encounter_before["patterns"]],
            [row["patternId"] for row in encounter_after["patterns"][:-1]],
        )
        self.assertEqual(
            "VALTAN_ANIMATION_PHASE2_PIPELINE_TEST",
            encounter_after["patterns"][-1]["patternId"],
        )
        self.assertEqual(
            [row["actionId"] for row in bindings_before["bindings"]],
            [row["actionId"] for row in bindings_after["bindings"][:-1]],
        )
        self.assertEqual(
            "valtan.animation.phase2.pipeline-test-chain.step-01",
            bindings_after["bindings"][-1]["actionId"],
        )
        self.assertEqual(
            pipeline.AUDITION_ONLY,
            encounter_after["patterns"][-1]["selectionMode"],
        )

        pipeline._assert_unmanaged_raw_rows_preserved(
            encounter_before_text,
            first[pipeline.ENCOUNTER_REL],
            "patterns",
            "patternId",
            managed_pattern_ids,
        )
        managed_action_ids = {
            stage["actionId"]
            for pattern in staged["patterns"]
            for stage in pattern["stages"]
        }
        pipeline._assert_unmanaged_raw_rows_preserved(
            bindings_before_text,
            first[pipeline.BINDINGS_REL],
            "bindings",
            "actionId",
            managed_action_ids,
        )

        projected_docs = copy.deepcopy(self.docs)
        projected_docs[pipeline.ENCOUNTER_REL] = encounter_after
        projected_docs[pipeline.BINDINGS_REL] = bindings_after
        pipeline.validate_legacy_products(
            projected_docs[pipeline.LEGACY_REL],
            projected_docs,
            managed_pattern_ids,
        )

    def test_explicit_retirement_removes_cues_without_shifting_legacy_bindings(self) -> None:
        staged = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        retired_id = "VALTAN_THREE"
        retired = next(row for row in staged["patterns"] if row["patternId"] == retired_id)
        action_ids = {stage["actionId"] for stage in retired["stages"]}
        cue_ids = {cue["cueId"] for stage in retired["stages"] for cue in stage["effectCues"]}
        self.assertTrue(cue_ids)
        staged["retiredPatternIds"].append(retired_id)
        staged["patterns"].remove(retired)
        staged["decisionModel"]["manualAuditions"] = [
            row for row in staged["decisionModel"]["manualAuditions"] if row["patternId"] != retired_id
        ]
        staged["decisionModel"]["scriptedSequence"]["patternIds"].remove(retired_id)

        first = pipeline.project_v2_products(self.root, self.docs, staged)
        encounter = json.loads(first[pipeline.ENCOUNTER_REL])
        remaining = {row["patternId"] for row in encounter["patterns"]}
        self.assertNotIn(retired_id, remaining)
        self.assertTrue({"VALTAN_FRONT_BACK_FRONT", "VALTAN_GHOST_TRANSITION_15"}.issubset(remaining))
        cues = json.loads(first[pipeline.CUES_REL])["cues"]
        self.assertTrue(cue_ids.isdisjoint(row["bindingId"] for row in cues))
        bindings = json.loads(first[pipeline.BINDINGS_REL])["bindings"]
        before = self.docs[pipeline.BINDINGS_REL]["bindings"]
        self.assertEqual(len(before), len(bindings))
        for old, new in zip(before, bindings):
            expected = (
                {"actionId": old["actionId"], "playbackMode": "NONE", "clips": []}
                if old["actionId"] in action_ids else old
            )
            self.assertEqual(expected, new)

        # A second projection reads the already-retired Product state. It must
        # neither recreate its cues nor turn the ordinal tombstones back on.
        projected_docs = copy.deepcopy(self.docs)
        with tempfile.TemporaryDirectory(prefix="valtan-retirement.") as temporary:
            root = Path(temporary)
            for relative, text in first.items():
                path = root / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(text.encode("utf-8"))
                projected_docs[relative] = json.loads(text)
            self.assertEqual(first, pipeline.project_v2_products(root, projected_docs, staged))

        for retired_rows in ([*staged["retiredPatternIds"], retired_id], ["VALTAN_WHIRLWIND"], "not-an-array"):
            invalid = copy.deepcopy(staged)
            invalid["retiredPatternIds"] = retired_rows
            with self.subTest(retired_rows=retired_rows):
                with self.assertRaisesRegex(pipeline.PipelineError, "retiredPatternIds"):
                    pipeline.validate_v2_master(
                        invalid, self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL]
                    )

    def test_removed_manual_audition_fails_closed_before_stale_product_projection(self) -> None:
        staged = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        removed_id = staged["decisionModel"]["manualAuditions"][-1]["patternId"]
        removed_pattern = next(
            row for row in staged["patterns"] if row["patternId"] == removed_id
        )
        removed_cue_ids = {
            cue["cueId"]
            for stage in removed_pattern["stages"]
            for cue in stage["effectCues"]
        }
        staged["decisionModel"]["manualAuditions"] = [
            row
            for row in staged["decisionModel"]["manualAuditions"]
            if row["patternId"] != removed_id
        ]
        staged["patterns"] = [
            row for row in staged["patterns"] if row["patternId"] != removed_id
        ]
        staged["decisionModel"]["scriptedSequence"]["patternIds"] = [
            pattern_id
            for pattern_id in staged["decisionModel"]["scriptedSequence"]["patternIds"]
            if pattern_id != removed_id
        ]

        remaining_cue_policies = {
            cue_id: policy
            for cue_id, policy in pipeline.MANAGED_CUE_SCALE_POLICIES.items()
            if cue_id not in removed_cue_ids
        }
        # Isolate the stale Product ownership boundary.  Otherwise removing a
        # cue-owning audition correctly fails first at the independent sequence
        # reference or managed cue scale-policy closure.
        with mock.patch.object(
            pipeline,
            "MANAGED_CUE_SCALE_POLICIES",
            remaining_cue_policies,
        ):
            with self.assertRaisesRegex(
                pipeline.PipelineError,
                "Product AUDITION_ONLY ownership drift.*" + removed_id,
            ):
                pipeline.project_v2_products(self.root, self.docs, staged)

    def test_gameplay_publisher_rejects_partial_phase_boundary_product(self) -> None:
        encounter = copy.deepcopy(self.docs[pipeline.ENCOUNTER_REL])
        arena_break = next(
            row
            for row in encounter["patterns"]
            if row["patternId"] == "VALTAN_ARENA_BREAK_109"
        )
        arena_break["triggerHealthBar"] = 100
        arena_break["triggerOrder"] = 2
        with tempfile.TemporaryDirectory() as temporary:
            overlay_root = Path(temporary)
            encounter_path = overlay_root / pipeline.ENCOUNTER_REL
            encounter_path.parent.mkdir(parents=True)
            encounter_path.write_text(
                json.dumps(encounter, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            completed = subprocess.run(
                [
                    "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                    "-File",
                    str(
                        self.root
                        / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"
                    ),
                    "-Mode", "Validate",
                    "-InputOverlayRoot", str(overlay_root),
                ],
                cwd=self.root,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
        self.assertNotEqual(0, completed.returncode)
        # Windows PowerShell may hard-wrap a terminating error at the host
        # console width even though the subprocess streams are captured.  The
        # diagnostic contract is its text, not those presentation-only breaks.
        diagnostic = "".join((completed.stdout + completed.stderr).split())
        self.assertIn(
            "triggerHealthBarmustequalthefinalphase-1WINDOW.toHealthBar",
            diagnostic,
        )

    def test_gameplay_publisher_rejects_raw_finale_order_invulnerability_and_late_cycle(self) -> None:
        for defect in ("reordered attacks", "invulnerable owner", "late child cycle"):
            with self.subTest(defect=defect), tempfile.TemporaryDirectory() as temporary:
                encounter = copy.deepcopy(self.docs[pipeline.ENCOUNTER_REL])
                finale = next(row for row in encounter["patterns"]
                              if row["patternId"] == "VALTAN_GHOST_FINALE")
                expected = "Ghost finale must remain damageable and use its three ordered attacks"
                if defect == "reordered attacks":
                    finale["finale"]["ghostPatternIds"].reverse()
                elif defect == "invulnerable owner":
                    finale["invulnerableWhileRunning"] = True
                else:
                    child = next(row for row in encounter["patterns"]
                                 if row["patternId"] == "VALTAN_WHIRLWIND")
                    # A terminal first stage makes the late two-node cycle unreachable.
                    # The Product validator must still inspect every node.
                    child["stages"][0]["branches"] = [
                        {"outcome": "TIMEOUT", "nextActionId": None}]
                    tail = child["stages"][-1]
                    tail["branches"] = [
                        {"outcome": "TIMEOUT", "nextActionId": child["stages"][-2]["actionId"]}]
                    expected = "Finite pattern stage graph contains a cycle: VALTAN_WHIRLWIND"
                overlay_root = Path(temporary)
                encounter_path = overlay_root / pipeline.ENCOUNTER_REL
                encounter_path.parent.mkdir(parents=True)
                encounter_path.write_text(
                    json.dumps(encounter, ensure_ascii=False, indent=2) + "\n",
                    encoding="utf-8",
                )
                completed = subprocess.run(
                    ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                     str(self.root / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"),
                     "-Mode", "Validate", "-InputOverlayRoot", str(overlay_root)],
                    cwd=self.root, capture_output=True, text=True, encoding="utf-8", check=False,
                )
                self.assertNotEqual(0, completed.returncode)
                self.assertIn(expected, completed.stdout + completed.stderr)

    def test_world_publisher_rejects_static_ghost_and_preserves_runtime(self) -> None:
        runtime_roots = (
            Path("Server/Bin/DataFiles/World"),
            Path("Client/Bin/DataFiles/World"),
        )

        def runtime_bytes(repository: Path) -> dict[Path, bytes]:
            return {
                path.relative_to(repository): path.read_bytes()
                for runtime_root in runtime_roots
                for path in (repository / runtime_root).rglob("*")
                if path.is_file()
            }

        original_runtime = runtime_bytes(self.root)
        for runtime_root in runtime_roots:
            self.assertTrue(
                any(path.is_relative_to(runtime_root) for path in original_runtime),
                f"Missing canonical World runtime fixture: {runtime_root}",
            )
        with tempfile.TemporaryDirectory() as temporary:
            repository_root = Path(temporary) / "repository"
            inputs = [
                Path("Tools/WorldPipeline/Publish-WorldGameplay.ps1"),
                Path("Data/Actors/CharacterCatalog.json"),
                Path("Data/Actors/BossCatalog.json"),
                Path("Data/Actors/NpcCatalog.json"),
                Path("Data/Actors/MonsterCatalog.json"),
                Path("Data/Balance/MonsterProfiles.json"),
                Path(pipeline.ENCOUNTER_REL),
            ]
            for area_id in (
                "LV_BER_BERNCASTLE", "LV_LUT_HEARTRB_ED",
                "LV_DEV_TRAINING_GROUND", "LV_LOBBY_CLASSSELECT_SL00",
            ):
                for name in (
                    "Gameplay.world.json", "SpawnGroups.world.json",
                    "EncounterProps.world.json",
                ):
                    relative = Path("Data/Worlds") / area_id / name
                    if (self.root / relative).is_file():
                        inputs.append(relative)
                deploy = Path("Data/Maps/Authoring") / area_id / f"{area_id}.deployplacements"
                if (self.root / deploy).is_file():
                    inputs.append(deploy)
            for relative in inputs:
                destination = repository_root / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(self.root / relative, destination)
            for relative, payload in original_runtime.items():
                destination = repository_root / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(payload)
            command = [
                "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                str(repository_root / "Tools/WorldPipeline/Publish-WorldGameplay.ps1"),
                "-Mode", "Validate",
            ]
            baseline = subprocess.run(
                command, cwd=repository_root, capture_output=True, text=True,
                encoding="utf-8", check=False,
            )
            self.assertEqual(0, baseline.returncode, baseline.stdout + baseline.stderr)
            self.assertEqual(original_runtime, runtime_bytes(repository_root))

            world_path = repository_root / "Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json"
            world = pipeline.read_json(world_path)
            boss = next(row for row in world["placements"]
                        if row["placementId"] == "boss.valtan.center")
            self.assertEqual("BOSS_VALTAN", boss["archetypeId"])
            boss["archetypeId"] = "BOSS_VALTAN_GHOST"
            world_path.write_text(
                json.dumps(world, ensure_ascii=False, indent=2) + "\n", encoding="utf-8",
            )
            rejected = subprocess.run(
                command, cwd=repository_root, capture_output=True, text=True,
                encoding="utf-8", check=False,
            )
            self.assertNotEqual(0, rejected.returncode)
            self.assertIn(
                "Boss placement archetype does not match encounter 'ENCOUNTER_VALTAN'.",
                rejected.stdout + rejected.stderr,
            )
            self.assertEqual(original_runtime, runtime_bytes(repository_root))
        self.assertEqual(original_runtime, runtime_bytes(self.root))

    def test_draft_patch_revision_duplicate_and_radial_v18_projection(self) -> None:
        wrong = self.draft_patch()
        wrong["sourceRevision"] = "0" * 64
        with self.assertRaisesRegex(pipeline.DraftPatchError, "sourceRevision precondition"):
            pipeline.apply_draft_patch(
                self.migrate(),
                self.docs[pipeline.BOSS_PROFILES_REL],
                self.docs[pipeline.DAMAGE_REL],
                wrong,
                self.source_manifest["sourceManifestId"],
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        duplicate = self.draft_patch()
        duplicate["operations"].append(copy.deepcopy(duplicate["operations"][2]))
        with self.assertRaisesRegex(pipeline.DraftPatchError, "duplicate draft target"):
            pipeline.apply_draft_patch(
                self.migrate(),
                self.docs[pipeline.BOSS_PROFILES_REL],
                self.docs[pipeline.DAMAGE_REL],
                duplicate,
                self.source_manifest["sourceManifestId"],
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        radial = self.draft_patch()
        volley = next(row for row in radial["operations"] if row["op"] == "SET_AXE_VOLLEY")
        volley["countPerResolvedTarget"] = 3
        volley["layout"] = {
            "kind": "RADIAL_AROUND_TARGET",
            "radiusM": 2.0,
            "startAngleDegrees": 0.0,
            "angleStepDegrees": 120.0,
        }
        staged, _, _, _ = pipeline.apply_draft_patch(
            self.migrate(),
            self.docs[pipeline.BOSS_PROFILES_REL],
            self.docs[pipeline.DAMAGE_REL],
            radial,
            self.source_manifest["sourceManifestId"],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        staged_volley = next(
            event
            for pattern in staged["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
            for event in stage["events"]
            if event["eventId"]
            == "event.valtan.high-jump.airborne.spawn-target-axe"
        )
        self.assertEqual(
            {
                "kind": "INTERVAL",
                "count": 3,
                "firstOffsetMs": 0,
                "intervalMs": 1333,
            },
            staged_volley["spawnSchedule"],
        )
        self.assertEqual(
            {
                "kind": "RANDOM_NAVIGABLE_CIRCLE",
                "anchor": "BOSS_SPAWN_POSITION",
                "count": 4,
                "radiusM": 14.0,
                "heightToleranceM": 1.0,
            },
            staged_volley["arenaRandom"],
        )
        projected = pipeline.project_v2_products(
            self.root, self.docs, staged, migration_fixture=True
        )
        encounter = json.loads(projected[pipeline.ENCOUNTER_REL])
        high_jump = next(
            row for row in encounter["patterns"]
            if row["patternId"] == "VALTAN_HIGH_JUMP"
        )
        airborne = next(
            row for row in high_jump["stages"] if row["stageId"] == "AIRBORNE"
        )
        self.assertEqual(
            {
                "trigger": "ENTER",
                "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
                "targetId": "combatobject.valtan.high-jump.target-axe",
                "targetingPolicy": "PER_ALIVE_PLAYER",
                "countPerResolvedTarget": 3,
                "layout": "RADIAL",
                "radiusM": 2.0,
                "startAngleDegrees": 0.0,
                "angleStepDegrees": 120.0,
                "allowOverlap": False,
                "maximumTotalObjects": 36,
                "spawnCount": 3,
                "spawnIntervalMs": 1333,
                "arenaRandomCount": 4,
                "arenaRandomRadiusM": 14.0,
                "arenaHeightToleranceM": 1.0,
                "arenaAnchorPolicy": "BOSS_SPAWN_POSITION",
            },
            airborne["actions"][0],
        )
        with tempfile.TemporaryDirectory() as temporary:
            candidate_root = Path(temporary) / "candidates"
            pointer = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
                draft_patch=radial,
            )
            self.assertFalse(pointer["activeRuntimeChanged"])
            self.assertTrue((candidate_root / "current-candidate.json").exists())

    def test_saved_authoring_revision_chains_and_publishes_without_touching_v1(self) -> None:
        source_path = self.root / pipeline.MASTER_REL
        source_before = pipeline.sha256_file(source_path)
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            first = pipeline.save_authoring(self.root, authoring_root, self.draft_patch())
            self.assertFalse(first["activeRuntimeChanged"])
            first_revision = first["revisionId"]
            first_master, first_bosses, first_damage = pipeline.load_authoring_revision(
                self.root,
                authoring_root,
                first_revision,
                self.source_manifest,
                self.docs,
            )
            self.assertEqual(2, first_master["formatVersion"])
            self.assertEqual(61000, first_bosses["bosses"][0]["maximumHp"])
            self.assertEqual(
                325,
                next(
                    row["damageRatePercent"]
                    for row in first_damage["profiles"]
                    if row["damageProfileId"] == "damage.valtan.circular-spin"
                ),
            )

            second_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": first_revision,
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 350,
                    }
                ],
            }
            validation = pipeline.validate_draft_patch(
                self.root, second_patch, authoring_root
            )
            self.assertEqual(first_revision, validation["baseRevision"])
            second = pipeline.save_authoring(self.root, authoring_root, second_patch)
            self.assertNotEqual(first_revision, second["revisionId"])
            candidate_root = temporary_root / "candidates"
            published = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
                authoring_root=authoring_root,
                authoring_revision=second["revisionId"],
            )
            revision_root = candidate_root / "revisions" / published["revisionId"]
            bosses = pipeline.read_json(revision_root / pipeline.BOSS_PROFILES_REL)
            damage = pipeline.read_json(revision_root / pipeline.DAMAGE_REL)
            self.assertEqual(61000, bosses["bosses"][0]["maximumHp"])
            self.assertEqual(
                350,
                next(
                    row["damageRatePercent"]
                    for row in damage["profiles"]
                    if row["damageProfileId"] == "damage.valtan.circular-spin"
                ),
            )
            manifest = pipeline.read_json(revision_root / "revision-manifest.json")
            self.assertEqual(second["revisionId"], manifest["authoringBaseRevision"])
        self.assertEqual(source_before, pipeline.sha256_file(source_path))

    def test_authoring_save_uses_current_head_compare_and_swap(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            authoring_root = Path(temporary) / "authoring"
            first = pipeline.save_authoring(
                self.root, authoring_root, self.draft_patch()
            )
            second_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": first["revisionId"],
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 350,
                    }
                ],
            }
            second = pipeline.save_authoring(
                self.root, authoring_root, second_patch
            )
            head_before = (authoring_root / "current-authoring.json").read_bytes()
            revisions_before = {
                row.name for row in (authoring_root / "revisions").iterdir()
            }
            stale_patch = copy.deepcopy(second_patch)
            stale_patch["operations"][0]["value"] = 375
            with self.assertRaisesRegex(
                pipeline.DraftPatchError, "current saved authoring head"
            ) as rejected:
                pipeline.save_authoring(self.root, authoring_root, stale_patch)
            self.assertEqual("SOURCE_REVISION_MISMATCH", rejected.exception.error_code)
            self.assertEqual(
                second["revisionId"],
                pipeline.read_json(authoring_root / "current-authoring.json")[
                    "revisionId"
                ],
            )
            self.assertEqual(
                head_before, (authoring_root / "current-authoring.json").read_bytes()
            )
            self.assertEqual(
                revisions_before,
                {row.name for row in (authoring_root / "revisions").iterdir()},
            )
            self.assertFalse((authoring_root / ".save.lock").exists())
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            with self.assertRaisesRegex(
                pipeline.DraftPatchError, "current saved authoring head"
            ):
                pipeline.validate_draft_patch(
                    self.root, stale_patch, authoring_root
                )
            candidate_root = Path(temporary) / "candidates"
            with self.assertRaisesRegex(
                pipeline.DraftPatchError, "current saved authoring head"
            ):
                pipeline.publish_candidate(
                    self.root,
                    candidate_root,
                    draft_patch=stale_patch,
                    authoring_root=authoring_root,
                )
            with self.assertRaisesRegex(
                pipeline.DraftPatchError, "current saved authoring head"
            ):
                pipeline.publish_candidate(
                    self.root,
                    candidate_root,
                    authoring_root=authoring_root,
                    authoring_revision=first["revisionId"],
                )
            self.assertFalse((candidate_root / "current-candidate.json").exists())
            self.assertFalse((candidate_root / ".publish.lock").exists())

    def test_existing_authoring_revision_collision_revalidates_every_artifact(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            authoring_root = Path(temporary) / "authoring"
            patch = self.draft_patch()
            saved = pipeline.save_authoring(self.root, authoring_root, patch)
            revision_root = authoring_root / "revisions" / saved["revisionId"]
            gameplay_path = revision_root / pipeline.GAMEPLAY_AUTHORING_REL
            original = gameplay_path.read_bytes()
            pipeline._write_fsync(gameplay_path, original + b" ")

            # Re-enter the deterministic same-revision branch without a head.
            # A manifest-only comparison used to accept the tampered immutable
            # directory and recreate a pointer to corrupt bytes.
            (authoring_root / "current-authoring.json").unlink()
            with self.assertRaisesRegex(
                pipeline.PipelineError,
                "authoring artifact hash/path mismatch",
            ):
                pipeline.save_authoring(self.root, authoring_root, patch)

            self.assertFalse((authoring_root / "current-authoring.json").exists())
            self.assertEqual(original + b" ", gameplay_path.read_bytes())
            self.assertFalse((authoring_root / ".save.lock").exists())
            self.assertFalse((authoring_root / ".save-journal.json").exists())

    def test_authoring_save_failure_injection_rolls_back_every_pointer_stage(self) -> None:
        points = (
            "after_stage", "after_validate", "after_revision_manifest",
            "before_promote", "after_promote", "before_pointer", "after_pointer",
        )
        for point in points:
            with self.subTest(point=point), tempfile.TemporaryDirectory() as temporary:
                authoring_root = Path(temporary) / "authoring"
                with self.assertRaises(pipeline.InjectedFailure):
                    pipeline.save_authoring(
                        self.root,
                        authoring_root,
                        self.draft_patch(),
                        fail_at=point,
                    )
                self.assertFalse((authoring_root / "current-authoring.json").exists())
                self.assertFalse((authoring_root / ".save-journal.json").exists())
                self.assertFalse((authoring_root / ".save.lock").exists())
                revisions = authoring_root / "revisions"
                self.assertFalse(revisions.exists() and any(revisions.iterdir()))
                self.assertFalse(any(authoring_root.glob(".stage.*")))
                self.assertFalse(any(authoring_root.glob(".current-authoring.stage.*")))

    def test_authoring_subprocess_hard_crash_recovers_staged_and_promoted(self) -> None:
        tracked = (
            pipeline.MASTER_REL,
            pipeline.COMBAT_AUTHORING_REL,
            pipeline.WORLD_SET_REL,
            pipeline.LEGACY_REL,
            pipeline.BOSS_PROFILES_REL,
            pipeline.DAMAGE_REL,
        )
        before = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            initial = pipeline.save_authoring(
                self.root, authoring_root, self.draft_patch()
            )
            initial_pointer_bytes = (authoring_root / "current-authoring.json").read_bytes()
            next_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": initial["revisionId"],
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 351,
                    }
                ],
            }
            invalid_patch = copy.deepcopy(next_patch)
            invalid_patch["sourceRevision"] = "0" * 64
            next_path = temporary_root / "next.json"
            invalid_path = temporary_root / "invalid.json"
            pipeline._write_fsync(next_path, pipeline.json_text(next_patch).encode("utf-8"))
            pipeline._write_fsync(
                invalid_path, pipeline.json_text(invalid_patch).encode("utf-8")
            )
            base_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
            ]

            staged_crash = subprocess.run(
                [
                    *base_command,
                    "--draft-patch",
                    str(next_path),
                    "--crash-at",
                    "after_journal_staged",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, staged_crash.returncode)
            staged_journal = pipeline.read_json(authoring_root / ".save-journal.json")
            staged_target = Path(staged_journal["targetPath"])
            self.assertEqual(initial_pointer_bytes, (authoring_root / "current-authoring.json").read_bytes())
            recovered = subprocess.run(
                [*base_command, "--draft-patch", str(invalid_path)],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, recovered.returncode)
            self.assertEqual(initial_pointer_bytes, (authoring_root / "current-authoring.json").read_bytes())
            self.assertFalse(staged_target.exists())
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())
            self.assertFalse(any(authoring_root.glob(".stage.*")))

            promoted_crash = subprocess.run(
                [
                    *base_command,
                    "--draft-patch",
                    str(next_path),
                    "--crash-at",
                    "after_promote",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, promoted_crash.returncode)
            promoted_journal = pipeline.read_json(authoring_root / ".save-journal.json")
            promoted_revision = promoted_journal["revisionId"]
            self.assertEqual(initial_pointer_bytes, (authoring_root / "current-authoring.json").read_bytes())
            recovered = subprocess.run(
                [*base_command, "--draft-patch", str(invalid_path)],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, recovered.returncode)
            self.assertEqual(
                promoted_revision,
                pipeline.read_json(authoring_root / "current-authoring.json")["revisionId"],
            )
            self.assertTrue(Path(promoted_journal["targetPath"]).is_dir())
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())

            pointer_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": promoted_revision,
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 352,
                    }
                ],
            }
            pointer_path = temporary_root / "pointer.json"
            pipeline._write_fsync(
                pointer_path, pipeline.json_text(pointer_patch).encode("utf-8")
            )
            pointer_crash = subprocess.run(
                [
                    *base_command,
                    "--draft-patch",
                    str(pointer_path),
                    "--crash-at",
                    "after_pointer",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, pointer_crash.returncode)
            pointer_revision = pipeline.read_json(
                authoring_root / ".save-journal.json"
            )["revisionId"]
            recovered = subprocess.run(
                [*base_command, "--draft-patch", str(invalid_path)],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, recovered.returncode)
            self.assertEqual(
                pointer_revision,
                pipeline.read_json(authoring_root / "current-authoring.json")["revisionId"],
            )
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())
        after = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        self.assertEqual(before, after)

    def test_authoring_micro_crash_windows_recover_without_permanent_lockout(self) -> None:
        expectations = {
            "after_staged_journal_temp": "ROLLBACK",
            "after_promoted_journal_temp": "ROLLBACK",
            "after_journal_unlink": "COMMIT",
        }
        for crash_point, outcome in expectations.items():
            with self.subTest(crash_point=crash_point), tempfile.TemporaryDirectory() as temporary:
                temporary_root = Path(temporary)
                authoring_root = temporary_root / "authoring"
                patch_path = temporary_root / "draft.json"
                pipeline._write_fsync(
                    patch_path,
                    pipeline.json_text(self.draft_patch()).encode("utf-8"),
                )
                crashed = subprocess.run(
                    [
                        sys.executable,
                        str(Path(pipeline.__file__).resolve()),
                        "--repository-root",
                        str(self.root),
                        "save-authoring",
                        "--authoring-root",
                        str(authoring_root),
                        "--draft-patch",
                        str(patch_path),
                        "--crash-at",
                        crash_point,
                    ],
                    capture_output=True,
                    text=True,
                    encoding="utf-8",
                    check=False,
                )
                self.assertEqual(
                    pipeline.HARD_CRASH_EXIT_CODE,
                    crashed.returncode,
                    crashed.stderr,
                )
                self.assertTrue((authoring_root / ".save.lock").is_file())
                if crash_point == "after_journal_unlink":
                    committed_revision = pipeline.read_json(
                        authoring_root / "current-authoring.json"
                    )["revisionId"]
                    self.assertFalse(
                        (authoring_root / ".save-journal.json").exists()
                    )
                _, payload, effective_revision = (
                    pipeline.source_manifest_with_authoring(
                        self.root, authoring_root
                    )
                )
                if outcome == "COMMIT":
                    self.assertEqual(committed_revision, effective_revision)
                    self.assertEqual(
                        committed_revision, payload["authoringRevision"]
                    )
                else:
                    self.assertEqual(
                        self.source_manifest["sourceManifestId"], effective_revision
                    )
                    self.assertIsNone(payload["authoringRevision"])
                    self.assertFalse(
                        (authoring_root / "current-authoring.json").exists()
                    )
                    revisions = authoring_root / "revisions"
                    self.assertFalse(
                        revisions.exists() and any(revisions.iterdir())
                    )
                self.assertFalse((authoring_root / ".save.lock").exists())
                self.assertFalse(
                    (authoring_root / ".save-journal.json").exists()
                )
                self.assertFalse(any(authoring_root.glob(".stage.*")))
                self.assertFalse(
                    any(authoring_root.glob(".save-journal.json.stage.*"))
                )

    def test_legacy_mutable_lock_update_and_delete_windows_are_recoverable(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(
                patch_path,
                pipeline.json_text(self.draft_patch()).encode("utf-8"),
            )
            base_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
                "--draft-patch",
                str(patch_path),
            ]
            crashed = subprocess.run(
                [*base_command, "--crash-at", "after_journal_staged"],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            journal_path = authoring_root / ".save-journal.json"
            lock_path = authoring_root / ".save.lock"
            journal = pipeline.read_json(journal_path)
            lock = pipeline.read_json(lock_path)
            lock["journalFileSha256"] = pipeline.sha256_file(journal_path)
            pipeline._write_fsync(
                lock_path, pipeline.json_text(lock).encode("utf-8")
            )
            os.replace(journal["stagePath"], journal["targetPath"])
            journal["state"] = "PROMOTED"
            journal = pipeline._seal_journal(journal)
            pipeline._write_fsync(
                journal_path, pipeline.json_text(journal).encode("utf-8")
            )
            _, payload, committed_revision = pipeline.source_manifest_with_authoring(
                self.root, authoring_root
            )
            self.assertEqual(journal["revisionId"], committed_revision)
            self.assertEqual(committed_revision, payload["authoringRevision"])

            next_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": committed_revision,
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 351,
                    }
                ],
            }
            pipeline._write_fsync(
                patch_path, pipeline.json_text(next_patch).encode("utf-8")
            )
            crashed = subprocess.run(
                [*base_command, "--crash-at", "after_pointer"],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            next_revision = pipeline.read_json(
                authoring_root / "current-authoring.json"
            )["revisionId"]
            lock = pipeline.read_json(lock_path)
            lock["journalFileSha256"] = pipeline.sha256_file(journal_path)
            pipeline._write_fsync(
                lock_path, pipeline.json_text(lock).encode("utf-8")
            )
            journal_path.unlink()
            _, payload, effective_revision = pipeline.source_manifest_with_authoring(
                self.root, authoring_root
            )
            self.assertEqual(next_revision, effective_revision)
            self.assertEqual(next_revision, payload["authoringRevision"])
            self.assertFalse(lock_path.exists())

    def test_cooperating_readers_recover_authoring_hard_crashes_before_read(self) -> None:
        tracked = (
            pipeline.MASTER_REL,
            pipeline.ENCOUNTER_REL,
            pipeline.COMBAT_PRODUCT_REL,
            pipeline.WORLD_PRODUCT_REL,
            pipeline.BOSS_PROFILES_REL,
            pipeline.DAMAGE_REL,
        )
        before = {
            relative: pipeline.sha256_file(self.root / relative)
            for relative in tracked
        }
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            candidate_root = temporary_root / "candidates"
            initial = pipeline.save_authoring(
                self.root, authoring_root, self.draft_patch()
            )
            initial_pointer = (authoring_root / "current-authoring.json").read_bytes()
            pipeline_path = str(Path(pipeline.__file__).resolve())
            save_command = [
                sys.executable,
                pipeline_path,
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
            ]

            staged_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": initial["revisionId"],
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 351,
                    }
                ],
            }
            staged_path = temporary_root / "staged.json"
            pipeline._write_fsync(
                staged_path, pipeline.json_text(staged_patch).encode("utf-8")
            )
            crashed = subprocess.run(
                [
                    *save_command,
                    "--draft-patch",
                    str(staged_path),
                    "--crash-at",
                    "after_journal_staged",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            source_reader = subprocess.run(
                [
                    sys.executable,
                    pipeline_path,
                    "--repository-root",
                    str(self.root),
                    "source-manifest",
                    "--authoring-root",
                    str(authoring_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, source_reader.returncode, source_reader.stderr)
            self.assertEqual(
                initial_pointer,
                (authoring_root / "current-authoring.json").read_bytes(),
            )
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())
            self.assertFalse(any(authoring_root.glob(".stage.*")))

            crashed = subprocess.run(
                [
                    *save_command,
                    "--draft-patch",
                    str(staged_path),
                    "--crash-at",
                    "after_promote",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            promoted_revision = pipeline.read_json(
                authoring_root / ".save-journal.json"
            )["revisionId"]
            followup_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": promoted_revision,
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 352,
                    }
                ],
            }
            followup_path = temporary_root / "followup.json"
            pipeline._write_fsync(
                followup_path, pipeline.json_text(followup_patch).encode("utf-8")
            )
            draft_reader = subprocess.run(
                [
                    sys.executable,
                    pipeline_path,
                    "--repository-root",
                    str(self.root),
                    "validate-draft",
                    "--draft-patch",
                    str(followup_path),
                    "--authoring-root",
                    str(authoring_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, draft_reader.returncode, draft_reader.stderr)
            self.assertEqual(
                promoted_revision,
                pipeline.read_json(authoring_root / "current-authoring.json")[
                    "revisionId"
                ],
            )

            crashed = subprocess.run(
                [
                    *save_command,
                    "--draft-patch",
                    str(followup_path),
                    "--crash-at",
                    "after_promote",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            publish_revision = pipeline.read_json(
                authoring_root / ".save-journal.json"
            )["revisionId"]
            candidate_reader = subprocess.run(
                [
                    sys.executable,
                    pipeline_path,
                    "--repository-root",
                    str(self.root),
                    "publish-candidate",
                    "--candidate-root",
                    str(candidate_root),
                    "--authoring-root",
                    str(authoring_root),
                    "--authoring-revision",
                    publish_revision,
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, candidate_reader.returncode, candidate_reader.stderr)
            candidate_pointer = pipeline.read_json(
                candidate_root / "current-candidate.json"
            )
            candidate_manifest = pipeline.read_json(
                candidate_root
                / candidate_pointer["manifest"]
            )
            self.assertEqual(
                publish_revision, candidate_manifest["authoringBaseRevision"]
            )
            self.assertEqual(
                publish_revision,
                pipeline.read_json(authoring_root / "current-authoring.json")[
                    "revisionId"
                ],
            )
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())
        after = {
            relative: pipeline.sha256_file(self.root / relative)
            for relative in tracked
        }
        self.assertEqual(before, after)

    def test_source_manifest_exposes_only_a_fully_validated_authoring_base(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            output_path = temporary_root / "repository-source.json"
            command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "source-manifest",
                "--authoring-root",
                str(authoring_root),
                "--output",
                str(output_path),
            ]

            no_pointer = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, no_pointer.returncode, no_pointer.stderr)
            no_pointer_result = json.loads(no_pointer.stdout)
            self.assertEqual(
                self.source_manifest["sourceManifestId"],
                no_pointer_result["sourceRevision"],
            )
            self.assertIsNone(
                no_pointer_result["payload"]["authoringRevision"]
            )
            self.assertEqual(
                self.source_manifest["sourceManifestId"],
                no_pointer_result["payload"]["sourceManifestId"],
            )
            self.assertEqual(self.source_manifest, pipeline.read_json(output_path))
            self.assertNotIn("authoringRevision", pipeline.read_json(output_path))

            live_root = temporary_root / "live-authoring"
            live_root.mkdir()
            live_lock, _, _ = pipeline._acquire_transaction_lock(
                live_root, "authoring"
            )
            live_command = list(command)
            live_command[live_command.index(str(authoring_root))] = str(live_root)
            try:
                rejected = subprocess.run(
                    live_command,
                    capture_output=True,
                    text=True,
                    encoding="utf-8",
                    check=False,
                )
                self.assertEqual(1, rejected.returncode)
                self.assertIn("live process", rejected.stderr)
                self.assertTrue((live_root / ".save.lock").is_file())
            finally:
                os.close(live_lock)
                (live_root / ".save.lock").unlink(missing_ok=True)

            saved = pipeline.save_authoring(
                self.root, authoring_root, self.draft_patch()
            )
            pointer_path = authoring_root / "current-authoring.json"
            valid_pointer_bytes = pointer_path.read_bytes()
            current = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, current.returncode, current.stderr)
            current_result = json.loads(current.stdout)
            self.assertEqual(saved["revisionId"], current_result["sourceRevision"])
            self.assertEqual(
                saved["revisionId"],
                current_result["payload"]["authoringRevision"],
            )
            self.assertEqual(self.source_manifest, pipeline.read_json(output_path))

            corrupt_pointer = pipeline.read_json(pointer_path)
            corrupt_pointer["unexpected"] = True
            pipeline._write_fsync(
                pointer_path,
                pipeline.json_text(corrupt_pointer).encode("utf-8"),
            )
            corrupt_pointer_bytes = pointer_path.read_bytes()
            rejected = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, rejected.returncode)
            self.assertEqual(corrupt_pointer_bytes, pointer_path.read_bytes())
            self.assertEqual(self.source_manifest, pipeline.read_json(output_path))

            pipeline._write_fsync(pointer_path, valid_pointer_bytes)
            target = (
                authoring_root
                / "revisions"
                / saved["revisionId"]
                / pipeline.GAMEPLAY_AUTHORING_REL
            )
            target.write_bytes(target.read_bytes() + b" ")
            rejected = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, rejected.returncode)
            self.assertEqual(valid_pointer_bytes, pointer_path.read_bytes())
            self.assertTrue(target.is_file())
            self.assertEqual(self.source_manifest, pipeline.read_json(output_path))

    def test_promoted_recovery_rolls_back_when_repository_source_changed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            repository_root = temporary_root / "repository"
            for row in self.source_manifest["files"]:
                source = self.root / row["path"]
                destination = repository_root / row["path"]
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, destination)
            local_source = pipeline.source_manifest(repository_root)
            patch = self.draft_patch()
            patch["sourceRevision"] = local_source["sourceManifestId"]
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(
                patch_path, pipeline.json_text(patch).encode("utf-8")
            )
            authoring_root = temporary_root / "authoring"
            pipeline_path = str(Path(pipeline.__file__).resolve())
            crashed = subprocess.run(
                [
                    sys.executable,
                    pipeline_path,
                    "--repository-root",
                    str(repository_root),
                    "save-authoring",
                    "--authoring-root",
                    str(authoring_root),
                    "--draft-patch",
                    str(patch_path),
                    "--crash-at",
                    "after_promote",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            journal = pipeline.read_json(authoring_root / ".save-journal.json")
            promoted_target = Path(journal["targetPath"])
            self.assertTrue(promoted_target.is_dir())
            self.assertFalse((authoring_root / "current-authoring.json").exists())

            changed_source_path = repository_root / pipeline.GAMEPLAY_AUTHORING_REL
            changed_source_path.write_bytes(changed_source_path.read_bytes() + b" ")
            changed_source = pipeline.source_manifest(repository_root)
            self.assertNotEqual(
                local_source["sourceManifestId"],
                changed_source["sourceManifestId"],
            )
            recovered = subprocess.run(
                [
                    sys.executable,
                    pipeline_path,
                    "--repository-root",
                    str(repository_root),
                    "source-manifest",
                    "--authoring-root",
                    str(authoring_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, recovered.returncode, recovered.stderr)
            result = json.loads(recovered.stdout)
            self.assertEqual(
                changed_source["sourceManifestId"], result["sourceRevision"]
            )
            self.assertIsNone(result["payload"]["authoringRevision"])
            self.assertFalse((authoring_root / "current-authoring.json").exists())
            self.assertFalse(promoted_target.exists())
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())

    def test_candidate_subprocess_hard_crash_recovers_staged_and_promoted(self) -> None:
        tracked = (
            pipeline.MASTER_REL,
            pipeline.ENCOUNTER_REL,
            pipeline.BINDINGS_REL,
            pipeline.CUES_REL,
            pipeline.COMBAT_PRODUCT_REL,
            pipeline.WORLD_PRODUCT_REL,
            pipeline.BOSS_PROFILES_REL,
            pipeline.DAMAGE_REL,
        )
        before = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            candidate_root = temporary_root / "candidates"
            initial = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
            )
            initial_pointer_bytes = (candidate_root / "current-candidate.json").read_bytes()
            draft_path = temporary_root / "draft.json"
            wrong_source_path = temporary_root / "wrong-source.json"
            pipeline._write_fsync(
                draft_path, pipeline.json_text(self.draft_patch()).encode("utf-8")
            )
            wrong_source = copy.deepcopy(self.source_manifest)
            wrong_source["sourceManifestId"] = "0" * 64
            pipeline._write_fsync(
                wrong_source_path, pipeline.json_text(wrong_source).encode("utf-8")
            )
            base_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "publish-candidate",
                "--candidate-root",
                str(candidate_root),
            ]

            staged_crash = subprocess.run(
                [
                    *base_command,
                    "--draft-patch",
                    str(draft_path),
                    "--crash-at",
                    "after_journal_staged",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, staged_crash.returncode)
            staged_journal = pipeline.read_json(candidate_root / ".publish-journal.json")
            staged_target = Path(staged_journal["targetPath"])
            self.assertEqual(initial_pointer_bytes, (candidate_root / "current-candidate.json").read_bytes())
            recovered = subprocess.run(
                [
                    *base_command,
                    "--expected-source-manifest",
                    str(wrong_source_path),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, recovered.returncode)
            self.assertEqual(initial_pointer_bytes, (candidate_root / "current-candidate.json").read_bytes())
            self.assertFalse(staged_target.exists())
            self.assertFalse((candidate_root / ".publish-journal.json").exists())
            self.assertFalse((candidate_root / ".publish.lock").exists())
            self.assertFalse(any(candidate_root.glob(".stage.*")))

            promoted_crash = subprocess.run(
                [
                    *base_command,
                    "--draft-patch",
                    str(draft_path),
                    "--crash-at",
                    "after_promote",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, promoted_crash.returncode)
            promoted_journal = pipeline.read_json(candidate_root / ".publish-journal.json")
            promoted_revision = promoted_journal["revisionId"]
            self.assertEqual(initial_pointer_bytes, (candidate_root / "current-candidate.json").read_bytes())
            recovered = subprocess.run(
                [
                    *base_command,
                    "--expected-source-manifest",
                    str(wrong_source_path),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, recovered.returncode)
            self.assertEqual(
                promoted_revision,
                pipeline.read_json(candidate_root / "current-candidate.json")["revisionId"],
            )
            self.assertTrue(Path(promoted_journal["targetPath"]).is_dir())
            self.assertFalse((candidate_root / ".publish-journal.json").exists())
            self.assertFalse((candidate_root / ".publish.lock").exists())
        after = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        self.assertEqual(before, after)

    def test_candidate_micro_crash_windows_recover_without_permanent_lockout(self) -> None:
        expectations = {
            "after_staged_journal_temp": "ROLLBACK",
            "after_promoted_journal_temp": "ROLLBACK",
            "after_journal_unlink": "COMMIT",
        }
        for crash_point, outcome in expectations.items():
            with self.subTest(crash_point=crash_point), tempfile.TemporaryDirectory() as temporary:
                temporary_root = Path(temporary)
                candidate_root = temporary_root / "candidates"
                patch_path = temporary_root / "draft.json"
                pipeline._write_fsync(
                    patch_path,
                    pipeline.json_text(self.draft_patch()).encode("utf-8"),
                )
                crashed = subprocess.run(
                    [
                        sys.executable,
                        str(Path(pipeline.__file__).resolve()),
                        "--repository-root",
                        str(self.root),
                        "publish-candidate",
                        "--candidate-root",
                        str(candidate_root),
                        "--draft-patch",
                        str(patch_path),
                        "--crash-at",
                        crash_point,
                    ],
                    capture_output=True,
                    text=True,
                    encoding="utf-8",
                    check=False,
                )
                self.assertEqual(
                    pipeline.HARD_CRASH_EXIT_CODE,
                    crashed.returncode,
                    crashed.stderr,
                )
                self.assertTrue((candidate_root / ".publish.lock").is_file())
                if crash_point == "after_journal_unlink":
                    committed_revision = pipeline.read_json(
                        candidate_root / "current-candidate.json"
                    )["revisionId"]
                    self.assertFalse(
                        (candidate_root / ".publish-journal.json").exists()
                    )
                pipeline._recover_durable_transaction(
                    self.root, candidate_root, "candidate"
                )
                if outcome == "COMMIT":
                    self.assertEqual(
                        committed_revision,
                        pipeline.read_json(
                            candidate_root / "current-candidate.json"
                        )["revisionId"],
                    )
                else:
                    self.assertFalse(
                        (candidate_root / "current-candidate.json").exists()
                    )
                    revisions = candidate_root / "revisions"
                    self.assertFalse(
                        revisions.exists() and any(revisions.iterdir())
                    )
                self.assertFalse((candidate_root / ".publish.lock").exists())
                self.assertFalse(
                    (candidate_root / ".publish-journal.json").exists()
                )
                self.assertFalse(any(candidate_root.glob(".stage.*")))
                self.assertFalse(
                    any(candidate_root.glob(".publish-journal.json.stage.*"))
                )

    def test_corrupt_or_escaping_durable_journal_fails_closed_without_cleanup(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(
                patch_path, pipeline.json_text(self.draft_patch()).encode("utf-8")
            )
            command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
                "--draft-patch",
                str(patch_path),
                "--crash-at",
                "after_journal_staged",
            ]
            crashed = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            journal_path = authoring_root / ".save-journal.json"
            lock_path = authoring_root / ".save.lock"
            journal = pipeline.read_json(journal_path)
            stage = Path(journal["stagePath"])
            journal["stagePath"] = str(temporary_root / "escape" / stage.name)
            journal = pipeline._seal_journal(journal)
            pipeline._write_fsync(
                journal_path, pipeline.json_text(journal).encode("utf-8")
            )
            with self.assertRaisesRegex(pipeline.PipelineError, "path escapes"):
                pipeline.save_authoring(
                    self.root, authoring_root, self.draft_patch()
                )
            self.assertTrue(lock_path.is_file())
            self.assertTrue(journal_path.is_file())
            self.assertTrue(stage.is_dir())

        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(
                patch_path, pipeline.json_text(self.draft_patch()).encode("utf-8")
            )
            command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
                "--draft-patch",
                str(patch_path),
                "--crash-at",
                "after_journal_staged",
            ]
            crashed = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            journal_path = authoring_root / ".save-journal.json"
            lock_path = authoring_root / ".save.lock"
            stage = Path(pipeline.read_json(journal_path)["stagePath"])
            journal_path.write_bytes(journal_path.read_bytes() + b" ")
            with self.assertRaisesRegex(pipeline.PipelineError, "not canonical"):
                pipeline.save_authoring(
                    self.root, authoring_root, self.draft_patch()
                )
            self.assertTrue(lock_path.is_file())
            self.assertTrue(journal_path.is_file())
            self.assertTrue(stage.is_dir())

    def test_revision_parent_reparse_is_rejected_for_authoring_and_candidate(self) -> None:
        for kind in ("authoring", "candidate"):
            with self.subTest(kind=kind), tempfile.TemporaryDirectory() as temporary:
                temporary_root = Path(temporary)
                transaction_root = temporary_root / kind
                outside = temporary_root / (kind + "-outside")
                transaction_root.mkdir()
                outside.mkdir()
                revisions = transaction_root / "revisions"
                self.create_directory_reparse(revisions, outside)
                try:
                    with self.assertRaisesRegex(
                        pipeline.PipelineError, "reparse point"
                    ):
                        if kind == "authoring":
                            pipeline.save_authoring(
                                self.root,
                                transaction_root,
                                self.draft_patch(),
                            )
                        else:
                            pipeline.publish_candidate(
                                self.root,
                                transaction_root,
                                expected_source_manifest=self.source_manifest,
                                draft_patch=self.draft_patch(),
                            )
                    self.assertEqual([], list(outside.iterdir()))
                    pointer_name = (
                        "current-authoring.json"
                        if kind == "authoring"
                        else "current-candidate.json"
                    )
                    self.assertFalse((transaction_root / pointer_name).exists())
                finally:
                    self.remove_directory_reparse(revisions)

    def test_cli_returns_structured_success_and_error_contract(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(patch_path, pipeline.json_text(self.draft_patch()).encode("utf-8"))
            command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "validate-draft",
                "--draft-patch",
                str(patch_path),
            ]
            completed = subprocess.run(
                command, capture_output=True, text=True, encoding="utf-8",
                errors="replace", check=False
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            result = json.loads(completed.stdout)
            self.assertEqual(
                {
                    "schema", "formatVersion", "command", "ok", "sourceRevision",
                    "candidateRevision", "errors", "payload",
                },
                set(result),
            )
            self.assertTrue(result["ok"])
            self.assertEqual([], result["errors"])
            self.assertIsNone(result["candidateRevision"])

            bad = self.draft_patch()
            bad["sourceRevision"] = "0" * 64
            pipeline._write_fsync(patch_path, pipeline.json_text(bad).encode("utf-8"))
            completed = subprocess.run(
                command, capture_output=True, text=True, encoding="utf-8",
                errors="replace", check=False
            )
            self.assertEqual(1, completed.returncode)
            result = json.loads(completed.stderr)
            self.assertFalse(result["ok"])
            self.assertEqual("SOURCE_REVISION_MISMATCH", result["errors"][0]["errorCode"])
            self.assertEqual(self.source_manifest["sourceManifestId"], result["sourceRevision"])
            self.assertIsNone(result["candidateRevision"])

            preview_path = temporary_root / "Valtan.pattern.v2.json"
            migrate_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "migrate-preview",
                "--output",
                str(preview_path),
            ]
            completed = subprocess.run(
                migrate_command, capture_output=True, text=True, encoding="utf-8",
                errors="replace", check=False
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            result = json.loads(completed.stdout)
            self.assertTrue(result["ok"])
            self.assertIsNone(result["candidateRevision"])
            self.assertEqual(2, pipeline.read_json(preview_path)["formatVersion"])

            source_before = pipeline.sha256_file(self.root / pipeline.MASTER_REL)
            migrate_command[-1] = str(self.root / pipeline.MASTER_REL)
            completed = subprocess.run(
                migrate_command, capture_output=True, text=True, encoding="utf-8",
                errors="replace", check=False
            )
            self.assertEqual(1, completed.returncode)
            result = json.loads(completed.stderr)
            self.assertEqual("SOURCE_OVERWRITE_FORBIDDEN", result["errors"][0]["errorCode"])
            self.assertEqual(source_before, pipeline.sha256_file(self.root / pipeline.MASTER_REL))

            pipeline._write_fsync(
                patch_path, pipeline.json_text(self.draft_patch()).encode("utf-8")
            )
            authoring_root = temporary_root / "authoring"
            save_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
                "--draft-patch",
                str(patch_path),
            ]
            completed = subprocess.run(
                save_command, capture_output=True, text=True, encoding="utf-8",
                errors="replace", check=False
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            result = json.loads(completed.stdout)
            self.assertTrue(result["ok"])
            self.assertIsNone(result["candidateRevision"])
            authoring_revision = result["payload"]["authoringRevision"]
            self.assertEqual(authoring_revision, result["payload"]["pointer"]["revisionId"])

            candidate_root = temporary_root / "candidates"
            publish_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "publish-candidate",
                "--candidate-root",
                str(candidate_root),
                "--authoring-root",
                str(authoring_root),
                "--authoring-revision",
                authoring_revision,
            ]
            completed = subprocess.run(
                publish_command, capture_output=True, text=True, encoding="utf-8",
                errors="replace", check=False
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            result = json.loads(completed.stdout)
            self.assertTrue(result["ok"])
            self.assertEqual(authoring_revision, result["sourceRevision"])
            self.assertEqual(authoring_revision, result["payload"]["authoringRevision"])
            self.assertEqual(
                result["candidateRevision"], result["payload"]["pointer"]["revisionId"]
            )
            self.assertEqual(
                "ENCOUNTER_RESET", result["payload"]["applyClass"]
            )

    def test_powershell_balance_tool_entrypoint_saves_and_publishes(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(
                patch_path, pipeline.json_text(self.draft_patch()).encode("utf-8")
            )
            wrapper = self.root / "Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1"
            authoring_root = temporary_root / "authoring"
            missing = subprocess.run(
                [
                    "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(wrapper), "-Mode", "ValidateDraft", "-RepositoryRoot", str(self.root),
                    "-AuthoringRoot", str(authoring_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, missing.returncode)
            self.assertEqual(
                "DRAFT_PATCH_REQUIRED", json.loads(missing.stderr)["errors"][0]["errorCode"]
            )
            completed = subprocess.run(
                [
                    "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(wrapper), "-Mode", "SaveAuthoring", "-RepositoryRoot", str(self.root),
                    "-AuthoringRoot", str(authoring_root), "-DraftPatchPath", str(patch_path),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            saved = json.loads(completed.stdout)
            authoring_revision = saved["payload"]["authoringRevision"]
            self.assertIsNone(saved["candidateRevision"])

            completed = subprocess.run(
                [
                    "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(wrapper), "-Mode", "SourceManifest", "-RepositoryRoot", str(self.root),
                    "-AuthoringRoot", str(authoring_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            resumed = json.loads(completed.stdout)
            self.assertEqual(authoring_revision, resumed["sourceRevision"])
            self.assertEqual(
                authoring_revision, resumed["payload"]["authoringRevision"]
            )

            candidate_root = temporary_root / "candidates"
            completed = subprocess.run(
                [
                    "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(wrapper), "-Mode", "PublishCandidate", "-RepositoryRoot", str(self.root),
                    "-AuthoringRoot", str(authoring_root), "-AuthoringRevision", authoring_revision,
                    "-CandidateRoot", str(candidate_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            published = json.loads(completed.stdout)
            self.assertEqual(authoring_revision, published["sourceRevision"])
            self.assertEqual(
                published["candidateRevision"],
                published["payload"]["pointer"]["revisionId"],
            )
            self.assertEqual(
                "ENCOUNTER_RESET", published["payload"]["applyClass"]
            )

    def test_failure_injection_preserves_sources_products_and_pointer(self) -> None:
        points = (
            "after_stage", "after_validate", "after_revision_manifest",
            "before_promote", "after_promote", "before_pointer", "after_pointer",
        )
        tracked = (
            pipeline.MASTER_REL,
            pipeline.COMBAT_AUTHORING_REL,
            pipeline.WORLD_SET_REL,
            pipeline.LEGACY_REL,
            pipeline.ENCOUNTER_REL,
            pipeline.BINDINGS_REL,
            pipeline.CUES_REL,
            pipeline.COMBAT_PRODUCT_REL,
            pipeline.WORLD_PRODUCT_REL,
            pipeline.BOSS_PROFILES_REL,
            pipeline.DAMAGE_REL,
        )
        before = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        for point in points:
            with self.subTest(point=point), tempfile.TemporaryDirectory() as temporary:
                candidate_root = Path(temporary) / "candidates"
                with self.assertRaises(pipeline.InjectedFailure):
                    pipeline.publish_candidate(
                        self.root,
                        candidate_root,
                        expected_source_manifest=self.source_manifest,
                        draft_patch=self.draft_patch(),
                        fail_at=point,
                    )
                self.assertFalse((candidate_root / "current-candidate.json").exists())
                self.assertFalse((candidate_root / ".publish-journal.json").exists())
                self.assertFalse((candidate_root / ".publish.lock").exists())
                revisions = candidate_root / "revisions"
                self.assertFalse(revisions.exists() and any(revisions.iterdir()))
                self.assertFalse(any(candidate_root.glob(".stage.*")))
                self.assertFalse(any(candidate_root.glob(".current-candidate.stage.*")))
        after = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        self.assertEqual(before, after)


class ValtanDynamicManualLineageContractTests(unittest.TestCase):
    def test_empty_manual_and_promotion_lineage_is_valid(self) -> None:
        master = {
            "bossArchetypeId": "BOSS_VALTAN",
            "encounterId": "ENCOUNTER_VALTAN",
            "patterns": [],
            "decisionModel": {"manualAuditions": []},
        }
        debug_presentation = {
            "schema": "lostark.valtan-pattern-presentation-debug",
            "formatVersion": 1,
            "bossArchetypeId": "BOSS_VALTAN",
            "encounterId": "ENCOUNTER_VALTAN",
            "chains": [],
        }
        promotion_manifest = {
            "schema": "lostark.valtan-animation-chain-promotions",
            "formatVersion": 2,
            "bossArchetypeId": "BOSS_VALTAN",
            "encounterId": "ENCOUNTER_VALTAN",
            "sourceDocument": "Data/Animation/Debug/Valtan.patternpresentation.debug.json",
            "presentationProfile": "BOSS_VALTAN",
            "clipAliases": [],
            "animationIntakeOnly": [],
            "patterns": [],
        }
        pipeline.validate_manual_audition_animation_lineage(
            master, debug_presentation, promotion_manifest
        )


def main() -> int:
    global REPOSITORY_ROOT
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    args, unittest_args = parser.parse_known_args()
    REPOSITORY_ROOT = args.repository_root.resolve()
    unittest.main(argv=[sys.argv[0], *unittest_args], verbosity=2, exit=False)
    result = unittest.TestProgram(argv=[sys.argv[0], *unittest_args], testRunner=unittest.TextTestRunner(verbosity=0), exit=False)
    return 0 if result.result.wasSuccessful() else 1


if __name__ == "__main__":
    # Build the suite once so CI receives a conventional non-zero exit code.
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    args, remaining = parser.parse_known_args()
    REPOSITORY_ROOT = args.repository_root.resolve()
    program = unittest.main(argv=[sys.argv[0], *remaining], verbosity=2, exit=False)
    raise SystemExit(0 if program.result.wasSuccessful() else 1)
