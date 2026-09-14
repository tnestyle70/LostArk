"""Stage the shared flame library and authored flame-wave consumers.

Preparation never writes live authoring. Applying a reviewed candidate requires
unchanged input hashes and closed Client/Server processes; a failed replacement
restores every file already replaced. Runtime data remains publisher-owned.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
from install_kouku_effect_library import identity, replace_root_value

COMPOSITION = 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
STAFF = 'effect.kouku.gate3.staff.flame.full.restore'
SHARED = 'effect.kouku.gate3.firebreath.shared'
WAVE = 'effect.kouku.common.flame.wave.full'
KOUKU_WAVE = WAVE + '.koukusaydon'


def read(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))


def payload(value):
    return (json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf-8')


def digest(value):
    return hashlib.sha256(value).hexdigest()


def repository_path(value):
    path = Path(value)
    path = path.resolve() if path.is_absolute() else (ROOT / path).resolve()
    if not path.is_relative_to(ROOT):
        raise ValueError(f'Candidate path leaves repository: {path}')
    return path


def replace_pattern_records(original, patterns):
    """Keep untouched user Pattern bytes, including formatting and field order."""
    text = original.decode('utf-8-sig')
    match = re.search(r'^  "patterns"\s*:\s*\[', text, re.MULTILINE)
    if not match:
        raise ValueError('Missing patterns array')
    cursor, decoder, edits, seen = match.end(), json.JSONDecoder(), [], set()
    replacements = {p['patternId']: p for p in patterns}
    newline = '\r\n' if '\r\n' in text else '\n'

    def encode(value):
        return json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False).replace('\n', newline + '    ')

    while True:
        while text[cursor].isspace() or text[cursor] == ',':
            cursor += 1
        if text[cursor] == ']':
            break
        value, length = decoder.raw_decode(text[cursor:])
        key = value['patternId']
        seen.add(key)
        if key not in replacements:
            raise ValueError('Removing an existing user Pattern is unsupported')
        if replacements[key] != value:
            edits.append((cursor, cursor + length, encode(replacements[key])))
        cursor += length
    added = [p for p in patterns if p['patternId'] not in seen]
    if added:
        previous = cursor
        while text[previous - 1].isspace():
            previous -= 1
        edits.append((previous, cursor, (',' if seen else '') + newline +
                      (',' + newline).join('    ' + encode(p) for p in added) + newline + '  '))
    for begin, end, value in reversed(edits):
        text = text[:begin] + value + text[end:]
    if json.loads(text)['patterns'] != patterns:
        raise ValueError('Pattern serialization changed candidate data')
    return (b'\xef\xbb\xbf' if original.startswith(b'\xef\xbb\xbf') else b'') + text.encode('utf-8')


def occurrence(pattern, resource, start, duration, *, bone='', follow=False):
    ordinal = pattern['nextPresentationOccurrenceOrdinal']
    pattern['nextPresentationOccurrenceOrdinal'] += 1
    row = dict(occurrenceId=pattern['patternId'] + '.presentation.' + str(ordinal),
               resourceId=resource['resourceId'], startMs=start, durationMs=duration,
               positionOffset=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1],
               fadeInMs=0, fadeOutMs=0, dissolveStart=.95, dissolveEnd=1,
               brightnessMultiplier=1, volume=1, followBoss=follow, debugRender=True,
               bone=bone, boneTarget='BODY', regionId='', cardSymbol='NONE', cardColor='NONE',
               anchorKind='BOSS', worldId='', logicOccurrenceId='', worldOccurrenceId='')
    pattern.setdefault('presentationOccurrences', []).append(row)
    return row


def clip_start(pattern, clip):
    matches, clock = [], 0
    for stage in pattern['stages']:
        for animation in stage['animationOccurrences']:
            if animation['runtimeClip'] == clip:
                matches.append(clock + animation['startOffsetMs'])
                if animation['sourceStartMs'] or animation['playRate'] != 1:
                    raise ValueError('Reinspect the edited source clip before retiming flame cues')
        clock += stage['durationMs']
    if len(matches) != 1:
        raise ValueError(f'Expected one source clip in {pattern["patternId"]}: {clip}')
    return matches[0], pattern.get('durationMs', clock)


def wave_pattern(composition, profile, action_id, name, resource):
    reference = read(ROOT / f'Data/Animation/Reference/KoukuSaydon/{profile}.actionreference.json')
    action = next(a for a in reference['actions'] if a['sourceActionId'] == action_id)
    chosen = [next(s for s in action['stages'] if s['stageId'] == key) for key in ('stage-001', 'stage-003')]
    ordinal = composition['nextPatternOrdinal']
    composition['nextPatternOrdinal'] += 1
    pattern_id = 'KAKULSAYDON_G1_PATTERN_' + str(ordinal)
    gate = 'GATE1' if profile == 'MN_RPCT_05' else 'GATE3'
    p = dict(patternId=pattern_id, actorProfileId='MN_RPCT_05', gateId=gate,
             targetBossPlacementId='boss.kakulsaydon.' + ('g1' if gate == 'GATE1' else 'g3') + '.saydon',
             displayName=name, authoringStatus='DRAFT', category='MECHANIC',
             nextStageOrdinal=3, nextAnimationOrdinal=3, nextLogicOccurrenceOrdinal=1,
             nextSummonOccurrenceOrdinal=1, nextWorldOccurrenceOrdinal=1,
             nextSceneProfileOccurrenceOrdinal=1, nextPresentationOccurrenceOrdinal=1,
             stages=[], logicOccurrences=[], summonOccurrences=[], worldOccurrences=[],
             sceneProfileOccurrences=[], presentationOccurrences=[], resetBossToSpawn=False)
    for i, source_stage in enumerate(chosen, 1):
        slot, = source_stage['slots']
        animation = dict(occurrenceId=pattern_id + '.animation.' + str(i), profileId=profile,
                         sourceActionId=action_id, sourceStageId=source_stage['stageId'],
                         sourceSlotId=slot['slotId'], referenceRevision=reference['referenceRevision'],
                         runtimeClip=slot['runtimeClip'], startOffsetMs=0,
                         sourceStartMs=slot['sourceStartMs'], playMs=slot['playMs'],
                         playRate=slot['playRate'], endPolicy='EXACT')
        p['stages'].append(dict(stageId='STAGE_' + str(i), actionId=pattern_id + '.stage.' + str(i),
                                stageKind='ACTIVE' if i == 1 else 'RECOVERY',
                                durationMs=slot['playMs'], animationOccurrences=[animation]))
    end = 1904 + resource['durationMs']
    if end > sum(s['durationMs'] for s in p['stages']):
        p['stages'][-1]['durationMs'] = end - p['stages'][0]['durationMs']
        p['stages'][-1]['animationOccurrences'][0]['endPolicy'] = 'LOOP_TO_WINDOW'
    occurrence(p, resource, 1904, resource['durationMs'])
    composition['patterns'].append(p)
    return p


def prepare(manifests, output):
    output = repository_path(output)
    if not output.is_relative_to(ROOT / 'out'):
        raise ValueError('Preparation output must be under out/')
    snapshots, staged, rows, dependencies = {}, {}, [], {}

    def snapshot(relative):
        path = repository_path(relative)
        key = path.relative_to(ROOT).as_posix()
        if key not in snapshots:
            snapshots[key] = path.read_bytes() if path.exists() else None
        return snapshots[key]

    for manifest_path in manifests:
        manifest = read(repository_path(manifest_path))
        for path, expected in manifest.get('inputHashes', {}).items():
            actual = digest(repository_path(path).read_bytes())
            if actual != expected:
                raise ValueError(f'Generator input changed; regenerate candidate: {path}')
            dependencies[repository_path(path).relative_to(ROOT).as_posix()] = expected
        for row in manifest['documents']:
            candidate = repository_path(row['candidatePath']).read_bytes()
            document = json.loads(candidate)
            asset = row['effectAssetId']
            relative = 'Data/Effects/Authored/' + asset + '.effect.json'
            if row['path'] != relative or document['effectAssetId'] != asset:
                raise ValueError('Effect identity/destination mismatch')
            if any(old['effectAssetId'] == asset for old in rows):
                raise ValueError(f'Duplicate candidate asset: {asset}')
            snapshot(relative)
            staged[relative] = candidate
            rows.append(row)

    catalog_path, tree_path = 'Data/Effects/EffectCatalog.json', 'Data/Effects/EffectResourceTree.json'
    catalog, tree, composition = [json.loads(snapshot(p)) for p in (catalog_path, tree_path, COMPOSITION)]
    before_composition = copy.deepcopy(composition)
    resources = {r['assetId']: r for r in composition['presentationResources'] if r['kind'] == 'EFFECT'}
    nodes = {n['id']: n for n in tree['nodes']}
    references = {(r['kind'], r['assetId']): r for r in tree['references']}
    known = {r['effectAssetId']: r for r in catalog['effects']}

    def category(labels):
        parent, branch = 'root.v1', []
        for label in labels:
            branch.append(label)
            key = 'kouku.category.' + identity('/'.join(branch))
            value = dict(id=key, parentId=parent, kind='CATEGORY', displayName=label)
            if key in nodes and nodes[key] != value:
                raise ValueError(f'Category collision: {key}')
            nodes[key], parent = value, key
        return parent

    for row in rows:
        asset, name = row['effectAssetId'], row['displayName']
        entry = dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT',
                     authoringPath='Effects/Authored/' + asset + '.effect.json')
        if asset in known and known[asset] != entry:
            raise ValueError(f'Catalog identity collision: {asset}')
        if asset not in known:
            catalog['effects'].append(entry)
        reference = references.get(('V1', asset))
        parent = (reference['parentId'] if asset == STAFF and reference else
                  row.get('parentId') or (category(row['categoryPath']) if row.get('categoryPath') else reference['parentId']))
        if parent not in nodes:
            raise ValueError(f'Unknown candidate parent: {parent}')
        references[('V1', asset)] = dict(kind='V1', assetId=asset, displayName=name, parentId=parent)
        if asset not in resources:
            resource = dict(resourceId='kakulsaydon.effect.' + identity(asset), displayName=name,
                            defaultAnchorKind=row.get('defaultAnchorKind', 'BOSS'), kind='EFFECT',
                            assetId=asset, resourceKind='V1_EFFECT', elementId='', durationMs=row['durationMs'],
                            shape='BOX', colliderKind='GEOMETRY', halfExtents=[1, 1, 1], radiusM=3,
                            halfAngleDegrees=45)
            composition['presentationResources'].append(resource)
            resources[asset] = resource
        resources[asset].update(displayName=name, durationMs=row['durationMs'])
    if STAFF in resources:
        resources[STAFF]['displayName'] = '불뿜기'
        references[('V1', STAFF)]['displayName'] = '불뿜기'
        nodes[references[('V1', STAFF)]['parentId']]['displayName'] = '불뿜기'
        relative = 'Data/Effects/Authored/' + STAFF + '.effect.json'
        original = snapshot(relative)
        staged[relative] = replace_root_value(original, 'displayName', '불뿜기')

    changed_patterns = []
    p49 = next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_49')
    start, length = clip_start(p49, 'rpct00_att_battle_10_02')
    kouku_wave = resources[KOUKU_WAVE]
    if any(o['resourceId'] == kouku_wave['resourceId'] for o in p49['presentationOccurrences']):
        raise ValueError('Wave integration already exists; inspect instead of duplicating')
    wave_end = start + 1904 + kouku_wave['durationMs']
    if wave_end > length:
        # The fixed timeline owns the remaining decal tail; existing authored
        # animation boxes keep their speed, source range and stage durations.
        p49['durationMs'] = wave_end
    occurrence(p49, kouku_wave, start + 1904, kouku_wave['durationMs'])
    changed_patterns.append(p49['patternId'])
    p43 = next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_43')
    start, length = clip_start(p43, 'rpct00_att_battle_9_02')
    if any(o['resourceId'] == resources[SHARED]['resourceId'] for o in p43['presentationOccurrences']):
        raise ValueError('Backstep integration already exists; inspect instead of duplicating')
    occurrence(p43, resources[SHARED], start, min(length - start, resources[SHARED]['durationMs']),
               bone='bip001-head', follow=True)
    changed_patterns.append(p43['patternId'])
    for profile, action, name in [('MN_RPCT_05', 4219820, '세이튼_화염 파동'),
                                   ('MN_RPCT_07', 4219948, '쿠크세이튼_화염 파동')]:
        if any(p['displayName'] == name for p in composition['patterns']):
            raise ValueError(f'Existing user pattern needs explicit rebase: {name}')
        wave = resources[WAVE] if profile == 'MN_RPCT_05' else kouku_wave
        changed_patterns.append(wave_pattern(composition, profile, action, name, wave)['patternId'])
        ref = f'Data/Animation/Reference/KoukuSaydon/{profile}.actionreference.json'
        dependencies[ref] = digest((ROOT / ref).read_bytes())

    # No unrelated user pattern, lane, ID or authored payload can disappear.
    after_by_id = {p['patternId']: p for p in composition['patterns']}
    for p in before_composition['patterns']:
        after = copy.deepcopy(after_by_id[p['patternId']])
        if p['patternId'] in changed_patterns:
            after['presentationOccurrences'] = after['presentationOccurrences'][:-1]
            after['nextPresentationOccurrenceOrdinal'] -= 1
        if p['patternId'] == p49['patternId']:
            if 'durationMs' in p:
                after['durationMs'] = p['durationMs']
            else:
                after.pop('durationMs', None)
        if after != p:
            raise ValueError(f'Unrelated user pattern fields changed: {p["patternId"]}')
    composition['revision'] += 1
    from project_kouku_saydon_composition import (
        _publication_candidate, _saved_pattern_inventory, validate_document, validate_publishable,
    )
    # The regular publisher admits each complete Pattern separately. Existing
    # unfinished editor rows remain visible and must not be "fixed" or deleted
    # merely to validate this change (for example the empty P32 Showtime draft).
    _saved_pattern_inventory(composition, ROOT)
    checked = _publication_candidate(composition, set(changed_patterns))
    validate_document(checked, ROOT)
    validate_publishable(checked, ROOT)
    value = snapshots[COMPOSITION]
    for field in ('presentationResources', 'nextPatternOrdinal', 'revision'):
        value = replace_root_value(value, field, composition[field], rows=field == 'presentationResources')
    value = replace_pattern_records(value, composition['patterns'])
    staged[COMPOSITION] = value
    tree['nodes'], tree['references'] = list(nodes.values()), list(references.values())
    value = replace_root_value(snapshots[tree_path], 'nodes', tree['nodes'], rows=True)
    staged[tree_path] = replace_root_value(value, 'references', tree['references'], rows=True)
    staged[catalog_path] = payload(catalog)
    for suffix in ('', '.filters'):
        relative = 'Client/Default/Client.vcxproj' + suffix
        original = snapshot(relative)
        text = original.decode('utf-8-sig')
        newline = '\r\n' if '\r\n' in text else '\n'
        entries = []
        for row in rows:
            include = '..\\..\\' + row['path'].replace('/', '\\')
            if 'Include="' + include + '"' not in text:
                entries.append('    <None Include="' + include + '">' +
                               ('<Filter>96.DataFiles</Filter>' if suffix else '') + '</None>')
        if entries:
            prefix, close, tail = text.rpartition('</Project>')
            if not close or tail.strip():
                raise ValueError('Invalid Client project closure')
            text = prefix + '  <ItemGroup>' + newline + newline.join(entries) + newline + '  </ItemGroup>' + newline + close + tail
        ET.fromstring(text)
        staged[relative] = (b'\xef\xbb\xbf' if original.startswith(b'\xef\xbb\xbf') else b'') + text.encode('utf-8')

    files = []
    for relative, after in staged.items():
        before = snapshots[relative]
        if before == after:
            continue
        destination = output / 'after' / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(after)
        if before is not None:
            backup = output / 'before' / relative
            backup.parent.mkdir(parents=True, exist_ok=True)
            backup.write_bytes(before)
        files.append(dict(path=relative, beforeSha256=digest(before) if before is not None else None,
                          afterSha256=digest(after)))
    receipt = dict(installed=False, beforeRevision=before_composition['revision'],
                   afterRevision=composition['revision'], files=files, inputHashes=dependencies,
                   changedPatterns=changed_patterns, unchangedUserPatternsAndFields=True,
                   waveAnchor='One BOSS snapshot at first warning; all ten impact points share it',
                   manualVisualValidation='USER_PENDING')
    output.mkdir(parents=True, exist_ok=True)
    (output / 'receipt.json').write_bytes(payload(receipt))
    print(json.dumps(dict(output=str(output), files=len(files), patterns=changed_patterns,
                          beforeRevision=receipt['beforeRevision'], afterRevision=receipt['afterRevision'])))


def replace_atomically(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary = tempfile.mkstemp(prefix=path.name + '.', suffix='.pending', dir=path.parent)
    try:
        with os.fdopen(descriptor, 'wb') as stream:
            stream.write(value)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def apply(output):
    output = repository_path(output)
    receipt = read(output / 'receipt.json')
    if receipt['installed']:
        raise ValueError('Candidate has already been applied')
    active = subprocess.run(['powershell', '-NoProfile', '-Command',
                             "@(Get-Process -Name Client,Server -ErrorAction SilentlyContinue).Count"],
                            capture_output=True, text=True, check=True)
    if int(active.stdout.strip()):
        raise ValueError('Save and close Client/Server before applying the reviewed candidate')
    for relative, expected in receipt['inputHashes'].items():
        if digest(repository_path(relative).read_bytes()) != expected:
            raise ValueError(f'Stale generator input: {relative}')
    for row in receipt['files']:
        target = repository_path(row['path'])
        actual = digest(target.read_bytes()) if target.exists() else None
        if actual != row['beforeSha256']:
            raise ValueError(f'Stale authoring input: {target}; regenerate on latest saved data')
        if digest((output / 'after' / row['path']).read_bytes()) != row['afterSha256']:
            raise ValueError(f'Candidate changed after validation: {row["path"]}')
        if row['beforeSha256'] is not None and digest((output / 'before' / row['path']).read_bytes()) != row['beforeSha256']:
            raise ValueError(f'Backup changed after preparation: {row["path"]}')
    committed = []
    try:
        for row in receipt['files']:
            path = repository_path(row['path'])
            actual = digest(path.read_bytes()) if path.exists() else None
            if actual != row['beforeSha256']:
                raise ValueError(f'Concurrent authoring edit: {path}')
            replace_atomically(path, (output / 'after' / row['path']).read_bytes())
            committed.append(row)
    except Exception:
        for row in reversed(committed):
            path = repository_path(row['path'])
            if digest(path.read_bytes()) != row['afterSha256']:
                raise RuntimeError(f'Concurrent write during rollback; preserve both versions: {path}')
            if row['beforeSha256'] is None:
                path.unlink()
            else:
                replace_atomically(path, (output / 'before' / row['path']).read_bytes())
        raise
    receipt['installed'] = True
    (output / 'receipt.json').write_bytes(payload(receipt))
    print(json.dumps(dict(installed=True, revision=receipt['afterRevision'], files=len(committed))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--manifest', action='append', type=Path, default=[])
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    if args.apply:
        if args.manifest:
            parser.error('--apply consumes the reviewed receipt, not new manifests')
        apply(args.output)
    else:
        if not args.manifest:
            parser.error('Preparation requires at least one --manifest')
        prepare(args.manifest, args.output)
