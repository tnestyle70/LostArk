# 2026-09-02 발탄 패턴 · 콜라이더 저작 · Composition 병목 구현 계획서

작성 기준은 이 저장소의 현재 코드와 `Data` 실측이다. 브랜치는 `GB/valtan-pattern-bug-fix`,
HEAD는 `fbd30c8e`이며 working tree는 `Data/Valtan/Valtan.presentation.json`,
`Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json` 두 파일의 cue 삭제 diff와
`Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.*`, `Data/Animation/RootMotion/Valtan.rootmotion.json`의
개행 변화만 가지고 있다.

이 문서는 구현 계획서다. 각 G는 실제 파일과 종료 증거를 소유하며, 전체 코드 정본은
디테일 계획서에서 확장한다.

이번 작업에서 실제 반영한 Collider·Counter·0.6초 Effect 세 범위와 검증 경계는
[Composition Collider · Counter · Effect 구현 결과](2026-09-02_COMPOSITION_COLLIDER_COUNTER_EFFECT_IMPLEMENTATION_RESULT.md)를 따른다.

---

## 요구사항 전수 대조표

사용자 원문을 문장 단위로 쪼갠 31개 항목이다. 이 표가 누락 판정의 정본이며,
아래 본문의 절/G 번호가 각 항목의 소유자다.

### A. 질문 — 답이 문서 안에 있어야 하는 것 (6)

| # | 원문 요구 | 답이 있는 곳 | 상태 |
|---|---|---|---|
| Q1 | "이 stage의 개념이 정확하게 어떤 건지 잘 모르겠어" | 0.1 | 답변 완료 |
| Q2 | "5000ms 입력했을 때 실질적으로 뭐가 변하고 로직 상에서 이 stage라는 게 정확히 어떤 의미인지" | 0.2 | 답변 완료 |
| Q3 | "왜 reject되었는지 이전 수정 기준으로 어떤 부분이 해결이 안 됐는지" | 0.3 (1차 / 1.5차 / 2차) | 답변 완료 |
| Q4 | "카운터를 위해 존재하는 건가? total time이면 역할을 작게, 분기점이면 branch로 이름 짓는 방향이 맞는 거잖아" | 0.5 | 사용자 판단이 맞음 — G00에 반영 |
| Q5 | "지금 카운터 로직이 정확히 어떤 식으로 되어있는지. 모든 방향? 발탄 look 기준 180도?" | 1.1~1.3 | 답변 완료 — Stage마다 다름, 게이트 3개 |
| Q6 | "버러지들 패턴 기준으로 카운터 공격 가능한 시퀀스 나열해줘" | 1.4 | 답변 완료 — 정확히 3구간 |

### B. 저작 도구 · 병목 (3)

| # | 원문 요구 | G | 상태 |
|---|---|---|---|
| T1 | "composition workbench 기준으로 모든 save를 막는 병목들 전부 다 같이 수정" | G00 | 원인 4개 확정 (A1~A4) |
| T2 | "debug collider들 전부 다 띄우자. 스킬들 기준으로 콜라이더 없는 것들이 너무 많아" | G01 + G04 | 원인 확정 (S1, R: 94개 중 43개 미저작) |
| T3 | "공식적으로 튜닝할 수 있는 툴 — collider 추가, detail 패널에서 사이즈·모양 조절, 정해진 시간 동안, collider type 선택, damage면 실제 데미지 수치 조절해서 저장" | G03 | 현재 있는 것 / 없는 것 4개 확정 |

### C. 마력구 (`VALTAN_STAGGER_SLOT`) (3)

| # | 원문 요구 | G | 상태 |
|---|---|---|---|
| M1 | "0.5m 정도만 올리자 지금 조차도 너무 높게 올라갔어" | G05 | 원인 확정 — Pattern `verticalOffsetM: 3.0` |
| M2 | "1000 데미지가 들어가거나, 넣지 못하고 공격 시퀀스로 들어가는 그 타이밍 기준으로 올라갔던 위치를 다시 아래로 — 이 로직이 존재하지 않아" | G05 | 원인 확정 — restore가 `FinishPattern`에만 있어 Stage 전이는 안 탄다 |
| M3 | "가운데 검은색 마력구 존재해야 하는데 보이지 않아" | G06 | 좁힌 가설 — D-1 단위 불일치(3m vs 3cm)가 유력, 1프레임 덤프로 확정 |

### D. 카운터 (3)

| # | 원문 요구 | G | 상태 |
|---|---|---|---|
| C1 | "카운터 성공과 실패에 대한 logic 분기 흐름 검증" | 1.1~1.5 | 검증 완료 — 결함 3개(C1/C2/C3) 도출 |
| C2 | "3연속 공격 카운터 패턴에는 windup이 없고, 카운터 쳐야 하는 내려치기의 경우는 loop가 없다" | G14 (P1/P2/P3) | 원인 확정 — `mesh_att_battle_14_loop`가 clipmap에 없음 |
| C3 | "3연속 공격 카운터 패턴 - 그로기, 3번 연속 재생되는 시퀀스로 구성되어 있는데 이 부분 뭔지 모르겠음" | G14 (P1) | 정체 확정 — 7 Stage 전부 `mesh_abn_groggy_1_*` 자리표시자 |

### E. 잡기와 왼손 부착 (4)

| # | 원문 요구 | G | 상태 |
|---|---|---|---|
| H1 | "뒤돌아잡기 후 날리기의 collider. debug로 콜라이더 보이게 하기. 지금 잡히는 기준을 아예 모르겠음" | G01 + G08 | 판정 실측 완료 + 안 보이는 진짜 원인 확정 (S1) |
| H2 | "왼손 부착 로직 전부 파헤치기. 정확히 왼손인지도 모르겠고, world offset 때문인지, 발탄 기준 오프셋이 적용된 건지, 콜라이더 때문에 밀려난 건지" | G08 | 파헤침 완료 — **오프셋 혼입 가설은 코드상 기각.** 남은 변수 3개 |
| H3 | **확정 계약** "플레이어는 자신의 월드 행렬을 offset 행렬로 변환하고 발탄의 왼쪽 손 뼈 행렬을 가져와 offset 행렬과 곱하여 플레이어를 발탄의 손 위치로 옮긴다" | G08 | **현재 코드가 이미 이 계약이다** — 유지하고 `gripLocalOffset`만 추가 |
| H4 | "버러지 패턴과 뒤돌아잡기후날리기, 정확히 왼손에 부착" | G08 | 두 패턴 모두 `attachmentSlot BOSS_LEFT_HAND` 확인 |

### F. 이펙트 (3)

| # | 원문 요구 | G | 상태 |
|---|---|---|---|
| E1 | "중앙 이동 후 피자 패턴 사자후 이펙트 넣기. v2에 존재하는 사자후, 버러지들 패턴 기준으로 있는 사자후. 넣은 다음에 save" | G07 | 원인 확정 — binding 0개. 템플릿은 `VALTAN_BIND_SLOT` |
| E2 | "발악패턴 파운딩 이펙트 루프로 재생. 현재 2번만 재생. 이전 PR과 비교해서 group의 loop 재생 문제인지 검토" | G02 | 원인 확정 — **V2 runtime에 `EACH_LOOP` 구현이 없다** (문자열 0회). group 문제 아님. 단 G02만으로는 부족(5장 경고) |
| E3 | "침묵 패턴 자물쇠 말고, 쿨타임시 생성되는 마스크 R 값 올린 거로 씌우기" | G15 | 원인 확정 — 자물쇠는 발탄 Effect가 아니라 `HUD_Layout.json`의 `Skill_R_SilenceMask` |

### G. 패턴 동작 (6)

| # | 원문 요구 | G | 상태 |
|---|---|---|---|
| P1 | "모아치기 플레이어 기준 반대로 돌아가는 버그 수정. 플레이어 따라가게 피자 패턴 로직대로" | G09 | 가설 — `LOCK_NEAREST_ON_START`가 caller 포인터만 씀. 진단 1단계 필요 |
| P2 | "39 2페이즈 4방향 공격이 무조건 server entry failed. 가끔 원인 모를 것도. 플레이어 죽고 나서 pending/idle 전환 작업 때문인 것 같다" | G16 | **원인 확정** — Six Pizza/Struggling의 delayed visual-only 돌기둥 owner가 Server off-navigation exact 허용 집합에서 누락돼 room runtime failure → orderly FIN → Lobby recovery가 발생 |
| P3 | "발탄 사망 이후 stage 남아 있는 것 때문에 플레이어 추적함. 시퀀스 이후 스테이지 잘라야 함" | G11 (M) | 원인 확정 — death audition이 HP를 0으로 만들지 않아 CHASE 복귀 |
| P4 | "발탄 부활시 유령 발탄이 부활해야 하는데 그냥 발탄 모델이 부활해서 패턴 재생" | G11 (N) | 부분확정 — 스왑 조건은 `iGameplayPhase >= 3`, 실패 시 격리 로그. 로그 먼저 |
| P5 | "부위 파괴 애니메이션 이후 recovery 시퀀스까지 넣어서 구현, 4방향 돌생성 및 파괴 사운드까지" | G13 | 원인 확정 — 원본 4 clip 6983ms 중 `_end_1` 하나만 쓰고 Stage 시계는 `_start` 길이(1400ms) |
| P6 | "워프 한 번 돌진이 2000ms 넘음. 끝 사거리 도달하면 발탄이 사라지고 0.5초 뒤 포탈 생성. 지금은 끝 사거리 도달 이후에도 안 사라지고 애니메이션이 계속 재생" | G10 | 원인 확정 — leg 2300ms 중 이동은 1300ms에 끝나고 남은 1000ms gap 동안 body와 loop clip이 유지 |

### H. 사운드 (3)

| # | 원문 요구 | G | 상태 |
|---|---|---|---|
| S1 | "지금 클립과 이름 기준으로 맞춰서 없는 사운드들 적용" | G12 | 원인 확정 — 562 cue 존재, cue 0개 Stage 목록 확보. 리소스 공백 아님 |
| S2 | "땅구르기 쿵쿵" | G12 | 이벤트 확정 — `G_Voltan2_FootStep1`(4 variants) |
| S3 | "돌 터지는 시점 기준으로 돌 터지는 사운드. 타이밍은 내가 직접 수정할게" | G12 + G13 | **선행 차단 요인 2개 확정** — 카탈로그 미선언(S3) + `presentationEventId` 바인딩 키 부재 |

### I. 메타 (1)

| # | 원문 요구 | 상태 |
|---|---|---|
| X1 | "비평 에이전트 바탕으로 적어도 3번 검토해서 제시" | 비평 1회분 반영 + 실측 재검증. 이후 턴에서 추가 비평 결과는 도착하지 않았고, 새로 채운 G05~G16은 전부 직접 코드/데이터 대조로 작성 |

### 요약

```
질문 6개    전부 답변 완료
작업 25개   원인 확정 19  ·  부분확정/좁힌 가설 3 (M3, P4, H2)  ·  런타임 진단 필요 2 (P1, P2)  ·  메타 1
```

**착수 전에 먼저 풀어야 하는 차단 요인 3개와 admission 교정 1개** — S2는 후속 실측에서
차단 요인이 아닌 것으로 교정됐다.

| 차단 | 막는 대상 | 내용 |
|---|---|---|
| S1 | H1, T2, T3 | 라이브 아레나는 hit offset 뒤 300ms만 그린다. Stage 전체 geometry는 Tool preview 전용 |
| S2 (교정) | C2, C3 | `VALTAN_TRIPLE_COUNTER`는 managed non-manual `CorePatternIds` 경로로 Workbench/Boss Play/Gameplay Details가 이미 열린다. Details 잠금 해제를 위한 `manualAuditions` 승격은 하지 않는다 |
| S3 | S3(돌 소리) | 돌 사운드 2개가 `CharacterSoundCatalog.json`에 미선언이라 문서 전체가 거부된다 |
| A3 | P5 | Replace의 `3u == Clips.size()` 상수가 4-clip 부위 파괴 시퀀스를 거부한다 |

---

## 0. 질문 먼저 — Composition Sequencer의 Stage가 무엇인가

### 0.1 Stage는 "Server가 소유하는 하나의 action 구간"이다

발탄의 한 Pattern은 `Data/Valtan/Valtan.gameplay.json`의 `patterns[].stages[]` 배열이다.
Stage 하나는 다음을 **동시에** 소유한다.

| Stage가 소유하는 것 | 필드 | 실제 소비자 |
|---|---|---|
| 안정 ID | `stageId`, `actionId` | Server 상태, Client presentation join, Effect/Sound cue의 scope |
| 이 구간의 서버 시계 | `durationMs` | `CValtanBrain`이 이 ms 동안 이 Stage에 머문다 |
| 다음으로 갈 곳 | `defaultNextActionId` | 시간이 다 되면 가는 기본 경로 |
| **분기점** | `branches[]` (`COUNTER_HIT`, `TIMEOUT`, `ANY_PLAYER_GRABBED`, `NAVIGATION_BLOCKED`, `HEALTH_DAMAGE_THRESHOLD_REACHED`, `STAGGER_BROKEN`, `ALL_PLAYERS_GRABBED`) | `ApplyPublishedOutcomeBranch` |
| 판정 | `hit.shape` / `hit.schedule` / `serverDamageProfileId` / `playerResponse` / `attachmentSlot` | `CBossCombatRuntime`, `CServerCollisionSystem` |
| 카운터 가능 여부 | `events[]`의 `SET_BOSS_FLAG boss.flag.counterable` + `counterProxy` | `Try_TriggerCounter` |
| 이동 | `motion` (`PORTAL_TARGET_RUSH`, `FORWARD`, `LEAP_*`) | `CValtanBrain` |
| 표현 | 같은 `stageId`의 `Valtan.presentation.json` `animation.occurrences[]` | `CValtan` |

즉 **Stage는 "패턴 전체 시간을 몇 등분한 눈금"이 아니라, 서버 상태 기계의 노드 하나**다.
Stage 경계가 곧 분기점이고, 판정이 바뀌는 지점이고, 애니메이션 clip이 교체되는 지점이다.

실측 예시 — 버러지 패턴(`VALTAN_TRASH`)은 22개 Stage로 다음을 표현한다.

```
STEP_01..STEP_06  중앙 이동 + 버러지 소환 windup      (판정 없음)
STEP_07           카운터 창 1회차   1000ms  WINDUP    counterProxy + COUNTER_HIT 분기
STEP_08            돌진 판정         667ms  ACTIVE    BOX 6.0m x 2.5m, playerResponse=CAPTURE
RUSH_MISS          실패 회복        1000ms  RECOVERY
RECHARGE_WAIT_02   재충전           4100ms  WINDUP
RETRY_WINDUP_02   카운터 창 2회차   1000ms  WINDUP    counterProxy + COUNTER_HIT 분기
RETRY_RUSH_02      돌진 판정         667ms
...(3회차 동일)...
CATCH_COUNTER / CATCH_PRE_IMPACT / CATCH_SLAM / EXECUTE_TAIL   잡기 성공 경로
GROGGY             그로기           4433ms
```

`STEP_07 -> STEP_08`이라는 경계가 존재하는 이유는 시간을 나누고 싶어서가 아니라
"여기서부터 카운터가 아니라 잡기 판정"이기 때문이다.

### 0.2 그래서 5000ms를 입력했을 때 실제로 무슨 일이 일어났는가

`Render_PatternDurationControl`(`Client/Private/ActionCompositionWorkbench.cpp:8409`)의
`Pattern Total Duration (ms)`는 **패턴 전체 길이 슬라이더가 아니다.** 실제 동작은 이렇다.

```
입력 5000 커밋
  마지막 Stage(PART_BREAK)의 sequenceRole이 "WAIT"가 아님
  요청 5000 > 현재 합계 1400
    -> BuildNextManualStageIdentity()  ->  stageId "COMPOSITION_01"
    -> Insert_ValtanManualStageAfter(..., role "WAIT", 3600ms)
    -> m_strSelectedStageId = "COMPOSITION_01"     <-- 선택이 여기로 이동한다
```

`BuildValtanManualStage`(`Client/Private/BalanceTool.cpp:818`)가 만드는 WAIT Stage는

```cpp
stage.strSequenceRole = "WAIT";
stage.strStageKind    = "WAIT" == stageRole ? "ACTIVE" : stageRole;   // <-- ACTIVE로 저장
stage.strAnimationEndPolicy = "NONE";
stage.bSuppressAnimation    = true;
```

그리고 타임라인 Stage lane의 라벨은
`Stage.strStageId + " | " + Stage.strStageKind`(`ActionCompositionWorkbench.cpp:3712`)다.

**결과: 화면에는 `COMPOSITION_01 | ACTIVE`라고 찍히지만 내부 `sequenceRole`은 여전히 `WAIT`다.**
이것이 "reject 이유를 모르겠다"의 직접 원인이다.

### 0.3 두 번의 reject가 각각 다른 이유였다

**1차 reject (Total Duration을 올리기 전, PART_BREAK 대상 Append)**

`Data/Valtan/Valtan.presentation.json`의 PART_BREAK Stage는 이렇게 저장돼 있다.

```json
"animation": { "endPolicy": "LOOP_TO_STAGE_END", "occurrences": [
  { "clip": "mesh_dmg_parts_end_1", "playMs": 0, "repeatUntilStageEnd": true } ] }
```

`Apply_SelectedSequenceToStage`의 Append 가드(`ActionCompositionWorkbench.cpp:2189`)는

```cpp
if (bAppend && any_of(slots, [](s){ return 0u == s.playMs || s.repeatUntilStageEnd; }))
    "Append rejected: convert the current looping/native-duration slot to an exact wall-clock first."
```

`playMs == 0 && repeatUntilStageEnd == true` 이므로 **무조건 reject**다.

**1.5차 — Replace로 우회해도 막힌다 (아직 보지 못한 세 번째 병목)**

사용자가 고른 `레이드 발탄_2번째 부위 파괴 | 4 clips`의 실체는
`Data/Animation/Reference/Valtan/Valtan.clipseq:118`이다.

```
420628 "레이드 발탄_2번째 부위 파괴" seq=2 mode=HOLD
       clips="mesh_dmg_parts_start_1,mesh_dmg_parts_loop_1,mesh_dmg_parts_end_1,mesh_idle_battle_1"
Valtan.clipcuts:118   cuts="1.400,0.400,2.850,2.333"   -> 합계 6983ms
```

`Apply_SelectedSequenceToStage`의 Replace 경로는 시퀀스 합계가 Stage 시계를 넘으면
HOLD-chain fit 정책으로만 통과시킨다.

