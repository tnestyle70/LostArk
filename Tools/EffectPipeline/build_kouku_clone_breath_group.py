"""Derive the independent red breath from the original enabled action notify.

This script writes only its own evidence and one authored effect document.
Catalog/tree/Composition registration belongs to the calling installation.
"""
import argparse
import copy
import hashlib
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
import build_kouku_action_effect_groups as actions

source = actions.source
EVIDENCE = ROOT / 'out/KoukuSaitenGroups20260912/clone'
ASSET = 'effect.kouku.gate3.clone.breath'
AUTHORED_DURATION_MS = 4279
SYSTEM = 'fx_mn_rpct_07_v.par_v_rpct_breath_01_loc_int'
TITLE = '분신소환_기분나빠_브레스'
LIBRARY = ROOT / 'Data/Effects/Authored' / ('effect.kouku.source.' + SYSTEM + '.effect.json')
OUTPUT = ROOT / 'Data/Effects/Authored' / (ASSET + '.effect.json')
ACTION_SOURCE = source.SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json'
MODEL_CONTRACT = ROOT / 'out/KoukuActionEffects20260912/MN_RPCT_07.model_contract.json'
MODULE_EVIDENCE = ROOT / 'out/KoukuAllEffects20260912'
REPAIR_FIELDS = {
    ('particlemodulerotation', 'startrotation'),
    ('particlemodulemeshrotation_seeded', 'startrotation'),
    ('particlemodulelocationprimitivecylinder', 'startradius'),
    ('particlemodulelocationprimitivecylinder', 'velocityscale'),
    ('particlemodulelocationprimitivesphere', 'startradius'),
    ('particlemodulelocationprimitivesphere_seeded', 'velocityscale'),
    ('particlemodulespawn', 'rate'),
}


def repair_source_defaults(document):
    """Recover missing nested cooked fields in this source system only.

    A serialized Distribution=None clears the UObject pointer, not the inherited
    cooked lookup payload. Retain nonempty authored distributions and all other
    data, including palette, timing, mesh TypeData and the source +X root basis.
    """
    instances = source.read(MODULE_EVIDENCE / 'source_module_inputs.json')['records']
    defaults = {r['fullPath']: r for r in source.read(MODULE_EVIDENCE / 'source_class_defaults.json')['records']}
    repairs = []
    for element in document['elements']:
        assert element['sourceNode'].split('|')[-1].startswith(SYSTEM + '.')
        for module in element['sourceRecipe']['modules']:
            for distribution in module['distributions']:
                field = distribution['propertyPath']
                if (module['className'], field) not in REPAIR_FIELDS:
                    continue
                if distribution['lookupTable'] or distribution['keys']:
                    continue
                instance = instances[module['objectPath']]
                delta = {k.casefold(): v for k, v in instance['properties'][field]['value']['properties'].items()}
                assert set(delta) == {'distribution'} and delta['distribution']['value'] == 0
                chain, key = [], instance.get('archetypeFullPath') or 'engine.default__' + module['className']
                while key:
                    record = defaults[key]
                    chain.append(record)
                    key = record.get('archetypeFullPath')
                inherited = {}
                for record in reversed(chain):
                    properties = {k.casefold(): v for k, v in record['properties'].items()}
                    if field in properties:
                        tag = properties[field]
                        assert tag['structType'].casefold().startswith('rawdistribution')
                        inherited.update({k.casefold(): v['value'] for k, v in tag['value']['properties'].items()})
                assert inherited.get('lookuptable') and any(inherited['lookuptable'])
                before = copy.deepcopy(distribution)
                for authored, raw in (('operation', 'op'), ('lookupTableNumElements', 'lookuptablenumelements'),
                        ('lookupTableChunkSize', 'lookuptablechunksize'), ('lookupTableTimeScale', 'lookuptabletimescale'),
                        ('lookupTableStartTime', 'lookuptablestarttime'), ('lookupTable', 'lookuptable')):
                    distribution[authored] = copy.deepcopy(inherited[raw])
                repairs.append(dict(elementId=element['id'], sourceModule=module['objectPath'], propertyPath=field,
                    sourceExportIndex=instance['exportIndex'], serializedDelta=delta,
                    inheritedClassChain=[r['fullPath'] for r in chain], before=before, after=copy.deepcopy(distribution)))
    return repairs


