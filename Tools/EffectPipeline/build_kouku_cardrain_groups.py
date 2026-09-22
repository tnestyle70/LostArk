"""Build card-rain and overhead-card candidates without replacing live documents.

Card meshes/materials and Cascade distributions are source-derived. The yellow
one-metre warning and its grouping are explicit project presentation choices.
"""
from pathlib import Path
import copy
import hashlib
import json
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'Tools/LevelPlacementExtractor')]
from extract_action_effect_notifies import scan_length_prefixed_strings
from build_kouku_backstep_flame_groups import project_mesh_rotation, remap

OUT = ROOT / 'out/CardRain20260922'
AUTHORED = ROOT / 'Data/Effects/Authored'


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2,
                               allow_nan=False) + '\n', encoding='utf-8')


def leaf(system):
    return read(AUTHORED / ('effect.kouku.source.' + system + '.effect.json'))


def identity():
    return dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])


def document(asset, name):
    return dict(schema='lostark.effect-authoring', version=13,
                effectAssetId=asset, displayName=name,
                particleSystem=dict(uniformScaleMultiplier=1, yawOffsetDegrees=0,
                                    directionYawDegrees=0, initialSpeedMultiplier=1),
                modelCues=[], elements=[])


def append(doc, original, role, delay, snapshot=True):
    group = doc['effectAssetId']
    mapping = {e['id']: 'kouku.cardrain.' + hashlib.sha256(
        (group + role + e['id']).encode()).hexdigest()[:24] for e in original['elements']}
    added = remap(copy.deepcopy(original['elements']), mapping)
    for element in added:
        element['groupId'] = group
        element['detail']['timing']['startDelaySeconds'] += delay
        if snapshot:
            element['actionCueAttachment'] = dict(enabled=True, follow=False,
                sourceAnchorSlotId='root', runtimeAnchorSlotId='root', runtimeBoneName='',
                snapshotRootSourceBasisYawDegrees=-90, socketLocalTransform=identity())
        project_mesh_rotation(element)
        if element.get('sourcePresentation', {}).get('enabled'):
            element['sourcePresentation']['sourceTimeSeconds'] += delay
        if 'particleSystemOccurrenceId' in element.get('sourceRecipe', {}):
            element['sourceRecipe']['particleSystemOccurrenceId'] = group + '.' + role
        doc['elements'].append(element)
    return added


def source_timing():
    path = ROOT / 'out/KoukuAllEffects20260912/source/Projectile/421980301.loa'
    raw = path.read_bytes()
    strings = scan_length_prefixed_strings(raw, 0, len(raw))
    timers = [t for t in strings if t['value'] == 'CEFSequenceSummonsActionTimer']
    assert len(timers) == 3
    explosion = struct.unpack_from('<f', raw, timers[1]['sourceOffset'] - 16)[0]
    card = struct.unpack_from('<f', raw, timers[2]['sourceOffset'] - 16)[0]
    assert abs(explosion - 1.65) < 1e-6 and card == 1.5
    particles = [t for t in strings if t['value'].startswith("ParticleSystem'")]
    contracts = []
    for token in particles:
        end = token['sourceOffset'] + 5 + len(token['value'])
        trs = dict(positionCm=list(struct.unpack_from('<3f', raw, end + 76)),
                   rotationUnreal=list(struct.unpack_from('<3i', raw, end + 100)),
                   scale=list(struct.unpack_from('<3f', raw, end + 136)))
        assert trs == dict(positionCm=[0, 0, 0], rotationUnreal=[0, 0, 0], scale=[1, 1, 1])
        contracts.append(dict(system=token['value'].split("'")[1], **trs))
    return dict(projectileId=421980301, sourceSha256=hashlib.sha256(raw).hexdigest(),
                cardStartSeconds=card, impactSeconds=round(explosion, 2), particles=contracts)


def restore_nested_defaults(doc):
    defaults = read(ROOT / 'out/KoukuAllEffects20260912/source_class_defaults.json')['records']
    instances = read(ROOT / 'out/KoukuAllEffects20260912/source_module_inputs.json')['records']
    repairs = []
    for e in doc['elements']:
        for module in e['sourceRecipe']['modules']:
            if module['className'] != 'particlemodulelocationdirect':
                continue
            d = next(d for d in module['distributions'] if d['propertyPath'] == 'scalefactor')
            if d['lookupTable'] or d['keys']:
                continue
            source = instances[module['objectPath']]['properties']['scalefactor']['value']['properties']
            assert list(source) == ['distribution'] and source['distribution']['value'] == 0
            cdo = next(r for r in defaults if r['fullPath'] == 'engine.default__particlemodulelocationdirect')
            fields = cdo['properties']['ScaleFactor']['value']['properties']
            assert fields['LookupTable']['value'] == [1.] * 8
            for target, original in [('operation', 'Op'), ('lookupTableNumElements', 'LookupTableNumElements'),
                    ('lookupTableChunkSize', 'LookupTableChunkSize'), ('lookupTableTimeScale', 'LookupTableTimeScale'),
                    ('lookupTableStartTime', 'LookupTableStartTime'), ('lookupTable', 'LookupTable')]:
                d[target] = copy.deepcopy(fields[original]['value'])
            repairs.append(dict(elementId=e['id'], module=module['objectPath'],
                basis='SOURCE_CDO_NESTED_RAW_DISTRIBUTION_INHERITANCE', scale=[1, 1, 1]))
    return repairs


