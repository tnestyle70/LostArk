"""Acquire the original class-select ParticleSystems through the shared importer."""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import sys
import copy
import struct


def project(output, require_native):
    """Retain shared Cascade recipes and exact native material/VF bindings."""
    from build_kouku_action_effect_groups import restored_index
    source = aura.source
    index = restored_index(output)
    occurrences = source.read(output / 'source_occurrences.json')
    assets = sorted({o['effectAssetId'] for o in occurrences})
    source.SELECTED = {asset: ([], asset.rsplit('.', 1)[-1]) for asset in assets}
    notifies = [dict(notifyId=asset, sourceType='WorldAura', actionId=asset,
        cue=dict(parameterOverrides=[dict(name='alpha', type='scalar', scalarValue=.10000000149011612)]
                 if asset.endswith('par_q_watersplash_01') else [])) for asset in assets]
    source.project(output, index, notifies, occurrences, {}, output / 'projected')
    native = {}
    if require_native:
        native = {key: p['material'] for p in source.read(output / 'native/native_material_patch.json')['programs']
                  for key in p['occurrences']}
    geometries = source.read(output / 'geometry_installation.json') if require_native else []
    mesh_assets = {row['sourceObject']: row['assetId'] for row in geometries}
    by_element = {o['elementId']: o for o in occurrences}
    receipt = []
    for asset in assets:
        doc = source.read(output / 'projected' / f'effect.kouku.gate1.{asset}.full.restore.effect.json')
        doc.update(effectAssetId=asset, displayName='Guardian class selection | ' + asset.rsplit('.', 1)[-1])
        for element in doc['elements']:
            element['groupId'] = asset
            element['displayName'] = by_element[element['id']]['sourceEmitter'].rsplit('.', 1)[-1]
            element['actionCueAttachment']['enabled'] = False
            if require_native:
                element['material'] = copy.deepcopy(native[element['id']])
                element['detail']['uv'].update(start=[0, 0], speed=[0, 0], wave=False, sequence=False)
                element['detail']['color']['emissiveIntensity'] = 1
            if element['sourceRecipe']['rendererShape'] == 'mesh':
                occurrence = by_element[element['id']]
                typed = next(index.objects[k] for k in occurrence['moduleOrder']
                             if index.objects[k].class_name == 'particlemoduletypedatamesh')
                assert source.imported.prop(typed.properties, 'boverridematerial', False), typed.key
                element['detail']['mesh']['useModelMaterial'] = False
                element['detail']['mesh']['sourceTypeDataRotationDegrees'] = [
                    float(source.imported.prop(typed.properties, field, 0)) for field in ('roll', 'pitch', 'yaw')]
                if require_native:
                    element['resources'] = [dict(slotId='meshModel', assetId=mesh_assets[occurrence['sourceMesh']])]
            if require_native:
                for resource in element['resources'] + element['material']['sourceProfile'].get('textures', []):
                    assert (ROOT / 'Client/Bin/Resources' / resource['assetId']).is_file(), resource
        # A world PSC owns its original emitter loops. The phase controller
        # supplies a finite source-loop horizon and extends it without respawn.
        infinite = any(e['sourceRecipe']['emitterLoopCount'] == 0 for e in doc['elements'])
        retained = any(e['detail']['particle']['lifeTimeSeconds'] == [0., 0.] for e in doc['elements'])
        source.write(output / ('candidate' if require_native else 'source-candidate') / (asset + '.effect.json'), doc)
        receipt.append(dict(assetId=asset, nativeInfinite=infinite, sourceZeroLifetime=retained, elements=len(doc['elements']),
            emitterLoops=[e['sourceRecipe']['emitterLoopCount'] for e in doc['elements']]))
    source.write(output / 'effect-library.json', dict(assets=receipt, nativeReady=require_native))

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/EffectPipeline"))
import build_gate3_world_auras as aura

