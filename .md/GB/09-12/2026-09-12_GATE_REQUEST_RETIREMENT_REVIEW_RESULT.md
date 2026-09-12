# 관문 이동 요청 취소·재시도 검토 결과

## G00. 범위와 결론

`2026-09-12_KOUKU_SEQUENCE_COMBAT_HANDOFF_IMPLEMENTATION_PLAN.md`의 관문 종료 전투 연결 중
NetworkManager local spawn token, WorldEntityCommandSink의 optional out token,
PlayerController retire, Level timeout·late reply 경계를 추가로 검토했다.

현재 서버의 room/입장 승인 순서 아래에서 취소 A → 동일 placement 재요청 B → 늦은 A 응답 무시 →
B 승인 계약이 유지된다. 실제 제품 함수/분기문을 추출한 CPU fixture 7개 사례가 통과했다.
월드 초기화 너머까지 tombstone을 보존하는 변경은 하지 않았다. 그 선택의 서버 barrier 근거와
이 전제를 위반한 synthetic 응답 순서에서의 한계를 아래에 구분한다.

제품 변경은 NetworkManager.cpp의 Reset_WorldInboundState에 추가한 영어 ASCII 주석 2줄뿐이다.
기존 bytes를 decode/re-encode하지 않고 해당 ASCII 위치에만 삽입했다. 제품 기능, Shared/Server
wire, Data, Resources를 변경하지 않았다. Client/UI 실행·조작·캡처와 제품 빌드도 수행하지 않았다.

## G01. 요청과 소비 경계

- `Client/Private/NetworkManager.cpp:1592` Send_SpawnWorldEntity는 성공한 전송에 process-local
  uint64 token을 붙인다. 0은 실패/미상이며 요청 queue는 64개로 제한된다. send 실패가 연결을
  닫아 queue를 초기화한 경우도 확인한 뒤 자기 항목만 정리한다.
- `Client/Private/NetworkManager.cpp:2129` Try_Consume_WorldEntitySpawnResult는 응답과 token을
  함께 반환하고 빈 queue에서는 out token을 0으로 돌려준다. optional 인자로 기존 호출자도 유지한다.
- Handle_Frame의 S2C_WORLD_ENTITY_SPAWN_RESULT 분기는 placementId가 같은 요청 중 가장 오래된
  항목을 꺼낸다. 취소는 그 요청을 네트워크 queue에서 삭제하지 않으므로 뒤의 동일 placement B와
  섞이지 않는다. Level은 placementId와 정확한 token이 모두 같은 경우만 pending을 제거한다.
- `Client/Private/PlayerController.cpp:904` retire는 pending sequence, armed, success와 delayed
  상태를 지운다. 다음 요청은 증가한 sequence를 사용하며 실제 reply 소비자는 sequence와 world를
  모두 대조한다. 이미 Server에 제출한 이동/생성 자체를 rollback하는 계약은 아니다.
- `Client/Private/Level_KakulSaydonArena.cpp:1181` timeout은 15초에 Level retire를 호출한다.
  retire는 pending gate·placement map·timer·player request·HUD focus·audition transition을 정리한다.
  늦은 응답은 새 요청과 맞지 않아 gate/HUD/Flow를 완료시키지 않는다.
- `Client/Private/NetworkWorldEntityCommandSink.cpp:5`는 optional out token을 NetworkManager로
  그대로 전달한다. MainApp::CancelKoukuGateCompletePlay의 기존 연결도 gate retirement를 호출한다.

## G02. world reset에서 FIFO를 비우는 근거

`Reset_WorldInboundState`는 연결 준비, C2S_ENTER_WORLD 전송 후, S2C_ENTER_ACCEPTED 소비,
Fail_Protocol/Close_ServerConnection 및 기존 harness boundary가 호출한다. token counter 자체는
이 함수에서 초기화하지 않는다.

1. `Server/Private/ServerApp.cpp:4673`은 하나의 room thread에서 모든 simulation의 Tick을 끝낸 뒤
   Handle_WorldTransfers를 실행한다. 이미 처리된 spawn의 응답은 이 시점 전에 큐에 들어간다.
2. `Server/Private/ServerApp.cpp:4929`는 target REGISTER/ENTER를 stage한 뒤 old room의
   Commit_WorldTransferDeparture를 동기 실행한다. `Server/Private/GameRoom.cpp:1773`은 다음
   target Tick 이전에 Leave를 완료하며, Leave는 기존 session binding을 제거한다(`:2868`).
3. 아직 처리하지 않은 old-room spawn은 제거된 session에서 응답을 만들 수 없다. 이미 처리된
   응답은 target 입장 승인보다 먼저 송신 큐에 들어갔으므로 승인 후 늦게 나올 수 없다.
