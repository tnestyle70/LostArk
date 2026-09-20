"""Project original Valtan Matinee camera curves onto the admitted Product clocks."""
from pathlib import Path
import argparse, copy, json, sys
ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'Tools/EffectPipeline'),str(ROOT/'Tools/KoukuSaydonPipeline'),str(ROOT/'Tools/LevelPlacementExtractor')]
import build_valtan_full_restore as writer
import build_gate2_intro_composition as base

def frames(sequence):
    by_time={};previous_group=None
    for segment in sequence['shots']:
        group=segment['shot']['displayName'].rsplit(' / ',1)[-1];start=segment['startMs']
        cut=previous_group is not None and group!=previous_group
        if cut:
            by_time[start-1]=dict(copy.deepcopy(by_time[start]),timeMs=start-1)
        for key in segment['shot']['cameraTrack']['keyframes']:
            t=start+key['timeMs'];by_time[t]=dict(key,timeMs=t)
        if cut:by_time[start]['cutBefore']=True
        previous_group=group
    return [by_time[t]for t in sorted(by_time)]

def sample(keys,t):
    a=next(x for x in reversed(keys)if x['timeMs']<=t)
    b=next((x for x in keys if x['timeMs']>t),a)
    if a['timeMs']==t:return copy.deepcopy(a)
    if b.get('cutBefore') or a is b:return dict(a,timeMs=t)
    u=(t-a['timeMs'])/(b['timeMs']-a['timeMs'])
    def quat(x):
        f=base.np.array(x['lookAt'])-base.np.array(x['eye']);f/=base.np.linalg.norm(f)
        right=base.np.cross(base.np.array(x['up']),f);right/=base.np.linalg.norm(right)
        return base.Rotation.from_matrix(base.np.column_stack([right,base.np.cross(f,right),f])).as_quat()
    rotation=base.Rotation.from_quat(base.slerp(quat(a),quat(b),u)).as_matrix()
    eye=base.np.array(a['eye'])*(1-u)+base.np.array(b['eye'])*u
    return dict(timeMs=t,eye=eye.tolist(),lookAt=(eye+rotation[:,2]*10).tolist(),up=rotation[:,1].tolist(),fovYDegrees=a['fovYDegrees']*(1-u)+b['fovYDegrees']*u)

def cut_keys(keys,start,end,identity):
    result=[sample(keys,start)]+[copy.deepcopy(k)for k in keys if start<k['timeMs']<end]+[sample(keys,end)]
    for ordinal,key in enumerate(result):
        key['timeMs']-=start;key['sceneId']=identity+'.key.'+str(ordinal)
    result[0].pop('cutBefore',None)
    assert len(result)<=512
    return result

def main():
    parser=argparse.ArgumentParser(__doc__);parser.add_argument('--source-root',type=Path,required=True);parser.add_argument('--evidence-root',type=Path,required=True);parser.add_argument('--install',action='store_true');args=parser.parse_args()
    sequences=json.loads((args.source_root/'native-camera-candidates.json').read_bytes())['sequences']
    camera_path=ROOT/'Data/Encounters/Valtan/ValtanCinematicCamera.json';camera_before=camera_path.read_bytes();camera=json.loads(camera_before)
    presentation_path=ROOT/'Data/Valtan/Valtan.presentation.json';presentation_before=presentation_path.read_bytes();presentation=json.loads(presentation_before)
    gameplay=json.loads((ROOT/'Data/Valtan/Valtan.gameplay.json').read_bytes());receipts=[]
    for name,scene,matinee,pattern_id,stages in [
        ('entrance','LV_LUT_HEARTRB_ED_SCENE06A',53,'VALTAN_ENTRANCE_CINEMATIC',['ESTABLISH','ARENA_REVEAL','HERO_HANDOFF']),
        ('trash','LV_LUT_HEARTRB_ED_SCENE06A',54,'VALTAN_TRASH',['STEP_05','STEP_06'])]:
        sequence=next(x for x in sequences if x['scene']==scene and x['matinee']==matinee);keys=frames(sequence)
        gp=next(p for p in gameplay['patterns']if p['patternId']==pattern_id);pp=next(p for p in presentation['patterns']if p['patternId']==pattern_id)
        offset=0
        for stage_id in stages:
            stage=next(s for s in gp['stages']if s['stageId']==stage_id);ps=next(s for s in pp['stages']if s['stageId']==stage_id)
            identity=('camera.valtan.entrance.'+stage_id.lower().replace('_','-') if name=='entrance' else 'camera.valtan.source.trash.'+stage_id.lower().replace('_','-'))
            duration=stage['durationMs'];cue=dict(cueId=identity,patternId=pattern_id,stageId=stage_id,durationMs=duration,interpolation='LINEAR',easing='LINEAR',shakeAmplitude=0,shakeDurationMs=0,keyframes=cut_keys(keys,offset,offset+duration,identity))
            old=next((c for c in camera['cues']if c['cueId']==identity),None)
            if old:camera['cues'][camera['cues'].index(old)]=cue
            else:camera['cues'].append(cue)
            old_ids={i['cameraInvocationId'] for i in ps['cameraInvocations']}
            assert not old_ids or len(old_ids)==1
            ps['cameraInvocations']=[dict(cameraInvocationId=next(iter(old_ids),identity+'.invocation'),cameraCueId=identity,trigger='ENTER',startOffsetMs=0,durationPolicy='EXPLICIT',durationMs=duration)]
            receipts.append(dict(cueId=identity,sourceScene=scene,sourceMatinee=matinee,sourceStartMs=offset,sourceEndMs=offset+duration,keys=len(cue['keyframes'])))
            offset+=duration
        assert offset==(24708 if name=='entrance' else 6374)
    finale=next(x for x in sequences if x['scene']=='LV_LUT_HEARTRB_ED_SCENE04A' and x['matinee']==24)
    camera['deathCue']=dict(cueId='camera.valtan.clear.wide',durationMs=23000,interpolation='LINEAR',easing='LINEAR',shakeAmplitude=0,shakeDurationMs=0,keyframes=cut_keys(frames(finale),0,23000,'camera.valtan.source.finale'))
    args.evidence_root.mkdir(parents=True,exist_ok=True)
    writes=[]
    for path,before,document in [(camera_path,camera_before,camera),(presentation_path,presentation_before,presentation)]:
        (args.evidence_root/(path.name+'.before')).write_bytes(before);after=writer.encode_json(document);(args.evidence_root/(path.name+'.candidate')).write_bytes(after);writes.append((path,before,after))
    (args.evidence_root/'source-camera-joins.json').write_bytes(writer.encode_json(dict(stages=receipts,deathDurationMs=23000,sourceTimeScale=1,visualStatus='USER_PENDING')))
    if args.install:writer.commit_writes(writes)
    print(json.dumps(dict(cues=len(receipts),deathDurationMs=23000,installed=args.install)))
if __name__=='__main__':main()
