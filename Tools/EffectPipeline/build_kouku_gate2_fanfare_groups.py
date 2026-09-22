"""Stage Gate 2's six rays and front fanfare from the saved native leaves.

Action 4219713 owns a B_02 circle followed by three enabled bilateral B calls.
Their source FRotators and relative onset are retained without changing saved
leaf edits. The separate front music resource remains an explicit authored reuse.
"""
from __future__ import annotations

import argparse
import base64
import copy
import hashlib
import json
import math
import struct
from pathlib import Path

import build_kouku_backstep_flame_groups as groups
import build_kouku_action_effect_groups as actions

ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / 'Data/Effects/Authored'
PREFIX = 'effect.kouku.gate2.fanfare.'
SOURCE = 'effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_'


def read_occurrences():
    path = ROOT / 'out/RaidAudio20260922/source/MN_RPCZ_00.action-effects.json'
    model_path = ROOT / 'out/KoukuActionEffects20260912/MN_RPCZ_00.model_contract.json'
    action = next(a for a in json.loads(path.read_text(encoding='utf-8'))['actions']
                  if a['actionId'] == 4219713)
    stage = next(s for s in action['stages'] if s['stageIndex'] == 1)
    model = json.loads(model_path.read_text(encoding='utf-8'))
    rows = []
    for ordinal in (11, 19, 20, 21, 22):
        notify = next(n for n in stage['notifies']
                      if n['notifyId'].endswith(f'/notify-{ordinal:03d}'))
        cue = actions.decode_notify(notify, model['sourceSockets'], model['installedBones'])
        raw = base64.b64decode(notify['serializedPayload']['data'], validate=True)
        transform = cue['sourceTransformByteOffset']
        # This compact record stores integer UE FRotator at +40, not the
        # separate zero float vector at +28 consumed by the generic decoder.
        assert struct.unpack_from('<3f', raw, transform + 28) == (0, 0, 0)
        rotator = list(struct.unpack_from('<3i', raw, transform + 40))
        expected = 0 if ordinal in (11, 19) else (ordinal - 19) * 8192
        assert rotator == [0, expected, 0]
        assert cue['enabled'] == (ordinal != 19)
        assert cue['localTransform']['position'] == [0, 0, 0]
        assert cue['localTransform']['scale'] == [1, 1, 1]
        assert cue['attachment']['mode'] == 'SNAPSHOT_ROOT'
        suffix = 'b_02_loc_int' if ordinal == 11 else 'b_loc_int'
        assert cue['sourceParticleSystem'].split("'")[1].lower().endswith('trumpet_' + suffix)
        rows.append(dict(notifyId=notify['notifyId'], enabled=cue['enabled'],
            sourceParticleSystem=cue['sourceParticleSystem'], suffix=suffix,
            sourceTimeSeconds=notify['localTimeSeconds'], durationSeconds=notify['durationSeconds'],
            sourceTransformByteOffset=transform, sourceRotatorByteOffset=transform + 40,
            sourceRotator=rotator, sourceYawDegrees=expected * 360 / 65536,
            runtimeYawDegrees=expected * 360 / 65536 - 90,
            sourcePayloadSha256=hashlib.sha256(raw).hexdigest()))
    origin = rows[0]['sourceTimeSeconds']
    for row in rows:
        row['relativeStartSeconds'] = row['sourceTimeSeconds'] - origin
    return rows, dict(path=path.relative_to(ROOT).as_posix(),
                     sha256=hashlib.sha256(path.read_bytes()).hexdigest()), stage['animationClips']


def read_leaf(suffix):
    path = AUTHORED / (SOURCE + suffix + '.effect.json')
    raw = path.read_bytes()
    return json.loads(raw), dict(path=path.relative_to(ROOT).as_posix(),
                                sha256=hashlib.sha256(raw).hexdigest())


def rotate_y(value, degrees):
    angle = math.radians(degrees)
    x, y, z = value
    return [x * math.cos(angle) + z * math.sin(angle), y,
            -x * math.sin(angle) + z * math.cos(angle)]


