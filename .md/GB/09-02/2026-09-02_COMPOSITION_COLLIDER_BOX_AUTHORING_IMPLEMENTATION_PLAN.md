# 2026-09-02 Composition Workbench 콜라이더 박스 저작 구현 계획서

이 문서는 `Action Composition Workbench`의 Collider lane에서 **박스를 만들고, Box Detail에서
type / damage 수치 / 사이즈 / lifetime을 조절해 저장**하는 하나의 수직 슬라이스만 소유한다.

같은 날짜의 `2026-09-02_VALTAN_PATTERN_COLLIDER_AND_COMPOSITION_BOTTLENECK_PLAN.md`는 발탄 패턴
22개 항목 전체(G00~G16)를 소유하는 상위 계획서이며 현재 다른 세션이 계속 갱신하고 있다.
이 문서는 그 계획서의 `G03 · Workbench Collider 저작 확장`을 실제 파일·함수·계약 수준으로
확장한 하위 계획서다. 두 문서를 병합하지 않으며 같은 설명을 복제하지 않는다.

구현 뒤 실제 반영 상태와 검증 경계는
[Composition Collider · Counter · Effect 구현 결과](2026-09-02_COMPOSITION_COLLIDER_COUNTER_EFFECT_IMPLEMENTATION_RESULT.md)를 따른다.

---

## 0. 실측 기준점

```text
branch        GB/valtan-pattern-bug-fix
HEAD          fbd30c8e
dirty         Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.camerashots.json
              Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json
              Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json
              Data/Animation/RootMotion/Valtan.rootmotion.json
              Data/Valtan/Valtan.presentation.json
```

작업 전에 현재 working tree source가 실제로 join되는지 먼저 확인했다.

```text
valtan_tuning_pipeline.validate_repository(".")  ->  OK
  gameplaySourceVersion 1 / presentationSourceVersion 1 / joinedSourceVersion 2
  managedPatterns 42 / legacyPatterns 25 / worldMembers 97 / combatObjects 5
  sourceManifestId 61b43f7b02eb05de21b258e85c5b370e2a6a5aacf569d5417da89c8504ed98aa
```

**즉 지금 Save가 막히는 이유는 split source 손상이 아니다.** 아래 3장에서 실제 병목을 분리한다.

`Data/Valtan/Valtan.gameplay.json` 실측 분포.

| 항목 | 값 |
|---|---|
| patterns | 42 |
| stages | 193 |
| `hit.shape.kind` | NONE 148 / BOX 23 / CIRCLE 13 / CONE 6 / CROSS 2 / SIX_DIRECTIONS 1 |
| `decisionModel.manualAuditions` | 31 (MANUAL_SERVER_AUDITION 23 + DERIVED_SERVER_PATTERN 8) |
| manual audition이 없는 Pattern | 11 (`VALTAN_WHIRLWIND`, `VALTAN_FOUR_SLASH`, `VALTAN_HIGH_JUMP`, `VALTAN_DASH_CHARGE`, `VALTAN_FLOOR_WIPE_130`, `VALTAN_FIST_IN_OUT`, `VALTAN_TRIPLE_COUNTER`, `VALTAN_ARENA_BREAK_109`, `VALTAN_ENTRANCE_CINEMATIC`, `VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK`, `VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK`) |
| hit 없는 Stage | manual 117 / canonical 31 |
| hit 있는 Stage | manual 32 / canonical 13 |
| `hit.anchor`를 가진 Stage | **0** |
| `hit.activation`을 가진 Stage | **0** |
| `damage.valtan.*` 프로파일 | 33 (`Data/Balance/DamageProfiles.json`, `damageRatePercent`) |

---

## 1. 지금 콜라이더가 만들어지는 전체 경로

### 1.1 데이터 정본 — Stage 하나가 hit 하나를 소유한다

콜라이더는 독립 오브젝트가 아니다. `Valtan.gameplay.json`의 `patterns[].stages[].hit` **필드 하나**다.

```json
{
  "stageId": "CHARGE",
  "actionId": "valtan.dash-charge.charge",
  "stageKind": "ACTIVE",
  "durationMs": 2000,
  "hit": {
    "shape": { "kind": "BOX", "lengthM": 10.0, "halfWidthM": 2.5 },
    "schedule": { "kind": "INTERVAL", "count": 1, "firstOffsetMs": 0, "intervalMs": 0 },
    "serverDamageProfileId": "damage.valtan.dash-charge",
    "pushRangeM": 2.0, "pushMs": 150, "knockdown": true, "downMs": 1000
  }
}
```

여기서 나오는 첫 번째 구조적 결론이다.

```text
한 Stage = 한 콜라이더.  "박스를 두 개 추가"는 이 스키마에 존재하지 않는다.
Stage 안에서 시간을 나누고 싶으면 schedule(펄스)이나 activation(지속 구간)을 쓴다.
정말로 두 개의 서로 다른 셰이프가 필요하면 Stage를 하나 더 만든다.
```

`hit`가 가질 수 있는 키는 pipeline이 exact로 강제한다
(`Tools/ValtanPipeline/valtan_tuning_pipeline.py:4124~4231`).

