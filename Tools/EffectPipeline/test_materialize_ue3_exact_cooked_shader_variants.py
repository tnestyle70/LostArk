#!/usr/bin/env python3

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


TOOL_ROOT = Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

import materialize_ue3_exact_cooked_shader_variants as target  # noqa: E402


def fixture(renderer_type: str = "SpriteParticle") -> tuple[dict, dict, dict, dict, bytes]:
    bytecode = b"DXBC-fixture"
    digest = target.sha256_bytes(bytecode)
    family_id = "ue3.material.fixture.family"
    source_material = "fixture_mic.fixture_material"
    shader_type = "tbasepasspixelshaderfnolightmappolicyskylight"
    shader_id = "0123456789abcdef0123456789abcdef"
    vf_types = ["fparticledynamicparametervertexfactory"]
    if renderer_type == "MeshParticle":
        vf_types = ["flocalvertexfactory"]
    exact = {
        "familyId": family_id,
        "rendererType": renderer_type,
        "sourceMaterialPath": source_material,
        "parentMaterialPath": "fixture_parent.fixture_material",
        "baseMaterialIdHex": "00112233445566778899aabbccddeeff",
        "mic": {
            "engineEqualityStaticParameterSetSha256": "a" * 64,
            "engineEqualityStaticParameterSet": {
                "baseMaterialIdHex": "00112233445566778899aabbccddeeff",
                "staticSwitchParameters": [],
                "staticComponentMaskParameters": [],
                "terrainLayerWeightParameters": [],
            },
        },
        "materialMap": {"trailerPlatformOrdinal": 4},
        "structuralVfPassCandidate": {
            "rendererType": renderer_type,
            "policy": {"selectionFidelity": "STRUCTURAL_ONLY"},
            "selectedPixelPassReference": {
                "shaderType": shader_type,
                "shaderIdHex": shader_id,
                "vertexFactoryTypes": vf_types,
            },
            "actualVfPassAdmission": False,
            "admissionBlockers": ["NATIVE_EMITTER_VERTEX_FACTORY_ABI_UNPROVEN"],
        },
        "cookedPixelShader": {
            "shaderType": shader_type,
            "shaderIdHex": shader_id,
            "dxbc": {"byteSize": len(bytecode), "sha256": digest},
        },
        "nativeShaderObjectBinding": {
            "status": target.NATIVE_STATUS,
            "shaderObject": {"shaderIdHex": shader_id},
            "bindingSemanticSha256": "b" * 64,
            "scalarGroups": [],
            "vectors": [],
            "textures": [
                {
                    "expressionIndexOrGroup": 0,
                    "baseIndex": 0,
                    "numBytesOrResources": 1,
                    "bufferIndexOrSamplerIndex": 0,
                }
            ],
            "constantBufferClosure": {
                "declaredConstantBuffer0Float4Count": 3,
                "boundConstantBuffer0Slots": [1, 2],
            },
            "textureSampleClosure": {
                "materialSamplePairs": ["t0/s0"],
                "unownedEngineSamplePairs": [],
            },
            "dxbcDeclarationClosure": {"profile": "ps_5_0"},
            "wireEntryFormat": "fixture",
        },
    }
    replay = {
        "pixelShaderCreation": "RAW_EXACT_DXBC_TO_ID3D11DEVICE_CREATEPIXELSHADER",
        "carrierVertexShader": {
            "sourceSha256": "c" * 64,
            "compiledDxbcSha256": "d" * 64,
            "linkageContract": "SIGNATURE_COMPATIBLE",
            "inputSignature": [{"semanticName": "TEXCOORD", "semanticIndex": 0}],
            "outputSignature": [{"semanticName": "TEXCOORD", "semanticIndex": 0}],
            "signatureClosure": {"pass": True},
        },
        "outputSignature": [{"semanticName": "SV_Target", "register": 0}],
        "renderTargetCount": 1,
        "runtimeDeclarations": {"constantBufferFloat4Counts": {"0": 3}},
        "nativeBinding": {"pass": True},
        "baseline": {
            "rt0Nonzero": True,
            "mrtContract": {"declaredTargets": [{"register": 0}], "pass": True},
        },
        "externalOpacityInputSensitivity": {
            "mutation": {"register": 0, "row": 0, "lane": 0},
            "pass": True,
        },
        "structuralFixedInputReplayAdmission": True,
        "sourceValueReplayAdmission": False,
        "actualVfPassAdmission": False,
        "runtimeAdmission": False,
        "visualAdmission": False,
    }
    all_rows = [
        {
            "slot": 0,
            "ownership": "ENGINE_OR_RENDERER_INPUT_UNBOUND_AT_G03_5",
            "value": None,
        },
        {
            "slot": 1,
            "ownership": "MATERIAL_UNIFORM_EXPRESSION",
            "value": [1.0, 2.0, 3.0, 4.0],
        },
        {
            "slot": 2,
            "ownership": "MATERIAL_UNIFORM_EXPRESSION",
            "value": [5.0, 6.0, 7.0, 8.0],
        },
    ]
    if renderer_type == "MeshParticle":
        all_rows[1] = {
            "slot": 1,
            "ownership": "ENGINE_OR_RENDERER_INPUT_UNBOUND_AT_G03_5",
            "value": None,
        }
        all_rows.append(
            {
                "slot": 3,
                "ownership": "MATERIAL_UNIFORM_EXPRESSION",
                "value": [9.0, 10.0, 11.0, 12.0],
            }
        )
        exact["nativeShaderObjectBinding"]["constantBufferClosure"][
            "declaredConstantBuffer0Float4Count"
        ] = 4
    material_rows = [
        {
            "slot": row["slot"],
            "source": "PIXEL_VECTOR_EXPRESSION",
            "expressionIndex": index,
            "value": row["value"],
        }
        for index, row in enumerate(all_rows)
        if row["ownership"] == "MATERIAL_UNIFORM_EXPRESSION"
    ]
    uniform = {
        "status": target.UNIFORM_STATUS,
        "uniformExpressionCounts": {"pixelVectorExpressions": len(material_rows)},
        "sourceValueUniformCb0ClosureAdmission": True,
        "sourceValueUniformEvaluation": {
            "evaluationContext": {"gameTimeSeconds": 0.0, "realTimeSeconds": 0.0},
            "pixelVectorValuesSemanticSha256": "e" * 64,
            "pixelScalarValuesSemanticSha256": "f" * 64,
            "evaluationStats": {"expressionCount": len(material_rows)},
            "nativeCb0": {
                "declaredFloat4Count": len(all_rows),
                "materialBoundFloat4Count": len(material_rows),
                "engineOrRendererUnboundFloat4Count": len(all_rows) - len(material_rows),
                "materialRows": material_rows,
                "allRows": all_rows,
                "materialRowsSemanticSha256": "1" * 64,
                "nativeScalarGroupPackingEvidence": {
                    "fidelity": "EPIC_PACKED_SCALAR_ABI_CORROBORATED_BUT_TARGET_PADDING_UNRESOLVED",
                    "nativeScalarGroupPackingSourceClosed": False,
                    "sourceExactAdmission": False,
                    "targetPaddingFreeCorroborationAdmission": False,
                    "targetPaddingFree": False,
                    "selectedGroupCount": 1,
                    "completeGroupCount": 0,
                    "partialGroupCount": 1,
                    "packedLittleEndianSha256": None,
                },
            },
        },
    }
    texture = {
        "status": target.TEXTURE_STATUS,
        "sourceMaterialPath": source_material,
        "uniformTextureBindings": [
            {
                "uniformExpressionIndex": 0,
                "expressionType": "fmaterialuniformexpressiontextureparameter",
                "parameterFNameKey": "maintex#0",
                "effectiveSourceObjectPath": "fx_tex.fixture",
                "textureRegister": "t0",
                "samplerRegister": "s0",
                "bindingFidelity": "SOURCE_EXACT",
                "sourceTexture2D": {
                    "samplerAndColorSpace": {
                        "addressU": {
                            "value": "ta_clamp",
                            "sourceExact": True,
                            "fidelity": "SOURCE_EXACT_SERIALIZED_PROPERTY",
                        },
                        "addressV": {
                            "value": None,
                            "sourceExact": False,
                            "valueCandidate": "ta_wrap",
                            "candidateFidelity": "EXPLICIT_FIXTURE_CANDIDATE",
                        },
                        "filterSelector": {
                            "value": "tf_linear",
                            "sourceExact": True,
                            "fidelity": "SOURCE_EXACT_V975_CDO_SERIALIZED_PROPERTY",
                        },
                        "hardwareFilter": {
                            "value": None,
                            "sourceExact": False,
                            "valueCandidate": "linear",
                            "candidateFidelity": "EXPLICIT_FIXTURE_CANDIDATE",
                        },
                        "colorSpace": {
                            "value": "srgb",
                            "sourceExact": True,
                            "fidelity": "SOURCE_EXACT_V975_CDO_SERIALIZED_PROPERTY",
                        },
                        "sourceExactColorSpace": True,
                        "sourceExactFilterSelector": True,
                        "sourceExactAddressAxisCount": 1,
                        "sourceExactHardwareFilter": False,
                        "sourceExactSamplerAndColorSpace": False,
                    }
                },
                "ddsIdentity": {
                    "sourceExactDds": {"byteSize": 128, "sha256": "2" * 64},
                    "runtimeDimensionMaster": {
                        "relativePath": "Effect/Fixture/fixture.dds",
                        "status": "PRESENT",
                        "byteSize": 128,
                        "sha256": "2" * 64,
                        "sourceExactParity": True,
                    },
                },
            }
        ],
        "sourceExactTextureBindingAdmission": True,
        "runtimeDdsParityAdmission": True,
        "sourceExactSamplerAdmission": False,
        "sourceValueTextureSamplerAdmission": False,
        "sourceExactColorSpaceBindingCount": 1,
        "sourceExactFilterSelectorBindingCount": 1,
        "sourceExactAddressAxisCount": 1,
        "sourceExactAddressAxisDenominator": 2,
        "sourceExactHardwareFilterBindingCount": 0,
        "blockers": ["FILTER_TF_DEFAULT_TEXTURELODSETTINGS_UNRESOLVED"],
    }
    return exact, replay, uniform, texture, bytecode


