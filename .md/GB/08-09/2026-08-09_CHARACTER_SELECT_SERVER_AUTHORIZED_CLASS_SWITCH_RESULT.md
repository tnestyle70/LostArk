# Character Select Server 권위 통합·실시간 클래스 변경 결과

## 완료 상태

Character Select의 socket 없는 Preview와 별도 Server Play 경로를 제거하고, Lobby가
`WORLD_ID::CHARACTER_SELECT_ARENA` 입장을 승인한 뒤에만 여는 단일 Server gameplay Level로 통합했다.
같은 Level의 여섯 class thumbnail을 누르면 별도 확인 없이 typed command가 Server로 전달되고,
승인 snapshot이 도착한 뒤 같은 player/entity의 Client presentation, controller, camera와 animation target을
새 class로 교체한다.

요청 범위였던 Client 1대의 연속 class 변경과 변경 후 skill 사용 계약까지 구현했다. 일반 monster와
Valtan spawn, 상세 multi-client 예외 처리는 이번 변경에 포함하지 않았다.

## 실제 변경

### Shared protocol v13

- `C2S_CHANGE_CHARACTER_CLASS`와 `S2C_CHARACTER_CLASS_CHANGE_RESULT`를 추가했다.
- 요청 sequence, requested/active class와 명시적 거절 이유를 wire 계약으로 정의했다.
- `PLAYER_SNAPSHOT`에 Server 확정 `eCharacterClass`를 추가했다.
- enum, zero/stale sequence, accepted/result 상호 일관성, truncated/trailing payload를 staged decode에서 검증한다.

### Server 권위 class 교체

- Character Select Arena에서만 class 변경을 승인한다.
- 살아 있는 player는 같은 `PlayerId`, `NetEntityId`, 위치와 yaw를 유지한다.
- 사망 상태에서는 기존 `playerSpawn` placement를 다시 검증하고 최초 입장과 같은 Server navigation projection을
  거친 위치에서 부활한다.
- action 중이거나 사망 상태에서도 요청을 받으며, 승인 시 이동/path/trigger/action/skill/combo/hold/cooldown,
  HP/resource/regen/stance를 새 profile 기준으로 초기화한다.
- player 복사본에 변경을 stage한 뒤 모든 검증이 성공할 때만 commit한다. wrong world, stale sequence,
  unsupported/same class와 잘못된 spawn은 기존 player를 보존한다.
- 변경 직후 이전 class skill은 거절되고 새 class skill만 승인되는 contract test를 추가했다.

### Character Select 단일 Server Level

- Lobby의 Character Select command도 Bern/Valtan과 같은 tokenized Server admission 경로를 사용한다.
- `PREVIEW` mode, `Preview / Server Play` 선택, `CHARACTER_TEST_ENTRY_MODE` handoff와 local preview object 경로를
  제거했다.
- Level은 승인된 기존 socket을 소비해 replicated local player가 준비될 때까지 기다린다. 연결 실패·거절·5초
  승인 timeout과 진입 후 disconnect는 기존 계약대로 Lobby로 돌아가며 local fallback은 없다.
- ImGui main list와 product accordion에 Warlord, Slayer, Lance Master, Gunslinger, Artist,
  Dimension Master 여섯 class를 제공한다. 전용 illustration이 없는 Gunslinger/Slayer는 공통 선택 tile과
  class label을 사용한다.
- thumbnail 클릭 전 target class asset admission을 수행한다. 실패·Server 거절·result timeout에서는 기존
  character를 유지하고 상태 메시지를 표시한다.
- class 변경 pending 중에는 다른 stage 진입을 막는다. 이후 Bern/Valtan 입장은 마지막 Server 승인 class를
  `C2S_ENTER_WORLD`에 사용한다.

### Client presentation 교체와 입력 연속성

- snapshot class mismatch 때 새 `CCharacter`를 먼저 stage한 다음 `CNetObjectRegistry::Replace`가 같은
  handle/generation slot을 원자적으로 교체한다.
- registry 또는 layer commit 실패 시 새 presentation을 버리고 기존 record/character를 복원한다.
  복구 가능한 asset/presentation 실패는 socket을 끊지 않고 상태 메시지만 전달한다.
- `CPlayerController::Rebind_LocalCharacter`는 move/action sequence를 유지하고 held/release input edge만
  초기화한다. 따라서 같은 server entity에서 class만 바뀐 뒤에도 command sequence가 되감기지 않는다.
- commit 뒤 camera, `CAnimationTargetService`, controller와 `CCharacterSelectionState`를 새 local character에
  다시 bind한다.

## `vector subscription out of range` 원인과 수정

오류는 Client gameplay가 아니라 `NetworkProtocolHarness`의 구형 world snapshot fixture가 새 class field를
채우지 못한 상태에서 빈 player 벡터의 `[0]`을 검사해 발생했다. fixture를 protocol v13 형태로 갱신하고,
벡터 크기를 먼저 확인하는 방어 검사를 추가했다. 수정 뒤 Debug/Release 하네스 모두 `failures : 0`이다.

## 자동 검증

| 검증 | 결과 |
|---|---|
| NetworkProtocolHarness Debug | PASS, `failures : 0` |
| NetworkProtocolHarness Release | PASS, `failures : 0` |
| Server Debug build + `Server.exe --contract-test` | PASS, `failures : 0` |
| Server Release build + `Server.exe --contract-test` | PASS, `failures : 0` |
| Client Debug build | PASS |
| Client Release build | PASS |
| ClientFrontendHarness Debug/Release build | PASS |
| Character Select Server command/admission tests | PASS |
| Registry same-entity atomic replacement/거절 보존 tests | PASS |
| ProjectAudit Character Select/HUD/Camera 계약 | PASS |

ClientFrontendHarness 전체 실행의 이번 기능 관련 5개 항목은 모두 PASS했다. 전체 집계의 기존 19개 실패는
clean worktree에 팀 관리 대상 Effect/Character runtime Resources가 없는 상태에서 발생한 resource 검사다.

ProjectAudit 전체 집계는 8개 실패다. 이번 기능 계약은 통과했고, 남은 실패는 clean worktree의 map manifest,
Character runtime asset과 Effect authoring/runtime 부재 및 기존 DimensionMaster Effect semantic drift다.

## 수동 검증과 남은 경계

- clean worktree에는 Git 제외 대상 `Client/Bin/Resources` runtime 입력이 없어 실제 ImGui 클릭과 화면 전환을
  이용한 수동 Client smoke는 실행하지 않았다.
- 단일 Client의 연속 class 변경, 상태 reset, 이전/new class skill gate는 protocol/server/client 자동 계약으로
  검증했다.
- 일반 monster, Valtan spawn과 multi-client 동시 변경 정책은 요청대로 후속 수직 슬라이스로 남겼다.
