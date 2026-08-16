# 2026-08-15 발탄 All Effects 트리와 패턴 인덱스

발탄 이펙트 복원만 다루는 단일 참조 문서다. 두 가지를 정한다.

1. Effect Tool의 All Effects에서 발탄이 **어떻게 보여야 하는가**
2. 패턴과 occurrence를 **어떻게 찾는가**

조사 경위와 [B] 실행 계획은
[`2026-08-15_VALTAN_PATTERN_EFFECT_RESTORATION_SURVEY_AND_PLAN.md`](2026-08-15_VALTAN_PATTERN_EFFECT_RESTORATION_SURVEY_AND_PLAN.md)를 본다.
이 문서는 화면 구조와 인덱스만 담는다.

---

## 1. 캐릭터와 발탄의 축 대응

발탄은 입력 슬롯이 없다. `skillId` 자리에 Server가 확정하는 `patternId`가 들어간다.
그 외 계층은 캐릭터와 1:1이다.

```text
캐릭터   Character > 클래스 > 스킬(inputSlot/skillId) > clip > Element
발탄     Boss      > Valtan > 패턴(patternId)         > stage > Element
```

상단 selector는 Codex가 `Character / Boss` 7-option으로 이미 확장했고 Valtan이 그 안에 있다.

---

## 2. All Effects 트리 구조

```text
[상단 selector]  Character / Boss  ▸  Valtan

Boss ▸ Valtan
│
├─ VALTAN_GHOST_TRANSITION_15   15줄 최종 지면파괴와 망령화      6 stage / 1,609 cue
│   ├─ outer            mesh_abn_groggy_1_loop
│   │   ├─ [Family] SPRITE_ADDITIVE      n
│   │   ├─ [Family] MESH_TRANSLUCENT     n
│   │   └─ [Element] … Solo / Visible / Detail
│   ├─ inner
│   ├─ portal
│   ├─ tracking
│   ├─ four-directions
│   └─ ghost
│
├─ VALTAN_TRIPLE_COUNTER        연속 카운터 내려찍기            7 stage /   889 cue
├─ VALTAN_SUPER_SMASH           초강력 내려찍기 콤보            3 stage /   598 cue
└─ …  총 31 패턴
```

### 2.1 각 계층이 소유하는 것

| 계층 | 표시 | 조작 |
|---|---|---|
| Valtan | 패턴 31개 목록 | — |
| 패턴 | `patternId`, 한글 표시명, stage 수, cue 수 | Play All |
| stage | `semanticStageId`, clip 이름 | Play / Solo |
| Family | 13 authoring family 중 해당 것, element 수 | Family Solo |
| Element | fidelity 배지, 리소스 lane | Visible / Transform / Color / DDS / Save |

캐릭터와 동일한 `Open for Editing` / `Play All` / `Family` / `Solo` 계약을 그대로 쓴다.
발탄 전용 조작을 새로 만들지 않는다.

### 2.2 stage 이름은 패턴마다 다르다

캐릭터는 `BA1~BA4`처럼 균일하지만 발탄 stage는 패턴의 의미를 따른다. 실측된 어휘다.

```text
공격형   windup / active / recovery
지연형   windup / delayed / impact / recovery
이동형   charge / lock / smash / recovery
점프형   takeoff / airborne / land / recovery
카운터   first / first-fail / second / second-fail / third / third-fail / recovery
기믹형   outer / inner / portal / tracking / four-directions / ghost
```

트리에 표시할 stage 순서는 Server 정본인 `ValtanEncounter.json`의 `patterns[].stages[]` 순서를
그대로 쓴다. 이름이나 action ID로 순서를 추정하지 않는다. 현재
`Valtan.actionbindings.json`은 action ID 사전순으로 생성되어 authored stage가 있는 29패턴 중
28패턴의 순서가 encounter와 다르므로, 순서 정본 또는 제품 입력으로 사용하면 안 된다.

---

