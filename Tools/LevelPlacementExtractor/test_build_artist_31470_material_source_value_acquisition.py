#!/usr/bin/env python3
"""Mutation tests for the Artist F Material source-value acquisition receipt."""

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parents[1]
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

import build_artist_31470_material_source_value_acquisition as acquisition


class MaterialSourceValueAcquisitionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.contract_path = (
            REPO_ROOT
            / "Data/Effects/Imported/Artist/Materials/skill.31470.typed-material-evidence-contract.json"
        )
        cls.runtime_path = (
            REPO_ROOT
            / "Data/Effects/Imported/Artist/Materials/skill.31470.material-runtime-oracle.receipt.json"
        )
        cls.render_path = (
            REPO_ROOT
            / "Data/Effects/Imported/Artist/Materials/skill.31470.material-render-state-evidence.receipt.json"
        )
        cls.shader_path = (
            REPO_ROOT
            / "Data/Effects/Imported/Artist/Materials/skill.31470.shader-cache-oracle.receipt.json"
        )
        cls.output_path = (
            REPO_ROOT
            / "Data/Effects/Imported/Artist/Materials/skill.31470.material-source-value-acquisition.receipt.json"
        )
        cls.source_archive_root = Path(
            r"C:\Users\user\Desktop\Resource_LostArk\00_SourcePackages"
        )
        cls.source_pack_root = (
            cls.source_archive_root / "Effect_DIMENSIONMASTER_20260803_v3"
        )
        cls.current_install_root = Path(
            r"C:\ProgramData\Smilegate\Games\LOSTARK"
        )
        cls.contract = acquisition.load_json(cls.contract_path)
        cls.runtime = acquisition.load_json(cls.runtime_path)
        cls.render = acquisition.load_json(cls.render_path)
        cls.shader = acquisition.load_json(cls.shader_path)
        cls.committed = acquisition.load_json(cls.output_path)
        cls.rebuilt = acquisition.build_receipt(
            cls.contract,
            cls.runtime,
            cls.render,
            cls.shader,
            cls.contract_path,
            cls.runtime_path,
            cls.render_path,
            cls.shader_path,
            cls.source_archive_root,
            cls.source_pack_root,
            cls.current_install_root,
        )

    @classmethod
    def validate(cls, receipt: dict) -> None:
        acquisition.validate_receipt(
            receipt,
            cls.contract,
            cls.runtime,
            cls.render,
            cls.shader,
            cls.contract_path,
            cls.runtime_path,
            cls.render_path,
            cls.shader_path,
            cls.source_archive_root,
            cls.source_pack_root,
            cls.current_install_root,
        )

    @staticmethod
    def reseal(receipt: dict) -> None:
        receipt.pop("receiptSha256", None)
        receipt["receiptSha256"] = acquisition.canonical_sha256(receipt)

    def test_committed_receipt_matches_raw_sources(self) -> None:
        self.assertEqual(self.committed, self.rebuilt)
        self.validate(self.committed)

    def test_static_guid_value_and_override_are_raw_source_owned(self) -> None:
        rows = self.committed["matrices"]["staticPermutationRows"]
        self.assertEqual(
            66,
            sum(row["micNativeSelection"]["exactNameAndGuidMatchCount"] for row in rows),
        )
        self.assertEqual(23, sum(row["sourceValueAcquired"] for row in rows))
        self.assertEqual(
            43,
            sum("NONOVERRIDE_ENTRY" in row["sourceValueDecision"] for row in rows),
        )
        self.assertEqual(
            28,
            sum(row["micNativeSelection"]["exactNameAndGuidMatchCount"] == 0 for row in rows),
        )
        for row in rows:
            selection = row["micNativeSelection"]
            if selection["exactNameAndGuidMatchCount"] != 1:
                continue
            entry = selection["entry"]
            self.assertEqual(
                row["parentExpression"]["expressionGuidHex"],
                entry["rawExpressionGuidHex"],
            )
            self.assertEqual(
                entry["value"],
                bool(int.from_bytes(bytes.fromhex(entry["rawValueUint32Hex"]), "little")),
            )
            self.assertEqual(
                entry["bOverride"],
                bool(
                    int.from_bytes(
                        bytes.fromhex(entry["rawOverrideUint32Hex"]), "little"
                    )
                ),
            )

    def test_coordinated_static_guid_reseal_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.committed)
        row = next(
            row
            for row in mutated["matrices"]["staticPermutationRows"]
            if row["sourceValueAcquired"]
        )
        replacement = "00" * 16
        row["parentExpression"]["expressionGuidHex"] = replacement
        row["parentExpression"]["expressionGuidProperty"]["value"]["hex"] = replacement
        row["micNativeSelection"]["entry"]["expressionGuidHex"] = replacement
        row["micNativeSelection"]["entry"]["rawExpressionGuidHex"] = replacement
        mutated["summary"]["staticRowSetSha256"] = acquisition.canonical_sha256(
            mutated["matrices"]["staticPermutationRows"]
        )
        self.reseal(mutated)
        with self.assertRaisesRegex(ValueError, "stale or laundered"):
            self.validate(mutated)

    def test_previous_exact_four_are_blocked_and_denominator_is_72(self) -> None:
        rows = self.committed["matrices"]["strictSamplerRows"]
        exact = [
            row
            for row in rows
            if row["baselineKind"] == "PREVIOUSLY_ADMITTED_EXACT_REAUDIT"
        ]
        self.assertEqual(72, len(rows))
        self.assertEqual(4, len(exact))
        self.assertEqual(71, sum(row["bindingOriginAndOwner"]["bindingOrigin"] == "INSTANCE_OVERRIDE" for row in rows))
        self.assertEqual(1, sum(row["bindingOriginAndOwner"]["bindingOrigin"] == "PARENT_DEFAULT" for row in rows))
        self.assertTrue(all(row["strictReauditDecision"] == "BLOCKED" for row in exact))
        self.assertTrue(all(not row["fullDescriptorSourceExact"] for row in rows))
        self.assertEqual(
            3,
            sum(
                "srgb" in row["partialSourceExactFields"]
                for row in rows
            ),
        )
        self.assertEqual(
            9,
            sum("addressx" in row["partialSourceExactFields"] for row in rows),
        )

    def test_omitted_default_cannot_be_resealed_as_source_exact(self) -> None:
        mutated = copy.deepcopy(self.committed)
        row = next(
            row
            for row in mutated["matrices"]["strictSamplerRows"]
            if row["baselineKind"] == "PREVIOUSLY_ADMITTED_EXACT_REAUDIT"
            and row["textureExportEvidence"]["fields"]["addressx"]["status"]
            == "OMITTED_FROM_EXPORT"
        )
        row["fullDescriptorSourceExact"] = True
        row["sourceValueAcquired"] = True
        row["sourceValueDecision"] = "SOURCE_EXACT_FULL_DESCRIPTOR"
        row["strictReauditDecision"] = "PASS"
        row["executionReady"] = True
        mutated["summary"]["strictSamplerSourceValueAcquiredCount"] = 1
        mutated["summary"]["strictExecutionReadyCount"] = 1
        mutated["summary"]["strictSamplerRowSetSha256"] = acquisition.canonical_sha256(
            mutated["matrices"]["strictSamplerRows"]
        )
        self.reseal(mutated)
        with self.assertRaisesRegex(ValueError, "stale or laundered"):
            self.validate(mutated)

    def test_execution_and_product_remain_closed(self) -> None:
        admission = self.committed["admission"]
        self.assertFalse(admission["upstreamFrozen627EvidenceIntegrity"])
        self.assertFalse(admission["executionReady"])
        self.assertFalse(admission["product"])
        self.assertFalse(admission["r2Entry"])
        self.assertEqual(0, self.committed["summary"]["strictExecutionReadyCount"])
        self.assertEqual(0, self.committed["summary"]["productCount"])


if __name__ == "__main__":
    unittest.main()