4. `Server/Private/ClientSession.cpp:505`의 outbound mutex와 `:556`의 push_back은 reliable
   spawn result와 ENTER_ACCEPTED의 순서를 유지한다. Snapshot만 별도 coalescing 대상이다.
5. C2S_ENTER_WORLD는 기존 gameplay binding이 있는 socket의 임의 재입장을 허용하지 않는다
   (`Server/Private/ServerApp.cpp:3684` 부근). 초기 진입은 새 연결이고, 제품 방 이동은 위 transfer를 따른다.
6. Close_ServerConnection과 Fail_Protocol은 socket 중단·receive thread join·raw frame 정리 후
   world 상태를 초기화하므로 이전 연결의 reply가 새 연결의 token을 소비하지 않는다.

따라서 현재 제품에서는 world acceptance가 old-room reply의 경계가 된다. 반대로 tombstone을
world 너머 무조건 보존하면 취소되어 응답이 없는 A가 다음 world의 B 응답을 소비할 수 있다.
GameRoom::Enqueue_Detailed의 LEAVE는 session의 아직 처리되지 않은 ingress command를 지우며
(`Server/Private/GameRoom.cpp:1352`), direct transfer도 떠난 session의 응답을 만들지 않는다.

이 근거는 단순 FIFO가 임의의 world-crossing 응답 순서를 해결한다는 뜻이 아니다. 같은 socket에서
acceptance 이후에 old A 결과를 허용하는 서버 구조로 바뀌면 현재 결과에는 world/request ID가 없어
B와 구분할 수 없다. 그 경우에는 서버가 wire request token을 그대로 반환하는 계약이 필요하다.
이 의존성을 NetworkManager Reset_WorldInboundState의 spawn queue clear 바로 앞에 주석으로 남겼다.

## G03. 실행한 CPU fixture

`out/GateRequestRetirement20260912/make_fixture.py`는 현재 제품의 Send/Consume, spawn reply switch,
controller Request/Cancel/Retire 및 reply matching, Level reply/timeout/complete와 gate retire의 실제
함수 또는 분기문을 그대로 추출한다. reset 검사는 해당 spawn queue clear 문장을 추출한다.
`extraction_receipt.json`은 각 추출 구간의 SHA-256을 기록한다.

송수신은 실제 Shared PacketMessages/PacketWriter/PacketReader/PacketFrame과 GameplayDataRevision
소스를 컴파일하여 serialize→parse한다. socket IO와 HUD/audition side effect는 비UI fixture seam이며
실제 연결이나 제품 Level 전체를 실행하지 않는다.

| 사례 | 결과 |
|---|---|
| cancel A → 동일 placement B → spawn/teleport A 지연 응답 → B 응답 | A 무시, B만 gate 완료 |
| 14.99초 유지 → 15초 초과 timeout → 재요청 → old/new 응답 | pending 전체 retire, 재시도 성공 |
| old 응답 처리 → acceptance reset → 새 요청 | queue clear, token 단조 증가, 새 요청 승인 |
| old 미처리 요청 폐기 → acceptance reset → 새 요청 | 응답 없는 이전 요청이 새 요청을 막지 않음 |
| send 실패 및 send 중 연결 reset | 자기 요청 정리, out token 0 |
| 64개 요청 상한 및 빈 응답 소비 | 초과 요청 거부, 빈 소비 token 0 |
| 다른 placement/world 응답 및 spawn 거절 | unrelated 무시, 거절은 gate 성공으로 소비하지 않음 |

별도의 LIMITATION 사례는 서버 barrier를 의도적으로 위반하여 reset 후 old A 응답을 주입한다.
이 입력은 B token으로 매핑되는 것을 확인했고 PASS로 숨기지 않고 한계로 출력한다. 실제 Server
barrier를 재현한 네트워크 통합 시험이나 packet capture를 수행했다는 의미는 아니다.

최종 `run_fixture.cmd`는 MSVC `/MP2` 이하, 모든 object/exe/PDB를 out 아래에만 기록했다.
compile exit 0, fixture exit 0, 관련 `git diff --check` exit 0이다. 처음 out link에서
GameplayDataRevision의 3개 참조가 누락되어 그 실제 Shared CPP를 추가한 뒤 통과했다.
제품 link/build 상태는 통합 담당의 별도 결과를 따른다.

## G04. 완료와 남은 경계

이번 검토에서 위 서버 계약 안의 기능 오류는 확인되지 않았다. 새 fixture가 확인한 범위는 요청
correlation/retirement의 CPU 상태이며 실제 Server+Client 왕복, HUD 화면, 15초 사용 체감과 최종
Complete Play 전투 시작은 사용자 실행 확인이 남는다. 모든 산출물은 out-only이며 별도 제품
validator나 새 runtime 경로를 추가하지 않았다.
