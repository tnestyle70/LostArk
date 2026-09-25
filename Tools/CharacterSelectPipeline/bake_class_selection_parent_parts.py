"""Copy the baked source parent pose into a source part's exact bone palette.

Secondary SkelControlAnimDynamics/SkelControlOscillate are explicitly reported,
not replaced by invented animation. This writes only offline ordinary WANMs.
"""
from __future__ import annotations
import argparse
import hashlib
from pathlib import Path
import numpy as np
import bake_guardian_selection as bake


def run(profile_path, output):
    profile_path, output = profile_path.resolve(), output.resolve()
    assert output.is_relative_to(bake.ROOT/'out')
    profile = bake.read(profile_path)
    parent_receipt_path = Path(profile['parentPoseReceipt'])
    parent_receipt = bake.read(parent_receipt_path)
    assert parent_receipt['complete'] and parent_receipt['classId'] == profile['classId']
    receipt = dict(classId=profile['classId'], actors=[], resources=[], boneBindings=[],
                   installed=False, inputs={}, parentPoseReceipt=str(parent_receipt_path))
    def record(path):
        payload=path.read_bytes(); receipt['inputs'][str(path)]=hashlib.sha256(payload).hexdigest(); return payload
    record(profile_path); record(parent_receipt_path)
    by_actor={item['actorId']:item for item in parent_receipt['actors']}
    for actor in profile['actors']:
        if 'parentPoseActorId' not in actor: continue
        parent=by_actor[actor['parentPoseActorId']]
        parent_path=Path(parent['donorModel']); target_path=Path(actor['geometryModelPath'])
        assert hashlib.sha256(record(parent_path)).hexdigest()==parent['sha256']
        record(target_path)
        source=bake.base.wm.read_wmodel(parent_path,include_geometry=False)
        target=bake.base.wm.read_wmodel(target_path,include_geometry=False,animation_names=())
        lookup={bone.name.casefold():i for i,bone in enumerate(source.skeleton_bones)}
        source_rest,target_rest=bake.rest_pose(source),bake.rest_pose(target)
        for bone in target.skeleton_bones:
            if bone.name.casefold() not in lookup: continue
            pbone=source.skeleton_bones[lookup[bone.name.casefold()]]
            tp=target.skeleton_bones[bone.parent] if bone.parent>=0 else None
            pp=source.skeleton_bones[pbone.parent] if pbone.parent>=0 else None
            if tp and pp and tp.name.casefold()!=pp.name.casefold():
                assert tp.parent==pp.parent==-1, ('Parent hierarchy differs',bone.name)
                assert np.allclose(tp.transform,np.eye(4).flatten(),atol=1e-7)
                assert np.allclose(pp.transform,np.eye(4).flatten(),atol=1e-7)
            else: assert bool(tp)==bool(pp)
        absent=[b.name for b in target.skeleton_bones if b.name.casefold() not in lookup]
        phases, details, clips=[],[],{}
        source_clips={clip.name:clip for clip in source.animations}
        for phase in actor['phases']:
            name=phase['name']; source_clip=source_clips[parent['clips'][name]]
            duration=next(p['durationMs'] for p in parent['phases'] if p['name']==name)/1000.
            ticks={key[0]/source_clip.ticks_per_second for channel in source_clip.channels
                   for keys in (channel.position_keys,channel.rotation_keys,channel.scale_keys) for key in keys}
            times=sorted({0.,duration,*[min(duration,max(0.,time)) for time in ticks]})
            assert len(times)<=20000
            samples=[]
            for time in times:
                pose=bake.base.pose_sample(source,source_clip,time)
                samples.append([pose[lookup[b.name.casefold()]] if b.name.casefold() in lookup else target_rest[i]
                                for i,b in enumerate(target.skeleton_bones)])
            clips[name]=phase['clipName']; phases.append((phase['clipName'],times,samples))
            details.append(dict(name=name,clipName=phase['clipName'],durationMs=round(duration*1000),
                                sampleCount=len(times),sourceParentClip=source_clip.name))
        asset=actor['outputAssetId']; relative=Path(asset)
        assert not relative.is_absolute() and '..' not in relative.parts and relative.parts[0]=='Character'
        saved=bake.save_model(target_path,output/'Resources'/asset,target,phases,True,Path(actor['resourceRoot']))
        secondary=actor.get('secondaryMotionBoundary',
            'Original secondary dynamics/oscillation is not evaluated; absent parent bones preserve source reference pose.')
        detail=dict(actorId=actor['actorId'],parentPoseActorId=actor['parentPoseActorId'],
                    sourceComponentIndex0=actor['sourceComponentIndex0'],clips=clips,phases=details,
                    animationSetAssetId=asset,sourceReferencePoseBones=absent,
                    secondaryMotionBoundary=secondary,**saved)
        receipt['actors'].append(detail)
        receipt['resources'].append(dict(animationSetAssetId=asset,candidatePath=saved['candidatePath'],
            sourceComponentIndex0=actor['sourceComponentIndex0'],clips=clips,sha256=saved['sha256']))
        print('parent pose baked',actor['actorId'],len(target.skeleton_bones),'bones',flush=True)
    for path,expected in receipt['inputs'].items(): assert hashlib.sha256(Path(path).read_bytes()).hexdigest()==expected
    assert receipt['actors']
    receipt['complete']=True
    bake.write(output/'bake-receipt.json',receipt)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--profile',type=Path,required=True); parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args(); run(args.profile,args.output)
