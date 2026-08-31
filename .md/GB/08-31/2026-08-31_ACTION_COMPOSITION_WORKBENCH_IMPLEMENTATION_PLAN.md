# Action Composition Workbench Pattern Blueprint 저작 구현 계획서

## 목표

> 2026-08-31 범위 정정: 이 문서는 더 이상 Blueprint UI 자체를 최종 목표로 삼지 않는다.
> 사용자가 실제로 만들려는 Valtan Pattern을 `Data -> Shared -> Server -> Client presentation ->
> Composition Sequencer -> 실행형 oracle`로 하나씩 닫는 구현 계획서다. UI shell, source JSON 추가,
> source-token test만으로 완료를 선언하지 않는다.

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

이번 구현의 필수 범위는 다음과 같다.

| 수직 슬라이스 | 완료 계약 |
|---|---|
| Counter Logic box | `VALTAN_TRASH`, `VALTAN_TRIPLE_COUNTER`의 exact Stage counter window, proxy offset/radius, success next-window/Groggy/Recovery와 timeout failure branch를 Sequencer box와 Box Detail에서 편집하고 Server fixed-tick으로 검증 |
| 입장·재생 admission | 오래된 파일 SHA/bytes/generation/inventory snapshot 때문에 Lobby entry·Save·Complete Play가 막히지 않게 제거. JSON/schema/stable ID/path 검증, actual canonical load, Server gameplay revision CAS, rollback은 유지 |
| 무력화 Pattern | `VALTAN_STAGGER_SLOT` 빈 topology에서 시작해 Server stagger gauge, damage contribution, depleted branch와 구형 Effect presentation을 구현 |
| 속박 Pattern | `VALTAN_BIND_SLOT`에서 random alive player를 Server가 lock하고 약 10m 상승, 5초 이동/스킬 제한, 안전한 복귀와 disconnect/death rollback을 구현 |
| 침묵 Pattern | `VALTAN_SILENCE_SLOT`에서 5초 Server skill-use reject와 Client HUD cooldown-mask presentation을 구현. UI mask는 권위가 아님 |
| 땅구르기 후 사자후 | `VALTAN_GROUND_ROAR`에서 boss-relative yaw `0/90/180/270`의 정확히 네 rock combat object를 생성하고 5000ms 뒤 explode Effect와 함께 despawn |
| Six Pizza | random alive target 기준 Server-locked facing을 sector root에 적용하고, 늦게 생성되는 모든 Effect Element도 같은 회전 root/누적 회전을 소비 |
| 버러지 Pattern | Counter success Groggy, timeout rush, capture, left-hand attach, all/any grabbed 분기, miss/retry terminal을 유한 graph로 구현 |
| 3연속 Counter | 세 Counter window를 실제 Stage box로 표시하고 success progress/failure/마지막 실패 전멸 분기를 Server에서 검증 |
| 잡기 후 날리기 | server hit collider만 capture 권위로 사용하고 `BossModel * LeftHandBone * localOffset` anchor, release velocity/duration/yaw를 Box Detail에서 편집 |
| Collider Trigger 저작 | 선택한 Collider box에서 exact start/lifetime, Circle/Ring/Cone/Box 크기, Boss Root/Left Hand/locked target anchor, Damage/Grab/Counter/Bind 계열 typed trigger를 편집한다. Effect geometry는 판정으로 승격하지 않고 Server fixed-tick 소비와 Client debug mirror를 같은 값으로 검증 |
| Portal | `VALTAN_WARP`의 정확히 8회 돌진을 16m, 회차 사이 1000ms gap으로 source/Product/Server/Composition timeline에 동일하게 표시 |
| V2 Group | Effect Tool V2 Group preview와 Composition exact binding이 같은 group/leaf runtime을 통해 실제 재생되고 cache refresh 후 즉시 restage |

사용자가 Animation Sequence를 찾아 Pattern slot에 넣는 선택 작업은 사용자 저작 범위다. 구현은 빈
Pattern slot, 실제 typed owner Save, Server 기능, Sequencer 가시화와 실패 rollback을 제공한다.

## G00. 현재 물리 폴더 기준선

### 저장소 상태

2026-08-31 현재 물리 폴더 실측 기준:

