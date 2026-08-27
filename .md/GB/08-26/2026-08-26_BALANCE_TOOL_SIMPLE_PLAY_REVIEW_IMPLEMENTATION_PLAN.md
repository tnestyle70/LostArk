# Balance Tool 단순 Play·Decision·Pattern Review 구현 계획

## 1. 목표

`F1 -> Balance Tool`을 열고 10초 안에 다음 세 작업을 찾고 실행할 수 있게 한다.

1. 캐릭터의 실제 HP와 최근 Server damage를 보고, 죽은 캐릭터를 `Revive`한다.
2. 발탄의 실제 gameplay phase 진입 연출을 한 번의 버튼으로 실행한다.
3. 패턴 버튼을 눌러 실제 Server fixed-tick 패턴을 재생하고, 같은 화면에서
   “왜 선택됐는가”와 “지금 어느 stage/action이 실행 중인가”를 본다.

기본 화면은 Winters `AIDebugPanel`의 Actual/Decision evidence 우선 구조를 참고하고, 저작 화면 분리는
active `ChampionTuner`의 별도 tab 구성을 참고한다. one-click `Play & Review`는 LostArk에 새로 정의하는
UX이며 Winters에 같은 버튼이 있었다고 주장하지 않는다. Winters의 점수 계산식이나 UI를 그대로
복사하지 않고 LostArk의 실제 eligibility gate, authored/effective weight와 deterministic RNG ticket을
그대로 보여 준다.

기존 Balance Tool의 전체 저작 기능은 삭제하지 않고 `Advanced Authoring`으로 옮긴다. 기본
`Play & Review` 화면에서 Play가 Save/Publish/Apply를 자동 호출하거나 Client가 phase·damage를
직접 만드는 경로는 추가하지 않는다.

## 2. 용어와 이번 계획의 해석

- 요청의 캐릭터 `review 버튼`은 현재 코드와 HP/damage 문맥상 기존
  `Revive at death position`을 뜻하는 것으로 보고 화면에는 `Revive`로 표기한다.
  패턴과 의사결정을 읽는 기능은 별도의 `Review` 영역으로 부른다.
- `Gameplay Phase`는 Server `boss.iPhase`, snapshot
  `BossCombat.iGameplayPhase`가 소유하는 실제 1·2페이즈다.
- 현재 `CValtanPatternTree::Phases`는 gameplay phase가 아니라 체력바 기믹으로 나눈 파생 구간이다.
  새 UI와 타입에서는 `Health-bar Section`으로 이름을 바꾸고 두 개념을 합치지 않는다.
- `Runtime Decision`은 Server selector가 실제 패턴을 선택한 이유다.
- `Forced Audition`은 사용자가 버튼으로 패턴을 강제 재생한 경로다. 이 실행을 자연 선택의
  weighted 결정으로 표시하지 않는다.
- `Authoring Basis`는 규칙, shape/time, animation/effect reference와 provenance가 현재 값으로
  묶인 근거다. Runtime Decision과 별도 read-only 영역으로 표시한다.
- `Review`는 선택한 행과 화면만 바꾸는 무변경 동작이다. 파일 저장, publisher 실행,
  runtime pointer 교체를 하지 않는다.

## 3. 현재 실측

### 3.1 작업 기준선

- 현재 branch는 `codex/valtan-phase2-pattern-integration`이다.
- `BalanceTool.h/.cpp`, `ValtanPatternTree.h/.cpp`, Server gameplay와 08-26 발탄 문서에 다른 세션의
  미커밋 WIP가 있다. 특히 leap timing, high-jump axe schedule와 arena random 확장이 진행 중이다.
- 구현은 whole-file rewrite를 하지 않고 현재 diff 위에 delta로 적용한다. 이 계획서 작성은 해당
  WIP를 수정하거나 정리하지 않는다.

### 3.2 복잡성의 실제 원인

- `Client/Private/BalanceTool.cpp`는 현재 5,753줄이며 ImGui 호출이 약 291개다.
- 한 클래스가 JSON load/join, draft edit, diff/patch, publisher 실행, hot reload transaction,
  Server audition, live snapshot과 decision trace 렌더링을 함께 맡는다.
- 화면은 `210px 대상 목록 / 중앙 전체 편집기 / 300px Live` 고정 3열이다. 900px 폭을 요구하는
  decision table이 300px child에 들어가 가로 스크롤이 생긴다.
- 같은 패턴이 phase 요약, selection window 후보, mechanic, rotation, manual audition,
  legacy Product와 상세 stage 편집에서 반복된다. 사용자는 한 번 재생할 버튼을 찾기 위해
  weight, hit shape, animation occurrence, effect cue를 모두 지나가야 한다.
- `Play Server Pattern`, Save/Publish/Apply, revive, trace refresh가 한 개 `m_status`를 덮어쓴다.
- Player와 Valtan이 `m_dirty` 하나를 공유한다. Player 값을 바꾼 뒤 Valtan Save가 성공하면
  실제로 저장되지 않은 Player 변경도 clean처럼 보일 수 있다.
- Player 편집기는 입력을 허용하지만 일반 `Save()`는 generated Valtan Product 손실을 막기 위해
  disk save를 차단한다. 단순 화면에서 저장 가능한 것처럼 보이게 두면 안 된다.
- 생성자 `Reload()`가 긴 PowerShell 검증을 동기로 실행할 수 있고, 창의 `m_open=false`를 다시
  열지 못하는 경로가 있어 “10초 안에 첫 동작” 계약을 방해한다.

### 3.3 이미 완료되어 재사용할 계약

