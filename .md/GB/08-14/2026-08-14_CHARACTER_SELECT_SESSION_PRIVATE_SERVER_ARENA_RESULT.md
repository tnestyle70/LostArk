# 2026-08-14 Character Select 세션 격리 Server Arena 결과

## 0. 결론

Character Select는 offline/local simulation으로 바꾸지 않았다. Lobby에서 승인받은 기존 socket,
`CClientReplication`, network command sink, Server navigation·monster brain·collider·damage 경로를 그대로
사용한다.

바뀐 경계는 Server simulation 소유권 하나다.

- `CHARACTER_SELECT_ARENA`: 접속 session마다 독립된 `CGameRoom`
- `BERN`, `VALTAN_ARENA`, `TRAINING_GROUND`: world마다 하나의 shared `CGameRoom`

따라서 Character Select에서는 다른 session의 player, monster, HP, damage event를 볼 수 없지만,
Bern과 Valtan에서는 기존처럼 같은 authoritative world 상태를 공유한다.

## 1. 실제 구현

### Server routing

`CServerApp`은 world ID만 저장하던 binding을 실제 simulation binding으로 교체했다.

- Character Select 입장 시 ready 상태의 private `CGameRoom`을 stage하고 session에 bind한다.
- 일반 gameplay command는 binding 조회와 room enqueue를 같은 lock 구간에서 수행한다.
- world transfer는 source world ID뿐 아니라 exact source simulation identity까지 확인한다.
- target `REGISTER_SESSION`/`ENTER_WORLD`를 stage한 뒤 room thread에서 source departure를 동기
  commit하고 simulation binding을 target으로 교체한다. Target은 다음 tick에 join하므로 source가
  새 target player binding을 다시 지우지 않는다.
- Bern, Valtan, Training Ground는 shared simulation map에서만 resolve한다.

### Private arena lifetime

Session close 시 private arena map을 즉시 지우지 않는다. `LEAVE`를 정확한 simulation queue에 넣고
binding만 제거한다. Room thread가 queued command와 player/entity state가 모두 비었음을 확인한 뒤
arena를 seal한다. Seal 뒤에는 새 command를 거부하고 owner map에서 폐기한다.

이 순서로 disconnect와 command enqueue가 겹쳐도 queued `LEAVE`, audition reset, room lifetime이
중간에 끊기지 않는다.

### Client 경계

Client 코드와 packet 형식은 변경하지 않았다. Character Select는 계속 Server 필수이며 다음 기존
흐름을 사용한다.

```text
Lobby approval
  -> approved socket handoff
  -> CLevel_CharacterSelect
  -> CClientReplication
  -> CPlayerController / network command sinks
  -> session-private Server CGameRoom
  -> authoritative snapshot
```

`LocalArenaSimulation` 또는 가짜 snapshot 경로는 추가하지 않았다.

## 2. 변경 파일

- `Server/Public/ServerApp.h`
- `Server/Private/ServerApp.cpp`
- `Server/Private/Main.cpp`
- `Server/Public/GameRoom.h`
- `Server/Private/GameRoom.cpp`
- `Server/Private/ServerGameplayContractTests.cpp`
- `Tools/CharacterSelectIsolationHarness/Default/CharacterSelectIsolationHarness.vcxproj`
- `Tools/CharacterSelectIsolationHarness/Default/CharacterSelectIsolationHarness.vcxproj.filters`
- `Tools/CharacterSelectIsolationHarness/Private/CharacterSelectIsolationHarness.cpp`
- `Tools/Network/Run-CharacterSelectIsolationHarness.ps1`
- `Tools/Network/Run-ValtanFourPlayerHarness.ps1`
- `Tools/Build/Invoke-BuildAndRegression.ps1`
- `Framework.sln`
- `AGENTS.md`
- `CLAUDE.md`
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`

## 3. 자동 검증 상태

| 검증 | 결과 |
|---|---|
| Debug Server build | PASS |
| Debug `Server.exe --contract-test` | PASS, failures 0 |
| Release Server build | PASS |
| Release `Server.exe --contract-test` | PASS, failures 0 |
| queued `LEAVE` 처리 전 private arena seal 방지 | PASS |
| drain 후 seal 및 late command 거부 | PASS |
| shared Bern room의 private retirement 경로 거부 | PASS |
| Debug/Release live two-client Character Select isolation | PASS |
| A monster spawn + skill `34650` damage가 B에 미노출 | PASS |
| A disconnect/re-entry 중 B entity·tick 유지 | PASS |
| Bern 2-player shared snapshot | PASS |
| Bern → Valtan transfer 뒤 target move command | PASS |
| Debug/Release Valtan four-player baseline regression | PASS |
| XML/PowerShell parse | PASS |
| `git diff --check` | PASS |

두 live runner는 사용자 소유 TCP 7777을 건드리지 않았다. Character Select isolation은 17777,
Valtan baseline은 17778을 사용했고 종료 후 두 listener 모두 0이었다. MSBuild post-build의
`pwsh.exe is not recognized` 문구는 기존 환경 메시지이며 build exit code는 0이었다.

## 4. 수동 화면 검증 경계

에이전트는 Client를 실행하지 않았다. 사용자는 Server + Client로 직접 다음을 확인해야 한다.

1. Lobby에서 Character Select 진입
2. class 변경
3. 일반 monster 또는 Lugaru/Valtan spawn
4. quick-slot/LMB skill 사용
5. collider 표시와 HP/damage 반영
6. 다른 PC도 동시에 Character Select에 들어왔을 때 서로 보이지 않음
7. 두 사용자가 Bern 또는 Valtan에 들어가면 서로 보임

이 관찰 전에는 Character Select visual PASS를 기록하지 않는다.
