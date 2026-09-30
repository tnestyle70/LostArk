import copy
import unittest

from build_map_static_shadow_variant_set import build_shadow_variants
from build_map_rnm_variant_set import VariantSetError, build_variant_set
from test_build_map_rnm_variant_set import catalog_row, material_row, placement_row, rnm_row


class StaticShadowVariantsTests(unittest.TestCase):
    def inputs(self):
        catalogs = [catalog_row('MESH')]
        placements = [placement_row('p' + str(i), 'MESH') for i in range(3)]
        built = build_variant_set(catalog_rows=catalogs, placement_rows=placements,
            materials=[material_row('MESH', 'a'), material_row('MESH', 'b')],
            rnm=[rnm_row(p[1], 'MESH', 'P.avg', 'P.dir') for p in placements],
            lightmap_directory='Map/Lighting/AREA')
        return dict(catalog_rows=catalogs, placement_rows=placements,
            materials=built['materials'], placement_lighting=built['placementLighting'],
            shadows=[dict(sourcePlacementId='p' + str(i),
                staticShadow=dict(texture='Map/Lighting/AREA/s.dds', lightGuid='a'*32,
                    lightChannel=1, penumbraWidth=.05, penumbraBasis='PROJECT_ADAPTER', shadowExponent=2),
                shadowCoordinateScale=[.5, .5], shadowCoordinateBias=[i * .25, 0]) for i in range(2)])

    def test_shadowless_placement_does_not_inherit_majority_shadow(self):
        inputs = self.inputs()
        before = copy.deepcopy(inputs)
        result = build_shadow_variants(**inputs)
        self.assertEqual(inputs, before)
        self.assertEqual(result['addedVariants'], 1)
        self.assertEqual(result['boundShadowPlacements'], 2)
        ids = {row[1]: row[4] for row in result['placements']}
        self.assertEqual(ids['p0'], ids['p1'])
        self.assertNotEqual(ids['p0'], ids['p2'])
        for row in result['materials']:
            self.assertEqual('staticShadow' in row['bakedLighting'], row['assetId'] == ids['p0'])
        self.assertEqual(result['catalogs'][0][2], result['catalogs'][1][2])
        for row in result['placementLighting']:
            self.assertEqual(row['assetId'], ids[row['sourcePlacementId']])
            if row['sourcePlacementId'] == 'p1':
                self.assertEqual(row['shadowCoordinateBias'], [.25, 0])
            if row['sourcePlacementId'] == 'p2':
                self.assertNotIn('shadowCoordinateScale', row)

    def test_distinct_shadow_atlas_and_transfer_get_distinct_variants(self):
        inputs = self.inputs()
        inputs['shadows'][1]['staticShadow']['shadowExponent'] = 3
        result = build_shadow_variants(**inputs)
        self.assertEqual(result['addedVariants'], 2)
        self.assertEqual(len({p[4] for p in result['placements']}), 3)
        again = build_shadow_variants(**{**inputs, 'shadows': list(reversed(inputs['shadows']))})
        self.assertEqual(result, again)

    def test_missing_rnm_fails_without_changing_inputs(self):
        inputs = self.inputs()
        inputs['placement_lighting'] = inputs['placement_lighting'][1:]
        before = copy.deepcopy(inputs)
        with self.assertRaises(VariantSetError):
            build_shadow_variants(**inputs)
        self.assertEqual(inputs, before)

    def test_duplicate_source_shadow_is_rejected(self):
        inputs = self.inputs()
        inputs['shadows'].append(copy.deepcopy(inputs['shadows'][0]))
        with self.assertRaises(VariantSetError):
            build_shadow_variants(**inputs)


if __name__ == '__main__':
    unittest.main()
