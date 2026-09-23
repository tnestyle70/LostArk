#!/usr/bin/env python3
"""Compile verified source inputs to the existing Area mapmaterials contract.

The input manifest binds effective MIC parameters, exact texture exports and
component lighting by stable asset/slot IDs. Output is staged authoring data;
build_maptool_scene and Publish-MapAuthoring own scene admission and deployment.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import re
import struct
import sys

import source_map_surface
import source_map_surface_extra
from source_extraction_io import write_pair


class MaterialBuildError(ValueError):
    pass


class TrackedValues(dict):
    """Record exact source parameters consumed by a family mapper."""
    def __init__(self, values, consumed=None, generated=None):
        super().__init__(values)
        self.consumed = consumed if consumed is not None else set()
        self.generated = generated if generated is not None else set()

    def __getitem__(self, key):
        value = super().__getitem__(key)
        if key not in self.generated:
            self.consumed.add(key)
        return value

    def get(self, key, default=None):
        if key in self:
            return self[key]
        return default

    def copy(self):
        return TrackedValues(self, self.consumed, self.generated)

    def update(self, *args, **kwargs):
        replacements = dict(*args, **kwargs)
        self.generated.update(replacements)
        self.consumed.difference_update(replacements)
        super().update(replacements)


def require(condition, message):
    if not condition:
        raise MaterialBuildError(message)


def digest(payload):
    return hashlib.sha256(payload).hexdigest()


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def json_bytes(value):
    return (json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf-8')


def resource_path(root, asset_id):
    require(isinstance(asset_id, str) and '\\' not in asset_id and ':' not in asset_id,
            f'invalid Resources-relative ID: {asset_id!r}')
    relative = PurePosixPath(asset_id)
    require(not relative.is_absolute() and relative.parts and
            all(p not in ('..', '.') for p in relative.parts) and
            asset_id == relative.as_posix(), f'invalid Resources-relative ID: {asset_id!r}')
    result = (root / asset_id).resolve()
    require(result.is_relative_to(root.resolve()), f'resource escapes root: {asset_id}')
    return result


def verified_file(item, base):
    path = Path(item['path'])
    if not path.is_absolute():
        path = base / path
    path = path.resolve()
    payload = path.read_bytes()
    require(digest(payload) == item['sha256'], f'evidence hash mismatch: {path}')
    return path, payload


def check_texture(item, root):
    path = resource_path(root, item['assetId'])
    payload = path.read_bytes()
    require(digest(payload) == item['sha256'], f'texture hash mismatch: {item["assetId"]}')
    require(len(payload) >= 128 and payload[:4] == b'DDS ', f'not DDS: {path}')
    require(struct.unpack_from('<I', payload, 4)[0] == 124, f'invalid DDS header: {path}')
    actual_mips = max(1, struct.unpack_from('<I', payload, 28)[0])
    height, width = struct.unpack_from('<II', payload, 12)
    require(width > 0 and height > 0 and actual_mips <= max(width, height).bit_length(),
            f'invalid DDS dimensions/mip count: {path}')
    fourcc = payload[84:88]
    offset, faces = 128, 6 if struct.unpack_from('<I', payload, 112)[0] & 0x200 else 1
    block_bytes = {b'DXT1': 8, b'DXT3': 16, b'DXT5': 16, b'ATI1': 8, b'ATI2': 16}.get(fourcc)
    pixel_bytes = None
    if fourcc == b'DX10':
        require(len(payload) >= 148, f'truncated DDS DX10 header: {path}')
        fmt, dimension, flags, array_size, _ = struct.unpack_from('<5I', payload, 128)
        require(dimension == 3 and array_size == 1, f'unsupported DDS resource type: {path}')
        offset, faces = 148, 6 if flags & 4 else 1
        block_bytes = {71: 8, 72: 8, 74: 16, 75: 16, 77: 16, 78: 16,
                       80: 8, 81: 8, 83: 16, 84: 16}.get(fmt)
        pixel_bytes = {2: 16, 10: 8, 16: 8, 28: 4, 29: 4, 34: 4, 41: 4,
                       49: 2, 61: 1, 87: 4, 91: 4}.get(fmt)
    elif fourcc == b'\0\0\0\0':
        bits = struct.unpack_from('<I', payload, 88)[0]
        if bits in (8, 16, 24, 32):
            pixel_bytes = bits // 8
    require(block_bytes is not None or pixel_bytes is not None, f'unsupported DDS format: {path}')
    chain_bytes = sum(
        ((max(1, width >> level) + 3) // 4) * ((max(1, height >> level) + 3) // 4) * block_bytes
        if block_bytes else max(1, width >> level) * max(1, height >> level) * pixel_bytes
        for level in range(actual_mips))
    require(len(payload) == offset + faces * chain_bytes, f'truncated/trailing DDS payload: {path}')
    require(type(item.get('mipCount')) is int and actual_mips == item['mipCount'],
            f'source mip count missing or mismatched: {path}')
    require(item.get('colorSpace') in ('linear', 'srgb'), f'unknown texture color space: {path}')
    require(item.get('mipEvidence') in ('source-chain', 'source-unmipped', 'project-generated'),
            f'source mip evidence missing: {path}')
    require(item['mipEvidence'] != 'source-unmipped' or actual_mips == 1,
            f'source-unmipped texture has multiple mips: {path}')
    return dict(assetId=item['assetId'], sha256=item['sha256'], mipCount=actual_mips,
                colorSpace=item['colorSpace'], mipEvidence=item['mipEvidence'],
                width=width, height=height, faces=faces)


def check_finite(value, context):
    if isinstance(value, dict):
        for key, child in value.items():
            check_finite(child, f'{context}.{key}')
    elif isinstance(value, list):
        for child in value:
            check_finite(child, context)
    elif isinstance(value, float):
        require(math.isfinite(value) and abs(value) <= 3.4028234663852886e38,
                f'non-finite or out-of-float-range material input: {context}')


def validate_surface(row, resources):
    """Bounds shared by the existing CMapAssetCatalog surface carriers."""
    check_finite(row, row['assetId'])
    for key, value in row.items():
        if type(value) in (int, float) and key != 'sourceIceBumpOffset':
            require(value >= 0, f'{key} must be non-negative')
        if key.endswith('Color'):
            require(isinstance(value, list) and len(value) == 4 and all(type(x) in (int, float) and x >= 0 for x in value),
                    f'{key} must be four non-negative components')
    for key in ('uvTiling',):
        if key in row:
            require(len(row[key]) == 2 and all(type(x) in (int, float) and x > 0 for x in row[key]),
                    f'{key} must have two positive components')
    for key in ('reflectionTiling', 'detailNormalTiling', 'overlayTiling', 'sourceNormalTiling'):
        if key in row:
            require(row[key] > 0, f'{key} must be positive')
    if 'minimumRoughness' in row:
        require(0 < row['minimumRoughness'] <= 1, 'minimumRoughness must be in (0, 1]')
        require(0 <= row['vertexAlpha'] <= 1, 'vertexAlpha must be in [0, 1]')
    for key in ('normal', 'detailNormal', 'overlayNormal'):
        if key in row['textureColorSpace']:
            require(row['textureColorSpace'][key] == 'linear', f'{key} must be linear')
    if 'bakedLighting' in row:
        baked = row['bakedLighting']
        require(set(baked) == {'averageTexture', 'directionalTexture', 'colorSpace'}, 'invalid RNM fields')
        require(baked['colorSpace'] in ('linear', 'srgb') and all(
            resources[baked[key]]['colorSpace'] == baked['colorSpace']
            for key in ('averageTexture', 'directionalTexture')), 'RNM texture color space mismatch')
    if 'environment' in row:
        env = row['environment']
        fields = {'cubeTexture', 'brdfTexture', 'color', 'rotation'}
        require(isinstance(env, dict), 'invalid environment fields')
        if 'sourceIndirect' in env:
            fields.add('sourceIndirect')
        if 'legacyEnabled' in env:
            fields.add('legacyEnabled')
        require(set(env) == fields, 'invalid environment fields')
        if 'legacyEnabled' in env:
            require(type(env['legacyEnabled']) is bool and (env['legacyEnabled'] or 'sourceIndirect' in env),
                    'invalid environment legacyEnabled or missing source indirect inputs')
        require(len(env['color']) == 4 and all(type(x) in (int, float) and x >= 0 for x in env['color']) and
                len(env['rotation']) == 2 and all(type(x) in (int, float) for x in env['rotation']) and
                abs(sum(x*x for x in env['rotation']) - 1) < 0.001, 'invalid environment color/rotation')
        require(resources[env['cubeTexture']]['faces'] == 6 and resources[env['brdfTexture']]['faces'] == 1,
                'environment needs a cube and a 2D BRDF texture')
        if 'sourceIndirect' in env:
            require(row['family'] in ('bg_base_pbr_opa', 'bg_base_pbr_seamless_opa', 'bg_base_pbr_msk'),
                    'source indirect requires a PBR environment')
            indirect = env['sourceIndirect']
            require(isinstance(indirect, dict) and set(indirect) == {
                'model', 'cubeTexture', 'brdfTexture', 'color', 'rotation', 'packedSH', 'upperSkyColor', 'lowerSkyColor', 'ambientAndSkyFactor'},
                'invalid source indirect fields')
            require(indirect['model'] == 'UE3_NATIVE_PBR', 'invalid source indirect model')
            cube = indirect['cubeTexture']
            require(isinstance(cube, str) and cube in resources and resources[cube]['faces'] == 6 and
                    resources[cube]['colorSpace'] == 'linear', 'source indirect needs a verified linear cube texture')
            brdf = indirect['brdfTexture']
            require(isinstance(brdf, str) and brdf in resources and resources[brdf]['faces'] == 1 and
                    resources[brdf]['colorSpace'] == 'linear', 'source indirect needs a verified linear 2D BRDF texture')
            color, rotation = indirect['color'], indirect['rotation']
            require(isinstance(color, list) and len(color) == 4 and all(
                type(value) in (int, float) and math.isfinite(value) and 0 <= value <= 3.4028234663852886e38
                for value in color), 'invalid source indirect color')
            require(isinstance(rotation, list) and len(rotation) == 2 and all(
                type(value) in (int, float) and math.isfinite(value) and -1 <= value <= 1 for value in rotation) and
                abs(sum(value*value for value in rotation)-1) <= 0.0001, 'invalid source indirect rotation')
            packed = indirect['packedSH']
            require(isinstance(packed, list) and len(packed) == 7 and all(
                isinstance(sh_row, list) and len(sh_row) == 4 and all(
                    type(value) in (int, float) and math.isfinite(value) and -64 <= value <= 64
                    for value in sh_row) for sh_row in packed), 'invalid source indirect SH')
            require(packed[6][3] == 1, 'source indirect SH reserved w must be one')
            for key, count in (('upperSkyColor', 3), ('lowerSkyColor', 3), ('ambientAndSkyFactor', 4)):
                values = indirect[key]
                require(isinstance(values, list) and len(values) == count and all(
                    type(value) in (int, float) and math.isfinite(value) and
                    0 <= value <= (4 if key == 'ambientAndSkyFactor' and index == 3 else 64)
                    for index, value in enumerate(values)), f'invalid source indirect vector: {key}')
    if 'emissive' in row:
        e = row['emissive']
        require(e['intensity'] >= 0 and len(e['color']) == 4 and all(x >= 0 for x in e['color']) and
                len(e['uvTiling']) == 2 and all(x > 0 for x in e['uvTiling']) and
                0 <= e['flicker']['minimum'] <= 1 and e['flicker']['speed'] >= 0, 'invalid emissive bounds')
        if 'mode' in e['flicker']:
            require(row['family'] in ('bg_base_pbr_opa','bg_base_pbr_seamless_opa','bg_base_pbr_msk') and
                    e['flicker']['mode'] in ('none','nested'), 'invalid PBR emissive mode')
            if e['flicker']['mode'] == 'none':
                require(all(e['flicker'][key] == 0 for key in ('minimum','speed','phaseOffset')),
                        'steady PBR emissive has phase inputs')


def covered_render_flags(render, row):
    source_flags = render.get('sourceFlags', {})
    covered = []
    masked = row['family'].endswith(('_msk', '-masked'))
    if row['family'] == 'bg-source-opaque-masked':
        masked = bool(row['sourceFlags'] & 64)
    if row['family'] == 'bg_base_opa_overlay':
        masked = bool(row['sourceOverlayFlags'] & 64)
    for key, value in source_flags.items():
        if key == 'blendMode':
            require(value == ('BLEND_Masked' if masked else 'BLEND_Opaque'), 'source BlendMode branch mismatch')
        elif key == 'twoSided':
            require(type(value) is bool and value == (render['cullMode'] == 'none'), 'source TwoSided mismatch')
        elif key == 'isMasked':
            require(type(value) is bool and value == masked, 'source isMasked mismatch')
        elif key == 'disableDepthTest':
            require(value is False, 'source disabled depth test is unsupported')
        elif key == 'opacityMaskClipValue':
            require(type(value) in (int, float) and 0 <= value <= 1 and
                    (not masked or abs(value - 0.3333) <= 0.0001), 'source opacity clip threshold is unsupported')
        else:
            continue
        covered.append('renderFlag:' + key)
    return covered


def compile_materials(manifest_path, resources_root):
    manifest_path = manifest_path.resolve()
    manifest = read_json(manifest_path)
    require(manifest.get('format') == 'lostark-source-map-material-input' and
            manifest.get('formatVersion') == 1, 'unsupported source material input version')
    area = manifest['areaId']
    require(re.fullmatch(r'[A-Za-z0-9_.-]+', area) is not None, 'invalid areaId')
    sources = [dict(path=str(manifest_path), sha256=digest(manifest_path.read_bytes()))]
    parameter_path, parameter_bytes = verified_file(manifest['parameters'], manifest_path.parent)
    parameters = json.loads(parameter_bytes)
    require(parameters.get('format') == 'lostark-source-map-material-parameters' and
            parameters.get('formatVersion') == 1 and not parameters.get('failures'),
            'effective material extraction is incomplete or unsupported')
    for item in [manifest['parameters'], *parameters.get('sources', []), *manifest.get('evidence', [])]:
        path, payload = verified_file(item, manifest_path.parent)
        sources.append(dict(path=str(path), sha256=digest(payload)))
    require(manifest.get('evidence'), 'slot/component source evidence is required')
    component_lighting = None
    if manifest.get('componentLighting'):
        path, payload = verified_file(manifest['componentLighting'], manifest_path.parent)
        sources.append(dict(path=str(path), sha256=digest(payload)))
        component_lighting = json.loads(payload)
        require(component_lighting.get('format') == 'lostark-source-map-component-lighting' and
                component_lighting.get('formatVersion') == 1 and component_lighting.get('areaId') == area and
                not component_lighting.get('failures'), 'invalid component lighting extraction')
        package_path, package_bytes = verified_file(component_lighting['source'], manifest_path.parent)
        sources.append(dict(path=str(package_path), sha256=digest(package_bytes)))
    textures = {}
    resources = {}
    for item in manifest['textures']:
        full = item['sourceObject'].casefold()
        require('.' in full and full not in textures, f'duplicate/invalid texture source: {full}')
        resources[item['assetId']] = check_texture(item, resources_root)
        textures[full] = item
    for item in manifest.get('auxiliaryTextures', []):
        record = check_texture(item, resources_root)
        require(record['assetId'] not in resources or resources[record['assetId']] == record,
                f'conflicting resource declaration: {record["assetId"]}')
        resources[record['assetId']] = record
    rows, bindings, seen, used_resources = [], {}, set(), set()
    approximations = copy.deepcopy(manifest.get('projectApproximations', []))
    for slot in manifest['slots']:
        asset, name, source = slot['assetId'], slot['materialName'], slot['sourceMaterial'].casefold()
        require(re.fullmatch(r'[A-Za-z0-9_.:-]+', asset) is not None and bool(name), 'invalid asset/slot ID')
        require((asset, name) not in seen, f'duplicate material slot: {asset}/{name}')
        seen.add((asset, name))
        resolved = parameters['materials'][source]
        require(resolved.get('sourceMaterial', source).casefold() == source, 'source material mismatch')
        terminal = resolved['terminal'].casefold()
        require('.' in source and '.' in terminal, 'full source material paths are required')
        require(all(type(value) is bool for value in resolved['switches'].values()),
                f'{source}: static switches must be booleans')
        require(not resolved.get('unresolvedSwitches'), f'{source}: unresolved static switches')
        # Source inactive defaults are separate from missing active graph input.
        missing_switches = [x for x in resolved.get('unresolvedDefaults', [])
                            if 'switch' in str(x).casefold()]
        require(not missing_switches, f'{source}: unresolved switch defaults: {missing_switches}')
        require(not any(v for k, v in resolved['switches'].items() if 'dynamicfoliage' in k),
                f'{source}: source foliage vertex wind has no verified binding in this compiler')
        consumed_textures = set()
        def texture(parameter):
            source_object = resolved['textures'].get(parameter)
            fallback = slot.get('textureFallbacks', {}).get(parameter)
            if source_object is None and fallback:
                evidence_path, evidence_bytes = verified_file(fallback['evidence'], manifest_path.parent)
                require(type(fallback.get('expressionIndex')) is int and fallback['expressionIndex'] >= 0,
                        f'{source}: native fallback expression index required')
                sources.append(dict(path=str(evidence_path), sha256=digest(evidence_bytes)))
                native = json.loads(evidence_bytes)
                require(native.get('sourceMaterial', '').casefold() == source and any(
                    entry.get('expressionIndex') == fallback['expressionIndex'] and
                    entry.get('parameterName') == parameter and
                    entry.get('sourceObject', '').casefold() == fallback['sourceObject'].casefold()
                    for entry in native.get('textures', [])),
                    f'{source}: native texture fallback does not match source expression')
                source_object = fallback['sourceObject']
            require(source_object is not None, f'{source}: active texture {parameter} is NULL or unresolved')
            item = textures.get(source_object.casefold())
            require(item is not None, f'{source}: missing exact texture {source_object}')
            used_resources.add(item['assetId'])
            if parameter in ('texture_normal', 'texture_detail_normal', 'texture_overlay_normal'):
                require(item['colorSpace'] == 'linear', f'{parameter}: normal must be linear')
            consumed_textures.add(parameter)
            return item['assetId'], item['colorSpace']
        builder = source_map_surface_extra if terminal.endswith((
            '.bg_base_pbr_opa', '.bg_base_pbr_seamless_opa', '.bg_base_pbr_msk',
            '.bg_foliage_msk', '.bg_grass_msk', '.preset_overlay_snowice_opa')) else source_map_surface
        tracked = TrackedValues(resolved['values'])
        row = builder.build_surface(source, terminal, tracked, resolved['switches'],
                                    texture, asset_id=asset, material_name=name)
        component = slot['component']
        render = component['rendering']
        require(type(render['castsShadow']) is bool and render['renderMode'] == 'deferred' and
                render['cullMode'] in ('back', 'front', 'none'), f'{asset}: unsupported source render properties')
        row.update({k: render[k] for k in ('castsShadow', 'renderMode', 'cullMode')})
        require(component.get('lightingEvidence') in ('source-bound', 'source-absent'),
                f'{asset}: component lighting evidence is required')
        if component['lightingEvidence'] == 'source-absent':
            require(not component.get('bakedLighting') and not component.get('environment'),
                    f'{asset}: source-absent lighting has bindings')
        for field in ('bakedLighting', 'environment'):
            if component.get(field):
                row[field] = copy.deepcopy(component[field])
                for key, value in row[field].items():
                    if key.endswith('Texture'):
                        require(value in resources, f'{asset}: unverified {field} resource: {value}')
                        used_resources.add(value)
        if 'minimumRoughness' in component:
            row['minimumRoughness'] = component['minimumRoughness']
        if row['family'].startswith('bg_base_pbr'):
            require('minimumRoughness' in component and component.get('minimumRoughnessEvidence') in
                    ('source-bound', 'project-authored'), f'{asset}: minimum roughness evidence required')
            require(component.get('environmentEvidence') in ('source-bound', 'source-absent'),
                    f'{asset}: explicit environment evidence required for PBR')
            require(bool(row.get('environment')) == (component['environmentEvidence'] == 'source-bound'),
                    f'{asset}: environment evidence/binding mismatch')
            approximations.append(dict(assetId=asset, materialName=name,
                note='Carrier defaults: vertexAlpha=1, reflectionOriginOffset=[0,0]; minimumRoughness requires component evidence. Scene SH, native BRDF and dynamic MIC values are separate inputs.'))
        validate_surface(row, resources)
        if 'sourceIndirect' in row.get('environment', {}):
            used_resources.add(row['environment']['sourceIndirect']['cubeTexture'])
            used_resources.add(row['environment']['sourceIndirect']['brdfTexture'])
        rows.append(row)
        binding = bindings.setdefault(asset, dict(assetId=asset, materialNames=[], sourceMaterials=[],
            materialCoverage=[]))
        binding['materialNames'].append(name)
        binding['sourceMaterials'].append(source)
        covered = [('vector:' if isinstance(resolved['values'][key], list) else 'scalar:') + key
                   for key in sorted(tracked.consumed) if key in resolved['values']]
        covered += ['texture:' + key for key in sorted(consumed_textures)]
        covered += covered_render_flags(render, row)
        binding['materialCoverage'].append(dict(materialName=name, sourceMaterial=source,
                                               coveredSourceOnly=sorted(covered)))
    require(rows, 'no source material slots supplied')
    lighting = copy.deepcopy(manifest.get('placementLighting', []))
    seen_lighting = set()
    auxiliary_sources = {x['assetId']: x.get('sourceObject', '').casefold()
                         for x in manifest.get('auxiliaryTextures', [])}
    for instance in lighting:
        pair = instance['sourcePlacementId'], instance['assetId']
        require(pair not in seen_lighting and instance['assetId'] in bindings,
                f'duplicate/unbound placement lighting: {pair}')
        seen_lighting.add(pair)
        required = {'coordinateScale': 2, 'coordinateBias': 2, 'averageScale': 3, 'directionalScale': 3}
        for field, size in required.items():
            require(len(instance[field]) == size, f'invalid RNM {field}: {pair}')
        require(all(row.get('bakedLighting') for row in rows if row['assetId'] == instance['assetId']),
                f'RNM instance has no texture pair: {pair}')
        check_finite(instance, str(pair))
        if component_lighting:
            original = component_lighting['components'].get(instance['sourcePlacementId'])
            require(original is not None and original['status'] == 'RNM_TEXTURE_LIGHTMAP',
                    f'RNM source component is missing/unsupported: {pair}')
            native = original['lighting']
            require(native['nativeTailCompletelyConsumed'] and all(
                instance[key] == native[key] for key in required), f'RNM coordinate/scale source mismatch: {pair}')
            for material in rows:
                if material['assetId'] != instance['assetId']:
                    continue
                require(all(auxiliary_sources.get(material['bakedLighting'][key]) == native[key].casefold()
                            for key in ('averageTexture', 'directionalTexture')), f'RNM texture source mismatch: {pair}')
    output = dict(schema='lostark.map-materials', formatVersion=2, areaId=area,
                  materials=rows, placementLighting=lighting)
    receipt = dict(format='lostark-source-map-material-build', formatVersion=1, areaId=area,
        bindings=list(bindings.values()), resources=[resources[k] for k in sorted(used_resources)],
        inputs=sources, failures=[], runtimeMaterialInputsComplete=True,
        originalVisualFidelityVerified=False, projectApproximations=approximations,
        unresolvedRuntimeInputs=['Original dynamic MIC scalar updates', 'Scene SH/hemisphere lighting and native BRDF must be verified separately'])
    return output, receipt


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    for field in ('input', 'resources-root', 'output', 'receipt'):
        parser.add_argument('--' + field, type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        require(args.output.resolve() != args.input.resolve() and args.receipt.resolve() != args.input.resolve(),
                'input must not be overwritten')
        output, receipt = compile_materials(args.input, args.resources_root)
        source_paths = {Path(item['path']).resolve() for item in receipt['inputs']}
        source_paths.update(resource_path(args.resources_root, item['assetId']) for item in receipt['resources'])
        require(args.output.resolve() not in source_paths and args.receipt.resolve() not in source_paths,
                'source evidence/resources must not be overwritten')
        payload = json_bytes(output)
        receipt['output'] = dict(path=str(args.output.resolve()), sha256=digest(payload))
        write_pair(args.output, payload, args.receipt, json_bytes(receipt))
    except (MaterialBuildError, KeyError, TypeError, ValueError, OSError) as error:
        print(f'source map material build failed: {error}', file=sys.stderr)
        return 1
    print(f'{output["areaId"]}: {len(output["materials"])} source slots staged; original visual fidelity remains unverified')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
