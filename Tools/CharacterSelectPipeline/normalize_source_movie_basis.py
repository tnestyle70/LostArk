"""Reflect an offline skinned WModel from (X,Z,Y) into source WORLD (X,Z,-Y).

This changes mesh basis/winding/bounds, inverse binds, skeleton locals and all
animation TRS together. It preserves material and unrelated stream bytes. It
does not claim to recover source channels that the input cooker omitted.
"""
from __future__ import annotations
import argparse
from dataclasses import replace
import hashlib
import json
from pathlib import Path
import struct
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'Tools/ModelAssetConverter'))
sys.path.insert(0,str(ROOT/'Tools/ActorXAssetCooker'))
import cook_wmodel_geometry_contract as geom
import verify_dimensionmaster_summon_bind_pose as wm
import retime_wmodel_ticks as retime


def reflect_matrix(blob, offset):
    values=list(struct.unpack_from('<16f',blob,offset))
    for row in range(4):
        for col in range(4):
            if (row==2) != (col==2):
                values[row*4+col] = -values[row*4+col]
    struct.pack_into('<16f',blob,offset,*values)


def negate(blob, offset):
    struct.pack_into('<f',blob,offset,-struct.unpack_from('<f',blob,offset)[0])


def reflect(data):
    parsed=geom.parse_skinned_uv_wmodel(data)
    sections=[]
    for section in parsed['sections']:
        blob=bytearray(section.payload)
        if section.type_id==1:
            h=parsed['meshHeader'];vs=parsed['vertexStart'];ins=parsed['indexStart']
            for index in range(h[5]):
                base=vs+index*h[4]
                for offset in (8,20,40): negate(blob,base+offset)
                if parsed['versionMinor']==5: negate(blob,base+76)
            code='<H' if h[7]==2 else '<I'
            for desc in parsed['submeshes']:
                for index in range(0,desc[3],3):
                    a=ins+desc[2]+index*h[7];b=a+2*h[7]
                    first=struct.unpack_from(code,blob,a)[0];last=struct.unpack_from(code,blob,b)[0]
                    struct.pack_into(code,blob,a,last);struct.pack_into(code,blob,b,first)
            bones=ins+h[6]*h[7]
            for index in range(h[2]): reflect_matrix(blob,bones+index*wm.MESH_BONE.size+44)
            bounds=bones+h[2]*wm.MESH_BONE.size
            if h[8]:
                for index in range(h[1]):
                    at=bounds+index*geom.BOUNDS_V1.size
                    values=list(geom.BOUNDS_V1.unpack_from(blob,at))
                    values[2],values[5],values[8]=-values[5],-values[2],-values[8]
                    geom.BOUNDS_V1.pack_into(blob,at,*values)
        elif section.type_id==3:
            header=wm.SKELETON_HEADER.unpack_from(blob,16)
            for index in range(header[1]):
                reflect_matrix(blob,16+wm.SKELETON_HEADER.size+index*wm.SKELETON_BONE.size+76)
        elif section.type_id==4:
            header=wm.ANIMATION_HEADER.unpack_from(blob,16)
            channel=16+wm.ANIMATION_HEADER.size
            keys=channel+header[1]*wm.ANIMATION_CHANNEL.size
            for index in range(header[1]):
                row=wm.ANIMATION_CHANNEL.unpack_from(blob,channel+index*wm.ANIMATION_CHANNEL.size)
                for k in range(row[1]): negate(blob,keys+row[2]+k*wm.VECTOR_KEY.size+12)
                for k in range(row[3]):
                    at=keys+row[4]+k*wm.QUATERNION_KEY.size
                    negate(blob,at+4);negate(blob,at+8)
        sections.append(replace(section,payload=bytes(blob)))
    mesh=next(s.payload for s in sections if s.type_id==1)
    result=geom.rebuild_wmodel(parsed['modelHeader'],sections,mesh)
    geom.parse_skinned_uv_wmodel(result)
    return result


