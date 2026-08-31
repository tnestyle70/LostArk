# Action Composition Workbench Pattern Blueprint 저작 구현 계획서

## 목표

이미 병합된 Action Composition Workbench를 다시 만들지 않는다. 현재 가능한 Pattern 생성,
Animation·Effect·Sound Resource 연결, Stage clock/gap 조정, Sequencer box 편집과 읽기 전용
Boss Pattern graph를 그대로 사용한다. 이번 후속 작업은 다음 한 흐름을 안전하게 닫는다.

```text
Pattern 선택
-> Blueprint에서 지원된 outcome 분기 연결 또는 변경
-> 선택 route를 Sequencer의 일곱 lane으로 확인
-> Details에서 시간·거리·각도·Collider 수치 조정
-> canonical typed source에 Save
-> Validate / Publish / canonical reload
-> exact Server-active revision 확인
-> Boss Tool Complete Play / Restart / Queue Next
```

Blueprint는 새 JSON, 새 runtime VM, 새 Server authority가 아니다. `Data/Valtan` gameplay와
presentation 정본을 편집하는 UI projection이고, Boss Tool은 Publish된 revision을 재생하는
검증 도구다.

## G00. 현재 물리 폴더 기준선

### 저장소 상태

2026-08-31 실측 기준:

```text
canonical base : origin/main f9e6b8500d5fa09080bb5a8a7b428b32fa351610
merged PR      : #274 codex/composition-workbench
target branch  : codex/valtan-pattern-blueprint-authoring
shared worktree: 계획 도중 타 세션이 codex/kakul-saydon-animation-pattern-authoring으로
                 전환했고 Workbench/Animation/test 변경을 가진 dirty 상태
LAN role       : server-host, Server + Client profile
```

`codex/composition-workbench`의 tree와 `origin/main` tree는 byte-identical하다. 이 PLAN은
과거 WIP가 아니라 PR #274가 병합된 물리 source를 기준으로 한다. 실제 구현과 commit은 타 세션
변경을 보존한 뒤 target branch의 별도 clean worktree 또는 안전하게 복귀한 checkout에서만
시작한다. 현재 공유 dirty worktree에서 자동 stage/commit하지 않는다.

### 데이터 정본과 생성물

| 역할 | 정본/소비자 |
|---|---|
| Pattern Stage, branch, event, motion, Collider | `Data/Valtan/Valtan.gameplay.json` |
| Animation occurrence, Effect invocation, Camera invocation | `Data/Valtan/Valtan.presentation.json` |
| Pattern Sound occurrence | `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json` 별도 CAS owner |
| Pattern Shake occurrence | `Data/Animation/Authored/Valtan/Valtan.patternshakecues.json` projector 생성 read-only dependency |
| 패턴 간 Boss audition Flow | `Data/Encounters/Valtan/ValtanBossAuditionFlows.json` graph v2 source |
| 생성 Product | `Data/Encounters/Valtan/ValtanEncounter.json`, `Valtan.patternbindings.json`, `Valtan.patterneffectcues.json` |
| Client authoring/read view | `CValtanPatternTree` canonical load -> Workbench/Boss Tool inventory |
| Server gameplay 소비 | publisher -> `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` -> `CGameplayCatalog/CGameRoom/CValtanBrain` fixed tick |
| Client presentation 소비 | generated binding/cue Product -> `CValtan` Animation/Effect/Sound/Shake 표현 |

생성 Product는 read-only다. Workbench Save가 `patternbindings`나 `patterneffectcues`를 직접
교체하는 경로는 만들지 않는다.

### 현재 구현 완료 기반

- `CActionCompositionWorkbench`의 독립 ImGui window, Pattern/Preview/Sequencer/Details/
  Resources/Session/Boss Pattern 창이 존재한다.
- `Create New Pattern`은 선택 Animation chain을 stable Pattern/Stage/action/occurrence ID를 가진
  `MANUAL_SERVER_AUDITION`으로 승격한다.
- Animation Sequence Replace/Append, occurrence reorder/trim/duplicate/delete가 존재한다.
- Effect invocation add/update/remove와 Sound 별도 owner add/edit/remove가 존재한다.
- Stage Duration과 trailing gap, manual Pattern의 `ACTIVE/WINDUP/GROGGY/WAIT` Stage
  insert/move/remove가 존재한다.