```text
canonical base : origin/main 8180fd2d6de6e45e6ab1a62a1d26a64fe0c301cc
target branch  : codex/simple-authoring-ux
shared worktree: 여러 수직 슬라이스의 tracked/untracked 변경이 공존하는 dirty 상태
LAN role       : server-host, Server + Client profile
```

현재 HEAD와 `origin/main`은 동일하지만 물리 worktree는 dirty다. 이 PLAN은 커밋 이력의 완료
주장이 아니라 현재 물리 source/data/consumer를 기준으로 한다. 사용자 소유
`Data/Effects/Authored/effect.valtan.sky-axe.active.effect.json`과 관계없는 untracked Map Product는
보존한다. 명시적 요청 전에는 자동 stage/commit/push하지 않는다.

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

## G03. Counter Success typed target과 3연속 Counter vertical slice

기존 `CBalanceTool::Get_ValtanCounterWindowDraft`와
`Set_ValtanCounterWindowDraft`를 재사용한다. 두 번째 Counter writer를 만들지 않는다.

```text
[WINDUP]
  COUNTER_HIT o----------------> [다음 WINDUP | GROGGY | RECOVERY]
  TIMEOUT     o----------------> [같은 Pattern의 뒤쪽 failure Stage]
```

### Blueprint가 소유할 항목

- Counter Enabled/Disabled
- `COUNTER_HIT` target connect/retarget/disconnect
- 같은 Pattern의 뒤쪽 `WINDUP`, `GROGGY`, `RECOVERY` success Stage 후보 필터
- exact `TIMEOUT` failure Stage/action 표시와 보존
- 구조 변경 preview와 validation 결과

### Details가 계속 소유할 수치

- Counter window Stage duration
- counter proxy `BOSS_LOCAL` forward/right offset과 radius
- 선택 GROGGY Stage duration/gap
- GROGGY animation start/loop/end occurrence timing
- Effect/Sound occurrence와 release speed/duration/yaw

### admission

- source Stage는 `WINDUP`이어야 한다.
- success/timeout target은 같은 Pattern의 source보다 뒤쪽 Stage/action이어야 한다.
- success target은 다음 Counter의 `WINDUP`, 반격 성공의 `GROGGY`, 마지막 정상 종료의
  `RECOVERY` 중 하나다. 다른 kind, cross-Pattern, backward, cycle target은 거부한다.
- mutation 결과에는 enter/exit `boss.flag.counterable` event와 branch가 함께 존재해야 한다.
- 완전히 부재한 counter/groggy flag pair는 기존 writer가 함께 materialize할 수 있다. 한쪽만 있는
  pair, duplicate event, branch와 flag가 서로 다른 target을 가리키는 부분 상태는 거부한다.
- target이 `GROGGY`일 때만 mutation 후 groggy enter/exit flag pair를 요구하거나 생성한다.
  다음 `WINDUP`과 `RECOVERY`에는 groggy flag를 생성하지 않는다.
- Counter disable은 `COUNTER_HIT`과 paired counterable flag만 제거하며 기존 `TIMEOUT` 흐름은
  보존한다.
- 자기 자신, earlier Stage와 duplicate `COUNTER_HIT`/`TIMEOUT`은 거부한다.

현재 `VALTAN_TRASH`의 실제 연결을 native fixture로 고정한다.

```text
STEP_07 COUNTER_HIT -> valtan.sequence.center-trash-rush-if.groggy
STEP_07 TIMEOUT     -> valtan.sequence.center-trash-rush-if.step-08
```

`VALTAN_TRIPLE_COUNTER`는 sealed legacy row를 단순 복사하지 않는다. pipeline, canonical Client
loader, typed draft owner와 Server Product admission에 남아 있는 `COUNTER_HIT -> GROGGY` 강제를
먼저 위 계약으로 일반화한 뒤 다음 finite graph를 split source로 승격한다.

```text
COUNTER_1 1800 ms
├─ COUNTER_HIT -> COUNTER_2
└─ TIMEOUT     -> FAIL_1 (18 m damage) -> COUNTER_2

COUNTER_2 1600 ms
├─ COUNTER_HIT -> COUNTER_3
└─ TIMEOUT     -> FAIL_2 (18 m damage) -> COUNTER_3

COUNTER_3 1400 ms
├─ COUNTER_HIT -> RECOVERY
└─ TIMEOUT     -> FAIL_3 (100 m wipe) -> RECOVERY

RECOVERY 1200 ms -> END
```

