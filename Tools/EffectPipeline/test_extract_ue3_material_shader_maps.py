#!/usr/bin/env python3
"""Focused tests for the class-neutral UE3 material-map extractor."""

from __future__ import annotations

import copy
import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import extract_ue3_material_shader_maps as subject


def minimal_manifest() -> dict:
    target = {
        "targetId": "fixture",
        "familyId": "fixture.family",
        "rendererType": "SpriteParticle",
        "occurrenceIds": ["fixture.occurrence"],
        "sourcePackageFileName": "fixture.upk",
        "sourceMaterialPath": "fixture.source",
        "parentMaterialPath": "fixture.parent",
        "micObjectPath": "fixture.mic",
        "baseMaterialIdHex": "00112233445566778899aabbccddeeff",
        "staticParameterPolicy": subject.POLICY_REQUIRE_NATIVE,
        "expectedStatus": subject.STATUS_EXACT,
    }
    return {
        "schema": subject.TARGET_SCHEMA,
        "formatVersion": subject.TARGET_FORMAT_VERSION,
        "identity": {"fixture": True},
        "inputs": {
            "officialRefShaderCache": {},
            "sourcePackages": [
                {
                    "fileName": "fixture.upk",
                    "rawSha256": "0" * 64,
                }
            ],
        },
        "vertexFactoryPolicies": {
            "SpriteParticle": {
                "family": "PARTICLE_SPRITE",
                "excludeNameContains": ["beamtrail"],
                "passPixelShaderType": "basepass",
                "actualVfPassAdmission": False,
            }
        },
        "nativeShaderObjectBindingPolicy": {
            "candidateAdmission": "EXACTLY_ONE_STRUCTURAL_CLOSURE",
            "actualVfPassAdmission": False,
            "runtimeAdmission": False,
        },
        "targets": [target],
        "summary": {
            "targetCount": 1,
            "occurrenceCount": 1,
            "expectedExactCount": 1,
            "expectedBlockedCount": 0,
            "expectedExactDxbcCount": 1,
            "expectedExactNativeBindingCount": 1,
        },
    }


def wire_row(key: int, base: int, size: int, buffer_or_sampler: int) -> bytes:
    return struct.pack("<IHHH", key, base, size, buffer_or_sampler)


def wire_array(rows: list[bytes]) -> bytes:
    return struct.pack("<I", len(rows)) + b"".join(rows)


def valid_binding_triple(*, vector_keys: tuple[int, int] = (0, 1)) -> bytes:
    return b"".join(
        (
            wire_array([wire_row(0, 48, 16, 0)]),
            wire_array(
                [
                    wire_row(vector_keys[0], 16, 16, 0),
                    wire_row(vector_keys[1], 32, 16, 0),
                ]
            ),
            wire_array([wire_row(0, 0, 1, 0)]),
        )
    )


def binding_closure(
    *,
    observed: dict[str, int] | None = None,
    textures: list[int] | None = None,
    samplers: list[int] | None = None,
) -> dict:
    return {
        "declaredConstantBuffer0Float4Count": 4,
        "declaredTextureRegisters": [0] if textures is None else textures,
        "declaredSamplerRegisters": [0] if samplers is None else samplers,
        "observedSamplePairCounts": {"t0/s0": 1} if observed is None else observed,
    }


UNIFORM_COUNTS = {
    "pixelScalarExpressions": 4,
    "pixelVectorExpressions": 2,
    "pixelTexture2DExpressions": 1,
}


class BytesRangeReader:
    def __init__(self, payload: bytes) -> None:
        self.payload = payload
        self.logical_size = len(payload)

    def read_logical_range(self, offset: int, size: int) -> bytes:
        result = self.payload[offset : offset + size]
        if len(result) != size:
            raise ValueError("fixture range is truncated")
        return result


