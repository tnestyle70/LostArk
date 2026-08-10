#!/usr/bin/env python3
"""Mutation tests for the Artist F Material source-value acquisition receipt."""

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path
from unittest import mock


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
        cls.render = acquisition.load_json(cls.render_path)
        cls.shader = acquisition.load_json(cls.shader_path)
        cls.committed = acquisition.load_json(cls.output_path)
        cls.rebuilt = acquisition.build_receipt(
            cls.contract,
            cls.render,
            cls.shader,
            cls.contract_path,
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
            cls.render,
            cls.shader,
            cls.contract_path,
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

    def test_coordinated_static_guid_value_and_override_reseals_are_rejected(self) -> None:
        for mutation_kind in ("guid", "value", "override"):
            with self.subTest(mutation_kind=mutation_kind):
                mutated = copy.deepcopy(self.committed)
                row = next(
                    row
                    for row in mutated["matrices"]["staticPermutationRows"]
                    if row["sourceValueAcquired"]
                )
                entry = row["micNativeSelection"]["entry"]
                if mutation_kind == "guid":
                    replacement = "00" * 16
                    row["parentExpression"]["expressionGuidHex"] = replacement
                    row["parentExpression"]["expressionGuidProperty"]["value"][
                        "hex"
                    ] = replacement
                    entry["expressionGuidHex"] = replacement
                    entry["rawExpressionGuidHex"] = replacement
                elif mutation_kind == "value":
                    entry["value"] = not entry["value"]
                    entry["rawValueUint32Hex"] = (
                        "01000000" if entry["value"] else "00000000"
                    )
                else:
                    entry["bOverride"] = False
                    entry["rawOverrideUint32Hex"] = "00000000"
                    row["sourceValueAcquired"] = False
                    row["sourceValueDecision"] = (
                        "SOURCE_EXACT_NONOVERRIDE_ENTRY_OBSERVED_"
                        "INHERITANCE_SEMANTICS_UNVERIFIED"
                    )
                    row["executionDecision"] = "BLOCKED"
                mutated["summary"]["staticRowSetSha256"] = (
                    acquisition.canonical_sha256(
                        mutated["matrices"]["staticPermutationRows"]
                    )
                )
                self.reseal(mutated)
                with self.assertRaisesRegex(
                    ValueError, "static approved semantic projection changed"
                ):
                    self.validate(mutated)

    def test_pure_static_semantics_rederive_decoded_raw_values(self) -> None:
        for mutation_kind in (
            "entry-value",
            "entry-override",
            "entry-guid",
            "parent-default",
            "parent-guid",
        ):
            with self.subTest(mutation_kind=mutation_kind):
                mutated = copy.deepcopy(self.committed)
                row = next(
                    row
                    for row in mutated["matrices"]["staticPermutationRows"]
                    if row["matrixRowId"]
                    == "material-feasibility-static-104ba0eb7fef8369"
                )
                entry = row["micNativeSelection"]["entry"]
                if mutation_kind == "entry-value":
                    entry["value"] = False
                    expected_error = "MIC value decoded/raw semantics"
                elif mutation_kind == "entry-override":
                    entry["bOverride"] = False
                    expected_error = "MIC bOverride decoded/raw semantics"
                elif mutation_kind == "entry-guid":
                    entry["expressionGuidHex"] = "00" * 16
                    expected_error = "MIC ExpressionGUID decoded/raw semantics"
                elif mutation_kind == "parent-default":
                    row["parentExpression"]["defaultValueProperty"][
                        "value"
                    ] = False
                    expected_error = "parent default decoded/raw semantics"
                else:
                    row["parentExpression"]["expressionGuidHex"] = "00" * 16
                    expected_error = "parent ExpressionGUID decoded/raw semantics"
                mutated_static_sha = acquisition.canonical_sha256(
                    mutated["matrices"]["staticPermutationRows"]
                )
                mutated["summary"]["staticRowSetSha256"] = mutated_static_sha
                self.reseal(mutated)
                with mock.patch.object(
                    acquisition,
                    "APPROVED_STATIC_ROW_SET_SHA256",
                    mutated_static_sha,
                ):
                    with self.assertRaisesRegex(ValueError, expected_error):
                        acquisition.validate_receipt_semantics(
                            mutated, self.contract
                        )

    def test_pure_static_semantics_cover_all_three_outcome_families(self) -> None:
        cases = (
            (
                "override-value",
                "material-feasibility-static-104ba0eb7fef8369",
                lambda row: row["micNativeSelection"]["entry"].update(
                    value=False
                ),
                "MIC value decoded/raw semantics",
            ),
            (
                "nonoverride-value",
                "material-feasibility-static-25e206c498b44f77",
                lambda row: row["micNativeSelection"]["entry"].update(
                    value=False
                ),
                "MIC value decoded/raw semantics",
            ),
            (
                "nonoverride-override",
                "material-feasibility-static-25e206c498b44f77",
                lambda row: row["micNativeSelection"]["entry"].update(
                    bOverride=True
                ),
                "MIC bOverride decoded/raw semantics",
            ),
            (
                "nonoverride-guid",
                "material-feasibility-static-25e206c498b44f77",
                lambda row: row["micNativeSelection"]["entry"].update(
                    expressionGuidHex="00" * 16
                ),
                "MIC ExpressionGUID decoded/raw semantics",
            ),
            (
                "unmatched-parent-default",
                "material-feasibility-static-1b69b57952caaa03",
                lambda row: row["parentExpression"][
                    "defaultValueProperty"
                ].update(value=False),
                "parent default decoded/raw semantics",
            ),
            (
                "unmatched-parent-guid",
                "material-feasibility-static-1b69b57952caaa03",
                lambda row: row["parentExpression"].update(
                    expressionGuidHex="00" * 16
                ),
                "parent ExpressionGUID decoded/raw semantics",
            ),
            (
                "unmatched-forged-exact-count",
                "material-feasibility-static-1b69b57952caaa03",
                lambda row: row["micNativeSelection"].update(
                    exactNameAndGuidMatchCount=1
                ),
                "MIC entry owner/offset",
            ),
        )
        for label, row_id, mutate, expected_error in cases:
            with self.subTest(label=label):
                mutated = copy.deepcopy(self.committed)
                row = next(
                    row
                    for row in mutated["matrices"]["staticPermutationRows"]
                    if row["matrixRowId"] == row_id
                )
                mutate(row)
                mutated_static_sha = acquisition.canonical_sha256(
                    mutated["matrices"]["staticPermutationRows"]
                )
                mutated["summary"]["staticRowSetSha256"] = mutated_static_sha
                self.reseal(mutated)
                with mock.patch.object(
                    acquisition,
                    "APPROVED_STATIC_ROW_SET_SHA256",
                    mutated_static_sha,
                ):
                    with self.assertRaisesRegex(ValueError, expected_error):
                        acquisition.validate_receipt_semantics(
                            mutated, self.contract
                        )

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
        with self.assertRaisesRegex(
            ValueError, "strict sampler approved semantic projection changed"
        ):
            self.validate(mutated)

    def test_semantic_validator_rejects_coordinated_sampler_promotion(self) -> None:
        mutated = copy.deepcopy(self.committed)
        row = next(
            row
            for row in mutated["matrices"]["strictSamplerRows"]
            if row["matrixRowId"]
            == "material-feasibility-sampler-316a56b9a4bd256c"
        )
        row["fullDescriptorSourceExact"] = True
        row["sourceValueAcquired"] = True
        row["sourceValueDecision"] = "SOURCE_EXACT_FULL_DESCRIPTOR"
        row["strictReauditDecision"] = "PASS"
        row["executionReady"] = True
        mutated["summary"]["strictSamplerSourceValueAcquiredCount"] = 1
        mutated["summary"]["strictExecutionReadyCount"] = 1
        mutated_row_sha = acquisition.canonical_sha256(
            mutated["matrices"]["strictSamplerRows"]
        )
        mutated["summary"]["strictSamplerRowSetSha256"] = mutated_row_sha
        self.reseal(mutated)
        with mock.patch.object(
            acquisition,
            "APPROVED_STRICT_SAMPLER_ROW_SET_SHA256",
            mutated_row_sha,
        ):
            with self.assertRaisesRegex(ValueError, "sampler admission changed"):
                acquisition.validate_receipt_semantics(mutated, self.contract)

    def test_legacy_exact_sampler_decoded_fields_are_raw_contract_bound(self) -> None:
        for mutation_kind in ("decoded-value", "property-type"):
            with self.subTest(mutation_kind=mutation_kind):
                mutated = copy.deepcopy(self.committed)
                row = next(
                    row
                    for row in mutated["matrices"]["strictSamplerRows"]
                    if row["baselineKind"]
                    == "PREVIOUSLY_ADMITTED_EXACT_REAUDIT"
                    and any(
                        field.get("status") == "SERIALIZED_EXPLICIT"
                        and field_name != "srgb"
                        for field_name, field in row["textureExportEvidence"][
                            "fields"
                        ].items()
                    )
                )
                field_name, field = next(
                    (field_name, field)
                    for field_name, field in row["textureExportEvidence"][
                        "fields"
                    ].items()
                    if field.get("status") == "SERIALIZED_EXPLICIT"
                    and field_name != "srgb"
                )
                prop = field["property"]
                if mutation_kind == "decoded-value":
                    prop["value"] = "FORGED_DECODED_VALUE"
                    expected_error = "decoded/raw projection changed"
                else:
                    prop["propertyType"] = "boolproperty"
                    expected_error = "byte-property decoded/raw semantics changed"
                mutated_row_sha = acquisition.canonical_sha256(
                    mutated["matrices"]["strictSamplerRows"]
                )
                mutated["summary"]["strictSamplerRowSetSha256"] = mutated_row_sha
                self.reseal(mutated)
                with mock.patch.object(
                    acquisition,
                    "APPROVED_STRICT_SAMPLER_ROW_SET_SHA256",
                    mutated_row_sha,
                ):
                    with self.assertRaisesRegex(ValueError, expected_error):
                        acquisition.validate_receipt_semantics(
                            mutated, self.contract
                        )

    def test_execution_and_product_remain_closed(self) -> None:
        admission = self.committed["admission"]
        self.assertTrue(admission["upstreamMaterialEvidenceIntegrity"])
        self.assertFalse(admission["executionReady"])
        self.assertFalse(admission["product"])
        self.assertFalse(admission["r2Entry"])
        self.assertEqual(0, self.committed["summary"]["strictExecutionReadyCount"])
        self.assertEqual(0, self.committed["summary"]["productCount"])

    def test_all_top_level_acquisition_evidence_is_independently_pinned(self) -> None:
        cases = (
            "global-exhaustion",
            "vss-provider",
            "controlled-capture",
            "render-owner",
            "source-path",
            "source-extra-key",
            "provenance-cluster",
            "corrective-complete",
            "missing-artifact",
            "empty-blockers",
            "root-extra-key",
            "summary-product-extra",
        )
        for label in cases:
            with self.subTest(label=label):
                mutated = copy.deepcopy(self.committed)
                if label == "global-exhaustion":
                    mutated["externalArtifactSearch"]["scopeBoundary"][
                        "globalExhaustionClaim"
                    ] = True
                elif label == "vss-provider":
                    vss = mutated["externalArtifactSearch"]["scopeBoundary"][
                        "volumeShadowCopy"
                    ]
                    vss.update(
                        status="EXHAUSTED_NO_PROVIDER", admissionInput=True
                    )
                elif label == "controlled-capture":
                    capture = mutated["externalArtifactSearch"][
                        "controlledRuntimeCapture"
                    ]
                    capture.update(
                        safeProviderAvailable=True,
                        sourceRevisionRuntimeBundleAvailable=True,
                        sourceRevisionDebugOrCaptureApiAvailable=True,
                        currentInstalledProcessIsSourceRevisionAuthenticated=True,
                        decision="SOURCE_EXACT_CAPTURE_AVAILABLE",
                    )
                elif label == "render-owner":
                    mutated["matrices"]["renderStateRows"][0]["owner"] = (
                        "FORGED_PRODUCT_OWNER"
                    )
                    mutated["summary"]["renderRowSetSha256"] = (
                        acquisition.canonical_sha256(
                            mutated["matrices"]["renderStateRows"]
                        )
                    )
                elif label == "source-path":
                    mutated["source"][0]["path"] = "forged/provider.json"
                    mutated["source"][0]["canonicalTextSha256"] = "0" * 64
                elif label == "source-extra-key":
                    mutated["source"][0]["sourceExact"] = True
                elif label == "provenance-cluster":
                    mutated["provenanceClusters"]["staticRecipeCount"] = 0
                elif label == "corrective-complete":
                    mutated["coordinatedCorrectiveRequirements"][0][
                        "decision"
                    ] = "COMPLETE"
                elif label == "missing-artifact":
                    mutated["minimumMissingExternalArtifacts"]["sampler"] = (
                        "NONE_REQUIRED"
                    )
                elif label == "empty-blockers":
                    mutated["admission"]["blockers"] = []
                elif label == "root-extra-key":
                    mutated["forgedProductAdmission"] = True
                else:
                    mutated["summary"]["forgedProductAdmission"] = True
                self.reseal(mutated)
                with self.assertRaises(ValueError):
                    acquisition.validate_receipt_semantics(
                        mutated, self.contract
                    )

    def test_external_search_snapshots_are_qualified_corroboration_only(self) -> None:
        search = self.committed["externalArtifactSearch"]
        boundary = search["scopeBoundary"]
        self.assertEqual("ACCESSIBLE_LOCAL_AND_REMOTE_SCOPE_ONLY", boundary["claim"])
        self.assertFalse(boundary["globalExhaustionClaim"])
        self.assertEqual(
            "PERMISSION_UNCHECKED", boundary["volumeShadowCopy"]["status"]
        )
        self.assertFalse(boundary["volumeShadowCopy"]["admissionInput"])

        for snapshot_name in ("driverShaderCaches", "gitAndRemote"):
            snapshot = search[snapshot_name]
            self.assertEqual(
                "EXTERNAL_READ_ONLY_AUDIT_SNAPSHOT", snapshot["evidenceKind"]
            )
            self.assertFalse(snapshot["admissionInput"])
            self.assertTrue(snapshot["corroborationOnly"])
            self.assertFalse(snapshot["regeneratedByThisGenerator"])
            self.assertIsNone(snapshot["verificationManifest"])
            self.assertEqual("2026-08-10", snapshot["observedDate"])
            self.assertEqual(
                "SESSION_DATE_ONLY", snapshot["observationTimePrecision"]
            )
            self.assertTrue(snapshot["accessCaveat"])


if __name__ == "__main__":
    unittest.main()
