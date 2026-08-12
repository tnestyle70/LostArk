#!/usr/bin/env python3

from __future__ import annotations

import copy
import struct
import unittest

from extract_artist_31470_native_base_textures import (
    find_native_lookup,
    parse_dds,
)


def dds_fixture(*, fourcc: bytes = b"DXT1", payload_size: int = 8) -> bytes:
    raw = bytearray(128 + payload_size)
    raw[:4] = b"DDS "
    struct.pack_into("<I", raw, 4, 124)
    struct.pack_into("<6I", raw, 8, 0x00081007, 4, 4, payload_size, 0, 0)
    struct.pack_into("<2I", raw, 76, 32, 4)
    raw[84:88] = fourcc
    struct.pack_into("<5I", raw, 108, 0x1000, 0, 0, 0, 0)
    return bytes(raw)


def native_fixture() -> dict:
    return {
        "families": [
            {
                "familyId": "family-a",
                "source": {
                    "legacyTextureLookups": [
                        {
                            "index": 0,
                            "texCoordIndex": 0,
                            "textureIndex": 0,
                            "textureObjectPath": "tex.base",
                            "uScale": 1.0,
                            "vScale": 1.0,
                        }
                    ]
                },
            }
        ],
        "textureClosureDiagnostics": {
            "diagnostics": [
                {
                    "logicalTexturePath": "tex.base",
                    "status": "SOURCE_EXACT_BASE_DEFAULT_REPLACED_BY_ACTIVE_MIC",
                    "legacyTextureLookupResolutions": [
                        {
                            "parameterNames": ["map"],
                            "effectiveOverrideValues": ["tex.override"],
                            "baseDefaultOverriddenInAllActiveRecipes": True,
                            "overridePreservesBaseLogicalTexture": False,
                            "effectiveOverrideValuesRuntimeBound": True,
                            "activeRecipes": [
                                {
                                    "recipeId": "r",
                                    "sourceMaterialPath": "mi.r",
                                    "parameterNames": ["map"],
                                    "overrideValues": ["tex.override"],
                                    "baseDefaultEffective": False,
                                }
                            ],
                        }
                    ],
                }
            ]
        },
    }


class NativeBaseTextureTests(unittest.TestCase):
    def test_dds_parser_accepts_exact_top_mip_shape(self) -> None:
        parsed = parse_dds(dds_fixture(), "fixture")
        self.assertEqual(parsed["width"], 4)
        self.assertEqual(parsed["height"], 4)
        self.assertEqual(parsed["fourCC"], "DXT1")
        self.assertEqual(parsed["payloadByteCount"], 8)

    def test_dds_parser_rejects_format_and_payload_drift(self) -> None:
        with self.assertRaisesRegex(ValueError, "unsupported DDS FourCC"):
            parse_dds(dds_fixture(fourcc=b"DX10"), "fixture")
        with self.assertRaisesRegex(ValueError, "payload size mismatch"):
            parse_dds(dds_fixture(payload_size=16), "fixture")
        with self.assertRaisesRegex(ValueError, "truncated"):
            parse_dds(dds_fixture()[:64], "fixture")

    def test_native_lookup_join_requires_different_bound_mic_override(self) -> None:
        expected = {
            "logicalTexturePath": "tex.base",
            "familyId": "family-a",
            "parameterName": "map",
            "effectiveOverrideValue": "tex.override",
        }
        joined = find_native_lookup(native_fixture(), expected)
        self.assertFalse(joined["baseDefaultEffectiveForActiveArtist31470"])
        self.assertEqual(joined["effectiveOverrideValue"], "tex.override")

        changed = native_fixture()
        changed["textureClosureDiagnostics"]["diagnostics"][0][
            "legacyTextureLookupResolutions"
        ][0]["overridePreservesBaseLogicalTexture"] = True
        with self.assertRaisesRegex(ValueError, "MIC override precedence changed"):
            find_native_lookup(changed, expected)


if __name__ == "__main__":
    unittest.main()
