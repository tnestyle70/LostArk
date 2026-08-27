# 팀 네트워크 자동 탐색·세션 복구·통합 로딩률 구현 계획

최초 작성일: 2026-08-27 / 조사·계획 갱신일: 2026-08-28

코드 조사 기준: `main`, `0a08b0842afef5975569ea792898310cdb02d305` (PR #248 병합 포함)

문서가 있는 기존 checkout: `codex/team-endpoint-10-207-18-103`, `d77d7e0`. 이 checkout의 다른 작업
미커밋 변경은 조사 기준이나 이번 변경 대상으로 합치지 않는다. 코드 사실은 위 main의 별도 clean checkout으로 확인했다.

상태: **계획서만 갱신. 신규 코드·설정 적용, 빌드, Client 실행, commit/push, PR 생성·merge는 하지 않음.**

최신 사용자 요청은 실제 코드 반영 전 계획 확정이다. 아래의 파일·API·상수·명령은 모두 **구현 예정 계약**이며
현재 기능으로 안내하지 않는다. 대응 RESULT는 구현·검증 뒤에만 만든다. 현재 public endpoint 계약은 계속
`192.168.0.20:7777`이고 `10.207.18.103`은 이전 hotspot 예시다. AGENTS/CLAUDE/팀 사용서의 현재 계약도
이 계획만으로 변경하지 않는다.

## 0. 결론

Server의 `0.0.0.0:7777`은 그 PC의 모든 로컬 IPv4 interface에서 연결을 받겠다는 bind 값이다.
Client가 접속할 수 있는 원격 주소는 아니므로 `Client 0.0.0.0` 계약은 만들지 않는다.

사용자의 번거로움과 이번에 추가된 로딩 표시 문제를 다음 경계로 나눈다.

1. 같은 trusted hotspot/LAN의 Client는 IP를 입력하지 않는다. Client가 UDP directed broadcast로 정본 Server를
   찾고, 응답 datagram의 source IPv4와 광고된 TCP port로 접속한다.
2. 서로 다른 Wi-Fi, NAT 또는 인터넷에서는 LAN broadcast가 전달되지 않는다. 이 경우 Client는
   cancellable `GetAddrInfoExW` 기반 IPv4/FQDN endpoint를 지원하고, 팀이 제공한 VPN/overlay 또는 명시적으로 routing된
   trusted endpoint를 사용한다.
3. 짧은 Wi-Fi 단절과 지연은 즉시 Lobby fallback으로 바꾸지 않는다. Server가 transport detach 뒤 15초 logical
   session lease를 유지하고 same-world Client가 같은 `PlayerId/NetEntityId`와 authoritative state로 resume한다. grace 만료, protocol
   위반, Server 재시작, 명시적 퇴장처럼 복구할 수 없는 경우에만 기존 Lobby 복귀를 실행한다.
4. Loading은 맵·캐릭터·CSO·Effect 단계마다 같은 막대의 분모를 바꾸지 않는다. 한 번의 Level 전환에 고정된
   전체 작업 가중치를 사용하고, 실제 작업 완료만 천천히 표시한다. 긴 작업에서는 진행률을 유지하며 별도
   작업 중 표시를 사용한다. 실제 activation 성공 전에는 100%를 기록하지 않는다.

현재 protocol에는 인증과 TLS가 없다. 따라서 router port forwarding, UPnP, Windows Firewall
`RemoteAddress Any`로 임의 인터넷에 Server를 공개하는 것은 이번 계획에서 금지한다. Cross-network는
trusted routed network의 DNS/IPv4와 정확한 CIDR allowlist까지만 지원한다. 불특정 인터넷에서 별도 설치
없이 접속시키는 authenticated rendezvous/relay는 별도 인프라·보안 계획이다.

이 계획은 4인 room 정원을 늘리지 않는다. 연결이 잠시 끊긴 player도 15초 grace 동안 4인 정원을 계속
점유하며, valid resume만 자기 자리를 되찾는다. grace 중 다섯 번째 신규 입장은 계속 `ROOM_FULL`이다.

## 1. 현재 실측과 직접 원인

### 1.1 현재 설정과 조사 기준

- PR #248이 병합된 조사 기준 main의 공유 endpoint는 `192.168.0.20:7777`, Server bind는 `0.0.0.0:7777`이다.
- 원래 checkout의 필수 sync 결과는 `server-host`, TCP 7777 LocalSubnet rule ready, Client `.20:7777`이다.
  probe는 `not-listening`이므로 이 조사에서 Server가 실행 중이거나 4-PC 접속을 재검증했다고 기록하지 않는다.
- 같은 LAN의 다른 Client도 현재 구현에서는 모두 Server PC의 concrete endpoint `192.168.0.20:7777`을 사용한다.
- `Tools/Network/TeamLanEndpoint.json`, tracked debugger environment와 Git 제외 `.vcxproj.user`가 모두
  이 주소를 주입한다.
- `10.207.18.151`과 `10.207.18.103`처럼 hotspot DHCP 주소가 바뀌는 것은 정상이다. 문제는 주소가
  다르다는 사실이 아니라 Client 설정이 이전 Server 주소를 계속 들고 있는 것이다.
- 이전 hotspot의 연결 성공은 사용자 관찰 이력이며, 이번 자동 탐색·resume·로딩률 계획의 실행 PASS가 아니다.

### 1.2 Client의 현재 연결 흐름

```text
Lobby Begin_NetworkEntry
-> Resolve_ServerHost
   -> LOSTARK_SERVER_HOST
   -> Debug loopback opt-in
   -> compiled 192.168.0.20
-> Connect_To_Server
   -> IPv4 literal/localhost parse
   -> main thread에서 nonblocking connect + select 최대 1.5초
   -> socket을 blocking으로 복원
-> C2S_ENTER_WORLD
-> S2C_ENTER_ACCEPTED 최대 5초 대기
-> Loading/Level이 같은 socket을 계속 사용
```

현재 `Connect_To_Server`는 DNS 이름을 지원하지 않는다. `Resolve_ServerHost()`는 Lobby 표시에서도 매 frame
호출되므로 이 함수 안에서 UDP 탐색이나 blocking DNS를 실행하면 안 된다. 또한 sync script와
`Client.vcxproj`가 `LOSTARK_SERVER_HOST=192.168.0.20`을 항상 넣으므로, 명시 host가 자동 탐색보다 우선하는
정상적인 정책만 추가하면 Visual Studio 기본 실행에서는 discovery가 영원히 실행되지 않는다.

`NetworkManager.cpp:735`의 resolver는 일부 잘못된 명시 host도 현재 default로 되돌린다. AUTO 전환 때는
명시 override의 오류를 `INVALID_NETWORK_CONFIGURATION`으로 보존한다. `:961`의 main-thread `select`도
함께 옮겨야 하므로 UDP 탐색만 worker화하고 전체 접속이 비동기라고 완료 처리하지 않는다.

### 1.3 현재 끊김이 즉시 fallback이 되는 이유

```text
Client recv EOF/error
-> receiveRunning=false
-> Is_Connected=false
-> CClientReplication::Reset_World
-> Bern/Valtan/Development/CharacterSelect가 Lobby 전환 요청

Server recv/send EOF/error
-> On_SessionClosed
-> LEAVE(DISCONNECTED) 즉시 enqueue
-> binding erase
-> CGameRoom::Leave
-> player/entity/session map 삭제 + DESPAWN broadcast
```

지연이 있다는 사실 자체가 fallback 조건은 아니다. 실제 EOF, reset, timeout 또는 socket send/recv 실패가
연결 종료로 관측될 때 fallback이 된다. 다만 Server send socket timeout이 현재 250ms이고 첫 실패를
transport 종료로 승격할 수 있으므로 짧은 Wi-Fi stall이 실제 disconnect로 바뀔 여지는 있다.

현재 구조에서 투명 재접속이 불가능한 직접 이유는 다음과 같다.

- TCP accept마다 새 `SESSION_ID`를 만들고 이 ID가 transport와 gameplay/player/private arena identity를
  동시에 맡는다.
- `C2S_ENTER_WORLD`와 `S2C_ENTER_ACCEPTED`에 resume token, Server boot ID와 grace가 없다.
- reconnect는 새 player/entity admission일 뿐 기존 identity를 찾을 방법이 없다.
- Client는 disconnect 첫 frame에 presentation과 replication registry를 제거한다.
- 기존 socket의 inbound frame과 새 socket의 frame을 구분할 transport generation이 없다.
- send 실패가 transient 복구인지 terminal 종료인지 분류하는 logical-session 정책이 없다. PR #246의
  first-terminal-wins reason/JSONL capture는 이미 존재하므로 이를 새 transport 결과에서도 보존한다.
- `Close_ServerConnection()` 하나가 transient transport close와 terminal session forget을 모두 수행한다.

### 1.4 이미 수정된 cleanup과 새로 확장할 lifecycle 경계

PR #246은 일반 ingress 포화에서 `LEAVE`가 유실되던 결함, close/entry resurrection, Client recovery 시
socket 미종료를 이미 수정했다. `GameRoom.cpp:1020`은 session-deduplicated `m_CleanupCommands`를 별도로
사용하고 `Tick`에서 gameplay보다 먼저 drain한다. 일반 ingress 한도는 best-effort 768 / reliable 960이며,
`ServerApp.cpp:3103` 이후도 `leaveEnqueued` 결과와 invariant 오류를 기록한다.

따라서 이 계획은 해당 결함을 미해결로 재등록하거나 cleanup을 일반 queue로 되돌리지 않는다. 기존
exactly-once cleanup/진단 하네스를 baseline으로 보존하고, 새 DETACH/RESUME/COMMIT을 generation-aware
ordered lifecycle lane으로 확장한다. expiry는 room thread가 같은 lifecycle phase에서 직접 평가한다.

### 1.5 로딩률이 다시 시작하는 직접 원인

`Client/Private/Level_Loading.cpp:156` 이후는 Loader의 현재 작업 비율과 Effect target 비율을 번갈아
`m_fDisplayProgress`에 직접 대입한다. 전환 전체의 누적 분모는 없다. 코드상 다음 순서만으로 역행이 가능하다.

```text
맵 모델 99/100 → navigation(진행량 미상) → character model 0/N → Effect target 0/N
    같은 fill          같은 fill 반복 이동          새 단계 비율로 다시 대입
```

같은 파일의 `N/N`은 determinate 완료가 아니라 indeterminate로 분기하며, `:215`의 `fmod(... + dt * 0.45, 1)`는
같은 fill/glow를 약 2.22초마다 오른쪽에서 왼쪽으로 되돌린다. `Loader.cpp:421`의 `Set_Status()`도 현재 단계
counts를 0으로 초기화한다. 이는 사용자 관찰과 부합하는 **코드 경로 원인**이며 화면 재현 PASS를 의미하지 않는다.

현재 `Loader.cpp:207`의 순서는 `Execute_Load → Run_EffectLoadPreparation`이다. Effect target의 완료 수는
`:389`에서 정확한 main commit ACK를 받은 뒤 증가하므로 이 완료 의미는 바꾸지 않는다. PR #243의 runtime은
HLSL 재컴파일이 아니라 compiled CSO 읽기·검증·device 생성이며, 긴 단일 D3D 호출 중 세부 퍼센트는 없다.
CSO와 맵·Effect 진행을 서로 다른 전체 100%로 그리는 표시 문제와 네트워크 단절은 별개의 원인이다.

또한 Loading은 `:172`에서 이미 100%를 표시하지만 실제 `Create_Level → Activate_Profile → Change_Level`과
Bern pending identity commit은 `MainApp.cpp:4048` 이후다. G09는 실제 activation 완료 ACK까지 전체 epoch를
연결하되, 표시가 100%가 되기를 기다렸다가 activation하는 순환 대기는 만들지 않는다.

### 1.6 다른 작업과 통합할 기준

| 작업/변경 | 이번 조사에서 확인한 상태 | 이 계획의 통합 경계 |
|---|---|---|
| PR #243 Release Effect loading | 조사 기준 main에 반영됨 | compiled CSO, worker device-stage, exact target ACK/rollback을 보존하고 G09 계측만 확장 |
| `Add lobby fallback diagnostics` / PR #246 | 조사 기준 main에 반영됨 | 최초 실패 진단·priority cleanup을 유지하며 generation/lease를 추가 |
| 네트워크 연결 가이드 / PR #248 | 병합됨, 조사 기준 commit | 현재 `.20` 설정과 4인/독립 테스트 안내를 구현 전에는 유지 |
| `Review PR 247 merge readiness` | PR #247 OPEN, 조사 시 head `2775c34c`, 별도 검증 진행 | 해당 변경의 protocol 40과 world/encounter/nav 상태를 먼저 통합 기준으로 확정 |
| `발탄 전멸 동작 연결 수정` | Next Pattern one-slot Server queue 설계 단계 | OrderedSlots/reset 없는 queue의 실제 codec이 구현될 때만 resume full-state/epoch와 함께 연결 |
| `Screen Post와 Camera Shake 확장` | 계획 단계 | 실제 Effect 준비 target/core 소비자가 확정되면 G09 계측에 연결. 별도 UI/runtime이나 가짜 완료 경로를 만들지 않음 |

protocol은 조사 기준 main이 39, PR #247이 40을 사용한다. 여러 계획이 각각 39→40을 적용하면 서로 다른
wire를 같은 version으로 광고한다. **PR #247의 통합 기준 40 위에서 resume와 함께 묶어 배포하는 신규 wire를
41로 한 번 갱신**하는 것을 이 계획의 목표로 둔다. Next Pattern이 같은 통합 배포에 포함되면 그 message도
같은 41 schema에 포함한다. 먼저 독립 배포되어 기준 version이 더 높아졌다면 구현 시작 gate에서
`통합 대상의 실제 최신 version + 1`로 이 계획과 전체 codec/harness 기대값을 함께 갱신한다. 숫자 40/41을
각 브랜치에서 무조건 덮어쓰는 방식은 금지한다.

다른 작업의 dirty diff나 문서만으로 구현 완료를 추정하지 않는다. 통합 시에는 merge된 commit, 실제 consumer,
실행형 검증을 확인하며 이번 계획 작성은 다른 작업에 수정/merge 명령을 보내지 않는다.

## 2. 완료 조건과 제외 범위

### 2.1 자동 탐색 완료 조건

- 기본 Visual Studio 실행은 concrete `LOSTARK_SERVER_HOST`를 주입하지 않고 `AUTO` mode를 사용한다.
- 같은 broadcast domain의 Client는 Server IP를 입력하지 않고 최대 2,000ms 비동기 discovery 창을 거쳐 접속한다.
  첫 응답이 빨라도 창 끝까지 수집해 늦게 발견되는 두 번째 Server를 놓치지 않는다.
- 다중 Wi-Fi/NIC에서도 payload 주소를 신뢰하지 않고 실제 응답 source IPv4를 후보로 사용한다.
- 명시 `LOSTARK_SERVER_HOST=<IPv4|FQDN>`와 Debug loopback fixture는 discovery보다 우선하는 exclusive override다.
- `0.0.0.0`, unspecified/broadcast/multicast endpoint와 잘못된 port는 Client 후보에서 typed reject한다.
- AUTO는 LAN tier를 먼저 끝내고 valid Server가 0개일 때만 routed tier로 간다. 현재 active tier의 LAN 응답들 또는
  routed hello들에서 같은 stable team ID에 서로 다른 boot ID가 둘 이상이면 첫 성공 endpoint에 임의 접속하지
  않고 nickname/enter 전 `AMBIGUOUS_SERVER`로 실패한다. LAN과 routed를 동시에 probe하거나 cross-tier 비교하지
  않는다. 같은 boot ID의 multi-NIC/multi-A 주소만 한 Server 후보로 dedupe한다. 명시 DIRECT는 사용자가 고른
  endpoint만 시도하는 exclusive override라 이 다중 Server 탐색을 하지 않는다.
- discovery가 막힌 AP/client-isolation 환경은 trusted routed FQDN/IPv4 후보로 넘어간다.
- routed 후보도 없거나 실패하면 blind private-IP fallback을 하지 않고 최종 실패 이유와 시도한 source를
  Lobby 상태에 보존한다. `.20`/`.103`은 사용자가 DIRECT로 명시할 수 있는 주소일 뿐 AUTO 고정 후보가 아니다.

### 2.2 연결 복구 완료 조건

- connect, discovery, DNS, retry와 resume 때문에 Client main/UI thread가 1.5초 `select`를 기다리지 않는다.
- 활성 world에서 transient transport loss가 나면 presentation을 유지하고 gameplay input을 freeze한다.
- 15초 grace 안의 valid same-world resume은 같은 `SESSION_ID`, `PlayerId`, `NetEntityId`, world, class와 nickname을
  유지한다. position, HP/resource/action/cooldown/inventory/encounter는 fresh-spawn reset하지 않고 disconnect
  동안 계속 진행된 current authoritative state를 보낸다. disconnect보다 먼저 atomic world transfer가 commit된
  경우만 logical session/sequence를 유지하면서 target room의 새 authoritative IDs/world를 채택한다.
- detach/resume 자체는 다른 Client에 player despawn/spawn을 broadcast하지 않는다.
- offline 동안 달라진 전체 authoritative roster/state는 begin/end generation 안에 stage한 뒤 atomic commit한다.
- old transport frame/command/close callback은 transport generation fence에서 폐기한다.
- queued move/aim/skill command는 새 transport에 재생하지 않는다. reconnect 뒤 새 입력 edge만 전송한다.
- 4인 중 한 명이 grace 상태여도 다섯 번째 fresh enter는 `ROOM_FULL`; valid resume은 admission 없이 rebind한다.
- transient-loss path는 detach 뒤 15초 expiry에서 정확히 한 번 `DISCONNECTED` despawn과 slot release를 수행한다.
  authenticated leave/cancel과 terminal revoke는 각 typed reason으로 같은 cleanup을 앞당길 수 있다.
- explicit leave, protocol violation, invalid/replayed token, Server restart와 resync validation 실패는 자동 retry로
  숨기지 않고 terminal reason을 보존한 뒤 기존 Lobby recovery를 실행한다.
- heartbeat와 liveness clock은 transport worker가 소유한다. CSO/Loader/main-thread 지연 때문에 heartbeat가
  멈추거나 loading elapsed time을 네트워크 timeout으로 쓰지 않는다.

### 2.3 운영·보안 완료 조건

- repo가 소유하는 inbound rule은 Windows Firewall group `LostArk Team Network`와 stable display-name prefix
  `LostArk Team Server`로 식별한다. TCP 7777과 discovery UDP 7778의 Debug/Release `Server.exe` rule은
  `Profile=Private,Domain`, `RemoteAddress=LocalSubnet`, exact protocol/port/program만 허용하며 `Any`/Public profile을
  만들지 않는다. responder도 Network List Manager가 허용한 Private/Domain interface에서만 동작한다.
- schema v2 최초 sync는 현재 v1 script가 만든 ungrouped exact legacy family
  `LostArk Server Debug/Release TCP <port> (LAN)`도 repo-owned migration input으로 취급한다. rule의 program path가
  이 repo의 configuration별 Server.exe와 일치할 때만 삭제하고, 새 grouped expected set으로 교체한다.
- VPN/overlay를 쓸 때만 endpoint 문서에 명시한 trusted Client source CIDR별 TCP rule을 같은 group의 별도 stable name으로
  추가한다. sync는 현재 role/schema/port/program/CIDR의 expected set을 만든 뒤 repo-owned group의 stale port,
  stale CIDR와 이전 role rule을 제거한다. 제거 범위는 이 checkout의 exact Debug/Release program 경로와
  repo-owned 이름/group의 교집합이다. `-Role Client`도 그 범위만 제거하고 다른 checkout의 같은 이름 rule은 보존한다.
- token은 Server CSPRNG로 생성하고 log, discovery payload, nickname 또는 persistent file에 남기지 않는다.
- pending handshake, per-IP transport, total transport와 discovery reply에 bounded cap/rate limit이 있다.
- Server cooperative shutdown은 discovery/accept를 먼저 멈추고 room이 lifecycle을 계속 소비하는 동안 모든
  session producer를 stop/join한 뒤, producer-count 0과 empty-lane barrier를 확인하고 lease cleanup과 room
  stop/join을 끝낸다.

### 2.4 제외 범위

- 4인 room 정원 확대, matchmaking, public account login과 영구 player identity는 다루지 않는다.
- UPnP/NAT-PMP, 자동 router port forwarding, public IP 탐색을 추가하지 않는다.
- TLS, Server certificate, authenticated public relay/rendezvous는 별도 계획으로 남긴다.
- discovery 결과로 임의의 다른 팀 Server에 자동 접속하거나 first-response-wins 정책을 쓰지 않는다.
- disconnect 동안 Client simulation을 locally 계속 진행하거나 입력을 나중에 replay하지 않는다.
- protocol/content/load/presentation 오류를 네트워크 transient로 오인해 무한 retry하지 않는다.
- 에이전트가 Client/UI를 실행·조작하거나 사용자 대신 화면 PASS를 판정하지 않는다.
- 이번 G09는 CSO 내부 byte/GPU instruction 진행률 추정, 시간 기반 가짜 퍼센트, runtime HLSL compile 복구,
  Effect authoring UI 개편을 추가하지 않는다.

### 2.5 통합 로딩률 완료 조건

- 한 번의 실제 Level 전환은 하나의 `loadEpoch`와 전체 100 작업 가중치를 가진다. 맵·캐릭터·CSO·Effect의
  단계 비율은 상태 설명으로 따로 표시하고 전체 막대의 분모로 교체하지 않는다.
- 실제 성공/기존 정책상 허용된 격리 완료량만 credit을 얻으며, 화면은 그 credit을 넘지 않고 단조 증가한다.
- `TARGET_STAGED`만으로 Effect 완료를 세지 않는다. exact target commit ACK와 현재 revision readiness를 유지한다.
- 99%까지는 준비 작업, 마지막 1%는 MainApp의 실제 Level/profile/필요 identity commit 성공이다. smoothing이나
  표시가 100%에 도달하는 것은 activation 조건이 아니다.
- 네트워크 복구 대기는 loading epoch/counters를 초기화하지 않는다. terminal failure/cancel은 마지막 수치와
  원인을 남기고, 사용자가 명시적으로 새 loading/retry를 시작할 때만 새 epoch의 0%를 만든다.

## 3. 목표 구조와 ownership

### 3.1 endpoint mode와 후보 우선순위

Client의 connection mode는 다음 두 종류만 둔다.

| mode | 입력 | 동작 |
|---|---|---|
| `DIRECT` | 명시 `LOSTARK_SERVER_HOST=<IPv4|FQDN>` 또는 Debug loopback opt-in | 해당 endpoint만 시도한다. 실패해도 다른 Server로 몰래 바꾸지 않는다. |
| `AUTO` | 기본 팀 실행 | trusted LAN discovery -> trusted routed endpoint 순서로 bounded 후보를 시도한다. |

현재 TCP listener가 IPv4 전용이므로 `GetAddrInfoExW` 결과도 `AF_INET`만 소비한다. AUTO의 routed FQDN 결과는
endpoint 문서의 trusted Server endpoint CIDR에 포함되는 주소만 후보로 인정한다. DIRECT는 사용자가 명시한 진단 override라
후보 전환과 firewall 변경을 일으키지 않지만, public IPv4는 지원/보안 경계 밖이라는 상태를 표시한다.

`Tools/Network/TeamLanEndpoint.json`의 PR A version 2는 다음 의미를 소유한다.

- 128-bit stable team Server ID를 GUID 문자열로 저장한 값
- 수동 DIRECT rollback 안내용 `rollbackHost=192.168.0.20` (`10.207.18.103`은 hotspot 교체 예시로만 보존)
- TCP port `7777`
- discovery UDP port `7778`
- 기본 Client mode `auto`
- 기존 active-through KST 경계

PR B의 version 3은 routed profile마다 optional trusted FQDN/IPv4,
`trustedServerEndpointCidrs`와 `trustedClientSourceCidrs`를 별도 필수 배열로 추가한다. 전자는 Client DNS pinning,
후자는 Server firewall `RemoteAddress` 전용이며 서로 대신하거나 합집합하지 않는다. PR A에 아직
인증/boot identity를 확인할 수 없는 AUTO routed 후보를 미리 넣지 않는다.

`Tools/Network/Publish-TeamLanEndpoint.ps1`가 이 JSON을 validate해
`Shared/Public/Network/TeamLanEndpoint.generated.h`의 mode, stable team ID, TCP/discovery port와 PR B routed
host/Server-endpoint CIDR/Client-source CIDR를 만든다. Client/Server는 Tools 경로나 working-directory JSON을 runtime에 암묵적으로 읽지 않고
이 generated Shared default를 소비한다. Shared/Server/Client 각 vcxproj의 direct-build pre-build가
`$(ProjectDir)` 기준 repo 경로로 publisher `-Check`를 실행하고 JSON/header drift면 compile 전에 실패한다.
full build도 같은 read-only check를 먼저 실행한다.
generated header는 clean clone의 direct Shared build에도 존재하는 tracked bootstrap이며 Shared project/filters의
`ClInclude`로 등록한다. endpoint schema/value 변경자는 publisher를 실행해 JSON과 header를 같은 commit에 넣고,
build/regression은 생성물을 임의 수정하지 않은 채 먼저 `-Check`한다.

Client runtime override key는 다음으로 고정한다.

- `LOSTARK_NETWORK_MODE=AUTO|DIRECT`: 없으면 generated `AUTO`
- `LOSTARK_SERVER_HOST=<IPv4|FQDN>`: non-empty이면 mode 값보다 우선해 explicit DIRECT
- `LOSTARK_SERVER_PORT=1..65535`: 없으면 generated TCP port
- `LOSTARK_DISCOVERY_PORT=1..65535`: AUTO discovery test/diagnostic override, 없으면 generated UDP port

`DIRECT`인데 host가 없거나, unknown mode, empty/oversize host, `0.0.0.0`, malformed/overflow port는 typed
`INVALID_NETWORK_CONFIGURATION`이고 compiled fallback으로 조용히 바꾸지 않는다. tracked `Client.vcxproj`는
AUTO만 선언한다. 과거 `.vcxproj.user` host row에는 생성 주체 표식이 없으므로 사용자 수동 override와 자동
주입을 추정으로 구분하지 않는다. 최초 `Sync-TeamLanEndpoint.ps1 -UseAutoDiscovery`라는 명시적 전환에서만
해당 debugger host row를 제거하고 network mode/port를 정규화한다. 이후 기본 sync는 host를 재주입하지 않으며,
별도의 명시 DIRECT override가 있으면 그 사실을 출력하고 보존한다. 다른 사용자 environment variable은
항상 유지한다. parent process가 명시한 host와 Debug loopback
opt-in은 기존 우선순위를 유지한다. harness는 unset/AUTO/DIRECT/conflict/malformed env matrix를 검증한다.

effective expected TCP port는 valid `LOSTARK_SERVER_PORT`, 없으면 generated port다. DIRECT와 AUTO routed는 이
port로 connect한다. AUTO LAN은 discovery response의 advertised `tcpPort`를 쓰되 production에서는 effective
expected port와 정확히 같아야 하며 다르면 `DISCOVERY_PORT_MISMATCH`로 후보를 버린다.
`LOSTARK_DISCOVERY_PORT`는 query destination만 바꾸고 advertised TCP port를 override하지 않는다. loopback process
smoke는 test-only env/Server argument로 양쪽 expected TCP port를 같은 dynamic 값에 맞춘다. focused harness는
advertised/expected mismatch, malformed 0/overflow와 matching dynamic fixture를 검증한다.

`Sync-TeamLanEndpoint.ps1 -UseAutoDiscovery`는 Client debugger 환경의 고정 host 제거를 명시적으로 요청하는
새 마이그레이션 옵션이다. 기본 sync가 사용자의 명시 DIRECT 값을 몰래 삭제하지 않는다. stable team ID는
environment override가 아니라 generated Shared header가 정본이다.
사용자가 shell/launch에서 명시한 concrete host는 override로 유지한다. `rollbackHost`는 probe/안내에만 쓰고
자동 접속이나 machine role 판정에 쓰지 않는다.

Server machine role은 Git 제외 `Tools/Network/TeamLanRole.local.json`의 stable local marker가 소유한다.
최초 Server PC에서 `Sync-TeamLanEndpoint.ps1 -Role Server`를 한 번 실행하면 marker의 team ID/role을 검증해
저장하고, 이후 Auto 실행은 DHCP 주소가 바뀌어도 `server-host`를 유지한다. `-Role Client`는 해당 PC를 Client로
명시 전환한다. Server role admission은 이전 `.20` 소유가 아니라 active non-loopback IPv4 존재와
`0.0.0.0` bind 가능성을 검사한다. marker가 없으면 안전하게 Client로 판정한다.

AUTO discovery는 Windows `Private` 또는 `DomainAuthenticated` network profile에 연결된 adapter에서만
실행한다. Public profile은 sync가 명확한 진단을 내고 Client가 discovery를 건너뛰며 profile을 자동 변경하지
않는다. `rollbackHost`는 사용자가 DIRECT override로 명시하지 않는 한 현재 subnet에 있더라도 시도하지 않는다.

hotspot이 Windows에서 Public으로 분류되면 현재 v1의 `Profile Any + LocalSubnet`과 달리 새 정책에서는
바로 탐색되지 않는다. `NETWORK_PROFILE_NOT_TRUSTED`와 adapter 이름을 표시하고, 사용자가 그 hotspot을
신뢰할 때 Windows 네트워크 설정에서 해당 연결만 Private으로 전환한 뒤 sync/재탐색하도록 안내한다.
방화벽 전체 해제·Public 전체 허용·네트워크 프로필 자동 변경은 하지 않는다. UDP broadcast의 unicast 응답이
정책으로 차단되는 경우도 진단 대상이며 Server의 UDP rule 존재만으로 Client 탐색 성공을 보장하지 않는다.
[Microsoft Windows Firewall의 broadcast 응답 정책](https://learn.microsoft.com/en-us/windows/security/operating-system-security/network-security/windows-firewall/configure-with-command-line).

### 3.2 UDP LAN discovery wire

UDP discovery는 TCP `PACKET_FRAME`을 재사용하지 않는 작은 fixed datagram protocol이다.

```text
DISCOVERY_QUERY
  magic
  discoverySchemaVersion = 1
  tcpProtocolVersion = NETWORK_PROTOCOL_VERSION
  stableTeamServerId
  128-bit queryNonce

DISCOVERY_RESPONSE
  magic
  discoverySchemaVersion = 1
  status = OK | PROTOCOL_MISMATCH
  serverTcpProtocolVersion = NETWORK_PROTOCOL_VERSION
  stableTeamServerId
  echoed queryNonce
  tcpPort
  128-bit serverBootId
```

Client는 `GetAdaptersAddresses`와 Network List Manager category를 결합해 active, non-loopback,
Private/Domain의 Preferred IPv4 adapter마다 adapter 주소에 UDP socket을 bind하고 주소별 prefix로 directed
broadcast를 계산해 `:7778`로 보낸다. 송신 시점은 0/500/1,500ms, 전체 수집 창은 2,000ms의 고정
`steady_clock` deadline이다. 첫 query 유실과 느린 hotspot 응답을 허용하되 잘못된 응답이나 UI 지연으로
deadline을 연장하지 않으며 밀린 round를 한 번에 보내는 catch-up burst도 만들지 않는다.

adapter 최대 16개, scan당 수신 datagram 최대 256개, endpoint 후보 최대 32개를 bounded storage로 소유한다.
상한 초과는 `DISCOVERY_LIMIT_EXCEEDED`이며 잘린 후보 목록을 유일한 Server라고 자동 선택하지 않는다.
exact length, magic/schema/protocol/team ID/nonce, packet kind, reserved bytes, nonzero boot ID와 valid TCP port를
모두 검증한다. 유효한 Server가 하나인지는 창 종료 뒤에만 확정한다.

discovery schema v1은 PR A(v40)와 PR B(v41)에서 유지한다. magic/schema/team/nonce가 valid하지만 query의
TCP version이 Server와 다르면 responder는 silent drop 대신 `PROTOCOL_MISMATCH`와 자기 TCP version을 답하고,
Client는 TCP 후보로 쓰지 않는다. 이 typed mismatch는 discovery schema를 아는 PR A 이후 build 사이에서만
보장한다.

접속 IPv4는 response payload에 넣지 않고 `recvfrom` source address를 사용한다. 동일 boot ID가 여러 adapter
주소로 응답하면 interface metric, 응답 RTT와 numeric IPv4의 deterministic order로 후보를 만든다. 다른
boot ID가 같은 team ID로 동시에 응답하면 misconfiguration으로 보고 접속하지 않는다.

같은 boot ID가 상충하는 TCP port를 광고하면 정상 multi-NIC 중복으로 합치지 않는다. 재탐색/취소는
`scanGeneration`을 바꾸며 늦은 응답은 폐기한다. 이 단계의 다중 Server 결과는 선택 UI를 새로 만들지 않고
`AMBIGUOUS_SERVER`와 명시 DIRECT 사용 안내로 끝낸다. 미래 선택 UI도 vector index가 아닌
`scanGeneration + bootId + endpoint`를 제출해야 한다.

Server의 `CLanDiscoveryResponder`는 TCP listener가 정상 open된 뒤 시작한다. normal `AUTO` mode는 TCP bind가
`0.0.0.0`이고 team endpoint config가 있을 때만 `0.0.0.0:7778` responder를 켠다. 명시
`--bind-address 127.0.0.1` 격리 Server는 discovery를 자동으로 끄며, 기존 live harness도
`--disable-discovery`를 전달한다. 실제 discovery smoke만 `--enable-discovery --discovery-address 127.0.0.1
--discovery-port <dynamic> --network-contract-harness`를 명시한다. 이 test-only 조합은 socket이 exact loopback에
bind된 경우에만 loopback ingress를 trusted fixture로 분류하며 `0.0.0.0`/일반 제품 responder에는 적용할 수 없다.
유효 query에만 source endpoint로 응답한다. rate state는 fixed 256 source bucket, 60초 idle expiry,
source별 10/s burst 10과 process-global 100/s burst 200 token bucket을 injected clock으로 운용한다. table이 꽉 찼을
때 expired bucket이 없으면 새 source를 drop하고 allocation/rehash하지 않으며 global bucket이 먼저 소진돼도
drop한다. player, nickname, token, room 상태는 절대 광고하지 않는다.

AUTO를 요청한 Server의 초기 UDP bind, `IP_PKTINFO`/Winsock extension 초기화, 초기 trusted-interface 검증 중
하나라도 실패하면 startup을 실패시킨다. 아직 accept/room producer를 시작하지 않은 상태에서 UDP/TCP listener를
모두 rollback하고 `DISCOVERY_BIND_FAILED`, `DISCOVERY_INITIALIZATION_FAILED` 또는
`NO_TRUSTED_DISCOVERY_INTERFACE`를 원문 OS 오류와 함께 반환한다. TCP만 조용히 남겨 AUTO 준비 완료라고
출력하지 않는다. DIRECT 전용/격리 harness는 명시 `--disable-discovery`일 때만 TCP-only 성공을 허용한다.
이미 실행 중인 Server의 adapter/profile 변화는 process를 종료하지 않고 responder를 pause/rebind하며,
trusted interface가 없으면 `discovery-unavailable`을 명확히 표시한다. 활성 session은 기존 liveness/grace
정책으로 처리한다. UDP 점유/extension 실패/초기 Public-only/실행 중 NIC 변경을 각각 fixture로 검증한다.

stable team ID와 nonce는 잘못된 Server 선택과 stale response를 막는 식별/상관관계 값이지 인증 수단이
아니다. 같은 LAN의 공격자가 response를 위조하는 것을 암호학적으로 막지는 않으므로 discovery는 trusted
LAN 안에서만 사용한다. routed/public 경계의 Server authentication은 VPN 또는 별도 TLS 계획이 소유한다.

### 3.3 Server transport와 logical session 분리

기존 `SESSION_ID`는 stable logical gameplay lease ID로 유지하고, TCP accept마다 별도 `TRANSPORT_ID`와
monotonic generation을 발급한다.

wire에는 Server 전용 header를 노출하지 않고 `Shared/Public/Network/NetworkIds.h`의
`NETWORK_SESSION_ID/INVALID_NETWORK_SESSION_ID`를 사용한다. `ServerIds.h`의 `SESSION_ID`는 이 Shared
정수형의 alias이며 `TRANSPORT_ID`만 Server-local이다.

| owner | 소유 상태 |
|---|---|
| `CClientSession` | socket, peer endpoint, `TRANSPORT_ID`, generation, send/recv worker와 bounded outbound queue |
| `CSessionLeaseRegistry` | stable session/entry attempt, token/generation, world/private binding, last transfer receipt/epoch와 grace deadline |
| `CGameRoom` | `SERVER_PLAYER`, PlayerId/NetEntityId, gameplay state와 stable `SESSION_ID` mapping |
| `CServerApp` | TCP listener, discovery responder, transport map, room routing과 lifecycle lane |
| `CNetworkManager` | Client의 유일한 connection/session state owner, endpoint intent, raw ticket, recovery deadline와 terminal reason |
| `CClientTransport` | Client socket connect/send/recv와 transport generation. world/class/gameplay 정책은 모름 |
| `CServerEndpointResolver` | explicit DIRECT, trusted discovery와 routed 후보 생성. socket session과 Level을 소유하지 않음 |
| `CNetworkRecoveryPolicy` | injected clock/RNG 기반 retry/liveness/terminal 분류. I/O와 presentation을 소유하지 않음 |

새 helper를 두 번째 singleton Manager로 만들지 않는다. 모두 `CNetworkManager` 또는 `CServerApp`가 명확히
소유하고 shutdown 순서를 통제한다.

`CServerApp`는 process 시작 때 immutable boot ID와 token HMAC key를 정확히 한 번 만들고 discovery responder와
lease registry에 주입한다. 두 component가 서로 다른 boot identity를 생성하지 않는다. logical capacity는
`PENDING_ADMISSION`, `ACTIVE`, `GRACE`, `RESUME_PENDING`과 Character Select private arena reservation을 합쳐 최대
64개다.
cap 도달 시 fresh entry만 v41 `SERVER_BUSY`로 거부하고 기존 valid resume은 새 lease를 만들지 않으므로
영향받지 않는다.

### 3.4 Client connection state

```text
IDLE
-> DISCOVERING / RESOLVING
-> CONNECTING
-> AWAITING_ENTRY
-> ACTIVE
ACTIVE -> DEGRADED
DEGRADED -> ACTIVE        (8초 전 current-generation valid frame 회복)
DEGRADED -> RECOVERY_WAIT (8초 무응답/transport close)
-> RESUMING
-> RESYNCING
-> ACTIVE

어느 상태에서든 terminal reason
-> TERMINAL_FAILURE
-> presentation reset + Lobby
```

`CLevel_Lobby`는 target world/class/nickname과 pending identity UI만 소유한다.
`CNetworkManager::Begin_WorldEntry()`가 immutable intent를 받아 위 state machine을 시작하고,
`CNetworkManager::Update()`가 non-blocking progress/result를 소비한다. Lobby Render가 discovery를 실행하지 않는다.

transport close 의미를 다음처럼 분리한다.

- `Close_TransportTransient`: socket shutdown/join, old raw frame/send queue 폐기. ticket, logical identity와
  presentation은 유지한다.
- `Leave_SessionIntentional`: `C2S_SESSION_LEAVE` 뒤 lifecycle commit ACK를 bounded 대기한다. ACK 전에는 ticket을
  보존하며, ACK된 leave만 즉시 slot release로 완료한다.
- `End_SessionTerminal(reason, cleanupMode)`: clone/load/resync validation처럼 control channel을 계속 신뢰할 수 있는
  Client-local failure는 먼저 `Leave_SessionIntentional`을 수행하고 ACK 뒤 ticket/identity를 폐기한다. protocol
  corruption, invalid token 또는 unreachable Server처럼 channel을 신뢰할 수 없으면 transport와 raw token을
  secure-zero하되 non-secret boot/session/cleanup fence를 남긴다. 같은 boot에 fresh entry는 authoritative
  revoked/expired/not-found, Server boot 변경 또는 conservative liveness+grace fence가 끝나기 전 열지 않는다.

### 3.5 protocol v41 session resume

Shared TCP protocol은 append-only packet type을 추가하고 `NETWORK_PROTOCOL_VERSION`을 통합 기준 40에서 41로 올린다.

- `C2S_SERVER_HELLO`
- `S2C_SERVER_HELLO`
- `S2C_SESSION_CONTROL_REJECTED`
- `C2S_ENTRY_CANCEL`
- `S2C_ENTRY_CANCEL_ACK`
- `S2C_SESSION_TICKET`
- `C2S_SESSION_TICKET_ACK`
- `S2C_SESSION_TICKET_CONFIRMED`
- `C2S_SESSION_RESUME`
- `S2C_SESSION_RESUME_RESULT`
- `C2S_SESSION_RESUME_COMMIT`
- `S2C_SESSION_RESUME_COMMITTED`
- `C2S_SESSION_LEAVE`
- `S2C_SESSION_LEAVE_ACK`
- `C2S_HEARTBEAT`
- `S2C_HEARTBEAT_ACK`
- `S2C_FULL_RESYNC_BEGIN`
- `S2C_FULL_RESYNC_END`

v41 Client의 새 TCP transport는 enter/resume token보다 먼저 nonce를 가진 `C2S_SERVER_HELLO`를 보낸다.
Server는 protocol version, stable team ID, echoed nonce와 process boot ID만 답한다. initial entry는 expected team
ID를, resume은 ticket의 exact boot ID까지 일치시킨 뒤에만 nickname 또는 bearer token을 보낸다. 다른/모호한
boot ID에는 token을 노출하지 않고 `SERVER_RESTARTED`/`AMBIGUOUS_SERVER` terminal로 끝낸다.

`S2C_SESSION_CONTROL_REJECTED`는 bounded offending packet type, reason(`RATE_LIMITED`, `INVALID_PHASE`,
`MALFORMED_CONTROL`)과 retryability를 담는다. 이 세 reason은 terminal이며 Server는 frame을 bounded flush한 뒤
exact transport를 close한다. room thread가 structural observation의 generation을 다시 확인해 exact current bound
ACTIVE generation 또는 exact pending-reservation owner generation일 때만 해당 lease/reservation을 exactly once
revoke한다. stale old generation과 아직 bind되지 않은 `ALREADY_ATTACHED` recovery transport는 자기 socket만
close하고 current lease/player를 바꾸지 않는다. current owner의 reject frame이 유실돼 Client가 EOF를 transient로
오인해 한 번 resume해도 Server의 `REVOKED` result가 terminal loop를 닫는다. unknown reason/retryability는 protocol
decode failure다.

v41 Client는 한 번의 사용자 entry intent를 시작할 때 CSPRNG 128-bit `entryAttemptId`를 만들고 reconnect 동안
같은 값을 `C2S_ENTER_WORLD`에 반복한다. terminal/cancel 뒤 새 사용자 intent만 새 ID를 만든다. Server는 첫
요청의 world/class/nickname/revision과 room/spawn capacity를 validate한 뒤 `entryAttemptId`로
`PENDING_ADMISSION`을 stage한다. 이 단계는 다음 ticket과 slot/spawn reservation만 소유하며 player/entity,
binding map, private room과 다른 Client broadcast를 아직 commit하지 않는다.

물리 reservation의 단일 owner는 room-thread-owned `CGameRoom::m_PendingAdmissionReservations`이며
`entryAttemptId -> exact spawn slot, requested world/class`를 보관한다. PlayerId/NetEntityId는 ticket 대기 중
미리 잡지 않는다. ACK admission preflight가 room thread의 current monotonic allocators에서 ID를 claim하면서
counters를 즉시 advance하고, 뒤 encode/map failure로 rollback해도 burned ID는 재사용하지 않는다.
`Find_AvailablePlayerSpawn`과 room 4인 occupancy는 이 reservation과 committed ACTIVE/GRACE/RESUME_PENDING player를
합쳐 계산한다. 같은 attempt 재전송은 기존 claim 하나를 재사용하고 ACK commit은 claim을 release했다가 다시
찾지 않고 staged player로 원자적으로 consume한다. cancel/timeout은 claim을 exactly once release한다. 따라서
서로 다른 pending 네 개는 서로 다른 네 spawn을 점유하고 fifth fresh attempt는 `ROOM_FULL`이다.

- Server boot ID
- stable logical `SESSION_ID`
- 256-bit opaque active token
- token epoch
- grace duration 15,000ms

Server는 pending admission에 exact ticket을 고정하고 작은 reserved control frame으로
`S2C_SESSION_TICKET`만 먼저 보낸다. 같은 `entryAttemptId`와 byte-identical entry payload가 5초 window 안에
다른 transport generation으로 다시 오면 새 player/slot을 만들지 않고 pending transport를 rebind해 같은
ticket을 재전송한다. 같은 ID의 payload가 다르면 `ENTRY_ATTEMPT_CONFLICT`, 만료 뒤 bounded tombstone에 걸리면
`ENTRY_ATTEMPT_EXPIRED`다. ticket frame이 0 byte 또는 일부만 도달한 Client는 Server timeout을 추측해 기다리거나
fresh ID를 만들지 않고 initial-entry budget 안에서 같은 ENTER attempt를 다시 보낸다.

Client NetworkManager는 ticket을 검증·저장하고 idempotent `C2S_SESSION_TICKET_ACK`를 보낸다. current-generation
ACK 또는 새 transport의 exact pending token을 가진 `C2S_SESSION_RESUME`만 ticket 수신 증명이다. room thread는
그 증명을 받았을 때 admission batch 전체를 preflight하고, pending reservation을 player/entity/lease/binding으로
exactly once commit한 뒤 `S2C_SESSION_TICKET_CONFIRMED -> S2C_ENTER_ACCEPTED ->
S2C_FULL_RESYNC_BEGIN -> initial full-state closure -> S2C_FULL_RESYNC_END`를 publish한다. pending-token resume은
active-world token rotation을 하지 않고 이 같은 confirmation transition으로 정규화하며, 앞에
`S2C_SESSION_RESUME_RESULT(INITIAL_ADMISSION)`를 넣어 Client가 새 transport generation을 확정하게 한다.
duplicate ACK/resume은 상태를 다시 commit하지 않고 exact confirmation/result를 idempotently 재전송한다.

ticket confirmation과 initial full-sync END validation이 모두 끝나기 전에는 enter accepted를 Lobby/Level에
공개하지 않는다. Lobby에는 아직 replication Layer가 없으므로 presentation commit은 하지 않는다. accepted 뒤
Loading이 Level의 `CClientReplication`을 초기화하면 one-shot staged transaction을 넘겨 activation 전에
presentation을 commit한다. active-Level resume도 authoritative world가 current descriptor와 같을 때만 현재
Level에서 commit하고, 다르면 target Loading one-shot handoff를 사용한다. 5초
pending-admission expiry는 reservation과 token digest를 exactly once 폐기하고 player/despawn을 만들지 않는다.
expired-attempt tombstone은 15초, 최대 64개로 제한해 늦은 동일 ID를 typed reject한 뒤 제거한다.

Lobby cancel은 socket close를 pending cancel로 추측하지 않는다. 살아 있는 current transport에는
`C2S_ENTRY_CANCEL(entryAttemptId)`를 보내고, 이미 끊겼지만 ticket이 있으면 hello 뒤 credentialed cancel을 보낸다.
room thread가 pending reservation을 release하고 tombstone을 남긴 뒤 `S2C_ENTRY_CANCEL_ACK`를 보낸다. ticket ACK와
cancel race는 lane FIFO에서 하나만 이긴다. cancel이 먼저면 뒤 ACK는 `ENTRY_ATTEMPT_CANCELLED`, admission commit이
먼저면 attempt ID만으로 active player를 지우지 않고 `ENTRY_ATTEMPT_COMMITTED`를 답한다. current authenticated
session transport의 `C2S_SESSION_LEAVE` 또는 exact session ID/token/epoch를 포함해 constant-time 검증된 cancel만
explicit leave로 승격해 player/binding cleanup 뒤 cancel ACK한다.
duplicate cancel은 같은 결과를 idempotently 답한다. Client는 original 5초 pending deadline이 끝났다는 local
clock 추측만으로 새 `entryAttemptId`를 만들지 않는다. ticket ACK bytes가 Server에 commit됐을 수
있으므로 Client는 logical session/token을 포함한 cancel credential과 attempt ID를 secure하게 보존하고
reconnect 때 cancel을 entry/resume보다 먼저 재시도한다. 같은 boot ID의 cancel ACK, authoritative
`ENTRY_ATTEMPT_EXPIRED/REVOKED/NOT_FOUND`, 또는 새 Server boot ID로 old lease 소멸이 증명된 뒤에만 자동/사용자
재입장을 새 ID로 연다. ACK 뒤 commit race로 lease가 ACTIVE였다면 Server cancel 처리가 explicit leave까지
끝내므로 자기 reservation/slot을 중복 점유하지 않는다.

ticket을 한 byte도 받지 못한 pending Client가 transport까지 잃고 cancel하려면 attempt ID 단독 cancel을 새
transport에서 보내지 않는다. 같은 ENTER payload/attempt를 재전송해 pending transport를 rebind하고 ticket을
받은 뒤 cancel하거나, Server의 typed expired/not-found를 받아 cleanup을 확인한다. pending cancel은 exact currently
bound generation 또는 valid pending token을 요구하고, sniffed/guessed attempt ID alone은 reservation/ACTIVE state를
변경하지 않는다.

raw token은 Client process memory에만 두고 log/status/document에 출력하지 않는다. Server의 steady
`ACTIVE/GRACE` registry도 `HMAC-SHA-256(bootSecret, token)` digest만 소유한다. 다만 byte-identical retransmit이
필요한 `PENDING_ADMISSION`의 ticket raw token과 `RESUME_PENDING`의 successor raw token은 별도 fixed sensitive
buffer arena에 해당 phase 동안만 보존한다. arena는 시작 때 `VirtualLock`한 bounded 64+64 slot이며 lock/allocation
실패 시 새 admission/resume을 typed internal-security failure로 거부한다. registry raw slot은 exact ticket/result
재전송에만 읽으며 phase commit/cancel/expiry/revoke에서 `SecureZeroMemory` 후 반환한다. transition 뒤 steady
registry에는 digest만 남는다.

wire encode/send/parse에는 raw token의 transient copy가 불가피하므로 일반 `std::vector` lifetime에 맡기지 않는다.
Shared `CSensitiveByteBuffer`와 token-bearing direct frame builder가 payload+frame을 한 zeroizing owner에 만들고,
Server/Client outbound queue는 move-only sensitive frame variant를 partial send 완료/error/close에서 즉시 wipe한다.
stream parser는 token-bearing consumed range를 compaction 전에 wipe하고 receive scratch, decoded DTO/event와
NetworkManager raw ticket/pair도 RAII destruction에서 wipe한다. 일반 packet은 기존 vector path를 유지한다. digest
비교는 token 길이와 무관한 constant-time 비교를 사용한다. Client도 COMMITTED 뒤 predecessor, terminal/leave에서
모든 raw token buffer를 secure-zero한다. Server restart는 boot secret, pending raw slot, queued/parser transient와
모든 ticket을 함께 폐기한다.

resume은 다음 two-phase rotation을 사용한다.

```text
Client -> active token + boot ID + token epoch + resume attempt nonce
Server -> old logical lease에 새 transport를 원자적으로 reserve/rebind
       -> pending successor token + resume epoch를 포함한 RESUME_RESULT(ACCEPTED)
Server -> FULL_RESYNC_BEGIN
       -> 기존 full-state packet sequence
       -> FULL_RESYNC_END
Client -> 전체 stage 검증 후 local atomic commit, input은 계속 freeze
       -> SESSION_RESUME_COMMIT(resume epoch)
Server -> pending successor를 active로 승격
       -> SESSION_RESUME_COMMITTED
Client -> successor를 active로 확정, predecessor 폐기, input gate open
```

Client는 successor를 검증한 순간부터 `(predecessor, successor, resumeEpoch)` pair를 보존한다. commit 전
transport가 다시 끊기면 Server는 해당 epoch의 predecessor 또는 pending successor를 idempotently 인정한다.
Server가 commit한 뒤 COMMITTED frame만 Client에 도달하지 않은 경우 Client는 다음 reconnect에서 successor를
먼저 제출하고, Server는 이미 active인 같은 successor/epoch를 idempotently 인정해 COMMITTED를 재전송한다.
predecessor 거부만으로 terminal 처리하지 않고 보유한 successor attempt를 먼저 완료한다. 이미 다른
transport에 `ACTIVE`인 lease의 unrelated duplicate resume은 기존 연결을 탈취하지 않고 typed reject한다.

`S2C_SESSION_RESUME_RESULT` rejection reason은 최소 `INVALID_TOKEN`, `EXPIRED`, `REVOKED`,
`SERVER_RESTARTED`, `ALREADY_ATTACHED`, `WORLD_UNAVAILABLE`, `PROTOCOL_MISMATCH`를 구분한다. unknown enum과
잘못된 token 길이는 decode 실패이며 destination을 변경하지 않는다.

`ALREADY_ATTACHED`는 valid same-session token이지만 old generation이 Server 기준 아직 8초 liveness window 안인
경우 기존 connection을 탈취하지 않는 응답이다. unrelated duplicate는 terminal이지만, 같은 Client recovery
attempt에는 remaining old-transport liveness에서 계산한 bounded `retryAfterMs`를 주고 Client local recovery budget
안에서만 retry한다. Server가 8초에 old generation을 detach한 뒤 다음 attempt가 정상 resume한다.

accepted resume result는 resume context(`INITIAL_ADMISSION`, `INITIAL_ADMISSION_COMMITTED` 또는
`ACTIVE_WORLD`), boot/logical session ID, world, PlayerId/NetEntityId, active/pinned gameplay revisions,
token/epoch, optional committed world-transfer source/target/epoch receipt와 authoritative `remainingGraceMs`를
함께 보낸다. `INITIAL_ADMISSION`은 pending token possession을
증명한 뒤 같은 admission commit/batch를 계속하는 경우이고 successor rotation이 없다.

ticket ACK로 Server commit은 끝났지만 confirmation/accepted/full batch가 전부 유실된 Client는 local
`AWAITING_ENTRY`와 ticket의 logical session/`entryAttemptId`를 보존한다. reconnect resume request에
`expectedContext=INITIAL_ADMISSION`과 exact attempt ID를 넣는다. Server lease가 이미 ACTIVE/GRACE이고 저장된
attempt/session이 일치하면 active resume successor rotation을 수행하되 result context를
`INITIAL_ADMISSION_COMMITTED`로 보낸다. Client는 아직 없는 old PlayerId를 비교하지 않고 boot/session/attempt,
요청 world/class/nickname과 full closure를 검증해 authoritative PlayerId/NetEntityId를 transactionally 채택한다.
Lobby/Loading one-shot resync commit과 `SESSION_RESUME_COMMITTED`까지 끝난 뒤에만 Level activation/input을 연다.
`ACTIVE_WORLD`는 이미 presentation identity가 있는 Client만 사용하며 기본적으로 기존 identity와 exact match를
요구한다. 유일한 예외는 authenticated same boot/session/token의 committed transfer receipt가 Client current
world/PlayerId/NetEntityId와 receipt source에 exact match하고, source->target이 registry의 허용 edge이며,
transferEpoch가 마지막 채택 epoch보다 큰 unseen 값이고, resume result/full closure의 target world/new IDs와 exact
match하는 경우다. pre-existing Client pending marker는 추가 상관관계로 쓸 수 있지만 Server-authoritative trigger에는
필수가 아니다. 이 조건에서만 target room의 새 authoritative IDs를 Loading transaction에서 정확히 한 번 채택한다.

intentional leave는 Server room thread가 expiry와 같은 cleanup을 commit한 뒤
`S2C_SESSION_LEAVE_ACK`를 enqueue하고 `Request_Close_After_Flush`한다. Client는 ACK 뒤에만 local ticket을
폐기한다. ACK timeout 또는 이미 끊긴 socket에서는 Client UI/session을 terminal로 끝내되 Server slot 상태는
unknown이다. leave cleanup이 commit되고 ACK만 유실됐다면 이미 release됐고, leave 자체가 미도달이면 남은 grace가
정리한다. Client는 cleanup fence를 보수적으로 유지하며 네트워크가 끊긴 모든 leave에 즉시 release를 약속하지
않는다.

### 3.6 detach, grace와 full resync

current transport generation의 EOF/reset/liveness timeout만 stable lease를 `GRACE`로 바꾼다. room thread가
DETACH를 commit한 `detachedAt + 15,000ms`를 immutable grace deadline으로 한 번 만든다. 따라서 8초 half-open
detection 뒤에도 full 15초 resume window가 있고 old generation의 늦은 close callback은 무시한다.

detach는 다음 상태를 지우지 않는다.

- player/entity ID, class, nickname과 spawn placement
- position, yaw, HP/resource, death/action, cooldown/combo/stance
- inventory와 item count
- world entity, encounter, destruction, prop와 combat object 상태
- Character Select private simulation과 audition owner의 logical session identity
- Server command sequence replay fence
- `isCombatReady`, knockback/fall/attachment/TriggerMove와 이미 생성된 projectile의 Server 진행 상태

detach에서는 사용자 move goal/path, 미실행 pending command, buffered combo와 held input ownership만 해제한다.
HOLD는 기존 release 의미로 종료하며 `isCombatReady=false`나 모든 player simulation freeze로 무적을 만들지 않는다.
이미 시작한
Server-authoritative action, damage와 encounter simulation은 계속 진행되어 disconnect가 무적/rollback 수단이
되지 않는다.

resume은 `Join`을 다시 호출하지 않는다. 기존 player의 weak transport만 새 generation으로 rebind하고 initial
join에서 사용하는 full authoritative sync를 공용 함수로 추출해 보낸다. full closure는 다음 current state를
정확히 한 번 포함한다.

- 모든 player spawn과 current player snapshot
- 모든 world entity spawn
- 모든 combat object spawn
- world destruction full sync
- encounter prop full sync
- 자기 inventory snapshot
- active/pinned gameplay revisions

`Send_Accepted`, `Send_Spawned`, `Build_WorldEntitySpawnedPayload`, `Broadcast_WorldSnapshot`,
`Send_InventorySnapshot`, `Send_WorldDestructionFullSync`, `Send_EncounterPropSync`와 combat-object live builder를
공용화한다. roster/class/nickname뿐 아니라 boss/Esther gauge, attachment, destruction/prop epoch와 event
watermark까지 포함한다. PR #247/Next Pattern이 추가한 지속 상태가 있다면 그 실제 소비 필드를 이 closure에
추가하고, 일회성 연출 event를 재발사하는 방식으로 복구하지 않는다.

resync용 `S2C_WORLD_SNAPSHOT`은 authoritative roster/state/cooldown/combat-object 상태만 담고 transient
`DamageEvents`와 `BossCombatEvents`는 비운다. disconnect 직전 이미 본 hit/effect를 다시 재생하지 않으며,
`FULL_RESYNC_END` 뒤 ordinary snapshot부터 새 transient event를 소비한다.

room thread는 모든 frame을 먼저 encode/validate하고 capacity 512 frames, 8MiB인 dedicated
`Queue_ReliableBatch`에 `FULL_RESYNC_BEGIN -> full-state closure -> FULL_RESYNC_END`를 한 batch object로
preflight·enqueue한다. normal 128-frame/512KiB queue에 BEGIN부터 frame별로 넣지 않는다. frame/byte bound,
개별 frame의 기존 64KiB 한도, 하나뿐인 active resync batch와 exact generation을 한 lock에서 admission하고,
하나라도 초과하면 BEGIN을 전혀
보내지 않은 채 typed resume failure로 끝낸다. sender는 batch를 순서대로 drain하며 ordinary snapshot/delta는
END 뒤로 hold/coalesce한다. batch 안의 snapshot은 일반 snapshot coalescing 대상이 아니다. Server는
`SESSION_RESUME_COMMIT` 전까지 새 transport의 gameplay command를 받지 않는다.

initial admission의 ticket frame은 player commit 전 별도 control response다. ACK 또는 exact-token proof 뒤에는
confirmed/accepted/full closure 전체를 staged join receipt로 encode하고 `Reserve_ReliableBatch`로 sender에게 아직
보이지 않는 exact transport-generation reservation을 얻는다. pending slot/spawn reservation과 detached player,
lease/binding map mutation을 모두 prevalidate한 뒤 room commit을 한 번 수행하고,
`Publish_ReservedBatch`만 no-fail로 sender에 노출한다. 중간 실패는 staged join과 batch를 rollback하되 pending
attempt는 원래 deadline까지 같은 ID로 재증명할 수 있다. 따라서 ticket 전송 유실에는 player commit 0개이고,
confirmation batch의 capacity/encode/map 실패에도 accepted frame 0개와 room/lease/binding partial commit 0개다.
active-world resume도 batch reservation을 확보한 뒤 rebind하고 publish한다. reserved batch publish invariant가
깨지면 partial admission을 계속하지 않고 Server structural fail-fast로 끝낸다.

Client `ClientReplicationEvent` envelope는 모든 event에 transport generation과 optional resync generation을
붙이고 BEGIN/END barrier를 보존한다. NetworkManager/Replication coalescing은 generation이나 barrier를 넘어
event를 이동·교체하지 않는다. `CClientFullResyncTransaction`은 BEGIN의 expected count/world/revision을 기준으로
wire DTO를 stage하고 END에서 count, stable ID, 자신의 exact PlayerId/NetEntityId와 gameplay revision을 모두
검증한다.

wire validation 뒤에는 실제 presentation도 old live Layer를 건드리지 않고 stage한다. player/world entity
prototype을 detached clone으로 만들고 replication-owned `Layer_Player`, `Layer_WorldEntity`, registry, local
character handle, HUD/encounter/destruction candidate를 완성한다. `CClientReplication::Initialize`가 두 exact
layer를 미리 ensure하고, Engine `Object_Manager/GameInstance`의 batch replacement API가 expected old layer
identity 두 개를 preflight한 뒤 shared pointer만 no-fail swap한다. world destruction은 기존 Stage 계약으로
먼저 검증하고 sink `Set_States`가 자체 rollback한 뒤, layer/registry/HUD pointer swap처럼 실패하지 않는
commit만 수행한다. commit 전 clone/validation이 실패하면 old presentation은 그대로이고, stale expected
layer identity면 아무 swap도 하지 않는다.

15초 expiry, authenticated explicit leave/cancel 또는 terminal lease revoke에서만 `CGameRoom::Leave`를 정확히 한
번 호출하고 despawn, binding erase, spawn release와 empty private-room retirement를 수행한다.

### 3.7 heartbeat, 지연과 retry

TCP 지연만으로 session을 버리지 않도록 다음 값을 정본 상수와 injected test clock으로 고정한다.

- decode/semantic validation을 통과한 current-generation application frame만 liveness를 갱신한다.
  현재의 payload validation 전 `Record_InboundPacket` 진단 시각을 그대로 heartbeat 근거로 사용하지 않는다.
- 1초 동안 보낼 application frame이 없을 때만 Client가 heartbeat를 보낸다.
- Server는 heartbeat 또는 gameplay command를 5초 받지 못하면 transport를 `DEGRADED`로 기록한다.
- Client도 Server frame을 5초 받지 못하면 UI 상태만 `DEGRADED`로 바꾸고 presentation을 유지한다.
- 양쪽 모두 8초 전에 current-generation valid frame을 받으면 같은 socket에서 즉시 ACTIVE로 돌아가며 connect,
  detach나 token rotation을 시작하지 않는다.
- Client는 Server frame 8초 무응답이면 해당 TCP transport를 transient close하고 resume을 시작한다.
- Server도 Client application frame 8초 무응답이면 current socket을 shutdown/close하고 lifecycle lane에 exact
  generation DETACH를 넣는다. Client FIN/RST가 유실된 단방향 half-open도 이 경계에서 ACTIVE를 GRACE로 바꾼다.
- Client/Server socket은 nonblocking send와 readiness wait를 사용한다. **`WSAEWOULDBLOCK` 또는 readiness poll
  timeout만** 같은 offset에서 재시도하며 양수 byte count에서만 offset을 전진시킨다. poll/cancel 주기는 최대
  250ms, 전송 진전이 전혀 없는 window는 최대 3초다. socket 전체가 nonblocking이므로 receive worker도
  would-block/readiness/cancellation을 처리한다.
- 실제 `WSAETIMEDOUT`, reset, aborted 또는 blocking `SO_SNDTIMEO` 오류가 발생한 stream은 닫고 detach/resume한다.
  그 socket에서 frame을 재전송하거나 부분 전송 여부를 추측하지 않는다. 성공 send도 상대의 application ACK를
  뜻하지 않으므로 ticket/commit ACK 계약은 그대로 필요하다.
- reconnect backoff는 `0ms, 250ms, 500ms, 1000ms, 2000ms` 후 2000ms cap이며 각 시도에 최대 100ms
  injected jitter를 더한다.
- ticket은 15초 duration만 전달하며 Client가 Server의 absolute clock을 추측하지 않는다. Server deadline은
  room-thread `detachedAt + 15,000ms`이고 Client는 자기 8초 loss 감지/transport close부터 최대 15초 local budget을
  사용하되
  `S2C_SESSION_RESUME_RESULT`의 authoritative `remainingGraceMs`가 더 짧으면 즉시 줄인다. Server reject/expiry가
  최종 정답이고 retry는 grace를 연장하지 않는다.

Windows는 blocking send timeout 뒤 연결 상태가 불확정이므로 close하도록 명시한다. 위 정책은 이 제한에
따라 readiness 대기와 실제 transport 실패를 구분한 설계다.
[Microsoft SOL_SOCKET](https://learn.microsoft.com/en-us/windows/win32/winsock/sol-socket-socket-options),
[Microsoft send](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send).

heartbeat 송신, 수신 검증과 last-valid clock은 render/Loading loop와 독립된 transport worker에서 진행한다.
UI는 immutable 상태 snapshot만 읽는다. 단순히 RTT가 높다는 이유로 kick하지 않고 CSO/진행률 epoch와 network
transport/recovery epoch는 별도로 관리한다. 실제 decode/queue overflow는 transient로 숨기지 않는다.

recovery endpoint 선택은 다음 순서를 사용한다.

1. DIRECT는 같은 host/FQDN을 매 attempt cancellable resolve하며 다른 후보로 전환하지 않는다.
2. AUTO는 last-success endpoint를 한 번 fast path로 시도한다.
3. cached address/route/connect/hello가 실패하거나 wrong boot이면 token을 0 byte 유지한 채 fresh LAN discovery를
   실행하고 ticket의 stable team ID와 exact boot ID가 같은 response만 후보로 쓴다.
4. PR B version 3의 trusted routed FQDN은 매 attempt A record를 다시 resolve하고
   `trustedServerEndpointCidrs` 안의 주소만 후보로 쓴 뒤 TCP Server hello의 exact boot ID를 확인한다.
5. wrong boot candidate 하나만 보고 terminal로 끝내지 않고 remaining trusted LAN/routed A record를 recovery budget
   끝까지 검사한다. exact ticket boot를 찾으면 그 후보만 사용한다. 끝까지 exact boot가 없고 관측한 team Server가
   한 new boot면 `SERVER_RESTARTED`, 서로 다른 boot가 둘 이상이면 `AMBIGUOUS_SERVER`, 유효 hello 자체가 없으면
   transport/resolve timeout이다. 모든 wrong boot에는 token을 보내지 않는다. `rollbackHost`는 recovery에도 쓰지
   않는다.

focused fake-resolver harness는 cached-address failure 뒤 같은 boot ID의 새 DHCP source로 resume되는 경우와,
cached A의 wrong boot 뒤 discovery B의 exact old boot, multi-A wrong-first/correct-second, exact boot가 끝내 없을 때만
SERVER_RESTARTED/AMBIGUOUS가 되는 경우와 모든 wrong boot에 bearer token bytes가 0인 경우를 검증한다.
half-open fixture는 Server->Client 방향만 끊어 Client의 첫 resume이 retryable `ALREADY_ATTACHED`를 받고, Server
8초 detach 뒤 같은 token/identity로 성공하며 old generation이 state를 바꾸지 않는 경우를 검증한다.
fake-clock liveness는 5,000ms에 DEGRADED, 7,999ms에 current-generation valid frame을 받으면 token rotation/connect
0건으로 ACTIVE 복귀하고, stale-generation frame은 이 recovery에 쓰이지 않는지 검사한다.

Client outbound는 send worker의 bounded queue를 사용한다. move/aim은 latest value로 coalesce하고 reliable
command에는 별도 reserve를 둔다. transport loss 때 모든 미전송 gameplay command를 폐기하며 skill/move를
새 socket에 replay하지 않는다. receive parser는 worker-local로 두고 `shutdown -> join -> parser destroy`
순서를 지킨다. inbound result에는 transport generation을 붙여 stale frame을 main thread에서 폐기한다.

### 3.8 non-droppable Server lifecycle lane

transport receive/close callback은 room/binding/lease를 직접 수정하거나 일반 room queue에 session-control packet과
`LEAVE`를 넣지 않는다.
`CServerApp`는 global transport cap 64에서 도출한 capacity 256의 preallocated FIFO lifecycle lane을 structural
close reserve 64와 control capacity 192로 나눈다. 각 transport의 close는 producer one-shot이라 reserved slot을
둘 이상 소비하지 않는다. 다음 bounded immutable control/lifecycle observation과 monotonic sequence를 기록한다.

global transport permit은 socket/callback close 시 반환하지 않는다. accepted, active와 `CLOSING` record를 모두 cap
64에 세고 close observation이 room thread에서 consume되어 lifecycle effect를 commit한 뒤 ServerApp reaper가
session workers join/map erase를 끝낼 때만 permit을 반환한다. accept loop는 `accepted + closing < 64`일 때만 새
socket을 admit한다. close event가 transport record/permit reference를 reap receipt까지 소유하므로 undrained close
64개가 있는데 65번째 transport를 받아 structural reserve를 넘기는 churn이 불가능하다.

- validated hello와 `ENTER_ATTEMPT`/`RESUME_ATTEMPT`
- ticket ACK 또는 pending-token admission proof
- pending entry cancel
- active resume rebind와 resume commit
- explicit leave
- transport closed/detach

각 observation은 exact transport ID/generation, 알려진 경우 logical session, `entryAttemptId`, token/resume epoch와
idempotency nonce를 stamp한다. receive/send callback은 decode된 immutable DTO와 `TRANSPORT_CLOSED`만 이 lane에
기록하며 lease/binding을 직접 수정하지 않는다. `ROOM_LIFECYCLE_EVENT`는 일반 `ROOM_COMMAND_TYPE`이 아니고
hard-cap gameplay `Enqueue`를 절대 통과하지 않는다. hello/admission stage, ticket ACK,
`SESSION_RESUME_COMMIT`, detach/rebind와 binding commit을 모두 room thread가 observation 순서대로 수행한다.

외부 control은 lane 앞에서 transport별 fixed token bucket 20/s, burst 20과 state/type/attempt-or-epoch별 outstanding
one-shot slot을 통과한다. processing 전 byte-identical duplicate는 기존 outstanding observation으로 coalesce하고
room transition 뒤 duplicate는 cached idempotent response만 재전송한다. state-invalid control이나 rate/control
capacity 초과는 새 control observation을 넣지 않고 해당 transport에 terminal
`S2C_SESSION_CONTROL_REJECTED`를 bounded flush-close한다. 그 structural close observation은 terminal reason을
stamp해 structural reserve에 들어간다. room thread는 generation fence 뒤 current bound/pending owner만 revoke하고
stale/unbound offender는 socket만 닫으므로 외부 flood가 다른/current session을 탈취하거나 Server fatal shutdown,
transient resume loop를 유발할 수 없다.
token-bearing response의 cache는 ordinary frame bytes가 아니라 phase metadata와 locked sensitive slot reference이며,
매 재전송은 새 zeroizing sensitive frame을 만들고 send 종료에서 wipe한다.

room thread는 gameplay command queue보다 먼저 lifecycle lane을 FIFO로 drain하고 generation/lease state를
다시 검증한 뒤 처리한다. exact transport ID/generation의 duplicate close만 producer의 one-shot flag로
coalesce하고, detach/rebind의 서로 다른 transition은 latest-value로 덮어쓰지 않는다. 같은 logical
session의 stale event는 room thread가 state/epoch 비교로 폐기한다. duplicate ticket ACK/resume commit은
state를 두 번 바꾸지 않고 exact CONFIRMED/COMMITTED response만 재전송한다.

genuine internal structural close producer만 reserved lane invariant가 깨졌을 때 binding을 먼저 변경하거나 event를
버리지 않고 최대 100ms condition wait를 한다. one-shot 64 transport cap인데도 그 안에 admission되지 않으면 새
accept를 중단하고 dedicated lifecycle-overflow fatal 상태로 cooperative shutdown한다. Client control saturation은
이 fatal path를 사용하지 않는다. 일반 gameplay queue의 reliable 한도 960이 차도 detach cleanup은 유실되지 않으며, silent orphan
player보다 명시적 process failure를 선택한다.

grace expiry는 external lane event/producer가 아니다. room thread lifecycle phase가 FIFO control event를 처리한
전과 뒤 injected steady clock으로 `ExpireDueLeases(now)`를 직접 실행한다. exact `now >= deadline`이면 resume
validation보다 direct expiry가 먼저이며 `EXPIRED`를 답한다. 14,999ms에 실제 room-thread validation이 끝난
request만 RESUME_PENDING으로 갈 수 있고 original deadline은 그대로다. 따라서 lane full에서 room consumer가 자기
expiry enqueue를 기다리지 않으며, shutdown producer-count는 accept/session worker만 센다.

모든 Client-originated `ROOM_COMMAND`에는 logical session, `TRANSPORT_ID`와 generation을 stamp한다. room thread는
lifecycle lane을 먼저 drain한 뒤 gameplay command dispatch 직전에 current attached generation과 exact match를
다시 확인한다. close 직전 old socket이 enqueue한 move/skill이 detach 또는 새 transport rebind 뒤 실행되는
것을 금지한다.

pending unauthenticated transport는 global 64, source IPv4별 8로 제한하고 enter/resume 첫 packet deadline은
5초다. accept가 peer IPv4/port를 반환해 이 cap과 구조화된 log에 사용하되 token을 source IP에 묶지는 않는다.
Wi-Fi/VPN route가 바뀌어 source IP가 달라져도 valid token은 resume할 수 있어야 한다.

Server shutdown은 accept/discovery stop -> room을 `DRAINING` 상태로 두고 lane 소비 계속 -> 모든 session에
cooperative stop 요청 -> close callback을 lane에 기록하면서 session send/recv worker 전부 join -> 등록된
lane producer-count 0 barrier -> lifecycle lane empty barrier -> room thread에서 남은 lease revoke/cleanup -> room
stop/join -> room/discovery/Winsock destroy 순서다. cleanup 전에 worker를 join해 late close가 drain 뒤 들어오는
구멍을 막고, room consumer는 producer-count 0과 empty-lane을 둘 다 보기 전 종료하지 않는다. accept-running,
room-draining과 producer-count를 분리한다. grace expiry는 별도 worker가 아니라 room tick의 injected steady
clock이 소유한다.

### 3.9 data revision transaction과 Loading의 교차 경계

현재 `ServerApp.cpp:3617`, `:3984` 부근의 Debug data-revision 2PC는 `m_Sessions`, binding과 transport pointer의
정확한 participant 집합을 검사한다. logical lease가 남는데 transport가 없다는 이유로 그 participant를
건너뛰면 GRACE player의 동의 없이 revision이 바뀔 수 있다.

- transaction의 전체 participant 범위(현재 process의 bound room들)에 `PENDING_ADMISSION`, `GRACE`,
  `RESUME_PENDING` lease가 있으면 새 2PC prepare를
  `SESSION_RECOVERY_IN_PROGRESS`로 거부한다. prepare 중 detach가 확정되면 기존 transaction을 abort한다.
- participant/result/tombstone은 logical session과 transport generation을 같이 검증한다. old transport의
  ACK가 새 lease 상태를 승인하지 못한다.
- resync BEGIN에 고정한 active/pinned revision은 END/COMMIT까지 바꾸지 않는다. new revision이나 target world를
  이전 baseline에 끼워 넣지 않는다.
- Loading의 `loadEpoch`는 network transport generation과 다른 identity다. 복구 중에도 검증 가능한 로컬
  resource 작업은 계속할 수 있지만, target world의 current-generation full state와 session commit이 끝나기
  전에는 실제 activation을 보류한다. 표시 퍼센트를 네트워크 복구 성공 근거로 사용하지 않는다.
- CSO 누락/손상, renderer/core 실패, 잘못된 content revision은 원래 load/validation 실패다. 이를 ping 또는
  transient reconnect로 분류하지 않는다. target-level optional Effect 격리와 entry-required Map Effect 실패
  계약도 그대로 구분한다.

## 4. 수정 예정 파일과 역할

아래는 구현 시의 변경 목록이다. 이번 계획 전용 작업에서 이 파일들을 실제 수정하거나 생성하지 않는다.

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/.gitignore` | machine-local `Tools/Network/TeamLanRole.local.json` 추적 금지 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Object_Manager.h` | existing layer ensure와 exact-identity batch replacement contract |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Object_Manager.cpp` | prevalidated multi-layer pointer swap와 rollback receipt |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/GameInstance.h` | Client replication이 소비하는 layer transaction facade |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/GameInstance.cpp` | Object Manager batch transaction forwarding |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketType.h` | protocol v41, ticket/resume/leave/heartbeat/full-resync packet type |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/NetworkIds.h` | wire용 stable `NETWORK_SESSION_ID`와 invalid 값 |
| 추가(추적 생성물) | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/TeamLanEndpoint.generated.h` | JSON publisher가 만드는 Client/Server compile-time network default |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketMessages.h` | bounded session ticket/resume/result/commit/heartbeat/resync message 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Private/Network/PacketMessages.cpp` | 새 TCP message codec, enum/version/token validation과 destination-preserving decode |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/SensitiveByteBuffer.h` | move-only bounded byte owner와 wipe/inspection test seam |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Private/Network/SensitiveByteBuffer.cpp` | `SecureZeroMemory` clear/destructor와 locked-slot copy boundary |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketWriter.h` | token-bearing message의 sensitive-buffer direct writer overload |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Private/Network/PacketWriter.cpp` | intermediate vector 없는 bounded sensitive encode |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketFrame.h` | move-only sensitive final-frame builder 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Private/Network/PacketFrame.cpp` | token payload/frame one-owner build와 failure wipe |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketStreamParser.h` | token-bearing sensitive output와 consumed-range/reset wipe API |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Private/Network/PacketStreamParser.cpp` | payload copy/erase/compact 전 wipe와 parser-capacity reset |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketReader.h` | fixed-size byte span read API |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Private/Network/PacketReader.cpp` | partial/truncated read에서 cursor/destination 보존 |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/LanDiscoveryProtocol.h` | TCP frame과 분리된 fixed UDP discovery schema와 bounded 값 |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Private/Network/LanDiscoveryProtocol.cpp` | query/response encode/decode와 magic/schema/nonce 검증 |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Default/Shared.vcxproj` | 새 Shared C++ source/header 등록과 direct-build endpoint publisher `-Check` |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Default/Shared.vcxproj.filters` | 물리 Network filter에 새 파일 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Public/ServerIds.h` | logical `SESSION_ID`와 분리된 `TRANSPORT_ID`/generation |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Public/TcpListener.h` | accept 결과에 peer IPv4/port 반환 |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Private/TcpListener.cpp` | sockaddr 수집과 normalized peer endpoint |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Public/ClientSession.h` | transport generation, liveness와 ordinary/sensitive bounded outbound variant |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Private/ClientSession.cpp` | heartbeat, partial-send progress, sensitive frame/parser/scratch wipe와 close event |
| 추가 | `C:/Users/user/Desktop/LostArk/Server/Public/LanDiscoveryResponder.h` | UDP responder start/stop/join와 rate-limit 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Server/Private/LanDiscoveryResponder.cpp` | `0.0.0.0:7778`, IP_PKTINFO/NLM ingress profile filter와 source reply 구현 |
| 추가 | `C:/Users/user/Desktop/LostArk/Server/Public/SessionLeaseRegistry.h` | logical lease/token digest/transport generation/grace state 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Server/Private/SessionLeaseRegistry.cpp` | CSPRNG ticket, rotation, duplicate/replay/revoke와 deadline transition |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Public/RoomCommand.h` | Client-originated gameplay command의 logical session/transport generation fence |
| 추가 | `C:/Users/user/Desktop/LostArk/Server/Public/RoomLifecycleEvent.h` | 일반 command queue와 분리된 ordered transport/lease lifecycle edge |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Public/GameRoom.h` | stable logical session detach/rebind/full-sync API와 lifecycle invariant |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp` | state-preserving detach, exact rebind, expiry-only Leave와 full-state 공용화 |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Public/ServerPlayer.h` | weak transport와 logical session/generation 분리 |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Public/ServerApp.h` | discovery, transport map, lease registry와 non-droppable lifecycle lane ownership |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Private/ServerApp.cpp` | accept cap, first-packet routing, detach/resume/expiry, revision 2PC participant fence와 shutdown |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Private/Main.cpp` | discovery port/team ID/trusted scope parse와 contract-test 구성 |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Private/ServerGameplayContractTests.cpp` | lease, duplicate/stale/race, queue saturation, 4인 capacity와 private room 검증 |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Default/Server.vcxproj` | 새 파일, `Bcrypt.lib`/`Iphlpapi.lib`/`Ole32.lib`, NLM ingress profile, Debug/Release와 endpoint `-Check` |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Default/Server.vcxproj.filters` | 새 파일을 물리 Network/Session filter에 등록 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/ClientTransport.h` | async connect/send/recv, generation과 ordinary/sensitive bounded result 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/ClientTransport.cpp` | DNS connect, worker parser, sensitive range/scratch wipe, coalescing과 cooperative close |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/ServerEndpointResolver.h` | DIRECT/AUTO intent, ref-counted async operation/completion과 typed result |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/ServerEndpointResolver.cpp` | discovery, cancellable DNS lifetime, CIDR filter와 shutdown drain/fail-fast |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/NetworkRecoveryPolicy.h` | injected clock/RNG 기반 liveness/backoff/grace/terminal pure policy |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/NetworkRecoveryPolicy.cpp` | exact state transition과 retry deadline 계산 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/NetworkManager.h` | 유일한 session owner, async entry/recovery API와 terminal reason |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/NetworkManager.cpp` | session state machine, close 의미 분리와 모든 mutation Send API의 ACTIVE_COMMITTED fence |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/PlayerCommandSequenceState.h` | logical-session scoped next move/action sequence와 typed fresh/adopt 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/PlayerCommandSequenceState.cpp` | accepted-submit increment/wrap, fresh initialization과 session binding 검증 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/ClientReplication.h` | frozen presentation, staged full resync generation과 atomic commit API |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/ClientReplication.cpp` | transient loss에서 Reset 지연, resync rollback/commit와 terminal-only loss flag |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/ClientReplicationEvent.h` | BEGIN/END barrier, transport/resync generation과 batch ordering envelope |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/ClientFullResyncTransaction.h` | wire DTO, detached presentation layer, registry/HUD/world-state candidate와 commit receipt |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/ClientFullResyncTransaction.cpp` | bounded stage/validate, exact layer identity batch swap와 rollback/commit |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/WorldDestructionProjectionRuntime.h` | same-encounter resume용 authoritative full-replace stage/commit |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/WorldDestructionProjectionRuntime.cpp` | normal forward-sync와 resume replace validation 분리 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_Lobby.cpp` | blocking connect 제거, async 상태/실패 표시와 explicit cancel |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_Lobby.h` | Lobby-owned approval deadline 제거와 async intent/UI state |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_Bern.cpp` | raw disconnect 대신 terminal connection failure만 Lobby 소비 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_ValtanArena.cpp` | recovery 중 presentation 유지/input freeze와 terminal fallback |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_Development.cpp` | recovery state 소비와 terminal fallback |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_CharacterSelect.cpp` | private arena resume/expiry, raw `Is_Connected` 즉시 실패 제거 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_Loading.cpp` | recovery gate, 단일 전체 진행률/fill, 별도 heartbeat, epoch activation 요청 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_Loading.h` | phase-local fill 상태 대신 loadEpoch와 display-only 보간 상태 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/LoadingProgress.h` | production이 소비하는 순수 작업 가중치/epoch/완료량/단조 보간 계약, 별도 singleton 없음 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Loader.h` | 상태 문자열과 분리된 typed resource progress snapshot/epoch |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp` | core/map/model/bundle 성공 직후 계측, 실패·취소 원인 보존 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_LoadPreparationJob.h` | target ACK와 분리한 loading-scoped core/CSO progress snapshot |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_LoadPreparationJob.cpp` | exact epoch 검증과 thread-safe snapshot publish, 기존 mailbox/ACK 의미 유지 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_PresentationService.h` | 실제 loading caller의 core progress 전달 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp` | worker stage/core 관측 전달, main receipt settle과 readiness 분리 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_DocumentRenderer.h` | loading-scoped core step callback, 일반 runtime caller 기본 무관측 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_DocumentRenderer.cpp` | shader/buffer/texture 실제 생성 완료와 cache hit 관측 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/LevelTransitionService.h` | world-transfer handoff와 loadEpoch request/completion snapshot |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/LevelTransitionService.cpp` | world/recovery ordering, main-thread 전체 progress와 최종 activation receipt 소유 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp` | network lifecycle/input gate, 실제 Change_Level/identity commit 뒤 loadEpoch 완료 보고 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/NetworkPlayerCommandSink.cpp` | recovery 중 command reject, resume 뒤 새 edge만 허용 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/PlayerController.h` | recovery RMB release latch와 logical-session command sequence 보존 API |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/PlayerController.cpp` | held RMB 재전송 억제, resume character rebind와 N+1 move/action sequence 유지 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | 새 파일, `Iphlpapi.lib`/`Bcrypt.lib`/`Ole32.lib`, NLM, auto debugger env와 endpoint `-Check` |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | 새 파일을 물리 Network filter에 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Network/TeamLanEndpoint.json` | PR A v2 discovery/role, PR B v3 routed endpoint/source CIDR 분리 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/Network/Publish-TeamLanEndpoint.ps1` | schema v2/v3 validate, generated Shared default publish/check |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Network/Sync-TeamLanEndpoint.ps1` | auto config, firewall expected-set, routed endpoint/source CIDR와 role/probe 출력 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/Network/Test-TeamLanEndpointContract.ps1` | config/env/role와 firewall snapshot의 plan-only expected-set/action regression |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` | v41 TCP codec와 UDP discovery negative/round-trip 검증 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/NetworkConnectionContractHarness/Private/NetworkConnectionContractHarness.cpp` | endpoint policy, async transport generation, liveness/retry/resync transaction harness |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/NetworkConnectionContractHarness/Default/NetworkConnectionContractHarness.vcxproj` | production sources, Shared/Engine refs와 Win32 native link closure를 쓰는 Debug/Release harness build |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/NetworkConnectionContractHarness/Default/NetworkConnectionContractHarness.vcxproj.filters` | harness source/filter 등록 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/Network/Run-NetworkConnectionContractHarness.ps1` | Engine build -> UpdateLib -> Shared/harness -> 구성 일치 DLL 배포 -> fresh 실행/timeout |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/Network/Run-LanDiscoverySmoke.ps1` | 실제 Server responder를 loopback의 동적 UDP/TCP port에서 query하는 process smoke |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/ValtanFourPlayerHarness/Private/ValtanFourPlayerHarness.cpp` | grace 중 4/4, same-ID resume, expiry 뒤 replacement 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/CharacterSelectIsolationHarness/Private/CharacterSelectIsolationHarness.cpp` | private arena resume 보존과 expiry retirement 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Network/Run-ValtanFourPlayerHarness.ps1` | grace/resume fault 단계와 bounded timeout |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Network/Run-CharacterSelectIsolationHarness.ps1` | ticket/resume/expiry scenario와 bounded timeout |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp` | 실제 LoadingProgress helper, ACK/core/epoch/activation 실패와 단조성 실행형 테스트 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj` | 새 production header와 필요한 기존 test seam 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj.filters` | 추가 include/test의 물리 filter 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1` | 새 harness build/run과 Debug/Release network admission gate |
| 수정 | `C:/Users/user/Desktop/LostArk/Framework.sln` | 새 focused harness project/config 등록 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/AGENTS.md` | static endpoint에서 auto discovery/routed endpoint/grace public 경계로 교체 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/CLAUDE.md` | 실제 설정, 실행, 오류 상태와 build/run 사용법 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/.md/TEAM/README.md` | 팀 네트워크 정본 문서 순서와 sync 시작 절차 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/.md/TEAM/네트워크연결가이드.md` | 고정 IP→AUTO 전환, DIRECT/4인/독립 테스트, hotspot 신뢰 설정과 복구 상태 안내 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | LAN auto, routed cross-network, disconnect/resume 운영·실패 진단 |
| 구현 후 추가 | `C:/Users/user/Desktop/LostArk/.md/GB/08-27/2026-08-27_TEAM_NETWORK_AUTO_DISCOVERY_AND_SESSION_RECOVERY_IMPLEMENTATION_RESULT.md` | 실제 diff, 자동 검증, 두 PC/사용자 수동 검증과 미완료 보안 경계 |

새 C++ 파일은 모두 UTF-8 BOM 없이 작성한다. 기존 C++는 현재 인코딩을 유지한다. 새 production 파일은
각 `Shared/Server/Client.vcxproj`와 `.filters`에, 새 harness는 project/filters, `Framework.sln`과
`Invoke-BuildAndRegression.ps1`에 같은 변경 단위로 등록한다. orphan source나 로컬에 남은 stale exe를
검증 근거로 사용하지 않는다.

## G00. baseline과 lifecycle 무결성

### G00-1. 현재 동작을 failure reason으로 고정

`CClientTransport`를 추출하기 전에 PR #246의 기존 WSA/EOF/protocol first-terminal-wins 진단을 baseline으로
고정한다. 새 typed transport result가 기존 reason, local/remote endpoint, capture와 semantic recovery detail을
잃지 않도록 regression을 옮긴다. 정상 사용자의 Back/leave는 transport 고장으로 기록하지 않는다.

현재 `NetworkManager`의 stream parser는 receive worker local로 옮긴다. 기존 `Fail_Protocol`처럼 worker join
전에 shared parser를 reset할 수 있는 경로를 제거한다. 모든 socket close는 cancel flag, shutdown,
closesocket, bounded join, result queue drain 순서 하나를 사용한다.

### G00-2. 기존 priority cleanup 보존과 새 lifecycle 확장

현재 `LEAVE`는 이미 priority cleanup queue를 사용한다. 새 transport close observation은 generation이 붙은
lifecycle lane에 기록하고 room drain에서 기존 exactly-once cleanup으로 연결한다. 아직 resume을 활성화하기
전 compatibility policy는 즉시 expiry/Leave를 호출해 외부 동작을 유지한다. 기존 수정의 재구현으로 범위를
키우거나 원래 gameplay queue에 LEAVE를 되돌리지 않는다.

contract test에서 일반 reliable queue 한도 960 포화, 동시에 여러 close, duplicate close와 stale generation을 만들어도
player가 정확히 한 번 제거되고 binding/player map이 함께 정리됨을 먼저 통과시킨다. 이 gate 전에는 discovery
listener를 제품 기본으로 켜지 않는다.

old transport가 close 직전 enqueue한 move/skill을 남긴 뒤 lifecycle detach를 먼저 drain하는 경우와, 새
transport generation이 rebind된 뒤 old command가 도착하는 경우도 추가한다. 두 command 모두 generation
fence에서 gameplay state 무변경으로 폐기되어야 한다.

## G01. discovery wire와 focused test seam

PR A에서는 `LanDiscoveryProtocol`만 먼저 구현하고 TCP protocol은 통합 기준 v40을 유지한다. discovery datagram의
`tcpProtocolVersion`은 literal 40/41이 아니라 빌드의 `NETWORK_PROTOCOL_VERSION`을 광고한다. 아직 소비자가
없는 ticket/resume packet type과 codec을 미리 추가하지 않는다.

Shared는 discovery wire와 bounded validation만 소유하며 nonce 생성, socket, retry와 Level policy를 소유하지
않는다. `NetworkProtocolHarness`는 다음을 Debug/Release에서 검증한다.

- discovery query/response round-trip
- TCP version mismatch response/server version, wrong magic/schema/team ID/nonce
- truncated/trailing bytes와 fixed 128-bit ID의 byte order
- decode 실패 시 reader cursor와 destination 불변

새 `NetworkConnectionContractHarness`는 production `ServerEndpointResolver`, `NetworkRecoveryPolicy`,
`ClientTransport`와 뒤 G의 full-resync transaction을 직접 compile하고 Shared/Engine을 configuration-matched
ProjectReference로 link한다. 실제 UI나 Client singleton은 만들지 않고 in-process fake UDP/TCP peer와 Engine
layer fixture로 source address, generation, partial send/recv와 atomic layer swap을 검증한다. runner는
`EffectRenderContractHarness`의 배포 방식을 따라 Engine을 먼저 fresh build하고 repo root에서
`UpdateLib.bat <Debug|Release>`를 실행해 변경된 `Engine/Public`을 `EngineSDK/inc`에 publish한 뒤 Shared/harness를
build한다. 그 다음 같은 구성의 `Engine.dll` 및 FMOD/Assimp/PhysX runtime dependency closure를 harness output에
복사해 실행한다. 이전 구성의 stale header/DLL이 남아 있거나 build/publish/deploy 중 하나라도 실패하면 harness를
실행하지 않는다.
Harness vcxproj 자체가 production Client source를 compile하므로 Client.exe link 설정에 기대지 않고 Debug/Release
모두 `Ws2_32.lib;Iphlpapi.lib;Bcrypt.lib;Ole32.lib;OleAut32.lib;Uuid.lib`를 직접 link한다.

## G02. trusted LAN discovery 수직 슬라이스

Server TCP listener open 뒤 discovery responder를 시작하고 Client `AUTO` mode를 연결한다. responder socket은
`0.0.0.0:7778`에 bind하되 `WSARecvMsg`/`IP_PKTINFO`의 ingress interface index를 adapter/NLM profile snapshot과
대조해 Private/Domain query만 reply한다. profile 변경 notification에서 snapshot을 갱신하며 unknown/Public
ingress는 drop한다. firewall scope와 runtime reply policy를 같은 경계로 유지한다.

응답은 ingress interface의 실제 local unicast IPv4로 보낸다. 수신 `IN_PKTINFO.ipi_addr`는 broadcast 목적지일
수 있으므로 송신 source로 그대로 복사하지 않는다. `ipi_ifindex`와 Preferred unicast 주소를 대조해
`WSASendMsg`의 source/interface를 구성한다.
[Microsoft IN_PKTINFO](https://learn.microsoft.com/en-us/windows/win32/api/ws2ipdef/ns-ws2ipdef-in_pktinfo),
[Microsoft adapter 주소/prefix](https://learn.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_unicast_address_lh).

```text
Lobby Begin_WorldEntry
-> endpoint resolver worker
-> adapter별 directed broadcast 0/500/1500ms
-> source address 후보 collect/validate/dedupe
-> 2000ms 창 종료 뒤 유일한 compatible Server 확정
-> connector worker가 후보별 TCP connect
-> main CNetworkManager가 결과 소비
-> 기존 C2S_ENTER_WORLD/S2C_ENTER_ACCEPTED
```

이 G에서는 active-world session resume을 아직 켜지 않는다. discovery/connect 실패는 Lobby에 남고 기존
terminal 동작을 사용한다. 이렇게 동일 LAN IP 입력 제거를 첫 번째 독립 PR로 검증할 수 있다.

focused harness는 explicit override exclusive, `0.0.0.0` reject, loopback fixture 우선, Public-profile skip,
blind rollback-host 미사용, discovery timeout,
wrong/duplicate/stale nonce, 같은 boot ID 다중 NIC dedupe, 서로 다른 boot ID ambiguity, DNS multi-address,
cancel과 2,000ms deadline을 검증한다. 첫 query 유실 뒤 두 번째/세 번째 round 성공, 1,500ms 뒤 다른 boot가
도착하면 자동 선택 금지, 후보/adapter/datagram cap 초과와 stale scan 응답도 검사한다.
injected adapter/NLM snapshot에서 Private ingress는 reply하고 Public/unknown
ingress는 response 0개인지도 검사한다. discovery rate fixture는 256 live source 뒤 257번째 source가 state 증가나
reply 없이 drop되고, per-source/global burst와 injected refill/idle expiry가 exact bound를 지키는지 검사한다.

sync script는 Server role에서 repo-owned firewall expected set을 reconcile한다. PR A expected set은 Debug/Release
Server program별 TCP 7777과 UDP 7778, `Private,Domain`, `LocalSubnet` rule뿐이다. 같은 group의 old `Profile=Any`,
stale port/CIDR rule과 exact repo-path legacy ungrouped v1 TCP rule을 제거하고 Client role에서도 이 checkout의
exact program path인 grouped/legacy inbound rule만 제거한다. 다른 checkout program의 rule은 그대로 보존한다.
PR A에는 routed CIDR
rule이나 AUTO routed candidate를 아직 만들지 않는다. script harness는 Server->Client role 전환과 port 변경에서
stale rule 0개임을 검사한다. legacy Debug/Release `Profile=Any` fixture도 v2 Server sync 뒤 0개, Client 전환 뒤
0개여야 한다. expected mutation에 관리자 권한이 없으면 rule ready를 출력하지 않고
`firewall-reconciliation-requires-administrator`로 즉시 실패·안내하며, endpoint/debugger local 설정 완료와
firewall 미완료를 분리해 보고한다.

명시 `-UseAutoDiscovery` 이전의 custom DIRECT row 보존, 명시 전환 뒤 host 제거, 다음 기본 sync의 재주입 0건,
unrelated environment 보존, Server role이 `.20` 주소 소유를 더 이상 요구하지 않는 것도 script fixture로 검증한다.

`Run-LanDiscoverySmoke.ps1`는 실제 구성별 `Server.exe`를 명시 `127.0.0.1` TCP port와 동적 discovery port로
`--network-contract-harness`와 함께 시작하고, focused harness가 unicast query를 보내 응답 source, echoed nonce,
advertised TCP port와 boot ID를 검증한 뒤 bounded 종료한다. 이 격리 smoke의 loopback trusted fixture는 exact
loopback bind에서만 허용하며 팀 endpoint 자동 변경이나 Public/unknown ingress 허용 대상이 아니다.
기존 `Run-ValtanFourPlayerHarness.ps1`와 `Run-CharacterSelectIsolationHarness.ps1`는
`--disable-discovery`를 명시해 실행 중인 팀 Server의 UDP 7778과 충돌하거나 테스트 Server를 LAN에 광고하지
않는다.

## G03. Client async transport와 initial-entry retry

`Connect_To_Server`의 main-thread 1.5초 `select`를 `CClientTransport` connector worker로 옮긴다. DNS 결과를
순서대로 bounded connect하되 result에는 endpoint source, WSA error, elapsed와 transport generation을
포함한다. `CNetworkManager::Update()`는 result만 소비한다.

DNS는 blocking `getaddrinfo` worker가 아니라 overlapped `GetAddrInfoExW`와 `GetAddrInfoExCancel`을 사용한다.
각 ref-counted resolver operation이 `OVERLAPPED`, cancel handle, result/address buffer와 completion context를
completion callback까지 함께 소유한다. 5초 entry deadline/Level cancel은 operation generation을 cancelled로
mark하고 `GetAddrInfoExCancel`을 요청한 뒤 UI/Level에는 즉시 돌아간다. resolver service가 late completion까지
operation을 계속 소유하고 result만 generation-stale로 버리므로 cancel 직후 context/OVERLAPPED를 destroy하거나
DNS thread를 unbounded join하지 않는다.
service는 cancelled-but-incomplete를 포함해 outstanding operation 최대 16개를 fixed table로 소유하고 cap에서는
새 resolve를 `RESOLVER_BUSY`로 거부해 repeated cancel이 heap/context를 무한히 늘리지 못하게 한다.

process shutdown은 새 operation admission stop -> 모든 DNS cancel -> connector socket shutdown/close+worker join
-> resolver completion queue bounded 3초 drain 순서다. cancel completion이 3초 안에 오지 않으면 outstanding
operation/COM/Winsock storage를 파괴하거나 detached leak로 process를 계속하지 않고 dedicated
`NETWORK_RESOLVER_SHUTDOWN_TIMEOUT` exit code로 fail-fast한다. focused fake resolver는 Level cancel 뒤 completion을
지연해 frame 복귀와 stale-result discard를 확인하고, cancel completion을 끝까지 무시하는 child-process case가
3초 timeout exit code이며 context destructor/UAF가 없는지 검사한다. delayed cancel 16개 뒤 17번째 resolve가
allocation 없이 `RESOLVER_BUSY`이고 completion drain 뒤 다시 admission되는 cap case도 포함한다.

PR A v40 initial admission에서 자동 retry 가능한 구간은 TCP connect 실패 또는 `C2S_ENTER_WORLD` frame의 어떤
byte도 socket에 commit되기 전뿐이다. enter frame이 일부/전부 전송된 뒤 EOF/reset/approval timeout이 나면
Server가 이미 player를 commit했는지 알 수 없으므로 `AMBIGUOUS_ENTRY` terminal로 끝내고 자동 재입장하지
않는다. v40에는 entry attempt nonce/idempotency가 없어 이 경계를 넘겨 retry하면 duplicate player나 자기
자리의 `ROOM_FULL`을 만들 수 있다.

`ROOM_FULL`, protocol decode/version 위반, invalid content revision, 사용자 cancel과 load/presentation failure도
terminal이다. explicit DIRECT 실패는 AUTO 후보로 우회하지 않는다. PR B에서는 ticket/Server hello/lease가
ambiguous post-send reconnect를 대체한다.

initial entry는 탐색/DNS/connect에 최대 5초(그 안의 LAN 수집 2초), 그 뒤 Server approval에 최대 5초를 쓴다.
사용자 intent부터 전체 outer budget은 최대 10초이며 retry/다른 후보/hello가 이 deadline을 초기화하지 않는다.
PR B의 pending admission 5초와 복구 grace 15초는 별개의 Server deadline이다. retry/backoff 동안 main thread와
Lobby frame은 계속 진행하며 deadline을 넘긴 worker result는 attempt generation에서 stale로 폐기한다.

Lobby pending create identity는 retry 동안 유지하고 최종 terminal/cancel에서만 rollback한다. connect와
approval 상태를 사용자에게 짧게 표시하되 token, raw endpoint credential과 내부 stack을 출력하지 않는다.

send worker와 inbound generation fence를 이 G에서 먼저 제품 경로에 연결한다. old direct `Send_All` 호출을
남겨 두 번째 socket path를 만들지 않는다. Debug loopback와 기존 명시 `127.0.0.1` harness는 discovery를
거치지 않고 그대로 동작해야 한다.
다음 hello 시험은 G03 단독 PR A가 아니라 **G04 protocol v41 연결 뒤 PR B 통합 검증**에서 수행한다.
focused peer가 Client hello 직후 EOF/reset하는 DIRECT case는 nickname/token 0 byte와
`POSSIBLE_PROTOCOL_VERSION_MISMATCH`를 기대하되 remote exact version을 40이라고 단정하지 않는다. discovery
codec은 v40 query <-> v41 responder 양방향 mismatch status/server version을 별도로 고정한다.

## G04. Server logical lease, detach와 expiry

PR B 시작에서 `NETWORK_PROTOCOL_VERSION`을 통합 기준 40에서 41로 올리고 G01에서 미리 만들지 않은
Server hello/entry-attempt/cancel/ticket/resume/leave/heartbeat/full-resync packet과 fixed byte reader를 추가한다.
v41 `C2S_ENTER_WORLD`는 기존 bounded payload에 exact 128-bit `entryAttemptId`를 추가한다. 이 시점에
`NetworkProtocolHarness`가 모든 새 TCP message round-trip, token size, unknown enum/version, truncation,
trailing byte와 destination-preserving decode를 Debug/Release에서 먼저 통과해야 Server/Client runtime
consumer를 연결한다.

v41 transport의 first packet은 `C2S_SERVER_HELLO` 하나만 허용한다. hello ACK 뒤 같은 transport의 5초
handshake window 안에 initial selector `C2S_ENTER_WORLD`, `C2S_SESSION_RESUME` 또는 `C2S_ENTRY_CANCEL` 정확히
하나가 와야 한다. selector 뒤에는 해당 state의 ticket ACK/cancel/resume commit/leave follow-up만 허용한다.
token/nickname을 hello 전에 보내거나 unknown first packet을 보내면 protocol terminal이며 lease를 만들지 않는다.
cancel도 exact transport generation/attempt ID로 lane에 들어가며,
pending이면 release, cancelled tombstone이면 idempotent ACK, expired면 `ENTRY_ATTEMPT_EXPIRED`, active로 commit된
attempt면 attempt ID만으로 `ENTRY_ATTEMPT_COMMITTED`를 답하고 state를 바꾸지 않는다. exact currently bound
pending generation 또는 valid pending/active session token이 있어야 cancel authority가 생기며, active credential은
explicit-leave cleanup으로 승격한다. 전혀 모르는 ID는 `ENTRY_ATTEMPT_NOT_FOUND`이고 room state를 바꾸지 않는다.

first enter 전 transport와 enter accepted 뒤 logical lease를 분리한다. `CSessionLeaseRegistry`는 room thread에서
직렬화하고 다음 상태만 허용한다.

```text
PENDING_HANDSHAKE
-> PENDING_ADMISSION
-> ACTIVE
-> GRACE
-> RESUME_PENDING
-> ACTIVE
-> EXPIRED / REVOKED
```

invalid transition은 fallback enum으로 정상화하지 않고 contract failure다. handshake 5초 timeout,
pending admission timeout, protocol corruption과 explicit leave는 revoke한다. `PENDING_ADMISSION`은 slot/spawn과
ticket만 예약했고 gameplay player는 아직 commit하지 않은 상태다. 이 transport가 끊겨도 GRACE로 바꾸거나
original 5초 deadline을 연장하지 않으며 같은 `entryAttemptId` ENTER 또는 exact pending-token resume만 허용한다.
current `ACTIVE` transport의 transient close만 room-thread detachedAt부터 full 15초 GRACE로 간다. logical capacity의 fresh-entry rejection은
v41 `SERVER_BUSY`, world spawn 부족은 기존 `ROOM_FULL`을 사용한다.

`GRACE -> RESUME_PENDING`은 original `detachedAt + 15,000ms` deadline을 그대로 상속하고 result,
full sync 또는 COMMIT 재전송으로 연장하지 않는다. deadline에 도달하면 pending successor raw slot을 zeroize하고
transport generation을 close한 뒤 exactly-once expiry한다. 14,999ms에 resume result가 accepted됐어도
15,000ms 이후 도착한 `SESSION_RESUME_COMMIT`은 `EXPIRED`이며 player/slot을 되살리지 않는다.

`CGameRoom::Detach`와 `Rebind`는 stable logical session을 사용한다. private Character Select room/session,
world transfer와 data-revision transaction의 ordering도 room thread에서 확정한다. 연결 단위 Debug audition과
data-revision 2PC는 detach 즉시 abort하지만 product player/world state는 유지한다.

same-socket world transfer는 logical lease/token을 교체하지 않는다. `CServerApp`/global room thread가 lease state와
직교하는 `WORLD_TRANSFER_TRANSACTION` record를 stage하고 lease는 commit 전까지 `ACTIVE`와 source binding을
그대로 유지한다. target room의 위와 같은 spawn/occupancy reservation, target-room monotonic allocator에서 새
PlayerId/NetEntityId claim, transfer-authorized state install, target full-sync/outbound batch와 모든 map mutation을
먼저 preflight한다. target ID counter는 claim 때 advance하고 rollback에도 재사용하지 않는다.

준비가 끝나면 single room-thread transaction이 source map remove + target map install + lease binding 교체 +
reserved batch publish를 no-fail commit한다. 어느 preflight라도 실패하면 target reservation만 rollback하고 source
player/binding은 untouched다. lifecycle FIFO observation은 transaction 전 또는 후 경계에서만 처리되어 disconnect가
전이면 source, 후면 target exactly one room으로 resume하며, source/target 두 room 동시 잔존이나 zero-room window와
fresh `Join`을 만들지 않는다. target accepted/full-resync는 새 authoritative PlayerId/NetEntityId와 generation을
보내고 Client Loading은 transaction을 one-shot handoff한다. logical session/token, class/nickname과 command sequence
state는 계속 보존한다.

commit은 monotonic transferEpoch, source world/IDs와 target world/new IDs의 immutable receipt를 lease에 같이 저장한다.
target accepted batch가 0 byte 전달된 뒤 detach돼도 다음 authenticated resume result/full closure가 이 receipt를
재전송해 source presentation만 가진 Client가 target identity를 검증·채택할 수 있어야 한다.

contract test는 target full, ticket과 commit 사이 world-entity allocator advance, target ID collision,
reservation/queue/batch/map preflight fault와 commit 경계 앞/뒤 disconnect를 주입한다. 모든 경우 player는 exact 한
room에 남고 logical session/class/nickname/sequence를 보존하며, success는 target의 새 unique IDs, failure는 source
identity/state byte-for-byte 유지여야 한다.
별도 case는 transfer commit -> target batch 0-byte loss -> detach -> source presentation Client resume에서 receipt로
target을 정확히 한 번 채택한다. stale/replayed epoch, source identity mismatch, disallowed edge와 target closure
mismatch receipt는 partial Level transition 없이 reject한다.

Server contract test는 fake steady clock과 deterministic token seam을 사용해 room-thread detachedAt 기준
14,999ms에는 자리 보존, 15,000ms에는 exactly-once expiry임을 검증한다. room-thread가 14,999ms에 resume validation을
끝내 result를 accepted했어도 exact deadline 뒤 COMMIT은 `EXPIRED`이며, 15,000ms에 처리한 request는 result 전부터
`EXPIRED`가 이긴다. 두 경우 pending successor raw slot은 zeroize한다. lifecycle lane을 가득 채워도
room-owned expiry가 producer wait 없이 실행되어야 한다. shutdown은 grace player를 무한 대기하지 않고
revoke/room cleanup 뒤 bounded join한다.

initial ticket 0-byte/partial 미수신, 같은 attempt의 duplicate ENTER, payload conflict, pending timeout,
만료 tombstone 뒤 동일 attempt, pending cancel/duplicate cancel/cancel-vs-ACK commit race, ticket 수신 뒤 ACK 유실,
CONFIRMED 유실과 ACK 직후 disconnect를 각각 검사한다.
attempt ID만 탈취한 다른 transport의 pending/committed cancel은 reservation/player/binding을 전혀 바꾸지 않고,
valid bound-generation 또는 token cancel만 cleanup하는 negative authority case도 포함한다.
ticket/result frame 0-byte loss에서는 pending sensitive slot의 exact raw token을 byte-identical 재전송하고,
admission/resume commit·cancel·expiry·revoke 뒤에는 injected sensitive-arena seam이 해당 slot 전부 zeroized/free임을
검사한다. sensitive frame encode failure, queued partial-send close, parser truncation/compaction, stale event drop와
Client terminal destruction도 wipe hook으로 검사한다. steady ACTIVE/GRACE registry와 폐기된 queue/parser storage에는
digest 외 raw token sentinel이 남지 않아야 한다.
ticket 미수신 재접속은 같은 ticket/slot 하나만 사용하고 duplicate player나 자기 자리의 `ROOM_FULL`을 만들지
않아야 한다. pending timeout은 reservation/token만, ACTIVE revoke는 committed player/binding/private room을
exactly once 정리한다. max-bound initial/full-resync batch, batch reservation capacity failure와 staged join map
failure도 검사해 confirmation/accepted 0 frame과 room/lease/binding 0 partial commit을 확인한다.
별도 4인 reservation case는 unconfirmed pending 네 개가 서로 다른 spawn을 claim하고 fifth가 `ROOM_FULL`, 같은
attempt duplicate는 추가 claim 0개, cancel/timeout 하나 뒤 replacement 하나만 그 slot을 claim하는지 검사한다.
world-transfer target reservation도 같은 occupancy 함수와 rollback 결과를 사용해야 한다.

일반 reliable gameplay queue를 960까지 채운 상태에서도 ticket ACK, pending cancel과 `SESSION_RESUME_COMMIT`은 control lane을
통해 정확히 한 번 transition하고 duplicate에는 exact ACK/COMMITTED를 재전송해야 한다. shutdown fault test는
session stop 요청 뒤 늦은 close callback을 발생시켜 worker join/producer-count 0 전에 room cleanup이 시작되지
않고, empty-lane barrier 뒤 player/lease가 0인 상태로 끝나는지 검사한다.
exact current owner transport가 duplicate ACK/commit/cancel을 257회 보내는 flood는 transition exactly once, offending transport만
terminal rate-limit reject/lease revoke, Server process/다른 session 생존이어야 한다. reject frame 유실 뒤 resume도
`REVOKED`로 끝나 재연결 loop가 없어야 한다. 64 transport가 동시에 one valid control과 close를 보내는 burst도
control 192/structural 64 partition을 넘지 않고 모든 structural detach가 exactly once 처리되어야 한다.
같은 257-flood를 stale old generation과 `ALREADY_ATTACHED` unbound transport에서 반복하면 offender socket만 닫고
current player/lease/bound generation/gameplay state는 byte-for-byte 무변경이어야 한다.
room consumer를 멈춘 채 sequential accept/close churn을 64회 넘게 시도하는 case는 `accepted+closing <= 64`, 65번째
accept reject, structural reserve overflow/fatal 0을 확인하고 drain/reap 뒤에만 permit이 다시 admission되어야 한다.

## G05. Client recovery와 transactional full resync

active-world transport loss에서 `CClientReplication::Reset_World()`와 Level의 즉시 Lobby 요청을 미룬다.
NetworkManager가 `RECOVERY_WAIT/RESUMING/RESYNCING`이면 다음을 적용한다.

- current presentation과 HUD의 마지막 Server state 유지
- gameplay mouse, keyboard, quick-slot, held/ground target command 제출 차단
- controller edge/held state clear; resume 뒤 새 press가 있어야 전송
- recovery 상태와 Client local budget을 표시하되 Server의 exact expiry 시각처럼 표현하지 않음
- stale transport generation frame 폐기

Server resume accepted 뒤 full sync를 별도 generation에 stage한다. full sync는 initial join late-state send와 같은
Server helper를 사용하며, Client는 old registry를 수정하지 않은 채 expected counts/IDs/revisions를 검증한다.
local atomic commit 뒤에도 input을 freeze하고 `S2C_SESSION_RESUME_COMMITTED`를 받아 Server token/lease commit이
확정된 뒤에만 gate를 연다.

`RESUME_RESULT.world`가 current Level descriptor와 같을 때만 현재 Level layer에 in-place stage/commit한다. Server의
world-transfer transaction이 disconnect보다 먼저 commit되어 authoritative world가 다르면 expected-world mismatch를
resync corruption으로 처리하지 않는다. 먼저 committed transfer receipt의 boot/session/source/allowed edge/unseen
epoch/target closure 조건을 모두 검증한다. NetworkManager가 result와 bounded wire transaction을 one-shot 보존하고
`CLevelTransitionService`에 exact target descriptor의 typed transition을 제출한다. Loading이 target visual map과
`CClientReplication`/layers를 준비한 뒤 그 target에만 transaction을 commit하고, 성공 뒤
`SESSION_RESUME_COMMIT`, `SESSION_RESUME_COMMITTED` 순서까지 input을 freeze한다. source layer는 target preflight
전에는 건드리지 않으며 target load/commit 실패는 partial swap 없이 terminal leave cleanup으로 간다.
`INITIAL_ADMISSION_COMMITTED`도 같은 Loading handoff를 사용한다.

focused world-transfer race는 Server transaction 전 detach에서 source world/in-place resume, transaction 후
detach에서 target world/정확히 한 번 Loading transition을 기대한다. source resume은 기존 IDs, target resume은
validated committed receipt의 새 authoritative PlayerId/NetEntityId를 transactionally 채택하며 logical session과
command sequence를 유지한다. target batch 0-byte loss와 Client pending marker 없음도 정상 adoption case다. 정상
target-world/ID 차이를 resync validation failure로 분류하지 않는다.

각 Level은 기존 camera/UI gate와 `CNetworkManager::CanSubmitGameplayCommands()`를 conjunction으로 만들어
`CPlayerController::Update(false)`도 recovery frame마다 계속 호출한다. 이 기존 false 경로가 ground target,
held slot/HOLD와 LMB edge를 physical release까지 막는다. RMB에는 기존 release gate가 없으므로
`Update(false)`가 raw RMB down을 보면 별도 move-suppression-until-release latch를 세운다. gate가 다시 열려도
물리 RMB가 계속 눌린 동안 move/resend는 0건이며, release를 관측한 뒤 새 press에서만 보낸다. Character
Select처럼 Update 자체를 skip하는 경로는 false 호출로 바꾼다. `CNetworkPlayerCommandSink`는 두 번째
generation/phase fence로 남는다.

sink 경계만으로는 충분하지 않으므로 `CNetworkManager`의 모든 gameplay/world/debug/data-revision mutation
`Send_*` entry point가 공통 `ACTIVE_COMMITTED && currentTransportGeneration` gate를 가장 먼저 통과한다.
`RESUMING/RESYNCING`에는 hello, heartbeat, ticket ACK, resume/commit, leave/cancel 같은 명시 session-control
allowlist만 outbound queue에 들어갈 수 있다. `CNetworkWorldEntityCommandSink`, Valtan Level/service,
`MainApp`, Balance Tool처럼 NetworkManager를 직접 부르는 caller도 이 중앙 gate를 우회하지 못한다. focused
harness는 move/skill/world interaction/debug audition/data-revision family의 대표 direct API를 recovery phase마다
호출해 gameplay mutation outbound byte가 0인지 확인한다.

move/action next sequence는 Level별 value-member `CPlayerController`가 단독 소유하지 않고 NetworkManager의
logical-session scoped `PLAYER_COMMAND_SEQUENCE_STATE`가 소유한다. fresh accepted session만 새 state를
`move=1/action=1`로 만들고, accepted sink submission이 이 shared state를 증가시킨다. same-session clone swap과
world transfer는 `LevelTransitionService` one-shot handoff로 같은 state identity를 target Level controller에
`Bind_CommandSequenceState`한다. terminal cleanup 뒤에만 state를 폐기한다.

full-resync layer swap commit receipt에는 `logicalSessionContinued`, new local-character handle과 exact sequence-state
identity를 넣는다. 같은 Level의 successful commit은 generic `Set_LocalCharacter`보다 먼저
`CPlayerController::Rebind_LocalCharacter`를 호출하고, pointer가 이미 rebind되어 뒤의 Set은 no-op이다. 다른
Level의 target controller는 character rebind 전에 handed-off sequence state를 adopt한다. `Set_LocalCharacter`는
character 교체만으로 session sequence를 1로 reset하지 않는다. Rebind는 move resend timestamp를 지워도 RMB
release latch를 지우지 않는다.

PR B의 `NetworkConnectionContractHarness`는 `ClientFullResyncTransaction`과 Engine layer transaction fixture도
링크한다. BEGIN/data/END 뒤 ordinary delta ordering, expected-count mismatch, clone failure, stale old-layer
identity, two-layer preflight failure와 successful no-partial swap을 검증한다. raw-socket Valtan/CharacterSelect
harness만으로 Client presentation atomicity를 통과 처리하지 않는다. focused input fixture는 move/action을 N까지
accepted한 뒤 같은-controller clone swap과 source-Level -> 새 target-Level controller handoff를 각각 수행하고 첫
새 move/skill이 N+1이며 Server replay fence에서 승인되는지 확인한다. loss 당시 RMB held인 경우 COMMITTED 뒤 계속
held에서는 move 0건, physical release 뒤 새 press에서는 정확히 한 건의 N+1 move여야 한다.

resume reject/expiry/server restart/protocol error는 typed reason과 cleanup fence를 남기고 terminal reset/Lobby
recovery를 정확히 한 번 실행한다. clone/load/resync validation failure처럼 transport/session control이 valid한
경우에는 먼저 leave/ACK cleanup을 끝내고 ticket을 지운다. leave ACK가 오지 않으면 presentation은 terminal로
reset하되 같은 boot fresh entry는 Server grace/revoke가 확인될 때까지 막아 ghost slot의 `ROOM_FULL`을 피한다.
Bern, Valtan, Development, Character Select, Loading과 world transfer는 raw `Is_Connected()`가 아니라 typed
connection phase/terminal reason/cleanup fence를 소비한다.
focused fault case는 clone validation 실패 뒤 `SESSION_LEAVE`와 ACK 전 fresh ENTER 0건, ACK 뒤 slot release와 새
entry 허용을 확인하고, ACK 유실 case는 cleanup fence 동안 fresh entry가 계속 차단되는지 확인한다.

## G06. 4인·private arena와 fault regression

`ValtanFourPlayerHarness`의 현재 "disconnect 즉시 3명, fresh replacement 4명" 기대를 다음 계약으로 교체한다.

1. 네 Client가 입장한다.
2. 한 transport를 abrupt close한다.
3. grace 동안 room은 4/4이며 fifth fresh Client는 `ROOM_FULL`이다.
4. valid resume은 같은 PlayerId/NetEntityId이며 HP/position/action은 fresh-spawn reset이 아니라 disconnect 동안
   계속 진행된 현재 authoritative Server state와 일치한다. 다른 세 Client에는 duplicate spawn/despawn이 없다.
5. stale/duplicate token race는 정확히 한 transport만 이긴다.
6. 별도 scenario에서 15초 expiry 뒤 한 번만 despawn하고 replacement를 받아 다시 4/4가 된다.

`CharacterSelectIsolationHarness`는 private simulation이 grace 동안 유지되고 다른 session에 state를 broadcast하지
않으며 resume 뒤 동일 simulation을 소비하는지 확인한다. expiry 때만 queued leave, audition reset과 private
room retirement를 실행한다.

추가 fault는 half-open heartbeat timeout, nonblocking would-block 뒤 exact partial offset 진행,
실제 send timeout 뒤 같은 stream 재사용 금지, resume result/commit/committed
각 ACK 유실, initial ticket 0-byte/partial/ACK/confirmed 유실, entry cancel/ACK race, old transport late close,
room queue 포화 중 ticket ACK/resume commit, shutdown producer barrier, Server restart, world transfer와 detach
race다. production 15,000ms 경계는 fake-clock contract test가 검사하고 live runner는
loopback 전용 `--network-contract-harness`에서만 750ms grace를 주입한다. 일반 Server 실행은 이 override를
거부한다.

Debug revision 2PC는 GRACE 중 prepare 거부, prepare 중 detach abort, old-generation ACK 거부와 resync 동안
revision pinning을 검증한다. CSO/main-thread 작업을 8초 이상 지연한 fixture에서도 worker heartbeat가 계속
진행되면 network timeout이 발생하지 않아야 한다. 실제 네트워크까지 유실된 fixture만 loss/grace 정책을 탄다.

## G07. v41 cross-network 운영 경계

명시 DIRECT의 cancellable `GetAddrInfoExW` 지원은 G03/PR A에 포함한다. 자동으로 다음 trusted routed
endpoint를 고르는 AUTO routed tier만 PR B의 Server hello와 함께 활성화한다. VPN/overlay 자체를 앱이
설치하거나 구성하지 않는다.

운영자는 `TeamLanEndpoint.json`의 한 routed profile에 다음 셋을 함께 넣어야 routed mode가 admission된다.

- resolvable trusted FQDN 또는 IPv4
- resolved Server 주소를 제한하는 exact private/overlay `trustedServerEndpointCidrs`
- Server inbound remote source를 제한하는 exact private/overlay `trustedClientSourceCidrs`

sync script는 FQDN A record/Server interface가 `trustedServerEndpointCidrs`에 있고
`trustedClientSourceCidrs`로 가는 route가 실제 overlay next-hop을 쓰는지 별도로 확인한다. Windows Firewall
`RemoteAddress`에는 Client-source 목록만 넣고 Server-endpoint 목록을 합치지 않는다. 두 목록 중 하나가 없거나
public CIDR이면 자동 open을 거부한다. Client-source CIDR이 A에서 B로 바뀌거나 빈 routed profile이 되면 같은
실행에서 A rule을 제거하며, Server->Client role 전환도 LocalSubnet/routed rule을 모두 제거한다. routed
next-hop/interface의 NLM profile도 Private/Domain이어야 하며 Public/unknown이면 exact CIDR rule도 만들지 않고
`ROUTED_INTERFACE_PROFILE_NOT_TRUSTED`로 중단한다. script contract test는 endpoint/source CIDR이 서로 다른
site-to-site fixture, Client-source A -> B -> none, Server -> Client expected-set diff와 routed Private/Public profile을
firewall/route/NLM cmdlet mock으로 검증한다. endpoint CIDR이 firewall RemoteAddress에 섞이거나 source CIDR을 DNS
pinning에 쓰면 실패다.

Client는 매 resolve/reconnect에서 모든 A record를 `trustedServerEndpointCidrs`로 검사한다. PR B AUTO initial entry는 후보별 TCP hello만
remaining entry budget 안에서 수행하고 nickname/ENTER는 아직 보내지 않는다. 같은 stable team ID의 hello가
동일 boot ID면 multi-A를 dedupe한 뒤 deterministic endpoint 하나로 진행하고, 서로 다른 boot ID면 stale DNS의
첫 성공을 채택하지 않고 `AMBIGUOUS_SERVER`로 끝낸다. DIRECT는 명시 endpoint의 주소만 순차 시도하는 예외다.
DNS multi-A focused test는 same-boot dedupe, different-boot ambiguity와 ambiguity 전에 nickname/token 0 byte를
고정한다. token은 bearer credential이므로 TLS 없는 public 인터넷을 신뢰 경계로 만들지 않는다.

같은 hotspot인데 AP isolation이 켜진 경우 UDP discovery와 direct TCP가 모두 막힐 수 있다. 이 경우 코드가
무한 대기하거나 local gameplay로 fallback하지 않고 `LAN_DISCOVERY_BLOCKED_OR_NO_SERVER`를 표시하며,
trusted routed endpoint가 있을 때만 그 후보를 시도한다.

## G08. 문서, rollout과 PR admission

이번 턴은 계획서에서 멈춘다. 구현 승인을 받은 뒤에는 다음 세 검증 단위로 나눈다. 기존 문서의
`PR A`/`PR B` 표기는 범위 식별자로 유지하되, 사용자가 요청한 **통합 PR 안의 독립 commit/gate**로 묶을 수 있다.
문서만 있거나 다른 작업이 진행 중이라는 이유로 그 기능을 통합 완료/merge 가능으로 취급하지 않는다.

### PR P — 통합 로딩률

- G09의 단일 loadEpoch, 작업 가중치, CSO/core 계측과 실제 activation ACK
- 기존 EffectRenderContractHarness의 진행률/ACK/실패 실행형 회귀와 shader closure
- protocol 변경 없이 독립 검증 가능. A/B보다 먼저 구현·검증할 수 있음

### PR A — 자동 endpoint와 async initial connection

- G00 lifecycle baseline
- G01 discovery codec와 focused harness 기반, TCP protocol v40 유지
- G02 trusted LAN UDP discovery
- G03 Client async initial connect와 explicit DIRECT DNS
- Team endpoint schema/sync/firewall와 LAN 운영 문서

PR A는 gameplay session의 disconnect 동작을 바꾸지 않는다. 같은 LAN에서 IP 입력 없이 진입하는 두 PC
수동 확인까지 별도 증거를 받는다. PR P와 A만 통과했다고 B의 reconnect를 완료 처리하지 않는다.

### PR B — 15초 logical lease와 resume

- G04 시작의 protocol v41 session ticket/resume/full-resync codec와 harness
- G04 Server lease/detach/rebind/expiry
- G05 Client freeze/resume/transactional resync
- G06 4인/private arena/fault regression
- G07 Server hello 기반 routed endpoint/security admission과 최종 public 문서

PR B는 Client/Server/Shared/harness를 같은 revision으로 stop-the-world 배포한 뒤 Server부터 시작하고 v41 Client만
연다. compatibility fallback은 만들지 않는다. PR A discovery schema를 아는 v40/v41 build의 AUTO LAN은
`PROTOCOL_MISMATCH(serverTcpProtocolVersion)`를 typed 표시할 수 있다. DIRECT/routed에서 v41 Client가 hello response
전 EOF/reset을 받으면 실제 old Server version을 단정하지 않고 `POSSIBLE_PROTOCOL_VERSION_MISMATCH` terminal로
표시한다. v40 Client가 v41 Server의 control reject를 이해하는 것은 보장하지 않아 generic EOF가 될 수 있으므로
혼용 운영 자체를 admission하지 않는다.

public 문서는 구현 후 실제 명령과 검증 결과에 맞춰 갱신한다. 계획에 적었다는 이유만으로 AGENTS의 static
`.20` 계약을 먼저 지우지 않는다. PR A 적용 전 schema v1에서만 `.20`이 endpoint/server-host 판정
정본이다. schema v2 전환 뒤에는 `rollbackHost`라는 수동 DIRECT 안내값만 되고 local role marker가 Server
machine identity를 소유한다.

### 통합 순서와 겹치는 파일

네트워크 A/B 통합 브랜치는 PR #247의 실제 merge commit/검증을 기준으로 시작한다. protocol 비의존 P는
현재 main에서 먼저 독립 구현·검증한 뒤 그 commit을 통합할 수 있다. Next Pattern과 Screen Post/Camera
Shake는 각 작업의 실제 code/RESULT가 준비된 것만 통합 대상으로 삼고, 아직 계획인 기능을 placeholder로
넣지 않는다. wire는 §1.6의 한 번의 통합 version 갱신을 사용한다.

`MainApp.cpp`, `LevelTransitionService.*`, `Level_Loading.*`는 P와 B가 함께 변경하므로 한 소유자가 loadEpoch,
network generation, activation gate를 함께 조정한다. `NetworkManager.*`, `PacketMessages.*`, `ServerApp.*`,
`GameRoom.*`는 Next Pattern/PR #247의 최신 diff를 확인한 뒤 적용하고, public enum/payload는 단순 충돌 선택으로
합치지 않는다. Effect 공용 renderer는 Screen Post/Camera Shake의 실제 core 경로를 다시 확인한다.

최종 통합 gate는 Debug/Release build/regression, publisher validation, P/A/B 실행형 harness와 사용자 수동
관찰이다. 네트워크 가이드, CLAUDE, 팀 interface, 변경된 public 경계와 대응 RESULT만 같은 검증 단위로 갱신한다.
다른 작업의 무관한 dirty 파일, CSO/exe/EngineSDK/중간 산출물은 stage하지 않는다. commit/push/PR/merge 실행은
계획 이후의 별도 사용자 구현·배포 요청에서만 진행한다.

## G09. CSO·맵·캐릭터·Effect를 묶는 단일 로딩 진행률

### G09-1. 목표와 보존할 경로

한 번의 Loading 전환에서 전체 막대가 `99% → 0%`로 돌아가지 않게 한다. 그 목표 때문에 fake timer나
별도 resource runtime을 만들지 않는다. PR #243의 compiled CSO, Loader worker device-only stage,
main-thread bounded commit, capacity-2 result channel, exact `TARGET_COMMIT_ACK`를 그대로 사용한다.

현재 제품은 `Effect_PresentationService.cpp:2761`에서 stale/invalid worker result를 structural failure로
처리한다. load job에 REBASE primitive가 있다는 사실만으로 자동 revision 재시도가 구현됐다고 보지 않는다.
**이번 G09는 자동 REBASE를 새로 활성화하지 않는다.** 현재 실패 정책을 보존해 같은 epoch의 denominator나
revision을 임의 교체하지 않는다. 새 명시적 loading/retry만 새 epoch로 시작한다.

### G09-2. 파일 책임과 H 계약

신규 `Client/Public/LoadingProgress.h`는 현재 `CLevelTransitionService`, Loader/Loading과 기존 Effect harness가
직접 소비하는 순수 값/계산 계약이다. socket, D3D, resource 생성, UI object를 소유하지 않는다. 별도 Manager나
두 번째 Loader를 만들지 않는다. fixed lane storage는 `<array>`, epoch/count는 `<cstdint>`, clamp/min은
`<algorithm>`, finite 검증은 `<cmath>`를 사용한다. Level/descriptor 해석과 상태 문자열의 소유권은 기존
service/Loader snapshot에 두고 숫자 helper가 Engine/Network header에 의존하지 않게 한다.

| 제안 선언/상태 | 의미·단위·소유자 |
|---|---|
| `LOADING_WORK_LANE` | 아래 7개 lane의 typed ID. unknown/sentinel을 정상 lane으로 fallback하지 않음 |
| `LOADING_WORK_PLAN` | service가 검증한 target/profile의 loadEpoch와 lane별 fixed weight. 합은 10,000 basis points |
| `LOADING_WORK_SNAPSHOT` | loadEpoch, lane, 실제 완료/전체, determinate/terminal 상태. Effect 관측에는 jobEpoch/catalogRevision도 포함 |
| `loadEpoch` | MainApp가 실제 새 Loading transaction을 시작할 때 한 번 발급. socket generation/Effect jobEpoch/character ID와 별개 |
| lane ledger | main-thread `CLevelTransitionService` 소유. 최초 valid plan/count를 고정하고 이후 단조 완료만 반영 |
| `earnedBasisPoints` | 실제 완료가 획득한 작업 credit. 정상 준비 중 0..9,900, 실제 activation 성공만 10,000 |
| `displayProgress` | `CLevel_Loading`의 표시 전용 0..1 값. 상태·완료·activation 판단에 쓰지 않음 |
| 현재 작업 ID/elapsed | phase 진단용 logical shader/asset ID와 steady elapsed. 시간 자체는 progress를 올리지 않음 |
| terminal snapshot | service가 마지막 성공/실패/취소 한 건만 보존. Loading object 파괴 뒤에도 MainApp 결과와 원인이 남음 |

`Begin_Load`의 실제 호출자는 MainApp의 Loading 생성 경계, `Observe_Work`는 Loading이 읽은 worker immutable
snapshot, `Complete_Activation`은 MainApp의 성공 경계다. worker가 service나 CUIObject를 직접 수정하지 않는다.
`Set_Status()`는 상태 문자열/현재 작업 elapsed만 바꾸며 전체 ledger를 초기화하지 않는다.

### G09-3. 전체 가중치와 실제 완료 단위

아래는 **작업 가중치 정책이지 남은 시간 예측이나 실측 소요시간 비율이 아니다.** transition 시작에 Level과
현재 descriptor/실제 load profile로 적용 lane/weight를 고정한다. 각 lane의 실제 작업 목록/denominator는 해당 owner가 검증한
첫 snapshot에서 고정하고, 아직 모르는 lane은 0 credit과 작업 중 상태를 유지한다. 뒤 단계의 작업 수가
나왔다고 전체 100의 분모를 바꾸지 않는다.

| lane | Character Select | Valtan | Bern | Training | Map Editor / Lobby |
|---|---:|---:|---:|---:|---:|
| Level core / CSO | 10 | 10 | 15 | 15 | 99 |
| Map catalog / models / navigation | 35 | 25 | 60 | 50 | 0 |
| 선택 character bundle | 15 | 15 | 24 | 24 | 0 |
| Level 추가 presentation | 0 | 10 | 0 | 10 | 0 |
| Effect renderer core / CSO | 10 | 10 | 0 | 0 | 0 |
| 선택 Effect terminal commit | 29 | 29 | 0 | 0 | 0 |
| 실제 activation 완료 | 1 | 1 | 1 | 1 | 1 |

Map Editor/Lobby의 단순 경로는 현재 전체 `Execute_Load` 성공을 99-weight 작업의 완료로 사용한다. 이 경로가
세부 CSO별로 계측된 것처럼 표시하지 않는다. Training/Map Editor는 둘 다 하나의 `LEVEL::DEVELOPMENT`
descriptor를 사용한다. 실제 분기는 `Loader.cpp:700`의 Debug `CMapEditorWorkspaceService::Is_Requested()`다.
Loading 생성 시 이 요청을 읽어 `TRAINING`/`MAP_EDITOR` load profile snapshot을 한 번 고정하고, worker의
실제 분기와 weight 선택이 같은 snapshot을 소비하게 한다. Release는 기존 Training 경로만 사용한다.
새 Level enum이나 가짜 registry descriptor를 만들지 않는다. 알 수 없는 target/profile은 계획 오류다.

각 lane의 완료 의미는 다음과 같다.

- Level core: `Ready_MapAuthoringCore`, animated/shared character prototype의 실제 생성 성공 또는 검증된
  cache reuse를 관측한다. 함수 진입을 완료로 세지 않는다. core 작업은 map/character 준비 사이에 호출될 수
  있으므로 lane을 단순 현재 phase 번호로 덮어쓰지 않는다.
- Map: 해당 weight 안에서 catalog/scope 검증 10%, 필요한 map model prototype 성공 건수 85%, navigation
  성공/검증된 미사용 5%를 사용한다. `Add_Prototype` 성공 직후도 관측해 마지막 모델 `N/N`이 누락되지 않게 한다.
- Character: 기존 callback은 staged model 수이므로 bundle weight의 95%까지만 반영한다.
  `Ensure_Prototypes` transaction 성공이 나머지 5%다. 여러 class는 frozen bundle 목록의 누적 완료량을
  사용하고 class 변경마다 전체 비율을 0으로 만들지 않는다.
- 추가 presentation: Valtan은 boss, Esther 세 archetype, deploy bundle의 실제 terminal 결과를 관측한다.
  Training의 추가 prototype 경로도 실행되는 작업 목록만 고정한다. optional 실패 격리는 성공과 별도 상태다.
- Effect core: 아래 G09-4의 shader 6개 + buffer 2개 + solid texture 2개, 총 10개 실제 완료 단위를 쓴다.
- Effect target: 현재 selection/revision의 `prepared + 기존 정책상 허용된 failed/unavailable` terminal receipt를
  센다. `TARGET_STAGED`나 main ACK 이전에는 증가하지 않는다. background pending target은 분모에 넣지 않는다.
- Activation: prerequisite 수치가 아니라 실제 Level/profile/필요 identity commit 성공 receipt만 소비한다.

0개 작업은 divide-by-zero를 허용하는 숫자가 아니다. owner가 검증한 empty selection/not-applicable receipt가
있을 때만 그 lane을 완료/skip으로 settle한다. cache hit도 현재 revision/device/core identity 확인이 필요하다.
required Map Effect 실패를 optional failure로 바꾸거나 실패가 났다는 이유로 준비 성공을 만들지 않는다.

### G09-4. CSO 관측과 기존 CPP에 붙일 위치

현재 Engine Shader는 CSO path/byte 수/HRESULT/경과 시간의 완료 로그를 제공한다. CSO를 읽거나
`D3DX11CreateEffectFromMemory`가 실행 중일 때의 진짜 세부 백분율 callback은 없다. 로그 문자열을 파싱하거나
읽은 byte 수를 GPU 준비 완료율로 간주하지 않는다. Engine shader API를 바꾸지 않고 실제 caller에 관측을 붙인다.

```text
Loader의 기존 core/prototype 호출
  → logical shader/resource ID 시작 snapshot
  → 기존 생성/검증 함수
  → 성공 또는 원래 허용된 격리 결과를 lane ledger에 보고

Stage_LoadingProductTarget
  → Stage_VisualProgramTarget
  → Acquire_RendererCore
  → Build_RendererCore
  → shader/buffer/texture의 실제 완료 snapshot
  → 기존 TARGET_STAGED/main commit/TARGET_COMMIT_ACK
```

`Effect_DocumentRenderer.cpp:4890` 이후의 core 생성에 loading-scoped 관측 callback을 전달하고 일반 runtime
caller는 무관측 기본 경로를 쓴다. `core 0/10..10/10`, `shader 0/6..6/6`, current logical shader ID는 target ACK
counter와 별도 snapshot이다. callback은 작은 immutable 상태만 publish하며 D3D immediate context 접근이나
global cache lock 범위를 늘리지 않는다. cache가 이미 준비됐으면 exact identity 확인 후 cached-complete를
기록한다. core가 사용되지 않는 empty selection은 검증된 skip이다.

warm 진입에서는 selection이 비어 있지 않아도 `OwnedEffectAssetIds`가 0개일 수 있다. 현재
`Effect_PresentationService.cpp:2670`은 이 경우 즉시 CLOSE하므로 core callback을 기다리면 안 된다.
`selectedTotal`과 `ownedBatchTotal`은 별도의 frozen 분모다. owner가 현재 revision의 선택 target terminal
receipt를 다시 확인한 baseline ID 집합과 이번 worker의 exact ACK ID 집합을 중복 없이 합쳐 selected 완료량을
계산한다. baseline/worker count를 서로 다른 분모의 퍼센트로 직접 더하지 않는다.
empty owned batch는 `already-settled/no-owned-work` receipt로 target ledger를 seed하고 core lane을 검증된
cache/skip으로 settle한다. 이전 격리 실패는 cached-success로 이름을 바꾸지 않는다. optional core 실패로
실행하지 않은 나머지 단위는 dependency-skip/isolated-terminal로 표시하며 required 실패의 readiness는 여전히
실패다. 현재 receipt가 없거나 stale이면 기존 실패 정책을 따른다.

긴 단일 CSO/D3D 작업 동안은 이전 earned progress를 유지하고 current ID, elapsed와 별도 작업 중 표시만
갱신한다. 작업 자체가 끝나기 전에 퍼센트를 조금씩 만들어내지 않는다.

### G09-5. 표시 보간·수명·실제 activation

전체 earned 값은 lane별 fixed weight × 실제 완료 비율의 합이며 반복된 snapshot을 매 frame 더하지 않는다.
완료 ledger를 다시 계산하는 방식으로 중복 가산을 막는다. 같은 epoch에서 이미 알려진 denominator 변경,
완료량 역행, oversize count, invalid lane은 관측 오류로 보존하고 값은 변경하지 않는다. old epoch의 늦은
관측도 ledger를 변경하지 않으며, 제품 owner의 기존 structural failure 판정은 별도로 유지한다.

표시값의 단위는 0..1로 통일한다.

```text
dt = finite한 양수 delta만 허용하고 최대 0.1초로 제한
display += min(max(0, earned - display), 0.50 × dt)
```

최대 초당 50 percentage points, 한 update당 최대 5 points만 실제 완료량을 따라간다. 작업이 멈추면
display도 earned에서 멈춘다. 막대는 항상 왼쪽에 고정하고 기존 `fmod` 이동 fill/glow를 전체 bar에서 제거한다.
heartbeat/spinner는 별도 표시이며 퍼센트로 해석되지 않게 한다. 제품 bar/text는 기존 CUIObject/Draw_Text
경로를 사용하고 ImGui 진단 창을 제품 UI로 승격하지 않는다. 숫자는 내림 처리해 activation 전 반올림 100%도
금지한다.

```text
Loader SUCCEEDED
  + 현재 선택 Effect의 기존 readiness
  + result queue empty / worker epoch close
  + (복구 중이면) current session/full-state commit
→ 표시값과 무관하게 Request_Activation(loadEpoch)
→ MainApp: Create_Level → Activate_Profile → Change_Level
→ Bern이면 기존 pending identity commit 성공 확인
→ 같은 loadEpoch의 Complete_Activation: 100 기록
```

Loading 화면은 성공한 frame에 없어질 수 있다. 100% 화면을 억지로 한 frame 더 그리거나 보간 완료까지
전환을 지연하지 않는다. 그 대신 service의 final snapshot이 실제 100 완료를 보존한다. profile/Level 생성,
Change_Level/identity 실패는 기존 rollback/diagnostic을 수행하고 terminal snapshot에 마지막 수치와 원인을
남긴다. Loading 객체가 activation 중 파괴되어도 미완료로 오인해 같은 epoch를 cancel하지 않게
activation-requested ownership을 service로 넘긴다.

네트워크 복구 중에는 `접속 복구 대기`와 현재 준비율을 함께 표시한다. 같은 Loading의 로컬 준비가 완료되어도
network readiness 전에는 activation하지 않으며 99% 이하를 유지한다. grace 만료나 terminal validation 실패로
Loading 자체를 폐기할 때만 cancel한다. recovery retry가 resource progress를 다시 0으로 만들지 않는다.

현재 제품이 허용하지 않는 catalog/device identity 변경은 자동 REBASE가 아니라 원래 실패 경로로 끝난다.
기존 channel의 REBASE 단위 테스트는 그대로 유지하지만 G09가 새로운 자동 재시도 소비자를 추가하지 않는다.
향후 그 기능을 별도로 활성화할 때는 이미 수행한 credit과 새 revision readiness를 분리한 계획이 필요하다.

### G09-6. 구현 순서와 검증 gate

1. 현재 reset 경로를 기존 EffectRenderContractHarness의 production helper fixture로 먼저 고정한다.
2. `LoadingProgress.h`와 service 소유권/epoch 전달, Loader의 상태 문자열·counter 분리를 연결한다.
3. map/character 성공 경계와 core/CSO 관측을 붙이고 target ACK 완료 의미를 유지한다.
4. Loading의 전체 bar/별도 heartbeat/표시 보간을 교체하고 MainApp의 actual activation receipt를 연결한다.
5. Client 및 기존 Effect harness `.vcxproj`/`.filters`에 새 header/test를 등록하고 Debug/Release로 검증한다.

실행형 test의 실패 조건은 다음으로 고정한다.

| fixture | 기대 결과 |
|---|---|
| map 99/100 → opaque navigation → character 0/N → Effect 0/N | 전체 값·고정 fill이 역행하지 않음 |
| 긴 단일 CSO 호출 / main 작업 지연 | earned 이상 progress 0, worker heartbeat는 독립 진행 |
| core 단계 완료 / cache hit / empty selection | 검증된 작업 credit만 증가, 0 denominator는 typed skip 외 거부 |
| nonempty selection, owned batch 0개인 warm/all-isolated 진입 | current terminal receipt seed, core callback 없이 cache/skip settle, 실패를 성공으로 바꾸지 않음 |
| 부분 cache selection + 새 owned batch | frozen selected/owned 분모 구분, baseline/ACK ID 합집합으로 중복 완료 0건 |
| character N/N staged, bundle commit 실패 | bundle 완료 처리 금지, 기존 rollback 유지 |
| TARGET_STAGED, ACK 전/후, duplicate ACK | 전에는 terminal credit 0, exact commit 뒤 한 번만 증가 |
| cached / optional failed / unavailable / required Map Effect failed | 각 상태 구분, required 실패의 activation 0건 |
| target 전부 settled지만 queue pending 또는 worker open | 100과 activation 금지 |
| 준비 완료, display가 아직 낮음 | activation 요청은 지연하지 않음 |
| profile/Level/Change_Level/identity 실패 | 100 금지, terminal reason/마지막 progress 보존 |
| 실제 activation 성공 / 중복 completion / stale loadEpoch | 현재 epoch 성공 한 번만 100, 다른 관측은 상태 불변 |
| old jobEpoch/revision, denominator 변경/역행/초과 | 값 불변, 원래 owner의 실패 정책 유지 |
| 30/60/144 FPS, 긴/음수/NaN/무한대 delta | display 단조, earned 초과·NaN·반올림 100 없음 |
| cancel 뒤 늦은 callback / 새 명시 retry | old epoch 무시, 새 transaction에서만 0 초기화 |
| 같은 DEVELOPMENT에서 Map Editor/Training 선택 | frozen load profile과 worker 분기/weight 일치, Release에서 Debug Map Editor PASS 금지 |
| resource 완료 중 network detach/resume / grace 만료 | 같은 epoch 유지·network commit 뒤 activation, 만료 시 cancel |

fixture의 activation 실패 주입은 production ledger/gate 계약의 검증이지 실제 화면 전환 실행 증거가 아니다.
Debug/Release Client build와 기존 shader closure/Effect harness를 별도로 통과시키고, Character Select의 cold/warm
진입과 실제 바의 부드러움은 사용자가 직접 관찰한다. 자동으로 Client/UI를 켜거나 캡처하지 않는다.

## 5. 실패 분류

| 관측 | 자동 처리 | 최종 처리 |
|---|---|---|
| LAN discovery timeout | PR A는 중단, PR B는 configured trusted routed만 진행 | 후보 소진 시 Lobby에 원인 표시 |
| DNS resolve 실패 | PR B AUTO routed면 다음 trusted 후보 | DIRECT이면 즉시 terminal |
| PR A initial connect 또는 enter byte commit 전 실패 | 5초 entry budget에서 candidate/backoff retry | budget 소진 시 Lobby terminal |
| PR A enter byte commit 뒤 EOF/approval timeout | 자동 retry 금지 | `AMBIGUOUS_ENTRY` terminal |
| PR B ticket 미수신/partial ticket 뒤 transport loss | 같은 `entryAttemptId` ENTER로 pending admission/ticket 재사용 | fixed 5초 만료면 `ENTRY_ATTEMPT_EXPIRED` |
| PR B ticket 수신 뒤 confirmation 전 loss | exact pending token resume로 같은 admission proof | original 5초 만료면 terminal |
| PR B pending/commit-race cancel | credential 보존, reconnect cancel/ACK까지 새 attempt 차단 | 같은 boot의 expired/revoked/not-found 또는 새 boot도 cleanup 증명 |
| ACTIVE의 Server frame 5초 무응답 | `DEGRADED`, 화면/identity 유지 | 8초에 transient close + resume |
| ACTIVE transport loss | transient transport close | Server 15초 grace 안에 same-session resume |
| nonblocking would-block/readiness timeout | 같은 stream의 확인된 offset에서 bounded 대기 | 3초 no-progress면 transport loss로 분류 |
| 실제 blocking send timeout / reset / aborted | 해당 stream 폐기, 같은 frame의 추측 재전송 금지 | valid lease만 resume |
| `ROOM_FULL` | retry 금지 | Lobby에서 typed rejection 표시 |
| AUTO discovery TCP version mismatch | 해당 후보 금지, remaining trusted candidate 계속 | exact version 없으면 Server version 포함 terminal |
| DIRECT/routed pre-hello EOF/reset | retry budget 뒤 중단 | `POSSIBLE_PROTOCOL_VERSION_MISMATCH`, old version 단정 금지 |
| protocol/version/decode violation | raw token 폐기 + same-boot cleanup fence | immediate terminal/Lobby, fresh entry는 fence 뒤 |
| invalid/expired/replayed token | retry 금지 | terminal/Lobby |
| same-token half-open `ALREADY_ATTACHED` | 기존 active transport non-takeover + bounded retryAfter | Server 8초 detach 뒤 local budget 안에서 retry |
| unrelated duplicate resume | 기존 active transport 유지 | 새 attempt terminal typed reject |
| current-owner control rate/phase violation | terminal reject + generation-fenced lease revoke | frame 유실 후 resume도 `REVOKED`, loop 없음 |
| stale/unbound control violation | offending socket만 terminal close | current lease/player/binding 무변경 |
| Server boot ID 변경 | old token 무효 | terminal/Lobby, fresh enter는 사용자 명령에서만 |
| local clone/load/full-resync validation 실패 | old frozen presentation 부분 변경 금지 + valid channel leave/ACK | cleanup 뒤 terminal reset/Lobby |
| explicit cancel/quit/leave | leave/ACK bounded transaction | ACK되면 release 확인, ACK 유실은 already-released/grace unknown + cleanup fence |
| AP/client isolation | discovery/direct 실패 | trusted routed endpoint 없으면 명확히 중단 |
| Public hotspot / unicast 응답 정책 차단 | 정확한 adapter/profile/policy 진단 | 사용자 신뢰 설정 또는 DIRECT/routed 선택, 전역 방화벽 해제 없음 |
| discovery 후보/adapter/datagram 상한 초과 | 잘린 목록에서 자동 선택 금지 | `DISCOVERY_LIMIT_EXCEEDED` |
| Loading 단계 변경 / CSO opaque 대기 | 전체 ledger 유지, 별도 작업 중 표시 | phase별 0%/100%로 전체 막대 교체 금지 |
| Loading 준비 완료, network 복구 중 | 같은 loadEpoch로 99% 이하 유지 | full-state/session commit 뒤에만 activation |
| CSO/content/required resource/activation 실패 | 네트워크 재시도와 분리, 원래 rollback | 100 금지, terminal reason 보존 |

## 6. 구현 뒤 실행할 자동 검증

아래는 **구현 이후의 명령과 기대 조건**이다. 이번 계획 전용 턴에서 실행하거나 PASS로 기록하지 않는다.
새 스크립트/옵션은 아직 존재하지 않으므로 현재 사용법으로 안내하지 않는다.

### 6.1 focused protocol/connection 검증

```powershell
powershell -ExecutionPolicy Bypass -File `
    Tools/Network/Publish-TeamLanEndpoint.ps1 -Check

powershell -ExecutionPolicy Bypass -File `
    Tools/Network/Test-TeamLanEndpointContract.ps1

msbuild Shared/Default/Shared.vcxproj /m /p:Configuration=Debug /p:Platform=x64
msbuild Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj /m /p:Configuration=Debug /p:Platform=x64
Tools/NetworkProtocolHarness/Bin/Debug/NetworkProtocolHarness.exe

powershell -ExecutionPolicy Bypass -File `
    Tools/Network/Run-NetworkConnectionContractHarness.ps1 -Configuration Debug

powershell -ExecutionPolicy Bypass -File `
    Tools/Network/Run-LanDiscoverySmoke.ps1 -Configuration Debug
```

같은 명령을 Release에서도 실행한다. runner는 fresh project build를 먼저 수행하고 stale local exe를 실행하지
않는다.

### 6.2 Server/4인/private arena 검증

```powershell
Server/Bin/Debug/Server.exe --contract-test

powershell -ExecutionPolicy Bypass -File `
    Tools/Network/Run-ValtanFourPlayerHarness.ps1 -Configuration Debug

powershell -ExecutionPolicy Bypass -File `
    Tools/Network/Run-CharacterSelectIsolationHarness.ps1 -Configuration Debug
```

Server contract test는 최소 다음을 자동 판정한다.

- ticket CSPRNG seam과 digest comparison
- 128-bit entry attempt의 0-byte ticket retry, conflict/expiry/cancel과 duplicate-player 0건
- current/stale transport generation
- duplicate resume one-winner와 active connection non-takeover
- active/pending token rotation의 result/commit ACK 유실
- exact 15초 grace와 non-extendable deadline
- gameplay queue hard-cap에서도 ticket ACK/resume commit/lifecycle cleanup exactly once
- shutdown late-close producer barrier와 empty-lane 뒤 lease/player 0개
- detached player의 4인 capacity reservation
- position/HP/action/cooldown/inventory/sequence 보존
- expiry/revoke/server restart/world transfer race
- Character Select private simulation 보존/retirement
- malformed/unknown packet에서 ticket revoke와 partial room commit 없음

### 6.3 정본 build/regression

```powershell
powershell -ExecutionPolicy Bypass -File `
    Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug

powershell -ExecutionPolicy Bypass -File `
    Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release

powershell -ExecutionPolicy Bypass -File `
    Tools/Network/Sync-TeamLanEndpoint.ps1

git diff --check
```

full regression은 Engine, UpdateLib, Shared, NetworkProtocolHarness, Server contract, Client, 새 focused harness,
Valtan 4인과 Character Select isolation을 구성별 fresh output에서 build/run해야 한다. JSON과 project/filter XML
parse, 단계에 맞는 TeamLanEndpoint schema v2/v3, tracked generated header direct-build `-Check`, AUTO routed
multi-boot ambiguity, firewall legacy ungrouped ProfileAny migration, CIDR A -> B -> none/Server -> Client reconciliation,
routed Server-endpoint/Client-source CIDR 분리, NLM profile과 Private/Domain scope도 자동 gate에 포함한다. focused harness는 character clone/target-Level swap
뒤 move/action N+1 보존과 held-RMB release gate도 판정한다.

### 6.4 로딩률/CSO 회귀

정본 build 또는 Engine → UpdateLib → Client → Effect harness 구성 일치 build가 끝난 뒤 기존 runner를 사용한다.
이 runner 자체는 build를 수행하지 않으므로 stale exe 실행을 검증으로 인정하지 않는다.

```powershell
powershell -ExecutionPolicy Bypass -File `
    Tools/Build/Test-CompiledShaderClosure.ps1 -Configuration Debug

powershell -ExecutionPolicy Bypass -File `
    Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1 -Configuration Debug
```

Release도 같은 순서로 실행한다. G09의 production progress helper 결과, 기존 load-job ACK/rollback, worker
device-only 경계, queue/epoch 종료와 compiled shader closure를 함께 통과해야 한다. 문자열 source assertion만
추가하고 진행률 회귀를 닫았다고 하지 않는다. Effect 설정을 실제로 변경하는 통합 작업이 포함되면 기존
`Validate-EffectSources.ps1`와 `Sync-EffectDataProject.ps1 -Check`도 정본 full gate에서 확인한다.

## 7. 구현 뒤 사용자 수동 검증

에이전트는 Client/UI를 직접 실행하거나 화면을 대신 판정하지 않는다. 자동 검증이 끝난 뒤 사용자가 다음을
직접 수행하고 관찰 결과를 RESULT에 기록한다.

### 7.1 같은 hotspot/LAN

1. Server PC에서 최초 한 번 `Sync-TeamLanEndpoint.ps1 -Role Server -UseAutoDiscovery`로 명시 전환/local role
   marker를 만들고, 이후 기본 sync
   실행에서 `server-host`, Private/Domain profile, TCP 7777/UDP 7778 LocalSubnet ready를 확인한다.
2. Server + Client profile을 시작한다.
3. 다른 PC도 최초 `-Role Client -UseAutoDiscovery` 전환 뒤 host override 없이 기본 sync와 Client만 시작한다.
4. Lobby 상태가 discovery source IPv4를 골라 접속하고 정상 4인 입장하는지 본다.
5. Server PC의 Wi-Fi IP가 DHCP로 바뀐 뒤 endpoint literal을 다시 편집하지 않아도 discovery로 접속하는지 본다.
6. 두 Wi-Fi adapter가 켜진 PC에서도 wrong interface/duplicate Server로 가지 않는지 본다.
7. hotspot이 Public이면 무응답처럼 숨기지 않고 신뢰 설정 안내가 나오는지 확인한다. 다른 PC의 로컬 IP를
   Server IP와 같게 바꾸지 않는다. 모두 같은 Server를 선택하는 것이지 IP를 동일하게 할당하는 것이 아니다.

### 7.2 짧은 Wi-Fi 단절

1. 네 명이 같은 world에 입장한다.
2. 한 Client의 Wi-Fi를 2~8초 끊었다가 복구한다.
3. 실제 loss가 감지되면 Lobby로 즉시 가지 않고 frozen/recovering 상태를 표시하는지 본다. 5초 미만에
   valid packet이 돌아와 같은 socket을 유지한 경우는 정상 ACTIVE 지속이며 불필요한 reconnect를 요구하지 않는다.
4. resume 뒤 같은 PlayerId/NetEntityId이고 위치/HP/action이 fresh-spawn reset되지 않고 disconnect 동안 진행된
   current authoritative Server state와 일치하는지 본다.
5. 다른 세 Client에서 그 player의 duplicate despawn/spawn이 보이지 않는지 본다.
6. 별도 실행에서 abrupt transport close는 detach 뒤 15초, FIN/RST가 유실되는 Wi-Fi half-open은 최대
   8초 detection + 15초 grace를 넘기도록 24초 이상 끊고 정확히 한 번 Lobby 복귀와 slot release가 되는지 본다.

### 7.3 다른 Wi-Fi/routed network

1. 팀이 승인한 VPN/overlay route, FQDN, Server-endpoint CIDR과 Client-source CIDR을 분리해 준비하고 Server의 실제 routed interface가 NLM
   Private/Domain인지 sync 진단으로 확인한다. Public/unknown profile에서는 exact CIDR이어도 지원 ready가 아니다.
2. 다른 Wi-Fi의 Client에서 LAN discovery가 실패한 뒤 trusted routed endpoint로 접속하는지 본다.
3. 일반 public Wi-Fi에서 route/allowlist가 없을 때 public firewall을 자동으로 열거나 무한 대기하지 않는지 본다.
4. route가 바뀌어 source IP가 달라진 상태에서도 grace 안의 valid token resume이 되는지 본다.

### 7.4 Character Select와 전체 로딩률

1. 같은 구성/commit의 Server와 Client를 사용자가 직접 시작하고 Lobby → Character Select로 진입한다.
2. 새 process의 첫 진입(cold core)에서 CSO/core 단계와 전체 퍼센트를 함께 관찰한다. CSO 하나가 오래 걸리면
   수치가 유지되고 작업 중 표시만 진행되는 것이 정상이다.
3. map → character → Effect 전환에서 전체 막대가 0으로 되돌아가지 않는지, 완료분이 한꺼번에 들어와도
   표시만 부드럽게 따라가는지 확인한다.
4. 실제 activation 전 100%가 찍히지 않는지 확인한다. 성공 즉시 화면이 바뀌어 100% frame이 보이지 않아도
   service completion log가 정확하면 화면 지연을 추가하지 않는다.
5. Lobby로 돌아와 같은 process/revision의 재진입(warm cache), Valtan 및 기존 Bern/Training 진입을 확인한다.
6. 별도 단절 시험에서 loading epoch가 reset되지 않고, terminal failure/명시 새 retry만 새 0%인지 확인한다.

실제 CSO 파일을 임의 삭제·변조하는 실사용 시험은 요구하지 않는다. 누락/손상과 실패 rollback은 isolated
fixture에서 검증하고 사용자 시험은 정상 화면 흐름에 집중한다.

사용자의 서면 관찰 전에는 LAN auto discovery, Wi-Fi resume, 4인 visual 상태, loading 표시 또는 cross-network 접속을 수동
PASS로 기록하지 않는다.

## 8. 구현 순서와 admission gate

1. **현재는 계획서 검토까지만 수행한다.** 사용자 구현 요청 전에는 코드·설정·build·PR 작업을 시작하지 않는다.
2. 구현 승인 뒤 PR #247과 다른 통합 대상의 실제 commit/PLAN/RESULT를 확인하고 최신 main에서 전용
   `codex/` 브랜치를 만든다. P는 protocol 비의존이라 먼저 독립 진행할 수 있고 A/B 통합은 PR #247 기준을
   확정한 뒤 시작한다. 다른 작업의 dirty checkout을 자동 stage/merge하지 않는다.
3. G09를 독립 P 단위로 먼저 구현·검증할 수 있다. G00에서는 이미 반영된 PR #246 진단/priority cleanup을
   보존하고 새 generation/parser/close 수명만 확장한다.
4. G01/G02/G03으로 UDP codec, 2초 탐색, 명시 설정 마이그레이션과 async initial connect를 닫는다.
   A의 사용자 두 PC smoke와 isolated loopback 회귀를 별도로 받는다.
5. G04 시작에 §1.6의 통합 wire version을 확정하고 logical lease, stable identity, capacity와 data-revision
   participant 경계를 Server contract test로 닫는다.
6. G05에서 Client recovery/full-resync를 연결하고 Level별 즉시 disconnect 소비자를 한 번에 교체한다.
   P의 loadEpoch/activation ACK와 B의 network gate를 같은 MainApp/transition 경계에서 결합한다.
7. G06의 4인/private arena/부분 전송/heartbeat/fault test와 G07 routed allowlist·public exposure 금지를 검증한다.
8. Debug/Release 정본 regression과 domain publisher를 실행하고 실패는 baseline/신규 원인으로 분리한다.
   한 구성만 성공하거나 기존 실패를 누락한 상태를 전체 PASS로 기록하지 않는다.
9. 실제 구현 상태만 RESULT와 네트워크 가이드/public 문서에 반영한다. 사용자 LAN/짧은 단절/grace 초과/
   loading cold·warm/routed 관찰을 자동 결과와 분리한다.
10. 이후 사용자가 승인한 통합 PR에서는 P/A/B와 다른 기능을 검증 가능한 commit으로 구분하고 최종 diff,
    필수 check와 수동 판정을 확인한 뒤 merge한다. 이번 계획 작성은 그 PR 생성·merge를 실행하지 않는다.

각 gate가 실패하면 다음 G로 넘어가지 않는다. 특히 discovery가 동작한다는 이유로 session resume까지 완료로
처리하지 않고, reconnect가 된다는 이유로 full authoritative resync와 4인 capacity 보존을 생략하지 않는다.