```cpp
const bool_t bDeterministicHoldChain =
    3u == Selected->Clips.size() &&                    // <-- 정확히 3개여야 한다
    "start" == ClipReplacementRole(Clips[0]) &&
    "loop"  == ClipReplacementRole(Clips[1]) &&
    "end"   == ClipReplacementRole(Clips[2]);
if ("HOLD" != Selected->strMode || !bDeterministicHoldChain || !FitCompositionSequenceCutsToStage(...))
    "Replace rejected: ... no deterministic HOLD-chain fit policy."
```

이 시퀀스는 `start / loop / end` 뒤에 `mesh_idle_battle_1` 꼬리가 붙은 **4개**라
`bDeterministicHoldChain`이 false다. 정작 `FitCompositionSequenceCutsToStage` 자체는
`RequestedCutsMs.size() >= 3`이면 일반적으로 동작한다. **막고 있는 것은 호출자의 `3u ==` 상수다.**

게다가 사용자가 넣은 5000ms는 애초에 6983ms보다 작다. 즉 5000으로는 어떤 경로로도 들어가지 않는다.

**2차 reject (Total Duration 5000 이후, 자동 선택된 COMPOSITION_01 대상 Append)**

`ActionCompositionWorkbench.cpp:2157`

```cpp
if ("WAIT" == Stage.strSequenceRole)
    "Sequence slot edit rejected: WAIT is an explicit blank Stage and must remain Animation NONE."
```

스크린샷의 status는 이 2차 reject다. 그리고 이 상태에서 빠져나갈 UI 경로가 없다.
Details의 `Stage Role` 콤보는 `Draft.stageKind`(ACTIVE/WINDUP/GROGGY)만 바꾸고
`sequenceRole`은 건드리지 않으며, WAIT Stage는 `BuildValtanStageDraft`
(`BalanceTool.cpp:1620`)에서 `stageKindEditable=false, animationEditable=false, hitEditable=false`로
잠긴다. 즉 **WAIT -> ACTIVE 승격 경로가 존재하지 않는다.**

### 0.4 지금 당장 쓸 수 있는 우회 절차 (코드 변경 없음)

1. `Composition Boss Pattern` 창에서 `COMPOSITION_01` 선택 -> `Delete Stage`.
   (저장 전 draft라 `Remove_ValtanManualStage`의 provenance 검사에 걸리지 않는다.)
2. `PART_BREAK` Stage 선택 -> Details의 `Loop Stage Length (ms)`를 **6983 이상**으로 올린다.
   (5000이 아니다. 1400+400+2850+2333 = 6983ms.)
3. 같은 Stage에 `Replace Stage Slots`로 `레이드 발탄_2번째 부위 파괴 | 4 clips`를 넣는다.
   (Append가 아니라 Replace다. Replace가 loop slot을 exact slot 4개로 교체한다.
   Stage 시계가 6983 이상이면 fit 정책을 타지 않으므로 4-clip 게이트에도 걸리지 않는다.)
4. 이후 추가 clip이 필요하면 그때부터 Append가 admitted된다.

또는 2~3 대신 `Composition Boss Pattern -> New Stage (ms)`에 `6983`을 넣고
`Add after PART_BREAK: ACTIVE`로 실제 ACTIVE Stage를 만든 뒤 그 Stage에 Replace한다.

**주의**: Stage 시계가 시퀀스 합계보다 작으면 4-clip 게이트에 걸려 Replace도 거부된다.
반드시 시계를 먼저 올린다.

### 0.5 사용자 제안에 대한 판단 — 이름과 책임을 분리한다

사용자 제안("전체 시간이면 total time으로 작게, 분기점이면 본질대로 이름 짓자")이 맞다.
Stage는 **분기점 + 판정 + clip 교체 지점**이므로 total-time 컨트롤과 분리해야 한다. G00에서
다음으로 정리한다.

- `Pattern Total Duration (ms)`는 **읽기 전용 합계 표시**로 강등한다. 값 편집으로 Stage를
  자동 생성하지 않는다. (지금처럼 몰래 WAIT를 만드는 것이 사고의 원인이다.)
- Stage 추가는 `Composition Boss Pattern`의 명시적 `ACTIVE / WINDUP / GROGGY / WAIT·GAP` 버튼
  하나로 일원화한다.
- 타임라인 Stage 라벨을 `stageId | <역할>`로 바꾸되 **역할은 `sequenceRole`을 우선 표기**한다.
  WAIT이면 `COMPOSITION_01 | WAIT (blank gap)`로 찍는다.
- Details에 `Promote WAIT -> ACTIVE` 버튼을 추가한다. 이 버튼만이
  `sequenceRole`을 `ACTIVE`로 바꾸고 `bSuppressAnimation=false`,
  `animationEndPolicy="EXACT"`로 전환한다. Server 계약상 WAIT는 이미 `stageKind=ACTIVE`이므로
  bootstrap 호환은 깨지지 않는다.

---

## 1. 카운터 로직 — 지금 코드가 실제로 하는 일

질문: "모든 방향 기준 공격? 아니면 발탄 look 방향 기준 180도?"

**답: Stage마다 다르다. 발탄의 카운터는 세 가지 게이트를 순서대로 통과해야 성립한다.**

### 1.1 게이트 1 — 카운터 창(window)이 열려 있어야 한다

Stage의 `events[]`가 ENTER에 `SET_BOSS_FLAG boss.flag.counterable = true`,
EXIT에 `= false`를 발행한다. `Try_TriggerCounter`
(`Server/Private/BossCombatRuntime.cpp`)는 첫 줄에서

```cpp
if (!Has_Flag(state, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE) || !Publish_PatternOutcome(...))
    return false;
Set_Flag(state, COUNTERABLE, false);   // 1회성. 성공하면 즉시 닫힌다.
```

### 1.2 게이트 2 — 때린 스킬이 counterPower를 가져야 한다

`Data/Balance/PlayerSkills.json` 실측: **94개 스킬 중 `counterPower`를 가진 것은 단 1개다.**

```
LANCE_MASTER  슬롯 A  skillId 34580  절룡세  counterPower 1
```

즉 지금 빌드에서 카운터를 칠 수 있는 것은 **랜스마스터 A 하나뿐**이다. 나머지 5개 class는
어떤 스킬로도 카운터를 성립시킬 수 없다.

### 1.3 게이트 3 — counterProxy 위치 조건

`ContainsCounterProxySource(boss, hit.fSourceX, hit.fSourceZ)`

```cpp
// counterProxy가 없으면 -> 방향 무관, 무조건 통과
// kind == BOSS_FORWARD_ARC (arcDegrees는 반드시 180.0, offset/radius 0)
   return dot(source - bossPos, bossForward) >= 0;      // 발탄 정면 반평면 180도, 거리 제한 없음
// kind == BOSS_LOCAL_CIRCLE
   proxy = bossPos + forward*forwardOffsetM + right*rightOffsetM;
   return |source - proxy|^2 <= radiusM^2;              // 작은 원 안
```

### 1.4 현재 저작된 카운터 구간 전수

| Pattern | Stage | 길이 | 창 | proxy | 성공 시 |
|---|---|---|---|---|---|
| `VALTAN_TRIPLE_COUNTER` | `COUNTER_1` | 1800ms | Y | FORWARD_ARC 180° | `VALTAN_GROGGY_FOLLOWUP` |
| `VALTAN_TRIPLE_COUNTER` | `FAIL_1` | 600ms | Y | FORWARD_ARC 180° | `VALTAN_GROGGY_FOLLOWUP` |
| `VALTAN_TRIPLE_COUNTER` | `COUNTER_2` | 1600ms | Y | FORWARD_ARC 180° | `VALTAN_GROGGY_FOLLOWUP` |
| `VALTAN_TRIPLE_COUNTER` | `FAIL_2` | 600ms | Y | FORWARD_ARC 180° | `VALTAN_GROGGY_FOLLOWUP` |
| `VALTAN_TRIPLE_COUNTER` | `COUNTER_3` | 1400ms | Y | FORWARD_ARC 180° | `VALTAN_GROGGY_FOLLOWUP` |
| `VALTAN_COUNTER` | `STEP_02` | 1800ms | Y | **없음(전방위)** | `VALTAN_COUNTER_GROGGY` |
| **`VALTAN_TRASH`** | **`STEP_07`** | **1000ms** | Y | **BOSS_LOCAL 원 (앞 1.0m, r 2.25m)** | 같은 패턴 `GROGGY` |
| **`VALTAN_TRASH`** | **`RETRY_WINDUP_02`** | **1000ms** | Y | 동일 | 같은 패턴 `GROGGY` |
| **`VALTAN_TRASH`** | **`RETRY_WINDUP_03`** | **1000ms** | Y | 동일 | 같은 패턴 `GROGGY` |
| `VALTAN_TRASH_CATCH_IF` | `STEP_07` / `RETRY_WINDUP_02` / `RETRY_WINDUP_03` | 각 1000ms | Y | 동일 | 같은 패턴 `GROGGY` |

**버러지 패턴에서 카운터 가능한 시퀀스는 정확히 3구간이다.**
`STEP_07`(1회차), `RETRY_WINDUP_02`(2회차), `RETRY_WINDUP_03`(3회차).
셋 다 clip은 `mesh_att_battle_13_03`, 각 1000ms이고, 앞에 4100ms `RECHARGE_WAIT`가 붙는다.
카운터에 성공하면 세 구간 모두 같은 패턴의 `GROGGY`(4433ms)로 간다.

### 1.5 여기서 나오는 실제 결함 3개

- **결함 C1**: 버러지 패턴의 proxy는 "발탄 앞 1.0m, 반지름 2.25m 원"이다. 플레이어가 그 원
  안에 서 있어야만 카운터가 성립한다. 반면 3연속 카운터는 정면 180° 반평면(거리 무제한)이다.
  **같은 게임 안에서 두 규칙의 유효 면적이 자릿수 단위로 다르다.** 원작 발탄 카운터는 정면
  판정이므로 버러지도 FORWARD_ARC 180°로 통일하는 것이 맞다.
- **결함 C2**: 저작 스키마가 두 갈래다. `VALTAN_TRIPLE_COUNTER`는 `{"kind":"BOSS_FORWARD_ARC",...}`,
  `VALTAN_TRASH`는 `{"space":"BOSS_LOCAL",...}`. 같은 문서 안에 필드 이름이 다른 두 형태가 공존한다.
- **결함 C3**: `counterPower`를 가진 스킬이 1개뿐이라 5개 class는 카운터 자체를 테스트할 수 없다.

---

## 2. 실측으로 확정된 원인 목록

아래 표에서 **확정**은 코드/데이터를 직접 읽어 재현 경로까지 확인한 것,
**가설**은 정황은 맞지만 runtime 로그가 있어야 최종 확정되는 것이다.

| # | 증상 | 상태 | 원인 위치 |
|---|---|---|---|
| A1 | Append reject (loop slot) | 확정 | `ActionCompositionWorkbench.cpp:2189` + `Valtan.presentation.json` PART_BREAK `playMs 0 / repeatUntilStageEnd true` |
| A2 | Append reject (WAIT), 그런데 화면엔 ACTIVE | 확정 | `ActionCompositionWorkbench.cpp:2157` vs 라벨 `:3712`, `BalanceTool.cpp:826`이 WAIT의 stageKind를 ACTIVE로 저장 |
| A3 | Replace도 4-clip HOLD를 거부 | 확정 | `ActionCompositionWorkbench.cpp` 호출자의 `3u == Selected->Clips.size()` 상수. 시퀀스는 4 clip 6983ms |
| A4 | WAIT -> ACTIVE 승격 경로 없음 | 확정 | `BalanceTool.cpp:1620` `stageKindEditable/animationEditable/hitEditable` 모두 false |
| B | 마력구에서 너무 높이 뜬다 | 확정 | `Data/Valtan/Valtan.gameplay.json:2411` `verticalOffsetM: 3.0` |
| C | 마력구 1000딜 성공/실패 뒤 안 내려온다 | 확정 | `ValtanBrain.cpp:1743` apply는 `BeginPattern`, restore는 `FinishPattern`뿐. Stage 경계 restore 없음 |
| D | 가운데 검은 마력구가 안 보인다 | 좁힌 가설 | admission 실패 아님(문서·리소스·Multiply pass 전부 실재). 3.2의 D-1~D-4 참조 |
| E | 파운딩 이펙트가 2번만 재생 | 확정 | `EffectV2_Runtime.cpp` `Sync_Stage_Impl`이 `eRepeatPolicy`를 아예 읽지 않는다. binding 2개 = 재생 2회 |
| F | 피자 패턴 사자후 없음 | 확정 | `BOSS_VALTAN.effectv2bindings.json`에 `VALTAN_SIX_PIZZA_106` binding 0개 |
| G | 잡힌 플레이어가 정확히 왼손이 아니다 | 부분확정 | `ClientReplication.cpp:2161` 계약 자체는 맞음. bone 선택/원점 보정/서버 좌표 경합이 남은 변수 |
| H | 잡기 판정 기준을 모르겠다 | 확정 | `VALTAN_TRASH STEP_08` BOX 6.0m×2.5m, `VALTAN_CATCH_BREATH STEP_02` CONE 120°/8.0m. 토글 미노출보다 **S1(300ms만 번쩍임)이 결정적**이다 |
| I | 모아치기가 플레이어 반대로 돈다 | 가설 | `VALTAN_CHARGE`만 `LOCK_NEAREST_ON_START`. 시작 시 `nearestTarget==nullptr`이면 영구히 조준 실패 |
| J | 2페이즈 4방향 진입 시 Server entry failed | **원인 확정** | `world-update.pattern-scheduled-spawn-wave`가 Six Pizza/Struggling의 피해 없는 돌기둥 root를 exact nav 밖이라는 이유로 거절해 room을 fail-close하고, Client가 FIN 뒤 Lobby로 복귀하며 공통 문구를 표시했다 |
| K | 워프 1회 돌진이 2000ms 넘게 지속 | 확정 | leg 2300ms = 재조준 500 + 이동 800 + 후행 gap 1000. gap 동안 body가 계속 보이고 loop clip이 돈다. Client는 이미 stage 시계를 snapshot으로 알고 있어 protocol 변경 없이 닫을 수 있다 |
| L | 침묵 자물쇠 -> 쿨타임 마스크 | 확정 | 자물쇠는 발탄 Effect가 아니라 HUD다. S4/S5 참조. `VALTAN_SILENCE_SLOT`의 V2 binding·effectCue가 0개인 것은 별개 항목 |
| M | 사망 후 stage가 남아 추적 | 확정 | `VALTAN_GHOST_DEATH_AUDITION`은 3667ms 1 Stage. HP가 0이 아니면 종료 후 CHASE 복귀 |
| N | 부활 시 유령이 아니라 본체 | 부분확정 | `Valtan.cpp:3576` 스왑 조건은 `state.iGameplayPhase >= 3`. 실패 시 로그만 남기고 격리 |
| O | 부위 파괴에 recovery/돌/사운드 없음 | 확정 | `VALTAN_PART_BREAK`는 Stage 1개, effectCue 0, soundCue 0. `_start_1/_loop_1/_end_1`이 다 있는데 `_end_1`만 쓴다 |
| P1 | 3연속 카운터 clip이 전부 그로기 | 확정 | `VALTAN_TRIPLE_COUNTER` 7 Stage 전부 `mesh_abn_groggy_1_*` |
| P2 | 카운터 내려치기에 loop가 없다 | 확정 | `Valtan.clipmap`에 `mesh_att_battle_14_loop` 자체가 없는데 `VALTAN_COUNTER STEP_02`가 `_02`를 `LOOP_TO_STAGE_END`로 돌린다 |
| P3 | 3연속 내려치기에 windup이 없다 | 확정 | `VALTAN_THREE`는 `STEP_01/02/03` 전부 ACTIVE, WINDUP Stage 0개 |
| Q | 발탄 사운드 누락 | 확정 | 24개 Pattern의 60여 Stage에 cue 0개. `Sound/Valtan`에 252 wav = **133개 고유 이벤트** 실재(카탈로그 선언은 132). 리소스 공백이 아니라 저작 공백 |
| R | 스킬 콜라이더가 너무 많이 비어 있다 | 확정 | **94개 중 43개 미저작.** Gunslinger 12/12, Slayer 11/11 전부 없음 |
| S1 | 라이브 아레나에서 보스 판정 형태를 읽을 수 없다 | 확정 | `Valtan.cpp:418` — Stage 전체 geometry 아웃라인이 `isPreviewDriven` 전용. 라이브는 hit offset 뒤 **300ms**만 번쩍인다 |
| S2 | `VALTAN_TRIPLE_COUNTER` Workbench admission | **전제 교정** | Pattern은 `manualAuditions`에 없지만 managed non-manual `CorePatternIds`에 들어가 Workbench/Boss Play와 모든 안정 Stage의 Gameplay Details가 열린다. 기존 collider Tune은 canonical에도 허용되고, topology·Add·Remove만 manual 전용이다 |
| S3 | 돌 소리를 cue로 붙일 수 없다 | 확정 | `G_Voltan2_Attack09_ProjCreat1`(disk 4)·`ProjExp2`(disk 1)가 `CharacterSoundCatalog.json`에 미선언. `ValtanPatternSoundCueDocument.cpp:415`가 문서 전체를 거부한다 |
| S4 | 침묵 중 R만 잠긴 것처럼 보인다 | 확정 | Server는 `GameRoom.cpp:2751/2791/3151/3181` 네 곳에서 skillId를 보지 않고 **모든 스킬**을 막는데 HUD에는 `Skill_R_SilenceMask` 하나뿐이다 |
| S5 | `Skill_R_SilenceMask`가 자물쇠다 | 확정 | `HUD_Layout.json` 해당 slot의 `layers[0].path = UI/ItemUpgrade/buildup_lock_icon.png` |

---

## 3. 항목별 상세

### 3.1 (B/C) 마력구 = `VALTAN_STAGGER_SLOT`

구조 실측.

```
CHANNEL      12000ms  bossResponse ACCUMULATED_HEALTH_DAMAGE threshold 1000
             branch HEALTH_DAMAGE_THRESHOLD_REACHED -> nextPatternId VALTAN_GROGGY_FOLLOWUP
             branch TIMEOUT                         -> nextActionId  ...final-attack
             anim  mesh_att_battle_17_start 2000ms + mesh_att_battle_17_loop (LOOP_TO_STAGE_END)
FINAL_ATTACK  3000ms  CIRCLE r=100m, hitDelay 2900ms, damage.valtan.omnidirectional-wipe-130
             anim  mesh_att_battle_17_end 3000ms
```

부양은 **Pattern 단위** 필드다.

