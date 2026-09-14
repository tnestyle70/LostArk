"""Compose the user's enlarged Saydon flame and the original upright hoop.

Installed source leaves own Cascade simulation, geometry and native materials.
Only this pattern's placement and shared flame scale are authored here. Catalog
and Composition registration use install_kouku_effect_library separately.
"""
import argparse
import copy
import hashlib
import json
import math
from pathlib import Path

import build_kouku_gate1_full_restore as source
import build_kouku_showtime_warning_groups as groups

ROOT = source.ROOT
AUTHORED = ROOT / 'Data/Effects/Authored'
PREFIX = 'effect.kouku.gate3.backstep.'
FLAME_SOURCE = 'effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_01_loc_int'
RING_SOURCE = 'effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firering_01_loc_int'
RING_END_SOURCE = 'effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firering_02_loc_int'
FLAME_SCALE = 2.0
RING_SCALE = 0.7
RING_HEIGHT = 1.1
RING_SOURCE_CENTER_CM = 225.0


def remap(value, identities):
    if isinstance(value, dict):
        return {key: remap(item, identities) for key, item in value.items()}
    if isinstance(value, list):
        return [remap(item, identities) for item in value]
    return identities.get(value, value) if isinstance(value, str) else value


def project_mesh_rotation(element):
    if element['sourceRecipe']['rendererShape'] != 'mesh':
        return
    modules = [m for m in element['sourceRecipe']['modules']
               if m['className'] == 'particlemoduletypedatamesh']
    assert len(modules) == 1, element['id']
    values = {literal['propertyPath']: literal['value'] for literal in modules[0]['literals']}
    rotation = [values.get(axis, 0.0) for axis in ('roll', 'pitch', 'yaw')]
    assert all(isinstance(v, (int, float)) and math.isfinite(v) for v in rotation)
    element['detail']['mesh']['sourceTypeDataRotationDegrees'] = rotation


