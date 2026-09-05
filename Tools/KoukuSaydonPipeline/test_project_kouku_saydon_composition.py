import copy
import json
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest import mock


from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject


ROOT = Path(__file__).resolve().parents[2]


class KoukuSaydonCompositionProjectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.document = subject.load_json(ROOT / subject.SOURCE_PATH)

    def validate(self, document):
        subject.validate_document(document, ROOT)

    @staticmethod
    def pizza(document):
        return next(
            pattern
            for pattern in document["patterns"]
            if pattern.get("patternId") == "KAKULSAYDON_G1_PIZZA"
        )

    def test_seed_is_the_exact_six_stage_pizza_reference(self):
        self.validate(copy.deepcopy(self.document))
        self.assertEqual(2, self.document["revision"])
        self.assertEqual(["KAKULSAYDON_G1_PIZZA"], self.document["playAllPatternIds"])
        self.assertEqual(5, len(self.document["patterns"]))
        pattern = self.pizza(self.document)
        self.assertEqual("PRODUCT", pattern["authoringStatus"])
        self.assertEqual("MECHANIC", pattern["category"])
        self.assertEqual(7, pattern["nextAnimationOrdinal"])
        self.assertEqual(
            [
                ("WINDUP_A", "kakulsaydon.g1.pizza.windup-a", "WINDUP", 2500,
                 "stage-000", "rpcz00_att_battle_3_01"),
                ("SECTOR_SWEEP_A", "kakulsaydon.g1.pizza.sector-sweep-a", "ACTIVE", 4667,
                 "stage-001", "rpcz00_att_battle_3_07"),
                ("RECOVERY_A", "kakulsaydon.g1.pizza.recovery-a", "RECOVERY", 1000,
                 "stage-002", "rpcz00_att_battle_3_09"),
                ("WINDUP_B", "kakulsaydon.g1.pizza.windup-b", "WINDUP", 2500,
                 "stage-003", "rpcz00_att_battle_3_01"),
                ("SECTOR_SWEEP_B", "kakulsaydon.g1.pizza.sector-sweep-b", "ACTIVE", 4667,
                 "stage-004", "rpcz00_att_battle_3_07"),
                ("RECOVERY_B", "kakulsaydon.g1.pizza.recovery-b", "RECOVERY", 1000,
                 "stage-005", "rpcz00_att_battle_3_09"),
            ],
            [
                (
                    stage["stageId"],
                    stage["actionId"],
                    stage["stageKind"],
                    stage["durationMs"],
                    stage["animationOccurrences"][0]["sourceStageId"],
                    stage["animationOccurrences"][0]["runtimeClip"],
                )
                for stage in pattern["stages"]
            ],
        )
        for ordinal, stage in enumerate(pattern["stages"], 1):
            occurrence = stage["animationOccurrences"][0]
            self.assertEqual(4219714, occurrence["sourceActionId"])
            self.assertEqual("animation-000", occurrence["sourceSlotId"])
            self.assertEqual(0, occurrence["startOffsetMs"])
            self.assertEqual(stage["durationMs"], occurrence["playMs"])
            self.assertEqual("EXACT", occurrence["endPolicy"])
            self.assertEqual(
                f"KAKULSAYDON_G1_PIZZA.animation.{ordinal}",
                occurrence["occurrenceId"],
            )

    def test_rejects_unknown_fields_versions_ids_and_non_integer_revision(self):
        mutations = []
        extra = copy.deepcopy(self.document)
        extra["futureFamilies"] = []
        mutations.append(extra)
        wrong_version = copy.deepcopy(self.document)
        wrong_version["formatVersion"] = 2
        mutations.append(wrong_version)
        wrong_id = copy.deepcopy(self.document)
        wrong_id["bossPlacementId"] = "boss.kakulsaydon.g1.other"
        mutations.append(wrong_id)
        boolean_revision = copy.deepcopy(self.document)
        boolean_revision["revision"] = True
        mutations.append(boolean_revision)
        for mutation in mutations:
            with self.subTest(mutation=mutation):
                with self.assertRaises(subject.CompositionError):
                    self.validate(mutation)

    def test_rejects_duplicate_and_ahead_of_counter_identities(self):
        duplicate = copy.deepcopy(self.document)
        duplicate["patterns"].append(copy.deepcopy(self.pizza(duplicate)))
        duplicate["playAllPatternIds"].append("KAKULSAYDON_G1_PIZZA")
        with self.assertRaisesRegex(subject.CompositionError, "duplicate patternId"):
            self.validate(duplicate)

        pattern_counter = copy.deepcopy(self.document)
        self.pizza(pattern_counter)["patternId"] = "KAKULSAYDON_G1_PATTERN_1"
        pattern_counter["playAllPatternIds"] = ["KAKULSAYDON_G1_PATTERN_1"]
        with self.assertRaisesRegex(subject.CompositionError, "nextPatternOrdinal"):
            self.validate(pattern_counter)

        occurrence_counter = copy.deepcopy(self.document)
        self.pizza(occurrence_counter)["nextAnimationOrdinal"] = 6
        with self.assertRaisesRegex(subject.CompositionError, "nextAnimationOrdinal"):
            self.validate(occurrence_counter)

    def test_draft_may_be_empty_but_never_projects_or_replaces_product_inventory(self):
        document = copy.deepcopy(self.document)
        draft = copy.deepcopy(self.pizza(document))
        draft.update(
            patternId="KAKULSAYDON_G1_PATTERN_1",
            displayName="초안",
            authoringStatus="DRAFT",
            nextStageOrdinal=1,
            nextAnimationOrdinal=1,
            stages=[],
        )
        document["nextPatternOrdinal"] = 2
        document["patterns"].append(draft)
        self.validate(document)
        encounter = subject.project_encounter(document)
        presentation = subject.project_presentation(document)
        self.assertEqual(["KAKULSAYDON_G1_PIZZA"], encounter["playAllPatternIds"])
        self.assertEqual(1, len(encounter["patterns"]))
        self.assertEqual(6, len(presentation["bindings"]))

        self.pizza(document)["authoringStatus"] = "DRAFT"
        document["playAllPatternIds"] = []
        self.validate(document)
        with self.assertRaisesRegex(subject.CompositionError, "at least one PRODUCT"):
            subject.validate_publishable(document)

    def test_product_inventory_is_bounded_to_downstream_catalog_capacity(self):
        document = copy.deepcopy(self.document)
        seed = self.pizza(document)
        document["patterns"] = []
        document["playAllPatternIds"] = []
        for ordinal in range(1, subject.MAX_PRODUCT_PATTERNS + 2):
            pattern = copy.deepcopy(seed)
            pattern_id = f"KAKULSAYDON_G1_PATTERN_{ordinal}"
            pattern["patternId"] = pattern_id
            pattern["displayName"] = f"Product {ordinal}"
            pattern["nextStageOrdinal"] = 7
            pattern["nextAnimationOrdinal"] = 7
            for stage_index, stage in enumerate(pattern["stages"], 1):
                stage["stageId"] = f"STAGE_{stage_index}"
                stage["actionId"] = f"{pattern_id}.stage.{stage_index}"
                stage["animationOccurrences"][0]["occurrenceId"] = (
                    f"{pattern_id}.animation.{stage_index}"
                )
            document["patterns"].append(pattern)
            document["playAllPatternIds"].append(pattern_id)
        document["nextPatternOrdinal"] = subject.MAX_PRODUCT_PATTERNS + 2

        with self.assertRaisesRegex(subject.CompositionError, "64 PRODUCT patterns"):
            self.validate(document)

    def test_product_pattern_ids_cannot_collapse_to_one_runtime_action_id(self):
        document = copy.deepcopy(self.document)
        duplicate = copy.deepcopy(self.pizza(document))
        duplicate["patternId"] = "KAKULSAYDON.G1.PIZZA"
        duplicate["displayName"] = "Derived action collision"
        for ordinal, stage in enumerate(duplicate["stages"], 1):
            stage["actionId"] = f"kakulsaydon.g1.collision.stage.{ordinal}"
            stage["animationOccurrences"][0]["occurrenceId"] = (
                f"KAKULSAYDON.G1.PIZZA.animation.{ordinal}"
            )
        document["patterns"].append(duplicate)
        document["playAllPatternIds"].append("KAKULSAYDON.G1.PIZZA")

        with self.assertRaisesRegex(subject.CompositionError, "duplicate actionId"):
            self.validate(document)

    def test_normal_patterns_are_draft_only_in_the_animation_mvp(self):
        document = copy.deepcopy(self.document)
        self.pizza(document)["category"] = "NORMAL"
        with self.assertRaisesRegex(subject.CompositionError, "MECHANIC category"):
            self.validate(document)

    def test_display_name_uses_the_same_255_utf8_byte_ui_boundary(self):
        accepted = copy.deepcopy(self.document)
        accepted["patterns"][0]["displayName"] = "a" * 255
        self.validate(accepted)

        too_many_bytes = copy.deepcopy(self.document)
        too_many_bytes["patterns"][0]["displayName"] = "가" * 86
        with self.assertRaisesRegex(subject.CompositionError, "255 UTF-8 bytes"):
            self.validate(too_many_bytes)

        control = copy.deepcopy(self.document)
        control["patterns"][0]["displayName"] = "bad\nname"
        with self.assertRaisesRegex(subject.CompositionError, "control characters"):
            self.validate(control)

    def test_product_requires_one_full_stage_animation(self):
        missing = copy.deepcopy(self.document)
        self.pizza(missing)["stages"][0]["animationOccurrences"] = []
        with self.assertRaisesRegex(subject.CompositionError, "exactly one"):
            self.validate(missing)

        offset = copy.deepcopy(self.document)
        occurrence = self.pizza(offset)["stages"][0]["animationOccurrences"][0]
        occurrence["startOffsetMs"] = 1
        occurrence["playMs"] -= 1
        with self.assertRaisesRegex(subject.CompositionError, "whole stage"):
            self.validate(offset)

    def test_product_rejects_presentation_policies_the_client_cannot_run(self):
        variants = []
        source_offset = copy.deepcopy(self.document)
        stage = self.pizza(source_offset)["stages"][0]
        stage["durationMs"] = 100
        stage["animationOccurrences"][0]["playMs"] = 100
        stage["animationOccurrences"][0]["sourceStartMs"] = 1
        variants.append(source_offset)

        slow_rate = copy.deepcopy(self.document)
        stage = self.pizza(slow_rate)["stages"][0]
        stage["durationMs"] = 100
        stage["animationOccurrences"][0]["playMs"] = 100
        stage["animationOccurrences"][0]["playRate"] = 0.05
        variants.append(slow_rate)

        hold = copy.deepcopy(self.document)
        stage = self.pizza(hold)["stages"][0]
        stage["durationMs"] = 100
        stage["animationOccurrences"][0]["playMs"] = 100
        stage["animationOccurrences"][0]["endPolicy"] = "HOLD_LAST_POSE"
        variants.append(hold)

        for mutation in variants:
            with self.subTest(mutation=mutation):
                with self.assertRaisesRegex(
                    subject.CompositionError, "current runtime policy"
                ):
                    self.validate(mutation)

    def test_reference_metadata_does_not_gate_composition(self):
        for key, value in (
            ("referenceRevision", "0" * 64),
            ("runtimeClip", "rpcz00_idle_battle_1"),
            ("sourceSlotId", "animation-999"),
        ):
            document = copy.deepcopy(self.document)
            self.pizza(document)["stages"][0]["animationOccurrences"][0][key] = value
            with self.subTest(key=key), mock.patch.object(subject, "load_json") as read:
                self.validate(document)
                read.assert_not_called()

    def test_raw_model_clips_publish_without_extracted_action_metadata(self):
        document = copy.deepcopy(self.document)
        for stage in self.pizza(document)["stages"]:
            row = stage["animationOccurrences"][0]
            row.update(sourceActionId=0, sourceStageId="RAW",
                       sourceSlotId=row["runtimeClip"], referenceRevision="")
        self.validate(document)
        self.assertEqual([], subject.project_encounter(document)["patterns"][0]["sourceActionIds"])
        self.assertEqual(6, len(subject.project_presentation(document)["bindings"]))

    def test_bad_draft_does_not_block_product_publish(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / subject.SOURCE_PATH
            source.parent.mkdir(parents=True)
            document = copy.deepcopy(self.document)
            document["patterns"][0] = {"authoringStatus": "DRAFT", "stages": "broken"}
            source.write_text(json.dumps(document), encoding="utf-8")
            result = subject.run(root, "publish")
            self.assertEqual(2, result["outputCount"])
            before = (root / subject.ENCOUNTER_PATH).read_bytes()
            self.pizza(document)["stages"][0]["animationOccurrences"] = []
            source.write_text(json.dumps(document), encoding="utf-8")
            with self.assertRaises(subject.CompositionError):
                subject.run(root, "publish")
            self.assertEqual(before, (root / subject.ENCOUNTER_PATH).read_bytes())

    def test_play_all_is_exact_product_order(self):
        missing = copy.deepcopy(self.document)
        missing["playAllPatternIds"] = []
        with self.assertRaisesRegex(subject.CompositionError, "PRODUCT patternIds"):
            self.validate(missing)
        unknown = copy.deepcopy(self.document)
        unknown["playAllPatternIds"] = ["KAKULSAYDON_G1_UNKNOWN"]
        with self.assertRaisesRegex(subject.CompositionError, "PRODUCT patternIds"):
            self.validate(unknown)

    def test_product_projection_is_minimal_and_timeout_only(self):
        encounter = subject.project_encounter(self.document)
        self.assertEqual(
            {
                "schema",
                "formatVersion",
                "encounterId",
                "bossArchetypeId",
                "authority",
                "fixedTickHz",
                "sourceRevision",
                "playAllPatternIds",
                "patterns",
            },
            set(encounter),
        )
        pattern = encounter["patterns"][0]
        self.assertEqual("AUDITION_ONLY", pattern["selectionMode"])
        self.assertNotIn("branches", pattern)
        self.assertNotIn("actions", pattern)
        for stage in pattern["stages"]:
            self.assertEqual("NONE", stage["hitShape"])
            self.assertEqual(0, stage["hitCount"])
            self.assertEqual("", stage["serverDamageProfileId"])
            self.assertNotIn("branches", stage)
            self.assertNotIn("actions", stage)

        presentation = subject.project_presentation(self.document)
        self.assertEqual(
            {
                "schema",
                "formatVersion",
                "bossArchetypeId",
                "sourceRevision",
                "bindings",
            },
            set(presentation),
        )
        self.assertEqual(6, len(presentation["bindings"]))

    def test_publish_is_deterministic_and_validate_detects_stale_product(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / subject.SOURCE_PATH
            reference = root / subject.REFERENCE_ROOT / "MN_RPCZ_00.actionreference.json"
            source.parent.mkdir(parents=True)
            reference.parent.mkdir(parents=True)
            shutil.copy2(ROOT / subject.SOURCE_PATH, source)
            shutil.copy2(
                ROOT / subject.REFERENCE_ROOT / "MN_RPCZ_00.actionreference.json",
                reference,
            )
            result = subject.run(root, "publish")
            self.assertEqual(2, result["outputCount"])
            expected = subject.projected_outputs(subject.load_and_validate(root))
            subject.validate_outputs(root, expected)
            encounter = root / subject.ENCOUNTER_PATH
            encounter.write_text("{}\n", encoding="utf-8")
            with self.assertRaisesRegex(subject.CompositionError, "stale"):
                subject.validate_outputs(root, expected)

    def test_publish_rolls_back_both_products_after_post_commit_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            outputs = {
                Path("Data/first.json"): b'{"revision": 2}\n',
                Path("Data/second.json"): b'{"revision": 2}\n',
            }
            baselines = {}
            for relative in outputs:
                destination = root / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                baseline = b'{"revision": 1}\n'
                destination.write_bytes(baseline)
                baselines[relative] = baseline

            with mock.patch.object(
                subject,
                "validate_outputs",
                side_effect=subject.CompositionError("forced verification failure"),
            ):
                with self.assertRaisesRegex(
                    subject.CompositionError, "forced verification failure"
                ):
                    subject.publish_outputs(root, outputs)

            for relative, baseline in baselines.items():
                self.assertEqual(baseline, (root / relative).read_bytes())
            self.assertEqual([], list(root.rglob("*.rollback.*")))

    def test_publish_preserves_backup_and_reports_path_when_rollback_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            relative = Path("Data/product.json")
            destination = root / relative
            destination.parent.mkdir(parents=True)
            destination.write_bytes(b'{"revision": 1}\n')
            real_replace = subject.os.replace

            def fail_rollback(source, target):
                if ".rollback." in Path(source).name:
                    raise OSError("forced rollback failure")
                return real_replace(source, target)

            with mock.patch.object(
                subject,
                "validate_outputs",
                side_effect=subject.CompositionError("forced verification failure"),
            ), mock.patch.object(subject.os, "replace", side_effect=fail_rollback):
                with self.assertRaisesRegex(
                    subject.CompositionError,
                    r"rollback was incomplete; recovery backup\(s\) preserved: .*\.rollback\.",
                ):
                    subject.publish_outputs(
                        root, {relative: b'{"revision": 2}\n'}
                    )

            backups = list(destination.parent.glob(".product.json.rollback.*"))
            self.assertEqual(1, len(backups))
            self.assertEqual(b'{"revision": 1}\n', backups[0].read_bytes())
            self.assertEqual(b'{"revision": 2}\n', destination.read_bytes())

    def test_tracked_products_are_current(self):
        expected = subject.projected_outputs(self.document)
        subject.validate_outputs(ROOT, expected)


if __name__ == "__main__":
    unittest.main()
