# 2026-09-05 KoukuSaydon Lane/Box Composition Workbench 구현 계획서

> 문서 종류: 구현 계획서 (범위와 변경 단위 합의용. 각 G의 H/CPP 전문은 디테일 계획서에서 확장한다)
>
> 상태: 실측 완료 / 구현 전
>
> 기준 브랜치: `main` (HEAD `955f7971`). 이 worktree에는 다른 세션의 미커밋 문서
> `2026-09-05_KOUKU_SAYDON_SHIELD_STAGGER_AND_COMMON_GROGGY_PATTERN_IMPLEMENTATION_PLAN.md`가 있다.
> 그 문서의 Server 확장(G02)과 Boss Tool follow-up 인정(G03)은 이 계획의 G03/G04가 흡수하고,
> 그 문서의 stage 직접 저작 모델은 이 계획의 lane/box 모델로 대체한다. 그 문서는 수정하지 않는다.
>
> 선행 조사: [1~3관문 패턴 애니메이션 전수 조사](2026-09-05_KOUKU_SAYDON_GATE1_3_PATTERN_ANIMATION_SURVEY.md) §2,
> [Workbench·Boss Tool 최소 수직 슬라이스](2026-09-05_KOUKU_SAYDON_ACTION_COMPOSITION_WORKBENCH_AND_BOSS_TOOL_IMPLEMENTATION_PLAN.md) §4.3

## 0. 목표와 종료 증거

### 0.1 목표

쿠크세이튼 Action Workbench를 Effect Tool의 `Create Element -> slot/resource 선택 -> Details 편집` 구조와 같은
`+ Add -> 카테고리 자원 선택 -> box 생성 -> Details 편집` 구조로 바꾼다. 사용자는 Sequencer의 lane 위에
box(animation, logic, effect, scene profile, camera, sound, summon)만 놓는다. Server가 필요로 하는 stage는
사용자가 만들지 않고, publisher가 box의 시간 경계에서 결정적으로 파생한다.

첫 실제 소비자는 1관문 방패 무력화 패턴이다. Logic 카테고리에서 `Stagger Window`를 고르고, 정면 90도 안에서
들어온 플레이어 공격은 보스에게 들어가지 않고 공격자가 반사 damage를 받으며, 그 밖의 공격은 무력화 게이지를
채운다. 게이지가 차면 그로기 follow-up pattern, 시간 초과면 발산 clip으로 이어진다.

### 0.2 종료 증거

1. `KoukuSaydonComposition.json` formatVersion 3 migrate 뒤 `python -B Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode publish`가
   Pizza를 지금과 같은 6 stage(2500/4667/1000/2500/4667/1000)와 같은 clip 순서로 다시 투영한다(G01 동등성 테스트).
2. Workbench에서 Resources 탭 `Logic`의 `Stagger Window`를 `+ Add`로 놓고 Details에서 요구량·90도 반사를 편집·Save한 뒤
   Reload한 JSON이 §4.2와 같다(G02).
3. `Server.exe --contract-test`가 쿠크 Brain의 stagger 성공 follow-up, 시간 초과 CONTINUE, 정면 반사, 그로기 stage kind를 통과한다(G03).
4. `Publish-GameplayBalance.ps1`이 `PATTERNSTAGEACTION`, `PATTERNSTAGEBRANCH`, `PATTERNSTAGEFOLLOWUP`, `PATTERNSTAGESOURCEGATE` 행을 bootstrap에 싣고
   Debug Product 빌드가 통과한다(G04).
5. 사용자가 실제 Server+Client에서 Boss Tool `Play Selected` -> hold 중 후면 타격 -> 그로기 체인, 정면 타격 -> 플레이어 피해, 무타격 -> 발산 clip을 눈으로 확인한다.
   이 확인 전에는 visual PASS를 기록하지 않는다.

### 0.3 이 계획이 채택한 사용자 결정 기본값

사용자가 아직 답하지 않은 다섯 항목은 아래 기본값으로 진행한다. 값이 바뀌면 §4.2 데이터만 바뀌고 구조는 같다.

| 항목 | 기본값 |
|---|---|
| 방패 무력화 pattern | composition에 DRAFT로 이미 있는 `KAKULSAYDON_G1_SHIELD_STAGGER`(표시명 `1관문 / 방패 무력화 패턴`)를 사용한다. 130줄 반사 clip(4219710x, 4219763)을 이 pattern의 animation box로 쓴다 |
| 정면 90도 안 공격 | 보스 HP/게이지에 0, 공격자에게 반사 damage. `damage.kakulsaydon.shield-reflect` profile 신규 |
| 무력화 성공 조건 | `SET_STAGGER_GAUGE` 게이지와 스킬 `staggerDamage` 누적, outcome `STAGGER_BROKEN` |
| 시간 초과 | 발산 clip `rpcz00_att_battle_14_03` 재생. lethal hit은 G09(Collider lane) 뒤 |
| Logic 카테고리 | G02에 포함. Server 소비자(G03/G04) 전까지 PRODUCT publish는 projector가 거부하고 DRAFT 저장만 허용 |

## 1. 현재 실측

### 1.1 stage의 실제 의미

Server `BOSS_PATTERN_STAGE_DEFINITION`(`Server/Public/GameplayCatalog.h:584`)은 pattern 시간을 자른 한 구간이다.
구간마다 `durationMs` 시계, `stageKind`, ENTER/EXIT `actions`(flag/gauge/shield/spawn), 플레이어→보스 gate(`counterProxy`,
`bossResponse`), 보스→플레이어 hit, 그리고 outcome별 `branches`(같은 pattern의 `nextActionId` 또는 다른 pattern의
`nextPatternId`)를 가진다. presentation은 같은 `actionId`로 animation/effect/camera를 join한다.

발탄 카운터(`Data/Valtan/Valtan.gameplay.json:2876`)는 `COUNTER_1` 구간 하나에 counterable flag ENTER/EXIT,
`counterProxy BOSS_FORWARD_ARC 180`, `COUNTER_HIT -> VALTAN_GROGGY_FOLLOWUP`, `TIMEOUT -> first-fail`을 선언한다.
발탄 무력화 slot(`:2500`)은 `bossResponse ACCUMULATED_HEALTH_DAMAGE 1000`과 `HEALTH_DAMAGE_THRESHOLD_REACHED -> VALTAN_GROGGY_FOLLOWUP`이다.
Server는 `CBossCombatRuntime::Apply_PlayerHit`(`Server/Private/BossCombatRuntime.cpp:130`)이 outcome을 발행하고
`ValtanBrain.cpp:2148 ApplyPublishedOutcomeBranch`가 branch를 소비한다. 무력화 게이지형은 `SET_STAGGER_GAUGE boss.gauge.stagger` +
스킬 `staggerDamage` 누적 -> `STAGGER_BROKEN`이며 publisher가 branch stage에 ENTER/EXIT gauge action을 강제한다(`GameplayCatalog.cpp:5233`).

즉 stage는 "이 구간에서 어떤 판정 창이 열려 있고 결과가 나오면 어디로 가는가"의 Server 단위다. 사용자가 편집해야 할 개념이 아니라
box의 시간 경계에서 파생할 수 있는 개념이다.

### 1.2 쿠크 Workbench 현재 구조

| 항목 | 실측 |
|---|---|
| 문서 | `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` v2. pattern -> ordered `stages` -> `animationOccurrences`(stage-relative `startOffsetMs`) |
| C++ 문서 | `KOUKU_SAYDON_COMPOSITION_STAGE{stageId, actionId, stageKind, durationMs, AnimationOccurrences}` (`Client/Public/KoukuSaydonCompositionDocument.h`) |
| 편집 명령 | 전부 stage-scoped: `Add_Stage`, `Bind_Animation(patternId, stageId, ...)`, `Append_AnimationAsStage`, `Set_StageKind`, `Set_StageDuration`, `Move_Stage` |
| Commit | 모든 명령이 candidate 문서를 만들어 `Commit_Candidate -> CKoukuSaydonCompositionDocument::Validate`를 지나 `m_Draft` 교체 |
| Resources | `Render_ResourceTree`: 공용 `CompositionResourceTree`(Category/Profile 폴더) + Physical Clips + extracted action/stage/slot + Valtan sequence. 잎은 animation뿐 |
| Details | `Render_Details`: Pattern(name/category/authoring) -> `Add Stage`(kind/ms) -> 선택 Stage(kind/duration/up/down/delete/Bind) -> 선택 occurrence(move/trim/rate/endPolicy) |
| Timeline | `Render_Timeline`: `Stages` lane + `Animation` row(겹치면 Overlap n행). box drag/trim은 공용 `CompositionTimeline` helper |
| Shell | `CSequencerTool`이 `COMPOSITION_WORKBENCH_PANE{SEQUENCER, PATTERNS, RESOURCES, DETAILS, PREVIEW, BOSS_PATTERN, TOOLBAR}`를 session에 위임 |
| projector | `project_kouku_saydon_composition.py`: `STAGE_KEYS` exact 5개, `STAGE_KINDS` 3종, `_project_stage`가 hit 전부 0, `project_presentation`이 stage actionId별 binding |
| publisher | `Publish-GameplayBalance.ps1` 3138~3199행: stage exact 18 property, 암묵 `TIMEOUT -> 다음 stage` 행만 emit |
| Server | `CKoukuSaydonBrain::Validate_AnimationOnlyPattern`이 GROGGY/gate/actions/비-TIMEOUT branch 전부 거부, `Update`는 `durationMs`만 소비. `GameRoom::Update_KoukuSaydonBoss`(5360행)는 완료 시 Play All 다음 index |
| Client 재생 | `ClientReplication.cpp:3408` action edge마다 `Try_Resolve_Action(actionId)` -> `CNpc::Play_NetworkAction(clip, false, rate, 0.05f)`. offset/loop 없음. `KOUKU_SAYDON_ACTION_PRESENTATION{actionId, occurrenceId, clip, playMs, playRate}` |
| Boss Tool | `PRODUCT_PATTERN{patternId, displayName, category, Stages}`; audition service는 PLAY_SELECTED 중 ACTIVE pattern id가 요청 id와 다르면 ABORTED(`KoukuSaydonPatternAuditionService.cpp:330`) |

