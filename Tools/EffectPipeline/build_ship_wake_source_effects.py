"""Restore the Bern ship wake (water trail) from the original ship LookInfo.

Every voyage ship's LookInfo (EFDLShip_<ship>.loa, data4.lpk XmlData/LookInfo/
Ship) attaches its wake to the hull bone B_EffectRoot with one CEFParticleData
per particle system (Par_*_WaterTrail_N_01 near, _F_01 far, plus side foam and,
on the Icebreaker, funnel smoke). The block is the same CEFParticleData layout
a PlayParticleEffect notify carries after its action header, so each block is
spliced behind the header of a real notify and the Kouku source acquisition
does the rest: package closure, native material recovery and V1 projection are
the existing Inanna/vehicle/Maharaka cohort drivers.

There is no Action for a wake, so every ship is a synthetic one-stage action
(actionId = vehicleId) whose notifies start at 0 and last WAKE_SECONDS. The
Client keeps one occurrence per ship alive while the hull moves
(Character::Update_VehicleWake) and stops it at standstill; the source keeps
no speed switch (LookInfo carries none; the particles spawn at a fixed rate
in world space, so length follows the hull's own speed).

Stages: --acquire, --native, --install-native, --project [--install].
"""
from pathlib import Path
import argparse
import base64
import collections
import copy
import hashlib
import json
import re
import shutil
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
for extra in ('Tools/EffectPipeline', 'Tools/LevelPlacementExtractor',
              'Tools/VehiclePipeline', 'Tools/ModelAssetConverter'):
    sys.path.insert(0, str(ROOT / extra))
import build_esther_inanna_source_effects as drv
import build_kouku_gate1_full_restore as source
import build_vehicle_skill_effects as vehicle

# An ASCII evidence root: the Assimp geometry cook cannot open non-ASCII paths.
EVIDENCE = Path('C:/LostArkExtract/ShipWakeFX20260929')
UMODEL = Path('C:/Users/USER/OneDrive/바탕 화면/UModel/umodel_win32/umodel_lostark_v7.exe')
LOOKINFO = ROOT / 'out/Bern3Ship20260925/lpk4/EFGame_Extra/ClientData/XmlData/LookInfo/Ship'
# A real 4225615 PlayParticleEffect notify supplies the action header bytes.
HEADER_ACTION = ROOT / 'out/MaharakaReaudit20260926/MokoAction/MN_ISMP_00.action-effects.json'
HEADER_ACTION_ID = 4225615
TEXTURE_ROOT = 'Effect/Vehicle/Ship/Textures'
MESH_ROOT = 'Effect/Vehicle/Ship/Meshes'
MESH_ROOTS = (MESH_ROOT, 'Effect/Maharaka/Waterpang/Meshes', 'Effect/Esther/Inanna/Meshes',
              'Effect/Esther/Thirain/Meshes', 'Effect/Esther/Wei/Meshes', 'Effect/Esther/Ninave/Meshes',
              'Effect/KoukuSaydon/FullRestore/Meshes')
ASSET_PREFIX = 'effect.vehicle.ship.wake.'
# Bucket 4928 (4928..4991) already owns its Mesh/Particle wrappers; 4928..4953
# are installed, so this cohort takes the tail of the free range.
NATIVE_FIRST, NATIVE_LAST = 4960, 4991
# The loop window of one occurrence. The occurrence is sustained by the owner
# (bOwnerSustainedSourceLoops), so this only bounds the authored document.
WAKE_SECONDS = 10.0

# vehicleId -> (LookInfo file, runtime model, archetype)
SHIPS = {
    8200: ('EFDLShip_ESTOC.ESTOC_01.loa', 'VEHICLE_SHIP_ESTOC'),
    8201: ('EFDLShip_WHITEWIND.WHITEWIND_01.loa', 'VEHICLE_SHIP_WHITEWIND'),
    8202: ('EFDLShip_PIRATE.PIRATE_01.loa', 'VEHICLE_SHIP_ASTRAY'),
    8203: ('EFDLShip_ICEBREAKER.ICEBREAKER_01.loa', 'VEHICLE_SHIP_BARKSTORM'),
    8204: ('EFDLShip_GHOST.GHOST_01.loa', 'VEHICLE_SHIP_GHOST'),
    8205: ('EFDLShip_BRAHMS.BRAHMS_01.loa', 'VEHICLE_SHIP_BRAHMS'),
    8206: ('EFDLShip_TRAGON.TRAGON_01.loa', 'VEHICLE_SHIP_TRAGON'),
    8207: ('EFDLShip_SLOOP.SLOOP_01.loa', 'VEHICLE_SHIP_PNEUMA'),
    8208: ('EFDLShip_MAGICSHIP.MAGICSHIP_01.loa', 'VEHICLE_SHIP_LUMINOUS'),
}
BLOCK = b'\x10\0\0\0CEFParticleData\0'
CATALOG = ROOT / 'Data/Actors/VehicleCatalog.json'