- Server Collider/hit schedule, push/knockdown, grab release speed/duration/yaw 수치 편집이 존재한다.
- 기존 `COUNTER_HIT -> GROGGY`는 `VALTAN_COUNTER_WINDOW_EDIT`으로 typed 편집할 수 있다.
- `CActionCompositionGraphModel`과 native ImGui canvas가 default/selected/maximum path,
  deterministic node/edge layout, selection과 preview route를 제공한다.
- Save는 gameplay/presentation source -> Product projection -> canonical reload를 거치며,
  Server playback은 exact active revision으로 제한된다.

### 현재 미완료 delta

- canonical Pattern graph의 wire는 읽기 전용이다. edge 클릭은
  `m_BossPatternOutcomeOverrides`를 바꾸는 preview route 선택이며 JSON branch를 변경하지 않는다.
- 기존 canonical Pattern은 Stage add/remove/reorder가 제한된다. manual audition만 선형 topology를
  편집할 수 있다.
- `VALTAN_TRASH` 자체가 manual audition으로 분류돼도 `STEP_08 TIMEOUT -> RUSH_MISS`가 vector의
  즉시 다음 `CATCH_COUNTER`를 건너뛰므로 현재 linear topology gate가 insert/move/remove를 거부한다.
  버러지 구조 편집은 branch-aware transaction 없이 기존 manual Stage API를 넓혀 사용하지 않는다.
- `COUNTER_HIT` 이외의 `ANY_PLAYER_GRABBED`, `ALL_PLAYERS_GRABBED`,
  `NAVIGATION_BLOCKED`, `TIMEOUT` branch를 Blueprint에서 연결·변경하는 typed writer가 없다.
- grab release 편집은 기존 `RELEASE_GRABBED_PLAYERS` action의 speed/duration/yaw 수치에 한정된다.
  새 capture/grab/release action topology 생성은 아직 지원하지 않는다.
- `Duplicate Stage Bundle`과 dependency-aware delete가 없다.
- Preview/Sequencer transport, Details/Blueprint/Session Save, Resources/Session/Boss Tool Server Play가
  중복돼 UI 책임이 분산돼 있다.
- Resource와 graph 성능은 source-token 회귀 검사가 있으나 warm-frame disk I/O 0을 증명하는
  실행형 counter harness가 부족하다.
- Boss audition Flow graph v2의 비선형 graph cursor는 Server에서 아직 실행되지 않는다.
  이 항목은 Pattern 내부 Blueprint와 별도 작업이다.

Boss audition Flow의 현재 단계는 `G01 graph v2 저장 기반 완료 / G02 Server graph cursor 미구현 /
G03 node-card 저작 UI 부분 구현`이다. 비선형·watchdog·finite traversal draft는 저장할 수 있어도
현재 Server Apply는 fail-closed다.

### 현재 `VALTAN_TRASH` 정본

```text
STEP_01 -> STEP_02 -> STEP_03 -> STEP_04 -> STEP_05 -> STEP_06
                                                          |
                                                          v
                                                    STEP_07 WINDUP
                         +--------------------------------+------------------+
                         | COUNTER_HIT                                       | TIMEOUT
                         v                                                   v
                      GROGGY                                               STEP_08
          release + groggy start/loop/end                         BOX CAPTURE / LEFT_HAND
                         |                                                   |
                        END                         +-------------------------+------------------+
                                                    | ANY_PLAYER_GRABBED      | NAV_BLOCK/TIMEOUT
                                                    v                         v
                                             CATCH_COUNTER                RUSH_MISS
                                                    |                         |
                                                    v                        END
                                             CATCH_PRE_IMPACT
                                                    |
                                      +-------------+---------------+
                                      | ALL_PLAYERS_GRABBED          | TIMEOUT
                                      v                              v
                                EXECUTE_TAIL                     CATCH_SLAM
                                      |                              |
                                     END                            END
```

