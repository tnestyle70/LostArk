# -*- coding: utf-8 -*-
"""usage:
  <blender-python> build_hitshapes.py <Asset> [<Asset> ...] [--check]
"""
import io, json, os, re, struct, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
RESOURCES = os.path.join(REPO, 'Client', 'Bin', 'Resources')
TICK_RATE = 30.0
UNITS_TO_METERS = 0.01
MAX_SUB_HITS = 64
AREA_CIRCLE = 1
AREA_BOX = 2
AREA_FAN = 3


def read_clip_ticks(wmodel_path):
    data = open(wmodel_path, 'rb').read()
    cb = 16
    _m, section_count, _a, _f = struct.unpack_from('<4sIII', data, cb)
    ticks = {}
    for i in range(section_count):
        o = cb + 32 + i * 64
        kind, _idx, off, _size = struct.unpack_from('<IIQQ', data, o)
        name = data[o + 24:o + 64].split(b'\x00')[0].decode('ascii', 'replace')
        if kind != 4:
            continue
        b = cb + off
        _mg, _channels, duration_ticks, _tps, _k, _e, _l = struct.unpack_from('<4sIffIIB', data, b + 16)
        ticks[name] = duration_ticks
    return ticks


def read_hit_rows(asset):
    path = os.path.join(REPO, 'Data', 'Animation', 'Authored', asset, asset + '.animevents')
    hits = {}
    for line in io.open(path, 'rb').read().decode('latin-1').split('\n'):
        m = re.match(r'^"([^"]+)" HIT (.*)$', line.rstrip('\r'))
        if not m:
            continue
        fields = dict(kv.split('=', 1) for kv in m.group(2).split() if '=' in kv)
        area = int(fields.get('area', '0'))
        if area <= 0:
            continue
        hits.setdefault(m.group(1), []).append({
            'startMs': int(fields['startms']),
            'rep': max(1, int(fields.get('rep', '1'))),
            'repMs': int(fields.get('repms', '0')),
            'area': area,
            'ar': int(fields.get('ar', '0')),
            'aa': int(fields.get('aa', '0')),
            'ah': int(fields.get('ah', '0')),
            'ax': int(fields.get('ax', '0')),
            'arem': int(fields.get('arem', '0')),
            'maxt': int(fields.get('maxt', '0')),
            'push': int(fields.get('push', '0')),
            'pushr': int(fields.get('pushr', '0')),
        })
    for rows in hits.values():
        rows.sort(key=lambda h: h['startMs'])
    return hits


def read_projectile_rows(asset):
    path = os.path.join(REPO, 'Data', 'Animation', 'Authored', asset, asset + '.projectiles.json')
    if not os.path.exists(path):
        return {}
    document = json.load(io.open(path, encoding='utf-8'))
    if document.get('schema') != 'lostark.animation-projectiles' or document.get('formatVersion') != 1:
        raise SystemExit('%s: unexpected projectiles document header' % asset)
    rows = {}
    for entry in document['projectiles']:
        rows.setdefault((int(entry['skillId']), entry['clip']), []).append(entry)
    return rows


MAX_PROJECTILES = 8


def stage_projectiles(skill_id, entries, clip_ticks, projectile_rows, limit_ms, label):
    """Every object the stage's clips spawn, in stage-local time; the object's own
    hits keep their spawn-relative schedule and are not rescaled by playRate."""
    out = []
    elapsed_ms = 0.0
    for entry in entries:
        name = entry if isinstance(entry, str) else entry['clip']
        play_ms = 0 if isinstance(entry, str) else int(entry.get('playMs', 0))
        rate = 1.0 if isinstance(entry, str) else float(entry.get('playRate', 1.0))
        source_ms = clip_ticks[name] / TICK_RATE * 1000.0
        if play_ms:
            source_ms = min(source_ms, float(play_ms))
        for spawn in projectile_rows.get((skill_id, name), []):
            spawn_ms = elapsed_ms + spawn['startMs'] / rate
            out.append({
                'timeMs': min(int(round(spawn_ms)), limit_ms),
                'kind': spawn['kind'],
                'origin': spawn['origin'],
                'offsetForward': spawn['offsetForward'],
                'offsetRight': spawn['offsetRight'],
                'speed': spawn['speed'],
                'minDistance': spawn['minDistance'],
                'maxDistance': spawn['maxDistance'],
                'lifeMs': spawn['lifeMs'],
                'radius': spawn['radius'],
                'hits': [{k: v for k, v in h.items() if k != 'sourcePk'} for h in spawn['hits']],
            })
        elapsed_ms += source_ms / rate
    out.sort(key=lambda p: p['timeMs'])
    if len(out) > MAX_PROJECTILES:
        raise SystemExit('%s: more than %d projectiles' % (label, MAX_PROJECTILES))
    return out


