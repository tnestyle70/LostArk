# G01 — 마하라카 워터팡 원본 Camera·맵 발판 편집 등록

## G05 — 아레나 입장·카운트다운·제품 컷신과 배우 유지

전체 구현 전문: [G05 코드 부록](2026-09-28_MAHARAKA_WATERPANG_ENTRY_CODE.md).
추가 조사에서 기본 navigation이 전 구역 20.48m인 것을 확인했다. 기존 무대 18타일과 중앙 원판,
상단 통 2개의 정확한 placement/model geometry만 WaterpangEntry 0.25m region으로 굽는다.
나머지는 기존 평면을 보존하며 dynamic collapse/낙사나 섬 전체 bake를 이 작업에 추가하지 않는다.
사용자 jump 목적지 Y를 고치지 않고, 다음 이동의 ground query가 실제 22.4m 무대에 머무르게 한다.
Client의 기존 Camera sampler 한도와 동일한 512키를 수용하고 카메라/World actor는 같은 서버 시계를 쓴다.
MapTool 소유권을 넘기기 전에 제품 배우를 반환하며 닫은 뒤 예약의 현재 시각으로 재구성한다.
NPC 준비 대기는 5초로 제한하고 실패 사유를 diagnostic에 남긴다.

사용자가 저장한 jump1~3 위치와 jump1_1~3_1 도착점을 보존하여 출발점만 requiresInteract movePlayer로 활성화한다.
별도 아레나 위 triggerBox는 기존 PLAY_SEQUENCE 경로를 사용한다. 서버가 도입15 stage instance의
최초 진입만 받아 30Hz 기준 300tick 뒤 시작 시각을 확정한다. 기존 S2C_WORLD_SEQUENCE_PLAY의
iStartTick/iServerTick으로 예약을 전달하고 늦은 입장에도 같은 예약을 전송한다. 빈 방에서만 초기화한다.
카운트다운 중 플레이어가 나가더라도 방이 남아 있으면 예약은 유지한다. 재진입으로 타이머를 재시작하지 않는다.

Client Level_Development의 마하라카 분기는 기존 replication queue를 소비한다.
새 MaharakaWaterpangPresentation은 게시 CameraShots의 도입15와 연결된 WorldSequence를 읽고,
기존 CValtanCinematicCameraController::Sample_Cue / CWorldSequencePlayer로만 재생한다.
카메라만 종료하고 HOLD 배우/타워는 맵 퇴장까지 유지한다. G키 안내와 countdown 문구는
기존 CInteractKeyPromptView / UILabelFont를 사용한다. UI가 서버 이동·시작을 판정하지 않는다.
MapTool preview도 동일 배우 HOLD 문서를 사용하되 명시적인 Stop은 기존 rollback 의미를 유지한다.
previewNpcPlacementId의 제한을 STOP/HOLD로 확장하고 서버 승인 Level owner도 resolver를 제공한다.

새 Client C++ 파일은 Public/Private에 각각 추가하고 Client.vcxproj 및 filters에 등록한다.
검증은 출발/도착 좌표 보존, 반복 진입 latch/늦은 입장, HOLD/Stop 소유권, 잘못된 문서 실패,
World/Map publisher와 Product Debug 빌드다. 화면과 실제 G키·countdown 재생은 사용자 확인이다.

## G04 — 평상시 배치 보존 / 경기 도입 배치 분리

G03의 카메라 retarget은 평상시 섬57009의 배우 위치를 경기57011의 카메라에 맞춘 잘못된 기준이었다.
도입15 두 cameraTrack은 G03 이전 원본 키로 복귀한다. NPC와 타워는 해당 컷신의 WorldSequence에서만 이동한다.
permanent Gameplay.world.json과 mapplacements, 팀 렌더링 옵션은 수정하지 않는다.

`Tools/MapPipeline/stage_maharaka_waterpang_match_layout.py`의 prepare는 최신 저작 문서를 읽고,
57011 DeployData actor22/NPC570941의 정확한 XZ/yaw와 설치된 모델의 smile 면을 확인한다.
현재 head/support 접촉을 유지한 rigid group 이동을 계산하며, 높이는 원본 Z라고 주장하지 않는
PROJECT_FRAME_CONTACT_ADAPTER다. Prop570987의 모델 연결은 미확정이며 기존 SCENE04A export612
ITR_02453 원본 타워를 사용자 요청대로 함께 옮긴다. main은 before/candidate/report를 보존하고
명시적인 --apply에서 source freshness와 CAS를 검사한 뒤 두 저작 문서만 교체한다.

새 tower instance는 기존 MAP_PLACEMENT baseline-relative TRS 경로를 소비한다.
Stop_Instance의 기존 baseline 복원과 NPC preview suppression 해제로 평상시 상태로 돌아간다.
새 C++ 및 project/filter 등록은 없으며 G03 최종 Debug EXE를 사용한다.
서버의 전체 경기 시작/종료 상태머신 구현을 완료한 변경은 아니다. 우선 MapTool 경기 도입 편집 재생이다.

전체 코드와 실제 JSON 변경 블록: [G04 코드 부록](2026-09-28_MAHARAKA_WATERPANG_MATCH_CODE.md).
후보 보존/상대좌표 검사 → CameraShots/WorldSequences 정본 publisher 검증·게시·Check → 사용자 Play/Stop 순서다.

## G03 — 2026-09-28 사용자 영상 대조 수정

기존 G01/G02 기록을 지우지 않고 도입15의 카메라와 배우 표현을 확장한다.
원본 카메라 키의 초점은 현재 배치된 큰 모코모코와 일치하지 않는다. 외부 rebase의 원인은
확인되지 않았으므로 좌표 변환 버그로 단정하지 않는다. 카메라 -45도 heading, 실제 얼굴 위치
기준 close-up 및 작은 모코모코 rise/hold 배치는 PROJECT_VIDEO_RETARGET으로 분리한다.
원본 Matinee43 / group205 / track286 / MIC mn_ismp_00-2_mi의 opacity_intensity 전환은
6501ms=0, 6602ms=1, 9502ms=0으로 기존 materialTracks에서 재생한다.

### 소유권과 적용 순서

1. `retarget_maharaka_waterpang_intro.py`가 원본 SHA와 설치 모델 clip을 검사하고 out 후보를 만든다.
2. `WorldSequenceDocument`의 binding에 `previewNpcPlacementId`를 추가한다. Save/Load/equality/
   validation/publisher를 함께 확장한다. 모델 import는 NPC와 같은 .01 및 yaw-90도다.