### 1.3 Effect Tool의 구조(따라갈 형식)

`CEffect_Tool`은 `Render_EffectTypeSelector`(kind radio) -> `Create Element Draft`(`Try_CreateElementDraft`, `Effect_Tool.cpp:3937`) ->
Active Document에 Element 추가 -> 선택 -> `Render_Detail`이 `Render_KindDetail/Render_TransformDetail/Render_TimingDetail`로 kind별 분기한다.
Resource Slots/Grid는 선택 Element의 slot에 WModel/DDS를 바인딩한다. 이 계획의 대응은 다음과 같다.

| Effect Tool | Workbench |
|---|---|
| Element kind | box lane(ANIMATION/LOGIC/EFFECT/SCENE_PROFILE/CAMERA/SOUND/SUMMON) |
| Create Element | Resources 탭의 `+ Add` (cursor 위치에 box 생성) |
| Resource slot 바인딩 | box payload의 자원 ID(clip, logic kind, effectAssetId, profileId, cameraCueId, eventName) |
| Render_Detail kind 분기 | `Render_BoxDetails` lane 분기 |
| Save Changes | 기존 단일 파일 CAS `Save` |

### 1.4 family별 owner와 소비자 실측

| family | owner 자원 | 목록 API | 현재 쿠크 소비자 |
|---|---|---|---|
| Animation | `Data/Animation/Reference/KoukuSaydon/*.actionreference.json`, 설치 모델 clip 헤더 | `KOUKU_SAYDON_ACTION_REFERENCE_SET`, `Set_ModelResources` | Server stage clock + `ClientReplication` 재생 (있음) |
| Logic | 없음. Server enum이 정본: `BOSS_PATTERN_STAGE_OUTCOME`, `BOSS_PATTERN_STAGE_ACTION_KIND`, `BOSS_PATTERN_COUNTER_PROXY_KIND` | 코드 상수 표 | 쿠크 Brain이 거부 (없음, G03) |
| Effect | `Data/Effects/EffectCatalog.json` 179개 authored ID | `CEffectCatalog::Get_EffectAssetIds()` (`Effect_Catalog.h:681`) | 발탄만 `CValtan::m_PatternEffectCuesByActionId`. 쿠크 CNpc 경로 없음 (G05) |
| Scene Profile / Light | `Data/Rendering/Authored/RenderingProfiles.json` profiles 6개 | `CRenderingProfileService::Has_Profile/Activate_Profile` (`RenderingProfileService.h`) | Level 진입 profile만. pattern cue 없음 (G06). MapLightGroup 자원은 아직 없음 |
| Camera | `Data/Encounters/Valtan/ValtanCinematicCamera.json` v6 cues(patternId/stageId). 쿠크 문서 없음 | `CValtanCinematicCameraController::Sample_Cue`를 `Level_KakulSaydonArena.cpp:1526`이 이미 사용 | sequence shot만. pattern cue 없음 (G07) |
| Sound | `Data/Sound/CharacterSoundCatalog.json` classes(Valtan 포함, KoukuSaydon 없음) | `CSoundCueCatalog::Collect_EventNames(class)` | 없음 (G08) |
| Summon / Combat Object | `Data/Valtan/Valtan.combatobjects.json`(발탄 전용 schema) | 없음 | Server `SPAWN_COMBAT_OBJECT` action은 있으나 쿠크 Brain 거부, hit 적용자는 `ValtanBrain.cpp:2479 ApplyPatternHit` file-static (G09) |

### 1.5 Server에서 이미 있는 것과 없는 것

있는 것: `SET_STAGGER_GAUGE`, `SET_BOSS_FLAG counterable/groggy/invulnerable`, `counterProxy`, `bossResponse`, outcome `STAGGER_BROKEN/COUNTER_HIT/HEALTH_DAMAGE_THRESHOLD_REACHED`,
`PendingPatternFollowup`, `Apply_BossPatternStageTransition`(ENTER/EXIT action 원자 적용, `GameRoom.cpp:10469`), 플레이어 hit의 공격자 위치 `fSourceX/Z`(`PlayerSkillSystem.cpp:987`, caster 위치),
HUD snapshot `BOSS_COMBAT_SNAPSHOT{iCurrentStagger, iMaximumStagger, iFlags}`.

없는 것: 정면 arc 안 공격을 "반사"하는 gate(`counterProxy`는 180도 고정이고 counterPower만 gate), Kouku Brain의 branch/follow-up/gate 해석,
`CNpc::Play_NetworkAction`의 시작 offset(`CModel::Set_AnimTrackPosition`은 있음), Kouku entity의 effect/scene/camera/sound cue 소비자.

## 2. 설계 모델

### 2.1 lane과 box

pattern은 lane 위의 box 목록이다. 모든 box는 pattern-relative `startMs`와 `durationMs`, stable `boxId`(`<patternId>.box.<n>`)를 가진다.

| lane | payload | 시간 의미 | Server 투영 | Client 투영 |
|---|---|---|---|---|
| ANIMATION | clip 참조(profileId, sourceActionId, sourceStageId, sourceSlotId, referenceRevision, runtimeClip, sourceStartMs, playRate, endPolicy) | durationMs = playMs | 시작 시각이 stage 경계 | binding(clip, sourceStartMs, loop) |
| LOGIC | `logicId`(문서 안 Logic 정의 참조) + onSuccess/onTimeout | 판정 창 [start, start+duration) | stage kind, actions, gate, branches | HUD는 snapshot으로 자동 |
| EFFECT (G05) | effectAssetId, anchorSlotId, followPolicy, localTransform | 시작 offset, 0이면 natural | 없음 | effect cue |
| SCENE_PROFILE (G06) | profileId, blendMs | 유지 구간 | 없음 | scene cue |
| CAMERA (G07) | cameraCueId | 유지 구간 | 없음 | camera cue |
| SOUND (G08) | eventName | 시작 시각 | 없음 | sound cue |
| SUMMON (G09) | combatObjectArchetypeId, volley | 시작 시각 | `SPAWN_COMBAT_OBJECT` action | replicated entity |

lane 상수 `KOUKU_BOX_LANE`은 G01에서 전부 정의하되, `FAMILY_ENABLED[lane]` 표가 true인 lane만 Resources 탭과 timeline lane에 그린다.
소비자가 없는 family를 작동하는 항목처럼 표시하지 않기 위해서다. 각 family G가 끝날 때 그 표의 값만 true로 바꾼다.

### 2.2 stage 파생 규칙 (projector와 C++ 문서가 같은 규칙)

```text
boundaries = {0, pattern.durationMs}
           ∪ { box.startMs                          | box.lane == ANIMATION }
           ∪ { box.startMs, box.startMs+durationMs  | box.lane == LOGIC }
segments   = 정렬·중복 제거한 boundaries의 연속 쌍 [b_k, b_k+1), 길이 0 제외
stage k    = { stageId "SEG_<k+1>", actionId "<patternActionId>.seg-<k+1>", durationMs b_k+1 - b_k }
```

- animation box의 끝은 경계가 아니다. clip이 끝나면 Engine이 마지막 pose를 유지한다(`Engine/Private/Animation.cpp:80`). 지금의 hold 동작과 같다.
- stage kind: segment를 덮는 LOGIC box가 `GROGGY_WINDOW`면 `GROGGY`, `COUNTER_WINDOW`면 `WINDUP`, `STAGGER_WINDOW`/`DAMAGE_THRESHOLD_WINDOW`면 `ACTIVE`, 없으면 `WINDUP`.
- 판정 창(`COUNTER_WINDOW`, `STAGGER_WINDOW`, `DAMAGE_THRESHOLD_WINDOW`)이 segment 하나를 덮으면 지금의 발탄과 같은 stage 하나가 된다.
  창 안에서 animation box가 새로 시작해 여러 segment를 덮으면 ENTER action은 첫 segment, EXIT action은 마지막 segment에 emit하고 success branch는 창 안의 모든 segment에 같은 target으로 emit한다.
  segment 사이 TIMEOUT은 다음 segment다. Server gauge/flag는 EXIT action 전까지 stage를 넘어 유지되므로 runtime 변경은 없고,
  publisher의 "같은 stage에 ENTER/EXIT gauge action" 규칙(`GameplayCatalog.cpp:5233`)만 쿠크 block에서 창 단위 검사로 바꾼다(G04).
