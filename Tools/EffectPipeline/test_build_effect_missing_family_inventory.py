#!/usr/bin/env python3
"""Unit tests for the Effect missing-family inventory.

The inventory's value is that its columns are read out of the code and data
they describe rather than restated by hand.  These tests pin that: the typed
executor registry has to come from the header, the opcode provenance from the
renderer switch and the shared evaluator, and a parent path that lost its
package segment must not be given one by guesswork.
"""

from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from build_effect_missing_family_inventory import (  # noqa: E402
    DEFAULT_OUTPUT,
    REPOSITORY_ROOT,
    InventoryError,
    build_inventory,
    canonical_sha256,
    index_bulk_cooked_dxbc,
    merge_cooked_dxbc_evidence,
    package_of,
    parse_bounded_profile_opcodes,
    parse_executor_opcodes,
    parse_typed_executor_registry,
    write_inventory,
)


def extracted_row(parent: str, digest: str = "a" * 64) -> dict:
    return {
        "parentMaterialPath": parent,
        "status": "EXTRACTED",
        "carrier": "sprite",
        "rendererType": "SpriteParticle",
        "childMaterialPath": "pkg.group.child",
        "permutationSelection": "SINGLE_PERMUTATION",
        "admits": "COOKED_PROGRAM_ONLY",
        "occurrenceCount": 3,
        "dxbcSha256": digest,
        "dxbcByteSize": 128,
    }


def blocked_row(parent: str) -> dict:
    return {
        "parentMaterialPath": parent,
        "status": "BLOCKED",
        "blocker": "structural pixel pass reference is ambiguous",
        "occurrenceCount": 2,
    }


def bulk_receipt(rows: list[dict]) -> dict:
    extracted = [row for row in rows if row["status"] == "EXTRACTED"]
    document = {
        "schema": "lostark.effect-family-cooked-pixel-shaders",
        "formatVersion": 1,
        "identity": {"admits": "COOKED_PROGRAM_ONLY"},
        "inputs": {},
        "summary": {
            "familyCount": len(rows),
            "extractedCount": len(extracted),
            "blockedCount": len(rows) - len(extracted),
            "extractedOccurrenceCount": sum(
                row["occurrenceCount"] for row in extracted),
        },
        "families": rows,
    }
    document["artifactSha256"] = canonical_sha256(document)
    return document


def write_receipt(path: Path, document: dict) -> None:
    path.write_text(json.dumps(document), encoding="utf-8")

HEADER_SAMPLE = """
inline EFFECT_STRICT_TYPED_SOURCE_PROFILE Resolve_EffectStrictTypedSourceProfile(
\tconst std::string_view strSourceMaterialPath,
\tconst EFFECT_SOURCE_MATERIAL_DESC& Source)
{
\tif (!Source.bEnabled)
\t{
\t\treturn EFFECT_STRICT_TYPED_SOURCE_PROFILE::NONE;
\t}
\tif (Source.strProfileId == "one" &&
\t\tSource.strParentMaterialPath ==
\t\t\t"pkg.grp.first_parent")
\t{
\t\treturn EFFECT_STRICT_TYPED_SOURCE_PROFILE::FIRST;
\t}
\tif ((Source.strParentMaterialPath ==
\t\t\t"pkg.grp.second_parent_tr") ||
\t\t (Source.strParentMaterialPath ==
\t\t\t"pkg.grp.second_parent_ad"))
\t{
\t\treturn EFFECT_STRICT_TYPED_SOURCE_PROFILE::SECOND;
\t}
\treturn EFFECT_STRICT_TYPED_SOURCE_PROFILE::NONE;
}
"""

RENDERER_SAMPLE = """
\t\tcase Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::FIRST:
\t\t\tif (shape == "sprite")
\t\t\t\treturn 20u;
\t\t\tbreak;
\t\tcase Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::SECOND:
\t\t\treturn 99u;
\t\tcase Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::NONE:
\t\t\treturn 0u;
"""


class TypedExecutorRegistryTest(unittest.TestCase):
    def test_every_parent_before_a_return_belongs_to_that_profile(self) -> None:
        registry = parse_typed_executor_registry(HEADER_SAMPLE)
        self.assertEqual(registry["pkg.grp.first_parent"], ["FIRST"])
        self.assertEqual(registry["pkg.grp.second_parent_tr"], ["SECOND"])
        self.assertEqual(registry["pkg.grp.second_parent_ad"], ["SECOND"])

    def test_none_return_claims_no_parent(self) -> None:
        registry = parse_typed_executor_registry(HEADER_SAMPLE)
        self.assertNotIn("NONE", {
            profile for profiles in registry.values() for profile in profiles})

    def test_a_missing_resolver_is_a_hard_failure(self) -> None:
        with self.assertRaises(InventoryError):
            parse_typed_executor_registry("// no resolver here\n")


