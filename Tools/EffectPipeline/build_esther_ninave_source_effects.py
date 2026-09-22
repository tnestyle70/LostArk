"""Restore the Esther Ninave (Nineveh) Kouku cameo from its source FX systems.

The Kouku Esther cast (COMMONACTION 53203 -> CommonActionEffect 108) summons
the invisible carrier NPC 53400 (MN_ISTM_00) whose action has no visual
notify, so the visible Ninave is the SK_Parkunas cameo on MN_PPNN_00. No
Action file plays that clip with notifies; the Esther-only systems of
FX_ESTHER_PPNN_00 (spawn, esther_sk charge/arrow/impact, despawn) are placed
on a synthetic stage whose timing follows the clip and the previously
hand-authored V2 bindings. Acquisition, native materials, installation and
projection are the Inanna cameo driver's, with this module's identities.
"""
from pathlib import Path
import argparse
import base64
import copy
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
import build_esther_inanna_source_effects as drv
import build_kouku_gate1_full_restore as source
import build_vehicle_skill_effects as vehicle

EVIDENCE = ROOT / 'out/NinaveFX20260922'
CLIP = 'npc_sk_parkunas'
CLIP_SECONDS = 145.0 / 30.0
# Template notifies: a bow-socket cue (B_WP_08 -> b_wp_2) and a root cue from
# MN_PPNN_00.loa, whose payload layout the shared decoder already reads. Only
# their headers/payload layout are reused; systems, times and placements come
# from the Esther summons sequence below.
BOW_TEMPLATE = ('MN_PPNN_00', 4189601, 0, 'Par_D_PPNN_sk02_03')
ROOT_TEMPLATE = ('MN_PPNN_00', 4189601, 0, 'Par_D_PPNN_sk01_01')
# The Esther Ninave call: COMMONACTION 53203 -> EFTable_CommonActionEffect 108
# -> carrier NPC 534x0 skill 534000 -> SkillEffect Key 12 -> a
# CEFSequenceSummonsProjectileFixArea file. 531300/531320/532300 are the
# Ninave-voiced variants (same FX, 531300 adds a 3.3 s hit); 532300 is read.
PROJECTILE = Path('C:/Users/95jus/Downloads/SourceData/SourceData/LPK/data1/Common_Extra/XMLData/Projectile/532300.loa')
# Screen distortion has no restore carrier yet; the light and particle systems play.
SKIPPED_SYSTEMS = ('fx_cm_01.distortion.par_mp_concavedis_z_04',)


def sequence_timeline(raw):
    """Root actions and timer children of a CEFSequenceSummons projectile.

    A Timer's body starts with its child count; its delay float sits 16 bytes
    before the next sibling action (after the children), as the DimensionMaster
    V projectile decoder established for fixed-size timers."""
    import struct
    sys.path.insert(0, str(ROOT / 'Tools/LevelPlacementExtractor'))
    from extract_action_effect_notifies import scan_length_prefixed_strings
    strings = scan_length_prefixed_strings(raw, 0, len(raw))
    assert strings[0]['value'] == 'CEFSequenceSummonsProjectileFixArea'
    actions = [x for x in strings if x['value'].startswith('CEFSequenceSummonsAction')]
    events, i = [], 0
    while i < len(actions):
        action = actions[i]
        kind = action['value'][len('CEFSequenceSummonsAction'):]
        at = action['sourceOffset']
        if kind == 'Timer':
            count = struct.unpack_from('<i', raw, at + 4 + len(action['value']) + 1)[0]
            assert 1 <= count <= 8, (at, count)
            children = actions[i + 1:i + 1 + count]
            following = actions[i + 1 + count]['sourceOffset'] if i + 1 + count < len(actions) else len(raw) - 4
            delay = struct.unpack_from('<f', raw, following - 16)[0]
            assert 0.0 <= delay < 30.0, (at, delay)
            for child, nxt in zip(children, actions[i + 2:i + 2 + count] + [dict(sourceOffset=following)]):
                events.append((delay, child, nxt['sourceOffset']))
            i += 1 + count
        else:
            nxt = actions[i + 1]['sourceOffset'] if i + 1 < len(actions) else len(raw)
            events.append((0.0, action, nxt))
            i += 1
    return strings, events


