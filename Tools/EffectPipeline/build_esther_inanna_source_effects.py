"""Restore the Esther Inanna allied strike from its own source Action notifies.

MN_SLINN_00.loa action 543100 (the Kouku Esther summon 59620's allied skill,
DestroySkillIndex 543120 - 20) plays the strike as a three-clip chain
Att_Battle_1_01 -> 1_02 -> 1_03. Each clip becomes one full-restore document
whose elements carry the original notify time, local transform, snapshot-root
basis or bone follow (Bip001-R/L-Hand, B_Effectroot, B_Root); the NPC cue
document only names which document starts with which clip.

Source acquisition, native material recovery and document projection are the
Kouku pattern pipeline, driven from the installed game packages the same way
the vehicle skill effects were, exactly as the Silian restore. This driver owns
only its evidence root, the Inanna program range, the socket contract and the
three authored documents.
"""
from pathlib import Path
import argparse
import collections
import copy
import json
import math
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
for extra in ('Tools/EffectPipeline', 'Tools/LevelPlacementExtractor',
              'Tools/VehiclePipeline', 'Tools/ModelAssetConverter'):
    sys.path.insert(0, str(ROOT / extra))
import build_kouku_gate1_full_restore as source
import build_kouku_showtime_restore as library
import build_vehicle_skill_effects as vehicle
from extract_ue3_placements import resolve_physical_package
import extract_ue3_placements as ue3
from extract_ue3_effect_material_closure import load_package, find_export

ARCHETYPE = 'NPC_59620'
ASSET_PREFIX = 'effect.esther.inanna.'
ACTION_ID = 543100
ACTION_LOA = Path('C:/Users/95jus/Downloads/SourceData/SourceData/LPK/data3/EFGame_Extra/ClientData/XmlData/Action/MN_SLINN_00.loa')
ACTION_PROFILE = 'MN_SLINN_00'
# runtime clip -> (source stage index, label)
CLIPS = {
    'npc_att_battle_1_01': (0, 'Att_Battle_1_01'),
    'npc_att_battle_1_02': (1, 'Att_Battle_1_02'),
    'npc_att_battle_1_03': (2, 'Att_Battle_1_03'),
}
SOURCE_MESH = ('mn_slinn_00', 'mesh.mn_slinn_00_sk')
# The ground circle that outlives the summon is not an Action notify: SkillBuff
# 543100 (Archetype Inanna_001, Duration 20000 ms, BuffFXApply) keeps the zone
# loop alive and ends it. No .loa/.db/.xml references the loop, so the two zone
# systems are appended to stage 2 as synthetic notifies cloned from KZ_Start_01
# (same root placement), at the buff duration.
ZONE_SOURCE_NOTIFY = 'action-543100/stage-002/notify-007'
ZONE_BUFF_DURATION_SECONDS = 20.0
ZONE_NOTIFIES = (
    ('kzloop', 'FX_ESTHER_SLINN_00.Par_S_SLINN_KZBuff_01L', 0.0, ZONE_BUFF_DURATION_SECONDS),
    ('kzend', 'FX_ESTHER_SLINN_00.Par_S_SLINN_KZBuff_01E', ZONE_BUFF_DURATION_SECONDS, 0.0),
)