| 키 | 필수 | 의미 |
|---|---|---|
| `shape` | 항상 | `{kind}` + kind별 치수. NONE이면 `shape` 하나만 허용 |
| `schedule` | `activation`이 없을 때 | `INTERVAL{count,firstOffsetMs,intervalMs}` 또는 `EXPLICIT_OFFSETS{offsetsMs[1..64]}` |
| `activation` | `schedule` 대신 | `{kind:"ACTIVE_WINDOW", startMs, lifetimeMs, perTargetPolicy:"ONCE"}` |
| `anchor` | optional | `{kind:"BOSS_CURRENT" 또는 "STAGE_ORIGIN", forwardOffsetM, rightOffsetM, yawOffsetDegrees}` |
| `serverDamageProfileId` | NONE 아니면 | `damage.valtan.` 네임스페이스 강제 |
| `pushRangeM` / `pushMs` / `knockdown` / `downMs` | NONE 아니면 | 플레이어 반응 |
| `playerResponse` / `attachmentSlot` | 쌍으로 optional | `CAPTURE` + `BOSS_LEFT_HAND`만 |

`shape.kind`별 exact 필드 (`valtan_tuning_pipeline.py:1367~1372`).

```text
CIRCLE          kind, outerRadiusM
RING            kind, innerRadiusM, outerRadiusM
CONE            kind, angleDegrees, lengthM
BOX             kind, lengthM, halfWidthM
CROSS           kind, lengthM, halfWidthM
SIX_DIRECTIONS  kind, lengthM, halfWidthM
```

### 1.2 저작에서 파일까지의 여섯 단계

```text
[1] Client 읽기 모델      CValtanPatternTree  ->  VALTAN_STAGE_VIEW
      Client/Private/ValtanPatternTree.cpp:4125  hit.activation / anchor / schedule 파싱
      Client/Public/ValtanPatternTree.h:236~259  strHitShape, fHit*, HitOffsetsMs,
                                                 bHasHitAnchor, bHasHitActivation,
                                                 iHitActivationStartMs / LifetimeMs

[2] Balance 드래프트      CBalanceTool::Get_ValtanStageDraft
      Client/Private/BalanceTool.cpp:1570 BuildValtanStageDraft
      -> PATTERN_STAGE_EDIT (Client/Public/BalanceTool.h:36)

[3] Workbench Detail UI   Render_GameplayStageDetails
      Client/Private/ActionCompositionWorkbench.cpp:5645
      "Stage Hit (Boss -> Player)" 섹션 :5996 ~ :6360
      값이 바뀌면 :6678 SetValtanStageDraftWithSoundDependencyAdmission

[4] 드래프트 검증/반영    CBalanceTool::Set_ValtanStageDraft
      Client/Private/BalanceTool.cpp:2971
      hitChanged 판정 :3346, hitEditable 게이트 :3369,
      geometry :3374, schedule/push/response :3395 ~ :3440,
      실제 write-back :3478 ~ :3497

[5] 패치 생성            CBalanceTool::BuildValtanDraftPatch
      Client/Private/BalanceTool.cpp:7816
      SET_STAGE_HIT 조립 :8793 ~ :8867
      SET_DAMAGE_RATE 조립 :8206 부근

[6] 파일 트랜잭션        Save_Reload -> Save_ValtanCompositionProduct
      Client/Private/ActionCompositionWorkbench.cpp:4504
      Client/Private/BalanceTool.cpp:4293
      -> RunValtanDraftCommand(L"CommitCanonicalDraft")
      -> Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1
      -> valtan_tuning_pipeline.py:9402 SET_STAGE_HIT 적용
                                 :9423 stage["hit"] = deepcopy(operation["hit"])
```

### 1.3 파일 이후 — Product 투영, publish, Server 판정

```text
Data/Valtan/Valtan.gameplay.json  (+ Valtan.presentation.json)
   |  valtan_tuning_pipeline.py:5637 _compile_hit
   |     shape      -> hitShape / hitOuterRadius / hitInnerRadius /
   |                   hitAngleDegrees / hitLength / hitHalfWidth
   |     schedule   -> hitCount / hitIntervalMs / hitDelayMs / hitOffsetsMs
   |     activation -> hitActivation{kind,startMs,lifetimeMs,perTargetPolicy}, 나머지 hit* = 0
   |     anchor     -> hitAnchor
   v
Data/Encounters/Valtan/ValtanEncounter.json   (formatVersion 4, 65 patterns)
   |  Tools/GameplayPipeline/Publish-GameplayBalance.ps1:1900 ~ :2145
   |     PATTERNSTAGE 행 + optional PATTERNSTAGEHITAUTHORITY 행
   v
Server/Bin/DataFiles/... bootstrap
   |  Server/Private/GameplayCatalog.cpp:2530 PATTERNSTAGE 파싱
   |                                    :2700 PATTERNSTAGEHITAUTHORITY 파싱
   |                                    :4290 ~ :4470 stage hit 계약 전수 검증
   v
Server/Private/ValtanBrain.cpp
     :1629 ~ :1662  Stage 진입 시 boss.ePatternHitShape / ... 로 복사
     :2178          ResolvePatternHitTransform  (anchor 적용)
     :2221          ContainsPatternHit          (XZ 셰이프 판정)
     :2299          ApplyPatternHit             (rawDamage 계산 + Apply_WorldToPlayer)
     :2904          ACTIVE_WINDOW  -> startMs ~ startMs+lifetimeMs 동안 매 tick 평가
     :2917          PULSE_SCHEDULE -> hitOffsets / delay+interval 도달 시점마다 1회
```

