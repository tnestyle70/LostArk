# LostArk LAN 멀티플레이 접속 구조 학습·검증 계획

> **SUPERSEDED FOR CURRENT IMPLEMENTATION — 2026-08-03**
> 현재 수직 슬라이스는 `127.0.0.1:7777` 고정 Server+Client 실행만 구현한다.
> 이 문서의 LAN 개념 설명은 후속 참고로 보존하지만 ClientLaunchOptions, scenario,
> Client smoke harness에 의존한 구현 절은 적용하지 않는다. 현재 정본은
> `2026-08-03_CHARACTER_SELECT_LEVEL_VERTICAL_SLICE_PLAN.md`다.

## 문서 모드

- 작성 모드: `STRUCTURE_FIRST`
- 비교 기준: `SR_MinecraftDungeons`의 한 서버·다중 Client 접속 구조
- 적용 대상: 현재 LostArk의 `Lobby -> Multiplayer -> Server authority world` 경로
- 이번 문서는 완성 코드 전문을 먼저 제공하지 않는다. 파일의 존재 이유, 헤더 계약, 자료구조,
  함수 흐름, 불변식과 관측 방법을 고정하고 사용자가 구조를 먼저 작성할 수 있게 한다.

## 1. 현재 체크포인트와 이번 단계의 완료 조건

### 1.1 이미 구현된 것

현재 LostArk에는 LAN 멀티플레이에 필요한 제품 경로가 이미 하나 존재한다.

```text
Server.exe --bind-address 0.0.0.0
-> CTcpListener가 모든 로컬 IPv4 interface의 7777을 listen
-> 여러 Client가 host PC의 실제 사설 IPv4:7777로 connect
-> Client별 CClientSession 생성
-> C2S_ENTER_WORLD
-> WORLD_ID별 CGameRoom에 session 등록
-> S2C_ENTER_ACCEPTED + 기존 player spawn + 자기 spawn
-> 기존 Client에는 새 player spawn broadcast
-> 같은 room의 모든 Client에 30 Hz world snapshot broadcast
```

현재 Lobby도 `Multiplayer`를 선택하고 IPv4/port를 입력한 뒤 Bern, Valtan, Training 중 하나를
선택할 수 있다. 같은 `WORLD_ID`를 선택한 Client는 같은 `CGameRoom`에 들어간다. 접속 실패,
5초 승인 timeout, stage 진입 후 disconnect는 Local Preview로 자동 우회하지 않고 Lobby에 남거나
복귀한다.

즉, 새 transport나 두 번째 multiplayer 구현이 필요한 상태는 아니다. 현재 부족한 것은
“동시에 네 Client가 접속해 서로의 spawn과 같은 snapshot player 집합을 받았다”는 자동 회귀
증거다. 기존 `Invoke-NetworkEndpointSmoke.ps1`은 실제 LAN IPv4와 listener PID를 확인하지만
한 번에 Client 하나만 검증한다.

### 1.2 사람이 바로 실행하는 현재 절차

Host PC:

```text
Server/Bin/Debug/Server.exe --bind-address 0.0.0.0
```

각 Client:

```text
Client 실행
-> Lobby
-> Multiplayer
-> Host PC의 실제 사설 IPv4 입력 (예: 192.168.0.12)
-> port 7777
-> 같은 World 선택
-> Server 승인 후 stage 진입
```

`0.0.0.0`은 Server의 bind 대상이지 Client가 접속할 주소가 아니다. Client는 반드시 Host의
실제 IPv4를 입력한다. 같은 PC에서 시험할 때만 `127.0.0.1` 또는 `localhost`를 사용한다.
다른 PC에서 접속하려면 Windows 방화벽이 `Server.exe` 또는 TCP 7777 inbound를 허용해야 한다.
같은 LAN 밖의 인터넷 접속은 router port forwarding, overlay VPN 또는 relay가 별도로 필요하며
이번 수직 슬라이스에 포함하지 않는다.

### 1.3 이번 단계의 완료 조건

1. 하나의 실제 `0.0.0.0:7777` listener를 정확히 Server PID 하나가 소유한다.
2. 네 Client가 같은 LAN IPv4와 같은 `WORLD_ID`로 동시에 접속한다.
3. 네 Client가 서로 다른 유효 `PLAYER_ID`와 `NET_ENTITY_ID`를 승인받는다.
4. 각 Client가 받은 최신 world snapshot의 player 수가 4다.
5. 각 Client stage의 player layer에도 네 `CCharacter` presentation이 존재하고, 그중 정확히
   하나만 locally controlled다.
6. 네 Client report가 모두 원자적으로 작성되고 exit code가 0이다.
7. 종료 후 Server/Client process와 7777 listener가 남지 않는다.
8. 기존 단일 LAN endpoint, 승인 timeout, disconnect-to-Lobby smoke가 계속 통과한다.

현재 세 World에는 enabled `playerSpawn`이 각각 네 개 있다. `CGameRoom`은 사용 중이지 않은
stable spawn placement를 하나씩 배정하므로 이번 완료 기준은 네 명이다. 다섯 번째 Client는
snapshot 상한 32보다 먼저 spawn 부족으로 입장이 거부된다. 명시적인 입장 거부 packet과
32인 확장은 이번 단계가 아니라 후속 capacity 계약이다.

## 2. 전체 수직 흐름 한 줄

```text
Lobby UI command
-> CLobbyCommandService
-> CLevel_Lobby
-> CNetworkManager TCP connect + C2S_ENTER_WORLD
-> CClientSession receive/parser
-> CServerApp session-to-world routing
-> CGameRoom command queue
-> 30 Hz Join/Spawn/Snapshot authority
-> Client receive queue
-> CNetworkManager main-thread message decode
-> CClientReplication GameObject 생성·상태 적용
-> CCharacter presentation / HUD
```