# --- cameo profile -----------------------------------------------------------
# The Kouku Esther cast (COMMONACTION 53204 -> CommonActionEffect 108) summons
# the invisible carrier NPC 53500 (MN_ISTM_00) whose action carries no visual
# notify, so the visible Inanna is the PlaySkeletalMesh cameo that other casters
# play: MN_SLINN_00_SK_dead + SK_MagicShield with bone particles embedded in the
# notify, followed by the Shield dome / Tree / TreeDecal world set. The closest
# complete source of that cameo is NP_LPDA_01.loa action 4260115 stage 2.
# Times below are relative to the mesh notify (1.89 s into that stage); world
# positions were authored around the LPDA caster, so the dome/tree/aura are
# recentred on Inanna's root and the four TreeDecals keep their +-6 m cross.
PROFILE = 'chain'
CAMEO_ACTION_PROFILE = 'NP_LPDA_01'
CAMEO_ACTION_LOA = ACTION_LOA.with_name('NP_LPDA_01.loa')
CAMEO_SOURCE_ACTION_ID = 4260115
CAMEO_SOURCE_STAGE = 2
CAMEO_MESH_NOTIFY = 'action-4260115/stage-002/notify-017'
CAMEO_CLIP = 'npc_sk_magicshield'
CAMEO_CLIP_SECONDS = 121.0 / 30.0
# LPDA-only systems (its own hand cast, dust, shield hit and the story-driven
# shield crack) are not part of the Esther presentation.
CAMEO_WORLD_EXCLUDED = ('FX_MN_LPDA_01-3_X.', 'Par_X_SLINN_LPDA_Shield_Carck')
CAMEO_RECENTRED = ('Par_X_SLINN_LPDA_Tree_02_02', 'Par_X_SLINN_LPDA_Shield_02_01', 'Par_Z_SLINN_Buff01_01', 'Par_Z_SLINN_Buff01')
# The LPDA world set (shield dome, tree decals) is that story's Inanna; the
# user identified it as the Illiakan gate-4 hidden Inanna. The Kouku Esther's
# own zone is the d_ family no .loa references: teleport in/out around the
# cameo and the 14 m buff02 circle (start -> loop -> end). No source owns the
# loop length, so it is a tuned constant.
KOUKU_ZONE_LOOP_SECONDS = 15.0
KOUKU_ZONE_START_SECONDS = 2.0
KOUKU_WORLD = (
    ('teleport_in', 'FX_ESTHER_SLINN_00.par_d_slinn_00-4_teleport_01', 0.0, 0.0),
    ('zone_start', 'FX_ESTHER_SLINN_00.par_d_slinn_buff02_01s', KOUKU_ZONE_START_SECONDS, 0.0),
    ('zone_loop', 'FX_ESTHER_SLINN_00.par_d_slinn_buff02_02l', KOUKU_ZONE_START_SECONDS, KOUKU_ZONE_LOOP_SECONDS),
    ('zone_loop2', 'FX_ESTHER_SLINN_00.par_d_slinn_buff02_02l2', KOUKU_ZONE_START_SECONDS, KOUKU_ZONE_LOOP_SECONDS),
    ('zone_loop3', 'FX_ESTHER_SLINN_00.par_d_slinn_buff02_02l3', KOUKU_ZONE_START_SECONDS, KOUKU_ZONE_LOOP_SECONDS),
    ('zone_end', 'FX_ESTHER_SLINN_00.par_d_slinn_buff02_03e', KOUKU_ZONE_START_SECONDS + KOUKU_ZONE_LOOP_SECONDS, 0.0),
    ('zone_end1', 'FX_ESTHER_SLINN_00.par_d_slinn_buff02_03e1', KOUKU_ZONE_START_SECONDS + KOUKU_ZONE_LOOP_SECONDS, 0.0),
    ('teleport_out', 'FX_ESTHER_SLINN_00.par_d_slinn_00-4_teleport_02', CAMEO_CLIP_SECONDS, 0.0),
)


def configure_cameo():
    global PROFILE, ACTION_ID, ACTION_PROFILE, ACTION_LOA, CLIPS, ASSET_PREFIX
    PROFILE = 'cameo'
    ACTION_ID = CAMEO_SOURCE_ACTION_ID
    ACTION_PROFILE = CAMEO_ACTION_PROFILE
    ACTION_LOA = CAMEO_ACTION_LOA
    CLIPS = {CAMEO_CLIP: (0, 'SK_MagicShield')}
    ASSET_PREFIX = 'effect.esther.inanna.cameo.'
NPC_MODEL = 'Character/NPC/Npc_59620/Npc_59620.wmodel'
# Group 3904 keeps 3952..3967 after the Silian cohort; the runtime profile
# bound is 4607 and the 4352 group only carries two installed programs, so an
# overflow continues at 4354. Exact Silian/vehicle programs are reused by ID.
NATIVE_FIRST, NATIVE_LAST = 3952, 3967
SILIAN_REVIEWED = ROOT / 'out/SilianFX20260920/material/reviewed'
VEHICLE_REVIEWED = ROOT / 'out/VehicleSkillEffects20260914/material/reviewed'
TEXTURE_ROOT = 'Effect/Esther/Inanna/FullRestore/Textures'
# The 09-07 closure export installed every FX_ESTHER_SLINN_00 mesh with the
# x100 cook the projector's modelPreScale 0.01 expects; shared FX_SM meshes may
# already live in another Esther folder.
MESH_ROOTS = ('Effect/Esther/Inanna/Meshes', 'Effect/Esther/Thirain/Meshes', 'Effect/Esther/Wei/Meshes',
              'Effect/Esther/Ninave/Meshes', 'Effect/KoukuSaydon/FullRestore/Meshes')
AUTHORED = ROOT / 'Data/Effects/Authored'
RESOURCES = ROOT / 'Client/Bin/Resources'
CATALOG = ROOT / 'Data/Effects/EffectCatalog.json'
CUE_DOCUMENT = ROOT / 'Data/Effects/NpcActionCues' / (ARCHETYPE + '.npcactioncues.json')


def asset_id(clip):
    return ASSET_PREFIX + clip + '.full.restore'


def extract_actions(evidence):
    output = evidence / 'actions'
    subprocess.run([sys.executable, str(ROOT / 'Tools/LevelPlacementExtractor/extract_action_effect_notifies.py'),
        '--source', f'{ACTION_PROFILE}={ACTION_LOA}', '--output', str(output), '--action-id', str(ACTION_ID)], check=True)
    return output / f'{ACTION_PROFILE}.action-effects.json'


