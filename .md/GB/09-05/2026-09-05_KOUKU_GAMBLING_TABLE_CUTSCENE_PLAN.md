# 2026-09-05 쿠크세이튼 도박판 컷신 PLAN + 설계도

Area `LV_LUT_MIDNIGHTC_ED`. 원작 `SCENE04A / interpdata_2 / 27.000초`의 초록 카드
테이블 컷신을 팝업북과 같은 경로로 복원한다.

이 문서의 모든 수치는 `B9AVB2VAZIQRPQCJVKAVYRAVOKYPY806.upk`를 직접 열어 얻었다.
바탕화면 추출본은 근거가 아니라 **대조 대상**이다. 그 추출본이 어디서 어떻게 틀렸는지도
아래 3절에 수치로 적는다.

문서의 G 경계는 **G1(추출 게이트)까지**다. G1은 전체 코드와 실행 결과를 싣는다.
G2 이후는 4절 설계도에서 계약과 종료 증거를 확정하고, G1 산출물이 나온 뒤 같은 문서에
전체 코드를 덧붙인다.

---

## 1. 현재 실제 반영 상태

### 1.1 이미 있는 것 (팝업북에서 만든 재사용 자산)

| 자산 | 경로 | 이번에 쓰는 방식 |
|---|---|---|
| 좌표·회전 변환 정본 | `Tools/LevelPlacementExtractor/build_maptool_scene.py` | `convert_position` / `convert_rotation` 그대로 |
| 아레나 숨김 목록 461개 | `Client/Public/KakulArenaHiddenPlacements.h` | 컷신 세트 교체에 그대로 |
| 숨김 목록 생성기 | `Tools/KakulSaydonPipeline/build_arena_hidden_ids.py` | 변경 없음 |
| 월드 시퀀스 문서 | `Data/Maps/Authoring/.../LV_LUT_MIDNIGHTC_ED.worldsequences.json` rev 117, template 55, instance 59 | template/instance 추가 |
| 카메라 샷 문서 | `Data/Maps/Authoring/.../LV_LUT_MIDNIGHTC_ED.camerashots.json` rev 56, shot 4 | shot 1개 추가 |
| 클립 체이닝 | `WORLD_SEQUENCE_ANIMATION_TRACK::startMs` | 클립 37개 연결 |
| deploy prop 자세 재생 | `CDeployPropObject::Apply_AnimationAuthoringPose` | 소품 transform 재생 |
| 제품 컷신 진입 | `CLevel_KakulSaydonArena::Start_PopupBookCutscene` / `Apply_CutsceneSetVisible` | 도박판 분기 추가 |
| 파티 배치 | `CGameRoom::Place_PartyForCutscene` | instanceId 분기 추가 |
| 시네마틱 카메라 샘플러 | `VALTAN_CINEMATIC_CAMERA_CUE` + `KAKUL_CAMERA_SHOT::CameraTrack` | 컷 표현에 그대로 |

런타임 계약의 실제 구조체는 다음과 같다. 설계는 이 필드만 쓴다.

```cpp
struct WORLD_SEQUENCE_TRANSFORM_KEY
{
    uint32_t  timeMs = 0;
    float3_t  positionOffset = {};                        // 배치 기준 오프셋
    float4_t  rotationQuaternion = float4_t(0,0,0,1);
    float3_t  scaleMultiplier = float3_t(1,1,1);
    bool_t    visible = true;
};

struct WORLD_SEQUENCE_ANIMATION_TRACK
{
    std::string slotId;
    uint32_t    startMs = 0;
    std::string clipName;
    f32_t       playbackRate = 1.f;
    bool_t      loop = false;
    bool_t      holdLastFrame = true;
};
```

카메라 트랙 상한은 `Tools/CompositionPipeline/composition_pipeline.py`의
`CAMERA_TRACK_MAX_KEYFRAMES = 64`, `CAMERA_TRACK_MAX_DURATION_MS = 120000`이다.

### 1.2 없는 것

- SCENE04A를 우리 문서로 바꾸는 import 도구. 팝업북 때는 임시 폴더에서만 돌리고
  저장소에 남기지 않아 지금 다시 쓸 수 없다. **이번에는 도구로 커밋한다.**
- 테이블·의자·촛대 메시의 쿠킹 결과. 추출본은 `psk`/`gltf` 상태다.
- `WORLD_SEQUENCE_ANIMATION_TRACK`의 역재생 필드. 원작이 클립 4개를 역재생한다(3.8 참조).

---

## 2. 시퀀스와 트랙 실측

### 2.1 이 패키지의 시퀀스 4개

```text
efseqact_matinee_1 / interpdata_1   11950ms   그룹 28
efseqact_matinee_2 / interpdata_2   27000ms   그룹 98   <- 도박판 컷신
efseqact_matinee_4 / interpdata_4     620ms   그룹  2
efseqact_matinee_6 / interpdata_6    1000ms   그룹  3
```

### 2.2 interpdata_2 트랙의 활성·비활성

UE3은 `bDisableTrack`을 기본값일 때 저장하지 않는다. 즉 **기록돼 있으면 꺼진 트랙**이다.

```text
트랙 클래스                         전체   활성   비활성
interptrackmove                    111     75     36
interptrackfloatprop                44     44      0
interptrackanimcontrol              40     40      0
interptracktoggle                   35     35      0
interptrackdirector                  4      1      3
interptrackfade                      1      1      0
interptrackevent                     1      1      0
interptrackvisibility                1      1      0
```

**move 트랙 111개 중 36개(3분의 1)가 꺼져 있다.** 전부 가져오면 원작이 움직이지 않는
것을 움직이게 만든다.

### 2.3 move 트랙의 기준 프레임

```text
moveframe 미기록(= 월드)          68
moveframe imf_relativetoinitial   43
```

`default`가 월드라는 것은 추론이 아니라 대조로 확인했다. 자기 위치를 가진 카메라의
`default` 트랙 첫 키가 그 위치에서 1~7m 안에 있다.

```text
cam2  actor [-2.9927, 2.5232, 739.8415]   첫 키 [-1.87, 2.37, 739.13]   차이 1.34 m
cam3  actor [-349.0059, -60.3536, 481.73] 첫 키 [-348.80, -54.15, 479.03] 차이 6.77 m
cam6  actor [-302.6581, -98.6521, 453.50] 첫 키 [-302.94, -99.65, 451.98] 차이 1.84 m
```

`imf_relativetoinitial` 트랙은 반대로 첫 키와 마지막 키가 모두 `(0, 0, 0)`이다.
배치에 더해야 하는 오프셋이다.

---

## 3. 기존 추출본의 결함 세 가지

바탕화면 `쿠크_컷신_전체추출_20260904`는 검수 문서까지 붙어 있지만, 이 컷신을 만드는 데
필요한 세 가지가 틀렸다. 원인과 복구 방법을 모두 확인했다.

### 3.1 배열 트랙이 첫 요소에서 멈춘다

`SCENE04A_806.director.json`은 컷 트랙을 `decodedPartial: true`, 키 1개로 적었다.
그 1개는 `{"time": -1.01, "targetcamgroup": "cam1"}`이고 4개 트랙이 전부 같다.
`-1.01초`는 시퀀스 시작 이전이라 그대로 쓰면 컷 순서를 만들 수 없다. fade는 키 0개,
FOV 곡선도 키 0개였다.

원인은 `추출도구/la_props.py`의 `arrayproperty` 처리다. 요소 0을 읽은 뒤 종료 태그
다음 8바이트를 다음 속성 이름으로 잘못 읽어(`1_9`) 배열 나머지를 hex 덩어리로 삼켰다.

태그 배치를 이름 테이블로 확정했다.

```text
속성 태그 = 이름 FName(8) + 타입 FName(8) + size i32 + arrayIndex i32 + 값(size)
           구조체/열거형이면 값 앞에 이름 FName(8)이 하나 더 붙는다
           bool 은 size 0 이고 값이 1바이트다
배열 본문 = 요소 개수 i32 + 요소들, 각 요소는 'none' 태그로 끝난다

FDirectorTrackCut     time transitiontime targetcamgroup shotnumber
FInterpCurvePoint     inval outval arrivetangent leavetangent interpmode
FAnimControlTrackKey  starttime animseqname animstartoffset animendoffset
                      animplayrate blooping breverse benablerootmotion
```

요소 경계는 종료 태그를 믿지 않고 **요소 선두 태그 패턴**으로 찾는다. 컷·toggle·
visibility·event는 `(time, floatproperty)`, 곡선 점은 `(inval, floatproperty)`,
애니메이션은 `(starttime, floatproperty)`다. 이 방법으로 모든 배열이
`declared == decoded`가 됐다.

### 3.2 animcontrol 클립 절반을 잃었다

추출본 README는 "move / visibility / animcontrol 은 완전히 읽힌다"고 적었다.
**animcontrol은 사실이 아니다.**

```text
              내 판독   추출본
move 트랙        111      111    키 641 / 641   일치
animcontrol      37       18    <- 19개 누락
```

move는 트랙 수와 키 수가 정확히 일치하므로 두 판독이 서로를 검증한다. animcontrol만
절반이 빠졌다.

### 3.3 애니메이션의 재생 필드를 통째로 버렸다

추출본은 `{clip, timeMs}` 두 값만 남겼다. 실제 구조체에는 다음이 더 있다.

```text
animplayrate   재생 속도   0.2 와 0.6 이 실제로 쓰인다
blooping       루프 여부
breverse       역재생 여부   4개 클립이 역재생이다
animstartoffset / animendoffset   클립 구간
```

