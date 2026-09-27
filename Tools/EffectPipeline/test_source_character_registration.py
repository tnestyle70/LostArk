"""Dense native cohorts stay bounded through the real installer registration path."""
from pathlib import Path
import sys
import unittest

from native_shader_dispatch import expand_source_character_stage
from source_character_registration import (
    REGISTRY, extend_registry, registered_program_group, registered_programs,
    registry_groups, stage_registration,
)

ROOT = Path(__file__).resolve().parents[2]


class SourceCharacterRegistration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.registry = (ROOT / REGISTRY).read_text(encoding='utf8')
        cls.source = {}
        for stage in ('Base', 'Light'):
            path = ROOT / f'Engine/Bin/ShaderFiles/Shader_SourceCharacter{stage}Programs.hlsli'
            cls.source[stage] = expand_source_character_stage(path.read_text(encoding='utf8'), path.parent)
        sys.path.insert(0, str(ROOT / 'Tools/VehiclePipeline'))
        import build_vehicle_source_material as vehicle
        cls.configure = vehicle.read_text(vehicle.PARAMETER_HEADER)

    def test_dense_range_boundaries_and_other_allocations_are_stable(self):
        for value in (1088, 1103, 1104, 1119, 1120, 1135, 1136, 1151):
            first = value // 16 * 16
            self.assertEqual((first, first + 15), registered_program_group(value))
        for value in (600, 702, 1152, 1166, 1400, 1526):
            first = value // 64 * 64
            self.assertEqual((first, first + 63), registered_program_group(value))
        groups = registry_groups(self.registry)
        for item in ((84, 95), (96, 107), (108, 112), (1088, 1103),
                     (1104, 1119), (1120, 1135), (1136, 1151)):
            self.assertIn(item, groups)

    def test_reinstall_stages_no_files(self):
        staged, groups, programs = stage_registration(ROOT, (), self.source['Base'],
                                                      self.source['Light'], self.configure)
        self.assertEqual({}, staged)
        self.assertEqual(registry_groups(self.registry), groups)
        self.assertEqual(registered_programs(self.registry), programs)

    def test_new_id_in_existing_cohort_keeps_project_and_wrapper_inputs(self):
        number = max(registered_programs(self.registry)) + 1
        source = {}
        for stage in ('Base', 'Light'):
            function = f'SourceCharacter{stage}{number}'
            evaluator = f'SOURCE_CHARACTER_NATIVE_OUTPUT EvaluateSourceCharacter{stage}('
            body = (f'SOURCE_CHARACTER_NATIVE_OUTPUT {function}(SOURCE_CHARACTER_NATIVE_INPUT input)\n'
                    '{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }\n')
            text = self.source[stage].replace(evaluator, body + evaluator, 1)
            end_case = f'    case {number - 1}u: return SourceCharacter{stage}{number - 1}(input);\n'
            source[stage] = text.replace(end_case, end_case + f'    case {number}u: return {function}(input);\n', 1)
        staged, groups, _programs = stage_registration(ROOT, (number,), source['Base'], source['Light'],
                                                      self.configure + f'\nstaged.program = {number}u;\n')
        self.assertEqual([ROOT / REGISTRY], list(staged))
        self.assertEqual(registry_groups(self.registry), groups)

    def test_obsolete_dense_cohort_layout_is_rejected(self):
        text = self.registry
        for first in (1104, 1120, 1136):
            text = text.replace(f'        {{{first}u, {first + 15}u}},\n', '')
        text = text.replace('{1088u, 1103u}', '{1088u, 1151u}')
        with self.assertRaisesRegex(ValueError, 'IDs and cohorts disagree'):
            registry_groups(text)
        # Re-extraction uses the reviewed policy, rather than restoring 64 IDs.
        self.assertEqual(self.registry, extend_registry(text, ()))


if __name__ == '__main__':
    unittest.main()