def socket_contract(evidence):
    """The NPC skeletal mesh sockets, with the vehicle cook's bone-Y mirror."""
    path = evidence / 'source_socket_contract.json'
    if path.is_file():
        return source.read(path)
    logical, mesh_path = SOURCE_MESH
    package = load_package(resolve_physical_package(vehicle.UMODEL, vehicle.RELEASE, logical, 'kr'), ue3.LOSTARK_KR_AES_KEY)
    mesh = source.record_from_export(package, logical, find_export(package, mesh_path))
    assert mesh['className'] == 'skeletalmesh', mesh['className']
    prop = source.imported.prop
    sockets = []
    for order, export_index in enumerate(prop(mesh['properties'], 'sockets')):
        row = source.record_from_export(package, logical, package.exports[export_index - 1])
        props = row['properties']
        location = prop(props, 'relativelocation', {})
        rotation = prop(props, 'relativerotation', {})
        scale = prop(props, 'relativescale', {})
        position = [float(location.get(k, 0.0)) for k in ('x', 'y', 'z')]
        rotator = [float(rotation.get(k, 0.0)) for k in ('pitch', 'yaw', 'roll')]
        factors = [float(scale.get(k, 1.0)) for k in ('x', 'y', 'z')]
        sockets.append(dict(sourceIndex=order, socketName=prop(props, 'socketname'), boneName=prop(props, 'bonename'),
            sourceObject=row['fullPath'],
            sourceTransform=dict(positionUeUnits=position, rotationUnrealUnits=rotator, scale=factors),
            runtimeLocalTransform=dict(position=[position[0] * 0.01, -position[1] * 0.01, position[2] * 0.01],
                rotationDegrees=[rotator[0] * 360.0 / 65536.0, -rotator[1] * 360.0 / 65536.0, -rotator[2] * 360.0 / 65536.0],
                scale=factors),
            transformEvidence='EXPLICIT_SOCKET_PROPERTIES_BONE_Y_MIRRORED'))
    bones = vehicle.wmodel_bones(NPC_MODEL)
    missing = sorted({s['boneName'].casefold() for s in sockets} - {b.casefold() for b in bones})
    assert not missing, ('socket bones absent from the installed WModel', missing)
    contract = dict(schema='lostark.ue3-skeletal-mesh-sockets', formatVersion=1,
        source=dict(package=logical, skeletalMesh=mesh_path, sourcePackage=str(package.path),
            positionUnitScale=0.01, rotationUnitScaleDegrees=360.0 / 65536.0),
        runtimeModel=dict(assetId=NPC_MODEL, bones=bones), sockets=sockets)
    source.write(path, contract)
    return contract


def install_bone_resolution():
    """Hand_01 follows Bip001-R-Hand / Bip001-L-Hand: bones without socket
    names. The shared decoder resolves only b_* bones or socket-backed ones, so
    every other anchor is resolved against the installed WModel skeleton the
    contract carries, as the vehicle cues do. Both acquire and project decode
    through this one function."""
    if getattr(source.decode_typed_payload, 'resolvesBoneAnchors', False):
        return
    original = source.decode_typed_payload

    def decode(source_type, payload, contract, references, labels):
        cue = original(source_type, payload, contract, references, labels)
        attachment = cue.get('attachment') if isinstance(cue, dict) else None
        if attachment and attachment.get('mode') == 'FOLLOW_NAMED_ANCHORS':
            bones = {b.casefold(): b for b in (contract or {}).get('runtimeModel', {}).get('bones', [])}
            for anchor in attachment['runtimeAnchors']:
                runtime = bones.get(anchor['sourceAnchorName'].casefold())
                if anchor['resolutionStatus'] == 'MISSING_SOURCE_SOCKET' and runtime:
                    anchor.update(runtimeBoneName=runtime, resolutionStatus='EXACT_SOURCE_BONE')
            primary = attachment['runtimeAnchors'][0]
            attachment.update(runtimeBoneName=primary['runtimeBoneName'], runtimeResolutionStatus=primary['resolutionStatus'])
        return cue
    decode.resolvesBoneAnchors = True
    source.decode_typed_payload = decode


def replace_particle_reference(raw, old, new):
    """Swap the length-prefixed ParticleSystem'..' FString of a CEFParticleData block."""
    import struct
    old_ref = f"ParticleSystem'{old}'".encode('ascii') + b'\0'
    at = raw.find(old_ref)
    assert at >= 4 and struct.unpack_from('<i', raw, at - 4)[0] == len(old_ref), ('reference not found', old)
    new_ref = f"ParticleSystem'{new}'".encode('ascii') + b'\0'
    return raw[:at - 4] + struct.pack('<i', len(new_ref)) + new_ref + raw[at + len(old_ref):]


