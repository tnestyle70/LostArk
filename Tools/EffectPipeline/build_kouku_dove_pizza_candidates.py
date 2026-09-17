"""Stage the requested dove group and source-CDO pizza repair without live writes."""
from __future__ import annotations

import copy
import hashlib
import struct

from build_saydon_card_pattern_groups import AUTHORED, ROOT, leaf, read, renamed, write
from build_saydon_spinning_emitter_preview import constant, emission, literal

OUTPUT = ROOT / 'out/EffectV1DovePizza20260917/candidate'
SOURCE = ROOT / 'out/KoukuAllEffects20260912'
DOVE = 'effect.kouku.magic.paper.dove.group'
PIZZA = 'effect.kouku.pizza.explosion.group'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def vector(path, value):
    result = constant(path, 0)
    result.update(componentCount=3, lookupTableChunkSize=3,
                  lookupTable=[min(value), max(value)] + list(value) * 2)
    return result


def module(asset, name, cls, values, distributions=()):
    identity = asset + '.' + name
    return dict(stableId=identity, className=cls, objectPath=identity,
                literals=[literal('benabled', True)] + [literal(k, v) for k, v in values.items()],
                distributions=list(distributions))


def native_references(document):
    assets = set()
    for element in document['elements']:
        assets.update(item['assetId'] for item in element['resources'])
        assets.update(item['assetId'] for item in element['material']['sourceProfile'].get('textures', []))
    missing = [asset for asset in sorted(assets) if not (ROOT / 'Client/Bin/Resources' / asset).is_file()]
    assert not missing, missing
    return sorted(assets)


def input_receipts(*documents):
    return [dict(path=str((AUTHORED / (doc['effectAssetId'] + '.effect.json')).relative_to(ROOT)),
                 sha256=sha(AUTHORED / (doc['effectAssetId'] + '.effect.json'))) for doc in documents]


def registration(document, source_system):
    tree = read(ROOT / 'Data/Effects/EffectResourceTree.json')
    parent = next(r['parentId'] for r in tree['references'] if r.get('assetId') == source_system)
    asset = document['effectAssetId']
    return dict(catalogEntry=dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT',
        authoringPath=f'Effects/Authored/{asset}.effect.json'),
        treeReference=dict(kind='V1', assetId=asset, displayName=document['displayName'], parentId=parent))


def dove():
    flight = leaf('02_2_loc_int')
    impact = leaf('02_3_loc_int')
    document = renamed(flight, DOVE, '종이비둘기 4마리 원형 비행 · 폭발')
    document['elements'] = []
    document.pop('sourceModelPreview', None)
    event = DOVE + '.death'
    for lane in range(4):
        element = next(e for e in renamed(flight, f'{DOVE}.bird{lane}', '종이비둘기')['elements']
                       if e['sourceNode'].endswith('.particlespriteemitter_2'))
        element['groupId'] = DOVE + '.flight'
        element['displayName'] = f'종이비둘기 {lane + 1} / 원본 native 2893'
        emission(element, 0, 3, [dict(timeSeconds=0, countMinimum=1, countMaximum=1)])
        element['detail']['particle']['maxParticles'] = 1
        element['detail']['particle'].setdefault('sourceScale', {})['lifeTime'] = .3
        element['sourceRecipe']['modules'] += [
            module(DOVE, f'velocity{lane}', 'particlemodulevelocity',
                   {'bspawnmodule': True, 'binworldspace': False},
                   [vector('startvelocity', [800, 0, 0]), constant('startvelocityradial', 0)]),
            module(DOVE, f'orbit{lane}', 'particlemoduleorbit',
                   {'bspawnmodule': True, 'bupdatemodule': True, 'chainmode': 'eochainmode_add',
                    'offsetoptions.bprocessduringspawn': True,
                    'rotationoptions.bprocessduringspawn': True,
                    'rotationrateoptions.bprocessduringspawn': True},
                   [vector('offsetamount', [100, 0, 0]),
                    vector('rotationamount', [0, 0, lane / 4]),
                    vector('rotationrateamount', [0, 0, 1 / 3])]),
            module(DOVE, f'death{lane}', 'particlemoduleeventgenerator',
                   {'bspawnmodule': True, 'bupdatemodule': True,
                    'events[0].customname': event, 'events[0].type': 'epet_death',
                    'events[0].frequency': 1, 'events[0].buseorbitoffset': True})]
        document['elements'].append(element)
    burst_counts = []
    for i, element in enumerate(renamed(impact, DOVE + '.impact', '원본 종이비둘기 폭발')['elements']):
        count = sum(b['countMaximum'] for b in element['sourceRecipe']['bursts'])
        assert count > 0, element['id']
        burst_counts.append(count)
        element['groupId'] = DOVE + '.impact'
        emission(element, 0, 3.1, [])
        element['detail']['particle']['maxParticles'] *= 4
        element['detail']['particle']['burstCount'] = 0
        element['detail']['particle']['spawnRatePerSecond'] = 0
        for source_module in element['sourceRecipe']['modules']:
            if source_module['className'] == 'particlemodulespawn':
                source_module['distributions'] = [constant('rate', 0) if d['propertyPath'] == 'rate' else d
                                                  for d in source_module['distributions']]
        element['sourceRecipe']['modules'].append(module(DOVE, f'impact{i}',
            'particlemoduleeventreceiverspawn', {'bspawnmodule': True, 'bupdatemodule': True,
                'eventname': event, 'eventgeneratortype': 'epet_death',
                'busepsyslocation': False, 'binheritvelocity': False},
            [vector('inheritvelocityscale', [0, 0, 0]), constant('spawncount', count)]))
        document['elements'].append(element)
    projectile = SOURCE / 'source/Projectile/421980201.loa'
    motion = struct.unpack_from('<fff i i f i', projectile.read_bytes(), 1197)
    assert motion == (1, 50, 30, 800, 1500, 5, 1500), motion
    proof = dict(assetId=DOVE, installed=False, sourceFlightAsset=flight['effectAssetId'],
        sourceImpactAsset=impact['effectAssetId'], sourceProjectileSha256=sha(projectile),
        sourceInputs=input_receipts(flight, impact),
        sourceMotion=dict(scale=motion[0], radiusCm=motion[1], heightCm=motion[2],
            speedCmPerSecond=motion[3], maxSpeedCmPerSecond=motion[4], lifetimeSeconds=motion[5], maxDistanceCm=motion[6]),
        projectRequest=dict(birdCount=4, lifetimeSeconds=3, orbitRadiusM=1, orbitPeriodSeconds=3,
            direction='Original +X source travel, rotated by each authored occurrence root.',
            motionPolicy='Constant original initial speed 8m/s; the requested 3-second preview is not the original 15m projectile cap.'),
        impactBirths=sum(burst_counts)*4, impactElementCount=len(burst_counts),
        nativeAssets=native_references(document), manualVisualValidation='USER_PENDING',
        **registration(document, flight['effectAssetId']))
    return document, proof


