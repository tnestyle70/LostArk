"""Focused data/clock checks; these do not claim a rendered visual pass."""
import copy
import json
import unittest
from pathlib import Path
from trim_encore_lead_in import ROOT, AREA, CAMERA, WORLD, GLASS, trim_keys, trim_pattern, replace_row


def read(path):
    return json.loads((ROOT / path).read_bytes())


class EncoreTrimTests(unittest.TestCase):
    def test_byte_preservation(self):
        raw = b'{"revision":1,"patterns":[{"id":"other","v":1.00000000001}, {"id":"target","v":4}],"route":[7,2]}'
        changed = replace_row(raw, 'patterns', 'id', 'target', lambda x: x.update(v=3))
        self.assertEqual(changed.replace(b'"v":3', b'"v":4'), raw)

    def test_reject_ambiguous_id(self):
        with self.assertRaises(AssertionError):
            replace_row(b'{"rows":[{"id":"x"},{"id":"x"}]}', 'rows', 'id', 'x', lambda x: None)

    def test_reject_nonconstant_cut(self):
        with self.assertRaises(AssertionError):
            trim_keys([{'timeMs': 0, 'value': 0}, {'timeMs': 6000, 'value': 1}])

    def test_saved_sequences_and_sounds(self):
        for path, pattern_id in [
            ('Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json', 'KAKULSAYDON_G1_PATTERN_10'),
            ('Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json', 'KAKULSAYDON_G1_PATTERN_97')]:
            document = read(path)
            pattern = next(p for p in document['patterns'] if p['patternId'] == pattern_id)
            self.assertEqual(pattern['durationMs'], 21322)
            self.assertEqual(pattern['stages'][0]['durationMs'], 18333)
            boxes = {b['resourceId']: b for b in pattern['presentationOccurrences']}
            self.assertEqual(boxes['sound.kouku.140ac8e3578ea5f47409']['soundSourceStartMs'], 2900)
            self.assertEqual(boxes['sound.kouku.7fccf417eb9300dcdbdf']['soundSourceStartMs'], 5000)
            self.assertNotIn('sound.kouku.696b5ad453c3fd2116d7', boxes)
            self.assertEqual(boxes['subtitle.kouku.cin.37081_12_01']['startMs'], 5233)
            self.assertEqual(boxes['subtitle.kouku.cin.37081_12_02']['startMs'], 7367)
            self.assertEqual(boxes['subtitle.kouku.cin.37081_12_03']['startMs'], 11700)
            for box in pattern['presentationOccurrences'] + pattern['worldOccurrences']:
                self.assertLessEqual(box['startMs'] + box['durationMs'], pattern['durationMs'])

    def test_world_camera_and_shatter_share_clock(self):
        world = next(t for t in read(AREA + '.worldsequences.json')['templates'] if t['sequenceId'] == WORLD)
        shot = next(s for s in read(AREA + '.camerashots.json')['shots'] if s['shotId'] == CAMERA)
        self.assertEqual(world['durationMs'], 18333)
        self.assertEqual(shot['cameraTrack']['durationMs'], 18333)
        self.assertEqual(world['animationTracks'][0]['sourceStartMs'], 5000)
        self.assertEqual(len(world['effectTracks']), 14)
        self.assertEqual(sorted({e['startMs'] for e in world['effectTracks']}), [7500, 10433])
        self.assertEqual(shot['cameraTrack']['keyframes'][0]['timeMs'], 0)
        self.assertEqual(world['tracks'][0]['keys'][0]['timeMs'], 0)

    def test_native_glass_source_clock_and_resources(self):
        e = read('Data/Effects/Authored/' + GLASS + '.effect.json')['elements'][0]
        self.assertAlmostEqual(e['detail']['timing']['startDelaySeconds'], 7.5, places=5)
        self.assertEqual(e['sourceTransformTrack']['sourceTimeOriginSeconds'], 5)
        self.assertEqual(e['material']['sourceProfile']['runtimeShaderProfileId'], 'effect.ue3.kouku-2627-native.v1')
        for texture in e['material']['sourceProfile']['textures']:
            self.assertTrue((ROOT / 'Client/Bin/Resources' / texture['assetId']).is_file(), texture['assetId'])
        keys = e['sourceTransformTrack']['materialParameterTracks'][0]['keys']
        self.assertAlmostEqual(keys[1]['timeSeconds'] - 5, 7.5, places=5)
        self.assertAlmostEqual(keys[-1]['timeSeconds'] - 5, 10.433334, places=5)

    def test_clear_ui_and_caption_are_in_pre_post_pass(self):
        timing = read('Data/UI/RaidClear/RaidClear_Kouku_Layout.json')['encorePresentation']
        self.assertEqual(timing, {'sourceStartMs': 5000, 'sourceHideMs': 15433})
        for clock, expected in [(0, True), (3000, True), (7500, True), (10432, True), (10433, False)]:
            self.assertEqual(clock + timing['sourceStartMs'] < timing['sourceHideMs'], expected)
        ui = read('Data/UI/RaidClear/RaidClear_Kouku.keyframes.json')
        visible = []
        for layer in ui['layers']:
            keys = [k for k in layer['keyframes'] if k['frame'] <= 281]
            if keys and keys[-1].get('asset') and keys[-1].get('alpha', 1) > 0:
                visible.append(keys[-1])
                self.assertTrue((ROOT / 'Client/Bin/Resources' / keys[-1]['asset']).is_file())
        self.assertGreater(len(visible), 8)
        renderer = (ROOT / 'Engine/Private/Renderer.cpp').read_text(encoding='utf-8-sig')
        self.assertLess(renderer.index('object->Render_Group(RENDERGROUP::SCENE_UI)'),
                        renderer.index('End_MRT_SceneHDR'))


if __name__ == '__main__':
    unittest.main()
