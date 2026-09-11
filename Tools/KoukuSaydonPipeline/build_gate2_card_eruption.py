"""Cook par_q_cardfly_01's three mesh emitters as an editable CModel motion.

Source mesh geometry, three emitter layouts, 6/s spawn rate, 8s lifetime,
velocity bounds and acceleration/rotation curves come from the native UPK.
The deterministic random seed is authoring-owned; source random state is not
serialized. Geometry is batched by material into one ordinary skinned model.
"""
from pathlib import Path
import json
import math
import subprocess
import sys

import numpy as np
from scipy.spatial.transform import Rotation

ROOT=Path(__file__).resolve().parents[2]
WORK=ROOT/'out/KoukuGate2Restore20260911'
SOURCE=WORK/'Source/BG_RAD_KOUKUSATON_B/StaticMesh3'
OUTPUT=ROOT/'Client/Bin/Resources/Map/KakulSaydon/Gate2Intro/CardEruption'


def accessor(document,payload,index):
    a=document['accessors'][index];v=document['bufferViews'][a['bufferView']]
    width={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4,'MAT4':16}[a['type']]
    dtype={5121:'u1',5123:'<u2',5125:'<u4',5126:'<f4'}[a['componentType']]
    offset=v.get('byteOffset',0)+a.get('byteOffset',0)
    stride=v.get('byteStride',np.dtype(dtype).itemsize*width)
    return np.ndarray((a['count'],width),dtype=dtype,buffer=payload,offset=offset,
        strides=(stride,np.dtype(dtype).itemsize)).copy()


def source_mesh(name):
    path=SOURCE/(name+'.gltf');document=json.loads(path.read_text())
    payload=(path.parent/document['buffers'][0]['uri']).read_bytes()
    result=[]
    for p in document['meshes'][0]['primitives']:
        attributes={k:accessor(document,payload,i) for k,i in p['attributes'].items()}
        result.append((document['materials'][p['material']]['name'],attributes,
                       accessor(document,payload,p['indices']).reshape(-1)))
    return result


