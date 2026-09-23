"""Trim the saved encore by 5 seconds, preserving unrelated authored JSON bytes.

Default is candidate-only. --install uses byte freshness checks, backups and
atomic per-file replacement; on failure it rolls back only its own writes.
The original animation and sounds are not re-encoded: their source offsets move.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
import os
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CUT = 5000
CAMERA = 'kouku.bingo.encore.camera.1'
WORLD = 'sequence.kouku.bingo.encore.saydon'
GLASS = 'effect.kouku.bingo.encore.glass.screen'
AREA = 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED'


def replace_row(raw, array, identity, value, change):
    """Decode individual top-level array rows, replacing exactly one stable ID."""
    text = raw.decode('utf-8')
    match = re.search(r'"' + re.escape(array) + r'"\s*:\s*\[', text)
    assert match, array
    cursor = match.end()
    decoder = json.JSONDecoder()
    matches = []
    while True:
        while text[cursor].isspace() or text[cursor] == ',':
            cursor += 1
        if text[cursor] == ']':
            break
        node, end = decoder.raw_decode(text, cursor)
        if node.get(identity) == value:
            matches.append((cursor, end, node))
        cursor = end
    assert len(matches) == 1, (array, value, len(matches))
    begin, end, node = matches[0]
    change(node)
    replacement = json.dumps(node, ensure_ascii=False, separators=(',', ':'), allow_nan=False)
    return (text[:begin] + replacement + text[end:]).encode('utf-8')


def revision(raw):
    text = raw.decode('utf-8')
    text, count = re.subn(r'("revision"\s*:\s*)(\d+)',
        lambda m: m[1] + str(int(m[2]) + 1), text, count=1)
    assert count == 1
    return text.encode('utf-8')


def trim_keys(keys, key='timeMs', cut=CUT):
    previous = [k for k in keys if k[key] <= cut]
    assert previous
    boundary = copy.deepcopy(previous[-1])
    following = [k for k in keys if k[key] > cut]
    # Every cut used here is an exact authored key or lies in a constant segment.
    if boundary[key] != cut and following:
        ignored = {key, 'sceneId'}
        assert {k:v for k,v in boundary.items() if k not in ignored} == {
            k:v for k,v in following[0].items() if k not in ignored}, 'nonconstant cut requires interpolation'
    boundary[key] = 0
    result = [boundary] + copy.deepcopy(following)
    for row in result[1:]:
        row[key] -= cut
    return result


def trim_pattern(pattern, resources):
    assert pattern['durationMs'] == 26322
    assert len(pattern['stages']) == 1 and not pattern['stages'][0]['animationOccurrences']
    pattern['durationMs'] -= CUT
    pattern['stages'][0]['durationMs'] -= CUT
    assert not pattern['logicOccurrences'] and not pattern['summonOccurrences'] and not pattern['sceneProfileOccurrences']
    for world in pattern['worldOccurrences']:
        assert world['startMs'] == 0 and world['durationMs'] == 23333 and world['playbackSpeed'] == 1
        world['durationMs'] -= CUT
    retained = []
    for row in pattern['presentationOccurrences']:
        start, end = row['startMs'], row['startMs'] + row['durationMs']
        if end <= CUT:
            assert resources[row['resourceId']]['kind'] == 'SOUND'
            continue
        row['startMs'] = max(0, start - CUT)
        row['durationMs'] = end - CUT - row['startMs']
        if resources[row['resourceId']]['kind'] == 'SOUND':
            row['soundSourceStartMs'] = row.get('soundSourceStartMs', 0) + max(0, CUT - start)
        retained.append(row)
    pattern['presentationOccurrences'] = retained


def build(originals):
    result = {}
    for path, pattern_id in [
        ('Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json', 'KAKULSAYDON_G1_PATTERN_10'),
        ('Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json', 'KAKULSAYDON_G1_PATTERN_97')]:
        raw = originals[path]
        before = json.loads(raw)
        resources = {r['resourceId']: r for r in before['presentationResources']}
        raw = replace_row(raw, 'patterns', 'patternId', pattern_id, lambda p: trim_pattern(p, resources))
        for resource in resources.values():
            if resource['assetId'] in [CAMERA, GLASS, 'kouku.bingo.encore.fade.black']:
                assert not any(p['patternId'] != pattern_id and any(
                    o['resourceId'] == resource['resourceId'] for o in p['presentationOccurrences']) for p in before['patterns'])
                raw = replace_row(raw, 'presentationResources', 'resourceId', resource['resourceId'],
                    lambda r: r.update(durationMs=18333))
        result[path] = revision(raw)
        # Preserve all other saved patterns and route edits exactly in meaning.
        after = json.loads(result[path])
        assert [p for p in before['patterns'] if p['patternId'] != pattern_id] == [
            p for p in after['patterns'] if p['patternId'] != pattern_id]

    def world(t):
        assert t['durationMs'] == 23333
        t['durationMs'] -= CUT
        for track in t['tracks']:
            track['keys'] = trim_keys(track['keys'])
        for track in t['animationTracks']:
            assert track['startMs'] == 0 and track['playbackRate'] == 1 and not track.get('sourceStartMs', 0)
            track['sourceStartMs'] = CUT
        for track in t['effectTracks']:
            assert track['startMs'] >= CUT
            track['startMs'] -= CUT
    path = AREA + '.worldsequences.json'
    result[path] = revision(replace_row(originals[path], 'templates', 'sequenceId', WORLD, world))

    def camera(s):
        assert s['cameraTrack']['durationMs'] == 23333
        s['defaultHoldMs'] -= CUT
        s['cameraTrack']['durationMs'] -= CUT
        s['cameraTrack']['keyframes'] = trim_keys(s['cameraTrack']['keyframes'])
    path = AREA + '.camerashots.json'
    result[path] = revision(replace_row(originals[path], 'shots', 'shotId', CAMERA, camera))

    path = 'Data/Effects/Authored/' + GLASS + '.effect.json'
    def glass(e):
        assert 12.49 < e['detail']['timing']['startDelaySeconds'] < 12.51
        e['detail']['timing']['startDelaySeconds'] -= CUT / 1000
        e['sourceTransformTrack']['sourceTimeOriginSeconds'] += CUT / 1000
    result[path] = replace_row(originals[path], 'elements', 'id', 'kouku.encore.glass.screen', glass)
    path = 'Data/Effects/V2/Authored/kouku.bingo.encore.fade.black.effectv2.json'
    fade = json.loads(originals[path])
    assert fade['params']['lifetime'] == 23.333
    fade['params']['lifetime'] -= CUT / 1000
    fade['params']['screenPost']['intensityKeys'] = trim_keys(
        fade['params']['screenPost']['intensityKeys'], 'timeSeconds', CUT / 1000)
    result[path] = (json.dumps(fade, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf-8')
    return result


PATHS = [
    'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json',
    'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json',
    AREA + '.worldsequences.json', AREA + '.camerashots.json',
    'Data/Effects/Authored/' + GLASS + '.effect.json',
    'Data/Effects/V2/Authored/kouku.bingo.encore.fade.black.effectv2.json']


def atomic(path, raw):
    temporary = path.with_name(path.name + '.encore-trim.tmp')
    temporary.write_bytes(raw)
    os.replace(temporary, path)


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    originals = {p: (ROOT / p).read_bytes() for p in PATHS}
    candidates = build(originals)
    out = ROOT / 'out/KoukuEncoreTrim20260923'
    for p, raw in candidates.items():
        target = out / 'candidate' / p
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(raw)
        json.loads(raw)
    if args.install:
        assert all((ROOT / p).read_bytes() == raw for p, raw in originals.items()), 'Concurrent saved edits; nothing installed'
        for p, raw in originals.items():
            backup = out / 'before' / p
            backup.parent.mkdir(parents=True, exist_ok=True)
            assert not backup.exists(), 'Backup already exists; inspect prior install before retry'
            backup.write_bytes(raw)
        installed = []
        try:
            for p, raw in candidates.items():
                assert (ROOT / p).read_bytes() == originals[p], 'Concurrent edit: ' + p
                atomic(ROOT / p, raw)
                installed.append(p)
        except Exception:
            for p in reversed(installed):
                if (ROOT / p).read_bytes() == candidates[p]:
                    atomic(ROOT / p, originals[p])
            raise
    print(json.dumps({'installed': args.install, 'cutMs': CUT, 'files': [
        {'path': p, 'sha256': hashlib.sha256(raw).hexdigest()} for p, raw in candidates.items()]}, indent=2))


if __name__ == '__main__':
    main()