연결 성공은 세 단계로 구분한다.

```text
TCP_CONNECTED
!= WORLD_ACCEPTED
!= PRESENTATION_READY
```

- `TCP_CONNECTED`: 운영체제가 socket 연결을 만들었다.
- `WORLD_ACCEPTED`: Server가 protocol, world, class, capacity, spawn을 검증하고
  `S2C_ENTER_ACCEPTED`를 보냈다.
- `PRESENTATION_READY`: Client main thread가 spawn/snapshot을 실제 Character에 적용했다.

이 세 상태를 하나의 `bool connected`로 뭉개면 “socket은 열렸지만 입장하지 못한 상태”와
“입장 승인은 받았지만 에셋 생성이 실패한 상태”를 구분할 수 없다.

## 3. Dungeons에서 배울 점과 그대로 복사하지 않을 점

### 3.1 Dungeons가 증명한 구조

`SR_MinecraftDungeons`는 다음 개념을 사용한다.

- Server가 `INADDR_ANY`에 bind하여 LAN Client 접속을 받는다.
- accept한 socket마다 `CSession`을 만들고 session/player ID를 발급한다.
- 로그인한 새 Client에게 기존 player 목록을 보내고 기존 Client에게 새 spawn을 broadcast한다.
- Server game loop가 player state snapshot을 반복 전송한다.
- Client `CNetworkMgr`가 자신의 player ID와 remote player map을 구분한다.

“한 사람이 Server를 띄우고 같은 endpoint로 여러 사람이 들어온다”는 동작은 이 다섯 개념으로
성립한다.

### 3.2 LostArk에서 그대로 복사하지 않는 이유

| Dungeons 형태 | 그대로 복사하지 않는 이유 | LostArk의 현재 대안 |
|---|---|---|
| `CNetworkMgr`가 socket, packet, remote object 생성·update·render를 모두 소유 | transport와 Engine object 수명이 결합된다 | `CNetworkManager -> CLIENT_REPLICATION_EVENT -> CClientReplication` |
| `map<int, CSession*>` raw pointer owner | disconnect와 다른 thread의 `Find()`가 수명 경쟁을 만든다 | `unordered_map<SESSION_ID, shared_ptr<CClientSession>>` |
| stage/session manager singleton 다수 | 전역 파괴 순서와 숨은 결합이 생긴다 | `CServerApp` 명시적 owner, `CGameRoom` value boundary |
| packed C struct 전체를 그대로 send | compiler ABI, endian, 크기, 고정 배열 상한과 강결합된다 | `CPacketWriter/Reader`, versioned Shared message |
| Client 위치와 damage 값을 Server가 중계 | Client가 정답을 만들 수 있다 | intent command 후 Server 30 Hz authority |
| 모든 remote object를 network manager raw pointer container에 저장 | Layer 수명과 이중 owner가 된다 | `CNetObjectRegistry`의 weak handle/generation |

따라서 Dungeons는 동작 개념의 참고자료이고 LostArk 구현의 복사 원본이 아니다.

## 4. 기술 선택과 대안

### 4.1 Dedicated Server process를 유지하는 이유

선택: `Server.exe`를 별도 process로 띄우고 모든 Client가 같은 authority에 접속한다.

이유:

- 이동, 스킬, damage, boss와 spawn을 Client마다 복제 구현하지 않는다.
- Host Client가 종료돼도 Server process의 수명과 분리할 수 있다.
- Client presentation과 Server truth의 경계가 명확하다.
- 같은 executable과 protocol을 local PC, LAN PC, CI smoke에서 재사용한다.

대안인 listen server는 실행 UX는 편하지만 Client process 안에 Server 수명·port·room tick을
끼워 넣어 종료와 권위 경계를 복잡하게 만든다. 현재 팀 규모에서 “버튼 하나로 host”가 필요하면
추후 Client가 Server process를 안전하게 launch하는 orchestration을 만들 수 있지만, gameplay
authority를 Client 내부로 옮기지는 않는다.

### 4.2 TCP를 유지하는 이유

선택: 현재 join, command, spawn, snapshot을 하나의 TCP 연결로 유지한다.

이유:

- enter/spawn/despawn/skill command의 순서와 전달 보장이 중요하다.
- 현재 4인, 30 Hz, 작은 snapshot 범위에서는 구현 복잡도 대비 충분하다.
- `CPacketStreamParser`와 send-all 계약이 이미 harness로 검증된다.

UDP는 movement snapshot의 head-of-line blocking을 줄일 수 있지만 sequence, loss, reorder,
reliability channel, MTU와 fragmentation을 새로 해결해야 한다. 실제 profile에서 TCP 지연이
병목으로 확인되기 전에는 두 번째 transport를 만들지 않는다.

### 4.3 수동 IPv4 입력을 먼저 닫는 이유

선택: Lobby에서 `IPv4/localhost + port`를 명시한다.

대안:

- UDP broadcast discovery: 편하지만 다중 NIC, guest Wi-Fi isolation, firewall 예외와 중복
  server 선택 정책이 필요하다.
- 중앙 matchmaking/relay: 인터넷 플레이에는 적합하지만 인증, 배포, 운영 서버가 필요하다.
- Git/JSON에 Host IP 저장: 개인 환경값을 정본 데이터로 오염시키므로 금지한다.

현재 목표는 팀원이 Host의 사설 IPv4 하나를 공유해 접속하는 것이다. discovery는 현재 manual
endpoint 경로가 4-client harness로 닫힌 뒤 별도 수직 슬라이스로 판단한다.

### 4.4 Client별 receive thread를 현재 유지하는 이유

