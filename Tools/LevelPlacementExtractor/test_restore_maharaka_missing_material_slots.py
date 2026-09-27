import copy
import json
import unittest
from restore_maharaka_missing_material_slots import resolve_surface, append_preserving_text


class ExactMaterialRestorationTests(unittest.TestCase):
    def setUp(self):
        self.row = dict(assetId='first', materialName='SLOT_000_wood', sourceMaterial='original.mat.wood',
                        family='bg-source-opaque-masked', renderMode='deferred', diffuseBrightness=1,
                        bakedLighting={'averageTexture':'placement-specific'})

    def test_full_object_identity_not_leaf_name(self):
        self.assertIsNone(resolve_surface('other.mat.wood', [self.row])[0])

    def test_component_lighting_is_never_copied(self):
        original = copy.deepcopy(self.row)
        value, reason = resolve_surface('ORIGINAL.MAT.WOOD', [self.row])
        self.assertIsNone(reason)
        self.assertNotIn('bakedLighting', value)
        self.assertNotIn('assetId', value)
        self.assertEqual(self.row, original)

    def test_conflicting_user_tuning_is_not_silently_selected(self):
        other = dict(self.row, diffuseBrightness=2)
        self.assertEqual(resolve_surface('original.mat.wood', [self.row,other])[1], 'CONFLICTING_SURFACE_VALUES_FOR_SAME_SOURCE')

    def test_water_and_vertex_deformation_need_separate_admission(self):
        for row in (dict(self.row, family='source.map.water-42.v1'), dict(self.row,sourceFoliageWind={'program':'wind'})):
            self.assertEqual(resolve_surface('original.mat.wood', [row])[1], 'REQUIRES_SEPARATE_RENDER_OR_VERTEX_CONTRACT')

    def test_append_preserves_unrelated_text_and_existing_rows(self):
        before = b'{\r\n "materials": [\r\n  {"special":1.000}\r\n ],\r\n "untouched": [1,  2]\r\n}\r\n'
        after = append_preserving_text(before, [self.row])
        self.assertIn(b'{"special":1.000}', after)
        self.assertTrue(after.endswith(b'\r\n ],\r\n "untouched": [1,  2]\r\n}\r\n'))
        self.assertEqual(json.loads(after)['materials'], [{'special':1}, self.row])
        self.assertEqual(append_preserving_text(before, []), before)


if __name__ == '__main__':
    unittest.main()
