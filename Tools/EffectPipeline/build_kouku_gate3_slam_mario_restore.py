"""Prepare the source staff flame, Mario circles, and Odd Doll V1 groups.

Only new authored documents and their exact Resources dependencies are installed.
Original source actions, socket records and Cascade values remain read-only.
"""
import argparse
import copy
import hashlib
import json
import math
import mmap
import statistics
import struct
import subprocess
import sys
from pathlib import Path

import build_kouku_gate1_full_restore as source
import build_kouku_gate3_rainbow_native as native
from extract_action_effect_notifies import extract_action_document, scan_length_prefixed_strings

ROOT = source.ROOT
LIBRARIES = {
    101: ('mario.center.pentagram', 'Mario central pentagram', 'fx_mn_rpct_07_v.par_v_rpct_star_cast_loop_loc_int', 20),
    102: ('mario.center.portal', 'Mario central portal loop', 'fx_mn_rpct_07_v.par_v_rpct_atk_09_03_loop_loc_int', 20),
    103: ('mario.boss.pentagram', 'Mario boss pentagram cast', 'fx_mn_rpct_07_v.par_v_rpct_star_cast_01_loc_int', 2),
}
ASSETS = {4219951: ('staff.flame', 'Staff ground then flame'),
          4222305: ('doll.flame', 'Odd Doll dual flame')}


