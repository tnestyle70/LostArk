# Action Composition Workbench 구현 결과

## 2026-09-02 최신 통합 상태 — 아래 중간 기록보다 우선

아래에 남아 있는 2026-08-31 절은 각 중간 revision의 원인과 당시 검증 증거를 보존한 기록이다.
현재 구현·자동 검증 상태는 이 절을 정본으로 사용한다. 과거의 `57 patterns / 255 stages`,
V2 `81 authored / 80 bindings / 1 group`, TRASH retry terminal, 빌드 미실행 표시는 모두 현재
상태로 확대 해석하지 않는다.

| 항목 | 현재 구현 상태 | 자동 검증 | 사용자 visual |
|---|---|---|---|
| canonical graph | 실제 Product validator가 62 Pattern / 276 Stage를 읽고 managed 39 / legacy 25를 구분한다. | Gameplay publisher Validate PASS. 이번 dirty 통합본의 최종 Product/Core는 타 세션 취합 뒤 대기 | 미검증 |
| stale SHA/bytes entry blocker | Network entry와 replay는 현재 typed closure와 Server gameplay revision을 사용한다. 과거 고정 SHA/bytes generation을 정상 수정의 차단 조건으로 사용하던 경로는 교정했다. | 최종 Debug/Release Core PASS | 미검증 |
| Counter/상태 Pattern | Counter Logic box/proxy, 무력화 gauge, 속박(+10m/5초), 침묵(5초), Counter→Groggy의 typed source·Server·Client 소비가 연결됐다. | focused contract와 최종 Core PASS | 미검증 |
| Ground Roar 네 돌 | 발탄 yaw 기준 0/90/180/270의 독립 Server object 4개가 생성되고 5초 뒤 explode pulse와 함께 despawn한다. Composition은 같은 5초 tail을 표시한다. | focused contract와 최종 Core PASS | 미검증 |
| Six Pizza | Server가 occurrence 시작 때 고른 플레이어를 유지하고 authored landing center에서 그 플레이어의 현재 XZ를 향하는 yaw를 fixed tick마다 갱신한다. Client는 `arena.center.target-follow`의 동일 root handle에 공통 `+180°`를 적용해 초기 sector와 delayed Element가 플레이어 반대 방향으로 함께 회전한다. 피자 source JSON과 timing은 추가 수정하지 않았다. | focused contract와 Server/Client Debug `ClCompile` PASS. 통합 Core는 다른 세션 취합 뒤 대기 | 반대 방향 육안 확인 대기 |
| 버러지 retry | `VALTAN_TRASH`와 `VALTAN_TRASH_CATCH_IF` 모두 최초 rush 뒤 `RECHARGE_WAIT_02 → RETRY_RUSH_02 → RETRY_MISS_02 → RECHARGE_WAIT_03 → RETRY_RUSH_03 → RETRY_EXHAUSTED`의 finite retry를 가진다. | canonical/transaction/Core PASS | 미검증 |
| 잡기/왼손/날리기 | `BOSS_LEFT_HAND` capture, left-hand bone world matrix attachment, `ARENA_EJECTION` speed/duration/yaw editor와 Server boss-facing 상대 launch가 연결됐다. | focused contract와 최종 Core PASS | 미검증 |
| Portal | `VALTAN_WARP`는 8회, 16m, 500ms retarget, 800ms travel, 1000ms gap이며 Composition의 Delay/Rush/Gap box가 같은 clock을 표시한다. | focused contract와 최종 Core PASS | 미검증 |
| Phase 3 망령 본체/포탈 | primary `BOSS_VALTAN`의 NetEntityId·HP·damage·HUD 권위를 유지한 채 phase 3에서 ghost part group으로 교체한다. 부활 완료 뒤 exact 6 Pattern을 순환하고, 별도 5초 clock이 arena center 기준 네 꼭짓점의 portal missile을 같은 tick에 생성해 중심을 지나 반대 꼭짓점까지 돌진시킨다. | Gameplay publisher Validate, focused contract 4/4, Server/Client Debug `ClCompile` PASS | Ghost Drive asset 누락 및 실제 6-loop/4-portal 육안 확인 대기 |
| Effect Save 즉시 반영 | direct-authored Product Effect Save는 disk CAS 뒤 선택 catalog/GPU target을 원자 교체한다. 성공 뒤 새 spawn만 새 문서를 쓰고 이미 재생 중인 occurrence는 이전 immutable resource로 끝난다. 실패하면 disk를 이전 canonical로 CAS 복구하고 편집 draft를 dirty로 보존한다. | Effect focused/saved-row contract PASS, `Effect_Tool.cpp` 및 Client Debug `ClCompile` PASS | 새 통합 EXE에서 sky-axe 7-element Save/replay 확인 대기 |
| V1 Decal Normal Cut | V1 cutoff는 normal-mapped lighting normal 대신 depth 복원 위치의 기하 수신 노멀을 사용한다. GBuffer normal은 hemisphere 정렬에만 쓰므로 평평한 바닥은 통과하고 벽 cutoff는 유지한다. | shader contract 4/4, `fx_5_0` compile PASS | 바닥 decal 육안 확인 대기 |
| Saved Pattern/Flow Restart | Boss Verification 기본 재시작과 Pattern Flow 재시작은 저장된 `scriptedSequence`를 다시 읽고 walls/floors/props/collision/Nav/combat objects를 복구한 뒤 Pattern 01부터 시작한다. Boss Verification은 saved slot이 정확히 1개일 때만 허용한다. | restart focused contract 48/48, Client Debug `ClCompile` PASS | 실제 arena reset/replay 확인 대기 |
| Effect V2 | 92 authored / 84 bindings / 4 groups / 4 independent / 56 textures를 admit한다. `VALTAN_STRUGGLING` STEP_04~07에 4 group, 8 timeline box를 연결했고 canonical reload 시 V2 catalog를 한 번 stage한다. | V2/Workbench 59/59, Effect V2 validator 24/24, 최종 Core PASS | 실제 group/leaf 재생 미검증 |
| Root Motion transaction | candidate Encounter와 patternbindings에서 `Valtan.rootmotion.json`을 함께 투영하고 Create Pattern, ApplyTypedPatch, 일반 Apply의 동일 atomic commit/rollback closure에 포함한다. 현재 44 Pattern / 119 Stage / 6,617 sample이다. | transaction 49/49, typed patch 7/7, root-motion/atomic 9/9, freshness PASS | TRASH 전진 동작 미검증 |
| Tool 프레임 저하 | Effect JSON은 catalog metadata와 선택 문서 lazy decode를 사용한다. Workbench의 V2 eager load는 canonical reload 시 한 번만 실행하며 per-frame repository scan을 하지 않는다. | source/focused contract와 최종 Core PASS | 사용자 FPS smoke 대기 |

### `VALTAN_STRUGGLING` Effect V2 exact join

| Stage / clip | V2 group | Composition box start |
|---|---|---|
| `STEP_04 / mesh_att_battle_19_01` | `boss.valtan.impact` | 1233, 2233, 3233, 4200 ms |
| `STEP_05 / mesh_att_battle_19_02` | `boss.valtan.pounding.chase` | 0 ms |
| `STEP_06 / mesh_att_battle_19_03` | `boss.valtan.pounding` | 200, 400 ms |
| `STEP_07 / mesh_att_battle_19_04` | `boss.valtan.twohand` | 1033 ms |

Composition은 canonical admit 때 BOSS_VALTAN V2 catalog를 stage하므로 Resources tab을 먼저
열지 않아도 첫 timeline에 위 8개 box가 투영된다. Arena Clone 재생 경로는
`Play_EffectivePreview → Stage_LocalPatternAuthoringPreview → Notify_Clip / Sync_StageAuthoring →
Tick → Expand_Group`이며, group의 ordered child leaf를 같은 snapshot에서 펼친다.

### 최종 표준 빌드 증거

- Debug / Core: PASS, 31 steps 중 FAIL 0
  - `out/BuildPipeline/runs/20260831T211204254Z-debug-core-176cfba0.json`
- Release / Core: PASS, 31 steps 중 FAIL 0
  - `out/BuildPipeline/runs/20260831T213024268Z-release-core-f1dffc84.json`
- 두 evidence 모두 Engine, Shared, Server, Client, compiled shader closure와 Core harness를 포함한다.

구현/자동 검증/사용자 수동 검증은 계속 분리한다. 사용자가 새 Debug EXE에서 Workbench 첫
timeline의 8개 V2 box, Arena Clone/Complete Play의 실제 group 재생, TRASH retry 전진, 창 resize와
FPS를 확인하기 전에는 visual PASS로 기록하지 않는다.

## 결론

이번 변경은 기존 Animation Tool에 Pattern 저작 UI를 계속 누적하지 않고, F1에서 독립적으로
여는 `Action Composition Workbench`를 추가했다. 저작의 루트는 파일 목록이나 animation clip
index가 아니라 stable `Pattern ID`이며, 선택 Pattern 아래의 `Stage ID`와 animation occurrence를
기준으로 gameplay와 presentation 데이터를 한 화면에 join한다.

Workbench 자체는 새 runtime 정본이 아니다. 다음 기존 typed source owner를 stage하고, 공통
writer generation에서 Validate/Publish한 Product를 실제 canonical loader로 다시 읽는 orchestration
shell이다.

```text
Pattern
└─ Stage (Server wall clock / Stage Role)
   ├─ Animation Sequence Slot occurrence[]
   ├─ Server Collider + Hit Schedule
   ├─ Counter success -> same-Pattern GROGGY Stage
   ├─ Player Reaction / Grab Release velocity·duration·yaw
   ├─ Effect invocation
   └─ Sound cue occurrence
```

## 실제 반영 범위

### 1. 독립적이고 resize 가능한 Workbench

- `CActionCompositionWorkbench`를 새 Client tool로 추가하고 project/filter에 등록했다.
- F1 Developer Tools에서 기존 Animation Clip Tool과 별도로 열 수 있다.
- 첫 크기만 viewport에 맞춰 제안하며 `AlwaysAutoResize`, hard minimum, `NoResize`를 사용하지
  않는다.
- canonical join, model preview 또는 Server 상태가 실패해도 전체 창을 early-return으로 숨기지
  않는다. 이전 admitted view는 진단용으로 남기되 모든 Save/재생/Server mutation을 차단한다.
- raw filesystem 수만 건 대신 선택 Pattern과 실제 join된 owner/resource만 표시한다.

### 2. Pattern 중심 Browser와 Sequence Slot

- 현재 canonical Product의 61개 Valtan Pattern과 277개 Stage를 stable ID로 읽는다.
- model-independent Valtan animation intake 265개를 별도 Sequence browser에 의미 단위로
  표시한다.
- Sequence를 preview한 뒤 선택 Stage에 `Replace Stage Slots` 또는 `Append to Stage Slots`로
  넣을 수 있다.
- occurrence는 vector index가 아니라 `.composition.clip.NN` stable ID를 사용한다. 삭제 뒤
  Append도 기존 ID와 충돌하지 않는 다음 빈 번호를 선택한다.
- Replace는 exact clip을 먼저 재사용하고, 기존/신규 양쪽에서 유일한 `_start/_loop/_end` 역할만
  같은 논리 occurrence로 재사용한다. 역할 안에서 clip이 바뀌면 `PROJECT_AUTHORED`로 표시하고
  Effect·Sound·Shake source window를 후보 graph에서 다시 검증한다. 반복 loop처럼 역할이
  모호하거나 새로 추가된 box는 새 stable ID를 발급하며 dependency는 임의 대상을 따라가지 않고
  fail-closed한다.
- Stage 안에서 occurrence reorder/remove/trim을 지원한다. 마지막 slot이 무한/native-duration
  policy인 동안 뒤에 Append하는 잘못된 clock 구성은 거부한다.
- 선택 animation chain은 기존 Create Pattern transaction으로 전달하며, 생성 성공 뒤 새
  Pattern을 canonical reload해서 Workbench에서 바로 선택한다.

### 3. 내부 공백과 Server Stage clock

- Pattern 시간축의 기준은 Server Stage `durationMs`다.
- animation occurrence의 wall time보다 Stage가 길면 그 차이를 다음 Stage 전
  `HOLD_LAST_POSE` trailing gap으로 표시하고 저장한다.
- 도끼 Pattern처럼 마지막 동작 뒤 공백을 늘리는 작업은 Stage Duration을 늘리는 방식으로
  typed gameplay source에 저장한다.
- 임의의 두 slot 사이에 무명 blank key를 삽입하는 기능은 현재 지원하지 않는다. 의미 있는
  중간 공백은 별도 Stage로 모델링해야 한다.

