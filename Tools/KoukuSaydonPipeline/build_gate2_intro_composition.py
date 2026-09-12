"""Bake the SCENE04A entrance into existing World Sequence/Composition inputs.

The installed source UPK is read-only. Actor animation is sampled offline into
one ordinary WANM clip, preserving Matinee preroll, trimming, reverse, rates and
the A-over-B slot blend. Runtime still uses CModel and CWorldSequencePlayer.
The original scene coordinates are retained, including its underground set.
"""
from __future__ import annotations

import argparse
import copy
import json
import math
import os
import shlex
import shutil
import struct
import sys
import tempfile
from pathlib import Path

import numpy as np
from scipy.spatial.transform import Rotation

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/LevelPlacementExtractor"))
sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
from extract_ue3_effect_material_closure import load_package
import extract_ue3_placements as ue3
import verify_dimensionmaster_summon_bind_pose as wm
from build_gate2_intro_backdrops import build_backdrops, reduced_indices

AREA = "LV_LUT_MIDNIGHTC_ED"
AREA_DIR = ROOT / "Data/Maps/Authoring" / AREA
RESOURCES = ROOT / "Client/Bin/Resources"
EVIDENCE = ROOT / "out/KoukuGate2Restore20260911"
PACKAGES = Path("C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages")
DURATION = 27000
PATTERN_ID = "KAKULSAYDON_G1_PATTERN_3"
PREFIX = "kouku.gate2.intro"
BASIS = np.array([[1., 0., 0.], [0., 0., 1.], [0., -1., 0.]])
BONE_MODELS = {}


def read(path):
    return json.loads(Path(path).read_text(encoding="utf-8-sig"))


def write(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2,
                               allow_nan=False) + "\n", encoding="utf-8")


def unwrap(value):
    if isinstance(value, dict):
        if "type" in value and "value" in value:
            return unwrap(value["value"])
        if "properties" in value:
            return unwrap(value["properties"])
        return {k: unwrap(v) for k, v in value.items()}
    if isinstance(value, list):
        return [unwrap(v) for v in value]
    if isinstance(value, str):
        try:
            return value.encode("latin1").decode("cp949")
        except (UnicodeEncodeError, UnicodeDecodeError):
            return value
    return value


def extract_scene(package_path=None):
    package = load_package(package_path or PACKAGES / "B9AVB2VAZIQRPQCJVKAVYRAVOKYPY806.upk",
                           ue3.LOSTARK_KR_AES_KEY)
    original_decoder = ue3.decode_property_value
    def decoder(pt, st, payload, names, bool_value, pn=None, owner=None):
        if pt.lower() == "structproperty" and str(st).lower() == "interplookuptrack":
            properties, _ = ue3.parse_tagged_properties_at(payload,names,0,st)
            return dict(properties=properties)
        return original_decoder(pt,st,payload,names,bool_value,pn,owner)
    ue3.decode_property_value = decoder
    rows = {}
    for entry in package.exports:
        raw = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        try:
            properties, _ = ue3.parse_tagged_properties(raw, package.names, package.summary.version)
        except ue3.ExtractionError:
            # Empty property streams are normal for inherited no-op tracks.
            properties = {}
        rows[entry.index + 1] = dict(index=entry.index + 1,
            name=ue3.package_ref_path(entry.index + 1, package.imports, package.exports),
            cls=ue3.package_ref_name(entry.class_index, package.imports, package.exports),
            p=unwrap(properties))
    ue3.decode_property_value = original_decoder
    imports = {str(-x.index - 1): ue3.package_ref_path(-x.index - 1, package.imports, package.exports)
               for x in package.imports}
    if package_path is None:
        assert rows[394]["p"]["interplength"] == 27
        assert len(rows[394]["p"]["interpgroups"]) == 98
    return rows, imports


def vec(value, default=(0., 0., 0.)):
    if isinstance(value, dict):
        return np.array([value.get(k, default[i]) for i, k in enumerate("xyz")], dtype=float)
    return np.array(default, dtype=float)


def curve(points, seconds, default):
    if not points:
        return np.array(default, dtype=float)
    def value(row, name):
        result = row.get(name, 0.)
        return vec(result) if isinstance(result, dict) else np.asarray(result)
    if seconds <= points[0]["inval"]:
        return value(points[0], "outval")
    for a, b in zip(points, points[1:]):
        if seconds >= b["inval"] - 0.000001:
            continue
        dt = b["inval"] - a["inval"]
        u = (seconds - a["inval"]) / dt
        mode = a.get("interpmode", "cim_linear")
        if mode == "cim_constant":
            return value(a, "outval")
        if mode == "cim_linear":
            return value(a, "outval") * (1-u) + value(b, "outval") * u
        return ((2*u**3-3*u*u+1)*value(a, "outval") +
                (u**3-2*u*u+u)*dt*value(a, "leavetangent") +
                (-2*u**3+3*u*u)*value(b, "outval") +
                (u**3-u*u)*dt*value(b, "arrivetangent"))
    return value(points[-1], "outval")


def rotation(euler):
    roll, pitch, yaw = euler
    return Rotation.from_euler("ZYX", [yaw, -pitch, -roll], degrees=True).as_matrix()


def actor_rotation(properties):
    d = properties.get("rotation", {}).get("degrees", {})
    return rotation([d.get("roll", 0), d.get("pitch", 0), d.get("yaw", 0)])


def active_tracks(rows, group):
    if group is None:
        return []
    return [rows[t] for t in rows[group]["p"].get("interptracks", [])
            if not rows[t]["p"].get("bdisabletrack", False)]


def group_actor(rows, group, matinee=329):
    name = rows[group]["p"].get("groupname")
    link = next((x for x in rows[matinee]["p"]["variablelinks"]
                 if x["linkdesc"].casefold() == str(name).casefold()), {})
    return [rows[v]["p"]["objvalue"] for v in link.get("linkedvariables", [])
            if rows[v]["p"].get("objvalue")]


