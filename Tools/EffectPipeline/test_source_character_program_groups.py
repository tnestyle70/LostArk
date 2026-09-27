"""Native authoring partition must preserve source and isolate an actual edit."""
from pathlib import Path
import shutil
import tempfile
import unittest
import sys
from unittest import mock

from native_shader_dispatch import (
    SOURCE_CHARACTER_PROGRAM_GROUPS,
    SOURCE_CHARACTER_REGISTERED_PROGRAMS,
    expand_source_character_stage,
    partition_source_character_stage,
    write_partitioned_source_character_stage,
    write_if_changed,
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

    def test_registration_in_existing_cohort_changes_only_its_two_leaves(self):
        sys.path.insert(0, str(ROOT / 'Tools/VehiclePipeline'))
        import build_vehicle_source_material as vehicle
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            for path in SHADERS.glob('Shader_SourceCharacter*.hlsli'):
                shutil.copyfile(path, target / path.name)
            # Exercise the installer's sparse insertion and the actual stage
            # writer, including a new exact ID inside the final installed group.
            number = max(SOURCE_CHARACTER_REGISTERED_PROGRAMS) + 1
            cohort = next(first for first, last in SOURCE_CHARACTER_PROGRAM_GROUPS
                          if first <= number <= last)
            registered = (*SOURCE_CHARACTER_REGISTERED_PROGRAMS, number)
            before = {item.name: (item.read_bytes(), item.stat().st_mtime_ns)
                      for item in target.iterdir()}
            for stage in ('Base', 'Light'):
                path = target / f'Shader_SourceCharacter{stage}Programs.hlsli'
                function = (f'SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacter{stage}{number}'
                            '(SOURCE_CHARACTER_NATIVE_INPUT input)\n'
                            '{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }\n')
                with mock.patch.object(vehicle, f'{stage.upper()}_PROGRAMS', path):
                    source = vehicle.install_program(path, function, number, stage.lower())
                facade, leaves = partition_source_character_stage(source, stage, target,
                                                                  registered=registered)
                for name, text in {path.name: facade, **leaves}.items():
                    write_if_changed(target / name, text)
                self.assertEqual(source, expand_source_character_stage(
                    path.read_text(encoding='utf8'), target))
            changed = sorted(item.name for item in target.iterdir()
                             if before[item.name] != (item.read_bytes(), item.stat().st_mtime_ns))
            self.assertEqual([f'Shader_SourceCharacterBaseGroup{cohort:03d}.hlsli',
                              f'Shader_SourceCharacterLightGroup{cohort:03d}.hlsli'], changed)

    def test_light_material_edit_changes_its_leaf_only(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            for path in SHADERS.glob('Shader_SourceCharacter*.hlsli'):
                shutil.copyfile(path, target / path.name)
            path = target / 'Shader_SourceCharacterLightPrograms.hlsli'
            source = expand_source_character_stage(path.read_text(encoding='utf8'), target)
            source = source.replace('SourceCharacterLight9(SOURCE_CHARACTER_NATIVE_INPUT input)\n{',
                                    'SourceCharacterLight9(SOURCE_CHARACTER_NATIVE_INPUT input)\n{\n    // editor fixture', 1)
            before = {item.name: item.read_bytes() for item in target.iterdir()}
            write_partitioned_source_character_stage(path, source)
            changed = [item.name for item in target.iterdir() if item.read_bytes() != before[item.name]]
            self.assertEqual(['Shader_SourceCharacterLightGroup009.hlsli'], changed)

    def test_reordered_cohort_cases_are_rejected(self):
        path = SHADERS / 'Shader_SourceCharacterBasePrograms.hlsli'
        source = expand_source_character_stage(path.read_text(encoding='utf8'), SHADERS)
        first = '    case 1u: return SourceCharacterBase1(input);\n'
        later = '    case 9u: return SourceCharacterBase9(input);\n'
        source = source.replace(first, '', 1).replace(later, later + first, 1)
        with self.assertRaisesRegex(ValueError, 'Non-contiguous.*case cohort'):
            partition_source_character_stage(source, 'Base', SHADERS)

    def test_unregistered_program_is_rejected_before_writing(self):
        path = SHADERS / 'Shader_SourceCharacterBasePrograms.hlsli'
        source = expand_source_character_stage(path.read_text(encoding='utf8'), SHADERS)
        unsupported = max(last for _first, last in SOURCE_CHARACTER_PROGRAM_GROUPS) + 1
        source = source.replace('SourceCharacterBase88(', f'SourceCharacterBase{unsupported}(')
        with self.assertRaisesRegex(ValueError, 'registered CSO cohort'):
            partition_source_character_stage(source, 'Base', SHADERS)


if __name__ == '__main__':
    unittest.main()
