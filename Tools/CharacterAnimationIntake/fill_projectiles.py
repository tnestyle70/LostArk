# -*- coding: utf-8 -*-
"""usage:
  <blender-python> fill_projectiles.py <Asset> [<Asset> ...] [--check]

Promotes the reference <Asset>.projectiles (objects a skill spawns: missiles,
fixed areas, grenades, traces, with the SkillEffect hits the object itself
applies) into Data/Animation/Authored/<Asset>/<Asset>.projectiles.json for the
damage skills of the class. Only the clips of the authored skillbindings chain are
kept, only the skill's lowest clipseq group (the tripod-free chain) counts, the lowest
PK of a same-time spawn is the base object, and an
object without a damaging shaped hit (dmg > 0 against enemies, area > 0) is skipped.
"""
import io, json, os, re, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
REF = os.path.join(REPO, 'Data', 'Animation', 'Reference')
AUTH = os.path.join(REPO, 'Data', 'Animation', 'Authored')
UNITS_TO_METERS = 0.01

KINDS = ('MISSILE', 'FIXAREA', 'GRENADE', 'TRACE')
ORIGINS = {0: 'CASTER', 1: 'AIM'}


def read_lines(path):
    with io.open(path, 'rb') as f:
        return f.read().decode('utf-8').split('\n')


def parse_pairs(text):
    out = {}
    for m in re.finditer(r'(\w+)=("([^"]*)"|\S+)', text):
        out[m.group(1)] = m.group(3) if m.group(3) is not None else m.group(2)
    return out


def load_base_seq(asset):
    """The tripod-free chain of a skill is its lowest clipseq group; a higher
    group re-uses the same clips with the tripod's own spawns added."""
    base = {}
    for line in read_lines(os.path.join(REF, asset, asset + '.clipseq'))[1:]:
        m = re.match(r'^(\d+) "[^"]*" seq=(\d+)', line)
        if m:
            skill, seq = int(m.group(1)), int(m.group(2))
            base[skill] = min(base.get(skill, seq), seq)
    return base


def load_reference(asset):
    rows = []
    for line in read_lines(os.path.join(REF, asset, asset + '.projectiles'))[1:]:
        line = line.rstrip('\r')
        m = re.match(r'^(\d+) "[^"]*"(.*)$', line)
        if m:
            p = parse_pairs(m.group(2))
            rows.append({'skill': int(m.group(1)), 'pk': int(p['pk']), 'kind': p['kind'],
                         'seq': int(p['seq']), 'clip': p['clip'], 't': float(p['t']),
                         'origin': int(p.get('origin', '0')), 'ox': int(p.get('ox', '0')),
                         'oy': int(p.get('oy', '0')),
                         'radius': int(p['radius']), 'mindist': int(p['mindist']),
                         'maxdist': int(p['maxdist']), 'life': float(p['life']),
                         'speed': int(p['speed']), 'layout': p['layout'], 'hits': []})
            continue
        m = re.match(r'^  e (.*)$', line)
        if m and rows:
            rows[-1]['hits'].append(parse_pairs(m.group(1)))
    return rows


def hit_of(e):
    if 'missing' in e or int(e.get('area', '0')) <= 0 or int(e.get('dmg', '0')) <= 0:
        return None
    area = int(e['area'])
    aa = int(e.get('aa', '0'))
    contact = e.get('contact') == '1'
    rep = max(1, int(e.get('rep', '0')))
    push = int(e.get('push', '0'))
    return {
        'sourcePk': int(e['pk']),
        'trigger': 'CONTACT' if contact else 'TIMED',
        'atMs': 0 if contact else int(round(float(e['at']) * 1000.0)),
        'count': rep if contact else max(1, int(e['count'])),
        'everyMs': int(e.get('repms', '0')) if contact else int(round(float(e['every']) * 1000.0)),
        'areaType': area,
        'range': round(int(e['ar']) * UNITS_TO_METERS, 2),
        'angle': min(max(aa, 0), 360) if area == 3 else 0,
        'width': round(aa * UNITS_TO_METERS, 2) if area == 2 else 0.0,
        'height': round(int(e.get('ah', '0')) * UNITS_TO_METERS, 2),
        'offset': round(int(e.get('ax', '0')) * UNITS_TO_METERS, 2),
        'inner': round(int(e.get('arem', '0')) * UNITS_TO_METERS, 2),
        'maxTargets': int(e.get('maxt', '0')),
        'pushMs': max(0, push),
        'pushRange': round(int(e.get('pushr', '0')) * UNITS_TO_METERS, 2) if push > 0 else 0.0,
    }


