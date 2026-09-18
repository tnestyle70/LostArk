"""Build the G12 cutscene targets (2026-09-13) from source Matinees.

Targets:
  P3 KAKULSAYDON_G1_PATTERN_3  2관문_진입      camera shots regenerated from SCENE04A Matinee2
  P7 KAKULSAYDON_G1_PATTERN_7  3관문_진입      full SCENE02A Matinee10 window (0..35368ms)
  P8 KAKULSAYDON_G1_PATTERN_8  빙고_최종엔딩씬  new pattern from SCENE01B Matinee0 (0..49083ms)

Cameras and source props use the same generator functions as build_source_sequences
(make_cameras/static_worlds). P7 actors reuse the 2관문_클리어 baked worlds, which
were sampled from the same Matinee at the same 0ms origin. Rows that belong to
other owners (fade, effects, lights, scene profiles, world definitions) are moved
in time where the plan says so and never edited or deleted.

G13 (2026-09-13) world-document modes:
  --backdrops  installs the 165 SCENE04A set pieces hanging on the dummy camera
               actors (build_gate2_intro_backdrops) into P3.
  --p7-scale   gives P7 its own copies of the shared 2관문_클리어 actors with the
               source DrawScale track applied.
Both write the world document in the exact CWorldSequenceDocument::Save format
(the bytes the World Object Tool itself produces on Save); every existing value is
kept at float32 precision and verified after re-parse. The python indent=2 layout
of the same content no longer fits the 16MiB reader bound once the set pieces exist.

Without --install the candidate documents are written under OUT only.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
import math
import re
import shutil
import struct
from pathlib import Path

import build_source_sequences as bss
import build_gate2_intro_composition as base
import build_gate2_intro_backdrops as bd

ROOT = base.ROOT
OUT = ROOT / 'out/KoukuGateCutscenes20260913'
PHYSICAL = {  # logical scene -> obfuscated package in the installed game
    'SCENE04A': 'B9AVB2VAZIQRPQCJVKAVYRAVOKYPY806.upk',
    'SCENE02A': 'B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8M6.upk',
    'SCENE01B': 'B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8FD.upk',
}
P7_TIME_SHIFT_MS = 16710  # former source window start; existing P7 boxes keep their source clock
P7_ACTOR_DONORS = ['kakulsaydon.g1.world.28', 'kakulsaydon.g1.world.29', 'kakulsaydon.g1.world.30']
CONFIGS = [
    dict(id=3, name='2관문_진입', prefix='kouku.gate2.intro', scene='SCENE04A', matinee=329, data=394,
         duration=27000, start=0, gate='GATE2', combat=True, mode='cameras'),
    dict(id=7, name='3관문_진입', prefix='kouku.gate3.intro', scene='SCENE02A', matinee=63, data=117,
         duration=35368, start=0, gate='GATE3', combat=True, mode='full-window'),
    dict(id=8, name='빙고_최종엔딩씬', prefix='kouku.bingo.ending', scene='SCENE01B', matinee=32, data=45,
         duration=49083, start=0, gate='BINGO', combat=False, mode='new'),
]
GATE_BOSS = dict(GATE1='g1.saydon', GATE2='g2.kouku', GATE3='g3.saydon', BINGO='bingo.saydon')
WORLD_DOCUMENT_BYTES = 16 * 1024 * 1024  # WorldSequenceDocument.cpp MAX_DOCUMENT_BYTES
STABLE_ID = re.compile(r'^[A-Za-z0-9._-]{1,128}$')
DEFAULT_MOTION = dict(velocity=[0, 0, 0], acceleration=[0, 0, 0], angularVelocityDegrees=[0, 0, 0],
                      revolutionDegreesPerSecond=[0, 0, 0], revolutionOffset=[0, 0, 0], count=1, intervalMs=0, spreadDegrees=0, seed=0)
P7_SCALED_ACTORS = [  # SCENE02A actors whose active DrawScale float track the P5 bake did not apply
    dict(group=152, actor=69, donor='kakulsaydon.g1.world.28', label='LargeSaydon', tag='largesaydon'),
    dict(group=135, actor=70, donor='kakulsaydon.g1.world.29', label='Kouku', tag='kouku'),
]


def scene_rows(config):
    cache = bss.OUT / ('LV_LUT_MIDNIGHTC_ED_' + config['scene'] + '.json')
    if not cache.exists():
        rows, imports = base.extract_scene(base.PACKAGES / PHYSICAL[config['scene']])
        base.write(cache, dict(rows=rows, imports=imports))
    return bss.scene_rows(config)


def replace_cameras(docs, pattern, config, rows):
    """Replace this pattern's camera shots/resources/occurrences; keep every other box."""
    shots = base.make_cameras(rows, config['matinee'], config['data'], config['duration'],
                              config['prefix'], config['name'], config['start'])
    shot_prefix = config['prefix'] + '.camera.'
    resource_prefix = 'presentation.' + shot_prefix
    docs['cameras']['shots'] = [s for s in docs['cameras']['shots'] if not s['shotId'].startswith(shot_prefix)]
    docs['cameras']['shots'] += [s for _, _, s in shots]
    docs['composition']['presentationResources'] = [
        r for r in docs['composition']['presentationResources'] if not r['resourceId'].startswith(resource_prefix)]
    kept = [o for o in pattern['presentationOccurrences'] if not o['resourceId'].startswith(resource_prefix)]
    ordinal = pattern.get('nextPresentationOccurrenceOrdinal', len(pattern['presentationOccurrences']) + 1)
    added = []
    for start, end, shot in shots:
        r = bss.resource('presentation.' + shot['shotId'], shot['displayName'], 'CAMERA', shot['shotId'], end - start)
        docs['composition']['presentationResources'].append(r)
        added.append(bss.occurrence(pattern['patternId'], ordinal, r['resourceId'], start, end))
        ordinal += 1
    pattern['presentationOccurrences'] = added + kept
    pattern['nextPresentationOccurrenceOrdinal'] = ordinal
    return shots


