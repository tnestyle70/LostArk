"""Recover original native materials for source-qualified Kouku pattern leaves.

Reuses the shared raw MaterialMap/DXBC reader and native HLSL lowering. The
caller supplies an allocated range. Shared runtime files are not edited here.
"""
import argparse
import copy
import functools
import hashlib
import re
import shutil
import struct
import subprocess
import sys
from pathlib import Path
from build_kouku_gate3_rainbow_native import (
    ROOT, RELEASE, UMODEL, material, obj, pkg, fullref, sm, restore, norm, merge)
import build_kouku_gate3_rainbow_native as source_native
from native_material_tables import read_material_bytes


_parse_static_parameter_set = sm.parse_static_parameter_set


def parse_numbered_static_set(data, offset, names):
    """Expand FName's stored number with UE's number-minus-one string rule.

    The shared historical parser requires number zero. Normalize only those
    names in a private byte view, then retain hashes of the original bytes.
    Cache and MIC keys pass through the same canonical FName representation.
    """
    try:
        return _parse_static_parameter_set(data, offset, names)
    except ValueError as error:
        if 'numbered FName is unsupported' not in str(error):
            raise
    payload, table, numbered = bytearray(data), list(names), []
    cursor = offset + 16
    for array_name, stride in (('staticSwitchParameters', 32), ('staticComponentMaskParameters', 44),
                               ('normalParameters', 29), ('terrainLayerWeightParameters', 32)):
        if cursor + 4 > len(data):
            raise ValueError('Numbered static parameter array count is outside candidate bytes')
        count = struct.unpack_from('<I', data, cursor)[0]
        cursor += 4
        if count > 4096 or cursor + count * stride > len(data):
            raise ValueError('Numbered static parameter array is outside candidate bytes')
        for index in range(count):
            entry = cursor + index * stride
            name_index, number = struct.unpack_from('<II', data, entry)
            if name_index >= len(names):
                raise ValueError('Numbered static parameter FName is outside the source table')
            if number:
                expanded = names[name_index] + '_' + str(number - 1)
                numbered.append(dict(array=array_name, index=index, baseName=names[name_index], number=number))
                struct.pack_into('<II', payload, entry, len(table), 0)
                table.append(expanded)
        cursor += count * stride
    result = _parse_static_parameter_set(bytes(payload), offset, table)
    result['rawSha256'] = hashlib.sha256(data[offset:cursor]).hexdigest()
    result['sourceNumberedNames'] = numbered
    return result


sm.parse_static_parameter_set = parse_numbered_static_set

_source_object = obj
_source_package = pkg
_source_fullref = fullref
_startup_groups = {'enginematerials', 'engine_mi_shaders'}


@functools.lru_cache(None)
def pkg(name):
    if name in _startup_groups:
        # These original package groups are embedded exports of STARTUP, not
        # independently resolvable logical .upk packages. The canonical source
        # pack and historical UModel export retain this exact physical owner.
        return pkg('startup')
    if name == 'startup':
        candidates = list((restore.SOURCE / 'CanonicalSource/Shared/Packages/STARTUP/Source').glob('*.upk'))
        if not candidates:
            # STARTUP is present at ReleasePC's root in a retail installation.
            # Resolve its original logical identity instead of requiring one
            # machine's historical export folder.
            candidates = [restore.ue3.resolve_physical_package(UMODEL, RELEASE, 'startup', 'kr')]
        assert len(candidates) == 1, ('STARTUP source package identity', candidates)
        source_native.packages[name] = candidates[0]
    return _source_package(name)


def fullref(name, package, reference):
    if name in _startup_groups or name == 'startup':
        return restore.ue3.package_ref_path(reference, package.imports, package.exports)
    return _source_fullref(name, package, reference)


@functools.lru_cache(None)
def obj(path):
    """Resolve the source's compact localized-asset redirector before decoding."""
    package_name, relative = path.split('.', 1)
    package = pkg(package_name)
    if package_name in _startup_groups:
        entry = restore.find_export(package, path)
        raw = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        properties, end = source_native.parse_tagged_properties(raw, package.names, package.summary.version)
        return dict(path=path, package=package_name,
            className=restore.ue3.package_ref_name(entry.class_index, package.imports, package.exports),
            properties=properties, tail=raw[end:], serialSha256=hashlib.sha256(raw).hexdigest(),
            exportIndex=entry.index, sourcePackagePhysicalPath=str(package.path))
    entry = restore.find_export(package, relative)
    class_name = restore.ue3.package_ref_name(entry.class_index, package.imports, package.exports)
    if class_name.lower() == 'objectredirector':
        raw = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        assert len(raw) == 16, ('unreviewed ObjectRedirector size', path, len(raw))
        name_index, number, target_index = struct.unpack_from('<IIi', raw, 4)
        assert package.names[name_index].lower() == 'none' and number == 0 and target_index != 0
        target = fullref(package_name, package, target_index)
        assert target != path
        result = copy.deepcopy(obj(target))
        result['sourceRedirectorPath'] = path
        result['sourceRedirectorSha256'] = hashlib.sha256(raw).hexdigest()
        return result
    if class_name.lower() != 'locassetredirector':
        return _source_object(path)
    raw = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
    assert len(raw) == 20, ('unreviewed LocAssetRedirector size', path, len(raw))
    name_index, number, target_index, reserved = struct.unpack_from('<IIiI', raw, 4)
    assert package.names[name_index].lower() == 'none' and number == reserved == 0
    target = fullref(package_name, package, target_index)
    assert target != path and target_index != 0
    result = copy.deepcopy(obj(target))
    result['sourceRedirectorPath'] = path
    result['sourceRedirectorSha256'] = hashlib.sha256(raw).hexdigest()
    return result


# The shared material-chain decoder follows this task's source-resolved objects;
# this changes only this Python process, not the shared module's default API.
source_native.obj = obj
source_native.pkg = pkg
source_native.fullref = fullref


def native_key(program):
    """A source material/VF permutation is reusable only with both exact shaders."""
    return (program.get('resolvedMaterial', program.get('sourceMaterial')),
            program['rendererShape'], program['sourceVF'], program['sourceVS'], program['sourcePS'],
            bool(program.get('sourceTransformMesh', False)))