3. `ClientReplication::Find_NpcPlacement`는 정확한 stable ID의 기존 NPC만 조회한다.
   `Level_Development`와 `MapTool_Area`가 이 조회를 기존 WorldSequence TARGET_SET에 전달한다.
   `MapTool_Cutscenes::Build_CutsceneTargets`도 이 callback을 전달한다. Camera Play는 별도
   TARGET_SET을 구성하므로 World panel 연결만으로 이 소비자를 연결한 것으로 간주하지 않는다.
4. `WorldSequencePlayer_Objects::Apply_Objects`는 성공적으로 보이는 clone에만 NPC render
   suppression 토큰을 둔다. Release_Objects와 매 샘플 hide에서 토큰을 해제한다.
5. 후보 publisher 검증 및 Debug Product 증분 빌드 후 저장 승인을 확인한다. 승인 시 최신 파일을
   다시 읽고 stable ID/변경 field 단위로 병합한다. 같은 field 충돌은 거부하고 CAS/backup을 유지한다.
6. WorldSequences와 CameraShots를 정본 publisher로 게시한다. 사용자 직접 Play/seek/Stop으로
   얼굴 초점·표정·상승·복귀를 확인한다. 영상 fidelity는 자동 PASS로 처리하지 않는다.

새 C++ 파일과 project/filter 등록은 없다. 기존 모델·재질·클립만 사용하며 전역 rendering 옵션,
NPC gameplay 위치, intro20, 기존 발판 동작과 Foley는 유지한다. G03 전체 코드 스냅샷은
같은 폴더의 `2026-09-28_MAHARAKA_WATERPANG_VIDEO_CODE.md`에 기계적으로 보존한다.

## 현재 반영 경계

사용자가 제공한 2021 워터팡 영상과 설치본 SCENE03B를 대조한다.
이 G는 기존 MapTool Camera가 소비하는 카메라 2개와 발판 동작 4개를 등록한다.
경기 규칙·서버 낙사·물 분사·배우·사운드 연결 완료를 이 G의 결과로 주장하지 않는다.
맵·재질·조명·NPC·기존 배치 transform은 수정하지 않는다.

## 데이터 정본과 소비자

- 원본: LV_OCN_EVENTIS_MHP_SCENE03B, source audit의 실제 UPK와 SHA256을 확인한다.
- Matinee/Data: 41/156 흔들림, 42/157 붕괴, 44/159 붕괴 유지, 46/161 복구,
  43/158·45/160 도입 카메라. packageIndex는 1-based이고 배치 export ID는 0-based component이다.
- 원본 variableLinks → InterpGroup → actor.staticmeshcomponent → 기존 sourcePlacementId → stable placementId.
  이름 유사도·근접 좌표로 조인하지 않는다.
- WorldSequencePlayer: baseline position + baseline rotation × local offset,
  baseline rotation × local quaternion; baseline scale 유지.
- MapTool_Cutscenes: cutscenes.cameraCuts와 worldInstanceIds로 재생한다.
  동일 18개 발판은 동작당 독립 컷신으로 등록하여 동시 소유를 피한다.
- 원본 복구 Euler +180/-180 곡선과 inherited quaternion 설정은 구분한다.
  원본 키를 임의 unwrap하거나 “회전 없음”으로 바꾸지 않는다. 시각 검증 전 완료로 표시하지 않는다.
- 원본 붕괴 InterpLength 생략값은 설치본 Engine.u의 Default__InterpData.InterpLength=5.0을 따른다.
  마지막 movement key(약 3.644초) 이후에는 5초까지 마지막 자세를 유지한다.

## 파일·함수 역할

build_maharaka_waterpang_sequences.py는 원본을 읽고 기존 배치의 역변환으로 키를 만든다.
groups_for는 Matinee/Data 연결을 검증한다. sampled_keys는 원본 Hermite 곡선을
밀리초 키와 quaternion으로 투영하고 중간 샘플 오차를 검사한다.
build_motion은 정확히 18개 원본 component만 기존 배치에 묶는다.
consumed_camera_tracks는 실제 소비한 카메라/부모 Move/FOV/Director만 완료 목록에서 제외한다.
prepare는 기존 stable ID를 보존하고 수정된 동일 ID를 거부한다.
main은 out 후보·백업 후 --apply에서만 기존 원자적 transaction을 호출한다.
test 스크립트는 원본 대비 역변환, ID 누락·중복, 동시 편집, 중간 실패 rollback을 검사한다.

## builder 전체 코드