def vehicle_models():
    return {v['vehicleId']: v['modelAssetId'] for v in source.read(CATALOG)['vehicles'] if v.get('ship')}


def lookinfo_blocks(vehicle_id):
    """The CEFParticleData blocks of one ship LookInfo, in file order. Blocks
    without the bone/offset layout (the Icebreaker funnel smoke) are returned
    with bone=None so the caller can record them instead of guessing."""
    raw = (LOOKINFO / SHIPS[vehicle_id][0]).read_bytes()
    starts = []
    at = raw.find(BLOCK)
    while at >= 0:
        starts.append(at)
        at = raw.find(BLOCK, at + len(BLOCK))
    rows = []
    for index, start in enumerate(starts):
        end = starts[index + 1] if index + 1 < len(starts) else len(raw)
        block = raw[start:end]
        reference = re.search(rb"ParticleSystem'([^']+)'\0", block)
        bone = re.search(rb'\x01\0\0\0.\0\0\0(B_[A-Za-z_]+)\0', block[block.find(reference.group(0)):]) if reference else None
        rows.append(dict(index=index, system=reference.group(1).decode('ascii') if reference else '',
                         bone=bone.group(1).decode('ascii') if bone else None, block=block))
    return rows


def header_bytes():
    document = source.read(HEADER_ACTION)
    action = next(a for a in document['actions'] if int(a['actionId']) == HEADER_ACTION_ID)
    notify = next(n for n in action['stages'][1]['notifies'] if n['sourceType'] == 'PlayParticleEffect')
    raw = base64.b64decode(notify['serializedPayload']['data'])
    marker = raw.find(BLOCK, raw.find(b"ParticleSystem'") - 200)
    start = raw.rfind(BLOCK, 0, raw.find(b"ParticleSystem'"))
    return notify, raw[:start]


def synthetic_actions(evidence, ships=None):
    """One synthetic action per ship: a single 'Wake' stage with one notify per
    wake block. Written next to the extracted evidence, as the Maharaka jet."""
    template, head = header_bytes()
    actions = []
    skipped = []
    for vehicle_id in sorted(ships or SHIPS):
        notifies = []
        for row in lookinfo_blocks(vehicle_id):
            if row['bone'] is None or not row['system']:
                skipped.append(dict(vehicleId=vehicle_id, system=row['system'],
                                    reason='LOOKINFO_BLOCK_HAS_NO_BONE_ATTACHMENT_LAYOUT'))
                continue
            raw = head + row['block']
            notify = copy.deepcopy(template)
            notify.update(notifyId=f'action-{vehicle_id}/stage-000/notify-{row["index"]:03d}',
                localTimeSeconds=0.0, sourceEndSeconds=WAKE_SECONDS, durationSeconds=WAKE_SECONDS,
                assetReferences=[dict(className='ParticleSystem', objectPath=row['system'])],
                serializedLabels=['FX', 'CEFParticleData', f"ParticleSystem'{row['system']}'", row['bone']],
                synthetic=dict(kind='SHIP_LOOKINFO_CEFParticleData', lookInfo=SHIPS[vehicle_id][0],
                               blockIndex=row['index'], headerFrom=template['notifyId']))
            notify['serializedPayload'] = dict(template['serializedPayload'], data=base64.b64encode(raw).decode('ascii'),
                byteSize=len(raw), sha256=hashlib.sha256(raw).hexdigest())
            notifies.append(notify)
        actions.append(dict(sourceActionIndex=len(actions), actionId=vehicle_id, displayName=SHIPS[vehicle_id][1] + ' WAKE',
            sourceOffset=0, stages=[dict(stageIndex=0, stageName='Wake', sourceOffset=0,
                animationClips=[dict(clipName='run_battle_1', lengthSeconds=WAKE_SECONDS,
                                     notifyId=f'action-{vehicle_id}/stage-000/clip')],
                notifies=notifies, unsupportedUnresolved=[], summary={})], summary={}))
    document = dict(schema='lostark.ue3-action-effect-source', formatVersion=1, profileId='SHIP_WAKE',
        source=dict(kind='SHIP_LOOKINFO', root=str(LOOKINFO)), ownership={}, actionFilter={}, actions=actions,
        particleSystems=[], meshes=[], materials=[], textures=[], unsupportedUnresolved=skipped, summary={})
    path = evidence / 'actions' / 'SHIP_WAKE.action-effects.json'
    source.write(path, document)
    return path, skipped


