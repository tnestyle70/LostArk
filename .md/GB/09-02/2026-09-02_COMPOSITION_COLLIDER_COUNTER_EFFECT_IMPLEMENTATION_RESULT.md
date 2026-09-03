# 2026-09-02 Composition Collider · Counter · Effect 구현 결과

기준 브랜치: `GB/valtan-pattern-bug-fix`
기준 HEAD: `fbd30c8e9f4302b00e143a48f6ca76ab5a8c774a`

이 RESULT는 다음 두 계획 중 이번 작업에서 실제로 반영한 세 범위만 기록한다.

- [Composition Workbench 콜라이더 박스 저작 구현 계획](2026-09-02_COMPOSITION_COLLIDER_BOX_AUTHORING_IMPLEMENTATION_PLAN.md)
- [발탄 패턴 · 콜라이더 저작 · Composition 병목 구현 계획](2026-09-02_VALTAN_PATTERN_COLLIDER_AND_COMPOSITION_BOTTLENECK_PLAN.md)

```text
반영 범위 1  Action Composition Workbench의 Server collider Add/Tune/Remove
반영 범위 2  단일·3연속 내려치기 counter topology와 animation occurrence
반영 범위 3  0.6초 project-tuned Effect의 V2 Group 및 각 counter window 3회 배치
```

나머지 31개 요구사항 전체를 이 RESULT에서 완료 처리하지 않는다. 이번 세 범위의 코드·데이터는
working tree에 반영됐고 Debug Product의 Engine, Shared, Server, Client compile과 V1/V2 WARP closure까지
통과했다. FullDiagnostic은 마지막 Valtan model-view 검사에서 Git 비관리 Resources 두 파일이 없어
`BLOCKED`다. 이는 이번 collider/counter/effect 기능 실패가 아니다. Client 화면과 Effect 발생
형태·타이밍은 사용자가 직접 판정해야 한다.

---

## 1. 현재 상태

| 축 | 현재 상태 | 완료 판정 |
|---|---|---|
| Collider 저작 코드 | source/Product 반영, focused 5/5 + 11/11 + 55/55 | 자동 계약 PASS, Workbench 수동 화면 대기 |
| Counter source/Product | 단일·3연속 topology와 projection 반영 | counter 통합 24/24, promotion 17/17, typed transaction 22/22 PASS |
| Effect V2 leaf/group/binding | 0.6초 group과 21개 ONCE binding 반영 | pulse 4/4 및 V1/V2 validator PASS, 수동 시각 확인 대기 |
| Debug Product EXE | Engine -> Shared -> Server -> Client 및 CSO closure 완료 | **PASS — V1 1352/1352, V2 1352/1352 WARP** |
| Debug FullDiagnostic | 마지막 model-view 16건 중 11건 통과, 5건 resource prerequisite 차단 | **BLOCKED — 기능 회귀로 판정하지 않음** |
| Release EXE | 미실행 | 검증 증거 없음 |
| Client 수동 화면 | 미실행 | 사용자 전용 |

Balance Tool 자체 화면과 별도 기능 smoke는 이번 사용자 결정에 따라 요구하거나 PASS로 기록하지
않는다. 다만 Action Composition Workbench가 내부의 joined draft/save 경계를 사용하므로 관련 C++
compile과 Workbench `Save -> exact canonical reload` 계약은 자동 검증 대상으로 확인했다.

---

## 2. Collider 저작 반영

### 2.1 한 Stage의 collider 쓰기 경로를 하나로 고정

`Action Composition Workbench -> Stage Hit (Boss -> Player)`의 typed Details가 collider geometry,
timing, response와 damage를 쓰는 유일한 UI다. 기존 Balance Tool의 Valtan pattern panel은 현재 값과
timing을 읽기 전용으로 표시하며 직접 필드를 수정하지 않는다.

저작 경로는 다음과 같다.

```text
Action Composition Workbench typed Details
  -> CBalanceTool::PATTERN_STAGE_EDIT
  -> CBalanceTool::Set_ValtanStageDraft
  -> BuildValtanDraftPatch / SET_STAGE_HIT + SET_DAMAGE_RATE
  -> valtan_tuning_pipeline.py의 최종 Add/Tune/Remove 재검증
  -> split source + generated Product transaction
  -> exact canonical reload
```

UI를 우회한 `SET_STAGE_HIT`도 Python writer가 같은 admission을 다시 검사한다. 따라서 화면 버튼만
비활성화하고 command writer가 더 넓은 권한을 갖는 경로를 남기지 않았다.

### 2.2 Add/Tune/Remove admission 분리

기존 `hitEditable` 하나를 다음 세 파생 admission으로 분리했다.

| 명령 | admission |
|---|---|
| Add | hit가 없는 non-WAIT `MANUAL_SERVER_AUDITION` Stage |
| Tune | 기존 hit를 가진 non-WAIT Stage. canonical hit도 제자리 튜닝 가능 |
| Remove | 기존 hit를 가진 non-WAIT manual audition Stage이며 CAPTURE가 아님 |

