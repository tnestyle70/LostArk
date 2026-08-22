#!/usr/bin/env python3
"""Fail-closed tests for named ABI and bounded offline parity evidence."""

from __future__ import annotations

import copy
import hashlib
import io
import json
import struct
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest import mock

from Tools.EffectPipeline import build_effect_family_named_abi as builder
from Tools.EffectPipeline import verify_effect_family_named_abi_evidence as evidence
from Tools.EffectPipeline import verify_effect_family_time_varying_parity as parity


def raw_sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def attach_artifact(document: dict) -> dict:
    result = copy.deepcopy(document)
    result.pop("artifactSha256", None)
    result["artifactSha256"] = evidence.canonical_sha256(result)
    return result


def write_json_lf(path: Path, document: dict) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(json.dumps(document, indent=2, ensure_ascii=False) + "\n")


def identity(path: Path) -> dict:
    payload = path.read_bytes()
    return {"rawSha256": raw_sha256(payload), "byteSize": len(payload)}


def pin(inputs: dict, prefix: str, value: dict) -> None:
    inputs[f"{prefix}RawSha256"] = value["rawSha256"]
    inputs[f"{prefix}ByteSize"] = value["byteSize"]


def wire_array(rows: list[tuple[int, int, int, int]]) -> bytes:
    return struct.pack("<I", len(rows)) + b"".join(
        struct.pack("<IHHH", *row) for row in rows
    )


