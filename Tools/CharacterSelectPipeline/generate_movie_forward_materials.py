"""Emit native movie forward cases for the existing static-map forward renderer."""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


def read(path):
    return json.loads(path.read_bytes())


def primitive_inputs(source, program, stage):
    """Restore reviewed engine inputs even when opacity is consumed before o0.w.

    These exact LocalVF shaders have the same external opacity/mesh-color ABI
    as their admitted Effect versions. World actors have no particle tint, so
    their primitive color is identity; their MIC colors and curves stay intact.
    Other unowned rows (including distance-fade and world-position inputs) are
    deliberately outside this contract.
    """
    reviewed = {
        1500: ('f0d24fec647e5c4195be3c40a23b612c', False),
        1501: ('f0d24fec647e5c4195be3c40a23b612c', False),
        1502: ('abd407a677b45447981a5b9fc023037c', False),
        1506: ('8b228c7b319b544781cca408746673a2', True),
        1516: ('29f037b116fdaa43b77ca3573d14d7fa', False),
        1520: ('6e7b9a08f01bef4ca32ebf014794bb5f', False),
        1524: ('f19b3d3238562c47b3da05758da321c5', True),
    }
    if stage != 'Base' or program not in reviewed:
        return source
    shader, mesh_color = reviewed[program]
    if f'/ source program {shader}' not in source:
        raise ValueError(f'Movie primitive-input source shader changed: {program}')
    if not re.search(r'// \d+: mul r\d+\.\w+, r\d+\.\w+, cb0\[0\]\.x', source):
        raise ValueError(f'Movie opacity instruction changed: {program}')
    if re.search(r'^\s*source\[0\](?:\.[xyzw]+)?\s*=', source, re.M):
        raise ValueError(f'Movie primitive opacity is already owned: {program}')
    assignments = '    source[0].x=1.f; // Native external primitive opacity.\n'
    if mesh_color:
        if re.search(r'^\s*source\[1\](?:\.[xyzw]+)?\s*=', source, re.M):
            raise ValueError(f'Movie primitive color is already owned: {program}')
        assignments += '    source[1]=1.f; // World primitive RGBA identity; MIC colors are unchanged.\n'
    marker = '    float4 projection[4];'
    if source.count(marker) != 1:
        raise ValueError(f'Movie primitive-input insertion is ambiguous: {program}')
    return source.replace(marker, assignments + marker)


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


def world_camera_inputs(source, program, stage):
    """Restore the reviewed stone LocalVF world/camera PS contract."""
    reviewed = {
        (1518, 'Base'): 'e5ff54c5c354204e951b91b524341cb3',
        (1518, 'Light'): 'acc28065f1b00341a5874feb06c9082f',
    }
    shader = reviewed.get((program, stage))
    if shader is None:
        return source
    if f'/ source program {shader}' not in source:
        raise ValueError(f'Movie camera-input source shader changed: {program} {stage}')
    if re.search(r'^\s*source\[1\](?:\.[xyzw]+)?\s*=', source, re.M):
        raise ValueError(f'Movie camera input is already owned: {program} {stage}')
    if not re.search(r'// \d+: add r\d+\.\w+, r\d+\.\w+, -cb0\[1\]\.yzxy', source):
        raise ValueError(f'Movie camera subtraction changed: {program} {stage}')
    marker = '    float4 projection[4];'
    if source.count(marker) != 1:
        raise ValueError(f'Movie camera-input insertion is ambiguous: {program} {stage}')
    return source.replace(marker,
        '    source[1]=float4(input.sourceCameraPosition,1.f); // Original source-camera position; row 0 is neutral pre-view translation.\n' + marker)


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
                text.append(scene_inputs(primitive_inputs(source, n, stage), n, stage))
    for name, selected in [('IsSourceMovieForward', programs), ('IsSourceMovieLitForward', [p for p in programs if p['lighting'] != 'mlm_unlit'])]:
        text += [f'bool {name}()', '{', '    return '+ ' || '.join(f'g_SourceCharacterProgram=={p["program"]}u' for p in selected)+';', '}']
    for stage in ['Base', 'Light']:
        text += [f'SOURCE_CHARACTER_NATIVE_OUTPUT EvaluateSourceMovie{stage}(SOURCE_CHARACTER_NATIVE_INPUT input)', '{', '    switch(g_SourceCharacterProgram)', '    {']
        for p in programs:
            text += [f'    case {p["program"]}u: return SourceMovie{stage}{p["program"]}(input);']
        text += ['    }', '    return (SOURCE_CHARACTER_NATIVE_OUTPUT)0;', '}']
    text += ['#endif', '']
    output.mkdir(parents=True, exist_ok=True)
    # The masked stone is consumed by the existing deferred group 1472, not the
    # forward pass. Emit its reviewed functions beside the forward candidate.
    for stage in ('Base', 'Light'):
        path = root/'completion/props'/f'{stage.lower()}1518.hlsli'
        source = world_camera_inputs(path.read_text(encoding='utf8'), 1518, stage)
        (output/path.name).write_text(source, encoding='utf8')
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