같은 transaction에서 `Valtan.gameplay.json`/`Valtan.presentation.json`에 managed pattern을 추가하고,
`Valtan.legacy-compatibility.json`의 sealed entry와 gameplay의 세 `REFERENCE_ONLY_LEGACY` reaction
layer를 제거한다. generated Encounter/binding/cue Product는 projector만 생성한다. 기존 Product가
실제로 사용하던 groggy start/loop/end clip과 첫 carrier cue의 의미는 보존하되 공격 animation을
복원했다고 주장하지 않는다.

버러지 retry는 back-edge를 추가하지 않는다. 현재 finite graph validator가 cycle을 의도적으로
거부하므로 Counter/Rush attempt를 stable Stage/action ID로 세 번 명시적으로 unroll하고 세 번째
miss만 terminal `RUSH_MISS`로 보낸다. 각 attempt의 capture, Counter success Groggy, all/partial
grabbed 분기는 Server fixed-tick oracle로 따로 검증한다.

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

### Effect Tool V2와 Composition Workbench의 공용 Group 파이프라인

이 절은 목표 계약이다. 현재 source에 V2 leaf/group catalog, Group editor, Stage binding append와
Arena Clone 재생의 일부가 이미 있어도 아래 exact occurrence 편집과 중복 admission, cache refresh,
Server generation 경계가 모두 검증되기 전에는 공용 파이프라인 완료로 기록하지 않는다.

| owner | 소유하는 정본 | 소유하지 않는 것 |
|---|---|---|
| Effect Tool V2 | `Data/Effects/V2/Authored/*.effectv2.json` leaf body와 `Data/Effects/V2/Groups/*.effectv2group.json`의 `groupId`, group duration, ordered `children`, child-local start/duration/stop/offset/yaw/scale | Pattern/Stage, animation occurrence, Server stage clock |
| Composition Workbench | `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`의 exact binding occurrence와 선택 Stage/animation box 기준 start, anchor/follow/rotation/placement | leaf body, group child 배열, Effect material/particle 수치 |
| Product runtime | admitted binding → group → authored leaf를 펼쳐 현재 Server stage/clip clock에서 재생 | authoring draft 해석, Pattern JSON의 복제된 group body |

저장 그래프는 다음 한 방향만 사용한다.

```text
Effect Tool V2
  Authored leaf body
  + Group body / ordered children
             |
             v groupId reference
Composition Workbench
  BOSS_VALTAN.effectv2bindings.json exact occurrence
             |
             +-> local Arena Clone authoring preview
             `-> publish/build + world-entry presentation generation
                    -> saved Server-active Pattern playback
