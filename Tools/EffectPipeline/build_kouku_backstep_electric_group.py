"""Stage the electric ribbon capacity repair and source projectile preview.

No live authoring is written. Existing editing is preserved by a capacity-only
candidate. --aim-input creates a separately registered source-component preview
from three explicit authoring destinations; source MemoryPos is not a fixed fan.
"""
import argparse
import base64
import copy
import hashlib
import json
import math
from pathlib import Path
import re
import sqlite3
import struct

import build_kouku_gate1_full_restore as source
from build_kouku_effect_organization import effect_payload_id
from build_kouku_backstep_flame_groups import remap
from extract_action_effect_notifies import scan_length_prefixed_strings

ROOT = source.ROOT
EVIDENCE = ROOT / 'out/KoukuBackstepElectricRepair20260913'
ACTION = source.SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json'
DATABASE = source.SOURCE / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
PROJECTILE = ROOT / 'out/KoukuAllEffects20260912/source/Projectile/421990316.loa'
SYSTEM = 'fx_mn_rpct_07_v.par_v_rpct_elec_prj_02_loc_int'
LEAF = ROOT / 'Data/Effects/Authored' / ('effect.kouku.source.' + SYSTEM + '.effect.json')
ASSET = 'effect.kouku.gate3.backstep.electric.threeway.authored'
DISPLAY_NAME = '백스텝_세갈래전격(편집용배치)'
PREVIEW_ASSET = 'effect.kouku.gate3.backstep.electric.source.aim.preview'
PREVIEW_NAME = '감전빔 원본구성(도착점 편집)'
AREA_SYSTEM = 'fx_mn_rpct_07_v.par_v_rpct_elecarea_02_loc_int'
LIGHT_SYSTEM = 'fx_cm_02.light.par_mp_light_01_sine'
SOURCE_CACHE = ROOT / 'out/KoukuAllEffects20260912'


