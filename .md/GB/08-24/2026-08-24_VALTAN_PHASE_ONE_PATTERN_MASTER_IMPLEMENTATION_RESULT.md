# 2026-08-24 발탄 1페이즈 Pattern Master 구현 결과

## 1. 결론과 현재 상태

`Data/Valtan/Valtan.pattern.json`을 발탄 1페이즈의 공동 authoring 정본으로 추가했다. 이 문서는
Server pattern stage, ordered body-animation occurrence, Product Effect cue와 독립 Effect 사용 위치를
같은 `patternId -> stageId -> actionId` 축으로 연결한다. 제품 Server와 발탄 Arena는 master JSON을
직접 읽는 두 번째 runtime을 만들지 않고, projector가 갱신한 기존 typed 제품 문서를 계속 소비한다.

이번 admission은 정확히 `7 patterns + 2 independent effects`다. 일반 패턴 다섯 개는 160~109줄의
`WEIGHTED_POOL`에서 실제 `selectionWeight`와 조건을 소비하고, 130/109줄 mechanic queue는 이 랜덤
선택보다 먼저 실행된다. 일곱 managed pattern 자체에는 counterable window가 없으므로 임의의 counter
clip을 만들지 않았다. 대신 기존 Product에 이미 존재하는 네 counterable stage와 정확한 animation
action/clip을 `REFERENCE_ONLY_LEGACY` reaction layer로 연결했다.

| 구분 | 결과 |
|---|---|
| Pattern master와 projector | 구현 완료 |
| 7개 pattern typed projection | 구현 완료 |
| Phase-1 normal weighted selection | 구현 완료 |
| Counter reaction/animation reference layer | 구현 완료, 4개 기존 Product stage |
| 도끼/도넛 independent Effect library | 구현 완료 |
| Animation Tool master timeline | 구현 완료 |
| Effect Tool authoring timeline/independent tree | 구현 완료 |
| focused master/projector/typed Tool 검증 | PASS |
| 사용자 visual fidelity | `PENDING_USER_VISUAL_GATE` |
| 전체 Debug regression | Engine/Shared/Server/Client build와 발탄·balance·world·navigation 검증 PASS, aggregate는 기존 Effect project 등록 stale gate에서 종료 |
| 전체 Release regression | 이번 긴급 merge 전 미실행, focused 계약과 Debug 전체 build를 admission 근거로 사용 |
| EffectPipeline 최종 전체 상태 | 발탄 focused suites PASS, 전체 aggregate 결과는 아래 자동 검증 절 참고 |

## 2. 정본과 제품 투영 계약

공동 정본과 실제 소비 경로는 다음과 같다.

```text
Data/Valtan/Valtan.pattern.json
        |
        +-- Project-ValtanPatternMaster.ps1
        |      parse -> validate -> stage -> atomic commit
        |
        +-- Server typed products
        |      Data/Encounters/Valtan/ValtanEncounter.json
        |      Data/Encounters/Valtan/ValtanPatternRotations.json
        |      Data/Encounters/Valtan/ValtanCombatObjects.json
        |
        +-- Client presentation products
        |      Data/Animation/Authored/Valtan/Valtan.patternbindings.json
        |      Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json
        |      Data/Actors/BossCatalog.json combatObjectVisuals
        |
        +-- stable reference-only products
               Data/Balance/DamageProfiles.json
               Data/Encounters/Valtan/ValtanCinematicCamera.json
               Data/Encounters/Valtan/ValtanWorldEvents.json
               Data/Animation/Reference/Valtan/Valtan.clipseq
```

projector는 managed row만 교체하고 unmanaged legacy row는 보존한다. master 또는 참조 제품에서 malformed
ID, 중복 owner, 누락 action/clip/effect, stage wall drift, branch drift, 잘못된 independent owner가 발견되면
publish 전에 실패한다. projection receipt의 runtime authority도 기존 제품 문서로 고정되어 있어 master와
제품 JSON을 동시에 읽는 별도 boss runtime은 없다.

Client의 `CValtanPatternTree`는 master만 신뢰해 화면을 만드는 것이 아니라 Encounter, animation binding,
Effect cue, combat object, rotation product를 exact join한다. gameplay field나 branch, animation occurrence,
counter reaction action 중 하나라도 어긋나면 기존 view를 유지하고 새 authoring view admission을 거부한다.

