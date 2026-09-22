"""Stage Gate 2 juggling and entrance effects; never modify live authoring.

Original particles, materials, grenade speed/height and notification times are
retained. The three editable grenade destinations are presentation authoring:
the original server-selected target is not available to a library preview.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import struct
import sys

from build_saydon_card_pattern_groups import renamed
from build_saydon_spinning_emitter_preview import constant, emission, literal
from build_kouku_dove_pizza_candidates import module, vector, native_references
from build_kouku_backstep_flame_groups import project_mesh_rotation
from build_saydon_circus_world import inherit_missing_defaults
from build_kouku_backstep_electric_group import particle_parameters

ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / 'Data/Effects/Authored'
CACHE = ROOT / 'out/KoukuAllEffects20260912'
OUT = ROOT / 'out/Gate2Restoration20260922/source-effects'
SYSTEM = 'fx_mn_rpcz_00_u.par_u_rpcz_'
JUGGLE = 'effect.kouku.gate2.juggling.three.balls'
TRAIL = 'effect.kouku.gate2.entrance.rope.trail'
DROP = 'effect.kouku.gate2.entrance.drop.ball'


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')


def leaf(suffix):
    return read(AUTHORED / ('effect.kouku.source.' + SYSTEM + suffix + '.effect.json'))


def enabled(m):
    return next((p['value'] for p in m['literals'] if p['propertyPath'] == 'benabled'), True)


def set_literal(m, name, value):
    m['literals'] = [p for p in m['literals'] if p['propertyPath'] != name] + [literal(name, value)]


def lifetime(element, value):
    element['detail']['particle']['lifeTimeSeconds'] = [value, value]
    element['detail']['particle'].setdefault('sourceScale', {})['lifeTime'] = 1
    for m in element['sourceRecipe']['modules']:
        if m['className'] in ('particlemodulelifetime', 'particlemodulelifetime_seeded'):
            m['distributions'] = [constant('lifetime', value)]


def death(element, event):
    element['sourceRecipe']['modules'].append(module(event, 'generator', 'particlemoduleeventgenerator',
        {'bspawnmodule': True, 'bupdatemodule': True, 'events[0].customname': event,
         'events[0].type': 'epet_death', 'events[0].frequency': 1, 'events[0].buseorbitoffset': True}))


def receive(document, impact, event, duration, scale, position):
    """Use the production death-event position, including the terminal curve."""
    child = renamed(impact, event + '.impact', '착탄 폭발')
    for i, e in enumerate(child['elements']):
        count = sum(b['countMaximum'] for b in e['sourceRecipe']['bursts'])
        if count == 0:
            continue
        e['groupId'] = document['effectAssetId'] + '.impact'
        emission(e, 0, duration + .1, [])
        e['detail']['transform']['position'] = position
        e['detail']['transform']['scale'] = [scale] * 3
        e['detail']['particle'].update(burstCount=0, spawnRatePerSecond=0)
        for m in e['sourceRecipe']['modules']:
            if m['className'] == 'particlemodulespawn':
                m['distributions'] = [constant('rate', 0) if x['propertyPath'] == 'rate' else x
                                      for x in m['distributions']]
        e['sourceRecipe']['modules'].append(module(event, f'receiver{i}', 'particlemoduleeventreceiverspawn',
            {'bspawnmodule': True, 'bupdatemodule': True, 'eventname': event,
             'eventgeneratortype': 'epet_death', 'busepsyslocation': False, 'binheritvelocity': False},
            [vector('inheritvelocityscale', [0, 0, 0]), constant('spawncount', count)]))
        document['elements'].append(e)


def hand_sequence():
    """Reuse the decoded native notify anchors, replacing only material gaps."""
    result = read(ROOT / 'out/KoukuActionEffects20260912/candidate/'
                  'effect.kouku.action.mn_rpcz_00.4219716.stage000.effect.json')
    libraries = {}
    for e in result['elements']:
        path = e['sourcePresentation']['sourceObjectPath']
        system = path.rsplit('.', 1)[0]
        if system not in libraries:
            libraries[system] = {a['sourcePresentation']['sourceObjectPath']: a
                for a in read(AUTHORED / ('effect.kouku.source.' + system + '.effect.json'))['elements']}
        donor = libraries[system][path]
        e['material'] = copy.deepcopy(donor['material'])
        e['resources'] = copy.deepcopy(donor['resources'])
        if 'par_u_rpcz_jugle_01_1_loc_int.' in path:
            timing = e['detail']['timing']
            window = timing['lifeTimeSeconds']
            start = timing['startDelaySeconds']
            emission(e, start, window, copy.deepcopy(e['sourceRecipe']['bursts']))
            if max(e['detail']['particle']['lifeTimeSeconds']) > window:
                lifetime(e, window)
            # Native KillOnDeactivate ends the held ball at its exact notify
            # boundary. Keep source particles, with that finite source window.
            e['sourceTransformTrack'] = dict(sourceOccurrenceId=e['id'], sourceTimeOriginSeconds=0,
                previewOriginUE3Cm=[0, 0, 0], nodes=[dict(sourceObjectPath=e['id'], frame='WORLD',
                    initialPositionUE3Cm=[0, 0, 0], initialEulerDegrees=[0, 0, 0],
                    scaleUE3=[1]*3, positionKeys=[], eulerKeys=[])],
                alphaScaleKeys=[dict(timeSeconds=t, value=[alpha]*3, arriveTangent=[0]*3,
                    leaveTangent=[0]*3, interpolation='constant') for t, alpha in ((0, 1), (start + window, 0))])
        project_mesh_rotation(e)
    return renamed(result, JUGGLE, '저글링 3공·포물선·핑크 착탄')


def trajectory(asset, start, seconds, destination, initial):
    points = []
    count = math.ceil(seconds * 60)
    for frame in range(count + 1):
        t = seconds * frame / count
        u = t / seconds
        # Source max height is 250cm. Actual engine aim/height selection is not
        # serialized in this standalone library: this is an editable parabola.
        position = [initial[i] + (destination[i] - initial[i]) * u for i in range(3)]
        position[1] += 2.5 * 4 * u * (1 - u)
        ue = [position[0] * 100, -position[2] * 100, position[1] * 100]
        points.append(dict(timeSeconds=start + t, value=ue, arriveTangent=[0]*3,
                           leaveTangent=[0]*3, interpolation='linear'))
    return dict(sourceOccurrenceId=asset, sourceTimeOriginSeconds=0, previewOriginUE3Cm=[0, 0, 0],
        nodes=[dict(sourceObjectPath=asset + '/project-authored-target-parabola', frame='WORLD',
            initialPositionUE3Cm=points[0]['value'], initialEulerDegrees=[0, 0, 0],
            scaleUE3=[1, 1, 1], positionKeys=points, eulerKeys=[])])


def juggling(destinations):
    document = hand_sequence()
    projectile = (CACHE / 'source/Projectile/421971601.loa').read_bytes()
    motion = struct.unpack_from('<fff i i f i', projectile, 1848)
    assert motion == (1, 25, 25, 650, 700, 5, 1500)
    impact_parameters = particle_parameters(projectile, 592)
    assert impact_parameters['sourceScale'] == [.5] * 3
    mesh_geometry = geometry()
    # Native size is dimensionless 1.2. Keep the mesh origin and scale rather
    # than matching visual size to the 25cm gameplay collision primitive.
    bottom = (mesh_geometry['centerM'][1] - mesh_geometry['halfExtentsM'][1]) * 1.2
    shots = []
    for lane, (start, target) in enumerate(zip((2.2, 3.7, 5.2), destinations)):
        initial = [.2, 1.5, 0]
        final = [target[0], target[1] - bottom, target[2]]
        distance = math.hypot(final[0] - initial[0], final[2] - initial[2])
        assert 0 < distance <= 15, 'Grenade target exceeds original MaxDistance'
        seconds = distance / 6.5
        event = JUGGLE + f'.ball{lane}.death'
        flight = renamed(leaf('jugle_02_loc_int'), JUGGLE + f'.ball{lane}', '저글링 투사체')
        for e in flight['elements']:
            e['groupId'] = JUGGLE + '.flight'
            e['sourceTransformTrack'] = trajectory(e['id'], start, seconds, final, initial)
            emission(e, start, seconds, copy.deepcopy(e['sourceRecipe']['bursts']))
            e['detail']['particle']['localSpace'] = True
            if e['sourceRecipe']['rendererShape'] == 'mesh':
                lifetime(e, seconds)
                emission(e, start, seconds, [dict(timeSeconds=0, countMinimum=1, countMaximum=1)])
                e['detail']['particle']['maxParticles'] = 1
                death(e, event)
            project_mesh_rotation(e)
            document['elements'].append(e)
        receive(document, leaf('jugle_exp_01_loc_int'), event, start + seconds, .5, [0, .1, 0])
        shots.append(dict(launchSeconds=start, flightSeconds=seconds, impactSeconds=start + seconds,
                          initialM=initial, targetGroundM=target, modelEndpointM=final, event=event))
    return document, dict(sourceProjectile=421971601, sourceMotion=list(motion), sourceBallGeometry=mesh_geometry,
        sourceMeshSize=1.2, sourceImpactParticle=impact_parameters,
        motionBasis='PROJECT_AUTHORED_TARGET_PARABOLA_USING_SOURCE_INITIAL_SPEED_AND_MAX_HEIGHT',
        targetSelection='EDITABLE_LIBRARY_DESTINATIONS; not original runtime target selection', shots=shots)


def geometry():
    sys.path.insert(0, str(ROOT / 'Tools/KoukuSaydonPipeline'))
    from apply_saydon_ball_world_motions import geometry as measure
    return measure('Effect/KoukuSaydon/FullRestore/Meshes/fm_d_rhcn_00.wmodel')


def rope_trail():
    doc = renamed(leaf('circusbomb_trail_01_loc_int'), TRAIL, '쿠크줄타기트레일')
    for e in doc['elements']:
        # The native notify is FOLLOW FX_State_01 = spine1 + this socket basis.
        e['actionCueAttachment'].update(enabled=True, follow=True, sourceAnchorSlotId='FX_State_01',
            runtimeAnchorSlotId='FX_State_01', runtimeBoneName='bip001-spine1',
            socketLocalTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 270, 90], scale=[1]*3))
        e['detail']['transform']['position'] = [0, .2, 0]
        emission(e, 0, 3.834584951400757, copy.deepcopy(e['sourceRecipe']['bursts']))
    doc['sourceModelPreview'] = dict(gateId='GATE2', actorProfileId='MN_RPCZ_00',
        targetBossPlacementId='boss.kakulsaydon.g2.kouku', animations=[dict(runtimeClip='rpcz00_att_battle_7_01',
            startOffsetMs=0, sourceStartMs=2379, playMs=3835, playRate=1, endPolicy='HOLD_LAST_POSE')])
    return doc, dict(sourceAction=4219719, sourceNotify='action-4219719/stage-000/notify-015',
        occurrenceStartMs=2379, occurrenceDurationMs=3835, sourceSocket='FX_State_01',
        sourceLocalPositionCm=[0, 0, 20], categoryPath=['KoukuSaydon', '2관문', '패턴', '세이튼등장'])


def drop_ball():
    original = read(ROOT / 'out/KoukuAllEffectsLightClosure20260912/candidate/'
                    'effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_timeprj_01_loc_int.effect.json')
    native = {identity: row['material'] for row in read(CACHE / 'native/native_material_patch.json')['programs']
              for identity in row['occurrences']}
    programs = []
    for e in original['elements']:
        assert e['id'] in native, 'Use exact original occurrence material selection'
        e['material'] = copy.deepcopy(native[e['id']])
        programs.append(e['material']['sourceProfile']['runtimeShaderProfileId'])
        project_mesh_rotation(e)
    fixes = []
    inherit_missing_defaults(original, fixes)
    doc = renamed(original, DROP, '세이튼등장하강폭발공')
    ball = next(e for e in doc['elements'] if e['sourcePresentation']['sourceObjectPath'].endswith('.particlespriteemitter_1'))
    # Provider must precede the two source EmitterLocation children.
    doc['elements'].remove(ball)
    doc['elements'].insert(0, ball)
    event = DROP + '.death'
    death(ball, event)
    receive(doc, leaf('circus_exp_01_loc_int'), event, 3.05, 1, [0, 0, 0])
    raw = (CACHE / 'source/Projectile/421971901.loa').read_bytes()
    return doc, dict(sourceProjectile=421971901,
        sourceDropParticle=particle_parameters(raw, 3015), sourceExplosionParticle=particle_parameters(raw, 1672),
        sourceFall=dict(fromHeightCm=300, toHeightCm=20, particleLifetimeSeconds=3,
            normalizedCurveEnd=.05, fallSeconds=.15), sourceNativePrograms=programs,
        inheritedNestedDefaults=fixes, endpointPolicy='EXISTING_MESH_DEATH_EVENT_TO_ORIGINAL_CIRCUS_EXP_01',
        categoryPath=['KoukuSaydon', '2관문', '패턴', '세이튼등장'])


def build(output, destinations):
    result = []
    for doc, proof in (juggling(destinations), rope_trail(), drop_ball()):
        assert len(doc['displayName'].encode('utf8')) <= 64
        assert len({e['id'] for e in doc['elements']}) == len(doc['elements'])
        proof.update(effectAssetId=doc['effectAssetId'], displayName=doc['displayName'],
            elementCount=len(doc['elements']), nativeAssets=native_references(doc),
            manualVisualValidation='USER_PENDING')
        path = output / 'candidate/Data/Effects/Authored' / (doc['effectAssetId'] + '.effect.json')
        write(path, doc)
        proof['candidatePath'] = str(path.relative_to(ROOT))
        proof['catalogEntry'] = dict(effectAssetId=doc['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT',
            authoringPath='Effects/Authored/' + path.name)
        proof['categoryPath'] = proof.get('categoryPath', ['KoukuSaydon', '2관문', '패턴', '저글링'])
        result.append(proof)
    write(output / 'source-effects-manifest.json', dict(installed=False, effects=result))
    print(json.dumps([dict(assetId=r['effectAssetId'], elements=r['elementCount']) for r in result]))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--output', type=Path, default=OUT)
    parser.add_argument('--destinations', type=Path, help='JSON: exactly three [x,y,z] local ground destinations in metres')
    args = parser.parse_args()
    destinations = read(args.destinations) if args.destinations else [[8, 0, -2], [8, 0, 0], [8, 0, 2]]
    assert len(destinations) == 3 and all(len(p) == 3 and all(math.isfinite(v) for v in p) for p in destinations)
    assert args.output.resolve().is_relative_to((ROOT / 'out').resolve())
    build(args.output, destinations)
