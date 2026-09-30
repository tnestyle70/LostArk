# 콜로세움 M 월드맵·스퀘어홀 이동 구현 계획

## G00. 현재 실측과 구현 경계

사용자가 콜로세움에서도 M키로 기존 월드맵을 열고 스퀘어홀로 이동하도록 요청했다.
현재 브랜치는 `codex/colosseum-material-restore-20261001`, 기준 HEAD는 `a1329e0c2`다.
기존 재질 복원 및 다른 작업의 미커밋 변경을 보존한다.

`CMainApp::Update`의 M키는 이미 공통 입력이며 전경 창, 텍스트 입력, 연출 중 UI 억제를 따른다.
그러나 `Update_Minimap`과 `Find_ActivePlayerController`는 콜로세움을 연결하지 않는다.
`CWorldMapWindowView::Update`는 지도와 local snapshot이 없으면 창을 숨긴다.
현재 지도 데이터와 목적지 `squarehole.1`~`squarehole.3`은 베른에만 있다.
`CPlayerController::Request_UseSquareHole`와 기존 `C2S_USE_SQUAREHOLE`는 그대로 재사용한다.

콜로세움에서 열리는 창은 기존 베른 목적지 지도다. 다른 월드에 있는 플레이어 좌표를 베른 지도에
그리지 않고 지도 전체와 베른 스퀘어홀을 표시한다. 새 지도 그림, UI, packet 또는 local teleport는
만들지 않는다. 인트로 중 입력 억제와 ESC, modal 확인, UI pointer 소비는 기존 흐름을 유지한다.

## G01. Client 연결 파일과 계약

| 파일 | 변경 위치와 책임 |
|---|---|
| `Client/Public/Level_Development.h` | `Get_DebugCamera` 다음에 기존 controller 접근과 replica marker 수집을 공개한다. 새 owner는 만들지 않는다. |
| `Client/Private/Level_Development.cpp` | `Update`의 Maharaka 전용 승인 pump에 Colosseum을 포함하고 실제 `m_eLevel`을 넘긴다. |
| `Client/Private/MainApp.cpp` | `Find_ActivePlayerController`와 `Update_Minimap`에 활성 Colosseum owner를 연결한다. |
| `Client/Private/WorldMapWindowView.cpp` | `Update`에서 현재 월드와 표시할 목적지 지도를 구분한다. 베른 default area, labels, holes, zone tree를 재사용한다. |
| `Client/Public/WorldMapWindowView.h` | Colosseum 목적지 탐색과 marker 억제 계약을 기존 Update 설명에 반영한다. |

기존 H/CPP의 인코딩과 개행을 유지하고 MainApp의 profiler 수정, PlayerController의 우클릭
최적화는 보존한다. 새 C++/Data 파일이 없으므로 프로젝트와 filter 등록은 추가하지 않는다.

### CMainApp → Level owner → 기존 view

M edge가 창을 열면 `Update_Minimap`이 활성 Colosseum replica의 local snapshot을 수집한다.
미니맵은 Colosseum 지도 행이 없으므로 기존대로 숨기고 월드맵은 베른 default destination map을
선택한다. 지도 중심은 전체 영역 가운데이며 Colosseum의 local/party/boss/NPC 좌표는 표시하지
않는다. 현재 위치와 플레이어 표시 조작, 베른 전용 출항 요청은 목적지 탐색 중 비활성이다.
스퀘어홀 위치/트리/확인창은 기존 베른 데이터를 사용한다. Colosseum 귀환에는 요금 차감이
없으므로 이 경우에만 확인창의 표시용 요금과 실링 아이콘을 숨긴다.

### 확인 → typed command → Server 승인 → Level 전환

기존 확인창은 hole ID를 한 번 소비하고 닫힌다. MainApp이 활성 Colosseum controller에 전달하고
controller가 유효한 presentation과 NONE action을 확인한 뒤 기존 command sink로
제출한다. Server 담당은 Colosseum의 hole1~3 요청을 베른 `squarehole.N`으로 resolve하고 기존
stage/commit transfer를 사용한다. 실패하면 기존 room/player를 유지한다. Client는
`Pump_ServerApprovedWorldTransfer(m_eLevel)`로 승인된 Bern 진입만 소비한다.
승인 없는 local transform 변경이나 별도 socket 호출은 없다.