def complete_small_pentagram(document):
    """Reuse the existing large floor star at the source drawing's measured size."""
    sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
    from cook_wmodel_geometry_contract import parse_geometry_wmodel

    floor_prefix = 'kouku.mario.small.pentagram.floor.'
    drawing = [e for e in document['elements'] if not e['id'].startswith(floor_prefix)]
    assert len(drawing) == 8 and all(e['sourceRecipe']['rendererShape'] == 'mesh' for e in drawing)
    floor_leaf_id = 'boss.kouku.disarm.star.decal_1'
    leaf = source.read(ROOT / 'Data/Effects/V2/Authored' / (floor_leaf_id + '.effectv2.json'))
    group = source.read(ROOT / 'Data/Effects/V2/Groups/boss.kouku.disarm.effectv2group.json')
    children = [c for c in group['children'] if c['resource']['id'] == floor_leaf_id]
    assert len(children) == 5 and leaf['effectType'] == 'Decal' and not leaf['params']['loop']

    # UV V=.5 is the drawing ribbon's centre line, excluding its glow thickness.
    points, ends = [], []
    for element in drawing:
        size = next(d for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemodulesize'
                    for d in m['distributions'] if d['propertyPath'] == 'startsize')['lookupTable'][2:5]
        assert len(size) == 3 and max(size) - min(size) < 1e-6
        mesh = next(r['assetId'] for r in element['resources'] if r['slotId'] == 'meshModel')
        geometry = parse_geometry_wmodel((ROOT / 'Client/Bin/Resources' / mesh).read_bytes())
        scale = geometry['geometryPreScale'] * size[0]
        rotation = next(d for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemodulemeshrotation'
                        for d in m['distributions'] if d['propertyPath'] == 'startrotation')['lookupTable'][2:5]
        assert rotation[:2] == [0, 0] and element['detail']['mesh'].get('sourceTypeDataRotationDegrees', [0, 0, 0]) == [0, 0, 0]
        # The lookup table starts with two bounds, followed by UE roll/pitch/yaw
        # in turns. UE yaw becomes Client Y rotation after basis conversion.
        source_yaw = rotation[2] * math.tau
        for part in geometry['submeshes']:
            for vertex in part['vertices']:
                if abs(vertex['values'][7] - .5) >= .001:
                    continue
                x, z = vertex['values'][0] * scale, vertex['values'][2] * scale
                points.append((x * math.cos(source_yaw) + z * math.sin(source_yaw),
                               -x * math.sin(source_yaw) + z * math.cos(source_yaw)))
        life = element['detail']['particle']['lifeTimeSeconds'][1]
        ends += [element['detail']['timing']['startDelaySeconds'] +
                 element['sourceRecipe']['emitterDelaySeconds'] + b['timeSeconds'] + life
                 for b in element['sourceRecipe']['bursts']]
    radius = statistics.median(math.hypot(x, z) for x, z in points)
    target_tip_angle = math.atan2(*max(points, key=lambda p: p[1])[::-1])
    source_radius = statistics.mean(math.hypot(c['localTransform']['translation'][0],
        c['localTransform']['translation'][2]) for c in children) / math.cos(math.radians(72))
    ratio = radius / source_radius
    yaw = 90 - math.degrees(target_tip_angle)
    visible_end = max(ends)
    assert 0 < ratio < 1 and 0 < visible_end < 10
    angle = math.radians(yaw)

    def key(time, value):
        return dict(timeSeconds=time, value=[value] * 3, arriveTangent=[0, 0, 0],
                    leaveTangent=[0, 0, 0], interpolation='linear')

    floor = []
    for child in children:
        element = copy.deepcopy(drawing[0])
        element['id'] = floor_prefix + child['childId']
        element['displayName'] = '\uc791\uc740 \uc624\ub9dd\uc131 | \ubc14\ub2e5 ' + str(len(floor) + 1)
        element['sourceNode'] = 'project-authored:small-pentagram-floor|' + group['groupId'] + '|' + child['childId']
        element['kind'] = 'decal'
        element['resources'] = [dict(slotId='base', assetId=leaf['slots']['base']),
                                dict(slotId='dissolve', assetId=leaf['slots']['dissolve'])]
        element['material'] = dict(templateId='effect.standard', sourceMaterialPath='',
                                  renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False))
        element['sourceRecipe'] = dict(enabled=False, rendererShape='', emitterDelaySeconds=0,
            emitterDurationSeconds=0, emitterLoopCount=1, bursts=[], modules=[])
        element['sourcePresentation'] = dict(enabled=False)
        transform = element['detail']['transform']
        x, _, z = child['localTransform']['translation']
        transform['position'] = [(x * math.cos(angle) + z * math.sin(angle)) * ratio, 0,
                                 (-x * math.sin(angle) + z * math.cos(angle)) * ratio]
        transform['rotationDegrees'] = [0, child['localTransform']['rotation'][1] + yaw, 0]
        transform['scale'] = [1, 1, 1]
        element['detail']['timing'].update(startDelaySeconds=0, lifeTimeSeconds=visible_end,
                                           afterImageSeconds=0, dissolveStartNormalized=1)
        element['detail']['decal'] = dict(size=[v * ratio for v in leaf['params']['decal']['size']],
                                         depth=leaf['params']['decal']['depth'])
        element['detail']['mesh']['useModelMaterial'] = False
        element['detail']['sprite']['billboard'] = False
        # The V1 alpha curve keeps this one decal alive through the complete cast;
        # its fade-in uses the source floor's reveal fraction and its fade-out the
        # drawing's final flash lifetime. The source V2 group remains untouched.
        reveal_end = visible_end * leaf['params']['dissolveInEnd']
        flash_life = min(e['detail']['particle']['lifeTimeSeconds'][1] for e in drawing)
        element['sourceTransformTrack'] = dict(sourceOccurrenceId=element['sourceNode'],
            sourceTimeOriginSeconds=0, previewOriginUE3Cm=[0, 0, 0], nodes=[dict(
                sourceObjectPath='project-authored:small-pentagram-floor', frame='RELATIVE_TO_INITIAL',
                initialPositionUE3Cm=[0, 0, 0], initialEulerDegrees=[0, 0, 0], scaleUE3=[1, 1, 1],
                positionKeys=[key(0, 0)], eulerKeys=[key(0, 0)])],
            alphaScaleKeys=[key(0, 0), key(reveal_end, 1), key(visible_end - flash_life, 1), key(visible_end, 0)])
        floor.append(element)
    result = copy.deepcopy(document)
    result['displayName'] = '\uc791\uc740 \uc624\ub9dd\uc131'
    result['elements'] = drawing + floor
    return result, dict(sourceFloorGroup=group['groupId'], sourceFloorLeaf=floor_leaf_id,
        drawingRadiusM=radius, sourceFloorRadiusM=source_radius, scaleRatio=ratio,
        yawOffsetDegrees=yaw, visibleEndSeconds=visible_end, decalCount=len(floor))


