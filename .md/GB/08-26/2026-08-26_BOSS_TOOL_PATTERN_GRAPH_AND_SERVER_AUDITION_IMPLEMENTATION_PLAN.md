# Boss Tool 패턴 연결 그래프·서버 반복 검증 구현 계획

## 0. 작업 결론

현재 문제는 `VALTAN_HIGH_JUMP` 한 패턴의 Effect가 틀린 것이 아니라, 다음 권위 사슬을 한 화면에서
추적하고 같은 제품 경로로 반복 검증할 도구가 없다는 것이다.

```text
Server patternId / stageId / actionId / fixed tick
  -> animation occurrence
  -> boss-root Effect cue 또는 Server combat object Effect
  -> camera invocation / cinematic cue
  -> hit shape / damage / motion
  -> world event set / wall·floor·prop state
```

새 `Boss Tool`은 두 번째 로컬 보스 재생기를 만들지 않는다. `CValtanPatternTree`가 이미 수행하는 strict
authoring/Product join과 `CValtanPatternAuditionService`의 stable-ID Server 재생을 소비한다. 첫 구현은
현재 실제 계약에 맞춰 `Valtan Boss Tool`로 범위를 정직하게 고정하고, 다른 보스는 같은 public seam을
소비할 데이터가 생긴 뒤 확장한다.

1차 완료 단위는 다음 세 가지다.

1. 선택 Pattern과 현재 Server live Pattern/Stage를 stable ID로 대조한다.
2. Stage가 소유하거나 참조하는 animation, Effect 문서 내부 Element·resource DDS, combat object,
   camera, hit, world event를 한 화면에서 읽는다.
3. 선택 Pattern을 Server 제품 경로로 실행하고, 종료 뒤 자동 25단계 sequence로 새지 않는 IDLE HOLD와
   반복 재생을 제공한다.

Effect/animation/camera 연결의 추가·교체·삭제는 이 1차 graph truth 위에 별도 transactional authoring
단계로 붙인다. 첫 구현에서 여러 정본 JSON을 ImGui가 직접 부분 수정하지 않는다.

## 1. 현재 회귀의 실측 결과

### 1.1 추적 도끼 Effect mislink

`VALTAN_HIGH_JUMP / AIRBORNE`은 boss-root animation cue가 아니라 Server가 생성하는
`combatobject.valtan.high-jump.target-axe`를 소유한다. 해당 combat object의 정상 visual은 다음이다.

```text
combatobject.valtan.high-jump.target-axe
  -> combatobject.visual.valtan.high-jump.target-axe.v1
  -> effect.valtan.sky-axe.active
```

현재 dirty `Data/Actors/BossCatalog.json`은 마지막 Effect를
`effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01`로 바꿨다. 이것은 발탄 본체 TAKEOFF animation
cue이며 combat object의 world root에서 재생할 Effect가 아니다. 1차 변경에서 원래
`effect.valtan.sky-axe.active` 연결을 복구하고 graph validator가 independent Effect 선언과 실제 catalog
visual의 불일치를 거부하도록 한다.

### 1.2 도넛 뒤 Effect의 잘못된 귀속

`VALTAN_FIST_IN_OUT / INNER`는 animation `NONE`, stage-clock Effect 1회가 맞다. 그러나 기존
`PLAY_PATTERN_ID`가 완료 뒤 자동 25단계 sequence로 복귀하므로 다음 `VALTAN_HIGH_JUMP`,
`VALTAN_FLOOR_WIPE_130` 등의 Effect가 이어진다. 사용자는 도넛의 잔여 Effect인지 다음 Pattern인지
구분할 수 없었다.

`fx_e_decal_007_2.dds`는 현재 authored 문서 기준으로 도넛 문서의 resource가 아니다. 적어도 다음
Effect 문서들이 직접 소유한다.

