#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import struct
import unittest
from pathlib import Path

from extract_artist_31470_material_native_resource import (
    DEFAULT_MATERIAL_CONTRACT,
    DEFAULT_OUTPUT,
    active_recipe_texture_overrides,
    build_main_dissolve_priority,
    canonical_json_sha256,
    family_named_texture_parameters,
    parse_material_resource_tail,
    resolve_lookup_precedence,
    semantic_projection,
)


class FakeImport:
    def __init__(self, name: str) -> None:
        self.index = 0
        self.class_package = "engine"
        self.class_name = "texture2d"
        self.package_index = 0
        self.object_name = name


class FakePackage:
    def __init__(self) -> None:
        self.imports = [FakeImport("fixture_texture")]
        self.exports = []


def fixture_tail(*, texture_index: int = 0, remainder: bytes = b"") -> bytes:
    return (
        struct.pack("<IIII", 1, 0, 0, 2)
        + bytes.fromhex("00112233445566778899aabbccddeeff")
        + struct.pack("<I", 1)
        + struct.pack("<I", 1)
        + struct.pack("<i", -1)
        + struct.pack("<IIIIII", 0, 0, 1, 0, 0, 0)
        + struct.pack("<I", 1)
        + struct.pack("<ii2f", 0, texture_index, 1.0, 1.0)
        + struct.pack("<IIII", 0, 2, 0, 0)
        + remainder
    )


