# 2026-09-03 큰 도넛 추가 · Effect 연결 · Save 파이프라인 실행 RESULT

브랜치 `GB/valtan-bugfix-koukusaydon-pattern`, HEAD `de3cc494`.
이 문서는 실제 실행 결과만 소유한다. 계획은
`2026-09-03_VALTAN_GRIP_AND_PATTERN_REGRESSION_IMPLEMENTATION_PLAN.md`와
`2026-09-03_VALTAN_COMPOSITION_EFFECT_BINDING_IMPLEMENTATION_PLAN.md`가 소유한다.

작업 시작 시 worktree에는 codex 세션의 미커밋 변경(왼손 grip, 도끼 volley, edge별
`transitionPursuitMs`, 돌기둥 두 세트, Save abort 수정, 도넛 16-24 직접 변경)이 이미 있었다.
아래 "codex 변경이 남긴 미해결" 항목은 이번 세션이 만든 것이 아니다.

## 1. 요청과 반영 결과

| # | 요청 | 결과 |
|---|---|---|
| 1 | 3연속 공격에 내려치는 타이밍마다 이펙트 3개 | `boss.valtan.twohand` V2 group binding 3행 추가 |
| 2 | 3연속 공격 - 카운터에도 3번 재생 | 같은 group binding 3행 추가 |
| 3 | 사자후는 뒤 포효(`boss.valtan.shout.burst`)만 | 피자 패턴은 codex 세션이 이미 처리. 현재 상태 확인만 함 |
| 4 | 큰 도넛 Effect를 따로 만들고 패턴 추가 후 저장 | 신규 combat object · 패턴 · Effect 문서 · catalog · BossCatalog 등록 후 publish 완료 |
| 5 | 계획서 순서대로 실행 | 1~3단계와 Valtan 계약 테스트 실행 완료. 4단계 FullDiagnostic 미실행 |

## 2. 3연속 공격 / 카운터 Effect 연결

`Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`에 6행을 추가했다.
resource는 발악 패턴 `STEP_07`이 쓰는 것과 같은 `boss.valtan.twohand` group이다
(자식 9개: 바닥 decal 4장, `sw_1` x1.7, `fluid_1` 3개 ±40도 부채꼴, `blur_1`).

| bindingId | pattern / stage | clock | 근거 |
|---|---|---|---|
| `binding.valtan.project-tuned.three.001.smash-01` | `VALTAN_THREE / STEP_01` | `STAGE : 1617` | 원본 `mesh_att_battle_2_01` 임팩트 EFFECT 1617ms |
| `...three.002.smash-02` | `VALTAN_THREE / STEP_02` | `STAGE : 963` | 원본 `mesh_att_battle_2_02` 임팩트 963ms |
| `...three.003.smash-03` | `VALTAN_THREE / STEP_03` | `STAGE : 1300` | 원본 `mesh_att_battle_2_03` HIT 1300ms |
| `...triple-counter.001.smash-01` | `VALTAN_TRIPLE_COUNTER / FAIL_1` | `STAGE : 900` | Server hit `EXPLICIT_OFFSETS [900]` |
| `...triple-counter.002.smash-02` | `VALTAN_TRIPLE_COUNTER / FAIL_2` | `STAGE : 900` | 동일 |
| `...triple-counter.003.smash-03` | `VALTAN_TRIPLE_COUNTER / FAIL_3` | `STAGE : 900` | 동일 |

anchor는 Workbench의 `Append V2 Stage Binding`이 만드는 값과 동일하게 맞췄다
(`b_effectroot`, `SNAPSHOT_AT_START`, `TARGET_YAW`, identity transform, `ONCE`).
따라서 이후 Tool에서 드래그·복제·삭제로 계속 편집할 수 있다.

`STAGE` clock을 쓴 근거는 대상 stage가 전부 occurrence 1개이고
`sourceStartMs = 0`, `playRate = 1.0`이라 `CLIP_OCCURRENCE`와 벽시계 시각이 같기 때문이다.

## 3. 큰 도넛

### 3.1 문제의 출발점

codex 세션은 `combatobject.valtan.fist-in-out.donut`의 반경을 8-16에서 16-24로
직접 바꿨다. 이 combat object는 `VALTAN_FIST_IN_OUT` 하나가 소유하고
scriptedSequence의 두 occurrence가 같은 패턴을 참조하므로,
첫 도넛과 두 번째 도넛이 모두 16-24가 됐다.

