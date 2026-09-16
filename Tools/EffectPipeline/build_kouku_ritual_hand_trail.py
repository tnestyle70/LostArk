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
BURST_ACTION = 4219932
BURST_SYSTEMS = {
    'fx_mn_rpct_07_v.par_v_rptm_down_lighting_atk_02_loc_int': (12, 1.9939700365066528),
    'fx_mn_rpct_07_v.par_v_rpct_atk_09_01_loc_int': (13, 2.051466941833496),
}
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
    companions = [copy.deepcopy(e) for e in document['elements']
                  if e.get('sourcePresentation', {}).get('sourceObjectPath', '').rsplit('.', 1)[0] in BURST_SYSTEMS]
    drawing = [copy.deepcopy(e) for e in document['elements'] if e['id'] not in FLOOR_IDS and
               e.get('sourcePresentation', {}).get('sourceObjectPath', '').rsplit('.', 1)[0] not in BURST_SYSTEMS]
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
    result['elements'] = drawing + shot + companions
    return result, dict(preservedDrawingCount=8, preservedDrawingSha256=fingerprint(drawing),
        removedGrayFloorIds=sorted(FLOOR_IDS), addedSourceStarExplosionCount=7,
        sourceActionId=4219911, sourceNotify='action-4219911/stage-000/notify-008',
        sourceNotifyEnabled=False, compositionDecision='USER_REQUESTED_INDEPENDENT_SOURCE_SYSTEM_SEQUENCE',
        shotStartSeconds=shot_time)


def complete_small_pentagram_impact(document, evidence):
    """Append the original beam/debris impact without changing the saved star.

    Action4219932 uses the same 27_01 clip and impact times as Action4219911,
    but enables the companion impact too. The library intentionally contains
    that complete impact. Hand attachment, precast/gray floor and screen-post
    notifies remain separate resources instead of being silently folded in.
    """
    from build_kouku_encore_blackhole_beam import restore_nested_raw_defaults
    evidence.mkdir(parents=True, exist_ok=True)
    action = copy.deepcopy(next(a for a in source.read(ACTION)['actions'] if a['actionId'] == BURST_ACTION))
    stage = next(s for s in action['stages'] if s['stageIndex'] == 0)
    assert stage['animationClips'][0]['clipName'].lower() == 'att_battle_27_01'
    selected = []
    for system, (ordinal, seconds) in BURST_SYSTEMS.items():
        notify = next(n for n in stage['notifies'] if n['notifyId'].endswith('/notify-' + str(ordinal).zfill(3)))
        decoded = source.decode_typed_payload(notify['sourceType'], notify['serializedPayload'], None,
                                             notify['assetReferences'], notify['serializedLabels'])
        assert decoded['enabled'] and notify['localTimeSeconds'] == seconds
        assert notify['assetReferences'][0]['objectPath'].lower() == system
        assert decoded['attachment']['mode'] == 'SNAPSHOT_ROOT'
        assert decoded['localTransform']['scale'] == [2.0, 2.0, 2.0]
        selected.append(copy.deepcopy(notify))
    stage['notifies'] = selected
    action['stages'] = [stage]
    selected_path = evidence / 'selected_source_actions.json'
    source.write(selected_path, dict(actions=[action]))
    previous = source.ACTION, source.SELECTED
    source.ACTION, source.SELECTED = selected_path, {BURST_ACTION: ([0], 'Small pentagram original beam and impact')}
    try:
        index, notifies, occurrences, records = source.acquire(evidence)
        assert len(notifies) == 2 and len(occurrences) == 23
        programs = []
        for system in BURST_SYSTEMS:
            leaf = source.read(ROOT / 'Data/Effects/Authored' / ('effect.kouku.source.' + system + '.effect.json'))
            materials = {(e['sourcePresentation']['sourceObjectPath'], e['material']['sourceMaterialPath']): e['material']
                         for e in leaf['elements']}
            for occurrence in (o for o in occurrences if o['sourceSystem'] == system):
                material = materials[(occurrence['sourceEmitter'], occurrence['sourceMaterial'])]
                assert material['sourceProfile']['enabled']
                programs.append(dict(occurrences=[occurrence['elementId']], material=material))
        patch = evidence / 'native_material_patch.json'
        source.write(patch, dict(programs=programs))
        source.project(evidence, index, notifies, occurrences, records, evidence / 'projected', material_patch=patch)
    finally:
        source.ACTION, source.SELECTED = previous
    projected = source.read(evidence / 'projected' / ('effect.kouku.gate1.' + str(BURST_ACTION) + '.full.restore.effect.json'))
    restore_nested_raw_defaults(projected, evidence)
    notify_windows = {n['notifyId']: n['durationSeconds'] for n in notifies}
    window_repairs = []
    preserved = [copy.deepcopy(e) for e in document['elements']
                 if e.get('sourcePresentation', {}).get('sourceObjectPath', '').rsplit('.', 1)[0] not in BURST_SYSTEMS]
    assert len(preserved) == 15
    for element in projected['elements']:
        recipe = element['sourceRecipe']
        window = notify_windows[element['sourcePresentation']['sourceEventId']]
        if window > 0 and recipe['emitterDurationSeconds'] > window:
            assert recipe['emitterLoopCount'] == 1 and recipe['emitterDelaySeconds'] == 0
            recipe['emitterLoopCount'] = 0
            window_repairs.append(element['id'])
        element['groupId'] = STAR
        # Reuse the existing independent cast/shot root basis. Source notify TRS
        # and mesh/particle units are retained; do not apply the action-only yaw.
        element['actionCueAttachment']['enabled'] = False
        element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees', None)
    existing = [e for e in document['elements']
                if e.get('sourcePresentation', {}).get('sourceObjectPath', '').rsplit('.', 1)[0] in BURST_SYSTEMS]
    assert not existing or existing == projected['elements'], 'Saved impact edits differ; preserve the user draft instead of replacing it'
    result = copy.deepcopy(document)
    result['elements'] = preserved + projected['elements']
    source.write(evidence / 'impact-closure.json', dict(sourceActionId=BURST_ACTION,
        sourceClip='Att_Battle_27_01', preservedElementCount=15,
        preservedElementsSha256=fingerprint(preserved), addedElementCount=23,
        systems=BURST_SYSTEMS, sourceNotifyTransforms=[n['cue']['localTransform'] for n in notifies],
        notifyStopWindowElements=window_repairs,
        scope='USER_REQUESTED_LIBRARY_IMPACT; SOURCE_4219911_DISABLE_FLAGS_ARE_NOT_CHANGED'))
    return result