`hitEditable`은 기존 Animation Tool 소비자를 위한 tune-only 호환 alias로만 남는다. canonical no-hit
Stage의 Add, canonical collider의 Remove, CAPTURE collider의 geometry-only Remove는 C++ setter와
Python transaction writer 양쪽에서 거부된다. `DAMAGE -> CAPTURE` 전환은 같은 Pattern에 typed
`RELEASE_GRABBED_PLAYERS` 경로가 있고 attachment가 `BOSS_LEFT_HAND`일 때만 허용한다.

### 2.3 BOX 생성과 Details

Sequencer의 Collider lane `+`와 Details의 `Add Server Collider (BOX)`는 같은 초기화 함수를 사용한다.

```text
shape              BOX
lengthM            8.0
halfWidthM         2.5
schedule           1 pulse at 0ms
response           DAMAGE
anchor             absent / BOSS_CURRENT identity
damage profile     현재 선택된 admitted damage.valtan.* row
```

기존 hit에서는 다음을 편집할 수 있다.

- Collider Type: `Damage` 또는 admitted `Grab (Capture)`
- Shape / Size: `CIRCLE`, `RING`, `CONE`, `BOX`, `CROSS`, `SIX_DIRECTIONS`
- Timing: `Pulse Schedule` 또는 `Active Window (lifetime)`
- Damage: profile ID와 `damageRatePercent` 1~100000
- 진단: 같은 damage profile을 쓰는 전체 Valtan Stage 수와 방어 전 raw damage

damage rate는 두 번째 파일 writer를 만들지 않고 기존 canonical draft patch의 `SET_DAMAGE_RATE`로
`DamageProfiles.json` 및 provenance projection과 같은 transaction에 들어간다.

### 2.4 ACTIVE_WINDOW와 round-trip 보존

`PATTERN_STAGE_EDIT`가 기존 `hit.anchor`와 `hit.activation`을 읽고, 검증하고, 다시 패치에 쓴다.
따라서 geometry만 튜닝해도 이 두 블록이 사라지던 손실 경로를 닫았다.

```text
ACTIVE_WINDOW
  startMs >= 0
  lifetimeMs >= 1
  startMs + lifetimeMs <= Stage.durationMs
  pulse count/delay/interval/explicit offsets는 모두 0 또는 empty
  perTargetPolicy = ONCE
```

활성 구간은 반열린 구간 `[startMs, startMs + lifetimeMs)`다. `NONE`으로 제거할 때는 shape뿐 아니라
schedule, activation, anchor, damage, push/knockdown, response/attachment도 함께 identity 값으로
정규화한다.

### 2.5 timeline과 debug mirror

Workbench timeline의 collider block은 ACTIVE_WINDOW의 실제 시작과 끝을 폭으로 사용하며 이동·좌우
resize도 같은 Stage-local window를 수정한다. `CValtan`의 Debug mirror는 `hitCount == 0`인
ACTIVE_WINDOW도 등록하고, 해당 반열린 구간 동안 분홍 wire를 표시한다. 이 wire는 Server 판정의
표현 mirror일 뿐 damage 권위가 아니다.

### 2.6 Save 진단

Sequencer 상단은 `Pattern`, `Sound`, `EffectV2` owner의 clean/dirty/read-only/pinned 상태를 따로
표시한다. 실패 문장은 `[Pattern]`, `[Sound]`, `[EffectV2]`, `[Pipeline]` 중 최초 차단 owner로
시작한다. 저장 성공 조건은 여전히 모든 owner stage와 pipeline/reload가 끝나는 all-or-nothing
계약이며, 진단을 위해 원자성을 해체하지 않았다.

---

## 3. 단일 counter 내려치기 반영

`VALTAN_COUNTER`의 몸 animation과 Server branch를 다음으로 고정했다.

| Stage | Server clock | animation | 의미 |
|---|---:|---|---|
| `STEP_01` | 2000ms | `mesh_att_battle_14_01` exact 2000ms | 준비 |
| `STEP_02` | 1800ms | `mesh_att_battle_14_02` exact 1000ms + 같은 clip exact 800ms | counter window |
| `STEP_03` | 1667ms | `mesh_att_battle_14_03` exact 1667ms | counter 실패 공격 |

`STEP_02`에서 성공하면 cross-pattern branch가 `VALTAN_GROGGY_FOLLOWUP`으로 이동한다. 공용 groggy가
종료되면 일반 다음 pattern decision으로 복귀한다. TIMEOUT이면 `STEP_03` 공격을 재생하고 종료한다.
존재하지 않는 `mesh_att_battle_14_loop`를 가정하거나 one-shot clip을 `LOOP_TO_STAGE_END`로 돌리지
않는다.

기존 별도 Pattern `VALTAN_COUNTER_GROGGY`는 source/Product/rotation에서 제거하고
`retiredPatternIds`에 넣었다. 따라서 `카운터 쳐야 하는 내려치기 - 성공 그로기`라는 중복 Pattern을
별도 선택·재생하지 않는다.

---

## 4. 3연속 counter 내려치기 반영

`VALTAN_TRIPLE_COUNTER`는 groggy placeholder 일곱 Stage를 제거하고 단일 counter의
`14_01 / 14_02 / 14_03` 흐름을 세 번의 판정으로 확장했다.