def stage_authored_repair(evidence):
    evidence = evidence.resolve()
    assert evidence.is_relative_to(ROOT / 'out')
    before = OUTPUT.read_bytes()
    document = json.loads(before)
    assert document['effectAssetId'] == ASSET and len(document['elements']) == 15
    repairs = repair_source_defaults(document)
    assert len(repairs) == 10, ('Unexpected current missing fields; preserve changed authoring', len(repairs))
    assert not document.get('sourceModelPreview'), 'Preserve an existing authored actor preview'
    document['sourceModelPreview'] = breath_model_preview()
    candidate = evidence / 'candidate' / OUTPUT.name
    source.write(candidate, document)
    backup = evidence / 'baseline' / OUTPUT.name
    backup.parent.mkdir(parents=True, exist_ok=True)
    backup.write_bytes(before)
    composition_path = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
    tree_path = ROOT / 'Data/Effects/EffectResourceTree.json'
    composition, tree = source.read(composition_path), source.read(tree_path)
    resource = next(r for r in composition['presentationResources'] if r['assetId'] == ASSET)
    parent = next(r['parentId'] for r in tree['references'] if r['kind'] == 'V1' and r['assetId'] == ASSET)
    reference_revision = next(a['referenceRevision'] for p in composition['patterns']
        for s in p['stages'] for a in s['animationOccurrences'] if a['profileId'] == 'MN_RPCT_07')
    source.write(evidence / 'pattern_template.json', dict(
        operation='CREATE_NEW_PATTERN_WITH_FRESH_STABLE_IDS', displayName='기분나빠 | 브레스',
        actorProfileId='MN_RPCT_05', gateId='GATE3', targetBossPlacementId='boss.kakulsaydon.g3.saydon',
        category='MECHANIC', authoringStatus='DRAFT', resetBossToSpawn=False,
        stage=dict(stageKind='ACTIVE', durationMs=5167, animation=dict(profileId='MN_RPCT_07',
            sourceActionId=4219917, sourceStageId='stage-000', sourceSlotId='animation-000',
            referenceRevision=reference_revision, runtimeClip='rpct00_att_battle_31_01',
            startOffsetMs=0, sourceStartMs=0, playMs=5167, playRate=1, endPolicy='EXACT')),
        presentationOccurrence=dict(resourceId=resource['resourceId'], startMs=1989, durationMs=3000,
            positionOffset=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1],
            fadeInMs=0, fadeOutMs=0, dissolveStart=1, dissolveEnd=1, brightnessMultiplier=1, volume=1,
            followBoss=False, debugRender=True, bone='', boneTarget='BODY', regionId='', cardSymbol='NONE',
            cardColor='NONE', anchorKind='BOSS', worldId='', logicOccurrenceId='', worldOccurrenceId=''),
        sourceBasis='Source SNAPSHOT_ROOT at notify 1.989096999 seconds; effect normalizes source +X to owner +Z once.',
        existingPatternIdsUnchanged=[f'KAKULSAYDON_G1_PATTERN_{i}' for i in (44, 50, 51, 52, 53, 54, 55)],
        compositionInputSha256=hashlib.sha256(composition_path.read_bytes()).hexdigest()))
    source.write(evidence / 'source_default_repairs.json', dict(repairs=repairs,
        cause='SERIALIZED_DISTRIBUTION_NONE_REPLACED_INHERITED_COOKED_FIELDS', sourceSystem=SYSTEM,
        appearancePreserved=['native material palette and textures', 'all 15 elements and stable IDs',
            'root yaw -90 and authored occurrence transforms', 'notify duration and all clocks']))
    inputs = [OUTPUT, LIBRARY, ACTION_SOURCE, MODEL_CONTRACT, composition_path, tree_path,
        MODULE_EVIDENCE / 'source_module_inputs.json', MODULE_EVIDENCE / 'source_class_defaults.json', Path(__file__)]
    source.write(evidence / 'installation.json', dict(installed=False, stageOnly=True, documents=[dict(
        effectAssetId=ASSET, displayName=document['displayName'], path=OUTPUT.relative_to(ROOT).as_posix(),
        candidatePath=candidate.relative_to(ROOT).as_posix(), beforeSha256=hashlib.sha256(before).hexdigest(),
        candidateSha256=hashlib.sha256(candidate.read_bytes()).hexdigest(), durationMs=resource['durationMs'],
        defaultAnchorKind=resource['defaultAnchorKind'], resourceId=resource['resourceId'], parentId=parent)],
        inputHashes={p.relative_to(ROOT).as_posix() if p.is_relative_to(ROOT) else p.as_posix():
                     hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs},
        sourceActionId=4219917, sourceNotifyId='action-4219917/stage-000/notify-008',
        savedConsumers=[dict(patternId=p['patternId'], occurrenceId=o['occurrenceId']) for p in composition['patterns']
                        for o in p['presentationOccurrences'] if o['resourceId'] == resource['resourceId']],
        repairedFields=len(repairs), elements=15, nativeShaderChanges=False,
        manualVisualValidation='USER_PENDING', clientRun=False))
    print(json.dumps(dict(staged=True, repairedFields=len(repairs), candidate=str(candidate))))