팝업북에서 `playbackSpeed 0.7`을 두고 헤맨 것이 정확히 이 필드다.

---

## 4. 확정된 원본 내용

### 4.1 카메라 컷 목록 — 확정

`interpdata_2`의 director 그룹은 `interpgroupdirector_0 #526` 하나이고 컷 트랙을
4개 갖는다. UE3의 `UInterpGroupDirector::GetDirectorTrack()`은 `InterpTracks` 순서에서
`bDisableTrack`이 꺼진 첫 트랙을 반환한다. 저장된 순서와 플래그는 다음과 같다.

```text
슬롯 트랙                     off       bDisableTrack   컷
 0   interptrackdirector_0   453187    true            -1.01 cam1 | 8.65 cam2 | 13.95 cam3 | 19.49 cam4 | 24.54 cam1 | 27.78 dirgroup
 1   efinterptracksubtitle_0
 2   interptrackfade_0
 3   interptrackevent_0
 4   interptrackdirector_1   454044    true            -1.01 cam1 | 8.49 cam2 | 13.95 cam3 | 19.49 cam4 | 24.54 cam1 | 24.78 dirgroup
 5   interptrackdirector_2   454901    true            -1.01 cam1 | 2.10 cam2 | 13.95 cam3 | 19.49 cam4 | 23.95 cam1
 6   interptrackdirector_3   455626    (없음 = 켜짐)    -1.01 cam1 | 2.10 cam2 | 13.95 cam3 | 19.49 cam4 | 23.95 cam6
```

`bDisableTrack`은 값이 `0x01`인 bool이고, 세 트랙에만 기록돼 있다. 따라서 **재생되는
컷 목록은 `interptrackdirector_3` 하나로 확정된다.**

```text
-1.010s  cam1
 2.100s  cam2
13.950s  cam3
19.490s  cam4
23.950s  cam6
```

같은 규칙을 다른 시퀀스에 적용하면 `interpdata_1`은 `interptrackdirector_1`,
`interpdata_6`은 `interptrackdirector_3`이 켜져 있다. 시퀀스마다 정확히 하나가 켜져
있다는 불변식이 성립한다.

### 4.2 컷 카메라의 트랙 — 활성 트랙은 정확히 하나씩

```text
cam1  actor [0, 0, 0]                     move 11개 중 활성 2 (내용 동일한 중복)  키 11  창     0..28690
cam2  actor [-2.9927, 2.5232, 739.8415]   move  8개 중 활성 1                    키 15  창  2070..22550
cam3  actor [-349.0059, -60.3536, 481.73] move  2개 중 활성 1                    키  5  창 12370..23000
cam4  actor [0, 0, 0]                     move  4개 중 활성 1                    키  9  창 19080..25310
cam6  actor [-302.6581, -98.6521, 453.50] move  3개 중 활성 1                    키  3  창 23740..27000
```

`cam1`의 활성 두 트랙(`interptrackmove_383`, `_382`)은 position과 euler가 모두
동일하다. 하나만 쓴다. 전부 `default` 프레임이므로 **모든 카메라 키는 월드 좌표다.**
팝업북과 달리 더미 합성이 필요 없다.

컷 시각이 대상 카메라의 이동 창 안에 들어가는지도 확인했다.

```text
 2.10 -> cam2   창  2070..22550   포함
13.95 -> cam3   창 12370..23000   포함
19.49 -> cam4   창 19080..25310   포함
23.95 -> cam6   창 23740..27000   포함, 창 끝이 시퀀스 끝 27000 과 일치
```

### 4.3 FOV — 카메라마다 활성 트랙 하나

UE3 `FOVAngle`은 **수평**이고 우리 `fovYDegrees`는 **수직**이다.
`fovY = 2·atan(tan(fovX/2) / 1.5)`로 변환한 값을 함께 적는다.

```text
cam1   0ms 65도(수직 46.023) | 2890ms 65도 | 4420ms 50도(34.538) | 24540ms 50도
cam2  11040ms 50도(34.538)
cam3  14440ms 80도(58.445)
cam4  20080ms 60도(42.103) | 22810ms 60도 | 27330ms 80도(58.445)
cam6  24150ms 50도(34.538)
```

### 4.4 fade — 암전 구간이 컷을 가린다

```text
    0ms  1.0   검은 화면에서 시작
  970ms  0.0   밝아짐
11660ms  0.0
13480ms  1.0   암전
14720ms  1.0   암전 유지
16290ms  0.0   밝아짐
25590ms  0.0
27000ms  1.0   암전으로 끝
```

`13.95s` 컷(cam2 → cam3)이 `13.48~14.72s` 암전 구간 안에 있다. 화면이 검을 때 바뀌는
숨은 컷이고, 이 컷에서 카메라가 아레나에서 대기 무대로 건너뛴다(4.6 참조).

### 4.5 애니메이션 클립 37개 — 전체

```text
     시각  그룹     슬롯  클립                     배속   루프  역재생  추출본
    -6480  pc1      b    idle_normal_1            1.0   O     -      O
    -6480  pc2      b    idle_normal_1            1.0   O     -      O
    -6480  pc3      b    idle_normal_1            1.0   O     -      O
    -6480  pc4      b    idle_normal_1            1.0   O     -      O
    -3730  쿠크      a    idle_normal_1            1.0   O     -      O
    -3730  쿠크      b    idle_normal_1            1.0   O     -      O
    -1720  pc4      a    idle_battle_1            1.0   O     -      O
    -1290  세이튼     a    evt2_book01              1.0   -     -      O
    -1240  pc3      a    idle_battle_1            1.0   O     -      O
     -700  pc2      a    idle_battle_1            1.0   O     -      O
        0  tab1     a    idle_normal_1            1.0   O     -      O
        0  pc1      a    idle_battle_1            1.0   O     -      O
        0  책110     a    idle_normal_1            1.0   O     -      O
     1340  pc1      b    idle_normal_1_1          1.0   O     -      누락
     1340  pc3      b    idle_normal_1_1          1.0   O     -      누락
     2050  책       a    evt2_book02              1.0   -     -      O
     2470  pc2      b    idle_normal_1_1          1.0   O     -      누락
     3320  pc4      b    idle_normal_1_1          1.0   O     -      누락
     4110  책10      a    evt2_book02              1.0   -     -      O
     4330  세이튼     b    idle_normal_1            1.0   O     -      O
     4900  세이튼     b    evt2_book03              1.0   -     -      누락
     5320  tab1     a    evt2_table_open01        1.0   -     -      누락
     5830  세이튼     a    evt2_book02_loop         1.0   O     -      누락
     7450  세이튼     b    evt2_book01              1.0   -     역재생  누락
     9040  책10      a    evt2_book01              1.0   -     역재생  누락
    10300  책       a    evt2_book01              1.0   -     역재생  누락
    10450  세이튼     a    att_battle_16_01         1.0   -     -      누락
    11000  table    a    evt2_table_open01        1.0   -     역재생  O
    15190  book     a    evt2_book01              1.0   -     -      O
    15350  세이튼     a    idle_normal_1            1.0   O     -      누락
    15660  table    a    evt2_table_open01        1.0   -     -      누락
    18080  쿠크      a    att_battle_5_03          1.0   -     -      누락
    19680  쿠크      b    idle_battle_1            1.0   O     -      누락
    20970  쿠크      a    att_battle_10_01         1.0   -     -      누락
    21693  쿠크      a    att_battle_10_01         0.2   -     -      누락
    24050  쿠크      b    att_battle_9_02          0.6   -     -      누락
    24810  쿠크      a    idle_normal_1            0.6   -     -      누락
```

읽어야 할 것 셋.

- 테이블은 `evt2_table_open01`을 **세 번** 재생한다. `tab1`이 5.320초에 정방향,
  `table`이 11.000초에 **역재생**, 15.660초에 다시 정방향이다. 추출본만 보면 11.000초
  하나뿐이고 그것이 역재생인 줄도 알 수 없다.
- 쿠크는 18.080초부터 끝까지 연기한다. `att_battle_5_03` → `idle_battle_1` →
  `att_battle_10_01` → 같은 클립 **0.2배속** → `att_battle_9_02` **0.6배속** →
  `idle_normal_1` 0.6배속. 슬로모션 연출이며 추출본에는 하나도 없다.
- 음수 시각 클립 10개는 시퀀스 시작 시점에 이미 재생 중이라는 뜻이다. 진입할 때
  해당 위치의 포즈로 시작해야 한다.

### 4.6 무대가 두 곳이다

액터 위치를 z로 가르면 두 무리다.

```text
아레나 (z ≈ 737~748, y ≈ 2)
   카메라 cam2, bdum, dd, tab1dum, tabetcdum, tabldum
   이미터 pow1~pow4, pow11~pow41, hpow, loop, pp1, h1, po11
   조명   l1
대기 무대 (z ≈ 434~465, y ≈ -108.5)
   의자1~4  bg_rad_koukusaton_chair01_sm_hht
   can1/can2 bg_rad_koukusaton_deco19_sm_hht
   카메라 cam3(y -54), cam4(y -98), cam6(y -99)
```

대기 무대는 "주차장"이 아니라 **실제로 촬영되는 두 번째 무대**다. `cam4`는 `y = -98.43`,
`cam6`은 `y = -99.65`로 의자보다 10m 위에 있다. 즉 컷이 아레나에서 지하 무대로
건너뛴다.

```text
-1.01~ 2.10s  cam1  아레나 z≈748 에서 시작해 z≈324 까지 이동
 2.10~13.95s  cam2  아레나 z≈739
13.95~19.49s  cam3  대기 무대 z≈479   <- 암전 뒤 전환
19.49~23.95s  cam4  대기 무대 z≈451
23.95~27.00s  cam6  대기 무대 z≈452
```

