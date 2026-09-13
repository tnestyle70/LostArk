"""Build independent trumpet lasers and the original card-suit floor.

Only candidate documents are written. Registration and installation are owned
by the effect-library publisher; existing authored patterns are never replaced.
"""
import argparse
import base64
import copy
import hashlib
import json
import math
import struct
import sys
from pathlib import Path

import build_kouku_action_effect_groups as actions
import build_kouku_backstep_flame_groups as independent

source = actions.source
ROOT = source.ROOT
AUTHORED = ROOT / 'Data/Effects/Authored'
LASER_SYSTEM = 'fx_mn_rpct_05_l.par_l_rpct_05_sk_06_8'
FLOOR_SYSTEM = 'fx_mn_rpct_05_l.par_l_rpct_05_sk_06_9_loc_int'
PREFIX = 'effect.kouku.common.trumpet.'


def sample_source_socket(attachment):
    """Measure the emission origin from the installed rig at notify birth."""
    sys.path.insert(0, str(ROOT))
    from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as composition
    pose = composition.wmodel_pose

    def transform(rotation=(0, 0, 0), scale=1, position=(0, 0, 0)):
        pitch, yaw, roll = [math.radians(v) * .5 for v in rotation]
        sp, cp, sy, cy, sr, cr = math.sin(pitch), math.cos(pitch), math.sin(yaw), math.cos(yaw), math.sin(roll), math.cos(roll)
        return pose.affine_matrix((scale,) * 3,
            (cr * sp * cy + sr * cp * sy, cr * cp * sy - sr * sp * cy,
             sr * cp * cy - cr * sp * sy, cr * cp * cy + sr * sp * sy), position)

    catalog = source.read(ROOT / 'Data/Actors/BossCatalog.json')
    boss = next(b for b in catalog['bosses'] if b['archetypeId'] == 'BOSS_KAKULSAYDON_G1_SAYDON')
    model_path = ROOT / 'Client/Bin/Resources/Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'
    clip = 'rpct00_att_battle_19_01_loop'
    model = pose.read_wmodel(model_path, animation_names={clip})
    actor = dict(body=model, bodyPre=transform(scale=boss['bodyModelPreScale']),
                 poses={}, validatedClips=set())
    bone_name = attachment['runtimeBoneName']
    bone_index = [b.name for b in model.skeleton_bones].index(bone_name)
    bone = composition._sample_bone_bake_pose(actor, 'body', clip, 0)[bone_index]
    local = attachment['socketLocalTransform']
    assert local['scale'] == [1, 1, 1]
    socket = transform(local['rotationDegrees'], position=local['position'])
    world = pose.matrix_multiply(pose.matrix_multiply(socket, bone), transform((0, -90, 0)))
    scale = [math.sqrt(sum(world[r * 4 + c] ** 2 for c in range(3))) for r in range(3)]
    assert all(math.isfinite(v) for v in world) and max(scale) - min(scale) < .00001
    return dict(modelAssetId=model_path.relative_to(ROOT / 'Client/Bin/Resources').as_posix(),
        modelSha256=hashlib.sha256(model_path.read_bytes()).hexdigest(), runtimeClip=clip,
        seconds=0, runtimeBoneName=bone_name, bodyModelPreScale=boss['bodyModelPreScale'],
        sourceSocketLocalTransform=local, sourceSocketWorldMatrix=world,
        position=world[12:15], basisScale=scale,
        evidenceKind='INSTALLED_WMODEL_CPU_POSE_AND_SOCKET_SNAPSHOT')


def read_laser_notifies(action_path):
    action = next(a for a in source.read(action_path)['actions'] if a['actionId'] == 4219807)
    stage = next(s for s in action['stages'] if s['stageIndex'] == 2)
    sockets = actions.exact.socket_contract('mn_rpct_05.mesh.mn_rpct_05_sk')
    bones = {s['boneName'].lower(): None for s in sockets['sockets']}
    result = []
    for ordinal in (5, 6):
        notify = next(n for n in stage['notifies']
                      if n['notifyId'].endswith(f'/notify-{ordinal:03d}'))
        cue = actions.decode_notify(notify, sockets, bones)
        raw = base64.b64decode(notify['serializedPayload']['data'], validate=True)
        transform = cue['sourceTransformByteOffset']
        # These two compact CEFParticleData records store FRotator int32s at
        # +40. The generic decoder's +28 floats are a distinct zero vector.
        assert transform == 403 and cue['sourceParameterCountByteOffset'] == transform + 88
        assert struct.unpack_from('<3f', raw, transform + 28) == (0, 0, 0)
        rotation = list(struct.unpack_from('<3i', raw, transform + 40))
        assert rotation == ([0, 0, 0] if ordinal == 5 else [0, 8192, 0])
        cue['localTransform']['rotationDegrees'] = [rotation[0] * 360 / 65536,
            rotation[1] * 360 / 65536, -rotation[2] * 360 / 65536]
        assert cue['sourceParticleSystem'].split("'")[1].lower() == LASER_SYSTEM
        assert cue['localTransform']['position'] == [0, 0, 0]
        assert cue['localTransform']['scale'] == [1, 1, 1]
        assert cue['attachment']['sourceAnchorNames'] == ['FX_Prj_04']
        assert notify['localTimeSeconds'] == 0 and abs(notify['durationSeconds'] - 2.8) < 1e-6
        assert [(p['name'], p['type']) for p in cue['parameterOverrides']] == [('None', 'none'), ('Color', 'vector')]
        assert cue['parameterOverrides'][1]['vectorValue'] == [1, 1, 1]
        result.append(dict(notifyId=notify['notifyId'], sourceOffset=notify['sourceOffset'],
            sourcePayloadSha256=hashlib.sha256(raw).hexdigest(), sourceTransformByteOffset=transform,
            sourceRotatorByteOffset=transform + 40, sourceRotator=rotation,
            durationSeconds=notify['durationSeconds'], sourceTransform=cue['localTransform'],
            sourceAttachment=cue['attachment'], sourceParameters=cue['parameterOverrides']))
    return stage, result


