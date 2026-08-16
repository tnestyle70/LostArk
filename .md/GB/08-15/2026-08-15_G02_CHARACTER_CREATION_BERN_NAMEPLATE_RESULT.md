# 2026-08-15 G02 Character Creation, Bern Entry, Nameplate 결과

## 0. 결론

상태는 **IMPLEMENTED / AUTOMATED PASS / PARTIAL MANUAL PENDING**이다.

G02의 focused code contract는 실제 물리 폴더 `C:\Users\user\Desktop\LostArk`에 구현됐다.

- Character Select의 `Create Character`가 선택 class와 exact nickname을 pending identity로 stage한다.
- Lobby가 pending identity로 Server-approved Bern entry를 요청한다.
- resource load, rendering profile activation, 실제 `Change_Level(BERN)`까지 성공한 뒤에만 created identity를 commit한다.
- created identity가 없는 direct Valtan은 process-local `Test-<process-id>` audition nickname을 사용한다.
- Server가 보존·복제한 nickname을 Bern/Valtan 캐릭터 머리 위 nameplate가 읽는다.
- 실패는 pending identity만 취소하고 이미 commit된 created identity와 gameplay state는 보존한다.

Shared/protocol, Server contract, Client link, focused Client identity/nameplate harness는 Debug/Release에서
PASS했다. Character Select private isolation과 Bern shared exact-nickname proof도 focused
`-G02IdentityFast` live runner에서 Debug/Release PASS했다. 다만 default runner의 Bern→Valtan trigger 진입은
기존 Bern navigation path가 trigger까지 도달하지 못하고 spawn 근처에서 멈춰 timeout이 발생하므로 전체 live
runner를 PASS로 기록하지 않는다. Client 화면은 실행하지 않았으며 nameplate의 실제 위치·가독성은 사용자
판정 전까지 미완료다.

G02는 기존 `C2S_ENTER_WORLD::strNickName`과 `S2C_PLAYER_SPAWNED::strNickName`을 사용한다.
packet layout과 `NETWORK_PROTOCOL_VERSION`은 바꾸지 않았다.

## 1. 실제 구현

### 1.1 공통 nickname 계약

`LostArk::Shared::Is_Valid_PlayerNickname(std::string_view)`를 Client, packet codec, Server entry가 함께
소비한다.

- UTF-8 byte 수 1~32
- well-formed UTF-8
- embedded NUL, C0/C1 control code 금지
- 앞·뒤 ASCII whitespace 금지
- codec decode 실패 시 destination message 불변

한글도 code point 개수가 아니라 wire의 UTF-8 byte 수로 상한을 판정한다. nickname 중복은 허용한다.

### 1.2 process-session identity

`CCharacterSelectionState`는 한 mutex 아래 다음 상태를 소유한다.

- 현재 selected class
- optional pending creation `{class, nickname}`
- optional committed created nickname
- process lifetime 동안 동일한 audition nickname `Test-<process-id>`

world resolve 정책은 다음과 같다.

| world | class | nickname source |
|---|---|---|
| Character Select / Training | selected, 없으면 Lance Master | audition |
| Bern | pending creation, 없으면 selected + created | created identity가 없으면 실패 |
| Valtan | selected, 없으면 Lance Master | created, 없으면 audition |

invalid stage는 기존 상태를 바꾸지 않는다. cancel은 pending만 지우고 selected/created를 보존하며,
commit은 pending class와 nickname을 selected/created에 atomic replace한다.

### 1.3 Create Character → Bern transaction

Character Select의 `Create Character`는 ImGui command modal을 열고 exact input을 공통 validator에
전달한다. 모달이 열린 동안 class change, arena entity spawn과 gameplay command 제출을 막는다.

```text
Create Character
-> nickname validate
-> Stage_Creation(selected class, exact nickname)
-> Lobby command request
-> Lobby load request
-> old Character Select Level destructor가 socket/presentation 정리
-> Lobby Try_Resolve_ForWorld(BERN)
-> connect + C2S_ENTER_WORLD
-> S2C_ENTER_ACCEPTED
-> target resource load
-> rendering profile activation
-> Change_Level(BERN)
-> Commit_PendingCreation
```

Character Select는 Lobby command와 Lobby load가 성공하기 전에 active character, replication 또는 socket을
선행 teardown하지 않는다. request 실패 시 현재 Server Arena를 유지한다. Lobby connect/send/reject/timeout,
Loading 실패, profile activation 실패와 level 교체 실패는 pending creation을 취소한다.