## 3. 패턴 인덱스 — 31개 전체

`cue`는 원본 notify occurrence 수다. **Effect 문서 수가 아니다.**
`stage`가 0인 두 패턴은 원본 cue는 있으나 authored stage binding이 없다(§5).

| patternId | 표시명 | stage | cue | stage 목록 | 대표 actionId | 대표 clip |
|---|---|---:|---:|---|---|---|
| `VALTAN_GHOST_TRANSITION_15` | 15줄 최종 지면파괴와 망령화 | 6 | 1,609 | outer, inner, portal, tracking, four-directions, ghost | 420616+ | `mesh_abn_groggy_1_loop` |
| `VALTAN_TRIPLE_COUNTER` | 연속 카운터 내려찍기 | 7 | 889 | first, first-fail, second, second-fail, third, third-fail, recovery | 420640+ | `mesh_abn_groggy_1_end` |
| `VALTAN_SUPER_SMASH` | 초강력 내려찍기 콤보 | 3 | 598 | windup, active, recovery | 420619+ | `mesh_att_battle_12_01` |
| `VALTAN_SWING` | 휘두르기 | 3 | 581 | windup, active, recovery | 420601+ | `mesh_att_battle_1_01` |
| `VALTAN_FRONT_BACK_FRONT` | 앞뒤앞 내려찍기 | 3 | 538 | windup, active, recovery | 420637+ | `mesh_att_battle_19_01` |
| `VALTAN_EARTHQUAKE_SMASH` | 지진 찍기 | 4 | 463 | windup, delayed, impact, recovery | 420605+ | `mesh_att_battle_7_01` |
| `VALTAN_DOWN_SMASH` | 내려찍기 | 3 | 443 | windup, active, recovery | 420602+ | `mesh_att_battle_2_01` |
| `VALTAN_DASH_CHARGE` | 대쉬 돌진 | 3 | 412 | windup, active, recovery | 420604 | `mesh_att_battle_4_01` |
| `VALTAN_BIND_CHARGE_SMASH` | 구속 돌진 잡기 후 내려찍기 | 4 | 376 | charge, lock, smash, recovery | 420612+ | `mesh_att_battle_12_01` |
| `VALTAN_CHARGE_GRAB_ROAR` | 돌진 잡기 후 사자후 | 4 | 349 | charge, counter, roar, recovery | 420623+ | `mesh_att_battle_21_01` |
| `VALTAN_JUMP_SPIN` | 점프 찍기 후 휠윈드 | 4 | 342 | jump, land, spin, recovery | 420621+ | `mesh_att_battle_20_01` |
| `VALTAN_CENTER_GRAB_COUNTER_64` | 64줄 중앙 표적 폭발과 잡기 카운터 | 5 | 301 | center, explosion, counter, failed-charge, recovery | 420623+ | `mesh_att_battle_21_01` |
| `VALTAN_BACKSTEP_ATTACK` | 공격하면서 뒤로 빠지기 | 3 | 265 | windup, active, recovery | 420635+ | `mesh_att_battle_20_02` |
| `VALTAN_HIGH_JUMP` | 고공 점프 찍기 | 4 | 242 | takeoff, airborne, land, recovery | 420610 | `mesh_att_battle_8_01_end` |
| `VALTAN_FOUR_PILLARS_105` | 105줄 4기둥 추적 원뿔 | 4 | 242 | takeoff, yellow-zone, target-cone, recovery | 420610 | `mesh_att_battle_8_01_end` |
| `VALTAN_MAGIC_ORB_STAGGER_76` | 76줄 마력구 방어막 무력화 | 3 | 216 | shield, window, recovery | 420617+ | `mesh_abn_groggy_1_end` |
| `VALTAN_STOMP` | 발구르기 | 3 | 168 | windup, active, recovery | 420611 | `mesh_att_battle_11_01` |
| `VALTAN_FLOOR_WIPE_130` | 130줄 2연속 지면 강타 전멸기 | 5 | 161 | windup, first-smash, interval, second-smash, recovery | 420630 | `mesh_att_battle_15_04` |
| `VALTAN_FOUR_SLASH` | 4연속 베기 | 3 | 153 | windup, active, recovery | 420609 | `mesh_att_battle_10_01` |
| `VALTAN_PORTAL_RUSH` | 워프 돌진 콤보 | 4 | 115 | portal, rushes, finish, recovery | 420622 | `mesh_att_battle_18_01` |
| `VALTAN_MAGIC_CHOICE` | 마력기운 양자택일 | 4 | 104 | windup, inner, outer, recovery | 420608 | `mesh_att_battle_5_02_end` |
| `VALTAN_GROUND_WAVE_SMASH` | 지진파 내려찍기 | 3 | 96 | windup, active, recovery | 420615 | `mesh_att_battle_15_01` |
| `VALTAN_ARMOR_BREAK_OPENING` | 오프닝 외벽 충돌 부위 파괴 | 3 | 92 | charge, groggy, recovery | 420627+ | `mesh_dmg_parts_end_1` |
| `VALTAN_PARRY` | 큰 베기 반격 | 3 | 87 | stance, slash, recovery | 420606+ | `mesh_att_battle_9_01_end` |
| `VALTAN_IMPRISON_ROAR` | 감금 사자후 | 3 | 77 | windup, active, recovery | 420603 | `mesh_att_battle_5_01_end` |
| `VALTAN_WHIRLWIND` | 휠윈드 | 3 | 62 | windup, active, recovery | 420633 | `mesh_att_battle_20_02` |
| `VALTAN_ARENA_BREAK_33` | 33줄 두 번째 지형 파괴 | 4 | 42 | spin, landing, cutscene, recovery | 420629 | `mesh_att_battle_12_01` |
| **`VALTAN_ARENA_BREAK_109`** | 109줄 아레나 붕괴 | **0** | 42 | — | 420629 | — |
| **`VALTAN_LEDGE_ROAR`** | 낙사 사자후 | **0** | 39 | — | 420639 | — |
| `VALTAN_FIST_IN_OUT` | 두 손 내려찍기 안밖 폭발 | 4 | 38 | windup, inner, outer, recovery | 420638 | `mesh_att_battle_19_02` |
| `VALTAN_RED_BLADE_WAVE` | 붉은 검기 날리기 | 3 | 33 | windup, active, recovery | 420636 | `mesh_att_battle_12_10` |

