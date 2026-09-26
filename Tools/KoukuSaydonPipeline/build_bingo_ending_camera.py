"""Prepare the original SCENE01B final-ending camera tracks without installing.

P9 Sequence and P75 Action share the same twelve stable camera shot IDs. They
retain ten original Director cuts and the three historical partitions of Cam01.
A common source Slomo clock maps every pose and cut to elapsed milliseconds.
Reduction compares every integer elapsed millisecond against the original pose,
including camera roll, using the runtime's linear Eye/quaternion interpolation.
The result contains field patches and input hashes for a separate approved merge.
It does not restore DOF or rotational CameraShake, and never writes authoring data.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
import math
import sys
from pathlib import Path
import numpy as np
from scipy.spatial.transform import Rotation, Slerp
import build_gate2_intro_composition as base
from source_scene_clock import SourceSceneClock
ROOT = Path(__file__).resolve().parents[2]
DEFAULT_CAMERA_SOURCE = ROOT / 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json'
DEFAULT_SEQUENCE_SOURCE = ROOT / 'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json'
DEFAULT_ACTION_SOURCE = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'

def build_candidate(source: Path, cam_path: Path=DEFAULT_CAMERA_SOURCE, comp_path: Path=DEFAULT_SEQUENCE_SOURCE, action_path: Path=DEFAULT_ACTION_SOURCE) -> dict:
    """Read original inputs and return an unapplied, stable-ID camera candidate."""
    source = Path(source).resolve()
    cam_path = Path(cam_path).resolve()
    comp_path = Path(comp_path).resolve()
    action_path = Path(action_path).resolve()
    rows = {int(k): v for k, v in base.read(source)['rows'].items()}
    if rows[32]['cls'] != 'efseqact_matinee' or rows[45]['cls'] != 'interpdata':
        raise ValueError('Expected SCENE01B Matinee32 / InterpData45 source records')
    clock = SourceSceneClock.from_scene_rows(rows, 45)
    cam_raw = cam_path.read_bytes()
    comp_raw = comp_path.read_bytes()
    action_raw = action_path.read_bytes()
    camera = base.read(cam_path)
    composition = base.read(comp_path)
    existing = {s['shotId']: s for s in camera['shots']}
    pattern = next((x for x in composition['patterns'] if x['patternId'] == 'KAKULSAYDON_G1_PATTERN_9'))
    resources = {r['resourceId']: r for r in composition['presentationResources']}
    occurrences = {resources[o['resourceId']]['assetId']: o for o in pattern['presentationOccurrences'] if o['resourceId'] in resources and resources[o['resourceId']]['kind'] == 'CAMERA'}
    groups = {rows[g]['p'].get('groupname'): g for g in rows[45]['p']['interpgroups']}
    director = rows[95]['p']['cuttrack']
    # Keep the three historical Cam01 partitions, not new Director cuts.
    starts = [0.0, 1232.0, 2384.0] + [c['time'] * 1000 for c in director[1:]]
    ends = starts[1:] + [clock.source_duration_ms]
    shots = []
    patch = []
    audit = []
    windows = [(number, start, end) for number, (start, end) in enumerate(zip(starts, ends), 1)]
    for number, start, end in windows:
        cut = next((c for c in reversed(director) if c['time'] * 1000 <= start + 1e-06))
        group = groups[cut['targetcamgroup']]
        actor = base.group_actor(rows, group, 32)[0]
        fov_tracks = [r['p'] for r in base.active_tracks(rows, group) if r['cls'] == 'interptrackfloatprop' and r['p'].get('propertyname') == 'fovangle']
        wall_start = round(clock.to_elapsed_ms(start))
        wall_end = round(clock.to_elapsed_ms(end))
        duration = wall_end - wall_start
        times = np.arange(duration + 1, dtype=float)
        poses = []
        quaternions = []
        fovs = []
        for local in times:
            wall = min(clock.elapsed_duration_ms, max(0.0, wall_start + local))
            scene = min(end, max(start, clock.to_source_ms(wall)))
            position, rotation = base.world_pose(rows, group, actor, scene * 0.001, 32, 45)
            fov = float(base.curve(fov_tracks[-1].get('floattrack', {}).get('points', []), scene * 0.001, 90.0)) if fov_tracks else 90.0
            poses.append(position)
            quaternions.append(Rotation.from_matrix(rotation).as_quat())
            # Original FOVAngle is horizontal; the camera contract stores vertical FOV.
            fovs.append(math.degrees(2 * math.atan(math.tan(math.radians(fov) / 2) / (16 / 9))))
        positions = np.array(poses)
        quaternions = np.array(quaternions)
        fovs = np.array(fovs)
        # Measure the runtime interpolation against every integer elapsed millisecond.
        keep = {0, len(times) - 1}
        while True:
            worst = (1.0, None)
            measured = [0.0, 0.0, 0.0]
            ordered = sorted(keep)
            for left, right in zip(ordered, ordered[1:]):
                indexes = np.arange(left + 1, right)
                if not len(indexes):
                    continue
                alpha = (times[indexes] - times[left]) / (times[right] - times[left])
                iq = Slerp([0.0, 1.0], Rotation.from_quat([quaternions[left], quaternions[right]]))(alpha).as_quat()
                angles = np.degrees(2 * np.arccos(np.minimum(1.0, np.abs(np.sum(quaternions[indexes] * iq, axis=1)))))
                errors = np.linalg.norm(positions[indexes] - (positions[left] * (1 - alpha[:, None]) + positions[right] * alpha[:, None]), axis=1)
                ferrors = np.abs(fovs[indexes] - (fovs[left] * (1 - alpha) + fovs[right] * alpha))
                score = np.maximum(np.maximum(errors / 0.005, angles / 0.1), ferrors / 0.02)
                local_worst = int(np.argmax(score))
                measured = [max(measured[0], float(errors.max())), max(measured[1], float(angles.max())), max(measured[2], float(ferrors.max()))]
                if score[local_worst] > worst[0]:
                    worst = (float(score[local_worst]), int(indexes[local_worst]))
            if worst[1] is None:
                break
            if len(keep) >= 128:
                raise ValueError('Original camera needs more than 128 keys; stable shots must not be split automatically')
            keep.add(worst[1])
        shotid = f'kouku.bingo.ending.camera.{number}'
        original_id = f'kouku.bingo.ending.camera.{number}'
        shot = copy.deepcopy(existing[original_id])
        shot['shotId'] = shotid
        keys = []
        for index in sorted(keep):
            position = positions[index]
            rotation = Rotation.from_quat(quaternions[index]).as_matrix()
            keys.append(dict(sceneId=f'kouku.bingo.ending.camera{number}.k{len(keys)}', timeMs=int(times[index]), eye=position.tolist(), lookAt=(position + rotation[:, 0] * 10).tolist(), up=rotation[:, 1].tolist(), fovYDegrees=float(fovs[index])))
        shot.update(defaultHoldMs=duration, eye=keys[0]['eye'], lookAt=keys[0]['lookAt'], fovYDegrees=keys[0]['fovYDegrees'], blendInMs=0, blendOutMs=0)
        shot['cameraTrack'] = dict(durationMs=duration, interpolation='LINEAR', easing='LINEAR', keyframes=keys)
        shots.append(shot)
        row = occurrences[original_id]
        for field, value in [('startMs', wall_start), ('durationMs', duration)]:
            if row[field] != value:
                patch.append(dict(file=str(comp_path.relative_to(ROOT)).replace(chr(92), '/'), patternId=pattern['patternId'], list='presentationOccurrences', occurrenceId=row['occurrenceId'], field=field, expected=row[field], value=value, reason='Original SCENE01B camera on shared Slomo elapsed clock; current user timing is intentionally replaced only in the approved restoration.'))
        audit.append(dict(shotId=shotid, sourceStartMs=start, sourceEndMs=end, elapsedStartMs=wall_start, elapsedEndMs=wall_end, keyCount=len(keys), maximumErrors=dict(eyeM=measured[0], orientationDegrees=measured[1], fovYDegrees=measured[2])))
        print(shotid, wall_start, wall_end, len(keys), measured, flush=True)
    resource_patch = []
    for path, document, pattern_id in [(comp_path, composition, 'KAKULSAYDON_G1_PATTERN_9'), (action_path, json.loads(action_raw), 'KAKULSAYDON_G1_PATTERN_75')]:
        pattern_row = next((p for p in document['patterns'] if p['patternId'] == pattern_id))
        resource_rows = {r['resourceId']: r for r in document['presentationResources']}
        camera_rows = {resource_rows[o['resourceId']]['assetId']: o for o in pattern_row['presentationOccurrences'] if o['resourceId'] in resource_rows and resource_rows[o['resourceId']]['kind'] == 'CAMERA'}
        for shot, measurement in zip(shots, audit):
            row = camera_rows[shot['shotId']]
            resource = resource_rows[row['resourceId']]
            if path == action_path:
                for field, value in [('startMs', measurement['elapsedStartMs']), ('durationMs', shot['defaultHoldMs'])]:
                    if row[field] != value:
                        patch.append(dict(file=path.relative_to(ROOT).as_posix(), patternId=pattern_id, list='presentationOccurrences', occurrenceId=row['occurrenceId'], field=field, expected=row[field], value=value, reason='Original SCENE01B shared camera on the Slomo elapsed clock.'))
            if resource['durationMs'] != shot['defaultHoldMs']:
                resource_patch.append(dict(file=path.relative_to(ROOT).as_posix(), list='presentationResources', resourceId=resource['resourceId'], field='durationMs', expected=resource['durationMs'], value=shot['defaultHoldMs']))
    sys.path.insert(0, str(ROOT / 'Tools/CompositionPipeline'))
    import composition_pipeline as pipeline
    for shot in shots:
        pipeline._validate_camera_track(shot['cameraTrack'], shot['shotId'])
    if len(shots) != 12 or any((len(s['cameraTrack']['keyframes']) > 128 for s in shots)):
        raise ValueError('Expected the twelve existing camera identities under the 128-key bound')
    assert audit[0]['elapsedStartMs'] == 0 and audit[-1]['elapsedEndMs'] == round(clock.elapsed_duration_ms)
    assert all((a['elapsedEndMs'] == b['elapsedStartMs'] for a, b in zip(audit, audit[1:])))
    candidate = dict(sourceScene='SCENE01B', sourceSha256=hashlib.sha256(source.read_bytes()).hexdigest(), baselineCameraSha256=hashlib.sha256(cam_raw).hexdigest(), baselineCompositionSha256=hashlib.sha256(comp_raw).hexdigest(), baselineActionCompositionSha256=hashlib.sha256(action_raw).hexdigest(), sourceDurationMs=clock.source_duration_ms, elapsedDurationMs=clock.elapsed_duration_ms, shots=shots, compositionFieldPatch=patch, resourceFieldPatch=resource_patch, newPresentationResources=[], newPresentationOccurrences=[], nextPresentationOccurrenceOrdinal=dict(expected=pattern['nextPresentationOccurrenceOrdinal'], value=pattern['nextPresentationOccurrenceOrdinal']), audit=audit, validation=dict(keyframeLimit=128, largestShotKeys=max((len(s['cameraTrack']['keyframes']) for s in shots)), pythonActualTrackValidator='PASS all 12 camera tracks', coverage='Continuous 0..52317 elapsed ms', sourceComparison='Every integer elapsed millisecond; native CModel/GPU not executed'), limitation='Candidate only. Shared wall clock is required for actors/effects/events. No Data changes. Original ten Director cuts retain twelve existing shot identities; camera.2 retains its complete source path under the 128-key runtime bound.')
    return candidate

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True, help='Original SCENE01B rows/imports JSON cache (read-only).')
    parser.add_argument('--output', type=Path, required=True, help='Candidate JSON output under the repository out directory.')
    parser.add_argument('--camera-source', type=Path, default=DEFAULT_CAMERA_SOURCE)
    parser.add_argument('--sequence-source', type=Path, default=DEFAULT_SEQUENCE_SOURCE)
    parser.add_argument('--action-source', type=Path, default=DEFAULT_ACTION_SOURCE)
    args = parser.parse_args()
    output = args.output.resolve()
    if not output.is_relative_to((ROOT / 'out').resolve()):
        parser.error('--output must stay under the repository out directory; this tool does not install data')
    inputs = [args.source.resolve(), args.camera_source.resolve(), args.sequence_source.resolve(), args.action_source.resolve()]
    if output in inputs:
        parser.error('--output must not replace a source input')
    candidate = build_candidate(*inputs)
    base.write(output, candidate)
    print(f"Prepared {len(candidate['shots'])} camera shots; {len(candidate['compositionFieldPatch'])} occurrence timing fields; {len(candidate['resourceFieldPatch'])} resource duration fields. Output: {output}")
if __name__ == '__main__':
    main()