def sequence_rows(raw):
    """(time, system, positionMeters, scale) for every CreateFX particle and the
    CEFAN_Particle entries of the summoned SK_Parkunas mesh."""
    import struct
    strings, events = sequence_timeline(raw)
    world, bone = [], []
    for time, action, until in events:
        kind = action['value'][len('CEFSequenceSummonsAction'):]
        at = action['sourceOffset']
        if kind == 'CreateFX':
            for token in strings:
                if not (at < token['sourceOffset'] < until) or not token['value'].startswith("ParticleSystem'"):
                    continue
                end = token['sourceOffset'] + 4 + len(token['value']) + 1
                x, y, z = struct.unpack_from('<3f', raw, end + 76)
                scale = struct.unpack_from('<3f', raw, end + 136)
                system = token['value'][len("ParticleSystem'"):-1]
                # UE cm (x forward, y right, z up) -> engine metres (x forward, y up, z right).
                world.append(dict(time=round(time, 4), system=system, positionMeters=[x / 100.0, z / 100.0, y / 100.0], scale=list(scale)))
        elif kind == 'SkeletalMeshFX':
            assert any(t['value'] == 'SK_Parkunas' and at < t['sourceOffset'] < until for t in strings)
            payload = dict(notifyId='sequence-skeletalmesh', serializedPayload=dict(data=base64.b64encode(raw[at:until]).decode('ascii')))
            for entry in vehicle.skeletal_mesh_particles(payload, ''):
                bone.append(dict(time=round(time + entry['startSeconds'], 4), system=entry['system'], durationSeconds=entry['durationSeconds'], block=entry['block']))
    return world, bone


def configure():
    drv.PROFILE = 'cameo'
    drv.ARCHETYPE = 'NPC_59504'
    drv.ASSET_PREFIX = 'effect.esther.ninave.cameo.'
    drv.ACTION_ID = BOW_TEMPLATE[1]
    drv.ACTION_PROFILE = BOW_TEMPLATE[0]
    drv.ACTION_LOA = drv.ACTION_LOA.with_name(BOW_TEMPLATE[0] + '.loa')
    drv.CLIPS = {CLIP: (0, 'SK_Parkunas')}
    drv.SOURCE_MESH = ('mn_ppnn_00', 'mesh.mn_ppnn_00_sk')
    drv.NPC_MODEL = 'Character/NPC/Npc_59504/Npc_59504.wmodel'
    drv.TEXTURE_ROOT = 'Effect/Esther/Ninave/FullRestore/Textures'
    drv.MESH_ROOTS = ('Effect/Esther/Ninave/Meshes', 'Effect/Esther/Inanna/Meshes', 'Effect/Esther/Thirain/Meshes',
                      'Effect/Esther/Wei/Meshes', 'Effect/KoukuSaydon/FullRestore/Meshes')
    drv.CUE_DOCUMENT = ROOT / 'Data/Effects/NpcActionCues' / (drv.ARCHETYPE + '.npcactioncues.json')
    # The Inanna reviewed contract is a superset of the Silian/vehicle ones.
    drv.SILIAN_REVIEWED = ROOT / 'out/InannaFX20260922/material/reviewed'
    drv.CAMEO_RECENTRED = ()
    drv.cameo_actions = ninave_actions


def template_notify(evidence, spec):
    profile, action_id, stage_index, system = spec
    actions = evidence / 'actions' / f'{profile}.action-effects.json'
    if not actions.is_file():
        actions = drv.extract_actions(evidence)
    document = source.read(actions)
    action = next(a for a in document['actions'] if int(a['actionId']) == action_id)
    stage = next(s for s in action['stages'] if s['stageIndex'] == stage_index)
    notify = next(n for n in stage['notifies'] if n['sourceType'] == 'PlayParticleEffect'
                  and system.lower() in n['assetReferences'][0]['objectPath'].lower())
    return document, action, stage, notify


