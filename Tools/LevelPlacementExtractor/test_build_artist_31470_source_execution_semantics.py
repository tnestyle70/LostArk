from __future__ import annotations

import copy
import json
import unittest
from pathlib import Path
from typing import Any, Callable

import build_artist_31470_source_execution_semantics as builder
import verify_artist_31470_source_execution_semantics as oracle
from effect_source_contract_io import load_strict_json_object


ROOT = Path(__file__).resolve().parents[2]
RECEIPT = ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-execution-semantics.receipt.json"
)


class Artist31470SourceExecutionSemanticsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.receipt = load_strict_json_object(RECEIPT)
        cls.inputs = {
            name: load_strict_json_object(ROOT / row["path"])
            for name, row in cls.receipt["inputs"].items()
        }

    @staticmethod
    def reseal(receipt: dict[str, Any]) -> None:
        unsigned = copy.deepcopy(receipt)
        unsigned.pop("receiptSha256", None)
        receipt["receiptSha256"] = oracle.canonical_sha256(unsigned)

    @staticmethod
    def modules(receipt: dict[str, Any]) -> list[dict[str, Any]]:
        return [
            module for occurrence in receipt["occurrences"]
            for module in occurrence["modules"]
        ]

    @classmethod
    def distributions(cls, receipt: dict[str, Any]) -> list[dict[str, Any]]:
        return [
            row for module in cls.modules(receipt)
            for row in module["distributionAdapters"]
        ]

    def assert_mutation_rejected(
        self, mutation: Callable[[dict[str, Any]], None], pattern: str
    ) -> None:
        changed = copy.deepcopy(self.receipt)
        mutation(changed)
        self.reseal(changed)
        with self.assertRaisesRegex(ValueError, pattern):
            oracle.verify_receipt(
                changed, root=ROOT, inputs=self.inputs, release_root=None
            )

    def test_baseline_independent_oracle(self) -> None:
        result = oracle.verify_receipt(
            self.receipt, root=ROOT, inputs=self.inputs, release_root=None
        )
        self.assertEqual(result, {
            "modules": 399,
            "readyModules": 370,
            "blockedModules": 29,
            "distributions": 629,
            "readyDistributions": 626,
            "blockedDistributions": 3,
        })

    def test_builder_validator_keeps_product_closed(self) -> None:
        builder.validate_receipt(self.receipt)
        self.assertFalse(self.receipt["productAdmission"]["allowed"])
        self.assertEqual(
            self.receipt["summary"]["moduleDecisionCounts"],
            {"BLOCKED": 29, "READY_FOR_HANDLER": 370},
        )
        self.assertEqual(
            self.receipt["summary"]["distributionDecisionCounts"],
            {"BLOCKED": 3, "READY_FOR_HANDLER": 626},
        )

    def test_json_version_requires_exact_integer(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row.__setitem__("formatVersion", True),
            "version changed",
        )

    def test_typed_literal_value_cannot_be_resealed(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            module = self.modules(receipt)[0]
            module["typedPayload"]["literals"][0]["value"] = False
            payload = module["typedPayload"]
            payload["payloadSha256"] = oracle.canonical_sha256({
                name: copy.deepcopy(payload[name]) for name in (
                    "stableId", "className", "objectPath", "literals",
                    "distributions",
                )
            })

        self.assert_mutation_rejected(mutate, "typed literals changed")

    def test_module_order_cannot_be_resealed(self) -> None:
        self.assert_mutation_rejected(
            lambda row: self.modules(row)[1].__setitem__("order", 99),
            "module order changed",
        )

    def test_exact_custom_class_cannot_be_normalized(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            module = next(
                row for row in self.modules(receipt)
                if row["exactSourceClass"].startswith("efparticlemodule")
            )
            module["exactSourceClass"] = oracle.legacy_normalized(
                module["exactSourceClass"]
            )

        self.assert_mutation_rejected(mutate, "module class was normalized")

    def test_unknown_property_decision_cannot_be_resealed(self) -> None:
        self.assert_mutation_rejected(
            lambda row: self.modules(row)[0]["properties"][0].__setitem__(
                "decision", "SILENTLY_IGNORED"
            ),
            "unknown property decision",
        )

    def test_property_row_cannot_be_dropped(self) -> None:
        self.assert_mutation_rejected(
            lambda row: self.modules(row)[0]["properties"].pop(),
            "property coverage changed",
        )

    def test_primitive_leaf_cannot_reference_another_literal(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            module = next(
                row for row in self.modules(receipt)
                if len(row["primitiveLeaves"]) > 1
            )
            module["primitiveLeaves"][0]["payloadLiteralId"] = (
                module["primitiveLeaves"][1]["payloadLiteralId"]
            )

        self.assert_mutation_rejected(mutate, "leaf literal binding changed")

    def test_inline_numeric_oracle_cannot_be_resealed(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            row = next(
                item for item in self.distributions(receipt)
                if not item.get("legacyOccurrenceId")
            )
            row["numericOracleSamples"][0]["value"][0] += 1.0

        self.assert_mutation_rejected(mutate, "inline oracle changed")

    def test_archetype_fallback_constant_one_is_bound(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            row = next(
                item for item in self.distributions(receipt)
                if item.get("referenceId") == "distribution-target-000"
            )
            next(
                field for field in row["currentRevisionFields"]
                if field["fieldPath"] == "constant"
            )["value"] = 0.0

        self.assert_mutation_rejected(mutate, "local current/default fields changed")

    def test_normal_parameter_range_is_bound(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            row = next(
                item for item in self.distributions(receipt)
                if item.get("referenceId") == "distribution-target-009"
            )
            next(
                field for field in row["currentRevisionFields"]
                if field["fieldPath"] == "maxinput"
            )["value"] = 1.0

        self.assert_mutation_rejected(mutate, "local current/default fields changed")

    def test_action_cue_parameter_input_is_bound(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            row = next(
                item for item in self.distributions(receipt)
                if item.get("referenceId") == "distribution-target-009"
            )
            row["numericOracleSamples"][0]["parameterInput"]["value"] = 0.75
            row["numericOracleSamples"][0]["value"] = [0.75]

        self.assert_mutation_rejected(mutate, "ActionCue input binding changed")

    def test_custom_distribution_cannot_be_admitted(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            row = next(
                item for item in self.distributions(receipt)
                if item.get("referenceId") == "distribution-target-014"
            )
            row["decision"] = "READY_FOR_HANDLER"
            row["blockers"] = []

        self.assert_mutation_rejected(mutate, "custom local evaluator became executable")

    def test_custom_handler_catalog_cannot_be_admitted(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            row = next(
                item for item in receipt["handlerCapabilities"]
                if item["decision"] == "BLOCKED"
            )
            row["decision"] = "READY_FOR_HANDLER"
            row["blockers"] = []

        self.assert_mutation_rejected(mutate, "handler catalog decision changed")

    def test_blocker_union_cannot_lose_custom_evaluator(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            receipt["blockerUnion"].remove(oracle.CUSTOM_DISTRIBUTION_BLOCKER)
            receipt["productAdmission"]["blockers"].remove(
                oracle.CUSTOM_DISTRIBUTION_BLOCKER
            )

        self.assert_mutation_rejected(mutate, "blocker union propagation changed")

    def test_seed_policy_cannot_be_resealed(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            row = next(
                item for item in self.modules(receipt) if item["seed"] is not None
            )
            row["seed"]["policy"]["randomlySelectSeedArray"] = True

        self.assert_mutation_rejected(mutate, "seed .*changed")

    def test_point_light_default_cannot_be_resealed(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            field = next(
                row for row in receipt["pointLightAdapter"]["fields"]
                if row["fieldPath"] == "radius"
            )
            field["value"] = 1024.0

        self.assert_mutation_rejected(mutate, "PointLight field evidence changed")

    def test_required_local_space_default_cannot_be_resealed(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["currentRevisionDefaultEvidence"][
                "requiredLocalSpace"
            ].__setitem__("value", True),
            "Required local-space current default changed",
        )

    def test_selected_lod_cannot_become_runtime_input(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            receipt["occurrences"][0]["selectedLod"]["fields"][0][
                "decision"
            ] = "READY_FOR_HANDLER"

        self.assert_mutation_rejected(mutate, "selected LOD default")

    def test_native_tail_cannot_become_silent_input(self) -> None:
        self.assert_mutation_rejected(
            lambda row: self.modules(row)[0]["nativeTail"].__setitem__(
                "decision", "READY_FOR_HANDLER"
            ),
            "native tail decision changed",
        )

    def test_tracked_input_semantic_mutation_is_rejected(self) -> None:
        changed_inputs = copy.deepcopy(self.inputs)
        changed_inputs["candidate"]["elements"][0]["visible"] = False
        with self.assertRaisesRegex(ValueError, "tracked input semantic hash changed"):
            oracle.verify_receipt(
                self.receipt, root=ROOT, inputs=changed_inputs,
                release_root=None,
            )


if __name__ == "__main__":
    unittest.main()
