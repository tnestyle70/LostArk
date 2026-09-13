"""Source-exact native material programs and catalog rows for vehicle bodies.

extract    reads each MIC's static shader map from the retail RefShaderCache and
           writes its GPU-skin BasePass/directional-light pixel programs, uniform
           expressions, effective parameters and referenced textures.
verify     regenerates an installed program from its original MIC and requires
           the installed Base/Light functions to match byte for byte.
generate   writes one new Base/Light function pair and its CPU packing block.
install    inserts a generated program into both SourceCharacter program files
           and SourceCharacterMaterialParameters.h without renumbering others.
rows       emits the catalog modelMaterialOverrides rows for extracted MICs.
textures   copies the exact source textures the rows reference into Resources.
"""
from __future__ import annotations

import argparse
import functools
import hashlib
import json
import pathlib
import re
import shutil
import struct
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'Tools/LevelPlacementExtractor'), str(ROOT / 'Tools/EffectPipeline')]

BASE_TYPE = 'tbasepasspixelshaderfnolightmappolicyskylight'
LIGHT_TYPE = 'tlightpixelshaderfdirectionallightpolicyfnostaticshadowingpolicy'
GPU_SKIN_VF = 'fgpuskinvertexfactory'
LOOKUP_TEXTURES = {'texture_ibl', 'texture_brdf'}
TIME_PARAMETER_ROW = 63
RELEASE = pathlib.Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC')
BASE_PROGRAMS = ROOT / 'Engine/Bin/ShaderFiles/Shader_SourceCharacterBasePrograms.hlsli'
LIGHT_PROGRAMS = ROOT / 'Engine/Bin/ShaderFiles/Shader_SourceCharacterLightPrograms.hlsli'
CLIENT_SHADER_DIR = ROOT / 'Client/Bin/ShaderFiles'
PARAMETER_HEADER = ROOT / 'Client/Public/SourceCharacterMaterialParameters.h'
RESOURCES = ROOT / 'Client/Bin/Resources'


def fail(message):
    raise SystemExit('build_vehicle_source_material: ' + message)


def open_cache(d3dcompiler: pathlib.Path):
    import extract_artist_31470_shader_cache_oracle as oracle
    import extract_artist_31470_main_ref_shader_cache as ref_cache
    import extract_ue3_material_shader_maps as sm
    read_fname = oracle.read_fname

    def numbered_read_fname(data, offset, names):
        name, number, rest = read_fname(data, offset, names)
        return (f'{number}.{name}' if number else name), 0, rest
    oracle.read_fname = numbered_read_fname
    ref_cache.EXPECTED_D3DCOMPILER['byteSize'] = d3dcompiler.stat().st_size
    ref_cache.EXPECTED_D3DCOMPILER['sha256'] = hashlib.sha256(d3dcompiler.read_bytes()).hexdigest()
    cache = sm.package_tables(RELEASE / 'EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk')
    return sm, cache, sm.parse_shader_code_layout(cache), sm.D3DDisassembler(d3dcompiler)


