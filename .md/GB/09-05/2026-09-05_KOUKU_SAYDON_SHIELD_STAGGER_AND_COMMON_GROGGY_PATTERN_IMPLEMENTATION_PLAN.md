# 2026-09-05 KoukuSaydon 1관문 무력화 반사 패턴과 공용 그로기 Pattern 구현 계획서

> 문서 종류: 구현 계획서 (범위와 변경 단위 합의용, 전체 코드 전문은 디테일 계획서에서 확장)
>
> 상태: 실측 완료 / 구현 전
>
> 기준 브랜치: `codex/kouku-workbench-iteration` (다른 세션의 미커밋 변경이 많으므로 이 계획은 아래 파일만 건드린다)
>
> 선행 조사: [1~3관문 패턴 애니메이션 전수 조사](2026-09-05_KOUKU_SAYDON_GATE1_3_PATTERN_ANIMATION_SURVEY.md) §2

## 0. 목표와 종료 증거

목표는 1관문 `130줄 반사(응집된 에너지)` 패턴을 Boss Tool에서 `Play Selected`로 재생했을 때 Server가
`중앙이동 → 시전 → 구체 유지(hold) → [시간 초과: 전멸 발산] | [누적 damage 달성: 구체 파괴 → 공용 그로기]`를
하나의 audition occurrence 체인으로 실행하고, Client가 각 stage clip을 그대로 따라 재생하는 것이다.

종료 증거는 다음 넷이다.

1. `python -B Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode validate`와
   `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`이 새 Pattern 3개를 `PATTERNSTAGERESPONSE`,
   `PATTERNSTAGEBRANCH`, `PATTERNSTAGEFOLLOWUP` 행으로 bootstrap에 싣는다.
2. `Server.exe --contract-test`의 Kouku brain 검사가 threshold 분기, timeout 분기, follow-up 체인, GROGGY stage를 통과한다.
3. Debug Product 빌드 통과.
4. 사용자가 실제 Server+Client에서 Boss Tool `Play Selected` → hold 중 타격 → 그로기 체인, 그리고 무타격 → 전멸 clip을 눈으로 확인한다.
   이 확인 전에는 visual PASS를 기록하지 않는다.

## 1. 현재 실측

### 1.1 원본에는 지속 시간이 없다

- `MN_RPCZ_00.loa` 추출본은 stage별 clip 길이와 notify만 가진다. `Att_Battle_14_02`(0.667s)에 붙은 particle
  `Par_U_RPCZ_EnergyBalloon_01`의 duration 2.6s와 Paralyzation marker(0.2s, 2.4s)가 “여기서 pose를 잡고 기다린다”는
  흔적일 뿐, 실제 유지 시간·요구 damage·전멸 판정은 Skill/SkillEffect XML이 추출 범위에 없어 남아 있지 않다.
- 따라서 발탄 도끼 패턴처럼 `durationMs`를 Server stage clock으로 직접 저작해야 한다. 이 값은 clip 길이와 독립이다.

### 1.2 순서는 사용자 이해와 같다

```text
42197102 중앙이동(idle_battle_1, 연기+HidePawn)
→ 42197100 stage0 att_battle_14_01 (시전 1467ms)
→ 42197100 stage1 att_battle_14_02 (구체 유지, clip 667ms + 저작 hold)
   ├─ 시간 초과 → 42197100 stage2 att_battle_14_03 (전멸 발산 3167ms, Cohesion_Exp_02 @1.9s)
   └─ damage 달성 → 42197101 att_battle_14_04 (구체 파괴 2000ms, Cohesion_Exp_01 @0.48s)
                     → 4219763 그로기 dmg_critical_start(1000) → loop(1333) → end(1167)
```

### 1.3 이미 있는 Server 계약 (재사용)

