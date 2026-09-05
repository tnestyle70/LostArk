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
        # The live document grows with authored DRAFT patterns; only the Pizza
        # PRODUCT reference and the header contract are pinned here.
        self.assertGreaterEqual(self.document["revision"], 2)
        self.assertEqual(2, self.document["formatVersion"])
        self.assertEqual(["KAKULSAYDON_G1_PIZZA"], self.document["playAllPatternIds"])
        pattern = self.pizza(self.document)
        self.assertEqual("MN_RPCZ_00", pattern["actorProfileId"])
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
        wrong_version["formatVersion"] = 3
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
        ahead_id = f"KAKULSAYDON_G1_PATTERN_{pattern_counter['nextPatternOrdinal']}"
        self.pizza(pattern_counter)["patternId"] = ahead_id
        pattern_counter["playAllPatternIds"] = [ahead_id]
        with self.assertRaisesRegex(subject.CompositionError, "nextPatternOrdinal"):
            self.validate(pattern_counter)

        occurrence_counter = copy.deepcopy(self.document)
        self.pizza(occurrence_counter)["nextAnimationOrdinal"] = 6
        with self.assertRaisesRegex(subject.CompositionError, "nextAnimationOrdinal"):
            self.validate(occurrence_counter)

    def test_draft_may_be_empty_but_never_projects_or_replaces_product_inventory(self):
        document = copy.deepcopy(self.document)
        draft = copy.deepcopy(self.pizza(document))
        draft_ordinal = document["nextPatternOrdinal"]
        draft.update(
            patternId=f"KAKULSAYDON_G1_PATTERN_{draft_ordinal}",
            displayName="초안",
            authoringStatus="DRAFT",
            nextStageOrdinal=1,
            nextAnimationOrdinal=1,
            stages=[],
        )
        document["nextPatternOrdinal"] = draft_ordinal + 1
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

    @staticmethod
    def append_reference_sequence(pattern, profile_id, source_action_id=None):
        reference = subject.load_json(ROOT / subject.REFERENCE_ROOT / f"{profile_id}.actionreference.json")
        action = next(
            action for action in reference["actions"]
            if (source_action_id is None or action["sourceActionId"] == source_action_id)
            and any(stage["slots"] for stage in action["stages"])
        )
        ordinal = pattern["nextStageOrdinal"]
        pattern["nextStageOrdinal"] += 1
        stage = {
            "stageId": f"STAGE_{ordinal}",
            "actionId": f"{pattern['patternId']}.stage.{ordinal}",
            "stageKind": "ACTIVE", "durationMs": 0, "animationOccurrences": [],
        }
        for source_stage in action["stages"]:
            for slot in source_stage["slots"]:
                occurrence = {
                    "occurrenceId": f"{pattern['patternId']}.animation.{pattern['nextAnimationOrdinal']}",
                    "profileId": profile_id, "sourceActionId": action["sourceActionId"],
                    "sourceStageId": source_stage["stageId"], "sourceSlotId": slot["slotId"],
                    "referenceRevision": reference["referenceRevision"],
                    "runtimeClip": slot["runtimeClip"], "startOffsetMs": stage["durationMs"],
                    "sourceStartMs": slot["sourceStartMs"], "playMs": slot["playMs"],
                    "playRate": slot["playRate"],
                    "endPolicy": "LOOP_TO_WINDOW" if slot["loop"] else "EXACT",
                }
                pattern["nextAnimationOrdinal"] += 1
                stage["durationMs"] += slot["playMs"]
                stage["animationOccurrences"].append(occurrence)
        pattern["stages"].append(stage)
        return stage

    def test_known_actor_sequences_roundtrip_and_delete_without_retargeting(self):
        for profile_id, action_id in (("MN_RPCT_05", 4219811), ("MN_RPCT_06", None),
                                      ("MN_RPCT_07", None), ("MN_RPCZ_00", 0)):
            with self.subTest(profile=profile_id):
                document = copy.deepcopy(self.document)
                pattern = document["patterns"][0]
                owner = subject.resolve_actor_profile_id(profile_id)
                pattern["actorProfileId"] = owner
                stage = self.append_reference_sequence(pattern, profile_id, action_id)
                self.validate(document)
                reopened = json.loads(json.dumps(document, ensure_ascii=False))
                self.validate(reopened)
                self.assertEqual(document, reopened)
                self.assertTrue(all(row["profileId"] == profile_id for row in stage["animationOccurrences"]))
                if action_id == 4219811:
                    self.assertEqual(["rpct00_att_battle_12_06"],
                                     [row["runtimeClip"] for row in stage["animationOccurrences"]])
                if action_id == 0:
                    self.assertEqual(["stage-003", "stage-006"],
                                     [row["sourceStageId"] for row in stage["animationOccurrences"]])
                pattern["stages"][0]["animationOccurrences"].clear()
                self.validate(document)
                pattern["stages"].clear()
                self.validate(document)
                self.assertEqual(owner, pattern["actorProfileId"])
                self.assertEqual(self.document["patterns"][1:], document["patterns"][1:])

    def test_real_249_stage_saydon_action_fits_draft_but_product_stays_bounded(self):
        document = copy.deepcopy(self.document)
        draft = document["patterns"][0]
        draft["actorProfileId"] = "MN_RPCT_05"
        sequence = self.append_reference_sequence(draft, "MN_RPCT_05", 4219880)
        self.assertEqual(249, len(sequence["animationOccurrences"]))
        self.assertEqual(273134, sequence["durationMs"])
        self.assertEqual(249, len({row["sourceStageId"] for row in sequence["animationOccurrences"]}))
        draft["stages"] = []
        for ordinal, row in enumerate(sequence["animationOccurrences"], 1):
            row["startOffsetMs"] = 0
            draft["stages"].append({
                "stageId": f"STAGE_{ordinal}",
                "actionId": f"{draft['patternId']}.stage.{ordinal}",
                "stageKind": "ACTIVE", "durationMs": row["playMs"],
                "animationOccurrences": [row],
            })
        draft["nextStageOrdinal"] = 250
        self.validate(document)
        self.validate(json.loads(json.dumps(document)))
        product = self.pizza(document)
        template = product["stages"][0]
        product["stages"] = []
        for ordinal in range(1, 66):
            stage = copy.deepcopy(template)
            stage["stageId"] = f"STAGE_{ordinal}"
            stage["actionId"] = f"{product['patternId']}.stage.{ordinal}"
            stage["animationOccurrences"][0]["occurrenceId"] = f"{product['patternId']}.animation.{ordinal}"
            product["stages"].append(stage)
        product["nextStageOrdinal"] = product["nextAnimationOrdinal"] = 66
        with self.assertRaisesRegex(subject.CompositionError, "at most 64 rows"):
            self.validate(document)

    def test_actor_ownership_rejects_cross_model_unknown_and_nonphysical_owner(self):
        document = copy.deepcopy(self.document)
        pattern = document["patterns"][0]
        pattern["actorProfileId"] = "MN_RPCT_05"
        stage = self.append_reference_sequence(pattern, "MN_RPCT_05", 4219811)
        for owner in ("MN_RPCZ_00", "MN_RPCT_06", "MN_RPCT_07", "UNKNOWN", ""):
            with self.subTest(owner=owner):
                pattern["actorProfileId"] = owner
                with self.assertRaises(subject.CompositionError):
                    self.validate(document)
        pattern["actorProfileId"] = "MN_RPCT_05"
        stage["animationOccurrences"][0]["profileId"] = "UNKNOWN"
        with self.assertRaisesRegex(subject.CompositionError, "unknown animation profile"):
            self.validate(document)
        stage["animationOccurrences"][0]["profileId"] = "MN_RPCT_07"
        self.validate(document)

    def test_non_kouku_actor_cannot_publish_as_gate1_boss(self):
        document = copy.deepcopy(self.document)
        pattern = self.pizza(document)
        pattern["actorProfileId"] = "MN_RPCT_05"
        for stage in pattern["stages"]:
            stage["animationOccurrences"][0]["profileId"] = "MN_RPCT_05"
        with self.assertRaisesRegex(subject.CompositionError, "PRODUCT actorProfileId"):
            self.validate(document)

    def test_legacy_owner_derivation_accepts_alias_and_rejects_mixed_models(self):
        document = copy.deepcopy(self.document)
        draft = document["patterns"][0]
        stage = self.append_reference_sequence(draft, "MN_RPCT_05", 4219811)
        self.append_reference_sequence(draft, "MN_RPCT_07")
        document["formatVersion"] = 1
        for pattern in document["patterns"]:
            del pattern["actorProfileId"]
        self.validate(document)
        stage["animationOccurrences"][0]["profileId"] = "MN_RPCT_06"
        with self.assertRaisesRegex(subject.CompositionError, "does not match Pattern actorProfileId"):
            self.validate(document)
        draft["stages"].clear()
        self.validate(document)

    def test_v2_requires_explicit_owner_even_for_an_empty_pattern(self):
        document = copy.deepcopy(self.document)
        del document["patterns"][0]["actorProfileId"]
        with self.assertRaises(subject.CompositionError):
            self.validate(document)

    def test_referenced_action_zero_is_preserved_in_product_source_ids(self):
        document = copy.deepcopy(self.document)
        draft = document["patterns"][0]
        source_stage = self.append_reference_sequence(draft, "MN_RPCZ_00", 0)
        row = copy.deepcopy(source_stage["animationOccurrences"][0])
        stage = self.pizza(document)["stages"][0]
        row["occurrenceId"] = stage["animationOccurrences"][0]["occurrenceId"]
        stage["animationOccurrences"] = [row]
        stage["durationMs"] = row["playMs"]
        self.validate(document)
        self.assertEqual([0, 4219714], subject.project_encounter(document)["patterns"][0]["sourceActionIds"])
        row["referenceRevision"] = ""
        with self.assertRaisesRegex(subject.CompositionError, "lowercase SHA-256"):
            self.validate(document)

    def test_raw_stage_cannot_claim_a_nonzero_reference_action(self):
        document = copy.deepcopy(self.document)
        row = self.pizza(document)["stages"][0]["animationOccurrences"][0]
        row["sourceStageId"] = "RAW"
        with self.assertRaisesRegex(subject.CompositionError, "RAW clips must use sourceActionId 0"):
            self.validate(document)

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

    @staticmethod
    def without_catalog_boxes(document):
        """Drop live Logic/Summon boxes so a test-owned catalog stays self-consistent."""
        for pattern in document["patterns"]:
            for key in ("logicOccurrences", "nextLogicOccurrenceOrdinal",
                        "summonOccurrences", "nextSummonOccurrenceOrdinal"):
                pattern.pop(key, None)
        return document

    def test_logic_catalog_is_optional_on_read_and_never_reaches_product(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        document["nextLogicOrdinal"] = 2
        document["logics"] = [
            {
                "logicId": "kakulsaydon.g1.logic.1",
                "displayName": "방패 무력화",
                "logicType": "DURATION",
            }
        ]
        self.validate(copy.deepcopy(document))
        self.assertEqual(
            subject.project_encounter(copy.deepcopy(self.document)),
            subject.project_encounter(copy.deepcopy(document)),
        )

        pizza = self.pizza(document)
        pizza["nextLogicOccurrenceOrdinal"] = 2
        pizza["logicOccurrences"] = [
            {
                "occurrenceId": "KAKULSAYDON_G1_PIZZA.logic.1",
                "logicId": "kakulsaydon.g1.logic.1",
                "startMs": 0,
                "durationMs": 1000,
            }
        ]
        with self.assertRaisesRegex(subject.CompositionError, "Server consumer"):
            self.validate(copy.deepcopy(document))

        draft = copy.deepcopy(document)
        self.pizza(draft)["authoringStatus"] = "DRAFT"
        draft["playAllPatternIds"] = []
        self.validate(copy.deepcopy(draft))

        unknown_type = copy.deepcopy(draft)
        unknown_type["logics"][0]["logicType"] = "WINDOW"
        with self.assertRaisesRegex(subject.CompositionError, "logicType"):
            self.validate(unknown_type)
        ahead = copy.deepcopy(draft)
        ahead["nextLogicOrdinal"] = 1
        with self.assertRaisesRegex(subject.CompositionError, "nextLogicOrdinal"):
            self.validate(ahead)
        dangling = copy.deepcopy(draft)
        self.pizza(dangling)["logicOccurrences"][0]["logicId"] = "kakulsaydon.g1.logic.9"
        with self.assertRaisesRegex(subject.CompositionError, "unknown logicId"):
            self.validate(dangling)
        foreign_box = copy.deepcopy(draft)
        self.pizza(foreign_box)["logicOccurrences"][0]["occurrenceId"] = "KAKULSAYDON_G1_PIZZA.animation.9"
        with self.assertRaisesRegex(subject.CompositionError, r"\.logic\.<N>"):
            self.validate(foreign_box)

    def draft_with_logic_box(self):
        document = copy.deepcopy(self.document)
        pattern = self.pizza(document)
        document["patterns"] = [pattern]
        document["playAllPatternIds"] = []
        document["nextLogicOrdinal"] = 2
        document["logics"] = [{
            "logicId": "kakulsaydon.g1.logic.1",
            "displayName": "Logic window",
            "logicType": "DURATION",
        }]
        pattern["authoringStatus"] = "DRAFT"
        pattern["nextLogicOccurrenceOrdinal"] = 2
        pattern["logicOccurrences"] = [{
            "occurrenceId": pattern["patternId"] + ".logic.1",
            "logicId": "kakulsaydon.g1.logic.1",
            "startMs": 0,
            "durationMs": 1000,
        }]
        return document, pattern

    def test_logic_occurrences_reject_duplicate_and_invalid_ordinals(self):
        document, pattern = self.draft_with_logic_box()
        self.validate(copy.deepcopy(document))
        duplicate = copy.deepcopy(document)
        duplicate_pattern = self.pizza(duplicate)
        duplicate_pattern["logicOccurrences"].append(
            copy.deepcopy(duplicate_pattern["logicOccurrences"][0])
        )
        with self.assertRaisesRegex(subject.CompositionError, "duplicate Logic occurrenceId"):
            self.validate(duplicate)

        for ordinal in (0, 2, subject.MAX_ORDINAL):
            with self.subTest(ordinal=ordinal):
                invalid = copy.deepcopy(document)
                self.pizza(invalid)["logicOccurrences"][0]["occurrenceId"] = (
                    pattern["patternId"] + f".logic.{ordinal}"
                )
                with self.assertRaises(subject.CompositionError):
                    self.validate(invalid)
        for counter in (0, 1, subject.MAX_ORDINAL + 1):
            with self.subTest(counter=counter):
                invalid = copy.deepcopy(document)
                self.pizza(invalid)["nextLogicOccurrenceOrdinal"] = counter
                with self.assertRaisesRegex(subject.CompositionError, "nextLogicOccurrenceOrdinal"):
                    self.validate(invalid)

    def test_logic_windows_use_pattern_lifetime_and_bounded_stage_clock(self):
        document, pattern = self.draft_with_logic_box()
        lifetime_ms = sum(stage["durationMs"] for stage in pattern["stages"])
        pattern["logicOccurrences"][0].update(startMs=lifetime_ms - 1, durationMs=1)
        self.validate(copy.deepcopy(document))
        beyond_lifetime = copy.deepcopy(document)
        self.pizza(beyond_lifetime)["logicOccurrences"][0]["durationMs"] = 2
        with self.assertRaisesRegex(subject.CompositionError, "Pattern lifetime"):
            self.validate(beyond_lifetime)

        pattern["stages"] = pattern["stages"][:1]
        pattern["stages"][0]["durationMs"] = subject.MAX_TIMELINE_MS
        pattern["logicOccurrences"][0].update(
            startMs=subject.MAX_TIMELINE_MS - 1, durationMs=1
        )
        self.validate(copy.deepcopy(document))
        beyond_limit = copy.deepcopy(document)
        self.pizza(beyond_limit)["logicOccurrences"][0]["durationMs"] = 2
        with self.assertRaisesRegex(subject.CompositionError, "Logic box exceeds 600 seconds"):
            self.validate(beyond_limit)

        oversized, oversized_pattern = self.draft_with_logic_box()
        oversized_pattern["stages"][0]["durationMs"] = subject.MAX_TIMELINE_MS
        with self.assertRaisesRegex(subject.CompositionError, "Pattern exceeds 600 seconds"):
            self.validate(oversized)
        empty = copy.deepcopy(document)
        self.pizza(empty)["stages"] = []
        with self.assertRaisesRegex(subject.CompositionError, "Pattern lifetime"):
            self.validate(empty)

    def test_logic_occurrence_capacity_matches_client_1024_boxes(self):
        document, pattern = self.draft_with_logic_box()
        pattern["logicOccurrences"] = [{
            "occurrenceId": pattern["patternId"] + f".logic.{ordinal}",
            "logicId": "kakulsaydon.g1.logic.1",
            "startMs": 0,
            "durationMs": 1,
        } for ordinal in range(1, 1025)]
        pattern["nextLogicOccurrenceOrdinal"] = 1025
        self.validate(copy.deepcopy(document))
        extra = copy.deepcopy(pattern["logicOccurrences"][-1])
        extra["occurrenceId"] = pattern["patternId"] + ".logic.1025"
        pattern["logicOccurrences"].append(extra)
        pattern["nextLogicOccurrenceOrdinal"] = 1026
        with self.assertRaisesRegex(subject.CompositionError, "1024"):
            self.validate(document)

    def test_logic_box_outcomes_name_result_logics_on_duration_boxes_only(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        document["nextLogicOrdinal"] = 4
        document["logics"] = [
            {"logicId": "kakulsaydon.g1.logic.1", "displayName": "방패 무력화", "logicType": "DURATION"},
            {"logicId": "kakulsaydon.g1.logic.2", "displayName": "그로기 패턴", "logicType": "RESULT"},
            {"logicId": "kakulsaydon.g1.logic.3", "displayName": "전원 전멸", "logicType": "RESULT"},
        ]
        pizza = self.pizza(document)
        pizza["authoringStatus"] = "DRAFT"
        document["playAllPatternIds"] = []
        pizza["nextLogicOccurrenceOrdinal"] = 2
        pizza["logicOccurrences"] = [
            {
                "occurrenceId": "KAKULSAYDON_G1_PIZZA.logic.1",
                "logicId": "kakulsaydon.g1.logic.1",
                "startMs": 0,
                "durationMs": 1000,
                "onSuccessLogicId": "kakulsaydon.g1.logic.2",
                "onTimeoutLogicId": "kakulsaydon.g1.logic.3",
            }
        ]
        self.validate(copy.deepcopy(document))

        unwired = copy.deepcopy(document)
        box = self.pizza(unwired)["logicOccurrences"][0]
        del box["onSuccessLogicId"]
        box["onTimeoutLogicId"] = ""
        self.validate(unwired)

        not_result = copy.deepcopy(document)
        self.pizza(not_result)["logicOccurrences"][0]["onSuccessLogicId"] = "kakulsaydon.g1.logic.1"
        with self.assertRaisesRegex(subject.CompositionError, "RESULT logic"):
            self.validate(not_result)
        dangling = copy.deepcopy(document)
        self.pizza(dangling)["logicOccurrences"][0]["onTimeoutLogicId"] = "kakulsaydon.g1.logic.9"
        with self.assertRaisesRegex(subject.CompositionError, "RESULT logic"):
            self.validate(dangling)
        on_result_box = copy.deepcopy(document)
        self.pizza(on_result_box)["logicOccurrences"][0]["logicId"] = "kakulsaydon.g1.logic.2"
        with self.assertRaisesRegex(subject.CompositionError, "DURATION logic box"):
            self.validate(on_result_box)
        wrong_type = copy.deepcopy(document)
        self.pizza(wrong_type)["logicOccurrences"][0]["onSuccessLogicId"] = 7
        with self.assertRaisesRegex(subject.CompositionError, "must be text"):
            self.validate(wrong_type)

    def test_summon_catalog_and_boxes_follow_the_logic_rules(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        document["nextSummonOrdinal"] = 2
        document["summons"] = [
            {"summonId": "kakulsaydon.g1.summon.1", "displayName": "가짜 세이튼 3"}
        ]
        self.validate(copy.deepcopy(document))
        self.assertEqual(
            subject.project_encounter(copy.deepcopy(self.document)),
            subject.project_encounter(copy.deepcopy(document)),
        )
        pizza = self.pizza(document)
        pizza["nextSummonOccurrenceOrdinal"] = 2
        pizza["summonOccurrences"] = [
            {
                "occurrenceId": "KAKULSAYDON_G1_PIZZA.summon.1",
                "summonId": "kakulsaydon.g1.summon.1",
                "startMs": 0,
                "durationMs": 16334,
            }
        ]
        with self.assertRaisesRegex(subject.CompositionError, "Server consumer"):
            self.validate(copy.deepcopy(document))
        draft = copy.deepcopy(document)
        self.pizza(draft)["authoringStatus"] = "DRAFT"
        draft["playAllPatternIds"] = []
        self.validate(copy.deepcopy(draft))

        ahead = copy.deepcopy(draft)
        ahead["nextSummonOrdinal"] = 1
        with self.assertRaisesRegex(subject.CompositionError, "nextSummonOrdinal"):
            self.validate(ahead)
        dangling = copy.deepcopy(draft)
        self.pizza(dangling)["summonOccurrences"][0]["summonId"] = "kakulsaydon.g1.summon.9"
        with self.assertRaisesRegex(subject.CompositionError, "unknown summonId"):
            self.validate(dangling)
        foreign_box = copy.deepcopy(draft)
        self.pizza(foreign_box)["summonOccurrences"][0]["occurrenceId"] = "KAKULSAYDON_G1_PIZZA.logic.9"
        with self.assertRaisesRegex(subject.CompositionError, r"\.summon\.<N>"):
            self.validate(foreign_box)
        typed = copy.deepcopy(draft)
        typed["summons"][0]["summonType"] = "SPAWN"
        with self.assertRaisesRegex(subject.CompositionError, "summons\\[0\\]"):
            self.validate(typed)

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