def zone_actions(evidence):
    """The extracted action file plus the synthetic zone loop/end notifies."""
    import base64
    actions = evidence / 'actions' / f'{ACTION_PROFILE}.action-effects.json'
    if not actions.is_file():
        actions = extract_actions(evidence)
    patched = actions.with_name(f'{ACTION_PROFILE}.action-effects.zone.json')
    document = source.read(actions)
    action = next(a for a in document['actions'] if int(a['actionId']) == ACTION_ID)
    stage_index = int(ZONE_SOURCE_NOTIFY.split('/')[1].split('-')[1])
    stage = next(s for s in action['stages'] if s['stageIndex'] == stage_index)
    template = next(n for n in stage['notifies'] if n['notifyId'] == ZONE_SOURCE_NOTIFY)
    old_system = template['assetReferences'][0]['objectPath']
    stage['notifies'] = [n for n in stage['notifies'] if not n.get('synthetic')]
    synthetic = []
    for suffix, system, offset, duration in ZONE_NOTIFIES:
        row = copy.deepcopy(template)
        raw = replace_particle_reference(base64.b64decode(template['serializedPayload']['data']), old_system, system)
        row.update(notifyId=ZONE_SOURCE_NOTIFY + '-' + suffix, localTimeSeconds=template['localTimeSeconds'] + offset,
            sourceEndSeconds=template['localTimeSeconds'] + offset + duration, durationSeconds=duration,
            assetReferences=[dict(className='ParticleSystem', objectPath=system)],
            serializedLabels=[label.replace(old_system, system) for label in template['serializedLabels']],
            synthetic=dict(clonedFrom=ZONE_SOURCE_NOTIFY, owner='EFTable_SkillBuff 543100 Inanna_001 Duration 20000', system=system))
        row['serializedPayload'] = dict(template['serializedPayload'], data=base64.b64encode(raw).decode('ascii'),
            byteSize=len(raw), sha256='synthetic')
        stage['notifies'].append(row)
        synthetic.append(row['synthetic'] | dict(notifyId=row['notifyId'], localTimeSeconds=row['localTimeSeconds'], durationSeconds=duration))
    stage['notifies'].sort(key=lambda n: n['localTimeSeconds'])
    source.write(patched, document)
    source.write(evidence / 'synthetic_zone_notifies.json', synthetic)
    return patched


def cameo_actions(evidence):
    """One synthetic stage: the cameo mesh's embedded bone particles as
    PlayParticleEffect notifies plus the retimed world notifies of the source stage."""
    import base64
    actions = evidence / 'actions' / f'{ACTION_PROFILE}.action-effects.json'
    if not actions.is_file():
        actions = extract_actions(evidence)
    document = source.read(actions)
    action = next(a for a in document['actions'] if int(a['actionId']) == ACTION_ID)
    stage = next(s for s in action['stages'] if s['stageIndex'] == CAMEO_SOURCE_STAGE)
    mesh = next(n for n in stage['notifies'] if n['notifyId'] == CAMEO_MESH_NOTIFY)
    assert mesh['sourceType'] == 'PlaySkeletalMesh' and 'SK_MagicShield' in mesh['serializedLabels'], mesh['notifyId']
    mesh_start = mesh['localTimeSeconds']
    template = next(n for n in stage['notifies'] if n['sourceType'] == 'PlayParticleEffect'
                    and 'Par_X_SLINN_LPDA_Shield_02_01' in n['assetReferences'][0]['objectPath'])
    template_raw = base64.b64decode(template['serializedPayload']['data'])
    marker = bytes([16, 0, 0, 0]) + b'CEFParticleData' + bytes([0])
    header = template_raw[:template_raw.index(marker)]
    notifies, rows = [], []
    for number, entry in enumerate(vehicle.skeletal_mesh_particles(mesh, '')):
        assert entry['block'].startswith(marker), entry['system']
        row = copy.deepcopy(template)
        row.update(notifyId=f"{CAMEO_MESH_NOTIFY}-bone{number}", localTimeSeconds=entry['startSeconds'],
            sourceEndSeconds=entry['startSeconds'] + entry['durationSeconds'], durationSeconds=entry['durationSeconds'],
            assetReferences=[dict(className='ParticleSystem', objectPath=entry['system'])],
            serializedLabels=['Cameo', 'FX-SLINN', 'CEFParticleData', f"ParticleSystem'{entry['system']}'"],
            synthetic=dict(kind='PlaySkeletalMesh.CEFAN_Particle', clonedFrom=CAMEO_MESH_NOTIFY, system=entry['system']))
        raw = header + entry['block']
        row['serializedPayload'] = dict(template['serializedPayload'], data=base64.b64encode(raw).decode('ascii'), byteSize=len(raw), sha256='synthetic')
        notifies.append(row)
        rows.append(dict(notifyId=row['notifyId'], system=entry['system'], startSeconds=entry['startSeconds'], durationSeconds=entry['durationSeconds'], anchor='bone'))
    old_system = template['assetReferences'][0]['objectPath']
    for suffix, system, start, duration in KOUKU_WORLD:
        row = copy.deepcopy(template)
        raw = replace_particle_reference(template_raw, old_system, system)
        row.update(notifyId=f"{CAMEO_MESH_NOTIFY}-{suffix}", localTimeSeconds=start, sourceEndSeconds=start + duration,
            durationSeconds=duration, assetReferences=[dict(className='ParticleSystem', objectPath=system)],
            serializedLabels=[suffix, 'FX-SLINN', 'CEFParticleData', f"ParticleSystem'{system}'"],
            synthetic=dict(kind='kouku zone (no source notify)', clonedFrom=template['notifyId'], recentred=True,
                loopSeconds=KOUKU_ZONE_LOOP_SECONDS))
        row['serializedPayload'] = dict(template['serializedPayload'], data=base64.b64encode(raw).decode('ascii'), byteSize=len(raw), sha256='synthetic')
        notifies.append(row)
        rows.append(dict(notifyId=row['notifyId'], system=system, startSeconds=start, durationSeconds=duration, anchor='root', recentred=True))
    notifies.sort(key=lambda n: n['localTimeSeconds'])
    cameo = dict(document, actions=[dict(action, stages=[dict(stageIndex=0, stageName='SK_MagicShield', sourceOffset=stage['sourceOffset'],
        animationClips=[dict(clipName='SK_MagicShield', lengthSeconds=CAMEO_CLIP_SECONDS, notifyId=CAMEO_MESH_NOTIFY)],
        notifies=notifies, unsupportedUnresolved=[], summary=dict(notifyCount=len(notifies)))])])
    patched = actions.with_name(f'{ACTION_PROFILE}.action-effects.cameo.json')
    source.write(patched, cameo)
    source.write(evidence / 'synthetic_cameo_notifies.json', rows)
    return patched


