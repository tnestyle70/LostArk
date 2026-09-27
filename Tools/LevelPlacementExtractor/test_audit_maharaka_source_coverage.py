"""Inventory is evidence, never blanket admission of every source export."""
import unittest
import audit_maharaka_source_coverage as subject


def prop(value):
    return {'value': value}


class SourceCoverageTests(unittest.TestCase):
    def test_worldinfo_closure_excludes_orphan_streaming_objects(self):
        objects = [dict(packageIndex=1, className='worldinfo', properties={'streaminglevels': prop([2])}),
                   dict(packageIndex=2, className='eflevelstreamingalwaysloaded', properties={'packagename': prop('scene04a')}),
                   dict(packageIndex=3, className='levelstreamingalwaysloaded', properties={'packagename': prop('unreferenced')})]
        self.assertEqual([r['logicalPackage'] for r in subject.referenced_streaming_levels(objects)], ['SCENE04A'])
        objects[0]['properties']['streaminglevels'] = prop([9])
        with self.assertRaises(ValueError):
            subject.referenced_streaming_levels(objects)

    def test_duplicate_references_are_rejected(self):
        with self.assertRaises(ValueError):
            subject.referenced_streaming_levels([dict(packageIndex=1, className='worldinfo', properties={'streaminglevels': prop([2, 2])})])

    def test_null_override_is_not_a_missing_material_or_a_match(self):
        overrides = {'slots': [dict(slot=0, packageIndex=0, objectPath=None)]}
        result = subject.compare_material_overrides(overrides, [])
        self.assertEqual(result[0]['status'], 'NULL_OVERRIDE_REQUIRES_PARENT_MATERIAL_RESOLUTION')

    def test_slot_and_full_reference_must_both_match(self):
        overrides = {'slots': [dict(slot=0, packageIndex=-4, objectPath='a.mat.wood')]}
        material = dict(materialName='SLOT_000_wood', sourceMaterial='b.mat.wood')
        self.assertEqual(subject.compare_material_overrides(overrides, [material])[0]['status'], 'SOURCE_MATERIAL_DIFFERENCE')
        material['sourceMaterial'] = 'A.MAT.WOOD'
        self.assertEqual(subject.compare_material_overrides(overrides, [material])[0]['status'], 'SOURCE_MATERIAL_REFERENCE_MATCH')
        material['materialName'] = 'SLOT_001_wood'
        self.assertEqual(subject.compare_material_overrides(overrides, [material])[0]['status'], 'SLOT_IDENTITY_UNRESOLVED')

    def test_attachment_retains_bone_and_relative_transform(self):
        data = {'base': prop(83), 'basebonename': prop('bip001'), 'relativelocation': prop({'x': 1}), 'tag': prop('ignored')}
        self.assertEqual(subject.attachment(data), {'base': 83, 'basebonename': 'bip001', 'relativelocation': {'x': 1}})


if __name__ == '__main__':
    unittest.main()