def configure(vehicle_id, evidence_root=EVIDENCE):
    """Point the shared driver at one ship: its archetype, its synthetic action,
    its installed model (the bone contract) and this cohort's roots."""
    models = vehicle_models()
    vehicle.UMODEL = UMODEL
    drv.PROFILE = 'chain'
    drv.ARCHETYPE = SHIPS[vehicle_id][1]
    drv.ASSET_PREFIX = f'{ASSET_PREFIX}{vehicle_id}.'
    drv.ACTION_ID = vehicle_id
    drv.ACTION_PROFILE = 'SHIP_WAKE'
    drv.ACTION_LOA = evidence_root / 'actions' / 'SHIP_WAKE.action-effects.json'
    drv.CLIPS = {'run_battle_1': (0, 'Wake')}
    drv.SOURCE_MESH = ('SH_wake', 'mesh.none')
    drv.NPC_MODEL = models[vehicle_id]
    drv.TEXTURE_ROOT = TEXTURE_ROOT
    drv.MESH_ROOTS = MESH_ROOTS
    path = evidence_root / 'actions' / 'SHIP_WAKE.action-effects.json'
    drv.zone_actions = lambda evidence, path=path: path
    # A ship attaches to a hull bone, not a socket: the contract carries the
    # installed skeleton only, so B_EffectRoot resolves to EXACT_SOURCE_BONE.
    drv.socket_contract = lambda evidence, model=models[vehicle_id]: bone_contract(evidence, model)
    return evidence_root / f'ship{vehicle_id}'


def bone_contract(evidence, model):
    path = evidence / 'source_socket_contract.json'
    contract = dict(schema='lostark.ue3-skeletal-mesh-sockets', formatVersion=1,
        source=dict(package='', skeletalMesh='', sourcePackage='', positionUnitScale=0.01,
                    rotationUnitScaleDegrees=360.0 / 65536.0),
        runtimeModel=dict(assetId=model, bones=vehicle.wmodel_bones(model)), sockets=[])
    source.write(path, contract)
    return contract


REQUIRED_MODULE = 'engine.default__particlemodulerequired'


def native_environment():
    """The vehicle texture hook predates the resource_root keyword; the
    installed Resources root stays the destination (as the Maharaka cohort)."""
    vehicle.TEXTURE_ROOT = TEXTURE_ROOT
    pattern = vehicle.native_environment()
    hook = pattern.prepare_textures
    pattern.prepare_textures = lambda evidence, out, resource_root=None: hook(evidence, out)
    return pattern


