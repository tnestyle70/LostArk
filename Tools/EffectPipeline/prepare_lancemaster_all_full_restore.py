"""Recover the bound LanceMaster Action stages for native full restore.

Reads installed Action extraction and package objects. The already reviewed V
and Alt V source evidence remains unchanged; no authored document is rewritten.
"""
from pathlib import Path
import argparse, copy, hashlib, json, sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/LevelPlacementExtractor'))
from build_action_cue_recipe import decode_typed_payload


def normalize_clip(value):
    return value.lower().removeprefix('flm_')


def read(path):
    return json.loads(path.read_bytes())


def write(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf8')


def prepare(evidence, action_path):
    action = read(action_path)
    bindings = read(ROOT / 'Data/Animation/Authored/LanceMaster/LanceMaster.skillbindings.json')
    actions = {row['actionId']: row for row in action['actions']}
    socket = read(evidence / 'source_socket_contract.json')
    stages, cues, rejected = [], [], []
    for binding in bindings['bindings']:
        skill = binding['skillId']
        if skill in (34610, 34630):
            continue
        clips = binding['clips']
        combo = any(isinstance(clip, list) for clip in clips)
        source_stages = actions[skill]['stages']
        last_stage = -1
        for slot, group in enumerate(clips):
            group_clips = group if isinstance(group, list) else [group]
            for local_index, clip_value in enumerate(group_clips):
                clip = clip_value['clip'] if isinstance(clip_value, dict) else clip_value
                candidates = [stage for stage in source_stages if stage['stageIndex'] > last_stage
                              and any(normalize_clip(c['clipName']) == normalize_clip(clip)
                                      for c in stage['animationClips'])]
                if not candidates:
                    raise ValueError(('No ordered source Action stage for bound clip', skill, clip))
                # Action base sequence precedes mutually exclusive tripod variants.
                # A repeated clip name must never union their particle notifies.
                source_stage = candidates[0]
                last_stage = source_stage['stageIndex']
                suffix = f'.ba{slot + 1}' if combo else ''
                if len(group_clips) > 1:
                    suffix += f'.clip{local_index + 1}'
                elif not combo and len(clips) > 1:
                    suffix += f'.clip{slot + 1}'
                asset = f'effect.lancemaster.skill.{skill}{suffix}.full.restore'
                stage = dict(skillId=skill, sourceStageIndex=last_stage, stageIndex=len(stages),
                             clipName=clip, comboStage=slot if combo else None,
                             effectAssetId=asset, sourceCandidates=[s['stageIndex'] for s in candidates],
                             selection='FIRST_ORDERED_BASE_ACTION_STAGE_EXACT_CLIP',
                             durationSeconds=max(c['lengthSeconds'] for c in source_stage['animationClips']
                                                 if normalize_clip(c['clipName']) == normalize_clip(clip)))
                stages.append(stage)
                for notify in source_stage['notifies']:
                    if notify['sourceType'] != 'PlayParticleEffect':
                        continue
                    try:
                        decoded = decode_typed_payload(notify['sourceType'], notify['serializedPayload'],
                                                       socket, notify['assetReferences'], notify['serializedLabels'])
                    except ValueError as error:
                        rejected.append(dict(skillId=skill, clipName=clip, sourceNotify=notify['notifyId'],
                                             reason='SOURCE_TYPED_PAYLOAD_UNSUPPORTED', detail=str(error)))
                        continue
                    if not decoded.get('enabled'):
                        rejected.append(dict(skillId=skill, clipName=clip, sourceNotify=notify['notifyId'],
                                             reason='SOURCE_ACTION_NOTIFY_DISABLED'))
                        continue
                    if not decoded.get('particleDataDecoded') or not decoded.get('parameterOverridesDecoded'):
                        if not notify['assetReferences']:
                            rejected.append(dict(skillId=skill, clipName=clip, sourceNotify=notify['notifyId'],
                                                 reason='SOURCE_NOTIFY_HAS_NO_PARTICLE_SYSTEM',
                                                 labels=notify['serializedLabels']))
                            continue
                        raise ValueError(('Active source payload is not decoded', notify['notifyId'], decoded))
                    cues.append(dict(skillId=skill, stageIndex=stage['stageIndex'], sourceEnabled=True,
                                     **copy.deepcopy(notify), decoded=decoded))
    write(evidence / 'bound_source_stages.json', stages)
    write(evidence / 'raw_ppe_inputs.json', cues)
    write(evidence / 'source_action_exclusions.json', rejected)
    print('Bound stages', len(stages), 'active particle calls', len(cues), 'disabled calls', len(rejected))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--evidence', type=Path, default=ROOT / 'out/LanceMasterAllRestore20260910')
    parser.add_argument('--action', type=Path, required=True)
    args = parser.parse_args()
    prepare(args.evidence, args.action)