def world_pose(rows, group, actor, seconds, matinee=329, interp_data=394):
    p = rows[actor]["p"]
    location = vec(p.get("location"))
    base_rot = actor_rotation(p)
    track_base_position = None
    track_base_rotation = None
    attached = p.get('base') in BONE_MODELS and p.get('basebonename')
    if attached:
        parent, parent_group, cache = BONE_MODELS[p['base']]
        anim = parent.animations[0]
        sample_time=min(anim.duration_ticks/anim.ticks_per_second,max(0.,seconds))
        cache_key=round(sample_time*1000000)
        if cache_key not in cache:
            sampled=pose_sample(parent,anim,sample_time)
            local=[wm.affine_matrix(s,q,t)for t,q,s in sampled]
            cache[cache_key]=[np.array(x).reshape(4,4)for x in wm.combined_transforms(parent.skeleton_bones,local)]
        idx=next(i for i,b in enumerate(parent.skeleton_bones)if b.name.lower()==p['basebonename'].lower())
        bone=cache[cache_key][idx].copy();bone[3,:3]*=.01
        bone_rot=bone[:3,:3].T
        bone_rot/=np.linalg.norm(bone_rot,axis=0)
        pp,pr=world_pose(rows,parent_group,p['base'],seconds,matinee,interp_data)
        scale=rows[p['base']]['p'].get('drawscale',1.)
        track_base_position=BASIS.T@(pp+pr@(bone[3,:3]*scale))*100
        track_base_rotation=BASIS.T@(pr@bone_rot)@BASIS
        relative=BASIS@vec(p.get('relativelocation'))*.01
        location_client=pp+pr@(bone[3,:3]*scale+bone_rot@relative)
        relprops={'rotation':p.get('relativerotation',{})}
        attached_rot=pr@bone_rot@(BASIS@actor_rotation(relprops)@BASIS.T)
        if p.get('bignorebaserotation'):attached_rot=BASIS@base_rot@BASIS.T
        location=BASIS.T@location_client*100
        base_rot=BASIS.T@attached_rot@BASIS
    elif p.get('base') in rows:
        parent=p['base']
        parent_group=next((g for g in rows[interp_data]['p']['interpgroups']if parent in group_actor(rows,g,matinee)),None)
        pp,pr=world_pose(rows,parent_group,parent,seconds,matinee,interp_data)
        track_base_position=BASIS.T@pp*100
        track_base_rotation=BASIS.T@pr@BASIS
        if 'relativelocation' in p:
            relative=BASIS@vec(p['relativelocation'])*.01
        else:
            initial=rows[parent]['p']
            initial_rot=BASIS@actor_rotation(initial)@BASIS.T
            relative=initial_rot.T@(BASIS@(location-vec(initial.get('location')))*.01)
        location_client=pp+pr@relative
        attached_rot=pr@(BASIS@actor_rotation({'rotation':p.get('relativerotation',{})})@BASIS.T)
        if p.get('bignorebaserotation'):
            attached_rot=BASIS@base_rot@BASIS.T
        location=BASIS.T@location_client*100
        base_rot=BASIS.T@attached_rot@BASIS
    move = [x["p"] for x in active_tracks(rows, group) if x["cls"] == "interptrackmove"]
    if move:
        assert len(move) == 1, (group, "multiple active Move tracks")
        m = move[0]
        pos_points=copy.deepcopy(m.get("postrack", {}).get("points", []))
        rot_points=copy.deepcopy(m.get("eulertrack", {}).get("points", []))
        group_names={rows[g]['p'].get('groupname'):g for g in rows[interp_data]['p']['interpgroups']}
        for n,lookup in enumerate(m.get('lookuptrack',{}).get('points',[])):
            name=lookup.get('groupname','none')
            if name=='none':continue
            target=group_names[name]; target_actor=group_actor(rows,target,matinee)[0]
            lp,lr=world_pose(rows,target,target_actor,seconds,matinee,interp_data)
            ue_rot=BASIS.T@lr@BASIS
            yaw,pitch,roll=Rotation.from_matrix(ue_rot).as_euler('ZYX',degrees=True)
            pos_points[n]['outval']=dict(zip('xyz',BASIS.T@lp*100))
            rot_points[n]['outval']=dict(x=-roll,y=-pitch,z=yaw)
        pos = curve(pos_points, seconds, location)
        ang = curve(rot_points, seconds, [0, 0, 0])
        rot = rotation(ang)
        if m.get("moveframe") == "imf_relativetoinitial":
            pos = base_rot @ pos + location
            rot = base_rot @ rot
        elif track_base_position is not None:
            # UE3 attached Move keys are stored in the parent/bone frame.
            # Treating these local keys as absolute world positions strands
            # camera-attached backgrounds and the popup book's moving props.
            pos = track_base_rotation @ pos + track_base_position
            rot = track_base_rotation @ rot
    else:
        pos, rot = location, base_rot
    return BASIS @ pos * .01, BASIS @ rot @ BASIS.T


def clip_name(name):
    value = name.lower()
    for prefix in ("rpct00_", "rpcz00_", "rpct06_", "ao_"):
        if value.startswith(prefix):
            value = value[len(prefix):]
    return value.rsplit(".ao_", 1)[-1]


def pose_sample(model, anim, seconds):
    ticks = min(anim.duration_ticks, max(0., seconds * anim.ticks_per_second))
    out = []
    channels = {c.bone_index: c for c in anim.channels}
    for i, bone in enumerate(model.skeleton_bones):
        c = channels.get(i)
        if c:
            out.append((np.array(wm.sample_vector(c.position_keys, ticks, (0, 0, 0))),
                        np.array(wm.sample_quaternion(c.rotation_keys, ticks)),
                        np.array(wm.sample_vector(c.scale_keys, ticks, (1, 1, 1)))))
        else:
            mat = np.array(bone.transform).reshape(4, 4)
            scale = np.linalg.norm(mat[:3, :3], axis=1)
            out.append((mat[3, :3], Rotation.from_matrix((mat[:3, :3] / scale[:, None]).T).as_quat(), scale))
    return out


def anim_at(track, animations, seconds):
    keys = track["animseqs"]
    key = next((k for k in reversed(keys) if k["starttime"] <= seconds + 0.000001), keys[0])
    anim = animations[key["animseqname"]]
    length = anim.duration_ticks / anim.ticks_per_second
    start, end = key.get("animstartoffset", 0.), length-key.get("animendoffset", 0.)
    span = end-start
    assert span > 0, key
    age = max(0., seconds-key["starttime"]) * key.get("animplayrate", 1.)
    age = age % span if key.get("blooping") else min(age, span)
    return anim, end-age if key.get("breverse") else start+age