```python
"""Project original Waterpang camera/tile curves into the existing MapTool.

Prepare is read-only outside out/. --apply adds only reviewed authoring IDs;
it never runs the Client, publishes runtime files, or modifies map placements.
Non-transform source tracks are inventoried, NOT silently called implemented.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import shlex
import sys
from functools import lru_cache
from pathlib import Path

import numpy as np
from scipy.spatial.transform import Rotation

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/KoukuSaydonPipeline'))
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
import build_gate2_intro_composition as source
from build_source_sequences import project_registration
from source_character_registration import commit_staged_files

AREA = 'LV_OCN_EVENTIS_MHP'
SCENE = AREA + '_SCENE03B'
AUTHORING = ROOT / 'Data/Maps/Authoring' / AREA
AUDIT = ROOT / 'out/MaharakaMapRestoration20260927/full-source-audit' / (SCENE + '.objects.json')
OUT = ROOT / 'out/MaharakaWaterpang20260927'
PREFIX = 'maharaka.waterpang.source'
MOTIONS = ((41, 156, 'shake', '흔들림'),
           (42, 157, 'collapse', '붕괴'),
           (44, 159, 'collapsed', '붕괴 상태'),
           (46, 161, 'repair', '복구'))
CAMERAS = ((43, 158, 'intro15'), (45, 160, 'intro20'))


@lru_cache(maxsize=1)
def interp_defaults():
    path = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/NE1FENCQ4UNE9ZPRENOQS.u')
    package = source.load_package(path, source.ue3.LOSTARK_KR_AES_KEY)
    entries = [e for e in package.exports if source.ue3.package_ref_path(
        e.index+1, package.imports, package.exports).casefold() == 'default__interpdata']
    require(len(entries) == 1, 'Requires installed Engine InterpData CDO')
    entry = entries[0]
    props, _ = source.ue3.parse_tagged_properties(package.logical[entry.serial_offset:entry.serial_offset+entry.serial_size],
                                                  package.names, package.summary.version)
    props = {k.casefold(): v for k,v in source.unwrap(props).items()}
    require(isinstance(props.get('interplength'), (int, float)) and props['interplength'] > 0, 'Missing CDO InterpLength')
    return dict(physicalPackage=str(path), sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                objectPath='Default__InterpData', packageIndex=entry.index+1, interpLength=props['interplength'])


def encoded(document):
    return (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf-8')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def load_json(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def placements(raw):
    lines = raw.decode('utf-8-sig').splitlines()
    header = shlex.split(lines[0])
    require(header[:3] == ['LOSTARK_MAP_PLACEMENTS', '2', AREA], 'Placement header changed')
    result = {}
    for line in lines[1:]:
        fields = shlex.split(line)
        if not fields:
            continue
        require(len(fields) == 16, 'Unexpected placement row')
        if fields[1].startswith(SCENE + ':export:'):
            require(fields[1] not in result, 'Duplicate source component placement')
            result[fields[1]] = dict(id=fields[0], asset=fields[4],
                p=np.array(fields[5:8], dtype=float),
                r=Rotation.from_quat(np.array(fields[8:12], dtype=float)).as_matrix(),
                scale=np.array(fields[12:15], dtype=float), visible=fields[15])
    return result


def groups_for(rows, matinee, data):
    links = [v for v in rows[matinee]['p']['variablelinks'] if v['linkdesc'].casefold() == 'data']
    require(len(links) == 1 and links[0]['linkedvariables'] == [data], 'Matinee/Data identity mismatch')
    return rows[data]['p']['interpgroups']


def consumed_camera_tracks(rows, matinee, data):
    """Only Director, selected camera FOV/Move and animated parent Move are consumed."""
    groups = groups_for(rows, matinee, data)
    by_name = {rows[g]['p'].get('groupname'): g for g in groups}
    consumed = set()
    selected = set()
    for group in groups:
        for track in source.active_tracks(rows, group):
            if track['cls'] == 'interptrackdirector':
                consumed.add(track['index'])
                selected.update(by_name[c['targetcamgroup']] for c in track['p'].get('cuttrack', [])
                                if c['targetcamgroup'] in by_name)
    for group in selected:
        consumed.update(t['index'] for t in source.active_tracks(rows, group)
                        if t['cls'] == 'interptrackfloatprop' and t['p'].get('propertyname', '').casefold() == 'fovangle')
    pending = list(selected)
    visited = set()
    while pending:
        group = pending.pop()
        if group in visited:
            continue
        visited.add(group)
        consumed.update(t['index'] for t in source.active_tracks(rows, group) if t['cls'] == 'interptrackmove')
        for actor in source.group_actor(rows, group, matinee):
            parent = rows[actor]['p'].get('base')
            if parent in rows:
                pending.extend(g for g in groups if parent in source.group_actor(rows, g, matinee))
    return consumed


def angle(a, b):
    return math.degrees(2 * math.acos(min(1., abs(float(np.dot(a, b))))))


def sampled_keys(rows, group, actor, matinee, data, duration, baseline):
    tracks = source.active_tracks(rows, group)
    require(len(tracks) == 1 and tracks[0]['cls'] == 'interptrackmove', 'Tile has unsupported active tracks')
    move = tracks[0]['p']
    require(not move.get('busequatinterpolation', False), 'Quaternion source Move needs a different sampler')
    require(move.get('rotmode', 'imr_keyframed') == 'imr_keyframed', 'Unsupported tile look-at')
    times = {0, duration} | set(range(0, duration, 16))
    for field in ('postrack', 'eulertrack'):
        for point in move.get(field, {}).get('points', []):
            require(point.get('interpmode', 'cim_linear') in
                    ('cim_linear', 'cim_constant', 'cim_curveauto', 'cim_curveautoclamped',
                     'cim_curveuser', 'cim_curvebreak'), 'Unknown UE interpolation mode')
            # Keep adjacent millisecond samples at fractional authored boundaries.
            for ms in (math.floor(point['inval'] * 1000), math.ceil(point['inval'] * 1000)):
                times.update(t for t in (ms - 1, ms, ms + 1) if 0 <= t <= duration)
    cache = {}
    require(len(times) <= 4096, 'Initial tile curve exceeds runtime key limit')

    def pose(ms):
        if ms not in cache:
            p, r = source.world_pose(rows, group, actor, ms / 1000., matinee, data)
            cache[ms] = (baseline['r'].T @ (p - baseline['p']),
                         Rotation.from_matrix(baseline['r'].T @ r).as_quat())
        return cache[ms]

    # Refine against quarter/midpoint source samples, not only authored endpoints.
    while True:
        additions = set()
        ordered = sorted(times)
        for left, right in zip(ordered, ordered[1:]):
            a, b = pose(left), pose(right)
            for ms in {round(left + (right-left)*u) for u in (.25, .5, .75)} - {left, right}:
                t = (ms-left)/(right-left)
                p, q = pose(ms)
                if np.linalg.norm(p - (a[0]*(1-t)+b[0]*t)) > .001 or angle(q, source.slerp(a[1], b[1], t)) > .05:
                    additions.add(ms)
        if not additions:
            break
        times.update(additions)
        require(len(times) <= 4096, 'Tile curve exceeds runtime key limit')
    keys = []
    previous = None
    for ms in sorted(times):
        p, q = pose(ms)
        if previous is not None and np.dot(previous, q) < 0:
            q = -q
        previous = q
        keys.append(dict(timeMs=ms, positionOffset=p.tolist(), rotationQuaternion=q.tolist(),
                         scaleMultiplier=[1., 1., 1.], visible=True))
    return keys


def build_motion(rows, live, matinee, data, tag, name):
    groups = groups_for(rows, matinee, data)
    tile_groups = [g for g in groups if rows[g]['p'].get('groupname', '').startswith('t')
                   and rows[g]['p'].get('groupname', '')[1:].isdigit()]
    require(len(tile_groups) == 18, 'Expected exactly 18 source tile groups')
    source_length = rows[data]['p'].get('interplength')
    duration = math.ceil((source_length if source_length is not None else interp_defaults()['interpLength']) * 1000)
    require(0 < duration <= 600000, 'Invalid preview duration')
    tracks, bindings, evidence = [], [], []
    for group in tile_groups:
        actors = source.group_actor(rows, group, matinee)
        require(len(actors) == 1, 'Expected one bound source actor per tile')
        actor = actors[0]
        require(rows[actor]['cls'] == 'interpactor', 'Tile is not an InterpActor')
        component = rows[actor]['p']['staticmeshcomponent']
        source_id = f'{SCENE}:export:{component-1}'
        require(source_id in live, 'Missing exact source component placement: ' + source_id)
        baseline = live[source_id]
        p, r = source.world_pose(rows, None, actor, 0, matinee, data)
        require(np.linalg.norm(p-baseline['p']) < .00001, 'Installed tile position differs from source')
        require(np.linalg.norm(r-baseline['r']) < .00001, 'Installed tile rotation differs from source')
        scale = source.vec(rows[actor]['p'].get('drawscale3d'), (1, 1, 1))[[0, 2, 1]]
        scale *= rows[actor]['p'].get('drawscale', 1.)
        require(np.max(np.abs(scale-baseline['scale'])) < .00001, 'Installed tile scale differs from source')
        require(baseline['visible'] == '1', 'Cannot animate a hidden baseline tile')
        slot = rows[group]['p']['groupname']
        keys = sampled_keys(rows, group, actor, matinee, data, duration, baseline)
        tracks.append(dict(slotId=slot, keys=keys))
        bindings.append(dict(slotId=slot, targetKind='MAP_PLACEMENT', targetId=baseline['id']))
        evidence.append(dict(slotId=slot, actor=rows[actor]['name'], component=rows[component]['name'],
                             sourcePlacementId=source_id, placementId=baseline['id'], keyCount=len(keys)))
    require(len({b['targetId'] for b in bindings}) == 18, 'Duplicate tile binding')
    identity = PREFIX + '.' + tag
    label = '워터팡 / 원본 바닥 ' + name + ' (맵 동작 미리보기)'
    template = dict(sequenceId='sequence.'+identity, displayName=label, category='World',
                    durationMs=duration, interpolation='LINEAR', tracks=tracks, animationTracks=[])
    instance = dict(instanceId='world.sequence.instance.'+identity, templateId=template['sequenceId'],
                    enabled=True, startDelayMs=0, playbackSpeed=1, bindings=bindings)
    cutscene = dict(cutsceneId='cutscene.'+identity, displayName=label, durationMs=duration,
                    cameraCuts=[], worldInstanceIds=[instance['instanceId']])
    report = dict(matinee=matinee, interpData=data, durationMs=duration,
                  durationBasis='serialized InterpLength' if source_length is not None else 'installed Engine Default__InterpData.InterpLength',
                  bindings=evidence)
    return template, instance, cutscene, report


def append_owned(document, field, key, additions):
    existing = {r[key]: r for r in document.setdefault(field, [])}
    require(len(existing) == len(document[field]), 'Duplicate existing IDs: '+field)
    for row in additions:
        if row[key] in existing:
            require(existing[row[key]] == row, 'Preserve modified authoring row: '+row[key])
        else:
            document[field].append(row)
            existing[row[key]] = row


def prepare():
    audit = load_json(AUDIT)['source']
    package = Path(audit['physicalPackage'])
    require(hashlib.sha256(package.read_bytes()).hexdigest() == audit['sha256'], 'Original package changed; re-audit first')
    rows, imports = source.extract_scene(package)
    placement_path = AUTHORING / (AREA+'.mapplacements')
    placement_bytes = placement_path.read_bytes()
    live = placements(placement_bytes)
    templates, instances, shots, cutscenes, evidence = [], [], [], [], []
    for config in MOTIONS:
        t, i, c, report = build_motion(rows, live, *config)
        templates.append(t); instances.append(i); cutscenes.append(c); evidence.append(report)
    for matinee, data, tag in CAMERAS:
        groups_for(rows, matinee, data)
        duration = round(rows[data]['p']['interplength']*1000)
        identity = PREFIX+'.'+tag
        label = f'워터팡 / 원본 도입 카메라 {rows[matinee]["name"].rsplit("_", 1)[-1]} (카메라만)'
        camera_rows = source.make_cameras(rows, matinee, data, duration, identity, label)
        shots.extend(s for _start, _end, s in camera_rows)
        cutscenes.append(dict(cutsceneId='cutscene.'+identity, displayName=label, durationMs=duration,
            cameraCuts=[dict(cutId=identity+f'.cut.{n}', shotId=s['shotId'], startMs=start)
                        for n, (start, _end, s) in enumerate(camera_rows)], worldInstanceIds=[]))
    world_path = AUTHORING / (AREA+'.worldsequences.json')
    camera_path = AUTHORING / (AREA+'.camerashots.json')
    staged = {}
    for path, initial, additions in (
        (world_path, dict(schema='lostark.world-sequences', formatVersion=3, areaId=AREA, revision=1,
                         objectResources=[], templates=[], instances=[]),
         [('templates', 'sequenceId', templates), ('instances', 'instanceId', instances)]),
        (camera_path, dict(schema='lostark.camera-shots', formatVersion=1, areaId=AREA, revision=1, shots=[], cutscenes=[]),
         [('shots', 'shotId', shots), ('cutscenes', 'cutsceneId', cutscenes)])):
        before = path.read_bytes() if path.exists() else None
        document = json.loads(before.decode('utf-8-sig')) if before is not None else initial
        require(document['schema'] == initial['schema'] and document['formatVersion'] == initial['formatVersion']
                and document['areaId'] == AREA, 'Incompatible authoring document')
        original = copy.deepcopy(document)
        for field, key, values in additions:
            append_owned(document, field, key, values)
        if before is not None and document != original:
            document['revision'] += 1
        after = before if document == original and before is not None else encoded(document)
        require(len(after) <= (16777216 if path == world_path else 2097152), 'Merged document exceeds runtime byte limit')
        staged[path] = before, after
    catalog_path = ROOT/'Data/Maps/MapCatalog.json'
    before = catalog_path.read_bytes()
    text = before.decode('utf-8-sig')
    # Patch only this Area's object, retaining every other byte and row.
    start = text.rfind('{', 0, text.index('"id": "'+AREA+'"'))
    old, _ = json.JSONDecoder().raw_decode(text[start:])
    old_text = text[start:start+json.JSONDecoder().raw_decode(text[start:])[1]]
    additions = dict(sourceSequences=world_path.relative_to(ROOT).as_posix(),
                     sequences=f'Client/Bin/DataFiles/Map/{AREA}.worldsequences.json',
                     sourceCameraShots=camera_path.relative_to(ROOT).as_posix(),
                     cameraShots=f'Client/Bin/DataFiles/Map/{AREA}.camerashots.json')
    for key, value in additions.items():
        require(key not in old or old[key] == value, 'Conflicting MapCatalog path: '+key)
    missing = {k:v for k,v in additions.items() if k not in old}
    newline = '\r\n' if '\r\n' in text else '\n'
    changed = old_text.rstrip()[:-1].rstrip()
    if missing:
        changed += ',' + newline  # append fields without reformatting old entries
        changed += (','+newline).join('      '+json.dumps(k)+': '+json.dumps(v) for k,v in missing.items())
        changed += newline+'    }'
        text = text[:start]+changed+text[start+len(old_text):]
    json.loads(text)
    staged[catalog_path] = before, (b'\xef\xbb\xbf' if before.startswith(b'\xef\xbb\xbf') else b'')+text.encode('utf-8')
    for path, before, after in project_registration([world_path, camera_path]):
        staged[path] = before, after
    inventory = []
    for matinee, data in [(m,d) for m,d,_,_ in MOTIONS]+[(m,d) for m,d,_ in CAMERAS]:
        if (matinee, data) in [(m,d) for m,d,_ in CAMERAS]:
            consumed = consumed_camera_tracks(rows, matinee, data)
        else:
            consumed = {t['index'] for g in groups_for(rows, matinee, data)
                        if rows[g]['p'].get('groupname', '').startswith('t') and rows[g]['p'].get('groupname', '')[1:].isdigit()
                        for t in source.active_tracks(rows, g) if t['cls'] == 'interptrackmove'}
        for group in groups_for(rows, matinee, data):
            for track in source.active_tracks(rows, group):
                if track['index'] not in consumed:
                    inventory.append(dict(matinee=matinee, group=rows[group]['p'].get('groupname'),
                        track=track['index'], cls=track['cls'], properties=track['p'], status='not connected by this camera/tile importer'))
    report = dict(source=audit, interpDataDefault=interp_defaults(), importedTileMotions=evidence, cameraShotCount=len(shots), cutsceneCount=len(cutscenes),
                  unconnectedSourceTracks=inventory, imports=imports,
                  limitations=['Camera entries are camera-only: source actor/effect/audio tracks remain unconnected.',
                    'Tile previews stop/restore; source looping and server event/gameplay are not implemented.',
                    'Motions share 18 map placements and must not run concurrently.',
                    'Repair Euler curves cross +180/-180 and currently retain the authored full rotation; inherited UE quaternion interpolation still requires confirmation.',
                    'Source startup branch selection remains unverified.'])
    return staged, {placement_path: placement_bytes}, report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    staged, expected, report = prepare()
    OUT.mkdir(parents=True, exist_ok=True)
    for path, (before, after) in staged.items():
        relative = path.relative_to(ROOT)
        candidate = OUT/'candidate'/relative
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(after)
        if before is not None:
            backup = OUT/'before'/relative
            backup.parent.mkdir(parents=True, exist_ok=True)
            if not backup.exists():
                backup.write_bytes(before)
    (OUT/'source-registration-report.json').write_bytes(encoded(report))
    if args.apply:
        commit_staged_files(staged, expected=expected)
    print(json.dumps(dict(applied=args.apply, files=sum(a != b for a,b in staged.values()),
                          cameraShots=report['cameraShotCount'], cutscenes=report['cutsceneCount'],
                          tileMotions=len(report['importedTileMotions']), report=str(OUT/'source-registration-report.json')),
                     ensure_ascii=False))


if __name__ == '__main__':
    main()
```

