"""Recover SCENE07A's original full-screen glass material in the V1 carrier.

The source archive is produced by the glass extraction audit. Its material
serial hash and effective inputs must match the installed game before its
already decoded ShaderMap is reused. This generates a reviewed native cohort;
install_kouku_gate1_native_shaders.append_reviewed performs shader registration.
Missing original DDS dependencies are installed by the existing native pipeline.
"""
import argparse
import json
import shutil
from pathlib import Path

import build_kouku_gate3_rainbow_native as source
import build_kouku_pattern_native as native


def prepare(archive: Path, evidence: Path, umodel: Path, release: Path):
    read = lambda path: json.loads(path.read_bytes())

    def write(name, value):
        path = evidence / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf8')

    source.UMODEL = native.UMODEL = umodel
    source.RELEASE = native.RELEASE = release
    source.packages['startup'] = source.resolve_physical_package(umodel, release, 'startup', 'kr')
    original_pkg = native.pkg
    native.pkg = source.pkg = lambda name: native._source_package('startup') if name in (
        'startup', 'enginematerials', 'engine_mi_shaders') else original_pkg(name)
    scene = read(archive / 'scene-glass-tracks.json')
    track = scene['screenMaterialTrack']
    name = scene['scene'].lower() + '.' + track['name']
    assert track['cls'] == 'efinterptrackpostrendermaterial'
    assert scene['screenMaterial'] == 'bfx_mi_bg_00.fx_mi.fx_d_brokenglass_01_tr'
    write('source_module_inputs.json', dict(records={name: dict(
        fullPath=name, classPath='efgame.efinterptrackpostrendermaterial', properties=track['p'])}))
    write('source_class_defaults.json', dict(records=[]))
    write('source_occurrences.json', [dict(elementId='kouku.encore.glass.screen',
        rendererShape='screenPost', sourceMaterial=scene['screenMaterial'], sourceEmitter=name,
        sourcePostRenderMaterialTrack=name, moduleOrder=[], sourceMesh='')])
    output = evidence / 'native'
    (output / 'dxbc').mkdir(parents=True, exist_ok=True)
    shutil.copyfile(archive / 'material-map-scan.json', output / 'material_map_scan.json')
    for path in (archive / 'dxbc').glob('*.dxbc'):
        shutil.copyfile(path, output / 'dxbc' / path.name)
    saved = read(archive / 'material-input-with-map.json')
    fresh = native.material(scene['screenMaterial'])
    for field in ('sourceSerialSha256', 'baseId', 'parentMaterial',
                  'effectiveNumericOverrides', 'textureOverrides'):
        assert saved[field] == fresh[field], ('Source glass material changed', field)
    # The older extraction parsed this unnumbered map before the shared reader
    # was configured for numbered FNames. Reuse only its proven identical map.
    fresh['materialMap'] = saved['materialMap']
    fresh['mapKey'] = saved['mic']['engineEqualityStaticParameterSetSha256']
    textures = read(archive / 'texture-closure.json')
    expressions = saved['materialMap']['uniformExpressionSet']['pixelTexture2DExpressions']
    fresh['effectiveTextures'] = [dict(index=index, parameterName=expression.get('parameterName'),
        sourceReferencePath=texture['path'], sourceObjectPath=texture['path'], properties=texture['properties'])
        for index, (expression, texture) in enumerate(zip(expressions, textures, strict=True))]
    write('native/native_material_inputs.json', dict(materials=[fresh]))
    native.prepare(evidence, 2627, 2627)
    contract = read(output / 'native_runtime_contract.json')
    assert not contract['deferredPrograms'] and len(contract['programs']) == 1
    program = contract['programs'][0]
    assert program['program'] == 2627 and program['rendererShape'] == 'screenPost'
    assert program['sourcePS'] == 'b704e932ad66a6419217a18d5be491e8'


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--evidence', type=Path, required=True)
    parser.add_argument('--umodel', type=Path, required=True)
    parser.add_argument('--release', type=Path, default=source.RELEASE)
    args = parser.parse_args()
    prepare(args.archive, args.evidence, args.umodel, args.release)