def new_pattern(config):
    pid = 'KAKULSAYDON_G1_PATTERN_' + str(config['id'])
    return dict(patternId=pid, actorProfileId='MN_RPCT_05' if config['gate'] != 'GATE2' else 'MN_RPCZ_00',
                gateId=config['gate'],
                targetBossPlacementId='boss.kakulsaydon.' + GATE_BOSS[config['gate']],
                displayName=config['name'], authoringStatus='DRAFT', category='MECHANIC', enterCombatOnFinish=config['combat'],
                nextStageOrdinal=2, nextAnimationOrdinal=1, nextLogicOccurrenceOrdinal=1, nextSummonOccurrenceOrdinal=1,
                nextWorldOccurrenceOrdinal=1, nextSceneProfileOccurrenceOrdinal=1, nextPresentationOccurrenceOrdinal=1,
                stages=[dict(stageId='STAGE_1', actionId=pid + '.stage.1', stageKind='ACTIVE',
                             durationMs=config['duration'] - config['start'], animationOccurrences=[])],
                logicOccurrences=[], summonOccurrences=[], worldOccurrences=[], sceneProfileOccurrences=[],
                resetBossToSpawn=False, presentationOccurrences=[])


# --- CWorldSequenceDocument::Save document format (setprecision(9), inline arrays) ----

def f9(value):
    """The tool prints its in-memory float32 with 9 significant digits; the runtime
    reads every number as float32, so print the float32 the reader will hold."""
    if isinstance(value, bool):
        return 'true' if value else 'false'
    if isinstance(value, int):
        return str(value)
    return '%.9g' % f32(value)


def esc(text):
    return json.dumps(text, ensure_ascii=False)[1:-1]


def vec(values):
    return '[' + ', '.join(f9(v) for v in values) + ']'


def tool_object_resource(o):
    text = ('    {\n      "objectId": "%s",\n      "displayName": "%s",\n      "modelAssetId": "%s",\n      "anchorKind": "%s",\n'
            '      "diffuseTextureAssetId": "%s",\n      "modelPreScale": %s,\n      "animated": %s,\n      "scale": %s,\n'
            '      "sequenceInstanceId": "%s"') % (
        esc(o['objectId']), esc(o['displayName']), esc(o['modelAssetId']), esc(o['anchorKind']), esc(o['diffuseTextureAssetId']),
        f9(o['modelPreScale']), f9(o['animated']), vec(o['scale']), esc(o['sequenceInstanceId']))
    if o['anchorKind'] == 'BOSS':
        text += ',\n      "anchorBossArchetypeId": "%s",\n      "anchorBone": "%s"' % (esc(o['anchorBossArchetypeId']), esc(o['anchorBone']))
    if o.get('defaultMotionInstanceId'):
        text += ',\n      "defaultMotionInstanceId": "%s"' % esc(o['defaultMotionInstanceId'])
    if o.get('motionInstanceIds'):
        text += ',\n      "motionInstanceIds": [' + ', '.join('"%s"' % esc(x) for x in o['motionInstanceIds']) + ']'
    if o.get('materialSourceModelAssetId'):
        text += ',\n      "materialSourceModelAssetId": "%s"' % esc(o['materialSourceModelAssetId'])
    if o.get('mapMaterialBindings'):
        text += ',\n      "mapMaterialBindings": ['
        for n, b in enumerate(o['mapMaterialBindings']):
            text += (',' if n else '') + '\n        {"materialName": "%s", "sourceAssetId": "%s", "sourceMaterialName": "%s"' % (
                esc(b['materialName']), esc(b['sourceAssetId']), esc(b['sourceMaterialName']))
            if b.get('diffuseTextureAssetId'):
                text += ', "diffuseTextureAssetId": "%s"' % esc(b['diffuseTextureAssetId'])
            text += '}'
        text += '\n      ]'
    if o.get('materialProfile'):
        p = o['materialProfile']
        text += ',\n      "materialProfile": {\n        "materialName": "%s",\n        "sourceMaterial": "%s",\n        "family": "%s",\n        "parameters": {' % (
            esc(p['materialName']), esc(p['sourceMaterial']), esc(p['family']))
        first = True
        for name, value in p['parameters'].items():
            text += ('' if first else ',') + '\n          "%s": [%s, %s, %s, %s]' % (esc(name), f9(value[0]), f9(value[1]), f9(value[2]), f9(value[3]))
            first = False
        text += '\n        },\n        "textures": ['
        for n, t in enumerate(p['textures']):
            text += (',' if n else '') + '\n          {"expressionIndex": %s, "assetId": "%s", "colorSpace": "%s"}' % (
                f9(t['expressionIndex']), esc(t['assetId']), esc(t['colorSpace']))
        text += '\n        ]\n      }'
    return text + '\n    }'


