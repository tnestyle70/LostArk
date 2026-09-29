# 2026-09-30 마하라카 복귀 트리거 — 항구가 아니라 들어왔던 바다로 RESULT

빌드와 Client/Server 실행은 하지 않았다(사용자가 마지막에 한 번 빌드). 화면 확인은 사용자 몫이다.

## 1. 현재 동작 실측 (수정 전)

- `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json` 의 `island.exit.to.bern` 은 이미 `changeLevel` 이벤트에 선택 필드 `spawnPlacementId` 를 갖고 있었고 값이 `npc.bern.ship.harbormaster.1`(항구 NPC)이었다.
- 서버 경로: `CServerTriggerSystem::Build_WorldTransfer`(ServerTriggerSystem.cpp:948)가 `outTransfer.strSpawnPlacementOverrideId = action.strTargetId` 로 싣고, `GameRoom_Admission.cpp` 의 입장 staging 이 그 배치를 `Find_Placement` 로 찾아 **그 배치 전방 2.5 m 지점에 서고 배치를 바라보게**(yaw + 180°) 한 뒤 navigation 에 투영한다. 그래서 항구 NPC 자리로 갔다. 도착 위치 지정 기능은 이미 있었고 값이 항구였을 뿐이다.
- 제약 두 가지: (1) staging 이 override 배치의 `isEnabled` 를 요구한다. (2) 발행기는 enabled triggerBox 에 이벤트 정확히 1개를 요구한다. 즉 "아무것도 안 하는 착지 표식"을 enabled 로 만들 수 없다.
- 배 탑승: 월드 이동은 새 `SERVER_PLAYER` 를 만들어서 `iVehicleId`, 부두(dock) 상태가 버려진다. Bern 도착은 항상 도보였다. 탑승 상태는 스냅샷 `iVehicleId` 만으로 Client 가 표현(`Apply_NetworkVehicle`, HUD `Get_Player().iVehicleId`)하므로 서버 상태만 복원하면 Client 는 추가 변경이 필요 없다.

## 2. 바꾼 것

### 데이터
- `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json` (revision 925 → 926): 착지 표식 `island.return.sea.landing` 추가.
  disabled triggerBox, position [424.02, 10.95, -473.8], yaw 111.2, halfExtents [1,1,1], events [] (기존 `jump*_1` 표식과 같은 모양).
- `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json` (revision 276 → 277): `island.exit.to.bern` 의 `spawnPlacementId` 를 `island.return.sea.landing` 으로 교체. 다른 필드 변경 없음.

### 서버 코드 (프로토콜 변경 없음, 122 그대로)
- `Server/Private/GameRoom_Admission.cpp`
  - override 배치가 **disabled 이더라도 kind 가 TRIGGER_BOX 이면** 착지 표식으로 허용(그 외 disabled 는 기존대로 거부).
  - 탑승 복원: Bern 입장이고 override 가 있으면 세션 id 로 기억된 배 정보를 찾아 `iVehicleId`, `bShipDockValid`, 부두 좌표를 채운다. 기록이 없으면 도보로 선다.
- `Server/Public/GameRoom.h`: `SHIP_RETURN_STATE`(배 id + 부두 x/y/z/yaw) 와 `m_MaharakaShipReturnBySession`(세션 id → 상태) 추가.
- `Server/Private/GameRoom.cpp`: Bern 이 월드 이동 요청을 넘기는 지점에서, 대상이 MAHARAKA 이고 플레이어가 배를 타고 있으면 그 정보를 기록하고, 그 밖의 Bern 출발이면 기록을 지운다. 256개가 넘으면 비운다.

### 도착 위치 계산 (하드코딩 없이 표식 배치 데이터에서 유도)
- 서버 규칙: 도착점 = 표식 위치 + 2.5 m × (sin yaw, cos yaw), 바라보는 방향 = yaw + 180°.
- 목표: 배 정박점 (427.75, 10.95, -475.25) 에서 바다 쪽으로 1.5 m 더 나간 지점. 정박점은 내비 금지 구역(섬 + 6 m 여유)의 가장자리 셀이라 바로 그 셀에 세우면 경계에 걸린다.
- 결과: 도착점 (426.35, 10.95, -474.70), 바라보는 방향 291.2° (섬 원점 (440, -480) 에서 멀어지는 서북서 바다 쪽).
- BernSea 내비 원본(`LV_BER_BERNCASTLE.BernSea.navsource`)으로 확인: 도착점 셀 걸을 수 있음, 가장 가까운 막힌 셀까지 1.5 m, 정면 8 m 앞까지 모두 걸을 수 있음.

