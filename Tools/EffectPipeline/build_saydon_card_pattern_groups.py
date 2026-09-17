"""Assemble Saydon card/trumpet groups from installed source occurrences.

Writes candidates only. The live Composition and the user's edited leaves are
never rewritten by this builder. Source timings remain separate from the
explicit 6114/4184/6637 ms sequencing requested for the current patterns.
"""
from __future__ import annotations

import copy
import hashlib
import json
import math
from pathlib import Path

import build_kouku_gate1_full_restore as source
from build_kouku_backstep_flame_groups import project_mesh_rotation, remap

ROOT = source.ROOT
AUTHORED = ROOT / 'Data/Effects/Authored'
OUTPUT = ROOT / 'out/sayton-pattern-20260917/effects'
STAGES = ROOT / 'out/KoukuActionEffects20260912/candidate'
PREFIX = 'effect.kouku.card.match.'
SUITS = (('heart', '하트', 5, 2), ('clover', '클로버', 4, 0),
         ('diamond', '다이아', 6, 1), ('spade', '스페이드', 3, 3))


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2,
                               allow_nan=False) + '\n', encoding='utf-8')


def leaf(suffix):
    return read(AUTHORED / ('effect.kouku.source.fx_mn_rpct_05_l.'
                           'par_l_rpct_05_sk_' + suffix + '.effect.json'))


def renamed(document, asset, name):
    result = copy.deepcopy(document)
    result.update(effectAssetId=asset, displayName=name)
    if result['version'] == 15:
        result.setdefault('runtimeExtensions', dict(formatVersion=1, bakedEdgeHistories=[]))
    mapping = {e['id']: 'saydon.' + hashlib.sha256(
        (asset + '/' + e['id']).encode()).hexdigest()[:24]
        for e in result['elements']}
    result['elements'] = remap(result['elements'], mapping)
    for e in result['elements']:
        e['groupId'] = asset
    return result


def native_stage(action, ordinal):
    document = read(STAGES / f'effect.kouku.action.mn_rpct_05.{action}.stage{ordinal:03}.effect.json')
    libraries = {}
    for element in document['elements']:
        emitter = element['sourcePresentation']['sourceObjectPath']
        system = emitter.rsplit('.', 1)[0]
        if system not in libraries:
            library = read(AUTHORED / ('effect.kouku.source.' + system + '.effect.json'))
            libraries[system] = {e['sourcePresentation'].get('sourceObjectPath',
                e.get('sourceNode', '').split('|')[-1]): e for e in library['elements']}
        original = libraries[system][emitter]
        assert original['material']['sourceProfile']['enabled']
        assert original['material']['sourceMaterialPath'] == element['material']['sourceMaterialPath']
        element['material'] = copy.deepcopy(original['material'])
        element['resources'] = copy.deepcopy(original['resources'])
        if 'runtimeCarrier' in original:
            element['runtimeCarrier'] = copy.deepcopy(original['runtimeCarrier'])
        project_mesh_rotation(element)
    return document


def key(time, alpha):
    return dict(timeSeconds=time, value=[alpha] * 3, arriveTangent=[0] * 3,
                leaveTangent=[0] * 3, interpolation='constant')


def append_group(document, original, name, delay=0, cut=None):
    copied = renamed(original, document['effectAssetId'] + '.' + name, name)
    for element in copied['elements']:
        element['detail']['timing']['startDelaySeconds'] += delay
        presentation = element.get('sourcePresentation', {})
        if 'sourceTimeSeconds' in presentation:
            presentation['sourceTimeSeconds'] += delay
        if cut is not None:
            element['detail']['timing']['lifeTimeSeconds'] = min(
                element['detail']['timing']['lifeTimeSeconds'], cut - delay)
            assert 'sourceTransformTrack' not in element
            element['sourceTransformTrack'] = dict(sourceOccurrenceId=element['id'],
                sourceTimeOriginSeconds=0, previewOriginUE3Cm=[0, 0, 0],
                nodes=[dict(sourceObjectPath=element['id'], frame='WORLD',
                    initialPositionUE3Cm=[0, 0, 0], initialEulerDegrees=[0, 0, 0],
                    scaleUE3=[1, 1, 1], positionKeys=[], eulerKeys=[])],
                alphaScaleKeys=[key(delay, 1), key(cut, 0)])
        document['elements'].append(element)


