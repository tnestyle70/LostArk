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
            out.append({
                'timeMs': min(int(round(time_ms)), limit_ms),
                'repeatCount': h['rep'],
                'repeatMs': int(round(h['repMs'] / rate)),
                'areaType': h['area'],
                'range': round(h['ar'] * UNITS_TO_METERS, 2),
                'angle': min(max(h['aa'], 0), 360),
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
                hits = stage_hits(group, clip_ticks, clip_hits, int(combo_stages[index]['actionDurationMs']),
                                  '%s %d stage %d' % (asset, skill_id, index))
                if hits:
                    rows.append({'stageIndex': index, 'hits': hits})
            if rows:
                out.append({'skillId': skill_id, 'stages': rows})
            continue
        hits = stage_hits(stages[0], clip_ticks, clip_hits, int(skill['actionDurationMs']), '%s %d' % (asset, skill_id))
        if hits:
            out.append({'skillId': skill_id, 'hits': hits})
    return {
        'schema': 'lostark.animation-hit-shapes',
        'formatVersion': 1,
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
        print('%s: %d skills, %d hits, %s' % (asset, skill_count, hit_count, 'unchanged' if old == text else 'changed'))
        if old != text and not check:
            io.open(path, 'w', encoding='utf-8', newline='\n').write(text)


if __name__ == '__main__':
    main(sys.argv[1:])
