# 2026-08-15 발탄 패턴 이펙트 전수조사와 복원 계획

이 문서는 4직업 Track A 복원과 **병렬로** 준비할 수 있는 발탄(BOSS_VALTAN) 패턴 이펙트 복원의
현재 상태 전수조사와 실행 계획이다. 조사는 전부 read-only 실측이며 추정하지 않았다.

결론을 먼저 적는다. **발탄은 "정보가 없어서" 막힌 것이 아니다.** 원본 UPK 이펙트 증거는 캐릭터와
거의 같은 모양으로 이미 존재하고, 제품 패턴 31개 전부를 덮는다. 막힌 이유는 캐릭터용 파이프라인을
발탄 축(`actionId`)으로 돌려주는 **어댑터 한 개가 없어서**다.

---

## 1. 캐릭터 경로와 발탄 경로 대조

캐릭터는 다음 축으로 원본 UPK 이펙트를 제품에 연결한다.

```text
Data/Balance/PlayerSkills.json      (characterClass, inputSlot) -> skillId
  -> <Asset>.skillbindings.json     skillId -> ordered model clips
    -> clip 안의 effectref=asset animevent cue
      -> Data/Effects/Authored/<effect>.unified.effect.json
```

발탄은 입력 슬롯이 없고 Server가 확정하는 **패턴/액션**이 그 자리를 차지한다.

```text
Data/Encounters/Valtan/ValtanEncounter.json   patternId -> actionId / sourceActionIds
  -> Valtan.patternbindings.json              actionId  -> clip
    -> Valtan.effect-resource-catalog.json    actionIds/clipNames -> source particle system
      -> (아직 없음) Authored Effect 문서
```

**유일한 구조적 차이**는 cue의 저장 위치다.

```text
Data/Animation/Reference/Valtan/Valtan.animevents   879줄
  effectref= cue 개수                                    0
```

캐릭터는 clip 안의 `effectref=` animevent가 이펙트를 가리킨다. 발탄 animevents에는 그 cue가
**하나도 없다.** 대신 같은 정보가 catalog의 `actionIds` / `clipNames` 필드에 들어 있고
`bindingStatus`는 `ACTION_NOTIFY_BOUND`다. 즉 정보는 있고 **위치만 다르다.**

---

## 2. 현재 저장 상태 실측

### 2.1 애니메이션 축 — 거의 완료

```text
Data/Animation/Authored/Valtan/Valtan.patternbindings.json
  schema lostark.valtan-pattern-bindings, bossArchetypeId BOSS_VALTAN
  bindings 112   (actionId -> clip)
  예: { "actionId": "valtan.attack.swing.windup", "clip": "mesh_att_battle_1_01" }
```

112개 binding을 패턴 단위로 접으면 **30 패턴 그룹**이고, 각 패턴이 windup/active/recovery 등
stage로 나뉜다.

```text
7 stage  valtan.reactive.triple-counter
6 stage  valtan.mechanic.ghost-transition-15
5 stage  valtan.mechanic.floor-wipe-130
5 stage  valtan.mechanic.center-grab-counter-64
4 stage  valtan.attack.earthquake-smash / magic-choice / high-jump /
         bind-charge-smash / jump-spin / portal-rush / charge-grab-roar / fist-in-out
4 stage  valtan.mechanic.four-pillars-105 / arena-break-80
```

### 2.2 이펙트 축 — 1/31

```text
Data/Animation/Authored/Valtan/Valtan.patterneffects.json
  schema lostark.boss-pattern-effects
  bindings 1     ← VALTAN_WHIRLWIND 420633 active 하나뿐

Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json
  first-LOD carrier 9 중 ordinary 실행 3 / fail-closed 6
  productAdmission FAIL_CLOSED_NON_PRODUCT_CANARY
  EffectCatalog / animation event 제품 mapping 모두 false
```

### 2.3 원본 UPK 증거 — 이미 풍부하다