```text
effect.valtan.carrier-v1.mechanic.floor-wipe-130.second-smash.clip-01
effect.valtan.carrier-v1.attack.swing.active.clip-02
effect.valtan.carrier-v1.mechanic.four-pillars-105.target-cone.clip-01
```

Boss Tool은 Effect asset ID에서 멈추지 않고 `Effect Document -> Element -> kind -> resource asset ID`를
펼쳐 보여서 이 귀속을 실행 중 Stage와 함께 확인하게 한다.

### 1.3 중앙 점프·포효·벽 파괴

`VALTAN_ARENA_BREAK_109`는 하나의 긴 animation clip이 아니라 Server stage graph다.

```text
TAKEOFF -> DROP -> IMPACT -> IMPACT_HOLD -> WIDE_REVEAL -> RECOVERY
```

- Server motion은 arena center anchor와 leap window를 소유한다.
- 각 Stage는 animation occurrence와 camera invocation을 소유한다.
- `IMPACT / ENTER`가 gameplay phase 2와 outer-wall world event set을 실행한다.
- camera 값 자체는 `Data/Encounters/Valtan/ValtanCinematicCamera.json`이 소유한다.
- 벽/바닥 mutation은 world destruction 정본과 Server runtime이 소유한다.

Boss Tool은 이를 animation에 모두 종속된 것처럼 표시하지 않고 `ANIMATION`, `CAMERA`,
`SERVER EVENT`, `HIT`, `MOTION` lane으로 구분한다.

## 2. 정본과 소유권 경계

| 정보 | 정본 / 기존 parser | Boss Tool 권한 |
|---|---|---|
| Pattern, Stage, hit, motion, event | `Valtan.gameplay.json` / `CValtanPatternTree` | read-only inspect |
| animation occurrence, Effect cue, camera invocation | `Valtan.presentation.json` / `CValtanPatternTree` | read-only inspect |
| generated encounter/binding/cue parity | Valtan publisher Product / `CValtanPatternTree` | diagnostic |
| Effect element/resource | `Data/Effects/Authored/*.effect.json` / `CEffectDocumentCodec` + loaded `CEffectCatalog` document | lazy read-only inspect, runtime parity 표시 |
| cinematic cue/keyframe/FOV/shake | `ValtanCinematicCamera.json` / `CValtanCinematicCameraDocument` | read-only inspect |
| live pattern/stage/tick | Server snapshot / `CCombatHUDViewModel` | read-only follow |
| Pattern 실행 | `CValtanPatternAuditionService` typed stable-ID command | submit only |
| damage·wall·floor·combat object 발생 | Server product runtime | Tool에서 직접 실행 금지 |

Boss Tool은 Effect ID, camera ID, wall group ID를 Server command에 보내지 않는다. Tool이 보내는 실행
identity는 stable boss placement와 `patternId`뿐이며 실제 Stage 전이가 기존 제품 연결을 호출한다.

## 3. G00 — Joined graph view 확장

`VALTAN_STAGE_VIEW`에 split presentation parser가 이미 검증하는 camera invocation의 다음 typed view를
추가한다.

```text
cameraInvocationId
cameraCueId
trigger
startOffsetMs
durationPolicy
durationMs
```

`Build_SplitAuthoringProjection()`이 검증 후 버리던 값을 master stage에 전달하고 최종 Stage view에
commit한다. 별도 JSON parser를 만들지 않는다. World event는 기존 `VALTAN_STAGE_ACTION_VIEW`의
`TRIGGER_WORLD_EVENT_SET / targetId`와 `WorldEventTriggerRefs`를 stageId로 join한다.

추가 validator는 다음 불일치를 실패로 만든다.

- independent `SERVER_COMBAT_OBJECT`가 선언한 archetype/visual/Effect와 BossCatalog Product가 다름
- camera invocation이 같은 Stage의 duration을 벗어남
- camera cue ID가 cinematic camera document에 없음
- stage action이 가리키는 world event set이 graph에 표시되지 않음

