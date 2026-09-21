"""Validate additive authored bone clips against the installed WModel skeleton.

This is the publisher-side admission of the Client BoneAnimationDocument format;
CModel remains the only runtime sampler/compiler. No generated native replacement.
"""
from __future__ import annotations
import math
import re
import struct
from collections.abc import Mapping
from pathlib import Path
from typing import Any

SOURCE_REL = 'Data/Animation/Authored/Valtan/Valtan.boneclips.json'
PRODUCT_REL = 'Data/Valtan/Published/Valtan.boneclips.json'
STABLE = re.compile(r'[A-Za-z0-9_.-]{1,127}')


def read_skeleton(path: Path) -> tuple[int, set[str]]:
    data = path.read_bytes()
    def fail(message: str):
        raise ValueError('authored clip skeleton: ' + message)
    if len(data) < 48: fail('truncated WModel')
    magic, major, _, flags, size = struct.unpack_from('<4sHHII', data)
    if magic != b'WINT' or major != 1 or flags or size != len(data)-16: fail('WINT header')
    magic, count, *_ = struct.unpack_from('<4sIII4I', data, 16)
    if magic != b'WMOD' or not 2 <= count <= 4096 or 48+count*64 > len(data): fail('WMOD table')
    sections=[]
    for index in range(count):
        kind, _, offset, length, _ = struct.unpack_from('<IIQQ40s', data, 48+index*64)
        if offset > size or length > size-offset: fail('section bounds')
        if kind == 3: sections.append((16+offset,length))
    if len(sections)!=1: fail('one skeleton required')
    offset,length=sections[0]
    if length<48: fail('truncated WSKL')
    magic,major,_,flags,embedded=struct.unpack_from('<4sHHII',data,offset)
    if magic!=b'WINT' or major!=1 or flags or embedded+16!=length: fail('skeleton WINT')
    magic,bones,sockets,*_=struct.unpack_from('<4sII5I',data,offset+16)
    if magic!=b'WSKL' or not 1<=bones<=512 or sockets>256 or embedded!=32+bones*256+128+sockets*128: fail('WSKL size/count')
    names=set(); hashes=set(); skeleton=0xcbf29ce484222325
    for index in range(bones):
        hashed,raw,parent=struct.unpack_from('<Q64si',data,offset+48+index*256)
        name=raw.split(b'\0',1)[0].decode('ascii','strict')
        if not name or name in names or not hashed or hashed in hashes or not -1<=parent<index: fail('bone identity/hierarchy')
        names.add(name);hashes.add(hashed)
        skeleton=((skeleton^hashed)*0x100000001b3)&0xffffffffffffffff
    return skeleton,names


def validate_document(document: Any, body_path: Path, native_durations: Mapping[str,float]) -> dict[str,int]:
    def fail(message: str): raise ValueError('authored bone clips: '+message)
    def array(obj,key,limit):
        value=obj.get(key) if isinstance(obj,dict) else None
        if not isinstance(value,list) or len(value)>limit: fail(key+' array/count')
        return value
    def number(obj,key,lo,hi,integer=False):
        value=obj.get(key) if isinstance(obj,dict) else None
        if isinstance(value,bool) or not isinstance(value,(int,float)) or not math.isfinite(value) or not lo<=value<=hi or (integer and int(value)!=value): fail(key+' range/type')
        return value
    def stable(value): return isinstance(value,str) and value not in ('.','..') and STABLE.fullmatch(value)
    skeleton,bones=read_skeleton(body_path)
    if not isinstance(document,dict) or document.get('schema')!='lostark.authored-bone-clips' or document.get('formatVersion')!=1 or isinstance(document.get('formatVersion'),bool) or document.get('animationAssetId')!='Valtan': fail('schema/owner')
    hashed=document.get('skeletonHash')
    if not isinstance(hashed,str) or not re.fullmatch('[0-9a-fA-F]{16}',hashed) or int(hashed,16)!=skeleton: fail('skeleton hash mismatch')
    clips={}; sample_count=0
    for clip in array(document,'clips',32):
        if not isinstance(clip,dict): fail('clip object')
        name=clip.get('name');duration=number(clip,'durationMs',1,60000,True)
        if not stable(name) or not name.startswith('authored.') or name in clips or name in native_durations: fail('clip identity/duplicate/native collision')
        clips[name]=clip; total=0; ids=set(); times={0,duration}
        for frame in range(1,1801):
            time=frame*1000//30
            if time>=duration:break
            times.add(time)
        for segment in array(clip,'segments',128):
            if not isinstance(segment,dict):fail('segment object')
            sid=segment.get('id');source=segment.get('sourceClip')
            if not stable(sid) or sid in ids or not isinstance(source,str) or not 0<len(source)<128 or not isinstance(segment.get('loop'),bool):fail('source segment identity')
            ids.add(sid);total+=number(segment,'durationMs',1,60000,True)
            number(segment,'sourceStartMs',0,60000,True);number(segment,'playRate',.01,16)
            times.update((total,total-1))
        if clip['segments'] and total!=duration:fail('segment durations must cover clip')
        tracks=set()
        for track in array(clip,'tracks',len(bones)):
            if not isinstance(track,dict):fail('track object')
            bone=track.get('bone')
            if not isinstance(bone,str) or bone not in bones or bone in tracks:fail('unknown/duplicate bone')
            tracks.add(bone);previous=-1;keys=array(track,'keys',4096)
            if not keys:fail('empty track')
            for key in keys:
                time=number(key,'timeMs',0,duration,True)
                if time<=previous:fail('duplicate/unordered key time')
                previous=time;times.add(time)
                vectors={}
                for field,count in (('position',3),('rotation',4),('scale',3)):
                    vec=array(key,field,count)
                    if len(vec)!=count or any(isinstance(v,bool) or not isinstance(v,(int,float)) or not math.isfinite(v) or abs(v)>100000 for v in vec):fail('nonfinite/invalid vector')
                    vectors[field]=vec
                if min(vectors['scale'])<=.00001 or sum(v*v for v in vectors['rotation'])<.000001:fail('scale/quaternion')
        sample_count+=len(times)*len(bones)
        if sample_count>2000000:fail('two-million sampled-bone limit')
    done=set();visiting=set()
    def visit(name):
        if name in done:return
        if name in visiting:fail('source dependency cycle')
        visiting.add(name)
        for segment in clips[name]['segments']:
            source=segment['sourceClip']
            if source in clips:visit(source);duration=clips[source]['durationMs']
            elif source.startswith('authored.') or source not in native_durations:fail('missing source clip '+source)
            else:duration=native_durations[source]
            if segment['sourceStartMs']>=duration:fail('source start escapes clip')
        visiting.remove(name);done.add(name)
    for name in clips:visit(name)
    return {name:int(clip['durationMs']) for name,clip in clips.items()}