```

- Composition은 group을 선택해도 child를 `Valtan.gameplay.json`, `Valtan.presentation.json` 또는
  다른 Pattern JSON에 복사하지 않는다. group body/children의 변경은 Effect Tool V2 Save가
  계속 소유하고 Composition에는 같은 `groupId` reference만 남는다.
- 기본 Resource UX는 **groups-first**다. `boss.valtan.*` V2 Group을 기본 목록으로 제공하고,
  direct authored leaf는 `Advanced / Direct Leaves`를 명시적으로 연 경우에만 같은
  `boss.valtan.*` scope에서 선택할 수 있다. Esther/NPC/다른 owner leaf를 BOSS_VALTAN binding에
  편의상 노출하지 않는다.
- group binding이 펼치는 child leaf와 direct leaf가 같은 clock에 중복되면 저장을 거부한다.
  비교 clock은 같은 stage/clip owner에서
  `group binding startMs + child startMs == direct binding startMs`이고 effectId도 같은 경우다.
  UI append만이 아니라 C++ catalog cross-validation, Python V2 validator와 Product admission이
  같은 불변식을 검사해야 한다.
- `Add to selected animation box`는 선택 box의 stable clip occurrence와 Stage-local start를
  캡처해 group binding을 붙인다. 사용자가 다른 Stage/box를 선택한 뒤 command가 처리되거나
  baseline catalog generation이 바뀌면 stale command를 거부한다. vector ordinal은 binding
  identity가 아니다.
- Add/Move/Delete/Duplicate는 선택한 exact binding row 하나만 full-row baseline/CAS로 변경한다.
  Delete는 group body나 child leaf 파일을 지우지 않고 binding만 제거한다. Duplicate는 같은
  `groupId`와 placement를 새 exact occurrence로 복제하되 위 expanded-child 중복 validator를
  통과해야 한다.
- Composition timeline의 V2 Group block은 point처럼 0-width로 그리지 않고 group child의
  최대 end까지 display span을 넓힌다. 이 값은 표시/selection span일 뿐
  `Stage.durationMs`, group document의 semantic duration, Server action duration을 자동으로
  늘리거나 줄이지 않는다.
- successful Add/Move/Delete/Duplicate 뒤에는 Effect V2 catalog snapshot revision과 Product
  runtime binding/group/document cache를 함께 invalidation하고, 현재 Arena Clone의 같은 Pattern
  path/playhead를 새 snapshot으로 즉시 restage한다. 사용자가 Client를 재시작해야만 local preview가
  바뀌는 흐름은 authoring preview 계약이 아니다.

Arena Clone과 Product Server playback은 같은 Effect V2 runtime renderer를 재사용해도 admission
source가 다르다.

```text
local Arena Clone
  current authoring catalog snapshot + local Pattern draft
  -> immediate cache refresh/restage
  -> Server state와 Server-active gameplay revision을 바꾸지 않음

saved Product / Server Valtan
  현재 typed physical presentation closure
  + Server-active gameplay revision
  -> canonical reload 뒤 Complete Play / Restart