## 4. G01 — Valtan Boss Tool read-only inspector

새 파일은 `Client/Public/BossTool.h`, `Client/Private/BossTool.cpp`다. `_DEBUG`의
`CMainApp::DEBUG_TOOL::BOSS`로 lazy-create하며 F1 Developer Tools에서 연다.

### 4.0 Winters Engine ImGui 단순성 규칙

Winters의 정본 `.md/architecture/WINTERS_IMGUI_TOOL_DESIGN_GUIDE.md`와 실제 패널 구현을 Boss Tool의
acceptance contract로 사용한다. 모토는 `단순함은 궁극의 정교함이다`이며, 단순함은 기능 삭제가 아니라
`기능 범위는 완전하게 / 화면 구조는 최소하게 / 권위 흐름은 하나로 / 결과 확인은 즉시`라는 뜻이다.

- `CombatDebugPanel.cpp`: 현재 player/hover/last action을 먼저 보여 주고, 조작은 한 줄의 소수 버튼으로
  끝낸다.
- `SkillTimingPanel.cpp`: tool의 권한 범위를 첫 문장에 밝히고, champion/slot 하나를 고른 뒤 선택된 정의만
  보여 주며, 실행 결과를 버튼 바로 아래 status로 남긴다.
- `ModelAnimPanel.cpp`: target이 없으면 `Select ... above`로 즉시 끝내고, 선택 뒤 핵심 Animation만 기본으로
  열며 나머지 세부 기능은 접힌 section으로 둔다.

따라서 이 Boss Tool은 다음 규칙을 지킨다.

1. 첫 화면은 `현재 Server Pattern/Stage`, `선택 Pattern`, `Play Once / Loop / Stop After Current`만 먼저
   읽혀야 한다.
2. Pattern 하나와 Stage 하나만 선택한다. 전체 JSON tree, 모든 Stage 상세, 모든 Effect resource를 동시에
   펼치지 않는다.
3. 선택이 없거나 Server boss가 없으면 추측한 fallback을 표시하지 않고 짧은 empty-state/status로 끝낸다.
4. 선택 Stage의 연결은 `Animation / Effect / Camera / Hit·Motion / World` 다섯 줄 요약으로 보인다.
5. occurrence timing, Effect Element/resource, camera keyframe, 원인 검색은 단 하나의
   `Why / Advanced diagnostics` 아래에서 기본 접힘 상태로 제공한다.
6. 핵심 실행 결과와 실패 이유는 실행 버튼 바로 아래 한 줄 status로 보이며 로그 창을 따로 요구하지
   않는다.
7. 1차 도구에는 raw JSON editor, 복제된 Balance field, 의미 없는 수치 조절기, 비활성 placeholder tab을
   두지 않는다.

기본 화면의 세 질문은 이것으로 고정한다.

```text
지금 무엇이 재생 중인가?
선택 Stage는 무엇을 호출하는가?
같은 Server Pattern을 다시 재생하려면 어디를 누르는가?
```

#### 사용자 작업 계약

```text
사용자 작업: 패턴·애니메이션·Effect 작업자가 발탄 Pattern 하나를 선택해 Server 제품 경로로 반복
             재생하고, 현재 Stage와 연결 원인을 확인해 버그 owner를 찾을 때까지 관측한다.
대상 범위: CValtanPatternTree가 admission한 모든 Valtan gimmick/rotation/manual audition Pattern과 Stage.
필수 데이터: live pattern/stage/tick, animation, boss-root/combat-object Effect, camera, hit/motion, world event.
핵심 행동: Play Selected 1개. Repeat toggle과 반복 중 Stop After Current만 보조한다. 전멸 때만
           contextual Revive Player가 나타나 같은 선택의 검증을 이어 간다.
제외: gameplay/balance 수치 편집, raw JSON 편집, Effect/Animation/Camera 자체 authoring, Client local replay.
권위/저장: canonical Data -> existing strict parser/view -> typed Server audition -> lifecycle/snapshot 확인.
           Draft/persist 없음. Tool은 canonical 파일과 Server gameplay state를 직접 수정하지 않는다.
완료 증거: F1 -> Boss Tool에서 normal/empty/error 상태와 대표 Pattern 반복을 사용자가 직접 눈으로 확인한다.
```