## 3. Admission된 일곱 패턴과 시간

| Pattern | Stage 시간 | endPolicy / repeat | 구현된 핵심 계약 |
|---|---|---|---|
| `VALTAN_WHIRLWIND` | WINDUP 1333 / SPIN 1200 / RECOVERY 1467ms | LOOP / LOOP / LOOP, 모두 x1 | 전체 4000ms다. 원작 단발 휠윈드 선택을 기준으로 SPIN을 1.2초로 고정했고 0/350/700/1050ms 네 번의 Server 판정을 유지한다. |
| `VALTAN_DASH_CHARGE` | WINDUP 3650 / CHARGE 500 / GROGGY 5000 / RECOVERY 900 / PART_BREAK 1400ms | EXACT x3 / EXACT / LOOP / EXACT / LOOP | WINDUP은 `repeatCount: 3`과 현행 세 source cut 600+600+2450ms를 함께 명시한다. wall contact, timeout, part destroyed를 branch graph로 선택한다. |
| `VALTAN_FOUR_SLASH` | WINDUP 400 / SLASHES 3500 / SPIN 3167 / RECOVERY 800ms | LOOP / HOLD / HOLD / LOOP, 모두 x1 | 분리됐던 TRIPLE/ROTATION을 한 pattern으로 재결합했다. Server contact는 SLASHES 세 번과 SPIN 한 번, 총 네 번이다. |
| `VALTAN_FIST_IN_OUT` | WINDUP 1500 / INNER 2600 / OUTER 800 / RECOVERY 800ms | LOOP / LOOP / EXACT / EXACT, 모두 x1 | Server 판정은 INNER circle 0~7m와 OUTER ring 7~16m로 분리하고, 합성 도넛 visual은 independent library entry로 연다. |
| `VALTAN_HIGH_JUMP` | TAKEOFF 1933 / AIRBORNE 6000 / LAND 3200 / RECOVERY 400ms | 모두 LOOP x1 | TAKEOFF로 상승하고 AIRBORNE에서 정점 pose를 6초 유지한 뒤 LAND에서 잠근 target으로 하강한다. AIRBORNE 진입이 플레이어별 도끼 combat object를 만든다. |
| `VALTAN_FLOOR_WIPE_130` | WINDUP 1800 / FIRST_SMASH 800 / INTERVAL 2000 / SECOND_SMASH 500 / RECOVERY 1500ms | 모두 LOOP x1 | stable ID와 130줄 trigger를 보존했다. animation role은 WINDUP -> SECOND_SMASH -> WINDUP -> SECOND_SMASH 흐름으로 연결하되 gameplay stage ID는 고유하게 유지한다. |
| `VALTAN_ARENA_BREAK_109` | TAKEOFF 900 / DROP 700 / IMPACT 400 / IMPACT_HOLD 1100 / WIDE_REVEAL 2300 / RECOVERY 870ms | EXACT / EXACT / EXACT / EXACT / HOLD / LOOP, 모두 x1 | 표시명은 `중앙 이동 후 2페이즈 컷씬`이다. 기존 camera/world trigger를 stable ID로 참조하고 RECOVERY를 낙사 사자후 end clip에 연결한다. |

표의 LOOP는 JSON `LOOP_TO_STAGE_END`, HOLD는 `HOLD_LAST_POSE`의 축약이다.

각 animation stage는 `EXACT`, `HOLD_LAST_POSE`, `LOOP_TO_STAGE_END` 중 하나의 `endPolicy`를
명시한다. `repeatCount`는 유한한 occurrence 반복이고 `repeatUntilStageEnd`는 남은 stage wall을 채우는
loop이므로 같은 의미로 취급하지 않는다. `EXACT`는 source wall 합계가 Server stage와 2ms 이내로 맞아야
하고, underfill을 암묵적으로 hold하거나 master에 없는 clip을 projector가 합성하지 않는다.

돌진 branch는 다음 typed 경로를 사용한다.

```text
WINDUP --TIMEOUT--> CHARGE

CHARGE --WALL_CONTACT--> GROGGY
CHARGE --TIMEOUT-------> RECOVERY

GROGGY --PART_DESTROYED--> PART_BREAK
GROGGY --TIMEOUT---------> RECOVERY

RECOVERY / PART_BREAK --TIMEOUT--> pattern finish
```

