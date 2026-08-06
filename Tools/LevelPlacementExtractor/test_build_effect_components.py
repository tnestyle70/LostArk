#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path

from build_effect_components import (
    compile_assembly,
    remove_stale_generated_components,
    split_document,
)


def element(element_id: str, group_id: str, start: float) -> dict:
    return {
        "id": element_id,
        "displayName": element_id,
        "groupId": group_id,
        "sourceNode": f"source|{element_id}",
        "visible": True,
        "kind": "particle",
        "resources": [{"slotId": "base", "assetId": "Effect/Test.dds"}],
        "material": {
            "templateId": "effect.standard",
            "sourceMaterialPath": "source.material",
            "renderProfile": "alpha_two_sided_depth_read",
        },
        "detail": {"timing": {"startDelaySeconds": start}},
        "sourceRecipe": {"rendererShape": "sprite", "modules": []},
    }


class EffectComponentTests(unittest.TestCase):
    def test_split_compile_identity(self) -> None:
        document = {
            "schema": "lostark.effect-authoring",
            "version": 10,
            "effectAssetId": "effect.dimensionmaster.skill.1",
            "displayName": "test",
            "particleSystem": {},
            "modelCues": [],
            "elements": [
                element("a", "group.a", 1.25),
                element("b", "group.a", 1.5),
                element("c", "group.b", 0.25),
            ],
        }
        assembly, files = split_document(document, "DimensionMaster", "S")
        components = {row["componentAssetId"]: row for _, row in files}
        compiled = compile_assembly(assembly, components)
        self.assertEqual(document, compiled)
        self.assertEqual(2, len(files))
        self.assertEqual(
            0.0,
            files[0][1]["document"]["elements"][0]
            ["detail"]["timing"]["startDelaySeconds"],
        )
        self.assertEqual(
            files[0][1]["emitters"][0]["emitterId"],
            files[0][1]["emitters"][0]["elementId"],
        )

    def test_missing_component_rolls_back_output_value(self) -> None:
        assembly = {
            "schema": "lostark.effect-assembly",
            "version": 1,
            "effectAssetId": "effect.test",
            "displayName": "test",
            "sourceAuthoringVersion": 10,
            "componentCues": [{
                "componentAssetId": "effect.component.missing",
                "startDelaySeconds": 0.0,
            }],
        }
        before = copy.deepcopy(assembly)
        with self.assertRaises(ValueError):
            compile_assembly(assembly, {})
        self.assertEqual(before, assembly)

    def test_stale_generated_component_cleanup_is_source_scoped(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)

            def write(name: str, effect_id: str) -> None:
                (root / name).write_text(json.dumps({
                    "schema": "lostark.effect-component",
                    "version": 1,
                    "source": {"effectAssetId": effect_id},
                }), encoding="utf-8")

            write("current.wfx.json", "effect.test")
            write("stale.wfx.json", "effect.test")
            write("foreign.wfx.json", "effect.foreign")
            removed = remove_stale_generated_components(
                root, {"current.wfx.json"}, "effect.test"
            )
            self.assertEqual(["stale.wfx.json"], removed)
            self.assertTrue((root / "current.wfx.json").exists())
            self.assertTrue((root / "foreign.wfx.json").exists())
            self.assertFalse((root / "stale.wfx.json").exists())


if __name__ == "__main__":
    unittest.main()