```
Data/Valtan/Valtan.gameplay.json           "verticalOffsetM": 3.0     <- 저작 정본
  -> Publish-GameplayBalance.ps1:1703~1716                            <- 검증/publish (0 금지, |v|<=100)
  -> row tag는 PATTERNVERTICALOFFSET  (Publish-GameplayBalance.ps1:1713)
  -> Server GameplayCatalog.cpp:2240 "PATTERNVERTICALOFFSET" -> BOSS_PATTERN_DEFINITION::fVerticalOffsetM
  -> ValtanBrain.cpp:1743  BeginPattern:   fPatternVerticalBaseY = Y; Y += 3.0; applied = true
  -> ValtanBrain.cpp:592   RestorePatternVerticalOffset()  호출처는 FinishPattern / 사망 / abort 뿐
```

`Data/Animation/RootMotion/Valtan.rootmotion.json`의 `VALTAN_STAGGER_SLOT` 두 Stage는
`up` 샘플이 전부 `0.0`이다. 즉 높이는 100% 이 `verticalOffsetM` 하나에서 나온다.

`CHANNEL -> FINAL_ATTACK`은 같은 Pattern 안의 Stage 전이라 `FinishPattern`을 거치지 않는다.
따라서 **1000딜을 못 넣고 공격 시퀀스로 들어가면 3m 공중에서 전멸기를 쓴다.**
1000딜 성공은 `nextPatternId`(cross-pattern)라 `FinishPattern`을 타므로 그쪽만 우연히 내려온다.

**해결 방향**: 부양을 Pattern 단위에서 **Stage 단위**로 승격한다.

- `Valtan.gameplay.json` stage에 optional `verticalOffsetM`를 추가하고 `CHANNEL`에만 `0.5`를 준다.
  Pattern 단위 `verticalOffsetM`는 하위호환으로 유지하되 발탄 문서에서는 제거한다.
- publisher에 `PATTERNSTAGEVERTICAL` row를 추가한다.
- `ValtanBrain`의 Stage 진입/이탈 경로에서 apply/restore를 수행한다.
  즉 `CHANNEL` 진입 시 +0.5, `CHANNEL` 이탈(1000딜 성공/TIMEOUT 무관) 시 baseline 복귀.
- 기존 `BeginPattern`/`FinishPattern` 경로는 Pattern-scope 필드용으로 남긴다.

하드코딩 3.0을 기대하는 곳을 같은 변경 단위에서 전부 고쳐야 한다. **C++ 계약 테스트가 포함된다.**

```
Tools/ValtanPipeline/test_valtan_status_pattern_contract.py:90
Tools/ValtanPipeline/validate_valtan_requested_pattern_coverage.py:360
Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py:1529
Server/Private/ServerGameplayContractTests.cpp:4464   "PATTERNVERTICALOFFSET\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t3"
Server/Private/ServerGameplayContractTests.cpp:4468   같은 row의 "\t0" 거부 케이스
Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json
      -> verticalOffsetM은 ValtanEncounter provenance 항목이다. Pattern -> Stage scope 이동은
         Update-BalanceProvenanceReceipt.ps1 동기화가 필요하고, 누락하면
         Publish-GameplayBalance.ps1의 "Balance provenance coverage count mismatch"로 실패한다.
```

**이 G는 bootstrap format version bump를 동반한다.** 새 row tag(`PATTERNSTAGEVERTICALOFFSET`)를
추가하면 `GameplayCatalog.cpp`의 미지 row 처리가 `"Unknown gameplay bootstrap row kind"`로 hard reject하고,
Client의 `ValtanPresentationGenerationAdmission.cpp:230`도 같은 파일의 version을 검사한다. 따라서
`Shared/Public/GameplayDataRevision.h`의 `GAMEPLAY_BOOTSTRAP_FORMAT_VERSION` **30 -> 31**과
Server/Client/publisher를 **하나의 원자 커밋**으로 올려야 하며, 이 G는 순서상 맨 앞이나 맨 뒤에 둔다.
중간에 끼면 그 전후 커밋의 바이너리가 전부 bootstrap을 거부한다.

### 3.2 (D) 가운데 검은 마력구

binding은 이미 있다.

```
VALTAN_STAGGER_SLOT / CHANNEL
  GROUP boss.valtan.magicball, basis STAGE, startMs 0, ONCE, anchor b_effectroot, FOLLOW_SLOT, NATURAL
```

`Data/Effects/V2/Groups/boss.valtan.magicball.effectv2group.json` children:
`boss.valtan.egg.aura_1`, `egg.aura_2`, `egg.black_3`, `egg.cyan_1`. 검은 구는 `egg.black_3`다.

**"admission이 실패했다"는 최초 가설은 실측으로 기각됐다.** 다음이 전부 실재한다.

```
Data/Effects/V2/Authored/boss.valtan.egg.black_3.effectv2.json      존재
Client/Bin/Resources/Effect/Valtan/Meshes/FX_SM_00/fm_h_sphere_01_1.wmodel      81,804 bytes
Client/Bin/Resources/Effect/Valtan/Textures/FX_TEX_02/fx_d_trail_002_cl.dds     32,896 bytes
Shader_EffectMeshV2.hlsl -> EFFECT_V2_PASSES: pass 5 = MultiplyDepth (실재)
```

문서 실측값.

```json
"effectType": "Mesh", "blend": "Multiply", "billboard": true, "depthTest": true,
"loop": false, "lifetime": 10.0, "meshPreScale": 0.01,
"colorMul":    [0.0, 0.0, 0.0, 1.0],
"colorMulEnd": [1.0, 1.0, 1.0, 0.0],  "colorMulLerp": false,
"scale":    { "start": [1.6, 1.6, 1.6] },
"position": { "start": [0.0, 1.5, 3.0] }
```

블렌드 수식도 정상이다.

```
BS_EffectV2Multiply : SrcBlend = Dest_Color, DestBlend = Inv_Src_Alpha, Add
PS_EFFECT_V2_MULTIPLY : src.rgb *= src.a
=> result = (0 * dst) + dst * (1 - src.a) = dst * (1 - src.a)
```

즉 알파가 있는 곳을 **정확히 검게 곱해 내리는** 올바른 검은 구다. 렌더 경로 자체는 맞다.

남은 후보는 네 개이고, 전부 숫자로 확인 가능하다.

- **D-1 (가장 유력) 단위 불일치**. 이 월드는 센티미터 기준이다
  (`CValtan::Draw_PatternHitAreaDebug`의 `constexpr f32_t METERS_TO_UNITS = 100.f`).
  그런데 `position.start`는 `[0, 1.5, 3.0]`이다. 미터라면 앞 3m, 센티미터라면 **앞 3cm**다.
  후자면 구가 발탄 몸 원점에 파묻히고 `depthTest: true`라 body에 가려 안 보인다.
  같은 group의 `cyan_1`(Additive)은 가려져도 밝게 새어 나오지만, Multiply는 가려지면 완전히 사라진다.
  이것이 "시안/오라는 보이는데 검은 것만 안 보인다"와 정확히 일치한다.
- **D-2 lifetime**. `lifetime 10.0s, loop false`인데 `CHANNEL`은 12000ms다. 마지막 2초는 어차피 사라진다.
- **D-3 draw order**. group child 순서는 `aura_1, aura_2, black_3, cyan_1`이고 넷 다 `startMs 0`이다.
  BLEND 렌더 그룹의 실제 정렬은 같은 깊이에서 결정적이지 않다. Additive cyan이 뒤에 그려지면
  검은 구를 덮어쓴다.
- **D-4 `black_1` / `black_2`와의 차이**. `black_1`은 `blend: Opaque`, `colorMul: [-10,-10,-10,5]`이고
  `black_2`는 `black_3`과 완전히 동일하다. group은 `black_3` 하나만 쓴다.
  원본이 어느 것을 썼는지 대조가 필요하다.

**진단 단계(코드 수정 전)**: `CEffectV2Runtime::Last_Error()`와 `Report`를 F1 진단 패널에 노출하고,
`egg.black_3`의 최종 world 행렬 translation을 한 프레임 덤프해 발탄 root와의 거리(cm)를 찍는다.
3cm면 D-1 확정이고 수정은 `position.start`를 `[0, 150, 300]`으로 올리는 데이터 수정이다.
원인이 확정되기 전에 group 내용을 임의로 바꾸지 않는다.

### 3.3 (E) 발악패턴 파운딩 루프 — V2 runtime이 EACH_LOOP를 구현하지 않는다

이것이 이번 조사에서 가장 파급이 큰 발견이다.

`Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`은 `repeatPolicy`와
`clock.basis`를 저장하고, `CEffectV2Catalog`(`EffectV2_Catalog.cpp:413`)는 값이
`ONCE|EACH_LOOP`, `STAGE|CLIP_OCCURRENCE`인지 **검증까지 한다.**
그런데 `Client/Private/EffectV2_Runtime.cpp` 전체에 `EACH_LOOP` 문자열도,
`eRepeatPolicy` 참조도 **하나도 없다.**

`Sync_Stage_Impl`의 실제 동작은 이렇다.

```cpp
for (PENDING_SPAWN& Pending : State.StagePending)
    if (!Pending.bSpawned && fAgeSeconds >= Ms_ToSeconds(Pending.Binding.iStartMs))
        Spawn_OnTarget(...);          // bSpawned = true, 이후 영구히 재생 안 함
```

`bSpawned`는 Stage가 바뀌거나 age가 뒤로 갈 때만 리셋된다.

`VALTAN_STRUGGLING / STEP_06`의 파운딩 binding은 정확히 2개(`startMs` 200, 400)이고
둘 다 `EACH_LOOP`이다. **binding 2개 = 재생 2회.** 사용자가 본 그대로다.

부가 결함 2개가 같은 함수에 있다.

- `clock.basis == CLIP_OCCURRENCE`인 binding도 `Sync_Stage_Impl`이 **Stage 시계로** 처리한다.
  `Populate_BindingConvenience`(`EffectV2_Document.cpp:390`)는 `strClip = clipOccurrenceId`를
  채우지만, 이 값을 쓰는 `Notify_Clip`은 `Binding.strStage.empty()`인 binding만 확장한다.
  발탄 binding 42개는 전부 `actionId`가 있어 `strStage`가 비지 않으므로 **`Notify_Clip` 경로에
  발탄 binding은 단 하나도 들어가지 않는다.** 게다가 `Notify_Clip`이 비교하는
  `State.strClip`은 clip **이름**이고 binding의 `strClip`은 clip **occurrence ID**라
  타입 자체가 어긋나 있다.
- 따라서 `stopPolicy: CLIP_OCCURRENCE_END`도 실제로는 Stage 종료로 동작한다.

**해결 방향**: 이미 존재하는 V1 쪽 스캐너 계약을 그대로 이식한다.
`Client/Public/Valtan.h`의 `Resolve_ValtanPatternEffectOccurrenceScan`은
`bEachLoop`, `fLoopWallDurationSeconds`, `fPlaybackRate`, late-join catch-up까지 이미 푼 코드다.

- `PENDING_SPAWN`의 `bSpawned` 단일 bool을 `iNextLoopEpoch`로 대체한다.
- `Sync_Stage_Impl`에 현재 Stage의 clip occurrence wall map을 전달해
  `CLIP_OCCURRENCE` basis binding의 시작 시각과 loop 주기를 해석한다.
- `EACH_LOOP`이면 loop epoch마다 1회 spawn, `ONCE`면 epoch 0만 spawn.
- `EACH_LOOP`인데 대상 occurrence가 loop가 아니면 V1과 동일하게 fail-closed(무시)한다.

이 하나의 수정이 F(피자 사자후), K(워프), O(부위 파괴) 저작을 실제로 가능하게 만든다.

**다만 G02만으로 파운딩이 "계속" 재생되지는 않는다.** `EACH_LOOP`이 구현돼도 재생 횟수는
`floor(stageMs / nativeLoopMs)`다. `VALTAN_STRUGGLING / STEP_06`은 3000ms이고
`mesh_att_battle_19_03`의 원본 cut은 **3.200s**(`Valtan.clipcuts` 420624 seq=1)다. 런타임 native
길이가 원본 cut과 같다면 epoch은 1회뿐이라 **binding 2개 = 여전히 2회**다. G02 직후
`Resolve_ValtanCompositionNativeClipDurationMs`로 실제 모델 clip 길이를 측정하고,
native가 stage 이상이면 Stage 시계를 늘리거나 binding을 `startMs` 간격으로 명시 추가해야 한다.
5장의 경고와 같은 항목이다.

### 3.4 (F) 피자 패턴 사자후

`VALTAN_SIX_PIZZA_106`에는 V2 binding이 0개다. 사자후 자원은 이미 두 group으로 존재한다.

```
boss.valtan.shout        comet_1 x4 (yaw 0/90/180/270), comet_2 x4 (yaw 45/135/...) 200~900ms
boss.valtan.shout.burst  fog_1..4 + emit_1 + blur_4, 전부 0ms
```

기존 사용 예(그대로 따라가면 된다).

```
VALTAN_BIND_SLOT   STEP_01   shout        CLIP_OCCURRENCE 0ms   ONCE
VALTAN_BIND_SLOT   RECOVERY  shout.burst  CLIP_OCCURRENCE 0ms   ONCE
VALTAN_ROAR_CHARGE STEP_02   shout        CLIP_OCCURRENCE 0ms   ONCE
VALTAN_ROAR_CHARGE STEP_03   shout.burst  CLIP_OCCURRENCE 733ms ONCE
```

피자 패턴에서 사자후에 해당하는 Stage는 `STEP_06`(8000ms, `mesh_att_battle_12_06`, LOOP)와
그 직후 `STEP_07`이다. `STEP_06`에 `boss.valtan.shout`, `STEP_07`에 `boss.valtan.shout.burst`를
`VALTAN_ROAR_CHARGE`와 같은 형태로 추가한다. `STEP_06`이 LOOP Stage이므로 `EACH_LOOP`을
쓰려면 G02(3.3)가 먼저 끝나야 한다. 그 전에는 `ONCE`로 넣는다.

### 3.5 (G/H) 뒤돌아잡기 후 날리기 — 왼손 부착과 콜라이더

**사용자가 확정한 계약**: 보스에게 잡힌 경우 플레이어는 자신의 월드 행렬을 offset 행렬로
변환하고, 발탄의 왼손 뼈 행렬을 가져와 offset 행렬과 곱해 플레이어를 발탄의 손 위치로 옮긴다.

현재 코드는 이미 그 계약이다(`Client/Public/ClientReplication.h:133`).

```cpp
Build_LocalOffset(playerWorld, handWorld, out):
    playerBasis.r[3] = (0,0,0,1);  handBasis.r[3] = (0,0,0,1);
    local = playerBasis * inverse(handBasis);
    local.r[3] = (0,0,0,1);        // 평행이동 성분 제거 -> 회전/스케일만 남는다
Compose_World(local, handWorld, out):
    out = local * handWorld;       // 결과 위치 = handWorld의 위치 = 손 뼈 원점
```

`ClientReplication.cpp:2161`

```cpp
handWorld = body->Get_BoneMatrix("bip001-l-hand") * presentationRoot;
presentationRoot = m_pBodyVisualRootCom->World * m_pTransformCom->World;
```

`CBone::Update_CombinedTransformationMatrix`는 루트 뼈에서 `m_PreTransformMatrix`를 이미
곱하므로 `Get_BoneMatrix`는 PreTransform이 반영된 model-space 행렬이다. 즉 수식은 맞고
**"발탄 기준 월드 오프셋이 잘못 끼어들어 있다"는 가설은 코드상 성립하지 않는다.**
`local.r[3]`을 0으로 지우므로 결과 위치는 정확히 손 뼈 원점이다.

그렇다면 남은 변수는 셋이다.

1. **뼈 선택**. `bip001-l-hand`가 손목인지 손바닥인지. 잡는 위치는 보통 손바닥/손가락 쪽이다.
2. **플레이어 모델 원점**. 캐릭터 원점은 발바닥이다. 발바닥을 손 뼈 원점에 맞추면 몸이
   손 위로 뜨거나 아래로 처져 보인다. 이 계약에는 **손 기준 보정 translation이 없다.**
3. **Server 좌표 경합**. 잡힌 동안 Server가 보내는 player transform과 Client의 부착이
   같은 프레임에서 어느 쪽이 마지막에 쓰이는가.

**해결 방향**

- `attachmentSlot`에 optional `gripLocalOffset {forwardM, upM, rightM}`을 추가하고
  `Compose_World` 앞에 손 기준 보정 translation을 곱한다. 값은 Workbench Details에서 조절한다.
  (사용자가 확정한 "offset × 손 행렬" 계약은 그대로 두고, 그 offset에 저작 가능한
  평행이동 성분만 되돌려 주는 것이다.)
- `bip001-l-hand`의 자식 뼈 목록을 F1 진단에 덤프해 실제 손바닥 뼈를 고른다.
- 잡기 판정 콜라이더를 화면에 띄운다. `CValtan::Draw_PatternHitAreaDebug`
  (`Valtan.cpp:418`)는 이미 CIRCLE/RING/CONE/BOX/CROSS와 counterProxy까지 그린다.
  문제는 토글이다. `m_isPatternHitAreaDebugVisible`은 `Set_SkillHitAreaDebugVisible`
  (`Show Skill Hit Areas` 체크박스)에 묶여 있고, 그 체크박스는 **Character Select 화면에만**
  있다(`Level_CharacterSelect.cpp:1687`). Valtan Arena에는 없다. G01에서 전역 토글로 올린다.

잡기 판정 실측값.

```
VALTAN_TRASH        STEP_08 / RETRY_RUSH_02 / RETRY_RUSH_03
    BOX  length 6.0m  halfWidth 2.5m   offsets 0,100,200,300,400,500,600ms  (667ms 동안 7회 스윕)
    playerResponse CAPTURE   attachmentSlot BOSS_LEFT_HAND
VALTAN_CATCH_BREATH STEP_02
    CONE angle 120°  length 8.0m       offset 250ms (1회)
    playerResponse CAPTURE   attachmentSlot BOSS_LEFT_HAND
```

**왜 "잡히는 기준을 아예 모르겠는가"의 진짜 원인.** 토글이 없는 것보다 이쪽이 결정적이다.
`CClientReplication`의 기본값은 `m_isSkillHitAreaDebugVisible = true`
(`ClientReplication.h:665`)이고 spawn 시 `valtan->Set_PatternHitAreaDebugVisible(...)`로 전달된다
(`ClientReplication.cpp:1650`). 즉 **발탄 pattern hit wire는 이미 켜져 있다.** 그런데
`Draw_PatternHitAreaDebug`(`Valtan.cpp:418`)는 라이브 아레나에서 이렇게 동작한다.