def reuse_native_programs(selected, roots, source_materials=None):
    previous = {}
    for root in roots:
        contract = restore.read(root / 'native_runtime_contract.json')
        assert not contract.get('deferredPrograms'), (root, 'incomplete native reuse input')
        for program in contract['programs']:
            previous.setdefault(native_key(program), []).append(program)
    fresh, reused = [], []
    for selection in selected:
        candidates = previous.get(native_key(selection), [])
        if source_materials is not None:
            current = source_materials[selection['resolvedMaterial']]
            textures = {row['index']: row for row in current['effectiveTextures']}
            def same_source(program):
                same_textures = all(texture['sourceObjectPath'].casefold() ==
                    textures.get(texture['index'], {}).get('sourceObjectPath', '').casefold()
                    for texture in program['textures'])
                same_parameters = all(current['effectiveNumericOverrides'][parameter['kind'] + 's']
                    .get(parameter['name'], {}).get('value', parameter['default']) == parameter['effective']
                    for parameter in program['parameters'])
                return same_textures and same_parameters
            # A historical MIC may retain the same shaders while its texture or
            # numeric inputs changed. Disqualify that version before comparing
            # the remaining source-exact candidates with one another.
            candidates = [program for program in candidates if same_source(program)]
        if not candidates:
            fresh.append(selection)
        else:
            old = candidates[0]
            for duplicate in candidates[1:]:
                for field in ('textures', 'parameters', 'staticSwitches', 'parentMaterial'):
                    assert old[field] == duplicate[field], (native_key(selection), 'native reuse contract differs', field)
            row = copy.deepcopy(old)
            row['occurrences'] = list(selection['occurrences'])
            reused.append(row)
    return fresh, reused


def copy_native_cache(out, roots, refs):
    """Copy only source-ID addressed bytecode/object cache, never shader output."""
    for root in roots:
        for shader_id in refs:
            for folder, suffix in (('dxbc', '.dxbc'), ('shader_objects', '.bin'), ('shader_objects', '.json')):
                source = root / folder / (shader_id + suffix)
                target = out / folder / (shader_id + suffix)
                if source.is_file() and not target.exists():
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(source, target)