SYSTEMS = (
    "fx_pc_ddk_04.par_m_ddk_guillotinespin_exp_01_08",
    "fx_pc_ddk_04.par_m_ddk_guillotinespin_trail_01",
    "fx_pc_ddk_04.par_m_ddk_guillotinespin_exp_01",
    "fx_q_w_03.fx_par_01.par_q_pcselect_floatingember_01",
    "fx_q_w_03.fx_par_01.par_q_watersplash_01",
    "fx_post.fx_par.par_c_zoomblur_01",
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--materials", action="store_true")
    parser.add_argument("--cached-source", action="store_true")
    parser.add_argument("--first-program", type=int)
    parser.add_argument("--last-program", type=int)
    parser.add_argument("--resource-root", type=Path, default=ROOT / 'Client/Bin/Resources')
    parser.add_argument("--geometry", action="store_true")
    parser.add_argument("--project", action="store_true")
    parser.add_argument("--project-source", action="store_true")
    args = parser.parse_args()
    assert args.output.resolve().is_relative_to(ROOT / "out")
    aura.OUT = args.output
    aura.TARGETS = {"effect.classselect.guardianknight."+system: (system, system.rsplit(".", 1)[-1]) for system in SYSTEMS}
    original_record = aura.source.record_from_export
    loaded = {}
    redirects = []
    def redirect_record(package, logical, entry):
        class_name = aura.source.ue3.package_ref_name(entry.class_index, package.imports, package.exports).lower()
        if class_name != "objectredirector":
            return original_record(package, logical, entry)
        raw = package.logical[entry.serial_offset:entry.serial_offset+entry.serial_size]
        assert len(raw) == 16
        reference = struct.unpack_from("<i", raw, 12)[0]
        target = aura.source.qualify(logical, aura.source.ue3.package_ref_path(reference, package.imports, package.exports), reference)
        target_package, relative = target.split(".", 1)
        if target_package not in loaded:
            loaded[target_package] = aura.source.load_package(aura.PACKAGES[target_package], aura.source.ue3.LOSTARK_KR_AES_KEY)
        original = aura.source.qualify(logical, aura.source.ue3.package_ref_path(entry.index+1, package.imports, package.exports))
        result = copy.deepcopy(redirect_record(loaded[target_package], target_package,
            aura.source.find_export(loaded[target_package], relative)))
        result["fullPath"] = original
        result["redirectTarget"] = target
        redirects.append(dict(source=original, target=target))
        return result
    aura.source.record_from_export = redirect_record
    if not args.cached_source:
        _, occurrences, _ = aura.acquire()
        aura.source.write(args.output / 'source_redirects.json', redirects)
        inactive = [o for o in occurrences if 'guillotinespin_exp_01' in o['sourceSystem']]
        aura.source.write(args.output / 'inactive-source-occurrences.json', dict(
            reason='Original PSC bAutoActivate false and empty Matinee toggle track', occurrences=inactive))
        occurrences = [o for o in occurrences if o not in inactive]
        aura.source.write(args.output / 'source_occurrences.json', occurrences)
    else:
        occurrences = aura.source.read(args.output / 'source_occurrences.json')
    print(json.dumps(dict(systems=len(SYSTEMS), occurrences=len(occurrences),
        materials=len({row['sourceMaterial'] for row in occurrences}))), flush=True)
    if args.materials:
        assert args.first_program and args.last_program and args.first_program <= args.last_program
        native = aura.native
        native.UMODEL = aura.TOOL; native.RELEASE = aura.GAME
        native.source_native.UMODEL = aura.TOOL
        native.source_native.packages = aura.PACKAGES
        native.prepare(args.output, args.first_program, args.last_program,
            reuse_roots=[ROOT / 'out/GuardianEffects20260922/native/reviewed_install', ROOT / 'out/KoukuAllEffects20260912/native'],
            resource_root=args.resource_root)
    if args.geometry:
        aura.prepare_geometry()
    if args.project or args.project_source:
        project(args.output, args.project)


if __name__ == "__main__":
    main()
