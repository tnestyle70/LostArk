#!/usr/bin/env python3
from __future__ import annotations

import struct
import sys
import unittest
from pathlib import Path

TOOL_ROOT = Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

import extract_ue3_material_texture_sampler_closure as closure


def fname(index: int, number: int = 0) -> bytes:
    return struct.pack("<ii", index, number)


class TextureSamplerClosureTests(unittest.TestCase):
    def test_tagged_texture_override_preserves_full_fname_number(self) -> None:
        names = [
            "None",
            "ParameterName",
            "NameProperty",
            "ParameterValue",
            "ObjectProperty",
            "ExpressionGUID",
            "StructProperty",
            "Guid",
            "MainTex",
        ]
        payload = bytearray(struct.pack("<i", 1))
        payload += fname(1) + fname(2) + struct.pack("<ii", 8, 0) + fname(8, 3)
        payload += fname(3) + fname(4) + struct.pack("<ii", 4, 0) + struct.pack("<i", -7)
        payload += (
            fname(5)
            + fname(6)
            + struct.pack("<ii", 16, 0)
            + fname(7)
            + bytes(range(16))
        )
        payload += fname(0)

        rows = closure.parse_tagged_struct_array_raw(bytes(payload), names)

        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["parametername"]["value"], "MainTex")
        self.assertEqual(rows[0]["parametername"]["valueNameNumber"], 3)
        self.assertEqual(rows[0]["parametervalue"]["value"], -7)
        self.assertEqual(
            rows[0]["expressionguid"]["valueGuidHex"], bytes(range(16)).hex()
        )

    def test_uniform_binding_uses_full_fname_override_then_fallback(self) -> None:
        expressions = [
            {
                "typeName": "fmaterialuniformexpressiontextureparameter",
                "parameterName": "MainTex",
                "parameterNameNumber": 0,
                "referencedTextureIndex": 0,
            },
            {
                "typeName": "fmaterialuniformexpressiontextureparameter",
                "parameterName": "MainTex",
                "parameterNameNumber": 2,
                "referencedTextureIndex": 1,
            },
        ]
        wires = [
            {
                "expressionIndexOrGroup": 0,
                "baseIndex": 4,
                "bufferIndexOrSamplerIndex": 1,
            },
            {
                "expressionIndexOrGroup": 1,
                "baseIndex": 7,
                "bufferIndexOrSamplerIndex": 3,
            },
        ]
        references = [
            {"objectPath": "fx_tex.fallback_a"},
            {"objectPath": "fx_tex.fallback_b"},
        ]
        overrides = [
            {
                "index": 0,
                "parameterName": "MainTex",
                "parameterNameNumber": 2,
                "sourceObjectPath": "fx_tex.override_number_2",
            }
        ]

        bindings = closure.resolve_uniform_texture_bindings(
            expressions, wires, references, overrides
        )

        self.assertFalse(bindings[0]["micOverrideApplied"])
        self.assertEqual(bindings[0]["effectiveSourceObjectPath"], "fx_tex.fallback_a")
        self.assertEqual(bindings[0]["textureRegister"], "t4")
        self.assertEqual(bindings[0]["samplerRegister"], "s1")
        self.assertTrue(bindings[1]["micOverrideApplied"])
        self.assertEqual(
            bindings[1]["effectiveSourceObjectPath"], "fx_tex.override_number_2"
        )
        self.assertEqual(bindings[1]["parameterFNameKey"], "maintex#2")

    def test_sampler_projection_keeps_omitted_defaults_blocked(self) -> None:
        fields = {
            name: {"status": "OMITTED_FROM_EXPORT"}
            for name in ("addressx", "addressy", "srgb", "filter", "lodgroup")
        }

        result = closure.sampler_projection(fields)

        self.assertFalse(result["sourceExactSamplerAndColorSpace"])
        self.assertEqual(result["addressU"]["valueCandidate"], "ta_wrap")
        self.assertEqual(result["colorSpace"]["valueCandidate"], "srgb")
        self.assertIn(
            "FILTER_TF_DEFAULT_TEXTURELODSETTINGS_UNRESOLVED", result["blockers"]
        )

    def test_explicit_non_default_sampler_is_source_exact(self) -> None:
        fields = {
            "addressx": {"status": "SERIALIZED_EXPLICIT", "value": "TA_Clamp"},
            "addressy": {"status": "SERIALIZED_EXPLICIT", "value": "TA_Wrap"},
            "srgb": {"status": "SERIALIZED_EXPLICIT", "value": False},
            "filter": {"status": "SERIALIZED_EXPLICIT", "value": "TF_Nearest"},
            "lodgroup": {"status": "OMITTED_FROM_EXPORT"},
        }

        result = closure.sampler_projection(fields)

        self.assertTrue(result["sourceExactSamplerAndColorSpace"])
        self.assertEqual(result["resolvedFilter"], "point")
        self.assertEqual(result["colorSpace"]["value"], "linear")

    def test_tracked_receipt_is_sealed_and_keeps_runtime_visual_closed(self) -> None:
        receipt_path = (
            Path(__file__).resolve().parents[2]
            / "Data/Effects/Imported/DimensionMaster/Materials/"
            "skill.2050120.clip3.exact-texture-sampler-closure.receipt.json"
        )
        receipt = closure.read_json(receipt_path)

        closure.validate_receipt(receipt)
        self.assertEqual(receipt["summary"]["runtimeDdsParityTargetCount"], 5)
        glasshole = next(
            row for row in receipt["targets"]
            if row.get("targetId") == "dimensionmaster-w-glasshole-02"
        )
        self.assertTrue(glasshole["runtimeDdsParityAdmission"])
        self.assertEqual(
            receipt["inputs"]["extractor"]["path"],
            "Tools/EffectPipeline/extract_ue3_material_texture_sampler_closure.py",
        )
        self.assertFalse(receipt["scope"]["authoredGenericResourceSlotsRead"])
        self.assertFalse(receipt["scope"]["runtimeAdmission"])
        self.assertFalse(receipt["scope"]["visualAdmission"])


if __name__ == "__main__":
    unittest.main()