`GROGGY` 진입/이탈은 `boss.flag.groggy`를 Server에서 열고 닫는다. 벽 충돌이나 부위 파괴 코드는
stage 배열의 다음 원소를 추측하지 않고 이 branch graph의 stable action ID를 소비한다.

## 4. Phase-1 weighted normal selection

master의 `normalSelection`과 `ValtanPatternRotations.json` format 2가 다음 두 health band를 정확히
같은 후보 집합으로 투영한다.

| rotationId | Health bar | Mode |
|---|---:|---|
| `rotation.valtan.160.130` | 160 ~ 130 | `WEIGHTED_POOL` |
| `rotation.valtan.130.109` | 130 ~ 109 | `WEIGHTED_POOL` |

| Normal pattern | Weight | Range | Max consecutive | 추가 조건 |
|---|---:|---:|---:|---|
| `VALTAN_WHIRLWIND` | 20 | 0~12m | 2 | ANY armor / ANY phase |
| `VALTAN_DASH_CHARGE` | 30 | 5~20m | 2 | ARMORED / PHASE_ONE |
| `VALTAN_FOUR_SLASH` | 12 | 0~10m | 1 | ANY / ANY |
| `VALTAN_FIST_IN_OUT` | 14 | 0~16m | 1 | maximum health bar 130 |
| `VALTAN_HIGH_JUMP` | 14 | 2~18m | 1 | ANY / ANY |

`CValtanBrain`은 entrance와 이미 예약된 deterministic health mechanic을 먼저 처리한다. normal 선택 시에는
현재 band의 정확한 다섯 pattern ID만 whitelist로 넘기고, 각 pattern의 armor/phase/health/range/cooldown/
maximum-consecutive 조건을 적용한 뒤 남은 후보의 weight로 선택한다. 따라서 이전 ordered introduction이
160~109 구간을 영구 선점하지 않고, master에서 조정한 weight와 조건이 실제 다음 normal 선택에 반영된다.

130줄 `VALTAN_FLOOR_WIPE_130`과 109줄 `VALTAN_ARENA_BREAK_109`는 weight 0인 mechanic이며 기존
health-trigger queue의 결정론적 우선순위를 유지한다. 109줄 아래의 legacy rotation은 이번 일곱 패턴 admission
범위가 아니므로 기존 `ORDERED_INTRO_THEN_WEIGHTED` 동작과 unmanaged row를 보존했다.

## 5. Counter reaction/animation layer

일곱 managed pattern 중 `boss.flag.counterable`을 여는 stage는 없다. 이를 이유 없이 counterable로 바꾸거나
새 animation clip을 발명하지 않았다. master는 기존 Encounter의 모든 실제 counterable stage와 정확히
일치하는 네 `REFERENCE_ONLY_LEGACY` layer를 갖고, `COUNTER_HIT`/`TIMEOUT` branch와 Product animation
binding을 함께 exact join한다.

| Layer owner | Window action / clip | Success action / clip | Failure action / clip |
|---|---|---|---|
| `VALTAN_TRIPLE_COUNTER/COUNTER_1` | `valtan.reactive.triple-counter.first` / `mesh_abn_groggy_1_start` | `valtan.reactive.triple-counter.second` / `mesh_abn_groggy_1_loop` | `valtan.reactive.triple-counter.first-fail` / `mesh_abn_groggy_1_loop` |
| `VALTAN_TRIPLE_COUNTER/COUNTER_2` | `valtan.reactive.triple-counter.second` / `mesh_abn_groggy_1_loop` | `valtan.reactive.triple-counter.third` / `mesh_abn_groggy_1_loop` | `valtan.reactive.triple-counter.second-fail` / `mesh_abn_groggy_1_loop` |
| `VALTAN_TRIPLE_COUNTER/COUNTER_3` | `valtan.reactive.triple-counter.third` / `mesh_abn_groggy_1_loop` | `valtan.reactive.triple-counter.recovery` / `mesh_abn_groggy_1_end` | `valtan.reactive.triple-counter.third-fail` / `mesh_abn_groggy_1_loop` |
| `VALTAN_CENTER_GRAB_COUNTER_64/COUNTER_WINDOW` | `valtan.mechanic.center-grab-counter-64.counter` / `mesh_att_battle_21_03` | `valtan.mechanic.center-grab-counter-64.recovery` / `mesh_att_battle_21_04-1` | `valtan.mechanic.center-grab-counter-64.failed-charge` / `mesh_att_battle_21_04` |

