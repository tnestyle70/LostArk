"""Project Valtan's four original screen effects onto the existing native carrier.

The caller first acquires the original ParticleSystems and selects their exact
native shaders. This step requires an unchanged source MIC, static permutation,
numeric inputs, and textures before reusing the installed Kouku programs. It
writes candidates only; the Valtan action builder owns occurrence installation.
"""
from pathlib import Path
import argparse
import copy
import hashlib

import build_kouku_all_source_effects as library
import build_kouku_gate1_full_restore as source


SYSTEMS = {'fx_post.fx_par.par_c_filmnoise_01'} | {
    'fx_post.fx_par.par_c_zoomblur_0' + str(i) for i in (1, 2, 3)}


def build(evidence, reuse_root):
    read, write = source.read, source.write
    targets = {int(k): v for k, v in read(evidence / 'library_targets.json').items()}
    assert {v['system'] for v in targets.values()} == SYSTEMS
    native = evidence / 'native'
    reused = read(native / 'reused_native_programs.json')
    assert reused['freshProgramCount'] == 0
    assert not read(native / 'source_material_failures.json')
    fresh_inputs = {r['sourceMaterial']: r for r in read(native / 'native_material_inputs.json')['materials']}
    previous_inputs = {r['sourceMaterial']: r for r in read(reuse_root / 'native_material_inputs.json')['materials']}
    patch = read(native / 'native_material_patch.json')
    checks = []
    for program in reused['programs']:
        identity = program['sourceMaterial']
        fresh, previous = fresh_inputs[identity], previous_inputs[identity]
        # Exact source serialization also covers default expressions that do
        # not appear in the MIC's explicit scalar/vector override arrays.
        fields = ('sourceSerialSha256', 'mapKey', 'parentMaterial',
                  'effectiveNumericOverrides', 'textureOverrides', 'effectiveTextures')
        for field in fields:
            assert fresh[field] == previous[field], (identity, field, 'source inputs changed')
        material = next(r['material'] for r in patch['programs'] if r['program'] == program['program'])
        installed = read(source.ROOT / 'Data/Effects/Authored' /
                         ('effect.kouku.source.' + ('fx_post.fx_par.par_c_filmnoise_01'
                          if 'filmnoise' in identity else 'fx_post.fx_par.par_c_zoomblur_01') + '.effect.json'))
        expected = installed['elements'][0]['material']
        assert material == expected, (identity, 'installed native inputs differ')
        resources = []
        for texture in material['sourceProfile']['textures']:
            path = source.ROOT / 'Client/Bin/Resources' / texture['assetId']
            assert path.is_file(), path
            resources.append(dict(assetId=texture['assetId'], sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
        checks.append(dict(sourceMaterial=identity, sourceSerialSha256=fresh['sourceSerialSha256'],
            materialMapKey=fresh['mapKey'], sourceVS=program['sourceVS'], sourcePS=program['sourcePS'],
            runtimeShaderProfileId=program['runtimeShaderProfileId'], resources=resources))
    library.project(evidence, targets, native / 'native_material_patch.json', False, evidence)
    installation = read(evidence / 'installation.json')
    assert not installation['sourceFailures'] and len(installation['documents']) == 4
    candidates = []
    for row in installation['documents']:
        path = evidence / 'candidate' / Path(row['path']).name
        document = read(path)
        assert len(document['elements']) == 1
        element = document['elements'][0]
        assert element['kind'] == element['sourceRecipe']['rendererShape'] == 'screenPost'
        before = copy.deepcopy(element['sourceRecipe'])
        element['id'] = element['id'].replace('kouku.', 'valtan.', 1)
        element['groupId'] = document['effectAssetId']
        profile = 'screen.film-noise.reconstructed.v1' if 'filmnoise' in row['sourceParticleSystem'] else 'screen.zoom-blur.reconstructed.v1'
        # This enables the evaluated ScreenPosts consumer. The native material
        # supplies the original shader, curves, color, and dynamic parameters;
        # the enum selects the existing presentation carrier and is not a gain.
        element['detail']['screenPost'].update(enabled=True, profileId=profile,
            status='reconstructed_profile', intensity=1, secondaryIntensity=0,
            frequency=1, tint=[1, 1, 1, 1], randomSeed=element['detail']['particle']['randomSeed'])
        assert element['sourceRecipe'] == before
        document['version'] = 13
        write(path, document)
        candidates.append(dict(effectAssetId=document['effectAssetId'], sourceParticleSystem=row['sourceParticleSystem'],
            elementId=element['id'], screenPostEnabled=True,
            sourceEmitterLoops=element['sourceRecipe']['emitterLoopCount'],
            sourceTiming=element['detail']['timing']))
    write(evidence / 'screen_post_reuse_receipt.json', dict(materials=checks, documents=candidates,
        shaderAndMaterialInputStatus='EXACT_REUSED_SOURCE_INPUTS',
        carrier='EXISTING_FULL_VIEWPORT_NATIVE_SCREEN_POST', installed=False, visualStatus='USER_PENDING'))
    print('Valtan native screen-post candidates:', len(candidates), 'exact reused programs:', len(checks))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, required=True)
    parser.add_argument('--reuse-native-root', type=Path, required=True)
    args = parser.parse_args()
    build(args.evidence_root.resolve(), args.reuse_native_root.resolve())