def acquire_stage(evidence, clip):
    """One selected source stage through the shared Kouku acquisition."""
    vehicle.native_environment()
    install_bone_resolution()
    stage, label = CLIPS[clip]
    folder = evidence / 'stages' / clip
    folder.mkdir(parents=True, exist_ok=True)
    source.write(folder / 'source_socket_contract.json', socket_contract(evidence))
    source.ACTION = cameo_actions(evidence) if PROFILE == 'cameo' else zone_actions(evidence)
    source.SELECTED = {ACTION_ID: ([stage], label)}
    source.GRAPH = evidence / 'no-canonical-particle-graph'
    source.source_package = lambda domain, logical: vehicle.SCRIPT_PACKAGES[logical]
    result = source.acquire(folder, lambda name: resolve_physical_package(vehicle.UMODEL, vehicle.RELEASE, name, 'kr'))
    return folder, result


def acquire(evidence):
    summary = {}
    for clip in CLIPS:
        folder, (index, notifies, occurrences, records) = acquire_stage(evidence, clip)
        summary[clip] = dict(notifies=len(notifies), emitters=len(occurrences),
            emptyEmitters=len(source.read(folder / 'source_empty_emitters.json')),
            materials=len({o['sourceMaterial'] for o in occurrences}),
            shapes=dict(collections.Counter(o['rendererShape'] for o in occurrences)),
            attachments=dict(collections.Counter(n['cue']['attachment']['mode'] for n in notifies
                                                 if n['sourceType'] == 'PlayParticleEffect')))
    source.write(evidence / 'source_summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False))


def exact_texture_reuse(pattern, out):
    """Reuse an installed program only when its textures are the ones this
    source resolves now; an older cohort may have lowered the same material
    before the MIC's own native texture table was read."""
    original = pattern.reuse_native_programs

    def reuse(selected, roots, source_materials=None):
        # The shared generator now disqualifies texture/parameter drift itself
        # (source_materials); the name-keyed check below stays as a second gate.
        fresh, reused = original(selected, roots, source_materials)
        inputs = {m['sourceMaterial']: m for m in source.read(out / 'native_material_inputs.json')['materials']}
        kept, rejected = [], []
        for row in reused:
            effective = inputs[row['sourceMaterial']]['effectiveTextures']
            resolved = {t['parameterName']: t['sourceObjectPath'] for t in effective}
            # Engine-only expressions carry no parameter name; the generator
            # names them by expression index.
            resolved.update({'native_texture_%d' % t['index']: t['sourceObjectPath'] for t in effective})
            mismatch = [dict(name=t['name'], installed=t['sourceObjectPath'], resolved=resolved.get(t['name']))
                        for t in row['textures'] if resolved.get(t['name']) != t['sourceObjectPath']]
            if mismatch:
                rejected.append(dict(program=row['program'], sourceMaterial=row['sourceMaterial'], textures=mismatch))
                fresh.append(next(s for s in selected if pattern.native_key(s) == pattern.native_key(row)))
            else:
                kept.append(row)
        source.write(out / 'rejected_native_reuse.json', rejected)
        return fresh, kept
    pattern.reuse_native_programs = reuse

    # The candidate header generated inside prepare() is review output; a
    # deferred program stays listed in the merged contract and is excluded from
    # the install step exactly as the vehicle cohort did.
    # A reused program keeps its installed distortion pass; only fresh programs
    # get a companion lowered and compared against their own color row.
    original_distortion = pattern.prepare_distortion

    def prepare_distortion(out_dir, merged, first, *args, **kwargs):
        reused = {p['program'] for p in source.read(out_dir / 'reused_native_programs.json')['programs']}
        original_distortion(out_dir, dict(merged, programs=[p for p in merged['programs'] if p['program'] not in reused]), first, *args, **kwargs)
    pattern.prepare_distortion = prepare_distortion

    import install_kouku_gate1_native_materials as tables
    original_install = tables.install

    def install(contract_path, evidence, header_path, *args, **kwargs):
        contract = source.read(contract_path)
        if contract.get('deferredPrograms') and header_path.parent == out:
            complete = contract_path.with_name(contract_path.stem + '.complete.json')
            source.write(complete, dict(contract, deferredPrograms=[]))
            contract_path = complete
        return original_install(contract_path, evidence, header_path, *args, **kwargs)
    tables.install = install


def native_occurrence_excluded(occurrence):
    if occurrence['rendererShape'] == 'animationTrail':
        return 'SOURCE_TRAILS_NOT_PROJECTED'
    if not occurrence['sourceMaterial'] and occurrence['rendererShape'] != 'mesh':
        return 'SOURCE_NULL_MATERIAL_NON_MESH_EMITTER'
    return None


def native(evidence, first, last):
    vehicle.TEXTURE_ROOT = TEXTURE_ROOT
    pattern = vehicle.native_environment()
    root = evidence / 'material'
    root.mkdir(parents=True, exist_ok=True)
    exact_texture_reuse(pattern, root / 'native')
    excluded, occurrences, records, defaults = [], [], {}, {}
    for clip in CLIPS:
        folder = evidence / 'stages' / clip
        for occurrence in source.read(folder / 'source_occurrences.json'):
            reason = native_occurrence_excluded(occurrence)
            if reason:
                excluded.append(dict(elementId=occurrence['elementId'], sourceEmitter=occurrence['sourceEmitter'],
                    rendererShape=occurrence['rendererShape'], reason=reason))
                continue
            occurrences.append(occurrence)
        records.update(source.read(folder / 'source_module_inputs.json')['records'])
        for row in source.read(folder / 'source_class_defaults.json')['records']:
            defaults[row['fullPath']] = row
    source.write(root / 'source_occurrences.json', occurrences)
    source.write(root / 'native_input_exclusions.json', excluded)
    source.write(root / 'source_module_inputs.json', dict(records=records))
    source.write(root / 'source_class_defaults.json', dict(records=[defaults[k] for k in sorted(defaults)]))
    # The reviewed Silian contract already carries the vehicle programs it
    # kept; an Inanna re-run reuses its own reviewed contract first.
    reuse = [next(path for path in (root / 'reviewed', SILIAN_REVIEWED, VEHICLE_REVIEWED)
                  if (path / 'native_runtime_contract.json').is_file())]
    # Reused programs keep their IDs; a new permutation numbers after them.
    installed_ids = [p['program'] for p in source.read(reuse[0] / 'native_runtime_contract.json')['programs']]
    first = max(first, max(installed_ids) + 1)
    assert first <= last, ('native program range exhausted', first, last)
    vehicle.native_materials_prepare(pattern, root, first, last, reuse)
    contract = source.read(root / 'native' / 'native_runtime_contract.json')
    reused = source.read(root / 'native' / 'reused_native_programs.json')['programs']
    failures = source.read(root / 'native' / 'source_material_failures.json')
    summary = dict(programs=len(contract['programs']), reused=len(reused), deferred=len(contract['deferredPrograms']),
        sourceFailures=len(failures), excludedOccurrences=len(excluded), first=first, last=last,
        shapes=dict(collections.Counter(p['rendererShape'] for p in contract['programs'])),
        blends=dict(collections.Counter(p['nativeBlend'] for p in contract['programs'])))
    source.write(root / 'native_summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False))


def mesh_asset(source_mesh):
    package_name, relative = source_mesh.split('.', 1)
    for mesh_root in MESH_ROOTS:
        asset = f"{mesh_root}/{package_name.upper()}/{relative.rsplit('.', 1)[-1]}.wmodel"
        if (RESOURCES / asset).is_file():
            return asset
    raise AssertionError(('Inanna source mesh is not installed', source_mesh))


def installed_native_materials():
    """(source material, renderer shape) -> an installed native material block."""
    installed = {}
    for pattern in ('effect.kouku.*.effect.json', 'effect.vehicle.*.effect.json'):
        for path in sorted(AUTHORED.glob(pattern)):
            for element in source.read(path)['elements']:
                material = element.get('material', {})
                profile = str(material.get('sourceProfile', {}).get('runtimeShaderProfileId', ''))
                if profile.startswith('effect.ue3.kouku-'):
                    installed.setdefault((material['sourceMaterialPath'], element['sourceRecipe'].get('rendererShape')),
                                         (material, path.name))
    return installed


def project_clip(evidence, clip, installed, deferred_programs):
    folder, (index, notifies, occurrences, records) = acquire_stage(evidence, clip)
    projection = evidence / 'projection' / clip
    reviewed = evidence / 'material' / 'reviewed'
    deferred = dict(deferred_programs)
    deferred.update({o['elementId']: dict(reason=native_occurrence_excluded(o))
                     for o in occurrences if native_occurrence_excluded(o)})

    # A deferred program whose exact source material and renderer shape already
    # has an installed native program reuses that installed material.
    reused = {}
    for identity in list(deferred):
        occurrence = next((o for o in occurrences if o['elementId'] == identity), None)
        match = occurrence and installed.get((occurrence['sourceMaterial'], occurrence['rendererShape']))
        if match:
            reused[identity] = dict(material=match[0], sourceDocument=match[1], deferredReason=deferred.pop(identity).get('reason'))
    source.write(projection / 'reused_installed_native_materials.json',
        {k: dict(v, runtimeShaderProfileId=v['material']['sourceProfile']['runtimeShaderProfileId']) for k, v in reused.items()})
    # An emitter without a Lifetime module (teleport_01 emitter_5: UE3 infinite
    # lifetime) fails the portable carrier cardinality and would fail-close the
    # whole document; it is left out and recorded.
    module_inputs = source.read(folder / 'source_module_inputs.json')['records']
    for o in occurrences:
        if not any(
                module_inputs.get(m, {}).get('className') == 'particlemodulelifetime' for m in o['moduleOrder']):
            deferred[o['elementId']] = dict(reason='SOURCE_EMITTER_HAS_NO_LIFETIME_MODULE')
    usable = [o for o in occurrences if o['elementId'] not in deferred]
    by_id = {o['elementId']: o for o in usable}
    source.write(projection / 'deferred_occurrences.json',
        {o['elementId']: deferred[o['elementId']] for o in occurrences if o['elementId'] in deferred})
    patch = library.patch_simulation_providers(projection, reviewed / 'native_material_patch.json', index, usable)
    natives = {key: program['material'] for program in source.read(patch)['programs'] for key in program['occurrences']}
    natives.update({key: value['material'] for key, value in reused.items()})
    # Without a patch the shared projector keeps Kouku mesh paths unchecked;
    # native materials are applied below exactly as it would. The projected
    # attachment (snapshot-root basis or socket follow) and the notify local
    # transform are the placement contract and stay as projected.
    source.project(folder, index, notifies, usable, records, projection / 'raw')
    document = source.read(projection / 'raw' / f'effect.kouku.gate1.{ACTION_ID}.full.restore.effect.json')
    document.update(effectAssetId=asset_id(clip), displayName='Esther Inanna ' + ('cameo ' if PROFILE == 'cameo' else '') + CLIPS[clip][1])
    duplicates = []
    for element in document['elements']:
        # A notify that follows several anchors projects one element per anchor;
        # the copies carry the occurrence id plus the anchor name.
        occurrence_id = element['id'] if element['id'] in by_id else element['id'].rsplit('.', 1)[0]
        occurrence = by_id[occurrence_id]
        if occurrence['kind'] != 'light':
            material = natives.get(occurrence_id)
            assert material is not None, ('missing native material occurrence', element['id'])
            element['material'] = copy.deepcopy(material)
            assert element['material']['sourceMaterialPath'] == occurrence['sourceMaterial']
            element['detail']['uv'].update(start=[0, 0], speed=[0, 0], wave=False, sequence=False)
            element['detail']['color']['emissiveIntensity'] = 1
        # An emitter that lists the same ground-snap module object twice is
        # one snap; the portable runtime admits exactly one per emitter.
        modules = element['sourceRecipe']['modules']
        dropped = [m['stableId'] for m in modules
                   if m['className'] == 'efparticlemodulelocationonground' and '@reference:' in m['stableId']]
        if dropped:
            element['sourceRecipe']['modules'] = [m for m in modules if m['stableId'] not in dropped]
            duplicates.append(dict(elementId=element['id'], dropped=dropped))
        element['groupId'] = asset_id(clip)
        if PROFILE == 'cameo':
            notify = next(n for n in notifies if n['notifyId'] == element['sourcePresentation']['sourceEventId'])
            if notify.get('synthetic', {}).get('recentred'):
                element['detail']['transform']['position'] = [0.0, 0.0, 0.0]
        for resource in element['resources']:
            if resource['slotId'] == 'meshModel':
                resource['assetId'] = mesh_asset(occurrence['sourceMesh'])
    source.write(projection / 'dropped_duplicate_modules.json', duplicates)
    duration = math.ceil(max([e['detail']['timing']['startDelaySeconds'] + e['detail']['timing']['lifeTimeSeconds'] +
        max(e['detail']['particle']['lifeTimeSeconds']) for e in document['elements']] or [0]) * 1000)
    source.write(projection / 'candidate' / (asset_id(clip) + '.effect.json'), document)
    return document, dict(effectAssetId=asset_id(clip), clip=clip, elementCount=len(document['elements']), durationMs=duration,
        deferredEmitters=sorted(o['elementId'] for o in occurrences if o['elementId'] in deferred),
        attachments=dict(collections.Counter(
            ('follow:' + e['actionCueAttachment']['runtimeBoneName']) if e['actionCueAttachment'].get('follow')
            else 'snapshot-root' if e['actionCueAttachment'].get('enabled') else 'none' for e in document['elements'])),
        nativePrograms=sorted({e['material']['sourceProfile']['runtimeShaderProfileId'] for e in document['elements']
            if e['material'].get('sourceProfile', {}).get('enabled')}))


def project(evidence, install):
    reviewed = evidence / 'material' / 'reviewed'
    deferred_programs = {identity: row for row in source.read(reviewed / 'native_deferred_programs.json')
                         for identity in row.get('occurrences', [])}
    installed = installed_native_materials()
    writes, rows = [], []
    for clip in CLIPS:
        document, row = project_clip(evidence, clip, installed, deferred_programs)
        rows.append(row)
        path = AUTHORED / (asset_id(clip) + '.effect.json')
        writes.append((path, (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8')))
    # The NPC cue document names one full-restore document per clip; the
    # notify timeline lives inside each document.
    cues = dict(schema='lostark.npc-action-effect-cues', formatVersion=1, archetypeId=ARCHETYPE,
        cues=[dict(cueId=f'{ARCHETYPE.lower()}.{row["clip"]}', effectAssetId=row['effectAssetId'], clip=row['clip'],
                   startMs=0, durationMs=row['durationMs'], bone='b_effectroot', followBone=False) for row in rows])
    writes.append((CUE_DOCUMENT, (json.dumps(cues, ensure_ascii=False, indent=2) + '\n').encode('utf8')))
    catalog = source.read(CATALOG)
    catalog['effects'] = [row for row in catalog['effects'] if not row['effectAssetId'].startswith(ASSET_PREFIX)]
    catalog['effects'] += [dict(effectAssetId=row['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT',
                                authoringPath='Effects/Authored/' + row['effectAssetId'] + '.effect.json') for row in rows]
    writes.append((CATALOG, (json.dumps(catalog, ensure_ascii=False, indent=2) + '\n').encode('utf8')))
    stale = [path for path in AUTHORED.glob(ASSET_PREFIX + '*.effect.json')
             if path.name not in {asset_id(clip) + '.effect.json' for clip in CLIPS}]
    changed = []
    for path, payload in writes:
        before = path.read_bytes() if path.exists() else None
        if payload != before:
            changed.append(path.relative_to(ROOT).as_posix())
            if install:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(payload)
    if install:
        for path in stale:
            path.unlink()
    source.write(evidence / 'projection' / 'installation.json', dict(installed=install, changedPaths=changed,
        removedPaths=[p.relative_to(ROOT).as_posix() for p in stale], documents=rows))
    for row in rows:
        print('%-36s elements=%-3d deferred=%-2d duration=%dms programs=%d attachments=%s' % (
            row['clip'], row['elementCount'], len(row['deferredEmitters']), row['durationMs'],
            len(row['nativePrograms']), row['attachments']))
    print(('installed' if install else 'candidate') + ' files changed', len(changed), 'stale removed', len(stale) if install else 0)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/InannaFX20260922')
    parser.add_argument('--acquire', action='store_true')
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--native-first', type=int, default=NATIVE_FIRST)
    parser.add_argument('--native-last', type=int, default=NATIVE_LAST)
    parser.add_argument('--install-native', action='store_true')
    parser.add_argument('--project', action='store_true')
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--profile', choices=('chain', 'cameo'), default='chain')
    args = parser.parse_args()
    if args.profile == 'cameo':
        configure_cameo()
    evidence = args.evidence_root.resolve()
    if args.acquire:
        acquire(evidence)
    if args.native:
        native(evidence, args.native_first, args.native_last)
    if args.install_native:
        # The vehicle installer bounds its cohort; this cohort's own range applies.
        vehicle.VEHICLE_NATIVE_FIRST, vehicle.VEHICLE_NATIVE_LAST = min(args.native_first, 3712), args.native_last
        vehicle.install_native(evidence)
    if args.project:
        project(evidence, args.install)


if __name__ == '__main__':
    main()
