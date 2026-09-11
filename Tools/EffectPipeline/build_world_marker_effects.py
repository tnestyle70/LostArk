"""Build the two World marker V1 documents from the installed source packages.

Reuses the Cascade importer and native material contracts. Source packages and
existing authored effects are read-only; Resources payloads remain Git ignored.
"""
from pathlib import Path
import argparse
import collections
import copy
import hashlib
import subprocess
import sys

import build_kouku_gate1_full_restore as source

ROOT = source.ROOT
IMPORTED = source.imported
EVIDENCE = ROOT / 'out/WorldMarkers20260911'
TARGETS = {
    'effect.world.mouse_click': ('fx_bs_03.mark.par_b_picking_01', 'Mouse Click', 1.2),
    'effect.world.move_destination': ('fx_bs_03.mark.par_i_movetrack_01', 'Move Destination', 7.0),
}


def acquire(evidence=EVIDENCE):
    packages = {p.parent.name.lower(): p for p in
                (source.SOURCE / 'CanonicalSource/Effect/Closure/SourcePackages').glob('*/*.upk')}
    for domain in ('Effect', 'Shared'):
        for receipt_path in (source.SOURCE / 'CanonicalSource' / domain / 'Packages').glob('*/source.receipt.json'):
            receipt = source.read(receipt_path)
            packages[receipt['logicalPackage'].lower()] = receipt_path.parent / receipt['outputs'][0]['path']
    packages['core'] = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/9L6NC53E9WINO5FELWUN0.u')
    loaded, records, effective, references = {}, {}, {}, {}

    def record(key):
        if key not in records:
            package_name, relative = key.split('.', 1)
            if package_name not in loaded:
                loaded[package_name] = source.load_package(packages[package_name], source.ue3.LOSTARK_KR_AES_KEY)
            package = loaded[package_name]
            records[key] = source.record_from_export(package, package_name, source.find_export(package, relative))
        return records[key]

    def resolve(key, stack=()):
        if key in effective:
            return effective[key]
        assert key not in stack, ('source archetype cycle', key)
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
        row = record(key)
        selected.add(key)
        resolve(key)
        for ref in references[key]:
            path = ref['objectPath']
            name = path.rsplit('.', 1)[-1]
            if name.startswith(('particle', 'efparticle', 'distribution', 'efdistribution', 'pointlight', 'lightcomponent', 'default__')):
                collect(path)

    for system, _, _ in TARGETS.values():
        collect(system)
    index = IMPORTED.SourceIndex({}, {})
    for key in effective:
        row = records[key]
        obj = IMPORTED.SourceObject(key, key, row['className'], key, effective[key])
        for ref in references[key]:
            prop, path = ref['property'].lower(), ref['objectPath']
            obj.reference_paths.append((prop, path))
            if path in effective:
                obj.references.append((prop, path))
        index.objects[key] = obj
        index.by_source_id[key] = obj
    occurrences = []
    for asset, (system, _, duration) in TARGETS.items():
        for prop, emitter_key in index.objects[system].references:
            if prop != 'emitters':
                continue
            emitter = index.objects[emitter_key]
            lod = next(index.objects[k] for p, k in emitter.references if p.startswith('lodlevels'))
            modules = [index.objects[k] for p, k in lod.references if p in ('requiredmodule', 'modules', 'typedatamodule', 'spawnmodule')]
            kind, _, shape = IMPORTED.classify({'sourceSystemId': system}, modules)
            required = next(m for m in modules if m.class_name == 'particlemodulerequired')
            material = next(path for prop, path in required.reference_paths if prop == 'material')
            overrides = [path for m in modules if m.class_name == 'particlemodulemeshmaterial'
                         for prop, path in m.reference_paths if prop == 'meshmaterials']
            if overrides:
                assert len(overrides) == 1
                material = overrides[0]
            mesh = next((path for m in modules for prop, path in m.reference_paths if prop == 'mesh'), '')
            element_id = asset + '.' + emitter_key.rsplit('.', 1)[-1]
            occurrences.append(dict(elementId=element_id, effectAssetId=asset, actionId=asset,
                sourceNotify=asset, sourceType='WorldMarker', sourceSystem=system,
                sourceEmitter=emitter_key, sourceLOD=lod.key, sourceMaterial=material,
                rendererShape=shape, kind=kind, sourceMesh=mesh,
                moduleOrder=[m.key for m in modules], sourceTimeSeconds=0,
                sourceDurationSeconds=duration))
    source.write(evidence / 'source_occurrences.json', occurrences)
    source.write(evidence / 'source_module_inputs.json', {'records': {key: records[key] for key in selected}})
    source.write(evidence / 'source_class_defaults.json', {'records': [r for key, r in records.items() if key not in selected]})
    source.write(evidence / 'source_closure.json', dict(packages={k: str(p.path) for k, p in loaded.items()},
        objectCount=len(records), occurrenceCount=len(occurrences), manualVisualValidation='USER_PENDING'))
    return index, occurrences, records