현재 generated adapter 합계: **31 패턴 / 108 stage / 9,175 cue**

Server encounter 정본은 **31 패턴 / 117 stage**다. generated adapter의 108은 제품 완료 수가
아니며, missing 9 stage와 stale binding 4개를 포함한 projection gap은 §5를 따른다.

---

## 4. occurrence 찾는 법

세 문서만 알면 어떤 패턴이든 원본까지 역추적된다.

```text
Data/Animation/Authored/Valtan/Valtan.actionbindings.json     ← 시작점
  patterns[].patternId              화면에 보이는 패턴
  patterns[].stages[]               semanticStageId, gameplayActionId, clip
  patterns[].sourceActions[]        sourceActionId, profileId, 원본 한글 이름
    .stages[].animationClips[]      원본 clip
    .stages[].effectCues[]          notifyId, sourceType, localTimeSeconds,
                                    durationSeconds, assetReferences
                                        │
                                        ▼ assetReferences
Data/Effects/Imported/Valtan/Valtan.source-material-evidence.json
  sourceSystems[]  sourceSystemId, actionIds, clipNames, occurrenceCount, graph
  materials[]      materialId, parentMaterialPath, instanceTextures/Scalars/Vectors,
                   parentDeclaration(renderState / collected*Parameters)
  resources[]      resourceId, roles(texture/mesh/material/material_parent),
                   physicalPackage
```

### 4.1 검색 예시