### 4.7 의자는 아래에서 올라와 마지막에 미끄러진다

상대 트랙을 배치에 더해 해석한 결과다. `의자1` 기준이며 넷이 같은 모양이다.

```text
배치 [-295.93, -108.50, 465.19]
     0ms  오프셋 [0, 0, 0]           월드 y = -108.50   제자리
 11400ms  오프셋 [0, 187.45, 0]      월드 y =  +78.96   187m 위로 치워짐
 15030ms  오프셋 [0, 187.45, 0]      월드 y =  +78.96   유지
 16710ms  오프셋 [0, 175.02, 0]      월드 y =  +66.52   내려오기 시작
 17550ms  오프셋 [-5.09, 0, 0]       월드 y = -108.50   바닥에 닿고 x 로 5.09m 벗어남
 18080ms  오프셋 [0, 0, 0]           월드 y = -108.50   제자리로 미끄러짐
```

즉 의자는 11.4~17.5초 동안 화면 밖(187m 위)에 있다가 내려오고, **마지막 530ms에
5m 미끄러져 자리를 잡는다.** 녹화본에서 본 "끌리면서 붙는" 동작이 이 구간이다.
네 의자의 미끄러지는 거리는 각각 5.09 / 7.25 / 7.41 / 4.99m다.

상대 트랙을 해석하지 않고 오프셋 차이만 재면 187m 상승이 "이동"으로 잡힌다. 프레임을
먼저 확정해야 하는 이유다.

### 4.8 toggle / visibility / event

```text
toggle      53 트랙 전부 완전 해독   예: 0.00s etta_off | 1.34s etta_trigger | 11.95s etta_off
visibility   1 트랙  4키
event        2 트랙  19키 + 10키
```

이미터 13개(`card`, `card1`, `pow1~pow42`, `h1`, `pp1`)는 move 트랙이 없고 toggle만
갖는다. 카드 흩날림이 여기 들어간다.

### 4.9 조명

`pointlightmovable` 3개. `l1`은 아레나 `[-1.44, 2.28, 738.62]`에서 0~10.17초에
0.53m 움직이고, `l2`는 18.56초, `l3`은 18.20초에 키가 하나씩이다. 이 Area는 이미
`maplights.json` pair를 선언하므로 같은 레이어에 넣는다.

### 4.10 액터 링크가 없는 그룹 14개 — 복구 실패

`interpdata_2`의 그룹 14개는 추출본에서 `actor: null`이다. 원인을 확인했다.

```text
efseqact_matinee_2 에 variablelinks 속성이 없다
seqvar_object 406개는 objvalue(액터)만 갖고 그룹 이름 라벨이 없다
InterpGroupInst 는 런타임 객체라 패키지에 저장되지 않는다 (0개)
```

즉 **표준 Kismet 경로로는 이 패키지에서 그룹과 액터를 이을 수 없다.** 위치로
매칭하는 것도 실패했다. 이 14개의 move 트랙은 마지막 키가 대부분 `(0,0,0)`이라
상대 오프셋이고, 기준이 되는 배치를 모르면 월드 위치를 만들 수 없다.

```text
촛대1  최종 오프셋 [0, 0.71, 0]     기준 배치 불명
촛대2  최종 오프셋 [0, 0, 0]        기준 배치 불명
책 / 책1 / 책2 / 책11 / 책110 / cann1 / cann2 / tabrdum / ll / tab1 / loop / sk
```

화면에서 크게 보이는 양쪽 촛대가 여기 들어간다. `deco19` 메시를 든 `can1` / `can2`는
키가 하나뿐인 정지 배치이므로 **움직이는 촛대의 기준을 우리가 정해야 한다.**
이것은 복원이 아니라 저작이므로 별도 G로 분리한다.

### 4.11 대사와 사운드 — 원문 없음

```text
efinterptracksubtitle_0   msgtype(byte)  msgid(int)   문자열 없음
interptrackakevent_51     0.35초  scene_midnightc_ed_moveintosatonbook
interptrackakevent_34     0.45초  scene_midnightc_ed_movetocardmaze
interptrackakevent_50/52  AkEvent_BGM
```

자막 원문은 별도 문자열 테이블에 있고 이 패키지에 없다. 녹화본에서 읽은 대사는
`판이 깔렸으니, 신나게 놀아보자고!` 한 줄이다. 사운드는 Wwise 이벤트 참조라 파일이
아니다.

`msgid`가 `531`로 읽히는데 `names[531] = "none"`이다. `shotnumber`, `matineeindex`,
`objinstanceversion`도 전부 531로 읽힌다. 구조체 마지막 필드가 종료 태그와 겹쳐
읽히는 자리이므로 **이 값들은 신뢰하지 않는다.** 추출기는 `shotnumber`를 출력에서
제거한다.

### 4.12 카드는 파티클이지만 메시가 따로 있다

날아다니는 카드는 `par_q_cardfly_01`(3회)과 `par_q_rpct_exp_02`(9회) 이미터이고
umodel이 `ParticleSystem` 클래스를 열지 못한다. 그런데 추출본 `7_카드메시/`에
`card01`, `card01a`, `card01b`, `card01i`, `card01p` 다섯 종이 있고, 녹화본에서도
카드가 클로버·다이아 문양이 보이는 판형 메시로 날아간다.

우리 Effect 저작은 Mesh Particle family를 지원한다. 원본 파티클을 가져오는 것이
아니라 **추출한 카드 메시로 다시 저작**한다. 복원이 아니라 재현이므로 별도 G다.

### 4.13 녹화본 대조

`녹음 2026-09-05 122308.mp4`는 2분 18초 원작 방송분을 미디어 플레이어로 재생하며 찍은
10.87초 화면 녹화다(2560x1528, 30fps, 326프레임). 재생 위치가 0:18 → 0:27 → 0:33으로
건너뛰므로 **시각 측정에는 쓸 수 없고** 화면에 무엇이 나오는지 확인에만 썼다.

프레임에서 확인한 것과 데이터의 대응이다.

```text
초록 테이블이 원형으로 열림   bg_rad_koukusaton_table + evt2_table_open01
양쪽에 촛대                   can1/can2(정지) + 촛대1/촛대2(액터 불명)
의자 4개가 자리를 잡음        의자1~4, 4.7의 마지막 530ms 구간
카드가 폭발하듯 날림          par_q_cardfly_01 / par_q_rpct_exp_02
쿠크가 대사를 함              efinterptracksubtitle + akevent, 원문 없음
쿠크가 연기함                 4.5의 18.080초 이후 클립 6개
```

### 4.14 추출한 에셋

바탕화면 `쿠크_도박판컷신_에셋_20260904`, 372파일 1.88GB.

```text
bg_rad_koukusaton_table            스켈레탈 15.18 x 27.67m, 정점 50, 본 9개
                                   (bn, bone001~008), 머티리얼 floor15/15a/15b/16
bg_rad_koukusaton_table_evt2_ani   시퀀스 1개 = evt2_table_open01
bg_rad_koukusaton_chair01_sm_hht   의자
bg_rad_koukusaton_deco19_sm_hht    촛대
mn_rpcz_00_sk + 애님셋 3종          인형
7_카드메시/ card01, 01a, 01b, 01i, 01p
```

---

## 5. 설계도

### 5.1 데이터 흐름

```text
B9AVB...806.upk
   │  G1  extract_scene_matinee.py     (완전 해독 + declared==decoded 단언)
   ▼
Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/SCENE04A.interpdata_2.matinee.json
   │
   ├─ G3  카메라       ─▶ Data/Maps/Authoring/.../*.camerashots.json  shot.kouku.table
   ├─ G4  소품 배치     ─▶ Data/Maps/Imported/.../*.deployassets  (카탈로그 행)
   │                   ─▶ Data/Maps/Authoring/.../*.deployplacements (배치 행)
   ├─ G4  소품 움직임   ─▶ Data/Maps/Authoring/.../*.worldsequences.json  transform 트랙
   ├─ G4b 애니메이션    ─▶ 같은 문서의 animation 트랙 (startMs 체인)
   └─ G4c 자막         ─▶ Data/UI 계약 (원문은 우리가 작성)
                          │
                          ▼  Tools/MapPipeline/Publish-MapAuthoring.ps1 -Mode Publish
                       Client/Bin/DataFiles/Map/*
                          │
                          ▼  트리거 → Server → S2C_WORLD_SEQUENCE_PLAY
                       CLevel_KakulSaydonArena::Start_PopupBookCutscene 계열
```

원본 쿠킹 산출물(`.wmodel`, `.dds`)은 팀장 Drive의 `Client/Bin/Resources/Map/...`에
들어가며 Git이 추적하지 않는다.

### 5.2 원본 개념 → 우리 문서 매핑