#### 필수 데이터 범위표

| 사용자 질문 | 기본 화면 | Diagnostics | 유일한 owner |
|---|---|---|---|
| 지금 무엇이 재생 중인가 | live Pattern/Stage, 진행 시간, HP/phase | request/revision/tick identity | Server snapshot/lifecycle |
| 어떤 animation인가 | clip 이름 또는 `No body animation` | occurrence/source/play/rate | presentation binding |
| 어떤 Effect인가 | boss Effect/combat-object Effect ID | element/kind/resource DDS | presentation + Effect document |
| 어떤 camera인가 | cue ID | invocation timing/keyframe/FOV/shake | presentation + cinematic document |
| 어떤 판정·이동인가 | hit/damage/motion 한 줄 | schedule/push/down 수치 | gameplay document/Server |
| 무엇을 부수는가 | world action/target 한 줄 | trigger identity | gameplay/world event |
| 같은 것을 다시 보는 법 | `Play Selected`, `Repeat` | 없음 | audition service/Server |

#### 기본 화면 wireframe과 행동 예산

```text
┌─ Valtan Boss Tool ───────────────────────────────────────────────────┐
│ Live: PATTERN / STAGE   0.42 / 1.20 s   Phase 2   HP 109 bars       │
│ [Play Selected] [x Repeat] [Stop After Current] [Revive*] status    │
├─ Patterns ──────────────┬─ Selected Pattern / [Stage ▼]──────────────┤
│ [Search...............] │ Animation  ...                             │
│ pattern A               │ Effect     ...                             │
│ pattern B               │ Camera     ...                             │
│ pattern C               │ Hit/Motion ...                             │
│                         │ World      ...                             │
│                         │ ▸ Why / Advanced diagnostics               │
└─────────────────────────┴────────────────────────────────────────────┘
```

- Primary action: `Play Selected` 1개.
- Secondary interaction: `Repeat` toggle. `Stop After Current`는 Repeat가 켜진 동안 같은 lifecycle을 끝내는
  문맥 행동이며 강제 취소가 아니다.
- `Revive Player`는 HUD player HP가 0일 때만 보이는 typed Server command다. Repeat는 완료 상태에서 죽은
  player를 기다리고 revive snapshot 뒤 같은 pattern을 다시 submit한다.
- `Reload Graph`, raw request/revision, resource owner 검색은 Diagnostics에 둔다.
- 도구 유형은 canonical mutation이 없는 `Debug Observer + audition session control`이다. replay는 데이터
  편집 버튼이 아니라 같은 Server 관측 occurrence를 시작하는 typed control이며, 별도 Client replay path를
  만들지 않는다.
- first-action gate: 처음 본 작업자가 10초 안에 Pattern 선택과 `Play Selected`를 찾을 수 있어야 한다.

### 4.1 상단 live/audition bar

```text
Live: patternId / actionId / stageIndex / phase / HP
[Play Selected] [Repeat] [Stop After Current]
Audition status / failure reason
```

live stage는 vector index를 저장 identity로 쓰지 않는다. snapshot의 actionId를 먼저 exact join하고,
stageIndex는 Server 관찰 값으로 함께 표시한다. join 실패는 `UNKNOWN LIVE ACTION`으로 보이며 fallback
Stage를 선택하지 않는다.

### 4.2 Pattern 목록과 재생

- 검색은 displayName, patternId, actionId를 대상으로 한다.
- 선택 목록은 All Effects와 같은 공용 inventory인 Core 6 + manual audition 20만 같은 순서로 표시한다.
- 전체 joined graph의 다른 health mechanic/rotation/legacy Product는 live/owner 진단에만 유지하고 selector,
  Play, Repeat, Follow Live 선택으로 승격하지 않는다.