Lobby의 hardcoded `"Player"`와 selection 부재 시 created Lance Master처럼 commit하던 경로는 제거됐다.
Server approval만으로 identity를 commit하지 않으며 최종 commit 지점은 `CMainApp::Apply_LevelRequest()`의
성공한 Bern `Change_Level` 직후다.

### 1.4 Server 저장과 replication

`CGameRoom` entry validation은 Shared nickname validator를 사용한다. 정상 entry는
`SERVER_PLAYER::strNickName`에 exact nickname을 저장하고 `S2C_PLAYER_SPAWNED`가 같은 값을 broadcast한다.
기존 `CServerApp` world transfer는 source nickname을 target `C2S_ENTER_WORLD`에 전달한다.

nickname은 다음 용도로 사용하지 않는다.

- player/session lookup key
- Party member identity
- vector/map ordering key
- reconnect authentication
- Client 재실행 뒤 영구 저장

이번 G의 `Server 저장`은 살아 있는 Server player/session과 그 world transfer lifetime까지만 뜻한다.

### 1.5 Bern/Valtan nameplate

`CNetObjectRegistry::Get_LivePlayers()`가 replicated record와 살아 있는 `CCharacter` presentation pair를
반환한다. `CClientReplication::Collect_PlayerViews()`는 stable player/entity ID, class, Server-replicated
nickname과 weak character를 read-only DTO로 만든다.

`CWorldPlayerNameplateView`는 다음 순서만 수행한다.

1. weak character와 transform resolve
2. 머리 위 2.2 world-unit offset
3. current VIEW/PROJ와 viewport로 world-to-screen projection
4. camera 뒤, clip 범위 밖, invalid/expired presentation skip
5. strict UTF-8 → UTF-16 변환
6. `Font_YG330` centered text draw

Bern과 Valtan Level은 replication DTO를 수집해 이 view에 전달한다. source nickname은
`NET_PLAYER_RECORD::strNickName`뿐이며 local pending/created nickname을 직접 그리지 않는다. projection,
UTF-8 변환 또는 font draw 문제가 생겨도 character, snapshot, input과 damage는 유지하고 해당 nameplate만
생략한다.

## 2. G02 변경 파일

### Shared / Server

- `Shared/Public/Network/PacketMessages.h`
- `Shared/Private/Network/PacketMessages.cpp`
- `Server/Private/GameRoom.cpp`

### Client identity와 entry

- `Client/Public/CharacterSelectionState.h`
- `Client/Private/CharacterSelectionState.cpp`
- `Client/Public/Level_CharacterSelect.h`
- `Client/Private/Level_CharacterSelect.cpp`
- `Client/Public/Level_Lobby.h`
- `Client/Private/Level_Lobby.cpp`
- `Client/Private/Level_Loading.cpp`
- `Client/Private/MainApp.cpp`

### Client nameplate

- `Client/Public/NetObjectRegistry.h`
- `Client/Private/NetObjectRegistry.cpp`
- `Client/Public/ClientReplication.h`
- `Client/Private/ClientReplication.cpp`
- `Client/Public/WorldPlayerNameplateView.h`
- `Client/Private/WorldPlayerNameplateView.cpp`
- `Client/Public/Level_Bern.h`
- `Client/Private/Level_Bern.cpp`
- `Client/Public/Level_ValtanArena.h`
- `Client/Private/Level_ValtanArena.cpp`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`

### Harness

- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/CharacterSelectIsolationHarness/Private/CharacterSelectIsolationHarness.cpp`
- `Tools/Network/Run-CharacterSelectIsolationHarness.ps1`
- `Tools/ValtanFourPlayerHarness/Private/ValtanFourPlayerHarness.cpp`

## 3. 자동 검증

### 3.1 PASS

| 검증 | 결과 |
|---|---|
| Shared x64 Debug build | PASS |
| Shared x64 Release build | PASS |
| NetworkProtocolHarness Debug | PASS, `failures : 0` |
| NetworkProtocolHarness Release | PASS, `failures : 0` |
| Server x64 Debug build | PASS |
| Debug `Server.exe --contract-test` | PASS, `failures : 0` |
| Server x64 Release build | PASS |
| Release `Server.exe --contract-test` | PASS, `failures : 0` |
| Client x64 Debug build/link | PASS |
| Client x64 Release build/link | PASS |
| ClientFrontendHarness Debug `--g02-nameplate-fast` | PASS, 23 assertions, `failures : 0` |
| ClientFrontendHarness Release `--g02-nameplate-fast` | PASS, 23 assertions, `failures : 0` |
| CharacterSelectIsolation Debug `-G02IdentityFast` | PASS, private combat/isolation + Bern exact nickname, `failures : 0` |
| CharacterSelectIsolation Release `-G02IdentityFast` | PASS, private combat/isolation + Bern exact nickname, `failures : 0` |
| ValtanFourPlayerHarness Debug wrapper (`17778`) | PASS, initial/second generation exact nicknames 포함 4 assertions, `failures : 0` |
| ValtanFourPlayerHarness Release wrapper (`17780`) | PASS, initial/second generation exact nicknames 포함 4 assertions, `failures : 0` |
| G02/project/document targeted `git diff --check` | PASS |

