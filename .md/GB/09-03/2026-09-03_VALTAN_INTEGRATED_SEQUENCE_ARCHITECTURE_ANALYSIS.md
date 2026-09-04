# 2026-09-03 발탄 통합 시퀀스 구조 분석 (설계 전 원리 실측)

브랜치 `GB/valtan-bugfix-koukusaydon-pattern`, HEAD `063e1a1a`, 미커밋 worktree 기준.
이 문서는 코드·데이터를 실측한 분석서다. 구현 계획이 아니며 아무 파일도 바꾸지 않았다.
다음 단계의 통합 구조 PLAN은 이 문서의 5장을 입력으로 작성한다.

읽은 정본: `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, `.md/TEAM/README.md`,
09-02 FINALE/COUNTER RESULT, 09-03 TOOLCHAIN RESULT, LARGE_DONUT RESULT, GRIP/EFFECT_BINDING PLAN,
INTEGRATED_SEQUENCER PLAN.

---

## 1. 데이터 계층: 발탄 한 패턴은 다섯 owner 파일과 세 개의 시계로 쪼개져 있다

| owner | 파일 | 소유 내용 | 시계 |
|---|---|---|---|
| Gameplay (Server 진실) | `Data/Valtan/Valtan.gameplay.json` | pattern 42개, stage(`stageKind`, `durationMs`, `hit`, `motion`, `events`, `branches`), `serverMotion`, `decisionModel.scriptedSequence` 52 step + `transitionPursuitMs` | Server fixed 30Hz stage clock |
| Presentation (Client 애니/V1 이펙트/카메라) | `Data/Valtan/Valtan.presentation.json` | stage별 `animation.occurrences[]`(clip, `sourceStartMs`, `playMs`, `endPolicy` EXACT/HOLD_LAST_POSE/LOOP_TO_STAGE_END), `effectCues[]`(V1, `clipOccurrenceId` 또는 stage clock, `anchorSlotId`, `stopPolicy`), `cameraInvocations[]` | clip occurrence clock |
| Effect V2 bindings | `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json` | 70행. `resource` LEAF/GROUP, `scope` pattern/stage/actionId, `clock` STAGE 또는 CLIP_OCCURRENCE + `repeatPolicy` ONCE/EACH_LOOP, `anchor`, `stopPolicy` | STAGE 또는 clip occurrence clock |
| Sound | `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json` 646행, `Valtan.combatobjectsoundcues.json` 9행 | pattern/stage/clipOccurrenceId + `startMs` + `repeatPolicy`. combat object는 `hitId`/`presentationEventId` 키 | clip occurrence clock, 별도 CAS owner |
| Combat object | `Data/Valtan/Valtan.combatobjects.json` 9종 + `Data/Actors/BossCatalog.json combatObjectVisuals` + presentation `independentEffects` | Server가 소유하는 도끼/도넛/돌기둥/포탈 charge. Effect는 `clientVisualId`로 따로 연결 | Server object lifetime |

- generated Product: `Project-ValtanPatternMaster.ps1 -Mode PublishV2`가 위 source를 `Data/Encounters/Valtan/ValtanEncounter.json`, `Valtan.patternbindings.json`, `Valtan.patterneffectcues.json`으로 투영한다. `Publish-GameplayBalance.ps1 -Mode Publish`가 Server bootstrap을 만든다. Server는 재시작해야 적용된다.
- source만 고치고 PublishV2를 안 돌리면 strict join이 실패해 Boss Tool·All Effects·Composition이 전부 read-only가 된다. 09-03 LARGE_DONUT RESULT §5.1이 실제로 겪은 잠금이다.
- 패턴 admission 등급: `CORE`(원본 mechanic, 도구 topology 편집 불가), `MANUAL_SERVER_AUDITION`(animation chain 승격, topology 편집 가능), `DERIVED_SERVER_PATTERN`(파생, weight 0). `promote_valtan_animation_chains.py:793`이 승격 stage를 전부 `hit NONE / events [] / branches []`로 만든다. 이것이 "어떤 패턴은 콜라이더가 있고 어떤 패턴은 없는" 첫 번째 원인이다.

---

## 2. Server 실행 원리

### 2.1 tick 흐름

```text
CValtanBrain::Update (ValtanBrain.cpp:2626)
  HP 0 / DEAD               -> 모든 pattern 상태 초기화, DEAD 전이 (:2629~2680)
  phase threshold           -> Set_GameplayPhase
  scriptedSequence cursor   -> iRotationStepIndex, pursuit tick 소모 (:1304)
  BeginPattern (:1780)      -> iPatternSequence++, target lock, leap landing 결정, EnterPatternStage(첫 stage)
  매 tick                   -> stage clock, ApplyPatternHit (:2465), TryConsumeDamageLessCounterProxy (:3013)
                               ApplyPublishedOutcomeBranch (:2135) -> ApplyStageBranch (:2068)
                               TIMEOUT 은 clock 이 끝난 뒤에만 평가
  FinishPattern (:1930)     -> terminal receipt, cursor++ , pursuit ticks = transitionPursuitMs
