"""Restore Event_02's original 5.5-second phase-two Matinee in the saved pattern.

The original cinematic starts at the saved Att_Battle_12_03 +400ms boundary.
Its finite duration changes only the cinematic recovery stage; FX durations do
not modify any stage. Existing gameplay phase/wall authority events stay intact.
"""
from pathlib import Path
import argparse
import copy
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT/'Tools/ValtanPipeline'), str(ROOT/'Tools/EffectPipeline')]
import bake_valtan_original_cinematic_actors as bake
import restore_valtan_original_cameras as camera_tools
import build_valtan_full_restore as writer


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, required=True)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    args.evidence_root.mkdir(parents=True, exist_ok=True)
    source = bake.source
    scene_path = bake.SCENES/'LV_LUT_HEARTRB_ED_SCENE02A.json'
    scene = source.read(scene_path)
    rows = {int(k): v for k, v in scene['rows'].items()}
    assert rows[104]['p']['interplength'] == 5.5
    assert source.group_actor(rows, 135, 52) == [74]
    assert rows[74]['p']['lookinfokey'].lower() == 'efdlchar_mn_rpbf_01.mn_rpbf_01'
    assert rows[399]['p']['objvalue'] == 20 and source.group_actor(rows, 126, 52) == [20]
    # The source Kismet SetCameraTarget373 selects CameraActor20 directly.
    # There is no Director cut: construct its equivalent one-shot sampling view.
    rows[999999] = dict(cls='interptrackdirector', p=dict(cuttrack=[dict(time=0, targetcamgroup='c1')]))
    rows[184]['p']['interptracks'].append(999999)
    shots = [dict(startMs=a, endMs=b, shot=x) for a, b, x in source.make_cameras(
        rows, 52, 104, 5500, 'valtan.source.phase2', 'Valtan original phase2')]
    source_frames = camera_tools.frames(dict(shots=shots))
    files, writes = {}, []

    def document(relative):
        path = ROOT/relative
        before = path.read_bytes()
        value = json.loads(before)
        files[path] = [before, value]
        return value

    gameplay = document('Data/Valtan/Valtan.gameplay.json')
    presentation = document('Data/Valtan/Valtan.presentation.json')
    cameras = document('Data/Encounters/Valtan/ValtanCinematicCamera.json')
    sequence = document('Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json')
    rootmotion = document('Data/Animation/RootMotion/Valtan.rootmotion.json')
    cliptemplates = document('Data/Valtan/Valtan.cliptemplates.json')
    # The restored cinematic is no longer the shared landing clip, so its
    # former post-impact landing-contact waiver would be stale provenance.
    cliptemplates['allowlist'] = [row for row in cliptemplates['allowlist']
        if row['clipOccurrenceId'] != 'valtan.mechanic.arena-break-109.wide-reveal.clip.01']
    gp = next(p for p in gameplay['patterns'] if p['patternId'] == 'VALTAN_ARENA_BREAK_109')
    pp = next(p for p in presentation['patterns'] if p['patternId'] == gp['patternId'])
    recovery = next(s for s in gp['stages'] if s['stageId'] == 'RECOVERY')
    assert recovery['durationMs'] in (870, 2700)
    recovery['durationMs'] = 2700
    for p in rootmotion['patterns']:
        if p['patternId'] != gp['patternId']:
            continue
        for stage in p['stages']:
            if stage['stageId'] == 'RECOVERY' and stage['durationMs'] == 870:
                stage['durationMs'] = 2700
                stage['samples'].append(dict(stage['samples'][-1], timeMs=2700))

    clip_name = 'valtan.cinematic.phase2'
    tree, tree_receipt = bake.inherited_cinematic_tree()
    model_receipts = []
    for asset in bake.OUTPUT.values():
        path = bake.RESOURCES/asset
        before = path.read_bytes()
        model = bake.wm.read_wmodel(path)
        existing = next((a for a in model.animations if a.name == clip_name), None)
        if existing:
            assert abs(existing.duration_ticks/existing.ticks_per_second - 5.5) < .00001
            continue
        times, poses, detail = bake.bake(model, rows, 135, 5.5, tree)
        trailer = next(payload[-8:] for kind, _, _, payload in bake.sections(before) if kind == 4)
        payload = bake.animation_payload(model, times, poses, trailer, bake.wm.FILE_HEADER.unpack_from(before)[2])
        after = bake.append(before, [(clip_name, payload)])
        candidate = args.evidence_root/'candidate'/asset
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(after)
        checked = bake.wm.read_wmodel(candidate)
        assert len(checked.animations) == len(model.animations) + 1
        writes.append((path, before, after))
        model_receipts.append(dict(asset=asset, preservedAnimationCount=len(model.animations),
            animationCount=len(checked.animations), candidateSha256=bake.sha(after), **detail))

    source_origin = source.world_pose(rows, 135, 74, 0, 52, 104)[0]
    anchor = source.np.array(gp['serverMotion']['landingPosition'])
    shift = anchor - source_origin
    for key in source_frames:
        for field in ('eye', 'lookAt'):
            key[field] = (source.np.array(key[field]) + shift).tolist()
    original_presentation = json.loads((args.evidence_root/'backup/Data/Valtan/Valtan.presentation.json').read_bytes()) if (args.evidence_root/'backup/Data/Valtan/Valtan.presentation.json').exists() else copy.deepcopy(presentation)
    original_pattern = next(p for p in original_presentation['patterns'] if p['patternId'] == gp['patternId'])
    previous_receipt = json.loads((args.evidence_root/'receipt.json').read_bytes()) if (args.evidence_root/'receipt.json').exists() else {}
    clip_offsets = {}
    for stage_id, start, end in [('IMPACT_HOLD', 0, 500), ('WIDE_REVEAL', 500, 2800), ('RECOVERY', 2800, 5500)]:
        stage = next(s for s in pp['stages'] if s['stageId'] == stage_id)
        cue = next(c for c in cameras['cues'] if c['patternId'] == gp['patternId'] and c['stageId'] == stage_id)
        keys = camera_tools.cut_keys(source_frames, start, end, cue['cueId'])
        if stage_id == 'IMPACT_HOLD':
            # Preserve the existing first600ms camera and clip prefix; source
            # Event_02 begins after12_02 tail200ms +12_03 animation400ms.
            old = copy.deepcopy(cue['keyframes'])
            for key in old:
                key.setdefault('up', [0, 1, 0])
            prefix = camera_tools.cut_keys(old, 0, 599, cue['cueId'] + '.prefix')
            for key in keys:
                key['timeMs'] += 600
            keys[0]['cutBefore'] = True
            keys = prefix + keys
            occurrences = stage['animation']['occurrences']
            if not any(o['clip'] == clip_name for o in occurrences):
                assert len(occurrences) == 2 and occurrences[0]['playMs'] == 200
                occurrences[1]['playMs'] = 400
                occurrences.append(dict(clipOccurrenceId=stage['actionId']+'.original-phase2.clip', clip=clip_name,
                    mappingBasis='SOURCE_REVIEWED_DELTA', sourceStartMs=0, playMs=500, playRate=1., repeatUntilStageEnd=False))
            duration = 1100
        else:
            originals = next(s for s in original_pattern['stages'] if s['stageId'] == stage_id)['animation']['occurrences']
            current = {o['clipOccurrenceId']: o for o in stage['animation']['occurrences']}
            rebuilt, offset = [], start
            for ordinal, old in enumerate(originals):
                duration = old['playMs'] if stage_id == 'WIDE_REVEAL' else end-start
                previous = current.get(old['clipOccurrenceId'], old) if previous_receipt.get('cueOffsetsApplied') else old
                clip_offsets[old['clipOccurrenceId']] = offset - previous['sourceStartMs']
                rebuilt.append(dict(old, clip=clip_name, mappingBasis='SOURCE_REVIEWED_DELTA',
                    sourceStartMs=offset, playMs=duration, playRate=1., repeatUntilStageEnd=False))
                offset += duration
            assert offset == end
            stage['animation']['occurrences'] = rebuilt
            duration = end-start
        stage['animation'].update(endPolicy='EXACT', repeatCount=1)
        cue.update(durationMs=duration, interpolation='LINEAR', easing='LINEAR', keyframes=keys,
            trackingMode='BOSS_XZ', trackingOrigin=anchor.tolist())
        stage['cameraInvocations'][0]['durationMs'] = duration

    for leaf in ('patternsoundcues', 'patternshakecues'):
        cues = document('Data/Animation/Authored/Valtan/Valtan.' + leaf + '.json')
        for cue in cues['cues']:
            cue['startMs'] += clip_offsets.get(cue['clipOccurrenceId'], 0)

    resource_id = 'world.object.valtan.source-preview.phase2'
    resource = copy.deepcopy(next(r for r in sequence['objectResources'] if r['objectId'].endswith('.body')))
    resource.update(objectId=resource_id, displayName='Valtan original phase2 actor74', scale=[1.3]*3)
    if not any(r['objectId'] == resource_id for r in sequence['objectResources']):
        sequence['objectResources'].append(resource)
    keys = []
    for ms in source.source_times(rows, 135, 0, 5500, 33):
        position, rotation = source.world_pose(rows, 135, 74, ms/1000, 52, 104)
        quaternion = source.Rotation.from_matrix(rotation).as_quat()
        if quaternion[3] < 0:
            quaternion = -quaternion
        keys.append(dict(timeMs=ms, positionOffset=(position-source_origin).tolist(),
            rotationQuaternion=quaternion.tolist(), scaleMultiplier=[1, 1, 1], visible=True))
    identity = 'sequence.LV_LUT_HEARTRB_ED.valtan.source-preview.phase2'
    template = dict(sequenceId=identity, displayName='Valtan original phase2 / Event_02', category='Cutscene',
        durationMs=5500, interpolation='LINEAR', tracks=[dict(slotId='actor', keys=keys)],
        animationTracks=[dict(slotId='actor', clipName=clip_name, startMs=0, sourceStartMs=0,
            playbackRate=1., loop=False, holdLastFrame=True)])
    previous = next((t for t in sequence['templates'] if t['sequenceId'] == identity), None)
    if previous is None:
        sequence['templates'].append(template)
        sequence['instances'].append(dict(instanceId='world.sequence.instance.valtan.source-preview.phase2',
            templateId=identity, enabled=True, startDelayMs=0, playbackSpeed=1., anchorKind='WORLD',
            position=anchor.tolist(), motionEnd='STOP', nextMotionId='',
            bindings=[dict(slotId='actor', targetKind='OBJECT_RESOURCE', targetId=resource_id)]))
    else:
        assert all(previous[k] == value for k, value in template.items()), 'Preserve edited phase2 actor lane'
    previous_sequence = json.loads(files[ROOT/'Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json'][0])
    if sequence != previous_sequence:
        sequence['revision'] += 1
    for path, (before, value) in files.items():
        after = writer.encode_json(value)
        if path.name == 'Valtan.cliptemplates.json':
            text = before.decode('utf8')
            after = re.sub(r',?\s*\{[^{}]*"clipOccurrenceId": "valtan\.mechanic\.arena-break-109\.wide-reveal\.clip\.01"[^{}]*\}',
                           '', text).encode('utf8')
            assert json.loads(after) == value
        writes.append((path, before, after))
    for path, before, after in writes:
        backup = args.evidence_root/'backup'/path.relative_to(ROOT)
        backup.parent.mkdir(parents=True, exist_ok=True)
        if not backup.exists():
            backup.write_bytes(before)
    receipt = dict(sourceSceneSha256=scene['sha256'], sourceAction=420629, sourceSignal='Event_02',
        sourceMatinee=52, sourceActor=74, sourceCamera=20, patternStartMs=2600, durationMs=5500,
        patternEndMs=8100, sourceToProductTranslation=shift.tolist(), sourceDrawScale=rows[74]['p']['drawscale'],
        models=model_receipts or previous_receipt.get('models', []), inheritedTree=tree_receipt,
        cueOffsetsApplied=True, visualStatus='USER_PENDING')
    (args.evidence_root/'receipt.json').write_bytes(writer.encode_json(receipt))
    if args.install:
        writer.commit_writes(writes)
    print(json.dumps(dict(installed=args.install, cinematicDurationMs=5500, patternEndMs=8100)))


if __name__ == '__main__':
    main()
