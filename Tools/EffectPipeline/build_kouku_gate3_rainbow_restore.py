"""Restore the original RPCT07 fire-grid warning and burst as standalone V1 Effects.

The full original first LOD is retained for each ParticleSystem. These library
previews use the selected player root. Original Projectile 421991401..421991405
references identify the paired systems; external placement is owned separately.
Native material bindings must be supplied by the existing source shader pipeline.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import json
import math
import sys

import build_kouku_gate1_full_restore as source
from extract_ue3_skeletal_mesh_sockets import parse_socket_contract
from decode_kouku_gate3_rainbow_grid import decode as decode_grid

ROOT = source.ROOT
TARGETS = {
    1: dict(stage=0, notify='source-system/rpct07-fire-bigearthwave/preview-root',
            system='fx_mn_rpct_07_v.par_v_rpct_fire_bigearthwave_01_loc_int', count=10,
            asset='effect.kouku.gate3.rainbow.fire.wave.full.restore',
            name='Kouku Gate3 Rainbow Fire Wave Full Restore', standalone=True),
    2: dict(stage=0, notify='source-system/rpct07-fire-decal-t/preview-root',
            system='fx_mn_rpct_07_v.par_v_rpct_fire_decal_t_01_loc_int', count=7,
            asset='effect.kouku.gate3.rainbow.fire.warning.full.restore',
            name='Kouku Gate3 Rainbow Fire Warning Full Restore', standalone=True),
    3: dict(stage=0, notify='source-system/rpct07-fire-grid-light/preview-root',
            system='fx_cm_02.light.par_mp_light_01', count=1,
            asset='effect.kouku.gate3.rainbow.fire.light.full.restore',
            name='Kouku Gate3 Rainbow Fire Light Full Restore', standalone=True),
}


def acquire(evidence):
    grid = decode_grid(source.SOURCE, evidence, source.read, source.write)
    light = next(p for p in grid['projectiles']['421991401']['particles']
                 if p['sourceParticleSystem'] == TARGETS[3]['system'])
    TARGETS[3]['parameterOverrides'] = [dict(name=p['name'],
        type='scalar' if p['sourceTypeCode'] == 1 else 'vector',
        **({'scalarValue': p['value']} if p['sourceTypeCode'] == 1 else {'vectorValue': p['value']}))
        for p in light['parameterOverrides'] if p['sourceTypeCode'] in (1, 3)]
    chosen = dict(sourceKind='ORIGINAL_PARTICLE_SYSTEM_LIBRARY_PREVIEW',
        sourceProjectiles=[421991401,421991402,421991403,421991404,421991405], actions=[])
    for action_id, target in TARGETS.items():
        if target.get('standalone'):
            # This adapter represents an isolated original ParticleSystem, not
            # a decoded CEF notify or an invented original gameplay action.
            # Positive dictionary ordinals are private projector grouping keys.
            cue = dict(enabled=True, particleDataDecoded=True, parameterOverridesDecoded=True,
                sourceKind='ORIGINAL_PARTICLE_SYSTEM_LIBRARY_PREVIEW',
                parameterOverrides=copy.deepcopy(target.get('parameterOverrides', [])),
                attachment=dict(mode='SNAPSHOT_ROOT', sourceAnchorNames=[], runtimeAnchors=[],
                    runtimeAnchorSlotId='root', runtimeBoneName='', runtimeResolutionStatus='EXACT_ROOT_SNAPSHOT',
                    socketLocalTransform=dict(position=[0,0,0], rotationDegrees=[0,0,0], scale=[1,1,1])),
                localTransform=dict(position=[0,0,0], rotationDegrees=[0,0,0], scale=[1,1,1]))
            notify = dict(notifyId=target['notify'], sourceType='PlayParticleEffect', localTimeSeconds=0,
                durationSeconds=target.get('previewSeconds', 0), serializedPayload=cue, serializedLabels=[],
                assetReferences=[dict(className='ParticleSystem', objectPath=target['system'])])
            chosen['actions'].append(dict(actionId=action_id, sourceKind=cue['sourceKind'],
                stages=[dict(stageIndex=0, animationClips=[], notifies=[notify])]))
            target['sourceTimeSeconds'] = 0
            continue
    source.write(evidence / 'selected_source_actions.json', chosen)
    sockets = source.SOURCE / 'CanonicalSource/Character/UModelExports/MN_RPCT_07/Export/MN_RPCT_07/mesh/mn_rpct_07_sk.props.txt'
    if sockets.is_file():
        source.write(evidence / 'source_socket_contract.json', parse_socket_contract(sockets))
    source.ACTION = evidence / 'selected_source_actions.json'
    source.SELECTED = {i: ([t['stage']], t['name']) for i, t in TARGETS.items()}
    original_decode = source.decode_typed_payload
    def decode(kind, payload, *args):
        return copy.deepcopy(payload) if isinstance(payload, dict) and payload.get('sourceKind') == 'ORIGINAL_PARTICLE_SYSTEM_LIBRARY_PREVIEW' else original_decode(kind, payload, *args)
    source.decode_typed_payload = decode
    index, notifies, occurrences, records = source.acquire(evidence)
    for action_id, target in TARGETS.items():
        assert sum(o['actionId'] == action_id for o in occurrences) == target['count']
    return index, notifies, occurrences, records


def project(evidence, material_patch, install=False):
    index, notifies, occurrences, records = acquire(evidence)
    destination = evidence / 'projected'
    source.project(evidence, index, notifies, occurrences, records, destination, material_patch)
    writes, documents, leaves = [], [], {}
    for action_id, target in TARGETS.items():
        document = source.read(destination / f'effect.kouku.gate1.{action_id}.full.restore.effect.json')
        document['effectAssetId'] = target['asset']
        document['displayName'] = target['name']
        assert len(document['displayName'].encode('utf8')) <= 64
        for element in document['elements']:
            # This is an isolated ParticleSystem library preview. The complete
            # Cascade modules/parameters remain; boss socket ownership does not.
            element['groupId'] = target['asset']
            element['actionCueAttachment']['enabled'] = False
            # The shared boss-action projector supplies an ActorX/FBX skeleton
            # correction. A standalone ParticleSystem / fixed-area projectile
            # has no RPCT skeleton: its coordinates already use (X, Z, -Y).
            element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees', None)
            element['sourcePresentation']['sourceTimeSeconds'] = target['sourceTimeSeconds']
            element['detail']['transform'].update(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
        assert len(document['elements']) == target['count']
        leaves[target['system']] = document
    grid, grid_receipt = project_grid(evidence, leaves)
    output_documents = list(leaves.values()) + [grid]
    for document in output_documents:
        source.write(evidence / 'candidate' / (document['effectAssetId'] + '.effect.json'), document)
        path = ROOT / 'Data/Effects/Authored' / (document['effectAssetId'] + '.effect.json')
        before = path.read_bytes() if path.exists() else None
        payload = (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8')
        assert before is None or json.loads(before) == document, f'Preserve existing authored edits: {path}'
        writes.append((path, before, payload))
        if document is grid:
            documents.append(grid_receipt)
        else:
            action_id, target = next((i, t) for i, t in TARGETS.items() if t['asset'] == document['effectAssetId'])
            documents.append(dict(effectAssetId=target['asset'], sourceActionId=None,
                sourceNotify=target['notify'], sourceParticleSystem=target['system'],
                elementCount=len(document['elements']),
                durationMs=source.read(evidence / 'projection.json')['durationMs'][str(action_id)]))
    assert material_patch is not None or not install, 'Native material closure is required for installation'
    for path, before, _ in writes:
        assert (path.read_bytes() if path.exists() else None) == before, f'Concurrent edit: {path}'
    changed = []
    for path, before, payload in writes:
        if payload != before:
            changed.append(str(path.relative_to(ROOT)))
            if install:
                path.write_bytes(payload)
    source.write(evidence / 'installation.json', dict(installed=install, changedPaths=changed,
        documents=documents, previewAnchor='SELECTED_PLAYER_ROOT',
        screenshotIdentity='SOURCE_DIAGONAL_FIRE_GRID_MATCH_REQUIRES_USER_VISUAL_REVIEW',
        fieldGridPlacement='ORIGINAL_ACTION_4219916_STAGE002_SKILLEFFECT_PROJECTILE_CHAIN',
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(installed=install, changedPaths=changed, documents=documents), ensure_ascii=False))


def project_grid(evidence, leaves):
    grid = source.read(evidence / 'decoded_grid.json')
    document = copy.deepcopy(next(iter(leaves.values())))
    document.update(effectAssetId='effect.kouku.gate3.rainbow.grid.full.restore',
                    displayName='Kouku Gate3 Rainbow Grid Full Restore', elements=[])
    calls, end_seconds = [], 0
    for occurrence_index, occurrence in enumerate(grid['occurrences']):
        projectile = grid['projectiles'][occurrence['projectileId']]
        row = occurrence['sourceSkillEffect']
        position = [row['AreaOffsetX'] * .01, row['AreaOffsetZ'] * .01, -row['AreaOffsetY'] * .01]
        for particle_index, particle in enumerate(projectile['particles']):
            system = particle['sourceParticleSystem']
            leaf = leaves[system]
            if system != TARGETS[3]['system']:
                assert all(p['name'].lower() == 'none' for p in particle['parameterOverrides']), 'Unprojected source parameter override'
            when = occurrence['notifySeconds'] + particle['timerSeconds']
            yaw = row['AreaOffsetAngle'] + particle['sourceRotationDegrees'][1]
            cue_id = occurrence['notifyId'] + '/projectile-' + occurrence['projectileId'] + f'/particle-{particle_index:02d}'
            group_id = f'kouku.g3.grid.4219916.002.{occurrence_index:02d}.{particle_index:02d}'
            assert particle['sourcePositionCm'] == [0, 0, 0] and particle['sourceScale'] == [1, 1, 1]
            # The native FRotator yaw becomes Client Y rotation. Location uses
            # the established UE3 cm -> Client m (X, Z, -Y) basis once.
            # Source Cascade modules retain their own native distributions.
            for original in leaf['elements']:
                element = copy.deepcopy(original)
                element['id'] = group_id + '.' + original['id'].rsplit('.', 1)[1]
                element['groupId'] = group_id
                role = 'Warning' if 'decal_t' in system else 'Wave' if 'bigearthwave' in system else 'Light'
                element['displayName'] = f"Grid{row['AreaOffsetAngle']} {occurrence_index % 5 + 1} {role}{particle_index + 1} | " + original['sourcePresentation']['sourceObjectPath'].rsplit('.', 1)[1]
                element['sourceNode'] = cue_id + '|' + original['sourcePresentation']['sourceObjectPath']
                element['detail']['transform'].update(position=position, rotationDegrees=[0, yaw, 0], scale=[1, 1, 1])
                element['detail']['timing']['startDelaySeconds'] += when
                element['sourcePresentation'].update(sourceActionCueId=cue_id, sourceEventId=cue_id,
                    sourceOccurrenceIndex=occurrence_index, sourceTimeSeconds=when)
                timing = element['detail']['timing']
                tail = 0 if element['kind'] == 'light' else max(element['detail']['particle']['lifeTimeSeconds'])
                end_seconds = max(end_seconds, timing['startDelaySeconds'] + timing['lifeTimeSeconds'] + tail)
                document['elements'].append(element)
            calls.append(dict(sourceNotify=occurrence['notifyId'], sourceProjectileId=occurrence['projectileId'],
                sourceParticleByteOffset=particle['sourceByteOffset'], sourceParticleSystem=system,
                groupId=group_id, elementCount=len(leaf['elements']), startSeconds=when,
                positionMeters=position, clientYawDegrees=yaw, sourceParameterOverrides=particle['parameterOverrides']))
    assert len(calls) == 42 and len(document['elements']) == 342
    assert len({e['id'] for e in document['elements']}) == 342
    receipt = dict(effectAssetId=document['effectAssetId'], sourceActionId=4219916, sourceStageIndex=2,
        sourceNotifyCount=10, sourceParticleCallCount=42, elementCount=342,
        durationMs=math.ceil(end_seconds * 1000), calls=calls,
        sourceJoinStatus='SOURCE_JOIN_COMPLETE',
        sourceOwnerLifetimeSeconds=4, particleStopAtOwnerEnd='NO_UNPROVEN_OWNER_END_KILL_ADDED',
        previewAnchor='SELECTED_PLAYER_ROOT', manualVisualValidation='USER_PENDING')
    source.write(evidence / 'grid_projection.json', receipt)
    return document, {k: v for k, v in receipt.items() if k != 'calls'}


def prepare_geometry(evidence):
    sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
    import cook_wmodel_geometry_contract as geometry
    name = 'fm_d_squarecross_002'
    resource_root = ROOT / 'Client/Bin/Resources'
    gltf = source.SOURCE / 'EffectRuntimeClosureExports-20260829/fx_sm_00/Export/FX_SM_00' / (name + '.gltf')
    legacy = resource_root / 'Effect/KoukuSaydon/Meshes/fx_sm_00' / (name + '.wmodel')
    manifest = source.SOURCE / 'CanonicalSource/Effect/Closure/closure.manifest.json'
    package_row = next(p for p in source.read(manifest)['sourcePackages'] if p['logicalPackage'].lower() == 'fx_sm_00')
    package = manifest.parent / package_row['outputRelativePath']
    observation = evidence / 'squarecross_geometry_observation.json'
    digest = lambda p: hashlib.sha256(p.read_bytes()).digest()
    source.write(observation, dict(sourceObject='fx_sm_00.' + name, sourceGltf=str(gltf),
        legacyWmodel=str(legacy), sourceGltfSha256=digest(gltf).hex(), legacyWmodelSha256=digest(legacy).hex(),
        evidence='OBSERVED_EXISTING_SOURCE_GLTF_AND_LEGACY_WMODEL'))
    provenance = geometry.GeometryProvenanceEvidence('fx_sm_00.' + name, digest(manifest), 'OBSERVED_SOURCE_RECEIPT',
        digest(package), digest(ROOT / 'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe'),
        digest(observation), digest(observation))
    payload, receipt = geometry.cook_wmodel_geometry_contract(gltf, legacy, provenance)
    output = resource_root / 'Effect/KoukuSaydon/FullRestore/Meshes' / (name + '.wmodel')
    output.parent.mkdir(parents=True, exist_ok=True)
    if not output.exists() or output.read_bytes() != payload:
        output.write_bytes(payload)
    source.write(evidence / 'squarecross_geometry_cook.json', receipt)
    print('Geometry prepared', output.relative_to(resource_root).as_posix())


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuRainbowMatched20260911')
    parser.add_argument('--native-material-patch', type=Path)
    parser.add_argument('--acquire-only', action='store_true')
    parser.add_argument('--prepare-geometry', action='store_true')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    if args.prepare_geometry:
        prepare_geometry(args.evidence_root)
    elif args.acquire_only:
        acquire(args.evidence_root)
    else:
        project(args.evidence_root, args.native_material_patch, args.install)
