"""Prepare field-preserving Kouku effect candidates; never install authoring data."""
from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / 'Data/Effects/Authored'
OUTPUT = ROOT / 'out/KoukuEffectRecovery20260925'
GHOST = 'par_l_rpct_05_sk_13_1_loc_int.particlespriteemitter_0'


def write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf8')


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def restore_ghosts(doc):
    ghost, = [e for e in doc['elements'] if e['sourceNode'].endswith(GHOST)]
    assert ghost['visible'] and not ghost['detail']['particle']['localSpace']
    assert ghost['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-2999-native.v1'
    scale = ghost['detail']['particle']['sourceScale']
    # 8 m/s, the original 30 cm SpawnPerUnit, and 168.75 ms retain five births.
    # These are explicit user appearance tuning, not newly inferred source values.
    scale['lifeTime'] = 1.125
    ghost['detail']['color']['multiply'][3] = .5


def follow_dice_axes(doc):
    for element in doc['elements']:
        axis_locks = [v['value'] for m in element['sourceRecipe']['modules']
                      for v in m['literals'] if v['propertyPath'] == 'lockaxisflags']
        if any(v in ('epal_rotate_x', 'epal_rotate_y', 'epal_rotate_z', 'epal_x', 'epal_y', 'epal_z') for v in axis_locks):
            element['detail']['sprite']['followEmitterAxisRotation'] = True


def hold_blackhole(doc):
    center = '.center.' in doc['effectAssetId']
    for element in doc['elements']:
        source = element['sourceNode'].split('|')[0]
        timing, recipe = element['detail']['timing'], element['sourceRecipe']
        if '/stage-003/' in source:
            timing['startDelaySeconds'] = 0
            if center:
                if recipe['emitterDurationSeconds'] >= 9:
                    recipe['emitterDurationSeconds'] = 13
                    timing['lifeTimeSeconds'] = 13
                if element['detail']['particle']['lifeTimeSeconds'][1] >= 9:
                    element['detail']['particle']['sourceScale']['lifeTime'] = 1.3
            elif '/notify-001' in source:
                # The original ray is a short pulse emitter. Repeat that pulse
                # through the requested hold, preserving each particle's curve.
                recipe['emitterLoopCount'] = 0
                timing['lifeTimeSeconds'] = 13
        elif '/stage-004/' in source:
            timing['startDelaySeconds'] = 13
        elif '/stage-006/' in source:
            timing['startDelaySeconds'] = 13.2 + max(0, timing['startDelaySeconds'] - 15.933)
        else:
            raise ValueError('Unexpected blackhole stage: ' + source)
    preview = doc.get('sourceModelPreview')
    if preview:
        animations = []
        for animation in preview['animations']:
            if animation['runtimeClip'] == 'rpct00_att_battle_12_02':
                continue
            if animation['runtimeClip'] == 'rpct00_att_battle_12_04':
                animation.update(startOffsetMs=0, playMs=13000, endPolicy='LOOP_TO_WINDOW')
            else:
                animation['startOffsetMs'] += 13000 - 15733
            animations.append(animation)
        preview['animations'] = animations


def tune_contact(doc):
    doc['particleSystem']['uniformScaleMultiplier'] *= 1.5
    for element in doc['elements']:
        element['detail']['transform']['position'][1] += .5


def patch_composition(document):
    """Return latest saved document with only requested stable rows/fields changed."""
    result = copy.deepcopy(document)
    patterns = {p['patternId']: p for p in result['patterns']}
    definitions = {p['logicId']: p for p in result['logics']}
    resources = {p['resourceId']: p for p in result['presentationResources']}
    p78 = patterns['KAKULSAYDON_G1_PATTERN_78']
    original = definitions['kakulsaydon.g1.logic.73']
    assert original['judgementKind'] == 'PURSUIT_PROJECTILES'
    for index, ordinal in enumerate(range(125, 129)):
        identity = f'kakulsaydon.g1.logic.{ordinal}'
        target = definitions[identity]
        for key, value in original.items():
            if key not in ('logicId', 'displayName', 'logicType', 'judgementKind', 'spawnIntervalMs'):
                target[key] = copy.deepcopy(value)
        target.update(logicType='TRIGGER', triggerKind='PURSUIT_PROJECTILES',
                      visualIds=[original['visualIds'][index]], cardSymbols=[original['cardSymbols'][index]])
    p78['logicOccurrences'] = [o for o in p78['logicOccurrences'] if o['logicId'] != original['logicId']]
    for occurrence in p78['presentationOccurrences']:
        if resources[occurrence['resourceId']].get('assetId') == 'effect.kouku.card.match.emit.full.restore':
            occurrence['rotationDegrees'][1] += 180
    # Isolate the dice impact tuning from the spinning card's shared impact.
    contact = copy.deepcopy(resources[original['contactVisualId']])
    contact['resourceId'] = 'kakulsaydon.effect.card-dice-contact'
    contact['displayName'] = '주사위 추적카드 접촉 폭발'
    contact['assetId'] = 'effect.kouku.card.match.contact.recovery'
    if contact['resourceId'] not in resources:
        result['presentationResources'].append(contact)
    for ordinal in range(125, 129):
        definitions[f'kakulsaydon.g1.logic.{ordinal}']['contactVisualId'] = contact['resourceId']
    medusa = patterns['KAKULSAYDON_G1_PATTERN_94']
    medusa['animationRootVerticalScale'] = 0
    face_resource = next(r for r in result['presentationResources']
                         if r.get('assetId') == 'effect.kouku.bingo.medusa.face.full.restore')
    if not any(o['resourceId'] == face_resource['resourceId'] for o in medusa['presentationOccurrences']):
        ordinal = medusa['nextPresentationOccurrenceOrdinal']
        face_id = f'KAKULSAYDON_G1_PATTERN_94.presentation.{ordinal}'
        face = copy.deepcopy(next(o for o in medusa['presentationOccurrences']
                                  if resources[o['resourceId']].get('assetId') == 'effect.kouku.bingo.medusa.attack.full.restore'))
        face.update(occurrenceId=face_id, resourceId=face_resource['resourceId'], startMs=3167, durationMs=3261)
        medusa['presentationOccurrences'].append(face)
        medusa['nextPresentationOccurrenceOrdinal'] = ordinal + 1
    p11 = patterns['KAKULSAYDON_G1_PATTERN_11']
    red_end = max(o['startMs'] + o['durationMs'] for o in p11['presentationOccurrences']
                  if resources[o['resourceId']].get('assetId') == 'boss.kouku.medusa.red')
    for occurrence in p11['presentationOccurrences']:
        if resources[occurrence['resourceId']].get('assetId') == 'boss.kouku.medusa.blue':
            occurrence['durationMs'] = red_end - occurrence['startMs']
    return result


def patch_catalog(document):
    result = copy.deepcopy(document)
    asset = 'effect.kouku.card.match.contact.recovery'
    if not any(row['effectAssetId'] == asset for row in result['effects']):
        result['effects'].append(dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT',
                                     authoringPath=f'Effects/Authored/{asset}.effect.json'))
    return result