```text
Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json
  schema  lostark.unbound-class-particle-resource-catalog
  bindingStatus            ACTION_NOTIFY_BOUND
  sourcePackageGraphs      17
  sourceSystems           193
  assets                  855
  materialParameterBindings 335
  unresolvedLogicalPackages  0
  missingActionSourceSystems 0
  distinct actionId       140

Data/Effects/Imported/Valtan/Valtan.action-particle-resource-catalog.json
  sourceSystems 165 / assets 793 / materialParameterBindings 299

추출 원본 요약 (MN_RPBF_00.action-effects.json)
  action object 109 / stage 2,100 / clip occurrence 2,071 / notify 19,237
  unique particle system 153
```

sourceSystem 한 행의 실제 모양이다. 캐릭터의 `skillId -> clips -> effect`와 구조가 같다.

```json
{
  "sourceAsset": "FX_BS_00.hit.par_d_groggyhit_01",
  "logicalPackage": "FX_BS_00",
  "resolutionStatus": "ACTION_BOUND_SOURCE_SYSTEM",
  "actionIds":   [420627, 420628, 420654, 420676, 420677, 420678, 4273305],
  "actionNames": ["레이드 발탄_1번째 외벽 파괴", ...],
  "clipNames":   ["Abn_Groggy_1_Start", "Dmg_Parts_Start_1"],
  "occurrenceCount": 13,
  "graph": { "rootNodeId": "FX_BS_00:export:23306", "resourceBindings": ... }
}
```

---

## 3. 제품 패턴 31개 × 원본 커버리지

`ValtanEncounter.json`의 제품 패턴 31개를 catalog의 `actionIds`와 조인한 실측이다.
**31/31 전부 원본 particle system을 가진다. 미커버 패턴은 0이다.**

| systems | patternId | 표시 이름 | sourceActionIds |
|---:|---|---|---|
| 140 | `VALTAN_GHOST_TRANSITION_15` | 15줄 최종 지면파괴와 망령화 | 420616, 420624~420626, 420634, 420651~420653, 420658, 420659, 420665 |
| 58 | `VALTAN_TRIPLE_COUNTER` | 연속 카운터 내려찍기 | 420640~420647 |
| 47 | `VALTAN_SUPER_SMASH` | 초강력 내려찍기 콤보 | 420619, 420620, 420656, 420657 |
| 26 | `VALTAN_BACKSTEP_ATTACK` | 공격하면서 뒤로 빠지기 | 420635, 420664 |
| 24 | `VALTAN_FRONT_BACK_FRONT` | 앞뒤앞 내려찍기 | 420637, 420666 |
| 20 | `VALTAN_JUMP_SPIN` | 점프 찍기 후 휠윈드 | 420621, 420663 |
| 20 | `VALTAN_EARTHQUAKE_SMASH` | 지진 찍기 | 420605, 420662 |
| 20 | `VALTAN_DOWN_SMASH` | 내려찍기 | 420602, 420661 |
| 20 | `VALTAN_CHARGE_GRAB_ROAR` | 돌진 잡기 후 사자후 | 420623, 420631, 420632 |
| 19 | `VALTAN_BIND_CHARGE_SMASH` | 구속 돌진 잡기 후 내려찍기 | 420612~420614 |
| 15 | `VALTAN_ARMOR_BREAK_OPENING` | 오프닝 외벽 충돌 부위 파괴 | 420627, 420628, 420654, 420655 |
| 14 | `VALTAN_SWING` | 휘두르기 | 420601, 420660 |
| 14 | `VALTAN_PARRY` | 큰 베기 반격 | 420606, 420607 |
| 14 | `VALTAN_FOUR_SLASH` | 4연속 베기 | 420609 |
| 14 | `VALTAN_CENTER_GRAB_COUNTER_64` | 64줄 중앙 표적 폭발과 잡기 카운터 | 420623, 420631 |
| 13 | `VALTAN_MAGIC_CHOICE` | 마력기운 양자택일 | 420608 |
| 13 | `VALTAN_FLOOR_WIPE_130` | 130줄 2연속 지면 강타 전멸기 | 420630 |
| 11 | `VALTAN_IMPRISON_ROAR` | 감금 사자후 | 420603 |
| 11 | `VALTAN_GROUND_WAVE_SMASH` | 지진파 내려찍기 | 420615 |
| 9 | `VALTAN_MAGIC_ORB_STAGGER_76` | 76줄 마력구 방어막 무력화 | 420617, 420618 |
| 8 | `VALTAN_RED_BLADE_WAVE` | 붉은 검기 날리기 | 420636 |
| 8 | `VALTAN_HIGH_JUMP` | 고공 점프 찍기 | 420610 |
| 8 | `VALTAN_FOUR_PILLARS_105` | 105줄 4기둥 추적 원뿔 | 420610 |
| 6 | `VALTAN_PORTAL_RUSH` | 워프 돌진 콤보 | 420622 |
| 6 | `VALTAN_LEDGE_ROAR` | 낙사 사자후 | 420639 |
| 6 | `VALTAN_FIST_IN_OUT` | 두 손 내려찍기 안밖 폭발 | 420638 |
| 5 | `VALTAN_WHIRLWIND` | 휠윈드 | 420633 |
| 5 | `VALTAN_ARENA_BREAK_33` | 33줄 두 번째 지형 파괴 | 420629 |
| 5 | `VALTAN_ARENA_BREAK_109` | 109줄 아레나 붕괴 | 420629 |
| 4 | `VALTAN_DASH_CHARGE` | 대쉬 돌진 | 420604 |
| 3 | `VALTAN_STOMP` | 발구르기 | 420611 |

