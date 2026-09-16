from pathlib import Path
import copy
import json
import struct
import tempfile
import unittest
from unittest.mock import patch

import build_source_map_materials as builder


class SourceMapMaterialTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.resources = self.root / 'Resources'
        self.texture = self.resources / 'Map/source.dds'
        self.texture.parent.mkdir(parents=True)
        header = [0] * 31
        header[0], header[2], header[3], header[6] = 124, 4, 4, 3
        header[18], header[19], header[20], header[26] = 32, 4, int.from_bytes(b'DXT1', 'little'), 0x1000
        self.texture.write_bytes(b'DDS ' + struct.pack('<31I', *header) + b'x' * 24)
        self.source = 'source.material.zero'
        values = {key: 1.0 for key in (
            'diffuse_brightness', 'normal_intensity', 'reflection_contrast', 'metallic_intensity',
            'metallic_power', 'roughness_intensity', 'roughness_power', 'ao_intensity', 'ao_power',
            'specular_pbr_intensity', 'nonmetallic_brightness', 'metallic_brightness')}
        values.update(diffuse_brightness=0.0, diffuse_color=[1, 1, 1, 1], reflection_color=[1, 1, 1, 1])
        self.parameter = dict(format='lostark-source-map-material-parameters', formatVersion=1,
            failures=[], sources=[], materials={self.source: dict(sourceMaterial=self.source,
                terminal='original.graph.bg_base_pbr_opa', values=values,
                switches={'1.use_diffuse_to_albedo': True, '1.use_normalmap': True},
                textures={key: 'source.texture.dds' for key in (
                    'texture_diffuse', 'texture_normal', 'texture_detail_normal', 'texture_orm', 'texture_reflection')})})
        self.parameter_path = self.root / 'parameters.json'
        self.evidence = self.root / 'component.json'
        self.evidence.write_text('{"sourceComponent": "original.component"}')
        self.manifest = dict(format='lostark-source-map-material-input', formatVersion=1, areaId='AREA',
            evidence=[self.file_record(self.evidence)], textures=[dict(sourceObject='source.texture.dds',
                assetId='Map/source.dds', sha256=builder.digest(self.texture.read_bytes()),
                colorSpace='linear', mipCount=3, mipEvidence='source-chain')], slots=[dict(assetId='MODEL',
                materialName='SLOT', sourceMaterial=self.source, component=dict(
                    rendering=dict(castsShadow=True, renderMode='deferred', cullMode='back'),
                    lightingEvidence='source-absent', environmentEvidence='source-absent',
                    minimumRoughness=0.04, minimumRoughnessEvidence='project-authored'))])
        self.input = self.root / 'input.json'
        self.save()

    @staticmethod
    def file_record(path):
        return dict(path=str(path), sha256=builder.digest(path.read_bytes()))

    def save(self):
        self.parameter_path.write_bytes(builder.json_bytes(self.parameter))
        self.manifest['parameters'] = self.file_record(self.parameter_path)
        self.input.write_bytes(builder.json_bytes(self.manifest))

    def compile(self):
        self.save()
        return builder.compile_materials(self.input, self.resources)

    def test_explicit_zero_and_source_slot_are_preserved(self):
        output, receipt = self.compile()
        self.assertEqual(output['materials'][0]['diffuseBrightness'], 0.0)
        self.assertEqual(receipt['bindings'][0]['sourceMaterials'], [self.source])
        self.assertTrue(receipt['runtimeMaterialInputsComplete'])
        self.assertFalse(receipt['originalVisualFidelityVerified'])
        self.assertEqual(receipt['resources'][0]['mipCount'], 3)

    def test_unknown_source_parameter_does_not_receive_category_coverage(self):
        self.parameter['materials'][self.source]['values']['unknown_parameter'] = 42
        _, receipt = self.compile()
        fields = receipt['bindings'][0]['materialCoverage'][0]['coveredSourceOnly']
        self.assertIn('scalar:diffuse_brightness', fields)
        self.assertNotIn('scalar:unknown_parameter', fields)
        self.assertNotIn('coveredSourceOnly', receipt['bindings'][0])

    def test_unimplemented_pbr_normal_and_emissive_permutations_fail(self):
        switches = self.parameter['materials'][self.source]['switches']
        switches['1.use_normalmap'] = False
        with self.assertRaisesRegex(ValueError, 'without normalmap'):
            self.compile()
        switches['1.use_normalmap'] = True
        switches['1.use_emissive'] = True
        with self.assertRaisesRegex(ValueError, 'NULL or unresolved'):
            self.compile()

    def test_steady_pbr_emission_and_exact_metallic_mask_fold(self):
        material=self.parameter['materials'][self.source]
        material['switches'].update({'1.use_emissive':True,'1.use_diffuse_metallicmask':True})
        material['textures']['texture_emissive']='source.texture.dds'
        material['values'].update(emissive_color=[1,1,1,1],emissive_intensity=.01,
            emissive_uv_tiling=[1,1,1,1],metallicmask_diffuse_color=[1,1,1,1],
            metallicmask_diffuse_brightness=1.2)
        row=self.compile()[0]['materials'][0]
        self.assertEqual(row['emissive']['flicker'],dict(minimum=0,speed=0,phaseOffset=0,mode='none'))
        self.assertEqual((row['diffuseBrightness'],row['nonmetallicBrightness'],row['metallicBrightness']), (1,0,1.2))
        material['values']['metallicmask_diffuse_color']=[.5,1,1,1]
        with self.assertRaisesRegex(ValueError,'cannot be folded exactly'): self.compile()
        material['values']['metallicmask_diffuse_color']=[1,1,1,1]
        material['values']['metallic_brightness']=2
        with self.assertRaisesRegex(ValueError,'cannot be folded exactly'): self.compile()

    def test_invalid_scalar_bounds_and_normal_colorspace_fail(self):
        self.manifest['slots'][0]['component']['minimumRoughness'] = -1
        with self.assertRaisesRegex(ValueError, 'non-negative'):
            self.compile()
        self.manifest['slots'][0]['component']['minimumRoughness'] = 0.04
        self.manifest['textures'][0]['colorSpace'] = 'srgb'
        with self.assertRaisesRegex(ValueError, 'normal must be linear'):
            self.compile()

    def test_missing_source_scalar_and_unknown_active_branch_fail(self):
        values = self.parameter['materials'][self.source]['values']
        del values['diffuse_brightness']
        with self.assertRaises(KeyError):
            self.compile()
        values['diffuse_brightness'] = 0
        self.parameter['materials'][self.source]['switches']['unknown_native_branch'] = True
        with self.assertRaisesRegex(ValueError, 'unsupported PBR switches'):
            self.compile()

    def test_null_texture_requires_matching_native_expression(self):
        self.parameter['materials'][self.source]['textures']['texture_orm'] = None
        with self.assertRaisesRegex(ValueError, 'NULL or unresolved'):
            self.compile()
        native = self.root / 'native.json'
        native.write_bytes(builder.json_bytes(dict(sourceMaterial=self.source,
            textures=[dict(expressionIndex=4, parameterName='texture_orm', sourceObject='source.texture.dds')])))
        self.manifest['slots'][0]['textureFallbacks'] = dict(texture_orm=dict(
            sourceObject='source.texture.dds', expressionIndex=4, evidence=self.file_record(native)))
        self.assertEqual(self.compile()[0]['materials'][0]['ormTexture'], 'Map/source.dds')
        self.manifest['slots'][0]['textureFallbacks']['texture_orm']['expressionIndex'] = 5
        with self.assertRaisesRegex(ValueError, 'does not match source expression'):
            self.compile()

    def test_mip_count_truncation_hash_and_path_fail(self):
        original = copy.deepcopy(self.manifest)
        self.manifest['textures'][0]['mipCount'] = 1
        with self.assertRaisesRegex(ValueError, 'mip count'):
            self.compile()
        self.manifest = copy.deepcopy(original)
        self.texture.write_bytes(self.texture.read_bytes()[:-1])
        with self.assertRaisesRegex(ValueError, 'hash mismatch'):
            self.compile()
        self.manifest['textures'][0]['sha256'] = builder.digest(self.texture.read_bytes())
        with self.assertRaisesRegex(ValueError, 'DDS payload'):
            self.compile()
        self.manifest = copy.deepcopy(original)
        self.manifest['textures'][0]['assetId'] = '../outside.dds'
        with self.assertRaisesRegex(ValueError, 'Resources-relative'):
            self.compile()

    def test_evidence_and_unbound_environment_fail(self):
        self.evidence.write_text('{}')
        with self.assertRaisesRegex(ValueError, 'evidence hash mismatch'):
            self.compile()
        self.manifest['evidence'] = [self.file_record(self.evidence)]
        component = self.manifest['slots'][0]['component']
        component['environmentEvidence'] = 'source-bound'
        with self.assertRaisesRegex(ValueError, 'environment evidence/binding mismatch'):
            self.compile()

    def test_rnm_instances_remain_distinct_and_duplicate_fails(self):
        component = self.manifest['slots'][0]['component']
        component.update(lightingEvidence='source-bound', bakedLighting=dict(
            averageTexture='Map/source.dds', directionalTexture='Map/source.dds', colorSpace='linear'))
        instance = dict(sourcePlacementId='AREA:export:1', assetId='MODEL', coordinateScale=[1, 1],
                        coordinateBias=[0, 0], averageScale=[1, 1, 1], directionalScale=[1, 1, 1])
        second = dict(instance, sourcePlacementId='AREA:export:2', coordinateBias=[0.25, 0.5])
        self.manifest['placementLighting'] = [instance, second]
        output, _ = self.compile()
        self.assertNotEqual(output['placementLighting'][0]['coordinateBias'],
                            output['placementLighting'][1]['coordinateBias'])
        self.manifest['placementLighting'].append(instance)
        with self.assertRaisesRegex(ValueError, 'duplicate/unbound'):
            self.compile()

    def test_cli_failure_preserves_previous_output_and_receipt(self):
        output, receipt = self.root / 'output.json', self.root / 'receipt.json'
        output.write_bytes(b'previous output')
        receipt.write_bytes(b'previous receipt')
        self.parameter['materials'][self.source]['switches']['unsupported'] = True
        self.save()
        result = builder.main(['--input', str(self.input), '--resources-root', str(self.resources),
                               '--output', str(output), '--receipt', str(receipt)])
        self.assertEqual(result, 1)
        self.assertEqual(output.read_bytes(), b'previous output')
        self.assertEqual(receipt.read_bytes(), b'previous receipt')

    def test_second_replace_failure_rolls_back_first_output(self):
        output, receipt = self.root / 'output.json', self.root / 'receipt.json'
        output.write_bytes(b'old')
        receipt.write_bytes(b'old receipt')
        real_replace = builder.os.replace
        def replace(source, target):
            if target == receipt:
                raise OSError('injected promotion failure')
            return real_replace(source, target)
        with patch.object(builder.os, 'replace', side_effect=replace):
            with self.assertRaisesRegex(OSError, 'injected'):
                builder.write_pair(output, b'new', receipt, b'new receipt')
        self.assertEqual(output.read_bytes(), b'old')
        self.assertEqual(receipt.read_bytes(), b'old receipt')

    def test_component_lighting_reader_output_is_joined_by_source_id(self):
        component = self.manifest['slots'][0]['component']
        component.update(lightingEvidence='source-bound', bakedLighting=dict(
            averageTexture='Map/source.dds', directionalTexture='Map/source.dds', colorSpace='linear'))
        instance = dict(sourcePlacementId='AREA:export:1', assetId='MODEL', coordinateScale=[1, 1],
                        coordinateBias=[0, 0], averageScale=[1, 1, 1], directionalScale=[1, 1, 1])
        self.manifest['placementLighting'] = [instance]
        self.manifest['auxiliaryTextures'] = [copy.deepcopy(self.manifest['textures'][0])]
        source = dict(format='lostark-source-map-component-lighting', formatVersion=1, areaId='AREA',
            source=self.file_record(self.evidence), failures=[], components={instance['sourcePlacementId']: dict(
                status='RNM_TEXTURE_LIGHTMAP', lighting=dict(instance, nativeTailCompletelyConsumed=True,
                    averageTexture='source.texture.dds', directionalTexture='source.texture.dds'))})
        path = self.root / 'lighting.json'
        path.write_bytes(builder.json_bytes(source))
        self.manifest['componentLighting'] = self.file_record(path)
        self.assertEqual(len(self.compile()[0]['placementLighting']), 1)
        instance['coordinateBias'] = [0.5, 0]
        with self.assertRaisesRegex(ValueError, 'coordinate/scale source mismatch'):
            self.compile()


if __name__ == '__main__':
    unittest.main()
