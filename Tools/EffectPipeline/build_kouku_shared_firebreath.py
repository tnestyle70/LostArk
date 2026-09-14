"""Stage the selected staff Fire_01/02 as one reusable neutral firebreath.

The current authored staff document is the appearance source. This tool never
installs into Data: its receipt records the exact baseline for a later guarded
integration. Pattern timing, World instances, catalog and tree belong to callers.
"""
import argparse
import copy
import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE_ASSET = 'effect.kouku.gate3.staff.flame.full.restore'
SHARED_ASSET = 'effect.kouku.gate3.firebreath.shared'
SHARED_NAME = '3관문_세이튼_공통 불뿜기'
SOURCE_NAME = '불뿜기'
SYSTEMS = {
    'fx_mn_rpct_05_g.par_g_rpct_05_fire_01_loc_int': 14,
    'fx_mn_rpct_05_g.par_g_rpct_05_fire_02_loc_int': 11,
}
SOURCE_BASIS_SCALE = 1.7
# Cooked lookupTable starts with a two-value range header. The actual Fire
# velocity payload is UE -Y, which Playback already converts to Client +Z.
NEUTRAL_YAW_DEGREES = 0.0


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def source_system(element):
    return element['sourceNode'].split('|')[-1].split('.particles')[0]


def remap(value, identities):
    if isinstance(value, dict):
        return {key: remap(item, identities) for key, item in value.items()}
    if isinstance(value, list):
        return [remap(item, identities) for item in value]
    return identities.get(value, value) if isinstance(value, str) else value


def selected_firebreath(document):
    """Return only the two exact source fire systems; reject changed ownership."""
    assert document['effectAssetId'] == SOURCE_ASSET
    elements = [e for e in document['elements'] if source_system(e) in SYSTEMS]
    assert {system: sum(source_system(e) == system for e in elements)
            for system in SYSTEMS} == SYSTEMS
    assert len({e['id'] for e in elements}) == 25
    starts = {e['detail']['timing']['startDelaySeconds'] for e in elements}
    assert len(starts) == 1 and abs(next(iter(starts)) - 1.56) < .00001
    for element in elements:
        attachment = element['actionCueAttachment']
        assert attachment['enabled'] and attachment['follow']
        assert attachment['runtimeBoneName'] == 'bip001-head'
        assert attachment['runtimeAnchorSlotId'] == 'WP_3_1'
        assert attachment['socketLocalTransform'] == dict(
            position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
        assert not element.get('transformInheritance', {}).get('enabled')
        assert not element.get('sourceTransformTrack') and not element.get('runtimeCarrier')
        assert element['sourceRecipe']['rendererShape'] == 'sprite'
        assert element['material']['sourceProfile']['enabled']
    return elements


def make_shared_firebreath(document):
    """Retain source simulation, remove preparation and expose root-local +Z."""
    selected = selected_firebreath(document)
    result = copy.deepcopy(document)
    result.update(effectAssetId=SHARED_ASSET, displayName=SHARED_NAME, elements=[], modelCues=[])
    result.pop('sourceModelPreview', None)
    result.pop('sourceAnchorAnimations', None)
    # The per-element frame can also be copied into a ring document with an
    # identity particle-system transform without double rotation or scaling.
    assert document['particleSystem'] == dict(uniformScaleMultiplier=1,
        yawOffsetDegrees=0, directionYawDegrees=0, initialSpeedMultiplier=1)
    identities = {e['id']: 'kouku.shared.firebreath.' +
        hashlib.sha256(e['id'].encode()).hexdigest()[:24] for e in selected}
    start = selected[0]['detail']['timing']['startDelaySeconds']
    for original in selected:
        element = remap(copy.deepcopy(original), identities)
        element['groupId'] = SHARED_ASSET
        element['actionCueAttachment'] = dict(enabled=False, follow=False,
            sourceAnchorSlotId='root', runtimeAnchorSlotId='root', runtimeBoneName='',
            socketLocalTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1]))
        transform = element['detail']['transform']
        # A new user transform must be reviewed, never silently discarded.
        assert transform == dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0],
            revolutionDegreesPerSecond=[0, 0, 0], scale=[1, 1, 1], velocityPerSecond=[0, 0, 0])
        transform['rotationDegrees'] = [0, NEUTRAL_YAW_DEGREES, 0]
        transform['scale'] = [SOURCE_BASIS_SCALE] * 3
        element['detail']['timing']['startDelaySeconds'] -= start
        result['elements'].append(element)
    return result


