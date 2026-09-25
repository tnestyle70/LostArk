"""Restore an Esther cameo from its summons sequence (default: Bahuntur, 532200)."""
from pathlib import Path
import argparse
import base64
import copy
import re
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
import build_esther_inanna_source_effects as drv
import build_esther_ninave_source_effects as ninave
import build_kouku_gate1_full_restore as source
import build_vehicle_skill_effects as vehicle

EVIDENCE = ROOT / 'out/BahunturFX20260924'
ARCHETYPE = 'NPC_59060'
ASSET_PREFIX = 'effect.esther.bahuntur.cameo.'
CLIP = 'npc_sk_breathofarcturus'
CLIP_SECONDS = 4.1
CAMEO_MESH = 'SK_BreathOfArcturus'
SOURCE_MESH = ('mn_yobr_00', 'mesh.mn_yobr_00_sk')
TEXTURE_ROOT = 'Effect/Esther/Balthorr/FullRestore/Textures'
PROJECTILE = ninave.PROJECTILE.with_name('532200.loa')
BOW_TEMPLATE = ninave.BOW_TEMPLATE
ROOT_TEMPLATE = ninave.ROOT_TEMPLATE
NATIVE_FIRST, NATIVE_LAST = 4648, 4799
SKIPPED_SYSTEMS = ()


def sequence_actions(raw):
    actions = []
    for match in re.finditer(rb'CEFSequenceSummonsAction(\w+)\x00', raw):
        at = match.start() - 4
        if struct.unpack_from('<i', raw, at)[0] == len(match.group(0)):
            actions.append(dict(at=at, kind=match.group(1).decode('ascii'), end=match.end()))
    for number, action in enumerate(actions):
        action['until'] = actions[number + 1]['at'] if number + 1 < len(actions) else len(raw)
    return actions


def sequence_timeline(raw):
    actions = sequence_actions(raw)
    events, i = [], 0
    while i < len(actions):
        action = actions[i]
        if action['kind'] == 'Timer':
            count = struct.unpack_from('<i', raw, action['end'])[0]
            assert 0 <= count <= 8, (action['at'], count)
            if not count:
                i += 1
                continue
            children = actions[i + 1:i + 1 + count]
            delay = struct.unpack_from('<f', raw, children[-1]['until'] - 16)[0]
            assert 0.0 <= delay < 30.0, (action['at'], delay)
            events += [(delay, child) for child in children]
            i += 1 + count
        else:
            events.append((0.0, action))
            i += 1
    return events


def mesh_transform(body):
    """SkeletalMeshFX spawn: the last three equal scale floats, position 24 bytes before them."""
    for at in range(len(body) - 12, 24, -1):
        scale = struct.unpack_from('<3f', body, at)
        if scale[0] == scale[1] == scale[2] and 0.05 <= scale[0] <= 20.0:
            return list(struct.unpack_from('<3f', body, at - 24)), scale[0]
    raise AssertionError('SkeletalMeshFX transform not found')


def to_metres(position_cm, origin_cm):
    x, y, z = (p - o for p, o in zip(position_cm, origin_cm))
    return [x / 100.0, z / 100.0, y / 100.0]


def sequence_rows(raw):
    world, bone, props, cameo = [], [], [], None
    for time, action in sequence_timeline(raw):
        body = raw[action['at']:action['until']]
        if action['kind'] == 'CreateFX':
            for match in re.finditer(rb"ParticleSystem'([^']+)'\x00", body):
                end = action['at'] + match.end()
                world.append(dict(time=round(time, 4), system=match.group(1).decode('ascii'),
                                  sourcePositionCm=list(struct.unpack_from('<3f', raw, end + 76)),
                                  scale=list(struct.unpack_from('<3f', raw, end + 136))))
        elif action['kind'] == 'SkeletalMeshFX':
            position, scale = mesh_transform(body)
            payload = dict(notifyId='sequence-skeletalmesh', serializedPayload=dict(data=base64.b64encode(body).decode('ascii')))
            particles = vehicle.skeletal_mesh_particles(payload, '')
            if re.search(rb'\x00' + CAMEO_MESH.encode('ascii') + rb'\x00', body):
                assert cameo is None
                cameo = dict(time=round(time, 4), positionCm=position)
                bone += [dict(time=round(e['startSeconds'], 4), system=e['system'], durationSeconds=e['durationSeconds'],
                              block=e['block']) for e in particles]
            else:
                mesh = re.search(rb"SkeletalMesh'([^']+)'", body).group(1).decode('ascii')
                props.append(dict(mesh=mesh, time=round(time, 4), positionCm=position, scale=scale))
                world += [dict(time=round(time + e['startSeconds'], 4), system=e['system'], sourcePositionCm=position,
                               scale=[scale] * 3, carrierMesh=mesh) for e in particles]
    assert cameo is not None
    for row in world:
        row['sequenceTime'] = row['time']
        row['time'] = round(max(0.0, row['time'] - cameo['time']), 4)
        row['positionMeters'] = to_metres(row['sourcePositionCm'], cameo['positionCm'])
    return world, bone, dict(cameo, props=props)


