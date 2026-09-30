"""Build winner ceremony from SCENE01A's actual Kismet/Matinee binding.

ceremony -> seqact_setcameratarget_5 -> efseqact_matinee_34 -> interpdata_33.
cameraactor_40 Base=cameraactor_39. interpgroup_193 is the camera-local track;
interpgroup_194 is the boom track (90-degree UE yaw). No intro floor offset.
Position and Euler curves retain original arrive/leave tangents and are sampled
at30Hz so the existing Client camera needs no new UE curve runtime.
"""
import argparse,json,math,struct
from pathlib import Path
import numpy as np
import extract_scene_matinee as source


def sample(keys,t):
    if t<=keys[0]['timeMs']:return np.array(keys[0]['value'],dtype=float)
    if t>=keys[-1]['timeMs']:return np.array(keys[-1]['value'],dtype=float)
    a,b=next((a,b) for a,b in zip(keys,keys[1:]) if a['timeMs']<=t<=b['timeMs'])
    dt=(b['timeMs']-a['timeMs'])/1000
    u=(t-a['timeMs'])/(b['timeMs']-a['timeMs'])
    av,bv=np.array(a['value']),np.array(b['value'])
    if a['mode']=='cim_linear':return av*(1-u)+bv*u
    if a['mode']=='cim_constant':return av
    return ((2*u**3-3*u*u+1)*av+(u**3-2*u*u+u)*dt*np.array(a['leave'])+
            (-2*u**3+3*u*u)*bv+(u**3-u*u)*dt*np.array(b['arrive']))


def main():
    p=argparse.ArgumentParser()
    p.add_argument('--package',type=Path,required=True)
    p.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2])
    a=p.parse_args()
    pkg=source.Package(str(a.package))
    exports={e['name']:e for e in pkg.exports}
    def curve(track,field):return source.curve_keys(pkg,exports[track],field,'structproperty')
    boom=curve('interptrackmove_184','postrack')
    local=curve('interptrackmove_183','postrack')
    rotation=curve('interptrackmove_183','eulertrack')
    # Validate actual root event and exact references before output.
    def refs(name,key):
        e=exports[name];at=e['offset'];end=at+e['size'];pattern=pkg.head_bytes(key,'objectproperty')
        values=[]
        while (at:=pkg.data.find(pattern,at,end))>=0:
            v=struct.unpack_from('<i',pkg.data,at+24)[0]
            if v>0:values.append(pkg.exports[v-1]['name'])
            at+=8
        return values
    assert 'efseqact_matinee_34' in refs('seqact_setcameratarget_5','linkedop')
    assert refs('cameraactor_40','base')==['cameraactor_39']
    keys=[]
    for t in sorted(set([round(i*1000/30) for i in range(151)]+[2000,5000])):
        b=sample(boom,t);l=sample(local,t);rot=sample(rotation,t)
        # UE boom yaw90: (local x,local y) ->(-local y,local x).
        ue=b+np.array([-l[1],l[0],l[2]])
        pitch=math.radians(rot[1])
        keys.append({'timeMs':t,'eye':[round(float(ue[0]*.01),7),round(float(ue[2]*.01),7),round(float(-ue[1]*.01),7)],
                     'forward':[0,round(math.sin(pitch),7),round(-math.cos(pitch),7)]})
    raw=next(s for s in source.collect(pkg) if s['data']=='interpdata_33')
    camera_group=next(g for g in raw['groups'] if g['group']=='interpgroup_193')
    fov_track=next(t for t in camera_group['tracks'] if t.get('property')=='fovangle')
    assert len(fov_track['keys'])==1 and fov_track['keys'][0]['timeMs']==0
    source_fov=fov_track['keys'][0]['value']
    # This conversion is the project's existing intro presentation policy, not
    # an extra source property. Preserve the raw source field beside it.
    letterbox_aspect=2.35
    adapted_fov=math.degrees(2*math.atan(math.tan(math.radians(source_fov)/2)/(16/9)*letterbox_aspect))
    actors=[]
    # Original side actors, not the middle winner, form the two-player adaptation.
    for name in ['interpgroup_206','interpgroup_207']:
        group=next(g for g in raw['groups'] if g['group']==name)
        move=next(t for t in group['tracks'] if t['class']=='interptrackmove')
        anim=next(t for t in group['tracks'] if t['class']=='interptrackanimcontrol' and t['slot']=='a')['clips'][0]
        ue_yaw=math.radians(move['euler'][0]['rollPitchYawDegrees'][2])
        yaw=math.degrees(math.atan2(math.cos(ue_yaw),-math.sin(ue_yaw)))
        actors.append({'position':move['position'][0]['value'],'yawDegrees':round(yaw,6),
                       'clipName':anim['clip'],'clipStartMs':anim['timeMs'],'loop':anim['looping']})
    doc={'schema':'lostark.colosseum-victory-cutscene','formatVersion':1,
         'source':{'package':a.package.name,'scene':'LV_PVP_COLOSSEUM_SCENE01A','matinee':'interpdata_33',
                   'eventChain':['ceremony','seqact_setcameratarget_5','efseqact_matinee_34','interpdata_33'],
                   'actorSelection':'Project 2v2 adaptation uses original left/right winner slots; middle slot omitted.',
                   'cameraCurve':'Source cubic Hermite tangents sampled at30Hz. Original native duration property omitted; last key5000ms.',
                   'rawSourceFovXDegrees':source_fov,
                   'projectionAdaptation':'Project intro policy: preserve vertical angle when converting16:9 to2.35. Not an additional native source property; user camera review required.',
                   'timing':'3475ms source UI announcement,5000ms source last camera key;250ms project transition fade.'},
         'bannerDurationMs':3475,'durationMs':5000,'returnFadeMs':250,'floorY':12.64,
         'fovXDegrees':round(adapted_fov,6),'letterboxAspect':letterbox_aspect,
         'fade':[{'timeMs':0,'value':0},{'timeMs':4750,'value':0},{'timeMs':5000,'value':1}],
         'shots':[{'startMs':0,'endMs':5000,'keys':keys,'forward':keys[0]['forward']}],
         'actors':actors}
    title_path=a.repo/'Data/UI/Colosseum/Result_TitleTracks.json'
    title=json.loads(title_path.read_text(encoding='utf8'))
    assert title['schema']=='lostark.colosseum-result-title-tracks' and title['formatVersion']==1
    doc['bannerTitles']=title['bannerTitles']
    doc['source']['bannerTitleSource']='EFUI_COLOSSEUM.colosseumplaying_loc_int titleString_lb native40fps'
    output=a.repo/'Data/Camera/ColosseumVictory.cutscene.json'
    output.write_text(json.dumps(doc,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(output,'camera samples',len(keys),'actors',len(actors))


if __name__=='__main__':main()
