#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name(
    "build_dimensionmaster_source_material_evidence.py"
)
SPEC = importlib.util.spec_from_file_location(
    "build_dimensionmaster_source_material_evidence", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class BuildDimensionMasterSourceMaterialEvidenceTests(unittest.TestCase):
    def test_direct_material_dump_is_canonical_evidence(self) -> None:
        source_path = "bfx_m.bfx.direct_glow"
        catalog = {
            "schema": "lostark.dimensionmaster-material-candidate-catalog",
            "characterClass": "DIMENSIONMASTER",
            "skills": [{"skillId": 10}],
            "unresolvedMaterialBindings": [{
                "sourceMaterialPath": source_path,
            }],
        }
        extractor_receipt = {"candidates": [{
            "material_path": source_path,
            "resolutionStatus": "RESOLVED_UMODEL_DUMP",
            "source_file": "source.upk",
            "parent": source_path,
            "materialEvidenceStatus": "SOURCE_MATERIAL_DUMP",
            "materialEvidencePropsCandidateCount": 1,
            "materialEvidenceFile": "dump/direct.umodel-dump.txt",
            "materialEvidenceSha256": "dump-sha",
            "sourceEvidenceFile": "dump/direct.umodel-dump.txt",
            "sourceEvidenceSha256": "dump-sha",
            "materialEvidence": {
                "renderState": {"blendMode": "BLEND_Additive"},
                "collectedScalarParameters": [{
                    "name": "centerglow_str", "group": "None", "value": 2.0,
                }],
            },
            "scalars": [{"name": "centerglow_str", "value": 2.0}],
        }]}

        evidence, receipt = MODULE.build_evidence(
            catalog, {"materials": {}}, extractor_receipt
        )

        row = evidence["materials"][source_path][0]
        self.assertNotIn("fallbackBlockedReason", row)
        self.assertEqual("dump-sha", row["materialEvidenceSha256"])
        parent = evidence["parentMaterialEvidence"][
            row["materialEvidenceRef"]
        ]
        self.assertEqual("dump-sha", parent["propsFileSha256"])
        self.assertEqual(
            2.0,
            parent["materialEvidence"]["collectedScalarParameters"][0][
                "value"
            ],
        )
        self.assertEqual(0, receipt["summary"]["failClosedCandidateCount"])

    def test_missing_parent_props_becomes_fail_closed_material(self) -> None:
        catalog = {
            "schema": "lostark.dimensionmaster-material-candidate-catalog",
            "characterClass": "DIMENSIONMASTER",
            "skills": [{"skillId": 10}],
            "unresolvedMaterialBindings": [{
                "sourceMaterialPath": "fx_pkg.fx_mi.card",
            }],
        }
        extractor_receipt = {"candidates": [{
            "material_path": "fx_pkg.fx_mi.card",
            "resolutionStatus": "RESOLVED_UMODEL_EXPORT",
            "source_file": "fx_pkg.upk",
            "materialEvidenceStatus": "MISSING_OR_AMBIGUOUS_SOURCE_MATERIAL_PROPS",
            "materialEvidencePropsCandidateCount": 0,
        }]}

        evidence, receipt = MODULE.build_evidence(
            catalog, {"materials": {}}, extractor_receipt
        )

        row = evidence["materials"]["fx_pkg.fx_mi.card"][0]
        self.assertEqual(
            "MISSING_OR_AMBIGUOUS_PARENT_MATERIAL_PROPS",
            row["fallbackBlockedReason"],
        )
        self.assertEqual(1, receipt["summary"]["failClosedCandidateCount"])

    def test_unique_parent_props_stay_executable_and_missing_export_blocks(self) -> None:
        catalog = {
            "schema": "lostark.dimensionmaster-material-candidate-catalog",
            "characterClass": "DIMENSIONMASTER",
            "skills": [{"skillId": 10}],
            "unresolvedMaterialBindings": [
                {"sourceMaterialPath": "fx_pkg.fx_mi.safe"},
                {"sourceMaterialPath": "fx_pkg.fx_mi.absent"},
            ],
        }
        extractor_receipt = {"candidates": [{
            "material_path": "fx_pkg.fx_mi.safe",
            "resolutionStatus": "RESOLVED_UMODEL_EXPORT",
            "source_file": "fx_pkg.upk",
            "parent": "fx_m.safe",
            "materialEvidenceStatus": "SOURCE_MATERIAL_PROPS",
            "materialEvidencePropsCandidateCount": 1,
            "materialEvidencePropsFile": "FX_M/Material3/safe.props.txt",
            "materialEvidencePropsSha256": "parent-sha",
            "propsFile": "FX_MI/MaterialInstanceConstant/safe.props.txt",
            "propsFileSha256": "instance-sha",
            "materialEvidence": {"renderState": {
                "blendMode": "BLEND_Additive", "twoSided": False,
            }},
        }]}

        evidence, receipt = MODULE.build_evidence(
            catalog, {"materials": {}}, extractor_receipt
        )

        safe = evidence["materials"]["fx_pkg.fx_mi.safe"][0]
        absent = evidence["materials"]["fx_pkg.fx_mi.absent"][0]
        self.assertNotIn("fallbackBlockedReason", safe)
        self.assertEqual("fx_m.safe", safe["parent"])
        self.assertEqual("instance-sha", safe["propsFileSha256"])
        self.assertNotIn("materialEvidence", safe)
        ref = safe["materialEvidenceRef"]
        self.assertEqual(
            "parent-sha",
            evidence["parentMaterialEvidence"][ref]["propsFileSha256"],
        )
        self.assertEqual(
            "UNRESOLVED_OR_AMBIGUOUS_MATERIAL_EXPORT",
            absent["fallbackBlockedReason"],
        )
        self.assertEqual(1, receipt["summary"]["failClosedCandidateCount"])
        self.assertEqual(1, receipt["summary"]["missingExtractorCandidateCount"])

    def test_hashless_parent_props_are_not_canonical_evidence(self) -> None:
        catalog = {
            "schema": "lostark.dimensionmaster-material-candidate-catalog",
            "characterClass": "DIMENSIONMASTER",
            "skills": [{"skillId": 10}],
            "unresolvedMaterialBindings": [{
                "sourceMaterialPath": "fx_pkg.fx_mi.safe",
            }],
        }
        extractor_receipt = {"candidates": [{
            "material_path": "fx_pkg.fx_mi.safe",
            "resolutionStatus": "RESOLVED_UMODEL_EXPORT",
            "source_file": "fx_pkg.upk",
            "parent": "fx_m.safe",
            "materialEvidenceStatus": "SOURCE_MATERIAL_PROPS",
            "materialEvidencePropsCandidateCount": 1,
            "materialEvidence": {"renderState": {}},
        }]}
        evidence, _ = MODULE.build_evidence(
            catalog, {"materials": {}}, extractor_receipt
        )
        row = evidence["materials"]["fx_pkg.fx_mi.safe"][0]
        self.assertEqual(
            "MISSING_PARENT_MATERIAL_PROPS_PROVENANCE",
            row["fallbackBlockedReason"],
        )


if __name__ == "__main__":
    unittest.main()