def project(evidence, material_patch):
    index, occurrences, records = acquire(evidence)
    source.SELECTED = {asset: ([], label) for asset, (_, label, _) in TARGETS.items()}
    notifies = [dict(notifyId=asset, sourceType='WorldMarker', cue={}, actionId=asset)
                for asset in TARGETS]
    destination = evidence / 'projected'
    source.project(evidence, index, notifies, occurrences, records, destination)
    native = {key: p['material'] for p in source.read(material_patch)['programs'] for key in p['occurrences']}
    by_element = {o['elementId']: o for o in occurrences}
    for asset, (system, label, _) in TARGETS.items():
        path = destination / ('effect.kouku.gate1.' + asset + '.full.restore.effect.json')
        doc = source.read(path)
        doc.update(effectAssetId=asset, displayName=label)
        for element in doc['elements']:
            element['groupId'] = asset
            element['actionCueAttachment']['enabled'] = False
            element['material'] = copy.deepcopy(native[element['id']])
            element['detail']['uv'].update(start=[0, 0], speed=[0, 0], wave=False, sequence=False)
            element['detail']['color']['emissiveIntensity'] = 1
            required = next(index.objects[key] for key in by_element[element['id']]['moduleOrder']
                            if index.objects[key].class_name == 'particlemodulerequired')
            # UE3's omitted integer field retains zero. The generic importer
            # uses a one-shot authoring default; World markers preserve the
            # original unbounded emitter loops and the three explicit bursts.
            element['sourceRecipe']['emitterLoopCount'] = IMPORTED.prop(required.properties, 'emitterloops', 0)
            if element['sourceRecipe']['rendererShape'] == 'mesh':
                element['detail']['mesh']['useModelMaterial'] = False
                typed = next(index.objects[key] for key in by_element[element['id']]['moduleOrder']
                             if index.objects[key].class_name == 'particlemoduletypedatamesh')
                element['detail']['mesh']['sourceTypeDataRotationDegrees'] = [
                    float(IMPORTED.prop(typed.properties, field, 0)) for field in ('roll', 'pitch', 'yaw')]
                material_modules = [index.objects[key] for key in by_element[element['id']]['moduleOrder']
                                    if index.objects[key].class_name == 'particlemodulemeshmaterial']
                if material_modules:
                    assert len(material_modules) == 1
                    material_module = material_modules[0]
                    # The source is a single entry array. Preserve the Required
                    # material as inactive evidence and bind its actual slot
                    # through the existing CModel source material slot path.
                    assert len(IMPORTED.prop(material_module.properties, 'meshmaterials')) == 1
                    assert not IMPORTED.prop(typed.properties, 'boverridematerial', False)
                    slot_material = element['material']
                    assert [path for prop, path in material_module.reference_paths if prop == 'meshmaterials'] == [slot_material['sourceMaterialPath']]
                    element['detail']['mesh']['sourceMaterialSlots'] = [
                        dict(sourceMaterialIndex=0, material=slot_material)]
                    element['material'] = dict(templateId='effect.standard',
                        sourceMaterialPath=next(path for prop, path in required.reference_paths if prop == 'material'),
                        renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False),
                        execution=dict(enabled=False, failClosed=True))
                    projected = next(m for m in element['sourceRecipe']['modules']
                                     if m['className'] == 'particlemodulemeshmaterial')
                    path_literal = next(v for v in projected['literals']
                                        if v['propertyPath'] == 'meshmaterials.objectpath')
                    path_literal['propertyPath'] = 'meshmaterials[0].objectpath'
                for resource in element['resources']:
                    resource['assetId'] = resource['assetId'].replace('Effect/KoukuSaydon/FullRestore/Meshes/', 'Effect/World/Meshes/')
            material_resources = element['material']['sourceProfile'].get('textures', []) + [
                texture for slot in element['detail']['mesh'].get('sourceMaterialSlots', [])
                for texture in slot['material']['sourceProfile']['textures']]
            for resource in element['resources'] + material_resources:
                assert (ROOT / 'Client/Bin/Resources' / resource['assetId']).is_file(), resource
        source.write(ROOT / 'Data/Effects/Authored' / (asset + '.effect.json'), doc)


