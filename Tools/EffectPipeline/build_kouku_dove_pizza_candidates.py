"""Stage the requested dove group and source-CDO pizza repair without live writes."""
from __future__ import annotations

import copy
import argparse
import hashlib
import math
import struct

from build_saydon_card_pattern_groups import AUTHORED, ROOT, leaf, read, renamed, write
from build_saydon_spinning_emitter_preview import constant, emission, literal

OUTPUT = ROOT / 'out/EffectV1DovePizza20260917/candidate'
SOURCE = ROOT / 'out/KoukuAllEffects20260912'
DOVE = 'effect.kouku.magic.paper.dove.group'
PIZZA = 'effect.kouku.pizza.explosion.group'
PIZZA_SOURCE_RESTORED = PIZZA + '.source-restored'


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


def dove_single_file_track(lane, straight_scale=.5):
    """Shorten the lead-in, keeping tangent speed continuous through the turn."""
    assert 0 <= lane < 4
    speed, spacing, straight, turn = 8., .8, .7, 2.
    assert straight_scale in (1., 2 / 3, .5)
    # Retain the two historical tracks only to recognize safe migrations.
    if straight_scale == .5:
        straight *= straight_scale
    straight_speed = speed if straight_scale == .5 else speed * straight_scale
    radius = speed * turn / math.pi
    positions, angles = [], []
    for frame in range(181):
        time = frame / 60
        travel_time = time - lane * spacing / speed
        if travel_time <= straight:
            position, yaw = [straight_speed * travel_time, 0., 0.], 0.
        elif travel_time <= straight + turn:
            angle = math.pi * (travel_time - straight) / turn
            position = [straight_speed * straight + radius * math.sin(angle), radius * (1-math.cos(angle)), 0.]
            yaw = math.degrees(angle)
        else:
            position, yaw = [straight_speed * straight - straight_speed * (travel_time - straight - turn), 2*radius, 0.], 180.
        def key(value):
            return dict(timeSeconds=time, value=value, arriveTangent=[0, 0, 0],
                leaveTangent=[0, 0, 0], interpolation='linear')
        positions.append(key([v * 100 for v in position]))
        angles.append(key([0, 0, yaw]))
    return dict(sourceOccurrenceId=DOVE + f'.authored.single-file.bird{lane}', sourceTimeOriginSeconds=0,
        previewOriginUE3Cm=[0, 0, 0], nodes=[dict(sourceObjectPath=DOVE + f'/project-authored-half-turn/bird{lane}',
        frame='WORLD', initialPositionUE3Cm=positions[0]['value'], initialEulerDegrees=[0, 0, 0],
        scaleUE3=[1, 1, 1], positionKeys=positions, eulerKeys=angles)])