`VALTAN_WHIRLWIND`는 systems 5로 **하위권**이다. 지금까지 유일하게 만든 canary가 원본 규모로는
가장 작은 축에 속한다는 뜻이며, 나머지 30개는 이보다 크다.

---

## 4. 복원 방향 — 캐릭터 파이프라인 재사용

신규 작업은 **[A] 하나뿐**이고 나머지는 이미 있는 것을 발탄 입력으로 돌리는 것이다.

### [A] actionId 축 어댑터 — **구현 완료**

`Tools/EffectPipeline/build_valtan_action_bindings.py`로 구현했고 실행했다.
출력은 `Data/Animation/Authored/Valtan/Valtan.actionbindings.json`(4.2 MB)과 receipt다.

```text
patternCount                 31
patternsWithSourceActions    31    ← 누락 0
patternsWithStages           29    ← 2개는 §4.1 참조
authoredStageCount          108
sourceActionReferenced       63 / indexed 152
unmappedSourceActionCount    89    ← 제품 패턴 밖 (피격/그로기 반응 등)
missingSourceActionCount      0
effectCueCount            9,175
  Effect             3,475   ← 캐릭터 effectref= 와 동일 notify
  PlayParticleEffect 4,791
  PlayDecalEffect      529
  Trails               259
  TrailGhostEffect     100
  DefaultParticle       21
```

cue가 많은 패턴 상위는 다음과 같다. §3의 systems 순위와 대체로 일치하지만
`VALTAN_SWING`(581)과 `VALTAN_DASH_CHARGE`(412)처럼 systems 기준으로는 낮았는데 실제 cue는 많은
패턴이 있다. 착수 우선순위는 cue 수를 함께 본다.

```text
1,609 cues / 6 stage  VALTAN_GHOST_TRANSITION_15  15줄 최종 지면파괴와 망령화
  889 cues / 7 stage  VALTAN_TRIPLE_COUNTER       연속 카운터 내려찍기
  598 cues / 3 stage  VALTAN_SUPER_SMASH          초강력 내려찍기 콤보
  581 cues / 3 stage  VALTAN_SWING                휘두르기
  538 cues / 3 stage  VALTAN_FRONT_BACK_FRONT     앞뒤앞 내려찍기
  463 cues / 4 stage  VALTAN_EARTHQUAKE_SMASH     지진 찍기
  443 cues / 3 stage  VALTAN_DOWN_SMASH           내려찍기
  412 cues / 3 stage  VALTAN_DASH_CHARGE          대쉬 돌진
```

