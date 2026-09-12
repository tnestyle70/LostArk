"""Register restored Kouku groups in the V1 library and existing authoring trees.

Only successfully installed source documents are accepted. Existing unrelated
catalog entries, composition edits and project registrations are preserved.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
NAMES = {
    'staff.flame.full.restore': ('지팡이 내려찍기', '바닥 예고 후 화염기둥'),
    'doll.flame.full.restore': ('마리오', '기괴한 인형 양쪽 화염'),
    'mario.center.pentagram.full.restore': ('마리오', '중앙 오망성'),
    'mario.center.portal.full.restore': ('마리오', '중앙 진입 포탈'),
    'mario.boss.pentagram.full.restore': ('마리오', '작은 오망성 원본 후보'),
    'mario.center.full.restore': ('마리오', '중앙 오망성와 포탈'),
    'showtime.gun.create': ('쇼타임/기관총', '기관총 생성'),
    'showtime.gun.loop': ('쇼타임/기관총', '기관총 유지'),
    'showtime.gun.end': ('쇼타임/기관총', '기관총 종료'),
    'showtime.gun.shell': ('쇼타임/기관총', '탄피'),
    'showtime.gun.muzzle': ('쇼타임/기관총', '총구 발사'),
    'showtime.gun.signature': ('쇼타임/기관총', '발사 섬광'),
    'showtime.gun.ground': ('쇼타임/장판·조준·폭발', '사격 바닥 표시'),
    'showtime.airstrike.impact': ('쇼타임/공 낙하·폭발', '공습 폭발'),
    'showtime.airstrike.missile': ('쇼타임/공 낙하·폭발', '공습 투사체'),
    'showtime.ball.impact': ('쇼타임/공 낙하·폭발', '공 충돌 폭발'),
    'showtime.ball.red': ('쇼타임/공 낙하·폭발', '빨간 공 폭발'),
    'showtime.circle.impact01': ('쇼타임/장판·조준·폭발', '원형 장판 폭발 01'),
    'showtime.circle.impact03': ('쇼타임/장판·조준·폭발', '원형 장판 폭발 03'),
    'showtime.ball.drop': ('쇼타임/공 낙하·폭발', '공 낙하'),
    'showtime.napalm': ('쇼타임/장판·조준·폭발', '화염 장판'),
    'showtime.target.fixed': ('쇼타임/장판·조준·폭발', '바닥 고정 조준점'),
    'showtime.target.tracking': ('쇼타임/장판·조준·폭발', '바닥 추적 조준점'),
    'showtime.target.end': ('쇼타임/장판·조준·폭발', '조준 종료'),
    'showtime.fire.impact': ('쇼타임/장판·조준·폭발', '화염 폭발'),
    'showtime.bullet': ('쇼타임/기관총', '총알 비행·충돌'),
}


def read(path): return json.loads(path.read_text(encoding='utf-8-sig'))
def payload(value): return (json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8')
def identity(value): return hashlib.sha256(value.encode('utf8')).hexdigest()[:20]


def register(manifests, organization, output, install):
    snapshots, staged = {}, {}
    def load(relative):
        path = ROOT / relative
        snapshots[path] = path.read_bytes()
        return json.loads(snapshots[path])
    catalog = load('Data/Effects/EffectCatalog.json')
    tree = load('Data/Effects/EffectResourceTree.json')
    composition = load('Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json')
    source_organization = read(organization) if organization else {}
    source_rows = {r['sourceSystem']: r for r in source_organization.get('particleSystems', [])}
    documents = []
    for path in manifests:
        manifest = read(path)
        assert manifest['installed'], f'Source documents are not installed: {path}'
        documents += manifest['documents']
    nodes = {n['id']: n for n in tree['nodes']}
    references = {(r['kind'], r['assetId']): r for r in tree['references']}
    def category(labels):
        parent, path = 'root.v1', []
        for label in labels:
            path.append(label)
            key = 'kouku.category.' + identity('/'.join(path))
            row = dict(id=key, parentId=parent, kind='CATEGORY', displayName=label)
            assert key not in nodes or nodes[key] == row
            nodes[key] = row
            parent = key
        return parent
    known = {r['effectAssetId']: r for r in catalog['effects']}
    resources = {r['assetId']: r for r in composition['presentationResources'] if r['kind'] == 'EFFECT'}
    pattern = next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_32')
    assert pattern['gateId'] == 'GATE3' and '쇼타임' in pattern['displayName']
    additions = []
    for row in documents:
        path = ROOT / row['path']
        doc = read(path)
        asset = row['effectAssetId']
        assert doc['effectAssetId'] == asset and doc['elements']
        for element in doc['elements']:
            if element['kind'] not in ('light', 'screenPost') and not element.get('sourceRecipe', {}).get('simulationOnly'):
                assert element['material'].get('sourceProfile', {}).get('enabled'), f'Material projection is incomplete: {asset}'
        relative = path.relative_to(ROOT / 'Data').as_posix()
        entry = dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT', authoringPath=relative)
        assert asset not in known or known[asset] == entry
        if asset not in known:
            catalog['effects'].append(entry)
            known[asset] = entry
        suffix = asset.removeprefix('effect.kouku.gate3.')
        if suffix in NAMES:
            branch, label = NAMES[suffix]
            labels = ['KoukuSaydon', '3관문', '패턴', '세이튼'] + branch.split('/')
            name = '3관문_세이튼_' + branch.split('/')[0] + '_' + label
        else:
            labels = row.get('categoryPath', [])
            name = row['displayName']
            if not labels:
                source_row = source_rows.get(row.get('sourceParticleSystem'), {})
                sequence_uses = source_row.get('sequenceUses', [])
                if sequence_uses:
                    sequence = sequence_uses[0]
                    classification = sequence['classification']
                    gate = classification.get('gateId', 'SHARED')
                    labels = ['KoukuSaydon', {'GATE1':'1관문','GATE2':'2관문','GATE3':'3관문'}.get(gate,'공통'),
                        '연출', classification.get('displayName') or sequence['sourceScene'], sequence['groupName']]
                else:
                    labels = ['KoukuSaydon', '공통', '개별 이펙트', row.get('sourceParticleSystem', asset).split('.')[0]]
        assert len(name.encode('utf8')) <= 256
        references[('V1', asset)] = dict(kind='V1', assetId=asset, displayName=name, parentId=category(labels))
        if doc['displayName'] != name:
            snapshots[path] = path.read_bytes()
            doc['displayName'] = name
            staged[path] = payload(doc)
        if asset not in resources:
            resource = dict(resourceId='kakulsaydon.effect.' + identity(asset), displayName=name,
                defaultAnchorKind='BOSS' if doc.get('sourceModelPreview') else 'MAP', kind='EFFECT', assetId=asset,
                resourceKind='V1_EFFECT', elementId='', durationMs=row['durationMs'], shape='BOX', colliderKind='GEOMETRY',
                halfExtents=[1,1,1], radiusM=3, halfAngleDegrees=45)
            composition['presentationResources'].append(resource)
            resources[asset] = resource
        else:
            resources[asset]['displayName'] = name
        if suffix.startswith('showtime.') and not any(o['resourceId'] == resources[asset]['resourceId'] for o in pattern['presentationOccurrences']):
            ordinal = pattern['nextPresentationOccurrenceOrdinal']
            pattern['presentationOccurrences'].append(dict(occurrenceId=pattern['patternId'] + '.presentation.' + str(ordinal),
                resourceId=resources[asset]['resourceId'], startMs=0, durationMs=row['durationMs'], positionOffset=[0,0,0],
                rotationDegrees=[0,0,0], scale=[1,1,1], fadeInMs=0, fadeOutMs=0, dissolveStart=.95, dissolveEnd=1,
                brightnessMultiplier=1, volume=1, followBoss=bool(doc.get('sourceModelPreview')), debugRender=True,
                bone='', boneTarget='BODY', regionId='', cardSymbol='NONE', cardColor='NONE',
                anchorKind=resources[asset]['defaultAnchorKind'], worldId='', logicOccurrenceId='', worldOccurrenceId=''))
            pattern['nextPresentationOccurrenceOrdinal'] = ordinal + 1
        additions.append(dict(effectAssetId=asset, displayName=name, categoryPath=labels, elements=len(doc['elements']),
            durationMs=row['durationMs'], path=row['path']))
    tree['nodes'] = list(nodes.values())
    tree['references'] = list(references.values())
    for relative, value in [('Data/Effects/EffectCatalog.json', catalog), ('Data/Effects/EffectResourceTree.json', tree)]:
        staged[ROOT / relative] = payload(value)
    composition_path = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
    if json.loads(snapshots[composition_path]) != composition:
        composition['revision'] += 1
        staged[composition_path] = payload(composition)
    for suffix in ('', '.filters'):
        path = ROOT / ('Client/Default/Client.vcxproj' + suffix)
        snapshots[path] = path.read_bytes()
        text = snapshots[path].decode('utf8')
        newline = '\r\n' if '\r\n' in text else '\n'
        entries = []
        for row in additions:
            include = '..\\..\\' + row['path'].replace('/', '\\')
            if 'Include="' + include + '"' in text: continue
            entries.append('    <None Include="' + include + '">' + ('<Filter>96.DataFiles</Filter>' if suffix else '') + '</None>')
        if entries:
            prefix, closing, tail = text.rpartition('</Project>')
            assert closing and not tail.strip()
            text = prefix + '  <ItemGroup>' + newline + newline.join(entries) + newline + '  </ItemGroup>' + newline + closing + tail
            ET.fromstring(text)
            staged[path] = text.encode('utf8')
    for path, before in snapshots.items():
        assert path.read_bytes() == before, f'Concurrent authoring edit: {path}'
    changed = []
    for path, value in staged.items():
        if value != snapshots[path]:
            changed.append(path.relative_to(ROOT).as_posix())
            if install: path.write_bytes(value)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(payload(dict(installed=install, documents=additions, changedPaths=changed,
        showtimePatternId=pattern['patternId'], showtimeMode='INDEPENDENT_GROUP_TUNING_DRAFT',
        manualVisualValidation='USER_PENDING')))
    print(json.dumps(dict(installed=install, groups=len(additions), changedFiles=len(changed))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--manifest', action='append', type=Path, required=True)
    parser.add_argument('--organization', type=Path)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuGate3Effects20260912/library_installation.json')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    register(args.manifest, args.organization, args.output, args.install)
