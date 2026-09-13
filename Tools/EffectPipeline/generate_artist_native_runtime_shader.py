"""Offline generation of the selected Artist native material programs.

Reuses the earlier instruction translator; no runtime interpreter is added.
Engine depth reconstruction is an explicit centimetre adapter. Original source
expressions, named leaves, sample modes, masks and operation order are retained.
"""
from pathlib import Path
import argparse, json, re, math, hashlib, copy

ROOT = Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description='Generate bounded Artist native HLSL from extracted source shader maps and Resources mappings.')
parser.add_argument('--source-dir',type=Path,default=ROOT/'out/ArtistCoreRestore20260909')
parser.add_argument('--selection-file', type=Path,
    help='Read an explicitly selected companion pass list from this file.')
parser.add_argument('--output-dir', type=Path,
    help='Write a companion pass result without copying the shared source archive.')
parser.add_argument('--profile-domain', choices=('artist','kouku'), default='artist')
parser.add_argument('--program-start',type=int,default=None,
    help='Allocate an additional non-overlapping Artist program range without renumbering the original programs.')
parser.add_argument('--install-additional-groups',action='store_true',
    help='Install the generated additional program groups while preserving every existing Artist shader group.')
arguments=parser.parse_args()
OUT=arguments.source_dir.resolve()
source = json.loads((OUT / 'native_material_inputs.json').read_text())
def args(s):
    return [x.strip() for x in re.split(',\\s*(?![^()]*\\))', s)]

def immediate(s, integer=False):
    numbers = [x.strip() for x in s[2:-1].split(',')]
    numbers += numbers[-1:] * (4 - len(numbers))
    if integer:
        return 'uint4(' + ','.join(((hex(int(n, 16) & 4294967295) if n.lower().startswith('0x') else str(int(n, 10) & 4294967295)) + 'u' for n in numbers)) + ')'

    def lane(n):
        if n.lower().startswith('0x'):
            return 'asfloat(' + n + 'u)'
        return n if '.' in n or 'e' in n.lower() else 'asfloat(' + str(int(n, 10) & 4294967295) + 'u)'
    return 'float4(' + ','.join((lane(n) for n in numbers)) + ')'

def uint_operand(s):
    return immediate(s.strip(), True) if s.strip().startswith('l(') else 'asuint(' + operand(s) + ')'

def operand(s, destination=False):
    s = s.strip()
    if s == 'null':
        return 'unused'
    negative = s.startswith('-')
    s = s[1:] if negative else s
    absolute = s.startswith('|') and s.endswith('|')
    s = s[1:-1] if absolute else s
    if s.startswith('l('):
        s = immediate(s)
    else:
        s = re.sub('\\bcb0\\[', 'source[', s)
        s = re.sub('\\bcb1\\[', 'projection[', s)
        s = re.sub('\\bcb2\\[', 'passValues[', s)
        s = s.replace('vCoverage.x', '1.0')
        if not destination:
            s = re.sub('\\.([xyzw])$', lambda m: '.' + m[1] * 4, s)
    if absolute:
        s = 'abs(' + s + ')'
    return '-(' + s + ')' if negative else s

def result_mask(dst, value):
    match = re.search('\\.([xyzw]+)$', dst)
    mask = match.group(1) if match else 'xyzw'
    return operand(dst, True) + ' = (' + value + ').' + mask + ';'

def translated(ins, textureMap, stage):
    op, _, tail = ins.partition(' ')
    a = args(tail)
    sat = op.endswith('_sat')
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
    if a[0].startswith('oMask'):
        return '// Coverage is owned by the product rasterizer.'
    if op.startswith('sample'):
        dst, uv, tex, sampler = a[:4]
        m = re.match('t(\\d+)\\.([xyzw]+)', tex)
        register = int(m[1])
        swizzle = m[2]
        if register in textureMap:
            index = textureMap[register]
            sample = f'g_SourceCharacterTexture{index}'
            mode = 'SourceCharacterLookupSampler' if index in currentLookup else 'SourceCharacterSampler'
            if 'sample_l' in op:
                sample += f'.SampleLevel({mode}, ({operand(uv)}).xy, ({operand(a[4])}).x)'
            elif 'sample_b' in op:
                sample += f'.SampleBias({mode}, ({operand(uv)}).xy, ({operand(a[4])}).x)'
            else:
                sample += f'.Sample({mode}, ({operand(uv)}).xy)'
        elif 'texturecube' in op:
            sample = 'float4(0.0,0.0,0.0,0.0)'
        else:
            sample = 'float4(sqrt(saturate(input.shadow)).xxx,1.0)' if stage == 'light' else 'float4(0.0,0.0,0.0,0.0)'
        return result_mask(dst, '(' + sample + ').' + swizzle)
    src = [operand(x) for x in a[1:]]
    if op in ['mov', 'mov_sat']:
        value = src[0]
    elif op in ['add', 'mul', 'div']:
        value = '(' + src[0] + ')' + {'add': '+', 'mul': '*', 'div': '/'}[op] + '(' + src[1] + ')'
    elif op == 'mad':
        value = '(' + src[0] + ')*(' + src[1] + ')+(' + src[2] + ')'
    elif op in ['dp2', 'dp3', 'dp4']:
        mask = {'dp2': 'xy', 'dp3': 'xyz', 'dp4': 'xyzw'}[op]
        value = 'dot((' + src[0] + ').' + mask + ',(' + src[1] + ').' + mask + ').xxxx'
    elif op in ['min', 'max']:
        value = op + '(' + ','.join(src) + ')'
    elif op == 'rcp':
        value = '1.0/(' + src[0] + ')'
    elif op in ['rsq', 'sqrt', 'log', 'exp', 'frc', 'round_ni', 'round_pi']:
        value = {'rsq': 'rsqrt', 'sqrt': 'sqrt', 'log': 'log2', 'exp': 'exp2', 'frc': 'frac', 'round_ni': 'floor', 'round_pi': 'ceil'}[op] + '(' + src[0] + ')'
    elif op in ['lt', 'ge', 'ne', 'eq']:
        value = 'asfloat((uint4)((' + src[0] + ')' + {'lt': '<', 'ge': '>=', 'ne': '!=', 'eq': '=='}[op] + '(' + src[1] + ')) * 0xffffffffu)'
    elif op == 'movc':
        value = '(' + uint_operand(a[1]) + ' != 0u) ? (' + src[1] + ') : (' + src[2] + ')'
    elif op in ['and', 'or', 'xor']:
        value = 'asfloat(' + uint_operand(a[1]) + {'and': ' & ', 'or': ' | ', 'xor': ' ^ '}[op] + uint_operand(a[2]) + ')'
    elif op == 'not':
        value = 'asfloat(~' + uint_operand(a[1]) + ')'
    elif op == 'ftou':
        value = 'asfloat((uint4)(' + src[0] + '))'
    elif op == 'utof':
        value = '(float4)(' + uint_operand(a[1]) + ')'
    elif op == 'itof':
        value = '(float4)(asint(' + src[0] + '))'
    elif op == 'iadd':
        value = 'asfloat(' + uint_operand(a[1]) + ' + ' + uint_operand(a[2]) + ')'
    elif op == 'bfi':
        value = 'SourceCharacterBitInsert(' + ','.join((uint_operand(x) for x in a[1:])) + ')'
    elif op in ['ishl', 'ushr']:
        value = 'asfloat(' + uint_operand(a[1]) + (' << ' if op == 'ishl' else ' >> ') + '(' + uint_operand(a[2]) + ' & 31u))'
    elif op in ['deriv_rtx_coarse', 'deriv_rty_coarse']:
        value = ('ddx_coarse' if 'rtx' in op else 'ddy_coarse') + '(' + src[0] + ')'
    elif op == 'sincos':
        parts = []
        if a[0] != 'null':
            parts.append(result_mask(a[0], 'sin(' + operand(a[2]) + ')'))
        if a[1] != 'null':
            parts.append(result_mask(a[1], 'cos(' + operand(a[2]) + ')'))
        return ' '.join(parts)
    else:
        raise ValueError((op, ins))
    if sat:
        value = 'saturate(' + value + ')'
    return result_mask(a[0], value)