def command_extract(arguments):
    from extract_ue3_effect_material_closure import (
        LOSTARK_KR_AES_KEY, decode_material_instance, find_export, load_package,
        package_ref_name, package_ref_path, parse_tagged_properties, tagged_value)
    from extract_ue3_placements import resolve_physical_package
    sm, cache, layout, disassembler = open_cache(arguments.d3dcompiler)

    @functools.lru_cache(None)
    def package(name):
        return load_package(resolve_physical_package(arguments.umodel, RELEASE, name, 'kr'), LOSTARK_KR_AES_KEY)

    def full_reference(name, loaded, reference):
        path = package_ref_path(reference, loaded.imports, loaded.exports)
        return name + '.' + path if reference > 0 else path

    @functools.lru_cache(None)
    def export(path):
        name, relative = path.split('.', 1)
        loaded = package(name)
        entry = find_export(loaded, relative)
        serial = loaded.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        properties, end = parse_tagged_properties(serial, loaded.names, loaded.summary.version)
        return dict(package=name, className=package_ref_name(entry.class_index, loaded.imports, loaded.exports),
                    properties=properties, tail=serial[end:])

    @functools.lru_cache(None)
    def material(path):
        current = export(path)
        loaded = package(current['package'])
        if current['className'] == 'materialinstanceconstant':
            parent = material(full_reference(current['package'], loaded, tagged_value(current['properties'], 'parent')))
            static = sm.decode_static_set_from_tail(current['tail'], bytes.fromhex(parent['baseId']), loaded.names,
                                                    sm.POLICY_BLOCK_ABSENT)
            if static.get('status') == sm.STATUS_BLOCKED:
                static = parent['static']
            original = decode_material_instance(loaded, path.split('.', 1)[1])
            scalars, vectors, textures = dict(parent['scalars']), dict(parent['vectors']), dict(parent['textures'])
            for row in original['scalarParameters']:
                scalars[row['name']] = row['value']
            for row in original['vectorParameters']:
                vectors[row['name']] = row['value']
            for row in original['textureParameters']:
                textures[row['name']] = full_reference(current['package'], loaded, row['packageIndex']) if row['packageIndex'] else None
            return dict(parentMaterial=parent['parentMaterial'], baseId=parent['baseId'], static=static,
                        scalars=scalars, vectors=vectors, textures=textures)
        base_id = current['tail'][16:32].hex()
        parsed = sm.parse_static_parameter_set(bytes.fromhex(base_id) + struct.pack('<IIII', 0, 0, 0, 0), 0, loaded.names)
        static = {'staticParameterSet': sm.public_static_set(parsed),
                  'engineEqualityStaticParameterSetSha256': sm.canonical_json_sha256(sm.engine_equivalent_static_parameter_set(parsed))}
        return dict(parentMaterial=path, baseId=base_id, static=static, scalars={}, vectors={}, textures={})

    rows = {target: material(target) for target in arguments.materials}
    scans = sm.scan_base_material_contexts(cache, layout, sorted({row['baseId'] for row in rows.values()}))
    arguments.out.mkdir(parents=True, exist_ok=True)
    for target, row in rows.items():
        equality = row['static']['engineEqualityStaticParameterSetSha256']
        material_map = sm.parse_material_map(cache, layout, sm.select_unique_map_context(scans[row['baseId']], equality), equality)
        factories = [v for v in material_map['vertexFactories'] if v['vertexFactoryType'] == GPU_SKIN_VF]
        if len(factories) != 1:
            fail(f'{target} has no unique GPU-skin vertex factory')
        references = [s for s in factories[0]['shaderReferences'] if s['shaderType'] in (BASE_TYPE, LIGHT_TYPE)]
        if sorted(s['shaderType'] for s in references) != sorted((BASE_TYPE, LIGHT_TYPE)):
            fail(f'{target} lacks its BasePass or directional light pixel shader')
        bytecodes = sm.extract_selected_packed_dxbc(cache, layout, references)
        objects = sm.extract_selected_shader_objects(cache, layout, references, allow_mixed_code_preambles=True)
        programs = {}
        for reference in references:
            bytecode = bytecodes[reference['shaderIdHex']]['_bytecode']
            disassembly = disassembler.disassemble(bytecode)
            closure = sm.parse_dxbc_declaration_closure(disassembly, allow_textureless=True)
            raw = objects['byShaderId'][reference['shaderIdHex']]
            bindings = sm.select_unique_native_binding_arrays(raw['_bytes'], raw['logicalOffset'],
                                                              material_map['uniformExpressionCounts'], closure)
            programs[reference['shaderType']] = dict(shaderId=reference['shaderIdHex'],
                                                     disassembly=dict(declarations=disassembly['declarations'],
                                                                      instructions=disassembly['instructions']),
                                                     bindings=bindings)
        source = export(target)
        loaded = package(source['package'])
        count = struct.unpack_from('<I', source['tail'], 36)[0]
        texture_expressions = material_map['uniformExpressionSet']['pixelTexture2DExpressions']
        if not (len(texture_expressions) <= count <= 64 and len(source['tail']) >= 40 + 4 * count):
            fail(f'{target} referenced texture table is not at the static-resource offset')
        referenced = struct.unpack_from('<' + 'i' * count, source['tail'], 40)
        textures = []
        for index, expression in enumerate(texture_expressions):
            name = expression.get('parameterName')
            path = row['textures'].get(name) if name else None
            if not path:
                path = full_reference(source['package'], loaded, referenced[expression['referencedTextureIndex']])
            srgb = tagged_value(export(path)['properties'], 'srgb')
            textures.append(dict(expressionIndex=index, parameterName=name, sourceObject=path,
                                 srgb=True if srgb is None else bool(srgb)))
        document = dict(sourceMaterial=target, parentMaterial=row['parentMaterial'], mapKey=equality,
                        scalars=row['scalars'], vectors=row['vectors'], textures=textures,
                        uniformExpressionSet={k: material_map['uniformExpressionSet'][k] for k in
                                              ('pixelVectorExpressions', 'pixelScalarExpressions', 'pixelTexture2DExpressions')},
                        programs=programs)
        path = arguments.out / (target + '.json')
        path.write_text(json.dumps(document, indent=1, default=str), encoding='utf8')
        print('extracted', target, programs[BASE_TYPE]['shaderId'], programs[LIGHT_TYPE]['shaderId'])