- `CValtanPatternTree`의 split authoring/Product/presentation strict join
- stable `bossPlacementId + patternId`의 `CValtanPatternAuditionService`
- Server fixed-tick pattern 실행과 `PENDING -> ACTIVE -> COMPLETED/ABORTED` lifecycle
- immutable pinned gameplay revision과 presentation revision admission
- Server decision trace의 source, rotation, exclusion, authored/effective weight, RNG interval/ticket
- `CCombatHUDViewModel`의 실제 player/boss snapshot과 최근 128개 damage event
- Valtan Validate Draft -> Save Authoring -> Publish Candidate -> 2PC Apply Hot Reload

이번 작업은 이 경계를 UI에서 다시 투영한다. 두 번째 Client pattern runtime, local phase toggle,
local damage simulator를 만들지 않는다.

### 3.4 현재 패턴 수와 묶음

현재 정본 실측은 다음과 같다.

| 기본 화면 묶음 | 현재 수 | 의미 |
|---|---:|---|
| Managed Automatic | 5 | 두 P1 weighted window가 공유하는 자동 후보 |
| Managed Mechanics | 2 | 130줄 전멸, 109줄 arena break/phase 2 진입 |
| Manual Audition | 20 | `authoringPhase` 2인 19개와 3인 candidate 1개, `AUDITION_ONLY` |
| Legacy Product | 26 | 아직 split authoring이 소유하지 않는 Product pattern |
| 합계 | 53 | Product `ValtanEncounter.json`의 전체 pattern closure |

Product selection 기준으로는 `NORMAL 24 / HEALTH_BAR 9 / AUDITION_ONLY 20`, 총 53 patterns와
225 stages다. 위 네 묶음은 53개를 중복 없이 한 번씩 보여 주는 저작 소유권 기준이고,
각 카드에는 실제 selection kind와 window/mechanic/rotation badge를 따로 붙인다.
여기서 `authoringPhase`는 animation-first 저작 분류일 뿐 Server `boss.iPhase`가 아니다.

두 managed window는 `(130,160]`, `(109,130]`이며 동일한 다섯 후보를 가진다.

```text
VALTAN_WHIRLWIND       20
VALTAN_DASH_CHARGE     30
VALTAN_FOUR_SLASH      12
VALTAN_FIST_IN_OUT     14
VALTAN_HIGH_JUMP       14
```

버튼은 다섯 개만 그리고 각 버튼에 두 window badge를 표시한다. 같은 패턴을 window마다 복제하지 않는다.
별도 `ValtanDebugAudition.json`의 52개 row는 하나 이상의 `{patternId, repeat}`와 arena/prop
precondition을 가진 “연출 시퀀스”다. 53개 pattern partition에 섞지 않고 Review의
`Sequences`에서 별도 관계로 보여 준다.

## 4. 정본 관계와 화면 투영

저장된 단일 `phase -> group -> pattern` 트리는 없다. 다음 stable-ID 관계 그래프가 실제 구조다.

```text
selectionWindowId -> selectionSetId -> patternId
mechanicId -------------------------> patternId
legacy rotation section -----------> patternId[]
manual audition row ---------------> patternId
timeline rowId/commandId ----------> [{ patternId, repeat }]

patternId
  -> stageId / actionId
  -> clipOccurrenceId
  -> cueOccurrenceId / effectAssetId / camera cue
  -> hit / combat object / world event
```

```text
Data/Valtan split authoring + generated Product + animation/effect/camera documents
        |
        v
CValtanPatternTree                    read-only joined authoring view
        |
        v
BalanceToolReviewProjection          grouping/breadcrumb/button projection only
        |
        +-> working source projection        Draft/Authoring only
        +-> immutable revision cache         Actual join only

Server CValtanBrain
        +-> decision trace ----------> Runtime Decision
        +-> world snapshot ----------> Actual Pattern / stage highlight
        +-> audition lifecycle ------> global Play status
```

새 grouping 배열이나 phase alias를 JSON에 저장하지 않는다. `selectionWindows/selectionSets`,
`mechanics`, `manualAuditions`, legacy closure와 timeline row를 매 load마다 동적으로 투영한다.

## 5. 기본 화면 계약

### 5.1 최상위 구조

```text
+ LostArk Balance Tool ------------------------------------------------------+
| [Play & Review] [Advanced Authoring]                                      |
| Target: [Characters] [Valtan]       Server Active rev / Draft rev / state |
|----------------------------------------------------------------------------|
| 선택 대상의 단순 실행 콘솔                                                 |
|----------------------------------------------------------------------------|
| [Now] [Decision] [Pattern] [Sequences] [Basis]                            |
| 넓은 Review 영역                                                           |
+----------------------------------------------------------------------------+
```

- 최초 탭은 `Play & Review`다.
- `Advanced Authoring`을 열기 전에는 Validate/Save/Publish/Apply와 전체 stage editor를 보이지 않는다.
- 창을 다시 선택하면 반드시 `m_open=true`로 복구한다.
- 첫 open은 live HUD와 local read-only projection을 즉시 그리되 source 상태를
  `LOCAL_UNVERIFIED / SOURCE_ADMITTED / SOURCE_FAILED`로 명시한다. local view는 탐색과 Server가
  다시 검증하는 stable-ID command에만 쓰고 `Actual joined`의 근거로 승격하지 않는다.
- 현재 Server active revision과 일치하는 immutable review cache가 있으면 `SOURCE_ADMITTED`로 시작한다.
  working source/saved authoring을 판정하는 긴 manifest query와 publisher validation은 사용자가
  Advanced를 열거나 명시적으로 Validate할 때 수행한다. admission 전에는 Save/Publish/Apply를 disable하고,
  실패해도 직전 admitted draft/baseline/cache를 교체하지 않는다.
