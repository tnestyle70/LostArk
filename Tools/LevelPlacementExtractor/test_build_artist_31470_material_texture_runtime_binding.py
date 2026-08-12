#!/usr/bin/env python3
from __future__ import annotations

import copy
import json
import shutil
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


def seal_deployment_receipt(receipt: dict) -> None:
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
        cls.deployment_receipt = binding.read_json(
            binding.DEFAULT_EXACT_DDS_DEPLOYMENT
        )
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
        deployment_receipt: dict | None = None,
        candidate: dict | None = None,
    ) -> None:
        binding.validate_receipt(
            receipt,
            self.policy,
            self.contract,
            self.resource_manifest,
            self.exact_dds,
            self.candidate if candidate is None else candidate,
            runtime_oracle=self.runtime_oracle,
            acquisition=self.acquisition,
            runtime_cook=runtime_cook,
            resource_export=resource_export,
            source_pack=source_pack,
            deployment_receipt=deployment_receipt,
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

    def deployed_resource(self, receipt: dict) -> dict:
        return next(
            row
            for row in receipt["textureResources"]
            if row["status"] == binding.DEPLOYED_STATUS
        )

    def test_checked_receipt_is_valid(self) -> None:
        self.validate(self.receipt)
        self.validate(self.receipt, require_approval=False)
        self.assertEqual(self.receipt["summary"]["resolvedBindingRowCount"], 77)
        self.assertEqual(self.receipt["summary"]["unresolvedBindingRowCount"], 0)
        self.assertEqual(self.receipt["summary"]["runtimeCookBindingRowCount"], 73)
        self.assertEqual(self.receipt["summary"]["deploymentBindingRowCount"], 4)
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
        self.assertEqual(len(self.receipt["materialTextureBindings"]), 77)
        self.assertEqual(len(self.receipt["textureResources"]), 52)
        self.assertEqual(len(self.receipt["provisioningProposals"]), 4)
        self.assertEqual(
            {row["logicalTexturePath"] for row in self.receipt["textureResources"]},
            {row["logicalTexturePath"] for row in self.policy["samplerPolicies"]},
        )
        self.assertEqual(
            sum(len(row["materialOccurrenceIds"]) for row in self.receipt["materialTextureBindings"]),
            94,
        )

    def test_four_exact_dds_rows_are_reconstructed_deployment_bound(self) -> None:
        deployed = {
            row["logicalTexturePath"]: row
            for row in self.receipt["textureResources"]
            if row["status"] == binding.DEPLOYED_STATUS
        }
        self.assertEqual(set(deployed), binding.EXPECTED_MISSING_LOGICAL_PATHS)
        for logical, row in deployed.items():
            self.assertEqual(
                row["status"],
                "RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT",
            )
            self.assertTrue(row["runtimeAssetAdmission"])
            self.assertFalse(row["sourceExact"])
            self.assertEqual(
                row["deploymentEvidence"]["basis"],
                binding.DEPLOYMENT_BASIS,
            )
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
            self.assertTrue(row["runtimeAssetAdmission"])
            self.assertEqual(
                row["deploymentStatus"],
                binding.DEPLOYMENT_COMPLETE_STATUS,
            )

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
        row = self.deployed_resource(candidate)
        row["status"] = binding.RESOLVED_STATUS
        row["deploymentEvidence"] = None
        row["runtimeAssetAdmission"] = True
        row["blockers"] = []
        seal_row(row)
        seal_receipt(candidate)
        self.assert_invalid(candidate)

    def test_supplied_deployment_receipt_coordinated_mutations_reject(self) -> None:
        def remove_row(value: dict) -> None:
            value["assets"].pop()

        def swap_rows(value: dict) -> None:
            value["assets"][0], value["assets"][1] = (
                value["assets"][1],
                value["assets"][0],
            )

        def mutate_asset(value: dict, field: str, replacement: object) -> None:
            row = value["assets"][0]
            if field in {"byteCount", "rawSha256"}:
                row["deployedFile"][field] = replacement
            else:
                row[field] = replacement
            seal_row(row)

        mutations = (
            remove_row,
            swap_rows,
            lambda value: mutate_asset(
                value,
                "runtimeAssetId",
                "Effect/Artist/Textures/FX_TEX_00/forged.dds",
            ),
            lambda value: mutate_asset(value, "byteCount", 1),
            lambda value: mutate_asset(value, "rawSha256", "0" * 64),
            lambda value: mutate_asset(value, "deploymentStatus", "PROPOSED"),
        )
        for mutate in mutations:
            candidate = copy.deepcopy(self.deployment_receipt)
            mutate(candidate)
            seal_deployment_receipt(candidate)
            with self.subTest(mutate=repr(mutate)), self.assertRaisesRegex(
                ValueError,
                "supplied deployment receipt differs from approved tracked authority",
            ):
                self.validate(
                    self.receipt,
                    require_approval=False,
                    deployment_receipt=candidate,
                )

    def test_deployment_runtime_asset_must_equal_exact_dds_fixture_id(self) -> None:
        candidate = copy.deepcopy(self.deployment_receipt)
        row = candidate["assets"][0]
        forged = "Effect/Artist/Textures/FX_TEX_00/forged_safe.dds"
        row["runtimeAssetId"] = forged
        row["deployedFile"]["runtimeAssetId"] = forged
        seal_row(row)
        seal_deployment_receipt(candidate)
        exact_rows = {
            row["logicalObjectPath"].casefold(): row
            for row in self.exact_dds["assets"]
        }
        with self.assertRaisesRegex(ValueError, "deployment row contract changed"):
            binding.validate_deployment_rows(candidate, exact_rows)

    def test_coordinated_candidate_observation_reseal_rejects_pinned_bytes(self) -> None:
        mutated_candidate = copy.deepcopy(self.candidate)
        receipt = copy.deepcopy(self.receipt)
        resource = next(
            row for row in receipt["textureResources"]
            if row["candidateObservations"]
        )
        original = resource["candidateObservations"][0]
        changed = False
        for element in mutated_candidate["elements"]:
            for candidate_resource in element["resources"]:
                if (
                    element.get("id") == original["elementId"]
                    and candidate_resource.get("assetId") == original["runtimeAssetId"]
                    and candidate_resource.get("slotId") == original["slotId"]
                ):
                    candidate_resource["slotId"] = "forged_slot_id"
                    changed = True
                    break
            if changed:
                break
        self.assertTrue(changed)
        resource["candidateObservations"] = binding.candidate_observations(
            mutated_candidate,
            resource["runtimeAssetId"],
        )
        seal_row(resource)
        receipt["materialTextureBindings"] = binding.build_material_bindings(
            self.policy,
            self.contract,
            receipt["textureResources"],
        )
        receipt["provisioningProposals"] = binding.build_provisioning_proposals(
            receipt["textureResources"]
        )
        seal_receipt(receipt)
        with self.assertRaisesRegex(
            ValueError,
            "supplied tracked object differs from approved authority: nativeV14Candidate",
        ):
            self.validate(
                receipt,
                require_approval=False,
                candidate=mutated_candidate,
            )
        with self.assertRaisesRegex(
            ValueError,
            "supplied tracked object differs from approved authority: nativeV14Candidate",
        ):
            binding.build_receipt(
                self.policy,
                self.contract,
                self.resource_manifest,
                self.exact_dds,
                mutated_candidate,
                self.runtime_cook,
                self.resource_export,
                self.source_pack,
                runtime_oracle=self.runtime_oracle,
                acquisition=self.acquisition,
            )

    def test_all_supplied_tracked_objects_must_equal_pinned_bytes(self) -> None:
        base = (
            self.policy,
            self.contract,
            self.runtime_oracle,
            self.acquisition,
            self.resource_manifest,
            self.exact_dds,
            self.candidate,
        )
        labels = (
            "materialPolicy",
            "typedMaterialContract",
            "materialRuntimeOracle",
            "materialSourceValueAcquisition",
            "resourceSourceManifest",
            "exactDdsRecoveryReceipt",
            "nativeV14Candidate",
        )
        for index, label in enumerate(labels):
            supplied = list(base)
            supplied[index] = copy.deepcopy(supplied[index])
            supplied[index]["coordinatedForgedSemantic"] = label
            with self.subTest(label=label), self.assertRaisesRegex(
                ValueError,
                f"supplied tracked object differs from approved authority: {label}",
            ):
                binding.bind_supplied_tracked_inputs(*supplied)

    def test_deployment_authority_identity_mutations_reject(self) -> None:
        fields = (
            "artifactAuthorityCommit",
            "artifactAuthorityTree",
            "gitBlob",
            "receiptSha256",
            "approvalProjectionSha256",
            "implementationProjectionSha256",
        )
        for field in fields:
            candidate = copy.deepcopy(self.receipt)
            candidate["sourceEvidence"]["exactDdsRuntimeDeploymentReceipt"][field] = (
                "0" * 40 if field in {"artifactAuthorityCommit", "artifactAuthorityTree", "gitBlob"}
                else "0" * 64
            )
            seal_receipt(candidate)
            with self.subTest(field=field):
                self.assert_pure_invalid(candidate)

    def test_deployment_resource_remove_and_ab_swap_reject(self) -> None:
        removed = copy.deepcopy(self.receipt)
        index = next(
            index
            for index, row in enumerate(removed["textureResources"])
            if row["status"] == binding.DEPLOYED_STATUS
        )
        removed["textureResources"].pop(index)
        seal_receipt(removed)
        self.assert_pure_invalid(removed)

        swapped = copy.deepcopy(self.receipt)
        deployed = [
            row
            for row in swapped["textureResources"]
            if row["status"] == binding.DEPLOYED_STATUS
        ]
        deployed[0]["deploymentEvidence"], deployed[1]["deploymentEvidence"] = (
            deployed[1]["deploymentEvidence"],
            deployed[0]["deploymentEvidence"],
        )
        seal_row(deployed[0])
        seal_row(deployed[1])
        seal_receipt(swapped)
        self.assert_pure_invalid(swapped)

    def test_fake_completed_deployment_status_rejects(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        proposal = candidate["provisioningProposals"][0]
        proposal["deploymentStatus"] = "COMPLETED_WITHOUT_POST_VERIFY"
        seal_row(proposal)
        seal_receipt(candidate)
        self.assert_pure_invalid(candidate)

    def test_pure_validator_reads_and_rejects_mutated_deployment_receipt_bytes(self) -> None:
        mutated = copy.deepcopy(self.deployment_receipt)
        row = mutated["assets"][0]
        row["deployedFile"]["rawSha256"] = "0" * 64
        seal_row(row)
        seal_deployment_receipt(mutated)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "deployment-receipt.json"
            path.write_text(
                json.dumps(mutated, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            with mock.patch.object(binding, "DEFAULT_EXACT_DDS_DEPLOYMENT", path):
                with self.assertRaisesRegex(
                    ValueError,
                    "approved exact-DDS deployment tracked bytes changed",
                ):
                    self.validate(self.receipt, require_approval=False)

    def test_pure_validator_reads_and_rejects_mutated_deployed_runtime_bytes(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            runtime_root = Path(directory) / "Resources"
            for row in self.deployment_receipt["assets"]:
                relative = Path(*row["runtimeAssetId"].split("/"))
                source = binding.DEFAULT_RUNTIME_RESOURCES / relative
                target = runtime_root / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(source, target)
            first = self.deployment_receipt["assets"][0]
            first_path = runtime_root / Path(*first["runtimeAssetId"].split("/"))
            payload = bytearray(first_path.read_bytes())
            payload[-1] ^= 0x01
            first_path.write_bytes(payload)
            with mock.patch.object(binding, "DEFAULT_RUNTIME_RESOURCES", runtime_root):
                with self.assertRaisesRegex(
                    ValueError,
                    "deployed runtime payload identity changed",
                ):
                    self.validate(self.receipt, require_approval=False)

    def test_product_and_source_exact_promotions_reject(self) -> None:
        candidates: list[dict] = []

        product = copy.deepcopy(self.receipt)
        product["admission"]["product"] = True
        seal_receipt(product)
        candidates.append(product)

        contract = copy.deepcopy(self.receipt)
        contract["bindingContract"]["sourceExact"] = True
        seal_receipt(contract)
        candidates.append(contract)

        resource = copy.deepcopy(self.receipt)
        deployed = self.deployed_resource(resource)
        deployed["sourceExact"] = True
        seal_row(deployed)
        seal_receipt(resource)
        candidates.append(resource)

        binding_row = copy.deepcopy(self.receipt)
        binding_row["materialTextureBindings"][0]["sourceExact"] = True
        seal_row(binding_row["materialTextureBindings"][0])
        seal_receipt(binding_row)
        candidates.append(binding_row)

        for candidate in candidates:
            with self.subTest(index=candidates.index(candidate)):
                self.assert_pure_invalid(candidate)

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