def patch_tree(document):
    result = copy.deepcopy(document)
    asset = 'effect.kouku.card.match.contact.recovery'
    if not any(row.get('assetId') == asset for row in result['references']):
        source = next(row for row in result['references'] if row.get('assetId') == 'effect.kouku.card.match.explosion')
        result['references'].append(dict(source, assetId=asset, displayName='주사위 추적카드 접촉 폭발'))
    return result


def prepare():
    entries = []
    def stage(asset, change, new_asset=None):
        source = AUTHORED / (asset + '.effect.json')
        raw = source.read_bytes()
        before = json.loads(raw)
        candidate = copy.deepcopy(before)
        change(candidate)
        if new_asset:
            candidate['effectAssetId'] = new_asset
            candidate['displayName'] = '주사위 추적카드 접촉 폭발'
        target = OUTPUT / 'candidate' / ((new_asset or asset) + '.effect.json')
        backup = OUTPUT / 'before' / source.name
        backup.parent.mkdir(parents=True, exist_ok=True)
        backup.write_bytes(raw)
        write(target, candidate)
        assert source.read_bytes() == raw, 'Saved source changed while preparing candidate'
        entries.append(dict(sourcePath=source.relative_to(ROOT).as_posix(),
                            beforeSha256=digest(raw),
                            candidatePath=target.relative_to(ROOT).as_posix(),
                            targetPath=(AUTHORED / target.name).relative_to(ROOT).as_posix(),
                            candidateSha256=digest(target.read_bytes()), newAsset=bool(new_asset)))
    for suit in ('heart', 'spade', 'clover', 'diamond'):
        stage('effect.kouku.card.spinning.' + suit, restore_ghosts)
    stage('effect.kouku.card.match.emit.full.restore', follow_dice_axes)
    for suffix in ('full.restore', 'center.full.restore'):
        stage('effect.kouku.bingo.blackhole.' + suffix, hold_blackhole)
    composition_path = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
    composition = json.loads(composition_path.read_bytes())
    resource_id = next(l for l in composition['logics'] if l['logicId'] == 'kakulsaydon.g1.logic.73')['contactVisualId']
    contact_asset = next(r for r in composition['presentationResources'] if r['resourceId'] == resource_id)['assetId']
    stage(contact_asset, tune_contact, 'effect.kouku.card.match.contact.recovery')
    write(OUTPUT / 'composition.candidate.json', patch_composition(composition))
    catalog_entries = []
    for name, patch in (('EffectCatalog.json', patch_catalog), ('EffectResourceTree.json', patch_tree)):
        source = ROOT / 'Data/Effects' / name
        raw = source.read_bytes()
        target = OUTPUT / 'metadata' / name
        backup = OUTPUT / 'before' / name
        backup.write_bytes(raw)
        write(target, patch(json.loads(raw)))
        catalog_entries.append(dict(sourcePath=source.relative_to(ROOT).as_posix(),
                                    beforeSha256=digest(raw), candidatePath=target.relative_to(ROOT).as_posix(),
                                    targetPath=source.relative_to(ROOT).as_posix(),
                                    candidateSha256=digest(target.read_bytes()), newAsset=False))
    write(OUTPUT / 'manifest.json', dict(installed=False, documents=entries,
          metadataDocuments=catalog_entries,
          compositionPatchFunction='Tools/EffectPipeline/prepare_kouku_effect_recovery.py:patch_composition',
          blackholeHoldDurationMs=13000, blackholeExplosionAtMs=13000,
          blackholePrepHasSourceEffects=False, renderingSettingsChanged=False))
    print(json.dumps(dict(candidates=len(entries), manifest=str(OUTPUT / 'manifest.json'))))


if __name__ == '__main__':
    prepare()
