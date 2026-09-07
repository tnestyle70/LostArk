import copy
import json
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest import mock


from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject


ROOT = Path(__file__).resolve().parents[2]
WORLD_SEQUENCES = Path(str(subject.WORLD_SEQUENCE_PATH).format(area=subject.AREA_ID))
FIRST_PRODUCT_ID = "KAKULSAYDON_G1_PATTERN_1"
DRAFT_ID = "KAKULSAYDON_G1_PATTERN_3"
ROULETTE_ID = "KAKULSAYDON_G1_PATTERN_7"
DANCE_ID = "KAKULSAYDON_G1_PATTERN_6"
GAZE_ID = "KAKULSAYDON_G1_PATTERN_2"


def copy_repository_inputs(root: Path) -> None:
    """The files a temp-root projection reads beside the composition."""
    for relative in (subject.BOSS_CATALOG_PATH, subject.LIGHT_RESOURCES_PATH,
                     WORLD_SEQUENCES, Path(f"Data/Maps/Authoring/{subject.AREA_ID}/{subject.AREA_ID}.mapplacements")):
        destination = root / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROOT / relative, destination)


class KoukuSaydonCompositionProjectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.document = subject.load_json(ROOT / subject.SOURCE_PATH)

    def validate(self, document):
        subject.validate_document(document, ROOT)

    @staticmethod
    def find(document, pattern_id):
        return next(
            pattern
            for pattern in document["patterns"]
            if pattern.get("patternId") == pattern_id
        )

    @classmethod
    def first_product(cls, document):
        return cls.find(document, FIRST_PRODUCT_ID)

    @classmethod
    def draft(cls, document):
        return cls.find(document, DRAFT_ID)

    @staticmethod
    def product_patterns(document):
        return [p for p in document["patterns"] if p["authoringStatus"] == "PRODUCT"]

    @classmethod
    def product_binding_count(cls, document):
        return sum(
            len(stage["animationOccurrences"])
            for pattern in cls.product_patterns(document)
            for stage in pattern["stages"]
        )

    @staticmethod
    def strip_lanes(pattern):
        """Drop every lane box of a pattern so a copy can live under another id."""
        for key in ("logicOccurrences", "summonOccurrences", "worldOccurrences",
                    "sceneProfileOccurrences", "presentationOccurrences"):
            pattern[key] = []
        for key in ("nextLogicOccurrenceOrdinal", "nextSummonOccurrenceOrdinal",
                    "nextWorldOccurrenceOrdinal", "nextSceneProfileOccurrenceOrdinal", "nextPresentationOccurrenceOrdinal"):
            pattern[key] = 1
        return pattern

    def test_stagger_effects_expand_every_authored_disarm_child_without_a_duplicate_group(self):
        pattern = self.first_product(self.document)
        resources = {row["resourceId"]: row for row in self.document["presentationResources"]}
        effects = {row["occurrenceId"]: row for row in pattern["presentationOccurrences"]
                   if resources[row["resourceId"]]["kind"] == "EFFECT"}
        group = subject.load_json(ROOT / "Data/Effects/V2/Groups/boss.kouku.disarm.effectv2group.json")
        window = pattern["logicOccurrences"][0]
        self.assertEqual(len(group["children"]), len(effects))
        for child in group["children"]:
            ordinal = int(child["childId"].rsplit(".", 1)[1]) + 1
            row = effects[f"{FIRST_PRODUCT_ID}.presentation.{ordinal}"]
            resource = resources[row["resourceId"]]
            self.assertEqual(("LEAF", child["resource"]["id"]), (resource["resourceKind"], resource["assetId"]))
            self.assertEqual(child["localTransform"]["translation"], row["positionOffset"])
            self.assertEqual(child["localTransform"]["rotation"], row["rotationDegrees"])
            self.assertEqual(child["localTransform"]["scale"], row["scale"])
            self.assertEqual(window["startMs"] + child["startMs"], row["startMs"])
            self.assertLessEqual(row["startMs"] + row["durationMs"], window["startMs"] + window["durationMs"])
            self.assertEqual(("BOSS", "", 0, 0), (row["anchorKind"], row["bone"], row["fadeInMs"], row["fadeOutMs"]))
            if resource["assetId"] == "boss.kouku.disarm.star.smoke_1":
                leaf = subject.load_json(ROOT / "Data/Effects/V2/Authored/boss.kouku.disarm.star.smoke_1.effectv2.json")
                emission = leaf["params"]["lifetime"]
                tail = max(leaf["params"]["particle"]["lifetime"])
                required_ms = round((emission + tail) * 1000 / leaf["params"]["playRate"])
                self.assertEqual(required_ms, row["durationMs"])
                self.assertFalse(leaf["params"]["loop"])
        encounter = subject.project_encounter(copy.deepcopy(self.document))
        projected = self.find(encounter, FIRST_PRODUCT_ID)
        shield = next(row for row in projected["logicWindows"] if row["windowId"] == window["occurrenceId"])
        self.assertEqual([([0, .5, 0], 0), ([0, .5, .5], 180)],
                         [(row["center"], row["yawDegrees"]) for row in shield["cardRegions"]])
        self.assertTrue(all(row["anchorKind"] == "BOSS_CURRENT" and row["shape"] == "SECTOR"
                            for row in shield["cardRegions"]))
        self.assertEqual(71.737272, shield["shieldArcDegrees"])
        for row in shield["cardRegions"]:
            self.assertEqual(35.868636, row["halfAngleDegrees"])
            self.assertEqual(2.148847, row["radiusM"])

    def test_stagger_reflection_rejects_a_non_sector_collider(self):
        document = copy.deepcopy(self.document)
        pattern = self.first_product(document)
        collider = next(row for row in pattern["presentationOccurrences"] if row.get("logicOccurrenceId"))
        resource = next(row for row in document["presentationResources"] if row["resourceId"] == collider["resourceId"])
        resource["shape"] = "BOX"
        with self.assertRaisesRegex(subject.CompositionError, "boss-pivot SECTOR"):
            subject.project_encounter(document)

    def test_stagger_reflection_rejects_a_detached_visual_collider(self):
        document = copy.deepcopy(self.document)
        pattern = self.first_product(document)
        collider = next(row for row in pattern["presentationOccurrences"] if row.get("logicOccurrenceId"))
        collider["followBoss"] = False
        with self.assertRaisesRegex(subject.CompositionError, "following boss-pivot SECTOR"):
            subject.project_encounter(document)

    def test_live_document_pins_the_gate1_saydon_products(self):
        self.validate(copy.deepcopy(self.document))
        self.assertGreaterEqual(self.document["revision"], 51)
        self.assertEqual(2, self.document["formatVersion"])
        self.assertEqual(
            ["KAKULSAYDON_G1_PATTERN_1", "KAKULSAYDON_G1_PATTERN_2", "KAKULSAYDON_G1_PATTERN_4",
             "KAKULSAYDON_G1_PATTERN_5", "KAKULSAYDON_G1_PATTERN_6", "KAKULSAYDON_G1_PATTERN_7"],
            self.document["playAllPatternIds"],
        )
        for pattern in self.product_patterns(self.document):
            self.assertEqual("MN_RPCT_05", pattern["actorProfileId"])
            self.assertEqual("MECHANIC", pattern["category"])
            for stage in pattern["stages"]:
                self.assertEqual(1, len(stage["animationOccurrences"]))
                self.assertEqual("EXACT", stage["animationOccurrences"][0]["endPolicy"])
        self.assertNotIn("KAKULSAYDON_G1_PIZZA", [p["patternId"] for p in self.document["patterns"]])
        self.assertEqual("DRAFT", self.draft(self.document)["authoringStatus"])
        self.assertEqual({"maximum": 100, "clownHoldMs": 15000}, self.document["madnessPolicy"])
        self.assertEqual(
            {"월드_룰렛": "world.sequence.instance.8", "월드_커튼": "world.sequence.instance.curtain_drop"},
            {row["displayName"]: row["sequenceInstanceId"] for row in self.document["worlds"]},
        )
        self.assertEqual(
            {"씬프로필_암전": "scene.kakulsaydon.find-true-dark.v1"},
            {row["displayName"]: row["renderingProfileId"] for row in self.document["sceneProfiles"]},
        )
        roulette = self.find(self.document, ROULETTE_ID)
        self.assertEqual(4, len(roulette["logicOccurrences"]))
        self.assertEqual(1, len(roulette["worldOccurrences"]))
        self.assertEqual(1, len(self.find(self.document, DANCE_ID)["worldOccurrences"]))
        self.assertEqual(1, len(self.find(self.document, GAZE_ID)["sceneProfileOccurrences"]))
        resources = {row["resourceId"]: row for row in self.document["presentationResources"]}
        for pattern_id in (GAZE_ID, DANCE_ID, ROULETTE_ID):
            with self.subTest(pattern=pattern_id):
                pattern = self.find(self.document, pattern_id)
                self.assertEqual(1, len(pattern["sceneProfileOccurrences"]))
                scene = pattern["sceneProfileOccurrences"][0]
                lights = [row for row in pattern["presentationOccurrences"]
                          if resources[row["resourceId"]]["kind"] == "LIGHT"]
                self.assertEqual(2, len(lights))
                self.assertEqual({("light.runtime.1", "PLAYER"), ("light.runtime.2", "BOSS")},
                                 {(resources[row["resourceId"]]["assetId"], row["anchorKind"]) for row in lights})
                for row in lights:
                    self.assertEqual((scene["startMs"], scene["durationMs"]),
                                     (row["startMs"], row["durationMs"]))
                    self.assertTrue(row["followBoss"])

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
        duplicate["patterns"].append(copy.deepcopy(self.first_product(duplicate)))
        duplicate["playAllPatternIds"].append(FIRST_PRODUCT_ID)
        with self.assertRaisesRegex(subject.CompositionError, "duplicate patternId"):
            self.validate(duplicate)

        pattern_counter = copy.deepcopy(self.document)
        ahead_id = f"KAKULSAYDON_G1_PATTERN_{pattern_counter['nextPatternOrdinal']}"
        self.draft(pattern_counter)["patternId"] = ahead_id
        with self.assertRaisesRegex(subject.CompositionError, "nextPatternOrdinal"):
            self.validate(pattern_counter)

        occurrence_counter = copy.deepcopy(self.document)
        self.first_product(occurrence_counter)["nextAnimationOrdinal"] = 2
        with self.assertRaisesRegex(subject.CompositionError, "nextAnimationOrdinal"):
            self.validate(occurrence_counter)

    def test_draft_may_be_empty_but_never_projects_or_replaces_product_inventory(self):
        document = copy.deepcopy(self.document)
        draft = self.strip_lanes(copy.deepcopy(self.first_product(document)))
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
        self.assertEqual(self.document["playAllPatternIds"], encounter["playAllPatternIds"])
        self.assertEqual(len(self.document["playAllPatternIds"]), len(encounter["patterns"]))
        self.assertEqual(self.product_binding_count(self.document), len(presentation["bindings"]))

        for pattern in self.product_patterns(document):
            pattern["authoringStatus"] = "DRAFT"
        document["playAllPatternIds"] = []
        self.validate(document)
        with self.assertRaisesRegex(subject.CompositionError, "at least one PRODUCT"):
            subject.validate_publishable(document)

    def test_product_inventory_is_bounded_to_downstream_catalog_capacity(self):
        document = copy.deepcopy(self.document)
        seed = self.strip_lanes(copy.deepcopy(self.first_product(document)))
        document["patterns"] = []
        document["playAllPatternIds"] = []
        for ordinal in range(1, subject.MAX_PRODUCT_PATTERNS + 2):
            pattern = copy.deepcopy(seed)
            pattern_id = f"KAKULSAYDON_G1_PATTERN_{ordinal}"
            pattern["patternId"] = pattern_id
            pattern["displayName"] = f"Product {ordinal}"
            pattern["nextStageOrdinal"] = len(seed["stages"]) + 1
            pattern["nextAnimationOrdinal"] = len(seed["stages"]) + 1
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
        duplicate = self.strip_lanes(copy.deepcopy(self.first_product(document)))
        duplicate["patternId"] = "KAKULSAYDON.G1.PATTERN.1"
        duplicate["displayName"] = "Derived action collision"
        for ordinal, stage in enumerate(duplicate["stages"], 1):
            stage["actionId"] = f"kakulsaydon.g1.collision.stage.{ordinal}"
            stage["animationOccurrences"][0]["occurrenceId"] = (
                f"KAKULSAYDON.G1.PATTERN.1.animation.{ordinal}"
            )
        document["patterns"].append(duplicate)
        document["playAllPatternIds"].append("KAKULSAYDON.G1.PATTERN.1")

        with self.assertRaisesRegex(subject.CompositionError, "duplicate actionId"):
            self.validate(document)

    def test_normal_patterns_are_draft_only_in_the_animation_mvp(self):
        document = copy.deepcopy(self.document)
        self.first_product(document)["category"] = "NORMAL"
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
        self.first_product(missing)["stages"][0]["animationOccurrences"] = []
        with self.assertRaisesRegex(subject.CompositionError, "exactly one"):
            self.validate(missing)

        offset = copy.deepcopy(self.document)
        occurrence = self.first_product(offset)["stages"][0]["animationOccurrences"][0]
        occurrence["startOffsetMs"] = 1
        occurrence["playMs"] -= 1
        with self.assertRaisesRegex(subject.CompositionError, "whole stage"):
            self.validate(offset)

    def test_product_rejects_presentation_policies_the_client_cannot_run(self):
        variants = []
        source_offset = copy.deepcopy(self.document)
        stage = self.first_product(source_offset)["stages"][0]
        stage["durationMs"] = 100
        stage["animationOccurrences"][0]["playMs"] = 100
        stage["animationOccurrences"][0]["sourceStartMs"] = 1
        variants.append(source_offset)

        slow_rate = copy.deepcopy(self.document)
        stage = self.first_product(slow_rate)["stages"][0]
        stage["durationMs"] = 100
        stage["animationOccurrences"][0]["playMs"] = 100
        stage["animationOccurrences"][0]["playRate"] = 0.05
        variants.append(slow_rate)

        hold = copy.deepcopy(self.document)
        stage = self.first_product(hold)["stages"][0]
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
            ("runtimeClip", "rpct00_idle_battle_1"),
            ("sourceSlotId", "animation-999"),
        ):
            document = copy.deepcopy(self.document)
            self.first_product(document)["stages"][0]["animationOccurrences"][0][key] = value
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
        draft_index = next(
            index for index, pattern in enumerate(self.document["patterns"])
            if pattern["patternId"] == DRAFT_ID
        )
        for profile_id, action_id in (("MN_RPCT_05", 4219811), ("MN_RPCT_06", None),
                                      ("MN_RPCT_07", None), ("MN_RPCZ_00", 0)):
            with self.subTest(profile=profile_id):
                document = copy.deepcopy(self.document)
                pattern = self.draft(document)
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
                others = [p for i, p in enumerate(self.document["patterns"]) if i != draft_index]
                self.assertEqual(others, [p for i, p in enumerate(document["patterns"]) if i != draft_index])

    def test_real_249_stage_saydon_action_fits_draft_but_product_stays_bounded(self):
        document = copy.deepcopy(self.document)
        draft = self.draft(document)
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
        product = self.first_product(document)
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
        pattern = self.draft(document)
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

    def test_actor_without_arena_boss_body_cannot_publish(self):
        # Pure validation accepts any physical actor; publish joins the actor body
        # against Data/Actors/BossCatalog.json, so a body no arena boss presents
        # on is refused only there.
        document = copy.deepcopy(self.document)
        document["patterns"] = [self.strip_lanes(self.first_product(document))]
        document["playAllPatternIds"] = [FIRST_PRODUCT_ID]
        self.validate(document)
        catalog = subject.load_json(ROOT / subject.BOSS_CATALOG_PATH)
        self.assertIn("MN_RPCT_05", subject.arena_boss_archetypes_by_profile(ROOT))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            copy_repository_inputs(root)
            catalog_path = root / subject.BOSS_CATALOG_PATH
            catalog["bosses"] = [
                boss for boss in catalog["bosses"]
                if boss["archetypeId"] == subject.BOSS_ARCHETYPE_ID
            ]
            catalog_path.write_text(json.dumps(catalog), encoding="utf-8")
            self.assertEqual(
                {"MN_RPCZ_00": [subject.BOSS_ARCHETYPE_ID]},
                subject.arena_boss_archetypes_by_profile(root),
            )
            with self.assertRaisesRegex(subject.CompositionError, "PRODUCT actorProfileId"):
                subject.validate_publishable(document, root)
            encounter = subject.project_encounter(document, root)
            self.assertEqual([], encounter["patterns"][0]["bossArchetypeIds"])
        encounter = subject.project_encounter(document, ROOT)
        self.assertEqual(
            subject.arena_boss_archetypes_by_profile(ROOT)["MN_RPCT_05"],
            encounter["patterns"][0]["bossArchetypeIds"],
        )

    def test_legacy_owner_derivation_accepts_alias_and_rejects_mixed_models(self):
        document = copy.deepcopy(self.document)
        draft = self.draft(document)
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
        del self.draft(document)["actorProfileId"]
        with self.assertRaises(subject.CompositionError):
            self.validate(document)

    def test_referenced_action_zero_is_preserved_in_product_source_ids(self):
        document = copy.deepcopy(self.document)
        draft = self.draft(document)
        source_stage = self.append_reference_sequence(draft, "MN_RPCT_05", 0)
        row = copy.deepcopy(source_stage["animationOccurrences"][0])
        product = self.first_product(document)
        stage = product["stages"][0]
        row["occurrenceId"] = stage["animationOccurrences"][0]["occurrenceId"]
        row["startOffsetMs"] = 0
        row["endPolicy"] = "EXACT"
        stage["animationOccurrences"] = [row]
        stage["durationMs"] = row["playMs"]
        draft["stages"].clear()
        self.validate(document)
        self.assertEqual(
            [0] + sorted({s["animationOccurrences"][0]["sourceActionId"] for s in product["stages"][1:]}),
            subject.project_encounter(document)["patterns"][0]["sourceActionIds"],
        )
        row["referenceRevision"] = ""
        with self.assertRaisesRegex(subject.CompositionError, "lowercase SHA-256"):
            self.validate(document)

    def test_raw_stage_cannot_claim_a_nonzero_reference_action(self):
        document = copy.deepcopy(self.document)
        row = self.first_product(document)["stages"][0]["animationOccurrences"][0]
        row["sourceStageId"] = "RAW"
        with self.assertRaisesRegex(subject.CompositionError, "RAW clips must use sourceActionId 0"):
            self.validate(document)

    def test_raw_model_clips_publish_without_extracted_action_metadata(self):
        document = copy.deepcopy(self.document)
        for stage in self.first_product(document)["stages"]:
            row = stage["animationOccurrences"][0]
            row.update(sourceActionId=0, sourceStageId="RAW",
                       sourceSlotId=row["runtimeClip"], referenceRevision="")
        self.validate(document)
        self.assertEqual([], subject.project_encounter(document)["patterns"][0]["sourceActionIds"])
        self.assertEqual(
            self.product_binding_count(self.document),
            len(subject.project_presentation(document)["bindings"]),
        )

    def test_bad_draft_does_not_block_product_publish(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / subject.SOURCE_PATH
            source.parent.mkdir(parents=True)
            copy_repository_inputs(root)
            document = copy.deepcopy(self.document)
            document["patterns"] = [
                {"authoringStatus": "DRAFT", "stages": "broken"} if p["patternId"] == DRAFT_ID else p
                for p in document["patterns"]
            ]
            source.write_text(json.dumps(document), encoding="utf-8")
            result = subject.run(root, "publish")
            self.assertEqual(2, result["outputCount"])
            before = (root / subject.ENCOUNTER_PATH).read_bytes()
            self.first_product(document)["stages"][0]["animationOccurrences"] = []
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
                "madnessPolicy",
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
                "lightResourceRevision",
                "bindings", "patterns",
            },
            set(presentation),
        )
        self.assertEqual(self.product_binding_count(self.document), len(presentation["bindings"]))

    @classmethod
    def without_catalog_boxes(cls, document):
        """Drop live lane boxes so a test-owned catalog stays self-consistent."""
        for pattern in document["patterns"]:
            cls.strip_lanes(pattern)
        return document

    def test_logic_catalog_is_optional_on_read_and_untyped_logic_never_reaches_product(self):
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
        baseline = self.without_catalog_boxes(copy.deepcopy(self.document))
        self.assertEqual(
            subject.project_encounter(baseline),
            subject.project_encounter(copy.deepcopy(document)),
        )

        product = self.first_product(document)
        product["nextLogicOccurrenceOrdinal"] = 2
        product["presentationOccurrences"] = []
        product["logicOccurrences"] = [
            {
                "occurrenceId": f"{FIRST_PRODUCT_ID}.logic.1",
                "logicId": "kakulsaydon.g1.logic.1",
                "startMs": 0,
                "durationMs": 1000,
            }
        ]
        with self.assertRaisesRegex(subject.CompositionError, "without a judgementKind"):
            self.validate(copy.deepcopy(document))

        draft = copy.deepcopy(document)
        for pattern in self.product_patterns(draft):
            pattern["authoringStatus"] = "DRAFT"
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
        self.first_product(dangling)["logicOccurrences"][0]["logicId"] = "kakulsaydon.g1.logic.9"
        with self.assertRaisesRegex(subject.CompositionError, "unknown logicId"):
            self.validate(dangling)
        foreign_box = copy.deepcopy(draft)
        self.first_product(foreign_box)["logicOccurrences"][0]["occurrenceId"] = f"{FIRST_PRODUCT_ID}.animation.9"
        with self.assertRaisesRegex(subject.CompositionError, r"\.logic\.<N>"):
            self.validate(foreign_box)

    def draft_with_logic_box(self):
        document = copy.deepcopy(self.document)
        pattern = self.strip_lanes(self.first_product(document))
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
        pattern["presentationOccurrences"] = []
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
        duplicate_pattern = self.first_product(duplicate)
        duplicate_pattern["logicOccurrences"].append(
            copy.deepcopy(duplicate_pattern["logicOccurrences"][0])
        )
        with self.assertRaisesRegex(subject.CompositionError, "duplicate Logic occurrenceId"):
            self.validate(duplicate)

        for ordinal in (0, 2, subject.MAX_ORDINAL):
            with self.subTest(ordinal=ordinal):
                invalid = copy.deepcopy(document)
                self.first_product(invalid)["logicOccurrences"][0]["occurrenceId"] = (
                    pattern["patternId"] + f".logic.{ordinal}"
                )
                with self.assertRaises(subject.CompositionError):
                    self.validate(invalid)
        for counter in (0, 1, subject.MAX_ORDINAL + 1):
            with self.subTest(counter=counter):
                invalid = copy.deepcopy(document)
                self.first_product(invalid)["nextLogicOccurrenceOrdinal"] = counter
                with self.assertRaisesRegex(subject.CompositionError, "nextLogicOccurrenceOrdinal"):
                    self.validate(invalid)

    def test_logic_windows_use_pattern_lifetime_and_bounded_stage_clock(self):
        document, pattern = self.draft_with_logic_box()
        lifetime_ms = sum(stage["durationMs"] for stage in pattern["stages"])
        pattern["logicOccurrences"][0].update(startMs=lifetime_ms - 1, durationMs=1)
        self.validate(copy.deepcopy(document))
        beyond_lifetime = copy.deepcopy(document)
        self.first_product(beyond_lifetime)["logicOccurrences"][0]["durationMs"] = 2
        with self.assertRaisesRegex(subject.CompositionError, "Pattern lifetime"):
            self.validate(beyond_lifetime)

        pattern["stages"] = pattern["stages"][:1]
        pattern["stages"][0]["durationMs"] = subject.MAX_TIMELINE_MS
        pattern["logicOccurrences"][0].update(
            startMs=subject.MAX_TIMELINE_MS - 1, durationMs=1
        )
        self.validate(copy.deepcopy(document))
        beyond_limit = copy.deepcopy(document)
        self.first_product(beyond_limit)["logicOccurrences"][0]["durationMs"] = 2
        with self.assertRaisesRegex(subject.CompositionError, "Logic box exceeds 600 seconds"):
            self.validate(beyond_limit)

        oversized, oversized_pattern = self.draft_with_logic_box()
        oversized_pattern["stages"][0]["durationMs"] = subject.MAX_TIMELINE_MS
        with self.assertRaisesRegex(subject.CompositionError, "Pattern exceeds 600 seconds"):
            self.validate(oversized)
        empty = copy.deepcopy(document)
        self.first_product(empty)["stages"] = []
        with self.assertRaisesRegex(subject.CompositionError, "Pattern lifetime"):
            self.validate(empty)

    def test_logic_occurrence_capacity_matches_client_1024_boxes(self):
        document, pattern = self.draft_with_logic_box()
        pattern["presentationOccurrences"] = []
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

    def test_legacy_singular_outcomes_name_result_logics_on_duration_boxes_only(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        document["nextLogicOrdinal"] = 4
        document["logics"] = [
            {"logicId": "kakulsaydon.g1.logic.1", "displayName": "방패 무력화", "logicType": "DURATION"},
            {"logicId": "kakulsaydon.g1.logic.2", "displayName": "그로기 패턴", "logicType": "RESULT"},
            {"logicId": "kakulsaydon.g1.logic.3", "displayName": "전원 전멸", "logicType": "RESULT"},
        ]
        for pattern in self.product_patterns(document):
            pattern["authoringStatus"] = "DRAFT"
        document["playAllPatternIds"] = []
        product = self.first_product(document)
        product["nextLogicOccurrenceOrdinal"] = 2
        product["presentationOccurrences"] = []
        product["logicOccurrences"] = [
            {
                "occurrenceId": f"{FIRST_PRODUCT_ID}.logic.1",
                "logicId": "kakulsaydon.g1.logic.1",
                "startMs": 0,
                "durationMs": 1000,
                "onSuccessLogicId": "kakulsaydon.g1.logic.2",
                "onTimeoutLogicId": "kakulsaydon.g1.logic.3",
            }
        ]
        self.validate(copy.deepcopy(document))
        box = product["logicOccurrences"][0]
        self.assertEqual(["kakulsaydon.g1.logic.2"], subject.outcome_logic_ids(box, "Success"))
        self.assertEqual([], subject.outcome_logic_ids(box, "Fail"))
        self.assertEqual(["kakulsaydon.g1.logic.3"], subject.outcome_logic_ids(box, "Timeout"))

        unwired = copy.deepcopy(document)
        box = self.first_product(unwired)["logicOccurrences"][0]
        del box["onSuccessLogicId"]
        box["onTimeoutLogicId"] = ""
        self.validate(unwired)

        not_result = copy.deepcopy(document)
        self.first_product(not_result)["logicOccurrences"][0]["onSuccessLogicId"] = "kakulsaydon.g1.logic.1"
        with self.assertRaisesRegex(subject.CompositionError, "RESULT logic"):
            self.validate(not_result)
        dangling = copy.deepcopy(document)
        self.first_product(dangling)["logicOccurrences"][0]["onTimeoutLogicId"] = "kakulsaydon.g1.logic.9"
        with self.assertRaisesRegex(subject.CompositionError, "RESULT logic"):
            self.validate(dangling)
        on_result_box = copy.deepcopy(document)
        self.first_product(on_result_box)["logicOccurrences"][0]["logicId"] = "kakulsaydon.g1.logic.2"
        with self.assertRaisesRegex(subject.CompositionError, "DURATION logic box"):
            self.validate(on_result_box)
        wrong_type = copy.deepcopy(document)
        self.first_product(wrong_type)["logicOccurrences"][0]["onSuccessLogicId"] = 7
        with self.assertRaisesRegex(subject.CompositionError, "must be text"):
            self.validate(wrong_type)
        wrong_list = copy.deepcopy(document)
        self.first_product(wrong_list)["logicOccurrences"][0]["onFailLogicIds"] = "kakulsaydon.g1.logic.3"
        with self.assertRaisesRegex(subject.CompositionError, "list of text"):
            self.validate(wrong_list)

    def test_typed_logic_definitions_follow_their_kind(self):
        logics = {logic["logicId"]: logic for logic in self.document["logics"]}
        self.assertEqual("STAGGER_WINDOW", logics["kakulsaydon.g1.logic.1"]["judgementKind"])
        self.assertEqual(1000, logics["kakulsaydon.g1.logic.1"]["threshold"])
        self.assertEqual("FOLLOWUP_PATTERN", logics["kakulsaydon.g1.logic.7"]["outcomeKind"])
        self.assertEqual("KAKULSAYDON_G1_PATTERN_4", logics["kakulsaydon.g1.logic.7"]["followupPatternId"])
        self.assertEqual(
            [1, 2, 3, 0],
            [logics[f"kakulsaydon.g1.logic.{n}"]["poseIndex"] for n in (10, 11, 12, 13)],
        )
        self.assertEqual(8, len(logics["kakulsaydon.g1.logic.14"]["regionIds"]))
        self.assertEqual("TRIGGER", logics["kakulsaydon.g1.logic.9"]["logicType"])

        def mutate(logic_id, **changes):
            document = copy.deepcopy(self.document)
            logic = next(row for row in document["logics"] if row["logicId"] == logic_id)
            for key, value in changes.items():
                if value is None:
                    logic.pop(key, None)
                else:
                    logic[key] = value
            return document

        rejected = {
            "unknown judgement kind": mutate("kakulsaydon.g1.logic.1", judgementKind="COIN_FLIP"),
            "values of another kind": mutate("kakulsaydon.g1.logic.1", poseIndex=1),
            "judgement values on a RESULT": mutate("kakulsaydon.g1.logic.2", threshold=5),
            "outcome values on a DURATION": mutate("kakulsaydon.g1.logic.1", percent=5),
            "unknown outcome kind": mutate("kakulsaydon.g1.logic.2", outcomeKind="HEAL"),
            "percent outcome without percent": mutate("kakulsaydon.g1.logic.4", percent=0),
            "percent on a death outcome": mutate("kakulsaydon.g1.logic.2", percent=10),
            "duration on a damage outcome": mutate("kakulsaydon.g1.logic.4", durationMs=100),
            "follow-up without a target": mutate("kakulsaydon.g1.logic.7", followupPatternId=""),
            "target on a non follow-up": mutate("kakulsaydon.g1.logic.2", followupPatternId="KAKULSAYDON_G1_PATTERN_4"),
            "follow-up naming an unknown pattern": mutate("kakulsaydon.g1.logic.7", followupPatternId="KAKULSAYDON_G1_PATTERN_99"),
            "pose index past the four poses": mutate("kakulsaydon.g1.logic.10", poseIndex=4),
            "duplicate region ID": mutate("kakulsaydon.g1.logic.14", regionIds=["same"] * 8),
            "invalid region ID": mutate("kakulsaydon.g1.logic.14", regionIds=["has space"]),
            "gaze cone wider than a half turn": mutate("kakulsaydon.g1.logic.16", halfAngleDegrees=181),
            "trigger with values": mutate("kakulsaydon.g1.logic.9", threshold=1),
        }
        for label, document in rejected.items():
            with self.subTest(label=label):
                with self.assertRaises(subject.CompositionError):
                    self.validate(document)
        # A typed value may be dropped again on a DRAFT-only definition.
        untyped = mutate("kakulsaydon.g1.logic.16", judgementKind=None, halfAngleDegrees=None, maxDistanceM=None)
        with self.assertRaisesRegex(subject.CompositionError, "without a judgementKind"):
            self.validate(untyped)
        self.find(untyped, GAZE_ID)["authoringStatus"] = "DRAFT"
        untyped["playAllPatternIds"].remove(GAZE_ID)
        self.validate(untyped)

    def test_outcome_slots_follow_kind_rules_and_capacity(self):
        document = copy.deepcopy(self.document)
        gaze_box = next(
            box for box in self.find(document, GAZE_ID)["logicOccurrences"]
            if box["logicId"] == "kakulsaydon.g1.logic.16"
        )
        self.assertEqual([], gaze_box["onTimeoutLogicIds"])
        self.assertEqual(["kakulsaydon.g1.logic.3"], gaze_box["onFailLogicIds"])
        stagger_box = self.first_product(document)["logicOccurrences"][0]
        self.assertEqual([], stagger_box["onFailLogicIds"])
        dance_boxes = self.find(document, DANCE_ID)["logicOccurrences"]
        for box in dance_boxes:
            self.assertEqual(box["onTimeoutLogicIds"], box["onFailLogicIds"])
        self.validate(copy.deepcopy(document))

        end_tick_timeout = copy.deepcopy(document)
        next(
            box for box in self.find(end_tick_timeout, GAZE_ID)["logicOccurrences"]
            if box["logicId"] == "kakulsaydon.g1.logic.16"
        )["onTimeoutLogicIds"] = ["kakulsaydon.g1.logic.3"]
        with self.assertRaisesRegex(subject.CompositionError, "no Timeout slot"):
            self.validate(end_tick_timeout)
        stagger_fail = copy.deepcopy(document)
        self.first_product(stagger_fail)["logicOccurrences"][0]["onFailLogicIds"] = ["kakulsaydon.g1.logic.2"]
        with self.assertRaisesRegex(subject.CompositionError, "no Fail slot"):
            self.validate(stagger_fail)
        too_many = copy.deepcopy(document)
        self.find(too_many, DANCE_ID)["logicOccurrences"][0]["onTimeoutLogicIds"] = [
            "kakulsaydon.g1.logic.4", "kakulsaydon.g1.logic.5", "kakulsaydon.g1.logic.6",
            "kakulsaydon.g1.logic.15", "kakulsaydon.g1.logic.8",
        ]
        with self.assertRaisesRegex(subject.CompositionError, "more than 4"):
            self.validate(too_many)
        followup_on_dance = copy.deepcopy(document)
        self.find(followup_on_dance, DANCE_ID)["logicOccurrences"][0]["onTimeoutLogicIds"] = ["kakulsaydon.g1.logic.7"]
        with self.assertRaisesRegex(subject.CompositionError, "FOLLOWUP_PATTERN is only valid"):
            self.validate(followup_on_dance)
        stacked = copy.deepcopy(document)
        self.find(stacked, DANCE_ID)["logicOccurrences"][0]["onTimeoutLogicIds"] = [
            "kakulsaydon.g1.logic.15", "kakulsaydon.g1.logic.5",
        ]
        self.validate(copy.deepcopy(stacked))
        window = subject.project_encounter(stacked)["patterns"][4]["logicWindows"][0]
        self.assertEqual(
            [("MADNESS_GAUGE_ADD_PERCENT", 50), ("MAX_HP_PERCENT_DAMAGE", 10)],
            [(row["kind"], row["percent"]) for row in window["onTimeout"]],
        )
        # A follow-up the Server starts must itself be a published Product.
        draft_followup = copy.deepcopy(document)
        self.find(draft_followup, "KAKULSAYDON_G1_PATTERN_4")["authoringStatus"] = "DRAFT"
        draft_followup["playAllPatternIds"].remove("KAKULSAYDON_G1_PATTERN_4")
        self.validate(copy.deepcopy(draft_followup))
        with self.assertRaisesRegex(subject.CompositionError, "must name a PRODUCT pattern"):
            subject.validate_publishable(draft_followup, ROOT)

    def test_summon_and_trigger_boxes_are_authoring_only_on_product(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        # Untyped legacy TRIGGERs remain authoring-only; typed triggers project.
        trigger = next(row for row in document["logics"] if row["logicId"] == "kakulsaydon.g1.logic.9")
        for key in subject.LOGIC_TRIGGER_VALUE_KEYS:
            trigger.pop(key, None)
        document["nextSummonOrdinal"] = 2
        document["summons"] = [
            {"summonId": "kakulsaydon.g1.summon.1", "displayName": "가짜 세이튼 3"}
        ]
        self.validate(copy.deepcopy(document))
        baseline = subject.project_encounter(self.without_catalog_boxes(copy.deepcopy(self.document)))
        product = self.first_product(document)
        product["nextSummonOccurrenceOrdinal"] = 2
        product["summonOccurrences"] = [
            {
                "occurrenceId": f"{FIRST_PRODUCT_ID}.summon.1",
                "summonId": "kakulsaydon.g1.summon.1",
                "startMs": 0,
                "durationMs": 16334,
            }
        ]
        product["nextLogicOccurrenceOrdinal"] = 2
        product["presentationOccurrences"] = []
        product["logicOccurrences"] = [
            {
                "occurrenceId": f"{FIRST_PRODUCT_ID}.logic.1",
                "logicId": "kakulsaydon.g1.logic.9",
                "startMs": 0,
                "durationMs": 1000,
            }
        ]
        self.validate(copy.deepcopy(document))
        self.assertEqual(baseline, subject.project_encounter(copy.deepcopy(document)))

        ahead = copy.deepcopy(document)
        ahead["nextSummonOrdinal"] = 1
        with self.assertRaisesRegex(subject.CompositionError, "nextSummonOrdinal"):
            self.validate(ahead)
        dangling = copy.deepcopy(document)
        self.first_product(dangling)["summonOccurrences"][0]["summonId"] = "kakulsaydon.g1.summon.9"
        with self.assertRaisesRegex(subject.CompositionError, "unknown summonId"):
            self.validate(dangling)
        foreign_box = copy.deepcopy(document)
        self.first_product(foreign_box)["summonOccurrences"][0]["occurrenceId"] = f"{FIRST_PRODUCT_ID}.logic.9"
        with self.assertRaisesRegex(subject.CompositionError, r"\.summon\.<N>"):
            self.validate(foreign_box)
        typed = copy.deepcopy(document)
        typed["summons"][0]["summonType"] = "SPAWN"
        with self.assertRaisesRegex(subject.CompositionError, "summons\\[0\\]"):
            self.validate(typed)

    def test_product_world_state_requires_a_resolved_model_resource(self):
        document = copy.deepcopy(self.document)
        sequences = subject.load_world_sequences(ROOT, subject.AREA_ID)
        world_box = self.find(document, DANCE_ID)["worldOccurrences"][0]
        world = next(row for row in document["worlds"] if row["worldId"] == world_box["worldId"])
        instance = next(row for row in sequences["instances"] if row["instanceId"] == world["sequenceInstanceId"])
        instance["bindings"] = [{"slotId": "object", "targetKind": "OBJECT_RESOURCE", "targetId": "missing.object"}]
        with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
            with self.assertRaisesRegex(subject.CompositionError, "unresolved model resource"):
                subject.project_encounter(document)
        sequences.setdefault("objectResources", []).append({"objectId": "missing.object", "modelAssetId": "Map/Test/card.wmodel"})
        with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
            result = subject.project_encounter(document)
        projected = next(row for row in result["patterns"] if row["patternId"] == DANCE_ID)
        self.assertEqual(world_box["durationMs"], projected["worldSequences"][0]["durationMs"])

    def test_world_and_scene_profile_lanes_validate_and_project(self):
        document = copy.deepcopy(self.document)
        roulette = self.find(document, ROULETTE_ID)
        world_box = roulette["worldOccurrences"][0]
        world_definition = next(w for w in document["worlds"] if w["worldId"] == world_box["worldId"])
        encounter = subject.project_encounter(document)
        projected = next(p for p in encounter["patterns"] if p["patternId"] == ROULETTE_ID)
        self.assertEqual(
            [{"sequenceInstanceId": "world.sequence.instance.8", "startMs": world_box["startMs"],
              "durationMs": world_box["durationMs"],
              "playbackSpeed": world_box["playbackSpeed"], "positionOffset": world_definition.get("positionOffset", [0, 0, 0]),
              "anchorKind": world_definition.get("anchorKind", "NONE"),
              "anchorPosition": world_definition.get("anchorPosition", [0, 0, 0])}],
            projected["worldSequences"],
        )
        self.assertTrue(all(box.get("enabled", True) for box in roulette["logicOccurrences"]))
        self.assertEqual([8,8,8,1], [len(window["cardRegions"]) for window in projected["logicWindows"]])
        pairs = {(symbol,color) for symbol in subject.CARD_SYMBOLS for color in ("RED","BLACK")}
        for window in projected["logicWindows"][:3]:
            self.assertEqual(pairs, {(r["cardSymbol"],r["cardColor"]) for r in window["cardRegions"]})
            self.assertTrue(all(abs(r["radiusM"]-8.0)<0.001 and r["anchorKind"] == "BOSS_SPAWN" for r in window["cardRegions"]))
        gaze = next(p for p in encounter["patterns"] if p["patternId"] == GAZE_ID)
        self.assertEqual("scene.kakulsaydon.find-true-dark.v1", gaze["sceneProfiles"][0]["renderingProfileId"])
        dance = next(p for p in encounter["patterns"] if p["patternId"] == DANCE_ID)
        self.assertEqual("world.sequence.instance.curtain_drop",dance["worldSequences"][0]["sequenceInstanceId"])
        # A partial mapping fails projection and leaves the previous Product intact.
        unmapped = copy.deepcopy(document)
        next(l for l in unmapped["logics"] if l["logicId"] == "kakulsaydon.g1.logic.14")["regionIds"].pop()
        with self.assertRaisesRegex(subject.CompositionError,"eight explicit"):
            subject.project_encounter(unmapped)
        duplicated = copy.deepcopy(document)
        cards = self.find(duplicated,ROULETTE_ID)["presentationOccurrences"]
        cards[1]["cardSymbol"],cards[1]["cardColor"] = cards[0]["cardSymbol"],cards[0]["cardColor"]
        with self.assertRaisesRegex(subject.CompositionError,"eight distinct"):
            subject.project_encounter(duplicated)

        rejected = {}
        no_world = copy.deepcopy(document)
        self.find(no_world, ROULETTE_ID)["worldOccurrences"] = []
        with self.assertRaisesRegex(subject.CompositionError, "WORLD"):
            subject.project_encounter(no_world)
        twice = copy.deepcopy(document)
        twice_roulette = self.find(twice, ROULETTE_ID)
        second = copy.deepcopy(twice_roulette["worldOccurrences"][0])
        second["occurrenceId"] = f"{ROULETTE_ID}.world.2"
        twice_roulette["worldOccurrences"].append(second)
        twice_roulette["nextWorldOccurrenceOrdinal"] = 3
        rejected["same instance from two boxes"] = (twice, "from two boxes")
        late = copy.deepcopy(document)
        self.find(late, ROULETTE_ID)["worldOccurrences"][0]["startMs"] = 40000
        rejected["world box after the pattern"] = (late, "after the Pattern lifetime")
        slow = copy.deepcopy(document)
        self.find(slow, ROULETTE_ID)["worldOccurrences"][0]["playbackSpeed"] = 0.01
        rejected["playback speed below the floor"] = (slow, "playbackSpeed")
        unknown_world = copy.deepcopy(document)
        self.find(unknown_world, ROULETTE_ID)["worldOccurrences"][0]["worldId"] = "kakulsaydon.g1.world.9"
        rejected["unknown world"] = (unknown_world, "unknown worldId")
        long_scene = copy.deepcopy(document)
        self.find(long_scene, GAZE_ID)["sceneProfileOccurrences"][0]["durationMs"] += 1
        rejected["scene profile past the pattern"] = (long_scene, "exceeds the Pattern lifetime")
        unknown_scene = copy.deepcopy(document)
        self.find(unknown_scene, GAZE_ID)["sceneProfileOccurrences"][0]["sceneProfileId"] = "kakulsaydon.g1.sceneprofile.9"
        rejected["unknown scene profile"] = (unknown_scene, "unknown sceneProfileId")
        world_ahead = copy.deepcopy(document)
        world_ahead["nextWorldOrdinal"] = 1
        rejected["world catalog ahead of its counter"] = (world_ahead, "nextWorldOrdinal")
        bad_profile = copy.deepcopy(document)
        bad_profile["sceneProfiles"][0]["renderingProfileId"] = "has space"
        rejected["scene profile with an unstable rendering id"] = (bad_profile, "renderingProfileId")
        bad_policy = copy.deepcopy(document)
        bad_policy["madnessPolicy"]["maximum"] = 0
        rejected["madness maximum of zero"] = (bad_policy, "madnessPolicy maximum")
        for label, (mutated, message) in rejected.items():
            with self.subTest(label=label):
                with self.assertRaisesRegex(subject.CompositionError, message):
                    self.validate(mutated)

    def test_generic_presentation_resources_project_timed_instances(self):
        document = copy.deepcopy(self.document)
        document["presentationResources"] = []
        for world in document.get("worlds", []):
            world.pop("companionEffectResourceId", None)
        for candidate in document["patterns"]:
            candidate["presentationOccurrences"] = []
        pattern = self.first_product(document)
        pattern["presentationOccurrences"] = []
        for ordinal, (kind, asset) in enumerate((
            ("EFFECT", "Effect/V2/example.group.json"), ("SOUND", "Sound/example.wav"),
            ("CAMERA", "camera.shot.example"), ("COLLIDER", "")), 1):
            resource_id = f"kakulsaydon.g1.presentation.{ordinal}"
            document["presentationResources"].append({
                "resourceId": resource_id, "displayName": kind, "kind": kind, "assetId": asset,
            })
            pattern["presentationOccurrences"].append({
                "occurrenceId": f"{pattern['patternId']}.presentation.{ordinal}",
                "resourceId": resource_id, "startMs": 10, "durationMs": 500,
                "fadeInMs": 50, "fadeOutMs": 50, "positionOffset": [1, 2, 3],
            })
        document["nextPresentationResourceOrdinal"] = 5
        pattern["nextPresentationOccurrenceOrdinal"] = 5
        self.validate(copy.deepcopy(document))
        projected = next(p for p in subject.project_presentation(document)["patterns"]
                         if p["patternId"] == pattern["patternId"])
        rows = projected["presentationOccurrences"]
        self.assertEqual(["EFFECT", "SOUND", "CAMERA", "COLLIDER"], [row["kind"] for row in rows])
        self.assertTrue(all(row["durationMs"] == 500 and row["resourceDurationMs"] == 1000 for row in rows))
        self.assertEqual([1, 2, 3], rows[0]["positionOffset"])
        self.assertEqual([1.0, 1.0, 1.0], rows[3]["halfExtents"])
        rejected = copy.deepcopy(document)
        self.first_product(rejected)["presentationOccurrences"][0]["fadeOutMs"] = 500
        with self.assertRaisesRegex(subject.CompositionError, "fades exceed"):
            self.validate(rejected)
        rejected = copy.deepcopy(document)
        rejected["presentationResources"][1]["assetId"] = "Sound/../outside.wav"
        with self.assertRaisesRegex(subject.CompositionError, "Resources-relative"):
            self.validate(rejected)
        scenes = next(p for p in subject.project_presentation(document)["patterns"] if p["patternId"] == GAZE_ID)
        self.assertTrue(any(row["kind"] == "SCENE_PROFILE" and row["fadeInMs"] == 500
                            for row in scenes["presentationOccurrences"]))

    def light_document(self, anchor="PLAYER", asset="light.test.character"):
        document = copy.deepcopy(self.document)
        ordinal = document.get("nextPresentationResourceOrdinal", 1)
        document["nextPresentationResourceOrdinal"] = ordinal + 1
        resource_id = f"kakulsaydon.g1.presentation.{ordinal}"
        document.setdefault("presentationResources", []).append({
            "resourceId": resource_id, "displayName": "Character spotlight", "kind": "LIGHT",
            "assetId": asset, "resourceKind": "", "defaultAnchorKind": anchor, "durationMs": 500,
        })
        pattern = self.first_product(document)
        ordinal = pattern.get("nextPresentationOccurrenceOrdinal", 1)
        pattern["nextPresentationOccurrenceOrdinal"] = ordinal + 1
        box = {"occurrenceId": f"{pattern['patternId']}.presentation.{ordinal}",
               "resourceId": resource_id, "startMs": 0, "durationMs": 500,
               "anchorKind": anchor, "brightnessMultiplier": 2.5}
        pattern.setdefault("presentationOccurrences", []).append(box)
        return document, box

    @staticmethod
    def write_light_catalog(root, revision=1, lights=None):
        path = root / subject.LIGHT_RESOURCES_PATH
        path.parent.mkdir(parents=True, exist_ok=True)
        source = subject.load_json(ROOT / subject.LIGHT_RESOURCES_PATH)
        fixture = copy.deepcopy(next(row for row in source["lights"] if row["defaultAnchorKind"] == "PLAYER"))
        fixture["lightResourceId"] = "light.test.character"
        path.write_bytes(subject.serialize_json({
            "schema": "lostark.light-resources", "formatVersion": 1, "revision": revision,
            "nextLightResourceOrdinal": source["nextLightResourceOrdinal"],
            "lights": [*source["lights"], *(lights if lights is not None else [fixture])],
        }))

    def test_light_anchors_project_without_server_gameplay_rows(self):
        baseline = subject.project_encounter(self.document, ROOT)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.write_light_catalog(root)
            for anchor in ("PLAYER", "BOSS", "MAP"):
                with self.subTest(anchor=anchor):
                    document, box = self.light_document(anchor)
                    self.validate(document)
                    projected = subject.project_presentation(document, root)
                    row = next(row for pattern in projected["patterns"] for row in pattern["presentationOccurrences"]
                               if row["occurrenceId"] == box["occurrenceId"])
                    self.assertEqual(("LIGHT", anchor, 2.5), (row["kind"], row["anchorKind"], row["brightnessMultiplier"]))
                    self.assertEqual(baseline, subject.project_encounter(document, ROOT))

    def test_light_rejects_invalid_anchor_contract_before_projection(self):
        for changes in ({"bone": "bip001-head"}, {"followBoss": False}, {"scale": [2, 1, 1]},
                        {"anchorKind": "WORLD"}, {"brightnessMultiplier": float("nan")},
                        {"brightnessMultiplier": 17}, {"worldId": "world.bad"}):
            with self.subTest(changes=changes):
                document, box = self.light_document()
                box.update(changes)
                with self.assertRaises(subject.CompositionError):
                    self.validate(document)
        document, box = self.light_document()
        document["presentationResources"][-1].update(kind="CAMERA")
        with self.assertRaisesRegex(subject.CompositionError, "anchorKind"):
            self.validate(document)

    def test_light_catalog_changes_rejoin_without_composition_edits(self):
        document, _ = self.light_document()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.write_light_catalog(root, revision=1)
            first = subject.project_presentation(document, root)
            self.write_light_catalog(root, revision=2)
            second = subject.project_presentation(document, root)
            self.assertEqual(first["patterns"], second["patterns"])
            self.assertEqual((1, 2), (first["lightResourceRevision"], second["lightResourceRevision"]))
            self.write_light_catalog(root, revision=3, lights=[])
            with self.assertRaisesRegex(subject.CompositionError, "missing Light Resources"):
                subject.project_presentation(document, root)

    def test_light_map_alias_joins_only_the_declared_area_source(self):
        document, _ = self.light_document("MAP", "light.map.stage")
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.write_light_catalog(root, lights=[])
            map_path = Path(f"Data/Maps/Authoring/{subject.AREA_ID}/{subject.AREA_ID}.maplights.json")
            (root / map_path).parent.mkdir(parents=True, exist_ok=True)
            source = subject.load_json(ROOT / subject.LIGHT_RESOURCES_PATH)["lights"][0]
            map_document = subject.load_json(ROOT / map_path)
            map_document["lights"] = [{**{key: value for key, value in source.items()
                                        if key not in {"lightResourceId", "defaultAnchorKind", "localOffset", "localRotationDegrees"}},
                                      "lightId": "light.map.stage", "groupId": "test", "enabled": True,
                                      "position": source["localOffset"], "rotationDegrees": source["localRotationDegrees"]}]
            (root / map_path).write_bytes(subject.serialize_json(map_document))
            with self.assertRaisesRegex(subject.CompositionError, "missing Light Resources"):
                subject.project_presentation(document, root)
            catalog = {"areas": [{"id": subject.AREA_ID, "sourceLights": map_path.as_posix(),
                                   "lights": f"Client/Bin/DataFiles/Map/{subject.AREA_ID}.maplights.json"}]}
            (root / "Data/Maps/MapCatalog.json").write_bytes(subject.serialize_json(catalog))
            self.assertEqual(1, subject.project_presentation(document, root)["lightResourceRevision"])
            map_document["lights"] = []
            (root / map_path).write_bytes(subject.serialize_json(map_document))
            with self.assertRaisesRegex(subject.CompositionError, "missing Light Resources"):
                subject.project_presentation(document, root)

    def test_boss_spawn_reset_and_world_anchor_project(self):
        document = copy.deepcopy(self.document)
        pattern = self.find(document, ROULETTE_ID)
        pattern["resetBossToSpawn"] = True
        world = next(w for w in document["worlds"] if w["worldId"] == pattern["worldOccurrences"][0]["worldId"])
        world.update(anchorKind="BOSS_SPAWN", anchorPosition=[-.319, 1.9, 737.531], positionOffset=[0, .58, 0])
        self.validate(copy.deepcopy(document))
        projected = next(p for p in subject.project_encounter(document)["patterns"] if p["patternId"] == ROULETTE_ID)
        self.assertTrue(projected["resetBossToSpawn"])
        self.assertEqual("BOSS_SPAWN", projected["worldSequences"][0]["anchorKind"])
        self.assertEqual([-.319, 1.9, 737.531], projected["worldSequences"][0]["anchorPosition"])
        pattern["resetBossToSpawn"] = 1
        with self.assertRaisesRegex(subject.CompositionError, "resetBossToSpawn must be a boolean"):
            self.validate(document)

    def test_publish_is_deterministic_and_validate_detects_stale_product(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / subject.SOURCE_PATH
            source.parent.mkdir(parents=True)
            shutil.copy2(ROOT / subject.SOURCE_PATH, source)
            copy_repository_inputs(root)
            result = subject.run(root, "publish")
            self.assertEqual(2, result["outputCount"])
            expected = subject.projected_outputs(subject.load_and_validate(root), root)
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

    def test_linked_collider_reuses_duration_trigger_and_result_windows(self):
        for logic_type, field, kind in (("DURATION", "judgementKind", "AREA_OVERLAP"),
                                        ("TRIGGER", "triggerKind", "ENTER_AREA")):
            with self.subTest(kind=kind):
                document = copy.deepcopy(self.document)
                pattern = self.strip_lanes(self.first_product(document))
                logic_id = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']}"
                document["nextLogicOrdinal"] += 1
                document["logics"].append({"logicId":logic_id,"displayName":"Geometry judgement","logicType":logic_type,field:kind})
                resource_id = f"kakulsaydon.g1.presentation.{document['nextPresentationResourceOrdinal']}"
                document["nextPresentationResourceOrdinal"] += 1
                document["presentationResources"].append({"resourceId":resource_id,"displayName":"Geometry","kind":"COLLIDER","assetId":"",
                    "shape":"BOX","halfExtents":[2,1,3]})
                box_id = pattern["patternId"]+".logic.1"
                pattern["nextLogicOccurrenceOrdinal"] = 2
                pattern["logicOccurrences"] = [{"occurrenceId":box_id,"logicId":logic_id,"startMs":0,"durationMs":1000,
                    "onSuccessLogicIds":["kakulsaydon.g1.logic.4"],"onTimeoutLogicIds":["kakulsaydon.g1.logic.5"]}]
                pattern["nextPresentationOccurrenceOrdinal"] = 2
                pattern["presentationOccurrences"] = [{"occurrenceId":pattern["patternId"]+".presentation.1","resourceId":resource_id,
                    "startMs":0,"durationMs":1000,"logicOccurrenceId":box_id,"positionOffset":[0,0,5],"rotationDegrees":[0,90,0]}]
                self.validate(document)
                result = next(p for p in subject.project_encounter(document)["patterns"] if p["patternId"] == FIRST_PRODUCT_ID)
                self.assertEqual(1,len(result["logicWindows"]))
                self.assertEqual([],result["mechanicTriggers"])
                window = result["logicWindows"][0]
                self.assertEqual(kind,window["kind"])
                self.assertEqual("BOSS_CURRENT",window["cardRegions"][0]["anchorKind"])
                self.assertEqual([2,1,3],window["cardRegions"][0]["halfExtents"])
                self.assertEqual(1,len(window["onSuccess"]))
                pattern["presentationOccurrences"][0]["durationMs"] = 999
                with self.assertRaisesRegex(subject.CompositionError,"identical timing"):
                    self.validate(document)

    def test_circle_fail_duration_preserves_authored_result_and_debug_render(self):
        document = copy.deepcopy(self.document)
        pattern = self.find(document,ROULETTE_ID)
        circle = next(row for row in pattern["presentationOccurrences"] if row["logicOccurrenceId"] == pattern["logicOccurrences"][-1]["occurrenceId"])
        circle["scale"] = [3,1,1]
        circle["debugRender"] = False
        self.validate(document)
        encounter = next(p for p in subject.project_encounter(document)["patterns"] if p["patternId"] == ROULETTE_ID)
        window = encounter["logicWindows"][-1]
        self.assertEqual(("AREA_OVERLAP","FAIL"),(window["kind"],window["insideOutcome"]))
        self.assertEqual("CIRCLE",window["cardRegions"][0]["shape"])
        self.assertAlmostEqual(24.0,window["cardRegions"][0]["radiusM"],places=3)
        self.assertEqual([50,50],[r["percent"] for r in window["onFail"]])
        flattened = next(p for p in subject.project_presentation(document)["patterns"] if p["patternId"] == ROULETTE_ID)
        self.assertFalse(next(r for r in flattened["presentationOccurrences"] if r["occurrenceId"] == circle["occurrenceId"])["debugRender"])
        invalid = copy.deepcopy(document)
        next(l for l in invalid["logics"] if l["logicId"] == pattern["logicOccurrences"][-1]["logicId"])["insideOutcome"] = "DAMAGE"
        with self.assertRaisesRegex(subject.CompositionError,"insideOutcome"):
            self.validate(invalid)

    def test_world_enter_trigger_projects_real_transform_keys_and_clock(self):
        document = copy.deepcopy(self.document)
        pattern = self.strip_lanes(self.first_product(document))
        logic_id = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']}"
        document["nextLogicOrdinal"] += 1
        document["logics"].append({"logicId":logic_id,"displayName":"WORLD entry","logicType":"TRIGGER","triggerKind":"ENTER_AREA"})
        resource_id = f"kakulsaydon.g1.presentation.{document['nextPresentationResourceOrdinal']}"
        document["nextPresentationResourceOrdinal"] += 1
        document["presentationResources"].append({"resourceId":resource_id,"displayName":"Trigger box","kind":"COLLIDER","assetId":"","resourceKind":"","shape":"BOX"})
        box_id = pattern["patternId"]+".logic.1"
        pattern["nextLogicOccurrenceOrdinal"] = pattern["nextPresentationOccurrenceOrdinal"] = pattern["nextWorldOccurrenceOrdinal"] = 2
        pattern["logicOccurrences"] = [{"occurrenceId":box_id,"logicId":logic_id,"startMs":200,"durationMs":1000,
            "onSuccessLogicIds":["kakulsaydon.g1.logic.4"]}]
        pattern["worldOccurrences"] = [{"occurrenceId":pattern["patternId"]+".world.1","worldId":"kakulsaydon.g1.world.1","startMs":200,"durationMs":1200,"playbackSpeed":2}]
        pattern["presentationOccurrences"] = [{"occurrenceId":pattern["patternId"]+".presentation.1","resourceId":resource_id,"startMs":200,"durationMs":1000,
            "logicOccurrenceId":box_id,"anchorKind":"WORLD","worldId":"kakulsaydon.g1.world.1","positionOffset":[0,0,1],"rotationDegrees":[0,90,0]}]
        self.validate(document)
        sequences = subject.load_world_sequences(ROOT,subject.AREA_ID)
        instance = next(r for r in sequences["instances"] if r["instanceId"] == "world.sequence.instance.8")
        instance["startDelayMs"] = 100
        template = next(r for r in sequences["templates"] if r["sequenceId"] == instance["templateId"])
        template["durationMs"] = 1100
        template["interpolation"] = "SMOOTH_STEP"
        keys = [{"timeMs":0,"positionOffset":[0,0,0],"rotationQuaternion":[0,0,0,1],"scaleMultiplier":[1,1,1],"visible":True},
                {"timeMs":1100,"positionOffset":[10,0,0],"rotationQuaternion":[0,0.7071067811865475,0,0.7071067811865475],"scaleMultiplier":[2,2,2],"visible":True}]
        template["tracks"][0]["keys"] = keys
        with mock.patch.object(subject,"load_world_sequences",return_value=sequences):
            output = next(r for r in subject.project_encounter(document)["patterns"] if r["patternId"] == FIRST_PRODUCT_ID)
        region = output["logicWindows"][0]["cardRegions"][0]
        self.assertEqual([0,0,1],region["center"])
        self.assertEqual(90,region["yawDegrees"])
        track = region["worldTrack"]
        self.assertEqual((200,100,1100,2.0,"SMOOTH_STEP"),(track["startMs"],track["startDelayMs"],track["durationMs"],track["playbackSpeed"],track["interpolation"]))
        self.assertEqual([4,4,4],track["baselineScale"])
        self.assertEqual([10,0,0],track["keys"][1]["positionOffset"])
        self.assertAlmostEqual(0.7071067811865475,track["keys"][1]["rotationY"])
        self.assertEqual("MAX_HP_PERCENT_DAMAGE",output["logicWindows"][0]["onSuccess"][0]["kind"])
        keys[1]["timeMs"] = 0
        with mock.patch.object(subject,"load_world_sequences",return_value=sequences):
            with self.assertRaisesRegex(subject.CompositionError,"increasing"):
                subject.project_encounter(document)

    def test_world_companion_effect_is_explicit_and_keeps_independent_timing(self):
        document = copy.deepcopy(self.document)
        pattern = self.strip_lanes(self.first_product(document))
        resource_id = f"kakulsaydon.g1.presentation.{document['nextPresentationResourceOrdinal']}"
        document["nextPresentationResourceOrdinal"] += 1
        document["presentationResources"].append({"resourceId":resource_id,"displayName":"Curtain companion",
            "kind":"EFFECT","assetId":"boss.kouku.curtain_1","resourceKind":"LEAF"})
        world_id = f"kakulsaydon.g1.world.{document['nextWorldOrdinal']}"
        document["nextWorldOrdinal"] += 1
        document["worlds"].append({"worldId":world_id,"displayName":"Curtain World","sequenceInstanceId":document["worlds"][0]["sequenceInstanceId"],
            "companionEffectResourceId":resource_id})
        world_box_id = pattern["patternId"]+".world.1"
        pattern["nextWorldOccurrenceOrdinal"] = pattern["nextPresentationOccurrenceOrdinal"] = 2
        pattern["worldOccurrences"] = [{"occurrenceId":world_box_id,"worldId":world_id,"startMs":0,"durationMs":1500,"playbackSpeed":1}]
        pattern["presentationOccurrences"] = [{"occurrenceId":pattern["patternId"]+".presentation.1","resourceId":resource_id,
            "startMs":100,"durationMs":900,"worldOccurrenceId":world_box_id}]
        self.validate(document)
        output = next(row for row in subject.project_presentation(document)["patterns"] if row["patternId"] == pattern["patternId"])
        self.assertEqual(1,len(output["presentationOccurrences"]))
        row = output["presentationOccurrences"][0]
        self.assertEqual((100,900,world_box_id),(row["startMs"],row["durationMs"],row["worldOccurrenceId"]))
        detached = copy.deepcopy(document)
        self.first_product(detached)["presentationOccurrences"] = []
        self.validate(detached)  # Existing World boxes are not auto-projected a second time.
        self.assertEqual([],next(row for row in subject.project_presentation(detached)["patterns"] if row["patternId"] == pattern["patternId"])["presentationOccurrences"])
        invalid = copy.deepcopy(document)
        self.first_product(invalid)["presentationOccurrences"][0]["worldOccurrenceId"] = pattern["patternId"]+".world.99"
        with self.assertRaisesRegex(subject.CompositionError,"same-pattern"):
            self.validate(invalid)
        invalid = copy.deepcopy(document)
        invalid["worlds"][-1]["companionEffectResourceId"] = "kakulsaydon.g1.presentation.999999"
        with self.assertRaisesRegex(subject.CompositionError,"EFFECT resource"):
            self.validate(invalid)
        invalid = copy.deepcopy(document)
        target = self.first_product(invalid)
        duplicate = copy.deepcopy(target["presentationOccurrences"][0])
        duplicate["occurrenceId"] = target["patternId"]+".presentation.2"
        target["nextPresentationOccurrenceOrdinal"] = 3
        target["presentationOccurrences"].append(duplicate)
        with self.assertRaisesRegex(subject.CompositionError,"at most one"):
            self.validate(invalid)

    def test_tracked_products_are_current(self):
        expected = subject.projected_outputs(self.document)
        subject.validate_outputs(ROOT, expected)


    def test_typed_trigger_projects_exact_target_and_rejects_missing_clone(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        trigger = next(row for row in document["logics"] if row["logicId"] == "kakulsaydon.g1.logic.9")
        trigger.update(triggerKind="REAL_GAZE_TELEPORT", teleportPosition=[-6.36, 1.3, 937.92],
                       clonePatternId=FIRST_PRODUCT_ID, clockHours=[4, 7, 10])
        product = self.find(document, GAZE_ID)
        product["nextLogicOccurrenceOrdinal"] = 2
        product["presentationOccurrences"] = []
        product["logicOccurrences"] = [{"occurrenceId": GAZE_ID + ".logic.1",
            "logicId": trigger["logicId"], "startMs": 2000, "durationMs": 1000}]
        self.validate(document)
        projected = subject.project_encounter(document)
        row = next(p for p in projected["patterns"] if p["patternId"] == GAZE_ID)["mechanicTriggers"][0]
        self.assertEqual([-6.36, 1.3, 937.92], row["teleportPosition"])
        self.assertEqual([4, 7, 10], row["clockHours"])
        trigger["clonePatternId"] = "KAKULSAYDON_MISSING_CLONE"
        with self.assertRaisesRegex(subject.CompositionError, "clone pattern must be PRODUCT"):
            subject.project_encounter(document)
        trigger["clockHours"] = [5, 5, 10]
        with self.assertRaisesRegex(subject.CompositionError, "three different hours"):
            self.validate(document)

    def test_hud_enter_trigger_projects_mode_on_authored_start_tick(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        trigger = next(row for row in document["logics"] if row["logicId"] == "kakulsaydon.g1.logic.9")
        for key in subject.LOGIC_TRIGGER_VALUE_KEYS:
            trigger.pop(key, None)
        trigger.update(triggerKind="HUD_ENTER", hudMode="MAZE")
        product = self.find(document, GAZE_ID)
        product["nextLogicOccurrenceOrdinal"] = 2
        product["presentationOccurrences"] = []
        product["logicOccurrences"] = [{"occurrenceId": GAZE_ID + ".logic.1",
            "logicId": trigger["logicId"], "startMs": 1333, "durationMs": 1000}]
        self.validate(document)
        row = next(p for p in subject.project_encounter(document)["patterns"]
                   if p["patternId"] == GAZE_ID)["mechanicTriggers"][0]
        self.assertEqual(("HUD_ENTER", "MAZE", 1333), (row["kind"], row["hudMode"], row["startMs"]))

if __name__ == "__main__":
    unittest.main()