def configure():
    drv.PROFILE = 'cameo'
    drv.ARCHETYPE = ARCHETYPE
    drv.ASSET_PREFIX = ASSET_PREFIX
    drv.ACTION_ID = BOW_TEMPLATE[1]
    drv.ACTION_PROFILE = BOW_TEMPLATE[0]
    drv.ACTION_LOA = drv.ACTION_LOA.with_name(BOW_TEMPLATE[0] + '.loa')
    drv.CLIPS = {CLIP: (0, CAMEO_MESH)}
    drv.SOURCE_MESH = SOURCE_MESH
    drv.NPC_MODEL = f'Character/NPC/Npc_{ARCHETYPE[4:]}/Npc_{ARCHETYPE[4:]}.wmodel'
    drv.TEXTURE_ROOT = TEXTURE_ROOT
    drv.MESH_ROOTS = ('Effect/Esther/Ninave/Meshes', 'Effect/Esther/Inanna/Meshes', 'Effect/Esther/Thirain/Meshes',
                      'Effect/Esther/Wei/Meshes', 'Effect/KoukuSaydon/FullRestore/Meshes', 'Effect/Esther/Balthorr/Meshes')
    drv.CUE_DOCUMENT = ROOT / 'Data/Effects/NpcActionCues' / (drv.ARCHETYPE + '.npcactioncues.json')
    drv.SILIAN_REVIEWED = ROOT / 'out/NinaveFX20260922/material/reviewed'
    drv.CAMEO_RECENTRED = ()
    drv.cameo_actions = cameo_actions
    drv.project_clip = scaled_project_clip


def scaled_project_clip(evidence, clip, installed, deferred_programs, project_clip=drv.project_clip):
    document, row = project_clip(evidence, clip, installed, deferred_programs)
    scales = {r['notifyId']: r['scale'] for r in source.read(evidence / 'synthetic_cameo_notifies.json')['rows'] if 'scale' in r}
    for element in document['elements']:
        scale = scales.get(element['sourcePresentation']['sourceEventId'])
        if scale:
            transform = element['detail']['transform']
            transform['scale'] = [v * s for v, s in zip(transform['scale'], scale)]
    source.write(evidence / 'projection' / clip / 'candidate' / (drv.asset_id(clip) + '.effect.json'), document)
    return document, row


