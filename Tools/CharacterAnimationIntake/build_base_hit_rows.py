# -*- coding: utf-8 -*-
"""usage:
  python build_base_hit_rows.py --table-root <TableData dir> <Asset> [<Asset> ...] [--check]

Writes Data/Animation/Reference/<Asset>/<Asset>.basehits: the SkillEffect rows a
skill's own clips judge although their PK left the skill's decade
(34610 -> 346110..346113). A row belongs to the base skill when its damage
coefficients (ValueA, ValueB, ValueF, Key) equal a row the skill tooltip names.

A second row kind, `particle=`, covers the base chain's trailing ParticleHit
notifies. A ParticleHit carries no SkillEffect PK: the judgement lives in the
particle asset, and a tripod's Effect notify follows it by a few tens of ms only
in that tripod's group. When the base chain's last clip strikes after its last
Effect judgement, those strikes take the tail of the tooltip's damage list (the
rows no Effect notify of the chain already judges), and the row carries that
SkillEffect's shape so fill_animevents_hit_shapes.py stamps it instead of
dropping the notify. A clip without any Effect judgement keeps that script's
positional skilltiming rule and gets no particle row.

A `pull=` row names a base-chain Effect judgement whose SkillEffect PushType is 1:
the .animnotify extraction kept only the push range, so without it the source's
pull toward the caster became a push away.
"""
import io, os, re, sqlite3, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
REF = os.path.join(REPO, 'Data', 'Animation', 'Reference')
HEADER = 'LOSTARK_BASE_HIT_ROWS 1 "%s" %d'


def read_lines(path):
    with io.open(path, 'rb') as f:
        return f.read().decode('latin-1').split('\n')


def decode(value):
    if isinstance(value, bytes):
        for encoding in ('utf-8', 'cp949'):
            try:
                return value.decode(encoding)
            except UnicodeDecodeError:
                pass
    return value or ''


def base_chain_clips(asset):
    seqs = {}
    for line in read_lines(os.path.join(REF, asset, asset + '.clipseq'))[1:]:
        m = re.match(r'^(\d+) "[^"]*" seq=(\d+).*clips="([^"]*)"', line)
        if m:
            seqs.setdefault(int(m.group(1)), []).append((int(m.group(2)), m.group(3).split(',')))
    return {clip for chains in seqs.values() for clip in min(chains)[1] if clip}


def clip_effect_rows(asset):
    rows = []
    skill = None
    base_clips = base_chain_clips(asset)
    for line in read_lines(os.path.join(REF, asset, asset + '.animnotify'))[1:]:
        m = re.match(r'^"([^"]+)" skill=(\d+)', line)
        if m:
            skill = int(m.group(2)) if m.group(1) in base_clips else None
            continue
        if skill is None or 'kind=HIT src=Effect' not in line:
            continue
        pk = re.search(r'asset="(\d+)"', line)
        area = re.search(r' area=(\d+)', line)
        if pk and area and int(area.group(1)) > 0:
            rows.append((skill, int(pk.group(1))))
    return rows


def build(asset, effects, messages):
    def values(pk):
        return effects.execute(
            'SELECT ValueA, ValueB, ValueF, Key FROM SkillEffect WHERE PrimaryKey = ? '
            'ORDER BY ABS(SecondaryKey - 10) LIMIT 1', (pk,)).fetchone()

    def tooltip_rows(skill):
        row = messages.execute('SELECT MSG FROM GameMsg WHERE KEY = ?',
                               ('tip.desc.skill_%d' % skill,)).fetchone()
        return [int(pk) for pk in re.findall(r'@1:(\d+)', decode(row[0]) if row else '')]

    accepted = {}
    for skill, pk in clip_effect_rows(asset):
        if pk // 10 == skill or pk in accepted.get(skill, {}):
            continue
        named = tooltip_rows(skill)
        if pk in named:
            accepted.setdefault(skill, {})[pk] = pk
            continue
        mine = values(pk)
        match = next((n for n in named if mine is not None and values(n) == mine), None)
        if match is not None:
            accepted.setdefault(skill, {})[pk] = match
    lines = []
    for skill in sorted(accepted):
        for pk in sorted(accepted[skill]):
            lines.append('%d pk=%d match=%d' % (skill, pk, accepted[skill][pk]))
    return [HEADER % (asset, len(lines))] + lines