def prepare(evidence, first, last, reuse_roots=(), resource_root=None):
    out = evidence / 'native'
    out.mkdir(parents=True, exist_ok=True)
    write = lambda name, value: restore.write(out / name, value)
    records = restore.read(evidence / 'source_module_inputs.json')['records']
    records.update({r['fullPath']: r for r in restore.read(evidence / 'source_class_defaults.json')['records']})
    occurrences, material_sources = [], []
    for original in restore.read(evidence / 'source_occurrences.json'):
        if original['rendererShape'] == 'light':
            continue
        occurrence = copy.deepcopy(original)
        render_mode = restore.imported.prop(records[occurrence['sourceEmitter']]['properties'], 'emitterrendermode', 'erm_normal')
        if occurrence['sourceMaterial'] == 'enginematerials.defaultparticle' and render_mode in ('erm_none', 'erm_point'):
            continue
        if not occurrence['sourceMaterial']:
            # The source null material on a mesh emitter uses its mesh section
            # material. Resolve only a source-exported single material section,
            # corroborated by the actual StaticMesh native object reference.
            assert occurrence['rendererShape'] == 'mesh' and occurrence['sourceMesh'], original
            mesh = obj(occurrence['sourceMesh'])
            package = pkg(mesh['package'])
            mesh_name = occurrence['sourceMesh'].rsplit('.', 1)[-1]
            search = restore.SOURCE / 'EffectRuntimeClosureExports-20260829' / mesh['package']
            gltfs = list(search.rglob(mesh_name + '.gltf'))
            if not gltfs:
                gltfs = list((evidence / 'source_geometry_export' / mesh['package']).rglob(mesh_name + '.gltf'))
            assert len(gltfs) == 1, (occurrence['sourceMesh'], gltfs)
            gltf = restore.read(gltfs[0])
            slots = {p['material'] for m in gltf['meshes'] for p in m['primitives']}
            assert len(slots) == 1, ('multiple original mesh materials require section projection', occurrence['sourceMesh'])
            name = gltf['materials'][slots.pop()]['name'].lower()
            refs = [-(entry.index + 1) for entry in package.imports
                    if entry.object_name.lower() == name and 'material' in entry.class_name.lower()]
            assert len(refs) == 1 and mesh['tail'].count(struct.pack('<i', refs[0])) == 1, ('unclosed native mesh section material', name, refs)
            resolved = fullref(mesh['package'], package, refs[0])
            material_sources.append(dict(elementId=occurrence['elementId'], sourceRequiredMaterial=None,
                sourceMesh=occurrence['sourceMesh'], sourceGltf=str(gltfs[0]), sourceMaterial=resolved,
                sourceNativeMaterialReference=refs[0], sourceSerialSha256=mesh['serialSha256']))
            occurrence['sourceMaterial'] = resolved
        occurrences.append(occurrence)
    write('source_mesh_material_resolution.json', material_sources)
    rows, source_failures = [], []
    for path in sorted({o['sourceMaterial'] for o in occurrences}):
        try:
            rows.append(material(path))
        except Exception as error:
            source_failures.append(dict(sourceMaterial=path, stage='MATERIAL_SOURCE_ACQUISITION',
                occurrences=[o['elementId'] for o in occurrences if o['sourceMaterial'] == path],
                reason=f'{type(error).__name__}: {error}'))
    write('source_material_failures.json', source_failures)
    acquired_materials = {row['sourceMaterial'] for row in rows}
    occurrences = [o for o in occurrences if o['sourceMaterial'] in acquired_materials]
    cache = sm.package_tables(RELEASE / 'EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk')
    layout = sm.parse_shader_code_layout(cache)
    scan_path = out / 'material_map_scan.json'
    scans = restore.read(scan_path) if scan_path.exists() else {}
    for root in reuse_roots:
        cached = root / 'material_map_scan.json'
        if cached.is_file():
            for base_id, contexts in restore.read(cached).items():
                scans.setdefault(base_id, contexts)
    # Historical receipts can refer to the same material GUID at a different
    # offset in another installed shader-cache revision. Validate its original
    # GUID before reusing a range; a stale range must be located again.
    for base_id in {r['baseId'] for r in rows} & set(scans):
        if any(cache['reader'].read_logical_range(context['logicalOffset'], 16) != bytes.fromhex(base_id)
               for context in scans[base_id]['materialMapContexts']):
            del scans[base_id]
    missing = sorted({r['baseId'] for r in rows} - set(scans))
    if missing:
        scans.update(sm.scan_base_material_contexts(cache, layout, missing))
    write('material_map_scan.json', scans)
    textures, resolved_rows = {}, []
    cached_rows = {}
    for cache_root in [out] + list(reuse_roots):
        for cache_name in ('native_material_inputs.json', 'native_material_inputs.partial.json'):
            cache_path = cache_root / cache_name
            if cache_path.is_file():
                for cached in restore.read(cache_path)['materials']:
                    cached_rows.setdefault(cached['sourceMaterial'], cached)
    for row in rows:
        try:
            cached = cached_rows.get(row['sourceMaterial'])
            comparable = ('sourceSerialSha256', 'baseId', 'parentMaterial', 'effectiveNumericOverrides', 'textureOverrides')
            if cached and all(cached.get(field) == row.get(field) for field in comparable):
                row.update(copy.deepcopy(cached))
            else:
                equality = row['mic']['engineEqualityStaticParameterSetSha256']
                context = sm.select_unique_map_context(scans[row['baseId']], equality)
                row['materialMap'] = sm.parse_material_map(cache, layout, context, equality)
                row['mapKey'] = equality
                parent = obj(row['parentMaterial'])
                package = pkg(parent['package'])
                tail = parent['tail']
                assert len(tail) >= 40 and struct.unpack_from('<I', tail)[0] == 1 and tail[16:32].hex() == row['baseId']
                count = struct.unpack_from('<I', tail, 36)[0]
                assert count <= 256 and len(tail) >= 40 + 4 * count
                references = struct.unpack_from('<' + 'i' * count, tail, 40)
                row['effectiveTextures'] = []
                for index, expression in enumerate(row['materialMap']['uniformExpressionSet']['pixelTexture2DExpressions']):
                    name = expression.get('parameterName')
                    number = expression.get('parameterNameNumber', 0)
                    if number:
                        name += '_' + str(number - 1)
                    path = row['textureOverrides'].get(name)
                    if not path:
                        # Static MIC permutations can cache default texture
                        # expressions absent from the parent's cooked variant.
                        # Read that same source MIC's native resource table;
                        # never replace an unresolved input with a guessed map.
                        source_object = obj(row['sourceMaterial'])
                        native_tail = source_object['tail']
                        static_offset = row['mic'].get('staticParameterSetOffsetInNativeTail', 0)
                        if len(native_tail) >= 40 and static_offset >= 40:
                            source_count = struct.unpack_from('<I', native_tail, 36)[0]
                            source_end = 40 + 4 * source_count
                            source_index = expression['referencedTextureIndex']
                            if (0 <= source_index < source_count <= 256 and
                                    source_end <= min(len(native_tail), static_offset)):
                                source_ref = struct.unpack_from('<i', native_tail, 40 + 4 * source_index)[0]
                                if source_ref:
                                    path = fullref(source_object['package'], pkg(source_object['package']), source_ref)
                                    row.setdefault('nativeTextureDefaultEvidence', []).append(dict(
                                        expressionIndex=index, sourceReference=source_ref,
                                        sourceSerialSha256=source_object['serialSha256'], sourceTexture=path))
                    if not path:
                        if 0 <= expression['referencedTextureIndex'] < len(references):
                            path = fullref(parent['package'], package, references[expression['referencedTextureIndex']])
                        else:
                            # A static MIC permutation can retain a texture absent from
                            # the parent's cooked default permutation reference array.
                            # Resolve its exact named TextureParameter expression.
                            candidates = []
                            parent_prefix = row['parentMaterial'].split('.', 1)[1] + '.'
                            for entry in package.exports:
                                relative = restore.ue3.package_ref_path(entry.index + 1, package.imports, package.exports)
                                if not relative.lower().startswith(parent_prefix.lower()):
                                    continue
                                expression_object = obj(parent['package'] + '.' + relative)
                                props = expression_object['properties']
                                if str(restore.imported.prop(props, 'parametername', '')).lower() == str(name).lower():
                                    texture_ref = restore.imported.prop(props, 'texture', 0)
                                    if texture_ref:
                                        candidates.append(fullref(parent['package'], package, texture_ref))
                            assert len(set(candidates)) == 1, (row['sourceMaterial'], expression, candidates)
                            path = candidates[0]
                    try:
                        texture = obj(path)
                    except Exception as error:
                        raise ValueError(f'Original texture {path}: {error}') from error
                    assert texture['className'] == 'texture2d', (path, texture['className'])
                    row['effectiveTextures'].append(dict(index=index, parameterName=name,
                        sourceReferencePath=path, sourceObjectPath=texture['path'], properties=texture['properties']))
                    textures[texture['path']] = texture['properties']
            resolved_rows.append(row)
            if len(resolved_rows) % 64 == 0:
                write('native_material_inputs.partial.json', dict(materials=resolved_rows))
        except Exception as error:
            source_failures.append(dict(sourceMaterial=row['sourceMaterial'], stage='MATERIAL_MAP_AND_TEXTURE_RESOLUTION',
                occurrences=[o['elementId'] for o in occurrences if o['sourceMaterial'] == row['sourceMaterial']],
                reason=f'{type(error).__name__}: {error}'))
    rows = resolved_rows
    textures = {texture['sourceObjectPath']: texture['properties'] for row in rows for texture in row['effectiveTextures']}
    write('source_material_failures.json', source_failures)
    write('native_material_inputs.json', dict(materials=rows))
    write('required_native_textures.json', textures)
    by_material = {r['sourceMaterial']: r for r in rows}
    occurrences = [o for o in occurrences if o['sourceMaterial'] in by_material]
    @functools.lru_cache(None)
    def effective(path):
        row = records[path]
        parent = row.get('archetypeFullPath')
        if not parent and not path.startswith(('engine.', 'efgame.', 'core.')):
            cls = row['classPath']
            parent = cls.split('.')[0] + '.default__' + cls.split('.')[-1]
        return merge(effective(parent) if parent else {}, norm(row['properties']))
    def value(properties, key, default=None):
        result = properties.get(key, default)
        return result.get('value') if isinstance(result, dict) and 'value' in result else result
    selected, refs, shader_material, vf_decisions = {}, {}, {}, []
    for occurrence in occurrences:
        try:
            shape = occurrence['rendererShape']
            original_shape = shape
            type_classes = [records[p]['classPath'].rsplit('.', 1)[-1] for p in occurrence['moduleOrder']
                            if p in records and 'typedata' in records[p]['classPath']]
            if any('typedataanimtrail' in name for name in type_classes):
                shape = 'animationTrail'
            elif any('typedataribbon' in name for name in type_classes):
                shape = 'ribbon'
            elif any('typedatabeam' in name for name in type_classes):
                shape = 'beam'
            row = by_material[occurrence['sourceMaterial']]
            component = occurrence.get('sourceStaticMeshComponent')
            post_track = occurrence.get('sourcePostRenderMaterialTrack') or occurrence.get('sourceActionPostMaterial')
            if post_track:
                # Matinee post-render materials use the original full-screen
                # LocalVF quad, not a fabricated Cascade required module.
                assert shape == 'screenPost' and not component
                assert records[post_track]['classPath'].rsplit('.', 1)[-1] in ('efinterptrackpostrendermaterial', 'efpostprocessmaterialeffectskill')
                required = post_track
                properties = norm(records[post_track]['properties'])
            elif component:
                assert shape == 'mesh' and records[component]['classPath'] == 'engine.staticmeshcomponent'
                required = component
                properties = norm(records[component]['properties'])
            else:
                required = next(p for p in occurrence['moduleOrder'] if 'particlemodulerequired' in p)
                properties = effective(required)
            dynamic = any('parameterdynamic' in p for p in occurrence['moduleOrder'])
            if shape == 'mesh' or post_track:
                vf = 'flocalvertexfactory'
            elif shape == 'decal':
                vf = 'flocaldecalvertexfactory'
            elif shape in ('animationTrail', 'ribbon', 'beam'):
                vf = 'fparticlebeamtrail' + ('dynamicparameter' if dynamic else '') + 'vertexfactory'
            else:
                assert shape in ('sprite', 'screenPost'), ('unmapped source renderer', shape, occurrence['elementId'])
                subuv = str(value(properties, 'interpolationmethod', 'psuvim_none')).lower() != 'psuvim_none'
                vf = 'fparticle' + ('subuv' if subuv else '') + ('offsetcenter' if value(properties, 'boffsetcenter', False) else '')
                vf += ('dynamicparameter' if dynamic else '') + 'vertexfactory'
            requested_vf = vf
            available = {v['vertexFactoryType'] for v in row['materialMap']['vertexFactories']}
            single_cell_uv = False
            if vf not in available and shape in ('sprite', 'screenPost') and 'subuv' in vf:
                # Original Firedc emitters enable linear interpolation but
                # retain a 1x1 grid and have no SubUV animation module. Both
                # UV sets are exactly the ordinary sprite UV in this case.
                if (value(properties, 'subimages_horizontal', 1) == 1 and
                    value(properties, 'subimages_vertical', 1) == 1 and
                    not any('subuv' in records[p]['classPath'] for p in occurrence['moduleOrder'] if p in records)):
                    candidate = vf.replace('subuv', '')
                    if candidate in available or candidate.replace('dynamicparameter', '') in available:
                        vf, single_cell_uv = candidate, True
            # Some source MICs cache only the otherwise identical sprite variant
            # with dynamic inputs. Preserve that original material permutation;
            # the existing particle carrier supplies its ordinary default inputs.
            # This does not substitute mesh or beam shaders for a sprite layout.
            if vf not in available and shape in ('sprite', 'screenPost') and not dynamic:
                candidate = vf.removesuffix('vertexfactory') + 'dynamicparametervertexfactory'
                if candidate in available:
                    vf = candidate
            if vf not in available and 'dynamicparameter' in vf and vf.replace('dynamicparameter', '') in available:
                # The original material map may compile away an unused Dynamic
                # Parameter module. Retain its exact non-dynamic cached program.
                vf = vf.replace('dynamicparameter', '')
            factories = [v for v in row['materialMap']['vertexFactories'] if v['vertexFactoryType'] == vf]
            assert len(factories) == 1, (occurrence['elementId'], vf)
            vf_decisions.append(dict(elementId=occurrence['elementId'], sourceRequiredModule=required,
                sourceTypeDataClasses=type_classes, sourceInventoryShape=original_shape, selectedRendererShape=shape,
                dynamicParameterModule=dynamic, requestedVF=requested_vf, selectedOriginalMaterialVF=vf,
                status='ORIGINAL_SINGLE_CELL_SPRITE_UV_EQUIVALENCE' if single_cell_uv else
                    'ORIGINAL_MATERIAL_VF' if requested_vf == vf else 'ORIGINAL_SPRITE_DYNAMIC_VARIANT_DEFAULT_INPUTS'))
            shaders = factories[0]['shaderReferences']
            pixels = [s for s in shaders if s['shaderType'] == 'tbasepasspixelshaderfnolightmappolicyskylight']
            vertices = [s for s in shaders if 'basepassvertexshaderfnolightmappolicy' in s['shaderType'] and 'nodensitypolicy' in s['shaderType']]
            assert len(pixels) == len(vertices) == 1
            pixel, vertex = pixels[0], vertices[0]
            key = (row['sourceMaterial'], vf, shape, bool(component))
            program = selected.setdefault(key, dict(resolvedMaterial=row['sourceMaterial'], sourceVF=vf,
                sourceVS=vertex['shaderIdHex'], sourcePS=pixel['shaderIdHex'], rendererShape=shape, occurrences=[]))
            if component:
                program['sourceTransformMesh'] = True
                program.setdefault('sourceStaticMeshComponents', []).append(component)
            program['occurrences'].append(occurrence['elementId'])
            referenced_shaders = [pixel, vertex]
            if value(row['parentProperties'], 'busesdistortion', False):
                distortion_ps = [s for s in shaders if s['shaderType'] == 'tdistortionmeshpixelshader<fdistortmeshaccumulatepolicy>']
                distortion_vs = [s for s in shaders if s['shaderType'] == 'tdistortionmeshvertexshader<fdistortmeshaccumulatepolicy>']
                # This original OneLayer permutation samples SceneColor in its
                # base pass (the already restored SDNative374 ABI). Its source
                # map contains no separate accumulation pass. Other materials
                # still require the complete original distortion pair.
                single_pass = (row['sourceMaterial'] == 'fx_mastermaterial.fx_mi.fx_mm_onelayerdistortion_02_01_ad'
                    and vf == 'fparticledynamicparametervertexfactory'
                    and program['sourceVS'] == '5825675b4ffbc840ad691ec56973cf7e'
                    and program['sourcePS'] == 'eb2bcd5c8f3c6c49805ab689687b14f6')
                if single_pass:
                    assert not distortion_ps and not distortion_vs, 'OneLayer source pass layout changed'
                    program['sourceSceneColorBasePass'] = True
                else:
                    assert len(distortion_ps) == len(distortion_vs) == 1, ('missing original distortion pair', occurrence['elementId'], vf)
                    program['distortionPass'] = dict(sourcePS=distortion_ps[0]['shaderIdHex'], sourceVS=distortion_vs[0]['shaderIdHex'])
                    referenced_shaders.extend([distortion_ps[0], distortion_vs[0]])
            for shader in referenced_shaders:
                refs[shader['shaderIdHex']] = shader
                shader_material[shader['shaderIdHex']] = row
        except Exception as error:
            source_failures.append(dict(sourceMaterial=occurrence['sourceMaterial'], stage='SOURCE_VERTEX_FACTORY_SELECTION',
                occurrences=[occurrence['elementId']], reason=f'{type(error).__name__}: {error}'))
    write('source_material_failures.json', source_failures)
    fresh, reused = reuse_native_programs(list(selected.values()), reuse_roots, {row['sourceMaterial']: row for row in resolved_rows})
    assert len(fresh) <= last - first + 1, ('native program range exhausted', len(fresh), first, last)
    write('selected_runtime_material_programs.json', dict(programs=fresh, errors=[]))
    write('reused_native_programs.json', dict(programs=reused,
        sourceRoots=[str(root) for root in reuse_roots], freshProgramCount=len(fresh)))
    write('source_vf_selections.json', vf_decisions)
    companion = []
    selected_by_key = {native_key(s): s for s in selected.values()}
    for program_id, selection in [(first+i, p) for i,p in enumerate(fresh)] + [(p['program'], selected_by_key[native_key(p)]) for p in reused]:
        if selection.get('distortionPass'):
            companion.append(dict(selection, program=program_id, **selection['distortionPass']))
    write('selected_distortion_programs.json', dict(programs=companion))
    required_ids = {sid for p in fresh + companion for sid in (p['sourceVS'], p['sourcePS'])}
    refs = {sid: reference for sid, reference in refs.items() if sid in required_ids}
    copy_native_cache(out, reuse_roots, refs)
    missing_refs = [r for sid, r in refs.items() if not (out / 'full_programs' / (sid + '.json')).exists()]
    if missing_refs:
        (out / 'dxbc').mkdir(exist_ok=True)
        (out / 'full_programs').mkdir(exist_ok=True)
        missing_bytecodes = [r for r in missing_refs if not (out / 'dxbc' / (r['shaderIdHex'] + '.dxbc')).exists()]
        bytecodes = sm.extract_selected_packed_dxbc(cache, layout, missing_bytecodes) if missing_bytecodes else {}
        pixels = [r for r in missing_refs if 'pixelshader' in r['shaderType']]
        object_root = out / 'shader_objects'
        object_root.mkdir(exist_ok=True)
        missing_objects = [r for r in pixels if not (object_root / (r['shaderIdHex'] + '.bin')).exists()]
        objects = sm.extract_selected_shader_objects(cache, layout, missing_objects,
            allow_mixed_code_preambles=True) if missing_objects else None
        if objects:
            for sid, raw in objects['byShaderId'].items():
                (object_root / (sid + '.bin')).write_bytes(raw['_bytes'])
                write('shader_objects/' + sid + '.json', dict(logicalOffset=raw['logicalOffset'],
                    uniformExpressionCounts=shader_material[sid]['materialMap']['uniformExpressionCounts']))
        disassembler = sm.D3DDisassembler(sm.DEFAULT_D3DCOMPILER)
        for reference in missing_refs:
            try:
                sid = reference['shaderIdHex']
                bytecode_path = out / 'dxbc' / (sid + '.dxbc')
                bytecode = bytecodes[sid]['_bytecode'] if sid in bytecodes else bytecode_path.read_bytes()
                if not bytecode_path.exists():
                    bytecode_path.write_bytes(bytecode)
                disassembly = disassembler.disassemble(bytecode)
                chunks = sm.dxbc_chunk_payloads(bytecode)
                program = dict(shaderId=sid, disassembly=disassembly,
                    inputSignature=sm.parse_dxbc_signature(chunks.get('ISGN', chunks.get('ISG1'))))
                if disassembly['profile'].startswith('ps'):
                    declaration = sm.parse_dxbc_declaration_closure(disassembly, allow_textureless=True, allow_absent_cb0=True)
                    raw = restore.read(object_root / (sid + '.json'))
                    raw['_bytes'] = (object_root / (sid + '.bin')).read_bytes()
                    material_map = shader_material[sid]['materialMap']
                    required_vectors = {i for i, expression in enumerate(material_map['uniformExpressionSet']['pixelVectorExpressions'])
                        if expression.get('parameterName') == 'selectioncolor'
                        and expression.get('typeName') == 'fmaterialuniformexpressionvectorparameter'} if (
                            not declaration['declaredTextureRegisters'] or
                            not material_map['uniformExpressionCounts']['pixelTexture2DExpressions']) and declaration['declaredConstantBuffer0Float4Count'] else set()
                    candidates = sm.scan_native_binding_array_candidates(raw['_bytes'], raw['logicalOffset'],
                        material_map['uniformExpressionCounts'], declaration,
                        allow_engine_only_textures=True, required_vector_expression_indices=required_vectors)
                    # Material bias samples need a native expression wire. These
                    # two original blur programs also bias-sample engine SceneColor:
                    # the exact DXBC identity and complete native wire closure
                    # distinguish those reads from a missing material texture.
                    reviewed_scene_bias = {
                        # Guardian action PostProcessChain: exact original PS and native sampler closure.
                        '599a6ad60ea6494a83850077e6cd3356': ('95bba2769a4da159253987811d98093ff2af583cddf8f139ec7ca1dbcc12d502', ['t3/s0'], ['t0/s2', 't1/s1', 't2/s3']),
                        '5a405d04c4e0fa42a1342f8da1615ca0': ('e988fd2b5a355d2ff582c6937a52ed713bf584b6409c42ff29aae49d38fa67a0', ['t3/s0'], ['t0/s2', 't1/s1', 't2/s3', 't4/s4']),
                        # Dragon Look original PS: reviewed SceneColor bias reads.
                        '1000024695759244a7297e6a1c0a53e1': ('938247dc2c5216a2ae240972720393e10aac69a82ecee180f1aeace8446b3a0a', ['t0/s0'], ['t1/s1']),
                        '7f34a1fedf7a614cbcd4e656c906fe5e':
                            ('13ada0f252499b71e4aaeaf4988c6631790041a07f0132dbb68419915da0b66f', ['t1/s0'], ['t0/s2', 't2/s1']),
                        'a43001dc99c4224bbab23b82863a4bc7':
                            ('064645dda06795824e16ba8fa900acc4d4ed52790694e3c1bfe8ab13091a6adc', ['t0/s0'], ['t1/s1']),
                        '3fc4c0de7f119c49b1e0478e97872fc9':
                            ('bc1f4c2b5ee13676838fe5a1397e720ddbf52d75ca1ad8aa59d654b55f9ca08e', ['t0/s0'], []),
                        'd7c41b166d92df4bbdbdd2d52910bb43':
                            ('2176e2951142dbb388b7b81b65374506d08950c8021e0028d793341e36fa0020', ['t1/s0'], ['t0/s2', 't2/s1']),
                        'b9fc10ac51695c41b71ae1807fe6a47d':
                            ('7cb228db6767cfb62296b31cd8fb9b0c0c8ae3febeb69bea791217b19f3466d0', ['t0/s0'], [], [0, 1, 2, 3, 4, 5, 6]),
                    }.get(sid)
                    if reviewed_scene_bias:
                        assert declaration['instructionSha256'] == reviewed_scene_bias[0]
                        candidates = [candidate for candidate in candidates
                            if candidate['constantBufferClosure']['unownedConstantBuffer0Slots'] == (reviewed_scene_bias[3] if len(reviewed_scene_bias) > 3 else [0])
                            and candidate['textureSampleClosure']['unownedEngineSamplePairs'] == reviewed_scene_bias[1]
                            and candidate['textureSampleClosure']['materialSamplePairs'] == reviewed_scene_bias[2]]
                    bias_pairs = {f"{sample['textureRegister']}/{sample['samplerRegister']}"
                        for sample in disassembly.get('sampleInstructions', [])
                        if sample['instruction'].startswith('sample_b_')}
                    if reviewed_scene_bias:
                        bias_pairs.difference_update(reviewed_scene_bias[1])
                    candidates = [candidate for candidate in candidates if bias_pairs.issubset(
                        candidate['textureSampleClosure']['materialSamplePairs'])]
                    if sid == '9aa5e61191a9654290657484e8c9cae6':
                        # Same source localcrack PS qualified in the 2026-09-15
                        # VNative66 restoration: 37 same-class shader objects
                        # establish the scalar-array offset 152, not the false
                        # vector-array interpretation at 148. Pin the actual
                        # bytecode and semantic wire before reusing that ABI.
                        assert hashlib.sha256(bytecode).hexdigest() == 'cb21a8e5eb108ff1f70a60acdd6446d5ee387b922634cd1f5e4d2a096d478c1a'
                        candidates = [candidate for candidate in candidates
                            if candidate['bindingArraysOffsetInShaderObject'] == 152
                            and candidate['bindingSemanticSha256'] == 'b1c5843f0ae303f802f881a8db7cadfc0f9f4f07209bcd93ceedade1e3a7589c']
                    write('shader_objects/' + sid + '.candidates.json', dict(declaration=declaration, candidates=candidates))
                    if candidates and declaration['declaredConstantBuffer0Float4Count'] == 0:
                        groups = {}
                        for candidate in candidates:
                            groups.setdefault(candidate['bindingSemanticSha256'], []).append(candidate)
                        # No material constants survived this original shader.
                        # Repeated zero-byte arrays are equivalent only when all
                        # texture wires and their complete closure also match.
                        if len(groups) == 1:
                            equivalent = next(iter(groups.values()))
                            candidates = [dict(equivalent[0],
                                equivalentBindingArrayOffsets=[c['bindingArraysOffsetInShaderObject'] for c in equivalent])]
                    if len(candidates) != 1:
                        # Identical source programs already qualified by the shared
                        # restoration path retain their audited binding identity.
                        known = list((ROOT / 'out').glob('*/full_programs/' + sid + '.json'))
                        known += list((ROOT / 'out').glob('*/native/full_programs/' + sid + '.json'))
                        hashes = set()
                        for path in known:
                            previous = restore.read(path)
                            if previous.get('disassembly') == disassembly and previous.get('bindings'):
                                hashes.add(previous['bindings']['bindingSemanticSha256'])
                        candidates = [candidate for candidate in candidates if candidate['bindingSemanticSha256'] in hashes]
                    assert len(candidates) == 1, (sid, 'ambiguous source native bindings', candidates)
                    program['bindings'] = dict(candidates[0], candidateCount=1)
                write('full_programs/' + sid + '.json', program)
            except Exception as error:
                source_failures.append(dict(sourceShader=reference['shaderIdHex'], stage='ORIGINAL_SHADER_BINDING_CLOSURE',
                    reason=f'{type(error).__name__}: {error}'))
        write('source_material_failures.json', source_failures)
    generate_native(evidence, first, last, resource_root=resource_root)