### 3.2 이번에 만든 수직 슬라이스

| 파일 | 변경 |
|---|---|
| `Data/Valtan/Valtan.combatobjects.json` | `...donut`을 8-16으로 복구. `...donut-large` 16-24 신규 |
| `Data/Valtan/Valtan.gameplay.json` | `VALTAN_FIST_IN_OUT_LARGE` 패턴 신규. `scriptedSequence.patternIds[11]`을 이 패턴으로 교체. `manualAuditions`에 `DERIVED_SERVER_PATTERN` 등록 |
| `Data/Valtan/Valtan.presentation.json` | 같은 패턴의 presentation 엔트리와 `valtan.independent-effect.donut-large` 추가 |
| `Data/Effects/Authored/effect.valtan.project-tuned.large-donut.effect.json` | 신규. 기존 도넛 문서를 복제해 재조정 |
| `Data/Effects/EffectCatalog.json` | 위 asset 등록 |
| `Data/Actors/BossCatalog.json` | `combatObjectVisuals`에 `donut-large` -> 새 Effect, `worldScale [1.5,1.5,1.5]` |

stable ID는 다음과 같다.

```text
patternId            VALTAN_FIST_IN_OUT_LARGE      displayName "큰 도넛"
actionId             valtan.attack.fist-in-out-large
entryActionId        valtan.attack.fist-in-out-large.inner
spawnEventId         event.valtan.fist-in-out-large.spawn-donut
combatObject         combatobject.valtan.fist-in-out.donut-large
hitId                hit.valtan.fist-in-out.donut-large.01
effectAssetId        effect.valtan.project-tuned.large-donut
clientVisualId       combatobject.visual.valtan.fist-in-out.donut-large.v1
independentEffectId  valtan.independent-effect.donut-large
```

이제 `patternIds[10]`이 8-16 도넛, `[11]`이 16-24 큰 도넛이고
그 사이 `transitionPursuitMs[10] = 100`은 그대로다.

### 3.3 Effect 문서 재조정과 남은 한계

원본 도넛 문서는 gameplay 8-16, `worldScale 1.5` 기준으로 저작됐다.
큰 도넛은 16-24이므로 바깥 경계 비율 `24/16 = 1.5`로 아트워크를 키우고,
충격 링 particle은 밴드 중심을 정확히 옮겼다.

| element | 변경 |
|---|---|
| `donut.telegraph.outer.red` | scale 22 -> 33 |
| `sprite_particle_6` | scale 18 -> 27 |
| `donut.telegraph.inner.grow` | scale 17 -> 25.5 |
| `donut.impact.wave.black` x2 | `spawnShape.radius/innerRadius` 8 -> 13.3333333 (world 12m -> 20m) |

충격 링 위치는 정확히 유도된 값이다. `20 / 1.5 = 13.3333`이 새 밴드 중심이다.

**정확히 일치시키지 못한 부분을 명시한다.** 원본 텔레그래프 아트워크는
구멍:바깥 비율이 `8:16 = 1:2`로 고정돼 있고 큰 도넛은 `16:24 = 1:1.5`다.
비율이 다르므로 uniform scale로는 두 경계를 동시에 맞출 수 없다.
현재 문서는 **바깥 경계 24m를 기준**으로 맞췄고, 그 결과 구멍이 약 12m로 읽혀
Server 판정 안쪽 경계 16m보다 관대하게 보인다.
정확히 맞추려면 Effect Tool에서 `donut.telegraph.inner.grow` 하나만 별도로 키워야 하며,
이는 사용자 육안 판정이 필요한 작업이라 임의로 정하지 않았다.

## 4. Save 파이프라인 실행 결과

실행한 명령과 결과다.

| # | 명령 | 결과 |
|---|---|---|
| 1 | `Project-ValtanPatternMaster.ps1 -Mode PublishV2` | 성공. `changed=5 artifacts=7` -> 이후 `changed=1` |
| 2 | `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate` | `ok: true`, managedPatterns 42, combatObjects 9 |
| 3 | `Publish-GameplayBalance.ps1 -Mode Publish` | 성공. 6 player profiles, 230 skill rows, 109 damage profiles, 9 boss combat objects, 65 boss patterns, 280 pattern stages |
| 4 | `Tools/EffectToolV2/validate_effect_v2.py` | 성공. 119 authored, 139 bindings, 12 groups, 9 independent, 79 textures |
| 5 | `Tools/EffectPipeline/Validate-EffectSources.ps1` | 성공. directSourceCount 176 |
| 6 | `git diff --check` | 공백 오류 없음 |