class EvidenceFixture:
    def __init__(self, root: Path) -> None:
        self.root = root
        self.shader_map_path = root / "shader-map.json"
        self.cooked_path = root / "cooked.json"
        self.named_path = root / "named.json"
        self.parity_path = root / "parity.json"
        self.cooked_directory = root / "CookedShaders"
        self.authored_directory = root / "Authored"
        self.cache_path = root / "cache.upk"
        self.compiler_path = root / "d3dcompiler_47.dll"
        self.cooked_directory.mkdir()
        self.authored_directory.mkdir()
        self.cache_path.write_bytes(b"pinned-cache")
        self.compiler_path.write_bytes(b"pinned-compiler")
        self.parents = [evidence.CANONICAL_PARENT, "fx.test.second"]
        self.dxbc: dict[str, tuple[str, bytes]] = {}
        for index, parent in enumerate(self.parents):
            payload = b"DXBC" + index.to_bytes(4, "little")
            digest = raw_sha256(payload)
            self.dxbc[parent] = (digest, payload)
            (self.cooked_directory / f"{digest}.dxbc").write_bytes(payload)
        self.write_upstream()
        self.write_named()
        self.write_authored()
        self.write_parity()

    @staticmethod
    def counts(vector_count: int, texture_count: int = 0) -> dict:
        return {
            "pixelVectorExpressions": vector_count,
            "pixelScalarExpressions": 0,
            "pixelTexture2DExpressions": texture_count,
            "textureCubeExpressions": 0,
            "vertexVectorExpressions": 0,
            "vertexScalarExpressions": 0,
            "vertexTexture2DExpressions": 0,
            "hullVectorExpressions": 0,
            "hullScalarExpressions": 0,
            "hullTexture2DExpressions": 0,
            "domainVectorExpressions": 0,
            "domainScalarExpressions": 0,
            "domainTexture2DExpressions": 0,
        }

    def write_upstream(self) -> None:
        shader_map = attach_artifact({
            "schema": evidence.SHADER_MAP_SCHEMA,
            "formatVersion": evidence.FORMAT_VERSION,
            "families": [
                {"parentMaterialPath": parent} for parent in self.parents
            ],
        })
        write_json_lf(self.shader_map_path, shader_map)
        families = []
        for index, parent in enumerate(self.parents):
            digest, payload = self.dxbc[parent]
            families.append({
                "parentMaterialPath": parent,
                "status": "EXTRACTED",
                "carrier": "mesh",
                "childMaterialPath": f"fx.child.{index}",
                "dxbcSha256": digest,
                "dxbcByteSize": len(payload),
                "uniformExpressionCounts": self.counts(
                    1 if index == 0 else 0,
                    1 if index == 0 else 0),
            })
        cooked = attach_artifact({
            "schema": evidence.COOKED_SCHEMA,
            "formatVersion": evidence.FORMAT_VERSION,
            "families": families,
        })
        write_json_lf(self.cooked_path, cooked)

    def mapping_row(self, parent: str, timed: bool) -> dict:
        digest, _ = self.dxbc[parent]
        vector_lanes = []
        if timed:
            vector_lanes = [{
                "expressionIndex": 0,
                "constantRegister": "cb0[1]",
                "constantRow": 1,
                "parameterNames": ["pan_speed"],
                "expression": "foldedmath(scalarparameter:pan_speed, time)",
                "timeDependent": True,
            }]
        texture_slots = []
        native_vectors = []
        native_textures = []
        if timed:
            native_vectors = [{
                "expressionIndexOrGroup": 0,
                "baseIndex": 16,
                "numBytesOrResources": 16,
                "bufferIndexOrSamplerIndex": 0,
            }]
            native_textures = [{
                "expressionIndexOrGroup": 0,
                "baseIndex": 0,
                "numBytesOrResources": 1,
                "bufferIndexOrSamplerIndex": 0,
            }]
            texture_slots = [{
                "expressionIndex": 0,
                "textureRegister": "t0",
                "samplerRegister": "s0",
                "parameterName": "diff_tex",
                "referencedTextureIndex": 0,
                "isParameter": True,
            }]
        closure = {
            "declaredConstantBuffer0Float4Count": 2,
            "minimumNativeBoundConstantBuffer0Slot": 1 if timed else None,
            "maximumNativeBoundConstantBuffer0Slot": 1 if timed else None,
            "boundConstantBuffer0Slots": [1] if timed else [],
            "unownedConstantBuffer0Slots": [0] if timed else [0, 1],
            "leadingUnownedConstantBuffer0Slots": [0] if timed else [0, 1],
            "trailingUnownedConstantBuffer0Slots": [],
            "unownedConstantBuffer0SlotPolicy":
                "PRESERVE_ENGINE_OR_PASS_OWNED_PREFIX_AND_SUFFIX_ROWS",
            "scalarUniformExpressionGroupDenominator": 0,
            "nativeScalarWireCount": 0,
            "scalarExpressionGroupsWithoutNativeWire": [],
        }
        texture_closure = {
            "materialSamplePairs": ["t0/s0"] if timed else [],
            "unownedEngineSamplePairs": [],
            "allObservedSamplePairCounts": {"t0/s0": 1} if timed else {},
        }
        semantic = {
            "scalarGroups": [],
            "vectors": native_vectors,
            "textures": native_textures,
            "constantBufferClosure": closure,
            "textureSampleClosure": texture_closure,
        }
        return {
            "parentMaterialPath": parent,
            "status": evidence.RESOLVED,
            "friendlyName": parent.rsplit(".", 1)[-1],
            "carrier": "mesh",
            "childMaterialPath": (
                "fx.child.0" if parent == self.parents[0] else "fx.child.1"),
            "dxbcSha256": digest,
            "instructionCount": 1,
            "declaredConstantBuffer0Float4Count": 2,
            "uniformExpressionCounts": self.counts(
                1 if timed else 0, 1 if timed else 0),
            "nativeBindingWire": {
                "selectionMode": (
                    evidence.STRICT_BINDING_SELECTION
                    if timed else evidence.LENIENT_BINDING_SELECTION),
                "offsetInShaderObject": 64,
                "byteSize": 12,
                "rawSha256": "1" * 64,
                "bindingSemanticSha256": evidence.canonical_sha256(semantic),
                **semantic,
            },
            "scalarLanes": [],
            "vectorLanes": vector_lanes,
            "textureSlots": texture_slots,
            "summary": {
                "scalarLaneCount": 0,
                "vectorLaneCount": len(vector_lanes),
                "textureSlotCount": len(texture_slots),
                "namedScalarLaneCount": 0,
                "namedVectorLaneCount": len(vector_lanes),
                "timeDependentScalarLaneCount": 0,
                "timeDependentVectorLaneCount": len(vector_lanes),
                "timeDependentRegisters": (
                    ["cb0[1]"] if timed else []),
            },
            "admits": evidence.NAMED_ADMISSION,
        }

    def named_document(self) -> dict:
        shader_map = json.loads(self.shader_map_path.read_text(encoding="utf-8"))
        cooked = json.loads(self.cooked_path.read_text(encoding="utf-8"))
        inputs = {
            "shaderMapArtifactSha256": shader_map["artifactSha256"],
            "cookedPixelShadersArtifactSha256": cooked["artifactSha256"],
            "refShaderCacheFileName": self.cache_path.name,
            "d3dCompilerFileName": self.compiler_path.name,
            "cookedShaderProgramCount": 2,
        }
        pin(inputs, "shaderMap", identity(self.shader_map_path))
        pin(inputs, "cookedPixelShaders", identity(self.cooked_path))
        pin(inputs, "refShaderCache", identity(self.cache_path))
        pin(inputs, "d3dCompiler", identity(self.compiler_path))
        programs = {
            digest: identity(self.cooked_directory / f"{digest}.dxbc")
            for digest, _ in self.dxbc.values()
        }
        inputs["cookedShaderSetSha256"] = evidence.canonical_sha256(programs)
        return {
            "schema": evidence.NAMED_ABI_SCHEMA,
            "formatVersion": evidence.FORMAT_VERSION,
            "identity": {"admits": evidence.NAMED_ADMISSION},
            "inputs": inputs,
            "summary": {
                "familyCount": 2,
                "resolvedNamedMappingCount": 2,
                "blockedCount": 0,
                "bindingSelectionCounts": {
                    evidence.LENIENT_BINDING_SELECTION: 1,
                    evidence.STRICT_BINDING_SELECTION: 1,
                },
                "bindingOutcomeCounts": {
                    f"{evidence.STRICT_BINDING_SELECTION}:{evidence.RESOLVED}": 1,
                    f"{evidence.LENIENT_BINDING_SELECTION}:{evidence.RESOLVED}": 1,
                },
                "blockerCounts": {},
                "blockedParents": [],
                "blockedParentSetSha256": evidence.canonical_sha256([]),
                "lenientResolvedParents": [self.parents[1]],
                "lenientResolvedParentSetSha256":
                    evidence.canonical_sha256([self.parents[1]]),
            },
            "families": [
                self.mapping_row(self.parents[0], True),
                self.mapping_row(self.parents[1], False),
            ],
        }

    def write_named(self, document: dict | None = None) -> None:
        write_json_lf(
            self.named_path,
            attach_artifact(document if document is not None
                            else self.named_document()),
        )

    def write_authored(self) -> None:
        document = {
            "elements": [{
                "id": evidence.CANONICAL_ELEMENT_ID,
                "material": {
                    "sourceMaterialPath": "fx.child.0",
                    "renderProfile": "alpha_two_sided_depth_read",
                    "sourceProfile": {
                        "semanticStatus": "reconstructed_profile",
                        "parentMaterialPath": evidence.CANONICAL_PARENT,
                    },
                },
            }]
        }
        write_json_lf(
            self.authored_directory
            / f"{evidence.CANONICAL_EFFECT_ASSET_ID}.effect.json",
            document,
        )

    def parity_document(self) -> dict:
        shader_map = json.loads(self.shader_map_path.read_text(encoding="utf-8"))
        cooked = json.loads(self.cooked_path.read_text(encoding="utf-8"))
        named = json.loads(self.named_path.read_text(encoding="utf-8"))
        authored_path = (
            self.authored_directory
            / f"{evidence.CANONICAL_EFFECT_ASSET_ID}.effect.json"
        )
        digest, _ = self.dxbc[evidence.CANONICAL_PARENT]
        dxbc_path = self.cooked_directory / f"{digest}.dxbc"
        inputs = {
            "shaderMapArtifactSha256": shader_map["artifactSha256"],
            "cookedPixelShadersArtifactSha256": cooked["artifactSha256"],
            "namedAbiArtifactSha256": named["artifactSha256"],
            "authoredDocument":
                "Data/Effects/Authored/"
                f"{evidence.CANONICAL_EFFECT_ASSET_ID}.effect.json",
            "refShaderCacheFileName": self.cache_path.name,
            "d3dCompilerFileName": self.compiler_path.name,
            "cookedDxbcFileName": dxbc_path.name,
        }
        for prefix, path in (
            ("shaderMap", self.shader_map_path),
            ("cookedPixelShaders", self.cooked_path),
            ("namedAbi", self.named_path),
            ("authoredDocument", authored_path),
            ("cookedDxbc", dxbc_path),
            ("refShaderCache", self.cache_path),
            ("d3dCompiler", self.compiler_path),
        ):
            pin(inputs, prefix, identity(path))
        samples = [{
            "gameTimeSeconds": game_time,
            "worstRelativeDelta": 0.0,
            "renderTarget0": [0.37, 0.61, 0.5, 0.0],
        } for game_time in evidence.CANONICAL_TIMES]
        return {
            "schema": evidence.PARITY_SCHEMA,
            "formatVersion": evidence.FORMAT_VERSION,
            "identity": {
                "admits": evidence.PARITY_ADMISSION,
                "proves": [
                    "DXBC_AND_REGENERATED_TRANSLATION_AGREE_AT_RECONSTRUCTED_CB0",
                    "OFFLINE_EVALUATOR_PRODUCES_TIME_VARYING_CB0_ROWS",
                ],
                "doesNotProve": sorted(evidence.REQUIRED_NON_PROOFS),
            },
            "inputs": inputs,
            "occurrence": {
                "parentMaterialPath": evidence.CANONICAL_PARENT,
                "effectAssetId": evidence.CANONICAL_EFFECT_ASSET_ID,
                "elementId": evidence.CANONICAL_ELEMENT_ID,
                "sourceMaterialPath": "fx.child.0",
                "sourceProfileSemanticStatus": "reconstructed_profile",
                "renderProfile": "alpha_two_sided_depth_read",
                "dxbcSha256": digest,
                "instructionCount": 1,
                "gameTimesSampled": evidence.CANONICAL_TIMES,
                "worstParityDelta": 0.0,
                "constantMotionAcrossTime": 1.0,
                "movingConstantRows": ["cb0[1]"],
                "expectedTimeDependentRows": ["cb0[1]"],
                "outputMotionAcrossTime": 0.0,
                "outputMotionCaveat":
                    "REPLAY_BINDS_1X1_TEXTURES_SO_UV_PANNING_CANNOT_SHOW",
                "boundConstantRows": ["cb0[1]"],
                "fixtureBoundary": {
                    "translationSource":
                        "REGENERATED_FROM_CURRENT_TRANSLATOR_NOT_CHECKED_HLSLI",
                    "carrierInputs": "SYNTHETIC_CONSTANT_FLOAT4_ROWS",
                    "engineConstantBuffers": "SYNTHETIC_CONSTANT_FLOAT4_ROWS",
                    "textures": "SYNTHETIC_CONSTANT_1X1_RGBA",
                    "runtimeRendererExecuted": False,
                },
                "samples": samples,
                "parityResult": evidence.PARITY_PASS,
                "motionResult": evidence.MOTION_PASS,
                "admits": evidence.PARITY_ADMISSION,
            },
        }

    def write_parity(self, document: dict | None = None) -> None:
        write_json_lf(
            self.parity_path,
            attach_artifact(document if document is not None
                            else self.parity_document()),
        )

    def verify(self):
        with (
            mock.patch.object(evidence, "EXPECTED_EXTRACTED_FAMILY_COUNT", 2),
            mock.patch.object(evidence, "EXPECTED_UNIQUE_PROGRAM_COUNT", 2),
            mock.patch.object(
                evidence, "EXPECTED_STRICT_NON_EMPTY_FAMILY_COUNT", 1),
            mock.patch.object(
                evidence, "EXPECTED_LENIENT_EMPTY_RESOURCE_FAMILY_COUNT", 1),
            mock.patch.object(
                evidence, "EXPECTED_STRICT_RESOLVED_COUNT", 1),
            mock.patch.object(
                evidence, "EXPECTED_LENIENT_RESOLVED_COUNT", 1),
            mock.patch.object(
                evidence, "EXPECTED_LENIENT_BLOCKED_COUNT", 0),
            mock.patch.object(evidence, "EXPECTED_BLOCKED_PARENTS", ()),
            mock.patch.object(
                evidence, "EXPECTED_BLOCKED_PARENT_SET_SHA256",
                evidence.canonical_sha256([])),
            mock.patch.object(
                evidence, "EXPECTED_LENIENT_RESOLVED_PARENTS",
                (self.parents[1],)),
            mock.patch.object(
                evidence, "EXPECTED_LENIENT_RESOLVED_PARENT_SET_SHA256",
                evidence.canonical_sha256([self.parents[1]])),
        ):
            return evidence.verify(
                self.shader_map_path,
                self.cooked_path,
                self.named_path,
                self.parity_path,
                self.cooked_directory,
                self.authored_directory,
                self.cache_path,
                self.compiler_path,
            )


