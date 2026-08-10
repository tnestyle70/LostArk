#!/usr/bin/env python3
from __future__ import annotations

import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

import build_artist_31470_material_texture_runtime_binding as binding


def seal_row(row: dict) -> None:
    row.pop("rowSha256", None)
    row["rowSha256"] = binding.canonical_sha256(row)


def seal_receipt(receipt: dict) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = binding.canonical_sha256(receipt)


class MaterialTextureRuntimeBindingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.policy = binding.read_json(binding.DEFAULT_POLICY)
        cls.contract = binding.read_json(binding.DEFAULT_CONTRACT)
        cls.runtime_oracle = binding.read_json(binding.DEFAULT_RUNTIME_ORACLE)
        cls.acquisition = binding.read_json(binding.DEFAULT_ACQUISITION)
        cls.resource_manifest = binding.read_json(binding.DEFAULT_RESOURCE_MANIFEST)
        cls.exact_dds = binding.read_json(binding.DEFAULT_EXACT_DDS)
        cls.candidate = binding.read_json(binding.DEFAULT_CANDIDATE)
        cls.runtime_cook = binding.read_json(binding.DEFAULT_RUNTIME_COOK)
        cls.resource_export = binding.read_json(binding.DEFAULT_RESOURCE_EXPORT)
        cls.source_pack = binding.read_json(binding.DEFAULT_SOURCE_PACK)
        cls.receipt = binding.read_json(binding.DEFAULT_OUTPUT)

    def validate(
        self,
        receipt: dict,
        *,
        require_approval: bool = True,
        runtime_cook: dict | None = None,
        resource_export: dict | None = None,
        source_pack: dict | None = None,
    ) -> None:
        binding.validate_receipt(
            receipt,
            self.policy,
            self.contract,
            self.resource_manifest,
            self.exact_dds,
            self.candidate,
            runtime_oracle=self.runtime_oracle,
            acquisition=self.acquisition,
            runtime_cook=runtime_cook,
            resource_export=resource_export,
            source_pack=source_pack,
            require_approval=require_approval,
        )

    def assert_invalid(self, receipt: dict) -> None:
        with self.assertRaises(ValueError):
            self.validate(receipt)

    def assert_pure_invalid(self, receipt: dict) -> None:
        with self.assertRaises(ValueError):
            self.validate(receipt, require_approval=False)

    def resolved_resource(self, receipt: dict) -> dict:
        return next(row for row in receipt["textureResources"] if row["status"] == binding.RESOLVED_STATUS)

    def unresolved_resource(self, receipt: dict) -> dict:
        return next(row for row in receipt["textureResources"] if row["status"] == binding.UNRESOLVED_STATUS)

    def test_checked_receipt_is_valid(self) -> None:
        self.validate(self.receipt)
        self.validate(self.receipt, require_approval=False)
        self.assertEqual(self.receipt["summary"]["resolvedBindingRowCount"], 68)
        self.assertEqual(self.receipt["summary"]["unresolvedBindingRowCount"], 4)
        self.assertFalse(self.receipt["admission"]["product"])

    def test_external_rebuild_is_deterministic(self) -> None:
        rebuilt = binding.build_receipt(
            self.policy,
            self.contract,
            self.resource_manifest,
            self.exact_dds,
            self.candidate,
            self.runtime_cook,
            self.resource_export,
            self.source_pack,
            runtime_oracle=self.runtime_oracle,
            acquisition=self.acquisition,
        )
        self.assertEqual(rebuilt, self.receipt)

    def test_exact_denominators_and_reverse_coverage(self) -> None:
        self.assertEqual(len(self.receipt["materialTextureBindings"]), 72)
        self.assertEqual(len(self.receipt["textureResources"]), 48)
        self.assertEqual(len(self.receipt["provisioningProposals"]), 4)
        self.assertEqual(
            {row["logicalTexturePath"] for row in self.receipt["textureResources"]},
            {row["logicalTexturePath"] for row in self.policy["samplerPolicies"]},
        )
        self.assertEqual(
            sum(len(row["materialOccurrenceIds"]) for row in self.receipt["materialTextureBindings"]),
            83,
        )

    def test_four_exact_dds_rows_remain_unresolved(self) -> None:
        unresolved = {
            row["logicalTexturePath"]: row
            for row in self.receipt["textureResources"]
            if row["status"] == binding.UNRESOLVED_STATUS
        }
        self.assertEqual(set(unresolved), binding.EXPECTED_MISSING_LOGICAL_PATHS)
        for logical, row in unresolved.items():
            self.assertIsNone(row["runtimeAssetId"])
            self.assertFalse(row["runtimeAssetAdmission"])
            self.assertEqual(row["candidateObservations"], [])
            self.assertEqual(
                row["exactDdsEvidence"]["fixtureAssetId"],
                next(
                    asset["fixtureAssetId"]
                    for asset in self.exact_dds["assets"]
                    if asset["logicalObjectPath"] == logical
                ),
            )

    def test_proposals_use_only_nested_exact_dds_fixture_ids(self) -> None:
        expected = {row["fixtureAssetId"] for row in self.exact_dds["assets"]}
        actual = {row["proposedRuntimeAssetId"] for row in self.receipt["provisioningProposals"]}
        self.assertEqual(actual, expected)
        for row in self.receipt["provisioningProposals"]:
            self.assertEqual(row["policy"], binding.PROVISIONING_POLICY)
            self.assertFalse(row["sourceExact"])
            self.assertFalse(row["runtimeAssetAdmission"])

    def test_source_pack_gap_is_explicit_and_does_not_invent_hashes(self) -> None:
        gaps = [row for row in self.receipt["textureResources"] if row["sourcePackage"] is None]
        self.assertEqual(len(gaps), 3)
        self.assertEqual(
            {row["logicalTexturePath"] for row in gaps},
            {
                "wp_mn_lrcn_01.tex.wp_mn_lrcn_01_d",
                "wp_mn_lrcn_01.tex.wp_mn_lrcn_01_n",
                "wp_mn_lrcn_01.tex.wp_mn_lrcn_01_s",
            },
        )

    def test_format_version_rejects_bool_float_and_string(self) -> None:
        for value in (True, 1.0, "1"):
            with self.subTest(value=value):
                candidate = copy.deepcopy(self.receipt)
                candidate["formatVersion"] = value
                seal_receipt(candidate)
                self.assert_invalid(candidate)

    def test_binding_owner_reseal_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = candidate["materialTextureBindings"][0]
        row["recipeId"] = next(
            other["recipeId"]
            for other in candidate["materialTextureBindings"]
            if other["recipeId"] != row["recipeId"]
        )
        seal_row(row)
        seal_receipt(candidate)
        self.assert_invalid(candidate)

    def test_occurrence_reseal_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = candidate["materialTextureBindings"][0]
        row["materialOccurrenceIds"] = ["source-active-999"]
        seal_row(row)
        seal_receipt(candidate)
        self.assert_invalid(candidate)

    def test_descriptor_and_srv_reseal_are_rejected(self) -> None:
        for field in ("samplerDescriptor", "srvIdentity"):
            with self.subTest(field=field):
                candidate = copy.deepcopy(self.receipt)
                row = candidate["materialTextureBindings"][0]
                if field == "samplerDescriptor":
                    row[field]["sRgb"] = not row[field]["sRgb"]
                    row["samplerDescriptorSha256"] = binding.canonical_sha256(row[field])
                else:
                    row[field]["sRgb"] = not row[field]["sRgb"]
                    row["srvIdentitySha256"] = binding.canonical_sha256(row[field])
                seal_row(row)
                seal_receipt(candidate)
                self.assert_invalid(candidate)

    def test_runtime_path_traversal_absolute_backslash_and_wrong_prefix_reject(self) -> None:
        for asset_id in (
            "../Effect/Artist/Textures/a.dds",
            "C:/Effect/Artist/Textures/a.dds",
            "Effect\\Artist\\Textures\\a.dds",
            "Map/Artist/Textures/a.dds",
            "Effect/Artist/Textures/a.png",
        ):
            with self.subTest(asset_id=asset_id):
                candidate = copy.deepcopy(self.receipt)
                row = self.resolved_resource(candidate)
                row["runtimeAssetId"] = asset_id
                row["runtimeCookEvidence"]["runtimeAssetId"] = asset_id
                seal_row(row)
                seal_receipt(candidate)
                self.assert_invalid(candidate)

    def test_runtime_asset_casefold_collision_rejects(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        resolved = [row for row in candidate["textureResources"] if row["status"] == binding.RESOLVED_STATUS]
        resolved[1]["runtimeAssetId"] = resolved[0]["runtimeAssetId"].upper()
        resolved[1]["runtimeCookEvidence"]["runtimeAssetId"] = resolved[1]["runtimeAssetId"]
        seal_row(resolved[1])
        seal_receipt(candidate)
        self.assert_invalid(candidate)

    def test_unresolved_promotion_without_deploy_receipt_rejects(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = self.unresolved_resource(candidate)
        row["status"] = binding.RESOLVED_STATUS
        row["runtimeAssetId"] = row["exactDdsEvidence"]["fixtureAssetId"]
        row["runtimeAssetAdmission"] = True
        row["blockers"] = []
        seal_row(row)
        seal_receipt(candidate)
        self.assert_invalid(candidate)

    def test_proposal_target_policy_and_source_exact_mutations_reject(self) -> None:
        mutations = (
            lambda row: row.__setitem__("proposedRuntimeAssetId", "Effect/Artist/Textures/forged.dds"),
            lambda row: row.__setitem__("policy", "SOURCE_EXACT"),
            lambda row: row.__setitem__("sourceExact", True),
        )
        for mutate in mutations:
            candidate = copy.deepcopy(self.receipt)
            row = candidate["provisioningProposals"][0]
            mutate(row)
            seal_row(row)
            seal_receipt(candidate)
            self.assert_invalid(candidate)

    def test_missing_duplicate_and_reordered_rows_reject(self) -> None:
        mutations = (
            lambda value: value["materialTextureBindings"].pop(),
            lambda value: value["materialTextureBindings"].append(copy.deepcopy(value["materialTextureBindings"][0])),
            lambda value: value["materialTextureBindings"].reverse(),
        )
        for mutate in mutations:
            candidate = copy.deepcopy(self.receipt)
            mutate(candidate)
            seal_receipt(candidate)
            self.assert_invalid(candidate)

    def test_coordinated_source_package_reseal_hits_external_or_independent_pin(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = next(row for row in candidate["textureResources"] if row["sourcePackage"] is not None)
        row["sourcePackage"]["rawSha256"] = "0" * 64
        seal_row(row)
        seal_receipt(candidate)
        with self.assertRaisesRegex(
            ValueError,
            "texture resources are not pinned external-evidence-derived|independent approval pin",
        ):
            self.validate(candidate)

    def test_pure_validator_rejects_unknown_texture_resource_semantic(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = self.resolved_resource(candidate)
        row["unknownFutureSemantic"] = {"ready": True}
        seal_row(row)
        seal_receipt(candidate)
        self.assert_pure_invalid(candidate)

    def test_pure_validator_rejects_unknown_cook_semantic(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = self.resolved_resource(candidate)
        row["runtimeCookEvidence"]["unknownFutureSemantic"] = 17
        seal_row(row)
        seal_receipt(candidate)
        self.assert_pure_invalid(candidate)

    def test_pure_validator_rejects_forged_source_package_sha(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = next(row for row in candidate["textureResources"] if row["sourcePackage"] is not None)
        row["sourcePackage"]["rawSha256"] = "0" * 64
        seal_row(row)
        seal_receipt(candidate)
        self.assert_pure_invalid(candidate)

    def test_pure_validator_rejects_lowercased_source_package_identity(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = next(
            row
            for row in candidate["textureResources"]
            if row["sourcePackage"] is not None
            and row["sourcePackage"]["logicalPackage"]
            != row["sourcePackage"]["logicalPackage"].lower()
        )
        row["sourcePackage"]["logicalPackage"] = row["sourcePackage"]["logicalPackage"].lower()
        seal_row(row)
        seal_receipt(candidate)
        self.assert_pure_invalid(candidate)

    def test_candidate_observation_is_exact_asset_id_not_basename(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = next(row for row in candidate["textureResources"] if row["candidateObservations"])
        row["candidateObservations"][0]["runtimeAssetId"] = (
            "Effect/Artist/Textures/other/" + PureName(row["runtimeAssetId"])
        )
        seal_row(row)
        seal_receipt(candidate)
        self.assert_invalid(candidate)

    def test_external_cook_full_path_not_basename_join(self) -> None:
        cook = copy.deepcopy(self.runtime_cook)
        logical = self.receipt["textureResources"][0]["logicalTexturePath"]
        row = next(row for row in cook["assets"] if row["sourceAssetPath"] == logical)
        row["sourceAssetPath"] = logical.rsplit(".", 1)[-1]
        with self.assertRaises(ValueError):
            binding.build_receipt(
                self.policy,
                self.contract,
                self.resource_manifest,
                self.exact_dds,
                self.candidate,
                cook,
                self.resource_export,
                self.source_pack,
                runtime_oracle=self.runtime_oracle,
                acquisition=self.acquisition,
            )

    def test_external_raw_receipt_coordinated_reseal_attacks_reject(self) -> None:
        cases = (
            ("cook", self.runtime_cook, "runtimeRoot", "C:/forged"),
            ("export", self.resource_export, "sourceManifest", "forged.json"),
            ("source-pack", self.source_pack, "sourceManifest", "forged.json"),
        )
        for label, source, key, value in cases:
            with self.subTest(label=label), tempfile.TemporaryDirectory() as directory:
                path = Path(directory) / f"{label}.json"
                mutated = copy.deepcopy(source)
                mutated[key] = value
                path.write_text(json.dumps(mutated, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
                kwargs = {
                    "runtime_cook_path": path if label == "cook" else binding.DEFAULT_RUNTIME_COOK,
                    "resource_export_path": path if label == "export" else binding.DEFAULT_RESOURCE_EXPORT,
                    "source_pack_path": path if label == "source-pack" else binding.DEFAULT_SOURCE_PACK,
                }
                with self.assertRaises(ValueError):
                    binding.build_receipt(
                        self.policy,
                        self.contract,
                        self.resource_manifest,
                        self.exact_dds,
                        self.candidate,
                        mutated if label == "cook" else self.runtime_cook,
                        mutated if label == "export" else self.resource_export,
                        mutated if label == "source-pack" else self.source_pack,
                        runtime_oracle=self.runtime_oracle,
                        acquisition=self.acquisition,
                        **kwargs,
                    )

    def test_pure_validator_reads_and_rejects_mutated_default_external_bytes(self) -> None:
        mutated = copy.deepcopy(self.runtime_cook)
        mutated["runtimeRoot"] = "C:/forged-runtime-root"
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "runtime-cook-receipt.json"
            path.write_text(
                json.dumps(mutated, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            with mock.patch.object(binding, "DEFAULT_RUNTIME_COOK", path):
                with self.assertRaisesRegex(
                    ValueError,
                    "approved external evidence byte count changed|approved external raw evidence changed",
                ):
                    self.validate(self.receipt, require_approval=False)

    def test_pure_validator_rejects_supplied_coordinated_runtime_asset_reseal(self) -> None:
        logical = "fx_tex_00.fx_a_decal_013"
        forged_asset = "Effect/Artist/Textures/forged_resealed.dds"
        mutated_cook = copy.deepcopy(self.runtime_cook)
        cook_row = next(
            row for row in mutated_cook["assets"] if row["sourceAssetPath"] == logical
        )
        cook_row["runtimeAssetId"] = forged_asset

        candidate = copy.deepcopy(self.receipt)
        resource = next(
            row for row in candidate["textureResources"]
            if row["logicalTexturePath"] == logical
        )
        resource["runtimeAssetId"] = forged_asset
        resource["runtimeCookEvidence"]["runtimeAssetId"] = forged_asset
        resource["candidateObservations"] = []
        seal_row(resource)
        for binding_row in candidate["materialTextureBindings"]:
            if binding_row["logicalTexturePath"] == logical:
                binding_row["runtimeAssetId"] = forged_asset
                seal_row(binding_row)
        seal_receipt(candidate)

        with self.assertRaisesRegex(
            ValueError,
            "supplied runtime cook object differs from approved external authority",
        ):
            self.validate(
                candidate,
                require_approval=False,
                runtime_cook=mutated_cook,
                resource_export=self.resource_export,
                source_pack=self.source_pack,
            )

    def test_strict_json_rejects_duplicate_keys_and_bom(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            duplicate = Path(directory) / "duplicate.json"
            duplicate.write_text('{"schema":1,"schema":2}', encoding="utf-8")
            with self.assertRaises(ValueError):
                binding.read_json(duplicate)
            bom = Path(directory) / "bom.json"
            bom.write_bytes(b"\xef\xbb\xbf{}")
            with self.assertRaises(ValueError):
                binding.read_json(bom)

    def test_tracked_text_hash_is_lf_crlf_invariant(self) -> None:
        payload = binding.DEFAULT_POLICY.read_bytes().decode("utf-8").replace("\r\n", "\n").replace("\r", "\n")
        with tempfile.TemporaryDirectory() as directory:
            lf = Path(directory) / "lf.json"
            crlf = Path(directory) / "crlf.json"
            lf.write_bytes(payload.encode("utf-8"))
            crlf.write_bytes(payload.replace("\n", "\r\n").encode("utf-8"))
            self.assertEqual(binding.tracked_text_sha256(lf), binding.tracked_text_sha256(crlf))


def PureName(asset_id: str) -> str:
    return asset_id.rsplit("/", 1)[-1]


if __name__ == "__main__":
    unittest.main(verbosity=2)