def build():
    # The native cooked lookup table's first two values are extrema, not keys.
    source=json.loads((WORK/'cardfly.json').read_text(encoding='utf-8'))
    def unwrap(v):
        if isinstance(v,dict):
            if 'type'in v and 'value'in v:return unwrap(v['value'])
            if 'properties'in v:return unwrap(v['properties'])
            return {k:unwrap(x)for k,x in v.items()}
        return [unwrap(x)for x in v]if isinstance(v,list)else v
    props={r['index']:unwrap(r['p']) for r in source}
    rate=props[58025]['rate']['lookuptable'][2]
    lifetime=props[27455]['lifetime']['lookuptable'][2]
    assert rate==6 and lifetime==8
    acceleration=props[15613]['acceloverlife']
    acceleration_values=np.array(acceleration['lookuptable'][2:]).reshape(-1,3)
    rotation_curve=props[35274]['lifemultiplier']
    rotation_values=np.array(rotation_curve['lookuptable'][2:]).reshape(-1,3)
    emission_seconds=6.
    count=int(rate*emission_seconds)
    duration=emission_seconds+lifetime
    frame_count=round(duration*30)+1
    times=np.arange(frame_count,dtype=float)/30
    random=np.random.default_rng(4210401)
    basis=np.array([[1.,0.,0.],[0.,0.,1.],[0.,-1.,0.]])
    # All six SCENE04A actors have this pitch/roll. Yaw leaves world gravity
    # unchanged. Convert the module's world-space acceleration into their local
    # emitter frame before CWorldSequencePlayer reapplies the actor transform.
    actor_rotation=Rotation.from_euler('ZYX',[0,17.42431640625,.24169921875],degrees=True).as_matrix()
    materials={};streams={};tracks=[]
    for emitter in range(3):
        mesh=source_mesh('bg_rad_koukusaton_card01k_sm' if emitter==2 else 'bg_rad_koukusaton_card01d_sm')
        velocity=props[64257 if emitter==0 else 64258]['startvelocity']['lookuptable'][2:8]
        for ordinal in range(count):
            joint=len(tracks)
            for material,attributes,indices in mesh:
                materials.setdefault(material,len(materials))
                stream=streams.setdefault(material,dict(position=[],normal=[],uv=[],joint=[],weight=[],index=[],count=0))
                positions=attributes['POSITION']*100
                normals=attributes['NORMAL']
                texcoords=attributes['TEXCOORD_0']
                n=len(positions)
                stream['position'].append(positions);stream['normal'].append(normals);stream['uv'].append(texcoords)
                js=np.zeros((n,4),dtype='<u2');js[:,0]=joint
                ws=np.zeros((n,4),dtype='<f4');ws[:,0]=1
                stream['joint'].append(js);stream['weight'].append(ws)
                stream['index'].append(indices+stream['count']);stream['count']+=n
            birth=ordinal/rate
            initial_velocity=random.uniform(velocity[:3],velocity[3:])
            rotation_rate=random.uniform(-1.,1.,3)+random.uniform(-.2,.2,3)
            # No initial MeshRotation module exists in the native emitter.
            initial_rotation=np.zeros(3)
            position=np.zeros(3);vel=initial_velocity.copy();angles=initial_rotation.copy()
            positions=[];rotations=[];scales=[];previous_age=0.
            for t in times:
                age=max(0.,t-birth);alive=birth<=t<birth+lifetime
                if alive:
                    dt=age-previous_age
                    rel=age/lifetime
                    u=max(0.,min(len(acceleration_values)-1.,(rel-acceleration['lookuptablestarttime'])*acceleration['lookuptabletimescale']))
                    left=int(u);right=min(left+1,len(acceleration_values)-1)
                    acc=acceleration_values[left]*(1-(u-left))+acceleration_values[right]*(u-left)
                    acc=actor_rotation.T@acc
                    position+=vel*dt+acc*.5*dt*dt;vel+=acc*dt
                    u=max(0.,min(len(rotation_values)-1.,(rel-rotation_curve['lookuptablestarttime'])*rotation_curve['lookuptabletimescale']))
                    left=int(u);right=min(left+1,len(rotation_values)-1)
                    mult=rotation_values[left]*(1-(u-left))+rotation_values[right]*(u-left)
                    angles+=rotation_rate*mult*dt*360
                    previous_age=age
                positions.append(basis@position)
                rotation=Rotation.from_euler('ZYX',[angles[2],-angles[1],-angles[0]],degrees=True).as_matrix()
                rotations.append(Rotation.from_matrix(basis@rotation@basis.T).as_quat())
                scales.append([1.,1.,1.] if alive else [.000001]*3)
            tracks.append((positions,rotations,scales))
    document=dict(asset=dict(version='2.0',generator='LostArk native card emitter bake'),
        scene=0,scenes=[dict(nodes=[0,1])],nodes=[dict(name='CardSkeleton',children=list(range(2,len(tracks)+2))),dict(name='CardMesh',mesh=0,skin=0)],
        meshes=[dict(primitives=[])],materials=[dict(name=name)for name in materials],
        buffers=[dict(uri='cards.bin',byteLength=0)],bufferViews=[],accessors=[],
        skins=[dict(joints=list(range(2,len(tracks)+2)),skeleton=0)],animations=[dict(name='card_eruption',samplers=[],channels=[])])
    document['nodes'] += [dict(name=f'card_{i:03d}')for i in range(len(tracks))]
    payload=bytearray()
    def put(values,kind,component=5126):
        data=np.asarray(values,dtype={5126:'<f4',5123:'<u2',5125:'<u4'}[component])
        while len(payload)%4:payload.append(0)
        v=len(document['bufferViews']);a=len(document['accessors'])
        document['bufferViews'].append(dict(buffer=0,byteOffset=len(payload),byteLength=data.nbytes))
        row=dict(bufferView=v,componentType=component,count=len(data),type=kind)
        if kind=='SCALAR':row.update(min=[float(data.min())],max=[float(data.max())])
        document['accessors'].append(row);payload.extend(data.tobytes());return a
    for material,stream in streams.items():
        attributes={key:put(np.concatenate(stream[field]),kind,component)for key,field,kind,component in [
            ('POSITION','position','VEC3',5126),('NORMAL','normal','VEC3',5126),('TEXCOORD_0','uv','VEC2',5126),
            ('JOINTS_0','joint','VEC4',5123),('WEIGHTS_0','weight','VEC4',5126)]}
        document['meshes'][0]['primitives'].append(dict(attributes=attributes,
            indices=put(np.concatenate(stream['index']),'SCALAR',5125),material=materials[material]))
    document['skins'][0]['inverseBindMatrices']=put(np.tile(np.eye(4).reshape(1,16),(len(tracks),1)),'MAT4')
    clock=put(times,'SCALAR');animation=document['animations'][0]
    for joint,(positions,rotations,scales) in enumerate(tracks):
        for path,kind,values in [('translation','VEC3',positions),('rotation','VEC4',rotations),('scale','VEC3',scales)]:
            index=len(animation['samplers'])
            animation['samplers'].append(dict(input=clock,output=put(values,kind),interpolation='LINEAR'))
            animation['channels'].append(dict(sampler=index,target=dict(node=joint+2,path=path)))
    document['buffers'][0]['byteLength']=len(payload)
    (WORK/'cards.gltf').write_text(json.dumps(document),encoding='utf-8');(WORK/'cards.bin').write_bytes(payload)
    OUTPUT.mkdir(parents=True,exist_ok=True)
    args=[str(ROOT/'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe'),str(WORK/'cards.gltf'),'-o',str(OUTPUT/'CardEruption.wmodel'),'--no-auto-textures']
    for material in materials:
        mat=next((WORK/'Source').rglob(material+'.mat'))
        fields=dict(line.split('=',1)for line in mat.read_text().splitlines()if'='in line)
        tex=next((WORK/'Source').rglob(fields['Diffuse']+'.dds'))
        args+=['--material-remap',material+'='+str(tex)]
    subprocess.run(args,check=True)
    report=dict(sourceSystem='fx_q_w_01.fx_par_02.par_q_cardfly_01',sourceEmitters=3,
        spawnRatePerSecond=rate,lifetimeSeconds=lifetime,emissionSeconds=emission_seconds,
        particles=len(tracks),durationMs=round(duration*1000),animation='card_eruption',
        modelAssetId=(OUTPUT/'CardEruption.wmodel').relative_to(ROOT/'Client/Bin/Resources').as_posix(),
        materialSlots=list(materials),randomSeed=4210401,
        remainingSourceBoundaries=['Native FX material MIC programs and camera-offset module are not represented by the mesh bake.'])
    (WORK/'card_eruption.result.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report))


if __name__=='__main__':build()