- 실제 Counter window는 포획 뒤의 `CATCH_COUNTER`가 아니라 포획 전 `STEP_07`이다.
- `STEP_07 COUNTER_HIT -> GROGGY`, `TIMEOUT -> STEP_08`은 이미 제품 정본에 존재한다.
- GROGGY animation은 `start 1833 + loop 600 + end 2000 = 4433 ms`다.
- `RUSH_MISS`와 `CATCH_SLAM`은 terminal이다. 재충전·재돌진 branch는 아직 없다.
- `VALTAN_TRASH_CATCH_*`는 subgraph가 아니라 별도 `AUDITION_ONLY` Pattern이다.

첫 Blueprint 저작 수직 슬라이스는 기존 `STEP_07 -> GROGGY`의 편집 UX를 닫는다. Counter 위치를
`CATCH_COUNTER`로 옮기는 것은 단순 UI 변경이 아니라 gameplay source topology migration이므로
별도 콘텐츠 Gate에서만 수행한다.

## G01. ImGui 책임을 한 곳씩만 남긴다

### 최종 화면

```text
┌ Composition Patterns ────┐  ┌ Composition Pattern Blueprint ────────┐
│ Pattern / Stage 선택      │  │ 내부 Stage와 outcome 분기             │
│ Create New Pattern        │  │ 연결·변경은 지원된 typed pin만         │
└───────────────────────────┘  └───────────────────────────────────────┘

┌ Composition Sequencer ──────────────────────────────────────────────┐
│ [Play] [Pause] [Stop] [Restart] [Loop]  Preview Route [Counter ▼] │
│ Stage | Animation | Effect | Sound | Logic | Collider | Camera     │
└──────────────────────────────────────────────────────────────────────┘

┌ Composition Details ─────┐  ┌ Composition Resources ────────────────┐
│ 선택 block/edge의 수치    │  │ 선택 owner의 의미 있는 catalog만      │
│ ms, m, m/s, degree        │  │ Animation / Effect / Sound / Camera   │
└───────────────────────────┘  └───────────────────────────────────────┘

┌ Composition Session / Validation ──────────────────────────────────┐
│ Draft/Product/Server revision                                      │
│ [Validate & Save Pattern] [Save Sound] [Publish/Reload]            │
│ [Open Boss Tool]                                                    │
└─────────────────────────────────────────────────────────────────────┘
```

### 단일 책임

- 재생 transport는 Sequencer 한 곳만 소유한다. Preview 창은 viewport와 owner 상태만 표시한다.
- Pattern Save/Publish는 Session 한 곳만 소유한다. Details와 Blueprint의 Save 버튼은 제거하고
  dirty 표시와 `Focus Session`만 제공한다.
- Server Complete/Restart/Next는 Boss Tool만 실행한다. Session은 exact revision 상태와
  `[Open Boss Tool]` deep-link만 소유한다.
- Blueprint node/edge 클릭은 선택만 바꾼다. preview route 변경은 명시적인
  `Preview This Outcome` 명령으로 분리하고 항상 `PREVIEW ONLY / bytes unchanged`를 표시한다.
- Details는 숫자만 편집한다. Stage Role, branch target, Counter outcome 같은 구조 편집은
  Blueprint로 이동한다.
- 각 독립 window는 `ImGuiCond_FirstUseEver` 기본 위치/크기만 사용하며 hard min/max와
  `AlwaysAutoResize`를 두지 않는다.

### 공통 선택

```text
COMPOSITION_SELECTION
  patternId
  stageId
  actionId
  owner
  stableOccurrenceId 또는 branch stable identity
```

Blueprint node, edge, Sequencer block, Details와 Resources가 같은 stable selection을 소비한다.
선택은 pointer나 vector index를 보존하지 않고 canonical/draft generation이 바뀌면 다시 resolve한다.

## G02. Blueprint capability와 typed pin 계약

Graph node마다 다음 capability를 계산해 표시한다.

```text
READ
SELECT_ROUTE
CONNECT
RETARGET
DISCONNECT
ADD_STAGE
DUPLICATE_STAGE_BUNDLE
DELETE_STAGE
```

- capability는 UI가 추측하지 않고 gameplay owner가 exact reason과 함께 반환한다.
- derived `TIMEOUT` edge는 독립 JSON row가 아니므로 writer가 없으면 read-only다.
- target 후보는 같은 Pattern의 stable `actionId`로 resolve하며 `stageId`, 배열 순서, pointer를
  branch 저장 ID로 사용하지 않는다.
