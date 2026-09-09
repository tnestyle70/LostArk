"""Assemble every bound LanceMaster full restore from recovered native evidence.

Original unified documents and the approved V full restore are read-only. No
Catalog, resource-tree, animation event, or project registration is edited here.
"""
from pathlib import Path
import argparse, collections, copy, json, sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/ActorXAssetCooker'))
import retime_wmodel_ticks as animation_ticks


def read(path):
    return json.loads(path.read_bytes())


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf8')


def normalize_horse_clips(evidence):
    expected = {'sk_super_squalllance_02': 2.2, 'sk_super_squalllance_03': 1.0,
                'sk_super_squalllance_04': 41 / 30}
    rows = []
    for section in range(4):
        asset = f'Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section{section}.wmodel'
        path = ROOT / 'Client/Bin/Resources' / asset
        data = animation_ticks.read_bounded(path)
        clips = animation_ticks.find_animation_sections(data)
        if {name for _, name in clips} != set(expected):
            raise ValueError(('Horse source clip set changed', asset))
        rates = {animation_ticks.ANIMATION_HEADER.unpack_from(data, offset)[3] for offset, _ in clips}
        if rates not in ({30.0}, {1000.0}):
            raise ValueError(('Horse clip rate is neither source cook nor runtime rate', asset, rates))
        report = animation_ticks.retime(data, 30, next(iter(rates)))
        animation_ticks.verify(data, report, 30)
        for clip in report:
            if abs(clip['durationTicks'] / 30 - expected[clip['name']]) > 1e-5:
                raise ValueError(('Horse runtime wall-clock duration differs from source PSA', asset, clip))
        if rates == {1000.0}:
            animation_ticks.write_atomically(path, bytes(data))
        rows.append(dict(assetId=asset, clips=report))
    write(evidence / 'horse-runtime-contract.json', rows)


def repair_altv_solo_documents(evidence):
    excluded_path = evidence / 'altv-solo-exclusions.json'
    previous = read(excluded_path) if excluded_path.exists() else []
    collision_receipt = evidence / 'horse_solo_collision_exclusions.json'
    if collision_receipt.exists():
        previous.extend(read(collision_receipt))
    excluded = {(r['document'], r['elementId']): r for r in previous}
    for path in (ROOT / 'Data/Effects/Authored').glob('effect.lancemaster.skill.34630.*.full.restore.effect.json'):
        document = read(path)
        kept, changed = [], False
        for element in document['elements']:
            modules = [m for m in element['sourceRecipe'].get('modules', [])
                       if m['className'] in ('particlemodulecollision', 'particlemoduleeventgenerator')
                       and not any(l['propertyPath'] == 'benabled' and l['value'] is False
                                   for l in m['literals'])]
            if modules:
                reason = ('SOURCE_COLLISION_SIMULATION_NOT_SUPPORTED' if any(
                    m['className'] == 'particlemodulecollision' for m in modules)
                          else 'SOLO_REQUIRES_LIVE_SIBLING_PARTICLE_PROVIDER')
                excluded[document['effectAssetId'], element['id']] = dict(
                    document=document['effectAssetId'], elementId=element['id'],
                    sourceNode=element.get('sourceNode'), sourceMaterial=element['material'].get('sourceMaterialPath'),
                    reason=reason, modules=modules)
                changed = True
                continue
            attachment = element.get('actionCueAttachment', {})
            if attachment.get('modelCueId'):
                slot = attachment['modelCueId'] + '.anchor.' + attachment['sourceAnchorSlotId']
                changed |= slot != attachment['runtimeAnchorSlotId']
                attachment['runtimeAnchorSlotId'] = slot
            kept.append(element)
        if changed:
            document['elements'] = kept
            write(path, document)
    write(excluded_path, list(excluded.values()))
    return list(excluded.values())


