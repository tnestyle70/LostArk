# G04 전체 코드 · 원본 카메라와 경기 배치

기존 PLAN의 G04 부록이다. C++/Python은 현재 구현 전문, JSON은 승인 후 저장·게시한 변경 행이다. 새 C++ 및 project/filter 등록 없음. 자동 생성 스냅샷이며 원본 파일을 직접 수정한 뒤 재생성한다.

## C:\Users\USER\source\졸업팀폴\LostArk/Tools/MapPipeline/stage_maharaka_waterpang_match_layout.py

```python
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
```

## Publisher 교체 함수 · Read-WorldSequenceDocument

```powershell
function Read-WorldSequenceDocument {
    param([string]$Path)
    if ([IO.FileInfo]::new($Path).Length -gt 16777216) { throw 'World sequence source exceeds the Client 16 MiB admission limit' }
    $raw = [IO.File]::ReadAllText($Path, [Text.UTF8Encoding]::new($false, $true))
    try { $document = $raw | ConvertFrom-Json }
    catch { throw "World sequence JSON parse failed: $Path" }
    $rootProperties = @('schema','formatVersion','areaId','revision','templates','instances')
    if ($document.formatVersion -eq 3) { $rootProperties += 'objectResources' }
    if ($document.formatVersion -eq 3 -and $null -ne $document.PSObject.Properties['objectFolders']) { $rootProperties += 'objectFolders' }
    Assert-ExactJsonProperties $document $rootProperties 'World sequence root'
    if ($document.schema -isnot [string] -or
        $document.schema -ne 'lostark.world-sequences' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -notin @(2.0, 3.0) -or
        $document.areaId -isnot [string] -or $document.areaId -ne $AreaId -or
        -not (Test-JsonNumber $document.revision) -or
        [double]$document.revision -lt 1 -or [double]$document.revision -gt 4294967295 -or
        [double]$document.revision -ne [math]::Floor([double]$document.revision)) {
        throw "World sequence header is invalid: $Path"
    }
    if ($document.templates -isnot [System.Array] -or
        $document.instances -isnot [System.Array]) {
        throw "World sequence templates and instances must be arrays: $Path"
    }
    # WorldSequenceDocument.cpp 의 상한과 동일하게 검사한다.
    $templates = @($document.templates)
    $instances = @($document.instances)
    if ($templates.Count -gt 512 -or $instances.Count -gt 2048) {
        throw "World sequence document exceeds its limits: $Path"
    }
    $stableId = '^[A-Za-z0-9._-]{1,128}$'
    function Assert-SequenceVector($Value, [string]$Label, [bool]$Positive = $false) {
        if ($Value -isnot [System.Array] -or @($Value).Count -ne 3) { throw "$Label must contain three numbers" }
        foreach ($component in $Value) {
            if (-not (Test-JsonNumber $component) -or [math]::Abs([double]$component) -gt 100000 -or
                ($Positive -and [double]$component -lt 0.000001)) { throw "$Label component is invalid" }
        }
    }
    function Assert-SequenceAssetPath([string]$Value, [bool]$Model) {
        if ([string]::IsNullOrWhiteSpace($Value) -or $Value.Length -gt 1024 -or
            $Value.StartsWith('/') -or $Value.Contains(':') -or $Value.Contains('\') -or
            $Value -match '[\x00-\x1f\x7f]' -or @($Value.Split('/') | Where-Object { $_ -in @('', '.', '..') }).Count -gt 0 -or
            ($Model -and -not $Value.EndsWith('.wmodel', [StringComparison]::OrdinalIgnoreCase))) {
            throw "Invalid Resources-relative world object asset ID: $Value"
        }
    }
    $objectResources = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $hierarchy = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    if ($null -ne $document.PSObject.Properties['objectFolders']) {
        if ($document.formatVersion -ne 3 -or $document.objectFolders -isnot [System.Array] -or $document.objectFolders.Count -gt 2048) {
            throw 'World object folder list is invalid'
        }
        foreach ($folder in $document.objectFolders) {
            $fields = @('folderId','displayName')
            foreach ($optional in @('anchorKind','parentId')) {
                if ($null -ne $folder.PSObject.Properties[$optional]) { $fields += $optional }
            }
            Assert-ExactJsonProperties $folder $fields 'World object folder'
            if ($folder.folderId -isnot [string] -or $folder.folderId -cnotmatch $stableId -or
                $hierarchy.ContainsKey($folder.folderId) -or $folder.displayName -isnot [string] -or
                [Text.UTF8Encoding]::new($false, $true).GetByteCount($folder.displayName) -notin 1..128 -or
                $folder.displayName -match '[\x00-\x1f\x7f]') {
                throw 'Invalid or duplicate World object folder'
            }
            $anchor = 'WORLD'; $parent = ''
            if ($null -ne $folder.PSObject.Properties['anchorKind']) {
                if ($folder.anchorKind -isnot [string] -or $folder.anchorKind -cnotin @('WORLD','PLAYER','BOSS')) {
                    throw 'World object folder anchor must be WORLD, PLAYER or BOSS'
                }
                $anchor = $folder.anchorKind
            }
            if ($null -ne $folder.PSObject.Properties['parentId']) {
                if ($folder.parentId -isnot [string] -or ($folder.parentId -cne '' -and $folder.parentId -cnotmatch $stableId)) {
                    throw 'World object folder parent must be a stable ID or empty'
                }
                $parent = $folder.parentId
            }
            $hierarchy.Add($folder.folderId, [pscustomobject]@{ Anchor = $anchor; Parent = $parent })
        }
    }
    $sequenceMaterialCatalogs = $null
    if ($document.formatVersion -eq 3) {
        if ($document.objectResources -isnot [System.Array] -or @($document.objectResources).Count -gt 2048) {
            throw 'World object resource list is invalid'
        }
        foreach ($resource in $document.objectResources) {
            $fields = @('objectId','displayName','modelAssetId','modelPreScale','animated','scale')
            foreach ($optional in @('diffuseTextureAssetId','sequenceInstanceId','anchorKind','anchorBossArchetypeId','anchorBone','defaultMotionInstanceId','materialProfile','materialSourceModelAssetId','mapMaterialBindings','motionInstanceIds','combatBody','animationSetAssetId','presentationBossArchetypeId','parentId')) {
                if ($null -ne $resource.PSObject.Properties[$optional]) { $fields += $optional }
            }
            Assert-ExactJsonProperties $resource $fields 'World object resource'
            if ($resource.objectId -isnot [string] -or $resource.objectId -cnotmatch $stableId -or
                $objectResources.ContainsKey($resource.objectId) -or $resource.displayName -isnot [string] -or
                [Text.Encoding]::UTF8.GetByteCount($resource.displayName) -notin 1..128 -or
                $resource.displayName -match '[\x00-\x1f\x7f]' -or
                $resource.modelAssetId -isnot [string] -or $resource.animated -isnot [bool] -or
                -not (Test-JsonNumber $resource.modelPreScale) -or
                [double]$resource.modelPreScale -lt 0.000001 -or [double]$resource.modelPreScale -gt 100000) {
                throw "Invalid world object resource: $($resource.objectId)"
            }
            Assert-SequenceVector $resource.scale 'World object scale' $true
            if ($null -ne $resource.PSObject.Properties['anchorKind'] -and $resource.anchorKind -cnotin @('WORLD','PLAYER','BOSS')) {
                throw 'World object resource anchor must be WORLD, PLAYER or BOSS'
            }
            $resourceAnchor = 'WORLD'
            if ($null -ne $resource.PSObject.Properties['anchorKind']) { $resourceAnchor = [string]$resource.anchorKind }
            $parent = ''
            if ($null -ne $resource.PSObject.Properties['parentId']) {
                if ($resource.parentId -isnot [string] -or ($resource.parentId -cne '' -and $resource.parentId -cnotmatch $stableId)) {
                    throw 'World object parent must be a stable ID or empty'
                }
                $parent = $resource.parentId
            }
            if ($hierarchy.ContainsKey($resource.objectId)) { throw 'Duplicate World object hierarchy ID' }
            $hierarchy.Add($resource.objectId, [pscustomobject]@{ Anchor = $resourceAnchor; Parent = $parent })
            $anchorBossArchetypeId = ''
            $anchorBone = ''
            foreach ($field in @('anchorBossArchetypeId','anchorBone')) {
                if ($null -ne $resource.PSObject.Properties[$field] -and $resource.$field -isnot [string]) {
                    throw "World object $field must be text"
                }
            }
            if ($null -ne $resource.PSObject.Properties['anchorBossArchetypeId']) { $anchorBossArchetypeId = $resource.anchorBossArchetypeId }
            if ($null -ne $resource.PSObject.Properties['anchorBone']) { $anchorBone = $resource.anchorBone }
            if ([Text.UTF8Encoding]::new($false, $true).GetByteCount($anchorBone) -gt 128 -or
                $anchorBone -match '[\x00-\x1f\x7f]' -or
                ($resourceAnchor -ceq 'BOSS' -and $anchorBossArchetypeId -cnotmatch $stableId) -or
                ($resourceAnchor -cne 'BOSS' -and ($anchorBossArchetypeId -cne '' -or $anchorBone -cne ''))) {
                throw 'World object boss anchor requires a stable boss ID and bounded optional bone; other anchors cannot carry boss fields'
            }
            $alias = $null -ne $resource.PSObject.Properties['sequenceInstanceId'] -and $resource.sequenceInstanceId -ne ''
            if ($null -ne $resource.PSObject.Properties['combatBody']) {
                $body = $resource.combatBody
                $bodyFields = @('maxHp','localCenterM','halfExtentsM','lifetimePolicy')
                if ($null -ne $body.PSObject.Properties['shape']) {
                    $bodyFields += 'shape'
                    if ($body.shape -isnot [string] -or $body.shape -cnotin @('BOX','ELLIPSOID')) { throw 'World Object combat shape must be BOX or ELLIPSOID' }
                }
                Assert-ExactJsonProperties $body $bodyFields 'World Object combat body'
                if ($alias -or $resourceAnchor -cne 'WORLD' -or
                    $null -ne $resource.PSObject.Properties['motionInstanceIds'] -or
                    -not (Test-JsonNumber $body.maxHp) -or [double]$body.maxHp -lt 1 -or [double]$body.maxHp -gt 1000000000 -or
                    [double]$body.maxHp -ne [math]::Floor([double]$body.maxHp) -or
                    $body.lifetimePolicy -isnot [string] -or $body.lifetimePolicy -cne 'UNTIL_DESTROYED') {
                    throw 'Combat body requires a WORLD model, bounded HP and UNTIL_DESTROYED lifetime'
                }
                Assert-SequenceVector $body.localCenterM 'World Object combat center'
                Assert-SequenceVector $body.halfExtentsM 'World Object combat half extents' $true
                foreach ($axis in $body.halfExtentsM) {
                    if ([double]$axis -lt 0.001 -or [double]$axis -gt 1000) { throw 'Combat half extents must be 0.001..1000 metres' }
                }
            }
            if ($null -ne $resource.PSObject.Properties['motionInstanceIds']) {
                $members = $resource.motionInstanceIds
                if ($members -isnot [System.Array] -or $members.Count -lt 1 -or $members.Count -gt 32 -or
                    $resourceAnchor -cne 'WORLD' -or $alias -or $resource.modelAssetId -cne '' -or $resource.animated -or
                    @($resource.scale | Where-Object { [double]$_ -ne 1.0 }).Count -ne 0) {
                    throw 'Object group requires 1..32 Map motion IDs, no model/alias, and unit scale'
                }
                foreach ($field in @('defaultMotionInstanceId','diffuseTextureAssetId')) {
                    if ($null -ne $resource.PSObject.Properties[$field] -and
                        ($resource.$field -isnot [string] -or $resource.$field -cne '')) { throw "Object group cannot carry $field" }
                }
                if ($null -ne $resource.PSObject.Properties['materialProfile'] -or
                    $null -ne $resource.PSObject.Properties['materialSourceModelAssetId'] -or
                    ($null -ne $resource.PSObject.Properties['mapMaterialBindings'] -and
                     ($resource.mapMaterialBindings -isnot [System.Array] -or $resource.mapMaterialBindings.Count -ne 0))) {
                    throw 'Object group cannot carry material input'
                }
                if ($null -ne $resource.PSObject.Properties['animationSetAssetId']) { throw 'Object group cannot carry an animation set' }
                if ($null -ne $resource.PSObject.Properties['presentationBossArchetypeId']) { throw 'Object group cannot carry a presentation boss' }
                $memberIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
                foreach ($id in $members) {
                    if ($id -isnot [string] -or $id -cnotmatch $stableId -or -not $memberIds.Add($id)) {
                        throw 'Object group needs unique stable motion IDs'
                    }
                }
                $objectResources[$resource.objectId] = $resource
                continue
            }
            if ($alias) {
                if ($resource.sequenceInstanceId -isnot [string] -or $resource.sequenceInstanceId -cnotmatch $stableId -or
                    ($null -ne $resource.PSObject.Properties['anchorKind'] -and $resource.anchorKind -cne 'WORLD') -or
                    $resource.modelAssetId -ne '' -or $resource.animated -or
                    ($null -ne $resource.PSObject.Properties['diffuseTextureAssetId'] -and $resource.diffuseTextureAssetId -ne '')) {
                    throw 'World object alias must refer only to an existing sequence instance'
                }
            } else {
                Assert-SequenceAssetPath $resource.modelAssetId $true
                if ($null -ne $resource.PSObject.Properties['diffuseTextureAssetId']) {
                    if ($resource.diffuseTextureAssetId -isnot [string]) { throw 'World object diffuse path must be a string' }
                    if ($resource.diffuseTextureAssetId -ne '') { Assert-SequenceAssetPath $resource.diffuseTextureAssetId $false }
                }
            }
            if ($null -ne $resource.PSObject.Properties['animationSetAssetId']) {
                # A separate AnimSet WModel only makes sense for a skinned body that plays clips.
                if ($alias -or -not $resource.animated -or $resource.animationSetAssetId -isnot [string] -or $resource.animationSetAssetId -eq '') {
                    throw "Invalid world object animation set: $($resource.objectId)"
                }
                Assert-SequenceAssetPath $resource.animationSetAssetId $true
            }
            if ($null -ne $resource.PSObject.Properties['presentationBossArchetypeId']) {
                # The product boss assembly borrows this skinned body's bone palette.
                if ($alias -or -not $resource.animated -or $resource.presentationBossArchetypeId -isnot [string] -or
                    $resource.presentationBossArchetypeId -cnotmatch '^[A-Za-z0-9_.-]{1,128}$') {
                    throw "Invalid world object presentation boss: $($resource.objectId)"
                }
            }
            if ($null -ne $resource.PSObject.Properties['materialSourceModelAssetId']) {
                if ($alias -or $resource.materialSourceModelAssetId -isnot [string] -or $resource.materialSourceModelAssetId -eq '') { throw 'Invalid world object material source model' }
                Assert-SequenceAssetPath $resource.materialSourceModelAssetId $true
                $sourceNames = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $resource.materialSourceModelAssetId)).Names
                $targetNames = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $resource.modelAssetId)).Names
                if ($null -eq $sequenceMaterialCatalogs) {
                    $sequenceMaterialCatalogs = @{}
                    foreach ($catalogName in @('CharacterCatalog','BossCatalog')) {
                        $catalogPath = Join-Path $ProjectRoot "Data\Actors\$catalogName.json"
                        $sequenceMaterialCatalogs[$catalogName] = [IO.File]::ReadAllText($catalogPath, [Text.UTF8Encoding]::new($false, $true)) | ConvertFrom-Json
                    }
                }
                $sourceModel = $resource.materialSourceModelAssetId
                $owners = @($sequenceMaterialCatalogs.CharacterCatalog.characters | Where-Object {
                    $_.bodyModel -ceq $sourceModel -or @($_.equipmentModels) -ccontains $sourceModel -or @($_.weaponModels) -ccontains $sourceModel
                })
                if ($owners.Count -gt 1 -or ($owners.Count -eq 1 -and $owners[0].runtimeStatus -cne 'supported')) { throw "World Object material source ownership is invalid: $sourceModel" }
                $sourceRows = if ($owners.Count -eq 1) { @($owners[0].modelMaterialOverrides | Where-Object { $_.modelAssetId -ceq $sourceModel }) }
                    else { @($sequenceMaterialCatalogs.BossCatalog.modelMaterialOverrides | Where-Object { $_.modelAssetId -ceq $sourceModel }) }
                if (@($sourceRows).Count -eq 0) { throw "World Object original model has no catalog material overrides: $sourceModel" }
                $boundSourceNames = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
                foreach ($material in $sourceRows) {
                    $name = $material.materialName
                    if (-not $boundSourceNames.Add($name) -or -not $sourceNames.ContainsKey($name) -or -not $targetNames.ContainsKey($name)) { throw "Derived World Object lost original material slot: $name" }
                    foreach ($texture in @($material.textures)) {
                        Assert-SequenceAssetPath $texture.assetId $false
                        if (-not [IO.File]::Exists((Join-Path $runtimeResourceRoot $texture.assetId))) { throw "World Object original material texture is absent: $($texture.assetId)" }
                    }
                }
            }
            if ($null -ne $resource.PSObject.Properties['mapMaterialBindings']) {
                if ($alias -or $resource.mapMaterialBindings -isnot [array] -or $resource.mapMaterialBindings.Count -gt 64) { throw 'Invalid world object map material bindings' }
                $boundMaterials = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
                if ($null -ne $resource.PSObject.Properties['materialProfile']) { [void]$boundMaterials.Add($resource.materialProfile.materialName) }
                $targetNames = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $resource.modelAssetId)).Names
                if ($null -eq $cinematicMapMaterials) {
                    try { $cinematicMapMaterials = ([IO.File]::ReadAllText($authoringMaterialPath, [Text.UTF8Encoding]::new($false, $true)) | ConvertFrom-Json).materials }
                    catch { throw "World Object map material source JSON parse failed: $authoringMaterialPath" }
                }
                foreach ($binding in $resource.mapMaterialBindings) {
                    $bindingFields = @('materialName','sourceAssetId','sourceMaterialName')
                    if ($null -ne $binding.PSObject.Properties['diffuseTextureAssetId']) { $bindingFields += 'diffuseTextureAssetId'; Assert-SequenceAssetPath $binding.diffuseTextureAssetId $false }
                    if ($null -ne $binding.PSObject.Properties['unlit']) {
                        $bindingFields += 'unlit'
                        if ($binding.unlit -isnot [bool]) { throw 'World Object map material unlit flag must be boolean' }
                    }
                    Assert-ExactJsonProperties $binding $bindingFields 'World object map material binding'
                    $sourceRows = @($cinematicMapMaterials | Where-Object { $_.assetId -ceq $binding.sourceAssetId -and $_.materialName -ceq $binding.sourceMaterialName })
                    if (-not $targetNames.ContainsKey($binding.materialName) -or $sourceRows.Count -ne 1 -or $sourceRows[0].family -cne 'bg-source-opaque-masked') { throw 'World Object map material slot/source is absent or unsupported' }
                    if ($null -ne $binding.PSObject.Properties['diffuseTextureAssetId'] -and -not [IO.File]::Exists((Join-Path $runtimeResourceRoot $binding.diffuseTextureAssetId))) { throw 'World Object map surface texture is absent' }
                    if ($binding.sourceAssetId -isnot [string] -or $binding.sourceAssetId -cnotmatch $stableId -or -not $boundMaterials.Add($binding.materialName)) { throw 'Invalid or duplicate map material binding' }
                    foreach ($name in @($binding.materialName,$binding.sourceMaterialName)) {
                        if ($name -isnot [string] -or [Text.Encoding]::UTF8.GetByteCount($name) -notin 1..63 -or $name -match '[\x00-\x1f\x7f]') { throw 'Invalid map material binding name' }
                    }
                }
            }
            if ($null -ne $resource.PSObject.Properties['materialProfile']) {
                if ($alias) { throw 'World object sequence alias cannot own a material profile' }
                $material = $resource.materialProfile
                Assert-ExactJsonProperties $material @('materialName','sourceMaterial','family','parameters','textures') 'World object material'
                if ($material.materialName -isnot [string] -or [Text.Encoding]::UTF8.GetByteCount($material.materialName) -notin 1..63 -or
                    $material.sourceMaterial -isnot [string] -or [Text.Encoding]::UTF8.GetByteCount($material.sourceMaterial) -notin 1..512 -or
                    $material.materialName -match '[\x00-\x1f\x7f]' -or $material.sourceMaterial -match '[\x00-\x1f\x7f]' -or
                    $material.family -isnot [string] -or $material.family -cnotmatch '^source\.character\.[a-z0-9.-]+\.v1$') {
                    throw 'World object material identity or native family is invalid'
                }
                $nativeContract = Get-SequenceNativeMaterialContract $material.family
                $parameterNames = $nativeContract.ParameterNames
                Assert-ExactJsonProperties $material.parameters $parameterNames 'World object native material parameters'
                foreach ($name in $parameterNames) {
                    $value = $material.parameters.$name
                    if ($value -is [array]) {
                        if ($value.Count -ne 4) { throw "Invalid native material vector: $name" }
                        $values = $value
                    } else { $values = @($value) }
                    foreach ($component in $values) {
                        if (-not (Test-JsonNumber $component) -or [Math]::Abs([double]$component) -gt 1000000) {
                            throw "Invalid native material parameter: $name"
                        }
                    }
                }
                if ($material.textures -isnot [array] -or $material.textures.Count -gt 16) {
                    throw 'World object native material texture expression count is invalid'
                }
                $textureMask = 0
                foreach ($texture in $material.textures) {
                    Assert-ExactJsonProperties $texture @('expressionIndex','assetId','colorSpace') 'World object native material texture'
                    if (-not (Test-JsonNumber $texture.expressionIndex) -or [double]$texture.expressionIndex % 1 -ne 0 -or $texture.expressionIndex -notin 0..15 -or
                        $texture.assetId -isnot [string] -or $texture.colorSpace -cnotin @('srgb','linear')) {
                        throw 'Invalid world object native material texture'
                    }
                    $bit = 1 -shl [int]$texture.expressionIndex
                    if (($textureMask -band $bit) -ne 0) { throw 'Duplicate world object native texture expression' }
                    $textureMask = $textureMask -bor $bit
                    Assert-SequenceAssetPath $texture.assetId $false
                    if (-not [IO.File]::Exists((Join-Path $runtimeResourceRoot $texture.assetId))) {
                        throw "World object material texture is missing: $($texture.assetId)"
                    }
                }
                if ($textureMask -ne $nativeContract.TextureMask) { throw 'World object native texture coverage is incomplete' }
            }
            $objectResources[$resource.objectId] = $resource
        }
    }
    foreach ($entry in $hierarchy.GetEnumerator()) {
        $visited = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        [void]$visited.Add($entry.Key)
        $current = $entry.Value; $depth = 0
        while ($current.Parent -cne '') {
            if (-not $hierarchy.ContainsKey($current.Parent) -or $hierarchy[$current.Parent].Anchor -cne $entry.Value.Anchor) {
                throw 'World object parent must exist in the same anchor category'
            }
            $depth++
            if (-not $visited.Add($current.Parent) -or $depth -gt 64) {
                throw 'World object hierarchy contains a cycle or exceeds 64 parents'
            }
            $current = $hierarchy[$current.Parent]
        }
    }
    $trackCounts = @{}
    $templateRows = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $templateIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($template in $templates) {
        $templateProperties = @('sequenceId','displayName','category','durationMs','interpolation','tracks','animationTracks')
        if ($document.formatVersion -eq 3 -and $null -ne $template.PSObject.Properties['objectMotion']) { $templateProperties += 'objectMotion' }
        if ($document.formatVersion -eq 3 -and $null -ne $template.PSObject.Properties['effectTracks']) { $templateProperties += 'effectTracks' }
        if ($document.formatVersion -eq 3 -and $null -ne $template.PSObject.Properties['colliderTracks']) { $templateProperties += 'colliderTracks' }
        foreach ($lane in @('soundTracks','subtitleTracks','materialTracks')) {
            if ($document.formatVersion -eq 3 -and $null -ne $template.PSObject.Properties[$lane]) { $templateProperties += $lane }
        }
        Assert-ExactJsonProperties $template $templateProperties 'World sequence template'
        if ($template.sequenceId -isnot [string] -or
            $template.sequenceId -notmatch $stableId -or
            -not $templateIds.Add([string]$template.sequenceId) -or
            $template.displayName -isnot [string] -or
            $template.displayName.Length -lt 1 -or
            $template.displayName.Length -gt 128 -or
            $template.category -isnot [string] -or
            $template.category.Length -lt 1 -or $template.category.Length -gt 64 -or
            -not (Test-JsonNumber $template.durationMs) -or
            [double]$template.durationMs -le 0 -or
            [double]$template.durationMs -gt 600000 -or
            $template.interpolation -isnot [string] -or
            $template.interpolation -notin @('LINEAR','SMOOTH_STEP') -or
            $template.tracks -isnot [System.Array] -or
            $template.animationTracks -isnot [System.Array]) {
            throw "World sequence template is invalid: $($template.sequenceId)"
        }
        if ([double]$template.durationMs -ne [math]::Floor([double]$template.durationMs)) { throw 'World sequence duration must be integer milliseconds' }
        if ($null -ne $template.PSObject.Properties['objectMotion']) {
            $motion = $template.objectMotion
            $motionProperties = @('velocity','acceleration','angularVelocityDegrees','revolutionDegreesPerSecond','revolutionOffset','count','intervalMs','spreadDegrees','seed')
            if ($null -ne $motion.PSObject.Properties['spawnHalfExtents']) {
                $motionProperties += 'spawnHalfExtents'
                Assert-SequenceVector $motion.spawnHalfExtents 'World object spawn half extents'
                if (@($motion.spawnHalfExtents | Where-Object { [double]$_ -lt 0 }).Count -gt 0) {
                    throw 'World object spawn half extents must be nonnegative'
                }
            }
            if ($null -ne $motion.PSObject.Properties['emissions']) { $motionProperties += 'emissions' }
            Assert-ExactJsonProperties $motion $motionProperties 'World object motion'
            foreach ($field in @('velocity','acceleration','angularVelocityDegrees','revolutionDegreesPerSecond','revolutionOffset')) {
                Assert-SequenceVector $motion.$field "World object motion $field"
            }
            foreach ($field in @('count','intervalMs','seed')) {
                if (-not (Test-JsonNumber $motion.$field) -or [double]$motion.$field -lt 0 -or
                    [double]$motion.$field -gt 4294967295 -or [double]$motion.$field -ne [math]::Floor([double]$motion.$field)) { throw "Invalid object motion $field" }
            }
            $lastEmissionMs = ([double]$motion.count - 1) * [double]$motion.intervalMs
            if ($null -ne $motion.PSObject.Properties['emissions']) {
                # Authored rows own the count; interval and spread are the seeded emitter's and must stay zero.
                $emissions = @($motion.emissions)
                if ($motion.emissions -isnot [System.Array] -or $emissions.Count -lt 1 -or $emissions.Count -gt 128 -or
                    [double]$motion.count -ne $emissions.Count -or [double]$motion.intervalMs -ne 0 -or [double]$motion.spreadDegrees -ne 0) {
                    throw "World object emissions must be 1..128 rows with count equal to the row count and zero interval/spread: $($template.sequenceId)"
                }
                $lastEmissionMs = 0
                foreach ($emission in $emissions) {
                    Assert-ExactJsonProperties $emission @('positionOffset','yawDegrees','startDelayMs') 'World object emission'
                    Assert-SequenceVector $emission.positionOffset 'World object emission positionOffset'
                    if (-not (Test-JsonNumber $emission.yawDegrees) -or [double]$emission.yawDegrees -lt -36000 -or [double]$emission.yawDegrees -gt 36000 -or
                        -not (Test-JsonNumber $emission.startDelayMs) -or [double]$emission.startDelayMs -lt 0 -or [double]$emission.startDelayMs -gt 600000 -or
                        [double]$emission.startDelayMs -ne [math]::Floor([double]$emission.startDelayMs)) {
                        throw "Invalid World object emission row: $($template.sequenceId)"
                    }
                    if ([double]$emission.startDelayMs -gt $lastEmissionMs) { $lastEmissionMs = [double]$emission.startDelayMs }
                }
            }
            if ($motion.count -lt 1 -or $motion.count -gt 128 -or $motion.intervalMs -gt 600000 -or
                (($null -eq $template.PSObject.Properties['effectTracks'] -or @($template.effectTracks).Count -eq 0) -and
                    $lastEmissionMs -ge [double]$template.durationMs) -or
                -not (Test-JsonNumber $motion.spreadDegrees) -or $motion.spreadDegrees -lt 0 -or $motion.spreadDegrees -gt $(if ($null -ne $template.PSObject.Properties['effectTracks'] -and @($template.effectTracks).Count -gt 0) { 360 } else { 180 })) {
                throw 'World object spawn count, interval or spread exceeds its lifetime'
            }
        }
        $templateRows[[string]$template.sequenceId] = $template
        $tracks = @($template.tracks)
        $animationTracks = @($template.animationTracks)
        $total = $tracks.Count + $animationTracks.Count
        if ($total -lt 1 -or $total -gt 64) {
            throw "World sequence template track count is invalid: $($template.sequenceId)"
        }
        $slotIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($track in $tracks) {
            Assert-ExactJsonProperties $track @('slotId','keys') 'World sequence track'
            if ($track.slotId -isnot [string] -or $track.slotId -notmatch $stableId -or
                -not $slotIds.Add([string]$track.slotId) -or
                $track.keys -isnot [System.Array]) {
                throw "World sequence track is invalid: $($template.sequenceId)"
            }
            $keys = @($track.keys)
            if ($keys.Count -lt 2 -or $keys.Count -gt 4096) {
                throw "World sequence key count is invalid: $($template.sequenceId)/$($track.slotId)"
            }
            $previous = -1
            foreach ($key in $keys) {
                Assert-ExactJsonProperties $key `
                    @('timeMs','positionOffset','rotationQuaternion',
                      'scaleMultiplier','visible') 'World sequence key'
                if (-not (Test-JsonNumber $key.timeMs) -or
                    [double]$key.timeMs -le $previous -or
                    [double]$key.timeMs -gt [double]$template.durationMs -or
                    $key.visible -isnot [bool] -or
                    $key.positionOffset -isnot [System.Array] -or
                    @($key.positionOffset).Count -ne 3 -or
                    $key.rotationQuaternion -isnot [System.Array] -or
                    @($key.rotationQuaternion).Count -ne 4 -or
                    $key.scaleMultiplier -isnot [System.Array] -or
                    @($key.scaleMultiplier).Count -ne 3) {
                    throw "World sequence key is invalid: $($template.sequenceId)/$($track.slotId)"
                }
                foreach ($component in (@($key.positionOffset) +
                        @($key.rotationQuaternion) + @($key.scaleMultiplier))) {
                    if (-not (Test-JsonNumber $component)) {
                        throw "World sequence key component is not finite: $($template.sequenceId)/$($track.slotId)"
                    }
                }
                Assert-SequenceVector $key.positionOffset 'World sequence key position'
                Assert-SequenceVector $key.scaleMultiplier 'World sequence key scale'
                for ($axis = 0; $axis -lt 3; $axis++) {
                    $scale = [double]$key.scaleMultiplier[$axis]
                    if ([math]::Abs($scale) -lt 0.000001 -or
                        [math]::Sign($scale) -ne [math]::Sign([double]$keys[0].scaleMultiplier[$axis])) {
                        throw 'World sequence scale is singular or changes axis sign'
                    }
                }
                $quaternionLength = 0.0
                foreach ($component in $key.rotationQuaternion) { $quaternionLength += [double]$component * [double]$component }
                if ([math]::Abs([math]::Sqrt($quaternionLength) - 1.0) -gt 0.001 -or
                    [double]$key.timeMs -ne [math]::Floor([double]$key.timeMs)) { throw 'World sequence key time or quaternion is invalid' }
                $previous = [double]$key.timeMs
            }
            if ([double]$keys[0].timeMs -ne 0 -or
                [double]$keys[$keys.Count - 1].timeMs -ne [double]$template.durationMs) {
                throw "World sequence track must span the whole duration: $($template.sequenceId)/$($track.slotId)"
            }
        }
        # An animation slot may carry an ordered clip chain, checked against
        # that slot's previous start instead of a plain unique set. A Deploy
        # slot may also carry a transform track so one binding can walk an
        # animated prop while its clips play.
        $animationSlotStarts = @{}
        foreach ($track in $animationTracks) {
            $trackProperties = @('slotId','clipName','playbackRate','loop','holdLastFrame')
            if ($null -ne $track.PSObject.Properties['displayName']) {
                $trackProperties += 'displayName'
                if ($track.displayName -isnot [string] -or
                    ([Text.UTF8Encoding]::new($false, $true)).GetByteCount($track.displayName) -gt 128 -or
                    $track.displayName -match '[\x00-\x1f\x7f]') {
                    throw "World sequence animation track displayName is invalid: $($template.sequenceId)"
                }
            }
            if ($null -ne $track.PSObject.Properties['startMs']) {
                $trackProperties += 'startMs'
            }
            if ($null -ne $track.PSObject.Properties['sourceStartMs']) {
                $trackProperties += 'sourceStartMs'
                if (-not (Test-JsonNumber $track.sourceStartMs) -or
                    [double]$track.sourceStartMs -lt 0 -or [double]$track.sourceStartMs -gt 600000 -or
                    [double]$track.sourceStartMs -ne [math]::Floor([double]$track.sourceStartMs)) {
                    throw "World sequence animation source start is invalid: $($template.sequenceId)"
                }
            }
            if ($null -ne $track.PSObject.Properties['sourceEndMs']) {
                $trackProperties += 'sourceEndMs'
                $sourceStartMs = 0
                if ($null -ne $track.PSObject.Properties['sourceStartMs']) { $sourceStartMs = [double]$track.sourceStartMs }
                if (-not (Test-JsonNumber $track.sourceEndMs) -or
                    [double]$track.sourceEndMs -lt 0 -or [double]$track.sourceEndMs -gt 600000 -or
                    [double]$track.sourceEndMs -ne [math]::Floor([double]$track.sourceEndMs) -or
                    ([double]$track.sourceEndMs -ne 0 -and [double]$track.sourceEndMs -le $sourceStartMs)) {
                    throw "World sequence animation source end is invalid: $($template.sequenceId)"
                }
            }
            Assert-ExactJsonProperties $track $trackProperties `
                'World sequence animation track'
            $startMs = 0
            if ($null -ne $track.PSObject.Properties['startMs']) {
                if (-not (Test-JsonNumber $track.startMs) -or
                    [double]$track.startMs -lt 0 -or
                    [double]$track.startMs -ne [math]::Floor([double]$track.startMs) -or
                    [double]$track.startMs -ge [double]$template.durationMs) {
                    throw "World sequence animation track start is invalid: $($template.sequenceId)"
                }
                $startMs = [int][double]$track.startMs
            }
            if ($track.slotId -isnot [string] -or $track.slotId -notmatch $stableId -or
                $track.clipName -isnot [string] -or
                $track.clipName.Length -lt 1 -or $track.clipName.Length -gt 128 -or
                -not (Test-JsonNumber $track.playbackRate) -or
                [double]$track.playbackRate -lt 0.05 -or
                [double]$track.playbackRate -gt 8.0 -or
                $track.loop -isnot [bool] -or
                $track.holdLastFrame -isnot [bool]) {
                throw "World sequence animation track is invalid: $($template.sequenceId)"
            }
            $slot = [string]$track.slotId
            if ($animationSlotStarts.ContainsKey($slot)) {
                if ($startMs -le $animationSlotStarts[$slot]) {
                    throw "World sequence animation chain must advance: $($template.sequenceId)/$slot"
                }
            } else {
                # A slot may also carry a transform track, so only a second
                # animation chain on the same slot is a conflict.
                [void]$slotIds.Add($slot)
            }
            $animationSlotStarts[$slot] = $startMs
        }
        if ($null -ne $template.PSObject.Properties['effectTracks']) {
            if ($template.effectTracks -isnot [System.Array] -or $total + @($template.effectTracks).Count -gt 64) {
                throw 'World Object effectTracks must be a bounded array'
            }
            $effectIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            $emissionMs = 0
            if ($null -ne $template.PSObject.Properties['objectMotion']) {
                $emissionMs = ($template.objectMotion.count - 1) * $template.objectMotion.intervalMs
                if ($null -ne $template.objectMotion.PSObject.Properties['emissions']) {
                    $emissionMs = 0
                    foreach ($emission in @($template.objectMotion.emissions)) {
                        if ([double]$emission.startDelayMs -gt $emissionMs) { $emissionMs = [double]$emission.startDelayMs }
                    }
                }
            }
            foreach ($effect in $template.effectTracks) {
                $effectProperties = @('effectTrackId','slotId','resourceKind','resourceId','timing','startMs','durationMs','positionOffset','rotationDegrees','scale')
                foreach ($optional in @('followObject','inheritObjectRotation','bone','fitEffectToDuration','loopEffectToDuration')) {
                    if ($null -ne $effect.PSObject.Properties[$optional]) { $effectProperties += $optional }
                }
                Assert-ExactJsonProperties $effect $effectProperties 'World Object effect track'
                if ($null -ne $effect.PSObject.Properties['fitEffectToDuration'] -and
                    ($effect.fitEffectToDuration -isnot [bool] -or
                    ($effect.fitEffectToDuration -and $effect.resourceKind -cne 'V1_EFFECT'))) {
                    throw 'World Object Effect fit must be boolean and true requires V1_EFFECT'
                }
                if ($null -ne $effect.PSObject.Properties['loopEffectToDuration'] -and
                    ($effect.loopEffectToDuration -isnot [bool] -or
                    ($effect.loopEffectToDuration -and $effect.resourceKind -cne 'V1_EFFECT'))) {
                    throw 'World Object Effect loop must be boolean and true requires V1_EFFECT'
                }
                if ($null -ne $effect.PSObject.Properties['fitEffectToDuration'] -and $effect.fitEffectToDuration -and
                    $null -ne $effect.PSObject.Properties['loopEffectToDuration'] -and $effect.loopEffectToDuration) {
                    throw 'World Object Effect fit and loop are mutually exclusive'
                }
                if (($null -ne $effect.PSObject.Properties['followObject'] -and $effect.followObject -isnot [bool]) -or
                    ($null -ne $effect.PSObject.Properties['inheritObjectRotation'] -and $effect.inheritObjectRotation -isnot [bool]) -or
                    ($null -ne $effect.PSObject.Properties['bone'] -and ($effect.bone -isnot [string] -or
                    [Text.Encoding]::UTF8.GetByteCount([string]$effect.bone) -gt 256 -or $effect.bone -match '[\x00-\x1f\x7f]'))) {
                    throw 'Invalid World Object effect followObject or bone'
                }
                if ($effect.effectTrackId -isnot [string] -or $effect.effectTrackId -notmatch $stableId -or
                    -not $effectIds.Add([string]$effect.effectTrackId) -or $effect.slotId -isnot [string] -or
                    -not $slotIds.Contains([string]$effect.slotId) -or $effect.resourceKind -cnotin @('LEAF','GROUP','V1_EFFECT') -or
                    $effect.resourceId -isnot [string] -or $effect.resourceId -notmatch $stableId -or
                    $effect.timing -cnotin @('TIME','MOTION_END')) {
                    throw 'Invalid World Object effect identity, resource kind or slot'
                }
                foreach ($field in @('startMs','durationMs')) {
                    if (-not (Test-JsonNumber $effect.$field) -or $effect.$field -lt 0 -or $effect.$field -gt 600000 -or
                        $effect.$field -ne [math]::Floor([double]$effect.$field)) { throw "Invalid World Object effect $field" }
                }
                $effectStart = if ($effect.timing -ceq 'MOTION_END') { $template.durationMs } else { $effect.startMs }
                if ($effect.startMs -gt $template.durationMs -or $effect.durationMs -lt 1 -or
                    ($effect.timing -ceq 'MOTION_END' -and $effect.startMs -ne 0) -or
                    [math]::Max($template.durationMs, $effectStart + $effect.durationMs) + $emissionMs -gt 600000) {
                    throw 'World Object effect exceeds its motion or presentation span'
                }
                Assert-SequenceVector $effect.positionOffset 'World Object effect position'
                Assert-SequenceVector $effect.rotationDegrees 'World Object effect rotation'
                Assert-SequenceVector $effect.scale 'World Object effect scale' $true
            }
        }
        if ($null -ne $template.PSObject.Properties['colliderTracks']) {
            if ($template.colliderTracks -isnot [System.Array] -or
                @($template.tracks).Count + @($template.animationTracks | Where-Object { $null -ne $_ }).Count + @($template.effectTracks | Where-Object { $null -ne $_ }).Count + @($template.colliderTracks).Count -gt 64) {
                throw 'World Object colliderTracks exceed the combined 64-track limit'
            }
            if (@($template.colliderTracks).Count -gt 0 -and ([double]$template.objectMotion.spreadDegrees -ne 0 -or @($template.objectMotion.spawnHalfExtents | Where-Object { $null -ne $_ -and [double]$_ -ne 0 }).Count)) { throw 'World Object collider publication requires zero random spread and spawn extents' }
            $colliderIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            foreach ($row in $template.colliderTracks) {
                $fields = @('colliderTrackId','slotId','startMs','durationMs','positionOffset','halfExtents','yawDegrees','behavior','damagePercent','gripLocalOffset')
                if ($null -ne $row.PSObject.Properties['attachmentBone']) { $fields += 'attachmentBone' }
                if ($null -ne $row.PSObject.Properties['shape']) { $fields += 'shape' }
                Assert-ExactJsonProperties $row $fields 'World Object collider track'
                $colliderShape = 'BOX'
                if ($null -ne $row.PSObject.Properties['shape']) {
                    if ($row.shape -isnot [string] -or $row.shape -cnotin @('BOX','CYLINDER')) { throw 'Invalid World Object collider shape' }
                    $colliderShape = $row.shape
                }
                if ($row.colliderTrackId -isnot [string] -or $row.colliderTrackId -notmatch $stableId -or -not $colliderIds.Add($row.colliderTrackId) -or
                    $row.slotId -isnot [string] -or @($template.tracks | Where-Object { $_.slotId -ceq $row.slotId }).Count -ne 1 -or
                    $row.behavior -cnotin @('DAMAGE','INSTANT_DEATH','HOOK_CAPTURE')) { throw 'Invalid collider identity, transform slot or behavior' }
                foreach ($field in @('startMs','durationMs','damagePercent')) {
                    if (-not (Test-JsonNumber $row.$field) -or [double]$row.$field -ne [math]::Floor([double]$row.$field)) { throw "Collider $field must be an integer" }
                }
                if ($row.startMs -lt 0 -or $row.durationMs -lt 1 -or $row.startMs + $row.durationMs -gt $template.durationMs -or
                    $row.damagePercent -lt 0 -or $row.damagePercent -gt 100 -or
                    ($row.behavior -ceq 'DAMAGE' -and $row.damagePercent -lt 1) -or
                    ($row.behavior -cne 'DAMAGE' -and $row.damagePercent -ne 0) -or
                    -not (Test-JsonNumber $row.yawDegrees) -or [math]::Abs([double]$row.yawDegrees) -gt 36000) { throw 'Invalid collider clock, damage or yaw' }
                foreach ($field in @('positionOffset','halfExtents','gripLocalOffset')) {
                    Assert-SequenceVector $row.$field "Collider $field"
                    $maximum = if ($field -ceq 'halfExtents') { 1000 } else { 100000 }
                    if (@($row.$field | Where-Object { [math]::Abs([double]$_) -gt $maximum -or ($field -ceq 'halfExtents' -and [double]$_ -le 0.001) }).Count) { throw 'Invalid collider vector range' }
                }
                if ($colliderShape -ceq 'CYLINDER' -and ([math]::Abs([double]$row.halfExtents[0] - [double]$row.halfExtents[2]) -gt 0.0001 -or $row.behavior -ceq 'HOOK_CAPTURE')) {
                    throw 'Cylinder Collider requires equal X/Z radii and cannot capture a hook'
                }
                if ($row.behavior -cne 'HOOK_CAPTURE' -and (@($row.gripLocalOffset | Where-Object { [double]$_ -ne 0 }).Count -or -not [string]::IsNullOrEmpty([string]$row.attachmentBone))) { throw 'Only hook capture carries a grip or bone' }
                if ($null -ne $row.PSObject.Properties['attachmentBone'] -and ($row.attachmentBone -isnot [string] -or
                    [Text.Encoding]::UTF8.GetByteCount([string]$row.attachmentBone) -gt 256 -or $row.attachmentBone -match '[\x00-\x1f\x7f]')) { throw 'Invalid collider attachmentBone' }
            }
        }
        $totalTracks = 0
        foreach ($lane in @('tracks','animationTracks','effectTracks','colliderTracks','soundTracks','subtitleTracks','materialTracks')) {
            if ($null -ne $template.PSObject.Properties[$lane]) {
                if ($template.$lane -isnot [System.Array]) { throw "World $lane must be an array" }
                $totalTracks += @($template.$lane).Count
                if ($lane -in @('soundTracks','subtitleTracks','materialTracks')) {
                    for ($rowIndex = 0; $rowIndex -lt $template.$lane.Count; ++$rowIndex) {
                        if ($null -eq $template.$lane[$rowIndex]) { throw "World $lane cannot contain null rows" }
                    }
                }
            }
        }
        if ($totalTracks -gt 64) { throw 'World sequence exceeds the combined 64-track limit' }
        $soundIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($row in @($template.soundTracks | Where-Object { $null -ne $_ })) {
            $soundFields = @('soundTrackId','assetId','startMs','durationMs','volume')
            if ($row.PSObject.Properties['loopToDuration']) {
                if ($row.loopToDuration -isnot [bool]) { throw 'World sound loopToDuration must be boolean' }
                $soundFields += 'loopToDuration'
            }
            Assert-ExactJsonProperties $row $soundFields 'World sound track'
            if ($row.soundTrackId -isnot [string] -or $row.soundTrackId -cnotmatch $stableId -or -not $soundIds.Add($row.soundTrackId) -or
                $row.assetId -isnot [string] -or -not $row.assetId.StartsWith('Sound/', [StringComparison]::Ordinal) -or
                -not $row.assetId.EndsWith('.wav', [StringComparison]::Ordinal)) { throw 'Invalid World sound identity or asset ID' }
            Assert-SequenceAssetPath $row.assetId $false
            if ([Text.UTF8Encoding]::new($false, $true).GetByteCount($row.assetId) -gt 1024) { throw 'World sound asset ID exceeds its byte limit' }
            foreach ($field in @('startMs','durationMs')) {
                if (-not (Test-JsonNumber $row.$field) -or [double]$row.$field -ne [math]::Floor([double]$row.$field)) { throw "Sound $field must be integer milliseconds" }
            }
            if ($row.startMs -lt 0 -or $row.startMs -gt $template.durationMs -or $row.durationMs -lt 1 -or
                [double]$row.startMs + [double]$row.durationMs -gt 600000 -or
                -not (Test-JsonNumber $row.volume) -or $row.volume -lt 0 -or $row.volume -gt 4) { throw 'Invalid World sound time or volume' }
            if (-not (Test-Path -LiteralPath (Join-Path $runtimeResourceRoot $row.assetId) -PathType Leaf)) { throw "World sound asset is missing: $($row.assetId)" }
        }
        $subtitleIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($row in @($template.subtitleTracks | Where-Object { $null -ne $_ })) {
            Assert-ExactJsonProperties $row @('subtitleTrackId','stringId','text','position','slotId','startMs','durationMs') 'World subtitle track'
            foreach ($field in @('subtitleTrackId','stringId','text','position','slotId')) {
                if ($row.$field -isnot [string]) { throw "Subtitle $field must be text" }
            }
            if ($row.subtitleTrackId -cnotmatch $stableId -or -not $subtitleIds.Add($row.subtitleTrackId) -or $row.stringId -cnotmatch $stableId -or
                [Text.UTF8Encoding]::new($false, $true).GetByteCount($row.text) -notin 1..4096 -or
                $row.text -match '[\x00-\x09\x0b-\x1f\x7f<>]' -or $row.position -cnotin @('NORMAL','UPPER','BALLOON')) { throw 'Invalid World subtitle identity or plain UTF-8 text' }
            if ($row.position -ceq 'BALLOON') {
                if ($row.slotId -cnotmatch $stableId -or @($template.tracks | Where-Object { $_.slotId -ceq $row.slotId }).Count -ne 1) { throw 'Balloon subtitle requires an existing transform slot' }
            } elseif ($row.slotId -cne '') { throw 'Screen subtitle cannot bind an Object slot' }
            foreach ($field in @('startMs','durationMs')) {
                if (-not (Test-JsonNumber $row.$field) -or [double]$row.$field -ne [math]::Floor([double]$row.$field)) { throw "Subtitle $field must be integer milliseconds" }
            }
            if ($row.startMs -lt 0 -or $row.durationMs -lt 1 -or [double]$row.startMs + [double]$row.durationMs -gt $template.durationMs) { throw 'World subtitle exceeds its template clock' }
        }
        $materialTargets = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($row in @($template.materialTracks | Where-Object { $null -ne $_ })) {
            Assert-ExactJsonProperties $row @('slotId','materialName','curves') 'World material track'
            if ($row.slotId -isnot [string] -or -not $slotIds.Contains($row.slotId) -or
                $row.materialName -isnot [string] -or [Text.Encoding]::UTF8.GetByteCount($row.materialName) -notin 1..256 -or
                $row.materialName -match '[\x00-\x1f\x7f]' -or -not $materialTargets.Add("$($row.slotId):$($row.materialName)") -or
                $row.curves -isnot [System.Array] -or $row.curves.Count -notin 1..64) { throw 'Invalid World material target or curves' }
            $parameters = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            foreach ($curve in $row.curves) {
                Assert-ExactJsonProperties $curve @('parameter','keys') 'World material curve'
                if ($curve.parameter -isnot [string] -or [Text.Encoding]::UTF8.GetByteCount($curve.parameter) -notin 1..128 -or
                    $curve.parameter -match '[\x00-\x1f\x7f]' -or -not $parameters.Add($curve.parameter) -or
                    $curve.keys -isnot [System.Array] -or $curve.keys.Count -notin 1..4096) { throw 'Invalid World material curve' }
                $previous = -1
                foreach ($key in $curve.keys) {
                    Assert-ExactJsonProperties $key @('timeMs','value','interpolation') 'World material key'
                    if (-not (Test-JsonNumber $key.timeMs) -or $key.timeMs -ne [math]::Floor([double]$key.timeMs) -or
                        $key.timeMs -le $previous -or $key.timeMs -gt $template.durationMs -or
                        $key.value -isnot [System.Array] -or $key.value.Count -ne 4 -or
                        $key.interpolation -cnotin @('LINEAR','CONSTANT')) { throw 'Invalid World material key time or interpolation' }
                    foreach ($value in $key.value) {
                        if (-not (Test-JsonNumber $value) -or [math]::Abs([double]$value) -gt 1000000) { throw 'Invalid World material key value' }
                    }
                    $previous = $key.timeMs
                }
                if ($curve.keys[0].timeMs -ne 0 -or $curve.keys[-1].timeMs -ne $template.durationMs) { throw 'World material curve must cover its motion' }
            }
        }
        # One binding per slot, so a chained slot and a slot that also carries
        # a transform track each still count once.
        $trackCounts[[string]$template.sequenceId] = $slotIds.Count
    }
    $instanceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $instanceRows = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    foreach ($instance in $instances) {
        $instanceProperties = @('instanceId','templateId','enabled','startDelayMs','playbackSpeed','bindings')
        foreach ($optional in @('anchorKind','position','motionEnd','nextMotionId','walkableSurface','loopFullPresentation')) {
            if ($document.formatVersion -eq 3 -and $null -ne $instance.PSObject.Properties[$optional]) { $instanceProperties += $optional }
        }
        Assert-ExactJsonProperties $instance $instanceProperties 'World sequence instance'
        if ($null -ne $instance.PSObject.Properties['anchorKind'] -and $instance.anchorKind -cnotin @('WORLD','PLAYER','BOSS')) { throw 'Invalid world object anchor' }
        $instanceAnchor = 'WORLD'
        if ($null -ne $instance.PSObject.Properties['anchorKind']) { $instanceAnchor = [string]$instance.anchorKind }
        if ($null -ne $instance.PSObject.Properties['position']) { Assert-SequenceVector $instance.position 'World object instance position' }
        if ($instance.instanceId -isnot [string] -or
            $instance.instanceId -notmatch $stableId -or
            -not $instanceIds.Add([string]$instance.instanceId) -or
            $instance.templateId -isnot [string] -or
            -not $trackCounts.ContainsKey([string]$instance.templateId) -or
            $instance.enabled -isnot [bool] -or
            -not (Test-JsonNumber $instance.startDelayMs) -or
            [double]$instance.startDelayMs -lt 0 -or
            [double]$instance.startDelayMs -gt 600000 -or
            -not (Test-JsonNumber $instance.playbackSpeed) -or
            [double]$instance.playbackSpeed -lt 0.05 -or
            [double]$instance.playbackSpeed -gt 8.0 -or
            $instance.bindings -isnot [System.Array]) {
            throw "World sequence instance is invalid: $($instance.instanceId)"
        }
        if ([double]$instance.startDelayMs -ne [math]::Floor([double]$instance.startDelayMs)) { throw 'World sequence start delay must be integer milliseconds' }
        $bindings = @($instance.bindings)
        if ($bindings.Count -ne $trackCounts[[string]$instance.templateId]) {
            throw "World sequence instance binding count does not match its template: $($instance.instanceId)"
        }
        if ($instanceAnchor -ceq 'BOSS' -and ($bindings.Count -ne 1 -or $bindings[0].targetKind -cne 'OBJECT_RESOURCE')) {
            throw 'Boss-anchored world sequence requires exactly one Object Resource binding'
        }
        $motionEnd = 'STOP'
        if ($null -ne $instance.PSObject.Properties['motionEnd']) {
            if ($instance.motionEnd -isnot [string] -or $instance.motionEnd -cnotin @('STOP','HOLD','LOOP','NEXT')) { throw 'Invalid world object motion completion' }
            $motionEnd = [string]$instance.motionEnd
        }
        if ($null -ne $instance.PSObject.Properties['loopFullPresentation'] -and
            ($instance.loopFullPresentation -isnot [bool] -or
            ($instance.loopFullPresentation -and ($motionEnd -cne 'LOOP' -or
             $bindings.Count -ne 1 -or $bindings[0].targetKind -cne 'OBJECT_RESOURCE')))) {
            throw 'Full presentation loop requires a boolean and one looping Object Resource'
        }
        $nextMotionId = ''
        if ($null -ne $instance.PSObject.Properties['nextMotionId']) {
            if ($instance.nextMotionId -isnot [string]) { throw 'Invalid world object next motion ID' }
            $nextMotionId = [string]$instance.nextMotionId
        }
        if (($motionEnd -eq 'NEXT' -and $nextMotionId -cnotmatch $stableId) -or
            ($motionEnd -ne 'NEXT' -and $nextMotionId -ne '') -or
            ($motionEnd -ne 'STOP' -and ($bindings.Count -ne 1 -or $bindings[0].targetKind -cne 'OBJECT_RESOURCE'))) {
            throw "Invalid world object motion completion: $($instance.instanceId)"
        }
        if ($null -ne $instance.PSObject.Properties['walkableSurface']) {
            $surface = $instance.walkableSurface
            Assert-ExactJsonProperties $surface @('radiusM','localHeightM') 'Walkable surface'
            foreach ($field in @('radiusM','localHeightM')) {
                if ($surface.$field -isnot [ValueType] -or $surface.$field -is [bool] -or
                    [double]::IsNaN([double]$surface.$field) -or [double]::IsInfinity([double]$surface.$field)) {
                    throw "Walkable surface $field must be finite"
                }
            }
            $surfaceTemplate = $templateRows[$instance.templateId]
            if ([double]$surface.radiusM -lt 0.001 -or [double]$surface.radiusM -gt 1000 -or
                [math]::Abs([double]$surface.localHeightM) -gt 10000 -or $bindings.Count -ne 1 -or
                $bindings[0].targetKind -cne 'MAP_PLACEMENT' -or $motionEnd -cne 'STOP' -or
                @($surfaceTemplate.tracks).Count -ne 1 -or @($surfaceTemplate.animationTracks).Count -ne 0) {
                throw "Walkable surface requires one fixed Map placement: $($instance.instanceId)"
            }
            $first = $surfaceTemplate.tracks[0].keys[0]
            foreach ($key in $surfaceTemplate.tracks[0].keys) {
                if ([math]::Abs([double]$key.rotationQuaternion[0]) -gt 0.00001 -or
                    [math]::Abs([double]$key.rotationQuaternion[2]) -gt 0.00001 -or
                    [double]$key.scaleMultiplier[0] -le 0 -or [double]$key.scaleMultiplier[1] -le 0 -or
                    [math]::Abs([double]$key.scaleMultiplier[0] - [double]$key.scaleMultiplier[2]) -gt 0.00001) {
                    throw "Walkable surface must be horizontal with uniform X/Z scale: $($instance.instanceId)"
                }
                foreach ($axis in 0..2) {
                    if ($key.positionOffset[$axis] -ne $first.positionOffset[$axis] -or
                        $key.scaleMultiplier[$axis] -ne $first.scaleMultiplier[$axis]) {
                        throw "Walkable surface position and scale must stay fixed: $($instance.instanceId)"
                    }
                }
            }
        }
        $instanceRows.Add([string]$instance.instanceId, $instance)
        $boundSlots = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $boundTargets = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $template = $templateRows[[string]$instance.templateId]
        foreach ($subtitle in @($template.subtitleTracks | Where-Object { $null -ne $_ -and $_.position -ceq 'BALLOON' })) {
            if (@($bindings | Where-Object { $_.slotId -ceq $subtitle.slotId -and $_.targetKind -ceq 'OBJECT_RESOURCE' }).Count -ne 1) {
                throw "World balloon subtitle requires its Object Resource binding: $($instance.instanceId)"
            }
        }
        if ($null -ne $template.PSObject.Properties['effectTracks'] -and @($template.effectTracks).Count -gt 0 -and
            ($bindings.Count -ne 1 -or $bindings[0].targetKind -cne 'OBJECT_RESOURCE')) {
            throw 'World Object effect lanes require one Object Resource binding'
        }
        foreach ($binding in $bindings) {
            $bindingFields = @('slotId','targetKind','targetId')
            if ($null -ne $binding.PSObject.Properties['previewNpcPlacementId']) { $bindingFields += 'previewNpcPlacementId' }
            Assert-ExactJsonProperties $binding $bindingFields 'World sequence binding'
            if ($null -ne $binding.PSObject.Properties['previewNpcPlacementId']) {
                $resource = $objectResources[$binding.targetId]
                if ($binding.targetKind -cne 'OBJECT_RESOURCE' -or $null -eq $resource -or -not $resource.animated -or
                    $binding.previewNpcPlacementId -isnot [string] -or $binding.previewNpcPlacementId -cnotmatch $stableId -or
                    $bindings.Count -ne 1 -or $instanceAnchor -cne 'WORLD' -or $motionEnd -cne 'STOP' -or
                    -not [string]::IsNullOrEmpty($resource.sequenceInstanceId) -or
                    ($null -ne $template.objectMotion -and $template.objectMotion.count -ne 1) -or
                    @($template.colliderTracks | Where-Object { $null -ne $_ }).Count -ne 0 -or
                    $null -ne $resource.PSObject.Properties['combatBody'] -or
                    $null -ne $resource.PSObject.Properties['presentationBossArchetypeId']) {
                    throw "Invalid editor NPC replacement: $($instance.instanceId)"
                }
                $world = Get-Content -LiteralPath (Join-Path $ProjectRoot "Data/Worlds/$AreaId/Gameplay.world.json") -Raw -Encoding UTF8 | ConvertFrom-Json
                $npc = @($world.placements | Where-Object { $_.placementId -ceq $binding.previewNpcPlacementId -and $_.kind -ceq 'npc' -and $_.enabled })
                $npcCatalog = Get-Content -LiteralPath (Join-Path $ProjectRoot 'Data/Actors/NpcCatalog.json') -Raw -Encoding UTF8 | ConvertFrom-Json
                $actor = @($npcCatalog.npcs | Where-Object { $npc.Count -eq 1 -and $_.archetypeId -ceq $npc[0].archetypeId })
                if ($npc.Count -ne 1 -or $actor.Count -ne 1 -or $actor[0].modelAssetId -cne $resource.modelAssetId) {
                    throw "NPC preview must match one enabled placement and its exact model: $($instance.instanceId)"
                }
            }
            if ($binding.slotId -isnot [string] -or
                -not $boundSlots.Add([string]$binding.slotId) -or
                $binding.targetKind -isnot [string] -or
                $binding.targetKind -cnotin @('MAP_PLACEMENT','DEPLOY_PLACEMENT','OBJECT_RESOURCE') -or
                $binding.targetId -isnot [string] -or
                ($binding.targetKind -eq 'OBJECT_RESOURCE' -and $binding.targetId -cnotmatch $stableId) -or
                ($binding.targetKind -ne 'OBJECT_RESOURCE' -and $binding.targetId -notmatch '^[0-9]{1,20}$') -or
                -not $boundTargets.Add("$($binding.targetKind):$($binding.targetId)")) {
                throw "World sequence binding is invalid: $($instance.instanceId)"
            }
            $template = $templateRows[$instance.templateId]
            $transformTracks = @($template.tracks | Where-Object { $_.slotId -ceq $binding.slotId })
            $animationTracks = @($template.animationTracks | Where-Object { $_.slotId -ceq $binding.slotId })
            foreach ($material in @($template.materialTracks | Where-Object { $null -ne $_ -and $_.slotId -ceq $binding.slotId })) {
                if ($binding.targetKind -cne 'OBJECT_RESOURCE' -or -not $objectResources.ContainsKey($binding.targetId)) { throw 'World material track requires an Object Resource' }
                $profile = $objectResources[$binding.targetId].materialProfile
                if ($null -eq $profile -or $profile.materialName -cne $material.materialName) { throw 'World material track requires its exact Object material profile' }
                foreach ($curve in $material.curves) {
                    if ($null -eq $profile.parameters.PSObject.Properties[$curve.parameter]) { throw "World material parameter is absent: $($curve.parameter)" }
                }
            }
            if ($binding.targetKind -eq 'OBJECT_RESOURCE') {
                if (-not $objectResources.ContainsKey($binding.targetId)) { throw 'Unknown world object binding resource' }
                $resource = $objectResources[$binding.targetId]
                $resourceAnchor = 'WORLD'
                if ($null -ne $resource.PSObject.Properties['anchorKind']) { $resourceAnchor = [string]$resource.anchorKind }
                if (($instanceAnchor -ceq 'BOSS') -ne ($resourceAnchor -ceq 'BOSS')) {
                    throw 'Boss-anchored world sequence and Object Resource anchors must match'
                }
                if ($resource.modelAssetId -eq '' -or ($animationTracks.Count -gt 0 -and -not $resource.animated) -or
                    ($transformTracks.Count -eq 0 -and $animationTracks.Count -eq 0)) { throw 'Invalid object resource slot or animation binding' }
            } else {
                foreach ($track in $transformTracks) {
                    if (@($track.keys[0].scaleMultiplier | Where-Object { [double]$_ -lt 0 }).Count -gt 0) {
                        throw 'Signed scale requires an Object Resource binding'
                    }
                }
                $numericTarget = [uint64]0
                if (-not [uint64]::TryParse($binding.targetId, [ref]$numericTarget) -or $numericTarget -eq 0) { throw 'Placed sequence target ID is outside uint64 range' }
                if (($null -ne $instance.PSObject.Properties['anchorKind'] -and $instance.anchorKind -ne 'WORLD') -or
                    ($null -ne $instance.PSObject.Properties['position'] -and @($instance.position | Where-Object { $_ -ne 0 }).Count -gt 0)) {
                    throw 'Placed sequences cannot use object instance anchors'
                }
                if (($binding.targetKind -eq 'MAP_PLACEMENT' -and ($transformTracks.Count -ne 1 -or $animationTracks.Count -gt 0)) -or
                    ($binding.targetKind -eq 'DEPLOY_PLACEMENT' -and $animationTracks.Count -eq 0)) { throw 'Invalid placed sequence slot binding' }
            }
        }
    }
    foreach ($instance in $instances) {
        if ($null -eq $instance.PSObject.Properties['motionEnd'] -or $instance.motionEnd -cne 'NEXT') { continue }
        if (-not $instanceRows.ContainsKey($instance.nextMotionId)) { throw "Unknown NEXT motion: $($instance.instanceId)" }
        $target = $instanceRows[$instance.nextMotionId]
        if (-not $target.enabled -or @($target.bindings).Count -ne 1 -or
            $target.bindings[0].targetKind -cne 'OBJECT_RESOURCE' -or
            $target.bindings[0].targetId -cne $instance.bindings[0].targetId -or
            $target.bindings[0].slotId -cne $instance.bindings[0].slotId) {
            throw "NEXT motion must target an enabled object state with the same resource and slot: $($instance.instanceId)"
        }
        foreach ($state in @($instance, $target)) {
            $stateTemplate = $templateRows[$state.templateId]
            if ($null -ne $stateTemplate.PSObject.Properties['objectMotion'] -and $stateTemplate.objectMotion.count -ne 1) {
                throw "NEXT motion requires a single object emission: $($instance.instanceId)"
            }
        }
    }
    foreach ($instance in $instances) {
        $visited = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $current = $instance
        $depth = 0
        while ($null -ne $current.PSObject.Properties['motionEnd'] -and $current.motionEnd -ceq 'NEXT') {
            $depth += 1
            if (-not $visited.Add([string]$current.instanceId) -or $depth -gt 32) {
                throw "World object NEXT motion chain contains a cycle or exceeds 32 links: $($instance.instanceId)"
            }
            $current = $instanceRows[$current.nextMotionId]
        }
    }
    foreach ($resource in $objectResources.Values) {
        if ($null -ne $resource.PSObject.Properties['motionInstanceIds']) {
            foreach ($id in $resource.motionInstanceIds) {
                if (-not $instanceRows.ContainsKey($id)) { throw "Unknown Object group motion: $id" }
                $member = $instanceRows[$id]
                if (($null -ne $member.PSObject.Properties['anchorKind'] -and $member.anchorKind -cne 'WORLD') -or
                    ($null -ne $member.PSObject.Properties['motionEnd'] -and $member.motionEnd -cnotin @('STOP', 'LOOP')) -or
                    @($member.bindings).Count -ne 1 -or $member.bindings[0].targetKind -cne 'OBJECT_RESOURCE') {
                    throw 'Object group member must be one Map Object motion ending with Stop or Loop'
                }
                $model = $objectResources[$member.bindings[0].targetId]
                if ($null -eq $model -or $model.modelAssetId -ceq '' -or $null -ne $model.PSObject.Properties['motionInstanceIds']) {
                    throw 'Object group member must bind a model, not a group'
                }
            }
            continue
        }
        if ($null -ne $resource.PSObject.Properties['sequenceInstanceId'] -and $resource.sequenceInstanceId -ne '' -and
            -not $instanceIds.Contains($resource.sequenceInstanceId)) { throw 'Unknown world object sequence alias' }
        if ($null -eq $resource.PSObject.Properties['defaultMotionInstanceId']) { continue }
        $defaultId = $resource.defaultMotionInstanceId
        if ($defaultId -isnot [string]) { throw 'Default Motion instance ID must be text' }
        if ($defaultId -ceq '') { continue }
        if ($defaultId -cnotmatch $stableId -or -not $instanceRows.ContainsKey($defaultId)) { throw 'Default Motion instance does not exist' }
        $motion = $instanceRows[$defaultId]
        $alias = $null -ne $resource.PSObject.Properties['sequenceInstanceId'] -and $resource.sequenceInstanceId -cne ''
        if (-not $motion.enabled -or ($alias -and $defaultId -cne $resource.sequenceInstanceId) -or
            (-not $alias -and (@($motion.bindings).Count -ne 1 -or $motion.bindings[0].targetKind -cne 'OBJECT_RESOURCE' -or
                              $motion.bindings[0].targetId -cne $resource.objectId))) {
            throw 'Default Motion must be an enabled instance of the same Object'
        }
    }
    # Publish the exact snapshot just validated, even if another editor saves
    # the source while this asynchronous process is checking it.
    $reader = [IO.StringReader]::new($raw)
    $validatedLines = [Collections.Generic.List[string]]::new()
    try {
        while ($null -ne ($line = $reader.ReadLine())) { $validatedLines.Add($line) }
    } finally { $reader.Dispose() }
    return $validatedLines.ToArray()
}
```

## 저장·게시 JSON 변경 행 · Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.camerashots.json

revision: 4 → 5

### shots / maharaka.waterpang.source.intro15.camera.1

```json
{
  "shotId": "maharaka.waterpang.source.intro15.camera.1",
  "displayName": "워터팡 / 원본 도입 카메라 15 (카메라만) 카메라 / cm010",
  "defaultHoldMs": 4784,
  "transitionEasing": "LINEAR",
  "activation": "PATTERN_ONLY",
  "sequenceInstanceId": "",
  "box": {
    "center": [
      59.44954003242618,
      35.44,
      -968.8011991852993
    ],
    "halfExtents": [
      1,
      1,
      1
    ],
    "yawDegrees": 0
  },
  "eye": [
    59.44954003242618,
    35.44,
    -968.8011991852993
  ],
  "lookAt": [
    65.22420520435367,
    29.722749654568023,
    -974.6292598095366
  ],
  "fovYDegrees": 29.39495760413827,
  "blendInMs": 0,
  "blendOutMs": 0,
  "priority": 100,
  "cameraTrack": {
    "durationMs": 4784,
    "interpolation": "LINEAR",
    "easing": "LINEAR",
    "keyframes": [
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k0",
        "timeMs": 0,
        "eye": [
          59.44954003242618,
          35.44,
          -968.8011991852993
        ],
        "lookAt": [
          65.22420520435367,
          29.722749654568023,
          -974.6292598095366
        ],
        "up": [
          0.41649068419220714,
          0.8202821674693243,
          -0.39201106580418704
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k1",
        "timeMs": 224,
        "eye": [
          59.44954003242618,
          35.44,
          -968.8011991852993
        ],
        "lookAt": [
          65.22420520435367,
          29.722749654568023,
          -974.6292598095366
        ],
        "up": [
          0.41649068419220714,
          0.8202821674693243,
          -0.39201106580418704
        ],
        "fovYDegrees": 29.44646485184345
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k2",
        "timeMs": 464,
        "eye": [
          59.44954003242618,
          35.44,
          -968.8011991852993
        ],
        "lookAt": [
          65.22420520435367,
          29.722749654568023,
          -974.6292598095366
        ],
        "up": [
          0.41649068419220714,
          0.8202821674693243,
          -0.39201106580418704
        ],
        "fovYDegrees": 29.6036181845647
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k3",
        "timeMs": 720,
        "eye": [
          59.44954003242618,
          35.44,
          -968.8011991852993
        ],
        "lookAt": [
          65.22420520435367,
          29.722749654568023,
          -974.6292598095366
        ],
        "up": [
          0.41649068419220714,
          0.8202821674693243,
          -0.39201106580418704
        ],
        "fovYDegrees": 29.86580783546286
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k4",
        "timeMs": 1008,
        "eye": [
          59.448212902200275,
          35.440285263011496,
          -968.7998707089272
        ],
        "lookAt": [
          65.22299575075273,
          29.72328844815288,
          -974.6280634366053
        ],
        "up": [
          0.41647011797310296,
          0.8202998836207019,
          -0.3919958440689553
        ],
        "fovYDegrees": 30.248357341502906
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k5",
        "timeMs": 1040,
        "eye": [
          59.43367077354691,
          35.44341189961616,
          -968.7853139087097
        ],
        "lookAt": [
          65.20974335591967,
          29.729194079286202,
          -974.6149537110462
        ],
        "up": [
          0.4162448594178936,
          0.8204939935911565,
          -0.3918288191149465
        ],
        "fovYDegrees": 30.295187135311807
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k6",
        "timeMs": 1072,
        "eye": [
          59.40375627627942,
          35.449846058577855,
          -968.7553694527554
        ],
        "lookAt": [
          65.1824816337522,
          29.74134851334877,
          -974.5879837053025
        ],
        "up": [
          0.41578168688766765,
          0.8208930975881182,
          -0.3914844967313899
        ],
        "fovYDegrees": 30.342709923989716
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k7",
        "timeMs": 1104,
        "eye": [
          59.3592974993861,
          35.45941232624239,
          -968.7108665109442
        ],
        "lookAt": [
          65.1419629637356,
          29.759423885013977,
          -974.5478958836031
        ],
        "up": [
          0.41509353765191587,
          0.8214856636800401,
          -0.39097143037283694
        ],
        "fovYDegrees": 30.390878669018072
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k8",
        "timeMs": 1136,
        "eye": [
          59.301122531855235,
          35.47193528895558,
          -968.6526342531553
        ],
        "lookAt": [
          65.08893802717381,
          29.78309391018292,
          -974.4954311484199
        ],
        "up": [
          0.41419324577386174,
          0.8222599282323113,
          -0.3902980471110604
        ],
        "fovYDegrees": 30.43964626845343
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k9",
        "timeMs": 1184,
        "eye": [
          59.189953667775484,
          35.49587981887303,
          -968.541357208125
        ],
        "lookAt": [
          64.98759033329875,
          29.828378069637385,
          -974.3951451285225
        ],
        "up": [
          0.4124729259820144,
          0.8237358447175065,
          -0.38900558281260145
        ],
        "fovYDegrees": 30.51381732087105
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k10",
        "timeMs": 1216,
        "eye": [
          59.101111112974586,
          35.51502708447077,
          -968.4524292786145
        ],
        "lookAt": [
          64.90657618753424,
          29.86461432413308,
          -974.3149723194698
        ],
        "up": [
          0.41109796317250696,
          0.8249118236793223,
          -0.3879677149318883
        ],
        "fovYDegrees": 30.563875545544054
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k11",
        "timeMs": 1248,
        "eye": [
          59.00145067899519,
          35.536517097327504,
          -968.3526741277046
        ],
        "lookAt": [
          64.81567507453036,
          29.905310526461108,
          -974.2250081810938
        ],
        "up": [
          0.40955530017969966,
          0.8262272171106881,
          -0.3867984511347534
        ],
        "fovYDegrees": 30.614367231448966
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k12",
        "timeMs": 1280,
        "eye": [
          58.8918004548256,
          35.560174443789045,
          -968.2429209252748
        ],
        "lookAt": [
          64.71563441743422,
          29.950143225728837,
          -974.1259905074195
        ],
        "up": [
          0.40785758530619803,
          0.8276698102638034,
          -0.3855059990273528
        ],
        "fovYDegrees": 30.665244960721235
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k13",
        "timeMs": 1312,
        "eye": [
          58.772988529454096,
          35.585823710201204,
          -968.1239988412042
        ],
        "lookAt": [
          64.60720175905736,
          29.99878878106685,
          -974.0186571597291
        ],
        "up": [
          0.40601747886638134,
          0.829227402209409,
          -0.38409858406406894
        ],
        "fovYDegrees": 30.716461239517578
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k14",
        "timeMs": 1344,
        "eye": [
          58.645842991868996,
          35.6132894829098,
          -967.9967370453726
        ],
        "lookAt": [
          64.49112502258826,
          30.050923114906897,
          -973.90374630298
        ],
        "up": [
          0.4040476691707213,
          0.8308878412252163,
          -0.3825844695512447
        ],
        "fovYDegrees": 30.767968496290692
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k15",
        "timeMs": 1376,
        "eye": [
          58.51119193105857,
          35.64239634826064,
          -967.8619647076594
        ],
        "lookAt": [
          64.36815275135966,
          30.10622149079325,
          -973.78199662455
        ],
        "up": [
          0.40196088687933457,
          0.832639057428231,
          -0.3809719746440754
        ],
        "fovYDegrees": 30.819719080241004
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k16",
        "timeMs": 1440,
        "eye": [
          58.22268559571496,
          35.7048317022723,
          -967.5732050861056
        ],
        "lookAt": [
          64.10452017087624,
          30.225006968018622,
          -973.5209393532277
        ],
        "up": [
          0.39748761354502476,
          0.8363661275844062,
          -0.37748549337391246
        ],
        "fovYDegrees": 30.923759222183108
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k17",
        "timeMs": 1504,
        "eye": [
          57.91409423532965,
          35.7717264630027,
          -967.2643533355791
        ],
        "lookAt": [
          63.822312559310376,
          30.35252725274018,
          -973.2414124916917
        ],
        "up": [
          0.3927007935008756,
          0.8403147567107976,
          -0.3737073673850384
        ],
        "fovYDegrees": 31.028198826458983
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k18",
        "timeMs": 1568,
        "eye": [
          57.592042561808995,
          35.841677321218334,
          -966.9420428151152
        ],
        "lookAt": [
          63.52756019886504,
          30.486143745967937,
          -972.9493625589413
        ],
        "up": [
          0.38770488432480243,
          0.8443940215879545,
          -0.36970753167498904
        ],
        "fovYDegrees": 31.13265371769669
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k19",
        "timeMs": 1712,
        "eye": [
          56.85252596393897,
          36.0029719053385,
          -966.2019942433689
        ],
        "lookAt": [
          62.849914496154724,
          30.795241016865692,
          -972.2774258977365
        ],
        "up": [
          0.37625100769019937,
          0.85360333986062,
          -0.36027283743144545
        ],
        "fovYDegrees": 31.365734000204938
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k20",
        "timeMs": 1792,
        "eye": [
          56.454506016694125,
          36.09031181561062,
          -965.8037373013144
        ],
        "lookAt": [
          62.4848275866181,
          30.963150709095483,
          -971.9149925529913
        ],
        "up": [
          0.3701166943318876,
          0.8584729289517519,
          -0.3550181162051268
        ],
        "fovYDegrees": 31.492787924153824
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k21",
        "timeMs": 1840,
        "eye": [
          56.22712912719141,
          36.14044707987894,
          -965.5762474754949
        ],
        "lookAt": [
          62.27620438344653,
          31.059683535050635,
          -971.7076853448455
        ],
        "up": [
          0.36663213338060513,
          0.8612297542305094,
          -0.35194344602626515
        ],
        "fovYDegrees": 31.567771472707076
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k22",
        "timeMs": 1872,
        "eye": [
          56.082034335766956,
          36.17255986015538,
          -965.4310918369529
        ],
        "lookAt": [
          62.143075596162554,
          31.121564359462013,
          -971.57529424994
        ],
        "up": [
          0.3644199974524298,
          0.8629803828477218,
          -0.3499470306728868
        ],
        "fovYDegrees": 31.617138321438393
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k23",
        "timeMs": 1936,
        "eye": [
          55.811146677151065,
          36.232850309098524,
          -965.1601220131372
        ],
        "lookAt": [
          61.894590735622124,
          31.23782577135325,
          -971.3278961865952
        ],
        "up": [
          0.360325579162892,
          0.8662337883330178,
          -0.34612786792044825
        ],
        "fovYDegrees": 31.714158176934838
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k24",
        "timeMs": 1968,
        "eye": [
          55.68700998793622,
          36.26067715045685,
          -965.0359661676224
        ],
        "lookAt": [
          61.78078674674778,
          31.291509829796986,
          -971.2144175069998
        ],
        "up": [
          0.35847175641400936,
          0.8677199465410725,
          -0.3443255642966601
        ],
        "fovYDegrees": 31.761712934326667
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k25",
        "timeMs": 2096,
        "eye": [
          55.2180136516434,
          36.36659361844859,
          -964.5669787191541
        ],
        "lookAt": [
          61.353390229444315,
          31.497978571272405,
          -970.7841920636503
        ],
        "up": [
          0.35254808438074864,
          0.8733821058916477,
          -0.3360261080745597
        ],
        "fovYDegrees": 31.944414663118543
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k26",
        "timeMs": 2192,
        "eye": [
          54.84923293414249,
          36.45002757945644,
          -964.1982336700627
        ],
        "lookAt": [
          61.020923262397204,
          31.664119364916395,
          -970.4436576792174
        ],
        "up": [
          0.34958062353393565,
          0.8778859404346874,
          -0.32727612689704294
        ],
        "fovYDegrees": 32.07216395472884
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k27",
        "timeMs": 2288,
        "eye": [
          54.46899447811462,
          36.53618494812102,
          -963.8180565020697
        ],
        "lookAt": [
          60.68041183146221,
          31.83800142842317,
          -970.0906449691837
        ],
        "up": [
          0.3477281007455067,
          0.8825030829382174,
          -0.31665987519176586
        ],
        "fovYDegrees": 32.19047434120192
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k28",
        "timeMs": 2400,
        "eye": [
          54.0167504699826,
          36.63886299592656,
          -963.3659199126859
        ],
        "lookAt": [
          60.277467199842285,
          32.04735696332144,
          -969.6684192884102
        ],
        "up": [
          0.3467957559774903,
          0.8878757303336303,
          -0.30233986029057336
        ],
        "fovYDegrees": 32.3147825390795
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k29",
        "timeMs": 2640,
        "eye": [
          53.047608500827785,
          36.85987875080212,
          -962.3971572666794
        ],
        "lookAt": [
          59.41771169625638,
          32.501592533501466,
          -968.7553902387934
        ],
        "up": [
          0.34834123205394035,
          0.8985581215703317,
          -0.2669301185912524
        ],
        "fovYDegrees": 32.52181261290543
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k30",
        "timeMs": 2736,
        "eye": [
          52.67186797287838,
          36.94604977961904,
          -962.0216326269037
        ],
        "lookAt": [
          59.08463505163644,
          32.67845634591727,
          -968.3984707680378
        ],
        "up": [
          0.34999569169900974,
          0.902259538681689,
          -0.2518546022014251
        ],
        "fovYDegrees": 32.57830508376813
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k31",
        "timeMs": 2816,
        "eye": [
          52.368789788303275,
          37.01582380946209,
          -961.7187652792625
        ],
        "lookAt": [
          58.815666801124685,
          32.82086081974065,
          -968.1093882767817
        ],
        "up": [
          0.35171395530041283,
          0.9050139394613865,
          -0.2392635848338003
        ],
        "fovYDegrees": 32.612413055513
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k32",
        "timeMs": 2928,
        "eye": [
          51.964927270096936,
          37.10927900775409,
          -961.3152482634268
        ],
        "lookAt": [
          58.45638559840466,
          33.00953888344174,
          -967.7225175479932
        ],
        "up": [
          0.3545364184694545,
          0.9083078344054843,
          -0.22199280604666305
        ],
        "fovYDegrees": 32.63879913212432
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k33",
        "timeMs": 3024,
        "eye": [
          51.64249526956468,
          37.18441686790899,
          -960.9931620144786
        ],
        "lookAt": [
          58.16835238067578,
          33.158458297023344,
          -967.4122621500796
        ],
        "up": [
          0.35725971567666104,
          0.910573809366661,
          -0.20789620787816027
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k34",
        "timeMs": 3072,
        "eye": [
          51.490956895141245,
          37.21994653531633,
          -960.8418145708044
        ],
        "lookAt": [
          58.03243303315253,
          33.22759554937663,
          -967.2660050677383
        ],
        "up": [
          0.3587038119072948,
          0.9115077701824318,
          -0.20120924486769387
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k35",
        "timeMs": 3120,
        "eye": [
          51.34660095247127,
          37.25395666601333,
          -960.6976620372261
        ],
        "lookAt": [
          57.90251466602822,
          33.292738362759884,
          -967.1264056885444
        ],
        "up": [
          0.36019255393291694,
          0.9123079405064596,
          -0.1948218308612898
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k36",
        "timeMs": 3200,
        "eye": [
          51.12345709878974,
          37.306937485429856,
          -960.4748858281159
        ],
        "lookAt": [
          57.70054968616609,
          33.39152376230061,
          -966.9100539280034
        ],
        "up": [
          0.36275355332660864,
          0.9133449649352288,
          -0.18496171110904308
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k37",
        "timeMs": 3264,
        "eye": [
          50.96210782391942,
          37.345669625865256,
          -960.3138573187881
        ],
        "lookAt": [
          57.553294734797376,
          33.46085240060018,
          -966.7531468939007
        ],
        "up": [
          0.36485604046155423,
          0.9139103725057088,
          -0.17789856875532625
        ],
        "fovYDegrees": 32.64206322821486
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k38",
        "timeMs": 3328,
        "eye": [
          50.81742428296623,
          37.380846883631946,
          -960.1695190828545
        ],
        "lookAt": [
          57.41992770800619,
          33.52070151802691,
          -966.612051681044
        ],
        "up": [
          0.36698945859937476,
          0.9142449860590396,
          -0.17168238914590087
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k39",
        "timeMs": 3392,
        "eye": [
          50.690705464715364,
          37.412184764039075,
          -960.0431708943528
        ],
        "lookAt": [
          57.30151738929719,
          33.57028237508656,
          -966.4880876301662
        ],
        "up": [
          0.3691384840413319,
          0.9143532142118328,
          -0.16641808573636868
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k40",
        "timeMs": 3456,
        "eye": [
          50.58325035795197,
          37.439398772395805,
          -959.9361125273203
        ],
        "lookAt": [
          57.19912707373381,
          33.608810232745824,
          -966.3825663370887
        ],
        "up": [
          0.3712874629918091,
          0.914239238376291,
          -0.16221046463852415
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k41",
        "timeMs": 3520,
        "eye": [
          50.49609603658071,
          37.46225582594118,
          -959.8493876825949
        ],
        "lookAt": [
          57.113617514700536,
          33.635778615081655,
          -966.2965949571748
        ],
        "up": [
          0.3734129565455928,
          0.9139133592920845,
          -0.15913873064510836
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k42",
        "timeMs": 3616,
        "eye": [
          50.384262058264426,
          37.49210444964777,
          -959.7386985080831
        ],
        "lookAt": [
          57.00275154614351,
          33.67448314601562,
          -966.1901607052046
        ],
        "up": [
          0.3760267402718455,
          0.9135793308811757,
          -0.15484410478682623
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k43",
        "timeMs": 3696,
        "eye": [
          50.300848532601925,
          37.514425581419204,
          -959.6567796886094
        ],
        "lookAt": [
          56.920381779407535,
          33.71054124473595,
          -966.1152813374108
        ],
        "up": [
          0.3776206284500641,
          0.9135609042793553,
          -0.15102693515153234
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k44",
        "timeMs": 3792,
        "eye": [
          50.211575959474644,
          37.53838376863589,
          -959.5698368911424
        ],
        "lookAt": [
          56.83239246633833,
          33.75560971038075,
          -966.0394125500734
        ],
        "up": [
          0.37902572477959345,
          0.913735898323925,
          -0.1463769383115863
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k45",
        "timeMs": 3888,
        "eye": [
          50.133113823962816,
          37.55951532723021,
          -959.4941664362133
        ],
        "lookAt": [
          56.75498660398646,
          33.79961528012977,
          -965.9759835056002
        ],
        "up": [
          0.38008590798479813,
          0.9140037871592888,
          -0.14188650256401977
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k46",
        "timeMs": 3984,
        "eye": [
          50.06440627896286,
          37.57809296548621,
          -959.4285822814039
        ],
        "lookAt": [
          56.68686157866314,
          33.839342584281255,
          -965.9220275449379
        ],
        "up": [
          0.38101967024591427,
          0.9142395004041981,
          -0.13780474152356564
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k47",
        "timeMs": 4096,
        "eye": [
          49.995167584191606,
          37.59690282671466,
          -959.3632330917305
        ],
        "lookAt": [
          56.61741676875869,
          33.875977095424766,
          -965.8671186565084
        ],
        "up": [
          0.38223729502888143,
          0.9143126175991008,
          -0.13389207440354484
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k48",
        "timeMs": 4208,
        "eye": [
          49.93609219963104,
          37.61304075552403,
          -959.3081147502737
        ],
        "lookAt": [
          56.556883305242124,
          33.8972582608884,
          -965.8164237156211
        ],
        "up": [
          0.3839163900896334,
          0.9139890819267296,
          -0.13127133555837212
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k49",
        "timeMs": 4320,
        "eye": [
          49.88550347918618,
          37.626939802569126,
          -959.2613438656009
        ],
        "lookAt": [
          56.50324883416841,
          33.89806097763928,
          -965.7652587732239
        ],
        "up": [
          0.38639493183446366,
          0.9130780312093552,
          -0.13033597191676713
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k50",
        "timeMs": 4448,
        "eye": [
          49.83592883263333,
          37.6406384567638,
          -959.2156967892953
        ],
        "lookAt": [
          56.447669514662294,
          33.86739447223706,
          -965.7001023123048
        ],
        "up": [
          0.39064165917590976,
          0.9110549290150016,
          -0.13182568199619746
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera1.k51",
        "timeMs": 4784,
        "eye": [
          49.72912715669047,
          37.670160781238664,
          -959.1180595604626
        ],
        "lookAt": [
          56.31783857377003,
          33.724026148129106,
          -965.522503589402
        ],
        "up": [
          0.4028029711568599,
          0.9040985282055872,
          -0.1426731149295372
        ],
        "fovYDegrees": 32.64206322821485
      }
    ]
  }
}
```

### shots / maharaka.waterpang.source.intro15.camera.2

```json
{
  "shotId": "maharaka.waterpang.source.intro15.camera.2",
  "displayName": "워터팡 / 원본 도입 카메라 15 (카메라만) 카메라 / cm010",
  "defaultHoldMs": 4723,
  "transitionEasing": "LINEAR",
  "activation": "PATTERN_ONLY",
  "sequenceInstanceId": "",
  "box": {
    "center": [
      49.72912715669047,
      37.670160781238664,
      -959.1180595604626
    ],
    "halfExtents": [
      1,
      1,
      1
    ],
    "yawDegrees": 0
  },
  "eye": [
    49.72912715669047,
    37.670160781238664,
    -959.1180595604626
  ],
  "lookAt": [
    56.31783857377003,
    33.724026148129106,
    -965.522503589402
  ],
  "fovYDegrees": 32.64206322821485,
  "blendInMs": 0,
  "blendOutMs": 0,
  "priority": 100,
  "cameraTrack": {
    "durationMs": 4723,
    "interpolation": "LINEAR",
    "easing": "LINEAR",
    "keyframes": [
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k0",
        "timeMs": 0,
        "eye": [
          49.72912715669047,
          37.670160781238664,
          -959.1180595604626
        ],
        "lookAt": [
          56.31783857377003,
          33.724026148129106,
          -965.522503589402
        ],
        "up": [
          0.4028029711568599,
          0.9040985282055872,
          -0.1426731149295372
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k1",
        "timeMs": 176,
        "eye": [
          49.68142737869289,
          37.683200620039614,
          -959.0759377852436
        ],
        "lookAt": [
          56.26140994117535,
          33.695853362253835,
          -965.463808426017
        ],
        "up": [
          0.40442232405777606,
          0.9026999495713346,
          -0.14688561824636703
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k2",
        "timeMs": 368,
        "eye": [
          49.635611261641024,
          37.695619956703624,
          -959.0365591220112
        ],
        "lookAt": [
          56.209163097939054,
          33.69679639295124,
          -965.4238767406584
        ],
        "up": [
          0.4028903720841502,
          0.9027805349899268,
          -0.1505544875624652
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k3",
        "timeMs": 592,
        "eye": [
          49.590279985286905,
          37.707788957733214,
          -958.9988134585129
        ],
        "lookAt": [
          56.159741960179716,
          33.729223131279284,
          -965.4029670579365
        ],
        "up": [
          0.3975538245895173,
          0.9045415044774789,
          -0.15412859316774005
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k4",
        "timeMs": 816,
        "eye": [
          49.5535983940587,
          37.71753088424899,
          -958.9693450974524
        ],
        "lookAt": [
          56.121563607448266,
          33.78336844933684,
          -965.4024002729838
        ],
        "up": [
          0.38932334343969605,
          0.9075326822718894,
          -0.15751750652332763
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k5",
        "timeMs": 1024,
        "eye": [
          49.527193938577454,
          37.72446947754186,
          -958.9488890599251
        ],
        "lookAt": [
          56.09511004217181,
          33.8420696123774,
          -965.4133650126314
        ],
        "up": [
          0.379942471957755,
          0.9108854111960587,
          -0.1610325609087111
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k6",
        "timeMs": 1248,
        "eye": [
          49.50690965090985,
          37.729743011506955,
          -958.9337553609255
        ],
        "lookAt": [
          56.075382645183474,
          33.902399107968705,
          -965.4304161314711
        ],
        "up": [
          0.36889374788055296,
          0.9145607952335278,
          -0.16581904171854484
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k7",
        "timeMs": 1685,
        "eye": [
          49.49132612474519,
          37.73373437224929,
          -958.9226994595143
        ],
        "lookAt": [
          56.06068418688946,
          33.963661594223794,
          -965.4518713748288
        ],
        "up": [
          0.3482502950995629,
          0.9198151708171117,
          -0.1807257134382243
        ],
        "fovYDegrees": 32.64202487325161
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k8",
        "timeMs": 1696,
        "eye": [
          49.508280727135244,
          37.726672536623354,
          -958.9390868563461
        ],
        "lookAt": [
          56.077905944074196,
          33.95730878601562,
          -965.4683993335188
        ],
        "up": [
          0.34794871616450807,
          0.9198849282524942,
          -0.18095140146838956
        ],
        "fovYDegrees": 32.62148936107029
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k9",
        "timeMs": 1712,
        "eye": [
          49.5870337244763,
          37.693870855653635,
          -959.0152052921226
        ],
        "lookAt": [
          56.157899029577734,
          33.9277998062811,
          -965.5451699163217
        ],
        "up": [
          0.3480048559330084,
          0.9199724954568004,
          -0.1803974164172908
        ],
        "fovYDegrees": 32.52617939665478
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k10",
        "timeMs": 1728,
        "eye": [
          49.727244655195825,
          37.63547128610312,
          -959.1507258365707
        ],
        "lookAt": [
          56.30031510412046,
          33.87526153342901,
          -965.6818488249525
        ],
        "up": [
          0.3486246850287492,
          0.9200421639899685,
          -0.1788386017314536
        ],
        "fovYDegrees": 32.35674782296995
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k11",
        "timeMs": 1744,
        "eye": [
          49.92595446859361,
          37.5527064667404,
          -959.3427885754224
        ],
        "lookAt": [
          56.50214473862588,
          33.80080275960255,
          -965.8755475717959
        ],
        "up": [
          0.3497816133519345,
          0.9200891841974049,
          -0.17631992537394103
        ],
        "fovYDegrees": 32.11714437868756
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k12",
        "timeMs": 1760,
        "eye": [
          50.180204113969396,
          37.44680903633408,
          -959.58853359441
        ],
        "lookAt": [
          56.760377494092424,
          33.705532896576834,
          -966.1233764489555
        ],
        "up": [
          0.35144793472111346,
          0.9201058816138642,
          -0.17288584615241845
        ],
        "fovYDegrees": 31.811416570629582
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k13",
        "timeMs": 1776,
        "eye": [
          50.487034540622936,
          37.31901163365274,
          -959.8851009792655
        ],
        "lookAt": [
          57.07200212729755,
          33.590561837759054,
          -966.4224447825196
        ],
        "up": [
          0.3535950198081641,
          0.920082165300502,
          -0.16858045872165542
        ],
        "fovYDegrees": 31.443683702050098
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k14",
        "timeMs": 1792,
        "eye": [
          50.84348669785399,
          37.17054689746497,
          -960.2296308157212
        ],
        "lookAt": [
          57.4340067852756,
          33.45699982682073,
          -966.7698611733922
        ],
        "up": [
          0.3561934979312974,
          0.9200060111899786,
          -0.16344764117530666
        ],
        "fovYDegrees": 31.01811193779318
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k15",
        "timeMs": 1808,
        "eye": [
          51.24660153496231,
          37.00264746653937,
          -960.619263189509
        ],
        "lookAt": [
          57.843379188939664,
          33.30595733360579,
          -967.162733724711
        ],
        "up": [
          0.3592134285748367,
          0.9198639213884212,
          -0.1575311996381257
        ],
        "fovYDegrees": 30.538890843684573
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k16",
        "timeMs": 1824,
        "eye": [
          51.69342000124765,
          36.81654597964454,
          -961.0511381863611
        ],
        "lookAt": [
          58.29710680615224,
          33.13854493784648,
          -967.5981702558763
        ],
        "up": [
          0.36262446337720083,
          0.9196413602011674,
          -0.15087500511265642
        ],
        "fovYDegrees": 30.010211751991804
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k17",
        "timeMs": 1840,
        "eye": [
          52.18098304600978,
          36.61347507554905,
          -961.5223958920096
        ],
        "lookAt": [
          58.792177014485716,
          32.95587322111411,
          -968.0732785032019
        ],
        "up": [
          0.36639599919331445,
          0.9193231674415033,
          -0.1435231186271199
        ],
        "fovYDegrees": 29.43624822171632
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k18",
        "timeMs": 1856,
        "eye": [
          52.70633161854843,
          36.39466739302151,
          -962.0301763921867
        ],
        "lookAt": [
          59.325577253855364,
          32.75905266733337,
          -968.5851663071927
        ],
        "up": [
          0.3704973227007208,
          0.9188939493625414,
          -0.13551990147764073
        ],
        "fovYDegrees": 28.821138783638162
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k19",
        "timeMs": 1872,
        "eye": [
          53.26650666816339,
          36.161355570830494,
          -962.5716197726246
        ],
        "lookAt": [
          59.894295169151384,
          32.549193572096826,
          -969.1309417864516
        ],
        "up": [
          0.37489774688514366,
          0.9183384473342353,
          -0.12691010806152764
        ],
        "fovYDegrees": 28.168972086853056
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k20",
        "timeMs": 1888,
        "eye": [
          53.85854914415437,
          35.91477224774461,
          -963.143866119055
        ],
        "lookAt": [
          60.49531874294517,
          32.32740596093045,
          -969.7077134982067
        ],
        "up": [
          0.3795667396154113,
          0.9176418841806472,
          -0.11773895946167755
        ],
        "fovYDegrees": 27.48377449710789
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k21",
        "timeMs": 1904,
        "eye": [
          54.47949999582115,
          35.65615006253244,
          -963.7440555172105
        ],
        "lookAt": [
          61.125636418297944,
          32.094799516581446,
          -970.3125905854492
        ],
        "up": [
          0.38447404440286204,
          0.9167902879053236,
          -0.10805219656712277
        ],
        "fovYDegrees": 26.769500138300515
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k22",
        "timeMs": 1920,
        "eye": [
          55.1264001724635,
          35.38672165396259,
          -964.3693280528231
        ],
        "lookAt": [
          61.782237211655946,
          31.85248351532616,
          -970.9426829106656
        ],
        "up": [
          0.3895897933391776,
          0.9157707923717577,
          -0.09789611210226848
        ],
        "fovYDegrees": 26.03002331748396
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k23",
        "timeMs": 1936,
        "eye": [
          55.79629062338115,
          35.10771966080364,
          -965.0168238116248
        ],
        "lookAt": [
          62.4621108157782,
          31.601566772228434,
          -971.5951011761457
        ],
        "up": [
          0.39488461212075565,
          0.9145719143763913,
          -0.08731756148761345
        ],
        "fovYDegrees": 25.269133230735566
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k24",
        "timeMs": 1952,
        "eye": [
          56.48621229787386,
          34.82037672182419,
          -965.6836828793478
        ],
        "lookAt": [
          63.16224769260788,
          31.343157595217534,
          -972.2669570308486
        ],
        "up": [
          0.4003297169983177,
          0.9131838064569898,
          -0.07636395296715418
        ],
        "fovYDegrees": 24.49053081218922
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k25",
        "timeMs": 1968,
        "eye": [
          57.193206145241405,
          34.52592547579282,
          -966.3670453417241
        ],
        "lookAt": [
          63.879639155967915,
          31.078363747799237,
          -972.9553631638043
        ],
        "up": [
          0.4058970034377849,
          0.9115984847215215,
          -0.06508321791101626
        ],
        "fovYDegrees": 23.697827561042942
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k26",
        "timeMs": 1984,
        "eye": [
          57.91431311478352,
          34.22559856147813,
          -967.0640512844859
        ],
        "lookAt": [
          64.61127744393593,
          30.80829242016493,
          -973.6574333840312
        ],
        "up": [
          0.41155912624237034,
          0.9098100309625446,
          -0.05352376263446171
        ],
        "fovYDegrees": 22.894546160934077
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k27",
        "timeMs": 2000,
        "eye": [
          58.64657415579996,
          33.9206286176487,
          -967.7718407933654
        ],
        "lookAt": [
          65.35415578073159,
          30.534050208420084,
          -974.3702826869511
        ],
        "up": [
          0.41728957086555596,
          0.907814768338963,
          -0.04173440346423353
        ],
        "fovYDegrees": 22.084122692088428
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k28",
        "timeMs": 2016,
        "eye": [
          59.38703021759049,
          33.61224828307315,
          -968.4875539540947
        ],
        "lookAt": [
          66.10526842793189,
          30.25674310161715,
          -975.0910273072877
        ],
        "up": [
          0.423062715638979,
          0.9056114099590037,
          -0.029764287127998293
        ],
        "fovYDegrees": 21.26991022838864
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k29",
        "timeMs": 2064,
        "eye": [
          61.62797802060416,
          32.678971322555576,
          -970.653636204702
        ],
        "lookAt": [
          68.37797117760334,
          29.417483129141665,
          -977.2718147414813
        ],
        "up": [
          0.44039657026151907,
          0.8977780821699777,
          0.006736176741114014
        ],
        "fovYDegrees": 18.83693123961783
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k30",
        "timeMs": 2096,
        "eye": [
          63.108669063645884,
          32.06233310590403,
          -972.0848775361112
        ],
        "lookAt": [
          69.87922223568388,
          28.862901152175223,
          -978.7123389594667
        ],
        "up": [
          0.451740532614316,
          0.8916082954797251,
          0.031066680303397126
        ],
        "fovYDegrees": 17.25423486821338
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k31",
        "timeMs": 2112,
        "eye": [
          63.836155185375794,
          31.759375840992085,
          -972.7880744083134
        ],
        "lookAt": [
          70.61668200048061,
          28.59039661634561,
          -979.4199690684692
        ],
        "up": [
          0.4572872591668097,
          0.8882748395171264,
          0.04308331561672743
        ],
        "fovYDegrees": 16.483760192078265
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k32",
        "timeMs": 2128,
        "eye": [
          64.55112297297808,
          31.46163665671414,
          -973.4791755324903
        ],
        "lookAt": [
          71.34136691933989,
          28.322552492249702,
          -980.115344439119
        ],
        "up": [
          0.46272554124089565,
          0.884797778482701,
          0.0549360052733841
        ],
        "fovYDegrees": 15.731140825552014
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k33",
        "timeMs": 2144,
        "eye": [
          65.2506133757525,
          31.1703481918388,
          -974.1553209943739
        ],
        "lookAt": [
          72.05028004803413,
          28.06047025995747,
          -980.79559150051
        ],
        "up": [
          0.4680379736599633,
          0.8811969113194636,
          0.06657671284550506
        ],
        "fovYDegrees": 14.99929369756108
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k34",
        "timeMs": 2160,
        "eye": [
          65.93166734299878,
          30.88674308513465,
          -974.8136508796962
        ],
        "lookAt": [
          72.74042542644104,
          27.80525096496182,
          -981.4578377881503
        ],
        "up": [
          0.4732081603755294,
          0.8774944941776863,
          0.07795799921659013
        ],
        "fovYDegrees": 14.291114286347852
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k35",
        "timeMs": 2176,
        "eye": [
          66.5913258240167,
          30.612053975370284,
          -975.4513052741895
        ],
        "lookAt": [
          73.40880801185494,
          27.557995262409236,
          -982.0992118617022
        ],
        "up": [
          0.47822067613298225,
          0.8737151097100266,
          0.08903309487662217
        ],
        "fovYDegrees": 13.6094835345833
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k36",
        "timeMs": 2192,
        "eye": [
          67.22662976810605,
          30.34751350131429,
          -976.065424263586
        ],
        "lookAt": [
          74.05243359953856,
          27.31980346508851,
          -982.7168432092952
        ],
        "up": [
          0.4830610178993318,
          0.8698855110050328,
          0.09975595586016295
        ],
        "fovYDegrees": 12.957274299990994
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k37",
        "timeMs": 2208,
        "eye": [
          67.83462012456653,
          30.094354301735265,
          -976.6531479336176
        ],
        "lookAt": [
          74.66830873040732,
          27.091775594931743,
          -983.307862138439
        ],
        "up": [
          0.48771554613344614,
          0.8660344411186489,
          0.11008130111905559
        ],
        "fovYDegrees": 12.337357263016935
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k38",
        "timeMs": 2224,
        "eye": [
          68.41233784269791,
          29.853809015401797,
          -977.2116163700165
        ],
        "lookAt": [
          75.25344058576508,
          26.87501143783229,
          -983.8693996535625
        ],
        "up": [
          0.4921714159852157,
          0.8621924292014274,
          0.11996462943238548
        ],
        "fovYDegrees": 11.752606234037074
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k39",
        "timeMs": 2240,
        "eye": [
          68.95682387179995,
          29.62711028108248,
          -977.7379696585148
        ],
        "lookAt": [
          75.80483686903821,
          26.67061060164155,
          -984.3985873201976
        ],
        "up": [
          0.4964164985061914,
          0.8583915642178623,
          0.12936221430720904
        ],
        "fovYDegrees": 11.2059028252764
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k40",
        "timeMs": 2256,
        "eye": [
          69.46511916117242,
          29.415490737545905,
          -978.2293478848446
        ],
        "lookAt": [
          76.31950567448696,
          26.47967257727108,
          -984.8925571158303
        ],
        "up": [
          0.5004392919401364,
          0.8546652472109258,
          0.1382310757114395
        ],
        "fovYDegrees": 10.700140477271054
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k41",
        "timeMs": 2272,
        "eye": [
          69.93426466011506,
          29.22018302356066,
          -978.6828911347382
        ],
        "lookAt": [
          76.7944553429068,
          26.303296802897687,
          -985.3484412674312
        ],
        "up": [
          0.5042288231375054,
          0.8510479229779436,
          0.1465289279026317
        ],
        "fovYDegrees": 10.23822785660264
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k42",
        "timeMs": 2288,
        "eye": [
          70.36130131792763,
          29.042419777895354,
          -979.0957394939275
        ],
        "lookAt": [
          77.22669430436906,
          26.142582731347915,
          -985.7633720756758
        ],
        "up": [
          0.5077745391032844,
          0.8475747918929692,
          0.15421410307114375
        ],
        "fovYDegrees": 9.823091671032113
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k43",
        "timeMs": 2304,
        "eye": [
          70.74327008390986,
          28.88343363931856,
          -979.4650330481447
        ],
        "lookAt": [
          77.61323090808908,
          25.998629900823268,
          -986.1344817258546
        ],
        "up": [
          0.5110661886433406,
          0.8442815024367208,
          0.16124545100176504
        ],
        "fovYDegrees": 9.457678980306943
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k44",
        "timeMs": 2320,
        "eye": [
          71.07721190736154,
          28.744457246598873,
          -979.787911883122
        ],
        "lookAt": [
          77.9510732395524,
          25.872538009220236,
          -986.4589020854672
        ],
        "up": [
          0.5140936940210882,
          0.8412038247801092,
          0.16758221547368107
        ],
        "fovYDegrees": 9.144959116033405
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k45",
        "timeMs": 2336,
        "eye": [
          71.36016773758242,
          28.626723238504894,
          -980.0615160845913
        ],
        "lookAt": [
          78.23722892507394,
          25.765406992397885,
          -986.733764488485
        ],
        "up": [
          0.516847012474835,
          0.8383773055151093,
          0.1731838886648034
        ],
        "fovYDegrees": 8.887925362295181
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k46",
        "timeMs": 2352,
        "eye": [
          71.58917852387223,
          28.531464253805208,
          -980.2829857382849
        ],
        "lookAt": [
          78.46870492401366,
          25.678337106851682,
          -986.9561995062586
        ],
        "up": [
          0.5193159873778262,
          0.8358369033423642,
          0.17801004540429943
        ],
        "fovYDegrees": 8.689596590336837
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k47",
        "timeMs": 2368,
        "eye": [
          71.76128521553076,
          28.45991293126841,
          -980.4494609299351
        ],
        "lookAt": [
          78.64250730892304,
          25.612429017364835,
          -987.1233367050354
        ],
        "up": [
          0.5214901887492092,
          0.8336166052152221,
          0.18202015972897878
        ],
        "fovYDegrees": 8.553019085783923
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k48",
        "timeMs": 2384,
        "eye": [
          71.87352876185773,
          28.41330190966309,
          -980.5580817452735
        ],
        "lookAt": [
          78.75564103395226,
          25.568783890327044,
          -987.2323043900434
        ],
        "up": [
          0.5233587427467418,
          0.8317490221134048,
          0.18517340684867484
        ],
        "fovYDegrees": 8.481268855720316
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k49",
        "timeMs": 2393,
        "eye": [
          71.90924904812037,
          28.398505556618677,
          -980.5926841818339
        ],
        "lookAt": [
          78.79149411530624,
          25.55443018450596,
          -987.2669585328729
        ],
        "up": [
          0.5242712061761776,
          0.8308651536864394,
          0.18655508238636995
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k50",
        "timeMs": 2448,
        "eye": [
          72.05930020954973,
          28.338608328108993,
          -980.738171021005
        ],
        "lookAt": [
          78.9415452767356,
          25.494532955996274,
          -987.412445372044
        ],
        "up": [
          0.5293166071567816,
          0.8259848844087077,
          0.19383730321165638
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k51",
        "timeMs": 2496,
        "eye": [
          72.17807208732305,
          28.294413475239995,
          -980.8533323559017
        ],
        "lookAt": [
          79.06031715450891,
          25.450338103127276,
          -987.5276067069407
        ],
        "up": [
          0.5333625821775401,
          0.8219710308792564,
          0.1997197544766152
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k52",
        "timeMs": 2544,
        "eye": [
          72.28600676195047,
          28.256975741364,
          -980.9579880371732
        ],
        "lookAt": [
          79.16825182913634,
          25.41290036925128,
          -987.6322623882122
        ],
        "up": [
          0.5370904223675885,
          0.8181915350649862,
          0.20517429212502927
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k53",
        "timeMs": 2608,
        "eye": [
          72.41391356078053,
          28.216295874327717,
          -981.0820122562222
        ],
        "lookAt": [
          79.29615862796639,
          25.372220502214997,
          -987.7562866072612
        ],
        "up": [
          0.5415892325340307,
          0.8135237346743519,
          0.21180235202823902
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k54",
        "timeMs": 2656,
        "eye": [
          72.49847791256346,
          28.19175833575623,
          -981.1640119824359
        ],
        "lookAt": [
          79.38072297974932,
          25.34768296364351,
          -987.8382863334749
        ],
        "up": [
          0.5446261372108174,
          0.8103050843818974,
          0.21630543426291893
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k55",
        "timeMs": 2720,
        "eye": [
          72.59701299979703,
          28.165603407825394,
          -981.2595616823598
        ],
        "lookAt": [
          79.4792580669829,
          25.321528035712674,
          -987.9338360333988
        ],
        "up": [
          0.5482486226566946,
          0.8063925883086343,
          0.22170800904779916
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k56",
        "timeMs": 2784,
        "eye": [
          72.68037044836254,
          28.14534611576656,
          -981.3403967697609
        ],
        "lookAt": [
          79.5626155155484,
          25.30127074365384,
          -988.0146711207999
        ],
        "up": [
          0.5514079905492053,
          0.8029137013201074,
          0.22644826382848016
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k57",
        "timeMs": 2848,
        "eye": [
          72.74971840119079,
          28.129239310975272,
          -981.4076492746486
        ],
        "lookAt": [
          79.63196346837665,
          25.285163938862553,
          -988.0819236256876
        ],
        "up": [
          0.5541293997362149,
          0.7998661598530021,
          0.23055310596475118
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k58",
        "timeMs": 2912,
        "eye": [
          72.80622500121267,
          28.115535844847088,
          -981.4624512270325
        ],
        "lookAt": [
          79.68847006839853,
          25.27146047273437,
          -988.1367255780715
        ],
        "up": [
          0.556436370653504,
          0.7972450615823619,
          0.23404887779370467
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k59",
        "timeMs": 2992,
        "eye": [
          72.86057969160218,
          28.09912453547976,
          -981.5151696550212
        ],
        "lookAt": [
          79.74282475878805,
          25.25504916336704,
          -988.1894440060602
        ],
        "up": [
          0.5587706623722003,
          0.7945570836709349,
          0.23760132083065458
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k60",
        "timeMs": 3072,
        "eye": [
          72.89897652072295,
          28.080326123461013,
          -981.5524138877246
        ],
        "lookAt": [
          79.78122158790882,
          25.236250751348294,
          -988.2266882387636
        ],
        "up": [
          0.5605316009808691,
          0.7925050785790411,
          0.24029154110840473
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k61",
        "timeMs": 3152,
        "eye": [
          72.92369701773676,
          28.055728209172795,
          -981.5763949212547
        ],
        "lookAt": [
          79.80594208492262,
          25.211652837060075,
          -988.2506692722937
        ],
        "up": [
          0.5617560847202621,
          0.7910657323636582,
          0.24216752127341715
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k62",
        "timeMs": 3232,
        "eye": [
          72.93702271180537,
          28.02191839299704,
          -981.589323751724
        ],
        "lookAt": [
          79.81926777899123,
          25.17784302088432,
          -988.263598102763
        ],
        "up": [
          0.5624783549145351,
          0.7902118815173529,
          0.24327614466166506
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k63",
        "timeMs": 3296,
        "eye": [
          72.94101219653648,
          27.985944838154072,
          -981.593195019283
        ],
        "lookAt": [
          79.82325726372234,
          25.141869466041353,
          -988.267469370322
        ],
        "up": [
          0.5627161199864004,
          0.789930010511725,
          0.2436414307961545
        ],
        "fovYDegrees": 8.470699434133696
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k64",
        "timeMs": 3317,
        "eye": [
          72.94124777949334,
          27.97208507987302,
          -981.5934236665857
        ],
        "lookAt": [
          79.82349284840319,
          25.128009713550743,
          -988.2676980183145
        ],
        "up": [
          0.5627311942284289,
          0.7899121264417509,
          0.24366459640768
        ],
        "fovYDegrees": 8.505731890479693
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k65",
        "timeMs": 3344,
        "eye": [
          72.92529236451087,
          27.95246837990329,
          -981.5801061089753
        ],
        "lookAt": [
          79.80776000502217,
          25.10914083667041,
          -988.2544695736967
        ],
        "up": [
          0.5619017417126314,
          0.7908571175918792,
          0.24251072597841108
        ],
        "fovYDegrees": 8.638325528721339
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k66",
        "timeMs": 3360,
        "eye": [
          72.90134074623442,
          27.939859449190546,
          -981.5601106463271
        ],
        "lookAt": [
          79.78414195952124,
          25.09765325577956,
          -988.2346077369335
        ],
        "up": [
          0.560654845840354,
          0.7922697750489516,
          0.2407794579685168
        ],
        "fovYDegrees": 8.76112776210136
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k67",
        "timeMs": 3376,
        "eye": [
          72.86708820398418,
          27.926562863600907,
          -981.5315107804428
        ],
        "lookAt": [
          79.7503656723044,
          25.085958638693082,
          -988.2061987080954
        ],
        "up": [
          0.5588669609734701,
          0.7942788900016452,
          0.23830393372756614
        ],
        "fovYDegrees": 8.915313348773369
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k68",
        "timeMs": 3392,
        "eye": [
          72.82293111352247,
          27.912615060528825,
          -981.4946343022035
        ],
        "lookAt": [
          79.70682144574603,
          25.074073849990278,
          -988.1695678367035
        ],
        "up": [
          0.5565531719001258,
          0.7968505871419017,
          0.23511211925274236
        ],
        "fovYDegrees": 9.099645932786876
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k69",
        "timeMs": 3408,
        "eye": [
          72.76926585061153,
          27.89805247736876,
          -981.4498090024905
        ],
        "lookAt": [
          79.65389954721589,
          25.062015769406386,
          -988.1250404332852
        ],
        "up": [
          0.55372711449931,
          0.7999489174414748,
          0.23123151202308195
        ],
        "fovYDegrees": 9.312896196361075
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k70",
        "timeMs": 3424,
        "eye": [
          72.70648879101367,
          27.882911551515168,
          -981.3973626721847
        ],
        "lookAt": [
          79.59199021531626,
          25.049801289036832,
          -988.0729417763948
        ],
        "up": [
          0.5504012823079886,
          0.8035361846344987,
          0.22668927724256077
        ],
        "fovYDegrees": 9.553841408611804
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k71",
        "timeMs": 3440,
        "eye": [
          72.63499631049116,
          27.867228720362508,
          -981.3376231021673
        ],
        "lookAt": [
          79.52148366584586,
          25.03744731065938,
          -988.0135971193305
        ],
        "up": [
          0.5465873226749063,
          0.80757325599358,
          0.2215123809068154
        ],
        "fovYDegrees": 9.821265000543209
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k72",
        "timeMs": 3456,
        "eye": [
          72.55518478480631,
          27.851040421305235,
          -981.2709180833195
        ],
        "lookAt": [
          79.44277009749266,
          25.024970743177644,
          -987.947331696543
        ],
        "up": [
          0.5422963205823776,
          0.8120198597547355,
          0.21572771738169186
        ],
        "fovYDegrees": 10.113956139877288
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k73",
        "timeMs": 3472,
        "eye": [
          72.4674505897214,
          27.83438309173781,
          -981.1975754065226
        ],
        "lookAt": [
          79.35623969725641,
          25.01238850018889,
          -987.8744707297379
        ],
        "up": [
          0.5375390684492544,
          0.8168348711645138,
          0.209362229497968
        ],
        "fovYDegrees": 10.430709283228868
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k74",
        "timeMs": 3488,
        "eye": [
          72.3721901009987,
          27.817293169054683,
          -981.1179228626575
        ],
        "lookAt": [
          79.26228264560287,
          24.99971749767864,
          -987.795339433672
        ],
        "up": [
          0.5323263204372075,
          0.8219765887418653,
          0.20244301946491075
        ],
        "fovYDegrees": 10.770323686840529
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k75",
        "timeMs": 3520,
        "eye": [
          72.1606757456891,
          27.781961293919167,
          -980.9409993372473
        ],
        "lookAt": [
          79.05364930628599,
          24.974176877047853,
          -987.6195667106655
        ],
        "up": [
          0.5205785693941817,
          0.8330720513052248,
          0.18705322884572764
        ],
        "fovYDegrees": 11.51335395316791
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k76",
        "timeMs": 3536,
        "eye": [
          72.04521463062677,
          27.7637922162557,
          -980.8443839374646
        ],
        "lookAt": [
          78.9397533895244,
          24.9613410839036,
          -987.5235757263592
        ],
        "up": [
          0.5140669307508184,
          0.8389418809345134,
          0.1786384928353736
        ],
        "fovYDegrees": 11.914387059712944
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k77",
        "timeMs": 3568,
        "eye": [
          71.79686640449852,
          27.726629967709602,
          -980.6364848181495
        ],
        "lookAt": [
          78.69475406823123,
          24.93562305572053,
          -987.3170107103044
        ],
        "up": [
          0.49983225338726267,
          0.8511189443717367,
          0.1605124948574166
        ],
        "fovYDegrees": 12.76954968226006
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k78",
        "timeMs": 3584,
        "eye": [
          71.66477204495713,
          27.707709671615895,
          -980.5258566803795
        ],
        "lookAt": [
          78.5644311139935,
          24.92277460778957,
          -987.2070872123501
        ],
        "up": [
          0.4921378272940758,
          0.85734566171707,
          0.1508601182592198
        ],
        "fovYDegrees": 13.221306697358882
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k79",
        "timeMs": 3616,
        "eye": [
          71.38672471173135,
          27.66937292275945,
          -980.2928822030201
        ],
        "lookAt": [
          78.29008990442861,
          24.897183238628287,
          -986.9755847529958
        ],
        "up": [
          0.4756752977084593,
          0.8698824029152289,
          0.1305282201226847
        ],
        "fovYDegrees": 14.16723723683983
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k80",
        "timeMs": 3664,
        "eye": [
          70.94095281296933,
          27.611173968719058,
          -979.91904184565
        ],
        "lookAt": [
          77.85019449546972,
          24.85931282602763,
          -986.6040728571072
        ],
        "up": [
          0.4485796265175541,
          0.8883608677441583,
          0.09793511798235388
        ],
        "fovYDegrees": 15.674293252715925
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k81",
        "timeMs": 3712,
        "eye": [
          70.47025285379418,
          27.55301691523878,
          -979.5238140998298
        ],
        "lookAt": [
          77.38560795064262,
          24.822466143681428,
          -986.2112617720318
        ],
        "up": [
          0.41913296521474863,
          0.9056936014090717,
          0.0636133463744687
        ],
        "fovYDegrees": 17.258242252597263
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k82",
        "timeMs": 3824,
        "eye": [
          69.33512748250335,
          27.423006408566785,
          -978.5681577992494
        ],
        "lookAt": [
          76.26478065476255,
          24.743014254682095,
          -985.2612549124934
        ],
        "up": [
          0.34546829840261584,
          0.9382575707591799,
          -0.01801071103216248
        ],
        "fovYDegrees": 21.064942600306903
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k83",
        "timeMs": 3872,
        "eye": [
          68.85862726226352,
          27.372094315539368,
          -978.1655579804179
        ],
        "lookAt": [
          75.79405228373004,
          24.712844683169475,
          -984.8609497661431
        ],
        "up": [
          0.31389816577771296,
          0.9480644973423141,
          -0.051396988243015425
        ],
        "fovYDegrees": 22.65939214633982
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k84",
        "timeMs": 3920,
        "eye": [
          68.40357494579804,
          27.32548729822356,
          -977.7799223062298
        ],
        "lookAt": [
          75.34434219091439,
          24.685652631011777,
          -984.4774608823715
        ],
        "up": [
          0.2837476753145238,
          0.9553437075738647,
          -0.08249640690736576
        ],
        "fovYDegrees": 24.177991344515753
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k85",
        "timeMs": 3952,
        "eye": [
          68.11740724708172,
          27.297293494001124,
          -977.5366269266548
        ],
        "lookAt": [
          75.06142514057292,
          24.669399902983123,
          -984.2354918132726
        ],
        "up": [
          0.26494834113247867,
          0.9589032445832113,
          -0.1015231208096836
        ],
        "fovYDegrees": 25.128843409924816
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k86",
        "timeMs": 3984,
        "eye": [
          67.84869946528859,
          27.271741804650766,
          -977.3074269845439
        ],
        "lookAt": [
          74.79566946695398,
          24.654801970848794,
          -984.0075190512413
        ],
        "up": [
          0.24755260146027414,
          0.9615521483366598,
          -0.11889144434905712
        ],
        "fovYDegrees": 26.016378978067305
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k87",
        "timeMs": 4000,
        "eye": [
          67.72188398264394,
          27.260047846538765,
          -977.1989322797405
        ],
        "lookAt": [
          74.67020483006107,
          24.648164912864353,
          -983.8995967947068
        ],
        "up": [
          0.2394781387233665,
          0.962575859325531,
          -0.1268776423067034
        ],
        "fovYDegrees": 26.43253593497412
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k88",
        "timeMs": 4032,
        "eye": [
          67.48531171266991,
          27.23900589041339,
          -976.9957923570431
        ],
        "lookAt": [
          74.43605744074881,
          24.636297291347883,
          -983.6975116716626
        ],
        "up": [
          0.22476859635146323,
          0.9641118726363521,
          -0.14130596284594396
        ],
        "fovYDegrees": 27.201752880531846
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k89",
        "timeMs": 4064,
        "eye": [
          67.27412687486479,
          27.22133479704925,
          -976.8133036894328
        ],
        "lookAt": [
          74.22689327918297,
          24.626415213154008,
          -983.5159473231121
        ],
        "up": [
          0.21223165048268533,
          0.9650911486317014,
          -0.15348225098081747
        ],
        "fovYDegrees": 27.876198824127975
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k90",
        "timeMs": 4080,
        "eye": [
          67.1790456824313,
          27.21385441738878,
          -976.7306230534882
        ],
        "lookAt": [
          74.13265767128505,
          24.622260088537995,
          -983.4336760253167
        ],
        "up": [
          0.20687157040983922,
          0.9654185351054367,
          -0.158654358373918
        ],
        "fovYDegrees": 28.173863399070605
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k91",
        "timeMs": 4096,
        "eye": [
          67.0915004753269,
          27.207326065601993,
          -976.6540886039588
        ],
        "lookAt": [
          74.04584116808672,
          24.61865096030744,
          -983.3575136037819
        ],
        "up": [
          0.20216620587287892,
          0.9656613280125035,
          -0.16317789306181402
        ],
        "fovYDegrees": 28.443006068725843
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k92",
        "timeMs": 4112,
        "eye": [
          67.01188762931386,
          27.20178617908334,
          -976.5840281317259
        ],
        "lookAt": [
          73.96683484139369,
          24.61560437692928,
          -983.2877862541069
        ],
        "up": [
          0.1981518150451173,
          0.9658357498179132,
          -0.16702443703840136
        ],
        "fovYDegrees": 28.682017347283967
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k93",
        "timeMs": 4128,
        "eye": [
          66.9406035201545,
          27.197271195227277,
          -976.5207694276702
        ],
        "lookAt": [
          73.89602973981619,
          24.613136896253156,
          -983.2248201455353
        ],
        "up": [
          0.19486415226625525,
          0.9659563582666044,
          -0.1701654374009991
        ],
        "fovYDegrees": 28.889294874505747
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k94",
        "timeMs": 4160,
        "eye": [
          66.82190516764621,
          27.19119014760128,
          -976.4137299039688
        ],
        "lookAt": [
          73.77791673354024,
          24.60985871821987,
          -983.1182530896127
        ],
        "up": [
          0.1904075141221843,
          0.9660885411139173,
          -0.1744073086031474
        ],
        "fovYDegrees": 29.202301494041098
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k95",
        "timeMs": 4192,
        "eye": [
          66.71142163208704,
          27.186078873430425,
          -976.313622377656
        ],
        "lookAt": [
          73.66789483084594,
          24.60714827537112,
          -983.0185905062411
        ],
        "up": [
          0.18661738922665733,
          0.9661725928258303,
          -0.1780013227772156
        ],
        "fovYDegrees": 29.369521810188793
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k96",
        "timeMs": 4210,
        "eye": [
          66.67136886871765,
          27.18478020933878,
          -976.2763220130653
        ],
        "lookAt": [
          73.62795980154361,
          24.606462298548674,
          -982.9814036191475
        ],
        "up": [
          0.18564991867206732,
          0.9661898194972036,
          -0.1789171327654681
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k97",
        "timeMs": 4288,
        "eye": [
          66.5699147169714,
          27.191763951507188,
          -976.1755800475133
        ],
        "lookAt": [
          73.52527504304985,
          24.613452929885597,
          -982.8819408348703
        ],
        "up": [
          0.18560616570372918,
          0.9661900034454125,
          -0.1789615279743484
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k98",
        "timeMs": 4400,
        "eye": [
          66.4164318169742,
          27.22061530206961,
          -976.0226860455379
        ],
        "lookAt": [
          73.36671089973369,
          24.64230428044802,
          -982.7343127445982
        ],
        "up": [
          0.18547057099041114,
          0.9661900034454127,
          -0.17910205062657591
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k99",
        "timeMs": 4464,
        "eye": [
          66.33341326319933,
          27.242041835783752,
          -975.9398434260462
        ],
        "lookAt": [
          73.27991616682299,
          24.66373081416216,
          -982.6553783776143
        ],
        "up": [
          0.1853698023605263,
          0.9661900034454127,
          -0.17920634367949456
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k100",
        "timeMs": 4528,
        "eye": [
          66.25996951838408,
          27.26361389070298,
          -975.8664912206519
        ],
        "lookAt": [
          73.20266837633109,
          24.68530286908139,
          -982.5859588218572
        ],
        "up": [
          0.1852682901024833,
          0.9661900034454127,
          -0.17931128777814231
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k101",
        "timeMs": 4576,
        "eye": [
          66.21413796878599,
          27.278229445575718,
          -975.8206885290692
        ],
        "lookAt": [
          73.154258234496,
          24.699918423954127,
          -982.5428193648434
        ],
        "up": [
          0.18519947948799523,
          0.9661900034454125,
          -0.17938235710216688
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k102",
        "timeMs": 4624,
        "eye": [
          66.17854863390083,
          27.290212154041278,
          -975.7851060870349
        ],
        "lookAt": [
          73.11655405287863,
          24.711901132419687,
          -982.5094196640492
        ],
        "up": [
          0.1851430440806815,
          0.9661900034454126,
          -0.17944060429761294
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k103",
        "timeMs": 4672,
        "eye": [
          66.15528282144567,
          27.298398572348933,
          -975.7618361338144
        ],
        "lookAt": [
          73.091843012084,
          24.720087550727342,
          -982.4876405464803
        ],
        "up": [
          0.1851044776688668,
          0.9661900034454127,
          -0.1794803877561269
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.camera2.k104",
        "timeMs": 4723,
        "eye": [
          66.14639367743558,
          27.3016357421875,
          -975.7529427278397
        ],
        "lookAt": [
          73.08238229049314,
          24.72332472056591,
          -982.4793365785832
        ],
        "up": [
          0.18508922492591803,
          0.9661900034454126,
          -0.17949611711253421
        ],
        "fovYDegrees": 29.39495760413827
      }
    ]
  }
}
```

### cutscenes / cutscene.maharaka.waterpang.source.intro15

```json
{
  "cutsceneId": "cutscene.maharaka.waterpang.source.intro15",
  "displayName": "워터팡 / 도입 15 · 원본 카메라·경기 배치",
  "durationMs": 9507,
  "cameraCuts": [
    {
      "cutId": "maharaka.waterpang.source.intro15.cut.0",
      "shotId": "maharaka.waterpang.source.intro15.camera.1",
      "startMs": 0
    },
    {
      "cutId": "maharaka.waterpang.source.intro15.cut.1",
      "shotId": "maharaka.waterpang.source.intro15.camera.2",
      "startMs": 4784
    }
  ],
  "worldInstanceIds": [
    "world.sequence.instance.maharaka.waterpang.source.intro15.match-stand",
    "world.sequence.instance.maharaka.waterpang.source.intro15.stage",
    "world.sequence.instance.maharaka.waterpang.source.intro15.mokomoko",
    "world.sequence.instance.maharaka.waterpang.source.intro15.cannon"
  ]
}
```

## 저장·게시 JSON 변경 행 · Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.worldsequences.json

revision: 4 → 5

### templates / sequence.maharaka.waterpang.source.intro15.mokomoko

```json
{
  "sequenceId": "sequence.maharaka.waterpang.source.intro15.mokomoko",
  "displayName": "워터팡 / mokomoko 표정",
  "category": "World",
  "durationMs": 9507,
  "interpolation": "LINEAR",
  "tracks": [
    {
      "slotId": "actor",
      "keys": [
        {
          "timeMs": 0,
          "positionOffset": [
            81.8360400390625,
            24.3577745710577,
            -991.514375
          ],
          "rotationQuaternion": [
            0.0,
            0.9209172415291894,
            0.0,
            -0.3897581740698565
          ],
          "scaleMultiplier": [
            1.0,
            1.0,
            1.0
          ],
          "visible": true
        },
        {
          "timeMs": 9507,
          "positionOffset": [
            81.8360400390625,
            24.3577745710577,
            -991.514375
          ],
          "rotationQuaternion": [
            0.0,
            0.9209172415291894,
            0.0,
            -0.3897581740698565
          ],
          "scaleMultiplier": [
            1.0,
            1.0,
            1.0
          ],
          "visible": true
        }
      ]
    }
  ],
  "animationTracks": [
    {
      "slotId": "actor",
      "clipName": "idle_normal_1",
      "startMs": 0,
      "playbackRate": 1,
      "loop": true,
      "holdLastFrame": true
    }
  ],
  "materialTracks": [
    {
      "slotId": "actor",
      "materialName": "mn_ismp_00-2_mi",
      "curves": [
        {
          "parameter": "opacity_intensity",
          "keys": [
            {
              "timeMs": 0,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 6501,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 6602,
              "value": [
                1.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 9502,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 9507,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            }
          ]
        }
      ]
    }
  ]
}
```

### templates / sequence.maharaka.waterpang.source.intro15.match-stand

```json
{
  "sequenceId": "sequence.maharaka.waterpang.source.intro15.match-stand",
  "displayName": "워터팡 / 경기 중 타워 배치",
  "category": "World",
  "durationMs": 9507,
  "interpolation": "LINEAR",
  "tracks": [
    {
      "slotId": "stand",
      "keys": [
        {
          "timeMs": 0,
          "positionOffset": [
            15.747189798051751,
            -0.8413069489423002,
            -6.656925129515184
          ],
          "rotationQuaternion": [
            0.0,
            0.3755861784892171,
            0.0,
            -0.9267874743045819
          ],
          "scaleMultiplier": [
            1,
            1,
            1
          ],
          "visible": true
        },
        {
          "timeMs": 9507,
          "positionOffset": [
            15.747189798051751,
            -0.8413069489423002,
            -6.656925129515184
          ],
          "rotationQuaternion": [
            0.0,
            0.3755861784892171,
            0.0,
            -0.9267874743045819
          ],
          "scaleMultiplier": [
            1,
            1,
            1
          ],
          "visible": true
        }
      ]
    }
  ],
  "animationTracks": []
}
```

### instances / world.sequence.instance.maharaka.waterpang.source.intro15.match-stand

```json
{
  "instanceId": "world.sequence.instance.maharaka.waterpang.source.intro15.match-stand",
  "templateId": "sequence.maharaka.waterpang.source.intro15.match-stand",
  "enabled": true,
  "startDelayMs": 0,
  "playbackSpeed": 1,
  "bindings": [
    {
      "slotId": "stand",
      "targetKind": "MAP_PLACEMENT",
      "targetId": "14892776262101167312"
    }
  ]
}
```
