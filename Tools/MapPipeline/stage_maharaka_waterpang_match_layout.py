"""Restore source camera; stage the existing tower/NPC only during intro15.

57011 source XY/yaw is authoritative for match staging, not island 57009.
Installed head/support pivot height is explicitly PROJECT_FRAME_CONTACT_ADAPTER.
No permanent Map placement or gameplay NPC transform is changed.
"""
import argparse
import json
from pathlib import Path
import sqlite3
import struct
import sys
from dataclasses import replace

import retarget_maharaka_waterpang_intro as previous
import build_maharaka_waterpang_sequences as b
sys.path.insert(0, str(b.ROOT/'Tools/WorldPipeline'))
sys.path.insert(0, str(b.ROOT/'Tools/ModelAssetConverter'))
from audit_maharaka_source_npcs import read_deploy
from test_maharaka_npc_population import rows
from verify_dimensionmaster_summon_bind_pose import read_wmodel, sample_animation

OUT = b.ROOT/'out/MaharakaMatchLayout20260928'
SOURCE = Path('C:/LostArkExtract/MaharakaFunctions20260926')
TAG = 'maharaka.waterpang.source.intro15'
STAND_SOURCE = b.AREA+'_SCENE04A:export:612'


def prepare():
    camera_path = b.AUTHORING/(b.AREA+'.camerashots.json')
    sequence_path = b.AUTHORING/(b.AREA+'.worldsequences.json')
    placement_path = b.AUTHORING/(b.AREA+'.mapplacements')
    gameplay_path = b.ROOT/f'Data/Worlds/{b.AREA}/Gameplay.world.json'
    originals = {p:p.read_bytes() for p in (camera_path,sequence_path,placement_path,gameplay_path)}
    camera = b.load_json(camera_path)
    sequence = b.load_json(sequence_path)
    original_camera = b.load_json(previous.OUT/'before'/camera_path.relative_to(b.ROOT))
    prior_camera = b.load_json(previous.OUT/'candidate'/camera_path.relative_to(b.ROOT))
    scene = next(c for c in camera['cutscenes'] if c['cutsceneId'] == 'cutscene.'+TAG)
    old_scene = next(c for c in original_camera['cutscenes'] if c['cutsceneId'] == scene['cutsceneId'])
    b.require(scene['cameraCuts'] == old_scene['cameraCuts'], 'Concurrent cut timetable change')
    ids = {cut['shotId'] for cut in old_scene['cameraCuts']}
    for index, shot in enumerate(camera['shots']):
        if shot['shotId'] not in ids:
            continue
        raw = next(s for s in original_camera['shots'] if s['shotId'] == shot['shotId'])
        edited = next(s for s in prior_camera['shots'] if s['shotId'] == shot['shotId'])
        camera['shots'][index] = previous.merge_reviewed(edited,raw,shot)
        b.require(camera['shots'][index]['cameraTrack'] == raw['cameraTrack'], 'Source camera restoration failed')
    scene['displayName'] = '워터팡 / 도입 15 · 원본 카메라·경기 배치'
    duration = scene['durationMs']
    deploy = SOURCE/'mapdata/Common_Extra/MapData/57011/DeployData.loa'
    database = SOURCE/'db/EFGame_Extra/ClientData/TableData/EFTable_Npc.db'
    db = sqlite3.connect(database.as_uri()+'?mode=ro', uri=True)
    db.row_factory = sqlite3.Row
    npc, = [row for row in read_deploy(deploy,db) if row['npcId'] == 570941]
    db.close()
    b.require(npc['actorId'] == 22 and npc['definition']['Model'] == 'EFDLChar_MN_ISMP_00.MN_ISMP_00', 'Wrong arena actor')
    # Previously inventoried actor offset; reread exact binary floats, not rounded audit JSON.
    binary = deploy.read_bytes()
    offset = 12232
    marker = b'CEFDeployActor_Prop\0'
    b.require(binary[offset-len(marker):offset] == marker, 'Prop record boundary changed')
    b.require(struct.unpack_from('<i',binary,offset+0x64)[0] == 570987, 'Arena support identity changed')
    prop_position = struct.unpack_from('<3f',binary,offset)
    prop_yaw = struct.unpack_from('<i',binary,offset+0x10)[0]*360/65536
    target_npc = b.np.array(npc['position'])
    npc_rotation = b.Rotation.from_euler('y',npc['sourceYawUnits']*360/65536,degrees=True)
    stand, = [r for r in rows(placement_path) if r[1] == STAND_SOURCE]
    baseline_position = b.np.array(list(map(float,stand[5:8])))
    baseline_rotation = b.Rotation.from_quat(list(map(float,stand[8:12])))
    normal_npc = next(p for p in b.load_json(gameplay_path)['placements'] if p['placementId'] == previous.BIG)
    normal_rotation = b.Rotation.from_euler('y',normal_npc['yawDegrees']-90,degrees=True)
    group_rotation = npc_rotation*normal_rotation.inv()
    stand_rotation = group_rotation*baseline_rotation
    model_path = b.ROOT/'Client/Bin/Resources/Character/NPC/Maharaka/MN_ISMP_00/MN_ISMP_00.wmodel'
    model = read_wmodel(model_path)
    idle = next(a for a in model.animations if a.name == 'idle_normal_1')
    smile = model.submeshes[2]
    # Installed source WMOD skinned format has 76-byte vertices.
    face_model = replace(model, vertices=model.vertices[smile[0]//76:smile[0]//76+smile[1]])
    face_local = b.np.array(sample_animation(face_model,idle,0)['bounds']['center'])*.01
    face_offset = npc_rotation.apply(face_local)
    cut = old_scene['cameraCuts'][-1]
    shot = next(s for s in original_camera['shots'] if s['shotId'] == cut['shotId'])
    key = min(shot['cameraTrack']['keyframes'], key=lambda k:abs(k['timeMs']+cut['startMs']-7500))
    eye = b.np.array(key['eye']); forward = b.np.array(key['lookAt'])-eye
    forward /= b.np.linalg.norm(forward)
    right = b.np.cross(b.np.array(key['up']),forward); right /= b.np.linalg.norm(right)
    up = b.np.cross(forward,right)
    # Keep exact source horizontal position/yaw. Match the installed smile mesh's
    # height to the untouched narrow source close-up; do not claim original Z.
    head_height = eye[1] - ((target_npc[0]+face_offset[0]-eye[0])*up[0] +
                          (target_npc[2]+face_offset[2]-eye[2])*up[2])/up[1] - face_offset[1]
    target_npc[1] = head_height
    stand_model = b.ROOT/f'Client/Bin/Resources/Map/{b.AREA}/{stand[4]}/{stand[4]}.wmodel'
    # Move the installed head+support as one rigid group. The neighbouring Prop
    # record is not an exact model join for ITR_02453, so do not silently substitute
    # its transform and break the known head/support contact.
    target_stand = target_npc + group_rotation.apply(baseline_position-b.np.array(normal_npc['position']))
    b.require(abs((target_npc-target_stand)[1]-(b.np.array(normal_npc['position'])-baseline_position)[1])<1e-9,
              'Head/support height relation changed')
    actor_template = next(t for t in sequence['templates'] if t['sequenceId'] == 'sequence.'+TAG+'.mokomoko')
    for actor_key in actor_template['tracks'][0]['keys']:
        actor_key['positionOffset'] = target_npc.tolist()
        actor_key['rotationQuaternion'] = npc_rotation.as_quat().tolist()
    stand_id = TAG+'.match-stand'
    b.require(not any(i['instanceId']=='world.sequence.instance.'+stand_id for i in sequence['instances']), 'Already installed')
    stand_key = dict(timeMs=0,positionOffset=baseline_rotation.inv().apply(target_stand-baseline_position).tolist(),
                     rotationQuaternion=(baseline_rotation.inv()*stand_rotation).as_quat().tolist(),
                     scaleMultiplier=[1,1,1],visible=True)
    template = dict(sequenceId='sequence.'+stand_id,displayName='워터팡 / 경기 중 타워 배치',category='World',
                    durationMs=duration,interpolation='LINEAR',
                    tracks=[dict(slotId='stand',keys=[stand_key,dict(stand_key,timeMs=duration)])],animationTracks=[])
    instance = dict(instanceId='world.sequence.instance.'+stand_id,templateId=template['sequenceId'],enabled=True,
                    startDelayMs=0,playbackSpeed=1,bindings=[dict(slotId='stand',targetKind='MAP_PLACEMENT',targetId=stand[0])])
    sequence['templates'].append(template); sequence['instances'].append(instance)
    scene['worldInstanceIds'].insert(0,instance['instanceId'])
    camera['revision'] += 1; sequence['revision'] += 1
    for path in (deploy,model_path,stand_model): originals[path] = path.read_bytes()
    report = dict(sourceNpc=npc,sourceSupport=dict(definitionId=570987,offset=offset,
        positionCm=prop_position,yawDegrees=prop_yaw,modelJoin='Existing resolved SCENE04A stand reused; 570987 model table missing'),
        heightBasis='PROJECT_FRAME_CONTACT_ADAPTER_NOT_SOURCE_Z',targetNpc=target_npc.tolist(),
        targetStand=target_stand.tolist(),faceLocal=face_local.tolist(),
        ordinaryGameplayAndPlacementsUnchanged=True,sourceCameraRestored=True,visualApproval=False)
    return {camera_path:(originals[camera_path],b.encoded(camera)),sequence_path:(originals[sequence_path],b.encoded(sequence))},originals,report


def main():
    parser=argparse.ArgumentParser(description=__doc__); parser.add_argument('--apply',action='store_true')
    args=parser.parse_args()
    staged,originals,report=prepare()
    OUT.mkdir(parents=True,exist_ok=True)
    for path,(before,after) in staged.items():
        for label,raw in [('before',before),('candidate',after)]:
            target=OUT/label/path.relative_to(b.ROOT); target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(raw)
    (OUT/'report.json').write_bytes(b.encoded(report))
    if args.apply: b.commit_staged_files(staged,expected=originals)
    print(json.dumps(dict(applied=args.apply,targetNpc=report['targetNpc'],targetStand=report['targetStand'],files=len(staged))))


if __name__=='__main__': main()
