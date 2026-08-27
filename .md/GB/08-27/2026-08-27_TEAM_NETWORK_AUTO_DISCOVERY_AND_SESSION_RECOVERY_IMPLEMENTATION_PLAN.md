# 팀 네트워크 자동 탐색·세션 복구 확장 구현 계획

기준일: 2026-08-27

계획 기준 브랜치: `codex/team-endpoint-10-207-18-103`

계획 기준 HEAD: `d77d7e021edefc435119b23f9aab53315e3f870a`

상태: 계획 확정, 구현 전

구현은 `10.207.18.103` 보정 PR #245가 `main`에 병합된 뒤
`codex/network-auto-discovery-session-recovery` 전용 브랜치에서 시작한다. 현재 endpoint 보정 PR에는
이 계획의 대규모 protocol/transport 변경을 섞지 않는다.

## 0. 결론

Server의 `0.0.0.0:7777`은 그 PC의 모든 로컬 IPv4 interface에서 연결을 받겠다는 bind 값이다.
Client가 접속할 수 있는 원격 주소는 아니므로 `Client 0.0.0.0` 계약은 만들지 않는다.

사용자의 번거로움을 없애는 지원 경계는 다음 세 가지로 고정한다.

1. 같은 trusted hotspot/LAN의 Client는 IP를 입력하지 않는다. Client가 UDP directed broadcast로 정본 Server를
   찾고, 응답 datagram의 source IPv4와 광고된 TCP port로 접속한다.
2. 서로 다른 Wi-Fi, NAT 또는 인터넷에서는 LAN broadcast가 전달되지 않는다. 이 경우 Client는
   cancellable `GetAddrInfoExW` 기반 IPv4/FQDN endpoint를 지원하고, 팀이 제공한 VPN/overlay 또는 명시적으로 routing된
   trusted endpoint를 사용한다.
3. 짧은 Wi-Fi 단절과 지연은 즉시 Lobby fallback으로 바꾸지 않는다. Server가 transport detach 뒤 15초 logical
   session lease를 유지하고 same-world Client가 같은 `PlayerId/NetEntityId`와 authoritative state로 resume한다. grace 만료, protocol
   위반, Server 재시작, 명시적 퇴장처럼 복구할 수 없는 경우에만 기존 Lobby 복귀를 실행한다.

현재 protocol에는 인증과 TLS가 없다. 따라서 router port forwarding, UPnP, Windows Firewall
`RemoteAddress Any`로 임의 인터넷에 Server를 공개하는 것은 이번 계획에서 금지한다. Cross-network는
trusted routed network의 DNS/IPv4와 정확한 CIDR allowlist까지만 지원한다. 불특정 인터넷에서 별도 설치
없이 접속시키는 authenticated rendezvous/relay는 별도 인프라·보안 계획이다.

이 계획은 4인 room 정원을 늘리지 않는다. 연결이 잠시 끊긴 player도 15초 grace 동안 4인 정원을 계속
점유하며, valid resume만 자기 자리를 되찾는다. grace 중 다섯 번째 신규 입장은 계속 `ROOM_FULL`이다.

## 1. 현재 실측과 직접 원인

### 1.1 현재 정상 기준

- 공유 Server PC는 `10.207.18.103/24`를 소유하고 Server는 `0.0.0.0:7777`에 listen한다.
- 사용자가 Server Release와 Client Release를 직접 실행한 뒤
  `10.207.18.103:<ephemeral> -> 10.207.18.103:7777`의 TCP `Established`를 확인했다.
- 같은 LAN의 다른 Client도 현재는 모두 concrete endpoint `10.207.18.103:7777`을 사용해야 한다.
- `Tools/Network/TeamLanEndpoint.json`, tracked debugger environment와 Git 제외 `.vcxproj.user`가 모두
  이 주소를 주입한다.
- `10.207.18.151`과 `10.207.18.103`처럼 hotspot DHCP 주소가 바뀌는 것은 정상이다. 문제는 주소가
  다르다는 사실이 아니라 Client 설정이 이전 Server 주소를 계속 들고 있는 것이다.

### 1.2 Client의 현재 연결 흐름