```

### 2.2 분기 primitive는 하나뿐이다

`branches[] = { outcome, nextActionId | nextPatternId }`.

| outcome | 소비자 |
|---|---|
| `TIMEOUT` | stage clock 종료 |
| `COUNTER_HIT` | `CBossCombatRuntime::Try_TriggerCounter` 성공 시 latch |
| `HEALTH_DAMAGE_THRESHOLD_REACHED` | `bossResponse ACCUMULATED_HEALTH_DAMAGE` |
| `PART_DESTROYED`, `ANY/ALL_PLAYERS_GRABBED`, `NAVIGATION_BLOCKED`, `WALL_CONTACT` | 각 runtime latch |

- `nextActionId`는 같은 pattern 안의 stage 점프다.
- `nextPatternId`는 **현재 pattern을 COMPLETED로 끝내고** `PendingPatternFollowup`에 넣어 다음 tick에 새 pattern occurrence(`iPatternSequence` 증가)를 시작한다(`:2081~2108`). 즉 cross-pattern 분기는 항상 "패턴 종료 + 새 패턴 시작"이다. Client는 이것을 pattern edge로 본다.
- 둘 다 있으면 INVALID_RUNNING_DEFINITION으로 abort한다(`:2073`).

### 2.3 카운터 판정 두 경로

1. damage hit 경로: `ApplyPatternHit`(`:2465`)에서 stage `hit.shape`가 플레이어에게 닿는 순간 `CPlayerSkillSystem::Try_Counter`(`:2513`). 플레이어가 `SKILL` 상태, `iComboStage == 1`, skill kind `COUNTER`, combo 2단이어야 한다.
2. damage-less proxy 경로: `TryConsumeDamageLessCounterProxy`(`:2164`). stage에 `COUNTER_HIT` branch가 있고 `hit.shape == NONE`이며 `counterProxy`가 있고 `COUNTERABLE` flag가 켜져 있을 때, 플레이어 카운터 스킬의 `maximumRange` 원과 보스 body 원(`collisionRadius 1.4`)이 겹치면 `Try_Counter`를 staged로 호출한다. `BOSS_FORWARD_ARC`는 `arc 180 / offset 0 / radius 0` exact 계약이다(`BossCombatRuntime.cpp:43~50`). 09-03 LARGE_DONUT RESULT §7의 "counterProxy는 죽은 데이터" 판정은 현재 코드에서는 더 이상 맞지 않는다. 실제 인게임 동작은 사용자 확인 대상이다.

### 2.4 사망과 부활은 "진짜 사망"이 아니라 패턴이다

- 진짜 사망: HP 0이면 `DEAD` action. Client `Begin_NetworkDeathPresentation`(`Valtan.cpp:4106`)이 actor `presentationClips.dead`를 재생하고 V1/V2 이펙트를 전부 정리한다. 부활 경로는 없다.
- 사용자가 보는 "죽고 살아남"은 scriptedSequence의 다음 세 step이다.

```text
[14] VALTAN_STRUGGLING            마지막 두 clip 19_05/19_06 = 원본 "하얗게 불사르고 망령화"
      wait-after 1000ms           -> Server CHASE (사용자가 본 "1초 추적")
