"""Restore the Waterpang water-gun muzzle effects from their source actions.

The Maharaka Waterpang water gun is the carried prop ITR_02164 ("Water Pro
MK-1", EFTable_Prop 15000, CommonAction 54015 "rifle carry"). Its player
actions live in XmlData/Action/GADGET.loa; the "[Maharaka]" set fires on the
gun prop clips pr_itr_02164_att_1..4_01:

  56932 single shot     Att_1_01  FX_BS_07.Gadget.Par_L_WaterGun01_Sk_03
  56902 burst           Att_2_01  FX_BS_07.Gadget.Par_L_WaterGun01_Sk_01
  57032 area shot       Att_3_01  FX_BS_07.Gadget.Par_L_WaterGun02_Sk_03
  57002 one-shot        Att_4_01  FX_BS_07.Gadget.Par_L_WaterGun02_Sk_01

Every particle notify follows the gun's socket FX_Prj_01 (ITR_02164 b_root +
(60, 0, 3) cm). The gun rides the player's bip001-prop3 with a zero socket
(SC_Prop3_01 on all seven class bodies), so the gun b_root IS bip001-prop3.

Acquisition, native material recovery, installation and projection reuse the
Inanna/vehicle cohort drivers exactly as the Waterpang hazard builder does;
this module owns its evidence root, program range and the attachment rebind:

* runtime bone b_root -> bip001-prop3;
* the shared socket contract writes the socket in the vehicle/NPC Y-mirrored
  bone basis, while player bones are Z-mirrored (measured PSA->WModel rule
  position (px, py, -pz)). The two differ by 180 deg about X, so the socket
  becomes position (0.60, 0, -0.03) m with rotation (180, 0, 0).
"""
from pathlib import Path
import argparse
import collections
import json
import shutil
import sys

ROOT = Path(__file__).resolve().parents[2]
for extra in ('Tools/EffectPipeline', 'Tools/LevelPlacementExtractor',
              'Tools/VehiclePipeline', 'Tools/ModelAssetConverter'):
    sys.path.insert(0, str(ROOT / extra))
import build_esther_inanna_source_effects as drv
import build_kouku_gate1_full_restore as source
import build_vehicle_skill_effects as vehicle

# An ASCII evidence root: the Assimp geometry cook cannot open non-ASCII paths.
EVIDENCE = Path('C:/LostArkExtract/MaharakaWaterGunFX20260928')
UMODEL = Path('C:/LostArkExtract/umodel/umodel_lostark_v7.exe')
# The extracted GADGET.loa is copied under the evidence root by --stage-source.
ACTION_LOA = EVIDENCE / 'source' / 'GADGET.loa'
TEXTURE_ROOT = 'Effect/Maharaka/WaterGun/Textures'
MESH_ROOT = 'Effect/Maharaka/WaterGun/Meshes'
MESH_ROOTS = (MESH_ROOT, 'Effect/Maharaka/Waterpang/Meshes', 'Effect/Esther/Inanna/Meshes',
              'Effect/Esther/Thirain/Meshes', 'Effect/Esther/Wei/Meshes', 'Effect/Esther/Ninave/Meshes',
              'Effect/KoukuSaydon/FullRestore/Meshes')
# Group 4928 already owns its Mesh/Particle wrappers and runtime rows
# (4939..4991 free when this cohort was allocated): no shader file is added.
NATIVE_FIRST, NATIVE_LAST = 4940, 4979
ASSET_PREFIX = 'effect.maharaka.watergun.'
# Socket bones are checked against an installed player body; every class body
# carries b_root and bip001-prop3.
PLAYER_MODEL = 'Character/LanceMaster/LanceMaster.wmodel'
PLAYER_PROP_BONE = 'bip001-prop3'
# FX_Prj_01 in the player Z-mirrored prop-bone basis (see module docstring).
MUZZLE_POSITION = [0.6, 0.0, -0.03]
MUZZLE_ROTATION = [180.0, 0.0, 0.0]

PROFILES = {
    'att_1': dict(action=56932, clips={'watergun_att_1': (0, 'PR_ITR_02164_Att_1_01')}),
    'att_2': dict(action=56902, clips={'watergun_att_2': (0, 'PR_ITR_02164_Att_2_01')}),
    'att_3': dict(action=57032, clips={'watergun_att_3': (0, 'PR_ITR_02164_Att_3_01')}),
    'att_4': dict(action=57002, clips={'watergun_att_4': (0, 'PR_ITR_02164_Att_4_01')}),
}
REQUIRED_MODULE = 'engine.default__particlemodulerequired'


def configure(name, evidence_root=EVIDENCE):
    profile = PROFILES[name]
    vehicle.UMODEL = UMODEL
    drv.PROFILE = 'chain'
    drv.ARCHETYPE = 'PLAYER_WATERGUN'
    drv.ASSET_PREFIX = ASSET_PREFIX
    drv.ACTION_ID = profile['action']
    drv.ACTION_PROFILE = 'GADGET'
    drv.ACTION_LOA = ACTION_LOA
    drv.CLIPS = profile['clips']
    drv.SOURCE_MESH = ('ITR_02164', 'mesh.itr_02164_sk')
    drv.NPC_MODEL = PLAYER_MODEL
    drv.TEXTURE_ROOT = TEXTURE_ROOT
    drv.MESH_ROOTS = MESH_ROOTS
    drv.zone_actions = plain_actions
    return evidence_root / name


def plain_actions(evidence):
    actions = evidence / 'actions' / f'{drv.ACTION_PROFILE}.action-effects.json'
    if not actions.is_file():
        actions = drv.extract_actions(evidence)
    return actions


