#!/usr/bin/env python3
"""Focused tests for the Artist F RefShaderCache parser."""

from __future__ import annotations

import copy
import struct
import unittest

from extract_artist_31470_main_ref_shader_cache import (
    parse_uniform_expression,
    parse_uniform_expression_set,
    resolve_constant_buffer_bindings,
    validate_receipt,
)


NAMES = [
    "none",
    "fmaterialuniformexpressionconstant",
    "fmaterialuniformexpressionscalarparameter",
    "fmaterialuniformexpressionvectorparameter",
    "fmaterialuniformexpressionfoldedmath",
    "fmaterialuniformexpressionappendvector",
    "fmaterialuniformexpressionsine",
    "fmaterialuniformexpressiontime",
    "fmaterialuniformexpressiontexture",
    "fmaterialuniformexpressiontextureparameter",
    "scalar",
    "vector",
    "texture",
]


def fname(name: str, number: int = 0) -> bytes:
    return struct.pack("<ii", NAMES.index(name), number)


def constant(value: tuple[float, float, float, float]) -> bytes:
    return fname("fmaterialuniformexpressionconstant") + struct.pack("<4fB", *value, 15)


def scalar(name: str = "scalar", value: float = 2.0) -> bytes:
    return fname("fmaterialuniformexpressionscalarparameter") + fname(name) + struct.pack("<f", value)


class UniformExpressionTests(unittest.TestCase):
    def test_packed_recursive_expression_has_no_alignment_padding(self) -> None:
        payload = (
            fname("fmaterialuniformexpressionappendvector")
            + fname("fmaterialuniformexpressionsine")
            + fname("fmaterialuniformexpressionfoldedmath")
            + constant((3.14, 0.0, 0.0, 0.0))
            + scalar()
            + bytes([2])
            + struct.pack("<I", 1)
            + scalar(value=1.0)
            + struct.pack("<I", 1)
        )
        row = parse_uniform_expression(payload, 0, NAMES)
        self.assertEqual(row["endOffset"], len(payload))
        self.assertEqual(row["componentsFromA"], 1)
        self.assertTrue(row["a"]["isCosine"])
        self.assertEqual(row["a"]["input"]["operationOrdinal"], 2)

    def test_invalid_type_number_is_rejected(self) -> None:
        payload = fname("fmaterialuniformexpressiontime", 1)
        with self.assertRaisesRegex(ValueError, "numbered"):
            parse_uniform_expression(payload, 0, NAMES)

    def test_invalid_constant_value_type_is_rejected(self) -> None:
        payload = fname("fmaterialuniformexpressionconstant") + struct.pack(
            "<4fB", 0.0, 0.0, 0.0, 0.0, 14
        )
        with self.assertRaisesRegex(ValueError, "value type"):
            parse_uniform_expression(payload, 0, NAMES)

    def test_thirteen_array_denominator_is_consumed(self) -> None:
        payload = struct.pack("<I", 1) + scalar()
        payload += struct.pack("<I", 0) * 12
        row = parse_uniform_expression_set(payload, 0, NAMES)
        self.assertEqual(row["endOffset"], len(payload))
        self.assertEqual(len(row["pixelVectorExpressions"]), 1)
        self.assertEqual(row["domainTexture2DExpressions"], [])

    def test_scalar_group_packing_and_unbound_group(self) -> None:
        material_map = {
            "uniformExpressionSet": {
                "pixelVectorExpressions": [{}, {}],
                "pixelScalarExpressions": [{} for _ in range(9)],
            },
            "selectedOriginalDxbc": [
                {
                    "shaderType": "tbasepasspixelshaderfnolightmappolicyskylight",
                    "disassembly": {"declarations": ["dcl_constantbuffer CB0[8], immediateIndexed"]},
                }
            ],
        }
        binding = {
            "vectors": [
                {
                    "expressionIndexOrGroup": 0,
                    "baseIndex": 32,
                    "numBytesOrResources": 16,
                    "bufferIndexOrSamplerIndex": 0,
                },
                {
                    "expressionIndexOrGroup": 1,
                    "baseIndex": 48,
                    "numBytesOrResources": 16,
                    "bufferIndexOrSamplerIndex": 0,
                },
            ],
            "scalarGroups": [
                {
                    "expressionIndexOrGroup": 1,
                    "baseIndex": 112,
                    "numBytesOrResources": 16,
                    "bufferIndexOrSamplerIndex": 0,
                }
            ],
        }
        result = resolve_constant_buffer_bindings(material_map, binding)
        self.assertEqual(result["unboundPackedScalarGroups"], [0, 2])
        self.assertEqual(
            [(row["expressionIndex"], row["constantBufferSlot"], row["component"]) for row in result["scalarBindings"]],
            [(4, 7, "x"), (5, 7, "y"), (6, 7, "z"), (7, 7, "w")],
        )


class ReceiptMutationTests(unittest.TestCase):
    def test_unsealed_mutation_is_rejected(self) -> None:
        receipt = {
            "schema": "lostark.artist-31470-main-ref-shader-cache-receipt",
            "formatVersion": 2,
            "decision": {},
            "receiptSha256": "not-a-digest",
        }
        with self.assertRaisesRegex(ValueError, "digest"):
            validate_receipt(receipt)


if __name__ == "__main__":
    unittest.main()
