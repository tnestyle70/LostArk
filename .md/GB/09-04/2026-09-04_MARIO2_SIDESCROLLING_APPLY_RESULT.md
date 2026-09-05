# 마리오2(SideScrolling 2) 원본 이식 결과

`마리오2.mp4`(2160×1440, 30fps, 37.7초)를 프레임 단위로 대조하고, 바탕화면
`쿠크_원본_움직임데이터\마리오_전수` 추출본을 근거로 마리오2 카드박스 구간을
`LV_LUT_MIDNIGHTC_ED` 저작 문서에 이식했다. 이 작업은 데이터 전용이며 C++ 변경이 없다.

같은 작업 트리에서 다른 세션(`error documentation review`)이 마리오1을 동시에 작업 중이었다.
파일 소유 경계와 발행 순서를 사전 합의하고 진행했으며 마리오1 자료는 전부 보존됐다.

## 1. 영상 실측

`cv2` Farneback optical flow로 1131 프레임 전부의 밝기·수평/수직 흐름·프레임 차이를 냈다.

```text
암전 구간      1.97~2.90s, 35.73~36.73s
마리오2 구간   3.9s ~ 35.7s (31.8초)
컷             없음. 15.37~15.73s 와 24.27~24.53s 의 dx 급변은 0.4초에 걸친 빠른 팬이다.
정지 구간      13~14s, 22~23s 의 프레임 차이 1.63~1.71 로 배경이 사실상 정지
```

정지 구간의 프레임 차이가 매우 낮다는 것이 중요하다. 영상에서 배경이 활발해 보이는 것은
대부분 카메라 이동이 만드는 시차이고, 자체 모션의 실제 진폭은 아래 실측대로 작다.

캐릭터 화면 높이는 마리오1 135px, 마리오2 90px이다. 마리오1의 확정 거리 18m를 기준으로
비율 1.5를 적용해 마리오2 카메라 거리를 27m로 잡았다.

## 2. 원본 데이터 실측

마리오2 좌표 안의 오브젝트는 174개이나 `LV_LUT_MIDNIGHTCPRACTICE_*` 41+6+1개는
`LV_LUT_MIDNIGHTC_ED_SL02`의 복사본이다(클래스별 개수가 정확히 일치). 이를 제외한
실제 마리오2는 126개다.

```text
LV_LUT_MIDNIGHTC_ED_SL02       efmotionstaticmeshactor 23, interpactor 11, 조명 4
LV_LUT_MIDNIGHTC_ED_SCENE01A   cameraactor 36, interpactor 25, triggervolume 4,
                               efmatineepathnode 6, efmatineepathnodevolume 3, emitter 3
LV_LUT_MIDNIGHTC_ED_PS         staticmeshactor 6
SOUNDSTREAM / MUSIC            efsoundambientvolume 1, efsoundmusicvolume 1
```

마리오2의 매티니 38그룹 중 메시에 붙은 것은 **1개뿐**이고 나머지는 카메라 34, 파티클 3이다.
즉 마리오1·4와 달리 마리오2의 배경 움직임은 매티니가 아니라 자체 모션이 소유한다.

자체 모션 23개의 실측값이다. `05_우리배치ID별_움직임.json`이 이미 우리 배치 ID에
좌표 오차 0.001m 이내로 1:1 매핑해 두었고 누락은 0개였다.

```text
카드 19개(card01b/d/f/h/i/n/o/r)
    상하 이동  ±0.10m,  주기 10s 또는 40s,  UE axis_z(위)
    그중 8개   연속 회전 ±20도/s  (18초에 한 바퀴)
    그중 11개  왕복 회전 ±20도,  주기 50s
deco06 4개
    수평 이동  ±0.30m,  주기 5s,  UE axis_y
    왕복 회전  ±8도,    주기 5s,  UE axis_x
```

모션 공식은 `value(t) = range * sin(2*pi*(t + ftime) / cycle)`이다. 원본이 저장한
`vprevalue`와 대조해 검증했다. 예: 회전 range 20, cycle 50, ftime 39.204 →
계산 −19.55도, 저장 `vprevalue` yaw −3543 rotator = −19.46도로 0.5% 이내 일치.

단위는 위치가 UE cm, 회전이 도다. 좌표 변환 `ours = (X/100, Z/100, -Y/100)`과
회전 규약 `perm[1,0,2] axes[2,1,0] signs[1,1,-1]`(다른 세션이 144쌍으로 실측, 중앙오차
0.0005도)를 그대로 재사용했다. 결과로 UE `axis_z`→우리 +Y, `axis_y`→우리 +Z,
`axis_x`→우리 −X가 된다.

