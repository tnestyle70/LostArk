#!/usr/bin/env python3
from __future__ import annotations

import copy
import unittest

try:
    import build_artist_31470_reconstructed_runtime_program as builder
except ModuleNotFoundError:
    from Tools.EffectPipeline import (
        build_artist_31470_reconstructed_runtime_program as builder,
    )


class Artist31470ReconstructedRuntimeProgramTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.program = builder.build_program()
        builder.validate_program(cls.program)

    @staticmethod
    def _reseal_row(program: dict, section: str, index: int) -> None:
        row = program[section][index]
        row.pop("rowSha256", None)
        row["rowSha256"] = builder.canonical_sha256(row)

    @staticmethod
    def _reseal_route(program: dict) -> None:
        route = program["policyRouteBinding"]
        route.pop("bindingSha256", None)
        route["bindingSha256"] = builder.canonical_sha256(route)

    @staticmethod
    def _reseal_program(program: dict) -> None:
        program["sectionDigests"] = [
            builder.section_digest(section, program[section])
            for section in builder.SECTION_NAMES
        ]
        program["summary"] = builder.recompute_summary(program)
        program.pop("programSha256", None)
        program["programSha256"] = builder.canonical_sha256(program)

    def _assert_invalid(self, mutation) -> None:
        program = copy.deepcopy(self.program)
        changed_rows = mutation(program) or []
        for section, index in changed_rows:
            self._reseal_row(program, section, index)
        self._reseal_program(program)
        with self.assertRaises(builder.ContractError):
            builder.validate_program(program)

    def test_01_exact_denominators_and_admission_are_closed(self) -> None:
        summary = self.program["summary"]
        self.assertEqual(summary["emitterCount"], 35)
        self.assertEqual(summary["moduleCount"], 399)
        self.assertEqual(summary["propertyCount"], 1434)
        self.assertEqual(summary["primitiveLeafCount"], 1572)
        self.assertEqual(summary["distributionCount"], 629)
        self.assertEqual(summary["actionScheduleCount"], 7)
        self.assertEqual(summary["materialPolicyCount"], 260)
        self.assertEqual(summary["geometryCarrierCount"], 7)
        self.assertEqual(summary["geometryUseCount"], 13)
        self.assertEqual(summary["sourceExactRowCount"], 0)
        self.assertEqual(summary["resolvedMaterialTextureBindingCount"], 77)
        self.assertNotIn(
            "MATERIAL_TEXTURE_RUNTIME_ASSET_UNRESOLVED", self.program["blockerUnion"]
        )
        self.assertNotIn(
            "EXACT_DDS_TRANSACTIONAL_DEPLOYMENT_PENDING", self.program["blockerUnion"]
        )
        self.assertIn("R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE", self.program["blockerUnion"])
        self.assertFalse(self.program["admission"]["runtimeExecution"])
        self.assertFalse(self.program["admission"]["product"])

    def test_02_nested_type_and_path_attack_matrix_is_rejected(self) -> None:
        def schedule_bool(program):
            program["actionSchedules"][0]["globalTimeSeconds"] = True
            return [("actionSchedules", 0)]

        def diagnostic_mesh_string(program):
            index = next(i for i, row in enumerate(program["emitters"])
                         if row["rendererRuntimeConfig"]["mesh"] is not None)
            program["emitters"][index]["rendererRuntimeConfig"]["mesh"][
                "useModelMaterial"
            ] = "false"
            return [("emitters", index)]

        def post_enabled_string(program):
            index = next(i for i, row in enumerate(program["emitters"])
                         if row["screenPostAdapter"] is not None)
            adapter = program["emitters"][index]["screenPostAdapter"]
            adapter["enabled"] = "true"
            adapter.pop("adapterSha256")
            adapter["adapterSha256"] = builder.canonical_sha256(adapter)
            return [("emitters", index)]

        def capability_output_bool(program):
            index = next(i for i, row in enumerate(program["modules"])
                         if row["selectionKind"] == "RECONSTRUCTED_HANDLER")
            program["modules"][index]["capabilityNumericSamples"][0][
                "outputValues"
            ][0] = True
            return [("modules", index)]

        def distribution_output_bool(program):
            program["distributions"][0]["samples"][0]["outputValues"][0] = True
            return [("distributions", 0)]

        def family_expected_bool(program):
            program["materialFamilies"][0]["numericSamples"][0][
                "expectedFloat4"
            ][0] = True
            return [("materialFamilies", 0)]

        def sampler_srgb_integer(program):
            index = next(i for i, row in enumerate(program["materialPolicyRows"])
                         if row["domain"] == "SAMPLER_DESCRIPTOR")
            program["materialPolicyRows"][index]["samplerDescriptor"]["sRgb"] = 1
            return [("materialPolicyRows", index)]

        def geometry_vertex_bool(program):
            program["geometryCarriers"][0]["submeshes"][0]["vertexCount"] = True
            return [("geometryCarriers", 0)]

        def material_texture_traversal(program):
            program["materialTextureBindings"][0]["runtimeAssetId"] = "../escape.dds"
            return [("materialTextureBindings", 0)]

        def renderer_texture_traversal(program):
            program["rendererTextureResources"][0]["assetId"] = "../escape.dds"
            return [("rendererTextureResources", 0)]

        def evaluator_version_bool(program):
            contract = program["materialEvaluatorOracleContract"]
            contract["version"] = True
            contract.pop("contractSha256")
            contract["contractSha256"] = builder.canonical_sha256(contract)
            return []

        attacks = (
            schedule_bool, diagnostic_mesh_string, post_enabled_string,
            capability_output_bool, distribution_output_bool, family_expected_bool,
            sampler_srgb_integer, geometry_vertex_bool, material_texture_traversal,
            renderer_texture_traversal, evaluator_version_bool,
        )
        for attack in attacks:
            with self.subTest(attack=attack.__name__):
                self._assert_invalid(attack)

    def test_03_approval_and_owner_coordinated_reseals_are_rejected(self) -> None:
        def source_approval(program):
            index = next(i for i, row in enumerate(program["modules"])
                         if row["selectionKind"] == "RECONSTRUCTED_HANDLER")
            module = program["modules"][index]
            value = module["approvalRequiredMutatedOutput"] + "_FORGED"
            module["approvalRequiredMutatedOutput"] = value
            module["approvalSourceRow"]["requiredMutatedOutput"] = value
            module["approvalSourceRowSha256"] = builder.canonical_sha256(
                module["approvalSourceRow"]
            )
            ordered = sorted(
                (row for row in program["modules"]
                 if row["selectionKind"] == "RECONSTRUCTED_HANDLER"),
                key=lambda row: row["approvalSourceOrder"],
            )
            program["policyRouteBinding"]["sourcePolicyRowProjectionSha256"] = (
                builder.canonical_sha256([row["approvalSourceRow"] for row in ordered])
            )
            self._reseal_route(program)
            return [("modules", index)]

        def material_approval(program):
            row = program["materialPolicyRows"][0]
            row["approvalPolicyFamilyId"] += "_FORGED"
            row["approvalMaterialRow"]["policyFamilyId"] = row[
                "approvalPolicyFamilyId"
            ]
            row["approvalMaterialRowSha256"] = builder.canonical_sha256(
                row["approvalMaterialRow"]
            )
            ordered = sorted(program["materialPolicyRows"],
                             key=lambda item: item["approvalMaterialOrder"])
            program["policyRouteBinding"]["materialPolicyRowProjectionSha256"] = (
                builder.canonical_sha256([item["approvalMaterialRow"] for item in ordered])
            )
            self._reseal_route(program)
            return [("materialPolicyRows", 0)]

        def arithmetic_approval(program):
            row = program["materialFamilies"][0]
            row["evaluatorId"] += ".forged"
            row["approvalArithmeticRow"]["evaluatorId"] = row["evaluatorId"]
            row["approvalArithmeticRowSha256"] = builder.canonical_sha256(
                row["approvalArithmeticRow"]
            )
            ordered = sorted(program["materialFamilies"],
                             key=lambda item: item["approvalArithmeticOrder"])
            program["policyRouteBinding"]["materialArithmeticRowProjectionSha256"] = (
                builder.canonical_sha256([item["approvalArithmeticRow"] for item in ordered])
            )
            self._reseal_route(program)
            return [("materialFamilies", 0)]

        def geometry_approval(program):
            row = program["geometryCarriers"][0]
            row["approvalGeometryRow"]["payloadSha256"] = "1" * 64
            row["approvalGeometryRowSha256"] = builder.canonical_sha256(
                row["approvalGeometryRow"]
            )
            ordered = sorted(program["geometryCarriers"],
                             key=lambda item: item["approvalGeometryOrder"])
            program["policyRouteBinding"]["geometryPolicyRowProjectionSha256"] = (
                builder.canonical_sha256([item["approvalGeometryRow"] for item in ordered])
            )
            self._reseal_route(program)
            return [("geometryCarriers", 0)]

        def material_texture_owner_swap(program):
            left = 0
            right = next(i for i, row in enumerate(program["materialTextureBindings"])
                         if row["recipeId"]
                         != program["materialTextureBindings"][left]["recipeId"])
            rows = program["materialTextureBindings"]
            rows[left]["samplerPolicyRowId"], rows[right]["samplerPolicyRowId"] = (
                rows[right]["samplerPolicyRowId"], rows[left]["samplerPolicyRowId"]
            )
            return [("materialTextureBindings", left),
                    ("materialTextureBindings", right)]

        def renderer_texture_owner_swap(program):
            left = 0
            right = next(i for i, row in enumerate(program["rendererTextureResources"])
                         if row["emitterId"]
                         != program["rendererTextureResources"][left]["emitterId"])
            rows = program["rendererTextureResources"]
            rows[left]["emitterId"], rows[right]["emitterId"] = (
                rows[right]["emitterId"], rows[left]["emitterId"]
            )
            return [("rendererTextureResources", left),
                    ("rendererTextureResources", right)]

        for attack in (
            source_approval, material_approval, arithmetic_approval, geometry_approval,
            material_texture_owner_swap, renderer_texture_owner_swap,
        ):
            with self.subTest(attack=attack.__name__):
                self._assert_invalid(attack)

    def test_04_operational_and_typed_source_mutations_are_rejected(self) -> None:
        def rounded_schedule(program):
            program["actionSchedules"][1]["globalTimeSeconds"] = 1.3377
            return [("actionSchedules", 1)]

        def duration_default_zero(program):
            index = next(i for i, row in enumerate(program["emitters"])
                         if row["timing"]["emitterDurationPolicy"]
                         == "CURRENT_REVISION_CDO_RECONSTRUCTED_DEFAULT_V1")
            timing = program["emitters"][index]["timing"]
            timing["emitterDurationSeconds"] = 0.0
            timing["sourceProjectionSha256"] = builder.canonical_sha256(
                {key: timing[key] for key in builder.EMITTER_TIMING_KEYS[:9]}
            )
            timing.pop("timingSha256")
            timing["timingSha256"] = builder.canonical_sha256(timing)
            return [("emitters", index)]

        def burst_sentinel_laundering(program):
            index = next(i for i, row in enumerate(program["emitters"])
                         if row["timing"]["bursts"])
            program["emitters"][index]["timing"]["bursts"][0]["countMinimum"] = 0
            timing = program["emitters"][index]["timing"]
            timing.pop("timingSha256")
            timing["timingSha256"] = builder.canonical_sha256(timing)
            return [("emitters", index)]

        def seeded_lifetime_owner_swap(program):
            emitter_index = next(
                i for i, emitter in enumerate(program["emitters"])
                if any(program["modules"][j]["exactSourceClass"]
                       == "particlemodulelifetime_seeded"
                       for j in range(len(program["modules"]))
                       if program["modules"][j]["emitterId"] == emitter["emitterId"])
            )
            replacement = next(row["moduleId"] for row in program["modules"]
                               if row["exactSourceClass"] == "particlemodulelifetime")
            timing = program["emitters"][emitter_index]["timing"]
            timing["lifetimeModuleId"] = replacement
            timing["sourceProjectionSha256"] = builder.canonical_sha256(
                {key: timing[key] for key in builder.EMITTER_TIMING_KEYS[:9]}
            )
            timing.pop("timingSha256")
            timing["timingSha256"] = builder.canonical_sha256(timing)
            return [("emitters", emitter_index)]

        def capability_projection(program):
            index = next(i for i, row in enumerate(program["modules"])
                         if row["selectionKind"] == "RECONSTRUCTED_HANDLER")
            program["modules"][index]["capabilityLiteralBindingsSha256"] = "0" * 64
            return [("modules", index)]

        def unknown_property_semantic(program):
            index = next(i for i, row in enumerate(program["properties"])
                         if row["consumptionDecision"] == "READY_FOR_HANDLER")
            program["properties"][index]["consumptionDecision"] = "UNKNOWN_SEMANTIC"
            return [("properties", index)]

        def ribbon_cap_laundering(program):
            index = next(i for i, row in enumerate(program["emitters"])
                         if row["ribbonAdapter"] is not None)
            adapter = program["emitters"][index]["ribbonAdapter"]
            adapter["operationalMaxPoints"] = 64
            adapter.pop("adapterSha256")
            adapter["adapterSha256"] = builder.canonical_sha256(adapter)
            return [("emitters", index)]

        def ribbon_orientation_unknown(program):
            index = next(i for i, row in enumerate(program["emitters"])
                         if row["ribbonAdapter"] is not None)
            adapter = program["emitters"][index]["ribbonAdapter"]
            adapter["orientationPolicy"] = "UNKNOWN_ORIENTATION"
            adapter.pop("adapterSha256")
            adapter["adapterSha256"] = builder.canonical_sha256(adapter)
            return [("emitters", index)]

        for attack in (
            rounded_schedule, duration_default_zero, burst_sentinel_laundering,
            seeded_lifetime_owner_swap, capability_projection, unknown_property_semantic,
            ribbon_cap_laundering, ribbon_orientation_unknown,
        ):
            with self.subTest(attack=attack.__name__):
                self._assert_invalid(attack)

    def test_05_descriptor_and_srv_coordinated_mutations_are_rejected(self) -> None:
        def d3d_descriptor(program):
            index = next(i for i, row in enumerate(program["materialPolicyRows"])
                         if row["d3dDescriptorOracle"] is not None)
            oracle = program["materialPolicyRows"][index]["d3dDescriptorOracle"]
            kind = oracle["descriptorKind"]
            key = ({
                "D3D11_DEPTH_STENCIL_DESC": "DepthFunc",
                "D3D11_RASTERIZER_DESC": "CullMode",
                "D3D11_SAMPLER_DESC": "AddressU",
            })[kind]
            oracle["expectedDescriptor"][key] += 1
            oracle["actualDescriptor"][key] += 1
            return [("materialPolicyRows", index)]

        def d3d_srv(program):
            index = next(i for i, row in enumerate(program["materialPolicyRows"])
                         if row["d3dSrvOracle"] is not None)
            oracle = program["materialPolicyRows"][index]["d3dSrvOracle"]
            oracle["expectedSrv"]["Format"] += 1
            oracle["actualSrv"]["Format"] += 1
            return [("materialPolicyRows", index)]

        self._assert_invalid(d3d_descriptor)
        self._assert_invalid(d3d_srv)

    def test_06_shape_identity_and_admission_mutations_are_rejected(self) -> None:
        def extra_key(program):
            program["modules"][0]["forged"] = 1
            return [("modules", 0)]

        def missing_key(program):
            del program["modules"][0]["sourceObjectId"]
            return [("modules", 0)]

        def duplicate_id(program):
            program["emitters"][1]["emitterId"] = program["emitters"][0]["emitterId"]
            return [("emitters", 1)]

        def runtime_flip(program):
            program["admission"]["runtimeExecution"] = True
            return []

        def product_flip(program):
            program["admission"]["product"] = True
            return []

        for attack in (extra_key, missing_key, duplicate_id, runtime_flip, product_flip):
            with self.subTest(attack=attack.__name__):
                self._assert_invalid(attack)

    def test_07_tracked_json_lf_crlf_are_canonical_equivalent(self) -> None:
        raw = builder.git_blob(builder.SOURCE_EVIDENCE_COMMIT,
                               builder.ACTION_CUE_RECIPE_PATH)
        crlf = raw.replace(b"\n", b"\r\n")
        self.assertEqual(builder.canonical_lf_text_sha256(raw),
                         builder.canonical_lf_text_sha256(crlf))
        self.assertEqual(builder.canonical_sha256(
            builder.load_json_bytes(raw, "LF fixture")
        ), builder.canonical_sha256(
            builder.load_json_bytes(crlf, "CRLF fixture")
        ))

    def test_08_material_texture_receipt_projection_is_immutable(self) -> None:
        self.assertEqual(len(self.program["inputArtifacts"]), 13)
        self.assertEqual(
            self.program["summary"]["resolvedMaterialTextureBindingCount"], 77
        )
        self.assertEqual(sum(
            row["sourceReceiptStatus"] == "RESOLVED_EXACT_RUNTIME_COOK_RECEIPT"
            for row in self.program["materialTextureBindings"]
        ), 73)
        self.assertEqual(sum(
            row["sourceReceiptStatus"]
            == "RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT"
            for row in self.program["materialTextureBindings"]
        ), 4)

        def deployed_asset_swap(program):
            deployed = [
                i for i, row in enumerate(program["materialTextureBindings"])
                if row["sourceReceiptStatus"]
                == "RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT"
            ]
            left, right = next(
                (left, right)
                for left in deployed
                for right in deployed
                if left < right
                and program["materialTextureBindings"][left]["runtimeAssetId"]
                != program["materialTextureBindings"][right]["runtimeAssetId"]
            )
            rows = program["materialTextureBindings"]
            rows[left]["runtimeAssetId"], rows[right]["runtimeAssetId"] = (
                rows[right]["runtimeAssetId"], rows[left]["runtimeAssetId"]
            )
            return [("materialTextureBindings", left),
                    ("materialTextureBindings", right)]

        def safe_asset_substitution(program):
            index = next(
                i for i, row in enumerate(program["materialTextureBindings"])
                if row["resolutionStatus"] == "RESOLVED_EXACT_RUNTIME_ASSET"
            )
            program["materialTextureBindings"][index]["runtimeAssetId"] = (
                "Effect/Artist/Textures/forged-but-safe.dds"
            )
            return [("materialTextureBindings", index)]

        def deployed_status_and_basis_downgrade(program):
            index = next(
                i for i, row in enumerate(program["materialTextureBindings"])
                if row["sourceReceiptStatus"]
                == "RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT"
            )
            row = program["materialTextureBindings"][index]
            row["bindingBasis"] = "EXACT_FULL_LOGICAL_PATH_RUNTIME_COOK_RECEIPT"
            row["sourceReceiptStatus"] = "RESOLVED_EXACT_RUNTIME_COOK_RECEIPT"
            row["sourceProvisioningProposalId"] = ""
            row["sourceProvisioningProposalRowSha256"] = ""
            row["sourceDeploymentRowId"] = ""
            row["sourceDeploymentRowSha256"] = ""
            return [("materialTextureBindings", index)]

        def deployed_proposal_and_row_forge(program):
            index = next(
                i for i, row in enumerate(program["materialTextureBindings"])
                if row["sourceReceiptStatus"]
                == "RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT"
            )
            row = program["materialTextureBindings"][index]
            row["sourceProvisioningProposalId"] += ".forged"
            row["sourceProvisioningProposalRowSha256"] = "1" * 64
            row["sourceDeploymentRowId"] += ".forged"
            row["sourceDeploymentRowSha256"] = "2" * 64
            return [("materialTextureBindings", index)]

        def deployment_evidence_omission(program):
            index = next(
                i for i, row in enumerate(program["materialTextureBindings"])
                if row["sourceReceiptStatus"]
                == "RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT"
            )
            row = program["materialTextureBindings"][index]
            row["sourceDeploymentRowId"] = ""
            row["sourceDeploymentRowSha256"] = ""
            return [("materialTextureBindings", index)]

        def old_unresolved_row_downgrade(program):
            index = next(
                i for i, row in enumerate(program["materialTextureBindings"])
                if row["sourceReceiptStatus"]
                == "RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT"
            )
            row = program["materialTextureBindings"][index]
            row["runtimeAssetId"] = None
            row["resolutionStatus"] = "UNRESOLVED_RUNTIME_ASSET"
            row["bindingBasis"] = "EXACT_DDS_PROVISIONING_PENDING_NO_RUNTIME_ASSET"
            row["sourceReceiptStatus"] = "UNRESOLVED_RUNTIME_ASSET"
            row["sourceDeploymentRowId"] = ""
            row["sourceDeploymentRowSha256"] = ""
            row["blockers"] = [
                "MATERIAL_TEXTURE_RUNTIME_ASSET_UNRESOLVED",
                "EXACT_DDS_TRANSACTIONAL_DEPLOYMENT_PENDING",
                "R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE",
            ]
            sections = {
                section: program[section] for section in builder.SECTION_NAMES
            }
            program["blockerOwnership"] = (
                builder.build_blocker_ownership_contract(sections)
            )
            program["blockerUnion"] = (
                builder.derive_program_blockers_from_sections(sections)
            )
            program["admission"]["blockers"] = program["blockerUnion"]
            return [("materialTextureBindings", index)]

        def receipt_artifact_alias(program):
            index = next(
                i for i, row in enumerate(program["inputArtifacts"])
                if row["artifactId"] == "materialTextureBinding"
            )
            program["inputArtifacts"][index]["artifactId"] += ".forged"
            return [("inputArtifacts", index)]

        def receipt_artifact_sha(program):
            index = next(
                i for i, row in enumerate(program["inputArtifacts"])
                if row["artifactId"] == "materialTextureBinding"
            )
            program["inputArtifacts"][index]["selfSha256"] = "0" * 64
            return [("inputArtifacts", index)]

        def receipt_artifact_omission(program):
            program["inputArtifacts"] = [
                row for row in program["inputArtifacts"]
                if row["artifactId"] != "materialTextureBinding"
            ]
            return []

        def old_68_4_receipt_authority(program):
            index = next(
                i for i, row in enumerate(program["inputArtifacts"])
                if row["artifactId"] == "materialTextureBinding"
            )
            row = program["inputArtifacts"][index]
            row.update({
                "authorityCommitId": "fda3b5637847f9205915ad25ff02215424024b88",
                "authorityTreeId": "2f00f00851ee93f498dd6c13d6a3055209d4d8c3",
                "blobId": "3105c22a3c8e9b73b47b721ffab72d1254fc1750",
                "versionValue": 1,
                "trackedTextSha256": (
                    "3a097a174df6b940989c7ce6c7b4e3b7798256d200cf73e25e694dc827e4346e"
                ),
                "canonicalJsonSha256": (
                    "bd0abbb81cc6daa46f83f9ca8850512f597280c8df42fe5ee23b8e140720bcaf"
                ),
                "selfSha256": (
                    "39c91577c09b853fa55a8fd5531c1cddc4fef928d77a6caa7f67c472a56159e0"
                ),
            })
            return [("inputArtifacts", index)]

        for attack in (
            deployed_asset_swap, safe_asset_substitution,
            deployed_status_and_basis_downgrade, deployed_proposal_and_row_forge,
            deployment_evidence_omission, old_unresolved_row_downgrade,
            receipt_artifact_alias, receipt_artifact_sha, receipt_artifact_omission,
            old_68_4_receipt_authority,
        ):
            with self.subTest(attack=attack.__name__):
                self._assert_invalid(attack)

    def test_09_all_thirteen_input_artifacts_are_exact_authorities(self) -> None:
        attacks = (
            ("sourceSemantics", "authorityCommitId", "0" * 40),
            ("sourceSemantics", "authorityTreeId", "1" * 40),
            ("sourceCandidate", "authorityTreeId", "2" * 40),
            ("materialContract", "blobId", "3" * 40),
            ("geometryBinding", "canonicalJsonSha256", "4" * 64),
            ("geometryReceipt", "selfSha256", "5" * 64),
            ("materialTextureBinding", "trackedTextSha256", "6" * 64),
            ("materialTextureBinding", "schema", "forged.schema"),
        )
        for artifact_id, field, value in attacks:
            def mutate(program, artifact_id=artifact_id, field=field, value=value):
                index = next(
                    i for i, row in enumerate(program["inputArtifacts"])
                    if row["artifactId"] == artifact_id
                )
                program["inputArtifacts"][index][field] = value
                return [("inputArtifacts", index)]

            with self.subTest(artifact=artifact_id, field=field):
                self._assert_invalid(mutate)

    def test_10_capability_and_blocker_authorities_are_rederived(self) -> None:
        def capability_family_swap(program):
            rows = [
                row for row in program["modules"]
                if row["selectionKind"] == "RECONSTRUCTED_HANDLER"
            ]
            left = rows[0]
            right = next(
                row for row in rows
                if row["capabilityPolicyFamilyId"]
                != left["capabilityPolicyFamilyId"]
            )
            index = program["modules"].index(left)
            program["modules"][index]["capabilityPolicyFamilyId"] = (
                right["capabilityPolicyFamilyId"]
            )
            return [("modules", index)]

        def capability_source_row_sha(program):
            index = next(
                i for i, row in enumerate(program["modules"])
                if row["selectionKind"] == "RECONSTRUCTED_HANDLER"
            )
            program["modules"][index]["capabilitySourceRowSha256"] = "0" * 64
            return [("modules", index)]

        def required_blockers_removed_together(program):
            removed = {
                "MANUAL_HUMAN_EYE_VALIDATION_0_OF_35",
                "R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE",
                "SOURCE_FIDELITY_BLOCKERS_PRESERVED",
                "MATERIAL_TEXTURE_RUNTIME_ASSET_UNRESOLVED",
                "R5_SIX_RENDERER_FAMILIES_NOT_COMPLETE",
            }
            program["blockerUnion"] = [
                token for token in program["blockerUnion"] if token not in removed
            ]
            program["admission"]["blockers"] = [
                token for token in program["admission"]["blockers"]
                if token not in removed
            ]
            return []

        for attack in (
            capability_family_swap, capability_source_row_sha,
            required_blockers_removed_together,
        ):
            with self.subTest(attack=attack.__name__):
                self._assert_invalid(attack)

    def test_11_strict_numeric_and_emitter_source_identity_contracts(self) -> None:
        for value in ([False], ["1.0"]):
            with self.subTest(vector=value):
                with self.assertRaises(builder.ContractError):
                    builder.vector_values(value, 1)
        with self.assertRaises(builder.ContractError):
            builder.material_value_slots([False, 0, 0, 0])

        emitter_attacks = (
            ("sourceEmitterPath", "forged.emitter"),
            ("sourceEmitterNodeId", "forged-node"),
            ("sourceElementId", "forged-element"),
            ("selectedLodNodeId", "forged-lod-node"),
            ("selectedLodRecordSha256", "0" * 64),
        )
        for field, value in emitter_attacks:
            def mutate(program, field=field, value=value):
                program["emitters"][0][field] = value
                return [("emitters", 0)]

            with self.subTest(emitter_field=field):
                self._assert_invalid(mutate)

    def test_12_blocker_owner_laundering_is_rejected_per_family(self) -> None:
        cases = (
            ("CURRENT_REVISION_CROSS_REVISION_EVIDENCE", 29, "module"),
            ("EXACT_NATIVE_PARTICLE_OUTPUT_ORACLE_REQUIRED", 29, "module"),
            ("FULL_CULL_MODE_UNRESOLVED", 61, "material_cull"),
            ("NO_EXACT_GUID_NATIVE_ENTRY", 28, "material_policy"),
            ("SAMPLER_EVIDENCE_MISSING", 73, "material_policy"),
            ("SOURCE_VALUE_PROVIDER_UNAVAILABLE", 166, "material_policy"),
            ("STATIC_PERMUTATION_SELECTION_UNRESOLVED", 28, "material_policy"),
            ("R3_TYPED_TIMING_EXECUTOR_NOT_COMPLETE", 35, "timing"),
            ("SOURCE_ERA_REQUIRED_DELAY_DEFAULT_UNPROVEN", 25, "timing"),
            ("SOURCE_ERA_REQUIRED_DURATION_DEFAULT_UNPROVEN", 24, "timing"),
            ("R5_SCREEN_POST_RUNTIME_PROBE_NOT_COMPLETE", 1, "screen"),
            ("R5_POINT_LIGHT_RUNTIME_PROBE_NOT_COMPLETE", 1, "light"),
            ("R5_DECAL_RUNTIME_PROBE_NOT_COMPLETE", 3, "decal"),
            ("R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE", 77, "texture"),
        )

        for token, expected_count, owner_kind in cases:
            def mutate(
                program, token=token, expected_count=expected_count,
                owner_kind=owner_kind,
            ):
                changed: set[tuple[str, int]] = set()
                removed = 0

                def remove_from(owner, field):
                    nonlocal removed
                    values = owner[field]
                    if token in values:
                        owner[field] = [value for value in values if value != token]
                        removed += 1
                        return True
                    return False

                if owner_kind == "module":
                    for index, row in enumerate(program["modules"]):
                        if remove_from(row, "preservedBlockers"):
                            changed.add(("modules", index))
                elif owner_kind == "material_cull":
                    for section in ("materialRecipes", "materialOccurrences"):
                        for index, row in enumerate(program[section]):
                            if remove_from(row, "preservedBlockers"):
                                changed.add((section, index))
                elif owner_kind == "material_policy":
                    for index, row in enumerate(program["materialPolicyRows"]):
                        if remove_from(row, "evidenceBlockers"):
                            changed.add(("materialPolicyRows", index))
                elif owner_kind == "texture":
                    for index, row in enumerate(program["materialTextureBindings"]):
                        if remove_from(row, "blockers"):
                            changed.add(("materialTextureBindings", index))
                else:
                    adapter_field = {
                        "screen": "screenPostAdapter",
                        "light": "lightAdapter",
                        "decal": "decalAdapter",
                    }.get(owner_kind)
                    for index, row in enumerate(program["emitters"]):
                        if owner_kind == "timing":
                            owner = row["timing"]
                            sha_field = "timingSha256"
                        else:
                            owner = row[adapter_field]
                            sha_field = "adapterSha256"
                        if owner is not None and remove_from(owner, "blockers"):
                            owner.pop(sha_field)
                            owner[sha_field] = builder.canonical_sha256(owner)
                            changed.add(("emitters", index))

                self.assertEqual(removed, expected_count)
                program["blockerUnion"] = [
                    value for value in program["blockerUnion"] if value != token
                ]
                program["admission"]["blockers"] = [
                    value for value in program["admission"]["blockers"]
                    if value != token
                ]
                sections = {
                    section: program[section] for section in builder.SECTION_NAMES
                }
                program["blockerOwnership"] = (
                    builder.build_blocker_ownership_contract(sections)
                )
                return sorted(changed)

            with self.subTest(token=token):
                self._assert_invalid(mutate)

    def test_13_typedata_mesh_rotation_is_source_joined_not_order_tuned(self) -> None:
        mesh_emitters = [
            row for row in self.program["emitters"]
            if row["rendererRuntimeConfig"]["mesh"] is not None
        ]
        self.assertEqual(len(mesh_emitters), 13)
        non_identity = [
            row for row in mesh_emitters
            if row["rendererRuntimeConfig"]["mesh"]
            ["sourceTypeDataRotationDegrees"] != [0.0, 0.0, 0.0]
        ]
        self.assertEqual(len(non_identity), 1)
        weapon = non_identity[0]
        self.assertEqual(
            weapon["sourceElementId"],
            "fx_pc_sdm_07.par_v_smd_onestroke_weapon_01."
            "particlespriteemitter_6",
        )
        self.assertEqual(
            weapon["rendererRuntimeConfig"]["mesh"],
            {
                "useModelMaterial": False,
                "sourceTypeDataRotationDegrees": [0.0, 0.0, 90.0],
            },
        )

        type_data = next(
            row for row in self.program["modules"]
            if row["emitterId"] == weapon["emitterId"]
            and row["exactSourceClass"] == "particlemoduletypedatamesh"
        )
        rotation_literals = {
            row["propertyPath"]: row
            for row in self.program["literals"]
            if row["moduleId"] == type_data["moduleId"]
            and row["propertyPath"] in {"roll", "pitch", "yaw"}
        }
        self.assertEqual(set(rotation_literals), {"yaw"})
        self.assertEqual(rotation_literals["yaw"]["variant"], "F64")
        self.assertEqual(rotation_literals["yaw"]["f64Value"], 90.0)

        def forge_axis_order(program):
            index = next(
                i for i, row in enumerate(program["emitters"])
                if row["emitterId"] == weapon["emitterId"]
            )
            program["emitters"][index]["rendererRuntimeConfig"]["mesh"][
                "sourceTypeDataRotationDegrees"
            ] = [90.0, 0.0, 0.0]
            return [("emitters", index)]

        self._assert_invalid(forge_axis_order)


if __name__ == "__main__":
    unittest.main()