```text
SETUP       14_01                    2000ms
COUNTER_1   14_02 1000 + 800         1800ms  counterable
FAIL_1      14_03                    1667ms  attack -> COUNTER_2
COUNTER_2   14_02 1000 + 800         1800ms  counterable
FAIL_2      14_03                    1667ms  attack -> COUNTER_3
COUNTER_3   14_02 1000 + 800         1800ms  counterable
FAIL_3      14_03                    1667ms  attack -> end
```

세 `COUNTER_n`만 paired `boss.flag.counterable` ENTER/EXIT와 `BOSS_FORWARD_ARC 180deg` proxy를
소유한다. 어느 회차든 `COUNTER_HIT`이면 `VALTAN_GROGGY_FOLLOWUP`으로 이동한다. 실패한 회차는
해당 `FAIL_n` 공격을 실행한 뒤 다음 counter 회차로 진행하며, 세 번째 실패 공격 뒤 종료한다.
FAIL Stage에는 counter flag/proxy를 남기지 않았다.

### 4.1 selection과 Workbench admission 보존

`VALTAN_TRIPLE_COUNTER`는 `decisionModel.manualAuditions`나 DERIVED row로 승격하지 않았다. 두 selection
set에서 각각 weight 4를 유지하며 전체 candidate row 수도 12로 HEAD와 같다. selection candidate와
manual audition의 교집합은 pipeline이 fail-close하므로, Details를 열기 위한 manual 승격은 허용되는
우회가 아니다.

이 Pattern은 managed non-manual `CorePatternIds` 경로로 Workbench와 Boss Play에 노출되고 모든 안정
Stage의 Gameplay/Logic/Collider Details가 열린다. 기존 collider Tune은 canonical Stage에도 허용된다.
반면 Stage topology와 collider Add/Remove는 계속 manual-only이므로, 이번 topology 재저작은 정본
source/pipeline transaction으로 반영했다.

---

## 5. 0.6초 V2 Effect Group과 정확히 3회 배치

사용자가 만든 `effect.valtan.project-tuned.sequence.trash`의 선택 particle을 다음 V2 Product 자원으로
올렸다.

```text
leaf   boss.valtan.project-tuned.sequence.trash.pulse
       lifetime 0.6s / particle lifetime 0.6s / loop false

group  boss.valtan.project-tuned.sequence.trash.pulse-group
       duration 600ms / leaf child 1개 / Deactivate at 600ms
```

일반 `EACH_LOOP` runtime을 이번 변경에서 새로 구현하지 않았다. 요청한 유한 3회를 exact하게 만들기
위해 각 occurrence에 `repeatPolicy: ONCE` binding을 세 줄씩 명시했다.

| Pattern / Stage | clip wall | 세 시작 시각 | binding 수 |
|---|---:|---|---:|
| `VALTAN_TRASH / STEP_07` | `13_03` 1000ms | 0 / 200 / 400ms | 3 |
| `VALTAN_TRASH / RETRY_WINDUP_02` | `13_03` 1000ms | 0 / 200 / 400ms | 3 |
| `VALTAN_TRASH / RETRY_WINDUP_03` | `13_03` 1000ms | 0 / 200 / 400ms | 3 |
| `VALTAN_COUNTER / STEP_02` | `14_02` 1000 + 800ms | global 0 / 600 / 1200ms | 3 |
| `VALTAN_TRIPLE_COUNTER / COUNTER_1` | `14_02` 1000 + 800ms | global 0 / 600 / 1200ms | 3 |
| `VALTAN_TRIPLE_COUNTER / COUNTER_2` | `14_02` 1000 + 800ms | global 0 / 600 / 1200ms | 3 |
| `VALTAN_TRIPLE_COUNTER / COUNTER_3` | `14_02` 1000 + 800ms | global 0 / 600 / 1200ms | 3 |

총 21개 binding이며 모두 `b_effectroot`, `FOLLOW_SLOT`, `TARGET_YAW`, `NATURAL`을 사용한다.
1000ms `13_03`에서는 600ms particle 세 개가 Stage 끝까지 모두 닫히도록 200ms 간격으로 겹쳐
배치했다. 1800ms `14_02` 두 occurrence에서는 첫 clip의 0/600ms와 두 번째 clip의 200ms가
Stage-global 0/600/1200ms를 이룬다.

기존 V1 source는 `ValtanPatternAuthoringEffects.json`의 `DRAFT_ATTACHED` 저작 입력으로 보존한다.
제품 catalog와 `VALTAN_TRASH / STEP_01` direct cue에서는 제거해 V2 Group 외의 추가 재생을 막았다.
더 이상 참조되지 않는 이전 triple-counter V1 carrier asset/catalog row도 제거했다.

---

## 6. 변경 파일

### Collider authoring/runtime mirror

- `Client/Public/BalanceTool.h`
- `Client/Private/BalanceTool.cpp`
- `Client/Private/ActionCompositionWorkbench.cpp`
- `Client/Public/Valtan.h`
- `Client/Private/Valtan.cpp`
- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`
- `Tools/ValtanPipeline/test_valtan_stage_hit_activation_authoring.py`
- 기존 Action Composition/Balance/transaction 회귀 파일

### Counter source와 projection

- `Data/Valtan/Valtan.gameplay.json`
- `Data/Valtan/Valtan.presentation.json`
- `Data/Encounters/Valtan/ValtanEncounter.json`
- `Data/Encounters/Valtan/ValtanPatternRotations.json`
- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`
- `Server/Private/ServerGameplayContractTests.cpp`
- `Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py`
- `Tools/ValtanPipeline/promote_valtan_animation_chains.py`
- counter/cross-pattern/finale focused 회귀 파일

