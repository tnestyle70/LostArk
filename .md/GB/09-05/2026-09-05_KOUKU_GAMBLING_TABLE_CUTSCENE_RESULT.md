# 2026-09-05 쿠크세이튼 도박판 컷신 1차 RESULT

Area `LV_LUT_MIDNIGHTC_ED`. 원작 `SCENE04A / interpdata_2`의 테이블 구간
(`15.660 ~ 27.000초`)을 팝업북과 같은 경로로 올리고, MapTool 에서
`Play gambling table` 로 재생되게 했다. 첫 시도에서 반대쪽 절반을 만든 경위는
아래 `교정` 절에 있다.

계획서는 같은 폴더의 `2026-09-05_KOUKU_GAMBLING_TABLE_CUTSCENE_PLAN.md` 다.

## 구현 완료

### 테이블 쿠킹

- `Tools/ActorXAssetCooker/Cook-KoukuSaydonGamblingTable.ps1` 추가.
  검증된 `Cook-KoukuSaydonInteractionProps.ps1` 을 파생해 원본과 자산 목록만 바꿨다.
  두 번째 쿠킹 경로를 만들지 않고 `PSK/glTF + PSA -> Blender 3.0 -> FBX ->
  ModelAssetConverter -> WModel` 을 그대로 쓴다.
- 원본은 `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/_GamblingTable/` 에 둔다.
  기존 `_Lever_ITR_02283` 과 같은 자리다.
- 산출물 `AnimatedProps/DEPLOY_CINE_KOUKU_TABLE/DEPLOY_CINE_KOUKU_TABLE.wmodel`
  79,028 bytes. 섹션 4개, 스켈레톤 있음, 애니메이션 1개 `evt2_table_open01`.
- 텍스처 4장이 머티리얼에 붙었고 원본과 SHA-256 이 일치한다.

```text
bg_rad_koukusaton_floor16_mi_hht   -> bg_rad_koukusaton_floor16_d_hht.dds
bg_rad_koukusaton_floor15_mi_hht   -> bg_rad_koukusaton_floor15_d_hht.dds
bg_rad_koukusaton_floor15a_mi_hht  -> bg_rad_koukusaton_floor15a_d_hht_loc_int.dds
bg_rad_koukusaton_floor15b_mi_hht  -> bg_rad_koukusaton_floor15b_d_hht_loc_int.dds
```

### 등록

네 문서를 한 번에 갱신했다.

| 문서 | 추가한 것 |
|---|---|
| `Data/Maps/Imported/.../LV_LUT_MIDNIGHTC_ED.deployassets` | `DEPLOY_CINE_KOUKU_TABLE` 행, count 4 -> 5 |
| `Data/Maps/Authoring/.../LV_LUT_MIDNIGHTC_ED.deployplacements` | 배치 8번, count 6 -> 7 |
| `Data/Maps/Authoring/.../LV_LUT_MIDNIGHTC_ED.worldsequences.json` | 템플릿 `sequence.LV_LUT_MIDNIGHTC_ED.gambling_table` 과 인스턴스 `world.sequence.instance.gambling_table`, revision 119 |
| `Data/Maps/Authoring/.../LV_LUT_MIDNIGHTC_ED.camerashots.json` | shot `shot.kouku.table`, 키 16개, revision 59 |

배치 8번은 아레나 중앙 `(-0.2883, 1.3253, 737.6292)`, yaw `-135도`, scale 1 이다.
이 절반의 카메라 셋이 모두 향하는 `데스크기둥` 이 여기로 오도록 세트를 평행이동했다.

### 카메라

원작의 재생되는 컷 목록은 `interptrackdirector_3` 이며 계획서 4.1 에서 확정했다.
테이블을 찍는 뒤 세 컷을 옮겼다. 창 기준 시각으로 다시 쓰면 이렇다.

```text
cam3   0 ~ 3830ms      멀리서 들어온다, 수직 화각 58.45도
cam4   3830 ~ 8290ms   테이블 위, 수직 화각 42.10 -> 58.45도
cam6   8290 ~ 11340ms  마지막, 수직 화각 34.54도
```

컷은 `3829/3830`, `8289/8290` 처럼 1ms 간격 두 키로 넣었다. 선형 보간이 1ms 안에서
끝나므로 30fps 한 프레임 안에 전환된다. 스키마를 바꾸지 않았다.

키는 16개이고 상한은 64개다.

전방 벡터는 원본 `eulertrack` 의 (Roll, Pitch, Yaw) 에서 만들었다. 처음에 축을 잘못
잡아 모든 키가 90도 틀어졌고, 조준 오차를 재서 잡았다. 최종 조준 오차는 0.2~19.6도다.

### MapTool 재생

- `Client/Public/MapTool.h`, `Client/Private/MapTool.cpp` 에
  `Play_CutsceneGamblingTable()` 과 `Is_CutsceneGamblingTablePlaying()` 추가.
- `World Sequence` 탭의 `Cutscene Arena Preview` 에 `Play gambling table` 버튼을
  `Play 2 (original)` 옆에 붙였다.
- 카메라 구동은 기존 `Apply_CutsceneCameraTrack` 하나만 쓴다. 바인딩된 시퀀스가
  재생 중이면 우선순위가 높은 shot 이 카메라를 잡는 기존 규칙 그대로이며,
  `shot.kouku.table` 은 priority 21 로 책의 20 보다 높다.