class BuilderBoundaryTests(unittest.TestCase):
    def test_texture_free_native_abi_is_a_legal_unique_candidate(self) -> None:
        payload = b"\xff" * 73 + b"".join((
            wire_array([(0, 16, 16, 0)]),
            wire_array([(0, 32, 16, 0)]),
            wire_array([]),
        )) + b"\xff" * 19
        counts = {
            "pixelScalarExpressions": 1,
            "pixelVectorExpressions": 1,
            "pixelTexture2DExpressions": 0,
        }
        closure = {
            "declaredConstantBuffer0Float4Count": 3,
            "declaredTextureRegisters": [],
            "declaredSamplerRegisters": [],
            "observedSamplePairCounts": {},
        }
        result = builder.select_named_abi_binding_arrays(
            payload, 1000, counts, closure)
        self.assertEqual(result["textures"], [])
        self.assertEqual(
            result["constantBufferClosure"]["boundConstantBuffer0Slots"],
            [1, 2],
        )

    def test_lenient_ambiguity_exposes_structured_candidate_count(self) -> None:
        triple = b"".join((
            wire_array([(0, 16, 16, 0)]),
            wire_array([(0, 32, 16, 0)]),
            wire_array([]),
        ))
        first = b"\xff" * 73 + triple
        payload = first + b"\xff" * (211 - len(first)) + triple
        counts = {
            "pixelScalarExpressions": 1,
            "pixelVectorExpressions": 1,
            "pixelTexture2DExpressions": 0,
        }
        closure = {
            "declaredConstantBuffer0Float4Count": 3,
            "declaredTextureRegisters": [],
            "declaredSamplerRegisters": [],
            "observedSamplePairCounts": {},
        }
        with self.assertRaises(builder.BindingCandidateError) as captured:
            builder.select_named_abi_binding_arrays(
                payload, 1000, counts, closure)
        self.assertEqual(captured.exception.candidate_count, 2)
        self.assertEqual(
            captured.exception.reason_code,
            builder.AMBIGUOUS_CANDIDATE_REASON,
        )

    def test_builder_has_no_broad_exception_to_blocked_conversion(self) -> None:
        source = Path(builder.__file__).read_text(encoding="utf-8")
        self.assertNotIn("except Exception", source)

    def test_texture_free_dxbc_declaration_is_legal(self) -> None:
        result = builder.parse_named_abi_dxbc_closure({
            "profile": "ps_5_0",
            "declarations": [
                "dcl_constantbuffer CB0[3], immediateIndexed"
            ],
            "instructions": ["mov o0.xyzw, cb0[1].xyzw"],
            "normalizedDisassemblySha256": "0" * 64,
            "declarationSha256": "1" * 64,
            "instructionSha256": "2" * 64,
            "instructionCount": 1,
        })
        self.assertEqual(result["observedSamplePairCounts"], {})

    def test_non_empty_sampled_family_keeps_strict_selector(self) -> None:
        disassembly = {
            "instructions": ["sample r0.xyzw, r0.xyxx, t0.xyzw, s0"],
        }
        counts = {
            "pixelScalarExpressions": 0,
            "pixelVectorExpressions": 1,
            "pixelTexture2DExpressions": 1,
        }
        shader_object = {"_bytes": b"shader", "logicalOffset": 10}
        strict_closure = {"strict": True}
        strict_wire = {"wire": True}
        with (
            mock.patch.object(
                builder, "parse_dxbc_declaration_closure",
                return_value=strict_closure) as parse_strict,
            mock.patch.object(
                builder, "select_unique_native_binding_arrays",
                return_value=strict_wire) as select_strict,
            mock.patch.object(
                builder, "parse_named_abi_dxbc_closure") as parse_lenient,
            mock.patch.object(
                builder, "select_named_abi_binding_arrays") as select_lenient,
        ):
            closure, wire, mode = builder.resolve_named_abi_binding(
                disassembly, shader_object, counts)
        self.assertEqual(mode, builder.STRICT_BINDING_SELECTION)
        self.assertIs(closure, strict_closure)
        self.assertIs(wire, strict_wire)
        parse_strict.assert_called_once()
        select_strict.assert_called_once()
        parse_lenient.assert_not_called()
        select_lenient.assert_not_called()

    def test_partial_generation_cannot_overwrite_canonical_receipt(self) -> None:
        with self.assertRaisesRegex(builder.NamedAbiError, "partial"):
            builder.main(["--only", "fx.test"])