**미실행:** `Invoke-BuildAndRegression.ps1 -Profile FullDiagnostic`(C++ 빌드),
Server/Client 실행, 화면 판정. 따라서 runtime/visual PASS로 기록하지 않는다.

## 5. Save 과정에서 실제로 발생한 문제

`gotchas.md`에 적힌 많은 항목 중 **이번에 실제로 터진 것만** 적는다.
나머지는 이번 실행에서 발생하지 않았다.

### 5.1 split authoring Product drift — 실제 발생, Tool 잠금의 진짜 원인

```
"message": "split authoring Product drift; run PublishV2: Data/Encounters/Valtan/ValtanEncounter.json"
```

codex 세션이 `Valtan.gameplay.json` 저작 source를 고친 뒤 PublishV2를 돌리지 않아
생성물이 어긋나 있었다. 이 상태에서는 canonical admission이 실패하고
`bMutationAdmitted = false`가 되어 Composition Animation Tool의
Append 버튼 두 개, Save, tree가 전부 잠긴다.
사용자가 겪은 "save도 못 하고 tree가 다 깨진다"의 원인이 이것이다.

조치: PublishV2 실행. 이후 `ok: true`.

### 5.2 provenance receipt에 신규 행을 만들어 주는 도구가 없음 — 실제 발생

```
Balance provenance coverage count mismatch. receipt=3405 expected=3418
```

combat object를 하나 추가하면 receipt에 13행(그 object의 13개 property)이 필요하다.
그런데 `project_provenance_receipt`는 **기존 행의 값만 갱신**하고 새 행을 만들지 않는다.
어떤 도구도 신규 행을 생성하지 않으므로 손으로 13행을 써 넣고
`coverage.fieldEntryCount` 3405 -> 3418, `bossCombatObjectCount` 8 -> 9를 맞췄다.

### 5.3 PowerShell 5.1 int/float 직렬화 불일치 — 실제 발생, 실제 버그

```
Balance provenance is missing or stale:
  Data/Encounters/Valtan/ValtanEncounter.json#pattern:VALTAN_HIGH_JUMP.patterns[8].stages
```

receipt에는 `arenaHeightToleranceM: 1`, `arenaRandomRadiusM: 14`(정수)가,
생성물에는 `1.0`, `14.0`(실수)이 들어 있었다.

* Python은 `1 == 1.0`이 True라 `project_provenance_receipt`가 갱신을 건너뛰었다.
* `Publish-GameplayBalance.ps1`은 `ConvertTo-Json` 문자열을 비교하므로 `1`과 `1.0`을 다르게 봤다.

즉 **두 검증기가 서로 다른 동등성 기준을 써서 영원히 수렴하지 않는 상태**였다.
저장소 전체에서 이 드리프트는 정확히 1건이었다.

조치: `Tools/ValtanPipeline/valtan_tuning_pipeline.py`에 `_receipt_value_identity()`를
추가해 `json.dumps`로 타입까지 구분해 비교하도록 고쳤다. 재실행 후 자동 수렴했다.

### 5.4 하드코딩된 인벤토리 세 곳 — 실제 발생

independent effect를 하나 추가했을 뿐인데 세 곳을 손으로 고쳐야 했다.

| 위치 | 종류 |
|---|---|
| `valtan_tuning_pipeline.py:91` `REQUIRED_LIVE_INDEPENDENT_EFFECT_IDS` | frozenset 상수 |
| `test_valtan_combat_object_hit_effect_presentation_contract.py:247` | 순서 있는 리스트 |
| `test_valtan_pattern_tree_contract.py:780` `expected_independent_ids` | set 리터럴 |

세 곳 모두 데이터에서 유도하지 않고 값을 복제해 둔 목록이다.

### 5.5 `compatibilitySelectionWeight` / `manualAuditions` 계약

신규 패턴은 `compatibilitySelectionWeight`가 0이면 `mechanics` 또는 `manualAuditions`에
있어야 한다. 처음에 `MANUAL_SERVER_AUDITION`으로 넣었더니
`manualAuditions must exact-join promoted animation lineage`로 거부됐다.
`MANUAL_SERVER_AUDITION`은 `Valtan.animation-chain-promotions.json`의 승격 계보와
정확히 조인돼야 하고, 승격 계보가 없는 파생 패턴은 `DERIVED_SERVER_PATTERN`을 쓴다.
`derived.fist-in-out-large` / `DERIVED_SERVER_PATTERN`으로 바꿔 통과했다.