```

따라서 Effect V2 catalog reload는 authoring snapshot과 Arena Clone을 즉시 갱신한다. 입장 시점에
캡처한 파일 byte count/SHA/inventory generation을 이후의 Save·Complete Play 차단 조건으로 사용하지
않는다. 재생 시점에는 현재 binding/group/leaf를 typed parse/validate하고, 같은 read transaction 안에서
파일이 바뀌었으면 candidate를 버리고 이전 admitted view를 유지한다. Server가 고정하는 것은 gameplay
revision과 exact Pattern occurrence이며 presentation 파일의 과거 SHA가 아니다.

이 정정은 검증을 없애는 것이 아니다. malformed JSON/schema/stable ID/path, missing group/leaf,
duplicate expanded occurrence, canonical graph load 실패는 계속 거부한다. 단, 실패 이유는 사용자가
고칠 수 있는 한 줄 상태와 owning Tool deep-link로 표시하고 raw endpoint/WSA/SHA dump를 기본 UI에
노출하지 않는다.

### Pattern Save와 기존 Sound debt의 no-new-debt 검증

Pattern Sound가 없는 animation/Stage를 저장할 때, 전혀 바뀌지 않은 다른 Pattern의 기존 Sound
timing debt를 전체 graph strict re-admission해서 Save를 막지 않는다. 다만 Sound row의
`patternId/stageId/actionId/clipOccurrenceId` exact identity는 항상 검증한다.

- candidate가 해당 Sound Stage의 action ID, Stage duration 또는 animation occurrence timing/shape를
  바꾸지 않았으면 기존 timing window debt는 이번 transaction의 변경분이 아니므로 strict timing
  재검증을 생략한다.
- candidate가 Sound가 붙은 Stage의 위 필드 중 하나를 바꾸거나 Stage/occurrence를 삭제·중복시키면
  strict Sound timing admission을 반드시 다시 실행하고 실패 시 Pattern Save 전체를 거부한다.
- 이 정책은 legacy debt를 정상으로 승인하는 fallback이 아니다. 이번 변경이 debt를 만들거나
  확장하지 않는 `no-new-debt` 경계이며, Sound owner Save/CAS와 runtime apply는 계속 별도다.

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

### G08-P0. Six Pizza 고정 회전 기준 수직 슬라이스

Blueprint/Effect Details의 첫 실제 콘텐츠 연결은 `VALTAN_SIX_PIZZA_106` composite Effect
한 건으로 닫는다. Counter/Grab branch writer와 같은 커밋에 섞지 않는다.

```text
Server pattern start
-> random alive target lock
-> authored landing center에서 target까지의 facing yaw lock
-> Product cue anchor arena.center.facing
-> composite cue root = landing center translation + locked facing rotation
-> sector / landing / impact child Element가 같은 root를 공유
```

- gameplay source의 `LOCK_RANDOM_ALIVE_ON_START`, `LOCK_FACING_ON_START`,
  `LEAP_TO_ANCHOR`, `moveToAnchorBeforeTakeoff`는 이미 정본이며 변경하지 않는다.
- presentation source의 Six Pizza cue만 `pattern.target.snapshot`에서
  `arena.center.facing`으로 바꾼다. generated `Valtan.patterneffectcues.json`은 publisher만
  다시 만든다.
- Workbench Effect Details는 선택 anchor의 위치 기준과 회전 기준을 read-only 문장으로
  명시한다. `arena.center.facing`은 `Authored Landing Center / Server Locked Pattern Facing`,
  `pattern.target.snapshot`은 `Target Snapshot Position / Player Snapshot Yaw`다.
- Effect body의 `groupId`는 목록/Solo 단위이고 transform parent가 아니다.
  `transformInheritance`와 particle-only yaw를 공통 피자 root로 사용하지 않는다.
- 이 P0의 "마지막 rotation"은 패턴 시작에 Server가 잠근 고정 facing이다. 시간에 따라
  회전한 sector의 누적 최종각을 landing이 샘플하는 root rotation track은 별도 typed
  presentation schema/runtime/harness slice로 남긴다.

종료 증거는 source -> publisher Product -> actual `CValtanPatternTree` admission,
Server locked facing 계약, Client arena-center matrix helper, 늦게 시작하는 composite Element의
동일 root 합성과 실패 시 이전 matrix 보존이다. 사용자의 Client 육안 판정 전에는 visual PASS로
기록하지 않는다.

## G08-A. stale SHA/bytes 차단 제거와 실제 admission

### 보존할 검증

```text
current typed source/Product parse
-> schema/stable ID/reference/path validate
-> candidate stage
-> same-read still-current 확인
-> canonical graph admission
-> commit 또는 이전 admitted view 보존
```

- `NetworkManager` world entry는 Server가 승인한 gameplay revision과 protocol만 pin한다.
- `ValtanPresentationGenerationAdmission`은 과거 receipt의 byte count/SHA를 현재 파일과 비교해
  entry/replay를 막지 않는다. 현재 physical closure를 typed parse한 결과만 candidate로 사용한다.
- `BossTool`, Composition, Lobby의 기본 UI에서는 raw SHA, WSA, packet, endpoint provenance dump를
  제거한다. 실패는 `Pattern data invalid`, `Server not ready`, `Reload required`처럼 한 줄로 표시한다.
- `Validate_StillCurrent`, exact Server revision CAS, partial-write rollback, canonical loader 실패 차단은
  제거하지 않는다. "diagnostics 삭제"를 검증 삭제로 해석하지 않는다.

실행 oracle은 (1) presentation 파일이 world entry 뒤 합법적으로 교체돼도 gameplay revision이 같고
현재 closure가 유효하면 reload/replay 가능, (2) malformed candidate는 이전 view 보존, (3) Server
gameplay revision stale은 reject, (4) mid-read replacement는 commit하지 않음을 각각 고정한다.

## G08-B. Counter, 버러지, 3연속 Counter

Sequencer의 `Logic` lane에는 generic 텍스트가 아니라 exact Stage-owned Counter box를 그린다.

```text
Counter box
  stableId      = <pattern>/<stage>/logic/counter
  startMs       = Stage start
  durationMs    = Stage duration
  proxy.anchor  = BOSS_LOCAL
  proxy.offset  = x/z
  proxy.radiusM = positive range
  success       = COUNTER_HIT -> GROGGY/progress
  timeout       = TIMEOUT -> authored failure target