def split_arguments(text):
    return [x.strip() for x in re.split(r',\s*(?![^()]*\))', text)]


def immediate(text, integer=False):
    numbers = [x.strip() for x in text[2:-1].split(',')]
    numbers += numbers[-1:] * (4 - len(numbers))
    if integer:
        return 'uint4(' + ','.join(((hex(int(n, 16) & 4294967295) if n.lower().startswith('0x') else str(int(n, 10) & 4294967295)) + 'u' for n in numbers)) + ')'

    def lane(n):
        if n.lower().startswith('0x'):
            return 'asfloat(' + n + 'u)'
        return n if '.' in n or 'e' in n.lower() else 'asfloat(' + str(int(n, 10) & 4294967295) + 'u)'
    return 'float4(' + ','.join(lane(n) for n in numbers) + ')'


def operand(text, destination=False):
    text = text.strip()
    if text == 'null':
        return 'unused'
    negative = text.startswith('-')
    text = text[1:] if negative else text
    absolute = text.startswith('|') and text.endswith('|')
    text = text[1:-1] if absolute else text
    if text.startswith('l('):
        text = immediate(text)
    else:
        text = re.sub(r'\bcb0\[', 'source[', text)
        text = re.sub(r'\bcb1\[', 'projection[', text)
        text = re.sub(r'\bcb2\[', 'passValues[', text)
        text = re.sub(r'\bo(\d)\b', r'output.targets[\1]', text)
        text = text.replace('vCoverage.x', '1.0')
        if not destination:
            text = re.sub(r'\.([xyzw])$', lambda m: '.' + m[1] * 4, text)
    if absolute:
        text = 'abs(' + text + ')'
    return '-(' + text + ')' if negative else text


def uint_operand(text):
    return immediate(text.strip(), True) if text.strip().startswith('l(') else 'asuint(' + operand(text) + ')'


def result_mask(destination, value):
    match = re.search(r'\.([xyzw]+)$', destination)
    return operand(destination, True) + ' = (' + value + ').' + (match.group(1) if match else 'xyzw') + ';'