- 창 공통 상단에는 `Server Active`, `Draft`, `Draft != Active`, 연결/world, Debug-only 상태를
  한 줄로 표시한다.

### 5.2 Characters

```text
Live Character: Lance Master

Server Actual
  HP       12,430 / 20,000
  Damage   OUT 1,280 (latest) | IN 170 (latest)

Authored Target
  [Lance Master] [Gunslinger] [Slayer] [Artist] [DimensionMaster] [Warlord]
  Max HP       20,000
  Base Damage  Attack Power 100

[Revive]                  status: available only when the Server player is dead
```

- `Server Actual`은 class selector가 아니라 현재 session local player의
  `HUD_PLAYER_STATE.eCharacterClass`를 따른다. class selector는 `Authored Target`만 바꾼다.
- `HP`의 큰 값은 local Server current/max다. 같은 class의 authored maximum과 다르면 작은
  mismatch badge만 보인다.
- 현재 `DAMAGE_EVENT::isOutgoing`만으로는 shared room의 OUT damage를 local player에게 귀속할 수 없다.
  wire에 stable source `NET_ENTITY_ID`를 추가하고 local player entity로 filter한 최신 OUT, local player가
  target인 최신 IN만 `Damage`로 표시한다. source identity를 받지 못한 event는 `Room-wide`로 명시하고
  선택 class의 실제 피해처럼 보이지 않는다.
- 단일 `Damage` authoring field를 새로 만들지 않는다. 기본 저작 수치는 `attackPower`이고,
  스킬별 multiplier는 `DamageProfiles.json`의 서로 다른 stable ID다.
- `Revive`는 `IPlayerCommandSink::Request_RevivePlayer` typed 경계를 그대로 사용한다.
  연결됨, 현재 player 존재, HP 0 조건에서만 활성화한다.
- Max HP와 Attack Power 편집은 Advanced의 `Player Basics`에 둔다. Save는
  `PlayerProfiles.json`만 target patch하고 receipt sync/Validate가 성공한 뒤에만 commit한다.
  일반 Player balance는 Publish 뒤 Server 재시작 전에는 적용됐다고 표시하지 않는다.

### 5.3 Valtan Play console

```text
Gameplay Phase
  Actual: P1, 146 bars
  [Reset P1 / Entrance] [Enter P2 / 109 transition] [P3 unavailable]

Pattern Context: [P1] [P2]
Filter: [Managed Automatic 5] [Mechanics 2] [Manual 20] [Legacy 26] [Search]

  [> Whirlwind] [> Dash Charge] [> Four Slash] [> Fist In/Out] [> High Jump]
     badges: NORMAL | windows 160-131, 130-110 | managed

Audition: ENTERING_CONTEXT -> TARGET_ACTIVE | pattern | stage | pinned revision
```

- pattern tile의 주 버튼 한 번이 선택, Review follow와 Server Play를 함께 수행한다.
- 버튼 영역 상단에 “Debug encounter를 reset하고 재생한다”는 고정 설명을 둔다. 매 클릭 modal은 띄우지 않는다.
- in-flight, disconnected, 이미 확인된 wrong world에서는 버튼을 disable하고 한 줄 이유를 보인다.
  현재 handshake에는 Server Debug/Release capability가 없으므로 최초 클릭 전 Release를 추측해 disable하지
  않는다. `REJECTED_RELEASE_BUILD`를 받으면 같은 connection generation 동안 후속 Play를 disable한다.
- Play 결과는 tool-local 문자열이 아니라 process-wide audition snapshot 한 개로 표시한다.
- pattern card는 stable `patternId`만 제출한다. Product vector index 기반 `PLAY_PATTERN`은 사용하지 않는다.
- P3 phase-entry button은 authored gameplay transition이 없어 unavailable이다. `authoringPhase: 3`인
  candidate pattern 자체는 Manual Audition에서 기존 stable-ID Play가 가능하되 “P3 진입”으로 표시하지 않는다.
- 선택 context와 target의 pinned Product `phaseRequirement`와 minimum/maximum phase가 맞지 않으면
  기본 버튼을 disable한다. eligibility를 우회하는 시험은 Advanced의 명시적 `Forced override`로만
  분리하고 Actual phase audition으로 부르지 않는다.

### 5.4 한 화면의 action budget

| 화면 | Primary action | Secondary action | 숨기는 동작 |
|---|---|---|---|
| Character | Revive | authored target 선택 | save/publish, 전체 skill tuning |
| Valtan Phase | Enter selected phase | phase context 선택 | raw HP/phase mutation |
| Valtan Pattern | pattern tile Play | filter/search | stage/hit/effect 편집 |
| Decision | trace row 선택 | Refresh | raw wire/전체 diagnostics |
| Advanced | 현재 단계의 Validate/Save/Publish/Apply | Reload | runtime Play 자동 결합 |

## 6. Phase와 pattern audition의 Server 계약

### 6.1 현재 경계로 닫히지 않는 부분

현재 `PLAY_PATTERN_ID`는 매 실행 전에 보스를 초기 phase 1 상태로 reset한다. 109 연출을 끝내
phase 2가 된 뒤 다른 pattern 버튼을 누르면 다시 phase 1로 돌아간다. 따라서 UI에서
`Enter P2`를 먼저 누르고 기존 pattern button을 이어 누르는 것만으로는 “P2 문맥에서 특정 패턴”이 아니다.

### 6.2 P1 reset과 단일 phase entry

- `Reset P1 / Entrance`는 encounter의 실제 `introPatternId`와 reset/entrance 경로를 실행한다.
  이는 `SET_GAMEPLAY_PHASE(1)` 전이가 아니라 새 phase 1 encounter 상태와 intro를 재현하는 Debug reset이다.