## 3. 구현 완료

### 3.1 자체 모션 23개 → world sequence

`Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`
revision 112 → 113.

```text
sequence.LV_LUT_MIDNIGHTC_ED.mario_m2_spin   연속 회전 카드 8장   tracks 8
sequence.LV_LUT_MIDNIGHTC_ED.mario_m2_deco   deco06 4개          tracks 4
sequence.LV_LUT_MIDNIGHTC_ED.mario_m2_sway   왕복 회전 카드 11장  tracks 11
공통: durationMs 240000, interpolation LINEAR, keys 241/track (1초 간격)
인스턴스 world.sequence.instance.mario_m2_{spin,deco,sway}
```

처음에는 23개를 한 시퀀스에 담았으나 워크스루 정거장 때문에 셋으로 나눴다. 근거는
아래 3.3에 있다. 8 + 4 + 11 = 23으로 배치가 정확히 한 번씩만 들어가므로 두 인스턴스가
같은 대상을 다투지 않는다. 위치가 아니라 모션 종류로 나눈 이유는 이 23개가 특정 단에
속한 소품이 아니라 Y −10.4~+12.8, 가로 92m로 퍼진 배경 카드이기 때문이다. 이 분할은
원작에 있는 구분이 아니라 우리 엔진에 상시 모션이 없어 트리거로 켜야 해서 생긴
저작 선택이다.

`CWorldSequencePlayer::Compose_SampledRecord`(`WorldSequencePlayer.cpp:264`)가
`position += rotate(key.positionOffset, baseline)`과
`XMQuaternionMultiply(key, baseline)`을 쓰므로 키는 배치의 로컬 프레임이다.
원본 모션은 월드 축이므로 굽는 단계에서 baseline을 앞뒤로 상쇄했다.

```text
key.positionOffset      = rotate(worldOffset, conj(baseline))
key.rotationQuaternion  = inv(baseline) * worldRot * baseline      (Hamilton)
```

한계는 시퀀스 트랜스폼 트랙에 `loop` 필드가 없다는 것이다(`WorldSequenceDocument.h:68`의
`loop`는 애니메이션 트랙 전용). 그래서 연속 회전을 240초 분량으로 구워 두었다.
구간 통과가 약 31초이므로 여유는 7배 이상이지만, 240초를 넘겨 머무르면 마지막 자세로 멈춘다.

### 3.2 카메라

`LV_LUT_MIDNIGHTC_ED.camerashots.json` revision 31 → 32에 `shot.mario2` 추가.
다른 세션이 만든 `follow` 블록을 그대로 사용한다.

```text
yaw       33.442도  (원작 cameraactor 클러스터 8대. 진행축과 86.1도로 가장 직각)
pitch     0         (원작 cameraactor pitch 0, eye Y = lookAt Y)
fov       50        (원작 fovangle)
거리      27m       (영상 실측 비율)
높이      4.5m
follow    eyeOffset [-22.53, 4.5, 14.879], lookAtOffset [0.0, 4.5, 0.0]
box       중심 (-1426.3, -5.0, -1205.9), halfExtents (60, 30, 60)
```

`shot.mario1`(마리오1 중심, halfExtents 45)과 300m 이상 떨어져 박스가 겹치지 않는다.

원작 카메라 36대는 서로 다른 포즈가 각각 여러 벌 있고, 그중 일부에 `bhardattach = True`가
붙어 있다. 이 값은 아래 8절의 파서 수정 뒤에야 읽힌 것이다. 하드 어태치는 그 카메라가
다른 액터에 붙어 따라다닌다는 뜻이므로, 고정 카메라가 아니라 `follow` 로 재구성한 선택의
근거가 된다. 다만 무엇에 붙어 있었는지는 해독하지 않았으므로 부착 대상까지 복원한 것은
아니다. 마리오1도 같은 패턴이라고 다른 세션이 확인했다.

### 3.3 트리거

`Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` revision 1861 → 1862.

```text
Mario2_Trigger_1  _7   (-1442.82, -10.75, -1187.67)  -> mario_m2_spin
Mario2_Trigger_3  _8   (-1434.55,  -6.35, -1194.43)  -> mario_m2_deco
Mario2_Trigger_2  _9   (-1428.90,  -3.51, -1203.74)  -> mario_m2_29
Mario2_Trigger_4  _10  (-1439.81,  -3.51, -1215.13)  -> mario_m2_sway
halfExtents (6, 4, 6), triggerOnce true
```