def generate_native(evidence, first, last, resource_root=None):
    """Lower a previously extracted original cohort without reopening packages."""
    out = evidence / 'native'
    write = lambda name, value: restore.write(out / name, value)
    reused = restore.read(out / 'reused_native_programs.json')['programs']
    source_failures = restore.read(out / 'source_material_failures.json')
    companion = restore.read(out / 'selected_distortion_programs.json')['programs']
    prepare_textures(evidence, out, resource_root=resource_root)
    subprocess.run([sys.executable, str(ROOT / 'Tools/EffectPipeline/generate_artist_native_runtime_shader.py'),
        '--source-dir', str(out), '--program-start', str(first), '--profile-domain', 'kouku'] +
        (['--resource-root', str(resource_root)] if resource_root else []), check=True)
    import install_kouku_gate1_native_materials as tables
    generated = restore.read(out / 'native_runtime_contract.json')
    if source_failures:
        generated['deferredPrograms'].extend(source_failures)
        write('native_runtime_contract.json', generated)
    merged = dict(generated, programs=generated['programs'] + reused)
    if companion:
        prepare_distortion(out, merged, first, resource_root=resource_root)
    write('merged_native_runtime_contract.json', merged)
    tables.FIRST = min([first] + [p['program'] for p in reused])
    tables.LAST = max([last] + [p['program'] for p in reused])
    candidate = out / 'Effect_ArtistMaterial.candidate.h'
    candidate.write_bytes(read_material_bytes(ROOT / 'Client/Public/Effect_ArtistMaterial.h'))
    tables.install(out / 'merged_native_runtime_contract.json', out, candidate, resource_root=resource_root)