- `Enter P2`는 `VALTAN_ARENA_BREAK_109`의 HEALTH_BAR crossing을 실행한다.
- 단일 P1/P2 버튼은 새 wire를 만들지 않는다. process-wide service가 이미 완료된
  `PLAY_ENTRANCE`와 `PLAY_HEALTH_BAR(109)` operation/result를 한 소비자로 감싼다.
- phase 2 값은 Client나 Debug packet이 직접 쓰지 않는다. 109 pattern의 `IMPACT ENTER`에 있는
  `SET_GAMEPLAY_PHASE(2)`가 Server fixed tick에서 commit한다.
- P1 reset은 Product `introPatternId`에서, 실제 phase entry button은 정확히 하나의
  `SET_GAMEPLAY_PHASE` event를 가진 Product pattern에서 동적으로 만든다. 같은 의미를 새 JSON
  배열로 복제하지 않는다.

### 6.3 phase context + target pattern composite audition

기존 `CValtanPatternAuditionService`를 두 번째 manager로 복제하지 않고 다음 요청까지 일반화한다.

```text
Submit Phase Entry
  bossPlacementId + phaseEntryPatternId

Submit Pattern In Context
  bossPlacementId + phaseEntryPatternId + targetPatternId
```

P2 composite의 Server 순서는 다음과 같다.

```text
exact request validation
-> active GameplayDataRevision pin
-> boss/world audition reset
-> 110 -> 109 health crossing stage
-> CValtanBrain이 실제 VALTAN_ARENA_BREAK_109 선택
-> IMPACT에서 gameplay phase 2 commit
-> entry pattern completion과 phase 2 snapshot 확인
-> reset 없이 targetPatternId를 PendingPatternIds에 queue
-> target pattern ACTIVE
-> COMPLETED 또는 typed ABORTED
```

P1 context는 reset phase 1에서 target을 바로 queue한다. `Reset P1 / Entrance` 버튼을 눌렀을 때만 entrance 자체를
끝까지 실행한다.

Shared audition protocol의 신규 의미는 `context -> target without reset` composite operation에만
한정하고 exact echoed identity와 현재 lifecycle leg(`CONTEXT` 또는 `TARGET`)을 추가한다. 기존
`PLAY_ENTRANCE`, `PLAY_HEALTH_BAR`, `PLAY_PATTERN_ID` wire와 호출자는 compatibility wrapper로 유지한다.
Server는 다음 경우 target을 절대 queue하지 않는다.

- phase entry pattern이 intro도 아니고 `SET_GAMEPLAY_PHASE`를 소유한 HEALTH_BAR pattern도 아님
- target pattern 또는 boss placement가 active pinned revision에 없음
- target의 pinned Product phase requirement/range가 완성된 context phase를 허용하지 않음
- entry가 끝났지만 Server phase가 요청 context와 다름
- owner session/world/room audition epoch가 바뀜
- 다른 audition이 in-flight임
- Release Server, boss dead, player not engaged, gameplay revision admission 실패

Client presentation revision이 없거나 join되지 않는 것은 Server gameplay를 막지 않는다. target은 계속
Server에서 실행하고 해당 Client의 Pattern/Basis detail만 `presentation unavailable/isolated`로 격리한다.

composite의 CONTEXT leg는 기존 `PLAY_HEALTH_BAR` 구현을 복사하지 않는다. encounter/destruction reset,
bait/engagement, `iLastEvaluatedHealthBar`, ground-removal pattern의 wall staging까지 소유한 현재 one-click
health-bar preflight를 공통 helper로 추출해 그대로 사용한다. entry 종료 후 target handoff 직전에
owner session/world/room epoch, boss alive, player engaged를 다시 검증하고 같은 gameplay revision pin을
TARGET terminal까지 유지한다.

Level의 기존 one-based pattern browser와 phase 버튼을 새 기본 UI와 중복 노출하지 않는다.
environment timeline, wall/pillar처럼 별도 목적이 있는 진단만 Level advanced panel에 남기고,
phase/pattern user-facing command는 process-wide service로 이관한다.

## 7. Review 영역

### 7.1 Now

첫 화면에는 다음 한 줄들을 우선 표시한다.

```text
Actual: Phase 2 | 108 bars | VALTAN_ARENA_BREAK_109
Stage: IMPACT (3/6) | action ARENA_BREAK_IMPACT | 0.42 / 1.20 sec
Audition: TARGET_ACTIVE | sequence 18 | revision a1b2c3d4e5f6
Decision: FORCED_HEALTH_BAR -> VALTAN_ARENA_BREAK_109
```

`HUD_BOSS_STATE`에 snapshot server tick, `iActionStartTick`, pinned definition revision을 보존한다.
elapsed 표시는 기존 tick wrap 계산을 재사용한 read-only 표현이며 gameplay stage 진행을 Client가
재판정하지 않는다. pinned revision의 joined definition을 사용할 수 없으면 ID와 stage index까지만
보이고 `Actual definition unavailable`로 격리한다.
live health bar 계산은 편집 중인 Boss draft의 `maximumHealthBars`가 아니라
`HUD_BOSS_STATE.iMaximumHealthBars`만 사용한다.

### 7.2 Decision

Server trace를 좁은 오른쪽 child에서 넓은 Review pane으로 옮긴다.