def slerp(a, b, alpha):
    # Existing offline CModel verifier's interpolation, including antipodes.
    return np.array(wm.sample_quaternion([(0., *a), (1., *b)], alpha))


def visible_table_mesh(payload):
    """SCENE04A Table overrides material slots 0 and 3 with transparent_inst."""
    nested = list(wm.FILE_HEADER.unpack_from(payload))
    header = list(wm.MESH_HEADER.unpack_from(payload, wm.FILE_HEADER.size))
    assert nested[:4] == [b"WINT", 1, 0, 0]
    assert nested[-1] == len(payload) - wm.FILE_HEADER.size
    assert header[1] == 4 and header[4] == 76
    table_start = wm.FILE_HEADER.size + wm.MESH_HEADER.size
    rows = [list(wm.SUBMESH_DESC.unpack_from(payload, table_start+i*wm.SUBMESH_DESC.size)) for i in range(4)]
    vertex_start = table_start + 4*wm.SUBMESH_DESC.size
    index_start = vertex_start + header[5]*header[4]
    tail_start = index_start + header[6]*header[7]
    bone_end = tail_start + header[2] * wm.MESH_BONE.size
    bounds_stride = 40  # Engine MESH_BOUNDS_V1, one entry per submesh.
    bounds_end = bone_end + (header[1] * bounds_stride if header[8] else 0)
    assert bounds_end == len(payload), "Table legacy WMSH has an unsupported tail"
    vertices, indices, kept, kept_bounds = bytearray(), bytearray(), [], bytearray()
    for original_slot, row in enumerate(rows):
        if row[4] in (0, 3):
            continue
        original_vertex, count, original_index, index_count = row[:4]
        row[0], row[2] = len(vertices), len(indices)
        vertices += payload[vertex_start+original_vertex:vertex_start+original_vertex+count*header[4]]
        indices += payload[index_start+original_index:index_start+original_index+index_count*header[7]]
        kept.append(row)
        if header[8]:
            start = bone_end + original_slot * bounds_stride
            kept_bounds += payload[start:start + bounds_stride]
    header[1], header[5], header[6] = len(kept), len(vertices)//header[4], len(indices)//header[7]
    body = wm.MESH_HEADER.pack(*header) + b''.join(wm.SUBMESH_DESC.pack(*r)for r in kept) + vertices + indices + payload[tail_start:bone_end] + kept_bounds
    nested[-1] = len(body)
    return wm.FILE_HEADER.pack(*nested)+body


def write_clip(source, destination, model, samples, name, label):
    """Keep mesh/skeleton/material bytes; replace animations with one WANM."""
    data = source.read_bytes()
    file_header = list(wm.FILE_HEADER.unpack_from(data))
    header = list(wm.MODEL_HEADER.unpack_from(data, wm.FILE_HEADER.size))
    sections = []
    skeleton_trailer = None
    for i in range(header[1]):
        kind, index, offset, size, raw_name = wm.SECTION_DESC.unpack_from(
            data, wm.FILE_HEADER.size+wm.MODEL_HEADER.size+i*wm.SECTION_DESC.size)
        if kind != 4:
            section = data[16+offset:16+offset+size]
            if kind == 1 and label == 'Table':
                section = visible_table_mesh(section)
            sections.append([kind, index, raw_name, section])
        elif skeleton_trailer is None:
            skeleton_trailer = data[16+offset+size-8:16+offset+size]
    key_bytes = bytearray()
    channel_bytes = bytearray()
    n = len(samples)
    for bone, b in enumerate(model.skeleton_bones):
        offsets = []
        for channel, fmt in [(0, wm.VECTOR_KEY), (1, wm.QUATERNION_KEY), (2, wm.VECTOR_KEY)]:
            offsets.append(len(key_bytes))
            for frame, sample in enumerate(samples):
                key_bytes += fmt.pack(float(frame), *sample[bone][channel])
        channel_bytes += wm.ANIMATION_CHANNEL.pack(b.name_hash,n,offsets[0],n,offsets[1],n,offsets[2],bone,0)
    assert skeleton_trailer is not None
    payload = (wm.ANIMATION_HEADER.pack(b"WANM",len(model.skeleton_bones),float(n-1),30.,n*3*len(model.skeleton_bones),0,0,b"\0"*7)
               +channel_bytes+key_bytes+skeleton_trailer)
    payload = wm.FILE_HEADER.pack(b"WINT",1,file_header[2],0,len(payload))+payload
    sections.append([4,0,name.encode().ljust(40,b"\0"),payload])
    header[1], header[2] = len(sections), 1
    table = bytearray()
    body = bytearray()
    offset = wm.MODEL_HEADER.size + len(sections)*wm.SECTION_DESC.size
    for kind, index, raw_name, payload in sections:
        table += wm.SECTION_DESC.pack(kind,index,offset,len(payload),raw_name)
        body += payload
        offset += len(payload)
    payload = wm.MODEL_HEADER.pack(*header)+table+body
    file_header[-1] = len(payload)
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(wm.FILE_HEADER.pack(*file_header)+payload)
    texture_source = source.parent / "textures"
    if texture_source.is_dir():
        shutil.copytree(texture_source, destination.parent/"textures", dirs_exist_ok=True)
    check = wm.read_wmodel(destination)
    assert len(check.animations) == 1 and check.animations[0].name == name
    assert abs(check.animations[0].duration_ticks / check.animations[0].ticks_per_second - (n-1)/30.) < .0001
    return check