이 layer는 Animation/Effect 작업자가 window/success/failure animation을 같은 typed view에서 검증하게 하지만,
일곱 managed pattern이나 160~109 weighted pool에 legacy pattern을 추가하지 않는다. Product에서 counterable
stage가 늘거나 줄었는데 master layer 집합을 같은 변경에서 갱신하지 않으면 projector와 Client join이 실패한다.

## 6. 도끼와 도넛 independent Effect

`INDEPENDENT EFFECT`는 asset 복사 폴더가 아니라 같은 stable Product Effect를 pattern 밖에서도 단독으로
열고 재생하는 library view다. pattern stage에는 사용 reference만 남기며 같은 Effect의 editable row를
이중으로 만들지 않는다.

| Independent ID | Effect asset | Ownership | Trigger |
|---|---|---|---|
| `valtan.independent-effect.target-axe` | `effect.valtan.sky-axe.active` | `SERVER_COMBAT_OBJECT` | HIGH_JUMP AIRBORNE의 `STAGE_ENTER_PER_ALIVE_PLAYER` |
| `valtan.independent-effect.donut-in-out` | `effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01` | `SERVER_PATTERN_STAGE` | FIST_IN_OUT INNER의 `PATTERN_TIMELINE` Product cue |

도끼는 `combatobject.valtan.high-jump.target-axe`와
`combatobject.visual.valtan.high-jump.target-axe.v1`의 typed mapping을 사용한다. Server가 world origin,
life, damage와 복제를 소유하고 Client는 BossCatalog가 연결한 visual Effect만 그린다. 도넛은 INNER clip
occurrence의 Product cue offset, anchor/follow/local transform과 stop window를 그대로 사용한다.

두 independent ID는 선언한 owner stage에 각각 정확히 한 번만 참조된다. 도넛의 owner는
`VALTAN_FIST_IN_OUT/INNER`이고 `OUTER.effectRefs`는 비어 있다. INNER가 unified Effect를 한 번 시작하며
OUTER stage는 같은 independent ID를 다시 시작하지 않는다. projector focused harness와 Effect Tool contract
test는 다른 stage의 두 번째 참조를 실패시키며, Client exact join도 owner stage가 다르면
`master independent Effect reference is stale`로 admission을 거부한다. 실제 Arena와 Effect Tool authoring
preview는 모두 `stopPolicy: natural`인 Effect의 문서 lifetime을 다음 stage까지 유지한다. 따라서 INNER에서
한 번 spawn한 unified Effect가 OUTER까지 이어지고, 명시적 `CUE_END`만 source end window에서 종료된다.

### 현재 도끼 범위와 후속 경계

현재 구현된 도끼 origin policy는 **`LOCKED_TARGET_PER_ALIVE_PLAYER`**다. AIRBORNE 진입 시 살아 있고 낙하
중이 아닌 플레이어마다 target을 하나씩 잠그고 도끼 combat object를 한 번 생성한다. 각 object의 life는
AIRBORNE과 같은 6000ms이고, Server fixed tick은 그 target의 유효 위치를 첫 timed pulse인 1200ms까지
추적한다. 첫 pulse를 발사하면 follow flag를 끄고 마지막 위치에 고정한다. 판정은 1200ms에 한 번,
CIRCLE 3.5m, `damage.valtan.high-jump`이며 visual은 BossCatalog의
`combatobject.visual.valtan.high-jump.target-axe.v1 -> effect.valtan.sky-axe.active` mapping을 쓴다.

**랜덤 아레나 위치 도끼 생성은 이번 구현에 포함되지 않았다.** 이를 추가할 때는 Effect를 복제하거나
Client 좌표를 정답으로 사용하지 않고, 다음 항목을 하나의 별도 Server vertical slice로 닫아야 한다.

1. combat-object의 random arena-position origin policy와 authoring field
2. encounter/session/action에 귀속된 결정론적 RNG stream과 seed 소비 순서
3. arena bounds, navigation projection, 최소 간격과 실패 정책
4. 확정된 spawn origin의 Server snapshot과 Client world-root presentation
5. 같은 origin에서 damage/hit와 Effect가 일치하는 contract harness