- 첫 writer는 `COUNTER_HIT`만 지원한다. 다른 outcome pin은 보이되 disabled reason을 표시한다.
- 임의 `Script`, packet/socket node, 자유 문자열 action, 무제한 cycle/back-edge는 제공하지 않는다.
- graph node 위치는 session-local ImGui 상태다. 별도 layout JSON이나 runtime 정본으로 저장하지 않는다.

지원된 pin drag는 JSON을 직접 수정하지 않고 다음 순서를 사용한다.

```text
drag source outcome pin
-> filtered target 후보 표시
-> stable source/target identity 재확인
-> owner typed draft transaction
-> candidate graph pure projection
-> dependency validation
-> draft commit
-> Blueprint/Sequencer/Details generation 갱신
```

실패하면 draft와 이전 graph snapshot을 그대로 유지하고 exact reason을 edge inspector와 Session에
동시에 표시한다.

## G03. Counter -> Groggy 첫 vertical slice

기존 `CBalanceTool::Get_ValtanCounterWindowDraft`와
`Set_ValtanCounterWindowDraft`를 재사용한다. 두 번째 Counter writer를 만들지 않는다.

```text
[STEP_07 WINDUP]
  COUNTER_HIT o----------------> [GROGGY]
  TIMEOUT     o----------------> [STEP_08]
```

### Blueprint가 소유할 항목

- Counter Enabled/Disabled
- `COUNTER_HIT` target connect/retarget/disconnect
- 같은 Pattern의 뒤쪽 `GROGGY` Stage 후보 필터
- 구조 변경 preview와 validation 결과

### Details가 계속 소유할 수치

- Counter window Stage duration
- counter proxy `BOSS_LOCAL` forward/right offset과 radius
- 선택 GROGGY Stage duration/gap
- GROGGY animation start/loop/end occurrence timing
- Effect/Sound occurrence와 release speed/duration/yaw

### admission

- source Stage는 `WINDUP`이어야 한다.
- target은 같은 Pattern의 source보다 뒤쪽 `GROGGY` Stage/action이어야 한다.
- mutation 결과에는 enter/exit `boss.flag.counterable` event와 branch가 함께 존재해야 한다.
- 완전히 부재한 counter/groggy flag pair는 기존 writer가 함께 materialize할 수 있다. 한쪽만 있는
  pair, duplicate event, branch와 flag가 서로 다른 target을 가리키는 부분 상태는 거부한다.
- target GROGGY에는 mutation 후 groggy enter/exit flag pair가 존재해야 한다.
- cross-Pattern target, 자기 자신, earlier Stage와 duplicate `COUNTER_HIT`은 거부한다.

현재 `VALTAN_TRASH`의 실제 연결을 native fixture로 고정한다.

```text
STEP_07 COUNTER_HIT -> valtan.sequence.center-trash-rush-if.groggy
STEP_07 TIMEOUT     -> valtan.sequence.center-trash-rush-if.step-08
```

## G04. Sequencer 구조 편집과 명시적 공백

### 현재 기능 보존

- `Create New Pattern`
- Sequence Replace/Append
- box drag/trim/duplicate/delete
- Stage duration과 trailing `HOLD_LAST_POSE` gap
- manual Pattern Stage insert/move/remove

### 공백 계약

```text
Stage Duration - Animation wall time = trailing gap
```

선택 Stage 뒤의 공백은 `Gap after sequence`로 조절한다. 두 animation 사이의 의미 있는 중간
공백은 anonymous blank key가 아니라 실제 `WAIT / GAP` Stage로 만든다.

```text
[MANUAL ACTIVE] -> [WAIT 500 ms] -> [NEXT ACTIVE]
```

첫 Counter slice의 `COUNTER_HIT`은 뒤쪽 GROGGY에 직접 연결한다. Counter 성공과 GROGGY 사이에
WAIT를 삽입하려면 WAIT 이후 GROGGY closure를 인정하는 별도 typed topology writer와 Server
validator가 필요하므로 G03 완료 범위로 주장하지 않는다.

