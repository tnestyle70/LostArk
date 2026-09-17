"""Build the two source spinning-card volleys without changing live authoring.

Writes only candidate V1 documents and provenance under the requested evidence
directory. The caller installs/registers the candidate after codec validation.
The original Action stages, native card mesh/material and Cascade spin stay
intact; the existing sourceTransformTrack carries each flying ParticleSystem.
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
import sys

import build_kouku_gate1_full_restore as source
from build_kouku_effect_organization import effect_payload_id
from build_kouku_backstep_flame_groups import remap, project_mesh_rotation
from extract_action_effect_notifies import scan_length_prefixed_strings

ROOT = source.ROOT
AUTHORED = ROOT / 'Data/Effects/Authored'
ASSET = 'effect.kouku.common.spinning.card.throw'
ACTION_ID = 4219819
PROJECTILE_ID = 421981901
PROJECTILE = ROOT / 'out/KoukuAllEffects20260912/source/Projectile/421981901.loa'
STAGE_ROOT = ROOT / 'out/KoukuActionEffects20260912/candidate'
SYSTEM = 'fx_mn_rpct_05_l.par_l_rpct_05_sk_13_1_loc_int'
DATABASE = source.SOURCE / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
CLIP_REFERENCE = ROOT / 'Data/Animation/Reference/KoukuSaydon/MN_RPCT_05.actionreference.json'


def digest(path):
    return dict(path=path.as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def source_contract():
    action = next(a for a in source.read(source.ACTION)['actions'] if a['actionId'] == ACTION_ID)
    clip_reference = next(a for a in source.read(CLIP_REFERENCE)['actions'] if a['sourceActionId'] == ACTION_ID)
    assert [s['stageIndex'] for s in action['stages']] == list(range(6))
    raw = PROJECTILE.read_bytes()
    tokens = scan_length_prefixed_strings(raw, 0, len(raw))
    assert tokens[0]['value'] == 'CEFSequenceSummonsProjectileMissile'
    particles = [t for t in tokens if t['value'].startswith("ParticleSystem'")]
    assert len(particles) == 3
    assert particles[0]['value'].split("'")[1].lower() == SYSTEM
    token = particles[0]
    end = token['sourceOffset'] + 5 + len(token['value'])
    particle = dict(sourceByteOffset=token['sourceOffset'], sourceSystem=SYSTEM,
        sourcePositionCm=list(struct.unpack_from('<3f', raw, end + 76)),
        sourceRotationUnreal=list(struct.unpack_from('<3i', raw, end + 100)),
        sourceScale=list(struct.unpack_from('<3f', raw, end + 136)))
    assert struct.unpack_from('<i', raw, end + 148)[0] == 0
    assert particle['sourcePositionCm'] == [0, 0, 0]
    assert particle['sourceRotationUnreal'] == [0, 0, 0]
    assert particle['sourceScale'] == [1, 1, 1]
    # The concrete Missile record's bounded motion fields follow its three
    # initial CEFProjectileParticleData entries. Keep offsets/value evidence
    # separate from the explicitly selected straight authoring preview route.
    fields = dict(scale=struct.unpack_from('<f', raw, 1197)[0],
        radiusCm=struct.unpack_from('<f', raw, 1201)[0],
        heightCm=struct.unpack_from('<f', raw, 1205)[0],
        speedCmPerSecond=struct.unpack_from('<i', raw, 1209)[0],
        maxSpeedCmPerSecond=struct.unpack_from('<i', raw, 1213)[0],
        maxLifeSeconds=struct.unpack_from('<f', raw, 1217)[0],
        maxDistanceCm=struct.unpack_from('<i', raw, 1221)[0])
    assert fields == dict(scale=1, radiusCm=40, heightCm=30, speedCmPerSecond=800,
        maxSpeedCmPerSecond=1500, maxLifeSeconds=5, maxDistanceCm=1500), fields
    stages, launches, offset = [], [], 0.0
    with sqlite3.connect('file:' + DATABASE.as_posix() + '?mode=ro', uri=True) as database:
        database.row_factory = sqlite3.Row
        table = dict(database.execute('select * from SkillEffect where PrimaryKey=?', (PROJECTILE_ID,)).fetchone())
    assert table['Key'] == 12 and table['ValueA'] == PROJECTILE_ID
    assert [table[k] for k in ('AreaOffsetX', 'AreaOffsetY', 'AreaOffsetZ', 'AreaOffsetAngle')] == [0] * 4
    for stage in action['stages']:
        index = stage['stageIndex']
        clip = stage['animationClips'][0]
        mapped = next(s for s in clip_reference['stages'] if s['stageOrdinal'] == index)['slots']
        assert len(mapped) == 1 and mapped[0]['extractedClip'] == clip['clipName']
        assert mapped[0]['runtimeClip'] == 'rpct00_' + clip['clipName'].lower()
        next_stage = [n for n in stage['notifies'] if n['sourceType'] == 'MonsterMoveNextStage']
        duration = round(next_stage[0]['localTimeSeconds'] if next_stage else clip['lengthSeconds'], 3)
        expected = (2.73, 4, 3.5)[index % 3]
        assert duration == expected
        stages.append(dict(stageIndex=index, startSeconds=offset, durationSeconds=duration,
            runtimeClip=mapped[0]['runtimeClip'],
            sourceClipSeconds=clip['lengthSeconds'],
            endPolicy='LOOP_TO_WINDOW' if index % 3 == 1 else 'HOLD_LAST_POSE'))
        for notify in stage['notifies']:
            if notify['sourceType'] != 'Effect':
                continue
            effect_id, at = effect_payload_id(notify['serializedPayload'])
            if effect_id != PROJECTILE_ID:
                assert effect_id == 421981903
                continue
            payload = base64.b64decode(notify['serializedPayload']['data'])
            angle = struct.unpack_from('<i', payload, at + 4)[0]
            # Serialized Effect occurrence origin is 20cm forward, 70cm up.
            local_position = list(struct.unpack_from('<3f', payload, at + 40))
            assert local_position == [20, 0, 70], local_position
            launches.append(dict(notifyId=notify['notifyId'], stageIndex=index,
                localTimeSeconds=round(notify['localTimeSeconds'], 3),
                startSeconds=round(offset + notify['localTimeSeconds'], 3),
                sourceAngleDegrees=angle, sourcePositionCm=local_position,
                sourceEffectId=effect_id, payloadSha256=notify['serializedPayload']['sha256']))
        offset = round(offset + duration, 3)
    assert len(launches) == 24
    for index in (1, 4):
        rows = [row for row in launches if row['stageIndex'] == index]
        assert [row['sourceAngleDegrees'] for row in rows] == [90, 210, 330, 120, 240, 0, 150, 270, 30, 180, 300, 60]
        assert [row['localTimeSeconds'] for row in rows] == [.7] * 3 + [1] * 3 + [1.3] * 3 + [1.6] * 3
    return dict(actionId=ACTION_ID, projectileId=PROJECTILE_ID, stages=stages,
        launches=launches, projectileParticle=particle, missileFields=fields,
        projectileSkillEffect=table, sourceInputs=[digest(p) for p in (source.ACTION, PROJECTILE, DATABASE, CLIP_REFERENCE)],
        skippedCallbackSystems=[p['value'].split("'")[1].lower() for p in particles[1:]],
        presentationBoundary='Two serialized 12-card volleys; server hit callbacks and target-dependent caster movement are not synthesized.',
        motionLayoutBasis='EFSequenceSummonsProjectile reflected ResScale/CollisionSize/Height/Speed/MaxSpeed/Lifetime/MaxDistance; build_kouku_backstep_electric_group.py',
        motionBasis='Initial Speed 8m/s, MaxSpeed 15m/s, Lifetime 5s, MaxDistance 15m. Preview keeps initial speed and stops at 15m; native acceleration is not inferred.')


def native_stage(index, inputs):
    path = STAGE_ROOT / f'effect.kouku.action.mn_rpct_05.{ACTION_ID}.stage{index:03d}.effect.json'
    inputs.append(digest(path))
    document = source.read(path)
    libraries = {}
    for element in document['elements']:
        emitter = element['sourcePresentation']['sourceObjectPath']
        system = emitter.rsplit('.', 1)[0]
        if system not in libraries:
            library_path = AUTHORED / ('effect.kouku.source.' + system + '.effect.json')
            inputs.append(digest(library_path))
            libraries[system] = {e['sourcePresentation']['sourceObjectPath']: e for e in source.read(library_path)['elements']}
        native = libraries[system][emitter]
        assert native['material']['sourceMaterialPath'] == element['material']['sourceMaterialPath']
        assert native['material']['sourceProfile']['enabled']
        element['material'] = copy.deepcopy(native['material'])
        element['resources'] = copy.deepcopy(native['resources'])
        if 'runtimeCarrier' in native:
            element['runtimeCarrier'] = copy.deepcopy(native['runtimeCarrier'])
        project_mesh_rotation(element)
    return document


def append_elements(document, original, occurrence, delay):
    group_id = 'kouku.spin.group.' + hashlib.sha256(occurrence.encode()).hexdigest()[:24]
    ids = {e['id']: 'kouku.spin.' + hashlib.sha256((occurrence + '|' + e['id']).encode()).hexdigest()[:24]
        for e in original['elements']}
    elements = remap(copy.deepcopy(original['elements']), ids)
    for element in elements:
        element['groupId'] = group_id
        element['detail']['timing']['startDelaySeconds'] += delay
        element['sourcePresentation']['sourceTimeSeconds'] += delay
        if 'particleSystemOccurrenceId' in element['sourceRecipe']:
            element['sourceRecipe']['particleSystemOccurrenceId'] = occurrence
        document['elements'].append(element)
    return elements


def key(time, values):
    return dict(timeSeconds=time, value=values, arriveTangent=[0, 0, 0],
        leaveTangent=[0, 0, 0], interpolation='linear')


def append_flight(document, library, launch, motion):
    occurrence = launch['notifyId'] + '/projectile-421981901'
    elements = append_elements(document, library, occurrence, launch['startSeconds'])
    speed, distance = motion['speedCmPerSecond'], motion['maxDistanceCm']
    active = min(motion['maxLifeSeconds'], distance / speed)
    angle = math.radians(launch['sourceAngleDegrees'])
    x, y, z = launch['sourcePositionCm']
    origin = [x * math.cos(angle) - y * math.sin(angle),
        x * math.sin(angle) + y * math.cos(angle), z]
    finish = [origin[0] + speed * active * math.cos(angle),
        origin[1] + speed * active * math.sin(angle), z]
    for element in elements:
        project_mesh_rotation(element)
        element['detail']['timing']['lifeTimeSeconds'] = active
        if element['sourceRecipe']['rendererShape'] == 'ribbon':
            trail = element['detail']['trail']
            # TypeData's 500 is a capacity, not the point count. The existing
            # fixed-step carrier emits at most once per sample interval and
            # expires points at the source .4s lifetime. Reserve that complete
            # live history plus two endpoints; leave original TypeData intact.
            trail['maxPoints'] = min(trail['maxPoints'],
                math.ceil(trail['pointLifeTimeSeconds'] / trail['sampleIntervalSeconds']) + 2)
        element['actionCueAttachment'].update(enabled=True, follow=False,
            sourceAnchorSlotId='root', runtimeAnchorSlotId='root', runtimeBoneName='',
            socketLocalTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1]),
            snapshotRootSourceBasisYawDegrees=-90)
        # Translation is evaluated on the emitter at both spawn and update.
        # World-space smoke/ribbon preserve their own birth transform while
        # local-space native cards follow the missile and keep Cascade spin.
        element['sourceTransformTrack'] = dict(sourceOccurrenceId=occurrence,
            sourceTimeOriginSeconds=0, previewOriginUE3Cm=[0, 0, 0],
            nodes=[dict(sourceObjectPath=f'projectile/{PROJECTILE_ID}/{launch["notifyId"]}',
                frame='WORLD', initialPositionUE3Cm=origin,
                initialEulerDegrees=[0, 0, launch['sourceAngleDegrees']], scaleUE3=[1, 1, 1],
                positionKeys=[key(launch['startSeconds'], origin), key(launch['startSeconds'] + active, finish)],
                eulerKeys=[])])
        emitter = element['sourcePresentation']['sourceObjectPath']
        element['sourceNode'] = occurrence + '|' + emitter
        element['sourcePresentation'].update(sourceActionCueId=launch['notifyId'],
            sourceEventId=launch['notifyId'], sourceTimeSeconds=launch['startSeconds'])
    return dict(notifyId=launch['notifyId'], startSeconds=launch['startSeconds'],
        activeSeconds=active, distanceMeters=distance * .01, sourceOriginCm=origin,
        sourceEndCm=finish, elementIds=[e['id'] for e in elements])


def validate(document):
    elements = document['elements']
    assert len(elements) == 224
    assert len({e['id'] for e in elements}) == len(elements)
    assert all(re.fullmatch(r'[A-Za-z0-9_.-]{1,128}', e['id']) and
        re.fullmatch(r'[A-Za-z0-9_.-]{1,128}', e['groupId']) and
        len(e['sourceNode'].encode('utf-8')) <= 256 for e in elements)
    paths = set()
    def visit(value):
        if isinstance(value, dict):
            for item in value.values(): visit(item)
        elif isinstance(value, list):
            for item in value: visit(item)
        elif isinstance(value, str) and value.startswith('Effect/'):
            assert ':' not in value and '..' not in Path(value).parts
            paths.add(value)
    visit(document)
    missing = [p for p in paths if not (ROOT / 'Client/Bin/Resources' / p).is_file()]
    assert not missing, missing
    cards = [e for e in elements if e.get('sourceTransformTrack') and
        any(r['assetId'].endswith('/fm_l_rpct_rcard_01_sm.wmodel') for r in e['resources'])]
    assert len(cards) == 24
    for card in cards:
        assert card['detail']['mesh']['sourceTypeDataRotationDegrees'] == [0, 0, 90]
        module = next(m for m in card['sourceRecipe']['modules'] if m['className'] == 'particlemodulemeshrotationrate')
        assert module['distributions'][0]['lookupTable'][2:5] == [0, 0, 3]
        assert card['material']['sourceProfile']['enabled']
    json.dumps(document, allow_nan=False)
    return dict(elements=len(elements), flyingCards=len(cards), movingEmitters=192,
        nativeMaterials=sum(bool(e['material']['sourceProfile']['enabled']) for e in elements),
        resources=len(paths), missingResources=missing, sourceCardSpinTurnsPerSecond=3,
        totalTrailPointCapacity=sum(e['detail']['trail']['maxPoints'] for e in elements if e['kind'] == 'trail'),
        visualValidation='USER_PENDING')


def card_geometry(document):
    sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
    from cook_wmodel_geometry_contract import parse_geometry_wmodel
    card = next(e for e in document['elements'] if e.get('sourceTransformTrack') and
        any(r['assetId'].endswith('/fm_l_rpct_rcard_01_sm.wmodel') for r in e['resources']))
    asset = next(r['assetId'] for r in card['resources'] if r['slotId'] == 'meshModel')
    path = ROOT / 'Client/Bin/Resources' / asset
    geometry = parse_geometry_wmodel(path.read_bytes())
    vertices = [v['values'] for part in geometry['submeshes'] for v in part['vertices']]
    bounds = [[min(v[i] for v in vertices), max(v[i] for v in vertices)] for i in range(3)]
    size = next(d for m in card['sourceRecipe']['modules'] if m['className'] == 'particlemodulesize'
        for d in m['distributions'] if d['propertyPath'] == 'startsize')['lookupTable'][2:5]
    return dict(mesh=digest(path), sourceMesh='fx_sm_00.fm_l_rpct_rcard_01_sm',
        vertexCount=len(vertices), bounds=bounds, geometryPreScale=geometry['geometryPreScale'],
        sourceStartSize=size, measuredExtentsMeters=[(b[1] - b[0]) * geometry['geometryPreScale'] * s for b, s in zip(bounds, size)],
        sourceMaterial=card['material']['sourceMaterialPath'],
        runtimeShaderProfileId=card['material']['sourceProfile'].get('runtimeShaderProfileId'),
        nativeTextures=card['material']['sourceProfile'].get('textures', []),
        sourceTypeDataRotationDegrees=card['detail']['mesh']['sourceTypeDataRotationDegrees'],
        referenceComparison='User image has a dark rectangular card with ornate bright border; native mesh/material reused; user visual judgment pending.')


def build(evidence):
    evidence = evidence.resolve()
    assert evidence == (ROOT / 'out/KoukuSpinningCards20260913').resolve(), 'Keep this task output isolated.'
    evidence.mkdir(parents=True, exist_ok=True)
    contract = source_contract()
    inputs = contract['sourceInputs']
    library_path = AUTHORED / ('effect.kouku.source.' + SYSTEM + '.effect.json')
    library = source.read(library_path)
    inputs.append(digest(library_path))
    assert len(library['elements']) == 8
    document = dict(schema='lostark.effect-authoring', version=15,
        effectAssetId=ASSET, displayName='세이튼_빙글빙글돌며카드던지기',
        particleSystem=dict(uniformScaleMultiplier=1, yawOffsetDegrees=0, directionYawDegrees=0, initialSpeedMultiplier=1),
        modelCues=[], elements=[], runtimeExtensions=dict(formatVersion=1, bakedEdgeHistories=[]),
        sourceModelPreview=dict(gateId='GATE1', actorProfileId='MN_RPCT_05',
            targetBossPlacementId='boss.kakulsaydon.g1.saydon', animations=[]))
    for stage in contract['stages']:
        document['sourceModelPreview']['animations'].append(dict(runtimeClip=stage['runtimeClip'],
            startOffsetMs=round(stage['startSeconds'] * 1000), sourceStartMs=0,
            playMs=round(stage['durationSeconds'] * 1000), playRate=1, endPolicy=stage['endPolicy']))
        if stage['stageIndex'] % 3 != 2:
            original = native_stage(stage['stageIndex'], inputs)
            append_elements(document, original, f'kouku.spin.action.{stage["stageIndex"]}', stage['startSeconds'])
    flights = [append_flight(document, library, launch, contract['missileFields']) for launch in contract['launches']]
    validation = validate(document)
    candidate = evidence / 'candidate' / (ASSET + '.effect.json')
    source.write(candidate, document)
    source.write(evidence / 'source_contract.json', contract)
    source.write(evidence / 'flight_tracks.json', flights)
    source.write(evidence / 'validation.json', validation)
    source.write(evidence / 'card_geometry.json', card_geometry(document))
    duration = max(max(e['detail']['timing']['startDelaySeconds'] + e['detail']['timing']['lifeTimeSeconds'] +
        e['detail']['timing']['afterImageSeconds'] + max(e['detail']['particle']['lifeTimeSeconds'])
        for e in document['elements']), 20.46)
    source.write(evidence / 'installation.json', dict(installed=False, documents=[dict(
        effectAssetId=ASSET, displayName=document['displayName'],
        path='Data/Effects/Authored/' + candidate.name, durationMs=math.ceil(duration * 1000),
        elementCount=len(document['elements']), defaultAnchorKind='BOSS', followBoss=True,
        categoryPath=['KoukuSaydon', '1관문', '패턴', '세이튼', '세이튼_빙글빙글돌며카드던지기'])],
        sourceActionId=ACTION_ID, sourceStageIndices=list(range(6)), projectileCount=24,
        liveCompositionChanged=False, sourceContract='source_contract.json', validation=validation))
    print(json.dumps(dict(candidate=candidate.as_posix(), **validation), ensure_ascii=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuSpinningCards20260913')
    args = parser.parse_args()
    sys.stdout.reconfigure(encoding='utf-8')
    build(args.evidence_root)