```

- Logic `+`는 선택 Stage에 Counter를 materialize하고 기본 proxy를 명시적으로 저장한다.
- box body drag는 occurrence 시작을 바꾸지 않는다. Counter는 Stage clock 소유이므로 right-trim 또는
  Box Detail duration이 Stage duration을 바꾼다.
- Box Detail은 enable, proxy offset X/Z, radius, success/failure target을 보여준다. `Delete`는 해당
  Counter flag/proxy/branch만 제거하고 Stage/Animation을 지우지 않는다.
- `VALTAN_TRASH`는 STEP_07 success Groggy와 timeout capture/rush를 보존하면서 finite miss/retry를
  실제 Stage bundle로 저장한다. capture 후 `ANY_PLAYER_GRABBED`, `ALL_PLAYERS_GRABBED`,
  counter success release, timeout execute/slam을 Server trace로 검증한다.
- `VALTAN_TRIPLE_COUNTER`는 세 Counter Stage를 각각 별도 box로 표시한다. 성공은 다음 window/progress,
  실패는 authored failure, 마지막 실패는 wipe/terminal로 구분한다. 한 bool을 세 구간에 공유하지 않는다.

## G08-C. 무력화·속박·침묵 Pattern

세 Pattern은 source에 빈 5000ms Stage와 `animation.mode = NONE`으로 먼저 존재한다. 빈 slot 자체를
완료로 기록하지 않고 다음 gameplay consumer가 연결된 뒤에만 Pattern slice가 완료된다.

### 무력화

```text
Pattern start -> stagger shield/gauge active
eligible Server damage -> gauge decrement
gauge <= 0 -> STAGGER_SUCCESS/GROGGY branch
timeout      -> authored failure/terminal
Client       -> snapshot gauge + surrounding sphere Effect presentation
```

게이지와 damage contribution은 Server Balance/gameplay owner가 소유한다. ImGui는 값과 branch를
편집하고 HUD/Effect는 snapshot을 표시할 뿐 로컬 damage로 감소시키지 않는다.

### 속박

```text
random alive target lock
-> Server movement/skill restraint
-> target Y + 10m presentation/authoritative pose policy
-> 5000ms hold
-> navigation-projected release position
```

death, disconnect, level transfer, Pattern cancel에서 restraint와 attachment를 반드시 해제한다. Client
transform 강제만으로 구현하지 않는다.

### 침묵

```text
Pattern silence window 5000ms
-> Server rejects skill commands for affected player(s)
-> snapshot status end tick
-> Client quick-slot cooldown mask presentation
```

HUD mask red/height는 presentation이며 Server silence authority를 대신하지 않는다. 종료·cancel·re-entry
후 status가 남지 않는 native oracle을 둔다.

## G08-D. 땅구르기 후 사자후 네 방향 돌

`VALTAN_GROUND_ROAR`의 typed Server action이 exact 네 combat object를 생성한다.

```text
pattern/boss transform at spawn
  + yaw 0   -> rock occurrence 0
  + yaw 90  -> rock occurrence 1
  + yaw 180 -> rock occurrence 2
  + yaw 270 -> rock occurrence 3

each rock: visible active Effect -> lifetime 5000ms -> reliable explode event -> hit Effect -> despawn
```

- 네 occurrence는 vector index가 아니라 stable occurrence ID를 가진다.
- boss yaw를 두 번 더하거나 world cardinal로 고정하지 않는다.
- 사용자가 지정하지 않은 damage/radius를 임의로 추가하지 않는다. visual-only typed object가 불가능하면
  실제 기존 authored 근거를 사용하고 PROJECT_TUNED 값으로 명시한다.
- rollback/Pattern cancel/room teardown에서 남은 rock을 모두 제거한다.
- Sequencer Effect/Logic lane에는 네 spawn과 5000ms lifetime/explosion을 한 clock으로 표시한다.

## G08-E. Six Pizza 회전 root

기존 `arena.center.facing` 고정 facing은 첫 단계일 뿐이다. 실제 완료 계약은 다음과 같다.

```text
random alive player snapshot
-> Server locks pizza root yaw
-> sector rotation track evaluates root yaw(t)
-> every early/late Element evaluates Transform = arena center * root yaw(t/terminal) * local element
-> landing/impact uses the same locked/final root, never raw player yaw independently
```

Effect element별 yaw 복사로 우회하지 않고 composite invocation의 typed rotation root 하나를 사용한다.
data -> snapshot -> matrix -> early sector spawn -> late element spawn native test에서 동일 root identity와
각도를 검증한다. anchor/facing source가 없으면 fallback 회전으로 정상처럼 재생하지 않는다.

## G08-F. 잡기 후 날리기와 left-hand anchor

- capture query는 Server hit collider만 사용한다. 노란 Effect geometry나 Client PhysX는 잡기 판정이 아니다.
- attachment transform은 `BossWorld * LeftHandBoneWorld/Model * authoredLocalOffset`의 한 helper를
  Server snapshot과 Client presentation이 같은 stable attachment slot 의미로 소비한다.
- Box Detail에는 collider shape/offset/range, attachment slot/local offset, release speed, duration,
  yaw, knockdown/down duration을 표시한다.
- Counter/cancel/death/disconnect는 release와 상태 rollback을 수행한다.
- release velocity/rotation은 Pattern source와 Server action이 소유하며 Client가 임의 계산하지 않는다.

## G08-G. Portal 8회 돌진

정본 Pattern은 legacy 별도 portal Pattern이 아니라 `VALTAN_WARP`다.

```text
STEP_02 ... STEP_09 (8 rush stages)
  retargetDelay = 500ms
  distance      = 16m
  speed         = 20m/s
  travel        = 800ms
  postRushGap   = 1000ms
  stage clock   = 2300ms