PARTICLE_PAIR_SECONDS = 0.06
SHAPE_COLUMNS = ('AreaType', 'AreaRange', 'AreaAngle', 'AreaHeight', 'AreaOffsetX',
                 'AreaRemoveRange', 'MaxAmount', 'PushMinTime', 'PushMinRange',
                 'FreezeTime', 'FreezeBlendInTime', 'FreezeBlendOutTime')
SHAPE_FIELDS = ('area', 'ar', 'aa', 'ah', 'ax', 'arem', 'maxt', 'push', 'pushr',
                'fz', 'fzin', 'fzout')


def to_ms(seconds):
    return int(round(seconds * 1000.0))


def base_chains(asset):
    seqs = {}
    for line in read_lines(os.path.join(REF, asset, asset + '.clipseq'))[1:]:
        m = re.match(r'^(\d+) "[^"]*" seq=(\d+).*clips="([^"]*)"', line)
        if m:
            seqs.setdefault(int(m.group(1)), []).append((int(m.group(2)), m.group(3).split(',')))
    return {skill: [c for c in min(chains)[1] if c] for skill, chains in seqs.items()}


def chain_hit_notifies(asset, chains):
    """(clip, t, src, pk) of every HIT notify on a base chain, in chain order."""
    by_clip = {}
    clip = None
    for line in read_lines(os.path.join(REF, asset, asset + '.animnotify'))[1:]:
        m = re.match(r'^"([^"]+)"', line)
        if m:
            clip = m.group(1)
            continue
        if clip is None or ' kind=HIT ' not in line:
            continue
        src = re.search(r' src=(\w+)', line)
        t = re.search(r' t=([\d.]+)', line)
        pk = re.search(r' asset="(\d+)"', line)
        area = re.search(r' area=(\d+)', line)
        if src.group(1) == 'Effect' and not (pk and area and int(area.group(1)) > 0):
            continue
        by_clip.setdefault(clip, []).append(
            (clip, float(t.group(1)), src.group(1), int(pk.group(1)) if pk else 0))
    return {skill: [row for c in clips for row in by_clip.get(c, [])]
            for skill, clips in chains.items()}


def load_repeats(asset):
    out = {}
    path = os.path.join(REF, asset, asset + '.hitrepeats')
    if not os.path.exists(path):
        return out
    for line in read_lines(path)[1:]:
        m = re.match(r'^pk=(\d+) t=(\d+) rep=(\d+)', line)
        if m:
            out[(int(m.group(1)), int(m.group(2)))] = int(m.group(3))
    return out