- 팝업북과 도박판은 각자의 running 플래그를 갖는다. 도박판이 끝나면 카메라를 돌려주고
  배치 8번을 `DESPAWNED` 로 되돌릴 뿐, 팝업북의 아레나 인계에는 관여하지 않는다.

## 교정: 처음에 반대쪽 절반을 만들었다

첫 시도는 마티네 `0 ~ 13.95초`를 옮겼다. 사용자 녹화본과 원본을 대조해서 그것이
**도박판이 아니라 쿠크의 책 대사 장면**임을 확인했다.

두 영상의 시계는 암전으로 맞췄다. 원본 녹화본의 암전이 `14.93 ~ 16.90초`, 마티네
fade 가 `13.48 ~ 16.29초` 이므로 `영상 시각 = 마티네 시각 + 1.03초` 다.

```text
마티네  6.0초 (영상  7.03초)  쿠크가 책을 들고 대사한다
                              "몸풀기는 여기까지 하고, 다음 장으로 넘어가 볼까?"
마티네 20.5초 (영상 21.53초)  쿠크가 초록 카드 테이블 위에 서 있다
```

화면의 초록색 점유율도 같은 결론이다. 마티네 20.5초에서 27.5% 로 정점이고, 처음
만든 `0 ~ 13.95초` 구간에서는 0~3% 다.

내가 틀린 지점은 이렇다. cam1/cam2 가 아레나 좌표(z 약 737)에 있으니 그쪽이 도박판일
것이라 보고, 지하 무대(y 약 -108)의 cam3/cam4/cam6 은 "우리에게 없는 무대"라며 잘랐다.
실제로는 반대였다. 테이블은 그 두 번째 무대에서 찍히고, 아레나 카메라는 그 앞의 책
대사를 찍는다. 그래서 런타임 영상에서 카메라가 아레나 장식물 안에 박혀 있었다.
cam2 는 쿠크 얼굴 2m 앞 근접샷인데 우리 아레나 그 자리에는 아무것도 없기 때문이다.

### 다시 만든 구간

```text
창       15660 ~ 27000ms  (11340ms)   테이블이 열리는 프레임에서 시작한다
컷       cam3 15660~19490 | cam4 19490~23950 | cam6 23950~27000
평행이동 (297.4417, 103.4853, 290.5892)
```

평행이동의 기준은 그룹 `데스크기둥` 이다. 테이블의 중심 기둥이고, 이 절반의 세 카메라가
모두 그것을 향한다. 검산하면 cam3 는 약 0도, cam4 는 5.7도, cam6 는 15도로 맞는다.

처음에는 세 카메라의 조준선을 최소제곱으로 교차시켜 앵커를 잡았는데, 그 계산이 광선을
직선으로 다뤄 카메라 **뒤쪽** 점을 골랐다. 그대로 두면 모든 키가 128~175도 틀어진다.
`데스크기둥` 으로 바꾸고 나서 조준 오차가 0.2~19.6도가 됐다.

키는 16개, 상한은 64개다. 배치 8번은 아레나 중앙 `(-0.2883, 1.3253, 737.6292)` 로 옮겼다.

## 검증

- 쿠킹: 클립 이름과 길이 단언 통과(`evt2_table_open01`, 1.5333초 = 46프레임/30fps).
  텍스처 4장 SHA-256 일치.
- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish` 성공.
  배치 3231행, 파일 6개. 교정 뒤 다시 실행해 camerashots revision 59,
  worldsequences revision 119 로 반영했다.
- 런타임 문서 4개에 항목이 모두 들어갔다.
  `deployassets` 1행, `deployplacements` 8번, `worldsequences` 인스턴스 1개,
  `camerashots` shot 1개(키 16, duration 11340).
- Client Debug 빌드 성공.
- **화면 확인은 하지 않았다.** 사용자가 직접 재생해야 한다.

## 이번 범위에서 내가 정한 것

계획서 8절의 결정 대기 항목을 실행을 위해 다음과 같이 정했다. 되돌리기 쉬운 값이다.

1. **테이블 구간만 옮겼다.** 원작 `15.660 ~ 27.000초` 다. 그 앞의 책 대사 구간과
   맨 앞 아레나 원경은 넣지 않았다.
2. **테이블 애니메이션이 창 시작과 같이 열린다.** 원작에서 `table` 이 15.660초에
   정방향으로 여는 프레임을 창의 0으로 잡았으므로 원작과 같은 순간이다.
3. **의자·촛대·카드·대사·쿠크 연기는 넣지 않았다.** 계획서 G4~G5b 범위다.

## 남은 것

- 의자 4개와 촛대는 정적 메시라 쿠킹만 하면 붙는다. 다만 촛대의 움직이는 그룹은
  액터 링크가 원본에 없어 기준 배치를 저작해야 한다(계획서 4.10).
- 카드 흩날림은 추출한 카드 메시 5종으로 Effect Tool 에서 재저작한다(계획서 5.7).
- 대사 자막은 원문이 패키지에 없다(계획서 4.11).
- 테이블 지연 5.320초를 살리려면 `WORLD_SEQUENCE_ANIMATION_TRACK` 에 대기 표현이
  필요하다. 역재생 `reverse` 필드도 같은 구조체에 아직 없다(계획서 5.6).
- 제품 레벨 트리거 연결은 하지 않았다. 지금은 MapTool 재생까지다.

## 교정 2: 팝업북 아레나 위에 얹어 놓고 치우지 않았다

사용자 지적으로 확인했다. 원본의 이 구간은 **배경이 새까맣다.** 텐트도, 관객석도,
아레나도 없이 테이블과 쿠크만 어둠 속에 있다.

그런데 이 구현은 팝업북 컷신 보스가 서 있는 아레나 중앙
`(-0.2883, 1.3253, 737.6292)` 을 앵커로 잡고 그 위에 세트를 얹었다. 계획서 5.4 는
방법 A 를 고르면 "아레나 기존 배치와 겹치면 컷신 동안 숨겨야 한다"고 적었는데
그 숨김을 구현하지 않았다. 그래서 서커스 아레나가 모든 컷의 배경으로 남았고,
팝업북의 책(배치 7)과 보스(배치 5)까지 화면에 서 있었다.

`Play_CutsceneGamblingTable` 이 이제 다음을 한다.

```text
Apply_CutsceneArenaVisibility(true)   아레나 배치 461개를 치운다
Hide_CutsceneSet()                    팝업북 컷신 사본 41~299 를 치운다
Set_States  배치 8, 9 를 INTACT, 배치 5, 7 을 DESPAWNED
```

끝나면 `Apply_CutsceneArenaVisibility(false)` 로 아레나를 돌려준다. 팝업북 쪽
`m_bCutsceneOriginalRunning` 경로는 건드리지 않으므로 두 컷신의 인계는 서로 독립이다.

## 교정 3: 회전각과 초록 바닥

프레이밍이 원본과 달라 두 가지를 더 고쳤다.

### 테이블 회전각

배치 8번의 yaw 를 `-135도` 로 두었는데, 그 각이 하필 부채꼴 판을 카메라와 쿠크
사이에 정확히 세우는 각이었다. 앵커 높이를 세 번 바꿔도 안 고쳐진 이유다.

근접 키 9개마다 눈에서 쿠크 가슴으로 광선을 쏴 메시에 막히는지 세고 yaw 를 10도
간격으로 훑었다.

```text
  0도  9/9 관통  여유 1.16m
 60도  9/9       여유 3.43m
100도  9/9       여유 4.96m   <- 채택
130도  9/9       여유 1.58m
180도  0/9 전부 차단
225도  1/9       <- 기존 -135도
```

`100도` 로 바꾸자 쿠크가 화면 중앙에 원본과 같은 크기로 들어왔다.

### 초록 카드 바닥

그래도 쿠크 발밑에 초록이 깔리지 않았다. 원인은 쿠킹한 메시 자체였다.

```text
bg_rad_koukusaton_table 의 초록 면 331.2 m2
  프레임  0  법선 (-1.00, 0.00, -0.10) 등, 수직
  프레임 46  법선 (-0.78, 0.62,  0.00) 등, 수직
```

어느 포즈에서도 초록 면이 눕지 않는다. 이 메시는 바닥이 아니라 접혔다 펴지는
칸막이다. 실제 카드 바닥은 같은 `floor15/16` 머티리얼을 쓰는 **정적 메시**이고,
`MAP_..._FLOOR16C_SM_HHT` 계열 46개가 아레나 중앙 32~38m 안에 이미 배치돼 있다.

그런데 그 46개가 전부 컷신 중 숨기는 461개 목록 안에 있었다. 배경을 비우려던 교정 2가
도박판 바닥까지 지우고 있었다.

`Reveal_GamblingTableFloor()` 가 아레나 숨김 직후 `FLOOR15` / `FLOOR16` 을 자산 ID 로
찾아 다시 켠다. 텐트와 관객석은 사라지고 카드 바닥은 남는다. 팝업북 경로는 이 함수를
부르지 않으므로 인계 동작이 그대로다.

## 오프라인 렌더 증거

에이전트는 Client 를 조작하거나 화면을 캡처하지 않는다. 대신 같은 원본과 같은 카메라
키로 Blender 헤드리스 렌더를 만들어 자산과 프레이밍을 확인했다. 게임 스크린샷이 아니고
visual PASS 도 아니다.

`.md/GB/09-05/assets/2026-09-05_KOUKU_GAMBLING_TABLE/`

```text
01_table_folded_frame00.png       클립 0프레임, 접힌 상태
02_table_opening_frame24.png      펼쳐지는 중
03_table_open_frame46.png         다 펼쳐진 초록 판과 갈색 테두리
10_shot_00000ms_cam3_approach.png 창 시작, 72m 원경
11_shot_03829ms_cam3_end.png      cam3 끝
12_shot_04770ms_cam4_close.png    cam4 근접, 초록 판이 화면을 채운다
13_shot_08290ms_cam6_start.png    cam6 시작
14_shot_11340ms_end.png           창 끝
20_original_matinee_20.5s.jpg     원본 같은 시점: 쿠크가 테이블 위에 서 있다
21_original_matinee_06.0s_book_dialogue.jpg  첫 시도가 만들었던 책 대사 장면
3x_textured_*.png                 같은 카메라 키를 실제 DDS 4장과 EEVEE 로 렌더
```

`3x_textured_06140ms.png` 에서 초록 펠트의 마름모 문양과 금색 장식 테두리가 원본과
같은 것을 볼 수 있다. 워크벤치 렌더의 단조로운 색은 재질이 없어서였고, 쿠킹된
텍스처 자체는 정상이다.

렌더에서 확인한 것과 확인하지 못한 것은 다음과 같다.

```text
확인됨   쿠킹된 테이블이 원본과 같은 형상이다. 초록 판에 갈색 장식 테두리.
확인됨   evt2_table_open01 이 실제로 접힌 상태에서 펼쳐진다.
확인됨   카메라가 원경에서 들어와 테이블 근접으로 가는 구성이 원본과 같다.
미확인   원본 근접샷의 화면 중앙에는 쿠크가 테이블 위에 서 있다. 이번 범위에
         쿠크 연기가 없으므로 그 자리가 비어 있다.