def repair_geometry_defaults(document):
    instances = read(SOURCE / 'source_module_inputs.json')['records']
    defaults = {r['fullPath']: r for r in read(SOURCE / 'source_class_defaults.json')['records']}
    repairs = []
    for element in document['elements']:
        for mod in element['sourceRecipe']['modules']:
            if mod['className'] not in ('particlemodulelocationprimitivecylinder_seeded',
                                       'efparticlemodulelocationcirclesurface'):
                continue
            for dist in mod['distributions']:
                field = dist['propertyPath']
                if field not in ('startradius', 'velocityscale') or dist['lookupTable'] or dist['keys']:
                    continue
                instance = instances[mod['objectPath']]
                delta = {k.casefold(): v for k, v in instance['properties'][field]['value']['properties'].items()}
                assert set(delta) == {'distribution'} and delta['distribution']['value'] == 0
                package = instance['classPath'].split('.')[0]
                key = instance.get('archetypeFullPath') or package + '.default__' + mod['className']
                chain, inherited = [], {}
                while key:
                    record = defaults[key]
                    chain.append(record)
                    key = record.get('archetypeFullPath')
                for record in reversed(chain):
                    props = {k.casefold(): v for k, v in record['properties'].items()}
                    if field in props:
                        inherited.update({k.casefold(): v['value'] for k, v in props[field]['value']['properties'].items()})
                assert inherited.get('lookuptable') and any(inherited['lookuptable'])
                before = copy.deepcopy(dist)
                for authored, raw in (('operation', 'op'), ('lookupTableNumElements', 'lookuptablenumelements'),
                    ('lookupTableChunkSize', 'lookuptablechunksize'), ('lookupTableTimeScale', 'lookuptabletimescale'),
                    ('lookupTableStartTime', 'lookuptablestarttime'), ('lookupTable', 'lookuptable')):
                    dist[authored] = copy.deepcopy(inherited[raw])
                repairs.append(dict(elementId=element['id'], sourceModule=mod['objectPath'],
                    field=field, sourceExportIndex=instance['exportIndex'], before=before,
                    after=copy.deepcopy(dist), inheritedClassChain=[r['fullPath'] for r in chain]))
    return repairs


def pizza():
    docs = [read(AUTHORED / f'effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigarea_exp_{i:02}_loc_int.effect.json')
            for i in (2, 3)]
    document = renamed(docs[0], PIZZA, '피자 바닥 무지개 · 검정 빨강 경계 폭발')
    document['elements'] += renamed(docs[1], PIZZA + '.boundary', '원본 피자 경계')['elements']
    document.pop('sourceModelPreview', None)
    repairs = repair_geometry_defaults(document)
    assert repairs
    proof = dict(assetId=PIZZA, installed=False, sourceAssets=[d['effectAssetId'] for d in docs],
        sourceInputs=input_receipts(*docs),
        sourceRepairs=repairs, nativeAssets=native_references(document),
        preserved='Every material, TypeData mesh pre-rotation, source mesh StartRotation, user transform and nonempty distribution.',
        safeWedge='Original two 120-degree meshes plus one 90-degree mesh preserve a measured approximately 85.2-degree empty union. No global rotation or inferred exact-90 clipping was applied.',
        issue='Distribution=None instance tags had discarded inherited cooked CDO radial radius/velocity scale. Velocity-facing sprites then received zero velocity.',
        manualVisualValidation='USER_PENDING', **registration(document, docs[0]['effectAssetId']))
    return document, proof


def main():
    for build in (dove, pizza):
        document, proof = build()
        filename = document['effectAssetId'] + '.effect.json'
        write(OUTPUT / filename, document)
        proof['candidateSha256'] = sha(OUTPUT / filename)
        write(OUTPUT / (document['effectAssetId'] + '.receipt.json'), proof)
        print(document['effectAssetId'], len(document['elements']))


if __name__ == '__main__':
    main()
