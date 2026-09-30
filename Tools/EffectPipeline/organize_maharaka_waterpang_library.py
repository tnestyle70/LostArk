"""Stage or install Waterpang names without replacing unrelated authoring fields.

Stage first; apply only after the editor's saved-disk handoff. Source restoration
is installed separately before apply. Every write is guarded, backed up, atomic,
and rolled back only if it is still this transaction's own value.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path

from install_kouku_effect_library import replace_root_value
from sync_kouku_effect_tree import stage_project_metadata

ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / 'Data/Effects/Authored'
TREE = ROOT / 'Data/Effects/EffectResourceTree.json'
CATALOG = ROOT / 'Data/Effects/EffectCatalog.json'
WORLD = ROOT / 'Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.worldsequences.json'
PREFIX = 'effect.maharaka.'
NAMES = {
    'waterpang.cannon.jet.full.restore': ('hazards', '중앙 기둥 물 뿜기'),
    'waterpang.cannon.telegraph': ('hazards', '중앙 기둥 물 뿜기 예고'),
    'waterpang.cannon.att_battle_3_01.full.restore': ('hazards', '중앙 기둥 분사 시작'),
    'waterpang.cannon.att_battle_3_04.full.restore': ('hazards', '중앙 기둥 분사 종료'),
    'waterpang.cannon.att_battle_3_02.part0.full.restore': ('hazards', '중앙 기둥 회전 물결'),
    'waterpang.cannon.att_battle_3_02.part1.full.restore': ('hazards', '중앙 기둥 회전 분사 1'),
    'waterpang.cannon.att_battle_3_02.part2.full.restore': ('hazards', '중앙 기둥 회전 분사 2'),
    'waterpang.cannon.att_battle_3_02.part3.full.restore': ('hazards', '중앙 기둥 회전 분사 3'),
    'waterpang.mokomoko.telegraph': ('hazards', '중앙 바닥 장판'),
    'waterpang.mokomoko.ground.att_battle_1_01.full.restore': ('hazards', '중앙 바닥 터지기'),
    'waterpang.mokomoko.att_battle_1_01.full.restore': ('hazards', '중앙 모코모코 물 폭포'),
    'watergun.watergun_att_2.full.restore': ('skills', 'Q 연발 물총 총구'),
    'watergun.q.flight': ('skills', 'Q 연발 물총 날아가기'),
    'watergun.q.hit': ('skills', 'Q 연발 물총 맞기'),
    'watergun.w.flight': ('skills', 'W 물폭탄 던지기'),
    'watergun.w.hit': ('skills', 'W 물폭탄 터지기'),
    'watergun.e.speed': ('skills', 'E 빠르게 이동하기'),
    'watergun.watergun_att_1.full.restore': ('skills', 'R 기본 물총 총구'),
    'watergun.r.flight': ('skills', 'R 기본 물총 날아가기'),
    'watergun.r.hit': ('skills', 'R 기본 물총 맞기'),
    'watergun.shot.start': ('skills', 'Q·R 물방울 발사'),
    'watergun.watergun_att_3.full.restore': ('variants', 'MK2 범위 물총 총구'),
    'watergun.watergun_att_4.full.restore': ('variants', 'MK2 단발 물총 총구'),
}
SEQUENCES = {
    'sequence.maharaka.waterpang.attack.mokomoko': '워터팡 | 중앙 바닥 장판·터지기',
    'sequence.maharaka.waterpang.attack.cannon.start': '워터팡 | 중앙 기둥 분사 시작',
    'sequence.maharaka.waterpang.attack.cannon.loop.cw': '워터팡 | 중앙 기둥 물 뿜기·시계 회전',
    'sequence.maharaka.waterpang.attack.cannon.loop.ccw': '워터팡 | 중앙 기둥 물 뿜기·반시계 회전',
    'sequence.maharaka.waterpang.attack.cannon.end': '워터팡 | 중앙 기둥 분사 종료',
}


def read(path):
    return json.loads(path.read_bytes())


def digest(data):
    return hashlib.sha256(data).hexdigest()


def rename_templates(original):
    """Change only each selected template's displayName bytes, not its tracks."""
    text = original.decode('utf-8-sig')
    match = re.search(r'^  "templates"\s*:\s*\[', text, re.MULTILINE)
    assert match, 'Missing templates'
    decoder, cursor, edits, found = json.JSONDecoder(), match.end(), [], set()
    while True:
        cursor += len(text[cursor:]) - len(text[cursor:].lstrip())
        if text[cursor] == ']':
            break
        row, length = decoder.raw_decode(text[cursor:])
        target = SEQUENCES.get(row['sequenceId'])
        if target and row['displayName'] != target:
            span = text[cursor:cursor + length]
            field = re.search(r'"displayName"\s*:\s*', span)
            assert field
            _, consumed = decoder.raw_decode(span[field.end():])
            edits.append((cursor + field.end(), cursor + field.end() + consumed,
                          json.dumps(target, ensure_ascii=False)))
        found.add(row['sequenceId'])
        cursor += length
        cursor += len(text[cursor:]) - len(text[cursor:].lstrip())
        if text[cursor] == ',':
            cursor += 1
    assert set(SEQUENCES) <= found
    for start, end, replacement in reversed(edits):
        text = text[:start] + replacement + text[end:]
    before = json.loads(original)
    for row in before['templates']:
        if row['sequenceId'] in SEQUENCES:
            row['displayName'] = SEQUENCES[row['sequenceId']]
    assert json.loads(text) == before
    encoded = (b'\xef\xbb\xbf' if original.startswith(b'\xef\xbb\xbf') else b'') + text.encode('utf8')
    return replace_root_value(encoded, 'revision', before['revision'] + 1) if edits else original