- 같은 clip이 연속 반복되는 action(예: `att_battle_6_04` ×3)은 Append 시 하나의 `LOOP_TO_WINDOW` animation box(`durationMs` = 반복 합)로 합친다. 반복 구간 위에 창을 놓으면 segment 하나가 된다.
- `GROGGY_WINDOW`는 여러 segment를 덮을 수 있다. 각 segment의 kind가 GROGGY가 되고 action은 emit하지 않는다.
- LOGIC box끼리는 겹치지 않는다.
- branch: 창의 segment에 `onSuccess`(kind별 outcome)와 `onTimeout` branch를 emit한다. 창이 없는 segment는 지금처럼 암묵 `TIMEOUT -> 다음 segment`(마지막은 `-`)다.

branch target은 세 종류다.

| target | JSON | Server 투영 |
|---|---|---|
| `CONTINUE` | `{ "kind": "CONTINUE" }` | 다음 segment actionId (마지막이면 pattern 완료) |
| `JUMP_TO_BOX` | `{ "kind": "JUMP_TO_BOX", "boxId": "KAKULSAYDON_G1_SHIELD_STAGGER.box.4" }` | 그 box의 `startMs`에서 시작하는 segment의 actionId. box는 같은 pattern의 ANIMATION 또는 LOGIC box |
| `FOLLOWUP_PATTERN` | `{ "kind": "FOLLOWUP_PATTERN", "patternId": "KAKULSAYDON_G1_SHIELD_BREAK" }` | `nextPatternId`. 대상은 같은 문서의 PRODUCT `role FOLLOWUP` pattern |
| `END_PATTERN` | `{ "kind": "END_PATTERN" }` | 빈 target (pattern 완료) |

### 2.3 Logic 정의와 Logic kind

Logic은 pattern처럼 이름을 붙여 만드는 재사용 자원이다. composition 문서의 `logics` 배열이 정의를 소유한다.

구현 상태(2026-09-05, [RESULT](2026-09-05_KOUKU_SAYDON_LOGIC_RESOURCE_CATEGORY_RESULT.md)): 현재 v2 stage 문서 위에
`logics[]{logicId, displayName, logicType}`와 pattern별 `logicOccurrences[]{occurrenceId, logicId, startMs, durationMs}`를
읽기 선택·쓰기 필수 key로 먼저 넣었고, Resources 탭 7개, Logic 탭의 Create/Append, Logic lane과 Box Detail 수명 편집까지 닫았다.
`logicType`은 `DURATION`(판정 창), `TRIGGER`(pattern 시작 조건), `RESULT`(결과 처리)이며 아래 `scope`/`kind`는 이 type 위에
얹는 값이다. G01의 v3 migrate는 이 두 배열을 그대로 옮긴다.

```text
logic 정의 = { logicId "kakulsaydon.g1.logic.<n>", displayName, scope, kind, kind별 판정 값 }
logic box  = { boxId, lane LOGIC, startMs, durationMs, logic { logicId, onSuccess, onTimeout } }
```

- 판정 값(요구량, 각도, 반사 profile, 임계치)은 정의가 소유한다. 같은 정의를 여러 pattern에 Append하면 같은 판정을 쓴다.
- 시간과 branch target은 box가 소유한다. "성공하면 어느 pattern/box로 가는가"는 놓인 pattern에 따라 다르기 때문이다.
- `scope`는 `WINDOW`(timeline 판정 창)와 `TRIGGER`(pattern 시작 조건, 예: HP 50% crossing)다. TRIGGER는 timeline lane이 아니라 pattern header slot에 Append되며
  Server pattern selection이 소비한다. 이 계획의 G02~G04는 `WINDOW`만 구현하고 `TRIGGER`는 kind 표에 예약만 둔다(현재 쿠크 encounter는 `AUDITION_ONLY`).
- 결과 처리(예: 실패 시 전원 삐에로 변신, 전멸 hit)는 branch target과 별도의 outcome action이다. G09 Collider와 후속 Server action kind(`SET_PLAYER_TRANSFORM`)로 늘리며 이 계획에서는 target(`JUMP_TO_BOX`/`FOLLOWUP_PATTERN`)만 지원한다.

| kind | 값 | Server 투영 | success outcome |
|---|---|---|---|
| `COUNTER_WINDOW` | 없음 | ENTER/EXIT `SET_BOSS_FLAG boss.flag.counterable`, `counterProxy BOSS_FORWARD_ARC 180` | `COUNTER_HIT` |
| `STAGGER_WINDOW` | `staggerRequired`(1..1000000), optional `attackSourceGate{arcDegrees, reflectDamageProfileId}` | ENTER `SET_STAGGER_GAUGE boss.gauge.stagger = staggerRequired`, EXIT `= 0`, `PATTERNSTAGESOURCEGATE` | `STAGGER_BROKEN` |
| `DAMAGE_THRESHOLD_WINDOW` | `threshold`(1..) | `bossResponse ACCUMULATED_HEALTH_DAMAGE` | `HEALTH_DAMAGE_THRESHOLD_REACHED` (onSuccess는 FOLLOWUP_PATTERN만, 대상 첫 segment GROGGY) |
| `GROGGY_WINDOW` | 없음 | segment kind `GROGGY` | 없음 (onTimeout만) |

새 kind는 이 표에 행을 추가하고 projector/publisher/Brain validate/Workbench Details 네 곳을 같은 변경 단위로 늘린다. 문자열 fallback으로 모르는 kind를 통과시키지 않는다.

### 2.4 정면 반사 gate (새 Server primitive)

`attackSourceGate`는 stage 소유 optional 값이다. `counterProxy`를 재사용하지 않는 이유는 그 gate가 counterable 창의 counterPower만 걸러내고 180도가 고정이기 때문이다.

```text
플레이어 스킬 hit(공격자 위치 fSourceX/Z)
→ CServerCombatHitRuntime::Apply_PlayerToWorld
→ CBossCombatRuntime::Apply_PlayerHit
   invulnerable 검사 뒤, boss.bPatternHasAttackSourceGate이고 공격자가 boss yaw 기준 정면 arcDegrees 안이면
   result.bReflected = true로 즉시 반환 (shield/HP/bossResponse/stagger/part 전부 미적용)
→ Apply_PlayerToWorld가 SERVER_COMBAT_HIT_RESULT::REFLECTED 반환
→ PlayerSkillSystem applyDamage가 SERVER_WORLD_TO_PLAYER_HIT{iRawDamage = Resolve_Damage(boss attackPower, reflect profile rate), fSource = boss 위치}로
   CServerCombatHitRuntime::Apply_WorldToPlayer(공격자)
```

플레이어 피해·사망·damage event는 이미 `Apply_WorldToPlayer` 한 곳이 소유하므로 두 번째 플레이어 피해 경로를 만들지 않는다.

### 2.5 화면과 명령

```text
Resources 창
  [Animation] [Logic] [Effect] [Scene Profile] [Camera] [Sound] [Summon]   (FAMILY_ENABLED만 표시)
  탭마다: 검색 → 목록(공용 CompositionResourceTree) → 선택 → Preview(가능한 것만) → "+ Add at Cursor"
  Logic 탭만 이름·kind로 "Create Logic"을 가진다(다른 family의 정의는 Effect Tool, Camera Tool, Rendering Workbench 같은 owner tool이 만든다)
Sequencer 창
  ruler / Stages(파생, 읽기 전용) / Animation rows / Logic / Effect / Scene Profile / Camera / Sound / Summon
  lane 머리의 "+"는 그 family Resources 탭으로 focus (COMPOSITION_WORKBENCH_VIEW_REQUEST)
Box Detail 창
  Pattern(name, category, authoring, role, durationMs)
  선택 box lane별 Details (Animation: move/trim/rate/endPolicy, Logic: kind 값·onSuccess·onTimeout, 나머지는 family G에서)
  Server Segments: 파생 stage 표(읽기 전용)
```

기존 `Add Stage`, `Stage Kind`, `Stage Up/Down`, `Delete Stage`, `Bind Selected Animation`, `Append as Stage`, `Add Animation Row`, `Append Action as Stages`,
`Append Action to Selected Stage`는 제거하거나 `+ Add`로 바꾼다. `Append Action`은 action의 slot들을 cursor부터 이어 붙이는 여러 box 추가로 유지한다.

## 3. 변경할 파일

새 C++ 파일은 G09(Server 공용 hit runtime)에서만 생긴다. 그 외 G는 기존 파일만 바꾸므로 `.vcxproj`/`.filters` 변경이 없다.

