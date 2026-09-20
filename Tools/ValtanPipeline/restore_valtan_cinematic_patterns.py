"""Join verified original cinematic AnimSets to the existing Valtan pattern path.

Stable stage/occurrence IDs stay intact.  Source-time cue offsets are translated
with their animation segment so their existing stage-relative timing survives.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import json
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
import build_valtan_full_restore as writer


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--candidate-root', type=Path, required=True)
    parser.add_argument('--evidence-root', type=Path, required=True)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    receipt = json.loads((args.candidate_root / 'receipt.json').read_bytes())
    files = {}

    def document(relative):
        path = ROOT / relative
        before = path.read_bytes()
        value = json.loads(before)
        files[path] = [before, value]
        return value

    catalog = document('Data/Actors/BossCatalog.json')
    sequence = document('Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json')
    presentation = document('Data/Valtan/Valtan.presentation.json')
    gameplay = document('Data/Valtan/Valtan.gameplay.json')
    camera = document('Data/Encounters/Valtan/ValtanCinematicCamera.json')
    writes = []
    for model, archetype, object_id in zip(receipt['models'], ['BOSS_VALTAN', 'BOSS_VALTAN_GHOST'], ['body', 'ghost']):
        target = model['targetAssetId']
        payload = (args.candidate_root / target).read_bytes()
        assert hashlib.sha256(payload).hexdigest() == model['candidateSha256']
        assert model['preservedAnimationCount'] == 146 and model['animationCount'] == 149
        target_path = ROOT / 'Client/Bin/Resources' / target
        before = target_path.read_bytes() if target_path.exists() else None
        writes.append((target_path, before, payload))
        boss = next(b for b in catalog['bosses'] if b['archetypeId'] == archetype)
        assert boss['animationSetId'] in (model['sourceAssetId'], target)
        boss['animationSetId'] = target
        resource = next(r for r in sequence['objectResources'] if r['objectId'] == 'world.object.valtan.source-preview.' + object_id)
        assert resource['presentationBossArchetypeId'] == archetype
        resource['animationSetAssetId'] = target

    clip_offsets = {}
    joins = []
    for pattern_id, suffix, stage_ids in (
        ('VALTAN_ENTRANCE_CINEMATIC', 'entrance', ['ESTABLISH', 'ARENA_REVEAL', 'HERO_HANDOFF']),
        ('VALTAN_TRASH', 'trash', ['STEP_05', 'STEP_06']),
        ('VALTAN_GHOST_DEATH_AUDITION', 'finale', ['STEP_01']),
    ):
        gp = next(p for p in gameplay['patterns'] if p['patternId'] == pattern_id)
        pp = next(p for p in presentation['patterns'] if p['patternId'] == pattern_id)
        if suffix == 'finale':
            assert gp['compatibilitySelectionWeight'] == 0
            gp['stages'][0]['durationMs'] = 23000
        offset = 0
        for stage_id in stage_ids:
            stage = next(s for s in pp['stages'] if s['stageId'] == stage_id)
            duration = next(s['durationMs'] for s in gp['stages'] if s['stageId'] == stage_id)
            animation = stage['animation']
            assert len(animation['occurrences']) == 1 and not stage['effectCues']
            occurrence = animation['occurrences'][0]
            clip_offsets[occurrence['clipOccurrenceId']] = offset - occurrence['sourceStartMs']
            occurrence.update(clip='valtan.cinematic.' + suffix, mappingBasis='SOURCE_REVIEWED_DELTA', sourceStartMs=offset,
                playMs=duration, playRate=1.0, repeatUntilStageEnd=False)
            animation.update(endPolicy='EXACT', repeatCount=1)
            if suffix == 'finale':
                cue = copy.deepcopy(camera['deathCue'])
                cue.update(cueId='camera.valtan.source.finale.audition', patternId=pattern_id, stageId=stage_id)
                for ordinal, key in enumerate(cue['keyframes']):
                    key['sceneId'] = cue['cueId'] + '.key.' + str(ordinal)
                camera['cues'] = [c for c in camera['cues'] if c['cueId'] != cue['cueId']] + [cue]
                stage['cameraInvocations'] = [dict(cameraInvocationId=cue['cueId'] + '.invocation', cameraCueId=cue['cueId'],
                    trigger='ENTER', startOffsetMs=0, durationPolicy='EXPLICIT', durationMs=duration)]
            joins.append(dict(patternId=pattern_id, stageId=stage_id, clip=occurrence['clip'], sourceStartMs=offset, playMs=duration))
            offset += duration
        for instance_suffix in ([suffix, 'entrance.colorless'] if suffix == 'entrance' else [suffix]):
            template = next(t for t in sequence['templates'] if t['sequenceId'] == 'sequence.LV_LUT_HEARTRB_ED.valtan.source-preview.' + instance_suffix)
            assert template['durationMs'] == offset
            template['animationTracks'] = [dict(slotId='actor', clipName='valtan.cinematic.' + suffix,
                displayName='Original Matinee blended animation', startMs=0, sourceStartMs=0, playbackRate=1.0, loop=False, holdLastFrame=True)]

    for leaf in ('patternsoundcues', 'patternshakecues'):
        cues = document('Data/Animation/Authored/Valtan/Valtan.' + leaf + '.json')
        for cue in cues['cues']:
            cue['startMs'] += clip_offsets.get(cue['clipOccurrenceId'], 0)
    previous_sequence = json.loads(files[ROOT / 'Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json'][0])
    if sequence != previous_sequence:
        sequence['revision'] += 1
    args.evidence_root.mkdir(parents=True, exist_ok=True)
    for path, (before, value) in files.items():
        after = writer.encode_json(value)
        (args.evidence_root / (path.name + '.before')).write_bytes(before)
        (args.evidence_root / (path.name + '.candidate')).write_bytes(after)
        writes.append((path, before, after))
    (args.evidence_root / 'pattern-joins.json').write_bytes(writer.encode_json(dict(joins=joins, sourceReceipt=receipt,
        consumer='Existing Server Complete Play -> Valtan stage camera -> WorldSequencePlayer', visualStatus='USER_PENDING')))
    if args.install:
        writer.commit_writes(writes)
    print(json.dumps(dict(installed=args.install, stageJoins=len(joins), verifiedAnimSets=len(receipt['models']))))


if __name__ == '__main__':
    main()
