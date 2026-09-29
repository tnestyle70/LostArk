# 2026-09-29 마하라카 섬 입항·복귀 (베른 ↔ 마하라카 changeLevel) RESULT

새 바다 레벨은 만들지 않는다. 베른의 배 바다(fork 1이 확장)에 마하라카 섬 외형이 놓이고, 이 문서는
그 선착장에서 G로 `BERN -> MAHARAKA`로 이동하고 마하라카에서 베른 항구로 돌아오는 경로를 다룬다.
빌드와 Client/Server 실행은 하지 않았다(리드가 마지막에 한 번 빌드). 화면 확인은 사용자 몫이다.

## 1. 조사로 확인한 것 (코드 근거)

- 서버 이동 파이프라인은 월드 종류에 무관하다. `CServerApp::Transfer_SessionWorld`
  (`ServerApp.cpp:5012~5227`)가 대상 방에 REGISTER/ENTER를 넣고 원래 방에서 `Leave`한 뒤 binding을 바꾼다.
  마하라카 방은 서버 시작 때 이미 공유 시뮬레이션으로 만들어진다(`ServerApp.cpp:2562`).
- 막고 있던 곳은 세 군데뿐이었다: `Build_WorldTransfer`의 대상 월드 목록(BERN/VALTAN_ARENA만),
  `WorldBootstrap` 파서의 대상 월드 파서, 게시자의 changeLevel 검증(Bern/Valtan 짝만).
- 탈것: 목적지 방은 새 `SERVER_PLAYER`를 만들어 `iVehicleId`가 기본값(없음)이고, 원래 방은
  `Leave`로 플레이어를 지운다. 그래서 배에서 이동해도 별도 하차 코드가 필요 없고, 마하라카에는 배 상태가 넘어가지 않는다.
  서버 트리거 평가는 탑승 여부를 보지 않으므로 배 위에서 G가 동작한다(`Activate_Interact`, `ServerTriggerSystem.cpp:370~412`).
  `Build_WorldTransfer`는 액션이 `NONE`일 때만 허용하는데, 쾌속 항해(부스트)는 `iShipBoostEndTick`만 쓰고
  `VEHICLE_SKILL` 상태로 들어가지 않으므로 항해 중에도 `NONE`이다. 일반 탈것 스킬(Q/W/E)을 쓰는 중에는 G가 막힌다(의도).
- 트리거로 발화한 이동은 소지품·재화를 싣지 않아 매번 초기 지급으로 리셋되던 것을 확인했다
  (`Handle_ReturnToBern`만 실었다). 마하라카 왕복에서는 `CarriedInventory/CarriedPurse/HonorTitle`을 싣도록 했다.
  Bern↔Valtan 트리거는 기존 동작을 유지한다.
- 클라이언트: 이동 승인 뒤 레벨 전환은 `CLevelTransitionService::Pump_ServerApprovedWorldTransfer`가 하며
  `switch(accepted.eWorldId)`에 `MAHARAKA`가 없었고, 마하라카 셸(`CLevel_Development`)은 이 펌프를 호출하지 않았다.
  Loading 레벨은 `m_eNextLevelID` 범용 처리라 `Request_Load(LEVEL::MAHARAKA)`가 로비 경로와 같이 동작한다.

## 2. 변경 (프로토콜 변경 없음, 현재 121 유지)

