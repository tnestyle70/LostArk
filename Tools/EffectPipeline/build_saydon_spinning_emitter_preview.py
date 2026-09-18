"""Build one native sprite emitter and shared death bursts, without live writes."""
from __future__ import annotations

import argparse
import copy
import hashlib
import struct
from pathlib import Path

from build_saydon_card_pattern_groups import (AUTHORED, ROOT, current_preview,
    leaf, read, renamed, scale_spinning_card_geometry, follow_local_card_axes, write)

OUTPUT = ROOT / 'out/EffectV1Review20260917/candidate'
ASSET = 'effect.kouku.card.spinning.emitter'
COMPOSITION = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
ACTION = Path('C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_05.action-effects.json')
PROJECTILE = ROOT / 'out/KoukuAllEffects20260912/source/Projectile/421981901.loa'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def literal(path, value):
    return dict(propertyPath=path, kind='boolean' if isinstance(value, bool) else
        'string' if isinstance(value, str) else 'number', value=value)


def constant(path, value):
    return dict(propertyPath=path, sourceClass='', sourceObjectPath='',
        componentCount=1, operation=1, randomLockAxes=0, lookupTableChunkSize=1,
        lookupTableNumElements=1, lookupTableTimeScale=0, lookupTableStartTime=0,
        defaultMinimum=[0]*4, defaultMaximum=[0]*4, lookupTable=[value]*4, keys=[])


def module(name, class_name, values, distributions=()):
    identity = ASSET + '.' + name
    return dict(stableId=identity, className=class_name, objectPath=identity,
        literals=[literal('benabled', True)] + [literal(k, v) for k, v in values.items()],
        distributions=list(distributions))


def emission(element, start, duration, bursts):
    element['detail']['timing'].update(startDelaySeconds=start, lifeTimeSeconds=duration)
    recipe = element['sourceRecipe']
    recipe.update(emitterDelaySeconds=0, emitterDurationSeconds=duration,
        emitterLoopCount=1, bursts=copy.deepcopy(bursts))
    for source_module in recipe['modules']:
        if source_module['className'] == 'particlemodulerequired':
            for item in source_module['literals']:
                if item['propertyPath'] == 'emitterduration':
                    item['value'] = duration
        if source_module['className'] == 'particlemodulespawn':
            source_module['literals'] = [item for item in source_module['literals']
                if not item['propertyPath'].startswith('burstlist[')]
            for i, burst in enumerate(bursts):
                source_module['literals'] += [literal(f'burstlist[{i}].{field}', value)
                    for field, value in [('count', burst['countMaximum']),
                        ('countlow', burst['countMinimum']), ('time', burst['timeSeconds'])]]


def source_schedule():
    composition = read(COMPOSITION)
    preview = current_preview(48)
    clips = [a for a in preview['animations'] if a['runtimeClip'] == 'rpct00_att_battle_4_02']
    assert clips and all(a['playRate'] == 1 for a in clips)
    start = clips[0]['startOffsetMs']
    end = clips[-1]['startOffsetMs'] + clips[-1]['playMs']
    assert all(a['startOffsetMs'] + a['playMs'] == b['startOffsetMs'] for a, b in zip(clips, clips[1:]))
    action = next(a for a in read(ACTION)['actions'] if a['actionId'] == 4219819)
    notifies = [n for n in action['stages'][1]['notifies'] if n['sourceType'] == 'Effect' and n['localTimeSeconds'] > .6]
    times = [round(n['localTimeSeconds'] * 1000) for n in notifies]
    unique = sorted(set(times))
    assert unique == [700, 1000, 1300, 1600] and all(times.count(t) == 3 for t in unique)
    raw = PROJECTILE.read_bytes()
    scale, radius, height, speed, max_speed, life, distance = struct.unpack_from('<fff i i f i', raw, 1197)
    assert (scale, radius, height, speed, max_speed, life, distance) == (1, 40, 30, 800, 1500, 5, 1500)
    shots = []
    for wave, at in enumerate(range(start + unique[0], end, unique[1] - unique[0])):
        for lane in range(3):
            index = len(shots)
            shots.append(dict(index=index, wave=wave, startSeconds=at / 1000,
                speedMps=speed / 100,
                flightSeconds=distance / speed, distanceM=distance / 100))
    return preview, shots, dict(compositionRevision=composition['revision'],
        compositionSha256=digest(COMPOSITION), sourceActionSha256=digest(ACTION),
        sourceProjectileSha256=digest(PROJECTILE), sourceMotionOffset=1197,
        sourceMotion=dict(resScale=scale, collisionRadiusCm=radius, heightCm=height,
            speedCmPerSecond=speed, maxSpeedCmPerSecond=max_speed, lifetimeSeconds=life,
            maxDistanceCm=distance), sourceWaveTimesMs=unique, sourceShotsPerWave=3,
        sourceStageTransitionMs=4000, currentSpinWindowMs=[start, end],
        requestedExtension='Continue source 300ms cadence to the current contiguous spin window end.',
        directionPolicy='PROJECT_RANDOM_HORIZONTAL_VELOCITY_CONE; existing module particle RNG', previewFlightPolicy='Constant source initial Speed; stop at MaxDistance and preview burst. No inferred acceleration.',
        sourceModelPreview=preview, fullShotSchedule=shots)