## 하네스 전체 코드

```python
"""Source identity and runtime-delta regression; no Client/UI execution."""
import copy
import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import numpy as np
from scipy.spatial.transform import Rotation
import build_maharaka_waterpang_sequences as build


class ImportContracts(unittest.TestCase):
    def test_merge_preserves_and_rejects_modified_or_duplicate_identity(self):
        doc = {'rows': [{'id': 'other', 'value': 7}]}
        row = {'id': 'owned', 'value': 9}
        build.append_owned(doc, 'rows', 'id', [row, row])
        self.assertEqual(doc['rows'], [{'id': 'other', 'value': 7}, row])
        with self.assertRaises(ValueError):
            build.append_owned(doc, 'rows', 'id', [{'id': 'owned', 'value': 10}])
        with self.assertRaises(ValueError):
            build.append_owned({'rows': [row, row]}, 'rows', 'id', [])

    def test_initial_key_limit(self):
        rows = {1: {'p': {'interptracks': [2]}}, 2: {'cls': 'interptrackmove', 'p': {}}}
        with self.assertRaisesRegex(ValueError, 'Initial tile curve'):
            build.sampled_keys(rows, 1, 3, 4, 5, 65536, {})

    def test_freshness_rejects_before_any_write(self):
        with tempfile.TemporaryDirectory(prefix='waterpang-contract-') as folder:
            a, b = Path(folder)/'a', Path(folder)/'b'
            a.write_bytes(b'newer-user-edit'); b.write_bytes(b'old')
            with self.assertRaises(ValueError):
                build.commit_staged_files({a: (b'old', b'replace'), b: (b'old', b'replace')})
            self.assertEqual(a.read_bytes(), b'newer-user-edit')
            self.assertEqual(b.read_bytes(), b'old')

    def test_second_promote_failure_rolls_back(self):
        import source_character_registration as transaction
        with tempfile.TemporaryDirectory(prefix='waterpang-contract-') as folder:
            a, b = Path(folder)/'a', Path(folder)/'b'
            a.write_bytes(b'old-a'); b.write_bytes(b'old-b')
            real = transaction.os.replace
            count = 0
            def fail_second(src, dst):
                nonlocal count
                count += 1
                if count == 2:
                    raise OSError('injected promote failure')
                return real(src, dst)
            with patch.object(transaction.os, 'replace', side_effect=fail_second):
                with self.assertRaises(OSError):
                    build.commit_staged_files({a: (b'old-a', b'new-a'), b: (b'old-b', b'new-b')})
            self.assertEqual(a.read_bytes(), b'old-a')
            self.assertEqual(b.read_bytes(), b'old-b')


class OriginalCurveContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        audit = build.load_json(build.AUDIT)
        cls.rows, _ = build.source.extract_scene(Path(audit['source']['physicalPackage']))
        cls.live = build.placements((build.AUTHORING/(build.AREA+'.mapplacements')).read_bytes())
        root = build.OUT/'candidate/Data/Maps/Authoring'/build.AREA
        cls.world = build.load_json(root/(build.AREA+'.worldsequences.json'))
        cls.camera = build.load_json(root/(build.AREA+'.camerashots.json'))

    def test_every_tile_binding_and_midpoint_world_pose(self):
        for matinee, data, tag, _ in build.MOTIONS:
            identity = build.PREFIX+'.'+tag
            template = next(t for t in self.world['templates'] if t['sequenceId'] == 'sequence.'+identity)
            instance = next(i for i in self.world['instances'] if i['templateId'] == template['sequenceId'])
            self.assertEqual(len(instance['bindings']), 18)
            self.assertEqual(len({b['targetId'] for b in instance['bindings']}), 18)
            groups = {self.rows[g]['p'].get('groupname'): g for g in build.groups_for(self.rows, matinee, data)}
            for track in template['tracks']:
                group = groups[track['slotId']]
                actor, = build.source.group_actor(self.rows, group, matinee)
                component = self.rows[actor]['p']['staticmeshcomponent']
                baseline = self.live[f'{build.SCENE}:export:{component-1}']
                binding = next(b for b in instance['bindings'] if b['slotId'] == track['slotId'])
                self.assertEqual(binding['targetId'], baseline['id'])
                keys = track['keys']
                self.assertLessEqual(len(keys), 4096)
                self.assertEqual(keys[0]['timeMs'], 0)
                self.assertEqual(keys[-1]['timeMs'], template['durationMs'])
                for left, right in zip(keys, keys[1:]):
                    self.assertGreater(right['timeMs'], left['timeMs'])
                    self.assertEqual(left['scaleMultiplier'], [1., 1., 1.])
                    self.assertAlmostEqual(np.linalg.norm(left['rotationQuaternion']), 1., places=7)
                    ms = round((left['timeMs']+right['timeMs'])/2)
                    t = (ms-left['timeMs'])/(right['timeMs']-left['timeMs'])
                    offset = np.array(left['positionOffset'])*(1-t)+np.array(right['positionOffset'])*t
                    q = build.source.slerp(left['rotationQuaternion'], right['rotationQuaternion'], t)
                    p, r = build.source.world_pose(self.rows, group, actor, ms/1000., matinee, data)
                    self.assertLessEqual(np.linalg.norm(baseline['p']+baseline['r']@offset-p), .001001)
                    expected = Rotation.from_matrix(baseline['r'].T@r).as_quat()
                    self.assertLessEqual(build.angle(q, expected), .050001)

    def test_camera_and_independent_motion_cutscenes(self):
        self.assertEqual(len(self.camera['cutscenes']), 6)
        shots = {s['shotId']: s for s in self.camera['shots']}
        instances = {i['instanceId'] for i in self.world['instances']}
        for cutscene in self.camera['cutscenes']:
            self.assertLessEqual(len(cutscene['worldInstanceIds']), 1)
            self.assertTrue(set(cutscene['worldInstanceIds']) <= instances)
            previous_end = 0
            for cut in cutscene['cameraCuts']:
                shot = shots[cut['shotId']]
                self.assertEqual(shot['activation'], 'PATTERN_ONLY')
                self.assertGreaterEqual(cut['startMs'], previous_end)
                previous_end = cut['startMs']+shot['cameraTrack']['durationMs']
                self.assertLessEqual(previous_end, cutscene['durationMs'])
                self.assertLessEqual(len(shot['cameraTrack']['keyframes']), 128)

    def test_bad_component_identity_fails_instead_of_nearest_neighbor(self):
        live = copy.deepcopy(self.live)
        live.pop(next(iter(live)))
        with self.assertRaisesRegex(ValueError, 'Missing exact source component'):
            build.build_motion(self.rows, live, *build.MOTIONS[0])


class SourceFoleyContracts(unittest.TestCase):
    def test_direct_media_source_clock_and_stage_ownership(self):
        import install_maharaka_waterpang_foley as foley
        staged, media = foley.prepare()
        world_path = build.AUTHORING/(build.AREA+'.worldsequences.json')
        world = json.loads(staged[world_path][1])
        collapse = next(t for t in world['templates'] if t['sequenceId'] == 'sequence.'+build.PREFIX+'.collapse')
        self.assertEqual(collapse['durationMs'], 5000)
        self.assertEqual(build.interp_defaults()['objectPath'], 'Default__InterpData')
        timings = {}
        for template in world['templates']:
            for sound in template.get('soundTracks', []):
                timings[sound['soundTrackId']] = sound['startMs']
                self.assertTrue(sound['assetId'].startswith('Sound/Maharaka/WaterpangSource/'))
                self.assertGreater(sound['durationMs'], 0)
                self.assertEqual(len(template['tracks']), 18)
        self.assertEqual(timings, {build.PREFIX+'.collapse.foley': 0,
                                  build.PREFIX+'.intro15.foley': 172,
                                  build.PREFIX+'.intro20.foley': 403})
        self.assertEqual(media['scene_maharakap_waterpangstart']['mediaId'], 714113137)
        self.assertEqual(media['scene_maharakap_fallout_foley']['mediaId'], 427337176)


if __name__ == '__main__':
    unittest.main()
```