def native_environment():
    vehicle.TEXTURE_ROOT = TEXTURE_ROOT
    pattern = vehicle.native_environment()
    hook = pattern.prepare_textures
    pattern.prepare_textures = lambda evidence, out, resource_root=None: hook(evidence, out)
    return pattern


def native(first, last):
    configure('att_1')
    pattern = native_environment()
    root = EVIDENCE / 'material'
    root.mkdir(parents=True, exist_ok=True)
    drv.exact_texture_reuse(pattern, root / 'native')
    excluded, occurrences, records, defaults, seen = [], [], {}, {}, set()
    for name, profile in PROFILES.items():
        for clip in profile['clips']:
            folder = EVIDENCE / name / 'stages' / clip
            module_inputs = source.read(folder / 'source_module_inputs.json')['records']
            for occurrence in source.read(folder / 'source_occurrences.json'):
                if occurrence['elementId'] in seen:
                    continue
                seen.add(occurrence['elementId'])
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
    configure('att_1')
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
    raise AssertionError(('Water-gun source mesh is not installed', source_mesh))


def ensure_meshes(evidence):
    vehicle.MESH_ROOT = MESH_ROOT
    for clip in drv.CLIPS:
        folder = evidence / 'stages' / clip
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


def bind_player_prop(document):
    """Move every gun-socket follower onto the player's prop bone. Returns the
    (followers, snapshots) counts; repeating it is a no-op."""
    followers = snapshots = 0
    for element in document['elements']:
        attachment = element['actionCueAttachment']
        if not attachment['enabled']:
            continue
        if not attachment['follow']:
            snapshots += 1
            continue
        assert attachment['runtimeBoneName'] in ('b_root', PLAYER_PROP_BONE), attachment
        assert attachment['runtimeAnchorSlotId'].casefold() == 'fx_prj_01', attachment
        attachment['runtimeBoneName'] = PLAYER_PROP_BONE
        socket = attachment['socketLocalTransform']
        socket.update(position=list(MUZZLE_POSITION), rotationDegrees=list(MUZZLE_ROTATION))
        assert socket['scale'] == [1.0, 1.0, 1.0], socket
        followers += 1
    return followers, snapshots


AUTHORED = ROOT / 'Data/Effects/Authored'
CATALOG = ROOT / 'Data/Effects/EffectCatalog.json'
RESOURCES = ROOT / 'Client/Bin/Resources'


def project(install):
    reviewed = EVIDENCE / 'material' / 'reviewed'
    deferred_path = reviewed / 'native_deferred_programs.json'
    deferred = {identity: row for row in (source.read(deferred_path) if deferred_path.is_file() else [])
                for identity in row.get('occurrences', [])}
    installed = drv.installed_native_materials()
    documents, rows = [], []
    for name, profile in PROFILES.items():
        evidence = configure(name)
        drv.mesh_asset = mesh_asset
        shutil.copytree(reviewed, evidence / 'material' / 'reviewed', dirs_exist_ok=True)
        ensure_meshes(evidence)
        for clip in profile['clips']:
            document, row = drv.project_clip(evidence, clip, installed, deferred)
            document['displayName'] = 'Waterpang water gun ' + clip
            row['attachmentRebind'] = bind_player_prop(document)
            documents.append(document)
            rows.append(row)
    writes = [(AUTHORED / (d['effectAssetId'] + '.effect.json'),
               (json.dumps(d, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8')) for d in documents]
    catalog = source.read(CATALOG)
    catalog['effects'] = [row for row in catalog['effects'] if not row['effectAssetId'].startswith(ASSET_PREFIX)]
    catalog['effects'] += [dict(effectAssetId=d['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT',
                                authoringPath='Effects/Authored/' + d['effectAssetId'] + '.effect.json') for d in documents]
    writes.append((CATALOG, (json.dumps(catalog, ensure_ascii=False, indent=2) + '\n').encode('utf8')))
    changed = []
    for path, payload in writes:
        if not path.exists() or path.read_bytes() != payload:
            changed.append(path.relative_to(ROOT).as_posix())
            if install:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(payload)
    source.write(EVIDENCE / 'projection_installation.json', dict(installed=install, changedPaths=changed, documents=rows))
    for row in rows:
        print('%-50s elements=%-4d deferred=%-3d duration=%dms programs=%d attachments=%s rebind=%s' % (
            row['effectAssetId'], row['elementCount'], len(row['deferredEmitters']), row['durationMs'],
            len(row['nativePrograms']), row['attachments'], row['attachmentRebind']))
    print(('installed' if install else 'candidate') + ' files changed', len(changed))


def stage_source(gadget):
    ACTION_LOA.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(gadget, ACTION_LOA)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--stage-source', type=Path, help='extracted XmlData/Action/GADGET.loa')
    parser.add_argument('--profile', choices=sorted(PROFILES))
    parser.add_argument('--acquire', action='store_true')
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--install-native', action='store_true')
    parser.add_argument('--native-first', type=int, default=NATIVE_FIRST)
    parser.add_argument('--native-last', type=int, default=NATIVE_LAST)
    parser.add_argument('--project', action='store_true')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    if args.stage_source:
        stage_source(args.stage_source)
    if args.acquire:
        drv.acquire(configure(args.profile))
    if args.native:
        native(args.native_first, args.native_last)
    if args.install_native:
        install_native(args.native_first, args.native_last)
    if args.project:
        project(args.install)


if __name__ == '__main__':
    main()