| 원본 | 우리 문서 | 변환 규칙 |
|---|---|---|
| `InterpTrackMove` `moveframe=default` | `WORLD_SEQUENCE_TRANSFORM_KEY.positionOffset` | 월드 키에서 배치를 빼서 오프셋으로 만든다 |
| `InterpTrackMove` `imf_relativetoinitial` | 같은 필드 | 오프셋을 그대로 쓴다 |
| `eulertrack` (Roll, Pitch, Yaw) | `rotationQuaternion` | `build_maptool_scene.py`의 `convert_rotation` |
| `InterpTrackAnimControl` | `WORLD_SEQUENCE_ANIMATION_TRACK` | `starttime→startMs`, `animseqname→clipName`, `animplayrate→playbackRate`, `blooping→loop` |
| `breverse` | **새 필드 `reverse`** | 5.6 참조 |
| `InterpTrackToggle` / `Visibility` | `WORLD_SEQUENCE_TRANSFORM_KEY.visible` | `etta_off/evta_hide → false` |
| `InterpTrackDirector` 컷 | `VALTAN_CINEMATIC_CAMERA_CUE.Keyframes` | 5.3 참조 |
| `FOVAngle` (수평) | `fovYDegrees` (수직) | `2·atan(tan(fovX/2)/1.5)` |
| `InterpTrackFade` | (미사용) | 사용자가 "조명/암전 필요없다"고 결정 |
| `InterpTrackAkEvent` | (범위 밖) | Wwise, 파일 없음 |
| emitter + `par_*` | Effect 재저작 | 카드만, 5.7 참조 |

### 5.3 컷을 하나의 연속 트랙으로 표현하는 방법

우리 `VALTAN_CINEMATIC_CAMERA_CUE`는 키프레임 하나의 목록이고 컷 개념이 없다.
`LINEAR` 보간이므로 **같은 시각에 가까운 두 키를 두면 사실상 즉시 전환**이 된다.

```text
컷 시각 t 에서
   키 A : timeMs = t - 1,  이전 카메라가 t 에서 갖는 자세
   키 B : timeMs = t,      다음 카메라가 t 에서 갖는 자세
```

1ms 구간에서 보간이 일어나므로 30fps에서 한 프레임 안에 끝난다. 새 스키마 필드가
필요 없고 기존 샘플러를 그대로 쓴다.

키 예산은 다음과 같이 맞는다.

```text
cam1  11키    cam2  15키    cam3  5키    cam4  9키    cam6  3키   = 43
컷 경계 5곳에 각 1키 추가                                        = 48
상한 CAMERA_TRACK_MAX_KEYFRAMES = 64
```

**48 ≤ 64이므로 팝업북에서 쓴 오차 기준 솎기가 필요 없다.** 원본 키를 그대로 옮긴다.

구간 밖 카메라의 자세는 그 카메라 트랙을 그 시각에 평가해 얻는다. `cam3`의 창은
12370부터인데 컷은 13950이므로 창 안이다. `cam6`의 창은 23740부터이고 컷은 23950이라
역시 창 안이다. 모든 컷이 대상 카메라의 창 안에 있음은 4.2에서 확인했다.

`lookAt`은 원본에 없다. 카메라의 `eulertrack`이 (Roll, Pitch, Yaw)를 주므로
전방 벡터를 만들고 `eye + forward * d`를 `lookAt`으로 쓴다. `d`는 표현에 영향이 없는
임의 거리이며 1m로 고정한다.

### 5.4 소품 배치와 좌표

의자·촛대의 배치는 원작 그대로 지하 무대(`y ≈ -108.5`)에 둘 수 없다. 우리 아레나에는
지하 무대가 없기 때문이다. 두 가지 중 하나를 고른다.

```text
방법 A  대기 무대를 통째로 아레나 위로 올린다
        모든 대기쪽 배치와 cam3/cam4/cam6 키에 같은 평행이동을 더한다
        평행이동 = (아레나 중심) - (대기 무대 중심)
        장점: 원본 구도가 그대로 유지된다
        단점: 아레나 기존 배치와 겹치면 컷신 동안 숨겨야 한다

방법 B  대기 무대를 그 자리에 두고 컷신 동안만 카메라를 보낸다
        배치와 카메라 키를 원본 좌표 그대로 쓴다
        장점: 변환이 없다. 겹침이 없다
        단점: 맵 밖 좌표에 소품이 생기므로 navigation/컬링 확인이 필요하다
```

팝업북은 방법 A를 썼고("컷신 세트를 아레나 바닥에 정렬했다") 아레나 배치 461개를
숨기는 목록이 이미 있다. **방법 A를 기본으로 하되, 평행이동 값은 G4에서 실측으로
정하고 원본 구도가 유지되는지 절두체 검사로 확인한다.**

### 5.5 transform 키 생성 규칙

```text
for 각 활성 move 트랙:
    if disabled: 건너뛴다                       # 36개가 여기서 빠진다
    if moveFrame == 'imf_relativetoinitial':
        positionOffset = 키 값 그대로
    else:                                       # 월드
        positionOffset = 키 값 - 배치 위치
    rotationQuaternion = convert_rotation(eulertrack 같은 시각 값)
    visible = 같은 시각의 toggle/visibility 상태
```

`cim_constant` 구간은 값을 유지하다 다음 키에서 순간이동한다. 우리 런타임은 키 사이를
보간하므로, **constant 구간의 끝에 같은 값의 키를 하나 더 넣어** 순간이동을 보존한다.
팝업북에서 이 처리를 하지 않았다면 소품이 미끄러져 들어왔을 것이다.

### 5.6 애니메이션 체인과 역재생

한 슬롯의 클립들은 `startMs` 오름차순 체인으로 넣는다. 슬롯은 원본의 `slotname`
(`a` / `b`)을 그대로 쓰되, 우리 계약의 첫 트랙은 `startMs = 0`이어야 하므로 음수 시각
클립은 `startMs = 0`으로 당기고 `animstartoffset`에 해당하는 만큼 클립 내부에서
앞당겨 시작한다.

역재생 클립 4개는 현재 필드가 없다. 다음을 추가한다.

```text
파일   Client/Public/WorldSequenceDocument.h
작업   WORLD_SEQUENCE_ANIMATION_TRACK 의 holdLastFrame 바로 아래에 추가
추가   bool_t reverse = false;
이유   evt2_book01 3회와 evt2_table_open01 1회가 역재생이다
소비자 CWorldSequencePlayer 의 deploy 분기에서 샘플 시각을 뒤집는다
검증   WorldSequenceDocument.cpp 파서/작성기, composition_pipeline.py,
       Publish-MapAuthoring.ps1 세 검증기에 optional 필드로 추가
```

### 5.7 카드 흩날림

`par_q_cardfly_01`을 그대로 가져올 수 없으므로 Effect Tool에서 Mesh Particle로
재저작한다. 입력은 추출본의 카드 메시 5종이고, 발생 시각과 지속은 원본의 toggle
트랙에서 가져온다. 이것은 재현이며 원본과 1:1 일치를 주장하지 않는다.

### 5.8 런타임 호출 흐름

```text
2-1Stage_Move 와 같은 triggerBox
   → CServerTriggerSystem::Evaluate_Entries
   → CGameRoom::Broadcast_WorldSequencePlay(instanceId)
        └ Place_PartyForCutscene(instanceId)      도박판 분기 추가
   → S2C_WORLD_SEQUENCE_PLAY
   → CClientReplication
   → CLevel_KakulSaydonArena::Start_ServerRequestedSequence
        └ 도박판 instanceId 이면 Start_GamblingTableCutscene
             ├ Apply_CutsceneSetVisible(true)     아레나 숨김 461개 ↔ 컷신 세트
             ├ 소품 인스턴스 전부 Play
             └ shot.kouku.table 카메라 트랙 시작
   → 매 프레임 Update_CutsceneRetire
        └ 시퀀스가 멈추면 Apply_CutsceneSetVisible(false) + 소품 DESPAWNED
```

`Start_PopupBookCutscene`과 같은 모양이므로 두 컷신이 한 진입 함수의 분기가 되도록
공통화한다. 두 번째 진입 경로를 만들지 않는다.

### 5.9 스키마 변경 요약

```text
추가  WORLD_SEQUENCE_ANIMATION_TRACK::reverse (bool, 기본 false)
      소비자: WorldSequenceDocument.cpp, WorldSequencePlayer.cpp,
              composition_pipeline.py, Publish-MapAuthoring.ps1
없음  카메라 샷은 기존 필드로 컷을 표현한다 (5.3)
없음  toggle/visibility 는 기존 visible 필드로 표현한다
```

---

## 6. 팝업북에서 3일을 쓴 실패와 이번에 막는 장치

### 6.1 부분 해독을 정상 데이터로 믿었다

**게이트: 추출기는 모든 배열에서 `declared == decoded`를 단언하고, 어긋나면 예외로
중단해 파일을 쓰지 않는다.** 부분 해독 문서가 생기지 않는 것이 이 단계의 전부다.

### 6.2 다른 도구의 검수 문구를 그대로 믿었다

추출본 README는 animcontrol이 완전히 읽힌다고 적었지만 절반이 빠져 있었다(3.2).

**게이트: 추출기는 move / animcontrol도 직접 읽고, 기존 추출본과 트랙 수·키 수를
대조해 출력에 남긴다.** move 111/641이 일치하고 animcontrol만 37 대 18로 갈리는 것을
확인했다.

### 6.3 카메라 기준계를 고정으로 가정했다

**게이트: 프레임은 `moveframe` 태그로 판정하고, 자기 위치를 가진 액터의 첫 키가 그
위치에서 몇 m 떨어져 있는지로 검산한다.** 4.2에서 cam2/cam3/cam6이 1.34 / 6.77 /
1.84m로 나왔으므로 월드로 확정했다.

### 6.4 FOV 축을 바꾸지 않았다

**게이트: 변환은 추출기 안에서 한 번만 하고 `fovXDegrees`와 `fovYDegrees`를 함께
기록한다.**

### 6.5 검증 지표가 틀렸다

보스를 점으로 보고 조준 오차를 재서 정상 프레임을 실패로 판정했다.