ns = {name: globals()[name] for name in ['args', 'immediate', 'uint_operand', 'operand', 'result_mask', 'translated']}
original_translate=translated

def f(x):
    s = format(float(x), '.9g')
    return s if '.' in s or 'e' in s.lower() else s + '.0'

def vec(x):
    return 'float4(' + ', '.join(f(v) for v in x) + ')'

def walk(x):
    if isinstance(x, dict):
        yield x
        for v in x.values():
            yield from walk(v)
    elif isinstance(x, list):
        for v in x:
            yield from walk(v)

asset_map=json.loads((OUT/'texture_asset_map.json').read_text())
def asset(path):
    if path not in asset_map:raise ValueError(('source texture has no exact runtime mapping',path))
    return asset_map[path]

prefix = '''// Artist native RT0 material programs, generated from exact source shader maps.
// The existing CModel and effect carriers provide the declared runtime inputs.
#ifndef EFFECT_ARTIST_NATIVE_HLSLI
#define EFFECT_ARTIST_NATIVE_HLSLI
#ifndef ARTIST_NATIVE_MODEL_ONLY
#include "Shader_EffectSliceSceneDepth.hlsli"
#include "Shader_EffectCubeSampleScene.hlsli"
#endif

float4 g_ArtistSourceMaterialParameters[32];
float g_ArtistSourceMaterialTime = 0.f;

float4 ArtistNativeAppend(float4 a, float4 b, uint n)
{
    if (n == 1u) return float4(a.x, b.xyz);
    if (n == 2u) return float4(a.xy, b.xy);
    if (n == 3u) return float4(a.xyz, b.x);
    return a;
}
float4 ArtistNativePeriodic(float4 a) { return sign(a) * frac(abs(a)); }

struct ARTIST_NATIVE_INPUT
{
    float2 uv;
    float2 uv1;
    float2 uvNext;
    float subUVBlend;
    float3 sourceWorldPosition;
    float3 sourceBasisX;
    float3 sourceBasisZ;
    float handedness;
    float4 vertexColor;
    float2 screenUV;
    float projectionW;
    float projectionZ;
    float3 tangentView;
    float4 color;
    float4 dynamicParameter;
    bool frontFace;
    float4 sourceProjection[4];
    float3 tangentUp;
    float3 sourceCameraPosition;
    float3 sourceActorPosition;
    float3 skyUpperColor;
    float3 skyLowerColor;
    float3 ambientColor;
    float skyIntensity;
    float4 decalProjection; // Source near/far (cm), opacity.
};
'''
for i in range(10):
    prefix += f'\nfloat4 ArtistNativeSample{i}(float2 uv, float lod, bool explicitLod)\n{{\n'
    prefix += f'    const uint mode = (((g_SourceTextureClampVMask >> {i}u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> {i}u) & 1u);\n'
    for mode, sampler in [(1,'LinearClampUSampler'), (2,'LinearClampVSampler'), (3,'LinearClampUVSampler'), (0,'LinearSampler')]:
        conditional = f'if (mode == {mode}u) ' if mode else ''
        prefix += f'    {conditional}return explicitLod ? g_SourceTexture{i}.SampleLevel({sampler}, uv, lod) : g_SourceTexture{i}.SampleBias({sampler}, uv, lod);\n'
    prefix += '}\n'