def build(asset):
    bindings = json.load(io.open(os.path.join(AUTH, asset, asset + '.skillbindings.json'), encoding='utf-8'))
    balance = json.load(io.open(os.path.join(REPO, 'Data', 'Balance', 'PlayerSkills.json'), encoding='utf-8'))
    skills = {int(s['skillId']): s for s in balance['skills'] if s['characterClass'] == bindings['characterClass']}
    reference = load_reference(asset)
    base_seq = load_base_seq(asset)
    out = []
    skipped = 0
    for binding in sorted(bindings['bindings'], key=lambda b: int(b['skillId'])):
        skill_id = int(binding['skillId'])
        skill = skills.get(skill_id)
        if skill is None or not skill.get('serverDamageProfileId'):
            continue
        entries = binding['clips']
        stages = [list(e) for e in entries] if entries and isinstance(entries[0], list) else [list(entries)]
        chain = {e if isinstance(e, str) else e['clip'] for stage in stages for e in stage}
        candidates = [r for r in reference if r['skill'] == skill_id and r['clip'] in chain
                      and r['kind'] in KINDS and r['layout'] != 'none'
                      and r['seq'] == base_seq.get(skill_id, r['seq'])]
        groups = {}
        for row in candidates:
            key = (row['clip'], round(row['t'], 3), row['kind'])
            best = groups.get(key)
            if best is None or row['pk'] < best['pk']:
                groups[key] = row
        for key in sorted(groups, key=lambda k: (k[0], k[1], k[2])):
            row = groups[key]
            hits = [h for h in (hit_of(e) for e in row['hits']) if h is not None]
            if not hits:
                skipped += 1
                continue
            # AreaOrigin 0 spawns on the caster (offset forward/right), 1 at the
            # aim point; the rarer anchor codes are not modelled and stay out.
            if row['origin'] not in ORIGINS:
                print('%s %d: projectile %d uses AreaOrigin %d, left out' % (
                    asset, skill_id, row['pk'], row['origin']))
                skipped += 1
                continue
            hits.sort(key=lambda h: (h['trigger'] != 'CONTACT', h['atMs'], h['sourcePk']))
            out.append({
                'skillId': skill_id,
                'sourcePk': row['pk'],
                'clip': row['clip'],
                'startMs': int(round(row['t'] * 1000.0)),
                'kind': row['kind'],
                'origin': ORIGINS[row['origin']],
                'offsetForward': round(row['ox'] * UNITS_TO_METERS, 2),
                'offsetRight': round(row['oy'] * UNITS_TO_METERS, 2),
                'speed': round(row['speed'] * UNITS_TO_METERS, 2),
                'minDistance': round(row['mindist'] * UNITS_TO_METERS, 2),
                'maxDistance': round(row['maxdist'] * UNITS_TO_METERS, 2),
                'lifeMs': int(round(row['life'] * 1000.0)),
                'radius': round(row['radius'] * UNITS_TO_METERS, 2),
                'hits': hits,
            })
    return {
        'schema': 'lostark.animation-projectiles',
        'formatVersion': 1,
        'animationAssetId': asset,
        'characterClass': bindings['characterClass'],
        'projectiles': out,
    }, skipped


def main(argv):
    check = '--check' in argv
    assets = [a for a in argv if not a.startswith('--')]
    if not assets:
        raise SystemExit(__doc__)
    for asset in assets:
        document, skipped = build(asset)
        text = json.dumps(document, indent=2, ensure_ascii=False) + '\n'
        path = os.path.join(AUTH, asset, asset + '.projectiles.json')
        old = io.open(path, encoding='utf-8').read() if os.path.exists(path) else None
        print('%s: %d projectiles for %d skills (%d visual-only skipped), %s' % (
            asset, len(document['projectiles']), len({p['skillId'] for p in document['projectiles']}),
            skipped, 'unchanged' if old == text else 'changed'))
        if old != text and not check:
            io.open(path, 'w', encoding='utf-8', newline='\n').write(text)


if __name__ == '__main__':
    main(sys.argv[1:])