## 7. Animation Tool과 Effect Tool 동작

### Animation Tool

- `Valtan Pattern Master (Authoritative)`가 일곱 managed pattern의 primary authoring 화면이다.
- master stage, occurrence, source start/play time, play rate와 Server wall을 사용해 전체 body timeline을
  재생하고 seek한다.
- 돌진은 hardcoded stage 배열이 아니라 master branch graph에서 normal, wall-groggy, part-break path를 만든다.
- `presentationSources`, source action/sequence index와 각 stage의 `endPolicy`를 화면에 표시한다.
- 새 preview는 전체 timeline admission이 성공한 뒤에만 기존 pose를 교체하고, source reference preview와
  동시에 두 animation owner를 실행하지 않는다.
- 기존 `Valtan.patternpreview.json`/`Valtan.clipseq`의 1~67 화면은
  `Secondary / Read-only Source Reference`로 유지한다. 패턴 정답이나 runtime binding이 아니다.
- Phase-1 weighted range/후보/조건과 네 counter reaction action/clip도 같은 typed projection에서 확인한다.

### Effect Tool

- All Effects의 Valtan root에 pattern tree와 같은 레벨의 `INDEPENDENT EFFECT` tree를 추가했다.
- 도끼와 도넛은 각각 한 번만 나열되고, owner pattern 아래에는 같은 stable asset의 reference만 보인다.
- `Play Authoring Timeline`은 선택한 branch의 모든 stage body occurrence를 순서대로 재생한다.
- pattern Effect를 선택한 상태로 전체 timeline을 재생하면 stage-global cue offset, anchor/follow/local
  transform, stop window와 Effect clock을 유지한다.
- 현재 authored Effect가 valid하지만 drawable element가 없으면 Effect 시작 실패만 상태로 표시하고 body
  animation 전체 timeline은 animation-only로 계속 재생한다. `Play Saved Effect`가 drawable 전까지 막히는
  것과 `Play Authoring Timeline`이 animation 검증을 계속 허용하는 것은 서로 다른 계약이다.
- 도넛 `SERVER_PATTERN_STAGE`는 Product cue 경로로, 도끼 `SERVER_COMBAT_OBJECT`는 replicated world-root
  경로로 preview ownership을 구분한다.
- saved Effect 문서의 element decode는 Open/Play 전까지 지연해 All Effects tree refresh가 모든 Effect를
  선제 decode하지 않는다.
- `COUNTER REACTION (REFERENCE ONLY)`는 기존 exact action/clip만 보여 주며 current managed pattern으로
  승격하거나 별도 gameplay를 만들지 않는다.
- `Play Server Pattern`은 replicated Valtan과 Server command 경로를 사용하는 별도 검증이다. authoring
  timeline preview가 Server pattern을 대신하지 않는다.

## 8. Server authority와 담당별 작업 경계

### 보스 패턴 담당자

master에서 health/phase/armor/range/weight/maximum-consecutive 조건, stage duration, hit shape/schedule,
`serverDamageProfileId`, motion, combat-object action과 branch graph를 조정한다. damage 숫자를 master에
복제하지 않고 `DamageProfiles.json`의 stable profile을 참조한다. 실제 target, 이동, hit overlap, damage,
groggy/counter flag, mechanic 예약과 combat-object origin은 Server fixed tick이 확정한다.

### 애니메이션 담당자

stage의 ordered occurrence, source window, play rate, `repeatCount`, `endPolicy`, `mappingBasis`와 승인된
`presentationSources`를 조정한다. Server bootstrap에 clip 이름을 넣지 않고, Tool이 exact Product join에
성공하는지 전체 timeline으로 확인한다.

### Effect 담당자

master에서는 stable `effectRef`, cue occurrence와 owner stage를 확인한다. element, Revolution, Life Time,
anchor/local transform의 실제 저작은 `Data/Effects/Authored/*.effect.json`이 소유한다. Effect preview의
world proxy, mesh와 collider는 gameplay hit 권위가 아니다.

### UI 담당자

damage font는 master나 DamageProfiles에서 예상 수치를 다시 계산하지 않는다. Server가 보낸
`DAMAGE_EVENT.iAmount`만 표시한다.

