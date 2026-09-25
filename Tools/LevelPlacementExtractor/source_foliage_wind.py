"""Bounded source foliage vertex-program inputs for the existing map carrier."""
from __future__ import annotations
import json
import math
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
from evaluate_ue3_material_uniform_expressions import evaluate_expression

PROGRAM = 'UE3_FOLIAGE_VS_E4FE'
SHADER_ID = 'e4fe43228f19284f9ca096e27b675db6'
INSTRUCTION_SHA = '59a97d3f9c4de20b8cddde8e285ecb9c03f8bde21763186c4ec7cf1463c8a633'
PROGRAMS = {
    PROGRAM: (SHADER_ID, INSTRUCTION_SHA, 181, 2, 16, 10),
    'UE3_FOLIAGE_VS_A1C6': ('a1c6456c3a04114abd875c56a006963d',
        'e55de237331277ac27534338bc3c1a979dc3a3c0a68169e838b7dfcfe5cb45d6', 132, 0, 7, 1),
    'UE3_FOLIAGE_VS_1C39': ('1c39a832e3e5f745a865324bed15057e',
        '5a0af0d3dc625aac63fad8350f34f1b43e1928f6666bff01c5ee9efe7038c539', 164, 1, 10, 4),
}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate(value):
    require(isinstance(value, dict) and set(value) == {'program', 'localCenter', 'localBounds', 'actorPositionSourceCm', 'windDirectionSpeedSource', 'playerPositionSource', 'scalarRows'}, "Invalid foliage wind contract: isinstance(value, dict) and set(value) == {'program', 'localCenter', 'localBounds', 'actorPositionSourceCm', 'windDirectionSpeedSource', 'playerPositionSource', 'scalarRows'}")
    require(value['program'] in PROGRAMS, 'Unknown source foliage vertex program')
    for key in ('localCenter', 'localBounds', 'actorPositionSourceCm',
                'windDirectionSpeedSource', 'playerPositionSource'):
        require(isinstance(value[key], list) and len(value[key]) == 4, 'Invalid foliage wind contract: isinstance(value[key], list) and len(value[key]) == 4')
        require(all((type(x) in (int, float) and math.isfinite(x) and (abs(x) <= 100000000.0) for x in value[key])), 'Invalid foliage wind contract: all((type(x) in (int, float) and math.isfinite(x) and (abs(x) <= 100000000.0) for x in value[key]))')
    require(value['localCenter'][3] == 1 and value['actorPositionSourceCm'][3] == 0, "Invalid foliage wind contract: value['localCenter'][3] == 1 and value['actorPositionSourceCm'][3] == 0")
    require(all((0 < x <= 100000.0 for x in value['localBounds'])), "Invalid foliage wind contract: all((0 < x <= 100000.0 for x in value['localBounds']))")
    # This admission has source-absent scene wind, not an invented live wind service.
    require(value['windDirectionSpeedSource'] == [0, 0, 1, 0], "Invalid foliage wind contract: value['windDirectionSpeedSource'] == [0, 0, 1, 0]")
    rows = value['scalarRows']
    require(isinstance(rows, list) and len(rows) == 4, 'Invalid foliage wind contract: isinstance(rows, list) and len(rows) == 4')
    require(all((isinstance(row, list) and len(row) == 4 and all((type(x) in (int, float) and math.isfinite(x) and (abs(x) <= 100000000.0) for x in row)) for row in rows)), 'Invalid foliage wind contract: all((isinstance(row, list) and len(row) == 4 and all((type(x) in (int, float) and math.isfinite(x) and (abs(x) <= 100000000.0) for x in row)) for row in rows))')
    spec = PROGRAMS[value['program']]
    flat = [lane for row in rows for lane in row]
    require(flat[spec[5]] == 0 and all(x == 0 for x in flat[spec[4]:]), 'Invalid wind time or unused scalar lanes')
    if value['program'] == PROGRAM:
        require(rows[0][2] > 0, 'Non-positive original foliage affect distance')
    elif value['program'] == 'UE3_FOLIAGE_VS_A1C6':
        require(value['localCenter'] == [0, 0, 0, 1] and value['playerPositionSource'] == [0, 0, 0, 0],
                'Basic source wind pivots at object origin and has no player input')
    else:
        require(rows[0][0] > 0 and value['playerPositionSource'] == [99999, 99999, 0, 0],
                'Invalid source grass affect distance or original no-player sentinel')