## 정본 등록 블록

MapCatalog의 id=LV_OCN_EVENTIS_MHP 행에만 아래 필드를 추가한다.
기존 행의 나머지 필드·다른 Area는 byte-preserving 방식으로 보존한다.

```json
{
  "sourceSequences": "Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.worldsequences.json",
  "sequences": "Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.worldsequences.json",
  "sourceCameraShots": "Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.camerashots.json",
  "cameraShots": "Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.camerashots.json"
}
```

생성 JSON 2개는 Client.vcxproj의 None 항목, filters의 96.DataFiles에 등록한다.
새 C++ 파일·스키마·런타임 경로는 없다. Python 코드는 프로젝트 compile item이 아니다.
실제 생성 키 전체는 out/MaharakaWaterpang20260927/candidate의 JSON 정본으로 보존한다.

## 검증과 설치

1. prepare: 원본 패키지/정본 배치 fresh snapshot, 후보 생성.
2. 독립 리뷰와 unittest: 원본 component 일치, 곡선 중간 샘플 ≤1mm/0.05도,
   독립 컷신, 키/byte 제한, 사용자 수정 보존, rollback.
3. 실행 중인 Client 편집 여부 확인. 디스크 변경 fresh check 뒤 --apply.
4. Publish-MapAuthoring.ps1의 해당 Area Validate → Publish → Check.
   다른 domain/리소스를 함께 변경하지 않는다.
