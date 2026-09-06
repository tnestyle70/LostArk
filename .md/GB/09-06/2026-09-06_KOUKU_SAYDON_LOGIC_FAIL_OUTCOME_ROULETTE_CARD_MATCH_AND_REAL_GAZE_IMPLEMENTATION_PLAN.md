# 2026-09-06 KoukuSaydon Logic FAIL outcome 분리, 룰렛 카드 판정, 진짜 세이튼 응시 판정 구현 계획서

> 문서 종류: 구현 계획서 (Logic 판정 모델 확장 + Server 판정 런타임 2종)
>
> 상태: 실측 완료 / 구현 전 / §8 미결 3건은 G01 착수 전에 사용자 결정
>
> 기준 브랜치: `codex/kouku-scale-1p7` (composition revision 50, protocol 59)
>
> 선행 문서: [Lane/Box Workbench 계획](../09-05/2026-09-05_KOUKU_SAYDON_LANE_BOX_COMPOSITION_WORKBENCH_IMPLEMENTATION_PLAN.md), [무력화 반사 계획](../09-05/2026-09-05_KOUKU_SAYDON_SHIELD_STAGGER_AND_COMMON_GROGGY_PATTERN_IMPLEMENTATION_PLAN.md), [관문 수정·광기 수치·광대 아바타 계획](2026-09-06_KOUKU_SAYDON_GATE_FIX_HAMMER_MADNESS_CLOWN_AVATAR_IMPLEMENTATION_PLAN.md). 그 문서의 G05 “Logic 런타임(후속)” 중 진짜 찾기·RESULT 적용 부분은 이 문서가 소유한다.
>
> 원작 근거: [패턴 애니메이션 조사서](../09-05/2026-09-05_KOUKU_SAYDON_GATE1_3_PATTERN_ANIMATION_SURVEY.md) `진짜를 찾아봐_하트셋트/총잡이셋트` 절, 인벤·나무위키 1관문 공략(§1.3).

## 0. 목표와 종료 증거

1. **DURATION box가 Success·Fail·Timeout 세 결과를 따로 연결한다.** 조건 불일치(Fail)와 창 만료(Timeout)는 다른 RESULT를 가리킬 수 있고, 판정 시점이 창 종료 한 번인 kind는 Timeout 연결을 거부한다.
2. **룰렛 카드 판정이 Server에서 플레이어마다 실행된다.** 창이 열리면 Server가 플레이어별 카드 문양을 배정해 snapshot으로 보내고, 창이 끝나는 tick에 플레이어가 선 부채꼴의 문양과 비교한다. 일치는 Success(연결 없으면 아무 일 없음), 불일치는 Fail RESULT(광기 게이지 +50%)다.
3. **진짜 세이튼 응시 판정이 Server에서 플레이어마다 실행된다.** 창이 끝나는 tick에 플레이어 몸 방향 기준 반각 안에 패턴 소유 보스가 있으면 Success, 없으면 Fail RESULT(즉사)다.
4. **RESULT가 typed 값으로 적용된다.** `INSTANT_DEATH`, `MAX_HP_PERCENT_DAMAGE`, `MADNESS_GAUGE_ADD_PERCENT` 세 kind를 Server가 기존 `Apply_WorldToPlayer`와 `iCurrentMadness`로 적용한다.
5. **HUD 미준비를 격리한다.** 카드 문양은 snapshot과 F1 텍스트까지만 연결하고 머리 위 이미지는 UI 슬라이스에 남긴다. Client는 판정을 만들지 않는다.
6. **룰렛 연출이 패턴 시계에 붙는다.** 패턴의 WORLD box 시작 tick에 Server가 기존 `S2C_WORLD_SEQUENCE_PLAY`로 룰렛 instance를 재생시키고, 룰렛은 시퀀스대로 나타나 돌고 멈추고 사라진다. 판정 창의 종료 시각은 projector가 같은 시퀀스의 정지 구간에서 yaw를 읽어 Product에 굽는다. 저작자는 yaw를 손으로 옮겨 적지 않는다.

종료 증거: Debug Product 빌드 PASS, `NetworkProtocolHarness` failures 0(v60 snapshot round-trip), `Server.exe --contract-test` failures 0(룰렛 일치/불일치, 응시 성공/실패, abort 시 결과 미적용, WORLD box tick에 재생 broadcast), python 테스트 OK(`test_project_kouku_saydon_composition.py`, `test_kouku_saydon_runtime_inputs.py`), publisher 재실행 뒤 bootstrap에 `PATTERNLOGIC`/`PATTERNLOGICOUTCOME`/`PATTERNWORLDSEQUENCE` 행, `git diff --check` 0. 사용자 runtime 확인은 §7.

## 1. 현재 실측

### 1.1 Logic 저작·투영 현재

| 항목 | 현재 값 | 위치 |
|---|---|---|
| Logic 정의 | `{logicId, displayName, logicType}`만. 판정 값 없음 | `KoukuSaydonCompositionDocument.h:49~57` |
| Logic box | `{occurrenceId, logicId, startMs, durationMs, onSuccessLogicId, onTimeoutLogicId}` | `KoukuSaydonCompositionDocument.h:61~73`, reader `.cpp:1008~1035`, writer `.cpp:1310~1320` |
| Box Detail Outcomes | `Success`, `Timeout` 두 combo. RESULT 정의만 나열 | `KoukuSaydonActionWorkbench.cpp:4397~4440`, `Set_LogicBoxOutcome(patternId, occurrenceId, bool success, resultLogicId)` `.cpp:3871` |
| Selected Logic 패널 | 이름·type·참조 수·Append만. 값 편집 없음 | `Render_LogicResources` |
| projector | `LOGIC_KEYS`(113), `LOGIC_OCCURRENCE_OPTIONAL_KEYS = {onSuccessLogicId, onTimeoutLogicId}`(122). **PRODUCT 패턴에 logic box가 있으면 거부**(391~395). outcome은 RESULT만, DURATION box만 허용(399~412). Product stage는 animation-only 고정(`_project_stage` 684) |
| publisher | Kouku 블록(3030~3275)이 `PATTERNBOSS/PATTERNPOLICY/PATTERNSOURCE/PATTERNSTAGE/PATTERNSTAGEBRANCH(TIMEOUT→다음)`만 emit. stage는 animation-only 검사 |
| Server 정의 | `BOSS_PATTERN_DEFINITION`(`GameplayCatalog.h:798~`)에 Logic 개념 없음. `Validate_AnimationOnlyPattern`은 stage TIMEOUT branch 하나만 통과 |
| Server 실행 | `Update_KoukuSaydonBoss`(`GameRoom.cpp:5604~5760`) → `CKoukuSaydonBrain::Update`가 stage duration만 본다. 플레이어 접근은 GameRoom의 `m_Players`(`GameRoom.h:1194`)만 가능 |
| 저작 문서 | revision 50. Logic 정의 9개(DURATION `방패무력화`, RESULT `전원전멸/즉사/최대체력20%/10%/50%감소/무력화성공/삐에로변신`, TRIGGER `순간이동`). `PATTERN_1`에 DURATION box 1개(Success→무력화성공, Timeout→전원전멸), `PATTERN_2`에 TRIGGER 1988ms·Summon 2007~26133ms, `PIZZA`(PRODUCT, 6 stage 16334ms)와 `PATTERN_6`(댄스타임 21 stage)에는 box 없음 |

