#!/usr/bin/env python3
"""Focused contract tests for class-neutral UE3 fixed-input replay."""

from __future__ import annotations

import struct
import sys
import unittest
import copy
from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))
import replay_ue3_material_pixel_shaders as replay


def exact_target(target_id: str) -> dict:
    return {
        "targetId": target_id,
        "cookedPixelShader": {"exactOneDxbcContainer": True},
        "nativeShaderObjectBinding": {
            "status": replay.NATIVE_EXACT,
        },
    }


def signature_payload(rows: list[tuple[str, int, int, int, int, int]]) -> bytes:
    header_size = 8 + len(rows) * 24
    names = bytearray()
    encoded_rows = bytearray()
    for name, semantic_index, system_value, component_type, register, mask in rows:
        name_offset = header_size + len(names)
        names.extend(name.encode("ascii") + b"\0")
        encoded_rows.extend(
            struct.pack(
                "<5I2BH",
                name_offset,
                semantic_index,
                system_value,
                component_type,
                register,
                mask,
                mask,
                0,
            )
        )
    return struct.pack("<II", len(rows), 8) + encoded_rows + names


class ReplayPlanTests(unittest.TestCase):
    def test_zero_fixture(self) -> None:
        self.assertEqual(replay.build_replay_plan([]), [])

    def test_one_fixture(self) -> None:
        rows = replay.build_replay_plan([exact_target("one")])
        self.assertEqual([row["targetId"] for row in rows], ["one"])

    def test_two_fixtures_preserve_manifest_order(self) -> None:
        rows = replay.build_replay_plan(
            [exact_target("first"), exact_target("second")]
        )
        self.assertEqual(
            [row["targetId"] for row in rows], ["first", "second"]
        )

    def test_failure_fixture_rejects_duplicate_identity(self) -> None:
        with self.assertRaisesRegex(ValueError, "duplicated"):
            replay.build_replay_plan(
                [exact_target("same"), exact_target("same")]
            )

    def test_failure_fixture_rejects_missing_exact_binding(self) -> None:
        row = exact_target("bad")
        row["nativeShaderObjectBinding"]["status"] = "BLOCKED"
        with self.assertRaisesRegex(ValueError, "native binding"):
            replay.build_replay_plan([row])


class SignatureTests(unittest.TestCase):
    def test_signature_parser_and_dynamic_carrier(self) -> None:
        payload = signature_payload(
            [
                ("TEXCOORD", 0, 0, 3, 2, 0x3),
                ("SV_Position", 0, 1, 3, 8, 0xF),
                ("SV_IsFrontFace", 0, 9, 1, 9, 0x1),
            ]
        )
        rows = replay.parse_signature(payload)
        self.assertEqual(rows[0]["register"], 2)
        self.assertEqual(rows[1]["systemValueType"], 1)
        source, count = replay.build_carrier_source(rows)
        self.assertEqual(count, 9)
        self.assertIn(b"TEXCOORD0", source)
        self.assertIn(b"SV_Position0", source)
        self.assertNotIn(b"SV_IsFrontFace", source)

    def test_signature_parser_rejects_truncated_row(self) -> None:
        with self.assertRaisesRegex(ValueError, "truncated"):
            replay.parse_signature(struct.pack("<II", 1, 8))

    def test_output_register_count_keeps_mrt_holes(self) -> None:
        rows = [
            {
                "semanticName": "SV_Target",
                "systemValueType": 64,
                "register": register,
            }
            for register in (0, 2, 5)
        ]
        self.assertEqual(replay.output_register_count(rows), 6)

    def test_carrier_signature_closure(self) -> None:
        pixel = [
            {
                "semanticName": "TEXCOORD",
                "semanticIndex": 0,
                "systemValueType": 0,
                "componentType": 3,
                "register": 2,
                "mask": 3,
            },
            {
                "semanticName": "SV_IsFrontFace",
                "semanticIndex": 0,
                "systemValueType": 9,
                "componentType": 1,
                "register": 8,
                "mask": 1,
            },
        ]
        carrier = [
            {
                "semanticName": "TEXCOORD",
                "semanticIndex": 0,
                "componentType": 3,
                "register": 0,
                "mask": 15,
            }
        ]
        closure = replay.close_carrier_signature(pixel, carrier)
        self.assertTrue(closure["pass"])
        self.assertEqual(closure["linkedSemanticCount"], 1)
        self.assertEqual(closure["rasterizerOwnedSystemValueCount"], 1)

    def test_carrier_signature_closure_rejects_missing_semantic(self) -> None:
        pixel = [
            {
                "semanticName": "TEXCOORD",
                "semanticIndex": 7,
                "systemValueType": 0,
                "componentType": 3,
                "register": 0,
                "mask": 3,
            }
        ]
        with self.assertRaisesRegex(ValueError, "absent or ambiguous"):
            replay.close_carrier_signature(pixel, [])

    def test_carrier_signature_closure_rejects_type_and_mask(self) -> None:
        pixel = [
            {
                "semanticName": "TEXCOORD",
                "semanticIndex": 0,
                "systemValueType": 0,
                "componentType": 3,
                "register": 0,
                "mask": 15,
            }
        ]
        producer = {
            "semanticName": "TEXCOORD",
            "semanticIndex": 0,
            "componentType": 1,
            "register": 0,
            "mask": 15,
        }
        with self.assertRaisesRegex(ValueError, "component type"):
            replay.close_carrier_signature(pixel, [producer])
        producer["componentType"] = 3
        producer["mask"] = 3
        with self.assertRaisesRegex(ValueError, "mask"):
            replay.close_carrier_signature(pixel, [producer])