[15] VALTAN_GHOST_DEATH_AUDITION   mesh_dead_1 3667ms, EXIT 에 SUPPRESS_INTER_STEP_PURSUIT
      wait 0                       (suppress 가 pursuit tick 을 0 으로 만든다)
[16] VALTAN_GHOST_RESPAWN_AUDITION mesh_respawn_1 3000ms, ENTER 에 SET_GAMEPLAY_PHASE 3
      -> Client Apply_BossCombatState 가 phase>=3 snapshot 에서 BOSS_VALTAN_GHOST part group 으로 교체 (Valtan.cpp:3873~3893)
      -> 완료 edge 에서 GameRoom::Activate_ValtanGhostPhaseLoop (:13450~13462, :12502)
[17] VALTAN_GHOST_FINALE ...      loop 활성 뒤에는 GameRoom.cpp:13204 가 sequence pointer 를 ghost loop 로 바꾸므로
                                   제품 경로에서 [17] 이후 row 는 더 이상 소비되지 않는다
```

- ghost loop = `VALTAN_GHOST_FINALE.finale.ghostPatternIds` 6개(`SIX_PIZZA_106, GROUND_ROAR, STAGGER_SLOT, BIND_SLOT, SILENCE_SLOT, TRIPLE_COUNTER`) ORDERED_ONCE_THEN_IDLE. step 사이에 `Begin_ValtanGhostRelocation`(1 tick 숨김 + 무적 + 결정적 재배치)과 5초 간격 `VALTAN_GHOST_PORTAL_ONCE` scheduler가 붙는다.
- 문자열 하드코딩 두 곳이 "사망+부활을 하나로 합치기"를 막는다.
  - `SUPPRESS_INTER_STEP_PURSUIT`는 `GameRoom.cpp:10303~10314`, `:10527~10537`에서 `VALTAN_GHOST_DEATH_AUDITION / valtan.sequence.dead.step-01`만 허용한다.
  - loop 활성화는 `previousPatternId == "VALTAN_GHOST_RESPAWN_AUDITION"`(`:13456`)이다.
- `SET_GAMEPLAY_PHASE`는 stage 단위 event라 합친 패턴의 두 번째 stage ENTER에 둘 수 있다(`valtan_tuning_pipeline.py:4683`).

### 2.5 이동 motion

- `LEAP_TO_TARGET`(HIGH_JUMP): `BeginPattern :1837`이 lock한 target 위치를 landing으로 쓴다. `landingPosition`(아레나 중앙 좌표)은 target이 없을 때의 fallback이다. 따라서 지금 추적 도끼 점프는 **중앙 점프가 아니라 target 위 점프**다.
- `LEAP_TO_ANCHOR + moveToAnchorBeforeTakeoff`(SIX_PIZZA): 첫 stage의 `takeoffStartMs`(800ms) 동안 현재 위치에서 anchor로 선형 이동(`ValtanBrain.cpp:160`), `travelStageId` STEP_03에서 낙하. 그런데 돌기둥 volley는 STEP_01 **ENTER**에 `RADIAL_AROUND_BOSS`로 생성되므로 보스가 아직 중앙에 도착하기 전 위치를 기준으로 4개가 박힌다. V1 텔레그래프 cue는 `arena.center.target-follow` anchor라 중앙에 그려진다. 사용자가 본 "중앙에서 시작 안 하고 발탄 위치에서 시작"은 이 두 기준이 다른 결과다. `RADIAL_AROUND_BOSS`에는 `arenaRandom.anchor`처럼 anchor policy가 없다.

---

## 3. Client 표현 원리: 세 lane, 세 stop 규칙, 두 개의 재생기

### 3.1 snapshot edge

`CValtan::Apply_NetworkState`(`Valtan.cpp:4176`)

```text
edge 판정  = patternId / actionId / iPatternSequence / iPatternStageIndex / iActionStartTick 중 하나라도 변함
edge 이면  1) 애니 seek (Apply_PatternPresentationSample)
           2) CEffectPresentationService::Stop_BossAction(이전 actionStartTick)   (:4300)
              -> stopPolicy != NATURAL 인 V1 만 제거 (Effect_PresentationService.cpp:256, :4865)
           3) attempted key set 초기화
           4) Spawn_DuePatternEffectCues / SoundCues / ShakeCues(actionAge)      (:4466)
           5) CEffectV2Runtime::Sync_Stage(actionId, age, clip clocks)          (:4478)
              -> stage 바뀌면 Prune_Spawned(stage lane) : bStopWithClip 인 것만 Finish, NATURAL 은 계속 (EffectV2_Runtime.cpp:877)