현재 Server는 접속별 `CClientSession` receive thread를 사용한다. 네 명 규모에서는 각 session의
parser와 종료 흐름을 이해하기 쉽고, room state는 thread가 직접 변경하지 않으므로 안전하다.
수백·수천 session에서는 IOCP가 적합하지만 지금 IOCP로 교체하면 기능 검증보다 lifetime과
completion ownership 구현이 더 커진다. profile과 목표 동시 접속 수가 바뀔 때 결정한다.

## 5. 식별자와 자료구조가 이렇게 나뉘는 이유

### 5.1 세 ID는 서로 대체할 수 없다

| ID | 소유자 | 수명 | 용도 |
|---|---|---|---|
| `SESSION_ID` | `CServerApp` | TCP 연결 한 번 | socket/session routing |
| `PLAYER_ID` | `CGameRoom` | 그 room에 입장한 player | gameplay player lookup |
| `NET_ENTITY_ID` | `CGameRoom` | replicated entity 한 번 | Server/Client 공통 entity identity |

재접속하면 같은 사람이라도 새 `SESSION_ID`를 받는다. `PLAYER_ID`는 Server gameplay record이고,
`NET_ENTITY_ID`는 Client presentation registry까지 건너간다. socket handle, pointer, layer index는
어느 ID의 대체물도 아니다.

### 5.2 Server 자료구조

`std::map<WORLD_ID, unique_ptr<CGameRoom>> m_GameRooms`

- ServerApp가 room의 유일한 강한 owner다.
- `WORLD_ID`로 명시적으로 찾는다.
- 정렬 iteration이 30 Hz room tick 순서를 결정적으로 유지한다.

`std::unordered_map<SESSION_ID, shared_ptr<CClientSession>> m_Sessions`

- session ID lookup이 주 사용 패턴이라 평균 O(1)이 적합하다.
- ServerApp가 장기 강한 owner다.
- callback 처리 중 지역 `shared_ptr`을 복사하면 map lock을 푼 뒤에도 session 수명이 유지된다.

`std::unordered_map<SESSION_ID, WORLD_ID> m_WorldBySessionId`

- move/skill packet에는 world를 반복 전송하지 않는다.
- 인증된 session이 처음 bind한 room으로 command를 route한다.
- 한 session은 동시에 하나의 world에만 존재해야 한다.

`std::deque<ROOM_COMMAND> m_InboundCommands`

- receive thread 여러 개가 생산하고 room thread 하나가 소비한다.
- 도착 순서를 유지한다.
- tick 시작에 `swap`하면 mutex 보유 시간을 짧게 하고 현재 batch를 안정적으로 처리한다.
- command를 vector 중간에서 지우지 않으므로 deque가 흐름과 맞는다.

`std::unordered_map<SESSION_ID, weak_ptr<CClientSession>> CGameRoom::m_Sessions`

- room은 session을 사용할 수 있지만 session 수명을 소유하지 않는다.
- ServerApp와 room이 서로 `shared_ptr`을 보유하는 lifetime cycle을 막는다.

`std::map<PLAYER_ID, SERVER_PLAYER> m_Players`

- gameplay lookup뿐 아니라 snapshot 생성 순서가 필요하다.
- `PLAYER_ID` 정렬 순서로 같은 state가 결정적으로 serialize된다.

역방향 lookup map 두 개:

- `sessionId -> playerId`: command actor를 인증된 session으로 결정한다.
- `netEntityId -> playerId`: replicated entity를 gameplay player로 해석한다.

Client command에는 player ID를 넣지 않는다. 그렇지 않으면 Client A가 Client B의 ID를 적어 보내는
위조 경로가 생긴다.

### 5.3 Client 자료구조

`CPacketStreamParser`

- TCP `recv()` 한 번은 packet 한 개와 일치하지 않는다.
- 부분 frame과 여러 frame이 합쳐진 byte stream을 누적해 정확한 app frame으로 분리한다.

`std::deque<PACKET_FRAME> m_InboundFrames`

- receive worker가 생산하고 main thread `Update()`가 소비한다.
- TCP 도착 순서인 spawn -> snapshot -> despawn을 유지한다.
- main thread가 queue를 swap한 뒤 decode하므로 Engine을 worker에서 건드리지 않는다.

`std::deque<CLIENT_REPLICATION_EVENT> m_ReplicationEvents`

- packet 의미를 Client presentation 의미로 한 번 번역한다.
- 여러 packet 종류의 순서를 하나의 queue에서 보존한다.

`CNetObjectRegistry`

- `NET_ENTITY_ID -> slot index + generation`으로 변환한다.
- Layer가 실제 `CCharacter` 수명을 소유하고 registry는 `weak_ptr`만 보관한다.
- 삭제된 slot이 재사용돼도 generation이 달라 stale handle이 새 Character를 가리키지 않는다.

## 6. 기존 파일별 존재 이유와 책임 지도

### `Server/Public/TcpListener.h`, `Server/Private/TcpListener.cpp`

```text
파일이 존재하는 이유: listen socket의 open/accept/close 수명을 ServerApp에서 분리한다.
한 문장 역할: bind address와 port를 OS listener로 바꾼다.
소유하는 상태: listen SOCKET, 마지막 WSA error.
소유하지 않는 상태: connected Client socket, session, player, room.
생성 시점: CServerApp 멤버 생성.
파괴 시점: CServerApp 소멸 역순.
직접 호출자: CServerApp::Run, Accept_Loop, Shutdown.
직접 피호출자: WinSock socket/bind/listen/accept/closesocket.
실패 경로: bool/INVALID_SOCKET와 Get_LastErrorCode -> ServerApp 로그와 process exit.
```

### `Server/Public/ServerApp.h`, `Server/Private/ServerApp.cpp`

