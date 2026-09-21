"""Add the 앵콜컷신 (SCENE07A Matinee 23, 23333ms) entry to the Kouku Boss and Sequence compositions.

The entry is created the way the existing 빙고_최종엔딩씬 was: one BINGO-gate Pattern per
composition (the Boss tab document and the Sequence tab document), whose presentation
occurrences reference camera shots sampled from the source Matinee
(build_gate2_intro_composition.make_cameras) and the source fade track
(build_source_sequences.fade). The Pattern is cloned from the existing 빙고_최종엔딩씬 of the
same document so every field the Workbench writes stays in its saved form.

Only the encore entry is generated. build_gate_cutscenes_g12.py must not be re-run for it:
that script regenerates every other cutscene of CONFIGS.

Not represented (no authoring resource kind exists, or the baker cannot reproduce it): subtitles,
the fake "던전 클리어" emblem UI, the broken-glass post-render material, particle groups, the Saydon
light and the sound events. See the RESULT document.

Without --install only candidate documents are written under OUT. --install checks a byte
baseline of every target immediately before writing and replaces each file atomically.

Second pass (--actor / --install-actor): adds the boss actor 쿠크세이튼_03 (Matinee group 51) the way
the other source Sequences carry Saydon (build_source_sequences.actor_world): one new 30 ticks/s
WANM appended to a candidate copy of the Character MN_RPCT_05 body, preserving its mesh,
skeleton, materials and existing clips. Installation appends only that named WANM, plus one World
Sequence object/template/instance appended to worldsequences.json (text insertion, existing bytes
untouched) and one `worlds` row + one worldOccurrence in each composition document. Not baked by the
baker: the skelcontrolstrength tracks (j_dn, h_dn, h_up, ee_up, l_t2), the fc1 slot and hit_color.

Third pass (fit_encore_camera_to_actor_scale.py, run after --install-actor): the source draws this actor
at native size while the project body is admitted at 0.017, so the 2-key source camera written here is
re-keyed 1.7x farther from the actor root. This script only writes the source camera and never undoes that
pass: the camera shot id collision assert below stops a second --install.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
import re
import shutil
import sys
from pathlib import Path

from scipy.spatial.transform import Rotation

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build_gate2_intro_composition as base  # noqa: E402
import build_source_sequences as bss  # noqa: E402
from bake_character_cinematic_clips import install_baked_clip  # noqa: E402

ROOT = base.ROOT
OUT = ROOT / 'out/KoukuEncoreComposition20260920'
PACKAGE = 'B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8L6.upk'  # SCENE07A, Matinee efseqact_matinee_23
TEMPLATE_NAME = '빙고_최종엔딩씬'
CONFIG = dict(name='앵콜컷신', prefix='kouku.bingo.encore', scene='SCENE07A', matinee=21, data=44,
              duration=23333, start=0, gate='BINGO')
PATHS = dict(
    boss=ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json',
    sequence=ROOT / 'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json',
    cameras=bss.AREA / (base.AREA + '.camerashots.json'),
    independent=ROOT / 'Data/Effects/V2/Independent.json',
)
CRLF = '\r\n'


def dump_doc(document):
    """The layout the Workbench saved these documents in: indent 2, CRLF (verified byte-identical)."""
    return (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n').replace('\n', CRLF).encode('utf-8')


def scene_rows():
    cache = OUT / 'SCENE07A.json'
    if not cache.exists():
        rows, imports = base.extract_scene(base.PACKAGES / PACKAGE)
        base.write(cache, dict(rows=rows, imports=imports))
    return {int(k): v for k, v in base.read(cache)['rows'].items()}


def find_template(document):
    matches = [p for p in document['patterns'] if p['displayName'] == TEMPLATE_NAME and p['gateId'] == CONFIG['gate']]
    assert len(matches) == 1, ('template pattern', TEMPLATE_NAME, len(matches))
    return matches[0]


def build_pattern(template, pattern_id, shots, leaf):
    duration = CONFIG['duration'] - CONFIG['start']
    pattern = copy.deepcopy(template)
    pattern['patternId'] = pattern_id
    pattern['displayName'] = CONFIG['name']
    assert len(pattern['stages']) == 1
    pattern['stages'][0]['actionId'] = pattern_id + '.stage.1'
    pattern['stages'][0]['durationMs'] = duration
    if 'durationMs' in pattern:  # the Boss document stores the Pattern length as well
        pattern['durationMs'] = duration
    occurrences = []
    ordinal = 1
    for start, end, shot in shots:
        occurrences.append(bss.occurrence(pattern_id, ordinal, 'presentation.' + shot['shotId'], start, end))
        ordinal += 1
    if leaf:
        occurrences.append(bss.occurrence(pattern_id, ordinal, 'presentation.' + leaf['effectId'], 0, duration, 'MAP'))
        ordinal += 1
    pattern['presentationOccurrences'] = occurrences
    pattern['nextPresentationOccurrenceOrdinal'] = ordinal
    return pattern


def add_to_composition(document, shots, leaf):
    """Append the pattern and its presentation resources; never edit an existing row."""
    duration = CONFIG['duration'] - CONFIG['start']
    assert not any(p['displayName'] == CONFIG['name'] for p in document['patterns']), 'already present'
    template = find_template(document)
    pattern_id = 'KAKULSAYDON_G1_PATTERN_' + str(document['nextPatternOrdinal'])
    assert not any(p['patternId'] == pattern_id for p in document['patterns']), pattern_id
    resources = [bss.resource('presentation.' + s['shotId'], s['displayName'], 'CAMERA', s['shotId'], e - b)
                 for b, e, s in shots]
    if leaf:
        resources.append(bss.resource('presentation.' + leaf['effectId'], leaf['displayName'], 'EFFECT',
                                      leaf['effectId'], duration, 'LEAF'))
    bss.merge(document, 'presentationResources', 'resourceId', resources)
    document['patterns'].append(build_pattern(template, pattern_id, shots, leaf))
    document['nextPatternOrdinal'] += 1
    document['revision'] += 1
    return pattern_id


def insert_shots(raw, shots):
    """Append shot objects after the last existing one without touching any existing byte.

    camerashots.json mixes float spellings (script shortest form and the tool's 17 digits), so it
    is edited as text, not re-serialized."""
    text = raw.decode('utf-8')
    tail = CRLF + '  ]' + CRLF + '}' + CRLF
    assert text.endswith(tail), 'unexpected camerashots layout'
    chunks = []
    for shot in shots:
        body = json.dumps(shot, ensure_ascii=False, indent=2, allow_nan=False)
        chunks.append(CRLF.join('    ' + line for line in body.split('\n')))
    text = text[:-len(tail)] + ',' + CRLF + (',' + CRLF).join(chunks) + tail
    revision = re.findall(r'(?m)^  "revision": (\d+),\r$', text)
    assert len(revision) == 1, revision
    text = re.sub(r'(?m)^  "revision": %s,\r$' % revision[0], '  "revision": %d,\r' % (int(revision[0]) + 1), text, count=1)
    return text.encode('utf-8')


def sha(data):
    return hashlib.sha256(data).hexdigest()


ACTOR = dict(group=51, source='Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel', label='Saydon',
             pre_scale=.017, source_scale=1.,
             material_source='Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel')
WORLDS_PATH = bss.AREA / (base.AREA + '.worldsequences.json')
ACTOR_WMODEL = ACTOR['source']
ACTOR_DONOR = 'Map/KakulSaydon/SourceSequences/%s/%s/%s.wmodel' % (CONFIG['prefix'], ACTOR['label'], ACTOR['label'])


def find_array(text, key):
    """(open, close) indexes of the top-level array `key` in compact JSON; string and escape aware."""
    marker = '"%s":[' % key
    assert text.count(marker) == 1, (key, text.count(marker))
    start = text.index(marker) + len(marker) - 1
    depth, in_string, escaped = 0, False, False
    for index in range(start, len(text)):
        ch = text[index]
        if in_string:
            if escaped:
                escaped = False
            elif ch == '\\':
                escaped = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
        elif ch in '[{':
            depth += 1
        elif ch in ']}':
            depth -= 1
            if depth == 0:
                return start, index
    raise AssertionError(key)


def append_compact(raw, additions):
    """Append rows to top-level arrays of the compact World Sequence document; existing bytes stay."""
    text = raw.decode('utf-8')
    spans = {key: find_array(text, key) for key in additions}
    for key in sorted(additions, key=lambda k: spans[k][1], reverse=True):
        close = spans[key][1]
        assert text[close - 1] not in '[,', ('empty array', key)
        chunk = ','.join(json.dumps(row, ensure_ascii=False, separators=(',', ':'), allow_nan=False)
                         for row in additions[key])
        text = text[:close] + ',' + chunk + text[close:]
    head = text[:300]
    match = re.search(r'"revision":(\d+)', head)
    assert match and head.count('"revision":') == 1, head
    text = text[:match.start(1)] + str(int(match.group(1)) + 1) + text[match.end(1):]
    return text.encode('utf-8')


def prepare_candidate_resources():
    """Bake against a candidate Resources root so nothing is written to the project until --install-actor."""
    root = OUT / 'candidate_resources'
    source = base.RESOURCES / ACTOR['source']
    linked = root / ACTOR['source']
    linked.parent.mkdir(parents=True, exist_ok=True)
    # Canonical bodies may gain new clips between runs. Keep a fresh independent
    # copy; an in-place write to a hard link could otherwise alter the live body.
    temporary = linked.with_name(linked.name + '.source.tmp')
    shutil.copy2(source, temporary)
    os.replace(temporary, linked)
    return root


def build_actor(rows):
    """Bake into the candidate root; the material-slot check of the derived model sees the candidate file."""
    project_resources = base.RESOURCES
    root = prepare_candidate_resources()
    material_module = sys.modules[bss.inherit_material_source.__module__]
    original_resource_path = material_module.resource_path

    def resource_path(repository_root, asset_id):
        if asset_id == ACTOR_WMODEL:
            candidate = root / asset_id
            if not candidate.is_file():
                raise ValueError('Missing candidate input: ' + asset_id)
            return candidate
        return original_resource_path(repository_root, asset_id)

    base.RESOURCES = root
    material_module.resource_path = resource_path
    try:
        obj, template, instance, world = bss.actor_world(CONFIG, rows, ACTOR['group'], ACTOR['source'],
                                                         ACTOR['label'], ACTOR['pre_scale'], ACTOR['source_scale'],
                                                         ACTOR['material_source'])
    finally:
        base.RESOURCES = project_resources
        material_module.resource_path = original_resource_path
    assert obj['modelAssetId'] == ACTOR_WMODEL, obj['modelAssetId']
    add_step_keys(rows, template)
    stage_textures(root / ACTOR_WMODEL)
    return obj, template, instance, world, root / ACTOR_WMODEL


def stage_textures(candidate_model):
    """The baked WModel keeps the source material paths, so the source textures/ folder sits next to it."""
    source = base.RESOURCES / Path(ACTOR['source']).parent / 'textures'
    target = candidate_model.parent / 'textures'
    target.mkdir(exist_ok=True)
    for file in sorted(source.iterdir()):
        linked = target / file.name
        if not linked.exists():
            try:
                os.link(file, linked)
            except OSError:
                shutil.copy2(file, linked)


def add_step_keys(rows, template):
    """Give every cim_constant step its arrival key and refine the moving curve segments.

    base.source_times samples a step at round(ms) and ms-1 with world_pose(ms / 1000). A source key at
    7833.339 ms rounds down to 7833, so world_pose still returns the old value there and the new value only
    arrives at the next 200 ms boundary: a 167 ms slide instead of a step. Sample the first whole millisecond
    after each step as well (the 1 ms segment between the two keys is the step).
    """
    move = next(t for t in base.active_tracks(rows, ACTOR['group']) if t['cls'] == 'interptrackmove')['p']
    actor = base.group_actor(rows, ACTOR['group'], CONFIG['matinee'])[0]
    keys = template['tracks'][0]['keys']
    present = {k['timeMs'] for k in keys}
    arrivals = set()
    for field in ('postrack', 'eulertrack'):
        points = move[field]['points']
        for before, after in zip(points, points[1:]):
            if before.get('interpmode') == 'cim_constant':
                arrival = math.floor(after['inval'] * 1000) + 1
                if CONFIG['start'] < arrival < CONFIG['duration']:
                    arrivals.add(arrival)
    # base.source_times samples a moving curve segment every 200 ms; refine it to 33 ms (about one 30 Hz tick).
    for field in ('postrack', 'eulertrack'):
        points = move[field]['points']
        for before, after in zip(points, points[1:]):
            moving = any(abs(before['outval'][axis] - after['outval'][axis]) > 1. for axis in 'xyz')  # source cm
            if moving and str(before.get('interpmode', '')).startswith('cim_curve'):
                arrivals.update(range(math.ceil(before['inval'] * 1000) + 33, math.floor(after['inval'] * 1000), 33))
    for ms in sorted(arrivals - {k + CONFIG['start'] for k in present}):
        pos, rotation = base.world_pose(rows, ACTOR['group'], actor, ms / 1000., CONFIG['matinee'], CONFIG['data'])
        keys.append(dict(timeMs=ms - CONFIG['start'], positionOffset=pos.tolist(),
                         rotationQuaternion=Rotation.from_matrix(rotation).as_quat().tolist(),
                         scaleMultiplier=[ACTOR['source_scale']] * 3, visible=True))
    keys.sort(key=lambda k: k['timeMs'])
    for key in keys:  # the World Sequence validator requires a normalized quaternion with non-negative w (q == -q)
        if key['rotationQuaternion'][3] < 0:
            key['rotationQuaternion'] = [-x + 0. for x in key['rotationQuaternion']]
    assert 2 <= len(keys) <= 256 and all(a['timeMs'] < b['timeMs'] for a, b in zip(keys, keys[1:])), len(keys)


def add_actor_to_composition(document, world):
    duration = CONFIG['duration'] - CONFIG['start']
    matches = [p for p in document['patterns'] if p['displayName'] == CONFIG['name'] and p['gateId'] == CONFIG['gate']]
    assert len(matches) == 1, ('encore pattern', len(matches))
    pattern = matches[0]
    assert not pattern['worldOccurrences'] and pattern['nextWorldOccurrenceOrdinal'] == 1, 'actor already present'
    assert not any(w['sequenceInstanceId'] == world['sequenceInstanceId'] for w in document['worlds']), 'world present'
    # project_kouku_saydon_composition._validate_catalog only accepts kakulsaydon.g1.world.<N> below nextWorldOrdinal
    # (plus the gate2 intro source prefix), so this Boss/Sequence row uses the Workbench form, not world.<prefix>.<label>.
    row = dict(world)
    row['worldId'] = 'kakulsaydon.g1.world.' + str(document['nextWorldOrdinal'])
    assert not any(w['worldId'] == row['worldId'] for w in document['worlds']), row['worldId']
    document['worlds'].append(row)
    document['nextWorldOrdinal'] += 1
    pattern['worldOccurrences'].append(dict(occurrenceId=pattern['patternId'] + '.world.1', worldId=row['worldId'],
                                            startMs=0, durationMs=duration, playbackSpeed=1))
    pattern['nextWorldOccurrenceOrdinal'] = 2
    document['revision'] += 1
    return pattern['patternId'], row['worldId']


def actor_main(args):
    paths = dict(boss=PATHS['boss'], sequence=PATHS['sequence'], worlds=WORLDS_PATH)
    baseline = {k: p.read_bytes() for k, p in paths.items()}
    boss = json.loads(baseline['boss'].decode('utf-8-sig'))
    sequence = json.loads(baseline['sequence'].decode('utf-8-sig'))
    worlds = json.loads(baseline['worlds'].decode('utf-8-sig'))

    rows = scene_rows()
    obj, template, instance, world, wmodel_candidate = build_actor(rows)
    for key, identity, row in (('objectResources', 'objectId', obj), ('templates', 'sequenceId', template),
                               ('instances', 'instanceId', instance)):
        assert not any(r[identity] == row[identity] for r in worlds[key]), ('already present', row[identity])
    patterns = {}
    patterns['boss'] = add_actor_to_composition(boss, world)
    patterns['sequence'] = add_actor_to_composition(sequence, world)
    outputs = {
        paths['boss']: dump_doc(boss),
        paths['sequence']: dump_doc(sequence),
        paths['worlds']: append_compact(baseline['worlds'], dict(objectResources=[obj], templates=[template],
                                                                 instances=[instance])),
    }
    # the World Sequence text insertion must equal appending the rows to the parsed document
    expected = json.loads(baseline['worlds'].decode('utf-8-sig'))
    expected['revision'] += 1
    expected['objectResources'].append(obj)
    expected['templates'].append(template)
    expected['instances'].append(instance)
    assert json.loads(outputs[paths['worlds']].decode('utf-8')) == expected, 'World Sequence insertion mismatch'
    assert len(outputs[paths['worlds']]) < 16 * 1024 * 1024, 'World Sequence exceeds runtime byte bound'

    shutil.rmtree(OUT / 'candidate_actor', ignore_errors=True)
    for path, data in outputs.items():
        target = OUT / 'candidate_actor' / path.relative_to(ROOT)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    model = base.wm.read_wmodel(wmodel_candidate)
    clip_name = template['animationTracks'][0]['clipName']
    matches = [animation for animation in model.animations if animation.name == clip_name]
    assert len(matches) == 1, ('encore named clip', clip_name)
    clip = matches[0]
    report = dict(installed=args.install_actor, patterns=patterns, actorWModel=ACTOR_WMODEL,
                  wmodelBytes=wmodel_candidate.stat().st_size, wmodelSha256=sha(wmodel_candidate.read_bytes()),
                  textures=sorted(f.name for f in (wmodel_candidate.parent / 'textures').iterdir()),
                  clipName=clip.name, clipTicksPerSecond=clip.ticks_per_second, clipDurationTicks=clip.duration_ticks,
                  templateKeys=len(template['tracks'][0]['keys']), objectId=obj['objectId'],
                  instanceId=instance['instanceId'], templateId=template['sequenceId'],
                  baselineSha256={p.relative_to(ROOT).as_posix(): sha(baseline[k]) for k, p in paths.items()},
                  candidateSha256={p.relative_to(ROOT).as_posix(): sha(d) for p, d in outputs.items()},
                  manualVisualValidation='USER_PENDING')
    base.write(OUT / 'report_actor.json', report)

    if args.install_actor:
        for key, path in paths.items():
            if path.read_bytes() != baseline[key]:
                raise ValueError('Concurrent authoring changed: ' + str(path))
        destination = base.RESOURCES / ACTOR_WMODEL
        backup = OUT / 'backup_actor'
        backup.mkdir(parents=True, exist_ok=True)
        for key, path in paths.items():
            (backup / path.name).write_bytes(baseline[key])
        destination.parent.mkdir(parents=True, exist_ok=True)
        for file in sorted((wmodel_candidate.parent / 'textures').iterdir()):
            target = destination.parent / 'textures' / file.name
            if target.exists():
                assert sha(target.read_bytes()) == sha(file.read_bytes()), 'texture differs: ' + file.name
                continue
            target.parent.mkdir(exist_ok=True)
            temporary = target.with_name(target.name + '.encore.tmp')
            shutil.copyfile(file, temporary)
            assert sha(temporary.read_bytes()) == sha(file.read_bytes())
            os.replace(temporary, target)
        install_baked_clip(base.RESOURCES, ACTOR_DONOR, clip.name, donor_path=wmodel_candidate)
        report['installedWmodelSha256'] = sha(destination.read_bytes())
        for path, data in outputs.items():
            temporary = path.with_name(path.name + '.encore.tmp')
            temporary.write_bytes(data)
            os.replace(temporary, path)
        base.write(OUT / 'report_actor.json', report)
    print(json.dumps(report, ensure_ascii=False, indent=1), flush=True)


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--actor', action='store_true', help='candidate files for the boss actor pass only')
    parser.add_argument('--install-actor', action='store_true', help='install the boss actor pass')
    args = parser.parse_args()
    if args.actor or args.install_actor:
        return actor_main(args)

    baseline = {k: p.read_bytes() for k, p in PATHS.items()}
    leaf_path = ROOT / 'Data/Effects/V2/Authored' / (CONFIG['prefix'] + '.fade.black.effectv2.json')
    assert not leaf_path.exists(), 'fade leaf already exists'
    docs = {k: json.loads(baseline[k].decode('utf-8-sig')) for k in ('boss', 'sequence', 'independent')}
    camera_doc = json.loads(baseline['cameras'].decode('utf-8-sig'))

    rows = scene_rows()
    assert rows[44]['p']['interplength'] == 23.33333396911621 and len(rows[44]['p']['interpgroups']) == 23
    shots = base.make_cameras(rows, CONFIG['matinee'], CONFIG['data'], CONFIG['duration'], CONFIG['prefix'],
                              CONFIG['name'], CONFIG['start'])
    assert shots, 'no camera shot'
    shot_docs = [s for _, _, s in shots]
    existing_ids = {s['shotId'] for s in camera_doc['shots']}
    assert not existing_ids & {s['shotId'] for s in shot_docs}, 'camera shot id collision'
    assert len(camera_doc['shots']) + len(shot_docs) <= 128, 'camera shot bound'
    for shot in shot_docs:
        assert len(shot['cameraTrack']['keyframes']) <= 64, shot['shotId']
    leaf = bss.fade(CONFIG, rows)

    patterns = {key: add_to_composition(docs[key], shots, leaf) for key in ('sequence', 'boss')}
    outputs = {
        PATHS['boss']: dump_doc(docs['boss']),
        PATHS['sequence']: dump_doc(docs['sequence']),
        PATHS['cameras']: insert_shots(baseline['cameras'], shot_docs),
    }
    if leaf:
        if leaf['effectId'] not in docs['independent']['effects']:
            docs['independent']['effects'].append(leaf['effectId'])
        outputs[PATHS['independent']] = dump_doc(docs['independent'])
        outputs[leaf_path] = dump_doc(leaf)
    assert len(outputs[PATHS['cameras']]) < 2 * 1024 * 1024, 'Camera exceeds runtime byte bound'

    shutil.rmtree(OUT / 'candidate', ignore_errors=True)
    for path, data in outputs.items():
        target = OUT / 'candidate' / path.relative_to(ROOT)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    report = dict(installed=args.install, patterns=patterns,
                  cameraShots=[dict(shotId=s['shotId'], startMs=b, endMs=e, keys=len(s['cameraTrack']['keyframes']))
                               for b, e, s in shots],
                  fade=(dict(effectId=leaf['effectId'], keys=len(leaf['params']['screenPost']['intensityKeys']),
                             lifetimeSeconds=leaf['params']['lifetime']) if leaf else None),
                  baselineSha256={p.relative_to(ROOT).as_posix(): sha(baseline[k]) for k, p in PATHS.items()},
                  candidateSha256={p.relative_to(ROOT).as_posix(): sha(d) for p, d in outputs.items()},
                  manualVisualValidation='USER_PENDING')
    base.write(OUT / 'report.json', report)

    if args.install:
        for key, path in PATHS.items():
            if path.read_bytes() != baseline[key]:
                raise ValueError('Concurrent authoring changed: ' + str(path))
        backup = OUT / 'backup'
        backup.mkdir(parents=True, exist_ok=True)
        for key, path in PATHS.items():
            (backup / path.name).write_bytes(baseline[key])
        for path, data in outputs.items():
            temporary = path.with_name(path.name + '.encore.tmp')
            temporary.write_bytes(data)
            os.replace(temporary, path)
    print(json.dumps(report, ensure_ascii=False, indent=1), flush=True)


if __name__ == '__main__':
    main()