WAIT는 새 Server enum이 아니다. 현재 계약대로
`stageKind=ACTIVE + sequenceRole=WAIT + animation NONE + no hit/motion/action`으로 저장한다.

### Duplicate의 두 의미

- `Duplicate Box`: Animation/Effect/Sound occurrence 하나와 새 stable occurrence ID만 복제한다.
- `Duplicate Stage Bundle`: Stage, action, animation/effect occurrence와 내부 branch reference를
  새 stable ID로 remap하는 구조 transaction이다.

Stage Bundle은 gameplay/presentation/Sound/Shake dependency preview가 모두 성공해야 한다.
Sound는 별도 CAS owner이므로 Pattern Save가 Sound Save까지 atomic했다고 표시하지 않는다.
Shake는 projector 생성 read-only dependency이므로 새 occurrence로 정확히 재투영할 수 있는
projector transaction이 없으면 Shake-linked bundle을 차단한다. 어느 dependency도 조용히 누락하거나
기존 occurrence ID를 새 Stage에 공유하지 않는다.

## G05. Grab/Rush outcome과 버러지 finite retry

Counter slice가 닫힌 뒤 다음 outcome을 하나씩 typed adapter로 연다.

| outcome | source precondition | target precondition |
|---|---|---|
| `ANY_PLAYER_GRABBED` | CAPTURE hit + attachment slot | captured-state consumer Stage |
| `ALL_PLAYERS_GRABBED` | grabbed players가 존재하는 Stage | execute 또는 slam Stage |
| `NAVIGATION_BLOCKED` | rush/navigation motion Stage | miss/recovery Stage |
| explicit `TIMEOUT` | source Stage clock | 같은 Pattern의 forward target 또는 terminal |

현재 canonical graph는 DAG이며 `CActionCompositionGraphModel`이 cycle을 거부한다. 버러지 재돌진은
숨은 loop count나 `RUSH_MISS -> STEP_07` back-edge로 만들지 않고 유한 Stage Bundle을 실제로
materialize한다.

```text
기존 RUSH_MISS
   -> RECHARGE_WAIT_02
   -> RETRY_WINDUP_02
   -> RETRY_RUSH_02
        | ANY_PLAYER_GRABBED -> 기존 CATCH 경로
        | NAV_BLOCK/TIMEOUT  -> RETRY_MISS_02
                                  -> RECHARGE_WAIT_03
                                  -> RETRY_WINDUP_03
                                  -> RETRY_RUSH_03
                                       | GRABBED -> 기존 CATCH 경로
                                       | MISS    -> RETRY_EXHAUSTED terminal
```

- retry 횟수는 authoring recipe 입력일 뿐 Product에는 실제 Stage/action/branch가 저장된다.
- 기존 `RUSH_MISS` stable ID는 rename하지 않는다. terminal target만 첫 새 retry Stage로 바꾸고,
  `RECHARGE_WAIT_02` 이후 새 Stage에만 새 stable ID를 발급한다.
- 원본 `STEP_06/07/08` Animation chain을 duplicate할 때 occurrence ID와 Effect/Sound dependency를
  새 stable ID로 remap한다.
- terminal policy와 maximum attempts가 명시돼야 한다.
- `CATCH_COUNTER`를 실제 Counter window로 바꾸는 요구는 이 retry와 섞지 않는다. 필요하면
  `STEP_07` counter flag/branch 제거, `CATCH_COUNTER` role/event/branch 추가, animation clock과
  Server harness를 포함한 별도 source migration으로 수행한다.

Boss Tool의 `ValtanBossAuditionFlows.json` graph v2 finite edge는 패턴 간 실행 순서다. 위 패턴 내부
retry를 Boss Flow로 우회 구현하지 않는다.

## G06. Resource와 owner별 편집 완결

### Context Resource Drawer

- Animation block 선택 시 Valtan Sequence catalog만 표시한다.
- Effect block 선택 시 referenced V1 Effect와 V2 document/group catalog만 표시한다.
- Sound block 선택 시 typed Sound event catalog만 표시한다.
- Camera는 실제 invocation/body consumer capability가 없으면 `INSPECT`로 표시한다.
- 닫힌 domain이나 전체 Resources tree를 재귀 순회하지 않는다.
- render frame에서는 filesystem, JSON parse, hash, canonical load를 실행하지 않는다.
- explicit Open/Refresh에서만 direct-child/background snapshot을 만들고 최신 generation만 commit한다.