def prepare(candidates, baseline=None):
    writes, names, inputs = [], {}, {}
    source_files = {p.name: p for directory in candidates for p in directory.glob('*.effect.json')}
    for suffix, (_, label) in NAMES.items():
        asset = PREFIX + suffix
        path = AUTHORED / (asset + '.effect.json')
        source = path if path.exists() else source_files.get(path.name)
        assert source, ('Missing restored effect document', asset)
        data = source.read_bytes()
        document = json.loads(data)
        assert document['effectAssetId'] == asset and document['elements']
        desired = '워터팡 | ' + label
        names[asset] = document['displayName']
        if baseline and asset in baseline['names']:
            assert document['displayName'] in (baseline['names'][asset], desired), ('Concurrent effect rename', asset)
        after = replace_root_value(data, 'displayName', desired)
        writes.append((path, path.read_bytes() if path.exists() else None, after))
        inputs[source] = digest(data)

    before = CATALOG.read_bytes()
    catalog = json.loads(before)
    known = {r['effectAssetId']: r for r in catalog['effects']}
    assert len(known) == len(catalog['effects'])
    for suffix in NAMES:
        asset = PREFIX + suffix
        row = dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT',
                   authoringPath='Effects/Authored/' + asset + '.effect.json')
        if asset in known:
            assert all(known[asset].get(k) == v for k, v in row.items()), ('Conflicting catalog entry', asset)
        else:
            catalog['effects'].append(row)
    after = replace_root_value(before, 'effects', catalog['effects'], rows=True)
    writes.append((CATALOG, before, after if json.loads(before) != catalog else before))

    before = TREE.read_bytes()
    tree = json.loads(before)
    nodes = {r['id']: r for r in tree['nodes']}
    refs = {(r['kind'], r['assetId']): r for r in tree['references']}
    assert len(nodes) == len(tree['nodes']) and len(refs) == len(tree['references'])
    assert nodes['world']['kind'] == 'CATEGORY'
    for identifier, parent, name in (
        ('world.maharaka', 'world', '마하라카'),
        ('world.maharaka.waterpang', 'world.maharaka', '워터팡'),
        ('world.maharaka.waterpang.hazards', 'world.maharaka.waterpang', '중앙 기둥·바닥'),
        ('world.maharaka.waterpang.skills', 'world.maharaka.waterpang', 'Q·W·E·R 물총'),
        ('world.maharaka.waterpang.variants', 'world.maharaka.waterpang', '원본 MK2 변형')):
        expected = dict(id=identifier, parentId=parent, kind='CATEGORY', displayName=name)
        assert identifier not in nodes or nodes[identifier] == expected, ('Conflicting category', identifier)
        nodes[identifier] = expected
    for suffix, (branch, label) in NAMES.items():
        asset = PREFIX + suffix
        row = dict(refs.get(('V1', asset), {}))
        row.update(kind='V1', assetId=asset, displayName='워터팡 | ' + label,
                   parentId='world.maharaka.waterpang.' + branch)
        refs[('V1', asset)] = row
    tree['nodes'], tree['references'] = list(nodes.values()), list(refs.values())
    after = replace_root_value(before, 'nodes', tree['nodes'], rows=True)
    after = replace_root_value(after, 'references', tree['references'], rows=True)
    writes.append((TREE, before, after if json.loads(before) != tree else before))

    before = WORLD.read_bytes()
    world_names = {r['sequenceId']: r['displayName'] for r in json.loads(before)['templates']
                   if r['sequenceId'] in SEQUENCES}
    if baseline:
        for key, value in world_names.items():
            assert value in (baseline['worldNames'][key], SEQUENCES[key]), ('Concurrent sequence rename', key)
    writes.append((WORLD, before, rename_templates(before)))
    writes += stage_project_metadata([AUTHORED / (PREFIX + s + '.effect.json') for s in NAMES])
    manifest = dict(names=names, worldNames=world_names, effects=[dict(effectAssetId=PREFIX + suffix,
        displayName='워터팡 | ' + label, group=branch) for suffix, (branch, label) in NAMES.items()])
    return writes, inputs, manifest