좌표는 원작 `triggervolume_7/8/9/10`을 그대로 쓴다. 번호가 진행 순서와 어긋나는 것은
`_1`과 `_2`를 먼저 발행한 뒤 `_3`, `_4`를 덧붙였기 때문이며, MapTool 워크스루가 이름이
아니라 카메라 진행 방향 사영으로 정렬하므로 동작에는 영향이 없다.

처음에는 시퀀스 대상이 있는 두 개(`_7`, `_9`)만 연결했다가 넷으로 늘렸다. 이유는
워크스루가 지나는 거리다. `shot.mario2`의 follow 오프셋에서 얻은 화면 오른쪽 벡터
(−0.5511, −0.8345)에 정거장을 사영하면 이렇게 된다.

```text
triggervolume_7    0.0m   Y -10.75
triggervolume_8    1.1m   Y  -6.35
triggervolume_9    5.7m   Y  -3.51
triggervolume_10  21.2m   Y  -3.51
```

두 개만 쓰면 화면 가로 진행이 5.7m뿐이고, 넷을 쓰면 21.2m에 3D 경로 38.6m가 되며
높이도 +7.25m 올라간다. 마리오2는 마리오1 같은 긴 가로 구간이 아니라 3단으로 올라가는
구조다. 원작 `efmatineepathnode` 9개가 Y −8.98 / −6.42 / −3.85 세 높이에 쌍으로 있고
각 쌍의 한쪽 끝이 `cameraactor_19 / 23 / 24` 위치와 일치한다. 트리거 Y도 그 세 높이와
맞는다. 원작이 단마다 카메라 레일을 하나씩 둔 것으로 보이나 레일 자체는 복원하지 않았다.

### 3.4 배치는 이미 완비돼 있었다

마리오2 중심 90m 안의 우리 배치는 689행이다.

```text
LV_LUT_MIDNIGHTC_ED_SL02       658
LV_LUT_MIDNIGHTC_ED_SCENE01A    25   (원작 SCENE01A interpactor 25개와 정확히 일치)
LV_LUT_MIDNIGHTC_ED_PS           6   (원작 PS staticmeshactor 6개와 일치)
```

마리오2에 새로 추가할 메시 배치는 없었다. `mapplacements`는 이번 작업에서 수정하지 않았다.

### 3.5 MapTool 워크스루 프리뷰 — 스테이지 공용화

`Play Mario1` 워크스루는 `mario_m1_` 접두사만 고르고 정거장을 X 내림차순으로 정렬하는
마리오1 전용 코드였다. 마리오2는 카메라 yaw 가 달라 X 내림차순이 진행 방향이 아니고,
그대로 쓰면 `_1 -> _4 -> _3 -> _2` 로 거꾸로 간다.

`MapTool.cpp`/`MapTool.h` 는 마리오1 작업 세션이 미빌드 변경을 들고 소유하고 있었으므로
그 세션이 일반화를 맡았고, 이 문서 작성자는 데이터와 검증값을 제공했다. 두 세션이
독립적으로 같은 설계에 도달했다. 축을 쓰지 않고 follow shot 에서 화면 오른쪽 벡터
`cross(up, forward) = (forward.z, 0, -forward.x)` 를 구해 정거장을 사영해 정렬한다.

```text
Collect_MarioWalkStages()          트리거의 playSequence 대상에서 스테이지를 발견한다.
                                   `world.sequence.instance.mario_` 다음부터 첫 `_` 까지가
                                   스테이지 토큰이다. mario_m2_spin -> m2, mario_m1_23 -> m1.
                                   하드코딩된 스테이지 목록이 없다.
Play_MarioWalkthrough(stageToken)  Play_Mario1Walkthrough 를 대체한다.
UI                                 Cutscene Arena Preview > Side Scrolling Walkthrough 에
                                   발견된 스테이지마다 버튼(Play m1, Play m2)과 Stop.
```

마리오1 회귀가 없다는 근거는 사영 순서가 기존 X 내림차순과 같은 결과를 준다는 것이다.
발행된 실제 데이터로 확인했다.