**게이트: 판정은 절두체 포함률로 한다.** 테이블은 15.18 x 27.67m 판이므로 사각 볼륨을
두고 각 키프레임에서 몇 %가 절두체 안에 들어오는지 센다.

### 6.6 데이터에 없는 보정을 발명했다

**게이트: 카메라 키의 모든 값은 추출기가 낸 트랙에서 나온다.** 코드에 상수 회전이나
오프셋을 넣지 않는다. 사람이 만지는 것은 MapTool의 키 편집이고 그 결과는
`camerashots.json`에 남는다.

### 6.7 publisher를 Validate로 돌리고 결과를 믿었다

**게이트: 모든 publish는 `-Mode Publish`로 실행하고 receipt 생성 시각을 확인한 뒤에만
다음 단계로 간다.**

### 6.8 MapTool에서만 되고 제품 레벨에서 깨졌다

**게이트: 종료 증거는 MapTool 재생이 아니라 트리거를 밟은 제품 레벨 결과다.**

### 6.9 import 스크립트를 저장소에 남기지 않았다

**게이트: 추출기는 `Tools/KakulSaydonPipeline/`에 테스트와 함께 커밋한다.** 남은 씬
6개(SCENE01B, 01C, 02A, 02B, 06A, 07A)도 같은 도구로 처리한다.

### 6.10 꺼진 트랙을 못 보고 전부 가져왔다

move 트랙의 3분의 1이 `bDisableTrack`이다(2.2).

**게이트: 추출기는 모든 트랙에 `disabled`를 기록하고, 변환기는 꺼진 트랙을 건너뛴다.**

### 6.11 상대 트랙을 월드로 읽었다

의자의 187m는 이동이 아니라 지하 대기 위치에서의 오프셋이다(4.7).

**게이트: 변환기는 `moveFrame`을 보고 분기하고, 변환 후 마지막 키의 월드 위치가
배치값과 일치하는지 소품마다 검사한다.** 팝업북이 158개 중 150개로 확인한 것과 같은
검사다.

### 6.12 그룹은 있는데 액터가 없는 것을 못 본 채로 넘어갔다

**게이트: G4 시작 전에 액터 미연결 그룹 14개를 목록으로 만들고 각각 연결 메시를
찾았는지 명시한다.** 못 찾은 것은 구현하지 않고 RESULT에 남긴다. 추측으로 메시를
붙이지 않는다.

---

## 7. G 구성과 종료 증거

| G | 내용 | 종료 증거 |
|---|---|---|
| G1 | 원본 패키지 완전 해독 도구와 테스트 | 모든 배열 `declared == decoded`; 산출 JSON; 테스트 통과; 기존 추출본과 move 111/641 일치 |
| G2 | 테이블·의자·촛대 쿠킹 | `.wmodel` 생성, 카탈로그 admission 통과 |
| G3 | 카메라 트랙 저작 | 키 48개 이하; 컷 5곳이 1ms 쌍으로 들어감; 각 키에서 테이블 볼륨 절두체 포함률 확인 |
| G4 | 소품 배치와 transform 트랙 | 꺼진 트랙 36개 제외; 상대/월드 분기 적용; 마지막 키가 배치값과 일치 |
| G4b | 애니메이션 체인과 `reverse` 필드 | 클립 37개가 원본 시각·배속으로 재생; 세 검증기 통과 |
| G4c | 대사 자막 | 지정 시각에 뜨고 사라짐 |
| G5 | 트리거·파티 배치·정리를 제품 레벨에 연결 | 트리거를 밟아 제품 레벨에서 재생되고 끝나면 세트가 정리됨 |
| G5b | 카드 흩날림 Effect 재저작 | 카드가 테이블 위로 날고 지정 시각에 사라짐 |
| G6 | 촛대 기준 저작, 남은 파티클 17종, Wwise 사운드 | RESULT에 구현/미구현 분리 기록 |

G2 이전에 사용자 결정이 필요한 항목은 8절에 모았다.

---

## 8. 사용자 결정이 필요한 것

1. **대기 무대 처리** — 5.4의 방법 A(아레나 위로 평행이동)와 방법 B(원본 좌표 유지)
   중 선택. 기본은 A다.
2. **대사 문구** — 원문이 패키지에 없다. 녹화본에서 읽은
   `판이 깔렸으니, 신나게 놀아보자고!`를 쓸지 다른 문구를 쓸지.
3. **촛대** — 움직이는 촛대의 기준 배치를 우리가 정해야 한다(4.10). 원작 화면을 보고
   위치를 잡을지, 아니면 `can1`/`can2` 정지 배치만 쓰고 움직임을 뺄지.
4. **쿠크 연기 범위** — 18.080초 이후 슬로모션 클립 6개를 넣을지. 넣으려면 쿠크 모델과
   `att_battle_*` 클립이 있어야 한다.

---

## 9. 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/USER/source/졸업팀폴/LostArk/Tools/KakulSaydonPipeline/extract_scene_matinee.py` | 원본 패키지를 완전 해독해 JSON으로 낸다 |
| 추가 | `C:/Users/USER/source/졸업팀폴/LostArk/Tools/KakulSaydonPipeline/test_extract_scene_matinee.py` | 완전 해독 단언과 좌표·FOV 변환 회귀 |
| 생성 | `C:/Users/USER/source/졸업팀폴/LostArk/Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/SCENE04A.interpdata_2.matinee.json` | G1 산출물. G3~G4c의 입력 |

G1은 C++ 파일을 추가하지 않으므로 `.vcxproj` / `.vcxproj.filters` 등록이 없다.
G4b의 `reverse` 필드는 기존 파일 수정이므로 역시 등록이 없다.

---

## 10. 파일별 전체 구현 코드

### 10-1. `C:/Users/USER/source/졸업팀폴/LostArk/Tools/KakulSaydonPipeline/extract_scene_matinee.py`

변경 종류: 추가

이 파일이 존재하는 이유는 바탕화면 추출본이 컷·fade·FOV·toggle을 부분 해독으로 남기고
animcontrol 클립 절반과 재생 필드 전부를 잃었기 때문이다. 이 도구는 패키지를 직접 열고,
배열마다 선언된 요소 수와 실제로 읽은 수가 같은지 단언하며, 하나라도 어긋나면 파일을
쓰지 않고 중단한다. 좌표와 화각 변환도 여기서 한 번만 수행해 소비자가 축을 다시 고르지
않게 한다.

함수별 한 줄 책임은 다음과 같다.

```text
Package.name_at        지정 위치의 FName 을 이름 문자열로 읽는다
Package.head_bytes     요소 선두 태그 두 개를 바이트 패턴으로 만든다
tagged_properties      한 위치에서 종료 태그까지 속성 목록을 읽는다
find_property          한 오브젝트 전체에서 이름·타입이 맞는 속성 값 위치를 찾는다
scalar_value           태그 타입에 맞는 스칼라 또는 12바이트 벡터를 읽는다
read_struct_array      요소 선두 태그로 경계를 잡아 배열 전 요소를 읽고 declared 와 대조한다
drop_suspect_fields    종료 태그와 겹쳐 읽히는 상수 필드를 제거한다
object_array           객체 참조 배열을 export 인덱스 목록으로 읽는다
is_disabled            bDisableTrack 이 기록돼 있는지로 트랙 활성 여부를 판정한다
cut_keys               director 트랙의 컷 목록
curve_keys             FInterpCurve 의 points, 중첩된 floattrack 도 처리한다
switch_keys            toggle / visibility / event 키
vector_curve           postrack / eulertrack 을 우리 축으로 읽는다
anim_sequences         클립, 시각, 배속, 루프, 역재생, 구간 오프셋
to_our_position        UE3 cm/Z-up 을 우리 m/Y-up 으로
horizontal_to_vertical_fov  UE3 수평 화각을 우리 수직 화각으로
read_track             한 트랙을 클래스에 맞는 레코드로 만든다
collect                InterpData → 그룹 → 트랙을 모으고 재생될 director 를 고른다
main                   인자를 받아 산출 JSON 을 쓴다
```

불변식 네 가지다. 첫째, 배열은 `declared == len(decoded)`가 아니면 예외다. 둘째, 좌표와
화각 변환은 이 파일 밖에서 다시 하지 않는다. 셋째, 모든 트랙이 `disabled`를 기록하므로
소비자가 꺼진 트랙을 모르고 쓰는 일이 없다. 넷째, 재생되는 director 는 저장 순서에서
`bDisableTrack` 이 꺼진 첫 트랙이며 이는 엔진 규칙과 같다.