def shader_object_table(
    rows: list[tuple[int, bytes, bytes]],
    *,
    platform: int = 4,
) -> tuple[dict, dict]:
    payload = bytearray(struct.pack("<II", platform, len(rows)))
    for type_index, shader_id, body in rows:
        start = len(payload)
        end = start + 48 + len(body)
        payload.extend(struct.pack("<ii", type_index, 0))
        payload.extend(shader_id)
        payload.extend(bytes.fromhex("22" * 20))
        payload.extend(struct.pack("<I", end))
        payload.extend(body)
    payload.extend(struct.pack("<I", 1))
    package = {
        "reader": BytesRangeReader(bytes(payload)),
        "names": ["basepass", "other"],
    }
    layout = {
        "shaderCodeSectionEndLogicalOffset": 0,
        "platform": platform,
        "shaderObjectCount": len(rows),
    }
    return package, layout


class TargetManifestTests(unittest.TestCase):
    def test_accepts_class_neutral_single_target(self) -> None:
        rows = subject.validate_target_manifest(minimal_manifest())
        self.assertEqual(rows[0]["targetId"], "fixture")

    def test_rejects_duplicate_occurrence_assignment(self) -> None:
        document = minimal_manifest()
        duplicate = copy.deepcopy(document["targets"][0])
        duplicate["targetId"] = "fixture-two"
        document["targets"].append(duplicate)
        document["summary"]["targetCount"] = 2
        document["summary"]["expectedExactCount"] = 2
        with self.assertRaisesRegex(ValueError, "multiple targets"):
            subject.validate_target_manifest(document)


class StaticSetPolicyTests(unittest.TestCase):
    def test_absent_tail_is_explicit_blocker_not_parent_fallback(self) -> None:
        result = subject.decode_static_set_from_tail(
            b"",
            bytes.fromhex("00112233445566778899aabbccddeeff"),
            [],
            subject.POLICY_BLOCK_ABSENT,
        )
        self.assertEqual(result["status"], subject.STATUS_BLOCKED)
        self.assertEqual(result["baseMaterialIdOccurrenceCount"], 0)

    def test_required_tail_cannot_silently_fallback(self) -> None:
        with self.assertRaisesRegex(ValueError, "required MIC"):
            subject.decode_static_set_from_tail(
                b"",
                bytes.fromhex("00112233445566778899aabbccddeeff"),
                [],
                subject.POLICY_REQUIRE_NATIVE,
            )

    def test_zero_row_static_set_decodes_by_engine_identity(self) -> None:
        base_id = bytes.fromhex("00112233445566778899aabbccddeeff")
        payload = base_id + b"\x00\x00\x00\x00" * 4
        result = subject.decode_static_set_from_tail(
            payload,
            base_id,
            [],
            subject.POLICY_REQUIRE_NATIVE,
        )
        self.assertEqual(result["status"], "MIC_NATIVE_STATIC_SET_DECODED")
        self.assertEqual(
            result["engineEqualityStaticParameterSet"]["baseMaterialIdHex"],
            base_id.hex(),
        )


