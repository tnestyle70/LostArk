#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path

from build_effect_components import (
    component_directory,
    compile_assembly,
    remove_relocated_generated_components,
    remove_stale_generated_components,
    remove_unadmitted_generated_artifacts,
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
    def test_component_identity_never_depends_on_input_slot(self) -> None:
        document = {
            "schema": "lostark.effect-authoring",
            "version": 10,
            "effectAssetId": "effect.dimensionmaster.skill.42",
            "displayName": "test",
            "particleSystem": {},
            "modelCues": [],
            "elements": [element("a", "group.a", 0.0)],
        }
        _assembly_q, files_q = split_document(
            document, "DimensionMaster", "Q"
        )
        _assembly_s, files_s = split_document(
            document, "DimensionMaster", "S"
        )

        self.assertEqual(files_q[0][0], files_s[0][0])
        self.assertEqual(
            files_q[0][1]["componentAssetId"],
            files_s[0][1]["componentAssetId"],
        )
        self.assertEqual(
            "effect.component.dimensionmaster.skill.42.00",
            files_q[0][1]["componentAssetId"],
        )

    def test_ba_stage_identity_has_distinct_component_directory(self) -> None:
        self.assertEqual(
            "skill.2050010.ba1",
            component_directory(
                "effect.dimensionmaster.skill.2050010.ba1",
                "DimensionMaster",
            ),
        )
        self.assertNotEqual(
            component_directory(
                "effect.dimensionmaster.skill.2050010.ba1",
                "DimensionMaster",
            ),
            component_directory(
                "effect.dimensionmaster.skill.2050010.ba2",
                "DimensionMaster",
            ),
        )

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

    def test_relocated_component_cleanup_preserves_foreign_sources(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            old = root / "Q"
            expected = root / "skill.10"
            old.mkdir()
            expected.mkdir()

            def write(path: Path, effect_id: str) -> None:
                path.write_text(json.dumps({
                    "schema": "lostark.effect-component",
                    "version": 1,
                    "source": {"effectAssetId": effect_id},
                }), encoding="utf-8")

            write(old / "relocated.wfx.json", "effect.dimensionmaster.skill.10")
            write(old / "foreign.wfx.json", "effect.dimensionmaster.skill.20")
            write(expected / "current.wfx.json", "effect.dimensionmaster.skill.10")

            removed = remove_relocated_generated_components(
                root, expected, "effect.dimensionmaster.skill.10"
            )

            self.assertEqual([str(old / "relocated.wfx.json")], removed)
            self.assertFalse((old / "relocated.wfx.json").exists())
            self.assertTrue((old / "foreign.wfx.json").exists())
            self.assertTrue((expected / "current.wfx.json").exists())

    def test_unadmitted_cleanup_removes_only_generated_dimensionmaster_files(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            components = root / "Components"
            assemblies = root / "Assemblies"
            components.mkdir()
            assemblies.mkdir()

            def write_component(name: str, effect_id: str) -> None:
                (components / name).write_text(json.dumps({
                    "schema": "lostark.effect-component",
                    "version": 1,
                    "source": {"effectAssetId": effect_id},
                }), encoding="utf-8")

            def write_assembly(name: str, effect_id: str) -> None:
                (assemblies / name).write_text(json.dumps({
                    "schema": "lostark.effect-assembly",
                    "version": 1,
                    "effectAssetId": effect_id,
                }), encoding="utf-8")

            admitted = "effect.dimensionmaster.skill.10"
            candidate = "effect.dimensionmaster.skill.99"
            write_component("admitted.wfx.json", admitted)
            write_component("candidate.wfx.json", candidate)
            write_component("foreign.wfx.json", "effect.other.skill.99")
            write_assembly("admitted.assembly.json", admitted)
            write_assembly("candidate.assembly.json", candidate)
            write_assembly("foreign.assembly.json", "effect.other.skill.99")

            removed_components, removed_assemblies = (
                remove_unadmitted_generated_artifacts(
                    components, assemblies, {admitted}
                )
            )

            self.assertEqual(
                [str(components / "candidate.wfx.json")],
                removed_components,
            )
            self.assertEqual(
                [str(assemblies / "candidate.assembly.json")],
                removed_assemblies,
            )
            self.assertTrue((components / "admitted.wfx.json").exists())
            self.assertTrue((components / "foreign.wfx.json").exists())
            self.assertTrue((assemblies / "admitted.assembly.json").exists())
            self.assertTrue((assemblies / "foreign.assembly.json").exists())


if __name__ == "__main__":
    unittest.main()