```text
[m1] 화면오른쪽 (-0.6727, -0.7399)  T6 0.0m / T5 17.6m / T4 28.8m / T3 53.1m / T2 67.0m / T1 72.8m
     X 내림차순과 같은 순서인가: 예
[m2] 화면오른쪽 (-0.5511, -0.8345)  T1 0.0m / T3 1.1m / T2 5.7m / T4 21.2m
     X 내림차순과 같은 순서인가: 아니오
```

워커는 구간 길이와 위치를 모두 3D로 계산하므로 마리오2의 +7.25m 등반이 반영된다.
읽어서 확인했다. 양 끝 연장(`MARIO_WALK_LEAD_METRES` 14m)만 수평이며 높이를 유지한다.

이 문서 작성자는 코드를 읽어 계약을 확인했고, `/Zs` 검사는 해당 파일을 소유한 세션이
실행해 exit 0 을 보고했다. `/Zs` 는 구문·의미 검사만 하고 `.obj` 등 산출물을 만들지
않으며 `IntDir`/`OutDir` 를 건드리지 않으므로, `CLAUDE.md` 가 금지하는 "Visual Studio 와
자동화 빌드를 겹쳐 실행"에 해당하지 않는다. 이 세션이 `cl.exe` 를 돌리지 않은 것은
금지 때문이 아니라 보수적인 선택이며, 그래서 이 문서는 그 exit 0 을 인용값으로 적고
자체 실측으로 적지 않는다. 실제로 링크까지 하는 Client 빌드는 사용자가 수행한다.

## 4. 실행한 자동 검증

```text
베이크 역검증        런타임 합성식으로 키를 되돌려 월드값과 대조
                     위치 최대오차 5.39e-06 m, 회전 최대오차 8.99e-05 도
바인딩 대상 실재     mario_m2_ambient 23개 전부 mapplacements 에 존재, 누락 0
트리거 대상 실재     Mario2_Trigger_1/2 가 가리키는 인스턴스 2개 모두 존재
시퀀스 계약          트랙 23 <= 32, 키 241 <= 256, 첫 키 0, 끝 키 = duration, 단조 증가
회전 궤적            t=0 에서 0도, t=9s 에서 180도, t=18s 에서 360도 (원본 20도/s와 일치)
Publish-MapAuthoring -Mode Validate    exit 0, PlacementCount 3231
Publish-WorldGameplay -Mode Validate   KAKULSAYDON_ARENA 24 placements
Publish-MapAuthoring -Mode Publish     exit 0, SHA256 dbdc348a30e3853daa00cf4a9a160f341cc7a987f868a2c11630b8bea1a4ee47
Publish-WorldGameplay -Mode Publish    exit 0, KAKULSAYDON_ARENA.worldbootstrap 갱신
런타임 문서 확인     shot.mario2 follow, worldbootstrap 의 Mario2_Trigger_1~4

시퀀스 3분할 뒤 재발행 (revision 114)
저장 키 전수 검증    5543개를 원본 수식과 대조. 위치 최대오차 7.62e-06 m,
                     회전 최대오차 8.99e-05 도. 반올림된 저장값 기준이다.
배치 커버리지        spin 8 + deco 4 + sway 11 = 23, 중복 0, 누락 0
런타임 확인          세 인스턴스의 바인딩 합계 23, 구 mario_m2_ambient 제거 확인,
                     worldbootstrap 에 Mario2_Trigger_1~4 와 Mario1_Trigger 6개 공존
다른 세션 자료 보존  mario_m1_23/24/26/27/28/44, shot.mario1 follow,
                     Mario1_Trigger_1~6 의 시퀀스 바인딩 전부 무변경
```

## 5. 미검증 — 사용자 육안 확인 필요

에이전트는 Client를 실행하거나 화면을 판정하지 않는다. 아래는 사용자가 직접 확인한다.

```text
F1 -> Map Tool -> World Sequence 에서 mario_m2_ambient 선택 후 Play
  카드 8장이 계속 돌고, 11장이 천천히 흔들리고, deco06 4개가 5초 주기로 움직이는지
마리오2 구간 진입 시 shot.mario2 프레이밍이 맞는지 (거리 27m, 높이 4.5m)
Mario2_Trigger_1 이 실제로 밟히는 높이에 있는지
```

프레이밍이 안 맞으면 `camerashots.json`의 두 숫자만 고치면 되고 재빌드가 필요 없다.
거리는 `eyeOffset`의 X/Z를 같은 비율로, 높이는 `eyeOffset`과 `lookAtOffset`의 Y를 함께 올린다.