def tool_template(t):
    m = dict(DEFAULT_MOTION, **t.get('objectMotion', {}))
    text = ('    {\n      "sequenceId": "%s",\n      "displayName": "%s",\n      "category": "%s",\n      "durationMs": %s,\n'
            '      "interpolation": "%s",\n      "objectMotion": {\n') % (
        esc(t['sequenceId']), esc(t['displayName']), esc(t['category']), f9(t['durationMs']), esc(t['interpolation']))
    for name in ('velocity', 'acceleration', 'angularVelocityDegrees', 'revolutionDegreesPerSecond', 'revolutionOffset'):
        text += '        "%s": %s,\n' % (name, vec(m[name]))
    extents = m.get('spawnHalfExtents')
    if extents and any(x != 0 for x in extents):
        text += '        "spawnHalfExtents": %s,\n' % vec(extents)
    text += '        "count": %s, "intervalMs": %s, "spreadDegrees": %s, "seed": %s' % (
        f9(m['count']), f9(m['intervalMs']), f9(m['spreadDegrees']), f9(m['seed']))
    if m.get('emissions'):
        text += ',\n        "emissions": ['
        for n, e in enumerate(m['emissions']):
            text += ('\n' if n == 0 else ',\n') + '          {"positionOffset": %s, "yawDegrees": %s, "startDelayMs": %s}' % (
                vec(e['positionOffset']), f9(e['yawDegrees']), f9(e['startDelayMs']))
        text += '\n        ]'
    text += '\n      },\n      "tracks": ['
    for n, track in enumerate(t['tracks']):
        text += ('\n' if n == 0 else ',\n') + '        {\n          "slotId": "%s",\n          "keys": [' % esc(track['slotId'])
        for k, key in enumerate(track['keys']):
            text += ('\n' if k == 0 else ',\n') + (
                '            {\n              "timeMs": %s,\n              "positionOffset": %s,\n'
                '              "rotationQuaternion": %s,\n              "scaleMultiplier": %s,\n              "visible": %s\n            }') % (
                f9(key['timeMs']), vec(key['positionOffset']), vec(key['rotationQuaternion']), vec(key['scaleMultiplier']), f9(key['visible']))
        text += (']\n' if not track['keys'] else '\n          ]\n') + '        }'
    text += ('],\n' if not t['tracks'] else '\n      ],\n') + '      "animationTracks": ['
    for n, track in enumerate(t.get('animationTracks', [])):
        text += ('\n' if n == 0 else ',\n') + '        { "slotId": "%s", "clipName": "%s"' % (esc(track['slotId']), esc(track['clipName']))
        if track.get('displayName'):
            text += ', "displayName": "%s"' % esc(track['displayName'])
        text += ', "startMs": %s, "playbackRate": %s, "loop": %s, "holdLastFrame": %s }' % (
            f9(track.get('startMs', 0)), f9(track['playbackRate']), f9(track['loop']), f9(track['holdLastFrame']))
    text += ']' if not t.get('animationTracks') else '\n      ]'
    if t.get('effectTracks'):
        text += ',\n      "effectTracks": ['
        for n, e in enumerate(t['effectTracks']):
            text += (',\n' if n else '\n') + ('        { "effectTrackId": "%s", "slotId": "%s", "resourceKind": "%s", "resourceId": "%s", "timing": "%s", '
                                               '"startMs": %s, "durationMs": %s, "positionOffset": %s, "rotationDegrees": %s, "scale": %s }') % (
                esc(e['effectTrackId']), esc(e['slotId']), esc(e['resourceKind']), esc(e['resourceId']), esc(e['timing']),
                f9(e['startMs']), f9(e['durationMs']), vec(e['positionOffset']), vec(e['rotationDegrees']), vec(e['scale']))
        text += '\n      ]'
    return text + '\n    }'


def tool_instance(i):
    text = ('    {\n      "instanceId": "%s",\n      "templateId": "%s",\n      "enabled": %s,\n      "startDelayMs": %s,\n'
            '      "playbackSpeed": %s,\n      "anchorKind": "%s",\n      "position": %s,\n      "motionEnd": "%s",\n'
            '      "nextMotionId": "%s",\n') % (
        esc(i['instanceId']), esc(i['templateId']), f9(i['enabled']), f9(i['startDelayMs']), f9(i['playbackSpeed']),
        esc(i['anchorKind']), vec(i['position']), esc(i['motionEnd']), esc(i['nextMotionId']))
    if i.get('walkableSurface'):
        text += '      "walkableSurface": { "radiusM": %s, "localHeightM": %s },\n' % (
            f9(i['walkableSurface']['radiusM']), f9(i['walkableSurface']['localHeightM']))
    text += '      "bindings": ['
    for n, b in enumerate(i['bindings']):
        text += ('\n' if n == 0 else ',\n') + '        { "slotId": "%s", "targetKind": "%s", "targetId": "%s" }' % (
            esc(b['slotId']), esc(b['targetKind']), esc(b['targetId']))
    return text + (']\n' if not i['bindings'] else '\n      ]\n') + '    }'