rows = []
program_code = []
selections=json.loads((arguments.selection_file or OUT/'selected_runtime_material_programs.json').read_text())['programs']
byid={r['sourceMaterial']:r for r in source['materials']};errors=[]
for ordinal, selection in enumerate(selections):
    program=selection.get('program', arguments.program_start+ordinal if arguments.program_start is not None else (460+ordinal if ordinal<100 else 820+ordinal-100))
    try:
        r=byid[selection['resolvedMaterial']]
        if r['sourceMaterial'] in ('fx_mastermaterial.fx_mi.fx_mm_onelayerdistortion_02_01_ad','fx_m_mi_j_00.fx_mi.fx_j_pa_hologram_01_01_tr'):
            errors.append({'program':program,'material':r['sourceMaterial'],'occurrences':selection['occurrences'],'reason':'Explicit new engine/VF input requires closure: OneLayer clipZ and cb0[10].x; Hologram source-world varying, cb0[0].xyz origin and .w opacity, distinct source VS.'});continue

        kouku_ice = (arguments.profile_domain == 'kouku' and
            selection['sourceVS'] == '5298fd1cc3a2f64dab8401a32a6bef3c' and
            selection['sourcePS'] == '97220ed990d76147b9d9778b693101c0')
        if not kouku_ice and selection['sourceVS'] in ('5298fd1cc3a2f64dab8401a32a6bef3c','792dc3606e73a443b8013c7adc2b97d8','cae853451adaf845b2558adb48edc97d'):
            errors.append({'program':program,'material':r['sourceMaterial'],'reason':'Distinct non-unlit engine CB prefix, world-position varying; separately restore exact inputs.'});continue
        name=r['sourceMaterial'].rsplit('.',1)[-1]
        sid=selection['sourcePS']
        p=json.loads((OUT/'full_programs'/(sid+'.json')).read_text())
        uniform = copy.deepcopy(r['materialMap']['uniformExpressionSet'])
        for node in walk(uniform):
            if node.get('parameterNameNumber',0):
                node['nativeParameterFName']={'name':node['parameterName'],'number':node['parameterNameNumber']}
                node['parameterName']=node['parameterName']+'_'+str(node['parameterNameNumber']-1)
                node['parameterNameNumber']=0
        bindings = p['bindings']
        numeric=r['effectiveNumericOverrides']
        overrides={('scalarparameter',k):v['value'] for k,v in numeric['scalars'].items()}
        overrides.update({('vectorparameter',k):v['value'] for k,v in numeric['vectors'].items()})
        params = {}; distinctDefaults={}
        for node in walk(uniform):
            kind = node.get('typeName', '').removeprefix('fmaterialuniformexpression')
            if kind not in ('scalarparameter','vectorparameter'):
                continue
            assert node['parameterNameNumber'] == 0
            key = (kind, node['parameterName'])
            if key in params and params[key]['default'] != node['defaultValue']:
                distinctDefaults.setdefault(key,[params[key]['default']]).append(node['defaultValue'])
            params[key] = {'kind':'scalar' if kind=='scalarparameter' else 'vector',
                           'name':node['parameterName'], 'default':node['defaultValue']}
        fixedNodeDefaults={k:v for k,v in distinctDefaults.items() if k not in overrides}
        for key in fixedNodeDefaults:params.pop(key)
        scalar_count = sum(k[0] == 'scalarparameter' for k in params)
        scalar_slot = 0
        vector_slot = (scalar_count + 3) // 4
        for key in sorted(params):
            param = params[key]
            if key[0] == 'scalarparameter':
                param.update(row=scalar_slot//4, lane=scalar_slot%4)
                scalar_slot += 1
            else:
                param.update(row=vector_slot, lane=None)
                vector_slot += 1
            param['effective'] = overrides.get(key,param['default'])
        assert vector_slot <= 32

        def expression(x):
            t = x['typeName'].removeprefix('fmaterialuniformexpression')
            if t == 'constant': return vec(x['value'])
            if t in ('scalarparameter','vectorparameter'):
                if x['parameterName'] == 'meshemitterdynamicparameter': return 'input.dynamicParameter'
                if (t,x['parameterName']) in fixedNodeDefaults:return vec(x['defaultValue'] if t=='vectorparameter' else [x['defaultValue']]*4)
                a = params[t,x['parameterName']]
                return 'g_ArtistSourceMaterialParameters[%du]' % a['row'] + ('.'+'xyzw'[a['lane']]*4 if t=='scalarparameter' else '')
            if t in ('time','realtime'): return 'g_ArtistSourceMaterialTime.xxxx'
            if t == 'foldedmath':
                assert x['operationOrdinal'] in range(4)
                return '('+expression(x['a'])+['+','-','*','/'][x['operationOrdinal']]+expression(x['b'])+')'
            if t == 'sine': return ('cos' if x['isCosine'] else 'sin')+'('+expression(x['input'])+')'
            if t == 'appendvector': return 'ArtistNativeAppend('+expression(x['a'])+','+expression(x['b'])+','+str(x['componentsFromA'])+'u)'
            if t == 'periodic': return 'ArtistNativePeriodic('+expression(x['input'])+')'
            if t == 'max': return 'max('+expression(x['a'])+','+expression(x['b'])+')'
            if t == 'floor': return 'floor('+expression(x['input'])+')'
            if t == 'abs': return 'abs('+expression(x['input'])+')'
            if t == 'fmod': return 'fmod('+expression(x['a'])+','+expression(x['b'])+')'
            if t == 'clamp': return 'clamp('+expression(x['input'])+','+expression(x['minimum'])+','+expression(x['maximum'])+')'
            raise ValueError(('unhandled expression', t))

        textures = []
        for x in r['effectiveTextures']:
            prop = x.get('properties', {})
            normal = prop.get('srgb',{}).get('value') is False
            textures.append({'index':x['index'], 'name':x.get('parameterName') or 'native_texture_'+str(x['index']),
                 'sourceObjectPath':x['sourceObjectPath'], 'assetId':asset(x['sourceObjectPath']),
                 'addressU':'clamp' if prop.get('addressx',{}).get('value')=='ta_clamp' else 'wrap',
                 'addressV':'clamp' if prop.get('addressy',{}).get('value')=='ta_clamp' else 'wrap',
                 'colorSpace':'linear' if normal else 'srgb',
                 'samplingEvidence':'source-texture-tags-and-project-class-default.v1',
                 'nativeFormat':prop.get('format',{}).get('value'),
                 'colorSpaceEvidence':'EXPLICIT_NATIVE_SRGB_FALSE' if normal else 'PROJECT_INTERPRETED_CLASS_DEFAULT_SRGB',
                 'exists':(ROOT/'Client/Bin/Resources'/asset(x['sourceObjectPath'])).is_file()})
        texture_map={x['baseIndex']:x['expressionIndexOrGroup'] for x in bindings['textures']}
        declarations=p['disassembly']['declarations'];instructions=p['disassembly']['instructions']
        last_rt0=max(i for i,line in enumerate(instructions) if re.search(r'\bo0\.',line))
        instructions=instructions[:last_rt0+1]
        ntemps=int(next(d.split()[-1] for d in declarations if d.startswith('dcl_temps')))
        mesh=selection['rendererShape'] in ['mesh','staticMesh']
        model=selection['rendererShape']=='skeletalMesh'
        decal=selection['rendererShape']=='decal' and arguments.profile_domain=='kouku'
        # These original lit particle programs use the same tangent-up/scene
        # contract as the already restored skeletal and decal carriers. Keep
        # exact VS/PS qualification: TEXCOORD5 is world position in the three
        # prop programs, but clip position in the heat mesh and debris sprite.
        kouku_lit = {
            '0536b29c3525994fa5ccf365c7edc279': ('3973e380b469714a933b0f1c2c776d5c', 27, 'world', [0, 1, 27, 28, 29]),
            '10215f76a54bc242a767938c5056e6fd': ('3973e380b469714a933b0f1c2c776d5c', 24, 'world', [0, 1, 24, 25, 26]),
            '1e5c374e163c74468caf9b27d3d32081': ('0c1413bd3ee54d449ce7fdac8c7f1542', 6, 'actor_only', [0, 6, 7, 8]),
            '1eb6e82b0befd243ba7ffc9e49b6d067': ('0c1413bd3ee54d449ce7fdac8c7f1542', 10, 'opacity', [0, 9, 10, 11, 12]),
            '2b41020e9d64484f9b0c844745af2143': ('239396ffe9f57b47a19ee2955207d2e8', 29, 'actor', [0, 1, 2, 29, 30, 31]),
            '42385aa99c04e44daca13d0f3c9bba1a': ('98c6e0ba26dfb24c861e43351b178e02', 20, 'opacity', [0, 1, 20, 21, 22]),
            '42ebb4e66c0c0b4b92db497fcd69ccc2': ('0c1413bd3ee54d449ce7fdac8c7f1542', 10, 'opacity', [0, 9, 10, 11, 12]),
            '7f3d666a6ef0984881ca1d6f72aa3a5a': ('239396ffe9f57b47a19ee2955207d2e8', 18, 'world', [0, 1, 18, 19, 20]),
            '87df68790a46d341b4221de10307d126': ('239396ffe9f57b47a19ee2955207d2e8', 14, 'world', [0, 1, 14, 15, 16]),
            '9b28c9df74055140a218839b324676e8': ('0c1413bd3ee54d449ce7fdac8c7f1542', 14, 'none', [14, 15, 16]),
            'ab0d30ff2ccc914b84c29564ae10ccd8': ('239396ffe9f57b47a19ee2955207d2e8', 27, 'world', [0, 1, 27, 28, 29]),
            'ef38165f71918a49b4c84fb870c74334': ('3973e380b469714a933b0f1c2c776d5c', 30, 'actor', [0, 1, 2, 30, 31, 32]),
            'fd7807729e65a14eae4074a3937bd61f': ('3973e380b469714a933b0f1c2c776d5c', 19, 'world', [0, 1, 19, 20, 21]),
            'e5ff54c5c354204e951b91b524341cb3': ('239396ffe9f57b47a19ee2955207d2e8', 23, 'world', [0, 1, 23, 24, 25]),
            '231a7f149fd8054589dc4baf0825beb0': ('3973e380b469714a933b0f1c2c776d5c', 27, 'actor', [0, 1, 2, 27, 28, 29]),
            '21310dd53f047b4c9c6d215723e659ae': ('70a7b0749eb5904898747857eecc9da2', 7, 'opacity', [0, 7, 8, 9]),
            '3a96e00bdfda46489bb6aa32ae1ac89c': ('0c1413bd3ee54d449ce7fdac8c7f1542', 6, 'color', [0, 5, 6, 7, 8]),
            '8c7feae3b54e7a46835555bfa86e7e6e': ('239396ffe9f57b47a19ee2955207d2e8', 26, 'actor', [0, 1, 2, 26, 27, 28]),
        }.get(sid) if arguments.profile_domain=='kouku' else None
        if kouku_lit:
            assert selection['sourceVS'] == kouku_lit[0], ('Kouku lit vertex shader mismatch', selection['sourceVS'], kouku_lit[0])
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == kouku_lit[3], ('Kouku lit engine rows mismatch', bindings['constantBufferClosure']['unownedConstantBuffer0Slots'], kouku_lit[3])
        # Additional original LocalDecal prefixes share the same depth/opacity
        # and tangent-up inputs. Assert both the bytecode pair and all unowned
        # engine rows before assigning the existing scene adapter.
        kouku_decal = {
            'b29278299ea44042a002876ca0882388': ('210e6cca2279674b899df1b88698fd1c', None, []),
            '2b965d3b41e96649a3bdbb1f627220f5': ('5d79421dc8571c45aa49790f50274f51', 15, [0, 1, 2, 13, 14, 15, 16, 17]),
            '2f90e06395943342a0358e5a108cc044': ('772e94581a5e6548b9529bc7cc103bca', None, [0, 1, 2]),
            '31e28c4451b3b942b46cdca51e41ed25': ('5d79421dc8571c45aa49790f50274f51', 13, [0, 1, 2, 11, 12, 13, 14, 15]),
            '6861d20b5b3dcb4da749ef0527cd7c61': ('5d79421dc8571c45aa49790f50274f51', 14, [0, 1, 2, 12, 13, 14, 15, 16]),
            '75f85c320aac2348b627cf36a23eaa8f': ('5d79421dc8571c45aa49790f50274f51', 7, [0, 1, 2, 5, 6, 7, 8, 9]),
            '7c9480f7ccb19943a483bd123ba9e785': ('5d79421dc8571c45aa49790f50274f51', 19, [0, 1, 2, 17, 18, 19, 20, 21]),
            '87e3ec1a518e5b478b308c5308ce7773': ('5d79421dc8571c45aa49790f50274f51', 15, [0, 1, 2, 13, 14, 15, 16, 17]),
            '89b63739ee6a9748ac3cd766197e05b0': ('5d79421dc8571c45aa49790f50274f51', 14, [0, 1, 2, 12, 13, 14, 15, 16]),
            '98db4d025dd8144697a4bdf58b96a698': ('5d79421dc8571c45aa49790f50274f51', 14, [0, 1, 2, 12, 13, 14, 15, 16]),
            'a274569aeaf4494e9d17bbd0f6ba86f1': ('5d79421dc8571c45aa49790f50274f51', 9, [0, 1, 2, 7, 8, 9, 10, 11]),
            'bafb98b5f548184bab9a40cf2ed5d9c1': ('5d79421dc8571c45aa49790f50274f51', 16, [0, 1, 2, 14, 15, 16, 17, 18]),
            'e0c1218459011e4ba1a50550c3bf358e': ('5d79421dc8571c45aa49790f50274f51', 17, [0, 1, 2, 15, 16, 17, 18, 19]),
            'f5b16f4555698c43993f2cde2666d2d2': ('772e94581a5e6548b9529bc7cc103bca', None, [0, 1, 2]),
        }.get(sid) if decal else None
        if kouku_decal:
            assert selection['sourceVS'] == kouku_decal[0]
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == kouku_decal[2]
        kouku_ground = decal and sid in (
            'ef9cad5cf42011438c3b2ff2ecf7baee',
            'b123a95ca0a96e488268a58a2998fa43',
            '76df7d394e7b4242b700d26cf69db77b')
        if kouku_ground:
            # GroundEffect's LocalDecal VS exports absolute world position at
            # TEXCOORD5. Its PS adds a pre-view translation before the existing
            # projection/color/opacity prefix, unlike Cascade LocalDecal.
            assert selection['sourceVS'] == '51afa7d015c2db45bc7c7faf7300c9c3'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0, 1, 2, 3]
        if decal and not kouku_decal and not kouku_ground and sid not in ('be9bb8ea52a06b40bc25b550e349b5b9','316b66ee3867964da197becf270077f0','aacf33d926f3884493fb98d76d43506c','92378d29e44d7046b15b6af899336298','cd75326f74ef024d827113811196cae2'):
            raise ValueError(('Unreviewed source decal prefix',sid))
        source_cb_count = bindings["constantBufferClosure"]["declaredConstantBuffer0Float4Count"]
        lines=[f'// {name}: {sid}; selected map {r["mapKey"]}.',
               f'float4 ArtistNative{program}(ARTIST_NATIVE_INPUT input)', '{',
               f'    float4 source[{max(3, source_cb_count)}]; [unroll] for (uint i=0u; i<{max(3, source_cb_count)}u; ++i) source[i]=0.f;',
               '    source[0].x=1.f; // Project engine opacity multiplier.',
               '    float4 output=0.f;']
        if model:
            lines += ['    // Existing scene adapter: source world origin is absolute; camera is converted to source cm.', '    source[0]=0.f;', '    source[1]=float4(input.sourceCameraPosition,1.f);', '    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];']
            if 'tig_00' in r['sourceMaterial']:
                lines += ['    source[2]=float4(input.sourceActorPosition,0.f); // Native actor-position seed for the emissive pulse.', '    source[3].x=input.color.a;']
                sky=28
            else:
                lines += ['    source[1].w=input.color.a;'];sky=24
            lines += [f'    source[{sky}]=float4(input.skyUpperColor,0.f);',f'    source[{sky+1}]=float4(input.skyLowerColor,0.f);',f'    source[{sky+2}]=float4(input.ambientColor,input.skyIntensity);']
        if mesh: lines += ['    source[1]=input.color; // Native mesh particle color prefix.']
        if kouku_ice:
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0, 1]
            lines += ['    source[0]=float4(0.f,0.f,0.f,1.f); // Original pre-view translation XYZ and opacity W.']
        kouku_ice_distortion = arguments.profile_domain == 'kouku' and sid == '78af666c4c816d4c93fb104c48d4bafe'
        if kouku_ice_distortion:
            assert selection['sourceVS'] == '9c33f202c8d9194ca250340eb7c4da1f'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == []
            # The original local VS exports LocalToWorld at TEXCOORD5; its
            # distortion PS then applies ViewProjection from CB1 itself.
            lines += ['    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];']
        kouku_world_distortion = arguments.profile_domain == 'kouku' and sid == '2d8c822c88934149bfa9587fd772c1e8'
        if kouku_world_distortion:
            assert selection['sourceVS'] in (
                '48f2462ce60a75419377f4ac51e71569',
                '959f42f324748b4386aab65adea222f9',
                'f560771685f7254fb3e269da6253d5d9'), ('Unreviewed world-position distortion VS', selection['sourceVS'])
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == []
            assert 'dcl_constantbuffer CB1[4], immediateIndexed' in declarations
            # These original ribbon/sprite VSs export unprojected position at
            # TEXCOORD5 and apply CB1 only to SV_POSITION. Their PS projects
            # TEXCOORD5 itself, so supplying clip position would project twice.
            lines += ['    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];']
        if kouku_lit:
            sky, prefix_kind = kouku_lit[1:3]
            if prefix_kind in ('world', 'actor'):
                lines += ['    source[0]=0.f; // Absolute source world position needs no pre-view translation.', '    source[1]=float4(input.sourceCameraPosition,1.f);', '    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];']
                if prefix_kind=='actor': lines += ['    source[2]=float4(input.sourceActorPosition,0.f); // Original actor-position pulse seed.']
            elif prefix_kind=='color':
                lines += ['    source[0]=input.color; // Native opaque mesh particle color has no opacity prefix.']
            elif prefix_kind=='actor_only':
                lines += ['    source[0]=float4(input.sourceActorPosition,0.f); // Native card UV seed is actor location in UE centimetres.']
            lines += [f'    source[{sky}]=float4(input.skyUpperColor,0.f);', f'    source[{sky+1}]=float4(input.skyLowerColor,0.f);', f'    source[{sky+2}]=float4(input.ambientColor,input.skyIntensity);']
        if mesh and sid == '8f0b8e72c2782945b5c7c927c80a73c5':
            # Quest's opaque pass has no leading opacity uniform. Its sole
            # engine-owned row is particle color; selectioncolor binds row 1.
            assert selection['sourceVS'] == '7025758b7227e342a3118ce0b01b81b2'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0]
            lines += ['    source[0]=input.color; // Native opaque quest mesh particle color prefix.']
        if decal:
            if kouku_ground:
                lines += ['    source[0]=0.f; // Absolute source world position: neutral pre-view translation.', '    source[1]=float4(input.decalProjection.xy,0.f,0.f);', '    source[2]=input.color; // Original GroundEffect ActiveColorValue.', '    source[3].x=input.decalProjection.z;']
            else:
                lines += ['    source[0]=float4(input.decalProjection.xy,0.f,0.f);', '    source[1]=input.color; // Source decal material color, including particle color modules.', '    source[2].x=input.decalProjection.z;']
            sky=kouku_decal[1] if kouku_decal else {'be9bb8ea52a06b40bc25b550e349b5b9':7,'316b66ee3867964da197becf270077f0':15}.get(sid)
            if sky is not None:
                lines += [f'    source[{sky}]=float4(input.skyUpperColor,0.f);', f'    source[{sky+1}]=float4(input.skyLowerColor,0.f);', f'    source[{sky+2}]=float4(input.ambientColor,input.skyIntensity);']
        for b in bindings['vectors']:
            exp=uniform['pixelVectorExpressions'][b['expressionIndexOrGroup']]
            lines += [f'    source[{b["baseIndex"]//16}] = {expression(exp)};']
        for b in bindings['scalarGroups']:
            for lane,exp in enumerate(uniform['pixelScalarExpressions'][b['expressionIndexOrGroup']*4:b['expressionIndexOrGroup']*4+4]):
                lines += [f'    source[{b["baseIndex"]//16}].'+ 'xyzw'[lane]+f' = ({expression(exp)}).x;']
        pass_count = max(4, next((int(re.search(r'CB2\[(\d+)\]', d)[1]) for d in declarations if d.startswith('dcl_constantbuffer CB2[')), 0))
        if pass_count > 4:
            assert arguments.profile_domain == 'kouku' and sid in (
                '1eb6e82b0befd243ba7ffc9e49b6d067', '42ebb4e66c0c0b4b92db497fcd69ccc2',
                '52f3a078c5510e46a8de35cbed7fda61', 'fb6f0054b2bc094ab3b058418968b930'), ('Unreviewed source pass constants', sid, pass_count)
        lines += [f'    float4 passValues[{pass_count}]; [unroll] for(uint passIndex=0u;passIndex<{pass_count}u;++passIndex) passValues[passIndex]=0.f;',
                  '    passValues[0]=float4(.5f,-.5f,.5f,.5f);']
        if decal or kouku_lit or model:
            lines += ['    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.']
        if pass_count == 5:
            lines += ['    passValues[4]=float4(0.f,0.f,0.f,1.f); // Neutral specular override; only the original secondary MRT consumes it.']
        if pass_count == 7:
            # Original CB2[6].xy is the render target size: the source PS uses
            # it for a pixel checker or width/height aspect ratio.
            lines += ['    uint viewportWidth, viewportHeight; g_EffectSceneDepthTexture.GetDimensions(viewportWidth,viewportHeight);',
                      '    passValues[6]=float4(max(float2(viewportWidth,viewportHeight),1.f),0.f,0.f);']
        dynamic='dynamicparameter' in selection['sourceVF'];subuv='subuv' in selection['sourceVF']
        for sig in p['inputSignature']:
            semantic=sig['semanticName'].lower();index=sig['semanticIndex'];reg=sig['register']
            if semantic=='texcoord':
                values={10:'float4(input.sourceBasisX,'+('input.subUVBlend' if subuv else '0.f')+')',11:'float4(input.sourceBasisZ,input.handedness)',0:('float4(input.uv,input.uv1)' if mesh else 'float4(input.uv,'+('input.uvNext' if subuv else 'float2(0.f,0.f)')+')'),1:'input.color',2:('input.dynamicParameter' if dynamic else 'float4(0.f,0.f,0.f,0.f)'),4:'float4(0.f,0.f,0.f,1.f)',6:'float4(input.tangentView,1.f)',5:'float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW)'}
                if model:
                    values.update({0:'float4(input.uv,0.f,0.f)',4:'float4(0.f,0.f,0.f,1.f)',5:'float4(input.sourceWorldPosition,1.f)',6:'float4(input.tangentView,1.f)',7:'float4(input.tangentUp,0.f)'})
                if decal:
                    values.update({0:'float4(input.uv,input.uv1)',7:'float4(input.tangentUp,0.f)'})
                if kouku_ground:
                    values[5]='float4(input.sourceWorldPosition,1.f)'
                if kouku_ice_distortion or kouku_world_distortion:
                    values[5]='float4(input.sourceWorldPosition,1.f)'
                if kouku_lit:
                    values[7]='float4(input.tangentUp,0.f)'
                    if kouku_lit[2] in ('world','actor'): values[5]='float4(input.sourceWorldPosition,1.f)'
                if kouku_ice:
                    values[5]='float4(input.sourceWorldPosition,1.f)'
                if index not in values:
                    raise ValueError(f'Native TEXCOORD{index} has no {selection["rendererShape"]} carrier adapter; source VS {selection["sourceVS"]}, engine CB0 slots {selection.get("cb0Unowned",[])}.')
                val=values[index]
            elif semantic=='color':val='input.vertexColor' if index==0 else 'float4(0.f,0.f,0.f,0.f)'
            elif semantic=='sv_isfrontface':val='asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u))'
            else:raise ValueError(('unsupported input semantic',sig))
            lines += [f'    float4 v{reg} = {val}; // native {semantic}{index}']
        lines += ['    float4 '+', '.join(f'r{i}=0.f' for i in range(ntemps))+';']
        skip=set();adapt=[];depthPlans={};depthReconstruct={}
        # Device-Z conversion may be scheduled after independent RGB operations.
        # Track the scalar live range instead of assuming six adjacent instructions.
        for sampleIndex, sampleInstruction in enumerate(instructions):
            if not sampleInstruction.startswith('sample'):continue
            sa=ns['args'](sampleInstruction.partition(' ')[2]);tr=int(re.search(r't(\d+)',sa[2])[1])
            if tr in texture_map or sampleIndex+1>=len(instructions):continue
            rawDestination=sa[0]
            if not re.fullmatch(r'r\d+\.[xyzw]',rawDestination):continue
            minimum=instructions[sampleIndex+1]
            if not minimum.startswith('min ') or '0.999000' not in minimum:continue
            ma=ns['args'](minimum.partition(' ')[2])
            if ma[0]!=rawDestination or ma[1]!=rawDestination:continue
            for reconstructionIndex in range(sampleIndex+2,len(instructions)-3):
                block=instructions[reconstructionIndex:reconstructionIndex+4]
                if not (block[0].startswith('mad ') and block[1].startswith('mad ') and block[2].startswith('div ') and block[3].startswith('add ') and 'cb2[1]' in block[0] and 'cb2[1]' in block[1]):continue
                ba=ns['args'](block[0].partition(' ')[2]);bb=ns['args'](block[1].partition(' ')[2])
                if ba[1]!=rawDestination or bb[1]!=rawDestination:continue
                register,lane=rawDestination.split('.')
                for middle in instructions[sampleIndex+2:reconstructionIndex]:
                    da=ns['args'](middle.partition(' ')[2])
                    assert not (da and da[0].split('.')[0]==register and lane in da[0].partition('.')[2]),('depth live range overwritten',sid,middle)
                dst=ns['args'](block[3].partition(' ')[2])[0]
                depthPlans[sampleIndex]=(rawDestination,reconstructionIndex,dst)
                depthReconstruct[reconstructionIndex]=(rawDestination,dst)
                skip.add(sampleIndex+1);skip.update(range(reconstructionIndex+1,reconstructionIndex+4));break
            else:raise ValueError(('unclosed native depth sample',sid,sampleInstruction))
        for i,ins in enumerate(instructions):
            if (model or decal or kouku_lit or kouku_ice) and re.search(r'\bo[1-9]\.',ins):continue # Existing forward carrier consumes RT0; other native MRT writes remain recorded in the source archive.
            if i in skip:continue
            if i in depthReconstruct:
                raw,dst=depthReconstruct[i]
                lines += [f'    // Native {i+1}-{i+4}: reconstructed view depth is supplied by the runtime adapter.',f'    {dst} = {raw};'];continue
            if i in depthPlans:
                raw,reconstructionIndex,dst=depthPlans[i];a=ns['args'](ins.partition(' ')[2])
                lines += [f'    // Native {i+1}: source device depth mapped to centimetre view depth; reconstruction at {reconstructionIndex+1}.','    '+raw+' = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, ('+operand(a[1])+').xy, 0.f).y * 100000.f;']
                adapt.append({'sourceInstructionStart':i+1,'sourceReconstructionStart':reconstructionIndex+1,'kind':'runtime-depth-y-times-100000-to-source-centimetres'});continue
            if ins.startswith('sample'):
                op,_,tail=ins.partition(' ');a=ns['args'](tail);reg=int(re.search(r't(\d+)',a[2])[1])
                if reg not in texture_map:
                    # Source samples device Z, clamps .999, then reconstructs view Z.
                    # Runtime Target_Depth.y already stores clipW / 1000 (metres).
                    block=instructions[i:i+6]
                    if len(block)==6 and block[1].startswith('min ') and block[2].startswith('mad ') and block[3].startswith('mad ') and block[4].startswith('div ') and block[5].startswith('add '):
                        dst=ns['args'](block[5].partition(' ')[2])[0]
                        lines += ['    // Native '+str(i+1)+'-'+str(i+6)+': device-Z reconstruction replaced by Target_Depth.y view-Z adapter.', '    '+dst+' = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, ('+operand(a[1])+').xy, 0.f).y * 100000.f;']
                        skip.update(range(i+1,i+6));adapt.append({'sourceInstructionStart':i+1,'sourceInstructionEnd':i+6,'kind':'runtime-depth-y-times-100000-to-source-centimetres'});continue
                    swizzle=re.search(r't\d+\.([xyzw]+)',a[2])[1]
                    mode='SampleLevel' if 'sample_l' in op else ('SampleBias' if 'sample_b' in op else 'Sample')
                    sample='Read_EffectSceneColor'+mode.removeprefix('Sample')+'(LinearClampUVSampler, ('+operand(a[1])+').xy'+(', ('+operand(a[4])+').x' if len(a)>4 else '')+')'
                    lines += [f'    // {i+1}: {ins} (project resolved HDR SceneColor snapshot adapter)', '    '+result_mask(a[0],sample+'.'+swizzle)]
                    adapt.append({'sourceInstructionStart':i+1,'kind':'resolved-HDR-SceneColor-snapshot-project-adapter'});continue
                index=texture_map[reg];swizzle=re.search(r't\d+\.([xyzw]+)',a[2])[1]
                sample=f'ArtistNativeSample{index}(('+operand(a[1])+').xy, ('+(operand(a[4]) if len(a)>4 else 'float4(0.f,0.f,0.f,0.f)')+').x, '+('true' if 'sample_l' in op else 'false')+')'
                translated=result_mask(a[0],sample+'.'+swizzle)
            elif ins=='loop': translated='[loop] while (true) {'
            elif ins=='endloop': translated='}'
            elif ins.startswith('breakc_nz'): translated='if (('+ns['uint_operand'](ins.split(' ',1)[1])+').x != 0u) break;'
            elif ins.startswith('discard_nz'): translated='if (('+ns['uint_operand'](ins.split(' ',1)[1])+').x != 0u) clip(-1.f);'
            else: translated=original_translate(ins,{},'base')
            translated=re.sub(r'\bo0\b','output',translated)
            assert not re.search(r'\bo[1-9]\b',translated)
            lines += [f'    // {i+1}: {ins}','    '+translated]
        lines+=['    return output;','}']
        program_code += (lines if model else ['#ifndef ARTIST_NATIVE_MODEL_ONLY']+lines+['#endif'])+['']
        row={'program':program,'runtimeShaderProfileId':f'effect.ue3.{arguments.profile_domain}-{program}-native.v1',
             'sourceMaterial':r['sourceMaterial'],'parentMaterial':r['parentMaterial'],'rendererShape':selection['rendererShape'],
             'occurrences':selection['occurrences'],'sourceVF':selection['sourceVF'],'sourceVS':selection['sourceVS'],'sourcePS':sid,'parameters':list(params.values()),'parameterFloat4Rows':vector_slot,'perNodeUnboundDefaults':[{'kind':k[0],'name':k[1],'values':v} for k,v in fixedNodeDefaults.items()],
             'textures':textures,'nativeRT0InstructionCount':last_rt0+1,'depthAdapter':adapt,
             'requiresDepthTextureForViewport':True,'requiresDepthSample':any(a['kind'].startswith('runtime-depth') for a in adapt),'requiresSceneColor':any(a['kind'].startswith('resolved-HDR') for a in adapt),
             'requiresSourceUV1':mesh and any(sig['semanticName'].lower()=='texcoord' and sig['semanticIndex']==0 and re.search(r'\bv'+str(sig['register'])+r'\.[xyzw]*[zw]', '\n'.join(instructions)) for sig in p['inputSignature']),'requiresTangentView':any(sig['semanticName'].lower()=='texcoord' and sig['semanticIndex']==6 and re.search(r'\bv'+str(sig['register'])+r'\.', '\n'.join(instructions)) for sig in p['inputSignature']),
             'nativeBlend':r['parentProperties'].get('blendmode',{}).get('value','blend_opaque'),
             'nativeTwoSided':r['parentProperties'].get('twosided',{}).get('value',False),
             'staticSwitches':r.get('mic',{}).get('staticParameterSet',{}).get('staticSwitchParameters',[])}
        row['modelCue']=model
        rows.append(row)
    except Exception as ex:
        errors.append({"program":program,"sourceMaterial":selection["resolvedMaterial"],"occurrences":selection["occurrences"],"reason":str(ex)})

