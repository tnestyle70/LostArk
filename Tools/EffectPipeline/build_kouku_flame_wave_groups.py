"""Stage a user-authored 1/2/3/4 flame wave from three installed V1 leaves.

The 300 ms row spacing, 1200 ms warning, ten locations and pillar multiplier
are authoring choices. Only the first impact's 3104 ms animation alignment
comes from the enabled WandDecal notify. No disabled FireWave notify is enabled.
This command never installs a document or changes a Composition/catalog/project.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import struct
from pathlib import Path

from build_kouku_dance_grid_groups import append_leaf, validate
from build_kouku_showtime_warning_groups import independent_document

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUT = ROOT / 'out/KoukuFlameWave20260914'
PREFIX = 'effect.kouku.common.flame.wave.'
FIRST_WARNING_MS = 1904
WARNING_MS = 1200
ROW_MS = 300
PILLAR_SCALE = 1.5
SOURCES = {
    'warning': 'fx_cm_02.light.par_m_light_001',
    'pillar': 'fx_mn_rpct_05_l.par_l_rpct_05_sk_04_11_loc_int',
    'decal': 'fx_mn_rpct_05_g.par_g_rpct_05_wand_decal_loc_int',
}
LABELS = {'warning': '화염파동_전조', 'pillar': '화염파동_불기둥1.5',
          'decal': '화염파동_바닥', 'full': '화염 파동_전조1~4·불기둥1~4',
          'full.koukusaydon': '화염 파동_전조1~4·불기둥1~4'}


def read(path: Path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write(path: Path, value) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')


def f32(value: float) -> float:
    return struct.unpack('<f', struct.pack('<f', value))[0]


def duration_ms(document) -> int:
    # CEffectPlayback::Calculate_ElementEndSeconds's conservative active+tail
    # contract, including independent source-emitter delays and LIGHT's no-tail.
    ends = []
    for element in document['elements']:
        if not element['visible']:
            continue
        timing, recipe, particle = (element['detail']['timing'],
                                   element['sourceRecipe'], element['detail']['particle'])
        active = f32(timing['lifeTimeSeconds'])
        if recipe['enabled'] and recipe['emitterDurationSeconds'] > 0 and recipe['emitterLoopCount']:
            active = f32(f32(recipe['emitterDurationSeconds']) * recipe['emitterLoopCount'])
        tail = 0 if element['kind'] == 'light' else f32(max(particle['lifeTimeSeconds']) * particle['sourceScale']['lifeTime'])
        end = f32(timing['startDelaySeconds'])
        for value in (recipe['emitterDelaySeconds'], active, timing['afterImageSeconds'], tail):
            end = f32(end + f32(value))
        ends.append(end)
    return math.ceil(f32(max(ends) * 1000))


def make_leaf(role: str, source):
    document = independent_document(source, PREFIX + role, LABELS[role])
    assert not document.get('modelCues')
    for element in document['elements']:
        assert not element['actionCueAttachment']['enabled']
        assert not element['transformInheritance']['enabled']
        assert not element.get('sourceTransformTrack', {}).get('enabled', False)
        transform = element['detail']['transform']
        assert transform['position'] == [0, 0, 0] and transform['scale'] == [1, 1, 1]
        assert transform['rotationDegrees'] == [0, 0, 0]
    if role == 'pillar':
        # Whole pillar at its local origin: one multiplier, no particle-rate or
        # lifetime changes. Full composition folds this into each pillar TRS.
        document['particleSystem']['uniformScaleMultiplier'] = PILLAR_SCALE
    if role == 'warning':
        assert len(document['elements']) == 1
        element = document['elements'][0]
        assert element['kind'] == 'light' and element['sourceRecipe']['rendererShape'] == 'light'
        assert element['detail']['light']['enabled']
        element['detail']['timing']['lifeTimeSeconds'] = WARNING_MS / 1000
        element['detail']['particle']['lifeTimeSeconds'] = [WARNING_MS / 1000] * 2
        recipe = element['sourceRecipe']
        recipe['authoredModuleOverrides'] = True
        recipe['emitterDurationSeconds'] = WARNING_MS / 1000
        lifetime_fields = 0
        for module in recipe['modules']:
            if module['className'] == 'particlemodulerequired':
                for literal in module['literals']:
                    if literal['propertyPath'] in ('emitterduration', 'emitterdurationlow'):
                        literal['value'] = WARNING_MS / 1000
            if module['className'] != 'particlemodulelifetime':
                continue
            for distribution in module['distributions']:
                if distribution['propertyPath'] != 'lifetime':
                    continue
                assert not distribution['lookupTable'] and not distribution['keys']
                distribution['defaultMinimum'] = [WARNING_MS / 1000, 0, 0, 0]
                distribution['defaultMaximum'] = [WARNING_MS / 1000, 0, 0, 0]
                lifetime_fields += 1
        assert lifetime_fields == 1
    return document


def warning_parameters(source, output: Path):
    """Use the existing CDO/distribution projector for the exact enabled cue."""
    from build_kouku_action_effect_groups import restored_index, project_parameters
    from build_kouku_albion_cross_groups import decode_particle
    action_path = Path('C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829') / (
        'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_05.action-effects.json')
    action = next(a for a in read(action_path)['actions'] if a['actionId'] == 4219820)
    stage = next(s for s in action['stages'] if s['stageIndex'] == 1)
    notify = next(n for n in stage['notifies'] if n['notifyId'] == 'action-4219820/stage-001/notify-008')
    cue = decode_particle(notify)
    assert cue['enabled'] and cue['attachment']['mode'] == 'SNAPSHOT_ROOT'
    assert cue['localTransform']['position'] == [0, 0, 0]
    assert any(r['objectPath'].lower() == SOURCES['warning'] for r in notify['assetReferences'])
    restored = ROOT / 'out/KoukuAllEffects20260912'
    inputs = {path.relative_to(ROOT).as_posix(): sha(path) for path in (
        restored / 'source_module_inputs.json', restored / 'source_class_defaults.json')}
    index = restored_index(restored)
    document = copy.deepcopy(source)
    for element in document['elements']:
        project_parameters(index, element['sourceRecipe'], cue)
    evidence = dict(notifyId=notify['notifyId'], sourceTimeSeconds=notify['localTimeSeconds'],
        sourcePayloadSha256=notify['serializedPayload']['sha256'],
        sourceInputs={action_path.as_posix(): sha(action_path)}, cue=cue,
        lightRangeOwner='Original PointLightComponent.radius; Size parameter is not radius.',
        lifetimeAdapter='Original 0.5 s Lifetime is replaced by USER_AUTHORED 1.2 s warning.')
    write(output / 'source-warning-notify.json', evidence)
    return document, inputs, evidence


def read_measurement(path: Path):
    measured = read(path)
    assert measured['kind'] == 'NATIVE_PLAYBACK_AND_INSTALLED_GEOMETRY'
    assert measured['pillarScale'] == PILLAR_SCALE
    assert measured['failures'] == 0 and measured['positiveAlphaOnly']
    for relative, expected in measured['inputHashes'].items():
        source = (ROOT / relative).resolve()
        assert source.is_relative_to(ROOT), relative
        assert sha(source).lower() == expected.lower(), 'Refresh native measurement: ' + relative
    return measured


def build(output: Path, measurement_path: Path):
    output = output.resolve()
    # --output may organize candidates under out, but cannot redirect writes to
    # Data, Resources, source files, or the running editor's saved Composition.
    out_root = (ROOT / 'out').resolve()
    if not output.is_relative_to(out_root) or output == out_root:
        raise ValueError('Candidate output must be a dedicated directory under repository out/.')
    measured = read_measurement(measurement_path)
    input_hashes = dict(measured['inputHashes'])
    input_hashes[str(Path(__file__).resolve().relative_to(ROOT)).replace('\\', '/')] = sha(Path(__file__))
    for helper in ('build_kouku_dance_grid_groups.py', 'build_kouku_showtime_warning_groups.py',
                   'build_kouku_action_effect_groups.py', 'build_kouku_albion_cross_groups.py',
                   'build_kouku_gate1_full_restore.py'):
        path = Path(__file__).with_name(helper)
        input_hashes[path.relative_to(ROOT).as_posix()] = sha(path)
    leaves = {}
    for role, system in SOURCES.items():
        path = ROOT / 'Data/Effects/Authored' / ('effect.kouku.source.' + system + '.effect.json')
        source = read(path)
        input_hashes[path.relative_to(ROOT).as_posix()] = sha(path)
        assert source['particleSystem']['uniformScaleMultiplier'] == 1
        if role == 'warning':
            source, parameter_inputs, warning_evidence = warning_parameters(source, output)
            input_hashes.update(parameter_inputs)
        leaves[role] = make_leaf(role, source)
    assert [len(leaves[role]['elements']) for role in SOURCES] == [1, 12, 13]

    # The main rising flame has measured 4.5 m horizontal diameter at x1.5.
    # Add 0.5 m core clearance and round upward to a half-metre authoring grid.
    spacing = math.ceil((measured['coreHorizontalDiameterM'] + .5 - 1e-5) * 2) / 2
    forward_step = round(spacing * math.sqrt(3) / 2, 3)
    first_distance = spacing
    full = copy.deepcopy(leaves['warning'])
    full.update(effectAssetId=PREFIX + 'full', displayName=LABELS['full'], elements=[], modelCues=[])
    full['particleSystem']['uniformScaleMultiplier'] = 1
    points, boxes = [], []
    for row in range(1, 5):
        for column in range(1, row + 1):
            point_id = f'flame-wave.r{row}.c{column}'
            position = [round((column - (row + 1) / 2) * spacing, 3), 0,
                        round(first_distance + (row - 1) * forward_step, 3)]
            warning_ms = (row - 1) * ROW_MS
            point = dict(pointId=point_id, groupId=point_id, row=row, column=column,
                         positionMeters=position, warningMs=warning_ms,
                         impactMs=warning_ms + WARNING_MS)
            points.append(point)
            for role in SOURCES:
                start_ms = point['warningMs'] if role == 'warning' else point['impactMs']
                call = dict(positionMeters=position, clientYawDegrees=0,
                            startSeconds=start_ms / 1000,
                            sourceStartSeconds=(start_ms + FIRST_WARNING_MS) / 1000,
                            sourceEventId='user-authored/' + point_id + '/' + role)
                previous_count = len(full['elements'])
                # Existing V1 composition helper owns ID remapping, source
                # history identities and attachment neutralization.
                append_leaf(full, leaves[role], call, len(boxes))
                for element in full['elements'][previous_count:]:
                    element['groupId'] = point_id
                    element['displayName'] = f'R{row} C{column} {role} | ' + element['displayName']
                    if role == 'pillar':
                        element['detail']['transform']['scale'] = [PILLAR_SCALE] * 3
                boxes.append(dict(editableCandidateId=point_id + '.' + role,
                    effectAssetId=PREFIX + role, selectionGroupId=point_id,
                    startMs=FIRST_WARNING_MS + start_ms, durationMs=duration_ms(leaves[role]),
                    positionOffset=position, rotationDegrees=[0, 0, 0], scale=[1, 1, 1],
                    anchorKind='BOSS', followBoss=False,
                    installationStatus='EDITING_ONLY_NO_SHARED_SNAPSHOT_CLOCK'))

    assert len(full['elements']) == 260 and len(points) == 10 and len(boxes) == 30
    assert len({e['groupId'] for e in full['elements']}) == 10
    assert min(e['detail']['timing']['startDelaySeconds'] for e in full['elements']) == 0
    assert [sum(p['row'] == row for p in points) for row in range(1, 5)] == [1, 2, 3, 4]
    # A second tree view has a distinct document asset ID. Keep element IDs and
    # their portable random identities identical across independent documents.
    # No second generation formula or per-actor timing/geometry copy exists.
    alias = copy.deepcopy(full)
    alias['effectAssetId'] = PREFIX + 'full.koukusaydon'
    assert alias['elements'] == full['elements'] and alias['particleSystem'] == full['particleSystem']
    documents = []
    for role, document in {**leaves, 'full': full, 'full.koukusaydon': alias}.items():
        checked = validate(document)
        destination = 'Data/Effects/Authored/' + document['effectAssetId'] + '.effect.json'
        candidate = output / 'candidate' / Path(destination).name
        write(candidate, document)
        row = dict(effectAssetId=document['effectAssetId'],
            candidatePath=candidate.as_posix(), path=destination,
            displayName=document['displayName'], durationMs=duration_ms(document),
            categoryPath=['KoukuSaydon', '공통', '화염 파동'], defaultAnchorKind='BOSS',
            elementCount=len(document['elements']), validation=checked)
        if role == 'full':
            row['parentId'] = 'kouku.category.5ce52bf6c75f791535c5'
        elif role == 'full.koukusaydon':
            row['parentId'] = 'kouku.category.551c3ecfdbde844124d4'
        documents.append(row)
    manifest = dict(schema='lostark.kouku-flame-wave-candidate', version=1,
        stageOnly=True, installed=False, clientUIStarted=False, visualValidation='USER_PENDING',
        documents=documents, inputHashes=input_hashes,
        warningNotifyEvidence=warning_evidence,
        sourceAlignment=dict(sourceActorProfiles=['MN_RPCT_05', 'MN_RPCT_07'],
            sourceActionIds=[4219820, 4219948], sourceStageIndex=1,
            runtimeClip='rpct00_att_battle_10_02', sourceClipMs=4266.666889190674,
            sourceEnabledWandDecalSeconds=3.1035890579223633, roundedFirstImpactMs=3104,
            noDisabledFireWaveEnabled=True),
        authoredTiming=dict(basis='USER_AUTHORED', fullBoxStartMs=FIRST_WARNING_MS,
            rowSpacingMs=ROW_MS, warningDurationMs=WARNING_MS,
            warningStartsMs=[FIRST_WARNING_MS + i * ROW_MS for i in range(4)],
            impactStartsMs=[FIRST_WARNING_MS + WARNING_MS + i * ROW_MS for i in range(4)]),
        authoredPlacement=dict(basis='USER_AUTHORED_FROM_MEASURED_CORE_FOOTPRINT',
            forwardAxis='+Z', horizontalSpacingM=spacing, forwardStepM=forward_step,
            firstDistanceM=first_distance, coreGapM=spacing - measured['coreHorizontalDiameterM'],
            haloOverlapM=measured['haloHorizontalDiameterM'] - spacing,
            measurementPath=measurement_path.resolve().as_posix(), points=points),
        primaryConsumer=dict(effectAssetId=PREFIX + 'full', anchorKind='BOSS', followBoss=False,
            startMs=FIRST_WARNING_MS, positionOffset=[0, 0, 0], rotationDegrees=[0, 0, 0],
            scale=[1, 1, 1], sharedSnapshot='ONE_EFFECT_OCCURRENCE_ROOT_AT_WARNING_ORIGIN'),
        editingOnlyBoxCandidates=boxes,
        limitations=['Par_M_Light_001 is a typed point light; no substitute warning texture is added.',
            'Thirty separate BOSS boxes capture different start clocks; not ready for live installation.',
            'Bounds are native CPU geometry/particle footprint measurements, not shader pixels or user visual approval.'])
    write(output / 'manifest.json', manifest)
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=DEFAULT_OUT)
    parser.add_argument('--measurement', type=Path, default=DEFAULT_OUT / 'bounds.json')
    args = parser.parse_args()
    manifest = build(args.output, args.measurement)
    print(json.dumps(dict(stageOnly=True, output=str(args.output), documents=len(manifest['documents']),
                          elements=sum(d['elementCount'] for d in manifest['documents'])), ensure_ascii=False))


if __name__ == '__main__':
    main()
