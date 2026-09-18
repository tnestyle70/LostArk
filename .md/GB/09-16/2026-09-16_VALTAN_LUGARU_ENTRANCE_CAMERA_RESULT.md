# 발탄 1관문 입장(루가루) 컷신 — 쿠크 카메라샷 방식 이식 결과

2026-09-16. 요청: `Stage_MiniBoss`를 밟으면 (50.87, 10.14, −81.02)로 이동하면서 1관문 입장 컷신이
나오게 하고, 그 연출을 `루가루_입장컷신`이라는 이름으로 만든다. 방식은 "쿠크 만들던 방식대로".

## 1. 실측으로 확인한 출발 상태 (사실)

- `Stage_MiniBoss`(`Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`)의 저장 이벤트는
  `activateSpawnGroup spawn.valtan.stage02.miniboss` 하나뿐이었다. 이동 이벤트는 없었다.
  화면의 `Server moved player to (...)` 문구는 F1 Debug `Move Player` 응답이다
  (`Client/Private/PlayerController.cpp:1423`).
- `Publish-WorldGameplay.ps1:731`은 enabled triggerBox에 이벤트를 **정확히 1개**만 허용한다.
  따라서 한 트리거가 이동과 스폰을 함께 할 수 없다.
- 발탄 시네마틱 카메라(`Data/Encounters/Valtan/ValtanCinematicCamera.json`)는 복제된 발탄 보스
  스냅샷의 `(patternId, stageId)`로만 선택된다(`Client/Private/Level_ValtanArena.cpp:1501`).
  미니보스 구간에는 발탄 보스가 없으므로(발탄은 `Stage_Boss`의 `activateEncounter`로 등장)
  이 경로로는 트리거 시점에 컷신을 띄울 수 없었다.
- 쿠크는 다른 경로를 쓴다. `LV_LUT_MIDNIGHTC_ED.camerashots.json`의 샷 114개 중 `AUTO` 26개는
  **플레이어가 샷의 `box` 안에 들어오면** 발동하고(`Find_ActiveCameraShot`), `PATTERN_ONLY` 88개는
  보스 패턴이 부른다. 샷은 `CameraShot_ToCue()`로 발탄 시네마틱 cue로 변환돼 같은 재생기
  (`CValtanCinematicCameraController`)를 쓴다. 서버·프로토콜을 거치지 않는 Client 표현 레이어다.
- 발탄 Area는 `Data/Maps/MapCatalog.json`에 `sourceCameraShots`/`cameraShots` 쌍이 없었고
  (쿠크에만 있었다), 샷 런타임도 `CLevel_KakulSaydonArena`에만 있었다. 그래서 발탄 Map Tool의
  Camera·World Sequence 탭이 비어 있었다.

## 2. 원본 컷신 추출 (사실)

- 원본: `LV_LUT_HEARTRB_ED_SCENE07A` = `978T90T8XHW4FTFB8IWP8IMIWNW6J4.upk`,
  `efseqact_matinee_37`(export 24) / `interpdata_37`, `interplength` 13.000초.
- 활성 감독 트랙은 1개이고 컷도 1개다: `[-0.033초 cam1]`. 즉 원본은 13초 내내 `cam1`이다.
  (꺼진 감독 트랙 2개에 cam3/cam2 컷이 남아 있으나 재생되지 않는다.)
- `cam1`의 전방 벡터는 전 구간 `(0.5, -0.707107, -0.5)`로 **완전히 고정**이고, 위치만
  (41.468, 21.454, −72.381) → (46.814, 22.202, −83.528)로 **12.39m 이동**하는 달리샷이다.
- FOV는 `fovangle` 단일 키 50도(수평). 16:9 환산 수직 FOV = 29.395도.
- 좌표 변환은 기존 `Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py`의 `BASIS`와
  `world_pose`를 그대로 썼다(UE x,y,z → 클라 x, z, −y, ×0.01).
- 교차 검증: 원본 `pc더미1`의 시작 위치가 클라 (50.09, 10.24, −76.35)로, 사용자가 지정한
  착지 좌표 (50.87, 10.14, −81.02)와 같은 자리다. `cam1`의 시선도 그 지점을 향한다
  (t=0 시선 거리 17.020m, lookAt (49.978, 9.418, −80.892)).
- 스크립트와 증거: `out/ValtanLugaruEntrance20260916/`
  (`build_lugaru_entrance_camera.py`, `write_camerashots.py`, `SOURCE_CAMERA.json`,
  `cam1_trajectory.csv`). 원본 패키지는 읽기 전용으로만 접근했다.

## 3. 변경한 것

### 3-1. 데이터