### 1.2 Server에 이미 있는 재료

| 계약 | 위치 | 재사용 |
|---|---|---|
| 플레이어 위치·yaw | `SERVER_PLAYER::fPositionX/Z, fYawDegrees`(`ServerPlayer.h:133~137`). yaw는 이동·스킬 조준(`PlayerSkillSystem.cpp:471`)으로 갱신, forward = `(sin yaw, cos yaw)` | 응시 판정의 A 벡터 |
| HP·광기 | `iCurrentHp/iMaximumHp`(175~176), `iCurrentMadness/iMaximumMadness/eMadnessForm`(192~194, protocol 59) | RESULT 적용 대상. **iMaximumMadness는 아직 0** |
| 플레이어 피해 | `SERVER_WORLD_TO_PLAYER_HIT{iRawDamage, fSourceX/Z, push, knockdown, iServerTick}` → `Apply_WorldToPlayer`가 `Apply_Defense` 뒤 HP 차감·DEAD 전이·damage event | INSTANT_DEATH·MAX_HP 피해의 단일 소유자 |
| 각도 규약 | volley slot `x = sin(deg)·r, z = cos(deg)·r`(`GameRoom.cpp:11253~11261`), 0° = +Z, 시계 방향 증가 | 부채꼴 index 계산 |
| 결정론 난수 | `Mix_DeterministicRandom(identity ^ Hash_StableId(...))`(Valtan 유령) | 카드 배정 |
| audition 상태 | `KOUKUSAYDON_PATTERN_AUDITION_STATE`(`GameRoom.h:425~441`) | 창 ledger의 owner |
| snapshot | `PLAYER_SNAPSHOT`(`PacketMessages.h:777~779`), `NETWORK_PROTOCOL_VERSION = 59`(`PacketType.h:39`) | 카드 문양 필드 추가 지점 |

### 1.3 원작 조건 (문헌·추출물)

| 기믹 | 원작 | 이 계획의 판정 |
|---|---|---|
| 룰렛(48줄) | 3회 회전, 회전마다 머리 위 문양이 바뀜, 자기 문양 칸에 서야 함, 실패 시 피해. 문양은 하트·스페이드·클로버·다이아 | 창(회전 1회)마다 카드 재배정, 창 종료 tick에 칸 문양 비교. 벌칙은 저작 RESULT(광기 +50%) |
| 세이튼을 찾아라(110줄) | 분신 4명 중 다른 동작을 하는 진짜를 **캐릭터 몸 방향으로 바라봐야** 생존. 평타로 방향 갱신. 하트/총은 식별 표식일 뿐 | 창 종료 tick에 `플레이어 forward` vs `플레이어→보스` 각도차 ≤ 반각. 보스 forward는 판정에 안 들어감 |
| 진짜 action | `4219832/4219842 …_진짜 공격`에만 `att_battle_20_04`·`SendNpcSignal`·`ViewShake`. 가짜에는 없음 | 창 종료 = 진짜의 `20_04` 시작에 맞춰 저작 |

### 1.4 재사용 좌표·수치

- 룰렛 바닥 배치 40번: `MAP_ROULETTE_BG_RAD_KOUKUSATON_FLOORROULETTE01_SM`, 위치 `(-0.319, 1.9, 737.531)`, 회전 identity, scale 4, 기본 `visible 0` (`LV_LUT_MIDNIGHTC_ED.mapplacements:2977`).
- 1관문 세이튼 placement `boss.kakulsaydon.g1.saydon` `(-0.07, 1.32, 942.33)`, yaw 245.
- Server는 map placement를 읽지 않는다. 룰렛 중심·반경은 volley anchor처럼 **저작 값의 복사**로 Logic 정의에 둔다.

### 1.5 룰렛 연출 재료 (이미 있는 것)

| 계약 | 위치 | 내용 |
|---|---|---|
| world sequence 문서 | `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` (formatVersion 2, template 55·instance 59) → `Publish-MapAuthoring.ps1`이 `Client/Bin/DataFiles/Map/<Area>.worldsequences.json`으로 publish | Client만 읽는 presentation 정본 |
| 룰렛 template | `sequence.LV_LUT_MIDNIGHTC_ED.5`(“roulette”), 22200ms, LINEAR, track `object` 139 key(`timeMs, positionOffset, rotationQuaternion, scaleMultiplier, visible`) | 첫 key `visible true`(생성), 마지막 key 22200 `visible false`(소멸) |
| 룰렛 instance | `world.sequence.instance.8` → template `.5`, binding `object → MAP_PLACEMENT 40`, `startDelayMs 0`, `playbackSpeed 1` | Play 대상 stable ID |
| Server → Client 재생 명령 | `S2C_WORLD_SEQUENCE_PLAY{strSequenceInstanceId}`(`PacketMessages.h:2021`), `CGameRoom::Broadcast_WorldSequencePlay(instanceId)`(`GameRoom.cpp:4469`), trigger action이 호출(`GameRoom.cpp:1917`) | 이미 lever→bridge, 컷신, 마리오에 사용 |
| Client 소비 | `CClientReplication::Consume_WorldSequencePlays()` → `CLevel_KakulSaydonArena` 711행 → `m_SequencePlayer.Play(instanceId, targets)` 758행. `CWorldSequencePlayer`가 baseline 대비 회전·visible을 매 frame 합성 | 새 Client 코드 없이 재생 가능 |

template의 정지(hold) 구간 (quaternion → Y축 yaw):