def mario_projectile(evidence):
    sys.path.insert(0, str(ROOT / 'Tools/LpkPipeline'))
    import unpack_lpk as lpk
    archive = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/data1.lpk')
    with archive.open('rb') as stream, mmap.mmap(stream.fileno(), 0, access=mmap.ACCESS_READ) as packed:
        key, base = lpk.REGIONS['KR'][0].encode('latin1'), bytes.fromhex(lpk.REGIONS['KR'][1])
        entry = next(e for e in lpk.read_index(packed, key) if e['path'].lower().endswith('projectile\\422200101.loa'))
        raw = lpk.extract(packed, entry, key, base)
    assert struct.unpack_from('<f', raw, 1014)[0] == 20
    particles = {}
    for token in scan_length_prefixed_strings(raw, 0, len(raw)):
        if not token['value'].startswith("ParticleSystem'"):
            continue
        at = token['sourceOffset'] + 4 + len(token['value']) + 1
        position = list(struct.unpack_from('<3f', raw, at + 76))
        rotation = list(struct.unpack_from('<3i', raw, at + 100))
        scale = list(struct.unpack_from('<3f', raw, at + 136))
        count = struct.unpack_from('<i', raw, at + 148)[0]
        assert count in (0, 1)
        if count:
            cursor = at + 152
            length = struct.unpack_from('<i', raw, cursor)[0]
            assert raw[cursor + 4:cursor + 4 + length] == b'None\0'
        particles[token['value'].split("'")[1].lower()] = dict(sourceByteOffset=token['sourceOffset'],
            sourcePositionCm=position, sourceRotator=rotation, sourceScale=scale,
            position=[position[0] * .01, position[2] * .01, -position[1] * .01],
            rotationDegrees=[rotation[0] * 360 / 65536, rotation[1] * 360 / 65536, -rotation[2] * 360 / 65536], scale=scale)
    assert set(particles) == {LIBRARIES[101][2], LIBRARIES[102][2]}
    source.write(evidence / 'mario_projectile_source.json', dict(projectileId=422200101,
        archive=str(archive), archiveEntry=entry['path'], sha256=hashlib.sha256(raw).hexdigest(),
        lifetimeSeconds=20, particles=particles))
    return particles


def socket_contract(model_path):
    mesh = native.obj(model_path)
    package = native.pkg(mesh['package'])
    sockets = []
    for ordinal, reference in enumerate(native.tagged_value(mesh['properties'], 'sockets')):
        path = native.fullref(mesh['package'], package, reference)
        row = native.obj(path)
        props = row['properties']
        get = lambda key, default: native.tagged_value(props, key) if key in props else default
        position = get('relativelocation', {'x': 0, 'y': 0, 'z': 0})
        rotation = get('relativerotation', {'pitch': 0, 'yaw': 0, 'roll': 0})
        scale = get('relativescale', {'x': 1, 'y': 1, 'z': 1})
        positions = [position[k] for k in ('x', 'y', 'z')]
        rotations = [rotation[k] for k in ('pitch', 'yaw', 'roll')]
        scales = [scale[k] for k in ('x', 'y', 'z')]
        sockets.append(dict(sourceIndex=ordinal, socketName=get('socketname', ''), boneName=get('bonename', ''),
            sourceTransform=dict(positionUeUnits=positions, rotationUnrealUnits=rotations, scale=scales),
            runtimeLocalTransform=dict(position=[v * .01 for v in positions],
                rotationDegrees=[v * 360 / 65536 for v in rotations], scale=scales),
            transformEvidence='ORIGINAL_SKELETALMESH_SOCKET_EXPORT_AND_UE3_DEFAULTS',
            sourceObjectPath=path, sourceSerialSha256=row['serialSha256']))
    return dict(schema='lostark.ue3-skeletal-mesh-sockets', formatVersion=1,
        source=dict(objectPath=model_path, sourcePackage=str(package.path), sha256=mesh['serialSha256'],
            positionUnitScale=.01, rotationUnitScaleDegrees=360 / 65536), sockets=sockets)


