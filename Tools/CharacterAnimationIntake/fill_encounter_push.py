# -*- coding: utf-8 -*-
"""usage:
  <blender-python> fill_encounter_push.py <EFTable_SkillEffect.db> [--check]

Stamps the official player push and knockdown of every Valtan pattern into the
stage rows of Data/Encounters/Valtan/ValtanEncounter.json. A pattern's
representative row is the HIT-notified SkillEffect row of its sourceActionIds
with the largest |PushMinRange| (lowest PK on a tie); damaging stages carry it,
non-damaging stages carry zeros so the stage schema stays uniform. --check
verifies the stamped fields without writing.
"""
import io, json, os, re, sqlite3, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
NOTIFY = os.path.join(REPO, 'Data', 'Animation', 'Reference', 'Valtan', 'Valtan.animnotify')
ENCOUNTER = os.path.join(REPO, 'Data', 'Encounters', 'Valtan', 'ValtanEncounter.json')
UNITS_TO_METERS = 0.01
DEFAULT_PUSH_MS = 150
DEFAULT_DOWN_MS = 2000


def load_action_hit_pks():
    actions = {}
    current = None
    with io.open(NOTIFY, 'rb') as f:
        lines = f.read().decode('utf-8').split('\n')
    for line in lines:
        header = re.match(r'^"[^"]+" skill=(\d+) ', line)
        if header:
            current = int(header.group(1))
            actions.setdefault(current, set())
            continue
        if current is None:
            continue
        hit = re.match(r'^\s+n .*kind=HIT src=Effect asset="(\d{8})"', line)
        if hit:
            actions[current].add(int(hit.group(1)))
    return actions


def load_push_rows(db_path, pks):
    rows = {}
    con = sqlite3.connect(db_path)
    cur = con.cursor()
    for pk in sorted(pks):
        cur.execute(
            'SELECT PushMinRange, PushMinTime, FallDown, HitTypeDownMin '
            'FROM SkillEffect WHERE PrimaryKey = ?', (pk,))
        row = cur.fetchone()
        if row is not None:
            rows[pk] = {'push': int(row[0]), 'pushMs': int(row[1]),
                        'fallDown': int(row[2]) != 0, 'downMs': int(row[3])}
    con.close()
    return rows


def representative(action_ids, action_pks, push_rows):
    best_pk = None
    for action in action_ids:
        for pk in sorted(action_pks.get(action, ())):
            row = push_rows.get(pk)
            if row is None or (0 == row['push'] and not row['fallDown']):
                continue
            if best_pk is None:
                best_pk = pk
                continue
            best = push_rows[best_pk]
            if abs(row['push']) > abs(best['push']) or (
                    abs(row['push']) == abs(best['push']) and pk < best_pk):
                best_pk = pk
    if best_pk is None:
        return None
    row = push_rows[best_pk]
    push_ms = row['pushMs'] if row['pushMs'] > 0 else (
        DEFAULT_PUSH_MS if 0 != row['push'] else 0)
    down_ms = row['downMs'] if row['downMs'] > 0 else (
        DEFAULT_DOWN_MS if row['fallDown'] else 0)
    return {'pk': best_pk,
            'pushRangeM': round(row['push'] * UNITS_TO_METERS, 4),
            'pushMs': push_ms,
            'knockdown': row['fallDown'],
            'downMs': down_ms}


ZERO = {'pushRangeM': 0.0, 'pushMs': 0, 'knockdown': False, 'downMs': 0}
FIELD_PATTERN = re.compile(
    r', "pushRangeM": [-0-9.]+, "pushMs": \d+, "knockdown": (?:true|false), "downMs": \d+')


def format_fields(values):
    range_text = ('%.4f' % values['pushRangeM']).rstrip('0').rstrip('.')
    if '.' not in range_text and 'e' not in range_text:
        range_text += '.0'
    return (', "pushRangeM": %s, "pushMs": %d, "knockdown": %s, "downMs": %d'
            % (range_text, values['pushMs'],
               'true' if values['knockdown'] else 'false', values['downMs']))


def main():
    args = [a for a in sys.argv[1:] if a != '--check']
    check_only = '--check' in sys.argv[1:]
    if len(args) != 1:
        sys.stderr.write(__doc__)
        return 2
    db_path = args[0]
    if not os.path.isfile(db_path):
        sys.stderr.write('missing SkillEffect db: %s\n' % db_path)
        return 2

    action_pks = load_action_hit_pks()
    all_pks = set()
    for pks in action_pks.values():
        all_pks.update(pks)
    push_rows = load_push_rows(db_path, all_pks)

    with io.open(ENCOUNTER, 'rb') as f:
        raw = f.read().decode('utf-8')
    document = json.loads(raw)
    pattern_values = {}
    for pattern in document['patterns']:
        values = representative(
            pattern.get('sourceActionIds', ()), action_pks, push_rows)
        pattern_values[pattern['patternId']] = values

    lines = raw.split('\n')
    out = []
    current_pattern = None
    stamped = 0
    drift = 0
    for line in lines:
        m = re.search(r'"patternId": "([^"]+)"', line)
        if m:
            current_pattern = m.group(1)
        if '"stageId"' in line and current_pattern is not None:
            values = pattern_values.get(current_pattern)
            damaging = re.search(r'"serverDamageProfileId": "[^"]+"', line)
            fields = format_fields(
                values if (values and damaging) else ZERO)
            stripped = FIELD_PATTERN.sub('', line)
            replaced = re.sub(
                r'(, "serverDamageProfileId": "[^"]*")',
                r'\1' + fields.replace('\\', '\\\\'), stripped, count=1)
            if replaced == stripped:
                sys.stderr.write('stage line without damage profile field: %s\n'
                                 % line.strip()[:80])
                return 1
            if replaced != line:
                drift += 1
            stamped += 1
            out.append(replaced)
            continue
        out.append(line)

    used = sorted((p, v['pk']) for p, v in pattern_values.items() if v)
    for pattern_id, pk in used:
        print('%s <- %d %s' % (pattern_id, pk,
                               format_fields(pattern_values[pattern_id]).strip(', ')))
    print('patterns with push: %d / %d, stages stamped: %d, changed: %d'
          % (len(used), len(pattern_values), stamped, drift))

    if check_only:
        return 1 if drift else 0
    if drift:
        with io.open(ENCOUNTER, 'wb') as f:
            f.write('\n'.join(out).encode('utf-8'))
    return 0


if __name__ == '__main__':
    sys.exit(main())