```text
Trace timeline: [#411 natural] [#412 health] [#413 audition] ...

Selected: VALTAN_DASH_CHARGE
Server Source: WEIGHTED
Derived Context: managed window
Phase/Bar: P1 / 142
Window -> Set: window.valtan.phase1.160.130 -> selectionset.valtan.160.130
RNG: ticket 37 / total 90

Ordinal | Pattern | State | Auth | Effective | Probability | Ticket interval | Reason
1       | ...     | OK    | 20   | 20        | 22.22%      | [0,20)          | -
2       | ...     | PICK  | 30   | 30        | 33.33%      | [20,50)         | selected
3       | ...     | BLOCK | 12   | 0         | 0%           | -               | cooldown
```

- summary는 selected/source/breadcrumb를 먼저 보여 준다.
- Server wire가 직접 주는 값은 source와 `rotationId`다. 위 Window/Set breadcrumb는 각 trace의
  `DefinitionRevision + rotationId`를 같은 revision의 review projection에 exact join해 파생하고,
  revision definition을 찾지 못하면 Actual 값처럼 추측하지 않고 숨긴다.
- candidate table은 RNG interval 의미가 보존되도록 authored ordinal 순서를 유지하고 선택 행을 강조한다.
- exclusion은 `wrong phase`, `range`, `cooldown`, `repeat`, `disabled` 등의 readable chip으로 표시한다.
- `Score`라는 이름과 가짜 utility score를 만들지 않는다.
- forced pattern button trace는 `FORCED_AUDITION`, P2 entry는 `FORCED_HEALTH_BAR`로 명시한다.
- Server에 이미 있는 bounded 32 trace ring을 조회할 수 있도록 typed query/response를 확장한다.
  현재 과거 ring row는 revision을 개별 보존하지 않으므로 새 bounded record가 각 trace의 pinned
  `DefinitionRevision`과 boss identity를 함께 캡처해야 한다.
- 64 KiB frame 안에 64-candidate full trace 32개를 넣지 않는다. timeline batch는 작은 summary만
  byte-budget/pagination으로 보내고, 사용자가 고른 exact trace 한 건의 candidate detail은 별도 query로
  가져온다. `CValtanDecisionDebugViewModel`이 world generation별 summary 최대 32개와 선택 detail을
  소유한다. NetworkManager는 transport/queue만 담당한다.
- 응답은 trace sequence 오름차순이며 duplicate를 거부한다. old/new revision이 섞인 history는
  합법적이므로 active revision과 다르다는 이유로 버리지 않고, local definition join 실패는 해당 row
  detail만 격리한다.
- raw mixed RNG, packet identity와 전체 exclusion mask는 `Decision Diagnostics`를 펼쳤을 때만 보인다.

### 7.3 Pattern

선택한 pattern을 다음 순서로 보여 준다.

```text
Membership
  Managed Automatic
  windows: 160-131, 130-110
  timeline rows: 07, 18

Ordered execution
  [WINDUP] -> [DASH ACTIVE] -> [RECOVERY]
                 ^ live Server stage

Active stage
  stageId / actionId / duration
  animation occurrence(s)
  effect/camera cue(s)
  hit and combat-object badge
  world-event badge
```

- stage/action을 stable ID로 선택하고 live `stageIndex`는 pinned revision 안에서 highlight에만 사용한다.
- animation/effect/camera 값은 owning 문서의 read-only join이다. Balance Tool이 저장하지 않는다.
- `CBalanceTool`은 immutable runtime/saved artifact에서 만든 read-only projection을
  `GameplayDataRevision`별 bounded cache로 소유한다. 최초 active revision과 hot-reload candidate commit
  시 exact projection을 넣고 실패하면 기존 cache를 유지한다.
- Server snapshot, audition lifecycle, decision trace와 cache의 revision이 같을 때만
  `Actual joined` badge를 표시한다. working draft tree를 Actual에 연결하지 않는다. matching cache가
  없으면 ID와 stage index만 보이는 것이 기본이다.
- draft와 active가 다르면 두 값을 합쳐 보이지 않고 `Draft != Active`로 분리한다.

### 7.4 Sequences

- `ValtanDebugAudition.json`의 stable `rowId/commandId`를 시간순으로 표시한다.
- 각 row는 pattern/repeat와 arena/prop precondition을 펼쳐 보여 준다.
- 자연 selector의 selection window와 다른 종류의 묶음이라는 설명을 고정한다.
- row Play는 기존 Server timeline operation을 process-wide command service로 통합한 뒤에만 노출한다.
  단일 pattern button이 timeline row를 암묵적으로 실행하지 않는다.

### 7.5 Basis

한 pattern의 저작 근거를 다음 네 lane으로 나눠 read-only로 보여 준다.

1. `Rule`: selection kind, phase/range/bar, cooldown/repeat, window/mechanic membership
2. `Shape / Time`: stage order, duration, server motion, hit/combat object
3. `Presentation`: animation occurrence, Effect/camera cue, independent presentation reference
4. `Basis`: field-level provenance와 `PROJECT_TUNED` 여부

08-21 master 문서의 A/B/C/D 관찰 근거를 새 runtime JSON으로 복사하지 않는다. 현재 structured
authoring과 provenance에 exact join 가능한 ID만 표시하고, 문서 근거는 read-only 참고 위치로 남긴다.

## 8. 상태 owner와 실패 보존

| 상태 | owner | 성공 시 | 실패 시 |
|---|---|---|---|
| Player/Valtan draft | `CBalanceTool`의 domain별 draft | 해당 domain만 dirty | 기존 draft 유지 |
| pattern grouping | pure review projection | 새 projection 일괄 교체 | 직전 projection 유지 |
| revision별 Review definition | `CBalanceTool` bounded immutable cache | exact revision projection 추가 | 기존 cache 유지/ID-only |
| live HP/damage/boss | `CCombatHUDViewModel` | 최신 Server snapshot | stale/invalid 명시 |
| decision history/evidence | `CValtanDecisionDebugViewModel` | exact sequence append/detail 교체 | 기존 history 유지 |
| decision packet | `CNetworkManager` transport queue | exact frame 전달 | typed transport 실패 |
| phase/pattern command | generalized audition service | lifecycle snapshot 갱신 | terminal reason 보존 |
| gameplay 실행/phase | Server room / `CValtanBrain` | fixed-tick commit | target queue 금지/typed abort |
| Valtan authoring revision | 기존 immutable revision pipeline | CAS/2PC commit | active revision 유지 |
| Player authoring | targeted JSON transaction | receipt sync와 Validate 후 commit | 원본 JSON/receipt 복원 |