class OpcodeProvenanceTest(unittest.TestCase):
    def test_executor_opcodes_come_from_the_renderer_switch(self) -> None:
        opcodes = parse_executor_opcodes(RENDERER_SAMPLE)
        self.assertEqual(opcodes["FIRST"], [20])
        self.assertEqual(opcodes["SECOND"], [99])

    def test_the_none_case_claims_no_opcode(self) -> None:
        self.assertNotIn("NONE", parse_executor_opcodes(RENDERER_SAMPLE))

    def test_bounded_opcodes_come_from_the_shared_evaluator(self) -> None:
        opcodes = parse_bounded_profile_opcodes(
            "if (0 == g_SourceMaterialProfile)\n"
            "else if (16 == g_SourceMaterialProfile || "
            "36 == g_SourceMaterialProfile)\n")
        self.assertEqual(opcodes, {0, 16, 36})


class PackageSegmentTest(unittest.TestCase):
    def test_a_fully_qualified_path_names_its_package(self) -> None:
        self.assertEqual(
            package_of("fx_m_mi_00.fx_m.fx_d_pa_master_01_tr"), "fx_m_mi_00")

    def test_a_two_segment_path_has_no_package(self) -> None:
        """`fx_m` is a group, not a package: guessing one invents provenance."""
        self.assertIsNone(package_of("fx_m.fx_d_pa_shine_02_ad"))