```text
파일이 존재하는 이유: Server process 전체 수명과 thread 경계를 한 owner가 조율해야 한다.
한 문장 역할: listener, sessions, rooms를 생성하고 packet frame을 room command로 route한다.
소유하는 상태: WinSock, listener, rooms, session strong owner map, session-world binding, threads.
소유하지 않는 상태: player transform/HP/action, Client GameObject.
생성 시점: Server main의 stack.
파괴 시점: Run 종료 또는 실패 후 stack unwind.
직접 호출자: Server main.
직접 피호출자: CTcpListener, CClientSession, CGameRoom.
실패 경로: malformed frame/unknown packet/world -> 해당 session close; bootstrap/listener 실패 -> process nonzero.
```

### `Server/Public/ClientSession.h`, `Server/Private/ClientSession.cpp`

```text
파일이 존재하는 이유: 접속한 Client 하나의 socket/parser/receive worker를 한 lifetime으로 묶는다.
한 문장 역할: TCP byte를 frame으로 만들고 ServerApp callback에 전달하며 frame을 send한다.
소유하는 상태: connected SOCKET, parser, receive thread, send mutex, bound player ID.
소유하지 않는 상태: room membership truth, player state, broadcast 대상 결정.
생성 시점: Accept_Loop가 client socket을 accept한 직후.
파괴 시점: ServerApp가 closed queue를 reap하고 마지막 shared_ptr을 놓을 때.
직접 호출자: CServerApp, CGameRoom send 함수.
직접 피호출자: CPacketStreamParser, recv/send/shutdown.
실패 경로: receive/send 오류 -> Request_Close/Notify_Closed -> ServerApp -> room LEAVE -> reap.
```

### `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp`

```text
파일이 존재하는 이유: 같은 World의 gameplay state를 하나의 30 Hz writer가 소유해야 한다.
한 문장 역할: room command를 적용하고 spawn, movement, skill, boss와 snapshot을 결정한다.
소유하는 상태: players, world entities, ID counters, server tick, command queue.
소유하지 않는 상태: listen socket, Client rendering, UI.
생성 시점: CServerApp::Run이 supported WORLD_ID마다 생성.
파괴 시점: ServerApp 종료.
직접 호출자: ServerApp room loop와 packet routing.
직접 피호출자: world bootstrap, gameplay catalog, navigation, skill system, boss brain, ClientSession send.
실패 경로: invalid join -> session close; send 중간 실패 -> Rollback_Join/close; leave -> despawn broadcast.
```

### `Client/Public/NetworkManager.h`, `Client/Private/NetworkManager.cpp`

```text
파일이 존재하는 이유: socket worker와 Client main thread 사이에 frame 경계가 필요하다.
한 문장 역할: endpoint에 connect하고 Shared message를 송수신해 replication event로 바꾼다.
소유하는 상태: server socket, receive thread, parser, inbound frames, local network IDs.
소유하지 않는 상태: Character/GameObject 수명, Level 전환 결정, damage 판정.
생성 시점: function-local singleton 최초 Get.
파괴 시점: process 종료 전 MainApp shutdown.
직접 호출자: Level_Lobby, MainApp, PlayerCommandSink, CClientReplication.
직접 피호출자: WinSock, Shared writer/reader/frame builder.
실패 경로: connect/send/parse/recv 오류 -> last WSA error/connection false -> Lobby 또는 stage recovery.
```

### `Client/Public/LobbyCommandService.h`, `Client/Private/Level_Lobby.cpp`

```text
파일이 존재하는 이유: UI 선택을 socket 호출 및 Level 변경과 분리한다.
한 문장 역할: UI intent를 한 번 stage하고 Lobby main-thread state machine이 소비한다.
소유하는 상태: pending enter command; Lobby의 awaiting-acceptance/deadline/pending world.
소유하지 않는 상태: packet byte, session/player ID 발급, Character 생성.
직접 호출자: 현재 ImGui Lobby, 향후 image UI widget command binding.
직접 피호출자: CNetworkManager와 CSceneTransitionService.
실패 경로: command validation -> status; connect/send/timeout/mismatch -> connection close 후 Lobby 유지.
```

## 7. 핵심 함수 흐름과 상태 변화

### 7.1 Server 시작

```text
main
-> CLI의 --bind-address 검증
-> CServerApp::Run(bindAddress)
-> WORLD_ID별 CGameRoom 생성과 bootstrap ready 검사
-> CWinSockContext::Initialize
-> CTcpListener::Open(bindAddress, 7777)
-> m_isRunning = true
-> Room_Loop thread 시작
-> Accept_Loop thread 시작
```

불변식:

- room이 하나라도 ready가 아니면 listener를 열지 않는다.
- listener open이 실패하면 thread를 시작하지 않는다.
- `m_isRunning == true`일 때 listener와 두 top-level thread의 종료 경로가 존재해야 한다.

### 7.2 Client A의 TCP 접속

```text
Lobby image/ImGui command
-> CLobbyCommandService::Request_EnterWorld
-> CLevel_Lobby::Update가 Try_Consume
-> CLevel_Lobby::Request_EnterWorld
-> CNetworkManager::Close_ServerConnection (stale 연결 정리)
-> Connect_To_Server(host, 7777)
-> nonblocking connect + 1.5초 bounded select
-> blocking receive mode 복귀
-> receive state reset
-> Receive_Loop thread 시작
```

`Connect_To_Server()`가 true를 반환해도 World 입장이 완료된 것은 아니다. 이 함수가 변경하는 것은
socket과 receive worker 상태뿐이다. Level 전환 상태는 변경하지 않는다.

### 7.3 Server accept와 session 생성