### owner 경계

- Animation/Effect invocation/Camera timing은 presentation source를 편집한다.
- Sound는 별도 Sound source와 CAS를 사용한다.
- Effect element/material body는 Effect Tool deep-link가 소유한다.
- Collider/hit/push/grab은 gameplay source와 Server fixed tick이 소유한다.
- Effect geometry를 Collider로 추론하거나 Client PhysX를 판정 정본으로 표시하지 않는다.

## G07. Save, Publish, canonical reload, Boss replay

Session 창의 한 실행 흐름만 사용한다.

```text
owner draft
-> validate
-> shared writer admission
-> source stage/commit
-> generated Product projection
-> physical source/Product revision verify
-> CValtanPatternTree canonical reload
-> immutable runtime candidate
-> Server apply 또는 REENTRY_REQUIRED
-> exact Server-active revision
-> Complete Play / Restart / Queue Next
```

- Pattern draft를 Server에서 직접 재생하지 않는다.
- Pattern source와 Sound source의 dirty/commit/runtime apply 상태를 분리해 표시한다.
- `STALE_PRESERVED`와 `REJECTED`에서는 표시만 허용하고 Save/branch mutation/Server action을 막는다.
- CAS conflict, publisher 실패, canonical reload 실패, Server apply 실패는 성공으로 축약하지 않는다.
- commit하지 못한 owner는 byte-identical이어야 하고 이전 admitted view를 유지한다.
- Restart는 arena reset이 아니라 exact occurrence/revision CAS다. 벽 복구는 Boss Tool의
  `Fresh / Restore Arena` 명령으로 분리한다.

## G08. 변경 파일과 함수 책임

| 파일 | 이번 계획의 delta |
|---|---|
| `Client/Public/ActionCompositionWorkbench.h` | 단일 selection, explicit route-preview request, graph edge edit UI state, 중복 transport/Save 상태 정리 |
| `Client/Private/ActionCompositionWorkbench.cpp` | Sequencer 단일 transport, Details numeric-only, Session 단일 Save/Replay, Context Resource 경로 |
| `Client/Private/ActionCompositionWorkbench_Blueprint.cpp` | typed pin hit/drag/drop, capability/disabled reason, edge inspector, explicit preview route |
| `Client/Public/ActionCompositionGraphModel.h` | persisted edge identity와 UI capability를 구분한 pure projection 입력/출력 |
| `Client/Private/ActionCompositionGraphModel.cpp` | `VALTAN_TRASH` exact path, invalid target, candidate graph rollback projection |
| `Client/Public/BalanceTool.h` | 기존 Counter adapter 유지, outcome/Stage Bundle typed request와 capability query 추가 |
| `Client/Private/BalanceTool.cpp` | gameplay source candidate stage/validate/commit, stable ID remap, branch/dependency validation |
| `Client/Private/ValtanPatternFlowDocument.cpp` | Pattern 내부 graph와 무관. 이번 PR에서 수정하지 않음 |
| `Data/Valtan/Valtan.gameplay.json` | UI/transaction이 닫힌 뒤 버러지 retry 등 실제 content slice만 별도 commit |
| `Data/Valtan/Valtan.presentation.json` | duplicated Stage animation/effect occurrence의 stable join |
| `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json` | 별도 Sound owner transaction이 필요한 경우만 변경 |
| `Data/Animation/Authored/Valtan/Valtan.patternshakecues.json` | 직접 Save 금지. animation occurrence 변경 후 projector 결과와 dependency를 검증 |
| `Tools/ValtanPatternAuditionServiceHarness/...` | Client graph/transaction/canonical loader와 playback revision/protocol audition oracle |
| `Server/Private/ServerGameplayContractTests.cpp` | finite retry branch trace, exact attempt/terminal, collider·grab Server fixed-tick 결과 |
| `Client/Default/Client.vcxproj`, `.filters` | 새 native test/source가 생길 때만 물리 폴더 위치 그대로 등록 |