class ClosureTests(unittest.TestCase):
    @staticmethod
    def target_signature(register: int) -> dict:
        return {
            "semanticName": "SV_Target",
            "semanticIndex": register,
            "systemValueType": 0,
            "componentType": 3,
            "register": register,
            "mask": 15,
        }

    def test_mrt_closure_writes_declared_and_preserves_hole(self) -> None:
        outputs = [
            [1.0, 2.0, 3.0, 4.0],
            [replay.SENTINEL] * 4,
            [5.0, 6.0, 7.0, 8.0],
        ]
        closure = replay.validate_mrt_contract(
            outputs, [self.target_signature(0), self.target_signature(2)]
        )
        self.assertTrue(closure["pass"])
        self.assertEqual(closure["sentinelHoleRegisters"], [1])

    def test_mrt_closure_rejects_declared_sentinel(self) -> None:
        with self.assertRaisesRegex(ValueError, "stayed sentinel"):
            replay.validate_mrt_contract(
                [[replay.SENTINEL] * 4], [self.target_signature(0)]
            )

    def test_mrt_closure_rejects_modified_hole(self) -> None:
        outputs = [
            [1.0, 2.0, 3.0, 4.0],
            [0.0, replay.SENTINEL, replay.SENTINEL, replay.SENTINEL],
            [5.0, 6.0, 7.0, 8.0],
        ]
        with self.assertRaisesRegex(ValueError, "hole"):
            replay.validate_mrt_contract(
                outputs, [self.target_signature(0), self.target_signature(2)]
            )

    def test_native_binding_closes_against_declarations(self) -> None:
        declarations = {
            "constantBufferFloat4Counts": {"0": 2},
            "textureRegisters": [0],
            "samplerRegisters": [1],
        }
        native = {
            "dxbcDeclarationClosure": {
                "declaredConstantBuffer0Float4Count": 2,
                "declaredTextureRegisters": [0],
                "declaredSamplerRegisters": [1],
            },
            "constantBufferClosure": {
                "declaredConstantBuffer0Float4Count": 2,
                "boundConstantBuffer0Slots": [1],
            },
            "textureSampleClosure": {
                "materialSamplePairs": ["t0/s1"],
                "unownedEngineSamplePairs": [],
                "allObservedSamplePairCounts": {"t0/s1": 1},
            },
        }
        self.assertTrue(
            replay.validate_native_binding_closure(native, declarations)["pass"]
        )

    def test_native_binding_rejects_undeclared_sample(self) -> None:
        declarations = {
            "constantBufferFloat4Counts": {"0": 2},
            "textureRegisters": [0],
            "samplerRegisters": [0],
        }
        native = {
            "dxbcDeclarationClosure": {
                "declaredConstantBuffer0Float4Count": 2,
                "declaredTextureRegisters": [0],
                "declaredSamplerRegisters": [0],
            },
            "constantBufferClosure": {
                "declaredConstantBuffer0Float4Count": 2,
                "boundConstantBuffer0Slots": [1],
            },
            "textureSampleClosure": {
                "materialSamplePairs": ["t0/s1"],
                "unownedEngineSamplePairs": [],
                "allObservedSamplePairCounts": {"t0/s1": 1},
            },
        }
        with self.assertRaisesRegex(ValueError, "not declared"):
            replay.validate_native_binding_closure(native, declarations)

    def test_native_binding_rejects_cb_extent_mismatch(self) -> None:
        declarations = {
            "constantBufferFloat4Counts": {"0": 3},
            "textureRegisters": [0],
            "samplerRegisters": [0],
        }
        native = {
            "dxbcDeclarationClosure": {
                "declaredConstantBuffer0Float4Count": 2,
                "declaredTextureRegisters": [0],
                "declaredSamplerRegisters": [0],
            },
            "constantBufferClosure": {
                "declaredConstantBuffer0Float4Count": 2,
                "boundConstantBuffer0Slots": [1],
            },
            "textureSampleClosure": {
                "materialSamplePairs": ["t0/s0"],
                "unownedEngineSamplePairs": [],
                "allObservedSamplePairCounts": {"t0/s0": 1},
            },
        }
        with self.assertRaisesRegex(ValueError, "cb0 extent"):
            replay.validate_native_binding_closure(native, declarations)

    def test_output_receipt_rejects_runtime_overclaim(self) -> None:
        document = replay.read_json(replay.DEFAULT_OUTPUT)
        changed = copy.deepcopy(document)
        changed["decision"]["runtimeAdmission"] = True
        replay.seal(changed)
        with self.assertRaisesRegex(ValueError, "overclaims"):
            replay.validate_output_receipt(changed)

    def test_output_receipt_rejects_pass_count_mismatch(self) -> None:
        document = replay.read_json(replay.DEFAULT_OUTPUT)
        changed = copy.deepcopy(document)
        changed["summary"]["dynamicCarrierLinkPassCount"] -= 1
        replay.seal(changed)
        with self.assertRaisesRegex(ValueError, "closure changed"):
            replay.validate_output_receipt(changed)


if __name__ == "__main__":
    unittest.main()