def cameo_actions(evidence):
    document, action, stage, bow = ninave.template_notify(evidence, BOW_TEMPLATE)
    _, _, _, root = ninave.template_notify(evidence, ROOT_TEMPLATE)
    world, bone, cameo = sequence_rows(PROJECTILE.read_bytes())
    marker = bytes([16, 0, 0, 0]) + b'CEFParticleData' + bytes([0])
    bow_raw = base64.b64decode(bow['serializedPayload']['data'])
    header = bow_raw[:bow_raw.index(marker)]
    notifies, rows = [], []
    for number, entry in enumerate(bone):
        assert entry['block'].startswith(marker), entry['system']
        row = copy.deepcopy(bow)
        row.update(notifyId=f"{bow['notifyId']}-bone{number}", localTimeSeconds=entry['time'],
            sourceEndSeconds=entry['time'] + entry['durationSeconds'], durationSeconds=entry['durationSeconds'],
            assetReferences=[dict(className='ParticleSystem', objectPath=entry['system'])],
            serializedLabels=[f'bone{number}', 'FX-' + ARCHETYPE, 'CEFParticleData', f"ParticleSystem'{entry['system']}'"],
            synthetic=dict(kind='summons sequence SkeletalMeshFX.CEFAN_Particle', clonedFrom=bow['notifyId'],
                projectile=PROJECTILE.name, anchor='bone', recentred=False))
        payload = header + entry['block']
        row['serializedPayload'] = dict(bow['serializedPayload'], data=base64.b64encode(payload).decode('ascii'), byteSize=len(payload), sha256='synthetic')
        notifies.append(row)
        rows.append(dict(notifyId=row['notifyId'], system=entry['system'], anchor='bone', startSeconds=entry['time'],
                         durationSeconds=entry['durationSeconds']))
    old_system = root['assetReferences'][0]['objectPath']
    skipped = [entry for entry in world if entry['system'].lower() in SKIPPED_SYSTEMS]
    for number, entry in enumerate(world):
        if entry in skipped:
            continue
        raw_row = drv.replace_particle_reference(base64.b64decode(root['serializedPayload']['data']), old_system, entry['system'])
        row = copy.deepcopy(root)
        row.update(notifyId=f"{root['notifyId']}-world{number}", localTimeSeconds=entry['time'], sourceEndSeconds=entry['time'],
            durationSeconds=0.0, assetReferences=[dict(className='ParticleSystem', objectPath=entry['system'])],
            serializedLabels=[f'world{number}', 'FX-' + ARCHETYPE, 'CEFParticleData', f"ParticleSystem'{entry['system']}'"],
            synthetic=dict(kind='summons sequence ' + ('SkeletalMeshFX particle at the mesh spawn' if entry.get('carrierMesh') else 'CreateFX'),
                clonedFrom=root['notifyId'], projectile=PROJECTILE.name, anchor='root', recentred=True, carrierMesh=entry.get('carrierMesh'),
                positionMeters=entry['positionMeters'], sourceScale=entry['scale'], sequenceTime=entry['sequenceTime']))
        row['serializedPayload'] = dict(root['serializedPayload'], data=base64.b64encode(raw_row).decode('ascii'), byteSize=len(raw_row), sha256='synthetic')
        notifies.append(row)
        rows.append(dict(notifyId=row['notifyId'], system=entry['system'], anchor='root', startSeconds=entry['time'],
                         sequenceTime=entry['sequenceTime'], positionMeters=entry['positionMeters'], scale=entry['scale'],
                         carrierMesh=entry.get('carrierMesh')))
    notifies.sort(key=lambda n: n['localTimeSeconds'])
    staged = dict(document, actions=[dict(action, stages=[dict(stageIndex=0, stageName=CAMEO_MESH, sourceOffset=stage['sourceOffset'],
        animationClips=[dict(clipName=CAMEO_MESH, lengthSeconds=CLIP_SECONDS, notifyId=bow['notifyId'])],
        notifies=notifies, unsupportedUnresolved=[], summary=dict(notifyCount=len(notifies)))])])
    patched = evidence / 'actions' / f'{BOW_TEMPLATE[0]}.action-effects.cameo.json'
    source.write(patched, staged)
    source.write(evidence / 'synthetic_cameo_notifies.json', dict(projectile=str(PROJECTILE), cameo=cameo, rows=rows, skipped=skipped))
    return patched


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--evidence-root', type=Path, default=EVIDENCE)
    parser.add_argument('--acquire', action='store_true')
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--native-first', type=int, default=NATIVE_FIRST)
    parser.add_argument('--native-last', type=int, default=NATIVE_LAST)
    parser.add_argument('--install-native', action='store_true')
    parser.add_argument('--project', action='store_true')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    configure()
    evidence = args.evidence_root.resolve()
    if args.acquire:
        drv.acquire(evidence)
    if args.native:
        drv.native(evidence, args.native_first, args.native_last)
    if args.install_native:
        vehicle.VEHICLE_NATIVE_FIRST, vehicle.VEHICLE_NATIVE_LAST = min(args.native_first, 3712), args.native_last
        vehicle.install_native(evidence)
    if args.project:
        drv.project(evidence, args.install)


if __name__ == '__main__':
    main()