def make_document(asset, name, calls):
    document = copy.deepcopy(calls[0]['leaf'])
    document.update(effectAssetId=asset, displayName=name, elements=[], modelCues=[])
    document['particleSystem'].update(uniformScaleMultiplier=1, yawOffsetDegrees=0,
                                     directionYawDegrees=0, initialSpeedMultiplier=1)
    document.pop('sourceModelPreview', None)
    document.pop('sourceAnchorAnimations', None)
    if document['version'] >= 15:
        document['runtimeExtensions'] = dict(formatVersion=1, bakedEdgeHistories=[])
    for call in calls:
        rows = call['leaf']['elements']
        selected = [e for e in rows if call['selection'] == 'all' or
                    (e['sourceRecipe']['rendererShape'] == 'decal') == (call['selection'] == 'ground')]
        group = 'kouku.backstep.' + hashlib.sha256((asset + '/' + call['role']).encode()).hexdigest()[:20]
        ids = {e['id']: group + '.' + str(i) for i, e in enumerate(selected)}
        for original in selected:
            element = remap(copy.deepcopy(original), ids)
            element['groupId'] = group
            attachment = element['actionCueAttachment']
            attachment.update(enabled=False, follow=False, sourceAnchorSlotId='root',
                              runtimeAnchorSlotId='root', runtimeBoneName='')
            attachment.pop('snapshotRootSourceBasisYawDegrees', None)
            attachment.pop('modelCueId', None)
            attachment['socketLocalTransform'] = dict(position=[0, 0, 0],
                rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
            transform = element['detail']['transform']
            transform.update(position=call['position'], rotationDegrees=[0, call['yaw'], 0],
                             scale=[call['scale']] * 3)
            element['detail']['timing']['startDelaySeconds'] += call.get('start', 0)
            project_mesh_rotation(element)
            document['elements'].append(element)
    assert document['elements']
    assert len({e['id'] for e in document['elements']}) == len(document['elements'])
    assert len(name.encode('utf-8')) <= 64
    return document


def inspect_document(document):
    resources = set()
    def visit(value):
        if isinstance(value, dict):
            for item in value.values():
                visit(item)
        elif isinstance(value, list):
            for item in value:
                visit(item)
        elif isinstance(value, str) and value.startswith('Effect/'):
            assert '..' not in Path(value).parts and ':' not in value
            resources.add(value)
    visit(document)
    missing = [p for p in sorted(resources) if not (ROOT / 'Client/Bin/Resources' / p).is_file()]
    assert not missing, missing
    end = 0
    for element in document['elements']:
        assert element['material']['sourceProfile']['enabled']
        assert not element['actionCueAttachment']['enabled']
        timing, recipe = element['detail']['timing'], element['sourceRecipe']
        duration = timing['lifeTimeSeconds']
        if recipe['emitterDurationSeconds'] > 0 and recipe['emitterLoopCount']:
            duration = recipe['emitterDurationSeconds'] * recipe['emitterLoopCount']
        # Source decal recipes are particle simulations too; their particle tail
        # follows the same Calculate_ElementEndSeconds contract as sprite/mesh.
        tail = max(element['detail']['particle']['lifeTimeSeconds'])
        end = max(end, timing['startDelaySeconds'] + recipe['emitterDelaySeconds'] +
                  duration + timing['afterImageSeconds'] + tail)
    return dict(durationMs=math.ceil(end * 1000), elementCount=len(document['elements']),
                resourceCount=len(resources), missingResources=missing)


def build(evidence, install):
    leaves, inputs = {}, []
    for role, asset in [('flame', FLAME_SOURCE), ('ring', RING_SOURCE), ('end', RING_END_SOURCE)]:
        path = AUTHORED / (asset + '.effect.json')
        leaves[role] = source.read(path)
        inputs.append(dict(path=path.relative_to(ROOT).as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
    assert len(leaves['flame']['elements']) == 14
    assert sum(e['sourceRecipe']['rendererShape'] == 'decal' for e in leaves['flame']['elements']) == 3
    assert len(leaves['ring']['elements']) == 10 and len(leaves['end']['elements']) == 7

    # Original Projectile421982502 supplies .7 scale and 1.1m height. Its +90
    # yaw cancels the common source-X to Client-forward -90 yaw. Move only the
    # independent pivot to the hoop centre, retaining relative smoke positions.
    ring = dict(role='ring', leaf=leaves['ring'], selection='all', scale=RING_SCALE,
                yaw=0, position=[0, RING_HEIGHT, RING_SOURCE_CENTER_CM * .01 * RING_SCALE])
    # Both usages consume this same call; no second ring-specific flame recipe.
    flame = dict(role='flame', leaf=leaves['flame'], selection='flame', scale=FLAME_SCALE,
                 yaw=-90, position=[0, RING_HEIGHT, 0])
    ground = dict(role='ground', leaf=leaves['flame'], selection='ground', scale=FLAME_SCALE,
                  yaw=-90, position=[0, 0, 0])
    end = dict(role='ring.end', leaf=leaves['end'], selection='all', scale=RING_SCALE,
               yaw=0, position=ring['position'])
    specs = [
        ('flame', '백스탭불뿜기_확대화염포', [flame]),
        ('ground', '백스탭불뿜기_바닥불길', [ground]),
        ('full', '백스탭불뿜기_화염포·바닥', [flame, ground]),
        ('ring', '화염링_생성·유지', [ring]),
        ('ring.flame', '화염링_동일화염포', [ring, flame]),
        ('ring.end', '화염링_원본종료잔불', [end]),
    ]
    documents, rows = [], []
    for suffix, name, calls in specs:
        document = make_document(PREFIX + suffix, name, calls)
        checked = inspect_document(document)
        source.write(evidence / 'candidate' / (document['effectAssetId'] + '.effect.json'), document)
        documents.append(document)
        rows.append(dict(effectAssetId=document['effectAssetId'], displayName=name,
            path='Data/Effects/Authored/' + document['effectAssetId'] + '.effect.json',
            categoryPath=['KoukuSaydon', '3관문', '패턴', '세이튼', '백스탭 불뿜기',
                          '화염링' if suffix.startswith('ring') else '화염포'],
            defaultAnchorKind='BOSS', followBoss=False, **checked))
    hoop = next(e for e in documents[3]['elements'] if e['sourceRecipe']['rendererShape'] == 'mesh')
    assert hoop['detail']['mesh']['sourceTypeDataRotationDegrees'] == [0, -90, 0]
    # Check the shared cannon's actual parameters, not merely its source ID.
    def cannon_payload(doc):
        result = []
        for element in doc['elements']:
            if element['sourcePresentation']['sourceObjectPath'].startswith('fx_mn_rpct_05_l.par_l_rpct_05_sk_01_loc_int.'):
                result.append({k: v for k, v in element.items() if k not in ('id', 'groupId')})
        return result
    assert cannon_payload(documents[0]) == cannon_payload(documents[4])
    assert len(cannon_payload(documents[0])) == 11
    if install:
        groups.install_independent_documents(documents)
    source.write(evidence / 'installation.json', dict(installed=install, documents=rows,
        sourceInputs=inputs, sourceProjectileId=421982502, sourceActions=[4219825, 4219951],
        sourceProjectileTransform=dict(positionUE3Cm=[120, 0, 110], rotationUnreal=[0, 16384, 0], scale=.7),
        sharedFlameSource=FLAME_SOURCE, sharedFlameScale=FLAME_SCALE,
        sharedFlameIdentityEqual=True, sourceTypeDataMeshRotationDegrees=[0, -90, 0],
        authoredPlacement='Forward +Z; hoop pivot centred above ground; flame origin equals hoop centre.',
        fidelityBoundary='Original native materials and modules; enlarged shared cannon and pivot placement are user-requested composition.',
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(installed=install, documents=len(rows), elements=sum(r['elementCount'] for r in rows))))


def stage_shared_firebreath(evidence, shared_path):
    """Stage current ring/backstep edits using one reviewed neutral flame payload.

    Source leaves and user files are never installed here. The parent publisher
    consumes the exact input hashes and performs its existing guarded commit.
    """
    from build_kouku_shared_firebreath import copy_shared_firebreath_elements

    evidence, shared_path = evidence.resolve(), shared_path.resolve()
    assert evidence.is_relative_to((ROOT / 'out').resolve()), 'Candidates must stay under out'
    assert shared_path.is_relative_to(ROOT.resolve()), 'Shared input must have a repository-relative identity'
    shared = source.read(shared_path)
    identity_system = dict(uniformScaleMultiplier=1, yawOffsetDegrees=0,
                           directionYawDegrees=0, initialSpeedMultiplier=1)
    assert shared['particleSystem'] == identity_system
    assert len(shared['elements']) == 25
    assert all(element['detail']['transform']['position'] == [0, 0, 0] and
        element['detail']['transform']['rotationDegrees'] == [0, 0, 0] and
        element['detail']['transform']['scale'] == [1.7] * 3 and
        not element['actionCueAttachment']['enabled'] and
        not element.get('sourceTransformTrack') and not element.get('runtimeCarrier')
        for element in shared['elements']), 'Review a changed shared flame frame before integration'
    inputs = {shared_path.relative_to(ROOT).as_posix(): hashlib.sha256(shared_path.read_bytes()).hexdigest()}
    documents, writes = [], []
    ring_source = 'fx_mn_rpct_05_g.par_g_rpct_05_firering_01_loc_int.'
    flame_source = 'fx_mn_rpct_05_l.par_l_rpct_05_sk_01_loc_int.'
    for suffix in ('ring', 'ring.flame', 'flame', 'full'):
        path = AUTHORED / (PREFIX + suffix + '.effect.json')
        before = path.read_bytes()
        document = json.loads(before.decode('utf-8-sig'))
        candidate = copy.deepcopy(document)
        assert document['effectAssetId'] == PREFIX + suffix
        assert document['particleSystem'] == identity_system
        assert not document.get('sourceContract') and not document.get('sourceModelPreview')
        assert not document['modelCues'], 'Preserve user-authored model ownership'
        ring_ids, old_flames = [], []
        for element in candidate['elements']:
            system = element['sourcePresentation']['sourceObjectPath']
            if system.startswith(ring_source):
                transform = element['detail']['transform']
                assert not element['actionCueAttachment']['enabled']
                assert not element.get('sourceTransformTrack') and not element.get('runtimeCarrier')
                assert not element.get('transformInheritance', {}).get('enabled')
                assert transform['position'] == [0, 1.1, 1.575] and transform['scale'] == [.7] * 3
                # Scale around the ground origin, including the source hoop's
                # 225 cm eccentricity. Its centre rises 1.1 -> 2.2 m while the
                # bottom keeps its near-zero authored ground clearance.
                transform['position'] = [value * 2 for value in transform['position']]
                transform['scale'] = [value * 2 for value in transform['scale']]
                ring_ids.append(element['id'])
            elif system.startswith(flame_source) and element['sourceRecipe']['rendererShape'] != 'decal':
                transform = element['detail']['transform']
                assert not element['actionCueAttachment']['enabled']
                assert not element.get('sourceTransformTrack') and not element.get('runtimeCarrier')
                assert not element.get('transformInheritance', {}).get('enabled')
                assert transform['position'] == [0, 1.1, 0]
                assert transform['rotationDegrees'] == [0, -90, 0] and transform['scale'] == [2] * 3
                old_flames.append(element)
        assert len(ring_ids) == (10 if suffix.startswith('ring') else 0)
        assert len(old_flames) == (0 if suffix == 'ring' else 11)
        replacement = []
        if old_flames:
            assert len({element['groupId'] for element in old_flames}) == 1
            origin = [0, 2.2 if suffix == 'ring.flame' else 1.1, 0]
            replacement = copy_shared_firebreath_elements(shared, old_flames[0]['groupId'], origin)
            old_ids = {element['id'] for element in old_flames}
            first_id = old_flames[0]['id']
            candidate['elements'] = [item for element in candidate['elements']
                for item in (replacement if element['id'] == first_id else
                             [] if element['id'] in old_ids else [element])]
            # The helper owns the common recipe/frame; no legacy cannon yaw or
            # scale is reapplied after its stable reference remap.
            assert replacement == copy_shared_firebreath_elements(shared, old_flames[0]['groupId'], origin)
        else:
            origin = None
        kept = {element['id'] for element in document['elements']} - set(ring_ids) - {
            element['id'] for element in old_flames}
        assert [element for element in document['elements'] if element['id'] in kept] == [
            element for element in candidate['elements'] if element['id'] in kept]
        assert len(kept) == (3 if suffix == 'full' else 0), 'Unexpected authored members must be reviewed'
        assert len({element['id'] for element in candidate['elements']}) == len(candidate['elements'])
        checked = inspect_document(candidate)
        relative = path.relative_to(ROOT).as_posix()
        inputs[relative] = hashlib.sha256(before).hexdigest()
        baseline = evidence / 'baseline' / relative
        baseline.parent.mkdir(parents=True, exist_ok=True)
        baseline.write_bytes(before)
        target = evidence / 'candidate' / relative
        source.write(target, candidate)
        documents.append(dict(effectAssetId=candidate['effectAssetId'], path=relative,
            candidatePath=target.relative_to(ROOT).as_posix(), displayName=candidate['displayName'],
            defaultAnchorKind='BOSS', **checked))
        writes.append(dict(path=relative, baselineSha256=inputs[relative],
            candidateSha256=hashlib.sha256(target.read_bytes()).hexdigest(),
            ringElementIds=ring_ids, removedFlameIds=[element['id'] for element in old_flames],
            sharedFlameIds=[element['id'] for element in replacement], sharedFlameOriginM=origin,
            preservedOtherElementIds=sorted(kept)))
    # The user also selected the ball-riding library flame for replacement.
    # Keep its stable asset/tree identity, but explicitly record that its old
    # nine-element appearance is being replaced by the shared authored flame.
    ball_asset = 'effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_firebreath_breath_01_loc_int'
    ball_path = AUTHORED / (ball_asset + '.effect.json')
    before = ball_path.read_bytes()
    ball = json.loads(before.decode('utf-8-sig'))
    assert ball['effectAssetId'] == ball_asset and len(ball['elements']) == 9
    assert ball['particleSystem'] == identity_system and not ball['modelCues']
    assert not ball.get('sourceContract') and not ball.get('sourceModelPreview')
    assert all(not element['actionCueAttachment']['enabled'] and
        element['sourcePresentation']['sourceObjectPath'].startswith(ball_asset.removeprefix('effect.kouku.source.') + '.')
        for element in ball['elements'])
    candidate = copy.deepcopy(ball)
    candidate['displayName'] = '공굴리기_공통 불뿜기'
    candidate['elements'] = copy_shared_firebreath_elements(shared, ball_asset, [0, 0, 0])
    checked = inspect_document(candidate)
    relative = ball_path.relative_to(ROOT).as_posix()
    inputs[relative] = hashlib.sha256(before).hexdigest()
    baseline = evidence / 'baseline' / relative
    baseline.parent.mkdir(parents=True, exist_ok=True)
    baseline.write_bytes(before)
    target = evidence / 'candidate' / relative
    source.write(target, candidate)
    documents.append(dict(effectAssetId=ball_asset, path=relative,
        candidatePath=target.relative_to(ROOT).as_posix(), displayName=candidate['displayName'],
        defaultAnchorKind='BOSS', **checked))
    writes.append(dict(path=relative, baselineSha256=inputs[relative],
        candidateSha256=hashlib.sha256(target.read_bytes()).hexdigest(), ringElementIds=[],
        removedFlameIds=[element['id'] for element in ball['elements']],
        sharedFlameIds=[element['id'] for element in candidate['elements']], sharedFlameOriginM=[0, 0, 0],
        replacementPolicy='USER_REQUESTED_SHARED_APPEARANCE; original source archive and baseline preserved'))
    end_path = AUTHORED / (PREFIX + 'ring.end.effect.json')
    inputs[end_path.relative_to(ROOT).as_posix()] = hashlib.sha256(end_path.read_bytes()).hexdigest()
    source.write(evidence / 'installation.json', dict(installed=False, stageOnly=True,
        documents=documents, writes=writes, inputHashes=inputs, sharedEffectAssetId=shared['effectAssetId'],
        ringScaleMultiplier=2, scalingPivotM=[0, 0, 0], ringCenterBeforeM=[0, 1.1, 0],
        ringCenterAfterM=[0, 2.2, 0], sourceHoopCenterOffsetCm=225,
        preservedRingEnd=True, manualVisualValidation='USER_PENDING'))
    print('Staged shared firebreath in four groups and ground-aligned 2x hoop in two; no Data writes')


def stage_flame_origin_repair(evidence, samples_path):
    """Keep current authoring and restore the distance-driven fire layer only.

    Input is a numeric sample of the installed CModel's actual head socket;
    translation is rebased at the first pose, without another actor/authoring
    scale or an invented spawn rate. Existing runtime SourceTransformTrack is
    applied after the element's local transform. A translation-only track
    therefore preserves that transform's position, direction and scale.
    """
    evidence = evidence.resolve()
    assert evidence.is_relative_to((ROOT / 'out').resolve()), 'Candidates must stay under out'
    sample = source.read(samples_path)
    assert sample['model'] == 'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'
    assert sample['preScale'] == .017 and sample['clip'] == 'rpct00_att_battle_14_02'
    assert sample['socket'] == 'FX_Prj_02' and sample['bone'] == 'bip001-head'
    assert sample['sourceStartSeconds'] == .1 and sample['sourceDurationSeconds'] == 1.7
    frames = sample['samples']
    assert len(frames) == 103
    keys, previous = [], -1
    for index, frame in enumerate(frames):
        time, matrix = frame['seconds'], frame['matrix']
        assert math.isfinite(time) and time > previous and abs(time - index / 60) < 1e-6
        assert len(matrix) == 16 and all(math.isfinite(v) for v in matrix)
        assert all(abs(matrix[i]) < 1e-6 for i in (3, 7, 11)) and matrix[15] == 1
        # CModel has already applied .017 to its 100x source rig basis.
        assert all(abs(math.sqrt(sum(matrix[r * 4 + c] ** 2 for c in range(3))) - 1.7) < 1e-4
                   for r in range(3))
        # Inverse of the existing UE3 cm -> Client metre translation adapter.
        position = [matrix[12] * 100, -matrix[14] * 100, matrix[13] * 100]
        keys.append(dict(timeSeconds=time, value=position, arriveTangent=[0, 0, 0],
                         leaveTangent=[0, 0, 0], interpolation='linear'))
        previous = time
    assert previous >= 1.7 - 1e-6
    first = keys[0]['value']
    travel = sum(math.dist(a['value'], b['value']) * .01 for a, b in zip(keys, keys[1:]))
    assert 1.5 < travel < 1.55, 'The measured source head motion changed; inspect before regenerating'
    object_path = 'fx_mn_rpct_05_l.par_l_rpct_05_sk_01_loc_int.particlespriteemitter_8'
    source_occurrence = 'action-4219940/stage-001/notify-002/FX_Prj_02'
    writes = []
    for suffix in ('flame', 'full', 'ring.flame'):
        target = AUTHORED / (PREFIX + suffix + '.effect.json')
        before = target.read_bytes()
        document = json.loads(before.decode('utf-8-sig'))
        elements = [e for e in document['elements']
                    if e['sourcePresentation']['sourceObjectPath'] == object_path]
        assert len(elements) == 1
        element = elements[0]
        assert not element['actionCueAttachment']['enabled'], 'Preserve a user-authored attachment'
        assert 'sourceTransformTrack' not in element, 'Preserve an existing authored motion track'
        assert element['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-2580-native.v1'
        assert element['detail']['particle']['spawnRatePerSecond'] == 0
        assert element['detail']['particle']['burstCount'] == 0 and not element['sourceRecipe']['bursts']
        modules = [m for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemodulespawnperunit']
        assert len(modules) == 1
        assert next(x['value'] for x in modules[0]['literals'] if x['propertyPath'] == 'unitscalar') == 30
        assert next(x['lookupTable'] for x in modules[0]['distributions']
                    if x['propertyPath'] == 'spawnperunit') == [1, 1, 1, 1]
        element['sourceTransformTrack'] = dict(sourceOccurrenceId=source_occurrence,
            sourceTimeOriginSeconds=-element['detail']['timing']['startDelaySeconds'],
            previewOriginUE3Cm=first, nodes=[dict(sourceObjectPath=source_occurrence + '/baked-head-translation',
                frame='WORLD', initialPositionUE3Cm=first, initialEulerDegrees=[0, 0, 0], scaleUE3=[1, 1, 1],
                positionKeys=keys, eulerKeys=[])])
        candidate = evidence / 'candidate' / target.relative_to(ROOT)
        source.write(candidate, document)
        baseline = evidence / 'baseline' / target.relative_to(ROOT)
        baseline.parent.mkdir(parents=True, exist_ok=True)
        baseline.write_bytes(before)
        unchanged = copy.deepcopy(document)
        del next(e for e in unchanged['elements'] if e['id'] == element['id'])['sourceTransformTrack']
        assert unchanged == json.loads(before.decode('utf-8-sig'))
        checked = inspect_document(document)
        writes.append(dict(target=target.relative_to(ROOT).as_posix(),
            candidate=candidate.relative_to(ROOT).as_posix(), baseline=baseline.relative_to(ROOT).as_posix(),
            baselineSha256=hashlib.sha256(before).hexdigest(),
            candidateSha256=hashlib.sha256(candidate.read_bytes()).hexdigest(),
            effectAssetId=document['effectAssetId'], modifiedElementId=element['id'], **checked))
    source.write(evidence / 'installation.json', dict(installed=False, writes=writes,
        sourceSamples=dict(path=str(samples_path.resolve()), sha256=hashlib.sha256(samples_path.read_bytes()).hexdigest()),
        sourceOccurrenceId=source_occurrence, sourceModel=sample['model'], sourceClip=sample['clip'],
        sourceSocket=sample['socket'], sourceStartSeconds=.1, sourceDurationSeconds=1.7,
        sourceOwnerScale=1.7, measuredTravelMetres=travel, sampledFrames=len(keys),
        fidelityBoundary='Current selected Sk_01 layer only: actual head translation rebased at its initial pose. '
            'Other emitters, source recipes, materials and user transforms remain unchanged. '
            'This is not a complete original backstep action or animated head orientation reconstruction.',
        validationStatus='CANDIDATE_ONLY', manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(installed=False, documents=len(writes), sampledFrames=len(keys), sourceTravelMetres=travel)))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuSaitenGroups20260912/backstep')
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--flame-motion-samples', type=Path,
                        help='Stage three current flame repairs from actual CModel head samples; never installs')
    parser.add_argument('--shared-firebreath', type=Path,
                        help='Stage current ring/backstep groups with the reviewed neutral shared flame; never installs')
    args = parser.parse_args()
    if args.shared_firebreath:
        assert not args.install and not args.flame_motion_samples, 'Shared flame integration is a separate stage-only operation'
        stage_shared_firebreath(args.evidence_root, args.shared_firebreath)
    elif args.flame_motion_samples:
        assert not args.install, 'The live authoring repair is candidate-only'
        stage_flame_origin_repair(args.evidence_root, args.flame_motion_samples)
    else:
        build(args.evidence_root, args.install)
