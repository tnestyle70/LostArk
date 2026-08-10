#!/usr/bin/env python3
"""Mutation tests for the Artist F Material oracle-acquisition receipt."""

from __future__ import annotations

import copy
import json
import unittest
from pathlib import Path

import build_artist_31470_material_oracle_acquisition as oracle


REPO_ROOT = Path(__file__).resolve().parents[2]
MATERIAL_ROOT = REPO_ROOT / "Data/Effects/Imported/Artist/Materials"
CONTRACT_PATH = MATERIAL_ROOT / "skill.31470.typed-material-evidence-contract.json"
RAW_RECEIPT_PATH = (
    MATERIAL_ROOT / "skill.31470.material-render-state-evidence.receipt.json"
)
CLOSURE_PATH = MATERIAL_ROOT / "skill.31470.active-material-closure.json"
ORACLE_PATH = MATERIAL_ROOT / "skill.31470.material-oracle-acquisition.receipt.json"


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def reseal(value: dict) -> dict:
    value.pop("receiptSha256", None)
    value["receiptSha256"] = oracle.canonical_sha256(value)
    return value


class Artist31470MaterialOracleAcquisitionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.contract = load(CONTRACT_PATH)
        cls.raw_receipt = load(RAW_RECEIPT_PATH)
        cls.closure = load(CLOSURE_PATH)
        cls.receipt = load(ORACLE_PATH)

    def validate(self, receipt: dict) -> None:
        oracle.validate_receipt(
            receipt,
            self.contract,
            self.raw_receipt,
            self.closure,
            CONTRACT_PATH,
            RAW_RECEIPT_PATH,
            CLOSURE_PATH,
        )

    def test_checked_receipt_and_denominators(self) -> None:
        self.validate(copy.deepcopy(self.receipt))
        summary = self.receipt["summary"]
        self.assertEqual(23, summary["materialFamilyCount"])
        self.assertEqual(27, summary["materialRecipeCount"])
        self.assertEqual(925, summary["survivingExpressionCount"])
        self.assertEqual(1803, summary["cookedNullExpressionCount"])
        self.assertEqual(125, summary["resolvedInputEdgeCount"])
        self.assertEqual(502, summary["unresolvedInputEdgeCount"])
        self.assertEqual(0, summary["oracleAvailableFamilyCount"])
        self.assertEqual(0, summary["implementedEvaluatorCount"])
        self.assertEqual(0, summary["productFamilyCount"])
        self.assertEqual(23, summary["installedMaterialLeafFamilyCount"])
        self.assertEqual(1596, summary["shaderCacheExportCount"])
        self.assertEqual(11, summary["selectedShaderCacheCandidateCount"])

    def test_exact_json_integer_and_root_identity_cannot_be_resealed(self) -> None:
        for value in (True, 1.0, "1"):
            mutated = copy.deepcopy(self.receipt)
            mutated["formatVersion"] = value
            with self.assertRaises(ValueError):
                self.validate(reseal(mutated))
        for name, value in (
            ("root", "WARLORD/31470/F"),
            ("characterClass", "WARLORD"),
            ("skillId", 999),
            ("inputSlot", "Q"),
        ):
            mutated = copy.deepcopy(self.receipt)
            mutated[name] = value
            with self.assertRaises(ValueError):
                self.validate(reseal(mutated))

    def test_environment_search_and_script_identity_cannot_be_resealed(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        shader = mutated["environmentSearch"]["shaderCache"]
        shader["candidates"][0]["serialSha256"] = "3" * 64
        shader["candidateSha256"] = oracle.canonical_sha256(
            shader["candidates"]
        )
        with self.assertRaises(ValueError):
            self.validate(reseal(mutated))

        mutated = copy.deepcopy(self.receipt)
        mutated["environmentSearch"]["scriptPackages"][0]["sha256"] = "1" * 64
        with self.assertRaises(ValueError):
            self.validate(reseal(mutated))

        mutated = copy.deepcopy(self.receipt)
        leaves = mutated["environmentSearch"]["installedMaterialLeaves"]
        leaves["families"][0]["exportIndex"] += 1
        leaves["familyEvidenceSha256"] = oracle.canonical_sha256(
            leaves["families"]
        )
        with self.assertRaises(ValueError):
            self.validate(reseal(mutated))

        mutated = copy.deepcopy(self.receipt)
        search = mutated["environmentSearch"]["shaderCache"][
            "nativeStateKeyJoinSearch"
        ]
        search["matchCount"] = 1
        search["matches"] = [
            {
                "stateKeyHex": "00" * 16,
                "shaderCacheExportIndex": 311,
                "shaderCacheObjectPath": "sc_lv_customizingtool_classselect_sl01",
                "offsetInShaderCacheSerial": 0,
            }
        ]
        search["outcome"] = "DIRECT_STATE_KEY_BINDING_FOUND"
        with self.assertRaises(ValueError):
            self.validate(reseal(mutated))

    def test_native_tail_and_node_evidence_cannot_be_resealed(self) -> None:
        for mutator in (
            lambda family: family["baseMaterialRawEvidence"].__setitem__(
                "nativeTailSha256", "2" * 64
            ),
            lambda family: family["survivingEvidence"]["nodeTypeCounts"].__setitem__(
                "materialexpressionforged", 1
            ),
            lambda family: family["missingExactInputs"].__setitem__(
                "unresolvedInputEdgeCount",
                family["missingExactInputs"]["unresolvedInputEdgeCount"] - 1,
            ),
            lambda family: family["recipeAcquisition"][0].__setitem__(
                "nativeTailByteCount",
                family["recipeAcquisition"][0]["nativeTailByteCount"] + 1,
            ),
            lambda family: family["candidateArtifactsChecked"][1][
                "rawIdentity"
            ]["nativeStateKey"].__setitem__("hex", "ff" * 16),
        ):
            mutated = copy.deepcopy(self.receipt)
            mutator(mutated["families"][0])
            mutated["summary"]["familyAcquisitionSha256"] = oracle.canonical_sha256(
                mutated["families"]
            )
            with self.assertRaises(ValueError):
                self.validate(reseal(mutated))

    def test_oracle_evaluator_and_product_cannot_be_promoted(self) -> None:
        for field, value in (
            ("oracleStatus", "SOURCE_EXACT_NUMERIC_ORACLE"),
            ("sourceExactGraph", True),
            ("evaluatorImplemented", True),
            ("executable", True),
            ("product", True),
        ):
            mutated = copy.deepcopy(self.receipt)
            mutated["families"][0][field] = value
            mutated["summary"]["familyAcquisitionSha256"] = oracle.canonical_sha256(
                mutated["families"]
            )
            with self.assertRaises(ValueError):
                self.validate(reseal(mutated))

        mutated = copy.deepcopy(self.receipt)
        mutated["families"][0]["minimumIndependentNumericOracle"][
            "imageValidationAllowed"
        ] = True
        mutated["summary"]["familyAcquisitionSha256"] = oracle.canonical_sha256(
            mutated["families"]
        )
        with self.assertRaises(ValueError):
            self.validate(reseal(mutated))

    def test_recipe_and_family_rows_cannot_be_swapped_after_sealing(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        first = next(row for row in mutated["families"] if len(row["recipeAcquisition"]) > 1)
        first["recipeAcquisition"][0], first["recipeAcquisition"][1] = (
            first["recipeAcquisition"][1],
            first["recipeAcquisition"][0],
        )
        mutated["summary"]["familyAcquisitionSha256"] = oracle.canonical_sha256(
            mutated["families"]
        )
        with self.assertRaises(ValueError):
            self.validate(reseal(mutated))

        mutated = copy.deepcopy(self.receipt)
        mutated["families"][0], mutated["families"][1] = (
            mutated["families"][1],
            mutated["families"][0],
        )
        mutated["summary"]["familyAcquisitionSha256"] = oracle.canonical_sha256(
            mutated["families"]
        )
        with self.assertRaises(ValueError):
            self.validate(reseal(mutated))


if __name__ == "__main__":
    unittest.main()
