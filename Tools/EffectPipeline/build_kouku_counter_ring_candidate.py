"""Copy the user-confirmed blue mesh ring; keep its saved Valtan source intact."""
import copy
import hashlib
from build_saydon_card_pattern_groups import AUTHORED, ROOT, read, renamed, write
from build_kouku_dove_pizza_candidates import native_references, sha

ASSET = 'effect.kouku.common.counter.ring'
RESOURCE = 'kakulsaydon.effect.counter.ring'
SOURCE = AUTHORED / 'effect.valtan.project-tuned.sequence.trash.effect.json'
OUTPUT = ROOT / 'out/EffectV1ChargeCounter20260917/candidate'
COMPOSITION = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'


def build():
    original = read(SOURCE)
    assert len(original['elements']) == 1 and original['elements'][0]['id'] == 'mesh_particle_11'
    document = renamed(original, ASSET, '카운터 이펙트')
    document['elements'][0]['displayName'] = '사용자 파란 메시 링'
    document['modelCues'] = []
    document.pop('sourceModelPreview', None)
    assert document['elements'][0]['detail']['timing']['startDelaySeconds'] == 0
    assert document['elements'][0]['detail']['timing']['lifeTimeSeconds'] == .6
    required = native_references(document)
    composition = read(COMPOSITION)
    resource = dict(resourceId=RESOURCE, displayName='카운터 이펙트', defaultAnchorKind='BOSS',
        kind='EFFECT', assetId=ASSET, resourceKind='V1_EFFECT', elementId='', durationMs=617,
        shape='BOX', colliderKind='GEOMETRY', halfExtents=[1, 1, 1], radiusM=3, halfAngleDegrees=45)
    occurrences = []
    for ordinal, start, end in [(80, 0, 950), (81, 300, 2200)]:
        pattern = next(p for p in composition['patterns'] if p['patternId'] == f'KAKULSAYDON_G1_PATTERN_{ordinal}')
        occurrences.append(dict(patternId=pattern['patternId'], operation='ADD_IF_ABSENT_ASSET',
            sourceCounterWindowMs=[start, end],
            visualTimingPolicy='Play the unchanged 600ms user ring once at the CounterAttack window start. The 617ms occurrence includes the measured 1/60s birth delay; no loop or time stretch.',
            occurrence=dict(occurrenceId=pattern['patternId'] + '.presentation.counter.ring',
                resourceId=RESOURCE, startMs=start, durationMs=617, positionOffset=[0, 0, 0],
                rotationDegrees=[0, 0, 0], scale=[1, 1, 1], fadeInMs=0, fadeOutMs=0,
                dissolveStart=1, dissolveEnd=1, brightnessMultiplier=1, volume=1, followBoss=True,
                debugRender=True, bone='', boneTarget='BODY', regionId='', cardSymbol='NONE',
                cardColor='NONE', anchorKind='BOSS', worldId='', logicOccurrenceId='', worldOccurrenceId='')))
    category = dict(id='kouku.category.' + hashlib.sha256('세이튼_카운터'.encode()).hexdigest()[:20],
        parentId='kouku.category.d9def6e601e406cf41f1', kind='CATEGORY', displayName='세이튼_카운터')
    write(OUTPUT / (ASSET + '.effect.json'), document)
    write(OUTPUT / 'counter-ring.receipt.json', dict(assetId=ASSET, installed=False,
        candidateSha256=sha(OUTPUT / (ASSET + '.effect.json')),
        sourceInputs=[dict(path=str(SOURCE.relative_to(ROOT)), sha256=sha(SOURCE))],
        sourceElementId='mesh_particle_11', userSelection='B_CONFIRMED',
        nativeAssets=required, preserved='Every saved visual field, resource, material, particle count, size, color, local TRS and 600ms lifetime; only stable identity/display names differ.',
        sourceUnchanged=True, sourceNameHistory='The actual saved displayName is Project Tuned / Trash / Floor And Hand Composite. Trash is part of the authored name, not evidence of a deleted-bin state. No source deletion or move; no verified history of why this name was chosen.',
        catalogEntry=dict(effectAssetId=ASSET, payloadKind='DIRECT_AUTHORED_DOCUMENT',
            authoringPath=f'Effects/Authored/{ASSET}.effect.json'),
        treeCategory=category, treeReference=dict(kind='V1', assetId=ASSET,
            displayName='카운터 이펙트', parentId=category['id']),
        compositionPatch=dict(sourceSha256=sha(COMPOSITION), sourceRevision=composition['revision'],
            presentationResource=resource, patternOperations=occurrences),
        sharedKoukuSpiderReplacementOwner='server_patterns', manualVisualValidation='USER_PENDING'))
    print(ASSET, 'sourceLifetimeMs=600 resourceDurationMs=617')


if __name__ == '__main__':
    build()