| 계약 | 위치 | 상태 |
|---|---|---|
| ACTIVE stage의 누적 damage 임계치 | `BOSS_PATTERN_STAGE_DEFINITION::eBossResponseKind/iBossResponseThreshold` (`Server/Public/GameplayCatalog.h:584`) | Valtan `VALTAN_STAGGER_SLOT`이 사용 |
| 임계치 도달 outcome 발행 | `CBossCombatRuntime::Apply_PlayerHit` → `Publish_PatternOutcome(HEALTH_DAMAGE_THRESHOLD_REACHED)` (`Server/Private/BossCombatRuntime.cpp:184`) | boss 종류 무관, player hit 경로 `ServerCombatHitRuntime.cpp:130`이 Kouku boss에도 적용 |
| stage 분기 | `BOSS_PATTERN_STAGE_BRANCH{eOutcome, strNextActionId | strNextPatternId}` | bootstrap 행 `PATTERNSTAGEBRANCH`(local) / `PATTERNSTAGEFOLLOWUP`(cross-pattern) |
| follow-up 예약 | `SERVER_WORLD_ENTITY::PendingPatternFollowup` (`Server/Public/ServerWorldEntity.h:118`) | Valtan `ApplyStageBranch`가 사용 |
| GROGGY stage kind | `BOSS_PATTERN_STAGE_KIND::GROGGY` → wire `PATTERN_ACTIVE` | 촉발 검증: `HEALTH_DAMAGE_THRESHOLD_REACHED` follow-up 대상의 첫 stage는 GROGGY여야 함 (`GameplayCatalog.cpp:4710`) |
| 전멸 hit | stage `hitShape CIRCLE 100m + damage.valtan.omnidirectional-wipe-130(100000%)` | Valtan `FINAL_ATTACK`/`WIPE` stage 예시 |
| HUD 게이지 | `SNAPSHOT.BossCombat.iResponseThreshold/iResponseProgress` → `CCombatHUDViewModel` | Kouku boss도 `Apply_Boss`로 이미 흘러감 |

### 1.4 Kouku 경로가 아직 막아 둔 것