| 파일 | 변경 |
|---|---|
| `Server/Private/WorldBootstrap.cpp` | changeLevel 대상에 `MAHARAKA` 허용. payload 2(`changeLevel 2 <world> <landingPlacementId>`)를 받아 `action.strTargetId`에 저장. payload 1은 기존과 동일 |
| `Server/Private/ServerTriggerSystem.cpp/.h` | `Build_WorldTransfer`가 대상 `MAHARAKA` 허용, 출발/도착이 마하라카면 소지품·재화·칭호를 실음, 요청에 착지 배치 ID(`strSpawnPlacementOverrideId`) 전달 |
| `Tools/WorldPipeline/Publish-WorldGameplay.ps1` | changeLevel: 대상 `BERN/VALTAN_ARENA/MAHARAKA`, 마하라카는 베른과만 연결, 선택 필드 `spawnPlacementId`(없으면 기존 행 그대로), `interactAction`에 `dock:<이름>` 허용(1~32자, 서버 행에는 실리지 않는 표시 전용) |
| `Client/Private/LevelTransitionService.cpp` | 승인 월드 `MAHARAKA` -> `LEVEL::MAHARAKA` |
| `Client/Private/Level_Development.cpp` | 마하라카 셸 Update에서 위 펌프 호출(베른/발탄/쿠크와 동일 패턴) |
| `Client/Private/WorldGameplayDocument.cpp` | `WorldId_ToString/Try_ParseWorldId`에 `MAHARAKA`, changeLevel 이벤트의 선택 `spawnPlacementId` 검증·보존·직렬화, 대상 검증에 `MAHARAKA` |
| `Client/Private/InteractKeyPromptView.cpp/.h` | `dock:<이름>` 액션. 이름이 프롬프트 문구가 됨(원작의 `<지명> [G]`). 아이콘은 기존 `Icon_check.png` 재사용(새 리소스 없음) |

## 3. 데이터

- 마하라카 `Gameplay.world.json` revision 275 -> 276: `island.exit.to.bern`(interact-gated changeLevel
  `BERN`, 착지 `npc.bern.ship.harbormaster.1`, 표시 문구 "베른으로 돌아가기") 추가. 위치 (62.7, 20.48, -975.6)는
  서버가 실제로 플레이어를 세우는 해변 스폰 4개의 무게중심이다. `jump1/jump3`, `waterpang.arena.start`와 겹치지 않는다.
- 게시: `Publish-WorldGameplay.ps1 -WorldId MAHARAKA` Validate/Publish 성공(42 placements).
  `MAHARAKA.worldbootstrap` 변경은 헤더(revision 276, 42행)와 새 행 하나뿐이다:
  `island.exit.to.bern ... changeLevel 2 BERN npc.bern.ship.harbormaster.1`. Client 게시본은 변경 없음.
- 복귀 착지 지점 검증: `npc.bern.ship.harbormaster.1`(264.41, 11.79, -204.03, yaw 2.4) 앞 2.5 m
  = (264.51, -201.53)는 게시된 베른 격자에서 `Bern3` 영역이 걸을 수 있는 셀(플래그 1)이고 `BernSea`에서는 걸을 수 없다(0).
  즉 부두 위다. 서버의 override 스폰이 NPC 앞 2.5 m에 세우는 규칙(`GameRoom_Admission.cpp:122~136`)과 맞는다.

## 4. 판단해서 다르게 한 점

- fork 1이 준 잠정 `harbourReturnSpawn`(300, 10.95, -235)은 열린 바다 위다. 도보로 돌아오는 플레이어가
  물에 서게 되므로 쓰지 않고 항구 관리인 앞 부두를 복귀 지점으로 썼다.
- `interactAction`의 `dock:<이름>`은 스키마를 늘리지 않고 문자열 규칙으로 처리했다(게시자 표시 전용 필드).

## 5. 베른 입항 트리거 (fork 1 최종 좌표 반영)

- `island_dock.json` FINAL(dockTrigger 414.25, 10.95, -423.75, half 7/4/7, yaw 155.5)을 그대로 사용했다.
- `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json` revision 922 -> 923, `island.dock.to.maharaka` 추가
  (interact-gated, `interactAction` = `dock:마하라카 썸머 캠프`, changeLevel `MAHARAKA`, 착지 배치 미지정 = 마하라카 기본 스폰).
  CRLF·기존 배열 서식 그대로 텍스트 삽입(백업: 작업 tmp `island_entry_backup`).
- 게시: BERN Validate/Publish 성공(68 placements). `BERN.worldbootstrap` diff는 헤더(923/68)와 새 행
  `island.dock.to.maharaka ... changeLevel 1 MAHARAKA` 하나뿐이다.