| G | 파일 | 역할 |
|---|---|---|
| G01 | `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | v3 migrate 결과(revision 3) |
| G01 | `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | v3 key/validate, `--mode migrate-v2`, stage 파생, encounter/presentation 투영 |
| G01 | `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` | v3 규칙, Pizza 동등성, 파생 규칙 테스트 |
| G01 | `Client/Public/KoukuSaydonCompositionDocument.h`, `Client/Private/KoukuSaydonCompositionDocument.cpp` | box 구조체, v3 parse/serialize/validate, `Derive_Segments` |
| G01 | `Client/Private/KoukuSaydonActionWorkbench.cpp`, `Client/Public/KoukuSaydonActionWorkbench.h` | 명령을 box 기준으로 교체(G01은 Animation만), preview 요청 box 기준 |
| G01 | `Client/Private/MainApp.cpp` (1240행 부근 Kouku pattern preview 소비) | box `startMs` 기준 로컬 preview |
| G01 | `Client/Public/KoukuSaydonBossTool.h`, `Client/Private/KoukuSaydonBossTool.cpp` | Product stage 목록은 파생 stage를 그대로 읽음(변경 최소) |
| G02 | `Client/Public/KoukuSaydonActionWorkbench.h`, `Client/Private/KoukuSaydonActionWorkbench.cpp` | Resources 탭 바, `+ Add`, lane별 timeline, `Render_BoxDetails`, Logic Details |
| G02 | `Client/Private/SequencerTool.cpp` | lane "+"의 view request를 Resources 탭 focus로 전달 |
| G03 | `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp` | `attackSourceGate` stage 필드와 `PATTERNSTAGESOURCEGATE` 행 parse/validate |
| G03 | `Server/Public/ServerWorldEntity.h` | 실행 중 gate 값 mirror 필드 |
| G03 | `Server/Public/BossCombatRuntime.h`, `Server/Private/BossCombatRuntime.cpp` | `bReflected`, gate 판정 |
| G03 | `Server/Public/ServerCombatHitRuntime.h`, `Server/Private/ServerCombatHitRuntime.cpp` | `REFLECTED` 결과 |
| G03 | `Server/Private/PlayerSkillSystem.cpp` | 반사 damage를 공격자에게 적용 |
| G03 | `Server/Public/KoukuSaydonBrain.h`, `Server/Private/KoukuSaydonBrain.cpp` | generic 해석기: GROGGY, actions, gate, branch, follow-up |
| G03 | `Server/Private/GameRoom.cpp` (`Update_KoukuSaydonBoss`, `Apply_BossPatternStageTransition` 호출, `Clear_KoukuSaydonPatternAudition`) | stage 전환 시 ENTER/EXIT action 원자 적용, follow-up 체인 |
| G03 | `Server/Private/ServerGameplayContractTests.cpp` | 쿠크 Brain 검사 |
| G04 | `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`, 테스트 | Logic box 투영(actions/gate/branches/followup), role 기반 playAll |
| G04 | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` (2994~3213행 Kouku block) | 새 행 emit과 검사 |
| G04 | `Data/Balance/DamageProfiles.json`, `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` | `damage.kakulsaydon.shield-reflect` (`PROJECT_TUNED`) |
| G04 | `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | 방패 무력화 3 pattern 저작 |
| G04 | `Client/Public/KoukuSaydonPresentationAssetService.h`, `Client/Private/KoukuSaydonPresentationAssetService.cpp` | binding v2(`sourceStartMs`, `loop`) admission |
| G04 | `Client/Public/Npc.h`, `Client/Private/Npc.cpp` | `Play_NetworkAction` 시작 offset overload |
| G04 | `Client/Private/ClientReplication.cpp` (3395~3435행) | offset/loop 재생 |
| G04 | `Client/Public/KoukuSaydonBossTool.h`, `Client/Private/KoukuSaydonBossTool.cpp`, `Client/Public/KoukuSaydonPatternAuditionService.h`, `Client/Private/KoukuSaydonPatternAuditionService.cpp` | follow-up pattern id를 같은 audition으로 인정 |
| G05 | projector, `KoukuSaydonPresentationAssetService.*`, `ClientReplication.cpp`, `Level_KakulSaydonArena.cpp`, Workbench | Effect lane과 `KoukuSaydon.patterneffectcues.json` |
| G06 | projector, `ClientReplication.cpp`, `MainApp.cpp`, Workbench | Scene Profile lane과 `KoukuSaydon.patternscenecues.json` |
| G07 | `Data/Encounters/KoukuSaydon/KoukuSaydonCinematicCamera.json`(신규 데이터), projector, `Level_KakulSaydonArena.cpp`, Workbench | Camera lane |
| G08 | `Data/Sound/CharacterSoundCatalog.json`(class `KoukuSaydon`), projector, `ClientReplication.cpp`, Workbench | Sound lane |
| G09 | `Server/Private/BossPatternHitRuntime.cpp`, `Server/Public/BossPatternHitRuntime.h`(신규, `Server/Default/Server.vcxproj`와 `.filters` 등록), `ValtanBrain.cpp`, `KoukuSaydonBrain.cpp`, `Data/KoukuSaydon/Gate1/KoukuSaydonCombatObjects.json`(신규), publisher, Workbench | Summon/Collider lane |
| 문서 | `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, `CLAUDE.md`의 Action Workbench 단락, 대응 RESULT | public 계약 변경 시에만 |

## 4. 데이터와 호출 흐름

### 4.1 composition v3 골격

```json
{
  "schema": "lostark.kouku-saydon-composition",
  "formatVersion": 3,
  "revision": 3,
  "compositionId": "boss.composition.kakulsaydon.gate1",
  "encounterId": "ENCOUNTER_KAKULSAYDON_G1",
  "bossArchetypeId": "BOSS_KAKULSAYDON_G1_KOUKU",
  "bossPlacementId": "boss.kakulsaydon.g1.kouku",
  "areaId": "LV_LUT_MIDNIGHTC_ED",
  "fixedTickHz": 30,
  "nextPatternOrdinal": 1,
  "playAllPatternIds": ["KAKULSAYDON_G1_PIZZA", "KAKULSAYDON_G1_SHIELD_STAGGER"],
  "patterns": []
}
```

header 값은 현재 v2 문서의 실제 값이며 migrate가 `formatVersion`과 `revision`만 바꾸고 `logics: []`, `nextLogicOrdinal: 1`을 추가한다. `playAllPatternIds`의 두 번째 항목은 G04에서 방패 무력화가 PRODUCT가 될 때 들어간다.
문서 최상위에는 `logics`(§2.3 정의 배열)와 `nextLogicOrdinal`(logicId 발급 카운터)이 더 있고, pattern은 다음 key만 가진다.

```text
patternId, actorProfileId, displayName, authoringStatus, category, role, durationMs, nextBoxOrdinal, boxes
```

`role`은 `ROOT` 또는 `FOLLOWUP`. `playAllPatternIds`는 PRODUCT이면서 ROOT인 pattern만 authored 순서로 담는다.
`durationMs`는 모든 box 끝 이상이어야 하며 마지막 segment가 여기까지 늘어난다. `nextBoxOrdinal`은 `boxId` 발급 카운터다.

### 4.2 방패 무력화 저작값 (G04에서 저장)

```json
{
  "patternId": "KAKULSAYDON_G1_SHIELD_STAGGER",
  "actorProfileId": "MN_RPCZ_00",
  "displayName": "1관문 / 방패 무력화 패턴",
  "authoringStatus": "PRODUCT",
  "category": "MECHANIC",
  "role": "ROOT",
  "durationMs": 16634,
  "nextBoxOrdinal": 5,
  "boxes": [
    {
      "boxId": "KAKULSAYDON_G1_SHIELD_STAGGER.box.1",
      "lane": "ANIMATION",
      "startMs": 0,
      "durationMs": 1467,
      "animation": {
        "profileId": "MN_RPCZ_00",
        "sourceActionId": 42197100,
        "sourceStageId": "stage-000",
        "sourceSlotId": "animation-000",
        "referenceRevision": "8d2e0188d80efc498123d497cddc7b660abbea784220a53aa5cf644803af8745",
        "runtimeClip": "rpcz00_att_battle_14_01",
        "sourceStartMs": 0,
        "playRate": 1.0,
        "endPolicy": "EXACT"
      }
    },
    {
      "boxId": "KAKULSAYDON_G1_SHIELD_STAGGER.box.2",
      "lane": "ANIMATION",
      "startMs": 1467,
      "durationMs": 667,
      "animation": {
        "profileId": "MN_RPCZ_00",
        "sourceActionId": 42197100,
        "sourceStageId": "stage-001",
        "sourceSlotId": "animation-000",
        "referenceRevision": "8d2e0188d80efc498123d497cddc7b660abbea784220a53aa5cf644803af8745",
        "runtimeClip": "rpcz00_att_battle_14_02",
        "sourceStartMs": 0,
        "playRate": 1.0,
        "endPolicy": "HOLD_LAST_POSE"
      }
    },
    {
      "boxId": "KAKULSAYDON_G1_SHIELD_STAGGER.box.3",
      "lane": "LOGIC",
      "startMs": 1467,
      "durationMs": 12000,
      "logic": {
        "logicId": "kakulsaydon.g1.logic.1",
        "onSuccess": { "kind": "FOLLOWUP_PATTERN", "patternId": "KAKULSAYDON_G1_SHIELD_BREAK" },
        "onTimeout": { "kind": "CONTINUE" }
      }
    },
    {
      "boxId": "KAKULSAYDON_G1_SHIELD_STAGGER.box.4",
      "lane": "ANIMATION",
      "startMs": 13467,
      "durationMs": 3167,
      "animation": {
        "profileId": "MN_RPCZ_00",
        "sourceActionId": 42197100,
        "sourceStageId": "stage-002",
        "sourceSlotId": "animation-000",
        "referenceRevision": "8d2e0188d80efc498123d497cddc7b660abbea784220a53aa5cf644803af8745",
        "runtimeClip": "rpcz00_att_battle_14_03",
        "sourceStartMs": 0,
        "playRate": 1.0,
        "endPolicy": "EXACT"
      }
    }
  ]
}
```

box.3이 참조하는 Logic 정의는 문서 최상위 `logics`에 있다.

```json
{
  "logicId": "kakulsaydon.g1.logic.1",
  "displayName": "방패 무력화",
  "scope": "WINDOW",
  "kind": "STAGGER_WINDOW",
  "staggerRequired": 400,
  "attackSourceGate": {
    "arcDegrees": 90.0,
    "reflectDamageProfileId": "damage.kakulsaydon.shield-reflect"
  }
}
```

`staggerRequired 400`은 스킬 `staggerDamage 10` 기준 40회 타격이며 첫 값이다. 같은 정의를 다른 pattern(예: 3관문 `쿠크세이튼_무력화 시작` 4219945의 `att_battle_6_04` ×3 반복 구간)에 Append하면 판정은 같고 시간과 branch만 그 pattern의 box가 가진다.
단, PRODUCT publish와 Server 재생은 현재 Gate 1 Kouku 본체(`MN_RPCZ_00`)만 가능하므로 Saydon 본체 pattern은 DRAFT 저작과 로컬 preview까지다. follow-up 두 pattern은 다음과 같다.

```text
KAKULSAYDON_G1_SHIELD_BREAK  (FOLLOWUP, durationMs 2000)
  box.1 ANIMATION 0..2000   rpcz00_att_battle_14_04 (42197101 stage-000)
  box.2 LOGIC     0..2000   GROGGY_WINDOW, onTimeout FOLLOWUP_PATTERN KAKULSAYDON_COMMON_GROGGY