### 5.6 발생하지 않은 것

`gotchas.md`와 계획서 11절에 적힌 다음 항목은 이번 실행에서 **발생하지 않았다.**

```text
Ring Save 의 abort() / use-after-free      (Tool 미실행)
LNK1104 / LNK1168 출력물 잠금               (빌드 미실행)
Effect V2 header/row, unknown group/leaf   (validator 통과)
Gameplay.bootstrap version / row count     (publish 성공)
disconnected server session / socket 10054 (Server 미실행)
pattern flow does not match Server-active  (Server 미실행)
```

## 6. Valtan 계약 테스트 현황

repo root에서 `PYTHONPATH=.`로 실행해야 한다.
`Tools/ValtanPipeline`에서 직접 실행하면 4개가 `ModuleNotFoundError: No module named 'Tools'`로
가짜 실패한다. 이는 코드 문제가 아니라 실행 위치 문제다.

### 6.1 이번 세션이 고친 것

| 테스트 | 원인 | 조치 |
|---|---|---|
| `test_valtan_combat_object_hit_effect_presentation_contract` | independent effect 하드코딩 리스트 | `donut-large` 추가 |
| `test_valtan_pattern_tree_contract` (1건) | 같은 하드코딩 set | `donut-large` 추가 |
| `test_valtan_pattern_master_v2` (4건) | `EXPECTED_SCRIPTED_SEQUENCE` 상수가 51개 구버전 | 현재 canonical sequence로 재생성 |

`test_valtan_pattern_master_v2`는 7 failure -> 3 failure로 줄었다.

### 6.2 codex 변경이 남긴 미해결 (이번 세션이 만들지 않음)

codex 세션이 `scriptedSequence`에 `transitionPursuitMs`를 추가하고
도끼 volley를 복구했지만 대응 테스트 fixture를 갱신하지 않았다.

| 테스트 | 실패 내용 |
|---|---|
| `test_valtan_camera_tool_contract` | `set(scripted_sequence) == {sequenceId, mode, interStepPursuitMs, patternIds}` — 5번째 키 `transitionPursuitMs`를 모른다 |
| `test_valtan_pattern_tree_contract` (1건) | fixture가 `transitionPursuitMs` 없이 sequence를 만들어 PipelineError |
| `test_valtan_pattern_master_v2` (errors 3건) | 같은 원인 |
| `test_valtan_pattern_master_v2` (failure 1건) | 도끼 volley 기대값이 `1/0/4` 구버전 |
| `test_valtan_canonical_typed_patch_transaction` (1건) | 미분류 |

`HEAD`의 `patternIds`는 51개이고 `FIST_IN_OUT`이 index 10 하나뿐이었다.
두 번째 도넛과 `transitionPursuitMs`는 전부 codex 세션의 추가다.

### 6.3 환경 문제

`test_valtan_pattern_master_v2`의 error 2~3건은
`UnicodeDecodeError: 'utf-8' codec can't decode byte 0xb0`다.
PowerShell 하위 프로세스의 **CP949 한국어 오류 출력**을 UTF-8로 디코딩해서 나는 것이며
Valtan 데이터와 무관한 테스트 하네스의 로케일 버그다.

## 7. 카운터가 적용되지 않는 원인

사용자 보고: 버러지 패턴과 3연속 카운터는 이펙트가 뜨고 logic에 COUNTER가 떠도
공격이 카운터로 인정되지 않는다. 3회 땅 치기 후 돌진은 정상.

### 7.1 Server의 유일한 카운터 트리거

`Server/Private/ValtanBrain.cpp:2399`

```cpp
if (CPlayerSkillSystem::Try_Counter(player, catalog, serverTick))
{
    (void)CBossCombatRuntime::Try_TriggerCounter(boss, serverTick);
    continue;
}
```

이 코드는 `Apply_PatternHit` **안**에 있다. 즉 카운터는
**보스의 pattern hit이 실제로 플레이어에게 판정되는 순간에만** 평가된다.

`CPlayerSkillSystem::Try_Counter`의 전제는 다음과 같다.

```cpp
if (PLAYER_ACTION_STATE::SKILL != player.eAction || 1u != player.iComboStage)
    return false;
if (PLAYER_SKILL_KIND::COUNTER != skill->eSkillKind || 2u != skill->ComboStages.size())
    return false;
```