### Effect V2 pulse

- `Data/Effects/V2/Authored/boss.valtan.project-tuned.sequence.trash.pulse.effectv2.json`
- `Data/Effects/V2/Groups/boss.valtan.project-tuned.sequence.trash.pulse-group.effectv2group.json`
- `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`
- `Data/Effects/Authored/effect.valtan.project-tuned.sequence.trash.effect.json`
- `Data/Effects/ValtanPatternAuthoringEffects.json`
- `Data/Effects/EffectCatalog.json`
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`
- `Tools/ValtanPipeline/test_valtan_project_tuned_counter_pulse_effect_contract.py`

새 C++ 파일은 없으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다.

---

## 7. 자동 검증 상태

| 검증 | 상태 |
|---|---|
| `test_action_composition_collision_authoring_contract.py` | **PASS — 5/5** |
| `test_valtan_stage_hit_activation_authoring.py` | **PASS — 11/11** |
| `test_action_composition_workbench_regression_oracles.py` | **PASS — 55/55** |
| `test_valtan_balance_tool_contract.py` | **PASS — 33/33**, code contract이며 Balance Tool UI smoke가 아님 |
| counter authoring + cross-pattern follow-up + triple split | **PASS — 24/24**, triple split 6/6 포함 |
| `test_valtan_animation_chain_promotion.py` | **PASS — 17/17** |
| `test_valtan_canonical_typed_patch_transaction.py` | **PASS — 22/22** |
| `test_valtan_project_tuned_counter_pulse_effect_contract.py` | **PASS — 4/4** |
| Action Composition sound owner / pattern sound focused | **PASS — 20/20 + 4/4** |
| `Project-ValtanPatternMaster.ps1 -Mode Validate` | **PASS — managedPatterns 41** |
| `Publish-GameplayBalance.ps1 -Mode Validate` | **PASS — boss pattern 64 / Stage 278 / Valtan timeline row 52** |
| Effect V1 validator | **PASS — directSourceCount 175 / unboundReferenceCount 1** |
| Effect V2 validator | **PASS — authored 117 / binding 132 / group 11 / independent 8 / texture 74** |
| 변경 JSON parse | **PASS — 6/6** |
| `git diff --check` | **PASS — exit 0** |
| Debug Product | **PASS — Engine/Shared/Server/Client, WARP pixel V1=1352 / V2=1352** |
| Debug FullDiagnostic | **BLOCKED — 아래 Git 비관리 Resources prerequisite 부재** |

Debug Product 정본 receipt는 `out/BuildPipeline/receipts/product.debug.receipt.json`이다. 완료 시각은
`2026-09-02T12:45:59.5763120Z`, source input SHA-256은
`c55433637666a7a11ef12cfccfd79d2ea77b25fb1fa999ffae4af8a14334d34e`다. 안정 실행 evidence는
`out/BuildPipeline/runs/20260902T123301551Z-debug-product-e649d545.json`에 남아 있다.

### 7.1 FullDiagnostic 차단 판정

FullDiagnostic은 코드 검사와 build, Effect/Workbench focused 구간, 69개 crash-window 검사 등 후반
구간까지 통과했다. 마지막 `test_valtan_model_view_composition.py` 16건에서 11건이 통과했고 다음
1 failure + 4 errors가 발생했다.

- `ghost_catalog_joins...`
- `ghost_donor_matches...`
- `ghost_donor_translation...`
- `ghost_material_dependencies...`
- `normal_and_ghost_death...`의 ghost subtest

다섯 건의 공통 원인은 팀장이 Drive로 관리하는 `Client/Bin/Resources` 아래 다음 두 파일이 현재
물리적으로 없다는 것이다.

```text
Character/Valtan/Ghost/MN_RPBF_02.wmodel
Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel
```

따라서 FullDiagnostic 전체 판정은 `BLOCKED`이며 collider/counter/effect 구현의 test failure로
승격하지 않는다. 두 Resource를 복구한 뒤 같은 source에서 FullDiagnostic을 재실행해야 전체 PASS를
기록할 수 있다.

---

## 8. 원본 31개 요구사항 감사

원문을 `질문 6 + 작업 25`로 셌다. X1 메타 검토는 이 31개 밖의 별도 항목이다. `완료`는 코드가
필요한 항목이면 구현·자동 계약까지 닫혔다는 뜻이고, `수동확인`은 source와 자동 검증이 닫혔지만
사용자 화면 판정이 남았다는 뜻이다.

| 판정 | 수 | 원본 ID | 현재 경계 |
|---|---:|---|---|
| 완료 | 8 | Q1~Q6, C1, H3 | 질문 답변, counter 분기 흐름, 잡기 계약 확정 |
| 부분 | 7 | T1, T2, C2, H1, H2, H4, S1 | 일부 writer/debug/counter/sound 계약만 반영, 범용 경로는 미완 |
| 미구현 | 8 | M1, M2, E1, E2, E3, P3, P6, S2 | 마력구 높이·하강, 사자후/파운딩 loop/침묵, 사망 추적, 워프, 땅구르기 사운드 |
| 수동확인 | 2 | T3, C3 | Workbench collider/damage 저장·재로드와 3연속 counter 실제 화면 판정 |
| 차단 | 6 | M3, P1, P2, P4, P5, S3 | runtime 재현·Resource/카탈로그·4-clip gate가 먼저 필요 |
| 합계 | **31** | 질문 6 + 작업 25 | X1 제외 |

X1은 3회 중 1회 비평만 실측 반영돼 별도 `부분`이다. 새 counter/effect 요청은 원본 31개와 별도로
이 RESULT의 3~5절처럼 source/Product와 자동 계약까지 반영됐고, Client 수동 gameplay/visual 판정만
남았다.

### 8.1 핵심 잔여 병목과 교정

- `VALTAN_TRIPLE_COUNTER`를 `manualAuditions`에 넣어야 Details가 열린다는 기존 S2 전제는 틀렸다.
  이 Pattern은 두 selection set의 candidate weight 4를 유지하며 managed non-manual Core 경로에서
  Workbench/Boss Play/Details가 열린다. candidate와 manual의 교집합은 pipeline이 거부하므로 추가하지
  않는 것이 정본이다.
- P5 부위 파괴는 Workbench의 3-clip HOLD/Replace gate가 4-clip 6983ms sequence를 거부하고, 필요한
  sound event의 catalog/schema 연결도 닫히지 않아 시작 전 차단 상태다.
- T2/H1의 live persistent Stage collider geometry, H2/H4의 `gripLocalOffset`과 bone attachment 전수,
  일부 player `HitShapes`는 아직 범용 디버그·부착 계약이 아니다.
- E2의 범용 `EACH_LOOP` runtime은 여전히 없다. 이번 버러지/counter는 0.6초 ONCE binding을 세 번
  명시해 요청한 exact 유한 재생만 닫았다.
- P1/P2/P4는 각각 yaw, 생존/사망별 entry failure, ghost swap의 runtime 분리 재현이 필요하다.
  T3/C3와 함께 사용자의 Workbench/gameplay/visual 확인 전에는 수동 PASS가 아니다.
- FullDiagnostic의 별도 자동화 차단은 7.1절의 Ghost Resource 두 파일이다. 원본 기능 병목과 섞어
  collider/counter/effect 회귀로 판정하지 않는다.

## 9. Kakul Composition — PLAN-READY BLOCKER

Kakul Composition은 이번 변경에서 구현하지 않았다. 현재 네 planner profile은 모두
`REFERENCE_ONLY`이며 합계 349 action, 4072 Stage, 3692 WModel slot의 저작 참고 자료다.

```text
MN_RPCT_05
MN_RPCT_06
MN_RPCT_07  -> 현재 planner preview body는 MN_RPCT_05 공유
MN_RPCZ_00
```

`LV_LUT_MIDNIGHTC_ED` world와 stage marker/camera shot 문서는 있지만 제품 boss placement,
BossProfiles/catalog, Server 권위 combat runner, Client boss presentation을 잇는 vertical slice는 없다.
Kakul authored action/pattern binding도 `REFERENCE_ONLY`이고, Workbench Direct Replace/Append와 저장
transaction은 Valtan-compatible sequence로 제한돼 있다. 따라서 world가 존재한다는 사실만으로 Kakul
boss가 제품 Composition owner라고 볼 수 없다.

구현에 착수할 수 있는 순서는 다음으로 고정한다.

| 순서 | 먼저 닫을 계약 |
|---:|---|
| 1 | `MN_RPCT_05`, `05`와 별도인 `06`, `05` body를 공유하는 planner `07`, `MN_RPCZ_00` 중 제품 boss의 정확한 archetype/body identity를 사용자·팀이 선택 |
| 2 | 선택 ID를 stable actor/boss catalog와 `BossProfiles`, Server-authoritative encounter/world placement, navigation/damage/collider 계약에 연결 |
| 3 | 같은 ID를 Client product presentation/prototype/model mapping에 연결하고 spawn 실패 rollback을 닫음 |
| 4 | Valtan 전용 composition runner/provider를 두 번째 runtime 없이 generic boss pattern runner/provider로 추출하고 Valtan과 선택 Kakul boss가 함께 소비 |
| 5 | 현재 없는 일반 next-stage writer와 typed Camera owner transaction을 구현하고 parse -> validate -> stage -> commit -> save/reload를 닫음 |
| 6 | 그 뒤에만 Kakul의 Pattern/Animation/Effect/Sound/Collider/Camera tabs와 Save를 제품 Workbench에 노출 |
| 7 | domain publisher/harness, Server+Client Product build, 실패 rollback, 사용자 gameplay/visual 확인으로 완료 판정 |

첫 번째 blocker는 정확한 archetype/body 선택이다. 특히 `MN_RPCT_07`의 planner preview가
`MN_RPCT_05` body를 공유한다는 이유로 두 ID 중 하나를 제품 정체성으로 임의 선택할 수 없다. 또한
현재 next-stage writer는 Valtan manual draft 전용이고 Kakul camera shot은 level document일 뿐
Workbench Camera lane의 typed writer/save transaction이 아니다. 이 섹션은 구현 완료 보고가 아니라
선택 즉시 별도 PLAN으로 옮길 수 있는 착수 순서다.

## 10. 남은 경계

- Client의 최종 visual fidelity와 실제 발생 타이밍은 사용자가 직접 확인해야 한다.
- Effect V2의 범용 `EACH_LOOP`/clip-epoch runtime 구현은 이번 exact 3회 요구에 포함하지 않았다.
- 한 Stage는 계속 `hit` 하나만 소유한다. 복수 geometry가 필요하면 Stage 분할 또는 별도 combat object
  수직 슬라이스가 필요하다.
- CAPTURE collider 제거는 grab branch, attachment와 release action을 함께 다루는 별도 typed transaction이
  필요하다.
- 저장 뒤 Server 적용은 기존 계약대로 Product publish/build와 Server 재시작·world 재진입이 필요하다.
- 상위 31개 요구사항 중 마력구, 사자후, 파운딩 일반 loop, 왼손 grip offset, 부위 파괴, 사운드,
  워프, 침묵 마스크 등은 이 RESULT의 완료 범위가 아니다.

## 11. 사용자 수동 확인 경로

```text
1. Visual Studio에서 Server + Client profile을 사용자가 Ctrl+F5
2. Lobby -> Valtan 진입
3. F1 -> Action Composition Workbench
4. manual audition의 hit 없는 non-WAIT Stage에서 Collider lane +
   -> BOX 8.0m / half-width 2.5m draft 확인
