"""Install reviewed Kouku programs in bounded 64-profile native shader groups.

Run build_kouku_gate1_native_shader_programs.py first.
Existing character program groups and project registrations remain untouched.
"""
import argparse
import json
import re
import uuid
import xml.etree.ElementTree as ET
from pathlib import Path
from native_shader_dispatch import (expand_dispatch_includes, insert_grouped_cases,
                                    write_partitioned_dispatch)

ROOT = Path(__file__).resolve().parents[2]


def update(path, transform):
    original = path.read_bytes() if path.is_file() else b""
    newline = "\r\n" if b"\r\n" in original else "\n"
    text = original.decode("utf8").replace("\r\n", "\n")
    result = transform(text).replace("\n", newline).encode("utf8")
    if result != original:
        path.write_bytes(result)


def section(text, name, body, marker):
    begin, end = f"// BEGIN {name}\n", f"// END {name}\n"
    text = re.sub(re.escape(begin) + r".*?" + re.escape(end), "", text, flags=re.S)
    assert text.count(marker) == 1, marker
    return text.replace(marker, begin + body + end + marker, 1)


def conditional_blocks(text):
    """Keep every complete generated carrier guard and its body together."""
    blocks, current, depth = [], [], 0
    for line in text.splitlines(keepends=True):
        stripped = line.strip()
        if not depth:
            if not stripped or stripped.startswith('//'):
                continue
            assert stripped.startswith(('#if ', '#ifdef ', '#ifndef ')), stripped
        current.append(line)
        if stripped.startswith(('#if ', '#ifdef ', '#ifndef ')):
            depth += 1
        elif stripped == '#endif':
            depth -= 1
            assert depth >= 0
            if not depth:
                blocks.append(''.join(current))
                current = []
    assert not depth and not current, 'Unclosed generated carrier guard'
    return blocks


def installed_kouku_programs(shaders):
    paths = sorted(shaders.glob('Shader_EffectKoukuNativeGroup*.hlsli'))
    programs = set()
    for path in paths:
        current = {int(value) for value in re.findall(r'float4 ArtistNative(\d+)\(', path.read_text(encoding='utf8'))}
        assert not programs.intersection(current), ('Duplicate installed Kouku program', path)
        programs.update(current)
    return programs


def installed_kouku_cases(shaders):
    text = expand_dispatch_includes(
        (shaders / 'Shader_EffectArtistNative.hlsli').read_text(encoding='utf8'), shaders)
    match = re.search(r'// BEGIN KOUKU NATIVE CASES\n(.*?)// END KOUKU NATIVE CASES', text, re.S)
    assert match, 'Installed Kouku native dispatch is missing'
    return match[1]


def route_scene_bloom_samples(text):
    """Keep older generated bodies on the shared HDR/bloom snapshot sampler."""
    return re.sub(r'\bg_EffectSceneColorTexture\.Sample(Level|Bias)?\s*\(',
                  lambda match: 'Read_EffectSceneColor' + (match[1] or '') + '(', text)