5. git diff --check, JSON/XML parse. 데이터 전용 변경이므로 C++ 빌드를 만들지 않는다.
6. 사용자가 마하라카 → F1 → Open Map Tool → Camera에서 직접 Play/Stop/Scrub 확인.
   에이전트는 Client 실행/조작/캡처/visual PASS를 하지 않는다.

## G02 — 시작·붕괴 원본 효과음

SOUND_SCENE_OCEAN1/3의 원본 Event→단일 Play→Sound→media 연결을 확인한다.
시작 media714113137, 붕괴 media427337176이다. 시작은 각 Matinee의 AkEvent 키에서
172/403ms를 계산하며, 붕괴는 0ms이다. 두 PCM은 원본 WEM과 SHA로 대조한다.
기존 WorldSequence의 soundTracks·재생/Seek/Stop·tail 처리만 사용한다.
도입 시퀀스는 기존 발판18개의 기본자세와 효과음을 소유하며, 새 dummy모델을 만들지 않는다.
BGM MusicSwitch(type13), Stop/Pause/Resume 전환을 단순 WAV 이벤트로 바꾸지 않는다.
원본 음원과 시각은 보존하지만 Wwise 전체 버스 믹스/gain/상태 복원 완료는 아니다.

설치는 G01 고정 후보, 정확히 일치하는 이전 G02 결과, 현재 목표 문서만 허용한다. 다르면 사용자 수정으로
간주해 중단한다. 컷신 길이보다 긴 효과음은 기존 tail 정책을 사용하고 Stop 시 정리한다.
다른 배치/조명/재질과 C++ 파일은 변경하지 않는다. Resource WAV 2개와 JSON2개만
원자적 교체한다. JSON은 이미 프로젝트96.DataFiles에 등록돼 있다.