def translate(instruction, texture_map, lookup, stage):
    op, _, tail = instruction.partition(' ')
    a = split_arguments(tail)
    saturate = op.endswith('_sat')
    op = op.removesuffix('_sat')
    if op in ('if_nz', 'if_z'):
        return 'if ((' + uint_operand(a[0]) + ').x' + (' != 0u)' if op == 'if_nz' else ' == 0u)') + ' {'
    if op == 'else':
        return '} else {'
    if op == 'endif':
        return '}'
    if op == 'ret':
        return 'return output;'
    if op == 'discard_nz':
        return 'if ((' + uint_operand(a[0]) + ').x != 0u) { output.discarded = true; return output; }'
    if a and a[0].startswith('oMask'):
        return '// Coverage is owned by the product rasterizer.'
    if op.startswith('sample'):
        destination, uv, texture = a[:3]
        match = re.match(r't(\d+)\.([xyzw]+)', texture)
        register, swizzle = int(match[1]), match[2]
        if register in texture_map:
            index = texture_map[register]
            sampler = 'SourceCharacterLookupSampler' if index in lookup else 'SourceCharacterSampler'
            sample = f'g_SourceCharacterTexture{index}'
            if 'sample_l' in op:
                sample += f'.SampleLevel({sampler}, ({operand(uv)}).xy, ({operand(a[4])}).x)'
            elif 'sample_b' in op:
                sample += f'.SampleBias({sampler}, ({operand(uv)}).xy, ({operand(a[4])}).x)'
            else:
                sample += f'.Sample({sampler}, ({operand(uv)}).xy)'
        elif 'texturecube' in op:
            sample = 'float4(0.0,0.0,0.0,0.0)'
        else:
            sample = 'float4(sqrt(saturate(input.shadow)).xxx,1.0)' if stage == 'light' else 'float4(0.0,0.0,0.0,0.0)'
        return result_mask(destination, '(' + sample + ').' + swizzle)
    source = [operand(x) for x in a[1:]]
    if op == 'mov':
        value = source[0]
    elif op in ('add', 'mul', 'div'):
        value = '(' + source[0] + ')' + {'add': '+', 'mul': '*', 'div': '/'}[op] + '(' + source[1] + ')'
    elif op == 'mad':
        value = '(' + source[0] + ')*(' + source[1] + ')+(' + source[2] + ')'
    elif op in ('dp2', 'dp3', 'dp4'):
        mask = {'dp2': 'xy', 'dp3': 'xyz', 'dp4': 'xyzw'}[op]
        value = 'dot((' + source[0] + ').' + mask + ',(' + source[1] + ').' + mask + ').xxxx'
    elif op in ('min', 'max'):
        value = op + '(' + ','.join(source) + ')'
    elif op == 'rcp':
        value = '1.0/(' + source[0] + ')'
    elif op in ('rsq', 'sqrt', 'log', 'exp', 'frc', 'round_ni', 'round_pi'):
        value = {'rsq': 'rsqrt', 'sqrt': 'sqrt', 'log': 'log2', 'exp': 'exp2', 'frc': 'frac', 'round_ni': 'floor', 'round_pi': 'ceil'}[op] + '(' + source[0] + ')'
    elif op in ('lt', 'ge', 'ne', 'eq'):
        value = 'asfloat((uint4)((' + source[0] + ')' + {'lt': '<', 'ge': '>=', 'ne': '!=', 'eq': '=='}[op] + '(' + source[1] + ')) * 0xffffffffu)'
    elif op == 'movc':
        value = '(' + uint_operand(a[1]) + ' != 0u) ? (' + source[1] + ') : (' + source[2] + ')'
    elif op in ('and', 'or', 'xor'):
        value = 'asfloat(' + uint_operand(a[1]) + {'and': ' & ', 'or': ' | ', 'xor': ' ^ '}[op] + uint_operand(a[2]) + ')'
    elif op == 'not':
        value = 'asfloat(~' + uint_operand(a[1]) + ')'
    elif op == 'ftou':
        value = 'asfloat((uint4)(' + source[0] + '))'
    elif op == 'utof':
        value = '(float4)(' + uint_operand(a[1]) + ')'
    elif op == 'itof':
        value = '(float4)(asint(' + source[0] + '))'
    elif op == 'iadd':
        value = 'asfloat(' + uint_operand(a[1]) + ' + ' + uint_operand(a[2]) + ')'
    elif op == 'bfi':
        value = 'SourceCharacterBitInsert(' + ','.join(uint_operand(x) for x in a[1:]) + ')'
    elif op in ('ishl', 'ushr'):
        value = 'asfloat(' + uint_operand(a[1]) + (' << ' if op == 'ishl' else ' >> ') + '(' + uint_operand(a[2]) + ' & 31u))'
    elif op in ('deriv_rtx_coarse', 'deriv_rty_coarse'):
        value = ('ddx_coarse' if 'rtx' in op else 'ddy_coarse') + '(' + source[0] + ')'
    elif op == 'sincos':
        parts = []
        if a[0] != 'null':
            parts.append(result_mask(a[0], 'sin(' + operand(a[2]) + ')'))
        if a[1] != 'null':
            parts.append(result_mask(a[1], 'cos(' + operand(a[2]) + ')'))
        return ' '.join(parts)
    else:
        fail(f'unsupported DXBC instruction: {instruction}')
    if saturate:
        value = 'saturate(' + value + ')'
    return result_mask(a[0], value)


def g9(value):
    return format(float(value), '.9g')


def uses_time(node):
    if not isinstance(node, dict):
        return False
    if node['typeName'] == 'fmaterialuniformexpressiontime':
        return True
    return any(uses_time(v) for v in node.values() if isinstance(v, dict))


def parameter_names(node, found):
    if isinstance(node, dict):
        if node['typeName'] in ('fmaterialuniformexpressionscalarparameter', 'fmaterialuniformexpressionvectorparameter'):
            found.add(node['parameterName'])
        for value in node.values():
            parameter_names(value, found)
    return found


FOLD = {0: ('+', 'add'), 1: ('-', 'subtract'), 2: ('*', 'multiply')}