def bake_actor(rows, group, source, label):
    model = wm.read_wmodel(source)
    animations = {clip_name(a.name): a for a in model.animations}
    tracks = {r["p"]["slotname"]: r["p"] for r in active_tracks(rows, group)
              if r["cls"] == "interptrackanimcontrol" and r["p"].get("animseqs")}
    assert set(tracks) <= {"a", "b"} and "a" in tracks
    for track in tracks.values():
        for key in track["animseqs"]:
            assert key["animseqname"] in animations, (label, key["animseqname"])
    controls=[]
    if label in ('Saydon','HandBook'):
        tree_file='animblending_kuk2' if label=='Saydon' else 'evt2_animblending_mix_scale'
        tree_rows={r['index']:dict(r,p=unwrap(r['p']))for r in read(EVIDENCE/(tree_file+'.json'))}
        tree=next(r for r in tree_rows.values()if r['cls']=='animtree')
        strength={r['p']['skelcontrolname']:r['p']for r in active_tracks(rows,group)
                  if r['cls']=='interptrackskelcontrolstrength'}
        bone_indices={b.name.lower():i for i,b in enumerate(model.skeleton_bones)}
        for chain in tree['p'].get('skelcontrollists',[]):
            idx=bone_indices.get(chain['bonename'].lower());ref=chain['controlhead']
            while ref:
                c=tree_rows[ref]['p'];ref=c.get('nextcontrol',0)
                if idx is not None and c.get('controlname') in strength:
                    controls.append((idx,c,strength[c['controlname']]))
    samples = []
    for frame in range(811):
        seconds = frame / 30.
        anim, t = anim_at(tracks["a"], animations, seconds)
        a = pose_sample(model, anim, t)
        if "b" in tracks:
            anim, t = anim_at(tracks["b"], animations, seconds)
            b = pose_sample(model, anim, t)
            alpha = float(curve(tracks["a"].get("floattrack", {}).get("points", []), seconds, 1.))
            a = [(bp*(1-alpha)+ap*alpha, slerp(bq,aq,alpha), bs*(1-alpha)+asc*alpha)
                 for (ap,aq,asc),(bp,bq,bs) in zip(a,b)]
        for index,control,strength in controls:
            alpha=float(curve(strength.get('floattrack',{}).get('points',[]),seconds,control.get('controlstrength',1.)))
            p,q,s=a[index]
            if control.get('bapplytranslation'):
                delta=BASIS@vec(control.get('bonetranslation'))*(.01 if label=='Saydon' else 1.)
                p=p+delta*alpha if control.get('baddtranslation') else p*(1-alpha)+delta*alpha
            if control.get('bapplyrotation'):
                delta=BASIS@actor_rotation({'rotation':control.get('bonerotation',{})})@BASIS.T
                target=Rotation.from_matrix(delta@Rotation.from_quat(q).as_matrix()).as_quat() if control.get('baddrotation') else Rotation.from_matrix(delta).as_quat()
                q=slerp(q,target,alpha)
            if 'bonescale' in control:s=s*(1-alpha+alpha*control['bonescale'])
            a[index]=(p,q,s)
        samples.append(a)
    destination = RESOURCES / f"Map/KakulSaydon/Gate2Intro/{label}/{label}.wmodel"
    checked = write_clip(source,destination,model,samples,"gate2_intro_27s",label)
    return destination.relative_to(RESOURCES).as_posix(), checked


def source_times(rows, group, start=0, end=DURATION, step=150):
    times = set(range(start,end,step)) | {start,end}
    for track in active_tracks(rows,group):
        for field in ("postrack","eulertrack","floattrack"):
            points = track["p"].get(field,{}).get("points",[])
            for a,b in zip(points,points[1:]):
                ms = round(b["inval"]*1000)
                if start < ms < end:
                    times.add(ms)
                    if a.get("interpmode") == "cim_constant":times.add(ms-1)
    assert len(times) <= 256, (group,len(times))
    return sorted(times)


def object_resource(label, model, animated, pre_scale=.01, material_source=""):
    value = dict(objectId=f"world.object.{PREFIX}.{label.lower()}",displayName=label,
        modelAssetId=model,anchorKind="WORLD",diffuseTextureAssetId="",modelPreScale=pre_scale,
        animated=animated,scale=[1,1,1],sequenceInstanceId="",defaultMotionInstanceId="")
    if material_source:value["materialSourceModelAssetId"] = material_source
    return value


def make_world(rows, group, actor, resource, label):
    tag = label.lower()
    seqid, instid = f"sequence.{PREFIX}.{tag}", f"world.sequence.instance.{PREFIX}.{tag}"
    scale = vec(rows[actor]["p"].get("drawscale3d"),(1,1,1))[[0,2,1]]*rows[actor]["p"].get("drawscale",1.)
    keys=[]
    for ms in source_times(rows,group):
        p,r=world_pose(rows,group,actor,ms/1000.)
        q=Rotation.from_matrix(r).as_quat()
        if q[3]<0:q=-q
        keys.append(dict(timeMs=ms,positionOffset=p.tolist(),rotationQuaternion=q.tolist(),
                         scaleMultiplier=scale.tolist(),visible=True))
    template = dict(sequenceId=seqid,displayName="2관문 진입 / "+label,category="World",durationMs=DURATION,
        interpolation="LINEAR",tracks=[dict(slotId="actor",keys=keys)],animationTracks=[])
    if resource["animated"]:
        template["animationTracks"]=[dict(slotId="actor",clipName="gate2_intro_27s",startMs=0,
            playbackRate=1,loop=False,holdLastFrame=True)]
    instance=dict(instanceId=instid,templateId=seqid,enabled=True,startDelayMs=0,playbackSpeed=1,
        anchorKind="WORLD",position=[0,0,0],motionEnd="STOP",nextMotionId="",
        bindings=[dict(slotId="actor",targetKind="OBJECT_RESOURCE",targetId=resource["objectId"])])
    world=dict(worldId=f"world.{PREFIX}.{tag}",displayName="2관문 진입 / "+label,sequenceInstanceId=instid,
        positionOffset=[0,0,0],anchorKind="NONE",anchorPosition=[0,0,0],companionEffectResourceId="")
    resource["defaultMotionInstanceId"]=instid
    return template,instance,world