def add_call(document, leaf, role, yaw, position, start=0):
    group = 'kouku.gate2.fanfare.' + hashlib.sha256(
        (document['effectAssetId'] + '/' + role).encode()).hexdigest()[:20]
    identities = {e['id']: group + '.' + str(i) for i, e in enumerate(leaf['elements'])}
    for original in leaf['elements']:
        element = groups.remap(copy.deepcopy(original), identities)
        element['groupId'] = group
        attachment = element['actionCueAttachment']
        attachment.update(enabled=False, follow=False, sourceAnchorSlotId='root',
                          runtimeAnchorSlotId='root', runtimeBoneName='')
        attachment.pop('snapshotRootSourceBasisYawDegrees', None)
        attachment.pop('modelCueId', None)
        attachment['socketLocalTransform'] = dict(position=[0, 0, 0],
            rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
        transform = element['detail']['transform']
        # A nonzero saved pitch/roll requires the runtime quaternion helper;
        # do not silently replace an author's compound rotation on regeneration.
        assert transform['rotationDegrees'][0] == transform['rotationDegrees'][2] == 0
        rotated = rotate_y(transform['position'], yaw)
        transform['position'] = [a + b for a, b in zip(rotated, position)]
        transform['rotationDegrees'][1] += yaw
        transform['velocityPerSecond'] = rotate_y(transform['velocityPerSecond'], yaw)
        element['detail']['timing']['startDelaySeconds'] += start
        groups.project_mesh_rotation(element)
        # Explicit independent group rotation follows the existing Tool helper
        # contract for source fixed-axis sprites. Camera-facing sprites retain it.
        if element['sourceRecipe']['rendererShape'] == 'sprite':
            if any(m['className'].startswith('particlemoduleorientationaxislock') and
                   any(l['propertyPath'] == 'lockaxisflags' and
                       l['value'] not in ('epal_none', 'EPAL_NONE') for l in m['literals'])
                   for m in element['sourceRecipe']['modules']):
                element['detail']['sprite']['followEmitterAxisRotation'] = True
        if 'particleSystemOccurrenceId' in element['sourceRecipe']:
            element['sourceRecipe']['particleSystemOccurrenceId'] = group
        document['elements'].append(element)


def document(asset, label, template):
    result = copy.deepcopy(template)
    result.update(effectAssetId=asset, displayName=label, elements=[], modelCues=[])
    result['particleSystem'].update(uniformScaleMultiplier=1, yawOffsetDegrees=0,
        directionYawDegrees=0, initialSpeedMultiplier=1)
    result.pop('sourceModelPreview', None)
    result.pop('sourceAnchorAnimations', None)
    if result['version'] >= 15:
        result['runtimeExtensions'] = dict(formatVersion=1, bakedEdgeHistories=[])
    return result


def build(output):
    output = output.resolve()
    assert output.is_relative_to((ROOT / 'out').resolve())
    leaves, inputs = {}, []
    calls, source_input, clips = read_occurrences()
    for suffix in ('b_loc_int', 'b_02_loc_int', 'd_music_loc_int'):
        leaves[suffix], evidence = read_leaf(suffix)
        inputs.append(evidence)
    radial = document(PREFIX + 'circle.six-rays', '원형과6방향폭발', leaves['b_loc_int'])
    for call in calls:
        if call['enabled']:
            add_call(radial, leaves[call['suffix']], call['notifyId'],
                     call['runtimeYawDegrees'], [0, 0, 0], call['relativeStartSeconds'])
    front = document(PREFIX + 'front.music', '쿠크전방팡파레', leaves['d_music_loc_int'])
    add_call(front, leaves['d_music_loc_int'], 'front-music', -90, [0, 0, 0])
    rows = []
    for value in (radial, front):
        checked = groups.inspect_document(value)
        path = 'Data/Effects/Authored/' + value['effectAssetId'] + '.effect.json'
        candidate = output / 'candidate' / Path(path).name
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        rows.append(dict(effectAssetId=value['effectAssetId'], displayName=value['displayName'],
            path=path, candidatePath=candidate.relative_to(ROOT).as_posix(),
            categoryPath=['KoukuSaydon', '2관문', '패턴', '쿠크_3갈레바닥장판폭발'],
            defaultAnchorKind='BOSS', followBoss=False, **checked))
    report = dict(installed=False, documents=rows, sourceInputs=inputs + [source_input],
        sourceActionId=4219713, sourceStageIndex=1, sourceAnimationClips=clips,
        sourceOccurrences=calls,
        sourceCircleOnsetSeconds=calls[0]['sourceTimeSeconds'],
        sourceRayDelaySeconds=calls[1]['relativeStartSeconds'],
        rayHeadingsDegrees=[-135, -45, 0, 45, 135, 180],
        rayHeadingConvention='atan2(runtime Z, runtime X), +180 and -180 equivalent; saved unmodified source leaf.',
        arrangementBasis='SOURCE_ACTION_OCCURRENCES: enabled notify020/021/022 FRotator yaw45/90/135; notify019 yaw0 disabled. Runtime occurrence yaw=source yaw-90 once. Not 60-degree equal spacing.',
        timingBasis='Compound time0 is source clip Att_Battle_3_06 circle onset0.9440249800682068; rays start at1.1245650053024292 (relative0.1805400252342224). Linking to an authored pattern remains pending.',
        frontMusicBasis='PROJECT_AUTHORED_REUSE: D_Music has no direct notify in Action4219713; it belongs to Action4219715 Att_Battle_3_08 at0.751487. Front placement remains authored, not an original4219713 occurrence.',
        circleBasis='B_02 is notify011 of Action4219713. D_02 belongs to separate Action4219715 donut at2.078297 and is left unchanged.',
        materialBasis='Current saved source Cascade/native leaves, including existing element edits.',
        manualVisualValidation='USER_PENDING')
    (output / 'installation.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(documents=len(rows), elements=sum(r['elementCount'] for r in rows), installed=False)))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/Gate2Restoration20260922/fanfare')
    build(parser.parse_args().output)
