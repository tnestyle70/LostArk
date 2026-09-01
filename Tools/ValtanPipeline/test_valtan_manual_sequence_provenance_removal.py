#!/usr/bin/env python3
"""Focused regression for deleting a previously appended manual Sequence."""

from __future__ import annotations

import copy
import pathlib
import sys
import unittest


sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline


ROOT = pathlib.Path(__file__).resolve().parents[2]


def occurrence(ordinal: int, clip: str, basis: str = "PROJECT_AUTHORED") -> dict:
    return {
        "clipOccurrenceId": f"ground-roar.clip-{ordinal:02d}",
        "clip": clip,
        "mappingBasis": basis,
        "sourceStartMs": 0,
        "playMs": 1800 if ordinal == 1 else 1000,
        "playRate": 1.0,
        "repeatUntilStageEnd": False,
    }


def master() -> dict:
    clips = [
        "mesh_att_battle_11_01",
        "mesh_att_battle_17_start",
        "mesh_att_battle_17_loop",
        "mesh_att_battle_17_end",
        "mesh_att_battle_17_loop",
        "mesh_att_battle_17_end",
    ]
    return {
        "bossArchetypeId": "BOSS_VALTAN",
        "encounterId": "ENCOUNTER_VALTAN",
        "decisionModel": {
            "manualAuditions": [
                {
                    "patternId": "VALTAN_GROUND_ROAR",
                    "sourceChainId": "sequence.400440.0",
                    "authoringPhase": 1,
                    "admissionState": "MANUAL_SERVER_AUDITION",
                }
            ]
        },
        "patterns": [
            {
                "patternId": "VALTAN_GROUND_ROAR",
                "sourceActionIds": [400440, 420617],
                "presentationSources": [
                    {
                        "sourceActionId": 400440,
                        "sequenceIndex": 0,
                        "role": "PRIMARY",
                    },
                    {
                        "sourceActionId": 420617,
                        "sequenceIndex": 1,
                        "role": "REFERENCE_420617_1",
                    },
                ],
                "stages": [
                    {
                        "stageId": "STEP_01",
                        "sequenceRole": "STEP",
                        "animation": {
                            "endPolicy": "EXACT",
                            "repeatCount": 1,
                            "occurrences": [
                                occurrence(ordinal, clip)
                                for ordinal, clip in enumerate(clips, 1)
                            ],
                        },
                    }
                ],
            }
        ],
    }


def debug_presentation() -> dict:
    return {
        "schema": "lostark.valtan-pattern-presentation-debug",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "encounterId": "ENCOUNTER_VALTAN",
        "chains": [
            {
                "chainId": "sequence.400440.0",
                "targetPatternId": "VALTAN_GROUND_ROAR",
                "targetStageId": "STEP_01",
                "animation": {
                    "endPolicy": "NATIVE_CLIP_LENGTHS",
                    "repeatCount": 1,
                    "occurrences": [
                        {
                            "clipOccurrenceId": "debug.ground-roar.clip-01",
                            "clip": "mesh_att_battle_11_01",
                            "mappingBasis": "PROJECT_AUTHORED",
                            "sourceStartMs": 0,
                            "playMs": 1800,
                            "playRate": 1.0,
                            "repeatUntilStageEnd": False,
                        }
                    ],
                },
            }
        ],
    }


def promotion_manifest() -> dict:
    return {
        "schema": "lostark.valtan-animation-chain-promotions",
        "formatVersion": 2,
        "bossArchetypeId": "BOSS_VALTAN",
        "encounterId": "ENCOUNTER_VALTAN",
        "sourceDocument": "Data/Valtan/Valtan.presentation.debug.json",
        "presentationProfile": "BOSS_VALTAN",
        "clipAliases": [],
        "animationIntakeOnly": [],
        "patterns": [
            {
                "sourceChainId": "sequence.400440.0",
                "patternId": "VALTAN_GROUND_ROAR",
                "displayName": "Ground Roar",
                "authoringPhase": 1,
                "admissionState": "MANUAL_SERVER_AUDITION",
                "sourceActionId": 400440,
                "sourceSequenceIndex": 0,
            }
        ],
    }


