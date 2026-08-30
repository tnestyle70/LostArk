# 8전투원 Raid AI 구조 결과

## 1. 이번에 완료한 것

- `ValtanFourPlayerHarness` C++ project/source/filter와 남은 ignored build output 및 빈 물리 폴더를 삭제했다.
- `Run-ValtanFourPlayerHarness.ps1`을 삭제했다.
- `Framework.sln`, Core runner와 native wrapper contract에서 해당 project를 제거했다.
- 고정 `4/4`를 표시하던 Lobby 문구를 human slot의 generic `ROOM_FULL` 문구로 교정했다.
- 기존 `NetworkProtocolHarness`에 8개 unique raid participant의 world snapshot 왕복 계약을 추가했다.
- 일반 `ROOM_FULL` protocol과 Server의 초과 입장 atomic reject, stable-slot replacement, empty-room reset contract는 보존했다.
- 활성 build/network/owner 문서에서 삭제한 실행 명령과 잘못된 assertion 소유권을 교정했다.
- 기본 `Core`에서 Character Select `Core` scenario만 한 번 실행하고 `Party2`/`Party4`는 `FullDiagnostic`로 옮겨, 동일 검증 묶음의 Server 재기동을 3회에서 1회로 줄였다.

## 2. 삭제된 하네스가 실제로 검사하던 것

- 실제 TCP Client 4개의 Valtan 입장
- unique PlayerId/NetEntityId, exact nickname과 4-player snapshot
- 5번째 Client의 typed `ROOM_FULL`
- disconnect 뒤 3명, replacement 뒤 4명
- 전원 퇴장 뒤 두 번째 4인 세대

이 하네스는 skill, boss, AI, axe 1~8, multi-room 2PC, hot reload 또는 pinned generation을 검사하지 않았다. 그런 assertion을 소유한다는 기존 handoff 표는 잘못되어 교정했다.

## 3. 현재 8인 지원 상태

### 이미 가능한 것

- network snapshot 최대 32 actor
- Server의 `m_Players` 전체 snapshot/boss target 처리
- Client의 동적 remote actor registry

### 아직 막혀 있는 것

- Valtan world/publisher가 human `playerSpawn` 4개만 표현하고 `serverBotSpawn`/raid composition을 아직 모름
- `SERVER_PLAYER` command ingress가 human session에 결합
- bot control kind, bot spawn/profile과 fixed-tick command producer 없음
- 마지막 human 퇴장 시 bot 제거 후 arena reset하는 lifecycle 없음

따라서 현재 결과는 `8-player wire capacity PASS`이지 `4 human + 4 bot raid runtime PASS`가 아니다.

## 4. 검증 결과

| 검증 | 결과 | 증거 |
|---|---|---|
| active solution/project reference 감사 | PASS | solution project 13개, `ValtanFourPlayerHarness` active reference 0개 |
| PowerShell parse | PASS | build runner, native exit contract, Character Select wrapper |
| NetworkProtocolHarness Debug build/run | PASS | 오류 0, `Eight Raid Participant World Snapshot Round Trip`, 전체 failures 0 |
| Client Debug incremental build | PASS | `Level_Lobby.cpp` compile/link, 오류 0 |
| `Framework.sln` Debug x64 제품 Build | PASS | 오류 0, 34.28초 |
| runner `Core -SkipBuild -AllowLocalEffectResources` | PASS | Character Select `Scenario=Core`, 전체 52.45초 |
| native wrapper/owned-process contract | 부분 PASS | wrapper 6개와 Character Select success/failure/timeout은 PASS; 전체 script는 삭제된 Artist candidate에 먼저 막히는 기존 EffectRender rejection probe 때문에 FAIL |

기존 같은 PC의 `Core` 83.64초와 비교하면 Character Select party scenario 분리 뒤 52.45초로 31.19초, 약 37% 감소했다. `SkipBuild` 측정이므로 compile 시간 비교가 아니라 validation/runtime path 비교다.

Client visual/audio와 raid AI 품질은 이번 범위가 아니며 PASS로 기록하지 않는다. `8 actor wire`는 통과했지만 Server bot runtime이 아직 없으므로 `4 human + 4 bot raid`도 PASS가 아니다.

## 5. 다음 수직 슬라이스

`human session adapter + server bot command producer -> one player command executor -> 8 actor snapshot -> last-human cleanup/reset`을 하나의 변경으로 닫는다. `MAX_PARTY_MEMBERS=4`는 유지하고 Valtan raid composition을 별도로 8로 정의한다.