미확인   재질과 조명. Blender workbench 는 평면 색만 칠한다.
```

## 사용자가 확인할 절차

```text
1. Client Debug 실행
2. Lobby -> Test
3. F1 -> Developer Tools -> Map Tool
4. Area LV_LUT_MIDNIGHTC_ED 로드
5. World Sequence 탭 -> Cutscene Arena Preview -> [Play gambling table]
```

초록 카드 테이블이 펼쳐지고, 카메라가 멀리서 다가와 3.8초와 8.3초에 두 번 컷되며,
11.34초에 끝나면서 카메라와 소품이 돌아오면 정상이다.

키를 다듬으려면 `Camera` 탭의 `shot.kouku.table` 헤더에서 `Hold Cutscene`,
`Play This Key`, `Set From Free Camera` 를 팝업북과 같은 방식으로 쓴다.

## 09-06 추가: 조명, 의자, 촛대, 카드

사용자 지적. 이펙트와 자막과 사운드가 없어도 조명, 의자, 촛대, 카드는 원본과
같이 만들 수 있는데 하지 않았다. 맞는 지적이었다. 그 넷을 만들면서 앞서 넣은
저작이 실제로는 재생 자체를 막고 있었다는 것을 찾아 함께 고쳤다.

### 찾은 결함

```text
1. Deploy 프롭은 시퀀스로 움직일 수 없다.
   CWorldSequencePlayer::Play 는 DEPLOY_PLACEMENT 바인딩마다 애니메이션
   트랙을 찾고, 없으면 인스턴스 전체를 거절한다(WorldSequencePlayer.cpp:375).
   의자 넷을 DEPLOY_PLACEMENT 에 트랜스폼 트랙으로 묶어 두었으므로 이 상태의
   [Play gambling table] 은 아무것도 재생하지 못한다.

2. 의자 오프셋 단위가 100배였다.
   matinee 의 move 트랙은 UE 축의 센티미터다. 187.454 를 그대로 미터로 썼다.
   올바른 변환은 (x, z, -y) * 0.01 이고 실제 이동은 뒤로 1.87 m 미끄러지는
   것이다.

3. 트랙 상한.
   WorldSequenceDocument.h 의 MAX_TRACK_COUNT 는 32 이고 tracks 와
   animationTracks 의 합에 걸린다. 테이블과 쿠크가 이미 여덟을 쓴다.

4. 패치 스크립트의 정규식이 소스 180 줄을 지웠다.
   lazy 수량자가 첫 Set_States 안에서 끝나지 못하고 두 함수 뒤의 닫는 괄호까지
   삼켰다. git 에 있던 절반은 diff 에서, 이번 세션에 쓴 절반은 세션 로그에서
   원문 그대로 복구했다.
```

### 만든 것

```text
조명   Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED.maplights.json 신규.
       interpdata_2 의 l2 l3 를 같은 rigid offset 으로 옮긴 둘과, 덱 위
       key 와 반대편 fill 둘. MapCatalog 에 sourceLights/lights 쌍 추가.
       Area 는 그 전까지 라이트 레이어 자체가 없었다.

의자   맵 배치 5000..5003. 각자 자기 프레임으로 뒤 1.87~1.97 m 에서 시작해
       1.74~2.54 초에 제자리로 미끄러진다. 원본 키 여섯 개 중 창 안의 넷을 쓴다.

촛대   맵 배치 5004..5005. 원본 move 트랙이 키 하나뿐이라 움직임이 없다.
       트랙은 두 키만 두는데, 배치를 invisible 로 저작했으므로 그것이 컷신
       동안 보이게 하는 수단이다.

카드   맵 배치 5006..5029, 전용 템플릿 sequence...gambling_cards 24 트랙.
       matinee 가 par_q_cardfly_01 을 21040ms 에 트리거하므로 창 기준
       5380ms 에 터진다. 열여덟은 덱 테두리에서 16~30 m 솟구치며 3~9 m
       퍼지고, 여섯은 마지막 cam6 렌즈 앞에서 떨어진다.