| 구간(ms, sequence-local) | yaw | 의미 |
|---|---|---|
| 3500~5500 | 0 | 1회전 정지 2초 → **판정 1** (5500) |
| 6400~7400 | 180 | 반 바퀴 뒤 1초 정지 |
| 10900~12900 | 180 | 2회전 정지 2초 → **판정 2** (12900) |
| 13800~14800 | 0 | 반 바퀴 뒤 1초 정지 |
| 18300~20300 | 0 | 3회전 정지 2초 → **판정 3** (20300) |
| 21200~22200 | 180, visible false | 소멸 |

yaw는 placement 40의 identity 회전에 합성되므로 그대로 세계 yaw다. 판정은 2초 정지 구간의 끝에 두고, 1초 정지는 연출이다.

## 2. 요구사항 ↔ 반영

| # | 요구 | 반영 | G |
|---|---|---|---|
| R1 | Fail과 Timeout을 다른 결과로 연결 | box `onFailLogicId` 추가, Box Detail `Fail` row, projector/publisher/Server slot 3개 | G01 |
| R2 | 룰렛: 카드 vs 부채꼴 문양, 일치 → 없음, 불일치 → 광기 +50% | DURATION kind `ROULETTE_CARD_MATCH`, 창 열림에 카드 배정(snapshot), 창 종료 판정, RESULT `MADNESS_GAUGE_ADD_PERCENT 50` | G01, G02, G04 |
| R3 | 진짜 찾기: 창 종료 시점 시야각 안에 진짜가 없으면 죽음 | DURATION kind `GAZE_REAL_BOSS{halfAngleDegrees}`, RESULT `INSTANT_DEATH` | G01, G02, G03 |
| R4 | 판정은 플레이어마다 | 창 종료 tick에 살아 있는 플레이어 전원을 개별 판정, RESULT도 개별 적용 | G02 |
| R5 | HUD·회색광대 미준비 | 카드 문양은 snapshot + F1 텍스트. 머리 위 이미지·회색광대 body는 범위 밖 | G04 |
| R6 | 패턴 시작에 룰렛이 생기고 돌다 멈추는 연출과 판정을 한 시계로 | pattern WORLD box(`worldSequenceOccurrences`) → Server가 시작 tick에 `Broadcast_WorldSequencePlay`, projector가 판정 창 종료 시각의 시퀀스 yaw를 `stopYawDegrees`로 파생 | G01, G02, G05 |

## 3. 판정 모델

Logic 정의는 palette(kind와 kind 공통 값), box는 배치(창·결과 연결·배치별 값)를 소유한다. 이 계획의 Server 소비자는 두 kind다.

| kind | 판정 시점 | 판정 대상 | 사용 slot | 정의 값 | box 값 |
|---|---|---|---|---|---|
| `ROULETTE_CARD_MATCH` | 창 종료 tick 1회 | 플레이어 개별 | Success, Fail (**Timeout 금지**) | `sectorCount`, `sectorSymbols[]`(룰렛 mesh yaw 0°에서 +Z부터 시계 방향, 길이 = sectorCount, 토큰 HEART/SPADE/CLUB/DIAMOND), `centerX`, `centerZ`, `outerRadiusM`, `worldSequenceInstanceId`(룰렛 instance) | 없음. `stopYawDegrees`는 projector가 같은 패턴의 WORLD box와 시퀀스 template에서 파생한다(§5.6) |
| `GAZE_REAL_BOSS` | 창 종료 tick 1회 | 플레이어 개별 | Success, Fail (**Timeout 금지**) | `halfAngleDegrees`(1~180) | 없음 |
| `STAGGER_WINDOW` (무력화 계획 소유) | 창 중 연속 | 보스 | Success, Timeout | 그 계획의 값 | 없음 |
| `POSE_INPUT` (댄스타임, 후속) | 창 중 연속 | 플레이어 개별 | Success, Fail, Timeout | 후속 | 후속 |

Timeout은 “창이 끝났는데 조건이 아직 충족되지 않았다”는 연속 판정 전용 결과다. 창 종료에 한 번 판정하는 kind는 그 순간 Success 아니면 Fail로 반드시 결정되므로 Timeout이 존재하지 않는다. projector와 Box Detail이 이 규칙을 강제한다.

RESULT 정의 typed 값:

| outcomeKind | 값 | 적용 |
|---|---|---|
| `INSTANT_DEATH` | 없음 | `Apply_WorldToPlayer(raw = iCurrentHp, bIgnoreDefense = true)` |
| `MAX_HP_PERCENT_DAMAGE` | `percent` 1~100 | `raw = round(iMaximumHp × percent / 100)`, `bIgnoreDefense = true` |
| `MADNESS_GAUGE_ADD_PERCENT` | `percent` 1~100 | `iCurrentMadness = min(iMaximumMadness, iCurrentMadness + round(iMaximumMadness × percent / 100))` |

기존 RESULT 이름과의 대응: `즉사` → INSTANT_DEATH, `최대체력10%감소` → MAX_HP_PERCENT_DAMAGE 10(20/50도 같은 방식), 룰렛 실패용 `광기게이지50%증가`는 새 RESULT 정의로 만든다. `전원전멸`, `무력화성공`, `삐에로변신`은 이 계획에서 typed 값을 갖지 않으며 연결되면 projector가 “미지원 RESULT kind”로 거부한다.

## 4. 변경할 파일

새 C++ 파일 2개(`Server/Public/KoukuSaydonLogicRuntime.h`, `Server/Private/KoukuSaydonLogicRuntime.cpp`)를 `Server.vcxproj`(`KoukuSaydonBrain.h/.cpp` 항목 44·76행 옆)와 `.filters`(56·102행 옆, Filter `Public`/`Private`)에 등록한다. Client 새 파일 없음.

