"""Apply the requested Saydon timing to saved authoring with field-level CAS.

Effect groups must first pass the native codec and be registered in the library.
Existing clip chains, occurrence transforms and unrelated user edits are kept.
The command writes an evidence draft by default; --install replaces the saved
Composition only if no external writer changed the bytes read at entry.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
PATH = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
OUTPUT = ROOT / 'out/sayton-pattern-20260917'


def encode(value):
    return json.dumps(value, ensure_ascii=False, allow_nan=False)


def field_span(text, key, indent):
    match = re.search(r'^' + ' ' * indent + '"' + re.escape(key) + r'"\s*:\s*', text, re.M)
    assert match, key
    _, length = json.JSONDecoder().raw_decode(text[match.end():])
    return match.end(), match.end() + length


def replace_field(text, key, value, indent):
    start, end = field_span(text, key, indent)
    return text[:start] + encode(value) + text[end:]


def rows(text, key):
    start, end = field_span(text, key, 2)
    position = start + 1
    while position < end:
        while text[position] in ' \r\n\t,':
            position += 1
        if text[position] == ']':
            break
        value, length = json.JSONDecoder().raw_decode(text[position:])
        yield position, position + length, value
        position += length


def replace_row_fields(text, array, identity_key, identity, updates):
    start, end, row = next(row for row in rows(text, array) if row[2][identity_key] == identity)
    block = text[start:end]
    for key, value in updates.items():
        if key in row:
            block = replace_field(block, key, value, 6)
        else:
            block = block[:-1].rstrip() + ',\n      "' + key + '": ' + encode(value) + '\n    }'
    return text[:start] + block + text[end:]


def append_row(text, array, value):
    start, end = field_span(text, array, 2)
    previous = text[start:end - 1].rstrip()
    return text[:start] + previous + (',' if previous[-1] != '[' else '') + '\n    ' + encode(value) + '\n  ' + text[end - 1:]


def resource_id(asset):
    return 'kakulsaydon.effect.' + hashlib.sha256(asset.encode()).hexdigest()[:20]


def apply(install=False):
    before = PATH.read_bytes()
    text = before.decode('utf-8-sig')
    document = json.loads(text)
    patterns = {p['patternId']: copy.deepcopy(p) for p in document['patterns']}
    resources = {r['assetId']: r for r in document['presentationResources'] if r['kind'] == 'EFFECT'}
    prefix = 'effect.kouku.card.match.'
    suits = ['heart', 'clover', 'diamond', 'spade']
    contact = resource_id(prefix + 'explosion')
    assert resources[prefix + 'explosion']['resourceId'] == contact
    for stem in [prefix, 'effect.kouku.card.spinning.']:
        for suit in suits:
            assert resources[stem + suit]['durationMs'] == 1000
    updates = {}
    def pattern(n):
        p = patterns[f'KAKULSAYDON_G1_PATTERN_{n}']
        updates.setdefault(p['patternId'], {})
        return p

    def present(p, asset, start, duration):
        assert not any(o['resourceId'] == resources[asset]['resourceId'] for o in p['presentationOccurrences'])
        ordinal = p['nextPresentationOccurrenceOrdinal']
        p['presentationOccurrences'].append(dict(occurrenceId=p['patternId'] + '.presentation.' + str(ordinal),
            resourceId=resources[asset]['resourceId'], startMs=start, durationMs=duration,
            positionOffset=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1],
            fadeInMs=0, fadeOutMs=0, dissolveStart=.95, dissolveEnd=1, brightnessMultiplier=1,
            volume=1, followBoss=True, debugRender=True, bone='', boneTarget='BODY', regionId='',
            cardSymbol='NONE', cardColor='NONE', anchorKind='BOSS', worldId='', logicOccurrenceId='', worldOccurrenceId=''))
        p['nextPresentationOccurrenceOrdinal'] = ordinal + 1
        updates[p['patternId']].update(presentationOccurrences=p['presentationOccurrences'],
                                      nextPresentationOccurrenceOrdinal=ordinal + 1)

    p = pattern(47)
    present(p, 'effect.kouku.common.trumpet.card.floor', 0, sum(s['durationMs'] for s in p['stages']))
    p = pattern(78)
    present(p, prefix + 'dice.diamond.explosion', 0, resources[prefix + 'dice.diamond.explosion']['durationMs'])
    present(p, prefix + 'emit', 6114, 1333)
    logic = next(l for l in document['logics'] if l['logicId'] == 'kakulsaydon.g1.logic.73')
    assert logic.get('judgementKind', '') in ('', 'PURSUIT_PROJECTILES')
    fields = dict(judgementKind='PURSUIT_PROJECTILES', visualIds=[resource_id(prefix + suit) for suit in suits],
        contactVisualId=contact, speedMps=3, contactRadiusM=.5, spawnRadiusM=3,
        lifetimeMs=0, spawnIntervalMs=0, homing=True, countPerWave=4)
    text = replace_row_fields(text, 'logics', 'logicId', logic['logicId'], fields)
    box = next(b for b in p['logicOccurrences'] if b['logicId'] == logic['logicId'])
    box.update(startMs=6114, durationMs=sum(s['durationMs'] for s in p['stages']) - 6114)
    updates[p['patternId']]['logicOccurrences'] = p['logicOccurrences']

    p = pattern(48)
    spin_stages, stage_start = [], 0
    for stage in p['stages']:
        if any(a['runtimeClip'] == 'rpct00_att_battle_4_02' for a in stage['animationOccurrences']):
            spin_stages.append((stage_start, stage_start + stage['durationMs']))
        stage_start += stage['durationMs']
    assert spin_stages and all(a[1] == b[0] for a, b in zip(spin_stages, spin_stages[1:]))
    spin_start, spin_end = spin_stages[0][0] + 700, spin_stages[-1][1]
    assert spin_end > spin_start
    ordinal = document['nextLogicOrdinal']
    spinning_id = 'kakulsaydon.g1.logic.' + str(ordinal)
    spin = dict(logicId=spinning_id, displayName='세이튼_회전카드_클립구간연속발사', logicType='DURATION',
        judgementKind='PURSUIT_PROJECTILES', visualIds=[resource_id('effect.kouku.card.spinning.' + suit) for suit in suits],
        contactVisualId=contact, speedMps=8, maxDistanceM=15, contactRadiusM=.4, spawnRadiusM=.2,
        lifetimeMs=5000, spawnIntervalMs=300, homing=False, countPerWave=3)
    text = append_row(text, 'logics', spin)
    text = replace_field(text, 'nextLogicOrdinal', ordinal + 1, 2)
    occurrence_ordinal = p['nextLogicOccurrenceOrdinal']
    assert not p['logicOccurrences']
    p['logicOccurrences'].append(dict(occurrenceId=p['patternId'] + '.logic.' + str(occurrence_ordinal),
        logicId=spinning_id, startMs=spin_start, durationMs=spin_end - spin_start,
        enabled=True, onSuccessLogicIds=[], onFailLogicIds=[], onTimeoutLogicIds=[]))
    updates[p['patternId']].update(logicOccurrences=p['logicOccurrences'],
        nextLogicOccurrenceOrdinal=occurrence_ordinal + 1)

    p = pattern(79)
    wand = next(b for b in p['presentationOccurrences'] if b['resourceId'] == resources['effect.kouku.firecross.wand.spin']['resourceId'])
    wand['startMs'] = 3596
    updates[p['patternId']]['presentationOccurrences'] = p['presentationOccurrences']
    for identity, fields in updates.items():
        text = replace_row_fields(text, 'patterns', 'patternId', identity, fields)
    text = replace_field(text, 'revision', document['revision'] + 1, 2)
    after = json.loads(text)
    for old in document['patterns']:
        new = next(p for p in after['patterns'] if p['patternId'] == old['patternId'])
        assert new['stages'] == old['stages']
        assert new == dict(old, **updates.get(old['patternId'], {}))
    OUTPUT.mkdir(parents=True, exist_ok=True)
    (OUTPUT / 'timing-composition.candidate.json').write_text(text, encoding='utf8')
    assert PATH.read_bytes() == before, 'Concurrent Composition edit; candidate preserved.'
    if install:
        PATH.write_text(text, encoding='utf8', newline='')
    evidence = dict(installed=install, sourceTrace=dict(projectiles=[421980901,421980902,421980903,421980904],
        speedMps=3, maxSpeedMps=3, radiusM=.5, maxDistanceM=30, lifetimeMs=[10000,20000,10000,10000]),
        sourceSpinning=dict(actionId=4219819, projectileId=421981901, speedMps=8, maxSpeedMps=15, radiusM=.4,
            maxDistanceM=15, lifetimeMs=5000, sourceVolleys=2, sourceWavesPerVolley=4, shotsPerWave=3, intervalMs=300),
        requestedOverrides=dict(cardSpawnMs=6114, traceLifetimeMs=0, traceSpawnRadiusM=3, spinDirection='SERVER_SEEDED_RANDOM',
            trumpetStartMs=4184, trumpetImpactMs=6637, wandSpinStartMs=3596,
            spinWindowMs=[spin_start, spin_end], spinWaves=len(range(spin_start, spin_end, 300)),
            motionPolicy='Constant source initial Speed with source lifetime/distance caps; no inferred native acceleration.'),
        changedPatternIds=list(updates), manualVisualValidation='USER_PENDING')
    (OUTPUT / 'timing-installation.json').write_text(json.dumps(evidence, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(encode(dict(installed=install, revision=after['revision'], patterns=list(updates))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--install', action='store_true')
    apply(parser.parse_args().install)
