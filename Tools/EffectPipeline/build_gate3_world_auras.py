"""Recover Gate 3 preparation auras from original DeployData/LookInfo references.

Uses the existing Cascade importer, material-map decoder and native generator.
Generated candidates stay under out until the separate checked installation.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import json
import subprocess
import sys

import build_kouku_gate1_full_restore as source
import build_kouku_pattern_native as native

ROOT = source.ROOT
OUT = ROOT / 'out/Gate3Aura20260922'
GAME = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC')
TOOL = ROOT / 'out/GuardianEffects20260922/tool/umodel_lostark_v7.exe'
TARGETS = {
    'effect.world.entry_aura': ('bfx_low_01.etc.par_g_itr_t_ef_on', '월드 | 진입오라'),
    'effect.world.entry_aura.active': ('bfx_low_01.etc.par_g_itr_t_ef_gooff', '월드 | 진입오라 · 활성'),
    'effect.world.respawn_aura': ('fx_itr_10175.par_g_waiting_01', '월드 | 리스폰오라'),
}


class Packages(dict):
    def __missing__(self, key):
        scripts = {'core': '9L6NC53E9WINO5FELWUN0.u', 'engine': 'NE1FENCQ4UNE9ZPRENOQS.u',
                   'efgame': 'NU1V7NCQ4YAE9ZPJVNOQS.u'}
        path = GAME / scripts[key] if key in scripts else source.ue3.resolve_physical_package(TOOL, GAME, key, 'kr')
        assert path.is_file(), path
        self[key] = path
        return path


PACKAGES = Packages()


def acquire():
    loaded, records, effective, references = {}, {}, {}, {}

    def record(key):
        if key not in records:
            package_name, relative = key.split('.', 1)
            if package_name not in loaded:
                loaded[package_name] = source.load_package(PACKAGES[package_name], source.ue3.LOSTARK_KR_AES_KEY)
            package = loaded[package_name]
            records[key] = source.record_from_export(package, package_name, source.find_export(package, relative))
        return records[key]

    def resolve(key, stack=()):
        if key in effective:
            return effective[key]
        assert key not in stack, key
        row = record(key)
        parent = row.get('archetypeFullPath')
        class_default = row['classPath'].split('.')[0] + '.default__' + row['classPath'].split('.')[-1]
        if not parent and key != class_default:
            parent = class_default
        effective[key] = source.merge(resolve(parent, stack + (key,)) if parent else {}, source.norm(row['properties']))
        roots = {k.lower() for k in row['properties']}
        references[key] = [copy.deepcopy(r) for r in references.get(parent, [])
            if r['property'].split('.')[0].split('[')[0].lower() not in roots] + copy.deepcopy(row['references'])
        return effective[key]

    selected = set()

    def collect(key):
        if key in selected:
            return
        selected.add(key)
        resolve(key)
        for ref in references[key]:
            path = ref['objectPath']
            if path.rsplit('.', 1)[-1].startswith(('particle', 'efparticle', 'distribution', 'efdistribution', 'pointlight', 'lightcomponent', 'default__')):
                collect(path)

    for system, _ in TARGETS.values():
        collect(system)
    imported = source.imported
    index = imported.SourceIndex({}, {})
    for key, props in effective.items():
        obj = imported.SourceObject(key, key, records[key]['className'], key, props)
        for ref in references[key]:
            prop, path = ref['property'].lower(), ref['objectPath']
            obj.reference_paths.append((prop, path))
            if path in effective:
                obj.references.append((prop, path))
        index.objects[key] = obj
        index.by_source_id[key] = obj
    occurrences = []
    for asset, (system, _) in TARGETS.items():
        for prop, emitter_key in index.objects[system].references:
            if prop != 'emitters':
                continue
            lod = next(index.objects[k] for p, k in index.objects[emitter_key].references if p.startswith('lodlevels'))
            if not imported.prop(lod.properties, 'benabled', True):
                continue
            modules = [index.objects[k] for p, k in lod.references if p in ('requiredmodule', 'modules', 'typedatamodule', 'spawnmodule')]
            kind, _, shape = imported.classify({'sourceSystemId': system}, modules)
            required = next(m for m in modules if m.class_name == 'particlemodulerequired')
            material = next(path for prop, path in required.reference_paths if prop == 'material')
            mesh = next((path for m in modules for prop, path in m.reference_paths if prop == 'mesh'), '')
            occurrences.append(dict(elementId=asset + '.' + emitter_key.rsplit('.', 1)[-1], effectAssetId=asset,
                actionId=asset, sourceNotify=asset, sourceType='WorldAura', sourceSystem=system,
                sourceEmitter=emitter_key, sourceLOD=lod.key, sourceMaterial=material, rendererShape=shape,
                kind=kind, sourceMesh=mesh, moduleOrder=[m.key for m in modules],
                sourceTimeSeconds=0, sourceDurationSeconds=10))
    source.write(OUT / 'source_occurrences.json', occurrences)
    source.write(OUT / 'source_module_inputs.json', {'records': {key: records[key] for key in selected}})
    source.write(OUT / 'source_class_defaults.json', {'records': [r for key, r in records.items() if key not in selected]})
    source.write(OUT / 'packages.json', {k: str(v) for k, v in PACKAGES.items()})
    return index, occurrences, records


def materials():
    # The shared decoder closes absent CB0 and engine-only texture programs as
    # well as ordinary material arrays. It does not modify installed shaders.
    native.UMODEL = TOOL
    native.RELEASE = GAME
    native.source_native.UMODEL = TOOL
    native.source_native.packages = PACKAGES
    native.prepare(OUT, 4416, 4447)


def prepare_geometry():
    sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
    import cook_wmodel_geometry_contract as geometry
    resources = ROOT / 'Client/Bin/Resources'
    converter = ROOT / 'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe'
    digest = lambda path: hashlib.sha256(path.read_bytes()).digest()
    rows = []
    for key in sorted({row['sourceMesh'] for row in source.read(OUT / 'source_occurrences.json') if row['sourceMesh']}):
        package, relative = key.split('.', 1)
        name = relative.rsplit('.', 1)[-1]
        destination = resources / 'Effect/World/Meshes' / (name + '.wmodel')
        destination.parent.mkdir(parents=True, exist_ok=True)
        existing = resources / 'Effect/KoukuSaydon/FullRestore/Meshes' / (name + '.wmodel')
        if existing.exists():
            payload = existing.read_bytes()
            mode = 'REUSE_SOURCE_GEOMETRY'
        else:
            output = OUT / 'mesh-export' / package
            output.mkdir(parents=True, exist_ok=True)
            if not list(output.rglob(name + '.gltf')):
                command = [str(TOOL), '-export', '-game=lostark', '-kr', '-nameresolve',
                    f'-path={GAME}', f'-out={output}', '-dds', '-gltf', '-nooverwrite', f'-obj={name}', package]
                completed = subprocess.run(command, capture_output=True, text=True, encoding='utf8',
                    errors='replace', creationflags=subprocess.CREATE_NO_WINDOW)
                (output / (name + '.log')).write_text(completed.stdout + completed.stderr, encoding='utf8')
                assert completed.returncode == 0, completed.stdout
            gltf, = output.rglob(name + '.gltf')
            legacy = output / (name + '.wmodel')
            if not legacy.exists():
                subprocess.run([str(converter), str(gltf), '-o', str(legacy), '--scale', '100',
                    '--no-auto-textures'], check=True, capture_output=True)
            observation = output / (name + '.observation.json')
            source.write(observation, dict(sourceObject=key, gltf=str(gltf), gltfSha256=digest(gltf).hex(),
                legacy=str(legacy), legacySha256=digest(legacy).hex()))
            provenance = geometry.GeometryProvenanceEvidence(key, digest(OUT / 'source_occurrences.json'),
                'OBSERVED_SOURCE_RECEIPT', digest(PACKAGES[package]), digest(converter), digest(observation), digest(observation))
            payload, receipt = geometry.cook_wmodel_geometry_contract(gltf, legacy, provenance)
            source.write(output / (name + '.cook.json'), receipt)
            mode = 'SOURCE_GLTF_WMODEL_GEOMETRY_PARITY'
        if not destination.exists():
            destination.write_bytes(payload)
        # An existing owned resource is never silently replaced during source acquisition.
        decoded = geometry.parse_geometry_wmodel(destination.read_bytes())
        assert decoded['submeshes'], destination
        rows.append(dict(sourceObject=key, assetId=destination.relative_to(resources).as_posix(), mode=mode,
            sha256=digest(destination).hex()))
    source.write(OUT / 'geometry_installation.json', rows)


def project():
    index, occurrences, records = acquire()
    source.SELECTED = {asset: ([], label) for asset, (_, label) in TARGETS.items()}
    notifies = [dict(notifyId=asset, sourceType='WorldAura', cue={}, actionId=asset) for asset in TARGETS]
    destination = OUT / 'projected'
    source.project(OUT, index, notifies, occurrences, records, destination)
    patch = OUT / 'native/native_material_patch.json'
    native_materials = {key: row['material'] for row in source.read(patch)['programs'] for key in row['occurrences']}
    by_element = {row['elementId']: row for row in occurrences}
    for asset, (system, label) in TARGETS.items():
        original = destination / ('effect.kouku.gate1.' + asset + '.full.restore.effect.json')
        document = source.read(original)
        document.update(effectAssetId=asset, displayName=label)
        for element in document['elements']:
            element['groupId'] = asset
            element['actionCueAttachment']['enabled'] = False
            element['material'] = copy.deepcopy(native_materials[element['id']])
            element['detail']['uv'].update(start=[0, 0], speed=[0, 0], wave=False, sequence=False)
            element['detail']['color']['emissiveIntensity'] = 1
            if element['sourceRecipe']['rendererShape'] == 'mesh':
                element['detail']['mesh']['useModelMaterial'] = False
                typed = next(index.objects[key] for key in by_element[element['id']]['moduleOrder']
                    if index.objects[key].class_name == 'particlemoduletypedatamesh')
                assert source.imported.prop(typed.properties, 'boverridematerial', False), typed.key
                element['detail']['mesh']['sourceTypeDataRotationDegrees'] = [
                    float(source.imported.prop(typed.properties, field, 0)) for field in ('roll', 'pitch', 'yaw')]
                for resource in element['resources']:
                    resource['assetId'] = resource['assetId'].replace('Effect/KoukuSaydon/FullRestore/Meshes/', 'Effect/World/Meshes/')
            for resource in element['resources'] + element['material']['sourceProfile'].get('textures', []):
                assert (ROOT / 'Client/Bin/Resources' / resource['assetId']).is_file(), resource
        source.write(OUT / 'candidate' / (asset + '.effect.json'), document)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--stage', choices=('source', 'materials', 'geometry', 'project'), default='source')
    parser.add_argument('--evidence-root', type=Path, default=OUT)
    parser.add_argument('--game-release', type=Path, default=GAME)
    parser.add_argument('--umodel', type=Path, default=TOOL)
    args = parser.parse_args()
    OUT, GAME, TOOL = args.evidence_root.resolve(), args.game_release.resolve(), args.umodel.resolve()
    {'source': acquire, 'materials': materials, 'geometry': prepare_geometry, 'project': project}[args.stage]()