| 위치 | 현재 제한 | 이번 변경 |
|---|---|---|
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | `STAGE_KEYS` exact, `STAGE_KINDS={WINDUP,ACTIVE,RECOVERY}`, PRODUCT는 `playMs == durationMs`+EXACT, 모든 hit/branch 0 투영 | G01 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` Kouku block (2998~3213행) | stage exact property 18개, stageKind 3종, 암묵 `TIMEOUT → 다음 stage` 행만 생성 | G01 |
| `Server/Private/KoukuSaydonBrain.cpp` | `Validate_AnimationOnlyPattern`이 GROGGY/bossResponse/비-TIMEOUT branch/follow-up 거부, `Update`는 `durationMs` 경과만 본다 | G02 |
| `Server/Private/GameRoom.cpp::Update_KoukuSaydonBoss` (5192행) | `PATTERN_COMPLETED`면 바로 Play All 다음 index 또는 COMPLETED | G02 |
| `Client/Private/KoukuSaydonPatternAuditionService.cpp:330` | PLAY_SELECTED 중 ACTIVE lifecycle의 pattern id가 요청 id와 다르면 ABORTED | G03 |
| `Client/Private/KoukuSaydonPresentationAssetService.cpp:214` | binding은 `endPolicy == "EXACT"`, `startOffsetMs == 0`, action당 1개만 admission | G03 (LOOP_TO_WINDOW 허용) |
| `Client/Private/ClientReplication.cpp:3411` | `Play_NetworkAction(clip, false, rate, 0.05f)` 고정 non-loop | G03 |
| `Client/Public/KoukuSaydonCompositionDocument.h` / `.cpp` | stage 구조체에 gameplay 필드 없음, `Is_StageKind` 3종 | G03 |
| `Client/Private/KoukuSaydonActionWorkbench.cpp` | `STAGE_KINDS` 3종, Details에 Duration/Kind만 있음, gap·threshold·branch UI 없음 | G03 |

### 1.5 “stage 빈 시간”이 현재 어떻게 동작하는가

- Engine `CAnimation::Update_TransformationMatrix`(`Engine/Private/Animation.cpp:80`)는 non-loop clip이 끝나면
  track position을 `m_fDuration`에 고정한다. 즉 Client는 별도 코드 없이 **마지막 pose를 유지**한다.
- Server는 `boss.iPatternStageDurationMs`가 지날 때까지 stage를 바꾸지 않는다(`KoukuSaydonBrain.cpp::Update`).
- 그러므로 `durationMs > playMs`인 stage는 “clip 재생 → 마지막 pose hold → Server clock 만료 시 다음 stage”가 되며,
  발탄 Workbench의 `Selected Stage Gap (ms)`(`ValtanActionWorkbench.cpp:9726`)이 하는 일과 같다.
  발탄은 gap > 0이면 `HOLD_LAST_POSE`로 표기하지만 Kouku 제품 runtime은 EXACT만 받으므로 Kouku는 EXACT를 유지하고
  `durationMs`만 늘린다.
- 지금은 projector가 PRODUCT에 `playMs == durationMs`를 강제해 이 gap을 저장할 수 없다. G01이 이 규칙을
  `playMs <= durationMs`로 완화하는 것이 hold 저작의 전제다.

## 2. 변경할 파일

새 C++ 파일은 없다. `.vcxproj`/`.filters` 등록 변경도 없다.

| G | 파일 | 역할 |
|---|---|---|
| G01 | `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | Pattern 3개 저작 (`SHIELD_STAGGER`, `SHIELD_BREAK`, `COMMON_GROGGY`) |
| G01 | `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | optional stage gameplay key, GROGGY, hold 완화, role 기반 playAll, 투영 |
| G01 | `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` | 규칙 변경 반영 |
| G01 | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | Kouku block에 response/branch/follow-up 행 emit |
| G02 | `Server/Public/KoukuSaydonBrain.h`, `Server/Private/KoukuSaydonBrain.cpp` | 분기·threshold·follow-up 평가 |
| G02 | `Server/Private/GameRoom.cpp` (`Update_KoukuSaydonBoss`, `Clear_KoukuSaydonPatternAudition`) | follow-up 체인을 같은 audition으로 이어 재생 |
| G02 | `Server/Private/ServerGameplayContractTests.cpp` | Kouku brain 검사 확장 |
| G03 | `Client/Public/KoukuSaydonCompositionDocument.h`, `Client/Private/KoukuSaydonCompositionDocument.cpp` | stage gameplay 필드 parse/serialize/validate |
| G03 | `Client/Private/KoukuSaydonActionWorkbench.cpp` | GROGGY kind, Stage Gap, Boss Response, Branch 편집 |
| G03 | `Client/Private/KoukuSaydonPresentationAssetService.cpp`, `Client/Public/KoukuSaydonPresentationAssetService.h`, `Client/Private/ClientReplication.cpp` | `LOOP_TO_WINDOW` loop 재생 |
| G03 | `Client/Private/KoukuSaydonBossTool.cpp`, `Client/Private/KoukuSaydonPatternAuditionService.cpp`, `Client/Public/KoukuSaydonPatternAuditionService.h` | follow-up pattern id를 같은 audition으로 인정 |
| G04 (선택) | `Server/Private/KoukuSaydonBrain.cpp`, `Server/Private/ValtanBrain.cpp`, 공용 hit helper | 전멸 stage에 실제 lethal hit 적용 |
| G05 (선택) | `Server/Private/KoukuSaydonBrain.cpp`, `GameRoom.cpp` | `RETURN_TO_ARENA_CENTER` stage action으로 실제 중앙 이동 |

## 3. 데이터와 호출 흐름

### 3.1 저작 JSON (G01에서 추가하는 stage optional key)

stage에는 다음 optional key를 허용한다. 없으면 오늘과 같은 animation-only stage다.

```json
"bossResponse": { "kind": "ACCUMULATED_HEALTH_DAMAGE", "threshold": 50000 },
"branches": [
  { "outcome": "HEALTH_DAMAGE_THRESHOLD_REACHED", "nextPatternId": "KAKULSAYDON_G1_SHIELD_BREAK" },
  { "outcome": "TIMEOUT", "nextActionId": "kakulsaydon.g1.shield-stagger.wipe" }
]
```

pattern에는 `"role": "ROOT" | "FOLLOWUP"`를 추가한다. `playAllPatternIds`는 PRODUCT이면서 ROOT인 pattern만 authored 순서로 담는다.
FOLLOWUP은 Boss Tool에서 단독 `Play Selected`가 가능하지만 Play All 순서에는 들어가지 않는다.

### 3.2 새 Pattern 3개 (저작 값)

`durationMs`, `threshold`는 Server 저작값이며 사용자가 튜닝한다. 아래는 첫 값이다.

| pattern | stage | kind | durationMs | clip(playMs) | gameplay |
|---|---|---|---:|---|---|
| `KAKULSAYDON_G1_SHIELD_STAGGER` (ROOT) | `CENTER_MOVE` | WINDUP | 2000 | `rpcz00_idle_battle_1`(2000) | G05 전까지 표현만 (실제 이동 없음) |
| | `CAST` | WINDUP | 1467 | `rpcz00_att_battle_14_01`(1467) | |
| | `SHIELD_HOLD` | ACTIVE | **12000** | `rpcz00_att_battle_14_02`(667) → 11333ms hold | `bossResponse 50000`, branch `THRESHOLD → SHIELD_BREAK`, `TIMEOUT → WIPE` |
| | `WIPE` | ACTIVE | 3167 | `rpcz00_att_battle_14_03`(3167) | G04에서 hit CIRCLE 100m + `damage.valtan.omnidirectional-wipe-130` delay 1900 |
| `KAKULSAYDON_G1_SHIELD_BREAK` (FOLLOWUP) | `BREAK` | **GROGGY** | 2000 | `rpcz00_att_battle_14_04`(2000) | branch `TIMEOUT → nextPatternId KAKULSAYDON_COMMON_GROGGY` |
| `KAKULSAYDON_COMMON_GROGGY` (FOLLOWUP) | `GROGGY_START` | GROGGY | 1000 | `rpcz00_dmg_critical_start_1`(1000) | |
| | `GROGGY_LOOP` | GROGGY | **6000** | `rpcz00_dmg_critical_loop_1`(1333, `LOOP_TO_WINDOW`) | G03 loop 재생 전에는 EXACT + hold |
| | `GROGGY_END` | RECOVERY | 1167 | `rpcz00_dmg_critical_end_1`(1167) | |

`SHIELD_BREAK`의 첫 stage를 GROGGY로 두는 이유는 `GameplayCatalog.cpp:4710`의 규칙(임계치 follow-up 대상은 첫 stage가 GROGGY)
때문이다. 성공 폭발 clip을 GROGGY kind로 재생해도 wire는 `PATTERN_ACTIVE`이므로 Client 표현은 같다.

각 occurrence는 `startOffsetMs 0`, `sourceStartMs 0`, `playRate 1.0`, `endPolicy EXACT`(loop stage만 `LOOP_TO_WINDOW`)이다.
`sourceActionId/sourceStageId/sourceSlotId/referenceRevision`은 Workbench Resource 트리에서 `Append as Stage`로 만든 값을
그대로 쓴다(`referenceRevision 8d2e0188…`). 손으로 JSON을 고치면 `revision`을 1 올려야 Workbench CAS가 새 baseline으로 읽는다.

### 3.3 publish 행 (G01 publisher가 새로 emit)

```text
PATTERNSTAGE          ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_STAGGER  2  SHIELD_HOLD  kakulsaydon.g1.shield-stagger.shield-hold  ACTIVE  12000  NONE 0 0 0 0 0 0 0 0 - 0 0 0 0
PATTERNSTAGERESPONSE  ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_STAGGER  kakulsaydon.g1.shield-stagger.shield-hold  ACCUMULATED_HEALTH_DAMAGE  50000
PATTERNSTAGEFOLLOWUP  ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_STAGGER  kakulsaydon.g1.shield-stagger.shield-hold  HEALTH_DAMAGE_THRESHOLD_REACHED  KAKULSAYDON_G1_SHIELD_BREAK
PATTERNSTAGEBRANCH    ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_STAGGER  kakulsaydon.g1.shield-stagger.shield-hold  TIMEOUT  kakulsaydon.g1.shield-stagger.wipe
PATTERNSTAGEFOLLOWUP  ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_BREAK    kakulsaydon.g1.shield-break.break          TIMEOUT  KAKULSAYDON_COMMON_GROGGY
PATTERNSEQUENCESTEP   ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_PLAY_ALL  0  KAKULSAYDON_G1_PIZZA            100
PATTERNSEQUENCESTEP   ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_PLAY_ALL  1  KAKULSAYDON_G1_SHIELD_STAGGER   0
```

branch가 저작되지 않은 stage는 지금처럼 암묵 `TIMEOUT → 다음 stage`(마지막 stage는 `-`) 행을 유지한다.
`PATTERNSTAGERESPONSE`/`FOLLOWUP`/`BRANCH` 행 파서는 `GameplayCatalog.cpp:2968/3550/3593`에 이미 있다.

### 3.4 Server 실행 흐름 (G02)

```text
Boss Tool Play Selected(KAKULSAYDON_G1_SHIELD_STAGGER)
→ CGameRoom::Evaluate_KoukuSaydonPatternAudition  (기존)
→ Update_KoukuSaydonBoss: PENDING → Begin_Pattern            (기존)
→ CKoukuSaydonBrain::Update 매 tick
   ├─ Enter_Stage(SHIELD_HOLD)에서 boss.ePatternBossResponseKind/iPatternBossResponseThreshold 설정
   ├─ player skill hit → ServerCombatHitRuntime → CBossCombatRuntime::Apply_PlayerHit → outcome 발행 (기존 generic)
   ├─ Update: stage.Branches 중 non-TIMEOUT outcome을 CBossCombatRuntime::Consume_PatternOutcome로 소비
   │    └─ nextPatternId → boss.PendingPatternFollowup 예약 + Finish_Pattern(COMPLETED) → FOLLOWUP_QUEUED
   └─ durationMs 경과: TIMEOUT branch → nextActionId(Enter_Stage) | nextPatternId(예약) | 없음(완료)