`m_status`는 다음처럼 분리한다.

- load/join status
- Player authoring status
- Valtan authoring/pipeline status
- audition status
- decision query status
- revive status

Player/Valtan dirty와 loaded baseline도 분리한다. 한 domain의 Save/Reload가 다른 domain의 변경을
clean 처리하거나 버리지 못한다.

## 9. 변경 파일과 역할

### 새 파일

- `Client/Public/BalanceToolReviewProjection.h`
  - ImGui, network, disk를 소유하지 않는 pattern group, membership, phase entry와 breadcrumb view를 선언한다.
- `Client/Private/BalanceToolReviewProjection.cpp`
  - `VALTAN_PATTERN_TREE_VIEW`에서 53-pattern exact partition과 phase entry를 stage한 뒤 일괄 반환한다.
- `Client/Public/ValtanDecisionDebugViewModel.h`
  - world generation별 bounded trace summary, 선택 trace detail과 revision join 상태를 선언한다.
- `Client/Private/ValtanDecisionDebugViewModel.cpp`
  - NetworkManager queue를 소비해 sequence/revision/boss identity를 검증하고 staged history를 commit한다.

네 파일은 `Client/Default/Client.vcxproj`와 `Client.vcxproj.filters`의 기존 물리 폴더에만 등록한다.
새 filter를 만들거나 기존 파일을 재배치하지 않는다.

### 기존 Client

- `Client/Public/BalanceTool.h`, `Client/Private/BalanceTool.cpp`
  - Play & Review/Advanced mode, 단순 Player/Valtan console, domain dirty/status, revision별 immutable
    review cache와 넓은 Review pane을 소유한다.
- `Client/Public/ValtanPatternTree.h`, `Client/Private/ValtanPatternTree.cpp`
  - `VALTAN_HEALTH_BAR_SECTION_VIEW`와 `Get_HealthBarSections()` semantic accessor를 추가한다. 기존
    `VALTAN_PHASE_VIEW/Phases` storage는 Effect Tool compile compatibility를 위해 한 migration 동안 alias로
    유지하고 값을 복제하지 않는다. UI 표기와 신규 코드는 Health-bar Section만 사용한다.
- `Client/Public/ValtanPatternAuditionService.h`, `Client/Private/ValtanPatternAuditionService.cpp`
  - phase entry와 composite context/target lifecycle을 한 process-wide owner로 일반화한다.
- `Client/Public/CombatHUDViewModel.h`, `Client/Private/CombatHUDViewModel.cpp`
  - boss server tick, action start tick, pinned revision과 local-player latest IN/OUT damage summary를 보존한다.
- `Client/Public/ClientReplication.h`, `Client/Private/ClientReplication.cpp`
  - outer snapshot server tick, entity action start/pinned revision과 local player entity identity를 HUD producer에 전달한다.
- `Client/Public/NetworkManager.h`, `Client/Private/NetworkManager.cpp`
  - exact audition result/lifecycle와 paged decision summary/detail을 전용 service/view model queue에 전달한다.
- `Client/Public/Level_ValtanArena.h`, `Client/Private/Level_ValtanArena.cpp`
  - 중복 phase/pattern browser를 service 호출로 이관하고 environment 전용 diagnostics만 남긴다.
- `Client/Private/MainApp.cpp`
  - Balance Tool 재열기와 service update 순서를 보존한다.

### Shared/Server

- `Shared/Public/Network/PacketType.h`, `Shared/Public/Network/PacketMessages.h`,
  `Shared/Private/Network/PacketMessages.cpp`와 packet type 등록
  - composite request, exact result, lifecycle leg, source entity가 있는 damage event, paged trace summary와
    exact detail을 encode/decode하고 `NETWORK_PROTOCOL_VERSION`을 같은 변경에서 올린다.
- `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp`
  - 기존 health-bar preflight helper, 한 room 한 audition state machine, revision pin,
    context -> target commit/abort와 trace summary/detail query를 소유한다.
- `Server/Private/ValtanBrain.cpp`
  - 기존 selection 알고리즘은 바꾸지 않고 forced source, actual phase transition과 trace별 revision identity를 보존한다.
- `Server/Private/PlayerSkillSystem.cpp`, `Server/Private/MonsterBrain.cpp`,
  `Server/Private/ServerCombatHitRuntime.cpp`
  - damage event를 만든 authoritative source entity ID를 채운다.
- `Server/Private/ServerGameplayContractTests.cpp`, `NetworkProtocolHarness`
  - 정상/잘못된 ID·phase·revision·duplicate·preflight rollback/late abort와 packet round-trip을 실행형으로 검증한다.

### Tool/문서

- `Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py`
- `Tools/ValtanPipeline/test_valtan_balance_tool_contract.py`
- `Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py`
- 신규 focused projection/audition fixture가 필요하면 같은 폴더에 추가
- `.md/TEAM/BALANCE_TOOL_OWNER_HANDOFF.md`
- `.md/TEAM/BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`
- `.md/TEAM/발탄인수인계서.md`
- 대응 `*_RESULT.md`

