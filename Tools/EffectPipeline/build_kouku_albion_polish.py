"""Stage Albion's source electric call groups and requested warning fill.

No catalog, Composition, Resources or authored source is installed here. The
registration manifest carries current source hashes for the caller's CAS commit.
"""
import argparse
import copy
import hashlib
from pathlib import Path
import re

import build_kouku_albion_cross_groups as albion
import build_kouku_gate1_full_restore as source
import build_kouku_showtime_warning_groups as warning

ROOT = source.ROOT
AUTHORED = ROOT / 'Data/Effects/Authored'
CATEGORY = ['KoukuSaydon', '3관문', '패턴', '세이튼', '알비온']
LINE = 'fx_mn_rpct_07_v.par_v_rpct_line_atk_01_loc_int'
CROSS_ASSET = 'effect.kouku.albion.cross.electric.impact'
THREE_ASSET = 'effect.kouku.albion.frontthree.electric.impact'
BACKSTEP_ASSET = 'effect.kouku.gate3.backstep.electric.threeway.authored'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest() if path.exists() else None


def world_space_electric(document):
    """Override runtime particle space; retain the raw source Required evidence."""
    assert document['effectAssetId'] in (CROSS_ASSET, THREE_ASSET, BACKSTEP_ASSET)
    for element in document['elements']:
        element['detail']['particle']['localSpace'] = False


def three_ray_memberships(asset_id):
    """Recover membership from the original stable-ID remap, never current TRS.

    The source Stage3 call IDs were composed with direction.0/1/2. Replaying
    only their identity remap preserves membership after arbitrary user edits
    to positions, rotations, delays or element ordering.
    """
    assert asset_id in (THREE_ASSET, BACKSTEP_ASSET)
    leaf = source.read(AUTHORED / ('effect.kouku.source.' + LINE + '.effect.json'))
    identity_template = dict(elements=[dict(id=element['id']) for element in leaf['elements']])
    assert len(identity_template['elements']) == 14
    prefix = 'manual.albion.frontthree.ray.' if asset_id == THREE_ASSET else 'manual.kouku.backstep.threeway.ray.'
    memberships = {}
    for ordinal in range(3):
        identity = albion.independent(identity_template, THREE_ASSET + '.direction.' + str(ordinal), '')
        identity = albion.independent(identity, THREE_ASSET, '')
        if asset_id == BACKSTEP_ASSET:
            identity = albion.independent(identity, BACKSTEP_ASSET, '')
        memberships[prefix + str(ordinal + 1)] = {element['id'] for element in identity['elements']}
    assert len(set.union(*memberships.values())) == 42
    return memberships


def split_three_rays(document):
    memberships = three_ray_memberships(document['effectAssetId'])
    by_id = {element['id']: element for element in document['elements']}
    assert len(by_id) == len(document['elements']) == 42
    assert set(by_id) == set.union(*memberships.values()), 'Current element IDs no longer match source rays'
    for group_id, element_ids in memberships.items():
        for element_id in element_ids:
            by_id[element_id]['groupId'] = group_id
    return memberships


def stage_group_edits(output):
    """Stage only the current cross/threeway/backstep documents, preserving edits."""
    records = []
    for asset_id in (CROSS_ASSET, THREE_ASSET, BACKSTEP_ASSET):
        target = AUTHORED / (asset_id + '.effect.json')
        raw = target.read_bytes()
        before_sha = hashlib.sha256(raw).hexdigest()
        before = source.read(target)
        assert sha(target) == before_sha, 'Source changed while reading'
        candidate = copy.deepcopy(before)
        if asset_id == CROSS_ASSET:
            assert len(candidate['elements']) == 56
            assert all(not element['actionCueAttachment']['enabled'] for element in candidate['elements'])
            changed = sum(bool(element['detail']['particle']['localSpace']) for element in candidate['elements'])
            world_space_electric(candidate)
            delta = dict(field='detail.particle.localSpace', changedElements=changed,
                sourceRequiredLiteralsPreserved=True, outerBossFollowOwnedByComposition=True)
        else:
            memberships = split_three_rays(candidate)
            delta = dict(field='groupId', changedElements=sum(a['groupId'] != b['groupId']
                for a, b in zip(before['elements'], candidate['elements'])),
                groups=[dict(groupId=key, elementIds=sorted(value), sourceSide=side)
                    for (key, value), side in zip(memberships.items(), ('right', 'center', 'left'))])
        restored = copy.deepcopy(candidate)
        for original, element in zip(before['elements'], restored['elements']):
            if asset_id == CROSS_ASSET:
                element['detail']['particle']['localSpace'] = original['detail']['particle']['localSpace']
            else:
                element['groupId'] = original['groupId']
        assert restored == before, 'Unexpected change outside requested field'
        candidate_path = output / 'candidate' / target.name
        before_path = output / 'before' / target.name
        before_path.parent.mkdir(parents=True, exist_ok=True)
        before_path.write_bytes(raw)
        source.write(candidate_path, candidate)
        assert sha(target) == before_sha, 'Source changed during staging'
        records.append(dict(effectAssetId=asset_id, targetPath=str(target), candidatePath=str(candidate_path),
            beforePath=str(before_path), beforeSha256=before_sha, candidateSha256=sha(candidate_path), **delta))
    source.write(output / 'registration.json', dict(scope='CURRENT_SOURCE_FIELDS_ONLY_NO_INSTALL', documents=records))
    return records


