from __future__ import annotations

import unittest

from Tools.EffectPipeline import build_valtan_action_bindings as bindings


class ValtanActionBindingBuilderTests(unittest.TestCase):
    def test_v1_clip_is_normalized_to_one_occurrence(self) -> None:
        rows = bindings.binding_clip_occurrences(
            {"actionId": "valtan.test.active", "clip": "mesh_att_test"}
        )
        self.assertEqual(len(rows), 1)
        self.assertEqual(
            rows[0]["clipOccurrenceId"], "valtan.test.active.clip.01"
        )
        self.assertEqual(rows[0]["mappingBasis"], "LEGACY_V1_MIGRATION")
        self.assertTrue(rows[0]["loop"])

    def test_v2_occurrence_identity_and_timing_are_preserved(self) -> None:
        expected = {
            "clipOccurrenceId": "valtan.test.active.clip.02",
            "clip": "mesh_att_test_02",
            "mappingBasis": "ANIMATION_PR_127",
            "sourceStartMs": 120,
            "playMs": 0,
            "playRate": 1.25,
            "loop": False,
        }
        rows = bindings.binding_clip_occurrences(
            {"actionId": "valtan.test.active", "clips": [expected]}
        )
        self.assertEqual(rows, [expected])

    def test_canonical_v2_build_preserves_all_137_occurrences(self) -> None:
        document, receipt = bindings.build_document()
        stages = [
            stage
            for pattern in document["patterns"]
            for stage in pattern["stages"]
        ]
        self.assertEqual(document["formatVersion"], 2)
        self.assertEqual(receipt["formatVersion"], 2)
        self.assertEqual(len(stages), 137)
        occurrence_ids = [stage["clipOccurrenceId"] for stage in stages]
        self.assertEqual(len(occurrence_ids), len(set(occurrence_ids)))
        self.assertIn("valtan.attack.swing.active.clip.02", occurrence_ids)
        self.assertEqual(receipt["summary"]["authoredStageCount"], 137)
        patterns = {row["patternId"]: row for row in document["patterns"]}
        self.assertNotIn("VALTAN_FOUR_SLASH", patterns)
        self.assertEqual(
            {
                "valtan.attack.four-slash.windup.clip.01",
                "valtan.attack.four-slash.active.clip.01",
            },
            {
                row["clipOccurrenceId"]
                for row in patterns["VALTAN_TRIPLE_SLASH"]["stages"]
            },
        )
        self.assertEqual(
            {
                "valtan.attack.four-slash.active.clip.02",
                "valtan.attack.four-slash.recovery.clip.01",
            },
            {
                row["clipOccurrenceId"]
                for row in patterns["VALTAN_ROTATION_SLASH"]["stages"]
            },
        )

    def test_split_projection_cannot_overwrite_historical_sealed_output(self) -> None:
        document, receipt = bindings.build_document()
        with self.assertRaisesRegex(
            bindings.BindingError,
            "sealed historical Valtan action bindings differ",
        ):
            bindings.assert_sealed_outputs_unchanged(document, receipt)

    def test_v2_rejects_non_final_loop_bad_rate_and_bad_basis(self) -> None:
        base = {
            "clipOccurrenceId": "valtan.test.active.clip.01",
            "clip": "mesh_att_test_01",
            "mappingBasis": "PROJECT_AUTHORED",
            "sourceStartMs": 0,
            "playMs": 0,
            "playRate": 1.0,
            "loop": False,
        }
        second = {
            **base,
            "clipOccurrenceId": "valtan.test.active.clip.02",
            "clip": "mesh_att_test_02",
            "loop": True,
        }
        non_final_loop = {**base, "loop": True}
        with self.assertRaises(bindings.BindingError):
            bindings.binding_clip_occurrences({
                "actionId": "valtan.test.active",
                "clips": [non_final_loop, second],
            })
        with self.assertRaises(bindings.BindingError):
            bindings.binding_clip_occurrences({
                "actionId": "valtan.test.active",
                "clips": [{**base, "playRate": 16.01}],
            })
        with self.assertRaises(bindings.BindingError):
            bindings.binding_clip_occurrences({
                "actionId": "valtan.test.active",
                "clips": [{**base, "mappingBasis": "NAME_GUESS"}],
            })

    def test_v2_rejects_duplicate_occurrence_ids_during_build(self) -> None:
        rows = [
            {
                "actionId": "valtan.test.one",
                "clips": [{
                    "clipOccurrenceId": "valtan.test.shared.clip.01",
                    "clip": "mesh_one",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 0,
                    "playRate": 1.0,
                    "loop": True,
                }],
            },
            {
                "actionId": "valtan.test.two",
                "clips": [{
                    "clipOccurrenceId": "valtan.test.shared.clip.01",
                    "clip": "mesh_two",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 0,
                    "playRate": 1.0,
                    "loop": True,
                }],
            },
        ]
        with self.assertRaises(bindings.BindingError):
            bindings.index_binding_clips(rows)


if __name__ == "__main__":
    unittest.main()