## 9. Historical receipt와 live Product 책임 분리

4연속 공격을 `VALTAN_FOUR_SLASH`로 재결합하면서 현재 제품 owner와 과거 migration 증거의 책임을
분리했다.

- live `VALTAN_FOUR_SLASH` Encounter/binding/cue owner는 pattern master와 projector가 소유한다.
- 과거 `VALTAN_TRIPLE_SLASH`/`VALTAN_ROTATION_SLASH` 분리 owner는 `retiredPatternIds`이며 current Product
  cue owner로 다시 들어올 수 없다.
- `Data/Animation/Authored/Valtan/Valtan.pattern-occurrence-v2-migration.receipt.json`과
  `Data/Effects/Imported/Valtan/CarrierV1/Valtan.carrier-v1-materialization-receipt.v1.json`은 당시의
  migration/carrier `baselineIdentity`를 보존하는 historical evidence다. master publish가 이 baseline을
  현재 제품 identity로 다시 봉인하지 않는다.
- 기존 migration/reseal 도구는 rejoined successor가 존재하면 현재 binding/cue의 exact successor identity만
  검증한다. historical mutation을 live authoring에 재적용하지 않으며, valid successor에 대한 write mode는
  identity operation이다.
- live Product가 historical Carrier 문서와 달라진 일곱 Effect는
  `Valtan.carrier-v1-successor-lineage-receipt.v1.json`이 `RETAIN`, `REPLACED_BY`, `INTENTIONALLY_REMOVED`와
  새 element ID를 명시한다. 기존 5MB historical receipt는 한 바이트도 다시 봉인하지 않는다.
- materializer는 current master의 109 컷씬 selection `420629 sequence 3`을 historical `sequence 1`로
  투영한 뒤 기존 분모 `exact 660 / materialized 657`을 그대로 재현하고, successor가 소유한 문서는
  validate-only로 취급한다. 따라서 팀원의 Product Effect를 historical writer가 다시 덮어쓰지 않는다.
- EffectPipeline은 imported source inventory, carrier evidence와 historical receipt의 보존/검증을 계속
  소유한다. pattern master projector는 현재 pattern/stage/action/cue join만 소유한다.

따라서 과거 receipt가 현재 Product를 되돌리는 writer가 되지 않고, 반대로 live master 변경이 원본 추출과
migration의 historical 증거를 덮어쓰지도 않는다.

## 10. 자동 검증 결과

현재 branch에서 확인된 focused 검증은 다음과 같다.

| 검증 | 결과 |
|---|---|
| `Project-ValtanPatternMaster.ps1 -Mode Validate` | PASS |
| `Test-ValtanPatternMaster.ps1` | PASS |
| `test_animation_tool_valtan_pattern_master.py` + `test_valtan_pattern_tree_contract.py` | 15 tests PASS |
| `test_effect_tool_valtan_saved_rows.py` | 28 tests PASS |
| Carrier materializer + successor lineage suites | 10 + 9 tests PASS |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS |

focused harness는 정상 publish/validate뿐 아니라 잘못된 property/ID, duplicate, Product drift, branch drift,
중간 commit 실패 rollback, unmanaged row 보존, weighted pool에 unrelated legacy ID 삽입, 존재하지 않는 counter
action 삽입을 거부하는 경로를 검사한다. Tool contract tests는 정확히 일곱 pattern, stage wall과 source clock,
graph-driven dash path, 세 end policy, seek, read-only legacy reference, exact weighted rotation과 counter
Product join을 검사한다.

표준 Debug wrapper도 실행했다.

| 검증 | 결과 |
|---|---|
| `Invoke-BuildAndRegression.ps1 -Configuration Debug` Engine/UpdateLib/Shared/Server/Client build | PASS, `Client/Bin/Debug/Client.exe` link 완료 |
| Debug wrapper 내 Valtan master publish/validate/focused harness | PASS, 7 patterns / 31 stages / 2 independent effects / 4 counter reactions / 15 cue refs |
| Debug wrapper 내 Valtan Python + Effect Tool tests | 7 + 28 tests PASS |
| Debug wrapper 내 Gameplay balance/world/items/navigation/Effect catalog validation | PASS |
| `Sync-EffectDataProject.ps1 -Check` | PASS: 2304 files / 219 filters |
| `materialize_valtan_carrier_v1.py --mode check` | PASS: APPLIED / exact 660 / materialized 657 / changed 0 |
| Server x64 Debug + `Server.exe --contract-test` | PASS: failures 0 |
| Debug wrapper 최종 exit | Valtan gates 이후 기존 floor-emissive source assertion 1건에서 FAIL |
| `Invoke-BuildAndRegression.ps1 -Configuration Release` | 긴급 merge 전 미실행 |