class MapSelectionTests(unittest.TestCase):
    def test_selects_one_engine_equality_context(self) -> None:
        scan = {
            "materialMapContexts": [
                {"engineEqualityStaticParameterSetSha256": "a", "logicalOffset": 1},
                {"engineEqualityStaticParameterSetSha256": "b", "logicalOffset": 2},
            ]
        }
        self.assertEqual(subject.select_unique_map_context(scan, "b")["logicalOffset"], 2)

    def test_rejects_ambiguous_engine_equality_context(self) -> None:
        scan = {
            "materialMapContexts": [
                {"engineEqualityStaticParameterSetSha256": "a"},
                {"engineEqualityStaticParameterSetSha256": "a"},
            ]
        }
        with self.assertRaisesRegex(ValueError, "absent or ambiguous"):
            subject.select_unique_map_context(scan, "a")

    def test_multiple_sprite_vfs_can_converge_on_one_pixel_shader(self) -> None:
        material_map = {
            "vertexFactories": [
                {
                    "vertexFactoryType": "fparticledynamicparametervertexfactory",
                    "shaderReferences": [
                        {"shaderType": "basepass", "shaderIdHex": "a" * 32}
                    ],
                },
                {
                    "vertexFactoryType": "fparticlesubuvdynamicparametervertexfactory",
                    "shaderReferences": [
                        {"shaderType": "basepass", "shaderIdHex": "a" * 32}
                    ],
                },
                {
                    "vertexFactoryType": "fparticlebeamtrailvertexfactory",
                    "shaderReferences": [
                        {"shaderType": "basepass", "shaderIdHex": "b" * 32}
                    ],
                },
            ]
        }
        policy = {
            "family": "PARTICLE_SPRITE",
            "excludeNameContains": ["beamtrail"],
            "passPixelShaderType": "basepass",
            "actualVfPassAdmission": False,
        }
        result = subject.select_structural_vf_pass_candidate(
            material_map, "SpriteParticle", policy
        )
        self.assertEqual(result["vertexFactoryCandidateCount"], 2)
        self.assertEqual(result["uniquePixelPassReferenceCount"], 1)
        self.assertEqual(
            result["selectedPixelPassReference"]["shaderIdHex"], "a" * 32
        )
        self.assertFalse(result["actualVfPassAdmission"])

    def test_local_mesh_policy_selects_only_local_vf(self) -> None:
        material_map = {
            "vertexFactories": [
                {
                    "vertexFactoryType": "flocalvertexfactory",
                    "shaderReferences": [
                        {"shaderType": "basepass", "shaderIdHex": "c" * 32}
                    ],
                },
                {
                    "vertexFactoryType": "fgpuskinvertexfactory",
                    "shaderReferences": [
                        {"shaderType": "basepass", "shaderIdHex": "c" * 32}
                    ],
                },
            ]
        }
        policy = {
            "family": "LOCAL_MESH",
            "vertexFactoryType": "flocalvertexfactory",
            "passPixelShaderType": "basepass",
            "actualVfPassAdmission": False,
        }
        result = subject.select_structural_vf_pass_candidate(
            material_map, "MeshParticle", policy
        )
        self.assertEqual(result["vertexFactoryCandidateCount"], 1)
        self.assertEqual(
            result["selectedPixelPassReference"]["shaderIdHex"], "c" * 32
        )

    def test_artist_packed_descriptor_primitive_decodes_dword_slice(self) -> None:
        shader_id = bytes.fromhex("11" * 16)
        word0 = 1 | (3 << 18)
        word1 = 5 << 18
        descriptor = shader_id + struct.pack("<II", word0, word1)
        positions = [
            {"uncompressedByteSize": 16},
            {"uncompressedByteSize": 64},
        ]
        result = subject.decode_packed_shader_code_slice(descriptor, positions)
        self.assertEqual(result["codeBlobIndex"], 1)
        self.assertEqual(result["sliceOffsetInUncompressedBlob"], 12)
        self.assertEqual(result["sliceByteSize"], 20)


class UniformExpressionTests(unittest.TestCase):
    def test_periodic_wraps_one_expression_without_artist_mutation(self) -> None:
        names = [
            "fmaterialuniformexpressionperiodic",
            "fmaterialuniformexpressionscalarparameter",
            "speed",
        ]
        payload = (
            struct.pack("<ii", 0, 0)
            + struct.pack("<ii", 1, 0)
            + struct.pack("<ii", 2, 0)
            + struct.pack("<f", 1.25)
        )
        expression = subject.parse_uniform_expression(payload, 0, names)
        self.assertEqual(
            expression["typeName"],
            "fmaterialuniformexpressionperiodic",
        )
        self.assertEqual(expression["input"]["parameterName"], "speed")
        self.assertEqual(expression["endOffset"], len(payload))

    def test_numbered_parameter_fname_is_preserved(self) -> None:
        names = [
            "fmaterialuniformexpressionvectorparameter",
            "color",
        ]
        payload = (
            struct.pack("<ii", 0, 0)
            + struct.pack("<ii", 1, 3)
            + struct.pack("<4f", 1.0, 2.0, 3.0, 4.0)
        )
        expression = subject.parse_uniform_expression(payload, 0, names)
        self.assertEqual(expression["parameterName"], "color")
        self.assertEqual(expression["parameterNameNumber"], 3)