def stage_hits(entries, clip_ticks, clip_hits, limit_ms, label):
    out = []
    elapsed_ms = 0.0
    for entry in entries:
        name = entry if isinstance(entry, str) else entry['clip']
        play_ms = 0 if isinstance(entry, str) else int(entry.get('playMs', 0))
        rate = 1.0 if isinstance(entry, str) else float(entry.get('playRate', 1.0))
        if name not in clip_ticks:
            raise SystemExit('%s: clip %s is not in the body model' % (label, name))
        source_ms = clip_ticks[name] / TICK_RATE * 1000.0
        if play_ms:
            source_ms = min(source_ms, float(play_ms))
        for h in clip_hits.get(name, []):
            time_ms = elapsed_ms + h['startMs'] / rate
            # Official AreaType: 1 circle/ring, 2 forward box whose AreaAngle
            # is the width in cm, 3 fan whose AreaAngle is the sweep in degrees.
            out.append({
                'timeMs': min(int(round(time_ms)), limit_ms),
                'repeatCount': h['rep'],
                'repeatMs': int(round(h['repMs'] / rate)),
                'areaType': h['area'],
                'range': round(h['ar'] * UNITS_TO_METERS, 2),
                'angle': min(max(h['aa'], 0), 360) if h['area'] == AREA_FAN else 0,
                'width': round(h['aa'] * UNITS_TO_METERS, 2) if h['area'] == AREA_BOX else 0.0,
                'height': round(h['ah'] * UNITS_TO_METERS, 2),
                'offset': round(h['ax'] * UNITS_TO_METERS, 2),
                'inner': round(h['arem'] * UNITS_TO_METERS, 2),
                'maxTargets': h['maxt'],
                # Official push: duration in ms (0 = no push) and signed range,
                # negative pulling the target toward the caster.
                'pushMs': max(0, h['push']),
                'pushRange': round(h['pushr'] * UNITS_TO_METERS, 2) if h['push'] > 0 else 0.0,
            })
        elapsed_ms += source_ms / rate
    out.sort(key=lambda h: h['timeMs'])
    if sum(h['repeatCount'] for h in out) > MAX_SUB_HITS:
        raise SystemExit('%s: more than %d sub-hits' % (label, MAX_SUB_HITS))
    return out


def build(asset):
    catalog = json.load(io.open(os.path.join(REPO, 'Data', 'Actors', 'CharacterCatalog.json'), encoding='utf-8'))
    entry = next(c for c in catalog['characters'] if c['assetId'] == asset)
    clip_ticks = read_clip_ticks(os.path.join(RESOURCES, *entry['bodyModel'].split('/')))
    bindings = json.load(io.open(os.path.join(REPO, 'Data', 'Animation', 'Authored', asset, asset + '.skillbindings.json'), encoding='utf-8'))
    balance = json.load(io.open(os.path.join(REPO, 'Data', 'Balance', 'PlayerSkills.json'), encoding='utf-8'))
    skills = {int(s['skillId']): s for s in balance['skills'] if s['characterClass'] == bindings['characterClass']}
    clip_hits = read_hit_rows(asset)
    projectile_rows = read_projectile_rows(asset)
    out = []
    for binding in sorted(bindings['bindings'], key=lambda b: int(b['skillId'])):
        skill_id = int(binding['skillId'])
        skill = skills.get(skill_id)
        if skill is None or not skill.get('serverDamageProfileId'):
            continue
        entries = binding['clips']
        stages = [list(e) for e in entries] if entries and isinstance(entries[0], list) else [list(entries)]
        combo_stages = list(skill.get('comboStages') or [])
        if skill['skillKind'] in ('COMBO', 'HOLD', 'COUNTER'):
            if len(stages) != len(combo_stages):
                raise SystemExit('%s %d: stage count mismatch' % (asset, skill_id))
            rows = []
            for index, group in enumerate(stages):
                label = '%s %d stage %d' % (asset, skill_id, index)
                limit_ms = int(combo_stages[index]['actionDurationMs'])
                hits = stage_hits(group, clip_ticks, clip_hits, limit_ms, label)
                projectiles = stage_projectiles(skill_id, group, clip_ticks, projectile_rows, limit_ms, label)
                if hits or projectiles:
                    row = {'stageIndex': index, 'hits': hits}
                    if projectiles:
                        row['projectiles'] = projectiles
                    rows.append(row)
            if rows:
                out.append({'skillId': skill_id, 'stages': rows})
            continue
        label = '%s %d' % (asset, skill_id)
        limit_ms = int(skill['actionDurationMs'])
        hits = stage_hits(stages[0], clip_ticks, clip_hits, limit_ms, label)
        projectiles = stage_projectiles(skill_id, stages[0], clip_ticks, projectile_rows, limit_ms, label)
        if hits or projectiles:
            row = {'skillId': skill_id, 'hits': hits}
            if projectiles:
                row['projectiles'] = projectiles
            out.append(row)
    return {
        'schema': 'lostark.animation-hit-shapes',
        'formatVersion': 3,
        'animationAssetId': asset,
        'characterClass': bindings['characterClass'],
        'skills': out,
    }


def main(argv):
    check = '--check' in argv
    assets = [a for a in argv if not a.startswith('--')]
    if not assets:
        raise SystemExit(__doc__)
    out_dir = os.path.join(REPO, 'Data', 'Animation', 'HitShapes')
    os.makedirs(out_dir, exist_ok=True)
    for asset in assets:
        document = build(asset)
        text = json.dumps(document, indent=2, ensure_ascii=False) + '\n'
        path = os.path.join(out_dir, asset + '.hitshapes.json')
        old = io.open(path, encoding='utf-8').read() if os.path.exists(path) else None
        skill_count = len(document['skills'])
        hit_count = sum(len(s.get('hits', [])) + sum(len(st['hits']) for st in s.get('stages', [])) for s in document['skills'])
        projectile_count = sum(len(s.get('projectiles', [])) + sum(len(st.get('projectiles', [])) for st in s.get('stages', []))
                               for s in document['skills'])
        print('%s: %d skills, %d hits, %d projectiles, %s' % (
            asset, skill_count, hit_count, projectile_count, 'unchanged' if old == text else 'changed'))
        if old != text and not check:
            io.open(path, 'w', encoding='utf-8', newline='\n').write(text)


if __name__ == '__main__':
    main(sys.argv[1:])