def stage_threeway_world_space(output):
    """Disable particle local space in the latest three-ray documents only."""
    records = []
    for asset_id in (THREE_ASSET, BACKSTEP_ASSET):
        target = AUTHORED / (asset_id + '.effect.json')
        raw = target.read_bytes()
        before_sha = hashlib.sha256(raw).hexdigest()
        before = source.read(target)
        assert sha(target) == before_sha, 'Source changed while reading'
        candidate = copy.deepcopy(before)
        changed = sum(bool(element['detail']['particle']['localSpace']) for element in candidate['elements'])
        world_space_electric(candidate)
        restored = copy.deepcopy(candidate)
        for original, element in zip(before['elements'], restored['elements']):
            element['detail']['particle']['localSpace'] = original['detail']['particle']['localSpace']
        assert restored == before, 'Unexpected change outside requested particle space'
        candidate_path = output / 'candidate' / target.name
        before_path = output / 'before' / target.name
        before_path.parent.mkdir(parents=True, exist_ok=True)
        before_path.write_bytes(raw)
        candidate_raw, replacements = re.subn(rb'("localSpace"\s*:\s*)true\b', rb'\g<1>false', raw)
        assert replacements == changed, 'Unexpected runtime particle space fields'
        candidate_path.parent.mkdir(parents=True, exist_ok=True)
        candidate_path.write_bytes(candidate_raw)
        assert source.read(candidate_path) == candidate, 'Unexpected raw field replacement'
        assert sha(target) == before_sha, 'Source changed during staging'
        records.append(dict(effectAssetId=asset_id, targetPath=str(target), candidatePath=str(candidate_path),
            beforePath=str(before_path), beforeSha256=before_sha, candidateSha256=sha(candidate_path),
            field='detail.particle.localSpace', changedElements=changed,
            sourceRequiredLiteralsPreserved=True, outerBossFollowOwnedByComposition=True))
    source.write(output / 'registration.json', dict(scope='CURRENT_SOURCE_FIELDS_ONLY_NO_INSTALL', documents=records))
    return records


def stage_blue_circle_fill(output):
    """Fill the existing blue warning at its existing 2-second lightning onset."""
    records = []
    for asset_id in ('effect.kouku.albion.bluecircle.warning',
                     'effect.kouku.albion.bluecircle.warning.impact.runtime'):
        target = AUTHORED / (asset_id + '.effect.json')
        raw = target.read_bytes()
        before_sha = hashlib.sha256(raw).hexdigest()
        before = source.read(target)
        assert sha(target) == before_sha, 'Source changed while reading'
        candidate = copy.deepcopy(before)
        circles = [element for element in candidate['elements'] if
            element['material']['sourceProfile'].get('runtimeShaderProfileId') == 'effect.ue3.kouku-3600-native.v1']
        assert len(circles) == 1
        circle = circles[0]
        completion = circle['detail']['timing']['startDelaySeconds'] + circle['detail']['timing']['lifeTimeSeconds']
        assert completion == 2.0
        if len(candidate['elements']) > 1:
            other = [element for element in candidate['elements'] if element is not circle]
            first_lightning = min(element['detail']['timing']['startDelaySeconds'] +
                                  element['sourceRecipe']['emitterDelaySeconds'] for element in other)
            assert first_lightning == completion, 'Warning end and lightning start no longer match'
        fill = warning.animate_radial_fill(circle)
        track = next(track for track in circle['sourceTransformTrack']['materialParameterTracks']
                     if track['name'] == 'inner')
        # This requested warning fills at the lightning boundary, rather than
        # the reusable helper's ordinary fade-out start. Keep alpha untouched.
        track['keys'][-1]['timeSeconds'] = completion + circle['sourceTransformTrack']['sourceTimeOriginSeconds']
        fill.update(fillCompleteSeconds=completion, boundary='EXISTING_LIGHTNING_START', existingAlphaPreserved=True)
        stripped = copy.deepcopy(candidate)
        for old, new in zip(before['elements'], stripped['elements']):
            if new['id'] == circle['id']:
                new['sourceTransformTrack'] = copy.deepcopy(old['sourceTransformTrack'])
        assert stripped == before, 'Unexpected change outside blue circle material track'
        candidate_path = output / 'candidate' / target.name
        before_path = output / 'before' / target.name
        before_path.parent.mkdir(parents=True, exist_ok=True)
        before_path.write_bytes(raw)
        source.write(candidate_path, candidate)
        assert sha(target) == before_sha, 'Source changed during staging'
        records.append(dict(effectAssetId=asset_id, targetPath=str(target), candidatePath=str(candidate_path),
            beforePath=str(before_path), beforeSha256=before_sha, candidateSha256=sha(candidate_path),
            changedElementId=circle['id'], field='sourceTransformTrack.materialParameterTracks',
            fill=fill, unchangedCompositionFit=True))
    source.write(output / 'registration.json', dict(scope='BLUE_CIRCLE_NATIVE_INNER_ONLY_NO_INSTALL', documents=records))
    return records


