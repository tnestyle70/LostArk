from pathlib import Path
import base64
import hashlib
import json
import struct
import tempfile
import unittest
from unittest import mock
import xml.etree.ElementTree as ET

import build_valtan_full_restore as builder


class ValtanFullRestoreAnimationMetadataTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.root_patch = mock.patch.object(builder, 'ROOT', self.root)
        self.root_patch.start()
        self.addCleanup(self.root_patch.stop)
        self.addCleanup(self.temp.cleanup)
        for suffix in ('', '.filters'):
            path = self.root / ('Client/Default/Client.vcxproj' + suffix)
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b'<Project>\r\n  <ItemGroup><None Include="preserve.json" /></ItemGroup>\r\n</Project>\r\n')

    def install_fixture(self, action=420624, stage=7, clip='Att_Battle_19_01', seconds=5):
        asset = f'effect.valtan.action.{action}.stage{stage:03d}.full.restore'
        path = self.root / 'Data/Effects/Authored' / (asset + '.effect.json')
        path.parent.mkdir(parents=True, exist_ok=True)
        # Formatting and unknown edited content must survive metadata installation.
        path.write_text(json.dumps(dict(effectAssetId=asset, userEdited='keep'), indent=4), encoding='utf8')
        return path, dict(effectAssetId=asset, sourceActionId=action, sourceStageIndex=stage,
                         originalAnimationClips=[dict(clipName=clip, lengthSeconds=seconds, loop=False,
                             previewWallMs=round(seconds * 1000), previewWallBasis='SOURCE_ANIMATION_WINDOW')])

    def test_installed_source_metadata_preserves_effect_and_project_edits(self):
        path, row = self.install_fixture(400440, 0, 'Att_Battle_11_01', 1.7999999523162842)
        before = path.read_bytes()
        count = builder.install_animation_metadata([row], self.root / 'evidence')
        self.assertEqual(count, 1)
        self.assertEqual(path.read_bytes(), before)
        result = json.loads((self.root / builder.ANIMATIONS).read_bytes())
        self.assertEqual(result['effects'][0]['animationClips'], [dict(clipName='mesh_att_battle_11_01', playMs=1800, loop=False,
            previewWallMs=1800, previewWallBasis='SOURCE_ANIMATION_WINDOW')])
        for suffix in ('', '.filters'):
            xml = (self.root / ('Client/Default/Client.vcxproj' + suffix)).read_bytes()
            self.assertIn(b'Include="preserve.json"', xml)
            self.assertEqual(xml.count(b'Include="..\\..\\Data\\Effects\\ValtanFullRestoreAnimations.json"'), 1)
            ET.fromstring(xml)
        builder.install_animation_metadata([row], self.root / 'evidence')
        self.assertEqual(path.read_bytes(), before)

    def test_existing_mappings_survive_incremental_stage_install(self):
        _, first = self.install_fixture()
        builder.install_animation_metadata([first], self.root / 'evidence')
        _, second = self.install_fixture(420603, 2, 'Att_Battle_5_01_Loop', .8999999761581421)
        write = builder.animation_metadata_write([second], [second['effectAssetId']])
        builder.commit_writes([write])
        rows = json.loads((self.root / builder.ANIMATIONS).read_bytes())['effects']
        self.assertEqual({row['effectAssetId'] for row in rows}, {first['effectAssetId'], second['effectAssetId']})

    def test_ambiguous_clip_or_changed_existing_mapping_rejects_without_writes(self):
        _, row = self.install_fixture()
        builder.install_animation_metadata([row], self.root / 'evidence')
        path = self.root / builder.ANIMATIONS
        before = path.read_bytes()
        row['originalAnimationClips'][0]['lengthSeconds'] = 4
        with self.assertRaisesRegex(AssertionError, 'Preserve existing animation'):
            builder.animation_metadata_write([row], [row['effectAssetId']])
        row['originalAnimationClips'].append(dict(clipName='Other', lengthSeconds=1))
        with self.assertRaisesRegex(AssertionError, 'Expected one original'):
            builder.animation_metadata_write([row], [row['effectAssetId']])
        self.assertEqual(path.read_bytes(), before)

    def test_replace_failure_restores_prior_files_and_removes_new_mapping(self):
        _, row = self.install_fixture()
        projects = {path: path.read_bytes() for path in (self.root / 'Client/Default').iterdir()}
        actual_replace = builder.os.replace
        calls = 0

        def fail_third_replace(source, target):
            nonlocal calls
            calls += 1
            if calls == 3:
                raise OSError('injected project install failure')
            return actual_replace(source, target)

        with mock.patch.object(builder.os, 'replace', side_effect=fail_third_replace):
            with self.assertRaisesRegex(OSError, 'injected'):
                builder.install_animation_metadata([row], self.root / 'evidence')
        self.assertFalse((self.root / builder.ANIMATIONS).exists())
        for path, before in projects.items():
            self.assertEqual(path.read_bytes(), before)
        self.assertFalse(list(self.root.rglob('*.staged')))

    def test_concurrent_project_edit_rejects_before_metadata_replacement(self):
        _, row = self.install_fixture()
        writes = [builder.animation_metadata_write([row], [row['effectAssetId']])] + builder.project_writes([], True)
        changed = self.root / 'Client/Default/Client.vcxproj'
        changed.write_bytes(b'<Project><!--another editor--></Project>')
        with self.assertRaisesRegex(AssertionError, 'Concurrent edit'):
            builder.commit_writes(writes)
        self.assertFalse((self.root / builder.ANIMATIONS).exists())
        self.assertIn(b'another editor', changed.read_bytes())

    def test_missing_loop_field_is_upgraded_without_replacing_existing_values(self):
        path, row = self.install_fixture()
        authored_before = path.read_bytes()
        builder.install_animation_metadata([row], self.root / 'evidence')
        metadata_path = self.root / builder.ANIMATIONS
        metadata = json.loads(metadata_path.read_bytes())
        del metadata['effects'][0]['animationClips'][0]['loop']
        metadata_path.write_bytes(builder.encode_json(metadata))
        builder.install_animation_metadata([row], self.root / 'evidence')
        self.assertIs(json.loads(metadata_path.read_bytes())['effects'][0]['animationClips'][0]['loop'], False)
        self.assertEqual(path.read_bytes(), authored_before)
        row['originalAnimationClips'][0]['loop'] = True
        with self.assertRaisesRegex(AssertionError, 'Preserve existing animation'):
            builder.animation_metadata_write([row], [row['effectAssetId']])

    def test_missing_source_loop_is_rejected_instead_of_guessed_from_name(self):
        _, row = self.install_fixture(420603, 2, 'Att_Battle_5_01_Loop', 4)
        del row['originalAnimationClips'][0]['loop']
        with self.assertRaisesRegex(AssertionError, 'Missing decoded original'):
            builder.animation_metadata_write([row], [row['effectAssetId']])

    def test_stage_preview_uses_source_transition_not_particle_tail_or_clip_period(self):
        clip = dict(lengthSeconds=.833333, loop=True)
        stage = dict(notifies=[
            dict(sourceType='PlayParticleEffect', localTimeSeconds=1.5, durationSeconds=4.5),
            dict(sourceType='MonsterMoveNextStage', localTimeSeconds=3.9)])
        self.assertEqual(builder.source_stage_preview_timing(stage, clip),
            dict(previewWallMs=3900, previewWallBasis='SOURCE_UNCONDITIONAL_STAGE_TRANSITION'))
        stage['notifies'][-1]['localTimeSeconds'] = .5
        self.assertEqual(builder.source_stage_preview_timing(stage, clip)['previewWallMs'], 500)

    def test_conditional_check_is_explicit_preview_coverage_not_claimed_stage_end(self):
        stage = dict(notifies=[dict(sourceType='MonsterMoveNextStageConditionStatusEffect', localTimeSeconds=2.9)])
        self.assertEqual(builder.source_stage_preview_timing(stage, dict(lengthSeconds=.9)),
            dict(previewWallMs=2900, previewWallBasis='PREVIEW_COVERS_SOURCE_CONDITIONAL_CHECK'))
        self.assertEqual(builder.source_stage_preview_timing(dict(notifies=[]), dict(lengthSeconds=1.8)),
            dict(previewWallMs=1800, previewWallBasis='SOURCE_ANIMATION_WINDOW'))
        stage['notifies'][0]['localTimeSeconds'] = float('nan')
        with self.assertRaisesRegex(AssertionError, 'Invalid source stage'):
            builder.source_stage_preview_timing(stage, dict(lengthSeconds=.9))

    def test_missing_preview_budget_upgrade_preserves_existing_authored_values(self):
        path, row = self.install_fixture()
        before = path.read_bytes()
        builder.install_animation_metadata([row], self.root / 'evidence')
        metadata_path = self.root / builder.ANIMATIONS
        metadata = json.loads(metadata_path.read_bytes())
        for field in ('previewWallMs', 'previewWallBasis'):
            del metadata['effects'][0]['animationClips'][0][field]
        metadata_path.write_bytes(builder.encode_json(metadata))
        builder.install_animation_metadata([row], self.root / 'evidence')
        self.assertEqual(path.read_bytes(), before)
        self.assertEqual(json.loads(metadata_path.read_bytes())['effects'][0]['animationClips'][0]['previewWallMs'], 5000)
        row['originalAnimationClips'][0]['previewWallMs'] = 4900
        with self.assertRaisesRegex(AssertionError, 'Preserve existing animation'):
            builder.animation_metadata_write([row], [row['effectAssetId']])

    def test_verified_portal_bone_local_offsets_do_not_change_other_notifies(self):
        cue = dict(attachment=dict(mode='FOLLOW_NAMED_ANCHORS', sourceAnchorNames=['FX_Att_01'],
                   runtimeAnchors=[dict(runtimeBoneName='b_effectroot')]),
                   localTransform=dict(sourcePositionUeUnits=[-90, 66, 0], position=[-.9, 0, -.66],
                                       rotationDegrees=[0, 0, 0], scale=[1, 1, 1]))
        builder.apply_verified_portal_bone_local_position(cue, dict(notifyId='action-420630/stage-002/notify-008'))
        self.assertEqual(cue['localTransform']['position'], [-.9, 0, -.66])
        builder.apply_verified_portal_bone_local_position(cue, dict(notifyId='action-420624/stage-001/notify-001'))
        self.assertEqual(cue['localTransform']['position'], [-.9, .66, 0])
        cue['localTransform']['rotationDegrees'] = [0, 90, 0]
        with self.assertRaises(AssertionError):
            builder.apply_verified_portal_bone_local_position(cue, dict(notifyId='action-420624/stage-001/notify-001'))

    def test_original_anim_loop_flag_comes_from_verified_payload(self):
        # Original 420612/stage003 is looping despite having no _Loop suffix.
        clip = dict(clipName='Att_battle_13_03', notifyId='action-420612/stage-003/notify-000')
        payload = dict(byteSize=292, sha256='defb6e561bc50a202fd471a75f43f03208dc659f1a9fb3f7a35368934634a15d',
            data='Q0VGQWN0aW9uTm90aWZ5X0FuaW0AAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAACAPwAAAAABAAAAAAAAAAAAAAAFAAAAQW5pbQBkAAAAEQAAAEF0dF9iYXR0bGVfMTNfMDMAAAAAAAUAAABOb25lAAAAAAAFAAAATm9uZQAAAAAAAAAAAAAAgD/NzEw+zczMPQEAAAAAAAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAUAAABOb25lAAUAAABOb25lABEAAABBdHRfYmF0dGxlXzEzXzAzAAAAAAAAAAAAIwAAAA==')
        notify = dict(sourceType='Anim', notifyId=clip['notifyId'], serializedPayload=payload)
        self.assertIs(builder.original_animation_loop(clip, notify), True)
        payload['sha256'] = '0' * 64
        with self.assertRaises(AssertionError):
            builder.original_animation_loop(clip, notify)

    def test_four_direction_reads_signed_rotator_separately_and_preserves_position(self):
        raw = bytearray(100)
        struct.pack_into('<3i', raw, 40, 0, -16384, 0)
        payload = dict(data=base64.b64encode(raw).decode('ascii'), byteSize=len(raw),
            sha256=hashlib.sha256(raw).hexdigest())
        cue = dict(enabled=True, attachment=dict(mode='SNAPSHOT_ROOT'), sourceTransformByteOffset=0,
            sourceParameterCountByteOffset=88,
            localTransform=dict(position=[2.75, 0, -.5], rotationDegrees=[0, 0, 0]))
        notify = dict(notifyId='action-420624/stage-006/notify-033', sourceType='PlayParticleEffect', serializedPayload=payload)
        builder.apply_verified_four_direction_rotator(cue, notify)
        self.assertEqual(cue['localTransform']['rotationDegrees'], [0, 0, 0])
        notify['notifyId'] = 'action-420624/stage-007/notify-033'
        builder.apply_verified_four_direction_rotator(cue, notify)
        self.assertEqual(cue['localTransform']['rotationDegrees'], [0, -90, 0])
        self.assertEqual(cue['localTransform']['position'], [2.75, 0, -.5])
        payload['sha256'] = '0' * 64
        with self.assertRaises(AssertionError):
            builder.apply_verified_four_direction_rotator(cue, notify)


if __name__ == '__main__':
    unittest.main()
