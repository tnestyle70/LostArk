"""Stage original charge dust, arm trails and streaks with the existing body cue."""
import copy
from pathlib import Path

from build_saydon_card_pattern_groups import (AUTHORED, ROOT, STAGES, current_preview,
                                             project_mesh_rotation, read, renamed, write)
from build_kouku_dove_pizza_candidates import native_references, registration, sha

ASSET = 'effect.kouku.gate1.charge_counter.group'
FX_ASSET = 'effect.kouku.gate1.charge_counter.fx'
FX_RESOURCE = 'kakulsaydon.effect.charge.counter.fx'
OUTPUT = ROOT / 'out/EffectV1ChargeCounter20260917/candidate'
COMPOSITION = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
ACTION = Path('C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_05.action-effects.json')
AFTERIMAGE = ROOT / 'out/EffectV1Afterimage20260917/candidates/effect.kouku.gate1.charge_counter.afterimage.effect.json'


def build():
    preview = current_preview(80)
    dash = [a for a in preview['animations'] if a['runtimeClip'] == 'rpct00_att_battle_22_04']
    assert len(dash) == 1 and dash[0]['sourceStartMs'] == 0 and dash[0]['playRate'] == 1
    stage = read(STAGES / 'effect.kouku.action.mn_rpct_05.4219863.stage001.effect.json')
    for element in stage['elements']:
        emitter = element['sourceNode'].split('|')[-1]
        source_doc = read(AUTHORED / ('effect.kouku.source.' + emitter.rsplit('.', 1)[0] + '.effect.json'))
        source_element = next(e for e in source_doc['elements'] if e['sourceNode'].endswith('|' + emitter))
        if element['kind'] == 'light':
            # Notify color/radius overrides already reside in the projected light.
            assert source_element['kind'] == 'light'
            continue
        assert source_element['material']['sourceProfile']['enabled']
        assert source_element['material']['sourceMaterialPath'] == element['material']['sourceMaterialPath']
        element['material'] = copy.deepcopy(source_element['material'])
        element['resources'] = copy.deepcopy(source_element['resources'])
        if 'runtimeCarrier' in source_element:
            element['runtimeCarrier'] = copy.deepcopy(source_element['runtimeCarrier'])
        project_mesh_rotation(element)
    document = renamed(stage, ASSET, '돌진 카운터 · 잔상과 돌진 이펙트')
    document['version'] = 15
    document.setdefault('runtimeExtensions', dict(formatVersion=1, bakedEdgeHistories=[]))
    document['sourceModelPreview'] = preview
    document['modelCues'] = copy.deepcopy(read(AFTERIMAGE)['modelCues'])
    for element in document['elements']:
        element['detail']['timing']['startDelaySeconds'] += dash[0]['startOffsetMs'] / 1000
        if 'sourcePresentation' in element:
            element['sourcePresentation']['sourceTimeSeconds'] += dash[0]['startOffsetMs'] / 1000
    assert len(document['elements']) == 21
    actions = read(ACTION)['actions']
    relevant = [a for a in actions if a['actionId'] in (4219863, 4219866)]
    counter = [dict(actionId=a['actionId'], **n) for a in relevant for s in a['stages']
               for n in s['notifies'] if n['sourceType'] == 'CounterAttack']
    charge = next(a for a in relevant if a['actionId'] == 4219863)
    emitters = [n for n in charge['stages'][1]['notifies']
                if n['sourceType'] in ('PlayParticleEffect', 'TrailGhostEffect', 'PawnMaterialParam')]
    source_assets = sorted({'effect.kouku.source.' + e['sourceNode'].split('|')[-1].rsplit('.', 1)[0]
                            for e in document['elements']})
    required = native_references(document)
    write(OUTPUT / (ASSET + '.effect.json'), document)
    # Product already owns the actor and its body/weapon afterimage helper.
    # Keep the standalone V1 cue bundle separate from the Pattern FX document.
    fx = renamed(document, FX_ASSET, '돌진 카운터 · 원본 먼지와 양팔 궤적')
    fx['modelCues'] = []
    write(OUTPUT / (FX_ASSET + '.effect.json'), fx)
    composition = read(COMPOSITION)
    pattern = next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_80')
    pattern_duration = sum(s['durationMs'] for s in pattern['stages'])
    resource = dict(resourceId=FX_RESOURCE, displayName=fx['displayName'], defaultAnchorKind='BOSS',
        kind='EFFECT', assetId=FX_ASSET, resourceKind='V1_EFFECT', elementId='', durationMs=5199,
        shape='BOX', colliderKind='GEOMETRY', halfExtents=[1, 1, 1], radiusM=3, halfAngleDegrees=45)
    occurrence = dict(occurrenceId=pattern['patternId'] + '.presentation.charge.fx',
        resourceId=FX_RESOURCE, startMs=0, durationMs=pattern_duration, positionOffset=[0, 0, 0],
        rotationDegrees=[0, 0, 0], scale=[1, 1, 1], fadeInMs=0, fadeOutMs=0,
        dissolveStart=1, dissolveEnd=1, brightnessMultiplier=1, volume=1, followBoss=True,
        debugRender=True, bone='', boneTarget='BODY', regionId='', cardSymbol='NONE',
        cardColor='NONE', anchorKind='BOSS', worldId='', logicOccurrenceId='', worldOccurrenceId='',
        fitEffectToDuration=False, loopEffectToDuration=False)
    write(OUTPUT / 'charge-pattern-fx.receipt.json', dict(assetId=FX_ASSET, installed=False,
        candidateSha256=sha(OUTPUT / (FX_ASSET + '.effect.json')),
        derivedFromAssetId=ASSET, modelCueCount=0, particleElementCount=21,
        sourceModelPreviewPolicy='Metadata retained for standalone source-bone preview. Product uses the actual P80 animation history and actor via Make_SourceAnchorSampler.',
        duplicationPolicy='Only this FX-only asset is attached to P80. The live CNpc body/weapon afterimage helper remains the sole Product afterimage owner; the V1 group and previous afterimage candidate must not also be added.',
        existingPresentationOccurrences=copy.deepcopy(pattern['presentationOccurrences']),
        playbackConsumer='CKoukuSaydonPresentationPlayer::Sample_Session -> CEffectPresentationService::Spawn_LevelPlacement + externally sampled source anchors',
        bodyAfterimageConsumer='Charge_AfterimageActive -> CNpc::Set_ChargeAfterimageEnabled -> body/weapon CSkeletalAfterimage',
        tailPolicy=dict(standaloneDurationMs=5199, patternDurationMs=pattern_duration,
            particleTailClippedByPatternEnd=pattern_duration < 5199,
            note='Keep the saved animation chain and pattern end. No time stretch and no full-tail completion claim.'),
        compositionPatch=dict(sourceSha256=sha(COMPOSITION), sourceRevision=composition['revision'],
            presentationResource=resource, resourceOperation='ADD_IF_ABSENT_OR_IDENTICAL',
            patternOperations=[dict(patternId=pattern['patternId'], operation='ADD_IF_ABSENT_ASSET',
                occurrence=occurrence)]),
        **registration(fx, 'effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dash_atk_01_01_loc_int')))
    write(OUTPUT / 'charge-counter.receipt.json', dict(assetId=ASSET, installed=False,
        candidateSha256=sha(OUTPUT / (ASSET + '.effect.json')),
        sourceActionSha256=sha(ACTION), bodyCueCandidateSha256=sha(AFTERIMAGE),
        sourceInputs=[dict(path='Data/Effects/Authored/' + asset + '.effect.json',
            sha256=sha(AUTHORED / (asset + '.effect.json'))) for asset in source_assets],
        sourceChargeNotifies=emitters, sourceCounterNotifies=counter,
        nativeAssets=required, sourceModelPreview=preview,
        originalParticleScope='3 dust + 6 left arm + 6 right arm + 1 light + 5 dash streak elements; original notify TRS/socket/material/particle recipes retained.',
        bodyAfterimageScope='Existing PROJECT_AUTHORED skinned body cue. Original TrailGhost timing only; material appearance is not asserted identical to the original.',
        priorAfterimageCandidate=dict(path=str(AFTERIMAGE.relative_to(ROOT)), unchanged=True,
            relationship='The group copies its two body cues and adds original attached FX on the same clock. Register the new group independently; do not overwrite the prior afterimage candidate.'),
        playbackScope='Standalone V1 PlayAll bundle only. Do not attach this modelCue group to live P80: its CNpc body/weapon afterimage helper already owns those samples.',
        patternFxOnlyCandidate=dict(assetId=FX_ASSET, receipt='charge-pattern-fx.receipt.json',
            modelCueCount=0, relationship='Same original particle recipes and source anchors, with no body/afterimage model cues; this is the only new P80 charge FX occurrence.'),
        counterRingStatus='NOT_IDENTIFIED: CounterAttack notify has no ParticleSystem reference; default EFActionNotify_CounterAttack properties are empty. Common FX_BS_00 counter hit systems are impact effects, not proven readiness ring assets.',
        manualVisualValidation='USER_PENDING',
        **registration(document, 'effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_dash_atk_01_01_loc_int')))
    print(ASSET, len(document['elements']))


if __name__ == '__main__':
    build()
