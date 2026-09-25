"""Native authoring partition must preserve source and isolate an actual edit."""
from pathlib import Path
import shutil
import tempfile
import unittest
import sys

from native_shader_dispatch import (
    SOURCE_CHARACTER_PROGRAM_GROUPS,
    expand_source_character_stage,
    partition_source_character_stage,
    write_partitioned_source_character_stage,
)

ROOT = Path(__file__).resolve().parents[2]
SHADERS = ROOT / 'Engine/Bin/ShaderFiles'


class SourceCharacterProgramGroups(unittest.TestCase):
    def test_sparse_insert_stays_before_the_later_baked_helper(self):
        sys.path.insert(0, str(ROOT / 'Tools/VehiclePipeline'))
        from build_vehicle_source_material import install_program
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            path = target / 'Shader_SourceCharacterBasePrograms.hlsli'
            function = lambda name: ('SOURCE_CHARACTER_NATIVE_OUTPUT ' + name +
                '(SOURCE_CHARACTER_NATIVE_INPUT input)\n{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }\n')
            original = (function('SourceCharacterBase1155') + function('SourceMapMonsterBaked1400') +
                function('SourceCharacterBase1400') +
                'SOURCE_CHARACTER_NATIVE_OUTPUT EvaluateSourceCharacterBase(SOURCE_CHARACTER_NATIVE_INPUT input)\n'
                '{\n    switch(0) {\n    case 1155u: return SourceCharacterBase1155(input);\n'
                '    case 1400u: return SourceCharacterBase1400(input);\n    }\n}\n')
            path.write_text(original, encoding='utf8')
            result = install_program(path, function('SourceCharacterBase1166'), 1166, 'base')
            self.assertLess(result.index('SourceCharacterBase1166('), result.index('SourceMapMonsterBaked1400('))
            facade, leaves = partition_source_character_stage(result, 'Base', target,
                groups=((1152, 1215), (1344, 1407)), registered=(1155, 1166, 1400))
            for name, text in leaves.items():
                (target / name).write_text(text, encoding='utf8')
            self.assertEqual(result, expand_source_character_stage(facade, target))

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
        unsupported = max(last for _first, last in SOURCE_CHARACTER_PROGRAM_GROUPS) + 1
        source = source.replace('SourceCharacterBase88(', f'SourceCharacterBase{unsupported}(')
        with self.assertRaisesRegex(ValueError, 'registered CSO cohort'):
            partition_source_character_stage(source, 'Base', SHADERS)


if __name__ == '__main__':
    unittest.main()
