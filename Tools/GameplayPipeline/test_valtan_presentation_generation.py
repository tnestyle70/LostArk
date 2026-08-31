import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import valtan_presentation_generation as generation


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class ValtanPresentationGenerationTests(unittest.TestCase):
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
