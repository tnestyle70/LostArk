from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path
from typing import Any, Callable

import build_artist_31470_custom_handler_oracle as builder
import verify_artist_31470_custom_handler_oracle as oracle
from effect_source_contract_io import load_strict_json_object


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-execution-semantics.receipt.json"
)
RECEIPT = ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.custom-handler-oracle.receipt.json"
)


class Artist31470CustomHandlerOracleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source = load_strict_json_object(SOURCE)
        cls.receipt = load_strict_json_object(RECEIPT)

    @staticmethod
    def reseal(receipt: dict[str, Any]) -> None:
        unsigned = copy.deepcopy(receipt)
        unsigned.pop("receiptSha256", None)
        receipt["receiptSha256"] = oracle.canonical_sha256(unsigned)

    def assert_mutation_rejected(
        self, mutation: Callable[[dict[str, Any]], None], pattern: str
    ) -> None:
        changed = copy.deepcopy(self.receipt)
        mutation(changed)
        self.reseal(changed)
        with self.assertRaisesRegex(ValueError, pattern):
            builder.validate_receipt(changed, self.source)

    def test_baseline_builder_and_independent_oracle(self) -> None:
        builder.validate_receipt(self.receipt, self.source)
        result = oracle.verify_shallow(ROOT, self.source, self.receipt)
        self.assertEqual(result, {
            "ready": 381,
            "blocked": 18,
            "distributionBlocked": 3,
        })

    def test_all_29_source_blockers_have_one_explicit_owner_row(self) -> None:
        owners = self.receipt["moduleBlockerOwnership"]
        self.assertEqual(len(owners), 29)
        self.assertEqual(len({row["moduleOccurrenceId"] for row in owners}), 29)
        self.assertTrue(all(row["ownerIds"] for row in owners))
        self.assertEqual(
            sum(row["postJoinDecision"] == "READY_FOR_HANDLER" for row in owners), 11
        )
        self.assertEqual(
            sum(row["postJoinDecision"] == "BLOCKED" for row in owners), 18
        )
        self.assertEqual(self.receipt["summary"]["ownerlessBlockerCount"], 0)

    def test_json_version_requires_exact_integer(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row.__setitem__("formatVersion", True), "version changed"
        )

    def test_standard_class_cannot_be_normalized(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["standardSeededHandlers"][0].__setitem__(
                "exactSourceClass", "particlemodulecolor"
            ),
            "class order changed",
        )

    def test_capability_grant_cannot_be_reassigned(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["capabilityGrants"][0].__setitem__(
                "exactSourceClass", row["capabilityGrants"][1]["exactSourceClass"]
            ),
            "capability grant join changed",
        )

    def test_standard_occurrence_cannot_be_reassigned(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            left = receipt["standardSeededHandlers"][0]["occurrences"][0]
            right = receipt["standardSeededHandlers"][1]["occurrences"][0]
            left["moduleOccurrenceId"] = right["moduleOccurrenceId"]

        self.assert_mutation_rejected(mutate, "occurrence identity|source occurrence join")

    def test_fixed_source_seed_cannot_be_resealed(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            sample = receipt["standardSeededHandlers"][0]["occurrences"][0][
                "numericOracleSamples"
            ][0]
            sample["fixedSeed"] += 1

        self.assert_mutation_rejected(mutate, "numeric input parity|source occurrence join")

    def test_fixed_stream_value_cannot_be_resealed(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            sample = receipt["standardSeededHandlers"][0]["occurrences"][0][
                "numericOracleSamples"
            ][0]
            sample["randomUnits"][0] += 0.125
            handler_input = {
                name: copy.deepcopy(sample[name]) for name in (
                    "payloadSha256", "emitterTime", "fixedSeed", "fixedSeedSource",
                    "randomStreamAlgorithm", "randomStreamDrawOffset", "randomUnits",
                    "evaluatedDistributions",
                )
            }
            digest = oracle.canonical_sha256(handler_input)
            sample["baseHandlerInputSha256"] = digest
            sample["exactSeededHandlerInputSha256"] = digest

        self.assert_mutation_rejected(mutate, "numeric input parity|source occurrence join")

    def test_native_dispatch_decision_cannot_be_resealed(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["standardSeededHandlers"][0][
                "currentNativeDispatchEvidence"
            ].__setitem__("decision", "INFERRED_FROM_NAME"),
            "native dispatch proof changed",
        )

    def test_custom_handler_blocker_cannot_be_removed(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            row = receipt["blockedCustomModuleHandlers"][0]
            row["decision"] = "READY_FOR_EXACT_NATIVE_ALIAS"
            row["blockers"] = []

        self.assert_mutation_rejected(mutate, "custom handler blocker")

    def test_custom_handler_occurrence_cannot_move_family(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            receipt["blockedCustomModuleHandlers"][0]["occurrenceIds"][0] = (
                receipt["blockedCustomModuleHandlers"][1]["occurrenceIds"][0]
            )

        self.assert_mutation_rejected(mutate, "occurrence identity|source occurrence join")

    def test_custom_distribution_blocker_cannot_be_removed(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            row = receipt["blockedCustomDistributionEvaluator"]
            row["decision"] = "READY_FOR_HANDLER"
            row["blockers"] = []

        self.assert_mutation_rejected(mutate, "custom distribution blocker")

    def test_module_owner_row_cannot_be_dropped(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["moduleBlockerOwnership"].pop(),
            "module blocker ownership coverage|summary changed",
        )

    def test_module_owner_id_cannot_be_empty(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["moduleBlockerOwnership"][0].__setitem__("ownerIds", []),
            "owner is empty|source join changed",
        )

    def test_module_owner_cannot_be_reassigned(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            ready = next(
                row for row in receipt["moduleBlockerOwnership"]
                if row["postJoinDecision"] == "READY_FOR_HANDLER"
            )
            ready["ownerIds"] = [receipt["capabilityGrants"][1]["handlerCapabilityId"]]

        self.assert_mutation_rejected(mutate, "source join changed")

    def test_distribution_owner_row_cannot_be_dropped(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["distributionBlockerOwnership"].pop(),
            "distribution blocker ownership coverage|summary changed",
        )

    def test_product_admission_cannot_be_opened(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            receipt["productAdmission"]["allowed"] = True
            receipt["productAdmission"]["blockers"] = []

        self.assert_mutation_rejected(mutate, "Product blocker boundary|Product admission")

    def test_source_execution_identity_cannot_be_resealed(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["sourceExecutionReceipt"].__setitem__(
                "canonicalJsonSha256", "0" * 64
            ),
            "source execution receipt join changed",
        )

    def test_duplicate_top_level_key_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "duplicate.json"
            path.write_text('{"schema":"a","schema":"b"}', encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "duplicate JSON key"):
                load_strict_json_object(path)

    def test_duplicate_nested_key_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "duplicate.json"
            path.write_text('{"outer":{"x":1,"x":2}}', encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "duplicate JSON key"):
                load_strict_json_object(path)

    def test_native_binary_byte_mutation_is_rejected(self) -> None:
        source_path = Path(
            r"C:\ProgramData\Smilegate\Games\LOSTARK\Binaries\Win64\EFEngine.dll"
        )
        if not source_path.is_file():
            self.skipTest("installed EFEngine.dll unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            changed = Path(temporary) / "EFEngine.dll"
            data = bytearray(source_path.read_bytes())
            data[-1] ^= 1
            changed.write_bytes(data)
            with self.assertRaisesRegex(ValueError, "native binary SHA changed"):
                builder.verify_file_identity(changed, builder.EXPECTED_NATIVE["EFEngine.dll"])


if __name__ == "__main__":
    unittest.main()