## 3. 트리거 재진입 루프

- 도착점은 입항 트리거 `island.dock.to.maharaka` (430.1, -476.64, half 8) 의 상자 안이다. 그러나 이 트리거는 `requiresInteract: true` 라서 G 키를 눌러야만 발동한다. 도착만으로 다시 마하라카로 튕기지 않는다. 도착 직후 `마하라카 썸머 캠프 [G]` 프롬프트가 뜨는 것은 정상이다.
- 착지 표식 자체는 disabled + events 없음이라 트리거로 동작하지 않는다.

## 4. 배 탑승 상태

- 배를 타고 마하라카로 갔다면: 돌아올 때 같은 배에 탄 채, 출발 전 부두(dock)를 그대로 물려받아 바다에 선다. 하차하면 처음 배를 탔던 항구 부두로 돌아간다(기존 `End_ShipVoyage` 동작).
- 배 기록이 없는 경우(예: Debug 로비에서 마하라카로 직접 입장한 세션): 도보로 바다 위 도착점에 선다. BernSea 격자는 걸을 수 있는 영역이라 이동은 되지만 물 위를 걷는 모습이 된다. H 로 배에 타면 그 자리를 부두로 삼는다.
- 기록은 Bern 방이 세션 id 로 들고 있다. 월드 이동에서 세션 id 는 그대로 이어진다(ServerApp 의 REGISTER_SESSION 이 같은 iSessionId 를 쓴다). 서버를 재시작하면 기록은 사라진다.
- 프로토콜, 탑승 규칙, Client 코드는 바꾸지 않았다.

## 5. 실행한 검증

- `cl /Zs`(출력 없는 구문 검사): `GameRoom_Admission.cpp`, `GameRoom.cpp` 오류 0(코드 페이지 C4819 경고만). 제품 빌드는 하지 않았다.
- 두 Gameplay.world.json JSON parse 통과. `git diff --check` 통과(CRLF 경고만).
- 게시: `Publish-WorldGameplay.ps1 -Mode Publish -WorldId BERN`(18 s, 69 배치), `-WorldId MAHARAKA`(17 s, 42 배치). 게시본 확인:
  - `BERN.worldbootstrap` 헤더 revision 926, `island.return.sea.landing` 행 존재(enabled 0).
  - `MAHARAKA.worldbootstrap` 의 `island.exit.to.bern` 행이 `changeLevel 2 BERN island.return.sea.landing`.
  - Client 뷰어 문서 두 개에 새 id 포함.
- Map Area 게시는 필요 없었고 하지 않았다.

## 6. 사용자가 해야 할 일 / 확인할 것

- **Server 재빌드 후 재시작 필요.** 서버 C++ 3개 파일이 바뀌었다(Client 코드 변경 없음). 게시본 반영을 위해서도 Server 재시작이 필요하다.
- 확인: 베른에서 배로 섬 앞에 가서 G 로 마하라카 입장 → 마하라카에서 `island.exit.to.bern` 상자에서 G → 베른 바다의 섬 앞(서쪽)에 같은 배를 탄 채 나타나는지, 방향이 섬 반대쪽 바다인지, 하차 시 항구 부두로 돌아가는지.
- 방향은 섬에서 멀어지는 쪽(291.2°)으로 정했다. 원작이 항구 쪽(약 332°)을 보는 것이 자연스러우면 착지 표식 yaw(현재 111.2)와 위치를 함께 바꿔야 한다(도착점 = 위치 + 2.5 m × (sin yaw, cos yaw), 방향 = yaw + 180°).

## 7. 남은 경계

- 화면 확인은 하지 않았다. 배 기록 복원은 코드 리뷰와 구문 검사까지만 했고 실행 검증은 사용자의 재시작 후 확인이 필요하다.
- 같은 문서를 만지는 다른 fork(마하라카 jump 트리거 표시)가 있으므로, 그쪽 변경도 이번 MAHARAKA 게시본에 함께 반영되어 있다(같은 원본에서 게시).