```

의자와 촛대와 카드는 전부 맵 배치이고 `visible 0` 으로 저작했다. Deploy 로
남은 것은 테이블과 쿠크와 덱과 받침 넷뿐이라 KAKUL_GAMBLING_PROP_LAST_ID 가
17 에서 11 로 돌아왔다. 쿠킹해 둔 CHAIR/CANDELABRA/CARD_A~C wmodel 은 이제
쓰지 않는다. 아레나가 같은 소스 메시를 이미 맵 에셋으로 싣고 있기 때문이다.

### 실행한 검증

```text
cl /Zs MapTool.cpp                      exit 0 (C4819 외 경고 없음)
Publish-MapAuthoring -Mode Publish       3261 placements, FileCount 7
쿠킹 치수 대조                            의자 6.09x14.62x5.6, 촛대 16.01,
                                         카드 1.42x0.98 로 아레나가 이미 싣고
                                         있는 같은 소스 에셋과 완전히 일치
카메라 투영 점검                          check_cards_in_frame.py
                                         5.6~8.0초 화면 안 2~11장
                                         8.3~11.0초 3.6~6.4 m 거리에 1~6장
git diff --check                         공백 오류 없음
```

Visual Studio 가 열려 있어 이 트리에서 빌드는 돌리지 않았다. 컴파일 확인은
출력 파일을 만들지 않는 구문 검사다.

### 아직 아닌 것

```text
par_q_cardfly_01 파티클 자체, 자막, 사운드.
쿠크의 연기는 애니메이션 트랙으로만 있고 표정과 시선은 없다.
조명 값(색, 반경, 밝기)은 저작값이지 추출값이 아니다. provenance 에 그렇게 적었다.
사용자의 실제 화면 확인.
```

## 09-06 두 번째: 프레임 단위 녹화본과 대조

사용자가 `바탕 화면\도박 1.mp4`로 원본을 한 프레임씩 넘기며 녹화해 주었다.
1709 프레임 중 화면이 실제로 바뀐 지점 250개를 뽑아 스텝으로 삼고, 컷 위치는
스텝 간 차이의 극값으로 찾았다. 스텝 80→81, 106→107, 172→173 세 곳이며
디렉터 트랙의 컷 셋(cam3 13950, cam4 19490, cam6 23950)과 개수가 맞는다.

### 녹화본이 알려준 것

```text
1. 테이블이 자기 초록 펠트를 만든다.
   스텝 0~19가 접힌 상태에서 펴지는 과정인데, 초록 패널이 세로로 서 있다가
   내려앉아 반달을 만든다. 쿠킹된 wmodel 도 재질 5개에 floor15 텍스처를 갖고
   있다. 따로 놓았던 초록 덱(map placement)과 나무 받침(deploy)은 같은 면을
   몇 미터 옆에 한 번 더 그리고 있었다. 사용자 스크린샷에서 초록 판이 세트
   옆에 떠 보이던 것이 이것이다. 둘 다 제거했다.

2. 쿠크는 처음부터 서 있지 않는다.
   group 33 의 live move 트랙은 키 두 개이고 mode 가 cim_constant 다.
   0ms 에 (-331.3, -101.4, 359.35), 18110ms 에 (-301.04, -101.46, 450.33).
   같은 오프셋으로 옮기면 각각 90m 밖과 (-3.5948, 2.0251, 740.9241) yaw -135 다.
   즉 테이블 전개와 의자 슬라이드 동안 그는 화면 밖에 있고 창 기준 2450ms 에
   테이블 위로 스냅한다. 이 저작은 그를 프레임 0부터, 그것도 데이터가 주는
   자리에서 4.6m 떨어진 곳에 세워 두고 있었다.

3. 마지막 컷의 카드 밀도.
   스텝 173~230 은 큰 카드가 화면을 가득 채운다. 24장으로는 모자라 52장으로
   늘렸고, 템플릿 한 장의 32트랙 한도 때문에 인스턴스 둘로 나눴다.
```

애니메이션 클립과 의자 키는 대조 결과 이미 맞았다. `att_battle_5_03` 18080,
`idle_battle_1` 19680, `att_battle_10_01` 20970 과 21693(rate 0.2),
`att_battle_9_02` 24050(rate 0.6), `idle_normal_1` 24810(rate 0.6) 이 창 기준
2420 / 4020 / 5310 / 6033 / 8390 / 9150 으로 저작본과 일치한다. 의자 여섯 키의
창 안 네 개도 같다. 촛대는 move 키가 하나뿐이라 원본에서도 움직이지 않는다.
녹화본에서 솟아오르는 것처럼 보이는 것은 cam3 가 46m 에서 34m 로 내려오기
때문이다.

### 이번에 바꾼 것

```text
Deploy   8 테이블, 9 쿠크 둘뿐. 덱과 받침 제거로 범위가 8..9 로 줄었다.
쿠크     배치를 (-3.5948, 2.0251, 740.9241) yaw -135 로 옮기고,
         시퀀스 시계가 2450ms 를 넘을 때 INTACT 로 바꾸는 상태 전환을 추가했다.
         Deploy 프롭은 트랙으로 못 움직이므로 키가 아니라 상태로 처리한다.
카드     52장, 인스턴스 gambling_cards 와 gambling_cards_b 각 26 트랙.
         출발 높이를 테이블 상판 2.06 으로 올렸다.
```

### 실행한 검증

```text
cl /Zs MapTool.cpp                 exit 0
Publish-MapAuthoring               3289 placements, FileCount 7
바인딩 대상 실재                     60개 전부 존재, deploy 8·9 존재
템플릿 트랙 한도                     14 / 26 / 26 전부 32 이하
카메라 투영                          클로즈업 8.6~10.4초 화면 안 17~25장,
                                    와이드 6.2초 17장
