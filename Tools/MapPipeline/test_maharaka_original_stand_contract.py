"""Focused contact, scoped replacement, freshness and rollback contracts."""
import tempfile
import re
from pathlib import Path
import unittest
from unittest.mock import patch

from restore_maharaka_original_stand import ROOT, replace_table, top_at
import source_character_registration as transaction


class OriginalStandContractTests(unittest.TestCase):
    def test_native_family_identity_does_not_collide_with_guardian_or_npc(self):
        packing = (ROOT/'Client/Private/SourceCharacterMaterialParameters_Generated.inl').read_text(encoding='utf-8-sig')
        expected = {
            'source.character.equipment-native-1526.v1': 1526,
            'source.character.maharaka-ismp-1.v1': 1528,
            'source.character.maharaka-ismp-2.v1': 1527,
            'source.character.maharaka-itr02453-01.v1': 1532,
            'source.character.maharaka-itr02453-02.v1': 1529,
            'source.character.maharaka-itr02453-03.v1': 1530,
            'source.character.maharaka-itr02453-04.v1': 1531,
            'source.character.maharaka-resident-female.v1': 1474,
            'source.character.maharaka-resident-male.v1': 1475,
        }
        rows = re.findall(r'family == "([^"]+)"\)\s*\{\s*\[&\]\(\)\s*\{\s*staged.program = (\d+)u;', packing)
        resolved = dict(rows)
        for family, program in expected.items():
            self.assertEqual(int(resolved[family]), program)
        self.assertEqual(len(set(expected.values())), len(expected))
        for phase in ('Base', 'Light'):
            shader = (ROOT/f'Client/Bin/ShaderFiles/Shader_SourceCharacter{phase}Group1472.hlsli').read_text()
            self.assertEqual(shader, (ROOT/f'Engine/Bin/ShaderFiles/Shader_SourceCharacter{phase}Group1472.hlsli').read_text())
            for program in expected.values():
                self.assertEqual(shader.count(f'SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacter{phase}{program}('), 1)
                self.assertEqual(shader.count(f'case {program}u: return SourceCharacter{phase}{program}(input);'), 1)

    def test_cpu_and_shader_static_classification_agree(self):
        cpu = (ROOT/'Client/Public/SourceMovieMaterialPrograms.h').read_text()
        shader = (ROOT/'Client/Bin/ShaderFiles/Shader_SourceMovieStaticInputs.hlsli').read_text()
        mirror = (ROOT/'Engine/Bin/ShaderFiles/Shader_SourceMovieStaticInputs.hlsli').read_text()
        self.assertEqual(shader, mirror)
        def programs(text, name):
            body = re.search(r'bool '+name+r'\([^)]*\)\s*\{([^}]+)', text).group(1)
            pairs = re.findall(r'program\s*>=\s*(\d+)u\s*&&\s*program\s*<=\s*(\d+)u', body)
            return {n for a, b in pairs for n in range(int(a), int(b)+1)}
        supported = programs(cpu, 'Is_Static')
        self.assertEqual(supported, programs(shader, 'IsSourceMovieStatic'))
        self.assertTrue({1529, 1530, 1531, 1532} <= supported)
        self.assertTrue({1526, 1527, 1528}.isdisjoint(supported))

    def test_only_selected_old_rows_removed(self):
        before = b'LOSTARK_MAP_PLACEMENTS 2 "AREA" 3\r\nkeep central\r\nold stand\r\nkeep npc\r\n'
        after = replace_table(before, lambda line: line == 'old stand', 'original stand')
        self.assertEqual(after, b'LOSTARK_MAP_PLACEMENTS 2 "AREA" 3\r\nkeep central\r\nkeep npc\r\noriginal stand\r\n')

    def test_top_contact_uses_triangles_not_whole_bounds(self):
        placement = ['1', 'source', 'area', 'actor', 'asset', '0', '10', '0', '0', '0', '0', '1', '1', '1', '1', '1']
        triangles = [((-100, 200, -100), (100, 200, -100), (0, 200, 100)),
                     ((500, 800, 500), (600, 800, 500), (500, 800, 600))]
        self.assertEqual(top_at(triangles, placement, 0, 0), 12)
        with self.assertRaisesRegex(ValueError, 'No original support triangle'):
            top_at(triangles, placement, 100, 100)

    def test_concurrent_edit_is_not_overwritten(self):
        with tempfile.TemporaryDirectory() as folder:
            p = Path(folder)/'user-edit'
            p.write_bytes(b'user change')
            with self.assertRaisesRegex(ValueError, 'Concurrent'):
                transaction.commit_staged_files({p: (b'old', b'candidate')})
            self.assertEqual(p.read_bytes(), b'user change')

    def test_second_write_failure_rolls_back_first(self):
        with tempfile.TemporaryDirectory() as folder:
            a, b = Path(folder)/'a', Path(folder)/'b'
            a.write_bytes(b'old-a')
            b.write_bytes(b'old-b')
            original_replace = transaction.os.replace
            def replace(source, destination):
                if destination == b:
                    raise OSError('injected second promotion failure')
                return original_replace(source, destination)
            with patch.object(transaction.os, 'replace', side_effect=replace):
                with self.assertRaises(OSError):
                    transaction.commit_staged_files({a: (b'old-a', b'new-a'), b: (b'old-b', b'new-b')})
            self.assertEqual(a.read_bytes(), b'old-a')
            self.assertEqual(b.read_bytes(), b'old-b')
            self.assertEqual(sorted(p.name for p in Path(folder).iterdir()), ['a', 'b'])


if __name__ == '__main__':
    unittest.main()