class ParityBoundaryTests(unittest.TestCase):
    def test_compare_rejects_nan_inf_and_shape_mismatch(self) -> None:
        with self.assertRaisesRegex(parity.ParityError, "NaN/Inf"):
            parity.compare([[float("nan")]], [[float("nan")]])
        with self.assertRaisesRegex(parity.ParityError, "shape"):
            parity.compare([[0.0]], [[0.0, 1.0]])

    def test_failed_parity_preserves_existing_receipt(self) -> None:
        with tempfile.TemporaryDirectory(prefix="parity-preserve-") as temporary:
            output = Path(temporary) / "parity.json"
            output.write_bytes(b"existing-receipt")
            row = {
                "parityResult": parity.PARITY_FAIL,
                "motionResult": parity.MOTION_PASS,
                "worstParityDelta": 1.0,
                "movingConstantRows": ["cb0[1]"],
            }
            with (
                mock.patch.object(parity, "DEFAULT_OUTPUT", output),
                mock.patch.object(parity, "verify", return_value=(row, {})),
                redirect_stdout(io.StringIO()),
                redirect_stderr(io.StringIO()),
            ):
                status = parity.main([
                    "--parent", parity.CANONICAL_PARENT,
                    "--effect-asset-id", parity.CANONICAL_EFFECT_ASSET_ID,
                    "--element-id", parity.CANONICAL_ELEMENT_ID,
                ])
            self.assertEqual(status, 1)
            self.assertEqual(output.read_bytes(), b"existing-receipt")

    def test_noncanonical_request_preserves_canonical_receipt(self) -> None:
        with tempfile.TemporaryDirectory(prefix="parity-canonical-") as temporary:
            output = Path(temporary) / "parity.json"
            output.write_bytes(b"canonical")
            with (
                mock.patch.object(parity, "DEFAULT_OUTPUT", output),
                mock.patch.object(parity, "verify") as verify,
                redirect_stdout(io.StringIO()),
                redirect_stderr(io.StringIO()),
            ):
                status = parity.main([
                    "--parent", "fx.other",
                    "--effect-asset-id", parity.CANONICAL_EFFECT_ASSET_ID,
                    "--element-id", parity.CANONICAL_ELEMENT_ID,
                ])
            self.assertEqual(status, 1)
            verify.assert_not_called()
            self.assertEqual(output.read_bytes(), b"canonical")