def make_document(asset, name, leaf, calls):
    document = copy.deepcopy(leaf)
    document.update(effectAssetId=asset, displayName=name, elements=[], modelCues=[])
    document['particleSystem'].update(uniformScaleMultiplier=1, yawOffsetDegrees=0,
                                     directionYawDegrees=0, initialSpeedMultiplier=1)
    document.pop('sourceModelPreview', None)
    document.pop('sourceAnchorAnimations', None)
    if document['version'] >= 15:
        document['runtimeExtensions'] = dict(formatVersion=1, bakedEdgeHistories=[])
    else:
        document.pop('runtimeExtensions', None)
    for call in calls:
        group = 'kouku.trumpet.' + hashlib.sha256((asset + '/' + call['role']).encode()).hexdigest()[:20]
        ids = {e['id']: group + '.' + str(i) for i, e in enumerate(leaf['elements'])}
        for original in leaf['elements']:
            element = independent.remap(copy.deepcopy(original), ids)
            element['groupId'] = group
            if 'particleSystemOccurrenceId' in element['sourceRecipe']:
                element['sourceRecipe']['particleSystemOccurrenceId'] = asset + '/' + call['role']
            attachment = element['actionCueAttachment']
            attachment.update(enabled=False, follow=False, sourceAnchorSlotId='root',
                              runtimeAnchorSlotId='root', runtimeBoneName='')
            attachment.pop('snapshotRootSourceBasisYawDegrees', None)
            attachment.pop('modelCueId', None)
            attachment['socketLocalTransform'] = dict(position=[0, 0, 0],
                rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
            element['detail']['transform'].update(position=call['position'],
                rotationDegrees=[0, call['yaw'], 0], scale=call.get('scale', [1, 1, 1]))
            if 'durationSeconds' in call:
                element['detail']['timing']['lifeTimeSeconds'] = call['durationSeconds']
                element['sourcePresentation'].update(sourceActionCueId=call['role'],
                    sourceEventId=call['role'], sourceTimeSeconds=0)
            independent.project_mesh_rotation(element)
            document['elements'].append(element)
    assert len({e['id'] for e in document['elements']}) == len(document['elements'])
    assert len(name.encode('utf-8')) <= 64
    return document


def build(organization_path, evidence):
    organization = source.read(organization_path)
    entry = next(a for a in organization['actions']
                 if a['profileId'] == 'MN_RPCT_05' and a['actionId'] == 4219807)
    action_path = Path(entry['sourceActionPath'])
    stage, notifies = read_laser_notifies(action_path)
    inputs, leaves = [], {}
    for system in (LASER_SYSTEM, FLOOR_SYSTEM):
        path = AUTHORED / ('effect.kouku.source.' + system + '.effect.json')
        leaves[system] = source.read(path)
        assert len(leaves[system]['elements']) == 20
        inputs.append(dict(path=path.relative_to(ROOT).as_posix(),
                           sha256=hashlib.sha256(path.read_bytes()).hexdigest()))

    laser_leaf = leaves[LASER_SYSTEM]
    beams = [e for e in laser_leaf['elements'] if e['sourceRecipe']['rendererShape'] == 'beam']
    assert len(beams) == 16
    targets = []
    for element in beams:
        distribution = next(d for m in element['sourceRecipe']['modules']
            if m['className'] == 'particlemodulebeamtarget' for d in m['distributions']
            if d['propertyPath'] == 'target')
        values = distribution['lookupTable'][2:]
        assert all(abs(values[i] + 360) < .001 for i in range(2, len(values), 3))
        targets.append(tuple(values[:3]))
    directions = sorted(set(targets))
    assert len(directions) == 4
    snapshot = sample_source_socket(notifies[0]['sourceAttachment'])
    # The independently editable group freezes the actual notify-birth socket
    # origin and inherited actor scale. Its radial frame uses owner yaw; the
    # spinning weapon basis is retained as evidence, not applied to world rays.
    calls = [dict(role=n['notifyId'], position=snapshot['position'], scale=snapshot['basisScale'],
                  yaw=-90 + n['sourceTransform']['rotationDegrees'][1],
                  durationSeconds=n['durationSeconds']) for n in notifies]
    lasers = make_document(PREFIX + 'radial.lasers', '트럼펫_8방향레이저', laser_leaf, calls)
    headings = sorted({round((math.degrees(math.atan2(-v[1], v[0])) + call['yaw']) % 360, 6)
                       for v in directions for call in calls})
    assert headings == list(range(0, 360, 45))
    # The original Color override is already the source leaf's white default.
    for element in laser_leaf['elements']:
        for module in element['sourceRecipe']['modules']:
            for distribution in module['distributions']:
                if distribution['sourceClass'] == 'distributionvectorparticleparameter':
                    assert distribution['defaultMinimum'][:3] == distribution['defaultMaximum'][:3] == [1, 1, 1]

    preview, _, clips, _ = actions.model_contract('MN_RPCT_05', evidence)
    mapped_stage = next(s for s in entry['stages'] if s['stageIndex'] == 2)
    animations, missing = actions.stage_animations(mapped_stage, stage, clips)
    assert not missing and len(animations) == 1
    assert animations[0]['runtimeClip'] == 'rpct00_att_battle_19_01_loop'
    lasers['sourceModelPreview'] = dict(**preview, animations=animations)

    floor_leaf = leaves[FLOOR_SYSTEM]
    floor = make_document(PREFIX + 'suit.floor', '트럼펫_카드문양장판(원본)',
        floor_leaf, [dict(role='source-floor', position=[0, 0, 0], yaw=-90)])
    # Reuse the known owning action's installed actor/clip as preview context.
    # This does not assert the projectile floor's animation-relative birth time.
    floor['sourceModelPreview'] = copy.deepcopy(lasers['sourceModelPreview'])
    # The full original floor includes its onset particles; preserve every
    # emitter's size, delay, duration and particle lifetime for later editing.
    for original, element in zip(floor_leaf['elements'], floor['elements']):
        assert original['detail']['timing'] == element['detail']['timing']
        assert original['detail']['particle'] == element['detail']['particle']
        assert original['sourceRecipe']['modules'] == element['sourceRecipe']['modules']

    rows = []
    for document, system in [(lasers, LASER_SYSTEM), (floor, FLOOR_SYSTEM)]:
        checked = independent.inspect_document(document)
        path = 'Data/Effects/Authored/' + document['effectAssetId'] + '.effect.json'
        source.write(evidence / 'candidate' / Path(path).name, document)
        rows.append(dict(effectAssetId=document['effectAssetId'], displayName=document['displayName'],
            path=path, sourceParticleSystem=system,
            categoryPath=['KoukuSaydon', '1관문', '패턴', '세이튼', '세이튼_트럼펫장판소환'],
            defaultAnchorKind='BOSS', followBoss=False, **checked))
    source.write(evidence / 'installation.json', dict(installed=False, documents=rows,
        sourceInputs=inputs, sourceActionPath=str(action_path),
        sourceActionSha256=hashlib.sha256(action_path.read_bytes()).hexdigest(),
        sourceActionId=4219807, sourceStageIndex=2, laserNotifies=notifies,
        laserDirectionHeadingsDegrees=headings, sourceBeamTargetHeightCm=-360,
        independentLaserSocketSnapshot=snapshot,
        independentPlacementBasis='Installed notify-birth FX_Prj_04 position/scale snapshot; editable upright owner-yaw radial frame with source-X to Client-forward yaw -90. Original spinning socket rotation is not followed.',
        floorTimingPolicy='Original per-emitter delay, duration, particle lifetime and size are unchanged.',
        previewPolicy='Both groups use the exact installed Action4219807 stage002 actor/clip as read-only preview context; no Composition animation is injected. The floor context does not assert its projectile birth timing.',
        excludedExplosionSource='fx_mn_rpct_05_l.par_l_rpct_05_sk_06_2_loc_int',
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(installed=False, documents=len(rows), elements=sum(r['elementCount'] for r in rows),
                          laserDirections=len(headings), candidateRoot=str(evidence / 'candidate'))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--organization', type=Path, default=ROOT / 'out/KoukuAllEffects20260912/organization.json')
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuTrumpetGroups20260913')
    args = parser.parse_args()
    build(args.organization, args.evidence_root)
