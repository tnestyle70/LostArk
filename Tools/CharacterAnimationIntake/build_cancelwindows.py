# -*- coding: utf-8 -*-
"""usage:
  <blender-python> build_cancelwindows.py <Asset> [<Asset> ...] [--check]
"""
import io, json, os, re, struct, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
RESOURCES = os.path.join(REPO, 'Client', 'Bin', 'Resources')
TICK_RATE = 30.0
MAX_WINDOWS = 8

# The animators labelled the windows in Korean and the vocabulary drifted between
# classes: LanceMaster and DimensionMaster leave the generic window's payload
# empty, Artist spells out both kinds, Warlord writes 입력캔슬 for the shared one.
# Buffered pre-input (선*), dodge-only and tripod/stance names are not cancels.
SKILL_CANCEL_PAYLOADS = frozenset(
    ('', '스킬캔슬', '[스킬캔슬]', '입력캔슬', '[입력캔슬]'))
MOVE_CANCEL_PAYLOADS = frozenset(
    ('', '이동캔슬', '[이동캔슬]', '입력캔슬', '[입력캔슬]'))


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


def read_cancel_rows(asset):
    path = os.path.join(REPO, 'Data', 'Animation', 'Authored', asset, asset + '.animevents')
    rows = {}
    for line in io.open(path, encoding='utf-8').read().split('\n'):
        m = re.match(r'^"([^"]+)" CANCEL (.*)$', line.rstrip('\r'))
        if not m:
            continue
        rest = m.group(2)
        payload = re.search(r'payload="([^"]*)"', rest)
        fields = dict(kv.split('=', 1) for kv in rest.split() if '=' in kv and not kv.startswith('payload='))
        if 'startms' not in fields or 'endms' not in fields:
            continue
        start_ms = int(fields['startms'])
        end_ms = int(fields['endms'])
        if end_ms <= start_ms:
            continue
        text = payload.group(1) if payload else ''
        rows.setdefault(m.group(1), []).append({
            'startMs': start_ms,
            'endMs': end_ms,
            'skill': text in SKILL_CANCEL_PAYLOADS,
            'move': text in MOVE_CANCEL_PAYLOADS,
        })
    return rows


def merge(windows, limit_ms):
    """Overlapping and touching windows collapse: the server only asks whether the
    action clock is inside one, so the count is presentation, not meaning."""
    clamped = []
    for start_ms, end_ms in windows:
        start_ms = max(0, min(int(round(start_ms)), limit_ms))
        end_ms = max(0, min(int(round(end_ms)), limit_ms))
        if end_ms > start_ms:
            clamped.append((start_ms, end_ms))
    clamped.sort()
    out = []
    for start_ms, end_ms in clamped:
        if out and start_ms <= out[-1][1]:
            out[-1][1] = max(out[-1][1], end_ms)
        else:
            out.append([start_ms, end_ms])
    return out


def stage_windows(entries, clip_ticks, clip_cancels, limit_ms, label):
    """Clip-local windows laid onto the stage clock the same way the hit shapes
    are: each clip contributes its own span, shortened by playMs and divided by
    playRate, and the next clip starts where the previous one stopped."""
    skill_windows = []
    move_windows = []
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
        for row in clip_cancels.get(name, []):
            # A window that opens after the clip is cut short never happens.
            if row['startMs'] >= source_ms:
                continue
            start_ms = elapsed_ms + row['startMs'] / rate
            end_ms = elapsed_ms + min(row['endMs'], source_ms) / rate
            if row['skill']:
                skill_windows.append((start_ms, end_ms))
            if row['move']:
                move_windows.append((start_ms, end_ms))
        elapsed_ms += source_ms / rate
    skill_windows = merge(skill_windows, limit_ms)
    move_windows = merge(move_windows, limit_ms)
    for kind, windows in (('skill', skill_windows), ('move', move_windows)):
        if len(windows) > MAX_WINDOWS:
            raise SystemExit('%s: more than %d %s cancel windows' % (label, MAX_WINDOWS, kind))
    return skill_windows, move_windows


def build(asset):
    catalog = json.load(io.open(os.path.join(REPO, 'Data', 'Actors', 'CharacterCatalog.json'), encoding='utf-8'))
    entry = next(c for c in catalog['characters'] if c['assetId'] == asset)
    clip_ticks = read_clip_ticks(os.path.join(RESOURCES, *entry['bodyModel'].split('/')))
    bindings = json.load(io.open(os.path.join(REPO, 'Data', 'Animation', 'Authored', asset, asset + '.skillbindings.json'), encoding='utf-8'))
    balance = json.load(io.open(os.path.join(REPO, 'Data', 'Balance', 'PlayerSkills.json'), encoding='utf-8'))
    skills = {int(s['skillId']): s for s in balance['skills'] if s['characterClass'] == bindings['characterClass']}
    clip_cancels = read_cancel_rows(asset)
    out = []
    for binding in sorted(bindings['bindings'], key=lambda b: int(b['skillId'])):
        skill_id = int(binding['skillId'])
        skill = skills.get(skill_id)
        if skill is None:
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
                skill_windows, move_windows = stage_windows(
                    group, clip_ticks, clip_cancels, limit_ms, label)
                if skill_windows or move_windows:
                    rows.append({
                        'stageIndex': index,
                        'skillCancel': skill_windows,
                        'moveCancel': move_windows,
                    })
            if rows:
                out.append({'skillId': skill_id, 'stages': rows})
            continue
        label = '%s %d' % (asset, skill_id)
        limit_ms = int(skill['actionDurationMs'])
        skill_windows, move_windows = stage_windows(
            stages[0], clip_ticks, clip_cancels, limit_ms, label)
        if skill_windows or move_windows:
            out.append({
                'skillId': skill_id,
                'skillCancel': skill_windows,
                'moveCancel': move_windows,
            })
    return {
        'schema': 'lostark.animation-cancel-windows',
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
    out_dir = os.path.join(REPO, 'Data', 'Animation', 'CancelWindows')
    os.makedirs(out_dir, exist_ok=True)
    for asset in assets:
        document = build(asset)
        text = json.dumps(document, indent=2, ensure_ascii=False) + '\n'
        path = os.path.join(out_dir, asset + '.cancelwindows.json')
        old = io.open(path, encoding='utf-8').read() if os.path.exists(path) else None
        skill_count = len(document['skills'])
        window_count = sum(
            len(s.get('skillCancel', [])) + len(s.get('moveCancel', [])) +
            sum(len(st['skillCancel']) + len(st['moveCancel']) for st in s.get('stages', []))
            for s in document['skills'])
        print('%s: %d skills, %d windows, %s' % (
            asset, skill_count, window_count, 'unchanged' if old == text else 'changed'))
        if old != text and not check:
            io.open(path, 'w', encoding='utf-8', newline='\n').write(text)


if __name__ == '__main__':
    main(sys.argv[1:])