class MaterialNativeResourceTests(unittest.TestCase):
    def test_bounded_tail_decodes_reference_and_lookup(self) -> None:
        row = parse_material_resource_tail(fixture_tail(), FakePackage())
        self.assertEqual(row["parsedByteCount"], len(fixture_tail()))
        self.assertEqual(row["unparsedByteCount"], 0)
        self.assertEqual(row["referencedTextures"][0]["objectPath"], "fixture_texture")
        self.assertEqual(row["legacyTextureLookups"][0]["textureIndex"], 0)
        self.assertEqual(row["legacyTextureLookups"][0]["uScale"], 1.0)
        self.assertEqual(
            row["legacyTextureLookups"][0]["fidelity"],
            "SOURCE_EXACT_LEGACY_TEXTURE_LOOKUP_METADATA_NOT_SHADER_UV_UNIFORM",
        )

    def test_invalid_lookup_index_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "texture index"):
            parse_material_resource_tail(fixture_tail(texture_index=1), FakePackage())

    def test_truncation_and_trailing_bytes_are_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "truncated|shape"):
            parse_material_resource_tail(fixture_tail()[:-4], FakePackage())
        with self.assertRaisesRegex(ValueError, "unparsed bytes|shape"):
            parse_material_resource_tail(fixture_tail(remainder=b"\x00\x00\x00\x00"), FakePackage())

    def test_semantic_projection_cannot_promote_legacy_lookup_to_shader_uv(self) -> None:
        decoded = parse_material_resource_tail(fixture_tail(), FakePackage())
        decoded.update({"className": "material"})
        projected = semantic_projection(decoded)
        encoded = json.dumps(projected, sort_keys=True)
        self.assertNotIn("shaderUvUniform", encoded)
        self.assertNotIn("arithmeticGraph", encoded)
        self.assertEqual(projected["legacyTextureLookups"][0]["uScale"], 1.0)
        self.assertEqual(canonical_json_sha256(projected), canonical_json_sha256(copy.deepcopy(projected)))

    def test_lookup_parameter_join_and_active_recipe_filter_are_exact(self) -> None:
        families = [
            {
                "familyId": "family-a",
                "physicalPackage": "BASE.upk",
                "materialObjectPath": "pkg.base",
            }
        ]
        closure = {
            "materials": [
                {
                    "parentGraph": {
                        "physicalPackage": "base.UPK",
                        "graph": {
                            "materialPath": "PKG.BASE",
                            "namedTextures": [
                                {"name": "Map", "sourceObjectPath": "tex.base"}
                            ],
                        },
                    }
                }
            ]
        }
        contract = {
            "occurrences": [{"materialRecipeId": "active"}],
            "materialRecipes": [
                {
                    "recipeId": "active",
                    "sourceMaterialPath": "mi.active",
                    "arithmeticFamilyId": "family-a",
                    "inputs": {
                        "textureOverrides": [
                            {
                                "parameterName": "MAP",
                                "value": "tex.override",
                            }
                        ]
                    },
                },
                {
                    "recipeId": "inactive",
                    "sourceMaterialPath": "mi.inactive",
                    "arithmeticFamilyId": "family-a",
                    "inputs": {"textureOverrides": []},
                },
            ],
        }
        self.assertEqual(
            family_named_texture_parameters(families, closure),
            {"family-a": {"tex.base": {"map"}}},
        )
        recipes = active_recipe_texture_overrides(contract)
        self.assertEqual(len(recipes["family-a"]), 1)
        self.assertEqual(recipes["family-a"][0]["textureOverrides"], {"map": "tex.override"})

    def test_lookup_precedence_distinguishes_replacement_identity_and_default(self) -> None:
        named = {"family-a": {"tex.base": {"map"}}}
        replacement = resolve_lookup_precedence(
            "tex.base",
            ["family-a"],
            named,
            {
                "family-a": [
                    {
                        "recipeId": "r",
                        "sourceMaterialPath": "mi.r",
                        "textureOverrides": {"map": "tex.override"},
                    }
                ]
            },
            {"tex.override"},
        )
        self.assertTrue(replacement["replacedWithDifferentTextureInAllActiveRecipes"])
        self.assertFalse(replacement["logicalTextureEffectiveAfterMicPrecedence"])
        self.assertTrue(
            replacement["legacyTextureLookupResolutions"][0][
                "effectiveOverrideValuesRuntimeBound"
            ]
        )

        identity = resolve_lookup_precedence(
            "tex.base",
            ["family-a"],
            named,
            {
                "family-a": [
                    {
                        "recipeId": "r",
                        "sourceMaterialPath": "mi.r",
                        "textureOverrides": {"map": "tex.base"},
                    }
                ]
            },
            {"tex.base"},
        )
        self.assertTrue(identity["identityOverridePreservesLogicalTexture"])
        self.assertTrue(identity["logicalTextureEffectiveAfterMicPrecedence"])

        inherited = resolve_lookup_precedence(
            "tex.base",
            ["family-a"],
            named,
            {
                "family-a": [
                    {
                        "recipeId": "r",
                        "sourceMaterialPath": "mi.r",
                        "textureOverrides": {},
                    }
                ]
            },
            {"tex.base"},
        )
        self.assertTrue(inherited["baseDefaultEffectiveForActiveArtist31470"])
        self.assertTrue(inherited["logicalTextureEffectiveAfterMicPrecedence"])

    def test_main_dissolve_native_metadata_has_no_effective_membership_delta(self) -> None:
        native = json.loads(Path(DEFAULT_OUTPUT).read_text(encoding="utf-8"))
        contract = json.loads(Path(DEFAULT_MATERIAL_CONTRACT).read_text(encoding="utf-8"))
        priority = build_main_dissolve_priority(native["families"], contract)
        self.assertEqual(priority["summary"]["priorityFamilyCount"], 2)
        self.assertEqual(priority["summary"]["legacyTextureLookupCount"], 0)
        self.assertEqual(priority["summary"]["effectiveBindingCount"], 9)
        self.assertEqual(priority["summary"]["effectiveUniqueTextureCountByFamily"], [2, 6])
        self.assertEqual(
            priority["summary"]["metadataEffectiveTextureMembershipDeltaCount"],
            0,
        )
        self.assertTrue(
            all(row["shaderUvUniformAdmission"] is False for row in priority["rows"])
        )


if __name__ == "__main__":
    unittest.main()