```python
"""Read a Lost Ark cutscene matinee completely, or fail.

The shipped desktop export stopped after the first element of every array
property, so the camera cut order, the screen fades, the field-of-view curves
and the toggles that reveal each prop were unknown while still looking like
real values. Those tracks are what decide the shot, so every array is walked
element by element and a document is only written when each array's declared
element count matches the number actually decoded.

Ownership is read the same way the engine reads it. InterpData names its
groups, a group names its tracks, and UInterpGroupDirector plays the first
track in that order whose bDisableTrack is not set, which is how one cut list
is chosen out of the several a scene can carry.

Coordinates and the field-of-view axis are converted here once. UE3 stores
centimetres with Z up and a horizontal FOVAngle; the runtime documents use
metres with Y up and a vertical fovYDegrees, and mixing the two up cost days
on the pop-up book cutscene.
"""
from __future__ import annotations

import argparse
import io
import json
import math
import os
import struct
import sys
from typing import Any

# The package reader lives with the original extraction tools.
_TOOLS_ENV = 'LOSTARK_SCENE_TOOLS'
_DEFAULT_TOOLS = os.path.join(
    os.path.expanduser('~'), 'OneDrive', '바탕 화면',
    '쿠크_컷신_전체추출_20260904', '추출도구')

# UE3 authored these scenes for a 3:2 viewport, and FOVAngle is horizontal.
SOURCE_ASPECT = 1.5
# Every element of an array of tagged structs opens with one of these pairs,
# which is how element boundaries are found without trusting a terminator.
CUT_HEAD_FIELDS = ('time', 'floatproperty')
CURVE_HEAD_FIELDS = ('inval', 'floatproperty')
ANIM_HEAD_FIELDS = ('starttime', 'floatproperty')
# A property tag is name(8) + type(8) + size(4) + arrayIndex(4).
TAG_HEADER_BYTES = 24
# Fields whose decoded value is the name id of 'none' in every element carry
# no information; shotnumber is the one this scene family writes that way.
SUSPECT_CONSTANT_FIELDS = ('shotnumber',)


def _load_package_reader() -> Any:
    tools = os.environ.get(_TOOLS_ENV, _DEFAULT_TOOLS)
    if tools not in sys.path:
        sys.path.insert(0, tools)
    try:
        import la_upk  # type: ignore
    except ImportError as exc:
        raise SystemExit(
            'la_upk.py not found. Set %s to the folder that holds it '
            '(looked in %s).' % (_TOOLS_ENV, tools)) from exc
    return la_upk


class Package:
    """One decrypted package plus the tables needed to read its objects."""

    def __init__(self, path: str) -> None:
        la_upk = _load_package_reader()
        self.data, _flags, _header = la_upk.load(path)
        self.names, self.imports, self.exports = la_upk.tables(self.data)
        self.name_id = {name: index for index, name in enumerate(self.names)}
        self.none_id = self.name_id.get('none', -1)

    def name_at(self, offset: int) -> str:
        index, number = struct.unpack_from('<2I', self.data, offset)
        base = self.names[index] if index < len(self.names) else '<%d>' % index
        return base if number == 0 else '%s_%d' % (base, number - 1)

    def head_bytes(self, first: str, second: str) -> bytes:
        return (struct.pack('<2I', self.name_id[first], 0) +
                struct.pack('<2I', self.name_id[second], 0))

    def export_end(self, export: dict) -> int:
        return export['offset'] + export['size']


def tagged_properties(pkg: Package, start: int, end: int) -> list[tuple]:
    """Tagged properties as (name, type, size, valueOffset, nextOffset).

    Stops at the 'none' terminator or at the first tag that cannot be a
    property, so a caller can tell a clean run from noise. A bool stores its
    value in one byte after the header and declares size zero.
    """
    rows: list[tuple] = []
    offset = start
    while offset + TAG_HEADER_BYTES <= end:
        pname = pkg.name_at(offset)
        if pname.lower() == 'none':
            break
        ptype = pkg.name_at(offset + 8)
        if not ptype.endswith('property'):
            break
        size, _array_index = struct.unpack_from('<2i', pkg.data, offset + 16)
        body = offset + TAG_HEADER_BYTES
        if ptype in ('structproperty', 'byteproperty'):
            body += 8                      # the struct or enum name follows
        if size < 0 or body + size > end:
            break
        width = 1 if ptype == 'boolproperty' else size
        if body + width > end:
            break
        rows.append((pname, ptype, size, body, body + width))
        offset = body + width
    return rows


def find_property(pkg: Package, export: dict, wanted_name: str,
                  wanted_type: str) -> tuple[int, int] | None:
    """Locate one property's (valueOffset, size) anywhere in the object.

    Each UE3 class writes a different amount of native data before its tagged
    properties and some properties sit after a long array, so the whole object
    is scanned on four byte steps rather than only its first bytes.
    """
    start = export['offset']
    end = pkg.export_end(export)
    offset = start
    while offset + TAG_HEADER_BYTES <= end:
        if pkg.name_at(offset) == wanted_name and \
                pkg.name_at(offset + 8) == wanted_type:
            size, _array_index = struct.unpack_from('<2i', pkg.data,
                                                    offset + 16)
            body = offset + TAG_HEADER_BYTES
            if wanted_type in ('structproperty', 'byteproperty'):
                body += 8
            width = 1 if wanted_type == 'boolproperty' else size
            if size >= 0 and body + width <= end:
                return body, size
        offset += 4
    return None


def scalar_value(pkg: Package, ptype: str, size: int, offset: int) -> Any:
    if ptype == 'floatproperty' and size == 4:
        return round(struct.unpack_from('<f', pkg.data, offset)[0], 6)
    if ptype == 'intproperty' and size == 4:
        return struct.unpack_from('<i', pkg.data, offset)[0]
    if ptype == 'nameproperty' and size == 8:
        return pkg.name_at(offset)
    if ptype == 'byteproperty' and size == 8:
        return pkg.name_at(offset)
    if ptype == 'boolproperty':
        return pkg.data[offset] != 0
    if ptype == 'structproperty' and size == 12:
        return [round(v, 6) for v in struct.unpack_from('<3f', pkg.data,
                                                        offset)]
    return None


def read_struct_array(pkg: Package, body: int, size: int,
                      head: tuple[str, str], label: str) -> list[dict]:
    """Every element of an array of tagged structs, or raise.

    Element starts are found by the pair of tags each element opens with,
    because the shipped reader mis-walked the terminator and silently stopped
    after element zero.
    """
    declared = struct.unpack_from('<i', pkg.data, body)[0]
    if declared < 0:
        raise ValueError('%s: negative element count %d' % (label, declared))
    if declared == 0:
        return []
    end = body + size
    pattern = pkg.head_bytes(head[0], head[1])
    starts: list[int] = []
    probe = body + 4
    while True:
        found = pkg.data.find(pattern, probe, end)
        if found < 0:
            break
        starts.append(found)
        probe = found + 8
    rows: list[dict] = []
    for element_start in starts:
        fields: dict[str, Any] = {}
        for pname, ptype, psize, voffset, _next in tagged_properties(
                pkg, element_start, end):
            value = scalar_value(pkg, ptype, psize, voffset)
            if value is None:
                break
            fields[pname] = value
        if fields:
            rows.append(fields)
    if len(rows) != declared:
        raise ValueError('%s: declared %d elements but decoded %d'
                         % (label, declared, len(rows)))
    return rows


def drop_suspect_fields(pkg: Package, rows: list[dict]) -> list[dict]:
    """Remove fields that decode to the name id of 'none' in every element.

    UE3 writes a struct's last property immediately before the element
    terminator, and this build's director cut writes a shotnumber whose bytes
    read back as that terminator's name id in every cut of every track. A
    field that never varies and equals that id is noise, not a shot number.
    """
    cleaned = []
    for row in rows:
        copy = dict(row)
        for field in SUSPECT_CONSTANT_FIELDS:
            if field in copy and copy[field] == pkg.none_id:
                del copy[field]
        cleaned.append(copy)
    return cleaned


def object_array(pkg: Package, export: dict, wanted: str) -> list[int]:
    """An array of object references, as export indices."""
    found = find_property(pkg, export, wanted, 'arrayproperty')
    if not found:
        return []
    body, size = found
    count = struct.unpack_from('<i', pkg.data, body)[0]
    if count <= 0 or size - 4 < count * 4:
        return []
    return list(struct.unpack_from('<%di' % count, pkg.data, body + 4))


def is_disabled(pkg: Package, export: dict) -> bool:
    """UE3 omits a bool at its default, so a written bDisableTrack means on."""
    found = find_property(pkg, export, 'bdisabletrack', 'boolproperty')
    if not found:
        return False
    body, _size = found
    return pkg.data[body] != 0


def cut_keys(pkg: Package, export: dict) -> list[dict]:
    """The camera cut list of one InterpTrackDirector."""
    found = find_property(pkg, export, 'cuttrack', 'arrayproperty')
    if not found:
        return []
    body, size = found
    label = 'cuttrack of %s' % export['name']
    rows = drop_suspect_fields(
        pkg, read_struct_array(pkg, body, size, CUT_HEAD_FIELDS, label))
    return [{'timeMs': int(round(r.get('time', 0.0) * 1000.0)),
             'transitionMs': int(round(r.get('transitiontime', 0.0) * 1000.0)),
             'camera': r.get('targetcamgroup')} for r in rows]


def curve_keys(pkg: Package, export: dict, container: str,
               container_type: str) -> list[dict]:
    """The points of one FInterpCurve, whether it is nested or not.

    A fade track exposes 'points' directly; a float property track wraps the
    same curve in a 'floattrack' struct, so the struct is opened first and
    'points' is searched inside it.
    """
    found = find_property(pkg, export, container, container_type)
    if not found:
        return []
    body, size = found
    if container_type == 'arrayproperty':
        points_body, points_size = body, size
    else:
        points_body = None
        points_size = 0
        end = body + size
        offset = body
        while offset + TAG_HEADER_BYTES <= end:
            if pkg.name_at(offset) == 'points' and \
                    pkg.name_at(offset + 8) == 'arrayproperty':
                points_size, _array_index = struct.unpack_from(
                    '<2i', pkg.data, offset + 16)
                points_body = offset + TAG_HEADER_BYTES
                break
            offset += 4
        if points_body is None:
            return []
    label = '%s of %s' % (container, export['name'])
    rows = read_struct_array(pkg, points_body, points_size,
                             CURVE_HEAD_FIELDS, label)
    return [{'timeMs': int(round(r.get('inval', 0.0) * 1000.0)),
             'value': r.get('outval'),
             'arrive': r.get('arrivetangent', 0.0),
             'leave': r.get('leavetangent', 0.0),
             'mode': r.get('interpmode')} for r in rows]


def switch_keys(pkg: Package, export: dict, container: str) -> list[dict]:
    """Toggle, visibility and event keys all share the cut element head."""
    found = find_property(pkg, export, container, 'arrayproperty')
    if not found:
        return []
    body, size = found
    label = '%s of %s' % (container, export['name'])
    rows = read_struct_array(pkg, body, size, CUT_HEAD_FIELDS, label)
    keys = []
    for row in rows:
        key = {'timeMs': int(round(row.get('time', 0.0) * 1000.0))}
        for field in ('toggleaction', 'action', 'activecondition',
                      'eventname'):
            if field in row:
                key[field] = row[field]
        keys.append(key)
    return keys


def to_our_position(x: float, y: float, z: float) -> list[float]:
    """UE3 centimetres with Z up become our metres with Y up."""
    return [round(x * 0.01, 6), round(z * 0.01, 6), round(-y * 0.01, 6)]


def horizontal_to_vertical_fov(fov_x_degrees: float,
                               aspect: float = SOURCE_ASPECT) -> float:
    """UE3 FOVAngle is horizontal; our documents store a vertical angle."""
    if fov_x_degrees <= 0.0 or fov_x_degrees >= 180.0:
        return fov_x_degrees
    half = math.radians(fov_x_degrees) * 0.5
    return round(math.degrees(2.0 * math.atan(math.tan(half) / aspect)), 4)


def vector_curve(pkg: Package, export: dict, container: str) -> list[dict]:
    """One FInterpCurveVector as our own axes.

    UE3 keeps position and rotation as separate curves on the same track, and
    a constant segment holds its value and then jumps, which is how a prop
    arrives from the staging copy of the set.
    """
    found = find_property(pkg, export, container, 'structproperty')
    if not found:
        return []
    body, size = found
    end = body + size
    points_body = None
    points_size = 0
    offset = body
    while offset + TAG_HEADER_BYTES <= end:
        if pkg.name_at(offset) == 'points' and                 pkg.name_at(offset + 8) == 'arrayproperty':
            points_size, _array_index = struct.unpack_from(
                '<2i', pkg.data, offset + 16)
            points_body = offset + TAG_HEADER_BYTES
            break
        offset += 4
    if points_body is None:
        return []
    label = '%s of %s' % (container, export['name'])
    rows = read_struct_array(pkg, points_body, points_size,
                             CURVE_HEAD_FIELDS, label)
    keys = []
    for row in rows:
        value = row.get('outval') or [0.0, 0.0, 0.0]
        key = {'timeMs': int(round(row.get('inval', 0.0) * 1000.0)),
               'mode': row.get('interpmode')}
        if container == 'postrack':
            key['value'] = to_our_position(*value)
        else:
            # Euler comes out of FRotator::Euler as (Roll, Pitch, Yaw).
            key['rollPitchYawDegrees'] = value
        keys.append(key)
    return keys


def anim_sequences(pkg: Package, export: dict) -> list[dict]:
    """Clip, start time and the playback fields the shipped export dropped."""
    found = find_property(pkg, export, 'animseqs', 'arrayproperty')
    if not found:
        return []
    body, size = found
    label = 'animseqs of %s' % export['name']
    rows = read_struct_array(pkg, body, size, ANIM_HEAD_FIELDS, label)
    keys = []
    for row in rows:
        keys.append({
            'timeMs': int(round(row.get('starttime', 0.0) * 1000.0)),
            'clip': row.get('animseqname'),
            'startOffsetMs': int(round(row.get('animstartoffset', 0.0) * 1000.0)),
            'endOffsetMs': int(round(row.get('animendoffset', 0.0) * 1000.0)),
            'playRate': row.get('animplayrate', 1.0),
            'looping': bool(row.get('blooping', False)),
            'reverse': bool(row.get('breverse', False)),
            'rootMotion': bool(row.get('benablerootmotion', False)),
        })
    return keys


TRACK_READERS = {
    'interptracktoggle': ('toggle', 'toggletrack'),
    'interptrackvisibility': ('visibility', 'visibilitytrack'),
    'interptrackevent': ('event', 'eventtrack'),
}


def read_track(pkg: Package, export: dict) -> dict | None:
    """One track as a record, with every array fully decoded."""
    class_name = export['className']
    # bDisableTrack is written only when set, and it is set on move tracks
    # too, so a track that exists is not automatically a track that plays.
    record = {'track': export['name'], 'class': class_name,
              'offset': export['offset'], 'disabled': is_disabled(pkg, export)}
    if class_name == 'interptrackdirector':
        record['cuts'] = cut_keys(pkg, export)
        return record
    if class_name in ('interptrackfade', 'interptrackslomo',
                      'interptrackcolorscale'):
        keys = curve_keys(pkg, export, 'points', 'arrayproperty')
        if not keys:
            keys = curve_keys(pkg, export, 'floattrack', 'structproperty')
        record['keys'] = keys
        return record
    if class_name == 'interptrackfloatprop':
        named = find_property(pkg, export, 'propertyname', 'nameproperty')
        if not named:
            return None
        record['property'] = pkg.name_at(named[0])
        keys = curve_keys(pkg, export, 'floattrack', 'structproperty')
        if 'fov' in record['property'].lower():
            for key in keys:
                key['fovXDegrees'] = key['value']
                key['fovYDegrees'] = horizontal_to_vertical_fov(key['value'])
        record['keys'] = keys
        return record
    if class_name == 'interptrackmove':
        frame = find_property(pkg, export, 'moveframe', 'byteproperty')
        # Written only when it differs from the class default, so an absent
        # tag and a present one are different contracts and both are recorded.
        record['moveFrame'] = pkg.name_at(frame[0]) if frame else 'default'
        record['position'] = vector_curve(pkg, export, 'postrack')
        record['euler'] = vector_curve(pkg, export, 'eulertrack')
        return record
    if class_name == 'interptrackanimcontrol':
        slot = find_property(pkg, export, 'slotname', 'nameproperty')
        record['slot'] = pkg.name_at(slot[0]) if slot else None
        record['clips'] = anim_sequences(pkg, export)
        return record
    if class_name in TRACK_READERS:
        bucket, container = TRACK_READERS[class_name]
        record['kind'] = bucket
        record['keys'] = switch_keys(pkg, export, container)
        return record
    return None


def collect(pkg: Package) -> list[dict]:
    """Sequences, their groups and the tracks this reader understands."""
    sequences = []
    for index, export in enumerate(pkg.exports, start=1):
        if export['className'] != 'interpdata':
            continue
        groups = []
        for group_ref in object_array(pkg, export, 'interpgroups'):
            if not 0 < group_ref <= len(pkg.exports):
                continue
            group_export = pkg.exports[group_ref - 1]
            tracks = []
            for track_ref in object_array(pkg, group_export, 'interptracks'):
                if not 0 < track_ref <= len(pkg.exports):
                    continue
                record = read_track(pkg, pkg.exports[track_ref - 1])
                if record:
                    tracks.append(record)
            groups.append({'group': group_export['name'],
                           'class': group_export['className'],
                           'tracks': tracks})
        # UE3 plays the first director track that is not disabled.
        live = None
        for group in groups:
            if group['class'] != 'interpgroupdirector':
                continue
            for track in group['tracks']:
                if track['class'] != 'interptrackdirector':
                    continue
                if not track.get('disabled'):
                    live = {'group': group['group'], 'track': track['track'],
                            'offset': track['offset'], 'cuts': track['cuts']}
                    break
            if live:
                break
        sequences.append({'data': export['name'], 'groupCount': len(groups),
                          'liveDirectorTrack': live, 'groups': groups})
    return sequences


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description='Fully decode a Lost Ark cutscene matinee package.')
    parser.add_argument('--package', required=True,
                        help='path to the scene .upk')
    parser.add_argument('--output', required=True,
                        help='path of the JSON document to write')
    parser.add_argument('--label', default='',
                        help='logical scene name recorded in the document')
    args = parser.parse_args(argv)

    pkg = Package(args.package)
    sequences = collect(pkg)

    document = {
        'schema': 'lostark.scene-matinee',
        'formatVersion': 2,
        'package': os.path.basename(args.package),
        'label': args.label or os.path.basename(args.package),
        'coordinateSystem': 'metres, Y up; UE3 (x, y, z) cm mapped to '
                            '(x*0.01, z*0.01, -y*0.01)',
        'fovNote': 'fovXDegrees is the UE3 horizontal FOVAngle; fovYDegrees '
                   'is the vertical angle our camera documents use, at '
                   'aspect %.3f' % SOURCE_ASPECT,
        'directorNote': 'liveDirectorTrack is the first track of the director '
                        'group whose bDisableTrack is not set, which is the '
                        'one UInterpGroupDirector plays.',
        'sequences': sequences,
    }

    with io.open(args.output, 'w', encoding='utf-8') as handle:
        json.dump(document, handle, ensure_ascii=False, indent=1)
        handle.write('\n')

    for sequence in sequences:
        live = sequence['liveDirectorTrack']
        print('%-14s groups=%-4d live director: %s' % (
            sequence['data'], sequence['groupCount'],
            live['track'] if live else 'none'))
        if live:
            print('    %s' % ' | '.join(
                '%.2fs %s' % (c['timeMs'] / 1000.0, c['camera'])
                for c in live['cuts']))
    print('written: %s' % args.output)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
```

