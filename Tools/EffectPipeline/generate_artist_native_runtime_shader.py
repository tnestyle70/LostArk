"""Offline generation of the selected Artist native material programs.

Reuses the earlier instruction translator; no runtime interpreter is added.
Engine depth reconstruction is an explicit centimetre adapter. Original source
expressions, named leaves, sample modes, masks and operation order are retained.
"""
from pathlib import Path
import argparse, json, re, math, hashlib, copy, struct

ROOT = Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description='Generate bounded Artist native HLSL from extracted source shader maps and Resources mappings.')
parser.add_argument('--resource-root', type=Path, default=ROOT/'Client/Bin/Resources',
    help='Validate texture identities against this explicit staged Resources root.')
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
source = json.loads((OUT / 'native_material_inputs.json').read_text(encoding='utf-8-sig'))
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
        # DXBC reads the source once before either destination is written.
        # An aliased sine destination must not change cosine's input.
        parts = ['{ const float4 sourceAngle = ' + operand(a[2]) + ';']
        if a[0] != 'null':
            parts.append(result_mask(a[0], 'sin(sourceAngle)'))
        if a[1] != 'null':
            parts.append(result_mask(a[1], 'cos(sourceAngle)'))
        parts.append('}')
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

asset_map=json.loads((OUT/'texture_asset_map.json').read_text(encoding='utf-8-sig'))
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
// Native2893 lit paper mesh only. Capacity matches CLight_Manager::Replace_SceneLights (16).
uint g_KoukuDoveDirectionalCount = 0u;
float4 g_KoukuDoveDirectionalDirections[16];
float4 g_KoukuDoveDirectionalColors[16];
float4 g_ArtistSourceMacroUV;
float4 g_ArtistSourceWorldToLocal[3];
float4 g_ArtistSourceLocalToWorld[3];
float4 g_ArtistSourceWorldToView[3];

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
    float3 sourceDecalTangent;
    float3 sourceDecalBinormal;
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

KOUKU_DOVE_DIRECT_LIGHT_ADAPTER = '''    // Native2893 also writes diffuse (o3), specular (o5), and two-sided
    // normals (82-99) for deferred lighting. Its forward effect carrier must
    // consume them; RT0 alone contains only ambient/sky and transition glow.
    const float3 doveDiffuse = r1.xyz;
    const float3 doveSpecular = r4.xyz * 10.f; // Inverse native RT5 sqrt/.1 encoding.
    const float dovePower = saturate(source[8].z * .002f) * 500.f;
    const float3 doveTangentNormal = r2.xyz * (input.frontFace ? 1.f : -1.f) * source[0].x;
    const float3 doveBasisX = input.sourceBasisX * rsqrt(max(dot(input.sourceBasisX, input.sourceBasisX), 1e-12f));
    const float3 doveBasisZ = input.sourceBasisZ * rsqrt(max(dot(input.sourceBasisZ, input.sourceBasisZ), 1e-12f));
    const float3 doveBasisY = cross(doveBasisZ, doveBasisX) * input.handedness;
    float3 doveNormal = float3(dot(doveBasisX, doveTangentNormal),
        dot(doveBasisY, doveTangentNormal), dot(doveBasisZ, doveTangentNormal));
    doveNormal *= rsqrt(max(dot(doveNormal, doveNormal), 1e-12f));
    float3 doveView = input.sourceCameraPosition - input.sourceWorldPosition;
    doveView *= rsqrt(max(dot(doveView, doveView), 1e-12f));
    [loop] for (uint lightIndex = 0u; lightIndex < g_KoukuDoveDirectionalCount; ++lightIndex)
    {
        // Directions and normals use the same UE source basis (X,-Z,Y).
        float3 toLight = -g_KoukuDoveDirectionalDirections[lightIndex].xyz;
        toLight *= rsqrt(max(dot(toLight, toLight), 1e-12f));
        float3 halfVector = toLight + doveView;
        halfVector *= rsqrt(max(dot(halfVector, halfVector), 1e-12f));
        const float ndoth = abs(dot(doveNormal, halfVector));
        const float lobe = ndoth < .000001f ? 0.f : min(pow(ndoth, dovePower), 1.f);
        // Existing recovered source-map Lambert/Blinn direct-light contract.
        // No extra ambient, fabricated exposure, shadow, or source SH here.
        output.rgb += g_KoukuDoveDirectionalColors[lightIndex].rgb *
            (doveDiffuse * saturate(dot(doveNormal, toLight)) + clamp(doveSpecular * lobe, 0.f, 2.f));
    }
'''