git diff --check                    공백 오류 없음
```

파티클(카드 흩날림 par_q_cardfly_01, 등장 연기, 테이블 위 마법진), 자막,
사운드는 여전히 없다. 빌드는 Visual Studio 가 열려 있어 돌리지 않았다.

## 09-06 세 번째: 사용자 프레임 대조로 잡은 여섯 가지

사용자가 원본 프레임을 하나씩 짚어 주었다. 그 지적과 데이터 대조로 다음을 고쳤다.

```text
1. 책을 지우고 있었다.
   Play_CutsceneGamblingTable 이 book(deploy 7)을 DESPAWNED 로 만들고 있었다.
   원본은 이 장면 전체를 펼쳐진 책 위에서 한다. INTACT 로 바꾸고 arena_rise 의
   책 despawn 타이머도 비활성으로 둔다.

2. 세트가 책보다 5m 아래에 있었다.
   촬영 무대 바닥 높이 -5.011 을 그대로 옮겨 왔는데 우리 책의 페이지는 팝업북
   컷신이 아레나 바닥을 남기는 y 0.00~0.03 이다. 프롭·쿠크·카드·카메라 16키·
   조명 4개를 전부 +5.011 올렸다. 테이블 상판이 7.011 로 온다.

3. 의자가 낙하하지 않았다.
   원본은 촛대가 일어서려 할 때 가운데 두 개가 먼저 떨어지고 이어 바깥 두 개가
   기울어진 채 떨어진다. 14m 위에서 30도 기울여, 가운데 250~800ms,
   바깥 600~1150ms 로 넣었다. 착지 뒤에 기존 1.87m 슬라이드가 이어진다.

4. 촛대가 누웠다 일어나지 않았다.
   80도로 누운 자세에서 400~2200ms 동안 기립하도록 회전 키를 넣었다. 회전은
   배치 원점에서 걸리는데 그 원점이 촛대의 밑동이라 경첩이 바닥에 생긴다.

5. 의자 스케일이 틀렸다.
   맵의 SL03 이 원본 도박장 구역이고 거기 의자 넷은 0.75, 촛대 둘은 1.0,
   초록 펠트는 1.0 이다. 의자만 1.0 으로 놓고 있었다. 0.75 로 고쳤다.
   같은 구역에서 펠트(y 10.56)와 의자 바닥(y 3.52)의 차이가 7.04m 인데,
   촬영 무대에서 잰 desk 상단과 바닥의 차이와 정확히 같다. 높이 계산의 교차 확인이다.

6. 마지막 컷의 카드가 화면을 덮었다.
   26장을 2.8~6.2m 에 두어 34도 렌즈에서 쿠크가 완전히 가렸다. 14장으로 줄이고
   4.5~9.5m 로 밀었다. 화면 안 카드가 21~25장에서 10~14장이 되었다.
```

의자 배치 자체는 이미 맞았다. 넷 다 반경 18.23m 에 정면 오차 0.0~0.1도로 테이블을
보고 있고, 촛대 반경 18.24m 는 SL03 의 18.2m 와 일치한다. 흩어져 보인 것은 5번이
아니라 2번, 세트가 책 아래에 떠 있었기 때문이다.

### 실행한 검증

```text
cl /Zs MapTool.cpp        exit 0
Publish-MapAuthoring      3277 placements, FileCount 7
바인딩                     48개 전부 대상 존재, 템플릿 트랙 14/26/14
카메라 프레이밍             0~3500ms 세트 전체, 4500~6000ms 테이블과 쿠크,
                          7500ms 세트 전체, 9000~11000ms 클로즈업
카드 프레이밍               와이드 17장, 클로즈업 10~14장
도식 렌더                   set_schematic.png 12컷, 원본 프레임과 구성 대조
git diff --check           공백 오류 없음
```

여전히 없는 것은 파티클 셋이다. 테이블 위 마법진, 착지 연기, 카드 흩날림
par_q_cardfly_01. 자막과 사운드도 없다. 촛불은 아레나 에셋에 emissive 텍스처가
있어 켜져 보이지만 특정 시점에 점등되지는 않는다.

### 마지막 두 가지

```text
7. 책이 닫힌 채 서 있었다.
   책을 살려 놓기만 하고 자세를 안 줬다. Deploy 프롭은 스폰될 때 bind pose 인데
   그게 닫힌 책이다. animated.book 슬롯에 ao_evt2_book02 를 rate 8(상한)으로
   holdLastFrame 재생해 296ms 만에 펼치고 그 자세를 유지한다. 그 시점 카메라가
   48m 밖이라 이미 펼쳐진 상태로 보인다. 컷신이 끝나면 책도 같이 despawn 한다.

8. 쿠크를 Play 전에 숨기고 있었다.
   Play 는 DEPLOY_PLACEMENT 바인딩마다 Begin_AnimationAuthoringPreview 를
   호출한다. 그 전에 DESPAWNED 로 만들면 바인딩이 실패할 수 있다. 숨기는 것을
   Play 뒤로 옮겼다.
