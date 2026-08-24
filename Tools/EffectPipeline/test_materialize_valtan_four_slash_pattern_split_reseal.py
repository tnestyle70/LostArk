#!/usr/bin/env python3
"""Focused tests for the Valtan split-owner receipt reseal."""

from __future__ import annotations

import copy
import importlib.util
import os
from pathlib import Path
import struct
import tempfile
import unittest


SCRIPT = Path(__file__).with_name(
    "reseal_valtan_four_slash_pattern_split.py"
)
SPEC = importlib.util.spec_from_file_location(
    "valtan_four_slash_pattern_split_reseal",
    SCRIPT,
)
assert SPEC is not None and SPEC.loader is not None
RESEAL = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(RESEAL)
MIGRATION = RESEAL.migration

OLD_SUCCESSOR_CONTRACT = {
    "carrierReceiptPath": (
        "Data/Effects/Imported/Valtan/CarrierV1/"
        "Valtan.carrier-v1-materialization-receipt.v1.json"
    ),
    "carrierReceiptCanonicalSha256": (
        "a1a0515a6072c52097bef9ada0ab681fe8c8c46c4d64d902817d4e2e4e826e00"
    ),
    "currentCueCanonicalSha256": (
        "6b0ce162e27eab4f8839c1aaec1e88e202afb314addde04864fa18252fde9b83"
    ),
    "currentCueCount": 44,
    "retiredCueCount": 105,
    "replacementMappingCount": 48,
    "retiredWithoutSuccessorCount": 57,
    "uniqueReplacementBindingCount": 42,
    "carrierClipCueCount": 43,
    "newCarrierCueWithoutRetiredPredecessorCount": 1,
    "carrierRetiredBaselineCueCount": 96,
    "legacyRetiredBaselineCueCount": 2,
    "survivingBaselineCueCount": 1,
    "postBaselineRetiredCueCount": 9,
}


def _codec_float(value: float) -> float:
    runtime = struct.unpack("<f", struct.pack("<f", value))[0]
    return float(format(runtime, ".9g"))


def _simulate_cpp_codec_numbers(value: object) -> object:
    if isinstance(value, float):
        return _codec_float(value)
    if isinstance(value, list):
        return [_simulate_cpp_codec_numbers(row) for row in value]
    if isinstance(value, dict):
        return {
            key: _simulate_cpp_codec_numbers(row)
            for key, row in value.items()
        }
    return value


def _next_float32(value: float) -> float:
    bits = struct.unpack("<I", struct.pack("<f", value))[0]
    return struct.unpack("<f", struct.pack("<I", bits + 1))[0]


class ValtanFourSlashPatternSplitResealTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.carrier = MIGRATION.read_json(RESEAL.CARRIER_RECEIPT_PATH)
        cls.receipt = MIGRATION.read_json(RESEAL.MIGRATION_RECEIPT_PATH)
        cls.bindings = MIGRATION.read_json(RESEAL.BINDINGS_PATH)
        cls.cues = MIGRATION.read_json(RESEAL.CUES_PATH)
        cls.action_bindings = MIGRATION.read_json(
            RESEAL.ACTION_BINDINGS_PATH
        )

    def test_checked_in_successor_is_idempotent_and_preserves_baseline(
        self,
    ) -> None:
        baseline = copy.deepcopy(self.receipt["baselineIdentity"])
        state, carrier, receipt = RESEAL.build_resealed_receipts(
            self.carrier,
            self.receipt,
            self.bindings,
            self.cues,
            self.action_bindings,
        )
        self.assertEqual("RESEALED_SUCCESSOR", state)
        self.assertEqual(self.carrier, carrier)
        self.assertEqual(self.receipt, receipt)
        self.assertEqual(baseline, receipt["baselineIdentity"])
        self.assertEqual(
            RESEAL.EXPECTED_BASELINE_IDENTITY_CANONICAL_SHA256,
            MIGRATION.canonical_sha256(receipt["baselineIdentity"]),
        )

    def test_exact_sealed_preimage_upgrades_to_checked_in_successor(self) -> None:
        carrier = copy.deepcopy(self.carrier)
        receipt = copy.deepcopy(self.receipt)
        carrier.pop("clip01ScreenPostSuccessorOverlay")
        proof = carrier.pop("fourSlashPatternSplitOwnerReseal")
        groups = {
            row.get("cueBindingId"): row for row in carrier["clipGroups"]
        }
        mappings = {
            row["retiredBindingId"]: row
            for row in carrier["retiredOwnerSuccessorMappings"]
        }
        for transfer in proof["ownerTransfers"]:
            group = groups[transfer["replacementBindingId"]]
            mapping = mappings[transfer["retiredBindingId"]]
            old_owner = transfer["oldOwner"]
            group["patternId"] = old_owner["patternId"]
            group["semanticStageId"] = old_owner["stageId"]
            group["gameplayActionId"] = old_owner["actionId"]
            mapping["patternId"] = old_owner["patternId"]
            mapping["stageId"] = old_owner["stageId"]
            mapping["actionId"] = old_owner["actionId"]
        carrier["outputs"]["cues"]["canonicalSha256"] = (
            MIGRATION.FOUR_SLASH_OLD_CUE_CANONICAL_SHA256
        )
        receipt["carrierV1SuccessorContract"] = copy.deepcopy(
            OLD_SUCCESSOR_CONTRACT
        )
        self.assertEqual(
            RESEAL.EXPECTED_OLD_CARRIER_RECEIPT_CANONICAL_SHA256,
            MIGRATION.canonical_sha256(carrier),
        )
        self.assertEqual(
            RESEAL.EXPECTED_OLD_MIGRATION_RECEIPT_CANONICAL_SHA256,
            MIGRATION.canonical_sha256(receipt),
        )

        state, upgraded_carrier, upgraded_receipt = (
            RESEAL.build_resealed_receipts(
                carrier,
                receipt,
                self.bindings,
                self.cues,
                self.action_bindings,
            )
        )
        self.assertEqual("SEALED_PREIMAGE", state)
        self.assertEqual(self.carrier, upgraded_carrier)
        self.assertEqual(self.receipt, upgraded_receipt)

    def test_screen_post_overlay_seals_only_typed_projection_after_migration(
        self,
    ) -> None:
        document = MIGRATION.read_json(
            MIGRATION.FOUR_SLASH_CLIP01_DOCUMENT_PATH
        )
        proof = MIGRATION.validate_four_slash_clip01_screen_post_overlay(
            self.carrier,
            document,
        )
        self.assertEqual(
            MIGRATION.FOUR_SLASH_CLIP01_BASE_CANONICAL_SHA256,
            proof["materializerPreimage"]["canonicalSha256"],
        )
        self.assertEqual(
            MIGRATION.FOUR_SLASH_CLIP01_FINAL_CANONICAL_SHA256,
            proof["finalDocument"]["canonicalSha256"],
        )
        self.assertEqual(
            [
                row["elementId"]
                for row in MIGRATION.FOUR_SLASH_CLIP01_SCREEN_POST_ELEMENTS
            ],
            [row["id"] for row in document["elements"][-2:]],
        )
        authored = copy.deepcopy(document)
        authored["elements"].pop(0)
        authored["elements"].insert(
            0,
            {
                "id": "valtan.clip01.weapon-slash.manual-test",
                "kind": "particle",
            },
        )
        authored["elements"].append(
            {
                "id": "screen.manual.unrelated-test",
                "kind": "screenPost",
            }
        )
        self.assertEqual(
            proof,
            MIGRATION.validate_four_slash_clip01_screen_post_overlay(
                self.carrier,
                authored,
            ),
        )

        codec_saved = _simulate_cpp_codec_numbers(copy.deepcopy(authored))
        self.assertIsInstance(codec_saved, dict)
        for element in codec_saved["elements"]:
            if element.get("id") not in {
                row["elementId"]
                for row in MIGRATION.FOUR_SLASH_CLIP01_SCREEN_POST_ELEMENTS
            }:
                continue
            # CEffectDocumentCodec omits disabled presentation defaults.
            element["detail"]["light"] = {"enabled": False}
        self.assertEqual(
            proof,
            MIGRATION.validate_four_slash_clip01_screen_post_overlay(
                self.carrier,
                codec_saved,
            ),
        )

        mutated = copy.deepcopy(document)
        mutated["elements"][-1]["id"] = "occurrence.invalid-screen-post"
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "protected identity/order drifted",
        ):
            MIGRATION.validate_four_slash_clip01_screen_post_overlay(
                self.carrier,
                mutated,
            )

        duplicated = copy.deepcopy(document)
        duplicated["elements"].append(copy.deepcopy(duplicated["elements"][-2]))
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "protected identity/order drifted",
        ):
            MIGRATION.validate_four_slash_clip01_screen_post_overlay(
                self.carrier,
                duplicated,
            )

        reversed_pair = copy.deepcopy(document)
        reversed_pair["elements"][-2:] = reversed(reversed_pair["elements"][-2:])
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "protected identity/order drifted",
        ):
            MIGRATION.validate_four_slash_clip01_screen_post_overlay(
                self.carrier,
                reversed_pair,
            )

        runtime_mutation = copy.deepcopy(codec_saved)
        film = next(
            row
            for row in runtime_mutation["elements"]
            if row.get("id")
            == MIGRATION.FOUR_SLASH_CLIP01_SCREEN_POST_ELEMENTS[0][
                "elementId"
            ]
        )
        film["detail"]["screenPost"]["intensity"] = _next_float32(0.08)
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "runtime/identity drifted",
        ):
            MIGRATION.validate_four_slash_clip01_screen_post_overlay(
                self.carrier,
                runtime_mutation,
            )

        clip02 = copy.deepcopy(codec_saved)
        clip02["elements"].append(
            {
                "id": MIGRATION.FOUR_SLASH_CLIP02_SCREEN_POST_OCCURRENCE_ID,
                "kind": "screenPost",
            }
        )
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "clip-02 ScreenPost occurrence contaminated",
        ):
            MIGRATION.validate_four_slash_clip01_screen_post_overlay(
                self.carrier,
                clip02,
            )

    def test_any_fourth_or_mutated_transfer_fails_closed(self) -> None:
        carrier = copy.deepcopy(self.carrier)
        transfer = copy.deepcopy(
            carrier["fourSlashPatternSplitOwnerReseal"]["ownerTransfers"][0]
        )
        transfer["replacementBindingId"] = "cue.valtan.invalid.fourth"
        carrier["fourSlashPatternSplitOwnerReseal"]["ownerTransfers"].append(
            transfer
        )
        with self.assertRaisesRegex(
            RESEAL.ResealError,
            "neither the sealed preimage nor successor",
        ):
            RESEAL.build_resealed_receipts(
                carrier,
                self.receipt,
                self.bindings,
                self.cues,
                self.action_bindings,
            )

        cues = copy.deepcopy(self.cues)
        cues["cues"][0]["localTransform"]["position"][0] = 0.25
        with self.assertRaisesRegex(
            RESEAL.ResealError,
            "split cue document hash drifted",
        ):
            RESEAL.build_resealed_receipts(
                self.carrier,
                self.receipt,
                self.bindings,
                cues,
                self.action_bindings,
            )

    def test_source_sequence_hash_is_derived_from_current_actionbindings(
        self,
    ) -> None:
        branch_id, sequence_sha256, path = (
            RESEAL.derive_four_slash_source_sequence_path(
                self.action_bindings
            )
        )
        self.assertEqual(MIGRATION.FOUR_SLASH_SOURCE_BRANCH_ID, branch_id)
        self.assertEqual(
            MIGRATION.FOUR_SLASH_SOURCE_SEQUENCE_CANONICAL_SHA256,
            sequence_sha256,
        )
        self.assertEqual([8, 9, 10], [row["sourceStageIndex"] for row in path])

        action_bindings = copy.deepcopy(self.action_bindings)
        pattern = next(
            row
            for row in action_bindings["patterns"]
            if row["patternId"] == "VALTAN_FOUR_SLASH"
        )
        source_action = next(
            row
            for row in pattern["sourceActions"]
            if row["sourceActionId"] == 420609
        )
        stage = next(
            row for row in source_action["stages"] if row["stageIndex"] == 9
        )
        stage["animationClips"] = ["Att_Battle_10_01"]
        with self.assertRaisesRegex(
            RESEAL.ResealError,
            "source sequence branch/hash drifted",
        ):
            RESEAL.derive_four_slash_source_sequence_path(action_bindings)

    def test_atomic_failure_restores_both_receipts(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            first = root / "carrier.json"
            second = root / "nested" / "migration.json"
            first.write_bytes(b"carrier-before")
            calls = 0

            def fail_second(source: str, target: str) -> None:
                nonlocal calls
                calls += 1
                if calls == 2:
                    raise OSError("injected replace failure")
                os.replace(source, target)

            with self.assertRaisesRegex(
                RESEAL.ResealError,
                "transaction failed",
            ):
                RESEAL._atomic_replace(
                    {
                        first: b"carrier-after",
                        second: b"migration-after",
                    },
                    replace=fail_second,
                )
            self.assertEqual(b"carrier-before", first.read_bytes())
            self.assertFalse(second.exists())


class ValtanRejoinedFourSlashResealTests(unittest.TestCase):
    """The retired writer must validate, and never rewrite, its successor."""

    def test_check_accepts_current_rejoined_successor(self) -> None:
        self.assertEqual(0, RESEAL.main(["--check"]))

    def test_write_mode_is_an_identity_operation_for_the_successor(self) -> None:
        paths = (RESEAL.BINDINGS_PATH, RESEAL.CUES_PATH)
        before = {path: path.read_bytes() for path in paths}
        self.assertEqual(0, RESEAL.main(["--write"]))
        self.assertEqual(before, {path: path.read_bytes() for path in paths})


if __name__ == "__main__":
    unittest.main()