def current_preview(pattern_ordinal):
    composition = read(ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json')
    pattern = next(p for p in composition['patterns']
                   if p['patternId'] == f'KAKULSAYDON_G1_PATTERN_{pattern_ordinal}')
    animations, offset = [], 0
    for stage in pattern['stages']:
        for occurrence in stage['animationOccurrences']:
            animation = {k: occurrence[k] for k in ('runtimeClip', 'startOffsetMs',
                'sourceStartMs', 'playMs', 'playRate', 'endPolicy')}
            animation['startOffsetMs'] += offset
            animations.append(animation)
        offset += stage['durationMs']
    return dict(gateId=pattern['gateId'], actorProfileId=pattern['actorProfileId'],
                targetBossPlacementId=pattern['targetBossPlacementId'], animations=animations)


def fixed_subimage(element, tile):
    recipe = element['sourceRecipe']
    required = next(m for m in recipe['modules'] if m['className'] == 'particlemodulerequired')
    mode = next(v for v in required['literals'] if v['propertyPath'] == 'interpolationmethod')
    assert mode['value'] == 'psuvim_random'
    mode['value'] = 'psuvim_linear'
    module = next(m for m in recipe['modules'] if m['className'] == 'particlemodulesubuv')
    distribution = next(d for d in module['distributions'] if d['propertyPath'] == 'subimageindex')
    distribution.update(lookupTable=[tile] * 4, lookupTableTimeScale=0,
                        lookupTableStartTime=0, keys=[])


def duration_ms(document):
    return math.ceil(max(e['detail']['timing']['startDelaySeconds'] +
        e['detail']['timing']['lifeTimeSeconds'] +
        e['detail']['timing']['afterImageSeconds'] +
        max(e['detail']['particle']['lifeTimeSeconds'])
        for e in document['elements']) * 1000)


def build():
    documents, rows = [], []
    dice = renamed(native_stage(4219840, 0), PREFIX + 'dice.diamond.explosion', '주사위 다이아 폭발')
    documents.append((dice, '카드 짝 맞추기'))

    # Each source Trace projectile already supplies the matching card mesh,
    # symbol, local-space particles and exact per-suit native material inputs.
    # One repeatable second is the visual clock, not a gameplay expiration.
    for suit, label, suffix, tile in SUITS:
        card = renamed(leaf(f'08_{suffix}_loc_int'), PREFIX + suit, label)
        card.pop('sourceModelPreview', None)
        card.pop('sourceAnchorAnimations', None)
        for element in card['elements']:
            project_mesh_rotation(element)
        documents.append((card, '카드 짝 맞추기'))

    explosion = renamed(leaf('08_7_loc_int'), PREFIX + 'explosion', '카드폭발')
    documents.append((explosion, '카드 짝 맞추기'))

    mouth = renamed(native_stage(4219840, 2), PREFIX + 'emit', '카드출력 이펙트')
    mouth['elements'] = [e for e in mouth['elements']
                         if '.par_l_rpct_05_sk_08_loc_int.' in e['sourceNode']]
    assert len(mouth['elements']) == 6
    documents.append((mouth, '카드 짝 맞추기'))

    # The six remaining elements are the user's saved edit. Do not restore
    # removed source mesh/emitters or change the saved transforms.
    spin_leaf = leaf('13_1_loc_int')
    for suit, label, suffix, tile in SUITS:
        spin = renamed(spin_leaf, 'effect.kouku.card.spinning.' + suit, '회전 카드_' + label)
        spin.pop('sourceModelPreview', None)
        spin.pop('sourceAnchorAnimations', None)
        card = next(e for e in spin['elements'] if any(
            t['assetId'].endswith('/fx_l_symbol_45.dds')
            for t in e['material']['sourceProfile'].get('textures', [])))
        fixed_subimage(card, tile)
        documents.append((spin, '세이튼_회전하며 카드 날리기'))

    trumpet = renamed(read(AUTHORED / 'effect.kouku.common.trumpet.radial.lasers.effect.json'),
                      'effect.kouku.common.trumpet.card.floor', '트럼펫_카드장판')
    trumpet['elements'] = []
    trumpet['sourceModelPreview'] = current_preview(47)
    append_group(trumpet, read(AUTHORED / 'effect.kouku.common.trumpet.radial.lasers.effect.json'),
                 'lasers', 4.184, 6.637)
    append_group(trumpet, read(AUTHORED / 'effect.kouku.common.trumpet.suit.floor.effect.json'),
                 'floor', 4.184, 6.637)
    # The original one-second suit particles disappear before the requested
    # 2.453-second floor interval. Preserve their recipe/material curves and
    # apply the existing authored lifetime multiplier to this floor group only.
    # The absolute alpha cutoff still retires every floor layer at impact.
    for element in trumpet['elements']:
        if element['groupId'].endswith('.floor'):
            element['detail']['particle']['sourceScale']['lifeTime'] = 2.453
    append_group(trumpet, leaf('06_2_loc_int'), 'impact', 6.637)
    documents.append((trumpet, '트럼펫장판소환'))

    for document, category in documents:
        assert len({e['id'] for e in document['elements']}) == len(document['elements'])
        assert len(document['displayName'].encode('utf8')) <= 64
        resources = set()
        for element in document['elements']:
            assert element['material']['sourceProfile']['enabled']
            for row in element['resources'] + element['material']['sourceProfile'].get('textures', []):
                resources.add(row['assetId'])
        missing = [p for p in resources if not (ROOT / 'Client/Bin/Resources' / p).is_file()]
        assert not missing, (document['effectAssetId'], missing)
        path = 'Data/Effects/Authored/' + document['effectAssetId'] + '.effect.json'
        write(OUTPUT / 'candidate' / Path(path).name, document)
        looping_card = document['effectAssetId'] in {
            prefix + suit for prefix in (PREFIX, 'effect.kouku.card.spinning.')
            for suit, _, _, _ in SUITS}
        rows.append(dict(effectAssetId=document['effectAssetId'], displayName=document['displayName'],
            path=path, durationMs=1000 if looping_card else duration_ms(document), elementCount=len(document['elements']),
            defaultAnchorKind='BOSS', followBoss=bool(document.get('sourceModelPreview')),
            categoryPath=['KoukuSaydon', '1관문', '패턴', '세이튼', category]))
    write(OUTPUT / 'installation.json', dict(installed=False, documents=rows,
        sourceActionDice=4219840, sourceDiceClip='rpct00_att_battle_11_02',
        sourceTraceProjectiles=[421980901, 421980902, 421980903, 421980904],
        sourceAtlas=dict(assetId='Effect/KoukuSaydon/Textures/FX_TEX_HIGH_03/fx_l_symbol_45.dds',
                         tileOrder=['clover', 'diamond', 'heart', 'spade'], turnsPerSecond=3),
        requestedTimingMs=dict(cardEmit=6114, trumpetStart=4184, trumpetImpact=6637),
        requestedTrumpetFloorLifetimeMultiplier=2.453,
        cardLifetimePolicy='Server-owned lifetime; Client repeats source presentation while alive.',
        sourceMouthSocket='FX_Prj_01 / b_wp_1 (original action attachment)',
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(candidates=len(rows), elements=sum(r['elementCount'] for r in rows),
                          manifest=str(OUTPUT / 'installation.json'))))


if __name__ == '__main__':
    build()
