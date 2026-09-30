"""Restore the island-entrance anchor symbol (the golden anchor at the Maharaka sea island pier).

Original chain (sea map 30703 DeployData, prop 1040158):

  EFTable_Prop 1040158 -> Model EFDLProp_ITR_10066.ITR_10066
  -> LookInfo Prop/EFDLProp_ITR_10066.ITR_10066.loa (SkeletalMesh ITR_Box128_SK, invisible box)
  -> 3 x CEFParticleData ParticleSystem'FX_BS_03.mark.Par_G_Symbol_Anchor_01'

The harbour docking square (ITR_10297) is a different system and stays under its own asset.
Acquisition, native material recovery, installation and projection reuse the
Ship-wake / Inanna / vehicle cohort drivers; this module owns its evidence root,
program range and the world-root rebind (the marker is spawned as a Level-owned
world root, not attached to a bone).
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
import sys

ROOT = Path(__file__).resolve().parents[2]
for extra in ('Tools/EffectPipeline', 'Tools/LevelPlacementExtractor',
              'Tools/VehiclePipeline', 'Tools/ModelAssetConverter'):
    sys.path.insert(0, str(ROOT / extra))
import build_esther_inanna_source_effects as drv
import build_kouku_gate1_full_restore as source
import build_vehicle_skill_effects as vehicle
import build_ship_wake_source_effects as wake

EVIDENCE = Path('C:/LostArkExtract/IslandAnchorSymbolFX20260929')
UMODEL = wake.UMODEL
LOOKINFO_FILE = Path('C:/Users/USER/.claude/jobs/46aea322/tmp/anchor10066/EFDLProp_ITR_10066.ITR_10066.loa')
TEXTURE_ROOT = 'Effect/Bern/AnchorMarker/Textures'
MESH_ROOT = 'Effect/Bern/AnchorMarker/Meshes'
MESH_ROOTS = (MESH_ROOT,) + wake.MESH_ROOTS
ASSET_PREFIX = 'effect.bern.anchor.marker.'
ACTION_ID = 1040158
CLIP = 'marker'
# The Bern island is placed at 2.0x its retail size (bern_isl72.ISLAND_SCALE); the marker follows the island so the
# retail island-to-marker proportion holds. The retail Prop scale itself is 100 percent (DeployData).
MARKER_SCALE = 2.0
# 4969..4991 are free in bucket 4928 (4961..4968 are the ship wake).
NATIVE_FIRST, NATIVE_LAST = 4973, 4991
MARKER_SECONDS = 10.0
# Any installed model that owns b_root satisfies the shared bone contract; the
# document is rebound to a world root afterwards.
CONTRACT_MODEL = 'Character/LanceMaster/LanceMaster.wmodel'
BLOCK = wake.BLOCK
RESOURCES = ROOT / 'Client/Bin/Resources'
AUTHORED = ROOT / 'Data/Effects/Authored'
CATALOG = ROOT / 'Data/Effects/EffectCatalog.json'


def synthetic_actions(evidence):
    template, head = wake.header_bytes()
    raw = LOOKINFO_FILE.read_bytes()
    starts = [m.start() for m in re.finditer(re.escape(BLOCK), raw)]
    assert len(starts) == 3, starts  # the prop lists the same system three times; block 0 is the base one
    block = raw[starts[0]:starts[1]]
    reference = re.search(rb"ParticleSystem'([^']+)'\0", block)
    bone = re.search(rb'\x01\0\0\0.\0\0\0(B_[A-Za-z_]+)\0', block[block.find(reference.group(0)):])
    system = reference.group(1).decode('ascii')
    payload = head + block
    notify = copy.deepcopy(template)
    notify.update(notifyId=f'action-{ACTION_ID}/stage-000/notify-000', localTimeSeconds=0.0,
        sourceEndSeconds=MARKER_SECONDS, durationSeconds=MARKER_SECONDS,
        assetReferences=[dict(className='ParticleSystem', objectPath=system)],
        serializedLabels=['FX', 'CEFParticleData', f"ParticleSystem'{system}'", bone.group(1).decode('ascii') if bone else 'Bone001'],
        synthetic=dict(kind='PROP_LOOKINFO_CEFParticleData', lookInfo=LOOKINFO_FILE.name, blockIndex=0,
                       headerFrom=template['notifyId']))
    notify['serializedPayload'] = dict(template['serializedPayload'], data=base64.b64encode(payload).decode('ascii'),
        byteSize=len(payload), sha256=hashlib.sha256(payload).hexdigest())
    action = dict(sourceActionIndex=0, actionId=ACTION_ID, displayName='ANCHOR MARKER', sourceOffset=0,
        stages=[dict(stageIndex=0, stageName='Marker', sourceOffset=0,
            animationClips=[dict(clipName=CLIP, lengthSeconds=MARKER_SECONDS, notifyId=f'action-{ACTION_ID}/stage-000/clip')],
            notifies=[notify], unsupportedUnresolved=[], summary={})], summary={})
    document = dict(schema='lostark.ue3-action-effect-source', formatVersion=1, profileId='ANCHOR_MARKER',
        source=dict(kind='PROP_LOOKINFO', root=str(LOOKINFO_FILE.parent)), ownership={}, actionFilter={}, actions=[action],
        particleSystems=[], meshes=[], materials=[], textures=[], unsupportedUnresolved=[], summary={})
    path = evidence / 'actions' / 'ANCHOR_MARKER.action-effects.json'
    source.write(path, document)
    return path, system, bone.group(1).decode('ascii') if bone else None


def configure():
    vehicle.UMODEL = UMODEL
    drv.PROFILE = 'chain'
    drv.ARCHETYPE = 'PROP_ISLAND_ANCHOR_SYMBOL'
    drv.ASSET_PREFIX = ASSET_PREFIX
    drv.ACTION_ID = ACTION_ID
    drv.ACTION_PROFILE = 'ANCHOR_MARKER'
    drv.ACTION_LOA = EVIDENCE / 'actions' / 'ANCHOR_MARKER.action-effects.json'
    drv.CLIPS = {CLIP: (0, 'Marker')}
    drv.SOURCE_MESH = ('ITR_10066', 'mesh.none')
    drv.NPC_MODEL = CONTRACT_MODEL
    drv.TEXTURE_ROOT = TEXTURE_ROOT
    drv.MESH_ROOTS = MESH_ROOTS
    path = drv.ACTION_LOA
    drv.zone_actions = lambda evidence, path=path: path
    drv.socket_contract = lambda evidence: wake.bone_contract(evidence, CONTRACT_MODEL)
    return EVIDENCE / 'marker'


def native_environment():
    vehicle.TEXTURE_ROOT = TEXTURE_ROOT
    pattern = vehicle.native_environment()
    hook = pattern.prepare_textures
    pattern.prepare_textures = lambda evidence, out, resource_root=None: hook(evidence, out)
    return pattern


def native(first, last):
    configure()
    pattern = native_environment()
    root = EVIDENCE / 'material'
    root.mkdir(parents=True, exist_ok=True)
    drv.exact_texture_reuse(pattern, root / 'native')
    excluded, occurrences = [], []
    folder = EVIDENCE / 'marker' / 'stages' / CLIP
    module_inputs = source.read(folder / 'source_module_inputs.json')['records']
    for occurrence in source.read(folder / 'source_occurrences.json'):
        reason = drv.native_occurrence_excluded(occurrence, module_inputs)
        if reason:
            excluded.append(dict(elementId=occurrence['elementId'], sourceEmitter=occurrence['sourceEmitter'],
                                 rendererShape=occurrence['rendererShape'], reason=reason))
            continue
        occurrences.append(occurrence)
    records = dict(module_inputs)
    records[wake.REQUIRED_MODULE] = dict(fullPath=wake.REQUIRED_MODULE, classPath='engine.particlemodulerequired',
                                         archetypeFullPath=None, properties={})
    defaults = {r['fullPath']: r for r in source.read(folder / 'source_class_defaults.json')['records']}
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
    configure()
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
    raise AssertionError(('Anchor marker source mesh is not installed', source_mesh))


def ensure_meshes(evidence):
    vehicle.MESH_ROOT = MESH_ROOT
    folder = evidence / 'stages' / CLIP
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


def bind_world_root(document):
    """The marker is a Level-owned world root: every element leaves the bone
    follow of the shared contract and becomes a root snapshot with identity
    socket and basis. Returns the number rebound; repeating it is a no-op."""
    count = 0
    for element in document['elements']:
        attachment = element['actionCueAttachment']
        if not attachment['enabled']:
            continue
        attachment.update(follow=False, sourceAnchorSlotId='root', runtimeAnchorSlotId='root', runtimeBoneName='',
                          snapshotRootSourceBasisYawDegrees=0)
        attachment['socketLocalTransform'] = dict(position=[0.0, 0.0, 0.0], rotationDegrees=[0.0, 0.0, 0.0],
                                                  scale=[1.0, 1.0, 1.0])
        count += 1
    return count


def project(install):
    reviewed = EVIDENCE / 'material' / 'reviewed'
    deferred_path = reviewed / 'native_deferred_programs.json'
    deferred_rows = source.read(deferred_path) if deferred_path.is_file() else []
    installed = drv.installed_native_materials()
    evidence = configure()
    drv.mesh_asset = mesh_asset
    if reviewed.is_dir():
        shutil.copytree(reviewed, evidence / 'material' / 'reviewed', dirs_exist_ok=True)
    ensure_meshes(evidence)
    stage = evidence / 'stages' / CLIP / 'source_occurrences.json'
    deferred = {o['elementId']: dict(reason=row['reason'], program=row['program'])
                for row in deferred_rows for o in source.read(stage) if o['sourceMaterial'] == row['material']}
    document, row = drv.project_clip(evidence, CLIP, installed, deferred)
    document['displayName'] = 'Bern anchor marker'
    document['particleSystem']['uniformScaleMultiplier'] = MARKER_SCALE
    row['worldRootElements'] = bind_world_root(document)
    writes = [(AUTHORED / (document['effectAssetId'] + '.effect.json'),
               (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8'))]
    catalog = source.read(CATALOG)
    catalog['effects'] = [r for r in catalog['effects'] if not r['effectAssetId'].startswith(ASSET_PREFIX)]
    catalog['effects'].append(dict(effectAssetId=document['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT',
        authoringPath='Effects/Authored/' + document['effectAssetId'] + '.effect.json'))
    writes.append((CATALOG, (json.dumps(catalog, ensure_ascii=False, indent=2) + '\n').encode('utf8')))
    changed = []
    for path, payload in writes:
        if not path.exists() or path.read_bytes() != payload:
            changed.append(path.relative_to(ROOT).as_posix())
            if install:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(payload)
    source.write(EVIDENCE / 'projection_installation.json', dict(installed=install, changedPaths=changed, documents=[row]))
    print(json.dumps(row, ensure_ascii=False, indent=1)[:3000])
    print(('installed' if install else 'candidate') + ' files changed', len(changed))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--acquire', action='store_true')
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--install-native', action='store_true')
    parser.add_argument('--native-first', type=int, default=NATIVE_FIRST)
    parser.add_argument('--native-last', type=int, default=NATIVE_LAST)
    parser.add_argument('--project', action='store_true')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    if args.acquire:
        path, system, bone = synthetic_actions(EVIDENCE)
        print('system', system, 'bone', bone)
        folder = configure()
        folder.mkdir(parents=True, exist_ok=True)
        drv.acquire(folder)
    if args.native:
        native(args.native_first, args.native_last)
    if args.install_native:
        install_native(args.native_first, args.native_last)
    if args.project:
        project(args.install)


if __name__ == '__main__':
    main()