#### 4.1 stage가 없는 패턴 2개

`Valtan.patternbindings.json`에 해당 gameplay actionId 접두사의 stage가 없다. 원본 cue는 있다.

```text
VALTAN_LEDGE_ROAR       valtan.attack.ledge-roar          cues 39
VALTAN_ARENA_BREAK_109  valtan.mechanic.arena-break-109   cues 42
```

이름 추정으로 clip을 만들지 않았다. 애니메이션 담당자가 이 두 패턴의 stage 바인딩을 추가하면
어댑터를 다시 돌리는 것만으로 닫힌다.

#### 4.2 원래 계획했던 설계 (참고)

캐릭터의 `skillbindings.json`(skillId → ordered clips)에 대응하는 발탄 문서를 catalog에서
생성한다.

```text
입력 (전부 read-only)
  ValtanEncounter.json          patternId -> actionId / sourceActionIds
  Valtan.patternbindings.json   actionId  -> clip
  Valtan.effect-resource-catalog.json
                                sourceSystems[].actionIds / clipNames / graph

출력 (신규)
  Data/Animation/Authored/Valtan/Valtan.actionbindings.json
    (patternId, stageId, actionId) -> ordered clips -> source particle systems
```

계약으로 고정할 것.

1. `actionId`는 encounter 문서에 실재하는 값만 사용한다. catalog에만 있는 140개 중
   제품 패턴 밖 action은 별도 목록으로 분리하고 제품 대상에 넣지 않는다.
2. clip은 `patternbindings.json`이 이미 선언한 것만 사용한다. 이름 추정으로 clip을 만들지 않는다.
3. 하나의 sourceSystem이 여러 actionId에 묶여 있으면(위 예시는 7개) 그 다중성을 보존한다.
   임의로 하나를 골라 다른 것을 버리지 않는다.
4. 매핑 키는 기존 계약 그대로 `BOSS_VALTAN + patternId + actionId`다.

### [B] 기존 materializer 재사용

`Tools/EffectPipeline/materialize_four_class_track_a_candidates.py`와 같은 계약으로
`effect.valtan.pattern.<actionId>.<stage>.unified.effect.json`을 생성한다. 새 materializer를
만들지 않는다. 4직업에서 닫은 admission/fidelity/override 계약을 그대로 상속한다.

### [C] Wave 0 evidence를 발탄으로 확장

4직업과 동일 절차로 발탄 parent Material3 props를 추출한다. 인프라는 전부 재사용한다.

```text
extract_umodel_material_dependencies.py  (--parent-package-map 포함)
build_dimensionmaster_source_material_evidence.py  (FOUR_CLASS 일반화 완료)
  -> Data/Effects/Imported/Valtan/Valtan.source-material-evidence.json
```

발탄 catalog는 `sourcePackageGraphs 17` / `unresolvedLogicalPackages 0`이라 패키지 해석은
이미 닫혀 있다.

### [D] All Effects 나열

상단 `Character / Boss` selector의 Valtan은 Codex가 이미 추가했다. 그 아래를
`패턴 31 -> stage -> element` 트리로 구성해 캐릭터와 동일하게 Open / Play All / Family / Solo를
제공한다.

---

## 5. 우선순위

원본 규모(systems)와 전투 노출을 함께 본 순서다. 실제 착수 순서는 4직업 종료 뒤 결정한다.

