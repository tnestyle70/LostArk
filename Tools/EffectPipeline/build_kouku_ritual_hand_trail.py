"""Build the ritual hand ribbon and restore the existing small pentagram sequence.

Original files and installed authoring are read-only. The star cast/shot source
notifies are disabled in Action4219911; their independent library composition is
an explicit user request, while the hand notify retains its original enable flag.
"""
import argparse
import copy
import hashlib
import json
import math
from pathlib import Path

import build_kouku_gate1_full_restore as source
from extract_ue3_skeletal_mesh_sockets import parse_socket_contract
from build_kouku_threeway_breath_group import SOCKETS
from build_kouku_gold_trails_restore import verify_required_default

ROOT = source.ROOT
ACTION = source.SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json'
HAND = 'effect.kouku.gate3.ritual.hand.trail.full.restore'
STAR = 'effect.kouku.gate3.mario.boss.pentagram.full.restore'
HAND_SYSTEM = 'fx_mn_rpct_07_v.par_v_rpct_handswing_trail_01_loc_int'
SHOT_SYSTEM = 'fx_mn_rpct_07_v.par_v_rpct_star_shot_01_loc_int'
SHOT_ACTION = 10304
FLOOR_IDS = {'kouku.mario.small.pentagram.floor.child.authored.' + str(i).zfill(5) for i in range(16, 21)}


def fingerprint(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=False).encode()).hexdigest()


def source_projection(evidence):
    """Decode the active hand cue and explicitly audition the disabled star shot."""
    sockets = parse_socket_contract(SOCKETS)
    source.write(evidence / 'source_socket_contract.before-basis.json', sockets)
    socket = next(s for s in sockets['sockets'] if s['socketName'].casefold() == 'startcontrol')
    assert socket['boneName'].casefold() == 'b_wp_1'
    assert socket['sourceTransform'] == dict(positionUeUnits=[75, 0, 0], rotationUnrealUnits=[0, 0, 0], scale=[1, 1, 1])
    # Same installed RPCT05 b_wp_1 zero-rotation basis as the measured midcontrol
    # contract in build_kouku_threeway_breath_group; source cm become metres once.
    socket['runtimeLocalTransform'] = dict(position=[.75, 0, 0], rotationDegrees=[-90, 0, 0], scale=[1, 1, 1])
    source.write(evidence / 'source_socket_contract.json', sockets)
    action = next(a for a in source.read(ACTION)['actions'] if a['actionId'] == 4219911)
    stage = next(s for s in action['stages'] if s['stageIndex'] == 0)
    hand = next(n for n in stage['notifies'] if n['notifyId'].endswith('/notify-001'))
    shot = next(n for n in stage['notifies'] if n['notifyId'].endswith('/notify-008'))
    cast = next(n for n in stage['notifies'] if n['notifyId'].endswith('/notify-002'))
    decoded = {n['notifyId']: source.decode_typed_payload(n['sourceType'], n['serializedPayload'], sockets,
               n['assetReferences'], n['serializedLabels']) for n in (hand, cast, shot)}
    assert decoded[hand['notifyId']]['enabled'] is True
    assert decoded[cast['notifyId']]['enabled'] is False and decoded[shot['notifyId']]['enabled'] is False
    source.write(evidence / 'original_ritual_notifies.json', dict(sourceActionId=4219911,
        sourceStageIndex=0, animationClips=stage['animationClips'], notifies=[hand, cast, shot], decoded=decoded))
    hand_action = copy.deepcopy(action)
    hand_action['stages'] = [copy.deepcopy(stage)]
    hand_action['stages'][0]['notifies'] = [copy.deepcopy(hand)]
    shot_cue = copy.deepcopy(decoded[shot['notifyId']])
    shot_cue.update(enabled=True, sourceKind='USER_REQUESTED_ORIGINAL_DISABLED_SYSTEM_PREVIEW')
    shot_notify = copy.deepcopy(shot)
    shot_notify['serializedPayload'] = shot_cue
    selected = dict(actions=[hand_action, dict(actionId=SHOT_ACTION, stages=[dict(stageIndex=0,
        animationClips=[], notifies=[shot_notify])])])
    source.write(evidence / 'selected_source_actions.json', selected)
    previous = source.ACTION, source.SELECTED, source.decode_typed_payload
    original_decode = source.decode_typed_payload
    def decode(kind, payload, contract=None, references=None, labels=None):
        if isinstance(payload, dict) and payload.get('sourceKind') == 'USER_REQUESTED_ORIGINAL_DISABLED_SYSTEM_PREVIEW':
            return copy.deepcopy(payload)
        return original_decode(kind, payload, contract, references, labels)
    try:
        source.ACTION = evidence / 'selected_source_actions.json'
        source.SELECTED = {4219911: ([0], '제물 의식 손 궤적'), SHOT_ACTION: ([0], '작은 오망성 별 폭발')}
        source.decode_typed_payload = decode
        index, notifies, occurrences, records = source.acquire(evidence)
        # The complete Required CDO chain has integer-zero EmitterLoops. As in
        # the existing golden-ribbon restore, the notify owns the stop window.
        chain = verify_required_default(evidence / 'source_class_defaults.json')
        loop_defaults = []
        for occurrence in occurrences:
            for key in occurrence['moduleOrder']:
                module = index.objects[key]
                if module.class_name == 'particlemodulerequired' and 'emitterloops' not in module.properties:
                    module.properties['emitterloops'] = dict(type='IntProperty', structType=None, value=0)
                    loop_defaults.append(key)
        source.write(evidence / 'source_zero_loop_defaults.json', dict(classDefaultChain=chain, modules=loop_defaults))
        materials = {}
        for system in (HAND_SYSTEM, SHOT_SYSTEM):
            leaf = source.read(ROOT / 'Data/Effects/Authored' / ('effect.kouku.source.' + system + '.effect.json'))
            for element in leaf['elements']:
                materials[(element['sourcePresentation']['sourceObjectPath'], element['material']['sourceMaterialPath'])] = element['material']
        programs = []
        for occurrence in occurrences:
            material = materials[(occurrence['sourceEmitter'], occurrence['sourceMaterial'])]
            assert material['sourceProfile']['enabled'], occurrence
            programs.append(dict(occurrences=[occurrence['elementId']], material=material))
        patch = evidence / 'native_material_patch.json'
        source.write(patch, dict(programs=programs))
        preview = source.source_model_preview(ACTION, 4219911, [0], 'GATE3', 'MN_RPCT_05', 'boss.kakulsaydon.g3.saydon')
        source.project(evidence, index, notifies, occurrences, records, evidence / 'projected',
            material_patch=patch, source_model_previews={4219911: preview})
    finally:
        source.ACTION, source.SELECTED, source.decode_typed_payload = previous
    return (source.read(evidence / 'projected/effect.kouku.gate1.4219911.full.restore.effect.json'),
            source.read(evidence / 'projected' / f'effect.kouku.gate1.{SHOT_ACTION}.full.restore.effect.json'),
            shot['localTimeSeconds'])


