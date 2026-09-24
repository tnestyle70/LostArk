"""Prepare the requested raid fixes without replacing the live authoring document.

Collision dimensions are authored gameplay geometry. Measured mesh bounds are
used only for the Albion floor cross, never for flying debris or an entire FX.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
COMPOSITION = Path('Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json')
WORLDS = Path('Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json')
PREFIX = 'KAKULSAYDON_G1_PATTERN_'
TRIGGER = 'kakulsaydon.g1.logic.508'
RESULT = 'kakulsaydon.g1.logic.98'


def read(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))


def write(path, value):
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    Path(path).write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def rotate_y(v, degrees):
    a = math.radians(degrees)
    return [v[0]*math.cos(a)+v[2]*math.sin(a), v[1], -v[0]*math.sin(a)+v[2]*math.cos(a)]


# PROJECT_AUTHORED gameplay geometry measured from the installed MN_RPCT_05,
# Sample_SourceAnchorWorlds and Make_ParticleSpriteWorld, beam29, 2050..3050ms.
# Separate eye OBBs contain the sampled XZ shaft quads, excluding dark discs and
# debris. P123 includes the user's saved pitch/roll. These are not source damage
# dimensions or a claim about GPU texture alpha coverage.
BEAM_GEOMETRY = {
    62: (([12.228298,0,1.451182],84.076051,[1.743945,.5,12.294109]),
         ([12.143078,0,-1.584339],98.680594,[1.833367,.5,12.270746])),
    123: (([12.210108,0,1.656949],81.392083,[1.943348,.5,12.405150]),
          ([12.379245,0,-1.189785],94.336654,[1.920003,.5,12.334836])),
}


def prepare(document):
    out = copy.deepcopy(document)
    patterns = {p['patternId']: p for p in out['patterns']}
    resources = {r['resourceId']: r for r in out['presentationResources']}
    logics = {l['logicId']: l for l in out['logics']}
    changes = []

    def pat(n): return patterns[PREFIX + str(n)]
    def asset(o): return resources[o['resourceId']].get('assetId', '')
    def effects(p, name): return [o for o in p['presentationOccurrences'] if asset(o) == name]
    def colliders(p): return [o for o in p['presentationOccurrences'] if resources[o['resourceId']]['kind'] == 'COLLIDER']
    def logic(p, o): return next(l for l in p['logicOccurrences'] if l['occurrenceId'] == o['logicOccurrenceId'])
    def window(p, o, start, duration=34):
        o.update(startMs=start, durationMs=duration)
        logic(p, o).update(startMs=start, durationMs=duration, logicId=TRIGGER)
    def resource(name, shape, **geometry):
        ordinal = out['nextPresentationResourceOrdinal']
        out['nextPresentationResourceOrdinal'] += 1
        rid = f'kakulsaydon.g1.presentation.{ordinal}'
        value = dict(resourceId=rid, displayName=name, defaultAnchorKind='BOSS', kind='COLLIDER',
                     assetId='', resourceKind='GROUP', elementId='', durationMs=100,
                     colliderKind='GEOMETRY', shape=shape, halfExtents=[1,.5,1], radiusM=1,
                     halfAngleDegrees=180)
        value.update(geometry)
        out['presentationResources'].append(value)
        resources[rid] = value
        return rid
    def add_hit(p, fx, rid, delay, duration, local_offset, shared_window=None):
        assert fx.get('rotationDegrees', [0,0,0])[::2] == [0,0], 'requires yaw-only effect'
        start = fx['startMs'] + delay
        lid = shared_window
        if lid is None:
            n = p['nextLogicOccurrenceOrdinal']; p['nextLogicOccurrenceOrdinal'] += 1
            lid = f"{p['patternId']}.logic.{n}"
            p['logicOccurrences'].append(dict(occurrenceId=lid, logicId=TRIGGER, startMs=start,
                durationMs=duration, enabled=True, onSuccessLogicIds=[RESULT], onFailLogicIds=[], onTimeoutLogicIds=[]))
        offset = rotate_y([a*b for a,b in zip(local_offset,fx['scale'])], fx['rotationDegrees'][1])
        n = p['nextPresentationOccurrenceOrdinal']; p['nextPresentationOccurrenceOrdinal'] += 1
        hit = copy.deepcopy(fx)
        hit.update(occurrenceId=f"{p['patternId']}.presentation.{n}", resourceId=rid,
            startMs=start, durationMs=duration, logicOccurrenceId=lid, debugRender=True,
            positionOffset=[a+b for a,b in zip(fx['positionOffset'],offset)])
        hit.pop('effectSourceStartMs', None)
        p['presentationOccurrences'].append(hit)
        changes.append(dict(kind='impact', patternId=p['patternId'], source=fx['occurrenceId'], collider=hit['occurrenceId'], startMs=start))
        return hit

    assert logics[TRIGGER]['triggerKind'] == 'ENTER_AREA'
    rain = logics['kakulsaydon.g1.logic.131']
    assert rain['randomScaleMin'] == 1 and rain['randomScaleMax'] == 2
    rain['randomScaleMax'] = 4
    changes.append(dict(kind='card-rain-scale', minimum=1, maximum=4, spawnIntervalMs=rain['randomSpawnIntervalMs']))

    for number in (100, 116, 113):
        hits = colliders(pat(number)); assert len(hits) == 1
        o = hits[0]
        assert o['rotationDegrees'][1] == 90
        o['rotationDegrees'][1] = 270
        o['positionOffset'][0] *= -1
        o['positionOffset'][2] *= -1
        changes.append(dict(kind='wind-reverse', patternId=pat(number)['patternId']))

    # One shared window prevents the crossing rectangles from dealing two hits.
    cross = resource('노란 십자 장판 피해', 'BOX', halfExtents=[1.27536,.5,5.27533])
    for number in (79,119,122):
        p = pat(number)
        fx, = effects(p, 'effect.kouku.albion.cross.electric.impact')
        hits = [o for o in colliders(p) if o['startMs'] == 5834]
        assert len(hits) == 2 and hits[0]['logicOccurrenceId'] == hits[1]['logicOccurrenceId']
        for o, yaw in zip(hits, (0,90)):
            o.update(resourceId=cross, anchorKind=fx['anchorKind'], followBoss=fx['followBoss'],
                positionOffset=[fx['positionOffset'][0],fx['positionOffset'][1],fx['positionOffset'][2]-1],
                rotationDegrees=[0,fx['rotationDegrees'][1]+yaw,0], scale=list(fx['scale']))
            window(p, o, fx['startMs']+34, 100)
        for name, old_start in (('circle',10599),('innerdonut',11399),('outerdonut',11899)):
            fx, = effects(p, f'effect.kouku.gate1.blade-dance.{name}.impact')
            o, = [o for o in colliders(p) if o['startMs']==old_start]
            for field in ('anchorKind','followBoss','positionOffset','rotationDegrees','scale'):
                o[field] = copy.deepcopy(fx[field])
            window(p, o, fx['startMs'], 34)
        changes.append(dict(kind='yellow-cross-and-ring-alignment', patternId=p['patternId']))

    p = pat(83)
    balls = effects(p, 'effect.kouku.gate1.circus.ball.launch')
    assert len(balls) == 6
    last = max(balls, key=lambda o:o['startMs'])
    p['presentationOccurrences'].remove(last)
    # Original source rings preserve the two safe gaps. Only impact timing moves
    # to the first visible native impact tick; airborne balls never own damage.
    for fx in effects(p, 'effect.kouku.gate1.circus.rainbow.impact'):
        hits = [o for o in colliders(p) if o['startMs']==fx['startMs']]
        assert len(hits)==3 and {resources[o['resourceId']]['radiusM'] for o in hits}=={1,4,7}
        for o in hits: window(p,o,fx['startMs']+34)
    changes.append(dict(kind='circus-five-launches-impact-only', removedOccurrenceId=last['occurrenceId']))

    # Native Sk02_3 rings begin at 3041.7 ms, after flight. Four separate ring
    # centers share one damage window. Radius rounds the measured 4.67778 m
    # maximum ring diameter; it is not claimed as exact texture alpha coverage.
    dove = resource('종이비둘기 폭발 피해', 'CIRCLE', radiusM=2.35)
    for fx in effects(pat(82), 'effect.kouku.magic.paper.dove.group'):
        shared = None
        for x in (-2.4,-1.6,-.8,0):
            hit = add_hit(pat(82), fx, dove, 3050, 100, [x,0,-10.1859], shared)
            shared = hit['logicOccurrenceId']

    # Both eye shafts share one damage window. Preserve the authored FX tilt.
    for number in (62,123):
        p = pat(number)
        fx, = effects(p, 'effect.kouku.bingo.encore.blackhole.beam.full.restore')
        expected_rotation = [0,0,0] if number==62 else [-20.700000762939453,-4.050000190734863,20.149999618530273]
        assert fx['rotationDegrees']==expected_rotation and fx['positionOffset']==[0,0,0] and fx['scale']==[1,1,1], 'beam measurement inputs changed'
        planar=copy.deepcopy(fx);planar['rotationDegrees']=[0,0,0];planar['scale']=[1,1,1]
        shared=None
        for eye,(center,yaw,half) in enumerate(BEAM_GEOMETRY[number]):
            beam = resource(f'세이튼 눈 레이저 피해 {number}/{eye+1}', 'BOX', halfExtents=half)
            hit = add_hit(p, planar, beam, 2050, 1000, [0,0,0],shared)
            hit.update(positionOffset=center,rotationDegrees=[0,yaw,0])
            shared=hit['logicOccurrenceId']
    pat(62)['displayName'] = '세이튼_눈 레이저'
    flow = next(f for f in out['patternFlows'] if f['gateId']=='GATE3')
    flow['entries'].insert(-1, dict(entryId='kakulsaydon.flow.gate3.0924.laser',kind='PATTERN',targetId=PREFIX+'62',waitAfterMs=0))
    parent = pat(96)
    duration = sum(s['durationMs'] for s in pat(123)['stages'])
    children = []
    shift = 0
    for child in parent['patternOccurrences']:
        child['startMs'] += shift
        children.append(child)
        if child['occurrenceId'] in (PREFIX+'96.pattern.14', PREFIX+'96.pattern.38'):
            n = parent['nextPatternOccurrenceOrdinal']; parent['nextPatternOccurrenceOrdinal'] += 1
            children.append(dict(occurrenceId=f"{parent['patternId']}.pattern.{n}",patternId=PREFIX+'123',
                startMs=child['startMs']+child['durationMs'],durationMs=duration,repeat=False))
            shift += duration
    assert shift == 2*duration
    parent['patternOccurrences'] = children
    previous_duration = parent['durationMs']
    parent['durationMs'] += shift
    for window in parent['logicOccurrences']:
        if window['startMs']==0 and window['durationMs']==previous_duration:
            window['durationMs'] += shift
    changes.append(dict(kind='laser-flow', gate3Pattern=62,bingoPattern=123,bingoAddedDurationMs=shift))

    out['revision'] = document['revision']+1
    return out, changes


def prepare_worlds(document):
    out = copy.deepcopy(document)
    t = next(t for t in out['templates'] if t['sequenceId']=='world.object.kouku.cutting_blade.state.mario_phase2_instant')
    assert t['objectMotion']['count']==8
    t['objectMotion'].update(count=1, emissions=[dict(positionOffset=[0,1,0],yawDegrees=222,startDelayMs=0)])
    delta = rotate_y(t['tracks'][0]['keys'][-1]['positionOffset'],222)
    assert math.hypot(delta[0]+13.03,delta[2]+16.16)<.001, delta
    out['revision'] = document['revision']+1
    return out


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    candidate, changes=prepare(read(ROOT/COMPOSITION))
    worlds=prepare_worlds(read(ROOT/WORLDS))
    write(args.output/'Composition.candidate.json',candidate)
    write(args.output/'WorldSequences.candidate.json',worlds)
    write(args.output/'raid-polish.receipt.json',dict(changes=changes,inputs=[dict(path=str(p),sha256=hashlib.sha256((ROOT/p).read_bytes()).hexdigest()) for p in (COMPOSITION,WORLDS)]))
    print(json.dumps(dict(revision=candidate['revision'],changes=len(changes))))


if __name__=='__main__': main()
