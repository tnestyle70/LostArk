#!/usr/bin/env python3
"""Focused contract for promoted manual Pattern Stage/Sequence authoring."""

from __future__ import annotations

import copy
import json
import pathlib
import sys
import unittest


sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline


ROOT = pathlib.Path(__file__).resolve().parents[2]
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
WORKBENCH_CPP = ROOT / "Client/Private/ActionCompositionWorkbench.cpp"


def pattern(master: dict, pattern_id: str) -> dict:
    return next(row for row in master["patterns"] if row["patternId"] == pattern_id)


def stage(owner: dict, stage_id: str) -> dict:
    return next(row for row in owner["stages"] if row["stageId"] == stage_id)


class ValtanManualStageAuthoringContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.docs = pipeline.load_pipeline_documents(ROOT)
        cls.source_revision = pipeline.source_manifest(ROOT)["sourceManifestId"]
        cls.master = pipeline.join_v2_authoring(
            cls.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            cls.docs[pipeline.PRESENTATION_AUTHORING_REL],
            cls.docs[pipeline.WORLD_SET_REL],
            cls.docs[pipeline.COMBAT_AUTHORING_REL],
        )

    def patch(self, operations: list[dict], owner: dict | None = None) -> dict:
        candidate, _, _, count = pipeline.apply_draft_patch(
            self.master if owner is None else owner,
            copy.deepcopy(self.docs[pipeline.BOSS_PROFILES_REL]),
            copy.deepcopy(self.docs[pipeline.DAMAGE_REL]),
            {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": self.source_revision,
                "operations": operations,
            },
            self.source_revision,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
            repository_root=ROOT,
        )
        self.assertEqual(len(operations), count)
        return candidate

    @staticmethod
    def sequence_420617_animation(*, invalid_clip: bool = False) -> dict:
        clips = [
            ("mesh_att_battle_17_start", 2000),
            ("mesh_att_battle_17_loop", 1000),
            ("mesh_att_battle_17_end", 3000),
            ("mesh_att_battle_17_loop", 1000),
            ("mesh_att_battle_17_end", 3000),
        ]
        if invalid_clip:
            clips[1] = ("mesh_att_battle_19_01", 1000)
        return {
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [
                {
                    "clipOccurrenceId": (
                        f"valtan.sequence.four.step-01.clip-{index:02d}"
                    ),
                    "clip": clip,
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": play_ms,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                }
                for index, (clip, play_ms) in enumerate(clips, 1)
            ],
        }

    def sequence_source_operations(self, *, invalid_clip: bool = False) -> list[dict]:
        return [
            {
                "op": "ADD_PATTERN_SEQUENCE_SOURCE",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "sourceActionId": 420617,
                "sequenceIndex": 1,
                "role": "REFERENCE_420617_1",
            },
            {
                "op": "SET_STAGE_DURATION",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "STEP_01",
                "durationMs": 10000,
            },
            {
                "op": "SET_STAGE_ANIMATION",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "STEP_01",
                "animation": self.sequence_420617_animation(
                    invalid_clip=invalid_clip
                ),
            },
        ]

    def assert_provenance_rejected(
        self,
        operations: list[dict],
        message: str = "exact ordered concatenation",
    ) -> None:
        before = copy.deepcopy(self.master)
        with self.assertRaisesRegex(
            pipeline.DraftPatchError, message
        ) as raised:
            self.patch(operations, before)
        self.assertEqual("SOURCE_PROVENANCE_MISMATCH", raised.exception.error_code)
        self.assertEqual(self.master, before)

    def test_sequence_source_append_adds_exact_ordered_provenance(self) -> None:
        candidate = self.patch(self.sequence_source_operations())
        authored = pattern(candidate, "VALTAN_SEQUENCE_FOUR")
        self.assertEqual([420624, 420617], authored["sourceActionIds"])
        self.assertEqual(
            {
                "sourceActionId": 420617,
                "sequenceIndex": 1,
                "role": "REFERENCE_420617_1",
            },
            authored["presentationSources"][-1],
        )
        self.assertEqual(
            [
                "mesh_att_battle_17_start",
                "mesh_att_battle_17_loop",
                "mesh_att_battle_17_end",
                "mesh_att_battle_17_loop",
                "mesh_att_battle_17_end",
            ],
            [
                row["clip"]
                for row in stage(authored, "STEP_01")["animation"]["occurrences"]
            ],
        )

        self.assert_provenance_rejected(
            self.sequence_source_operations(invalid_clip=True)
        )

    def test_ground_roar_delta_primary_accepts_exact_magic_emission_append(self) -> None:
        original = pattern(self.master, "VALTAN_GROUND_ROAR")
        original_occurrences = copy.deepcopy(
            stage(original, "STEP_01")["animation"]["occurrences"]
        )
        candidate = self.patch(
            [
                {
                    "op": "ADD_PATTERN_SEQUENCE_SOURCE",
                    "patternId": "VALTAN_GROUND_ROAR",
                    "sourceActionId": 400437,
                    "sequenceIndex": 0,
                    "role": "REFERENCE_400437_0",
                },
                {
                    "op": "SET_STAGE_DURATION",
                    "patternId": "VALTAN_GROUND_ROAR",
                    "stageId": "STEP_01",
                    "durationMs": 9091,
                },
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_GROUND_ROAR",
                    "stageId": "STEP_01",
                    "animation": {
                        "endPolicy": "HOLD_LAST_POSE",
                        "repeatCount": 1,
                        "occurrences": [
                            *original_occurrences,
                            {
                                "clipOccurrenceId": (
                                    "valtan.sequence.sequence.400440.0.step-01."
                                    "composition.clip.01"
                                ),
                                "clip": "mesh_evt1_att_battle_5_01_end",
                                "mappingBasis": "PROJECT_AUTHORED",
                                "sourceStartMs": 0,
                                "playMs": 2633,
                                "playRate": 1.0,
                                "repeatUntilStageEnd": False,
                            },
                        ],
                    },
                },
            ]
        )
        authored = pattern(candidate, "VALTAN_GROUND_ROAR")
        authored_stage = stage(authored, "STEP_01")
        self.assertEqual(9091, authored_stage["durationMs"])
        self.assertEqual([400440, 400425, 400437], authored["sourceActionIds"])
        self.assertEqual(
            [
                "mesh_att_battle_11_01",
                "mesh_att_battle_5_01_end",
                "mesh_evt1_att_battle_5_01_end",
            ],
            [row["clip"] for row in authored_stage["animation"]["occurrences"]],
        )
        self.assertTrue(
            all(
                row["mappingBasis"] == "SOURCE_REVIEWED_DELTA"
                for row in authored_stage["animation"]["occurrences"]
            )
        )

        gameplay, presentation = pipeline.split_v2_authoring(
            candidate,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        rejoined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_manual_audition_animation_lineage(
            rejoined,
            pipeline.read_json(ROOT / pipeline.DEBUG_PRESENTATION_REL),
            pipeline.read_json(ROOT / pipeline.ANIMATION_PROMOTION_MANIFEST_REL),
            repository_root=ROOT,
        )

    def test_sequence_source_rejects_reordered_occurrences(self) -> None:
        operations = self.sequence_source_operations()
        occurrences = operations[2]["animation"]["occurrences"]
        occurrences[1], occurrences[2] = occurrences[2], occurrences[1]
        self.assert_provenance_rejected(operations)

    def test_sequence_source_rejects_duplicate_removal(self) -> None:
        operations = self.sequence_source_operations()
        del operations[2]["animation"]["occurrences"][3]
        self.assert_provenance_rejected(operations)

    def test_sequence_source_rejects_set_union_assembly(self) -> None:
        operations = self.sequence_source_operations()
        operations.insert(
            1,
            {
                "op": "ADD_PATTERN_SEQUENCE_SOURCE",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "sourceActionId": 400440,
                "sequenceIndex": 0,
                "role": "REFERENCE_400440_0",
            },
        )
        operations[2]["durationMs"] = 11800
        occurrences = operations[3]["animation"]["occurrences"]
        occurrences.insert(
            1,
            {
                "clipOccurrenceId": "",
                "clip": "mesh_att_battle_11_01",
                "mappingBasis": "PROJECT_AUTHORED",
                "sourceStartMs": 0,
                "playMs": 1800,
                "playRate": 1.0,
                "repeatUntilStageEnd": False,
            },
        )
        for index, occurrence in enumerate(occurrences, 1):
            occurrence["clipOccurrenceId"] = (
                f"valtan.sequence.four.step-01.clip-{index:02d}"
            )
        self.assert_provenance_rejected(operations)

    def test_sequence_source_rejects_primary_role(self) -> None:
        operation = self.sequence_source_operations()[0]
        operation["role"] = "PRIMARY"
        before = copy.deepcopy(self.master)
        with self.assertRaisesRegex(
            pipeline.DraftPatchError, "deterministic exact-tuple role"
        ) as raised:
            self.patch([operation], before)
        self.assertEqual("FIELD_NOT_ALLOWED", raised.exception.error_code)
        self.assertEqual(self.master, before)

    def test_sequence_source_rejects_unused_provenance(self) -> None:
        self.assert_provenance_rejected([self.sequence_source_operations()[0]])

    def test_sequence_source_accepts_an_exact_reused_occurrence_identity(self) -> None:
        owner = copy.deepcopy(self.master)
        source_animation = self.sequence_420617_animation()
        owner_stage = stage(pattern(owner, "VALTAN_SEQUENCE_FOUR"), "STEP_01")
        # The first logical slot already contains the exact first source clip.
        # Replace is allowed to preserve that stable row while the rest of the
        # selected Sequence is materialized with new identities.
        owner_stage["animation"]["occurrences"] = [
            copy.deepcopy(source_animation["occurrences"][0])
        ]
        candidate = self.patch(self.sequence_source_operations(), owner)
        candidate_stage = stage(
            pattern(candidate, "VALTAN_SEQUENCE_FOUR"), "STEP_01"
        )
        self.assertEqual(
            [row["clip"] for row in source_animation["occurrences"]],
            [
                row["clip"]
                for row in candidate_stage["animation"]["occurrences"]
            ],
        )
        self.assertEqual(
            source_animation["occurrences"][0]["clipOccurrenceId"],
            candidate_stage["animation"]["occurrences"][0][
                "clipOccurrenceId"
            ],
        )

    def test_sequence_source_accepts_finite_materialized_hold_loops(self) -> None:
        clips = [
            "mesh_att_battle_5_01_start",
            "mesh_att_battle_5_01_loop",
            "mesh_att_battle_5_01_loop",
            "mesh_att_battle_5_01_end",
        ]
        operations = [
            {
                "op": "ADD_PATTERN_SEQUENCE_SOURCE",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "sourceActionId": 400425,
                "sequenceIndex": 0,
                "role": "REFERENCE_400425_0",
            },
            {
                "op": "SET_STAGE_DURATION",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "STEP_01",
                "durationMs": 5000,
            },
            {
                "op": "SET_STAGE_ANIMATION",
                "patternId": "VALTAN_SEQUENCE_FOUR",
                "stageId": "STEP_01",
                "animation": {
                    "endPolicy": "EXACT",
                    "repeatCount": 1,
                    "occurrences": [
                        {
                            "clipOccurrenceId": (
                                "valtan.sequence.four.step-01.clip-01"
                                if index == 1
                                else f"valtan.sequence.four.step-01.hold-{index:02d}"
                            ),
                            "clip": clip,
                            "mappingBasis": "PROJECT_AUTHORED",
                            "sourceStartMs": 0,
                            "playMs": 1250,
                            "playRate": 1.0,
                            "repeatUntilStageEnd": False,
                        }
                        for index, clip in enumerate(clips, 1)
                    ],
                },
            },
        ]
        candidate = self.patch(operations)
        candidate_stage = stage(
            pattern(candidate, "VALTAN_SEQUENCE_FOUR"), "STEP_01"
        )
        self.assertEqual(
            clips,
            [
                row["clip"]
                for row in candidate_stage["animation"]["occurrences"]
            ],
        )

    def test_sequence_source_accepts_an_end_only_ordered_slice(self) -> None:
        owner = copy.deepcopy(self.master)
        original = pattern(owner, "VALTAN_GROUND_ROAR")
        original["sourceActionIds"] = [400440]
        original["presentationSources"] = original["presentationSources"][:1]
        original_stage = stage(original, "STEP_01")
        stomp = copy.deepcopy(original_stage["animation"]["occurrences"][0])
        original_stage["durationMs"] = 1800
        original_stage["animation"] = {
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [stomp],
        }
        candidate = self.patch(
            [
                {
                    "op": "ADD_PATTERN_SEQUENCE_SOURCE",
                    "patternId": "VALTAN_GROUND_ROAR",
                    "sourceActionId": 400425,
                    "sequenceIndex": 0,
                    "role": "REFERENCE_400425_0",
                },
                {
                    "op": "SET_STAGE_DURATION",
                    "patternId": "VALTAN_GROUND_ROAR",
                    "stageId": "STEP_01",
                    "durationMs": 6458,
                },
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_GROUND_ROAR",
                    "stageId": "STEP_01",
                    "animation": {
                        "endPolicy": "HOLD_LAST_POSE",
                        "repeatCount": 1,
                        "occurrences": [
                            stomp,
                            {
                                "clipOccurrenceId": (
                                    "valtan.sequence.sequence.400440.0.step-01."
                                    "roar.clip-05"
                                ),
                                "clip": "mesh_att_battle_5_01_end",
                                "mappingBasis": "PROJECT_AUTHORED",
                                "sourceStartMs": 0,
                                "playMs": 4433,
                                "playRate": 1.0,
                                "repeatUntilStageEnd": False,
                            }
                        ],
                    },
                },
            ],
            owner,
        )
        authored = pattern(candidate, "VALTAN_GROUND_ROAR")
        self.assertEqual([400440, 400425], authored["sourceActionIds"])
        self.assertEqual(
            {
                "sourceActionId": 400425,
                "sequenceIndex": 0,
                "role": "REFERENCE_400425_0",
            },
            authored["presentationSources"][-1],
        )
        self.assertEqual(
            ["mesh_att_battle_11_01", "mesh_att_battle_5_01_end"],
            [
                row["clip"]
                for row in stage(authored, "STEP_01")["animation"]["occurrences"]
            ],
        )

    def test_managed_canonical_pattern_preserves_exact_cross_source_sequence(self) -> None:
        expected_clips = [
            "mesh_abn_groggy_1_start",
            "mesh_abn_groggy_1_loop",
            "mesh_abn_groggy_1_loop",
            "mesh_abn_groggy_1_loop",
            "mesh_abn_groggy_1_end",
        ]
        dash = pattern(self.master, "VALTAN_DASH_CHARGE")
        part_break = pattern(self.master, "VALTAN_PART_BREAK")
        self.assertEqual([420604, 400430], dash["sourceActionIds"])
        self.assertEqual([420627], part_break["sourceActionIds"])
        self.assertEqual(
            [
                {"sourceActionId": 420604, "sequenceIndex": 2, "role": "PRIMARY"},
                {
                    "sourceActionId": 400430,
                    "sequenceIndex": 0,
                    "role": "REFERENCE_400430_0",
                },
            ],
            dash["presentationSources"],
        )
        self.assertEqual(
            [{"sourceActionId": 420627, "sequenceIndex": 1, "role": "PRIMARY"}],
            part_break["presentationSources"],
        )
        recovery = stage(dash, "GROGGY")
        self.assertEqual(6833, recovery["durationMs"])
        self.assertEqual("GROGGY", recovery["stageKind"])
        self.assertEqual(
            "DESTROY_FIRST_ELIGIBLE", recovery["partDamagePolicy"]
        )
        self.assertEqual(
            expected_clips,
            [
                row["clip"]
                for row in recovery["animation"]["occurrences"]
            ],
        )
        self.assertEqual(
            [1833, 1333, 1333, 334, 2000],
            [row["playMs"] for row in recovery["animation"]["occurrences"]],
        )

        gameplay, presentation = pipeline.split_v2_authoring(
            self.master,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        rejoined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_manual_audition_animation_lineage(
            rejoined,
            pipeline.read_json(ROOT / pipeline.DEBUG_PRESENTATION_REL),
            pipeline.read_json(ROOT / pipeline.ANIMATION_PROMOTION_MANIFEST_REL),
            repository_root=ROOT,
        )

    def test_existing_sequence_append_can_be_removed_with_its_provenance(self) -> None:
        owner = self.ground_roar_with_420617_append()
        original = pattern(owner, "VALTAN_GROUND_ROAR")
        original_stage = stage(original, "STEP_01")
        self.assertEqual(
            [400440, 400425, 420617], original["sourceActionIds"]
        )
        self.assertEqual(7, len(original_stage["animation"]["occurrences"]))

        candidate = self.patch(
            [
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_GROUND_ROAR",
                    "stageId": "STEP_01",
                    "animation": {
                        "endPolicy": "HOLD_LAST_POSE",
                        "repeatCount": 1,
                        "occurrences": copy.deepcopy(
                            original_stage["animation"]["occurrences"][:2]
                        ),
                    },
                }
            ],
            owner,
        )
        ground_roar = pattern(candidate, "VALTAN_GROUND_ROAR")
        self.assertEqual([400440, 400425], ground_roar["sourceActionIds"])
        self.assertEqual(
            [
                {
                    "sourceActionId": 400440,
                    "sequenceIndex": 0,
                    "role": "PRIMARY",
                },
                {
                    "sourceActionId": 400425,
                    "sequenceIndex": 0,
                    "role": "REFERENCE_400425_0",
                },
            ],
            ground_roar["presentationSources"],
        )
        self.assertEqual(
            [
                "mesh_att_battle_11_01",
                "mesh_att_battle_5_01_end",
            ],
            [
                occurrence["clip"]
                for occurrence in stage(ground_roar, "STEP_01")["animation"][
                    "occurrences"
                ]
            ],
        )
        self.assertEqual(
            "SOURCE_REVIEWED_DELTA",
            stage(ground_roar, "STEP_01")["animation"]["occurrences"][0][
                "mappingBasis"
            ],
        )

        gameplay, presentation = pipeline.split_v2_authoring(
            candidate,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        rejoined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_manual_audition_animation_lineage(
            rejoined,
            pipeline.read_json(ROOT / pipeline.DEBUG_PRESENTATION_REL),
            pipeline.read_json(ROOT / pipeline.ANIMATION_PROMOTION_MANIFEST_REL),
            repository_root=ROOT,
        )

    def test_partial_sequence_append_removal_keeps_ordered_slice_provenance(self) -> None:
        owner = self.ground_roar_with_420617_append()
        original = pattern(owner, "VALTAN_GROUND_ROAR")
        original_occurrences = stage(original, "STEP_01")["animation"][
            "occurrences"
        ]
        candidate = self.patch(
            [
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_GROUND_ROAR",
                    "stageId": "STEP_01",
                    "animation": {
                        "endPolicy": "HOLD_LAST_POSE",
                        "repeatCount": 1,
                        "occurrences": copy.deepcopy(original_occurrences[:-1]),
                    },
                }
            ],
            owner,
        )
        ground_roar = pattern(candidate, "VALTAN_GROUND_ROAR")
        self.assertEqual(
            [400440, 400425, 420617], ground_roar["sourceActionIds"]
        )
        self.assertEqual(3, len(ground_roar["presentationSources"]))

        gameplay, presentation = pipeline.split_v2_authoring(
            candidate,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        rejoined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_manual_audition_animation_lineage(
            rejoined,
            pipeline.read_json(ROOT / pipeline.DEBUG_PRESENTATION_REL),
            pipeline.read_json(
                ROOT / pipeline.ANIMATION_PROMOTION_MANIFEST_REL
            ),
            repository_root=ROOT,
        )

    def ground_roar_with_420617_append(self) -> dict:
        owner = copy.deepcopy(self.master)
        ground_roar = pattern(owner, "VALTAN_GROUND_ROAR")
        ground_roar["sourceActionIds"].append(420617)
        ground_roar["presentationSources"].append(
            {
                "sourceActionId": 420617,
                "sequenceIndex": 1,
                "role": "REFERENCE_420617_1",
            }
        )
        ground_roar_stage = stage(ground_roar, "STEP_01")
        ground_roar_stage["durationMs"] += 10000
        appended = self.sequence_420617_animation()["occurrences"]
        original_occurrences = copy.deepcopy(
            ground_roar_stage["animation"]["occurrences"]
        )
        for index, occurrence in enumerate(
            appended, len(original_occurrences) + 1
        ):
            occurrence["clipOccurrenceId"] = (
                f"valtan.sequence.sequence.400440.0.step-01.composition.clip-{index:02d}"
            )
        ground_roar_stage["animation"] = {
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [
                *original_occurrences,
                *appended,
            ],
        }
        return owner

    @staticmethod
    def authored_animation() -> dict:
        return {
            "endPolicy": "HOLD_LAST_POSE",
            "repeatCount": 1,
            "occurrences": [
                {
                    "clipOccurrenceId": "valtan.sequence.charge.step-01.composition.02",
                    "clip": "mesh_att_battle_9_01_loop",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 300,
                    "playMs": 600,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
                {
                    "clipOccurrenceId": "valtan.sequence.charge.step-01.clip-01",
                    "clip": "mesh_att_battle_9_01_start",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 500,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
            ],
        }

    def composed_operations(self) -> list[dict]:
        return [
            {
                "op": "SET_STAGE_KIND",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "stageKind": "WINDUP",
            },
            {
                "op": "SET_STAGE_KIND",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_03",
                "stageKind": "GROGGY",
            },
            {
                "op": "SET_STAGE_DURATION",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "durationMs": 2200,
            },
            {
                "op": "SET_STAGE_ANIMATION",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "animation": self.authored_animation(),
            },
            {
                "op": "SET_STAGE_COUNTER_WINDOW",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "enabled": True,
                "successStageId": "STEP_03",
                "successActionId": "valtan.sequence.charge.step-03",
                "timeoutStageId": "STEP_02",
                "timeoutActionId": "valtan.sequence.charge.step-02",
            },
        ]

    def test_manual_stage_sequence_gap_kind_and_counter_project_together(self) -> None:
        original = pattern(self.master, "VALTAN_CHARGE")
        original_identity = [
            (row["stageId"], row["actionId"], row["defaultNextActionId"])
            for row in original["stages"]
        ]
        candidate = self.patch(self.composed_operations())
        authored = pattern(candidate, "VALTAN_CHARGE")
        self.assertEqual(
            self.master["decisionModel"]["manualAuditions"],
            candidate["decisionModel"]["manualAuditions"],
            "composition must not promote or rewrite AUDITION_ONLY selection ownership",
        )
        for field in (
            "actionId",
            "entryActionId",
            "targetPolicy",
            "aimPolicy",
            "eligibility",
            "sourceActionIds",
            "presentationSources",
        ):
            self.assertEqual(original[field], authored[field])
        self.assertEqual(
            original_identity,
            [
                (row["stageId"], row["actionId"], row["defaultNextActionId"])
                for row in authored["stages"]
            ],
            "typed composition must preserve stable Stage/Action IDs and topology",
        )
        source = stage(authored, "STEP_01")
        target = stage(authored, "STEP_03")
        self.assertEqual("WINDUP", source["stageKind"])
        self.assertEqual("GROGGY", target["stageKind"])
        self.assertEqual(2200, source["durationMs"])
        self.assertEqual(
            [
                "mesh_att_battle_9_01_loop",
                "mesh_att_battle_9_01_start",
            ],
            [row["clip"] for row in source["animation"]["occurrences"]],
        )
        self.assertEqual(
            [300, 0],
            [row["sourceStartMs"] for row in source["animation"]["occurrences"]],
        )
        self.assertTrue(
            all(
                row["mappingBasis"] == "SOURCE_REVIEWED_DELTA"
                for row in source["animation"]["occurrences"]
            )
        )
        self.assertEqual(
            "SOURCE_REVIEWED_DELTA",
            source["effectCues"][0]["mappingBasis"],
            "a retained cue must stay joined to its retagged occurrence",
        )
        self.assertEqual(
            [{"outcome": "COUNTER_HIT", "nextActionId": target["actionId"]}],
            [row for row in source["branches"] if row["outcome"] == "COUNTER_HIT"],
        )

        gameplay, presentation = pipeline.split_v2_authoring(
            candidate,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        rejoined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_manual_audition_animation_lineage(
            rejoined,
            pipeline.read_json(ROOT / pipeline.DEBUG_PRESENTATION_REL),
            pipeline.read_json(ROOT / pipeline.ANIMATION_PROMOTION_MANIFEST_REL),
        )
        outputs = pipeline.project_v2_products(ROOT, self.docs, rejoined)
        bindings = json.loads(outputs[pipeline.BINDINGS_REL])
        binding = next(
            row
            for row in bindings["bindings"]
            if row["actionId"] == "valtan.sequence.charge.step-01"
        )
        self.assertEqual(
            ["mesh_att_battle_9_01_loop", "mesh_att_battle_9_01_start"],
            [row["clip"] for row in binding["clips"]],
        )
        encounter = json.loads(outputs[pipeline.ENCOUNTER_REL])
        product = next(
            row
            for row in encounter["patterns"]
            if row["patternId"] == "VALTAN_CHARGE"
        )
        product_source = stage(product, "STEP_01")
        self.assertEqual("WINDUP", product_source["stageKind"])
        self.assertEqual(2200, product_source["durationMs"])

    def test_invalid_kind_non_manual_kind_and_partial_failure_preserve_input(self) -> None:
        cases = (
            {
                "op": "SET_STAGE_KIND",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "stageKind": "RECOVERY",
            },
            {
                "op": "SET_STAGE_KIND",
                "patternId": "VALTAN_HIGH_JUMP",
                "stageId": "AIRBORNE",
                "stageKind": "WINDUP",
            },
        )
        for operation in cases:
            before = copy.deepcopy(self.master)
            with self.subTest(operation=operation), self.assertRaises(
                pipeline.DraftPatchError
            ):
                self.patch([operation], before)
            self.assertEqual(self.master, before)

        before = copy.deepcopy(self.master)
        operations = [
            {
                "op": "SET_STAGE_DURATION",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "durationMs": 2200,
            },
            cases[1],
        ]
        with self.assertRaises(pipeline.DraftPatchError):
            self.patch(operations, before)
        self.assertEqual(self.master, before)

    def test_linked_counter_stage_kinds_cannot_be_left_dangling(self) -> None:
        linked = self.patch(self.composed_operations())
        cases = (
            (
                {
                    "op": "SET_STAGE_KIND",
                    "patternId": "VALTAN_CHARGE",
                    "stageId": "STEP_01",
                    "stageKind": "ACTIVE",
                },
                "COUNTER_SOURCE_KIND_LOCKED",
            ),
            (
                {
                    "op": "SET_STAGE_KIND",
                    "patternId": "VALTAN_CHARGE",
                    "stageId": "STEP_03",
                    "stageKind": "ACTIVE",
                },
                "COUNTER_TARGET_KIND_LOCKED",
            ),
        )
        for operation, error_code in cases:
            before = copy.deepcopy(linked)
            with self.subTest(operation=operation), self.assertRaises(
                pipeline.DraftPatchError
            ) as raised:
                self.patch([operation], before)
            self.assertEqual(error_code, raised.exception.error_code)
            self.assertEqual(linked, before)

    def test_cpp_emitter_orders_counter_topology_before_field_patches(self) -> None:
        source = BALANCE_CPP.read_text(encoding="utf-8")
        emitter = source[
            source.index("bool Client::CBalanceTool::BuildValtanDraftPatch(") :
            source.index("bool Client::CBalanceTool::RunValtanDraftCommand(")
        ]
        phase_tokens = (
            "counterDisableOperations.begin(), counterDisableOperations.end()",
            "manualStagePreCounterTopologyOperations.begin()",
            "stageRoleOperations.begin(), stageRoleOperations.end()",
            "counterEnableOperations.begin(), counterEnableOperations.end()",
            "manualStagePostCounterTopologyOperations.begin()",
            "operations.begin(), operations.end()",
        )
        positions = [emitter.index(token) for token in phase_tokens]
        self.assertEqual(sorted(positions), positions)

        # This is the exact topology order emitted for a newly promoted
        # source Stage that precedes its Groggy target in Stage order.
        emitted_topology = [
            operation
            for kind in ("SET_STAGE_KIND", "SET_STAGE_COUNTER_WINDOW")
            for operation in self.composed_operations()
            if operation["op"] == kind
        ]
        candidate = self.patch(emitted_topology)
        authored = pattern(candidate, "VALTAN_CHARGE")
        self.assertEqual("WINDUP", stage(authored, "STEP_01")["stageKind"])
        self.assertEqual("GROGGY", stage(authored, "STEP_03")["stageKind"])
        self.assertEqual(
            "valtan.sequence.charge.step-03",
            next(
                branch["nextActionId"]
                for branch in stage(authored, "STEP_01")["branches"]
                if branch["outcome"] == "COUNTER_HIT"
            ),
        )

    def test_workbench_append_uses_a_free_stable_occurrence_after_delete(self) -> None:
        source = WORKBENCH_CPP.read_text(encoding="utf-8")
        allocator = source[
            source.index("std::string BuildNextCompositionSlotId(") :
            source.index("bool_t ComputeExactAnimationWallMs(")
        ]
        self.assertIn("ordinal <= Slots.size()", allocator)
        self.assertIn("Slot.clipOccurrenceId == candidate", allocator)
        append = source[
            source.index("bool_t Client::CActionCompositionWorkbench::Apply_SelectedSequenceToStage(") :
            source.index("bool_t Client::CActionCompositionWorkbench::Seek_EffectivePreview(")
        ]
        self.assertIn("BuildNextCompositionSlotId(", append)
        self.assertNotIn(
            "BuildCompositionSlotId(\n\t\t\t\tPattern.strPatternId",
            append,
        )

        # A middle delete leaves .01/.03 with size two.  Size-based allocation
        # reproduced .03; the bounded free-suffix scan selects .02.
        existing = {
            "VALTAN_CHARGE.STEP_01.composition.clip.01",
            "VALTAN_CHARGE.STEP_01.composition.clip.03",
        }
        candidate = next(
            f"VALTAN_CHARGE.STEP_01.composition.clip.{ordinal:02d}"
            for ordinal in range(1, len(existing) + 2)
            if f"VALTAN_CHARGE.STEP_01.composition.clip.{ordinal:02d}"
            not in existing
        )
        self.assertEqual("VALTAN_CHARGE.STEP_01.composition.clip.02", candidate)
        self.assertNotIn(candidate, existing)

        self.assertIn(
            '"Release Velocity (m/s)", &Action.fSpeedMps, 0.05f, 0.f, 50.f',
            source,
        )
        self.assertIn(
            '"Release Duration (ms)", &iReleaseDuration, 5.f, 0, 5000',
            source,
        )

    def test_last_sequence_slot_remove_becomes_manual_animation_none(self) -> None:
        workbench = WORKBENCH_CPP.read_text(encoding="utf-8")
        details = workbench[
            workbench.index(
                "void Client::CActionCompositionWorkbench::Render_AnimationStageDetails("
            ) : workbench.index(
                "void Client::CActionCompositionWorkbench::Render_Details("
            )
        ]
        self.assertIn("if (iRemove < Draft.animationSlots.size())", details)
        remove_block = details[
            details.index("if (iRemove < Draft.animationSlots.size())") :
            details.index("else if (iMoveUp", details.index("if (iRemove <"))
        ]
        self.assertNotIn("Draft.animationSlots.size() > 1u", remove_block)
        self.assertIn('Draft.animationEndPolicy = "NONE"', details)
        self.assertIn("Draft.animationRepeatCount = Draft.animationSlots.empty() ? 0u", details)
        self.assertIn("Animation Mode: NONE", details)

        balance = BALANCE_CPP.read_text(encoding="utf-8")
        setter = balance[
            balance.index("bool Client::CBalanceTool::Set_ValtanStageDraft(") :
            balance.index("bool Client::CBalanceTool::Get_ValtanCounterWindowDraft(")
        ]
        for token in (
            "const bool bAnimationNone = candidate.animationSlots.empty()",
            "!pattern->bManualServerAudition || isWaitStage",
            '"NONE" != candidate.animationEndPolicy',
            "0u != candidate.animationRepeatCount",
            "stage->ClipOccurrences.clear()",
            'stage->strAnimationEndPolicy = "NONE"',
            "stage->iAuthoringRepeatCount = 0u",
            "stage->bSuppressAnimation = true",
        ):
            self.assertIn(token, setter)
        emitter = balance[
            balance.index("bool Client::CBalanceTool::BuildValtanDraftPatch(") :
            balance.index("bool Client::CBalanceTool::RunValtanDraftCommand(")
        ]
        self.assertIn(r'\"animation\": { \"mode\": \"NONE\" }', emitter)

        pipeline_source = (
            ROOT / "Tools/ValtanPipeline/valtan_tuning_pipeline.py"
        ).read_text(encoding="utf-8")
        self.assertIn('if set(animation) == {"mode"}:', pipeline_source)
        self.assertIn(
            'stage["animation"] = {"mode": ANIMATION_MODE_NONE}',
            pipeline_source,
        )
        self.assertIn("has_reviewed_none_delta", pipeline_source)

    def test_balance_backend_exposes_manual_capability_and_rejects_stale_mutation(self) -> None:
        header = BALANCE_H.read_text(encoding="utf-8")
        source = BALANCE_CPP.read_text(encoding="utf-8")
        for token in (
            "stageKindEditable",
            "SET_STAGE_KIND",
            "MANUAL_SERVER_AUDITION Stage admits ACTIVE, WINDUP, or GROGGY",
            "draft.durationEditable = true",
            "!stage.bSuppressAnimation && !stage.ClipOccurrences.empty()",
            "disable this Stage's Counter window before changing its WINDUP kind",
            "disable every Counter window targeting this GROGGY Stage before changing its kind",
        ):
            self.assertIn(token, header + source)
        gate = "Require_ValtanAuthoringAdmission"
        self.assertIn(gate, header)
        for function in (
            "Set_ValtanStageDraft",
            "Set_ValtanCounterWindowDraft",
            "Set_ValtanHighJumpAxeCountDraft",
        ):
            body = source[source.index(f"bool Client::CBalanceTool::{function}(") :]
            body = body[: body.index("\n}")]
            self.assertIn(gate, body)
        self.assertIn("stale-preserved graph is display-only", source)
        render = source[
            source.index("void Client::CBalanceTool::RenderValtanPatternAuthoring()") :
            source.index("void Client::CBalanceTool::RenderValtanDecisionTrace(")
        ]
        self.assertIn("STALE PRESERVED / READ ONLY", render)
        self.assertIn("ImGui::BeginDisabled(!bAuthoringAdmitted)", render)


if __name__ == "__main__":
    unittest.main(verbosity=2)