**"휘두르기 이펙트가 어디서 오나"**

```text
1. actionbindings에서 patternId=VALTAN_SWING
2. stages: windup/active/recovery, clip=mesh_att_battle_1_01
3. sourceActionIds: 420601, 420660
4. sourceActions[].stages[].effectCues[] → assetReferences
5. source-material-evidence의 sourceSystems에서 그 asset 조회
   → actionIds에 420601이 있는 system들이 실제 원본
```

**"이 DDS를 쓰는 패턴이 뭔가"**

```text
1. source-material-evidence의 resources[]에서 resourceId 조회
2. 그 asset을 참조하는 sourceSystems 확인
3. sourceSystems[].actionIds → actionbindings에서 그 actionId를 가진 패턴 역조회
```

### 4.2 stable identity

| 축 | 키 | 안정성 |
|---|---|---|
| 패턴 | `patternId` | Server encounter 문서 소유 |
| stage | `gameplayActionId` (`valtan.attack.swing.windup`) | authored 바인딩 소유 |
| 원본 action | 숫자 `actionId` (420601) | UPK 소유 |
| 원본 system | `sourceSystemId` (`FX_BS_00.hit.par_d_groggyhit_01`) | UPK 소유 |
| material | `materialId` = sourceMaterialPath | UPK 소유 |
| resource | `resourceId` = sourceAssetPath | UPK 소유 |

runtime mapping key는 `BOSS_VALTAN + patternId + actionId`다. 이 계약은 바꾸지 않는다.

---

## 5. 현재 projection gap

원본 cue는 있으나 authored animation stage binding이 없다. 트리에서 **패턴은 보이되 stage가 없는
상태**로 표시하고, 이름이나 순서로 stage/clip을 만들지 않는다.

| patternId | 표시명 | cue | 필요한 것 |
|---|---|---:|---|
| `VALTAN_ARENA_BREAK_109` | 109줄 아레나 붕괴 | 42 | 6개 `valtan.mechanic.arena-break-109.*` stage 바인딩 |
| `VALTAN_LEDGE_ROAR` | 낙사 사자후 | 39 | 3개 `valtan.attack.ledge-roar.*` stage 바인딩 |

또한 `Valtan.patternbindings.json`에는 encounter에 없는 `valtan.mechanic.arena-break-80.*`
stale row 4개가 있다. 실제 근거로 missing 9개를 추가하고 stale 4개를 제거한 뒤에도,
`build_valtan_action_bindings.py`가 encounter stage 순서와 stage-local source offset을 보존하도록
교정되기 전에는 재실행 결과를 제품 입력으로 승격하지 않는다.

---

## 6. 규모 참고

```text
패턴                31        stage 108        cue 9,175
sourceSystem       193        material 335     parent family 123
resource           855        texture 346 / mesh 52 / material 335 / material_parent 123
물리 누락            0
```

`cue 9,175`는 notify occurrence 수다. 같은 sourceSystem/material/resource가 여러 action·stage에서
반복되므로 **Effect 문서는 이보다 훨씬 적다.** definition 축은 material 335 / system 193이고
family 축은 123이다.

비교: 4직업 전체가 101문서 / 4,777 element였다.

---

## 7. 트리 구현 시 지킬 것

- 상단 selector의 `Boss > Valtan`은 UI 전용이다. `CHARACTER_CLASS` enum을 확장하지 않는다.
- stage 순서는 `ValtanEncounter.json`의 패턴별 stage 선언 순서를 따른다. 현재 generated
  `Valtan.actionbindings.json` 선언 순서를 제품 정본으로 사용하지 않는다.
- `unmappedSourceActionIds` 89개(피격·그로기 반응 등)는 제품 트리에 노출하지 않는다.
- fidelity 배지와 admission 판정은 4직업 G1/G3 계약을 그대로 상속한다. 발탄 전용 완화 규칙을 만들지 않는다.
- 화면 판정은 사용자 전용이다.