def tool_document(d):
    text = '{\n  "schema": "%s",\n  "formatVersion": %s,\n  "areaId": "%s",\n  "revision": %s,\n  "objectResources": [' % (
        esc(d['schema']), f9(d['formatVersion']), esc(d['areaId']), f9(d['revision']))
    text += ''.join(('\n' if n == 0 else ',\n') + tool_object_resource(o) for n, o in enumerate(d['objectResources']))
    text += ('],\n' if not d['objectResources'] else '\n  ],\n') + '  "templates": ['
    text += ''.join(('\n' if n == 0 else ',\n') + tool_template(t) for n, t in enumerate(d['templates']))
    text += ('],\n' if not d['templates'] else '\n  ],\n') + '  "instances": ['
    text += ''.join(('\n' if n == 0 else ',\n') + tool_instance(i) for n, i in enumerate(d['instances']))
    return text + (']\n' if not d['instances'] else '\n  ]\n') + '}\n'


def normalize_document(doc):
    """What the tool's Save/Load round trip keeps: omitted empty optionals, default motion."""
    doc = copy.deepcopy(doc)
    for o in doc['objectResources']:
        for k in ('defaultMotionInstanceId', 'materialSourceModelAssetId', 'motionInstanceIds', 'mapMaterialBindings', 'materialProfile'):
            if k in o and not o[k]:
                del o[k]
        if o['anchorKind'] != 'BOSS':
            o.pop('anchorBossArchetypeId', None)
            o.pop('anchorBone', None)
        for b in o.get('mapMaterialBindings', []):
            if 'diffuseTextureAssetId' in b and not b['diffuseTextureAssetId']:
                del b['diffuseTextureAssetId']
    for t in doc['templates']:
        m = dict(DEFAULT_MOTION, **t.get('objectMotion', {}))
        if not m.get('spawnHalfExtents') or not any(x != 0 for x in m['spawnHalfExtents']):
            m.pop('spawnHalfExtents', None)
        if not m.get('emissions'):
            m.pop('emissions', None)
        t['objectMotion'] = m
        for a in t.get('animationTracks', []):
            a.setdefault('startMs', 0)
            if 'displayName' in a and not a['displayName']:
                del a['displayName']
        if 'effectTracks' in t and not t['effectTracks']:
            del t['effectTracks']
    for i in doc['instances']:
        if 'walkableSurface' in i and not i['walkableSurface']:
            del i['walkableSurface']
    return doc


def f32(value):
    return struct.unpack('f', struct.pack('f', value))[0]


def same_f32(a, b, path=''):
    """First difference between two parsed documents at float32 precision, or None."""
    if isinstance(a, dict) and isinstance(b, dict):
        if set(a) != set(b):
            return path + ' keys ' + str(sorted(set(a) ^ set(b)))
        for k in a:
            r = same_f32(a[k], b[k], path + '/' + k)
            if r:
                return r
        return None
    if isinstance(a, list) and isinstance(b, list):
        if len(a) != len(b):
            return path + ' length'
        for n, (x, y) in enumerate(zip(a, b)):
            r = same_f32(x, y, path + '[%d]' % n)
            if r:
                return r
        return None
    if isinstance(a, bool) or isinstance(b, bool) or isinstance(a, str) or isinstance(b, str):
        return None if a == b else path + ' value'
    return None if f32(a) == f32(b) else path + ' number %r %r' % (a, b)


def world_document_bytes(baseline_bytes, doc):
    """Serialize in the tool's format, keeping the file's line-ending/BOM convention, and prove equivalence."""
    text = tool_document(doc)
    if b'\r\n' in baseline_bytes[:4096]:
        text = text.replace('\n', '\r\n')
    data = (b'\xef\xbb\xbf' if baseline_bytes.startswith(b'\xef\xbb\xbf') else b'') + text.encode('utf-8')
    assert len(data) <= WORLD_DOCUMENT_BYTES, len(data)
    difference = same_f32(json.loads(text), normalize_document(doc))
    assert difference is None, difference
    return data