- 공용 inventory의 6/20/26, unique stable ID, exact resolve, manual admission이 깨지면 reload를 fail-closed한다.
- 선택 ID는 reload 뒤 같은 ID가 있을 때만 유지한다.
- `[Play Selected]`는 기존 stable-ID Server service를 쓴다.
- `Repeat`가 켜져 있으면 COMPLETED 뒤 같은 stable ID를 다시 submit한다. 매 occurrence의 Server reset을
  그대로 사용한다.
- `[Stop After Current]`는 다음 submit만 막고 이미 승인된 Server occurrence를 로컬에서 끊지 않는다.
- Stage 선택은 별도 거대 tree 대신 선택 Pattern 바로 아래의 단일 combo를 사용한다.

### 4.3 Stage 연결 상세

기본 화면은 각 Stage의 다음 lane을 한 줄씩 요약한다.

```text
SERVER     duration, kind, hit shape/schedule/damage, motion, actions, branches
ANIMATION occurrenceId, clip, sourceStart/playMs/playRate/end policy
EFFECT     cue/occurrence/effectAssetId/timing basis/anchor/follow/stop/scale
OBJECT     combatObjectArchetypeId/clientVisualId/effectAssetId/spawn schedule
CAMERA     invocationId/cueId/start/duration + cue keyframes/FOV/easing/shake
WORLD      worldEventSetId and trigger edge
```

`Why / Advanced diagnostics`를 사용자가 열었을 때만 `CEffectDocumentCodec::Load()`로 해당 authored
문서를 읽고 다음 resource tree를 표시한다. 파일 timestamp가 바뀌면 cache를 갱신하며,
`CEffectCatalog::Find_Loaded()`의 next-spawn catalog document와 canonical serialize 결과가 같을 때만
`NEXT-SPAWN MATCHED - replay required`로 표시한다. catalog 미로드 또는 불일치는 `LOCAL UNVERIFIED`로
표시해 current disk owner를 실제 active occurrence owner로 오인하지 않게 한다. source timestamp 또는
catalog shared document가 바뀌면 기존 검색 결과는 `STALE`로 숨기고 재검색을 요구한다.

```text
effectAssetId
  elementId | displayName | particle/decal/mesh/... | visible
    slotId -> Resources-relative assetId
```

따라서 `fx_e_decal_007_2.dds`가 어느 Pattern/Stage/Effect/Element에서 왔는지 파일 검색 없이 확인할 수
있다. parse 실패는 그 Effect row만 격리하고 기존 graph를 유지한다.

### 4.4 view·authority·중복 제거

```text
view owner       CValtanPatternTree + CCombatHUDViewModel + camera/effect read-only documents
draft/persist    해당 없음
apply owner      CValtanPatternAuditionService -> Server CGameRoom
ack/freshness    audition lifecycle + boss snapshot + pinned definition revision
```

기존 `Level_ValtanArena::Render_AuditionPanel()`의 pattern browse/replay를 새 화면에 복사하지 않는다. Arena의
중복 panel 호출은 제거하고 이미 시작된 bounded transaction/timeline update만 안전하게 마무리한다. 전체
검증 workflow의 visible entry point는 `F1 -> Boss Tool`이다. Balance Tool의 Server replay/revive는 제거한다.
Effect Tool에는 작업 중인 연결 Effect 옆에서 같은 stable pattern ID를 한 번 제출하는 얇은
`Play Server` shortcut만 허용한다. Boss Tool과 Effect Tool은 같은 `CValtanPatternAuditionService`를 사용하며,
Repeat/Stop/Revive와 live graph는 Boss Tool만 소유한다.