class EvidenceVerifierTests(unittest.TestCase):
    def use_fixture(self) -> EvidenceFixture:
        temporary = tempfile.TemporaryDirectory(prefix="named-abi-evidence-")
        self.addCleanup(temporary.cleanup)
        return EvidenceFixture(Path(temporary.name))

    def test_accepts_exact_bounded_evidence(self) -> None:
        fixture = self.use_fixture()
        result = fixture.verify()
        self.assertEqual(result["resolvedNamedMappingCount"], 2)

    def test_rejects_schema_even_with_fresh_artifact_hash(self) -> None:
        fixture = self.use_fixture()
        document = json.loads(fixture.named_path.read_text(encoding="utf-8"))
        document["schema"] = "lostark.wrong"
        write_json_lf(fixture.named_path, attach_artifact(document))
        with self.assertRaisesRegex(evidence.VerificationError, "schema"):
            fixture.verify()

    def test_rejects_artifact_hash_drift(self) -> None:
        fixture = self.use_fixture()
        document = json.loads(fixture.named_path.read_text(encoding="utf-8"))
        document["summary"]["familyCount"] += 1
        write_json_lf(fixture.named_path, document)
        with self.assertRaisesRegex(evidence.VerificationError, "artifactSha256"):
            fixture.verify()

    def test_rejects_crlf_even_when_json_and_artifact_are_valid(self) -> None:
        fixture = self.use_fixture()
        fixture.named_path.write_bytes(
            fixture.named_path.read_bytes().replace(b"\n", b"\r\n"))
        with self.assertRaisesRegex(evidence.VerificationError, "LF-only"):
            fixture.verify()

    def test_rejects_missing_and_extra_named_denominator(self) -> None:
        for mutation, message in (
            ("missing", "denominator differs"),
            ("extra", "outside cooked denominator"),
        ):
            with self.subTest(mutation=mutation):
                fixture = self.use_fixture()
                document = fixture.named_document()
                if mutation == "missing":
                    document["families"].pop()
                    document["summary"]["familyCount"] = 1
                    document["summary"]["resolvedNamedMappingCount"] = 1
                else:
                    extra = copy.deepcopy(document["families"][1])
                    extra["parentMaterialPath"] = "fx.test.extra"
                    document["families"].append(extra)
                    document["summary"]["familyCount"] = 3
                    document["summary"]["resolvedNamedMappingCount"] = 3
                fixture.write_named(document)
                with self.assertRaisesRegex(evidence.VerificationError, message):
                    fixture.verify()

    def test_rejects_duplicate_family(self) -> None:
        fixture = self.use_fixture()
        document = fixture.named_document()
        document["families"][1] = copy.deepcopy(document["families"][0])
        fixture.write_named(document)
        with self.assertRaisesRegex(evidence.VerificationError, "duplicates family"):
            fixture.verify()

    def test_rejects_missing_explicit_resolved_status(self) -> None:
        fixture = self.use_fixture()
        document = fixture.named_document()
        document["families"][0].pop("status")
        fixture.write_named(document)
        with self.assertRaisesRegex(
            evidence.VerificationError, "explicit valid status"
        ):
            fixture.verify()

    def test_rejects_unstructured_or_non_ambiguous_blocker(self) -> None:
        fixture = self.use_fixture()
        document = fixture.named_document()
        document["families"][1] = {
            "parentMaterialPath": fixture.parents[1],
            "status": evidence.BLOCKED,
            "blocker": "native binding-array candidate is ambiguous: 2",
            "bindingSelectionMode": evidence.LENIENT_BINDING_SELECTION,
        }
        fixture.write_named(document)
        with self.assertRaisesRegex(evidence.VerificationError, "structured"):
            fixture.verify()

        fixture = self.use_fixture()
        document = fixture.named_document()
        document["families"][1] = {
            "parentMaterialPath": fixture.parents[1],
            "status": evidence.BLOCKED,
            "blocker": {
                "reasonCode": evidence.AMBIGUOUS_CANDIDATE_REASON,
                "candidateCount": 1,
            },
            "bindingSelectionMode": evidence.LENIENT_BINDING_SELECTION,
        }
        fixture.write_named(document)
        with self.assertRaisesRegex(evidence.VerificationError, "exceed 1"):
            fixture.verify()

    def test_rejects_input_pin_and_dxbc_hash_drift(self) -> None:
        fixture = self.use_fixture()
        document = fixture.named_document()
        document["inputs"]["refShaderCacheRawSha256"] = "0" * 64
        fixture.write_named(document)
        with self.assertRaisesRegex(
            evidence.VerificationError, "cache.*SHA|RefShaderCache.*SHA"
        ):
            fixture.verify()

        fixture = self.use_fixture()
        digest, _ = fixture.dxbc[fixture.parents[1]]
        (fixture.cooked_directory / f"{digest}.dxbc").write_bytes(b"drift")
        with self.assertRaisesRegex(evidence.VerificationError, "DXBC raw SHA"):
            fixture.verify()

    def test_rejects_lane_summary_drift(self) -> None:
        fixture = self.use_fixture()
        document = fixture.named_document()
        document["families"][0]["summary"]["vectorLaneCount"] = 0
        fixture.write_named(document)
        with self.assertRaisesRegex(evidence.VerificationError, "vectorLaneCount"):
            fixture.verify()

    def test_rejects_lane_deletion_cb_row_move_and_ts_rewire(self) -> None:
        fixture = self.use_fixture()
        document = fixture.named_document()
        document["families"][0]["vectorLanes"] = []
        fixture.write_named(document)
        with self.assertRaisesRegex(evidence.VerificationError, "vector lanes"):
            fixture.verify()

        fixture = self.use_fixture()
        document = fixture.named_document()
        lane = document["families"][0]["vectorLanes"][0]
        lane["constantRow"] = 0
        lane["constantRegister"] = "cb0[0]"
        fixture.write_named(document)
        with self.assertRaisesRegex(evidence.VerificationError, "moved"):
            fixture.verify()

        fixture = self.use_fixture()
        document = fixture.named_document()
        document["families"][0]["textureSlots"][0][
            "textureRegister"] = "t1"
        fixture.write_named(document)
        with self.assertRaisesRegex(evidence.VerificationError, "t/s wiring"):
            fixture.verify()

    def test_rejects_old_source_exact_label_and_nonfinite_delta(self) -> None:
        fixture = self.use_fixture()
        document = fixture.parity_document()
        document["occurrence"]["parityResult"] = "SOURCE_VALUE_PARITY"
        fixture.write_parity(document)
        with self.assertRaisesRegex(evidence.VerificationError, "success status"):
            fixture.verify()

        fixture = self.use_fixture()
        document = fixture.parity_document()
        document["occurrence"]["worstParityDelta"] = float("nan")
        fixture.write_parity(document)
        with self.assertRaisesRegex(evidence.VerificationError, "finite"):
            fixture.verify()

    def test_rejects_incomplete_nonproof_boundary(self) -> None:
        fixture = self.use_fixture()
        document = fixture.parity_document()
        document["identity"]["doesNotProve"].pop()
        fixture.write_parity(document)
        with self.assertRaisesRegex(evidence.VerificationError, "non-proof"):
            fixture.verify()


if __name__ == "__main__":
    unittest.main()