def complete_small_pentagram(document, explosion, shot_time):
    """Preserve the eight authored drawing emitters and replace only gray floor5."""
    drawing = [copy.deepcopy(e) for e in document['elements'] if e['id'] not in FLOOR_IDS]
    # Re-running after installation preserves the same original drawing stream.
    drawing = [e for e in drawing if not e['id'].startswith('kouku.' + str(SHOT_ACTION) + '.')]
    assert len(drawing) == 8 and all(e['sourcePresentation']['sourceObjectPath'].startswith(
        'fx_mn_rpct_07_v.par_v_rpct_star_cast_01_loc_int.') for e in drawing)
    shot = copy.deepcopy(explosion['elements'])
    assert len(shot) == 7
    for element in shot:
        element['groupId'] = STAR
        # This independent floor preview shares the preserved drawing's root
        # basis. Do not add the action-only snapshot yaw to one half of the star.
        element['actionCueAttachment']['enabled'] = False
        element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees', None)
        assert element['detail']['timing']['startDelaySeconds'] == shot_time
    result = copy.deepcopy(document)
    result['displayName'] = '작은 오망성'
    result['elements'] = drawing + shot
    return result, dict(preservedDrawingCount=8, preservedDrawingSha256=fingerprint(drawing),
        removedGrayFloorIds=sorted(FLOOR_IDS), addedSourceStarExplosionCount=7,
        sourceActionId=4219911, sourceNotify='action-4219911/stage-000/notify-008',
        sourceNotifyEnabled=False, compositionDecision='USER_REQUESTED_INDEPENDENT_SOURCE_SYSTEM_SEQUENCE',
        shotStartSeconds=shot_time)


def build(evidence):
    hand, explosion, shot_time = source_projection(evidence)
    hand.update(effectAssetId=HAND, displayName='제물 의식_손 궤적')
    assert len(hand['elements']) == 3
    for element in hand['elements']:
        element['groupId'] = HAND
        attachment = element['actionCueAttachment']
        assert attachment['enabled'] and attachment['follow'] and attachment['runtimeBoneName'] == 'b_wp_1'
        assert attachment['runtimeAnchorSlotId'] == 'startcontrol'
    assert sum(e.get('runtimeCarrier', {}).get('kind') == 'cascadeRibbonV1' for e in hand['elements']) == 2
    baseline_path = ROOT / 'Data/Effects/Authored' / (STAR + '.effect.json')
    baseline = source.read(baseline_path)
    source.write(evidence / 'small_pentagram.before.json', baseline)
    star, sequence = complete_small_pentagram(baseline, explosion, shot_time)
    source.write(evidence / 'small_pentagram_sequence.json', sequence)
    rows = []
    for document in (hand, star):
        asset = document['effectAssetId']
        source.write(evidence / 'candidate' / (asset + '.effect.json'), document)
        duration = max(e['detail']['timing']['startDelaySeconds'] + e['detail']['timing']['lifeTimeSeconds'] +
            e['detail']['timing']['afterImageSeconds'] + max(e['detail']['particle']['lifeTimeSeconds'])
            for e in document['elements'])
        rows.append(dict(effectAssetId=asset, displayName=document['displayName'],
            path='Data/Effects/Authored/' + asset + '.effect.json', durationMs=math.ceil(duration * 1000),
            elementCount=len(document['elements']), defaultAnchorKind='BOSS', followBoss=True,
            categoryPath=['KoukuSaydon', '3관문', '패턴', '세이튼', '제물 의식' if asset == HAND else '작은 오망성']))
    source.write(evidence / 'installation.json', dict(installed=False, documents=rows,
        sourceInputs=[dict(path=str(p), sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in (ACTION, SOCKETS)],
        preservedDrawingSha256=sequence['preservedDrawingSha256'], sourceStarNotifyEnabled=False,
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(documents=[dict(asset=r['effectAssetId'], elements=r['elementCount']) for r in rows]), ensure_ascii=True))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuRitualHandTrail20260913')
    options = parser.parse_args()
    build(options.evidence_root)