| G | 파일 | 역할 |
|---|---|---|
| G01 | `Client/Public/KoukuSaydonCompositionDocument.h`, `Client/Private/KoukuSaydonCompositionDocument.cpp` | 정의 `strJudgementKind`+값, RESULT `strOutcomeKind/iPercent`, box `strOnFailLogicId`; pattern `WorldSequenceOccurrences{occurrenceId, sequenceInstanceId, startMs}`+`iNextWorldSequenceOccurrenceOrdinal`; reader optional, writer 항상 기록; Validate |
| G01 | `Client/Public/KoukuSaydonActionWorkbench.h`, `Client/Private/KoukuSaydonActionWorkbench.cpp` | `Set_LogicBoxOutcome` slot enum, Box Detail `Fail` row·Timeout 비활성, Selected Logic kind/값 편집, `Count_LogicReferences`에 onFail; WORLD lane(`Append_WorldSequenceBox`, 시작 이동, 삭제); `Set_WorldSequenceResources(vector<WORLD_SEQUENCE_RESOURCE>)`와 ROULETTE 정의의 `Fill from placement` 버튼 |
| G01 | `Client/Public/Level_KakulSaydonArena.h`, `Client/Private/Level_KakulSaydonArena.cpp`, `Client/Private/MainApp.cpp` | Level이 로드한 `CWorldSequencePlayer::Get_Document()` instance 목록과 binding된 `MAP_RUNTIME_PLACED_ENTRY`(위치·scale·모델 local bounds)를 읽기 전용 `WORLD_SEQUENCE_RESOURCE{instanceId, displayName, durationMs, placementId, x, z, scale, boundsRadiusM}`로 만들어 `Set_ModelResources`처럼 Workbench에 전달 |
| G01 | `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`, `test_project_kouku_saydon_composition.py` | key 집합, kind/값 검증, slot 규칙, PRODUCT 허용(소비 가능한 kind만), Product `logicWindows`·`worldSequences` 투영, worldsequences.json 읽어 `stopYawDegrees` 파생·hold 검증 |
| G01 | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`, `Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py` | Kouku Product pattern key `logicWindows`·`worldSequences`, `PATTERNLOGIC`/`PATTERNLOGICOUTCOME`/`PATTERNWORLDSEQUENCE` 행 |
| G01 | `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp` | enum 3개, `BOSS_PATTERN_LOGIC_WINDOW`, `BOSS_PATTERN_WORLD_SEQUENCE{strInstanceId, iStartMs}`, `BOSS_PATTERN_DEFINITION::LogicWindows/WorldSequences`, 행 parse |
| G01 | `Server/Private/KoukuSaydonBrain.cpp` | `Validate_AnimationOnlyPattern`이 LogicWindows/WorldSequences 허용(창 ⊂ 패턴 길이, 같은 kind 겹침 금지, WORLD 시작 ≤ 패턴 길이) |
| G02 | `Server/Public|Private/KoukuSaydonLogicRuntime.*` (새 파일) | 창 ledger, open/evaluate, RESULT 적용, WORLD box 시작 tick에 `Broadcast_WorldSequencePlay` 요청 |
| G02 | `Server/Public/ServerCombatHitRuntime.h`, `Server/Private/ServerCombatHitRuntime.cpp` | `SERVER_WORLD_TO_PLAYER_HIT::bIgnoreDefense` |
| G02 | `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp` | `KOUKUSAYDON_PATTERN_AUDITION_STATE::LogicWindows` ledger, `Update_KoukuSaydonBoss` 호출 지점, abort/clear 정리, 광기 최대치(§8) |
| G02 | `Server/Private/ServerGameplayContractTests.cpp` | 룰렛·응시·abort 계약 테스트 |
| G03 | (G01·G02로 완결) `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | `PATTERN_2`에 `GAZE_REAL_BOSS` box·`즉사` Fail 연결은 Workbench Save로 저작 |
| G05 | (G01·G02로 완결) `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | `PIZZA`에 WORLD box(`world.sequence.instance.8`, 0ms)와 ROULETTE 창 3개(종료 5500/12900/20300) 저작, 패턴 길이 22200 이상으로 조정 |
| G05 (선택) | `Client/Private/Level_KakulSaydonArena.cpp` 또는 `MainApp.cpp` Debug | 룰렛 부채꼴 경계·문양 Debug wire(같은 §5.4 수식). 시각 확인용, 판정 아님 |
| G04 | `Shared/Public/Network/PacketType.h`, `PacketMessages.h`, `Shared/Private/Network/PacketMessages.cpp`, `Tools/NetworkProtocolHarness/...` | v60, `MECHANIC_CARD_SYMBOL`, `PLAYER_SNAPSHOT::eMechanicCardSymbol`, writer/reader/validator, round-trip |
| G04 | `Server/Public/ServerPlayer.h`, `Server/Private/GameRoom.cpp` | `eMechanicCardSymbol` 필드, snapshot 기록, 리셋 3곳 |
| G04 | `Client/Public|Private/CombatHUDViewModel.*`, `Client/Private/MainApp.cpp` | `HUD_PLAYER_STATE::eMechanicCardSymbol`, F1 `KoukuSaydon Arena` `Card: HEART` 텍스트 |

## 5. 데이터와 호출 흐름

### 5.1 저작 JSON (v2 유지, optional key 추가)

```json
"logics": [
  { "logicId": "kakulsaydon.g1.logic.10", "displayName": "세이튼로직_룰렛카드", "logicType": "DURATION",
    "judgementKind": "ROULETTE_CARD_MATCH",
    "sectorCount": 6, "sectorSymbols": ["HEART","SPADE","CLUB","DIAMOND","HEART","SPADE"],
    "centerX": -0.319, "centerZ": 737.531, "outerRadiusM": 12.0,
    "worldSequenceInstanceId": "world.sequence.instance.8" },
  { "logicId": "kakulsaydon.g1.logic.11", "displayName": "세이튼로직_진짜응시", "logicType": "DURATION",
    "judgementKind": "GAZE_REAL_BOSS", "halfAngleDegrees": 45 },
  { "logicId": "kakulsaydon.g1.logic.12", "displayName": "세이튼로직_광기게이지50%증가", "logicType": "RESULT",
    "outcomeKind": "MADNESS_GAUGE_ADD_PERCENT", "percent": 50 },
  { "logicId": "kakulsaydon.g1.logic.3", "displayName": "세이튼로직_즉사", "logicType": "RESULT",
    "outcomeKind": "INSTANT_DEATH" }
],
"worldSequenceOccurrences": [
  { "occurrenceId": "KAKULSAYDON_G1_PIZZA.world.1", "sequenceInstanceId": "world.sequence.instance.8", "startMs": 0 }
],
"logicOccurrences": [
  { "occurrenceId": "KAKULSAYDON_G1_PIZZA.logic.1", "logicId": "kakulsaydon.g1.logic.10",
    "startMs": 0, "durationMs": 5500,
    "onSuccessLogicId": "", "onFailLogicId": "kakulsaydon.g1.logic.12", "onTimeoutLogicId": "" },
  { "occurrenceId": "KAKULSAYDON_G1_PIZZA.logic.2", "logicId": "kakulsaydon.g1.logic.10",
    "startMs": 5500, "durationMs": 7400,
    "onSuccessLogicId": "", "onFailLogicId": "kakulsaydon.g1.logic.12", "onTimeoutLogicId": "" },
  { "occurrenceId": "KAKULSAYDON_G1_PIZZA.logic.3", "logicId": "kakulsaydon.g1.logic.10",
    "startMs": 12900, "durationMs": 7400,
    "onSuccessLogicId": "", "onFailLogicId": "kakulsaydon.g1.logic.12", "onTimeoutLogicId": "" },
  { "occurrenceId": "KAKULSAYDON_G1_PATTERN_2.logic.2", "logicId": "kakulsaydon.g1.logic.11",
    "startMs": 2007, "durationMs": 21960,
    "onSuccessLogicId": "", "onFailLogicId": "kakulsaydon.g1.logic.3", "onTimeoutLogicId": "" }
]
```

- 정의의 kind별 값 key는 그 kind에서만 허용된다. `judgementKind`가 없는 DURATION은 “값 없는 창”으로 읽히며 Workbench 편집은 되지만 PRODUCT 투영은 거부된다(기존 `방패무력화` 정의 보존).
- reader는 새 key를 optional로 읽고 writer는 정의 type·kind가 요구하는 key만 쓴다. `onFailLogicId`는 항상 쓴다.
- WORLD box는 `sequenceInstanceId`와 `startMs`만 소유한다. 시퀀스 길이·회전·visible은 world sequence 문서의 것이고 composition은 복제하지 않는다. 한 패턴에 같은 instance의 WORLD box는 하나다.
- ROULETTE 창의 종료 시각(`startMs + durationMs`)은 WORLD box 시작 기준 시퀀스 시간으로 환산해 정지 구간 안에 있어야 한다. 위 예는 5500/12900/20300으로 §1.5의 판정 1~3에 맞춘 값이다. 창의 시작은 카드가 배정·표시되는 시각이므로 앞 판정 직후로 두어 “회전마다 문양이 바뀐다”를 재현한다.

### 5.2 Product·publisher 행

projector는 PRODUCT 패턴의 box를 `worldSequences[]`와 `logicWindows[]`로 투영한다(stage는 그대로 animation-only). ROULETTE 창의 `stopYawDegrees`는 저작 값이 아니라 **파생 값**이다.

```json
"worldSequences": [
  { "sequenceInstanceId": "world.sequence.instance.8", "startMs": 0 }
],
"logicWindows": [
  { "windowId": "KAKULSAYDON_G1_PIZZA.logic.1", "kind": "ROULETTE_CARD_MATCH",
    "startMs": 0, "durationMs": 5500,
    "sectorCount": 6, "sectorSymbols": ["HEART","SPADE","CLUB","DIAMOND","HEART","SPADE"],
    "centerX": -0.319, "centerZ": 737.531, "outerRadiusM": 12.0, "stopYawDegrees": 0.0,
    "halfAngleDegrees": 0,
    "onSuccess": { "kind": "NONE", "percent": 0 },
    "onFail":    { "kind": "MADNESS_GAUGE_ADD_PERCENT", "percent": 50 },
    "onTimeout": { "kind": "NONE", "percent": 0 } }
]
```

publisher(`Publish-GameplayBalance.ps1` Kouku 블록)는 pattern `Assert-ExactProperties`에 `worldSequences`, `logicWindows`를 더하고 다음 행을 emit한다. 값 열은 kind가 쓰지 않으면 `0`/`-`다.

```text
PATTERNWORLDSEQUENCE\t<encounterId>\t<patternId>\t<startMs>\t<sequenceInstanceId>
PATTERNLOGIC\t<encounterId>\t<patternId>\t<windowIndex>\t<windowId>\t<kind>\t<startMs>\t<durationMs>\t<sectorCount>\t<centerX>\t<centerZ>\t<outerRadiusM>\t<stopYawDegrees>\t<halfAngleDegrees>\t<HEART,SPADE,...|->
PATTERNLOGICOUTCOME\t<encounterId>\t<patternId>\t<windowId>\t<SUCCESS|FAIL|TIMEOUT>\t<outcomeKind>\t<percent>
```

검증: `startMs + durationMs ≤ Σ stage durationMs`, 같은 kind 창 겹침 금지, `sectorSymbols` 길이 = `sectorCount`(2~64), 토큰 4종, `outerRadiusM > 0`, `halfAngleDegrees` 1~180, AT_END kind의 TIMEOUT 행 금지, outcomeKind별 percent 범위, WORLD `sequenceInstanceId`가 composition `areaId`의 worldsequences 문서에 존재하고 `MAP_PLACEMENT` binding을 가질 것. Server `GameplayCatalog.cpp`는 `PATTERNSTAGEBRANCH` parse(3585~)와 같은 방식으로 owner pattern을 찾아 `LogicWindows`/`WorldSequences`에 push하고, `PATTERNLOGICOUTCOME`은 같은 windowId의 slot을 채운다(중복 slot 거부). Server는 instance ID를 문자열로만 다루며 시퀀스 내용을 알지 않는다.

### 5.3 Server 실행 흐름 (G02)

```text
Update_KoukuSaydonBoss (GameRoom.cpp:5604)
→ PENDING → Begin_Pattern 성공 시
   ledger = CKoukuSaydonLogicRuntime::Build(pattern->LogicWindows, serverTick)   // startTick/endTick = 시작 tick + ceil(ms·30/1000)