def hlsl(node, time_parameter):
    kind = node['typeName']
    if kind == 'fmaterialuniformexpressiontime':
        return 'g_SourceCharacterTime.xxxx'
    if kind == 'fmaterialuniformexpressionconstant':
        return 'float4(' + ','.join(g9(v) for v in node['value']) + ')'
    if kind in ('fmaterialuniformexpressionscalarparameter', 'fmaterialuniformexpressionvectorparameter'):
        if node['parameterName'] != time_parameter:
            fail(f'shader-side parameter {node["parameterName"]} has no reserved row')
        return f'source[{TIME_PARAMETER_ROW}].xxxx'
    if kind == 'fmaterialuniformexpressionfoldedmath':
        return '(' + hlsl(node['a'], time_parameter) + FOLD[node['operationOrdinal']][0] + hlsl(node['b'], time_parameter) + ')'
    if kind == 'fmaterialuniformexpressionsine':
        return ('cos(' if node['isCosine'] else 'sin(') + hlsl(node['input'], time_parameter) + ')'
    if kind == 'fmaterialuniformexpressionappendvector':
        return 'SourceCharacterAppend(' + hlsl(node['a'], time_parameter) + ',' + hlsl(node['b'], time_parameter) + ',' + str(node['componentsFromA']) + 'u)'
    fail(f'unsupported uniform expression {kind}')


def cpp_float(value):
    text = g9(value)
    return (text if '.' in text or 'e' in text else text + '.') + 'f'


def cpp(node):
    kind = node['typeName']
    if kind == 'fmaterialuniformexpressiontime':
        return 'Value{}'
    if kind == 'fmaterialuniformexpressionconstant':
        return 'Value{' + ','.join(cpp_float(v) for v in node['value']) + '}'
    if kind in ('fmaterialuniformexpressionscalarparameter', 'fmaterialuniformexpressionvectorparameter'):
        return 'parameter("' + node['parameterName'] + '")'
    if kind == 'fmaterialuniformexpressionfoldedmath':
        return FOLD[node['operationOrdinal']][1] + '(' + cpp(node['a']) + ',' + cpp(node['b']) + ')'
    if kind == 'fmaterialuniformexpressionsine':
        return 'wave(' + cpp(node['input']) + ',' + ('true' if node['isCosine'] else 'false') + ')'
    if kind == 'fmaterialuniformexpressionappendvector':
        return 'append(' + cpp(node['a']) + ',' + cpp(node['b']) + ',' + str(node['componentsFromA']) + 'u)'
    fail(f'unsupported uniform expression {kind}')