class ManualSequenceProvenanceRemovalTests(unittest.TestCase):
    def test_full_slot_removal_prunes_the_exact_deterministic_reference(self) -> None:
        original = master()
        candidate = copy.deepcopy(original)
        candidate_pattern = candidate["patterns"][0]
        candidate_pattern["stages"][0]["animation"]["occurrences"] = [
            occurrence(1, "mesh_att_battle_11_01", "SOURCE_REVIEWED_DELTA")
        ]

        pipeline._prune_removed_manual_sequence_provenance(
            original,
            candidate,
            ROOT,
            {"VALTAN_GROUND_ROAR": 0},
        )

        self.assertEqual([400440], candidate_pattern["sourceActionIds"])
        self.assertEqual(
            [
                {
                    "sourceActionId": 400440,
                    "sequenceIndex": 0,
                    "role": "PRIMARY",
                }
            ],
            candidate_pattern["presentationSources"],
        )
        pipeline.validate_manual_audition_animation_lineage(
            candidate,
            debug_presentation(),
            promotion_manifest(),
            repository_root=ROOT,
        )

    def test_partial_ordered_slice_keeps_the_deterministic_reference(self) -> None:
        original = master()
        candidate = copy.deepcopy(original)
        occurrences = candidate["patterns"][0]["stages"][0]["animation"][
            "occurrences"
        ]
        del occurrences[-1]
        for row in occurrences:
            row["mappingBasis"] = "SOURCE_REVIEWED_DELTA"

        pipeline._prune_removed_manual_sequence_provenance(
            original,
            candidate,
            ROOT,
            {"VALTAN_GROUND_ROAR": 0},
        )
        self.assertEqual(
            original["patterns"][0]["presentationSources"],
            candidate["patterns"][0]["presentationSources"],
        )
        pipeline.validate_manual_audition_animation_lineage(
            candidate,
            debug_presentation(),
            promotion_manifest(),
            repository_root=ROOT,
        )

    def test_malformed_reference_is_not_silently_pruned(self) -> None:
        original = master()
        candidate = copy.deepcopy(original)
        occurrences = candidate["patterns"][0]["stages"][0]["animation"][
            "occurrences"
        ]
        occurrences[:] = [
            occurrence(
                1,
                "mesh_att_battle_11_01",
                "SOURCE_REVIEWED_DELTA",
            )
        ]
        original["patterns"][0]["presentationSources"][1][
            "role"
        ] = "REFERENCE_MALFORMED"
        candidate["patterns"][0]["presentationSources"][1][
            "role"
        ] = "REFERENCE_MALFORMED"

        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "malformed deterministic Sequence provenance",
        ):
            pipeline._prune_removed_manual_sequence_provenance(
                original,
                candidate,
                ROOT,
                {"VALTAN_GROUND_ROAR": 0},
            )
            pipeline.validate_manual_audition_animation_lineage(
                candidate,
                debug_presentation(),
                promotion_manifest(),
                repository_root=ROOT,
            )
        self.assertEqual(
            original["patterns"][0]["presentationSources"],
            candidate["patterns"][0]["presentationSources"],
        )

    def test_retained_reference_rejects_disconnected_reordered_or_replaced_rows(
        self,
    ) -> None:
        for mutation in ("disconnected", "reordered", "replacement"):
            with self.subTest(mutation=mutation):
                original = master()
                candidate = copy.deepcopy(original)
                occurrences = candidate["patterns"][0]["stages"][0][
                    "animation"
                ]["occurrences"]
                primary = occurrences[0]
                source_rows = occurrences[1:]
                if mutation == "disconnected":
                    occurrences[:] = [
                        primary,
                        source_rows[0],
                        *source_rows[2:],
                    ]
                elif mutation == "reordered":
                    occurrences[:] = [
                        primary,
                        source_rows[0],
                        source_rows[2],
                        source_rows[1],
                        *source_rows[3:],
                    ]
                else:
                    occurrences[:] = [
                        primary,
                        source_rows[0],
                        {
                            **occurrence(
                                7,
                                "mesh_att_battle_19_01",
                                "SOURCE_REVIEWED_DELTA",
                            ),
                            "clipOccurrenceId": "ground-roar.replacement-01",
                        },
                        *source_rows[2:],
                    ]
                for row in occurrences:
                    row["mappingBasis"] = "SOURCE_REVIEWED_DELTA"

                with self.assertRaisesRegex(
                    pipeline.PipelineError,
                    "declared Sequence source has no exact ordered Product occurrence slice",
                ):
                    pipeline._prune_removed_manual_sequence_provenance(
                        original,
                        candidate,
                        ROOT,
                        {"VALTAN_GROUND_ROAR": 0},
                    )
                self.assertEqual(
                    original["patterns"][0]["presentationSources"],
                    candidate["patterns"][0]["presentationSources"],
                )

    def test_global_span_assignment_backtracks_around_an_earlier_overlap(
        self,
    ) -> None:
        selected = pipeline._assign_non_overlapping_sequence_spans(
            [
                [("STEP_01", ("a", "b")), ("STEP_01", ("c",))],
                [("STEP_01", ("a", "b"))],
            ]
        )
        self.assertEqual(
            [("STEP_01", ("c",)), ("STEP_01", ("a", "b"))],
            selected,
        )

    def test_independent_duplicate_outside_retained_reference_slice_is_allowed(
        self,
    ) -> None:
        original = master()
        candidate = copy.deepcopy(original)
        occurrences = candidate["patterns"][0]["stages"][0]["animation"][
            "occurrences"
        ]
        for row in occurrences:
            row["mappingBasis"] = "SOURCE_REVIEWED_DELTA"
        occurrences.append(
            {
                **occurrence(
                    7,
                    "mesh_att_battle_19_01",
                    "PROJECT_AUTHORED",
                ),
                "clipOccurrenceId": "ground-roar.duplicate-01",
            }
        )

        pipeline._prune_removed_manual_sequence_provenance(
            original,
            candidate,
            ROOT,
            {"VALTAN_GROUND_ROAR": 0},
        )
        self.assertEqual(
            original["patterns"][0]["presentationSources"],
            candidate["patterns"][0]["presentationSources"],
        )
        pipeline.validate_manual_audition_animation_lineage(
            candidate,
            debug_presentation(),
            promotion_manifest(),
            repository_root=ROOT,
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