### 7.2 실측 대조

| pattern | 카운터 stage | `hit.shape` | `counterProxy` | 결과 |
|---|---|---|---|---|
| `VALTAN_DASH_CHARGE` | `CHARGE` | **BOX 10 x 5m + schedule** | 없음 | **동작** |
| `VALTAN_TRASH` | `STEP_07` | **NONE** | 있음 | 불가 |
| `VALTAN_TRASH_CATCH_IF` | `STEP_07`, `RETRY_WINDUP_02/03` | **NONE** | 있음 | 불가 |
| `VALTAN_TRIPLE_COUNTER` | `COUNTER_1/2/3` | **NONE** | 있음 | 불가 |
| `VALTAN_COUNTER` | `STEP_02` | **NONE** | 없음 | 불가 |

`hit.shape`가 `NONE`이면 `Apply_PatternHit`의 대상 루프가 아예 돌지 않는다.
그러면 `Try_Counter`가 한 번도 호출되지 않으므로,
`COUNTERABLE` flag가 켜지고 이펙트가 뜨고 logic에 COUNTER가 표시돼도
플레이어의 카운터 스킬이 인정될 경로가 없다.

돌진만 되는 이유는 `CHARGE` stage가 실제 BOX 판정을 가지고 있어
그 판정이 플레이어에게 닿는 순간 `Try_Counter`가 호출되기 때문이다.

### 7.3 `counterProxy`는 죽은 데이터다

`stage.bHasCounterProxy`, `fCounterProxyForwardOffsetM`, `fCounterProxyRightOffsetM`,
`fCounterProxyRadiusM`, `fCounterProxyArcDegrees`는
`GameplayCatalog.cpp:2903`에서 파싱돼 `ValtanBrain.cpp:1694`에서 boss 상태로 복사되고
Client debug wire로 복제된다. 그러나 **Server의 어떤 hit test도 이 값을 읽지 않는다.**

그래서 `VALTAN_TRIPLE_COUNTER`의 `radiusM: 0.0`도 아무 검증에 걸리지 않았다.
아무도 읽지 않는 값이라 0이어도 문제가 드러나지 않는다.

저작 형태도 두 갈래로 갈라져 있다.

```text
VALTAN_TRIPLE_COUNTER  {"kind": "BOSS_FORWARD_ARC", "forwardOffsetM": 0.0,
                        "rightOffsetM": 0.0, "radiusM": 0.0, "arcDegrees": 180.0}
VALTAN_TRASH           {"space": "BOSS_LOCAL", "forwardOffsetM": 1.0,
                        "rightOffsetM": 0.0, "radiusM": 2.25}
```

`kind`가 있는 형태와 `space`가 있는 형태가 섞여 있는데,
소비자가 없으므로 둘 다 통과한다.

### 7.4 고치는 방법 두 가지

이번 세션에서는 구현하지 않았다. 요청 범위 밖이고 Server 판정 계약 변경이다.

```text
A안  카운터 stage 에 실제 hit.shape 를 저작한다
     COUNTER_1/2/3 와 TRASH STEP_07 에 전방 판정과 schedule 을 주면
     기존 Apply_PatternHit -> Try_Counter 경로가 그대로 동작한다
     Server 코드 변경 없음. 다만 그 stage 가 실제로 데미지를 주게 되므로
     damage profile 을 0 으로 둘지 별도 결정이 필요하다

B안  counterProxy 를 실제 소비자에 연결한다
     bPatternHasCounterProxy 가 켜진 stage 에서 hit.shape 와 무관하게
     proxy 원/부채꼴에 대한 판정을 돌려 Try_Counter 를 호출한다
     radiusM 0.0 을 publisher 가 거부하도록 검증을 함께 추가해야 한다
     저작 형태 두 갈래도 하나로 통일해야 한다
```

원작 거동에 가까운 것은 B안이다. 카운터 stage는 데미지를 주지 않으면서
전방 카운터 판정만 열려 있어야 하기 때문이다.

## 8. 남은 작업

```text
FullDiagnostic 빌드와 Server/Client 실행 (사용자)
큰 도넛 텔레그래프 구멍 12m -> 16m 재조정 (Effect Tool, 육안 판정)
카운터 A/B 선택과 구현
codex 세션이 남긴 테스트 fixture 5건 갱신
test_valtan_pattern_master_v2 의 CP949 디코딩 버그
워프 leg 재정합 (G05, 미착수)
```