def acquire(evidence):
    action_path = source.SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json'
    boss = source.read(action_path)
    flame = copy.deepcopy(next(a for a in boss['actions'] if a['actionId'] == 4219951))
    flame['stages'] = [s for s in flame['stages'] if s['stageIndex'] in (1, 2)]
    doll = extract_action_document(source.SOURCE / 'RemainingCharacterExtraction-20260906/loa/MN_CDMD_00.loa',
                                  'MN_CDMD_00', action_ids={4222305})['actions'][0]
    doll['stages'] = [doll['stages'][0]]
    selected = dict(actions=[flame, doll])
    mario = mario_projectile(evidence)
    for key, (suffix, title, system, seconds) in LIBRARIES.items():
        cue = dict(enabled=True, particleDataDecoded=True, parameterOverridesDecoded=True,
            sourceKind='ORIGINAL_PARTICLE_SYSTEM_LIBRARY_PREVIEW', parameterOverrides=[],
            attachment=dict(mode='SNAPSHOT_ROOT', sourceAnchorNames=[], runtimeAnchors=[],
                runtimeAnchorSlotId='root', runtimeBoneName='', runtimeResolutionStatus='EXACT_ROOT_SNAPSHOT',
                socketLocalTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])),
            localTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1]))
        if system in mario:
            cue['localTransform'] = {field: mario[system][field] for field in ('position', 'rotationDegrees', 'scale')}
        selected['actions'].append(dict(actionId=key, stages=[dict(stageIndex=0, animationClips=[], notifies=[dict(
            notifyId='source-system/' + suffix, sourceType='PlayParticleEffect', localTimeSeconds=0,
            durationSeconds=seconds, serializedPayload=cue, serializedLabels=[],
            assetReferences=[dict(className='ParticleSystem', objectPath=system)])])]))
    source.write(evidence / 'selected_source_actions.json', selected)
    boss_socket = socket_contract('mn_rpct_05.mesh.mn_rpct_05_sk')
    doll_socket = socket_contract('mn_cdmd_00.mesh.mn_cdmd_00_sk_loc_int')
    source.write(evidence / 'boss_source_socket_contract.json', boss_socket)
    source.write(evidence / 'doll_source_socket_contract.json', doll_socket)
    # Names overlap across actors, so choose the contract by the owning action.
    original_decode = source.decode_typed_payload
    def decode(kind, payload, sockets=None, references=None, labels=None):
        if isinstance(payload, dict) and payload.get('sourceKind') == 'ORIGINAL_PARTICLE_SYSTEM_LIBRARY_PREVIEW':
            return copy.deepcopy(payload)
        selected_socket = doll_socket if any('cdmd' in r.get('objectPath', '').lower() for r in references or []) else boss_socket
        return original_decode(kind, payload, selected_socket, references, labels)
    source.decode_typed_payload = decode
    source.ACTION = evidence / 'selected_source_actions.json'
    source.SELECTED = {4219951: ([1, 2], 'Staff flame'), 4222305: ([s['stageIndex'] for s in doll['stages']], 'Odd Doll flame')}
    source.SELECTED.update({key: ([0], row[1]) for key, row in LIBRARIES.items()})
    index, notifies, occurrences, records = source.acquire(evidence, external_package_resolver=lambda logical: native.pkg(logical).path)
    # Source next-stage notifies own the stage boundary. The casting clip is
    # 1.666667s, but this action explicitly advances at 1.56s; its one-second
    # fire clip remains in the second stage until the 1.9s transition.
    cast, fire = flame['stages']
    cast_window = next(n['localTimeSeconds'] for n in cast['notifies'] if n['sourceType'] == 'MonsterMoveNextStage')
    fire_window = next(n['localTimeSeconds'] for n in fire['notifies'] if n['sourceType'] == 'MonsterMoveNextStage')
    by_notify = {n['notifyId']: n for n in notifies}
    for notify in notifies:
        if notify['actionId'] == 4219951 and notify['selectedStage'] == 2:
            notify['globalTimeSeconds'] = cast_window + notify['localTimeSeconds']
    for occurrence in occurrences:
        occurrence['sourceTimeSeconds'] = by_notify[occurrence['sourceNotify']]['globalTimeSeconds']
    source.write(evidence / 'source_notifies.json', notifies)
    source.write(evidence / 'source_occurrences.json', occurrences)
    source.write(evidence / 'staff_action_timeline.json', dict(actionId=4219951,
        stages=[dict(sourceStage=1, runtimeClip='rpct00_att_battle_9_01', startSeconds=0, playSeconds=cast_window),
                dict(sourceStage=2, runtimeClip='rpct00_att_battle_9_02', startSeconds=cast_window, playSeconds=fire_window),
                dict(sourceStage=3, runtimeClip='rpct00_att_battle_9_03', startSeconds=cast_window + fire_window, playSeconds=2)],
        stageBoundarySource='ORIGINAL_MONSTER_MOVE_NEXT_STAGE_NOTIFY'))
    source.write(evidence / 'selection_summary.json', dict(
        documents={str(key):dict(assetId='effect.kouku.gate3.' + suffix + '.full.restore',
            baseEmitterCount=sum(o['actionId'] == key for o in occurrences))
            for key, (suffix, _) in {**ASSETS, **{k: v[:2] for k, v in LIBRARIES.items()}}.items()},
        materialCount=len({o['sourceMaterial'] for o in occurrences if o['rendererShape'] != 'light'}),
        shapes=sorted({o['rendererShape'] for o in occurrences})))
    return index, notifies, occurrences, records