def validate_world_rows(result, doc):
    """Mirror the reader/Validate rules of WorldSequenceDocument.cpp for the new rows."""
    objects = {o['objectId'] for o in doc['objectResources']}
    templates = {t['sequenceId'] for t in doc['templates']}
    instances = {i['instanceId'] for i in doc['instances']}
    new_objects = set()
    for o in result['resources']:
        assert STABLE_ID.match(o['objectId']) and o['objectId'] not in objects and o['objectId'] not in new_objects, o['objectId']
        assert 0 < len(o['displayName']) <= 128 and o['anchorKind'] == 'WORLD'
        assert (base.RESOURCES / o['modelAssetId']).is_file(), o['modelAssetId']
        assert math.isfinite(o['modelPreScale']) and 1e-6 <= o['modelPreScale'] <= 1e5
        new_objects.add(o['objectId'])
    new_templates = {}
    for t in result['templates']:
        assert STABLE_ID.match(t['sequenceId']) and t['sequenceId'] not in templates and t['sequenceId'] not in new_templates, t['sequenceId']
        assert 0 < len(t['displayName']) <= 128 and 1 <= len(t['tracks']) <= 32 and 0 < t['durationMs'] <= 600000
        slots = set()
        for track in t['tracks']:
            assert STABLE_ID.match(track['slotId']) and track['slotId'] not in slots
            slots.add(track['slotId'])
            keys = track['keys']
            assert 2 <= len(keys) <= 256 and keys[0]['timeMs'] == 0 and keys[-1]['timeMs'] == t['durationMs'], t['sequenceId']
            previous = -1
            for key in keys:
                assert isinstance(key['timeMs'], int) and key['timeMs'] > previous
                previous = key['timeMs']
                assert all(math.isfinite(x) and abs(x) <= 1e5 for x in key['positionOffset'] + key['scaleMultiplier'])
                assert all(x >= 1e-6 for x in key['scaleMultiplier']), (t['sequenceId'], key['scaleMultiplier'])
                q = key['rotationQuaternion']
                assert abs(math.sqrt(sum(x * x for x in q)) - 1) <= 5e-4 and q[3] >= 0
        new_templates[t['sequenceId']] = slots
    for i in result['instances']:
        assert STABLE_ID.match(i['instanceId']) and i['instanceId'] not in instances, i['instanceId']
        slots = new_templates[i['templateId']]
        assert len(i['bindings']) == len(slots) and {b['slotId'] for b in i['bindings']} == slots
        targets = [b['targetId'] for b in i['bindings']]
        assert len(set(targets)) == len(targets), i['instanceId']  # one resource per instance binding
        assert all(b['targetKind'] == 'OBJECT_RESOURCE' and b['targetId'] in (new_objects | objects) for b in i['bindings'])
        assert i['anchorKind'] == 'WORLD' and i['motionEnd'] == 'STOP' and 0.05 <= i['playbackSpeed'] <= 8 and i['enabled']
    assert len(doc['templates']) + len(result['templates']) <= 512
    assert len(doc['instances']) + len(result['instances']) <= 2048
    assert len(doc['objectResources']) + len(result['resources']) <= 2048


def write_candidate(tag, paths, world_bytes, composition, report):
    out = OUT / tag
    if out.exists():
        shutil.rmtree(out)
    target = out / 'candidate' / paths['worlds'].relative_to(ROOT)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(world_bytes)
    base.write(out / 'candidate' / paths['composition'].relative_to(ROOT), composition)
    base.write(out / 'report.json', report)


def install_documents(paths, baseline, world_bytes, composition):
    for key, path in paths.items():
        if path.read_bytes() != baseline[key]:
            raise ValueError('Concurrent authoring changed: ' + str(path))
    paths['worlds'].write_bytes(world_bytes)
    base.write(paths['composition'], composition)


def load_world_inputs():
    paths = dict(composition=ROOT / 'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json',
                 worlds=bss.AREA / (base.AREA + '.worldsequences.json'))
    baseline = {k: p.read_bytes() for k, p in paths.items()}
    world_doc = json.loads(baseline['worlds'].decode('utf-8-sig'))
    composition = json.loads(baseline['composition'].decode('utf-8-sig'))
    return paths, baseline, world_doc, composition


