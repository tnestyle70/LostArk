#!/usr/bin/env python3
from __future__ import annotations

import argparse
import copy
import hashlib
import re
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import promote_valtan_animation_chains as promotion
import author_valtan_phase_two_mechanics as phase_two


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def reviewed_closure_counts(root: Path) -> tuple[int, int]:
    manifest = promotion._read_json(root / promotion.MANIFEST_REL)
    debug_document = promotion._read_json(root / promotion.DEBUG_REL)
    chains_by_id = {row["chainId"]: row for row in debug_document["chains"]}
    promotions = manifest["patterns"]
    promoted_chains = [
        chains_by_id[row["sourceChainId"]] for row in promotions
    ]
    return promotion._reviewed_closure_counts(promotions, promoted_chains)


class ValtanAnimationChainPromotionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = REPOSITORY_ROOT.resolve()

    def test_reviewed_chain_closure_and_stable_ids(self) -> None:
        gameplay, presentation, receipt = promotion.build_candidates(self.root)
        saved_gameplay = promotion._read_json(
            self.root / promotion.GAMEPLAY_REL
        )
        saved_presentation = promotion._read_json(
            self.root / promotion.PRESENTATION_REL
        )

        def stable_pattern_ids(document: dict) -> list[str]:
            pattern_ids = [row["patternId"] for row in document["patterns"]]
            self.assertTrue(pattern_ids)
            self.assertEqual(len(pattern_ids), len(set(pattern_ids)))
            self.assertTrue(
                all(
                    isinstance(pattern_id, str)
                    and re.fullmatch(r"VALTAN_[A-Z0-9_]+", pattern_id)
                    is not None
                    for pattern_id in pattern_ids
                )
            )
            return pattern_ids

        gameplay_ids = stable_pattern_ids(gameplay)
        presentation_ids = stable_pattern_ids(presentation)
        saved_gameplay_ids = stable_pattern_ids(saved_gameplay)
        saved_presentation_ids = stable_pattern_ids(saved_presentation)
        self.assertEqual(gameplay_ids, presentation_ids)
        self.assertEqual(set(gameplay_ids), set(saved_gameplay_ids))
        self.assertEqual(
            set(presentation_ids), set(saved_presentation_ids)
        )
        expected_pattern_count, expected_stage_count = reviewed_closure_counts(
            self.root
        )
        self.assertEqual(expected_pattern_count, receipt["patternCount"])
        self.assertEqual(expected_stage_count, receipt["stageCount"])
        self.assertEqual(
            expected_stage_count,
            sum(len(pattern["occurrences"]) for pattern in receipt["patterns"]),
        )
        six_pizza = next(
            row for row in receipt["patterns"]
            if row["patternId"] == "VALTAN_SIX_PIZZA_106"
        )
        self.assertEqual("STEP_01", six_pizza["occurrences"][0]["targetStageId"])
        self.assertEqual(
            "valtan.sequence.center-six-pizza-charge.step-01.clip-01",
            six_pizza["occurrences"][0]["targetClipOccurrenceId"],
        )
        patterns_by_id = {
            row["patternId"]: row for row in receipt["patterns"]
        }
        ghost_wrappers = [
            patterns_by_id["VALTAN_GHOST_RESPAWN_AUDITION"],
            patterns_by_id["VALTAN_GHOST_DEATH_AUDITION"],
        ]
        self.assertEqual(
            ["VALTAN_GHOST_RESPAWN_AUDITION", "VALTAN_GHOST_DEATH_AUDITION"],
            [row["patternId"] for row in ghost_wrappers],
        )
        self.assertEqual(
            ["mesh_respawn_1", "mesh_dead_1"],
            [row["occurrences"][0]["resolvedClip"] for row in ghost_wrappers],
        )
        self.assertTrue(all(row["stageCount"] == 1 for row in ghost_wrappers))

    def test_refresh_preserves_ground_roar_appended_sequence_source(self) -> None:
        saved_gameplay = promotion._read_json(self.root / promotion.GAMEPLAY_REL)
        saved_presentation = promotion._read_json(
            self.root / promotion.PRESENTATION_REL
        )

        def row(document: dict) -> dict:
            return next(
                item
                for item in document["patterns"]
                if item["patternId"] == "VALTAN_GROUND_ROAR"
            )

        saved_g = row(saved_gameplay)
        saved_g["sourceActionIds"].append(400437)
        saved_g["stages"][0]["durationMs"] += 2633
        saved_p = row(saved_presentation)
        saved_p["presentationSources"].append(
            {
                "sourceActionId": 400437,
                "sequenceIndex": 0,
                "role": "REFERENCE_400437_0",
            }
        )
        saved_p["stages"][0]["animation"]["occurrences"].append(
            {
                "clipOccurrenceId": (
                    "valtan.sequence.sequence.400440.0.step-01."
                    "composition.clip.01"
                ),
                "clip": "mesh_evt1_att_battle_5_01_end",
                "mappingBasis": "SOURCE_REVIEWED_DELTA",
                "sourceStartMs": 0,
                "playMs": 2633,
                "playRate": 1.0,
                "repeatUntilStageEnd": False,
            }
        )

        gameplay, presentation, _receipt = self._build_with_source_fixture(
            saved_gameplay, saved_presentation
        )

        promoted_g = row(gameplay)
        self.assertEqual(
            [400440, 400425, 400437], saved_g["sourceActionIds"]
        )
        self.assertEqual(saved_g["sourceActionIds"], promoted_g["sourceActionIds"])
        self.assertEqual(
            saved_g["stages"][0]["durationMs"],
            promoted_g["stages"][0]["durationMs"],
        )

        promoted_p = row(presentation)
        self.assertEqual(
            {
                "sourceActionId": 400437,
                "sequenceIndex": 0,
                "role": "REFERENCE_400437_0",
            },
            saved_p["presentationSources"][-1],
        )
        self.assertEqual(
            saved_p["presentationSources"], promoted_p["presentationSources"]
        )
        self.assertEqual(
            saved_p["stages"][0]["animation"]["occurrences"],
            promoted_p["stages"][0]["animation"]["occurrences"],
        )
        promotion.validate_and_project(self.root, gameplay, presentation)

    def test_production_closure_counts_follow_reviewed_manifest_and_debug(self) -> None:
        promotions = [{"sourceChainId": "phase-three-a"}, {"sourceChainId": "phase-three-b"}]
        chains = [
            {"animation": {"occurrences": [{}, {}]}},
            {"animation": {"occurrences": [{}]}},
        ]
        self.assertEqual(
            (2, 3), promotion._reviewed_closure_counts(promotions, chains)
        )

        future_receipt = {
            "patternCount": 21,
            "stageCount": 95,
            "patterns": [{"occurrences": []}],
        }
        with mock.patch.object(
            promotion,
            "build_candidates",
            return_value=({}, {}, future_receipt),
        ), mock.patch.object(
            promotion,
            "validate_and_project",
            return_value={},
        ):
            result = promotion.run(self.root, "Validate")
        self.assertEqual(21, result["patternCount"])
        self.assertEqual(95, result["stageCount"])

    def test_native_explicit_and_loop_durations_are_frozen(self) -> None:
        _gameplay, _presentation, receipt = promotion.build_candidates(self.root)
        occurrences = {
            row["sourceClipOccurrenceId"]: row
            for pattern in receipt["patterns"]
            for row in pattern["occurrences"]
        }
        native = occurrences["valtan.debug.center-six-pizza-charge.clip.01"]
        self.assertEqual("NATIVE_WMODEL", native["resolution"])
        self.assertEqual("EXACT", native["endPolicy"])
        self.assertGreater(native["productPlayMs"], 0)
        self.assertEqual(native["nativeSourceMs"], native["stageDurationMs"])

        loop = occurrences["valtan.debug.center-six-pizza-charge.clip.06"]
        self.assertEqual("EXPLICIT_WALL_LOOP", loop["resolution"])
        self.assertEqual(8000, loop["stageDurationMs"])
        self.assertEqual(0, loop["productPlayMs"])
        self.assertEqual("LOOP_TO_STAGE_END", loop["endPolicy"])

        exact = next(
            row
            for row in occurrences.values()
            if row["resolution"] == "EXPLICIT_WALL_EXACT"
        )
        self.assertEqual("EXACT", exact["endPolicy"])
        self.assertGreater(exact["productPlayMs"], 0)

    def test_catch_breath_preserves_tuned_target_and_exact_four_stage_sequence(self) -> None:
        gameplay, presentation, _receipt = promotion.build_candidates(self.root)
        gameplay_pattern = next(
            pattern
            for pattern in gameplay["patterns"]
            if pattern["patternId"] == "VALTAN_CATCH_BREATH"
        )
        self.assertEqual(
            "LOCK_RANDOM_ALIVE_BEHIND_ON_START", gameplay_pattern["targetPolicy"]
        )
        self.assertEqual("LOCK_FACING_ON_START", gameplay_pattern["aimPolicy"])
        self.assertEqual(
            [2000, 500, 4000, 2000],
            [stage["durationMs"] for stage in gameplay_pattern["stages"]],
        )
        self.assertEqual(
            [
                {
                    "outcome": "ANY_PLAYER_GRABBED",
                    "nextActionId": "valtan.sequence.catch-breath.step-03",
                },
                {"outcome": "TIMEOUT", "nextActionId": None},
            ],
            gameplay_pattern["stages"][1]["branches"],
        )

        presentation_pattern = next(
            pattern
            for pattern in presentation["patterns"]
            if pattern["patternId"] == "VALTAN_CATCH_BREATH"
        )
        occurrences = [
            stage["animation"]["occurrences"][0]
            for stage in presentation_pattern["stages"]
        ]
        self.assertEqual(
            [
                "mesh_att_battle_21_01",
                "mesh_att_battle_21_02",
                "mesh_att_battle_21_03",
                "mesh_att_battle_21_04",
            ],
            [occurrence["clip"] for occurrence in occurrences],
        )
        self.assertTrue(occurrences[2]["repeatUntilStageEnd"])
        self.assertEqual(0, occurrences[2]["playMs"])

    def test_jump_whirlwind_facing_survives_promotion_and_reauthoring(self) -> None:
        saved_gameplay = promotion._read_json(self.root / promotion.GAMEPLAY_REL)
        saved_presentation = promotion._read_json(self.root / promotion.PRESENTATION_REL)
        promoted_gameplay, promoted_presentation, _receipt = promotion.build_candidates(self.root)
        authored_gameplay, authored_presentation = phase_two.build()

        def pattern(document: dict) -> dict:
            return next(
                row for row in document["patterns"]
                if row["patternId"] == "VALTAN_ATTACK_WHIRLWIND"
            )

        expected_events = [[], [], [], [{
            "eventId": "event.valtan.attack-whirlwind.reaim",
            "trigger": "ENTER",
            "kind": "RETARGET_RANDOM_ALIVE",
        }]]
        saved_pattern = pattern(saved_gameplay)
        saved_animations = [
            stage["animation"] for stage in pattern(saved_presentation)["stages"]
        ]
        for name, gameplay, presentation in (
            ("saved", saved_gameplay, saved_presentation),
            ("promoted", promoted_gameplay, promoted_presentation),
            ("reauthored", authored_gameplay, authored_presentation),
        ):
            with self.subTest(path=name):
                actual = pattern(gameplay)
                self.assertEqual(
                    ("LOCK_NEAREST_ON_START", "LOCK_FACING_ON_START"),
                    (actual["targetPolicy"], actual["aimPolicy"]),
                )
                self.assertEqual(expected_events, [stage["events"] for stage in actual["stages"]])
                self.assertEqual(saved_pattern, actual)
                self.assertEqual(
                    saved_animations,
                    [stage["animation"] for stage in pattern(presentation)["stages"]],
                )

        phase_two.author_existing_patterns(authored_gameplay, authored_presentation)
        self.assertEqual(
            expected_events,
            [stage["events"] for stage in pattern(authored_gameplay)["stages"]],
        )

    def test_promotion_and_reauthoring_preserve_canonical_scripted_sequence(self) -> None:
        saved_gameplay = promotion._read_json(self.root / promotion.GAMEPLAY_REL)
        expected_reference = copy.deepcopy(
            saved_gameplay["decisionModel"]["scriptedSequence"]
        )
        self.assertEqual(
            {"sequenceId", "mode", "interStepPursuitMs", "patternIds"},
            set(expected_reference),
        )
        promoted_gameplay, promoted_presentation, _receipt = promotion.build_candidates(self.root)
        self.assertEqual(
            expected_reference,
            promoted_gameplay["decisionModel"]["scriptedSequence"],
        )
        for author in (
            phase_two.author_existing_patterns,
            phase_two.author_terrain_pairs,
        ):
            with self.subTest(author=author.__name__):
                author(promoted_gameplay, promoted_presentation)
                self.assertEqual(
                    expected_reference,
                    promoted_gameplay["decisionModel"]["scriptedSequence"],
                )

    def test_reviewed_clip_aliases_cover_promoted_and_retired_intake(self) -> None:
        _gameplay, _presentation, receipt = promotion.build_candidates(self.root)
        aliases = {
            row["sourceClip"]: row["resolvedClip"]
            for pattern in receipt["patterns"]
            for row in pattern["occurrences"]
            if row["aliasApplied"]
        }
        self.assertEqual(
            {
                "att_battle_19_02": "mesh_att_battle_19_02",
                "att_battle_19_04": "mesh_att_battle_19_04",
                "att_battle_20_02": "mesh_att_battle_20_02",
                "att_battle_20_03": "mesh_att_battle_20_03",
                "att_battle_20_04": "mesh_att_battle_20_04",
            },
            aliases,
        )
        manifest = promotion._read_json(self.root / promotion.MANIFEST_REL)
        self.assertEqual(
            {**aliases, "att_battle_2_03": "mesh_att_battle_2_03"},
            manifest["clipAliases"],
        )

    def test_retired_intake_and_saved_cue_unlinks_survive_animation_refresh(self) -> None:
        gameplay, presentation, receipt = promotion.build_candidates(self.root)
        retired_id = "VALTAN_SEQUENCE_FRONT_BACK_FRONT"
        self.assertIn(retired_id, gameplay["retiredPatternIds"])
        for document in (gameplay, presentation, receipt):
            self.assertNotIn(retired_id, {row["patternId"] for row in document["patterns"]})
        saved = promotion._read_json(self.root / promotion.PRESENTATION_REL)
        cue_lists = lambda document: {
            (pattern["patternId"], stage["stageId"]): stage["effectCues"]
            for pattern in document["patterns"]
            for stage in pattern["stages"]
        }
        self.assertEqual(cue_lists(saved), cue_lists(presentation))
        authored_gameplay, authored_presentation = phase_two.build()
        self.assertEqual(cue_lists(saved), cue_lists(authored_presentation))
        self.assertNotIn(retired_id, {row["patternId"] for row in authored_gameplay["patterns"]})
        four = next(row for row in authored_gameplay["patterns"] if row["patternId"] == "VALTAN_SEQUENCE_FOUR")
        self.assertEqual(["STEP_01"], [stage["stageId"] for stage in four["stages"]])
        unlinked_ids = {
            "cue.valtan.carrier-v1.mechanic.arena-break-109.impact.clip-01",
            "cue.valtan.carrier-v1.mechanic.arena-break-109.roar-recovery.clip-01",
            "cue.valtan.carrier-v1.mechanic.floor-wipe-130.second-smash.clip-01",
            "cue.valtan.requested.20260827.struggling.composite",
        }
        self.assertTrue(unlinked_ids.isdisjoint(
            cue["cueId"] for rows in cue_lists(presentation).values() for cue in rows
        ))

        manifest = promotion._read_json(self.root / promotion.MANIFEST_REL)
        invalid = copy.deepcopy(manifest)
        retired = invalid["animationIntakeOnly"].pop(0)
        self.assertEqual("front-back-front", retired["sourceChainId"])
        retired.update(patternId=retired_id, admissionState="MANUAL_SERVER_AUDITION")
        invalid["patterns"].insert(8, retired)
        read_json = promotion._read_json
        with mock.patch.object(
            promotion, "_read_json",
            side_effect=lambda path: invalid if path == self.root / promotion.MANIFEST_REL else read_json(path),
        ):
            with self.assertRaisesRegex(promotion.PromotionError, "retired patterns cannot be promoted"):
                promotion.build_candidates(self.root)

    def test_animation_refresh_preserves_server_and_presentation_enrichment(self) -> None:
        generated_gameplay = {
            "patternId": "VALTAN_WARP",
            "targetPolicy": "NONE",
            "aimPolicy": "NONE",
            "eligibility": {"minimumRangeM": 0.0},
            "invulnerableWhileRunning": False,
            "serverMotion": None,
            "reactions": [],
            "stages": [
                {
                    "stageId": "STEP_01",
                    "actionId": "valtan.sequence.warp.step-01",
                    "durationMs": 900,
                    "stageKind": "ACTIVE",
                    "defaultNextActionId": None,
                    "hit": {"shape": {"kind": "NONE"}},
                    "motion": None,
                    "events": [],
                    "branches": [],
                }
            ],
        }
        existing_gameplay = {
            **generated_gameplay,
            "targetPolicy": "LOCK_RANDOM_ALIVE_ON_START",
            "aimPolicy": "LOCK_FACING_ON_START",
            "stages": [
                {
                    **generated_gameplay["stages"][0],
                    "durationMs": 2300,
                    "hit": {"shape": {"kind": "BOX"}},
                    "motion": {
                        "kind": "PORTAL_TARGET_RUSH",
                        "retargetDelayMs": 500,
                        "speedMps": 20.0,
                        "distanceM": 16.0,
                    },
                    "events": [{"eventId": "event.valtan.warp.portal"}],
                }
            ],
        }
        preserved_gameplay = promotion._preserve_manual_gameplay_enrichment(
            generated_gameplay, existing_gameplay
        )
        self.assertEqual(
            "LOCK_RANDOM_ALIVE_ON_START", preserved_gameplay["targetPolicy"]
        )
        self.assertEqual(
            "BOX", preserved_gameplay["stages"][0]["hit"]["shape"]["kind"]
        )
        self.assertEqual(2300, preserved_gameplay["stages"][0]["durationMs"])

        generated_presentation = {
            "patternId": "VALTAN_WARP",
            "stages": [
                {
                    "stageId": "STEP_01",
                    "actionId": "valtan.sequence.warp.step-01",
                    "sequenceRole": "STEP",
                    "animation": {"occurrences": [{"clip": "mesh_att_battle_18_01"}]},
                    "effectCues": [],
                    "cameraInvocations": [],
                }
            ],
        }
        existing_presentation = {
            **generated_presentation,
            "stages": [
                {
                    **generated_presentation["stages"][0],
                    "effectCues": [{"bindingId": "cue.valtan.warp.portal"}],
                }
            ],
        }
        preserved_presentation = promotion._preserve_manual_presentation_enrichment(
            generated_presentation, existing_presentation
        )
        self.assertEqual(
            "cue.valtan.warp.portal",
            preserved_presentation["stages"][0]["effectCues"][0]["bindingId"],
        )

    def test_animation_refresh_rejects_enrichment_stage_identity_drift(self) -> None:
        generated = {
            "patternId": "VALTAN_WARP",
            "stages": [
                {
                    "stageId": "STEP_01",
                    "actionId": "valtan.sequence.warp.step-01",
                }
            ],
        }
        existing = {
            "stages": [
                {
                    "stageId": "STEP_01",
                    "actionId": "valtan.sequence.warp.renamed",
                }
            ]
        }
        with self.assertRaises(promotion.PromotionError):
            promotion._preserve_manual_presentation_enrichment(
                generated, existing
            )

    def _build_with_source_fixture(self, gameplay: dict, presentation: dict) -> tuple:
        read_json = promotion._read_json
        snapshots = {
            self.root / promotion.GAMEPLAY_REL: gameplay,
            self.root / promotion.PRESENTATION_REL: presentation,
        }
        with mock.patch.object(
            promotion, "_read_json",
            side_effect=lambda path: copy.deepcopy(snapshots[path])
            if path in snapshots else read_json(path),
        ):
            return promotion.build_candidates(self.root)

    def test_trash_capture_refresh_keeps_runtime_graph_and_intake_receipt_separate(self) -> None:
        saved_g = promotion._read_json(self.root / promotion.GAMEPLAY_REL)
        saved_p = promotion._read_json(self.root / promotion.PRESENTATION_REL)
        gameplay, presentation, receipt = self._build_with_source_fixture(saved_g, saved_p)
        trash = phase_two.pattern(gameplay, "VALTAN_TRASH")
        trash_p = phase_two.pattern(presentation, "VALTAN_TRASH")
        self.assertEqual(phase_two.pattern(saved_g, "VALTAN_TRASH"), trash)
        self.assertEqual(phase_two.pattern(saved_p, "VALTAN_TRASH"), trash_p)
        self.assertEqual(22, len(trash["stages"]))
        counter = phase_two.stage(trash, "STEP_07")
        captured_hold = phase_two.stage(trash, "CATCH_COUNTER")
        pre_impact = phase_two.stage(trash, "CATCH_PRE_IMPACT")
        self.assertEqual(1000, counter["durationMs"])
        self.assertEqual(200, captured_hold["durationMs"])
        self.assertEqual(1500, captured_hold["durationMs"] + pre_impact["durationMs"])
        self.assertEqual({
            "ALL_PLAYERS_GRABBED": phase_two.stage(trash, "EXECUTE_TAIL")["actionId"],
            "TIMEOUT": phase_two.stage(trash, "CATCH_SLAM")["actionId"],
        }, {row["outcome"]: row["nextActionId"] for row in pre_impact["branches"]})
        self.assertEqual("DAMAGE_GRABBED_PLAYERS", phase_two.stage(trash, "CATCH_SLAM")["events"][0]["kind"])
        self.assertEqual("EXECUTE_GRABBED_PLAYERS", phase_two.stage(trash, "EXECUTE_TAIL")["events"][0]["kind"])
        self.assertEqual("HOLD", phase_two.stage(trash, "GROGGY")["events"][0]["releaseMode"])
        self.assertIn(420631, trash["sourceActionIds"])
        intake = phase_two.pattern(receipt, "VALTAN_TRASH")
        self.assertEqual(8, intake["stageCount"])
        self.assertEqual(8, len(intake["occurrences"]))
        self.assertNotIn(420631, intake["sourceActionIds"])

    def test_trash_capture_refresh_preserves_tuned_proxy_and_branch_cues(self) -> None:
        gameplay = promotion._read_json(self.root / promotion.GAMEPLAY_REL)
        presentation = promotion._read_json(self.root / promotion.PRESENTATION_REL)
        trash = phase_two.pattern(gameplay, "VALTAN_TRASH")
        trash_p = phase_two.pattern(presentation, "VALTAN_TRASH")
        counter = phase_two.stage(trash, "STEP_07")
        counter["counterProxy"]["radiusM"] = 1.75
        counter_p = phase_two.stage(trash_p, "CATCH_COUNTER")
        cue = copy.deepcopy(next(
            row for pattern in presentation["patterns"] for stage in pattern["stages"]
            for row in stage["effectCues"]
        ))
        cue.update(
            cueId="cue.valtan.fixture.capture-counter",
            occurrenceId="cue.valtan.fixture.capture-counter.occurrence.01",
            clipOccurrenceId=counter_p["animation"]["occurrences"][0]["clipOccurrenceId"],
            sourceStartMs=0,
            sourceEndMs=None,
        )
        counter_p["effectCues"] = [cue]
        camera = copy.deepcopy(next(
            row for pattern in presentation["patterns"] for stage in pattern["stages"]
            for row in stage["cameraInvocations"]
        ))
        camera.update(cameraInvocationId="camera.valtan.fixture.capture-counter", durationMs=200)
        counter_p["cameraInvocations"] = [camera]
        before = copy.deepcopy((gameplay, presentation))
        refreshed_g, refreshed_p, _receipt = self._build_with_source_fixture(gameplay, presentation)
        self.assertEqual(before, (gameplay, presentation))
        self.assertEqual(trash, phase_two.pattern(refreshed_g, "VALTAN_TRASH"))
        self.assertEqual(trash_p, phase_two.pattern(refreshed_p, "VALTAN_TRASH"))
        repeated_g, repeated_p, _receipt = self._build_with_source_fixture(refreshed_g, refreshed_p)
        self.assertEqual(refreshed_g, repeated_g)
        self.assertEqual(refreshed_p, repeated_p)

    def test_trash_capture_refresh_rejects_extension_identity_and_clock_drift(self) -> None:
        source_g = promotion._read_json(self.root / promotion.GAMEPLAY_REL)
        source_p = promotion._read_json(self.root / promotion.PRESENTATION_REL)
        cases = (
            ("gameplay", lambda row: row["stages"].pop(), "stage closure/order drift"),
            ("presentation", lambda row: row["stages"].reverse(), "stage closure/order drift"),
            ("gameplay", lambda row: row["stages"].append(copy.deepcopy(row["stages"][-1])), "stage closure/order drift"),
            ("presentation", lambda row: phase_two.stage(row, "RUSH_MISS").update(stageId="UNREVIEWED_BRANCH"), "stage closure/order drift"),
            ("gameplay", lambda row: phase_two.stage(row, "EXECUTE_TAIL").update(actionId="valtan.fixture.wrong"), "action identity drift"),
            ("gameplay", lambda row: phase_two.stage(row, "CATCH_COUNTER").update(durationMs=201), "branch duration drift"),
            ("gameplay", lambda row: phase_two.stage(row, "CATCH_PRE_IMPACT").update(durationMs=1299), "branch duration drift"),
            ("gameplay", lambda row: phase_two.stage(row, "EXECUTE_TAIL").update(durationMs=1499), "branch duration drift"),
            ("gameplay", lambda row: phase_two.stage(row, "STEP_07").pop("counterProxy"), "counter proxy is missing"),
            ("presentation", lambda row: phase_two.stage(row, "CATCH_SLAM")["animation"]["occurrences"][0].update(sourceStartMs=1400), "branch source slice drift"),
            ("presentation", lambda row: phase_two.stage(row, "GROGGY")["animation"]["occurrences"].pop(), "branch source slice drift"),
            ("gameplay", lambda row: phase_two.stage(row, "CATCH_SLAM").update(defaultNextActionId="valtan.fixture.missing"), "default edge leaves its pattern"),
            ("gameplay", lambda row: phase_two.stage(row, "CATCH_PRE_IMPACT")["branches"][0].update(nextActionId="valtan.fixture.missing"), "branch leaves its pattern"),
        )
        for index, (domain, mutate, error) in enumerate(cases):
            with self.subTest(case=index, domain=domain), self.assertRaisesRegex(promotion.PromotionError, error):
                gameplay, presentation = copy.deepcopy((source_g, source_p))
                mutate(phase_two.pattern(gameplay if domain == "gameplay" else presentation, "VALTAN_TRASH"))
                self._build_with_source_fixture(gameplay, presentation)

    def test_trash_capture_invalid_typed_events_still_fail_product_validation(self) -> None:
        source_g = promotion._read_json(self.root / promotion.GAMEPLAY_REL)
        source_p = promotion._read_json(self.root / promotion.PRESENTATION_REL)
        cases = (
            lambda row: phase_two.stage(row, "CATCH_SLAM")["events"][0].update(trigger="EXIT"),
            lambda row: phase_two.stage(row, "GROGGY")["events"][0].update(releaseMode="UNKNOWN"),
            lambda row: phase_two.stage(row, "STEP_07")["counterProxy"].update(radiusM=0.0),
        )
        for index, mutate in enumerate(cases):
            with self.subTest(case=index):
                gameplay, presentation = copy.deepcopy((source_g, source_p))
                mutate(phase_two.pattern(gameplay, "VALTAN_TRASH"))
                candidate_g, candidate_p, _receipt = self._build_with_source_fixture(gameplay, presentation)
                with self.assertRaisesRegex(promotion.PromotionError, "promoted split/Product validation failed"):
                    promotion.validate_and_project(self.root, candidate_g, candidate_p)

    def test_atomic_commit_rolls_back_every_replaced_target(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            targets = {
                root / "one.json": b"new-one\n",
                root / "two.json": b"new-two\n",
                root / "three.json": b"new-three\n",
            }
            for index, path in enumerate(targets, start=1):
                path.write_bytes(f"old-{index}\n".encode("ascii"))
            before = {path: path.read_bytes() for path in targets}
            with self.assertRaises(promotion.PromotionError):
                promotion._atomic_commit(targets, inject_failure_after=2)
            self.assertEqual(before, {path: path.read_bytes() for path in targets})
            self.assertFalse(any(root.glob("*.tmp")))
            self.assertFalse(any(root.glob(".*.tmp")))

    def test_validate_mode_does_not_mutate_repository_products(self) -> None:
        tracked = (
            promotion.GAMEPLAY_REL,
            promotion.PRESENTATION_REL,
            "Data/Encounters/Valtan/ValtanEncounter.json",
            "Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
            "Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json",
        )
        before = {relative: sha256(self.root / relative) for relative in tracked}
        result = promotion.run(self.root, "Validate")
        expected_pattern_count, expected_stage_count = reviewed_closure_counts(
            self.root
        )
        self.assertEqual(expected_pattern_count, result["patternCount"])
        self.assertEqual(expected_stage_count, result["stageCount"])
        self.assertEqual(before, {relative: sha256(self.root / relative) for relative in tracked})


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    arguments, remaining = parser.parse_known_args()
    REPOSITORY_ROOT = arguments.repository_root.resolve()
    program = unittest.main(argv=[sys.argv[0], *remaining], verbosity=2, exit=False)
    raise SystemExit(0 if program.result.wasSuccessful() else 1)
