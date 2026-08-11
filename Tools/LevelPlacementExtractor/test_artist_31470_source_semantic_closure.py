from __future__ import annotations

import copy
import json
import subprocess
import sys
import tempfile
import unittest
from collections import Counter
from pathlib import Path
from typing import Any, Callable
from unittest import mock

import artist_31470_source_semantic_closure as semantic_closure
import build_artist_31470_source_contract as source_contract
import verify_artist_31470_source_semantic_closure as semantic_oracle


ROOT = Path(__file__).resolve().parents[2]
SOURCE_RECEIPT = ROOT / (
    "Data/Effects/Imported/Artist/skill.31470.source-receipt.json"
)
ACTION_CUE_RECIPE = ROOT / (
    "Data/Effects/Imported/Artist/skill.31470.action-cue-recipe.json"
)
ACTIVE_INVENTORY = ROOT / (
    "Data/Effects/Imported/Artist/"
    "skill.31470.source-active-effect-inventory.receipt.json"
)
NORMALIZED_GRAPH = ROOT / (
    "Data/Effects/Imported/Artist/Graphs/"
    "skill.31470.normalized-effect-graph.json"
)
EXTERNAL_MODULE_CLOSURE = ROOT / (
    "Data/Effects/Imported/Artist/Modules/"
    "skill.31470.external-module-closure.json"
)
SOURCE_EVIDENCE = ROOT / (
    "Data/Effects/Imported/Artist/skill.31470.source-evidence-envelope.json"
)
LOCAL_REFERENCE_CLOSURE = ROOT / (
    "Data/Effects/Imported/Artist/Graphs/"
    "skill.31470.local-reference-closure.json"
)
MATERIAL_CLOSURE = ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.active-material-closure.json"
)
GEOMETRY_PARITY = ROOT / (
    "Data/Effects/Imported/Artist/Geometry/"
    "skill.31470.wmodel-geometry-parity.receipt.json"
)

FROZEN_INPUTS = {
    "activeInventory": ACTIVE_INVENTORY,
    "normalizedGraph": NORMALIZED_GRAPH,
    "externalModuleClosure": EXTERNAL_MODULE_CLOSURE,
    "sourceEvidence": SOURCE_EVIDENCE,
    "localReferenceClosure": LOCAL_REFERENCE_CLOSURE,
}

NESTED_DUPLICATE_KEYS = {
    "activeInventory": "activeCuePredicate",
    "normalizedGraph": "sourceNodeId",
    "externalModuleClosure": "logicalPackage",
    "sourceEvidence": "path",
    "localReferenceClosure": "sourceReceiptSha256",
}


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise AssertionError(f"JSON root is not an object: {path}")
    return value