### 10-2. `C:/Users/USER/source/졸업팀폴/LostArk/Tools/KakulSaydonPipeline/test_extract_scene_matinee.py`

변경 종류: 추가

패키지가 없는 PC 에서도 돌아야 하므로 순수 변환 함수와 배열 판독 규칙만 검사한다.
배열 검사는 실제 UE3 바이트 배치를 그대로 만든 가짜 패키지로 수행한다.

```python
"""Regression for the scene matinee reader.

The reader exists because a partial decode looked like real data, so the test
that matters is the one that proves a short array raises instead of writing a
plausible document. Coordinate and field-of-view conversion are checked here
too because both were wrong once in the pop-up book cutscene.
"""
from __future__ import annotations

import math
import os
import struct
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import extract_scene_matinee as ext  # noqa: E402


class FakePackage:
    """Enough of Package to exercise read_struct_array on real byte layout."""

    def __init__(self, names: list[str], data: bytes) -> None:
        self.names = names
        self.data = data
        self.name_id = {name: index for index, name in enumerate(names)}

    def name_at(self, offset: int) -> str:
        index, number = struct.unpack_from('<2I', self.data, offset)
        base = self.names[index] if index < len(self.names) else '<%d>' % index
        return base if number == 0 else '%s_%d' % (base, number - 1)

    def head_bytes(self, first: str, second: str) -> bytes:
        return (struct.pack('<2I', self.name_id[first], 0) +
                struct.pack('<2I', self.name_id[second], 0))


NAMES = ['none', 'time', 'transitiontime', 'targetcamgroup', 'shotnumber',
         'floatproperty', 'nameproperty', 'intproperty', 'cam1', 'cam2',
         'toggleaction', 'byteproperty', 'etta_on', 'etta_off']


def _tag(pkg_names: list[str], name: str, ptype: str, size: int) -> bytes:
    return (struct.pack('<2I', pkg_names.index(name), 0) +
            struct.pack('<2I', pkg_names.index(ptype), 0) +
            struct.pack('<2i', size, 0))


def _cut_element(time_s: float, camera: str) -> bytes:
    body = _tag(NAMES, 'time', 'floatproperty', 4)
    body += struct.pack('<f', time_s)
    body += _tag(NAMES, 'transitiontime', 'floatproperty', 4)
    body += struct.pack('<f', 0.0)
    body += _tag(NAMES, 'targetcamgroup', 'nameproperty', 8)
    body += struct.pack('<2I', NAMES.index(camera), 0)
    body += _tag(NAMES, 'shotnumber', 'intproperty', 4)
    body += struct.pack('<i', 531)
    body += struct.pack('<2I', NAMES.index('none'), 0)
    return body


def _cut_array(declared: int, cuts: list[tuple[float, str]]) -> bytes:
    payload = struct.pack('<i', declared)
    for time_s, camera in cuts:
        payload += _cut_element(time_s, camera)
    return payload


class ReadStructArrayTest(unittest.TestCase):
    def test_reads_every_element(self) -> None:
        payload = _cut_array(2, [(0.0, 'cam1'), (13.95, 'cam2')])
        pkg = FakePackage(NAMES, payload)
        rows = ext.read_struct_array(pkg, 0, len(payload),
                                     ext.CUT_HEAD_FIELDS, 'test')
        self.assertEqual(2, len(rows))
        self.assertEqual('cam1', rows[0]['targetcamgroup'])
        self.assertAlmostEqual(13.95, rows[1]['time'], places=4)

    def test_short_array_raises_instead_of_returning_partial(self) -> None:
        # Two elements declared, one written: exactly the failure the shipped
        # export hid behind a decodedPartial flag.
        payload = _cut_array(2, [(0.0, 'cam1')])
        pkg = FakePackage(NAMES, payload)
        with self.assertRaises(ValueError) as caught:
            ext.read_struct_array(pkg, 0, len(payload),
                                  ext.CUT_HEAD_FIELDS, 'test')
        self.assertIn('declared 2', str(caught.exception))

    def test_empty_array_is_not_an_error(self) -> None:
        payload = struct.pack('<i', 0)
        pkg = FakePackage(NAMES, payload)
        self.assertEqual([], ext.read_struct_array(
            pkg, 0, len(payload), ext.CUT_HEAD_FIELDS, 'test'))


def _toggle_element(time_s: float, action: str) -> bytes:
    body = _tag(NAMES, 'time', 'floatproperty', 4)
    body += struct.pack('<f', time_s)
    body += _tag(NAMES, 'toggleaction', 'byteproperty', 8)
    body += struct.pack('<2I', NAMES.index('none'), 0)      # enum name slot
    body += struct.pack('<2I', NAMES.index(action), 0)
    body += struct.pack('<2I', NAMES.index('none'), 0)
    return body


class ToggleArrayTest(unittest.TestCase):
    def test_toggle_elements_share_the_cut_head(self) -> None:
        # Toggle, visibility and event all open with (time, floatproperty),
        # which is why one head pair serves every switch track.
        payload = struct.pack('<i', 2)
        payload += _toggle_element(0.0, 'etta_off')
        payload += _toggle_element(11.95, 'etta_on')
        pkg = FakePackage(NAMES, payload)
        rows = ext.read_struct_array(pkg, 0, len(payload),
                                     ext.CUT_HEAD_FIELDS, 'toggle')
        self.assertEqual(2, len(rows))
        self.assertEqual('etta_off', rows[0]['toggleaction'])
        self.assertAlmostEqual(11.95, rows[1]['time'], places=4)


class ConversionTest(unittest.TestCase):
    def test_position_axes(self) -> None:
        # UE3 centimetres, Z up -> metres, Y up.
        self.assertEqual([1.0, 3.0, -2.0], ext.to_our_position(100.0, 200.0,
                                                               300.0))

    def test_vertical_fov_is_smaller_than_horizontal(self) -> None:
        vertical = ext.horizontal_to_vertical_fov(50.0)
        self.assertLess(vertical, 50.0)
        expected = math.degrees(2.0 * math.atan(
            math.tan(math.radians(50.0) * 0.5) / ext.SOURCE_ASPECT))
        self.assertAlmostEqual(expected, vertical, places=3)

    def test_degenerate_fov_is_passed_through(self) -> None:
        self.assertEqual(0.0, ext.horizontal_to_vertical_fov(0.0))
        self.assertEqual(180.0, ext.horizontal_to_vertical_fov(180.0))


if __name__ == '__main__':
    unittest.main()
```