def install_partitioned_groups(shader_text, case_text):
    """Generate declarations, dispatch, runtime selection and project inputs together."""
    shaders = ROOT / 'Client/Bin/ShaderFiles'
    groups, functions = {}, set()
    for block in conditional_blocks(route_scene_bloom_samples(shader_text)):
        names = re.findall(r'float4 (ArtistNative(\d+)(?:Distortion)?)\(', block)
        assert len(names) == 1, 'A carrier block must contain one native function'
        name, number = names[0]
        number = int(number)
        assert 2304 <= number <= 3711 and name not in functions, name
        assert not re.search(r'\bprojection\[', block) or re.search(r'float4\s+projection\[4\]', block), (
            'Stale generated projection adapter; regenerate this source cohort with '
            'generate_artist_native_runtime_shader.py before installing', name)
        functions.add(name)
        groups.setdefault(number // 64 * 64, []).append(block)
    # Generic Decal/Trail carriers retain the legacy profile-group 2304 macro.
    # Open only their own Kouku blocks across the new buckets; other families
    # and Mesh/Particle wrappers keep their original group filtering.
    def group_condition(group, shared_carriers=()):
        condition = f'!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == {group}'
        return condition + ''.join(f' || defined(EFFECT_NATIVE_{carrier.upper()}_CARRIER)' for carrier in shared_carriers)
    cases, programs, carrier_groups = [], set(), {carrier: set() for carrier in ('Mesh', 'Particle', 'Decal', 'Trail')}
    input_blocks = conditional_blocks(case_text)
    incoming = {int(re.search(r'case (\d+)u:', block)[1]): block for block in input_blocks}
    assert len(incoming) == len(input_blocks), 'Duplicate incoming Kouku native case'
    previous = conditional_blocks(installed_kouku_cases(shaders))
    installed_ids = [int(re.search(r'case (\d+)u:', block)[1]) for block in previous]
    assert set(installed_ids) <= incoming.keys(), 'Supply every installed native case'
    ordered = ''.join(incoming[identifier] for identifier in installed_ids)
    additions = [block for identifier, block in incoming.items() if identifier not in installed_ids]
    ordered = insert_grouped_cases(ordered, additions)
    for block in conditional_blocks(ordered):
        ids = re.findall(r'case (\d+)u:', block)
        assert len(ids) == 1
        number = int(ids[0])
        assert number not in programs and f'ArtistNative{number}' in functions
        programs.add(number)
        group = number // 64 * 64
        shared_carriers = [carrier for carrier in ('Decal', 'Trail')
                           if f'!defined(EFFECT_NATIVE_{carrier.upper()}_CARRIER)' not in block]
        block, count = re.subn(
            r'!defined\(EFFECT_NATIVE_PROFILE_GROUP\) \|\| EFFECT_NATIVE_PROFILE_GROUP == \d+'
            r'(?: \|\| defined\(EFFECT_NATIVE_(?:DECAL|TRAIL)_CARRIER\))*',
            group_condition(group, shared_carriers), block)
        assert count == 1
        cases.append(block)
        for carrier, buckets in carrier_groups.items():
            if f'!defined(EFFECT_NATIVE_{carrier.upper()}_CARRIER)' not in block:
                buckets.add(group)
    assert {int(name.removeprefix('ArtistNative')) for name in functions if not name.endswith('Distortion')} == programs
    assert installed_kouku_programs(shaders) <= programs, 'Supply every installed Kouku native program'
    # The separately restored World markers retain their definitions and dispatch.
    # Their native2351..2359 profiles live in the first 64-profile carrier bucket.
    artist = expand_dispatch_includes(
        (shaders / 'Shader_EffectArtistNative.hlsli').read_text(encoding='utf8'), shaders)
    world = re.search(r'// BEGIN WORLD NATIVE CASES\n(.*?)// END WORLD NATIVE CASES', artist, re.S)
    if world:
        for block in conditional_blocks(world[1]):
            ids = re.findall(r'case (\d+)u:', block)
            assert len(ids) == 1 and int(ids[0]) // 64 * 64 == 2304
            for carrier, buckets in carrier_groups.items():
                if f'!defined(EFFECT_NATIVE_{carrier.upper()}_CARRIER)' not in block:
                    buckets.add(2304)
    for group, blocks in sorted(groups.items()):
        content = f'// Original Kouku material programs {group}..{group + 63}; native IDs and expressions are unchanged.\n'
        content += '\n'.join(blocks)
        update(shaders / f'Shader_EffectKoukuNativeGroup{group}.hlsli', lambda _, content=content: content)
    includes = ''.join(
        '#if !defined(ARTIST_NATIVE_MODEL_ONLY) && '
        f"({group_condition(group, [carrier for carrier in ('Decal', 'Trail') if group in carrier_groups[carrier]])})\n"
        f'#include "Shader_EffectKoukuNativeGroup{group}.hlsli"\n#endif\n'
        for group in sorted(groups))
    updated_artist = section(
        section(artist, 'KOUKU NATIVE GROUP', includes, '#ifndef ARTIST_NATIVE_MODEL_ONLY\nEFFECT_PS_OUT Shade_EffectArtistNative'),
        'KOUKU NATIVE CASES', ''.join(cases), '    default: clip(-1.f); return output;')
    dispatch_files = write_partitioned_dispatch(shaders / 'Shader_EffectArtistNative.hlsli', updated_artist)
    entries = [(carrier, group) for carrier, buckets in carrier_groups.items()
               if carrier in ('Mesh', 'Particle') for group in sorted(buckets)]
    for carrier, group in entries:
        content = f'#define EFFECT_SHADER_FAMILY 7\n#define EFFECT_NATIVE_PROFILE_GROUP {group}\n'
        content += f'#include "Shader_Effect{carrier}FamilyCarrier.hlsli"\n'
        update(shaders / f'Shader_VtxEffect{carrier}Kouku{group}.hlsl', lambda _, content=content: content)
    def table(text):
        text = re.sub(r'^        EFFECT_SHADER_PROGRAM_ROW\((?:MESH|PARTICLE), ARTIST, \d+u, \d+u, "Shader_VtxEffect(?:Mesh|Particle)Kouku\d+\.hlsl"\),\n', '', text, flags=re.M)
        rows = ''.join(f'        EFFECT_SHADER_PROGRAM_ROW({carrier.upper()}, ARTIST, {group}u, {group + 63}u, "Shader_VtxEffect{carrier}Kouku{group}.hlsl"),\n' for carrier, group in entries)
        marker = '    }};\n#undef EFFECT_SHADER_PROGRAM_ROW'
        assert text.count(marker) == 1
        text = text.replace(marker, rows + marker)
        count = len(re.findall(r'^        EFFECT_SHADER_PROGRAM_ROW\(', text, re.M))
        return re.sub(r'std::array<EFFECT_SHADER_PROGRAM_DESC, \d+u>', f'std::array<EFFECT_SHADER_PROGRAM_DESC, {count}u>', text, count=1)
    update(ROOT / 'Client/Public/Effect_ShaderFamily.h', table)
    for suffix in ('', '.filters'):
        def register(text):
            additions = []
            for carrier, group in entries:
                name = f'..\\Bin\\ShaderFiles\\Shader_VtxEffect{carrier}Kouku{group}.hlsl'
                if f'Include="{name}"' not in text:
                    additions.append(f'    <FxCompile Include="{name}"><Filter>97.ShaderFiles</Filter></FxCompile>' if suffix else
                        f'    <FxCompile Include="{name}">\n      <DisableOptimizations Condition="\'$(Platform)\'==\'x64\'">false</DisableOptimizations>\n      <AdditionalOptions Condition="\'$(Platform)\'==\'x64\'">/O1 %(AdditionalOptions)</AdditionalOptions>\n    </FxCompile>')
            for group in sorted(groups):
                name = f'..\\Bin\\ShaderFiles\\Shader_EffectKoukuNativeGroup{group}.hlsli'
                if f'Include="{name}"' not in text:
                    additions.append(f'    <None Include="{name}"><Filter>97.ShaderFiles</Filter></None>' if suffix else f'    <None Include="{name}" />')
            for filename in dispatch_files:
                name = f'..\\Bin\\ShaderFiles\\{filename}'
                if f'Include="{name}"' not in text:
                    additions.append(f'    <None Include="{name}"><Filter>97.ShaderFiles</Filter></None>' if suffix else f'    <None Include="{name}" />')
            if additions:
                prefix, marker, tail = text.rpartition('</Project>')
                assert marker
                text = prefix + '  <ItemGroup>\n' + '\n'.join(additions) + '\n  </ItemGroup>\n' + marker + tail
            xml = ET.fromstring(text)
            ns = {'m': 'http://schemas.microsoft.com/developer/msbuild/2003'}
            for metadata in xml.findall('./m:ItemGroup/m:ProjectReference/m:Project', ns):
                assert len(metadata) == 0
                uuid.UUID(metadata.text.strip())
            compile_items = [item.attrib['Include'] for item in xml.findall('./m:ItemGroup/m:FxCompile', ns)]
            assert len(compile_items) == len(set(compile_items))
            assert all(f'..\\Bin\\ShaderFiles\\Shader_VtxEffect{carrier}Kouku{group}.hlsl' in compile_items for carrier, group in entries)
            return text
        update(ROOT / ('Client/Default/Client.vcxproj' + suffix), register)
    return len(programs), len(groups), len(entries)


def regroup_installed():
    """Migrate reviewed installed bodies without reconstructing or changing a formula."""
    shaders = ROOT / 'Client/Bin/ShaderFiles'
    source = '\n'.join(path.read_text(encoding='utf8') for path in sorted(shaders.glob('Shader_EffectKoukuNativeGroup*.hlsli')))
    return install_partitioned_groups(source, installed_kouku_cases(shaders))


def append_reviewed(source_dir):
    """Add a bounded reviewed cohort while preserving installed shader bodies."""
    contract = json.loads((source_dir / 'native_runtime_contract.json').read_bytes())
    rows = contract['programs']
    assert rows and not contract.get('deferredPrograms')
    owned = {row['program'] for row in rows}
    assert len(owned) == len(rows) and owned <= set(range(2304, 3712))
    assert all(not row.get('distortionPass') for row in rows), 'Use the full pass-aware installer for a new distortion cohort'
    generated = route_scene_bloom_samples(
        (source_dir / 'Shader_EffectArtistNative.hlsli').read_text(encoding='utf8'))
    additions = re.findall(r'(#ifndef ARTIST_NATIVE_MODEL_ONLY\n// [^\n]+\nfloat4 ArtistNative(\d+)\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}\n#endif)', generated, re.S)
    additions = {int(identifier): block for block, identifier in additions if int(identifier) in owned}
    assert set(additions) == owned, 'Reviewed native function closure is incomplete'
    shaders = ROOT / 'Client/Bin/ShaderFiles'
    installed = '\n'.join(path.read_text(encoding='utf8') for path in sorted(shaders.glob('Shader_EffectKoukuNativeGroup*.hlsli')))
    cases = installed_kouku_cases(shaders)
    previous_blocks, case_blocks = [], []
    for block in conditional_blocks(installed):
        identifier = int(re.search(r'float4 ArtistNative(\d+)', block)[1])
        if identifier in owned:
            body = re.search(r'float4 ArtistNative\d+\(.*?\n\}', block, re.S)
            expected = re.search(r'float4 ArtistNative\d+\(.*?\n\}', additions[identifier], re.S)
            assert body and expected and body[0] == expected[0], ('Native ID already owns a different program', identifier)
        else:
            previous_blocks.append(block)
    for block in conditional_blocks(cases):
        if int(re.search(r'case (\d+)u:', block)[1]) not in owned:
            case_blocks.append(block)
    for row in rows:
        carrier = {'mesh': 'MESH', 'decal': 'DECAL', 'ribbon': 'TRAIL', 'beam': 'TRAIL',
                   'animationTrail': 'TRAIL', 'animTrail': 'TRAIL', 'screenPost': 'SCREEN_POST'}.get(row['rendererShape'], 'PARTICLE')
        guard = ' && '.join(f'!defined(EFFECT_NATIVE_{kind}_CARRIER)' for kind in
                           ('MESH', 'PARTICLE', 'DECAL', 'TRAIL', 'SCREEN_POST') if kind != carrier)
        identifier = row['program']
        previous_blocks.append('#if ' + guard + '\n' + additions[identifier] + '\n#endif\n')
        opaque = 'true' if row['nativeBlend'] in ('blend_additive', 'blend_masked', 'blend_opaque') else 'false'
        case_blocks.append(f'#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == {identifier // 64 * 64}) && {guard}\n'
                           f'    case {identifier}u: nativeColor=ArtistNative{identifier}(input); opaqueCoverage={opaque}; break;\n#endif\n')
    # Material descriptors have the same reviewed IDs and preserve peer arrays.
    import install_kouku_gate1_native_materials as materials
    materials.install(source_dir / 'native_runtime_contract.json', source_dir, ROOT / 'Client/Public/Effect_ArtistMaterial.h')
    return install_partitioned_groups('\n'.join(previous_blocks), ''.join(case_blocks))


def install(source_dir, append_source_dir=None):
    def read_contract(directory):
        active = directory / "active_native_runtime_contract.json"
        return json.loads((active if active.is_file() else directory / "native_runtime_contract.json").read_bytes())
    contract = read_contract(source_dir)
    rows = contract["programs"]
    assert not contract["deferredPrograms"], "Every selected source program must be recovered."
    source = (source_dir / "Shader_EffectArtistNative.hlsli").read_text(encoding="utf8")
    append_directories = ([] if append_source_dir is None else
                          [append_source_dir] if isinstance(append_source_dir, Path) else append_source_dir)
    for directory in append_directories:
        additional = read_contract(directory)
        assert not additional["deferredPrograms"], "Additional source programs must be complete."
        rows += additional["programs"]
        source += "\n" + (directory / "Shader_EffectArtistNative.hlsli").read_text(encoding="utf8")
    identifiers = {r["program"] for r in rows}
    assert len(identifiers) == len(rows) and set(range(2304, 2342)) <= identifiers <= set(range(2304, 3712))
    # A newly recovered pass may belong to a byte-identical program reused from
    # an older cohort. Join that pass by its original material/VS/PS identity;
    # the older base-color function and stable program ID remain authoritative.
    pass_sources = {}
    by_id = {row['program']: row for row in rows}
    for directory in [source_dir] + append_directories:
        merged_path = directory / 'merged_native_runtime_contract.json'
        if not merged_path.is_file():
            continue
        merged = json.loads(merged_path.read_bytes())
        assert not merged.get('deferredPrograms'), 'Merged native source closure is incomplete.'
        for recovered in merged['programs']:
            existing = by_id.get(recovered['program'])
            if existing is None and (directory / 'active_native_runtime_contract.json').is_file():
                assert not recovered.get('distortionPass'), ('Inactive reused pass requires an active identity', recovered['program'])
                continue
            assert existing is not None, ('Missing recovered base program', recovered['program'])
            for field in ('sourceMaterial', 'sourceVS', 'sourcePS', 'rendererShape'):
                assert existing[field] == recovered[field], ('Reused native identity changed', recovered['program'], field)
            if recovered.get('distortionPass'):
                existing['distortionPass'] = recovered['distortionPass']
                pass_sources[recovered['program']] = directory / recovered['distortionPass']['hlsli']
    blocks = re.findall(r"(#ifndef ARTIST_NATIVE_MODEL_ONLY\n// [^\n]+\nfloat4 ArtistNative(\d+)\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}\n#endif)", source, re.S)
    blocks = [(block, identifier) for block, identifier in blocks if int(identifier) in identifiers]
    assert len(blocks) == len(rows) and {int(i) for _, i in blocks} == identifiers
    shaders = ROOT / "Client/Bin/ShaderFiles"
    installed = installed_kouku_programs(shaders)
    assert installed <= identifiers, ("Supply all installed Kouku groups; refusing to remove programs", sorted(installed - identifiers))
    def carrier_for(row):
        return {"mesh": "MESH", "decal": "DECAL", "animationTrail": "TRAIL", "animTrail": "TRAIL", "ribbon": "TRAIL", "beam": "TRAIL", "screenPost": "SCREEN_POST"}.get(row["rendererShape"], "PARTICLE")
    def carrier_guard(row):
        return " && ".join(f"!defined(EFFECT_NATIVE_{kind}_CARRIER)" for kind in
            ("MESH", "PARTICLE", "DECAL", "TRAIL", "SCREEN_POST") if kind != carrier_for(row))
    by_program = {row["program"]: row for row in rows}
    if 2360 in by_program:
        def bind_macro_uv(text):
            declaration = "float4 g_ArtistSourceMacroUV;"
            if declaration not in text:
                marker = "float g_ArtistSourceMaterialTime = 0.f;"
                assert text.count(marker) == 1
                text = text.replace(marker, marker + "\n" + declaration, 1)
            return text
        update(shaders / "Shader_EffectArtistNative.hlsli", bind_macro_uv)
    if 2349 in by_program:
        assert by_program[2349].get("enginePrefixAdapter", {}).get("requiresEmitterWorldToLocal")
        def bind_world_to_local(text):
            declaration = "float4 g_ArtistSourceWorldToLocal[3];"
            if declaration not in text:
                marker = "float g_ArtistSourceMaterialTime = 0.f;"
                assert text.count(marker) == 1
                text = text.replace(marker, marker + "\n" + declaration, 1)
            return text
        update(shaders / "Shader_EffectArtistNative.hlsli", bind_world_to_local)
    assert by_program[2310].get("distortionPass"), "Disto05 requires its original accumulation pass."
    extra = ""
    companion_blocks = {}
    for companion in set(pass_sources.values()):
        code = companion.read_text(encoding='utf8')
        found = re.findall(r'(float4 ArtistNative(\d+)Distortion\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\})', code, re.S)
        assert found, ('No original distortion functions', companion)
        for block, identifier in found:
            identifier = int(identifier)
            assert identifier not in companion_blocks or companion_blocks[identifier] == block
            companion_blocks[identifier] = block
    for row in rows:
        if not row.get('distortionPass'):
            continue
        program = row['program']
        if program in pass_sources:
            assert program in companion_blocks, ('Missing original distortion function', program)
            code = companion_blocks[program] + '\n'
        else:
            assert program == 2310, ('Unresolved original distortion companion', program)
            code = (source_dir / 'KoukuDistortionProgram.hlsli').read_text(encoding='utf8')
        extra += '\n#if !defined(ARTIST_NATIVE_MODEL_ONLY) && ' + carrier_guard(row) + '\n' + code + '#endif\n'
    shader_text = "\n\n".join("#if " + carrier_guard(by_program[int(identifier)]) + "\n" + block + "\n#endif"
        for block, identifier in blocks) + extra + "\n"
    cases = ""
    for row in rows:
        cases += f"#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == {row['program'] // 64 * 64}) && " + carrier_guard(row) + "\n"
        opaque = "true" if row["nativeBlend"] in ("blend_additive", "blend_masked", "blend_opaque") else "false"
        if row.get("distortionPass"):
            cases += f'''    case {row["program"]}u:
    {{
        nativeColor=ArtistNative{row["program"]}(input);
        const float4 accumulated=ArtistNative{row["program"]}Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }}
#endif
'''
        else:
            cases += f'    case {row["program"]}u: nativeColor=ArtistNative{row["program"]}(input); opaqueCoverage={opaque}; break;\n#endif\n'
    summary = install_partitioned_groups(shader_text, cases)

    # Descriptor admission alone is insufficient: the vertex and pixel carrier
    # dispatch gates must reach every newly installed program as well.
    for name, expected in (("Shader_EffectMeshFamilyCarrier.hlsli", 2),
                           ("Shader_EffectParticleFamilyCarrier.hlsli", 2),
                           ("Shader_VtxEffectDecal.hlsl", 1),
                           ("Shader_VtxEffectTrail.hlsl", 1)):
        def extend_dispatch(text):
            pattern = r'g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= \d+u'
            assert len(re.findall(pattern, text)) == expected, name
            return re.sub(pattern, 'g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= 3711u', text)
        update(shaders / name, extend_dispatch)

    print(f"Installed {summary[0]} Kouku native programs in {summary[1]} groups and {summary[2]} existing-family shader carriers.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--source-dir", type=Path)
    source.add_argument("--regroup-installed", action="store_true")
    source.add_argument('--append-reviewed-dir', type=Path)
    parser.add_argument("--append-source-dir", type=Path, action="append", default=[])
    args = parser.parse_args()
    if args.append_reviewed_dir:
        assert not args.append_source_dir
        print('Appended reviewed programs/groups/carriers:', append_reviewed(args.append_reviewed_dir.resolve()))
    elif args.regroup_installed:
        assert not args.append_source_dir, "Regroup consumes installed shader bodies only"
        print("Regrouped programs/groups/carriers:", regroup_installed())
    else:
        install(args.source_dir.resolve(), [directory.resolve() for directory in args.append_source_dir])
