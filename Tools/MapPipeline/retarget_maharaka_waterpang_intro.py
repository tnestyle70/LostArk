"""Video-guided intro15 camera + source smile in the existing MapTool player.

Camera framing and cannon clip scheduling are PROJECT_ADAPTED, not extracted
Matinee coordinates. The original importer and intro20 remain unchanged.
Prepare writes only out/. Apply requires the reviewed byte-identical candidate.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
from pathlib import Path

import build_maharaka_waterpang_sequences as b

OUT = b.ROOT / 'out/MaharakaCameraVideo20260928'
IDENTITY = b.PREFIX + '.intro15'
BIG = 'npc.maharaka.source57009.actor100'
SMALL = 'npc.maharaka.source57009.actor188'


def merge_reviewed(before, candidate, current, path=()):
    """Merge only reviewed fields; preserve unrelated Save edits by stable ID."""
    if before == candidate:
        return current
    if path == ('revision',):
        return current + 1
    if current == before:
        return candidate
    if current == candidate:
        return current
    if all(isinstance(v, dict) for v in (before, candidate, current)):
        result = copy.deepcopy(current)
        for key in set(before) | set(candidate):
            if key not in before:
                b.require(key not in current or current[key] == candidate[key], 'Concurrent added field: '+str(path+(key,)))
                result[key] = candidate[key]
            elif key not in candidate:
                b.require(key not in current or current[key] == before[key], 'Concurrent removed field: '+str(path+(key,)))
                result.pop(key, None)
            elif before[key] != candidate[key]:
                b.require(key in current, 'Concurrent removed target: '+str(path+(key,)))
                result[key] = merge_reviewed(before[key], candidate[key], current[key], path+(key,))
        return result
    identity = {'shots':'shotId','cutscenes':'cutsceneId','objectResources':'objectId',
                'templates':'sequenceId','instances':'instanceId'}.get(path[-1] if path else '')
    if identity and all(isinstance(v, list) for v in (before, candidate, current)):
        maps = [{row[identity]:row for row in rows} for rows in (before,candidate,current)]
        b.require(all(len(m)==len(rows) for m,rows in zip(maps,(before,candidate,current))), 'Duplicate stable identity')
        merged = merge_reviewed(*maps, path=path+('by-id',))
        order = [row[identity] for row in current] + [row[identity] for row in candidate if row[identity] not in maps[2]]
        return [merged[key] for key in order if key in merged]
    if path and path[-1] == 'worldInstanceIds' and candidate[:len(before)] == before:
        return current + [value for value in candidate[len(before):] if value not in current]
    raise RuntimeError('Concurrent edit of reviewed field: '+'.'.join(path))


def smooth(t):
    t = min(1., max(0., t))
    return t*t*(3.-2.*t)


def prepare():
    paths = [b.AUTHORING/(b.AREA+'.'+suffix+'.json') for suffix in ('camerashots', 'worldsequences')]
    dependencies = paths + [b.ROOT/'Data/Actors/NpcCatalog.json',
                            b.ROOT/f'Data/Worlds/{b.AREA}/Gameplay.world.json']
    original = {p: p.read_bytes() for p in dependencies}
    camera, world, npcs, gameplay = [json.loads(original[p].decode('utf-8-sig')) for p in dependencies]
    actors = {p['placementId']: p for p in gameplay['placements']}
    source_path = Path(b.load_json(b.AUDIT)['source']['physicalPackage'])
    b.require(hashlib.sha256(source_path.read_bytes()).hexdigest() == b.load_json(b.AUDIT)['source']['sha256'], 'Source package changed')
    rows, imports = b.source.extract_scene(source_path)
    b.require(imports['-70'] == 'mn_ismp_00.mat.mn_ismp_00-2_mi', 'Source smile material identity changed')
    duration = round(rows[158]['p']['interplength']*1000)
    b.require(duration == 9507, 'Source timing changed')
    scene = next(c for c in camera['cutscenes'] if c['cutsceneId'] == 'cutscene.'+IDENTITY)
    b.require(scene['durationMs'] == duration, 'Preserve user duration edit')
    big_position = b.np.array(actors[BIG]['position'])
    # Installed centimetre model: face is in front (+Z after NPC -90deg import).
    face = big_position + b.np.array([0., 1.5, 1.0])
    close_eye = face + b.np.array([-1.15, .3, 5.3])
    arena = b.np.array(actors[SMALL]['position'])
    heading = b.Rotation.from_euler('y', -45., degrees=True).as_matrix()
    source_close, _ = b.source.world_pose(rows, 206, 3, 8.101, 43, 158)
    rotated_close = arena + heading @ (source_close-arena)
    translate_close = close_eye - rotated_close
    fov_track = rows[304]['p']['floattrack']['points']
    samples = []
    for cut in scene['cameraCuts']:
        shot = next(s for s in camera['shots'] if s['shotId'] == cut['shotId'])
        start = cut['startMs']
        end = start + shot['cameraTrack']['durationMs']
        times = set(range(start, end, 50)) | {start, end}
        times.update(t for t in (6469,6602,7177,8101,8935,9502) if start <= t <= end)
        b.require(len(times) <= 128, 'Camera key capacity')
        keys = []
        for ms in sorted(times):
            p, r = b.source.world_pose(rows, 206, 3, ms/1000., 43, 158)
            p = arena + heading @ (p-arena)
            r = heading @ r
            weight = smooth((ms-6469)/(7177-6469)) * (1.-smooth((ms-8101)/(8935-8101)))
            eye = p + translate_close*weight
            look = (p+r@b.np.array([10.,0.,0.]))*(1.-weight) + face*weight
            up = (r@b.np.array([0.,1.,0.]))*(1.-weight) + b.np.array([0.,1.,0.])*weight
            up /= b.np.linalg.norm(up)
            horizontal = float(b.source.curve(fov_track, ms/1000.,90.))
            vertical = b.math.degrees(2*b.math.atan(b.math.tan(b.math.radians(horizontal)/2)/(16/9)))
            keys.append(dict(sceneId=f'{IDENTITY}.video.k{ms}',timeMs=ms-start,
                eye=eye.tolist(),lookAt=look.tolist(),up=up.tolist(),fovYDegrees=vertical))
            if weight == 1.:
                ray = (look-eye)/b.np.linalg.norm(look-eye)
                miss = b.np.linalg.norm(b.np.cross(face-eye,ray))
                b.require(miss < 1e-8, 'Close-up must aim at the actual large face')
                samples.append(dict(timeMs=ms,eye=eye.tolist(),face=face.tolist(),rayMissM=float(miss)))
        shot['cameraTrack']['keyframes'] = keys
        for key in ('eye','lookAt','fovYDegrees'): shot[key] = keys[0][key]
        shot['box']['center'] = keys[0]['eye']
        shot['displayName'] = '워터팡 / 영상 맞춤 카메라 ' + str(start)
    scene['displayName'] = '워터팡 / 도입 15 · 영상 맞춤 카메라·표정'
    camera['revision'] += 1
    # The source targets the shared smile MIC; both original bodies own this MIC.
    smile = rows[286]['p']['floattrack']['points']
    smile_keys = [dict(timeMs=0,value=[0.,0.,0.,0.],interpolation='CONSTANT')]
    smile_keys += [dict(timeMs=round(p['inval']*1000),value=[p['outval'],0.,0.,0.],interpolation='CONSTANT') for p in smile]
    smile_keys.append(dict(timeMs=duration,value=[0.,0.,0.,0.],interpolation='CONSTANT'))
    for tag, placement_id in [('mokomoko',BIG),('cannon',SMALL)]:
        placement = actors[placement_id]
        actor = next(a for a in npcs['npcs'] if a['archetypeId'] == placement['archetypeId'])
        model = b.ROOT/'Client/Bin/Resources'/actor['modelAssetId']
        original[model] = model.read_bytes()
        decoded = b.source.wm.read_wmodel(model,include_geometry=False)
        b.require(all(any(a.name == clip for a in decoded.animations) for clip in ('idle_normal_1','att_battle_2_01','att_battle_2_02')), 'Missing exact installed clip')
        profile = copy.deepcopy(next(p for p in npcs['modelMaterialOverrides'] if p['modelAssetId'] == actor['modelAssetId'] and p['materialName'] == 'mn_ismp_00-2_mi'))
        del profile['modelAssetId']
        ident = IDENTITY+'.'+tag
        resource = dict(objectId='world.object.'+ident, displayName='워터팡 / '+tag,
            modelAssetId=actor['modelAssetId'], modelPreScale=.01, animated=True, scale=[1.,1.,1.],
            anchorKind='WORLD', materialProfile=profile)
        q = b.Rotation.from_euler('y',placement['yawDegrees']-90.,degrees=True).as_quat().tolist()
        key = dict(timeMs=0,positionOffset=placement['position'],rotationQuaternion=q,scaleMultiplier=[1.,1.,1.],visible=True)
        clips = [dict(slotId='actor',clipName='idle_normal_1',startMs=0,playbackRate=1,loop=True,holdLastFrame=True)]
        if tag == 'cannon':
            # Video-guided rise, using the original installed raise/hold clips.
            # Not asserted to be a serialized Matinee animation binding.
            clips += [dict(slotId='actor',clipName=name,startMs=t,playbackRate=1,loop=loop,holdLastFrame=True)
                      for name,t,loop in [('att_battle_2_01',8101,False),('att_battle_2_02',9101,True)]]
        template = dict(sequenceId='sequence.'+ident,displayName='워터팡 / '+tag+' 표정',category='World',
            durationMs=duration,interpolation='LINEAR',tracks=[dict(slotId='actor',keys=[key,dict(key,timeMs=duration)])],
            animationTracks=clips,materialTracks=[dict(slotId='actor',materialName=profile['materialName'],
                curves=[dict(parameter='opacity_intensity',keys=smile_keys)])])
        instance = dict(instanceId='world.sequence.instance.'+ident,templateId=template['sequenceId'],enabled=True,
            startDelayMs=0,playbackSpeed=1,anchorKind='WORLD',position=[0.,0.,0.],motionEnd='STOP',
            bindings=[dict(slotId='actor',targetKind='OBJECT_RESOURCE',targetId=resource['objectId'],previewNpcPlacementId=placement_id)])
        for field,identity,row in [('objectResources','objectId',resource),('templates','sequenceId',template),('instances','instanceId',instance)]:
            b.require(not any(x[identity] == row[identity] for x in world[field]), 'Already installed: reload the reviewed candidate instead of overwriting edits')
            world[field].append(row)
        scene['worldInstanceIds'].append(instance['instanceId'])
    world['revision'] += 1
    staged = {paths[0]:(original[paths[0]],b.encoded(camera)),paths[1]:(original[paths[1]],b.encoded(world))}
    report = dict(cameraBasis='PROJECT_VIDEO_RETARGET_NOT_RAW_SOURCE_COORDINATES',
        sourceSmile=dict(package=str(source_path),matinee=43,track=286,material=imports['-70'],keys=smile_keys),
        cannonMotionBasis='PROJECT_VIDEO_SCHEDULE_OF_SOURCE_RAISE_AND_HOLD_CLIPS',
        unchanged=['intro20','stage motions','foley','gameplay NPC positions','rendering options'],
        closeupSamples=samples,visualApproval=False)
    return staged, original, report


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply-reviewed',action='store_true')
    args=parser.parse_args()
    OUT.mkdir(parents=True,exist_ok=True)
    receipt=OUT/'reviewed-inputs.json'
    if args.apply_reviewed:
        manifest=b.load_json(receipt)
        expected={b.ROOT/rel:(OUT/'before'/rel).read_bytes() for rel in manifest}
        for path,raw in expected.items():
            b.require(hashlib.sha256(raw).hexdigest()==manifest[path.relative_to(b.ROOT).as_posix()], 'Review backup changed')
        staged = {}
        for path, raw in list(expected.items()):
            candidate_path = OUT/'candidate'/path.relative_to(b.ROOT)
            if not candidate_path.exists():
                continue
            current = path.read_bytes()
            merged = merge_reviewed(json.loads(raw.decode('utf-8-sig')),
                                    b.load_json(candidate_path), json.loads(current.decode('utf-8-sig')))
            backup = OUT/'before-apply'/path.relative_to(b.ROOT)
            backup.parent.mkdir(parents=True, exist_ok=True)
            backup.write_bytes(current)
            staged[path] = (current, b.encoded(merged))
            expected[path] = current
        b.commit_staged_files(staged,expected=expected)
        print('Applied reviewed camera and actor candidates; runtime publish is separate.')
        return
    b.require(not receipt.exists(),'Reviewed candidate already exists; preserve it until applied or explicitly re-reviewed')
    staged,expected,report=prepare()
    for path,raw in expected.items():
        dest=OUT/'before'/path.relative_to(b.ROOT);dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(raw)
    for path,(_,raw) in staged.items():
        dest=OUT/'candidate'/path.relative_to(b.ROOT);dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(raw)
    receipt.write_bytes(b.encoded({p.relative_to(b.ROOT).as_posix():hashlib.sha256(raw).hexdigest() for p,raw in expected.items()}))
    (OUT/'comparison-result.json').write_bytes(b.encoded(report))
    print(json.dumps(dict(candidate=str(OUT/'candidate'),closeupSamples=len(report['closeupSamples']),files=len(staged))))


if __name__ == '__main__':
    main()