def build(evidence, previous):
    normalize_horse_clips(evidence)
    altv_exclusions = repair_altv_solo_documents(evidence)
    old_programs = read(previous / 'native_runtime_contract.json')['programs']
    new_programs = [r for r in read(evidence / 'native_runtime_contract.json')['programs']
                    if r['program'] >= 1200]
    programs = {(r['sourceMaterial'], r['sourceVF']): r for r in old_programs + new_programs}
    old_materials = read(previous / 'native_material_patch.json')['programs']
    new_materials = [r for r in read(evidence / 'native_material_patch.json')['programs']
                     if r['program'] >= 1200]
    materials = {r['program']: r['material'] for r in old_materials + new_materials}
    selected = {r['elementId']: r for r in read(evidence / 'selected_runtime_material_programs.json')['occurrences']}
    source = {r['elementId']: r for r in read(evidence / 'new_occurrence_material_bindings.json')}
    geometry = {r['source']: r for r in read(evidence / 'mesh_cook_receipt.json')
                if r['status'] == 'COOKED_NATIVE_GEOMETRY'}
    stages = read(evidence / 'bound_source_stages.json')
    exclusions, documents, empty = [], [], []
    for stage in stages:
        asset = stage['effectAssetId']
        document = read(evidence / (asset + '.source-stage.effect.json'))
        source_count = len(document['elements'])
        output = []
        for element in document['elements']:
            key = element['id']
            original = source[key]
            modules = element['sourceRecipe']['modules']
            blocked = [m for m in modules if m['className'] in
                       ('particlemoduletypedataribbon', 'efparticlemodulelocationemitter',
                        'particlemodulelocationemitter', 'particlemoduleeventreceiverspawn',
                        'particlemoduleeventgenerator', 'particlemodulecollision')
                       and not any(l['propertyPath'] == 'benabled' and l['value'] is False
                                   for l in m['literals'])]
            reason = None
            if blocked:
                reason = 'SOURCE_RIBBON_HISTORY_GEOMETRY_NOT_SUPPORTED' if any(
                    m['className'] == 'particlemoduletypedataribbon' for m in blocked
                ) else 'SOURCE_COLLISION_SIMULATION_NOT_SUPPORTED' if any(
                    m['className'] == 'particlemodulecollision' for m in blocked
                ) else 'SOLO_REQUIRES_LIVE_SIBLING_PARTICLE_PROVIDER'
            selection = selected.get(key, {})
            program = programs.get((selection.get('resolvedMaterial'), selection.get('sourceVF')))
            light = element['kind'] == 'light' and element['detail']['light']['enabled'] and element['sourcePresentation'].get('status') == 'source_exact'
            if not light and program is None:
                reason = selection.get('status', 'NATIVE_INPUT_UNCLOSED')
                if reason == 'EXACT_MAP_AND_VF_SELECTED':
                    reason = 'SOURCE_ENGINE_PREFIX_OR_VERTEX_PROGRAM_UNCLOSED'
            if reason:
                exclusions.append(dict(document=asset, elementId=key, sourceActionCueId=original['sourceActionCueId'],
                                       sourceEmitter=original['sourceEmitter'], sourceMaterial=original['sourceMaterial'],
                                       reason=reason, blockingModules=[m['className'] for m in blocked]))
                continue
            element['visible'] = True
            element['transformInheritance'] = dict(enabled=False, masterElementId='')
            if light:
                element['material'] = dict(templateId='effect.standard', sourceMaterialPath='',
                                           renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False))
                element['resources'] = []
                output.append(element)
                continue
            element['material'] = copy.deepcopy(materials[program['program']])
            element['resources'] = []
            if program['rendererShape'] == 'mesh':
                typed = next(m for m in modules if m['className'] == 'particlemoduletypedatamesh')
                mesh = next(l['value'] for l in typed['literals'] if l['propertyPath'] == 'mesh.objectpath')
                cooked = geometry[mesh]
                element['resources'] = [dict(slotId='meshModel', assetId=cooked['assetId'])]
                element['detail']['mesh'].update(useModelMaterial=False, modelPreScale=cooked['modelPreScale'])
            if program['rendererShape'] == 'screenPost':
                element['kind'] = 'screenPost'
                element['sourceRecipe']['rendererShape'] = 'screenPost'
                material_name = program['sourceMaterial']
                profile = ('screen.zoom-blur.reconstructed.v1' if 'zoomblur' in material_name else
                           'screen.film-noise.reconstructed.v1' if 'filmnoise' in material_name else
                           'screen.rgb-noise.reconstructed.v1')
                element['detail']['screenPost'].update(enabled=True, profileId=profile, status='reconstructed_profile',
                                                       intensity=1, secondaryIntensity=0, frequency=1,
                                                       tint=[1, 1, 1, 1], randomSeed=1)
            element['detail']['uv'].update(start=[0, 0], speed=[0, 0], wave=False, sequence=False)
            element['sourcePresentation'] = dict(enabled=False)
            output.append(element)
        if not output:
            empty.append(dict(**stage, reason='SOURCE_HAS_NO_ACTIVE_RENDERABLE_OCCURRENCE'))
            continue
        document.update(version=13, elements=output, modelCues=[])
        write(ROOT / 'Data/Effects/Authored' / (asset + '.effect.json'), document)
        documents.append(dict(**stage, elements=len(output), sourceElements=source_count,
                              shapes=dict(collections.Counter(e['sourceRecipe']['rendererShape'] for e in output))))
    write(evidence / 'full_restore_admission.json', dict(documents=documents, emptyStages=empty, excluded=exclusions))
    write(ROOT / '.md/GB/09-09/2026-09-09_LANCEMASTER_ALL_FULL_RESTORE_EXCLUSIONS.json',
          dict(sourceActionExclusions=read(evidence / 'source_action_exclusions.json'), emptyStages=empty,
               occurrences=exclusions, altVExclusions=altv_exclusions))
    print('Documents', len(documents), 'elements', sum(r['elements'] for r in documents),
          'empty stages', len(empty), 'excluded', collections.Counter(r['reason'] for r in exclusions))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--evidence', type=Path, default=ROOT / 'out/LanceMasterAllRestore20260910')
    parser.add_argument('--previous', type=Path, default=ROOT / 'out/LanceMasterVARestore20260909')
    args = parser.parse_args()
    build(args.evidence, args.previous)
