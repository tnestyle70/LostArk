"""Prepare field-scoped Kouku raid connections from the latest saved documents.

This command only writes an out candidate. Installation is a separate CAS transaction.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path

ACTION = Path('Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json')
SEQUENCE = Path('Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json')
WORLD = Path('Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json')
PREFIX = 'KAKULSAYDON_G1_PATTERN_'


def prepare(action, sequence, world):
    action, sequence, world = copy.deepcopy((action, sequence, world))
    patterns = {int(p['patternId'].removeprefix(PREFIX)): p for p in action['patterns']}
    logics = {r['logicId']: r for r in action['logics']}
    def define(identity, **fields):
        row = next((r for r in action['logics'] if r.get('displayName') == fields.get('displayName') and r.get('logicType') == fields.get('logicType')), None)
        if row is None:
            identity = f"kakulsaydon.g1.logic.{action['nextLogicOrdinal']}"
            action['nextLogicOrdinal'] += 1
            row = {'logicId': identity}
            action['logics'].append(row)
            logics[identity] = row
        row.update(fields)
        return row
    def original(number):
        return logics[f'kakulsaydon.g1.logic.{number}']
    def result(identity, label, kind, **fields):
        return define(identity, displayName=label, logicType='RESULT', outcomeKind=kind,
                      percent=0, durationMs=0, followupPatternId=fields.pop('followupPatternId', ''), **fields)

    original(82).update(judgementKind='COUNTER_WINDOW', endsPatternOnSuccess=True)
    counter = result('kakulsaydon.raid.counter.success', '세이튼_돌진카운터_그로기', 'FOLLOWUP_PATTERN',
                     followupPatternId=PREFIX+'4')['logicId']
    for box in patterns[80]['logicOccurrences']:
        if box['logicId'] == original(82)['logicId']:
            box['onSuccessLogicIds'] = [counter]
    original(86)['judgementKind'] = 'CARD_DICE_BIND'
    # The authored Duration owns the full bind interval. Retain the duplicate Trigger as a disabled draft.
    for box in patterns[78]['logicOccurrences']:
        if box['logicId'] == original(87)['logicId']:
            box['enabled'] = False
    resources = {r['resourceId']: r for r in action['presentationResources']}
    maiden = [r for r in patterns[33]['presentationOccurrences'] if r.get('anchorKind') == 'MAP'
              and '아이언메이든' in resources[r['resourceId']]['displayName']]
    if len(maiden) != 1:
        raise ValueError('Mario phase 2 must have exactly one authored MAP Iron Maiden occurrence')
    original(89).update(triggerKind='MARIO_PHASE2_PLAYERS', teleportPosition=list(maiden[0]['positionOffset']))
    original(90).update(triggerKind='BOSS_TELEPORT_FACE_CENTER', teleportPosition=[5.96, 1.30, 950.59])
    placements = {r['placementId']: r for r in world['placements']}
    original(88).update(triggerKind='BOSS_TELEPORT_XZ', teleportPosition=list(placements['boss.kakulsaydon.g2.kouku']['position']))
    wipe = result('kakulsaydon.raid.mario.failure', '마리오_실패_전원즉사', 'INSTANT_DEATH')['logicId']
    for number, stage in ((88, 1), (91, 2), (92, 3), (93, 4)):
        enter = result(f'kakulsaydon.raid.mario.enter.{stage}', f'마리오_{stage}단계_입장', 'MARIO_ENTER',
                       marioStage=stage, followupPatternId=PREFIX+'33')['logicId']
        boxes = [b for b in patterns[number]['logicOccurrences'] if b['logicId'] == original(59)['logicId']]
        if len(boxes) != 1:
            raise ValueError(f'{number}: expected exactly one authored Mario entry Trigger')
        boxes[0]['onSuccessLogicIds'] = [enter]
        boxes[0]['onTimeoutLogicIds'] = [wipe]
    # Preserve the older completion-driven Mario entry as an independently playable pattern.
    for box in patterns[34]['logicOccurrences']:
        if box['logicId'] == original(59)['logicId']:
            box['onTimeoutLogicIds'] = [wipe]
    for box in patterns[33]['logicOccurrences']:
        if box['logicId'] == original(56)['logicId']:
            box['onTimeoutLogicIds'] = [wipe]

    flows = {row['gateId']: row for row in action['patternFlows']}
    def flow(gate, targets):
        row = flows[gate]
        existing = {(r['kind'], r['targetId']): r for r in row['entries']}
        used_ids = {r['entryId'] for r in row['entries']}
        new = []
        for i, (kind, target) in enumerate(targets):
            entry = existing.get((kind, target))
            if entry is None:
                ordinal = i + 1
                while f'{row["flowId"]}.raid.{ordinal}' in used_ids:
                    ordinal += 1
                entry_id = f'{row["flowId"]}.raid.{ordinal}'
                used_ids.add(entry_id)
                entry = {'entryId': entry_id, 'kind': kind, 'targetId': target, 'waitAfterMs': 1000}
            entry = copy.deepcopy(entry)
            entry['waitAfterMs'] = 0 if i == len(targets)-1 else entry['waitAfterMs']
            new.append(entry)
        row['entries'] = new
    def p(n): return ('PATTERN', PREFIX+str(n))
    def b(n): return ('BUNDLE', f'kakulsaydon.bundle.{n}')
    flow('GATE1', [p(n) for n in (1, 2, 6, 7, 29, 30, 47, 48, 58, 78, 79, 80, 81, 82, 83)])
    flow('GATE2', [b(1), b(2), b(3), b(6), b(7), p(21), b(9), b(10), p(27), p(85), p(86), p(87), b(4), p(25)])
    flow('GATE3', [p(n) for n in (38, 39, 40, 41, 43, 46, 49, 59, 52, 65, 66, 88, 91, 35, 92, 93)])
    # Initial boss templates stay dormant until the Server-owned cinematic handoff.
    placements['boss.kakulsaydon.g1.kouku']['enabled'] = False

    seq_patterns = {p['patternId']: p for p in sequence['patterns']}
    arrival_logic = next(r['logicId'] for r in sequence['logics'] if r.get('triggerKind') == 'ROOM_PLAYER_ARRIVAL')
    for number, center in ((3, (3.38, 10.56, 323.92)), (7, (-2.45, 1.32, 945.17))):
        pattern = seq_patterns[PREFIX+str(number)]
        boxes = pattern.setdefault('logicOccurrences', [])
        slots = {r['roomPlayerArrival']['playerSlot'] for r in boxes if r.get('enabled', True) and 'roomPlayerArrival' in r}
        duration = pattern.get('durationMs', 0) or sum(s['durationMs'] for s in pattern['stages'])
        for slot in range(4):
            if slot in slots:
                continue
            ordinal = pattern.get('nextLogicOccurrenceOrdinal', 1)
            pattern['nextLogicOccurrenceOrdinal'] = ordinal + 1
            boxes.append({'occurrenceId': f'{pattern["patternId"]}.logic.{ordinal}', 'logicId': arrival_logic,
                          'startMs': duration-1, 'durationMs': 1, 'enabled': True,
                          'onSuccessLogicIds': [], 'onFailLogicIds': [], 'onTimeoutLogicIds': [],
                          'roomPlayerArrival': {'playerSlot': slot, 'position': [center[0]+slot*0.85, center[1], center[2]]}})
    return action, sequence, world


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    paths = (ACTION, SEQUENCE, WORLD)
    sources = [(args.root/p).read_bytes() for p in paths]
    originals = [json.loads(value.decode('utf-8-sig')) for value in sources]
    candidates = prepare(*originals)
    manifest = {'files': []}
    for path, raw, before, after in zip(paths, sources, originals, candidates):
        if after != before:
            after['revision'] = before['revision']+1
        output = args.out/'candidate'/path
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(after, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
        manifest['files'].append({'path': path.as_posix(), 'baselineSha256': hashlib.sha256(raw).hexdigest(),
                                  'baselineRevision': before['revision'], 'candidateRevision': after['revision']})
    (args.out/'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n', encoding='utf-8')
    print(json.dumps(manifest))


if __name__ == '__main__':
    main()
