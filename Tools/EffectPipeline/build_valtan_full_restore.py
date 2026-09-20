"""Build Valtan source documents through the shared full-restore pipeline.

The library owns original first-LOD Cascade/native material data. This adapter
owns Valtan's installed skeleton and explicit original Action stage selection.
Existing authored Effects are never overwritten, and unresolved source notifies
remain explicit in the receipt instead of acquiring invented attachments.
"""
from pathlib import Path
import argparse
import base64
import collections
import copy
import hashlib
import json
import math
import os
import re
import sys
import struct
import tempfile
import xml.etree.ElementTree as ET

import build_kouku_action_effect_groups as action
import build_kouku_all_source_effects as library
import build_kouku_gate1_full_restore as source
import build_kouku_gate3_slam_mario_restore as exact
from extract_action_effect_notifies import read_length_prefixed_string

ROOT = source.ROOT
PROFILE = 'MN_RPBF_00'
BODY = 'Character/Valtan/MN_RPBF_01.wmodel'
ANIMATIONS = Path('Data/Effects/ValtanFullRestoreAnimations.json')


def decode_valtan_particle_notify(notify, sockets, bones):
    """Resolve the original ghost-only particle override before the common decoder.

    Action 15's Respawn_1 has a null base ParticleSystem. Its single typed
    CEFParticleDataModifier owns the MN_RPBF_02 system and Color parameter;
    the transform remains on the base CEFParticleData. Searching for the first
    ParticleSystem and treating its following bytes as a transform reads the
    modifier parameter table instead. Admit this measured source layout only.
    """
    if notify['notifyId'] != 'action-15/stage-002/notify-002':
        return action.decode_notify(notify, sockets, bones)
    payload = notify['serializedPayload']
    raw = base64.b64decode(payload['data'], validate=True)
    expected = 'af28c9ef1f68d77883cd9244b3c92341c9a9ca9bbbd04cd60f3334af4e9959eb'
    assert len(raw) == payload['byteSize'] == 739
    assert hashlib.sha256(raw).hexdigest() == payload['sha256'] == expected
    assert raw[240:256] == b'CEFParticleData\0'
    assert struct.unpack_from('<i', raw, 268)[0] == 0
    assert struct.unpack_from('<i', raw, 420)[0] == 0
    assert struct.unpack_from('<i', raw, 440)[0] == 1
    assert raw[448:472] == b'CEFParticleDataModifier\0'
    assert raw[476:507] == b'EFDLChar_MN_RPBF_02.MN_RPBF_02\0'
    assert raw[511:568] == b"ParticleSystem'FX_MN_RPBF_00_N.Par_N_RPBF_Spawn_Cast_01'\0"
    assert struct.unpack_from('<2i', raw, 568) == (1, 1)
    # Flatten only the selected model's PS and parameter table. The common
    # decoder still owns the complete base transform and parameter semantics.
    flattened = raw[:268] + raw[507:568] + raw[272:420] + raw[572:655]
    adapted = copy.deepcopy(notify)
    adapted['serializedPayload'] = dict(payload,
        data=base64.b64encode(flattened).decode('ascii'))
    cue = action.decode_notify(adapted, sockets, bones)
    assert cue['sourceTransformByteOffset'] == 389
    assert cue['attachment']['mode'] == 'SNAPSHOT_ROOT'
    assert cue['localTransform'] == dict(sourcePositionUeUnits=[0.0, 0.0, 0.0],
        position=[0.0, 0.0, 0.0], rotationDegrees=[0.0, 0.0, 0.0], scale=[1.0, 1.0, 1.0])
    assert len(cue['parameterOverrides']) == 1
    parameter = cue['parameterOverrides'][0]
    assert parameter['name'] == 'Color' and parameter['vectorValue'] == [1.0, 1.0, 1.0]
    cue['sourceTransformByteOffset'] = 332
    cue['sourceParameterCountByteOffset'] = 572
    parameter.update(sourceRecordByteOffset=576, sourceValueByteOffset=598)
    cue['sourceModelModifier'] = dict(model='EFDLChar_MN_RPBF_02.MN_RPBF_02',
        sourceClassByteOffset=448, sourceParticleSystemByteOffset=511,
        sourcePayloadSha256=expected)
    return cue


