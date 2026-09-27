#!/usr/bin/env python3
"""Stage ocean_trn bindings for the existing source.map.water-42 pixel program.

Copies effective MIC/master values and seven texture lanes without colour or
tiling approximations. This is a pixel binding, not proof that the source vertex
wave/static permutation is reproduced. Review source switches and the candidate,
then install through the Area publisher. Never write over the source in place.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import build_source_map_material_inputs as adapter_module
from source_extraction_io import write_atomic

FAMILY = 'source.map.water-42.v1'
PARAMETER_NAMES = (
    'selectioncolor', 'normal_tiling_panning', 'detail_normal_tiling_panning',
    'sky_color', 'sky_tiling_panning', 'reflection_color', 'reflection_tiling_panning',
    'fresnel_color', 'diffuse_color', 'diffuse_tiling_panning', 'normal_intensity',
    'normal_distortion_intensity', 'detail_normal_intensity', 'fresnel_power',
    'distortion_intensity', 'sky_power', 'sky_intensity', 'reflection_uv',
    'reflection_power', 'reflection_intensity', 'fresnel_tiling', 'fresnel_intensity',
    'mask_distortion_intensity', 'diffuse_saturation', 'specular_intensity',
    'specular_power', 'depth_bias', 'opacity', 'opacity_power', 'screen_distortion_intensity')
TEXTURE_LANES = (
    'texture_normal', 'detail_texture_normal', 'texture_sky', 'texture_reflection',
    'texture_fresnel', 'texture_diffuse', 'texture_diffuse_mask')
OCEAN_TERMINAL = 'specialresource.mat.ocean_trn'
# Source SRGB=false was checked directly for these engine textures, which have
# no recovery receipt. Other normals still require their source receipt.
VERIFIED_LINEAR = {
    'efmaster_material_prologue.tex.t_snow_normal', 'specialresource.tex.waterbump_tex',
    # EFMaster_Material_Prologue exports: PF_BC5 / TC_NormalmapBC5 / SRGB=false.
    'efmaster_material_prologue.tex.fx_a_water_144_n',
    'efmaster_material_prologue.tex.base_normal_01',
}


def build_row(asset_id, slot, resolved, texture):
    if resolved['terminal'].casefold() != OCEAN_TERMINAL:
        raise ValueError('not an ocean_trn material: ' + slot['objectPath'])
    parameters = {name: resolved['values'][name] for name in PARAMETER_NAMES
                  if name != 'selectioncolor'}
    parameters['selectioncolor'] = [0.0, 0.0, 0.0, 1.0]  # Editor selection disabled.
    textures = []
    for index, lane in enumerate(TEXTURE_LANES):
        source = resolved['textures'][lane]
        item = texture(source)
        color = item['colorSpace']
        if index < 2:
            if source.casefold() in VERIFIED_LINEAR:
                color = 'linear'
            if color != 'linear':
                raise ValueError('normal has no source linear evidence: ' + source)
        textures.append(dict(expressionIndex=index, assetId=item['assetId'], colorSpace=color))
    return dict(assetId=asset_id, materialName=slot['runtimeName'],
                sourceMaterial=slot['objectPath'].casefold(), family=FAMILY,
                parameters=parameters, textures=textures,
                renderMode='translucent', cullMode='back')


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('materials', 'output', 'mapwater', 'placements', 'parameters',
                 'runtime-manifest', 'mip-catalog', 'mip-results', 'mip-chains',
                 'asset-inventory', 'imported-catalog', 'resources-root', 'new-resources-root'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--area-id', required=True)
    args = parser.parse_args(argv)
    if args.output.resolve() == args.materials.resolve():
        raise ValueError('output must be a separate candidate, not the authoring source')
    args.runtime_area_root = 'Map/' + args.area_id
    args.engine_default_dir = 'Map/' + args.area_id + '_SOURCE_MATERIALS/EngineDefaults'
    adapter = adapter_module.Adapter(args)
    document = adapter_module.read_json(args.materials)
    waters = adapter_module.read_json(args.mapwater)['waters']
    runtime = {a['assetId']: a for a in adapter.runtime['assets']}
    existing = {(r['assetId'], r['materialName']): i for i, r in enumerate(document['materials'])}
    if len(existing) != len(document['materials']):
        raise ValueError('duplicate material identities in input')
    for water in waters:
        asset_id = water['assetId']
        slots = runtime[asset_id]['materials']
        if len(slots) != 1 or not slots[0].get('objectPath'):
            raise ValueError('water requires one resolved source slot: ' + asset_id)
        slot = slots[0]
        resolved = adapter.materials[slot['objectPath'].casefold()]
        row = build_row(asset_id, slot, resolved, adapter.texture)
        key = asset_id, slot['runtimeName']
        index = existing.get(key)
        if index is None:
            existing[key] = len(document['materials'])
            document['materials'].append(row)
        else:
            if document['materials'][index].get('family') not in ('source.map.water-41.v1', FAMILY):
                raise ValueError('refusing unrelated material family: ' + asset_id)
            document['materials'][index] = row
        print(asset_id, 'source switches:', resolved['switches'])
    payload = json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n'
    write_atomic(args.output, payload.encode('utf-8'))
    print('staged', len(waters), 'water rows;', len(document['materials']), 'total; visual validation pending')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