```text
CTcpListener::Accept
-> connected socket 반환
-> m_iNextSessionId.fetch_add(1)
-> shared_ptr<CClientSession> 생성
-> m_Sessions.emplace(sessionId, session)
-> session->Start
-> session Receive_Loop
```

`shared_ptr`가 필요한 이유는 ServerApp map이 owner인 동안 receive callback과 room send가 같은
session을 잠시 사용할 수 있기 때문이다. `CGameRoom`은 `weak_ptr`만 보관해 owner가 되지 않는다.

### 7.4 Enter World

```text
Client Send_EnterWorld
-> C2S_ENTER_WORLD { protocolVersion, worldId, classId, nickname }
-> Shared writer + packet frame
-> Send_All
-> Server CClientSession::Receive_Frame
-> CServerApp::On_SessionFrame
-> Shared reader의 지역 message 복원
-> protocol/world/remaining-byte 검증
-> Bind_SessionWorld(sessionId, worldId)
-> REGISTER_SESSION command enqueue
-> ENTER_WORLD command enqueue
```

`ROOM_COMMAND`가 필요한 이유는 session receive thread가 `CGameRoom::m_Players`를 직접 수정하지
않게 하기 위해서다. receive thread는 입력을 검증하고 명령만 생산한다. 실제 상태 변경은 다음
room tick 하나의 thread에서만 일어난다.

### 7.5 Join의 stage와 rollback

```text
CGameRoom::Tick
-> command queue swap
-> REGISTER_SESSION
-> ENTER_WORLD
-> Join 검증
-> 비사용 playerSpawn 선택
-> SERVER_PLAYER 지역값 구성
-> profile/navigation 검증
-> playerId/netEntityId 발급
-> player와 역 lookup map 세 곳 commit
-> session playerId bind
-> S2C_ENTER_ACCEPTED
-> world entities
-> 기존 players를 신규 Client에 spawn
-> 신규 player 자신을 신규 Client에 spawn
-> 신규 player를 기존 Clients에 broadcast
```

send 중간 실패 시 `Rollback_Join(sessionId)`가 player map, entity map, session-player map과 bound
player ID를 되돌린다. 부분 입장 player가 snapshot에 남으면 안 된다.

### 7.6 Client 승인과 presentation

```text
Client Receive_Loop
-> byte append
-> frame ready
-> m_InboundFrames push
-> main thread CNetworkManager::Update
-> Handle_Frame
-> S2C_ENTER_ACCEPTED를 pending acceptance로 저장
-> Level_Lobby::Update가 Try_Consume_EnterAccepted
-> pending world와 accepted world 비교
-> CSceneTransitionService::Request
-> stage Loader
-> CClientReplication::Update
-> spawn event를 CCharacter + CNetObjectRegistry로 commit
-> snapshot을 Character/HUD presentation에 적용
```

Level 전환은 Server 승인 뒤에만 발생한다. 승인 전에 Level을 바꾸면 Loader가 실패하거나 Server가
거부했을 때 원래 Lobby state를 복구하기 어렵다.

## 8. 이번에 실제로 수정할 파일과 이유

이번 수직 슬라이스는 production networking을 새로 만들지 않는다. 기존 다중 session 구조를
실제 네 Client로 검증할 수 있도록 관측 계약만 최소 확장한다.

### 8.1 `Client/Public/ClientLaunchOptions.h`

추가 상태:

```text
이름: ExpectedRoomPlayerCount
타입: std::optional<std::uint32_t>
표현하는 상태: smoke가 완료되기 전에 같은 snapshot에서 관측해야 할 player 수.
owner: CClientLaunchOptions 정본 options.
초기값: nullopt, 일반 제품 실행에는 요구 인원 없음.
값이 바뀌는 함수: Parse의 --expect-room-player-count 처리만.
불변식: smoke 실행에서만 사용하며 1..MAX_WORLD_SNAPSHOT_PLAYERS 범위.
```

기존 `<optional>`, `<cstdint>`로 표현할 수 있다. 이 값은 wire protocol도 Server 설정도 아니므로
Shared나 JSON에 넣지 않는다.

### 8.2 `Client/Private/ClientLaunchOptions.cpp`

추가 parse 흐름:

```text
--expect-room-player-count=N 발견
-> 중복 옵션 거부
-> 정수 전체 소비 확인
-> 1..MAX_WORLD_SNAPSHOT_PLAYERS 확인
-> --smoke와 multiplayer 조합인지 최종 교차 검증
-> ExpectedRoomPlayerCount에 commit
```

parse 도중 바로 global options를 바꾸지 않고 지역 `outOptions`에 stage한다. 잘못된 값이면 기존
runtime options를 부분 변경하지 않는다.

### 8.3 `Client/Public/NetworkManager.h`

추가 public 읽기 함수:

```text
함수명: Get_LatestWorldSnapshotPlayerCount
존재 이유: cohort smoke가 실제 Server snapshot의 room player 수를 관측한다.
호출자: CMainApp::UpdateSmokeHarness와 report writer.
호출 시점과 스레드: Client main thread.
입력: 없음.
반환: 마지막으로 완전히 decode된 snapshot의 Players.size, 아직 없으면 0.
읽는 멤버: m_iLatestWorldSnapshotPlayerCount.
변경하는 멤버: 없음.
외부 부작용: 없음.
```

```text
함수명: Get_LatestWorldSnapshotTick
존재 이유: player count가 이전 연결에서 남은 값이 아니라 현재 Server snapshot에서 왔음을 관측한다.
호출자: smoke/report.
반환: 마지막 valid snapshot의 server tick, 아직 없으면 0.
```

추가 private 멤버:

```text
std::uint32_t m_iLatestWorldSnapshotPlayerCount = 0;
std::uint32_t m_iLatestWorldSnapshotTick = 0;
```