KAKULSAYDON_COMMON_GROGGY    (FOLLOWUP, durationMs 8500)
  box.1 ANIMATION 0..1000     rpcz00_dmg_critical_start_1 (4219763 stage-000)
  box.2 ANIMATION 1000..7333  rpcz00_dmg_critical_loop_1  (4219763 stage-001, LOOP_TO_WINDOW, playMs 6333)
  box.3 ANIMATION 7333..8500  rpcz00_dmg_critical_end_1   (4219763 stage-002)
  box.4 LOGIC     0..8500     GROGGY_WINDOW, onTimeout END_PATTERN
```

`sourceStageId/sourceSlotId/referenceRevision`은 Workbench Resources에서 Append했을 때의 값을 그대로 쓴다. 손으로 JSON을 고치면 `revision`을 올려야 CAS baseline이 된다.

### 4.3 파생 결과 (projector가 encounter Product에 쓰는 stage)

```text
KAKULSAYDON_G1_SHIELD_STAGGER  boundaries {0, 1467, 13467, 16634}
  SEG_1  kakulsaydon.g1.shield.stagger.seg-1  WINDUP  1467   binding 14_01
  SEG_2  kakulsaydon.g1.shield.stagger.seg-2  ACTIVE  12000  binding 14_02 hold
         actions ENTER SET_STAGGER_GAUGE boss.gauge.stagger 400 / EXIT SET_STAGGER_GAUGE boss.gauge.stagger 0
         attackSourceGate BOSS_FORWARD_ARC 90 damage.kakulsaydon.shield-reflect
         branches STAGGER_BROKEN -> nextPatternId KAKULSAYDON_G1_SHIELD_BREAK, TIMEOUT -> seg-3
  SEG_3  kakulsaydon.g1.shield.stagger.seg-3  WINDUP  3167   binding 14_03, TIMEOUT -> -
```

bootstrap 행(G04 publisher):

```text
PATTERNSTAGE           ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_STAGGER  1  SEG_2  kakulsaydon.g1.shield.stagger.seg-2  ACTIVE  12000  NONE 0 0 0 0 0 0 0 0 - 0 0 0 0
PATTERNSTAGEACTION     ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_STAGGER  kakulsaydon.g1.shield.stagger.seg-2  0  ENTER  SET_STAGGER_GAUGE  boss.gauge.stagger  400  0
PATTERNSTAGEACTION     ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_STAGGER  kakulsaydon.g1.shield.stagger.seg-2  1  EXIT   SET_STAGGER_GAUGE  boss.gauge.stagger  0    0
PATTERNSTAGESOURCEGATE ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_STAGGER  kakulsaydon.g1.shield.stagger.seg-2  BOSS_FORWARD_ARC  90  damage.kakulsaydon.shield-reflect
PATTERNSTAGEFOLLOWUP   ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_STAGGER  kakulsaydon.g1.shield.stagger.seg-2  STAGGER_BROKEN  KAKULSAYDON_G1_SHIELD_BREAK
PATTERNSTAGEBRANCH     ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_SHIELD_STAGGER  kakulsaydon.g1.shield.stagger.seg-2  TIMEOUT  kakulsaydon.g1.shield.stagger.seg-3
PATTERNSEQUENCESTEP    ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_PLAY_ALL  0  KAKULSAYDON_G1_PIZZA           100
PATTERNSEQUENCESTEP    ENCOUNTER_KAKULSAYDON_G1  KAKULSAYDON_G1_PLAY_ALL  1  KAKULSAYDON_G1_SHIELD_STAGGER  0
```

`PATTERNSTAGEACTION`의 열은 기존 파서(`GameplayCatalog.cpp:3313`)가 요구하는 10열 `encounter pattern action order trigger kind target value durationMs`다. `PATTERNSTAGESOURCEGATE`는 G03이 추가하는 행이다.

### 4.4 Server 실행 흐름 (G03)

```text
Boss Tool Play Selected(KAKULSAYDON_G1_SHIELD_STAGGER)
→ CGameRoom::Evaluate_KoukuSaydonPatternAudition (기존)
→ Update_KoukuSaydonBoss: PENDING → Begin_Pattern → Apply_BossPatternStageTransition("", "", pattern, seg-1) (ENTER action 적용)
→ CKoukuSaydonBrain::Update 매 tick
   ├─ Enter_Stage가 boss.bPatternHasAttackSourceGate/arc/reflect profile, bPatternGroggy를 stage 값으로 설정
   ├─ 플레이어 hit → Apply_PlayerHit: 정면 arc면 bReflected → 공격자 피해; 아니면 stagger 누적 → STAGGER_BROKEN 발행
   ├─ Update: stage.Branches 중 TIMEOUT이 아닌 outcome을 Consume_PatternOutcome으로 소비 → Apply_Branch
   │    └─ nextPatternId → boss.PendingPatternFollowup 예약 + Finish_Pattern(COMPLETED) → FOLLOWUP_QUEUED
   └─ durationMs 경과: TIMEOUT branch → nextActionId(Enter_Stage) | nextPatternId | 없음(완료)
→ Update_KoukuSaydonBoss: STAGE_CHANGED면 Apply_BossPatternStageTransition(prev, next) (EXIT/ENTER 원자 적용; 실패는 audition ABORTED)
→ FOLLOWUP_QUEUED / PATTERN_COMPLETED + PendingPatternFollowup
   → lifecycle PATTERN_COMPLETED(완료 pattern) → 다음 tick Begin_Pattern(follow-up, pinned catalog) → lifecycle ACTIVE(follow-up id)
   → 체인 끝: Play All 다음 index 또는 COMPLETED
```

`Apply_BossPatternStageTransition`은 실패 시 `CValtanBrain::Fail_Mechanic`을 호출한다. 쿠크 boss에는 mechanic ledger가 없으므로 이 호출은 archetype이 `BOSS_VALTAN`일 때만 하도록 guard하고, 쿠크 경로는 false 반환을 받아 audition을 ABORTED로 닫는다.

### 4.5 Client 실행 흐름 (G04)

```text
S2C_WORLD_SNAPSHOT action edge (actionId 변경)
→ ClientReplication.cpp:3408 Try_Resolve_Action(actionId) → KOUKU_SAYDON_ACTION_PRESENTATION{clip, sourceStartMs, playRate, loop}
→ CNpc::Play_NetworkAction(clip, loop, rate, 0.05f, sourceStartMs)
   → CModel::Set_Animation → Start_Animation → Set_AnimTrackPosition(Get_CurrentAnimIndex(), sourceStartTicks)
→ binding이 없는 segment는 지금처럼 Play_DefaultIdle
→ HUD: BOSS_COMBAT_SNAPSHOT.iCurrentStagger/iMaximumStagger를 CCombatHUDViewModel이 이미 소비
```

binding 규칙(projector `project_presentation`): segment 시작에서 시작하는 animation box가 있으면 그 box; 없고 직전 box가 segment 시작을 덮으면
같은 clip의 continuation binding(`sourceStartMs = box.sourceStartMs + (segmentStart - box.startMs) * playRate`); 둘 다 없으면 binding 없음.
`LOOP_TO_WINDOW`는 `loop true`. `HOLD_LAST_POSE`는 `loop false`(Engine hold). presentation 문서 formatVersion 2.

### 4.6 Workbench 명령 흐름 (G02)

```text
Resources 탭 Logic → 이름 "방패 무력화" + kind STAGGER_WINDOW → "Create Logic"
→ CKoukuSaydonActionWorkbench::Create_Logic(displayName, kind)
   → candidate = m_Draft 복사, logicId = kakulsaydon.g1.logic.<nextLogicOrdinal++>, kind 기본값 → Commit_Candidate
Selected Logic 패널에서 요구량/90도 반사 편집 → Set_LogicDefinitionValues → Commit_Candidate
목록에서 그 Logic 선택 → "Append Logic at Cursor"
→ CKoukuSaydonActionWorkbench::Add_LogicBox(patternId, logicId, startMs=cursor, durationMs=1000)
   → candidate = m_Draft 복사, boxId = <patternId>.box.<nextBoxOrdinal++>, onSuccess/onTimeout 기본 CONTINUE
   → Commit_Candidate → CKoukuSaydonCompositionDocument::Validate(v3 규칙 + Derive_Segments 성공)
   → 성공: m_Draft 교체, dirty, 선택 = 새 boxId / 실패: draft 보존, 상태 메시지