class BulkCookedPixelShaderReceiptTest(unittest.TestCase):
    def test_absent_required_receipt_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(InventoryError, "is absent"):
                index_bulk_cooked_dxbc(Path(directory) / "absent.json")

    def test_corrupt_required_receipt_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "corrupt.json"
            path.write_text("{not-json", encoding="utf-8")
            with self.assertRaisesRegex(InventoryError, "is corrupt"):
                index_bulk_cooked_dxbc(path)

    def test_tampered_receipt_identity_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "tampered.json"
            document = bulk_receipt([extracted_row("pkg.group.parent")])
            document["families"][0]["dxbcByteSize"] = 256
            write_receipt(path, document)
            with self.assertRaisesRegex(InventoryError, "artifactSha256"):
                index_bulk_cooked_dxbc(path)

    def test_identical_duplicate_parent_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "duplicate.json"
            row = extracted_row("pkg.group.parent")
            write_receipt(path, bulk_receipt([row, dict(row)]))
            with self.assertRaisesRegex(InventoryError, "duplicate bulk"):
                index_bulk_cooked_dxbc(path)

    def test_conflicting_duplicate_parent_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "conflict.json"
            first = extracted_row("pkg.group.parent", "a" * 64)
            second = extracted_row("pkg.group.parent", "b" * 64)
            write_receipt(path, bulk_receipt([first, second]))
            with self.assertRaisesRegex(InventoryError, "conflicting bulk"):
                index_bulk_cooked_dxbc(path)

    def test_blocked_row_cannot_also_claim_an_extracted_blob(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "contradiction.json"
            row = blocked_row("pkg.group.parent")
            row["dxbcSha256"] = "a" * 64
            write_receipt(path, bulk_receipt([row]))
            with self.assertRaisesRegex(InventoryError, "conflicts"):
                index_bulk_cooked_dxbc(path)

    def test_join_key_is_the_exact_full_parent_path(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "receipt.json"
            first = extracted_row("pkg_a.group.shared_parent", "a" * 64)
            second = extracted_row("pkg_b.group.shared_parent", "b" * 64)
            write_receipt(path, bulk_receipt([first, second]))
            index = index_bulk_cooked_dxbc(path)
            self.assertEqual(set(index), {
                "pkg_a.group.shared_parent",
                "pkg_b.group.shared_parent",
            })
            self.assertEqual(
                index["pkg_a.group.shared_parent"]["dxbcSha256"], "a" * 64)
            self.assertEqual(
                index["pkg_b.group.shared_parent"]["dxbcSha256"], "b" * 64)

    def test_distinct_canary_variant_is_preserved_not_called_a_conflict(
            self) -> None:
        bulk = {
            "pkg.group.parent": {
                "provenance": "BULK_FAMILY_COOKED_PIXEL_SHADER_RECEIPT",
                "dxbcSha256": "a" * 64,
                "exactPixelShaderBlob": True,
                "productRuntime": False,
                "openBlockers": [],
            },
        }
        canary = {
            "pkg.group.parent": {
                "provenance": "UE3_EXACT_CANARY_VARIANT_RECEIPT",
                "dxbcSha256": "b" * 64,
                "exactPixelShaderBlob": True,
                "productRuntime": False,
                "openBlockers": [],
            },
        }
        merged = merge_cooked_dxbc_evidence(bulk, canary)
        self.assertEqual(
            merged["pkg.group.parent"]["joinedDxbcSha256s"],
            ["a" * 64, "b" * 64])
        self.assertEqual(
            merged["pkg.group.parent"]["canaryVariant"]["dxbcSha256"],
            "b" * 64)


class CurrentCorpusBulkJoinTest(unittest.TestCase):
    def test_bulk_receipt_replaces_the_five_canary_false_denominator(
            self) -> None:
        missing = REPOSITORY_ROOT / "__missing_inventory_test_input__"
        inventory = build_inventory(REPOSITORY_ROOT, missing, missing)
        summary = inventory["summary"]
        self.assertEqual(summary["cookedShaderEvidenceStatusCounts"], {
            "DXBC_EXTRACTION_BLOCKED": 13,
            "EXACT_DXBC_JOINED": 180,
            "EXACT_DXBC_NOT_JOINED": 12,
        })
        self.assertEqual(
            summary["cookedShaderEvidenceStatusOccurrenceCounts"], {
                "DXBC_EXTRACTION_BLOCKED": 359,
                "EXACT_DXBC_JOINED": 2178,
                "EXACT_DXBC_NOT_JOINED": 209,
            })
        self.assertEqual(
            summary["restorationStatusCounts"][
                "DXBC_JOINED_RUNTIME_EXECUTOR_NOT_IMPLEMENTED"],
            142)
        self.assertEqual(
            summary["restorationStatusCounts"]["DXBC_EXTRACTION_BLOCKED"],
            11)
        self.assertEqual(
            summary["blockerCounts"][
                "EXACT_COOKED_PIXEL_SHADER_NOT_JOINED"],
            12)
        evidence_only = [
            family for family in inventory["families"]
            if family["restorationStatus"] ==
            "DXBC_JOINED_RUNTIME_EXECUTOR_NOT_IMPLEMENTED"
        ]
        self.assertEqual(len(evidence_only), 142)
        self.assertTrue(all(
            family["formulaProvenance"] is None
            and family["cookedShaderEvidenceProvenance"] ==
            "BULK_FAMILY_COOKED_PIXEL_SHADER_RECEIPT"
            for family in evidence_only
        ))


class CheckedInInventoryTest(unittest.TestCase):
    def setUp(self) -> None:
        if not DEFAULT_OUTPUT.is_file():
            self.skipTest("inventory has not been generated")
        self.inventory = json.loads(
            DEFAULT_OUTPUT.read_text(encoding="utf-8"))

    def test_denominators_reconcile(self) -> None:
        summary = self.inventory["summary"]
        self.assertEqual(
            summary["sourceDerivedElementCount"]
            + summary["projectAuthoredApproximationElementCount"],
            summary["authoredElementCount"])
        self.assertEqual(
            summary["typedExecutorFamilyCount"]
            + summary["missingFamilyCount"],
            summary["familyCount"])
        self.assertEqual(
            summary["typedExecutorOccurrenceCount"]
            + summary["missingOccurrenceCount"],
            summary["familyOccurrenceCount"])

    def test_every_family_carries_a_status_and_an_identity(self) -> None:
        for family in self.inventory["families"]:
            self.assertTrue(family["parentMaterialPath"])
            self.assertTrue(family["restorationStatus"])
            self.assertEqual(
                bool(family["typedExecutors"]),
                family["restorationStatus"] == "TYPED_EXECUTOR_IMPLEMENTED")

    def test_missing_coverage_ranks_are_monotonic(self) -> None:
        running = 0
        for row in self.inventory["missingFamilyCoverageByRank"]:
            running += row["occurrenceCount"]
            self.assertEqual(running, row["cumulativeMissingOccurrences"])
        self.assertEqual(
            running, self.inventory["summary"]["missingOccurrenceCount"])


class InventoryWriterTest(unittest.TestCase):
    def test_writer_uses_utf8_lf_bytes(self) -> None:
        with tempfile.TemporaryDirectory(prefix="missing-family-writer-") as root:
            output = Path(root) / "inventory.json"
            write_inventory(output, {"label": "한글", "rows": [1, 2]})
            payload = output.read_bytes()
            self.assertNotIn(b"\r\n", payload)
            self.assertTrue(payload.endswith(b"\n"))
            self.assertEqual(
                json.loads(payload.decode("utf-8")),
                {"label": "한글", "rows": [1, 2]},
            )


if __name__ == "__main__":
    unittest.main()
