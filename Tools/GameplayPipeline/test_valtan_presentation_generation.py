import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import valtan_presentation_generation as generation


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class ValtanPresentationGenerationTests(unittest.TestCase):
    @staticmethod
    def _write_overlay_json(
        overlay: Path, relative: str, document: dict
    ) -> None:
        destination = overlay / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(
            json.dumps(document, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )

    @staticmethod
    def _valtan_effect_v2_paths(
        built: generation.PresentationGeneration,
    ) -> set[str]:
        return {
            artifact.path
            for artifact in built.artifacts
            if artifact.path == generation.EFFECT_V2_BINDINGS_REL
            or artifact.path.startswith(generation.EFFECT_V2_GROUP_ROOT_REL + "/")
            or artifact.path.startswith(generation.EFFECT_V2_AUTHORED_ROOT_REL + "/")
        }

    def test_generation_is_deterministic_and_keeps_live_pattern_sound_separate(self) -> None:
        first = generation.build_presentation_generation(REPOSITORY_ROOT)
        second = generation.build_presentation_generation(REPOSITORY_ROOT)

        self.assertEqual(first.generation_id, second.generation_id)
        self.assertEqual(first.manifest_bytes, second.manifest_bytes)
        paths = {artifact.path for artifact in first.artifacts}
        self.assertEqual(len(paths), len(first.artifacts))
        for _, fixed_path in generation.FIXED_ARTIFACTS:
            self.assertIn(fixed_path, paths)
        self.assertTrue(
            any(path.startswith("Data/Effects/Authored/") for path in paths)
        )
        self.assertNotIn(
            "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json",
            paths,
        )
        self.assertIn(generation.COMBAT_OBJECT_SOUND_CUES_REL, paths)
        self.assertIn(generation.CHARACTER_SOUND_CATALOG_REL, paths)
        self.assertIn(generation.EFFECT_V1_ALIASES_REL, paths)

    def test_every_exact_runtime_dependency_changes_generation(self) -> None:
        baseline = generation.build_presentation_generation(REPOSITORY_ROOT)
        for relative in (
            generation.COMBAT_OBJECT_SOUND_CUES_REL,
            generation.CHARACTER_SOUND_CATALOG_REL,
            generation.EFFECT_V1_ALIASES_REL,
        ):
            with self.subTest(relative=relative), tempfile.TemporaryDirectory() as temporary:
                overlay = Path(temporary)
                destination = overlay / relative
                destination.parent.mkdir(parents=True)
                destination.write_bytes((REPOSITORY_ROOT / relative).read_bytes() + b"\n")
                changed = generation.build_presentation_generation(
                    REPOSITORY_ROOT, overlay
                )
                self.assertNotEqual(baseline.generation_id, changed.generation_id)

    def test_boss_valtan_effect_v2_binding_group_leaf_closure_is_exact(self) -> None:
        built = generation.build_presentation_generation(REPOSITORY_ROOT)
        expected = {
            generation.EFFECT_V2_BINDINGS_REL,
            "Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json",
            *(
                f"Data/Effects/V2/Authored/boss.valtan.hand_{index}.effectv2.json"
                for index in range(1, 7)
            ),
            *(
                f"Data/Effects/V2/Authored/boss.valtan.hit_{index}.effectv2.json"
                for index in range(1, 4)
            ),
            "Data/Effects/V2/Authored/boss.valtan.decal_1.effectv2.json",
            "Data/Effects/V2/Authored/boss.valtan.decal_2.effectv2.json",
            "Data/Effects/V2/Authored/boss.valtan.spread_1.effectv2.json",
        }

        self.assertEqual(14, len(expected))
        self.assertEqual(expected, self._valtan_effect_v2_paths(built))

    def test_each_referenced_effect_v2_layer_changes_generation(self) -> None:
        baseline = generation.build_presentation_generation(REPOSITORY_ROOT)
        referenced = (
            generation.EFFECT_V2_BINDINGS_REL,
            "Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json",
            "Data/Effects/V2/Authored/boss.valtan.hit_1.effectv2.json",
        )
        for relative in referenced:
            with self.subTest(relative=relative), tempfile.TemporaryDirectory() as temporary:
                overlay = Path(temporary)
                destination = overlay / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes((REPOSITORY_ROOT / relative).read_bytes() + b"\n")
                changed = generation.build_presentation_generation(
                    REPOSITORY_ROOT, overlay
                )
                self.assertNotEqual(baseline.generation_id, changed.generation_id)

    def test_unreferenced_effect_v2_leaf_does_not_change_valtan_generation(self) -> None:
        baseline = generation.build_presentation_generation(REPOSITORY_ROOT)
        referenced = self._valtan_effect_v2_paths(baseline)
        all_leaves = {
            path.relative_to(REPOSITORY_ROOT).as_posix()
            for path in (REPOSITORY_ROOT / generation.EFFECT_V2_AUTHORED_ROOT_REL).glob(
                "*.effectv2.json"
            )
        }
        unreferenced = sorted(all_leaves - referenced)
        self.assertTrue(unreferenced)

        with tempfile.TemporaryDirectory() as temporary:
            overlay = Path(temporary)
            relative = unreferenced[0]
            destination = overlay / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes((REPOSITORY_ROOT / relative).read_bytes() + b"\n")
            changed = generation.build_presentation_generation(
                REPOSITORY_ROOT, overlay
            )
        self.assertEqual(baseline.generation_id, changed.generation_id)
        self.assertEqual(baseline.manifest_bytes, changed.manifest_bytes)

    def test_effect_v2_missing_or_mismatched_group_and_leaf_fail_closed(self) -> None:
        binding_source = json.loads(
            (REPOSITORY_ROOT / generation.EFFECT_V2_BINDINGS_REL).read_text(
                encoding="utf-8"
            )
        )
        group_relative = (
            "Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json"
        )
        group_source = json.loads(
            (REPOSITORY_ROOT / group_relative).read_text(encoding="utf-8")
        )
        leaf_relative = "Data/Effects/V2/Authored/boss.valtan.hit_1.effectv2.json"
        leaf_source = json.loads(
            (REPOSITORY_ROOT / leaf_relative).read_text(encoding="utf-8")
        )

        cases: list[tuple[str, str, dict]] = []
        missing_group = copy.deepcopy(binding_source)
        group_binding = next(row for row in missing_group["bindings"] if "group" in row)
        group_binding["group"] = "boss.valtan.missing-group"
        cases.append(("missing group", generation.EFFECT_V2_BINDINGS_REL, missing_group))

        mismatched_group = copy.deepcopy(group_source)
        mismatched_group["groupId"] = "boss.valtan.other-group"
        cases.append(("mismatched group", group_relative, mismatched_group))

        missing_leaf = copy.deepcopy(group_source)
        missing_leaf["children"][0]["effectId"] = "boss.valtan.missing-leaf"
        cases.append(("missing leaf", group_relative, missing_leaf))

        mismatched_leaf = copy.deepcopy(leaf_source)
        mismatched_leaf["effectId"] = "boss.valtan.other-leaf"
        cases.append(("mismatched leaf", leaf_relative, mismatched_leaf))

        for label, relative, document in cases:
            with self.subTest(case=label), tempfile.TemporaryDirectory() as temporary:
                overlay = Path(temporary)
                self._write_overlay_json(overlay, relative, document)
                with self.assertRaises(generation.PresentationGenerationError):
                    generation.build_presentation_generation(
                        REPOSITORY_ROOT, overlay
                    )

    def test_runtime_pins_catalog_assets_and_keeps_v1_local_preview_only(self) -> None:
        valtan = (REPOSITORY_ROOT / "Client/Private/Valtan.cpp").read_text(
            encoding="utf-8-sig"
        )
        pattern_sound = (
            REPOSITORY_ROOT
            / "Client/Private/ValtanPatternSoundCueDocument.cpp"
        ).read_text(encoding="utf-8-sig")
        combat_sound = (
            REPOSITORY_ROOT
            / "Client/Private/ValtanCombatObjectSoundCueDocument.cpp"
        ).read_text(encoding="utf-8-sig")
        catalog = (
            REPOSITORY_ROOT / "Client/Private/SoundCueCatalog.cpp"
        ).read_text(encoding="utf-8-sig")

        self.assertIn("Cue.ResolvedAssetIds", valtan)
        self.assertIn("cue.ResolvedAssetIds", valtan)
        self.assertNotIn('CSoundCueCatalog::Find_Variants("Valtan"', valtan)
        self.assertIn("!m_isServerAuthoritative", valtan)
        self.assertIn("Load_ClassSnapshot", pattern_sound)
        self.assertIn("Cue.ResolvedAssetIds = Event->second", pattern_sound)
        self.assertIn("Load_ClassSnapshot", combat_sound)
        self.assertIn("cue.ResolvedAssetIds = found->second", combat_sound)
        self.assertLess(
            catalog.index("if (!Parse_Catalog(Staged, strOutStatus))"),
            catalog.index("s_ClassEvents = std::move(Staged)"),
        )

    def test_unknown_effect_cue_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            overlay = Path(temporary)
            cue_path = overlay / generation.EFFECT_CUES_REL
            cue_path.parent.mkdir(parents=True)
            source = json.loads(
                (REPOSITORY_ROOT / generation.EFFECT_CUES_REL).read_text(
                    encoding="utf-8"
                )
            )
            source["cues"][0]["effectAssetId"] = "effect.not.in.catalog"
            cue_path.write_text(json.dumps(source), encoding="utf-8")

            with self.assertRaises(generation.PresentationGenerationError):
                generation.build_presentation_generation(
                    REPOSITORY_ROOT, overlay
                )

    def test_publish_collision_preserves_existing_bytes(self) -> None:
        built = generation.build_presentation_generation(REPOSITORY_ROOT)
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary)
            directory = output / generation.GENERATION_DIRECTORY
            directory.mkdir(parents=True)
            destination = directory / f"{built.generation_id}.json"
            original = b"not-the-generation"
            destination.write_bytes(original)

            with self.assertRaises(generation.PresentationGenerationError):
                generation.publish_generation_manifest(built, output)
            self.assertEqual(original, destination.read_bytes())


if __name__ == "__main__":
    unittest.main()
