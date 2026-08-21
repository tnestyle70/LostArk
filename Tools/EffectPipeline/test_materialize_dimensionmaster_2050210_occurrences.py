from __future__ import annotations

import copy
import importlib.util
import json
import pathlib
import sys
import tempfile
import unittest


SCRIPT = pathlib.Path(__file__).with_name(
    "materialize_dimensionmaster_2050210_occurrences.py"
)
SCRIPT_DIRECTORY = str(SCRIPT.parent)
if SCRIPT_DIRECTORY not in sys.path:
    sys.path.insert(0, SCRIPT_DIRECTORY)
SPEC = importlib.util.spec_from_file_location(
    "dimensionmaster_2050210_occurrence_materializer", SCRIPT
)
assert SPEC is not None and SPEC.loader is not None
materializer = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = materializer
SPEC.loader.exec_module(materializer)


class DimensionMaster2050210OccurrenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.target = materializer.load_json(materializer.TARGET_PATH)
        cls.source = materializer.load_json(materializer.IMPORTED_PATH)
        cls.receipt = materializer.load_json(materializer.RESTORATION_RECEIPT_PATH)
        cls.projected = materializer.build_source_rows(cls.source, cls.receipt)

    def baseline(self) -> dict:
        value = copy.deepcopy(self.target)
        value["elements"] = value["elements"][: len(materializer.EXPECTED_TUNED_ROWS)]
        return value

    def test_repository_has_exact_selective_projection_and_cue_contract(self) -> None:
        result = materializer.build_document(
            copy.deepcopy(self.target), self.source, self.receipt
        )
        self.assertEqual(result, self.target)
        self.assertEqual(len(result["elements"]), 12)
        materializer.validate_animation_event_text(
            materializer.ANIMATION_EVENTS_PATH.read_text(encoding="utf-8-sig")
        )

    def test_projection_preserves_nine_and_appends_only_three_missing_rows(self) -> None:
        baseline = self.baseline()
        before = copy.deepcopy(baseline["elements"])
        result = materializer.build_document(baseline, self.source, self.receipt)
        self.assertEqual(result["elements"][:9], before)
        self.assertEqual(result["elements"][9:], self.projected[1:])
        self.assertNotIn(
            materializer.SOURCE_TARGET_IDS[0],
            [row["id"] for row in result["elements"]],
        )

    def test_visible_cadence_is_exactly_four_with_inner_snapshot(self) -> None:
        result = materializer.build_document(
            self.baseline(), self.source, self.receipt
        )
        tuned = next(
            row
            for row in result["elements"]
            if row["id"] == materializer.TUNED_REFERENCE_ID
        )
        visual_rows = [tuned, *result["elements"][9:]]
        self.assertEqual(len(visual_rows), 4)
        self.assertEqual(
            tuple(
                row["detail"]["timing"]["startDelaySeconds"]
                for row in visual_rows
            ),
            materializer.SOURCE_START_SECONDS,
        )
        self.assertEqual(
            tuple(
                tuple(row["detail"]["transform"]["position"])
                for row in result["elements"][9:]
            ),
            materializer.SOURCE_LOCAL_POSITIONS[1:],
        )
        for row in result["elements"][9:]:
            self.assertTrue(row["actionCueAttachment"]["enabled"])
            self.assertFalse(row["actionCueAttachment"]["follow"])
            self.assertEqual(
                row["transformInheritance"],
                {"enabled": False, "masterElementId": ""},
            )

    def test_substitution_receipt_and_duplicate_source_node_are_explicit(self) -> None:
        admission = materializer.SOURCE_OCCURRENCE_ADMISSION_RECEIPT
        base = admission["rows"][0]
        self.assertEqual(
            base["disposition"], "SOURCE_EXACT_EVIDENCE_ONLY_NOT_ADMITTED"
        )
        self.assertEqual(
            base["substitutedByCurrent"], materializer.TUNED_REFERENCE_ID
        )
        self.assertEqual(
            [row["disposition"] for row in admission["rows"][1:]],
            ["ADMITTED_SOURCE_EXACT"] * 3,
        )
        self.assertTrue(admission["duplicateSourceNodePolicy"]["allowed"])

    def test_project_tuned_timing_is_not_relabelled_as_source_identity(self) -> None:
        provenance = materializer.TUNED_FIELD_PROVENANCE
        self.assertEqual(
            provenance["sourceIdentity"]["sourceStartDelaySeconds"], 0.60
        )
        tuned = provenance["projectTunedFields"][
            "detail.timing.startDelaySeconds"
        ]
        self.assertEqual(tuned, {"value": 0.25, "provenance": "PROJECT_TUNED"})

    def test_unknown_current_drift_and_partial_projection_fail_closed(self) -> None:
        drifted = self.baseline()
        drifted["elements"][0]["detail"]["timing"]["startDelaySeconds"] = 9.0
        with self.assertRaisesRegex(
            materializer.MaterializationError, "current tuned row drifted"
        ):
            materializer.build_document(drifted, self.source, self.receipt)

        partial = self.baseline()
        partial["elements"].append(copy.deepcopy(self.projected[1]))
        with self.assertRaisesRegex(
            materializer.MaterializationError, "unknown/partial"
        ):
            materializer.build_document(partial, self.source, self.receipt)

    def test_source_or_receipt_identity_drift_fails_closed(self) -> None:
        source = copy.deepcopy(self.source)
        row = next(
            value
            for value in source["elements"]
            if value["id"] == materializer.SOURCE_ELEMENT_IDS[2]
        )
        row["detail"]["transform"]["position"][2] = -9.0
        with self.assertRaisesRegex(
            materializer.MaterializationError, "source local pose changed"
        ):
            materializer.build_source_rows(source, self.receipt)

        receipt = copy.deepcopy(self.receipt)
        target = next(
            value
            for value in receipt["targets"]
            if value["targetEffectAssetId"] == materializer.TARGET_EFFECT_ID
        )
        receipt_row = next(
            value
            for value in target["particleRows"]
            if value["sourceElementId"] == materializer.SOURCE_ELEMENT_IDS[0]
        )
        receipt_row["targetElementId"] = "authored.source-particle.forged"
        with self.assertRaisesRegex(
            materializer.MaterializationError, "stable identity drifted"
        ):
            materializer.build_source_rows(self.source, receipt)

    def test_layout_append_and_atomic_roundtrip_are_byte_idempotent(self) -> None:
        baseline = self.baseline()
        baseline_text = json.dumps(baseline, ensure_ascii=False, indent=2) + "\n"
        result = materializer.build_document(baseline, self.source, self.receipt)
        rendered = materializer.render_materialized_text(
            baseline_text, baseline, result
        )
        self.assertEqual(
            materializer.parse_json_text(rendered, "test projection"), result
        )

        with tempfile.TemporaryDirectory() as temporary:
            target_path = pathlib.Path(temporary) / "target.effect.json"
            target_path.write_text(baseline_text, encoding="utf-8", newline="\n")
            changed = materializer.run(
                write=True,
                target_path=target_path,
                source_path=materializer.IMPORTED_PATH,
                receipt_path=materializer.RESTORATION_RECEIPT_PATH,
                animation_events_path=materializer.ANIMATION_EVENTS_PATH,
            )
            self.assertTrue(changed)
            first = target_path.read_bytes()
            self.assertEqual(materializer.load_json(target_path), result)
            changed_again = materializer.run(
                write=True,
                target_path=target_path,
                source_path=materializer.IMPORTED_PATH,
                receipt_path=materializer.RESTORATION_RECEIPT_PATH,
                animation_events_path=materializer.ANIMATION_EVENTS_PATH,
            )
            self.assertFalse(changed_again)
            self.assertEqual(target_path.read_bytes(), first)
            self.assertEqual(list(pathlib.Path(temporary).glob("*.tmp")), [])

    def test_duplicate_json_and_old_snapshot_cue_are_rejected(self) -> None:
        with self.assertRaisesRegex(
            materializer.MaterializationError, "duplicate JSON key"
        ):
            materializer.parse_json_text('{"a": 1, "a": 2}', "duplicate")
        cue = materializer.ANIMATION_EVENTS_PATH.read_text(encoding="utf-8-sig")
        cue = cue.replace(
            "follow=follow orientation=action_facing", "follow=snapshot"
        )
        with self.assertRaisesRegex(
            materializer.MaterializationError, "FOLLOW.*action_facing"
        ):
            materializer.validate_animation_event_text(cue)
        version_five = materializer.ANIMATION_EVENTS_PATH.read_text(
            encoding="utf-8-sig"
        ).replace(
            'LOSTARK_ANIM_EVENTS 6 "DimensionMaster"',
            'LOSTARK_ANIM_EVENTS 5 "DimensionMaster"',
        )
        with self.assertRaisesRegex(
            materializer.MaterializationError, "header/count changed"
        ):
            materializer.validate_animation_event_text(version_five)


if __name__ == "__main__":
    unittest.main()