### Tools/SoundPipeline/audit_maharaka_waterpang_audio.py

```python
"""Identify Waterpang AkEvent actions/media without flattening music containers.

Writes only out/ evidence. Source MusicSwitch (type13) is NOT missing audio,
and Stop/Resume actions must not become fresh one-shot sounds.
"""
import hashlib
import json
from pathlib import Path
import wwise_audio_package as w

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT/'out/MaharakaWaterpang20260927/audio'
EVENTS = ('scene_maharakap_waterpangstart', 'scene_maharakap_fallout_foley',
          'bgm_eventis_mhp_m05_scene_waterpangstart',
          'bgm_eventis_mhp_m05_scene_waterpangstart_skipend', 'bgm_eventis_mhp_m06_survival')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def fast_decrypt(body, stream_offset=0):
    key = w.keystream()
    start = stream_offset % len(key)
    size = len(body)
    pad = (key[start:] + key*(size//len(key)+2))[:size]
    return (int.from_bytes(body, 'big') ^ int.from_bytes(pad, 'big')).to_bytes(size, 'big')


def main():
    # Exact original banks: scene event media and music event control tree.
    filters = ('SOUND_SCENE_OCEAN1', 'SOUND_SCENE_OCEAN3', 'SOUND_BGM_OCEAN2')
    paths = sorted({p for f in filters for p in w.find_packages(w.DEFAULT_PACKAGE_ROOT, f)})
    probe = bytes(range(256))*1800
    for offset in (0, 1, w.KEYSTREAM_PERIOD-3):
        require(w.decrypt(probe, offset) == fast_decrypt(probe, offset), 'XOR equivalence failed')
    previous = w.decrypt
    try:
        w.decrypt = fast_decrypt
        packages = w.load_packages(paths)
        objects = w.merged_objects(packages)
        report = {'packages': [p.name for p in packages], 'events': []}
        for name in EVENTS:
            entry = objects.get(w.fnv1_32(name))
            require(entry is not None and entry[0] == w.HIRC_EVENT, 'Event not found: '+name)
            actions = []
            for identity in w.event_action_ids(entry[1]):
                require(identity in objects and objects[identity][0] == w.HIRC_ACTION, 'Missing action')
                payload = objects[identity][1]
                kind, target = w.action_fields(payload)
                actions.append(dict(id=identity, actionType=kind, target=target,
                                    targetType=objects.get(target, (-1,))[0], payload=payload.hex()))
            sources, unresolved = w.resolve_event(name, objects)
            record = dict(event=name, eventId=w.fnv1_32(name), actions=actions,
                          mediaIds=sources, unresolved=unresolved)
            if len(sources) == 1:
                matches = [(p, p.stream_by_id(sources[0])) for p in packages]
                payloads = [p.payload(e) for p,e in matches if e is not None]
                require(payloads and all(p == payloads[0] for p in payloads), 'Conflicting/missing source media')
                wem = OUT/(name+'.wem')
                require(wem.read_bytes() == payloads[0], 'Rendered candidate belongs to a different media')
                record['mediaSha256'] = hashlib.sha256(payloads[0]).hexdigest()
                record['wavSha256'] = hashlib.sha256(wem.with_suffix('.wav').read_bytes()).hexdigest()
            report['events'].append(record)
        OUT.mkdir(parents=True, exist_ok=True)
        (OUT/'source-event-evidence.json').write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
        print(json.dumps(report, ensure_ascii=False))
    finally:
        w.decrypt = previous


if __name__ == '__main__':
    main()
```

### Tools/MapPipeline/install_maharaka_waterpang_foley.py