데미지 실수치는 `ApplyPatternHit`(`ValtanBrain.cpp:2317`)에서 결정된다.

```cpp
const std::uint32_t rawDamage = CGameplayCatalog::Resolve_Damage(
    bossProfile->iAttackPower,
    catalog.Find_DamageRatePercent(boss.strDamageProfileId));
```

즉 **콜라이더가 소유하는 것은 `damageProfileId`(어느 수치를 쓸지)뿐이고, 실수치는
`Data/Balance/DamageProfiles.json`의 `damageRatePercent`가 소유한다.** 두 파일이 다른 owner다.

### 1.4 Client 디버그 와이어

`Client/Private/Valtan.cpp:363 Load_PatternHitAreaDebug`가
`Data/Encounters/Valtan/ValtanEncounter.json`을 읽어 `actionId -> PATTERN_HIT_AREA_DEBUG`를 만들고,
`:418 Draw_PatternHitAreaDebug`가 `CHitAreaWire::Draw`로 그린다.

```text
분홍  실제 펄스 창       hit 시각 ~ +300ms (MIN_VISIBLE_HIT_WINDOW_MS)
호박  authoring 기하     isPreviewDriven && 0 <= age <= stageDurationMs  (로컬 프리뷰에서만)
하늘  counter proxy      WINDUP Stage 전체
단위  METERS_TO_UNITS = 100.f   (월드는 cm, 저작은 m)
```

여기에 이미 확인된 결함이 하나 있다.

```cpp
// Client/Private/Valtan.cpp:382  (라이브)
// Client/Private/Valtan.cpp:1531 (로컬 프리뷰)
const bool_t bHasStageHit = !stage.hitShape.empty() &&
    "NONE" != stage.hitShape && 0u != stage.iHitCount;
```

`ACTIVE_WINDOW` 콜라이더는 정의상 `hitCount == 0`이다
(`GameplayCatalog.cpp:4362` `validActiveWindow`가 `0u == stage.iHitCount`를 요구).
따라서 **lifetime 콜라이더를 저작하면 Server는 정상 판정하는데 디버그 와이어에는 아무것도 안 그려진다.**

---

## 2. 요청 4가지와 현재 구현의 정확한 격차

사용자 요청: `collider +` -> 박스 생성 -> Box Detail에서 `type` / `damage 수치` / `사이즈` / `life time` 조절 -> Save.

### 2.1 `+`로 박스 생성

현재 Sequencer의 lane `+` 버튼(`ActionCompositionWorkbench.cpp:9025`)은 **아무것도 만들지 않는다.**
`Request_LaneAuthoring`(`:10331` COLLIDER 분기)이 하는 일은 Details 창을 열고
`m_strSelectedStableId = stageId + "/collider"`로 스크롤을 맞추는 것뿐이다.

실제 생성 버튼은 Details 안에 있다(`:6041`).

```cpp
const bool_t bManualColliderAddAdmitted =
    Pattern.bManualServerAudition &&
    "WAIT" != Stage.strSequenceRole && Draft.hitEditable;   // :6015
...
if (ImGui::Button("Add Server Collider"))                    // :6041
{
    Draft.hitShape = "CIRCLE";                               // <- 셰이프 고정
    Draft.hitOuterRadius = 5.0;
    Draft.hitCount = 1u; Draft.hitIntervalMs = 0u; Draft.hitDelayMs = 0u;
    Draft.hitOffsetsMs.clear();
    Draft.damageProfileId = DamageProfiles[m_iDamageProfileSelection];
    bChanged = true;
}
```

격차 세 가지.

```text
(a) 진입점이 lane "+"가 아니라 Details 안쪽 버튼이다.
(b) 생성 셰이프가 CIRCLE로 하드코딩되어 있다. BOX를 만들려면 만든 뒤 콤보에서 바꿔야 한다.
(c) 게이트 Draft.hitEditable  (BalanceTool.cpp:1629)
        !isWaitStage && (pattern.bManualServerAudition || "NONE" != stage.strHitShape)
    -> canonical 11개 Pattern의 hit 없는 Stage 31개는 "+"가 영구 비활성이다.
```

### 2.2 `type` 선택

`Collider Shape` 콤보(`:6081`)는 **hit가 이미 있을 때만** 나온다. 그리고 여기서 말하는 type이
셰이프인지 판정 종류인지를 UI가 구분하지 않는다. 현재 편집 가능한 것과 아닌 것.

| type 축 | 필드 | 현재 Workbench |
|---|---|---|
| 기하 셰이프 | `hit.shape.kind` | 편집 가능 (`:6081`) |
| 판정 결과 | `hit.playerResponse` | `Trigger Result` 콤보 존재하지만 **DAMAGE -> CAPTURE 단방향만**, 그것도 같은 Pattern이 이미 `RELEASE_GRABBED_PLAYERS`를 가질 때만 (`BalanceTool.cpp:3447~3469`) |
| 판정 방향 | counter proxy | 별도 섹션에 있고 hit와 분리되어 있음 (`:6470~`) |
| 시간 모델 | `schedule` vs `activation` | **activation은 UI에 아예 없다** |