def encode_json(value):
    return (json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8')


def original_animation_loop(clip, notify):
    """Read the serialized Anim repeat boolean, independently of the clip's name or length."""
    assert notify['sourceType'] == 'Anim' and notify['notifyId'] == clip['notifyId']
    payload = notify['serializedPayload']
    data = base64.b64decode(payload['data'], validate=True)
    assert len(data) == payload['byteSize']
    assert hashlib.sha256(data).hexdigest() == payload['sha256']
    assert data.startswith(b'CEFActionNotify_Anim\0')
    name, cursor = read_length_prefixed_string(data, 78)
    assert name == clip['clipName']
    # After the clip: two indexed optional names, two scalar words, playback
    # rate and blend-in/out floats, then the serialized repeat flag. Variable
    # strings are walked, so longer clip/socket names cannot move the flag.
    for _ in range(2):
        optional_name = read_length_prefixed_string(data, cursor + 4)
        assert optional_name is not None, 'Unsupported original Anim optional-name layout'
        _, cursor = optional_name
    cursor += 8
    rate, blend_in, blend_out = struct.unpack_from('<fff', data, cursor)
    assert all(math.isfinite(value) for value in (rate, blend_in, blend_out))
    assert math.isclose(rate, 1.0), 'Original Anim playRate needs explicit metadata support'
    assert blend_in >= 0 and blend_out >= 0
    flag = struct.unpack_from('<I', data, cursor + 12)[0]
    assert flag in (0, 1), 'Unsupported original Anim repeat flag'
    return bool(flag)


def source_stage_preview_timing(stage, clip):
    """Separate original animation span from finite source-stage preview time."""
    controls = [n for n in stage['notifies'] if n['sourceType'].startswith('MonsterMoveNextStage')]
    for notify in controls:
        value = notify['localTimeSeconds']
        assert isinstance(value, (int, float)) and not isinstance(value, bool)
        assert math.isfinite(value) and value > 0, 'Invalid source stage transition time'
    unconditional = [n for n in controls if n['sourceType'] == 'MonsterMoveNextStage']
    if unconditional:
        milliseconds = round(min(n['localTimeSeconds'] for n in unconditional) * 1000)
        basis = 'SOURCE_UNCONDITIONAL_STAGE_TRANSITION'
    elif controls:
        # A condition is not a proven transition. This editor-only budget covers
        # its source check while leaving Server authority and natural FX tails alone.
        milliseconds = max(round(clip['lengthSeconds'] * 1000),
                           round(max(n['localTimeSeconds'] for n in controls) * 1000))
        basis = 'PREVIEW_COVERS_SOURCE_CONDITIONAL_CHECK'
    else:
        milliseconds = round(clip['lengthSeconds'] * 1000)
        basis = 'SOURCE_ANIMATION_WINDOW'
    assert 0 < milliseconds <= 600000, 'Source preview wall time out of range'
    return dict(previewWallMs=milliseconds, previewWallBasis=basis)


def animation_receipt_with_loops(stage_receipt, source_actions):
    stages = {(action['actionId'], stage['stageIndex']): stage
              for action in source_actions['actions'] for stage in action['stages']}
    result = copy.deepcopy(stage_receipt)
    for row in result:
        stage = stages[(row['sourceActionId'], row['sourceStageIndex'])]
        notifies = {notify['notifyId']: notify for notify in stage['notifies']}
        source_clips = {clip['notifyId']: clip for clip in stage['animationClips']}
        for clip in row['originalAnimationClips']:
            exact_clip = source_clips[clip['notifyId']]
            assert clip['clipName'] == exact_clip['clipName'] and clip['lengthSeconds'] == exact_clip['lengthSeconds'], \
                'Original clip identity or stage wall time changed since projection'
            clip['loop'] = original_animation_loop(clip, notifies[clip['notifyId']])
            clip.update(source_stage_preview_timing(stage, clip))
    return result


def animation_metadata_write(stage_receipt, asset_ids):
    """Join exact installed stage IDs to their measured original clip, never a pattern alias."""
    path = ROOT / ANIMATIONS
    before = path.read_bytes() if path.exists() else None
    metadata = json.loads(before) if before is not None else dict(
        schema='lostark.valtan-full-restore-animations', formatVersion=1,
        bossArchetypeId='BOSS_VALTAN', effects=[])
    assert metadata['schema'] == 'lostark.valtan-full-restore-animations'
    assert metadata['formatVersion'] == 1 and metadata['bossArchetypeId'] == 'BOSS_VALTAN'
    existing = {row['effectAssetId']: row for row in metadata['effects']}
    assert len(existing) == len(metadata['effects']), 'Duplicate existing animation mapping'
    receipts = {row['effectAssetId']: row for row in stage_receipt}
    assert len(receipts) == len(stage_receipt), 'Duplicate source stage receipt'
    for asset in sorted(asset_ids):
        match = re.fullmatch(r'effect\.valtan\.action\.([0-9]+)\.stage([0-9]{3})\.full\.restore', asset)
        if match is None:
            continue
        row = receipts.get(asset)
        assert row is not None, f'Missing original animation receipt: {asset}'
        action_id, stage_index = map(int, match.groups())
        assert row['sourceActionId'] == action_id and row['sourceStageIndex'] == stage_index
        clips = row['originalAnimationClips']
        assert len(clips) == 1, f'Expected one original animation clip: {asset}: {clips}'
        clip = clips[0]
        name = clip['clipName'].lower()
        assert name and re.fullmatch(r'[a-z0-9_-]+', name), f'Invalid original clip: {asset}'
        if not name.startswith('mesh_'):
            name = 'mesh_' + name
        seconds = clip['lengthSeconds']
        assert isinstance(seconds, (int, float)) and not isinstance(seconds, bool)
        assert math.isfinite(seconds) and seconds > 0, f'Invalid original clip length: {asset}'
        milliseconds = round(seconds * 1000)
        assert 0 < milliseconds <= 600000, f'Original clip length out of range: {asset}'
        assert type(clip.get('loop')) is bool, f'Missing decoded original Anim repeat flag: {asset}'
        preview_ms = clip.get('previewWallMs')
        preview_basis = clip.get('previewWallBasis')
        assert type(preview_ms) is int and 0 < preview_ms <= 600000, f'Missing source preview wall time: {asset}'
        assert preview_basis in ('SOURCE_UNCONDITIONAL_STAGE_TRANSITION',
            'PREVIEW_COVERS_SOURCE_CONDITIONAL_CHECK', 'SOURCE_ANIMATION_WINDOW'), f'Invalid preview wall basis: {asset}'
        addition = dict(effectAssetId=asset, sourceActionId=action_id, sourceStageIndex=stage_index,
            animationClips=[dict(clipName=name, playMs=milliseconds, loop=clip['loop'],
                previewWallMs=preview_ms, previewWallBasis=preview_basis)])
        previous = copy.deepcopy(existing.get(asset))
        if previous is not None:
            # Upgrade only a missing repeat field; every prior authored value
            # still has to match the exact source entry before replacement.
            previous_clips = previous.get('animationClips', [])
            if len(previous_clips) == 1 and 'loop' not in previous_clips[0]:
                previous_clips[0]['loop'] = clip['loop']
            if len(previous_clips) == 1:
                for field in ('previewWallMs', 'previewWallBasis'):
                    if field not in previous_clips[0]:
                        previous_clips[0][field] = clip[field]
        assert previous is None or previous == addition, f'Preserve existing animation mapping: {asset}'
        existing[asset] = addition
    metadata['effects'] = [existing[key] for key in sorted(existing)]
    return path, before, encode_json(metadata)


def project_writes(asset_ids, include_animations):
    writes = []
    includes = ['..\\..\\Data\\Effects\\Authored\\' + asset + '.effect.json' for asset in sorted(asset_ids)]
    if include_animations:
        includes.append('..\\..\\Data\\Effects\\ValtanFullRestoreAnimations.json')
    for suffix in ('', '.filters'):
        path = ROOT / ('Client/Default/Client.vcxproj' + suffix)
        before = path.read_bytes()
        text = before.decode('utf8')
        newline = '\r\n' if '\r\n' in text else '\n'
        additions = [f'    <None Include="{include}"><Filter>96.DataFiles</Filter></None>' if suffix
                     else f'    <None Include="{include}" />'
                     for include in includes if f'Include="{include}"' not in text]
        if additions:
            prefix, marker, tail = text.rpartition('</Project>')
            assert marker
            text = prefix + '  <ItemGroup>' + newline + newline.join(additions) + newline + '  </ItemGroup>' + newline + marker + tail
        ET.fromstring(text)
        writes.append((path, before, text.encode('utf8')))
    return writes


def commit_writes(writes):
    """Stage the complete install before replacement and restore exact prior bytes on failure."""
    staged, committed = [], []
    try:
        for path, before, after in writes:
            assert (path.read_bytes() if path.exists() else None) == before, f'Concurrent edit: {path}'
            if before == after:
                continue
            path.parent.mkdir(parents=True, exist_ok=True)
            fd, name = tempfile.mkstemp(prefix=path.name + '.', suffix='.staged', dir=path.parent)
            temporary = Path(name)
            staged.append((path, before, after, temporary))
            with os.fdopen(fd, 'wb') as output:
                output.write(after)
        for path, before, after, temporary in staged:
            assert (path.read_bytes() if path.exists() else None) == before, f'Concurrent edit: {path}'
            os.replace(temporary, path)
            committed.append((path, before, after, temporary))
    except Exception:
        for path, before, after, temporary in reversed(committed):
            assert path.read_bytes() == after, f'Concurrent edit preserved during rollback: {path}'
            if before is None:
                path.unlink()
            else:
                temporary.write_bytes(before)
                os.replace(temporary, path)
        raise
    finally:
        for _, _, _, temporary in staged:
            temporary.unlink(missing_ok=True)
    return [path.relative_to(ROOT).as_posix() for path, _, _, _ in committed]


def install_animation_metadata(stage_receipt, evidence, source_actions=None):
    """Backfill source metadata for actual installed documents without rewriting an Effect."""
    assets = []
    for path in sorted((ROOT / 'Data/Effects/Authored').glob('effect.valtan.action.*.full.restore.effect.json')):
        asset = path.name.removesuffix('.effect.json')
        assert source.read(path)['effectAssetId'] == asset, f'Installed document identity mismatch: {path}'
        assets.append(asset)
    assert assets, 'No installed Valtan action-stage restore documents'
    if source_actions is not None:
        stage_receipt = animation_receipt_with_loops(stage_receipt, source_actions)
    writes = [animation_metadata_write(stage_receipt, assets)] + project_writes([], True)
    changed = commit_writes(writes)
    source.write(evidence / 'animation_metadata_installation.json', dict(changedFiles=changed,
        effectAssetIds=assets, preservedExistingAuthored=True, visualStatus='USER_PENDING'))
    return len(assets)


def model_contract(evidence, weapon_socket_contract=None):
    sys.path.insert(0, str(ROOT / 'Tools/CharacterCustomizing'))
    from align_rig_gltf import read_body_skeleton
    model = ROOT / 'Client/Bin/Resources' / BODY
    bones = dict(read_body_skeleton(model))
    sockets = exact.socket_contract('mn_rpbf_01.mesh.mn_rpbf_01_sk_loc_int')
    if weapon_socket_contract:
        weapon = source.read(weapon_socket_contract)
        installed_weapon = ROOT / 'Client/Bin/Resources' / weapon['installedWeapon']
        assert hashlib.sha256(installed_weapon.read_bytes()).hexdigest() == weapon['installedWeaponSha256']
        assert weapon['matchedVertices'] == 2237 and weapon['maximumPositionErrorMeters'] == 0
        assert weapon['weaponModelPreScale'] * weapon['bodyBoneImportScale'] == weapon['measuredNetSourceScale'] == 1
        for row in weapon['sockets']:
            assert row['runtimeBoneName'] in bones
            original = row['sourceSocket']
            assert not any(s['socketName'] == original['socketName'] for s in sockets['sockets'])
            sockets['sockets'].append(dict(socketName=original['socketName'], boneName=row['runtimeBoneName'],
                runtimeLocalTransform=row['runtimeSocketLocalTransform'],
                sourceObjectPath=original['sourceObjectPath'], sourceSerialSha256=original['sourceSerialSha256'],
                transformEvidence='ORIGINAL_WEAPON_SOCKET_BIND_POSE_AND_EXACT_INSTALLED_RIGID_GEOMETRY'))
    sockets['sockets'] += [dict(socketName='', boneName=bone) for bone in bones]
    source.write(evidence / 'installed_model_contract.json', dict(
        modelAssetId=BODY, modelSha256=hashlib.sha256(model.read_bytes()).hexdigest(),
        installedBones=bones, sourceSockets=sockets))
    return bones, sockets


def namespace(elements, histories):
    """Renamespace IDs and their actual provider references together."""
    ids = {e['id']: e['id'].replace('kouku.', 'valtan.', 1) for e in elements}
    history_ids = {h['historyId']: h['historyId'].replace('kouku.', 'valtan.', 1) for h in histories}
    for element in elements:
        element['id'] = ids[element['id']]
        carrier = element.get('runtimeCarrier', {})
        if carrier.get('historyId') in history_ids:
            carrier['historyId'] = history_ids[carrier['historyId']]
        for module in element.get('sourceRecipe', {}).get('modules', []):
            for literal in module['literals']:
                if literal['propertyPath'] == 'runtime.providerelementid':
                    literal['value'] = ids[literal['value']]
    for history in histories:
        history['historyId'] = history_ids[history['historyId']]


def apply_verified_portal_bone_local_position(cue, notify):
    # These original zero-rotation foot/dash offsets are expressed in the
    # imported b_effectroot bone basis, not UE world axes. The actual installed
    # skeleton/clip probe proves unchanged XYZ local axes for these six notifies.
    verified = {'action-420624/stage-001/notify-001', 'action-420624/stage-001/notify-002',
                'action-420624/stage-001/notify-005', 'action-420624/stage-006/notify-007',
                'action-420624/stage-006/notify-008', 'action-420624/stage-006/notify-010'}
    if notify['notifyId'] not in verified:
        return
    attachment = cue['attachment']
    transform = cue['localTransform']
    assert attachment['mode'] == 'FOLLOW_NAMED_ANCHORS'
    assert attachment['sourceAnchorNames'] in (['FX_Att_01'], ['b_effectroot'])
    assert all(anchor['runtimeBoneName'] == 'b_effectroot' for anchor in attachment['runtimeAnchors'])
    assert transform['rotationDegrees'] == [0, 0, 0]
    assert len(set(transform['scale'])) == 1
    transform['position'] = [value * .01 for value in transform['sourcePositionUeUnits']]


def apply_verified_four_direction_rotator(cue, notify):
    """Read the qualified four-direction CEFParticleData FRotator once."""
    verified = {6, 7, 8, 9, 10, 19, 20, 22, 23, 24, 30, 31, 33, 34, 35, 40, 41, 43, 44, 45}
    prefix = 'action-420624/stage-007/notify-'
    if not notify['notifyId'].startswith(prefix) or int(notify['notifyId'][len(prefix):]) not in verified:
        return
    assert notify['sourceType'] == 'PlayParticleEffect' and cue['enabled']
    assert cue['attachment']['mode'] == 'SNAPSHOT_ROOT'
    payload = notify['serializedPayload']
    raw = base64.b64decode(payload['data'], validate=True)
    assert len(raw) == payload['byteSize'] and hashlib.sha256(raw).hexdigest() == payload['sha256']
    transform = cue['sourceTransformByteOffset']
    assert cue['sourceParameterCountByteOffset'] == transform + 88
    assert struct.unpack_from('<3f', raw, transform + 28) == (0, 0, 0)
    # Same compact layout as the source-qualified Kouku Albion/trumpet path:
    # +28 is a separate float vector, +40 is FRotator(Pitch,Yaw,Roll) int32.
    # The four source snapshots retain their authored positions independently.
    rotator = list(struct.unpack_from('<3i', raw, transform + 40))
    assert rotator[0] == rotator[2] == 0 and -65536 <= rotator[1] <= 65536
    cue['localTransform']['sourceRotationUeRotator'] = rotator
    cue['localTransform']['sourceRotationByteOffset'] = transform + 40
    cue['localTransform']['rotationDegrees'] = [rotator[0] * 360 / 65536,
        rotator[1] * 360 / 65536, -rotator[2] * 360 / 65536]


def build_stages(source_document, library_root, selections, evidence, native_patch, weapon_socket_contract=None, additional_libraries=()):
    index = action.restored_index(library_root)
    installed = source.read(library_root / 'installation.json')
    templates = {r['sourceParticleSystem']: source.read(library_root / 'candidate' / Path(r['path']).name)
                 for r in installed['documents']}
    occurrences = collections.defaultdict(list)
    for row in source.read(library_root / 'source_occurrences.json'):
        occurrences[row['sourceSystem']].append(row)
    for extra_root in additional_libraries:
        extra_index = action.restored_index(extra_root)
        for key, obj in extra_index.objects.items():
            if key in index.objects:
                assert index.objects[key].properties == obj.properties, ('Shared source closure differs', key)
            else:
                index.objects[key] = obj
                index.by_source_id[key] = obj
        for row in source.read(extra_root / 'installation.json')['documents']:
            assert row['sourceParticleSystem'] not in templates, 'Duplicate projected source system'
            templates[row['sourceParticleSystem']] = source.read(extra_root / 'candidate' / Path(row['path']).name)
        for row in source.read(extra_root / 'source_occurrences.json'):
            occurrences[row['sourceSystem']].append(row)
    native_materials = {key: p['material'] for p in source.read(native_patch)['programs'] for key in p['occurrences']}
    actions = {row['actionId']: row for row in source.read(source_document)['actions']}
    bones, sockets = model_contract(evidence, weapon_socket_contract)
    documents, receipt = {}, []
    for action_id, stage_index in sorted(set(selections)):
        stage = next(s for s in actions[action_id]['stages'] if s['stageIndex'] == stage_index)
        asset = f'effect.valtan.action.{action_id}.stage{stage_index:03d}.full.restore'
        elements, histories, restored, failures, disabled = [], [], [], [], []
        for notify in stage['notifies']:
            kind = notify['sourceType']
            if kind not in ('PlayParticleEffect', 'Trails'):
                continue
            context = dict(sourceNotify=notify['notifyId'], sourceType=kind)
            systems = [r['objectPath'].lower() for r in notify.get('assetReferences', [])
                       if r['className'].lower() == 'particlesystem']
            if not systems:
                disabled.append(dict(**context, reason='SOURCE_NULL_PARTICLE_SYSTEM'))
                continue
            try:
                if kind == 'Trails':
                    assert len(systems) == 1 and occurrences[systems[0]], 'Missing original trail PS'
                    # Installed Valtan mesh skin offsets * bind hierarchy *
                    # preScale yield the original centimetre-to-metre size.
                    # Its imported root contains x100, so preScale/0.01 alone
                    # would incorrectly shrink the source trail by x100.
                    new, extra = action.project_trail(index, occurrences[systems[0]], notify, PROFILE,
                        asset, evidence, native_materials, measured_source_scale=1.0)
                    elements += new
                    histories += extra
                    restored.append(dict(**context, sourceParticleSystems=systems, elementCount=len(new)))
                    continue
                cue = decode_valtan_particle_notify(notify, sockets, bones)
                apply_verified_portal_bone_local_position(cue, notify)
                apply_verified_four_direction_rotator(cue, notify)
                if not cue.get('enabled'):
                    disabled.append(dict(**context, reason='SOURCE_DISABLED_NOTIFY'))
                    continue
                for system in systems:
                    template = templates.get(system)
                    lights = [o for o in occurrences[system] if o['rendererShape'] == 'light']
                    if lights:
                        light = action.project_light_occurrences(index, lights, cue, notify, evidence)
                        template = copy.deepcopy(template) if template else dict(elements=[])
                        template['elements'] = [e for e in template['elements'] if e['kind'] != 'light'] + light['elements']
                    if template is None:
                        raise ValueError('Source ParticleSystem is not projected: ' + system)
                    new, parameters = action.instantiate(template, cue, notify, asset, PROFILE, index)
                    elements += new
                    restored.append(dict(**context, sourceParticleSystem=system, elementCount=len(new),
                        sourceTransform=cue['localTransform'], sourceAttachment=cue['attachment'], parameters=parameters))
                    if 'sourceModelModifier' in cue:
                        restored[-1]['sourceModelModifier'] = cue['sourceModelModifier']
            except Exception as error:
                failures.append(dict(**context, sourceParticleSystems=systems,
                                     errorType=type(error).__name__, reason=str(error)))
        namespace(elements, histories)
        document = dict(schema='lostark.effect-authoring', version=15 if histories else 13, effectAssetId=asset,
            displayName=f"Valtan {action_id} / {stage['stageName']} [{stage_index}] full restore",
            particleSystem=dict(uniformScaleMultiplier=1, yawOffsetDegrees=0,
                                directionYawDegrees=0, initialSpeedMultiplier=1),
            modelCues=[], elements=elements)
        if histories:
            document['runtimeExtensions'] = dict(formatVersion=1,
                bakedEdgeHistories=sorted(histories, key=lambda row: row['historyId']))
        if elements:
            source.write(evidence / 'candidate' / (asset + '.effect.json'), document)
            documents[asset] = document
        animation_clips = copy.deepcopy(stage['animationClips'])
        notifies = {notify['notifyId']: notify for notify in stage['notifies']}
        for clip in animation_clips:
            clip['loop'] = original_animation_loop(clip, notifies[clip['notifyId']])
            clip.update(source_stage_preview_timing(stage, clip))
        receipt.append(dict(effectAssetId=asset, sourceActionId=action_id, sourceStageIndex=stage_index,
            originalAnimationClips=animation_clips, elementCount=len(elements),
            restored=restored, disabled=disabled, sourceFailures=failures,
            completeSourceParticleNotifies=not failures, visualStatus='USER_PENDING'))
    source.write(evidence / 'stage_projection.json', receipt)
    return documents, receipt


def install_documents(documents, evidence, stage_receipt):
    """Stage additions against current bytes; preserve every authored document."""
    writes = []
    for doc in documents.values():
        path = ROOT / 'Data/Effects/Authored' / (doc['effectAssetId'] + '.effect.json')
        before = path.read_bytes() if path.exists() else None
        assert before is None or json.loads(before) == doc, f'Preserve authored edits: {path}'
        writes.append((path, before, encode_json(doc)))
    path = ROOT / 'Data/Effects/EffectCatalog.json'
    before = path.read_bytes()
    catalog = json.loads(before)
    for asset in sorted(documents):
        row = dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT',
                   authoringPath='Effects/Authored/' + asset + '.effect.json')
        matches = [r for r in catalog['effects'] if r['effectAssetId'] == asset]
        assert not matches or matches == [row]
        if not matches:
            catalog['effects'].append(row)
    writes.append((path, before, encode_json(catalog)))
    writes.append(animation_metadata_write(stage_receipt, documents))
    writes.extend(project_writes(documents, True))
    changed = commit_writes(writes)
    source.write(evidence / 'document_installation.json', dict(changedFiles=changed,
        effectAssetIds=sorted(documents), preservedExistingAuthored=True, visualStatus='USER_PENDING'))


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--source-action', type=Path)
    parser.add_argument('--library-root', type=Path)
    parser.add_argument('--native-material-patch', type=Path)
    parser.add_argument('--evidence-root', type=Path, required=True)
    parser.add_argument('--source-stage', action='append', default=[], help='Exact ACTION_ID:STAGE_INDEX')
    parser.add_argument('--current-equivalence', type=Path)
    parser.add_argument('--weapon-socket-contract', type=Path)
    parser.add_argument('--additional-library', type=Path, action='append', default=[])
    parser.add_argument('--install-complete', action='store_true')
    parser.add_argument('--install-library', action='store_true')
    parser.add_argument('--install-animation-metadata', type=Path,
                        help='Backfill installed restore clip metadata from an existing stage_projection.json')
    args = parser.parse_args()
    if args.install_animation_metadata:
        if args.install_complete or args.install_library:
            parser.error('--install-animation-metadata cannot also rebuild/install Effect documents')
        count = install_animation_metadata(source.read(args.install_animation_metadata), args.evidence_root,
            source.read(args.source_action) if args.source_action else None)
        print(json.dumps(dict(installedAnimationMappings=count, effectDocumentsRewritten=0)))
        return
    if not all((args.source_action, args.library_root, args.native_material_patch)):
        parser.error('Effect projection requires --source-action, --library-root and --native-material-patch')
    selections = [tuple(map(int, token.split(':'))) for token in args.source_stage]
    if args.current_equivalence:
        selections += [(r['canonicalSourceStage']['actionId'], r['canonicalSourceStage']['stageIndex'])
                       for r in source.read(args.current_equivalence)['resolved']]
    docs, receipt = build_stages(args.source_action, args.library_root, selections,
                                 args.evidence_root, args.native_material_patch, args.weapon_socket_contract, args.additional_library)
    installed = {}
    if args.install_complete:
        installed.update({r['effectAssetId']: docs[r['effectAssetId']] for r in receipt
                          if r['completeSourceParticleNotifies'] and r['elementCount']})
    if args.install_library:
        for root in [args.library_root] + args.additional_library:
            for row in source.read(root / 'installation.json')['documents']:
                installed[row['effectAssetId']] = source.read(root / 'candidate' / Path(row['path']).name)
    if installed:
        install_documents(installed, args.evidence_root, receipt)
    print(json.dumps(dict(projectedStages=len(docs), sourceCompleteStages=sum(r['completeSourceParticleNotifies'] for r in receipt),
                          sourceFailures=sum(len(r['sourceFailures']) for r in receipt), installedDocuments=len(installed))))


if __name__ == '__main__':
    main()