def build():
    timing = source_timing()
    drop = document('effect.kouku.gate1.cardrain.drop', '세이튼_카드비_노란장판과낙하폭발')
    warning = read(AUTHORED / 'effect.kouku.gate3.showtime.circle.warning.effect.json')
    e = warning['elements'][0]
    e['detail']['decal']['size'] = [2, 2]
    e['detail']['particle']['startSize'] = e['detail']['particle']['endSize'] = [2, 2]
    e['detail']['particle']['lifeTimeSeconds'] = [1.65, 1.65]
    e['detail']['timing']['lifeTimeSeconds'] = 1.65
    e['detail']['color']['multiply'] = [1, .8, 0, 2]
    e['sourceRecipe']['emitterDurationSeconds'] = 1.65
    for module in e['sourceRecipe']['modules']:
        for literal in module['literals']:
            if literal['propertyPath'] == 'emitterduration': literal['value'] = 1.65
        for dist in module['distributions']:
            if dist['propertyPath'] == 'lifetime':
                dist['defaultMinimum'][0] = dist['defaultMaximum'][0] = 1.65
            if dist['propertyPath'] == 'startsize':
                dist['defaultMinimum'] = dist['defaultMaximum'] = [200, 200, 200, 0]
    track = e['sourceTransformTrack']
    track['alphaScaleKeys'][-2]['timeSeconds'] = 1.55
    track['alphaScaleKeys'][-1]['timeSeconds'] = 1.65
    track['materialParameterTracks'][0]['keys'][-1]['timeSeconds'] = 1.5
    for scalar in e['material']['sourceProfile']['scalars']:
        if scalar['name'] == 'decal_drawscale': scalar['value'] = 2
    append(drop, warning, 'warning', 0)
    append(drop, leaf('fx_mn_rpct_05_g.par_g_rpct_05_card_prj_01_loc_int'), 'fall', 1.5)
    append(drop, leaf('fx_mn_rpct_05_g.par_g_rpct_05_card_exp_01_loc_int'), 'impact', 1.65)
    repairs = restore_nested_defaults(drop)

    overhead = document('effect.kouku.common.spinning.card.overhead', '세이튼_빙글빙글_머리위카드')
    stage = read(ROOT / 'out/KoukuActionEffects20260912/candidate/effect.kouku.action.mn_rpct_05.4219819.stage000.effect.json')
    stage['elements'] = [e for e in stage['elements'] if '.par_l_rpct_05_sk_13_loc_int.' in e['sourceNode']]
    native = {e['sourceNode']: e for e in leaf('fx_mn_rpct_05_l.par_l_rpct_05_sk_13_loc_int')['elements']}
    for element in stage['elements']:
        key = element['sourcePresentation']['sourceObjectPath']
        original = next(e for k, e in native.items() if k.endswith(key))
        for field in ('material', 'resources'):
            element[field] = copy.deepcopy(original[field])
        if 'runtimeCarrier' in original: element['runtimeCarrier'] = copy.deepcopy(original['runtimeCarrier'])
    append(overhead, stage, 'overhead', 0, snapshot=False)
    repairs += restore_nested_defaults(overhead)
    overhead['sourceModelPreview'] = dict(gateId='GATE1', actorProfileId='MN_RPCT_05',
        targetBossPlacementId='boss.kakulsaydon.g1.saydon', animations=[dict(
            runtimeClip='rpct00_att_battle_4_04', startOffsetMs=0, sourceStartMs=0,
            playMs=2833, playRate=1, endPolicy='HOLD_LAST_POSE')])
    # Preserve the authored throw clock and all ring/dust/projectile occurrences.
    # Only the two preparation stages are split into the single overhead cue.
    flying = read(AUTHORED / 'effect.kouku.common.spinning.card.throw.effect.json')
    flying['effectAssetId'] = 'effect.kouku.common.spinning.card.flying'
    flying['displayName'] = '세이튼_빙글빙글_카드던지기_준비카드분리'
    flying['elements'] = [e for e in flying['elements']
                          if not e['sourceNode'].startswith(('action-4219819/stage-000/',
                                                            'action-4219819/stage-003/'))]
    assert len(flying['elements']) == 208
    mapping = {e['id']: 'kouku.cardflying.' + hashlib.sha256(e['id'].encode()).hexdigest()[:24]
               for e in flying['elements']}
    flying['elements'] = remap(flying['elements'], mapping)
    docs = []
    for doc, duration, anchor in ((drop, 7200, 'WORLD'), (overhead, 3250, 'BOSS'),
                                  (flying, 19990, 'BOSS')):
        path = OUT / 'candidate/Data/Effects/Authored' / (doc['effectAssetId'] + '.effect.json')
        write(path, doc)
        docs.append(dict(effectAssetId=doc['effectAssetId'], displayName=doc['displayName'],
            candidatePath=path.relative_to(ROOT).as_posix(), destination='Data/Effects/Authored/' + path.name,
            durationMs=duration, defaultAnchorKind=anchor, followBoss=anchor == 'BOSS',
            elementCount=len(doc['elements'])))
    write(OUT / 'source/cardrain-runtime-contract.json', dict(**timing,
        effectAssetId=drop['effectAssetId'], groundRadiusMeters=1,
        occurrenceScaleRange=[1, 2], scaleAxes='uniform XYZ', durationMs=7200,
        inheritedDefaultRepairs=repairs,
        projectTuned=['Yellow radius-1m ground warning', 'Warning/card/impact grouping',
                      'Server-authored random nav targets and occurrence scale'],
        sourceRetained=['15 card-prj emitters', '11 impact emitters', 'Mesh shape/material/emissive',
                        'LocationDirect fall curve', 'Seeded mesh rotation', 'Source projectile timer delays']))
    write(OUT / 'candidate-manifest.json', dict(documents=docs, resources=[],
        compositionPatches=[], sourceContract='source/cardrain-runtime-contract.json'))
    print(json.dumps(docs, ensure_ascii=True))


if __name__ == '__main__':
    build()
