# Lobby fallback session diagnostics 구현 계획

기준일: 2026-08-27

구현 브랜치: `codex/lobby-fallback-diagnostics`

구현 기준 HEAD: `730446cd` (`origin/main`, PR #244 endpoint 갱신 포함)

공유 Wi-Fi endpoint `10.207.18.151:7777` 갱신은 다른 세션의 변경 단위다. 이번 브랜치는
그 endpoint 값을 다시 수정하지 않고, 어떤 endpoint를 사용하더라도 Lobby fallback의 최초 원인을
Client 화면과 Server capture에서 식별한다. 조사 중 확인된 room-slot 누수와 close/entry race는 같은
검증 단위에서 회귀 수정한다.

## 0. 결론과 구현 경계

현재 gameplay 중에는 RTT 또는 snapshot age만으로 Lobby 복귀를 요청하는 Client timeout이 없다.
진행 중 fallback은 실제 socket 종료, protocol fail-close, Client presentation/resource 적용 실패,
Level loading/activation 실패에서 발생한다. 반면 Server에는 한 Client의 send buffer가 250ms 동안
막히면 해당 session을 격리하는 `SO_SNDTIMEO`가 있고, Wi-Fi가 FIN/RST 없이 끊기면 half-open session을
빨리 회수하는 heartbeat/keepalive 계약은 없다.

따라서 "네트워크가 느리다"를 원인으로 미리 확정하지 않는다. 이번 구현은 다음 가설을 한 번의
4인 검증으로 분리할 수 있게 한다.

1. 다른 PC의 Client/Shared binary 또는 Data/Resources revision 불일치
2. Server blocked send timeout, reliable outbound overflow 또는 room ingress overflow
3. Wi-Fi half-open session이 Valtan의 네 playerSpawn slot 중 하나를 ghost로 점유
4. Server-host의 local Client를 포함해 실제로 다섯 번째 session이 입장
5. Client loader, rendering profile, Level initialization 또는 replication presentation 실패

조사 과정에서 추측이 아니라 실제 코드로 두 가지 slot 누수 경로도 확인했다. Server room ingress가
포화되면 `LEAVE`가 거부된 뒤 binding만 사라져 player가 남을 수 있었고, Client의 loading 시작 실패와
일부 replication recovery는 Lobby를 요청하면서 socket을 닫지 않았다. 또한 close와 최초 ENTER가
교차하면 cleanup 뒤 REGISTER/ENTER가 session을 다시 만들 수 있었다. 이 세 경로는 계측만 하지 않고
guaranteed cleanup, 원자 entry admission, Client close 순서로 수정한다.

운영 실패에 runtime `assert()`를 사용하지 않는다. process를 종료하면 Lobby와 capture를 함께 잃기
때문이다. 제품 코드는 typed terminal reason을 first-writer-wins로 보존하고, assertion은 malformed frame,
slow reader, queue overflow와 reason 보존을 검증하는 실행형 harness에 둔다.

### 0.1 완료 조건

- Client가 orderly FIN, receive/send WSA error, invalid frame, parser/raw/event queue overflow,
  entry presentation revision 실패를 서로 다른 stable reason으로 보존한다.
- `Close_ServerConnection()`과 world state reset 뒤에도 최초 terminal reason, endpoint, protocol,
  world/player/entity, packet type, WSA, queue high-watermark와 마지막 수신 시각이 남는다.
- Bern, Valtan, Development, Character Select, Loading과 MainApp activation의 Lobby recovery가 typed
  diagnostic을 `CLevelTransitionService`에 넘기고 Lobby가 reason/source/detail/capture path를 항상 표시한다.
- Client는 connect/enter/accepted/terminal 같은 저빈도 event만 process별 JSONL에 기록한다.
  snapshot payload는 기록하지 않고 마지막 tick/시각과 queue 계수만 집계한다.
- Server는 peer endpoint, session/world/player, 최초 close reason, triggering packet/WSA와 outbound
  queue/drop/coalesce/send 지표를 session reap 전에 stdout과 JSONL에 기록한다.
- Client의 실제 `localEndpoint` IP:ephemeral-port를 `getsockname()`으로 기록해 direct LAN에서 Server의
  `peerAddress:peerPort`와 같은 TCP connection을 결정적으로 대조한다.
- Valtan room full은 정상적인 typed capacity rejection으로, malformed Client, send timeout,
  reliable overflow와 join preflight 실패는 각각 다른 Server reason으로 남는다.
- `LEAVE`는 bounded gameplay ingress와 분리한 session-deduplicated priority cleanup으로 항상 수용하고,
  cleanup 대상 뒤의 non-LEAVE와 close 뒤 entry resurrection을 거부한다.
- 기존 wire payload와 `NETWORK_PROTOCOL_VERSION`은 변경하지 않는다. 1차 검증에서 pre-admission
  동시 접속의 상관성이 부족하다고 실측될 때만 후속 PR에서 Client trace ID를 protocol에 추가한다.
- Client/UI는 에이전트가 실행하지 않는다. 자동 build/harness와 사용자가 수행할 4-PC smoke를
  RESULT에서 분리한다.

## 1. 현재 원인 소실 경로

```text
Client Receive_Loop / Handle_Frame
  -> WSA integer 하나 또는 아무 값 없이 socket 종료
  -> CClientReplication의 pending-connection-loss bool
  -> LevelTransitionService의 source string은 transition 소비와 함께 폐기
  -> Lobby에는 "Server disconnected"만 표시

Server recv / sender / room admission
  -> Request_Close()
  -> On_SessionClosed(sessionId)
  -> reason, peer, packet, queue/send metrics 없이 reap
```

특히 peer의 orderly close는 Client와 Server 모두 WSA 0으로 끝난다. Client의 accepted presentation
revision 검증 실패도 구체적인 `entryAdmissionFailure`를 만든 뒤 `WSAEINVAL` 하나로 축약한다.
Character Select는 presentation 실패와 실제 disconnect를 같은 source로 보고한다.

## 2. 구현 그룹

### G00. stable reason 계약

- `Shared/Public/Network/SessionDiagnostic.h`에 Client/Server가 공통으로 사용하는 append-only
  `SESSION_DIAGNOSTIC_REASON`과 stable ASCII name 변환을 추가한다.
- 이 header는 log schema이지 wire message가 아니다. packet type이나 protocol version을 추가하지 않는다.
- Shared project/filter에 header를 등록하고 unknown reason을 정상값으로 fallback하지 않는다.

### G01. Client network capture

- `CNetworkManager`가 connection generation별 현재 diagnostic snapshot을 소유한다.
- 새 connect 시작만 이전 terminal latch를 초기화한다. intentional close와 world reset은 최초 실패를
  덮거나 지우지 않는다.
- receive worker와 main thread가 함께 쓰므로 diagnostic mutex 아래 commit한다. terminal reason은
  first-writer-wins이며 connect/enter/accepted 같은 non-terminal event는 JSONL에만 append한다.
- JSONL은 module-adjacent `Diagnostics/client-session-<pid>.jsonl`에 저장하고, directory/file 실패는
  gameplay를 막지 않되 Lobby에 capture I/O 상태를 표시한다.

### G02. Lobby recovery handoff

- `CLevelTransitionService`의 load failure one-shot을 `CLIENT_RECOVERY_DIAGNOSTIC`으로 일반화한다.
  기존 `Report_LoadFailure`는 loader 호출자를 위한 wrapper로 유지한다.
- network recovery는 NetworkManager의 latched snapshot을 복사하고, local loader/activation/
  replication 실패는 exact source와 detail을 별도 reason으로 기록한다.
- Bern, Valtan, Development는 connection loss를 요청하기 전에 report한다.
- loading 시작 실패와 Bern/Valtan/Development connection-loss recovery는 원인을 먼저 latch한 뒤 socket을
  닫고 Lobby를 요청해 Server room slot을 남기지 않는다.
- Character Select는 expected Back과 실제 failure를 구분하고, presentation/timeout/disconnect의 원문을
  Level 파괴 전에 report한다.
- Lobby는 마지막 fallback diagnostic을 status와 분리해 항상 보이게 렌더하고 protocol/endpoint,
  reason/source, WSA/packet, world/player/entity, elapsed/last receive, capture path를 표시한다.

### G03. Server terminal capture

- `CClientSession`이 peer endpoint, last inbound packet과 first terminal reason을 소유한다.
- internal recv orderly close/error/parser failure, sender error/timeout, reliable overflow를 typed reason으로
  기록한다. 기존 caller를 깨지 않도록 generic application close default는 유지하되 핵심 admission/
  ingress 경로는 구체적인 reason을 전달한다.
- `CServerApp::On_SessionClosed`는 session erase 전에 world binding, player와 outbound metrics를 snapshot하고
  stdout 한 줄과 module-adjacent `Diagnostics/server-session-<pid>.jsonl`에 기록한다.
- `CGameRoom`은 LEAVE를 일반 ingress cap과 분리해 deduplicate하고 매 tick gameplay보다 먼저 처리한다.
  pending/in-flight cleanup 뒤의 일반 명령을 거부하며 ServerApp은 binding과 REGISTER/ENTER enqueue를
  같은 sessions-lock transaction으로 묶어 close race의 session 부활을 막는다.
- `CGameRoom::Join`의 invalid enter, spawn exhausted, profile/navigation/preflight, accepted/initial sync
  enqueue failure와 expected room full을 구분한다.

### G04. 검증과 4인 smoke

- Server gameplay contract tests에 first reason wins, orderly peer close, invalid frame, reliable overflow,
  slow-reader send failure의 exact reason assertion을 추가한다.
- 기존 NetworkProtocolHarness와 ValtanFourPlayerHarness를 실행해 wire 무변경, 4 accepted, 5th ROOM_FULL,
  disconnect replacement를 확인한다.
- Debug/Release Shared, Server, Client를 빌드하고 Server `--contract-test`, `git diff --check`를 통과시킨다.
- 사용자는 네 PC에서 같은 commit/build/Data/Resources로 실행하고 Lobby의 Client capture path와 Server
  `SessionDiagnostic` line을 UTC/peer/world/player로 대조한다. 평균 ping이 아니라 packet loss,
  blocked-send reason, last receive age와 queue high-watermark를 함께 판정한다.

## 3. 예상 수정 파일

| 구분 | 파일 | 역할 |
|---|---|---|
| 추가 | `Shared/Public/Network/SessionDiagnostic.h` | stable typed diagnostic reason/name |
| 수정 | `Shared/Default/Shared.vcxproj`, `.filters` | 새 public header 등록 |
| 추가 | `Client/Public/ClientSessionDiagnostic.h` | Client snapshot/recovery capture 계약 |
| 추가 | `Client/Private/ClientSessionDiagnostic.cpp` | format, UTC, JSONL capture I/O |
| 수정 | `Client/Default/Client.vcxproj`, `.filters` | 새 Client H/CPP 등록 |
| 수정 | `Client/Public/NetworkManager.h`, `Client/Private/NetworkManager.cpp` | first-terminal-wins transport/protocol 계측 |
| 수정 | `Client/Public/LevelTransitionService.h`, `Client/Private/LevelTransitionService.cpp` | typed Lobby recovery handoff |
| 수정 | `Client/Public/Level_Lobby.h`, `Client/Private/Level_Lobby.cpp` | 마지막 진단 상시 표시 |
| 수정 | `Client/Private/Level_Bern.cpp`, `Level_ValtanArena.cpp`, `Level_Development.cpp`, `Level_CharacterSelect.cpp`, `Level_Loading.cpp`, `MainApp.cpp` | fallback producer의 exact reason 전달 |
| 수정 | `Server/Public/ClientSession.h`, `Server/Private/ClientSession.cpp` | peer/close reason/packet/metrics latch |
| 수정 | `Server/Public/ServerApp.h`, `Server/Private/ServerApp.cpp` | typed close 요청과 structured capture |
| 수정 | `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp` | world admission 분류와 guaranteed priority cleanup |
| 수정 | `Server/Private/ServerGameplayContractTests.cpp` | reason/slow-reader/overflow assertions |
| 수정 | `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, `CLAUDE.md` | 4-PC capture 대조와 runtime 사용법 |
| 추가 | `.md/GB/08-27/2026-08-27_LOBBY_FALLBACK_SESSION_DIAGNOSTICS_RESULT.md` | 실제 diff, 자동 검증, 수동 4-PC pending 기록 |

새 C++ 파일은 Client diagnostic snapshot과 capture formatting이라는 실제 소비자를 가지며 UTF-8 BOM 없이
저장한다. 기존 C++ 파일은 현재 인코딩을 유지하고 ASCII 중심으로 수정한다.

## 4. 제외 범위

- 팀 endpoint 정본, 개인 `.vcxproj.user`, 방화벽과 launch profile 갱신
- Client 자동 실행, UI 조작, 화면 캡처와 visual PASS 판정
- 입증 전 send timeout 수치 변경, TCP heartbeat/keepalive 정책 변경, 별도 ghost session 강제 kick
- 운영 실패를 process crash로 바꾸는 runtime assertion
- 무제한 packet/snapshot dump, nickname 또는 gameplay payload 기록
- 진단 결과 없이 "Wi-Fi 속도 문제"로 결론내리는 것