def prepare_distortion(out, merged, first, resource_root=None):
    """Reuse the source accumulation pass in the existing signed distortion MRT."""
    folder = out / 'distortion'
    subprocess.run([sys.executable, str(ROOT / 'Tools/EffectPipeline/generate_artist_native_runtime_shader.py'),
        '--source-dir', str(out), '--selection-file', str(out / 'selected_distortion_programs.json'),
        '--output-dir', str(folder), '--program-start', str(first), '--profile-domain', 'kouku'] +
        (['--resource-root', str(resource_root)] if resource_root else []), check=True)
    distortion = restore.read(folder / 'native_runtime_contract.json')
    merged['deferredPrograms'].extend(dict(error, stage='ORIGINAL_DISTORTION_PASS') for error in distortion['deferredPrograms'])
    source = (folder / 'Shader_EffectArtistNative.hlsli').read_text(encoding='utf8')
    blocks = {int(number): block for block, number in re.findall(
        r'(#ifndef ARTIST_NATIVE_MODEL_ONLY\n// [^\n]+\nfloat4 ArtistNative(\d+)\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}\n#endif)', source, re.S)}
    rows = {row['program']: row for row in merged['programs']}
    emitted = []
    for row in distortion['programs']:
        number = row['program']
        if number not in rows:
            continue
        color = rows[number]
        # Resource aliases are interchangeable only with identical source inputs
        # and byte-identical DDS data. Retain the installed color program's ID.
        aliases = []
        resources = resource_root or ROOT / 'Client/Bin/Resources'
        for source_texture, companion_texture in zip(color['textures'], row['textures']):
            if source_texture['assetId'] == companion_texture['assetId']:
                continue
            first_texture = {key: value for key, value in source_texture.items() if key != 'assetId'}
            second_texture = {key: value for key, value in companion_texture.items() if key != 'assetId'}
            first_path, second_path = resources / source_texture['assetId'], resources / companion_texture['assetId']
            # A staged cohort may reuse an already installed color program.
            # Resolve that exact asset ID in the installed root when its bytes
            # were intentionally not copied into the candidate resource tree.
            if resource_root and not first_path.is_file():
                first_path = ROOT / 'Client/Bin/Resources' / source_texture['assetId']
            if resource_root and not second_path.is_file():
                second_path = ROOT / 'Client/Bin/Resources' / companion_texture['assetId']
            assert first_texture == second_texture and first_path.is_file() and second_path.is_file(), ('distortion texture alias inputs differ', number)
            source_hash = hashlib.sha256(first_path.read_bytes()).hexdigest()
            assert source_hash == hashlib.sha256(second_path.read_bytes()).hexdigest(), ('distortion texture alias bytes differ', number)
            aliases.append(dict(sourceAssetId=companion_texture['assetId'], canonicalAssetId=source_texture['assetId'], sha256=source_hash))
            companion_texture['assetId'] = source_texture['assetId']
        if aliases:
            color['verifiedDistortionTextureAliases'] = aliases
        assert color['parameters'] == row['parameters'] and color['textures'] == row['textures'], ('color/distortion material ABI differs', number)
        block = blocks[number].replace(f'ArtistNative{number}(', f'ArtistNative{number}Distortion(')
        # The two original passes discard independently. Zero accumulation
        # preserves the separate color result in the combined product MRT.
        block = block.replace('clip(-1.f);', 'return 0.f;')
        emitted.append(block)
        color['requiresDepthSample'] |= row['requiresDepthSample']
        color['requiresSceneColor'] |= row['requiresSceneColor']
        color['distortionPass'] = dict(sourcePS=row['sourcePS'], sourceVS=row['sourceVS'],
            functionName=f'ArtistNative{number}Distortion', hlsli='KoukuGenericDistortionPrograms.hlsli',
            depthAdapter=row['depthAdapter'], outputAdapter='SOURCE_POSITIVE_XY_MINUS_NEGATIVE_ZW_TO_SIGNED_DISTORTION_MRT')
    (out / 'KoukuGenericDistortionPrograms.hlsli').write_text('\n\n'.join(emitted) + '\n', encoding='utf8')