### 2.3 `damage 수치` 조절

`Damage Profile` 콤보는 **ID만 고른다**(`:6107`). 실수치 `damageRatePercent`는
`Data/Balance/DamageProfiles.json`에 있고 Balance Tool에서만 편집된다.

여기서 상위 계획서 G03의 서술을 실측으로 교정한다.

```text
상위 계획서 G03 변경 4:
  "Data/Balance/DamageProfiles.json에 별도 CAS 저장.
   Pattern source와 하나의 atomic writer라고 표시하지 않는다."

실측:
  BalanceTool.cpp:8206 부근이 damageRatePercent 변경을 SET_DAMAGE_RATE op로
  같은 draft patch에 넣는다.
  valtan_tuning_pipeline.py:10050~10071이 그 op로 patched_damage 행을 바꾸고,
  :10258 project_balance_products가 DAMAGE_REL을 투영하며,
  :10347 project_provenance_receipt가 바뀐 field를 PROJECT_TUNED로 동기화한다.

  즉 DamageProfiles.json은 Sound cue와 달리 이미 CommitCanonicalDraft와
  같은 하나의 atomic transaction 안에 있다. 별도 CAS owner를 새로 만들 필요가 없다.
```

Sound cue는 별도 owner가 맞다(`Save_Reload`가 baseline/candidate 바이트 쌍을 따로 stage한다).
Damage는 아니다. 이 차이를 섞으면 불필요한 두 번째 writer를 만들게 된다.

다만 실수치는 **공유 자원**이다. 33개 프로파일을 45개 hit Stage가 나눠 쓰므로,
한 값을 바꾸면 그 프로파일을 쓰는 모든 Stage가 같이 바뀐다. UI가 이걸 숨기면 안 된다.

### 2.4 `life time` 조절

`hit.activation`은 **Client 저작 계층을 제외한 모든 곳에 이미 구현돼 있다.**

| 계층 | 상태 | 위치 |
|---|---|---|
| source 스키마 | 있음 | `valtan_tuning_pipeline.py:4138, 4164~4176` |
| Product 투영 | 있음 | `valtan_tuning_pipeline.py:5654, 5691` |
| publisher | 있음 | `Publish-GameplayBalance.ps1:2076~2100, 2126` |
| Server 파싱 | 있음 | `GameplayCatalog.cpp:355~358, 2700~2732` |
| Server 계약 검증 | 있음 | `GameplayCatalog.cpp:4359~4392` |
| Server 판정 | 있음 | `ValtanBrain.cpp:1660~1662, 2322, 2904~2915` |
| Client 읽기 모델 | 있음 | `ValtanPatternTree.h:257~259`, `ValtanPatternTree.cpp:4125` |
| Product reference | 있음 | `EncounterPatternReference.h:41~43` |
| **`PATTERN_STAGE_EDIT`** | **없음** | `BalanceTool.h:36~79` |
| **`SET_STAGE_HIT` 패치** | **없음** | `BalanceTool.cpp:8813~8860` |
| **Workbench UI** | **없음** | `ActionCompositionWorkbench.cpp:6224~` |
| **디버그 와이어** | **없음** | `Valtan.cpp:382, 1531` |

그리고 이 격차는 단순 누락이 아니라 **잠재 데이터 손실 경로**다.

```text
Set_ValtanStageDraft (BalanceTool.cpp:3478~3497)
  -> hit* 필드만 write-back. bHasHitAnchor / bHasHitActivation은 손대지 않음

BuildValtanDraftPatch (BalanceTool.cpp:8813~8860)
  -> hit 오브젝트를 shape / schedule / damage / push / knockdown / down 으로만 조립
     anchor와 activation을 절대 emit하지 않음

valtan_tuning_pipeline.py:9423
  -> stage["hit"] = copy.deepcopy(operation["hit"])     # 통째 교체

결과: activation 또는 anchor를 가진 Stage에서 반경을 0.1m만 바꿔도
      그 두 블록이 파일에서 조용히 사라진다.
현재 anchor/activation을 쓰는 Stage가 0개라 아직 터지지 않았을 뿐이며,
lifetime 저작을 여는 순간 즉시 살아나는 버그다.
```

---

## 3. Save 병목 — 무엇이 실제로 막고 있는가

Save 실패를 하나로 뭉뚱그리면 안 된다. 실측한 게이트는 여섯 층이고 각각 다른 문장을 낸다.

### 3.1 게이트 전수

