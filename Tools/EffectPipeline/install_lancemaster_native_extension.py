"""Install newly recovered native programs while preserving reviewed V programs."""
from pathlib import Path
import argparse, collections, json, re
from native_shader_dispatch import (append_dispatch_cases, expand_dispatch_includes,
                                    write_if_changed, write_partitioned_dispatch)
from native_material_tables import read_material_source, write_material_source

ROOT = Path(__file__).resolve().parents[2]


def install(evidence):
    contract = json.loads((evidence / 'native_runtime_contract.json').read_bytes())
    programs = [r for r in contract['programs'] if 1200 <= r['program'] < 1600]
    generated = (evidence / 'Shader_EffectLanceMasterVANative.hlsli').read_text(encoding='utf8')
    groups = collections.defaultdict(list)
    for row in programs:
        index = row['program']
        match = re.search(r'^float4 LanceVANative' + str(index) + r'\(.*?^\}', generated, re.M | re.S)
        if not match:
            raise ValueError(('Missing recovered native function', index))
        groups[index // 64 * 64].append(match.group())
    shader_dir = ROOT / 'Client/Bin/ShaderFiles'
    for group, bodies in groups.items():
        name = f'Shader_EffectLanceMasterVANativeGroup{group}.hlsli'
        write_if_changed(shader_dir / name,
            f'// Recovered LanceMaster source programs {group}..{group + 63}.\n'
            '#ifndef LANCE_VA_NATIVE_MODEL_ONLY\n' + '\n\n'.join(bodies) + '\n#endif\n')
    path = shader_dir / 'Shader_EffectLanceMasterVANative.hlsli'
    source = expand_dispatch_includes(path.read_text(encoding='utf8'), shader_dir)
    marker = '\n#ifndef LANCE_VA_NATIVE_MODEL_ONLY\nEFFECT_PS_OUT Shade_EffectLanceMasterVANative'
    begin = source.index(marker)
    prefix, suffix = source[:begin], source[begin:]
    for group in sorted(groups):
        include = f'#include "Shader_EffectLanceMasterVANativeGroup{group}.hlsli"'
        if include not in prefix:
            prefix += f'\n#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == {group}\n{include}\n#endif\n'
    cases = []
    for row in programs:
        index = row['program']
        if f'    case {index}u:' in suffix:
            continue
        other = {'mesh': ['PARTICLE', 'SCREEN_POST'], 'sprite': ['MESH', 'SCREEN_POST'],
                 'screenPost': ['MESH', 'PARTICLE']}[row['rendererShape']]
        cases.append('\n'.join(['#if ' + ' && '.join(f'!defined(EFFECT_NATIVE_{x}_CARRIER)' for x in other),
                      f'#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == {index // 64 * 64}',
                      f'    case {index}u: nativeColor=LanceVANative{index}(input); additive=' +
                      str(row['nativeBlend'] == 'blend_additive').lower() + '; break;', '#endif', '#endif']) + '\n')
    write_partitioned_dispatch(path, prefix + append_dispatch_cases(suffix, cases))

    header_path = ROOT / 'Client/Public/Effect_LanceMasterVAMaterial.h'
    header = read_material_source(header_path)
    generated_header = (evidence / 'Effect_LanceMasterVAMaterial.generated.h').read_text(encoding='utf8')
    table = re.search(r'inline constexpr std::array<LANCEMASTER_VA_PROGRAM_DESC,(\d+)> LANCEMASTER_VA_PROGRAMS = \{\{\n(.*?)\n\}\};', header, re.S)
    generated_table = re.search(r'LANCEMASTER_VA_PROGRAMS = \{\{\n(.*?)\n\}\};', generated_header, re.S)
    existing_ids = {int(n) for n in re.findall(r'^    \{(\d+)u,', table.group(2), re.M)}
    new_ids = {row['program'] for row in programs} - existing_ids
    constants = []
    for index in sorted(new_ids):
        match = re.search(r'^inline constexpr std::array<std::string_view,\d+> LANCEMASTER_VA_TEXTURES_' +
                          str(index) + r'\b.*?(?=^inline constexpr std::array<std::string_view,|^inline constexpr std::array<LANCEMASTER_VA_PROGRAM_DESC,)',
                          generated_header, re.M | re.S)
        if not match:
            raise ValueError(('Missing recovered parameter constants', index))
        constants.append(match.group())
    added = [line for line in generated_table.group(1).splitlines()
             if int(re.match(r'    \{(\d+)u,', line).group(1)) in new_ids]
    replacement = ''.join(constants) + f'inline constexpr std::array<LANCEMASTER_VA_PROGRAM_DESC,{len(existing_ids) + len(new_ids)}> LANCEMASTER_VA_PROGRAMS = {{{{\n' + table.group(2) + '\n' + '\n'.join(added) + '\n}};'
    write_material_source(header_path, header[:table.start()] + replacement + header[table.end():],
                          expected_source=header)
    (evidence / 'installed_native_extension.json').write_text(json.dumps(
        dict(programs=sorted(row['program'] for row in programs), groups=sorted(groups),
             preservedExistingPrograms=sorted(existing_ids)), indent=2), encoding='utf8')
    print('Installed', len(programs), 'native programs in groups', sorted(groups))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--evidence', type=Path, default=ROOT / 'out/LanceMasterAllRestore20260910')
    install(parser.parse_args().evidence)
