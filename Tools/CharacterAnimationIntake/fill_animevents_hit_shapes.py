# -*- coding: utf-8 -*-
"""usage:
  <blender-python> fill_animevents_hit_shapes.py <Asset> [<Asset> ...] [--check]
"""
import io, os, re, struct, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
REF = os.path.join(REPO, 'Data', 'Animation', 'Reference')
AUTH = os.path.join(REPO, 'Data', 'Animation', 'Authored')


def f32(x):
    return struct.unpack('f', struct.pack('f', x))[0]


def to_ms(seconds):
    return int(f32(f32(f32(seconds) * f32(1000.0)) + f32(0.5)))


def read_lines(path):
    with io.open(path, 'rb') as f:
        data = f.read()
    return data.decode('latin-1').split('\n')


def parse_pairs(text):
    out = {}
    for m in re.finditer(r'(\w+)=("([^"]*)"|\S+)', text):
        out[m.group(1)] = m.group(3) if m.group(3) is not None else m.group(2)
    return out


def load_notify(asset):
    clips = {}
    order = []
    cur = None
    for line in read_lines(os.path.join(REF, asset, asset + '.animnotify'))[1:]:
        m = re.match(r'^"([^"]+)"', line)
        if m:
            cur = m.group(1)
            clips.setdefault(cur, [])
            order.append(cur)
            continue
        if cur is None or not line.startswith('  n '):
            continue
        p = parse_pairs(line[4:])
        if p.get('kind') != 'HIT':
            continue
        clips[cur].append((float(p['t']), float(p['d']), p.get('label', '')))
    return clips, order


def load_clipmap(asset):
    out = {}
    for line in read_lines(os.path.join(REF, asset, asset + '.clipmap'))[1:]:
        m = re.match(r'^"([^"]+)"(.*)$', line)
        if m:
            out[m.group(1)] = int(parse_pairs(m.group(2)).get('skill', '0'))
    return out


def load_clipseq(asset):
    chains = []
    for line in read_lines(os.path.join(REF, asset, asset + '.clipseq'))[1:]:
        m = re.match(r'^(\d+) "[^"]*"(.*)$', line)
        if not m:
            continue
        clips = [c for c in parse_pairs(m.group(2)).get('clips', '').split(',') if c]
        if clips:
            chains.append(clips)
    return chains


SHAPE_KEYS = ('rep', 'repms', 'fz', 'fzin', 'fzout', 'push', 'pushr',
              'area', 'ar', 'aa', 'ah', 'ax', 'arem', 'maxt')


def zero_shape():
    s = {k: 0 for k in SHAPE_KEYS}
    s['rep'] = 1
    return s


def load_skilltiming(asset):
    rows = []
    for line in read_lines(os.path.join(REF, asset, asset + '.skilltiming'))[1:]:
        m = re.match(r'^(\d+) "[^"]*"(.*)$', line)
        if m:
            p = parse_pairs(m.group(2))
            hits = []
            for span in p.get('hits', '').split(','):
                if '-' in span:
                    hits.append(zero_shape())
            rows.append({'id': int(m.group(1)), 'base': int(p.get('base', '0')),
                         'hits': hits, 'detail': False})
            continue
        m = re.match(r'^  (hit|shape) (.*)$', line)
        if m and rows:
            row = rows[-1]
            if not row['detail']:
                row['detail'] = True
                row['hits'] = []
            p = parse_pairs(m.group(2))
            s = zero_shape()
            for k in SHAPE_KEYS:
                if k in p:
                    s[k] = int(p[k])
            if s['rep'] < 1:
                s['rep'] = 1
            row['hits'].append(s)
    return rows


def find_reference_row(rows, skill_id):
    variant = None
    for row in rows:
        if not row['hits']:
            continue
        if row['id'] == skill_id:
            return row
        if variant is None and row['base'] == skill_id:
            variant = row
    return variant


def distinct_hits(hits):
    seen = []
    n = 0
    for h in hits:
        if h in seen:
            continue
        seen.append(h)
        n += 1
    return n