live pinned revision의 `available`은 runtime generation 존재만 뜻하므로 current workspace graph와 같다는
근거로 쓰지 않는다. NetworkManager가 world-entry 때 고정한 presentation artifact baseline과 현재 source
closure를 byte 비교하고, 같은 경우에만 `core presentation matches workspace`로 표시한다. Graph reload 뒤
다르면 `workspace changed; restart/publish`와 `Connections are unverified`를 즉시 표시한다. gameplay
gate/hit/motion/world/next는 Server pinned gameplay generation과 동일성을 증명하는 계약이 아직 없으므로
항상 current local authoring이라는 경계를 명시한다.

### 4.5 수동 화면 gate

- 기준 executable/scene/shortcut: Debug Client, Server-approved Valtan Arena, `F1 -> Boss Tool`.
- 해상도/DPI: 프로젝트 공식 최소값이 문서화되어 있지 않아 임의 값은 만들지 않는다(`CONFIRM_NEEDED`).
- normal: boss 존재, selected pattern 재생, live Stage/연결 요약 갱신.
- empty: boss 미생성/미선택 상태에서 짧은 다음 행동만 표시.
- error/freshness: Server disconnected 또는 graph/effect parse 실패가 성공처럼 보이지 않고 다음 행동을 표시.
- 사용자가 남길 capture 예시: `BossTool_Normal.png`, `BossTool_Empty.png`, `BossTool_ServerError.png`.
- 사용자 관찰 전에는 10초 gate, 잘림/HUD 겹침, visual fidelity를 PASS로 기록하지 않는다.

### 4.6 독립 비평 처리

| 지적 | 처리 | 근거 |
|---|---|---|
| 창 X/F1/다른 Tool 뒤 invisible Repeat | 수용 | visible/open이 아니면 repeat identity를 즉시 해제한다. |
| 첫 Pattern 자동 선택과 target 없는 Play | 수용 | 초기 선택을 비우고 action 옆 대상명과 동일 predicate 차단 이유를 표시한다. |
| Arena legacy audition panel 중복 | 수용 | 자동 render 호출을 제거하고 이미 시작된 bounded update만 유지한다. |
| Balance/Effect Server replay와 Balance revive 중복 | 수용 | Balance replay/revive는 제거한다. Effect Tool은 공용 service의 one-shot context shortcut만 두고 Repeat/Stop/Revive는 Boss Tool만 소유한다. |
| Server-pinned revision과 local graph 혼동 | 수용 | live freshness와 selected connection의 unverified 경고를 기본 화면에 표시한다. |
| available revision과 workspace graph 동일성 혼동 | 수용 | immutable world-entry artifact baseline을 current source와 byte 비교하고 mismatch는 restart/publish로 fail-closed한다. |
| Pattern motion/gate/branch 누락 | 수용 | gate·phase, pattern motion, next edge를 기본 요약에 추가하고 수치는 Diagnostics에 둔다. |
| all-boss link/unlink까지 1차에 요구 | P1 판정은 기각, 후속 범위로 수용 | strict joined graph와 전문 Tool transaction seam 없이 가짜 연결 버튼을 먼저 두면 Winters 금지 사례다. 1차 명칭을 `Valtan Boss Tool — read-only presentation connections`로 명시하고, graph truth 검증 뒤 G03으로 진행한다. |
| NetworkManager 직접 send 위험 | 수용됨 | Tool은 `Send_*`를 호출하지 않는다. 연결/freshness read-only capability만 읽고 실행은 typed audition/player sink만 사용한다. |
| source contract test 누락 | 수용 | 신규 harness가 local replay/spawn/send 금지, stable service, 초기 no-selection, close-stop contract를 검사한다. |
| status 혼용·resource scan 거짓 음성 | 수용 | audition status와 diagnostic status를 분리하고 load 실패 문서 수를 보고한다. |
| raw ID 우선·`<cstdio>` 누락 | 수용 | display name 우선, ID는 tooltip/Diagnostics, 필요한 include를 명시한다. |
| 전멸 뒤 Repeat 중단 | 수용 | 죽은 동안 resubmit하지 않고 contextual typed revive 뒤 같은 선택을 재개한다. |
| Stop/Revive 결과가 파생 상태에 덮임 | 수용 | explicit action feedback을 우선하고 revive snapshot ack 또는 다음 user action에서 해제한다. |
| local gameplay 행을 pinned Server 값으로 오인 | 수용 | gate/hit/motion/world/next는 current local authoring이며 pinned generation 동일성은 미증명이라고 기본 화면에 표시한다. |
| disk Effect owner와 loaded runtime cache 혼동 | 수용 | timestamp 재로드 뒤 loaded next-spawn catalog와 canonical 비교해 `NEXT-SPAWN MATCHED - replay required` 또는 `LOCAL UNVERIFIED`로 fail-closed한다. |
| Effect hot replacement 뒤 active occurrence 오인 | 수용 | catalog match는 next-spawn에만 한정하고 active occurrence는 검증됐다고 표시하지 않는다. 새 replay 뒤 눈으로 판정한다. |
| resource owner 검색 결과 stale | 수용 | source/catalog generation이 바뀌면 기존 결과를 숨기고 `Find Owner` 재실행을 요구한다. |

