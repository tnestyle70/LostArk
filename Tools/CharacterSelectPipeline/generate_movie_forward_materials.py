"""Emit native movie forward cases for the existing static-map forward renderer."""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


def read(path):
    return json.loads(path.read_bytes())


def scene_inputs(source, program, stage):
    # Native depth-fade PSs use device depth followed by cb2[1] reconstruction.
    # The two post meshes sample scene color in the original DXBC.
    lines = source.splitlines()
    for i, line in enumerate(lines):
        if not line.lstrip().startswith('//') or 'sample' not in line:
            continue
        instruction = re.sub(r'^\s*//\s*(?:\d+:\s*)?', '', line)
        if i+1 >= len(lines) or 'float4(0.0,0.0,0.0,0.0)' not in lines[i+1]:
            continue
        match = re.search(r'\s(r\d+\.[xyzw]+),\s*([^,]+),\s*t(\d+)\.', instruction)
        if not match:
            continue
        coordinate = match[2]
        replacement = None
        if program in (1510, 1511) and stage == 'Base':
            replacement = f'g_SourceMapSceneColor.SampleLevel(SourceMapDepthSampler,({coordinate}).xy,0.f)'
        elif 'sample_l_' in instruction and 'texture2d' in instruction and 'cb2[1]' in source:
            replacement = f'g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,({coordinate}).xy,0.f).rrrr'
        if replacement:
            lines[i+1] = lines[i+1].replace('float4(0.0,0.0,0.0,0.0)', replacement)
    source = '\n'.join(lines)+'\n'
    # Pass row 1 is the original cm-based device-depth reconstruction, not a MIC parameter.
    source = source.replace('float4(.5,-.5,.5,.5),float4(0,0,0,0)',
        'float4(.5,-.5,.5,.5),float4(0,0,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f))')
    return source


def generate(root, output):
    metadata = {}
    for folder in ['artist-dimensionmaster/background-native', 'lance-warlord/background-native',
                   'actor-material-candidates/props', 'lance-warlord/static-native']:
        for row in read(root/folder/'native/native_material_inputs.json')['materials']:
            metadata[row['sourceMaterial']] = row
    programs = []
    text = ['// Original movie MIC functions, consumed by the existing map forward pass.',
            '#ifndef SOURCE_MOVIE_STATIC_FORWARD_INCLUDED', '#define SOURCE_MOVIE_STATIC_FORWARD_INCLUDED']
    for folder in ['artist-dimensionmaster/background-native', 'lance-warlord/background-native', 'completion/props']:
        directory = root/folder
        generated = directory if folder == 'completion/props' else directory/'generated'
        for row in read(generated/'receipt.json')['programs']:
            properties = metadata[row['materials'][0]]['parentProperties']
            blend = properties.get('blendmode', {}).get('value', 'blend_opaque')
            if blend not in ('blend_translucent', 'blend_additive'):
                continue
            for mic in row['materials']:
                other = metadata[mic]['parentProperties'].get('blendmode', {}).get('value', 'blend_opaque')
                if other != blend:
                    raise ValueError('One native program has incompatible render modes')
            n = row['program']
            row = dict(row, blend=blend, lighting=properties.get('lightingmodel', {}).get('value', 'mlm_phong'))
            programs.append(row)
            stages = ['Base', 'Light']
            if folder != 'completion/props' and (generated/str(n)/'baked.hlsli').exists():
                stages.insert(0, 'Baked')
            for stage in stages:
                path = generated/f'{stage.lower()}{n}.hlsli' if folder == 'completion/props' else generated/str(n)/(stage.lower()+'.hlsli')
                if not path.exists() and stage == 'Light' and row['lighting'] == 'mlm_unlit':
                    source = f'SOURCE_CHARACTER_NATIVE_OUTPUT SourceMovieLight{n}(SOURCE_CHARACTER_NATIVE_INPUT input) {{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }}'
                else:
                    source = path.read_text(encoding='utf8')
                source = re.sub(r'\bSourceCharacter(Base|Light)'+str(n)+r'\b',
                                lambda match: 'SourceMovie'+match[1]+str(n), source)
                source = source.replace('SourceMapMonsterBaked'+str(n), 'SourceMovieBaked'+str(n))
                if stage == 'Base' and 'Baked' in stages:
                    # Baked and no-lightmap share material UV packing; their baked samples are explicit inputs.
                    at = source.index('{')+1
                    source = source[:at]+f'\n    if(input.hasBakedLighting) return SourceMovieBaked{n}(input);'+source[at:]
                text.append(scene_inputs(source, n, stage))
    for name, selected in [('IsSourceMovieForward', programs), ('IsSourceMovieLitForward', [p for p in programs if p['lighting'] != 'mlm_unlit'])]:
        text += [f'bool {name}()', '{', '    return '+ ' || '.join(f'g_SourceCharacterProgram=={p["program"]}u' for p in selected)+';', '}']
    for stage in ['Base', 'Light']:
        text += [f'SOURCE_CHARACTER_NATIVE_OUTPUT EvaluateSourceMovie{stage}(SOURCE_CHARACTER_NATIVE_INPUT input)', '{', '    switch(g_SourceCharacterProgram)', '    {']
        for p in programs:
            text += [f'    case {p["program"]}u: return SourceMovie{stage}{p["program"]}(input);']
        text += ['    }', '    return (SOURCE_CHARACTER_NATIVE_OUTPUT)0;', '}']
    text += ['#endif', '']
    output.mkdir(parents=True, exist_ok=True)
    (output/'Shader_SourceMovieStaticForward.hlsli').write_text('\n'.join(text), encoding='utf8')
    (output/'forward-material-receipt.json').write_text(json.dumps(programs, indent=2), encoding='utf8')
    header = ['#pragma once', '#include <cstdint>', '', 'namespace Client::SourceMovieMaterial', '{']
    for name, selected in [('Is_Forward', programs), ('Is_Additive', [p for p in programs if p['blend']=='blend_additive']), ('Needs_SceneColor', [p for p in programs if p['program'] in (1510,1511)])]:
        header += [f'inline bool {name}(uint32_t program)', '{', '    return '+ ' || '.join(f'program=={p["program"]}u' for p in selected)+';', '}']
    header += ['inline bool Is_Static(uint32_t program)', '{', '    return (program>=1100u && program<=1166u) || (program>=1400u && program<=1413u) || (program>=1500u && program<=1525u);', '}', '}', '']
    (output/'SourceMovieMaterialPrograms.h').write_text('\n'.join(header), encoding='utf8')
    print('Native forward programs:', len(programs))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    generate(args.root, args.output)