→ ACTIVE tick마다, m_KoukuSaydonBrain.Update 호출 **전에**
   CKoukuSaydonLogicRuntime::Update(boss, *pattern, ledger, m_Players, m_GameplayCatalog, serverTick, m_PendingDamageEvents, outSequencePlays, status)
     ├─ 시작되지 않은 WORLD box의 startTick 도달 → outSequencePlays.push_back(instanceId) → GameRoom이 Broadcast_WorldSequencePlay
     ├─ 열리지 않은 창의 startTick 도달 → Open
     │    ROULETTE: 살아 있는 플레이어마다 eMechanicCardSymbol = 문양집합[Mix_DeterministicRandom(seed ^ playerId) % 집합크기]
     │    GAZE: 없음
     └─ 열린 창의 endTick 도달 → Evaluate → 플레이어마다 Success/Fail → slot RESULT 적용 → 카드 초기화 → closed
→ Brain.Update가 PATTERN_COMPLETED / ABORTED → ledger 파기 (ABORTED는 미평가 창의 결과를 적용하지 않음)
→ Clear_KoukuSaydonPatternAudition, 사망, 퇴장 → 카드 초기화
```

- Brain 앞에서 평가하는 이유: 창 종료가 패턴 종료 tick과 같을 때 완료 처리보다 판정이 먼저 일어나야 한다.
- 판정 대상은 `iCurrentHp > 0`, `isCombatReady`, `eAction`이 DEAD/FALLING/GRABBED가 아닌 플레이어다. 나머지는 판정하지 않고 카드만 지운다.
- RESULT는 `Apply_WorldToPlayer`(피해 2종)와 광기 가산 한 함수에서만 적용한다. `bIgnoreDefense`가 true면 `Apply_Defense`를 건너뛴다. `Try_Counter`가 ABSORBED를 돌려주면 그 플레이어는 벌칙을 피한 것으로 두고 status에 남긴다(원작 카운터 무효 여부는 §8).
- `m_KoukuSaydonBrain.Update`는 바꾸지 않는다. 판정은 boss stage와 무관한 pattern-clock 창이므로 stage/branch에 투영하지 않는다.

### 5.4 판정 수식

공통: `deg(dx, dz) = atan2(dx, dz) · 180/π`, `wrap180(a) ∈ (−180, 180]`, `wrap360(a) ∈ [0, 360)`. 정규화·내적 대신 각도차를 쓴다. Server가 yaw를 도(degree)로 갖고 있어 NaN 없이 부채꼴 index까지 같은 값으로 얻는다.

**ROULETTE_CARD_MATCH (플레이어 p, 창 w)**

```text
dx = p.x − w.centerX, dz = p.z − w.centerZ
dist² > outerRadiusM²           → FAIL (룰렛 밖)
rel = wrap360(deg(dx, dz) − w.stopYawDegrees)
sector = floor(rel / (360 / sectorCount))   // 0 ≤ sector < sectorCount
sectorSymbols[sector] == p.card → SUCCESS, else FAIL
```

**GAZE_REAL_BOSS (플레이어 p, 패턴 소유 보스 b)**

```text
dx = b.x − p.x, dz = b.z − p.z
dist² < 0.01                    → SUCCESS (밀착)
diff = wrap180(deg(dx, dz) − p.fYawDegrees)
|diff| ≤ halfAngleDegrees       → SUCCESS, else FAIL
```

보스의 forward는 어느 식에도 들어가지 않는다. 플레이어가 보스 정면·뒤 어디에 서 있든 자기 몸이 보스를 향하면 성공이다. 진짜/가짜 분신 런타임이 들어온 뒤에도 `b`는 패턴을 소유한 본체이므로 이 식은 그대로다.

### 5.5 Client 흐름

```text
PLAYER_SNAPSHOT.eMechanicCardSymbol → CClientReplication → CCombatHUDViewModel::HUD_PLAYER_STATE.eMechanicCardSymbol
→ CMainApp F1 KoukuSaydon Arena: "Card: HEART" (NONE이면 숨김)
판정 결과 → 기존 DAMAGE_EVENT / HP / iCurrentMadness snapshot으로 관찰
```

Workbench Box Detail: `Success`, `Fail`, `Timeout` 세 combo. 정의 kind가 AT_END면 Timeout combo를 disabled로 그리고 tooltip “창 종료 판정 kind는 Timeout이 없습니다”. ROULETTE box Detail은 같은 패턴의 WORLD box 시작을 기준으로 환산한 시퀀스 시각과, 그 시각이 정지 구간이면 yaw를, 아니면 “정지 구간 밖” 경고를 **읽기 전용**으로 보여준다(`WORLD_SEQUENCE_RESOURCE`가 나르는 template을 `CWorldSequencePlayer::Sample_Track`으로 sampling). Selected Logic 패널은 DURATION이면 kind combo와 kind 값, RESULT면 outcomeKind combo와 percent를 편집하고 `Set_LogicDefinitionValues`로 candidate → `Commit_Candidate`한다. 실패는 draft를 유지한다.

Workbench는 world sequence 문서를 직접 열지 않는다. 지금 `Set_ModelResources`/`Set_SequenceResources`(animation clip 시퀀스)가 오는 것과 같은 방식으로 `CMainApp`이 아레나 Level의 `CWorldSequencePlayer::Get_Document()` instance 목록과 binding placement를 `WORLD_SEQUENCE_RESOURCE`로 넘긴다. WORLD lane의 instance 콤보는 이 목록을 쓰고, ROULETTE 정의의 `Fill from placement`는 선택 instance가 binding한 배치(40번)의 위치를 `centerX/centerZ`에, 모델 local bounds의 XZ 반경 × scale을 `outerRadiusM`에 복사한다. 칸 수와 문양 순서는 mesh 텍스처에 있어 코드가 읽지 못하므로 사용자가 적는다(현재 mesh 기준 6칸).

### 5.6 룰렛 연출·판정 결합

소유권을 세 층으로 나눈다. 시퀀스 내용은 world sequence 문서가, 재생 시각은 Server 패턴 시계가, 판정 yaw는 projector가 소유한다.

```text
저작 (Workbench)
  PIZZA에 WORLD box { instance.8, startMs 0 } 하나, ROULETTE 창 3개(종료 5500 / 12900 / 20300)
  Box Detail이 창 종료의 시퀀스 시각과 정지 yaw를 읽기 전용으로 표시 → 저작자는 정지 구간 안에 종료를 놓는다

