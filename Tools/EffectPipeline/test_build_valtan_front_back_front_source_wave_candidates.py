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
        cls.source_receipt = _json(SOURCE_ROOT, sut.OUTPUT_RECEIPT)
        cls.outputs, cls.receipt = sut.build_outputs(SOURCE_ROOT)

    def make_sealed_fixture(self) -> Path:
        temporary = tempfile.TemporaryDirectory(
            prefix="valtan-fbf-wave-sealed-composition-test."
        )
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        paths = {
            PurePosixPath(row["path"])
            for row in self.source_receipt["sourceGuards"]
        }
        paths.update(
            {
                sut.PROJECT_DRAWABLE_PROOF,
                sut.PROJECT_APPLICATION_RECEIPT,
                sut.PROJECT_DRAWABLE_PROOF_SCHEMA,
                sut.PROJECT_APPLICATION_RECEIPT_SCHEMA,
            }
        )
        paths.add(sut.OUTPUT_RECEIPT)
        paths.add(
            PurePosixPath(
                self.source_receipt["aggregateCanary"]["authoredDocumentPath"]
            )
        )
        paths.add(
            PurePosixPath(
                self.source_receipt["whirlwindCanary"]["authoredDocumentPath"]
            )
        )
        for row in self.source_receipt["candidates"]:
            paths.add(PurePosixPath(row["candidateDocumentPath"]))
            paths.add(PurePosixPath(row["targetAuthoredDocumentPath"]))
        for relative in sorted(paths, key=PurePosixPath.as_posix):
            source = SOURCE_ROOT.joinpath(*relative.parts)
            if not source.is_file():
                continue
            target = root.joinpath(*relative.parts)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
        return root

    @staticmethod
    def write_json(path: Path, value: dict) -> None:
        path.write_bytes(sut._json_bytes(value))

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

    def test_sealed_composition_metadata_tracks_current_exact_inputs(self) -> None:
        guard_by_path = {
            row["path"]: row["sha256"] for row in self.receipt["sourceGuards"]
        }
        for relative in (
            sut.PATTERN_BINDINGS,
            sut.ENCOUNTER,
            sut.PROJECT_PLAN,
            sut.PROJECT_OVERLAY,
        ):
            payload = SOURCE_ROOT.joinpath(*relative.parts).read_bytes()
            self.assertEqual(sut._sha256(payload), guard_by_path[relative.as_posix()])
        aggregate = self.receipt["aggregateCanary"]
        self.assertEqual(
            sut._sha256(
                SOURCE_ROOT.joinpath(*sut.AGGREGATE_AUTHORED.parts).read_bytes()
            ),
            aggregate["authoredDocumentSha256"],
        )
        self.assertEqual(
            guard_by_path[sut.PROJECT_PLAN.as_posix()],
            aggregate["projectPatchPlanSha256"],
        )
        self.assertEqual(
            guard_by_path[sut.PROJECT_OVERLAY.as_posix()],
            aggregate["projectOverlaySha256"],
        )

    def test_sealed_reseal_deep_preserves_candidates_canonical_docs_and_inputs(self) -> None:
        root = self.make_sealed_fixture()
        protected_paths = {
            sut.PATTERN_BINDINGS,
            sut.ENCOUNTER,
            sut.CATALOG,
            sut.CUES,
            sut.PROJECT_PLAN,
            sut.PROJECT_OVERLAY,
            sut.AGGREGATE_AUTHORED,
            *(
                PurePosixPath(row["candidateDocumentPath"])
                for row in self.source_receipt["candidates"]
            ),
            *(
                PurePosixPath(row["targetAuthoredDocumentPath"])
                for row in self.source_receipt["candidates"]
            ),
        }
        before = {
            relative: root.joinpath(*relative.parts).read_bytes()
            for relative in protected_paths
        }
        outputs, _ = sut.build_outputs(root)
        changed = {
            relative
            for relative, payload in outputs.items()
            if root.joinpath(*relative.parts).read_bytes() != payload
        }
        self.assertLessEqual(changed, {sut.OUTPUT_RECEIPT})
        sut.write_outputs(root, outputs)
        self.assertEqual(
            before,
            {
                relative: root.joinpath(*relative.parts).read_bytes()
                for relative in protected_paths
            },
        )

    def test_sealed_reseal_rejects_exact_binding_and_full_stage_mutation(self) -> None:
        binding_root = self.make_sealed_fixture()
        binding_path = binding_root.joinpath(*sut.PATTERN_BINDINGS.parts)
        bindings = _json(binding_root, sut.PATTERN_BINDINGS)
        binding = next(
            row for row in bindings["bindings"] if row.get("actionId") == sut.ACTION_ID
        )
        binding["clips"][0]["mappingBasis"] = "DRIFTED_MAPPING_BASIS"
        self.write_json(binding_path, bindings)
        with self.assertRaisesRegex(sut.CandidateError, "exact animation binding"):
            sut.build_outputs(binding_root)

        stage_root = self.make_sealed_fixture()
        encounter_path = stage_root.joinpath(*sut.ENCOUNTER.parts)
        encounter = _json(stage_root, sut.ENCOUNTER)
        pattern = next(
            row for row in encounter["patterns"] if row.get("patternId") == sut.PATTERN_ID
        )
        stage = next(
            row
            for row in pattern["stages"]
            if row.get("stageId") == sut.STAGE_ID
            and row.get("actionId") == sut.ACTION_ID
        )
        stage["hitDelayMs"] += 1
        self.write_json(encounter_path, encounter)
        with self.assertRaisesRegex(sut.CandidateError, "exact Server stage"):
            sut.build_outputs(stage_root)

    def test_sealed_reseal_rejects_project_identity_seal_and_source_wave_leak(self) -> None:
        identity_root = self.make_sealed_fixture()
        plan_path = identity_root.joinpath(*sut.PROJECT_PLAN.parts)
        plan = _json(identity_root, sut.PROJECT_PLAN)
        target = next(
            row
            for row in plan["targets"]
            if row.get("targetEffectAssetId") == sut.AGGREGATE_EFFECT_ID
        )
        target["clipOccurrenceId"] = "valtan.attack.front-back-front.active.clip.drift"
        self.write_json(plan_path, plan)
        with self.assertRaisesRegex(sut.CandidateError, "target identity"):
            sut.build_outputs(identity_root)

        seal_root = self.make_sealed_fixture()
        seal_plan_path = seal_root.joinpath(*sut.PROJECT_PLAN.parts)
        seal_plan = _json(seal_root, sut.PROJECT_PLAN)
        seal_target = next(
            row
            for row in seal_plan["targets"]
            if row.get("targetEffectAssetId") == sut.AGGREGATE_EFFECT_ID
        )
        seal_target["overlayDocumentSha256"] = "0" * 64
        self.write_json(seal_plan_path, seal_plan)
        with self.assertRaisesRegex(sut.CandidateError, "content seal"):
            sut.build_outputs(seal_root)

        unproved_root = self.make_sealed_fixture()
        unproved_overlay_path = unproved_root.joinpath(*sut.PROJECT_OVERLAY.parts)
        unproved_overlay = _json(unproved_root, sut.PROJECT_OVERLAY)
        unproved_overlay["elements"][0]["displayName"] = "unproved-overlay-mutation"
        self.write_json(unproved_overlay_path, unproved_overlay)
        unproved_plan_path = unproved_root.joinpath(*sut.PROJECT_PLAN.parts)
        unproved_plan = _json(unproved_root, sut.PROJECT_PLAN)
        unproved_target = next(
            row
            for row in unproved_plan["targets"]
            if row.get("targetEffectAssetId") == sut.AGGREGATE_EFFECT_ID
        )
        unproved_target["overlayDocumentSha256"] = sut._sha256(
            unproved_overlay_path.read_bytes()
        )
        self.write_json(unproved_plan_path, unproved_plan)
        with self.assertRaisesRegex(sut.CandidateError, "drawable proof binding"):
            sut.build_outputs(unproved_root)

        leak_root = self.make_sealed_fixture()
        leak_overlay_path = leak_root.joinpath(*sut.PROJECT_OVERLAY.parts)
        leak_overlay = _json(leak_root, sut.PROJECT_OVERLAY)
        leaked_source = self.source_receipt["candidates"][0]["candidateElements"][0][
            "sourceNode"
        ]
        leak_overlay["elements"][0]["sourceNode"] = leaked_source
        self.write_json(leak_overlay_path, leak_overlay)
        leak_plan_path = leak_root.joinpath(*sut.PROJECT_PLAN.parts)
        leak_plan = _json(leak_root, sut.PROJECT_PLAN)
        leak_target = next(
            row
            for row in leak_plan["targets"]
            if row.get("targetEffectAssetId") == sut.AGGREGATE_EFFECT_ID
        )
        leak_target["overlayDocumentSha256"] = sut._sha256(
            leak_overlay_path.read_bytes()
        )
        leak_target["desiredElements"][0]["sourceNode"] = leaked_source
        self.write_json(leak_plan_path, leak_plan)
        leak_plan_sha = sut._sha256(leak_plan_path.read_bytes())
        leak_proof_path = leak_root.joinpath(*sut.PROJECT_DRAWABLE_PROOF.parts)
        leak_proof = _json(leak_root, sut.PROJECT_DRAWABLE_PROOF)
        leak_proof["patchPlanSha256"] = leak_plan_sha
        leak_proof_target = next(
            row
            for row in leak_proof["targets"]
            if row.get("effectAssetId") == sut.AGGREGATE_EFFECT_ID
        )
        leak_proof_target["overlayDocumentSha256"] = sut._sha256(
            leak_overlay_path.read_bytes()
        )
        self.write_json(leak_proof_path, leak_proof)
        leak_receipt_path = leak_root.joinpath(*sut.PROJECT_APPLICATION_RECEIPT.parts)
        leak_receipt = _json(leak_root, sut.PROJECT_APPLICATION_RECEIPT)
        leak_receipt["inputs"]["patchPlanSha256"] = leak_plan_sha
        leak_receipt["inputs"]["drawableProofSha256"] = sut._sha256(
            leak_proof_path.read_bytes()
        )
        leak_receipt_target = next(
            row
            for row in leak_receipt["targets"]
            if row.get("effectAssetId") == sut.AGGREGATE_EFFECT_ID
        )
        leak_receipt_target["overlayDocumentSha256"] = sut._sha256(
            leak_overlay_path.read_bytes()
        )
        self.write_json(leak_receipt_path, leak_receipt)
        with self.assertRaisesRegex(sut.CandidateError, "leaked into project overlay"):
            sut.build_outputs(leak_root)

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
            source_paths.update(
                {
                    sut.PROJECT_DRAWABLE_PROOF_SCHEMA,
                    sut.PROJECT_APPLICATION_RECEIPT_SCHEMA,
                }
            )
            for relative in source_paths:
                source = SOURCE_ROOT.joinpath(*relative.parts)
                if not source.is_file():
                    continue
                target = root.joinpath(*relative.parts)
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target)
            sut.write_outputs(root, self.outputs)
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