rows = []
program_code = []
selections=json.loads((arguments.selection_file or OUT/'selected_runtime_material_programs.json').read_text(encoding='utf-8-sig'))['programs']
byid={r['sourceMaterial']:r for r in source['materials']};errors=[]
for ordinal, selection in enumerate(selections):
    program=selection.get('program', arguments.program_start+ordinal if arguments.program_start is not None else (460+ordinal if ordinal<100 else 820+ordinal-100))
    try:
        r=byid[selection['resolvedMaterial']]
        source_motion_blur = selection['sourcePS'] == 'b9fc10ac51695c41b71ae1807fe6a47d'
        reviewed_sd_pair = {
            'fx_mastermaterial.fx_mi.fx_mm_onelayerdistortion_02_01_ad':
                ('5825675b4ffbc840ad691ec56973cf7e', 'eb2bcd5c8f3c6c49805ab689687b14f6'),
            'fx_m_mi_j_00.fx_mi.fx_j_pa_hologram_01_01_tr':
                ('c7afe261ee6b2342b612cebb098b1933', 'e0b5fe98a973904985c18256b8eb8f8b'),
        }.get(r['sourceMaterial'])
        if reviewed_sd_pair:
            # Reuse SDNative374/375's recovered engine ABI, not a material-name
            # fallback for other source permutations. Uniforms still come from
            # this source map and this MIC's exact current values.
            assert (selection['sourceVS'], selection['sourcePS']) == reviewed_sd_pair
            assert selection['rendererShape'] == 'sprite'

        kouku_ice = (arguments.profile_domain == 'kouku' and
            selection['sourceVS'] == '5298fd1cc3a2f64dab8401a32a6bef3c' and
            selection['sourcePS'] == '97220ed990d76147b9d9778b693101c0')
        if not kouku_ice and selection['sourceVS'] in ('5298fd1cc3a2f64dab8401a32a6bef3c','792dc3606e73a443b8013c7adc2b97d8','cae853451adaf845b2558adb48edc97d'):
            errors.append({'program':program,'material':r['sourceMaterial'],'reason':'Distinct non-unlit engine CB prefix, world-position varying; separately restore exact inputs.'});continue
        name=r['sourceMaterial'].rsplit('.',1)[-1]
        sid=selection['sourcePS']
        p=json.loads((OUT/'full_programs'/(sid+'.json')).read_text(encoding='utf-8-sig'))
        uniform = copy.deepcopy(r['materialMap']['uniformExpressionSet'])
        for node in walk(uniform):
            if node.get('parameterNameNumber',0):
                node['nativeParameterFName']={'name':node['parameterName'],'number':node['parameterNameNumber']}
                node['parameterName']=node['parameterName']+'_'+str(node['parameterNameNumber']-1)
                node['parameterNameNumber']=0
        bindings = p['bindings']
        if source_motion_blur:
            assert arguments.profile_domain == 'kouku' and selection['rendererShape'] == 'screenPost'
            assert selection['sourceVF'] == 'fparticledynamicparametervertexfactory'
            assert selection['sourceVS'] == '5825675b4ffbc840ad691ec56973cf7e'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == list(range(7))
            # FMaterialShaderParameters serializes its three uniform arrays,
            # then LocalToWorld, WorldToLocal and WorldToView FShaderParameter
            # records (base byte, byte extent, CB index). Cross-checked against
            # the previously recovered spider WorldToLocal record at +6.
            wire = (OUT/'shader_objects'/(sid+'.bin')).read_bytes()
            at = bindings['bindingArraysOffsetInShaderObject'] + bindings['bindingArraysByteSize']
            assert struct.unpack_from('<HHH',wire,at) == (16,44,0)
            assert struct.unpack_from('<H',wire,at+8)[0] == 0
            assert struct.unpack_from('<HHH',wire,at+12) == (64,44,0)
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
        action_post_opacity = (arguments.profile_domain == 'kouku' and sid in (
            '1000024695759244a7297e6a1c0a53e1',
            '599a6ad60ea6494a83850077e6cd3356', '5a405d04c4e0fa42a1342f8da1615ca0'))
        if action_post_opacity:
            assert selection['rendererShape'] == 'screenPost'
            assert selection['sourceVF'] == 'flocalvertexfactory'
            assert selection['sourceVS'] == '8562847977cf324b900feff85799f43a'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0]
            # The action SkillValue owns this engine input separately from the
            # original material's Opacity parameter. Existing material tracks
            # sample its envelope without changing any material expression.
            assert ('scalarparameter', 'source_effect_opacity') not in params
            params[('scalarparameter', 'source_effect_opacity')] = {
                'kind': 'scalar', 'name': 'source_effect_opacity', 'default': 1.0}
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
            if t == 'frac': return 'frac('+expression(x['input'])+')'
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
                 'exists':(arguments.resource_root/asset(x['sourceObjectPath'])).is_file()})
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
            # SCENE03A white_t LocalVF: TEXCOORD7 is the source tangent-up
            # vector; CB0[5..7] owns sky/ambient and CB0[0].x owns opacity.
            '286c952473acd34a8cdd4e981db6ec1f': ('76933afe48b6a34380ab05bdd2a93dc1', 5, 'opacity', [0, 4, 5, 6, 7]),
            # Exact EngineMaterials.DefaultMaterial LocalVF: CB0[0] is the
            # bound selection color, [2..4] are sky/ambient. CB0[1] only
            # reaches the archived secondary material MRT, outside RT0.
            '2837f9c4eed1a745b242b4abb6f57be1': ('0c1413bd3ee54d449ce7fdac8c7f1542', 2, 'none', [1, 2, 3, 4]),
            # SCENE03A's attached curtain uses source vertex alpha for normal
            # strength, TEXCOORD6 view, TEXCOORD7 up and sky rows [10..12].
            '5add713d06b8684d84689591b7ef144f': ('0c1413bd3ee54d449ce7fdac8c7f1542', 10, 'none', [10, 11, 12]),
            '3a96e00bdfda46489bb6aa32ae1ac89c': ('0c1413bd3ee54d449ce7fdac8c7f1542', 6, 'color', [0, 5, 6, 7, 8]),
            '8c7feae3b54e7a46835555bfa86e7e6e': ('239396ffe9f57b47a19ee2955207d2e8', 26, 'actor', [0, 1, 2, 26, 27, 28]),
            # Valtan crack: absolute world XY plus opacity W in row 0,
            # mesh particle color in row 1 and tangent-up sky rows 14..16.
            'a80ec5aa8c23c54bbe0ed08528804b78': ('f4505ea22bef0b4895dbadfe1d6fc15c', 14, 'world_color', [0, 1, 14, 15, 16]),
            # Valtan heat mesh: particle color/opacity, tangent-up sky 7..9.
            # Row 6 and CB2[4] feed only the archived secondary MRT.
            '63b3600fc7b02a43999ea2a77bb81937': ('7266a7a50d72d14db4f432d74ed5818d', 7, 'opacity', [0, 1, 6, 7, 8, 9]),
            # Bern smoke/leaf RT0 consumes the engine opacity in Y. X is
            # the secondary normal MRT flag, not the visible opacity.
            '30a8a4c7070a6043bcc86e56fda2d34e': ('b203af051145314e8d66b49a6dc552cb', 3, 'opacity_y', [0, 3, 4, 5]),
            'e9352e2d45f99a47b54ef208b7a3ce2f': ('7266a7a50d72d14db4f432d74ed5818d', 5, 'opacity_y', [0, 1, 5, 6, 7]),
            # Valtan destruction's masked monster section uses the original
            # actor/world-position prefix and tangent-space sky at rows 25..27.
            '9ae9112a9f0431448e28da5cf85ef088': ('239396ffe9f57b47a19ee2955207d2e8', 25, 'actor', [0, 1, 2, 25, 26, 27]),
            'a60e035ff9e41d4099cdebe220d8e302': ('70a7b0749eb5904898747857eecc9da2', 4, 'opacity', [0, 4, 5, 6]),
            # Guardian ALT_V snow-rock StaticMesh: source LocalVF tangent-up and sky rows 7..9.
            'e5fe14836f42f94aad7d43d0499acdb7': ('0c1413bd3ee54d449ce7fdac8c7f1542', 7, 'none', [7, 8, 9]),
            # Ninave Esther arrow body (fx_l_me_transition_05_8_ma, masked LocalVF): row 0 is
            # particle RGBA (W is the mask threshold, XYZ tints the fresnel), TEXCOORD6 view,
            # TEXCOORD7 up, sky rows 7..9; row 6.z only feeds the archived secondary MRT.
            '77a224a19f03dd4683b192b1134b4832': ('0c1413bd3ee54d449ce7fdac8c7f1542', 7, 'color', [0, 6, 7, 8, 9]),
        }.get(sid) if arguments.profile_domain=='kouku' and not model else None
        # Valtan phase-two dust uses the same source tangent-up/sky carrier
        # with dynamic-parameter and sub-UV sprite VS permutations. Their
        # o7 is (tangentX.z,tangentY.z,normal.z), o4 is the dynamic parameter,
        # and o8 is clip position; qualify the complete VS/PS pair so the
        # existing LocalVF use of 21310... remains unchanged.
        if arguments.profile_domain == 'kouku' and not model:
            source_pair = (sid, selection['sourceVS'])
            kouku_lit = {
                ('21310dd53f047b4c9c6d215723e659ae', 'cf097fb30a0d434facb4ded0faee0083'):
                    ('cf097fb30a0d434facb4ded0faee0083', 7, 'opacity', [0, 7, 8, 9]),
                ('9e4781052a66534b9646434db8d482db', 'ea196d5080bed8449612de3b1bfc28c3'):
                    ('ea196d5080bed8449612de3b1bfc28c3', 3, 'opacity', [0, 3, 4, 5]),
                ('e386f4f096d15843a75f1c661c5d9d9d', 'cf097fb30a0d434facb4ded0faee0083'):
                    ('cf097fb30a0d434facb4ded0faee0083', 7, 'opacity', [0, 7, 8, 9]),
            }.get(source_pair, kouku_lit)
        if kouku_lit:
            assert selection['sourceVS'] == kouku_lit[0], ('Kouku lit vertex shader mismatch', selection['sourceVS'], kouku_lit[0])
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == kouku_lit[3], ('Kouku lit engine rows mismatch', bindings['constantBufferClosure']['unownedConstantBuffer0Slots'], kouku_lit[3])
        if sid == 'e5fe14836f42f94aad7d43d0499acdb7':
            assert mesh and selection['sourceVF'] == 'flocalvertexfactory'
            assert p['disassembly']['instructionSha256'] == '5ece5a292f333a290266bebdc61c7c1abda019271de855a285603749fd4359ef'
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
            '482aad52a559da4aa7e3dbfedbe9c4b9': ('772e94581a5e6548b9529bc7cc103bca', None, [0, 1, 2]),
            '0dc937f93ccdd2479f044bde8d2d07d4': ('5d79421dc8571c45aa49790f50274f51', 17, [0, 1, 2, 15, 16, 17, 18, 19]),
            'aafa1b2d458b5746b5f1db2d070d99a4': ('5d79421dc8571c45aa49790f50274f51', 15, [0, 1, 2, 13, 14, 15, 16, 17]),
            '0dce6ac02eeca942b3fa542f1b46a2f7': ('5d79421dc8571c45aa49790f50274f51', 15, [0, 1, 2, 13, 14, 15, 16, 17]),
            '26fddc3c63c71642b5dbb20d55a3e61c': ('772e94581a5e6548b9529bc7cc103bca', None, [0, 1, 2]),
            'db50bb574893fd46bb16839799006a50': ('772e94581a5e6548b9529bc7cc103bca', None, [0, 1, 2]),
            # Valtan entrance cast-end: original LocalDecal projection/opacity
            # prefix, with the same tangent-up sky expression at rows 12..14.
            # Rows 10/11 are declared engine suffix padding, unused by this PS.
            'b6cb82f89e22c24ebcdfdca8be587e9c': ('5d79421dc8571c45aa49790f50274f51', 12, [0, 1, 2, 10, 11, 12, 13, 14]),
            # Guardian exact LocalDecal pairs: projection/opacity and native sky suffix.
            '68bdce842c4e27469535a3c35ed0178f': ('5d79421dc8571c45aa49790f50274f51', 16, [0, 1, 2, 14, 15, 16, 17, 18]),
            '84ec8bb0a0305b4cb321d09f45bacb1c': ('5d79421dc8571c45aa49790f50274f51', 18, [0, 1, 2, 16, 17, 18, 19, 20]),
            'fd3e09c786366d44bad4a65ad9235917': ('5d79421dc8571c45aa49790f50274f51', 8, [0, 1, 2, 6, 7, 8, 9, 10]),
            '1b2f0475be98b446b3bbd35d354270e5': ('5d79421dc8571c45aa49790f50274f51', 12, [0, 1, 2, 10, 11, 12, 13, 14]),
            'df0f7ee6f8887648a9eb10a9ee209133': ('5d79421dc8571c45aa49790f50274f51', 15, [0, 1, 2, 13, 14, 15, 16, 17]),
            'da323567908fb948a423cf24a1eaae89': ('5d79421dc8571c45aa49790f50274f51', 9, [0, 1, 2, 7, 8, 9, 10, 11]),
            '015fa6f730aec44fb0479a94935919a7': ('5d79421dc8571c45aa49790f50274f51', 16, [0, 1, 2, 14, 15, 16, 17, 18]),
            '4c80ea0c0cb28e41b7cc44920bd162a5': ('5d79421dc8571c45aa49790f50274f51', 12, [0, 1, 2, 10, 11, 12, 13, 14]),
            # Guardian Dragon Resonance: exact LocalDecal projection and sky rows.
            'ccffbc09263fbe46887a361b9ea901f5': ('5d79421dc8571c45aa49790f50274f51', 13, [0, 1, 2, 11, 12, 13, 14, 15]),
            '3e2151eaa9254c408a93249c1647aaa6': ('5d79421dc8571c45aa49790f50274f51', 20, [0, 1, 2, 18, 19, 20, 21, 22]),
            # Guardian ALT V fixed-area impact and DragonDecal source permutations.
            'b319111a4ef50d40a6b4b960d6c835c7': ('5d79421dc8571c45aa49790f50274f51', 16, [0, 1, 2, 14, 15, 16, 17, 18]),
            '225313f3418d1644afd364c8fc464a7d': ('772e94581a5e6548b9529bc7cc103bca', None, [0, 1, 2]),
            # Specialist selection pond/butterfly decals: original clip-depth,
            # RGBA/opacity prefix; butterfly's sky expression is rows 19..21.
            # Rows 17/18 are declared suffix padding unused by its native PS.
            '91fd29777c6998448300eb5d573dd525': ('772e94581a5e6548b9529bc7cc103bca', None, [0, 1, 2]),
            'ef68ae7aec8f94458ef2cbb3c6bafd2d': ('5d79421dc8571c45aa49790f50274f51', 19, [0, 1, 2, 17, 18, 19, 20, 21]),
            # Warrior selection arcanebolter decal: same original projection
            # prefix, sky rows 13..15, unused declared suffix padding 11/12.
            '57983e11986fc848b45046b7922de2b2': ('5d79421dc8571c45aa49790f50274f51', 13, [0, 1, 2, 11, 12, 13, 14, 15]),
            # Circus-ball impact: LocalDecal receiver normal plus projected tangent/binormal.
            '70820928f0b4b14486394a81b0f88c89': ('772e94581a5e6548b9529bc7cc103bca', None, [0, 1, 2, 8, 9]),
        }.get(sid) if decal else None
        if kouku_decal:
            assert selection['sourceVS'] == kouku_decal[0]
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == kouku_decal[2]
        kouku_ground = decal and sid in (
            'ef9cad5cf42011438c3b2ff2ecf7baee',
            'b123a95ca0a96e488268a58a2998fa43',
            '76df7d394e7b4242b700d26cf69db77b',
            # GroundEffect 2113 rectangle retains the same LocalDecal
            # VS, world-position input and engine-owned CB0[0..3] prefix.
            '2dd9378b07e40a4893bb599b57c0d913',
            # Valtan GroundEffect 2002 circle has the same VS and prefix.
            # PS register v7 is TEXCOORD5 (absolute source world XY),
            # not TEXCOORD7; retain its radial mask and world-noise samples.
            '6b8e8fca5028ea449cbe6a1d5aebb3c6',
            # GroundEffect 2001 fan uses the same original projection ABI;
            # its radial/angular material expressions remain native.
            'c390f9f83fce2b4791421e47db4cdcc6')
        if kouku_ground:
            # GroundEffect's LocalDecal VS exports absolute world position at
            # TEXCOORD5. Its PS adds a pre-view translation before the existing
            # projection/color/opacity prefix, unlike Cascade LocalDecal.
            assert selection['sourceVS'] == '51afa7d015c2db45bc7c7faf7300c9c3'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0, 1, 2, 3]
        if decal and not kouku_decal and not kouku_ground and sid not in ('be9bb8ea52a06b40bc25b550e349b5b9','316b66ee3867964da197becf270077f0','aacf33d926f3884493fb98d76d43506c','92378d29e44d7046b15b6af899336298','cd75326f74ef024d827113811196cae2'):
            raise ValueError(('Unreviewed source decal prefix',sid))
        # These masked LocalVF permutations pack mesh particle color in row 0,
        # including opacity in W. The translucent X prefix zeros their mask.
        # Qualify the native ABI, not an effect name or allocated profile ID.
        masked_color_vertex_shader = {
            '3c21c37113be6c4789b61eb3f68c3e7d': 'd17daa101dec2b4493fce2f510407f32',
            '7b434a175bf676488b2d4d9c19e752fa': 'd17daa101dec2b4493fce2f510407f32',
            'e4a5f3955ef7e64d8f78f5e46deba267': '7025758b7227e342a3118ce0b01b81b2',
            '3b3eb9ea0d75844688479bb8f82cf8fc': 'd17daa101dec2b4493fce2f510407f32',
            'cb42fe675cadf94aac44e861c822207d': 'd17daa101dec2b4493fce2f510407f32',
            '496aee5a3382744d97f9dcdb89b6ae6c': 'd17daa101dec2b4493fce2f510407f32',
            '4ab03586b8ab06498e9d56b87dce4fee': 'd17daa101dec2b4493fce2f510407f32',
            '653bac92bfa279408665fa859b7765f6': 'd17daa101dec2b4493fce2f510407f32',
            'b0aebbf53cbd8249b28cd0b8fc7da17c': 'd17daa101dec2b4493fce2f510407f32',
            # Valtan cracked stone: mask alpha and final RGB both use CB0[0].
            '2232b9d7f30bd146afe89b9fc661279e': 'd17daa101dec2b4493fce2f510407f32',
        }.get(sid) if arguments.profile_domain == 'kouku' else None
        # Artist flowergarden's masked LocalVF uses the same row-0 RGBA ABI.
        if arguments.profile_domain == 'artist' and sid == '390b1fe8a7081c45bf96c8afc4bf11e9':
            masked_color_vertex_shader = 'd17daa101dec2b4493fce2f510407f32'
        opacity_prefix = '    source[0].x=1.f; // Project engine opacity multiplier.'
        if action_post_opacity:
            parameter = params[('scalarparameter', 'source_effect_opacity')]
            opacity_prefix = f'    source[0].x=g_ArtistSourceMaterialParameters[{parameter["row"]}].{"xyzw"[parameter["lane"]]}; // Source action SkillValue opacity.'
        if masked_color_vertex_shader:
            assert mesh and selection['sourceVF'] == 'flocalvertexfactory'
            assert selection['sourceVS'] == masked_color_vertex_shader, 'Masked particle color vertex shader mismatch'
            assert r['parentProperties']['blendmode']['value'] == 'blend_masked'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0]
            assert 'w' in ''.join(re.findall(r'cb0\[0\]\.([xyzw]+)', '\n'.join(instructions)))
            opacity_prefix = '    source[0]=input.color; // Native masked LocalVF binds particle RGB and opacity in row 0.'
        # These source fire programs consume absolute position at TEXCOORD5,
        # not projected position. The world-offset variant also consumes the
        # same emitter inverse already used by the native spider material.
        kouku_fire_world = {
            '2cce8741b31d5049ab2b10e26293372d': (
                ('c7afe261ee6b2342b612cebb098b1933', '1933037c8b482b47b9c7f17238734ad2'),
                [0, 1, 2, 3]),
            '05daa4c5d7a8e44bbd81c654399ddd09': (
                ('1933037c8b482b47b9c7f17238734ad2',), [0]),
        }.get(sid) if arguments.profile_domain == 'kouku' else None
        if kouku_fire_world:
            assert selection['rendererShape'] == 'sprite'
            assert selection['sourceVS'] in kouku_fire_world[0]
            assert r['parentProperties']['blendmode']['value'] == 'blend_translucent'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == kouku_fire_world[1]
            assert all(r['materialMap']['uniformExpressionCounts'][name] == 0 for name in (
                'vertexVectorExpressions', 'vertexScalarExpressions', 'vertexTexture2DExpressions'))
            vertex = json.loads((OUT/'full_programs'/(selection['sourceVS']+'.json')).read_text(encoding='utf-8-sig'))
            assert not vertex['disassembly']['sampleInstructions']
            assert 'mov o7.xyzw, r3.xyzw' in vertex['disassembly']['instructions']
            assert 'mul o0.w, r0.x, cb0[0].w' in instructions
            opacity_prefix = '    source[0]=float4(0.f,0.f,0.f,1.f); // Absolute source world position and neutral engine opacity.'
        source_cb_count = bindings["constantBufferClosure"]["declaredConstantBuffer0Float4Count"]
        lines=[f'// {name}: {sid}; selected map {r["mapKey"]}.',
               f'float4 ArtistNative{program}(ARTIST_NATIVE_INPUT input)', '{',
               f'    float4 source[{max(3, source_cb_count)}]; [unroll] for (uint i=0u; i<{max(3, source_cb_count)}u; ++i) source[i]=0.f;',
               opacity_prefix,
               '    float4 output=0.f;']
        if source_motion_blur:
            lines += ['    // Original local forward projected into the active source camera basis.',
                      '    [unroll] for(uint row=0u;row<3u;++row) { source[1u+row]=g_ArtistSourceLocalToWorld[row]; source[4u+row]=g_ArtistSourceWorldToView[row]; }']
        if kouku_fire_world and len(kouku_fire_world[1]) == 4:
            lines += ['    source[1]=g_ArtistSourceWorldToLocal[0];',
                      '    source[2]=g_ArtistSourceWorldToLocal[1];',
                      '    source[3]=g_ArtistSourceWorldToLocal[2];']
        if model and sid == '5f33bef7c823444d8983ab12adf5b7bb':
            # Terpeion wing ghost skin: engine rows are CB0[0].x opacity and
            # CB0[22..25] (lighting channel, sky upper/lower, ambient+intensity).
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0, 22, 23, 24, 25]
            lines += ['    source[0]=float4(input.color.a,0.f,0.f,0.f);',
                      '    source[23]=float4(input.skyUpperColor,0.f);', '    source[24]=float4(input.skyLowerColor,0.f);',
                      '    source[25]=float4(input.ambientColor,input.skyIntensity);']
        elif model:
            lines += ['    // Existing scene adapter: source world origin is absolute; camera is converted to source cm.', '    source[0]=0.f;', '    source[1]=float4(input.sourceCameraPosition,1.f);', '    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];']
            if 'tig_00' in r['sourceMaterial']:
                lines += ['    source[2]=float4(input.sourceActorPosition,0.f); // Native actor-position seed for the emissive pulse.', '    source[3].x=input.color.a;']
                sky=28
            elif arguments.profile_domain == 'kouku':
                # Skinned kouku-domain skins (vehicle PlaySkeletalMesh): engine rows are
                # the world prefix [0,1] and the three trailing sky rows of this PS.
                unowned = bindings['constantBufferClosure']['unownedConstantBuffer0Slots']
                sky = unowned[2] if len(unowned) == 5 else None
                assert sky is not None and unowned == [0, 1, sky, sky + 1, sky + 2], ('Skinned engine rows', sid, unowned)
                lines += ['    source[1].w=input.color.a;']
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
        kouku_world_distortion = arguments.profile_domain == 'kouku' and sid in (
            '2d8c822c88934149bfa9587fd772c1e8', '6ee267a9e874c343a3f8ce5f7a733a2a')
        if kouku_world_distortion:
            reviewed_vertices = (
                '48f2462ce60a75419377f4ac51e71569',
                '959f42f324748b4386aab65adea222f9',
                'f560771685f7254fb3e269da6253d5d9') if sid == '2d8c822c88934149bfa9587fd772c1e8' else ('140465382862754d99c15d13bf230261',)
            assert selection['sourceVS'] in reviewed_vertices, ('Unreviewed world-position distortion VS', selection['sourceVS'])
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == ([] if sid == '2d8c822c88934149bfa9587fd772c1e8' else [0])
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
            elif prefix_kind=='opacity_y':
                assert any(re.search(r'\bo0\.w,.*cb0\[0\]\.y', line) for line in instructions)
                lines += ['    source[0]=float4(0.f,1.f,0.f,0.f); // Native RT0 external opacity is row 0 Y.']
            elif prefix_kind=='world_color':
                vertex = json.loads((OUT/'full_programs'/(selection['sourceVS']+'.json')).read_text(encoding='utf-8-sig'))
                assert 'mov o8.xyzw, r0.xyzw' in vertex['disassembly']['instructions']
                assert 'mul o0.w, r0.x, cb0[0].w' in instructions
                lines += ['    source[0]=float4(0.f,0.f,0.f,1.f); // Absolute world origin and neutral external opacity.',
                          '    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];']
            lines += [f'    source[{sky}]=float4(input.skyUpperColor,0.f);', f'    source[{sky+1}]=float4(input.skyLowerColor,0.f);', f'    source[{sky+2}]=float4(input.ambientColor,input.skyIntensity);']
        guardian_world_position = sid in (
            '6c98ffeb71f43947910f9d868cbb2b55', 'c33eb51395d71c4b804cf843eee2a488',
            'e0b5fe98a973904985c18256b8eb8f8b')
        guardian_world_to_local = sid in (
            '34ab808091791e4ea33d0193d9a46f79', 'c33eb51395d71c4b804cf843eee2a488')
        if guardian_world_position:
            assert selection['rendererShape'] == 'sprite'
            assert selection['sourceVS'] in ('c7afe261ee6b2342b612cebb098b1933', '1933037c8b482b47b9c7f17238734ad2')
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == ([0,1,2,3] if guardian_world_to_local else [0])
            lines += ['    source[0]=float4(0.f,0.f,0.f,1.f); // Absolute source world origin and original opacity W.']
        if guardian_world_to_local:
            assert selection['rendererShape'] == 'sprite'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0,1,2,3]
            lines += ['    [unroll] for(uint row=0u;row<3u;++row) source[1u+row]=g_ArtistSourceWorldToLocal[row];']
        if sid == 'eb2bcd5c8f3c6c49805ab689687b14f6':
            assert reviewed_sd_pair and bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0, 10]
            lines += ['    source[10].x=0.f; // Existing SDNative374 project neutral scene attenuation; NOT a recovered source default.',
                      '    // Zero distortion/emission must preserve SceneColor: Scene*(1-x)=Scene requires x=0.']
        if sid == '9aa5e61191a9654290657484e8c9cae6':
            assert mesh and selection['sourceVS'] == 'e520045fc771e74d9a67d28b8621c46c'
            assert bindings['bindingSemanticSha256'] == 'b1c5843f0ae303f802f881a8db7cadfc0f9f4f07209bcd93ceedade1e3a7589c'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0]
            lines += ['    source[0]=input.dynamicParameter; // Original local-mesh distortion dynamic parameter prefix.']
        if sid == 'eaac28d26f446743a4f6e0a035d6cfb6':
            assert selection['rendererShape'] == 'sprite' and selection['sourceVS'] == '5825675b4ffbc840ad691ec56973cf7e'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0,1,2]
            lines += ['    source[0]=0.f; // Absolute origin: no pre-view translation.',
                      '    source[1]=float4(input.sourceCameraPosition,1.f);',
                      '    source[2].x=1.f; // Native external opacity follows the camera rows.']
        macro_uv_vertex = {
            '84810d1df112094a8330255c42fb5870': '2dd6d96a7e6c974fac82106409a5b9b8',
            '1c52877bf8998d4fa2659be93d5a9995': '5825675b4ffbc840ad691ec56973cf7e',
        }.get(sid)
        if macro_uv_vertex:
            assert selection['rendererShape'] == 'sprite' and selection['sourceVS'] == macro_uv_vertex
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0,1,2]
            if sid == '1c52877bf8998d4fa2659be93d5a9995':
                assert selection['sourceVF'] == 'fparticledynamicparametervertexfactory'
                assert r['sourceMaterial'] == 'fx_m_mi_00.fx_mi.fx_c_pa_fi_02_1_tr'
                assert [line for line in instructions if 'cb0[2]' in line] == ['mul o0.w, r0.x, cb0[2].x']
                assert 'add r0.yz, r0.yyzy, -cb0[0].xxyx' in instructions
                assert 'mad r1.xy, r0.yzyy, cb0[1].zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)' in instructions
            lines += ['    source[0]=float4(g_ArtistSourceMacroUV.xy,0.f,0.f);',
                      '    source[1]=float4(0.f,0.f,g_ArtistSourceMacroUV.zw);',
                      '    source[2].x=1.f; // Native ParticleMacroUV external opacity.']
        guardian_masked_color = sid in (
            '2077f21c99ab524387c5f96cadfede88', 'bc2b789f66273341bdcb9da418d00a0a',
            '2d85fecf0b7dd9478dc9dad608124fba', '24b9fab146df71489895883c8b8fadca',
            '3b18265f1649424d9c83b0a824f4033b', '75790cbcee5d864d9ec925c6158e33d7',
            '3fdc451142ff60449bcad09ea75935ed')
        if guardian_masked_color:
            # These exact masked LocalVF programs have no opacity-prefix row.
            # CB0[0] is MeshEmitterVertexColor (RGB tint, W mask coverage),
            # whereas CB0[1] already belongs to SelectionColor. Supplying the
            # translucent prefix leaves W zero and discards every shield/rock.
            assert mesh and selection['sourceVF'] == 'flocalvertexfactory'
            assert selection['sourceVS'] == 'd17daa101dec2b4493fce2f510407f32'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0]
            assert r['parentProperties']['blendmode']['value'] == 'blend_masked'
            assert any('cb0[0].w' in instruction for instruction in instructions)
            lines += ['    source[0]=input.color; // Native masked mesh particle RGBA prefix.']
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
        if sid == '70820928f0b4b14486394a81b0f88c89':
            assert p['disassembly']['instructionSha256'] == 'b3fbebdb27b75593b006e0a62b8fee469386b37e3a1349b19ab39137ce7b4d34'
            lines += ['    source[8]=float4(-input.sourceDecalTangent,0.f);',
                      '    source[9]=float4(-input.sourceDecalBinormal,0.f);']
        for b in bindings['vectors']:
            exp=uniform['pixelVectorExpressions'][b['expressionIndexOrGroup']]
            lines += [f'    source[{b["baseIndex"]//16}] = {expression(exp)};']
        for b in bindings['scalarGroups']:
            for lane,exp in enumerate(uniform['pixelScalarExpressions'][b['expressionIndexOrGroup']*4:b['expressionIndexOrGroup']*4+4]):
                lines += [f'    source[{b["baseIndex"]//16}].'+ 'xyzw'[lane]+f' = ({expression(exp)}).x;']
        # The super-action cylinder samples screen coordinates rather than mesh UVs.
        # Its exact LocalVF exports clip position, and CB2[6].xy supplies viewport size.
        screen_space_mesh = sid == '591444cc4fff1f449dd154b804f25bb8'
        if screen_space_mesh:
            assert mesh and selection['sourceVF'] == 'flocalvertexfactory'
            assert selection['sourceVS'] == '70ef36e78c7b254f96b485b73b366538'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0, 1]
            assert 'div r0.x, cb2[6].x, cb2[6].y' in instructions
            vertex = json.loads((OUT/'full_programs'/(selection['sourceVS']+'.json')).read_text(encoding='utf-8-sig'))
            assert 'mov o6.xyzw, r4.xyzw' in vertex['disassembly']['instructions']
            assert not vertex['disassembly']['sampleInstructions']
            assert all(r['materialMap']['uniformExpressionCounts'][name] == 0 for name in (
                'vertexVectorExpressions', 'vertexScalarExpressions', 'vertexTexture2DExpressions'))
        kouku_glass_post = (arguments.profile_domain == 'kouku' and
            sid == 'b704e932ad66a6419217a18d5be491e8')
        if kouku_glass_post:
            # SCENE07A's EFInterpTrackPostRenderMaterial: LocalVF UV0 is the
            # full-screen quad UV. CB2[6].xy supplies viewport aspect ratio;
            # TEXCOORD4 is neutral fog (0,0,0,1). Only RT0 is composited.
            assert selection['rendererShape'] == 'screenPost'
            assert selection['sourceVF'] == 'flocalvertexfactory'
            assert selection['sourceVS'] == '8562847977cf324b900feff85799f43a'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0]
            assert 'div r0.x, cb2[6].x, cb2[6].y' in instructions
            assert 'mul o0.w, r0.w, cb0[0].x' in instructions
        action_post_rt0 = (arguments.profile_domain == 'kouku' and sid in (
            '599a6ad60ea6494a83850077e6cd3356', '5a405d04c4e0fa42a1342f8da1615ca0'))
        if action_post_rt0:
            # Typed action material effect on the same full-screen LocalVF.
            # Only RT0 is composited; preserve auxiliary native MRTs in archive.
            assert selection['rendererShape'] == 'screenPost'
            assert selection['sourceVF'] == 'flocalvertexfactory'
            assert selection['sourceVS'] == '8562847977cf324b900feff85799f43a'
            assert bindings['constantBufferClosure']['unownedConstantBuffer0Slots'] == [0]
        pass_count = max(4, next((int(re.search(r'CB2\[(\d+)\]', d)[1]) for d in declarations if d.startswith('dcl_constantbuffer CB2[')), 0))
        if pass_count > 4:
            assert kouku_glass_post or screen_space_mesh or (arguments.profile_domain == 'kouku' and sid in (
                '1eb6e82b0befd243ba7ffc9e49b6d067', '42ebb4e66c0c0b4b92db497fcd69ccc2',
                '286c952473acd34a8cdd4e981db6ec1f',
                '2837f9c4eed1a745b242b4abb6f57be1',
                '63b3600fc7b02a43999ea2a77bb81937',
                '52f3a078c5510e46a8de35cbed7fda61', 'fb6f0054b2bc094ab3b058418968b930',
                # Terpeion wing ghost skin: CB2[3]/CB2[4] diffuse/specular overrides only.
                '5f33bef7c823444d8983ab12adf5b7bb',
                # Fighter earth-impact opaque fragment: row 4 feeds only o5
                # specular MRT after the final RT0 write; row 3 is the existing
                # neutral diffuse override. Exact LocalVF/prefix above applies.
                '3a96e00bdfda46489bb6aa32ae1ac89c',
                # Ninave Esther arrow mesh fx_l_me_transition_05_8_ma: CB2[3]/CB2[4]
                # are colour scale/offset (mad) overrides only, same shape as the wing.
                '77a224a19f03dd4683b192b1134b4832')), ('Unreviewed source pass constants', sid, pass_count)
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
        # Reviewed beam/trail VSs pass all four UV components through.
        # Preserve both pairs; the carrier adapter selects finite coverage and
        # distance-detail coordinates for each recovered pixel-program consumer.
        # WaterRibbon zw includes a finite end taper, not a transverse-edge proof.
        ribbon_uv1_vs = {
            '59a22eeec5a51f439595f929dddfe8bf': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            'e924ddbcfb7336408af5883ef3ddbf89': ('91ccb94877dac34e988dd1d7bf625e2c', 'mov o1.xyzw, v3.xyzw'),
            '4d739536c182294da40111bf6ba66fd1': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            '34c7dac3b50dcd40b8c18be67bbf7d5a': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            '602579e9d74c2744acfda3bca9b987bb': ('91ccb94877dac34e988dd1d7bf625e2c', 'mov o1.xyzw, v3.xyzw'),
            # Guardian trail/ribbon color and distortion share the reviewed four-lane VS ABI.
            '4fadeb6d325abb4da6ee7cb2cc48f1ea': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            'bdfaa809f0359f4195f9316fea2a4af2': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            'e827bcec62f20a4bac513dd9107aafc6': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            'd1e3e2d34da42d448045af7b08e27e15': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            '61d43cc5864dd7468d0f37af6614df44': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            '768fc78707aafe4c9dc0f4454fdaa459': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            '67ce158b5cfe9f4d8d2a11002c299008': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            '93cd08021eb2234cb0d474c6eda62f3c': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            '5955037cc36c674cb40f64dd5203c7f3': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            '2668cf8f704b6847822e1dfb58f7c37e': ('f6b274c2c28e4b45b0c2762be4e095fb', 'mov o2.xyzw, v3.xyzw'),
            'f5add518a67dfc4791fba737492b4868': ('91ccb94877dac34e988dd1d7bf625e2c', 'mov o1.xyzw, v3.xyzw'),
            'd0343de6d387fb448583bea9b26c6e80': ('91ccb94877dac34e988dd1d7bf625e2c', 'mov o1.xyzw, v3.xyzw'),
            'a5ce32cc4646da42a80d41c4af46a9a5': ('91ccb94877dac34e988dd1d7bf625e2c', 'mov o1.xyzw, v3.xyzw'),
            '9123301bd4b9e347943503e81cadd16f': ('91ccb94877dac34e988dd1d7bf625e2c', 'mov o1.xyzw, v3.xyzw'),
        }.get(sid) if arguments.profile_domain == 'kouku' and selection['rendererShape'] in ('ribbon', 'animationTrail', 'animTrail') else None
        if ribbon_uv1_vs:
            assert selection['sourceVF'] == 'fparticlebeamtraildynamicparametervertexfactory'
            assert selection['sourceVS'] == ribbon_uv1_vs[0], 'Unreviewed ribbon UV1 vertex shader'
            vertex = json.loads((OUT/'full_programs'/(selection['sourceVS']+'.json')).read_text(encoding='utf-8-sig'))
            assert ribbon_uv1_vs[1] in vertex['disassembly']['instructions']
        for sig in p['inputSignature']:
            semantic=sig['semanticName'].lower();index=sig['semanticIndex'];reg=sig['register']
            if semantic=='texcoord':
                values={10:'float4(input.sourceBasisX,'+('input.subUVBlend' if subuv else '0.f')+')',11:'float4(input.sourceBasisZ,input.handedness)',0:('float4(input.uv,input.uv1)' if mesh else 'float4(input.uv,'+('input.uvNext' if subuv else 'float2(0.f,0.f)')+')'),1:'input.color',2:('input.dynamicParameter' if dynamic else 'float4(0.f,0.f,0.f,0.f)'),4:'float4(0.f,0.f,0.f,1.f)',6:'float4(input.tangentView,1.f)',5:'float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW)'}
                if model:
                    values.update({0:'float4(input.uv,0.f,0.f)',4:'float4(0.f,0.f,0.f,1.f)',5:'float4(input.sourceWorldPosition,1.f)',6:'float4(input.tangentView,1.f)',7:'float4(input.tangentUp,0.f)'})
                if decal:
                    values.update({0:'float4(input.uv,input.uv1)',7:'float4(input.tangentUp,0.f)'})
                if ribbon_uv1_vs:
                    values[0]='float4(input.uv,input.uv1)'
                if kouku_ground or guardian_world_position:
                    values[5]='float4(input.sourceWorldPosition,1.f)'
                if kouku_ice_distortion or kouku_world_distortion or kouku_fire_world:
                    values[5]='float4(input.sourceWorldPosition,1.f)'
                if kouku_lit:
                    values[7]='float4(input.tangentUp,0.f)'
                    if kouku_lit[2] in ('world','actor','world_color'): values[5]='float4(input.sourceWorldPosition,1.f)'
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
            if (model or decal or kouku_lit or kouku_ice or kouku_glass_post or source_motion_blur or action_post_rt0) and re.search(r'\bo[1-9]\.',ins):continue # Existing forward carrier consumes RT0; other native MRT writes remain recorded in the source archive.
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
        if arguments.profile_domain == 'kouku' and program == 2893:
            assert sid == '1eb6e82b0befd243ba7ffc9e49b6d067'
            assert r['sourceMaterial'] == 'fx_m_mi_l_00.fx_mi.fx_l_me_sy_12_3_ma'
            assert p['disassembly']['instructions'][73] == 'sqrt o5.xyz, r4.xyzx'
            assert p['disassembly']['instructions'][79] == 'mov o3.xyz, r1.xyzx'
            assert p['disassembly']['instructions'][108] == 'mul_sat o3.w, cb0[8].z, l(0.002000)'
            lines += KOUKU_DOVE_DIRECT_LIGHT_ADAPTER.splitlines()
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
        row['usesOneLayerDistortion'] = sid == 'eb2bcd5c8f3c6c49805ab689687b14f6'
        if selection.get('sourceTransformMesh'):
            assert mesh and not model and selection.get('sourceStaticMeshComponents')
            row['sourceTransformMesh'] = True
            row['sourceStaticMeshComponents'] = selection['sourceStaticMeshComponents']
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
print('Generated',len(rows),'native material programs, deferred',errors,'lines',len(target.read_text(encoding='utf-8-sig').splitlines()))
