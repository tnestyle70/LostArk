#!/usr/bin/env python3
"""Focused contract for manual Pattern Stage topology authoring."""

from __future__ import annotations

import copy
import json
import pathlib
import sys
import unittest


sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline


ROOT = pathlib.Path(__file__).resolve().parents[2]


def find_pattern(master: dict, pattern_id: str) -> dict:
    return next(row for row in master["patterns"] if row["patternId"] == pattern_id)


def find_stage(pattern: dict, stage_id: str) -> dict:
    return next(row for row in pattern["stages"] if row["stageId"] == stage_id)


class ValtanManualStageTopologyPipelineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.documents = pipeline.load_pipeline_documents(ROOT)
        cls.source_revision = pipeline.source_manifest(ROOT)["sourceManifestId"]
        cls.master = pipeline.join_v2_authoring(
            cls.documents[pipeline.GAMEPLAY_AUTHORING_REL],
            cls.documents[pipeline.PRESENTATION_AUTHORING_REL],
            cls.documents[pipeline.WORLD_SET_REL],
            cls.documents[pipeline.COMBAT_AUTHORING_REL],
        )
        cls.debug_presentation = pipeline.read_json(
            ROOT / pipeline.DEBUG_PRESENTATION_REL
        )
        cls.promotion_manifest = pipeline.read_json(
            ROOT / pipeline.ANIMATION_PROMOTION_MANIFEST_REL
        )

    def apply(self, owner: dict, operations: list[dict]) -> dict:
        candidate, _, _, count = pipeline.apply_draft_patch(
            owner,
            copy.deepcopy(self.documents[pipeline.BOSS_PROFILES_REL]),
            copy.deepcopy(self.documents[pipeline.DAMAGE_REL]),
            {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": self.source_revision,
                "operations": operations,
            },
            self.source_revision,
            self.documents[pipeline.WORLD_SET_REL],
            self.documents[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(len(operations), count)
        return candidate

    def round_trip_and_validate_lineage(self, candidate: dict) -> dict:
        gameplay, presentation = pipeline.split_v2_authoring(
            candidate,
            self.documents[pipeline.WORLD_SET_REL],
            self.documents[pipeline.COMBAT_AUTHORING_REL],
        )
        rejoined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            self.documents[pipeline.WORLD_SET_REL],
            self.documents[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_manual_audition_animation_lineage(
            rejoined,
            self.debug_presentation,
            self.promotion_manifest,
        )
        return rejoined

    @staticmethod
    def wait_insert() -> dict:
        return {
            "op": "INSERT_MANUAL_STAGE_AFTER",
            "patternId": "VALTAN_SEQUENCE_FOUR",
            "afterStageId": "STEP_01",
            "stageId": "WAIT_01",
            "actionId": "valtan.sequence.four.wait-01",
            "stageRole": "WAIT",
            "durationMs": 750,
        }

    @staticmethod
    def counter_composition() -> list[dict]:
        return [
            {
                "op": "SET_STAGE_KIND",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "STEP_01",
                "stageKind": "WINDUP",
            },
            {
                "op": "INSERT_MANUAL_STAGE_AFTER",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "afterStageId": "STEP_01",
                "stageId": "GROGGY_01",
                "actionId": "valtan.sequence.four.groggy-01",
                "stageRole": "GROGGY",
                "durationMs": 1200,
            },
            {
                "op": "INSERT_MANUAL_STAGE_AFTER",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "afterStageId": "STEP_01",
                "stageId": "ACTIVE_02",
                "actionId": "valtan.sequence.four.active-02",
                "stageRole": "ACTIVE",
                "durationMs": 1000,
            },
            {
                "op": "SET_STAGE_COUNTER_WINDOW",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "STEP_01",
                "enabled": True,
                "successStageId": "GROGGY_01",
                "successActionId": "valtan.sequence.four.groggy-01",
                "timeoutStageId": "ACTIVE_02",
                "timeoutActionId": "valtan.sequence.four.active-02",
            },
        ]

    def test_wait_insert_is_runtime_active_none_and_projects(self) -> None:
        candidate = self.apply(self.master, [self.wait_insert()])
        pattern = find_pattern(candidate, "VALTAN_SEQUENCE_FOUR")
        self.assertEqual(["STEP_01", "WAIT_01"], [row["stageId"] for row in pattern["stages"]])
        wait = find_stage(pattern, "WAIT_01")
        self.assertEqual("WAIT", wait["sequenceRole"])
        self.assertEqual("ACTIVE", wait["stageKind"])
        self.assertEqual({"mode": "NONE"}, wait["animation"])
        self.assertEqual({"shape": {"kind": "NONE"}}, wait["hit"])
        self.assertIsNone(wait["motion"])
        for field in ("events", "branches", "effectCues", "cameraInvocations"):
            self.assertEqual([], wait[field])
        self.assertEqual(
            wait["actionId"], find_stage(pattern, "STEP_01")["defaultNextActionId"]
        )
        self.assertIsNone(wait["defaultNextActionId"])

        rejoined = self.round_trip_and_validate_lineage(candidate)
        outputs = pipeline.project_v2_products(ROOT, self.documents, rejoined)
        encounter = json.loads(outputs[pipeline.ENCOUNTER_REL])
        product = find_pattern(encounter, "VALTAN_SEQUENCE_FOUR")
        self.assertEqual("ACTIVE", find_stage(product, "WAIT_01")["stageKind"])
        bindings = json.loads(outputs[pipeline.BINDINGS_REL])
        binding = next(
            row
            for row in bindings["bindings"]
            if row["actionId"] == "valtan.sequence.four.wait-01"
        )
        self.assertEqual("NONE", binding["playbackMode"])
        self.assertEqual([], binding["clips"])

    def test_wait_promotion_preserves_identity_clock_and_round_trips(self) -> None:
        for role in ("ACTIVE", "WINDUP", "GROGGY"):
            with self.subTest(role=role):
                candidate = self.apply(
                    self.master,
                    [
                        self.wait_insert(),
                        {
                            "op": "PROMOTE_MANUAL_WAIT_STAGE",
                            "patternId": "VALTAN_SEQUENCE_FOUR",
                            "stageId": "WAIT_01",
                            "stageRole": role,
                        },
                    ],
                )
                promoted = find_stage(
                    find_pattern(candidate, "VALTAN_SEQUENCE_FOUR"), "WAIT_01"
                )
                self.assertEqual("WAIT_01", promoted["stageId"])
                self.assertEqual("valtan.sequence.four.wait-01", promoted["actionId"])
                self.assertEqual(750, promoted["durationMs"])
                self.assertEqual(role, promoted["sequenceRole"])
                self.assertEqual(role, promoted["stageKind"])
                self.assertEqual({"mode": "NONE"}, promoted["animation"])
                if role == "GROGGY":
                    self.assertEqual(
                        "CLOSED",
                        pipeline._stage_flag_contract(
                            promoted, "boss.flag.groggy"
                        ),
                    )
                else:
                    self.assertEqual([], promoted["events"])
                self.round_trip_and_validate_lineage(candidate)

    def test_wait_can_promote_then_receive_animation_in_one_patch(self) -> None:
        candidate = self.apply(
            self.master,
            [
                self.wait_insert(),
                {
                    "op": "PROMOTE_MANUAL_WAIT_STAGE",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "WAIT_01",
                    "stageRole": "ACTIVE",
                },
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "WAIT_01",
                    "animation": {
                        "endPolicy": "HOLD_LAST_POSE",
                        "repeatCount": 1,
                        "occurrences": [
                            {
                                "clipOccurrenceId": (
                                    "valtan.sequence.four.wait-01.clip-01"
                                ),
                                "clip": "mesh_att_battle_19_01",
                                "mappingBasis": "PROJECT_AUTHORED",
                                "sourceStartMs": 0,
                                "playMs": 600,
                                "playRate": 1.0,
                                "repeatUntilStageEnd": False,
                            }
                        ],
                    },
                },
            ],
        )
        promoted = find_stage(
            find_pattern(candidate, "VALTAN_SEQUENCE_FOUR"), "WAIT_01"
        )
        self.assertEqual("ACTIVE", promoted["sequenceRole"])
        self.assertEqual(
            "valtan.sequence.four.wait-01.clip-01",
            promoted["animation"]["occurrences"][0]["clipOccurrenceId"],
        )
        self.round_trip_and_validate_lineage(candidate)

    def test_wait_promotion_rejects_wrong_owner_and_role_atomically(self) -> None:
        for operation in (
            {
                "op": "PROMOTE_MANUAL_WAIT_STAGE",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "STEP_01",
                "stageRole": "ACTIVE",
            },
            {
                "op": "PROMOTE_MANUAL_WAIT_STAGE",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "WAIT_01",
                "stageRole": "WAIT",
            },
        ):
            baseline = (
                self.master
                if operation["stageId"] == "STEP_01"
                else self.apply(self.master, [self.wait_insert()])
            )
            before = copy.deepcopy(baseline)
            with self.subTest(operation=operation), self.assertRaises(
                pipeline.DraftPatchError
            ):
                self.apply(baseline, [operation])
            self.assertEqual(before, baseline)

    def test_groggy_insert_and_retag_are_independently_saveable(self) -> None:
        inserted = self.apply(
            self.master,
            [
                {
                    "op": "INSERT_MANUAL_STAGE_AFTER",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "afterStageId": "STEP_01",
                    "stageId": "GROGGY_01",
                    "actionId": "valtan.sequence.four.groggy-01",
                    "stageRole": "GROGGY",
                    "durationMs": 1200,
                }
            ],
        )
        inserted_stage = find_stage(
            find_pattern(inserted, "VALTAN_SEQUENCE_FOUR"), "GROGGY_01"
        )
        self.assertEqual("GROGGY", inserted_stage["stageKind"])
        self.assertEqual(
            "CLOSED",
            pipeline._stage_flag_contract(inserted_stage, "boss.flag.groggy"),
        )
        self.round_trip_and_validate_lineage(inserted)

        retagged = self.apply(
            self.master,
            [
                {
                    "op": "SET_STAGE_KIND",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "STEP_01",
                    "stageKind": "GROGGY",
                }
            ],
        )
        retagged_stage = find_stage(
            find_pattern(retagged, "VALTAN_SEQUENCE_FOUR"), "STEP_01"
        )
        self.assertEqual("GROGGY", retagged_stage["stageKind"])
        self.assertEqual(
            "CLOSED",
            pipeline._stage_flag_contract(retagged_stage, "boss.flag.groggy"),
        )
        self.round_trip_and_validate_lineage(retagged)

    def test_wait_rejects_every_non_clock_typed_mutation(self) -> None:
        with_wait = self.apply(self.master, [self.wait_insert()])
        operations = (
            {
                "op": "SET_STAGE_KIND",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "WAIT_01",
                "stageKind": "WINDUP",
            },
            {
                "op": "SET_STAGE_ANIMATION",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "WAIT_01",
                "animation": {
                    "endPolicy": "EXACT",
                    "repeatCount": 1,
                    "occurrences": [],
                },
            },
            {
                "op": "SET_STAGE_HIT",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "WAIT_01",
                "hit": {"shape": {"kind": "NONE"}},
            },
            {
                "op": "SET_STAGE_PORTAL_RUSH_MOTION",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "WAIT_01",
                "retargetDelayMs": 0,
                "speedMps": 10.0,
                "distanceM": 10.0,
            },
        )
        for operation in operations:
            before = copy.deepcopy(with_wait)
            with self.subTest(operation=operation), self.assertRaises(
                pipeline.DraftPatchError
            ) as raised:
                self.apply(before, [operation])
            self.assertEqual("WAIT_INVARIANT", raised.exception.error_code)
            self.assertEqual(with_wait, before)

        duration_only = self.apply(
            with_wait,
            [
                {
                    "op": "SET_STAGE_DURATION",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "WAIT_01",
                    "durationMs": 1250,
                }
            ],
        )
        self.assertEqual(
            1250,
            find_stage(
                find_pattern(duration_only, "VALTAN_SEQUENCE_FOUR"), "WAIT_01"
            )["durationMs"],
        )

    def test_final_master_validation_rejects_a_malformed_wait(self) -> None:
        candidate = self.apply(self.master, [self.wait_insert()])
        wait = find_stage(find_pattern(candidate, "VALTAN_SEQUENCE_FOUR"), "WAIT_01")
        wait["stageKind"] = "WINDUP"
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_v2_master(
                candidate,
                self.documents[pipeline.WORLD_SET_REL],
                self.documents[pipeline.COMBAT_AUTHORING_REL],
            )

    def test_one_stage_pattern_adds_groggy_then_counter_as_one_candidate(self) -> None:
        candidate = self.apply(self.master, self.counter_composition())
        pattern = find_pattern(candidate, "VALTAN_SEQUENCE_FOUR")
        source = find_stage(pattern, "STEP_01")
        target = find_stage(pattern, "GROGGY_01")
        self.assertEqual("WINDUP", source["stageKind"])
        self.assertEqual("GROGGY", target["stageKind"])
        self.assertEqual({"mode": "NONE"}, target["animation"])
        self.assertEqual(
            [
                {"outcome": "COUNTER_HIT", "nextActionId": target["actionId"]},
                {
                    "outcome": "TIMEOUT",
                    "nextActionId": "valtan.sequence.four.active-02",
                },
            ],
            source["branches"],
        )
        self.assertEqual("CLOSED", pipeline._stage_flag_contract(source, "boss.flag.counterable"))
        self.assertEqual("CLOSED", pipeline._stage_flag_contract(target, "boss.flag.groggy"))
        self.round_trip_and_validate_lineage(candidate)
        pipeline.project_v2_products(ROOT, self.documents, candidate)

    def test_added_stage_accepts_typed_slots_without_rewriting_intake(self) -> None:
        candidate = self.apply(
            self.master,
            [
                {
                    "op": "INSERT_MANUAL_STAGE_AFTER",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "afterStageId": "STEP_01",
                    "stageId": "ACTIVE_02",
                    "actionId": "valtan.sequence.four.active-02",
                    "stageRole": "ACTIVE",
                    "durationMs": 1000,
                },
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "ACTIVE_02",
                    "animation": {
                        "endPolicy": "HOLD_LAST_POSE",
                        "repeatCount": 1,
                        "occurrences": [
                            {
                                "clipOccurrenceId": "valtan.sequence.four.active-02.clip-01",
                                "clip": "mesh_att_battle_19_01",
                                "mappingBasis": "PROJECT_AUTHORED",
                                "sourceStartMs": 0,
                                "playMs": 600,
                                "playRate": 1.0,
                                "repeatUntilStageEnd": False,
                            }
                        ],
                    },
                },
            ],
        )
        added = find_stage(find_pattern(candidate, "VALTAN_SEQUENCE_FOUR"), "ACTIVE_02")
        self.assertEqual(
            "SOURCE_REVIEWED_DELTA",
            added["animation"]["occurrences"][0]["mappingBasis"],
        )
        rejoined = self.round_trip_and_validate_lineage(candidate)
        outputs = pipeline.project_v2_products(ROOT, self.documents, rejoined)
        bindings = json.loads(outputs[pipeline.BINDINGS_REL])
        binding = next(
            row
            for row in bindings["bindings"]
            if row["actionId"] == "valtan.sequence.four.active-02"
        )
        self.assertEqual(["mesh_att_battle_19_01"], [row["clip"] for row in binding["clips"]])

    def test_original_manual_step_can_remove_last_slot_to_none_and_reassign(self) -> None:
        pattern_id = "VALTAN_ROAR_CHARGE"
        stage_id = "STEP_02"
        source_stage = find_stage(find_pattern(self.master, pattern_id), stage_id)
        source_animation = copy.deepcopy(source_stage["animation"])
        none_operation = {
            "op": "SET_STAGE_ANIMATION",
            "patternId": pattern_id,
            "stageId": stage_id,
            "animation": {"mode": "NONE"},
        }
        without_animation = self.apply(self.master, [none_operation])
        pattern = find_pattern(without_animation, pattern_id)
        stage = find_stage(pattern, stage_id)
        self.assertEqual({"mode": "NONE"}, stage["animation"])
        self.assertEqual(source_stage["durationMs"], stage["durationMs"])
        self.assertEqual(source_stage["actionId"], stage["actionId"])

        rejoined = self.round_trip_and_validate_lineage(without_animation)
        outputs = pipeline.project_v2_products(ROOT, self.documents, rejoined)
        bindings = json.loads(outputs[pipeline.BINDINGS_REL])
        binding = next(
            row
            for row in bindings["bindings"]
            if row["actionId"] == source_stage["actionId"]
        )
        self.assertEqual("NONE", binding["playbackMode"])
        self.assertEqual([], binding["clips"])

        reassigned = self.apply(
            without_animation,
            [
                {
                    **none_operation,
                    "animation": source_animation,
                }
            ],
        )
        reassigned_stage = find_stage(
            find_pattern(reassigned, pattern_id), stage_id
        )
        self.assertEqual(
            [row["clip"] for row in source_animation["occurrences"]],
            [row["clip"] for row in reassigned_stage["animation"]["occurrences"]],
        )
        self.assertEqual(
            "SOURCE_REVIEWED_DELTA",
            reassigned_stage["animation"]["occurrences"][0]["mappingBasis"],
        )
        self.round_trip_and_validate_lineage(reassigned)

    def test_animation_none_union_is_manual_non_wait_only_and_fail_closed(self) -> None:
        with_wait = self.apply(self.master, [self.wait_insert()])
        cases = (
            (
                self.master,
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_HIGH_JUMP",
                    "stageId": "TAKEOFF",
                    "animation": {"mode": "NONE"},
                },
                "FIELD_NOT_ALLOWED",
            ),
            (
                with_wait,
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "WAIT_01",
                    "animation": {"mode": "NONE"},
                },
                "WAIT_INVARIANT",
            ),
            (
                self.master,
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "STEP_01",
                    "animation": {"mode": "NONE", "occurrences": []},
                },
                None,
            ),
        )
        for owner, operation, expected_code in cases:
            before = copy.deepcopy(owner)
            with self.subTest(operation=operation), self.assertRaises(
                (pipeline.DraftPatchError, pipeline.PipelineError)
            ) as raised:
                self.apply(before, [operation])
            if expected_code is not None:
                self.assertEqual(expected_code, raised.exception.error_code)
            self.assertEqual(owner, before)

    def test_move_preserves_ids_and_rebuilds_only_the_default_path(self) -> None:
        original = find_pattern(self.master, "VALTAN_CHARGE")
        identities = {
            (stage["stageId"], stage["actionId"]) for stage in original["stages"]
        }
        candidate = self.apply(
            self.master,
            [
                {
                    "op": "MOVE_MANUAL_STAGE",
                    "patternId": "VALTAN_CHARGE",
                    "stageId": "STEP_03",
                    "anchorStageId": "STEP_01",
                    "placement": "BEFORE",
                }
            ],
        )
        pattern = find_pattern(candidate, "VALTAN_CHARGE")
        self.assertEqual(
            ["STEP_03", "STEP_01", "STEP_02"],
            [stage["stageId"] for stage in pattern["stages"]],
        )
        self.assertEqual(find_stage(pattern, "STEP_03")["actionId"], pattern["entryActionId"])
        self.assertEqual(
            [
                find_stage(pattern, "STEP_01")["actionId"],
                find_stage(pattern, "STEP_02")["actionId"],
                None,
            ],
            [stage["defaultNextActionId"] for stage in pattern["stages"]],
        )
        self.assertEqual(
            identities,
            {(stage["stageId"], stage["actionId"]) for stage in pattern["stages"]},
        )
        self.round_trip_and_validate_lineage(candidate)

    def test_backward_counter_target_and_reorder_are_fail_closed(self) -> None:
        linked = self.apply(self.master, self.counter_composition())
        before_move = copy.deepcopy(linked)
        with self.assertRaises(pipeline.DraftPatchError) as raised:
            self.apply(
                before_move,
                [
                    {
                        "op": "MOVE_MANUAL_STAGE",
                        "patternId": "VALTAN_SEQUENCE_FOUR",
                        "stageId": "GROGGY_01",
                        "anchorStageId": "STEP_01",
                        "placement": "BEFORE",
                    }
                ],
            )
        self.assertEqual("CANDIDATE_VALIDATION_FAILED", raised.exception.error_code)
        self.assertIn("finite stage graph contains a cycle", str(raised.exception))
        self.assertEqual(linked, before_move)

        backward_candidate = self.apply(
            self.master,
            [
                {
                    "op": "INSERT_MANUAL_STAGE_AFTER",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "afterStageId": "STEP_01",
                    "stageId": "GROGGY_01",
                    "actionId": "valtan.sequence.four.groggy-01",
                    "stageRole": "GROGGY",
                    "durationMs": 1200,
                },
                {
                    "op": "INSERT_MANUAL_STAGE_AFTER",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "afterStageId": "GROGGY_01",
                    "stageId": "WINDUP_01",
                    "actionId": "valtan.sequence.four.windup-01",
                    "stageRole": "WINDUP",
                    "durationMs": 900,
                },
                {
                    "op": "INSERT_MANUAL_STAGE_AFTER",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "afterStageId": "WINDUP_01",
                    "stageId": "ACTIVE_02",
                    "actionId": "valtan.sequence.four.active-02",
                    "stageRole": "ACTIVE",
                    "durationMs": 1000,
                },
            ],
        )
        before_counter = copy.deepcopy(backward_candidate)
        with self.assertRaises(pipeline.DraftPatchError) as raised:
            self.apply(
                before_counter,
                [
                    {
                        "op": "SET_STAGE_COUNTER_WINDOW",
                        "patternId": "VALTAN_SEQUENCE_FOUR",
                        "stageId": "WINDUP_01",
                        "enabled": True,
                        "successStageId": "GROGGY_01",
                        "successActionId": "valtan.sequence.four.groggy-01",
                        "timeoutStageId": "ACTIVE_02",
                        "timeoutActionId": "valtan.sequence.four.active-02",
                    }
                ],
            )
        self.assertEqual("COUNTER_TARGET_NOT_FORWARD", raised.exception.error_code)
        self.assertIn(
            "counter success and timeout targets must be later same-pattern stages",
            str(raised.exception),
        )
        self.assertEqual(backward_candidate, before_counter)

    def test_counter_retarget_precedes_old_target_removal(self) -> None:
        linked = self.apply(self.master, self.counter_composition())
        insert_new_target = {
            "op": "INSERT_MANUAL_STAGE_AFTER",
            "patternId": "VALTAN_SEQUENCE_FOUR",
            "afterStageId": "GROGGY_01",
            "stageId": "GROGGY_02",
            "actionId": "valtan.sequence.four.groggy-02",
            "stageRole": "GROGGY",
            "durationMs": 1000,
        }
        retarget = {
            "op": "SET_STAGE_COUNTER_WINDOW",
            "patternId": "VALTAN_SEQUENCE_FOUR",
            "stageId": "STEP_01",
            "enabled": True,
            "successStageId": "GROGGY_02",
            "successActionId": "valtan.sequence.four.groggy-02",
            "timeoutStageId": "ACTIVE_02",
            "timeoutActionId": "valtan.sequence.four.active-02",
        }
        remove_old_target = {
            "op": "REMOVE_MANUAL_STAGE",
            "patternId": "VALTAN_SEQUENCE_FOUR",
            "stageId": "GROGGY_01",
        }
        candidate = self.apply(
            linked, [insert_new_target, retarget, remove_old_target]
        )
        pattern = find_pattern(candidate, "VALTAN_SEQUENCE_FOUR")
        self.assertEqual(
            ["STEP_01", "ACTIVE_02", "GROGGY_02"],
            [stage["stageId"] for stage in pattern["stages"]],
        )
        self.round_trip_and_validate_lineage(candidate)

        wrong_order = copy.deepcopy(linked)
        with self.assertRaises(pipeline.DraftPatchError) as raised:
            self.apply(
                wrong_order,
                [insert_new_target, remove_old_target, retarget],
            )
        self.assertEqual("COUNTER_TARGET_DANGLING", raised.exception.error_code)
        self.assertEqual(linked, wrong_order)

    def test_shared_capture_fragment_topology_is_rejected_before_mutation(self) -> None:
        before = copy.deepcopy(self.master)
        with self.assertRaises(pipeline.DraftPatchError) as raised:
            self.apply(
                before,
                [
                    {
                        "op": "INSERT_MANUAL_STAGE_AFTER",
                        "patternId": "VALTAN_TRASH_CATCH_FAIL",
                        "afterStageId": "RUSH_MISS",
                        "stageId": "WAIT_01",
                        "actionId": "valtan.sequence.rush-fail.wait-01",
                        "stageRole": "WAIT",
                        "durationMs": 500,
                    }
                ],
            )
        self.assertEqual(
            "MANUAL_TOPOLOGY_SHARED_FRAGMENT", raised.exception.error_code
        )
        self.assertEqual(self.master, before)

    def test_remove_is_safe_and_counter_dependencies_are_fail_closed(self) -> None:
        with_wait = self.apply(self.master, [self.wait_insert()])
        restored = self.apply(
            with_wait,
            [
                {
                    "op": "REMOVE_MANUAL_STAGE",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "WAIT_01",
                }
            ],
        )
        self.assertEqual(
            ["STEP_01"],
            [
                row["stageId"]
                for row in find_pattern(restored, "VALTAN_SEQUENCE_FOUR")["stages"]
            ],
        )

        linked = self.apply(self.master, self.counter_composition())
        for stage_id, error_code in (
            ("STEP_01", "COUNTER_SOURCE_DANGLING"),
            ("GROGGY_01", "COUNTER_TARGET_DANGLING"),
        ):
            before = copy.deepcopy(linked)
            with self.subTest(stage_id=stage_id), self.assertRaises(
                pipeline.DraftPatchError
            ) as raised:
                self.apply(
                    before,
                    [
                        {
                            "op": "REMOVE_MANUAL_STAGE",
                            "patternId": "VALTAN_SEQUENCE_FOUR",
                            "stageId": stage_id,
                        }
                    ],
                )
            self.assertEqual(error_code, raised.exception.error_code)
            self.assertEqual(linked, before)

        disabled = self.apply(
            linked,
            [
                {
                    "op": "SET_STAGE_COUNTER_WINDOW",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "STEP_01",
                    "enabled": False,
                    "successStageId": "GROGGY_01",
                    "successActionId": "valtan.sequence.four.groggy-01",
                    "timeoutStageId": "ACTIVE_02",
                    "timeoutActionId": "valtan.sequence.four.active-02",
                }
            ],
        )
        self.round_trip_and_validate_lineage(disabled)
        disabled_and_removed = self.apply(
            disabled,
            [
                {
                    "op": "REMOVE_MANUAL_STAGE",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "GROGGY_01",
                },
            ],
        )
        self.assertEqual(
            ["STEP_01", "ACTIVE_02"],
            [
                row["stageId"]
                for row in find_pattern(disabled_and_removed, "VALTAN_SEQUENCE_FOUR")["stages"]
            ],
        )

        # This is the exact operation order emitted by the C++ draft builder:
        # disable the loaded edge first, then remove the now dependency-free
        # target in one parse/validate/stage/commit candidate.
        atomic_disabled_and_removed = self.apply(
            linked,
            [
                {
                    "op": "SET_STAGE_COUNTER_WINDOW",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "STEP_01",
                    "enabled": False,
                    "successStageId": "GROGGY_01",
                    "successActionId": "valtan.sequence.four.groggy-01",
                    "timeoutStageId": "ACTIVE_02",
                    "timeoutActionId": "valtan.sequence.four.active-02",
                },
                {
                    "op": "REMOVE_MANUAL_STAGE",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "GROGGY_01",
                },
            ],
        )
        self.assertEqual(
            ["STEP_01", "ACTIVE_02"],
            [
                row["stageId"]
                for row in find_pattern(
                    atomic_disabled_and_removed, "VALTAN_SEQUENCE_FOUR"
                )["stages"]
            ],
        )
        self.round_trip_and_validate_lineage(atomic_disabled_and_removed)

    def test_counter_disable_then_groggy_retag_cleans_owned_flag(self) -> None:
        linked = self.apply(self.master, self.counter_composition())
        retagged = self.apply(
            linked,
            [
                {
                    "op": "SET_STAGE_COUNTER_WINDOW",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "STEP_01",
                    "enabled": False,
                    "successStageId": "GROGGY_01",
                    "successActionId": "valtan.sequence.four.groggy-01",
                    "timeoutStageId": "ACTIVE_02",
                    "timeoutActionId": "valtan.sequence.four.active-02",
                },
                {
                    "op": "SET_STAGE_KIND",
                    "patternId": "VALTAN_SEQUENCE_FOUR",
                    "stageId": "GROGGY_01",
                    "stageKind": "ACTIVE",
                },
            ],
        )
        pattern = find_pattern(retagged, "VALTAN_SEQUENCE_FOUR")
        target = find_stage(pattern, "GROGGY_01")
        self.assertEqual("ACTIVE", target["stageKind"])
        self.assertEqual(
            "ABSENT", pipeline._stage_flag_contract(target, "boss.flag.groggy")
        )
        self.round_trip_and_validate_lineage(retagged)

    def test_invalid_or_canonical_topology_operations_preserve_input(self) -> None:
        cases = (
            {
                **self.wait_insert(),
                "patternId": "VALTAN_HIGH_JUMP",
                "afterStageId": "AIRBORNE",
                "actionId": "valtan.high-jump.wait-01",
            },
            {**self.wait_insert(), "stageRole": "RECOVERY"},
            {**self.wait_insert(), "stageId": "STEP_01"},
            {**self.wait_insert(), "actionId": "valtan.sequence.four.step-01"},
            {
                "op": "MOVE_MANUAL_STAGE",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_03",
                "anchorStageId": "STEP_01",
                "placement": "AROUND",
            },
            {
                "op": "MOVE_MANUAL_STAGE",
                "patternId": "VALTAN_TRASH",
                "stageId": "STEP_02",
                "anchorStageId": "STEP_01",
                "placement": "BEFORE",
            },
        )
        for operation in cases:
            before = copy.deepcopy(self.master)
            with self.subTest(operation=operation), self.assertRaises(
                pipeline.DraftPatchError
            ):
                self.apply(before, [operation])
            self.assertEqual(self.master, before)

        before = copy.deepcopy(self.master)
        with self.assertRaises(pipeline.DraftPatchError):
            self.apply(
                before,
                [
                    self.wait_insert(),
                    {
                        "op": "MOVE_MANUAL_STAGE",
                        "patternId": "VALTAN_SEQUENCE_FOUR",
                        "stageId": "WAIT_01",
                        "anchorStageId": "STEP_01",
                        "placement": "INVALID",
                    },
                ],
            )
        self.assertEqual(self.master, before)

    def test_original_intake_stage_cannot_be_removed(self) -> None:
        before = copy.deepcopy(self.master)
        with self.assertRaises(pipeline.DraftPatchError) as raised:
            self.apply(
                before,
                [
                    {
                        "op": "REMOVE_MANUAL_STAGE",
                        "patternId": "VALTAN_CHARGE",
                        "stageId": "STEP_02",
                    }
                ],
            )
        self.assertEqual("SOURCE_STAGE_IMMUTABLE", raised.exception.error_code)
        self.assertEqual(self.master, before)


if __name__ == "__main__":
    unittest.main()