```cpp
constexpr f32_t MIN_VISIBLE_HIT_WINDOW_MS = 300.f;
for (각 hit offset)
    if (fAgeMs >= fTickMs && fAgeMs <= fTickMs + 300.f) { isHitWindow = true; break; }

const bool_t isAuthoringGeometryWindow =
    isPreviewDriven &&                       // <-- Animation Tool preview 전용
    fAgeMs >= 0.f && fAgeMs <= area.iStageDurationMs;

if (!isHitWindow && !isAuthoringGeometryWindow && !area.bHasCounterProxy)
    return;                                  // <-- 그 외에는 아무것도 안 그린다
```

Stage 전체 구간을 덮는 amber geometry 아웃라인은 `isPreviewDriven`일 때만 그려진다. 라이브
아레나에서는 **각 hit offset 뒤 300ms만 분홍 wire가 번쩍이고 사라진다.**
`VALTAN_TRASH STEP_08`은 전체가 667ms이므로 사람이 형태를 읽을 시간이 없다.
`m_isCombatColliderDebugVisible`은 기본 `false`(`ClientReplication.h:664`)라 body collider는
아예 꺼져 있고, 두 체크박스는 Character Select 화면에만 있다(`Level_CharacterSelect.cpp:1679/1687`).

즉 G01의 핵심은 "토글을 노출"이 아니라 **라이브 아레나에서도 Stage 전체 구간 geometry를 유지**하는
것이다.

### 3.6 (I) 모아치기 방향

```
VALTAN_CHARGE    targetPolicy LOCK_NEAREST_ON_START   aimPolicy TRACK_TARGET_EACH_TICK
VALTAN_CHARGE_2  targetPolicy NEAREST_EACH_TICK       aimPolicy TRACK_TARGET_EACH_TICK
VALTAN_SIX_PIZZA targetPolicy LOCK_RANDOM_ALIVE_ON_START
```

`BeginPatternTargetAndAim`(`ValtanBrain.cpp:709`)

```cpp
case LOCK_NEAREST_ON_START: selected = nearestTarget;                       // 호출자가 준 포인터
case LOCK_RANDOM_ALIVE_ON_START: selected = SelectRandomTarget(players,...) // players 맵에서 직접
```

`UpdatePatternTargetAndAim`은 `NEAREST_EACH_TICK`일 때만 대상을 재획득한다.
따라서 `LOCK_NEAREST_ON_START`는 **시작 프레임에 `nearestTarget`이 null이면 패턴 내내
`iPatternTargetEntityId`가 INVALID로 남아 한 번도 회전하지 않는다.** 직전 yaw를 그대로 유지하면
플레이어 기준으로는 "엉뚱한/반대 방향"으로 보인다. 피자가 정상인 이유는 `players` 맵에서
직접 뽑기 때문이다.

**해결 방향(3단)**

1. 진단 먼저: F1 진단에 `bossYaw / targetYaw / iPatternTargetEntityId`를 tick 로그로 남겨
   "회전을 안 하는 것"인지 "반대로 회전하는 것"인지 구분한다. 후자면 원인은 clip 쪽이고
   아래 2/3은 오답이다.
2. 코드: `LOCK_NEAREST_ON_START`가 `nearestTarget==nullptr`일 때 `players`에서
   `IsEngageablePlayer` 최근접을 직접 재탐색하도록 fallback을 넣는다. 이 정책을 쓰는 다른
   패턴도 같은 함정을 갖고 있다.
3. 데이터: `VALTAN_CHARGE`의 `targetPolicy`를 `NEAREST_EACH_TICK`으로 바꾼다
   (`VALTAN_CHARGE_2`와 동일). "플레이어 따라가게"라는 요구와 일치한다.

### 3.7 (K) 워프 패턴

```
STEP_01                       2000ms  mesh_att_battle_18_01
STEP_02..STEP_09 (8 legs) 각  2300ms  motion PORTAL_TARGET_RUSH
                                       retargetDelayMs 500 / speed 20.0 / distance 16.0
                                       => 재조준 500 + 이동 800 + 후행 gap 1000
                                       anim mesh_att_battle_18_02  LOOP_TO_STAGE_END
                                       hit BOX 8.0m x 2.5m
STEP_10                       1667ms  mesh_att_battle_18_03-2
```

원작은 `끝 사거리 도달 -> 즉시 소멸 -> 약 0.5초 뒤 포탈 생성 -> 다음 돌진`이다.
지금은 이동이 1300ms에 끝나는데도 body가 계속 보이고 loop clip이 남은 1000ms를 채운다.

**해결 방향**

- Stage에 optional `bodyVisibility { hiddenFromMs, hiddenToMs }`를 추가한다.
  leg마다 1300ms부터 Stage 끝까지 body를 숨긴다.
- 숨김 표현은 이미 있는 `CValtan::m_isGhostPresentationHidden` 경로를 재사용한다
  (`Valtan.cpp:3002` — `Late_Update` 제출 자체를 막고 `CEffectV2Runtime::Set_Ignored`를 건다).
  같은 경로를 typed `bodyVisibility`로 일반화하고 Server snapshot 필드로 복제한다.
- `trailingGapMs`를 1000 -> 500으로 줄이고 포탈 Effect를 gap 시작 시점에 건다.
  Workbench의 `Portal Gap After Rush` 컨트롤이 이미 이 값을 소유한다.

### 3.8 (M/N) 사망 이후 stage, 부활 시 유령

```
VALTAN_GHOST_DEATH_AUDITION    STEP_01  3667ms  mesh_dead_1     (branch 없음, next 없음)
VALTAN_GHOST_RESPAWN_AUDITION  STEP_01  3000ms  mesh_respawn_1
                                        ENTER: SET_GAMEPLAY_PHASE gameplayPhase 3
```

- **M**: 사망 audition은 HP를 0으로 만들지 않는다. 3667ms가 지나면 `FinishPattern`으로
  정상 종료되고 brain은 CHASE로 돌아가 플레이어를 따라간다. 해결은
  `VALTAN_GHOST_DEATH_AUDITION` STEP_01 EXIT에 typed 사망 hold 이벤트를 추가해
  brain이 pattern 선택과 추적을 멈추고 IDLE hold로 들어가게 하는 것이다.
  기존 `SET_BOSS_FLAG` 계약을 확장할지 새 event kind를 만들지는 디테일 계획서에서 정한다.
- **N**: Client 스왑 조건은 `Valtan.cpp:3576`

  ```cpp
  DesiredPresentationArchetype = state.iGameplayPhase >= 3 ? "BOSS_VALTAN_GHOST" : "BOSS_VALTAN";
  if (m_strPresentationPartArchetypeId != Desired) Replace_PresentationPartGroup(Desired, status);
  ```
  실패하면 `OutputDebugStringA`로 격리 로그만 남기고 본체 모델을 유지한다.
  따라서 원인은 (a) `iGameplayPhase`가 3으로 복제되지 않았거나 (b) `BOSS_VALTAN_GHOST`
  part group admission 실패다. 둘 다 로그가 남으므로 **먼저 로그를 확인한 뒤 고친다.**
  `CLAUDE.md`가 요구하는 gameplay bootstrap v26 + protocol v51 동시 배포 여부도 함께 본다.

### 3.9 (P) 3연속 계열 패턴 구조

사용자 지적이 데이터와 정확히 일치한다.

| Pattern | 실제 구조 | 문제 |
|---|---|---|
| `VALTAN_THREE` (3연속 내려치기) | `STEP_01/02/03`, clip `mesh_att_battle_2_01/02/03`, 판정은 STEP_03의 CONE 75°/15m 1회 | windup Stage 없음. 3번 내려치는데 판정이 1번 |
| `VALTAN_COUNTER` (카운터 쳐야 하는 내려치기) | `STEP_01` 2000ms WINDUP -> `STEP_02` 1800ms WINDUP(카운터 창) -> `STEP_03` CIRCLE 12m | **`Valtan.clipmap`에 `mesh_att_battle_14_*`는 `_01/_02/_03/_04-1/_04-2` 다섯 개뿐이고 `_loop` clip이 없다.** 그런데 STEP_02가 `mesh_att_battle_14_02`를 `LOOP_TO_STAGE_END`로 재생한다. 즉 one-shot clip을 1800ms 동안 반복 재생해 끊기는 것이다. 사용자 관찰 "loop가 없다"가 정확하다 |
| `VALTAN_TRIPLE_COUNTER` (3연속 - 카운터) | `COUNTER_1/FAIL_1/COUNTER_2/FAIL_2/COUNTER_3/FAIL_3/RECOVERY` 7 Stage 전부 `mesh_abn_groggy_1_*` | **clip이 전부 그로기 clip이다.** 내려치기 애니메이션이 하나도 없다 |

`VALTAN_TRIPLE_COUNTER`가 "3번 연속 재생되는 시퀀스로 구성돼 있는데 뭔지 모르겠다"의 정체는
이것이다. `mesh_abn_groggy_1_start / _loop / _end`만으로 7 Stage를 채워 놓은 자리표시자다.
`VALTAN_THREE`의 `mesh_att_battle_2_01/02/03`을 카운터 창 3회 구조에 재배치하고,
`FAIL_n`에 실제 내려치기 판정 clip을 넣어야 한다.

### 3.10 (O) 부위 파괴

현재: Stage 1개(`PART_BREAK`, 1400ms, `mesh_dmg_parts_end_1` LOOP), effectCue 0, soundCue 0,
hit NONE. 사용자가 원하는 것은 `부위 파괴 애니메이션 -> recovery 시퀀스`와
`4방향 돌 생성 및 파괴 사운드`다.

**"recovery 시퀀스"의 정답은 이미 원본에 있다.** `Valtan.clipmap`에 세 clip이 전부 존재한다.

```
mesh_dmg_parts_start_1   1400ms   부위 파괴 피격
mesh_dmg_parts_loop_1     400ms   유지
mesh_dmg_parts_end_1     2850ms   회복(= recovery)
mesh_idle_battle_1       2333ms   전투 idle 복귀
```

그런데 현재 제품 문서는 **`mesh_dmg_parts_end_1` 하나만** 쓰면서 Stage 시계를 1400ms(=`_start`의
길이)로 잡아 두었다. 즉 지금은 회복 clip을 1400ms만 잘라 loop로 돌리고 있고,
start/loop 단계와 idle 복귀가 통째로 빠져 있다. 이것이 "부위 파괴 애니메이션 이후 recovery
시퀀스까지 넣어서 부위 파괴 구현"의 실체다.

- 애니메이션: 0.4의 절차로 4 clip 시퀀스(6983ms)를 exact slot으로 넣는다.
  Stage를 둘로 나눌 경우 `PART_BREAK`(start+loop, 1800ms) / `PART_BREAK_RECOVERY`(end+idle, 5183ms)로
  자르면 Effect/Sound cue를 회복 구간에만 붙일 수 있다.
- 4방향 돌: `Valtan.combatobjects.json`에 이미 `combatobject.valtan.ground-roar.rock`
  (`FIXED_AREA`, lifetime 5000, hits `[]`, presentationEvent `pulse.valtan.ground-roar.rock.explode`
  at 5000ms)이 있다. **재사용할 저작 템플릿은 `VALTAN_CROSS`가 아니다.** `VALTAN_CROSS`는 Stage 1개
  (`STEP_01` 3000ms, hit NONE)에 `events`가 **0개**다. 실제 4방향 radial volley를 이미 소유한 것은
  `VALTAN_GROUND_ROAR / STEP_01`의 `valtan.ground-roar.cardinal-rocks` 이벤트다.

  ```json
  { "eventId": "valtan.ground-roar.cardinal-rocks", "trigger": "ENTER",
    "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
    "combatObjectArchetypeId": "combatobject.valtan.ground-roar.rock",
    "volleyPolicy": "BOSS_RELATIVE", "countPerResolvedTarget": 4,
    "layout": { "kind": "RADIAL_AROUND_BOSS", "radiusM": 4.9497475,
                "startAngleDegrees": 45.0, "angleStepDegrees": 90.0,
                "mappingBasis": "PROJECT_TUNED" },
    "spawnSchedule": { "kind": "INTERVAL", "count": 1, "firstOffsetMs": 0, "intervalMs": 0 },
    "arenaRandom": { "kind": "NONE" }, "allowOverlap": false, "maximumTotalObjects": 4 }
  ```

  이 블록을 `VALTAN_PART_BREAK`의 회복 Stage `ENTER`에 그대로 복사하고 `eventId`만 바꾼다.
- 사운드: 3.12 참조.

### 3.11 (L) 침묵 패턴

```
VALTAN_SILENCE_SLOT
  STEP_01        2633ms  mesh_evt1_att_battle_5_01_end
  SILENCE_APPLY   100ms  ENTER: SET_PLAYER_SILENCE durationMs 5000
```

V2 binding 0개, effectCue 0개. 발탄 쪽 Effect에는 자물쇠 표현이 없다. **자물쇠는 HUD에 있다.**

```
Data/UI/HUD/HUD_Layout.json  slot "Skill_R_SilenceMask"
  layers[0].path = "UI/ItemUpgrade/buildup_lock_icon.png"   <- 자물쇠
  layers[0].tint = [1, 1, 1, 1]
Data/UI/HUD/HUD_Layout.json  slot "Skill_Q_Cooldown" (쿨타임 마스크의 정본 형태)
  layers[0].path = "UI/Common/White1x1.png"
  layers[0].tint = [0, 0, 0, 0.5882352941176471]
```

소비자도 이미 살아 있다. `CMainApp`의 `Update_SilenceRSlotMask`
(`Client/Private/MainApp.cpp:343`, 호출 `:1834`)가 `HUD_PLAYER_STATE::iSilenceEndTick`을
`Is_ServerDeadlinePending`로 판정해 `pView->Set_SlotVisible("Skill_R_SilenceMask", bSilenced)`를
호출한다. `CUILayoutRuntime`에는 `Set_SlotVisible`, `Set_SlotTint`, `Set_SlotTintMultiplier`가
모두 있다(`Client/Public/UILayoutRuntime.h:41/43/46`).

**따라서 이 항목은 새 수직 슬라이스가 필요 없다. 지금 바로 데이터 + 20줄 수준의 수정이다.**
(이전 판단 "runtime factory/입력 라우터가 닫히기 전에는 완료로 기록할 수 없다"는 실측으로 기각됐다.
display-only HUD image widget과 `Set_SlotVisible`/`Set_SlotTint`는 이미 제품 경로다.)

여기서 함께 드러난 실제 결함이 하나 더 있다. **침묵은 R만 막지 않는다.**

```
Server/Private/GameRoom.cpp:2751   pending SKILL 커밋 차단
Server/Private/GameRoom.cpp:2791   C2S_USE_SKILL 차단
Server/Private/GameRoom.cpp:3151   C2S_RELEASE_SKILL 차단
Server/Private/GameRoom.cpp:3181   C2S_UPDATE_SKILL_AIM 차단
```

네 곳 모두 skillId를 보지 않는다. 즉 침묵 중에는 **모든 ACTIVE 슬롯**이 막히는데
HUD는 R 하나에만 자물쇠를 얹고 있다.

### 3.12 (Q) 사운드

`Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json`에 **562개 cue가 이미 있다.**
`Client/Bin/Resources/Sound/Valtan`에는 252개 wav = **133개 고유 이벤트**가 실재한다.
누락은 저작 공백이지 리소스 공백이 아니다.

cue가 하나도 없는 Stage(주요 항목만).

```
VALTAN_PART_BREAK       PART_BREAK
VALTAN_STAGGER_SLOT     CHANNEL, FINAL_ATTACK
VALTAN_BIND_SLOT        STEP_01, RECOVERY
VALTAN_SILENCE_SLOT     STEP_01, SILENCE_APPLY
VALTAN_GROUND_ROAR      STEP_01                      <- 땅구르기 쿵쿵
VALTAN_GROGGY_FOLLOWUP  GROGGY
VALTAN_COUNTER_GROGGY   GROGGY
VALTAN_GHOST_DEATH_AUDITION    STEP_01
VALTAN_GHOST_RESPAWN_AUDITION  STEP_01
VALTAN_GHOST_PORTAL_ONCE       ACTIVE
VALTAN_FIST_IN_OUT      INNER
VALTAN_TRASH            STEP_02, RECHARGE_WAIT_02/03, RETRY_WINDUP_02/03,
                        RETRY_RUSH_02/03, RETRY_MISS_02, RETRY_EXHAUSTED,
                        CATCH_SLAM, EXECUTE_TAIL
VALTAN_SIX_PIZZA_106    STEP_02, STEP_06, STEP_08
VALTAN_TERRAIN_DESTRUCTION  STEP_02, STEP_07, STEP_08, STEP_10
(외 DASH_CHARGE/GROGGY, FOUR_SLASH/WINDUP·RECOVERY, HIGH_JUMP/AIRBORNE·RECOVERY 등)
```

사용자가 지목한 두 건의 후보 이벤트.

- **땅구르기 쿵쿵**: `G_Voltan2_FootStep1`(발구름), 충격 계열 `G_Voltan2_Attack02_Shot1`,
  `G_Voltan2_Attack08_Shot1..5`. `VALTAN_GROUND_ROAR / STEP_01`에 건다.
- **돌 터지는 소리**: `G_Voltan2_Attack09_ProjExp1` / `ProjExp2`, 생성음 `ProjCreat1`.
  이미 `Valtan.combatobjectsoundcues.json`이 `high-jump.target-axe`의 `hitId`에
  `ProjExp1`을 걸어 두었다.

**여기서 계약 공백이 하나 있다.** `Valtan.combatobjectsoundcues.json`의 cue는
`combatObjectArchetypeId + hitId`로만 바인딩된다. 그런데 `ground-roar.rock`은
`hits: []`이고 `presentationEvents[]`의 `pulse.valtan.ground-roar.rock.explode`만 갖는다.
따라서 **"돌 터지는 시점"에 소리를 붙일 수 있는 바인딩 키가 지금 존재하지 않는다.**
`presentationEventId` 바인딩을 스키마에 추가해야 한다(G12).

타이밍은 사용자가 직접 조정하므로, 이 계획은 "정확한 이벤트를 정확한 Stage/occurrence에
연결"까지만 책임지고 `startMs`는 클립 시작 기준 보수적인 값으로 넣는다.

### 3.13 (R) 스킬 콜라이더 공백과 저작 도구

`Data/Animation/HitShapes/`에는 4개 파일뿐이다.

| class | PlayerSkills 정의 | hitshapes 저작 | 미저작 |
|---|---|---|---|
| LANCE_MASTER | 24 | 19 | 5 (34020, 34030, 34000, 34500, 34520) |
| **GUNSLINGER** | 12 | **0** | **12 전부** |
| **SLAYER** | 11 | **0** | **11 전부** |
| WARLORD | 17 | 9 | 8 (17020, 17025, 17170, 17240, 17250, 17800, 17810, 17820) |
| ARTIST | 16 | 12 | 4 (31110, 31050, 31020, 31030) |
| DIMENSIONMASTER | 14 | 11 | 3 (2050010, 2050020, 2050030) |
| **합계** | **94** | **51** | **43** |

