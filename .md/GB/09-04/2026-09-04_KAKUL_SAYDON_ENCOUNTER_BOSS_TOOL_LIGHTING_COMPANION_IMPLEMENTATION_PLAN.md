# 2026-09-04 쿠크세이튼 40일차 — 패턴 검증 모델·Boss Tool·Logic Graph·조명 연출·건슬 AI 구현 계획서

브랜치 `GB/KoukuSaydon-pattern1`, HEAD `9dd7b10c`(= `origin/main` merge PR #310), worktree clean.
읽은 정본: `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, `.md/GB/gotchas.local.md`, `.md/TEAM/README.md`,
`.md/TEAM/보스툴.md`, `.md/GB/계획서하네스규칙.local.md`, `.md/GB/local.md`,
`09-03 KAKUL_SAYDON_PATTERN_INTAKE_AND_COMPOSITION_SAVE_AUDIT`, `09-03 KAKUL_SAYDON_MODEL_AND_ANIMATION_INVENTORY_ANALYSIS`,
`09-03 INTEGRATED_SEQUENCER_IMPLEMENTATION_PLAN/RESULT`, `09-03 TOOLCHAIN_UNIFICATION_AND_BOTTLENECK_REMOVAL_RESULT`,
`09-04 KOUKU_ROULETTE_AND_BOOK_CUTSCENE_RESULT`.

이 문서는 **구현 계획서**다. 목표, 현재 실측, 변경 파일, 데이터·호출 흐름, G별 범위, 검증을 합의하는 문서이며
기존 H/CPP 전문은 싣지 않는다. 각 G의 코드는 선언 수준의 러프 계약이고, 실제 반영 전에 G별 디테일 계획서
또는 대화 delta 설명으로 확장한다. 사용자가 언급한 `쿠크세이튼공략.txt`와 `koukusaytonComposition.json`은 현재
저장소와 바탕화면에서 발견되지 않았다. 원작 기믹은 사용자 서술과 `Data/Animation/Reference/KakulSaydon/*.actionreference.json`의
`displayName`을 정본으로 삼았고, Composition은 실제 파일인 `Data/Compositions/Bosses/KakulSaydon.bosscomposition.json`을 기준으로 한다.

---

## 0. 요청 정리와 결론

| 사용자 요청 | 결론 | 소유 G |
|---|---|---|
| Debug에서 진입 즉시 전체 패턴이 재생되는 불편 제거. Debug 기본은 queue(대기), Release는 저장된 full pattern 자동 재생 | Server room에 `SERVER_BOSS_PLAYBACK_POLICY`를 둔다. Debug 빌드 기본 `HOLD_UNTIL_COMMAND`, Release `PRODUCT_AUTOMATIC`. 기존 `bAutomaticPatternSequenceAuditionHold`를 spawn 시점에 켜는 것이 전부이며 두 번째 결정 경로를 만들지 않는다 | G00 |
| 처음 카메라 시퀀스가 끝나는 큐와 타이밍에 맞춰 패턴 시작 | 입장 cinematic을 Server 패턴(첫 slot)으로 두고 그 stage ENTER에서 `PLAY_WORLD_SEQUENCE` stage action이 world sequence를 방송한다. Server stage clock 하나가 카메라·소품·보스 시작을 모두 소유한다 | G00 |
| Boss Tool에서 Valtan/KoukuSaydon 선택, composition 기준 pattern 나열, All/Current 두 탭, `[Live]` 행 표시와 text, Play Selected(무한 반복), Play Full Pattern(저장 순서 0부터 끝), Next Pattern을 보스별로 | `BOSS_TOOL_TARGET` 선택자, 행 `[Live]` 마커, Action bar 재편. All/Current 탭과 Repeat·Restart Saved Flow는 이미 있으므로 이름과 위치만 바꾼다 | G01 |
| 쿠크세이튼 보스 자체를 더 효율적으로 패턴 수행 | Valtan과 같은 split source(`Data/Kakul/*`) → projector → Server catalog/brain → Client presentation 계약을 encounter 단위로 일반화하고 첫 패턴 하나를 관통시킨다 | G02 |
| 1관문 기믹(카드 분배, 광기 게이지·삐에로, 130 무력화·반사, 110 하트핑·스포트라이트, 85 댄스타임, 50 룰렛, 카드 맞추기) | 전부 Server typed stage action / outcome / judgement 확장이며 Client는 표현과 입력 relay만 한다 | G03 |
| Logic flow를 상용 엔진 blueprint처럼 노드 연결·분기·counter 튜닝 | 기존 read-only `CBossLogicFlowRenderer`와 Workbench `Composition Boss Pattern` 캔버스를 하나의 편집 가능한 `CBossLogicGraphEditor`로 승격한다. writer는 여전히 `CBalanceTool` joined draft 하나다 | G04 |
| 맵별 조명 배치, 기믹 시작 시 암전 연출, 렌더 퀄리티 튜닝 | scene profile(전역 분위기) / map light(배치 조명) / pattern light cue(기믹 연출) 세 층으로 나눈다. Engine에 spot light를 추가하고 profile blend와 light group을 데이터로 둔다 | G05 |
| 건슬 AI: `도움!` 입력 시 20~30초 전투 참여, 이후 따라다니며 기믹 설명 말풍선, ChatGPT 연동 또는 저장된 JSON | Server가 소유하는 pseudo-player companion(GUNSLINGER class)으로 만든다. 대사는 scripted JSON이 기본이고 LLM은 Server 밖 sidecar가 선택적으로 담당한다. 말풍선은 기존 `S2C_CHAT` 경로를 그대로 쓴다 | G06 |
| 2·3관문, 마지막 빙고 UI 연출 | action 매핑만 고정하고 별도 계획서로 넘긴다 | G07 |

---

## 1. 현재 실측

### 1.1 쿠크세이튼 자산·데이터·Server

| 축 | 현재 | 근거 |
|---|---|---|
| Level / World | `LEVEL::KAKULSAYDON_ARENA`, `WORLD_ID::KAKULSAYDON_ARENA`, Debug Character Select의 `C2S_DEBUG_ENTER_KAKULSAYDON_ARENA` typed transfer로만 진입 | `Client/Private/LevelRegistry.cpp:156`, `Server/Private/GameRoom.cpp:3098` |
| Level scene profile | `scene.development.neutral.v1` (Valtan은 `scene.valtan.cool-low-key.v1`) | `LevelRegistry.cpp:158` |
| Map | `LV_LUT_MIDNIGHTC_ED` placement 2971, asset 318, 룰렛판(배치 40, 기본 숨김), 팝업북 컷신(배치 41~299, 4.5초), camera shot 2개, world sequence instance 10개 | `Data/Maps/MapCatalog.json`, `LV_LUT_MIDNIGHTC_ED.camerashots.json`, `.worldsequences.json` |
| Map light | 없음. Valtan만 `maplights` pair(PointLight 22개) | `MapCatalog.json` Valtan 항목 |
| Gameplay world | formatVersion 6, playerSpawn 9, triggerBox 6(`jump.1~3`, `paper.1~2`, `1Stage_Final`=circus finale playSequence), **boss placement 0** | `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` |
| Stage marker | SL01~SL05, `semanticStatus=SOURCE_LEVEL_ID_ONLY` | `StageMarkers.json` |
| Animation reference | 4 profile / 349 action / 4,072 stage / 3,692 slot, `REFERENCE_ONLY` | `Data/Animation/Reference/KakulSaydon/*.actionreference.json` |
| Animation 저작 | `actionbindings` 0행, `patternbindings` 0 pattern | `Data/Animation/Authored/KakulSaydon/*` |
| Boss Composition | `status REFERENCE_ONLY`, `bossArchetypeId null`, `patterns []`, source 12개 | `Data/Compositions/Bosses/KakulSaydon.bosscomposition.json` |
| Arena Sequencer | `SHADOW`, track 2개(circus finale world sequence + `shot.1`), 21,010 ms | `Data/Compositions/Sequences/KakulSaydonArena.sequencer.json` |
| BossCatalog / BossProfiles | v6 `BOSS_VALTAN`, `BOSS_VALTAN_GHOST`만. v4 Valtan 2건만 | `Data/Actors/BossCatalog.json`, `Data/Balance/BossProfiles.json` |
| Server brain / Encounter / combat object | 전부 없음. `Data/Kakul`, `Data/Encounters/KakulSaydon` 폴더 없음 | `ls Data/`, `ls Data/Encounters/` |
| 모델 | `MN_RPCZ_00`(1관문 쿠크, 무기 소켓 3개·무기 모델 없음), `MN_RPCT_00/05`(2관문 세이튼, 같은 skeleton·249 clip), `MN_RPCT_06`+`WP_MN_RPCT_06`(대형 세이튼, 무기가 본체와 같은 이름 clip을 재생), `MN_RPCT_07`=05 alias(3관문) | 09-03 인벤토리 분석 §1 |

즉 **보스가 아레나에 서서 패턴을 재생하는 데 필요한 것 중 애니메이션과 맵만 있다.** 이 사실이 G02를 모든 기믹 G보다 앞에 두는 이유다.

### 1.2 Valtan 패턴 재생 제어의 현재 구조

Server `CValtanBrain::SelectPattern`(`ValtanBrain.cpp:1198~`)의 우선순위는 다음과 같다.

```text
1. PendingPatternFollowup (cross-pattern 후속, FORCED_AUDITION)
2. scriptedSequence (Product 자동 프로그램, ORDERED_ONCE_THEN_IDLE)
   조건: !bScriptedPatternPlayback && !bAutomaticPatternSequenceAuditionOverride && PendingPatternIds.empty()
3. intro pattern (scriptedSequence가 없거나 override일 때만)
4. PendingPatternIds (Debug 강제 큐 / health-bar mechanic 큐)
5. bAutomaticPatternSequenceAuditionHold && scriptedSequence -> 아무것도 고르지 않고 대기
6. weighted rotation
```

`Data/Valtan/Valtan.gameplay.json`의 `decisionModel.scriptedSequence`는 51 slot을 가지며 첫 engage 직후 자동으로 시작한다.
이것이 사용자가 말한 "진입하자마자 전체 패턴이 재생된다"의 정확한 원인이다. `bAutomaticPatternSequenceAuditionHold`는
현재 `ARM_HEALTH_BAR` 뒤에만 켜지고, 켜져 있으면 5번에서 idle로 대기한다. 따라서 **Debug 기본 대기는 이 flag를
boss spawn 시점에 켜는 것으로 충분하다.** Release Server는 모든 audition op를 `REJECTED_RELEASE_BUILD`로 거부한다
(`Level_ValtanArena.cpp:795`).

Client 소유자는 `CValtanPatternAuditionService`(PLAY_PATTERN_ID / RESTART / Next 한 큐), `CValtanPatternFlowService`
(Debug ordered Flow), `CBossTool`(관측·명령). wire는 `C2S_VALTAN_AUDITION_REQUEST`가 이미 `strBossPlacementId`,
`strPatternId` stable ID를 나른다(`PacketMessages.h:1103`). `NETWORK_PROTOCOL_VERSION = 55`,
`GAMEPLAY_BOOTSTRAP_FORMAT_VERSION = 32`(`CLAUDE.md`의 v51/v26 서술은 낡았다. §1.7).

### 1.3 Boss Tool 현재 화면

`CBossTool::Render()`(`BossTool.cpp:1710`)는 창 제목 `Valtan Boss Tool`, 탭 `Boss Verification` / `Pattern Flow` / `Logic Flow`.
Boss Verification 왼쪽은 이미 `All Patterns`(`Render_PatternList`, CORE/ANIMATOR/DERIVED 구분) / `Current Patterns`
(`Render_CurrentPatternList`, 저장된 scriptedSequence 순서) 두 탭이고 오른쪽은 `Render_SelectedPattern`.
Action bar(`BossTool.cpp:3803`)는 `Play Selected Pattern (Keep Arena)`, `Restart Active Pattern (Keep Arena)`,
`Retry Restart Verdict`, `Repeat`(`m_bRepeat`), `Revive Player`. `Restart Saved Flow (Fresh Arena)`(=저장 순서 01부터 전체 재생)는
Pattern Flow 탭 안에 있다. Live는 `Render_LiveSummary`(`m_strLivePatternId`, `m_bFollowLive`)가 상단에 요약하지만 **목록 행에는
`[Live]` 마커가 없다.** `Next Pattern...`은 `m_NextPatternIds` 공용 inventory에서 고른다.

### 1.4 Logic / Blueprint 캔버스 현재 상태

| 파일 | 역할 | 편집 |
|---|---|---|
| `Client/Public/ActionCompositionGraphModel.h` | 한 Pattern의 stage 노드·outcome edge·경로·layout·hit-test 순수 투영 | 없음(투영) |
| `Client/Public/BossLogicFlowView.h` | `CBossLogicFlowRenderer` read-only 캔버스(pan/zoom, opt-in 선택), `CBossLogicFlowObservedEdgeResolver` live edge 관측 | 없음 |
| `Client/Private/ActionCompositionWorkbench_Blueprint.cpp` | `Render_BossPatternWindow`: outcome override preview, `COMPOSITION_NN` stage 추가 | stage 추가만 |
| `CBossTool::Render_FlowGraphEditor` | Pattern Flow slot 노드/edge 편집, `Save Flow` | pattern 순서만 |

즉 노드 캔버스 3개가 서로 다른 수준(패턴 순서 / 스테이지 분기)을 각각 그리고, **stage 분기·counter·event를 노드에서 직접 편집하는
경로는 없다.** 편집은 Composition Workbench의 표/Detail 패널에서 `CBalanceTool` joined draft를 거친다.

### 1.5 조명·렌더링 현재 상태

- Engine `LIGHT_DESC`(`Engine_Struct.h:21`)는 `eType, vDirection, vPosition, fRange, fFalloffExponent, vDiffuse/vAmbient/vSpecular`. `sizeof == 92` static_assert.
  `Shader_Deferred.hlsl`은 `PS_MAIN_DIRECTIONAL`, `PS_MAIN_POINT`만 있고 **spot light가 없다.**
- `Data/Rendering/Authored/RenderingProfiles.json` v1 revision 10: `globalQuality`(SSAO/Bloom/exposure/gamma/FXAA)와 scene profile
  (directional light, shadow, height fog, exposure/bloom multiplier). `CRenderingProfileService::Activate_Profile`은 즉시 전환이며 blend가 없다.
- Map point light: `<Area>.maplights.json` v1(`lightId, sourceLevel, sourceObjectId, position, radiusMeters, falloffExponent, color, brightness`),
  `CMapLightDocument::MAX_LIGHT_COUNT = 64`, `CMapLightPresentationRuntime`이 매 frame `CPresentation_Manager::Add_TransientLight`로 제출.
  Valtan Level만 소유하고 Kakul Level은 소유하지 않는다.
- Screen post 4종(`RGB_NOISE / ZOOM_BLUR / FILM_NOISE / CHROMATIC_ABERRATION`), screen overlay, Effect V1 point light emission(`Effect_LightPresentation.h`).
- F1 `RENDERING` Workbench는 profile 편집·Save/Publish·Benchmark. 렌더링 품질 로드맵은 09-03 TOOLCHAIN PLAN §12.2(PBR/IBL, CSM, TAA, SSR/GI, ACES).

### 1.6 채팅·NPC·Esther (건슬 AI 재료)

- 채팅: `C2S_CHAT` → `CGameRoom::Handle_Chat`(`GameRoom.cpp:4680`)이 방 전원에 `S2C_CHAT{iFromNetEntityId, strFromNickname, strText}` relay.
  Client `CChatWindowView`(제품 UI, ImGui 아님), `CWorldPlayerChatBubbleView`(`Try_Get_ActiveChatBubble(netEntityId)`로 nameplate 위 말풍선).
- Esther: `CEstherSkillSystem` + `Spawn_EstherSummon`(`GameRoom.cpp:3414`)이 NPC archetype을 world entity로 소환하고 clip 종료에 despawn.
  Server가 소유한 아군 entity의 살아 있는 예다.
- `CNpcBehaviorRuntime`(route/idle), `CMonsterBrain`(추적/공격), `CPlayerSkillSystem`(플레이어 스킬 판정)이 있고
  GUNSLINGER는 정식 playable class(`PlayerSkills.json` 38000 계열, `inputSlot` 보유).
- HTTP client는 Engine/Client/Server 어디에도 없다(`winhttp/libcurl` grep 0건). WinSock 래퍼 `WinSockContext.h`만 있다.

### 1.7 문서 drift

`CLAUDE.md`는 protocol v51 / gameplay bootstrap v26을 서술하지만 코드는 `NETWORK_PROTOCOL_VERSION = 55`, `GAMEPLAY_BOOTSTRAP_FORMAT_VERSION = 32`다.
이 계획의 첫 wire/bootstrap 변경 G에서 `CLAUDE.md`와 `AGENTS.md`의 숫자를 코드 값으로 교정한다.

---

## 2. 개념과 방향성

### 2.1 패턴 검증 모델: "보스는 명령을 기다린다"

```text
Release Server
  spawn -> 첫 engage -> scriptedSequence slot 01(입장 cinematic 패턴) -> slot 02부터 마지막 slot까지 순서대로 -> 종료 idle
  Debug op는 REJECTED_RELEASE_BUILD

Debug Server (기본 HOLD_UNTIL_COMMAND)
  spawn -> bAutomaticPatternSequenceAuditionHold = true -> IDLE 대기
  Boss Tool Play Selected     -> PLAY_PATTERN_ID 한 패턴, Repeat면 완료 tick마다 재제출
  Boss Tool Play Full Pattern -> FLOW_START(저장 slot 01부터), Stop After Current로 중단
  Boss Tool Auto Playback ON  -> SET_AUTOMATIC_PLAYBACK(PRODUCT_AUTOMATIC) -> Release와 같은 자동 진행을 Debug에서 검증
```

카메라 시퀀스 동기는 Client가 시퀀스 종료를 관측해 Server에 알리는 방식이 아니라, **Server 패턴 stage의 duration이 시퀀스 길이와 같고
그 stage ENTER가 시퀀스 시작을 방송**하는 방식이다. Valtan은 이미 `VALTAN_ENTRANCE_CINEMATIC`(8600+5800+5467 ms)이 카메라 cue와 같은 시계를
쓴다. 쿠크는 `KAKUL_G1_ENTRANCE`(21,010 ms, `KakulSaydonArena.sequencer.json`의 durationMs와 동일) stage ENTER에서
`PLAY_WORLD_SEQUENCE world.sequence.instance.circusfinale`를 방송하고, `shot.1`이 그 sequence에 묶여 있으므로 카메라도 같은 길이로 잡힌다.

### 2.2 보스 하나의 정본

```text
Data/<Boss>/<Boss>.gameplay.json        Server stage·branch·motion·collider·event·reaction·decisionModel
Data/<Boss>/<Boss>.presentation.json    ordered animation occurrence·Effect/Camera/Scene/Light invocation
Data/<Boss>/<Boss>.combatobjects.json   FIXED_AREA / MISSILE / HOMING 정의
Data/Actors/BossCatalog.json            archetype -> 모델·무기·part·combat-object visual
Data/Balance/BossProfiles.json          HP·health bar·이동·충돌·phase
Data/Worlds/<Area>/Gameplay.world.json  boss placement(disabled) + activateEncounter triggerBox
Data/Compositions/Bosses/<Boss>.bosscomposition.json   위 owner의 stable ID join manifest (본문 복제 금지)
        │ projector (Validate -> stage -> commit, CAS)
        ▼
Data/Encounters/<Boss>/*.json           generated Product (직접 편집 금지)
        │ Publish-GameplayBalance / Publish-WorldGameplay
        ▼
Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap, World/<WORLD>.worldbootstrap
```

쿠크는 이 구조를 **그대로** 쓴다. `gotchas.md`의 "gameplay source가 없는 encounter에 공용 Dataset/runtime부터 만들지 않는다"에 따라
공용 registry(`BossSourceRegistry`)는 G02에서 Kakul source/Product를 만들면서 Valtan과 Kakul 두 소비자를 **같은 변경 단위**로 옮길 때만 도입한다.
관문은 encounter 3개(`ENCOUNTER_KAKUL_GATE1/2/3`)로 나누고 world는 하나(`KAKULSAYDON_ARENA`)를 유지한다. 관문 이동은 기존
`activateEncounter` triggerBox와 stage marker 위치로 표현하며 새 Level을 만들지 않는다.

### 2.3 기믹은 Server typed contract다

원작 기믹은 전부 다음 세 가지 조합으로 환원된다.

| 원시 개념 | Server 계약 | Client |
|---|---|---|
| 플레이어 상태 부여(카드 무늬, 광기, 삐에로, 속박) | `SERVER_PLAYER` 필드 + `PLAYER_SNAPSHOT` 복제 + stage action | 머리 위 아이콘, 게이지, 모델 교체 표현 |
| 판정(바라보기, 입력 따라하기, 무늬 일치) | stage `judgement` + fixed tick 평가 + outcome/실패 응답(EXECUTE) | 입력 window만 relay, 판정 없음 |
| 오브젝트(카드 투사체, 세이튼 분신, 룰렛) | combat object(HOMING) / summon set / world sequence 방송 | 시각 표현 |

`CBossCombatRuntime`, `BOSS_PATTERN_STAGE_ACTION_KIND`, `BOSS_PATTERN_STAGE_OUTCOME`, `SET_PLAYER_BIND`, `SET_SHIELD`,
`SPAWN_COMBAT_OBJECT_VOLLEY`, `counterProxy`, `bossResponse`가 이미 있으므로 기믹마다 enum 값과 struct 필드를 추가하는 일이지 새 시스템이 아니다.

### 2.4 Logic Graph는 편집 가능한 blueprint, writer는 하나

편집 가능한 캔버스를 추가해도 저장 경로는 `CBalanceTool` joined draft → Composition `Save + Validate + Publish` 하나다.
캔버스는 노드·핀·엣지 드래그를 **typed draft mutation 명령**으로 변환할 뿐 JSON을 직접 만지지 않는다. canonical 11 pattern의 topology 잠금
(`manualAuditions` 밖)은 그대로 유지하고 잠긴 항목은 회색 핀으로 보인다.

### 2.5 조명·분위기 세 층

```text
scene profile   Level·관문·기믹 단위의 전역 분위기 (directional, ambient, fog, exposure, bloom)  RenderingProfiles.json
map light       맵에 달린 조명 배치 (point/spot, group, 기본 on/off)                          <Area>.maplights.json v2
pattern cue     패턴 stage가 켜고 끄는 연출 (profile 전환, light group 감쇠, 스포트라이트)     <Boss>.presentation.json
```

"기믹 시작 시 전체가 어두워진다"는 stage ENTER의 `sceneInvocation{profileId: scene.kakul.blackout.v1, transitionMs: 600}`과
`lightGroupInvocation{groupId: tent.lamps, intensityMultiplier: 0.05}`이고, "세이튼이 스포트라이트를 받는다"는 summon 앵커의
`lightCue{kind: SPOT, innerCone 12°, outerCone 22°}`다. 렌더 품질 자체(PBR/CSM/TAA)는 별도 수직 슬라이스이며 이 계획은 그 위에서 튜닝
가능한 데이터와 도구를 먼저 닫는다. 쿠크 화면의 인상은 조명 기법보다 **조명 배치·색·대비(암전+스포트)** 에서 먼저 결정되므로 이 순서가 맞다.

### 2.6 건슬 AI = Server pseudo-player

companion을 world entity가 아니라 **세션 없는 `SERVER_PLAYER`(GUNSLINGER)** 로 두면 이동·스킬·쿨다운·HP·피격·nameplate·말풍선이
전부 기존 플레이어 경로를 재사용한다. Client는 새 presentation 경로 없이 `S2C_PLAYER_SPAWNED`의 `isCompanion` 플래그만 읽는다.
대사 소유권은 세 겹이다.

```text
1. 트리거   Server 패턴 stage ENTER edge, assist 시작/종료, 플레이어 질문(채팅)
2. 문장     Data/Dialogue/KakulSaydon.companion.json (기본, 결정적)
3. LLM      Tools/CompanionChat/companion_chat_sidecar.py (선택, localhost, timeout 시 2번으로 fallback)
```

Server tick은 LLM 응답을 절대 기다리지 않는다. 응답은 room command로 들어와 다음 tick에 `S2C_CHAT`으로 나간다.

---

## 3. 데이터 규칙과 규약

### 3.1 ID 규칙

| 대상 | 형식 | 예 |
|---|---|---|
| encounterId | `ENCOUNTER_KAKUL_GATE<N>` | `ENCOUNTER_KAKUL_GATE1` |
| bossArchetypeId | `BOSS_KAKUL_<역할>` | `BOSS_KAKUL_G1_KOUKU`, `BOSS_KAKUL_G2_SAYDON_LARGE` |
| summon archetypeId | `SUMMON_KAKUL_<역할>` | `SUMMON_KAKUL_SAYDON_CLONE` |
| patternId | `KAKUL_G<N>_<SNAKE>` | `KAKUL_G1_PIZZA`, `KAKUL_G1_STAGGER_130`, `KAKUL_G1_HEART_PING_110` |
| actionId | `kakul.g<N>.<pattern>.<stage>` | `kakul.g1.pizza.windup` |
| stageId | 대문자 SNAKE, 패턴 안에서 유일 | `WINDUP`, `SLICE_1`, `JUDGE` |
| combatObjectArchetypeId | `combatobject.kakul.<이름>` | `combatobject.kakul.card.heart` |
| sceneProfileId | `scene.kakul.<관문>.<분위기>.v<N>` | `scene.kakul.gate1.circus-tent.v1`, `scene.kakul.blackout.v1` |
| lightId / groupId | `light.kakul.<관문>.<source>` / `group.kakul.<관문>.<역할>` | `group.kakul.gate1.tent-lamps` |
| companionId / lineId | `companion.<class>` / `line.<boss>.<pattern>.<n>` | `companion.gunslinger`, `line.kakul.g1.heart-ping.01` |

pointer, prototype tag, vector index, 한글 표시 이름은 ID가 아니다. `sourceActionId`(4219714 등)는 reference join 키이며 runtime ID로 쓰지 않는다.

### 3.2 문서 소유권

| 문서 | schema | 정본/생성물 | validator / publisher |
|---|---|---|---|
| `Data/Kakul/Kakul.gameplay.json` | `lostark.boss-gameplay-authoring` v1 (Valtan의 현재 schema 문자열은 alias로 계속 승인) | 정본 | `Tools/KakulSaydonPipeline/Project-KakulPatternMaster.ps1 -Mode Validate\|PublishV2` (Valtan projector 함수 재사용) |
| `Data/Kakul/Kakul.presentation.json` | `lostark.boss-presentation-authoring` v1 | 정본 | 위와 같은 transaction |
| `Data/Kakul/Kakul.combatobjects.json` | `lostark.boss-combat-object-authoring` v1 | 정본 | 위와 같은 transaction |
| `Data/Encounters/KakulSaydon/KakulEncounter.json`, `KakulPatternRotations.json`, `Kakul.patternbindings.json`, `Kakul.patterneffectcues.json`, `KakulCombatObjects.json` | 기존 Valtan Product schema | 생성물 | projector가 생성, 직접 편집 금지 |
| `Data/Actors/BossCatalog.json` v7 | `lostark.boss-catalog` | 정본 | `Publish-GameplayBalance.ps1` |
| `Data/Balance/BossProfiles.json` v5 | `lostark.boss-profiles` | 정본 | `Publish-GameplayBalance.ps1` |
| `Data/Balance/DamageProfiles.json` (`madnessGain` 필드 추가) | 기존 | 정본 | `Publish-GameplayBalance.ps1` |
| `Data/Balance/PlayerSkills.json` (`KAKUL_CLOWN` Q/W/E 3행) | 기존 | 정본 | `Publish-GameplayBalance.ps1` |
| `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` (boss placement, activateEncounter) | v6 | 정본 | `Publish-WorldGameplay.ps1` |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.maplights.json` | `lostark.map-light-presentation` v2 | 정본 | `Publish-MapAuthoring.ps1` (pair) |
| `Data/Rendering/Authored/RenderingProfiles.json` | v1 (`transitionMs`는 invocation 쪽 필드) | 정본 | Rendering Workbench Save/Publish |
| `Data/Companions/GunslingerCompanion.json` | `lostark.companion-profile` v1 | 정본 | `Publish-GameplayBalance.ps1`에 domain 추가 |
| `Data/Dialogue/KakulSaydon.companion.json` | `lostark.companion-dialogue` v1 | 정본 | `Tools/CompanionChat/Validate-CompanionDialogue.ps1` |
| `Data/Compositions/Bosses/KakulSaydon.bosscomposition.json` | 기존 v1, `status SHADOW`로 승격 | manifest | `Publish-Compositions.ps1` |

모든 JSON은 stable ID unique, formatVersion exact, unknown field 거부, `parse -> validate -> stage -> commit`, 실패 시 이전 문서 보존 규칙을 따른다.

### 3.3 첫 패턴 하나의 split source 예 (G02가 실제로 커밋할 최소 형태)

`Data/Kakul/Kakul.gameplay.json`

```json
{
  "schema": "lostark.boss-gameplay-authoring",
  "formatVersion": 1,
  "bossArchetypeId": "BOSS_KAKUL_G1_KOUKU",
  "encounterId": "ENCOUNTER_KAKUL_GATE1",
  "scope": "GATE1",
  "retiredPatternIds": [],
  "decisionModel": {
    "scriptedSequence": {
      "sequenceId": "sequence.kakul.gate1.server-authored.v1",
      "mode": "ORDERED_ONCE_THEN_IDLE",
      "interStepPursuitMs": 1000,
      "patternIds": ["KAKUL_G1_ENTRANCE", "KAKUL_G1_PIZZA"],
      "transitionPursuitMs": [0, 1000]
    },
    "selectionSets": [],
    "selectionWindows": [],
    "mechanics": [],
    "manualAuditions": [
      { "patternId": "KAKUL_G1_PIZZA", "admissionState": "MANUAL_SERVER_AUDITION" }
    ]
  },
  "counterReactionLayers": [],
  "patterns": [
    {
      "patternId": "KAKUL_G1_ENTRANCE",
      "displayName": "1관문 입장 - 팝업북",
      "category": "NORMAL",
      "compatibilitySelectionWeight": 1,
      "actionId": "kakul.g1.entrance",
      "entryActionId": "kakul.g1.entrance.book",
      "targetPolicy": "NONE",
      "aimPolicy": "NONE",
      "eligibility": {
        "armorRequirement": "ANY", "phaseRequirement": "ANY",
        "minimumGameplayPhase": 1, "maximumGameplayPhase": 1,
        "minimumHealthBarInclusive": 1, "maximumHealthBarInclusive": 150,
        "minimumRangeM": 0.0, "maximumRangeM": 200.0,
        "cooldownPolicy": "DERIVED_SOURCE_ACTION", "selectionCooldownMs": null, "cooldownGroupId": null,
        "repeatPolicy": { "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE", "limit": 1 }
      },
      "invulnerableWhileRunning": true,
      "sourceActionIds": [],
      "serverMotion": null,
      "reactions": [],
      "stages": [
        {
          "stageId": "BOOK", "actionId": "kakul.g1.entrance.book", "stageKind": "WINDUP",
          "durationMs": 21010, "defaultNextActionId": null,
          "hit": { "shape": { "kind": "NONE" } }, "motion": null,
          "events": [
            { "eventId": "event.kakul.g1.entrance.book.hide", "trigger": "ENTER", "kind": "SET_BOSS_FLAG", "flagId": "boss.flag.hidden", "enabled": true },
            { "eventId": "event.kakul.g1.entrance.book.sequence", "trigger": "ENTER", "kind": "PLAY_WORLD_SEQUENCE", "sequenceInstanceId": "world.sequence.instance.circusfinale" },
            { "eventId": "event.kakul.g1.entrance.book.show", "trigger": "EXIT", "kind": "SET_BOSS_FLAG", "flagId": "boss.flag.hidden", "enabled": false }
          ],
          "branches": []
        }
      ]
    },
    {
      "patternId": "KAKUL_G1_PIZZA",
      "displayName": "나팔 - 피자",
      "category": "NORMAL",
      "compatibilitySelectionWeight": 10,
      "actionId": "kakul.g1.pizza",
      "entryActionId": "kakul.g1.pizza.windup",
      "targetPolicy": "NEAREST_AT_START",
      "aimPolicy": "LOCK_AT_START",
      "eligibility": {
        "armorRequirement": "ANY", "phaseRequirement": "ANY",
        "minimumGameplayPhase": 1, "maximumGameplayPhase": 1,
        "minimumHealthBarInclusive": 1, "maximumHealthBarInclusive": 150,
        "minimumRangeM": 0.0, "maximumRangeM": 14.0,
        "cooldownPolicy": "DERIVED_SOURCE_ACTION", "selectionCooldownMs": null, "cooldownGroupId": null,
        "repeatPolicy": { "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE", "limit": 2 }
      },
      "invulnerableWhileRunning": false,
      "sourceActionIds": [4219714],
      "serverMotion": null,
      "reactions": [],
      "stages": [
        { "stageId": "WINDUP", "actionId": "kakul.g1.pizza.windup", "stageKind": "WINDUP", "durationMs": 2500, "defaultNextActionId": "kakul.g1.pizza.slice-1",
          "hit": { "shape": { "kind": "NONE" } }, "motion": null, "events": [], "branches": [] },
        { "stageId": "SLICE_1", "actionId": "kakul.g1.pizza.slice-1", "stageKind": "ACTIVE", "durationMs": 4667, "defaultNextActionId": "kakul.g1.pizza.recovery-1",
          "hit": { "shape": { "kind": "SECTOR", "outerRadiusM": 12.0, "angleDegrees": 45.0 }, "schedule": { "kind": "EXPLICIT_OFFSETS", "offsetsMs": [1500, 2600, 3700] },
                   "serverDamageProfileId": "damage.kakul.g1.pizza", "pushRangeM": 0.0, "pushMs": 0, "knockdown": false, "downMs": 0 },
          "motion": null, "events": [], "branches": [] },
        { "stageId": "RECOVERY_1", "actionId": "kakul.g1.pizza.recovery-1", "stageKind": "RECOVERY", "durationMs": 1000, "defaultNextActionId": null,
          "hit": { "shape": { "kind": "NONE" } }, "motion": null, "events": [], "branches": [] }
      ]
    }
  ]
}
```

`Kakul.presentation.json`은 같은 `patternId/stageId/actionId`에 `animation.occurrences[]`(`clip: rpcz00_att_battle_3_01` 등),
`effectCues[]`, `cameraInvocations[]`, 그리고 G05가 추가하는 `sceneInvocations[]`, `lightGroupInvocations[]`, `lightCues[]`를 붙인다.
피자 원작은 6 stage(3_01/3_07/3_09 ×2)이므로 위 3 stage를 두 번 이어 붙인 형태가 완성본이다. 원작 피자 판정은 8분할 장판(FIXED_AREA
combat object)이며 위 예는 첫 슬라이스를 위한 caster SECTOR 판정이다. combat object 버전은 `Kakul.combatobjects.json`에 `combatobject.kakul.pizza.slice`를
추가하고 stage에 `SPAWN_COMBAT_OBJECT_VOLLEY(BOSS_RELATIVE, 8방향)`로 교체한다.

### 3.4 BossCatalog v7 / BossProfiles v5

BossCatalog v7 추가 필드는 `bodyRole`, `weaponClipSync`, `summonKind`뿐이다. 나머지는 v6 그대로다.

```json
{
  "archetypeId": "BOSS_KAKUL_G1_KOUKU",
  "visualAssetId": "KAKUL_MN_RPCZ_00",
  "presentationScale": 1.0,
  "bodyModel": "Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel",
  "weaponModel": null,
  "armorModels": [],
  "armorParts": [],
  "combatObjectVisuals": []
}
```

```json
{
  "archetypeId": "BOSS_KAKUL_G2_SAYDON_LARGE",
  "visualAssetId": "KAKUL_MN_RPCT_06",
  "presentationScale": 1.0,
  "bodyModel": "Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel",
  "weaponModel": "Character/KoukuSaton/WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel",
  "weaponClipSync": "SAME_CLIP_NAME",
  "armorModels": [],
  "armorParts": [],
  "combatObjectVisuals": []
}
```

`weaponClipSync: SAME_CLIP_NAME`은 `WP_MN_RPCT_06`의 `wprpct06_att_battle_01_*`가 본체 `mn_rpct_06_sk.ao_att_battle_01_*`와 같은 번호 clip을
동시에 재생해야 한다는 인벤토리 §1.1 결론의 typed 표현이다. Valtan의 정적 `weaponModel` 부착과 구분한다.

BossProfiles v5는 archetype당 한 행이며 `maximumHealthBars`는 관문별 초기값(150/150/100)으로 두고 `PROJECT_TUNED`로 표시한다(§5 결정 항목).
1관문 광기 규칙은 `Kakul.gameplay.json` 최상위 `encounterRules.madness`가 소유한다.

```json
"encounterRules": {
  "cardSuit": { "assignment": "RANDOM", "excludeCompanions": true },
  "madness": { "maximum": 100, "decayPerSecond": 0, "fireGainPerSecond": 12, "clownDurationMs": 15000, "clownSkillClass": "KAKUL_CLOWN" }
}
```

### 3.5 Gameplay.world.json boss placement

기존 v6 `boss` kind와 `activateEncounter` triggerBox를 그대로 쓴다. 관문 3개는 `placementId` `boss.kakul.gate1/2/3`(disabled)과 stage marker 위치의
triggerBox 3개다. MapTool이 저장하고 `Publish-WorldGameplay.ps1`이 navigation walkable을 검사한다. 세이튼 분신 4자리 anchor는 `Kakul.gameplay.json`의
`arena.anchors[]`(`anchorId, position, yawDegrees`)가 소유하며 projector가 같은 navgrid로 walkable을 검사한다. 배치 좌표를 두 문서에 복제하지 않기
위해 anchor는 world 문서가 아니라 gameplay 문서에만 둔다.

### 3.6 기믹 typed 확장 (G03이 추가하는 enum/field 총목록)

```text
BOSS_PATTERN_STAGE_ACTION_KIND  += PLAY_WORLD_SEQUENCE, ASSIGN_PLAYER_CARD_SUITS, SET_PLAYER_FORM,
                                    SPAWN_SUMMON_SET, DESPAWN_SUMMON_SET, BEGIN_INPUT_JUDGEMENT,
                                    BEGIN_FACING_JUDGEMENT, EXECUTE_ALL_PLAYERS, ROLL_ROULETTE
BOSS_PATTERN_STAGE_OUTCOME      += SHIELD_BROKEN, JUDGEMENT_ALL_PASSED, JUDGEMENT_ANY_FAILED
stage field                     += hitReflect { kind: NONE|FORWARD_ARC, arcDegrees, reflectPercent }
SET_PLAYER_BIND                 += targetPolicy { kind: ALL_ALIVE|RANDOM_ALIVE_COUNT|SUIT_FILTER, count, suit: ROULETTE_RESULT|HEART|SPADE|CLOVER|DIAMOND, excludeCompanions }
combat object movement          += HOMING { speedMps, retargetEachTick, targetPolicy: UNBOUND_ALIVE|NEAREST_ALIVE, maximumLifetimeMs }
combat object hit response      += SUIT_JUDGEMENT { objectSuit, onMatch: DESTROY_OBJECT, onMismatch: EXECUTE }
SERVER_PLAYER                   += eCardSuit, eForm, iMadnessGauge, iMadnessMaximum, iFormEndTick, iMechanicInputWindowEndTick, bIsCompanion
PLAYER_SNAPSHOT                 += eCardSuit, eForm, iMadnessGauge, iMadnessMaximum, iMechanicInputWindowEndTick, isCompanion
BOSS combat snapshot            += iMechanicVariantIndex (댄스 variant, 하트/총 shooter role)
C2S                             += C2S_MECHANIC_INPUT { iRequestSequence, eInputSlot }
```

wire 변경은 한 번에 모은다. `NETWORK_PROTOCOL_VERSION 55 -> 56`, `GAMEPLAY_BOOTSTRAP_FORMAT_VERSION 32 -> 33`은 G03-1이 올리고 G03-2 이후와 G06은
같은 버전 안에서 필드를 채운다(각 G가 버전을 따로 올리지 않도록 G03-1의 Shared 변경에 모든 필드를 한 번에 선언한다).

### 3.7 조명 문서

`<Area>.maplights.json` v2

```json
{
  "schema": "lostark.map-light-presentation",
  "formatVersion": 2,
  "areaId": "LV_LUT_MIDNIGHTC_ED",
  "provenance": "SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED",
  "groups": [
    { "groupId": "group.kakul.gate1.tent-lamps", "enabledByDefault": true },
    { "groupId": "group.kakul.gate1.stage-spots", "enabledByDefault": false }
  ],
  "lights": [
    { "lightId": "light.kakul.gate1.pointlight_12", "kind": "POINT", "groupId": "group.kakul.gate1.tent-lamps",
      "sourceLevel": "LV_LUT_MIDNIGHTC_ED_SL01", "sourceObjectId": "SL01:export:812:pointlight_12",
      "position": [66.0, 6.5, -102.0], "radiusMeters": 9.0, "falloffExponent": 2.0, "color": [1.0, 0.72, 0.35, 1.0], "brightness": 4.0 },
    { "lightId": "light.kakul.gate1.spot_stage_a", "kind": "SPOT", "groupId": "group.kakul.gate1.stage-spots",
      "sourceLevel": null, "sourceObjectId": null,
      "position": [60.0, 14.0, -96.0], "direction": [0.2, -1.0, 0.3], "innerConeDegrees": 12.0, "outerConeDegrees": 22.0,
      "radiusMeters": 24.0, "falloffExponent": 1.5, "color": [1.0, 0.95, 0.85, 1.0], "brightness": 8.0 }
  ]
}
```

v1 문서(Valtan)는 `kind` 누락을 `POINT`, `groupId` 누락을 area 기본 group으로 읽는다. 자동 추측 변환은 하지 않고 reader가 v1/v2를 모두 exact로 승인한다.

`<Boss>.presentation.json` stage 확장

```json
"sceneInvocations": [
  { "invocationId": "scene.kakul.g1.heart-ping.blackout", "profileId": "scene.kakul.blackout.v1", "transitionMs": 600, "restorePolicy": "PATTERN_END" }
],
"lightGroupInvocations": [
  { "invocationId": "lightgroup.kakul.g1.heart-ping.lamps-off", "groupId": "group.kakul.gate1.tent-lamps", "intensityMultiplier": 0.05, "transitionMs": 600, "restorePolicy": "PATTERN_END" }
],
"lightCues": [
  { "cueId": "light.kakul.g1.heart-ping.spot", "kind": "SPOT", "anchor": { "kind": "SUMMON_ROLE", "role": "ANY" }, "localOffset": [0.0, 9.0, 0.0], "direction": [0.0, -1.0, 0.0],
    "innerConeDegrees": 10.0, "outerConeDegrees": 20.0, "radiusMeters": 14.0, "color": [1.0, 0.98, 0.9, 1.0], "brightness": 10.0, "stopPolicy": "STAGE_END" }
]
```

### 3.8 Companion / Dialogue 문서

`Data/Companions/GunslingerCompanion.json`

```json
{
  "schema": "lostark.companion-profile",
  "formatVersion": 1,
  "companionId": "companion.gunslinger",
  "characterClass": "GUNSLINGER",
  "displayName": "건슬링어 (AI)",
  "chatCommands": { "assist": ["도움!", "/도움"], "guide": ["/따라와"], "dismiss": ["/그만"] },
  "assist": { "durationMs": 20000, "engageRangeM": 8.0, "preferredDistanceM": 6.0, "decisionIntervalMs": 200, "dodgeLookaheadMs": 400,
              "skillRotation": [38020, 38050] },
  "guide": { "followDistanceM": 2.5, "repositionIntervalMs": 300 },
  "dialogueDocument": "Data/Dialogue/KakulSaydon.companion.json",
  "llm": { "enabled": false, "endpoint": "127.0.0.1:7778", "timeoutMs": 1500 }
}
```

`Data/Dialogue/KakulSaydon.companion.json`

```json
{
  "schema": "lostark.companion-dialogue",
  "formatVersion": 1,
  "companionId": "companion.gunslinger",
  "encounterId": "ENCOUNTER_KAKUL_GATE1",
  "persona": { "name": "건슬링어", "systemPrompt": "너는 로스트아크 쿠크세이튼 레이드의 숙련된 공대장이다. 한 문장으로 짧게 현재 패턴의 대처법을 말한다." },
  "lines": [
    { "lineId": "line.kakul.g1.heart-ping.01", "trigger": { "kind": "PATTERN_STAGE_ENTER", "patternId": "KAKUL_G1_HEART_PING_110", "stageId": "SUMMON" },
      "text": "네 방향 세이튼 중 총을 든 쪽을 바라봐!", "durationMs": 4000, "priority": 10 },
    { "lineId": "line.kakul.g1.assist.start", "trigger": { "kind": "ASSIST_START" }, "text": "20초만 같이 친다. 뒤는 맡겨!", "durationMs": 3000, "priority": 5 }
  ]
}
```

`trigger.kind`는 `PATTERN_STAGE_ENTER | PATTERN_OUTCOME | ASSIST_START | ASSIST_END | PLAYER_QUESTION`이다. validator는 `patternId/stageId`가 해당 encounter
Product에 존재하는지, `skillRotation`이 `PlayerSkills.json`의 GUNSLINGER ACTIVE skillId인지 검사한다.

---

## 4. G별 구현 범위

### G00 — Server 재생 정책과 입장 시퀀스 동기

#### 목표와 종료 증거

Debug Server에서 Valtan Arena에 진입하면 보스가 IDLE로 대기하고, Boss Tool `Play Selected`/`Play Full Pattern`/`Auto Playback ON`이 각각
한 패턴 / 저장 순서 전체 / Release와 같은 자동 진행을 시작한다. Release 빌드는 기존과 같이 첫 engage에서 scriptedSequence를 시작한다.
Kakul `KAKUL_G1_ENTRANCE`의 `PLAY_WORLD_SEQUENCE`가 Server broadcast로 circus finale를 시작하고 21,010 ms 뒤 보스가 나타난다(G02 뒤 확인).

#### 수정·신규 파일

| 파일 | 변경 |
|---|---|
| `Shared/Public/Network/PacketMessages.h` | `VALTAN_AUDITION_OPERATION::SET_AUTOMATIC_PLAYBACK` 추가. `iTargetHealthBar`를 mode(0=HOLD, 1=AUTO)로 재사용. `VALTAN_AUDITION_RESULT::PLAYBACK_POLICY_SET` 추가 |
| `Shared/Public/Network/PacketType.h` | `NETWORK_PROTOCOL_VERSION 55 -> 56` (G03-1의 wire 필드까지 같은 bump에 포함) |
| `Server/Public/GameRoom.h` | `enum class SERVER_BOSS_PLAYBACK_POLICY : uint8_t { PRODUCT_AUTOMATIC, DEBUG_HOLD_UNTIL_COMMAND }`, 멤버 `m_eBossPlaybackPolicy` |
| `Server/Private/GameRoom.cpp` | boss entity spawn/reset 지점에서 policy가 HOLD면 `bAutomaticPatternSequenceAuditionHold = true`. `Handle_ValtanAudition`에 `SET_AUTOMATIC_PLAYBACK` 분기. `Handle_ValtanAudition`의 `REJECTED_WRONG_WORLD` 검사를 `VALTAN_ARENA \|\| KAKULSAYDON_ARENA`로 확장 |
| `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp` | `BOSS_PATTERN_STAGE_ACTION_KIND::PLAY_WORLD_SEQUENCE`와 `strSequenceInstanceId` 필드, bootstrap row parse |
| `Server/Private/GameRoom.cpp` `Stage_BossPatternStageActions` | `PLAY_WORLD_SEQUENCE` ENTER edge에서 triggerBox `playSequence`가 쓰는 같은 broadcast 함수 호출 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | `PLAY_WORLD_SEQUENCE.sequenceInstanceId`가 `MapCatalog`의 해당 area `worldsequences.json` instance에 존재하는지 검사, bootstrap row 생성 |
| `Client/Public/ValtanPatternAuditionService.h/.cpp` | `Set_AutomaticPlayback(bool bAutomatic, std::string& strOutStatus)` (기존 Submit과 같은 one-shot verdict 큐) |
| `Client/Private/BossTool.cpp` | Action bar에 `Auto Playback (Release behavior)` checkbox → 위 서비스 호출 |
| `Tools/NetworkProtocolHarness` | 새 op/result round-trip fixture |
| `Server/Private/ServerGameplayContractTests.cpp` | HOLD 정책에서 spawn 뒤 N tick 동안 `SelectPattern`이 nullptr, AUTO 전환 뒤 scriptedSequence 시작; `PLAY_WORLD_SEQUENCE` ENTER 방송 1회 |
| `CLAUDE.md`, `AGENTS.md` | protocol/bootstrap 숫자 교정, Debug 기본 HOLD 서술 |

#### H 계약 (러프)

```cpp
// Server/Public/GameRoom.h — CGameRoom private 멤버 옆
enum class SERVER_BOSS_PLAYBACK_POLICY : std::uint8_t
{
	PRODUCT_AUTOMATIC,
	DEBUG_HOLD_UNTIL_COMMAND
};
/* Debug 기본은 HOLD. Release는 audition op 자체가 거부되므로 AUTO 고정. */
SERVER_BOSS_PLAYBACK_POLICY m_eBossPlaybackPolicy =
#ifdef _DEBUG
	SERVER_BOSS_PLAYBACK_POLICY::DEBUG_HOLD_UNTIL_COMMAND;
#else
	SERVER_BOSS_PLAYBACK_POLICY::PRODUCT_AUTOMATIC;
#endif
bool Apply_BossPlaybackPolicyOnSpawn(SERVER_WORLD_ENTITY& boss);
```

```cpp
// Server/Public/GameplayCatalog.h — BOSS_PATTERN_STAGE_ACTION_KIND 끝
PLAY_WORLD_SEQUENCE
// BOSS_PATTERN_STAGE_ACTION struct
std::string strSequenceInstanceId;
```

#### 호출 흐름

```text
Room boss spawn / Fresh Arena reset
→ Apply_BossPlaybackPolicyOnSpawn: HOLD면 boss.bAutomaticPatternSequenceAuditionHold = true
→ CValtanBrain::SelectPattern 5번 분기에서 IDLE 유지
Boss Tool Play Selected → PLAY_PATTERN_ID → 기존 PendingPatternIds 경로 (큐가 비면 hold도 함께 false가 되는 기존 코드 유지)
Boss Tool Play Full Pattern → FLOW_START → bScriptedPatternPlayback 경로
Boss Tool Auto Playback ON → SET_AUTOMATIC_PLAYBACK(1) → policy=AUTO, boss.bAutomaticPatternSequenceAuditionHold=false → scriptedSequence
Stage ENTER PLAY_WORLD_SEQUENCE → Broadcast(S2C_WORLD_SEQUENCE_PLAY) → Client Consume_WorldSequencePlays → Start_ServerRequestedSequence
```

불변식: `Play Selected`가 한 패턴을 끝낸 뒤 hold가 다시 켜져야 한다(현재 코드는 큐가 비면 hold를 false로 만든다). `FinishPattern`에서
policy가 HOLD이고 Flow/Next 소유자가 없으면 hold를 다시 true로 복원한다. Repeat는 Client 서비스가 `COMPLETED` 관측 뒤 재제출하므로 Server 변경이 없다.

#### 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
Server\Bin\Debug\Server.exe --contract-test
Tools\NetworkProtocolHarness\Bin\Debug\NetworkProtocolHarness.exe
```

사용자 smoke: `Server + Client` → Lobby → Valtan → 보스가 IDLE 대기 → F1 Boss Tool → Play Selected → 한 패턴 뒤 다시 대기 → Auto Playback ON → 자동 진행.

---

### G01 — Boss Tool 재편

#### 목표와 종료 증거

Boss Tool 상단에 `Valtan | KoukuSaydon` 선택자가 있고 현재 Level의 보스가 자동 선택된다. All Patterns / Current Patterns 두 탭 모두에서
Server가 재생 중인 패턴 행 앞에 `[Live]`가 붙고 행이 강조되며 목록 아래에 `Live: <한국어 이름> / <stageId> / seq N` 한 줄이 보인다.
Action bar는 `Play Selected`, `Repeat`, `Play Full Pattern`, `Stop After Current`, `Restart Active`, `Revive` 순서다. `Next Pattern...`은 선택된
보스의 inventory만 보인다. Valtan 기능 회귀 없음(기존 `test_valtan_boss_tool_pattern_flow_contract` PASS).

#### 수정·신규 파일

| 파일 | 변경 |
|---|---|
| `Client/Public/BossTool.h` | `struct BOSS_TOOL_TARGET`, `m_Target`, `Select_Target`, `Render_TargetSelector`, `Is_TargetLevelActive` |
| `Client/Private/BossTool.cpp` | 창 제목 `Boss Tool`, `Render_PatternList`/`Render_CurrentPatternList` 행 `[Live]` 마커, `Render_ActionBar` 재배치, `Render_NextPatternPicker` target 필터 |
| `Client/Public/ValtanPatternTree.h/.cpp` | `CValtanPatternTree::Load(const BOSS_SOURCE_DESCRIPTOR&)` 오버로드 (G02에서 Kakul descriptor 연결; G01은 Valtan descriptor 하나) |
| `Client/Private/MainApp.cpp` | F1 허브의 `Boss Tool` 항목이 `VALTAN_ARENA` 외 `KAKULSAYDON_ARENA`에서도 열리게 gating 확장(`MainApp.cpp:3678/3744/3795`의 VALTAN_ARENA 비교) |
| `Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py` | 새 버튼 라벨·Live 마커 text oracle |

#### H 계약 (러프)

```cpp
// Client/Public/BossTool.h — CBossTool public struct들 옆
struct BOSS_TOOL_TARGET final
{
	std::string strTargetId;        // "valtan" | "kakul-gate1"
	std::string strDisplayName;     // "Valtan" | "KoukuSaydon 1관문"
	std::string strEncounterId;     // ENCOUNTER_VALTAN | ENCOUNTER_KAKUL_GATE1
	std::string strBossPlacementId; // world Gameplay.world.json boss placementId
	LEVEL eLevel = LEVEL::END;      // 이 target을 live로 조작할 수 있는 Level
};
static const std::array<BOSS_TOOL_TARGET, 2u>& Get_Targets();
bool_t Select_Target(std::string_view strTargetId, std::string& strOutStatus);
[[nodiscard]] bool_t Is_TargetLevelActive() const;
void Render_TargetSelector();
void Render_LiveRowMarker(const VALTAN_PATTERN_VIEW& Pattern, bool_t bSelectedRow);
```

`BOSS_TOOL_TARGET.strBossPlacementId`는 기존 `BOSS_PLACEMENT_ID` 상수를 대체하는 값이며 Valtan은 현재 상수 값을 그대로 쓴다.
현재 Level과 다른 target을 고르면 목록은 read-only(Product inventory)로 보이고 모든 Server 명령은 `Is_TargetLevelActive()`로 fail-close한다.

#### 호출 흐름

```text
Render → Render_TargetSelector (Level 진입 시 자동 Select_Target)
→ Reload_CanonicalGraph(m_Target descriptor)
→ Render_PatternList: 각 행에서 Pattern.strPatternId == m_strLivePatternId 이면 "[Live] " 접두 + TableSetBgColor
→ 목록 하단 Live 한 줄 (m_strLivePatternId, m_strLiveStageId, HUD_BOSS_STATE.iPatternSequence)
Play Selected → Submit_SelectedPattern (기존)
Repeat        → m_bRepeat (기존, COMPLETED 관측 시 재제출)
Play Full Pattern → Restart_SavedFlow (기존 함수, Action bar로 이동)
Stop After Current → CValtanPatternFlowService::Stop_AfterCurrent (기존)
Next Pattern... → Build_AdmittedPatternIds가 m_Target.strEncounterId 기준 inventory만 반환
```

#### 검증

```powershell
python -m unittest Tools.ValtanPipeline.test_valtan_boss_tool_pattern_flow_contract
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

사용자 smoke: Valtan에서 `[Live]` 행이 Server 재생을 따라 움직이는지, `Play Full Pattern` 뒤 Current Patterns의 `[Live]`가 01부터 내려가는지.

---

### G02 — 쿠크세이튼 첫 수직 슬라이스 (보스가 서고 패턴 하나가 Server에서 돈다)

#### 목표와 종료 증거

Debug `Server + Client`로 Character Select → 쿠크 아레나 진입 → 1관문 trigger box 진입 → `KAKUL_G1_ENTRANCE`가 팝업북 시퀀스를 방송하고
21초 뒤 `BOSS_KAKUL_G1_KOUKU`가 보인다(HOLD 정책이면 여기서 대기). Boss Tool target `KoukuSaydon 1관문`에서 `KAKUL_G1_PIZZA`를 Play Selected하면
Server snapshot에 `patternId/actionId`가 흐르고 Client가 `rpcz00_att_battle_3_01/3_07/3_09`를 재생하며 SECTOR 판정으로 데미지가 들어온다.
`Server.exe --contract-test`, `NetworkProtocolHarness`, `Publish-GameplayBalance -Mode Validate`, 새 `Project-KakulPatternMaster.ps1 -Mode Validate` PASS.

#### 수정·신규 파일

| 파일 | 상태 | 역할 |
|---|---|---|
| `Data/Kakul/Kakul.gameplay.json`, `Kakul.presentation.json`, `Kakul.combatobjects.json` | 신규 | §3.3 |
| `Data/Actors/BossCatalog.json` v7 | 수정 | `BOSS_KAKUL_G1_KOUKU` (+ G2/G3 archetype은 §5 결정 뒤) |
| `Data/Balance/BossProfiles.json` v5 | 수정 | `BOSS_KAKUL_G1_KOUKU` 행, `encounterId ENCOUNTER_KAKUL_GATE1` |
| `Data/Balance/DamageProfiles.json` | 수정 | `damage.kakul.g1.pizza` |
| `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` | 수정 | `boss.kakul.gate1`(disabled, archetype `BOSS_KAKUL_G1_KOUKU`, SL01 근처) + `trigger.kakul.gate1.activate`(activateEncounter) |
| `Data/Encounters/KakulSaydon/*.json` | 생성물 | projector 출력 |
| `Data/Compositions/Bosses/KakulSaydon.bosscomposition.json` | 수정 | `bossArchetypeId`, `encounterId`, gameplay/presentation/combatobject source role 3개 추가, `patterns` 2개, `status SHADOW` |
| `Tools/KakulSaydonPipeline/Project-KakulPatternMaster.ps1`, `kakul_tuning_pipeline.py` | 신규 | `valtan_tuning_pipeline.py`의 projector/validator를 `BOSS_SOURCE_DESCRIPTOR`로 호출하는 얇은 진입. 함수 복제 금지 |
| `Tools/ValtanPipeline/valtan_tuning_pipeline.py` | 수정 | 경로·encounterId·archetype 상수를 descriptor 인자로 승격. Valtan 기본 descriptor는 현재 값 그대로 |
| `Tools/Build/BuildDomains.json` | 수정 | `kakul.product` validation domain (valtan.product와 같은 profile) |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | 수정 | encounter 디렉터리 목록에 `KakulSaydon` 추가, BossCatalog v7 / BossProfiles v5 검증, `GAMEPLAY_BOOTSTRAP_FORMAT_VERSION 33` |
| `Shared/Public/GameplayDataRevision.h` | 수정 | `GAMEPLAY_BOOTSTRAP_FORMAT_VERSION = 33` |
| `Server/Private/GameplayCatalog.cpp` | 수정 | v7/v5 parse, `weaponClipSync`, Kakul encounter row 승인. `hasExactValtanHighJumpTypedVolley` 같은 Valtan exact 검사가 Kakul row에 적용되지 않도록 encounterId로 분기 |
| `Server/Private/ValtanBrain.cpp`, `GameRoom.cpp` | 수정 | `ENCOUNTER_VALTAN`/`BOSS_VALTAN` 문자열 하드코딩 전수 조사 후 boss entity의 `strEncounterId/strArchetypeId`로 치환. 클래스 이름은 유지(리네임은 별도 합의) |
| `Client/Public/BossSourceRegistry.h`, `Client/Private/BossSourceRegistry.cpp` | 신규 | `BOSS_SOURCE_DESCRIPTOR` 2행 (Valtan, Kakul gate1). Boss Tool, All Effects, Composition, Sequencer가 같은 descriptor로 tree를 연다 |
| `Client/Private/ValtanPatternTree.cpp` | 수정 | 경로 상수를 descriptor로 치환 |
| `Client/Public/ValtanPresentationAssetService.h/.cpp`, `Client/Private/ClientReplication.cpp` | 수정 | `BOSS_*` archetype presentation을 BossCatalog 행 기준으로 lazy prototype commit (현재 Valtan 고정 부분 일반화), `weaponClipSync` 소비 |
| `Client/Private/Level_KakulSaydonArena.cpp` | 수정 | boss presentation admission을 Valtan Level과 같은 `CClientReplication` 경로로 연결, `boss.flag.hidden` 표현(GHOST_HIDDEN flag 재사용) |
| `Client/Private/Animation_Tool.cpp` | 수정 | `Build_KakulPatternFromAction`이 REFERENCE_ONLY patternbindings 대신 `Kakul.presentation.json` Create Pattern draft를 만드는 intake adapter (Valtan `promote_valtan_animation_chains.py` 경로 재사용) |
| `Tools/ValtanPatternAuditionServiceHarness` | 수정 | Kakul descriptor로 canonical graph load / `KAKUL_G1_PIZZA` stage join 계약 |
| `Server/Private/ServerGameplayContractTests.cpp` | 수정 | Kakul gate1 bootstrap load, PLAY_PATTERN_ID로 `KAKUL_G1_PIZZA` 실행, SECTOR hit 1회 이상 |

#### H 계약 (러프)

```cpp
// Client/Public/BossSourceRegistry.h
struct BOSS_SOURCE_DESCRIPTOR final
{
	std::string strTargetId;
	std::string strEncounterId;
	std::string strBossArchetypeId;
	std::string strAreaId;
	std::filesystem::path GameplaySourcePath;
	std::filesystem::path PresentationSourcePath;
	std::filesystem::path CombatObjectSourcePath;
	std::filesystem::path EncounterProductDirectory;
	std::filesystem::path BossCompositionPath;
	std::string strProductProjectionCommand;   // Project-*PatternMaster.ps1 -Mode PublishV2
};
class CBossSourceRegistry final
{
public:
	static const BOSS_SOURCE_DESCRIPTOR* Find(std::string_view strTargetId);
	static const BOSS_SOURCE_DESCRIPTOR* Find_ByEncounter(std::string_view strEncounterId);
	static const std::vector<BOSS_SOURCE_DESCRIPTOR>& All();
};
```

```cpp
// Data/Actors/BossCatalog.json v7 을 읽는 Server/Client 공통 필드
enum class BOSS_WEAPON_CLIP_SYNC : std::uint8_t { STATIC_ATTACH, SAME_CLIP_NAME };
```

#### 호출 흐름

```text
저작: Animation Tool(Resource Files -> MN_RPCZ_00 -> 4219714) -> Create Pattern draft -> Composition Workbench -> Save
→ Run-ValtanAuthoringSaveJob.ps1 (descriptor 인자) -> Kakul source CAS commit -> Project-KakulPatternMaster PublishV2
→ Publish-GameplayBalance -> Gameplay.bootstrap(v33) -> Server 재시작
런타임: trigger activateEncounter -> boss entity 활성 -> HOLD 대기 -> Boss Tool PLAY_PATTERN_ID
→ CValtanBrain::SelectPattern -> stage fixed tick -> snapshot -> CClientReplication -> Kakul boss presentation clip 재생
```

#### 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/KakulSaydonPipeline/Project-KakulPatternMaster.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
```

---

### G03 — 1관문 기믹

모든 하위 G는 `Data(gameplay/presentation) -> projector -> bootstrap -> Server stage action/judgement -> snapshot -> Client 표현/입력 relay -> harness` 한 수직 슬라이스다.
Client는 어떤 하위 G에서도 판정하지 않는다.

#### G03-1 카드 무늬 분배와 wire 확장 (다른 하위 G의 선행)

- Shared: `enum class KAKUL_CARD_SUIT : uint8_t { NONE, HEART, SPADE, CLOVER, DIAMOND, END }`, `enum class PLAYER_FORM : uint8_t { NORMAL, CLOWN, END }`,
  `PLAYER_SNAPSHOT` §3.6 필드 전부, `S2C_PLAYER_SPAWNED.isCompanion`, `C2S_MECHANIC_INPUT`. `NETWORK_PROTOCOL_VERSION 56`(G00과 같은 bump).
- Server: `SERVER_PLAYER` 필드, stage action `ASSIGN_PLAYER_CARD_SUITS { assignment: RANDOM|ORDERED, excludeCompanions }`. `encounterRules.cardSuit`가 기본값이고
  1관문 입장 패턴 `KAKUL_G1_ENTRANCE` EXIT에서 한 번 실행. RANDOM은 room seed 결정적 shuffle, ORDERED는 `PLAYER_ID` 오름차순으로 HEART, CLOVER, SPADE, DIAMOND.
- Client: `CCombatHUDViewModel::HUD_PLAYER_STATE.eCardSuit`, `CWorldPlayerCardSuitView`(nameplate 위 `UI/KakulSaydon/card_<suit>.png`, `Data/UI/KakulSaydon/CardSuit_Layout.json`).
  UI 이미지는 팀장 Drive `Client/Bin/Resources/UI/KakulSaydon/`에 둔다(§5 자산 항목).
- harness: NetworkProtocol round-trip, Server contract(4인 RANDOM이 4종 유일, ORDERED 고정 순서, companion 제외).

#### G03-2 광기 게이지와 삐에로 변신

- Server: `iMadnessGauge`는 보스 hit가 적용될 때 `DamageProfiles.json`의 `madnessGain`만큼, 화염 combat object 안에 있을 때 `fireGainPerSecond`만큼 증가.
  `maximum` 도달 시 `SET_PLAYER_FORM CLOWN`과 동등한 내부 전이(`iFormEndTick = now + clownDurationMs`), 종료 시 NORMAL. CLOWN 중 `(class, slot)` 해석은
  `CGameplayCatalog`가 `KAKUL_CLOWN` pseudo-class 스킬 행(Q/W/E)을 사용하고 원래 class 스킬은 silence와 같은 마스크로 막는다.
- Data: `PlayerSkills.json`에 `characterClass: "KAKUL_CLOWN"` 3행(원작 삐에로 스킬 수치는 사용자 결정), `CharacterCatalog.json`에 `KAKUL_CLOWN`은 `playable=false`.
  Lobby roster에는 절대 노출하지 않는다(`AGENTS.md` 여섯 class 규칙).
- Client: `CPlayerSkillCatalog::Resolve(effectiveClass, slot)`에서 `HUD_PLAYER_STATE.eForm == CLOWN`이면 `KAKUL_CLOWN`으로 조회. HUD 광기 게이지는 identity 게이지와
  같은 fill-ratio 슬롯(`Data/UI/KakulSaydon/Madness_Layout.json`). 삐에로 모델 교체는 §5 자산 확인 뒤 `CClientReplication`의 class replacement transaction과 같은 경로로
  form replacement를 추가한다. 모델이 없으면 게이지·스킬 마스크·Q/W/E까지만 이번 범위다.

#### G03-3 130줄 무력화 (보라 방패, 정면 반사, 실패 전멸)

- gameplay: `decisionModel.mechanics`에 `{ patternId: KAKUL_G1_STAGGER_130, trigger: HEALTH_BAR_CROSSING 130, oncePerEncounter, failurePolicy: ABORT_ENCOUNTER_REQUIRE_RESET }`.
  패턴 stage: `SHIELD_UP`(ENTER `SET_SHIELD maximum`, `hitReflect { FORWARD_ARC, 180°, 100% }`, `invulnerableWhileRunning=false`, branches
  `SHIELD_BROKEN -> STAGGER_SUCCESS`, `TIMEOUT -> WIPE`), `STAGGER_SUCCESS`(groggy clip, `SET_BOSS_FLAG groggy`), `WIPE`(ENTER `EXECUTE_ALL_PLAYERS`).
- Server: `CBossCombatRuntime::Apply_PlayerHit`가 stage `hitReflect`와 `BOSS_INCOMING_HIT.fSourceX/Z`로 정면 여부를 판정해 `BOSS_HIT_RESULT.iReflectedDamage`를 채우고
  GameRoom이 source player에게 `DAMAGE_EVENT`로 적용. shield가 0이 되는 tick에 `Publish_PatternOutcome(SHIELD_BROKEN)`. `EXECUTE_ALL_PLAYERS`는 companion 포함 전원 HP 0.
- Client: HUD 보스 shield bar(기존 `iCurrentShield/iMaximumShield` 필드 사용, 보라 tint), 반사 피격은 기존 damage event 표현.

#### G03-4 110줄 하트핑 (암전, 분신 4, 바라보기 판정, 스포트라이트)

- gameplay: `KAKUL_G1_HEART_PING_110` stages `VANISH`(ENTER `SET_BOSS_FLAG hidden`), `SUMMON`(ENTER `SPAWN_SUMMON_SET { summonSetId, archetypeId: SUMMON_KAKUL_SAYDON_CLONE, anchorIds: 4, roles: [SHOOTER ×1, HEART ×3] 랜덤 배정, lifetimeMs }`),
  `JUDGE`(ENTER `BEGIN_FACING_JUDGEMENT { targetRole: SHOOTER, toleranceDegrees: 45, resolveAtMs: 2500, failureResponse: EXECUTE }`, branches `JUDGEMENT_ALL_PASSED -> RETURN`, `JUDGEMENT_ANY_FAILED -> RETURN`),
  `RETURN`(ENTER `DESPAWN_SUMMON_SET`, `SET_BOSS_FLAG hidden false`).
- Server: summon set은 Esther summon과 같은 world entity 생성 경로를 쓰되 `SERVER_SUMMON_SET` 원장(`summonSetId, entity ids, roles`)을 boss entity가 소유한다.
  facing 판정은 resolve tick에 살아 있는 non-companion 각 player의 yaw와 shooter 방향의 각도 차로 결정한다.
- presentation: summon role별 animation(`SHOOTER: rpct05 4219842 총잡이 진짜`, `HEART: 4219832 하트 진짜`)은 `Kakul.presentation.json` `summonPresentations[]`가 소유하고
  `iMechanicVariantIndex`로 role을 복제한다. `sceneInvocations`(blackout), `lightGroupInvocations`(lamps 0.05), `lightCues`(summon 앵커 SPOT)는 G05 계약을 소비한다.
- Client: 분신 entity presentation은 world entity spawn 경로(`S2C_WORLD_ENTITY_SPAWNED` + BossCatalog summon archetype).

#### G03-5 85줄 댄스타임 (Q/W/E/R 따라하기)

- gameplay: `KAKUL_G1_DANCE_85` stages `INTRO`, `STEP_1..STEP_3`(각 ENTER `BEGIN_INPUT_JUDGEMENT { variants: [{inputSlot: Q, variantId: dance.power}, {W, dance.stab}, {E, dance.cheer}, {R, dance.break}], windowMs: 3000, failureResponse: EXECUTE }`,
  branches `JUDGEMENT_ANY_FAILED -> PUNISH`, `JUDGEMENT_ALL_PASSED -> next`), `PUNISH`, `RECOVERY`.
- Server: variant는 room seed로 선택해 `iMechanicVariantIndex`로 복제. window 동안 각 player의 첫 `C2S_MECHANIC_INPUT`만 채택(재입력 무시). 미입력도 실패.
- presentation: stage `animation.variants[]`(variantId → clip: `4219906 힘자랑 / 4219907 찌르기 / 4219908 만세 / 4219909 브레이크댄스`는 3관문 07 profile이며 1관문 쿠크(RPCZ_00)에는
  댄스 clip이 없다. **1관문 댄스타임은 세이튼(05 모델) 분신이 추는 원작 구조이므로 `SPAWN_SUMMON_SET` 1개 + summon presentation variant로 구현한다**(§5 결정).
- Client: `CPlayerController::Set_MechanicInputWindow(bool bOpen)` — window가 열려 있으면 Q/W/E/R press를 `Request_MechanicInput(slot)`으로 보내고 스킬 제출은 막는다.
  HUD는 window 남은 시간과 요구 입력 아이콘을 표시한다(입력 아이콘은 `KakulSaydon` UI 도메인).

#### G03-6 카드 맞추기 (3인 속박, 유도 카드, 무늬 불일치 즉사)

- gameplay: `KAKUL_G1_CARD_MATCH` stages `BIND`(ENTER `SET_PLAYER_BIND { targetPolicy: RANDOM_ALIVE_COUNT 3, excludeCompanions }`), `FIRE_1..FIRE_4`(각 `SPAWN_COMBAT_OBJECT combatobject.kakul.card.<suit>`,
  suit는 room seed 순열), `RELEASE`(EXIT bind 해제).
- combatobjects: `combatobject.kakul.card.heart` 등 4개, `kind: MISSILE`, `movement: HOMING { speedMps: 9, retargetEachTick: true, targetPolicy: UNBOUND_ALIVE, maximumLifetimeMs: 12000 }`,
  hit `trigger CONTACT`, `response: SUIT_JUDGEMENT { objectSuit: HEART, onMatch: DESTROY_OBJECT, onMismatch: EXECUTE }`.
- Server: `CCombatObjectRuntime`에 HOMING 이동과 SUIT_JUDGEMENT 응답 추가. contact 시 `player.eCardSuit == objectSuit`면 오브젝트 소멸, 아니면 EXECUTE.
- Client: BossCatalog `combatObjectVisuals` 4행(`effectAssetId` 카드 Effect는 §5 자산). 유도 이동은 기존 combat object 2-tick 보간.

#### G03-7 50줄 룰렛

- gameplay: `KAKUL_G1_ROULETTE_50` stages `SPIN`(ENTER `PLAY_WORLD_SEQUENCE world.sequence.instance.<roulette>` + `ROLL_ROULETTE { outcomes: [HEART, SPADE, CLOVER, DIAMOND] }`, durationMs 22200),
  `RESULT`(후속 stage action의 `targetPolicy.suit: ROULETTE_RESULT`로 참조). 결과가 무엇을 유발하는지는 §5 결정 항목이며, 결정 전에는 `RESULT` stage가 `SET_PLAYER_BIND { SUIT_FILTER ROULETTE_RESULT, 3000ms }`
  하나만 갖는다(placeholder가 아니라 실제 동작하는 최소 규칙).
- Server: roulette 결과는 room seed로 결정하고 boss combat snapshot `iMechanicVariantIndex`로 복제한다. Client 룰렛 소품 정지 위치와 Server 결과가 일치하도록
  world sequence의 정지 각도 4종을 `variantId`별 instance 4개(`roulette.heart` 등)로 두고 `PLAY_WORLD_SEQUENCE`가 variant instance를 고른다.

#### 검증(G03 공통)

```powershell
Server\Bin\Debug\Server.exe --contract-test
Tools\NetworkProtocolHarness\Bin\Debug\NetworkProtocolHarness.exe
powershell -ExecutionPolicy Bypass -File Tools/KakulSaydonPipeline/Project-KakulPatternMaster.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
```

Server contract 최소 시나리오: (a) 4인 카드 유일성, (b) madness 누적과 CLOWN 전이·복귀, (c) shield 0 → SHIELD_BROKEN, TIMEOUT → 전원 HP 0, 정면 hit 반사,
(d) shooter 반대 방향 player만 EXECUTE, (e) 잘못된 slot/미입력 실패·정답 통과, (f) 불일치 suit contact EXECUTE·일치 시 오브젝트 소멸, (g) roulette variant 결정성.

---

### G04 — Logic Graph 편집기 (blueprint)

#### 목표와 종료 증거

F1 `Logic Pattern` 창이 한 Pattern의 stage 노드와 outcome 핀을 그리고, 핀에서 노드로 드래그하면 branch가 draft에 추가되며, 노드 우클릭 inspector에서
duration·stageKind·counterProxy·bossResponse threshold·event·judgement를 편집한다. 편집은 Composition Workbench의 dirty 표시로 즉시 보이고 `Save + Validate + Publish`로만
저장된다. canonical 잠금 pattern은 핀이 회색이고 드래그가 거부 사유를 표시한다. Live overlay(현재 action 강조, observed edge)는 유지된다.
상위 encounter 그래프(scriptedSequence step, mechanics, selection window)도 같은 창에서 탭으로 열리고 mechanic의 healthBar/order/failurePolicy를 편집한다.

#### 수정·신규 파일

| 파일 | 상태 | 역할 |
|---|---|---|
| `Client/Public/BossLogicGraphEditor.h`, `Client/Private/BossLogicGraphEditor.cpp` | 신규 | 편집 가능한 캔버스. `CBossLogicFlowRenderer`의 그리기 코드를 옮기고 핀·드래그·inspector를 추가 |
| `Client/Public/BossLogicFlowView.h` | 수정 | `BOSS_LOGIC_FLOW_NODE_VIEW`에 outcome 핀 목록, 잠금 이유, judgement 요약 추가 |
| `Client/Public/BalanceTool.h`, `Client/Private/BalanceTool.cpp` | 수정 | typed draft mutation API: `Set_StageBranch`, `Remove_StageBranch`, `Set_StageCounterProxy`, `Set_StageBossResponse`, `Add_StageEvent`, `Remove_StageEvent`, `Set_StageJudgement`, `Set_MechanicTrigger`. 각각 `parse -> validate -> stage -> commit`이며 실패 시 draft 불변 |
| `Client/Private/BossTool.cpp` | 수정 | `Render_LogicPatternContent`가 editor를 호출. `Render_LogicFlowTab`은 encounter 그래프 탭 추가 |
| `Client/Private/ActionCompositionWorkbench_Blueprint.cpp` | 수정 | `Render_BossPatternWindow`가 같은 editor를 재사용(두 캔버스 코드 삭제). outcome override preview는 editor의 preview 모드로 흡수 |
| `Tools/ValtanPipeline/test_action_composition_workbench_regression_oracles.py` | 수정 | editor mutation이 draft API만 호출하고 JSON writer를 직접 호출하지 않는 text oracle |
| `Client/Default/Client.vcxproj`, `.filters` | 수정 | 신규 2파일, 필터 `03. Tools\01. Boss` |

#### H 계약 (러프)

```cpp
// Client/Public/BossLogicGraphEditor.h
enum class BOSS_LOGIC_GRAPH_MUTATION_KIND : std::uint8_t
{
	NONE,
	SET_BRANCH,
	REMOVE_BRANCH,
	SET_COUNTER_PROXY,
	SET_BOSS_RESPONSE,
	ADD_EVENT,
	REMOVE_EVENT,
	SET_JUDGEMENT,
	SET_STAGE_DURATION,
	SET_STAGE_KIND,
	SET_MECHANIC_TRIGGER
};
struct BOSS_LOGIC_GRAPH_MUTATION final
{
	BOSS_LOGIC_GRAPH_MUTATION_KIND eKind = BOSS_LOGIC_GRAPH_MUTATION_KIND::NONE;
	std::string strPatternId;
	std::string strStageId;
	std::string strOutcome;
	std::string strTargetActionId;
	std::string strTargetPatternId;
	std::string strEventId;
	std::string strMechanicId;
	std::uint32_t iValueMs = 0u;
	std::uint32_t iValueCount = 0u;
	float fValue = 0.f;
	std::string strEnumValue;
};
struct BOSS_LOGIC_GRAPH_EDIT_CONTEXT final
{
	bool_t bTopologyEditable = false;   // manualAuditions 안
	bool_t bTuneEditable = false;       // canonical에도 허용되는 제자리 튜닝
	std::string_view strLockReason;
	std::string_view strLiveActionId;
	const BOSS_LOGIC_FLOW_OBSERVED_EDGE* pObservedEdge = nullptr;
};
/* 캔버스는 mutation을 반환할 뿐 draft를 만지지 않는다. 소유자가 CBalanceTool에 제출한다. */
class CBossLogicGraphEditor final
{
public:
	static bool_t Render(
		const BOSS_LOGIC_FLOW_VIEW& View,
		const BOSS_LOGIC_GRAPH_EDIT_CONTEXT& Context,
		BOSS_LOGIC_FLOW_CANVAS_STATE& InOutState,
		BOSS_LOGIC_FLOW_SELECTION& OutSelection,
		std::vector<BOSS_LOGIC_GRAPH_MUTATION>& OutMutations);
};
```

#### 호출 흐름

```text
BossTool::Render_LogicPatternContent
→ CBossLogicGraphEditor::Render (pin drag / inspector) → OutMutations
→ CBalanceTool::Apply_LogicGraphMutation(mutation, status)  [validate → stage → commit draft, 실패 시 draft 보존·status]
→ Workbench dirty → Save + Validate + Publish (기존 Run-ValtanAuthoringSaveJob / Kakul descriptor)
→ exact reopen → Boss Tool Reload_CanonicalGraph → editor View 재투영
```

#### 검증

```powershell
python -m unittest Tools.ValtanPipeline.test_action_composition_workbench_regression_oracles
Tools\ValtanPatternAuditionServiceHarness\Bin\Debug\ValtanPatternAuditionServiceHarness.exe
```

사용자 smoke: `VALTAN_TRIPLE_COUNTER`에서 `COUNTER_HIT` 핀을 다른 pattern 노드로 드래그 → Save → Logic Pattern live 강조가 새 edge를 따라가는지.

---

### G05 — 조명·렌더링

#### G05-1 Kakul map light 추출과 Level 연결

- `Tools/LevelPlacementExtractor/extract_map_lights.py`(신규, `sync_valtan_tower_phase_registration.py`의 light 추출 부분 일반화) → `LV_LUT_MIDNIGHTC_ED.maplights.json` v2, `MapCatalog.json` pair 등록.
- `CMapLightDocument` v2 reader(`kind`, `groupId`, spot 필드), `MAX_LIGHT_COUNT`는 유지하되 **활성 group만** 제출한다(관문별 group). `CLevel_KakulSaydonArena`가 Valtan Level과 같이
  `CMapLightPresentationRuntime`을 소유하고 실패 시 진입 실패로 fail-close.
- MapTool light authoring panel: 추가/이동/색/반경/cone/group, Save는 `Publish-MapAuthoring.ps1` pair transaction. MapTool은 source를, 제품 Level은 runtime을 읽는 기존 경계 유지.

#### G05-2 Engine spot light

- `Engine_Typedef.h` `LIGHT` enum에 `SPOT`, `LIGHT_DESC`에 `float fInnerConeCos, fOuterConeCos` 추가(size static_assert 92 → 100, offset 갱신, 모든 소비자 재빌드).
- `Shader_Deferred.hlsl` `PS_MAIN_SPOT` pass(point attenuation × `smoothstep(outerCos, innerCos, dot(-L, dir))`), `CLight::Render_Desc` pass 선택, technique/pass 등록.
- Engine public header 변경이므로 `Invoke-BuildAndRegression -Profile Product` 필수, `Test-CompiledShaderClosure.ps1` PASS.

#### G05-3 Scene profile Kakul 세트와 blend

- `RenderingProfiles.json`에 `scene.kakul.gate1.circus-tent.v1`(따뜻한 저조도, fog 약), `scene.kakul.gate2.poker-hall.v1`(녹색·금색 대비), `scene.kakul.gate3.theater.v1`(붉은 무대), `scene.kakul.blackout.v1`(exposure 0.15, ambient 0.02).
  `LevelRegistry` Kakul 기본 profile → gate1. 값은 Rendering Workbench에서 사용자가 튜닝한다.
- `CRenderingProfileService::Begin_Transition(profileId, transitionMs)`, `Update(fTimeDelta)`: LIGHT_DESC/fog/exposure/bloom multiplier를 선형 보간해 매 frame `Apply_ActiveProfile`.
  `Get_ActiveProfileId()`는 target을 반환하고 `Is_Transitioning()`을 추가. `CMainApp`이 frame마다 `Update` 호출.

#### G05-4 Pattern light cue (기믹 연출)

- `Kakul.presentation.json`/`Valtan.presentation.json` stage에 §3.7 세 배열 추가, projector가 profileId/groupId 존재 검증, Product `*.patternlightcues.json` 생성.
- Client `CBossPatternLightPresentation`(신규): boss action ENTER/EXIT edge에서 `sceneInvocation`→`Begin_Transition`, `lightGroupInvocation`→`CMapLightPresentationRuntime::Set_GroupMultiplier`,
  `lightCue`→`CPresentation_Manager::Add_TransientLight`(anchor: BOSS root / SUMMON_ROLE entity / WORLD). restorePolicy는 stage/pattern 종료 edge에서 역순 복원. Effect cue와 같은 소비 시점을 쓴다.
- Sequencer v1 `SCENE_PROFILE` track(09-03 PLAN §3)은 컷신 전용이며 이 G에서는 Kakul 관문 클리어 sequence 하나만 연결한다.

#### G05-5 렌더 품질 로드맵 위치

이 계획은 09-03 §12.2의 로드맵을 바꾸지 않는다. 쿠크 화면에 효과가 큰 순서로 별도 슬라이스를 권고한다: (1) spot light + emissive bloom 튜닝(G05-2/3으로 대부분 확보), (2) CSM, (3) PBR-lite(roughness/metal 텍스처가 있는 asset부터), (4) TAA, (5) ACES.
Benchmark 섹션이 각 단계 전후 비용을 잰다.

#### 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/Build/Test-CompiledShaderClosure.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

사용자 smoke: 쿠크 진입 → 텐트 램프 point light가 보이는지 → Boss Tool `KAKUL_G1_HEART_PING_110` Play Selected → 600 ms 암전 → 분신 4개 스포트라이트 → 패턴 종료 복원.
화면 판정은 사용자 전용이다.

---

### G06 — 건슬 AI 컴패니언과 대사

#### 목표와 종료 증거

쿠크 아레나에서 채팅에 `도움!`을 입력하면 Server가 GUNSLINGER companion을 소환해 20초 동안 보스를 공격하고 활성 stage hit shape를 피하며, 종료 뒤 소유자 2.5 m 뒤를 따라다닌다.
Server 패턴 stage ENTER마다 `KakulSaydon.companion.json`의 line이 companion 머리 위 말풍선과 채팅 로그에 나온다. `llm.enabled=true`이고 sidecar가 떠 있으면 플레이어 질문에 LLM 답이
같은 말풍선으로 나오고, sidecar가 없거나 1.5초를 넘기면 scripted line으로 fallback한다. companion은 party roster, `ROOM_FULL`, 카드 분배, 기믹 대상에서 제외된다.

#### 수정·신규 파일

| 파일 | 상태 | 역할 |
|---|---|---|
| `Data/Companions/GunslingerCompanion.json`, `Data/Dialogue/KakulSaydon.companion.json` | 신규 | §3.8 |
| `Tools/CompanionChat/companion_dialogue.py`, `Validate-CompanionDialogue.ps1`, `test_companion_dialogue_contract.py` | 신규 | schema/ID/trigger join 검증, `Publish-GameplayBalance` domain에 `companion.profile` 추가 |
| `Tools/CompanionChat/companion_chat_sidecar.py`, `companion_chat.config.json` | 신규 | localhost:7778 line-delimited JSON, OpenAI-compatible `/v1/chat/completions`. API key는 환경 변수 `LOSTARK_COMPANION_LLM_API_KEY`만 읽는다. Git에 key 금지 |
| `Shared/Public/Network/PacketMessages.h` | 수정 | `S2C_PLAYER_SPAWNED.isCompanion`, `PLAYER_SNAPSHOT.isCompanion`(G03-1 bump에 포함) |
| `Server/Public/CompanionSystem.h`, `Server/Private/CompanionSystem.cpp` | 신규 | `SERVER_COMPANION_STATE`, `CCompanionSystem`(spawn/dismiss/mode/tick), `CCompanionBrain`(follow/assist 결정) |
| `Server/Public/CompanionDialogue.h`, `Server/Private/CompanionDialogue.cpp` | 신규 | scripted line 선택, LLM client(WinSock non-blocking, request id, timeout), 결과를 `ROOM_COMMAND_TYPE::COMPANION_DIALOGUE_RESULT`로 enqueue |
| `Server/Public/RoomCommand.h` | 수정 | `COMPANION_DIALOGUE_RESULT` |
| `Server/Private/GameRoom.cpp` | 수정 | `Handle_Chat`에 command parser(`chatCommands`), companion `SERVER_PLAYER` 생성(`iSessionId=INVALID`, `bIsCompanion`), roster/ROOM_FULL/카드/기믹 대상 제외, tick에서 `CCompanionSystem::Update` |
| `Server/Private/ServerApp.cpp` | 수정 | `--companion-llm` 인자로 `llm.enabled` override(기본 데이터 값) |
| `Client/Private/ClientReplication.cpp` | 수정 | `isCompanion` player를 같은 `CCharacter` 경로로 spawn, nameplate에 `(AI)` |
| `Client/Private/WorldPlayerChatBubbleView.cpp` | 수정 없음 | companion도 player view이므로 그대로 동작. 말풍선 art는 `UI/Chat/SpeechBubble.png` 자산이 오면 교체(§5) |
| `Server/Private/ServerGameplayContractTests.cpp`, `NetworkProtocolHarness` | 수정 | 아래 계약 |

#### H 계약 (러프)

```cpp
// Server/Public/CompanionSystem.h
enum class SERVER_COMPANION_MODE : std::uint8_t { INACTIVE, ASSIST, GUIDE, END };
struct SERVER_COMPANION_PROFILE final
{
	std::string strCompanionId;
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::uint32_t iAssistDurationTicks = 0u;
	float fAssistEngageRangeM = 0.f;
	float fAssistPreferredDistanceM = 0.f;
	std::uint32_t iDecisionIntervalTicks = 0u;
	std::uint32_t iDodgeLookaheadTicks = 0u;
	std::vector<LostArk::Shared::SKILL_ID> SkillRotation;
	float fGuideFollowDistanceM = 0.f;
	std::uint32_t iGuideRepositionIntervalTicks = 0u;
	std::vector<std::string> AssistCommands;
	std::vector<std::string> GuideCommands;
	std::vector<std::string> DismissCommands;
	bool bLlmEnabled = false;
	std::string strLlmEndpoint;
	std::uint32_t iLlmTimeoutMs = 0u;
};
struct SERVER_COMPANION_STATE final
{
	LostArk::Shared::PLAYER_ID iOwnerPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
	LostArk::Shared::PLAYER_ID iCompanionPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
	SERVER_COMPANION_MODE eMode = SERVER_COMPANION_MODE::INACTIVE;
	std::uint32_t iModeEndTick = 0u;
	std::uint32_t iNextDecisionTick = 0u;
	std::size_t iRotationCursor = 0u;
	std::string strLastLineId;
	std::uint32_t iLastLineTick = 0u;
	std::uint32_t iPendingDialogueRequestId = 0u;
};
class CCompanionSystem final
{
public:
	bool Initialize(const SERVER_COMPANION_PROFILE& profile, std::string& outStatus);
	bool Try_ParseChatCommand(std::string_view text, SERVER_COMPANION_MODE& outRequestedMode) const;
	bool Request_Assist(LostArk::Shared::PLAYER_ID ownerPlayerId, std::uint32_t serverTick, std::string& outStatus);
	bool Request_Guide(LostArk::Shared::PLAYER_ID ownerPlayerId, std::uint32_t serverTick, std::string& outStatus);
	bool Dismiss(LostArk::Shared::PLAYER_ID ownerPlayerId, std::string& outStatus);
	void Reset();
	[[nodiscard]] bool Is_Companion(LostArk::Shared::PLAYER_ID playerId) const;
	[[nodiscard]] const std::vector<SERVER_COMPANION_STATE>& Get_States() const { return m_States; }
private:
	SERVER_COMPANION_PROFILE m_Profile;
	std::vector<SERVER_COMPANION_STATE> m_States;
};
/* 결정만 한다. 이동·스킬 적용은 GameRoom의 기존 Handle_Move/Handle_UseSkill 내부 함수를 통해 실행한다. */
struct COMPANION_DECISION final
{
	bool bHasMoveGoal = false;
	float fMoveGoalX = 0.f;
	float fMoveGoalZ = 0.f;
	bool bHasSkill = false;
	LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
	float fAimX = 0.f;
	float fAimZ = 0.f;
};
class CCompanionBrain final
{
public:
	static COMPANION_DECISION Decide(
		const SERVER_COMPANION_PROFILE& profile,
		const SERVER_COMPANION_STATE& state,
		const SERVER_PLAYER& companion,
		const SERVER_PLAYER& owner,
		const SERVER_WORLD_ENTITY* boss,
		const BOSS_PATTERN_STAGE_DEFINITION* activeStage,
		const CServerNavigation& navigation,
		std::uint32_t serverTick);
};
```

```cpp
// Server/Public/CompanionDialogue.h
enum class COMPANION_DIALOGUE_TRIGGER : std::uint8_t
{
	PATTERN_STAGE_ENTER, PATTERN_OUTCOME, ASSIST_START, ASSIST_END, PLAYER_QUESTION, END
};
struct COMPANION_DIALOGUE_LINE final
{
	std::string strLineId;
	COMPANION_DIALOGUE_TRIGGER eTrigger = COMPANION_DIALOGUE_TRIGGER::END;
	std::string strPatternId;
	std::string strStageId;
	std::string strText;
	std::uint32_t iDurationMs = 0u;
	std::uint32_t iPriority = 0u;
};
struct COMPANION_DIALOGUE_RESULT final
{
	std::uint32_t iRequestId = 0u;
	LostArk::Shared::PLAYER_ID iCompanionPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
	bool bFromLlm = false;
	std::string strText;
};
class CCompanionDialogue final
{
public:
	bool Load(const std::filesystem::path& path, std::string_view expectedEncounterId, std::string& outStatus);
	const COMPANION_DIALOGUE_LINE* Find_ScriptedLine(COMPANION_DIALOGUE_TRIGGER trigger, std::string_view patternId, std::string_view stageId) const;
	bool Begin_LlmRequest(std::uint32_t requestId, LostArk::Shared::PLAYER_ID companionPlayerId, std::string_view context, std::string_view question, std::string& outStatus);
	void Poll(std::vector<COMPANION_DIALOGUE_RESULT>& outResults, std::uint32_t nowMs);
};
```

#### 호출 흐름

```text
C2S_CHAT "도움!" → Handle_Chat → relay + CCompanionSystem::Try_ParseChatCommand → Request_Assist
→ companion SERVER_PLAYER 생성(없으면) → S2C_PLAYER_SPAWNED(isCompanion) → Client CCharacter(GUNSLINGER) 표현
room tick → CCompanionBrain::Decide (ASSIST: 보스 6 m 유지, activeStage hit shape 예측 회피, SkillRotation 쿨다운 순환 / GUIDE: owner 2.5 m 뒤 추종)
→ 기존 내부 Handle_Move/Handle_UseSkill → CPlayerSkillSystem → snapshot
boss stage ENTER edge → CCompanionDialogue::Find_ScriptedLine → S2C_CHAT(from companion NetEntityId) → Client 말풍선 + 채팅 로그
C2S_CHAT 일반 문장(owner) + llm.enabled → Begin_LlmRequest → sidecar → Poll → COMPANION_DIALOGUE_RESULT room command → S2C_CHAT
timeout/실패 → Find_ScriptedLine(PLAYER_QUESTION) fallback
```

#### 검증

Server contract: (a) `도움!` 뒤 companion spawn, 20초 뒤 GUIDE 전환, (b) roster/ROOM_FULL 미포함, (c) 카드 분배·bind target 제외, (d) stage ENTER line 1회 방송,
(e) sidecar 부재 시 fallback line, (f) `/그만` dismiss. `python -m unittest Tools.CompanionChat.test_companion_dialogue_contract`, NetworkProtocol round-trip.
사용자 smoke: 쿠크에서 `도움!` → 전투 → 추종 → 패턴 말풍선.

---

### G07 — 2·3관문 배분과 빙고 UI (범위만 고정)

| 관문 | 원작 | profile / action (인벤토리 §3) | 필요 계약 |
|---|---|---|---|
| 2관문 125 세이튼 등장 | 대형 세이튼 등장 | `MN_RPCT_06` + `WP_MN_RPCT_06` `SAME_CLIP_NAME` | BossCatalog v7 weaponClipSync, `ENCOUNTER_KAKUL_GATE2` |
| 2관문 110 파1·빨2 | 카드 무늬 색 | `4219810~4219840` 검은/붉은 스페이드·클로버·하트·다이아 | G03-1 suit + color 확장 `KAKUL_CARD_COLOR` |
| 2관문 95 조커 찾기 | 진짜/가짜 | `4221807 시작`, `4221816 성공`, 카드뒤집기 `4221820~4221839` | summon set + facing/입력 판정 재사용 |
| 2관문 85 카드 미로 | 이동 판정 | `4219899/42198100/42198101` | Server trigger cell 판정(collisionBox 동적 활성) — dynamic collision 계약 별도 |
| 2관문 55 피자 | 장판 | `4219714` 재사용 | G02 combat object 피자 |
| 3관문 마리오 1~4 / 쇼타임 | 플랫폼 이동 + 컷신 | `world.sequence.instance.boss_showtime`, `4219905 빙고맵 오프닝`, `4219984 빙고 폭탄` | Sequencer v1 + boss combat snapshot `bingoBoard`(25 bit) + `Data/UI/KakulSaydon/Bingo_Layout.json` |

빙고 UI는 `CCombatHUDViewModel::HUD_BOSS_STATE.iBingoBoardMask`를 읽는 image widget이며 UI가 빙고를 판정하지 않는다. 별도 구현 계획서로 다룬다.

---

## 5. 사용자 결정 필요 항목

| # | 항목 | 권고 |
|---|---|---|
| 1 | 2관문 무대 세이튼 본체 `MN_RPCT_00`(기본) vs `MN_RPCT_05`(쿠크 파츠 포함) | 05 (3관문과 리소스 공유) |
| 2 | 카드 분배 `RANDOM` vs `ORDERED` | 데이터 `encounterRules.cardSuit.assignment`로 둘 다 지원, 기본 RANDOM |
| 3 | 도움 지속 20초 vs 30초 | `assist.durationMs` 20000 기본, Balance Tool에서 조정 |
| 4 | LLM 사용 여부와 provider(ChatGPT/OpenAI-compatible) | 기본 scripted, sidecar는 opt-in. Server 안에 HTTP client를 넣지 않는다 |
| 5 | 50줄 룰렛 결과가 유발하는 행동 | 결정 전 최소 규칙(결과 무늬 3초 속박) |
| 6 | 삐에로(플레이어 변신) 모델 리소스 존재 여부 | 팀장 Drive 확인. 없으면 G03-2는 게이지·스킬까지 |
| 7 | 카드/하트/총/스포트 관련 Effect·UI 자산(`UI/KakulSaydon/*`, 카드 Effect) | 추출 목록을 별도 인테이크 문서로 |
| 8 | 관문별 `maximumHealthBars` 초기값 150/150/100 | Balance Tool `PROJECT_TUNED` |
| 9 | Engine `LIGHT_DESC` 확장(spot) 승인 — 공개 헤더·size static_assert 변경 | 승인 권고, Product build로 닫음 |
| 10 | Debug 기본 HOLD를 Valtan에도 적용 | 적용 권고(Auto Playback 토글로 Release 검증 가능) |
| 11 | 1관문 댄스타임을 세이튼 분신(05)으로 구현 | 인벤토리상 쿠크(RPCZ_00)에 댄스 clip이 없으므로 분신 구현 권고 |
| 12 | 관문 = encounter 3개 + world 1개 | 권고안 그대로 |

결정 전까지는 해당 G를 시작하지 않는다. 계획서 안에 표식으로 미루지 않기 위해 각 항목은 위 권고 값으로 진행 가능하게 적었다.

---

## 6. 하지 않는 것

- Client에서 phase/HP/판정/빙고를 계산하는 어떤 경로도 만들지 않는다.
- `CValtanBrain`·`CValtan`·`CValtanPatternTree`의 리네임. 이름은 유지하고 encounter 인자로 일반화한다(리네임은 별도 합의).
- 두 번째 pattern parser, 두 번째 Server audition service, 두 번째 JSON writer, 두 번째 boss presentation 경로.
- PBR/CSM/TAA/SSR/ACES 구현. 이 계획은 그 위에서 튜닝할 데이터·도구·연출 계약까지다.
- ImGui 위젯을 제품 UI로 승격. 카드 아이콘·광기 게이지·빙고는 `Data/UI` layout JSON + `CUIObject` 경로다.
- Server 안에 HTTP client·API key. LLM은 sidecar와 localhost 소켓뿐이다.
- Sequencer v1 전체(09-03 PLAN G00~G05). 이 계획은 SCENE_PROFILE track 하나와 관문 클리어 sequence 하나만 연결한다.
- 에이전트의 Client 자율 실행·화면 캡처·visual PASS 기록.

---

## 7. 검증 명령 총괄

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
powershell -ExecutionPolicy Bypass -File Tools/KakulSaydonPipeline/Project-KakulPatternMaster.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/CompanionChat/Validate-CompanionDialogue.ps1
python -m unittest Tools.CompanionChat.test_companion_dialogue_contract
python -m unittest Tools.ValtanPipeline.test_valtan_boss_tool_pattern_flow_contract
python -m unittest Tools.ValtanPipeline.test_action_composition_workbench_regression_oracles
powershell -ExecutionPolicy Bypass -File Tools/Build/Test-CompiledShaderClosure.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
git diff --check
```

각 G의 RESULT는 구현 완료 / 자동 검증 / 사용자 수동 검증 / 남은 경계를 분리해 `.md/GB/<MM-DD>/`에 남긴다. Python contract PASS만으로 native admission을
결론내리지 않고, 변경 domain은 compiled harness와 `Server.exe --contract-test`로 다시 연다(`gotchas.md` consumer closure matrix).

---

## 8. 실행 순서

```text
G00 Server 재생 정책 (Valtan에서 즉시 검증 가능)          → RESULT
G01 Boss Tool 재편 (Valtan)                              → RESULT
G02 쿠크 첫 수직 슬라이스 (입장 + 피자 1패턴)            → RESULT, 이후 모든 Kakul G의 전제
G05-1/2/3 조명 기반 (map light, spot, profile blend)      → G03-4가 소비
G03-1 → G03-3 → G03-4 → G03-5 → G03-6 → G03-2 → G03-7      → 각 RESULT
G04 Logic Graph 편집기                                   → Valtan/Kakul 공용
G05-4 pattern light cue                                  → G03-4 하트핑 연출 완성
G06 건슬 AI                                              → 1관문 line 작성
G07 별도 계획서
```

G02와 G05-1은 서로 독립이라 병렬 가능하다. G03-2(광기·삐에로)는 자산 결정(§5-6)에 걸려 있어 뒤로 두었다.