def prepare_textures(evidence, out, resource_root=None):
    required = restore.read(out / 'required_native_textures.json')
    installed_resources = ROOT / 'Client/Bin/Resources'
    resources = Path(resource_root).resolve() if resource_root is not None else installed_resources
    resources.mkdir(parents=True, exist_ok=True)
    export_root = evidence / 'source_texture_export'
    by_name = {}
    for path in resources.rglob('*.dds'):
        by_name.setdefault((path.parent.name.lower(), path.stem.lower()), []).append(path)
    if resources.resolve() != installed_resources.resolve():
        # Installed files are read-only inputs when preparing an offline cohort.
        # Preserve their existing relative asset IDs in the candidate closure.
        for path in installed_resources.rglob('*.dds'):
            by_name.setdefault((path.parent.name.lower(), path.stem.lower()), []).append(path)
    for key in required:
        package, relative = key.split('.', 1)
        name = relative.rsplit('.', 1)[-1]
        if (package, name) in by_name:
            matches = by_name[(package, name)]
            if resources.resolve() != installed_resources.resolve():
                copied = []
                for match in matches:
                    if match.resolve().is_relative_to(resources.resolve()):
                        copied.append(match)
                        continue
                    destination = resources / match.relative_to(installed_resources)
                    destination.parent.mkdir(parents=True, exist_ok=True)
                    assert not destination.exists() or destination.read_bytes() == match.read_bytes(), destination
                    if not destination.exists():
                        shutil.copyfile(match, destination)
                    copied.append(destination)
                matches = copied
            if not any(path.relative_to(resources).parts[0] == 'Effect' for path in matches):
                # Effect document texture IDs are deliberately Effect-relative.
                # Reuse the exact source bytes, without widening the codec's
                # domain boundary to Character or Map resource trees.
                source = sorted(matches, key=lambda path: (len(str(path)), str(path)))[0]
                destination = resources / 'Effect/KoukuSaydon/FullRestore/Textures' / package / (name + '.dds')
                destination.parent.mkdir(parents=True, exist_ok=True)
                assert not destination.exists() or destination.read_bytes() == source.read_bytes(), destination
                if not destination.exists():
                    shutil.copyfile(source, destination)
            continue
        matches = [p for p in export_root.rglob('*.dds') if p.stem.lower() == name] if export_root.exists() else []
        if not matches:
            command = [str(UMODEL), '-export', '-game=lostark', '-kr', '-nameresolve',
                       f'-path={RELEASE}', f'-out={export_root.resolve()}', '-dds', '-nooverwrite',
                       f'-obj={name}', str(pkg(package).path) if package in _startup_groups else package]
            result = subprocess.run(command, cwd=UMODEL.parent, capture_output=True, text=True,
                encoding='utf8', errors='replace', creationflags=subprocess.CREATE_NO_WINDOW)
            export_root.mkdir(parents=True, exist_ok=True)
            (export_root / (package + '.' + name + '.log')).write_text(result.stdout + result.stderr, encoding='utf8')
            assert result.returncode == 0, (key, result.stdout[-1000:], result.stderr)
            matches = [p for p in export_root.rglob('*.dds') if p.stem.lower() == name]
        if not matches:
            # UModel keeps uncompressed RGBA source textures as TGA even with
            # -dds.  Recontainer the exact exported pixels, never substitute a
            # similarly named/default texture for the source material input.
            tgas = [p for p in export_root.rglob('*.tga') if p.stem.lower() == name]
            if len(tgas) == 1:
                from PIL import Image
                image = Image.open(tgas[0]).convert('RGBA')
                destination = tgas[0].with_suffix('.dds')
                image.save(destination, format='DDS')
                assert Image.open(destination).convert('RGBA').tobytes() == image.tobytes()
                restore.write(tgas[0].with_suffix('.dds.json'), dict(sourceTexture=key,
                    sourceTgaSha256=hashlib.sha256(tgas[0].read_bytes()).hexdigest(),
                    ddsSha256=hashlib.sha256(destination.read_bytes()).hexdigest(),
                    size=list(image.size), mode='LOSSLESS_UMODEL_RGBA_CONTAINER_CONVERSION'))
                matches = [destination]
        assert len(matches) == 1, (key, matches)
        destination = resources / 'Effect/KoukuSaydon/FullRestore/Textures' / package / (name + '.dds')
        destination.parent.mkdir(parents=True, exist_ok=True)
        assert not destination.exists() or destination.read_bytes() == matches[0].read_bytes(), destination
        if not destination.exists():
            shutil.copyfile(matches[0], destination)
    restore.prepare_textures(out, out / 'required_native_textures.json', resource_root=resources)