def make_cameras(rows, matinee=329, interp_data=394, duration=DURATION,
                 prefix=PREFIX, display_name="2관문 진입", range_start=0):
    directors=[x["p"]["cuttrack"] for g in rows[interp_data]["p"]["interpgroups"]
               for x in active_tracks(rows,g) if x["cls"]=="interptrackdirector"]
    assert len(directors)==1, (matinee,"Expected one enabled Director")
    director=directors[0]
    groups={rows[g]["p"].get("groupname"):g for g in rows[interp_data]["p"]["interpgroups"]}
    shots=[]
    for n,cut in enumerate(director):
        start=max(range_start,round(cut["time"]*1000)); end=min(duration,round(director[n+1]["time"]*1000)) if n+1<len(director) else duration
        if end<=start:continue
        group=groups[cut["targetcamgroup"]];actor=group_actor(rows,group,matinee)[0]
        fov_tracks=[x["p"] for x in active_tracks(rows,group) if x["cls"]=="interptrackfloatprop"
                    and x["p"].get("propertyname","").lower()=="fovangle"]
        def sample(ms):
            p,r=world_pose(rows,group,actor,ms/1000.,matinee,interp_data)
            fov=float(curve(fov_tracks[-1].get("floattrack",{}).get("points",[]),ms/1000.,90.)) if fov_tracks else 90.
            # UE3 FOV is horizontal; current composition viewport is 16:9.
            assert 0 < fov <= 180., (matinee,group,ms,fov)
            # Original zoom keys can hit zero or 180 degrees. Perspective
            # projection is singular there; retain source keys in evidence and
            # project into the runtime's strict (1,179) vertical-FOV range.
            fovy=max(1.1,min(178.9,math.degrees(2*math.atan(math.tan(math.radians(fov)/2)/(16/9)))))
            return p,Rotation.from_matrix(r).as_quat(),fovy

        # A fixed 150ms sample missed 63cm of the fast pullback. Reduce against
        # the actual source trajectory at 60Hz, preserving authored boundaries.
        candidates=sorted(set(range(start,end,16))|set(source_times(rows,group,start,end,end-start+1)))
        poses={t:sample(t)for t in candidates}
        def reduced_times(times):
            keep={0,len(times)-1}
            while True:
                ordered=sorted(keep);worst=(1.,None)
                for left,right in zip(ordered,ordered[1:]):
                    a,bp=poses[times[left]],poses[times[right]]
                    for index in range(left+1,right):
                        t=times[index];u=(t-times[left])/(times[right]-times[left]);p,q,f=poses[t]
                        iq=slerp(a[1],bp[1],u)
                        angle=math.degrees(2*math.acos(min(1.,abs(float(np.dot(q,iq))))))
                        error=max(float(np.linalg.norm(p-(a[0]*(1-u)+bp[0]*u)))/.005,angle/.1,abs(f-(a[2]*(1-u)+bp[2]*u))/.02)
                        if error>worst[0]:worst=(error,index)
                if worst[1] is None:return [[times[i]for i in sorted(keep)]]
                if len(keep)==64:
                    mid=len(times)//2
                    return reduced_times(times[:mid+1])+reduced_times(times[mid:])
                keep.add(worst[1])
        for segment_times in reduced_times(candidates):
            segment_start,segment_end=segment_times[0],segment_times[-1]
            keys=[]
            for ms in segment_times:
                p,q,fovy=poses[ms];r=Rotation.from_quat(q).as_matrix()
                forward=r@np.array([1.,0.,0.])
                keys.append(dict(sceneId=f"{prefix}.camera{len(shots)+1}.k{len(keys)}",timeMs=ms-segment_start,
                    eye=p.tolist(),lookAt=(p+forward*10).tolist(),up=(r@np.array([0.,1.,0.])).tolist(),fovYDegrees=fovy))
            shot=dict(shotId=f"{prefix}.camera.{len(shots)+1}",displayName=f"{display_name} 카메라 / {cut['targetcamgroup']}",
                defaultHoldMs=segment_end-segment_start,transitionEasing="LINEAR",activation="PATTERN_ONLY",sequenceInstanceId="",
                box=dict(center=keys[0]["eye"],halfExtents=[1,1,1],yawDegrees=0),eye=keys[0]["eye"],lookAt=keys[0]["lookAt"],
                fovYDegrees=keys[0]["fovYDegrees"],blendInMs=0,blendOutMs=0,priority=100,
                cameraTrack=dict(durationMs=segment_end-segment_start,interpolation="LINEAR",easing="LINEAR",keyframes=keys))
            shots.append((segment_start-range_start,segment_end-range_start,shot))
    return shots


def replace_row(path, list_key, identity_key, row):
    """Patch one JSON row without reformatting other users' authoring."""
    raw=path.read_text(encoding="utf-8")
    decoder=json.JSONDecoder()
    start=raw.index('"'+list_key+'"')
    start=raw.index('[',start)+1
    cursor=start
    while True:
        while raw[cursor].isspace() or raw[cursor]==',':cursor+=1
        if raw[cursor]==']':break
        value,end=decoder.raw_decode(raw,cursor)
        if value.get(identity_key)==row[identity_key]:
            replacement=json.dumps(row,ensure_ascii=False,indent=2,allow_nan=False)
            replacement=replacement.replace('\n','\n    ')
            path.write_text(raw[:cursor]+replacement+raw[end:],encoding="utf-8")
            return
        cursor=end
    raise ValueError((path,list_key,row[identity_key],"missing target row"))


def append_rows(path, list_key, identity_key, rows):
    if not rows:return
    raw=path.read_text(encoding="utf-8")
    decoder=json.JSONDecoder();start=raw.index('"'+list_key+'"');start=raw.index('[',start)
    values,end=decoder.raw_decode(raw,start)
    existing={v[identity_key] for v in values}
    for row in rows:
        if row[identity_key] in existing:
            replace_row(path,list_key,identity_key,row)
    rows=[r for r in rows if r[identity_key] not in existing]
    if not rows:return
    raw=path.read_text(encoding="utf-8");start=raw.index('"'+list_key+'"');start=raw.index('[',start)
    values,end=decoder.raw_decode(raw,start)
    body=raw[start+1:end-1].rstrip()
    suffix=',\n' if values else '\n'
    insertion=',\n'.join('    '+json.dumps(r,ensure_ascii=False,indent=2,allow_nan=False).replace('\n','\n    ') for r in rows)
    path.write_text(raw[:start+1]+body+suffix+insertion+'\n  '+raw[end-1:],encoding="utf-8")