5. Shape / Timing / Damage 수정 -> Save
   -> Pattern / Sound / EffectV2 owner 상태와 exact canonical reload 확인
6. Boss Tool에서 VALTAN_COUNTER 재생
   -> counter 성공은 공용 groggy, 실패는 14_03 공격
7. VALTAN_TRIPLE_COUNTER 재생
   -> 14_02 counter window와 14_03 실패 공격이 최대 3회 이어지는지 확인
8. VALTAN_TRASH의 세 13_03 window와 단일/3연속 counter의 각 14_02 window에서
   0.6초 Effect가 정확히 세 번 발생하는지 확인
```

사용자의 서면 확인 전에는 위 항목을 `manual first pixel`, `eye smoke`, `visual PASS`로 기록하지 않는다.

## 12. 2026-09-03 작업 중단 시점 정본

사용자 요청에 따라 여기서 추가 구현과 빌드를 멈춘다. 이 절이 위 8~10절의 오래된 잔여 현황을
대체한다. 요청된 Composition/Valtan 기능의 source와 Product 데이터 투영, focused 계약 보강은 거의
완료됐지만 canonical Product build는 아직 PASS가 아니므로 전체 완료로 판정하지 않는다.

### 12.1 확인된 자동 검증

- Valtan Pattern Master: 70/70 PASS
- Valtan pipeline 전체 discovery: 762개 중 처음 758개 PASS, 나머지 4개 stale oracle 교정 뒤 해당
  focused suite 10/10 PASS
- Effect Tool V2: 95/95 PASS
- Composition 전체 focused 계약: 147/147 PASS
- Composition typed patch transaction: 22/22 PASS
- PatternTree 27/27, triple-counter migration 6/6, cross-pattern fixture 7/7 PASS
- Effect cue transaction 5/5, Trash retry 6/6, Effect V2 binding 19/19 PASS
- player hit-shape coverage 4/4, build profile contract 14/14 PASS
- Valtan source authoring Validate, Pattern Master Product Validate, Gameplay Balance Validate PASS
- World Destruction publisher Validate 및 ContractTest PASS

위 수치는 각 suite를 실제 실행한 결과다. 마지막 네 stale oracle을 고친 뒤 762개 전체 discovery를
다시 한 번 반복 실행하지는 않았고, Client 화면·소리·gameplay는 사용자 전용 수동 판정 경계다.

### 12.2 후속 수정 항목

| 우선순위 | 항목 | 현재 상태와 종료 조건 |
|---:|---|---|
| 완료 | `verticalOffsetM` 보조 publisher 스키마 동기화 | `Publish-ValtanWorldDestruction.ps1`에 exact optional field와 finite/range, non-zero, active `bossResponse`, no pattern/stage motion 규칙을 반영했다. 정상 0.5m 및 다섯 negative 경계를 ContractTest로 확인했다. |
| P0 | canonical Product build | 위 오류를 고친 뒤 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`를 처음부터 PASS해야 한다. 현재 마지막 실행은 이 publisher 오류에서 중단됐다. |
| P1 | FullDiagnostic | Product PASS 뒤 Debug FullDiagnostic을 직렬 실행해야 한다. writer-lock regression과 같은 checkout에서 병렬 실행하지 않는다. |
| P1 | 사용자 수동 확인 | 새 Client에서 collider 지속 표시, donut 크기, live Pattern highlight, 추적 도끼 점프/착지와 target spawn, rock 위치/시각, counter 3회, Effect 반복, Sound timing, cooldown R mask, Ghost body/triangle portal을 사용자가 판정한다. |
| P2 | Ghost runtime pack 인계 | `Character/Valtan/Ghost/MN_RPBF_02.wmodel`, `MN_RPBF_02_AnimSet.wmodel`과 참조 DDS closure는 Git/LFS가 아니라 팀장 Drive 물리 Resource pack으로 전달·확인한다. |
| P2 | player hit-shape 정밀화 | 76개 중 68개는 authored shape, 8개는 명시적 회색 `maximumRange` fallback이다. 디버그 가시성은 닫혔지만 정확한 shape가 필요하면 해당 8개를 별도 저작한다. |

