"""Stage Bingo Medusa and blackhole from their actual source actions.

The current Composition owns Medusa's edited clip windows. Blackhole uses the
original 4219927 charge/ray/ball/explosion and groggy sequence; 4219983 is an eye
laser and remains a separate resource. This command never installs candidates.
"""
from pathlib import Path
import argparse
import base64
import collections
import copy
import hashlib
import math
import sys
import struct

import build_kouku_action_effect_groups as action

source = action.source
ROOT = source.ROOT


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def build(output, loa_root, library):
    from extract_action_effect_notifies import extract_action_document
    sys.path[:0] = [str(ROOT / 'Tools/CharacterCustomizing'),
                   str(ROOT / 'Tools/ModelAssetConverter')]
    from align_rig_gltf import read_body_skeleton
    from retime_wmodel_from_psa import read_wmodel_animation_sections

    composition_path = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
    composition = source.read(composition_path)
    sounds = {r['soundEvent'].lower(): r for r in composition['presentationResources']
              if r['kind'] == 'SOUND' and r.get('soundEvent')}
    medusa = next(p for p in composition['patterns']
                  if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_94')
    specs = []
    for stage in medusa['stages']:
        assert len(stage['animationOccurrences']) == 1
        clip = stage['animationOccurrences'][0]
        assert clip['sourceActionId'] == 42198102 and clip['sourceStartMs'] == 0
        assert clip['playRate'] == 1 and clip['startOffsetMs'] == 0
        specs.append((int(clip['sourceStageId'].split('-')[1]),
                      stage['durationMs'], clip['runtimeClip']))
    assert [s[0] for s in specs] == [0, 1, 2]
    jobs = [dict(asset='effect.kouku.bingo.medusa.attack.full.restore',
                 name='빙고 | 메두사 공격', profile='MN_RPCT_05', action=42198102,
                 patternId=medusa['patternId'], stages=specs),
            dict(asset='effect.kouku.bingo.blackhole.full.restore',
                 name='빙고 | 블랙홀·폭발·그로기', profile='MN_RPCT_07', action=4219927,
                 patternId='KAKULSAYDON_G1_PATTERN_107', stages=[
                     (2, 5400, 'rpct00_att_battle_12_02'),
                     (3, 10333, 'rpct00_att_battle_12_04'),
                     (4, 200, 'rpct00_att_battle_12_05'),
                     (6, 1167, 'rpct00_dmg_critical_start_1'),
                     (7, 5100, 'rpct00_dmg_critical_loop_1'),
                     (5, 1333, 'rpct00_dmg_critical_end_1')])]
    index = action.restored_index(library)
    occurrences = collections.defaultdict(list)
    for row in source.read(library / 'source_occurrences.json'):
        occurrences[row['sourceSystem']].append(row)
    model = ROOT / 'Client/Bin/Resources/Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'
    bones = dict(read_body_skeleton(model))
    clips = {c['name'] for c in read_wmodel_animation_sections(model.read_bytes())}
    contract_path = ROOT / 'out/KoukuActionEffects20260912/MN_RPCT_07.model_contract.json'
    sockets = copy.deepcopy(source.read(contract_path)['sourceSockets'])
    # The installed RPCT skeleton's original PSK basis was measured as .01*ReflectZ.
    # Apply that conversion only to the socket used by these source notifies.
    for socket in sockets['sockets']:
        if socket.get('socketName', '').lower() not in ('fx_buff_01', 'fx_hit_01'):
            continue
        transform = socket['sourceTransform']
        assert transform['rotationUnrealUnits'] == [0, 0, 0]
        x, y, z = transform['positionUeUnits']
        socket['runtimeLocalTransform'] = dict(position=[x * .01, -y * .01, z * .01],
                                              rotationDegrees=[-90, 0, 0], scale=[1, 1, 1])
    entries, templates, evidence_rows = [], [], []
    input_hashes = {str(composition_path.relative_to(ROOT)): digest(composition_path),
                    str(contract_path.relative_to(ROOT)): digest(contract_path),
                    str(model.relative_to(ROOT)): digest(model)}
    for job in jobs:
        loa = loa_root / (job['profile'] + '.loa')
        extracted = extract_action_document(loa, job['profile'], None, None,
                                           {job['action']}, None)
        source.write(output / 'source' / (job['profile'] + '.action-effects.json'), extracted)
        original = extracted['actions'][0]
        elements, animations, stages, audio = [], [], [], []
        offset = 0
        for stage_index, duration, runtime_clip in job['stages']:
            assert runtime_clip in clips, runtime_clip
            stage = next(s for s in original['stages'] if s['stageIndex'] == stage_index)
            stage_row = dict(sourceStageIndex=stage_index, durationMs=duration,
                             runtimeClip=runtime_clip, startMs=offset)
            stages.append(stage_row)
            animations.append(dict(runtimeClip=runtime_clip, startOffsetMs=offset,
                sourceStartMs=0, playMs=duration, playRate=1,
                endPolicy='LOOP_TO_WINDOW' if runtime_clip == 'rpct00_dmg_critical_loop_1' else 'HOLD_LAST_POSE'))
            for notify in stage['notifies']:
                if notify['sourceType'] == 'AKEvent':
                    raw = base64.b64decode(notify['serializedPayload']['data'])
                    prefix = b'CEFActionNotify_AKEvent\0'
                    assert raw.startswith(prefix)
                    enabled = struct.unpack_from('<I', raw, len(prefix) + 12)[0]
                    assert enabled in (0, 1)
                    if enabled:
                        event = next(r['objectPath'].lower() for r in notify['assetReferences']
                                     if r['className'].lower() == 'akevent')
                        resource = sounds.get(event)
                        audio.append(dict(sourceNotify=notify['notifyId'], soundEvent=event,
                            startMs=offset + round(notify['localTimeSeconds'] * 1000),
                            durationMs=max(1, round(notify['durationSeconds'] * 1000))
                                if notify['durationSeconds'] > 0 else resource['durationMs'] if resource else 0,
                            resourceId=resource['resourceId'] if resource else None,
                            sourceDurationSeconds=notify['durationSeconds']))
                if notify['sourceType'] != 'PlayParticleEffect':
                    continue
                cue = action.decode_notify(notify, sockets, bones)
                row = dict(actionId=job['action'], notifyId=notify['notifyId'],
                           enabled=cue['enabled'], sourceTimeSeconds=notify['localTimeSeconds'],
                           sourceDurationSeconds=notify['durationSeconds'])
                evidence_rows.append(row)
                if not cue['enabled']:
                    continue
                systems = [r['objectPath'].lower() for r in notify['assetReferences']
                           if r['className'].lower() == 'particlesystem']
                assert len(systems) == 1
                system = systems[0]
                path = ROOT / 'Data/Effects/Authored' / ('effect.kouku.source.' + system + '.effect.json')
                template = source.read(path)
                input_hashes[str(path.relative_to(ROOT))] = digest(path)
                light_rows = [r for r in occurrences[system] if r['rendererShape'] == 'light']
                if light_rows:
                    lights = action.project_light_occurrences(index, light_rows, cue, notify, output)
                    template['elements'] = [e for e in template['elements'] if e['kind'] != 'light'] + lights['elements']
                projected, parameters = action.instantiate(template, cue, notify, job['asset'], job['profile'], index)
                for element in projected:
                    element['groupId'] = 'manual.' + job['asset'] + '.' + notify['notifyId'].replace('/', '.')
                    element['detail']['timing']['startDelaySeconds'] += offset / 1000
                    element['sourcePresentation']['sourceTimeSeconds'] += offset / 1000
                    if element['kind'] == 'screenPost':
                        assert element['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-3327-native.v1'
                        element['detail']['screenPost'].update(enabled=True,
                            profileId='screen.film-noise.reconstructed.v1', intensity=1)
                        life = max(element['detail']['particle']['lifeTimeSeconds'])
                        element['detail']['timing']['lifeTimeSeconds'] = life
                        element['sourceRecipe']['emitterDurationSeconds'] = life
                elements.extend(projected)
                row.update(system=system, elementCount=len(projected), parameters=parameters,
                           attachment=cue['attachment'], localTransform=cue['localTransform'])
            offset += duration
        document = dict(schema='lostark.effect-authoring', version=13, effectAssetId=job['asset'],
            displayName=job['name'], particleSystem=dict(uniformScaleMultiplier=1,
                yawOffsetDegrees=0, directionYawDegrees=0, initialSpeedMultiplier=1),
            modelCues=[], elements=elements, sourceModelPreview=dict(gateId='BINGO',
                actorProfileId='MN_RPCT_05', targetBossPlacementId='boss.kakulsaydon.bingo.saydon',
                animations=animations))
        assert len(elements) == len({e['id'] for e in elements})
        candidate = output / 'candidate' / (job['asset'] + '.effect.json')
        source.write(candidate, document)
        def end_seconds(element):
            recipe, detail = element['sourceRecipe'], element['detail']
            timing = detail['timing']
            active = recipe['emitterDurationSeconds'] * recipe['emitterLoopCount'] \
                if recipe['emitterLoopCount'] else timing['lifeTimeSeconds']
            tail = max(detail['particle']['lifeTimeSeconds']) if element['kind'] == 'particle' else 0
            return timing['startDelaySeconds'] + recipe['emitterDelaySeconds'] + active + timing['afterImageSeconds'] + tail
        effect_duration = math.ceil(max(map(end_seconds, elements)) * 1000)
        # AKEvent's editor notify span is not the admitted one-shot media length.
        # Match the existing SOUND pipeline: play the event through the owner
        # window, including its authored particle tail, without repeating it.
        owner_duration = max(offset, effect_duration)
        for cue in audio:
            resource = sounds.get(cue['soundEvent'])
            if resource:
                cue['durationMs'] = min(resource['durationMs'], owner_duration - cue['startMs'])
        entries.append(dict(effectAssetId=job['asset'], displayName=job['name'],
            path='Data/Effects/Authored/' + candidate.name, candidatePath=str(candidate.resolve()),
            durationMs=effect_duration, animationDurationMs=offset,
            elementCount=len(elements), defaultAnchorKind='BOSS',
            categoryPath=['KoukuSaydon', '빙고', job['name'].split(' | ')[1]],
            sourceActionId=job['action'], sourceProfileId=job['profile']))
        templates.append(dict(patternId=job['patternId'], effectAssetId=job['asset'],
            sourceActionId=job['action'], sourceProfileId=job['profile'], stages=stages,
            animations=animations, durationMs=offset, effectStartMs=0,
            effectDurationMs=effect_duration, sounds=audio))
        if job['action'] == 4219927:
            # The explicit Bingo target is a fixed map position, independently
            # of the caster. Split complete source notify groups, never shaders
            # or particle modules. The Composition owns the absolute position.
            central_notifies = ('stage-003.notify-006', 'stage-004.notify-003', 'stage-004.notify-006')
            central = [e for e in document['elements'] if e['groupId'].endswith(central_notifies)]
            body = [e for e in document['elements'] if e not in central]
            assert len(central) == 40 and len(body) == 24
            center_asset = 'effect.kouku.bingo.blackhole.center.full.restore'
            center = copy.deepcopy(document)
            center.update(effectAssetId=center_asset, displayName='빙고 | 블랙홀 중앙 구체·폭발', elements=central)
            for element in central:
                element['id'] = element['id'] + '.center'
                element['groupId'] = element['groupId'].replace(job['asset'], center_asset)
                # Cancel only the original 500cm forward notify offset. The
                # original Y, TRS scale, source basis and emitter motion survive.
                assert element['detail']['transform']['position'][0] == 5.0
                element['detail']['transform']['position'][0] = 0.0
            document.update(displayName='빙고 | 블랙홀 시전자·광선·그로기', elements=body)
            source.write(candidate, document)
            center_path = output / 'candidate' / (center_asset + '.effect.json')
            source.write(center_path, center)
            body_duration = math.ceil(max(map(end_seconds, body)) * 1000)
            center_duration = math.ceil(max(map(end_seconds, central)) * 1000)
            entries[-1].update(displayName=document['displayName'], elementCount=len(body), durationMs=body_duration,
                               pairedEffectAssetId=center_asset)
            entries.append(dict(effectAssetId=center_asset, displayName=center['displayName'],
                path='Data/Effects/Authored/' + center_path.name, candidatePath=str(center_path.resolve()),
                durationMs=center_duration, animationDurationMs=offset, elementCount=len(central),
                defaultAnchorKind='MAP', categoryPath=['KoukuSaydon', '빙고', '블랙홀 중앙 구체·폭발'],
                sourceActionId=job['action'], sourceProfileId=job['profile'], pairedEffectAssetId=job['asset']))
            templates[-1]['effectOccurrences'] = [
                dict(effectAssetId=job['asset'], startMs=0, durationMs=body_duration,
                     anchorKind='BOSS', followBoss=True, positionOffset=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1]),
                dict(effectAssetId=center_asset, startMs=0, durationMs=center_duration,
                     anchorKind='MAP', followBoss=False, positionOffset=[-0.600000024, 0, 1147.43994],
                     rotationDegrees=[0, 0, 0], scale=[1, 1, 1])]
    source.write(output / 'installation.json', dict(installed=False, documents=entries,
        inputHashes=input_hashes, modelSha256=digest(model), manualVisualValidation='USER_PENDING'))
    source.write(output / 'pattern-templates.json', templates)
    source.write(output / 'source-occurrence-evidence.json', evidence_rows)
    print([(d['effectAssetId'], d['elementCount'], d['durationMs']) for d in entries])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuBingoRestore20260923')
    parser.add_argument('--loa-root', type=Path, default=ROOT / 'out/KoukuAllEffects20260912/source/Action')
    parser.add_argument('--library', type=Path, default=ROOT / 'out/KoukuAllEffects20260912')
    args = parser.parse_args()
    build(args.output, args.loa_root, args.library)