def add_vertex_sidecar(exact: dict, bytecode: bytes) -> None:
    digest = target.sha256_bytes(bytecode)
    exact["sourceEmitterVertexFactoryPass"] = {
        "occurrenceId": "fixture-occurrence-never-projected",
        "sourceRecipe": {
            "rendererShape": "sprite",
            "requiredModule": {
                "stableId": "fixture-required-never-projected",
                "className": "particlemodulerequired",
                "boffsetcenter": True,
                "screenalignment": "psa_rectangle",
            },
            "dynamicModule": {
                "stableId": "fixture-dynamic-never-projected",
                "className": "particlemoduleparameterdynamic",
                "updateflags": 15,
            },
        },
        "sourceEmitterVertexFactorySelection": {
            "selectionRule": "PARTICLE_REQUIRED_BOFFSETCENTER_PLUS_DYNAMIC_PARAMETER_MODULE",
            "vertexFactoryType": "fparticleoffsetcenterdynamicparametervertexfactory",
            "rejectedCompetingVertexFactoryTypes": [
                "fparticledynamicparametervertexfactory"
            ],
        },
        "authoringVertexPassSelection": {
            "densityPolicy": "NO_DENSITY_AUTHORING_BOUNDED",
            "vertexShaderType": "tbasepassvertexshaderfnolightmappolicyfnodensitypolicy",
            "vertexShaderIdHex": "fedcba9876543210fedcba9876543210",
        },
        "exactVertexShader": {
            "shaderType": "tbasepassvertexshaderfnolightmappolicyfnodensitypolicy",
            "shaderIdHex": "fedcba9876543210fedcba9876543210",
            "dxbc": {"byteSize": len(bytecode), "sha256": digest},
        },
        "vertexShaderDeclarationClosure": {
            "profile": "vs_5_0",
            "constantBufferFloat4Counts": {"0": 27, "1": 5},
        },
        "vertexShaderInputSignature": [{"semanticName": "POSITION", "semanticIndex": 0}],
        "vertexShaderOutputSignature": [{"semanticName": "TEXCOORD", "semanticIndex": 0}],
        "selectedPixelShaderInputSignature": [{"semanticName": "TEXCOORD", "semanticIndex": 0}],
        "vertexPixelSignatureClosure": {"linkedSemanticCount": 1, "pass": True},
        "admission": {
            "sourceEmitterVertexFactorySelection": True,
            "exactVertexShaderBlob": True,
            "exactVertexPixelSignatureClosure": True,
            "authoringNoDensityPass": True,
            "rawVertexShaderExecution": False,
            "sourceExactVertexCb": False,
            "productVfPass": False,
            "runtime": False,
            "visual": False,
        },
    }