TEAM 문서의 stale `33 patterns / 131 stages` 표는 실제 현재
`53 / 225 / split-owned 27(master-managed 7 + manual audition 20) + legacy 26`과 같은 변경에서 교정한다.

## 10. G별 구현 순서

### G00. 기준선과 안전 하네스 복구

현재 dirty WIP와 Product `53 patterns / 225 stages`, split-owned 27
(`master-managed 7 + manual audition 20`), legacy 26, timeline 52를
검증 fixture로 고정한다. `Run_ReadOnlyRoundTripContractTest`의 조기 return 뒤 unreachable 검증을
단순히 다시 실행하지 않는다. 그 tail은 empty `m_patterns` 이전의 33-pattern lossy serializer 전제이므로
obsolete block을 제거하고 domain serializer/projection invariant test로 교체한다. window reopen과 첫 open의
source admission 상태를 분리한다.

종료 증거는 기존 파일 bytes를 바꾸지 않는 local load, `LOCAL_UNVERIFIED -> SOURCE_ADMITTED/FAILED`,
admission 실패 시 Advanced 명령 차단, window 재열기, 현재 closure와 dirty domain 보존 focused test다.

### G01. Review projection과 용어 교정

pure `BalanceToolReviewProjection`을 추가한다. 53개 pattern을 네 그룹에 정확히 한 번 배치하고,
selection window/mechanic/legacy rotation/manual audition/timeline membership을 stable ID로 join한다.
compile-compatible semantic accessor로 `HealthBarSections`를 제공하고 실제 Server gameplay phase와
별도 필드로 유지한다. Effect Tool의 기존 alias/storage와 관련 EffectPipeline test가 깨지지 않는지 함께 확인한다.

종료 증거는 unknown/duplicate/missing ID, 같은 다섯 후보의 두-window 중복 버튼,
`authoringPhase`를 gameplay phase로 오인한 P3 가짜 phase entry를
모두 거부하는 projection contract다.

### G02. Character 단순 화면과 domain transaction 분리

Player 기본 화면은 session local player의 actual HP, source entity로 filter한 latest OUT/IN damage와
Revive만 보여 준다. class selector는 authored target만 바꾼다. authored max HP/AP는 요약만 표시하고
편집은 Advanced로 보낸다. Player/Valtan dirty, loaded baseline, status를 분리한다.
Player Save는 `PlayerProfiles.json` targeted patch -> provenance receipt sync -> gameplay Validate ->
atomic commit/rollback만 수행하며 Valtan generated Product를 serialize하지 않는다.

종료 증거는 여섯 class, alive/dead/disconnected revive gate, Player Save가 Valtan draft를 clean 처리하지
않는 것, `Update-BalanceProvenanceReceipt.ps1` 또는 Validate 중간 실패에서 PlayerProfiles/receipt 두 파일
hash가 보존되는 것이다. source entity가 다른 room-wide OUT event도 local Damage로 채택하지 않는다.

### G03. Phase-context Server audition

process-wide service가 기존 P1 entrance와 109 P2 operation/result queue를 한 owner로 감싼다. 신규 wire와
Server room state machine은 P2 entry 후 reset 없는 target queue와 lifecycle leg만 추가한다. composite
CONTEXT는 기존 health-bar environment/destruction preflight helper를 재사용한다. old stable-ID pattern
Submit은 compatibility wrapper로 유지하고 Level의 중복 user-facing phase/pattern 소비자를 이관한다.

종료 증거는 109 `IMPACT` 전에는 phase 1, event commit 뒤 phase 2, target pattern 동안 phase 2 유지,
실패 시 target 미실행과 typed abort다. preflight commit 전 실패만 기존 room state를 그대로 보존한다.
109가 여러 tick 실행된 뒤의 late failure는 arbitrary 이전 상태 복원을 약속하지 않고 필요하면 canonical
audition reset으로 정리한다.

기존 Effect Tool stable-ID play, Valtan Arena single pattern, Character Select private-room play와 여섯
environment-dependent rejection도 같은 G의 regression으로 유지한다.

### G04. Valtan one-click Play console

G01 projection과 G03 service로 phase/context/filter/pattern button grid와 global lifecycle strip을 그린다.
한 패턴은 한 버튼만 가지며 click이 stable ID를 제출하고 Review selection을 따라가게 한다.

종료 증거는 10초 first-action layout, in-flight single owner, wrong-world disable,
`REJECTED_RELEASE_BUILD` 뒤 connection-generation disable, Draft != Active 표시와 Play/Save 미결합 source test다.

### G05. Winters AIDebug 참고 Decision과 Actual Pattern Review

trace마다 pinned revision과 boss identity를 보존한 Server bounded record에서 paged summary를 가져오고,
선택한 한 trace의 detail만 별도로 가져와 evidence를 표시한다. `CValtanDecisionDebugViewModel`이 history와
selection을 소유한다. live snapshot의
phase/pattern/stage/action/tick/revision을 joined tree에 연결해 stage strip을 highlight하고,
membership, Sequence와 Basis를 별도 탭으로 제공한다.

종료 증거는 natural weighted, forced health, forced audition trace를 서로 다른 source로 표시하고,
authored ordinal/RNG interval이 보존되며 revision mismatch에서 잘못된 stage detail을 보여 주지 않는 것이다.

### G06. Advanced Authoring으로 기존 전체 편집기 격리