사용자가 요청한 SLN pull/build 계약을 닫기 위해 stale였던 Effect generated group을 동기화했고,
`Client.vcxproj/.filters`는 현재 2304개 DataFiles와 219개 filter의 generator 출력이다. Debug wrapper의 남은
실패는 이 변경과 무관하게 main에도 존재하던 floor-emissive test가 현재 두 mesh loop 구현에서 과거
`Render(EMISSIVE_MESH_INDEX)` 문자열만 기대하는 source assertion이다. 실제 resource root를 연결한 재검사에서
나머지 다섯 floor-emissive 검사는 통과했고 이 한 건만 실패했다. 발탄 Server contract는 별도로 전체 PASS했다.

이번 긴급 merge는 위 Debug 전체 compile/link, 발탄 projector/harness, typed Tool regression과 domain
publisher PASS를 근거로 한다. Release와 사용자 Client visual gate는 실행하지 않았다는 사실을 그대로
남기며 PASS로 추정하지 않는다.

## 11. 사용자 수동 검증

에이전트는 Client/UI를 자율 실행하거나 visual fidelity를 대신 PASS 처리하지 않는다. merge와 로컬 정본 동기화,
Server 재시작 뒤 사용자가 다음을 직접 확인해야 한다.

1. Animation Tool의 `Valtan Pattern Master (Authoritative)`에서 일곱 pattern과 위 stage 시간이 같은지 확인한다.
2. 휠윈드 전체 timeline에서 SPIN이 1.2초이고 두 body occurrence와 Effect Revolution이 실제로 회전하는지
   확인한다.
3. 돌진 normal / wall-groggy / part-break path에서 WINDUP 세 occurrence, 실제 전진 이동, 그로기와 회복
   animation 순서가 맞는지 확인한다.
4. 4연속 공격이 하나의 WINDUP/SLASHES/SPIN/RECOVERY timeline으로 재생되는지 확인한다.
5. All Effects의 `INDEPENDENT EFFECT`에 도끼와 도넛이 각각 한 번만 나오고 단독 Play가 되는지 확인한다.
6. HIGH_JUMP Server Pattern에서 살아 있는 플레이어마다 잠긴 target 위치에 도끼가 생성되고 AIRBORNE 6초,
   LAND 하강과 visual lifetime이 의도대로 맞는지 확인한다.
7. FIST_IN_OUT Server Pattern에서 INNER/OUTER 판정과 도넛 cue/anchor가 같은 timeline에 맞는지 확인한다.
8. FLOOR_WIPE_130과 ARENA_BREAK_109의 body sequence, camera cue, 벽 파괴/world event를 실제 Arena에서
   확인한다.
9. `COUNTER REACTION (REFERENCE ONLY)`에서 네 legacy layer의 window/success/failure action과 clip이 위 표와
   같고, Phase-1 weighted pool에는 추가되지 않았는지 확인한다.

위 화면 판정은 모두 **`PENDING_USER_VISUAL_GATE`**다. 특히 Revolution, anchor follow, 실제 돌진 mesh 이동,
camera와 wall destruction의 최종 시각 품질은 사용자 서면 확인 전까지 완료로 승격하지 않는다.

## 12. 명시적으로 남은 범위

- 랜덤 아레나 위치 도끼 생성은 미구현이다. 현재 정답은 `LOCKED_TARGET_PER_ALIVE_PLAYER`다.
- 현재 추적은 도끼의 6000ms life 전체가 아니라 첫 1200ms timed pulse까지다. 첫 pulse 뒤에는 마지막 위치에
  고정되며 지속 추적·다중 pulse도 미구현이다.
- 도넛 independent reference는 INNER 한 곳에만 존재한다. OUTER의 Server ring 판정은 유지하지만 같은
  visual reference를 OUTER에 중복하지 않는다.