`Gunslinger.hitshapes.json`, `Slayer.hitshapes.json` 파일 자체가 없다.
`CLAUDE.md`의 계약대로 셰이프가 없는 스킬은 `maximumRange` 원형 단일 판정으로 떨어진다.
즉 **두 class는 전 스킬이 "원형 1타"로 동작하고 있다.**

`Draw_SkillHitAreaDebug`(`Character.cpp:2108`)가 그리는 것은 Server의
`SKILLHIT/SKILLSTAGEHIT`가 아니라 Client `m_EffectCueDocument.Hits`(animevent HIT cue)다.
따라서 **"디버그 와이어가 보인다"가 "서버 판정이 있다"의 증거가 아니다.** 두 소스를 나란히
그려서 어긋남을 눈으로 잡을 수 있게 해야 한다.

**저작 도구 요구사항(사용자 요청)**

1. Composition Workbench에서 collider 추가 -> Details에서 모양/사이즈 조절 -> Save
2. 정해진 시간 동안 collider type 선택
3. damage collider면 실제 데미지 수치를 아래에서 조절해 저장

현재 구현된 것(`ActionCompositionWorkbench.cpp:6017~`).

- `Add Server Collider` 버튼 (CIRCLE r=5.0 기본값) — 게이트:
  `Pattern.bManualServerAudition && sequenceRole != "WAIT" && Draft.hitEditable`
- `Collider Shape` 콤보: CIRCLE / RING / CONE / BOX / CROSS / SIX_DIRECTIONS
- 기하(outer/inner/angle/length/halfWidth), 스케줄(delay/interval/count/explicit offsets)
- `Damage Profile` 콤보 (ID 선택만)

없는 것.

- **collider type 선택**: damage / capture / counter proxy 구분. 지금은 damage 전용이고
  `playerResponse`, `attachmentSlot`, `counterProxy`는 Workbench에서 편집할 수 없다.
- **damage 수치 편집**: `DamageProfiles.json`의 실제 수치는 Balance Tool에만 있다.
- **활성 구간(window)**: `hitActivation {startMs, lifetimeMs}` 필드가
  `VALTAN_STAGE_VIEW`(`ValtanPatternTree.h:257`)에는 있으나 Workbench Details에 없다.
- **플레이어 스킬 collider 저작**: Valtan Workbench는 보스 전용이다.
  플레이어 hitshapes는 Python 스크립트(`build_hitshapes.py`)로만 생성된다.

### 3.14 (J) "Server entry failed"는 하나의 원인이 아니다

이 문자열의 유일한 생산지는 `Client/Private/Level_Lobby.cpp:89`다.

```cpp
CLIENT_RECOVERY_DIAGNOSTIC recovery{};
if (CLevelTransitionService::Try_ConsumeRecovery(recovery))
{
    m_RecoveryDiagnostic = std::move(recovery);
    m_hasRecoveryDiagnostic = true;
    m_strStatus = "Server entry failed.";     // <-- 24가지 경로가 이 한 줄을 공유한다
}
```

즉 **"TCP가 끊겼다"가 아니라 "무엇인가가 Lobby로 되돌렸다"**이고, 실제 이유는
`CLIENT_RECOVERY_DIAGNOSTIC`의 `eReason / strSource / strDetail`에 보존돼 있는데
Lobby 화면에는 이 한 줄만 찍힌다. 저장소 전체의 `Report_Recovery` source는 24개이며
Valtan 진입/전투 경로에 걸리는 것은 다음 넷이다.

| `strSource` | `eReason` | 실제 의미 | 소켓을 누가 닫았나 |
|---|---|---|---|
| `level-valtan.initialize` | `CLIENT_ACTIVATION_LEVEL_CREATE_FAILED` | Level 초기화 단계 거절. `strDetail`이 거절한 stage 이름을 담는다 | 활성화 실패 경로 |
| `level-valtan.world-destruction-sync` | `CLIENT_REPLICATION_FAILED` | **Client** world-destruction projection이 Server sync를 거부 | **Client가 직접 close** |
| `level-valtan.encounter-prop-sync` | `CLIENT_REPLICATION_FAILED` | **Client** encounter prop presentation 실패 (`m_DeployRuntime.Get_Status()`) | **Client가 직접 close** |
| `level-valtan.network-connection-lost` | (network) | 진짜 TCP 단절 | Server 또는 네트워크 |

**넷 중 셋은 Server가 멀쩡한데 Client가 스스로 끊는 경로다.** 그러므로 "Server 크래시"부터
의심하는 것은 순서가 틀렸다. 먼저 `strSource`를 읽어야 한다.

진단 파일 위치는 실측으로 확정했다.

```
Client   <exe 폴더>\Diagnostics\client-session-<pid>.jsonl   (ClientSessionDiagnostic.cpp:273)
Server   <exe 폴더>\Diagnostics\server-session-<pid>.jsonl   (ServerApp.cpp:92)
```

**패턴 번호에 대한 주의.** 사용자가 말한 "39번"은 이 저장소의 어떤 문서 순서와도 맞지 않는다.

```
Valtan.gameplay.json   patterns[]   42개   -> 1-based 39 = VALTAN_GHOST_RESPAWN_AUDITION
ValtanEncounter.json   patterns[]   65개   -> 1-based 39 = VALTAN_TERRAIN_DESTRUCTION
                                               1-based 35 = VALTAN_SEQUENCE_FOUR "2페이즈 4방향 공격"
Encounter를 gameplay 소유분으로 필터    42개   -> 1-based 39 = VALTAN_GHOST_PORTAL_ONCE
```

displayName "2페이즈 4방향 공격"은 `VALTAN_SEQUENCE_FOUR` 하나뿐이므로 이름을 정본으로 삼되,
**진단에서는 UI 서수를 믿지 말고 JSONL의 `strDetail`과 Server 로그의 `patternId`를 기록한다.**
`VALTAN_SEQUENCE_FOUR` 자체는 Stage 1개(`STEP_01`, 5000ms, CROSS 18.0×2.5,
offsets 1790/2560/3330, `damage.valtan.four-slash`, knockdown 1200ms)이고
`events`/`branches`가 모두 비어 있어 **데이터만으로는 어떤 bounce 경로도 설명하지 못한다.**
`propBreakSetId`를 가진 Stage는 저장소 전체에서 `VALTAN_ARENA_BREAK_109`의 `TAKEOFF`/`DROP`
둘뿐이므로 `encounter-prop-sync` 경로도 이 패턴과는 무관하다.

---

## 4. G 구성

우선순위는 (1) 저작이 막힌 병목, (2) 판정을 눈으로 볼 수 있게 하는 것,
(3) 개별 패턴 결함 순이다. 병목과 가시성을 먼저 뚫지 않으면 나머지를 검증할 수 없다.

### 4.1 1단계 — 저작 병목 (선행 필수)

**G00 · Composition Stage 의미 정리와 Save 병목 제거**

| 항목 | 내용 |
|---|---|
| 파일 | `Client/Private/ActionCompositionWorkbench.cpp`, `Client/Private/ActionCompositionWorkbench_Blueprint.cpp`, `Client/Private/BalanceTool.cpp`, `Client/Public/BalanceTool.h` |
| 변경 1 | 타임라인 Stage 라벨을 `stageId \| sequenceRole`로 바꾼다. WAIT은 `WAIT (blank gap)`으로 명시 |
| 변경 2 | `Pattern Total Duration (ms)`를 읽기 전용 합계로 강등. Stage 자동 생성 제거 |
| 변경 3 | Details에 `Promote WAIT -> ACTIVE` 추가. `sequenceRole=ACTIVE`, `bSuppressAnimation=false`, `animationEndPolicy=EXACT` 전환. `Set_ValtanStageDraft`의 WAIT 불변식 검사를 승격 전용 경로로 분기 |
| 변경 4 | Append reject 문구에 해결 절차를 넣는다. loop slot이면 `Convert loop slot to exact` 버튼을 같은 자리에 제공 |
| 변경 5 | `Stage Role` 콤보가 현재 `stageKind`(예: `PART_BREAK`)를 목록에 없다고 0번(ACTIVE)으로 표시하는 문제 수정. 목록 밖 값은 읽기 전용으로 표시 |
| 변경 6 | HOLD-chain fit 게이트의 `3u == Selected->Clips.size()` 상수(`:2234`)를 푼다. `start/loop/end` + 역할 없는 tail(예: `mesh_idle_battle_1`)을 허용한다. **주의: 함수를 그대로 둘 수는 없다.** 현재 `FitCompositionSequenceCutsToStage`(`:411`)는 N>=3에서 `front()`/`back()`만 고정 edge로 두고 **가운데 전부**를 비례 스케일한다. 4-clip `[start, loop, end, idle]`에 그대로 태우면 `end`(회복 clip)까지 잘린다. `ClipReplacementRole`이 판정한 역할 배열을 함수에 넘겨 **`loop`인 인덱스만 스케일하고 `start`/`end`/역할 없음은 고정**하도록 오버로드를 추가한다. 기존 3-clip 호출자는 동작이 동일하다 |
| 변경 7 | Replace/Append reject 메시지에 **필요한 Stage 시계 ms 숫자**를 넣는다. 지금은 "exceeds the existing Server Stage clock"이라고만 하고 6983 같은 목표값을 알려 주지 않는다 |
| 종료 증거 | Debug 빌드 후 F1 -> Action Composition Workbench에서 `VALTAN_PART_BREAK`에 4 clip 시퀀스를 Replace/Append로 넣고 `Save + Validate + Publish`가 성공, canonical reload까지 통과 |

**G01 · Debug collider 전면 노출**

| 항목 | 내용 |
|---|---|
| 파일 | `Client/Private/MainApp.cpp`(F1 허브), `Client/Private/ClientReplication.cpp`, `Client/Private/Valtan.cpp`, `Client/Private/Character.cpp`, `Client/Private/Npc.cpp` |
| 변경 1 | Character Select 패널(`Level_CharacterSelect.cpp:1679/1687`)에만 있던 `Show Combat Colliders` / `Show Skill Hit Areas`를 **F1 Developer Tools 전역 토글**로 올린다. Valtan Arena / Bern / Development 전부에서 동작 |
| 변경 2 | 토글을 5종으로 분리: `Body Collider` / `Boss Pattern Hit (pulse)` / **`Boss Stage Geometry (persistent)`** / `Counter Proxy` / `Player Skill Hit` |
| 변경 2b | **핵심.** `Draw_PatternHitAreaDebug`의 `isAuthoringGeometryWindow`에 걸린 `isPreviewDriven` 조건을 새 `m_isStageGeometryDebugVisible`로 OR 확장한다. 이것만으로 라이브 아레나에서 Stage 전체 구간 amber 아웃라인이 유지되어 `STEP_08`의 667ms BOX를 눈으로 읽을 수 있다 |
| 변경 3 | `CValtan::Draw_PatternHitAreaDebug`의 CAPTURE Stage를 별색으로 그리고, `stageId / shape / damageProfile / hitOffsets` 텍스트 오버레이를 붙인다 |
| 변경 4 | 플레이어 쪽은 Client animevent 셰이프(현재)와 Server `SKILLHIT` 셰이프를 **다른 색으로 동시에** 그린다. 셰이프가 아예 없는 스킬은 `maximumRange` 원을 회색 점선으로 그려 "폴백 중"임을 드러낸다 |
| 종료 증거 | 사용자가 Valtan Arena에서 F1 토글로 버러지 STEP_08의 BOX 6.0×2.5, STEP_07의 counter proxy 원(앞 1.0m r2.25m), 건슬링어 전 스킬의 회색 폴백 원을 확인 |

**G02 · Effect V2 runtime의 clock/repeat 계약 구현**

| 항목 | 내용 |
|---|---|
| 파일 | `Client/Private/EffectV2_Runtime.cpp`, `Client/Public/EffectV2_Runtime.h` |
| 변경 1 | `PENDING_SPAWN`의 `bSpawned`를 `iNextLoopEpoch`로 대체 |
| 변경 2 | `Sync_Stage_Impl`에 현재 Stage의 clip occurrence wall map을 전달. `CLIP_OCCURRENCE` basis binding의 시작 시각을 occurrence 시작 + `startMs`로 해석 |
| 변경 3 | `EACH_LOOP`이면 occurrence loop 주기마다 1회 spawn. `ONCE`면 epoch 0만. `EACH_LOOP`인데 대상이 loop가 아니면 fail-closed |
| 변경 4 | `stopPolicy CLIP_OCCURRENCE_END`를 실제 occurrence 종료에 연결 |
| 변경 5 | 발탄 binding을 절대 태우지 않는 `Notify_Clip`의 clip-이름/occurrence-ID 불일치를 정리(사용처 없는 분기 제거 또는 NPC 전용으로 명시) |
| 종료 증거 | `VALTAN_STRUGGLING / STEP_06`에서 파운딩이 stage 끝까지 반복. `Valtan.cpp`의 V1 스캐너와 동일한 epoch 수가 나오는지 로그 대조 |

### 4.2 2단계 — 콜라이더/데미지 저작 도구

**G03 · Workbench Collider 저작 확장**

| 항목 | 내용 |
|---|---|
| 파일 | `Client/Private/ActionCompositionWorkbench.cpp`, `Client/Private/BalanceTool.cpp`, `Client/Public/BalanceTool.h`, `Client/Public/ValtanPatternTree.h`, `Client/Private/ValtanPatternTree.cpp` |
| 변경 1 | Collider Type 선택: `DAMAGE` / `CAPTURE` / `COUNTER_PROXY`. `playerResponse`, `attachmentSlot`을 Details에서 편집 |
| 변경 2 | `hitActivation {startMs, lifetimeMs}`를 Details에 노출 (요구사항 "정해진 시간 동안") |
| 변경 3 | counterProxy 편집 UI: kind(FORWARD_ARC/LOCAL_CIRCLE), arcDegrees, forward/right offset, radius |
| 변경 4 | `DAMAGE` 타입일 때 선택된 `serverDamageProfileId`의 **실수치를 같은 패널에서 편집**하고 `Data/Balance/DamageProfiles.json`에 별도 CAS 저장. Pattern source와 하나의 atomic writer라고 표시하지 않는다(Sound cue와 동일한 별도 owner 규칙) |
| 변경 5 | counterProxy 저작 스키마를 `{"kind": ...}` 한 가지로 통일하고 `{"space": "BOSS_LOCAL"}` 레거시는 read-only 수용 후 Save 시 정규화 |
| 종료 증거 | `VALTAN_TRASH / STEP_07`의 proxy를 FORWARD_ARC 180°로 바꿔 저장 -> publish -> Server 재시작 후 랜스마스터 A로 카운터 성립 |

**G04 · 플레이어 스킬 hit shape 43개 공백 해소**

| 항목 | 내용 |
|---|---|
| 파일 | `Data/Animation/HitShapes/{Gunslinger,Slayer}.hitshapes.json`(신규), 나머지 4개 갱신, `Tools/CharacterAnimationIntake/build_hitshapes.py`, `fill_animevents_hit_shapes.py` |
| 변경 1 | Gunslinger/Slayer의 `.animevents` HIT 행과 `skillbindings` 체인으로 hitshapes를 생성. HIT notify가 없는 스킬은 `PlayerSkills.json hitTimeMs` 위치에 skilltiming caster 셰이프를 합성(기존 계약) |
| 변경 2 | 나머지 4 class의 미저작 19개도 같은 절차로 채운다 |
| 변경 3 | `Publish-GameplayBalance.ps1`에 "class별 hitshapes 커버리지" 리포트를 추가. 0%인 class는 경고 |
| 종료 증거 | 94개 중 hitshapes 저작 94개. G01 디버그 와이어에서 회색 폴백 원이 사라진다 |

### 4.3 3단계 — 개별 패턴

**G05 · 마력구 부양을 Pattern scope에서 Stage scope로 내린다**

