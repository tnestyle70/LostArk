"""Stage common firebreath in the existing cross and three-socket resources.

This is an appearance replacement, not a reconstruction of the source effects.
Existing resource identities, warning/preparation/light elements and authored
occurrence windows remain owned by the saved documents and their installer.
"""
import argparse
import collections
import copy
import hashlib
import json
from pathlib import Path

from build_kouku_shared_firebreath import SHARED_ASSET, copy_shared_firebreath_elements

ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / 'Data/Effects/Authored'
CROSS_ASSETS = ('effect.kouku.firecross.impact.line',
                'effect.kouku.firecross.impact', 'effect.kouku.firecross.impact.full')
THREEWAY = 'effect.kouku.gate3.threeway.breath.full.restore'
CROSS_SOURCE = 'fx_mn_istm_00-4.par_d_istm_00-4_sk02_02'
THREEWAY_SOURCE = 'fx_mn_rpct_05_l.par_l_rpct_05_sk_01_loc_int'
REGENERATION_BUILDERS = ('build_kouku_albion_cross_groups.py', 'build_kouku_threeway_breath_group.py')
IDENTITY_PS = dict(uniformScaleMultiplier=1, yawOffsetDegrees=0,
                   directionYawDegrees=0, initialSpeedMultiplier=1)


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def system(element):
    return element['sourceNode'].split('|')[-1].split('.particles')[0]


def strings(value):
    if isinstance(value, dict):
        for item in value.values():
            yield from strings(item)
    elif isinstance(value, list):
        for item in value:
            yield from strings(item)
    elif isinstance(value, str):
        yield value


def assert_independent(element):
    assert not element.get('runtimeCarrier') and not element.get('sourceTransformTrack')
    assert not element.get('transformInheritance', {}).get('enabled')


def replace_cross(document, shared):
    assert document['particleSystem'] == IDENTITY_PS
    selected = [e for e in document['elements'] if system(e) == CROSS_SOURCE]
    expected = 10 if document['effectAssetId'].endswith('.line') else 20
    assert len(selected) == expected
    groups = collections.defaultdict(list)
    for element in selected:
        assert_independent(element)
        assert not element['actionCueAttachment']['enabled']
        transform = element['detail']['transform']
        assert transform['rotationDegrees'][0] == transform['rotationDegrees'][2] == 0
        key = json.dumps([transform, element['detail']['timing']['startDelaySeconds']], sort_keys=True)
        groups[key].append(element)
    assert len(groups) == expected // 10 and all(len(g) == 10 for g in groups.values())
    result = copy.deepcopy(document)
    replaced_ids = {e['id'] for e in selected}
    result['elements'] = [copy.deepcopy(e) for e in document['elements'] if e['id'] not in replaced_ids]
    assert not replaced_ids.intersection(strings(result)), 'Retained element references a replaced impact'
    branches = []
    for ordinal, group in enumerate(groups.values()):
        source = group[0]
        transform = source['detail']['transform']
        start = source['detail']['timing']['startDelaySeconds']
        for opposite in (0, 180):
            key = document['effectAssetId'] + f'.shared.arm.{ordinal}.{opposite}'
            elements = copy_shared_firebreath_elements(shared, key, transform['position'])
            yaw = transform['rotationDegrees'][1] + opposite
            for element in elements:
                detail = element['detail']
                detail['transform']['rotationDegrees'] = [0, yaw, 0]
                detail['transform']['scale'] = [a * b for a, b in zip(
                    detail['transform']['scale'], transform['scale'])]
                detail['transform']['velocityPerSecond'] = copy.deepcopy(transform['velocityPerSecond'])
                detail['transform']['revolutionDegreesPerSecond'] = copy.deepcopy(transform['revolutionDegreesPerSecond'])
                detail['timing']['startDelaySeconds'] += start
            result['elements'].extend(elements)
            branches.append(dict(groupId=key, yawDegrees=yaw, startSeconds=start,
                                 position=transform['position'], elementIds=[e['id'] for e in elements]))
    return result, dict(replaced=len(selected), retained=len(document['elements']) - len(selected), branches=branches)