```text
Lobby Begin_NetworkEntry
-> Resolve_ServerHost
   -> LOSTARK_SERVER_HOST
   -> Debug loopback opt-in
   -> compiled 10.207.18.103
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
`Client.vcxproj`가 `LOSTARK_SERVER_HOST=10.207.18.103`을 항상 넣으므로, 명시 host가 자동 탐색보다 우선하는
정상적인 정책만 추가하면 Visual Studio 기본 실행에서는 discovery가 영원히 실행되지 않는다.

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
- `Send_All` 실패는 error만 기록하고 연결 상태를 일관되게 전환하지 않는다.
- `Close_ServerConnection()` 하나가 transient transport close와 terminal session forget을 모두 수행한다.

### 1.4 구현 전에 먼저 닫을 Server lifecycle 결함

`CGameRoom::Enqueue`는 hard cap 1024에서 `LEAVE`도 거부할 수 있다. 그러나
`CServerApp::On_SessionClosed`는 `LEAVE` enqueue 결과를 확인하지 않고 gameplay binding을 먼저 지운다.
큐 포화 시 room에는 player가 남고 ServerApp에는 binding이 없는 고아 상태가 생길 수 있다.

resume은 disconnect player를 더 오래 보존하므로 이 결함을 그대로 두고 discovery부터 외부에 열지 않는다.
transport close, detach와 rebind는 일반 gameplay queue와 분리된 ordered lifecycle lane으로 전달하고, expiry는
room thread가 같은 lifecycle phase에서 직접 평가해 정확히 한 번 처리한다.

## 2. 완료 조건과 제외 범위

### 2.1 자동 탐색 완료 조건

- 기본 Visual Studio 실행은 concrete `LOSTARK_SERVER_HOST`를 주입하지 않고 `AUTO` mode를 사용한다.
- 같은 broadcast domain의 Client는 Server IP를 입력하지 않고 500ms bounded discovery 뒤 접속한다.
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
  Lobby 상태에 보존한다. `.103`은 사용자가 DIRECT로 명시할 수 있는 rollback 정보일 뿐 AUTO 후보가 아니다.

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
  stale CIDR와 이전 role rule을 제거한다. `-Role Client`는 repo-owned inbound Server rule을 전부 제거한다.
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
- 수동 DIRECT rollback 안내용 `rollbackHost=10.207.18.103`
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
AUTO만 선언하며 sync script는 Git 제외 `.vcxproj.user`의 이전 `LOSTARK_SERVER_HOST`와 위 network key만
정규화하고 다른 사용자 environment variable은 보존한다. parent process가 명시한 host와 Debug loopback
opt-in은 기존 우선순위를 유지한다. harness는 unset/AUTO/DIRECT/conflict/malformed env matrix를 검증한다.

effective expected TCP port는 valid `LOSTARK_SERVER_PORT`, 없으면 generated port다. DIRECT와 AUTO routed는 이
port로 connect한다. AUTO LAN은 discovery response의 advertised `tcpPort`를 쓰되 production에서는 effective
expected port와 정확히 같아야 하며 다르면 `DISCOVERY_PORT_MISMATCH`로 후보를 버린다.
`LOSTARK_DISCOVERY_PORT`는 query destination만 바꾸고 advertised TCP port를 override하지 않는다. loopback process
smoke는 test-only env/Server argument로 양쪽 expected TCP port를 같은 dynamic 값에 맞춘다. focused harness는
advertised/expected mismatch, malformed 0/overflow와 matching dynamic fixture를 검증한다.

`Sync-TeamLanEndpoint.ps1`은 Client debugger 환경에서 `LOSTARK_SERVER_HOST`를 제거하고 auto mode와 port
override만 정규화한다. stable team ID는 environment override가 아니라 generated Shared header가 정본이다.
사용자가 shell/launch에서 명시한 concrete host는 override로 유지한다. `rollbackHost`는 probe/안내에만 쓰고
자동 접속이나 machine role 판정에 쓰지 않는다.

Server machine role은 Git 제외 `Tools/Network/TeamLanRole.local.json`의 stable local marker가 소유한다.
최초 Server PC에서 `Sync-TeamLanEndpoint.ps1 -Role Server`를 한 번 실행하면 marker의 team ID/role을 검증해
저장하고, 이후 Auto 실행은 DHCP 주소가 바뀌어도 `server-host`를 유지한다. `-Role Client`는 해당 PC를 Client로
명시 전환한다. Server role admission은 이전 `.103` 소유가 아니라 active non-loopback IPv4 존재와
`0.0.0.0` bind 가능성을 검사한다. marker가 없으면 안전하게 Client로 판정한다.

AUTO discovery는 Windows `Private` 또는 `DomainAuthenticated` network profile에 연결된 adapter에서만
실행한다. Public profile은 sync가 명확한 진단을 내고 Client가 discovery를 건너뛰며 profile을 자동 변경하지
않는다. `rollbackHost`는 사용자가 DIRECT override로 명시하지 않는 한 현재 subnet에 있더라도 시도하지 않는다.

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
Private/Domain IPv4 adapter마다 adapter 주소에 UDP socket을 bind하고 subnet mask로 directed broadcast를
계산해 `:7778`로 두 번 전송한다. 두 round는 0ms와 200ms에 보내고 전체 수집 window는 500ms로 고정한다.
response count와 datagram byte 수는 bounded하고 magic/schema/protocol/team ID/nonce를 모두 검증한다.

discovery schema v1은 PR A(v39)와 PR B(v40)에서 유지한다. magic/schema/team/nonce가 valid하지만 query의
TCP version이 Server와 다르면 responder는 silent drop 대신 `PROTOCOL_MISMATCH`와 자기 TCP version을 답하고,
Client는 TCP 후보로 쓰지 않는다. 이 typed mismatch는 discovery schema를 아는 PR A 이후 build 사이에서만
보장한다.

접속 IPv4는 response payload에 넣지 않고 `recvfrom` source address를 사용한다. 동일 boot ID가 여러 adapter
주소로 응답하면 interface metric, 응답 RTT와 numeric IPv4의 deterministic order로 후보를 만든다. 다른
boot ID가 같은 team ID로 동시에 응답하면 misconfiguration으로 보고 접속하지 않는다.

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
cap 도달 시 fresh entry만 v40 `SERVER_BUSY`로 거부하고 기존 valid resume은 새 lease를 만들지 않으므로
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

### 3.5 protocol v40 session resume

Shared TCP protocol은 append-only packet type을 추가하고 `NETWORK_PROTOCOL_VERSION`을 39에서 40으로 올린다.

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

v40 Client의 새 TCP transport는 enter/resume token보다 먼저 nonce를 가진 `C2S_SERVER_HELLO`를 보낸다.
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

v40 Client는 한 번의 사용자 entry intent를 시작할 때 CSPRNG 128-bit `entryAttemptId`를 만들고 reconnect 동안
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

detach에서는 held move/aim/input ownership만 release하고 신규 gameplay command를 막는다. 이미 시작한
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

resync용 `S2C_WORLD_SNAPSHOT`은 authoritative roster/state/cooldown/combat-object 상태만 담고 transient
`DamageEvents`와 `BossCombatEvents`는 비운다. disconnect 직전 이미 본 hit/effect를 다시 재생하지 않으며,
`FULL_RESYNC_END` 뒤 ordinary snapshot부터 새 transient event를 소비한다.

room thread는 모든 frame을 먼저 encode/validate하고 capacity 512 frames, 8MiB인 dedicated
`Queue_ReliableBatch`에 `FULL_RESYNC_BEGIN -> full-state closure -> FULL_RESYNC_END`를 한 batch object로
preflight·enqueue한다. normal 128-frame/512KiB queue에 BEGIN부터 frame별로 넣지 않는다. frame/byte bound,
하나뿐인 active resync batch와 exact generation을 한 lock에서 admission하고, 하나라도 초과하면 BEGIN을 전혀
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

- 유효한 application frame은 모두 liveness를 갱신한다.
- 1초 동안 보낼 application frame이 없을 때만 Client가 heartbeat를 보낸다.
- Server는 heartbeat 또는 gameplay command를 5초 받지 못하면 transport를 `DEGRADED`로 기록한다.
- Client도 Server frame을 5초 받지 못하면 UI 상태만 `DEGRADED`로 바꾸고 presentation을 유지한다.
- 양쪽 모두 8초 전에 current-generation valid frame을 받으면 같은 socket에서 즉시 ACTIVE로 돌아가며 connect,
  detach나 token rotation을 시작하지 않는다.
- Client는 Server frame 8초 무응답이면 해당 TCP transport를 transient close하고 resume을 시작한다.
- Server도 Client application frame 8초 무응답이면 current socket을 shutdown/close하고 lifecycle lane에 exact
  generation DETACH를 넣는다. Client FIN/RST가 유실된 단방향 half-open도 이 경계에서 ACTIVE를 GRACE로 바꾼다.
- Client/Server send의 timeout/would-block은 한 번의 250ms 실패로 close하지 않고 동일 socket에서 남은 byte
  offset을 최대 3초 bounded progress window 동안 재시도한다.
- reconnect backoff는 `0ms, 250ms, 500ms, 1000ms, 2000ms` 후 2000ms cap이며 각 시도에 최대 100ms
  injected jitter를 더한다.
- ticket은 15초 duration만 전달하며 Client가 Server의 absolute clock을 추측하지 않는다. Server deadline은
  room-thread `detachedAt + 15,000ms`이고 Client는 자기 8초 loss 감지/transport close부터 최대 15초 local budget을
  사용하되
  `S2C_SESSION_RESUME_RESULT`의 authoritative `remainingGraceMs`가 더 짧으면 즉시 줄인다. Server reject/expiry가
  최종 정답이고 retry는 grace를 연장하지 않는다.

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
이 fatal path를 사용하지 않는다. 일반 queue cap 1024가 차도 detach cleanup은 유실되지 않으며, silent orphan
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

## 4. 수정 파일과 역할

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/.gitignore` | machine-local `Tools/Network/TeamLanRole.local.json` 추적 금지 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Object_Manager.h` | existing layer ensure와 exact-identity batch replacement contract |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Object_Manager.cpp` | prevalidated multi-layer pointer swap와 rollback receipt |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/GameInstance.h` | Client replication이 소비하는 layer transaction facade |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/GameInstance.cpp` | Object Manager batch transaction forwarding |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketType.h` | protocol v40, ticket/resume/leave/heartbeat/full-resync packet type |
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
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Private/ServerApp.cpp` | accept cap, first-packet routing, detach/resume/expiry 직렬화와 shutdown |
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
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_Loading.cpp` | admission/activation 중 recovery gate와 terminal rollback |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/LevelTransitionService.h` | world-transfer one-shot target descriptor/resync/sequence-state handoff |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/LevelTransitionService.cpp` | world transfer와 transport recovery ordering |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp` | network helper 초기화/종료, input gate와 session leave ordering |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/NetworkPlayerCommandSink.cpp` | recovery 중 command reject, resume 뒤 새 edge만 허용 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/PlayerController.h` | recovery RMB release latch와 logical-session command sequence 보존 API |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/PlayerController.cpp` | held RMB 재전송 억제, resume character rebind와 N+1 move/action sequence 유지 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | 새 파일, `Iphlpapi.lib`/`Bcrypt.lib`/`Ole32.lib`, NLM, auto debugger env와 endpoint `-Check` |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | 새 파일을 물리 Network filter에 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Network/TeamLanEndpoint.json` | PR A v2 discovery/role, PR B v3 routed endpoint/source CIDR 분리 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/Network/Publish-TeamLanEndpoint.ps1` | schema v2/v3 validate, generated Shared default publish/check |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Network/Sync-TeamLanEndpoint.ps1` | auto config, firewall expected-set, routed endpoint/source CIDR와 role/probe 출력 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/Network/Test-TeamLanEndpointContract.ps1` | config/env/role와 firewall snapshot의 plan-only expected-set/action regression |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` | v40 TCP codec와 UDP discovery negative/round-trip 검증 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/NetworkConnectionContractHarness/Private/NetworkConnectionContractHarness.cpp` | endpoint policy, async transport generation, liveness/retry/resync transaction harness |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/NetworkConnectionContractHarness/Default/NetworkConnectionContractHarness.vcxproj` | production sources, Shared/Engine refs와 Win32 native link closure를 쓰는 Debug/Release harness build |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/NetworkConnectionContractHarness/Default/NetworkConnectionContractHarness.vcxproj.filters` | harness source/filter 등록 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/Network/Run-NetworkConnectionContractHarness.ps1` | Engine build -> UpdateLib -> Shared/harness -> 구성 일치 DLL 배포 -> fresh 실행/timeout |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/Network/Run-LanDiscoverySmoke.ps1` | 실제 Server responder를 loopback의 동적 UDP/TCP port에서 query하는 process smoke |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/ValtanFourPlayerHarness/Private/ValtanFourPlayerHarness.cpp` | grace 중 4/4, same-ID resume, expiry 뒤 replacement 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/CharacterSelectIsolationHarness/Private/CharacterSelectIsolationHarness.cpp` | private arena resume 보존과 expiry retirement 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Network/Run-ValtanFourPlayerHarness.ps1` | grace/resume fault 단계와 bounded timeout |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Network/Run-CharacterSelectIsolationHarness.ps1` | ticket/resume/expiry scenario와 bounded timeout |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1` | 새 harness build/run과 Debug/Release network admission gate |
| 수정 | `C:/Users/user/Desktop/LostArk/Framework.sln` | 새 focused harness project/config 등록 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/AGENTS.md` | static endpoint에서 auto discovery/routed endpoint/grace public 경계로 교체 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/CLAUDE.md` | 실제 설정, 실행, 오류 상태와 build/run 사용법 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/.md/TEAM/README.md` | 팀 네트워크 정본 문서 순서와 sync 시작 절차 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | LAN auto, routed cross-network, disconnect/resume 운영·실패 진단 |
| 구현 후 추가 | `C:/Users/user/Desktop/LostArk/.md/GB/08-27/2026-08-27_TEAM_NETWORK_AUTO_DISCOVERY_AND_SESSION_RECOVERY_IMPLEMENTATION_RESULT.md` | 실제 diff, 자동 검증, 두 PC/사용자 수동 검증과 미완료 보안 경계 |

새 C++ 파일은 모두 UTF-8 BOM 없이 작성한다. 기존 C++는 현재 인코딩을 유지한다. 새 production 파일은
각 `Shared/Server/Client.vcxproj`와 `.filters`에, 새 harness는 project/filters, `Framework.sln`과
`Invoke-BuildAndRegression.ps1`에 같은 변경 단위로 등록한다. orphan source나 로컬에 남은 stale exe를
검증 근거로 사용하지 않는다.

## G00. baseline과 lifecycle 무결성

### G00-1. 현재 동작을 failure reason으로 고정

`CClientTransport`를 추출하기 전에 기존 connect/recv/send/close에서 발생하는 WSA/EOF/protocol 원인을
typed transport result로 보존한다. graceful EOF가 error 0으로 사라지거나 send 실패가 receive worker의
별도 실패를 기다리지 않게 한다.

현재 `NetworkManager`의 stream parser는 receive worker local로 옮긴다. 기존 `Fail_Protocol`처럼 worker join
전에 shared parser를 reset할 수 있는 경로를 제거한다. 모든 socket close는 cancel flag, shutdown,
closesocket, bounded join, result queue drain 순서 하나를 사용한다.

### G00-2. non-droppable lifecycle 선행 수정

Server의 transport close를 general room queue의 `LEAVE`와 binding erase로 바로 바꾸지 않고 lifecycle lane에
기록한다. 아직 resume을 활성화하기 전 compatibility policy는 lifecycle drain이 즉시 expiry/Leave를
호출하게 하여 외부 동작은 유지한다.

contract test에서 room queue 1024 포화, 동시에 여러 close, duplicate close와 stale generation을 만들어도
player가 정확히 한 번 제거되고 binding/player map이 함께 정리됨을 먼저 통과시킨다. 이 gate 전에는 discovery
listener를 제품 기본으로 켜지 않는다.

old transport가 close 직전 enqueue한 move/skill을 남긴 뒤 lifecycle detach를 먼저 drain하는 경우와, 새
transport generation이 rebind된 뒤 old command가 도착하는 경우도 추가한다. 두 command 모두 generation
fence에서 gameplay state 무변경으로 폐기되어야 한다.

## G01. discovery wire와 focused test seam

PR A에서는 `LanDiscoveryProtocol`만 먼저 구현하고 TCP protocol은 현재 v39를 유지한다. discovery datagram의
`tcpProtocolVersion`은 literal 39/40이 아니라 빌드의 `NETWORK_PROTOCOL_VERSION`을 광고한다. 아직 소비자가
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

```text
Lobby Begin_WorldEntry
-> endpoint resolver worker
-> adapter별 directed broadcast 두 round
-> source address 후보 collect/validate/dedupe
-> connector worker가 후보별 TCP connect
-> main CNetworkManager가 결과 소비
-> 기존 C2S_ENTER_WORLD/S2C_ENTER_ACCEPTED
```

이 G에서는 active-world session resume을 아직 켜지 않는다. discovery/connect 실패는 Lobby에 남고 기존
terminal 동작을 사용한다. 이렇게 동일 LAN IP 입력 제거를 첫 번째 독립 PR로 검증할 수 있다.

focused harness는 explicit override exclusive, `0.0.0.0` reject, loopback fixture 우선, Public-profile skip,
blind rollback-host 미사용, discovery timeout,
wrong/duplicate/stale nonce, 같은 boot ID 다중 NIC dedupe, 서로 다른 boot ID ambiguity, DNS multi-address,
cancel과 500ms deadline을 검증한다. injected adapter/NLM snapshot에서 Private ingress는 reply하고 Public/unknown
ingress는 response 0개인지도 검사한다. discovery rate fixture는 256 live source 뒤 257번째 source가 state 증가나
reply 없이 drop되고, per-source/global burst와 injected refill/idle expiry가 exact bound를 지키는지 검사한다.

sync script는 Server role에서 repo-owned firewall expected set을 reconcile한다. PR A expected set은 Debug/Release
Server program별 TCP 7777과 UDP 7778, `Private,Domain`, `LocalSubnet` rule뿐이다. 같은 group의 old `Profile=Any`,
stale port/CIDR rule과 exact repo-path legacy ungrouped v1 TCP rule을 제거하고 Client role에서는 grouped/legacy
repo-owned inbound rule을 전부 제거한다. PR A에는 routed CIDR
rule이나 AUTO routed candidate를 아직 만들지 않는다. script harness는 Server->Client role 전환과 port 변경에서
stale rule 0개임을 검사한다. legacy Debug/Release `Profile=Any` fixture도 v2 Server sync 뒤 0개, Client 전환 뒤
0개여야 한다. expected mutation에 관리자 권한이 없으면 rule ready를 출력하지 않고
`firewall-reconciliation-requires-administrator`로 즉시 실패·안내하며, endpoint/debugger local 설정 완료와
firewall 미완료를 분리해 보고한다.

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

PR A v39 initial admission에서 자동 retry 가능한 구간은 TCP connect 실패 또는 `C2S_ENTER_WORLD` frame의 어떤
byte도 socket에 commit되기 전뿐이다. enter frame이 일부/전부 전송된 뒤 EOF/reset/approval timeout이 나면
Server가 이미 player를 commit했는지 알 수 없으므로 `AMBIGUOUS_ENTRY` terminal로 끝내고 자동 재입장하지
않는다. v39에는 entry attempt nonce/idempotency가 없어 이 경계를 넘겨 retry하면 duplicate player나 자기
자리의 `ROOM_FULL`을 만들 수 있다.

`ROOM_FULL`, protocol decode/version 위반, invalid content revision, 사용자 cancel과 load/presentation failure도
terminal이다. explicit DIRECT 실패는 AUTO 후보로 우회하지 않는다. PR B에서는 ticket/Server hello/lease가
ambiguous post-send reconnect를 대체한다.

endpoint resolve와 connect의 initial-entry 전체 budget은 5초, TCP connect 뒤 Server approval budget은 별도
5초로 고정한다. retry/backoff 동안 main thread와 Lobby frame은 계속 진행하며 deadline을 넘긴 worker result는
attempt generation에서 stale로 폐기한다.

Lobby pending create identity는 retry 동안 유지하고 최종 terminal/cancel에서만 rollback한다. connect와
approval 상태를 사용자에게 짧게 표시하되 token, raw endpoint credential과 내부 stack을 출력하지 않는다.

send worker와 inbound generation fence를 이 G에서 먼저 제품 경로에 연결한다. old direct `Send_All` 호출을
남겨 두 번째 socket path를 만들지 않는다. Debug loopback와 기존 명시 `127.0.0.1` harness는 discovery를
거치지 않고 그대로 동작해야 한다.
focused peer가 Client hello 직후 EOF/reset하는 DIRECT case는 nickname/token 0 byte와
`POSSIBLE_PROTOCOL_VERSION_MISMATCH`를 기대하되 remote exact version을 39라고 단정하지 않는다. discovery
codec은 v39 query <-> v40 responder 양방향 mismatch status/server version을 별도로 고정한다.

## G04. Server logical lease, detach와 expiry

PR B 시작에서 `NETWORK_PROTOCOL_VERSION`을 39에서 40으로 올리고 G01에서 미리 만들지 않은
Server hello/entry-attempt/cancel/ticket/resume/leave/heartbeat/full-resync packet과 fixed byte reader를 추가한다.
v40 `C2S_ENTER_WORLD`는 기존 bounded payload에 exact 128-bit `entryAttemptId`를 추가한다. 이 시점에
`NetworkProtocolHarness`가 모든 새 TCP message round-trip, token size, unknown enum/version, truncation,
trailing byte와 destination-preserving decode를 Debug/Release에서 먼저 통과해야 Server/Client runtime
consumer를 연결한다.

v40 transport의 first packet은 `C2S_SERVER_HELLO` 하나만 허용한다. hello ACK 뒤 같은 transport의 5초
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
v40 `SERVER_BUSY`, world spawn 부족은 기존 `ROOM_FULL`을 사용한다.

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

일반 gameplay queue를 1024까지 채운 상태에서도 ticket ACK, pending cancel과 `SESSION_RESUME_COMMIT`은 control lane을
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

추가 fault는 half-open heartbeat timeout, send timeout 뒤 partial offset 진행, resume result/commit/committed
각 ACK 유실, initial ticket 0-byte/partial/ACK/confirmed 유실, entry cancel/ACK race, old transport late close,
room queue 포화 중 ticket ACK/resume commit, shutdown producer barrier, Server restart, world transfer와 detach
race다. production 15,000ms 경계는 fake-clock contract test가 검사하고 live runner는
loopback 전용 `--network-contract-harness`에서만 750ms grace를 주입한다. 일반 Server 실행은 이 override를
거부한다.

## G07. v40 cross-network 운영 경계

Client의 cancellable `GetAddrInfoExW` 지원과 trusted routed endpoint 선택은 PR B의 Server hello와 함께
활성화하지만 VPN/overlay 자체를 앱이 설치하거나 구성하지 않는다.

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

이 구현은 두 개의 검증 단위로 나눈다.

### PR A — 자동 endpoint와 async initial connection

- G00 lifecycle baseline
- G01 discovery codec와 focused harness 기반, TCP protocol v39 유지
- G02 trusted LAN UDP discovery
- G03 Client async initial connect와 explicit DIRECT DNS
- Team endpoint schema/sync/firewall와 LAN 운영 문서

PR A는 gameplay session의 disconnect 동작을 바꾸지 않는다. 같은 LAN에서 IP 입력 없이 진입하는 두 PC
수동 확인을 받은 뒤 병합한다.

### PR B — 15초 logical lease와 resume

- G04 시작의 protocol v40 session ticket/resume/full-resync codec와 harness
- G04 Server lease/detach/rebind/expiry
- G05 Client freeze/resume/transactional resync
- G06 4인/private arena/fault regression
- G07 Server hello 기반 routed endpoint/security admission과 최종 public 문서

PR B는 Client/Server/Shared/harness를 같은 revision으로 stop-the-world 배포한 뒤 Server부터 시작하고 v40 Client만
연다. compatibility fallback은 만들지 않는다. PR A discovery schema를 아는 v39/v40 build의 AUTO LAN은
`PROTOCOL_MISMATCH(serverTcpProtocolVersion)`를 typed 표시할 수 있다. DIRECT/routed에서 v40 Client가 hello response
전 EOF/reset을 받으면 실제 old Server version을 단정하지 않고 `POSSIBLE_PROTOCOL_VERSION_MISMATCH` terminal로
표시한다. v39 Client가 v40 Server의 control reject를 이해하는 것은 보장하지 않아 generic EOF가 될 수 있으므로
혼용 운영 자체를 admission하지 않는다.

public 문서는 구현 후 실제 명령과 검증 결과에 맞춰 갱신한다. 계획에 적었다는 이유만으로 AGENTS의 static
`.103` 계약을 먼저 지우지 않는다. PR A merge 전 schema v1에서만 `.103`이 endpoint/server-host 판정
정본이다. schema v2 전환 뒤에는 `rollbackHost`라는 수동 DIRECT 안내값만 되고 local role marker가 Server
machine identity를 소유한다.

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

## 6. 자동 검증

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

## 7. 사용자 수동 검증

에이전트는 Client/UI를 직접 실행하거나 화면을 대신 판정하지 않는다. 자동 검증이 끝난 뒤 사용자가 다음을
직접 수행하고 관찰 결과를 RESULT에 기록한다.

### 7.1 같은 hotspot/LAN

1. Server PC에서 최초 한 번 `Sync-TeamLanEndpoint.ps1 -Role Server`로 local role marker를 만들고, 이후 Auto
   실행에서 `server-host`, Private/Domain profile, TCP 7777/UDP 7778 LocalSubnet ready를 확인한다.
2. Server + Client profile을 시작한다.
3. 다른 PC에서 `LOSTARK_SERVER_HOST`를 지정하지 않고 sync 뒤 Client만 시작한다.
4. Lobby 상태가 discovery source IPv4를 골라 접속하고 정상 4인 입장하는지 본다.
5. Server PC의 Wi-Fi IP가 DHCP로 바뀐 뒤 endpoint literal을 다시 편집하지 않아도 discovery로 접속하는지 본다.
6. 두 Wi-Fi adapter가 켜진 PC에서도 wrong interface/duplicate Server로 가지 않는지 본다.

### 7.2 짧은 Wi-Fi 단절

1. 네 명이 같은 world에 입장한다.
2. 한 Client의 Wi-Fi를 2~8초 끊었다가 복구한다.
3. 해당 Client가 Lobby로 즉시 가지 않고 frozen/recovering 상태를 표시하는지 본다.
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

사용자의 서면 관찰 전에는 LAN auto discovery, Wi-Fi resume, 4인 visual 상태 또는 cross-network 접속을 수동
PASS로 기록하지 않는다.

## 8. 구현 순서와 admission gate

1. PR #245의 `.103` 보정을 병합하고 새 구현 브랜치를 만든다.
2. G00에서 dropped `LEAVE` 가능성과 parser/close 수명을 먼저 고정한다.
3. G01 protocol/discovery codec와 focused harness를 Debug/Release에서 통과시킨다.
4. G02/G03으로 같은 LAN auto discovery와 async initial connect를 닫고 PR A를 사용자 두 PC smoke까지 받는다.
5. G04에서 logical lease와 stable identity를 Server contract test로 닫는다.
6. G05에서 Client recovery/full-resync를 연결하되 Level별 raw disconnect consumer를 한 번에 교체한다.
7. G06의 4인/private arena/fault test를 통과한다.
8. G07 routed endpoint의 exact allowlist와 public exposure 금지를 검증한다.
9. Debug/Release full regression 뒤 RESULT와 public 문서를 실제 구현 상태로 갱신한다.
10. 사용자의 같은 LAN, 짧은 단절, grace 초과와 routed network 관찰을 분리 기록하고 PR B를 올린다.

각 gate가 실패하면 다음 G로 넘어가지 않는다. 특히 discovery가 동작한다는 이유로 session resume까지 완료로
처리하지 않고, reconnect가 된다는 이유로 full authoritative resync와 4인 capacity 보존을 생략하지 않는다.