| # | 게이트 | 위치 | 화면 문장 | 실제 의미 |
|---|---|---|---|---|
| 1 | Save 버튼 비활성 | `ActionCompositionWorkbench.cpp:8571` | 버튼이 회색 | `m_bAuthoringDraftDirty` / Sound dirty / EffectV2 dirty가 전부 false거나 admission이 ADMITTED가 아님 |
| 2 | 드래프트 거절 | `BalanceTool.cpp:3369` | `this canonical Stage does not own an editable Collider contract` | `hitEditable == false`. Save까지 못 감. 값이 드래프트에 들어가지도 않음 |
| 3 | 드래프트 거절 | `BalanceTool.cpp:3395`, `:3438` | `hit schedule or player reaction is invalid` | schedule / push / response 조합 위반 |
| 4 | revision pin | `ActionCompositionWorkbench.cpp:4517~4528` | `The Pattern files changed after this window opened` | 창을 연 뒤 다른 tool/publisher가 source를 바꿈. `STALE_PRESERVED`로 강등 |
| 5 | 공동 소유자 | `:4531~4600` | `Sound data ...` / `Animation links are invalid` / `Effect V2 ...` | **콜라이더와 무관한 owner의 실패가 콜라이더 저장을 같이 막는다** |
| 6 | 파이프라인 | `BalanceTool.cpp:4293` -> `Publish-ValtanTuningRuntimeSet.ps1` | `Composition Save failed; Pattern, Sound, and Effect V2 files were preserved: ...` | source/Product CAS, 30초 writer lock, joined validate 중 하나 실패 |

### 3.2 콜라이더 작업에서 실제로 걸리는 것

`validate_repository`가 통과했으므로 지금 상태에서 콜라이더 저장을 막는 것은 6번이 아니다.
남는 것은 2번과 5번이다.

**병목 A — `hitEditable` 게이트가 "추가"와 "수정"을 한 플래그로 묶었다.**

```cpp
// BalanceTool.cpp:1629
draft.hitEditable = !isWaitStage &&
    (pattern.bManualServerAudition || "NONE" != stage.strHitShape);
```

이 한 줄이 세 가지 서로 다른 판단을 겸하고 있다.

```text
"이 Stage에 콜라이더를 새로 만들어도 되는가"
"이미 있는 콜라이더의 기하/스케줄을 조정해도 되는가"
"이 콜라이더를 지워도 되는가"
```

그래서 canonical Pattern의 hit 없는 Stage 31개는 조정도 추가도 못 하고,
반대로 manual Pattern에서는 세 동작이 전부 같은 문장으로 거절된다.

**병목 B — Save가 3개 파일 owner와 pipeline/reload gate를 한 transaction으로 묶는다.**

`Save_Reload`(`:4504`)는 Pattern/Balance / Pattern Sound / Effect V2의 세 파일 owner와
Boss Tool의 transaction/reload gate를 하나의 성공 조건으로 묶는다. 콜라이더 반경 하나만 바꿔도 Sound generation이 pin되어 있으면
`Save is pinned while a Server Pattern occurrence owns the current Sound generation`으로 전부 막힌다.

이 all-or-nothing 자체는 계약이므로 깨지 않는다. 대신 **어느 owner가 막았는지를 버튼 옆에서
바로 읽을 수 있어야 한다.** 지금은 실패 문자열이 한 줄로 합쳐져 나온다.

**병목 C — 거절 문장이 다음 행동을 알려주지 않는다.**

```text
현재: "this canonical Stage does not own an editable Collider contract.
       Add/remove is available only to a manual audition Stage;
       an existing canonical hit may be tuned in place."
```

사용자가 실제로 알아야 하는 것은 "이 Pattern이 manual audition이 아니다"와
"그래서 지금 할 수 있는 것은 Create New Pattern이다"인데, 문장이 규칙만 말하고
**현재 Pattern이 어느 쪽인지를 말하지 않는다.**

### 3.3 해결 방향

```text
병목 A  hitEditable 한 플래그를 세 플래그로 분리한다.
          colliderAddAdmitted     : 없던 hit를 만들 수 있는가
          colliderTuneAdmitted    : 있는 hit의 값을 바꿀 수 있는가
          colliderRemoveAdmitted  : hit를 지울 수 있는가
        Set_ValtanStageDraft는 hitChanged를 add / tune / remove로 분류해
        해당 플래그만 검사한다. 기존 canonical hit의 in-place 튜닝 동작은 그대로 유지된다.

병목 B  Save 버튼 옆에 owner별 상태를 나눠 표시한다.
          Pattern  dirty/clean
          Sound    dirty/clean/pinned(사유)
          EffectV2 dirty/clean
        Save 실패 시 첫 실패 owner 이름을 문장 맨 앞에 둔다.

병목 C  거절 문장에 현재 Pattern의 admissionState와 다음 행동을 넣는다.
          "VALTAN_WHIRLWIND is a canonical Pattern (no manual audition row).
           New collider Add is unavailable. Tune the existing hit, or create a
           manual audition Pattern from this Sequence."
```

---

## 4. G 구성

새 C++ 파일은 만들지 않는다. `.vcxproj` / `.filters` 등록 변경 없음.

### G01 · `PATTERN_STAGE_EDIT`에 anchor / activation을 실어 왕복 손실을 막는다

