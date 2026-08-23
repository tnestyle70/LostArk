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
        cls.aggregate = materializer.load_json(materializer.TARGET_PATH)
        cls.source = materializer.load_json(materializer.IMPORTED_PATH)
        cls.receipt = materializer.load_json(materializer.RESTORATION_RECEIPT_PATH)
        cls.projected = materializer.build_source_rows(cls.source, cls.receipt)

    def baseline(self) -> dict:
        value = copy.deepcopy(self.aggregate)
        value["elements"] = value["elements"][
            : len(materializer.EXPECTED_TUNED_ROWS)
        ]
        return value

    @staticmethod
    def final_animation_text() -> str:
        source = materializer.ANIMATION_EVENTS_PATH.read_text(
            encoding="utf-8-sig"
        )
        lines = source.splitlines()
        old = [
            line
            for line in lines[1:]
            if 'payload="effect.dimensionmaster.skill.2050210.unified"'
            in line
        ]
        if len(old) == 1:
            lines.remove(old[0])
            lines.extend(materializer.EXPECTED_A_CUE_LINES)
        header, _ = lines[0].rsplit(" ", 1)
        lines[0] = f"{header} {len(lines) - 1}"
        return "\n".join(lines) + "\n"

    def test_repository_has_exact_four_document_partition_and_catalog(self) -> None:
        result = materializer.build_document(
            copy.deepcopy(self.aggregate), self.source, self.receipt
        )
        self.assertEqual(result, self.aggregate)
        actual = tuple(
            materializer.load_json(path)
            for path in materializer.OCCURRENCE_PATHS
        )
        materializer.validate_occurrence_documents(actual, result)
        materializer.validate_effect_catalog(
            materializer.load_json(materializer.EFFECT_CATALOG_PATH)
        )

        self.assertEqual(
            [9, 1, 1, 1], [len(document["elements"]) for document in actual]
        )
        union = [
            element["id"]
            for document in actual
            for element in document["elements"]
        ]
        self.assertEqual(
            [element["id"] for element in result["elements"]], union
        )
        self.assertEqual(len(union), len(set(union)))
        self.assertTrue(
            all(
                element["detail"]["timing"]["startDelaySeconds"] == 0
                for document in actual
                for element in document["elements"]
            )
        )

    def test_final_cues_are_four_exact_source_occurrences(self) -> None:
        final = self.final_animation_text()
        materializer.validate_animation_event_text(final)
        for cue, effect_id, start in zip(
            materializer.EXPECTED_A_CUE_LINES,
            materializer.OCCURRENCE_EFFECT_IDS,
            (250, 600, 900, 1300),
        ):
            self.assertIn(f"startms={start}", cue)
            self.assertIn(f'payload="{effect_id}"', cue)
            self.assertIn(
                'anchor="root" follow=follow orientation=action_facing', cue
            )
        self.assertNotIn(
            'payload="effect.dimensionmaster.skill.2050210.unified"',
            "\n".join(materializer.EXPECTED_A_CUE_LINES),
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

    def test_occurrence_partition_rejects_timing_or_identity_drift(self) -> None:
        timing = copy.deepcopy(self.aggregate)
        timing["elements"][9]["detail"]["timing"][
            "startDelaySeconds"
        ] = 0.61
        with self.assertRaisesRegex(
            materializer.MaterializationError,
            "aggregate occurrence start changed",
        ):
            materializer.build_occurrence_documents(timing)

        identity = copy.deepcopy(self.aggregate)
        identity["elements"][0], identity["elements"][1] = (
            identity["elements"][1],
            identity["elements"][0],
        )
        with self.assertRaisesRegex(
            materializer.MaterializationError,
            "row order/identity changed",
        ):
            materializer.build_occurrence_documents(identity)

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

    def test_write_is_idempotent_and_refuses_existing_split_drift(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = pathlib.Path(temporary)
            target_path = root / "aggregate.effect.json"
            target_path.write_text(
                json.dumps(self.baseline(), ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
                newline="\n",
            )
            event_path = root / "DimensionMaster.animevents"
            event_path.write_text(
                self.final_animation_text(), encoding="utf-8", newline="\n"
            )
            occurrence_paths = tuple(
                root / f"a{index}.effect.json" for index in range(1, 5)
            )

            changed = materializer.run(
                write=True,
                target_path=target_path,
                source_path=materializer.IMPORTED_PATH,
                receipt_path=materializer.RESTORATION_RECEIPT_PATH,
                animation_events_path=event_path,
                effect_catalog_path=materializer.EFFECT_CATALOG_PATH,
                occurrence_paths=occurrence_paths,
            )
            self.assertTrue(changed)
            first = [path.read_bytes() for path in occurrence_paths]
            self.assertFalse(
                materializer.run(
                    write=True,
                    target_path=target_path,
                    source_path=materializer.IMPORTED_PATH,
                    receipt_path=materializer.RESTORATION_RECEIPT_PATH,
                    animation_events_path=event_path,
                    effect_catalog_path=materializer.EFFECT_CATALOG_PATH,
                    occurrence_paths=occurrence_paths,
                )
            )
            self.assertEqual(
                first, [path.read_bytes() for path in occurrence_paths]
            )

            drifted = materializer.load_json(occurrence_paths[0])
            drifted["elements"][0]["detail"]["timing"][
                "startDelaySeconds"
            ] = 1
            occurrence_paths[0].write_text(
                json.dumps(drifted), encoding="utf-8", newline="\n"
            )
            with self.assertRaisesRegex(
                materializer.MaterializationError, "refusing overwrite"
            ):
                materializer.run(
                    write=True,
                    target_path=target_path,
                    source_path=materializer.IMPORTED_PATH,
                    receipt_path=materializer.RESTORATION_RECEIPT_PATH,
                    animation_events_path=event_path,
                    effect_catalog_path=materializer.EFFECT_CATALOG_PATH,
                    occurrence_paths=occurrence_paths,
                )

    def test_duplicate_json_and_aggregate_or_snapshot_cue_are_rejected(self) -> None:
        with self.assertRaisesRegex(
            materializer.MaterializationError, "duplicate JSON key"
        ):
            materializer.parse_json_text('{"a": 1, "a": 2}', "duplicate")

        cue = self.final_animation_text().replace(
            materializer.EXPECTED_A_CUE_LINES[0],
            materializer.EXPECTED_A_CUE_LINES[0].replace(
                "follow=follow orientation=action_facing", "follow=snapshot"
            ),
        )
        with self.assertRaisesRegex(
            materializer.MaterializationError, "four exact source-timed"
        ):
            materializer.validate_animation_event_text(cue)

        aggregate = self.final_animation_text().replace(
            materializer.EXPECTED_A_CUE_LINES[0],
            materializer.EXPECTED_A_CUE_LINES[0].replace(
                materializer.OCCURRENCE_EFFECT_IDS[0],
                materializer.TARGET_EFFECT_ID,
            ),
        )
        with self.assertRaisesRegex(
            materializer.MaterializationError, "no aggregate Product cue"
        ):
            materializer.validate_animation_event_text(aggregate)


if __name__ == "__main__":
    unittest.main()
