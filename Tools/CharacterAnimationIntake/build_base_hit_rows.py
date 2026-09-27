# -*- coding: utf-8 -*-
"""usage:
  python build_base_hit_rows.py --table-root <TableData dir> <Asset> [<Asset> ...] [--check]

Writes Data/Animation/Reference/<Asset>/<Asset>.basehits: the SkillEffect rows a
skill's own clips judge although their PK left the skill's decade
(34610 -> 346110..346113). A row belongs to the base skill when its damage
coefficients (ValueA, ValueB, ValueF, Key) equal a row the skill tooltip names.
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
        text = '\n'.join(build(asset, effects, messages)) + '\n'
        path = os.path.join(REF, asset, asset + '.basehits')
        old = io.open(path, 'rb').read().decode('utf-8') if os.path.exists(path) else None
        changed = old != text
        print('%s: %d rows, %s' % (asset, text.count('\n') - 1, 'changed' if changed else 'unchanged'))
        if changed and not check:
            with io.open(path, 'wb') as f:
                f.write(text.encode('utf-8'))


if __name__ == '__main__':
    main(sys.argv[1:])
