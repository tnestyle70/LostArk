"""Fit the 앵콜컷신 camera to the project Saydon body scale.

Why: the source Matinee draws the boss actor 쿠크세이튼_03 at its native size (the actor and its
skeletal mesh component carry no DrawScale/Scale, so the mesh is used 1:1, wmodel units x 0.01), while the
project body admits MN_RPCT_05 at bodyModelPreScale 0.017. The baked actor is therefore 0.017 / 0.01 = 1.7x
larger than the source frame was authored for, and the head/upper body leave the frame. The source
camera itself (position, aim, FOV) is faithful and is the baseline this tool reads.

What: a uniform scene scale about the actor root does not change the image, so instead of shrinking
the actor the camera is moved to  eye' = A + k (eye - A)  and  lookAt' = A + k (lookAt - A)  with A the
actor root at that time and k = 1.7. Directions, up and FOV are unchanged, so from the camera every
point of the enlarged actor projects exactly where the native-size actor projected in the source.
This is the camera-distance rule used for the 쇼타임 cameras (.md/GB/09-18 SHOWTIME_BERN_CONTINUATION).

The actor root A(t) is read from the installed World Sequence template (positionOffset keys), so the
camera track is keyed at the same times. While the actor is far above the camera (before its first
arrival key) the camera keeps the source pose; it glides to the fitted pose during the arrival window in
which the actor is still off screen.

Only the `keyframes` array of shot kouku.bingo.encore.camera.1 is rewritten (text surgery, every other
byte of camerashots.json is kept) and the document revision is bumped. Without --install a candidate
and a report are written under OUT. --install re-checks the byte baseline and replaces atomically.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
AREA = 'LV_LUT_MIDNIGHTC_ED'
CAMERAS = ROOT / 'Data/Maps/Authoring' / AREA / (AREA + '.camerashots.json')
WORLDS = ROOT / 'Data/Maps/Authoring' / AREA / (AREA + '.worldsequences.json')
OUT = ROOT / 'out/KoukuEncoreCameraFit20260920'
SHOT_ID = 'kouku.bingo.encore.camera.1'
TEMPLATE_ID = 'sequence.kouku.bingo.encore.saydon'
INSTANCE_ID = 'world.sequence.instance.kouku.bingo.encore.saydon'
PROJECT_BODY_PRESCALE = 0.017  # BossCatalog bodyModelPreScale of MN_RPCT_05, the value the baked actor uses
SOURCE_ACTOR_PRESCALE = 0.01   # native cm -> m; the source actor has no DrawScale
DEFAULT_RATIO = PROJECT_BODY_PRESCALE / SOURCE_ACTOR_PRESCALE
NEAR_ACTOR_METRES = 10.0       # the actor counts as "with the camera" once its root is this close to the eye
SIMPLIFY_TOLERANCE_M = 0.005   # linear interpolation of the kept keys may deviate this much from A(t)
MAX_KEYFRAMES = 128            # CLevel_KakulSaydonArena CAMERA_TRACK_MAX_KEYFRAMES / CameraTool limit
CRLF = '\r\n'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def add(a, b):
    return [a[i] + b[i] for i in range(3)]


def sub(a, b):
    return [a[i] - b[i] for i in range(3)]


def mul(a, s):
    return [a[i] * s for i in range(3)]


def norm(a):
    return math.sqrt(sum(x * x for x in a))


def about(anchor, point, ratio):
    """point moved to  anchor + ratio * (point - anchor)."""
    return add(anchor, mul(sub(point, anchor), ratio))


def read_actor_track():
    document = json.loads(WORLDS.read_bytes().decode('utf-8-sig'))
    template = next(t for t in document['templates'] if t['sequenceId'] == TEMPLATE_ID)
    instance = next(i for i in document['instances'] if i['instanceId'] == INSTANCE_ID)
    assert instance['templateId'] == TEMPLATE_ID and instance['anchorKind'] == 'WORLD', instance
    assert instance['position'] == [0, 0, 0] and instance['playbackSpeed'] == 1 and instance['startDelayMs'] == 0, instance
    assert len(template['tracks']) == 1 and template['tracks'][0]['slotId'] == 'actor', 'unexpected template tracks'
    keys = template['tracks'][0]['keys']
    times = [k['timeMs'] for k in keys]
    assert times == sorted(times) and len(set(times)) == len(times), 'actor key times must strictly increase'
    return times, [list(k['positionOffset']) for k in keys], template['durationMs']


def simplify(times, positions, tolerance):
    """Greedy key reduction: linear interpolation of the kept keys stays within tolerance metres."""
    keep = [0]
    i = 0
    count = len(times)
    while i < count - 1:
        best = i + 1
        j = i + 1
        while j < count:
            ok = True
            for m in range(i + 1, j):
                u = (times[m] - times[i]) / (times[j] - times[i])
                q = add(mul(positions[i], 1 - u), mul(positions[j], u))
                if max(abs(q[c] - positions[m][c]) for c in range(3)) > tolerance:
                    ok = False
                    break
            if not ok:
                break
            best = j
            j += 1
        keep.append(best)
        i = best
    return keep


def fitted_keyframes(source_key, duration_ms, ratio):
    eye, look, up, fov = source_key['eye'], source_key['lookAt'], source_key['up'], source_key['fovYDegrees']
    times, positions, template_duration = read_actor_track()
    assert template_duration == duration_ms, ('actor template and camera durations differ', template_duration, duration_ms)
    keep = simplify(times, positions, SIMPLIFY_TOLERANCE_M)
    near = [i for i in keep if norm(sub(positions[i], eye)) < NEAR_ACTOR_METRES]
    assert near, 'the actor never comes near the camera'
    first_near = near[0]
    assert first_near > 0 and keep.index(first_near) > 0, 'expected an arrival step key'
    last_far = keep[keep.index(first_near) - 1]
    # The actor sits on a plateau right after its arrival step, still below the camera; the camera
    # glides to the fitted pose across that plateau instead of jumping on the arrival key.
    fitted_times = list(near)
    if len(fitted_times) > 1 and max(abs(positions[fitted_times[0]][c] - positions[fitted_times[1]][c])
                                     for c in range(3)) <= SIMPLIFY_TOLERANCE_M:
        fitted_times = fitted_times[1:]
    rows = [(times[0], eye, look)]
    if times[last_far] != times[0]:
        rows.append((times[last_far], eye, look))
    for i in fitted_times:
        rows.append((times[i], about(positions[i], eye, ratio), about(positions[i], look, ratio)))
    if rows[-1][0] != duration_ms:
        rows.append((duration_ms, rows[-1][1], rows[-1][2]))
    assert len(rows) <= MAX_KEYFRAMES, ('too many camera keyframes', len(rows))
    frames = []
    for n, (t, e, l) in enumerate(rows):
        frames.append(dict(sceneId='kouku.bingo.encore.camera1.k%d' % n, timeMs=t, eye=e, lookAt=l, up=list(up),
                           fovYDegrees=fov))
    return frames


def same_track(a, b, tolerance=1e-9):
    if len(a) != len(b):
        return False
    for x, y in zip(a, b):
        if x['sceneId'] != y['sceneId'] or x['timeMs'] != y['timeMs'] or x['fovYDegrees'] != y['fovYDegrees'] or x['up'] != y['up']:
            return False
        for key in ('eye', 'lookAt'):
            if max(abs(x[key][c] - y[key][c]) for c in range(3)) > tolerance:
                return False
    return True


def shot_span(text):
    marker = '"shotId": "%s"' % SHOT_ID
    assert text.count(marker) == 1, 'shot id must exist exactly once'
    at = text.index(marker)
    start = text.rfind(CRLF + '    {' + CRLF, 0, at)
    end = text.index(CRLF + '    }', at)
    assert start >= 0 and end > at
    return start, end


def keyframes_span(text, start, end):
    opening = '"keyframes": [' + CRLF
    k0 = text.index(opening, start, end) + len(opening)
    k1 = text.index(CRLF + '        ]', k0, end)
    return k0, k1


def render_frames(frames):
    chunks = []
    for frame in frames:
        body = json.dumps(frame, ensure_ascii=False, indent=2, allow_nan=False)
        chunks.append(CRLF.join('          ' + line for line in body.split('\n')))
    return (',' + CRLF).join(chunks)


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--ratio', type=float, default=DEFAULT_RATIO, help='camera distance ratio (default 0.017/0.01)')
    args = parser.parse_args()

    raw = CAMERAS.read_bytes()
    text = raw.decode('utf-8')
    document = json.loads(text)
    shot = next(s for s in document['shots'] if s['shotId'] == SHOT_ID)
    current = shot['cameraTrack']['keyframes']
    source_key = current[0]
    duration_ms = shot['cameraTrack']['durationMs']
    assert source_key['timeMs'] == 0 and shot['eye'] == source_key['eye'] and shot['lookAt'] == source_key['lookAt'], \
        'the first keyframe must be the source camera pose'

    frames = fitted_keyframes(source_key, duration_ms, args.ratio)
    baseline = [source_key, dict(source_key, sceneId=current[-1]['sceneId'], timeMs=duration_ms)]
    pristine = current == baseline
    already = False
    if not pristine:
        # A track this tool wrote can be refitted with another ratio: infer the ratio from the last key
        # (eye' = A + k (eye - A)) and require the whole track to be exactly what this tool generates for it.
        times, positions, _ = read_actor_track()
        last_offset = sub(source_key['eye'], positions[-1])
        inferred = sum((current[-1]['eye'][c] - positions[-1][c]) * last_offset[c] for c in range(3)) / sum(x * x for x in last_offset)
        assert same_track(current, fitted_keyframes(source_key, duration_ms, inferred)), \
            'the shot keyframes were edited after the source import; refusing to overwrite them'
        already = abs(inferred - args.ratio) < 1e-9
        print('current track was fitted with ratio %.6f' % inferred)

    OUT.mkdir(parents=True, exist_ok=True)
    report = dict(shotId=SHOT_ID, ratio=args.ratio, alreadyFitted=already, keyframes=len(frames), baselineSha256=sha(raw),
                  frames=[dict(timeMs=f['timeMs'], eye=f['eye'], lookAt=f['lookAt']) for f in frames])
    if already:
        print('already fitted: %d keyframes, nothing to change' % len(frames))
        (OUT / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
        return

    start, end = shot_span(text)
    k0, k1 = keyframes_span(text, start, end)
    candidate = text[:k0] + render_frames(frames) + text[k1:]
    revisions = re.findall(r'(?m)^  "revision": (\d+),\r$', candidate)
    assert len(revisions) == 1, revisions
    candidate = re.sub(r'(?m)^  "revision": %s,\r$' % revisions[0], '  "revision": %d,\r' % (int(revisions[0]) + 1),
                       candidate, count=1)
    payload = candidate.encode('utf-8')
    parsed = json.loads(payload.decode('utf-8'))
    assert [s['shotId'] for s in parsed['shots']] == [s['shotId'] for s in document['shots']]
    for old, new in zip(document['shots'], parsed['shots']):
        if old['shotId'] != SHOT_ID:
            assert old == new, ('another shot changed', old['shotId'])
    fitted = next(s for s in parsed['shots'] if s['shotId'] == SHOT_ID)
    assert fitted['cameraTrack']['keyframes'] == frames
    fitted_no_keys = json.loads(json.dumps(fitted))
    fitted_no_keys['cameraTrack']['keyframes'] = current
    assert fitted_no_keys == shot, 'more than the keyframes changed'

    (OUT / 'candidate.camerashots.json').write_bytes(payload)
    report['candidateSha256'] = sha(payload)
    report['newRevision'] = parsed['revision']
    if args.install:
        backup = OUT / 'backup.camerashots.json'
        if not backup.exists():
            backup.write_bytes(raw)
        assert sha(CAMERAS.read_bytes()) == report['baselineSha256'], 'camerashots.json changed while preparing; rerun'
        temporary = CAMERAS.with_name(CAMERAS.name + '.encorefit.tmp')
        temporary.write_bytes(payload)
        os.replace(temporary, CAMERAS)
        assert sha(CAMERAS.read_bytes()) == report['candidateSha256'], 'installed bytes differ from the candidate'
        report['installed'] = True
    (OUT / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print('%s: %d keyframes (ratio %.4f), revision %d' % ('installed' if args.install else 'candidate written',
                                                        len(frames), args.ratio, parsed['revision']))
    for f in frames:
        print('  %6d ms  eye %s' % (f['timeMs'], [round(v, 3) for v in f['eye']]))


if __name__ == '__main__':
    main()