def digest(path):
    return dict(path=path.as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def reflected_contract(raw):
    # The old gameplay intake guessed Speed/MaxSpeed as min/max distance.
    # Validate the real class declarations and all 19 preceding serialized
    # flags before assigning names to these concrete LOA fields.
    from build_kouku_gate3_rainbow_native import RELEASE
    package_path = RELEASE / 'NU1V7NCQ4YAE9ZPJVNOQS.u'
    package = source.load_package(package_path, source.ue3.LOSTARK_KR_AES_KEY)
    def fields(name):
        owner = next(e for e in package.exports if e.object_name == name and
            source.ue3.package_ref_name(e.class_index, package.imports, package.exports).lower() == 'class')
        return [e.object_name for e in reversed(package.exports) if e.package_index == owner.index + 1 and
            source.ue3.package_ref_name(e.class_index, package.imports, package.exports).lower().endswith('property')]
    base_fields = fields('EFSequenceSummonsProjectile')
    assert base_fields[:19] == ['LocalControl', 'DestroyWhenSkillEnd', 'DestroyWhenSkillStageEnd',
        'DestroyWhenOwnerDied', 'DestroyWhenEnterParalyzation', 'RandomCreateDelayTime', 'ReversedDirection',
        'bIgnoreEffectShowOption', 'SyncZoneObject', 'DestoryClearFX', 'IgnoreBulletTime', 'AffectedByAtkSpd',
        'BeneficialPartyEffect', 'IsMoveableProjectile', 'Penerate', 'CollisionPreCheck',
        'bExcuteDestructionAfterHit', 'bDeactivateSequenceOnDestructionByCondition', 'bUseCollideAttackFilter']
    assert list(struct.unpack_from('<19I', raw, 56)) == [0] * 11 + [1, 0, 1, 0, 1, 1, 0, 0]
    at = base_fields.index('ResScale')
    assert base_fields[at:at + 8] == ['ResScale', 'CollisionSize', 'CollisionSize_HeightClientOnly',
        'Speed', 'MaxSpeed', 'Lifetime', 'MaxDistance', 'MaxApplyCount']
    assert 'MinDistance' not in base_fields
    assert fields('EFSequenceSummonsProjectileGrenade') == ['GrenadeMinHeight', 'GrenadeMaxHeight',
        'GrenadeMaxheightRatio', 'GrenadeMaxheightStartDist', 'StandardDistance', 'bOnNavMeshOnly',
        'ExtraBounceStartOffset', 'ExtraBounceDatas']
    enums = {}
    for name in ('SkillAreaOrigin', 'EFSummonsATSelect', 'MemoryPosType'):
        entry = next(e for e in package.exports if e.object_name == name and
            source.ue3.package_ref_name(e.class_index, package.imports, package.exports).lower() == 'enum')
        serial = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        count = struct.unpack_from('<I', serial, 16)[0]
        assert len(serial) == 20 + count * 8
        enums[name] = [package.names[struct.unpack_from('<I', serial, 20 + i * 8)[0]] for i in range(count)]
    assert enums['SkillAreaOrigin'][3] == 'SKILL_AREA_ORIGIN_MEMORIZED_AIM'
    assert enums['EFSummonsATSelect'][struct.unpack_from('<I', raw, 152)[0]] == 'EFSATS_AimPos'
    # The three packed particle blocks follow Trail / Explode / EnvExplode.
    # Explode owns one AdditionalParticleData (light); its sound is separate.
    assert base_fields[26:32] == ['TrailParticleData', 'ChangeTrailParticleDataArray',
        'ExplodeParticleData', 'ChangeExplodeParticleDataArray', 'EnvExplodeParticleData', 'ChangeEnvExplodeParticleDataArray']
    assert struct.unpack_from('<I', raw, 969)[0] == 1
    assert struct.unpack_from('<I', raw, 1712)[0] == 0
    # Start list: one AkEvent. Arrival/Hit list: SkillEffect + CameraShake.
    # CameraViewShake has 6 scalar fields, Loc(3*3), FOV(3), Rot(3*3), all
    # 4-byte serialized lanes: 27 values. Its end gives the two empty lists,
    # then the exact 40-byte zero-height/no-bounce Grenade tail.
    assert struct.unpack_from('<I', raw, 2034)[0] == 1
    assert struct.unpack_from('<I', raw, 2205)[0] == 2
    assert raw[2213:2249] == b'CEFSequenceSummonsActionSkillEffect\0'
    assert struct.unpack_from('<I', raw, 2281)[0] == 421990322
    assert raw[2293:2329] == b'CEFSequenceSummonsActionCameraShake\0'
    assert raw[2365:2384] == b'CEFCameraViewShake\0'
    assert raw[2492:2500] == bytes(8) and raw[2500:] == bytes(40)
    return dict(sourcePackage=digest(package_path), projectileFields=base_fields,
        grenadeFields=fields('EFSequenceSummonsProjectileGrenade'), enums=enums,
        motionFieldsByteOffset=1994, grenadeFieldsByteOffset=2500,
        particleSlots=[dict(slot='TrailParticleData', sourceByteOffset=160, system=SYSTEM),
            dict(slot='ExplodeParticleData', sourceByteOffset=515, system=AREA_SYSTEM,
                additionalParticles=[dict(sourceByteOffset=973, system=LIGHT_SYSTEM)]),
            dict(slot='EnvExplodeParticleData', sourceByteOffset=1716, system=None)],
        arrivalActions=['SkillEffect:421990322', 'CameraShake'],
        grenadeHeightCm=[0, 0], extraBounceCount=0,
        previousMotionInterpretationRejected='7500/7500 are Speed/MaxSpeed; 5000 is MaxDistance. No MinDistance property exists.')


def contract():
    actions = source.read(ACTION)['actions']
    backstep = next(a for a in actions if a['actionId'] == 4219921)
    action = next(a for a in actions if a['actionId'] == 4219962)
    assert len(backstep['stages']) == 1 and len(action['stages']) == 3
    assert backstep['stages'][0]['animationClips'][0]['clipName'] == 'Att_Battle_34_02'
    assert not any(n['sourceType'] == 'Effect' for n in backstep['stages'][0]['notifies'])
    assert [s['animationClips'][0]['clipName'] for s in action['stages']] == [
        'Idle_Battle_1', 'Att_Battle_34_02', 'Att_Battle_24_06']
    transition = next(n for n in action['stages'][1]['notifies'] if n['sourceType'] == 'MonsterMoveNextStage')
    assert abs(transition['localTimeSeconds'] - .9) < 1e-6
    launches = []
    with sqlite3.connect('file:' + DATABASE.as_posix() + '?mode=ro', uri=True) as database:
        database.row_factory = sqlite3.Row
        for notify in action['stages'][2]['notifies']:
            if notify['sourceType'] != 'Effect':
                continue
            raw = base64.b64decode(notify['serializedPayload']['data'])
            identifier, offset = effect_payload_id(notify['serializedPayload'])
            # Common CEFActionNotify header: class C string, three u32 values,
            # then bEnabled before the start/end/duration float triplet.
            enabled_offset = raw.index(0) + 1 + 12
            enabled = struct.unpack_from('<I', raw, enabled_offset)[0]
            assert enabled in (0, 1) and enabled_offset == 35
            timing = struct.unpack_from('<3f', raw, enabled_offset + 4)
            assert abs(timing[0] - notify['localTimeSeconds']) < 1e-6
            assert abs(timing[2] - notify['durationSeconds']) < 1e-6
            row = dict(database.execute('select * from SkillEffect where PrimaryKey=?', (identifier,)).fetchone())
            assert row['Key'] == 12 and row['ValueA'] == 421990316
            position = list(struct.unpack_from('<3f', raw, offset + 40))
            angle = struct.unpack_from('<i', raw, offset + 4)[0]
            assert position == [150, 0, 100] and angle == 0
            launches.append(dict(notifyId=notify['notifyId'], skillEffectId=identifier,
                enabled=bool(enabled), enabledByteOffset=enabled_offset,
                sourceStartSeconds=notify['localTimeSeconds'], sourcePositionCm=position,
                sourceAngleDegrees=angle, sourceArea={k: v for k, v in row.items() if k.startswith('Area')},
                sourcePayloadSha256=notify['serializedPayload']['sha256']))
    assert [r['skillEffectId'] for r in launches if r['enabled']] == [421990319, 421990321, 421990323]
    assert [r['skillEffectId'] for r in launches if not r['enabled']] == [421990313, 421990315]
    assert all(abs(r['sourceStartSeconds'] - 1.3) < 1e-6 for r in launches)
    raw = PROJECTILE.read_bytes()
    tokens = scan_length_prefixed_strings(raw, 0, len(raw))
    assert tokens[0]['value'] == 'CEFSequenceSummonsProjectileGrenade'
    particles = [t for t in tokens if t['value'].startswith("ParticleSystem'")]
    assert particles[0]['value'].split("'")[1].lower() == SYSTEM
    assert particles[0]['sourceOffset'] == 254
    # This concrete record has an FX_Prj_01 source attachment string. Its
    # source scale starts at 471, after that variable-length string; using a
    # fixed +136 after ParticleSystem would read the wrong field.
    assert next(t for t in tokens if t['sourceOffset'] == 381)['value'] == 'FX_Prj_01'
    particle_scale = list(struct.unpack_from('<3f', raw, 471))
    assert particle_scale == [1, 1, 1]
    reflected = reflected_contract(raw)
    motion = dict(scale=struct.unpack_from('<f', raw, 1994)[0],
        radiusCm=struct.unpack_from('<f', raw, 1998)[0], heightCm=struct.unpack_from('<f', raw, 2002)[0],
        speedCmPerSecond=struct.unpack_from('<i', raw, 2006)[0], maxSpeedCmPerSecond=struct.unpack_from('<i', raw, 2010)[0],
        maxLifeSeconds=struct.unpack_from('<f', raw, 2014)[0], maxDistanceCm=struct.unpack_from('<i', raw, 2018)[0])
    assert motion == dict(scale=1, radiusCm=50, heightCm=50, speedCmPerSecond=7500,
        maxSpeedCmPerSecond=7500, maxLifeSeconds=5, maxDistanceCm=5000), motion
    memory = next(n for n in action['stages'][0]['notifies'] if n['sourceType'] == 'MemoryPos')
    return dict(sourceActionId=4219962, sourceBackstepActionId=4219921, projectileId=421990316,
        sourceBackstepNextStageSeconds=transition['localTimeSeconds'],
        sourceCastLaunchSeconds=1.3, launches=launches, sourceParticleScale=particle_scale,
        sourceProjectileClass='CEFSequenceSummonsProjectileGrenade', sourceMotion=motion, reflectedLayout=reflected,
        sourceMemoryPosNotify=memory,
        sourceInputs=[digest(p) for p in (ACTION, DATABASE, PROJECTILE, LEAF)],
        followupSystems=[t['value'].split("'")[1].lower() for t in particles[1:]],
        enabledHeaderBasis='CEFActionNotify common enabled before matching start/end/duration triplet; disabled original launches remain excluded.',
        originalCallerBoundary='4219921 contains backstep animation and hand trail only. 4219962 owns the following enabled three-projectile attack.',
        authoredBoundary='Three explicit editable destinations substitute for unavailable live MEMORIZED_AIM/AreaOriginOption resolution. Source zero-height projectile speed, launch offset, arrival slot and ParticleSystem modules are retained. No static-image fan angles or Server gameplay are synthesized.')


def key(time, value, interpolation='linear'):
    return dict(timeSeconds=time, value=value, arriveTangent=[0, 0, 0],
        leaveTangent=[0, 0, 0], interpolation=interpolation)


def ribbon_capacity(element, speed_cm):
    modules = element['sourceRecipe']['modules']
    per_unit = next(m for m in modules if m['className'] == 'particlemodulespawnperunit')
    unit = next(v['value'] for v in per_unit['literals'] if v['propertyPath'] == 'unitscalar')
    rate = source.constant_distribution_value(next(d for d in per_unit['distributions']
        if d['propertyPath'] == 'spawnperunit'))[0]
    native = next(m for m in modules if m['className'] == 'particlemoduletypedataribbon')
    original = int(next(v['value'] for v in native['literals'] if v['propertyPath'] == 'maxparticleintrailcount'))
    lifetime = max(element['detail']['particle']['lifeTimeSeconds'])
    fixed_step = 1 / 60
    capacity = min(original, math.ceil(speed_cm * lifetime * rate / unit) +
        math.ceil(speed_cm * fixed_step * rate / unit) + 2)
    assert 2 <= capacity <= 512
    return capacity, dict(sourceMaxPoints=original, sourceUnitScalarCm=unit, sourceSpawnPerUnit=rate,
        sourceParticleLifetimeSeconds=lifetime, evaluatedSpeedCmPerSecond=speed_cm,
        fixedStepSeconds=fixed_step, requiredCapacity=capacity)


def track_speed(element):
    tracks = element['sourceTransformTrack']['nodes']
    return max(math.dist(a['value'], b['value']) / (b['timeSeconds'] - a['timeSeconds'])
        for track in tracks for a, b in zip(track['positionKeys'], track['positionKeys'][1:]))


def stage(evidence, target, document, operations):
    relative = target.relative_to(ROOT)
    candidate = evidence / 'candidate' / relative
    before = digest(target) if target.is_file() else dict(path=target.as_posix(), sha256=None)
    if target.is_file():
        baseline = evidence / 'baseline' / relative
        baseline.parent.mkdir(parents=True, exist_ok=True)
        baseline.write_bytes(target.read_bytes())
    source.write(candidate, document)
    operations.append(dict(target=relative.as_posix(), candidate=candidate.relative_to(evidence).as_posix(),
        baselineSha256=before['sha256'], candidateSha256=digest(candidate)['sha256']))


def validate(document):
    resources = set()
    def visit(value):
        if isinstance(value, dict):
            for item in value.values(): visit(item)
        elif isinstance(value, list):
            for item in value: visit(item)
        elif isinstance(value, str) and value.startswith('Effect/'):
            assert '..' not in Path(value).parts and ':' not in value
            resources.add(value)
    visit(document)
    missing = [r for r in resources if not (ROOT / 'Client/Bin/Resources' / r).is_file()]
    assert not missing, missing
    assert len({e['id'] for e in document['elements']}) == len(document['elements'])
    assert all(re.fullmatch(r'[A-Za-z0-9_.-]{1,128}', e['id']) for e in document['elements'])
    assert all(len(e['displayName'].encode('utf-8')) <= 64 for e in document['elements'])
    assert all(len(e['sourceNode'].encode('utf-8')) <= 256 for e in document['elements'])
    points = sum(e['detail']['trail']['maxPoints'] for e in document['elements'] if e['kind'] == 'trail')
    assert points <= 2048
    json.dumps(document, allow_nan=False)
    return dict(elementCount=len(document['elements']), totalTrailCapacity=points,
        resourceCount=len(resources), missingResources=missing)


def particle_parameters(raw, offset):
    token = next(t for t in scan_length_prefixed_strings(raw, 0, len(raw)) if t['sourceOffset'] == offset)
    base = offset + 5 + len(token['value'])
    assert struct.unpack_from('<2I', raw, base + 52) == (0, 0)
    position = list(struct.unpack_from('<3f', raw, base + 76))
    rotation = list(struct.unpack_from('<3i', raw, base + 100))
    scale = list(struct.unpack_from('<3f', raw, base + 136))
    count = struct.unpack_from('<I', raw, base + 148)[0]
    cursor = base + 152
    parameters = []
    for _ in range(count):
        length = struct.unpack_from('<I', raw, cursor)[0]
        assert 0 < length < 256 and raw[cursor + 3 + length] == 0
        name = raw[cursor + 4:cursor + 3 + length].decode('ascii')
        cursor += 4 + length
        kind = struct.unpack_from('<I', raw, cursor)[0]
        if kind == 1:
            parameter = dict(name=name, type='scalar', scalarValue=struct.unpack_from('<f', raw, cursor + 4)[0])
        else:
            assert kind == 3
            parameter = dict(name=name, type='vector', vectorValue=list(struct.unpack_from('<3f', raw, cursor + 12)))
        assert raw[cursor + 56:cursor + 65] == b'\x05\0\0\0None\0'
        cursor += 69
        parameters.append(parameter)
    return dict(sourceByteOffset=offset, sourceSystem=token['value'].split("'")[1].lower(),
        sourcePositionCm=position, sourceRotator=rotation, sourceScale=scale, parameterOverrides=parameters,
        parameterTableEndByteOffset=cursor)


def source_light(proof, evidence):
    from build_kouku_action_effect_groups import restored_index, project_light_occurrences
    index = restored_index(SOURCE_CACHE)
    particles = particle_parameters(PROJECTILE.read_bytes(), 1005)
    assert particles['sourcePositionCm'] == [0, 0, 100] and particles['sourceRotator'] == [0, 0, 0]
    assert len(particles['parameterOverrides']) == 6
    occurrences = [o for o in source.read(SOURCE_CACHE / 'source_occurrences.json')
        if o['sourceEmitter'].startswith(LIGHT_SYSTEM + '.')]
    assert len(occurrences) == 1
    original = next(n for n in source.read(SOURCE_CACHE / 'source_notifies.json')
        if n['notifyId'] == occurrences[0]['sourceNotify'])
    cue = copy.deepcopy(original['cue'])
    cue['parameterOverrides'] = copy.deepcopy(particles['parameterOverrides'])
    cue['localTransform'] = dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=particles['sourceScale'])
    result = project_light_occurrences(index, occurrences, cue, original, evidence)
    proof['explodeParticle'] = particle_parameters(PROJECTILE.read_bytes(), 609)
    proof['additionalLightParticle'] = particles
    return result