| 항목 | 내용 |
|---|---|
| 증상 | (B) 3m는 너무 높다 · (C) 1000딜을 못 넣고 FINAL_ATTACK으로 들어가면 공중에서 전멸기를 쓴다 |
| 근본 원인 | `verticalOffsetM`이 **Pattern 필드**다. apply는 `BeginPattern`(`ValtanBrain.cpp:1743`), restore는 `RestorePatternVerticalOffset`(`:592`)뿐이고 그 호출처는 `FinishPattern`/사망/abort(`:1728, :1868, :2479, :3020`)다. `CHANNEL -> FINAL_ATTACK`은 같은 Pattern 안의 Stage 전이라 어느 것도 타지 않는다 |
| 데이터 | `Data/Valtan/Valtan.gameplay.json` — Pattern `verticalOffsetM: 3.0` 제거, `CHANNEL` Stage에 `verticalOffsetM: 0.5` 추가 |
| Server 1 | `Server/Public/GameplayCatalog.h:573` `BOSS_PATTERN_STAGE_DEFINITION`에 `float fVerticalOffsetM = 0.f;` 추가 |
| Server 2 | `Server/Private/GameplayCatalog.cpp` — `PATTERNSTAGEVERTICALOFFSET` row 파서를 `"PATTERNSTAGEACTION"`(`:3173`) 블록과 동일한 owner 해석 절차(encounter -> pattern -> stage)로 추가. 기존 `PATTERNVERTICALOFFSET`(`:2240`)는 하위호환으로 유지 |
| Server 3 | `Server/Private/ValtanBrain.cpp` — `EnterPatternStage`(`:1602`)가 **유일한 Stage 진입 소유자**다. 여기서 이전 Stage offset을 `RestorePatternVerticalOffset`으로 되돌리고 새 Stage의 `fVerticalOffsetM != 0`이면 다시 apply한다. Pattern-scope 경로(`BeginPattern`/`FinishPattern`)는 손대지 않는다 |
| publisher | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` — `:1704~1716`의 Pattern 검증 옆에 Stage 검증(유한, 0 금지, 절댓값 100 이하)과 `PATTERNSTAGEVERTICALOFFSET` row emit 추가. `Get-BootstrapRowSortKey`(`:4608`)는 일반 정렬이라 수정 불필요 |
| 저작 validator | **누락하면 저작 문서가 열리지 않는다.** `Client/Private/ValtanPatternTree.cpp:5700`이 `verticalOffsetM`을 "패턴이 `bossResponse` Stage를 소유할 것"으로 검증한다. Stage scope 필드도 같은 강도로 검증하도록 확장한다 |
| 계약 테스트 | `Server/Private/ServerGameplayContractTests.cpp:4464`의 `PATTERNVERTICALOFFSET / ENCOUNTER_VALTAN / VALTAN_STAGGER_SLOT / 3` 정상 케이스와 `:4468`의 `0` 거부 케이스를 Stage row 쌍으로 추가 |
| Python 계약 | `Tools/ValtanPipeline/test_valtan_status_pattern_contract.py:90`, `validate_valtan_requested_pattern_coverage.py:360`, `author_valtan_phase_two_mechanics.py:1529`가 3.0을 기대한다. 같은 변경 단위에서 고친다 |
| provenance | `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`의 `verticalOffsetM` 항목을 Stage scope로 옮긴다. 누락하면 `Publish-GameplayBalance.ps1`이 `Balance provenance coverage count mismatch`로 실패한다 |
| **버전** | 새 row tag는 `GameplayCatalog.cpp:3947` `"Unknown gameplay bootstrap row kind"`로 hard reject된다. `Shared/Public/GameplayDataRevision.h:20` `GAMEPLAY_BOOTSTRAP_FORMAT_VERSION 30 -> 31`, `Publish-GameplayBalance.ps1:4688`의 `{ 30 }` -> `{ 31 }`, `Client/Private/ValtanPresentationGenerationAdmission.cpp:230` 검사까지 **하나의 원자 커밋**이어야 한다 |
| 순서 | 이 G는 **맨 앞 또는 맨 뒤**에 둔다. 중간에 끼우면 그 전후 커밋의 Server/Client 바이너리가 서로의 bootstrap을 거부한다 |
| 종료 증거 | Debug Next Pattern으로 `VALTAN_STAGGER_SLOT` 재생 → (1) 1000딜 성공 (2) 1000딜 실패 후 TIMEOUT 두 경로 모두에서 `CHANNEL` 이탈 tick에 `fPositionY`가 baseline으로 복귀. Server 로그의 `fPositionY` 전후 값을 기록 |

**G06 · 마력구 가운데 검은 구**

| 항목 | 내용 |
|---|---|
| 상태 | admission 실패 가설은 실측으로 기각. 문서·`fm_h_sphere_01_1.wmodel`(81,804B)·`fx_d_trail_002_cl.dds`(32,896B)·`Shader_EffectMeshV2.hlsl` pass 5 `MultiplyDepth`가 전부 실재하고 블렌드 수식도 정확하다 |
| 진단 1 (코드 수정 없음) | F1 진단 패널에 `CEffectV2Runtime::Last_Error()`와 `Report`를 노출한다. `boss.valtan.egg.black_3` occurrence가 아예 생성되지 않는지, 생성되고 안 보이는지를 먼저 가른다 |
| 진단 2 | 생성됐다면 그 occurrence의 최종 world 행렬 translation과 발탄 root의 거리를 **cm 단위로 한 프레임 덤프**한다 |
| 판정 | 거리가 약 3이면 **D-1 단위 불일치 확정**이다. 이 월드는 cm 기준(`Valtan.cpp`의 `METERS_TO_UNITS = 100.f`)인데 문서는 `position.start: [0.0, 1.5, 3.0]`이다. 3cm면 body 안에 파묻히고 `depthTest: true`라 가려진다. 같은 group의 Additive `cyan_1`은 가려져도 새어 나오지만 Multiply는 가려지면 완전히 사라진다 — "시안은 보이는데 검은 것만 안 보인다"와 정확히 일치 |
| 수정 (D-1 확정 시) | `Data/Effects/V2/Authored/boss.valtan.egg.black_3.effectv2.json`의 `position.start`를 `[0.0, 150.0, 300.0]`으로 |
| 겸사 수정 | `lifetime: 10.0` -> `12.0` (`CHANNEL`이 12000ms라 지금은 마지막 2초가 사라진다) |
| 남은 후보 | D-3 draw order — group child 4개가 전부 `startMs 0`이고 BLEND 그룹의 동일 깊이 정렬은 결정적이지 않다. D-4 — `black_1`(Opaque, `colorMul [-10,-10,-10,5]`)과 `black_2`(`black_3`과 동일)의 존재. 원본 대조 전에는 group 내용을 바꾸지 않는다 |
| 종료 증거 | 사용자가 마력구 중앙의 검은 구를 육안 확인. **에이전트는 판정하지 않는다** |

**G07 · 피자 패턴 사자후**

| 항목 | 내용 |
|---|---|
| 파일 | `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json` (현재 42 binding, `VALTAN_SIX_PIZZA_106`은 0개) |
| 자원 | `boss.valtan.shout` (comet_1 4개 yaw 0/90/180/270, comet_2 4개 yaw 45/135/…, 200~900ms), `boss.valtan.shout.burst` (fog_1..4 + emit_1 + blur_4, 전부 0ms) |
| 저작 템플릿 | 사용자가 지목한 "버러지들 패턴 기준 사자후"는 `VALTAN_BIND_SLOT`이다. `STEP_01`에 `shout`, `RECOVERY`에 `shout.burst`. `VALTAN_ROAR_CHARGE`(`STEP_02` shout / `STEP_03` shout.burst `startMs 733`)도 같은 형태다 |
| 추가 1 | `VALTAN_SIX_PIZZA_106 / STEP_06` (8000ms, `mesh_att_battle_12_06`, LOOP_TO_STAGE_END) ← `boss.valtan.shout`, basis `CLIP_OCCURRENCE`, `clipOccurrenceId` = `valtan.sequence.center-six-pizza-charge.step-06.clip-01`, `startMs 0`, anchor `b_effectroot` / `SNAPSHOT_AT_START` / `TARGET_YAW`, stop `NATURAL` |
| 추가 2 | `VALTAN_SIX_PIZZA_106 / STEP_07` (`mesh_att_battle_12_07`, 1200ms) ← `boss.valtan.shout.burst`, `clipOccurrenceId` = `valtan.sequence.center-six-pizza-charge.step-07.clip-01`, `startMs 733` |
| repeatPolicy | `STEP_06`은 LOOP Stage다. `EACH_LOOP`을 쓰려면 **G02가 먼저 끝나야 한다.** 그 전에는 `ONCE`로 넣는다 |
| bindingId | 기존 `binding.valtan.migrated.<NNN>.<hash>`를 그대로 흉내 내지 말고 새 저작임을 드러내는 안정 ID를 쓴다 |
| 검증 | `powershell -File Tools/EffectPipeline/Validate-EffectSources.ps1` |
| 종료 증거 | 피자 패턴 `STEP_06`에서 사자후 comet과 `STEP_07`의 burst가 재생. 육안 판정은 사용자 |

**G08 · 잡힌 플레이어를 정확히 왼손에 붙인다**

| 항목 | 내용 |
|---|---|
| 사용자 확정 계약 | 플레이어의 월드 행렬을 offset 행렬로 만들고, 발탄 왼손 뼈 행렬을 가져와 곱해 플레이어를 손 위치로 옮긴다 |
| 현재 코드 | **이미 그 계약이다.** `CPlayerHandGripTransform::Build_LocalOffset`은 두 basis의 `r[3]`을 0으로 지운 뒤 `playerBasis * inverse(handBasis)`를 만들고 다시 `local.r[3] = 0`으로 지운다. `Compose_World`는 `local * handWorld`다 (`Client/Public/ClientReplication.h:133~175`) |
| 수학 검증 | 회전 성분은 `playerRot * handRot의 역행렬 * handRot = playerRot`로 **정확히 상쇄된다.** 발탄의 model admission scale도 같은 이유로 남지 않는다. 위치는 `handWorld`의 위치 그대로다. **"발탄 기준 월드 오프셋이 잘못 끼어들었다"는 가설은 코드상 성립하지 않는다** |
| 남은 변수 1 — 원점 | 결과 위치가 손 뼈 **원점**이고 캐릭터 원점은 **발바닥**이다. 발바닥이 손 원점에 붙으므로 몸이 손 위로 뜬다. 이 계약에는 손 기준 보정 translation이 없다 |
| 남은 변수 2 — 뼈 | `VALTAN_LEFT_HAND_BONE = "bip001-l-hand"` (`ClientReplication.cpp:94`)는 손목이다. 원본 Effect도 같은 slot을 쓰지만(`effect.valtan.project-tuned.sequence.trash-catch-if.effect.json`) 실제로 쥐는 지점은 보통 손바닥/손가락이다 |
| 남은 변수 3 — 프레임 순서 | `Update_PlayerAttachmentPresentations`는 `CClientReplication::Update()` 말미(`:320`)에서 호출되고 Level `Update`가 매 프레임 이를 돌린다. 발탄 본 갱신과의 순서에 따라 1프레임 지연이 생길 수 있다 |
| 수정 1 | `attachmentSlot`에 optional `gripLocalOffset { forwardM, upM, rightM }`을 추가하고 `Compose_World` 직전에 손 기준 보정 translation을 곱한다. **단위는 cm이므로 저작값에 100을 곱한다.** 사용자가 확정한 "offset 곱하기 손 행렬" 계약은 그대로 두고, 지운 평행이동 성분만 저작 가능한 형태로 되돌려 주는 것이다 |
| 수정 2 | Workbench Details에서 `gripLocalOffset`을 실시간 조절 |
| 진단 | F1 진단에 `bip001-l-hand`의 자식 뼈 목록을 덤프해 실제 손바닥 뼈를 고른다. 더 적합한 뼈가 있으면 상수를 교체하고 `gripLocalOffset`을 0에 가깝게 만든다 |
| 판정 콜라이더 | `VALTAN_TRASH STEP_08 / RETRY_RUSH_02 / RETRY_RUSH_03` BOX 6.0m × 2.5m, offsets 0/100/200/300/400/500/600ms (667ms에 7회 스윕). `VALTAN_CATCH_BREATH STEP_02` CONE 120° / 8.0m, offset 250ms 1회. 둘 다 `playerResponse CAPTURE`, `attachmentSlot BOSS_LEFT_HAND`. **G01이 먼저 끝나야 이 형태를 눈으로 볼 수 있다** |
| 종료 증거 | G01 wire로 BOX 진입 → 잡힘 → 플레이어가 손바닥에 붙어 보인다. 육안 판정은 사용자 |

**G09 · 모아치기 조준**

| 항목 | 내용 |
|---|---|
| 데이터 | `VALTAN_CHARGE` = `targetPolicy LOCK_NEAREST_ON_START` + `aimPolicy TRACK_TARGET_EACH_TICK`. `VALTAN_CHARGE_2`는 `NEAREST_EACH_TICK`. `VALTAN_SIX_PIZZA_106`은 `LOCK_RANDOM_ALIVE_ON_START` |
| 코드 | `BeginPatternTargetAndAim`(`ValtanBrain.cpp:709`)에서 `LOCK_NEAREST_ON_START`는 **호출자가 준 `nearestTarget` 포인터**만 쓴다. `LOCK_RANDOM_ALIVE_ON_START`는 `players` 맵에서 직접 뽑으므로 피자가 정상인 이유가 이것이다 |
| 실패 모드 | 시작 프레임에 `nearestTarget == nullptr`이면 `iPatternTargetEntityId`가 `INVALID`가 되고, `UpdatePatternTargetAndAim`(`:752`)의 `FindPatternTarget`도 nullptr를 돌려주므로 **패턴 내내 한 번도 회전하지 않는다.** 직전 yaw 유지 = 플레이어 기준 "엉뚱한/반대" 방향 |
| 추가 확인 | 목표 재획득은 `NEAREST_EACH_TICK`일 때만 일어난다. 또 `Restart_FinaleCycle`(`:3189`)은 최근접이 아니라 **`players` 맵 순서의 첫 engageable**을 고르므로 audition 경로에서는 이것만으로도 조준이 어긋난다 |
| 1단계 진단 | Server tick 로그에 `boss.fYawDegrees / target yaw / iPatternTargetEntityId`를 남긴다. **"회전을 안 한다"와 "반대로 회전한다"를 먼저 가른다.** 후자면 원인은 clip의 root 방향이고 아래 2/3은 오답이다 |
| 2단계 코드 | `LOCK_NEAREST_ON_START`가 `nearestTarget == nullptr`일 때 `players`에서 `IsEngageablePlayer`(`:317`) 최근접을 직접 재탐색하는 fallback을 넣는다. 이 정책을 쓰는 다른 패턴도 같은 함정을 공유한다 |
| 3단계 데이터 | `VALTAN_CHARGE`의 `targetPolicy`를 `NEAREST_EACH_TICK`으로 (= `VALTAN_CHARGE_2`와 동일). "플레이어 따라가게"라는 요구와 정확히 일치한다 |
| 종료 증거 | 1인 세션에서 `VALTAN_CHARGE` 재생 중 좌우로 이동 → 발탄 yaw가 매 tick 추종. Server 로그 yaw 시계열로 확인 |

**G10 · 워프 leg — 끝 사거리 도달 후 사라졌다가 포탈로 재등장**

| 항목 | 내용 |
|---|---|
| 현재 | `STEP_01` 2000ms → `STEP_02..STEP_09` 8 legs 각 2300ms (`PORTAL_TARGET_RUSH`, retarget 500 + 이동 800 + 후행 gap 1000) → `STEP_10` 1667ms. leg clip `mesh_att_battle_18_02`가 `LOOP_TO_STAGE_END`라 gap 1000ms 동안 body가 계속 보이고 loop가 돈다 |
| 원작 | 끝 사거리 도달 → 즉시 소멸 → 약 0.5초 뒤 포탈 생성 → 다음 돌진 |
| **설계 판단 정정** | 이전 안(`bodyVisibility`를 Server Stage 필드로 올려 snapshot 복제)은 **과하다.** body를 그릴지 말지는 gameplay authority가 아니라 presentation이고, Client는 이미 `strPatternId / strActionId / iPatternStageIndex / iActionStartTick`을 `WORLD_ENTITY_SNAPSHOT`으로 받는다. 따라서 **Shared/Server/protocol 변경 없이** Client presentation 문서만으로 닫을 수 있다 |
| 수정 1 (데이터) | `Data/Valtan/Valtan.presentation.json`의 stage에 optional `bodyVisibility { hiddenFromMs, hiddenToMs }` 추가. 8 leg 전부 `{ 1300, stage end }` |
| 수정 2 (Client) | `CValtan::Late_Update`(`Valtan.cpp:2997`)의 억제 조건을 `m_isGhostPresentationHidden` **또는** 새 `m_isBodyHiddenByStageWindow`로 확장한다. 후자는 `m_fServerActionAgeSeconds`와 현재 stage의 `bodyVisibility`로 매 프레임 계산한다. `CEffectV2Runtime::Set_Ignored`는 **걸지 않는다** — 포탈/트레일 Effect는 body가 사라진 동안에도 남아야 한다 (ghost relocation 경로와 의도적으로 다르다) |
| 수정 3 (데이터) | leg Stage duration 2300 -> 1800 (`trailingGapMs` 1000 -> 500). Workbench의 `Portal Gap After Rush (ms)` 컨트롤(`ActionCompositionWorkbench.cpp:5816`)이 이미 이 값을 소유하고 8 leg 전체를 파생시킨다 |
| 수정 4 (Effect) | gap 시작 시점(=1300ms)에 포탈 생성 Effect binding을 건다 |
| 하지 않는 것 | 새 `BOSS_COMBAT_STATE_FLAG` 비트 추가. `BOSS_COMBAT_STATE_KNOWN_FLAG_MASK`(`PacketMessages.h:702`)가 바뀌면 protocol v51 -> v52와 Server/Client 동시 배포가 강제되는데, 이 기능에는 그만한 이유가 없다 |
| 종료 증거 | 한 leg가 1800ms이고 1300ms에 body가 사라져 500ms 뒤 포탈과 함께 재등장. 육안 판정은 사용자 |

**G11 · 사망 이후 stage 잔존과 부활 시 유령**

| 항목 | 내용 |
|---|---|
| M 현재 | `VALTAN_GHOST_DEATH_AUDITION`은 `STEP_01` 3667ms `mesh_dead_1` **한 Stage**이고 branch도 next도 없다. HP를 0으로 만들지 않으므로 3667ms 뒤 `FinishPattern`으로 정상 종료되고 brain이 CHASE로 복귀해 플레이어를 추적한다 |
| M 수정 | `VALTAN_GHOST_DEATH_AUDITION / STEP_01`의 `EXIT`에 typed 사망 hold 이벤트를 추가해 brain이 pattern 선택과 추적을 멈추고 IDLE hold로 들어가게 한다. **`boss.bAutomaticPatternSequenceAuditionHold`가 이미 `Transition(boss, IDLE)` + `MovePath.clear()`로 정확히 이 동작을 한다**(패턴 선택 실패 분기, `ValtanBrain.cpp:2775` 부근)므로 이 래치를 재사용하는 쪽이 새 상태를 만드는 것보다 낫다. `SET_BOSS_FLAG` 확장 여부는 디테일 계획서에서 정한다 |
| N 현재 | `VALTAN_GHOST_RESPAWN_AUDITION / STEP_01`(3000ms, `mesh_respawn_1`)의 `ENTER`가 `SET_GAMEPLAY_PHASE gameplayPhase 3`을 발행한다. Client 스왑은 `Valtan.cpp:3576`의 `state.iGameplayPhase >= 3 ? "BOSS_VALTAN_GHOST" : "BOSS_VALTAN"`이고, 실패하면 `[Client][Valtan] phase presentation swap isolated:` 로그만 남기고 본체 모델을 유지한다 |
| N 진단 순서 | (1) snapshot의 `iGameplayPhase`가 실제로 3인가 (2) `Replace_PresentationPartGroup`이 실패하는가 — **격리 로그가 이미 있으므로 먼저 로그를 읽는다.** (3) `Apply_BossCombatState`(`:3552`) 앞단의 조기 return 3개(`m_isServerAuthoritative`, `Is_ValidBossCombatState`, `iStateRevision` 역행)에 걸리지 않는지 |
| N 배포 조건 | `CLAUDE.md`가 명시한 gameplay bootstrap v26 + protocol v51 동시 배포 여부를 함께 확인한다. Server/Client 중 한쪽만 갱신됐으면 스왑 조건 자체가 도달하지 않는다 |
| 종료 증거 | 부활 시퀀스에서 `BOSS_VALTAN_GHOST` part group으로 교체되고, 사망 audition 종료 후 발탄이 IDLE에 머물러 추적하지 않는다 |

**G12 · 사운드**

| 항목 | 내용 |
|---|---|
| 현황 | `Valtan.patternsoundcues.json`에 **562 cue**가 이미 있고 131개 고유 이벤트를 쓴다. `Client/Bin/Resources/Sound/Valtan`에는 252 wav = **133개 고유 이벤트**가 실재한다. 누락은 저작 공백이지 리소스 공백이 아니다 |
| **선행 차단 요인** | `Data/Sound/CharacterSoundCatalog.json`의 `classes.Valtan`은 132 이벤트인데, 사용자가 지목한 **돌 소리 두 개가 카탈로그에 없다.** `ValtanPatternSoundCueDocument.cpp:415`가 미선언 이벤트를 `Valtan pattern Sound event is not declared by the catalog`로 **문서 전체 거부**하므로 cue보다 카탈로그가 먼저다 |
| 카탈로그 추가 | `G_Voltan2_Attack09_ProjCreat1` (disk 4 variants), `G_Voltan2_Attack09_ProjExp2` (disk 1 variant) |
| 카탈로그 무해 항목 | `G_Voltan1_Attack13_Loop1`은 카탈로그에 빈 배열로 선언돼 있고 파일이 없다. `:425`의 asset 루프가 0회 돌아 통과하고 `ResolvedAssetIds`가 비어 재생만 no-op이 된다(`:421` 주석의 명시된 계약). **결함이 아니므로 건드리지 않는다** |
| 땅구르기 쿵쿵 | `VALTAN_GROUND_ROAR / STEP_01`(6458ms)에 `G_Voltan2_FootStep1`(4 variants) + 충격 계열 `G_Voltan2_Attack02_Shot1`(3) / `G_Voltan2_Attack08_Shot1..5` |
| 돌 터지는 시점 | `combatobject.valtan.ground-roar.rock`은 `hits: []`이고 `presentationEvents[]`의 `pulse.valtan.ground-roar.rock.explode`(TIMED 5000ms)만 갖는다. 그런데 `Valtan.combatobjectsoundcues.json`의 cue는 `combatObjectArchetypeId + hitId`로만 바인딩된다(현재 cue 1개, `high-jump.target-axe`의 `hitId`에 `ProjExp1`). **"돌 터지는 시점"에 소리를 붙일 바인딩 키가 지금 존재하지 않는다** |
| 스키마 확장 | `lostark.valtan-combat-object-sound-cues`에 `presentationEventId` 바인딩을 추가하고 `hitId`와 배타(exactly-one)로 검증한다. 소비자는 `CValtan::Apply_CombatObjectPresentationEvent` |
| cue 0개 Stage | `VALTAN_PART_BREAK/PART_BREAK`, `VALTAN_STAGGER_SLOT/CHANNEL·FINAL_ATTACK`, `VALTAN_BIND_SLOT/STEP_01·RECOVERY`, `VALTAN_SILENCE_SLOT/STEP_01·SILENCE_APPLY`, `VALTAN_GROUND_ROAR/STEP_01`, `VALTAN_GROGGY_FOLLOWUP/GROGGY`, `VALTAN_COUNTER_GROGGY/GROGGY`, `VALTAN_GHOST_DEATH_AUDITION/STEP_01`, `VALTAN_GHOST_RESPAWN_AUDITION/STEP_01`, `VALTAN_GHOST_PORTAL_ONCE/ACTIVE`, `VALTAN_FIST_IN_OUT/INNER`, `VALTAN_TRASH`(11 Stage), `VALTAN_SIX_PIZZA_106/STEP_02·06·08`, `VALTAN_TERRAIN_DESTRUCTION/STEP_02·07·08·10` 외 |
| cue 형식 | `{ bindingId, occurrenceId, patternId, stageId, actionId, clipOccurrenceId, soundBank: "S_Mob_G_Voltan2", soundEvent, repeatPolicy: "once", startMs }` |
| 타이밍 | 사용자가 직접 조정한다. 이 G는 **정확한 이벤트를 정확한 Stage/occurrence에 연결**까지만 책임지고 `startMs`는 clip 시작 기준 보수값을 넣는다 |
| 종료 증거 | 각 Stage 진입 시 지정 이벤트가 재생되고 `ResolvedAssetIds`가 비지 않는다 |

**G13 · 부위 파괴 = 애니메이션 + recovery + 4방향 돌 + 사운드**

| 항목 | 내용 |
|---|---|
| 현재 | `VALTAN_PART_BREAK`는 Stage 1개(`PART_BREAK`, `stageKind PART_BREAK`, 1400ms, hit NONE, events 0, branches 0), presentation은 `mesh_dmg_parts_end_1`을 `playMs 0 / repeatUntilStageEnd true`로 돌린다. effectCue 0, soundCue 0 |
| recovery의 정답 | 원본에 4 clip이 전부 있다. `Valtan.clipseq:118` — `420628 "레이드 발탄_2번째 부위 파괴" seq=2 mode=HOLD`, cuts `1.400 / 0.400 / 2.850 / 2.333` = **6983ms** |
| | `mesh_dmg_parts_start_1` 1400 (피격) · `mesh_dmg_parts_loop_1` 400 (유지) · `mesh_dmg_parts_end_1` 2850 (**회복**) · `mesh_idle_battle_1` 2333 (전투 idle 복귀) |
| 진단 | 지금은 **회복 clip을 start의 길이(1400ms)만큼 잘라 loop로 돌리고 있다.** start/loop 단계와 idle 복귀가 통째로 빠졌다 |
| Stage 분할 | `PART_BREAK`(start+loop, **1800ms**) / `PART_BREAK_RECOVERY`(end+idle, **5183ms**). 나눠야 Effect/Sound cue를 회복 구간에만 붙일 수 있다 |
| 저작 절차 | G00 이후에는 Append로, G00 이전에는 0.4의 Replace 우회로 넣는다 |
| 4방향 돌 | 3.10의 `valtan.ground-roar.cardinal-rocks` 블록을 `PART_BREAK_RECOVERY`의 `ENTER`에 복사하고 `eventId`만 바꾼다 |
| 파괴 사운드 | G12의 `presentationEventId` 바인딩으로 `pulse.valtan.ground-roar.rock.explode`에 `G_Voltan2_Attack09_ProjExp1`을, 생성 시점에 `ProjCreat1`을 건다. **G12의 카탈로그 추가가 선행이다** |
| 종료 증거 | 부위 파괴 → 회복 시퀀스 → idle 복귀가 끊김 없이 이어지고, 4방향 돌이 생성·폭발하며 각각 소리가 난다 |

**G14 · 3연속 계열 재저작 — 자리표시자를 원본 clip으로 교체**

| 항목 | 내용 |
|---|---|
| **admission 교정** | `VALTAN_TRIPLE_COUNTER`는 `decisionModel.manualAuditions`에 넣지 않는다. managed non-manual `CorePatternIds` 경로가 Workbench/Boss Play와 모든 안정 Stage의 Gameplay Details를 이미 연다. 기존 collider Tune은 canonical에도 허용되지만 topology·Add·Remove는 manual 전용이므로, 이 재저작의 topology 변경은 정본 source/pipeline transaction이 소유한다 |
| **회전 소유 보존** | 이 Pattern은 두 selection set에서 각각 weight 4를 유지하고 candidate 총수는 12로 HEAD와 같다. selection candidate와 manual audition의 교집합은 pipeline이 fail-close하므로 Details를 열겠다는 이유로 `MANUAL_SERVER_AUDITION` 또는 `DERIVED_SERVER_PATTERN`을 추가하면 안 된다 |
| P1 현재 | `VALTAN_TRIPLE_COUNTER` 7 Stage(`COUNTER_1` 1800 / `FAIL_1` 600 / `COUNTER_2` 1600 / `FAIL_2` 600 / `COUNTER_3` 1400 / `FAIL_3` 600 / `RECOVERY` 1200)의 clip이 **전부 `mesh_abn_groggy_1_start/_loop/_end`**다. 내려치기 애니메이션이 하나도 없다. 사용자가 "3번 연속 재생되는 시퀀스인데 뭔지 모르겠다"고 한 것의 정체가 이 자리표시자다 |
| P1 원본 | `Valtan.clipmap:26~30` — `mesh_att_battle_14_01/_02/_03/_04-1` = `420642 "레이드 발탄_연속 카운터 내려찍기 (첫 공격)"`, `_04-2` = `420644 "(마지막 공격)"`. **3연속 카운터 전용 clip 패밀리가 이미 있다** |
| P1 원본 cut | `420642 seq=1`: `14_01` 1.900 (windup) → `14_02` **1.000** → `14_02` **1.900** → `14_03` 1.550 (내려치기) → `14_04-1` 1.000 … / `420644 seq=1`은 같은 반복 뒤 `14_04-2` 1.000 + `14_02` 0.900 |
| P1 재저작 | windup `14_01` 1900ms를 앞에 추가, `COUNTER_n` = `14_02` **exact 1000ms**, `FAIL_n` = `14_03` **exact 1550ms**, `RECOVERY` = `14_04-2`. Stage duration도 같은 값으로 맞춘다 |
| P2 현재 | `VALTAN_COUNTER / STEP_02`(1800ms WINDUP, 카운터 창)가 `mesh_att_battle_14_02`를 `LOOP_TO_STAGE_END`로 돌린다. 그런데 `Valtan.clipmap`에 `mesh_att_battle_14_loop`는 **존재하지 않는다.** one-shot clip을 1800ms 동안 반복 재생하니 끊겨 보인다 — 사용자 관찰 "loop가 없다"가 정확하다 |
| P2 수정 | 원본이 loop를 `14_02`의 **반복 + 서로 다른 cut(1.000 / 1.900)** 으로 표현하므로, `STEP_02`를 `14_02` exact 1000 + `14_02` exact 800의 2 slot으로 저작해 `LOOP_TO_STAGE_END`를 제거한다 |
| P3 현재 | `VALTAN_THREE`는 `STEP_01/02/03` 전부 `ACTIVE`이고 WINDUP Stage가 0개다. 판정은 `STEP_03`의 CONE 90° / 12m **1회**뿐인데 3번 내려친다 |
| P3 원본 | `420602 seq=2` = `mesh_idle_battle_1`(2.333) → `2_01`(**2.000**) → `2_02`(**1.100**) → `2_03`(**2.100**) → `2_01`(2.000) |
| P3 수정 | Stage duration을 2000 / 1100 / 2100으로 맞추고 **세 Stage 각각에 판정을 넣는다** (현재 1800 / 1200 / 2067) |
| 종료 증거 | 3연속 카운터가 그로기 포즈가 아니라 실제 내려찍기로 재생되고, 3연속 내려치기가 3번 모두 판정한다 |

**G15 · 침묵 마스크 — 자물쇠를 쿨타임 마스크(R 상향)로 교체**

| 항목 | 내용 |
|---|---|
| 파일 | `Data/UI/HUD/HUD_Layout.json`, `Client/Private/MainApp.cpp` |
| 수정 1 | `Skill_R_SilenceMask`의 `layers[0].path`를 `UI/ItemUpgrade/buildup_lock_icon.png` → `UI/Common/White1x1.png`, `tint`를 `[1,1,1,1]` → R 상향 값(예: `[0.85, 0.0, 0.0, 0.588]`). 알파 `0.5882352941176471`은 `Skill_Q_Cooldown`과 동일하게 맞춘다 |
| 수정 2 | 침묵은 **모든 ACTIVE 슬롯**을 막는다(`GameRoom.cpp:2751 / 2791 / 3151 / 3181`, 네 곳 모두 skillId를 보지 않는다). R 하나만 표시하는 현재 저작이 의미상 틀렸다. class별 ACTIVE 슬롯 전부에 `Skill_<Slot>_SilenceMask`를 추가한다 |
| 수정 3 | `Update_SilenceRSlotMask`(`MainApp.cpp:343`, 호출 `:1834`)를 `Update_SilenceSlotMasks`로 바꿔 슬롯 배열을 순회한다. 판정식 `Is_ServerDeadlinePending(Player.iServerTick, Player.iSilenceEndTick)`은 그대로 둔다 (tick wrap 처리가 이미 정확하다) |
| 발탄 쪽 | `VALTAN_SILENCE_SLOT`(`STEP_01` 2633ms `mesh_evt1_att_battle_5_01_end`, `SILENCE_APPLY` 100ms `SET_PLAYER_SILENCE durationMs 5000`)에는 V2 binding 0개다. 시전 표현만 별도로 붙인다 |
| 이전 판단 정정 | "layout JSON runtime factory와 입력 라우터가 닫히기 전에는 완료로 기록할 수 없다"는 **이 항목에는 해당하지 않는다.** `CUILayoutRuntime`은 이미 제품 경로이고 `Set_SlotVisible`/`Set_SlotTint`가 살아 있다. 미완인 것은 **interaction/command binding**(클릭 가능한 버튼)이지 display-only 이미지 위젯이 아니다 |
| 종료 증거 | 침묵 중 모든 ACTIVE 슬롯에 붉은 마스크가 덮이고, 자물쇠 아이콘이 사라진다 |

### 4.4 별도 — G16 · "Server entry failed" 진단

3.14에서 확정한 대로 이 문자열은 원인이 아니라 **증상 하나로 뭉쳐진 24개 경로의 공통 출구**다.
따라서 이 G는 코드 수정으로 시작하지 않는다.

| 단계 | 내용 |
|---|---|
| 1 | Client 재현. Valtan Arena에서 대상 패턴을 Debug Next Pattern으로 **3회 연속** 재현한다 |
| 2 | `<Client exe>\Diagnostics\client-session-<pid>.jsonl`의 마지막 recovery 이벤트에서 `eReason`, `strSource`, `strDetail`을 읽는다. 3.14 표에서 넷 중 어느 경로인지 즉시 결정된다 |
| 3 | 같은 시각 `<Server exe>\Diagnostics\server-session-<pid>.jsonl`의 session close reason, peerAddress:peerPort, exit code를 대조한다 |
| 4 | Server가 살아 있고 Client가 `world-destruction-sync` 또는 `encounter-prop-sync`로 끊었다면 **Client presentation 문제**다. `m_Replication.Try_Consume_PresentationFailure` / `m_DeployRuntime.Get_Status()` 문자열이 정확한 asset/placement를 지목한다 |
| 5 | `network-connection-lost`라면 그때 비로소 Server 크래시/ingress 포화를 본다. Server 마지막 로그 줄과 exit code를 기록한다 |
| 6 | `level-valtan.initialize`라면 `strDetail`이 거절한 stage 이름을 그대로 담고 있다 |
| 사용자 가설 검증 | "플레이어 사망 후 pending/idle 전환 작업 때문"이라면 재현 시 **플레이어 생존 상태와 사망 상태를 각각 3회씩** 나눠 기록한다. 사망 시에만 재현되면 `IsEngageablePlayer`(`ValtanBrain.cpp:317`)가 false가 되어 `target == nullptr`이 되는 경로와 연결된다 |
| 개선 (진단 후) | Lobby 상태 줄에 `strSource`를 함께 표시한다. 24개 경로가 한 문장을 공유하는 현재 상태가 이 항목을 처음부터 어렵게 만든 원인이다. **이 한 줄 개선은 원인과 무관하게 지금 해도 된다** |
| 데이터 소견 | `VALTAN_SEQUENCE_FOUR`는 Stage 1개(`STEP_01`, 5000ms, CROSS 18.0 × 2.5, offsets 1790/2560/3330, `damage.valtan.four-slash`, knockdown 1200ms)이고 `events`/`branches`가 비어 있다. `CROSS`는 Server가 정식 지원한다(`GameplayCatalog.cpp:282`, `ServerCollisionSystem.cpp:1022`). **데이터만으로는 어떤 bounce 경로도 설명되지 않는다** |
| 금지 | 이 진단 결과 없이 코드를 고치지 않는다 |

#### 4.4.1 2026-09-03 진단 확정과 수정 계약

Client/Server JSONL의 동일 시각·동일 socket tuple을 대조한 결과, 이 occurrence는 플레이어 사망,
패킷 포화 또는 Client replication parser가 원인이 아니다. `VALTAN_SIX_PIZZA_106`과
`VALTAN_STRUGGLING`의 delayed four-rock wave가 정확한 예정 tick에 실행될 때 일부 authored root가
navgrid 밖에 놓인다. 두 정의는 `FIXED_AREA + direction NONE + hits=[] + presentation pulse`인 순수
표현 오브젝트인데 기존 exact owner 집합에는 Ground Roar와 Part Break만 있어 Server가 room runtime
failure를 latch했다.

수정은 기존 의미 조건과 `count == 4`를 그대로 둔 채 아래 두 `(object, pattern, action)` 튜플만
추가한다. 좌표를 navgrid로 project/clamp하지 않고, 모든 visual object를 포괄 허용하지 않으며,
`hits`가 하나라도 생기면 strict navigation admission으로 복귀한다.

- `combatobject.valtan.six-pizza.rock-pillar / VALTAN_SIX_PIZZA_106 / valtan.sequence.center-six-pizza-charge.step-01`
- `combatobject.valtan.struggling.rock-pillar / VALTAN_STRUGGLING / valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-04`

회귀 계약은 두 owner 모두 ENTER 시 무생성, `1000ms=30 tick` 또는 `833ms=25 tick` 직전 no-op,
예정 tick의 정확한 4개 atomic spawn, 다음 tick 중복 없음, off-nav authored root·wire serialization을
검사한다. 같은 정의에 damage hit를 주입하면 실패하고 live/pending lifecycle이 모두 비어 있어야 한다.

---

## 5. 실행 순서와 의존성

G 사이에 실제 선후 관계가 있다. 이를 무시하면 뒤 G를 검증할 수 없거나 bootstrap이 깨진다.

```
[1] G05  bootstrap v30 -> v31 (원자 커밋, 반드시 맨 앞 또는 맨 뒤)
      |