def presentation_occurrence(resource, ordinal, start, end, anchor_kind="WORLD"):
    return dict(occurrenceId=f"{PATTERN_ID}.presentation.{ordinal}",resourceId=resource,startMs=start,durationMs=end-start,
        positionOffset=[0,0,0],rotationDegrees=[0,0,0],scale=[1,1,1],fadeInMs=0,fadeOutMs=0,dissolveStart=.95,
        dissolveEnd=1,brightnessMultiplier=1,volume=1,followBoss=False,debugRender=False,bone="",boneTarget="BODY",
        regionId="",cardSymbol="NONE",cardColor="NONE",anchorKind=anchor_kind,worldId="",logicOccurrenceId="",worldOccurrenceId="")


def install_sequence_pattern(path, pattern, worlds, presentations):
    """Keep the independent Sequence and every referenced definition in one save."""
    baseline = path.read_bytes()
    document = json.loads(baseline.decode("utf-8-sig"))
    if document.get("compositionId") != "boss.composition.kakulsaydon.sequencer":
        raise ValueError("Gate intro must target the independent Sequence composition")
    for key, id_key, rows in (("worlds", "worldId", worlds),
                              ("presentationResources", "resourceId", presentations)):
        additions = {row[id_key]: copy.deepcopy(row) for row in rows}
        if len(additions) != len(rows):
            raise ValueError("Duplicate generated Sequence definition: " + key)
        document[key] = [additions.pop(row[id_key], row) for row in document.get(key, [])]
        document[key].extend(additions.values())
    world_ids = {row["worldId"] for row in document["worlds"]}
    resource_ids = {row["resourceId"] for row in document["presentationResources"]}
    if (any(row["worldId"] not in world_ids for row in pattern.get("worldOccurrences", [])) or
        any(row["resourceId"] not in resource_ids for row in pattern.get("presentationOccurrences", []))):
        raise ValueError("Generated Sequence has unresolved World or Presentation definitions")
    matches = [index for index, row in enumerate(document["patterns"]) if row["patternId"] == pattern["patternId"]]
    if len(matches) != 1:
        raise ValueError("Generated Sequence needs one existing stable Pattern identity")
    document["patterns"][matches[0]] = copy.deepcopy(pattern)
    document["revision"] += 1
    text = json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    descriptor, temporary = tempfile.mkstemp(prefix=path.name + ".intro.", suffix=".tmp", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as output:
            output.write(text)
            output.flush()
            os.fsync(output.fileno())
        if path.read_bytes() != baseline:
            raise ValueError("Sequence composition changed during the intro import; existing file preserved")
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def map_binding(material, asset, slot):
    return dict(materialName=material,sourceAssetId=asset,sourceMaterialName=slot)


def book_materials():
    return [map_binding('bg_rad_koukusaton_floor18_mi_hht','MAP_55069666EA53_BG_RAD_KOUKUSATON_FLOOR18_SM_HHT','SLOT_000_bg_rad_koukusaton_floor18_mi_hht'),
            map_binding('bg_rad_koukusaton_floor09c_mi_rsh','MAP_55069666EA53_BG_RAD_KOUKUSATON_FLOOR18_SM_HHT','SLOT_001_bg_rad_koukusaton_floor09c_mi_rsh'),
            map_binding('bg_rad_koukusaton_floor17_mi_hht','MAP_ABCAA963E83F_BG_RAD_KOUKUSATON_FLOOR17_SM_HHT','SLOT_000_bg_rad_koukusaton_floor17_mi_hht')]


def migrate_popup_book(sequence_path):
    resource = object_resource('PopupBook',f'Map/{AREA}/AnimatedProps/DEPLOY_CINE_KOUKU_BOOK/DEPLOY_CINE_KOUKU_BOOK.wmodel',True)
    resource.update(objectId='world.object.kouku.popup.book',displayName='팝업북 / 복구 재질',scale=[2,2,2],
                    defaultMotionInstanceId='world.sequence.instance.original_book',mapMaterialBindings=book_materials())
    current=read(sequence_path)
    instance=next(x for x in current['instances']if x['instanceId']=='world.sequence.instance.original_book')
    template=next(x for x in current['templates']if x['sequenceId']==instance['templateId'])
    instance.update(position=[0,-1.41,737.28],bindings=[dict(slotId='animated.prop',targetKind='OBJECT_RESOURCE',targetId=resource['objectId'])])
    template['tracks']=[dict(slotId='animated.prop',keys=[dict(timeMs=t,positionOffset=[0,0,0],
        rotationQuaternion=[0,-.923879533,0,.382683432],scaleMultiplier=[1,1,1],visible=True)for t in [0,template['durationMs']]])]
    append_rows(sequence_path,'objectResources','objectId',[resource])
    replace_row(sequence_path,'instances','instanceId',instance)
    replace_row(sequence_path,'templates','sequenceId',template)


def card_worlds(rows):
    source=read(EVIDENCE/'card_eruption.result.json')
    resource=object_resource('CardEruption',source['modelAssetId'],True)
    resource.update(displayName='월드 오브젝트_카드분출',mapMaterialBindings=[
        map_binding('bg_rad_koukusaton_card01a_mi','MAP_0898592C705E_BG_RAD_KOUKUSATON_CARD01D_SM','SLOT_000_bg_rad_koukusaton_card01a_mi'),
        map_binding('bg_rad_koukusaton_card01d_mi','MAP_0898592C705E_BG_RAD_KOUKUSATON_CARD01D_SM','SLOT_001_bg_rad_koukusaton_card01d_mi'),
        map_binding('bg_rad_koukusaton_card01b_mi','MAP_2FFEEBF21501_BG_RAD_KOUKUSATON_CARD01K_SM','SLOT_000_bg_rad_koukusaton_card01b_mi')])
    template_id=f'sequence.{PREFIX}.carderuption'
    template=dict(sequenceId=template_id,displayName='카드분출 / 3 emitter',category='World',durationMs=source['durationMs'],
        interpolation='LINEAR',tracks=[dict(slotId='actor',keys=[dict(timeMs=t,positionOffset=[0,0,0],rotationQuaternion=[0,0,0,1],scaleMultiplier=[1,1,1],visible=True)for t in [0,source['durationMs']]])],
        animationTracks=[dict(slotId='actor',clipName='card_eruption',startMs=0,playbackRate=1,loop=False,holdLastFrame=True)])
    standalone=dict(instanceId=f'world.sequence.instance.{PREFIX}.carderuption',templateId=template_id,enabled=True,
        startDelayMs=0,playbackSpeed=1,anchorKind='WORLD',position=[0,0,0],motionEnd='STOP',nextMotionId='',
        bindings=[dict(slotId='actor',targetKind='OBJECT_RESOURCE',targetId=resource['objectId'])])
    resource['defaultMotionInstanceId']=standalone['instanceId']
    templates=[template];instances=[standalone];worlds=[];windows=[]
    for group in [472,473,446]:
        toggle=next(t['p']['toggletrack']for t in active_tracks(rows,group)if t['cls']=='interptracktoggle')
        start=round(next(x['time']for x in toggle if x['toggleaction']=='etta_trigger')*1000)
        end=round(next(x['time']for x in reversed(toggle)if x['toggleaction']=='etta_off')*1000)
        for actor in group_actor(rows,group):
            label=f'cards{actor}';r=copy.deepcopy(resource)
            tr,ins,world=make_world(rows,group,actor,r,label)
            tr['durationMs']=source['durationMs'];tr['tracks'][0]['keys']=[dict(k,timeMs=t)for k,t in zip([tr['tracks'][0]['keys'][0]]*2,[0,source['durationMs']])]
            tr['animationTracks'][0]['clipName']='card_eruption'
            templates.append(tr);instances.append(ins);worlds.append(world);windows.append((start,end))
    return resource,templates,instances,worlds,windows


def attachment_worlds(rows):
    """The two native Saydon component attachments keep their source rest pose."""
    resources=[];templates=[];instances=[];worlds=[];windows=[]
    parent=333;group=467
    attached=rows[rows[parent]['p']['skeletalmeshcomponent']]['p']['attachments']
    times=np.array(sorted(set(round(i*1000/30)for i in range(811))|set(source_times(rows,group))),dtype=int)
    for label,component,model in [('SaydonClub',1474,'Character/KoukuSaton/WP_MN_RPCT_05/WP_MN_RPCT_05.wmodel'),
                                  ('SaydonHead',1475,'Character/KoukuSaton/WP_MN_RPCT_08/wp_mn_rpct_08_1_sk.wmodel')]:
        source=next(x for x in attached if x['component']==component)
        pose_rows=dict(rows)
        pose_rows[component]=dict(p=dict(base=parent,basebonename=source['bonename'],
            relativelocation=source['relativelocation'],relativerotation=source['relativerotation']))
        poses=[world_pose(pose_rows,None,component,int(t)/1000)for t in times]
        positions=np.array([p for p,_ in poses]);quaternions=Rotation.from_matrix(np.array([r for _,r in poses])).as_quat()
        keep=reduced_indices(times,positions,quaternions)
        resource=object_resource(label,model,True,material_source=model)
        resources.append(resource)
        for offset in range(0,len(keep)-1,255):
            part=keep[offset:offset+256];start,end=int(times[part[0]]),int(times[part[-1]])
            tag=f'{label.lower()}.{offset//255+1}'
            sequence=f'sequence.{PREFIX}.{tag}';instance_id=f'world.sequence.instance.{PREFIX}.{tag}'
            keys=[]
            for index in part:
                q=quaternions[index]
                if q[3]<0:q=-q
                keys.append(dict(timeMs=int(times[index])-start,positionOffset=positions[index].tolist(),rotationQuaternion=q.tolist(),
                    scaleMultiplier=(vec(source.get('relativescale'),(1,1,1))[[0,2,1]]*rows[parent]['p'].get('drawscale',1.)).tolist(),visible=True))
            templates.append(dict(sequenceId=sequence,displayName='2관문 진입 / '+label,category='World',durationMs=end-start,
                interpolation='LINEAR',tracks=[dict(slotId='attachment',keys=keys)],animationTracks=[]))
            instances.append(dict(instanceId=instance_id,templateId=sequence,enabled=True,startDelayMs=0,playbackSpeed=1,
                anchorKind='WORLD',position=[0,0,0],motionEnd='STOP',nextMotionId='',
                bindings=[dict(slotId='attachment',targetKind='OBJECT_RESOURCE',targetId=resource['objectId'])]))
            worlds.append(dict(worldId=f'world.{PREFIX}.{tag}',displayName='2관문 진입 / '+label,sequenceInstanceId=instance_id,
                positionOffset=[0,0,0],anchorKind='NONE',anchorPosition=[0,0,0],companionEffectResourceId=''))
            windows.append((start,end))
    return resources,templates,instances,worlds,windows


def build(install):
    rows,imports=extract_scene()
    write(EVIDENCE/'source.json',dict(exports=list(rows.values()),imports=imports))
    defs=[('Saydon',467,333,RESOURCES/'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'),
          ('Kouku',428,332,RESOURCES/'Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel'),
          ('Table',500,1470,EVIDENCE/'Table/table.wmodel'),
          ('Book',499,1463,EVIDENCE/'Book/book.wmodel'),
          ('HandBook',517,1471,EVIDENCE/'Book/book.wmodel')]
    resources=[];templates=[];instances=[];worlds=[]
    for label,group,actor,source in defs:
        model,checked=bake_actor(rows,group,source,label)
        BONE_MODELS[actor]=(checked,group,{})
        material_source=source.relative_to(RESOURCES).as_posix() if source.is_relative_to(RESOURCES) else ''
        resource=object_resource(label,model,True,material_source=material_source)
        if label in ('Book','HandBook'):
            resource['mapMaterialBindings']=book_materials()
        elif label=='Table':
            asset='MAP_6CAA005B80DF_BG_RAD_KOUKUSATON_FLOOR15_SM_HHT_OVR_10E1C8701F8A'
            resource['mapMaterialBindings']=[map_binding('bg_rad_koukusaton_floor15_mi_hht',asset,'SLOT_000_bg_rad_koukusaton_floor15_mi_hht'),
                map_binding('bg_rad_koukusaton_floor15a_mi_hht',asset,'SLOT_001_bg_rad_koukusaton_floor15a_mi_hht')]
        template,instance,world=make_world(rows,group,actor,resource,label)
        resources.append(resource);templates.append(template);instances.append(instance);worlds.append(world)
        print('BAKED',label,model,flush=True)
    # Existing map geometry and admitted material definitions are reused. The
    # native chair/candle AnimControl rows are empty; their Move tracks animate.
    for label,group,actor,asset,slot in [
        *[(f'Chair{i+1}',g,a,'MAP_7396E9403ECB_BG_RAD_KOUKUSATON_CHAIR01_SM_HHT','SLOT_000_bg_rad_koukusaton_chair01_mi_hht')for i,(g,a)in enumerate([(474,292),(485,293),(496,294),(507,295)])],
        *[(f'Candle{i+1}',g,a,'MAP_2D5EAFA31C12_BG_RAD_KOUKUSATON_DECO19_SM_HHT','SLOT_000_bg_rad_koukusaton_deco19_mi_hht')for i,(g,a)in enumerate([(501,149),(502,150)])]]:
        model=f'Map/{AREA}/{asset}/{asset}.wmodel'
        assert (RESOURCES/model).is_file()
        resource=object_resource(label,model,False)
        resource['mapMaterialBindings']=[map_binding(slot,asset,slot)]
        template,instance,world=make_world(rows,group,actor,resource,label)
        resources.append(resource);templates.append(template);instances.append(instance);worlds.append(world)
    world_windows=[(0,DURATION)]*len(worlds)
    attached=attachment_worlds(rows)
    resources+=attached[0];templates+=attached[1];instances+=attached[2];worlds+=attached[3];world_windows+=attached[4]
    backdrops=build_backdrops(rows,imports,lambda actor,t:world_pose(rows,None,actor,t))
    resources+=backdrops['resources'];templates+=backdrops['templates'];instances+=backdrops['instances']
    worlds+=backdrops['worlds'];world_windows+=backdrops['windows']
    write(EVIDENCE/'backdrops.result.json',backdrops['receipt'])
    card,card_templates,card_instances,card_world_rows,card_windows=card_worlds(rows)
    resources.append(card);templates+=card_templates;instances+=card_instances;worlds+=card_world_rows;world_windows+=card_windows
    standalone_worlds=[dict(worldId=f'world.{PREFIX}.carderuption',displayName='월드 오브젝트_카드분출',
        sequenceInstanceId=card['defaultMotionInstanceId'],objectResourceId=card['objectId'],
        positionOffset=[0,0,0],anchorKind='NONE',anchorPosition=[0,0,0],companionEffectResourceId='')]
    shots=make_cameras(rows)
    camera_resources=[dict(resourceId=f"presentation.{PREFIX}.camera.{n+1}",displayName=shot['displayName'],
        defaultAnchorKind="WORLD",kind="CAMERA",assetId=shot['shotId'],resourceKind="",elementId="",durationMs=end-start,
        shape="BOX",colliderKind="GEOMETRY",halfExtents=[1,1,1],radiusM=3,halfAngleDegrees=45)
        for n,(start,end,shot) in enumerate(shots)]
    pattern=next(p for p in read(ROOT/'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json')['patterns']
                 if p['patternId']==PATTERN_ID)
    fade=dict(resourceId=f'presentation.{PREFIX}.fade',displayName='2관문 진입 / 원본 Fade',defaultAnchorKind='MAP',kind='EFFECT',
        assetId='kouku.gate2.intro.fade',resourceKind='GROUP',elementId='',durationMs=DURATION,
        shape='BOX',colliderKind='GEOMETRY',halfExtents=[1,1,1],radiusM=3,halfAngleDegrees=45)
    camera_resources.append(fade)
    lights=dict(fade,resourceId=f'presentation.{PREFIX}.lights',displayName='2관문 진입 / 원본 이동 조명',
                assetId='effect.kouku.gate2.intro.lights',resourceKind='V1_EFFECT')
    camera_resources.append(lights)
    pattern.update(nextStageOrdinal=2,nextWorldOccurrenceOrdinal=len(worlds)+1,
        nextPresentationOccurrenceOrdinal=len(shots)+3,
        stages=[dict(stageId="STAGE_1",actionId=PATTERN_ID+'.stage.1',stageKind="ACTIVE",durationMs=DURATION,animationOccurrences=[])],
        worldOccurrences=[dict(occurrenceId=f"{PATTERN_ID}.world.{i+1}",worldId=w['worldId'],startMs=world_windows[i][0],durationMs=world_windows[i][1]-world_windows[i][0],playbackSpeed=1)
                          for i,w in enumerate(worlds)],
        presentationOccurrences=[presentation_occurrence(camera_resources[i]['resourceId'],i+1,start,end)
                                 for i,(start,end,_) in enumerate(shots)])
    pattern['presentationOccurrences'].append(presentation_occurrence(fade['resourceId'],len(shots)+1,0,DURATION,'MAP'))
    pattern['presentationOccurrences'].append(presentation_occurrence(lights['resourceId'],len(shots)+2,0,DURATION,'MAP'))
    result=dict(objectResources=resources,templates=templates,instances=instances,worlds=worlds,
                standaloneWorlds=standalone_worlds,shots=[s for _,_,s in shots],presentations=camera_resources,pattern=pattern)
    write(EVIDENCE/'composition.rows.json',result)
    if install:
        sequence_path=AREA_DIR/f'{AREA}.worldsequences.json'
        for key,idkey in [('objectResources','objectId'),('templates','sequenceId'),('instances','instanceId')]:
            append_rows(sequence_path,key,idkey,result[key])
        migrate_popup_book(sequence_path)
        append_rows(AREA_DIR/f'{AREA}.camerashots.json','shots','shotId',result['shots'])
        composition=ROOT/'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
        append_rows(composition,'worlds','worldId',worlds+standalone_worlds)
        append_rows(composition,'presentationResources','resourceId',camera_resources)
        install_sequence_pattern(ROOT/'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json',
                                 pattern, worlds+standalone_worlds, camera_resources)
    return result


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--install',action='store_true',help='Install the generated rows into existing authoring documents')
    build(parser.parse_args().install)