def append_system(document, original, branch, role, launch, origin, destination, flight, scale=None):
    group = 'kouku.electric.aim.' + str(branch)
    occurrence = group + '.' + role
    identities = {e['id']: occurrence + '.' + hashlib.sha256(e['id'].encode()).hexdigest()[:20]
        for e in original['elements']}
    elements = remap(copy.deepcopy(original['elements']), identities)
    yaw = math.degrees(math.atan2(destination[1] - origin[1], destination[0] - origin[0]))
    position = origin if role == 'flight' else destination
    for ordinal, element in enumerate(elements):
        element.update(groupId=group, displayName=f'{branch + 1}번 ' + {'flight':'비행', 'arrival':'폭발', 'light':'라이트'}[role] + f' {ordinal + 1}',
            sourceNode=occurrence + '|' + element['sourcePresentation']['sourceObjectPath'])
        element['actionCueAttachment'].update(enabled=True, follow=False, sourceAnchorSlotId='root',
            runtimeAnchorSlotId='root', runtimeBoneName='', snapshotRootSourceBasisYawDegrees=-90,
            socketLocalTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1]))
        element['detail']['transform'].update(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
        delay = 0 if role == 'flight' else flight
        element['detail']['timing']['startDelaySeconds'] += delay
        element['sourcePresentation'].update(sourceEventId=launch['notifyId'] + '/' + role,
            sourceActionCueId=launch['notifyId'], sourceTimeSeconds=element['sourcePresentation'].get('sourceTimeSeconds', 0) + delay)
        element['sourceTransformTrack'] = dict(sourceOccurrenceId=occurrence, sourceTimeOriginSeconds=0,
            previewOriginUE3Cm=[0, 0, 0], nodes=[dict(sourceObjectPath=occurrence, frame='WORLD',
                initialPositionUE3Cm=position, initialEulerDegrees=[0, 0, yaw if role == 'flight' else 0],
                scaleUE3=scale or [1, 1, 1], positionKeys=([key(0, origin), key(flight, destination)] if role == 'flight' else []), eulerKeys=[])])
        if 'particleSystemOccurrenceId' in element['sourceRecipe']:
            element['sourceRecipe']['particleSystemOccurrenceId'] = occurrence
        if element['kind'] == 'trail':
            capacity, _ = ribbon_capacity(element, math.dist(origin, destination) / flight)
            element['detail']['trail']['maxPoints'] = capacity
        document['elements'].append(element)
    return [e['id'] for e in elements]


def build_source_preview(evidence, aim_input, proof, operations):
    inputs = source.read(aim_input)
    assert inputs['formatVersion'] == 1 and inputs['coordinateBasis'] == 'UE3_CM_OWNER_X_FORWARD_Z_UP'
    assert inputs['provenance'] == 'PROJECT_AUTHORED_EDITABLE_DESTINATIONS'
    aims = inputs['destinationsUE3Cm']
    assert len(aims) == 3 and all(len(p) == 3 and all(math.isfinite(v) for v in p) for p in aims)
    library = source.read(LEAF)
    area_path = ROOT / 'Data/Effects/Authored' / ('effect.kouku.source.' + AREA_SYSTEM + '.effect.json')
    area = source.read(area_path)
    assert len(library['elements']) == 6 and len(area['elements']) == 14
    light = source_light(proof, evidence)
    document = copy.deepcopy(library)
    document.update(effectAssetId=PREVIEW_ASSET, displayName=PREVIEW_NAME, elements=[])
    document.pop('sourceModelPreview', None)
    document['particleSystem'].update(uniformScaleMultiplier=1, yawOffsetDegrees=0, directionYawDegrees=0, initialSpeedMultiplier=1)
    tracks = []
    for branch, (launch, destination) in enumerate(zip([r for r in proof['launches'] if r['enabled']], aims)):
        origin = launch['sourcePositionCm']
        distance = math.dist(origin, destination)
        assert 0 < distance <= proof['sourceMotion']['maxDistanceCm'], 'Destination must stay within the source maximum range.'
        flight = distance / proof['sourceMotion']['speedCmPerSecond']
        assert flight < proof['sourceMotion']['maxLifeSeconds']
        flight_ids = append_system(document, library, branch, 'flight', launch, origin, destination, flight)
        area_ids = append_system(document, area, branch, 'arrival', launch, origin, destination, flight)
        light_position = [a + b for a, b in zip(destination, proof['additionalLightParticle']['sourcePositionCm'])]
        light_ids = append_system(document, light, branch, 'light', launch, origin, light_position, flight,
            proof['additionalLightParticle']['sourceScale'])
        tracks.append(dict(branch=branch, groupId='kouku.electric.aim.' + str(branch),
            sourceLaunchOffsetCm=origin, editableDestinationUE3Cm=destination, arrivalSeconds=flight,
            sourceSpeedCmPerSecond=proof['sourceMotion']['speedCmPerSecond'], flightElementIds=flight_ids,
            arrivalElementIds=area_ids, additionalLightElementIds=light_ids))
    proof['sourceInputs'] += [digest(area_path), digest(aim_input)]
    source.write(evidence / 'editable_destinations.json', inputs)
    source.write(evidence / 'source_preview_tracks.json', tracks)
    stage(evidence, ROOT / 'Data/Effects/Authored' / (PREVIEW_ASSET + '.effect.json'), document, operations)
    catalog_path = ROOT / 'Data/Effects/EffectCatalog.json'
    catalog = source.read(catalog_path)
    assert not any(e['effectAssetId'] == PREVIEW_ASSET for e in catalog['effects'])
    catalog['effects'].append(dict(effectAssetId=PREVIEW_ASSET, payloadKind='DIRECT_AUTHORED_DOCUMENT',
        authoringPath='Effects/Authored/' + PREVIEW_ASSET + '.effect.json'))
    stage(evidence, catalog_path, catalog, operations)
    tree_path = ROOT / 'Data/Effects/EffectResourceTree.json'
    tree = source.read(tree_path)
    parent = next(n['parentId'] for n in tree['references'] if n.get('assetId') == ASSET)
    node = dict(parentId=parent, kind='V1', displayName=PREVIEW_NAME, assetId=PREVIEW_ASSET)
    tree['references'].append(node)
    stage(evidence, tree_path, tree, operations)
    return validate(document)


def build(evidence, aim_input=None):
    evidence = evidence.resolve()
    assert evidence == EVIDENCE.resolve(), 'Keep task output under its isolated evidence directory.'
    evidence.mkdir(parents=True, exist_ok=True)
    proof = contract()
    operations = []
    path = ROOT / 'Data/Effects/Authored' / (ASSET + '.effect.json')
    document = source.read(path)
    capacities = []
    for element in document['elements']:
        if element['kind'] != 'trail': continue
        capacity, row = ribbon_capacity(element, track_speed(element))
        row.update(elementId=element['id'], previousCapacity=element['detail']['trail']['maxPoints'])
        element['detail']['trail']['maxPoints'] = capacity
        capacities.append(row)
    assert len(capacities) == 6
    stage(evidence, path, document, operations)
    validation = dict(existingCapacityRepair=validate(document), capacityEvidence=capacities)
    if aim_input:
        validation['sourcePreview'] = build_source_preview(evidence, aim_input, proof, operations)
    source.write(evidence / 'source_contract.json', proof)
    source.write(evidence / 'installation.json', dict(installed=False, writes=operations, validation=validation,
        sourceContract='source_contract.json', liveCompositionChanged=False,
        runtimeCodecValidation='PENDING', actualCpuPlaybackValidation='PENDING', manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(evidence=evidence.as_posix(), **validation), ensure_ascii=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=EVIDENCE)
    parser.add_argument('--aim-input', type=Path, help='Three explicit editable destinations; no fixed-angle default.')
    args = parser.parse_args()
    build(args.evidence_root, args.aim_input)