현재 pattern/stage/hit/motion/volley/presentation reference와 Validate/Save/Publish/Apply UI를
Advanced로 이동한다. 기존 필드와 transaction 의미를 삭제하거나 자동 결합하지 않는다.
필요한 sub-tab은 `Player Basics / Selection / Pattern / Stages / Combat Object / Pipeline` 순서로 두고
presentation-owned 값은 계속 read-only다. 이번 slice에서 lossless transaction을 닫는 Player authoring은
`maximumHp/attackPower`뿐이다. 기존 94 PlayerSkills와 DamageProfiles detail은 Review/diagnostic read-only로
보존하고, 별도 domain transaction이 생기기 전에는 editable처럼 표시하지 않는다.

종료 증거는 기존 Valtan authoring round-trip, immutable revision/CAS/2PC test가 그대로 통과하고
기본 화면에는 해당 편집기가 나타나지 않는 것이다.

### G07. 문서, 자동 검증, 사용자 수동 gate

정본 TEAM 문서의 현행 count, grouping, phase/context semantics와 operator 경로를 갱신한다.
실제 diff와 자동 검증만 RESULT에 PASS로 기록하고 Client visual과 occurrence fidelity는 사용자가
확인할 때까지 `수동 미검증`으로 남긴다.

## 11. 자동 검증

### 데이터와 projection

```powershell
python -B Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py
python -B Tools/ValtanPipeline/test_valtan_balance_tool_contract.py
python -B Tools/ValtanPipeline/test_valtan_pattern_master_v2.py
python -B Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1 -Mode Validate
```

Focused fixture는 다음을 검증한다.

- 53 pattern이 네 기본 그룹에 중복/누락 없이 한 번씩 존재
- 동일 managed 5 pattern은 두 window badge를 가지되 버튼은 하나
- Product 24 NORMAL / 9 HEALTH_BAR / 20 AUDITION_ONLY와 joined selection kind 일치
- 52 timeline row는 pattern partition과 별도이며 모든 pattern reference가 resolve
- actual gameplay phase와 Health-bar Section 혼용 없음
- P1은 `Reset/Entrance`, P2 109만 actual phase entry, P3 phase-entry만 disabled
- `authoringPhase: 3` candidate는 Manual Audition stable-ID Play에서 계속 보임

### protocol/Server

- protocol version bump와 composite request/result/lifecycle, damage source ID,
  trace summary pagination/exact detail의 Debug/Release packet round-trip
- 모든 frame이 64 KiB 한도 안에 있고 64-candidate detail 1건이 round-trip
- unknown placement/context/target, empty/traversal/oversized ID 거부
- duplicate request sequence, 다른 owner의 in-flight, disconnect/world transfer/room epoch 변경
- phase entry 중 gameplay revision pin, hot reload prepare와의 교차
- presentation revision unavailable이어도 Server target은 실행하고 Client detail만 격리
- 109 crossing -> IMPACT -> phase 2 -> target pattern 순서
- entry 실패/예상 밖 pattern/boss death에서 target 미실행과 typed ABORTED
- decision history 최대 32, oldest-first, mixed revision row 보존, duplicate sequence 없음
- `Server.exe --contract-test`

### build와 정적 검사

1. Shared + NetworkProtocolHarness x64 Debug/Release, harness 실행
2. Server x64 Debug/Release, `Server.exe --contract-test`
3. Client x64 Debug/Release compile
4. 변경된 JSON/XML parse
5. `git diff --check`
6. 필요 시 정본 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug/Release`

## 12. 사용자 수동 검증

에이전트는 Client/UI를 자율 실행·조작하거나 스크린샷을 만들지 않는다. 빌드와 실행 준비 뒤
사용자가 다음을 직접 확인한다.

1. Debug Server + Client를 `Server + Client` profile로 실행한다.
2. Valtan Arena에서 `F1 -> Balance Tool`을 열어 기본 탭이 `Play & Review`인지 확인한다.
3. 10초 안에 Character HP/Damage/Revive와 Valtan phase/pattern 버튼을 찾을 수 있는지 확인한다.
4. `Reset P1 / Entrance`, `Enter P2 / 109 transition`을 눌러 실제 entrance와 109 연출을 본다.
5. P2 context에서 manual/legacy pattern을 눌러 109 연출 뒤 같은 phase 2 상태에서 target이 이어지는지 본다.
6. Decision에서 자연 선택과 forced audition source가 다르게 표시되는지 본다.
7. Pattern에서 실제 stage highlight, animation/effect/camera/hit badge가 화면 발생 순서와 맞는지 본다.
8. Character가 죽었을 때만 Revive가 활성화되고 실제 Server snapshot HP가 회복되는지 본다.
9. disconnected, Release Server, in-flight, Draft != Active 실패 문구가 한 줄로 이해되는지 본다.

사용자가 pattern occurrence, animation 연결감, Effect/camera visual fidelity를 확인해 서면으로 승인하기
전에는 visual PASS 또는 완료로 기록하지 않는다.

## 13. 완료 조건과 제외 범위

완료 조건은 다음 전부다.

- 기본 화면에서 Player와 Valtan 주요 동작을 한 단계로 실행
- pattern grouping 53개 closure와 관계 breadcrumb 제공
- P2 actual entry와 P2-context target audition이 Server 권위로 동작
- Runtime Decision, Actual Pattern, Authoring Basis를 섞지 않고 표시
- 기존 Advanced authoring과 immutable transaction 회귀 없음
- 자동 검증 통과와 사용자 수동 visual 판정의 상태 분리

이번 범위에서 하지 않는다.

- phase 3 gameplay transition 발명
- selector weight/우선순위 알고리즘 변경
- ImGui를 제품 UI로 승격
- Animation/Effect/Camera 문서의 Balance Tool 저장
- Client local AI, phase, hit, damage 판정
- Play와 Save/Publish/Hot Reload 자동 결합
- 모든 legacy pattern을 split authoring으로 승격
- 에이전트의 Client 실행, 화면 캡처, visual fidelity PASS
