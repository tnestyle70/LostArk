"""Prepare a reviewable SCENE01B recovery against current saved authoring.

Never installs or publishes. Every output is written below --output, with a
field/stable-ID change list and the exact live input hashes for later merging.
Actor and camera candidates must already have passed their source audits.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path

import build_source_sequences as sequences
from source_scene_clock import SourceSceneClock

ROOT = Path(__file__).resolve().parents[2]
AREA = Path('Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED')
WORLD = AREA / 'LV_LUT_MIDNIGHTC_ED.worldsequences.json'
CAMERA = AREA / 'LV_LUT_MIDNIGHTC_ED.camerashots.json'
COMPOSITIONS = {
    Path('Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json'): 'KAKULSAYDON_G1_PATTERN_9',
    Path('Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'): 'KAKULSAYDON_G1_PATTERN_75',
}
PREFIX = 'kouku.bingo.ending.'
FX = 'effect.kouku.sequence.lv_lut_midnightc_ed_scene01b.efseqact_matinee_0.1'


def read(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))


def write(path, document, baseline=None):
    path.parent.mkdir(parents=True, exist_ok=True)
    def world_text(value, depth=0):
        # Match native WORLD Save's single-line sample/vector layout. Fully
        # expanding every vector would triple document size on the next load.
        encode = lambda item: json.dumps(item, ensure_ascii=False, allow_nan=False)
        if isinstance(value, dict):
            if 'timeMs' in value:
                return encode(value)
            return '{\n' + ',\n'.join('  ' * (depth+1) + encode(k) + ': ' + world_text(v, depth+1)
                for k, v in value.items()) + '\n' + '  ' * depth + '}'
        if isinstance(value, list) and any(isinstance(x, (dict, list)) for x in value):
            return '[\n' + ',\n'.join('  ' * (depth+1) + world_text(x, depth+1) for x in value) + '\n' + '  ' * depth + ']'
        return encode(value)
    text = world_text(document) if path.name.endswith('.worldsequences.json') else json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False)
    if baseline is not None:
        original = baseline.decode('utf-8-sig')
        old_document = json.loads(original)
        decoder = json.JSONDecoder()
        replacements = []
        def skip(index):
            while index < len(original) and original[index].isspace(): index += 1
            return index
        cursor = skip(original.index('{') + 1)
        while original[cursor] != '}':
            key, cursor = decoder.raw_decode(original, cursor)
            cursor = skip(cursor); assert original[cursor] == ':'
            start = skip(cursor + 1); old_value, end = decoder.raw_decode(original, start)
            if old_value != document[key]:
                value = document[key]
                if isinstance(old_value, list) and isinstance(value, list):
                    spans = []; item_cursor = skip(start + 1)
                    while original[item_cursor] != ']':
                        item, item_end = decoder.raw_decode(original, item_cursor)
                        spans.append((item, original[item_cursor:item_end]))
                        item_cursor = skip(item_end)
                        if original[item_cursor] == ',': item_cursor = skip(item_cursor + 1)
                    pieces = []
                    for item in value:
                        unchanged = next((raw for old, raw in spans if old == item), None)
                        pieces.append(unchanged if unchanged is not None else world_text(item, 2))
                    replacement = '[\n' + ',\n'.join('    ' + item for item in pieces) + '\n  ]' if pieces else '[]'
                else:
                    replacement = world_text(value, 1)
                replacements.append((start, end, replacement))
            cursor = skip(end)
            if original[cursor] == ',': cursor = skip(cursor + 1)
        assert old_document.keys() == document.keys(), 'Root additions require explicit merge handling'
        text = original
        for start, end, replacement in reversed(replacements):
            text = text[:start] + replacement + text[end:]
        assert json.loads(text) == document
        text = text.rstrip('\r\n')
    newline = '\r\n' if baseline is not None and b'\r\n' in baseline else '\n'
    text = text.replace('\r\n', '\n').replace('\r', '')
    path.write_bytes(((text.rstrip('\n') + '\n').replace('\n', newline)).encode('utf-8'))


def changes(before, after, path=()):
    """Readable merge receipt; arrays with identities are compared by stable ID."""
    if before == after:
        return []
    if isinstance(before, dict) and isinstance(after, dict):
        return [change for key in sorted(before.keys() | after.keys())
                for change in changes(before.get(key), after.get(key), path + (key,))]
    if isinstance(before, list) and isinstance(after, list) and before and after:
        identity = next((key for key in ('patternId', 'occurrenceId', 'resourceId', 'worldId',
            'objectId', 'sequenceId', 'instanceId', 'shotId', 'stageId')
            if all(isinstance(row, dict) and key in row for row in before + after)), None)
        if identity:
            left = {row[identity]: row for row in before}; right = {row[identity]: row for row in after}
            assert len(left) == len(before) and len(right) == len(after)
            return [change for key in sorted(left.keys() | right.keys())
                    for change in changes(left.get(key), right.get(key), path + (f'{identity}={key}',))]
    return [dict(path=list(path), before=before, after=after)]


def prepare(args):
    output = args.output.resolve()
    if output == ROOT / 'out' or not output.is_relative_to(ROOT / 'out'):
        raise ValueError('Candidate output must be a dedicated directory below repository out')
    actor = read(args.actor_patch); camera = read(args.camera_candidate); attachments = read(args.attachment_patch)
    audio = read(args.audio_patch)
    source = read(args.source_scene); rows = {int(k): v for k, v in source['rows'].items()}
    clock = SourceSceneClock.from_scene_rows(rows, 45)
    duration = round(clock.elapsed_duration_ms)
    baseline, documents = {}, {}

    def load(relative):
        raw = (ROOT / relative).read_bytes()
        baseline[relative] = raw
        documents[relative] = json.loads(raw.decode('utf-8-sig'))
        return documents[relative]

    world = load(WORLD)
    for collection, identity, patch_name in [('templates', 'sequenceId', 'changes'),
                                            ('objectResources', 'objectId', 'objectChanges')]:
        by_id = {row[identity]: row for row in world[collection]}
        for patch in actor[patch_name]:
            assert '.bingo.ending.' in patch[identity]
            target = by_id[patch[identity]]
            for field, value in patch['fields'].items():
                target[field] = copy.deepcopy(value['after'])
    # Only absent components are added. Native CWorldSequenceObject owns the hats.
    for collection, identity in [('objectResources', 'objectId'), ('templates', 'sequenceId'), ('instances', 'instanceId')]:
        for row in attachments[collection]:
            assert row[identity].endswith('.weapon'), row[identity]
            existing = next((x for x in world[collection] if x[identity] == row[identity]), None)
            if existing is not None and existing != row:
                raise ValueError('Attachment identity already has a different saved definition: ' + row[identity])
            if existing is None:
                world[collection].append(copy.deepcopy(row))
    world['revision'] += 1
    cameras = load(CAMERA)
    replacements = {shot['shotId']: shot for shot in camera['shots']}
    assert len(replacements) == 12
    for index, shot in enumerate(cameras['shots']):
        if shot['shotId'] in replacements:
            cameras['shots'][index] = copy.deepcopy(replacements.pop(shot['shotId']))
    assert not replacements
    cameras['revision'] += 1
    shot_windows = {shot['shotId']: (shot['elapsedStartMs'], shot['elapsedEndMs']) for shot in camera['audit']}
    subtitles = {row['strmsgid']: row for row in rows[29]['p']['subtitleinfoarr']}
    full_clock = clock.source_clock_keys(maximum_error_ms=.05)
    label_by_instance = {'world.sequence.instance.' + row['sequenceId'].removeprefix('sequence.'): row['displayName']
                         for row in world['templates'] if row['sequenceId'].startswith('sequence.' + PREFIX)}
    fx_durations = set()
    for relative, pattern_id in COMPOSITIONS.items():
        document = load(relative)
        pattern = next(row for row in document['patterns'] if row['patternId'] == pattern_id)
        resources = {row['resourceId']: row for row in document['presentationResources']}
        for occurrence in pattern['presentationOccurrences']:
            resource = resources[occurrence['resourceId']]
            asset = resource['assetId']
            if resource['kind'] == 'CAMERA':
                start, end = shot_windows[asset]
                occurrence.update(startMs=start, durationMs=end-start)
                resource['durationMs'] = end-start
            elif resource['kind'] == 'SUBTITLE':
                key = subtitles[asset]
                start = round(clock.to_elapsed_ms(key['time'] * 1000.))
                end = round(clock.to_elapsed_ms((key['time'] + key['duration']) * 1000.))
                occurrence.update(startMs=start, durationMs=end-start)
            elif resource['kind'] == 'EFFECT' and asset == FX:
                # Native emitter lifetime outlives Matinee; continue at its final speed 1.
                source_end = resource['durationMs']
                assert source_end > clock.source_duration_ms
                elapsed_end = round(clock.elapsed_duration_ms + source_end - clock.source_duration_ms)
                keys = copy.deepcopy(full_clock)
                keys.append(dict(timeMs=elapsed_end, sourceMs=source_end))
                occurrence.update(startMs=0, durationMs=elapsed_end, effectSourceTimeKeys=keys,
                    effectSourceStartMs=0, fitEffectToDuration=False, loopEffectToDuration=False,
                    fadeInMs=0, fadeOutMs=0)
                fx_durations.add(elapsed_end)
            elif resource['kind'] == 'SOUND' and resource.get('soundEventId') == 's_scene_ocean3_3.scene_midnightc_ed_raidcleared':
                occurrence['startMs'] = round(clock.to_elapsed_ms(100.))
        worlds = {row['worldId']: row for row in document['worlds']}
        for occurrence in pattern['worldOccurrences']:
            definition = worlds[occurrence['worldId']]
            assert definition['sequenceInstanceId'] in label_by_instance
            definition['displayName'] = label_by_instance[definition['sequenceInstanceId']]
            occurrence.update(startMs=0, durationMs=duration, playbackSpeed=1.)
        for instance in attachments['instances']:
            definition = next((row for row in document['worlds'] if row['sequenceInstanceId'] == instance['instanceId']), None)
            if definition is None:
                identity = f"kakulsaydon.g1.world.{document['nextWorldOrdinal']}"
                document['nextWorldOrdinal'] += 1
                definition = copy.deepcopy(next(iter(worlds.values())))
                definition.update(worldId=identity, displayName=label_by_instance[instance['instanceId']],
                    sequenceInstanceId=instance['instanceId'], positionOffset=[0,0,0], anchorKind='NONE',
                    anchorPosition=[0,0,0], companionEffectResourceId='')
                definition.pop('objectResourceId', None)
                document['worlds'].append(definition)
            if not any(x['worldId'] == definition['worldId'] for x in pattern['worldOccurrences']):
                ordinal = pattern['nextWorldOccurrenceOrdinal']; pattern['nextWorldOccurrenceOrdinal'] += 1
                pattern['worldOccurrences'].append(dict(occurrenceId=f'{pattern_id}.world.{ordinal}',
                    worldId=definition['worldId'], startMs=0, durationMs=duration, playbackSpeed=1.))
        sound_patch = next(row for row in audio['documents'] if row['path'] == relative.as_posix())
        assert sound_patch['patternId'] == pattern_id
        for patch in sound_patch['presentationOccurrenceChanges']:
            occurrence = next(x for x in pattern['presentationOccurrences'] if x['occurrenceId'] == patch['occurrenceId'])
            assert occurrence['resourceId'] == patch['resourceId']
            occurrence.update(patch['fields'])
        for patch in sound_patch['presentationResourceChanges']:
            resources[patch['resourceId']].update(patch['fields'])
        end = max(duration, *(x['startMs'] + x['durationMs'] for x in pattern['presentationOccurrences']))
        assert len(pattern['stages']) == 1 and not pattern['stages'][0]['animationOccurrences']
        pattern['durationMs'] = pattern['stages'][0]['durationMs'] = end
        document['revision'] += 1
        # Reuse the existing screen fade consumer, keeping this asset scene-specific.
        fade_id = PREFIX + 'fade.black'
        resource_id = 'presentation.' + fade_id
        if resource_id in resources:
            raise ValueError('Fade already authored; inspect it before preparing replacement')
        document['presentationResources'].append(sequences.resource(resource_id, '빙고 최종엔딩 / 원본 암전',
            'EFFECT', fade_id, end, 'LEAF'))
        ordinal = pattern['nextPresentationOccurrenceOrdinal']; pattern['nextPresentationOccurrenceOrdinal'] += 1
        occurrence = sequences.occurrence(pattern_id, ordinal, resource_id, 0, end, 'MAP', [0,0,0])
        occurrence['followBoss'] = False
        pattern['presentationOccurrences'].append(occurrence)
    assert len(fx_durations) == 1
    end = fx_durations.pop()
    fade = sequences.fade(dict(data=45, start=0, duration=49083, prefix=PREFIX[:-1], name='빙고 최종엔딩'), rows)
    assert fade is not None
    fade['params']['lifetime'] = end / 1000.
    fade_keys = fade['params']['screenPost']['intensityKeys']
    for key in fade_keys:
        key['timeSeconds'] = clock.to_elapsed_ms(key['timeSeconds'] * 1000.) / 1000.
    fade_keys.append(dict(timeSeconds=end / 1000., intensity=fade_keys[-1]['intensity']))
    fade_path = Path('Data/Effects/V2/Authored') / (fade['effectId'] + '.effectv2.json')
    if (ROOT/fade_path).exists():
        raise ValueError('Fade file already exists; inspect it before replacing')
    documents[fade_path] = fade
    independent = load(Path('Data/Effects/V2/Independent.json'))
    assert fade['effectId'] not in independent['effects']
    independent['effects'].append(fade['effectId'])
    catalog_patch = audio['catalogChange']
    catalog = load(Path(catalog_patch['path']))
    variants = catalog['classes'][catalog_patch['characterClass']]
    assert variants[catalog_patch['soundEventId']] == catalog_patch['before']
    variants[catalog_patch['soundEventId']] = catalog_patch['variants']
    file_rows = []
    for relative, document in documents.items():
        original = json.loads(baseline[relative].decode('utf-8-sig')) if relative in baseline else None
        patch = changes(original, document)
        write(output/relative, document, baseline.get(relative))
        file_rows.append(dict(path=relative.as_posix(), baselineSha256=hashlib.sha256(baseline[relative]).hexdigest()
            if relative in baseline else None, changes=patch))
    for relative, raw in baseline.items():
        if (ROOT/relative).read_bytes() != raw:
            raise ValueError('Saved input changed while preparing candidate: ' + str(relative))
    for relative, pattern_id in COMPOSITIONS.items():
        before = json.loads(baseline[relative].decode('utf-8-sig'))
        assert [p for p in before['patterns'] if p['patternId'] != pattern_id] == [p for p in documents[relative]['patterns'] if p['patternId'] != pattern_id]
    # Registration is also a candidate, not an edit to the active project.
    for path, raw, candidate in sequences.project_registration([ROOT/fade_path]):
        relative = path.relative_to(ROOT)
        target = output/relative; target.parent.mkdir(parents=True, exist_ok=True); target.write_bytes(candidate)
        file_rows.append(dict(path=relative.as_posix(), baselineSha256=hashlib.sha256(raw).hexdigest(), registrationOnly=True))
    receipt = dict(installed=False, published=False, sourceScene='SCENE01B', sceneEndMs=duration,
        effectTailEndMs=end, scope=list(COMPOSITIONS.values()), files=file_rows,
        preserved='Every other pattern including Sequence P10; shared Effect asset bytes; team render tuning',
        pending=['Final native candidate admission and freshness recheck before install/publish',
                 'Original DOF/shake/fog renderer audit'])
    write(output/'recovery-field-patch.json', receipt)
    print(json.dumps({k:v for k,v in receipt.items() if k != 'files'}, ensure_ascii=True, indent=2))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--source-scene', required=True, type=Path)
    parser.add_argument('--camera-candidate', required=True, type=Path)
    parser.add_argument('--actor-patch', required=True, type=Path)
    parser.add_argument('--attachment-patch', required=True, type=Path)
    parser.add_argument('--audio-patch', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    prepare(parser.parse_args())
