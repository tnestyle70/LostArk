"""Derive the independent red breath from the original enabled action notify.

This script writes only its own evidence and one authored effect document.
Catalog/tree/Composition registration belongs to the calling installation.
"""
import argparse
import copy
import hashlib
import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
import build_kouku_action_effect_groups as actions

source = actions.source
EVIDENCE = ROOT / 'out/KoukuSaitenGroups20260912/clone'
ASSET = 'effect.kouku.gate3.clone.breath'
SYSTEM = 'fx_mn_rpct_07_v.par_v_rpct_breath_01_loc_int'
TITLE = '분신소환_기분나빠_브레스'
LIBRARY = ROOT / 'Data/Effects/Authored' / ('effect.kouku.source.' + SYSTEM + '.effect.json')
OUTPUT = ROOT / 'Data/Effects/Authored' / (ASSET + '.effect.json')
ACTION_SOURCE = source.SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json'
MODEL_CONTRACT = ROOT / 'out/KoukuActionEffects20260912/MN_RPCT_07.model_contract.json'
MODULE_EVIDENCE = ROOT / 'out/KoukuAllEffects20260912'


def receipt(path):
    return dict(path=path.resolve().as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def derive(install=False):
    action = next(a for a in source.read(ACTION_SOURCE)['actions'] if a['actionId'] == 4219917)
    stage = next(s for s in action['stages'] if s['stageIndex'] == 0)
    notify = next(n for n in stage['notifies'] if n['notifyId'] == 'action-4219917/stage-000/notify-008')
    contract = source.read(MODEL_CONTRACT)
    cue = actions.decode_notify(notify, contract['sourceSockets'], contract['installedBones'])
    assert cue['enabled'] and cue['attachment']['mode'] == 'SNAPSHOT_ROOT'
    assert cue['attachment']['runtimeAnchors'] == []
    assert cue['sourceParticleSystem'].split("'")[1].lower() == SYSTEM
    assert cue['localTransform']['position'] == [1.0, .9, 0.0]
    assert cue['localTransform']['rotationDegrees'] == [0.0, 0.0, 0.0]
    assert all(abs(a - b) < 1e-6 for a, b in zip(cue['localTransform']['scale'], [1, 1.1, 1.1]))
    assert len(cue['parameterOverrides']) == 1
    assert cue['parameterOverrides'][0]['name'] == 'Color'
    assert cue['parameterOverrides'][0]['vectorValue'] == [1, 1, 1]
    assert abs(notify['durationSeconds'] - 2.9807300567626953) < 1e-9

    template = source.read(LIBRARY)
    assert len(template['elements']) == 15 and not template.get('modelCues')
    index = actions.restored_index(MODULE_EVIDENCE)
    elements, parameters = actions.instantiate(template, cue, notify, ASSET, 'MN_RPCT_07', index)
    identifiers = {e['id']: 'kouku.clone-breath.' + hashlib.sha256((ASSET + '|' + e['id']).encode()).hexdigest()[:24]
                   for e in elements}
    doc = copy.deepcopy(template)
    doc.update(effectAssetId=ASSET, displayName=TITLE, modelCues=[], elements=elements)
    doc['particleSystem']['yawOffsetDegrees'] = -90
    assert doc['particleSystem']['directionYawDegrees'] == 0
    for element in elements:
        element['id'] = identifiers[element['id']]
        element['groupId'] = ASSET
        element['detail']['timing']['startDelaySeconds'] = 0
        attachment = element['actionCueAttachment']
        # Append occurrence owns the stationary root. No model preparation or
        # second source-basis rotation is required by this standalone group.
        attachment.update(enabled=False, follow=False, runtimeBoneName='', snapshotRootSourceBasisYawDegrees=0)
        recipe = element['sourceRecipe']
        if 'particleSystemOccurrenceId' in recipe:
            recipe['particleSystemOccurrenceId'] = ASSET + '/root'
        for module in recipe['modules']:
            for literal in module['literals']:
                if literal['propertyPath'] == 'runtime.providerelementid':
                    literal['value'] = identifiers[literal['value']]

    # The actual mesh TypeData export and CDO have no originalpitch property.
    # Preserve that absence; arbitrary rotation would alter the source mesh.
    module_records = source.read(MODULE_EVIDENCE / 'source_module_inputs.json')['records']
    defaults = {r['fullPath']: r for r in source.read(MODULE_EVIDENCE / 'source_class_defaults.json')['records']}
    typedata = []
    for element in elements:
        for module in element['sourceRecipe']['modules']:
            if module['className'] != 'particlemoduletypedatamesh':
                continue
            record = module_records[module['objectPath']]
            chain, key = [], record.get('archetypeFullPath') or 'engine.default__particlemoduletypedatamesh'
            while key:
                default = defaults[key]
                chain.append(default)
                key = default.get('archetypeFullPath')
            assert not any('originalpitch' in source.norm(r.get('properties', {})) for r in [record] + chain)
            typedata.append(dict(sourceRecord=record, inheritedDefaults=chain, additionalRotationApplied=False))
    assert len(typedata) == 1

    resources = {r['assetId'] for e in elements for r in e['resources']}
    resources.update(t['assetId'] for e in elements for t in e['material']['sourceProfile']['textures'])
    missing = [r for r in sorted(resources) if not (ROOT / 'Client/Bin/Resources' / r).is_file()]
    assert not missing, ('Missing original resources', missing)
    assert all(e['material']['sourceProfile']['enabled'] for e in elements)
    assert len({e['id'] for e in elements}) == len(elements)
    if install:
        assert not OUTPUT.exists() or source.read(OUTPUT) == doc, 'Preserve authored edits: ' + str(OUTPUT)
    source.write(EVIDENCE / 'source_notify.json', dict(actionId=action['actionId'], sourceDisplayName=action['displayName'],
        stageIndex=stage['stageIndex'], sourceAnimationClips=stage['animationClips'], notify=notify, decodedCue=cue))
    source.write(EVIDENCE / 'source_mesh_typedata.json', dict(records=typedata))
    source.write(EVIDENCE / 'parameter_projection.json', dict(parameters=parameters))
    candidate = EVIDENCE / 'candidate' / OUTPUT.name
    source.write(candidate, doc)
    if install:
        source.write(OUTPUT, doc)
    end_seconds = []
    for element in elements:
        timing, recipe, particle = element['detail']['timing'], element['sourceRecipe'], element['detail']['particle']
        active = timing['lifeTimeSeconds']
        if recipe['enabled'] and recipe['emitterDurationSeconds'] > 0 and recipe['emitterLoopCount']:
            active = recipe['emitterDurationSeconds'] * recipe['emitterLoopCount']
        delay = recipe['emitterDelaySeconds'] if recipe['enabled'] else 0
        tail = max(particle['lifeTimeSeconds']) * (particle['sourceScale']['lifeTime'] if recipe['enabled'] else 1)
        end_seconds.append(timing['startDelaySeconds'] + delay + active + timing['afterImageSeconds'] + tail)
    duration = math.ceil(max(end_seconds) * 1000)
    assert duration == 3000, 'Original sprite/mesh recipe duration must match the 3-second playback window'
    source.write(EVIDENCE / 'installation.json', dict(installed=install, documents=[dict(
        effectAssetId=ASSET, displayName=TITLE, path=OUTPUT.relative_to(ROOT).as_posix(), elementCount=len(elements),
        durationMs=duration, sourceParticleSystem=SYSTEM, sourceActionId=4219917, sourceProfileId='MN_RPCT_07',
        sourceNotifyId=notify['notifyId'], categoryPath=['KoukuSaydon', '3관문', '패턴', '세이튼', '분신 소환·기분나빠'],
        defaultAnchorKind='BOSS', followBoss=False, sourceTransform=cue['localTransform'], sourceStartSeconds=notify['localTimeSeconds'],
        sourceDurationSeconds=notify['durationSeconds'], referenceImageIdentity='USER_VISUAL_COMPARISON_PENDING')],
        sourceEvidence=dict(inputs=[receipt(path) for path in [LIBRARY, ACTION_SOURCE, MODEL_CONTRACT,
            MODULE_EVIDENCE / 'source_module_inputs.json', MODULE_EVIDENCE / 'source_class_defaults.json', Path(__file__)]],
            identityMapping='User label is mapped to the recovered Madness Mark breath; the source action does not contain the user label.',
            derivation='Original enabled root notify instantiated through the existing decoder and parameter projector; independent start zero; source duration, offset, nonuniform scale and native palette retained.',
            basis=dict(sourceForward='+X', runtimeForward='+Z', systemYawDegrees=-90, attachmentBasisYawDegrees=0,
                normalizedRootOffset=[0, .9, 1], transformOrder='Local * SystemYaw * AppendRoot'),
            attachment='Append occurrence stationary BOSS root; followBoss=false; no skeletal model cue or enabled action attachment.',
            originalMeshTypeDataEvidence='source_mesh_typedata.json', sourceNotifyEvidence='source_notify.json',
            durationBasis='Calculate_ElementEndSeconds: source recipe duration times loops, emitter delay, timing afterimage and scaled particle lifetime tail.',
            resourceCount=len(resources), missingResources=missing, output=receipt(OUTPUT if install else candidate)),
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(installed=install, path=OUTPUT.relative_to(ROOT).as_posix(), elementCount=len(elements), durationMs=duration,
        resourceCount=len(resources), missingResources=missing, manifest=(EVIDENCE / 'installation.json').relative_to(ROOT).as_posix())))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--install', action='store_true', help='Install the candidate after preserving existing authored edits')
    derive(parser.parse_args().install)
