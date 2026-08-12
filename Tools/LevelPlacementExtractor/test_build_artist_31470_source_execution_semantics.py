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
RELEASE_ROOT = Path(r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC")
RECEIPT = ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-execution-semantics.receipt.json"
)


class Artist31470SourceExecutionSemanticsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.tracked_receipt = load_strict_json_object(RECEIPT)
        cls.inputs = {
            name: load_strict_json_object(ROOT / path)
            for name, path in builder.TRACKED_INPUTS.items()
        }
        cls.receipt = builder.build_receipt(
            root=ROOT, release_root=RELEASE_ROOT, inputs=cls.inputs,
            default_dependent_policy=builder.FAIL_CLOSED_DEFAULT_POLICY,
        )
        cls.current_engine_cdo_receipt = builder.build_receipt(
            root=ROOT, release_root=RELEASE_ROOT, inputs=cls.inputs,
            default_dependent_policy=builder.CURRENT_ENGINE_CDO_POLICY,
        )

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

    @classmethod
    def default_distributions(
        cls, receipt: dict[str, Any]
    ) -> list[tuple[dict[str, Any], dict[str, Any]]]:
        return [
            (module, row) for module in cls.modules(receipt)
            for row in module["distributionAdapters"]
            if row.get("defaultDependent") is True
        ]

    def assert_mutation_rejected(
        self, mutation: Callable[[dict[str, Any]], None], pattern: str
    ) -> None:
        self.assert_receipt_mutation_rejected(self.receipt, mutation, pattern)

    def assert_receipt_mutation_rejected(
        self, receipt: dict[str, Any],
        mutation: Callable[[dict[str, Any]], None], pattern: str,
    ) -> None:
        changed = copy.deepcopy(receipt)
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
            "readyModules": 257,
            "blockedModules": 142,
            "distributions": 629,
            "readyDistributions": 489,
            "blockedDistributions": 140,
        })

    def test_builder_validator_keeps_product_closed(self) -> None:
        builder.validate_receipt(self.receipt)
        self.assertFalse(self.receipt["productAdmission"]["allowed"])
        self.assertEqual(
            self.receipt["summary"]["moduleDecisionCounts"],
            {"BLOCKED": 142, "READY_FOR_HANDLER": 257},
        )
        self.assertEqual(
            self.receipt["summary"]["distributionDecisionCounts"],
            {"BLOCKED": 140, "READY_FOR_HANDLER": 489},
        )

    def test_tracked_receipt_is_intentionally_stale(self) -> None:
        with self.assertRaisesRegex(ValueError, "default-dependent policy"):
            builder.validate_receipt(self.tracked_receipt)

    def test_full35_default_dependent_gate_is_exact(self) -> None:
        self.assertEqual(self.receipt["full35DefaultDependentGate"], {
            "defaultDependentCount": 137,
            "spawnDefaultDependentCount": 36,
            "spawnRateScaleDefaultDependentCount": 35,
            "spawnRateScaleSourceDistributionKind": "RawDistributionFloat",
            "spawnRateDefaultDependentOccurrences": [2],
            "spawnExplicitZeroRateCount": 29,
            "spawnExplicitNonzeroRates": {
                "0": 7.0, "3": 40.0, "24": 80.0, "26": 50.0,
            },
            "spawnParameterRateOccurrences": [34],
        })
        rows = self.default_distributions(self.receipt)
        self.assertEqual(len(rows), 137)
        self.assertTrue(all(
            row["sourceNumericPayloadByteCount"] == 0
            and row["decision"] == "BLOCKED"
            and row["numericOracleSamples"] == []
            and builder.DEFAULT_DEPENDENT_BLOCKER
            in row["semanticClosureExecutionBlockers"]
            for _, row in rows
        ))

    def test_null_raw_distribution_zero_cannot_be_promoted(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            _, row = self.default_distributions(receipt)[0]
            row["decision"] = "READY_FOR_HANDLER"
            row["blockers"] = []
            row["numericOracleSamples"] = [{
                "time": 0.0,
                "randomUnits": [0.0, 0.25, 0.5, 0.75],
                "value": [0.0, 0.0, 0.0, 0.0],
            }]

        self.assert_mutation_rejected(
            mutate, "default-dependent zero fabrication"
        )

    def test_semantic_closure_default_blocker_cannot_be_discarded(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            _, row = self.default_distributions(receipt)[0]
            row["semanticClosureExecutionBlockers"].remove(
                builder.DEFAULT_DEPENDENT_BLOCKER
            )

        self.assert_mutation_rejected(
            mutate, "semantic closure distribution evidence changed"
        )

    def test_current_engine_cdo_policy_is_pinned_and_reconstructed_only(self) -> None:
        receipt = self.current_engine_cdo_receipt
        result = oracle.verify_receipt(
            receipt, root=ROOT, inputs=self.inputs, release_root=RELEASE_ROOT
        )
        self.assertEqual(result, {
            "modules": 399,
            "readyModules": 291,
            "blockedModules": 108,
            "distributions": 629,
            "readyDistributions": 524,
            "blockedDistributions": 105,
        })
        reconstructed = [
            (module, row) for module, row in self.default_distributions(receipt)
            if row["decision"] == "READY_FOR_HANDLER"
        ]
        self.assertEqual(len(reconstructed), 35)
        self.assertTrue(all(
            module["exactSourceClass"] == "particlemodulespawn"
            and row["propertyPath"] == "ratescale"
            and [sample["value"] for sample in row["numericOracleSamples"]]
            == [[1.0, 0.0, 0.0, 0.0]] * 3
            and row["defaultResolution"]["provenanceTier"]
            == builder.CURRENT_ENGINE_CDO_POLICY
            and row["defaultResolution"]["sourceExact"] is False
            and row["defaultResolution"]["sourceEraIdentityPinned"] is False
            for module, row in reconstructed
        ))
        evidence = receipt["defaultDependentPolicy"]["typedEvidence"]
        self.assertEqual(
            evidence["packageSha256"],
            "cee4257abe9a60730d48bab16e742f12123c71dd7f13faf7807c14647e989434",
        )
        self.assertEqual(
            evidence["classDefaultRecordSha256"],
            "0cb8fc23a1b827c3d25ba8ab518fca43aa4f425881fb8ca5fb775e96ccef7813",
        )
        spawn_two = next(
            module for module in receipt["occurrences"][2]["modules"]
            if module["exactSourceClass"] == "particlemodulespawn"
        )
        rate_two = next(
            row for row in spawn_two["distributionAdapters"]
            if row["propertyPath"] == "rate"
        )
        self.assertEqual(rate_two["decision"], "BLOCKED")
        self.assertEqual(receipt["summary"]["defaultDependentBlockedCount"], 102)

    def test_current_engine_cdo_policy_cannot_expand_to_spawn_rate(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            source = next(
                row for module, row in self.default_distributions(receipt)
                if module["exactSourceClass"] == "particlemodulespawn"
                and row["propertyPath"] == "ratescale"
            )
            target_module = next(
                module for module in receipt["occurrences"][2]["modules"]
                if module["exactSourceClass"] == "particlemodulespawn"
            )
            target = next(
                row for row in target_module["distributionAdapters"]
                if row["propertyPath"] == "rate"
            )
            for name in (
                "decision", "blockers", "numericOracleSamples",
                "reconstructedDescriptor", "defaultResolution", "fieldProvenance",
            ):
                target[name] = copy.deepcopy(source[name])

        self.assert_receipt_mutation_rejected(
            self.current_engine_cdo_receipt, mutate,
            "default-dependent zero fabrication",
        )

    def test_current_engine_package_pin_cannot_be_coordinated_away(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            replacement = "0" * 64
            engine = next(
                row for row in receipt["currentRevisionDefaultEvidence"][
                    "scriptPackages"
                ] if row["logicalPackage"].casefold() == "engine"
            )
            engine["sha256"] = replacement
            receipt["defaultDependentPolicy"]["typedEvidence"][
                "packageSha256"
            ] = replacement
            for _, row in self.default_distributions(receipt):
                if row["decision"] == "READY_FOR_HANDLER":
                    row["defaultResolution"]["packageSha256"] = replacement

        self.assert_receipt_mutation_rejected(
            self.current_engine_cdo_receipt, mutate,
            "current script package pin changed",
        )

    def test_spawn_rate_shape_mutation_fails_before_default_policy(self) -> None:
        changed = copy.deepcopy(self.inputs["candidate"])
        spawn = next(
            module for module in changed["elements"][0]["sourceRecipe"]["modules"]
            if module["className"].casefold() == "particlemodulespawn"
        )
        rate = next(
            row for row in spawn["distributions"]
            if row["propertyPath"].casefold() == "rate"
        )
        rate["lookupTable"] = [8.0, 8.0, 8.0, 8.0]
        with self.assertRaisesRegex(ValueError, "explicit nonzero Spawn Rates"):
            builder.full35_default_dependent_gate(
                self.inputs["semanticClosure"], changed
            )

    def test_spawn_ratescale_vector_type_drift_is_rejected(self) -> None:
        changed = copy.deepcopy(self.inputs["candidate"])
        spawn = next(
            module for module in changed["elements"][0]["sourceRecipe"]["modules"]
            if module["className"].casefold() == "particlemodulespawn"
        )
        rate_scale = next(
            row for row in spawn["distributions"]
            if row["propertyPath"].casefold() == "ratescale"
        )
        rate_scale["componentCount"] = 4
        with self.assertRaisesRegex(
            ValueError, "Spawn RateScale RawDistributionFloat source shape"
        ):
            builder.full35_default_dependent_gate(
                self.inputs["semanticClosure"], changed
            )

    def test_occurrence_34_spawn_parameter_branch_is_exact(self) -> None:
        spawn = next(
            module for module in self.receipt["occurrences"][34]["modules"]
            if module["exactSourceClass"] == "particlemodulespawn"
        )
        rate = next(
            row for row in spawn["distributionAdapters"]
            if row["propertyPath"] == "rate"
        )
        self.assertEqual(rate["exactSourceClass"],
                         "distributionfloatparticleparameter")
        self.assertEqual(rate["numericOracleSamples"], [{
            "sourceCueId": "skill-31470/clip-000/notify-029",
            "branch": "PARAMETER_INPUT",
            "parameterInput": {
                "name": "Spawn", "kind": "scalar", "value": 0.0,
                "sourceIndex": 0, "sourceValueByteOffset": 494,
            },
            "value": [0.0],
            "diagnosticStandardBaseValue": None,
            "blocked": False,
        }])

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
                and item["numericOracleSamples"]
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