def prepare_geometry(evidence):
    sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
    import cook_wmodel_geometry_contract as geometry
    converter = ROOT / 'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe'
    target = ROOT / 'Client/Bin/Resources/Effect/World/Meshes'
    target.mkdir(parents=True, exist_ok=True)
    cooked = evidence / 'mesh_cook'
    cooked.mkdir(parents=True, exist_ok=True)
    manifest = source.SOURCE / 'CanonicalSource/Effect/Closure/closure.manifest.json'
    package = next((source.SOURCE / 'CanonicalSource/Effect/Closure/SourcePackages/fx_sm_00').glob('*.upk'))
    digest = lambda path: hashlib.sha256(path.read_bytes()).digest()
    rows = []
    for name in ('fm_b_picking_001', 'fm_i_mark_01'):
        gltf = source.SOURCE / 'EffectRuntimeClosureExports-20260829/fx_sm_00/Export/FX_SM_00' / (name + '.gltf')
        legacy = cooked / (name + '.wmodel')
        invocation = [str(converter), str(gltf), '-o', str(legacy), '--scale', '100', '--pretransform']
        completed = subprocess.run(invocation, capture_output=True, text=True, check=True)
        log = cooked / (name + '.cook.json')
        source.write(log, dict(command=invocation, stdout=completed.stdout, stderr=completed.stderr,
            sourceObject='fx_sm_00.' + name, sourceGltf=str(gltf), sourceGltfSha256=digest(gltf).hex()))
        provenance = geometry.GeometryProvenanceEvidence('fx_sm_00.' + name, digest(manifest),
            'OBSERVED_SOURCE_RECEIPT', digest(package), digest(converter), digest(log), digest(log))
        payload, receipt = geometry.cook_wmodel_geometry_contract(gltf, legacy, provenance)
        destination = target / (name + '.wmodel')
        if not destination.is_file() or destination.read_bytes() != payload:
            destination.write_bytes(payload)
        decoded = geometry.parse_geometry_wmodel(payload)
        rows.append(dict(source='fx_sm_00.' + name, assetId=destination.relative_to(ROOT / 'Client/Bin/Resources').as_posix(),
            vertexCount=sum(len(s['vertices']) for s in decoded['submeshes']), hasColor0=decoded['hasColor0'],
            hasTexcoord1=decoded['hasTexcoord1'], modelPreScale=.01))
        source.write(cooked / (name + '.geometry.json'), receipt)
    source.write(evidence / 'geometry_installation.json', rows)


def prepare_textures(evidence, required_path):
    required = source.read(required_path)
    resource_root = ROOT / 'Client/Bin/Resources'
    source_root = source.SOURCE / 'EffectRuntimeClosureExports-20260829'
    all_source = collections.defaultdict(list)
    for path in source_root.rglob('*'):
        if path.suffix.lower() in ('.dds', '.tga'):
            all_source[path.stem.lower()].append(path)
    mapping, rows = {}, []
    for key, props in required.items():
        package, relative = key.split('.', 1)
        name = relative.rsplit('.', 1)[-1]
        candidates = [p for p in all_source[name] if ('/' + package + '/') in p.as_posix().lower()]
        candidates.sort(key=lambda p: (p.relative_to(source_root).parts[0].lower() != package,
                                       p.suffix != '.dds', len(str(p))))
        assert candidates, ('missing exact source texture', key)
        original = candidates[0]
        destination = resource_root / 'Effect/World/Textures' / package / (name + '.dds')
        destination.parent.mkdir(parents=True, exist_ok=True)
        if original.suffix.lower() == '.dds':
            payload = original.read_bytes()
            if not destination.is_file() or destination.read_bytes() != payload:
                destination.write_bytes(payload)
        else:
            from PIL import Image
            original_image = Image.open(original).convert('RGBA')
            original_image.save(destination, format='DDS')
            assert Image.open(destination).convert('RGBA').tobytes() == original_image.tobytes()
        import struct
        payload = destination.read_bytes()
        assert payload[:4] == b'DDS '
        height, width = struct.unpack_from('<II', payload, 12)
        assert (width, height) == (IMPORTED.prop(props, 'sizex'), IMPORTED.prop(props, 'sizey')), key
        mapping[key] = destination.relative_to(resource_root).as_posix()
        rows.append(dict(sourceObject=key, sourceFile=str(original), assetId=mapping[key]))
    source.write(evidence / 'texture_asset_map.json', mapping)
    source.write(evidence / 'texture_installation.json', rows)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=EVIDENCE)
    parser.add_argument('--native-material-patch', type=Path)
    parser.add_argument('--prepare-geometry', action='store_true')
    parser.add_argument('--required-textures', type=Path)
    arguments = parser.parse_args()
    if arguments.prepare_geometry:
        prepare_geometry(arguments.evidence_root)
    elif arguments.required_textures:
        prepare_textures(arguments.evidence_root, arguments.required_textures)
    elif arguments.native_material_patch:
        project(arguments.evidence_root, arguments.native_material_patch)
    else:
        _, rows, _ = acquire(arguments.evidence_root)
        print('World source occurrences:', dict(collections.Counter(r['effectAssetId'] for r in rows)))
