#!/usr/bin/env python3
"""Focused tests for the portable R8 offline raw-resource checkpoint."""

from __future__ import annotations

import copy
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("prepare_r8_raw_resource_inventory.py")
SPEC = importlib.util.spec_from_file_location("prepare_r8_raw_resource_inventory", MODULE_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"cannot import {MODULE_PATH}")
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)
REPO_ROOT = Path(__file__).resolve().parents[2]
CHECKPOINT = (
    REPO_ROOT
    / "Data/Effects/Imported/RawResourceInventory/"
    "R8.raw-resource-denominator.checkpoint.json"
)


class RawResourceCheckpointTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = MODULE.load_json(CHECKPOINT)

    def test_checked_checkpoint_has_exact_denominators_and_blockers(self) -> None:
        summary = MODULE.validate_checkpoint(self.document)
        self.assertEqual(summary["fourClassOccurrences"], 5232)
        self.assertEqual(summary["valtanOccurrences"], 21931)
        self.assertEqual(summary["fourClassAssetRequests"], 835)
        self.assertEqual(summary["valtanAssetRequests"], 377)
        self.assertEqual(self.document["fourClass"]["summary"]["sourceSystemCount"], 442)
        source_system_counts = {}
        for row in self.document["fourClass"]["sourceSystems"]:
            source_system_counts[row["characterClass"]] = (
                source_system_counts.get(row["characterClass"], 0) + 1
            )
        self.assertEqual(
            source_system_counts,
            {
                "ARTIST": 70,
                "DIMENSIONMASTER": 95,
                "LANCE_MASTER": 179,
                "WARLORD": 98,
            },
        )
        self.assertEqual(summary["checkpointStatus"], "FROZEN_WITH_BLOCKERS")
        self.assertEqual(self.document["blockerCount"], 18)
        self.assertEqual(len(self.document["reports"]["pinFailures"]), 2)
        self.assertEqual(
            {
                (row["characterClass"], row["skillId"], row["artifactKind"])
                for row in self.document["reports"]["pinFailures"]
            },
            {
                ("ARTIST", 31930, "importedDocument"),
                ("WARLORD", 17110, "normalizedGraph"),
            },
        )
        self.assertEqual(
            len(self.document["reports"]["preexistingRuntimeMissingRequests"]),
            191,
        )
        self.assertEqual(
            self.document["fourClass"]["summary"]["occurrenceSourceSystemCount"],
            424,
        )
        self.assertEqual(
            self.document["fourClass"]["summary"]["graphOnlyInactiveSourceSystemCount"],
            18,
        )
        self.assertEqual(
            self.document["fourClass"]["classAssetSummaries"][-1],
            {
                "characterClass": "WARLORD",
                "assetRequestCount": 328,
                "meshRequestCount": 47,
                "textureRequestCount": 281,
            },
        )
        self.assertEqual(
            self.document["reports"]["warlordCatalogGraphDisagreements"],
            [
                {
                    "code": "WARLORD_CATALOG_DEPENDENCY_ABSENT_FROM_NORMALIZED_GRAPH_BINDINGS",
                    "role": "texture",
                    "sourceAssetPath": "fx_tex_02.fx_d_environ_018",
                    "logicalPackage": "fx_tex_02",
                    "physicalPackageFileName": "YGI3SORGM3I1FGHA5BMJ8Y5CZ.upk",
                }
            ],
        )
        request = next(
            row
            for row in self.document["fourClass"]["assetRequests"]
            if row["sourceAssetPath"] == "fx_tex_02.fx_d_environ_018"
        )
        warlord = next(
            row for row in request["consumers"] if row["consumer"] == "WARLORD"
        )
        self.assertEqual(warlord["skillIds"], [17820])
        self.assertEqual(warlord["actionIds"], [17820])
        self.assertEqual(
            warlord["sourceSystemIds"], ["FX_PC_WGL_01.par_o_wgl_protect_02"]
        )
        self.assertEqual(
            warlord["ownershipStatuses"],
            ["ACTION_BOUND_CATALOG_OWNERSHIP", "RESOLVED_SOURCE_PACKAGE"],
        )

    def test_runtime_admission_is_strictly_false(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["admission"]["Render"] = True
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "admission"):
            MODULE.validate_checkpoint(mutated)

    def test_gpu_schema_policy_mutation_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["gpuSchemaBoundary"]["sampler"] = "FINAL_SAMPLER"
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "GPU schema boundary"):
            MODULE.validate_checkpoint(mutated)

    def test_blocker_report_and_status_mutation_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["reports"]["pinFailures"] = []
        mutated["blockerCount"] = 0
        mutated["checkpointStatus"] = "FROZEN_COMPLETE"
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "blocker|reports"):
            MODULE.validate_checkpoint(mutated)

    def test_coordinated_false_promotion_is_rejected_by_frozen_evidence(self) -> None:
        mutated = copy.deepcopy(self.document)
        for row in mutated["fourClass"]["pinnedArtifacts"]:
            row["pinStatus"] = "MATCH"
            row["pathKind"] = "REPO_RELATIVE"
        for request in mutated["fourClass"]["assetRequests"]:
            for consumer in request["consumers"]:
                if consumer["consumer"] == "WARLORD":
                    consumer["ownershipStatuses"].append("SOURCE_GRAPH_PIN_MATCH")
                    consumer["ownershipStatuses"] = sorted(
                        set(consumer["ownershipStatuses"])
                    )
        current_archive = mutated["valtan"]["currentInstalledArchiveSnapshot"]
        current_archive["matchesPinnedCorpusArchive"] = True
        current_archive["byteSize"] = MODULE.VALTAN_ARCHIVE_PIN["byteSize"]
        current_archive["sha256"] = MODULE.VALTAN_ARCHIVE_PIN["sha256"]
        for key in (
            "pinFailures",
            "weakAbsoluteSourceHints",
            "candidateIdCollisions",
            "physicalPackageIssues",
            "warlordCatalogGraphDisagreements",
            "valtanProvenanceIssues",
        ):
            mutated["reports"][key] = []
        summary = mutated["fourClass"]["summary"]
        summary["pinFailureCount"] = 0
        summary["absoluteSourceHintCount"] = 0
        summary["warlordCatalogGraphDisagreementCount"] = 0
        mutated["blockerEvidence"] = []
        mutated["blockerEvidenceSha256"] = MODULE.blocker_evidence_sha256([])
        mutated["blockerCount"] = 0
        mutated["checkpointStatus"] = "FROZEN_COMPLETE"
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "frozen baseline"):
            MODULE.validate_checkpoint(mutated)

    def test_self_digest_detects_mutation(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["fourClass"]["summary"]["sourceOccurrenceCount"] -= 1
        with self.assertRaisesRegex(MODULE.InventoryError, "self digest"):
            MODULE.validate_checkpoint(mutated)

    def test_nonportable_paths_are_rejected(self) -> None:
        for value in (
            "C:/absolute/file.dds",
            "C:\\absolute\\file.dds",
            "C:drive-relative/file.dds",
            "/rooted/file.dds",
            "Effect/../outside.dds",
        ):
            with self.subTest(value=value):
                with self.assertRaises(MODULE.InventoryError):
                    MODULE.validate_portable_strings({"value": value})

    def test_strict_json_rejects_duplicate_and_nonfinite_values(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            duplicate = Path(directory) / "duplicate.json"
            duplicate.write_text('{"value":1,"value":2}\n', encoding="utf-8")
            with self.assertRaisesRegex(MODULE.InventoryError, "duplicate JSON key"):
                MODULE.load_json(duplicate)
            nonfinite = Path(directory) / "nonfinite.json"
            nonfinite.write_text('{"value":NaN}\n', encoding="utf-8")
            with self.assertRaisesRegex(MODULE.InventoryError, "non-finite"):
                MODULE.load_json(nonfinite)

    def test_candidate_id_preserves_full_object_path(self) -> None:
        first = MODULE.candidate_id(
            "FourClass", "texture", "fx_tex_02.sk_wgl_gdd_01_d"
        )
        second = MODULE.candidate_id(
            "FourClass", "texture", "sk_wgl_gdd_01.tex.sk_wgl_gdd_01_d"
        )
        self.assertNotEqual(first.casefold(), second.casefold())
        self.assertEqual(
            first,
            "Effect/RawCandidates/FourClass/Textures/"
            "fx_tex_02/sk_wgl_gdd_01_d.dds",
        )
        self.assertEqual(
            second,
            "Effect/RawCandidates/FourClass/Textures/"
            "sk_wgl_gdd_01/tex/sk_wgl_gdd_01_d.dds",
        )

    def test_obfuscated_package_mapping_matches_observed_install(self) -> None:
        self.assertEqual(
            MODULE.obfuscate_package_name("fx_sm_03") + ".upk",
            "XFH2RGA2R0LF04YE903X0SMQ.upk",
        )
        self.assertEqual(
            MODULE.obfuscate_package_name("mn_pmsec_00") + ".upk",
            "9G1MU9FPB1GZZEFTQWU463S.upk",
        )

    def test_manifest_serialization_is_deterministic(self) -> None:
        projection = copy.deepcopy(self.document)
        projection.pop("selfDigest")
        first = MODULE.canonical_bytes(projection)
        second = MODULE.canonical_bytes(json.loads(first))
        self.assertEqual(first, second)
        self.assertEqual(
            MODULE.compute_self_digest(self.document), self.document["selfDigest"]
        )


if __name__ == "__main__":
    unittest.main()
