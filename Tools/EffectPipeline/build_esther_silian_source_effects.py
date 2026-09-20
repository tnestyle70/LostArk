"""Restore the Esther Silian strike from its own source Action notifies.

NP_LRSA_00.loa action 542600 plays the strike on two clips (stage 0
Att_Battle_7_01, stage 2 Evt1_SK_SwordofChampion_BK). Each clip becomes one
full-restore document whose elements carry the original notify time, local
transform, snapshot-root basis or R_Weapon socket follow; the NPC cue document
only names which document starts with which clip.

Source acquisition, native material recovery and document projection are the
Kouku pattern pipeline, driven from the installed game packages the same way
the vehicle skill effects were. This driver owns only its evidence root, the
Silian program range, the socket contract and the two authored documents.
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

ARCHETYPE = 'NPC_59030'
ASSET_PREFIX = 'effect.esther.silian.'
ACTION_ID = 542600
ACTION_LOA = Path('C:/Users/95jus/Downloads/SourceData/SourceData/LPK/data3/EFGame_Extra/ClientData/XmlData/Action/NP_LRSA_00.loa')
# runtime clip -> (source stage index, label)
CLIPS = {
    'npc_evt1_sk_swordofchampion_bk': (2, 'SwordofChampion'),
    'npc_att_battle_7_01': (0, 'Att_Battle_7_01'),
}
SOURCE_MESH = ('np_lrsa_00', 'mesh.np_lrsa_00_sk')
NPC_MODEL = 'Character/NPC/Npc_59030/Npc_59030.wmodel'
# The Kouku native range ends at 3967; 3912.. is the first unused block after
# the vehicle cohorts. Exact vehicle programs are reused by ID, not re-lowered.
NATIVE_FIRST, NATIVE_LAST = 3912, 3967
VEHICLE_REVIEWED = ROOT / 'out/VehicleSkillEffects20260914/material/reviewed'
TEXTURE_ROOT = 'Effect/Esther/Silian/FullRestore/Textures'
# Every Silian source mesh is already installed there with the x100 cook the
# projector's modelPreScale 0.01 expects.
MESH_ROOT = 'Effect/Esther/Thirain/Meshes'
AUTHORED = ROOT / 'Data/Effects/Authored'
RESOURCES = ROOT / 'Client/Bin/Resources'
CATALOG = ROOT / 'Data/Effects/EffectCatalog.json'
CUE_DOCUMENT = ROOT / 'Data/Effects/NpcActionCues' / (ARCHETYPE + '.npcactioncues.json')


def asset_id(clip):
    return ASSET_PREFIX + clip + '.full.restore'


def extract_actions(evidence):
    output = evidence / 'actions'
    subprocess.run([sys.executable, str(ROOT / 'Tools/LevelPlacementExtractor/extract_action_effect_notifies.py'),
        '--source', f'NP_LRSA_00={ACTION_LOA}', '--output', str(output), '--action-id', str(ACTION_ID)], check=True)
    return output / 'NP_LRSA_00.action-effects.json'


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


def acquire_stage(evidence, clip):
    """One selected source stage through the shared Kouku acquisition."""
    vehicle.native_environment()
    stage, label = CLIPS[clip]
    folder = evidence / 'stages' / clip
    folder.mkdir(parents=True, exist_ok=True)
    source.write(folder / 'source_socket_contract.json', socket_contract(evidence))
    actions = evidence / 'actions' / 'NP_LRSA_00.action-effects.json'
    if not actions.is_file():
        actions = extract_actions(evidence)
    source.ACTION = actions
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

    def reuse(selected, roots):
        fresh, reused = original(selected, roots)
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

    def prepare_distortion(out_dir, merged, first):
        reused = {p['program'] for p in source.read(out_dir / 'reused_native_programs.json')['programs']}
        original_distortion(out_dir, dict(merged, programs=[p for p in merged['programs'] if p['program'] not in reused]), first)
    pattern.prepare_distortion = prepare_distortion

    import install_kouku_gate1_native_materials as tables
    original_install = tables.install

    def install(contract_path, evidence, header_path):
        contract = source.read(contract_path)
        if contract.get('deferredPrograms') and header_path.parent == out:
            complete = contract_path.with_name(contract_path.stem + '.complete.json')
            source.write(complete, dict(contract, deferredPrograms=[]))
            contract_path = complete
        return original_install(contract_path, evidence, header_path)
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
    # kept, and its re-lowered ones disagree with the vehicle root by design.
    reuse = [next(path for path in (root / 'reviewed', VEHICLE_REVIEWED)
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
    asset = f"{MESH_ROOT}/{package_name.upper()}/{relative.rsplit('.', 1)[-1]}.wmodel"
    assert (RESOURCES / asset).is_file(), ('Silian source mesh is not installed', source_mesh, asset)
    return asset


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
    document.update(effectAssetId=asset_id(clip), displayName='Esther Silian ' + CLIPS[clip][1])
    duplicates = []
    for element in document['elements']:
        occurrence = by_id[element['id']]
        if occurrence['kind'] != 'light':
            material = natives.get(element['id'])
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
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/SilianFX20260920')
    parser.add_argument('--acquire', action='store_true')
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--native-first', type=int, default=NATIVE_FIRST)
    parser.add_argument('--native-last', type=int, default=NATIVE_LAST)
    parser.add_argument('--install-native', action='store_true')
    parser.add_argument('--project', action='store_true')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    evidence = args.evidence_root.resolve()
    if args.acquire:
        acquire(evidence)
    if args.native:
        native(evidence, args.native_first, args.native_last)
    if args.install_native:
        vehicle.install_native(evidence)
    if args.project:
        project(evidence, args.install)


if __name__ == '__main__':
    main()