def stage_rows(document, program):
    expressions = document['uniformExpressionSet']
    rows = [(b['baseIndex'] // 16, None, expressions['pixelVectorExpressions'][b['expressionIndexOrGroup']])
            for b in program['bindings']['vectors']]
    scalars = expressions['pixelScalarExpressions']
    for binding in program['bindings']['scalarGroups']:
        for lane in range(4):
            index = binding['expressionIndexOrGroup'] * 4 + lane
            rows.append((binding['baseIndex'] // 16, lane, scalars[index] if index < len(scalars) else None))
    return sorted(rows, key=lambda r: (r[0], -1 if r[1] is None else r[1]))


def time_parameter_of(document):
    names = set()
    for stage in (BASE_TYPE, LIGHT_TYPE):
        for _row, _lane, node in stage_rows(document, document['programs'][stage]):
            if node is not None and uses_time(node):
                parameter_names(node, names)
    if len(names) > 1:
        fail(f'time expressions need more than one reserved row: {sorted(names)}')
    return next(iter(names), None)


def texture_map_of(document, program):
    expressions = document['uniformExpressionSet']['pixelTexture2DExpressions']
    mapping, lookup, mask = {}, set(), 0
    for binding in program['bindings']['textures']:
        index = binding['expressionIndexOrGroup']
        mapping[binding['baseIndex']] = index
        mask |= 1 << index
        if expressions[index].get('parameterName') in LOOKUP_TEXTURES:
            lookup.add(index)
    return mapping, lookup, mask


def emit_function(document, family, number, stage):
    program = document['programs'][BASE_TYPE if stage == 'base' else LIGHT_TYPE]
    time_parameter = time_parameter_of(document)
    texture_map, lookup, _mask = texture_map_of(document, program)
    constants = 'g_SourceCharacterBaseConstants' if stage == 'base' else 'g_SourceCharacterLightConstants'
    name = ('SourceCharacterBase' if stage == 'base' else 'SourceCharacterLight') + str(number)
    lines = [f'// {family} / source program {program["shaderId"]}',
             f'SOURCE_CHARACTER_NATIVE_OUTPUT {name}(SOURCE_CHARACTER_NATIVE_INPUT input)', '{',
             '    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;',
             '    float4 source[64];',
             f'    [unroll] for (uint i=0u;i<64u;++i) source[i]={constants}[i];']
    for row, lane, node in stage_rows(document, program):
        if node is None or not uses_time(node):
            continue
        if lane is None:
            lines.append(f'    source[{row}]={hlsl(node, time_parameter)};')
        else:
            lines.append(f'    source[{row}].{"xyzw"[lane]}=({hlsl(node, time_parameter)}).x;')
    if stage == 'light':
        trailing = program['bindings']['constantBufferClosure']['trailingUnownedConstantBuffer0Slots']
        lines.append(f'    source[{trailing[0]}]=float4(input.lightColor,1.0);')
        lines.append(f'    source[{trailing[1]}].x=1.0;')
    lines.append('    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];')
    lines.append('    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};')
    lines.append('    float4 ' + ', '.join(f'v{i} = input.values[{i}]' for i in range(10)) + ';')
    temps = next(int(d.split()[1]) for d in program['disassembly']['declarations'] if d.startswith('dcl_temps'))
    lines.append('    float4 ' + ', '.join(f'r{i}=0.0' for i in range(temps)) + ';')
    for index, instruction in enumerate(program['disassembly']['instructions'], 1):
        lines.append(f'    // {index}: {instruction}')
        lines.append('    ' + translate(instruction, texture_map, lookup, stage))
    lines.append('}')
    return '\n'.join(lines) + '\n'


def emit_configure(document, family, number):
    time_parameter = time_parameter_of(document)
    lines = [f'    else if (family == "{family}")', '    {', f'        staged.program = {number}u;']
    if time_parameter is not None:
        lines.append(f'        staged.baseConstants[{TIME_PARAMETER_ROW}] = vector(parameter("{time_parameter}"));')
        lines.append(f'        staged.lightConstants[{TIME_PARAMETER_ROW}] = vector(parameter("{time_parameter}"));')
    for stage, program_type in (('base', BASE_TYPE), ('light', LIGHT_TYPE)):
        program = document['programs'][program_type]
        lines.append(f'        staged.{stage}TextureMask = {texture_map_of(document, program)[2]}u;')
        grouped = {}
        for row, lane, node in stage_rows(document, program):
            grouped.setdefault(row, []).append((lane, node))
        for row in sorted(grouped):
            entries = grouped[row]
            if entries[0][0] is None:
                lines.append(f'        staged.{stage}Constants[{row}] = vector({cpp(entries[0][1])});')
            else:
                lanes = [(cpp(node) + '[0]') if node is not None else '0.f' for _lane, node in entries]
                lines.append(f'        staged.{stage}Constants[{row}] = float4_t({",".join(lanes)});')
    lines.append('    }')
    return '\n'.join(lines) + '\n'


def read_text(path):
    return path.read_bytes().decode('utf8').replace('\r\n', '\n')


def installed_function(path, name):
    lines = read_text(path).split('\n')
    start = next((i for i, l in enumerate(lines) if l.startswith(f'SOURCE_CHARACTER_NATIVE_OUTPUT {name}(')), None)
    if start is None:
        return None
    head = start - 1 if lines[start - 1].startswith('// source.') else start
    end = next(i for i in range(start, len(lines)) if lines[i] == '}')
    return '\n'.join(lines[head:end + 1]) + '\n'


def installed_configure(family):
    lines = read_text(PARAMETER_HEADER).split('\n')
    start = next((i for i, l in enumerate(lines) if f'family == "{family}"' in l), None)
    if start is None:
        return None
    end = next(i for i in range(start + 2, len(lines)) if lines[i] == '    }')
    body = [l for l in lines[start:end + 1] if not l.strip().startswith('//')]
    return '\n'.join(body) + '\n'


def load_document(path):
    return json.loads(pathlib.Path(path).read_text(encoding='utf8'))


def command_verify(arguments):
    document = load_document(arguments.dump)
    failures = 0
    for stage, path in (('base', BASE_PROGRAMS), ('light', LIGHT_PROGRAMS)):
        name = ('SourceCharacterBase' if stage == 'base' else 'SourceCharacterLight') + str(arguments.program)
        installed = installed_function(path, name)
        generated = emit_function(document, arguments.family, arguments.program, stage)
        exact = installed == generated
        failures += 0 if exact else 1
        print(stage, 'EXACT' if exact else 'MISMATCH', len(generated.splitlines()), 'lines')
    installed = installed_configure(arguments.family)
    generated = emit_configure(document, arguments.family, arguments.program)
    reserved = f'staged.baseConstants[{TIME_PARAMETER_ROW}]', f'staged.lightConstants[{TIME_PARAMETER_ROW}]'
    comparable = ''.join(l + '\n' for l in generated.splitlines() if not l.strip().startswith(reserved))
    exact = installed is not None and installed.replace('    if (family', '    else if (family') in (generated, comparable)
    failures += 0 if exact else 1
    print('configure', 'EXACT' if exact else 'MISMATCH')
    if failures:
        fail(f'reference program {arguments.program} was not reproduced')


def command_generate(arguments):
    document = load_document(arguments.dump)
    arguments.out.mkdir(parents=True, exist_ok=True)
    (arguments.out / f'base{arguments.program}.hlsli').write_text(emit_function(document, arguments.family, arguments.program, 'base'), encoding='utf8')
    (arguments.out / f'light{arguments.program}.hlsli').write_text(emit_function(document, arguments.family, arguments.program, 'light'), encoding='utf8')
    (arguments.out / f'configure{arguments.program}.h').write_text(emit_configure(document, arguments.family, arguments.program), encoding='utf8')
    print('generated', arguments.program, arguments.family)


def write_preserving_newlines(path, text):
    before = path.read_bytes()
    newline = '\r\n' if b'\r\n' in before else '\n'
    after = text.replace('\r\n', '\n').replace('\n', newline).encode('utf8')
    if before != after:
        path.write_bytes(after)


def install_program(path, function, number, stage):
    text = read_text(path)
    prefix = 'SourceCharacterBase' if stage == 'base' else 'SourceCharacterLight'
    name = f'{prefix}{number}'
    existing = installed_function(path, name)
    if existing is not None:
        if existing != function:
            fail(f'{path.name} already holds a different {name}')
        return text
    evaluate = f'\n\n\nSOURCE_CHARACTER_NATIVE_OUTPUT Evaluate{prefix}(SOURCE_CHARACTER_NATIVE_INPUT input)'
    last_case = '    case 84u: return ' + prefix + '84(input);\n'
    if text.count(evaluate) != 1 or text.count(last_case) != 1 or f'case {number}u:' in text:
        fail(f'{path.name} dispatch anchors changed')
    text = text.replace(evaluate, '\n\n' + function.rstrip('\n') + evaluate)
    return text.replace(last_case, last_case + f'    case {number}u: return {name}(input);\n')


def command_install(arguments):
    base = (arguments.generated / f'base{arguments.program}.hlsli').read_text(encoding='utf8')
    light = (arguments.generated / f'light{arguments.program}.hlsli').read_text(encoding='utf8')
    configure = (arguments.generated / f'configure{arguments.program}.h').read_text(encoding='utf8')
    for path, function, stage in ((BASE_PROGRAMS, base, 'base'), (LIGHT_PROGRAMS, light, 'light')):
        write_preserving_newlines(path, install_program(path, function, arguments.program, stage))
        shutil.copyfile(path, CLIENT_SHADER_DIR / path.name)
    header = read_text(PARAMETER_HEADER)
    existing = installed_configure(arguments.family)
    if existing is None:
        anchor = '    else return false;\n    if (staged.program == 80u)\n'
        if header.count(anchor) != 1:
            fail('SourceCharacterMaterialParameters.h family anchor changed')
        header = header.replace(anchor, configure + anchor)
        write_preserving_newlines(PARAMETER_HEADER, header)
    elif existing != configure:
        fail(f'{arguments.family} is already installed with a different packing')
    print('installed program', arguments.program)


def command_rows(arguments):
    resources = json.loads(arguments.texture_map.read_text(encoding='utf8'))
    rows = []
    for spec in arguments.entry:
        dump_path, family, *generated = spec.split('=')
        document = load_document(dump_path)
        block = pathlib.Path(generated[0]).read_text(encoding='utf8') if generated else installed_configure(family)
        if block is None:
            fail(f'{family} is not installed')
        names = sorted(set(re.findall(r'parameter\("([^"]+)"\)', block)))
        defaults = {}

        def collect(node):
            if isinstance(node, dict):
                if node.get('typeName') in ('fmaterialuniformexpressionscalarparameter', 'fmaterialuniformexpressionvectorparameter'):
                    defaults.setdefault(node['parameterName'], node['defaultValue'])
                for value in node.values():
                    collect(value)
            elif isinstance(node, list):
                for value in node:
                    collect(value)
        collect(document['uniformExpressionSet'])
        parameters = {}
        for name in names:
            if name in document['vectors']:
                value = document['vectors'][name]
                parameters[name] = [float(value[k]) for k in ('r', 'g', 'b', 'a')] if isinstance(value, dict) else [float(v) for v in value]
            elif name in document['scalars']:
                parameters[name] = [float(document['scalars'][name])] * 4
            elif name in defaults:
                value = defaults[name]
                parameters[name] = [float(v) for v in value] if isinstance(value, list) else [float(value)] * 4
            else:
                fail(f'{document["sourceMaterial"]} parameter {name} has neither override nor default')
        mask = 0
        for value in re.findall(r'staged\.(?:base|light)TextureMask = (\d+)u', block):
            mask |= int(value)
        textures = []
        for texture in document['textures']:
            if not mask & (1 << texture['expressionIndex']):
                continue
            leaf = texture['sourceObject'].split('.')[-1].lower()
            if leaf not in resources:
                fail(f'{texture["sourceObject"]} has no Resources mapping')
            textures.append(dict(expressionIndex=texture['expressionIndex'], assetId=resources[leaf],
                                 colorSpace='srgb' if texture['srgb'] else 'linear'))
        if sum(1 << t['expressionIndex'] for t in textures) != mask:
            fail(f'{document["sourceMaterial"]} does not supply every required texture')
        material_name = document['sourceMaterial'].partition('.mat.')[2]
        rows.append(dict(modelAssetId=arguments.model, materialName=material_name,
                         sourceMaterial=document['sourceMaterial'], family=family,
                         parameters=parameters, textures=textures))
    arguments.out.write_text(json.dumps(rows, indent=2) + '\n', encoding='utf8')
    print('rows', len(rows), '->', arguments.out)


def command_textures(arguments):
    resources = json.loads(arguments.texture_map.read_text(encoding='utf8'))
    for leaf, asset_id in sorted(resources.items()):
        destination = RESOURCES / asset_id
        if asset_id.startswith('Character/SourceMaterials/'):
            if not destination.is_file():
                fail(f'shared source material is missing: {asset_id}')
            continue
        candidates = [p for p in arguments.staging.rglob(leaf + pathlib.Path(asset_id).suffix)]
        if len(candidates) != 1:
            fail(f'{leaf} must exist exactly once under the staging tree, found {len(candidates)}')
        destination.parent.mkdir(parents=True, exist_ok=True)
        if destination.is_file() and destination.read_bytes() != candidates[0].read_bytes():
            fail(f'{asset_id} already exists with different bytes')
        if not destination.is_file():
            shutil.copyfile(candidates[0], destination)
        print('texture', asset_id)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest='command', required=True)
    extract = commands.add_parser('extract')
    extract.add_argument('--umodel', type=pathlib.Path, required=True)
    extract.add_argument('--d3dcompiler', type=pathlib.Path, required=True)
    extract.add_argument('--out', type=pathlib.Path, required=True)
    extract.add_argument('materials', nargs='+')
    for name in ('verify', 'generate'):
        sub = commands.add_parser(name)
        sub.add_argument('--dump', type=pathlib.Path, required=True)
        sub.add_argument('--family', required=True)
        sub.add_argument('--program', type=int, required=True)
        if name == 'generate':
            sub.add_argument('--out', type=pathlib.Path, required=True)
    install = commands.add_parser('install')
    install.add_argument('--generated', type=pathlib.Path, required=True)
    install.add_argument('--family', required=True)
    install.add_argument('--program', type=int, required=True)
    rows = commands.add_parser('rows')
    rows.add_argument('--model', required=True)
    rows.add_argument('--texture-map', type=pathlib.Path, required=True)
    rows.add_argument('--out', type=pathlib.Path, required=True)
    rows.add_argument('entry', nargs='+')
    textures = commands.add_parser('textures')
    textures.add_argument('--texture-map', type=pathlib.Path, required=True)
    textures.add_argument('--staging', type=pathlib.Path, required=True)
    arguments = parser.parse_args()
    {'extract': command_extract, 'verify': command_verify, 'generate': command_generate,
     'install': command_install, 'rows': command_rows, 'textures': command_textures}[arguments.command](arguments)


if __name__ == '__main__':
    main()