첫 Counter slice는 기존 Balance adapter를 재사용하므로 새 Manager나 runtime class를 만들지 않는다.
Stage Bundle처럼 cross-owner orchestration이 실제로 필요해질 때만 좁은 transaction class를 추가하고,
같은 변경에서 native rollback test와 project/filter 등록을 포함한다.

## G09. 자동 검증

### focused contract

```powershell
python -B -m unittest `
  Tools.ValtanPipeline.test_valtan_counter_authoring_contract `
  Tools.ValtanPipeline.test_valtan_manual_stage_authoring_contract `
  Tools.ValtanPipeline.test_action_composition_manual_stage_topology_contract `
  Tools.ValtanPipeline.test_action_composition_sequence_identity_contract `
  Tools.ValtanPipeline.test_action_composition_sequencer_occurrence_timing_contract `
  Tools.ValtanPipeline.test_action_composition_effect_invocation_contract `
  Tools.ValtanPipeline.test_action_composition_sound_owner_contract `
  Tools.ValtanPipeline.test_action_composition_workbench_regression_oracles `
  Tools.ValtanPipeline.test_valtan_animation_chain_promotion `
  Tools.ValtanPipeline.test_valtan_animation_pattern_create_service `
  Tools.ValtanPipeline.test_valtan_animation_pattern_create_workbench_contract `
  Tools.ValtanPipeline.test_valtan_pattern_master_v2 `
  Tools.ValtanPipeline.test_valtan_manual_stage_topology_pipeline `
  Tools.ValtanPipeline.test_valtan_canonical_typed_patch_transaction `
  Tools.ValtanPipeline.test_valtan_pattern_tree_transaction_read_gate `
  Tools.ValtanPipeline.test_valtan_source_product_ownership_contract `
  Tools.ValtanPipeline.test_valtan_writer_admission `
  Tools.ValtanPipeline.test_valtan_gameplay_publisher_writer_admission `
  Tools.ValtanPipeline.test_valtan_boss_tool_contract `
  Tools.ValtanPipeline.test_valtan_boss_tool_pattern_flow_contract
```

source-token PASS만 완료 증거로 사용하지 않는다.

### native oracle

`ActionCompositionGraphModelContractTests`와 Valtan audition harness에 다음을 추가한다.

- 현재 `VALTAN_TRASH`의 STEP_07 Counter/Groggy와 Timeout/Rush exact stable ID
- graph edge 선택과 Sequencer selected path 일치
- invalid/cross-Pattern target, duplicate ID, dangling target, cycle 거부
- 실패 시 이전 graph/draft/source bytes 보존
- Stage Bundle stable ID remap과 모든 occurrence/branch dependency
- finite retry의 정확한 시도 횟수와 terminal trace
- unsupported cyclic retry와 attempt cap 위반의 Server reject/rollback
- Save 후 canonical loader가 같은 generation을 읽는지
- exact revision Complete/Restart/Next와 stale reject rollback

finite retry와 grab/rush branch의 실제 실행 trace는 Client audition harness만으로 끝내지 않고
`ServerGameplayContractTests.cpp`의 fixed-tick contract에도 고정한다.

실행 증거는 아래 표준 Debug Core가 같은 HEAD에서 harness를 build한 뒤 실행한 receipt를 우선한다.
실패 triage를 위해 직접 다시 실행할 때도 그 Debug Core build가 성공한 동일 revision의 binary만
사용한다. 기존 `Bin/Debug` 실행 파일을 build 전에 실행해 PASS 증거로 사용하지 않는다.

```powershell
& .\Tools\ValtanPatternAuditionServiceHarness\Bin\Debug\ValtanPatternAuditionServiceHarness.exe
```

### Resource/프레임 성능 oracle

native counter를 추가해 다음 값을 측정한다.

```text
warm idle 10,000 frames
scroll/resize 300 frames

directoryEntriesVisited = 0
filesOpened             = 0
jsonBytesRead           = 0
documentsParsed         = 0
hashBytes               = 0
graphRebuilds           = 0 unless generation changed
```

닫힌 10,000-file fixture는 Tool Open 때 0개를 방문해야 한다. explicit folder Open은 direct child만
background index하고 stale worker generation은 commit하지 않는다.