def prepare_geometry(evidence):
    sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
    import cook_wmodel_geometry_contract as geometry
    resources = ROOT / 'Client/Bin/Resources'
    meshes = sorted({o['sourceMesh'] for o in restore.read(evidence / 'source_occurrences.json') if o['sourceMesh']})
    output = resources / 'Effect/KoukuSaydon/FullRestore/Meshes'
    output.mkdir(parents=True, exist_ok=True)
    converter = ROOT / 'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe'
    manifest = restore.SOURCE / 'CanonicalSource/Effect/Closure/closure.manifest.json'
    rows = []
    for source_mesh in meshes:
        package_name, relative = source_mesh.split('.', 1)
        name = relative.rsplit('.', 1)[-1]
        destination = output / (name + '.wmodel')
        if destination.exists():
            decoded = geometry.parse_geometry_wmodel(destination.read_bytes())
            rows.append(dict(sourceObject=source_mesh, assetId=destination.relative_to(resources).as_posix(),
                bytes=destination.stat().st_size, mode='REUSE_INSTALLED_NATIVE_GEOMETRY',
                vertices=sum(len(s['vertices']) for s in decoded['submeshes'])))
            continue
        gltf = restore.SOURCE / 'EffectRuntimeClosureExports-20260829' / package_name / 'Export' / package_name.upper()
        matches = [p for p in gltf.rglob('*.gltf') if p.stem.lower() == name]
        if not matches:
            export = evidence / 'source_geometry_export' / package_name
            matches = [p for p in export.rglob('*.gltf') if p.stem.lower() == name]
            if not matches:
                export.mkdir(parents=True, exist_ok=True)
                result = subprocess.run([str(UMODEL), '-export', '-game=lostark', '-kr', '-nameresolve',
                    f'-path={RELEASE}', f'-out={export.resolve()}', '-dds', '-gltf', '-nooverwrite',
                    f'-obj={name}', package_name], cwd=UMODEL.parent, capture_output=True,
                    text=True, encoding='utf8', errors='replace', creationflags=subprocess.CREATE_NO_WINDOW)
                (export / (name + '.export.log')).write_text(result.stdout + result.stderr, encoding='utf8')
                assert result.returncode == 0, (source_mesh, result.stdout, result.stderr)
                matches = [p for p in export.rglob('*.gltf') if p.stem.lower() == name]
        assert len(matches) == 1, (source_mesh, matches)
        gltf = matches[0]
        legacy_root = resources / 'Effect/KoukuSaydon/Meshes'
        candidates = [p for p in legacy_root.rglob('*.wmodel') if p.stem.lower() == name]
        if not candidates:
            legacy = evidence / 'legacy_geometry' / (name + '.wmodel')
            legacy.parent.mkdir(parents=True, exist_ok=True)
            if not legacy.exists():
                result = subprocess.run([str(converter), str(gltf), '-o', str(legacy), '--scale', '100', '--no-auto-textures'],
                    capture_output=True, text=True, encoding='utf8', errors='replace', creationflags=subprocess.CREATE_NO_WINDOW)
                (legacy.parent / (name + '.log')).write_text(result.stdout + result.stderr, encoding='utf8')
                assert result.returncode == 0 and legacy.exists(), (source_mesh, result.stdout, result.stderr)
        else:
            assert len(candidates) == 1, candidates
            legacy = candidates[0]
        package = pkg(package_name).path
        observation = evidence / 'geometry_observations' / (name + '.json')
        digest = lambda p: hashlib.sha256(p.read_bytes()).digest()
        restore.write(observation, dict(sourceObject=source_mesh, sourceGltf=str(gltf), legacyWmodel=str(legacy),
            sourceGltfSha256=digest(gltf).hex(), legacyWmodelSha256=digest(legacy).hex(),
            evidence='OBSERVED_SOURCE_GLTF_AND_LEGACY_WMODEL'))
        provenance = geometry.GeometryProvenanceEvidence(source_mesh, digest(manifest), 'OBSERVED_SOURCE_RECEIPT',
            digest(package), digest(converter), digest(observation), digest(observation))
        payload, receipt = geometry.cook_wmodel_geometry_contract(gltf, legacy, provenance)
        destination.write_bytes(payload)
        restore.write(evidence / 'geometry_observations' / (name + '.cook.json'), receipt)
        decoded = geometry.parse_geometry_wmodel(payload)
        rows.append(dict(sourceObject=source_mesh, assetId=destination.relative_to(resources).as_posix(), bytes=len(payload),
            mode='SOURCE_GLTF_WMODEL_GEOMETRY_PARITY', vertices=sum(len(s['vertices']) for s in decoded['submeshes'])))
    restore.write(evidence / 'geometry_installation.json', rows)
    print('Prepared native source geometry', len(rows))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, required=True)
    parser.add_argument('--first', type=int)
    parser.add_argument('--last', type=int)
    parser.add_argument('--prepare-geometry', action='store_true')
    parser.add_argument('--generate-only', action='store_true')
    parser.add_argument('--reuse-native-root', action='append', type=Path, default=[])
    args = parser.parse_args()
    if args.prepare_geometry:
        prepare_geometry(args.evidence_root.resolve())
    else:
        assert args.first is not None and args.last is not None
        if args.generate_only:
            generate_native(args.evidence_root.resolve(), args.first, args.last)
        else:
            prepare(args.evidence_root.resolve(), args.first, args.last,
                    [root.resolve() for root in args.reuse_native_root])
