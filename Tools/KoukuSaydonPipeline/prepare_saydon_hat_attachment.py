"""Prepare the saved Saydon hat window through the existing WORLD prop path.

This command only writes candidates/evidence. The owner publisher installs the
latest stable-ID merge; it never edits a running tool's in-memory draft.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
COMPOSITION = Path('Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json')
WORLD = Path('Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json')
PATTERN_ID = 'KAKULSAYDON_G1_PATTERN_83'
LOGIC_ID = PATTERN_ID + '.logic.1'
LOGIC_DEFINITION_ID = 'kakulsaydon.g1.logic.81'
OBJECT_ID = 'world.object.kouku.saydon_hat_right'
TEMPLATE_ID = 'sequence.LV_LUT_MIDNIGHTC_ED.world_object.saydon_hat_right'
INSTANCE_ID = 'world.object.instance.kouku.saydon_hat_right'
HAND_MODEL = 'Character/KoukuSaton/WP_MN_RPCT_08/wp_mn_rpct_08_sk.wmodel'


def read(path: Path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path: Path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def prepare(output: Path):
    composition = read(ROOT / COMPOSITION)
    world = read(ROOT / WORLD)
    pattern = next(row for row in composition['patterns'] if row['patternId'] == PATTERN_ID)
    logic = next(row for row in pattern['logicOccurrences'] if row['occurrenceId'] == LOGIC_ID)
    definition = next(row for row in composition['logics'] if row['logicId'] == logic['logicId'])
    if definition['logicId'] != LOGIC_DEFINITION_ID:
        raise ValueError('The saved hat occurrence now uses another Logic; preserve it for review.')
    if definition['logicType'] != 'DURATION' or not logic['enabled']:
        raise ValueError('The saved hat owner must be an enabled DURATION window.')
    if definition.get('judgementKind', '') not in ('', 'ATTACHMENT_HOLD'):
        raise ValueError('The saved hat Logic has another judgementKind; preserve the conflicting field.')
    consumers = [(owner['patternId'], occurrence['occurrenceId'])
        for owner in composition['patterns'] for occurrence in owner.get('logicOccurrences', [])
        if occurrence['logicId'] == LOGIC_DEFINITION_ID]
    if consumers != [(PATTERN_ID, LOGIC_ID)]:
        raise ValueError('The hat Logic is shared outside its one measured occurrence; preserve all consumers.')
    definition_before = copy.deepcopy(definition)
    definition_after = dict(definition, judgementKind='ATTACHMENT_HOLD')
    if pattern['actorProfileId'] != 'MN_RPCT_05':
        raise ValueError('The saved hat owner no longer targets the measured Saydon body.')
    start, duration = logic['startMs'], logic['durationMs']
    if duration <= 0 or start + duration > pattern['durationMs']:
        raise ValueError('The saved hat interval does not fit inside its Pattern.')
    if any(row['objectId'] == OBJECT_ID for row in world['objectResources']):
        raise ValueError('Hat resource already exists; review its saved fields before updating.')
    title = definition['displayName']
    resource = dict(objectId=OBJECT_ID, displayName=title, modelAssetId=HAND_MODEL,
        anchorKind='BOSS', diffuseTextureAssetId='', modelPreScale=0.01, animated=True,
        scale=[1.7, 1.7, 1.7], sequenceInstanceId='',
        anchorBossArchetypeId='BOSS_KAKULSAYDON_G1_SAYDON', anchorBone='b_wp_1',
        defaultMotionInstanceId=INSTANCE_ID, materialSourceModelAssetId=HAND_MODEL)
    key = dict(timeMs=0, positionOffset=[0, 0, 0], rotationQuaternion=[0, 0, 0, 1],
        scaleMultiplier=[1, 1, 1], visible=True)
    end_key = dict(key, timeMs=duration)
    template = dict(sequenceId=TEMPLATE_ID, displayName=title, category='WorldObject',
        durationMs=duration, interpolation='LINEAR', objectMotion=dict(velocity=[0, 0, 0],
        acceleration=[0, 0, 0], angularVelocityDegrees=[0, 0, 0], revolutionDegreesPerSecond=[0, 0, 0],
        revolutionOffset=[0, 0, 0], count=1, intervalMs=0, spreadDegrees=0, seed=1),
        tracks=[dict(slotId='object', keys=[key, end_key])], animationTracks=[])
    instance = dict(instanceId=INSTANCE_ID, templateId=TEMPLATE_ID, enabled=True,
        startDelayMs=0, playbackSpeed=1, anchorKind='BOSS', position=[0, 0, 0], motionEnd='HOLD',
        nextMotionId='', bindings=[dict(slotId='object', targetKind='OBJECT_RESOURCE', targetId=OBJECT_ID)])
    world_id = 'kakulsaydon.g1.world.' + str(composition['nextWorldOrdinal'])
    world_definition = dict(worldId=world_id, displayName=title, sequenceInstanceId=INSTANCE_ID,
        positionOffset=[0, 0, 0], anchorKind='NONE', anchorPosition=[0, 0, 0], companionEffectResourceId='',
        objectResourceId=OBJECT_ID)
    occurrence = dict(occurrenceId=PATTERN_ID + '.world.' + str(pattern['nextWorldOccurrenceOrdinal']),
        worldId=world_id, startMs=start, durationMs=duration, playbackSpeed=1)
    patch = dict(schema='lostark.saydon-hat-candidate.v1', patternId=PATTERN_ID,
        sourceLogicOccurrence=copy.deepcopy(logic), compositionRevision=composition['revision'],
        sourceLogicDefinition=copy.deepcopy(definition_before),
        logicDefinitionBefore=definition_before, logicDefinitionAfter=definition_after,
        worldRevision=world['revision'], sourceHashes={str(path):hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
            for path in [COMPOSITION, WORLD]}, objectResource=resource, template=template, instance=instance,
        worldDefinition=world_definition, worldOccurrence=occurrence,
        installed=False, runtimeBinaryUpdated=False, visualValidation='USER_PENDING')
    world['objectResources'].append(resource); world['templates'].append(template); world['instances'].append(instance)
    world['revision'] += 1
    composition['worlds'].append(world_definition); composition['nextWorldOrdinal'] += 1
    definition['judgementKind'] = 'ATTACHMENT_HOLD'
    pattern['worldOccurrences'].append(occurrence); pattern['nextWorldOccurrenceOrdinal'] += 1
    composition['revision'] += 1
    write(output/'hat.patch.json', patch)
    write(output/'candidate'/COMPOSITION, composition)
    write(output/'candidate'/WORLD, world)
    print(f'Candidate only: {PATTERN_ID} {start}..{start+duration} ms, {occurrence["occurrenceId"]}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT/'out/SaydonHatRestoration20260917')
    prepare(parser.parse_args().output)