### publisher와 canonical admission

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Test-ValtanPatternMaster.ps1
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate -SkipValtanSplitProjection
python -B Tools/ValtanPipeline/validate_valtan_requested_pattern_coverage.py
```

### 표준 빌드

동일 최종 commit과 clean worktree에서 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Product
```

Server fixed-tick, collider, protocol 또는 presentation/render 광역 계약을 변경한 slice는 추가로:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
```

마지막에 변경 JSON parse, Client/Harness project/filter XML parse와 `git diff --check`를 실행한다.

## G10. 사용자 수동 검증

자동 검증 뒤 사용자가 `Server + Client` profile을 `Ctrl+F5`로 실행한다.

```text
1. F1 baseline FPS 기록
2. Action Composition의 독립 window 이동/resize
3. VALTAN_TRASH 선택
4. STEP_07 COUNTER_HIT -> GROGGY, TIMEOUT -> STEP_08 확인
5. edge/node/Sequencer block 선택 동기화 확인
6. VALTAN_TRASH STEP_07의 Counter enable/duration/proxy 수치와 GROGGY trailing gap 수정
7. 별도로 새 manual-linear audition Pattern에서 WAIT Stage insert/move/remove 확인
8. 저장 전 Server active Pattern이 변하지 않는지 확인
9. Validate / Save / Publish / canonical reload
10. 창 재오픈 뒤 값 유지 확인
11. Boss Tool에서 동일 revision Complete Play/Restart
12. Counter 성공 시 Groggy start/loop/end 재생 확인
13. stale/reload failure에서 Save/Play 차단과 이전 view 보존 확인
```

사용자의 서면 판정 전에는 FPS, resize, 시각적 Animation/Effect/Sound 결과를 `visual PASS`로
기록하지 않는다.

## G11. 커밋과 PR 분리

1. UI 단일 책임과 explicit preview route
2. Counter typed pin authoring과 native graph oracle
3. Resource/frame zero-I/O oracle
4. Stage Bundle transaction과 rollback
5. Grab/Rush outcome adapter
6. 버러지 finite retry content
7. Save/revision/Boss replay 통합 검증

각 커밋은 자신이 변경한 source/schema/project 등록/harness/PLAN·RESULT를 함께 포함한다. 버러지
retry와 Boss audition Flow graph runtime, Portal, Pizza, Silence, 3연속 Counter를 하나의 PR에
묶지 않는다.

## Milestone별 완료 조건

### Milestone A. Counter pin UX

- 기존 `STEP_07 COUNTER_HIT -> GROGGY`가 Blueprint에서 connect/retarget되고 기존 typed writer와
  native graph oracle이 통과한다.
- generic outcome wire와 버러지 retry는 이 Milestone 완료 조건이 아니다.

### Milestone B. branch-aware Stage Bundle

- gameplay/presentation/Sound/Shake dependency를 보존한 duplicate/delete/branch transaction과
  byte-identical rollback이 통과한다.
- `ANY_PLAYER_GRABBED`, `NAVIGATION_BLOCKED`, explicit `TIMEOUT`은 각자 admission이 닫힌 것만
  capability를 얻는다.

### Milestone C. 버러지 finite retry 콘텐츠

- 기존 `RUSH_MISS` identity를 유지한 채 새 retry Stage/action이 유한 DAG로 materialize된다.
- Server fixed-tick trace와 exact terminal policy가 native contract로 고정된다.

### 통합 완료 조건

- Blueprint wire 변경이 실제 gameplay source branch를 바꾸고 Preview-only 선택과 명확히 구분된다.
- Sequencer는 선택 route의 Animation/Effect/Sound/Logic/Collider/Camera를 같은 clock에 표시한다.
- Details는 numeric tuning만 소유하고 구조 편집을 중복하지 않는다.
- Save/Publish/Reload 후 actual canonical loader와 Server-active revision이 일치한다.
- 실패 시 draft/source/Product/previous admitted view의 rollback 계약이 증명된다.
- warm frame의 filesystem/JSON/hash/graph rebuild가 0이다.
- 현재 버러지 Counter/Groggy가 회귀하지 않고 finite retry는 실제 유한 Stage graph로 저장된다.
- Debug Core, Release Product, 필요한 FullDiagnostic과 사용자 수동 검증 결과가 RESULT에 분리된다.