Balance Tool 화면 검증은 사용자 지시대로 생략한다. Kakul boss 자체는 이번 범위가 아니며, 향후 boss
작업이 Composition의 동일한 explicit Stage/owner-scoped Save/parse-validate-stage-commit 계약을 재사용할
수 있게 만드는 범용 저작 도구 경계까지만 이번 변경의 의도다.

### 12.3 종료 상태

- 확인됐던 `verticalOffsetM` 보조 world-destruction publisher 차단은 Validate/ContractTest로 해제됐다.
- canonical Product 재빌드, FullDiagnostic과 사용자 수동 확인은 현재 PASS로 기록하지 않는다.
- 대규모 공유 dirty worktree라 자동 stage/commit/push는 하지 않았다.
- 이후 발견되는 누락은 이 절의 후속 수정 항목으로 추가하고, 이번 세션의 완료 수치에 소급해 섞지 않는다.

## 13. 2026-09-03 Client graph admission 회귀 교정

이 절은 12절 이후 사용자가 실제 Client에서 보고한 Lobby/Boss Tool/All Effects 회귀의 최신 판정이다.
정본 Valtan 데이터나 Effect catalog가 유실된 것이 아니라, 새 schema를 읽는 C++ 소비자들이 서로 다른
버전에 머물러 동일한 canonical graph의 admission이 실패했다. Boss Tool과 All Effects의 Valtan tree는
같은 admitted pattern graph를 사용하므로 앞단 strict join이 실패하면 둘 다 비어 보인다.

