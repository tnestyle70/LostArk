# 2026-09-30 마하라카에서 나올 때 타고 들어간 배로 다시 나오기 RESULT

빌드와 Client/Server 실행은 하지 않았다(사용자가 마지막에 한 번 빌드). 화면 확인은 사용자 몫이다. 커밋은 하지 않았다.

## 1. 진단: 왜 배 없이 도보였나

사용자 스크린샷(스크린샷 2026-09-30 023333.png)의 세션은 클라이언트 pid 12604(닉네임 Test-12604)이다. 진단 파일로 실제 진입 경로를 복원했다.

| 시각 | 사건 | 출처 |
|---|---|---|
| 02:23:45~53 | 로비에서 접속, 월드 1(BERN) 입장 승인, playerId 1 / net 154 | client-session-12604.jsonl |
| 02:28:19 | 배 8202 탑승 요청 후 Mounted | EffectFailure.user.log `vehicle.riding` |
| 02:28:20~38 | 항구에서 항해, 속도 약 13 m/s, (403, 10.95, -410) 까지 | 같은 로그 `ShipWake.Bern` |
| 02:29:00 | 월드 6(MAHARAKA) 입장 승인, net 130 | client-session-12604.jsonl |
| 02:30:46 | 월드 1(BERN) 입장 승인, playerId 2 / net 155 (배 없음) | client-session-12604.jsonl |

즉 로비 직접 입장이 아니라 **배를 탄 채 베른에서 입항 트리거 G로 마하라카에 들어간 경로**다. 그런데도 돌아올 때 배가 없었다.

원인(코드 근거): 직전 fork가 넣은 "배 기억"은 `GameRoom.cpp` 의 매 tick 트리거 진입 평가 루프(`Evaluate_Entries` 결과를 `m_PendingWorldTransfers` 에 넣는 곳)에만 있었다. 그런데 입항 트리거 `island.dock.to.maharaka` 는 `requiresInteract: true` 라서 G 키로만 발동하고, G 는 `GameRoom_PartyWorld.cpp` 의 `Handle_InteractTrigger` 를 지난다. 이 함수는 전송 요청을 `m_PendingWorldTransfers` 에 그대로 밀어 넣을 뿐 배를 기억하지 않는다. 그래서 `m_MaharakaShipReturnBySession` 에 기록이 없었고, 돌아올 때의 복원 코드(`GameRoom_Admission.cpp`)가 기록을 못 찾아 도보로 세웠다. 복원 코드 자체와 착지 위치는 정상이다.

같은 이유로 Debug 트리거 재생(`Handle_DebugWorldPlayback` 의 `Debug_Activate`)도 기억하지 못했다.

확인한 정상 항목(원인 아님):
- 복원 후 강제 하차: `Enforce_VehicleRidingState` 의 `Can_RideVehicle` 은 HP 가득, 행동 없음인 새 입장 플레이어를 통과시킨다. 베른은 탈것 월드다.
- 세션 id: 연결이 유지되어 마하라카 왕복에서 같은 sessionId 다(server-session/client-session 모두 sessionId 1).
- Client: `ClientReplication.cpp` 가 스냅샷의 `iVehicleId` 로 `Apply_NetworkVehicle` 을 부른다. Debug 빌드는 첫 탑승 배의 모델을 그 자리에서 준비한다. Client 변경 불필요.

한계: 서버 콘솔(stdout)의 `[ShipBoard]` 줄은 파일로 남지 않아 서버 쪽 기록 부재를 로그로 직접 확인하지는 못했다. 위 결론은 클라이언트 로그의 시간표와 서버 코드 경로 대조에 의한 것이다.

## 2. 수정

기억을 "전송이 방을 떠나는 단일 지점"으로 옮겼다. 방이 전송 요청을 꺼내는 `Try_DequeueWorldTransfer` 는 tick 트리거, G 상호작용, Debug 트리거, 어떤 staging 경로든 반드시 지나가고, 이 시점에 플레이어는 아직 방에 있다(이탈은 그 뒤 `Commit_WorldTransferDeparture`).

- `Server/Public/GameRoom.h`: `Remember_ShipForWorldTransfer(const SERVER_WORLD_TRANSFER_REQUEST&)` 선언 추가.
- `Server/Private/GameRoom.cpp`:
  - tick 루프 안의 기억 블록을 제거하고 같은 내용을 `Remember_ShipForWorldTransfer` 로 옮김(조건과 기록 내용 동일: 대상이 MAHARAKA 이고 `bShipDockValid` 이며 `iVehicleId` 가 유효하면 배 id와 부두 좌표 기록, 그 밖의 Bern 출발이면 기록 삭제, 256개 초과 시 비움).
  - `Try_DequeueWorldTransfer` 가 꺼낸 직후 `Remember_ShipForWorldTransfer` 를 호출.
