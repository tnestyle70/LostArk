"""Build out-only Saydon circus World motions and source-backed V1 groups.

The source callback graph owns five splits; the user owns the +90 +/-45 degree
directions and one-bounce presentation. Existing WorldSequence group/emissions
own every object and effect clock. This adds no gameplay or network authority.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import sys

import build_kouku_circus_ball_groups as circus
from build_kouku_backstep_flame_groups import project_mesh_rotation, remap

ROOT = circus.ROOT
sys.path.insert(0, str(ROOT / 'Tools/KoukuSaydonPipeline'))
from apply_saydon_ball_world_motions import geometry

OUT = ROOT / 'out/SaydonCircusWorld20260917'
WORLD = ROOT / 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json'
AUTHORED = ROOT / 'Data/Effects/Authored'
PREFIX = 'effect.kouku.gate1.circus.'
WORLD_PREFIX = 'world.object.kouku.saydon.circus.'
MI_PREFIX = 'world.object.instance.kouku.saydon.circus.'
SQ_PREFIX = 'sequence.kouku.saydon.circus.'


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def leaf(suffix):
    return read(AUTHORED / ('effect.kouku.source.fx_mn_ppct_00.par_k_ppct_vividfracture_' + suffix + '.effect.json'))


def group(asset, name, rows):
    result = copy.deepcopy(rows[0][0])
    result.update(effectAssetId=asset, displayName=name, elements=[], modelCues=[])
    result.pop('sourceModelPreview', None)
    for ordinal, (document, delay) in enumerate(rows):
        stem = 'circus.' + hashlib.sha256((asset + str(ordinal)).encode()).hexdigest()[:20]
        ids = {element['id']: stem + '.' + str(i) for i, element in enumerate(document['elements'])}
        elements = remap(copy.deepcopy(document['elements']), ids)
        for element in elements:
            element['groupId'] = asset
            element['detail']['timing']['startDelaySeconds'] += delay
            element['sourcePresentation']['sourceTimeSeconds'] += delay
            # The native library is root-relative; WorldSequence supplies its root.
            element['actionCueAttachment']['enabled'] = False
            project_mesh_rotation(element)
            result['elements'].append(element)
    return result


def inherit_missing_defaults(document, evidence):
    """Only restore missing nested CDO fields, never author replacement artwork."""
    defaults = read(ROOT / 'out/KoukuAllEffects20260912/source_class_defaults.json')['records']
    classes = {row['fullPath']: row for row in defaults}
    for element in document['elements']:
        for module in element['sourceRecipe']['modules']:
            pair = {'particlemodulelocationdirect': ('scalefactor', 'ScaleFactor'),
                    'particlemodulesize': ('startsize', 'StartSize')}.get(module['className'])
            if not pair:
                continue
            distribution = next((d for d in module['distributions'] if d['propertyPath'] == pair[0]), None)
            if not distribution or distribution['lookupTable'] or distribution['keys']:
                continue
            values = classes['engine.default__' + module['className']]['properties'][pair[1]]['value']['properties']
            assert values['LookupTable']['value'] == [1.0] * 8
            for target, source in [('operation', 'Op'), ('lookupTableNumElements', 'LookupTableNumElements'),
                    ('lookupTableChunkSize', 'LookupTableChunkSize'), ('lookupTableTimeScale', 'LookupTableTimeScale'),
                    ('lookupTableStartTime', 'LookupTableStartTime'), ('lookupTable', 'LookupTable')]:
                distribution[target] = copy.deepcopy(values[source]['value'])
            evidence.append(dict(module=module['objectPath'], property=pair[0], basis='SOURCE_CDO_NESTED_DEFAULT'))


def effect_track(name, asset, start, duration, offset, scale=1, follow=False, fit=False):
    return dict(effectTrackId=name, slotId='object', resourceKind='V1_EFFECT', resourceId=asset,
        followObject=follow, fitEffectToDuration=fit, bone='', timing='TIME', startMs=start,
        durationMs=duration, positionOffset=offset, rotationDegrees=[0, 0, 0], scale=[scale] * 3)


def curve_from_source(system):
    document = read(AUTHORED / ('effect.kouku.source.' + system + '.effect.json'))
    module = next(m for m in document['elements'][0]['sourceRecipe']['modules']
        if m['className'] == 'particlemodulelocationdirect' and circus.module_enabled(m))
    distribution = next(d for d in module['distributions'] if d['propertyPath'] == 'location')
    assert distribution['operation'] == 1 and distribution['lookupTableChunkSize'] == 3
    table = distribution['lookupTable'][2:]
    return [table[i:i + 3] for i in range(0, len(table), 3)]


def motion_pair(name, object_id, rows, keys, duration, effects):
    template = dict(sequenceId=SQ_PREFIX + name, displayName=name, category='WorldObject',
        durationMs=duration, interpolation='LINEAR',
        objectMotion=dict(velocity=[0, 0, 0], acceleration=[0, 0, 0], angularVelocityDegrees=[0, 0, 0],
            revolutionDegreesPerSecond=[0, 0, 0], revolutionOffset=[0, 0, 0], spawnHalfExtents=[0, 0, 0],
            count=len(rows), intervalMs=0, spreadDegrees=0, seed=4219806, emissions=rows),
        tracks=[dict(slotId='object', keys=keys)], animationTracks=[], effectTracks=effects)
    instance = dict(instanceId=MI_PREFIX + name, templateId=template['sequenceId'], enabled=True,
        startDelayMs=0, playbackSpeed=1, anchorKind='WORLD', position=[0, 0, 0],
        motionEnd='STOP', nextMotionId='',
        bindings=[dict(slotId='object', targetKind='OBJECT_RESOURCE', targetId=object_id)])
    return template, instance


def build(split_count=5):
    assert 1 <= split_count <= 5
    before = WORLD.read_bytes()
    baseline = json.loads(before.decode('utf-8-sig'))
    contract = circus.read_contract()
    contract.pop('previewPolicy', None)  # The older 63-element preview is not this World motion policy.
    assert len(contract['projectiles']) == 6
    source_ball = next(o for o in baseline['objectResources'] if o['objectId'] == 'world.object.mario.striped_ball')
    measured = geometry(source_ball['modelAssetId'])
    native_measured = geometry('Effect/KoukuSaydon/FullRestore/Meshes/fm_k_ppct_ball_01.wmodel')
    assert measured['vertexCount'] == native_measured['vertexCount'] == 290
    assert max(abs(a - b) for a, b in zip(measured['halfExtentsM'], native_measured['halfExtentsM'])) < 1e-6
    center, half = measured['centerM'], measured['halfExtentsM']
    defaults = []
    ball = leaf('ball_04')
    inherit_missing_defaults(ball, defaults)
    explosions = [leaf('exp_02'), leaf('exp_03'), leaf('exp_04')]
    impact = group(PREFIX + 'rainbow.impact', '세이튼 / 쓰리투원투하 | 무지개 도넛 폭발', [(d, 0) for d in explosions])
    drop = group(PREFIX + 'rainbow.drop', '세이튼 / 쓰리투원투하 | 공 낙하·상단 무지개·도넛 폭발',
        [(ball, 0)] + [(d, 1) for d in explosions])
    muzzle_leaf = read(AUTHORED / 'effect.kouku.gate3.showtime.gun.muzzle.effect.json')
    muzzle = group(PREFIX + 'gun.muzzle', '세이튼 / 쓰리투원투하 | 공 발사·쇼타임 총구', [(muzzle_leaf, 0)])
    # These are the two independent sprites around the original ball mesh.
    # Subtract that mesh's DirectLocation curve so the World object owns motion
    # exactly once. Emitter-location children remain only in the full V1 group.
    upper_source = copy.deepcopy(ball)
    upper_source['elements'] = [e for e in upper_source['elements']
        if e['sourcePresentation']['sourceObjectPath'].rsplit('_', 1)[-1] in ('30', '35')]
    for element in upper_source['elements']:
        module = next(m for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemodulelocationdirect')
        location = next(d for d in module['distributions'] if d['propertyPath'] == 'location')
        assert location['lookupTableChunkSize'] == 3 and len(location['lookupTable']) == 8
        values = location['lookupTable']
        start = [values[i + 2] - [0, 0, 1000][i] for i in range(3)]
        finish = [values[i + 5] - [0, 0, -50][i] for i in range(3)]
        location['lookupTable'] = [min(start + finish), max(start + finish)] + start + finish
    upper = group(PREFIX + 'ball.upper', '세이튼 / 쓰리투원투하 | 공 상단 무지개', [(upper_source, 0)])
    documents = [drop, impact, upper, muzzle]
    contract['sourceInputs'].extend(circus.digest(AUTHORED / (d['effectAssetId'] + '.effect.json'))
        for d in [leaf('ball_04'), *explosions, muzzle_leaf])
    candidate = dict(schema=baseline['schema'], formatVersion=3, areaId=baseline['areaId'], revision=1,
        objectResources=[], templates=[], instances=[])
    expected = []
    first_curve = curve_from_source(circus.SYSTEMS[0])[:9]
    child_curve = curve_from_source(circus.SYSTEMS[1])[:7]
    assert first_curve[3][2] == 20 and abs(first_curve[-1][2] - 20) < 1e-3
    assert child_curve[0][2] == child_curve[-1][2] == 20
    for mode, label in [('split', '세이튼 / 쓰리투원투하 | 공 1회 튕김·5회 분열'),
                        ('shot', '세이튼 / 쓰리투원투하 | 공 발사·5회 분열')]:
        object_id = WORLD_PREFIX + mode + '.model'
        obj = copy.deepcopy(source_ball)
        obj.update(objectId=object_id, displayName=label, scale=[1, 1, 1], anchorKind='WORLD',
            sequenceInstanceId='', defaultMotionInstanceId=MI_PREFIX + mode + '.g0')
        candidate['objectResources'].append(obj)
        candidate['objectResources'].append(dict(objectId=WORLD_PREFIX + mode, displayName=label,
            modelAssetId='', diffuseTextureAssetId='', modelPreScale=.01, animated=False,
            scale=[1, 1, 1], anchorKind='WORLD', sequenceInstanceId='', defaultMotionInstanceId='',
            motionInstanceIds=[MI_PREFIX + mode + '.g' + str(g) for g in range(split_count + 1)]))
        origins = [dict(positionOffset=[0, 0, 0], yawDegrees=0, startDelayMs=0)]
        for generation, projectile in enumerate(contract['projectiles'][:split_count + 1]):
            duration = round(projectile['maxLifeSeconds'] * 1000)
            scale = projectile['scale']
            curve = first_curve if generation == 0 and mode == 'split' else child_curve
            distance = projectile['maxDistanceCm'] * .01
            keys = []
            for index, sample in enumerate(curve):
                fraction = index / (len(curve) - 1)
                height = (sample[2] - 20) * .01
                desired = [0, half[1] * scale + height, distance * fraction]
                position = [desired[i] - center[i] * scale for i in range(3)]
                keys.append(dict(timeMs=round(fraction * duration), positionOffset=position,
                    rotationQuaternion=[0, 0, 0, 1], scaleMultiplier=[scale] * 3,
                    visible=index != len(curve) - 1))
            effects = [effect_track('upper', upper['effectAssetId'], 0, duration, center, follow=True, fit=True),
                effect_track('impact', impact['effectAssetId'], duration, 2500,
                    [center[0], center[1] - half[1], center[2]], scale=1 / scale)]
            if generation == 0 and mode == 'split':
                effects.append(effect_track('first.bounce', impact['effectAssetId'], keys[3]['timeMs'], 2500,
                    [center[0], center[1] - half[1], center[2]], scale=1 / scale))
            if generation == 0 and mode == 'shot':
                effects.append(effect_track('muzzle', muzzle['effectAssetId'], 0, 11000, center, scale=1 / scale))
            name = mode + '.g' + str(generation)
            rows = [dict(row, startDelayMs=generation * duration) for row in origins]
            template, instance = motion_pair(name, object_id, rows, keys, duration, effects)
            candidate['templates'].append(template)
            candidate['instances'].append(instance)
            following = []
            for emitter, row in enumerate(origins):
                radians = math.radians(row['yawDegrees'])
                end = [row['positionOffset'][0] + distance * math.sin(radians), 0,
                    row['positionOffset'][2] + distance * math.cos(radians)]
                expected.append(dict(mode=mode, generation=generation, emitter=emitter,
                    origin=row['positionOffset'], finish=end, yawDegrees=row['yawDegrees'], scale=scale,
                    startMs=generation * duration, endMs=(generation + 1) * duration))
                for delta in (45, 135):
                    following.append(dict(positionOffset=end, yawDegrees=row['yawDegrees'] + delta, startDelayMs=0))
            origins = following
    # Registration entries are semantic append inputs, never replacements for Live documents.
    for document in documents:
        write(OUT / 'candidates' / (document['effectAssetId'] + '.effect.json'), document)
    write(OUT / 'candidates/WorldSequence.entries.json', candidate)
    write(OUT / 'candidates/EffectCatalog.entries.json', dict(effects=[dict(effectAssetId=d['effectAssetId'],
        payloadKind='DIRECT_AUTHORED_DOCUMENT', authoringPath='Effects/Authored/' + d['effectAssetId'] + '.effect.json') for d in documents]))
    write(OUT / 'candidates/EffectResourceTree.entries.json', dict(nodes=[],
        references=[dict(kind='V1', assetId=d['effectAssetId'], displayName=d['displayName'],
            parentId='kouku.category.867ede1a948dc387a93d') for d in documents]))
    write(OUT / 'candidates/Composition.worlds.proposal.json', dict(registerOnly=True, patternMutation=None,
        definitions=[dict(stableProposalId='saydon.circus.' + mode, displayName=obj['displayName'],
            objectResourceId=obj['objectId'], sequenceInstanceId=obj['objectId'],
            positionOffset=[0, 0, 0], anchorKind='NONE', anchorPosition=[0, 0, 0], companionEffectResourceId='')
            for mode, obj in zip(('split', 'shot'), candidate['objectResources'][1::2])],
        existingGunResources=['world.object.kouku.saydon_showtime_gun_left', 'world.object.kouku.saydon_showtime_gun_right'],
        timelinePlacement='World group owns six STOP motions with emission delays; one Composition occurrence targets the group ID. P83 remains untouched.',
        presentationDurationMs=(split_count + 1) * 1500 + 2500))
    contract.update(requestedSplitCount=split_count, generatedBallCountPerWorld=2 ** (split_count + 1) - 1,
        nativeSplitCount=5, directions=dict(basis='USER_AUTHORED', parentAxisTurn=90, childOffsets=[-45, 45]),
        motionProjection=dict(basis='PROJECT_AUTHORED', policy='FIRST_BOUNCE_CURVE_RETIMED_TO_SOURCE_MISSILE_LIFETIME',
            firstSourceSamples=first_curve, childSourceSamples=child_curve, geometry=measured,
            material='Existing Mario StripedBall CMaterial; intentionally reused per user request.',
            centerPolicy='Cancel measured CModel pivot; rest bottom touches ground.',
            horizontalPolicy='SOURCE_MAX_DISTANCE_SPREAD_OVER_SOURCE_MAX_LIFETIME; source speed is retained as evidence, not claimed as reproduced.',
            scheduling='Six STOP motions in one model-less World group; per-emission birth delays scale with playback speed.',
            groupOrigin='First successful cue-birth provider matrix shared by every generation; pending origin retries.',
            damageAuthority=False, gameplayCollisionAdded=False), sourceDefaultRepairs=defaults,
        rainbowSource=dict(projectileId=421980613, systems=['VividFracture_Ball_04', 'VividFracture_Exp_02',
            'VividFracture_Exp_03', 'VividFracture_Exp_04'],
            upperPolicy='Original sprites 30/35 position minus original mesh5 position; World supplies motion once.',
            dropImpactDelaySeconds=1, delayBasis='PROJECT_AUTHORED matching source Ball_04 particle lifetime'),
        deployment=dict(installed=False, worldBaselineSha256=hashlib.sha256(before).hexdigest(),
            untouchedP83=True, clientRun=False, visualApproval=False), expectedObjects=expected)
    write(OUT / 'source-contract.json', contract)
    write(OUT / 'pending-registration.json', dict(installed=False,
        worldEntries='candidates/WorldSequence.entries.json', effects='candidates/EffectCatalog.entries.json',
        effectTree='candidates/EffectResourceTree.entries.json', composition='candidates/Composition.worlds.proposal.json',
        worldGroups=[WORLD_PREFIX + 'split', WORLD_PREFIX + 'shot'],
        rules=['Semantic append by stable ID after preserving the user draft; never replace the whole Live document.',
            'World group objectId is both Composition objectResourceId and sequenceInstanceId.',
            'Do not add or alter P83 animation stages or occurrences.',
            'Keep existing Showtime BOSS hand-bound WORLD guns as separately placeable resources.'],
        durations=dict(rainbowDropMs=3500, rainbowImpactMs=2500, upperMs=1800, muzzleMs=11000,
            worldSplitMs=(split_count + 1) * 1500 + 2500, worldShotMs=max(11000, (split_count + 1) * 1500 + 2500))))
    assert WORLD.read_bytes() == before, 'Authoring document changed while reading; rebuild candidates.'
    print(json.dumps(dict(output=str(OUT), installed=False, effects=len(documents),
        worldResources=len(candidate['objectResources']), splits=split_count,
        ballsPerWorld=2 ** (split_count + 1) - 1), ensure_ascii=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--split-count', type=int, default=5, choices=range(1, 6))
    build(parser.parse_args().split_count)