focused Client harness의 23개 assertion은 audition/created/pending world policy, invalid stage 불변성,
atomic commit/cancel, registry record-presentation pair, class replacement 뒤 nickname 유지, expired presentation
제외와 nameplate projection 성공·실패 경계를 포함한다.

### 3.2 전체 PASS로 기록하지 않은 실행

`CharacterSelectIsolationHarness`의 focused `--g02-identity-fast` / runner `-G02IdentityFast` 실행은
Debug/Release 모두 private Character Select combat/isolation과 Bern shared exact nickname proof를 성공했고
`failures : 0`으로 종료됐다. default mode는 그 다음 Bern→Valtan proof까지 실행한다. 현재 Client는
`(137.586, -22.464)`에서 change-level trigger `(144.8, -60.3)`을 향하지만 navigation 결과가
`(137.738, -22.688)` 근처에서 멈춰 20초 뒤 timeout된다.

따라서 현재 증거는 다음처럼 제한한다.

- Bern 입장과 자기/상대 exact nickname spawn replication: 증명됨
- Bern→Valtan transfer가 nickname을 보존한다는 코드 경로: 존재함
- 현재 live runner에서 trigger 발생 후 Valtan accepted + exact nickname + post-transfer command: 미증명

이는 nickname validator나 spawn echo rejection이 아니라 transfer trigger까지 도달하지 못한 기존
navigation/integration blocker다. trigger 도달을 고치거나 runner가 authoring상 유효한 trigger 진입 경로를
사용한 뒤 같은 proof를 다시 실행해야 한다.

`ValtanFourPlayerHarness`는 각 Client가 제출한 label과 자기 `S2C_PLAYER_SPAWNED::strNickName`의 exact
일치를 검사한다. 마지막 source 문구 수정 뒤 Debug/Release를 다시 빌드하고 wrapper를 `17778`/`17780`에서
각각 실행해 initial/second generation exact nicknames를 명시하는 4개 PASS와 `failures : 0`을 확인했다.
이 direct Valtan four-player 증거는 위 PASS 목록에 포함한다.

## 4. 사용자 수동 검증

에이전트는 Client나 UI를 실행하지 않았다. 다음 visual 항목은 사용자의 서면 관찰 전까지 미완료다.

1. `Server + Client` profile로 Lobby 진입
2. Lobby → Character Select Server 승인 진입
3. class 선택 → `Create Character`
4. 영문 또는 한글 nickname 입력 → Bern 자동 진입
5. 자기 캐릭터 머리 위 exact nickname 위치와 가독성 확인
6. 두 번째 Client가 Bern에 들어왔을 때 양쪽 캐릭터 nameplate 확인
7. invalid/edge-whitespace nickname이 modal draft를 보존하고 진입하지 않는지 확인
8. created identity 없는 Client의 direct Valtan이 `Test-<process-id>`를 표시하는지 확인
9. created identity가 있는 Client가 direct Valtan에서 같은 nickname을 표시하는지 확인

이 관찰 전에는 nameplate visual PASS, first pixel 또는 eye smoke를 기록하지 않는다.

## 5. 남은 경계

- Bern navigation/change-level trigger 도달과 Bern→Valtan live nickname transfer 재검증
- 사용자 직접 Bern/Valtan nameplate visual 판정
- nickname 영구 저장, 전역 중복 예약과 계정/캐릭터 DB
- G03의 Server-authoritative Party invite/roster와 stable social identity

Party는 nickname을 member key로 사용하지 않는다. G03은 별도 stable Party/player identity를 사용하고
G02의 `REPLICATED_PLAYER_VIEW`를 hover/nameplate presentation 입력으로만 재사용한다.

## 6. 문서와 Git 경계

- 구현/검증 폴더: `C:\Users\user\Desktop\LostArk`
- 현재 다른 세션의 Effect C++/JSON/도구 변경은 보존했으며 G02 문서 작업에서 수정하지 않았다.
- 새 Level, Loader, build command 또는 Resources 최상위 구조가 생기지 않았으므로 `CLAUDE.md`는 갱신하지 않았다.
- stage, commit, push는 실행하지 않았다.