def build(count_multiplier=2, output=OUTPUT):
    if count_multiplier not in (1, 2):
        raise ValueError('Card count multiplier must be 1 (source) or 2 (requested).')
    preview, shots, proof = source_schedule()
    original = leaf('13_1_loc_int')
    assert len(original['elements']) == 6
    explosion = read(AUTHORED / 'effect.kouku.card.match.explosion.effect.json')
    document = renamed(original, ASSET, '회전 카드 발사')
    symbol = next(e for e in document['elements'] if any(t['assetId'].endswith('/fx_l_symbol_45.dds')
        for t in e['material']['sourceProfile'].get('textures', [])))
    document['elements'] = [symbol]
    document['sourceModelPreview'] = preview
    start, end = (v/1000 for v in proof['currentSpinWindowMs'])
    bursts = [dict(timeSeconds=round(s['startSeconds']-start, 6), countMinimum=3, countMaximum=3)
        for s in shots if s['index'] % 3 == 0]
    emission(symbol, start, end-start, bursts)
    symbol['displayName'] = '원본 심볼 4종 / 무작위 방향 발사'
    symbol['groupId'] = ASSET + '.flight'
    symbol['detail']['particle']['maxParticles'] = len(shots)
    symbol['detail']['particle'].setdefault('sourceScale', {})['lifeTime'] = shots[0]['flightSeconds']
    # Keep the source three-shot bursts intact. The existing authored count
    # scale expands both births and the main emitter capacity exactly once.
    symbol['detail']['particle']['sourceScale']['count'] = count_multiplier
    event = ASSET + '.death'
    symbol['sourceRecipe']['modules'] += [module('velocity', 'particlemodulevelocitycone',
        {'bspawnmodule': True, 'binworldspace': False,
         'direction.x': 0, 'direction.y': 0, 'direction.z': 1},
        [constant('angle', 90), constant('velocity', proof['sourceMotion']['speedCmPerSecond'])]),
        module('death', 'particlemoduleeventgenerator',
        {'bspawnmodule': True, 'bupdatemodule': True, 'events[0].customname': event,
         'events[0].type': 'epet_death', 'events[0].frequency': 1,
         'events[0].firsttimeonly': False, 'events[0].lasttimeonly': False})]
    burst_doc = renamed(explosion, ASSET + '.impact', '카드 폭발 / 입자 수명 종료')
    for i, element in enumerate(burst_doc['elements']):
        count = sum(b['countMaximum'] for b in element['sourceRecipe']['bursts'])
        assert count > 0
        element['groupId'] = ASSET + '.impact'
        emission(element, 0, shots[-1]['startSeconds'] + shots[-1]['flightSeconds'] + .1, [])
        # Each additional card already sends its own death event. Keep the
        # existing capacity: it covers both authored counts' simultaneous bursts
        # without increasing the source explosion's particles per event.
        element['detail']['particle']['maxParticles'] *= len(shots)
        element['detail']['particle']['burstCount'] = 0
        inherit_velocity = constant('inheritvelocityscale', 0)
        inherit_velocity.update(componentCount=3, lookupTableChunkSize=0,
            lookupTableNumElements=0, lookupTable=[])
        element['sourceRecipe']['modules'].append(module(f'impact.{i}',
            'particlemoduleeventreceiverspawn', {'bspawnmodule': True, 'bupdatemodule': True,
                'eventname': event, 'eventgeneratortype': 'epet_death',
                'busepsyslocation': False, 'binheritvelocity': False},
            [inherit_velocity, constant('spawncount', count)]))
        document['elements'].append(element)
    scaled_cards = scale_spinning_card_geometry(document)
    follow_local_card_axes(document)
    proof.update(assetId=ASSET, installed=False, status='NATIVE_SPRITE_EMITTER_CPU_VALIDATION_PENDING',
        catalogEntry=dict(effectAssetId=ASSET, payloadKind='DIRECT_AUTHORED_DOCUMENT',
            authoringPath=f'Effects/Authored/{ASSET}.effect.json'),
        treeReference=dict(kind='V1', assetId=ASSET, displayName='회전 카드 발사',
            parentId='kouku.category.173df37e77c188aee068'),
        uiPath='KoukuSaydon / 1관문 / 패턴 / 세이튼 / 세이튼_회전하며 카드 날리기 / 회전 카드 발사',
        fullShotCount=len(shots) * count_multiplier, sourceExtendedShotCount=len(shots),
        sourceExtendedShotSchedule=shots,
        fullShotSchedule=[dict(shot, index=index * count_multiplier + copy_index,
            sourceShotIndex=index) for index, shot in enumerate(shots)
            for copy_index in range(count_multiplier)],
        authoredCountMultiplier=count_multiplier, authoredSizeMultiplier=1.5,
        sizeAdjustedElementIds=scaled_cards,
        sourceUserLeafSha256=digest(AUTHORED / 'effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_1_loc_int.effect.json'),
        requiredRuntime='Existing source sprite, velocity cone, random SubUV, death generator and receiver; no new runtime or duplicated shot groups.',
        preserved='Saved symbol local transform, native material, original random atlas and rotation modules. Source shared explosion materials/modules retained with death-only emission.',
        manualVisualValidation='USER_PENDING')
    write(output / (ASSET + '.effect.json'), document)
    write(output / 'spinning-emitter-schedule.json', proof)
    print(f'elements={len(document["elements"])} waves={len(bursts)} shots={len(shots) * count_multiplier} revision={proof["compositionRevision"]}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--count-multiplier', type=int, choices=(1, 2), default=2,
        help='Authored card count override; native bursts remain three shots.')
    parser.add_argument('--output', type=Path, default=OUTPUT)
    args = parser.parse_args()
    build(args.count_multiplier, args.output)