### 13.1 실제 실패 연쇄와 수정

| 순서 | 실제 원인 | 수정 |
|---:|---|---|
| 1 | split gameplay의 필수 nullable `motion:null`을 Client source reader가 property 존재만 보고 active motion으로 오판 | non-null motion object만 active로 판정하고 `VALTAN_STAGGER_SLOT/CHANNEL`의 Stage-owned `verticalOffsetM:0.5`와 boss response를 보존 |
| 2 | generated Product의 volley에 추가된 `firstSpawnOffsetMs`를 `CEncounterPatternReference` exact key set이 몰라 `VALTAN_HIGH_JUMP/AIRBORNE`를 거부 | required unsigned field, Stage-local 범위, 마지막 spawn 시각을 검증하고 Product fallback이 current v4를 수용 |
| 3 | `SUPPRESS_INTER_STEP_PURSUIT`가 source publisher와 Server에는 있었지만 Client split projection/Product fallback에는 없음 | ghost-death terminal `EXIT`의 exact owner/target/value/duration 계약을 양쪽 reader에 추가 |
| 4 | 유령 포털 Product fallback이 일반 RADIAL만 검사해 `VALTAN_GHOST_PORTAL_ONCE`의 삼각형을 exact하게 보호하지 못함 | 3점, 30도 시작, 120도 간격, radius 25.403411844343534m, 단일 volley/no supplement 규칙을 fail-close로 고정 |
| 5 | Animation Tool combat-object Effect lane이 `firstSpawnOffsetMs`를 버리고 0ms부터 표시 | Stage-local spawn offset에서 시작하고 `start + lifeMs`를 overflow-safe 계산한 뒤 Stage 끝으로 clamp |
| 6 | Product fallback의 단일 `v4 field is invalid` 메시지가 다음 실패를 가림 | motion/actions/branches를 분리해 `PATTERN/STAGE`와 실패 family가 바로 보이게 함 |

정본 JSON 변경은 없다. unknown key 무시나 fallback 완화로 우회하지 않고 source reader와 Product fallback이
동일한 truth table을 갖도록 고쳤다. 구체적인 재발 방지 원칙과 consumer closure matrix는
`.md/GB/gotchas.md`의 `Pattern/Effect schema evolution은 consumer closure matrix로 닫는다` 절이 정본이다.

### 13.2 재발 방지 게이트

- Core/FullDiagnostic에 Effect V2 catalog schema와 occurrence runtime 계약을 추가했다.
- Core/FullDiagnostic에 Valtan status/response 계약과 WorldDestruction `ContractTest`를 추가했다.
- Product profile은 빠른 제품 컴파일 역할을 유지한다. Pattern/Effect schema 변경 완료 증거는 fresh Core이며,
  Server 실행 의미가 바뀌면 FullDiagnostic까지 필요하다.
- 네이티브 하네스는 current split source와 freshly projected Product를 독립적으로 전부 읽고, 잘못된
  type/range/owner/trigger/conflict에서 last-good reference가 보존되는지 검사한다.