수정본은 같은 독립 비평 주체에게 P0/P1 재확인을 요청한다.

## 5. G02 — Server isolated completion HOLD

새 replay runtime은 만들지 않는다. `PLAY_PATTERN_ID`가 자연 완료되는 lifecycle edge에서 기존
`bAutomaticPatternSequenceAuditionOverride`와 `bAutomaticPatternSequenceAuditionHold`를 함께 유지해
다음 Server tick이 automatic 25-step sequence를 선택하지 못하게 한다.

다음 `PLAY_PATTERN_ID`는 기존 full reset 후 새 stable ID를 queue하므로 반복 재생이 가능하다. focused
Server contract는 다음을 증명한다.

1. 선택 Pattern PENDING -> ACTIVE -> COMPLETED.
2. 완료 후 여러 fixed tick 동안 boss는 IDLE이고 rotation cursor가 움직이지 않는다.
3. 다음 stable-ID request가 reset 후 동일/다른 Pattern을 다시 시작한다.
4. 일반 제품 encounter의 automatic sequence는 Debug audition을 사용하지 않았을 때 그대로다.

장기적으로 여러 Pattern subset, environment preset, repeat cursor를 편집하려면 기존
`m_ValtanTimelineAudition`을 `Boss Audition Program`으로 승격한다. `ISOLATED_REPEAT`와
`SCENARIO_SEQUENCE`를 분리하고 program lifecycle을 복제한다. 이것은 1차 단일 Pattern loop와 별도의
후속 수직 슬라이스다.

## 6. G03 — 연결 편집과 전문 Tool deep link

1차 inspector가 보여 주는 연결 graph를 기준으로 후속 구현한다.

### 6.1 Deep link

Boss Tool이 다른 Tool 객체나 파일 경로를 직접 소유하지 않는다. MainApp에 typed navigation intent를
제출한다.

```text
Open Animation(patternId, stageId, clipOccurrenceId)
Open Effect(effectAssetId)
Open Camera(cameraCueId)
```

MainApp가 active Tool을 바꾸고 각 전문 Tool의 최소 public open API를 호출한다. Effect Tool의 unsaved
modal과 document admission을 우회하지 않는다. 현재 cinematic Camera Tool은 없으므로 camera deep link는
별도 camera authoring serializer/editor가 생길 때까지 read-only다.

### 6.2 Link / unlink transaction

연결 편집은 lane별 정본 하나만 바꾸고 전체 pipeline validation을 통과한 뒤 commit한다.

- animation occurrence: Animation Tool과 `Valtan.presentation.json`
- Effect cue: Effect Tool과 `Valtan.presentation.json`
- combat-object visual: Boss catalog authoring lane
- camera invocation/cue: presentation + cinematic camera document의 staged transaction
- world destruction: gameplay event + destruction graph publisher

