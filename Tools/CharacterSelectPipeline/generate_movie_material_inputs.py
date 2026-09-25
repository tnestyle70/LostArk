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
                '    float4 view, float4 light, float4 up, float4 clip, float4 fog)', '{',
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