## 6. 적용하지 않은 것과 이유

```text
조명 3개   pointlightmovable 2 + dominantpointlight 1
           사용자가 조명 범위를 직접 가져갔다. 값만 뽑아 넘기고 적용하지 않았다.
           MapCatalog 의 sourceLights/lights 는 반드시 쌍으로 선언해야 하며 Area 단위다.
           light.mario2.dominantpointlight_0  (-1419.700, 6.590, -1213.140) r 12.001 falloff 4.0 br 5.0
           light.mario2.pointlightmovable_3   (-1400.970, 7.640, -1181.890) r 13.863 falloff 4.0 br 7.0
           light.mario2.pointlightmovable_5   (-1452.700, 7.640, -1233.620) r 13.863 falloff 4.0 br 7.0
           radius 와 falloff 와 brightness 는 원본 행에 직렬화된 실측값이다.
           발탄 문서의 falloff 가 class default 추론이었던 것과 구분된다.
           color 는 세 컴포넌트 전부 lightcolor 프로퍼티가 직렬화돼 있지 않다.
           UE3 는 기본값과 다른 프로퍼티만 직렬화하므로 이는 LightComponent 의 class
           default, 즉 백색 [1.0, 1.0, 1.0, 1.0] 이라는 뜻이다. 값을 못 읽은 것이 아니라
           원본이 백색이다. 다만 class default 자체는 원본 행이 아니라 UE3 규약에서
           온 추론이므로 그렇게 표시한다.
           참고로 다른 세션이 마리오3/4 에서 찾은 호박색 조명은 lightcolor 가 실제로
           직렬화된 경우이고, 그 바이트 순서는 FColor 의 (B, G, R, A) 다.
스포트라이트 1개
           dominantspotlight_1 (-1440.64, 34.73, -1194.89).
           Engine 의 LIGHT enum 에 SPOT 이 없고, 원본 컴포넌트에도 radius/brightness 가
           직렬화돼 있지 않다. 값 자체가 없어서 point 근사도 임의값이 된다.
파티클 3개 emitter_3/4/5, 전부 particlesystemcomponent_65 공유.
           (-1433.52,-11.60,-1198.92) (-1434.47,-11.60,-1219.65) (-1438.57,-11.60,-1223.52)
           파티클 템플릿 이름이 추출본에 없고, mapeffects pair 도 Area 단위 선언이다.
사운드 2개 SideScrolling 2 음악, SideScrolling_2_Card_zone 환경음. Area 에 사운드 레이어가 없다.
카메라 레일  원작은 단마다 카메라 레일(efmatineepathnode 쌍)을 두지만 우리 camerashots 는
           고정 eye/lookAt 또는 플레이어 추적 두 가지뿐이라 레일을 재현하지 않았다.
(MapTool 워크스루 프리뷰는 3.5 로 옮겼다. 적용됐다.)
```

## 7. 추정으로 넣은 값

원본에서 그대로 가져오지 못해 이쪽에서 정한 값이다. 사용자가 조절할 부분이다.

```text
카메라 거리 27m, 높이 4.5m   영상에서 잰 값이지 원본 데이터가 아니다
트리거 박스 크기 (6, 4, 6)   원작 브러시가 BSP 라 크기를 읽을 수 없다
트리거-시퀀스 짝짓기         원작 Kismet 연결을 해독하지 못했다. 대상이 있는 것끼리 붙였다
시퀀스 길이 240초            원본은 상시 동작이다. 우리 시퀀스에 loop 가 없어 구워 넣었다
```

## 8. 부수 발견 — 추출 파서의 bool 프로퍼티 버그

조명 값을 교차 확인하다 추출 파서 `la_props.py`의 버그를 찾았다. 마리오2 이식 자체에는
영향이 없지만 조명 인계 문서와 앞으로의 추출에 영향이 있어 남긴다.

```python
kind = cur.name().lower()      # 소문자로 접은 뒤
elif kind == 'BoolProperty':   # 대문자 리터럴과 비교하므로 절대 참이 되지 않는다
```

UE3는 BoolProperty의 값을 데이터 영역이 아니라 태그 헤더에 넣는다. Size는 0이고
ArrayIndex 다음에 1바이트가 붙는다. 분기를 타지 않으니 그 1바이트를 소비하지 않고,
같은 오브젝트에서 bool 뒤에 오는 프로퍼티가 전부 어긋난다. `Scene.props`가 이름이
식별자 모양이 아닌 지점에서 run을 끊기 때문에 어긋난 뒤는 조용히 버려지고,
결과적으로 "그 프로퍼티가 없다"처럼 보인다.