```

각 Stage의 authored motion samples는 500..1250ms, 50ms 간격으로 16개이며 기존 8m 구간을 16m로
늘린다. Composition Logic lane은 각 Stage마다 `Target Delay`, `Rush`, `Gap` 세 box를 보여주며
generic Motion 한 줄로 축약하지 않는다. source -> publisher Product -> Server trajectory/retarget trace ->
Composition exact box count/timing이 같은 값을 가져야 완료다.

## G08-H. Effect V2 Group 실제 재생

```text
Effect Tool V2 Group Save
-> groupId + ordered children admitted snapshot
-> Composition exact binding occurrence
-> Arena Clone Sync_StageAuthoring
-> shared V2 runtime expand group children
-> renderer spawn/update/stop

Product playback
-> generated binding
-> Sync_Stage
-> the same expand/render path
```

단순 목록 표시와 binding JSON 저장을 재생 PASS로 쓰지 않는다. group catalog snapshot reload 뒤 이전
cache가 남는지, group duration/child local timing이 0-width로 축약되는지, authored/Product selector가
다른 ID namespace를 쓰는지 실제 원인을 재현한다. focused oracle은 2-child group의 start/update/stop,
missing leaf rollback, reload 후 changed child timing, Arena Clone/Product 동등 occurrence를 검증한다.

## G08-I. Composition의 최종 표현

사용자 기본 화면에는 다음만 남긴다.

```text
[Patterns/Stages] [Resources] [Preview]
[Sequencer: Stage Animation Effect Sound Logic Collider Camera]
[Box Detail]
[Save] [Server Replay]
```

- Save 버튼은 선택 owner들의 typed transaction을 orchestration한다. publisher/diagnostic 절차를 버튼으로
  늘어놓지 않는다.
- 선택 box만 Box Detail을 연다. 삭제/복제/trim은 exact stable occurrence에만 적용한다.
- Logic lane은 Counter/Silence/Bind/Stagger/Portal motion/Grab branch를 typed box로 보여준다.
- render frame에서는 filesystem 순회, JSON parse, physical hashing, canonical reload를 하지 않는다.
- raw diagnostics는 기본 UI에서 제거하되 structured log와 focused harness는 유지한다.

## G08-J. Collider box와 typed Server trigger

Collider lane은 Effect box의 mesh/particle bounds를 복사하지 않는다. 하나의 `Collider` UI
카테고리에서도 판정 방향과 owner가 다르므로 다음 typed adapter로 분리한다.

| Box category | 실제 owner와 판정 |
|---|---|
| Damage | Stage `hit.shape + hit.schedule + serverDamageProfileId`; Server boss-to-player overlap |
| Grab | 위 Stage hit에 `playerResponse=CAPTURE`, `attachmentSlot=BOSS_LEFT_HAND`; 성공한 player만 left-hand attachment |
| Counter | WINDUP `counterProxy`와 `COUNTER_HIT/TIMEOUT`; player-attack-to-boss admission |
| Bind | `SET_PLAYER_BIND`의 locked target/status clock. 임의 Effect overlap으로 속박시키지 않으며, 별도 area-bind가 필요할 때만 typed hit response를 추가 |
| Stagger receiver | Boss body가 player damage에서 stagger contribution을 받는 상태 계약. boss attack hit shape와 혼합하지 않음 |
| None | 판정 없음. Effect/Animation만 보이는 presentation box |

선택한 box의 `Box Detail`은 해당 category에 존재하는 값만 노출한다.

```text
Collider Box Detail
  Category       Damage | Grab | Counter | Bind | Stagger Receiver | None
  Clock          Start ms / Lifetime ms / Tick interval or explicit offsets
  Shape          Circle | Ring | Cone | Box | Cross | Six Directions
  Anchor         Boss Root | Boss Left Hand | Locked Target | World
  Local pose     forward/right/up offset, yaw
  Geometry       radius / inner radius / angle / length / half width
  Trigger result Damage Profile | Capture Slot | Counter branch | Bind duration
  Reaction       push distance / push duration / knockdown / down duration
  [Delete Box]
