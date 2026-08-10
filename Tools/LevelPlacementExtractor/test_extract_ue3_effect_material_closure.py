#!/usr/bin/env python3

from __future__ import annotations

import struct
import unittest

from extract_ue3_effect_material_closure import (
    enabled_material_paths,
    enabled_material_renderer_shapes,
    enabled_system_ids,
    source_material_bindings,
    struct_linear_color,
)


class EffectMaterialClosureTests(unittest.TestCase):
    def test_linear_color_preserves_source_float_components(self) -> None:
        encoded = struct.pack("<4f", 0.1, 0.2, 0.3, 4.0).hex()
        actual = struct_linear_color({
            "structType": "linearcolor",
            "value": {"size": 16, "hex": encoded},
        })
        self.assertIsNotNone(actual)
        for left, right in zip(actual or [], [0.1, 0.2, 0.3, 4.0]):
            self.assertAlmostEqual(left, right)

    def test_disabled_variant_materials_are_not_admitted(self) -> None:
        recipe = {
            "cues": [
                {
                    "sourceType": "PlayParticleEffect",
                    "executionEnabled": True,
                    "assetReferences": [{"objectPath": "FX.Active"}],
                },
                {
                    "sourceType": "PlayParticleEffect",
                    "executionEnabled": False,
                    "assetReferences": [{"objectPath": "FX.Disabled"}],
                },
            ]
        }
        receipt = {
            "elementConversions": [
                {
                    "sourceSystemId": "fx.active",
                    "materialParameterEvidence": [
                        {"sourceMaterialPath": "FX_MI.Active"}
                    ],
                },
                {
                    "sourceSystemId": "fx.disabled",
                    "materialParameterEvidence": [
                        {"sourceMaterialPath": "FX_MI.Disabled"}
                    ],
                },
            ]
        }
        active = enabled_system_ids(recipe)
        self.assertEqual(active, {"fx.active"})
        self.assertEqual(
            enabled_material_paths(receipt, active), ["FX_MI.Active"]
        )
        self.assertEqual(
            enabled_material_renderer_shapes(receipt, active),
            {"fx_mi.active": ["unknown"]},
        )

    def test_unique_source_receipt_binding_is_indexed_case_insensitively(
        self,
    ) -> None:
        receipt = {
            "materialParameterBindings": [{
                "sourceMaterialPath": "FX_MI.Zoom",
                "resolutionStatus": "RESOLVED_UNIQUE_PATH",
                "sourcePhysicalPackage": "ABC.upk",
            }]
        }
        actual = source_material_bindings(receipt)
        self.assertEqual(
            actual["fx_mi.zoom"]["sourcePhysicalPackage"], "ABC.upk"
        )


if __name__ == "__main__":
    unittest.main()