투영 (projector)
  worldsequences.json(areaId)에서 instance.8 → template .5 → track "object"
  창마다 local = (logic.start + logic.duration) − world.startMs
  Sample_Track과 같은 규칙(LINEAR, 앞뒤 key 보간)으로 quaternion → yaw
  local−300ms .. local 구간의 yaw 변화가 0.01° 이하가 아니면 CompositionError("roulette window does not end on a stop")
  local이 template durationMs를 넘거나 그 key의 visible이 false면 거부
  Product logicWindows[].stopYawDegrees = yaw

실행 (Server)
  패턴 시작 tick T0. WORLD box startTick = T0 + ceil(0·30/1000) → Broadcast_WorldSequencePlay("world.sequence.instance.8")
  창 종료 tick = T0 + ceil(end·30/1000) → §5.4 ROULETTE 수식으로 판정

표현 (Client)
  Consume_WorldSequencePlays → m_SequencePlayer.Play(instance.8): 첫 key에서 배치 40이 보이고, 22200ms 마지막 key에서 다시 숨는다
  시퀀스 시작 지연은 네트워크 왕복 수십 ms이고 판정은 2초 정지 구간 끝에 있으므로 칸 index가 바뀌지 않는다
```

- `PIZZA`의 stage 합계는 지금 16334ms라 22200ms 시퀀스보다 짧다. 세 번째 판정(20300)을 두려면 패턴 길이를 22200 이상으로 늘려야 하며(stage clip 반복 또는 `HOLD_LAST_POSE`), projector가 `창 종료 ≤ 패턴 길이`로 강제한다. 패턴이 끝나도 Client 시퀀스는 자기 길이대로 끝난다.
- 룰렛 mesh의 문양 배열은 mesh yaw 0에서 +Z 칸을 `sectorSymbols[0]`로 두고 시계 방향으로 적는다. 배치 40의 회전이 identity라 mesh yaw = 세계 yaw다. 실제 mesh의 칸 수·문양 순서는 MapTool에서 눈으로 확인해 적는다(§8-1).
- 같은 패턴을 다시 Play하면 Server가 다시 broadcast하고 `CWorldSequencePlayer::Play`가 baseline에 대해 rewind한다. abort 시 시퀀스는 Client에서 끝까지 재생되며 Server는 판정만 버린다.

## 6. G별 구현 범위

### G01 — 판정 모델 계약 (Client 문서·Workbench → projector → publisher → Server 정의)
- 문서 struct·reader·writer·Validate: 정의 kind/값, RESULT kind/percent, box `onFail`, pattern `WorldSequenceOccurrences`. 삭제·참조 카운트에 `onFailLogicId` 포함.
- Workbench: `Set_LogicBoxOutcome`의 `bool success`를 `KOUKU_LOGIC_OUTCOME_SLOT{SUCCESS, FAIL, TIMEOUT}`로 교체, 세 row, Timeout 비활성 규칙, `Set_LogicDefinitionValues`; WORLD lane 한 줄(`Append_WorldSequenceBox`, `Set_WorldSequenceBoxStart`, `Delete_WorldSequenceBox`)과 ROULETTE Box Detail의 읽기 전용 시퀀스 시각/yaw 표시.
- projector: key 집합·kind·값·slot 규칙, PRODUCT 허용 조건(“모든 box의 kind가 Server 소비 가능”), `worldSequences`·`logicWindows` 투영, `stopYawDegrees` 파생·정지 검증(§5.6), 테스트 추가(Fail 연결, AT_END Timeout 거부, 값 범위, 겹침, PRODUCT 허용/거부, 정지 구간 밖 종료 거부, 미존재 instance 거부).
- publisher: `worldSequences`·`logicWindows` exact key, 세 행 emit, `test_kouku_saydon_runtime_inputs.py`에 행 기대값.
- Server: enum·struct·parse, `Validate_AnimationOnlyPattern` 허용 조건, `Find_AnimationOnlyPattern` 변경 없음.
- 종료: python 테스트 OK, publisher 실행 뒤 bootstrap grep에 세 행, Server 계약 테스트의 catalog parse PASS, Workbench에서 세 row 연결·WORLD box 저장·재로드 일치.

### G02 — Server 판정 런타임
- 새 파일 `CKoukuSaydonLogicRuntime`(static 함수 셋): `Build`, `Update`, `Discard`. 상태는 `KOUKUSAYDON_PATTERN_AUDITION_STATE::LogicWindows`(`{windowIndex, startTick, endTick, opened, closed}`)와 `WorldSequences`(`{index, startTick, started}`)에 둔다. WORLD 시작 tick에 instance ID를 돌려주고 GameRoom이 기존 `Broadcast_WorldSequencePlay`를 호출한다.
- `SERVER_WORLD_TO_PLAYER_HIT::bIgnoreDefense` 추가와 `Apply_WorldToPlayer` 분기 한 줄.
- 광기 최대치: 룰렛 RESULT가 동작하려면 `iMaximumMadness > 0`이어야 한다. §8 결정에 따라 `KAKULSAYDON_ARENA` 진입·부활 시 임시 상수 100을 채우거나, `madnessPolicy.maximum` publish를 선행한다. 최대치가 0이면 RESULT는 status만 남기고 값을 만들지 않는다.
- 계약 테스트(`ServerGameplayContractTests.cpp` Kouku 블록 옆): (a) 룰렛 창 — 플레이어 A는 자기 문양 칸, B는 다른 칸, C는 룰렛 밖 → A 광기 불변, B·C `+50%`; (b) 응시 창 — yaw가 보스를 향한 A 생존, 반대 B `iCurrentHp == 0 && DEAD`, 밀착 C 생존; (c) 창 종료 전 abort → 아무 값도 변하지 않음; (d) 패턴 종료 tick과 창 종료 tick이 같을 때 판정이 먼저 적용됨; (e) WORLD box 시작 tick에 outbound queue에 `S2C_WORLD_SEQUENCE_PLAY(instance.8)`가 한 번만 실린다.
- 종료: `Server.exe --contract-test` failures 0.

### G03 — 진짜 세이튼 응시 저작
- Workbench에서 `PATTERN_2`에 `GAZE_REAL_BOSS` box를 Summon 창과 같은 구간(2007ms~)에 놓고 Fail → `즉사`, Success 비연결로 Save. `authoringStatus`를 PRODUCT로 올려 projector가 통과하는지 확인한다(현재 `PATTERN_2`는 DRAFT).
- 분신 spawn·본체 relocation은 별도 Summon 런타임 슬라이스다. 그 전에는 본체 하나를 향하는지로 같은 판정을 검증한다.
- 종료: publish → Server 재시작 → 사용자 runtime 확인(§7).

### G04 — 카드 문양 복제 (protocol 60)
- Shared: `enum class MECHANIC_CARD_SYMBOL : uint8 { NONE, HEART, SPADE, CLUB, DIAMOND, END }`, `PLAYER_SNAPSHOT::eMechanicCardSymbol`(`eMadnessForm` 바로 아래), writer/reader, `Is_Valid_PlayerSnapshot`에 `< END`, `NETWORK_PROTOCOL_VERSION = 60`, 하네스 round-trip과 `END` 거부.
- Server: `SERVER_PLAYER::eMechanicCardSymbol`(`eMadnessForm` 아래), snapshot 기록, 리셋 3곳(spawn·class 변경·부활)과 audition clear에서 NONE.
- Client: ViewModel 필드 복사, F1 텍스트. HUD 이미지는 UI 슬라이스.
- 종료: `NetworkProtocolHarness` failures 0, Server+Client 같은 revision 재시작.

### G05 — 룰렛 저작과 시각 확인
- Workbench에서 `PIZZA`에 WORLD box(`world.sequence.instance.8`, 0ms)와 `세이튼로직_룰렛카드` 창 3개(종료 5500/12900/20300, Fail → `광기게이지50%증가`)를 놓고 패턴 길이를 22200 이상으로 맞춘 뒤 Save·publish. Box Detail의 읽기 전용 yaw가 0/180/0으로 읽히는지 확인한다.
- 선택: 아레나 Debug wire로 `centerX/centerZ`, `outerRadiusM`, `sectorCount`, 현재 창의 `stopYawDegrees`에서 부채꼴 경계와 문양 문자를 그린다. 시각 확인용이며 판정에는 관여하지 않는다. 룰렛 mesh 칸과 어긋나면 `sectorSymbols` 순서 또는 `sectorCount`를 고친다.
- 종료: publish → Server 재시작 → 사용자 runtime 확인(§7).

## 7. 검증 요약

| 단계 | 명령/방법 | 판정 |
|---|---|---|
| python | `PYTHONPATH=. python -m unittest Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition Tools.KoukuSaydonPipeline.test_kouku_saydon_runtime_inputs` (repo root, UTF-8) | OK |
| publish | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` → bootstrap에 `PATTERNLOGIC`/`PATTERNLOGICOUTCOME` grep | 행 존재 |
| build | `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug` | PASS |
| Shared | `NetworkProtocolHarness` | failures 0 |
| Server | `Server.exe --contract-test` | failures 0 (§6 G02 a~d) |
| diff | `git diff --check` | 0 |
| runtime (사용자) | Server+Client 재시작 → F1 `1관문 - 세이튼` → `PIZZA` Play: 패턴 시작과 함께 룰렛이 나타나 3.5초 돌고 멈추는 것을 확인, 정지마다 F1 `Card:` 문양 확인, 정지 끝에 다른 칸에 선 플레이어의 광기 값 상승·같은 칸은 불변, 22.2초에 룰렛이 사라짐 → `PATTERN_2` Play: 보스를 등진 플레이어만 창 종료 tick에 사망 | 눈으로 확인, 서면 판정 |