| 파일 | 변경 |
|---|---|
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.camerashots.json` | **신규**. `lostark.camera-shots` v1, revision 1, 샷 1개 |
| `Data/Maps/MapCatalog.json` | 발탄 Area에 `sourceCameraShots`/`cameraShots` 쌍 선언 |
| `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json` | rev 572 → 573 |

샷 내용:

- `shotId` `valtan.lugaru.entrance`, `displayName` `루가루_입장컷신`
- `activation` `AUTO`, `sequenceInstanceId` 빈 문자열
- `box` center (50.87, 10.14, −81.02), halfExtents (7, 4, 7), yaw 0
- `blendInMs` 600 / `blendOutMs` 1200 / `priority` 20 / `defaultHoldMs` 13000
- `cameraTrack` 13000ms, `CATMULL_ROM`, 키 64개. 각 키의 `lookAt`은 원본 상수 전방으로부터
  고정 거리 17.020m 지점이라 원본 화각 방향을 그대로 보존한다.

트리거:

- `Stage_MiniBoss` (49.15, 10.06, −64.61) → 이벤트를 `movePlayer` (50.87, 10.14, −81.02),
  `durationSeconds` 0.800000012, `arcHeight` 0 으로 교체.
- `Stage_MiniBoss_Spawn` **신규** (50.87, 10.14, −81.02), halfExtents (2,1,2), triggerOnce,
  이벤트 `activateSpawnGroup spawn.valtan.stage02.miniboss`. 기존 스폰은 그대로 유지된다
  (한 트리거에 이벤트 2개를 둘 수 없으므로 착지 지점으로 옮겼다).

### 3-2. 코드

| 파일 | 변경 |
|---|---|
| `Client/Public/Level_KakulSaydonArena.h` | `Parse_CameraShots`에 기본값 `{}`인 `expectedAreaId` 추가 |
| `Client/Private/Level_KakulSaydonArena.cpp` | 같은 함수가 전달된 Area를 검사하도록 2줄 변경. `Load_StageMarkers`의 동일 줄은 건드리지 않음 |
| `Client/Public/Level_ValtanArena.h` | `AREA_CAMERA_SHOT` 구조체, 4개 함수 선언, 상태 멤버 12개 |
| `Client/Private/Level_ValtanArena.cpp` | `Load_AreaCameraShots` / `Find_ActiveAreaCameraShot` / `Update_AreaCameraShots` / `Release_AreaCameraShot` 구현, 초기화·프레임·종료 경로 연결 |

설계 경계:

- 문서 파서와 cue 샘플러는 기존 소유자(`CLevel_KakulSaydonArena::Parse_CameraShots`,
  `CameraShot_ToCue`, `CValtanCinematicCameraController::Sample_Cue`)를 그대로 재사용한다.
  발탄에 새로 만든 것은 이 Area의 박스 선택과 블렌드 글루뿐이다.
- 발탄에는 World Sequence 레이어가 없으므로, 쿠크가 시퀀스 시계로 트랙을 샘플링하는 것과 달리
  **샷이 선택된 프레임부터의 자기 시계**로 샘플링한다. `sequenceInstanceId`가 붙은 샷과
  `PATTERN_ONLY` 샷은 발탄에서 로드 단계에서 건너뛴다.
- 샷은 전용 owner(`0x56414C54414E5348`)로 카메라를 잡고, 서버 시네마틱이 활성이면 즉시 놓는다.
  자유 카메라(F6) 중에도 놓는다. 박스 이탈 판정에는 쿠크와 같은 0.5m 히스테리시스를 쓴다.
- 프로토콜은 바꾸지 않았다(현재 88 유지). Server 코드는 이 단계에서는 바꾸지 않았고,
  6절에서 Debug 바이패스 한 곳만 수정했다.

### 3-3. 게시

- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Mode Publish` → FileCount 23, exit 0,
  `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.camerashots.json` 23,831 bytes 생성.
- `Publish-WorldGameplay.ps1 -Mode Publish` → VALTAN_ARENA 159 placements.
  `Server/Bin/DataFiles/World/VALTAN_ARENA.worldbootstrap` 159/160행에 두 트리거 확인.

## 4. 실행한 검증 / 하지 않은 검증

실행:

- `Publish-WorldGameplay.ps1 -Mode Validate` / `-Mode Publish` 통과.
- `Publish-MapAuthoring.ps1 -Mode Validate` / `-Mode Publish` 통과(카메라샷 문서 포함).
- 변경·생성한 JSON 3개 파싱 성공. `git diff --check` 이상 없음.
- 편집한 C++ 4개 파일의 CRLF·UTF-8 보존 확인.
- 서버 부트스트랩 행 직접 확인.

하지 않음:

- **빌드.** 같은 작업 폴더에 Visual Studio가 열려 있어 컴파일을 돌리지 않았다.
  C++를 바꿨으므로 사용자가 Product 빌드를 해야 적용된다.