def receipt(path):
    return dict(path=path.resolve().as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def extend_authored_duration(document, duration_ms=AUTHORED_DURATION_MS):
    """Extend continuous emission while retaining each original particle tail.

    This is the user's authored total window, not an original source duration.
    A single loop preserves the initial bursts; increasing particle lifetimes
    would also change motion, size and color curves over normalized age.
    """
    assert document['effectAssetId'] == ASSET and len(document['elements']) == 15
    assert isinstance(duration_ms, int) and 0 < duration_ms <= 600000
    result = copy.deepcopy(document)
    seconds = duration_ms / 1000
    rows = []
    for before, element in zip(document['elements'], result['elements']):
        recipe, timing = element['sourceRecipe'], element['detail']['timing']
        particle = element['detail']['particle']
        assert recipe['enabled'] and recipe['emitterLoopCount'] == 1
        spawn = next(m for m in recipe['modules'] if m['className'] == 'particlemodulespawn')
        rate = next(d for d in spawn['distributions'] if d['propertyPath'] == 'rate')
        values = rate['lookupTable']
        assert len(values) == 4 and len(set(values)) == 1 and not rate['keys'], 'Reassess a changed spawn curve'
        tail = max(particle['lifeTimeSeconds']) * particle.get('sourceScale', {}).get('lifeTime', 1)
        offset = timing['startDelaySeconds'] + recipe['emitterDelaySeconds'] + timing['afterImageSeconds']
        old_duration = recipe['emitterDurationSeconds']
        if values[0] > 0:
            duration = seconds - offset - tail
            assert duration >= old_duration - 1e-6, 'Requested window would shorten an emitter'
            recipe['emitterDurationSeconds'] = duration
            required = next(m for m in recipe['modules'] if m['className'] == 'particlemodulerequired')
            literal = next(v for v in required['literals'] if v['propertyPath'] == 'emitterduration')
            assert abs(literal['value'] - old_duration) < 1e-6
            literal['value'] = duration
        else:
            assert recipe['bursts'] and all(b['timeSeconds'] == 0 for b in recipe['bursts'])
            assert offset + old_duration + tail <= seconds
        timing['lifeTimeSeconds'] = seconds
        rows.append(dict(elementId=element['id'], continuous=values[0] > 0,
            beforeEmissionSeconds=old_duration, afterEmissionSeconds=recipe['emitterDurationSeconds'],
            delaySeconds=recipe['emitterDelaySeconds'], particleTailSeconds=tail,
            beforeEndSeconds=offset + old_duration + tail,
            afterEndSeconds=offset + recipe['emitterDurationSeconds'] + tail))
    assert sum(row['continuous'] for row in rows) == 13
    preview = result['sourceModelPreview']['animations']
    assert len(preview) == 1 and preview[0]['runtimeClip'] == 'rpct00_att_battle_31_01'
    assert preview[0]['sourceStartMs'] == 1989 and preview[0]['endPolicy'] == 'HOLD_LAST_POSE'
    preview[0]['playMs'] = duration_ms
    assert abs(max(row['afterEndSeconds'] for row in rows) - seconds) < 1e-6
    return result, rows


def stage_authored_duration(evidence, duration_ms=AUTHORED_DURATION_MS):
    """Stage a CAS candidate and preserve the current authoring byte layout."""
    evidence = evidence.resolve()
    assert evidence.is_relative_to(ROOT / 'out')
    before = OUTPUT.read_bytes()
    document = json.loads(before)
    candidate, rows = extend_authored_duration(document, duration_ms)
    text = before.decode('utf-8')
    number = r'[-+0-9.eE]+'
    def replace_values(pattern, values):
        nonlocal text
        matches = list(re.finditer(pattern, text))
        assert len(matches) == len(values), 'Authoring layout changed; do not rewrite unrelated fields'
        for match, value in reversed(list(zip(matches, values))):
            old = text[match.start(2):match.end(2)]
            if float(old) != value:
                text = text[:match.start(2)] + json.dumps(value) + text[match.end(2):]
    replace_values(r'("timing"\s*:\s*\{[^{}]*?"lifeTimeSeconds"\s*:\s*)(' + number + ')',
        [duration_ms / 1000] * len(rows))
    replace_values(r'("emitterDurationSeconds"\s*:\s*)(' + number + ')',
        [row['afterEmissionSeconds'] for row in rows])
    replace_values(r'("propertyPath"\s*:\s*"emitterduration"\s*,\s*"kind"\s*:\s*"number"\s*,\s*"value"\s*:\s*)(' + number + ')',
        [row['afterEmissionSeconds'] for row in rows])
    replace_values(r'("playMs"\s*:\s*)(' + number + ')', [duration_ms])
    after = text.encode('utf-8')
    assert json.loads(after) == candidate
    assert OUTPUT.read_bytes() == before, 'Authored effect changed while staging'
    baseline, target = evidence / 'baseline' / OUTPUT.name, evidence / 'candidate' / OUTPUT.name
    baseline.parent.mkdir(parents=True, exist_ok=True)
    target.parent.mkdir(parents=True, exist_ok=True)
    baseline.write_bytes(before)
    target.write_bytes(after)
    source.write(evidence / 'duration_projection.json', dict(durationMs=duration_ms,
        policy='Continuous emission plus original particle tail ends at the requested total duration; initial-only bursts remain single.',
        sourceArchiveUnchanged=True, originalSourceNotifyUnchanged=True, rows=rows))
    source.write(evidence / 'installation.json', dict(installed=False, stageOnly=True,
        path=OUTPUT.relative_to(ROOT).as_posix(), candidatePath=target.relative_to(ROOT).as_posix(),
        beforeSha256=hashlib.sha256(before).hexdigest(), candidateSha256=hashlib.sha256(after).hexdigest(),
        durationMs=duration_ms, elements=15, sustainedEmitters=13, initialOnlyEmitters=2,
        compositionWritten=False, clientRun=False, manualVisualValidation='USER_PENDING',
        previewBoundary='Source clip and sourceStartMs=1989 remain unchanged; existing HOLD_LAST_POSE owns the remaining authored window.'))
    print(json.dumps(dict(staged=True, candidate=str(target), durationMs=duration_ms)))


def breath_model_preview():
    preview = source.source_model_preview(ACTION_SOURCE, 4219917, [0], 'GATE3',
        'MN_RPCT_05', 'boss.kakulsaydon.g3.saydon')
    assert len(preview['animations']) == 1
    animation = preview['animations'][0]
    assert animation['runtimeClip'] == 'rpct00_att_battle_31_01' and animation['playMs'] == 5167
    # This independent asset begins at the source breath notify, not the actor's
    # preparation. Composition's full actor clip is a separate authored clock.
    animation.update(sourceStartMs=1989, playMs=3000)
    return preview


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

    default_repairs = repair_source_defaults(doc)
    assert len(default_repairs) == 10
    doc['sourceModelPreview'] = breath_model_preview()
    doc, authored_duration_rows = extend_authored_duration(doc)

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
    source.write(EVIDENCE / 'source_default_repairs.json', dict(repairs=default_repairs))
    candidate = EVIDENCE / 'candidate' / OUTPUT.name
    source.write(candidate, doc)
    if install:
        source.write(OUTPUT, doc)
    # The source clock above remains recorded independently from this explicit
    # user-authored extension. Rebuilding must retain the accepted total window.
    elements = doc['elements']
    duration = round(max(row['afterEndSeconds'] for row in authored_duration_rows) * 1000)
    assert duration == AUTHORED_DURATION_MS
    source.write(EVIDENCE / 'installation.json', dict(installed=install, documents=[dict(
        effectAssetId=ASSET, displayName=TITLE, path=OUTPUT.relative_to(ROOT).as_posix(), elementCount=len(elements),
        durationMs=duration, sourceParticleSystem=SYSTEM, sourceActionId=4219917, sourceProfileId='MN_RPCT_07',
        sourceNotifyId=notify['notifyId'], categoryPath=['KoukuSaydon', '3관문', '패턴', '세이튼', '분신 소환·기분나빠'],
        defaultAnchorKind='BOSS', followBoss=False, sourceTransform=cue['localTransform'], sourceStartSeconds=notify['localTimeSeconds'],
        sourceDurationSeconds=notify['durationSeconds'], referenceImageIdentity='USER_VISUAL_COMPARISON_PENDING')],
        sourceEvidence=dict(inputs=[receipt(path) for path in [LIBRARY, ACTION_SOURCE, MODEL_CONTRACT,
            MODULE_EVIDENCE / 'source_module_inputs.json', MODULE_EVIDENCE / 'source_class_defaults.json', Path(__file__)]],
            identityMapping='User label is mapped to the recovered Madness Mark breath; the source action does not contain the user label.',
            derivation='Original enabled root notify instantiated through the existing decoder and parameter projector; independent start zero; source offset, nonuniform scale, particle tails and native palette retained. User-authored total window is extended to 4279ms through continuous emission only.',
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
    parser.add_argument('--repair-authored', action='store_true', help='Stage only the missing source defaults in current authored data')
    parser.add_argument('--extend-authored-duration-ms', type=int, help='Stage a duration-only CAS candidate from current authoring')
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuCloneBreath20260914')
    args = parser.parse_args()
    if args.extend_authored_duration_ms is not None:
        assert not args.install and not args.repair_authored, 'Authored duration changes use a separate CAS installer'
        stage_authored_duration(args.evidence_root, args.extend_authored_duration_ms)
    elif args.repair_authored:
        assert not args.install, 'The current authored repair uses a separate CAS installer'
        stage_authored_repair(args.evidence_root)
    else:
        derive(args.install)