[2] G00  Composition 저작 병목 제거      ---- 이것 없이 G13/G14의 저작이 불가능
[3] G01  Debug collider 전면 노출        ---- 이것 없이 G03/G04/G08의 검증이 불가능
[4] G02  Effect V2 clock/repeat 계약     ---- 이것 없이 G07의 EACH_LOOP과 E 항목이 불가능
      |
[5] G03  Workbench Collider/Damage 저작  (G00, G01 뒤)
    G04  플레이어 hitshapes 43개         (G01 뒤)
      |
[6] G06 G09 G10 G11 G15                  (서로 독립, 병렬 가능)
    G12  사운드 카탈로그 + presentationEventId 스키마
      |
[7] G07  피자 사자후                     (G02 뒤)
    G13  부위 파괴                       (G00, G12 뒤)
    G14  3연속 계열 재저작               (정본 source/pipeline transaction, manual 승격 없음)
      |
[8] G16  Server entry failed 진단        (언제든 가능, 코드 수정은 진단 후)
```

각 의존의 이유.

| 의존 | 이유 |
|---|---|
| G13 → G00 | 4 clip 6983ms 시퀀스를 넣으려면 `3u == Clips.size()` 게이트가 풀려야 한다 |
| G14 admission | managed non-manual Core Pattern이므로 Workbench/Boss Play/Details는 이미 열린다. topology는 source/pipeline transaction으로 바꾸고 `manualAuditions`에는 추가하지 않는다 |
| G08 → G01 | BOX 6.0×2.5가 667ms 동안 300ms씩만 번쩍이면 부착 위치를 눈으로 검증할 수 없다 |
| G03 → G01 | 저작한 collider를 즉시 화면에서 확인할 수 없으면 튜닝 도구가 아니다 |
| G04 → G01 | 회색 폴백 원이 사라지는 것이 이 G의 종료 증거다 |
| G07 → G02 | `STEP_06`이 LOOP Stage라 `EACH_LOOP` 없이는 사자후가 1회만 난다 |
| G13 → G12 | 돌 폭발 사운드는 `presentationEventId` 바인딩과 카탈로그 2개 추가가 선행이다 |
| 모든 G ↔ G05 | bootstrap format version이 바뀌므로 중간에 끼면 전후 커밋이 서로의 bootstrap을 거부한다 |

**E(파운딩 2회 재생)에 대한 추가 경고.** G02가 `EACH_LOOP`을 구현해도 재생 횟수는
`floor(stageMs / nativeLoopMs)`가 된다. `VALTAN_STRUGGLING / STEP_06`은 3000ms이고
`mesh_att_battle_19_03`의 원본 cut은 **3.200s**다. 즉 런타임 native 길이가 원본 cut과
같다면 epoch은 여전히 1회뿐이고 binding 2개 = **여전히 2회**다.
따라서 G02 직후에 **`Resolve_ValtanCompositionNativeClipDurationMs`로 실제 모델 clip 길이를
측정**하고, native ≥ stage면 (a) Stage 시계를 늘리거나 (b) binding을 `startMs` 간격으로
추가해 원하는 파운딩 횟수를 명시 저작해야 한다. G02만으로 끝났다고 판단하지 않는다.

---

## 6. G별 검증

### 6.1 공통 자동 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core
```