def apply_dove_single_file_path(document):
    """Stage motion only; preserve current authoring and reject unknown tracks."""
    staged = copy.deepcopy(document)
    birds = [e for e in staged['elements'] if e['groupId'] == DOVE + '.flight']
    assert len(birds) == 4, 'Expected the existing four-bird flight group'
    changes, lanes = [], set()
    for element in birds:
        modules = element['sourceRecipe']['modules']
        matching = [lane for lane in range(4) if any(m['stableId'] == DOVE + f'.death{lane}' for m in modules)]
        assert len(matching) == 1, 'Keep a unique original death-event owner for each bird'
        lane = matching[0]
        assert lane not in lanes
        lanes.add(lane)
        track = dove_single_file_track(lane)
        legacy_track = dove_single_file_track(lane, 1.)
        prior_candidate_track = dove_single_file_track(lane, 2 / 3)
        previous_track = element.get('sourceTransformTrack')
        assert element['detail']['particle']['localSpace']
        assert element['detail']['timing']['startDelaySeconds'] == 0
        assert element['detail']['timing']['lifeTimeSeconds'] == 3
        assert abs(element['detail']['particle']['sourceScale']['lifeTime'] - .3) < 1.e-6
        if 'sourceTransformTrack' in element:
            assert previous_track in (legacy_track, prior_candidate_track, track), 'Preserve an independently edited bird motion track'
        remove = {DOVE + f'.velocity{lane}', DOVE + f'.orbit{lane}'}
        removed = [m for m in modules if m['stableId'] in remove]
        assert len(removed) == 2 or (not removed and previous_track in (legacy_track, prior_candidate_track, track))
        if removed:
            assert {m['className'] for m in removed} == {'particlemodulevelocity', 'particlemoduleorbit'}
        if removed or previous_track != track:
            changes.append(dict(elementId=element['id'], removedModuleIds=sorted(remove),
                changedField='sourceTransformTrack', straightDistanceMultiplier=.5,
                basis='USER_REQUESTED_PROJECT_AUTHORED'))
        element['sourceRecipe']['modules'] = [m for m in modules if m['stableId'] not in remove]
        assert not any(m['className'] in ('particlemodulevelocity', 'particlemoduleorbit', 'particlemodulelocationdirect')
                       for m in element['sourceRecipe']['modules']), 'Do not compose another movement contribution'
        element['sourceTransformTrack'] = track
        assert element['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-2893-native.v1'
    return staged, changes


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
    saved_path = AUTHORED / (DOVE + '.effect.json')
    if saved_path.is_file():
        document = read(saved_path)
    document, motion_changes = apply_dove_single_file_path(document)
    projectile = SOURCE / 'source/Projectile/421980201.loa'
    motion = struct.unpack_from('<fff i i f i', projectile.read_bytes(), 1197)
    assert motion == (1, 50, 30, 800, 1500, 5, 1500), motion
    proof = dict(assetId=DOVE, installed=False, sourceFlightAsset=flight['effectAssetId'],
        sourceImpactAsset=impact['effectAssetId'], sourceProjectileSha256=sha(projectile),
        sourceInputs=input_receipts(flight, impact),
        sourceMotion=dict(scale=motion[0], radiusCm=motion[1], heightCm=motion[2],
            speedCmPerSecond=motion[3], maxSpeedCmPerSecond=motion[4], lifetimeSeconds=motion[5], maxDistanceCm=motion[6]),
        projectRequest=dict(birdCount=4, lifetimeSeconds=3, straightSpacingM=.8,
            curveArcSpacingM=.8, straightSpeedMPerSecond=8, curveSpeedMPerSecond=8,
            straightDistanceMultiplier=.5, leaderForwardDistanceM=2.8,
            leaderReturnDistanceM=5.2, whiteBodyBasis='PRESERVE_SAVED_EXPOSURE',
            perBirdLagSeconds=.1, leaderStraightSeconds=.35, halfTurnSeconds=2,
            turnRadiusM=16 / math.pi, turnAngleDegrees=180,
            trajectoryBasis='USER_REQUESTED_PROJECT_AUTHORED',
            direction='Original +X source travel, rotated by each authored occurrence root.',
            motionPolicy='Saved 3s lifetime and 0.1s follower delay preserved; 0.35s lead-in, 2s half-turn and 0.65s return all use 8m/s. This is not the original 15m projectile cap.'),
        savedInputSha256=sha(saved_path) if saved_path.is_file() else None,
        motionChanges=motion_changes,
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
    path = AUTHORED / (PIZZA + '.effect.json')
    # Exp02 is the preceding full-circle pulse, whereas Exp03 is the original
    # subsequent pizza hit. Preserve the saved Exp03 edits/IDs and remove only
    # the independently added full-circle stream requested by the user.
    if path.is_file():
        document = read(path)
    else:
        document = renamed(docs[1], PIZZA + '.boundary', '원본 피자 경계')
        document['effectAssetId'] = PIZZA
    prefix = 'fx_mn_rpcz_00_u.par_u_rpcz_bigarea_exp_02_loc_int.'
    removed = [e for e in document['elements'] if e['sourcePresentation']['sourceObjectPath'].startswith(prefix)]
    kept = [e for e in document['elements'] if not e['sourcePresentation']['sourceObjectPath'].startswith(prefix)]
    expected = {e['sourcePresentation']['sourceObjectPath'] for e in docs[1]['elements']}
    assert len(kept) == len(expected) == 16
    assert {e['sourcePresentation']['sourceObjectPath'] for e in kept} == expected
    document['elements'] = kept
    document['displayName'] = '피자 부채꼴 · 원본 검정 무지개 경계 폭발'
    document.pop('sourceModelPreview', None)
    repairs = repair_geometry_defaults(document)
    mesh_elements = [e for e in kept if any(r['slotId'] == 'meshModel' for r in e['resources'])]
    assert len(mesh_elements) == 3
    mesh_evidence = []
    for e in mesh_elements:
        rotation = next(m for m in e['sourceRecipe']['modules'] if m['className'] == 'particlemodulemeshrotation_seeded')
        mesh_evidence.append(dict(sourceEmitter=e['sourcePresentation']['sourceObjectPath'],
            resources=e['resources'], bursts=e['sourceRecipe']['bursts'],
            startRotation=rotation['distributions'], material=e['material']['sourceMaterialPath']))
    proof = dict(assetId=PIZZA, installed=False, sourceAssets=[d['effectAssetId'] for d in docs],
        sourceInputs=input_receipts(*docs),
        sourceRepairs=repairs, nativeAssets=native_references(document),
        savedInputSha256=sha(path) if path.is_file() else None,
        removedSourceElementIds=[e['id'] for e in removed], preservedSourceElementIds=[e['id'] for e in kept],
        sourceMeshEmitters=mesh_evidence,
        preserved='Every material, TypeData mesh pre-rotation, source mesh StartRotation, user transform and nonempty distribution.',
        safeWedge='Original two 120-degree meshes plus one 90-degree mesh and .45/.88/.65-turn source rotations have one geometric empty interval of about 85.2 degrees. Material opacity is time/UV dependent; this geometric fact is not visual approval.',
        issue='The independent group had mixed the full-circle Exp02 pulse with the subsequent Exp03 pizza hit. This candidate isolates the exact 16-emitter Exp03 stream. The screenshot internal opacity gaps are not proven to be an emission-count failure and are not filled by invented clones.',
        manualVisualValidation='USER_PENDING', **registration(document, docs[0]['effectAssetId']))
    return document, proof


def pizza_source_restored():
    """A separate source-derived document; never reset the edited pizza group."""
    source = read(AUTHORED / 'effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_bigarea_exp_03_loc_int.effect.json')
    document = renamed(source, PIZZA_SOURCE_RESTORED,
                       '쿠크_피자 | 피자 부채꼴 - 원본 검정 무지개 경계 폭발 2')
    document.pop('sourceModelPreview', None)
    assert len(document['elements']) == len(source['elements']) == 16
    repairs = repair_geometry_defaults(document)
    by_emitter = {e['sourcePresentation']['sourceObjectPath']: e for e in source['elements']}
    for element in document['elements']:
        original = by_emitter[element['sourcePresentation']['sourceObjectPath']]
        assert element['material'] == original['material']
        assert element['resources'] == original['resources']
        assert element['detail'] == original['detail']
    # The recovered shader has no world circle. This requested coverage is an
    # explicit project-authored layer, applied only to the two source dark sprites.
    masked = []
    for element in document['elements']:
        if element['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-3171-native.v1':
            element['detail']['sprite']['ownerRadialMask'] = dict(
                enabled=True, centerXZ=[0.0, 0.0], radius=6.6, feather=0.05)
            masked.append(element['id'])
    assert len(masked) == 2
    live = AUTHORED / (PIZZA + '.effect.json')
    proof = dict(assetId=PIZZA_SOURCE_RESTORED, installed=False,
        authoredCoverage=dict(classification='PROJECT_AUTHORED', elementIds=masked,
            coordinateSpace='Effect-origin XZ before ParticleSystem uniformScale/yaw and Frame.RootWorld',
            radius=6.6, feather=0.05, centerXZ=[0, 0],
            radiusEvidence='Installed sphere002/003 WModel preScale .01 and actual .2s Playback transform: 6.599988..6.600002 metres',
            reason='Original native3171 alpha extends beyond the rainbow geometry; no original global radial clip was recovered.'),
        sourceAssets=[source['effectAssetId']], sourceInputs=input_receipts(source),
        sourceRepairs=repairs, nativeAssets=native_references(document),
        preservedUserDocument=dict(path=str(live.relative_to(ROOT)), sha256=sha(live)),
        sourceElementMapping=[dict(sourceElementId=before['id'], candidateElementId=after['id'],
            sourceEmitter=after['sourcePresentation']['sourceObjectPath'])
            for before, after in zip(source['elements'], document['elements'])],
        geometryPolicy='Original sixteen Exp03 emitters, two dark-aura sprites, both distinct flow emitters, and original source transforms. Only two native3171 layers opt into an authored Effect-origin circle; no angular crop.',
        materialPolicy='Original native MIC/PS/texture/alpha inputs retained. PROJECT_AUTHORED radial coverage clips dark-aura after native shading; this additional mask is not claimed as original shader restoration.',
        manualVisualValidation='USER_PENDING', **registration(document, source['effectAssetId']))
    return document, proof


def main():
    parser = argparse.ArgumentParser(__doc__)
    selected = parser.add_mutually_exclusive_group()
    selected.add_argument('--pizza-only', action='store_true')
    selected.add_argument('--dove-only', action='store_true')
    selected.add_argument('--pizza-source-restored', action='store_true')
    parser.add_argument('--output', type=type(OUTPUT), default=OUTPUT)
    options = parser.parse_args()
    builders = ((pizza_source_restored,) if options.pizza_source_restored else
                (pizza,) if options.pizza_only else (dove,) if options.dove_only else (dove, pizza))
    for build in builders:
        document, proof = build()
        filename = document['effectAssetId'] + '.effect.json'
        write(options.output / filename, document)
        proof['candidateSha256'] = sha(options.output / filename)
        write(options.output / (document['effectAssetId'] + '.receipt.json'), proof)
        print(document['effectAssetId'], len(document['elements']))


if __name__ == '__main__':
    main()