| 항목 | 내용 |
|---|---|
| 파일 | `Client/Public/BalanceTool.h`, `Client/Private/BalanceTool.cpp` |
| 변경 1 | `PATTERN_STAGE_EDIT`(`:36`)에 `bool hasHitAnchor` / `std::string hitAnchorKind` / `double hitAnchorForwardOffsetM, hitAnchorRightOffsetM, hitAnchorYawOffsetDegrees`, `bool hasHitActivation` / `std::uint32_t hitActivationStartMs, hitActivationLifetimeMs` 추가 |
| 변경 2 | `BuildValtanStageDraft`(`:1570`)가 `VALTAN_STAGE_VIEW`의 같은 이름 필드를 그대로 복사 |
| 변경 3 | `Set_ValtanStageDraft`의 `hitChanged`(`:3346`)에 새 필드를 포함하고, write-back(`:3478~3497`)에서 `stage->bHasHitAnchor` / `bHasHitActivation` / `iHitActivation*`를 반영 |
| 변경 4 | `Set_ValtanStageDraft` 검증에 activation 규칙 추가. `ACTIVE_WINDOW`이면 `hitCount == 0 && hitIntervalMs == 0 && hitDelayMs == 0 && hitOffsetsMs.empty() && lifetimeMs >= 1 && startMs + lifetimeMs <= durationMs`. `PULSE_SCHEDULE`이면 activation 필드가 전부 0 |
| 변경 5 | `BuildValtanDraftPatch`의 `SET_STAGE_HIT`(`:8813~8860`)가 `activation`이 있으면 `schedule` 대신 `"activation": {kind, startMs, lifetimeMs, perTargetPolicy:"ONCE"}`를, `anchor`가 있으면 `"anchor": {...}`를 함께 emit |
| 변경 6 | `SameValtanColliderDraft`(`ActionCompositionWorkbench.cpp:878`)에 새 필드 비교 추가 |
| 종료 증거 | 임시 repo transaction fixture에서 `hit.activation`이 있는 Stage의 반경만 `SET_STAGE_HIT`로 바꾼다. commit 뒤 `activation`/`anchor`가 그대로 남고 invalid union/bounds/anchor 입력은 모든 파일을 보존한다. 실제 정본 데이터를 증명용으로 손수 고치지 않는다 |

이 G가 다른 모든 G보다 먼저다. 이걸 하지 않고 lifetime UI부터 만들면 첫 저장에서 값이 사라진다.

### G02 · 콜라이더 권한을 add / tune / remove로 분리한다

| 항목 | 내용 |
|---|---|
| 파일 | `Client/Public/BalanceTool.h`, `Client/Private/BalanceTool.cpp`, `Client/Private/ActionCompositionWorkbench.cpp`, `Tools/ValtanPipeline/valtan_tuning_pipeline.py` |
| 변경 1 | `PATTERN_STAGE_EDIT`에 `colliderAddAdmitted` / `colliderTuneAdmitted` / `colliderRemoveAdmitted`를 추가한다. 기존 `hitEditable`은 Animation Tool 소비자를 깨지 않도록 **tune-only 호환 alias**로 유지하며 Add/Remove 권한으로 쓰지 않는다 |
| 변경 2 | `BuildValtanStageDraft`(`:1629`)에서 `add = !isWaitStage && pattern.bManualServerAudition && "NONE" == stage.strHitShape`, `tune = !isWaitStage && "NONE" != stage.strHitShape`, `remove = !isWaitStage && pattern.bManualServerAudition && "NONE" != stage.strHitShape && "CAPTURE" != stage.strPlayerResponse`로 계산한다. canonical 기존 hit는 tune만 가능하다 |
| 변경 3 | `Set_ValtanStageDraft`(`:3369`)가 `hitChanged`를 세 부류로 나눠 각각의 플래그만 검사한다. 거절 문장에 `pattern->strAdmissionState`와 다음 행동을 포함 |
| 변경 4 | `PATTERN_STAGE_EDIT` 동등성 검사(`:3032~3037`)의 파생 플래그 비교 규칙은 그대로 유지한다. 새 세 플래그도 파생값이므로 candidate가 그대로 들고 오지 않으면 거절된다 |
| 변경 5 | Workbench의 `bManualColliderAddAdmitted`(`:6015`, `:7696`)와 `Draft.hitEditable` 분기(`:6070`, `:6360`)를 새 세 플래그로 교체 |
| 변경 6 | 최종 `SET_STAGE_HIT` writer도 old/new hit를 Add/Tune/Remove로 분류해 C++과 같은 권한을 재검증한다. canonical no-hit Add, canonical Remove, CAPTURE Remove는 UI를 우회한 명령도 거부하고 모든 파일을 보존한다 |
| 변경 7 | `RenderValtanManagedPattern`에 남은 직접 collider/damage 편집기는 ACTIVE_WINDOW를 pulse로 오인하고 typed setter를 우회하므로 읽기 전용 요약으로 내린다. 콜라이더 쓰기는 Action Composition Workbench Details 한 경로만 소유한다 |
| 종료 증거 | `VALTAN_WHIRLWIND / SPIN`(canonical, CIRCLE 10.0m)에서 반경 튜닝은 되고 Remove/Add는 사유가 적힌 문장과 함께 비활성. `VALTAN_TRASH`의 hit 없는 manual Stage에서 Add가 활성. temp transaction에서 우회 canonical Add/Remove와 CAPTURE Remove도 rollback |

### G03 · Sequencer Collider lane `+`가 실제로 박스를 만든다