tail="\n#ifndef ARTIST_NATIVE_MODEL_ONLY\nEFFECT_PS_OUT Shade_EffectArtistNative(uint profile, ARTIST_NATIVE_INPUT input)\n{\n    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;\n    float4 nativeColor=0.f;\n    bool opaqueCoverage=false;\n    switch(profile)\n    {\n"
for r in rows:
    if r['modelCue']:continue
    carrier={'mesh':'MESH','staticMesh':'MESH','screenPost':'SCREEN_POST'}.get(r['rendererShape'],'PARTICLE')
    others=[c for c in ['MESH','PARTICLE','SCREEN_POST'] if c!=carrier]
    tail+='#if '+ ' && '.join('!defined(EFFECT_NATIVE_'+c+'_CARRIER)' for c in others)+'\n'
    tail+=f'    case {r["program"]}u: nativeColor=ArtistNative{r["program"]}(input); opaqueCoverage='+('true' if r['nativeBlend']in ['blend_additive','blend_masked','blend_opaque'] else 'false')+'; break;\n'
    tail+='#endif\n'
tail+='    default: clip(-1.f); return output;\n    }\n    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity, opaqueCoverage ? 1.f : nativeColor.a);\n    output.Distortion=0.f;\n    if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);\n    return output;\n}\n#endif\n'
tail=tail.removesuffix('#endif\n')+'#endif\n'
tail+='\nfloat4 Shade_ArtistModelNative(uint profile, ARTIST_NATIVE_INPUT input)\n{\n    switch(profile)\n    {\n'
for r in rows:
    if r['modelCue']:tail+=f'    case {r["program"]}u: return ArtistNative{r["program"]}(input);\n'