class NativeShaderObjectBindingTests(unittest.TestCase):
    def test_shader_object_zero_table_member_fails_closed(self) -> None:
        wanted = bytes.fromhex("11" * 16)
        package, layout = shader_object_table(
            [(0, bytes.fromhex("33" * 16), b"\x00" * 70)]
        )
        with self.assertRaisesRegex(ValueError, "denominator changed"):
            subject.extract_selected_shader_objects(
                package,
                layout,
                [{"shaderType": "basepass", "shaderIdHex": wanted.hex()}],
            )

    def test_shader_object_one_table_member_ignores_embedded_pattern(self) -> None:
        wanted = bytes.fromhex("11" * 16)
        embedded_pattern = struct.pack("<ii", 0, 0) + wanted
        package, layout = shader_object_table(
            [
                (0, wanted, b"\x00" * 70),
                (1, bytes.fromhex("33" * 16), embedded_pattern + b"\x00" * 50),
            ]
        )
        result = subject.extract_selected_shader_objects(
            package,
            layout,
            [{"shaderType": "basepass", "shaderIdHex": wanted.hex()}],
        )
        self.assertEqual(result["byShaderId"][wanted.hex()]["shaderObjectIndex"], 0)
        self.assertEqual(result["table"]["shaderObjectCount"], 2)

    def test_shader_object_two_table_members_with_same_identity_fail_closed(self) -> None:
        wanted = bytes.fromhex("11" * 16)
        package, layout = shader_object_table(
            [(0, wanted, b"\x00" * 70), (0, wanted, b"\x00" * 80)]
        )
        with self.assertRaisesRegex(ValueError, "duplicated"):
            subject.extract_selected_shader_objects(
                package,
                layout,
                [{"shaderType": "basepass", "shaderIdHex": wanted.hex()}],
            )

    def test_zero_structural_candidate_fails_closed(self) -> None:
        payload = b"\x00" * 73 + valid_binding_triple(vector_keys=(0, 0))
        candidates = subject.scan_native_binding_array_candidates(
            payload, 1000, UNIFORM_COUNTS, binding_closure()
        )
        self.assertEqual(candidates, [])
        with self.assertRaisesRegex(ValueError, "absent or ambiguous: 0"):
            subject.select_unique_native_binding_arrays(
                payload, 1000, UNIFORM_COUNTS, binding_closure()
            )

    def test_one_candidate_is_discovered_without_fixed_188_offset(self) -> None:
        payload = b"\x00" * 73 + valid_binding_triple() + b"\x00" * 19
        selected = subject.select_unique_native_binding_arrays(
            payload, 1000, UNIFORM_COUNTS, binding_closure()
        )
        self.assertEqual(selected["candidateCount"], 1)
        self.assertEqual(selected["bindingArraysOffsetInShaderObject"], 73)
        self.assertEqual(
            selected["constantBufferClosure"][
                "declaredConstantBuffer0Float4Count"
            ],
            4,
        )

    def test_two_candidates_are_rejected_as_ambiguous(self) -> None:
        first = b"\x00" * 73 + valid_binding_triple()
        payload = first + b"\x00" * (211 - len(first)) + valid_binding_triple()
        candidates = subject.scan_native_binding_array_candidates(
            payload, 2000, UNIFORM_COUNTS, binding_closure()
        )
        self.assertEqual(
            [row["bindingArraysOffsetInShaderObject"] for row in candidates],
            [73, 211],
        )
        with self.assertRaisesRegex(ValueError, "absent or ambiguous: 2"):
            subject.select_unique_native_binding_arrays(
                payload, 2000, UNIFORM_COUNTS, binding_closure()
            )

    def test_disjoint_engine_sample_pair_is_preserved(self) -> None:
        payload = b"\x00" * 91 + valid_binding_triple()
        closure = binding_closure(
            observed={"t0/s0": 1, "t1/s1": 2},
            textures=[0, 1],
            samplers=[0, 1],
        )
        selected = subject.select_unique_native_binding_arrays(
            payload, 3000, UNIFORM_COUNTS, closure
        )
        self.assertEqual(
            selected["textureSampleClosure"]["unownedEngineSamplePairs"],
            ["t1/s1"],
        )

    def test_engine_pair_conflicting_with_material_register_is_rejected(self) -> None:
        payload = b"\x00" * 91 + valid_binding_triple()
        closure = binding_closure(
            observed={"t0/s0": 1, "t0/s1": 1},
            textures=[0],
            samplers=[0, 1],
        )
        self.assertEqual(
            subject.scan_native_binding_array_candidates(
                payload, 3000, UNIFORM_COUNTS, closure
            ),
            [],
        )


if __name__ == "__main__":
    unittest.main()