class MaterializeExactCookedVariantsTests(unittest.TestCase):
    def test_variant_key_is_family_permutation_scoped_and_sorts_vfs(self) -> None:
        exact, _, _, _, _ = fixture()
        selected = exact["structuralVfPassCandidate"]["selectedPixelPassReference"]
        selected["vertexFactoryTypes"] = ["vf_z", "vf_a"]
        key = target.variant_key(exact)
        self.assertEqual(key["structuralVertexFactoryCandidateTypes"], ["vf_a", "vf_z"])
        self.assertFalse(target.FORBIDDEN_SELECTOR_KEYS.intersection(key))

    def test_sampler_candidate_is_explicitly_approximate(self) -> None:
        _, _, _, texture, _ = fixture()
        sampler = target.preview_sampler(texture["uniformTextureBindings"][0])
        self.assertEqual(sampler["fidelity"], target.PREVIEW_FIDELITY)
        self.assertEqual(sampler["addressU"], "clamp")
        self.assertEqual(sampler["addressV"], "wrap")
        self.assertEqual(sampler["hardwareFilter"], "linear")
        self.assertFalse(sampler["sourceExact"])
        self.assertEqual(
            sampler["components"]["addressV"]["provenance"],
            "EXPLICIT_UPSTREAM_PREVIEW_CANDIDATE",
        )

    def test_unresolved_sampler_does_not_infer_preview_candidate(self) -> None:
        _, _, _, texture, _ = fixture()
        binding = texture["uniformTextureBindings"][0]
        sampler = binding["sourceTexture2D"]["samplerAndColorSpace"]
        sampler["addressV"].pop("valueCandidate")
        sampler["hardwareFilter"].pop("valueCandidate")
        self.assertIsNone(target.preview_sampler(binding))
        projected = target.slim_texture_binding(binding)
        partial = projected["sourceExactSamplerPartialEvidence"]
        self.assertTrue(partial["sourceExactColorSpace"])
        self.assertTrue(partial["sourceExactFilterSelector"])
        self.assertFalse(partial["sourceExactHardwareFilter"])
        self.assertFalse(partial["fullSamplerSourceExact"])

    def test_sprite_engine_row_policy_only_supplies_external_opacity(self) -> None:
        _, _, uniform, _, _ = fixture()
        cb0 = uniform["sourceValueUniformEvaluation"]["nativeCb0"]
        policy = target.engine_row_policy("SpriteParticle", cb0)
        self.assertEqual(policy["sourceUnboundSlots"], [0])
        self.assertEqual(policy["authoringPreviewCandidates"][0]["semantic"], "externalOpacity")
        self.assertEqual(policy["authoringPreviewCandidates"][0]["candidateValue"][0], 1.0)
        self.assertEqual(policy["unresolvedSlots"], [])

    def test_mesh_engine_row_policy_adds_rgba_candidate_without_admission(self) -> None:
        _, _, uniform, _, _ = fixture("MeshParticle")
        cb0 = uniform["sourceValueUniformEvaluation"]["nativeCb0"]
        policy = target.engine_row_policy("MeshParticle", cb0)
        self.assertEqual(policy["sourceUnboundSlots"], [0, 1])
        self.assertEqual(
            [row["semantic"] for row in policy["authoringPreviewCandidates"]],
            ["externalOpacity", "particleOrRendererRgba"],
        )
        self.assertFalse(policy["sourceExact"])

    def test_scalar_group_preview_row_keeps_native_packing_open(self) -> None:
        _, _, uniform, _, _ = fixture()
        cb0 = uniform["sourceValueUniformEvaluation"]["nativeCb0"]
        cb0["materialRows"][0]["source"] = "PIXEL_SCALAR_EXPRESSION_GROUP"
        rows = target.preview_cb0_rows(
            cb0, target.engine_row_policy("SpriteParticle", cb0)
        )
        scalar_row = next(
            row
            for row in rows
            if row.get("source") == "PIXEL_SCALAR_EXPRESSION_GROUP"
        )
        self.assertEqual(
            scalar_row["fidelity"],
            "PROJECT_PREVIEW_APPROXIMATE_SCALAR_GROUP_PACKING_UNPROVEN",
        )
        self.assertNotIn("SOURCE_EXACT", scalar_row["fidelity"])

    def test_padding_free_scalar_rows_project_corroborated_lane_order_only(self) -> None:
        _, _, uniform, _, _ = fixture()
        cb0 = uniform["sourceValueUniformEvaluation"]["nativeCb0"]
        cb0["materialRows"][0]["source"] = "PIXEL_SCALAR_EXPRESSION_GROUP"
        packing = cb0["nativeScalarGroupPackingEvidence"]
        packing.update(
            {
                "fidelity": "EPIC_PACKED_SCALAR_ABI_CORROBORATED_AND_TARGET_PADDING_FREE",
                "targetPaddingFreeCorroborationAdmission": True,
                "targetPaddingFree": True,
                "completeGroupCount": 1,
                "partialGroupCount": 0,
                "packedLittleEndianSha256": "3" * 64,
            }
        )
        rows = target.preview_cb0_rows(
            cb0, target.engine_row_policy("SpriteParticle", cb0)
        )
        scalar_row = next(
            row for row in rows
            if row.get("source") == "PIXEL_SCALAR_EXPRESSION_GROUP"
        )
        self.assertEqual(
            scalar_row["fidelity"], target.TARGET_PADDING_FREE_SCALAR_FIDELITY
        )
        projection = target.scalar_packing_projection(cb0)
        self.assertTrue(projection["targetPaddingFreeCorroborationAdmission"])
        self.assertFalse(projection["nativeScalarGroupPackingSourceClosed"])
        self.assertFalse(projection["sourceExactAdmission"])

    def test_one_rt0_variant_is_candidate_but_never_admitted(self) -> None:
        exact, replay, uniform, texture, bytecode = fixture()
        variant = target.build_variant(exact, replay, uniform, texture, bytecode)
        admission = variant["admission"]
        self.assertEqual(variant["sourceMaterialPath"], exact["sourceMaterialPath"])
        self.assertTrue(admission["authoringPreviewCandidate"])
        self.assertFalse(admission["authoringPreviewAdmission"])
        self.assertFalse(admission["productRuntime"])
        self.assertFalse(admission["actualVfPass"])
        self.assertFalse(admission["sourceExactSampler"])
        self.assertFalse(admission["sourceExactNativeScalarGroupPacking"])
        self.assertEqual(
            variant["authoringPreviewInputs"]["packingFidelity"]["blocker"],
            target.SCALAR_PACKING_BLOCKER,
        )

    def test_mrt_variant_is_not_a_preview_candidate(self) -> None:
        exact, replay, uniform, texture, bytecode = fixture()
        replay["outputSignature"] = [
            {"semanticName": "SV_Target", "register": 0},
            {"semanticName": "SV_Target", "register": 2},
        ]
        replay["renderTargetCount"] = 3
        variant = target.build_variant(exact, replay, uniform, texture, bytecode)
        self.assertFalse(variant["admission"]["authoringPreviewCandidate"])

    def test_exact_vertex_sidecar_keeps_execution_and_product_closed(self) -> None:
        exact, replay, uniform, texture, pixel_bytecode = fixture()
        vertex_bytecode = b"DXBC-vertex-fixture"
        add_vertex_sidecar(exact, vertex_bytecode)
        variant = target.build_variant(
            exact, replay, uniform, texture, pixel_bytecode, vertex_bytecode
        )
        evidence = variant["sourceEmitterVertexFactoryPassEvidence"]
        self.assertEqual(
            evidence["sourceEmitterVertexFactorySelection"]["vertexFactoryType"],
            "fparticleoffsetcenterdynamicparametervertexfactory",
        )
        self.assertEqual(
            evidence["authoringVertexPassSelection"]["densityPolicy"],
            "NO_DENSITY_AUTHORING_BOUNDED",
        )
        self.assertEqual(
            evidence["vertexShader"]["sha256"],
            target.sha256_bytes(vertex_bytecode),
        )
        self.assertTrue(evidence["vertexPixelSignatureClosure"]["pass"])
        self.assertFalse(evidence["admission"]["rawVertexShaderExecution"])
        self.assertFalse(evidence["admission"]["sourceExactVertexCb"])
        self.assertFalse(evidence["admission"]["productVfPass"])
        self.assertNotIn("occurrenceId", set(target.iter_keys(variant)))

    def test_contract_validation_rejects_selector_keys(self) -> None:
        exact, replay, uniform, texture, bytecode = fixture()
        variant = target.build_variant(exact, replay, uniform, texture, bytecode)
        contract = {
            "schema": target.SCHEMA,
            "formatVersion": target.FORMAT_VERSION,
            "variants": [variant],
            "skillId": 1,
        }
        contract["contractSha256"] = target.canonical_json_sha256(contract)
        with self.assertRaisesRegex(ValueError, "selector keys leaked"):
            target.validate_contract(
                contract, {target.sha256_bytes(bytecode): bytecode}
            )

    def test_contract_validation_accepts_sealed_source_contract(self) -> None:
        exact, replay, uniform, texture, bytecode = fixture()
        variant = target.build_variant(exact, replay, uniform, texture, bytecode)
        contract = {
            "schema": target.SCHEMA,
            "formatVersion": target.FORMAT_VERSION,
            "variants": [variant],
        }
        contract["contractSha256"] = target.canonical_json_sha256(contract)
        target.validate_contract(contract, {target.sha256_bytes(bytecode): bytecode})

    def test_tracked_contract_projects_glass_g03_6_evidence_fail_closed(self) -> None:
        contract = target.read_json(target.DEFAULT_OUTPUT)
        blobs = {}
        for variant in contract["variants"]:
            shaders = [variant["pixelShader"]]
            vertex_pass = variant["sourceEmitterVertexFactoryPassEvidence"]
            if vertex_pass is not None:
                shaders.append(vertex_pass["vertexShader"])
            for shader in shaders:
                blobs[shader["sha256"]] = (
                    target.REPOSITORY_ROOT / shader["repositoryRelativePath"]
                ).read_bytes()
        target.validate_contract(contract, blobs)
        summary = contract["summary"]
        self.assertEqual(summary["variantCount"], 5)
        self.assertEqual(summary["uniquePixelShaderBlobCount"], 5)
        self.assertEqual(summary["exactVertexShaderSidecarCount"], 1)
        self.assertEqual(summary["targetPaddingFreeScalarCorroborationCount"], 1)
        self.assertEqual(summary["sourceExactSamplerAdmissionCount"], 0)
        self.assertEqual(summary["productRuntimeAdmissionCount"], 0)
        glass = next(
            variant for variant in contract["variants"]
            if "glasshole.02" in variant["familyId"]
        )
        packing = glass["sourceValueUniformExpressions"][
            "nativeScalarGroupPackingEvidence"
        ]
        self.assertTrue(packing["targetPaddingFreeCorroborationAdmission"])
        self.assertFalse(packing["nativeScalarGroupPackingSourceClosed"])
        self.assertFalse(packing["sourceExactAdmission"])
        self.assertEqual(
            packing["packedLittleEndianSha256"],
            "23e40670db53319d0c8a013b9f63866c4ed6aa16afdb4837db3a9c0c242e356d",
        )
        scalar_rows = [
            row for row in glass["authoringPreviewInputs"]["timeZeroCb0Rows"]
            if row.get("source") == "PIXEL_SCALAR_EXPRESSION_GROUP"
        ]
        self.assertEqual(len(scalar_rows), 8)
        self.assertTrue(
            all(
                row["fidelity"] == target.TARGET_PADDING_FREE_SCALAR_FIDELITY
                for row in scalar_rows
            )
        )
        self.assertEqual(glass["authoringPreviewInputs"]["samplers"], [])
        self.assertTrue(glass["admission"]["sourceExactColorSpace"])
        self.assertTrue(glass["admission"]["sourceExactFilterSelector"])
        self.assertFalse(glass["admission"]["sourceExactSampler"])
        vertex_pass = glass["sourceEmitterVertexFactoryPassEvidence"]
        self.assertEqual(vertex_pass["vertexShader"]["byteCount"], 6500)
        self.assertEqual(
            vertex_pass["vertexShader"]["sha256"],
            "defae822f429760d30e0bfd31cef8c1217af8d7e43f794be6b42e85e6552995b",
        )
        self.assertEqual(
            vertex_pass["vertexShaderDeclarationClosure"][
                "constantBufferFloat4Counts"
            ],
            {"0": 27, "1": 5},
        )
        self.assertEqual(
            vertex_pass["vertexPixelSignatureClosure"]["linkedSemanticCount"], 8
        )
        self.assertFalse(vertex_pass["admission"]["rawVertexShaderExecution"])
        self.assertFalse(vertex_pass["admission"]["productVfPass"])
        self.assertFalse(glass["admission"]["productRuntime"])
        self.assertFalse(glass["admission"]["visual"])


if __name__ == "__main__":
    unittest.main()