### 4. Collider와 Effect의 권위 분리

- 기존 Server hit가 있는 canonical Stage에서는 collider를 편집·제거한다. 신규 collider 추가는
  non-WAIT manual audition Stage에서만 허용하며 canonical no-hit Stage에는 임의로 만들지 않는다.
- 지원 shape는 `CIRCLE`, `RING`, `CONE`, `BOX`, `CROSS`, `SIX_DIRECTIONS`이며 radius,
  angle, length, half-width와 Damage Profile을 조정한다.
- hit delay/interval/count 또는 기존 explicit hit offset을 같은 Stage clock에서 확인한다.
- push range/duration, knockdown/down duration도 Server player reaction owner에 저장한다.
- Effect row와 Collider row는 Sequencer에서 같은 Stage에 나란히 보이지만, 보이는 Effect mesh가
  hit authority나 collider로 자동 승격되지는 않는다.
- 현재 inline authoring은 Stage당 하나의 hit shape/schedule이다. 다중 collider stack과 Effect
  element에서 collider를 자동 생성하는 기능은 완료 범위가 아니다.

### 5. Counter에서 Groggy로 가는 typed edge

- Counter는 의미 없는 `true/false` 하나로 저장하지 않는다.
- `WINDUP` Stage가 Counter window를 소유하고, 성공 시 같은 Pattern의 정확한 `GROGGY`
  Stage/action stable ID를 가리킨다.
- UI에서 `Stage Role`, `Counter Enabled`, `Counter Success Groggy`를 함께 조정한다.
- 새 topology 저장 순서는 기존 edge disable -> Stage Role 변경 -> 새 edge enable/retarget으로
  고정해 중간 문서가 invalid해지는 회귀를 제거했다.
- GROGGY Stage가 없으면 Counter 활성화를 거부한다.

### 6. 잡기 후 날리기 수치 튜닝

- 기존 `RELEASE_GRABBED_PLAYERS` action이 있는 Stage를 선택하면 다음 수치를 Pattern Detail에
  표시하고 typed gameplay source로 저장한다.
  - Release Velocity: 0..50 m/s
  - Release Duration: 0..5000 ms
  - Release Rotation / Yaw Offset: -180..180 degrees
- 따라서 뒤돌아 잡기 후 반대 방향으로 날아가는 경우 yaw offset을 눈으로 조정할 수 있다.
- 기존 grab/capture/release topology와 왼손 attachment를 보존하는 수치 튜닝이다. 아무 Pattern에
  새 grab topology 전체를 생성하는 기능은 이번 범위가 아니다.

### 7. Sound와 Effect 관계

- Pattern Sound cue는 typed source owner에서 add/edit/remove하고 별도로 Save한다.
- animation occurrence 변경 전에 Sound의 `clipOccurrenceId` dependency를 후보 Stage에 대해
  검증한다. unsaved Sound draft가 있으면 Pattern/Sequence Save와 canonical reload를 차단한다.
- Sound Save는 Pattern canonical writer transaction에 합치지 않는다. 같은 read-generation
  admission과 occurrence dependency validation을 통과한 뒤 기존 Sound owner에 별도 CAS/atomic
  replace하고, committed source와 local reload 상태를 Pattern commit과 분리해 표시한다.
- Sound runtime 적용은 저장 직후로 가장하지 않는다. exact Pattern revision이 Server-active가 된
  뒤 `Retry Apply`로 실제 replay consumer에 적용한다. 적용 전·후 Server revision을 비교하고
  consumer-ready receipt를 그 exact revision에 고정한다. 그 전이나 적용 실패 시 committed source를
  숨기지 않고 runtime apply pending으로 남겨 Complete/Restart/Next를 막는다.
- Effect invocation은 typed Details에서 add/full update/exact remove한다. exact clip occurrence와
  source start/end, stop/repeat, anchor/follow, local position/rotation/scale, scale policy를 canonical
  `Data/Valtan/Valtan.presentation.json`에 저장한다. generated
  `Valtan.patterneffectcues.json`은 projector가 만드는 read-only Product다.
- invocation의 연결·시간·배치와 Effect asset body 편집은 분리한다. 후자는 Save/Publish/Reload된
  admitted occurrence와 전체 cue field가 일치할 때만 선택 `effectAssetId`의
  `Data/Effects/Authored/*.effect.json`을 Effect Tool로 여는 exact deep-link다. draft-only row에
  asset-only fallback은 없다.

### 8. Save, Publish, canonical reload, Server playback

`Save Pattern + Validate + Publish`는 generated
`Valtan.patternbindings.json`이나 `Valtan.patterneffectcues.json`을 직접 저장하지 않는다.

```text
typed source draft
-> validate
-> common writer generation commit
-> Product projection
-> local canonical graph reload
-> immutable runtime candidate publish
-> exact Server-active revision 확인
```

- source/Product commit과 Server runtime activation 상태를 분리해 표시한다.
- presentation generation이 바뀌어 현재 world에서 적용할 수 없으면 known-NACK를 보내지 않고
  `REENTRY_REQUIRED`로 표시한다.
- Complete Play와 Restart는 exact Server-active definition revision이 확인된 경우에만 열린다.
- Restart는 arena 전체 reset이 아니라 현재 또는 완료된 exact occurrence CAS다.
- `Fresh / Restore Arena`는 벽과 arena 상태를 복구하는 별도 명령이다.
- `Queue as Next`는 현재 Pattern을 끊지 않고 선택 Pattern을 기존 Server pending 경로에 넣는다.
- `Pattern Flow...`는 기존 canonical Flow editor를 열며 새로운 중복 Flow 정본을 만들지 않는다.

## 자동 검증 결과

### 데이터와 focused contract

- Root Motion을 포함한 Create/typed/general Apply transaction 묶음: 49/49 PASS
- canonical typed patch 강화 suite: 7/7 PASS
- Root Motion/atomic save suite: 9/9 PASS
- V2 clip projection, Effect invocation, Workbench regression: 59/59 PASS
- Effect V2 validator: 24/24 PASS
  - 92 authored / 84 bindings / 4 groups / 4 independent / 56 textures
- Root Motion freshness: 44 patterns / 119 stages / 6,617 samples, PASS
- canonical Product: 61 patterns / 277 stages / Complete Play 38 patterns

### 표준 빌드와 native runtime contract

- Debug / Core: PASS
  - evidence: `out/BuildPipeline/runs/20260831T211204254Z-debug-core-176cfba0.json`
  - 31 steps, FAIL 0
- Release / Core: PASS
  - evidence: `out/BuildPipeline/runs/20260831T213024268Z-release-core-f1dffc84.json`
  - 31 steps, FAIL 0
- 두 profile 모두 Engine, Shared, Server, Client build/link, compiled shader closure,
  NetworkProtocol, Character Select isolation과 Valtan native harness를 통과했다.

Debug Core에는 실제 `CValtanPatternTree::Load`를 호출하는
`ValtanPatternAuditionServiceHarness`를 편입했다. 따라서 Python source-token 검사만 통과하고
실제 Pattern 목록·Complete Play가 비는 회귀를 Core 성공으로 처리하지 않는다.

## 요청 Pattern 콘텐츠의 현재 경계

- `VALTAN_FLOOR_WIPE_130`은 첫 타격 뒤 266 ms, interval Stage 500 ms의 Server clock/gap을
  canonical Product에서 읽고 Details로 조정할 수 있다.
- `VALTAN_CATCH_BREATH`의 기존 release action은 현재 24 m/s, 500 ms, yaw 0 deg이며 세 값을
  Details에서 편집할 수 있다. 180도 보정의 실제 화면 방향은 사용자 visual 확인 전이다.
- `VALTAN_SIX_PIZZA`는 Pattern 시작 시 Server가 잠근 boss facing을 `arena.center.facing` root로
  사용하며 sector와 delayed Element가 같은 world root를 상속한다.
- `VALTAN_WARP`는 8회, 500 ms retarget, 20 m/s, 16 m rush, 1000 ms gap을 사용하고
  Composition Delay/Rush/Gap box가 같은 clock을 표시한다.
- `VALTAN_TRASH`와 `VALTAN_TRASH_CATCH_IF`는 counter/capture/left-hand branch와 함께
  `RECHARGE_WAIT_02 → RETRY_RUSH_02 → RETRY_MISS_02 → RECHARGE_WAIT_03 →
  RETRY_RUSH_03 → RETRY_EXHAUSTED`의 finite retry를 가진다.
- `VALTAN_STRUGGLING`의 animation STEP_04~07에는 현재 V2 group 4종이 8개 occurrence로
  연결돼 Composition Effect lane과 local preview runtime이 같은 binding snapshot을 소비한다.
- 무력화·속박·침묵은 stable gameplay Pattern과 Server 소비가 존재한다. 사용자가 원하는 최종
  animation Sequence 선택과 시각 품질은 별도 visual 확인 대상이다.

따라서 이번 결과는 요청 Pattern 전부의 콘텐츠 완성이 아니라, 위 콘텐츠를 안전하게 늘리고
튜닝할 수 있는 Pattern 중심 저작 수직 슬라이스와 첫 실제 데이터 적용 범위다.

## 사용자 수동 검증 대기

저장소 규칙에 따라 에이전트가 Client UI를 자율 실행하거나 visual PASS를 대신 판정하지 않았다.
다음 항목은 사용자의 첫 화면 확인 전까지 PASS가 아니다.

1. F1에서 `Action Composition Workbench`가 독립적으로 열린다.
2. 창을 작게 줄이고 다시 늘릴 수 있으며 Details/Sequencer/Data Files가 사라지지 않는다.
3. Valtan Pattern 목록과 Complete Play inventory가 보인다.
4. Pattern 선택 시 Stage, Sequence Slot, gap, Collider, Counter/Groggy, Sound/Effect 관계가 보인다.
5. 기존 Effect 목록과 선택 Pattern의 Effect 관계가 보인다.
6. Save 뒤 재진입/Server-active revision에서 Complete Play, Restart, Next가 동작한다.

## 의도적으로 완료라고 부르지 않는 경계

- Camera/Light/World lane은 inspection/deep-link이며 key add/drag/trim/save adapter가 없다.
- Arena Clone의 공통 Play/Seek/Stop은 Animation, Effect invocation과 collider mirror를 같은 Pattern
  clock에서 검증한다. Sound/Camera/World는 현재 inspection 또는 각 runtime owner 경로이며 clone의
  공통 local transport에 포함됐다고 보지 않는다.
- Effect invocation은 typed Details add/update/remove와 timeline body move를 지원하고,
  `cue_end` invocation은 오른쪽 trim으로 끝 시간을 조정한다. Effect asset 내부 element 편집은
  Effect Tool deep-link다. Sound cue는 point occurrence 이동만 지원하며 가짜 duration trim을 만들지
  않는다.
- Stage당 다중 collider, bone/Effect element anchor에서 collider를 새로 만드는 기능은 없다.
- 임의 중간 blank key는 없고 trailing gap 또는 별도 Stage만 지원한다.
- 기존 release action 수치는 편집하지만 새 capture/grab/release topology 전체 생성은 지원하지
  않는다.
- Pattern source와 Sound source는 각각의 typed owner commit이다. dependency admission과 적용
  순서는 공유하지만 하나의 가짜 다중-owner JSON이나 단일 atomic Save로 표현하지 않는다.
- 위 경계 때문에 현재 결과를 Unreal/Unity Sequencer 전체 완성이라고 부르지 않는다. Pattern
  Stage/Animation/Collider/Counter/Grab 수직 슬라이스가 실제 source에서 Server consumer까지
  닫힌 상태다.

## PR 구성 주의

공유 worktree에는 Action Composition과 무관한 DimensionMaster Effect, portal visual carrier,
Level navigation, Effect unlink/build guard 변경이 함께 존재하고, 현재 branch는 `origin/main`보다
3커밋 뒤다. 따라서 아직 PR-ready가 아니며 `git add -A`로 한 커밋을 만들지 않는다. 최소한 다음
검증 단위로 hunk를 분리한다.

1. canonical source/Product writer와 reader admission
2. resizable Workbench shell과 semantic Browser
3. typed Stage/Sequence/Collider/Counter/Grab authoring
4. Create Pattern transaction
5. Complete/Restart/Next/Flow protocol
6. 문서와 focused/native 검증

사용자 화면 검증 결과는 위 자동 검증과 별도로 이 문서에 추가한 뒤 PR 완료 상태를 판정한다.

## 2026-08-31 F1 / Workbench 프레임 회귀 교정

### 구현 상태

