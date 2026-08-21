#!/usr/bin/env python3
"""Tests for immutable FRONT_BACK_FRONT source-wave candidate generation."""

from __future__ import annotations

from copy import deepcopy
import json
import math
from pathlib import Path, PurePosixPath
import shutil
import tempfile
import unittest

import build_valtan_front_back_front_source_wave_candidates as sut
import validate_boss_pattern_effects as schema_validator


SOURCE_ROOT = Path(__file__).resolve().parents[2]
CANDIDATE_SCHEMA_PATH = Path(__file__).parent / "Schemas" / (
    "lostark.valtan-front-back-front-source-wave-candidates.schema.json"
)


def _json(root: Path, relative: str | PurePosixPath) -> dict:
    path = PurePosixPath(relative)
    return json.loads(root.joinpath(*path.parts).read_text(encoding="utf-8"))


class FrontBackFrontSourceWaveCandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.outputs, cls.receipt = sut.build_outputs(SOURCE_ROOT)

    def test_four_chronological_groups_partition_exact_100_elements(self) -> None:
        rows = self.receipt["candidates"]
        self.assertEqual(4, len(rows))
        self.assertEqual([1, 2, 3, 4], [row["waveOrdinal"] for row in rows])
        self.assertEqual([25, 25, 25, 25], [row["candidateElementCount"] for row in rows])
        pairs = [
            (element["elementId"], element["sourceNode"])
            for row in rows
            for element in row["candidateElements"]
        ]
        self.assertEqual(100, len(pairs))
        self.assertEqual(100, len(set(pairs)))
        self.assertEqual(12, sum(len(row["notifySystemTimingGroups"]) for row in rows))

    def test_cue_starts_are_safe_integer_floor_with_sub_millisecond_local_delay(self) -> None:
        rows = self.receipt["candidates"]
        self.assertEqual([1169, 2253, 3224, 4220], [row["cueSourceStartMs"] for row in rows])
        for row in rows:
            expected_floor = math.floor(row["sourceTimeSeconds"] * 1000.0)
            self.assertEqual(expected_floor, row["cueSourceStartMs"])
            expected_local = row["sourceTimeSeconds"] - expected_floor / 1000.0
            self.assertEqual(expected_local, row["cueLocalStartDelaySeconds"])
            self.assertGreaterEqual(expected_local, 0.0)
            self.assertLess(expected_local, 0.001000001)
            cue = row["cueRow"]
            self.assertEqual("natural", cue["stopPolicy"])
            self.assertIsNone(cue["sourceEndMs"])
            self.assertEqual(sut.CLIP_OCCURRENCE_ID, cue["clipOccurrenceId"])

    def test_each_v13_document_contains_only_its_25_wave_elements(self) -> None:
        for row in self.receipt["candidates"]:
            relative = PurePosixPath(row["candidateDocumentPath"])
            document = json.loads(self.outputs[relative].decode("utf-8"))
            self.assertEqual("lostark.effect-authoring", document["schema"])
            self.assertEqual(13, document["version"])
            self.assertEqual(row["effectAssetId"], document["effectAssetId"])
            self.assertEqual(row["documentDisplayName"], document["displayName"])
            self.assertEqual(25, len(document["elements"]))
            self.assertEqual(
                [(value["elementId"], value["sourceNode"]) for value in row["candidateElements"]],
                [(value["id"], value["sourceNode"]) for value in document["elements"]],
            )
            self.assertTrue(
                all(
                    value["detail"]["timing"]["startDelaySeconds"]
                    == row["cueLocalStartDelaySeconds"]
                    for value in document["elements"]
                )
            )

    def test_fourth_wave_is_explicit_auxiliary_and_never_a_gameplay_hit(self) -> None:
        auxiliary = self.receipt["candidates"][-1]
        self.assertEqual("auxiliary-source-wave", auxiliary["waveId"])
        self.assertEqual("auxiliary-source-wave", auxiliary["presentationRole"])
        self.assertEqual(
            "FORBIDDEN_AUXILIARY_NOT_GAMEPLAY_HIT",
            auxiliary["gameplayHitDisposition"],
        )
        self.assertEqual(3, self.receipt["clipIdentity"]["serverHitCount"])
        self.assertEqual(0, self.receipt["summary"]["canonicalMutationCount"])

    def test_all_effects_display_identity_is_aggregate_plus_four_source_cues(self) -> None:
        display = self.receipt["allEffectsClipDisplay"]
        self.assertEqual(5, display["expectedCueCountAfterApply"])
        self.assertEqual(
            [
                "Project Tuned Aggregate",
                "Source Wave 01",
                "Source Wave 02",
                "Source Wave 03",
                "Source Wave Aux",
            ],
            [row["displayLabel"] for row in display["cueDisplayOrder"]],
        )
        self.assertEqual(
            [900, 1169, 2253, 3224, 4220],
            [row["sourceStartMs"] for row in display["cueDisplayOrder"]],
        )

    def test_aggregate_project_overlay_binding_and_whirlwind_are_canaries_only(self) -> None:
        aggregate = self.receipt["aggregateCanary"]
        self.assertEqual(sut.AGGREGATE_EFFECT_ID, aggregate["effectAssetId"])
        self.assertEqual(0, aggregate["sourceElementAppendCount"])
        self.assertEqual(
            "PRESERVE_EXISTING_PROJECT_TUNED_AGGREGATE", aggregate["disposition"]
        )
        self.assertEqual(
            "FORBIDDEN",
            self.receipt["clipIdentity"]["animationBindingMutationDisposition"],
        )
        self.assertEqual(
            sut.WHIRLWIND_EFFECT_ID,
            self.receipt["whirlwindCanary"]["effectAssetId"],
        )
        output_paths = {path.as_posix() for path in self.outputs}
        self.assertNotIn(aggregate["authoredDocumentPath"], output_paths)
        self.assertNotIn(sut.PATTERN_BINDINGS.as_posix(), output_paths)
        self.assertNotIn(sut.CATALOG.as_posix(), output_paths)
        self.assertNotIn(sut.CUES.as_posix(), output_paths)

    def test_candidate_receipt_validates_against_schema(self) -> None:
        schema = json.loads(CANDIDATE_SCHEMA_PATH.read_text(encoding="utf-8"))
        schema_validator.validate_schema_instance(self.receipt, schema)

    def test_write_check_and_stale_detection_are_deterministic(self) -> None:
        with tempfile.TemporaryDirectory(prefix="valtan-fbf-wave-builder-test.") as temp:
            root = Path(temp)
            source_paths = {
                PurePosixPath(row["path"])
                for row in self.receipt["sourceGuards"]
            }
            source_paths.add(
                PurePosixPath(self.receipt["aggregateCanary"]["authoredDocumentPath"])
            )
            source_paths.add(
                PurePosixPath(self.receipt["whirlwindCanary"]["authoredDocumentPath"])
            )
            for relative in source_paths:
                source = SOURCE_ROOT.joinpath(*relative.parts)
                target = root.joinpath(*relative.parts)
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target)
            outputs, receipt = sut.build_outputs(root)
            sut.write_outputs(root, outputs)
            sut.check_outputs(root, outputs)
            first = {
                relative.as_posix(): root.joinpath(*relative.parts).read_bytes()
                for relative in outputs
            }
            second_outputs, second_receipt = sut.build_outputs(root)
            self.assertEqual(receipt, second_receipt)
            self.assertEqual(outputs, second_outputs)
            sut.write_outputs(root, second_outputs)
            self.assertEqual(
                first,
                {
                    relative.as_posix(): root.joinpath(*relative.parts).read_bytes()
                    for relative in outputs
                },
            )
            candidate_path = next(
                relative for relative in outputs if relative != sut.OUTPUT_RECEIPT
            )
            root.joinpath(*candidate_path.parts).write_bytes(b"{}\n")
            with self.assertRaises(sut.CandidateError):
                sut.check_outputs(root, outputs)


if __name__ == "__main__":
    unittest.main(verbosity=2)
