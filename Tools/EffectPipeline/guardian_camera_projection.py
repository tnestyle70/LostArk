"""Project retail SkillCam_DragonKnight_02 curves into the shared camera track.

Input is the original package's normalized tagged-property graph. The local
attachment uses the source SeqAct_AttachToActor zero offset/rotation defaults;
the live character root is applied by CEffectRecoveryCamera, not baked here.
"""
from __future__ import annotations
import bisect, math

def _v(value):
    return [float(value[k]) for k in ('x','y','z')] if isinstance(value,dict) else [float(value)]

def sample_curve(points, seconds):
    if not points or any(not math.isfinite(p['inval']) for p in points):raise ValueError('Invalid source curve')
    index=bisect.bisect_right([p['inval'] for p in points],seconds)
    if not index:return _v(points[0]['outval'])
    if index==len(points):return _v(points[-1]['outval'])
    a,b=points[index-1:index+1];dt=b['inval']-a['inval']
    if dt<=0:raise ValueError('Source curve times must increase')
    t=(seconds-a['inval'])/dt;av,bv=_v(a['outval']),_v(b['outval'])
    if a['interpmode']=='cim_constant':return av
    if a['interpmode']=='cim_linear':return [x+(y-x)*t for x,y in zip(av,bv)]
    if a['interpmode'] not in ('cim_curveauto','cim_curveautoclamped','cim_curveuser','cim_curvebreak'):raise ValueError('Unsupported source curve mode')
    return [(2*t**3-3*t*t+1)*x+(t**3-2*t*t+t)*dt*dx+(-2*t**3+3*t*t)*y+(t**3-t*t)*dt*dy
            for x,dx,y,dy in zip(av,_v(a['leavetangent']),bv,_v(b['arrivetangent']))]

def axes(euler):
    roll,pitch,yaw=map(math.radians,euler)
    sr,cr=math.sin(roll),math.cos(roll);sp,cp=math.sin(pitch),math.cos(pitch);sy,cy=math.sin(yaw),math.cos(yaw)
    # UE FRotationMatrix basis: X forward, Y right, Z up.
    return [[cp*cy,cp*sy,sp],[sr*sp*cy-cr*sy,sr*sp*sy+cr*cy,-sr*cp],[-cr*sp*cy-sr*sy,-cr*sp*sy+sr*cy,cr*cp]]

def transform(vector,basis):return [sum(vector[i]*basis[i][j] for i in range(3)) for j in range(3)]
def convert(vector,scale=1.):return [vector[0]*scale,vector[2]*scale,-vector[1]*scale]

def native_pose(graph,seconds):
    rows=graph['rows'];parent=rows['100']['p'];camera=rows['102']['p']
    parent_axes=axes(sample_curve(parent['eulertrack']['points'],seconds))
    parent_pos=sample_curve(parent['postrack']['points'],seconds)
    position=transform(sample_curve(camera['postrack']['points'],seconds),parent_axes)
    position=[a+b for a,b in zip(position,parent_pos)]
    camera_axes=axes(sample_curve(camera['eulertrack']['points'],seconds))
    forward=transform(camera_axes[0],parent_axes);up=transform(camera_axes[2],parent_axes)
    eye=convert(position,.01);forward=convert(forward)
    return dict(eye=eye,lookAt=[a+b for a,b in zip(eye,forward)],up=convert(up),
                fovDegrees=sample_curve(rows['81']['p']['floattrack']['points'],seconds)[0])

def project_camera(graph,effect_id,duration_ms=12000):
    rows=graph['rows']
    if rows['123']['p']['eventname'].lower()!='skillcam_dragonknight_02' or not rows['99']['p'].get('bdisabletrack'):raise ValueError('Unexpected camera source graph')
    attach=rows['110']['p']
    if not attach.get('buserelativeoffset') or not attach.get('buserelativerotation') or 'relativeoffset' in attach or 'relativerotation' in attach:raise ValueError('Camera source attachment differs')
    if rows['66']['p']['cuttrack']!=[dict(time=-0.051223017275333405,transitiontime=0.0,targetcamgroup='cm02',shotnumber=20)]:raise ValueError('Camera director differs')
    duration=3800
    times={0,duration,*[round(i*1000/120) for i in range(457)]}
    for track,name in [('100','postrack'),('100','eulertrack'),('102','postrack'),('102','eulertrack'),('81','floattrack')]:
        times.update(round(p['inval']*1000) for p in rows[track]['p'][name]['points'] if 0<=p['inval']*1000<=duration)
    keys=[dict(keyId=f'camera.guardianknight.native.{i:04}',timeMs=t,**native_pose(graph,t*.001)) for i,t in enumerate(sorted(times))]
    return dict(schema='lostark.effect-authoring-sequence',formatVersion=3,sequenceId=effect_id,
        model=dict(kind='MODEL_SEQUENCE',assetName='GuardianKnight',sequenceId='skill.49420',anchorMemberId=''),
        anchorMode='MODEL_ROOT',worldPosition=[0,0,0],effects=[dict(occurrenceId='guardian.altv.source.effect',owner='V1_DOCUMENT',
        effectId=effect_id,anchorSlotId='root',startMs=0,durationMs=max(duration,duration_ms),offset=[0,0,0],muted=False)],
        cameras=[dict(cameraId='camera.guardianknight.skillcam_dragonknight_02',displayName='성역의 엠버레스 원본 카메라',
        source='STANDARD_SKILLCAM_DRAGONKNIGHT: RemoteEvent123 -> Attach110 -> SetCameraTarget116 -> Matinee26/Data31; active parent Move100 + CM02 Move102 + FOV81. Stored Hermite tangents sampled at 120 Hz and source key times; disabled Move99 excluded. Source horizontal FOV and up retained. Source AttachToActor class-zero offset/rotation applied through live MODEL_ROOT; original action PLAY0/STOP3800ms.',
        space='MODEL_ROOT',fovAxis='HORIZONTAL',startMs=0,durationMs=duration,muted=False,interpolation='LINEAR',easing='LINEAR',keys=keys)])
