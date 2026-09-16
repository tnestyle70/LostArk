"""Prepare the Mario circus ball and Odd Doll data without touching open editors."""
import argparse
import copy
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WORLD = Path('Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json')
BALL = 'world.object.kouku.mario_circus_ball'
BALL_MOTION = 'world.object.instance.kouku.mario_circus_ball.aura'
BALL_TEMPLATE = 'sequence.kouku.mario_circus_ball.aura'
BALL_EFFECT = 'effect.kouku.gate3.mario.circus.ball.aura'
DOLL_EFFECT = 'effect.kouku.gate3.doll.flame.shared'


def encode(document):
    return (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf-8')


def encode_world(document, original):
    # Preserve all existing rows and numeric spelling. Only the requested fields
    # and three new stable rows change; expanding animation arrays exceeds 16 MiB.
    text = original.decode('utf-8')
    decoder = json.JSONDecoder()
    def whitespace(position):
        while text[position].isspace(): position += 1
        return position
    def members(start):
        result = {}
        position = whitespace(start + 1)
        while text[position] != '}':
            key, position = decoder.raw_decode(text, position)
            position = whitespace(position)
            assert text[position] == ':'
            first = whitespace(position + 1)
            value, last = decoder.raw_decode(text, first)
            result[key] = (first, last, value)
            position = whitespace(last)
            if text[position] == ',': position = whitespace(position + 1)
        return result
    def rows(start):
        result = []
        position = whitespace(start + 1)
        while text[position] != ']':
            value, last = decoder.raw_decode(text, position)
            result.append((position, last, value))
            position = whitespace(last)
            if text[position] == ',': position = whitespace(position + 1)
        return result
    def compact(value):
        return json.dumps(value, ensure_ascii=False, separators=(',', ':'), allow_nan=False)
    edits = []
    root = members(whitespace(0))
    first, last, _ = root['revision']
    edits.append((first, last, str(document['revision'])))
    for array, identity in [('objectResources', 'objectId'), ('templates', 'sequenceId'), ('instances', 'instanceId')]:
        old_rows = rows(root[array][0])
        targets = {row[identity]: row for row in document[array]}
        existing = {row[2][identity] for row in old_rows}
        for first, last, old in old_rows:
            target = targets[old[identity]]
            if old == target: continue
            fields = members(first)
            assert set(old) <= set(target), 'Existing fields must be preserved'
            for key in old:
                if old[key] != target[key]:
                    start, end, _ = fields[key]
                    edits.append((start, end, compact(target[key])))
            additions = [key for key in target if key not in old]
            if additions:
                end = next(reversed(fields.values()))[1]
                edits.append((end, end, ''.join(',\n      ' + json.dumps(key) + ': ' + compact(target[key]) for key in additions)))
        for index, row in enumerate(document[array]):
            if row[identity] in existing: continue
            predecessor = document[array][index - 1][identity]
            end = next(last for first, last, old in old_rows if old[identity] == predecessor)
            edits.append((end, end, ',\n    ' + compact(row)))
    for first, last, value in sorted(edits, reverse=True):
        text = text[:first] + value + text[last:]
    assert json.loads(text) == document, 'Source-preserving serialization changed semantics'
    result = text.encode('utf-8')
    if len(result) > 16 * 1024 * 1024:
        raise ValueError('World Object candidate exceeds the runtime 16 MiB limit.')
    return result


def body(measurement):
    return dict(maxHp=2000, localCenterM=measurement['localCenterM'],
                halfExtentsM=measurement['halfExtentsM'], lifetimePolicy='UNTIL_DESTROYED')


def build(source, bounds):
    result = copy.deepcopy(source)
    objects = {row['objectId']: row for row in result['objectResources']}
    templates = {row['sequenceId']: row for row in result['templates']}
    instances = {row['instanceId']: row for row in result['instances']}
    if BALL in objects or BALL_TEMPLATE in templates or BALL_MOTION in instances:
        raise ValueError('Circus ball already exists; preserve it and review its current edits.')
    changes = []
    for object_id in ('world.object.kouku.odd_doll', 'world.object.kouku.odd_doll.large'):
        resource = objects[object_id]
        resource['combatBody'] = body(bounds['doll'])
        motion = instances[resource['defaultMotionInstanceId']]
        template = templates[motion['templateId']]
        assert motion['bindings'] == [dict(slotId='object', targetKind='OBJECT_RESOURCE', targetId=object_id)]
        assert template['durationMs'] == 17314
        flame = next(row for row in template['effectTracks'] if row['effectTrackId'] == 'effect.doll.flame')
        assert flame['resourceId'] == 'effect.kouku.gate3.doll.flame.object-sustain15'
        flame['resourceId'] = DOLL_EFFECT
        # Preserve the user's placement/rotation, animation trim and speed.
        flame['fitEffectToDuration'] = False
        motion['motionEnd'] = 'LOOP'
        motion['loopFullPresentation'] = True
        motion['nextMotionId'] = ''
        changes.append(dict(objectId=object_id, motionId=motion['instanceId'], templateId=template['sequenceId']))
    resource = copy.deepcopy(objects['world.object.mario.striped_ball'])
    resource.update(objectId=BALL, displayName='마리오 소환 공 (원본 크기)',
                    defaultMotionInstanceId=BALL_MOTION, combatBody=body(bounds['ball']))
    resource['combatBody']['shape'] = 'ELLIPSOID'
    assert resource['scale'] == [1, 1, 1]
    assert abs(resource['modelPreScale'] - .01) < 1e-8
    template = copy.deepcopy(templates['sequence.mario.striped_ball.show'])
    template.update(sequenceId=BALL_TEMPLATE, displayName='마리오 소환 공_무지개빛에서 붉은빛 반복', durationMs=7500)
    assert len(template['tracks']) == 1 and len(template['tracks'][0]['keys']) == 2
    template['tracks'][0]['keys'][-1]['timeMs'] = 7500
    template['effectTracks'] = [dict(effectTrackId='effect.circus.ball.aura', slotId='object',
        resourceKind='V1_EFFECT', resourceId=BALL_EFFECT, timing='TIME', followObject=True,
        bone='', startMs=0, durationMs=7500, fitEffectToDuration=False,
        positionOffset=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])]
    motion = copy.deepcopy(instances['world.object.instance.mario.striped_ball.show'])
    motion.update(instanceId=BALL_MOTION, templateId=BALL_TEMPLATE, motionEnd='LOOP', loopFullPresentation=True)
    motion['bindings'][0]['targetId'] = BALL
    doll_index = next(i for i, row in enumerate(result['objectResources']) if row['objectId'] == 'world.object.kouku.odd_doll.large')
    result['objectResources'].insert(doll_index + 1, resource)
    result['templates'].append(template)
    result['instances'].append(motion)
    result['revision'] += 1
    changes.append(dict(objectId=BALL, motionId=BALL_MOTION, templateId=BALL_TEMPLATE))
    return result, changes


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--bounds', type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if not output.is_relative_to(ROOT / 'out'):
        raise ValueError('Staging must stay under repository out/.')
    source_bytes = (ROOT / WORLD).read_bytes()
    bounds = json.loads(args.bounds.read_bytes())
    for item in bounds['inputs']:
        if hashlib.sha256(Path(item['path']).read_bytes()).hexdigest() != item['sha256']:
            raise ValueError('Measured model has changed: ' + item['path'])
    candidate, changes = build(json.loads(source_bytes), bounds)
    path = output / WORLD
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(encode_world(candidate, source_bytes))
    (output / 'world-stage-receipt.json').write_bytes(encode(dict(
        installed=False, source=str(ROOT / WORLD), sourceSha256=hashlib.sha256(source_bytes).hexdigest(),
        candidate=str(path), candidateSha256=hashlib.sha256(path.read_bytes()).hexdigest(),
        sourceRevision=candidate['revision'] - 1, candidateRevision=candidate['revision'],
        changedObjects=changes, visualValidation='USER_PENDING')))
    print(json.dumps(dict(candidate=str(path), objects=len(candidate['objectResources']),
                          templates=len(candidate['templates']), instances=len(candidate['instances']))))


if __name__ == '__main__':
    main()