두 값은 `Handle_Frame()`과 smoke/report 모두 main thread에서 접근하므로 atomic이 필요 없다.
receive worker는 raw `PACKET_FRAME`만 queue에 넣고 이 멤버를 쓰지 않는다. 불필요한 atomic은
thread ownership을 흐린다.

### 8.4 `Client/Private/NetworkManager.cpp`

snapshot 관측값 갱신 흐름:

```text
S2C_WORLD_SNAPSHOT frame
-> Read_Message를 지역 snapshot에 수행
-> remaining byte 0 확인
-> protocol validation 성공
-> player count와 server tick을 멤버에 commit
-> snapshot을 CLIENT_REPLICATION_EVENT에 move
```

decode 실패 때 count를 갱신하면 안 된다. `Connect_To_Server()`의 새 연결 초기화와
`Close_ServerConnection()`의 정리에서 두 값을 0으로 되돌린다. 이전 Server의 player 수가 다음
접속의 성공 조건으로 재사용되면 안 된다.

### 8.5 `Client/Private/MainApp.cpp`

CPP 내부에만 둘 관측 구조:

```text
이름: SMOKE_PLAYER_LAYER_OBSERVATION
위치: MainApp.cpp anonymous namespace
상태: player Character 수, locally controlled Character 수.
Public 헤더로 올리지 않는 이유: 제품 기능이 아니라 smoke 내부 한 소비자만 사용하는 지역 타입이다.
```

player layer 관측 흐름:

```text
현재 level의 Layer_Player를 index 0부터 bounded 순회
-> CCharacter만 count
-> Is_LocallyControlled true 개수 count
-> expected count와 비교
-> latest snapshot player count와 비교
-> local controlled count가 정확히 1인지 확인
```

반복 상한은 `MAX_WORLD_SNAPSHOT_PLAYERS`다. 종료 조건은 layer index에서 object가 더 없거나 상한에
도달했을 때다. 무한 index 검색을 하지 않는다.

`UpdateSmokeHarness()`의 multiplayer player-stage 성공 분기 앞에 다음 gate를 둔다.

```text
ExpectedRoomPlayerCount가 없음 -> 기존 smoke 동작 유지
ExpectedRoomPlayerCount가 있음
-> network connected 확인
-> latest snapshot tick != 0
-> snapshot player count == expected
-> Layer_Player Character count == expected
-> locally controlled count == 1
-> 모두 만족한 뒤 기존 scenario 성공 조건 평가
```

report version을 올리고 다음 읽기 전용 증거를 추가한다.

- `localPlayerId`
- `localNetEntityId`
- `acceptedWorldId`
- `latestWorldSnapshotTick`
- `latestWorldSnapshotPlayerCount`
- `presentedPlayerCount`
- `locallyControlledPlayerCount`
- `expectedRoomPlayerCount`

report는 기존처럼 temporary file 작성 후 `MoveFileExW(...WRITE_THROUGH)`로 원자 승격한다.

### 8.6 `Tools/Build/Invoke-NetworkEndpointSmoke.ps1`

기존 파일을 확장하고 같은 역할의 두 번째 LAN smoke script를 만들지 않는다.

추가 함수의 자연어 계약:

```text
함수명: Invoke-MultiClientCohortScenario
존재 이유: listener 하나에 네 실제 Client.exe가 동시에 입장하는 계약을 증명한다.
호출자: script bottom의 정본 scenario sequence.
입력: LAN address, client/server path, timeout, client count 4.
출력: 성공 시 PASS log; 실패 시 throw.
읽는 상태: process exit, report JSON, port owner.
변경 상태: test process와 임시 report directory만.
부작용: Server 1개와 Client 4개 process 시작·종료.
실패 조건: listener owner 불일치, process 조기 종료, timeout, report 불일치, ID 중복, 잔류 process/port.
```

내부 흐름:

```text
7777 clear 확인
-> Server.exe --bind-address 0.0.0.0 시작
-> Server PID가 0.0.0.0:7777 listener인지 확인
-> Client 4개를 서로 다른 report path로 시작
-> 각 Client에 --expect-room-player-count=4 전달
-> 모든 Client bounded wait
-> 모든 exit code 0 확인
-> report 4개 parse
-> snapshot/presentation/local-control count 확인
-> local PlayerId 4개 unique 확인
-> local NetEntityId 4개 unique 확인
-> finally에서 자신이 띄운 process만 종료
-> listener cleanup 확인
```

PowerShell collection은 `List[Diagnostics.Process]` 또는 고정 길이 array로 process owner를 보관한다.
PID 문자열만 저장하지 않는 이유는 exit code와 bounded wait를 같은 process 객체에서 확인하기
위해서다. cleanup에서는 port의 임의 PID를 죽이지 않고 이 함수가 생성한 process만 종료한다.

### 8.7 `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`

추가 static contract check:

- cohort scenario가 기존 `Invoke-NetworkEndpointSmoke.ps1` 안에 존재한다.
- 네 Client 기대 인원 옵션과 report field가 함께 존재한다.
- Server bind는 여전히 명시적 `--bind-address 0.0.0.0`이다.
- Client가 `0.0.0.0`을 접속 주소로 하드코딩하지 않는다.

Audit는 runtime cohort 실행을 대신하지 않는다. 정적 check는 하네스가 실수로 제거되는 것을
막고, 실제 동시 접속 증거는 smoke 실행 결과가 담당한다.

### 8.8 구현 후 갱신할 문서