def preceding_chain_hits(chains, notify, clip):
    for chain in chains:
        count = 0
        for c in chain:
            if c == clip:
                return count
            count += distinct_hits(notify.get(c, []))
    return 0


def hit_row(clip, start, end, s):
    return ('"%s" HIT startms=%d endms=%d rep=%d repms=%d fz=%d fzin=%d fzout=%d '
            'push=%d pushr=%d area=%d ar=%d aa=%d ah=%d ax=%d arem=%d maxt=%d src=orig'
            % (clip, start, end, s['rep'], s['repms'], s['fz'], s['fzin'], s['fzout'],
               s['push'], s['pushr'], s['area'], s['ar'], s['aa'], s['ah'], s['ax'],
               s['arem'], s['maxt']))


def build_rows(asset):
    notify, order = load_notify(asset)
    clipmap = load_clipmap(asset)
    chains = load_clipseq(asset)
    timing = load_skilltiming(asset)
    generated = {}
    shaped = 0
    for clip in order:
        rows = notify.get(clip, [])
        if not rows:
            continue
        ref = find_reference_row(timing, clipmap.get(clip, 0)) if clip in clipmap else None
        ordinal = preceding_chain_hits(chains, notify, clip) if ref else 0
        added = []
        out = []
        for t, d, label in rows:
            start = to_ms(t)
            end = start + to_ms(d)
            key = (start, end, label)
            if key in added:
                continue
            added.append(key)
            s = zero_shape()
            if ref:
                src = ref['hits'][min(ordinal, len(ref['hits']) - 1)]
                ordinal += 1
                if src['area'] > 0:
                    s = dict(src)
                    shaped += 1
            out.append(hit_row(clip, start, end, s))
        generated[clip] = out
    return generated, shaped


def rewrite(asset, generated, check):
    path = os.path.join(AUTH, asset, asset + '.animevents')
    lines = read_lines(path)
    eol = '\r\n' if lines[0].endswith('\r') else '\n'
    lines = [l.rstrip('\r') for l in lines]
    if lines and lines[-1] == '':
        lines.pop()
    header = lines[0]
    m = re.match(r'^(LOSTARK_ANIM_EVENTS \d+ "[^"]+" )(\d+)$', header)
    if not m:
        raise SystemExit('%s: unexpected header %r' % (asset, header))
    body = lines[1:]
    orig_pat = re.compile(r'^"([^"]+)" HIT .* src=orig$')
    first_index = {}
    kept = []
    for i, line in enumerate(body):
        mm = orig_pat.match(line)
        if mm and mm.group(1) in generated:
            first_index.setdefault(mm.group(1), len(kept))
            kept.append(None)
        else:
            kept.append(line)
    out = []
    emitted = set()
    for i, line in enumerate(kept):
        if line is None:
            for clip, idx in first_index.items():
                if idx == i and clip not in emitted:
                    out.extend(generated[clip])
                    emitted.add(clip)
            continue
        out.append(line)
    for clip in generated:
        if clip not in emitted:
            out.extend(generated[clip])
            emitted.add(clip)
    new_header = '%s%d' % (m.group(1), len(out))
    text = eol.join([new_header] + out) + eol
    old_text = eol.join([header] + body) + eol
    changed = text != old_text
    print('%s: rows %s -> %d, HIT rows %d, %s' % (
        asset, m.group(2), len(out), sum(len(v) for v in generated.values()),
        'changed' if changed else 'unchanged'))
    if changed and not check:
        with io.open(path, 'wb') as f:
            f.write(text.encode('latin-1'))
    return changed


def main(argv):
    check = '--check' in argv
    assets = [a for a in argv if not a.startswith('--')]
    if not assets:
        raise SystemExit(__doc__)
    for asset in assets:
        generated, shaped = build_rows(asset)
        print('%s: %d clips with HIT notifies, %d rows shaped from skilltiming' % (
            asset, len(generated), shaped))
        rewrite(asset, generated, check)


if __name__ == '__main__':
    main(sys.argv[1:])
