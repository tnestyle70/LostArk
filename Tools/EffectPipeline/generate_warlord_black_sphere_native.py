"""Restore one source-qualified Warlord background material in the existing family.

The shared offline translator owns DXBC instruction translation and scene-depth
adapters. This tool supplies the exact recovered LocalVF contract and installs
only native673, preserving all other material tables and shader functions.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess
import sys

from native_material_tables import read_material_source, write_material_source
from native_shader_dispatch import write_if_changed


ROOT = Path(__file__).resolve().parents[2]
PROGRAM = 673
MATERIAL = 'fx_m_mi_o_00.fx_mi.fx_o_me_superactionspace_01_02_tr'
PS = '591444cc4fff1f449dd154b804f25bb8'
VS = '70ef36e78c7b254f96b485b73b366538'
MARKER = 'WARLORD BLACK SPHERE 673'


def read(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))


def marked(body):
    return f'// BEGIN {MARKER}\n{body.rstrip()}\n// END {MARKER}\n'


def replace_marked(source, block, insertion=None):
    pattern = re.compile(r'// BEGIN ' + MARKER + r'\n.*?// END ' + MARKER + r'\n', re.S)
    matches = list(pattern.finditer(source))
    if len(matches) > 1:
        raise ValueError('Repeated native673 ownership block')
    if matches:
        return pattern.sub(lambda _: block, source, count=1)
    if 'WarlordNative673(' in source or 'WARLORD_NATIVE_TEXTURES_673' in source:
        raise ValueError('Native673 already belongs to an unmarked implementation')
    insertion = len(source) if insertion is None else insertion
    return source[:insertion] + block + source[insertion:]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-dir', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    source_dir, output = args.source_dir.resolve(), args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    selections = read(source_dir / 'selected_runtime_material_programs.json')['programs']
    assert len(selections) == 1
    selection = selections[0]
    assert (selection['program'], selection['resolvedMaterial'], selection['sourcePS'],
            selection['sourceVS'], selection['sourceVF'], selection['rendererShape']) == (
                PROGRAM, MATERIAL, PS, VS, 'flocalvertexfactory', 'mesh')
    native = read(source_dir / 'native_material_inputs.json')['materials']
    assert len(native) == 1 and native[0]['sourceMaterial'] == MATERIAL
    assert native[0]['parentProperties']['blendmode']['value'] == 'blend_translucent'
    ps = read(source_dir / 'full_programs' / (PS + '.json'))
    assert ps['bindings']['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0, 1]
    last_rt0 = max(i for i, line in enumerate(ps['disassembly']['instructions']) if re.search(r'\bo0\.', line))
    rt0 = '\n'.join(ps['disassembly']['instructions'][:last_rt0 + 1])
    for signature in ps['inputSignature']:
        if signature['semanticName'].lower() == 'color':
            assert not re.search(r'\bv' + str(signature['register']) + r'\.', rt0)
    assert not any(s['semanticName'].lower() == 'texcoord' and s['semanticIndex'] == 0
                   for s in ps['inputSignature'])
    subprocess.run([sys.executable, '-X', 'utf8',
                    str(ROOT / 'Tools/EffectPipeline/generate_artist_native_runtime_shader.py'),
                    '--source-dir', str(source_dir), '--output-dir', str(output / 'translated')], check=True)
    contract = read(output / 'translated/native_runtime_contract.json')
    assert not contract['deferredPrograms'] and len(contract['programs']) == 1
    row = contract['programs'][0]
    assert row['program'] == PROGRAM and row['requiresDepthSample']
    assert not row['requiresSourceUV1'] and not row['requiresSceneColor'] and not row['requiresTangentView']
    assert not row['nativeTwoSided']
    row['runtimeShaderProfileId'] = f'effect.ue3.warlord-{PROGRAM}-native.v1'
    parent = row['parentMaterial']
    slug = re.sub('[^a-z0-9]+', '.', parent.lower()).strip('.')
    row['profileId'] = 'ue3.material.' + slug[:72] + '.' + hashlib.sha256(parent.encode()).hexdigest()[:12]
    row['renderProfile'] = 'ALPHA_ONE_SIDED_DEPTH_READ'
    translated = (output / 'translated/Shader_EffectArtistNative.hlsli').read_text(encoding='utf-8')
    function = re.search(r'// [^\n]+\nfloat4 ArtistNative673\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}', translated, re.S)[0]
    function = function.replace('ArtistNative', 'WarlordNative').replace('ARTIST_NATIVE_INPUT', 'WARLORD_NATIVE_INPUT')
    function = function.replace('g_ArtistSource', 'g_WarlordSource')
    guard = '#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)\n'
    function = marked(guard + function + '\n#endif\n')
    dispatch = marked(guard + '    case 673u: nativeColor=WarlordNative673(input); additive=false; break;\n#endif\n')
    q = lambda value: json.dumps(value, ensure_ascii=False)
    arrays = f'inline constexpr std::array<std::string_view,{len(row["textures"])}> WARLORD_NATIVE_TEXTURES_673 = {{{{' + ','.join(q(t['name']) for t in row['textures']) + '}};\n'
    arrays += f'inline constexpr std::array<WARLORD_NATIVE_PARAMETER_DESC,{len(row["parameters"])}> WARLORD_NATIVE_PARAMETERS_673 = {{{{\n'
    arrays += ''.join('    {' + q(p['name']) + f', {p["row"]}u, {p["lane"] or 0}u, ' + str(p['kind'] == 'vector').lower() + '},\n' for p in row['parameters']) + '}};\n'
    arrays += f'inline constexpr std::array<WARLORD_NATIVE_SWITCH_DESC,{len(row["staticSwitches"])}> WARLORD_NATIVE_SWITCHES_673 = {{{{\n'
    arrays += ''.join('    {' + q(s['parameterName']) + ', ' + str(s['value']).lower() + '},\n' for s in row['staticSwitches']) + '}};\n'
    arrays = marked(arrays)
    entry = '    {673u,' + ','.join(q(row[k]) for k in ['runtimeShaderProfileId', 'sourceMaterial', 'parentMaterial', 'profileId'])
    entry += ',true,"mesh",false,true,false,false,EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,WARLORD_NATIVE_TEXTURES_673,WARLORD_NATIVE_PARAMETERS_673,WARLORD_NATIVE_SWITCHES_673},\n'
    source_profile = dict(enabled=True, profileId=row['profileId'], runtimeShaderProfileId=row['runtimeShaderProfileId'],
        parentMaterialPath=parent, semanticStatus='reconstructed_profile',
        textures=[{k: t[k] for k in ['name', 'sourceObjectPath', 'assetId', 'addressU', 'addressV', 'colorSpace', 'samplingEvidence']} for t in row['textures']],
        scalars=[dict(name=p['name'], group='None', value=p['effective']) for p in row['parameters'] if p['kind'] == 'scalar'],
        vectors=[dict(name=p['name'], group='None', value=p['effective']) for p in row['parameters'] if p['kind'] == 'vector'],
        staticSwitches=[dict(name=s['parameterName'], group='None', value=s['value']) for s in row['staticSwitches']],
        dynamicParameterSemantics=['unbound'] * 4, subUVMode='none')
    material = dict(templateId='effect.source_material', sourceMaterialPath=MATERIAL,
                    renderProfile='alpha_one_sided_depth_read', sourceProfile=source_profile)
    for name, data in [('native_runtime_contract.json', dict(programs=[row], deferredPrograms=[])),
                       ('material.candidate.json', material)]:
        (output / name).write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    for name, text in [('native673.hlsli', function), ('dispatch673.hlsli', dispatch), ('tables673.inl', arrays + entry)]:
        (output / name).write_text(text, encoding='utf-8')
    if args.install:
        shader_dir = ROOT / 'Client/Bin/ShaderFiles'
        function_path = shader_dir / 'Shader_EffectWarlordNativeGroup000.hlsli'
        dispatch_path = shader_dir / 'Shader_EffectWarlordNativeDispatchBase0.hlsli'
        header_path = ROOT / 'Client/Public/Effect_WarlordNativeMaterial.h'
        old_function, old_dispatch = function_path.read_text(encoding='utf-8'), dispatch_path.read_text(encoding='utf-8')
        old_header = read_material_source(header_path)
        new_function = replace_marked(old_function, function)
        new_dispatch = replace_marked(old_dispatch, dispatch, old_dispatch.rfind('#endif'))
        marker = 'inline constexpr std::array<WARLORD_NATIVE_PROGRAM_DESC,'
        new_header = replace_marked(old_header, arrays, old_header.index(marker))
        entries = re.compile(r'^    \{673u,.*\n', re.M)
        found = entries.findall(new_header)
        if found and (len(found) != 1 or MATERIAL not in found[0]):
            raise ValueError('Native673 material identity collision')
        new_header = entries.sub('', new_header)
        start = new_header.index(marker)
        end = new_header.index('}};', start)
        body = new_header[new_header.index('\n', start) + 1:end] + entry
        count = sum(line.startswith('    {') for line in body.splitlines())
        new_header = new_header[:start] + f'{marker}{count}> WARLORD_NATIVE_PROGRAMS = {{{{\n' + body + new_header[end:]
        if function_path.read_text(encoding='utf-8') != old_function or dispatch_path.read_text(encoding='utf-8') != old_dispatch:
            raise RuntimeError('Shader changed during native673 staging')
        write_material_source(header_path, new_header, expected_source=old_header)
        write_if_changed(function_path, new_function)
        write_if_changed(dispatch_path, new_dispatch)
    print('Generated Warlord673 source contract; installed=' + str(args.install))


if __name__ == '__main__':
    main()