def particle_rows(asset, effects, messages):
    def effect_row(pk, columns):
        return effects.execute(
            'SELECT %s FROM SkillEffect WHERE PrimaryKey = ? ORDER BY ABS(SecondaryKey - 10) LIMIT 1'
            % ', '.join(columns), (pk,)).fetchone()

    def tooltip_rows(skill):
        row = messages.execute('SELECT MSG FROM GameMsg WHERE KEY = ?',
                               ('tip.desc.skill_%d' % skill,)).fetchone()
        return [int(pk) for pk in re.findall(r'@1:(\d+)', decode(row[0]) if row else '')]

    def judged_by(pk, left):
        if pk in left:
            return pk
        for columns in (('ValueA', 'ValueB', 'ValueF', 'Key'), SHAPE_COLUMNS[:5]):
            mine = effect_row(pk, columns)
            match = next((n for n in left if mine is not None and effect_row(n, columns) == mine), None)
            if match is not None:
                return match
        return None

    chains = base_chains(asset)
    notifies = chain_hit_notifies(asset, chains)
    repeats = load_repeats(asset)
    lines = []
    for skill in sorted(chains):
        rows = notifies.get(skill, [])
        left = tooltip_rows(skill)
        effect_hits = [(c, t, pk) for c, t, src, pk in rows if src == 'Effect']
        for clip, t, pk in effect_hits:
            target = judged_by(pk, left)
            for _ in range(repeats.get((pk, to_ms(t)), 1)):
                if target in left:
                    left.remove(target)
        judged_clips = {c for c, _, _ in effect_hits}
        particles = [(c, t) for c, t, src, _ in rows if src == 'ParticleHit' and c in judged_clips and
                     not any(c == ec and 0.0 <= et - t <= PARTICLE_PAIR_SECONDS for ec, et, _ in effect_hits)]
        if not particles or not left:
            continue
        if len(particles) != len(left):
            last = chains[skill][-1]
            last_effect = max([t for c, t, _ in effect_hits if c == last], default=-1.0)
            tail = [(c, t) for c, t in particles if c == last and t > last_effect]
            if not tail or len(tail) > len(left):
                print('%s %d: %d unpaired ParticleHit vs %d unjudged tooltip rows, no trailing tail; left out' % (
                    asset, skill, len(particles), len(left)))
                continue
            print('%s %d: %d unpaired ParticleHit vs %d unjudged tooltip rows; %d trailing strikes of %s take %s, the rest keep the skilltiming guess' % (
                asset, skill, len(particles), len(left), len(tail), last, left[-len(tail):]))
            particles, left = tail, left[-len(tail):]
        for (clip, t), pk in zip(particles, left):
            shape = effect_row(pk, SHAPE_COLUMNS)
            if shape is None or shape[0] <= 0:
                print('%s %d: tooltip row %d has no shape; left out' % (asset, skill, pk))
                continue
            lines.append('%d particle="%s" t=%d pk=%d %s' % (
                skill, clip, to_ms(t), pk,
                ' '.join('%s=%d' % (f, v) for f, v in zip(SHAPE_FIELDS, shape))))
    return lines


def pull_rows(asset, effects):
    chains = base_chains(asset)
    lines = []
    for skill in sorted(chains):
        for clip, t, src, pk in chain_hit_notifies(asset, chains).get(skill, []):
            if src != 'Effect':
                continue
            row = effects.execute(
                'SELECT PushType FROM SkillEffect WHERE PrimaryKey = ? ORDER BY ABS(SecondaryKey - 10) LIMIT 1',
                (pk,)).fetchone()
            if row is not None and row[0] == 1:
                lines.append('%d pull=%d' % (skill, pk))
    return sorted(set(lines), key=lambda l: (int(l.split()[0]), l))


def main(argv):
    check = '--check' in argv
    if '--table-root' not in argv:
        raise SystemExit(__doc__)
    root = argv[argv.index('--table-root') + 1]
    assets = [a for i, a in enumerate(argv)
              if not a.startswith('--') and (i == 0 or argv[i - 1] != '--table-root')]
    effects = sqlite3.connect(os.path.join(root, 'EFTable_SkillEffect.db'))
    messages = sqlite3.connect(os.path.join(root, 'EFTable_GameMsg.db'))
    for asset in assets:
        rows = build(asset, effects, messages)
        rows += particle_rows(asset, effects, messages)
        rows += pull_rows(asset, effects)
        rows[0] = HEADER % (asset, len(rows) - 1)
        text = '\n'.join(rows) + '\n'
        path = os.path.join(REF, asset, asset + '.basehits')
        old = io.open(path, 'rb').read().decode('utf-8') if os.path.exists(path) else None
        changed = old != text
        print('%s: %d rows, %s' % (asset, text.count('\n') - 1, 'changed' if changed else 'unchanged'))
        if changed and not check:
            with io.open(path, 'wb') as f:
                f.write(text.encode('utf-8'))


if __name__ == '__main__':
    main(sys.argv[1:])