def copy_shared_firebreath_elements(shared, destination_group, position=(0, 0, 0)):
    """Copy the same 25-element payload into a neutral composition at its pivot.

    The caller owns the destination document and must keep its particleSystem
    scale/yaw at identity. Position is in metres after the shared yaw/scale.
    Source recipes, local/world policy and each particle's source clock survive.
    """
    assert shared['effectAssetId'] == SHARED_ASSET and len(shared['elements']) == 25
    assert len(position) == 3 and all(math.isfinite(v) for v in position)
    assert destination_group and destination_group != SHARED_ASSET
    identities = {e['id']: 'kouku.shared.firebreath.' + hashlib.sha256(
        (destination_group + '/' + e['id']).encode()).hexdigest()[:24] for e in shared['elements']}
    copied = remap(copy.deepcopy(shared['elements']), identities)
    for element in copied:
        element['groupId'] = destination_group
        transform = element['detail']['transform']
        assert transform['position'] == [0, 0, 0]
        transform['position'] = list(position)
    return copied


def stage(evidence):
    source_path = ROOT / 'Data/Effects/Authored' / (SOURCE_ASSET + '.effect.json')
    before = source_path.read_bytes()
    source = json.loads(before.decode('utf-8-sig'))
    shared = make_shared_firebreath(source)
    renamed = copy.deepcopy(source)
    renamed['displayName'] = SOURCE_NAME
    baseline = evidence / 'baseline' / source_path.name
    baseline.parent.mkdir(parents=True, exist_ok=True)
    baseline.write_bytes(before)
    candidates, documents = [], []
    for document in (renamed, shared):
        path = evidence / 'candidate' / (document['effectAssetId'] + '.effect.json')
        write(path, document)
        target = ROOT / 'Data/Effects/Authored' / path.name
        # New resource creation cannot silently become replacement of an edit.
        if document['effectAssetId'] == SHARED_ASSET:
            assert not target.exists(), 'Shared resource already exists; review current edits first'
        candidates.append(dict(effectAssetId=document['effectAssetId'],
            target=target.relative_to(ROOT).as_posix(), candidate=path.relative_to(ROOT).as_posix(),
            baselineSha256=hashlib.sha256(before).hexdigest() if document is renamed else None,
            candidateSha256=hashlib.sha256(path.read_bytes()).hexdigest(),
            elementCount=len(document['elements'])))
        registration = dict(effectAssetId=document['effectAssetId'],
            candidatePath=path.relative_to(ROOT).as_posix(),
            path=target.relative_to(ROOT).as_posix(), displayName=document['displayName'],
            durationMs=5560 if document is renamed else 5500,
            defaultAnchorKind='BOSS', changeKind='DISPLAY_NAME_ONLY'
                if document is renamed else 'NEW_SHARED_FIREBREATH')
        if document is renamed:
            registration['parentId'] = 'kouku.category.06ae0cf5bf6b9aa797d4'
        else:
            registration['categoryPath'] = ['KoukuSaydon', '공통', '불뿜기']
        documents.append(registration)
    write(evidence / 'installation.json', dict(installed=False, sourceAsset=SOURCE_ASSET,
        writes=candidates, documents=documents,
        inputHashes={source_path.relative_to(ROOT).as_posix(): hashlib.sha256(before).hexdigest()},
        sourceSystems=SYSTEMS, sourcePreparationRemovedSeconds=1.56,
        neutralLocalForward=[0, 0, 1], perElementYawDegrees=NEUTRAL_YAW_DEGREES,
        preservedSourceBasisScale=SOURCE_BASIS_SCALE, particleSystemTransform='IDENTITY',
        sourceReferenceClosure=[e['id'] for e in selected_firebreath(source)],
        sharedElementIds=[e['id'] for e in shared['elements']],
        stageOnly=True, manualVisualValidation='USER_PENDING'))
    print('Staged shared firebreath25 and staff display-only rename; no Data writes')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuFlameCommon20260914')
    stage(parser.parse_args().evidence_root)