def ninave_actions(evidence):
    document, action, stage, bow = template_notify(evidence, BOW_TEMPLATE)
    _, _, _, root = template_notify(evidence, ROOT_TEMPLATE)
    raw = PROJECTILE.read_bytes()
    world, bone = sequence_rows(raw)
    marker = bytes([16, 0, 0, 0]) + b'CEFParticleData' + bytes([0])
    bow_raw = base64.b64decode(bow['serializedPayload']['data'])
    header = bow_raw[:bow_raw.index(marker)]
    notifies, rows, skipped = [], [], []
    for number, entry in enumerate(bone):
        assert entry['block'].startswith(marker), entry['system']
        row = copy.deepcopy(bow)
        row.update(notifyId=f"{bow['notifyId']}-bone{number}", localTimeSeconds=entry['time'],
            sourceEndSeconds=entry['time'] + entry['durationSeconds'], durationSeconds=entry['durationSeconds'],
            assetReferences=[dict(className='ParticleSystem', objectPath=entry['system'])],
            serializedLabels=[f'bone{number}', 'FX-Ninave', 'CEFParticleData', f"ParticleSystem'{entry['system']}'"],
            synthetic=dict(kind='summons sequence SkeletalMeshFX.CEFAN_Particle', clonedFrom=bow['notifyId'],
                projectile=PROJECTILE.name, anchor='bone', recentred=False))
        payload = header + entry['block']
        row['serializedPayload'] = dict(bow['serializedPayload'], data=base64.b64encode(payload).decode('ascii'), byteSize=len(payload), sha256='synthetic')
        notifies.append(row)
        rows.append(dict(notifyId=row['notifyId'], system=entry['system'], anchor='bone', startSeconds=entry['time'], durationSeconds=entry['durationSeconds']))
    old_system = root['assetReferences'][0]['objectPath']
    for number, entry in enumerate(world):
        if entry['system'].lower() in SKIPPED_SYSTEMS:
            skipped.append(entry)
            continue
        raw_row = drv.replace_particle_reference(base64.b64decode(root['serializedPayload']['data']), old_system, entry['system'])
        row = copy.deepcopy(root)
        row.update(notifyId=f"{root['notifyId']}-world{number}", localTimeSeconds=entry['time'], sourceEndSeconds=entry['time'],
            durationSeconds=0.0, assetReferences=[dict(className='ParticleSystem', objectPath=entry['system'])],
            serializedLabels=[f'world{number}', 'FX-Ninave', 'CEFParticleData', f"ParticleSystem'{entry['system']}'"],
            synthetic=dict(kind='summons sequence CreateFX', clonedFrom=root['notifyId'], projectile=PROJECTILE.name, anchor='root',
                recentred=True, positionMeters=entry['positionMeters'], sourceScale=entry['scale']))
        row['serializedPayload'] = dict(root['serializedPayload'], data=base64.b64encode(raw_row).decode('ascii'), byteSize=len(raw_row), sha256='synthetic')
        notifies.append(row)
        rows.append(dict(notifyId=row['notifyId'], system=entry['system'], anchor='root', startSeconds=entry['time'],
            positionMeters=entry['positionMeters'], scale=entry['scale']))
    notifies.sort(key=lambda n: n['localTimeSeconds'])
    cameo = dict(document, actions=[dict(action, stages=[dict(stageIndex=0, stageName='SK_Parkunas', sourceOffset=stage['sourceOffset'],
        animationClips=[dict(clipName='SK_Parkunas', lengthSeconds=CLIP_SECONDS, notifyId=bow['notifyId'])],
        notifies=notifies, unsupportedUnresolved=[], summary=dict(notifyCount=len(notifies)))])])
    patched = evidence / 'actions' / f'{BOW_TEMPLATE[0]}.action-effects.cameo.json'
    source.write(patched, cameo)
    source.write(evidence / 'synthetic_cameo_notifies.json', dict(projectile=str(PROJECTILE), rows=rows, skipped=skipped))
    return patched


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--evidence-root', type=Path, default=EVIDENCE)
    parser.add_argument('--acquire', action='store_true')
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--native-first', type=int, default=4470)
    parser.add_argument('--native-last', type=int, default=4479)
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
