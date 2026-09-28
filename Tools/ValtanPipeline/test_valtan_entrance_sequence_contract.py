from __future__ import annotations

import copy
import unittest
from pathlib import Path

import valtan_tuning_pipeline as pipeline


class ValtanEntranceSequenceContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = Path(__file__).resolve().parents[2]
        cls.docs = pipeline.load_pipeline_documents(cls.root)
        cls.master = pipeline.join_v2_authoring(
            cls.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            cls.docs[pipeline.PRESENTATION_AUTHORING_REL],
            cls.docs[pipeline.WORLD_SET_REL],
            cls.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        cls.sequence = copy.deepcopy(cls.master["decisionModel"]["scriptedSequence"])
        cls.sequence["mode"] = pipeline.HEALTH_ROTATION_MODE
        cls.sequence["entranceCinematicPatternId"] = pipeline.OPTIONAL_ENTRY_PATTERN_ID
        cls.patterns = {row["patternId"]: row for row in cls.master["patterns"]}

    def validate(self, sequence, patterns=None):
        return pipeline._validate_scripted_sequence(
            {"scriptedSequence": sequence}, self.patterns if patterns is None else patterns
        )

    def test_optional_gate_does_not_change_reference_order(self) -> None:
        self.assertEqual(tuple(self.sequence["patternIds"]), self.validate(self.sequence))
        legacy = copy.deepcopy(self.sequence)
        del legacy["entranceCinematicPatternId"]
        self.assertEqual(self.validate(legacy), self.validate(self.sequence))

    def test_health_gate_requires_the_reviewed_managed_cinematic(self) -> None:
        for value in (None, "", 1, "VALTAN_UNKNOWN", "VALTAN_WHIRLWIND"):
            invalid = copy.deepcopy(self.sequence)
            invalid["entranceCinematicPatternId"] = value
            with self.subTest(value=value), self.assertRaises(pipeline.PipelineError):
                self.validate(invalid)
        missing = copy.deepcopy(self.patterns)
        del missing[pipeline.OPTIONAL_ENTRY_PATTERN_ID]
        with self.assertRaises(pipeline.PipelineError):
            self.validate(self.sequence, missing)
        for field, value in (
            ("category", "MECHANIC"),
            ("targetPolicy", "LOCKED_PLAYER"),
            ("aimPolicy", "LOCKED_PLAYER"),
            ("invulnerableWhileRunning", False),
        ):
            invalid = copy.deepcopy(self.patterns)
            invalid[pipeline.OPTIONAL_ENTRY_PATTERN_ID][field] = value
            with self.subTest(field=field), self.assertRaises(pipeline.PipelineError):
                self.validate(self.sequence, invalid)

    def test_explicit_play_all_keeps_but_does_not_consume_automatic_gate(self) -> None:
        explicit = copy.deepcopy(self.sequence)
        explicit["mode"] = pipeline.SCRIPTED_SEQUENCE_MODE
        # An execution-mode override must not promote this field into a Flow slot.
        self.assertEqual(tuple(explicit["patternIds"]), self.validate(explicit))
        ignored_gate_policy = copy.deepcopy(self.patterns)
        ignored_gate_policy[pipeline.OPTIONAL_ENTRY_PATTERN_ID]["invulnerableWhileRunning"] = False
        self.assertEqual(tuple(explicit["patternIds"]), self.validate(explicit, ignored_gate_policy))

    def apply(self, master, operation):
        revision = "a" * 64
        return pipeline.apply_draft_patch(
            master,
            self.docs[pipeline.BOSS_PROFILES_REL],
            self.docs[pipeline.DAMAGE_REL],
            {"schema": pipeline.DRAFT_PATCH_SCHEMA, "formatVersion": 1,
             "sourceRevision": revision, "operations": [operation]},
            revision,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
            require_runtime_resources=False,
        )[0]

    def test_save_flow_preserves_gate_when_older_writer_omits_it(self) -> None:
        base = copy.deepcopy(self.master)
        base["decisionModel"]["scriptedSequence"]["entranceCinematicPatternId"] = (
            pipeline.OPTIONAL_ENTRY_PATTERN_ID
        )
        before = copy.deepcopy(base)
        operation = {"op": "SET_SCRIPTED_SEQUENCE", **copy.deepcopy(
            base["decisionModel"]["scriptedSequence"]
        )}
        operation["interStepPursuitMs"] = 900
        for include in (False, True):
            submitted = copy.deepcopy(operation)
            if not include:
                submitted.pop("entranceCinematicPatternId")
            with self.subTest(explicit_identity=include):
                saved = self.apply(base, submitted)["decisionModel"]["scriptedSequence"]
                self.assertEqual(pipeline.OPTIONAL_ENTRY_PATTERN_ID, saved["entranceCinematicPatternId"])
                self.assertEqual(operation["patternIds"], saved["patternIds"])
                self.assertEqual(operation["mode"], saved["mode"])
                self.assertEqual(900, saved["interStepPursuitMs"])
        self.assertEqual(before, base)

    def test_save_flow_cannot_replace_or_add_gate_identity(self) -> None:
        base = copy.deepcopy(self.master)
        base["decisionModel"]["scriptedSequence"]["entranceCinematicPatternId"] = (
            pipeline.OPTIONAL_ENTRY_PATTERN_ID
        )
        before = copy.deepcopy(base)
        operation = {"op": "SET_SCRIPTED_SEQUENCE", **copy.deepcopy(
            base["decisionModel"]["scriptedSequence"]
        )}
        operation["entranceCinematicPatternId"] = "VALTAN_ENTRANCE_CINEMATIC_IDLE"
        with self.assertRaisesRegex(pipeline.PipelineError, "identity cannot be changed"):
            self.apply(base, operation)
        self.assertEqual(before, base)
        del base["decisionModel"]["scriptedSequence"]["entranceCinematicPatternId"]
        operation["entranceCinematicPatternId"] = pipeline.OPTIONAL_ENTRY_PATTERN_ID
        with self.assertRaisesRegex(pipeline.PipelineError, "identity cannot be changed"):
            self.apply(base, operation)


if __name__ == "__main__":
    unittest.main()