- F1 root 창은 최초 크기만 제안하고 `SetNextWindowSizeConstraints`를 사용하지 않는다. 이번 교정에서
  추가했던 좁은 폭 전용 card 전환과 Animation Clip Tool 크기 정책 변경은 회수했다. 즉 F1 크기
  제한 제거만 남겼다.
- Action Composition Workbench의 ImGui 활성 상태 계산은 physical Product admission을 호출하지
  않는다. 기존에는 매 frame
  `Get_ServerActivePatternRevision -> Is_CurrentPresentationBaselineIntact -> presentation artifact
  read/SHA-256` 경로를 실행했다. 현재 Toolbar와 Sound runtime 상태 표시는 memory-only
  `Observe_ServerActivePatternRevision`을 사용하며, exact Product/Sound admission은 Complete Play,
  Restart, Next, Sound Apply의 실제 command edge에서만 실행한다.
- Sequencer는 중앙 작업 공간의 54%를 기본 사용하고 280..680 px 범위로 표시된다. Browser/Preview는
  26%로 줄여 joined track을 주 작업 화면으로 만들었다.
- 선택 Stage의 Sequencer 상단에 `Selected Stage Gap (ms)`를 추가했다. 값은
  `Animation wall + gap = Stage duration`으로 typed Balance draft에 stage되고, 0 ms는 `EXACT`,
  양수는 `HOLD_LAST_POSE`로 저장된다. `Save Pattern + Validate + Publish` 전에는 source draft이며
  generated Product를 직접 수정하지 않는다.

### 최신 자동 검증

- F1 semantic resource / navigation: 8/8 PASS
- F1 arena preservation / resize: 13/13 PASS
- Action presentation Workbench: 43/43 PASS
- Action Composition Sound owner: 14/14 PASS
- Action Composition regression oracle: 21/21 PASS
- 합계: 99/99 PASS
- 변경 파일 대상 `git diff --check`: PASS

### 아직 PASS가 아닌 항목

- 이 교정 뒤 표준 Debug Product 빌드는 아직 실행하지 않았다. 검증 시점에 기존
  `Client.exe` PID 49324와 `Server.exe` PID 53964가 출력물을 점유하고 있어 build guard가 링크를
  차단했다. 프로세스를 사용자가 종료한 뒤 같은 source revision으로 다시 빌드해야 한다.
- Workbench 지속 FPS, 확대한 Sequencer 높이와 gap drag/save 결과는 새 EXE에서 사용자 수동
  검증이 필요하다. 실행 중이던 화면은 수정 전 EXE이므로 이번 교정의 visual 증거가 아니다.

## 2026-08-31 Create Pattern 3 FPS / Details Save 교정

### 재현 원인과 소스 구현

- 사용자가 수정 전 Debug EXE에서 `Create New Pattern` 탭을 열었을 때 2.7~3 FPS를 확인했다.
  `Composition Session` 창이 가려진 것과는 무관하다.
- 탭 render wrapper가 `m_bValtanPatternMasterLoadAttempted`를 검사하면서도 reload 전에 latch를
  세우지 않았다. 그 결과 탭이 열린 동안 매 frame `Reload_ValtanPatternMaster`가 실행되어
  canonical Pattern tree와 Sound/Shake/Combat Object Sound source를 반복해서 읽고 join했다.
- reload 호출 전에 attempt latch를 세워 최초 command edge에서만 file-backed load를 실행한다.
  실패해도 render rate로 자동 재시도하지 않으며 명시적 Reload command가 재시도를 소유한다.
- `Composition Details` 상단에 `UNSAVED PATTERN DRAFT`와 `Save Pattern`을 추가했다. 버튼은
  admitted Pattern draft가 실제로 dirty이고 별도 Sound owner dependency가 깨끗할 때만 열린다.
  Details는 save request만 남기고, 같은 frame의 모든 window가 immutable Pattern/Stage view 사용을
  끝낸 뒤 `Save_Publish_Reload`를 실행한다. 따라서 canonical storage 교체 뒤 stale pointer로 다른
  패널을 계속 그리지 않는다.
- `Pattern` owner 표시는 `Pattern Root / Flow`로 바꾸고, branch/action Logic은 Pattern root와 같은
  것이 아니라 Sequencer `Logic` lane에서 선택해 `Gameplay / Logic / Collider` typed detail로
  편집한다는 안내를 추가했다.

### 이번 교정 자동 검증

- 새 회귀 oracle은 Create Pattern 최초 load latch가 reload보다 먼저 실행되는지 검사한다.
- 새 회귀 oracle은 Details Save가 직접 canonical storage를 교체하지 않고 모든 window render 뒤
  처리되는지 검사한다.
- focused Workbench/F1 suite: 138/138 PASS
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
- requested Pattern coverage validator: PASS, Product 33 / Encounter 57
- `Publish-GameplayBalance.ps1 -Mode Validate -SkipValtanSplitProjection`: 당시 중간 revision 기준 PASS,
  57 patterns / 255 stages / 52 audition rows. 최종 canonical 수치는 61 / 277이며 문서 첫 절을 따른다.
- 변경 파일 대상 `git diff --check`: PASS

### 빌드와 수동 확인 대기

- 이 절 작성 당시에는 새 Debug/Release EXE를 생성하지 않았다. 최종 Debug/Release Core PASS는
  문서 첫 절의 evidence로 대체됐다. 당시 사용자가 확인 중이던
  `Client.exe` PID 24936, `Server.exe` PID 60348와 Visual Studio `MSBuild` PID 50784/55788가 같은
  checkout/output을 사용 중이므로 자동 runner를 중첩하지 않았다.
- 사용자가 실행과 VS build를 종료한 뒤 exact source revision에서 Debug `FullDiagnostic`과 Release
  `Product`를 실행해야 한다.
- 새 Debug EXE에서 `Create New Pattern` 탭을 30초 이상 열어 둔 지속 FPS, Details의 dirty 표시와
  `Save Pattern` 결과는 사용자 수동 판정 전까지 visual PASS가 아니다.

## 2026-08-31 Save rollback / Boss Pattern Blueprint / Preview 권위 분리

### Save 실패 재현과 교정

- 사용자 화면의 실제 실패 이유는 Stage duration 8,000 ms가 아니라
  `Pattern Sound dependency does not resolve exactly one candidate Pattern/Stage:
  VALTAN_BACKSTEP_ATTACK/SWEEP`였다. 저장 전 Sound graph 검증이 Complete Play용
  `m_PlayableInventory` 필터 목록을 전체 canonical dependency 목록으로 잘못 재사용해,
  Product에는 존재하지만 개별 Complete Play 대상이 아닌 legacy-compatible Pattern을 누락했다.
- Browser는 계속 Complete Play inventory만 표시한다. Save와 manual Stage topology Sound
  preflight만 `Gimmicks + Rotation` 전체 canonical Pattern을 수집하는 별도 helper를 사용한다.
  화면 목록과 저장 dependency closure가 다시 섞이지 않게 경계를 분리했다.
- Details와 Session Save는 마지막 결과를 `LAST SAVE: SAVED` 또는 `LAST SAVE: FAILED`로
  보존하고, 실패 시 정확한 validator 원인을 바로 아래 표시한다.
- 후속 Composition 즉시 튜닝 slice에서
  `Data/Valtan/Valtan.gameplay.json`의 `VALTAN_HIGH_JUMP/AIRBORNE` 정본을 8,000 ms로
  변경하고 Product를 다시 투영했다. LOOP animation wall과 target-axe lifetime도 8,000 ms를
  따르며, axe hit은 각 object 생성 기준 +1,200 ms를 유지한다.

### Boss Pattern Blueprint와 Details 경계

- 선택 Pattern용 일곱 번째 opt-in 창
  `Composition Boss Pattern###CompositionBossPatternWindow`를 추가했다. JSON이나 catalog를
  render에서 다시 읽지 않고, 이미 조립된 effective `VALTAN_PATTERN_VIEW`를 순수 graph model에
  투영한다.
- graph는 Stage node와 authored/derived branch edge, default/selected/maximum 경로 시간을
  표시한다. node와 edge 선택은 기존 stable selection/Details를 재사용하고, outcome 선택은
  preview-only route다. 임의 wire 생성/삭제는 아직 typed writer가 없어 read-only다.
- manual Pattern의 Stage 추가, 이동, 삭제는 Blueprint의 선택 node 도구로 옮겼다. Details에서는
  Stage duration/gap, motion, collider, reaction, Counter 등 선택 element의 수치만 튜닝한다.
  WAIT/GAP은 animation NONE인 실제 Server Stage clock으로 저장한다.
- pure graph model은 duplicate Stage/action/outcome, dangling target와 cycle을 거부하고 실패 시
  이전 snapshot을 보존한다. 네이티브 계약은 `VALTAN_DASH_CHARGE`의 default 6,050 ms,
  WALL_CONTACT 선택 11,050 ms, maximum 11,550 ms 및 determinism/hit-test를 검증한다.

### Animation source preview와 Server Valtan

- `Selected Sequence` 상세와 Preview/Replace/Append/Create 동작을 긴 virtualized 목록 위로 옮겼다.
- raw `.clipseq/.clipcuts` source는 `Layer_AnimationPreview`의 collision-free
  `isServerAuthoritative=false` Valtan clone에서만 재생한다. 버튼을
  `Preview Sequence on Arena Clone`으로 명시하고 클릭 frame에 Composition preview-owner claim도
  함께 queue해, 다른 domain deep-link 뒤 다음 update에서 preview가 즉시 정지하는 경우를 막았다.
- 실제 Arena 본체는 저장된 stable Pattern만
  `BossTool -> ValtanPatternAuditionService -> C2S PLAY_PATTERN_ID -> Server -> snapshot`으로
  재생한다. 선택 source의 `PresentationSources`를 Complete Play inventory에 역매핑하고, 한 source를
  여러 Pattern이 사용하면 explicit owner combo를 표시한다. 매핑이 없으면
  Create Pattern -> Save/Publish -> Server revision 활성화 뒤 재생하도록 안내한다. raw clip 이름을
  Server authority command로 보내거나 현재 선택 Pattern을 임의 owner로 추측하지 않는다.
- `420605/sequence 3` 지진 찍기는 돌 생성 바닥 찍기의 가장 강한 source 후보이고, 일반 그로기는
  `400430/sequence 0`, 카운터 성공 그로기는 `420631/sequence 3..5` 후보이다. 시각 검증만이면
  Effect asset 내부에 4개 rock element와 5초 지연 explosion element를 저작할 수 있다. 그러나
  개별 위치·판정·수명·despawn을 갖는 제품 돌 4개는 Effect cue가 아니라 Server combat-object
  occurrence여야 한다. 기존 timed hit/lifetime/runtime join 기반은 있으나 boss 기준 고정 4방향
  layout과 일반 combat-object Workbench 저작 adapter는 아직 후속 수직 슬라이스다.

### 이번 자동 검증