def native(first, last):
    """One cohort for all nine ships: each source material x vertex factory
    permutation is lowered once inside this cohort's range and shared."""
    configure(8200)
    pattern = native_environment()
    root = EVIDENCE / 'material'
    root.mkdir(parents=True, exist_ok=True)
    drv.exact_texture_reuse(pattern, root / 'native')
    excluded, occurrences, records, defaults, seen = [], [], {}, {}, set()
    for vehicle_id in sorted(SHIPS):
        folder = EVIDENCE / f'ship{vehicle_id}' / 'stages' / 'run_battle_1'
        module_inputs = source.read(folder / 'source_module_inputs.json')['records']
        for occurrence in source.read(folder / 'source_occurrences.json'):
            identity = (occurrence['sourceEmitter'], occurrence['sourceMaterial'])
            if identity in seen:
                continue
            seen.add(identity)
            reason = drv.native_occurrence_excluded(occurrence, module_inputs)
            if reason:
                excluded.append(dict(elementId=occurrence['elementId'], sourceEmitter=occurrence['sourceEmitter'],
                    rendererShape=occurrence['rendererShape'], reason=reason))
                continue
            occurrences.append(occurrence)
        records.update(module_inputs)
        for row in source.read(folder / 'source_class_defaults.json')['records']:
            defaults[row['fullPath']] = row
    records[REQUIRED_MODULE] = dict(fullPath=REQUIRED_MODULE, classPath='engine.particlemodulerequired',
                                    archetypeFullPath=None, properties={})
    source.write(root / 'source_occurrences.json', occurrences)
    source.write(root / 'native_input_exclusions.json', excluded)
    source.write(root / 'source_module_inputs.json', dict(records=records))
    source.write(root / 'source_class_defaults.json', dict(records=[defaults[k] for k in sorted(defaults)]))
    reviewed = root / 'reviewed'
    vehicle.native_materials_prepare(pattern, root, first, last,
        [reviewed] if (reviewed / 'native_runtime_contract.json').is_file() else [])
    contract = source.read(root / 'native' / 'native_runtime_contract.json')
    failures = source.read(root / 'native' / 'source_material_failures.json')
    summary = dict(programs=len(contract['programs']), deferred=len(contract['deferredPrograms']),
        sourceFailures=len(failures), excludedOccurrences=len(excluded), first=first, last=last,
        shapes=dict(collections.Counter(p['rendererShape'] for p in contract['programs'])),
        blends=dict(collections.Counter(p['nativeBlend'] for p in contract['programs'])))
    source.write(root / 'native_summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False))


def install_native(first, last):
    configure(8200)
    native_environment()
    vehicle.VEHICLE_NATIVE_FIRST, vehicle.VEHICLE_NATIVE_LAST = first, last
    vehicle.install_native(EVIDENCE)


def mesh_asset(source_mesh):
    package_name, relative = source_mesh.split('.', 1)
    name = relative.rsplit('.', 1)[-1]
    for mesh_root in MESH_ROOTS:
        for asset in (f'{mesh_root}/{package_name.upper()}/{name}.wmodel', f'{mesh_root}/{name}.wmodel'):
            if (RESOURCES / asset).is_file():
                return asset
    raise AssertionError(('Ship wake source mesh is not installed', source_mesh))


def ensure_meshes(evidence):
    vehicle.MESH_ROOT = MESH_ROOT
    folder = evidence / 'stages' / 'run_battle_1'
    missing = []
    for occurrence in source.read(folder / 'source_occurrences.json'):
        if not occurrence['sourceMesh']:
            continue
        try:
            mesh_asset(occurrence['sourceMesh'])
        except AssertionError:
            missing.append(occurrence)
    if missing:
        vehicle.prepare_geometry(folder, missing)


AUTHORED = ROOT / 'Data/Effects/Authored'
EFFECT_CATALOG = ROOT / 'Data/Effects/EffectCatalog.json'
RESOURCES = ROOT / 'Client/Bin/Resources'


# The installed b_effectroot of all nine ships is bound at the model origin with
# the rotation RotY(-90 deg) (rows (0,0,1),(0,1,0),(-1,0,0), measured from each
# WModel skeleton). The LookInfo particle numbers (velocity -X = astern,
# position offsets) are authored in the hull frame (X bow, Y up), so a socket
# yaw of +90 deg cancels the bone rotation: socket * bone = identity.
BONE_ALIGN_YAW_DEGREES = 90.0


def align_to_hull(document):
    count = 0
    for element in document['elements']:
        attachment = element['actionCueAttachment']
        if not attachment['enabled']:
            continue
        assert attachment['runtimeBoneName'] == 'b_effectroot', attachment
        socket = attachment['socketLocalTransform']
        assert socket['rotationDegrees'] == [0.0, 0.0, 0.0], socket
        socket['rotationDegrees'] = [0.0, BONE_ALIGN_YAW_DEGREES, 0.0]
        count += 1
    return count


def rebind_patch(evidence):
    """The native patch names occurrences of the ships that first introduced a
    (source emitter, source material) pair. Every other ship carries the same
    pair under its own element ids, so its ids join the program of that pair."""
    stage = evidence / 'stages' / 'run_battle_1' / 'source_occurrences.json'
    cohort = {o['elementId']: (o['sourceEmitter'], o['sourceMaterial'])
              for o in source.read(EVIDENCE / 'material' / 'source_occurrences.json')}
    path = evidence / 'material' / 'reviewed' / 'native_material_patch.json'
    patch = source.read(path)
    program_of = {cohort[i]: program for program in patch['programs'] for i in program['occurrences'] if i in cohort}
    bound = {i for program in patch['programs'] for i in program['occurrences']}
    for occurrence in source.read(stage):
        key = (occurrence['sourceEmitter'], occurrence['sourceMaterial'])
        if occurrence['elementId'] not in bound and key in program_of:
            program_of[key]['occurrences'].append(occurrence['elementId'])
    source.write(path, patch)


def project(install):
    reviewed = EVIDENCE / 'material' / 'reviewed'
    deferred_path = reviewed / 'native_deferred_programs.json'
    deferred_rows = source.read(deferred_path) if deferred_path.is_file() else []
    installed = drv.installed_native_materials()
    documents, rows = [], []
    for vehicle_id in sorted(SHIPS):
        evidence = configure(vehicle_id)
        drv.mesh_asset = mesh_asset
        shutil.copytree(reviewed, evidence / 'material' / 'reviewed', dirs_exist_ok=True)
        ensure_meshes(evidence)
        rebind_patch(evidence)
        # The deferred rows name a material, not element ids: every occurrence of
        # that source material in this ship stays out of the document and is listed.
        stage = evidence / 'stages' / 'run_battle_1' / 'source_occurrences.json'
        deferred = {o['elementId']: dict(reason=row['reason'], program=row['program'])
                    for row in deferred_rows for o in source.read(stage)
                    if o['sourceMaterial'] == row['material']}
        document, row = drv.project_clip(evidence, 'run_battle_1', installed, deferred)
        document['displayName'] = f'Ship wake {vehicle_id}'
        row['hullAlignedElements'] = align_to_hull(document)
        documents.append(document)
        rows.append(row)
    writes = [(AUTHORED / (d['effectAssetId'] + '.effect.json'),
               (json.dumps(d, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8')) for d in documents]
    catalog = source.read(EFFECT_CATALOG)
    catalog['effects'] = [row for row in catalog['effects'] if not row['effectAssetId'].startswith(ASSET_PREFIX)]
    catalog['effects'] += [dict(effectAssetId=d['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT',
                                authoringPath='Effects/Authored/' + d['effectAssetId'] + '.effect.json') for d in documents]
    writes.append((EFFECT_CATALOG, (json.dumps(catalog, ensure_ascii=False, indent=2) + '\n').encode('utf8')))
    changed = []
    for path, payload in writes:
        if not path.exists() or path.read_bytes() != payload:
            changed.append(path.relative_to(ROOT).as_posix())
            if install:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(payload)
    source.write(EVIDENCE / 'projection_installation.json', dict(installed=install, changedPaths=changed, documents=rows))
    for row in rows:
        print('%-56s elements=%-4d deferred=%-3d duration=%dms programs=%d attachments=%s' % (
            row['effectAssetId'], row['elementCount'], len(row['deferredEmitters']), row['durationMs'],
            len(row['nativePrograms']), row['attachments']))
    print(('installed' if install else 'candidate') + ' files changed', len(changed))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--actions', action='store_true', help='write the synthetic action file and print the blocks')
    parser.add_argument('--acquire', action='store_true', help='acquire every ship through the shared Kouku acquisition')
    parser.add_argument('--ship', type=int, help='restrict a stage to one vehicleId')
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--install-native', action='store_true')
    parser.add_argument('--native-first', type=int, default=NATIVE_FIRST)
    parser.add_argument('--native-last', type=int, default=NATIVE_LAST)
    parser.add_argument('--project', action='store_true')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    if args.acquire:
        synthetic_actions(EVIDENCE)
        for vehicle_id in ([args.ship] if args.ship else sorted(SHIPS)):
            folder = configure(vehicle_id)
            folder.mkdir(parents=True, exist_ok=True)
            drv.acquire(folder)
    if args.native:
        native(args.native_first, args.native_last)
    if args.install_native:
        install_native(args.native_first, args.native_last)
    if args.project:
        project(args.install)
    if args.actions:
        path, skipped = synthetic_actions(EVIDENCE)
        document = source.read(path)
        for action in document['actions']:
            print(action['actionId'], [n['assetReferences'][0]['objectPath'] for n in action['stages'][0]['notifies']])
        print('skipped', json.dumps(skipped, ensure_ascii=False))
        print(path)


if __name__ == '__main__':
    main()