def commit(writes, inputs, backup):
    backup.mkdir(parents=True, exist_ok=False)
    pending, committed = {}, []
    def current(path):
        return path.read_bytes() if path.exists() else None
    def verify():
        for path, expected in inputs.items():
            assert digest(path.read_bytes()) == expected, ('Source changed', str(path))
        for path, before, _ in writes:
            assert current(path) == before, ('Authoring changed', str(path))
    try:
        verify()
        for path, before, after in writes:
            if before == after:
                continue
            relative = path.relative_to(ROOT)
            if before is not None:
                destination = backup / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(before)
            temporary = path.with_name(path.name + '.waterpang.tmp')
            with temporary.open('xb') as stream:
                stream.write(after)
            pending[path] = temporary
        verify()
        for path, before, after in writes:
            if path not in pending:
                continue
            assert current(path) == before, ('Changed during commit', str(path))
            pending[path].replace(path)
            committed.append((path, before, after))
    except BaseException:
        for path, before, after in reversed(committed):
            if current(path) != after:
                continue
            if before is None:
                path.unlink()
            else:
                with pending[path].open('xb') as stream:
                    stream.write(before)
                pending[path].replace(path)
        raise
    finally:
        for temporary in pending.values():
            if temporary.exists():
                temporary.unlink()


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--candidate', type=Path, action='append', default=[])
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    baseline = read(args.output / 'manifest.json') if args.apply else None
    writes, inputs, manifest = prepare([] if args.apply else args.candidate, baseline)
    if args.apply:
        assert all(before is not None for path, before, _ in writes if path.parent == AUTHORED), 'Install restored sources first'
        commit(writes, inputs, args.output / 'backup')
    else:
        for path, _, after in writes:
            destination = args.output / 'candidate' / path.relative_to(ROOT)
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(after)
        args.output.mkdir(parents=True, exist_ok=True)
        (args.output / 'manifest.json').write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(json.dumps(dict(applied=args.apply, effects=len(NAMES), sequences=len(SEQUENCES),
        changedFiles=sum(before != after for _, before, after in writes)), ensure_ascii=False))


if __name__ == '__main__':
    main()
