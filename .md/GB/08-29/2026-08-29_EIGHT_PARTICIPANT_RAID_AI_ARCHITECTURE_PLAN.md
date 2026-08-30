# 8전투원 Raid AI 구조 계획

## 1. 목표

- Valtan의 고정 `FourPlayer` 제품 개념을 제거한다.
- 현재 목표를 인간 Client 4명 + Server AI 4명 = 전투원 8명으로 정의한다.
- 봇을 가짜 TCP Client로 만들지 않고 인간과 같은 Server 전투 규칙을 통과시킨다.
- Valtan에서 닫은 구조를 재사용해 이후 Kakul-Saydon 같은 encounter별 AI를 추가할 수 있게 한다.
- 새 broad harness project를 만들지 않고 제품 contract, protocol contract, 최소 live scenario로 검증한다.

## 2. 현재 코드 실측

| 경계 | 현재 상태 | 판정 |
|---|---|---|
| TCP listener | `SOMAXCONN`, 4-session 상수 없음 | transport가 4인 병목은 아님 |
| world snapshot | `MAX_WORLD_SNAPSHOT_PLAYERS = 32` | 8 actor wire 수용 가능 |
| Server room | `m_Players`를 snapshot과 Valtan target pool로 사용 | 8 combat actor의 기반은 이미 있음 |
| human admission | 남은 enabled `playerSpawn`을 찾음 | Valtan authored spawn 4개는 현재 목표의 human slot 계약과 일치 |
| world publisher | Valtan enabled `playerSpawn` 4개만 검증 | human exact-4는 유지하되 bot slot/composition 검증을 추가해야 함 |
| party | `MAX_PARTY_MEMBERS = 4` | 인간 social party 계약이므로 유지 |
| command ingress | move/skill이 `SESSION_ID -> PLAYER_ID`를 필수로 resolve | Server bot이 직접 사용할 수 없음 |
| arena reset | `m_Players.empty()`일 때만 reset | bot이 남으면 마지막 인간 퇴장 후 reset 불가 |

따라서 `Server가 8명을 허용한다`는 말은 두 개로 나눈다.

1. 실제 network session: 인간 Client 4개
2. Server-authoritative replicated combat actor: 인간 4 + 봇 4 = 8개

향후 인간 8명을 지원하려면 별도로 human admission, 8개 human spawn, raid-group/two-party 계약을 구현한다. 현재 목표에 그 작업을 섞지 않는다.

## 3. Winters에서 가져올 원리

Winters의 재사용 대상은 AI 내용이 아니라 dependency direction이다.

```text
human packet -> session/auth -> normalized actor command --+
                                                      +--> one executor -> simulation -> snapshot
server bot brain -> deterministic command producer -----+
```

Winters의 tick은 human command drain 뒤 Server bot AI가 같은 command queue에 명령을 넣고, 둘 다 하나의 executor를 통과한 뒤 simulation과 snapshot을 실행한다. LostArk도 다음 순서로 맞춘다.

1. human wire command drain 및 session/sequence 검증
2. 직전 committed raid state로 bot perception/decision
3. human과 bot intent를 `PLAYER_ID` 기반 내부 command로 정규화
4. stable actor ID/sequence 순서로 하나의 player command executor 실행
5. player movement/skill, boss, collision, damage simulation
6. 8 actor state를 실제 human session 4개에 broadcast

Winters의 lane, turret, farm, recall, 10-bot roster와 대형 AI system은 복사하지 않는다. LostArk 봇은 boss pattern/stage/pose, party 상태, navigation, cooldown/resource를 읽는 raid 전용 brain이다.

## 4. 목표 Runtime 구조

### 4.1 Actor와 session 분리

- `m_Players`: 인간과 봇을 포함한 전투 actor 전체
- `m_PlayerIdByEntityId`: 인간과 봇 전체
- `m_PlayerIdBySessionId`, `m_Sessions`: 인간만
- `SERVER_PLAYER`에는 명시적인 control kind(`HUMAN_SESSION`, `SERVER_BOT`)를 둔다.
- 봇의 `iSessionId`는 invalid이며 party invite, chat, inventory persistence, reward session 대상이 아니다.
- boss target selection, combat overlap, damage, death와 snapshot은 control kind가 아니라 targetable combat actor 기준으로 처리한다.

### 4.2 하나의 command/executor

현재 `Handle_Move(sessionId, C2S_MOVE)`와 `Handle_UseSkill(sessionId, C2S_USE_SKILL)`의 session 인증과 실제 gameplay 적용을 분리한다.

- human adapter: session ownership, packet sequence, finite/range 검증
- bot producer: fixed-tick brain이 actor ID와 typed move/skill intent 생성
- common executor: `SERVER_PLAYER&`에 `Commit_MoveGoal`, `CPlayerSkillSystem`을 사용해 같은 cooldown/resource/action/damage 규칙 적용

봇 전용 damage, cooldown, movement 구현이나 `Handle_UseSkill`에 fake session ID를 넣는 우회는 만들지 않는다.

### 4.3 Data와 spawn