```

### 의자 낙하의 출처를 분명히 해 둔다

matinee 의 의자 move 트랙은 `imf_relativetoinitial` 이고 액터 회전은 yaw 뿐이며
UE3 의 up 은 +Z 다. 키 값 `(0, 187.454, 0)` 의 up 성분은 0 이므로 이 트랙은
**수평 1.87m 이동이지 낙하가 아니다.** 그런데 사용자가 프레임으로 짚어 준 원본에는
의자가 공중에서 떨어진다. 즉 낙하는 이 트랙 밖의 무언가가 만든다 - 팝업북 자체의
종이 구조이거나 다른 시퀀스일 수 있고, 현재 추출본에서는 찾지 못했다.

그래서 낙하는 **원본 영상을 보고 저작한 값**이다: 14m 위에서 30도 기울여,
가운데 둘 200~700ms, 바깥 둘 750~1250ms. 착지 뒤에 이어지는 1.87m 끌림만
matinee 값이다. 나중에 원본 트랙을 찾으면 이 네 줄을 갈아 끼우면 된다.

### 파티클·자막·사운드를 왜 못 넣는지

```text
파티클  이 엔진에서 map effect 는 CMapEffectPresentationRuntime 이 소유하고,
        활성화 창이 MAP_EFFECT_ACTIVATION_WINDOW { patternId, stageId } 로
        Server 보스 스테이지 클럭에 묶인다(Update_ServerPresentation). 클래스
        주석이 가짜 보스 owner 를 만들지 말라고 못박고 있다. world sequence 에서
        effect 를 쏘는 계약은 없다. 넣으려면 시퀀스 문서에 effect cue 배열을
        추가하고 MapTool 이 시계로 spawn 하는 새 수직 슬라이스가 필요하다.
자막    layout JSON 으로 image widget 을 만드는 runtime factory 와 UI input
        router 가 아직 구현 전이라고 CLAUDE.md 가 명시한다. ImGui 로 그리는 것은
        제품 UI 승격 금지에 걸린다.
사운드  원본 큐가 akevent 라 추출 대상이 아니고 대체 음원도 없다.
```

## 09-06 네 번째: 파티클을 결국 붙였다

앞에서 "월드 시퀀스에서 이펙트를 쏘는 계약이 없다"고 적은 것은 절반만 맞았다.
다시 파 보니 길이 하나 있었다.

```text
Spawn_WorldRoot        pBossBudgetAndLifetimeOwner 가 null 이면 거절한다
                       (Effect_PresentationService.cpp:4202). 보스 소유 경로다.
CMapEffectPresentation MAP_EFFECT_ACTIVATION_WINDOW 가 {patternId, stageId} 라
                       Server 보스 스테이지 클럭에 묶인다.
Spawn_LevelPlacement   레벨 인덱스, placement id, effect asset id, 월드 행렬만
                       받고 보스를 묻지 않는다. RootWorld 를 그대로 통과시켜
                       스케일도 먹는다. 이게 열려 있는 문이었다.
```

`CMainApp` 이 매 프레임 `Advance_ProductCuePreparation` 과
`Commit_PendingSpawns` 를 이미 돌리므로, 시작할 때 target 을
`Queue_ProductTargets_Priority` 로 큐에 넣어 두면 준비는 알아서 진행된다.

내용은 카탈로그가 이미 갖고 있는 것을 쓴다.

```text
5310ms  effect.valtan.carrier-v1.attack.magic-choice.outer.clip-01  scale 2.5
5310ms  effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01  scale 2.5
        테이블 상판 위 (0.8552, 7.02, 731.1442). 원본이 쿠크의 대사가 끝나며
        여는 청록 파문과 붉은 링 자리다.
700ms   effect.valtan.ground-roar.rock.explode  가운데 의자 둘 착지 지점
1250ms  effect.valtan.ground-roar.rock.explode  바깥 의자 둘 착지 지점
        원본이 의자가 책에 닿을 때 터뜨리는 먼지 자리다.
```

전부 fail-closed 다. 준비되지 않은 target 은 `Spawn_LevelPlacement` 가 false 를
반환하고 그 cue 만 건너뛴다. 컷신 나머지는 영향을 받지 않는다. 종료할 때
`Stop_WorldRoot` 로 전부 정리한다.

스케일 2.5 와 1.4 는 발탄 아레나용으로 저작된 이펙트를 이 테이블에 맞추려고
어림한 값이다. 화면을 본 적이 없으므로 첫 확인 뒤 조정해야 한다. 자막과 사운드는
여전히 없다 - layout runtime factory 가 구현 전이고 원본 사운드는 akevent 다.

## 09-06 다섯 번째: 자막과 사운드도 붙였다

"없어서 못 한다"고 두 번 적었는데 둘 다 틀렸다. 찾아보니 둘 다 있었다.

```text
자막  Engine 의 CFont_Manager 가 이미 한글 SpriteFont 를 갖고 있고
      (Client/Bin/Resources/Fonts/YoonGasiIIM.spritefont), CGameInstance 가
      Draw_Text / Measure_Text 를 중계한다. CharacterSelectWindowView 와
      ChatWindowView 가 이미 이 경로로 한글을 그린다. UI layout runtime
      factory 는 image widget 용이고 자막에는 필요 없었다.
      CMapTool::Render 에서 3900~10200ms 동안 화면 아래 가운데에 그린다.
      MapTool.cpp 는 BOM 없는 UTF-8 인데 빌드가 /utf-8 을 주지 않아 MSVC 가
      949 로 읽는다(이미 C4819 경고가 난다). 그래서 한글은 \uXXXX 로 적었다.

