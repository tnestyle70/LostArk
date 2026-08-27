# Lobby fallback session diagnostics 구현 결과

기준일: 2026-08-27

브랜치: `codex/lobby-fallback-diagnostics`

기준 `origin/main`: `730446cd`

## 0. 결론

Lobby fallback을 평균 네트워크 속도 문제로 미리 확정하지 않고 Client와 Server의 최초 실패를
stable reason으로 보존하도록 구현했다. Client Lobby는 reason/source/detail, remote/local endpoint,
protocol, world/player/entity, packet/WSA/HRESULT, terminal UTC, 마지막 Server tick, queue current/high와
capture 경로를 표시한다. Client와 Server는 각각 실행 파일 옆 `Diagnostics/*-session-<pid>.jsonl`에
payload와 nickname을 제외한 저빈도 session event를 남긴다.

조사 중 4인 room slot을 실제로 남길 수 있는 코드 결함도 확인해 함께 수정했다.

1. room ingress hard cap에서 `LEAVE`가 거부된 뒤 Server binding만 지워지면 player가 영구 ghost로
   남을 수 있었다.
2. Client loading 시작 실패와 Bern/Valtan/Development connection-loss recovery가 Lobby를 요청하면서
   socket을 닫지 않는 경로가 있었다.
3. close와 최초 ENTER가 교차하면 cleanup 뒤 늦은 REGISTER/ENTER가 session을 다시 만들 수 있었다.
4. world transfer target rollback LEAVE 실패를 검사하지 않아 target room state가 남을 수 있었다.

Server는 LEAVE를 일반 ingress cap과 분리한 session-deduplicated priority cleanup queue로 수용하고,
같은 session의 pending command를 취소하며 cleanup보다 뒤에 오는 non-LEAVE를 거부한다. 최초 binding과
REGISTER/ENTER enqueue는 Server sessions lock 아래 하나의 transaction으로 묶었다. Client recovery는
`원인 latch -> socket close -> Lobby request` 순서를 사용한다.

## 1. 구현 상태

### Client

- `SESSION_DIAGNOSTIC_REASON`을 사용해 connect/send/FIN/recv/parser/frame/raw queue/event queue/decode/
  presentation revision/load/activation/replication 실패를 구분한다.
- connection generation마다 first-terminal-wins snapshot을 유지한다. terminal 뒤 main thread가 앞서
  수신된 ACCEPTED/REJECTED/snapshot을 처리해도 world/player/entity/tick 보강은 유지한다.
- `getsockname()`의 Client IP:ephemeral port를 `localEndpoint`로 기록한다.
- semantic recovery는 terminal latch와 별개인 `recovery.reported` JSONL event로 reason/source/detail을
  보존한다. 따라서 `ROOM_FULL` frame과 뒤따른 FIN을 semantic reason과 transport symptom으로 함께 본다.
- Lobby에서 마지막 recovery diagnostic을 유지하고 Server JSONL 상관 방법을 표시한다.
- Character Select의 정상 Back은 failure로 기록하지 않는다.

### Server

- peer endpoint, last inbound packet/time, first terminal reason/native error와 terminal 순간 outbound depth를
  `CClientSession`에 보존한다.
- orderly close, recv/parser/frame, send timeout, reliable overflow, malformed/decode/unknown packet,
  missing binding, decoded command validation, bind/ingress/join/transfer 실패를 typed reason으로 분리한다.
- `lostark.server-session-diagnostic` formatVersion 1 JSON을 stdout과 process별 JSONL에 기록하고 생성 JSON을
  Server 계약 테스트의 parser로 재검증한다.
- `ROOM_FULL` context는 rejected candidate를 포함한다는 사실을 명시하고 candidate session ID와 현재
  active roster의 session/player/spawn/peer/last-inbound 정보를 bounded 문자열로 남긴다.
- cleanup은 gameplay보다 먼저 drain하고 private arena retirement는 queued/in-flight cleanup이 모두
  끝나야 허용한다.