DEAD 이면  Stop_BossOwner (V1 전부) + Sync_Stage("") (V2 전부)
```

- 현재 Valtan V1 cue는 전부 `stopPolicy natural`, V2 binding도 전부 `NATURAL`이다. 그래서 카운터 성공으로 pattern이 바뀌어도 counter-window/채널 이펙트가 자연 종료까지 남는다. 이는 런타임 결함이 아니라 데이터 선택이다. V1은 `CUE_END/END`, V2는 `STAGE_END/CLIP_OCCURRENCE_END/EXPLICIT`가 이미 존재한다.

### 3.2 Sound lane은 Server 전용이고 stop handle이 없다

- `Spawn_DuePatternSoundCues`(`Valtan.cpp:2665`)는 `!m_isServerAuthoritative`면 즉시 return한다(`:2677`). Workbench의 로컬 preview(`m_bLocalPatternAuthoringPreview`)는 이 함수를 호출조차 하지 않는다(`:1290`은 Effect만 호출).
- `CGameInstance::Play_Sound`는 fire-and-forget이라 seek/stop이 없다. Workbench가 스스로 "INSPECTION ONLY: Sound has no seek/stop handle"이라고 표시한다(`ActionCompositionWorkbench.cpp:6395`, `:9472`).
- combat-object sound는 Server가 복제한 hit/presentation event에서만 울린다(`:2860~2925`).
- 따라서 "Play 눌러도 사운드가 안 들림"은 회귀가 아니라 설계된 공백이다.

### 3.3 로컬 preview 경로

`Apply_LocalPatternPresentationSample`(`:1220`)은 draft cue(`m_LocalPreviewEffectCuesByActionId`)와 V2 authoring snapshot(`Sync_StageAuthoring`)을 쓰고 combat object도 로컬 인스턴스로 흉내낸다. 분기는 Server가 아니라 `CValtanPatternTree::Build_PreviewStagePath`의 4가지 preview path(Normal / Counter→Groggy / Wall→Groggy / Part Break)로 미리 고른다. 즉 preview에서 보는 분기는 "선택한 경로의 stage 나열"이지 Server outcome이 아니다.

### 3.4 ghost 표현 교체

`Apply_BossCombatState`(`:3855`)가 boss combat snapshot의 `iGameplayPhase >= 3`을 보고 `Replace_PresentationPartGroup("BOSS_VALTAN_GHOST")`를 호출한다. NetEntityId/HP는 유지되고 part group만 바뀐다. `GHOST_HIDDEN` flag는 relocation 1 tick 동안 그리기만 막는다.

---

## 4. 도구 계층: 왜 Stage +가 counter 분기를 주지 않는가

### 4.1 lane `+`가 실제로 하는 일 (`Request_LaneAuthoring`)

| lane | 동작 | 제약 |
|---|---|---|
| STAGE | topology Details 열기, "ACTIVE/WINDUP/GROGGY/WAIT를 선택 Stage 뒤에 삽입" | `MANUAL_SERVER_AUDITION`이고 `IsValtanManualStageTopologyLinear`(`BalanceTool.cpp:972`)를 통과해야 한다. 새 stage는 `hit NONE / events [] / branches []`(TIMEOUT만) |
| LOGIC | "Counter Box Detail" = `Set_ValtanStageCounterWindow`(`:4740~`) | 성공/타임아웃 target이 **같은 pattern의 typed stage**여야 한다. `nextPatternId`(cross-pattern) 쓰기 경로 없음. 일반 branch 편집기는 COUNTER_HIT를 거부(`:4614`) |
| COLLIDER | BOX 8.0 × half 2.5, pulse 1 @0ms, DAMAGE 기본 생성 | non-WAIT manual stage, CORE stage는 Tune만 |
| EFFECT | V1: 선택 clip occurrence에 cue append. V2: `Stage_AppendBossValtanStageBinding` | V2 append는 `STAGE clock / ONCE / b_effectroot / SNAPSHOT_AT_START / TARGET_YAW / NATURAL` 고정(`EffectV2_Catalog.cpp:1171`), 사후 편집은 startMs만 |
| SOUND | typed row 추가/편집 | 재생 없음 |

- 현재 `VALTAN_TRIPLE_COUNTER`의 `COUNTER_HIT -> nextPatternId VALTAN_GROGGY_FOLLOWUP`는 Python 저작 스크립트(`author_valtan_phase_two_mechanics.py`)와 손 편집으로만 만들어졌다. 도구는 읽을 수는 있지만(`ReadValtanCounterWindow`가 cross-pattern을 인식) 쓸 수 없다.
- Save는 Pattern+Sound+EffectV2를 한 트랜잭션으로 커밋한 뒤 PublishV2, gameplay publish까지 가야 Server가 새 revision을 본다. Server 재시작 전에는 적용이 아니다.

### 4.2 콜라이더가 있는 패턴과 없는 패턴

| 구분 | 근거 |
|---|---|
| hit가 있는 stage 41개 중 CORE 패턴이 대부분 | `author_valtan_phase_two_mechanics.py` 등 저작 스크립트가 직접 `hit.shape`를 썼다 |
| MANUAL_SERVER_AUDITION 패턴의 hit | 승격 시 전부 NONE(`promote_valtan_animation_chains.py:793`). 이후 Workbench Collider `+`로 사람이 하나씩 추가한 것만 있다 |
| 원본 skill 크기 자동 생성은 발탄에 없다 | `Data/Animation/Reference/Valtan/Valtan.skilltiming`은 shape 정보를 갖고 있지만(예: 420610 shapes=26) 소비자는 `Publish-GameplayBalance.ps1:1594`의 sourceActionId 존재 검사뿐이다. `Valtan.animevents` HIT 397행은 전부 `area=0`이다 |
| 플레이어는 자동 생성이 있다 | `build_hitshapes.py`가 `.animevents` HIT(area>0)와 skillbindings에서 `Data/Animation/HitShapes/<Asset>.hitshapes.json`을 만들고, HIT가 없는 스킬은 `fill_animevents_hit_shapes.py`가 `PlayerSkills.json hitTimeMs`에 skilltiming shape 1행을 합성한다 |

사용자가 기억하는 "스킬 사이즈 기준 자동 콜라이더"는 플레이어 6 class에만 있고 발탄에는 처음부터 없었다.

### 4.3 판정과 데미지 원리 (Server fixed tick, Engine/PhysX 비의존 XZ primitive)

```text
플레이어 -> 보스   PlayerSkillSystem.cpp:139 Hit_ShapeOverlaps
                  areaType 1 원/링, 2 전방 박스, 3 부채꼴  vs  보스 body 원(collisionRadius 1.4, BossProfiles.json)
                  스킬 damage rate 를 sub-hit 수로 분할 (DamageOfSubHit)
                  shape 없는 스킬은 maximumRange 원 단일 판정