Box Detail(Logic) 시간·분기 편집 → Move_Box / Trim_Box / Set_LogicBranch → Commit_Candidate
timeline drag/trim → Move_Box / Trim_Box → Commit_Candidate (segment 단일성 규칙 위반이면 거부하고 원위치)
Save → 기존 CKoukuSaydonCompositionDocument::Save_Atomic (revision+1, CAS)
Publish All PRODUCT → 기존 publisher process → projector v3
```

## 5. G별 구현 범위

### G01 — 문서 v3, migrate, stage 파생 (동작 변화 없음)

**목표**: stage 저작 문서를 lane/box 문서로 바꾸되 Pizza Product(6 stage, clip 순서, duration)가 바이트 단위로 동등하게 다시 투영된다.

**projector**

- `DOCUMENT_KEYS`에 `logics`, `nextLogicOrdinal`을 더하고 `PATTERN_KEYS`를 §4.1로 바꾼다. `BOX_COMMON_KEYS = {boxId, lane, startMs, durationMs}` + lane별 payload key(`animation`, `logic`)를 정의한다.
  `LOGIC_DEFINITION_KEYS = {logicId, displayName, scope, kind}` + kind별 값 key, `LOGIC_BOX_KEYS = {logicId, onSuccess, onTimeout}`. box의 `logicId`는 같은 문서의 정의를 가리켜야 하고 정의의 `logicId`는 `kakulsaydon.g1.logic.<n>` (n < nextLogicOrdinal)이다.
  `STAGE_KEYS`, `OCCURRENCE_KEYS`는 제거한다. `LANES = {ANIMATION, LOGIC}`(G05~G09에서 추가), `LOGIC_SCOPES = {WINDOW, TRIGGER}`, `LOGIC_KINDS` = §2.3, `BRANCH_TARGET_KINDS = {CONTINUE, JUMP_TO_BOX, FOLLOWUP_PATTERN, END_PATTERN}`.
- `--mode migrate-v2`: v2 문서를 읽어 stage 누적 시작 + `startOffsetMs`를 `startMs`로, occurrence를 ANIMATION box로, `stages`를 제거하고 `durationMs = 누적 stage 길이`, `role ROOT`, `formatVersion 3`, `revision + 1`로 쓴다.
  Pizza의 stage kind는 버린다(파생 규칙이 WINDUP으로 다시 만든다). 한 번만 실행하며 v2 입력을 계속 받지 않는다.
- `derive_segments(pattern)`: §2.2 규칙. 단일 segment 창 규칙과 LOGIC 겹침 금지를 여기서 검사한다.
- `_project_stage`가 파생 segment를 받아 지금과 같은 필드를 쓴다. 이 G에서는 LOGIC payload가 있어도 PRODUCT면 거부한다(`CompositionError("logic boxes are not publishable before G04")`).
- `project_presentation`이 §4.5 binding 규칙으로 쓰되 이 G에서는 continuation과 loop가 생기면 PRODUCT 거부(G04에서 해제).
- `validate_publishable`: playAll = PRODUCT ∧ ROOT.

**테스트**: `test_seed_is_the_exact_six_stage_pizza_reference`를 "migrate 뒤 파생 Product의 stages/bindings가 현재 tracked Product와 duration/clip/순서가 같다"로 바꾼다.
`test_product_requires_one_full_stage_animation`, `test_product_projection_is_minimal_and_timeout_only`는 box 기준으로 옮긴다. 새 테스트: 경계 파생, 창 단일성 위반 거부, LOGIC 겹침 거부, JUMP_TO_BOX가 경계 아닌 box를 가리키면 거부.

**C++ 문서**

- `KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE`를 `KOUKU_SAYDON_COMPOSITION_ANIMATION_BOX`(startOffsetMs 제거, playMs는 box durationMs)로,
  `KOUKU_SAYDON_COMPOSITION_STAGE`를 제거하고 `KOUKU_SAYDON_COMPOSITION_BOX{boxId, eLane, startMs, durationMs, Animation, Logic}`을 둔다.
  `KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{logicId, displayName, scope, kind, staggerRequired, threshold, hasAttackSourceGate, arcDegrees, reflectDamageProfileId}`(문서 최상위 `Logics`),
  `KOUKU_SAYDON_COMPOSITION_LOGIC_BOX{logicId, onSuccess, onTimeout}`, `KOUKU_SAYDON_BRANCH_TARGET{kind, boxId, patternId}`.
- 문서에 `Logics`, `iNextLogicOrdinal`; pattern에 `strRole`, `iDurationMs`, `iNextBoxOrdinal`, `Boxes`. Validate는 box의 `logicId`가 정의에 있어야 하고, 정의 삭제는 참조 box가 0개일 때만 허용한다.
- `KOUKU_SAYDON_DERIVED_SEGMENT{stageId, actionId, stageKind, startMs, durationMs, logicBoxId, animationBoxId, continuationBoxId}`와 `static bool_t Derive_Segments(pattern, out, status)`. Validate_Shape가 이를 호출한다.
- `Parse_Text`/`Serialize`는 v3만 읽고 쓴다. `Validate_Shape` 규칙: §2.2 + boxId ordinal ≤ nextBoxOrdinal, startMs+durationMs ≤ pattern.durationMs ≤ 600000, PRODUCT는 box ≤ 256/segment ≤ 64.

**Workbench**(G01 범위는 Animation만 box로)

- `Add_Stage/Set_StageKind/Set_StageDuration/Move_Stage/Delete_Stage/Append_AnimationAsStage/Bind_Animation/Move_AnimationToStage`를 제거하고
  `Add_AnimationBox(patternId, source, startMs)`, `Append_ActionBoxes(patternId, profileId, actionId, startMs)`, `Move_Box`, `Trim_AnimationBox`, `Set_AnimationPlayback`, `Duplicate_Box`, `Delete_Boxes`, `Set_PatternDuration`, `Set_PatternRole`로 바꾼다.
  `Delete_TimelineSelection/Duplicate_TimelineSelection`은 boxId 목록만 받는다.
- `Render_Timeline`: Stages lane은 `Derive_Segments` 결과를 읽기 전용으로 그린다. Animation rows는 box `startMs`로 배치한다(겹침 행 계산 로직은 그대로).
- `Render_Details`: Stage 단락을 `Server Segments` 읽기 전용 표로 바꾼다. Animation 단락은 box 필드.
- preview: `Consume_PatternPreviewRequest`가 box 기준 pattern을 넘기고 `MainApp.cpp` 1240행 부근 소비자가 `startMs`로 재생한다.
- Boss Tool `PRODUCT_STAGE`는 파생 stage를 Product에서 읽으므로 변경 없음.

**검증**: projector migrate/publish/validate, unittest(PYTHONPATH=. UTF-8), `KoukuSaydonCompositionDocument.cpp`·`KoukuSaydonActionWorkbench.cpp`·`MainApp.cpp` Debug x64 ClCompile, Client link 뒤
사용자: Reload → Pizza 6 box 표시 → drag/trim → Save → Reload 동일, Preview, Boss Tool Play Selected(Pizza)가 이전과 같이 재생.

### G02 — Resources 카테고리 탭, `+ Add`, lane별 Details

- `Render_ResourceTree`를 `Render_ResourcesWindow`의 탭 바로 감싼다: `KOUKU_RESOURCE_CATEGORY{ANIMATION, LOGIC, EFFECT, SCENE_PROFILE, CAMERA, SOUND, SUMMON}`. `FAMILY_ENABLED`가 false인 탭은 그리지 않는다(G02 시점 true: ANIMATION, LOGIC).
- Animation 탭: 기존 트리 유지. `Append as Stage`/`Add Animation Row`/`Append Action as Stages`/`Append Action to Selected Stage`를 `+ Add at Cursor`(clip 1개 또는 action slot 전체)로 바꾼다. 겹치면 새 행이다.
- Logic 탭: Patterns 창의 `Create Pattern`과 같은 형식으로 `이름 입력 + kind combo + Create Logic`(`Create_Logic` → 문서 `Logics`에 정의 추가, `logicId = kakulsaydon.g1.logic.<nextLogicOrdinal++>`).
  목록은 공용 `CompositionResourceTree`에 `Logic/<kind>/<displayName>` 잎으로 넣고, 선택하면 `Selected Logic` 패널에 kind별 값(`staggerRequired`/`threshold`, `attackSourceGate` 체크 + arc 1..179 + reflect profile ID)과 `참조 box n개`를 편집·표시한다(`Set_LogicDefinitionValues`, `Rename_Logic`, `Delete_Logic`은 참조 0개일 때만).
  `Append Logic at Cursor` → `Add_LogicBox(patternId, logicId, startMs=cursor, durationMs=1000)`.
- timeline: `Logic` lane 추가. 색은 발탄 `Lane_Color(LOGIC)`와 같은 계열. lane 머리 `+` 버튼 → `COMPOSITION_WORKBENCH_VIEW_REQUEST.focusResources` + 새 필드 `focusResourceCategory`로 그 탭 focus(`SequencerTool::Apply_ViewRequest` 전달).
- `Render_BoxDetails`: 선택 box lane으로 분기. Logic box Details: 참조 정의 이름·kind·판정 값(읽기 전용, `Open in Resources` 버튼으로 정의 편집), `Start ms`/`Duration ms`,
  `onSuccess`/`onTimeout` combo(target kind) + 대상 combo(JUMP_TO_BOX는 같은 pattern의 box 목록, FOLLOWUP_PATTERN은 PRODUCT ∧ FOLLOWUP pattern 목록).
  변경마다 `Move_Box`/`Trim_Box`/`Set_LogicBranch` → `Commit_Candidate`. 판정 값은 정의 하나에서만 바꾸므로 같은 Logic을 쓰는 모든 box가 함께 바뀐다.
- Pattern Details에 `Role` combo와 `Duration ms`.
- Server Segments 표에 각 segment의 kind/actions/branch 요약을 파생 결과로 표시한다(사용자가 Server가 받을 값을 확인).
- Save는 그대로. PRODUCT로 바꾸려는 pattern에 Logic box가 있으면 G04 전까지 projector가 publish에서 거부하고 Workbench는 그 상태 메시지를 그대로 보여 준다.

**검증**: 두 C++ 파일 ClCompile, `test_kouku_saydon_client_product_level_contract.py`의 공용 트리 검사 2개, 사용자: Logic 탭 → Stagger Window Add → Details 편집 → Save → JSON이 §4.2 box.3과 같음(값 제외) → Reload.

### G03 — Server generic 해석기와 정면 반사 gate

**GameplayCatalog**

- `BOSS_PATTERN_STAGE_DEFINITION`에 `bHasAttackSourceGate`, `fAttackSourceGateArcDegrees`, `strAttackSourceGateReflectDamageProfileId` 추가(`fCounterProxyArcDegrees` 바로 아래).
- `PATTERNSTAGESOURCEGATE encounter pattern action BOSS_FORWARD_ARC arc profile` 행 parse(`PATTERNSTAGECOUNTERPROXY` 분기 바로 아래). validate: ACTIVE stage만, arc 1..179, profile은 damage profile 존재, stage당 1개.
- publisher validate(4700행대)에 `STAGGER_BROKEN` follow-up 대상 규칙은 추가하지 않는다(현재 규칙 유지).

**ServerWorldEntity**: `bPatternHasAttackSourceGate`, `fPatternAttackSourceGateArcDegrees`, `strPatternAttackSourceGateReflectDamageProfileId`(`bPatternHasCounterProxy` 아래).

**BossCombatRuntime**: `BOSS_HIT_RESULT.bReflected`; `Apply_PlayerHit`이 invulnerable 검사 뒤 gate 판정(file-static `ContainsAttackSourceGate`: boss yaw forward와 source 방향의 각이 arc/2 이하)이면 `bReflected=true` 반환.

**ServerCombatHitRuntime**: `SERVER_COMBAT_HIT_RESULT::REFLECTED`; `Apply_PlayerToWorld`가 `bReflected`면 damage event 없이 REFLECTED 반환.

**PlayerSkillSystem**: file-static `ApplyPlayerHitDamage`가 결과를 반환하고, caster/projectile 두 호출자가 REFLECTED면 `SERVER_WORLD_TO_PLAYER_HIT{rawDamage = Resolve_Damage(bossProfile->iAttackPower, Find_DamageRatePercent(reflect profile)), fSource = boss 위치, push 0, knockdown false}`로 `Apply_WorldToPlayer(player)`를 호출한다.

**KoukuSaydonBrain**

- `KOUKUSAYDON_BRAIN_UPDATE_RESULT::FOLLOWUP_QUEUED` 추가(`PATTERN_COMPLETED` 아래).
- `Validate_AnimationOnlyPattern` → `Validate_AuditionPattern`으로 이름 변경. 허용 범위: stage kind + GROGGY, actions는 `SET_STAGGER_GAUGE boss.gauge.stagger`와 `SET_BOSS_FLAG boss.flag.counterable`(ENTER/EXIT 쌍), `counterProxy BOSS_FORWARD_ARC 180`, `bossResponse ACCUMULATED_HEALTH_DAMAGE`(ACTIVE), `attackSourceGate`(ACTIVE), branch outcome `TIMEOUT/COUNTER_HIT/STAGGER_BROKEN/HEALTH_DAMAGE_THRESHOLD_REACHED`(nextActionId=같은 pattern 또는 nextPatternId 또는 빈 값). hit/motion/spawn/counter 이외 action은 계속 거부(G09까지).
- `Enter_Stage`: counterProxy/bossResponse/attackSourceGate/`bPatternGroggy`를 stage 값으로 설정. bossResponse 변경은 `boss.BossCombat.iStateRevision` 증가.
- `Update`: durationMs 검사 전에 `stage.Branches` 중 TIMEOUT이 아닌 outcome을 `CBossCombatRuntime::Consume_PatternOutcome`으로 소비 → `Apply_Branch`. 만료 시 TIMEOUT branch → `Apply_Branch`; 없으면 다음 stage/완료.
- `Apply_Branch`: nextActionId → `Enter_Stage` → STAGE_CHANGED; nextPatternId → `boss.PendingPatternFollowup{strPatternId, PinnedDefinitionRevision, iSourcePatternSequence, iRootPatternSequence, iDepth+1}` + `Finish_Pattern(COMPLETED)` → FOLLOWUP_QUEUED; depth가 32를 넘으면 ABORTED. 이 한계는 `ValtanBrain.cpp:23`의 file-static `MAX_PATTERN_FOLLOWUP_DEPTH`와 같은 값이며 쿠크 파일에도 같은 이름의 file-static 상수로 둔다(public header로 올리지 않는다).
- `Finish_Pattern`: gate/response/groggy 초기화, `Clear_PatternOutcomes`. `Abort_Pattern`: `PendingPatternFollowup` 비움.

**GameRoom**

- `Update_KoukuSaydonBoss`: Begin_Pattern 직후와 STAGE_CHANGED 직후 `Apply_BossPatternStageTransition(prev pattern/action, next pattern/action, pinned revision)`을 호출한다. 실패는 ABORTED + `Clear_KoukuSaydonPatternAudition`. FOLLOWUP_QUEUED/PATTERN_COMPLETED + pending follow-up이면 lifecycle PATTERN_COMPLETED 큐 → 다음 tick `Find_AuditionPattern` → `Begin_Pattern`(같은 pinned revision) → lifecycle ACTIVE(follow-up id).
- `Apply_BossPatternStageTransition`의 `CValtanBrain::Fail_Mechanic` 호출을 `"BOSS_VALTAN" == boss.strArchetypeId`로 guard.
- `Clear_KoukuSaydonPatternAudition`과 ABORTED 경로에서 `PendingPatternFollowup` 비움, 마지막 stage EXIT action 적용(gauge 0).

**contract test**(`ServerGameplayContractTests.cpp:885` 블록 옆): (a) SHIELD_STAGGER seg-2에서 후면 hit `staggerDamage` 누적 → STAGGER_BROKEN → 다음 Update가 FOLLOWUP_QUEUED, pending = SHIELD_BREAK; (b) 정면 hit → `bReflected`, boss HP/stagger 불변, 공격자 HP 감소; (c) 무타격 → seg-3 STAGE_CHANGED → PATTERN_COMPLETED; (d) SHIELD_BREAK 첫 stage GROGGY, `bPatternGroggy` true, wire `PATTERN_ACTIVE`; (e) COMMON_GROGGY 완료 후 pending 없음.
이 검사는 G04 데이터 전이므로 test fixture catalog 텍스트로 §4.3 행을 직접 만든다.

**검증**: Server Debug 빌드, `Server.exe --contract-test` failures 0, 기존 Pizza 검사 유지.

### G04 — Logic 투영, publisher 행, 방패 무력화 데이터, Client 재생

- projector: LOGIC box를 §2.2/§2.3/§4.3대로 encounter Product에 투영(`stageKind`, `actions`, `counterProxy`, `bossResponse`, `attackSourceGate`, `branches`). presentation v2(continuation/loop 허용). `role` 검사, `FOLLOWUP_PATTERN` 대상 검증(DAMAGE_THRESHOLD는 대상 첫 segment GROGGY).
- publisher Kouku block: stage property를 기본 18개 + 존재 시 `actions`, `counterProxy`, `bossResponse`, `attackSourceGate`, `branches`로 만든다(발탄 block 1995~2027행 방식). `PATTERNSTAGEACTION`, `PATTERNSTAGECOUNTERPROXY`, `PATTERNSTAGERESPONSE`, `PATTERNSTAGESOURCEGATE`, `PATTERNSTAGEBRANCH`/`PATTERNSTAGEFOLLOWUP` emit. `playAllPatternIds`는 `branches[].nextPatternId`에 등장하는 pattern을 FOLLOWUP으로 판정해 제외.
- `DamageProfiles.json`에 `damage.kakulsaydon.shield-reflect`(rate 첫 값 300%)를 추가하고 provenance receipt에 `PROJECT_TUNED` 항목을 동기화한다. `Publish-GameplayBalance.ps1`의 63 damage profile coverage 검사를 64로 맞춘다.
- composition: §4.2 세 pattern 저장(revision+1).
- Client presentation: `KOUKU_SAYDON_ACTION_PRESENTATION`에 `iSourceStartMs`, `bLoop`; admission은 `endPolicy ∈ {EXACT, HOLD_LAST_POSE, LOOP_TO_WINDOW}`, `sourceStartMs ≤ native clip`. `CNpc::Play_NetworkAction` overload에 `fStartSeconds` → `Set_AnimTrackPosition`(ticks = seconds × 해당 clip의 ticks per second, admission 시 model 헤더에서 읽어 저장). `ClientReplication.cpp:3411`이 `bLoop`, `iSourceStartMs`를 넘긴다.
- Boss Tool: `PRODUCT_PATTERN.FollowupPatternIds`(Product `branches[].nextPatternId` 수집); `Play_Selected`가 follow-up 집합을 audition service에 전달하고 service는 ACTIVE/PATTERN_COMPLETED lifecycle에서 집합의 id를 같은 요청의 연속 occurrence로 인정한다.

**검증**: projector publish/validate, unittest, `Publish-GameplayBalance.ps1` 뒤 bootstrap grep(§4.3 행), Debug Product 빌드, contract test, 사용자 실제 재생(§0.2 5번).

### G05 — Effect lane

- 문서: `EFFECT` box payload `effect{effectAssetId, anchorSlotId "root", followPolicy "follow"|"snapshot", localTransform{position, rotationDegrees, scale}}`, `durationMs 0`은 natural stop.
- Resources Effect 탭: `CEffectCatalog::Get_EffectAssetIds()` 중 `authoringPath`가 `Data/Effects/Authored/`인 ID를 `Effect/<첫 segment>` 폴더로 나열. Preview는 Effect Tool의 로컬 preview 요청과 같은 typed 요청을 MainApp에 넘긴다.
- projector: `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patterneffectcues.json`(schema `lostark.kouku-saydon-pattern-effect-cues` v1) — 파생 segment `actionId`별 `{cueId, effectAssetId, offsetMs(segment-relative), durationMs, anchorSlotId, followPolicy, localTransform}`.
- Client: `CKoukuSaydonPresentationAssetService::Try_Resolve_EffectCues(actionId)`; `ClientReplication` Kouku entity의 action edge에서 cue를 offset으로 예약하고 `CEffectPresentationService`의 `Queue_ProductCues`(CValtan이 stage cue를 큐잉하는 같은 진입점)로 CNpc root anchor에 spawn. 대상 ID는 `Level_KakulSaydonArena` Loader가 `Queue_ProductTargets_Priority`로 준비 큐에 등록한다. 준비 실패 cue만 거부하고 animation은 계속.
- Workbench: Effect lane, Details(asset ID 표시, anchor/follow/transform, duration), `FAMILY_ENABLED[EFFECT]=true`.

**검증**: projector/unittest, Client 최소 컴파일, `Validate-EffectSources.ps1`, 사용자 실제 재생.

### G06 — Scene Profile lane

- 문서: `SCENE_PROFILE` box `sceneProfile{profileId, blendMs}`. Resources: `CRenderingProfileService` profile 목록(6). projector → `KoukuSaydon.patternscenecues.json`.
- Client: `ClientReplication`이 typed `SCENE_PROFILE_REQUEST{profileId, blendMs, apply|restore}`를 MainApp 큐에 넣고 MainApp이 `Activate_Profile`. 복구는 cue 종료, pattern abort/completed, boss despawn, level 퇴장 전부에서 base profile로. `FAMILY_ENABLED[SCENE_PROFILE]=true`.
- MapLightGroup은 owner 자원이 아직 없어 이 G에 넣지 않는다.

### G07 — Camera lane

- 데이터: `Data/Encounters/KoukuSaydon/KoukuSaydonCinematicCamera.json`(schema `lostark.encounter-cinematic-camera` v6, cues는 Camera Tool에서 저작). Resources: 이 문서의 `cueId` 목록.
- 문서: `CAMERA` box `camera{cameraCueId}`. projector → cue에 `patternId/stageId`(파생 segment)와 offset을 기록한 Product 문서.
- Client: `Level_KakulSaydonArena`가 Kouku entity action edge에서 cue를 시작하고 `CValtanCinematicCameraController::Sample_Cue`로 매 frame 샘플(1526행 sequence shot 경로와 같은 pose 적용). abort/completed에 follow 복귀.

### G08 — Sound lane

- 데이터: `CharacterSoundCatalog.json` classes에 `KoukuSaydon` 추가(팀장 Resources `Sound/` 상대 경로). Resources: `CSoundCueCatalog::Collect_EventNames("KoukuSaydon")`.
- 문서: `SOUND` box `sound{eventName}`. projector → `KoukuSaydon.patternsoundcues.json`. Client: action edge에서 offset 예약 → `CSoundCueCatalog::Find_Variants` → `CGameInstance` 사운드 재생(Character SOUND cue와 같은 경로).

### G09 — Summon / Collider lane (Server)

- `ValtanBrain.cpp:2479 ApplyPatternHit`와 `IsShieldedByCover`를 `Server/Public/BossPatternHitRuntime.h`/`Server/Private/BossPatternHitRuntime.cpp`로 옮긴다(새 파일, `Server.vcxproj` `ClInclude`/`ClCompile`과 `.filters` 등록). 두 Brain이 같은 함수를 호출한다.
- 데이터: `Data/KoukuSaydon/Gate1/KoukuSaydonCombatObjects.json`(발탄 `lostark.valtan-combat-object-authoring`과 같은 필드, encounterId만 다름). publisher가 `COMBATOBJECT` 행을 쿠크 encounter로 emit.
- 문서: `SUMMON` box `summon{combatObjectArchetypeId, volley}`, `COLLIDER` box `collider{shape, radius, angle, length, halfWidth, damageProfileId, hitOffsetsMs, push, knockdown}`(COLLIDER는 LOGIC처럼 경계 생성). projector가 `SPAWN_COMBAT_OBJECT` action과 hit 필드를 투영. Kouku Brain validate 해제, Update가 공용 hit runtime 호출.
- 발산 WIPE에 `damage.valtan.omnidirectional-wipe-130` CIRCLE 100m hit을 이 G에서 붙인다.

### 문서 갱신

G04 완료 시 `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`에 composition v3 lane/box와 파생 stage 규칙, Logic kind 표, `PATTERNSTAGESOURCEGATE` 행을 적고 `CLAUDE.md` Action Workbench 단락의 "쿠크 Effect/Sound/Camera/Profile/Light/Summon 저작·family 실행은 별도 구현 범위" 문장을 실제 완료 family로 갱신한다. 각 G의 RESULT는 실행한 검증과 사용자 미확인을 분리한다.

## 6. 검증 요약

| G | 명령 / 절차 | 기대 |
|---|---|---|
| G01 | projector migrate/publish/validate, unittest, Client ClCompile 3파일 + link | Pizza 동등 Product, Reload/Save round-trip |
| G02 | Client ClCompile 2파일, 공용 트리 검사 2개 | Logic Add/Details/Save JSON |
| G03 | Server Debug 빌드, `Server.exe --contract-test` | 쿠크 검사 5건, 기존 회귀 0 |
| G04 | projector, `Publish-GameplayBalance.ps1`, bootstrap grep, Debug Product 빌드 | §4.3 행, 세 pattern Product, Boss Tool follow-up |
| G05~G08 | 각 family projector 문서, Client 최소 컴파일, `Validate-EffectSources.ps1`(G05) | family box 저장·투영·재생 |
| G09 | Server 빌드(새 파일 등록), contract test | 두 Brain 같은 hit runtime |
| 공통 | `git diff --check`, 변경 JSON parse | 경고 없음 |

에이전트는 빌드·contract test·publish까지 실행하고 Client 실행과 화면 판정은 사용자가 한다.

## 7. 구현 전 확인이 필요한 항목

1. §0.3 기본값(pattern 동일성, 반사 정책, stagger 게이지, 시간 초과 clip, Logic 카테고리 시점) 확정.
2. `staggerRequired 400`, hold 12000ms, reflect rate 300%의 첫 값.
3. G01의 migrate가 Pizza stage id를 `SEG_n`으로 바꾸므로 현재 bootstrap의 Pizza stage id가 함께 바뀐다. Boss Tool은 index로 표시하므로 영향이 없지만 다른 PC의 Server/Client는 같은 publish로 갱신해야 한다.
4. G05 Effect cue anchor를 CNpc root로 고정할지, bone anchor(`Build_SourceBoneAnchorWorld`)까지 첫 슬라이스에 넣을지.


### G02 추가 검토 — Logic 저작 상태 보존과 검증 일치

2026-09-05 Logic Resource Category 결과 검토에서 확인한 범위다. Server 실행, 분기 필드와 Product 배포는 추가하지 않는다.

- `KoukuSaydonActionWorkbench.cpp`: 오류로 격리된 Pattern의 `strPreservedJson` 안에 남은 Logic 참조도 삭제 검사에 포함한다. 참조 구조를 해석할 수 없으면 Logic 정의 삭제만 거부하고 다른 편집과 Save는 유지한다. Logic 박스만 선택했을 때 기존 Stage/Animation 전용 Duplicate는 비활성화한다.
- `project_kouku_saydon_composition.py`: Logic 박스 ID 중복, 다음 ordinal과 ID의 관계, Pattern 전체 시간과 600초 한계, Pattern당 1024개 제한을 C++ 저장 계약과 맞춘다.
- 기존 Python 테스트와 `BossCompositionDocumentContractTests.cpp`의 Kouku editor 경로에 회귀 사례를 추가한다. 실제 Data는 수정하지 않고 임시 복사본에서 생성·배치·창 수정·Save/Reload와 격리 Pattern 참조 보존을 확인한다.
- C++ 컴파일과 Client 링크, 기존 `--kouku-composition-editor-contract`만 검증한다. 변경 전에도 발생한 Python 4건과 미publish 상태는 별도로 기록한다. IntelliSense 오류와 실제 빌드 오류를 구분한다.