tail+='    default: clip(-1.f); return 0.f;\n    }\n}\n#endif\n'
output_dir = arguments.output_dir.resolve() if arguments.output_dir else OUT
output_dir.mkdir(parents=True, exist_ok=True)
assert not arguments.install_additional_groups or output_dir == OUT
target=output_dir/'Shader_EffectArtistNative.hlsli'
target.write_text(prefix+'\n'+'\n'.join(program_code)+tail,encoding='utf-8')
if arguments.install_additional_groups:
    from native_shader_dispatch import (expand_dispatch_includes, write_if_changed,
                                        write_partitioned_dispatch)
    assert arguments.program_start is not None and arguments.program_start >= 1600
    generated='\n'.join(program_code)
    blocks=re.findall(r'(#ifndef ARTIST_NATIVE_MODEL_ONLY\n// [^\n]+\nfloat4 ArtistNative(\d+)\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}\n#endif)',generated,re.S)
    assert len(blocks)==len(rows), 'Every additional program must have one complete shader body.'
    groups={}
    for block,identifier in blocks:
        groups.setdefault(int(identifier)//64*64,[]).append(block)
    shader_root=ROOT/'Client/Bin/ShaderFiles'
    for group,group_blocks in groups.items():
        write_if_changed(shader_root/f'Shader_EffectArtistNativeGroup{group}.hlsli',
            f'// Single source owner for ArtistNative profiles {group}..{group+63}.\n'+'\n\n'.join(group_blocks)+'\n')
    main_path=shader_root/'Shader_EffectArtistNative.hlsli'
    main=expand_dispatch_includes(main_path.read_text(encoding='utf8'), shader_root)
    begin='// BEGIN ADDITIONAL ARTIST GROUPS\n'; end='// END ADDITIONAL ARTIST GROUPS\n'
    main=re.sub(re.escape(begin)+r'.*?'+re.escape(end),'',main,flags=re.S)
    includes=begin+''.join(f'#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == {group}\n#include "Shader_EffectArtistNativeGroup{group}.hlsli"\n#endif\n' for group in sorted(groups))+end
    marker='\n#ifndef ARTIST_NATIVE_MODEL_ONLY\nEFFECT_PS_OUT Shade_EffectArtistNative'
    assert marker in main
    main=main.replace(marker,'\n'+includes+marker,1)
    begin='// BEGIN ADDITIONAL ARTIST CASES\n'; end='// END ADDITIONAL ARTIST CASES\n'
    main=re.sub(re.escape(begin)+r'.*?'+re.escape(end),'',main,flags=re.S)
    cases=begin
    for row in rows:
        carrier={'mesh':'MESH','staticMesh':'MESH','screenPost':'SCREEN_POST'}.get(row['rendererShape'],'PARTICLE')
        others=[kind for kind in ['MESH','PARTICLE','SCREEN_POST'] if kind!=carrier]
        cases+='#if '+' && '.join('!defined(EFFECT_NATIVE_'+kind+'_CARRIER)' for kind in others)+'\n'
        cases+=f'#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == {row["program"]//64*64}\n'
        cases+=f'    case {row["program"]}u: nativeColor=ArtistNative{row["program"]}(input); opaqueCoverage='+('true' if row['nativeBlend'] in ['blend_additive','blend_masked','blend_opaque'] else 'false')+'; break;\n#endif\n#endif\n'
    cases+=end
    marker='    default: clip(-1.f); return output;'
    assert main.count(marker)==1
    write_partitioned_dispatch(main_path, main.replace(marker,cases+marker,1))
(output_dir/'native_runtime_contract.json').write_text(json.dumps({'programs':rows,'deferredPrograms':errors,'hlsli':str(target)},indent=2),encoding='utf-8')
print('Generated',len(rows),'native material programs, deferred',errors,'lines',len(target.read_text().splitlines()))