| 항목 | 내용 |
|---|---|
| 파일 | `Client/Private/ActionCompositionWorkbench.cpp`, `Client/Public/ActionCompositionWorkbench.h` |
| 변경 1 | `Request_LaneAuthoring`의 COLLIDER 분기(`:10331`)를 두 경로로 나눈다. 선택 Stage에 hit가 없고 `colliderAddAdmitted`이면 즉시 BOX 기본값으로 드래프트를 만들고 Details를 연다. 이미 있으면 지금처럼 Details만 연다 |
| 변경 2 | 새 기본값은 BOX로 통일한다. `hitShape="BOX"`, `hitLength=8.0`, `hitHalfWidth=2.5`, `hitCount=1`, `hitDelayMs=0`, `hitIntervalMs=0`, `damageProfileId`는 현재 콤보 선택값. 사용자의 "박스를 생성한다"와 일치시키고, 필요하면 Detail에서 셰이프를 바꾼다 |
| 변경 3 | Details의 `Add Server Collider`(`:6041`)도 같은 BOX 기본값을 쓰고 버튼 라벨을 `Add Server Collider (BOX)`로 바꾼다. 두 진입점이 같은 헬퍼 하나를 호출한다 |
| 변경 4 | lane `+` 툴팁을 lane별로 나눈다. COLLIDER는 `Create a Server collider box on the selected Stage.` 또는 비활성 사유 |
| 변경 5 | 타임라인 COLLIDER 박스 생성(`:3717~3746`)에서 `activation`이 있으면 시작/끝을 `stageStart+startMs` ~ `stageStart+startMs+lifetimeMs`로 계산한다. 지금은 펄스 스케줄만 보므로 lifetime 박스가 폭 0으로 그려진다 |
| 종료 증거 | Sequencer에서 Collider lane `+` 한 번으로 선택 Stage에 BOX 8.0 x 2.5 박스가 생기고 타임라인에 실제 폭으로 나타난다 |

### G04 · Box Detail — type / size / lifetime / damage 수치

| 항목 | 내용 |
|---|---|
| 파일 | `Client/Private/ActionCompositionWorkbench.cpp`, `Client/Public/BalanceTool.h`, `Client/Private/BalanceTool.cpp` |
| 변경 1 | `Stage Hit (Boss -> Player)` 섹션 맨 위에 `Collider Type` 콤보를 둔다. 값은 `Damage` / `Grab (Capture)`. 현재 `Trigger Result` 콤보(`:6126`)를 이 이름으로 올리고, 비활성일 때 사유(같은 Pattern에 `RELEASE_GRABBED_PLAYERS`가 없음)를 표시 |
| 변경 2 | `Collider Shape` 콤보(`:6081`)는 그대로 두되 섹션 이름을 `Shape / Size`로 묶고 BOX일 때 `Length (m)` / `Half Width (m)`가 항상 보이게 순서를 정리 |
| 변경 3 | `Timing` 서브섹션을 만들고 `Timing Mode` 라디오를 둔다. `Pulse Schedule`은 기존 delay/interval/count/explicit offsets, `Active Window (lifetime)`은 `Start (ms)` + `Lifetime (ms)`. 모드 전환 시 반대편 필드를 0/빈 값으로 정규화한다 |
| 변경 4 | `Type == Damage`일 때만 `Damage` 서브섹션을 그린다. `Damage Profile` 콤보 아래에 `Damage Rate (%)` 드래그를 둔다. 값은 `CBalanceTool`의 새 typed 경계로 읽고 쓴다 |
| 변경 5 | `CBalanceTool`에 `Get_ValtanDamageRateDraft(profileId, ratePercent, status)` / `Set_ValtanDamageRateDraft(profileId, ratePercent, status)`를 추가한다. `damage.valtan.` 접두 검사, `Require_ValtanAuthoringAdmission`, Gameplay publisher 정본 범위 **1..100000**, `MarkDirty(true)`. 기존 private `FindDamageRate`(`BalanceTool.h:607`)를 재사용하고 두 번째 damage 저장 경로를 만들지 않는다 |
| 변경 6 | `Damage Rate (%)` 옆에 이 프로파일을 쓰는 Stage 수를 표시한다. `Get_ValtanDamageProfileStageUserCountDraft`가 현재 draft의 Gimmicks/Rotation 전체를 훑어 exact stable ID 일치 수를 센다 |
| 변경 7 | 실제 피해량 미리보기를 한 줄 표시한다. `attackPower x rate% / 100 = raw damage`. `Get_ValtanBossAttackPowerDraft`가 정확히 하나인 `BOSS_VALTAN` draft를 typed read로 제공하고 Server와 같은 최소 1/uint32 saturation 계산을 쓴다 |
| 종료 증거 | `VALTAN_TRASH`의 manual Stage에 BOX를 만들고 Type=Damage, Length 6.0, Half Width 2.0, Active Window start 200 / lifetime 800, Damage Rate 300%로 저장. 저장 후 `Valtan.gameplay.json`에 `activation` 블록이, `DamageProfiles.json`에 새 rate가 함께 들어 있고 `validate_repository` 통과 |

### G05 · Save 진단과 디버그 와이어를 lifetime 콜라이더에 맞춘다

