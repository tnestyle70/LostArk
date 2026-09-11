"""Project action 4219776 source notifies onto the authored spider-counter clips.

Uses the existing Cascade projector and V1 Effect presentation. The two source
stage documents are installed explicitly; current pattern timing and user edits
are preserved with compare-before-write checks. Native programs are recovered by
the existing native-material pipeline and supplied through --native-material-patch.
"""
from pathlib import Path
import argparse
import copy
import json
import xml.etree.ElementTree as ET

import build_kouku_gate1_full_restore as source
from extract_ue3_skeletal_mesh_sockets import parse_socket_contract

ROOT = source.ROOT
ACTION_ID = 4219776
PATTERN_ID = 'KAKULSAYDON_G1_PATTERN_15'
STAGES = {1: ('rpcz00_att_battle_6_02', '눈빛·먼지'),
          2: ('rpcz00_att_battle_6_03', '검정·빨강 바닥과 돌진')}


def asset_id(stage):
    return f'effect.kouku.gate2.{ACTION_ID}.stage{stage}.full.restore'


def encoded(value):
    return (json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8')


def project(evidence, material_patch):
    source.ACTION = source.SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCZ_00.action-effects.json'
    source.SELECTED = {ACTION_ID: ([0, 1, 2, 3], '거미카운터')}
    sockets = source.SOURCE / 'CanonicalSource/Character/UModelExports/MN_RPCZ_00/Export/MN_RPCZ_00/mesh/mn_rpcz_00_sk.props.txt'
    source.write(evidence / 'source_socket_contract.json', parse_socket_contract(sockets))
    index, notifies, occurrences, records = source.acquire(evidence)
    assert len(occurrences) == 12
    docs, durations = {}, {}
    for stage, (clip, label) in STAGES.items():
        stage_evidence = evidence / f'stage{stage}'
        source.write(stage_evidence / 'source_socket_contract.json', source.read(evidence / 'source_socket_contract.json'))
        selected_notifies = [copy.deepcopy(n) for n in notifies if n['selectedStage'] == stage]
        by_notify = {n['notifyId']: n for n in selected_notifies}
        selected = [copy.deepcopy(o) for o in occurrences if o['sourceNotify'] in by_notify]
        for n in selected_notifies:
            n['globalTimeSeconds'] = n['localTimeSeconds']
        for occurrence in selected:
            occurrence['sourceTimeSeconds'] = by_notify[occurrence['sourceNotify']]['localTimeSeconds']
        source.SELECTED = {ACTION_ID: ([stage], label)}
        destination = stage_evidence / 'projected'
        source.project(stage_evidence, index, selected_notifies, selected, records, destination, material_patch)
        doc = source.read(destination / f'effect.kouku.gate1.{ACTION_ID}.full.restore.effect.json')
        doc['effectAssetId'] = asset_id(stage)
        doc['displayName'] = '쿠크 거미카운터 원본 ' + {1: '눈빛·먼지', 2: '돌진·바닥'}[stage]
        assert len(doc['displayName'].encode('utf8')) <= 64
        for element in doc['elements']:
            element['groupId'] = f'kouku.{ACTION_ID}.stage{stage}.full.restore'
            if element['sourceRecipe']['rendererShape'] != 'ribbon':
                continue
            recipe = element['sourceRecipe']
            typed = next(m for m in recipe['modules'] if m['className'] == 'particlemoduletypedataribbon')
            values = {v['propertyPath']: v['value'] for v in typed['literals']}
            per_unit = next(m for m in recipe['modules'] if m['className'] == 'particlemodulespawnperunit')
            unit_scalar = next(v['value'] for v in per_unit['literals'] if v['propertyPath'] == 'unitscalar')
            density = next(d for d in per_unit['distributions'] if d['propertyPath'] == 'spawnperunit')
            density_min, density_max = source.distribution_bounds(density)
            assert density_min == density_max and density_min > 0
            size = next(d for m in recipe['modules'] if m['className'] == 'particlemodulesize'
                        for d in m['distributions'] if d['propertyPath'] == 'startsize')
            assert size['defaultMinimum'] == size['defaultMaximum'] and not size['keys']
            size_values = size['lookupTable'][2::size['lookupTableChunkSize']] if size['lookupTable'] else [size['defaultMinimum'][0]]
            assert size['operation'] == 1 and len(set(size_values)) == 1
            width = abs(size_values[0]) * .01
            assert width > 0
            element['runtimeCarrier'] = dict(formatVersion=1, kind='cascadeRibbonV1', admission='bounded',
                                            typeDataModuleStableId=typed['stableId'])
            element['detail']['trail'].update(maxPoints=int(values['maxparticleintrailcount']),
                pointLifeTimeSeconds=max(element['detail']['particle']['lifeTimeSeconds']),
                sampleIntervalSeconds=1 / 60, minimumDistance=unit_scalar / density_min * .01,
                startWidth=width, endWidth=width, faceCamera=True,
                tilingDistanceWorldUnits=values['tilingdistance'] * .01,
                distanceTessellationStepWorldUnits=values['distancetessellationstepsize'] * .01)
            doc['version'] = 15
            doc['runtimeExtensions'] = dict(formatVersion=1, bakedEdgeHistories=[])
        assert len(doc['elements']) == {1: 5, 2: 9}[stage]
        for element in doc['elements']:
            for resource in element['resources'] + element['material']['sourceProfile']['textures']:
                assert (ROOT / 'Client/Bin/Resources' / resource['assetId']).is_file(), resource
        docs[stage] = doc
        durations[stage] = source.read(stage_evidence / 'projection.json')['durationMs'][str(ACTION_ID)]
        source.write(evidence / 'candidate' / f'{asset_id(stage)}.effect.json', doc)
    return docs, durations


def connect(composition, durations):
    result = copy.deepcopy(composition)
    pattern = next(p for p in result['patterns'] if p['patternId'] == PATTERN_ID)
    assert pattern['actorProfileId'] == 'MN_RPCZ_00' and pattern['gateId'] == 'GATE2'
    assert pattern['targetBossPlacementId'] == 'boss.kakulsaydon.g2.kouku'
    pattern_duration = sum(stage['durationMs'] for stage in pattern['stages'])
    resources = {}
    for stage, (clip, label) in STAGES.items():
        matching = [r for r in result['presentationResources'] if r.get('assetId') == asset_id(stage)]
        assert len(matching) <= 1
        if matching:
            resources[stage] = matching[0]
            continue
        ordinal = result['nextPresentationResourceOrdinal']
        identifier = f'kakulsaydon.g1.presentation.{ordinal}'
        assert not any(r['resourceId'] == identifier for r in result['presentationResources'])
        row = dict(resourceId=identifier, displayName=f'쿠크_거미카운터({clip})_{label}_FullRestore',
            defaultAnchorKind='BOSS', kind='EFFECT', assetId=asset_id(stage), resourceKind='V1_EFFECT',
            elementId='', durationMs=durations[stage], shape='BOX', colliderKind='GEOMETRY',
            halfExtents=[1, 1, 1], radiusM=3, halfAngleDegrees=45)
        result['presentationResources'].append(row)
        result['nextPresentationResourceOrdinal'] += 1
        resources[stage] = row
    base_time, links = 0, []
    for stage in pattern['stages']:
        for animation in stage['animationOccurrences']:
            selected = next((i for i, (clip, _) in STAGES.items() if clip == animation['runtimeClip']), None)
            if selected is None:
                continue
            assert animation['sourceActionId'] == ACTION_ID
            assert animation['sourceStageId'] == f'stage-{selected:03}'
            assert animation['sourceStartMs'] == 0 and animation['playRate'] == 1
            start = base_time + animation['startOffsetMs']
            remaining = pattern_duration - start
            assert remaining > 0, ('Source clip starts outside the existing pattern', animation)
            # Source particles can have a tail after the last animation. The
            # invocation must still stop at the existing pattern boundary;
            # extending that boundary would change the user's gameplay timing.
            invocation_duration = min(durations[selected], remaining)
            resource = resources[selected]
            matching = [p for p in pattern['presentationOccurrences']
                        if p['resourceId'] == resource['resourceId'] and p['startMs'] == start]
            assert len(matching) <= 1
            if not matching:
                ordinal = pattern['nextPresentationOccurrenceOrdinal']
                identifier = f'{PATTERN_ID}.presentation.{ordinal}'
                assert not any(p['occurrenceId'] == identifier for p in pattern['presentationOccurrences'])
                matching = [dict(occurrenceId=identifier, resourceId=resource['resourceId'], startMs=start,
                    durationMs=invocation_duration, positionOffset=[0, 0, 0], rotationDegrees=[0, 0, 0],
                    scale=[1, 1, 1], fadeInMs=0, fadeOutMs=0, dissolveStart=1, dissolveEnd=1,
                    brightnessMultiplier=1, volume=1, followBoss=True, debugRender=False, bone='',
                    boneTarget='BODY', regionId='', cardSymbol='NONE', cardColor='NONE', anchorKind='BOSS',
                    worldId='', logicOccurrenceId='', worldOccurrenceId='')]
                pattern['presentationOccurrences'].extend(matching)
                pattern['nextPresentationOccurrenceOrdinal'] += 1
            elif matching[0]['durationMs'] > remaining:
                # Repair only an invalid tail; preserve shorter user tuning.
                matching[0]['durationMs'] = remaining
            assert start + matching[0]['durationMs'] <= pattern_duration
            links.append(dict(animationOccurrenceId=animation['occurrenceId'], clip=animation['runtimeClip'],
                              startMs=start, durationMs=matching[0]['durationMs'],
                              sourceDurationMs=durations[selected], patternDurationMs=pattern_duration,
                              presentationOccurrenceId=matching[0]['occurrenceId'], assetId=asset_id(selected)))
        base_time += stage['durationMs']
    assert len(links) == 9 and sum(x['clip'].endswith('_03') for x in links) == 3
    if result != composition:
        result['revision'] += 1
    return result, links


def stage_install(docs, durations, evidence, replace_authored=False):
    writes = []
    def stage(path, updated):
        current = path.read_bytes() if path.exists() else None
        writes.append((path, current, updated))
    for doc in docs.values():
        path = ROOT / 'Data/Effects/Authored' / (doc['effectAssetId'] + '.effect.json')
        if path.exists() and not replace_authored:
            assert source.read(path) == doc, f'Existing authored edits differ; preserve {path}'
        stage(path, encoded(doc))
    catalog_path = ROOT / 'Data/Effects/EffectCatalog.json'
    catalog_bytes = catalog_path.read_bytes()
    catalog = json.loads(catalog_bytes)
    for doc in docs.values():
        row = dict(effectAssetId=doc['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT',
                   authoringPath='Effects/Authored/' + doc['effectAssetId'] + '.effect.json')
        matching = [r for r in catalog['effects'] if r['effectAssetId'] == row['effectAssetId']]
        assert not matching or matching == [row]
        if not matching:
            catalog['effects'].append(row)
    writes.append((catalog_path, catalog_bytes, encoded(catalog)))
    composition_path = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
    # Capture exactly the bytes used for projection, not a second read after it.
    composition_bytes = composition_path.read_bytes()
    composition = json.loads(composition_bytes)
    updated, links = connect(composition, durations)
    writes.append((composition_path, composition_bytes, encoded(updated)))
    for suffix in ('', '.filters'):
        path = ROOT / ('Client/Default/Client.vcxproj' + suffix)
        current = path.read_bytes()
        text = current.decode('utf8')
        newline = '\r\n' if '\r\n' in text else '\n'
        additions = []
        for doc in docs.values():
            name = '..\\..\\Data\\Effects\\Authored\\' + doc['effectAssetId'] + '.effect.json'
            if f'Include="{name}"' not in text:
                additions.append(f'    <None Include="{name}"><Filter>96.DataFiles</Filter></None>' if suffix
                                 else f'    <None Include="{name}" />')
        if additions:
            prefix, marker, tail = text.rpartition('</Project>')
            assert marker
            text = prefix + '  <ItemGroup>' + newline + newline.join(additions) + newline + '  </ItemGroup>' + newline + marker + tail
        ET.fromstring(text)
        writes.append((path, current, text.encode('utf8')))
    source.write(evidence / 'composition_before.json', composition)
    source.write(evidence / 'composition_candidate.json', updated)
    source.write(evidence / 'composition_links.json', links)
    return writes


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuSpiderSource20260911')
    parser.add_argument('--native-material-patch', type=Path, required=True)
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--replace-authored', action='store_true', help='Explicitly regenerate the two owned source documents.')
    args = parser.parse_args()
    docs, durations = project(args.evidence_root, args.native_material_patch)
    writes = stage_install(docs, durations, args.evidence_root, args.replace_authored)
    # Recheck every peer-owned input before any product file is written.
    for path, current, _ in writes:
        assert (path.read_bytes() if path.exists() else None) == current, f'Concurrent edit; rerun: {path}'
    changed = []
    for path, current, updated in writes:
        if updated == current:
            continue
        changed.append(str(path.relative_to(ROOT)))
        if args.install:
            assert (path.read_bytes() if path.exists() else None) == current, f'Concurrent edit; rerun: {path}'
            path.write_bytes(updated)
    source.write(args.evidence_root / 'installation.json', dict(installed=args.install,
        changedPaths=changed, elementCounts={str(s): len(d['elements']) for s, d in docs.items()},
        durationMs=durations, manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(installed=args.install, changedPaths=changed, durationMs=durations), ensure_ascii=False))


if __name__ == '__main__':
    main()
