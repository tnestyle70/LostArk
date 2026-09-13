"""Add missing Kouku authored effects to the existing saved-resource tree.

This is library organization, not Product admission or gameplay authoring.
Existing classifications and all effect/Composition documents remain intact.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
from pathlib import Path

from install_kouku_effect_library import replace_root_value

ROOT = Path(__file__).resolve().parents[2]
TREE = ROOT / 'Data/Effects/EffectResourceTree.json'
COMPOSITIONS = (
    ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json',
    ROOT / 'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json',
)
ACTORS = {'MN_RPCZ_00': '쿠크', 'MN_RPCT_05': '세이튼',
          'MN_RPCT_06': '대형 세이튼', 'MN_RPCT_07': '쿠크세이튼'}


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def gate_name(gate):
    return {'GATE1': '1관문', 'GATE2': '2관문', 'GATE3': '3관문'}.get(gate, '공통')


def bounded_name(name):
    return name.encode('utf8')[:256].decode('utf8', errors='ignore')


def composition_uses():
    by_asset, by_action = collections.defaultdict(list), collections.defaultdict(list)
    for path in COMPOSITIONS:
        document = read(path)
        resources = {row['resourceId']: row for row in document['presentationResources']}
        for pattern in document['patterns']:
            section = '연출' if 'Sequences' in path.parts else '패턴'
            category = ['KoukuSaydon', gate_name(pattern['gateId']), section,
                        ACTORS.get(pattern['actorProfileId'], pattern['actorProfileId']),
                        pattern['displayName']]
            context = dict(categoryPath=category, patternId=pattern['patternId'],
                           actorProfileId=pattern['actorProfileId'], gateId=pattern['gateId'])
            for occurrence in pattern.get('presentationOccurrences', []):
                resource = resources.get(occurrence['resourceId'])
                if resource and resource['kind'] == 'EFFECT':
                    by_asset[resource['assetId']].append(dict(context, displayName=resource['displayName']))
            for stage in pattern['stages']:
                for clip in stage.get('animationOccurrences', []):
                    if 'sourceActionId' in clip:
                        key = clip['profileId'], clip['sourceActionId']
                        if context not in by_action[key]:
                            by_action[key].append(context)
    return by_asset, by_action


def classify(asset, document, sources, actions, asset_uses, action_uses):
    # Existing live authoring associations take precedence over old survey labels.
    uses = asset_uses.get(asset, [])
    if uses:
        use = uses[0]
        return use['categoryPath'], use['displayName'], 'CURRENT_COMPOSITION', uses
    system = asset.removeprefix('effect.kouku.source.') if asset.startswith('effect.kouku.source.') else ''
    source = sources.get(system, {})
    action_keys = []
    for use in source.get('directActionUses', []):
        if use.get('enabled') is not False:
            action_keys.append((use['profileId'], use['actionId']))
    for kind in ('indirectProjectileUses', 'indirectNpcActionUses', 'indirectBuffUses'):
        for use in source.get(kind, []):
            action_keys.extend((origin['profileId'], origin['actionId']) for origin in use.get('originActions', []))
    action_keys = sorted(set(action_keys))
    current = [use for key in action_keys for use in action_uses.get(key, [])]
    if current:
        return current[0]['categoryPath'], document['displayName'], 'SOURCE_USED_BY_CURRENT_PATTERN', current
    candidates = [actions[key] for key in action_keys if key in actions]
    if candidates:
        action = candidates[0]
        return action['categoryPath'], document['displayName'], 'SOURCE_ACTION_LIBRARY_PATTERN', [
            dict(profileId=row['profileId'], actionId=row['actionId']) for row in candidates]
    sequences = source.get('sequenceUses', [])
    if sequences:
        occurrence = sequences[0]
        context = occurrence['classification']
        return ['KoukuSaydon', gate_name(context.get('gateId')), '연출',
                context.get('displayName') or occurrence['sourceScene']], document['displayName'], 'SOURCE_SEQUENCE', sequences
    preview = document.get('sourceModelPreview', {})
    gate = gate_name(preview.get('gateId'))
    if gate == '공통':
        for number in (1, 2, 3):
            if asset.startswith(f'effect.kouku.gate{number}.'):
                gate = f'{number}관문'
                break
    section = '연출' if any(token in asset for token in ('.intro.', '.sequence.', '.popup.', '.portal-arrival.')) else '패턴'
    # A library folder is not a fabricated executable boss Pattern.
    return ['KoukuSaydon', gate, section, '개별 이펙트'], document['displayName'], 'INDEPENDENT_LIBRARY', []


def sync(organization, output, install=False):
    original = TREE.read_bytes()
    tree = json.loads(original)
    prior = read(organization)
    sources = {row['sourceSystem']: row for row in prior['particleSystems']}
    actions = {(row['profileId'], row['actionId']): row for row in prior['actions']}
    asset_uses, action_uses = composition_uses()
    nodes = {row['id']: row for row in tree['nodes']}
    references = {(row['kind'], row['assetId']): row for row in tree['references']}
    before_nodes = list(tree['nodes'])
    before_refs = list(tree['references'])
    additions, inputs = [], []

    def category(labels):
        parent, parts = 'root.v1', []
        for label in labels:
            parts.append(label)
            identity = 'kouku.category.' + hashlib.sha256('/'.join(parts).encode('utf8')).hexdigest()[:20]
            row = dict(id=identity, parentId=parent, kind='CATEGORY', displayName=bounded_name(label))
            if identity in nodes:
                assert nodes[identity] == row, ('Category identity conflict', identity)
            else:
                nodes[identity] = row
            parent = identity
        return parent

    paths = sorted((ROOT / 'Data/Effects/Authored').glob('effect.kouku.*.effect.json'))
    for path in paths:
        asset = path.name.removesuffix('.effect.json')
        if ('V1', asset) in references:
            continue
        document = read(path)
        assert document['effectAssetId'] == asset
        assert document['schema'] == 'lostark.effect-authoring'
        labels, name, basis, uses = classify(asset, document, sources, actions, asset_uses, action_uses)
        row = dict(kind='V1', assetId=asset, displayName=bounded_name(name), parentId=category(labels))
        references['V1', asset] = row
        additions.append(dict(assetId=asset, categoryPath=labels, displayName=row['displayName'], basis=basis, uses=uses))
        inputs.append(dict(path=path.relative_to(ROOT).as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
    candidate = replace_root_value(original, 'nodes', list(nodes.values()), rows=True)
    candidate = replace_root_value(candidate, 'references', list(references.values()), rows=True)
    parsed = json.loads(candidate)
    assert parsed['nodes'][:len(before_nodes)] == before_nodes
    assert parsed['references'][:len(before_refs)] == before_refs
    assert all(('V1', path.name.removesuffix('.effect.json')) in references for path in paths)
    assert len(nodes) <= 4096 and len(references) <= 16384
    assert TREE.read_bytes() == original, 'Tree changed during organization; preserve current authoring.'
    output.mkdir(parents=True, exist_ok=True)
    (output / 'EffectResourceTree.before.json').write_bytes(original)
    (output / 'EffectResourceTree.candidate.json').write_bytes(candidate)
    if install and candidate != original:
        pending = TREE.with_name(TREE.name + '.kouku-sync.tmp')
        assert not pending.exists(), 'Previous pending tree must be inspected first.'
        pending.write_bytes(candidate)
        if TREE.read_bytes() != original:
            pending.unlink()
            raise RuntimeError('Tree changed before commit; previous file preserved.')
        pending.replace(TREE)
    report = dict(installed=install, authoredCount=len(paths), addedReferences=len(additions),
                  nodeCount=len(nodes), referenceCount=len(references), previousRowsPreserved=True,
                  classifications=dict(collections.Counter(row['basis'] for row in additions)),
                  added=additions, inputs=inputs)
    (output / 'organization-result.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(json.dumps({key: value for key, value in report.items() if key not in ('added', 'inputs')}))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--organization', type=Path, default=ROOT / 'out/KoukuAllEffects20260912/organization.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuEffectLibrary20260913/Tree')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    sync(args.organization, args.output, args.install)