- focused Workbench/F1 Python suite: 148/148 PASS
- `ActionCompositionGraphModelContractTests`: 6/6 PASS
- `ValtanPatternAuditionServiceHarness` 전체 실행: exit 0
- 새 Client source MSVC C++20 syntax-only compile: PASS
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`: PASS
- requested Pattern coverage: PASS, Product 33 / Encounter 57
- `Publish-GameplayBalance.ps1 -Mode Validate -SkipValtanSplitProjection`: 당시 중간 revision 기준 PASS,
  57 patterns / 255 stages / 52 audition rows. 최종 canonical 수치는 61 / 277이며 문서 첫 절을 따른다.
- Client/Harness project와 filter XML parse: PASS

### 빌드와 사용자 화면 확인 대기

- 이 절 작성 당시 `Client.exe` PID 43704, `Server.exe` PID 28916, Visual Studio PID 31472가 실행 중이어서
  Client link를 포함한 표준 build를 중첩하지 않았다. focused native harness만 별도 출력으로
  build/run했다.
- 사용자가 기존 Client/Server를 닫은 뒤 새 Debug build가 필요하다. 새 EXE에서 Save 성공 표시와
  재로드 뒤 8,000 ms 유지, Blueprint node/edge 선택, raw sequence clone 재생, owning saved Pattern의
  Server Valtan 재생을 각각 확인해야 하며 이 visual smoke는 아직 PASS가 아니다.

## 2026-08-31 통합 회귀 교정 및 Debug Product 빌드

### 그래프 lifecycle

- Blueprint는 live Balance generation을 다시 읽지 않고, Sequencer와 같은 immutable Pattern view의
  generation을 입력으로 받는다. 따라서 reload와 render가 같은 frame에 교차해도 stale graph가 새
  generation으로 잘못 고정되지 않는다.
- manual Pattern의 암시적 `TIMEOUT`은 vector의 즉시 다음 Stage로 연결한다. 명시적으로 저작한
  `TIMEOUT` target과 canonical event branch는 그대로 보존한다.
- Stage insert/move/remove 성공 시 preview route override를 비우고 route generation을 올린다.
  `Reset Route`를 제공하며, topology가 바뀐 frame과 rejected projection은 node/edge hit를 받지 않는다.
- edge 선택은 source branch ordinal과 Pattern/Stage/action/outcome/target stable identity를 다시 검증한다.

### source identity와 preview owner

- Create New Pattern intake가 사용자가 고른 `(sourceActionId, sourceSequenceIndex)`를 typed request와
  promotion manifest까지 보존한다. sequence `0`도 Python publisher와 Client parser가 같은 범위로
  인정한다.
- source에서 saved Pattern을 역조회할 때 exact `PRIMARY`만 owner로 인정한다. `REFERENCE`는 재생
  후보로 승격하지 않으며, canonical generation별 reverse index를 캐시해 Resources 창에서 매 frame
  전체 Pattern inventory를 복사·정렬·스캔하지 않는다.
- arena clone이 실제 preview 시작에 성공한 뒤에만 Composition viewport ownership을 요청한다.
- saved Pattern의 Server 재생은 현재 Server-active revision과 Pattern Sound runtime readiness만
  검사한다. 별개의 local draft dirty 상태는 immutable active revision 재생을 막지 않는다.
- Composition을 처음 열 때 Boss Tool canonical graph도 같은 explicit open action에서 stage/reload한다.

### 자동 검증

- focused Python 계약: 165/165 PASS
  - Workbench regression 39
  - presentation contract 45
  - manual topology 11
  - create service 19
  - create Workbench 7
  - Animation Tool master 12
  - Sound owner 14
  - sequence identity 3
  - occurrence timing 6
  - Effect invocation 9
- `ActionCompositionGraphModelContractTests`: 9/9 PASS
- `ValtanPatternAuditionServiceHarness` 전체 실행: exit 0
- `Tools/Build/test_build_profile_contract.py`: 11/11 PASS. 새 native graph source를 허용된 focused
  Client source 표면에 명시했다.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: PASS
  - Engine, Shared, Server, Client compile/link PASS
  - Product Effect resource-root 8 cases PASS
  - compiled shader closure PASS, V1/V2 WARP pixels 각각 1352
  - receipt: `out/BuildPipeline/receipts/product.debug.receipt.json`
  - evidence: `out/BuildPipeline/runs/20260831T040229448Z-debug-product-a28bb869.json`

### 남은 사용자 판정

- 새 `Client/Bin/Debug/Client.exe`는 생성됐다. Save 후 reload 유지, Blueprint route 선택/초기화,
  clone source preview와 saved active revision Server Valtan 재생의 화면 결과는 사용자가 직접
  확인해야 하며 아직 visual PASS로 기록하지 않는다.

## 2026-08-31 Sequencer 본질 편집 루프 마감

### Sequence와 box 편집

- 선택한 source Sequence가 3 clips이면 Replace/Append 결과도 정확히 3개의 Animation box로
  materialize한다. Groggy의 명시적 start/loop/end HOLD chain만 기존 Server Stage보다 길 때
  start/end를 보존하고 loop 구간을 Stage clock에 맞춘다. start/end/one-shot clip을 반복으로
  추측하지 않는다.
- Animation box body drag는 Stage 내부 순서를 stable occurrence ID 기준으로 바꾼다. Animation
  오른쪽 trim은 source/play window를, Effect body drag와 `cue_end` 오른쪽 trim은 invocation
  occurrence를, Sound body drag는 point timing을 typed owner draft에 반영한다.
- 선택 box 공통 도구에 `Duplicate Selected Box`, `Delete Selected Box`를 추가했다. Animation,
  Effect, Sound 모두 새 stable occurrence ID를 사용하며, dependency 검증을 통과한 뒤에만 draft가
  바뀐다.
- Animation/Effect/Sound lane의 `+`는 선택 Pattern/Stage/Animation box의 stable ID를 보존한 채
  큰 `Composition Resources` 창을 열고 해당 domain tab으로 이동한다. Animation Sequence는
  Replace/Append, Effect와 Sound는 선택 resource를 Stage 뒤에 Add할 수 있다. 긴 catalog는
  `ImGuiListClipper`로 visible row만 그린다.

### 생성에서 다시 편집으로 이어지는 흐름

- `Use for Create New Pattern` 성공 시 `Composition Patterns > Create New Pattern` tab을 자동으로
  열고 focus한다. 이 화면에서 stable Pattern ID와 `Display name`을 편집하고 Validate 후 Apply한다.
- 생성 transaction이 성공하면 canonical reload 후 새 Pattern을 선택하고 `Patterns / Stages` tab으로
  돌아온다. 이어서 다른 source Sequence를 고르고 `Append to Stage Slots`를 누르면 기존 clip 뒤에
  정확한 box가 추가되고 Stage duration이 합산 wall clock만큼 늘어난다.

### 통합 preview와 branch clock

- Sequencer에 Play/Pause/Stop/Restart/Loop와 scrub을 두고 선택 branch의 실제 Stage path만 하나의
  clock으로 만든다. branch 변경은 이전 preview를 정지하고, `Play Selected Stage (All Slots)`는
  해당 Stage가 속한 deterministic path를 찾은 뒤 Arena Clone에서 Animation, Effect와 collider
  mirror를 함께 시작한다.
- Sound native clip duration inventory는 resolved Valtan model별로 캐시하며 Level 변경 또는 model
  소멸 때 폐기한다. Sound resource catalog render에서는 전체 animation scan을 하지 않고 Add command
  시점에 한 번 authoritative validation한다.

### 검증과 남은 실행 경계

- Action Composition focused contract: 93/93 PASS.
- Create service/Workbench까지 합친 관련 contract: 119/119 PASS.
- Client Debug x64 `ClCompile`: PASS. 현재 저장소의 기존 CP949/C4819 warning만 있고 compile error는
  없다.
- 최신 변경 대상 `git diff --check`: PASS(LF/CRLF 안내만 출력).
- Pattern/Animation/Effect draft는 `Save & Apply`, Sound draft는 별도 `Save Sound Owner`가 소유한다.
  저장 버튼을 누르기 전 timeline 변경은 실행 중 Server pattern 정본이 아니다.
- 이 절의 최종 Product link는 사용자가 실행 중인 Client가 output을 점유한 동안 수행하지 않는다.
  새 EXE에서 Create tab 자동 이동, Sequence Append, box Duplicate/Delete/drag/trim, branch preview의
  화면 결과는 사용자 visual 판정 전까지 PASS로 기록하지 않는다.

## 2026-08-31 Create Pattern Python 실행 별칭 수정

- Create Pattern의 `patternId`와 `displayName` 검증을 통과한 뒤에도 Python backend가 시작되지 않던
  원인은 WindowsApps의 정상 `python.exe` App Execution Alias를
  `std::filesystem::is_regular_file()`와 `weakly_canonical()`로 거부한 것이었다. 이 PC의 alias는
  0-byte reparse point지만 실제 `Python 3.14.4`를 실행한다.
- resolver는 `SearchPathW`가 찾은 고정 실행 경로를 `GetFileAttributesW`로 검사해 누락 경로와
  directory는 계속 거부한다. filesystem target resolution 없이 `lexically_normal()`만 적용해
  non-directory App Execution Alias를 `CreateProcessW` 대상으로 인정한다. 사용자 입력은 계속
  JSON request 안에만 전달하며 shell을 사용하지 않는다.
- Composition data-only 창은 preview `m_AssetName`이 비어 있어도 Create baseline SHA를 고정 정본
  `Data/Valtan/Valtan.presentation.debug.json`에서 읽는다. 선택 Sequence를 별도로 saved intake로
  저장할 필요가 없다.
- 같은 clip을 여러 recovered action이 공유할 때 CURRENT_CHAIN의 명시적 `(sourceActionId,
  sourceSequenceIndex)`가 primary presentation identity를 소유한다. `Valtan.animnotify` action은
  exact source tuple이 없는 기존 saved chain의 fallback으로만 사용한다.
- `patternId`는 파일명이 아니며 영문/숫자/`_-.` stable ID다. `.json`을 붙이지 않는다.
  한글 저작 이름은 별도 `Display name`에 입력한다.
- focused Workbench/Create/route 계약: 70/70 PASS. 현재 호스트에서 명시적 WindowsApps
  `python.exe --version`도 exit 0이다. 사용자가 입력한 `VALTAN_GROUND_TICK`, action 400440,
  sequence 0, `mesh_att_battle_11_01` CURRENT_CHAIN 전체 Validate dry-run도 PASS했다.
- 수정본 `Animation_Tool.cpp` Client Debug x64 compile과 `Client.exe` link는 PASS했다. 실행 중인
  Server/Data를 유지하기 위해 정본 Product 재투영은 하지 않았고 Client project만 다시 링크했다.
  새 EXE의 실제 Validate/Apply 화면 결과는 사용자 확인 전이다.

## 2026-08-31 Effect Tool V2 Group 공용 파이프라인 재실측

이 절은 앞 절의 완료 표현을 현재 source와 dirty worktree 기준으로 좁힌다. 다른 세션이 같은
worktree에서 구현 중이므로 아래 `구현 확인`은 현재 파일에 존재하는 경계이고, `미완료` 항목과
이번 dirty diff는 build/harness와 사용자 화면 판정 전까지 완료 증거가 아니다.

### 현재 source에서 확인한 구현

- Effect Tool V2가 `Data/Effects/V2/Authored/*.effectv2.json` leaf와
  `Data/Effects/V2/Groups/*.effectv2group.json` group body/ordered children을 읽고 저장한다.
  group child는 authored leaf만 참조하고 group nesting은 parser/validator가 거부한다.
- `BOSS_VALTAN.effectv2bindings.json` binding은 leaf 또는 group 하나와 stage 또는 clip clock 하나를
  참조한다. Composition Resources는 V2 leaf/group snapshot을 읽고 선택 Stage actionId와
  Stage-local `startMs`로 binding을 append한 뒤 binding 파일을 원자 교체한다.
- group child 배열을 Valtan Pattern JSON에 복사하는 경로는 없다. Product runtime은 binding의
  `groupId`를 읽어 group child를 펼치고 authored leaf를 재생한다.
- local Arena Clone의 `CValtan`은 local Pattern action clock과 immutable catalog snapshot을
  `CEffectV2Runtime::Sync_StageAuthoring`에 전달하고, Server Valtan은 replicated Server action age로
  Product `Sync_Stage`를 호출한다. renderer 구현은 공유하지만 source generation은 분리되며 local
  clone은 Server Pattern이나 Server-active revision을 변경하지 않는다.
- dirty `Animation_Tool.cpp`에는 Sound row의 exact owner/action/occurrence identity를 항상 검사하되,
  Sound wall clock에 영향을 주는 Stage/action/animation occurrence가 바뀐 경우에만 strict timing
  window를 다시 검사하는 no-new-debt 분기가 들어와 있다. 관련 Python oracle도 unchanged unrelated
  Stage와 changed Sound Stage를 구분하는 case를 추가했다.

### 닫힌 V2 Group 편집 계약

- Composition Resources는 현재 `V2 Authored Effects`를 먼저, `V2 Effect Groups`를 다음
  category로 표시한다. 과거의 `Advanced: V2 Individual Leaves` 라벨은 더 이상 사용하지 않는다.
  BOSS_VALTAN binding 후보는 `boss.valtan.*` owner로 제한한다.
- Effect Tool V2가 leaf body와 ordered group children을 소유하고 Composition은
  `BOSS_VALTAN.effectv2bindings.json`의 `groupId`, exact Stage action, Stage-local `startMs`와 placement만
  소유한다. Pattern JSON에 group children을 펼쳐 복사하지 않는다.
- append/remove/duplicate/update-start는 persisted binding 전체 행을 stable baseline으로 사용하는
  typed mutation이다. non-default bone/follow/rotation/offset/yaw row도 ordinal 없이 정확히 선택하며
  stale·ambiguous match는 파일과 snapshot을 바꾸기 전에 거부한다.
- C++ catalog와 Python validator는 Group이 펼치는 child leaf와 direct leaf가 같은 Stage/clip clock에
  중복 배치되는 새 binding을 거부한다. documents/groups/bindings 전체 read-set의 normalized CAS가
  성공한 뒤에만 binding 파일을 atomic replace한다.
- Sequencer는 V2 Group을 최소 240 px, V2 Leaf를 최소 180 px의 식별 가능한 block으로 그린다.
  이 폭과 sub-row packing은 눈 검증용 projection이며 Stage duration이나 Effect lifetime을 변경하지
  않는다.
- local Arena Clone은 `CEffectV2Catalog`의 immutable authoring snapshot을 명시적으로 받아 Stage를
  restage한다. Effect Tool V2 Save와 catalog save는 `CEffectV2Runtime::Invalidate_Caches()`를
  호출하며, 다음 load/preview가 저장된 catalog revision을 다시 읽는다.

### local preview와 Complete Play의 현재 경계

과거 고정 SHA/bytes world-entry manifest와 현재 authoring 파일의 불일치 때문에 정상 수정이
막히던 경로는 current typed closure와 Server gameplay revision을 사용하도록 교정했다. Workbench
Save는 Pattern/Sound/Effect V2 owner를 한 transaction으로 저장하고 catalog cache를 갱신한 뒤
canonical graph를 reload한다. local preview와 Server-authoritative Complete Play의 권위 구분은
계속 유지한다.

```text
Arena Clone authoring preview
  saved Effect V2 catalog snapshot + current path/playhead restage
  -> 저장 직후 local 확인

Server Complete Play
  saved typed closure + Server gameplay revision
  -> Server-active occurrence와 함께 authoritative replay
```

local clone에서 보인 결과만으로 Server replay나 visual fidelity까지 PASS로 기록하지 않는다.

### 이번 변경의 자동 검증 상태

- V2 clip projection, Effect invocation, Workbench regression: 59/59 PASS.
- Effect V2 validator: 24/24 PASS.
- 실제 catalog: 92 authored / 84 bindings / 4 groups / 4 independent / 56 textures PASS.
- Debug / Core와 Release / Core: PASS. 최종 evidence는 문서 첫 절을 따른다.
- `git diff --check`: PASS(LF/CRLF 안내만 출력).
- 사용자 Arena Clone/Complete Play 화면 검증은 자동 검증 범위가 아니다. visual PASS는 사용자의
  새 EXE 확인 전까지 기록하지 않는다.

## 2026-08-31 Six Pizza 고정 facing Effect root 수직 슬라이스

### 구현

- `VALTAN_SIX_PIZZA_106/STEP_01` composite cue의 source owner anchor를
  `pattern.target.snapshot`에서 `arena.center.facing`으로 교정했다. Position은 저작된 피자 착지
  중심을 사용하고, yaw는 Server가 패턴 시작 시 고정한 boss facing을 사용한다.
- `Valtan.presentation.json`만 저작 정본으로 수정했고
  `Valtan.patterneffectcues.json`은 `Project-ValtanPatternMaster.ps1 -Mode PublishV2`가 다시 생성했다.
  generated Product 직접 Save 경로는 추가하지 않았다.
- Client runtime은 `arena.center`와 `arena.center.facing` root를 한 helper에서 parse/validate/stage한 뒤
  기존 cue scale root에 합성한다. 알 수 없는 anchor나 non-finite yaw는 caller의 기존 matrix를
  바꾸지 않고 거부한다.
- Action Composition Workbench Effect Details는 `arena.center.facing`을 선택했을 때
  `Authored Landing Center / Server Locked Pattern Facing`을 명시한다. 기존
  `pattern.target.snapshot`은 `Target Snapshot Position / Player Snapshot Yaw`로 구분한다.
- authoring helper 재실행도 Six Pizza를 `arena.center.facing`으로 유지하므로 다음 publish에서
  예전 target snapshot anchor로 회귀하지 않는다.

### canonical graph 회귀 교정

- `VALTAN_GROUND_ROAR` 승격 뒤에도 남아 있던 native oracle의 `57/33` 고정값을 실제 정본
  `58 canonical = 34 managed + 24 reference`, Complete Play 34로 교정했다.
- 단순 count만 맞추지 않고 Complete Play stable-ID set이 managed canonical stable-ID set과
  정확히 같은지 검사한다. Ground Roar가 Phase 1 `MANUAL_SERVER_AUDITION`이며 Animator bucket에만
  속하는 것도 고정했다.

### 자동 검증

- `Project-ValtanPatternMaster.ps1 -Mode PublishV2`: PASS, 7개 Product 중 cue Product 1개 갱신
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS, managed 34 / legacy 26
- Six Pizza/Workbench/master projection Python: 90/90 PASS, 340.757초
- `ValtanCanonicalGraphContractTests`: 6/6 PASS
  - 실제 canonical loader 58 patterns / 256 stages
  - Complete Play 34 patterns와 managed ID exact closure
  - source Effect의 late landing/sector/finale element가 공통 fixed root를 공유함
  - 잘못된 anchor와 non-finite facing 실패 시 기존 matrix 보존
- `ValtanPatternAuditionServiceHarness` 전체: exit 0
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core`: PASS
  - Engine, Shared, Server, Client compile/link PASS, compile error 0
  - Product Effect WARP V1/V2 각각 1352 pixels
  - presentation generation admission 및 Character Select isolation Core PASS
  - evidence: `out/BuildPipeline/runs/20260831T120436528Z-debug-core-a3242e36.json`
- `git diff --check`: PASS. 기존 LF/CRLF 안내만 출력했다.

### 아직 PASS가 아닌 경계

- 이 슬라이스는 패턴 시작 시 고정된 facing으로 composite Effect 전체를 한 번 회전시키는 계약이다.
  시간에 따라 계속 회전하는 root animation과 개별 element의 독립 회전 저작은 구현하지 않았다.
- Client를 자율 실행하지 않았다. 사용자가 `VALTAN_SIX_PIZZA_106` Complete Play로 착지 중심과
  sector/late element 방향을 확인하기 전까지 visual PASS가 아니다.
- 이 절 작성 당시에는 Release build가 없었지만, 최종 통합 tree는 이후 Release / Core를
  `out/BuildPipeline/runs/20260831T213024268Z-release-core-f1dffc84.json`으로 통과했다.
- 최종 LAN sync에서 이 PC는 `server-host`이며 TCP 7777 LocalSubnet 방화벽 rule도 ready로
  확인됐다. 실제 Server listening과 사용자 visual smoke는 별도 실행 경계다.

## 2026-09-01 Animation box 교차 Stage 이동과 Save 회귀 교정

### 구현

- Sequencer의 Animation body drag는 같은 Stage 안의 순서 변경뿐 아니라 드롭한 X 좌표의 editable
  Stage로 occurrence를 옮긴다. source/target Stage draft를 먼저 각각 검증하고 두 draft를 한 generation으로
  적용하며, 어느 한쪽이라도 실패하면 Pattern draft와 dirty/validation 상태를 모두 이전 값으로 되돌린다.
- V1 Effect, V2 Effect, Sound 또는 Shake가 exact occurrence에 연결된 Animation box는 참조를 고아로
  만들지 않도록 교차 Stage 이동을 명시적으로 거부한다. 새로 Append한 dependency-free Sequence box는
  CHARGE에서 RECOVERY 같은 다른 Stage로 옮길 수 있다.
- Animation/Effect/Sound만 바뀌어 Stage와 action stable ID 순서가 그대로인 Save는 manual Stage topology를
  재작성하지 않는다. 따라서 `VALTAN_TRASH/RETRY_EXHAUSTED` 같은 정상 분기 그래프가 선형 topology gate에
  걸려 전체 Composition Save를 막던 회귀를 범용적으로 제거했다. 화면의 Sound 보존 문구는 atomic rollback
  공통 메시지였으며 Sound cue 누락이 원인이 아니었다.

### 함께 반영된 저작 화면

- Animation Sequence Tree의 기본 필터는 All Categories다. Valtan, Kakul, Large Kakul, Saydon source가
  category/profile/mode 경로로 함께 보인다.
- Composition Resources와 Sequencer의 최초 기본 Y 크기를 기존 계산값의 2배로 올렸다. 두 창은 계속
  가장자리와 모서리로 resize 가능하며, 새 hidden ImGui ID를 사용해 과거 `imgui.ini`의 작은 저장 크기가
  새 기본값을 가리지 않는다.

### 자동 검증

- Action Composition, topology, Sound owner, resource category, window layout와 Boss Pattern Flow 관련
  확장 회귀: 204/204 PASS.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: PASS, exit 0.
  - Engine, Shared, Server, Client compile/link PASS
  - `Client/Bin/Debug/Client.exe` 생성
  - Product Effect resource-root 8 cases PASS
  - compiled shader closure PASS, V1/V2 WARP pixels 각각 1352
  - receipt: `out/BuildPipeline/receipts/product.debug.receipt.json`
  - evidence: `out/BuildPipeline/runs/20260901T045148312Z-debug-product-24c4afaa.json`
- `git diff --check`: PASS. 기존 LF/CRLF 안내만 출력했다.
- 새 실행 파일의 실제 drag, Save/reload와 화면·음향 결과는 사용자 판정 전까지 visual PASS로 기록하지
  않는다.

## 2026-09-01 후속 — canonical sequence Save/Restart와 최초 발탄 presentation 복구

### sequence Save 실패의 본질 원인과 정본 교정

- Boss 순서를 별도 `ValtanBossAuditionFlows.json`과 Product rotation이 각각 소유하던 이중 정본을 제거했다.
  유일한 authoring owner는 `Data/Valtan/Valtan.gameplay.json > decisionModel.scriptedSequence`이며 현재
  `patternIds` 31개와 `interStepPursuitMs=1000`을 직접 소유한다.
- `CValtanPatternFlowDocument`는 물리 파일 reader/writer가 아니라 inline sequence의 in-memory adapter다.
  `Save Flow`는 slot을 `SET_SCRIPTED_SEQUENCE` typed patch로 바꾸고 gameplay/presentation 및 generated
  Product closure를 shared canonical writer transaction으로 commit한다.
- `Data/Encounters/Valtan/ValtanBossAuditionFlows.json`, saved-Flow reader/resolver/publisher, 별도
  `PublishSavedFlow` CLI/PowerShell/Client service와 project 등록을 제거했다.
- source CAS는 canonical authoring/dependency owner만 해시한다. Encounter/Rotations/Bindings/Cues 같은
  generated Product drift는 source revision을 바꾸거나 Save를 선행 차단하지 않으며 같은 candidate에서
  재투영한 뒤 postcondition으로 확인한다.

### Save 결과와 writer lock

- canonical writer lock timeout은 30초이며 `CANONICAL_TRANSACTION_BUSY`에 owner PID, operation,
  acquisition age와 SELF/ANCESTOR/OTHER/UNKNOWN 관계를 보존한다. lock 파일은 retained byte-range lock이므로
  삭제로 우회하지 않는다.
- 물리 commit receipt 뒤 editor reopen이 실패하면 `COMMIT_SUCCEEDED_REOPEN_FAILED`, exact revision까지
  reload되면 `COMMITTED_AND_RELOADED`로 구분한다. 전자는 `true`를 반환해 이미 저장된 transaction을
  `Nothing was saved`로 오인하거나 반복 commit하지 않는다.
- Action Composition의 Save도 외부 commit 성공 뒤 Boss/Workbench local reload 실패를 물리 저장 실패로
  뒤집지 않는다. 다른 Pattern 선택은 pending stable ID를 보존한 `Save All & Switch / Discard Listed Drafts
  & Switch / Cancel` modal을 사용한다.

### Restart와 최초 발탄 표시

- canonical Save 뒤 같은 saved head를 immutable candidate로 publish하고 Debug Server 2PC apply를 준비한다.
  현재 실행 R1은 계속 R1을 pin하며 중간 교체되지 않는다.
- `Restart Flow`는 최신 saved candidate와 Server-active gameplay definition revision이 exact-equal일 때만
  gameplay sequence를 다시 읽고 첫 slot부터 시작한다. presentation admission 뒤 revision을 다시 비교해
  TOCTOU로 다른 definition을 시작하는 것도 막는다.
- Server 회귀는 기존 Flow R1 실행 중 catalog R2 commit 후 Restart가 새 saved 배열 첫 slot과 R2를 함께
  pin하는 경우를 검증한다.
- 최초 Arena의 authoritative boss spawn은 presentation generation lock 실패와 분리했다. Win32 33 같은
  transient canonical read lock이면 boss snapshot을 폐기하거나 영구 reject revision으로 latch하지 않고
  250ms bounded retry로 exact presentation closure를 다시 admit한다. Server gameplay entity와 damage authority는
  presentation 일시 실패 때문에 삭제하지 않는다.

### Sequencer와 자동 검증

- Composition Resources/Sequencer 기본 높이를 2배로 키우고 timeline row를 48px, canvas 최소 높이를 420px로
  확대했다. 창은 계속 resize 가능하며 새 hidden ImGui ID로 이전 작은 `imgui.ini` 기본값을 격리한다.
- 외부 duplicate validate mode의 live C++/Python/PowerShell/project caller는 0건이다. canonical 명령은
  `Project-ValtanPatternMaster.ps1 -Mode Validate` 하나를 사용한다.
- canonical `Validate`: PASS, managed 38 / legacy 25 / generated artifact 9,
  source revision `783b7cfb61d7d949491bc9d162a2ce14764c3b90992073a79bf4d09b04c762bd`.
- Balance/Boss Flow/PatternTree focused Python: 91/91 PASS.
- Composition atomic/workbench/topology/timing/Sound/resource/spawn focused Python: 113/113 PASS.
- canonical writer diagnostics focused: 18/18 PASS.
- canonical typed transaction + full Pattern master suite: 78/78 PASS.
- Valtan native harness: Audition 29/29, Flow 13/13, Tuning 9/9, canonical graph 4/4,
  Action Composition graph 9/9, Boss Logic view 4/4 및 presentation/encounter 문서 계약 PASS.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: PASS.
  evidence `out/BuildPipeline/runs/20260901T075418571Z-debug-product-62b8948a.json`.

Client/UI를 자율 실행하지 않았다. 첫 Arena 발탄 모델 가시성, Append → Save 후 물리 reload, dirty Pattern
전환 modal, Restart 01 재생과 실제 Effect/Animation/Sound 결과는 새 Debug Client에서 사용자가 직접 판정한다.

## 2026-09-01 후속 — Sequencer admission과 Boss Flow 저작 경계 교정

### 빈 Sequencer의 실제 원인과 정본 분리

- 화면의 `0 canonical patterns`와 빈 Sequencer는 Animation, Effect 또는 Sound resource가 없어서
  발생한 렌더 문제가 아니었다. 저장된 Boss audition Flow는 첫 `VALTAN_WHIRLWIND` 뒤에
  `VALTAN_DASH_CHARGE` 두 occurrence가 추가된 31개 node였지만, Product
  `ValtanPatternRotations.json`의 자동 scripted sequence는 29개였다. canonical loader가 두 독립
  order를 exact-equal로 비교해 `Valtan scripted-sequence Product parity drifted`로 전체 graph를
  fail-close한 것이 직접 원인이었다.
- `Valtan.gameplay.json`의 `flowId`가 실제 saved Flow를 가리키는 경우에는 saved Flow가 Boss Tool
  audition의 occurrence order와 pursuit를 소유한다. Product rotation은 자동 전투의 독립 order를
  계속 소유한다. loader는 두 문서 모두의 schema, sequence identity/mode, stable Pattern 존재를
  검증하지만 saved-Flow reference에 Product order/pursuit equality를 요구하지 않는다. legacy inline
  sequence는 기존 exact Product parity를 유지한다.
- 일반 `PublishV2`는 29개 Product 자동 rotation을 31개 audition Flow로 암묵 교체하지 않는다.
  saved Flow를 Product에 승격하는 작업은 별도 명시 옵션에서만 가능하다. 따라서 Sequencer admission
  복구가 Boss Tool의 Save/Restart를 Product publish나 2PC 완료에 종속시키지 않는다.

### Boss Verification과 Pattern Flow

- `Boss Verification`의 왼쪽 목록을 `All Patterns`와 `Current Patterns`로 분리했다. All은 admitted
  전체 inventory이고, Current는 unsaved draft가 아니라 `CValtanPatternFlowDocument`의 저장
  baseline을 stable slot ID 순서로 읽는다.
- `Save Flow`의 disk transaction이 성공하면 같은 frame에 saved baseline과 Current Patterns가
  갱신된다. 실행 중인 이전 revision은 이미 복사해 보낸 slot payload를 끝까지 유지하며, 화면은 이전
  revision이 실행 중임을 별도로 표시한다. `Restart Flow`는 저장 문서를 다시 읽은 뒤 그 saved order의
  첫 slot부터 replacement request를 보낸다.
- 실행 중 Flow 자체는 Add, Up, Down, `Discard Selected`, Save를 막지 않는다. unresolved Start나 다른
  Server command만 authoring을 잠근다. 따라서 Add → Save 뒤 다시 Add/정렬/저장하는 반복 튜닝이
  가능하다. `Discard Selected`는 선택 slot 하나만 제거하고, 상단 `Discard Changes...`는 저장 baseline
  전체로 되돌리는 별도 명령이다.
- Boss Tool Save는 이 Debug audition Flow 문서만 저장한다. Product 자동 rotation publish를 조용히
  연동하지 않으며, 현재 실행을 바꾸는 경계는 사용자가 누르는 `Restart Flow`다.

### 읽기 전용 Logic Flow

- Boss Tool의 세 번째 `Logic Flow` tab은 현재 Server Flow/Pattern을 먼저 표시하고, admitted Pattern의
  Stage를 role과 실제 clip 이름의 box로 투영한다. authored branch의 `COUNTER_HIT`와 `TIMEOUT`도 같은
  graph에서 연결해 현재 animation과 분기 관계를 함께 읽을 수 있다.
- 이 화면은 기존 Action Composition graph model의 read-only projection을 재사용한다. 기본 selection
  mutation은 비활성화되어 있고 Boss gameplay, Flow 또는 Composition source를 저장하지 않는다.
  불완전한 Counter 계약도 정상 badge로 추측하지 않고 경고 상태로 남긴다.

### 자동 검증 증거

- `Project-ValtanPatternMaster.ps1 -Mode Validate`: PASS.
- Composition/Build focused Python contract: 108/108 PASS.
- Valtan Pattern Tree focused contract: 27/27 PASS. saved Flow reference에서는 독립 order/pursuit를
  허용하고, inline sequence에서는 exact parity를 유지하는 정상·실패 fixture를 포함한다.
- Logic Flow ViewModel contract 4/4와 공용 Action Composition graph model contract 9/9: PASS.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: PASS.
  - evidence: `out/BuildPipeline/runs/20260901T060803420Z-debug-product-95798602.json`
  - 19 steps, FAIL 0. Engine, Shared, Server, Client compile/link와 compiled-shader closure를 포함한다.
  - `Client/Bin/Debug/Client.exe` 생성.
- 위 수치는 이번 후속 slice의 focused/build checkpoint다. dirty worktree 전체의 최종 통합 테스트는
  root 세션이 모든 agent 변경을 합친 뒤 별도로 실행하고 그 결과를 최종 증거로 기록한다.

### 아직 완료로 기록하지 않는 사용자 확인

- 에이전트는 Client를 실행하거나 ImGui를 자동 조작하지 않았다. 사용자는 새 Debug EXE에서
  Workbench의 Pattern 선택과 Sequencer box가 다시 나타나는지, Preview/Play가 실제 재생되는지를 직접
  확인해야 한다.
- Boss Tool에서는 Current Patterns가 Save 직후 바뀌는지, 실행 중에도 Add → Save → Add와
  Up/Down/Discard Selected가 반복되는지, Restart가 새 저장 순서의 첫 Pattern부터 시작하는지를 확인해야
  한다. Logic Flow의 현재 Pattern clip과 `COUNTER_HIT`/`TIMEOUT` 연결도 같은 수동 smoke 대상이다.
- 위 화면·재생·음향 결과는 사용자의 서면 판정 전까지 visual PASS가 아니다.

## 2026-09-01 후속 — 실사용 Composition Save 회귀 교정

### 서로 다른 두 Save 실패

- `Invalid number at byte 0`은 V2 Effect 데이터 자체의 parse 실패가 아니었다. 실행 중이던 구 Client는
  formatVersion 1 Effect binding의 baseline/candidate만 전달했지만, 갱신된 외부 wrapper가 V2 read-set까지
  선행 요구하면서 Python의 structured JSON 결과 전에 일반 PowerShell 오류를 출력했다. v1 pair는 기존 strict
  validator와 baseline CAS로 저장하고, 실제 v2 dirty pair만 read-set을 필수로 요구하도록 호환 경계를 교정했다.
- `player silence ENTER must match the Stage clock`은 위 오류와 별개다. `SET_STAGE_DURATION`이 Stage를
  5000ms에서 2633ms로 줄인 뒤에도 `SET_PLAYER_SILENCE`/`SET_PLAYER_BIND`의 ENTER window를 5000ms로
  남겨 최종 validator가 거부했다. 이제 같은 typed operation이 ENTER duration을 Stage clock에 맞춰 함께
  갱신하고 EXIT 0ms는 보존한다.
- 외부 pipeline이 structured JSON 전에 실패하는 경우에는 원문을 bounded diagnostic으로 표시한다. 이후 같은
  종류의 wrapper 오류를 `Invalid number at byte 0`으로 가리지 않는다.

### Effect V2와 Server Combat Object 구분

- 화면의 `effect.valtan.ground-roar.rock.active x4`는 삭제 대상 Effect V2 binding이 아니라 Server gameplay가
  네 방향으로 spawn하는 combat object visualization이다. 새 UI는 이를
  `Server Combat Object (read-only) x4`로 표시하고 선택 상세를 열되 Duplicate/Delete를 비활성화한다.
- 실제 V2 row는 `V2 Leaf` 또는 `V2 Group` owner로 구분한다. V2 row를 삭제해도 Server combat object row는
  남는 것이 정상이다.

### 물리 정본, revision과 검증

- 실패한 Save는 `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`을 보존했다. 현재 물리 문서는
  formatVersion 1, 15 rows, SHA-256
  `72FCD1CAA47F5C1462F6619EB832A429612F26C0452BAD6B446293C881AA1B85`다.
- 이전 `783b7cfb...`에서 현재 `deff0af...`로 바뀐 source revision은
  `Valtan.presentation.json`에 ground-roar cardinal-rocks Server Combat Object owner 1행을 추가한 결과다.
  Build/ClCompile이 source를 바꾼 것이 아니며, 다음 Client 시작은 현재 물리 head
  `deff0af125ed75ebc6e972c0586ffc9aed06172767b2e12f149d93ba83c49754`를 읽는다.
- focused 회귀 3/3 PASS: public wrapper v1 Effect delete structured commit, Stage duration status-clock cascade,
  combat-object read-only selection.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: PASS.
  - evidence: `out/BuildPipeline/runs/20260901T093323521Z-debug-product-70c82900.json`
  - 새 `Client/Bin/Debug/Client.exe`: 2026-09-01 18:33:03 KST,
    SHA-256 `37918FA2CAD1145F5E0291E79CAA17FBD5454EA04ED59B39D8D403A1FA5952F6`
- Client/UI를 에이전트가 실행하지 않았다. 실제 V2 row 재삭제 → Save → 물리 reload, Silence 2633ms Save,
  Restart 재생과 화면 결과는 사용자의 수동 smoke 전까지 PASS로 기록하지 않는다.

## 2026-09-01 후속 — Sequence 물리 Save와 Collider 타임라인 저작 닫기

### Sequence 삭제 Save의 실제 원인과 교정

- `VALTAN_GROUND_ROAR`에 Append한 Sequence를 삭제해도 `presentationSources`와
  `sourceActionIds`가 남아 `sequence append removed immutable promotion intake`로 Save가 거부되던 것이
  마지막 직접 원인이었다. manual audition의 `SET_STAGE_ANIMATION`에서 deterministic reference span 전체가
  사라진 경우에만 그 provenance와 미참조 source action을 함께 제거한다. 일부 occurrence만 삭제하거나
  다른 occurrence로 교체한 경우는 계속 fail-close한다.
- public PowerShell Save wrapper를 임시 저장소에서 실행해 Append 다섯 slot과 action `420617` provenance가
  실제 `Data/Valtan/Valtan.presentation.json` 및 `Valtan.gameplay.json`에서 제거되고, 남은 primary clip과
  Server combat-object action은 보존되는 것을 확인했다.
- 현재 source validate를 막던 Whirlwind Recovery cross cue의 clip join을 실제
  `valtan.attack.whirlwind.recovery.clip.01` owner로 교정했다. generated cue를 별도 정본으로 만들지 않았다.

### Collider 타임라인과 Box Detail

- Collider box 몸통 drag는 Stage-local hit schedule 전체를 같은 delta로 이동한다. explicit offsets는 모든
  pulse를 함께 옮기고 interval schedule은 first delay만 옮기므로 간격과 hit count는 보존된다. Stage 밖으로
  나가지 않도록 첫/마지막 pulse 기준으로 clamp한다.
- 왼쪽/오른쪽 edge는 각각 첫/마지막 pulse만 바꾼다. interval schedule을 edge 편집하면 exact explicit
  offsets로 materialize하고, 1-pulse collider는 순간 pulse이므로 resize를 거부하고 body move만 허용한다.
- Collider box 폭과 edge handle은 label 최소 폭이 아니라 실제 first/last pulse clock을 사용한다. 따라서
  `250 ms -> 900 ms` 같은 짧은 구간도 라벨 폭에 가려지지 않고 이동/resize 결과가 보인다.
- Box Detail의 circle/ring radius, cone angle/length, box/cross/six-directions length/half-width는 meter 단위
  clamped drag control로 편집한다. 기존 typed `SET_STAGE_HIT`와 공용 Composition Save만 사용하며 별도 collider
  파일이나 Client-only runtime owner를 만들지 않았다.
- 실행 중 타임라인 schedule 편집은 최신 `Get_ValtanPatternDraft` 값 복사본으로 Arena Clone을 같은 playhead에
  restage한다. Box Detail 크기 drag는 매 frame clone을 재생성하지 않고 편집 후 `Play/Restart`에서 최신
  draft를 읽는다.
- 기존 `CValtan` local authoring mirror를 그대로 사용한다. 발탄의 현재 transform/look 기준으로 Stage 전체에는
  amber wire, 실제 pulse 뒤 300ms에는 pink wire를 그리며 Server hit 판정 권위는 갖지 않는다.

### HOLD_LAST Stage preview

- Animation wall이 Stage보다 짧은 `HOLD_LAST_POSE`를 잘못 거부하던 Master timeline을 교정했다. 마지막
  playlist item과 전체 timeline을 Stage duration까지 연장해 마지막 pose, collider debug clock과 다음 Stage
  경계를 같은 Server clock에 유지한다.
- animation wall이 Stage보다 긴 경우는 계속 거부하고, `EXACT`/`LOOP_TO_STAGE_END`의 underfill도
  fail-close한다. 따라서 화면의 `11800 ms Stage / 1800 ms Animation / 10000 ms gap`은 preview 가능하다.

### 자동 검증과 실행 파일

- Collider/HOLD/Sequencer/collision focused: 36/36 PASS. public wrapper가 임시 물리
  `Valtan.gameplay.json`에 `CONE 110° / 14 m / offsets [250, 900]`을 실제 commit하는 검증을 포함한다.
- Sequence provenance, manual Stage authoring, v1 Effect delete, Stage status-clock cascade, read-only Server
  combat-object 회귀: 24/24 PASS.
- Action Presentation Workbench 광역 계약: 45/45 PASS. saved Product revision 인자가 추가된 현재
  `Start_FlowAtSlot`/`Start_Flow` 호출을 옛 무인자 문자열로 검사하던 oracle도 현재 계약으로 갱신했다.
- `Project-ValtanPatternMaster.ps1 -Mode Validate`: PASS, source revision
  `fa4d3645159c82d286994efd3da2ec8aba6f80f17c1f9625703dfd6c9d12b519`, managed 38 / legacy 25 /
  projected artifact 9.
- 주요 Valtan JSON 7개 parse PASS, `git diff --check` PASS(기존 LF/CRLF 안내만).
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: 19 steps 모두 PASS 또는 REUSED,
  compile/link와 compiled-shader closure PASS. evidence:
  `out/BuildPipeline/runs/20260901T101534864Z-debug-product-23de9d7c.json`.
- 생성된 `Client/Bin/Debug/Client.exe`: 2026-09-01 19:14:19 KST, 37,190,656 bytes,
  SHA-256 `D9CC831B7D6D440D64552932AE9181EC0DA04D9B260EC496CCE0225BC850C16B`.

Client/UI를 에이전트가 실행하지 않았다. 실제 body/edge 포인터 감각, Box Detail 변경 뒤 Restart의
발탄-local wire, Save 후 재선택/reload, 첫 Arena 발탄 표시와 실제 Effect/Animation/Sound 결과는 사용자의
수동 smoke 전까지 visual PASS가 아니다.

## 2026-09-01 후속 — Effect V2 CRLF baseline CAS 오탐 교정

- 실제 `BOSS_VALTAN.effectv2bindings.json`은 Git 내용 변경 없이 CRLF 22개인 3,683-byte 물리 파일이었다.
  Composition의 기존 C++ draft baseline은 이 문서를 parse한 뒤 LF 22개인 3,661-byte 문자열로 다시
  직렬화했다. Python canonical writer는 제공 baseline과 물리 owner를 raw bytes로 CAS하므로 의미상 같은
  JSON을 `changed after the Composition draft began`으로 오판했다.
- Effect V2 삭제/이동 뒤 dirty draft가 유지되면 이후 Sequence Save도 같은 Effect owner pair를 포함한다.
  실패 시 dirty draft를 보존하므로 Save를 반복해도 Pattern/Sound/Effect 전체 원자 transaction이 계속
  rollback됐다. Sequence topology나 실제 외부 writer 변경이 원인이 아니었다.
- `Stage_BossValtanBindings`가 유효한 물리 owner의 raw bytes를 함께 stage하고, 첫 Effect draft가 그 raw
  bytes를 exact baseline으로 pin하도록 교정했다. Save 직전 C++ CAS와 Python writer CAS가 같은 표현을
  비교하므로 CRLF/LF 오탐은 사라지고, 실제 owner byte 변경은 계속 fail-close한다.
- CRLF 물리 owner를 명시적으로 만드는 public wrapper 회귀와 C++ raw-baseline 경계 oracle을 추가했다.
  focused atomic Save 6/6, canonical typed patch/public wrapper 13/13 PASS, `git diff --check` PASS.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: PASS. evidence:
  `out/BuildPipeline/runs/20260901T105637322Z-debug-product-a61f8da5.json`.
- 새 `Client/Bin/Debug/Client.exe`: 2026-09-01 19:56:13 KST, 37,190,144 bytes,
  SHA-256 `F387389BDC77AE67E8D845B1F5B57EDD37ADEEC5399AC94084AC5967921D1067`.

Client/UI는 에이전트가 실행하지 않았다. 새 EXE에서 V2 row 삭제와 Sequence 삭제를 같은 draft에 둔 뒤
Save하여 dirty 해제, 물리 owner commit, 재선택 reload를 확인하는 것은 사용자 수동 smoke 경계다.

## 2026-09-01 후속 — Silence HUD 붉은 icon/cooldown presentation

### 구현

- `VALTAN_SILENCE_SLOT`의 기존 Server 권위 `iSilenceEndTick/iSilenceDurationTicks`와
  `CCombatHUDViewModel` cooldown projection은 변경하지 않았다. Client local 5초 timer나 별도 UI를
  만들지 않고 기존 `CUILayoutRuntime -> CUI_Sprite` quick-slot을 재사용한다.
- 침묵 deadline이 현재 Server tick보다 미래인 동안 Q/W/E/R/A/S/D/F/T/V 아이콘에는
  `(1.0, 0.2, 0.2, 1.0)` tint multiplier를 적용한다. Warlord/Artist Z/X keyframe 아이콘 네 개에도
  같은 multiplier를 적용하며 숨은 owner slot까지 매 frame white로 복원해 class 전환 뒤 상태가 남지 않는다.
- 기존 `Skill_<slot>_Cooldown` sweep은 침묵 동안 붉은 반투명으로 표시하고 종료·cancel snapshot에서는
  authoring의 검은 `150/255` alpha로 복원한다. 더 긴 실제 skill cooldown이 남으면 red만 종료되고 기존
  검은 arc/countdown은 계속된다.
- `CUI_Sprite`의 새 tint multiplier는 authored/static tint와 keyframe alpha를 대체하지 않고 shader bind
  직전에 component-wise 곱한다. 새 texture, draw call, UI JSON 또는 Server/Data source 변경은 없다.

### 자동 검증

- `python -B -m unittest Tools.ValtanPipeline.test_valtan_status_pattern_contract`: 8/8 PASS.
  Server deadline 소비, red/black 복원, 기존 cooldown slot/arc 재사용, 일반 및 Z/X icon multiplier와
  HUD JSON의 10개 기존 overlay를 source/data oracle로 확인한다.
- `python -B -m unittest Tools.Build.test_release_client_surface_contract`: 4/4 PASS.
- `Project-ValtanPatternMaster.ps1 -Mode Validate`: PASS, managed 38 / legacy 25 / projected artifact 9,
  source revision `166c60107d7aabaaa5642899493a6ab51fdf2aa1737110018a2885a4eb3408a3`.
- `Client.vcxproj /t:ClCompile` Debug x64: exit 0. 변경된 `MainApp.cpp`, `UILayoutRuntime.cpp`,
  `UI_Sprite.cpp`를 포함한 Client compile은 통과했다.
- 전체 `git diff --check`: PASS. 기존 LF/CRLF 변환 안내만 출력됐다.
- 표준 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`는 사용자가 실행 중인
  `Client.exe`와 `Server.exe`를 output guard가 감지해 link 전에 중단했다. 에이전트는 두 process를
  종료하지 않았고 새 EXE를 만들지 않았다.

### 사용자 수동 검증 경계

- 현재 실행 중인 EXE는 이 변경 전 바이너리다. Client/Server를 사용자가 종료한 뒤 Debug Product를 다시
  build하고 `Server + Client` profile로 재실행해야 새 presentation을 볼 수 있다.
- `VALTAN_SILENCE_SLOT`은 계속 `AUDITION_ONLY`, selection weight 0이며 일반 rotation에는 넣지 않았다.
  Boss Tool에서 이 Pattern을 명시 실행해 약 5초 red icon/arc/countdown, skill 거부와 이동 유지,
  종료·cancel의 white/black 복원을 확인해야 한다. 사용자의 서면 관찰 전까지 visual PASS가 아니다.

## 2026-09-01 후속 — Six Pizza target-follow 회전 root

### 구현

- `VALTAN_SIX_PIZZA_106`은 `LOCK_RANDOM_ALIVE_ON_START + TRACK_TARGET_EACH_TICK`을 사용한다.
  occurrence 시작 때 선택한 `PlayerId`는 바꾸지 않고, Server fixed tick마다 authored landing center
  `(156.03, 22.99751, -122.06)`에서 해당 플레이어의 현재 XZ로 향하는 yaw를 계산한다. 첫 800ms
  center approach 중에도 움직이는 boss 위치를 pivot으로 사용하지 않는다.
- 기존 boss pattern snapshot의 current yaw가 계속 Server 권위다. Shared message나 protocol field를 새로
  추가하지 않았다.
- Six Pizza composite cue는 `arena.center.target-follow + follow`를 사용한다. Client는 첫 cue spawn의
  world-root handle과 pattern sequence/target identity를 보관하고, 같은 occurrence의 승인 snapshot마다
  `Update_WorldRoot`로 동일 handle의 회전만 갱신한다. 새 sequence, pattern 종료, target 불일치·누락,
  비유한 yaw 또는 root 재구성 실패에서는 다른 플레이어나 0도로 fallback하지 않고 updater만 떼어 마지막
  유효 root를 고정한다. 이미 예약된 NATURAL delayed element는 그 root에서 끝난다.
- source Effect의 정적 sector particle 세 개
  (`requested.20260827.six-pizza.sector.yellow-05`,
  `authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.1`,
  `authored.copy.authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.1.1`)를
  `localSpace: true`로 바꿨다. 이미 보이는 sector와 이후 burst/finale가 공통 회전 root를 따른다.
- Product loader, source authoring, Workbench와 pipeline validator는 위 exact 조합만 admit한다. 기존
  `arena.center + snapshot`, `arena.center.facing + snapshot` 의미는 바꾸지 않았다.

### 자동 검증과 컴파일

- `Project-ValtanPatternMaster.ps1 -Mode PublishV2`: PASS, projected artifact 7개 확인.
- `Project-ValtanPatternMaster.ps1 -Mode Validate`: PASS, managed 38 / legacy 25 / projected artifact 9,
  source revision `1dbafae24a8c056fc3cf58411182cb70681c4b9778cbb68d2b83d2b94f1eb717`.
- target-follow focused Python: 6/6 PASS. Master V2 exact projection/invalid-combination/typed draft: 3/3 PASS.
- 기존 Action Composition Effect invocation/presentation Workbench: 55/55 PASS.
- `ValtanPatternAuditionServiceHarness` Debug build: PASS. target-follow canonical graph contract 4/4 PASS,
  갱신한 Encounter Product target/motion admission과 68 rejection/rollback cases PASS.
- Server Debug x64 `ClCompile`: exit 0. `ValtanBrain.cpp`를 포함한 Server 번역 단위가 컴파일됐다.
- Client Debug x64 `ClCompile`: exit 0. `Valtan.cpp`, cue Product/source parser, Pattern Tree, Workbench와
  Animation Tool을 포함한 Client 번역 단위가 컴파일됐다. 기존 C4819/C4828 인코딩 경고만 남았다.
- 주요 JSON parse, pipeline Python compile, task 파일 `git diff --check`: PASS.
- 전체 native harness 재실행은 현재 멀티세션 worktree의 Create/Project transaction lock(Win32 33)과
  독립 Pattern Sound document count 불일치 때문에 exit 1이다. 이번 변경의 Encounter와 target-follow
  검증은 통과했으며, 광역 통합 회귀 결과로 승격하지 않는다.

### 통합 빌드와 사용자 검증 경계

- 사용자 요청에 따라 다른 세션 변경을 취합하기 전 Product/Core 링크·광역 회귀는 실행하지 않았다.
  이 변경 단위는 컴파일 오류 없이 인계하며, 최종 통합 빌드 결과는 취합 세션에서 기록한다.
- Client/UI는 에이전트가 실행하지 않았다. 사용자는 새 통합 EXE에서 `VALTAN_SIX_PIZZA_106`을 실행하고
  시작 때 선택된 한 플레이어가 arena center 둘레를 움직일 때 이미 보이는 sector가 회전하는지, 이후
  19.5초/23초/28.5초 element도 그때의 동일 root 방향으로 생성되는지 확인해야 한다. 사용자 서면 확인
  전까지 visual PASS가 아니다.

## 2026-09-01 후속 — Ground Roar 4방향 돌 Independent 외형 확정

- `valtan.independent-effect.ground-roar-cardinal-rocks`는 `INDEPENDENT EFFECT`에 한 행만 둔다.
  `VALTAN_GROUND_ROAR/STEP_01`의 Server `SPAWN_COMBAT_OBJECT_VOLLEY`가 반경 2.25m,
  yaw `0/90/180/270`에 `combatobject.valtan.ground-roar.rock` world root 네 개를 만든다.
- 각 world root는 `effect.valtan.ground-roar.rock.active`의 단일 `kind=mesh` Element를 재생한다.
  문서에 mesh 네 개를 중복 저장하지 않으므로 Product 결과도 정확히 네 돌이다.
- active Element는 사용자가 확정한 CROSS 돌 외형을 재사용한다:
  `fm_d_stoneparts_003.wmodel`, electric base, stoneparts noise, fluid mask, noise dissolve,
  `opaque_back_depth_write`, 검정/청록 multiply와 `0.01` model pre-scale이다.
- standalone mesh 수명은 `detail.timing.lifeTimeSeconds=5.0`이 소유한다. `0.96`부터 마지막
  0.2초 hard dissolve가 진행되고, Server의 5000ms terminal explode pulse/despawn과 맞물린다.
- `test_valtan_combat_object_hit_effect_presentation_contract`: 15/15 PASS.
- independent authored 5000ms -> Product `lifeMs` 보존 focused test: 1/1 PASS.
- `Validate-EffectSources.ps1`: PASS, direct sources 177 / unbound references 0.
- `Project-ValtanPatternMaster.ps1 -Mode Validate`: PASS, managed 38 / legacy 25 / combat objects 4.
- Client/UI는 실행하지 않았다. 사용자는 Effect Tool `INDEPENDENT EFFECT`의
  `땅구르기 후 사자후 / 4방향 돌`에서 `Play Combat Object Lifecycle`로 네 위치·크기·5초 소멸을
  직접 판정한다.

## 2026-09-02 후속 — 망령 본체·4포탈·Pizza 180°·Effect Save·V1 Decal·Fresh Restart

### 구현 상태

- `VALTAN_GHOST_RESPAWN_AUDITION/STEP_01` ENTER가 gameplay phase 3을 확정한다. respawn 완료 뒤
  primary `BOSS_VALTAN`이 `VALTAN_SIX_PIZZA_106 → VALTAN_GROUND_ROAR →
  VALTAN_STAGGER_SLOT → VALTAN_BIND_SLOT → VALTAN_SILENCE_SLOT →
  VALTAN_TRIPLE_COUNTER`를 ordered loop로 반복한다. 별도 child boss를 damage authority로 승격하지
  않았고 기존 primary NetEntityId, HP, damage, HUD, reward owner를 유지한다.
- Client는 phase 3 snapshot에서 body/weapon/armor part group 전체를 `BOSS_VALTAN_GHOST` variant로
  stage한 뒤 한 번에 교체한다. resource/clone 실패 시 기존 normal group을 보존하며 Server combat state
  적용은 격리한다. phase가 3 미만으로 돌아오면 같은 transaction으로 normal group을 복구한다.
- `VALTAN_GHOST_PORTAL_ONCE`는 `combatobject.valtan.ghost.portal-charge` 네 개를 arena spawn center
  기준 반폭 22m의 네 꼭짓점에서 같은 tick에 만든다. `RADIAL_INWARD` missile은 5초 동안 중심을 지나
  반대 꼭짓점까지 이동하고 실제 `damage.valtan.portal-rush` contact damage를 사용한다. phase-3 auxiliary
  scheduler는 respawn 직후 한 번, 이후 150 Server tick(5초)마다 foreground 6-pattern loop와 독립 실행한다.
- Six Pizza의 player-target selection, fixed-tick yaw, 동일 mutable root, delayed Element lifetime과 source
  JSON은 유지했다. 최초 spawn과 후속 root update에 공통 `+180°`만 적용해 피자가 선택 플레이어의 반대
  방향을 가리키게 했다.
- direct-authored Product Effect Save는 disk CAS와
  `CEffectPresentationService::Reload_SelectedProductEffect`를 한 성공 경계로 연결했다. 선택 Effect의
  catalog document, GPU target, queue/budget/duration cache가 준비된 뒤에만 saved/runtime-equivalent 상태로
  commit한다. 준비 실패 시 이전 disk canonical을 exact CAS로 복구하고 편집 중 draft를 dirty로 남긴다.
  active occurrence는 기존 shared resource로 자연 종료하며 이후 새 spawn부터 저장본을 사용한다.
- 사용자가 `effect.valtan.sky-axe.active`에서 삭제한
  `...sprite_particle_8.1.1.2.1.1` 행은 복원하지 않았다. 물리 source는 현재 7 Element이며 이를 예전
  8개 구성으로 되돌리던 회귀도 삭제 보존 계약으로 교정했다. HIGH_JUMP의 Server schedule 자체는 계속
  `count=3 / interval=1333ms`이므로 combat-object spawn 횟수와 Effect 내부 Element 중복은 별개다.
- V1 decal `normalCutoff`는 normal map이 적용된 lighting normal 대신 depth reconstructed position의
  `ddx/ddy` 기하 normal을 사용한다. GBuffer normal은 hemisphere 정렬에만 사용해 평면 바닥 통과와 벽
  거부를 함께 유지했다. V2 shader와 authored cutoff 값은 변경하지 않았다.
- Boss Verification의 기본 버튼은 `Restart Saved Pattern (Fresh Arena)`이며 saved scriptedSequence가
  정확히 1 slot일 때 `Restart_SavedFlow(true)`를 사용한다. Pattern Flow의
  `Restart Flow (Fresh Arena)`와 함께 저장본 reload, exact Server-active revision admission, full arena
  reset, Pattern 01 시작을 공유한다. 과거 exact-occurrence restart API는 내부 고급 경계로만 보존한다.
- authoritative Valtan presentation reload는 world-entry 때의 stale exact receipt를 정상 Save의 영구
  차단 조건으로 사용하지 않는다. 현재 typed physical closure를 stage하고 `Validate_StillCurrent` 뒤
  aggregate commit하므로 정상 Save 뒤의 fresh restart가 이전 cache 때문에 무반응이 되지 않는다.

### 자동 검증

- `Publish-GameplayBalance.ps1 -Mode Validate`: PASS.
  - 6 player profiles / 230 skills / 109 damage profiles
  - 4 bosses / 5 boss combat objects / 62 boss patterns / 276 pattern stages
  - managed 39 / legacy 25 / projected artifacts 9
- `author_valtan_phase_two_mechanics.py --mode Validate`: PASS.
- `valtan_tuning_pipeline.py validate`: PASS, source revision
  `d7ccb698f96aaca7caafcfe03e0737bfd70c80cfbd2ed77125e4831b2b6b41f5`.
- Effect Save/saved rows, sky-axe deletion, V1 decal, Pizza/ghost part swap, phase-3 primary loop/portal,
  saved Pattern/Flow fresh restart focused Python: 143 tests PASS, 7 environment-dependent skip.
- V1 `Shader_VtxEffectDecal.hlsl` `fx_5_0` compile: PASS.
- Server Debug x64 `/t:ClCompile`: exit 0. `GameRoom.cpp`, `GameplayCatalog.cpp`,
  `CombatObjectRuntime.cpp`, `ValtanBrain.cpp`와 Server contract translation unit을 포함한다.
- Client Debug x64 `/t:ClCompile`: exit 0. `Valtan.cpp`, `BossTool.cpp`, `Effect_Tool.cpp`와 현재 Client
  translation unit 전체를 포함한다. 기존 C4819/C4828 encoding warning만 남았다.
- 관련 JSON 9개 parse와 전체 `git diff --check`: PASS.
- 실행 중이던 Client/Server process는 에이전트가 종료하지 않았다. 최종 확인 시 두 process는 이미
  실행 중이 아니었지만, 다른 세션 변경을 취합해 사용자가 한 번에 build하려는 경계에 따라 Product/Core
  link는 이번 slice에서 실행하지 않았다. 현재 Debug EXE는 이 후속 변경을 포함한다고 간주하지 않는다.

### 사용자 수동 검증 및 남은 입력

- 새 통합 EXE에서 sky-axe Effect를 열어 7 Element 상태로 Save한 뒤, 같은 프로세스에서 새
  HIGH_JUMP occurrence를 생성한다. 삭제한 red floor가 새 occurrence에 없고 이미 재생 중이던 occurrence만
  기존 모습으로 끝나는지 확인한다.
- `Client/Bin/Resources/Character/Valtan/Ghost/`는 현재 물리 pack에서 비어 있다. 필요한 Drive payload인
  `Character/Valtan/Ghost/MN_RPBF_02.wmodel`과
  `Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel`이 전달되기 전에는 phase 3이 일반 body를 보존하는
  것이 fail-safe 정상이다. asset이 준비된 뒤 primary HP/HUD 유지, exact 6-loop와 네 포탈 동시 돌진을
  사용자가 직접 판정해야 한다.
- Six Pizza는 선택 플레이어 반대쪽 `+180°`, V1 decal은 바닥 표시/벽 cutoff, 두 Fresh Arena Restart는
  벽·바닥·props·collision/Nav 복구와 저장 Pattern 01 시작을 각각 확인한다. 사용자 서면 관찰 전까지
  위 항목을 visual PASS로 기록하지 않는다.

## 2026-09-02 후속 — Cross Rock Wave 4축 연기 Sprite Particle

- `effect.valtan.sequence.cross`는 기존 Mesh Particle 4개를 보존하고 같은 4축에 Sprite Particle
  4개를 추가해 총 8개 Element를 소유한다. 각 smoke Element는 대응 rock Element와 position,
  rotation, scale, velocity `20m/s`, emitter life `0.5s`, fixed spacing `1.5m`, max `12`, particle life
  `2s`, world-space birth 값을 1:1로 공유한다.
- 연기 리소스는 같은 Valtan stone source group의 원본 `particlespriteemitter_10`을 근거로 한다.
  Base `fx_e_atypical_005_cl`, Noise `fx_c_noise_008`, Mask `fx_e_noise_002`, material provenance
  `fx_d_pa_turbulence_01_13_tr`, `alpha_two_sided_depth_read` 조합이다.
- v13 codec이 Sprite Particle의 positive transform-motion duration을 거부하므로 smoke만
  `transformMotionDurationSeconds=0`을 사용한다. Element velocity는 0.5초 local clock 동안 계속
  평가되고 `localSpace=false` fixed-spacing birth가 각 위치를 world에 고정하므로 rock lattice와
  동일한 생성 경로를 유지한다.
- `test_valtan_cross_rock_wave_effect`: 6/6 PASS. 4 Mesh + 4 Sprite, 양쪽의 4축 coverage,
  exact motion lattice pairing과 리소스/material 계약을 검증한다.
- `Validate-EffectSources.ps1`: PASS, direct sources 177 / unbound references 0.
- Effect source Python 회귀 42/42 PASS, Valtan All Effects 계약 37/37 PASS.
- Client/UI는 실행하지 않았다. 사용자는 Effect Tool에서 `effect.valtan.sequence.cross`를 다시
  `Load Saved`한 뒤 `All Particles` 또는 `Sprite Particles`로 돌 행렬과 연기 행렬의 정렬을 판정한다.