---

## 11. 적용 순서와 검증

1. 위 두 파일을 추가한다. C++ 변경이 없으므로 프로젝트 등록은 없다.

2. 테스트를 먼저 돌린다.

```powershell
python -m unittest discover -s Tools\KakulSaydonPipeline -p "test_extract_scene_matinee.py" -v
```

   현재 7개가 통과한다.

```text
test_degenerate_fov_is_passed_through
test_position_axes
test_vertical_fov_is_smaller_than_horizontal
test_empty_array_is_not_an_error
test_reads_every_element
test_short_array_raises_instead_of_returning_partial
test_toggle_elements_share_the_cut_head
```

3. 추출기를 돌린다. 패키지 경로는 이 PC 기준이다.

```powershell
python Tools\KakulSaydonPipeline\extract_scene_matinee.py `
  --package "C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages\B9AVB2VAZIQRPQCJVKAVYRAVOKYPY806.upk" `
  --output "Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\SCENE04A.interpdata_2.matinee.json" `
  --label SCENE04A
```

4. 성공하면 다음이 나온다. 오늘 실측한 값과 같아야 한다.

```text
interpdata_1   groups=28   live director: interptrackdirector_1
    0.00s cam1 | 6.58s cam3 | 7.51s cam1 | 8.58s cam2
interpdata_2   groups=98   live director: interptrackdirector_3
    -1.01s cam1 | 2.10s cam2 | 13.95s cam3 | 19.49s cam4 | 23.95s cam6
interpdata_4   groups=2    live director: none
interpdata_6   groups=3    live director: interptrackdirector_3
    0.00s cam1
written: Data\Maps\Imported\LV_LUT_MIDNIGHTC_ED\SCENE04A.interpdata_2.matinee.json
```

5. 실패하면 `declared N elements but decoded M` 예외가 나고 JSON 은 만들어지지 않는다.

6. 산출물 확인 항목이다.

```text
interpdata_2 의 liveDirectorTrack 이 interptrackdirector_3
그 컷 목록이 -1.01 cam1 | 2.10 cam2 | 13.95 cam3 | 19.49 cam4 | 23.95 cam6
move 트랙 111개, 그 중 disabled 36개
animcontrol 클립 37개, 그 중 playRate 가 1.0 이 아닌 것 3개
toggle 35 트랙, visibility 1, event 1, fade 1(8키)
```

7. G2 로 넘어가기 전에 8절의 사용자 결정 네 가지를 받는다.

---

## 12. 이 계획서가 확정하지 않은 것

- 액터 미연결 그룹 14개의 연결 메시. 패키지에 정보가 없다(4.10). 촛대 둘이 여기
  들어가며 기준 배치는 저작으로 정한다.
- 대사 원문. 자막 트랙이 `msgid` 만 갖는다(4.11).
- 대기 무대를 아레나로 옮기는 평행이동 값. G4 에서 실측으로 정한다(5.4).
- 카드 외 파티클 17종과 Wwise 사운드. 범위 밖이며 RESULT 에 미구현으로 남긴다.
- 원작 영상 대조. 받은 녹화본은 재생 위치가 건너뛰어 시각 측정에 쓸 수 없다(4.13).
  컷 목록은 패키지에서 확정했으므로 영상이 없어도 G3 을 시작할 수 있다.
