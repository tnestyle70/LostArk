"""Native authoring partition must preserve source and isolate an actual edit."""
from pathlib import Path
import shutil
import tempfile
import unittest

from native_shader_dispatch import (
    expand_source_character_stage,
    partition_source_character_stage,
    write_partitioned_source_character_stage,
)

ROOT = Path(__file__).resolve().parents[2]
SHADERS = ROOT / 'Engine/Bin/ShaderFiles'


class SourceCharacterProgramGroups(unittest.TestCase):
    def test_all_installed_programs_round_trip_and_noop_preserves_timestamps(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            for path in SHADERS.glob('Shader_SourceCharacter*.hlsli'):
                shutil.copyfile(path, target / path.name)
            for stage in ('Base', 'Light'):
                path = target / f'Shader_SourceCharacter{stage}Programs.hlsli'
                source = expand_source_character_stage(path.read_text(encoding='utf8'), target)
                before = {item: item.stat().st_mtime_ns for item in target.iterdir()}
                write_partitioned_source_character_stage(path, source)
                self.assertEqual(before, {item: item.stat().st_mtime_ns for item in target.iterdir()})
                self.assertEqual(source, expand_source_character_stage(path.read_text(encoding='utf8'), target))

    def test_existing_material_edit_changes_its_leaf_only(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            for path in SHADERS.glob('Shader_SourceCharacter*.hlsli'):
                shutil.copyfile(path, target / path.name)
            path = target / 'Shader_SourceCharacterBasePrograms.hlsli'
            source = expand_source_character_stage(path.read_text(encoding='utf8'), target)
            source = source.replace('SourceCharacterBase9(SOURCE_CHARACTER_NATIVE_INPUT input)\n{',
                                    'SourceCharacterBase9(SOURCE_CHARACTER_NATIVE_INPUT input)\n{\n    // editor fixture', 1)
            before = {item.name: item.read_bytes() for item in target.iterdir()}
            write_partitioned_source_character_stage(path, source)
            changed = [item.name for item in target.iterdir() if item.read_bytes() != before[item.name]]
            self.assertEqual(['Shader_SourceCharacterBaseGroup009.hlsli'], changed)

    def test_unregistered_program_is_rejected_before_writing(self):
        path = SHADERS / 'Shader_SourceCharacterBasePrograms.hlsli'
        source = expand_source_character_stage(path.read_text(encoding='utf8'), SHADERS)
        source = source.replace('SourceCharacterBase88(', 'SourceCharacterBase89(')
        with self.assertRaisesRegex(ValueError, 'registered CSO cohort'):
            partition_source_character_stage(source, 'Base', SHADERS)


if __name__ == '__main__':
    unittest.main()