`Engine/Public` 헤더를 건드린 G와 G05(Shared 변경)는 `-Profile FullDiagnostic`을 쓴다.

### 6.2 도메인별 validate

| 변경 도메인 | 명령 |
|---|---|
| `Data/Valtan/*`, `Data/Balance/*`, `Data/Encounters/*` | `powershell -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Validate` |
| `Data/Effects/V2/*`, `Data/Effects/Authored/*` | `powershell -File Tools/EffectPipeline/Validate-EffectSources.ps1` |
| `Data/Worlds/*` | `powershell -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Validate` |
| 발탄 패턴 커버리지 | `python Tools/ValtanPipeline/validate_valtan_requested_pattern_coverage.py` |

### 6.3 실행형 harness

```
Server.exe --contract-test
Tools/ValtanPatternAuditionServiceHarness
Tools/NetworkProtocolHarness            (G05, G10 검토 시)
```

### 6.4 G별 종료 판정 매트릭스

| G | 자동으로 판정 가능한 것 | 사용자만 판정 가능한 것 |
|---|---|---|
| G00 | 4 clip 6983ms Replace/Append 성공, `Save + Validate + Publish` + canonical reload 통과 | Workbench 화면의 라벨/버튼 가독성 |
| G01 | 토글 상태와 draw 호출 수 | wire가 실제로 보이는지 |
| G02 | epoch 수 로그가 V1 스캐너와 일치 | 파운딩이 끊기지 않는지 |
| G03 | 저장된 JSON의 collider/damage 값, publish 통과 | 튜닝한 판정 범위가 체감상 맞는지 |
| G04 | hitshapes 94/94, publisher 커버리지 리포트 0% class 없음 | 회색 폴백 원이 사라졌는지 |
| G05 | Server 로그의 `fPositionY` 전후 값, 계약 테스트 | 0.5m 부양 높이가 적절한지 |
| G06 | occurrence 생성 여부, world 행렬 거리(cm) | 검은 구가 보이는지 |
| G07 | binding 개수, `Validate-EffectSources` 통과 | 사자후 형태/타이밍 |
| G08 | 손 뼈 world 좌표와 플레이어 좌표의 차이 | 손바닥에 붙어 보이는지 |
| G09 | Server yaw 시계열이 target을 추종 | 방향이 자연스러운지 |
| G10 | leg 1800ms, hide window 1300ms | 사라짐/재등장이 원작과 닮았는지 |
| G11 | `iGameplayPhase == 3`, 스왑 로그, IDLE 유지 | 유령 모델이 맞는지 |
| G12 | `ResolvedAssetIds` 비지 않음, 카탈로그 선언 | 소리가 맞는지, 타이밍 |
| G13 | Stage 2개 6983ms, 돌 4개 spawn | 회복 동작이 자연스러운지 |
| G14 | clip 이름이 groggy가 아님, 3 Stage 판정 3회 | 내려찍기가 맞는지 |
| G15 | 슬롯 visible 토글, tint 값 | 붉은 마스크 가독성 |
| G16 | 동일 socket의 Client/Server JSONL, delayed scheduler 두 owner native contract, narrow owner source contract | Six Pizza/Struggling 재현에서 Lobby 복귀가 사라지는지 |

**수동 검증은 사용자 전용이다.** 에이전트는 빌드, 로그, 수치 진단, 실행 준비까지만 수행한다.
`manual first pixel`, `eye smoke`, `visual PASS`는 사용자의 서면 관찰이 있어야 기록한다.

### 6.5 사용자 확인 경로

```
Server + Client profile 로 Ctrl+F5
Lobby -> Character Select (Server 승인) -> Enter Valtan Map
F1 -> Developer Tools
   Debug Colliders 전역 토글 5종 (G01)
   Action Composition Workbench -> 대상 Pattern -> Stage -> Details
   Boss Tool -> Next Pattern 으로 대상 패턴 재생
```

---

## 7. 위험과 되돌리기

| 위험 | 영향 | 완화 |
|---|---|---|
| G05의 bootstrap v31 | 이전 Server/Client 바이너리가 새 bootstrap을 전부 거부 | 원자 커밋 + 순서 고정. 팀 전체가 같은 revision을 빌드해야 한다 |
| G02가 42개 binding 중 34개(CLIP_OCCURRENCE basis)의 동작을 바꾼다 | 지금 "우연히 맞아 보이던" 이펙트 타이밍이 전부 흔들릴 수 있다 | 변경 전 24 Pattern의 이펙트 재생 시각을 로그로 캡처해 전후 비교. 회귀가 크면 basis별로 단계 도입 |
| G00 변경 2가 `Pattern Total Duration` 편집으로 Stage를 만들던 기존 워크플로를 없앤다 | 그 동작에 의존하던 저작 손버릇이 깨진다 | 명시적 Stage 추가 버튼으로 대체하고 reject 메시지에 새 경로를 안내 |
| G03의 damage 수치 편집 | Pattern source와 `DamageProfiles.json`이 서로 다른 owner인데 한 화면에서 편집된다 | Sound cue와 동일하게 **별도 CAS 저장**으로 구현하고, UI에 "한 atomic writer가 아님"을 명시. `AGENTS.md`의 Balance provenance 동기화(`PROJECT_TUNED`)를 같은 단계에서 수행 |
| G14를 `manualAuditions`/DERIVED로 잘못 승격한다 | selection candidate와 manual audition의 교집합을 pipeline이 거부하고 Core audition inventory 소유권도 깨진다 | 승격하지 않는다. 두 selection set의 weight 4, candidate 총수 12와 managed non-manual `CorePatternIds` 경로를 focused contract로 고정한다 |
| G10의 `bodyVisibility`가 Client 전용 | Server는 body가 보인다고 가정하고 판정을 계속한다 | **의도된 설계다.** 숨김은 표현이고 판정은 그대로 유지된다. 사용자에게 "숨은 동안에도 맞는다"를 명시 |
| G12의 스키마 확장 | 기존 combat object sound cue 1개가 깨질 수 있다 | `hitId` / `presentationEventId` exactly-one 검증. 기존 cue는 `hitId` 경로로 그대로 통과 |
| Working tree의 기존 dirty 파일 | 다른 세션의 cue 삭제 diff가 섞여 커밋된다 | 각 G 커밋 전에 `git status --short`로 소유권을 분리하고 무관한 파일을 stage하지 않는다 |

---

## 8. 이 계획이 만들지 않는 것

- Client/UI 자율 실행, 화면 캡처, visual PASS 대리 판정
- `CValtan` 로컬 AI, Client 측 damage 판정
- 두 번째 Effect runtime, 두 번째 model 경로
- 생성물(`Client/Bin/DataFiles`, `Server/Bin/DataFiles`) 직접 편집
- 진단 없이 G16(Server entry failed)의 코드 수정
- protocol v51 -> v52 (G10을 flag 방식으로 만들지 않는 이유)
- 밸런스 Hot Reload (revision/tick commit 슬라이스가 닫히기 전까지)

---

## 부록 A. 쿠크세이튼 Composition 확장 조사 (구현 전 실측)

> **상태:** 이 부록은 2026-09-02 현재 코드·데이터를 읽어 확장 경계와 선행 결정을 정리한
> 조사 결과다. 쿠크세이튼 Product boss/pattern 또는 저작 UI를 구현했다는 뜻이 아니며,
> 별도 승인과 아래 Product identity 결정 전에는 기존 G00~G16 실행 순서에 포함하지 않는다.

### A.1 현재 닫혀 있는 범위와 닫히지 않은 범위

| 항목 | 실측 결과 | 판정 |
|---|---|---|
| Animation reference | `MN_RPCT_05` 115개, `MN_RPCT_06` 47개, `MN_RPCT_07` 102개, `MN_RPCZ_00` 85개로 총 **349 action**이 있다. 네 `*.patternbindings.json`은 모두 `authority: REFERENCE_ONLY`, `patterns: []`다 | clip 탐색·참고만 가능. Server Product pattern 정본이 아니다 |
| 물리 model body | `MN_RPCT_05`, `MN_RPCT_06`, `MN_RPCZ_00`의 `.wmodel`이 있고, `MN_RPCT_07`은 물리적으로 `MN_RPCT_05` body를 alias한다 | 어떤 body가 실제 Product actor인지 아직 미결정 |
| Arena world | `LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`은 15 placement(6 triggerBox, 9 playerSpawn)를 가지지만 **boss placement는 0개**다 | Arena admission과 boss encounter admission은 별개다 |
| Product identity | `BossCatalog.json`과 `BossProfiles.json`에는 Valtan/Ghost만 있고 쿠크세이튼 archetype/profile이 없다 | stable boss archetype/profile 선행 필요 |
| Server runner | generic bootstrap 구조와 BOX/`ACTIVE_WINDOW` 판정 자료형은 있지만 실제 boss update는 `CValtanBrain`에 결합돼 있다 | 쿠크 Product pattern을 실행하는 Server runner가 없다 |
| Client presentation | boss spawn은 `boss.valtan.client.v1`과 `CValtan`만 허용한다 | 쿠크 archetype을 표현하는 Product Client consumer가 없다 |
| Workbench | Kakul/Saydon reference action을 열 수 있으나 Stage graph, 저장 command, Balance/Boss Tool 연결은 `VALTAN_*`와 `CValtanPatternTree`에 결합돼 있다 | reference browser이지 쿠크 Product authoring provider가 아니다 |
| Effect/Sound/Camera | 물리 Effect/Sound 리소스는 있으나 쿠크 typed catalog/binding consumer가 없다. Camera Tool은 Valtan cinematic document/path에 결합돼 있다 | 탭을 노출할 Product owner와 runtime consumer가 없다 |
| Save | Kakul action/pattern reference 문서는 각자 파일 단위 atomic save만 수행한다 | gameplay·presentation·effect·sound·camera를 함께 보존하는 cross-owner transaction이 아니다 |

현재 Kakul focused 계약 테스트 네 묶음
(`test_kakul_animation_action_document_contract`,
`test_kakul_animation_pattern_document_contract`,
`test_kakul_world_admission`,
`test_kakul_client_product_level_contract`)은 **27 test PASS**다. 이 결과가 증명하는 범위는
reference 문서와 Arena world/level admission뿐이며, Product boss/pattern 지원의 증거로 사용하지 않는다.

### A.2 첫 번째로 확정해야 할 Product identity

구현보다 먼저 다음 중 어떤 body가 어떤 Product boss archetype/profile을 소유하는지 확정한다.

1. `MN_RPCT_05`와 같은 물리 body를 공유하는 `MN_RPCT_05` / `MN_RPCT_07`
2. 별도 body인 `MN_RPCT_06`
3. 별도 body인 `MN_RPCZ_00`

또한 위 항목을 쿠크와 세이튼의 별도 Product actor로 나눌지, 한 encounter의 phase/presentation
variant로 둘지도 같은 결정에 포함한다. 이 결정 전에는 `BOSS_KAKUL` 같은 stable ID, profile,
world placement 또는 Client prototype을 임의로 만들지 않는다.

### A.3 최소 수직 슬라이스와 순서

| 순서 | 닫아야 할 계약 | 최소 완료 조건 |
|---:|---|---|
| 1 | Product identity | 선택한 `MN_RPCT_05/07`, `MN_RPCT_06`, `MN_RPCZ_00` body와 boss archetype/profile/phase 관계를 문서·catalog stable ID로 확정 |
| 2 | World + actor admission | `BossCatalog.json`, `BossProfiles.json`, Kakul arena boss placement와 generic Client boss presentation factory를 같은 변경 단위로 연결 |
| 3 | Canonical sources | 기존 `REFERENCE_ONLY` 문서는 clip intake로 유지하고, Product gameplay/presentation/encounter 정본과 publisher를 별도로 추가. Reference 파일을 Product authority로 재해석하지 않음 |
| 4 | Common Server runner | Valtan 전용 특수 hook은 남기되 Stage graph/clock/transition/hit 실행을 공용 boss runner로 추출. Kakul도 Data -> publisher -> Server runner를 통과해야 함 |
| 5 | Common Workbench provider | 두 번째 Workbench를 만들지 않고, Valtan adapter와 Kakul provider가 같은 조회·typed mutation 계약을 구현. unsupported owner는 fail-closed |
| 6 | Stage + typed next-stage | stable action/stage ID로 Stage 추가와 명시적 후속 Stage 선택을 저장. vector index, dangling target, 암묵적 임의 분기, 무한 cycle은 거부 |
| 7 | Collider | 기존 Server XZ BOX의 `lengthM`, `halfWidthM`과 `ACTIVE_WINDOW(startMs, lifetimeMs)`를 공용 계약으로 재사용하고 Workbench preview를 같은 값에서 그림 |
| 8 | Animation/Effect/Sound/Camera tabs | owner-keyed animation occurrence, Effect V2 group/binding, sound cue, cinematic camera와 shake 문서를 Kakul consumer까지 연결. 각 dependency는 stable stage/occurrence ID로 검증 |
| 9 | One atomic Save | gameplay, presentation, animation, effect, sound, camera(+shake)의 baseline/candidate/read-set을 `sourceRevision` CAS로 stage -> validate -> project -> commit. 어느 owner라도 실패하면 전부 rollback하고 성공 뒤에만 editor reload |
| 10 | Product admission | BuildDomains/publisher/harness를 추가하고 Product -> Core -> 변경 domain FullDiagnostic까지 통과한 뒤에만 지원 상태로 승격 |

현재 Server BOX는 지면 XZ 판정 계약이다. `lengthM`은 전방 길이, `halfWidthM`은 좌우 반폭이며
`ACTIVE_WINDOW`는 Stage 안의 활성 시작 시각과 lifetime을 뜻한다. 요청의 BOX `size`가 높이까지
포함한 3D XYZ 크기를 의미한다면 기존 계약에 조용히 끼워 넣지 않고, Y overlap과 debug mirror를
포함하는 별도 Server-authority 계약으로 먼저 결정한다.

### A.4 필요한 validator와 실행형 harness

| 검증층 | 추가/확장할 focused 계약 |
|---|---|
| Product source/publisher | 유효한 archetype/profile/encounter/action/stage 한 세트, version/ID/path 오류, 중복 ID, missing profile/damage/clip, 생성물 drift를 검증하는 Kakul Product publisher test |
| Stage graph | typed next-stage의 dangling target, self/cycle, 도달 불가 Stage, 종료 없는 graph를 거부하고 Stage add/delete/move 뒤 stable ID 보존을 검증 |
| Collider | BOX 수치 범위, `ACTIVE_WINDOW`가 Stage duration 안에 있는지, 경계 tick과 target당 1회 hit, 잘못된 lifetime의 transaction rollback을 Server contract에서 검증 |
| Presentation dependencies | effect/sound/camera binding이 존재하는 action/stage/clip occurrence만 참조하는지, asset 누락이 gameplay가 아니라 해당 presentation만 격리하는지 검증 |
| Atomic writer | cross-owner `sourceRevision` 충돌, temp/project 실패, publisher reject 시 부분 commit 0건과 기존 canonical/product 보존을 검증 |
| Runtime | Server stage transition/branch/hit, generic Client spawn/despawn/presentation, normal rotation candidate 수 불변을 검증 |

focused test는 최소한
`test_kakul_action_composition_product_contract.py`,
`test_action_composition_boss_provider_contract.py`,
`test_kakul_canonical_typed_patch_transaction.py`,
`test_kakul_effect_v2_binding_contract.py`,
`test_kakul_pattern_sound_cue_contract.py`,
`test_kakul_cinematic_camera_contract.py`와
`ServerGameplayContractTests`의 Kakul stage/hit 사례를 소유해야 한다. 도메인 검증은
`Publish-WorldGameplay.ps1 -Validate`, 공용화한 gameplay publisher validate, Effect V2/Sound/Camera
validator를 포함하고, 최종 자동 검증은 Product, Core, 변경 domain FullDiagnostic 순서로 수행한다.
Client 화면과 visual fidelity는 계속 사용자가 직접 판정한다.

### A.5 금지하는 반쪽 경로

- 현재 `REFERENCE_ONLY` Kakul action/pattern을 Workbench에서 보이게 하거나 Direct Replace/Append만
  푸는 것으로 Product 지원 완료 처리하지 않는다.
- Product archetype/profile, world placement, publisher, Server runner와 Client consumer가 없는 상태에서
  **UI 탭만 여는 반쪽 경로를 금지한다.**
- Kakul을 `CValtan`, `CValtanBrain`, `Data/Valtan` 경로로 위장하거나 Client 로컬 AI/damage로 우회하지 않는다.
- 두 번째 Action Composition Workbench, 두 번째 Effect runtime, 생성물 직접 편집을 만들지 않는다.
- Camera를 별도 Save 성공으로 남겨 둔 채 나머지 owner만 commit하지 않는다. Camera와 shake까지 동일한
  atomic transaction의 read-set/rollback 경계에 들어온 뒤에만 `Save`를 제공한다.