### 13.3 이번 수정의 자동 검증

| 검증 | 결과 |
|---|---|
| fresh Debug `ValtanPatternAuditionServiceHarness` build | **PASS** |
| current source canonical graph | **PASS — 64 patterns / 279 stages** |
| Product encounter reference 전체 하네스 | **PASS — 87 rejection/rollback + 후속 계약** |
| 관련 Valtan/Effect V2/Build Python 묶음 | **PASS — 105/105** |
| Animation Tool pattern master | **PASS — 14/14** |
| rock-pillar group 연계 | **PASS — 6/6** |
| WorldDestruction `ContractTest` / `Validate` | **PASS / PASS** |
| Valtan Pattern Master `Validate` | **PASS — managed 41 / projected artifacts 9** |
| Gameplay Balance `Validate -SkipValtanSplitProjection` | **PASS — 64 patterns / 279 stages** |
| `git diff --check` | **PASS** |

full Debug Product/Client build와 실제 Lobby 진입, Boss Tool Pattern 목록, All Effects Valtan tree 및
visual/audio/gameplay는 사용자의 다음 `Ctrl+F5` 판정이 남아 있다. 이 수동 확인 전에는 화면 PASS로
기록하지 않는다. 13절 검증 당시 PC는 `server-host`였으며 Visual Studio 시작 대상은
`Server + Client` profile이었다.

## 14. 2026-09-03 delayed 돌기둥 Server session 종료 회귀 수정

### 14.1 확정 원인

Client의 `Valtan replication observed a disconnected Server session.`과 Lobby의
`Server entry failed.`는 서로 다른 실패가 아니었다. 동일 socket과 시각의 Client/Server JSONL을
대조하면 다음 한 연쇄다.

1. Six Pizza 또는 Struggling의 delayed rock-pillar wave가 예정 tick에 실행된다.
2. authored boss-relative 네 root 중 하나가 exact navgrid 밖에 놓인다.
3. Server가 `world-update.pattern-scheduled-spawn-wave`를 room runtime failure로 latch한다.
4. 다음 정상 Client 입력이 not-ready room에서 거절되고 Server가 TCP FIN으로 session을 닫는다.
5. Client는 `CLIENT_PEER_CLOSED` 뒤 Valtan replication state를 정리하고 Lobby로 복귀한다.
6. Lobby가 recovery의 상세 원인과 별개로 공통 상태 문자열 `Server entry failed.`를 표시한다.

실패 session에서 send failure/reliable rejection/drop은 모두 0이고 queue HWM도 한도보다 충분히
낮았다. 따라서 패킷 크기, Client queue overflow, timeout이나 network congestion은 배제했다.
또한 같은 failure source가 Six Pizza와 Struggling 양쪽 실제 session에서 반복돼 한 패턴만의 문제가
아님을 확인했다.

### 14.2 반영한 수정과 안전 경계

`Server/Private/GameRoom.cpp`의 기존 visual four-rock off-navigation admission에 다음 exact owner
두 개를 추가했다.

- `combatobject.valtan.six-pizza.rock-pillar / VALTAN_SIX_PIZZA_106 / valtan.sequence.center-six-pizza-charge.step-01`
- `combatobject.valtan.struggling.rock-pillar / VALTAN_STRUGGLING / valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-04`

기존 `FIXED_AREA`, direction `NONE`, 빈 `Hits`, 하나 이상의 presentation pulse, 정확히 4개라는 의미
조건은 유지했다. 모든 visual object를 허용하거나 authored root를 navgrid 안으로 project/clamp하지
않는다. 두 정의 중 하나라도 gameplay hit를 가지면 다시 strict navigation admission을 적용한다.

`ServerGameplayContractTests.cpp`에는 두 owner의 실제 delayed scheduler 경로를 추가했다. ENTER 직후
무생성, due 직전 no-op, `1000ms=30 tick` 및 `833ms=25 tick`의 due에서 정확히 4개 atomic spawn,
다음 tick 중복 없음, authored 좌표와 off-nav root, spawn wire serialization을 확인한다. 각 정의에
damage hit를 주입한 negative에서는 live object와 pending spawn/presentation/despawn이 모두 0이어야
한다. Python rock-pillar 계약도 두 exact tuple과 좁은 의미 guard를 함께 고정한다.

### 14.3 브랜치와 검증 상태

- 작업 시작 시 현재 `HEAD`와 `origin/main`은 모두 `063e1a1a`였다. 모든 ref/reflog와 unreachable
  commit을 조사했지만 이 두 owner 수정이 들어간 main 미동기화 commit은 없었다.
- 기억한 전날 수정 `0e2e84b0`은 이미 main에 merge됐지만 Ground Roar/Part Break 두 owner만 허용했고,
  같은 변경에서 추가된 Six Pizza/Struggling owner가 누락된 불완전한 수정이었다.
- 사용자 요청에 따라 이번 source 반영 뒤 build, publisher와 실행형 harness는 실행하지 않았다.
- `git diff --check`만 **PASS — exit 0**으로 확인했다. 자동 검증과 Client 수동 gameplay/visual 판정은
  아직 PASS로 기록하지 않는다.