`Unlink`는 runtime Product나 Effect 파일을 삭제하지 않는다. 선택된 stable reference만 staged draft에서
제거하고, orphan/required-owner validation을 통과해야 Save한다. Server hit 또는 world event를 Effect
삭제와 함께 암묵적으로 지우지 않는다.

## 7. 수정 파일

### 1차 구현

```text
Client/Public/BossTool.h (new)
Client/Private/BossTool.cpp (new)
Client/Public/MainApp.h
Client/Private/MainApp.cpp
Client/Private/Level_ValtanArena.cpp
Client/Default/Client.vcxproj
Client/Default/Client.vcxproj.filters
Client/Public/ValtanPatternTree.h
Client/Private/ValtanPatternTree.cpp
Data/Actors/BossCatalog.json
Server/Private/GameRoom.cpp
Server/Private/ServerGameplayContractTests.cpp
Tools/ValtanPipeline/test_valtan_boss_tool_contract.py (new)
이 PLAN과 대응 RESULT
```

단일 replay/revive owner를 만들기 위해 `BalanceTool.*`와 `Effect_Tool.*`에서는 기존 Server audition UI와
dependency만 제거한다. 각 Tool의 수치 authoring과 Effect local authoring/preview 변경은 보존한다.
`Animation_Tool.*`과 current authored Effect 문서는 이 Boss Tool 구현이 직접 수정하지 않는다.

## 8. 검증 계약

### 자동

1. JSON parse와 Valtan publisher Check.
2. `CValtanPatternTree` split source/Product parity 및 camera invocation projection harness.
3. Boss Tool source contract: 공용 tree/service/codec/document를 사용하고 직접 network/JSON parser를 만들지
   않았는지 확인.
4. Server Debug build와 `Server.exe --contract-test`.
5. Client x64 Debug compile/link. 실행 중 Client가 canonical exe를 점유하면 별도 output link로 증명한다.
6. `git diff --check`.

### 사용자 육안

1. Effect Tool -> All Effects -> Independent Effect의 도넛에서 `Play Server Owner`를 누른다.
   Boss Tool 상단에는 `VALTAN_FIST_IN_OUT [live only; outside All Effects list]`가 보이고 선택 목록에는
   나타나지 않아야 한다. 도넛 한 번 뒤 다른 Pattern/바닥 decal이 이어지지 않아야 한다.
2. `VALTAN_HIGH_JUMP` 선택.
   AIRBORNE의 owner가 `SERVER_COMBAT_OBJECT`, Effect가 `effect.valtan.sky-axe.active`로 보이고 도끼 세 번과
   LAND 원형 Effect가 분리되어 보여야 한다.
3. Effect resource 검색에서 `fx_e_decal_007_2.dds`를 찾으면 owner Pattern/Stage/Effect/Element와
   `NEXT-SPAWN MATCHED - replay required` 또는 `LOCAL UNVERIFIED`가 함께 표시되어야 한다. match 뒤에도
   active occurrence 증거는 아니므로 해당 owner Pattern을 새로 실행한 다음 시각 판정한다. source/catalog가
   바뀌어 `STALE`이 뜨면 `Find Owner`를 다시 누른다.
4. `VALTAN_ARENA_BREAK_109` 선택.
   TAKEOFF부터 RECOVERY까지 live Stage가 이동하고, IMPACT에서 wall event, 각 Stage의 camera cue,
   WIDE_REVEAL의 포효 animation이 같은 graph에서 보여야 한다.
5. 3연속 돌진, 잡기/날리기, floor destruction Pattern을 각각 반복해 이전 실행의 wall/floor/combat object가
   다음 isolated occurrence에 남지 않는지 확인한다.

사용자의 관찰 전에는 visual PASS로 기록하지 않는다.