- `CLAUDE.md`: Host 실행 명령, Host IPv4 공유, Client Lobby 입력, firewall 주의사항, 4-client
  smoke 명령.
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`: session/world/player/entity ID와 UI/Network/
  Server 경계.
- 대응 RESULT: 실제 실행한 구성, 네 report의 unique ID와 player count, 잔류 process 여부.

`AGENTS.md`의 bind와 multiplayer 불변식이 바뀌지 않으면 중복 문장을 추가하지 않는다.

## 9. include와 의존 방향

### `ClientLaunchOptions.h`

```text
include 대상: <optional>, <cstdint>
필요한 타입: optional<uint32_t>
Public 노출 이유: MainApp smoke가 immutable launch option으로 읽는다.
CPP로 숨길 수 있는지: 숨길 수 없다. CLIENT_LAUNCH_OPTIONS의 public value다.
```

### `NetworkManager.h`

```text
include 대상: <cstdint>
필요한 타입: uint32_t
Public 노출 이유: MainApp가 transport의 마지막 valid snapshot 관측값을 읽는다.
CPP로 숨길 수 있는지: 멤버는 private지만 getter 계약은 public이어야 한다.
```

새로 `Character.h`, `MainApp.h`, Engine header를 `NetworkManager.h`에 include하지 않는다.
NetworkManager가 presentation을 알게 되면 Dungeons의 monolithic manager 문제를 다시 만든다.

### `ClientLaunchOptions.cpp`

```text
include 대상: Network/PacketMessages.h
필요한 값: MAX_WORLD_SNAPSHOT_PLAYERS
Public 헤더에 노출해야 하는 이유: 없음.
CPP로 숨길 수 있는지: 가능하며 반드시 CPP에만 둔다.
include 이유: --expect-room-player-count 상한을 실제 wire snapshot 상한과 같은 정본으로 검증한다.
```

숫자 `32`를 Client option parser에 다시 하드코딩하지 않는다. snapshot 상한이 바뀌면 Shared의
한 상수와 그 소비자 검증이 함께 바뀌어야 한다.

의존 방향:

```text
Client UI -> LobbyCommandService -> Level_Lobby -> NetworkManager
NetworkManager -> Shared message/frame
ServerApp -> Shared message -> RoomCommand -> GameRoom
GameRoom -> ClientSession send
NetworkManager -> ClientReplicationEvent -> ClientReplication -> Character
```

금지 방향:

```text
UI -X-> socket/packet
Client GameObject -X-> Server class include
ClientSession receive worker -X-> GameRoom player map 직접 수정
NetworkManager receive worker -X-> Engine GameObject 생성
Client command -X-> playerId/netEntityId actor 지정
```

## 10. 불변식 체크리스트

### Listener와 endpoint

- Server 기본 bind는 `127.0.0.1`이다.
- LAN은 사용자가 `0.0.0.0` 또는 특정 사설 IPv4를 명시할 때만 열린다.
- Client endpoint는 `localhost` 또는 구체적인 IPv4다. `0.0.0.0`은 거부한다.
- Git JSON에 개인 IP와 firewall 설정을 저장하지 않는다.

### Session

- accepted socket 하나당 `SESSION_ID` 하나다.
- `m_Sessions`가 session의 유일한 장기 strong owner다.
- closed notification은 `m_hasNotifiedClosed.exchange(true)`로 한 번만 발생한다.
- receive thread 자신을 자기 안에서 join하지 않는다. room loop의 reap 또는 shutdown이 join한다.

### Room

- session 하나는 world 하나에만 bind된다.
- gameplay containers는 room thread만 변경한다.
- session receive threads는 `ROOM_COMMAND`만 enqueue한다.
- player 하나는 session ID, player ID, net entity ID, spawn placement ID가 모두 유효하다.
- 사용 중인 spawn placement를 다른 live player에게 재할당하지 않는다.
- join 중간 실패는 모든 lookup과 bound player ID를 rollback한다.

### Client

- socket connect만으로 stage를 변경하지 않는다.
- `S2C_ENTER_ACCEPTED` world가 pending world와 같을 때만 transition한다.
- spawn/snapshot GameObject 반영은 main thread에서만 한다.
- 한 Client world에는 locally controlled Character가 정확히 하나다.
- disconnect 시 socket, parser, inbound frames, IDs, snapshot 관측값과 replicated world를 비운다.

## 11. 사용자가 먼저 작성할 범위

개인 계획서 규칙에 따라 첫 구현 순서는 다음으로 제한한다.

### 1단계: 관측 상태 H 계약

사용자가 먼저 작성:

1. `CLIENT_LAUNCH_OPTIONS::ExpectedRoomPlayerCount`
2. `CNetworkManager`의 두 getter 선언
3. `CNetworkManager`의 두 private 관측 멤버

이 단계에서는 CPP를 작성하지 않는다. 각 멤버에 “누가 쓰고, 누가 읽고, 언제 0으로 돌아가는지”를
주석 또는 설명으로 확인한다.

### 2단계: snapshot commit/reset

사용자가 작성:

1. valid `S2C_WORLD_SNAPSHOT`에서만 count/tick commit
2. connect 초기화와 close 정리에서 0 reset
3. getter 구현

### 3단계: smoke gate와 report

사용자가 작성:

1. MainApp.cpp 내부 player layer 관측
2. expected cohort gate
3. report field와 version 갱신

### 4단계: 네 Client orchestration

사용자가 또는 Codex 설정 적용으로 작성:

1. 기존 endpoint smoke에 cohort 함수 추가
2. 네 process bounded wait
3. 네 report cross-validation
4. 자신이 띄운 process만 cleanup

사용자가 실제 코드 정답 또는 직접 반영을 명시하면 이 계획의 H/CPP 전체 함수를 공개·적용한다.

## 12. Breakpoint와 관찰값

Client A와 Client B 두 개부터 확인한 뒤 네 개로 늘린다.

### Client A

1. `CLevel_Lobby::Request_EnterWorld`
   - host, port, world ID, class
   - `m_isAwaitingEnterAcceptance` 변경 전후
2. `CNetworkManager::Connect_To_Server`
   - `connectResult`, `connectError`, `SO_ERROR`
   - receive thread 시작 시 socket 값
3. `CNetworkManager::Send_EnterWorld`
   - protocol version, world ID, class ID

### Server

4. `CServerApp::Accept_Loop`
   - Client A/B의 서로 다른 connected socket
   - 발급된 session ID
5. `CServerApp::On_SessionFrame`
   - packet type, session ID, target room
   - `m_WorldBySessionId` size
6. `CGameRoom::Join`
   - `m_Players.size()` 전후
   - 선택 spawn placement ID
   - 발급 player ID/net entity ID
7. `CGameRoom::Broadcast_Spawned`
   - 제외 session ID
   - send 대상 session 수
8. `CGameRoom::Broadcast_WorldSnapshot`
   - server tick
   - `Players.size() == 2`, 이후 4

### Client main thread

9. `CNetworkManager::Handle_Frame`
   - `S2C_ENTER_ACCEPTED`의 local IDs
   - snapshot tick/player count
10. `CClientReplication::Apply_Spawn`
    - spawned net entity ID
    - local ID와 같은지
    - registry size와 layer object
11. `CMainApp::UpdateSmokeHarness`
    - expected count
    - snapshot count
    - presented count
    - locally controlled count

각 Breakpoint에서 Call Stack과 Thread를 함께 본다. `CGameRoom::Join`은 room thread여야 하고,
`CClientReplication::Apply_Spawn`은 Client main thread여야 한다.

## 13. 프로젝트 설정과 등록

이번 계획은 새 C++ 파일을 추가하지 않는다.

- `Client.vcxproj`: 변경 없음
- `Client.vcxproj.filters`: 변경 없음
- include directory: 변경 없음
- project reference: 기존 `Client/Server -> Shared` 유지
- link library: 기존 `Ws2_32.lib` 유지

새 파일을 만들지 않는 이유는 현재 owner와 호출자가 이미 정확한 파일에 있기 때문이다. 별도의
`CMultiplayerManager`, `CLanManager`, `CSessionManager`를 추가하면 동일 역할의 두 번째 경로가 된다.

## 14. 빌드·실행 검증 순서

구현 후 다음 순서로 검증한다.

```text
1. Shared x64 Debug/Release
2. NetworkProtocolHarness Debug/Release 실행
3. Server x64 Debug/Release
4. Server.exe --contract-test
5. Client x64 Debug/Release
6. Invoke-NetworkEndpointSmoke.ps1 -Configuration Debug
   - 기존 single LAN endpoint
   - 신규 four-client cohort
   - disconnect recovery
   - enter approval timeout