→ Update_KoukuSaydonBoss: FOLLOWUP_QUEUED/PATTERN_COMPLETED + PendingPatternFollowup
   → lifecycle PATTERN_COMPLETED(완료 pattern) 큐
   → 다음 tick Begin_Pattern(follow-up, pinned catalog) → lifecycle ACTIVE(follow-up pattern id)
   → 체인이 끝나고 PendingPatternFollowup이 비면 기존 Play All index 진행 또는 COMPLETED
```

`m_KoukuSaydonPatternAudition.iPatternIndex`는 root pattern slot을 그대로 가리키고, follow-up은 같은 slot 안의 연속 occurrence로 취급한다.

### 3.5 Client 흐름 (G03)

- 표현: `ClientReplication.cpp:3411`은 stage `actionId`로 binding 하나를 찾아 clip을 재생한다. follow-up pattern의 stage도
  같은 경로다(actionId가 전역 유일). 변경은 `LOOP_TO_WINDOW`면 `isLoop=true`로 재생하는 것뿐이다.
- Boss Tool: Product의 `branches[].nextPatternId`를 모아 요청 pattern의 follow-up 집합을 audition service에 넘긴다.
  service는 PLAY_SELECTED 중 ACTIVE lifecycle의 pattern id가 요청 id이거나 그 follow-up 집합에 속하면 `[Live]`로 인정한다.
- Workbench: Details에 `Stage Gap (ms)`, `Boss Response threshold`, `Branch` 편집을 추가한다. Save는 기존 단일 파일 CAS 그대로다.

## 4. G별 구현 범위

### G01 — 데이터 계약: composition → Product → bootstrap

**projector `project_kouku_saydon_composition.py`**

- `STAGE_KEYS`를 필수 5개 + optional `bossResponse`, `branches`로 나눈다. `_exact_keys` 대신 필수 포함·optional 허용 검사를 쓴다.
- `PATTERN_KEYS`에 `role` 추가(`ROOT`/`FOLLOWUP`). 기존 문서에 `role`이 없으면 fail-closed가 아니라 `ROOT`로 읽고 Save 시 기록한다.
- `STAGE_KINDS`에 `GROGGY` 추가. `END_POLICIES`는 그대로.
- PRODUCT 규칙 완화: `startOffsetMs == 0`, `sourceStartMs == 0`, `playMs <= durationMs`, `endPolicy in {EXACT, LOOP_TO_WINDOW}`.
  (`test_product_requires_one_full_stage_animation`의 “whole stage” 케이스와
  `test_product_rejects_presentation_policies_the_client_cannot_run`의 `hold` 케이스를 `playMs > durationMs`, `HOLD_LAST_POSE` 거부로 바꾼다.)
- stage 검증 추가: `bossResponse`는 ACTIVE stage에만, `threshold ≥ 1`, pattern당 1개; `branches`는 1~8개, outcome ∈
  `{TIMEOUT, HEALTH_DAMAGE_THRESHOLD_REACHED}`, outcome 중복 금지, `nextActionId`는 같은 pattern의 다른 stage,
  `nextPatternId`는 같은 문서의 PRODUCT pattern, 둘 중 하나만; `HEALTH_DAMAGE_THRESHOLD_REACHED`는 `bossResponse` 있는 stage에서
  `nextPatternId`만 허용하고 대상 pattern의 첫 stage kind가 `GROGGY`여야 한다(Server 규칙 선반영).
- `_project_stage`: `bossResponse`, `branches`가 있으면 그대로 투영한다. hit 필드는 계속 0.
- `project_encounter`: `playAllPatternIds` = PRODUCT ∧ ROOT. `test_product_projection_is_minimal_and_timeout_only`는
  “branch가 없는 stage에는 branches 키가 없다”로 좁힌다.

**publisher `Publish-GameplayBalance.ps1` Kouku block**

- stage `Assert-ExactProperties` 목록을 기본 18개 + 존재 시 `bossResponse`, `branches`로 만든다(Valtan block 1995~2027행 방식).
- `stageKind`에 `GROGGY` 허용.
- `bossResponse`가 있으면 `PATTERNSTAGERESPONSE` 행 emit(kind 고정 검사, ACTIVE만).
- `branches`가 있으면 authored 행을 emit: `nextActionId` → `PATTERNSTAGEBRANCH`, `nextPatternId` → `PATTERNSTAGEFOLLOWUP`.
  없으면 기존 암묵 `TIMEOUT → 다음 stage` 행 유지. TIMEOUT이 정확히 하나인지 검사한다.
- `playAllPatternIds` 검사를 “PRODUCT 전체 순서”에서 “ROOT PRODUCT 순서”로 바꾼다. Product JSON에 `role`이 투영되지 않으므로
  publisher는 `branches[].nextPatternId`에 등장하는 pattern을 FOLLOWUP으로 판정한다.

**composition JSON**: §3.2의 세 pattern을 PRODUCT로 추가하고 `revision`을 올린다.

검증: `python -B Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode publish --repository-root .`,
`python -m unittest Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py`(PYTHONPATH=.),
`Tools/GameplayPipeline/Publish-GameplayBalance.ps1`, 생성 bootstrap에서 위 §3.3 행 grep,
`Server.exe --contract-test`(기존 검사 회귀; G02 전이므로 새 pattern은 `Validate_AnimationOnlyPattern`에서 unsupported로 거부되는 것이 정상).

### G02 — Server: `CKoukuSaydonBrain` 분기·임계치·follow-up

**`KoukuSaydonBrain.h`**

- `KOUKUSAYDON_BRAIN_UPDATE_RESULT`에 `FOLLOWUP_QUEUED` 추가(위치: `PATTERN_COMPLETED` 바로 아래).
- `Validate_AnimationOnlyPattern` → 이름 유지, 허용 범위만 확장: stage kind에 GROGGY, `eBossResponseKind ACCUMULATED_HEALTH_DAMAGE`(ACTIVE만),
  branch outcome `TIMEOUT`(nextActionId=같은 pattern stage | nextPatternId | 빈 값) 및 `HEALTH_DAMAGE_THRESHOLD_REACHED`(nextPatternId만).
  hit/motion/actions/counter proxy는 계속 거부(G04/G05 전까지).
- private static 추가: `Apply_Branch(boss, pattern, branch, serverTick)`, `Set_BossResponseState(boss, kind, threshold)`,
  `Find_StageByActionId(pattern, actionId, outIndex)`.

**`KoukuSaydonBrain.cpp`**

- `To_ServerAction`: `GROGGY → PATTERN_ACTIVE`.
- `Enter_Stage`: `Set_BossResponseState(stage.eBossResponseKind, stage.iBossResponseThreshold)`(누적 0, published false, 변경 시
  `boss.BossCombat.iStateRevision` 증가 — `ValtanBrain.cpp:62`의 file-static helper와 같은 책임을 Kouku 파일 안에 둔다),
  `boss.bPatternGroggy = (GROGGY == kind)`.
- `Update`: durationMs 검사 전에 `stage.Branches`를 순회해 `TIMEOUT`이 아닌 branch를
  `CBossCombatRuntime::Consume_PatternOutcome(boss, stage.strActionId, eOutcome)`으로 소비 → `Apply_Branch`.
  durationMs 경과 시 TIMEOUT branch를 찾아 `Apply_Branch`; TIMEOUT branch가 없으면 지금처럼 다음 stage/완료.
- `Apply_Branch`: `nextActionId` → `Enter_Stage(target, index, tick, false)` → `STAGE_CHANGED`;
  `nextPatternId` → `boss.PendingPatternFollowup{patternId, PinnedDefinitionRevision, iSourcePatternSequence, iRootPatternSequence, depth+1}` 설정 후
  `Finish_Pattern(COMPLETED)` → `FOLLOWUP_QUEUED`; 둘 다 비면 `Finish_Pattern(COMPLETED)` → `PATTERN_COMPLETED`.
  depth는 `MAX_PATTERN_FOLLOWUP_DEPTH`를 넘기면 ABORTED.
- `Finish_Pattern`: `Set_BossResponseState(NONE, 0)`, `CBossCombatRuntime::Clear_PatternOutcomes(boss)`, `bPatternGroggy=false`.
  receipt의 `iRootPatternSequence`는 follow-up이면 root sequence를 유지한다.
- `Abort_Pattern`: `boss.PendingPatternFollowup = {}`도 비운다.

**`GameRoom.cpp::Update_KoukuSaydonBoss`**

- brain 결과가 `FOLLOWUP_QUEUED`이거나 `PATTERN_COMPLETED`인데 `boss.PendingPatternFollowup.Is_Pending()`이면:
  lifecycle `PATTERN_COMPLETED`(완료 pattern) 큐 → `Find_AnimationOnlyPattern(*pinnedCatalog, followup.strPatternId)` →
  `Begin_Pattern`(같은 `PinnedGameplayRevision`) → `boss.PendingPatternFollowup = {}` → lifecycle `ACTIVE(follow-up id)` → return.
  begin 실패는 ABORTED + `Clear_KoukuSaydonPatternAudition`.
- `Clear_KoukuSaydonPatternAudition`/ABORTED 경로에서 `boss.PendingPatternFollowup = {}`.
- `Evaluate_KoukuSaydonPatternAudition`은 변경 없음(follow-up pattern도 PLAY_SELECTED 가능).

**contract test**: `ServerGameplayContractTests.cpp:885` 블록 옆에 추가 — (a) SHIELD_STAGGER를 시작하고 `SHIELD_HOLD`에서
`BOSS_INCOMING_HIT{iRawDamage=threshold}`를 `CBossCombatRuntime::Apply_PlayerHit`로 넣으면 다음 `Update`가 `FOLLOWUP_QUEUED`이고
`PendingPatternFollowup.strPatternId == "KAKULSAYDON_G1_SHIELD_BREAK"`; (b) 타격 없이 clock을 돌리면 `WIPE` stage로 `STAGE_CHANGED` 후 `PATTERN_COMPLETED`;
(c) SHIELD_BREAK → COMMON_GROGGY follow-up 예약; (d) COMMON_GROGGY의 GROGGY stage가 `PATTERN_ACTIVE`로 복제되고 `bPatternGroggy`가 true.

검증: Debug Product 빌드(Server), `Server.exe --contract-test` failures 0, `Framework.slnLaunch`로 Server 실행 후 Client Boss Tool
`Play Selected` → HUD threshold 게이지 표시 확인(사용자).

### G03 — Client: 저작 UI·표현·Boss Tool follow-up 인정

**`KoukuSaydonCompositionDocument.h/.cpp`**

- `KOUKU_SAYDON_COMPOSITION_STAGE`에 `std::optional<KOUKU_SAYDON_STAGE_BOSS_RESPONSE> BossResponse`,
  `std::vector<KOUKU_SAYDON_STAGE_BRANCH> Branches` 추가(`AnimationOccurrences` 바로 아래). pattern에 `std::string strRole`.
- `Parse_Text`: stage exact property 검사를 필수 5개 + optional 2개로 바꾸고 파싱; pattern `role` optional(기본 ROOT).
- `Serialize`: 값이 있을 때만 `bossResponse`, `branches`를 `animationOccurrences` 앞에 쓴다. `role`은 항상 쓴다.
- `Validate_Shape`: `Is_StageKind`에 GROGGY; projector와 같은 branch/bossResponse 규칙.

**`KoukuSaydonActionWorkbench.cpp`**

- `STAGE_KINDS`에 `"GROGGY"`.
- Details `Stage` 섹션(2636행 이후)에 `Selected Stage Gap (ms)` 추가: gap = `durationMs − (box startOffset + playMs)`;
  편집 시 `Set_StageDuration(box end + gap)`. 발탄과 달리 endPolicy는 바꾸지 않는다.
- `Boss Response` 체크박스 + threshold InputInt(ACTIVE stage에서만 활성) → `Set_StageBossResponse`(새 함수, Commit_Candidate 경유).
- `Branches` 목록: outcome combo(TIMEOUT / HEALTH_DAMAGE_THRESHOLD_REACHED), 대상 combo(같은 pattern의 stage 또는 PRODUCT pattern) → `Set_StageBranch`/`Remove_StageBranch`.
- Pattern 섹션에 `Role` combo(ROOT/FOLLOWUP).
- 타임라인 lane은 이미 `durationMs` 폭으로 그리므로 gap은 box 뒤 빈 lane으로 보인다.

**표현**: `KOUKU_SAYDON_ACTION_PRESENTATION`에 `bool_t bLoop`; admission에서 `endPolicy ∈ {EXACT, LOOP_TO_WINDOW}`;
`ClientReplication.cpp:3411`은 `action.bLoop`를 `Play_NetworkAction` 두 번째 인자로 넘긴다.

**Boss Tool / audition service**: `PRODUCT_PATTERN`에 `std::vector<std::string> FollowupPatternIds`(Product `branches[].nextPatternId` 수집);
`CKoukuSaydonPatternAuditionService::Play_Selected`에 follow-up 집합을 함께 전달하고 ACTIVE/PATTERN_COMPLETED lifecycle에서 그 집합의 id를
같은 요청의 연속 occurrence로 인정한다. `[Live]` 표시는 lifecycle이 준 pattern id 기준이다.

검증: Debug Product 빌드(Client), Workbench에서 Reload → Details 값 확인 → gap/threshold/branch 편집 → Save → 파일 diff가 §3.2와 같음 →
`Publish All PRODUCT` → Boss Tool Reload에 세 pattern 표시 → 사용자 실제 재생.

### G04 (선택) — 전멸 stage에 lethal hit

`ValtanBrain.cpp:2479`의 file-static `ApplyPatternHit`가 유일한 stage hit 적용자다. Kouku 전멸에 실제 damage를 넣으려면
이 함수를 `Server/Private/BossPatternHitRuntime.cpp`(새 파일, vcxproj 등록 필요) 같은 공용 helper로 옮겨 두 brain이 공유해야 한다.
그 뒤 Kouku `WIPE` stage에 `hitShape CIRCLE`, `hitOuterRadius 100`, `serverDamageProfileId damage.valtan.omnidirectional-wipe-130`, `hitCount 1`,
`hitDelayMs 1900`을 저작하고 projector/publisher/brain validate의 hit 거부를 해제한다. 두 번째 hit runtime을 Kouku 파일 안에 복제하지 않는다.

### G05 (선택) — 실제 중앙 이동

`BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER`를 `CENTER_MOVE` stage ENTER action으로 저작하고 Kouku brain이 `Actions` 중 이 kind만 허용,
`GameRoom::Apply_BossPatternStageActions`(10036행)가 Kouku boss에도 호출되도록 연결한다. 원본의 HidePawn/연기 despawn 표현은 Effect 저작 별도다.

## 5. 검증 요약

| 단계 | 명령 / 절차 | 기대 |
|---|---|---|
| G01 | projector validate/publish, unittest, `Publish-GameplayBalance.ps1` | Product 3 pattern, bootstrap에 RESPONSE/BRANCH/FOLLOWUP 행, playAll = [PIZZA, SHIELD_STAGGER] |
| G02 | Debug 빌드, `Server.exe --contract-test` | Kouku 검사 4건 통과, 기존 Pizza 검사 유지 |
| G03 | Debug 빌드, Workbench 편집·Save·Reload | 저장 JSON이 §3.2, Boss Tool에 follow-up 포함 3 pattern |
| runtime | 사용자: Server+Client, Boss Tool `Play Selected KAKULSAYDON_G1_SHIELD_STAGGER` | hold 중 HUD 게이지, 타격 시 BREAK→GROGGY, 무타격 시 WIPE clip |
| 공통 | `git diff --check` | 경고 없음 |

에이전트는 빌드·contract test·publish까지 실행하고, Client 실행과 화면 판정은 사용자가 한다.

## 6. 미결 사항 (구현 전 사용자 결정)

1. `SHIELD_HOLD` 12000ms, threshold 50000(최대 HP 1,000,000의 5%)의 첫 값.
2. `CENTER_MOVE`를 G05 전까지 idle 2000ms 표현으로 둘지, 아예 stage에서 빼고 `CAST`부터 시작할지.
3. `COMMON_GROGGY` loop 길이 6000ms와, G03 loop 재생 전에 EXACT hold(정지 pose)로 먼저 저장할지.
4. G04 전멸 lethal hit을 이번 변경 단위에 넣을지(공용 hit helper 추출이 필요해 발탄 파일도 건드린다).