class Artist31470SourceSemanticClosureTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.active_inventory = load_json(ACTIVE_INVENTORY)
        cls.normalized_graph = load_json(NORMALIZED_GRAPH)
        cls.external_module_closure = load_json(EXTERNAL_MODULE_CLOSURE)
        cls.source_evidence = load_json(SOURCE_EVIDENCE)
        cls.local_reference_closure = load_json(LOCAL_REFERENCE_CLOSURE)
        cls.closure = semantic_closure.build_semantic_closure(
            cls.active_inventory,
            cls.normalized_graph,
            cls.external_module_closure,
            cls.source_evidence,
            cls.local_reference_closure,
        )
        cls.oracle_result = semantic_oracle.verify_semantic_closure(
            cls.closure,
            cls.active_inventory,
            cls.normalized_graph,
            cls.external_module_closure,
            cls.source_evidence,
            cls.local_reference_closure,
        )

    @staticmethod
    def modules(closure: dict[str, Any]) -> list[dict[str, Any]]:
        return [
            module
            for occurrence in closure["occurrences"]
            for module in occurrence["modules"]
        ]

    @classmethod
    def distributions(cls, closure: dict[str, Any]) -> list[dict[str, Any]]:
        return [
            distribution
            for module in cls.modules(closure)
            for distribution in module["distributions"]
        ]

    @staticmethod
    def reseal(closure: dict[str, Any]) -> None:
        unsigned = copy.deepcopy(closure)
        unsigned.pop("closureSha256", None)
        closure["closureSha256"] = semantic_closure.canonical_sha256(unsigned)

    @staticmethod
    def recompute_admissions(row: dict[str, Any]) -> None:
        artifact_blockers = sorted(set(row["artifactBindingBlockers"]))
        row["artifactBindingBlockers"] = artifact_blockers
        row["artifactBindingIntegrity"] = {
            "verified": not artifact_blockers,
            "blockers": artifact_blockers,
        }
        execution_blockers = sorted(
            {
                *artifact_blockers,
                *row["executionBlockers"],
            }
        )
        row["executionAdmission"] = {
            "allowed": not execution_blockers,
            "blockers": execution_blockers,
        }
        product_blockers = sorted(
            {
                *row["artifactBindingBlockers"],
                *row["executionBlockers"],
                *row["productBlockers"],
            }
        )
        row["productAdmission"] = {
            "allowed": not product_blockers,
            "blockers": product_blockers,
        }

    def assert_resealed_mutation_rejected(
        self,
        mutate: Callable[[dict[str, Any]], None],
        expected_error: str,
    ) -> None:
        changed = copy.deepcopy(self.closure)
        mutate(changed)
        self.reseal(changed)
        with self.assertRaisesRegex(ValueError, expected_error):
            semantic_oracle.verify_semantic_closure(
                changed,
                self.active_inventory,
                self.normalized_graph,
                self.external_module_closure,
                self.source_evidence,
                self.local_reference_closure,
            )

    @staticmethod
    def duplicate_top_level_schema(path: Path) -> bytes:
        payload = path.read_bytes()
        opening = payload.find(b"{")
        if opening < 0:
            raise AssertionError(f"JSON object opening is missing: {path}")
        return (
            payload[: opening + 1]
            + b'\n  "schema": "FORGED_DUPLICATE_IGNORED",'
            + payload[opening + 1 :]
        )

    @staticmethod
    def duplicate_nested_key(path: Path, key: str) -> bytes:
        payload = path.read_text(encoding="utf-8")
        marker = f'"{key}":'
        marker_index = payload.find(marker)
        if marker_index < 0:
            raise AssertionError(f"nested duplicate marker is missing: {path}: {key}")
        line_start = payload.rfind("\n", 0, marker_index) + 1
        indentation = payload[line_start:marker_index]
        duplicate = f'{indentation}"{key}": null,\n'
        return (payload[:line_start] + duplicate + payload[line_start:]).encode(
            "utf-8"
        )

    def assert_duplicate_input_rejected_by_all_clis(
        self,
        mutated_name: str,
        mutated_path: Path,
        semantic_closure_path: Path,
        output_root: Path,
    ) -> None:
        inputs = dict(FROZEN_INPUTS)
        inputs[mutated_name] = mutated_path
        common_inputs = [
            "--active-inventory",
            str(inputs["activeInventory"]),
            "--normalized-graph",
            str(inputs["normalizedGraph"]),
            "--external-module-closure",
            str(inputs["externalModuleClosure"]),
            "--source-evidence",
            str(inputs["sourceEvidence"]),
            "--local-reference-closure",
            str(inputs["localReferenceClosure"]),
        ]
        commands = [
            [
                sys.executable,
                str(Path(semantic_closure.__file__).resolve()),
                *common_inputs,
                "--output",
                str(output_root / "generated-semantic-closure.json"),
            ],
            [
                sys.executable,
                str(Path(semantic_oracle.__file__).resolve()),
                "--semantic-closure",
                str(semantic_closure_path),
                *common_inputs,
            ],
            [
                sys.executable,
                str(Path(source_contract.__file__).resolve()),
                "--source-receipt",
                str(SOURCE_RECEIPT),
                "--action-cue-recipe",
                str(ACTION_CUE_RECIPE),
                "--active-inventory",
                str(inputs["activeInventory"]),
                "--normalized-graph",
                str(inputs["normalizedGraph"]),
                "--module-closure",
                str(inputs["externalModuleClosure"]),
                "--material-closure",
                str(MATERIAL_CLOSURE),
                "--source-evidence",
                str(inputs["sourceEvidence"]),
                "--local-reference-closure",
                str(inputs["localReferenceClosure"]),
                "--geometry-parity",
                str(GEOMETRY_PARITY),
                "--source-semantic-closure",
                str(semantic_closure_path),
                "--output-candidate",
                str(output_root / "candidate.json"),
                "--output-receipt",
                str(output_root / "receipt.json"),
                "--output-registry",
                str(output_root / "registry.json"),
                "--output-header",
                str(output_root / "registry.h"),
            ],
        ]
        for command in commands:
            completed = subprocess.run(
                command,
                cwd=ROOT,
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertEqual(
                completed.returncode,
                1,
                msg=(
                    f"duplicate-key CLI attack unexpectedly passed for "
                    f"{mutated_name}: {' '.join(command)}\n"
                    f"stdout={completed.stdout}\nstderr={completed.stderr}"
                ),
            )
            self.assertIn(
                "duplicate JSON key",
                completed.stderr,
                msg=f"duplicate-key failure reason was lost for {mutated_name}",
            )

    def test_all_frozen_inputs_reject_top_level_and_nested_duplicate_keys(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            temp_root = Path(directory)
            semantic_closure_path = temp_root / "semantic-closure.json"
            semantic_closure_path.write_text(
                json.dumps(self.closure, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            for input_name, input_path in FROZEN_INPUTS.items():
                for mutation_name, payload in (
                    ("top", self.duplicate_top_level_schema(input_path)),
                    (
                        "nested",
                        self.duplicate_nested_key(
                            input_path,
                            NESTED_DUPLICATE_KEYS[input_name],
                        ),
                    ),
                ):
                    mutated_path = temp_root / f"{input_name}-{mutation_name}.json"
                    mutated_path.write_bytes(payload)
                    output_root = temp_root / f"outputs-{input_name}-{mutation_name}"
                    output_root.mkdir()
                    self.assert_duplicate_input_rejected_by_all_clis(
                        input_name,
                        mutated_path,
                        semantic_closure_path,
                        output_root,
                    )

    def test_baseline_denominators_lod_and_product_are_exact(self) -> None:
        denominators = self.closure["summary"]["denominators"]
        self.assertEqual(
            {
                name: denominators[name]
                for name in (
                    "orderedModuleReferenceCount",
                    "topLevelTaggedPropertyCount",
                    "primitiveLeafCount",
                    "distributionCount",
                    "selectedLodFieldCount",
                )
            },
            {
                "orderedModuleReferenceCount": 399,
                "topLevelTaggedPropertyCount": 1434,
                "primitiveLeafCount": 1572,
                "distributionCount": 629,
                "selectedLodFieldCount": 70,
            },
        )
        self.assertEqual(
            self.closure["summary"]["selectedLodFieldDecisionCounts"],
            {"UNRESOLVED": 70},
        )
        self.assertEqual(
            self.closure["summary"]["moduleDecisionCounts"],
            {"UNRESOLVED": 399},
        )
        self.assertEqual(
            self.closure["summary"]["propertyDecisionCounts"],
            {"UNRESOLVED": 1434},
        )
        self.assertEqual(
            self.closure["summary"]["primitiveLeafDecisionCounts"],
            {"UNRESOLVED": 1572},
        )
        self.assertEqual(
            self.closure["summary"]["distributionDecisionCounts"],
            {"UNRESOLVED": 629},
        )
        self.assertFalse(self.closure["summary"]["semanticExecutionAdmission"])
        self.assertEqual(
            self.closure["productAdmission"],
            {
                "allowed": False,
                "admittedOccurrenceCount": 0,
                "totalOccurrenceCount": 35,
                "blockers": [
                    "DOWNSTREAM_COMPILER_HANDLER_RECEIPTS_REQUIRED",
                    "SOURCE_SEMANTIC_UNRESOLVED_ROWS_REMAIN",
                    "PRODUCT_ADMISSION_OWNED_BY_FINAL_INTEGRATION_GATE",
                ],
            },
        )
        self.assertEqual(self.oracle_result["productAdmission"], "0/35")

    def test_local_distribution_and_point_light_boundaries_are_exact(self) -> None:
        local = self.closure["localDistributionBindings"]
        self.assertEqual(len(local), 17)
        self.assertEqual(len({row["definitionId"] for row in local}), 15)
        self.assertEqual(len({row["referenceId"] for row in local}), 15)
        self.assertEqual(len({row["occurrenceId"] for row in local}), 17)
        self.assertTrue(all(row["classification"] == "UNRESOLVED" for row in local))
        self.assertTrue(all(not row["executionAdmission"]["allowed"] for row in local))

        point_lights = self.closure["pointLightBindings"]
        self.assertEqual(len(point_lights), 1)
        fields = {
            row["fieldName"]: row for row in point_lights[0]["fields"]
        }
        self.assertEqual(
            set(fields),
            {
                "brightness",
                "bcastcompositeshadow",
                "baffectcompositeshadowdirection",
                "lightguid",
                "lightmapguid",
                "radius",
                "falloffexponent",
                "lightcolor",
            },
        )
        for name in (
            "brightness",
            "bcastcompositeshadow",
            "baffectcompositeshadowdirection",
            "lightguid",
            "lightmapguid",
        ):
            self.assertEqual(fields[name]["sourceFidelity"], "SOURCE_EXACT")
        for name in ("radius", "falloffexponent", "lightcolor"):
            self.assertEqual(
                fields[name]["sourceFidelity"], "CURRENT_REVISION_EVIDENCE"
            )

    def test_native_tail_seed_and_default_denominators_are_exact(self) -> None:
        summary = self.closure["summary"]
        self.assertEqual(
            summary["nativeTailDecisionCounts"],
            {"UNRESOLVED": 248, "VERIFIED_IRRELEVANT": 151},
        )
        self.assertEqual(summary["seedDecisionCounts"], {"UNRESOLVED": 14})
        self.assertEqual(
            summary["implicitDefaultFamilyCounts"],
            {
                "Decal": 3,
                "Light": 1,
                "RequiredLocalSpace": 8,
                "Ribbon": 1,
                "ScreenPost": 1,
            },
        )
        self.assertEqual(
            summary["moduleSourceFidelityCounts"],
            {"CURRENT_REVISION_EVIDENCE": 248, "SOURCE_EXACT": 151},
        )

    def test_distribution_default_and_reconstruction_counts_are_exact(self) -> None:
        denominators = self.closure["summary"]["denominators"]
        self.assertEqual(denominators["defaultDependentDistributionCount"], 137)
        self.assertEqual(denominators["operationReconstructedDistributionCount"], 409)
        self.assertEqual(
            denominators["lookupChunkReconstructedDistributionCount"], 257
        )
        self.assertEqual(
            denominators["lookupCountReconstructedDistributionCount"], 257
        )
        self.assertEqual(denominators["explicitRandomOperationDistributionCount"], 82)
        self.assertEqual(
            self.closure["summary"]["distributionFieldEvidenceCounts"],
            {
                "defaultDependent": 137,
                "explicitRandomOperation": 82,
                "reconstructed:lookupTableChunkSize": 257,
                "reconstructed:lookupTableNumElements": 257,
                "reconstructed:operation": 409,
            },
        )

    def test_exact_source_classes_have_no_automatic_aliases(self) -> None:
        modules = self.modules(self.closure)
        self.assertTrue(all(module["exactSourceClass"] for module in modules))
        self.assertTrue(all(module["aliasId"] == "" for module in modules))
        self.assertTrue(all(module["aliasEvidenceId"] == "" for module in modules))
        self.assertTrue(
            all(
                module["normalizedClass"]
                == module["exactSourceClass"].casefold()
                for module in modules
            )
        )
        ef_modules = [
            module
            for module in modules
            if module["exactSourceClass"].casefold().startswith("efparticlemodule")
        ]
        seeded_modules = [
            module
            for module in modules
            if module["exactSourceClass"].casefold().endswith("_seeded")
        ]
        self.assertEqual(len(ef_modules), 15)
        self.assertEqual(len(seeded_modules), 14)
        self.assertTrue(
            all(
                module["normalizedClass"].startswith("efparticlemodule")
                for module in ef_modules
            )
        )
        self.assertTrue(
            all(module["normalizedClass"].endswith("_seeded") for module in seeded_modules)
        )

    def test_opt_in_source_contract_preserves_exact_classes_and_stays_blocked(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as directory:
            closure_path = Path(directory) / "source-semantic-closure.json"
            closure_path.write_bytes(semantic_closure.json_bytes(self.closure))
            candidate, receipt, registry = source_contract.build_source_contract(
                SOURCE_RECEIPT,
                ACTION_CUE_RECIPE,
                ACTIVE_INVENTORY,
                NORMALIZED_GRAPH,
                EXTERNAL_MODULE_CLOSURE,
                MATERIAL_CLOSURE,
                SOURCE_EVIDENCE,
                LOCAL_REFERENCE_CLOSURE,
                GEOMETRY_PARITY,
                "artist-f-source-contract.test.candidate.json",
                "artist-f-source-contract.test.registry.json",
                source_semantic_closure_path=closure_path,
            )

        module_coverage: list[dict[str, Any]] = []
        for element in candidate["elements"]:
            recipe = element["sourceRecipe"]
            raw_modules = recipe["modules"]
            coverage_rows = recipe["moduleCoverage"]
            self.assertEqual(len(coverage_rows), len(raw_modules))
            for raw_module, coverage in zip(raw_modules, coverage_rows):
                self.assertIn("exactSourceClass", coverage)
                self.assertIn("aliasId", coverage)
                self.assertEqual(
                    coverage["exactSourceClass"], raw_module["className"]
                )
                self.assertEqual(
                    coverage["normalizedClass"],
                    coverage["exactSourceClass"].casefold(),
                )
                self.assertEqual(coverage["aliasId"], "")
                self.assertEqual(coverage["status"], "unresolved")
                self.assertTrue(coverage["blockers"])
            module_coverage.extend(coverage_rows)

        self.assertEqual(len(module_coverage), 399)
        risky_rows = [
            row
            for row in module_coverage
            if row["exactSourceClass"].casefold().startswith("efparticlemodule")
            or row["exactSourceClass"].casefold().endswith("_seeded")
        ]
        self.assertEqual(len(risky_rows), 26)
        self.assertFalse(receipt["summary"]["sourceContractRuntimeAdmission"])
        self.assertFalse(receipt["productAdmission"]["allowed"])
        self.assertFalse(registry["runtimeAdmission"])

    def test_lod_and_point_light_blockers_reach_all_opt_in_coverage(
        self,
    ) -> None:
        lod_blockers = {
            "SELECTED_LOD_ENABLED_CLASS_DEFAULT_UNRESOLVED",
            "SELECTED_LOD_LEVEL_CLASS_DEFAULT_UNRESOLVED",
        }
        modules = self.modules(self.closure)
        self.assertEqual(len(modules), 399)
        self.assertEqual(
            [
                module["moduleOccurrenceId"]
                for module in modules
                if not lod_blockers.issubset(module["executionBlockers"])
            ],
            [],
        )

        point_light = self.closure["pointLightBindings"][0]
        self.assertEqual(len(point_light["fields"]), 8)
        field_blockers = {
            blocker
            for field in point_light["fields"]
            for blocker in field["executionBlockers"]
        }
        self.assertEqual(
            field_blockers,
            {
                "DOWNSTREAM_LIGHT_HANDLER_RECEIPT_REQUIRED",
                "INDEPENDENT_LIGHT_NUMERIC_OR_SEMANTIC_ORACLE_REQUIRED",
                "POINT_LIGHT_DEFAULT_FIELD_EXECUTION_UNRESOLVED",
                "POINT_LIGHT_GUID_RUNTIME_SEMANTICS_OR_IRRELEVANCE_UNPROVEN",
            },
        )
        binding_blockers = set(point_light["executionBlockers"])
        self.assertEqual(binding_blockers, {"LIGHT_RENDERER_NOT_COMPILED"})
        point_light_blockers = field_blockers | binding_blockers
        point_light_module = next(
            module
            for module in modules
            if module["moduleOccurrenceId"] == point_light["moduleOccurrenceId"]
        )
        self.assertTrue(
            point_light_blockers.issubset(
                point_light_module["executionBlockers"]
            )
        )

        point_light_occurrence = next(
            occurrence
            for occurrence in self.closure["occurrences"]
            if point_light_module in occurrence["modules"]
        )
        active_row = next(
            row
            for row in self.active_inventory["activeElements"]
            if row["activeElementId"] == point_light_occurrence["evidenceId"]
        )
        with tempfile.TemporaryDirectory() as directory:
            closure_path = Path(directory) / "source-semantic-closure.json"
            closure_path.write_bytes(semantic_closure.json_bytes(self.closure))
            candidate, receipt, _registry = source_contract.build_source_contract(
                SOURCE_RECEIPT,
                ACTION_CUE_RECIPE,
                ACTIVE_INVENTORY,
                NORMALIZED_GRAPH,
                EXTERNAL_MODULE_CLOSURE,
                MATERIAL_CLOSURE,
                SOURCE_EVIDENCE,
                LOCAL_REFERENCE_CLOSURE,
                GEOMETRY_PARITY,
                "artist-f-source-contract.test.candidate.json",
                "artist-f-source-contract.test.registry.json",
                source_semantic_closure_path=closure_path,
            )

        coverage_rows = [
            coverage
            for element in candidate["elements"]
            for coverage in element["sourceRecipe"]["moduleCoverage"]
        ]
        self.assertEqual(len(coverage_rows), 399)
        self.assertEqual(
            [
                coverage["moduleStableId"]
                for coverage in coverage_rows
                if not lod_blockers.issubset(coverage["blockers"])
            ],
            [],
        )
        point_light_element = next(
            element
            for element in candidate["elements"]
            if element["id"] == active_row["selectedLegacyElementId"]
        )
        point_light_coverage = point_light_element["sourceRecipe"][
            "moduleCoverage"
        ][point_light_module["order"]]
        self.assertTrue(
            point_light_blockers.issubset(point_light_coverage["blockers"])
        )
        self.assertEqual(self.oracle_result["productAdmission"], "0/35")
        self.assertEqual(
            (
                self.closure["productAdmission"]["admittedOccurrenceCount"],
                self.closure["productAdmission"]["totalOccurrenceCount"],
            ),
            (0, 35),
        )
        self.assertFalse(receipt["productAdmission"]["allowed"])

    def test_distribution_ids_are_stable_and_510_definitions_are_reused(self) -> None:
        distributions = self.distributions(self.closure)
        self.assertEqual(len(distributions), 629)
        self.assertEqual(len({row["occurrenceId"] for row in distributions}), 629)
        self.assertEqual(len({row["distributionId"] for row in distributions}), 629)
        definitions = Counter(row["definitionId"] for row in distributions)
        references_by_definition: dict[str, set[str]] = {}
        for row in distributions:
            references_by_definition.setdefault(row["definitionId"], set()).add(
                row["referenceId"]
            )
        self.assertEqual(len(definitions), 510)
        self.assertEqual(sum(count - 1 for count in definitions.values()), 119)
        self.assertTrue(any(count > 1 for count in definitions.values()))
        self.assertTrue(
            all(len(reference_ids) == 1 for reference_ids in references_by_definition.values())
        )
        rebuilt = semantic_closure.build_semantic_closure(
            self.active_inventory,
            self.normalized_graph,
            self.external_module_closure,
            self.source_evidence,
            self.local_reference_closure,
        )
        self.assertEqual(rebuilt["closureSha256"], self.closure["closureSha256"])
        self.assertEqual(
            [row["occurrenceId"] for row in self.distributions(rebuilt)],
            [row["occurrenceId"] for row in distributions],
        )

    def test_resealed_exact_source_class_mutation_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            self.modules(closure)[0]["exactSourceClass"] += "_mutated"

        self.assert_resealed_mutation_rejected(mutate, "module evidence binding changed")

    def test_resealed_alias_mutation_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            module = next(
                row
                for row in self.modules(closure)
                if row["exactSourceClass"].casefold().startswith("efparticlemodule")
            )
            module["normalizedClass"] = semantic_closure.legacy_normalized_class(
                module["exactSourceClass"]
            )
            module["aliasId"] = "unreviewed.automatic-ef-prefix.v1"
            module["aliasEvidenceId"] = "self-signed"

        self.assert_resealed_mutation_rejected(mutate, "unapproved source class alias")

    def test_resealed_source_fidelity_mutation_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            module = next(
                row
                for row in self.modules(closure)
                if row["sourceDocument"] == "externalModuleClosure"
            )
            module["sourceFidelity"] = "SOURCE_EXACT"

        self.assert_resealed_mutation_rejected(mutate, "module evidence binding changed")

    def test_resealed_native_tail_promotion_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            module = next(
                row
                for row in self.modules(closure)
                if row["nativeTail"]["classification"] == "UNRESOLVED"
            )
            module["nativeTail"]["classification"] = "VERIFIED_IRRELEVANT"
            module["nativeTail"]["oracleId"] = "self-signed.native-tail.v1"

        self.assert_resealed_mutation_rejected(
            mutate, "external native tail was silently ignored"
        )

    def test_resealed_seed_promotion_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            seed = next(
                row["seed"]
                for row in self.modules(closure)
                if isinstance(row["seed"], dict)
            )
            seed["classification"] = "EXECUTION_CONSUMED"
            seed["oracleStatus"] = "SELF_SIGNED"

        self.assert_resealed_mutation_rejected(mutate, "seed semantics were laundered")

    def test_resealed_implicit_default_promotion_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            default = next(
                row["implicitDefaults"][0]
                for row in self.modules(closure)
                if row["implicitDefaults"]
            )
            default["sourceFidelity"] = "SOURCE_EXACT"

        self.assert_resealed_mutation_rejected(mutate, "implicit default was promoted")

    def test_resealed_selected_lod_default_value_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            field = closure["occurrences"][0]["selectedLodSemantics"]["fields"][0]
            field["encodedValue"] = 0

        self.assert_resealed_mutation_rejected(mutate, "selected LOD default was laundered")

    def test_resealed_zero_default_distribution_laundering_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            distribution = next(
                row
                for row in self.distributions(closure)
                if row["fieldEvidence"]["defaultDependent"]
            )
            distribution["fieldEvidence"]["defaultDependent"] = False
            distribution["executionBlockers"] = [
                blocker
                for blocker in distribution["executionBlockers"]
                if blocker != "DISTRIBUTION_CLASS_DEFAULT_VALUE_UNRESOLVED"
            ]
            distribution["executionAdmission"] = {
                "allowed": False,
                "blockers": sorted(
                    {
                        *distribution["artifactBindingBlockers"],
                        *distribution["executionBlockers"],
                    }
                ),
            }

        self.assert_resealed_mutation_rejected(
            mutate, "inline distribution evidence was laundered"
        )

    def test_resealed_blocker_axis_mutation_is_rejected(self) -> None:
        token = "SOURCE_ERA_MODULE_PACKAGE_IDENTITY_NOT_PINNED"

        def mutate(closure: dict[str, Any]) -> None:
            module = next(
                row
                for row in self.modules(closure)
                if token in row["evidenceBlockers"]
            )
            module["evidenceBlockers"].remove(token)
            module["productBlockers"] = sorted(
                {*module["productBlockers"], token}
            )

        self.assert_resealed_mutation_rejected(
            mutate, "external module was laundered to SOURCE_EXACT"
        )

    def test_resealed_row_removal_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            module = next(
                row for row in self.modules(closure) if row["properties"]
            )
            module["properties"].pop()

        self.assert_resealed_mutation_rejected(mutate, "top-level property set changed")

    def test_resealed_duplicate_occurrence_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            closure["occurrences"].append(
                copy.deepcopy(closure["occurrences"][0])
            )

        self.assert_resealed_mutation_rejected(
            mutate, "semantic occurrence rows are duplicated or incomplete"
        )

    def test_resealed_duplicate_nested_rows_are_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            module = next(row for row in self.modules(closure) if row["properties"])
            module["properties"].append(copy.deepcopy(module["properties"][0]))

        self.assert_resealed_mutation_rejected(
            mutate, "top-level property set changed"
        )

    def test_resealed_duplicate_local_binding_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            closure["localDistributionBindings"].append(
                copy.deepcopy(closure["localDistributionBindings"][0])
            )

        self.assert_resealed_mutation_rejected(
            mutate, "local distribution binding denominator changed"
        )

    def test_resealed_unresolved_execution_opening_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            module = self.modules(closure)[0]
            module["evidenceBlockers"] = sorted(
                {
                    *module["evidenceBlockers"],
                    *module["executionBlockers"],
                }
            )
            module["executionBlockers"] = []
            module["executionAdmission"] = {"allowed": True, "blockers": []}
            product_blockers = sorted(
                {
                    *module["artifactBindingBlockers"],
                    *module["productBlockers"],
                }
            )
            module["productAdmission"] = {
                "allowed": not product_blockers,
                "blockers": product_blockers,
            }

        self.assert_resealed_mutation_rejected(
            mutate,
            (
                "unresolved row has no execution-closing blocker|"
                "blocker lineage changed: evidenceBlockers"
            ),
        )

    def test_resealed_local_lineage_forgery_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            local = next(
                row
                for row in closure["localDistributionBindings"]
                if row["sourceFidelity"] == "CURRENT_REVISION_EVIDENCE"
            )
            occurrence_id = local["occurrenceId"]
            module_row = next(
                row
                for row in self.distributions(closure)
                if row["occurrenceId"] == occurrence_id
            )
            for row in (local, module_row):
                row["sourceFidelity"] = "SOURCE_EXACT"
                row["exactSourceClass"] = "distributionfloatconstant"
                row["evaluatorId"] = "fallback.zero.current-revision.v1"
                row["definitionId"] = "distribution-definition::forged"
                row["referenceId"] = "distribution-reference::forged"

        self.assert_resealed_mutation_rejected(
            mutate, "local distribution decision changed"
        )

    def test_resealed_duplicate_point_light_field_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            fields = closure["pointLightBindings"][0]["fields"]
            fields.append(copy.deepcopy(fields[0]))

        self.assert_resealed_mutation_rejected(
            mutate, "PointLight semantic field denominator changed"
        )

    def test_resealed_subject_identity_mutation_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            closure["skillId"] = 31471

        self.assert_resealed_mutation_rejected(
            mutate, "semantic closure subject identity changed"
        )

    def test_resealed_external_property_fidelity_promotion_is_rejected(
        self,
    ) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            module = next(
                row
                for row in self.modules(closure)
                if row["sourceDocument"] == "externalModuleClosure"
                and row["properties"]
            )
            property_path = module["properties"][0]["propertyPath"]
            module["properties"][0]["sourceFidelity"] = "SOURCE_EXACT"
            for leaf in module["primitiveLeaves"]:
                if leaf["topLevelPropertyPath"] == property_path:
                    leaf["sourceFidelity"] = "SOURCE_EXACT"

        self.assert_resealed_mutation_rejected(
            mutate, "top-level property decision changed"
        )

    def test_resealed_external_distribution_fidelity_promotion_is_rejected(
        self,
    ) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            distribution = next(
                row
                for module in self.modules(closure)
                if module["sourceDocument"] == "externalModuleClosure"
                for row in module["distributions"]
                if "legacyOccurrenceId" not in row
            )
            distribution["sourceFidelity"] = "SOURCE_EXACT"

        self.assert_resealed_mutation_rejected(
            mutate, "inline distribution identity changed"
        )

    def test_resealed_point_light_selected_provenance_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            radius = next(
                row
                for row in closure["pointLightBindings"][0]["fields"]
                if row["fieldName"] == "radius"
            )
            radius["selectedTier"] = "INSTANCE_EXPLICIT"
            radius["selectedEvidenceStatus"] = "SOURCE_EXACT_PHYSICAL_PACKAGE"

        self.assert_resealed_mutation_rejected(
            mutate, "PointLight field decision changed"
        )

    def test_resealed_summary_fidelity_projection_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            closure["summary"]["moduleSourceFidelityCounts"] = {
                "SOURCE_EXACT": 399
            }

        self.assert_resealed_mutation_rejected(
            mutate, "semantic closure summary is not an independent projection"
        )

    def test_resealed_implicit_default_identity_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            default = next(
                module["implicitDefaults"][0]
                for module in self.modules(closure)
                if module["implicitDefaults"]
            )
            default["defaultId"] = "duplicate-default-id"
            default["fieldPath"] = "fallback.zero"
            default["evaluatorId"] = "fallback.zero.current.v1"
            default["oracleStatus"] = "NUMERICALLY_VERIFIED"

        self.assert_resealed_mutation_rejected(
            mutate, "implicit default was promoted"
        )

    def test_resealed_seed_evaluator_forgery_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            seed = next(
                module["seed"]
                for module in self.modules(closure)
                if isinstance(module["seed"], dict)
            )
            seed["evaluatorId"] = "fallback.zero.current.v1"
            seed["oracleStatus"] = "NUMERICALLY_VERIFIED"

        self.assert_resealed_mutation_rejected(
            mutate, "seed semantics were laundered"
        )

    def test_resealed_random_sample_order_opening_is_rejected(self) -> None:
        def mutate(closure: dict[str, Any]) -> None:
            distribution = next(
                row
                for row in self.distributions(closure)
                if row["fieldEvidence"]["explicitOperation"] in {2, 3}
            )
            distribution["executionBlockers"].remove(
                "DISTRIBUTION_RANDOM_STREAM_SAMPLE_ORDER_UNVERIFIED"
            )
            self.recompute_admissions(distribution)

        self.assert_resealed_mutation_rejected(
            mutate,
            (
                "distribution random sample order was opened|"
                "blocker lineage changed: executionBlockers"
            ),
        )

    def test_resealed_target007_raw_lineage_blocker_removal_is_rejected(
        self,
    ) -> None:
        changed = copy.deepcopy(self.closure)
        local = next(
            row
            for row in changed["localDistributionBindings"]
            if row["legacyReferenceId"] == "distribution-target-007"
        )
        nested = next(
            row
            for row in self.distributions(changed)
            if row.get("legacyOccurrenceId") == local["legacyOccurrenceId"]
        )
        blocker = "SOURCE_ERA_TARGET_PACKAGE_IDENTITY_NOT_PINNED"
        for row in (local, nested):
            self.assertIn(blocker, row["evidenceBlockers"])
            row["evidenceBlockers"].remove(blocker)
        self.reseal(changed)

        with self.assertRaises(ValueError):
            semantic_oracle.verify_semantic_closure(
                changed,
                self.active_inventory,
                self.normalized_graph,
                self.external_module_closure,
                self.source_evidence,
                self.local_reference_closure,
            )

    def test_resealed_reconstructed_field_blocker_removal_is_rejected(
        self,
    ) -> None:
        blocker_by_field = {
            "operation": "DISTRIBUTION_OPERATION_RECONSTRUCTION_UNVERIFIED",
            "lookupTableChunkSize": (
                "DISTRIBUTION_LOOKUP_CHUNK_RECONSTRUCTION_UNVERIFIED"
            ),
            "lookupTableNumElements": (
                "DISTRIBUTION_LOOKUP_COUNT_RECONSTRUCTION_UNVERIFIED"
            ),
        }
        for field_name, blocker in blocker_by_field.items():
            with self.subTest(field_name=field_name):
                changed = copy.deepcopy(self.closure)
                distribution = next(
                    row
                    for row in self.distributions(changed)
                    if field_name in row["fieldEvidence"]["reconstructedFieldNames"]
                    and blocker in row["executionBlockers"]
                )
                distribution["executionBlockers"].remove(blocker)
                self.recompute_admissions(distribution)
                self.reseal(changed)

                with self.assertRaises(ValueError):
                    semantic_oracle.verify_semantic_closure(
                        changed,
                        self.active_inventory,
                        self.normalized_graph,
                        self.external_module_closure,
                        self.source_evidence,
                        self.local_reference_closure,
                    )

    def test_resealed_property_distribution_ids_forgery_is_rejected(self) -> None:
        changed = copy.deepcopy(self.closure)
        prop = next(
            prop
            for module in self.modules(changed)
            for prop in module["properties"]
            if prop["distributionIds"]
        )
        prop["distributionIds"] = [
            "forged::distribution:zero",
            "forged::distribution:zero",
        ]
        self.reseal(changed)

        with self.assertRaises(ValueError):
            semantic_oracle.verify_semantic_closure(
                changed,
                self.active_inventory,
                self.normalized_graph,
                self.external_module_closure,
                self.source_evidence,
                self.local_reference_closure,
            )

    def test_resealed_selected_lod_oracle_forgery_is_rejected(self) -> None:
        changed = copy.deepcopy(self.closure)
        field = changed["occurrences"][0]["selectedLodSemantics"]["fields"][0]
        field["evaluatorId"] = "fallback.zero.current.v1"
        field["oracleStatus"] = "NUMERICALLY_VERIFIED"
        self.reseal(changed)

        with self.assertRaises(ValueError):
            semantic_oracle.verify_semantic_closure(
                changed,
                self.active_inventory,
                self.normalized_graph,
                self.external_module_closure,
                self.source_evidence,
                self.local_reference_closure,
            )

    def test_resealed_point_light_consumer_and_axes_forgery_is_rejected(
        self,
    ) -> None:
        changed = copy.deepcopy(self.closure)
        radius = next(
            row
            for row in changed["pointLightBindings"][0]["fields"]
            if row["fieldName"] == "radius"
        )
        radius["consumerId"] = "fallback.zero.current.v1"
        radius["evidenceBlockers"] = []
        radius["executionBlockers"] = ["GENERIC_PENDING"]
        self.recompute_admissions(radius)
        self.reseal(changed)

        with self.assertRaises(ValueError):
            semantic_oracle.verify_semantic_closure(
                changed,
                self.active_inventory,
                self.normalized_graph,
                self.external_module_closure,
                self.source_evidence,
                self.local_reference_closure,
            )

    def test_resealed_module_axis_shift_is_rejected_and_bridge_preserves_union(
        self,
    ) -> None:
        changed = copy.deepcopy(self.closure)
        occurrence = changed["occurrences"][0]
        module = occurrence["modules"][0]
        blocker = "DOWNSTREAM_MODULE_HANDLER_RECEIPT_REQUIRED"
        self.assertIn(blocker, module["executionBlockers"])
        module["executionBlockers"].remove(blocker)
        module["artifactBindingBlockers"] = sorted(
            {*module["artifactBindingBlockers"], blocker}
        )
        self.recompute_admissions(module)
        self.reseal(changed)

        with self.assertRaises(ValueError):
            semantic_oracle.verify_semantic_closure(
                changed,
                self.active_inventory,
                self.normalized_graph,
                self.external_module_closure,
                self.source_evidence,
                self.local_reference_closure,
            )

        active_row = next(
            row
            for row in self.active_inventory["activeElements"]
            if row["activeElementId"] == occurrence["evidenceId"]
        )
        with tempfile.TemporaryDirectory() as directory:
            closure_path = Path(directory) / "source-semantic-closure.json"
            closure_path.write_bytes(semantic_closure.json_bytes(changed))
            with mock.patch.object(
                source_contract,
                "verify_source_semantic_closure",
            ) as verify_mock:
                candidate, _receipt, _registry = (
                    source_contract.build_source_contract(
                        SOURCE_RECEIPT,
                        ACTION_CUE_RECIPE,
                        ACTIVE_INVENTORY,
                        NORMALIZED_GRAPH,
                        EXTERNAL_MODULE_CLOSURE,
                        MATERIAL_CLOSURE,
                        SOURCE_EVIDENCE,
                        LOCAL_REFERENCE_CLOSURE,
                        GEOMETRY_PARITY,
                        "artist-f-source-contract.test.candidate.json",
                        "artist-f-source-contract.test.registry.json",
                        source_semantic_closure_path=closure_path,
                    )
                )
                verify_mock.assert_called_once()

        element = next(
            row
            for row in candidate["elements"]
            if row["id"] == active_row["selectedLegacyElementId"]
        )
        coverage = element["sourceRecipe"]["moduleCoverage"][module["order"]]
        self.assertIn(blocker, coverage["blockers"])


if __name__ == "__main__":
    unittest.main()