def install_backdrops(install):
    config = CONFIGS[0]
    pid = 'KAKULSAYDON_G1_PATTERN_' + str(config['id'])
    paths, baseline, world_doc, composition = load_world_inputs()
    pattern = next(p for p in composition['patterns'] if p['patternId'] == pid)
    assert pattern['stages'][0]['durationMs'] == config['duration']
    cache = base.read(bss.OUT / ('LV_LUT_MIDNIGHTC_ED_' + config['scene'] + '.json'))
    rows = {int(k): v for k, v in cache['rows'].items()}
    result = bd.build_backdrops(rows, cache['imports'], base.parent_pose_sampler(rows), exclude_bone_attached=True)
    assert all(window == (0, config['duration']) for window in result['windows']), result['windows']
    placed = {o['worldId'] for o in pattern['worldOccurrences']}
    already = [w['worldId'] for w in result['worlds'] if w['worldId'] in placed]
    if already:
        assert len(already) == len(result['worlds']), 'partial backdrop install: ' + str(already)
        print(json.dumps(dict(installed=False, backdrops='already placed', worlds=len(already)), ensure_ascii=False), flush=True)
        return
    validate_world_rows(result, world_doc)
    registry = {w['worldId'] for w in composition['worlds']}
    assert not any(w['worldId'] in registry for w in result['worlds'])
    composition['worlds'] += result['worlds']
    ordinal = pattern['nextWorldOccurrenceOrdinal']
    for world, (start, end) in zip(result['worlds'], result['windows']):
        pattern['worldOccurrences'].append(dict(occurrenceId=f'{pid}.world.{ordinal}', worldId=world['worldId'],
                                                startMs=start, durationMs=end - start, playbackSpeed=1))
        ordinal += 1
    pattern['nextWorldOccurrenceOrdinal'] = ordinal
    composition['revision'] += 1
    merged = copy.deepcopy(world_doc)
    merged['revision'] += 1
    merged['objectResources'] += result['resources']
    merged['templates'] += result['templates']
    merged['instances'] += result['instances']
    world_bytes = world_document_bytes(baseline['worlds'], merged)
    keys = sum(len(track['keys']) for t in result['templates'] for track in t['tracks'])
    report = dict(installed=install, patternId=pid, worldOccurrences=len(result['worlds']), objectResources=len(result['resources']),
                  templates=len(result['templates']), instances=len(result['instances']), keys=keys,
                  worldBytesBefore=len(baseline['worlds']), worldBytesAfter=len(world_bytes), worldByteBound=WORLD_DOCUMENT_BYTES,
                  worldRevision=merged['revision'], compositionRevision=composition['revision'],
                  baseline={k: hashlib.sha256(v).hexdigest() for k, v in baseline.items()}, receipt=result['receipt'],
                  manualVisualValidation='USER_PENDING')
    write_candidate('backdrops', paths, world_bytes, composition, report)
    if install:
        install_documents(paths, baseline, world_bytes, composition)
    print(json.dumps({k: v for k, v in report.items() if k not in ('receipt', 'baseline')}, ensure_ascii=False), flush=True)


def sample_keys(keys, ms):
    """Linear position / slerp rotation between two transform keys, as the LINEAR runtime does."""
    from scipy.spatial.transform import Rotation, Slerp
    times = [k['timeMs'] for k in keys]
    if ms in times:
        key = dict(keys[times.index(ms)])
        # The reader flips w<0 quaternions on load; store that canonical form so the
        # candidate matches what a tool Save would write.
        if key['rotationQuaternion'][3] < 0:
            key['rotationQuaternion'] = [-x for x in key['rotationQuaternion']]
        return key
    i = max(n for n, t in enumerate(times) if t < ms)
    a, b = keys[i], keys[i + 1]
    u = (ms - a['timeMs']) / (b['timeMs'] - a['timeMs'])
    position = [x * (1 - u) + y * u for x, y in zip(a['positionOffset'], b['positionOffset'])]
    q = Slerp([0, 1], Rotation.from_quat([a['rotationQuaternion'], b['rotationQuaternion']]))([u])[0].as_quat()
    q = (-q if q[3] < 0 else q).tolist()
    return dict(timeMs=ms, positionOffset=position, rotationQuaternion=q, scaleMultiplier=list(a['scaleMultiplier']), visible=a['visible'])


