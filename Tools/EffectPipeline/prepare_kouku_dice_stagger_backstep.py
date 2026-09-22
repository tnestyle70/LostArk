"""Stage source Dice/Stagger effects and guarded Composition edits; never install."""
from __future__ import annotations
import argparse
import base64
import copy
import hashlib
import json
import re
import sys
from pathlib import Path

from build_saydon_card_pattern_groups import ROOT, AUTHORED, read, renamed, native_stage, leaf, current_preview
from build_kouku_flame_wave_groups import duration_ms
from guardian_afterimage_projection import decode_trail_ghost

COMPOSITION = 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
OUT = ROOT / 'out/KoukuPatternRestore20260922'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes((json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8'))


def effect(asset):
    return read(AUTHORED / (asset + '.effect.json'))


def buff_receipt(name):
    """Reuse the existing CEFParticleData decoder on its exact serialized suffix.

    The artificial outer notify header only enters the existing block decoder;
    no synthetic byte is interpreted as a source field. Offsets are translated
    back to the original LOA and the receipt labels this adapter explicitly.
    """
    sys.path.insert(0, str(ROOT / 'Tools/LevelPlacementExtractor'))
    from build_action_cue_recipe import decode_typed_payload
    path, = (ROOT / 'out/KoukuAllEffects20260912/source/ParticleSoundNew').glob('*' + name + '.loa')
    raw = path.read_bytes()
    records = []
    for reference in re.finditer(rb"ParticleSystem'[^']+'\x00", raw):
        start = reference.start() - 4
        header = bytearray(48)
        signature = b'CEFActionNotify_PlayParticleEffect\x00'
        header[:len(signature)] = signature
        header[47] = 1
        decoded = decode_typed_payload('PlayParticleEffect',
            {'data': base64.b64encode(bytes(header) + raw[start:]).decode()})
        assert decoded['particleDataDecoded'] and decoded['parameterOverridesDecoded']
        assert decoded['localTransform']['position'] == [0, 0, 0]
        assert decoded['localTransform']['rotationDegrees'] == [0, 0, 0]
        assert decoded['localTransform']['scale'] == [1, 1, 1]
        assert not decoded['parameterOverrides']
        records.append(dict(particleSystem=decoded['sourceParticleSystem'],
            sourceTransformByteOffset=decoded['sourceTransformByteOffset'] - 48 + start,
            localTransform=decoded['localTransform'], parameterOverrides=[],
            anchorNames=decoded['attachment']['sourceAnchorNames']))
    return dict(path=path.relative_to(ROOT).as_posix(), sha256=sha(raw), records=records,
        decoding='CEFParticleData suffix; synthetic outer header is not source evidence',
        runtimeAnchorPolicy='Original FX_Buff_01 mapped to actor-ground translation; no synthetic bone claim')


def prepare(output):
    output = output.resolve()
    assert output.is_relative_to(ROOT / 'out') and output != ROOT / 'out'
    raw = (ROOT / COMPOSITION).read_bytes()
    composition = json.loads(raw)
    resources = {r['resourceId']: r for r in composition['presentationResources']}
    patterns = {p['patternId']: p for p in composition['patterns']}
    documents, registrations, edits, additions, removals = [], [], [], [], []
    prefix = 'effect.kouku.'
    for suffix, source, name in (
        ('card.match.bind.floor', 'fx_mn_rpct_05_x.par_x_rpct_spotlight_01_loc_int', '주사위 속박 원형 문양·버블'),
        ('card.match.bind.release', 'fx_mn_rpct_05_x.par_x_rpct_spotlight_02_loc_int', '주사위 속박 해제'),
        ('gate1.stagger.shield.full.restore', 'fx_mn_rpct_05_l.par_l_rpct_05_shield_01_loc_int', '무력화 원본 방패'),
        ('gate1.stagger.explosion.full.restore', 'fx_mn_rpct_05_l.par_l_rpct_05_sk_12_5_loc_int', '무력화 원본 무지개 폭발')):
        doc = renamed(effect(prefix + 'source.' + source), prefix + suffix, name)
        doc.pop('sourceModelPreview', None)
        doc.pop('sourceAnchorAnimations', None)
        if suffix == 'gate1.stagger.shield.full.restore':
            for element in doc['elements']:
                if element['sourceNode'].rsplit('.', 1)[-1] in (
                        'particlespriteemitter_11', 'particlespriteemitter_12', 'particlespriteemitter_13'):
                    required = next(m for m in element['sourceRecipe']['modules']
                        if m['className'] == 'particlemodulerequired')
                    assert not any(v['propertyPath'] == 'emitterloops' for v in required['literals'])
                    # Same omitted Required integer default restored by repair_match_card:
                    # zero repeats the sustained shield while the buff owns its lifetime.
                    element['sourceRecipe']['emitterLoopCount'] = 0
        documents.append(doc)
    # Preserve the user's repaired flight, 1.5x dice geometry and real-rig TRS.
    documents.append(renamed(effect(prefix + 'card.match.dice.diamond.explosion'),
        prefix + 'card.match.dice.full.restore', '주사위 준비·다이아 폭발 원본'))
    for stage, suffix, name in ((1, 'prepare.full.restore', '카드 준비 원본 전체'),
                               (2, 'emit.full.restore', '카드 출력 원본 전체')):
        document = native_stage(4219840, stage)
        for element in document['elements']:
            if not element.get('runtimeCarrier'):
                continue
            source_path = element['sourcePresentation']['sourceObjectPath']
            original = next(e for e in effect(prefix + 'source.' + source_path.rsplit('.', 1)[0])['elements']
                if e['sourcePresentation'].get('sourceObjectPath', e['sourceNode'].split('|')[-1]) == source_path)
            # The old action projection defaulted every emitter to a sprite.
            # Preserve the admitted leaf's actual TypeData/carrier pairing.
            element['kind'] = original['kind']
            element['sourceRecipe']['rendererShape'] = original['sourceRecipe']['rendererShape']
            element['detail']['trail'] = copy.deepcopy(original['detail']['trail'])
        documents.append(renamed(document, prefix + 'card.match.' + suffix, name))
    # Reuse the existing source tracer projection, remove its authored vertex
    # bursts/explosion, and restore the exact source smoke material (native3008).
    star = effect(prefix + 'gate1.stagger.star.group')
    star['elements'] = [e for e in star['elements'] if '.explosion' not in e['groupId']]
    assert len(star['elements']) == 28
    donor = effect(prefix + 'gate1.charge_counter.fx')
    material = next(e['material'] for e in donor['elements']
        if e['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-3008-native.v1')
    original_path = material['sourceMaterialPath']
    restored = 0
    for e in star['elements']:
        if e['groupId'].endswith('.star.lines') and e['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-2992-native.v1':
            e['material'] = copy.deepcopy(material)
            e['displayName'] = e['displayName'].replace('smoke*', 'smoke')
            for module in e['sourceRecipe']['modules']:
                for value in module['literals']:
                    if value['propertyPath'] == 'material.objectpath': value['value'] = original_path
            restored += 1
    assert restored == 5
    star = renamed(star, prefix + 'gate1.stagger.star.draw.full.restore', '무력화 원본 별 그리기')
    star['sourceModelPreview'] = current_preview(1)
    documents.append(star)
    source = read(output / 'source/MN_RPCT_07.action-effects.json')
    row = next(n for a in source['actions'] if a['actionId'] == 4219951
        for s in a['stages'] for n in s['notifies'] if n['notifyId'] == 'action-4219951/stage-004/notify-003')
    ghost = decode_trail_ghost(row)
    assert abs(ghost['durationSeconds'] - .05) < 1e-7
    after = effect(prefix + 'gate1.charge_counter.afterimage')
    cue = copy.deepcopy(after['modelCues'][0])
    cue.update(cueId='saydon.backstep.source-ghost', clipName='rpct00_att_battle_34_04',
        durationSeconds=.55, colorMultiply=[1, 1, 1, .38])
    cue['afterimage'].update(emissionStartSeconds=0, emissionEndSeconds=ghost['durationSeconds'],
        sampleIntervalSeconds=.005, sampleLifetimeSeconds=ghost['sampleLifetimeSeconds'], maxSamples=64,
        sourceColorIntensity=ghost['sourceColorIntensity'], captureInitialPose=True)
    after = renamed(after, prefix + 'gate1.backstep.afterimage', '백스텝 흰색 실제 모델 잔상')
    after['modelCues'] = [cue]
    after['sourceModelPreview'] = current_preview(102)
    documents.append(after)

    for doc in documents:
        asset = doc['effectAssetId']
        refs = {r['assetId'] for e in doc['elements'] for r in
            e.get('resources', []) + e['material']['sourceProfile'].get('textures', [])}
        refs.update(c['modelAssetId'] for c in doc.get('modelCues', []))
        assert all((ROOT / 'Client/Bin/Resources' / ref).is_file() for ref in refs), asset
        assert all(e['material']['sourceProfile']['enabled'] for e in doc['elements'])
        assert len({e['id'] for e in doc['elements']}) == len(doc['elements'])
        assert len(doc['displayName'].encode()) <= 64
        relative = 'Data/Effects/Authored/' + asset + '.effect.json'
        live = ROOT / relative
        if live.exists():
            before = live.read_bytes()
            (output / 'before' / relative).parent.mkdir(parents=True, exist_ok=True)
            (output / 'before' / relative).write_bytes(before)
        else: before = None
        write(output / 'candidate' / relative, doc)
        duration = duration_ms(doc) if doc['elements'] else 550
        resource = dict(resourceId='kakulsaydon.effect.' + sha(asset.encode())[:20],
            displayName=doc['displayName'], kind='EFFECT', assetId=asset,
            defaultAnchorKind='BOSS', soundEvent='', resourceKind='V1_EFFECT', elementId='', durationMs=duration,
            shape='BOX', colliderKind='GEOMETRY', halfExtents=[1, 1, 1], radiusM=3, halfAngleDegrees=45)
        category = ['KoukuSaydon', '1관문', '패턴', '세이튼', '카드 짝 맞추기' if 'card.match' in asset else
            ('백스텝 후 불 뿜기' if 'backstep' in asset else '세이튼_무력화 시작')]
        registrations.append(dict(effectAssetId=asset, displayName=doc['displayName'], path=relative,
            candidatePath=(output / 'candidate' / relative).relative_to(ROOT).as_posix(),
            candidateSha256=sha((output / 'candidate' / relative).read_bytes()),
            beforeSha256=sha(before) if before is not None else None,
            beforeExists=before is not None, durationMs=duration, elements=len(doc['elements']),
            categoryPath=category, compositionResource=resource))

    by_asset = {r['effectAssetId']: r['compositionResource'] for r in registrations}
    def patch(pattern, occurrence, values):
        before, = [b for b in patterns[pattern]['presentationOccurrences'] if b['occurrenceId'] == occurrence]
        changes = [dict(fieldPath=[k], before=before.get(k), beforeExists=k in before, after=v)
            for k, v in values.items() if before.get(k) != v]
        if changes: edits.append(dict(collection='patterns', stableKey='patternId', stableId=pattern,
            childCollection='presentationOccurrences', childStableKey='occurrenceId', childStableId=occurrence,
            changes=changes))
    def replace_resource(n, occurrence, suffix, **values):
        pattern = 'KAKULSAYDON_G1_PATTERN_' + str(n)
        patch(pattern, pattern + '.presentation.' + str(occurrence),
            dict(resourceId=by_asset[prefix + suffix]['resourceId'], **values))
    replace_resource(78, 1, 'card.match.dice.full.restore')
    for n in (13, 30, 31, 32): replace_resource(78, n, 'card.match.emit.full.restore')
    p78 = patterns['KAKULSAYDON_G1_PATTERN_78']
    prep = copy.deepcopy(p78['presentationOccurrences'][0])
    prep_ordinal = p78['nextPresentationOccurrenceOrdinal']
    prep.update(occurrenceId=f'KAKULSAYDON_G1_PATTERN_78.presentation.{prep_ordinal}',
        resourceId=by_asset[prefix + 'card.match.prepare.full.restore']['resourceId'],
        startMs=5167, durationMs=by_asset[prefix + 'card.match.prepare.full.restore']['durationMs'],
        positionOffset=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
    additions.append(dict(collection='patterns', stableKey='patternId', stableId=p78['patternId'],
        childCollection='presentationOccurrences', childStableKey='occurrenceId', value=prep))
    edits.append(dict(collection='patterns', stableKey='patternId', stableId=p78['patternId'], changes=[
        dict(fieldPath=['nextPresentationOccurrenceOrdinal'], before=prep_ordinal, beforeExists=True, after=prep_ordinal + 1)]))
    for n in (1, 16): replace_resource(1, n, 'gate1.stagger.shield.full.restore',
        fitEffectToDuration=False, loopEffectToDuration=True)
    replace_resource(1, 2, 'gate1.stagger.star.draw.full.restore', startMs=2267, durationMs=10694,
        positionOffset=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
    replace_resource(1, 27, 'gate1.stagger.explosion.full.restore',
        durationMs=by_asset[prefix + 'gate1.stagger.explosion.full.restore']['durationMs'])
    p1 = patterns['KAKULSAYDON_G1_PATTERN_1']
    for n in (*range(3, 16), *range(17, 22)):
        identity = p1['patternId'] + '.presentation.' + str(n)
        before, = [b for b in p1['presentationOccurrences'] if b['occurrenceId'] == identity]
        assert resources[before['resourceId']]['assetId'].startswith('boss.kouku.disarm.') or (
            n == 3 and resources[before['resourceId']]['assetId'] == 'boss.kouku.blur_1')
        removals.append(dict(collection='patterns', stableKey='patternId', stableId=p1['patternId'],
            childCollection='presentationOccurrences', childStableKey='occurrenceId', childStableId=identity, before=before))
    placements = read(ROOT / 'Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json')
    # The canonical boss spawn positions are the authored centers for these arenas.
    actors = placements.get('placements', placements.get('actors', []))
    if not actors:
        actors = placements['bosses']
    centers = []
    for gate in (1, 3):
        actor, = [a for a in actors if a.get('placementId') == f'boss.kakulsaydon.g{gate}.saydon']
        centers.append(actor['position'])
    delta = [centers[0][i] - centers[1][i] for i in range(3)]
    p102 = patterns['KAKULSAYDON_G1_PATTERN_102']
    rings = [b for b in p102['presentationOccurrences'] if
        resources[b['resourceId']]['assetId'] == prefix + 'gate3.backstep.ring.flame']
    assert len(rings) == 7
    for b in rings:
        assert b['anchorKind'] == 'MAP' and not b['followBoss']
        patch(p102['patternId'], b['occurrenceId'],
            dict(positionOffset=[b['positionOffset'][i] + delta[i] for i in range(3)]))
    required_duration = max(b['startMs'] + (by_asset[prefix + 'gate1.stagger.explosion.full.restore']['durationMs']
        if b['occurrenceId'].endswith('.presentation.27') else b['durationMs']) for b in p1['presentationOccurrences'])
    if required_duration > p1.get('durationMs', 0):
        edits.append(dict(collection='patterns', stableKey='patternId', stableId=p1['patternId'], changes=[
            dict(fieldPath=['durationMs'], before=p1.get('durationMs'), beforeExists='durationMs' in p1, after=required_duration)]))
    write(output / 'source/buff-receipts.json', [buff_receipt(n) for n in
        ('KoukuSaton_Star_Jail', 'Reflect_Shield_Front', 'Reflect_Shield_Rear')])
    write(output / 'source/backstep-ghost-receipt.json', dict(notifyId=row['notifyId'], decoded=ghost,
        appearance='PROJECT_AUTHORED white/translucent on actual model; original rim/fade ABI not recovered'))
    write(output / 'manifest.json', dict(installed=False, documents=registrations, resourcesToInstall=[],
        compositionPatch=dict(path=COMPOSITION, beforeSha256=sha(raw), beforeRevision=composition['revision'],
            stableEdits=edits, stableAdditions=additions, stableRemovals=removals,
            resourceAdditions=[r['compositionResource'] for r in registrations]),
        backstepArenaCenters=centers, backstepArenaDelta=delta, correctedRingCount=len(rings),
        nativeAdmissionRequired=dict(programId=3008, carrier='sprite', restoredElementCount=5),
        existingDiceSourceElements=53, cardEmitBeforeElements=6, cardEmitAfterElements=19,
        manualVisualValidation='USER_PENDING'))
    assert (ROOT / COMPOSITION).read_bytes() == raw, 'Concurrent Composition save; rerun to restage guards.'
    print(json.dumps(dict(documents=len(registrations), edits=len(edits), additions=len(additions),
        removals=len(removals), delta=delta, manifest=str(output / 'manifest.json'))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=OUT)
    prepare(parser.parse_args().output)
