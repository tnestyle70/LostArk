"""Join recovered native movie PS groups to their original local-factory VS ABI.

The shader-map receipt and DXBC signatures are extraction artifacts, never guessed
from material names. Output is a candidate; --install explicitly replaces sources.
"""
from __future__ import annotations

import argparse
import collections
import json
from pathlib import Path

BASE = 'tbasepassvertexshaderfnolightmappolicyfnodensitypolicy'
LIGHT = 'tlightvertexshaderfdirectionallightpolicyfnostaticshadowingpolicy'

# TEXCOORD numbers alone do not distinguish world position from clip position.
# These original LocalVF instructions export the unprojected local-to-world row.
WORLD_POSITION_OUTPUTS = {
    (1518, 'Base'): ('239396ffe9f57b47a19ee2955207d2e8', 7, 'mov o7.xyzw, r3.xyzw'),
    (1518, 'Light'): ('8b4317e05c02a1499fd230ff26e7e0ea', 8, 'mov o8.xyzw, r3.xyzw'),
    (1523, 'Base'): ('520ba8522a929f49bed4a7b1e7088c26', 8, 'mov o8.xyzw, r0.xyzw'),
    (1523, 'Light'): ('b763054422c97445804647381a99e406', 6, 'mov o6.xyzw, r3.xyzw'),
}


def read(path):
    return json.loads(path.read_bytes())


def generate(evidence, receipts):
    groups = collections.defaultdict(list)
    proof = []
    for receipt in receipts:
        for row in read(receipt)['programs']:
            for stage, kind in [('Base', BASE), ('Light', LIGHT)]:
                signatures = set()
                ids = set()
                for mic in row['materials']:
                    sid = evidence['materials'][mic].get(kind)
                    if not sid:
                        continue  # Unlit materials have no light program.
                    ids.add(sid)
                    signature = evidence['programs'][sid]['outputSignature']
                    signatures.add(tuple((s['semantic'], s['semanticIndex'], s['register'], s['mask'])
                                         for s in signature if s['semantic'] != 'SV_Position'))
                if not signatures:
                    if stage != 'Light':
                        raise ValueError(f'Missing base VS: {row}')
                    continue
                if len(signatures) != 1:
                    raise ValueError(f'Pixel group {row["program"]} has incompatible {stage} VS layouts')
                sig = next(iter(signatures))
                world_output = WORLD_POSITION_OUTPUTS.get((row['program'], stage))
                if world_output:
                    sid, register, instruction = world_output
                    if ids != {sid} or instruction not in evidence['programs'][sid]['disassembly']['instructions']:
                        raise ValueError(f'Unreviewed movie world-position VS: {row["program"]} {stage}')
                    if not any(s[2] == register and s[3] == 15 for s in sig):
                        raise ValueError(f'Movie world-position register changed: {row["program"]} {stage}')
                groups[stage, sig].append(row['program'])
                proof.append(dict(program=row['program'], stage=stage, vertexShaders=sorted(ids), signature=sig))
    out = ['// Generated from original local-factory VS output signatures.',
           '#ifndef SOURCE_MOVIE_STATIC_INPUTS_INCLUDED', '#define SOURCE_MOVIE_STATIC_INPUTS_INCLUDED',
           'bool IsSourceMovieStatic(uint program)', '{',
           '    return (program >= 1100u && program <= 1166u) ||',
           '        (program >= 1400u && program <= 1413u) || (program >= 1500u && program <= 1525u);', '}']
    for stage in ['Base', 'Light']:
        out += [f'void PackSourceMovie{stage}Input(inout SOURCE_CHARACTER_NATIVE_INPUT input,',
                '    float4 tangentX, float4 tangentZ, float4 color, float2 uv,',
                '    float4 view, float4 light, float4 up, float4 clip, float4 fog, float4 world)', '{',
                '    if (!IsSourceMovieStatic(g_SourceCharacterProgram)) return;',
                '    [unroll] for (uint lane=0u;lane<10u;++lane) input.values[lane]=0.f;']
        for (which, sig), programs in groups.items():
            if which != stage:
                continue
            out.append('    if (' + ' || '.join(f'g_SourceCharacterProgram=={p}u' for p in programs) + ')')
            out.append('    {')
            for semantic, index, register, mask in sig:
                key = semantic, index
                values = {('TEXCOORD', 10): 'tangentX', ('TEXCOORD', 11): 'tangentZ',
                          ('COLOR', 0): 'color', ('COLOR', 1): 'float4(0.f,0.f,0.f,0.f)', ('TEXCOORD', 0): 'float4(uv,0.f,0.f)',
                          ('TEXCOORD', 6): 'view', ('TEXCOORD', 4): 'light' if stage == 'Light' else 'fog',
                          ('TEXCOORD', 5): 'float4(0.f,0.f,0.f,0.f)' if stage == 'Light' else 'clip',
                          ('TEXCOORD', 7): 'clip' if stage == 'Light' else 'up'}
                if key not in values:
                    raise ValueError(f'Unresolved VS semantic {key}')
                # Masks are preserved; unowned lanes remain zero, just like the VS.
                swizzle = ''.join(c for i, c in enumerate('xyzw') if mask & (1 << i))
                value = values[key]
                out.append(f'        input.values[{register}].{swizzle}=({value}).{swizzle};')
            for program in programs:
                world_output = WORLD_POSITION_OUTPUTS.get((program, stage))
                if world_output:
                    out.append(f'        if (g_SourceCharacterProgram=={program}u) input.values[{world_output[1]}]=world; // Original VS world position in source cm.')
            out += ['        return;', '    }']
        out += ['}']
    out += ['#endif', '']
    return '\n'.join(out), proof


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--vertex-evidence', type=Path, required=True)
    p.add_argument('--receipt', type=Path, action='append', required=True)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    source, proof = generate(read(args.vertex_evidence), args.receipt)
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output/'Shader_SourceMovieStaticInputs.hlsli').write_text(source, encoding='utf8')
    (args.output/'vertex-input-receipt.json').write_text(json.dumps(proof, indent=2), encoding='utf8')
    print('Joined source vertex layouts:', len(proof))


if __name__ == '__main__':
    main()
