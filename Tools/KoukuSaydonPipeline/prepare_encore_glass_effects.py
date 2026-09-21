"""Prepare SCENE07A's twelve shards and two sparks on the existing Encore actor.

Writes candidates and stable field patches only. Current actor/camera authoring is
input, never output. Native particle programs/materials come from installed V1.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import sys
from pathlib import Path

import numpy as np
from scipy.spatial.transform import Rotation, Slerp

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(Path(__file__).resolve().parent), str(ROOT / 'Tools/EffectPipeline')]
import build_gate2_intro_composition as base
import build_kouku_action_effect_groups as action
import build_kouku_sequence_effect_groups as groups

AREA = 'LV_LUT_MIDNIGHTC_ED'
WORLD = ROOT / 'Data/Maps/Authoring' / AREA / (AREA + '.worldsequences.json')
CAMERA = WORLD.with_name(AREA + '.camerashots.json')
TEMPLATE = 'sequence.kouku.bingo.encore.saydon'
INSTANCE = 'world.sequence.instance.kouku.bingo.encore.saydon'
SOURCE = ROOT / 'out/KoukuBingoGlass20260920'
OUT = ROOT / 'out/KoukuEncoreGlass20260921'


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def trigger_intervals(keys):
    """Trigger is finite source emission; only actual OFF is an emission bound."""
    out = []
    for i, key in enumerate(keys):
        if key['toggleaction'] == 'etta_off':
            continue
        assert key['toggleaction'] == 'etta_trigger', ('unexpected Toggle action', key)
        stop = next((k['time'] for k in keys[i + 1:] if k['toggleaction'] == 'etta_off'), None)
        out.append((key['time'], stop))
    assert out
    return out


def rotation_degrees(matrix):
    # DirectX XMMatrixRotationRollPitchYaw: column form Ry(yaw) Rx(pitch) Rz(roll).
    yaw, pitch, roll = Rotation.from_matrix(matrix).as_euler('YXZ', degrees=True)
    reconstructed = Rotation.from_euler('YXZ', [yaw, pitch, roll], degrees=True).as_matrix()
    assert np.max(np.abs(reconstructed - matrix)) < 1e-10
    return [float(pitch), float(yaw), float(roll)]


def sample_actor(keys, milliseconds):
    if milliseconds <= keys[0]['timeMs']:
        return np.array(keys[0]['positionOffset']), Rotation.from_quat(keys[0]['rotationQuaternion']).as_matrix()
    for left, right in zip(keys, keys[1:]):
        if milliseconds > right['timeMs']:
            continue
        ratio = (milliseconds - left['timeMs']) / (right['timeMs'] - left['timeMs'])
        position = np.array(left['positionOffset']) * (1-ratio) + np.array(right['positionOffset']) * ratio
        rotation = Slerp([0, 1], Rotation.from_quat([left['rotationQuaternion'], right['rotationQuaternion']]))([ratio])[0]
        return position, rotation.as_matrix()
    return np.array(keys[-1]['positionOffset']), Rotation.from_quat(keys[-1]['rotationQuaternion']).as_matrix()


def sample_camera(keys, milliseconds):
    if milliseconds <= keys[0]['timeMs']:
        return keys[0]
    for left, right in zip(keys, keys[1:]):
        if milliseconds > right['timeMs']:
            continue
        ratio = (milliseconds-left['timeMs']) / (right['timeMs']-left['timeMs'])
        return {key: np.array(left[key]) * (1-ratio) + np.array(right[key]) * ratio
                for key in ('eye', 'lookAt', 'up', 'fovYDegrees')}
    return keys[-1]


def project_point(point, camera):
    forward = np.array(camera['lookAt']) - camera['eye']; forward /= np.linalg.norm(forward)
    right = np.cross(camera['up'], forward); right /= np.linalg.norm(right)
    up = np.cross(forward, right)
    relative = point - camera['eye']
    depth = float(relative @ forward)
    height = math.tan(math.radians(float(camera['fovYDegrees'])) / 2) * depth
    return np.array([float(relative @ right) / (height * 16 / 9), float(relative @ up) / height]), depth


def duration_ms(document):
    return math.ceil(max(e['detail']['timing']['startDelaySeconds'] + e['detail']['timing']['lifeTimeSeconds']
                        + max(e['detail']['particle']['lifeTimeSeconds']) for e in document['elements']) * 1000)


def prepare(output=OUT):
    output = output.resolve()
    assert output.is_relative_to((ROOT / 'out').resolve()), 'Candidates must remain below repository out/'
    source_path = SOURCE / 'source-scene.json'
    tracks_path = SOURCE / 'scene-glass-tracks.json'
    source = read(source_path); rows = {int(k): v for k, v in source['rows'].items()}
    extracted = read(tracks_path)
    assert extracted['sourceSha256'] == digest(source_path)
    package_path = base.PACKAGES / 'B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8L6.upk'
    # The regular scene extractor decodes InterpLookupTrack in addition to the
    # transfer receipt's readable Move keys. Never infer lookup targets from a
    # truncated property hex preview or silently skip a dynamic lookup.
    decoded_rows, _ = base.extract_scene(package_path)
    world = read(WORLD); camera_document = read(CAMERA)
    template = next(t for t in world['templates'] if t['sequenceId'] == TEMPLATE)
    instance = next(i for i in world['instances'] if i['instanceId'] == INSTANCE)
    resource = next(r for r in world['objectResources'] if r['objectId'] == 'world.object.kouku.bingo.encore.saydon')
    assert instance['templateId'] == TEMPLATE and instance['anchorKind'] == 'WORLD'
    assert instance['position'] == [0, 0, 0] and instance['playbackSpeed'] == 1 and instance['startDelayMs'] == 0
    assert resource['scale'] == [1, 1, 1] and resource['modelPreScale'] == .017
    ratio = resource['modelPreScale'] / .01
    actor_keys = next(t['keys'] for t in template['tracks'] if t['slotId'] == 'actor')
    camera = next(s for s in camera_document['shots'] if s['shotId'] == 'kouku.bingo.encore.camera.1')
    assert rows[24]['p']['base'] == 3 and rows[3]['p']['base'] == 4
    assert not rows[24]['p'].get('drawscale') and not rows[24]['p'].get('drawscale3d')
    assert len(extracted['particleGroups']) == 14
    index = action.restored_index(ROOT / 'out/KoukuAllEffects20260912')
    inputs = {str(p.relative_to(ROOT)): digest(p) for p in (source_path, tracks_path, WORLD, CAMERA)}
    inputs[str(package_path)] = digest(package_path)
    effects, documents, catalog, evidence = [], [], [], []
    all_samples = []
    for group in extracted['particleGroups']:
        name = group['groupName']; props = group['actor']['p']
        assert props['base'] == 24 and not props.get('basebonename') and not props.get('bignorebaserotation')
        assert not group['component']['p'].get('instanceparameters')
        actor_id = next(i for i, row in rows.items() if row['name'] == group['actor']['name'])
        group_id = next(i for i in rows[44]['p']['interpgroups'] if rows[i]['p'].get('groupname') == name)
        assert base.group_actor(rows, group_id, 21) == [actor_id]
        move = next(t for t in group['tracks'] if t['cls'] == 'interptrackmove')
        assert move['p'].get('moveframe', 'imf_world') == 'imf_world'
        decoded_move = next(r for r in decoded_rows.values() if r['name'] == move['name'])
        assert decoded_move['p']['postrack'] == move['p']['postrack']
        assert decoded_move['p']['eulertrack'] == move['p']['eulertrack']
        lookup = decoded_move['p'].get('lookuptrack', {})
        assert 'hex' not in lookup
        assert all(p.get('groupname', 'none') in ('none', '') for p in lookup.get('points', []))
        position_ue = groups.constant_curve(move['p']['postrack'])
        euler_ue = groups.constant_curve(move['p']['eulertrack'])
        assert position_ue is not None and euler_ue is not None
        local_position = base.BASIS @ base.vec(position_ue) * .01
        local_rotation = base.BASIS @ base.rotation(base.vec(euler_ue)) @ base.BASIS.T
        intervals = trigger_intervals(next(t['p']['toggletrack'] for t in group['tracks'] if t['cls'] == 'interptracktoggle'))
        asset = 'effect.kouku.source.' + group['sourceSystem']
        source_effect = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
        inputs[str(source_effect.relative_to(ROOT))] = digest(source_effect)
        original = read(source_effect)
        start = round(intervals[0][0] * 1000)
        if name.startswith('spark'):
            assert len(intervals) == 3 and all(stop is not None for _, stop in intervals)
            asset = 'effect.kouku.bingo.encore.glass.' + name
            document = copy.deepcopy(original)
            document.update(effectAssetId=asset,
                            displayName='Encore glass / ' + name, elements=[])
            cue = dict(enabled=True, particleDataDecoded=True, parameterOverridesDecoded=True, parameterOverrides=[],
                       attachment=dict(mode='SNAPSHOT_ROOT', sourceAnchorNames=[], runtimeAnchors=[],
                                       runtimeAnchorSlotId='root', runtimeBoneName=''),
                       localTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1]))
            for ordinal, (birth, stop) in enumerate(intervals):
                notify = dict(notifyId='scene07a/' + name + '/activation-' + str(ordinal),
                              localTimeSeconds=birth-start*.001, durationSeconds=stop-birth)
                stream, parameters = action.instantiate(original, cue, notify, asset, extracted['scene'], index)
                for element in stream:
                    element['actionCueAttachment']['enabled'] = False
                    element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees', None)
                document['elements'] += stream
            relative_path = 'Data/Effects/Authored/' + asset + '.effect.json'
            write(output / 'candidate' / relative_path, document)
            documents.append(dict(path=relative_path, effectAssetId=asset, elementCount=len(document['elements'])))
            catalog.append(dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT',
                                authoringPath='Effects/Authored/' + asset + '.effect.json'))
        else:
            assert len(intervals) == 1 and intervals[0][1] is None
            assert all(e['sourceRecipe']['emitterLoopCount'] > 0 for e in original['elements'])
            document = original
        end = min(template['durationMs'], start + duration_ms(document))
        draw_scale = base.vec(props.get('drawscale3d'), (1, 1, 1)) * props.get('drawscale', 1)
        scale = [float(v * ratio) for v in draw_scale[[0, 2, 1]]]
        effect = dict(effectTrackId='effect.kouku.bingo.encore.glass.' + name, slotId='actor',
                      resourceKind='V1_EFFECT', resourceId=asset, timing='TIME', followObject=True,
                      inheritObjectRotation=True, bone='', startMs=start, durationMs=end-start,
                      fitEffectToDuration=False, loopEffectToDuration=False,
                      positionOffset=list(map(float, local_position*ratio)),
                      rotationDegrees=rotation_degrees(local_rotation), scale=scale)
        effects.append(effect)
        samples = []
        for milliseconds in sorted({start, end-1} | set(range(start, end, 17))):
            seconds = milliseconds*.001
            parent_position, parent_rotation = base.world_pose(rows, 51, 24, seconds, 21, 44)
            source_position, source_rotation = base.world_pose(rows, group_id, actor_id, seconds, 21, 44)
            assert np.max(np.abs(source_position-(parent_position+parent_rotation@local_position))) < 1e-9
            assert np.max(np.abs(source_rotation-parent_rotation@local_rotation)) < 1e-9
            current_position, current_rotation = sample_actor(actor_keys, milliseconds)
            actual_position = current_position + current_rotation @ (local_position * ratio)
            expected_position = parent_position + ratio * (source_position-parent_position)
            error = float(np.linalg.norm(actual_position-expected_position))
            current_camera = sample_camera(camera['cameraTrack']['keyframes'], milliseconds)
            source_camera = camera['cameraTrack']['keyframes'][0]
            source_ndc, source_depth = project_point(source_position, source_camera)
            current_ndc, current_depth = project_point(actual_position, current_camera)
            sample = dict(timeMs=milliseconds, sourceWorld=source_position.tolist(), world=actual_position.tolist(),
                          positionErrorMetres=error, ndcError=float(np.max(np.abs(source_ndc-current_ndc))),
                          sourceDepth=source_depth, currentDepth=current_depth)
            samples.append(sample); all_samples.append(sample)
        evidence.append(dict(groupName=name, sourceSystem=group['sourceSystem'], sourceActor=actor_id,
                             sourceGroup=group_id, parentChain=[actor_id, 24, 3, 4], intervals=intervals,
                             originalLocalUE3Cm=position_ue, originalEulerDegrees=euler_ue,
                             originalDrawScale=props.get('drawscale', 1), effectTrack=effect, samples=samples))
    existing = {e['effectTrackId']: e for e in template.get('effectTracks', [])}
    for effect in effects:
        assert effect['effectTrackId'] not in existing or existing[effect['effectTrackId']] == effect, 'Preserve edited glass track'
    candidate = copy.deepcopy(world)
    target = next(t for t in candidate['templates'] if t['sequenceId'] == TEMPLATE)
    target.setdefault('effectTracks', []).extend(e for e in effects if e['effectTrackId'] not in existing)
    candidate['revision'] += int(candidate != world)
    relative_world = WORLD.relative_to(ROOT)
    candidate_world = output / 'candidate' / relative_world
    candidate_world.parent.mkdir(parents=True, exist_ok=True)
    # World authoring is compact; pretty-printing thousands of baked track
    # keys can cross the runtime's 16 MiB document limit without adding data.
    candidate_world.write_text(json.dumps(candidate, ensure_ascii=False, separators=(',', ':'), allow_nan=False) + '\n', encoding='utf-8')
    # The native probe owns this exact object slice. Unrelated map/deploy
    # bindings require their full level catalogs and belong to owner publish.
    native_world = copy.deepcopy(candidate)
    native_world['templates'] = [copy.deepcopy(target)]
    native_world['instances'] = [copy.deepcopy(instance)]
    native_world['objectResources'] = [copy.deepcopy(resource)]
    write(output / 'native-world.json', native_world)
    composition_references = []
    for relative, pattern_id in (
            ('Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json', 'KAKULSAYDON_G1_PATTERN_97'),
            ('Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json', 'KAKULSAYDON_G1_PATTERN_10')):
        composition = read(ROOT / relative)
        pattern = next(p for p in composition['patterns'] if p['patternId'] == pattern_id)
        worlds = {w['worldId']: w for w in composition['worlds']}
        references = [o for o in pattern['worldOccurrences'] if worlds[o['worldId']]['sequenceInstanceId'] == INSTANCE]
        assert len(references) == 1, ('Existing Encore reference changed', relative)
        composition_references.append(dict(path=relative, patternId=pattern_id, instanceId=INSTANCE))
    patch = dict(worldPath=relative_world.as_posix(), templateId=TEMPLATE, effectTrackKey='effectTrackId',
                 effectTracks=effects, catalogPath='Data/Effects/EffectCatalog.json', catalogEntries=catalog,
                 documents=documents, compositionReferences=composition_references, inputs=inputs,
                 installPolicy='MERGE_STABLE_IDS_PRESERVE_UNRELATED_EDITS_CAS_BACKUP_ATOMIC')
    write(output / 'field-patch.json', patch)
    write(output / 'source-transform-evidence.json', evidence)
    report = dict(effectTrackCount=len(effects), sourceTriggerCount=sum(len(e['intervals']) for e in evidence),
                  candidateDocumentCount=len(documents), sceneScaleRatio=ratio, samples=len(all_samples),
                  maxPositionErrorMetres=max(s['positionErrorMetres'] for s in all_samples),
                  maxNdcError=max(s['ndcError'] for s in all_samples),
                  minCameraDepth=min(s['currentDepth'] for s in all_samples), liveDataChanged=False,
                  cameraAndActorChanged=False, visualStatus='USER_PENDING', inputs=inputs)
    assert report['maxPositionErrorMetres'] < .01, report
    # The actor passes the camera during the final charge; a spent emitter's
    # tail anchor may therefore cross behind it. Require source/runtime side
    # agreement, and positive depth at actual activations, not at dead tails.
    assert report['maxNdcError'] < .03, report
    assert all(s['sourceDepth'] * s['currentDepth'] > 0 for s in all_samples)
    for group in evidence:
        assert all(min(group['samples'], key=lambda s: abs(s['timeMs'] - birth*1000))['currentDepth'] > 0
                   for birth, _ in group['intervals'])
    write(output / 'receipt.json', report)
    print(json.dumps({k: v for k, v in report.items() if k != 'inputs'}), flush=True)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--output', type=Path, default=OUT)
    prepare(parser.parse_args().output)