def stage(output):
    output.mkdir(parents=True, exist_ok=True)
    action_path = ROOT / 'out/KoukuPatternGroups20260912/albion_cross/source/MN_RPCT_07.4219903.action.json'
    action = source.read(action_path)
    leaf_path = AUTHORED / ('effect.kouku.source.' + LINE + '.effect.json')
    leaf = source.read(leaf_path)
    records, call_evidence = [], []
    threeway = None

    def emit(doc, category, duration=None, **evidence):
        target = AUTHORED / (doc['effectAssetId'] + '.effect.json')
        candidate = output / 'candidate' / target.name
        source.write(candidate, doc)
        row = albion.record(doc, CATEGORY + [category], **evidence)
        row.update(candidatePath=str(candidate), targetPath=str(target), beforeSha256=sha(target),
                   candidateSha256=sha(candidate))
        if duration is not None:
            row.update(durationMs=duration, durationBasis='EXISTING_REGISTERED_VISIBILITY_WINDOW')
        records.append(row)

    # The installed native crossplane's terminal edge lies on -X. CPU playback
    # at .1 s gives endpoints [-9.5, 4.5] m. Source center yaw 180 points +X;
    # this group-only -90 degree basis maps it to the actor's +Z exactly once.
    # Keep the source leaf, its mesh rotation, dimensions and side angles intact.
    for stage_index, count, suffix, name, category in (
            (0, 4, 'cross.electric.impact', '알비온_십자전기폭발', '십자 전기 폭발'),
            (3, 3, 'frontthree.electric.impact', '알비온_전방세갈레전기폭발', '전방 3갈레 전기 폭발')):
        stage_row = next(row for row in action['stages'] if row['stageIndex'] == stage_index)
        calls = [row for row in stage_row['notifies'] if row['sourceType'] == 'PlayParticleEffect'
                 and any(ref['objectPath'].lower() == LINE for ref in row['assetReferences'])]
        assert len(calls) == count
        first = min(row['localTimeSeconds'] for row in calls)
        parts, evidence = [], []
        asset = 'effect.kouku.albion.' + suffix
        for index, notify in enumerate(calls):
            cue = albion.decode_particle(notify)
            assert cue['enabled'] and cue['localTransform']['position'] == [-1, 0, 0]
            assert all(abs(value - .8) < 1e-7 for value in cue['localTransform']['scale'])
            transform = copy.deepcopy(cue['localTransform'])
            transform['rotationDegrees'][1] -= 90
            x, y, z = transform['position']
            transform['position'] = [-z, y, x]
            delay = notify['localTimeSeconds'] - first
            part = albion.instance(leaf, asset + '.direction.' + str(index), name, transform, delay)
            parts.append(part)
            evidence.append(dict(notifyId=notify['notifyId'], sourceTimeSeconds=notify['localTimeSeconds'],
                standaloneDelaySeconds=delay, sourceCue=cue, independentTransform=transform))
        expected = [0, 16384, 32768, 49152] if count == 4 else [23119, 32768, 41870]
        assert sorted(row['sourceCue']['sourceFRotator'][1] for row in evidence) == expected
        document = albion.merge_documents(parts, asset, name)
        world_space_electric(document)
        if count == 3:
            split_three_rays(document)
            threeway = document
        assert len(document['elements']) == count * len(leaf['elements'])
        emit(document, category, sourceActionId=4219903, sourceStageIndex=stage_index,
             sourceParticleSystem=LINE, sourceCallCount=count, independentForwardBasisYawDegrees=-90)
        records[-1]['defaultAnchorKind'] = 'BOSS'
        call_evidence.append(dict(effectAssetId=asset, calls=evidence))

    for suffix in ('fourfan.warning.single', 'fourfan.warning'):
        path = AUTHORED / ('effect.kouku.albion.' + suffix + '.effect.json')
        before = source.read(path)
        document = copy.deepcopy(before)
        fill = [warning.animate_radial_fill(element) for element in document['elements']]
        stripped = copy.deepcopy(document)
        for original, changed in zip(before['elements'], stripped['elements']):
            changed['sourceTransformTrack'] = copy.deepcopy(original['sourceTransformTrack'])
        assert stripped == before  # No size, fade, material, source emitter or TRS edits.
        emit(document, '네 방향 부채꼴', duration=1700, fitEffectToDuration=True,
             fillCurves=fill, sourceChange='PROJECT_REQUESTED_NATIVE_INNER_TRACK_ONLY')

    disappear_path = AUTHORED / 'effect.kouku.gate3.showtime.saydon.disappear.effect.json'
    before = source.read(disappear_path)
    renamed = copy.deepcopy(before)
    renamed['displayName'] = '알비온_사라지기이펙트'
    assert {k: v for k, v in renamed.items() if k != 'displayName'} == {
        k: v for k, v in before.items() if k != 'displayName'}
    emit(renamed, '소멸 이펙트', duration=2201, existingCompositionResourceId='kakulsaydon.g1.presentation.66',
         sourceChange='DISPLAY_NAME_ONLY_SAME_ASSET_AND_RESOURCE')

    # Explicit user replacement: preserve this existing library identity while
    # adopting the new three-line visual; it is not an original backstep restore.
    backstep_asset = 'effect.kouku.gate3.backstep.electric.threeway.authored'
    assert threeway is not None and (AUTHORED / (backstep_asset + '.effect.json')).exists()
    backstep = albion.independent(threeway, backstep_asset, '백스텝_전방세갈레전기폭발')
    world_space_electric(backstep)
    split_three_rays(backstep)
    emit(backstep, '백스텝 후 감전빔', sourceChange='PROJECT_TUNED_EXPLICIT_USER_REPLACEMENT',
         replacementVisualSource=threeway['effectAssetId'])
    tree = source.read(ROOT / 'Data/Effects/EffectResourceTree.json')
    parent = next(row['parentId'] for row in tree['references'] if row['assetId'] == backstep_asset)
    nodes = {row['id']: row for row in tree['nodes']}
    category = []
    while parent in nodes:
        node = nodes[parent]
        category.insert(0, node['displayName'])
        parent = node.get('parentId')
    records[-1]['categoryPath'] = category
    records[-1]['defaultAnchorKind'] = 'BOSS'

    source.write(output / 'source-calls.json', dict(sourceActionPath=str(action_path),
        sourceActionSha256=sha(action_path), sourceLeafPath=str(leaf_path), sourceLeafSha256=sha(leaf_path),
        nativeBeamDirection='-X', actorForward='+Z', actorProfileId='MN_RPCT_05', groups=call_evidence))
    source.write(output / 'registration.json', dict(scope='CANDIDATES_ONLY_NO_TIMELINE_PLACEMENTS',
        documents=records, sharedShaderChanges=False,
        preserveExistingAssetIds=['effect.kouku.albion.fourfan.warning.single',
            'effect.kouku.albion.fourfan.warning', 'effect.kouku.gate3.showtime.saydon.disappear',
            backstep_asset]))
    return records


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuAlbionPolish20260915')
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--current-group-edits', action='store_true',
        help='Read latest saved documents; stage only cross runtime space and threeway manual groups.')
    mode.add_argument('--current-blue-circle-fill', action='store_true',
        help='Read latest blue warning documents; fill native inner at their existing lightning onset.')
    mode.add_argument('--current-threeway-world-space', action='store_true',
        help='Read latest three-ray documents; disable only runtime particle local space.')
    args = parser.parse_args()
    action = (stage_threeway_world_space if args.current_threeway_world_space else
        stage_blue_circle_fill if args.current_blue_circle_fill else stage_group_edits if args.current_group_edits else stage)
    rows = action(args.output.resolve())
    print('Staged', len(rows), 'documents:', ', '.join(row['effectAssetId'] for row in rows))