- 복원 코드(`GameRoom_Admission.cpp`), 착지 표식, 데이터, protocol(122), Client 는 바꾸지 않았다.

스레드: `Try_DequeueWorldTransfer` 는 `CServerApp` 의 방 스레드(`Handle_WorldTransfers`)에서 불리고 복원도 방 스레드에서 실행된다. 이전 코드도 방 스레드에서 같은 맵에 썼으므로 새 경쟁은 없다.

## 3. 상태 전이 추적 (코드 기준)

| 단계 | 위치 | 상태 |
|---|---|---|
| 배 탑승 | `Apply_SetVehicleRiding` -> `Begin_ShipVoyage` | `iVehicleId=8202`, `bShipDockValid=true`, 부두=탑승 위치 |
| 섬 앞에서 G | `Handle_InteractTrigger` -> `Activate_Interact` | 전송 요청이 `m_PendingWorldTransfers` 에 들어감 |
| 방이 요청을 꺼냄 | `Try_DequeueWorldTransfer` -> `Remember_ShipForWorldTransfer` (신규) | `m_MaharakaShipReturnBySession[sessionId] = {8202, 부두, yaw}` |
| 마하라카로 이동 | `Transfer_SessionWorld` -> `Commit_WorldTransferDeparture` | 플레이어는 도보로 마하라카 입장 |
| 마하라카 출구 G | 같은 경로, 대상 BERN + `island.return.sea.landing` | 마하라카는 Bern 방이 아니라 기록 안 건드림 |
| 베른 재입장 | `Stage_PlayerEntry` (`GameRoom_Admission.cpp`) | 기록 발견 -> `iVehicleId=8202`, 부두 복원 |
| 복제 | `GameRoom_Replication.cpp` | 스냅샷 `iVehicleId=8202` |
| Client | `ClientReplication.cpp` -> `Apply_NetworkVehicle` | 배 부착 |
| 하차 | `End_ShipVoyage` | 처음 부두로 복귀 |

## 4. 배 기록이 없는 경우의 정책

로비의 Maharaka 버튼으로 직접 입장한 세션에는 "타고 들어간 배"가 없다. 이번에는 기본 배를 임의로 태우는 정책을 넣지 않았다. 서버는 어떤 배를 골랐는지 모르고(Debug F1 선택은 Client 쪽 값), 부두로 쓸 육지 지점을 정하려면 추정이 필요하기 때문이다. 이 경우는 이전과 같이 도보로 바다 도착 지점에 서고, H 로 배에 타면 그 자리가 부두가 된다. 원하면 별도 결정으로 기본 배와 부두 규칙을 정할 수 있다.

## 5. 실행한 검증

- 인코딩: `GameRoom.cpp` 는 ASCII/CRLF, `GameRoom.h` 는 UTF-8(BOM 없음)/CRLF 이며 패치 후 두 파일 모두 맨 LF 0. 앵커 패치(스크립트와 원본 백업은 잡 tmp `return_ship/`).
- `cl /Zs` 구문 검사(`GameRoom.cpp`, `GameRoom_Admission.cpp`, `GameRoom_PartyWorld.cpp`, `ServerTriggerSystem.cpp`): 네 파일 모두 `cl` 이 파일을 처리했고 `error C` 0건(C4819 코드 페이지 경고만). 제품 빌드는 하지 않았다.
- `git diff --check`: 문제 없음(CRLF 경고만).
- 게시는 필요 없다(데이터 변경 없음).

## 6. 사용자가 할 일

- **Server 재빌드 후 재시작이 필요하다.** 서버 C++ 두 파일이 바뀌었다. 재시작하면 배 기록은 초기화되므로 다시 항해 -> 입항부터 시험해야 한다. Client 코드는 이번 수정으로 바뀌지 않았다.
- 확인 순서: 베른에서 배 탑승 -> 섬 앞에서 G 로 입항 -> 마하라카 출구 G(안내 문구는 기에나의 바다 [G]) -> 같은 배를 탄 채 섬 앞 바다에 도착 -> 하차하면 처음 항구 부두로 복귀.

## 7. 관찰(별개 항목, 조사하지 않음)

이 세션 클라이언트 진단 파일(pid 12604): `main-pump.stall` 이벤트가 여러 번 있었고 스크린샷 FPS 는 1.7 이었다. `EffectFailure.user.log` 의 이 pid 545줄 중 `V1.prepare.slow`/`prepare.renderer`/`prepare.document` 가 각각 140여 건, `V1.queue ... Product target is not prepared` 가 34건이다. 닻 표식(`AnchorMarker.Bern`)은 새 셰이더 program 이 없는 이전 빌드에서 `not prepared` 로 기록된 것으로 보이며, 재빌드 후 재확인이 필요하다.