def build(binding, material, verified_file, base, sources, tracked=None):
    require(set(binding) == {'nativeInputs', 'vertexProgram', 'bounds', 'ownerPositionSourceCm', 'ownerPositionEvidence'}, "Invalid foliage wind contract: set(binding) == {'nativeInputs', 'vertexProgram', 'bounds', 'ownerPositionSourceCm', 'ownerPositionEvidence'}")
    documents = {}
    for key in ('nativeInputs', 'vertexProgram', 'bounds', 'ownerPositionEvidence'):
        path, data = verified_file(binding[key], base)
        sources.append(dict(path=str(path), sha256=binding[key]['sha256']))
        documents[key] = json.loads(data)
    program = documents['vertexProgram']
    admitted = [key for key, spec in PROGRAMS.items() if spec[0] == program['shaderId']]
    require(len(admitted) == 1, 'Unreviewed foliage vertex program')
    program_name = admitted[0]
    spec = PROGRAMS[program_name]
    require(program['disassembly']['instructionSha256'] == spec[1] and
            program['disassembly']['instructionCount'] == spec[2], 'Foliage vertex instruction closure changed')
    matches = [row for row in documents['nativeInputs']['materials'] if row['sourceMaterial'].casefold() == material]
    require(len(matches) == 1, 'Invalid foliage wind contract: len(matches) == 1')
    native = matches[0]
    require(any(factory['vertexFactoryType'] == 'flocalvertexfactory' and
                any(ref['shaderIdHex'] == spec[0] and ref['shaderType'] ==
                    'tbasepassvertexshaderfnolightmappolicyfnodensitypolicy'
                    for ref in factory['shaderReferences'])
                for factory in native['materialMap']['vertexFactories']),
            'Material does not own this exact source foliage vertex shader')
    uniform = native['materialMap']['uniformExpressionSet']
    require(len(uniform['vertexVectorExpressions']) == spec[3] and len(uniform['vertexScalarExpressions']) == spec[4],
            'Foliage vertex uniform closure changed')
    require(not uniform['vertexTexture2DExpressions'], "Invalid foliage wind contract: not uniform['vertexTexture2DExpressions']")
    def track(node):
        if isinstance(node, dict):
            name = node.get('parameterName')
            if tracked is not None and name in tracked:
                tracked[name]  # Same explicit field-coverage accounting as the surface mapper.
            for child in node.values(): track(child)
        elif isinstance(node, list):
            for child in node: track(child)
    track(uniform['vertexVectorExpressions']); track(uniform['vertexScalarExpressions'])
    overrides = native['effectiveNumericOverrides']
    scalars = {(key, 0): value['value'] for key, value in overrides['scalars'].items()}
    vectors = {(key, 0): value['value'] for key, value in overrides['vectors'].items()}
    def evaluate(expression, time=0):
        kind = expression['typeName'].casefold()
        if kind == 'fmaterialuniformexpressionclamp':
            return [min(max(value, low), high) for value, low, high in zip(
                evaluate(expression['input'], time), evaluate(expression['minimum'], time),
                evaluate(expression['maximum'], time))]
        if kind == 'fmaterialuniformexpressionfoldedmath':
            require(expression['operationOrdinal'] == 2, 'This vertex closure contains only folded multiply')
            return [a*b for a,b in zip(evaluate(expression['a'], time), evaluate(expression['b'], time))]
        return evaluate_expression(expression, scalars, vectors,
                                   game_time_seconds=time, real_time_seconds=time)
    v = [evaluate(node) for node in uniform['vertexVectorExpressions']]
    s = [evaluate(node)[0] for node in uniform['vertexScalarExpressions']]
    require(all(evaluate(node, 7)[0] == (7 if index == spec[5] else s[index])
                for index, node in enumerate(uniform['vertexScalarExpressions'])), 'Foliage time lane changed')
    center = v[0] if v else [0, 0, 0, 1]
    require(center[3] == 1, 'Source foliage center is not a position')
    player = v[1] if program_name == PROGRAM else [0, 0, 0, 0]
    if program_name == 'UE3_FOLIAGE_VS_1C39':
        # The native engine input is dynamic scene-character position. These
        # original MIC sentinel parameters only supply the no-character case.
        names = ('playerposition_x', 'playerposition_y')
        require(all(overrides['scalars'].get(name, {}).get('value') == 99999 for name in names),
                'Original no-player MIC sentinel changed')
        player = [overrides['scalars'][name]['value'] for name in names] + [0, 0]
        for name in names:
            if tracked is not None and name in tracked:
                tracked[name]
    s.extend([0] * (16 - len(s)))
    b = documents['bounds']['nativeFBoxSphereBounds']
    require(len(b) == 7, 'Invalid foliage wind contract: len(b) == 7')
    owner = binding['ownerPositionSourceCm']
    require(documents['ownerPositionEvidence']['actorPositionSourceCm'] == owner, "Invalid foliage wind contract: documents['ownerPositionEvidence']['actorPositionSourceCm'] == owner")
    require(documents['ownerPositionEvidence']['sceneWindSourceCount'] == 0, "Invalid foliage wind contract: documents['ownerPositionEvidence']['sceneWindSourceCount'] == 0")
    # Loader's existing source-model preScale is .01; world transforms carry the
    # original dimensionless component scale. Source [x,y,z] -> runtime [x,z,-y].
    result = dict(program=program_name, localCenter=[center[0]*.01, center[2]*.01, -center[1]*.01, 1],
                  localBounds=[b[3]*.01, b[5]*.01, b[4]*.01, b[6]*.01],
                  actorPositionSourceCm=[*owner, 0], windDirectionSpeedSource=[0,0,1,0],
                  playerPositionSource=player, scalarRows=[s[i:i+4] for i in range(0,16,4)])
    validate(result)
    return result