| 순위 | 대상 | 근거 |
|---:|---|---|
| 1 | `VALTAN_GHOST_TRANSITION_15` (140) | 최대 규모, 페이즈 전환 핵심 |
| 2 | `VALTAN_TRIPLE_COUNTER` (58) | 카운터 기믹, 반복 노출 |
| 3 | `VALTAN_SUPER_SMASH` (47) | 군단장 스킬 |
| 4 | 기본 공격군 `SWING` / `DOWN_SMASH` / `FOUR_SLASH` / `EARTHQUAKE_SMASH` | 가장 자주 보이는 패턴 |
| 5 | 아레나 기믹 `ARENA_BREAK_109` / `FLOOR_WIPE_130` / `FOUR_PILLARS_105` | 이미 collapse 런타임 존재 |
| — | `VALTAN_WHIRLWIND` (5) | 이미 canary 존재. 회귀 control로 유지 |

---

## 6. 4직업 작업과의 병렬 안전성

현재 다른 세션이 만지고 있는 파일을 실측했다.

```text
Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli
Client/Private/Effect_DocumentRenderer.cpp
Client/Private/Effect_Playback.cpp
Client/Public/Effect_DocumentRenderer.h
Client/Public/Effect_MaterialTemplate.h
Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp
```

[A]와 [C]가 만지는 파일은 다음과 같다.

```text
신규  Tools/EffectPipeline/build_valtan_action_bindings.py
신규  Data/Animation/Authored/Valtan/Valtan.actionbindings.json
신규  Data/Effects/Imported/Valtan/Valtan.source-material-evidence.json
읽기  Data/Effects/Imported/Valtan/*.json
읽기  Data/Encounters/Valtan/ValtanEncounter.json
읽기  Data/Animation/Authored/Valtan/Valtan.patternbindings.json
```

**겹치는 파일이 0이다.** [A]와 [C]는 지금 병렬로 진행해도 충돌하지 않는다.

반대로 다음은 **병렬 금지**다.

| 작업 | 사유 |
|---|---|
| [B] materializer 실행 | `materialize_four_class_track_a_candidates.py`와 `EXPECTED_*` 공유. 4직업 재기준선과 충돌 |
| [D] Effect Tool 트리 | `Effect_Tool.cpp`를 다른 세션이 수정 중 |
| 101/발탄 문서 write | 4직업 baseline 육안검증 중. 지금 쓰면 원인 분리 불가 |

즉 **지금 병렬로 할 수 있는 것은 [A] 어댑터와 [C] evidence 추출까지**이고,
[B]/[D]는 4직업이 닫힌 뒤다.

---

## 7. 지켜야 할 경계

- 발탄 transform/action/phase/damage는 Server authority다. 이펙트는 표현만 담당하며
  `BOSS_VALTAN + patternId + actionId` 매핑 키를 바꾸지 않는다.
- catalog의 140 actionId 중 제품 패턴 밖(`Abn_Groggy`, `Dmg_Parts` 등 피격/그로기 반응)은
  제품 대상으로 승격하지 않고 별도 목록으로 남긴다.
- 4직업에서 닫은 admission 3분류(Full / Authoring Approximate / Hard)와 cue-scoped product
  승인 정책을 그대로 상속한다. 발탄 전용 완화 규칙을 만들지 않는다.
- 이름만 보고 clip이나 이펙트를 추정 연결하지 않는다. `actionIds`/`clipNames` 실측 값만 쓴다.
- 화면 판정은 사용자 전용이다.

## 8. 이 문서가 실행한 검증

read-only 실측만 수행했다. 코드/데이터 변경은 없다.

- `ValtanEncounter.json` 31 pattern / 7 state 파싱
- `Valtan.patternbindings.json` 112 binding, 30 pattern group 집계
- `Valtan.patterneffects.json` 1 binding 확인
- `Valtan.effect-resource-catalog.json` 193 system / 855 asset / 140 actionId 집계
- `Valtan.action-particle-resource-catalog.json` 165 system / 793 asset 집계
- `Valtan.animevents` 879줄 중 `effectref=` 0건 확인
- encounter 31 pattern × catalog actionId 조인, 커버리지 31/31
- 다른 세션 활성 파일 목록과 [A]/[C] 대상 파일 교차 확인
