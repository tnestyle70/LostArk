"""Project original Waterpang camera/tile curves into the existing MapTool.

Prepare is read-only outside out/. --apply adds only reviewed authoring IDs;
it never runs the Client, publishes runtime files, or modifies map placements.
Non-transform source tracks are inventoried, NOT silently called implemented.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import shlex
import sys
from functools import lru_cache
from pathlib import Path

import numpy as np
from scipy.spatial.transform import Rotation

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/KoukuSaydonPipeline'))
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
import build_gate2_intro_composition as source
from build_source_sequences import project_registration
from source_character_registration import commit_staged_files

AREA = 'LV_OCN_EVENTIS_MHP'
SCENE = AREA + '_SCENE03B'
AUTHORING = ROOT / 'Data/Maps/Authoring' / AREA
AUDIT = ROOT / 'out/MaharakaMapRestoration20260927/full-source-audit' / (SCENE + '.objects.json')
OUT = ROOT / 'out/MaharakaWaterpang20260927'
PREFIX = 'maharaka.waterpang.source'
MOTIONS = ((41, 156, 'shake', '흔들림'),
           (42, 157, 'collapse', '붕괴'),
           (44, 159, 'collapsed', '붕괴 상태'),
           (46, 161, 'repair', '복구'))
CAMERAS = ((43, 158, 'intro15'), (45, 160, 'intro20'))


@lru_cache(maxsize=1)
def interp_defaults():
    path = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/NE1FENCQ4UNE9ZPRENOQS.u')
    package = source.load_package(path, source.ue3.LOSTARK_KR_AES_KEY)
    entries = [e for e in package.exports if source.ue3.package_ref_path(
        e.index+1, package.imports, package.exports).casefold() == 'default__interpdata']
    require(len(entries) == 1, 'Requires installed Engine InterpData CDO')
    entry = entries[0]
    props, _ = source.ue3.parse_tagged_properties(package.logical[entry.serial_offset:entry.serial_offset+entry.serial_size],
                                                  package.names, package.summary.version)
    props = {k.casefold(): v for k,v in source.unwrap(props).items()}
    require(isinstance(props.get('interplength'), (int, float)) and props['interplength'] > 0, 'Missing CDO InterpLength')
    return dict(physicalPackage=str(path), sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                objectPath='Default__InterpData', packageIndex=entry.index+1, interpLength=props['interplength'])


def encoded(document):
    return (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf-8')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def load_json(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def placements(raw):
    lines = raw.decode('utf-8-sig').splitlines()
    header = shlex.split(lines[0])
    require(header[:3] == ['LOSTARK_MAP_PLACEMENTS', '2', AREA], 'Placement header changed')
    result = {}
    for line in lines[1:]:
        fields = shlex.split(line)
        if not fields:
            continue
        require(len(fields) == 16, 'Unexpected placement row')
        if fields[1].startswith(SCENE + ':export:'):
            require(fields[1] not in result, 'Duplicate source component placement')
            result[fields[1]] = dict(id=fields[0], asset=fields[4],
                p=np.array(fields[5:8], dtype=float),
                r=Rotation.from_quat(np.array(fields[8:12], dtype=float)).as_matrix(),
                scale=np.array(fields[12:15], dtype=float), visible=fields[15])
    return result


def groups_for(rows, matinee, data):
    links = [v for v in rows[matinee]['p']['variablelinks'] if v['linkdesc'].casefold() == 'data']
    require(len(links) == 1 and links[0]['linkedvariables'] == [data], 'Matinee/Data identity mismatch')
    return rows[data]['p']['interpgroups']


def consumed_camera_tracks(rows, matinee, data):
    """Only Director, selected camera FOV/Move and animated parent Move are consumed."""
    groups = groups_for(rows, matinee, data)
    by_name = {rows[g]['p'].get('groupname'): g for g in groups}
    consumed = set()
    selected = set()
    for group in groups:
        for track in source.active_tracks(rows, group):
            if track['cls'] == 'interptrackdirector':
                consumed.add(track['index'])
                selected.update(by_name[c['targetcamgroup']] for c in track['p'].get('cuttrack', [])
                                if c['targetcamgroup'] in by_name)
    for group in selected:
        consumed.update(t['index'] for t in source.active_tracks(rows, group)
                        if t['cls'] == 'interptrackfloatprop' and t['p'].get('propertyname', '').casefold() == 'fovangle')
    pending = list(selected)
    visited = set()
    while pending:
        group = pending.pop()
        if group in visited:
            continue
        visited.add(group)
        consumed.update(t['index'] for t in source.active_tracks(rows, group) if t['cls'] == 'interptrackmove')
        for actor in source.group_actor(rows, group, matinee):
            parent = rows[actor]['p'].get('base')
            if parent in rows:
                pending.extend(g for g in groups if parent in source.group_actor(rows, g, matinee))
    return consumed


def angle(a, b):
    return math.degrees(2 * math.acos(min(1., abs(float(np.dot(a, b))))))


def sampled_keys(rows, group, actor, matinee, data, duration, baseline):
    tracks = source.active_tracks(rows, group)
    require(len(tracks) == 1 and tracks[0]['cls'] == 'interptrackmove', 'Tile has unsupported active tracks')
    move = tracks[0]['p']
    require(not move.get('busequatinterpolation', False), 'Quaternion source Move needs a different sampler')
    require(move.get('rotmode', 'imr_keyframed') == 'imr_keyframed', 'Unsupported tile look-at')
    times = {0, duration} | set(range(0, duration, 16))
    for field in ('postrack', 'eulertrack'):
        for point in move.get(field, {}).get('points', []):
            require(point.get('interpmode', 'cim_linear') in
                    ('cim_linear', 'cim_constant', 'cim_curveauto', 'cim_curveautoclamped',
                     'cim_curveuser', 'cim_curvebreak'), 'Unknown UE interpolation mode')
            # Keep adjacent millisecond samples at fractional authored boundaries.
            for ms in (math.floor(point['inval'] * 1000), math.ceil(point['inval'] * 1000)):
                times.update(t for t in (ms - 1, ms, ms + 1) if 0 <= t <= duration)
    cache = {}
    require(len(times) <= 4096, 'Initial tile curve exceeds runtime key limit')

    def pose(ms):
        if ms not in cache:
            p, r = source.world_pose(rows, group, actor, ms / 1000., matinee, data)
            cache[ms] = (baseline['r'].T @ (p - baseline['p']),
                         Rotation.from_matrix(baseline['r'].T @ r).as_quat())
        return cache[ms]

    # Refine against quarter/midpoint source samples, not only authored endpoints.
    while True:
        additions = set()
        ordered = sorted(times)
        for left, right in zip(ordered, ordered[1:]):
            a, b = pose(left), pose(right)
            for ms in {round(left + (right-left)*u) for u in (.25, .5, .75)} - {left, right}:
                t = (ms-left)/(right-left)
                p, q = pose(ms)
                if np.linalg.norm(p - (a[0]*(1-t)+b[0]*t)) > .001 or angle(q, source.slerp(a[1], b[1], t)) > .05:
                    additions.add(ms)
        if not additions:
            break
        times.update(additions)
        require(len(times) <= 4096, 'Tile curve exceeds runtime key limit')
    keys = []
    previous = None
    for ms in sorted(times):
        p, q = pose(ms)
        if previous is not None and np.dot(previous, q) < 0:
            q = -q
        previous = q
        keys.append(dict(timeMs=ms, positionOffset=p.tolist(), rotationQuaternion=q.tolist(),
                         scaleMultiplier=[1., 1., 1.], visible=True))
    return keys


def build_motion(rows, live, matinee, data, tag, name):
    groups = groups_for(rows, matinee, data)
    tile_groups = [g for g in groups if rows[g]['p'].get('groupname', '').startswith('t')
                   and rows[g]['p'].get('groupname', '')[1:].isdigit()]
    require(len(tile_groups) == 18, 'Expected exactly 18 source tile groups')
    source_length = rows[data]['p'].get('interplength')
    duration = math.ceil((source_length if source_length is not None else interp_defaults()['interpLength']) * 1000)
    require(0 < duration <= 600000, 'Invalid preview duration')
    tracks, bindings, evidence = [], [], []
    for group in tile_groups:
        actors = source.group_actor(rows, group, matinee)
        require(len(actors) == 1, 'Expected one bound source actor per tile')
        actor = actors[0]
        require(rows[actor]['cls'] == 'interpactor', 'Tile is not an InterpActor')
        component = rows[actor]['p']['staticmeshcomponent']
        source_id = f'{SCENE}:export:{component-1}'
        require(source_id in live, 'Missing exact source component placement: ' + source_id)
        baseline = live[source_id]
        p, r = source.world_pose(rows, None, actor, 0, matinee, data)
        require(np.linalg.norm(p-baseline['p']) < .00001, 'Installed tile position differs from source')
        require(np.linalg.norm(r-baseline['r']) < .00001, 'Installed tile rotation differs from source')
        scale = source.vec(rows[actor]['p'].get('drawscale3d'), (1, 1, 1))[[0, 2, 1]]
        scale *= rows[actor]['p'].get('drawscale', 1.)
        require(np.max(np.abs(scale-baseline['scale'])) < .00001, 'Installed tile scale differs from source')
        require(baseline['visible'] == '1', 'Cannot animate a hidden baseline tile')
        slot = rows[group]['p']['groupname']
        keys = sampled_keys(rows, group, actor, matinee, data, duration, baseline)
        tracks.append(dict(slotId=slot, keys=keys))
        bindings.append(dict(slotId=slot, targetKind='MAP_PLACEMENT', targetId=baseline['id']))
        evidence.append(dict(slotId=slot, actor=rows[actor]['name'], component=rows[component]['name'],
                             sourcePlacementId=source_id, placementId=baseline['id'], keyCount=len(keys)))
    require(len({b['targetId'] for b in bindings}) == 18, 'Duplicate tile binding')
    identity = PREFIX + '.' + tag
    label = '워터팡 / 원본 바닥 ' + name + ' (맵 동작 미리보기)'
    template = dict(sequenceId='sequence.'+identity, displayName=label, category='World',
                    durationMs=duration, interpolation='LINEAR', tracks=tracks, animationTracks=[])
    instance = dict(instanceId='world.sequence.instance.'+identity, templateId=template['sequenceId'],
                    enabled=True, startDelayMs=0, playbackSpeed=1, bindings=bindings)
    cutscene = dict(cutsceneId='cutscene.'+identity, displayName=label, durationMs=duration,
                    cameraCuts=[], worldInstanceIds=[instance['instanceId']])
    report = dict(matinee=matinee, interpData=data, durationMs=duration,
                  durationBasis='serialized InterpLength' if source_length is not None else 'installed Engine Default__InterpData.InterpLength',
                  bindings=evidence)
    return template, instance, cutscene, report


def append_owned(document, field, key, additions):
    existing = {r[key]: r for r in document.setdefault(field, [])}
    require(len(existing) == len(document[field]), 'Duplicate existing IDs: '+field)
    for row in additions:
        if row[key] in existing:
            require(existing[row[key]] == row, 'Preserve modified authoring row: '+row[key])
        else:
            document[field].append(row)
            existing[row[key]] = row


def prepare():
    audit = load_json(AUDIT)['source']
    package = Path(audit['physicalPackage'])
    require(hashlib.sha256(package.read_bytes()).hexdigest() == audit['sha256'], 'Original package changed; re-audit first')
    rows, imports = source.extract_scene(package)
    placement_path = AUTHORING / (AREA+'.mapplacements')
    placement_bytes = placement_path.read_bytes()
    live = placements(placement_bytes)
    templates, instances, shots, cutscenes, evidence = [], [], [], [], []
    for config in MOTIONS:
        t, i, c, report = build_motion(rows, live, *config)
        templates.append(t); instances.append(i); cutscenes.append(c); evidence.append(report)
    for matinee, data, tag in CAMERAS:
        groups_for(rows, matinee, data)
        duration = round(rows[data]['p']['interplength']*1000)
        identity = PREFIX+'.'+tag
        label = f'워터팡 / 원본 도입 카메라 {rows[matinee]["name"].rsplit("_", 1)[-1]} (카메라만)'
        camera_rows = source.make_cameras(rows, matinee, data, duration, identity, label)
        shots.extend(s for _start, _end, s in camera_rows)
        cutscenes.append(dict(cutsceneId='cutscene.'+identity, displayName=label, durationMs=duration,
            cameraCuts=[dict(cutId=identity+f'.cut.{n}', shotId=s['shotId'], startMs=start)
                        for n, (start, _end, s) in enumerate(camera_rows)], worldInstanceIds=[]))
    world_path = AUTHORING / (AREA+'.worldsequences.json')
    camera_path = AUTHORING / (AREA+'.camerashots.json')
    staged = {}
    for path, initial, additions in (
        (world_path, dict(schema='lostark.world-sequences', formatVersion=3, areaId=AREA, revision=1,
                         objectResources=[], templates=[], instances=[]),
         [('templates', 'sequenceId', templates), ('instances', 'instanceId', instances)]),
        (camera_path, dict(schema='lostark.camera-shots', formatVersion=1, areaId=AREA, revision=1, shots=[], cutscenes=[]),
         [('shots', 'shotId', shots), ('cutscenes', 'cutsceneId', cutscenes)])):
        before = path.read_bytes() if path.exists() else None
        document = json.loads(before.decode('utf-8-sig')) if before is not None else initial
        require(document['schema'] == initial['schema'] and document['formatVersion'] == initial['formatVersion']
                and document['areaId'] == AREA, 'Incompatible authoring document')
        original = copy.deepcopy(document)
        for field, key, values in additions:
            append_owned(document, field, key, values)
        if before is not None and document != original:
            document['revision'] += 1
        after = before if document == original and before is not None else encoded(document)
        require(len(after) <= (16777216 if path == world_path else 2097152), 'Merged document exceeds runtime byte limit')
        staged[path] = before, after
    catalog_path = ROOT/'Data/Maps/MapCatalog.json'
    before = catalog_path.read_bytes()
    text = before.decode('utf-8-sig')
    # Patch only this Area's object, retaining every other byte and row.
    start = text.rfind('{', 0, text.index('"id": "'+AREA+'"'))
    old, _ = json.JSONDecoder().raw_decode(text[start:])
    old_text = text[start:start+json.JSONDecoder().raw_decode(text[start:])[1]]
    additions = dict(sourceSequences=world_path.relative_to(ROOT).as_posix(),
                     sequences=f'Client/Bin/DataFiles/Map/{AREA}.worldsequences.json',
                     sourceCameraShots=camera_path.relative_to(ROOT).as_posix(),
                     cameraShots=f'Client/Bin/DataFiles/Map/{AREA}.camerashots.json')
    for key, value in additions.items():
        require(key not in old or old[key] == value, 'Conflicting MapCatalog path: '+key)
    missing = {k:v for k,v in additions.items() if k not in old}
    newline = '\r\n' if '\r\n' in text else '\n'
    changed = old_text.rstrip()[:-1].rstrip()
    if missing:
        changed += ',' + newline  # append fields without reformatting old entries
        changed += (','+newline).join('      '+json.dumps(k)+': '+json.dumps(v) for k,v in missing.items())
        changed += newline+'    }'
        text = text[:start]+changed+text[start+len(old_text):]
    json.loads(text)
    staged[catalog_path] = before, (b'\xef\xbb\xbf' if before.startswith(b'\xef\xbb\xbf') else b'')+text.encode('utf-8')
    for path, before, after in project_registration([world_path, camera_path]):
        staged[path] = before, after
    inventory = []
    for matinee, data in [(m,d) for m,d,_,_ in MOTIONS]+[(m,d) for m,d,_ in CAMERAS]:
        if (matinee, data) in [(m,d) for m,d,_ in CAMERAS]:
            consumed = consumed_camera_tracks(rows, matinee, data)
        else:
            consumed = {t['index'] for g in groups_for(rows, matinee, data)
                        if rows[g]['p'].get('groupname', '').startswith('t') and rows[g]['p'].get('groupname', '')[1:].isdigit()
                        for t in source.active_tracks(rows, g) if t['cls'] == 'interptrackmove'}
        for group in groups_for(rows, matinee, data):
            for track in source.active_tracks(rows, group):
                if track['index'] not in consumed:
                    inventory.append(dict(matinee=matinee, group=rows[group]['p'].get('groupname'),
                        track=track['index'], cls=track['cls'], properties=track['p'], status='not connected by this camera/tile importer'))
    report = dict(source=audit, interpDataDefault=interp_defaults(), importedTileMotions=evidence, cameraShotCount=len(shots), cutsceneCount=len(cutscenes),
                  unconnectedSourceTracks=inventory, imports=imports,
                  limitations=['Camera entries are camera-only: source actor/effect/audio tracks remain unconnected.',
                    'Tile previews stop/restore; source looping and server event/gameplay are not implemented.',
                    'Motions share 18 map placements and must not run concurrently.',
                    'Repair Euler curves cross +180/-180 and currently retain the authored full rotation; inherited UE quaternion interpolation still requires confirmation.',
                    'Source startup branch selection remains unverified.'])
    return staged, {placement_path: placement_bytes}, report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    staged, expected, report = prepare()
    OUT.mkdir(parents=True, exist_ok=True)
    for path, (before, after) in staged.items():
        relative = path.relative_to(ROOT)
        candidate = OUT/'candidate'/relative
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(after)
        if before is not None:
            backup = OUT/'before'/relative
            backup.parent.mkdir(parents=True, exist_ok=True)
            if not backup.exists():
                backup.write_bytes(before)
    (OUT/'source-registration-report.json').write_bytes(encoded(report))
    if args.apply:
        commit_staged_files(staged, expected=expected)
    print(json.dumps(dict(applied=args.apply, files=sum(a != b for a,b in staged.values()),
                          cameraShots=report['cameraShotCount'], cutscenes=report['cutsceneCount'],
                          tileMotions=len(report['importedTileMotions']), report=str(OUT/'source-registration-report.json')),
                     ensure_ascii=False))


if __name__ == '__main__':
    main()