에이전트는 빌드·하네스·계약 테스트·publish까지 수행하고 runtime 재생과 화면 판정은 사용자가 한다.

## 8. 고정된 결정과 미결

고정:
- Fail과 Timeout은 다른 slot이다. 창 종료 1회 판정 kind(`ROULETTE_CARD_MATCH`, `GAZE_REAL_BOSS`)는 Timeout이 없고, 연속 판정 kind만 Timeout을 갖는다.
- 응시 판정의 벡터 쌍은 `플레이어 forward` vs `플레이어→보스`다. 보스 forward, 카메라, 대칭 벡터는 쓰지 않는다. 판정은 창 종료 tick 1회이며 누적 시간을 두지 않는다(Server yaw는 이동·조준으로만 바뀐다).
- 룰렛 판정은 플레이어 위치 기준 부채꼴 index다. Server는 map placement와 시퀀스를 읽지 않는다. 중심·반경은 저작 값, 정지 yaw는 projector가 world sequence 문서에서 파생한 값이다.
- 룰렛 연출은 새 런타임이 아니라 기존 `S2C_WORLD_SEQUENCE_PLAY` → `CWorldSequencePlayer` 경로를 패턴 시계에서 호출하는 것이다. 회전을 Server가 복제하지 않는다.
- RESULT 적용은 `Apply_WorldToPlayer`(피해)와 광기 가산 한 지점만 사용한다. Client는 판정하지 않는다.
- 판정 창은 stage/branch에 투영하지 않는다(pattern-clock 독립 창). 무력화 창은 기존 계획대로 stage/branch를 쓴다.