- **화면 확인.** 실제로 컷신이 잡히는지, 프레이밍이 원작과 맞는지는 사용자 육안 판정이다.

## 5. 남은 경계

- 사용자가 스크린샷으로 가리킨 곳은 `Composition Actions → Boss → Valtan`의 패턴 목록이었다.
  쿠크 방식에서 컷신 카메라의 저작 홈은 **Map Tool의 Camera 탭**이고, 이번 샷도 거기에
  `루가루_입장컷신`이라는 이름으로 나타난다. 같은 이름을 발탄 Boss 트리에도 넣으려면
  `Data/Valtan/Valtan.gameplay.json` + `Valtan.presentation.json`에 패턴을 추가하고
  projector를 다시 돌려야 하는데, 그 경로는 발탄 보스가 스폰돼 있어야 재생되므로
  미니보스 구간에서 실제로 나오는 연출은 되지 못한다. 그래서 이번에는 넣지 않았다.
- 원작에는 `cam1` 외에 PC 더미 8명, 흰늑대·검늑대 등장 애니메이션, 페이드, 사운드가 있다.
  이번에 옮긴 것은 카메라뿐이다. 나머지를 재현하려면 발탄 Area에 World Sequence 레이어
  (`sourceSequences`/`sequences` + `LV_LUT_HEARTRB_ED.worldsequences.json`)를 추가해야 한다.
- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.camerashots.json`은 아직 Git 미추적 상태다.
  맵 런타임 출력은 저작 원본과 함께 커밋하는 규칙이므로 커밋 시 포함해야 한다.

## 6. 1차 적용 후 실패와 근본 원인 (사실)

사용자가 빌드·재시작 후 밟아 보니 여전히 엉뚱한 곳으로 순간이동했다. 원인은 저작 데이터가 아니라
Server의 Debug 전용 경로였다.

- `Server/Private/ServerTriggerSystem.cpp:258-266`의 `m_bDebugValtanStageBypass`가 켜져 있으면
  `Stage_Boss`를 제외한 발탄 스테이지 트리거의 **저작 액션을 실행하지 않고**
  `Build_ValtanStageBypassMove`의 하드코딩 좌표로 `Begin_MovePlayer`를 호출한다.
- 그 표(`ServerTriggerSystem.cpp:362-368`)에 `{ "Stage_MiniBoss", 86.110f, 14.627f, -93.033f }`가
  있었다. 사용자 스크린샷의 (86.27, 15.37, −93.16)이 이 좌표다.
- 이 플래그는 `Server/Private/GameRoom.cpp:92-94`에서
  `WORLD_ID::VALTAN_ARENA == worldId` 하나로 켜진다. Debug 빌드의 발탄 월드면 **항상** 켜지며
  사용자 토글이 없다.
- 즉 `Gameplay.world.json`에 movePlayer를 저작해도 Debug 발탄에서는 실행되지 않았다.
  `Begin_MovePlayer` 자체는 목적지를 그대로 쓰며 navigation 투영을 하지 않는다
  (`ServerTriggerSystem.cpp:425-465`). 목적지가 밀린 것이 아니라 아예 다른 액션이 실행됐다.

정정: 5절 이전에 이 좌표를 F1 Debug `Move Player` 응답으로 설명했던 것은 틀렸다.
해당 문구를 만드는 코드가 `PlayerController.cpp:1423`인 것은 맞지만, 실제 이동을 일으킨 것은
위 Debug 바이패스다.

### 수정

| 파일 | 변경 |
|---|---|
| `Server/Private/ServerTriggerSystem.cpp` | 바이패스 표에서 `Stage_MiniBoss` 행 제거. 주석에 이유 기록 |
| `Server/Private/ServerGameplayContractTests_WorldTriggers.cpp` | 같은 계약 테스트의 대상을 `Stage_2`(94.762, −90.633)로 교체 |

표에는 이미 `Stage_1`이 같은 이유(진짜 액션을 실행해야 함)로 빠져 있었으므로 같은 방식을 따랐다.
`Stage_2`/`Stage_3`/`Stage_Boss`의 기존 지름길은 그대로 남는다.

적용에는 **Server 재빌드와 재시작**이 필요하다. 이 변경은 Server C++이며 프로토콜은 바꾸지 않았다.

## 7. 카메라가 무한 반복되던 원인과 수정 (사실)

Server 수정 후 이동은 맞게 됐으나 사용자가 "카메라가 계속 무한반복하는 느낌"이라고 보고했다.
원인은 6절과 달리 외부 코드가 아니라 3-2절에서 내가 새로 넣은 로직 자체였다.

확인한 사실:

- `CValtanCinematicCameraController::Sample_Cue`는 마지막 키 이후를 **clamp**한다
  (`ValtanCinematicCameraController.cpp:271-275`). 즉 cue 자체는 순환하지 않는다.
- `End_CinematicCamera()` 호출부 8곳은 전부 소멸자·전송·복구·오류 경로이고 매 프레임이 아니다.
- `CCamera_Free`는 `Is_PresentationOverrideActive()`를 존중하므로 follow가 오버라이드와 싸우지 않는다.

내 로직의 결함 두 가지:

1. **종료 조건이 없었다.** 저작 13초가 끝나도 샷을 놓지 않았다.
2. **cue 시계가 카메라 인계에 묶여 있었다.** `m_fAreaShotElapsedSeconds`를
   `shotId != m_strActiveAreaCameraShotId`일 때 0으로 리셋했는데,
   `Release_AreaCameraShot()`이 `m_strActiveAreaCameraShotId`를 비우므로 **어떤 이유로든 한 프레임
   해제되면 다음 프레임에 컷신이 처음부터 다시 재생**된다. 이것이 반복으로 보였다.
   여기에 `Sample_Cue` 실패 시 `0.f`로 다시 샘플링하는 폴백까지 있어 첫 키로 튀는 경로가 하나 더 있었다.

수정(`Client/Public/Level_ValtanArena.h`, `Client/Private/Level_ValtanArena.cpp`):

- `m_strAreaShotClockId` 추가. cue 시계를 **샷 ID에 귀속**시켜 카메라 인계나 일시 해제로는
  되감기지 않게 했다. 카메라 인계 블록과 `Release_AreaCameraShot()`은 더 이상 시계를 리셋하지 않는다.
- `m_strRetiredAreaCameraShotId` 추가. 저작 `durationMs`에 도달하면 그 샷을 은퇴시키고
  `blendOutMs` 동안 follow 카메라로 돌려준다. 플레이어가 그 볼륨을 **벗어나야** 다시 무장한다.
- `Sample_Cue` 실패 시 0초로 재샘플링하던 폴백을 제거하고, 해당 샷을 은퇴시킨 뒤 카메라를 돌려준다.

결과 동작: 트리거 → 이동 → 13초 컷신 1회 → 플레이어 시점 복귀. 박스 안에 서 있어도 재생되지 않고,
나갔다 다시 들어오면 다시 재생된다. 이 변경은 Client C++이므로 **Client 재빌드**가 필요하다.

## 8. 카메라 철회 (팀장 보류 결정)

팀장이 보류를 결정해 카메라 작업 전체를 걷어냈다. 위치 이동은 유지한다.

제거한 것:

- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.camerashots.json` 삭제
- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.camerashots.json` 삭제
- `Data/Maps/MapCatalog.json`의 발탄 `sourceCameraShots`/`cameraShots` 선언 제거 (main과 동일 복귀)
- `Client/Private/Level_ValtanArena.cpp`, `Client/Private/Level_KakulSaydonArena.cpp`,
  `Client/Public/Level_KakulSaydonArena.h` 전체 원복
- `Client/Public/Level_ValtanArena.h`에서 Area 카메라샷 구조체·선언·멤버만 제거.
  3-2절 이전의 Map Tool 접근자 8줄은 유지했다.

유지한 것:

- `Stage_MiniBoss`의 movePlayer (50.87, 10.14, −81.02)와 `Stage_MiniBoss_Spawn` 트리거
- 6절의 Server Debug 바이패스 수정과 그 계약 테스트
- 이전 작업인 플레이어 시작 위치 (8.80, 9.77, −20.22)

검증: `Publish-MapAuthoring.ps1 -Mode Validate` FileCount 22(카메라샷 포함 시 23)로 복귀,
`Publish-WorldGameplay.ps1 -Mode Publish` VALTAN_ARENA 159 placements, 부트스트랩 159/160행 확인.

### 저작 문서 서식 사고와 복구

작업 중 Map Tool 저장이 `Gameplay.world.json`을 통째로 다시 쓰면서 두 가지 오염이 생겼다.

1. 파일 전체가 다른 들여쓰기 서식으로 재작성되어 HEAD 대비 1323 insert / 2587 delete가 됐다.
2. 손대지 않은 collisionBox 약 150개가 float32 왕복 오차를 얻었다
   (예: `165.176488` → `165.176483`). 허용오차 1e-4 밖의 실제 변경은
   `Stage_MiniBoss`, `Stage_MiniBoss_Spawn`, `player_1~8` 뿐임을 대조로 확인했다.

복구: 원본 서식·정밀도를 가진 rev 572 백업을 기준본으로 되돌리고 트리거 수정만 다시 splice해
revision 573으로 올렸다. diff는 79줄로 줄었고 `git diff --check`도 통과한다.
교훈은 이 Area 문서를 편집할 때 Map Tool Save와 스크립트 편집을 섞지 말 것.