7. Release endpoint/cohort smoke
8. ProjectAudit
9. git diff --check
```

수동 팀 검증:

1. Host가 Debug 또는 Release Server를 `--bind-address 0.0.0.0`으로 실행한다.
2. Host에서 listener PID와 사설 IPv4를 확인한다.
3. 네 PC가 같은 world에 접속한다.
4. 각 화면에서 자신의 Character 하나와 다른 세 Character를 확인한다.
5. 한 Client가 이동하면 나머지 세 Client가 Server snapshot 결과를 본다.
6. 한 Client를 종료하면 나머지 세 Client에서 despawn되는지 확인한다.
7. Server 종료 시 모든 Client가 replicated state를 비우고 Lobby로 돌아가는지 확인한다.

자동 cohort는 join 경로를 닫고, 마지막 이동·despawn 수동 관찰은 다음 자동화 후보로 RESULT에
분리 기록한다. 실제로 자동화하지 않은 항목을 PASS로 쓰지 않는다.

## 15. 이번 단계에서 하지 않는 것

- UDP transport 추가
- LAN broadcast discovery
- Steam/EOS/중앙 matchmaking
- NAT traversal, relay, internet server hosting
- Client process 안의 listen server
- 계정 인증과 재접속 token
- party roster와 raid admission
- 다섯 명 이상 room capacity
- generic Monster 계약

이 항목들은 네 Client가 현재 한 Server room에서 확실히 연결되는 실행 계약을 닫은 뒤 각각 별도
수직 슬라이스로 검토한다.

## 16. 학습 완료 질문

구현 전에 다음 질문에 답할 수 있어야 한다.

1. `0.0.0.0`과 Host의 `192.168.x.x`는 각각 누가 사용하는 주소인가?
2. TCP connect 성공과 world enter 승인 성공은 왜 다른 상태인가?
3. 왜 receive thread가 `CGameRoom::m_Players`를 직접 수정하면 안 되는가?
4. `SESSION_ID`, `PLAYER_ID`, `NET_ENTITY_ID`를 하나로 합치면 어떤 수명 오류가 생기는가?
5. 왜 ServerApp는 session을 `shared_ptr`, GameRoom은 `weak_ptr`로 들고 있는가?
6. 왜 room command는 `deque`이고 tick 시작에 지역 deque로 swap하는가?
7. 왜 `m_Players`는 정렬된 `map`이고 session lookup은 `unordered_map`인가?
8. 왜 Client의 receive worker가 Character를 직접 생성하지 않는가?
9. Join에서 player map을 먼저 넣은 뒤 send가 실패하면 어떤 상태를 rollback해야 하는가?
10. 왜 네 Client smoke의 player count는 packet 수가 아니라 valid snapshot과 presentation을 함께
    확인해야 하는가?

이 질문에 답하고 Debug/Release cohort 결과까지 확인하면 이번 단계가 완료된다.