미결(G01 착수 전 결정):
1. **룰렛 문양 배열.** 칸 수는 사용자가 실제 mesh를 보고 **6칸**으로 결정했다(2026-09-06). 6칸에 문양 4종을 어떻게 배열할지(예: 4종 + 2종 반복, 또는 3종 × 2)와 `sectorSymbols[0]`(mesh yaw 0에서 +Z 방향 칸)이 어느 문양인지는 MapTool에서 배치 40번을 `visible`로 켜고 확인해 적는다. 중심·반경은 `Fill from placement`가 채운다.
6. **룰렛은 Summon이 아니다.** Summon은 Server world entity(archetype·HP·brain·`S2C_WORLD_ENTITY_SPAWNED`)를 만드는 lane이고, 룰렛은 이미 맵에 있는 배치 40번을 world sequence로 보이고 돌리는 Client presentation이다. 따라서 WORLD lane box(`worldSequenceOccurrences`)로 재생 시각만 패턴 시계에 묶고, Server는 `Broadcast_WorldSequencePlay`만 한다. 가짜 세이튼 3체만 Summon lane이다.
2. **광기 최대치.** `iMaximumMadness`가 0이라 룰렛 RESULT가 지금은 값을 만들 수 없다. (a) 이 계획 G02에서 아레나 진입 시 임시 100 고정, (b) 09-05 광기 계획의 `madnessPolicy.maximum` publish를 선행. 권장 (a).
3. **카운터 스킬로 벌칙을 막을 수 있는가.** 현재 `Apply_WorldToPlayer`는 `Try_Counter` ABSORBED를 먼저 본다. INSTANT_DEATH가 카운터에 막히는 것이 의도가 아니면 `bIgnoreDefense`와 함께 `bIgnoreCounter`를 둔다. 권장: 막히지 않게(`bIgnoreCounter = true`).
4. 진짜 찾기 반각: 09-06 계획이 45°로 고정했다. 저작 값이므로 60°로 완화해도 코드 변경은 없다.
5. **판정 3회 중 어느 정지에 두는가.** 시퀀스는 2초 정지 3회와 1초 정지 3회를 가진다. 기본은 2초 정지의 끝(5500/12900/20300) 3회이며, 1초 정지도 판정으로 쓰려면 창을 더 놓으면 된다(코드 변경 없음).