## G02. Server 연동 범위

Server 담당은 기존 `Handle_UseSquareHole`, 입장 stage 및 Colosseum→Bern transfer 경로를
확장한다. 기존 Bern 노래/암전/동일 월드 이동, 예약 출항 ID65535는 유지한다. 새 packet과
protocol version 변경은 없다. 목적지 ID 범위, busy/dead 거절, Server navigation 착지,
session/닉네임/직업/인벤토리·장비·내구도 보존, 실패 시 기존 room 유지를 계약 검사한다.
PlayerId와 NetEntityId는 기존 world transfer처럼 목적지 room에서 발급한다.

| 파일 | 변경 위치와 책임 |
|---|---|
| `Server/Private/GameRoom_Internal.h` | 베른 목적지 stable placement ID 1~3을 판별하는 기존 room 내부 helper를 둔다. |
| `Server/Private/GameRoom_PlayerCommands.cpp` | `Handle_UseSquareHole`의 기존 idle 검증 뒤 Colosseum 요청만 singleton batch transfer로 stage한다. 동일 session의 중복 대기 요청은 거절한다. |
| `Server/Private/GameRoom_Admission.cpp` | `Stage_PlayerEntry`의 베른 squarehole override는 NPC 접근 거리 2.5m를 적용하지 않는다. 실제 MOVE_PLAYER 좌표를 기존 정확한 navigation/height/collision 검증으로 승인한다. 출항 선박 복원은 적용하지 않는다. |
| `Server/Private/GameRoom_PartyWorld.cpp` | `Transfer_PartyTo`의 기존 target admission → initial frames → reliable FIFO reserve → source Leave → target commit을 Colosseum singleton에도 사용한다. 새 party/roster를 만들지 않고 남아 있는 source party만 기존 규칙대로 갱신한다. |
| `Server/Private/ServerGameplayContractTests_DebugTeleport.cpp` | 기존 Bern song 검사에 Colosseum 목적지 3개, invalid/busy/dead/중복, target admission 및 송신 실패 보존, 성공 후 정확한 좌표와 identity/inventory 보존을 추가한다. |

`ServerApp::Transfer_SessionWorld`는 이미 batch를 호출하고 성공 시에만 session world binding을
갱신한다. 이 consumer와 Shared packet은 변경하지 않는다. 목적지 party identity와 roster 없이
한 session만 이동하며, 기존 raid party transfer 분기는 유지한다. 배치가 준비되기 전 source
position, action, party, session binding을 변경하지 않는다. 새 C++ 파일과 프로젝트 등록은 없다.

## G03. 검증과 종료 증거

1. 변경 파일의 diff와 기존 dirty hunk 보존, `git diff --check`를 확인한다.
2. Client 변경 translation unit의 Debug/Release 최소 구문 검사를 실행한다. 실제 product build는
   통합 담당이 기존 실행 파일 점유와 사용자 승인을 확인한 뒤 진행한다.
3. Server 담당의 Colosseum hole1~3 성공, 잘못된 ID/busy/dead 거절, Bern 기존 경로 회귀 검증을
   연결한다. 구조 검사와 실제 UI 실행 결과를 구분해 RESULT에 기록한다.
4. 사용자는 Colosseum 인트로 뒤 M → 베른 스퀘어홀 선택 → 확인 → Bern 착지를 확인한다.
   창 위 클릭이 이동 명령으로 중복 소비되지 않는지, ESC가 확인창/지도 순서로 닫히는지,
   M 토글과 텍스트 입력·전경 focus가 기존처럼 동작하는지도 직접 확인한다.

Client/UI 실행·조작, 화면 캡처와 실행 프로세스 종료는 수행하지 않는다.