```

현재 Stage hit는 boss root/facing 기준의 discrete schedule이고 Counter proxy는 Stage 전체 clock의
continuous admission이다. 새 UI는 이 차이를 숨기지 않는다. `Lifetime`을 넣기 위해 discrete hit를
매 프레임 재판정하는 두 번째 경로를 만들지 않고, source schema에 명시적인 schedule kind와
once-per-target/repeat policy를 추가할 때만 Server fixed-tick runtime까지 같은 변경으로 확장한다.

최소 구현 순서는 다음과 같다.

1. 기존 Damage/Grab hit와 Counter proxy를 category별 box로 투영하고 실제 start/end를 표시한다.
2. 기존 shape/schedule/reaction 값을 Box Detail의 typed adapter로 저장한다.
3. Grab은 attachment slot과 release action dependency를 한 transaction으로 검증한다.
4. anchor/offset/lifetime 신규 필드는 pipeline, Gameplay bootstrap, `CGameplayCatalog`,
   `CValtanBrain/CServerCollisionSystem`, Client debug mirror가 모두 소비할 때만 활성화한다.
5. invalid shape, unknown trigger, hit schedule overrun, missing damage/grab dependency와 중간 실패가
   기존 source bytes와 active Server occurrence를 보존하는 oracle을 추가한다.

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

1. stale SHA/bytes entry/replay blocker 제거와 admission rollback oracle
2. UI 단일 Save/Box Detail/zero warm-frame I/O
3. Counter typed box + `VALTAN_TRIPLE_COUNTER` Server trace
4. 버러지 finite retry/capture/Groggy branch
5. 무력화 gameplay + gauge snapshot/Effect
6. 속박 gameplay + restraint/release rollback
7. 침묵 gameplay + skill reject/HUD presentation
8. Ground Roar four-rock combat-object lifetime/explosion
9. Six Pizza rotation root와 late Element inheritance
10. grab-after-turn left-hand anchor/release tuning
11. Portal eight-rush source/Product/Server/Composition contract
12. Effect V2 Group authoring/Product playback 동등 경로
13. Save/canonical reload/Server replay 통합 검증과 PLAN/RESULT 정합성

각 커밋은 자신이 변경한 source/schema/project 등록/harness/PLAN·RESULT를 함께 포함한다. 사용자가
최종적으로 하나의 PR을 원하더라도 위 검증 단위별 commit 경계는 유지한다. 현재 dirty tree 전체를
검증 없이 `git add .`로 한 커밋에 넣지 않는다.

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
- 무력화·속박·침묵은 빈 Pattern slot이 아니라 실제 Server consumer와 종료 rollback을 가진다.
- Ground Roar는 정확히 네 rock과 5000ms explosion/despawn, Six Pizza는 late Element까지 같은 회전
  root, Portal은 8 x (500ms target + 800ms travel + 1000ms gap)로 실행/표시된다.
- V2 Group은 Arena Clone과 Product replay 양쪽에서 같은 child occurrence를 재생한다.
- Debug Core, Release Product, 필요한 FullDiagnostic과 사용자 수동 검증 결과가 RESULT에 분리된다.