### Wire와 assertion 경계

- `NETWORK_PROTOCOL_VERSION`은 39로 유지했다. 새 reason은 log schema이며 wire payload가 아니다.
- 운영 실패에서 runtime `assert()`로 Client를 종료하지 않는다. schema 순서/이름/중복, malformed input,
  slow reader, queue overflow, cleanup hard-cap, close/entry race와 rollback은 실행형 harness assertion으로
  즉시 실패시킨다.

## 2. 자동 검증

| 검증 | 결과 |
|---|---|
| Client x64 Debug build | PASS |
| Shared + NetworkProtocolHarness x64 Debug build/run | PASS, `failures : 0` |
| Server x64 Debug build | PASS, warning 0 / error 0 |
| `Server/Bin/Debug/Server.exe --contract-test` | PASS, `failures : 0` |
| Client x64 Release build | PASS, `MSBUILD_EXIT=0` |
| Shared + NetworkProtocolHarness x64 Release build/run | PASS, `failures : 0` |
| Server x64 Release build + contract test | PASS, `failures : 0` |
| ValtanFourPlayerHarness Debug/Release | PASS, 4/4 입장·fifth `ROOM_FULL`·disconnect replacement·empty reset, `failures : 0` |
| project XML / JSONL parse / `git diff --check` | PASS |

신규 계약 테스트는 first reason 보존, protocol mismatch/decode/unknown 분류, invalid C2S_MOVE,
bound-close JSON과 cleanup, orderly FIN, invalid frame, reliable overflow, slow reader/send timeout,
graceful flush, hard-cap cleanup 우선순위·dedup, cleanup 뒤 entry 거부, transfer rollback과 fifth Valtan
`ROOM_FULL` roster를 포함한다.

## 3. 사용자 4-PC 검증

자동 검증은 Client 화면을 실행하거나 visual PASS를 대신하지 않았다. 네 PC 수동 검증은 아직
`PENDING`이다.

1. 네 Client와 Server가 같은 commit/configuration/Data/Resources인지 확인한다.
2. Server를 `0.0.0.0:7777`, Client를 `10.207.18.151:7777`로 실행한다.
3. 각 Client가 Lobby fallback되면 Lobby의 reason, local endpoint, terminal UTC와 capture path를 기록한다.
4. Server의 같은 시각 JSONL에서 `peerAddress:peerPort == Client localEndpoint`인 line을 찾는다.
5. `ROOM_FULL`이면 active roster의 네 peer를 실제 네 Client local endpoint와 대조한다. 일치하지 않는
   endpoint가 있으면 stale/ghost 후보이고, 전부 일치하면 Server-host Client를 포함한 실제 fifth entry다.
6. `lastInboundAgeMs`는 heartbeat가 없는 현재 계약에서 idle Client도 커질 수 있으므로 단독 ghost 증거로
   사용하지 않는다.

## 4. 남은 경계

- protocol v39에서는 끊어진 같은 TCP socket으로 Server-only terminal cause를 Lobby에 자동 전달할 수
  없다. Lobby의 local cause와 local endpoint를 Server JSONL에 대조한다. graceful close용 v40 terminal
  packet을 추가해도 실제 send timeout/단절에는 별도 Server log 또는 out-of-band 조회가 계속 필요하다.
- TCP heartbeat/keepalive와 평균 RTT 기반 kick 정책은 추가하지 않았다. 이번 capture의 실제 4-PC 결과
  없이 timeout 수치를 바꾸지 않는다.
- `ROOM_FULL`의 `registeredSessionsIncludingCandidate`는 rejected candidate 자신을 포함한다. ghost 판정은
  숫자나 last-inbound age가 아니라 active roster peer와 네 Client local endpoint의 불일치로 한다.
- Client/UI visual smoke와 실제 Wi-Fi 재현 판정은 사용자가 수행한 뒤 RESULT에 별도로 추가한다.
