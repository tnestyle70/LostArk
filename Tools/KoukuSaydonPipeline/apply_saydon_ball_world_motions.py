"""Add Saydon's rolling/eaten balls through the existing World Object path.

The rolling prop deliberately reuses the user's Mario StripedBall material.
Its measured pivot is cancelled by the existing rotation/revolution fields;
the source rolling rate, notify offset and installed boss bone scale are kept.
The eaten ball retains the original RH model and native material profile.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import sys

import apply_saydon_card_pattern_timing as patch

ROOT = patch.ROOT
WORLD = ROOT / 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json'
RESOURCES = ROOT / 'Client/Bin/Resources'


def geometry(asset):
    sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
    import cook_wmodel_geometry_contract as model
    payload = (RESOURCES / asset).read_bytes()
    try:
        vertices = [v['values'][:3] for m in model.parse_geometry_wmodel(payload)['submeshes'] for v in m['vertices']]
    except ValueError:
        vertices = [v[:3] for m in model.parse_legacy_wmodel(payload)[2] for v in m.vertices]
    low = [min(v[i] for v in vertices) * .01 for i in range(3)]
    high = [max(v[i] for v in vertices) * .01 for i in range(3)]
    return dict(vertexCount=len(vertices), centerM=[(a+b)/2 for a,b in zip(low, high)],
        halfExtentsM=[(b-a)/2 for a,b in zip(low, high)], sha256=hashlib.sha256(payload).hexdigest())


def bone_scale(bone, clip, seconds):
    sys.path.insert(0, str(ROOT))
    from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as composition
    pose = composition.wmodel_pose
    catalog = json.loads((ROOT / 'Data/Actors/BossCatalog.json').read_text(encoding='utf8'))
    boss = next(b for b in catalog['bosses'] if b['archetypeId'] == 'BOSS_KAKULSAYDON_G1_SAYDON')
    model = pose.read_wmodel(RESOURCES / boss['bodyModel'], include_geometry=False, animation_names={clip})
    actor = dict(body=model, bodyPre=pose.affine_matrix((boss['bodyModelPreScale'],)*3, (0,0,0,1), (0,0,0)),
        poses={}, validatedClips=set())
    index = [b.name for b in model.skeleton_bones].index(bone)
    matrix = composition._sample_bone_bake_pose(actor, 'body', clip, seconds)[index]
    scales = [math.sqrt(sum(matrix[r*4+c]**2 for c in range(3))) for r in range(3)]
    assert max(scales)-min(scales) < .00001
    return dict(modelAssetId=boss['bodyModel'], clip=clip, seconds=seconds, bone=bone,
        basisScale=scales, matrix=matrix)


def make_motion(name, label, object_id, duration, offset, rotation, angular, orbit):
    key = dict(timeMs=0, positionOffset=offset, rotationQuaternion=rotation,
        scaleMultiplier=[1,1,1], visible=True)
    return dict(sequenceId='sequence.kouku.saydon.' + name, displayName=label, category='WorldObject',
        durationMs=duration, interpolation='LINEAR', objectMotion=dict(velocity=[0,0,0], acceleration=[0,0,0],
            angularVelocityDegrees=angular, revolutionDegreesPerSecond=angular,
            revolutionOffset=orbit, count=1, intervalMs=0, spreadDegrees=0, seed=1),
        tracks=[dict(slotId='object', keys=[key, dict(key, timeMs=duration, visible=False)])], animationTracks=[]), \
        dict(instanceId='world.object.instance.kouku.saydon.' + name,
            templateId='sequence.kouku.saydon.' + name, enabled=True, startDelayMs=0, playbackSpeed=1,
            anchorKind='BOSS', position=[0,0,0], motionEnd='STOP', nextMotionId='',
            bindings=[dict(slotId='object', targetKind='OBJECT_RESOURCE', targetId=object_id)])


def apply(install=False):
    snapshots = {p: p.read_bytes() for p in (WORLD, patch.PATH)}
    text = snapshots[WORLD].decode('utf-8-sig')
    composition_text = snapshots[patch.PATH].decode('utf-8-sig')
    world = json.loads(text)
    composition = json.loads(composition_text)
    objects = {o['objectId']: o for o in world['objectResources']}
    roll = copy.deepcopy(objects['world.object.mario.striped_ball'])
    roll_id = 'world.object.kouku.saydon.rolling.ball'
    eat_id = 'world.object.kouku.saydon.eaten.ball'
    assert roll_id not in objects and eat_id not in objects
    pivot = geometry(roll['modelAssetId'])
    source_geometry = geometry('Effect/KoukuSaydon/FullRestore/Meshes/fm_k_ppct_ball_01.wmodel')
    assert pivot['vertexCount'] == source_geometry['vertexCount'] == 290
    assert max(abs(a-b) for a,b in zip(pivot['halfExtentsM'], source_geometry['halfExtentsM'])) < .0000001
    rig = bone_scale('b_root', 'rpct00_att_battle_26_01', 1.696)
    rig_scale = sum(rig['basisScale']) / 3
    scale = 1.100000023841858 * rig_scale
    # Source TypeData roll 90 maps to Client -X90. The source mesh rotation
    # rate is UE roll +1.05 turns/s, hence Client -X378 degrees/s.
    rotation = [-math.sqrt(.5), 0, 0, math.sqrt(.5)]
    angular = [-1.0499999523162842 * 360, 0, 0]
    center = pivot['centerM']
    rotated_center = [center[0]*scale, center[2]*scale, -center[1]*scale]
    # Original notify local -0.5m Y plus particle local [0,.1,-.2]m.
    desired = [0, -.4*rig_scale, -.2*rig_scale]
    offset = [d-c for d,c in zip(desired, rotated_center)]
    orbit = [-c for c in rotated_center]
    roll.update(objectId=roll_id, displayName='세이튼_공굴리기카운터_공', anchorKind='BOSS',
        anchorBossArchetypeId='BOSS_KAKULSAYDON_G1_SAYDON', anchorBone='b_root', scale=[scale]*3,
        defaultMotionInstanceId='world.object.instance.kouku.saydon.rolling.ball')
    roll_pattern = next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_81')
    roll_duration = sum(s['durationMs'] for s in roll_pattern['stages']) - 1696
    roll_motion, roll_instance = make_motion('rolling.ball', '공굴리기카운터_원작 회전', roll_id,
        roll_duration, offset, rotation, angular, orbit)
    eat = copy.deepcopy(objects['world.object.kouku.ball'])
    eat_rig = bone_scale('b_wp_2', 'rpct00_att_battle_17_01', 1.172)
    eat_scale = .800000011920929 * sum(eat_rig['basisScale']) / 3
    eat.update(objectId=eat_id, displayName='세이튼_작은공먹기', anchorKind='BOSS',
        anchorBossArchetypeId='BOSS_KAKULSAYDON_G1_SAYDON', anchorBone='b_wp_2', scale=[eat_scale]*3,
        defaultMotionInstanceId='world.object.instance.kouku.saydon.eaten.ball')
    eat_motion, eat_instance = make_motion('eaten.ball', '작은공먹기_손 따라가기', eat_id,
        1000, [0,0,0], [0,0,0,1], [0,0,0], [0,0,0])
    for key, values in [('objectResources',[roll,eat]), ('templates',[roll_motion,eat_motion]),
                        ('instances',[roll_instance,eat_instance])]:
        for value in values:
            text = patch.append_row(text, key, value)
    text = patch.replace_field(text, 'revision', world['revision'] + 1, 2)
    world_ordinal = composition['nextWorldOrdinal']
    for index, (pattern, obj, instance, start, duration) in enumerate([
            (roll_pattern, roll, roll_instance, 1696, roll_duration),
            (next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_79'),
             eat, eat_instance, 1172, 1000)]):
        identity = 'kakulsaydon.g1.world.' + str(world_ordinal + index)
        definition = dict(worldId=identity, displayName=obj['displayName'],
            objectResourceId=obj['objectId'], sequenceInstanceId=instance['instanceId'],
            positionOffset=[0,0,0], anchorKind='NONE', anchorPosition=[0,0,0], companionEffectResourceId='')
        composition_text = patch.append_row(composition_text, 'worlds', definition)
        ordinal = pattern['nextWorldOccurrenceOrdinal']
        assert not pattern['worldOccurrences']
        occurrence = dict(occurrenceId=pattern['patternId'] + '.world.' + str(ordinal),
            worldId=identity, startMs=start, durationMs=duration, playbackSpeed=1)
        composition_text = patch.replace_row_fields(composition_text, 'patterns', 'patternId', pattern['patternId'],
            dict(worldOccurrences=[occurrence], nextWorldOccurrenceOrdinal=ordinal+1))
    composition_text = patch.replace_field(composition_text, 'nextWorldOrdinal', world_ordinal+2, 2)
    composition_text = patch.replace_field(composition_text, 'revision', composition['revision']+1, 2)
    after = json.loads(text)
    for key, count in [('objectResources',2), ('templates',2), ('instances',2)]:
        assert after[key][:-count] == world[key]
    for asset in [roll['modelAssetId'], eat['modelAssetId'], eat['diffuseTextureAssetId']]:
        assert (RESOURCES / asset).is_file()
    # Closed-form centre cancellation: c*R0*R(t) + [d-cR0] +
    # [-cR0]*R(t)-[-cR0] = d for every rotation angle.
    assert all(abs(offset[i] - orbit[i] - desired[i]) < 1e-12 for i in range(3))
    for path, payload in [(WORLD,text), (patch.PATH,composition_text)]:
        (patch.OUTPUT / (path.stem + '.world-candidate.json')).write_text(payload, encoding='utf8')
    for path, before in snapshots.items():
        assert path.read_bytes() == before, f'Concurrent authoring edit: {path}'
    if install:
        WORLD.write_text(text, encoding='utf8', newline='')
        patch.PATH.write_text(composition_text, encoding='utf8', newline='')
    receipt = dict(installed=install, sourceActionRoll=4219866, sourceActionEat=4219804,
        sourceRollingMesh='fm_k_ppct_ball_01', requestedRollingModel=roll['modelAssetId'],
        materialBoundary='Mario mn_ppcc material intentionally reused; original wp_mn_ppct MIC is different.',
        marioGeometry=pivot, sourceGeometry=source_geometry, rollingRig=rig, eatingRig=eat_rig,
        sourceSizeRoll=1.1, sourceTurnsPerSecond=1.05, sourceSizeEat=.8,
        sourceNotifyPositionM=[0,-.5,0], sourceParticlePositionM=[0,.1,-.2],
        runtimeRollingScale=scale, runtimeEatingScale=eat_scale,
        requestedStartMs=dict(rolling=1696, eating=1172),
        worldSequenceRevision=after['revision'], compositionRevision=json.loads(composition_text)['revision'],
        manualVisualValidation='USER_PENDING')
    (patch.OUTPUT / 'ball-world-installation.json').write_text(json.dumps(receipt,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(patch.encode(dict(installed=install,worldSequenceRevision=after['revision'],objects=2)))


if __name__ == '__main__':
    parser=argparse.ArgumentParser(__doc__)
    parser.add_argument('--install', action='store_true')
    apply(parser.parse_args().install)