def project(evidence, material_patch, install):
    data = acquire(evidence)
    source.project(evidence, *data, evidence / 'projected', material_patch)
    documents, projected = [], {}
    for key, (suffix, title) in {**ASSETS, **{k: v[:2] for k, v in LIBRARIES.items()}}.items():
        document = source.read(evidence / 'projected' / f'effect.kouku.gate1.{key}.full.restore.effect.json')
        asset = 'effect.kouku.gate3.' + suffix + '.full.restore'
        document.update(effectAssetId=asset, displayName=title)
        for element in document['elements']:
            element['groupId'] = asset
            if key in LIBRARIES:
                element['actionCueAttachment']['enabled'] = False
                element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees', None)
        if key == 4219951:
            document['sourceModelPreview'] = dict(gateId='GATE3', actorProfileId='MN_RPCT_05',
                targetBossPlacementId='boss.kakulsaydon.g3.saydon', animations=[
                    dict(runtimeClip='rpct00_att_battle_9_01', startOffsetMs=0, sourceStartMs=0,
                         playMs=1560, playRate=1, endPolicy='HOLD_LAST_POSE'),
                    dict(runtimeClip='rpct00_att_battle_9_02', startOffsetMs=1560, sourceStartMs=0,
                         playMs=1900, playRate=1, endPolicy='HOLD_LAST_POSE'),
                    dict(runtimeClip='rpct00_att_battle_9_03', startOffsetMs=3460, sourceStartMs=0,
                         playMs=2000, playRate=1, endPolicy='HOLD_LAST_POSE')])
        elif key == 4222305:
            document['modelCues'] = [dict(cueId='kouku.odd_doll.flame.model',
                modelAssetId='Character/KoukuSaton/MN_CDMD_00/MN_CDMD_00.wmodel',
                clipName='att_battle_2_01', startDelaySeconds=0, durationSeconds=11.333333015441895,
                opacity=1, colorMultiply=[1, 1, 1, 1], holdLastFrame=True, alphaMode='OPAQUE', visible=True,
                localTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[.01, .01, .01],
                    revolutionDegreesPerSecond=[0, 0, 0], velocityPerSecond=[0, 0, 0]),
                assetPreTransform=dict(scale=[1, 1, 1], rotationDegrees=[0, 0, 0]))]
            # Installed OddDoll has unit bone bases and centimetre translations,
            # unlike RPCT05's 100x FBX skeleton basis. The cue moves the model
            # centimetres into world metres. Source socket offsets must stay in
            # that model basis; source particle values are already in metres.
            for element in document['elements']:
                attachment = element['actionCueAttachment']
                attachment['modelCueId'] = 'kouku.odd_doll.flame.model'
                attachment['runtimeAnchorSlotId'] = 'kouku.odd_doll.' + attachment['sourceAnchorSlotId']
                attachment['socketLocalTransform']['position'] = [v * 100 for v in attachment['socketLocalTransform']['position']]
                element['detail']['transform']['scale'] = [v * 100 for v in element['detail']['transform']['scale']]
        elif key == 103:
            document, floor_fit = complete_small_pentagram(document)
            source.write(evidence / 'small_pentagram_floor_fit.json', floor_fit)
        source.write(evidence / 'candidate' / (asset + '.effect.json'), document)
        projected[key] = document
        documents.append(dict(effectAssetId=asset, path='Data/Effects/Authored/' + asset + '.effect.json',
            displayName=title, category='Flame Staff' if key == 4219951 else 'Mario / Odd Doll' if key == 4222305 else 'Mario / Circles',
            sourceSystems=sorted({e['sourceRecipe']['sourceSystemId'] for e in document['elements']}) if all('sourceSystemId' in e['sourceRecipe'] for e in document['elements']) else sorted({o['sourceSystem'] for o in data[2] if o['actionId']==key}),
            nativeProfiles=sorted({e['material'].get('sourceProfile', {}).get('runtimeShaderProfileId', '') for e in document['elements']} - {''}),
            elementCount=len(document['elements']), durationMs=source.read(evidence / 'projection.json')['durationMs'][str(key)]))
        if install:
            assert material_patch, 'Install requires recovered original native material inputs'
            destination = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
            assert not destination.exists() or source.read(destination) == document, 'Preserve authored edits'
            source.write(destination, document)
    combined = copy.deepcopy(projected[101])
    combined.update(effectAssetId='effect.kouku.gate3.mario.center.full.restore', displayName='Mario central circle and portal')
    combined['elements'] += copy.deepcopy(projected[102]['elements'])
    for element in combined['elements']:
        element['groupId'] = combined['effectAssetId']
    source.write(evidence / 'candidate' / (combined['effectAssetId'] + '.effect.json'), combined)
    documents.append(dict(effectAssetId=combined['effectAssetId'],
        path='Data/Effects/Authored/' + combined['effectAssetId'] + '.effect.json', displayName=combined['displayName'],
        category='Mario / Circles', sourceSystems=[LIBRARIES[101][2], LIBRARIES[102][2]],
        sourceProjectileId=422200101, previewWindowSeconds=20, elementCount=len(combined['elements']),
        durationMs=max(row['durationMs'] for row in documents if row['effectAssetId'] in (projected[101]['effectAssetId'], projected[102]['effectAssetId']))))
    if install:
        destination = ROOT / 'Data/Effects/Authored' / (combined['effectAssetId'] + '.effect.json')
        assert not destination.exists() or source.read(destination) == combined, 'Preserve authored edits'
        source.write(destination, combined)
    source.write(evidence / 'installation.json', dict(installed=install, documents=documents, manualVisualValidation='USER_PENDING'))
    print(json.dumps(documents))


