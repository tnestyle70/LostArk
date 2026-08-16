#!/usr/bin/env python3
"""Focused tests for the four-class authored source Material compiler."""

from __future__ import annotations

import importlib.util
import copy
import sys
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name("build_four_class_source_material_contract.py")
SPEC = importlib.util.spec_from_file_location("four_class_material_contract", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class FourClassSourceMaterialContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.contract, cls.receipt = MODULE.build_projection()

    def test_exact_identity_denominator_and_admission_partition(self) -> None:
        counts = self.receipt["counts"]
        self.assertEqual(counts["seedMaterialIdentityCount"], 50)
        self.assertEqual(counts["compiledMaterialIdentityCount"], 769)
        self.assertEqual(
            counts["blockedMissingPhysicalPackageIdentityCount"], 9
        )
        self.assertEqual(
            counts["blockedMissingPhysicalPackageOccurrenceCount"], 12
        )
        self.assertEqual(counts["totalMaterialIdentityCount"], 828)
        self.assertEqual(counts["compiledOccurrenceCount"], 3907)
        self.assertEqual(counts["sourceEvidenceResolvedIdentityCount"], 765)
        self.assertEqual(counts["admittedIdentityCount"], 587)
        self.assertEqual(counts["fallbackBlockedIdentityCount"], 178)
        self.assertEqual(counts["sourceEvidenceBlockedIdentityCount"], 4)
        self.assertEqual(counts["sourceArtifactCount"], 51)
        self.assertEqual(counts["exactPhysicalWrapperNormalizedRowCount"], 90)
        self.assertEqual(
            counts["exactPhysicalWrapperMaterialIdentityCount"], 50
        )
        self.assertEqual(
            counts["exactPhysicalWrapperCompiledIdentityCount"], 48
        )
        self.assertEqual(counts["exactPhysicalWrapperSeedIdentityCount"], 2)
        self.assertEqual(
            counts["exactPhysicalWrapperAdmittedIdentityCount"], 36
        )
        self.assertEqual(
            counts["exactPhysicalWrapperAdmittedOccurrenceCount"], 223
        )
        self.assertEqual(
            counts["exactPhysicalWrapperFallbackBlockedIdentityCount"], 12
        )
        self.assertEqual(
            counts["exactPhysicalWrapperFallbackBlockedOccurrenceCount"], 89
        )
        self.assertEqual(
            counts["admittedIdentityCount"]
            + counts["fallbackBlockedIdentityCount"],
            counts["sourceEvidenceResolvedIdentityCount"],
        )

    def test_compiled_rows_are_unique_exact_material_package_tuples(self) -> None:
        rows = self.contract["materialIdentities"]
        keys = [MODULE.material_key(row) for row in rows]
        self.assertEqual(len(rows), 769)
        self.assertEqual(len(keys), len(set(keys)))
        self.assertTrue(all(path and package for path, package in keys))
        self.assertTrue(
            all(
                row["productAdmissionStatus"]
                in {
                    "ADMITTED_RECONSTRUCTED_PROFILE",
                    "BLOCKED_FALLBACK_PROFILE",
                    "BLOCKED_SOURCE_EVIDENCE",
                }
                for row in rows
            )
        )
        self.assertTrue(
            all(
                not str(row["sourceMaterialPath"]).casefold().startswith(
                    "enginematerials."
                )
                or row["productAdmissionStatus"] == "BLOCKED_SOURCE_EVIDENCE"
                for row in rows
            )
        )

    def test_physical_material_maps_are_hash_pinned_inputs(self) -> None:
        sources = self.receipt["sources"]
        maps = [row for row in sources if row["path"].endswith(".materials.json")]
        self.assertEqual(len(maps), 22)
        self.assertTrue(all(len(row["sha256"]) == 64 for row in maps))
        self.assertTrue(
            all("Resource_LostArk" in row["path"] for row in maps)
        )

    def test_seed_contracts_and_corpus_contract_are_disjoint(self) -> None:
        seed_keys = MODULE.load_seed_keys()
        corpus_keys = {
            MODULE.material_key(row)
            for row in self.contract["materialIdentities"]
        }
        self.assertEqual(len(seed_keys), 50)
        self.assertFalse(seed_keys & corpus_keys)

    def test_physical_material_join_never_uses_object_name_or_suffix(self) -> None:
        graph = {
            "materialParameterBindings": [
                {
                    "resolutionStatus": "RESOLVED_EXACT_SOURCE_PACKAGE",
                    "candidateCount": 1,
                    "sourcePhysicalPackage": "ExactPackage.upk",
                    "materialPath": "ExactPackage.Group.SharedName",
                    "sourceMaterialPath": "source.shared-name",
                }
            ]
        }
        material_map = {
            "source": {"file": "ExactPackage.upk"},
            "materials": [
                {
                    "material_path": "WrongPackage.Group.SharedName",
                    "parent": "Parent.Material",
                }
            ],
        }
        merged = MODULE.merge_exact_physical_material_evidence(
            graph, {"exactpackage.upk": copy.deepcopy(material_map)}
        )
        row = merged["materialParameterBindings"][0]
        self.assertEqual(row["resolutionStatus"], "MISSING_PHYSICAL_MATERIAL_ROW")
        self.assertEqual(row["candidateCount"], 0)
        self.assertNotIn("physicalMaterialEvidence", row)

    def test_duplicate_wrapper_is_normalized_only_by_one_exact_full_path(self) -> None:
        for wrapper_count in (2, 3):
            graph = {
                "materialParameterBindings": [
                    {
                        "resolutionStatus": "RESOLVED_EXACT_SOURCE_PACKAGE",
                        "candidateCount": wrapper_count,
                        "sourcePhysicalPackage": "ExactPackage.upk",
                        "materialPath": "ExactPackage.Group.Material",
                        "sourceMaterialPath": "source.exact",
                    }
                ]
            }
            material_map = {
                "source": {"file": "ExactPackage.upk"},
                "materials": [
                    {
                        "material_path": "ExactPackage.Group.Material",
                        "parent": "Parent.Material",
                    }
                ],
            }
            merged = MODULE.merge_exact_physical_material_evidence(
                graph, {"exactpackage.upk": copy.deepcopy(material_map)}
            )
            row = merged["materialParameterBindings"][0]
            self.assertEqual(row["candidateCount"], 1)
            self.assertEqual(
                row["physicalMaterialEvidence"][
                    "sourceWrapperCandidateCount"
                ],
                wrapper_count,
            )
            self.assertEqual(
                row["physicalMaterialEvidence"]["materialPath"],
                "ExactPackage.Group.Material",
            )

    def test_duplicate_wrapper_fails_for_zero_or_two_exact_full_path_rows(self) -> None:
        graph = {
            "materialParameterBindings": [
                {
                    "resolutionStatus": "RESOLVED_EXACT_SOURCE_PACKAGE",
                    "candidateCount": 2,
                    "sourcePhysicalPackage": "ExactPackage.upk",
                    "materialPath": "ExactPackage.Group.Material",
                    "sourceMaterialPath": "source.exact",
                }
            ]
        }
        for exact_rows, expected_status in (
            ([], "MISSING_PHYSICAL_MATERIAL_ROW"),
            (
                [
                    {"material_path": "ExactPackage.Group.Material"},
                    {"material_path": "ExactPackage.Group.Material"},
                ],
                "AMBIGUOUS_PHYSICAL_MATERIAL_ROW",
            ),
        ):
            material_map = {
                "source": {"file": "ExactPackage.upk"},
                "materials": exact_rows,
            }
            merged = MODULE.merge_exact_physical_material_evidence(
                graph, {"exactpackage.upk": copy.deepcopy(material_map)}
            )
            row = merged["materialParameterBindings"][0]
            self.assertEqual(row["resolutionStatus"], expected_status)
            self.assertEqual(row["candidateCount"], len(exact_rows))
            self.assertNotIn("physicalMaterialEvidence", row)

        zero_wrapper = copy.deepcopy(graph)
        zero_wrapper["materialParameterBindings"][0]["candidateCount"] = 0
        untouched = MODULE.merge_exact_physical_material_evidence(
            zero_wrapper,
            {
                "exactpackage.upk": {
                    "source": {"file": "ExactPackage.upk"},
                    "materials": [
                        {"material_path": "ExactPackage.Group.Material"}
                    ],
                }
            },
        )["materialParameterBindings"][0]
        self.assertEqual(untouched["candidateCount"], 0)
        self.assertNotIn("physicalMaterialEvidence", untouched)


if __name__ == "__main__":
    unittest.main()