def replace_threeway(document, shared):
    assert document['particleSystem'] == IDENTITY_PS
    assert document['sourceModelPreview']['actorProfileId'] == 'MN_RPCT_05'
    selected = [e for e in document['elements'] if system(e) == THREEWAY_SOURCE]
    groups = collections.defaultdict(list)
    for element in selected:
        assert_independent(element)
        attachment = element['actionCueAttachment']
        assert attachment['enabled'] and attachment['follow']
        groups[attachment['runtimeAnchorSlotId']].append(element)
    assert set(groups) == {'FX_Prj_01', 'FX_Prj_02', 'FX_Prj_03'}
    assert all(len(g) == 14 for g in groups.values())
    result = copy.deepcopy(document)
    replaced_ids = {e['id'] for e in selected}
    result['elements'] = [copy.deepcopy(e) for e in document['elements'] if e['id'] not in replaced_ids]
    assert not replaced_ids.intersection(strings(result)), 'Retained element references a replaced breath'
    branches = []
    for slot, group in groups.items():
        source = group[0]
        transform, attachment = source['detail']['transform'], source['actionCueAttachment']
        start = source['detail']['timing']['startDelaySeconds']
        assert transform['rotationDegrees'] == [0, 0, 0]
        assert all(e['detail']['transform'] == transform and e['actionCueAttachment'] == attachment and
                   e['detail']['timing']['startDelaySeconds'] == start for e in group)
        key = document['effectAssetId'] + '.shared.' + slot.lower()
        elements = copy_shared_firebreath_elements(shared, key, transform['position'])
        for element in elements:
            element['actionCueAttachment'] = copy.deepcopy(attachment)
            detail = element['detail']
            # Old Sk_01's velocity payload is UE +X, hence Client +X. The
            # shared template is +Z: align it with that existing socket axis.
            detail['transform']['rotationDegrees'] = [0, 90, 0]
            # Actual installed RPCT05 bones already contribute scale1.7.
            # Keep the authored main scale instead of multiplying 1.7 twice.
            detail['transform']['scale'] = copy.deepcopy(transform['scale'])
            detail['transform']['velocityPerSecond'] = copy.deepcopy(transform['velocityPerSecond'])
            detail['transform']['revolutionDegreesPerSecond'] = copy.deepcopy(transform['revolutionDegreesPerSecond'])
            detail['timing']['startDelaySeconds'] += start
        result['elements'].extend(elements)
        branches.append(dict(groupId=key, sourceAnchorSlot=slot, bone=attachment['runtimeBoneName'],
                             attachment=attachment, sourceLocalForward=[1, 0, 0], sharedLocalYawDegrees=90,
                             startSeconds=start, position=transform['position'],
                             elementIds=[e['id'] for e in elements]))
    assert len(result['elements']) == 110
    return result, dict(replaced=42, retained=35, branches=branches)


def stage(output):
    output = output.resolve()
    assert output.is_relative_to(ROOT / 'out'), 'Stage output must remain under out/'
    shared_path = AUTHORED / (SHARED_ASSET + '.effect.json')
    shared = read(shared_path)
    composition_path = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
    tree_path = ROOT / 'Data/Effects/EffectResourceTree.json'
    composition, tree = read(composition_path), read(tree_path)
    resources = {r['assetId']: r for r in composition['presentationResources']
                 if r['kind'] == 'EFFECT' and r['resourceKind'] == 'V1_EFFECT'}
    parents = {r['assetId']: r['parentId'] for r in tree['references'] if r['kind'] == 'V1'}
    scripts = [Path(__file__), Path(__file__).with_name('build_kouku_shared_firebreath.py')]
    scripts.extend(Path(__file__).with_name(name) for name in REGENERATION_BUILDERS)
    inputs = {p.relative_to(ROOT).as_posix(): sha(p) for p in (shared_path, composition_path, tree_path, *scripts)}
    documents, diagnostics = [], []
    for asset in (*CROSS_ASSETS, THREEWAY):
        path = AUTHORED / (asset + '.effect.json')
        before = path.read_bytes()
        original = json.loads(before)
        candidate, info = replace_threeway(original, shared) if asset == THREEWAY else replace_cross(original, shared)
        assert len({e['id'] for e in candidate['elements']}) == len(candidate['elements'])
        candidate_path = output / 'candidate' / path.name
        write(candidate_path, candidate)
        backup = output / 'baseline' / path.name
        backup.parent.mkdir(parents=True, exist_ok=True)
        backup.write_bytes(before)
        resource = resources[asset]
        consumers = [dict(patternId=p['patternId'], occurrenceId=o['occurrenceId'])
                     for p in composition['patterns'] for o in p['presentationOccurrences']
                     if o['resourceId'] == resource['resourceId']]
        relative = path.relative_to(ROOT).as_posix()
        inputs[relative] = hashlib.sha256(before).hexdigest()
        documents.append(dict(effectAssetId=asset, candidatePath=candidate_path.relative_to(ROOT).as_posix(),
            path=relative, displayName=original['displayName'], durationMs=resource['durationMs'],
            defaultAnchorKind=resource['defaultAnchorKind'], parentId=parents[asset],
            resourceId=resource['resourceId'], beforeSha256=inputs[relative],
            candidateSha256=sha(candidate_path), changeKind='REPLACE_DIRECTIONAL_FLAME_APPEARANCE'))
        diagnostics.append(dict(effectAssetId=asset, beforeElements=len(original['elements']),
            afterElements=len(candidate['elements']), existingResourceDurationMs=resource['durationMs'],
            savedConsumers=consumers, **info))
    write(output / 'installation.json', dict(installed=False, stageOnly=True, documents=documents,
        inputHashes=inputs, diagnostics=diagnostics, sharedAssetId=SHARED_ASSET,
        sharedNativeDurationSeconds=5.5, authoredOccurrenceWindowsChanged=False,
        emptyPatternAnimationsCreated=False, compositionRevision=composition['revision'],
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(staged=len(documents), output=str(output), liveDataWritten=False)))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuDirectionalFlame20260914')
    stage(parser.parse_args().output)