- 마하라카 프롬프트 표시용: `Client/Bin/DataFiles/World/LV_OCN_EVENTIS_MHP.viewer.world.json`이 없어
  마하라카 셸의 `CInteractKeyPromptView`가 읽을 파일이 없었다. 게시자의 viewer 출력 대상에 `MAHARAKA`를 추가해 새로 생성했다(신규 추적 파일).

## 6. 남은 것 / 불확실
- **베른에는 G 프롬프트 뷰가 없다.** `CInteractKeyPromptView`는 `CLevel_Development`(마하라카)만 만들고 `Level_Bern`은 만들지 않는다.
  그래서 베른 선착장에서 서버 G 트리거는 동작하지만 `마하라카 썸머 캠프 [G]` 문구는 아직 뜨지 않는다.
  표시하려면 Level_Bern에 뷰를 붙이고 BERN viewer 문서도 게시해야 한다(별도 작업, 이번엔 하지 않음).
- 도보 G가 배 탑승 중에도 오는지는 서버 코드상 허용이지만 실제 입력은 확인하지 않았다(사용자 확인 필요).
- 좌표는 fork 1의 바다 네비/섬 배치 확정값이다. 섬 위치가 움직이면 트리거 좌표를 다시 맞춘다.
- 트리거 이동은 개별 이동이다(파티 일괄 이동 아님). 가이드 동행자는 베른에 남는다.
- 빌드·실행·화면 확인은 하지 않았다.

## REMOVE DOCK TRIGGER (베른 입항 트리거 제거)

사용자 요청: 마하라카 섬을 바다에서 지우기로 해서, 섬을 가리키던 베른 입항 트리거도 지운다.

- 제거: `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`의 `island.dock.to.maharaka` 1행
  (triggerBox (414.25, 10.95, -423.75), half (7,4,7), yaw 155.5, changeLevel MAHARAKA,
  interactAction `dock:마하라카 썸머 캠프`). 이 트리거의 짝 행이나 마커는 없었다.
- revision: 923 -> 924. 커밋된 버전(922)과 비교하면 나머지 67개 행은 정확히 같다.
  편집은 백업 후 바이트 단위·동시 편집 검사를 거쳤고 CRLF와 파일 배치를 유지했다.
  백업: `C:\Users\USER\.claude\jobs\46aea322\tmp\dock_remove_backup`.
- 게시: `Publish-WorldGameplay.ps1 -Mode Validate/Publish -WorldId BERN` 성공(67 placements).
  `Server/Bin/DataFiles/World/BERN.worldbootstrap`은 게시 전후로 헤더(revision 923 -> 924,
  placement 68 -> 67)와 도크 행 1개만 달라졌고, 커밋된 버전과는 헤더 revision 한 줄만 다르다.
  `Client/Bin/DataFiles/World/BERN.npcpresentation.json`은 커밋된 버전과 같다.
- 남은 참조: 베른 월드 폴더의 다른 파일(`bern19.source-coordinates.json`)과 게시된 월드 문서에는
  `island.dock`을 가리키는 것이 없다. 도크 전용으로 남은 프롬프트/뷰어 항목도 없었다.
  (`dock:<이름>` 프롬프트 코드는 남아 있으나 쓰는 트리거가 없다. 코드는 건드리지 않았다.)
- 유지: 마하라카 쪽 복귀 트리거 `island.exit.to.bern`과 서버·클라이언트의 changeLevel 확장
  (BERN <-> MAHARAKA)은 그대로다. Debug Lobby의 Maharaka 입장 후 돌아오는 길로 계속 쓸 수 있다.
- 사용자에게 보이는 것: 배로 (414, -424) 부근에 가도 G 프롬프트도 이동도 없다.
  베른에서 마하라카로 가는 길은 다시 Debug Lobby의 `Maharaka` 버튼뿐이다.
- 하지 않은 것: 베른 맵 배치, 네비, 바다, 미니맵, 이펙트, Server/Shared/Client 코드. 빌드와 실행은 하지 않았다.