사운드 추출된 사운드 풀에 이 장면의 곡이 그대로 있다.
      Sound/KoukuSaton/S_BGM_COMMANDERRAID/
      bgm_midnightc_ed_m06_scene_movetobook__220452744.wav (2.6MB).
      Play 에서 Play_Music(volume 0.55, loop false), teardown 에서 Stop_Music.
      _WIN64 가드 안에 둔다. 원본 대사 음성은 여전히 없다 - akevent 라
      추출 대상이 아니었다.
```

이펙트 배율은 어림값으로 두지 않고 Cutscene Arena Preview 에 슬라이더 둘을
붙였다(Ring scale / Dust scale, 0.25~12). 저작 문서가 데칼 2.0 과 파티클 4.8 을
말하는데 그게 반지름인지 지름인지 문서가 정하지 않아 계산으로는 못 좁힌다.
한 번 보고 맞추면 그 값이 세션 동안 유지된다.

### 최종 자원 확인

```text
시퀀스 바인딩            49개 전부 대상 존재
이펙트 3종               카탈로그에 존재, 저작 문서 5KB / 68KB / 124KB
음악                     파일 존재 2,634,542 bytes
한글 폰트                YoonGasiIIM.spritefont 존재
cl /Zs MapTool.cpp       exit 0, 경고 없음
Publish-MapAuthoring     3277 placements, FileCount 7
git diff --check         공백 오류 없음
```

## 09-06 여섯 번째: 자막·사운드 제거, 그리고 아레나에 서 있던 테이블

사용자 요청으로 자막과 배경음악을 뺐다. `Render_GamblingSubtitle`, 자막 상수,
`Play_Music`/`Stop_Music` 호출과 음악 상수를 전부 제거했다. MapTool 에 남은
비ASCII 자막 리터럴도 사라졌다.

그리고 런타임 스크린샷에서 1관문 서커스 아레나 한가운데에 접힌 도박판 테이블이
서 있는 것이 확인됐다. 원인은 단순하다.

```text
Deploy placement 는 INTACT 로 스폰된다. 다른 상태를 원하면 누군가 말해 줘야 한다.
Level_KakulSaydonArena 는 종이다리와 컷신 보스를 그렇게 눌러 두는데, 이번에 추가한
8(테이블)과 9(쿠크)에 대해서는 아무도 말해 주지 않아 제품 레벨에서 그대로 섰다.
MapTool 도 마찬가지여서 Area 를 열면 편집 화면에도 서 있었다.
```

두 곳 모두에서 눌렀다.

```text
Level_KakulSaydonArena  KAKULSAYDON_GAMBLING_PREVIEW_PLACEMENT_IDS {8, 9} 를
                        로드 시 DESPAWNED 트랜잭션에 함께 넣는다. 기존
                        hiddenBridges 는 이제 둘을 다 담으므로 suppressedOnLoad
                        로 이름을 바꿨다.
MapTool                 Load_Placements 성공 끝에서 Set_GamblingPropsRevealed(false)
                        를 호출한다. 컷신이 시작할 때만 다시 보인다.
```

검증: `cl /Zs` 로 MapTool.cpp 와 Level_KakulSaydonArena.cpp 둘 다 exit 0,
자막·사운드 잔재 0건, `git diff --check` 공백 오류 없음.

## 09-06 마지막: 전부 제거

사용자 요청으로 도박판 컷신을 통째로 걷어냈다. 이 문서는 조사 내용(Matinee 추출,
deploy/map 배치 계약, 높이 교차 확인)을 남기려고 보존한다.

```text
코드
  MapTool.cpp                297줄, 함수 6개
  MapTool.h                   26줄, 선언 6개와 멤버 7개
  Level_KakulSaydonArena.cpp  19줄, 억제 상수와 트랜잭션
데이터
  mapplacements     :gambling: 46행     -> 3231행으로 복귀
  deployplacements  8·9 제거            -> HEAD 와 바이트 동일
  deployassets      TABLE 행 제거       -> HEAD 와 바이트 동일
  worldsequences    템플릿 65->62, 인스턴스 69->66
  camerashots       shot.kouku.table 제거, 5->4
  maplights.json    삭제, MapCatalog 의 sourceLights/lights 도 제거
에셋
  DEPLOY_CINE_KOUKU_{TABLE,CHAIR,CANDELABRA,CARDDECK,CARDBASE,CARD_A,CARD_B,CARD_C}
  _GamblingTable 스테이징 폴더
  Tools/ActorXAssetCooker/Cook-KoukuSaydonGamblingTable.ps1
```

남긴 것: 팝업북 컷신 전부, 마리오 시퀀스 루프, self-motion, 사용자의 editor 저작
201행(룰렛 id 40 포함), DEPLOY_CINE_KOUKU_BOOK 과 보스·종이무대 에셋.

검증: `cl /Zs` 로 MapTool.cpp 와 Level_KakulSaydonArena.cpp 둘 다 exit 0,
소스와 데이터에 `gambling` 문자열 0건, 선언만 남은 멤버 0건, 시퀀스 바인딩
696개 중 끊어진 것 0건, 카메라 샷 4개 모두 유효, publish 3231 placements /
FileCount 6, `git diff --check` 공백 오류 없음.
