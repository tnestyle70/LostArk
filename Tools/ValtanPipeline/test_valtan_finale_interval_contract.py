from __future__ import annotations
import copy
import json
import unittest
from pathlib import Path
import valtan_tuning_pipeline as pipeline


class ValtanFinaleIntervalContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root = Path(__file__).resolve().parents[2]
        cls.docs = pipeline.load_pipeline_documents(cls.root)
        cls.master = pipeline.join_v2_authoring(
            cls.docs[pipeline.GAMEPLAY_AUTHORING_REL], cls.docs[pipeline.PRESENTATION_AUTHORING_REL],
            cls.docs[pipeline.WORLD_SET_REL], cls.docs[pipeline.COMBAT_AUTHORING_REL])

    def candidate(self):
        master = copy.deepcopy(self.master)
        pattern = next(row for row in master["patterns"] if row["patternId"] == "VALTAN_GHOST_FINALE")
        pattern["finale"]["ghostPatternIds"] = ["VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH", "VALTAN_SEQUENCE_FOUR", "VALTAN_CROSS"]
        pattern["finale"].update(auxiliarySpawnIntervalMs=5000, portalSpawnIntervalMs=10000)
        return master, pattern

    def validate(self, master, pattern):
        pipeline._validate_finale(pattern, {row["patternId"]: row for row in master["patterns"]}, master["bossArchetypeId"])

    def test_optional_fields_and_bounds(self):
        for fields in ({}, {"auxiliarySpawnIntervalMs": 1}, {"portalSpawnIntervalMs": 600000},
                       {"auxiliarySpawnIntervalMs": 5000, "portalSpawnIntervalMs": 10000}):
            master, pattern = self.candidate()
            for key in ("auxiliarySpawnIntervalMs", "portalSpawnIntervalMs"):
                pattern["finale"].pop(key)
            pattern["finale"].update(fields)
            with self.subTest(fields=fields):
                self.validate(master, pattern)

    def test_invalid_intervals_are_rejected(self):
        for key in ("auxiliarySpawnIntervalMs", "portalSpawnIntervalMs"):
            for value in (None, True, "5000", 0, -1, 600001, 1.5):
                master, pattern = self.candidate()
                pattern["finale"][key] = value
                with self.subTest(key=key, value=value), self.assertRaises(pipeline.PipelineError):
                    self.validate(master, pattern)
        master, pattern = self.candidate()
        pattern["finale"]["spawnIntervalMs"] = 5000
        with self.assertRaises(pipeline.PipelineError):
            self.validate(master, pattern)

    def test_current_and_legacy_auxiliary_pool_are_distinct_from_main_loop(self):
        master, pattern = self.candidate()
        self.validate(master, pattern)
        pattern["finale"]["ghostPatternIds"] += ["VALTAN_CHARGE", "VALTAN_CHARGE_2"]
        self.validate(master, pattern)
        pattern["finale"]["ghostPatternIds"].pop()
        with self.assertRaisesRegex(pipeline.PipelineError, "auxiliary pool"):
            self.validate(master, pattern)

    def test_product_and_provenance_preserve_both_intervals(self):
        master, pattern = self.candidate()
        outputs = pipeline.project_v2_products(self.root, self.docs, master)
        encounter = json.loads(outputs[pipeline.ENCOUNTER_REL])
        product = next(row for row in encounter["patterns"] if row["patternId"] == pattern["patternId"])
        self.assertEqual(pattern["finale"], product["finale"])
        # Audition-only fixtures deliberately have no official live-pattern receipt.
        # Exercise field coverage with the same projected row admitted to the live set.
        product["selectionMode"] = next(row["selectionMode"] for row in encounter["patterns"]
                                          if row["selectionMode"] != pipeline.AUDITION_ONLY)
        outputs[pipeline.ENCOUNTER_REL] = json.dumps(encounter)
        receipt = json.loads(pipeline.project_provenance_receipt(self.root, outputs))
        entry = next(row for row in receipt["entries"] if row.get("targetId") == "pattern:VALTAN_GHOST_FINALE" and row["targetField"].endswith(".finale"))
        self.assertEqual(pattern["finale"], entry["resultValue"])
        self.assertEqual("PROJECT_TUNED", entry["basis"])

    def test_existing_save_operation_keeps_optional_presence(self):
        for explicit in (False, True):
            master, pattern = self.candidate()
            if not explicit:
                del pattern["finale"]["auxiliarySpawnIntervalMs"]
                del pattern["finale"]["portalSpawnIntervalMs"]
            before = copy.deepcopy(pattern["finale"])
            operation = {"op": "SET_SCRIPTED_SEQUENCE", **copy.deepcopy(master["decisionModel"]["scriptedSequence"])}
            operation["interStepPursuitMs"] = 901
            saved = pipeline.apply_draft_patch(master, self.docs[pipeline.BOSS_PROFILES_REL], self.docs[pipeline.DAMAGE_REL],
                {"schema": pipeline.DRAFT_PATCH_SCHEMA, "formatVersion": 1, "sourceRevision": "a"*64, "operations": [operation]},
                "a"*64, self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL], require_runtime_resources=False)[0]
            result = next(row for row in saved["patterns"] if row["patternId"] == pattern["patternId"])
            self.assertEqual(before, result["finale"])
            self.assertEqual(before, pattern["finale"])


if __name__ == "__main__":
    unittest.main()