def install_p7_scale(install):
    """P7-only copies of the shared 2관문_클리어 actors with the source DrawScale track applied.

    SCENE02A keys the actors' DrawScale (LargeSaydon 6.0->2.0 at 7.66-8.26s, Kouku
    1.0->0.3 at 7.76-8.26s) while they fly into the tunnel. The P5 bake left every
    scaleMultiplier at 1, so the flight shots framed actors three times too large.
    The shared P5 rows stay untouched; P7 references the copies.
    """
    config = CONFIGS[1]
    pid = 'KAKULSAYDON_G1_PATTERN_' + str(config['id'])
    paths, baseline, world_doc, composition = load_world_inputs()
    pattern = next(p for p in composition['patterns'] if p['patternId'] == pid)
    assert pattern['stages'][0]['durationMs'] == config['duration']
    rows = scene_rows(config)
    registry = {w['worldId']: w for w in composition['worlds']}
    instances = {i['instanceId']: i for i in world_doc['instances']}
    templates = {t['sequenceId']: t for t in world_doc['templates']}
    result = dict(resources=[], templates=[], instances=[], worlds=[])
    summary = []
    for spec in P7_SCALED_ACTORS:
        instance_id = f"world.sequence.instance.{config['prefix']}.{spec['tag']}"
        if any(registry[o['worldId']]['sequenceInstanceId'] == instance_id for o in pattern['worldOccurrences']):
            print(json.dumps(dict(installed=False, p7Scale='already placed', instanceId=instance_id), ensure_ascii=False), flush=True)
            return
        # The composition reader accepts only generated ids (kakulsaydon.g1.world.<ordinal>
        # below nextWorldOrdinal) or the gate2 source-import pair for World definitions.
        world_id = f"kakulsaydon.g1.world.{composition['nextWorldOrdinal']}"
        composition['nextWorldOrdinal'] += 1
        donor_world = registry[spec['donor']]
        donor_instance = instances[donor_world['sequenceInstanceId']]
        donor_template = templates[donor_instance['templateId']]
        assert len(donor_template['tracks']) == 1 and donor_template['durationMs'] == config['duration']
        assert not donor_template.get('effectTracks')
        tracks = [x['p'] for x in base.active_tracks(rows, spec['group'])
                  if x['cls'] == 'interptrackfloatprop' and x['p'].get('propertyname', '').lower() == 'drawscale']
        assert len(tracks) == 1, (spec['label'], len(tracks))
        points = tracks[0]['floattrack']['points']
        rest = rows[spec['actor']]['p'].get('drawscale', 1.)
        donor_keys = donor_template['tracks'][0]['keys']
        times = sorted({k['timeMs'] for k in donor_keys} |
                       {round(pt['inval'] * 1000) for pt in points if 0 <= round(pt['inval'] * 1000) <= config['duration']})
        assert len(times) <= 256, len(times)
        keys = []
        for ms in times:
            key = sample_keys(donor_keys, ms)
            factor = float(base.curve(points, ms / 1000., rest)) / rest
            assert math.isfinite(factor) and factor > 1e-6
            key['scaleMultiplier'] = [round(s * factor, 9) for s in key['scaleMultiplier']]
            keys.append(key)
        template = dict(sequenceId=f"sequence.{config['prefix']}.{spec['tag']}", displayName=f"{config['name']} / {spec['label']}",
                        category=donor_template['category'], durationMs=donor_template['durationMs'],
                        interpolation=donor_template['interpolation'], objectMotion=copy.deepcopy(donor_template.get('objectMotion', {})),
                        tracks=[dict(slotId=donor_template['tracks'][0]['slotId'], keys=keys)],
                        animationTracks=[dict(a) for a in donor_template['animationTracks']])
        instance = dict(donor_instance, instanceId=instance_id, templateId=template['sequenceId'],
                        bindings=[dict(b) for b in donor_instance['bindings']])
        world = dict(donor_world, worldId=world_id, displayName=template['displayName'], sequenceInstanceId=instance['instanceId'])
        result['templates'].append(template)
        result['instances'].append(instance)
        result['worlds'].append(world)
        summary.append(dict(label=spec['label'], donor=spec['donor'], worldId=world_id, restDrawScale=rest,
                            drawScaleKeys=[(round(pt['inval'], 3), pt['outval']) for pt in points], keys=len(keys),
                            scaleAtEnd=keys[-1]['scaleMultiplier']))
    validate_world_rows(result, world_doc)
    assert not any(w['worldId'] in registry for w in result['worlds'])
    for occurrence in pattern['worldOccurrences']:
        for spec, world in zip(P7_SCALED_ACTORS, result['worlds']):
            if occurrence['worldId'] == spec['donor']:
                occurrence['worldId'] = world['worldId']
    assert {o['worldId'] for o in pattern['worldOccurrences']} >= {w['worldId'] for w in result['worlds']}
    composition['worlds'] += result['worlds']
    composition['revision'] += 1
    merged = copy.deepcopy(world_doc)
    merged['revision'] += 1
    merged['templates'] += result['templates']
    merged['instances'] += result['instances']
    world_bytes = world_document_bytes(baseline['worlds'], merged)
    report = dict(installed=install, patternId=pid, actors=summary, worldBytesBefore=len(baseline['worlds']), worldBytesAfter=len(world_bytes),
                  worldRevision=merged['revision'], compositionRevision=composition['revision'],
                  baseline={k: hashlib.sha256(v).hexdigest() for k, v in baseline.items()}, manualVisualValidation='USER_PENDING')
    write_candidate('p7scale', paths, world_bytes, composition, report)
    if install:
        install_documents(paths, baseline, world_bytes, composition)
    print(json.dumps({k: v for k, v in report.items() if k != 'baseline'}, ensure_ascii=False), flush=True)


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--backdrops', action='store_true', help='install the SCENE04A set pieces into P3 instead of the camera targets')
    parser.add_argument('--p7-scale', action='store_true', help='give P7 its own actor copies with the source DrawScale track applied')
    args = parser.parse_args()
    if args.backdrops:
        install_backdrops(args.install)
        return
    if args.p7_scale:
        install_p7_scale(args.install)
        return
    paths = dict(composition=ROOT / 'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json',
                 cameras=bss.AREA / (base.AREA + '.camerashots.json'),
                 worlds=bss.AREA / (base.AREA + '.worldsequences.json'))
    baseline = {k: p.read_bytes() for k, p in paths.items()}
    docs = {k: json.loads(v.decode('utf-8-sig')) for k, v in baseline.items()}
    composition = docs['composition']
    patterns = {p['patternId']: p for p in composition['patterns']}
    world_ids = {w['worldId'] for w in composition['worlds']}
    summaries = []
    for config in CONFIGS:
        rows = scene_rows(config)
        pid = 'KAKULSAYDON_G1_PATTERN_' + str(config['id'])
        duration = config['duration'] - config['start']
        summary = dict(patternId=pid, name=config['name'], sourceScene=config['scene'], sourceMatinee=config['matinee'],
                       durationMs=duration, mode=config['mode'])
        if config['mode'] == 'new' and pid not in patterns:
            assert composition['nextPatternOrdinal'] == config['id'], ('nextPatternOrdinal', composition['nextPatternOrdinal'])
            pattern = new_pattern(config)
            composition['patterns'].append(pattern)
            patterns[pid] = pattern
            composition['nextPatternOrdinal'] += 1
        else:
            pattern = patterns[pid]
        if config['mode'] in ('cameras', 'new'):
            pattern['displayName'] = config['name']
            assert pattern['stages'][0]['durationMs'] == duration
        if config['mode'] == 'new':
            # The tool lists patterns by gate; the bingo ending belongs to the BINGO gate.
            pattern['gateId'] = config['gate']
            pattern['targetBossPlacementId'] = 'boss.kakulsaydon.' + GATE_BOSS[config['gate']]
            pattern['enterCombatOnFinish'] = config['combat']
        # A rerun finds the window already widened; only the first run moves boxes.
        if config['mode'] == 'full-window' and pattern['stages'][0]['durationMs'] == duration:
            summary['movedBoxes'] = 'already full window'
        elif config['mode'] == 'full-window':
            old = pattern['stages'][0]['durationMs']
            assert old == config['duration'] - P7_TIME_SHIFT_MS, ('unexpected P7 window', old)
            pattern['displayName'] = config['name']
            pattern['stages'][0]['durationMs'] = duration
            # Existing fade/effect/scene-profile boxes keep their source clock: same
            # source instant, now measured from the full-window origin.
            moved = []
            for box in pattern['presentationOccurrences']:
                if not box['resourceId'].startswith('presentation.' + config['prefix'] + '.camera.'):
                    box['startMs'] += P7_TIME_SHIFT_MS
                    assert box['startMs'] + box['durationMs'] <= duration, box['occurrenceId']
                    moved.append((box['occurrenceId'], box['startMs'], box['durationMs']))
            for box in pattern['sceneProfileOccurrences']:
                box['startMs'] += P7_TIME_SHIFT_MS
                assert box['startMs'] + box['durationMs'] <= duration, box['occurrenceId']
                moved.append((box['occurrenceId'], box['startMs'], box['durationMs']))
            summary['movedBoxes'] = moved
            # Actors: the 2관문_클리어 worlds were baked from this Matinee at 0ms.
            assert all(w in world_ids for w in P7_ACTOR_DONORS)
            ordinal = pattern['nextWorldOccurrenceOrdinal']
            pattern['worldOccurrences'] = []
            for world in P7_ACTOR_DONORS:
                pattern['worldOccurrences'].append(dict(occurrenceId=f'{pid}.world.{ordinal}', worldId=world,
                                                        startMs=0, durationMs=duration, playbackSpeed=1))
                ordinal += 1
            pattern['nextWorldOccurrenceOrdinal'] = ordinal
            summary['worldOccurrences'] = [o['worldId'] for o in pattern['worldOccurrences']]
        shots = replace_cameras(docs, pattern, config, rows)
        summary['cameraShots'] = [dict(shotId=s['shotId'], startMs=a, endMs=b, keys=len(s['cameraTrack']['keyframes']))
                                  for a, b, s in shots]
        if config['mode'] != 'cameras':
            # Source map props: none of these two Matinees animate a resolvable static mesh;
            # record the count so the report proves it rather than assuming it.
            props, sets, failures, count = bss.static_worlds(config, rows)
            assert count == 0 and not sets, ('unexpected source props', count, len(sets))
            summary['sourceStaticProps'] = count
        summaries.append(summary)
    for key in ('composition', 'cameras'):
        docs[key]['revision'] += 1
    camera_bytes = json.dumps(docs['cameras'], ensure_ascii=False, indent=2).encode('utf-8')
    assert len(camera_bytes) < 2 * 1024 * 1024, 'Camera exceeds runtime byte bound'
    assert len(docs['cameras']['shots']) <= 128, len(docs['cameras']['shots'])
    for shot in docs['cameras']['shots']:
        assert len(shot['cameraTrack']['keyframes']) <= 64 if 'cameraTrack' in shot else True, shot['shotId']
    outputs = {paths['composition']: docs['composition'], paths['cameras']: docs['cameras']}
    if OUT.exists():
        shutil.rmtree(OUT)
    for path, document in outputs.items():
        base.write(OUT / 'candidate' / path.relative_to(ROOT), document)
    base.write(OUT / 'report.json', dict(installed=args.install, patterns=summaries,
               cameraBytes=len(camera_bytes), cameraShots=len(docs['cameras']['shots']),
               baseline={k: hashlib.sha256(v).hexdigest() for k, v in baseline.items()},
               manualVisualValidation='USER_PENDING'))
    if args.install:
        for key, path in paths.items():
            if path.read_bytes() != baseline[key]:
                raise ValueError('Concurrent authoring changed: ' + str(path))
        for path, document in outputs.items():
            base.write(path, document)
    print(json.dumps(dict(installed=args.install, cameraShots=len(docs['cameras']['shots']), cameraBytes=len(camera_bytes),
                          patterns=[dict(p, cameraShots=len(p['cameraShots'])) for p in summaries]), ensure_ascii=False), flush=True)


if __name__ == '__main__':
    main()