- ordinary `playerSpawn` 4개를 단순히 8개로 늘리지 않는다. 그렇게 하면 human admission까지 8개로 열리고 봇 slot 예약이 사라진다.
- 기존 `Gameplay.world.json` schema를 확장해 human spawn과 `serverBotSpawn`을 typed placement로 구분한다.
- bot slot은 stable placement ID, character class, brain profile ID, spawn transform을 가진다.
- Valtan은 human 4 + bot 4, 다른 encounter는 자체 composition을 선언한다. 기존 human `playerSpawn` exact-4 검증은 유지하고 bot exact-4 및 total-8을 별도 검증한다. 전역 `8` 하드코딩으로 Kakul-Saydon까지 묶지 않는다.
- publisher는 slot ID 중복, class/profile/navigation, human/bot count, total snapshot capacity를 검증한다.

## 5. Lifecycle과 reset

봇은 raid room의 수명에 속한다.

1. 첫 human cohort의 Valtan admission이 commit되면 bot 4명을 stage한다.
2. 8 actor가 모두 준비된 뒤 encounter를 활성화한다.
3. human disconnect는 해당 human만 제거하되 정책에 따라 bot fill 또는 raid 유지 여부를 결정한다.
4. 마지막 human session이 나가면 bot command 생성을 중단하고 봇 4명을 despawn한다.
5. 그 다음 기존 empty-room boss/spawn-group/destruction/Esther reset을 실행한다.

현재 `m_Players.empty()` gate를 그대로 두면 봇 때문에 room이 영원히 비지 않는다. last-human 판단은 human session binding을 기준으로 하고, bot 제거가 끝난 뒤에만 기존 reset을 호출해야 한다.

Trigger도 현재 모든 `m_Players`를 순회하므로 `movePlayer`, `changeLevel`, encounter activation을 봇이 소비할 수 있는지 action별 정책을 명시한다. 기본은 human-only trigger다.

## 6. 최소 검증 계약

단순히 `입장 성공`과 `skill packet send 성공`만 보는 것은 부족하다. 다음 다섯 묶음이면 충분하고 그 이상 broad project는 필요 없다.

1. Roster/control contract
   - 4 valid human session + 4 invalid-session bot, 총 8 unique PlayerId/NetEntityId
   - bot이 session/party map에 들어가지 않음
2. Common executor parity
   - 같은 초기 상태의 human-normalized move/skill과 bot-produced move/skill이 같은 accept/reject 및 state 결과
   - invalid skill, cooldown, resource 부족도 동일하게 거부
3. Combat bidirectional contract
   - bot skill이 실제 `CPlayerSkillSystem`과 damage event를 통해 boss HP를 변경
   - Valtan target selection과 pattern damage가 bot에도 적용
4. Replication/live contract
   - protocol은 8-player snapshot과 unique entity ID를 왕복
   - live scenario는 실제 socket 4개가 동일한 8 actor spawn/snapshot, human 대표 skill과 bot 대표 skill의 authoritative 결과를 관찰
5. Lifecycle/determinism contract
   - 마지막 human leave -> bot despawn -> command queue/encounter/Esther/reset 정리
   - 같은 seed/initial state의 두 run에서 bot command trace와 final raid-state digest 동일
   - bounded soak에서 30 Hz tick, stuck/inactivity, damage-event capacity와 queue drain 검사

wire codec과 pure Server contract는 기존 `NetworkProtocolHarness`와 `ServerGameplayContractTests`에 넣는다. 실제 socket 증거는 하나의 generic live scenario에만 둔다. `ValtanFourPlayerHarness`를 이름만 바꿔 복원하지 않는다.

## 7. 구현 순서

### G1. 고정 FourPlayer 퇴역

- solution/project/wrapper/Core runner에서 `ValtanFourPlayerHarness` 제거
- 제품의 일반 `ROOM_FULL` codec과 atomic reject/reset contract 유지
- 8-player snapshot wire contract를 기존 NetworkProtocol에 추가
- 기본 `Core`의 Character Select live 검증은 `Scenario=Core` 한 번만 실행하고 `Party2`/`Party4`는 `FullDiagnostic`로 이동

### G2. Control source 분리

- human session adapter와 actor command executor 분리
- `SERVER_PLAYER` control kind 및 bot lifecycle 추가
- 기존 human behavior 무변경 contract 통과

### G3. Data-driven raid composition

- `serverBotSpawn` authoring/publisher/runtime 추가
- Valtan human 4 + bot 4 stage/commit/rollback
- 마지막 human exit reset 수정

### G4. Raid brain

- 이동, 생존, target 선정, 대표 skill rotation부터 시작
- Valtan pattern 회피/딜 window/부활을 작은 policy로 확장
- Kakul-Saydon은 Valtan foundation 이후 별도 encounter slice로 구현

### G5. 검증과 사용자 smoke

- 위 다섯 자동 계약 통과
- 사용자가 실제 Client 4개에서 8 actor 표시, 애니메이션/이펙트/사운드, raid 진행을 판정

## 8. 이번 변경의 완료 경계

이번 변경은 G1만 수행한다. `ValtanFourPlayerHarness` 삭제와 8-player wire proof는 완료하지만, Server bot actor, 8전투원 runtime composition, raid 완주 AI는 아직 구현하지 않는다. 따라서 이 변경만으로 제품 Server가 인간 4 + 봇 4 raid를 실행한다고 보고하지 않는다.