보스 -> 플레이어   ValtanBrain.cpp:2465 ApplyPatternHit
                  stage hit.shape CIRCLE/RING/CONE/BOX/CROSS/SIX_DIRECTIONS vs 플레이어 body 원(PLAYER_HALF_EXTENT_X)
                  schedule = pulse(EXPLICIT_OFFSETS/INTERVAL) 또는 ACTIVE_WINDOW(대상당 1회)
                  rawDamage = bossProfile.attackPower x DamageProfiles damageRatePercent
                  cover(돌기둥) 차폐, push/knockdown/down, CAPTURE 응답(왼손 잡기)
combat object      자체 hits[] 와 lifetime, Client 는 Effect 만 그린다
Client collider    CBounding_Sphere 는 Server 가 복제한 radius 의 Debug wire 뿐, damage 권위 없음
```

---

## 5. 왜 회귀와 누락이 반복되는가 (원리 수준)

1. **한 의미 단위가 다섯 파일 + generated Product + C++/Python 하드코딩 oracle에 흩어져 있다.** 도끼 volley 값 축소, 카운터 V1 effect 파일 삭제, 사자후 loop 제거가 한 커밋에서 "일부만" 바뀌면 strict join이 전체 tree를 잠그거나 조용히 사라진다. 메모리 `valtan-hardcoded-oracles`가 그 목록이다.
2. **이펙트 재생기가 둘이고 시계·anchor·stop 규칙이 다르다.** V1은 presentation.json의 clip occurrence cue, V2는 별도 bindings 파일의 STAGE/CLIP clock. 같은 "카운터 창 이펙트"가 V2 pulse-group 3행 + V1 cue 0행처럼 한쪽에만 있다.
3. **분기 모델이 패턴 종료형이다.** cross-pattern 분기는 항상 새 occurrence다. Client는 pattern edge에서 attempted key를 비우고 NATURAL 이펙트는 남긴다. Sound는 stop 자체가 없다. 그래서 "그로기로 넘어갔는데 이펙트/소리가 계속" 문제가 구조적으로 생긴다.
4. **Server가 문자열로 특수 처리하는 지점이 있다.** `SUPPRESS_INTER_STEP_PURSUIT`와 ghost loop 활성화는 특정 patternId/actionId에 박혀 있다. 패턴을 합치거나 이름을 바꾸면 validator 오류 없이 동작만 사라진다.
5. **도구 저작 표면이 데이터 모델보다 좁다.** cross-pattern counter, V2 anchor/repeat/stop 지정, group 자식 편집, 비선형 topology 삽입이 도구에 없어 사람이 JSON/스크립트를 고치고, 그 결과 Product drift와 oracle 불일치가 생긴다.
6. **Product drift lock.** source만 고친 상태에서 Tool을 열면 admission 실패로 전부 잠기고 "Tool이 고장났다"로 보인다.

---

## 6. 요구사항별 현재 상태와 필요한 변경 지점

| # | 요구 | 현재 상태 | 변경이 닿는 지점 |
|---|---|---|---|
| 1 | Play 시 사운드 | Server 전용 gate + stop handle 없음 | `Valtan.cpp:2677` gate에 local preview clock 허용, Engine `Play_Sound`가 channel handle을 돌려주고 `Stop_Sound(handle)` 추가, edge/seek 시 정리 |
| 2 | Stage +에서 counter 분기 | 도구가 same-pattern typed window만 씀 | `Set_ValtanStageCounterWindow`에 `nextPatternId` 성공 target 허용, `IsValtanManualStageTopologyLinear`가 counter edge를 가진 stage 삽입 허용 |
| 3 | UI 타이밍 | HUD ViewModel이 boss `patternId/actionId/iActionStartTick/stageIndex/flags`를 이미 가짐(`CombatHUDViewModel.h:89~95`) | 4번째 presentation lane(UI cue)을 sound cue와 같은 키(pattern/stage/clip/startMs)로 추가하고 같은 `Try_ResolveActionAgeSeconds` 시계를 쓴다 |
| 4 | 추적 도끼 중앙 점프 + V1 클립 연결 | motion은 `LEAP_TO_TARGET`(target 위 착지). V1 cue는 TAKEOFF/LAND에 이미 연결, catalog 존재. AIRBORNE에는 cue 없음 | serverMotion을 `LEAP_TO_ANCHOR + moveToAnchorBeforeTakeoff`로, 도끼 `arenaRandom.anchor`는 이미 `BOSS_SPAWN_POSITION`. AIRBORNE 8초에 붙일 V1 asset 선택은 사용자 결정 |
| 5 | 피자 패턴 중앙 시작 | 돌기둥 volley가 STEP_01 ENTER에 보스 현재 위치 기준 RADIAL | `RADIAL_AROUND_BOSS`에 `anchor: MOTION_LANDING_ANCHOR` 옵션(publisher+Server `GameRoom.cpp:9875~`) 또는 volley를 도착 뒤 stage로 이동 |
| 6 | 카운터 성공 시 이펙트 종료 | 전부 NATURAL이라 남음 | 해당 V1 cue `stopPolicy CUE_END`, V2 binding `STAGE_END`. V2는 append 후 stopPolicy 편집 API 필요(G13-B) |
| 7 | 3연속 실패 타격 2hand V2 + 사운드 | `FAIL_1/2/3`에 `boss.valtan.twohand` STAGE 900ms 3행과 sound `Attack13_Shot1 900 / ShotVox1 1000` 이미 존재 | 데이터 확인만. 실제 화면은 사용자 판정 |
| 8 | 사망+부활 한 클립, 유령 부활, 마지막 패턴 지속 | 세 step + 1000ms pursuit + 문자열 하드코딩 두 곳 | (a) STRUGGLING 뒤 wait 100 또는 suppress 일반화, (b) DEATH/RESPAWN을 한 pattern 두 stage로 합치고 `SET_GAMEPLAY_PHASE`를 2번째 stage ENTER로, (c) `GameRoom.cpp:10303/10527/13456`의 문자열을 typed finale/event로 교체, (d) "마지막 패턴 지속"은 ghost loop 6개 중 무엇을 남길지 `finale.ghostPatternIds` 결정 |
| 9 | 콜라이더 자동 생성 | 발탄에는 없음. skilltiming shape는 존재 | `build_valtan_hitshapes.py`(신규)가 `Valtan.skilltiming` shape → stage `hit` 초안을 만들고 Workbench Collider lane이 그 초안을 Tune하는 형태. Server 코드 변경 없음, publisher 검증만 |

---

## 7. 통합 구조 설계 방향 (PLAN 입력)

```text
원칙 1  한 pattern 의 모든 lane(Animation/Collider/Effect V1/V2/Sound/Camera/UI)은 같은 stage clock 위의 "cue" 다.
        stage 시작 tick 과 actionAge 하나만 공유하고 owner 파일은 유지한다. 두 번째 파일 형식을 만들지 않는다.
원칙 2  분기는 Server 의 branch primitive 그대로 두되, Client 는 pattern edge 마다
        "이전 occurrence 의 cue 를 stopPolicy 대로 정리" 하는 한 함수로 V1/V2/Sound/UI 를 같이 처리한다.
        -> Sound 에 handle 을 주는 것이 선행 조건.
원칙 3  Server 특수 문자열(suppress pursuit, ghost loop 활성화)을 stage event / finale 종류로 typed 화한다.
원칙 4  도구는 데이터 모델과 같은 폭을 갖는다: cross-pattern counter writer, V2 anchor/stop/repeat 편집,
        비선형 topology 삽입. 도구가 못 쓰는 필드를 남기면 다시 손 편집과 drift 가 생긴다.
원칙 5  하드코딩 oracle(Python 상수, C++ owner allowlist)은 데이터에서 유도하거나 validator 로 옮긴다.
```

이 문서는 여기까지가 범위다. 구현 G 분할, 전체 코드, 검증 명령은 별도 PLAN에서 작성한다.
