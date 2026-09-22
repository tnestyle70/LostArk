"""Stage the fourteen editable flame-wave points and missing source ground fire.

Only out/ is written. Existing authored element IDs and unrelated edits survive;
the manifest records stable-ID field changes for a later reviewed disk merge.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path

from build_kouku_dance_grid_groups import remap, validate
from build_kouku_flame_wave_groups import duration_ms

ROOT = Path(__file__).resolve().parents[2]
PREFIX = 'effect.kouku.common.flame.wave.'
GROUND_SOURCE = 'effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firewave_01_loc_int'
COMPOSITION = 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
SPACING = 3.5
OLD_SPACING = 5.0
OLD_FORWARD = 4.330
FORWARD = round(SPACING * math.sqrt(3) / 2, 3)


def read(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))


def digest(value):
    return hashlib.sha256(value).hexdigest()


def encode(value):
    return (json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf-8')


def effect_path(asset):
    return 'Data/Effects/Authored/' + asset + '.effect.json'


def point_position(row, column, *, old=False):
    count = row if old else row + 1
    spacing = OLD_SPACING if old else SPACING
    forward = OLD_FORWARD if old else FORWARD
    return [round((column - (count + 1) / 2) * spacing, 3), 0,
            round(spacing + (row - 1) * forward, 3)]


def ground_elements(source):
    """Retain exact source material/axis/size/lifetime; author only the location.

    FireWave's ground ring and fire are at different positions in its long strip.
    Their constant source StartLocation distributions are centered for reuse at
    one flame-wave point. This is an authored composition, not an enabled notify.
    """
    result = []
    for suffix, native in (('particlespriteemitter_16', 2874), ('particlespriteemitter_23', 2873)):
        element, = [copy.deepcopy(e) for e in source['elements'] if e['sourceNode'].endswith('.' + suffix)]
        assert element['material']['sourceProfile']['runtimeShaderProfileId'] == f'effect.ue3.kouku-{native}-native.v1'
        assert not element['actionCueAttachment']['enabled']
        assert not element['transformInheritance']['enabled']
        assert not element.get('sourceTransformTrack') and not element.get('runtimeCarrier')
        axis = [l['value'] for m in element['sourceRecipe']['modules'] for l in m['literals']
                if l['propertyPath'] == 'lockaxisflags']
        assert axis == ['epal_z']
        locations = [d for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemodulelocation'
                     for d in m['distributions'] if d['propertyPath'] == 'startlocation']
        assert len(locations) == 1
        distribution = locations[0]
        assert distribution['operation'] == 1 and distribution['componentCount'] == 3
        assert distribution['lookupTableNumElements'] == 1 and len(distribution['lookupTable']) == 8
        assert distribution['lookupTable'][2:5] == distribution['lookupTable'][5:8]
        assert not distribution['keys']
        distribution['lookupTable'] = [0.0] * 8
        distribution['defaultMinimum'] = distribution['defaultMaximum'] = [0.0] * 4
        element['sourceRecipe']['authoredModuleOverrides'] = True
        element['detail']['particle']['initialPositionMin'] = [0, 0, 0]
        element['detail']['particle']['initialPositionMax'] = [0, 0, 0]
        element['detail']['transform']['position'] = [0, .04, 0]
        element['displayName'] = 'FireWave ground | ' + suffix
        result.append(element)
    return result


def placed_ground(leaves, group, position, start, scope):
    rows = copy.deepcopy(leaves)
    ids = {e['id']: PREFIX + 'ground.' + digest((scope + '/' + e['id']).encode())[:20] for e in rows}
    rows = remap(rows, ids)
    for element in rows:
        element['groupId'] = group
        element['detail']['transform']['position'] = [position[0], position[1] + .04, position[2]]
        element['detail']['timing']['startDelaySeconds'] += start
        event = 'user-authored/' + scope + '/ground'
        element['sourceNode'] = event + '|' + element['sourcePresentation']['sourceObjectPath']
        element['sourcePresentation'].update(sourceActionCueId=event, sourceEventId=event,
                                              sourceTimeSeconds=start)
    return rows


def refine_full(before, leaves):
    document = copy.deepcopy(before)
    assert document['particleSystem']['uniformScaleMultiplier'] == 1
    assert document['particleSystem']['yawOffsetDegrees'] == 0
    original_groups = {}
    for element in document['elements']:
        original_groups.setdefault(element['groupId'], []).append(element)
    assert set(original_groups) == {f'flame-wave.r{r}.c{c}' for r in range(1, 5) for c in range(1, r + 1)}
    points = []
    for row in range(1, 5):
        for column in range(1, row + 2):
            point = f'flame-wave.r{row}.c{column}'
            group = 'manual.' + point
            position = point_position(row, column)
            if column <= row:
                members = original_groups[point]
                baseline = point_position(row, column, old=True)
            else:
                # Clone from the unmodified source document, not the shifted row.
                original = [e for e in before['elements'] if e['groupId'] == f'flame-wave.r{row}.c{row}']
                ids = {e['id']: PREFIX + 'point.' + digest((point + '/' + e['id']).encode())[:20] for e in original}
                members = remap(copy.deepcopy(original), ids)
                baseline = point_position(row, row, old=True)
                for element in members:
                    element['sourceNode'] = element['sourceNode'].replace(f'flame-wave.r{row}.c{row}', point)
                    for key in ('sourceActionCueId', 'sourceEventId'):
                        if key in element['sourcePresentation']:
                            element['sourcePresentation'][key] = element['sourcePresentation'][key].replace(f'flame-wave.r{row}.c{row}', point)
                document['elements'].extend(members)
            assert len(members) == 26
            delta = [position[i] - baseline[i] for i in range(3)]
            for element in members:
                element['groupId'] = group
                transform = element['detail']['transform']
                transform['position'] = [round(transform['position'][i] + delta[i], 6) for i in range(3)]
            # Existing per-point authored timing is retained even if a user retimed it.
            impact = min(e['detail']['timing']['startDelaySeconds'] for e in members if e['kind'] != 'light')
            document['elements'].extend(placed_ground(leaves, group, position, impact, point))
            points.append(dict(groupId=group, positionMeters=position, row=row, column=column, impactSeconds=impact))
    assert len(document['elements']) == 392
    assert len({e['groupId'] for e in document['elements']}) == 14
    return document, points


def field_changes(before, after, path=()):
    if isinstance(before, dict) and isinstance(after, dict):
        changes = []
        for key in sorted(before.keys() | after.keys()):
            if key not in before:
                changes.append(dict(fieldPath=list(path + (key,)), beforeExists=False, after=after[key]))
            elif key not in after:
                changes.append(dict(fieldPath=list(path + (key,)), before=before[key], afterExists=False))
            else:
                changes.extend(field_changes(before[key], after[key], path + (key,)))
        return changes
    return [] if before == after else [dict(fieldPath=list(path), before=before, after=after)]


def stable_effect_edits(before, after):
    original = {e['id']: e for e in before['elements']}
    edits = []
    for element in after['elements']:
        if element['id'] not in original:
            edits.append(dict(collection='elements', stableKey='id', stableId=element['id'], operation='append', value=element))
        else:
            changes = field_changes(original[element['id']], element)
            if changes:
                edits.append(dict(collection='elements', stableKey='id', stableId=element['id'], operation='fields', changes=changes))
    return edits


def prepare_composition_candidate(manifest, output):
    """Merge guarded duration fields into the latest disk text without reformatting.

    No live file is changed. Every edit locates stable resource/pattern/occurrence
    IDs and rejects a concurrent change to the same field. Whitespace and all
    unrelated tokens remain byte-identical, including a possible UTF-8 BOM.
    """
    snapshot = (ROOT / COMPOSITION).read_bytes()
    text = snapshot.decode('utf-8-sig')
    decoder = json.JSONDecoder()
    expected = json.loads(snapshot)
    replacements = []

    def skip(cursor):
        while text[cursor].isspace():
            cursor += 1
        return cursor

    def members(start):
        assert text[start] == '{'
        result, cursor = {}, skip(start + 1)
        while text[cursor] != '}':
            key_start = cursor
            key, cursor = decoder.raw_decode(text, cursor)
            cursor = skip(cursor)
            assert text[cursor] == ':'
            value_start = skip(cursor + 1)
            value, cursor = decoder.raw_decode(text, value_start)
            if key in result:
                raise ValueError('Duplicate JSON member: ' + key)
            result[key] = (value_start, cursor, value, key_start)
            cursor = skip(cursor)
            if text[cursor] == ',':
                cursor = skip(cursor + 1)
            else:
                assert text[cursor] == '}'
        return result

    def record(array_start, stable_key, identity):
        assert text[array_start] == '['
        cursor, matches = skip(array_start + 1), []
        while text[cursor] != ']':
            start = cursor
            value, cursor = decoder.raw_decode(text, cursor)
            if value[stable_key] == identity:
                matches.append(start)
            cursor = skip(cursor)
            if text[cursor] == ',':
                cursor = skip(cursor + 1)
            else:
                assert text[cursor] == ']'
        if len(matches) != 1:
            raise ValueError('Missing or duplicate stable ID: ' + identity)
        return matches[0]

    root = members(skip(0))
    for edit in manifest['compositionPatch']['stableEdits']:
        start = record(root[edit['collection']][0], edit['stableKey'], edit['stableId'])
        semantic, = [r for r in expected[edit['collection']] if r[edit['stableKey']] == edit['stableId']]
        if 'childCollection' in edit:
            start = record(members(start)[edit['childCollection']][0], edit['childStableKey'], edit['childStableId'])
            semantic, = [r for r in semantic[edit['childCollection']] if r[edit['childStableKey']] == edit['childStableId']]
        fields = members(start)
        for change in edit['changes']:
            if change['fieldPath'] != ['durationMs']:
                raise ValueError('Flame Composition patch only owns durationMs.')
            name = 'durationMs'
            if name in fields:
                begin, end, value, _ = fields[name]
                if change.get('beforeExists') is False or value != change['before']:
                    raise ValueError('Concurrent duration edit: ' + edit.get('childStableId', edit['stableId']))
                replacements.append((begin, end, str(change['after'])))
            else:
                if change.get('beforeExists') is not False:
                    raise ValueError('Expected duration field is now absent: ' + edit['stableId'])
                first_key = min(value[3] for value in fields.values())
                separator = text[start + 1:first_key]
                replacements.append((first_key, first_key, '"durationMs": ' + str(change['after']) + ',' + separator))
            semantic[name] = change['after']
    begin, end, revision, _ = root['revision']
    expected['revision'] = revision + 1
    replacements.append((begin, end, str(revision + 1)))
    replacements.sort(reverse=True)
    for i, (begin, end, replacement) in enumerate(replacements):
        if i and end > replacements[i - 1][0]:
            raise ValueError('Overlapping scalar patches.')
        text = text[:begin] + replacement + text[end:]
    assert json.loads(text) == expected, 'Minimal text patch changed unrelated values.'
    candidate = (b'\xef\xbb\xbf' if snapshot.startswith(b'\xef\xbb\xbf') else b'') + text.encode('utf-8')
    for folder, value in (('before', snapshot), ('candidate', candidate)):
        target = output / folder / COMPOSITION
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(value)
    manifest['compositionPatch'].update(beforeSha256=digest(snapshot), candidateSha256=digest(candidate),
        candidatePath=(output / 'candidate' / COMPOSITION).as_posix(), beforeRevision=revision,
        candidateRevision=revision + 1, scalarEdits=len(replacements), unrelatedTextPreserved=True)
    assert (ROOT / COMPOSITION).read_bytes() == snapshot, 'Composition changed while preparing the merge.'
    return manifest['compositionPatch']


def prepare(output):
    output = output.resolve()
    if not output.is_relative_to(ROOT / 'out') or output == ROOT / 'out':
        raise ValueError('Candidate output must be a dedicated directory under out/.')
    source_path = ROOT / effect_path(GROUND_SOURCE)
    source_snapshot = source_path.read_bytes()
    leaves = ground_elements(json.loads(source_snapshot))
    documents, points, snapshots = [], {}, {}
    for role in ('decal', 'full', 'full.koukusaydon'):
        relative = effect_path(PREFIX + role)
        snapshot = (ROOT / relative).read_bytes()
        snapshots[relative] = snapshot
        before = json.loads(snapshot)
        if role == 'decal':
            after = copy.deepcopy(before)
            group = 'manual.flame-wave.floor'
            for element in after['elements']:
                element['groupId'] = group
            after['elements'].extend(placed_ground(leaves, group, [0, 0, 0], 0, 'flame-wave.floor'))
        else:
            after, points[role] = refine_full(before, leaves)
        validation = validate(after)
        encoded = encode(after)
        for folder, data in (('before', snapshot), ('candidate', encoded)):
            target = output / folder / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        documents.append(dict(path=relative, effectAssetId=after['effectAssetId'],
            candidatePath=(output / 'candidate' / relative).as_posix(),
            beforeSha256=digest(snapshot), candidateSha256=digest(encoded),
            durationMs=duration_ms(after), validation=validation,
            stableEdits=stable_effect_edits(before, after)))

    composition_bytes = (ROOT / COMPOSITION).read_bytes()
    composition = json.loads(composition_bytes)
    composition_edits, preserved = [], []
    durations = {d['effectAssetId']: d['durationMs'] for d in documents}
    resources = {r['resourceId']: r for r in composition['presentationResources'] if r.get('assetId') in durations}
    for resource in resources.values():
        duration = durations[resource['assetId']]
        if duration != resource['durationMs']:
            composition_edits.append(dict(collection='presentationResources', stableKey='resourceId', stableId=resource['resourceId'],
                operation='fields', changes=[dict(fieldPath=['durationMs'], before=resource['durationMs'], after=duration)]))
    for pattern in composition['patterns']:
        maximum_end = 0
        for occurrence in pattern['presentationOccurrences']:
            resource = resources.get(occurrence['resourceId'])
            if not resource:
                continue
            if occurrence['durationMs'] != resource['durationMs']:
                preserved.append(dict(patternId=pattern['patternId'], occurrenceId=occurrence['occurrenceId'],
                    durationMs=occurrence['durationMs'], reason='User-authored duration differs from the default; preserved.'))
                continue
            duration = durations[resource['assetId']]
            maximum_end = max(maximum_end, occurrence['startMs'] + duration)
            if duration != occurrence['durationMs']:
                composition_edits.append(dict(collection='patterns', stableKey='patternId', stableId=pattern['patternId'],
                    childCollection='presentationOccurrences', childStableKey='occurrenceId', childStableId=occurrence['occurrenceId'],
                    operation='fields', changes=[dict(fieldPath=['durationMs'], before=occurrence['durationMs'], after=duration)]))
        old_end = pattern.get('durationMs', sum(s['durationMs'] for s in pattern['stages']))
        if maximum_end > old_end:
            change = dict(fieldPath=['durationMs'], after=maximum_end)
            if 'durationMs' in pattern:
                change['before'] = pattern['durationMs']
            else:
                change['beforeExists'] = False
            composition_edits.append(dict(collection='patterns', stableKey='patternId', stableId=pattern['patternId'],
                operation='fields', changes=[change]))
    before_path = output / 'before' / COMPOSITION
    before_path.parent.mkdir(parents=True, exist_ok=True)
    before_path.write_bytes(composition_bytes)
    manifest = dict(schema='lostark.kouku-flame-wave-refinement-candidate', version=1,
        stageOnly=True, installed=False, clientUIStarted=False, manualVisualValidation='USER_PENDING',
        documents=documents, points=points,
        sourceInputs={effect_path(GROUND_SOURCE): digest(source_snapshot)},
        compositionPatch=dict(path=COMPOSITION, beforeSha256=digest(composition_bytes), stableEdits=composition_edits,
                              preservedAuthoredOccurrences=preserved),
        layout=dict(rowCounts=[2, 3, 4, 5], groupCount=14, oldHorizontalSpacingM=5,
                    horizontalSpacingM=SPACING, oldForwardSpacingM=OLD_FORWARD, forwardSpacingM=FORWARD,
                    firstDistanceM=SPACING, spacingReductionPercent=30, groundHeightM=.04),
        groundSource=dict(assetId=GROUND_SOURCE, sourceEmitters=['particlespriteemitter_16', 'particlespriteemitter_23'],
            nativePrograms=[2874, 2873], authoredChanges=['Constant StartLocation recentered', 'Ground raised by 0.04 m'],
            unchanged=['Source material/textures', 'Source axis lock EPAL_Z', 'Source size, lifetime, alpha and color',
                       'Existing warning/pillar/WandDecal elements'],
            boundary='The disabled original FireWave notify is not enabled; these are authored independent ground elements.'),
        manualPath='Effect Tool V1 > Current Effect > Group by Anchor > manual.flame-wave.rN.cN > Group Center > Save Changes')
    prepare_composition_candidate(manifest, output)
    (output / 'manifest.json').write_bytes(encode(manifest))
    # Preparation is read-only with respect to every current authored input.
    for relative, snapshot in snapshots.items():
        assert (ROOT / relative).read_bytes() == snapshot, 'Source changed while preparing: ' + relative
    assert (ROOT / COMPOSITION).read_bytes() == composition_bytes, 'Composition changed while preparing.'
    assert source_path.read_bytes() == source_snapshot, 'Ground source changed while preparing.'
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuFlameWave20260922')
    args = parser.parse_args()
    result = prepare(args.output)
    print(json.dumps(dict(stageOnly=True, documents=len(result['documents']), groups=14,
                         output=str(args.output), preservedOccurrences=result['compositionPatch']['preservedAuthoredOccurrences'])))


if __name__ == '__main__':
    main()