def source_join(data,path):
    """Pin every output indexed corner and preserve omitted source channels."""
    parsed=geom.parse_skinned_uv_wmodel(data);h=parsed['meshHeader'];mesh=parsed['mesh']
    doc=json.loads(path.read_text(encoding='utf-8-sig'))
    assert len(doc['meshes'])==1
    primitives=doc['meshes'][0]['primitives'];assert len(primitives)==len(parsed['submeshes'])
    cache={};sidecars=[];position_error=uv_error=0.
    for primitive,desc in zip(primitives,parsed['submeshes']):
        attrs={name:geom.accessor_values(doc,path.parent,ref,cache)[0] for name,ref in primitive['attributes'].items()}
        source_indices=[v[0] for v in geom.accessor_values(doc,path.parent,primitive['indices'],cache)[0]]
        target_indices=struct.unpack_from('<'+('H' if h[7]==2 else 'I')*desc[3],mesh,parsed['indexStart']+desc[2])
        assert len(source_indices)==len(target_indices)
        joined={}
        for source_index,target_index in zip(source_indices,target_indices):
            at=parsed['vertexStart']+desc[0]+target_index*h[4]
            p=struct.unpack_from('<3f',mesh,at);uv=struct.unpack_from('<2f',mesh,at+24)
            expected=struct.unpack('<3f',struct.pack('<3f',*(x*100. for x in attrs['POSITION'][source_index])))
            pe=max(abs(a-b) for a,b in zip(p,expected));ue=max(abs(a-b)for a,b in zip(uv,attrs['TEXCOORD_0'][source_index]))
            assert pe<=2e-5 and ue<=1e-6,(pe,ue,source_index,target_index)
            position_error=max(position_error,pe);uv_error=max(uv_error,ue)
            value={name:values[source_index] for name,values in attrs.items() if name not in ('POSITION','JOINTS_0','WEIGHTS_0')}
            if target_index in joined: assert joined[target_index]==value,'Cook welded distinct source channels; split required'
            joined[target_index]=value
        assert len(joined)==desc[1]
        sidecars.append(dict(materialIndex=desc[4],vertexCount=desc[1],sourceAttributes=list(attrs),
            indexedSourceChannels=[joined[i] for i in range(desc[1])]))
    return dict(maxPositionErrorCm=position_error,maxUv0Error=uv_error,
        sourceGltf=str(path),sourceGltfSha256=hashlib.sha256(path.read_bytes()).hexdigest(),
        sourceBufferSha256=[hashlib.sha256(v).hexdigest()for _,v in sorted(cache.items())],submeshes=sidecars)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for arg in ('input','source-gltf','output','receipt'): parser.add_argument('--'+arg,type=Path,required=True)
    args=parser.parse_args()
    source=args.input.resolve();target=args.output.resolve();receipt=args.receipt.resolve()
    for p in (target,receipt):
        if not p.is_relative_to((ROOT/'out').resolve()):raise ValueError('Output must remain under repository out')
    if target==source or target.exists():raise ValueError('Write a new candidate, never overwrite input/output')
    original=source.read_bytes();result=reflect(original)
    assert reflect(result)==original,'Z reflection is not an exact involution'
    joined=source_join(result,args.source_gltf.resolve())
    data=bytearray(result)
    has_animation=any(s.type_id==4 for s in geom.parse_skinned_uv_wmodel(result)['sections'])
    timing=retime.retime(data,30.,None) if has_animation else []
    if has_animation:retime.verify(data,timing,30.)
    target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
    wm.read_wmodel(target,include_geometry=True)
    channels=receipt.with_suffix('.source-channels.json')
    channels.write_text(json.dumps(joined,separators=(',',':'),allow_nan=False)+'\n',encoding='utf-8')
    summary=dict(input=str(source),inputSha256=hashlib.sha256(original).hexdigest(),output=str(target),
        outputSha256=hashlib.sha256(data).hexdigest(),involutionByteIdentical=True,
        basis=[[1,0,0],[0,0,1],[0,-1,0]],translationScale=1,modelPreScale=.01,
        sourceChannelSidecar=str(channels),maxPositionErrorCm=joined['maxPositionErrorCm'],
        maxUv0Error=joined['maxUv0Error'],timing=timing,
        boundary='Existing channels reflected. Original normal/tangent sign/COLOR0/extraUV sidecar is source-joined, not yet fully consumed by all runtime skinned formats.')
    receipt.write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({k:v for k,v in summary.items()if k!='timing'}))


if __name__=='__main__':main()