- 도넛은 INNER natural cue 한 번으로 OUTER까지 문서 lifetime을 유지한다. OUTER에서 같은 independent ref를
  다시 spawn하지 않으며 실제 visual fidelity는 사용자 timeline/Arena 판정으로 남는다.
- `휠윈드 1.2초`는 전체 pattern이 아니라 SPIN stage 1.2초다.
- 돌진 WINDUP은 의미상 세 occurrence지만 현행 source wall은 동일 600ms 세 번이 아니라
  600+600+2450ms다. 정확히 600ms x3으로 바꾸려면 stage/cue/pose를 함께 다시 승인해야 한다.
- `VALTAN_FLOOR_WIPE_130`은 130줄 stable trigger를 유지한다. 115줄로 바꾸는 작업은 rotation과
  health mechanic을 함께 변경하는 별도 승인 범위다.
- 109줄 이하 legacy pattern 전체를 이번 master에 admission하지 않았다. 범위를 늘릴 때는 master,
  projector, typed Tool join, Server rotation과 harness를 같은 변경 단위로 확장해야 한다.

## 13. 2026-08-24 긴급 회귀 재감사

최신 정본/투영/Tool 변경을 다시 감사해 다음 회귀를 닫았다.

- 도넛 independent ID는 `FIST_IN_OUT/INNER`에만 한 번 존재한다. projector가 전체 managed pattern에서 같은
  ID의 참조 수를 세고 선언 owner stage의 정확히 한 행이 아니면 실패한다. `OUTER` 중복 참조 fixture도 focused
  harness에서 거부한다.
- Effect Tool의 Valtan tree 최초 load가 실패하면 매 frame JSON을 다시 읽지 않고 명시적 Refresh에서만 재시도한다.
- Valtan natural Product cue preview의 무조건적인 owner-stage end clamp를 제거했다. Arena와 Tool 모두 unified
  donut을 INNER에서 한 번 시작해 문서 lifetime으로 OUTER까지 유지하며, `CUE_END`는 source window로 계속
  제한한다.
- 복구된 도넛/점프/도끼 authored 문서와 runtime sealed 문서의 JSON/sha identity가 다시 일치한다. Effect Tool
  전체 suite가 모든 published V0 cue의 authored/runtime equality와 non-empty drawable inventory를 함께 검사한다.
  stable-ID ordered union 결과는 도넛 24, TAKEOFF 3, LAND 24, sky axe 6 elements이고 누락/중복은 0이다.

최종 read-only 재감사 결과는 다음과 같다.

| 검증 | 최신 결과 |
|---|---|
| 7-pattern exact static audit | PASS: 7 patterns / 31 stages / 5 weighted normals / 2 mechanics / dash repeat 3 / 2 independent effects / 4 counter layers |
| selection/damage reference audit | PASS: weights `20,30,12,14,14`, managed damage profile 8개 exact join |
| `Project-ValtanPatternMaster.ps1 -Mode Validate` | PASS: 7 / 31 / 2 / 4 / 15, product drift check true |
| `Test-ValtanPatternMaster.ps1` | PASS: invalid owner-stage duplicate와 transactional fixture 포함 |
| `test_valtan_pattern_tree_contract.py` | 8 tests PASS |
| `test_animation_tool_valtan_pattern_master.py` | 7 tests PASS |
| `test_effect_tool_valtan_saved_rows.py` | 28 tests PASS |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS: 33 boss patterns / 131 stages / 2 combat objects |
| `Publish-Effects.ps1` explicit team ResourceRoot pre-Validate / Publish / post-Validate | 모두 PASS: Validate 162 Effects / 171 bindings, Publish 162 Effects / 0 Components |

Effect publisher는 팀이 관리하는 실제 resource root
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources`를 `-ResourceRoot`로 명시해 pre-Validate, Publish,
post-Validate를 실행했다. 위 28-test focused suite도 이번 네 authored Effect와 runtime sealed 문서를 포함한 모든
published Valtan V0 cue의 exact equality를 통과했다. 최신 C++ 변경 뒤 Client Debug link와 Server Debug 전체
contract test도 다시 통과했다. 사용자 visual gate만 에이전트가 대신 실행하지 않았으며 merge 뒤 수동 확인해야 한다.