```python
"""Attach two source-confirmed one-shot media to Waterpang editor previews.

This is an explicit upgrade of the unedited G01 candidate, never a blanket
replacement of editor changes. Intro preview owns the 18 unchanged stage poses
and a source-timed audio lane. MusicSwitch/BGM and particle actors stay separate.
"""
import argparse
import copy
import hashlib
import io
import json
import wave
from pathlib import Path

import build_maharaka_waterpang_sequences as build


def prepare():
    audio_root = build.OUT/'audio'
    evidence = build.load_json(audio_root/'source-event-evidence.json')
    events = {e['event']: e for e in evidence['events']}
    source_audit = build.load_json(build.AUDIT)['source']
    package = Path(source_audit['physicalPackage'])
    build.require(hashlib.sha256(package.read_bytes()).hexdigest() == source_audit['sha256'], 'Original scene changed')
    rows, imports = build.source.extract_scene(package)

    def event_time(matinee, data, event_name):
        times = [event['time'] for group in build.groups_for(rows, matinee, data)
                 for track in build.source.active_tracks(rows, group) if track['cls'] == 'interptrackakevent'
                 for event in track['p'].get('akevents', [])
                 if imports.get(str(event['event']), '').rsplit('.', 1)[-1] == event_name]
        build.require(len(times) == 1, 'Requires one exact source event time: '+event_name)
        return round(times[0]*1000)
    staged, media = {}, {}
    for name in ('scene_maharakap_waterpangstart', 'scene_maharakap_fallout_foley'):
        event = events[name]
        build.require(len(event['actions']) == 1 and event['actions'][0]['actionType'] == 4
                      and event['actions'][0]['targetType'] == 2 and len(event['mediaIds']) == 1
                      and not event['unresolved'], 'Requires one direct original Sound Play: '+name)
        wav = (audio_root/(name+'.wav')).read_bytes()
        build.require(hashlib.sha256(wav).hexdigest() == event['wavSha256'], 'Decoded audio changed')
        with wave.open(io.BytesIO(wav)) as stream:
            duration = round(stream.getnframes()*1000/stream.getframerate())
            build.require(stream.getsampwidth() == 2 and stream.getnchannels() in (1, 2), 'Unsupported PCM')
        asset = 'Sound/Maharaka/WaterpangSource/'+name+'.wav'
        destination = build.ROOT/'Client/Bin/Resources'/asset
        before = destination.read_bytes() if destination.exists() else None
        build.require(before is None or before == wav, 'Preserve existing resource: '+asset)
        staged[destination] = before, wav
        media[name] = dict(assetId=asset, durationMs=duration, eventId=event['eventId'], mediaId=event['mediaIds'][0])
    paths = {name: build.AUTHORING/(build.AREA+'.'+name+'.json') for name in ('worldsequences', 'camerashots')}
    # Frozen G01 candidate proves ownership; a modified same-ID user row is not migrated.
    originals = {name: build.load_json(build.OUT/'candidate'/path.relative_to(build.ROOT)) for name,path in paths.items()}
    world = copy.deepcopy(originals['worldsequences'])
    cameras = copy.deepcopy(originals['camerashots'])
    collapse_id = 'sequence.'+build.PREFIX+'.collapse'
    collapse = next(t for t in world['templates'] if t['sequenceId'] == collapse_id)
    bindings = next(i['bindings'] for i in world['instances'] if i['templateId'] == collapse_id)
    collapse['soundTracks'] = [dict(soundTrackId=build.PREFIX+'.collapse.foley',
        assetId=media['scene_maharakap_fallout_foley']['assetId'],
        startMs=event_time(42, 157, 'scene_maharakap_fallout_foley'),
        durationMs=media['scene_maharakap_fallout_foley']['durationMs'], volume=1.)]
    for tag, matinee, data in (('intro15', 43, 158), ('intro20', 45, 160)):
        start = event_time(matinee, data, 'scene_maharakap_waterpangstart')
        identity = build.PREFIX+'.'+tag
        cutscene = next(c for c in cameras['cutscenes'] if c['cutsceneId'] == 'cutscene.'+identity)
        duration = cutscene['durationMs']
        # The intro does not move these actors: keep original placed stage poses,
        # with the preview's normal capture/Stop rollback ownership.
        tracks = [dict(slotId=b['slotId'], keys=[dict(timeMs=ms, positionOffset=[0.,0.,0.],
                   rotationQuaternion=[0.,0.,0.,1.], scaleMultiplier=[1.,1.,1.], visible=True)
                   for ms in (0,duration)]) for b in bindings]
        template = dict(sequenceId='sequence.'+identity+'.stage', displayName='워터팡 / 도입 무대 기본 자세·효과음 '+tag,
            category='World', durationMs=duration, interpolation='LINEAR', tracks=tracks, animationTracks=[],
            soundTracks=[dict(soundTrackId=identity+'.foley', assetId=media['scene_maharakap_waterpangstart']['assetId'],
                             startMs=start, durationMs=media['scene_maharakap_waterpangstart']['durationMs'], volume=1.)])
        instance = dict(instanceId='world.sequence.instance.'+identity+'.stage', templateId=template['sequenceId'],
                        enabled=True, startDelayMs=0, playbackSpeed=1, bindings=copy.deepcopy(bindings))
        build.append_owned(world, 'templates', 'sequenceId', [template])
        build.append_owned(world, 'instances', 'instanceId', [instance])
        cutscene['worldInstanceIds'] = [instance['instanceId']]
        cutscene['displayName'] = cutscene['displayName'].replace('(카메라만)', '(카메라·효과음)')
    world['revision'] += 1
    cameras['revision'] += 1
    previous_upgrade = {'worldsequences': copy.deepcopy(world), 'camerashots': copy.deepcopy(cameras)}
    # A missing serialized InterpLength inherits Engine's CDO (5s), not the last
    # movement key (~3.644s). Upgrade only our exact earlier generated candidate.
    collapse_duration = round(build.interp_defaults()['interpLength']*1000)
    collapse_scene = next(c for c in cameras['cutscenes'] if c['cutsceneId'] == 'cutscene.'+build.PREFIX+'.collapse')
    if collapse['durationMs'] != collapse_duration:
        build.require(collapse['durationMs'] < collapse_duration, 'Unexpected collapse window shortening')
        for track in collapse['tracks']:
            last = copy.deepcopy(track['keys'][-1])
            last['timeMs'] = collapse_duration
            track['keys'].append(last)
        collapse['durationMs'] = collapse_duration
        collapse_scene['durationMs'] = collapse_duration
        world['revision'] += 1
        cameras['revision'] += 1
    for name, document in (('worldsequences', world), ('camerashots', cameras)):
        path = paths[name]
        before = path.read_bytes()
        actual = json.loads(before.decode('utf-8-sig'))
        build.require(actual == originals[name] or actual == previous_upgrade[name] or actual == document,
                      'Authoring changed after G01: preserve editor changes and merge explicitly: '+str(path))
        after = before if actual == document else build.encoded(document)
        build.require(len(after) <= (16777216 if name == 'worldsequences' else 2097152), 'Document exceeds runtime cap')
        staged[path] = before, after
    return staged, media


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    staged, media = prepare()
    folder = build.OUT/'foley-upgrade'
    for path, (before, after) in staged.items():
        candidate = folder/'candidate'/path.relative_to(build.ROOT)
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(after)
        if before is not None:
            backup = folder/'before'/path.relative_to(build.ROOT)
            backup.parent.mkdir(parents=True, exist_ok=True)
            if not backup.exists():
                backup.write_bytes(before)
    if args.apply:
        build.commit_staged_files(staged)
    print(json.dumps(dict(applied=args.apply, files=sum(a != b for a,b in staged.values()), media=media), ensure_ascii=False))


if __name__ == '__main__':
    main()
```

### 추가 검사

SourceFoleyContracts.test_direct_media_source_clock_and_stage_ownership에서 단일 원본 media ID,
172/403/0ms 시작키, 실제18개 발판소유, 비어있지 않은 길이를 확인한다.
G01 7검사에 이 검사를 추가한 8검사를 통과한 뒤 publisher Validate/Publish/Check를 다시 실행한다.