```text
기존 파서  pointlightcomponent_1062: ... brightness, baffectcompositeshadowdirection 에서 끝
고친 파서  ... brightness, lightcolor, baffectcompositeshadowdirection=False,
           bprecomputedlightingisvalid=False, lightingchannels, lightaffectsclassification
```

고친 파서로 읽으니 `benabled`가 값으로 나온다. 다른 세션이 추론으로 남겨 둔 세 개가
전부 실제 FALSE, 즉 원본에서 꺼진 조명으로 확정됐다.

```text
마리오3  pointlightmovable_2   SL02 export 729       pointlightcomponent_2697
마리오4  pointlightmovable_1   SL02 export 728       pointlightcomponent_1064
마리오4  spotlightmovable_0    SCENE01A export 1612  radius 3000, brightness 7, inner 8.5, outer 9
```

마리오2의 세 조명은 `benabled` 자체가 없으므로 class default TRUE, 즉 켜져 있다.

이 김에 마리오2 자체 모션 57개 정의를 원본 패키지에서 다시 읽어 대조했다. 액터의
`motionarr` 오브젝트 인덱스로 찾고 클래스별 대표 필드를 want로 넘기는 방식이며
불일치 0개다. 모션 정의는 이름이 export 간 중복되므로(`efactormotionlocationcycle_6`
등) 이름으로 찾으면 엉뚱한 오브젝트를 집는다. 처음에 그렇게 해서 잘못된 불일치 9건을
봤고, 오브젝트 인덱스로 바꿔 해소했다.

이 버그의 사정권에 있던 나머지 입력도 고친 파서로 다시 읽어 대조했다. `cameraactor`는
프로퍼티에 bool 인 `bhardattach`가 있어 확인이 필요했다. 결과는 전부 동일하다.

```text
마리오2 cameraactor 36대, yaw 분포 33.442x8 / 50.691x8 / 45.461x8 / 45.0x4 / ...
  -> 내가 고른 33.442 와 대수까지 동일. fovangle 50.0, pitch 0.0 도 동일
triggervolume_7  (-1442.823, -10.748, -1187.668)   발행값 (-1442.82, -10.75, -1187.67)
triggervolume_9  (-1428.900,  -3.507, -1203.745)   발행값 (-1428.90,  -3.51, -1203.74)
```

`bhardattach`는 이제 True 로 읽히지만 `location`, `rotation`, `fovangle`이 그보다 앞에
있어 잘리지 않았다. 즉 발행한 마리오2 카메라와 트리거 값은 수정이 필요 없다.

다른 세션이 독립 재현한 결과 이 버그는 두 곳에 있었다. `la_props.py` 원본과, 메모리 상한을
넣으려고 `read_properties`를 몽키패치한 재구현본이다. 재구현본에는 bool 분기가 아예 없어서
원본만 고치면 추출 결과가 바뀌지 않는다. 같은 세션이 `bHidden` 29개와 interpgroup 162개를
다시 읽어 차이 0건을 확인했으므로 이미 발행된 마리오1 시퀀스와 visible=0 결정도 유효하다.

이 버그로 값이 실제로 틀어진 사례는 지금까지 0건이다. 잃었던 것은 bool 값 자체와
bool 뒤에 오던 프로퍼티(`lightingchannels` 등)뿐이다.

고친 파서 사본은 `C:\Users\USER\.claude\jobs\e7989141\tmp\la_props.py`에 있다.
원본 추출 스크립트는 다른 세션 소유라 수정하지 않았다.

## 9. 되돌리기

```text
C:\Users\USER\.claude\jobs\e7989141\tmp\backup\ws.before-mario2.json  -> worldsequences.json
C:\Users\USER\.claude\jobs\e7989141\tmp\backup\cs.before-mario2.json  -> camerashots.json
C:\Users\USER\.claude\jobs\e7989141\tmp\backup\gw.before-mario2.json  -> Gameplay.world.json
```

되돌린 뒤 `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish`와
`Publish-WorldGameplay.ps1 -Mode Publish`를 다시 실행한다. 단, 백업 시점 이후 다른 세션이
마리오1을 추가 수정했다면 그 변경까지 같이 되돌아간다.
