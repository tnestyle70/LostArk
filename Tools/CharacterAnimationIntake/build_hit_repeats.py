# -*- coding: utf-8 -*-
"""usage:
  python build_hit_repeats.py --action-root <XmlData/Action dir> <Asset> [<Asset> ...] [--check]

Writes Data/Animation/Reference/<Asset>/<Asset>.hitrepeats: the repeat count and
interval every CEFActionNotify_Effect carries after its timing floats (mark end
+32 int32 count, +36 float seconds), keyed by SkillEffect PK and start ms. The
.animnotify extraction never read them, so a notify that strikes N times reached
the Server as one hit.
"""
import io, os, re, struct, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
REF = os.path.join(REPO, 'Data', 'Animation', 'Reference')
ACTION_FILES = {
    'LanceMaster': 'LANCEMASTER.loa',
    'Warlord': 'GUNLANCER.loa',
    'Artist': 'YINYANGSHI.loa',
    'DimensionMaster': 'DIMENSIONMASTER.loa',
    'GuardianKnight': 'DRAGONKNIGHT.loa',
}
EFFECT_MARK = b'CEFActionNotify_Effect\x00'
HEADER = 'LOSTARK_HIT_REPEATS 1 "%s" %d'


def to_ms(seconds):
    return int(round(seconds * 1000.0))


def effect_pk(data, body):
    at = body + 12
    for _ in range(2):
        size = struct.unpack_from('<i', data, at)[0]
        span = -size * 2 if size < 0 else size
        if not 0 <= span <= 256:
            return 0
        at += 4 + span
    return struct.unpack_from('<i', data, at + 12)[0]


def read_repeats(path):
    data = open(path, 'rb').read()
    found = {}
    for m in re.finditer(re.escape(EFFECT_MARK), data):
        if struct.unpack_from('<i', data, m.start() - 4)[0] != len(EFFECT_MARK):
            continue
        end = m.end()
        base_row = struct.unpack_from('<i', data, end + 12)[0] == 1
        start = struct.unpack_from('<f', data, end + 16)[0]
        count = struct.unpack_from('<i', data, end + 32)[0]
        interval = struct.unpack_from('<f', data, end + 36)[0]
        pk = effect_pk(data, end + 28)
        if pk <= 0 or count < 2 or not 0.0 < interval < 30.0:
            continue
        key = (pk, to_ms(start))
        # The first base-flagged record is the skill's default action group;
        # later groups of the same PK are its tripod variants.
        if key not in found or (base_row and not found[key][1]):
            found[key] = ((count, to_ms(interval)), base_row)
    return {key: value for key, (value, _) in found.items()}


def main(argv):
    check = '--check' in argv
    if '--action-root' not in argv:
        raise SystemExit(__doc__)
    root = argv[argv.index('--action-root') + 1]
    assets = [a for i, a in enumerate(argv)
              if not a.startswith('--') and (i == 0 or argv[i - 1] != '--action-root')]
    for asset in assets:
        repeats = read_repeats(os.path.join(root, ACTION_FILES[asset]))
        lines = ['pk=%d t=%d rep=%d repms=%d' % (pk, t, count, interval)
                 for (pk, t), (count, interval) in sorted(repeats.items())]
        text = '\n'.join([HEADER % (asset, len(lines))] + lines) + '\n'
        path = os.path.join(REF, asset, asset + '.hitrepeats')
        old = io.open(path, 'rb').read().decode('utf-8') if os.path.exists(path) else None
        changed = old != text
        print('%s: %d repeating notifies, %s' % (asset, len(lines), 'changed' if changed else 'unchanged'))
        if changed and not check:
            with io.open(path, 'wb') as f:
                f.write(text.encode('utf-8'))


if __name__ == '__main__':
    main(sys.argv[1:])