def stage_small_pentagram_impact(evidence):
    path = ROOT / 'Data/Effects/Authored' / (STAR + '.effect.json')
    before = path.read_bytes()
    document = complete_small_pentagram_impact(json.loads(before), evidence / 'impact_source')
    candidate = evidence / 'candidate' / path.name
    source.write(candidate, document)
    source.write(evidence / 'installation.json', dict(installed=False, path=str(path.relative_to(ROOT)),
        beforeSha256=hashlib.sha256(before).hexdigest(), candidatePath=str(candidate),
        candidateSha256=hashlib.sha256(candidate.read_bytes()).hexdigest(), elementCount=len(document['elements']),
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(staged=True, elements=len(document['elements']), candidatePath=str(candidate))))


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
    star = complete_small_pentagram_impact(star, evidence / 'impact_source')
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


def stage_left_hand(evidence):
    """Retain the saved ritual recipe and use the original left-hand socket."""
    evidence = evidence.resolve()
    path = ROOT / 'Data/Effects/Authored' / (HAND + '.effect.json')
    before = path.read_bytes()
    hand = json.loads(before)
    sockets = parse_socket_contract(SOCKETS)
    socket = next(s for s in sockets['sockets'] if s['socketName'].casefold() == 'fx_l_hand_01')
    assert socket['boneName'].casefold() == 'bip001-l-hand'
    assert socket['sourceTransform'] == dict(positionUeUnits=[15, 0, 0], rotationUnrealUnits=[0, 0, 0], scale=[1, 1, 1])
    basis_path = ROOT / 'out/KoukuGate3BossAssembly20260912/body_socket_basis_evidence.json'
    basis = source.read(basis_path)
    model = ROOT / 'Client/Bin/Resources/Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'
    assert hashlib.sha256(model.read_bytes()).hexdigest() == '3fd4c21eb87e3ac66ff46d30ebadc164f0bf20530585b9d24a7e5e8ef62b1541'
    assert hashlib.sha256(basis_path.read_bytes()).hexdigest() == 'e3180b63b8c2670670f1790ed475ad41e4f95962cdb6dc65f205331ae71a3f1a'
    hand['displayName'] = '\uc800\uc8fc\uc758\uc2dd | \uc67c\uc190 \ud2b8\ub808\uc77c'
    assert len(hand['elements']) == 3
    previous = []
    for element in hand['elements']:
        attachment = element['actionCueAttachment']
        previous.append(dict(elementId=element['id'], attachment=copy.deepcopy(attachment)))
        assert attachment['enabled'] and attachment['follow']
        attachment.update(runtimeAnchorSlotId='fx_l_hand_01', runtimeBoneName='bip001-l-hand')
        attachment['socketLocalTransform'] = dict(position=[.15, 0, 0], rotationDegrees=[-90, 0, 0], scale=[1, 1, 1])
    # The original startcontrol/weapon emitter places this white ribbon 100cm
    # away from its socket. The user-requested hand adaptation emits at the
    # hand socket; retain the raw source leaf and record this local override.
    white = next(element for element in hand['elements'] if
        element['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-3007-native.v1')
    location = next(module for module in white['sourceRecipe']['modules'] if
        module['className'] == 'particlemodulelocation' and module['stableId'].endswith('.particlemodulelocation_1'))
    distribution = next(value for value in location['distributions'] if value['propertyPath'] == 'startlocation')
    original_location_table = [0.0, 100.0, 100.0, 0.0, 0.0, 100.0, 0.0, 0.0]
    assert distribution['operation'] == 1 and distribution['componentCount'] == 3
    assert distribution['lookupTable'] in (original_location_table, [0.0] * 8)
    before_location = copy.deepcopy(distribution['lookupTable'])
    ribbon_type = next(module for module in white['sourceRecipe']['modules'] if
        module['stableId'] == white['runtimeCarrier']['typeDataModuleStableId'])
    clip_source = next(value for value in ribbon_type['literals'] if value['propertyPath'] == 'bclipsourcesegement')
    assert clip_source['kind'] == 'boolean'
    before_clip_source = clip_source['value']
    # Keep the visible trail head on the hand between distance-based births.
    # This explicit user adaptation changes geometry only, not RNG or spawning.
    clip_source['value'] = False
    distribution['lookupTable'] = [0.0] * 8
    white['detail']['particle']['initialPositionMin'] = [0.0, 0.0, 0.0]
    white['detail']['particle']['initialPositionMax'] = [0.0, 0.0, 0.0]
    hand_adaptation = dict(elementId=white['id'], sourceModuleStableId=location['stableId'],
        originalStartLocationCm=[100, 0, 0], handStartLocationCm=[0, 0, 0],
        beforeLookupTable=before_location, rawSourceLeafModified=False,
        originalClipSourceSegment=True, handClipSourceSegment=False, beforeClipSourceSegment=before_clip_source,
        reason='USER_REQUESTED_LEFT_HAND_SOCKET_ORIGIN_AND_CONNECTED_HEAD; preserve original source facts.')
    candidate = evidence / 'candidate' / path.name
    source.write(candidate, hand)
    source.write(evidence / 'source-left-hand-socket.json', dict(sourceSocket=socket,
        authoredOverride='User requested left hand; original notify startcontrol remains provenance only.',
        beforeAttachments=previous, basisEvidence=str(basis_path), handAdaptation=hand_adaptation,
        sourceClip='rpct00_att_battle_27_01', originalNotifyAction=4219911))
    source.write(evidence / 'installation.json', dict(installed=False, documents=[dict(
        effectAssetId=HAND, displayName=hand['displayName'], path=str(path.relative_to(ROOT)).replace('\\', '/'),
        candidatePath=str(candidate.relative_to(ROOT)).replace('\\', '/'),
        beforeSha256=hashlib.sha256(before).hexdigest(), candidateSha256=hashlib.sha256(candidate.read_bytes()).hexdigest(),
        durationMs=4175, defaultAnchorKind='BOSS', followBoss=True,
        categoryPath=['KoukuSaydon', '3\uad00\ubb38', '\ud328\ud134', '\ucfe0\ud06c\uc138\uc774\ud2bc', '\uc800\uc8fc\uc758\uc2dd'])],
        inputHashes={str(path.relative_to(ROOT)).replace('\\', '/'): hashlib.sha256(before).hexdigest(),
                     str(basis_path.relative_to(ROOT)).replace('\\', '/'): hashlib.sha256(basis_path.read_bytes()).hexdigest(),
                     str(model.relative_to(ROOT)).replace('\\', '/'): hashlib.sha256(model.read_bytes()).hexdigest()},
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(staged=True, elements=3, bone='bip001-l-hand', socket='fx_l_hand_01')))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuRitualHandTrail20260913')
    parser.add_argument('--left-hand-only', action='store_true', help='Stage the saved ritual with the source left-hand socket; leave pentagram untouched.')
    parser.add_argument('--burst-only', action='store_true', help='Stage only the missing original small-pentagram beam/impact; preserve all saved star elements.')
    options = parser.parse_args()
    if options.burst_only:
        stage_small_pentagram_impact(options.evidence_root)
    elif options.left_hand_only:
        stage_left_hand(options.evidence_root)
    else:
        build(options.evidence_root)