| 항목 | 내용 |
|---|---|
| 파일 | `Client/Private/ActionCompositionWorkbench.cpp`, `Client/Private/Valtan.cpp`, `Client/Public/Valtan.h` |
| 변경 1 | Save 버튼 옆 표시를 owner별로 나눈다. `Pattern`은 clean/dirty/read-only, `Sound`는 clean/dirty/pinned/unavailable, `EffectV2`는 clean/dirty를 표시한다. 지금은 `Unsaved changes` 한 줄이라 어느 owner가 막았는지 알 수 없다 |
| 변경 2 | `Save_Reload` 실패 문자열 맨 앞에 실패 owner 토큰을 붙인다 (`[Pattern]`, `[Sound]`, `[EffectV2]`, `[Pipeline]`) |
| 변경 3 | `Valtan.cpp:382`와 `:1531`의 `bHasStageHit`에서 `0u != iHitCount` 조건을 `(0u != iHitCount || stage.bHasHitActivation)`으로 바꾼다 |
| 변경 4 | `PATTERN_HIT_AREA_DEBUG`(`Valtan.h:568`)에 `bHasActivation` / `iActivationStartMs` / `iActivationLifetimeMs`를 추가하고, `Draw_PatternHitAreaDebug`(`:440~461`)의 `isHitWindow` 판정에 activation의 반열린 구간 `[startMs, startMs + lifetimeMs)`을 포함한다. 라이브 아레나에서 lifetime 동안만 분홍 와이어가 유지된다 |
| 종료 증거 | G04에서 저장한 Active Window 콜라이더가 Server 실행 중 200ms ~ 1000ms 구간 동안 분홍 와이어로 보인다. 사용자가 직접 확인한다 |

---

## 5. 검증

### 5.1 자동 (에이전트가 실행)

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

```powershell
python Tools/ValtanPipeline/test_action_composition_collision_authoring_contract.py
python Tools/ValtanPipeline/test_action_composition_collider_timeline_authoring.py
python Tools/ValtanPipeline/test_valtan_balance_tool_contract.py
python Tools/ValtanPipeline/test_valtan_canonical_typed_patch_transaction.py
python Tools/ValtanPipeline/test_action_composition_atomic_save_contract.py
python Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py
```

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
git diff --check
```

새로 추가할 회귀 픽스처.

| 파일 | 검사 |
|---|---|
| `test_action_composition_collision_authoring_contract.py` | Details에 `Collider Type`, `Timing Mode`, `Active Window (lifetime)`, `Damage Rate (%)` 문자열이 존재. `Add Server Collider (BOX)` 기본값이 `hitLength` / `hitHalfWidth`를 쓰고 `hitOuterRadius`를 쓰지 않음 |
| `test_valtan_canonical_typed_patch_transaction.py` | `hit.activation`을 가진 Stage에 반경만 바꾸는 `SET_STAGE_HIT`를 적용해도 `activation` / `anchor`가 보존됨 (G01 회귀) |
| 신규 `test_valtan_stage_hit_activation_authoring.py` | `PATTERN_STAGE_EDIT`에 activation 필드가 있고, `BuildValtanDraftPatch`가 `activation`과 `schedule`을 동시에 emit하지 않으며, `ACTIVE_WINDOW`에서 `hitCount == 0`을 강제 |

### 5.2 수동 (사용자가 직접 실행·판정)

에이전트는 여기까지만 준비하고 멈춘다. Client/UI를 자율 실행하거나 화면을 캡처하지 않는다.

```text
1. Server + Client profile로 실행 (192.168.0.14:7777)
2. F1 -> Action Composition Workbench -> Open Valtan
3. VALTAN_TRASH 선택 -> hit 없는 manual Stage 선택
4. Sequencer의 Collider lane "+" 한 번 클릭
   기대: BOX 8.0 x 2.5 박스가 타임라인에 폭을 가지고 나타난다
5. Details에서 Type=Damage, Length 6.0, Half Width 2.0
   Timing Mode = Active Window, Start 200, Lifetime 800
   Damage Rate 300%
6. Save
   기대: "Saved" 표시. 실패하면 [Pattern] / [Sound] / [EffectV2] / [Pipeline] 중
         하나로 시작하는 문장
7. Publish Server Data 후 Server 재시작
8. Valtan Arena에서 해당 패턴 재생
   기대: stage 200ms ~ 1000ms 동안 분홍 와이어 유지, 그 안의 플레이어가 1회 피격
```

---

## 6. 이 계획이 만들지 않는 것

- **플레이어 스킬 콜라이더 저작.** `Data/Animation/HitShapes/`는 `build_hitshapes.py`가 소유하며 Valtan Workbench는 보스 전용이다. 상위 계획서 G04의 범위다.
- **한 Stage에 두 개 이상의 콜라이더.** `stage.hit`는 단수 필드다. 여러 셰이프가 필요하면 Stage를 추가하거나 combat object를 쓴다. 스키마를 배열로 바꾸는 것은 별도 수직 슬라이스다.
- **counter proxy를 콜라이더 type의 한 값으로 병합.** counter proxy는 플레이어 -> 보스 방향이고 hit는 보스 -> 플레이어 방향이다. 두 섹션을 그대로 분리해 둔다.
- **Save의 all-or-nothing 해체.** 세 파일 owner와 pipeline/reload gate의 원자성은 계약이다. 진단만 개선한다.
- **Hot Reload.** 저장 후 적용은 계속 Publish + Server 재시작이다.
- **canonical Pattern에 콜라이더 신규 추가 허용.** G02는 canonical Stage의 in-place 튜닝만 열고 Add는 계속 manual audition 전용으로 둔다. canonical에 hit를 추가하려면 그 Pattern을 manual audition으로 승격하는 별도 계약이 필요하다.