def prepare_geometry(evidence):
    sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
    import cook_wmodel_geometry_contract as geometry
    resources = ROOT / 'Client/Bin/Resources'
    source_manifest = source.SOURCE / 'CanonicalSource/Effect/Closure/closure.manifest.json'
    package_row = next(p for p in source.read(source_manifest)['sourcePackages'] if p['logicalPackage'].lower() == 'fx_sm_00')
    package = source_manifest.parent / package_row['outputRelativePath']
    converter = ROOT / 'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe'
    digest = lambda p: hashlib.sha256(p.read_bytes()).digest()
    rows = []
    for mesh in sorted({o['sourceMesh'] for o in source.read(evidence / 'source_occurrences.json') if o['sourceMesh']}):
        logical, name = mesh.split('.')
        gltf = source.SOURCE / 'EffectRuntimeClosureExports-20260829' / logical / 'Export' / logical.upper() / (name + '.gltf')
        legacy = resources / 'Effect/KoukuSaydon/Meshes' / logical / (name + '.wmodel')
        if not legacy.exists():
            legacy = evidence / 'geometry' / (name + '.wmodel')
            legacy.parent.mkdir(parents=True, exist_ok=True)
            if not legacy.exists():
                subprocess.run([str(converter), str(gltf), '-o', str(legacy), '--scale', '100', '--no-auto-textures'], check=True)
        observed = evidence / 'geometry' / (name + '.source.json')
        source.write(observed, dict(sourceObject=mesh, sourceGltf=str(gltf), legacyWmodel=str(legacy),
            sourceGltfSha256=digest(gltf).hex(), legacyWmodelSha256=digest(legacy).hex()))
        provenance = geometry.GeometryProvenanceEvidence(mesh, digest(source_manifest), 'OBSERVED_SOURCE_RECEIPT',
            digest(package), digest(converter), digest(observed), digest(observed))
        payload, receipt = geometry.cook_wmodel_geometry_contract(gltf, legacy, provenance)
        destination = resources / 'Effect/KoukuSaydon/FullRestore/Meshes' / (name + '.wmodel')
        assert not destination.exists() or destination.read_bytes() == payload, 'Preserve differing runtime mesh'
        destination.parent.mkdir(parents=True, exist_ok=True)
        if not destination.exists():
            destination.write_bytes(payload)
        source.write(evidence / 'geometry' / (name + '.cook.json'), receipt)
        rows.append(dict(sourceObject=mesh, assetId=destination.relative_to(resources).as_posix(),
                         vertexCount=receipt['geometry'], bytes=len(payload)))
    source.write(evidence / 'geometry_installation.json', rows)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuSlamMario20260912')
    parser.add_argument('--native-material-patch', type=Path)
    parser.add_argument('--project', action='store_true')
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--prepare-geometry', action='store_true')
    parser.add_argument('--complete-small-pentagram', action='store_true')
    arguments = parser.parse_args()
    if arguments.complete_small_pentagram:
        destination = ROOT / 'Data/Effects/Authored/effect.kouku.gate3.mario.boss.pentagram.full.restore.effect.json'
        before = destination.read_bytes()
        document, fit = complete_small_pentagram(source.read(destination))
        assert destination.read_bytes() == before, 'Preserve concurrent authored edits'
        source.write(destination, document)
        source.write(arguments.evidence_root / 'small_pentagram_floor_fit.json', fit)
        print(json.dumps(fit))
    elif arguments.prepare_geometry:
        prepare_geometry(arguments.evidence_root)
    elif arguments.project or arguments.install:
        project(arguments.evidence_root, arguments.native_material_patch, arguments.install)
    else:
        acquire(arguments.evidence_root)
