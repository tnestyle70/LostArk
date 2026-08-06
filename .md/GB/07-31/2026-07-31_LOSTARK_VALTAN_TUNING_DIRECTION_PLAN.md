# LostArk 발탄 실제 패턴·파괴 지형·공중 연출 통합 방향 계획서

- 최초 작성일: 2026-07-31
- 재검토일: 2026-08-05
- 문서 유형: 구현 전 상위 방향 문서
- 목표 프로필: `NORMAL_8P_2022` Gate 2
- 현재 브랜치: `codex/effect-tool-reboot`
- 구현 상태: 조사와 방향 확정. 이 문서의 G는 아직 구현 완료가 아니다.
- 상세 코드 원칙: 각 G 착수 시 현재 H/CPP 전문을 다시 실측하고 별도 G 계획에 전체 반영 코드를 싣는다.

이 문서는 다음 기존 문서의 현재 정본이다.

- `2026-07-31_LOSTARK_VALTAN_BOSS_PATTERN_IMPLEMENTATION_PLAN.md`의 Client-local 패턴안은 제품 경로에서 폐기한다.
- `2026-08-03_G9_VALTAN_ENCOUNTER_VERTICAL_SLICE_PLAN.md`의 제안보다 현재 실제 `CValtanBrain`/snapshot 구조를 우선한다.
- `2026-08-05_MAP_GAMEPLAY_TRIGGER_DESTROYABLE_NAV_EXTENSION_PLAN.md`의 trigger/destroyable 수직 슬라이스는 유지하되, 발탄 HP 기믹은 player trigger box가 아니라 Server encounter event로 발화한다.

## 1. 결론

어제 확인한 “LPK에서 가져온 발탄과 실제 레이드 발탄이 다르다”는 판단이 맞다. 더 정확히는
LPK 자산 자체가 틀린 것이 아니라, **일부 액션 파일과 일부 animation만 추출한 뒤 원작 의미와 다른
단일 패턴으로 연결한 현재 구현이 불완전하다.**

현재 제품 경로는 아래 한 패턴뿐이다.

```text
nearest alive player
-> chase
-> 8m 이내 PATTERN_WINDUP
-> 800ms 뒤 반경 8m 전원에게 350% 1회 피해
-> 300ms active
-> 1200ms recovery
```

이것은 원작 발탄의 패턴 선택, 방향 고정, cone/ring/line 판정, 카운터, 잡기, 약점 파괴,
무력화, 지형 파괴, 공중 이탈, 유령 전환을 재현하지 않는다. `phaseTwoHpPercent=50`도 snapshot의
phase byte만 2로 바꾸며 패턴 pool이나 지형 상태를 바꾸지 않는다.

첫 구현은 현재 확보된 18개 공격 clip으로 원작 액션명이 확인되는 다섯 패턴부터 닫고, 이후
누락 animation/effect를 추출해 scripted mechanic과 파괴 지형을 연결한다. 수치부터 임의로 늘리지 않는다.

## 2. C1~C8 관점

| 관점 | 이번 작업에서 고정할 내용 | 중요도 |
|---|---|---:|
| C1 기준계 | LPK notify는 clip-local time, Server는 30Hz tick, 영상은 frame time, 맵은 Engine world unit으로 명시 변환한다. | ★★★ |
| C2 이동>계산 | Action/Matinee/TriggerMap 추출과 JSON 검증은 offline에서 하고 runtime은 cook된 tick·shape·binding만 읽는다. | ★★★ |
| C3 공유는 비싸다 | Shared에는 semantic pattern/stage/world state만 싣고 clip·particle·camera asset ID는 Client에 둔다. | ★★★ |
| C4 수명은 선언된다 | Server room이 encounter state, Client Level이 arena presentation, `CValtan`이 boss visual state를 소유한다. | ★★★ |
| C5 이산화와 오차 | HP bar 참고값과 실제 trigger ratio, 영상 frame과 Server tick 반올림 정책을 분리한다. | ★★★ |
| C6 가지치기 | targetable phase, range, cooldown, repeat limit, scripted preemption으로 후보를 먼저 제거한 뒤 결정적 선택을 한다. | ★★★ |
| C7 권위와 정합성 | HP/phase/pattern/hit/knockback/world mutation은 Server, animation/effect/camera는 Client presentation이다. | ★★★ |
| C8 검증이 병목 | 두 Client tick 정합, late join, 파괴 rollback, 누락 presentation 격리, 실제 영상 비교를 각 G 종료 증거로 둔다. | ★★★ |

## 3. 문제 해결 ①~⑤

① 문제·제약: 현재 `VALTAN_BASIC_SWING`은 원작 `att_battle_2_01~03`의 의미와 다르고 발탄의 scripted mechanic과 arena state가 없다.

② 단순 해법의 문제: Client animation 종료나 ImGui 버튼으로 벽·phase를 바꾸면 두 Client와 Server navigation이 갈라진다.

③ 해결 방식: LPK action은 presentation/reference 근거로 추출하고, 검증된 encounter JSON을 `CValtanBrain`이 30Hz로 실행해 semantic action과 world event를 snapshot으로 보낸다.

④ 비교: 기존 단일 radial hit를 범용 Boss Manager로 감싸지 않는다. 발탄 하나의 실제 소비자인 `CValtanBrain`과 world state endpoint를 수직으로 확장한다.

⑤ 대가: action extractor, 누락 asset cook, encounter schema v2, protocol, world event binding, Client presentation 문서가 필요하다. 대신 phase·벽·공중 연출을 같은 Server truth에서 재현한다.

## 4. 2026-08-05 실측 결과

### 4.1 LPK에서 실제로 확인한 것

| 원본 | 크기 | SHA-256 | 판정 |
|---|---:|---|---|
| `MN_RPBF_00.loa` | 7,450,184 | `F61DF383BF20634CCDC5B0DB3EB9DDE1BC62C78717CA763A325E56285E370797` | 원작 발탄 공용/레이드 action graph의 핵심 |
| `MN_RPBF_01-1.loa` | 366,062 | `6FEA6FB228D95A019FA9C6D42E2A298CA334BE20ED186A84C98027015846FC2A` | `MN_RPBF_01` 변형 action 43400~43405 |
| `MN_RPBF_02-2.loa` | 645,481 | `60B6B1B17633E76A7A02A3C2514F2E54EEBDBC7625354DA697D46B924488EA4D` | 카제로스/협동 카운터 계열이 섞인 별도 변형. 2022 Gate 2 ghost 정본으로 간주하지 않음 |

`MN_RPBF_01-1.loa`의 실제 전투 action은 다음 여섯 개다.

| action ID | 원본 이름 | 현재 확보 clip |
|---:|---|---|
| 43400 | 레이드 발탄_지진 찍기 | Battle 7 |
| 43401 | 레이드 발탄_휠윈드 | Battle 20 |
| 43402 | 레이드 발탄_앞뒤앞 내려찍기 | Battle 19 + Battle 2 |
| 43403 | 레이드 발탄_대쉬 돌진 | Battle 4 |
| 43404 | 레이드 발탄_내려찍기 | Battle 2 |
| 43405 | 레이드 발탄_댄스 | 전투 대상 아님 |

따라서 현재 `BossCatalog.json`의 아래 연결은 clip 존재 여부만 맞고 semantic 의미는 틀리다.

```text
patternWindup   att_battle_2_01
patternActive   att_battle_2_02
patternRecovery att_battle_2_03
```

이 세 clip은 `VALTAN_BASIC_SWING`의 windup/active/recovery가 아니라 원작 action
`43404 / 420602 / 420661 내려찍기`의 ordered clip이다.

### 4.2 지금까지 보지 못했던 핵심 action

`MN_RPBF_00.loa`는 109개 `CEFActionObject`를 가지며, 그중 action ID `420600~420678`
범위에 원작 레이드 발탄 action 76개가 있다. 사용자 요구와 직접 연결되는 항목은 다음과 같다.

| action ID | 원본 이름 | 주요 clip/effect 근거 | 현재 상태 |
|---:|---|---|---|
| 420610 | 레이드 발탄_고공 점프 찍기 | `Att_Battle_8_01_Start/Loop/End` | clip 미확보 |
| 420621 / 420663 | 점프 찍기 후 휠윈드 / Normal | Battle 20 전체 | clip 확보 |
| 420622 | 워프 돌진 콤보 | Battle 18, portal/dash particle | clip·particle 미확보 |
| 420624 / 420665 | 하얗게 불사르고 망령화 / Normal | Battle 18→1→19, fake-dead/2nd-phase BGM, portal/body effect | Battle 19만 확보 |
| 420629 | 모든 지형물 파괴 연출 | Battle 12_01~03, `Par_O_RPBF_Atk_07_01/02` | clip·particle 미확보 |
| 420654 | 1페이즈 돌진 외벽 파괴 그로기 | `Par_D_RPBF_PartsDestruction_01` | effect 미확보 |
| 420658 | 마지막 지면파괴 사자후 콤보 | Battle 16, 20, Event Battle 5, 11 | 대부분 미확보 |
| 420637 / 420666 | 앞뒤앞 내려찍기 / Normal | Battle 19 + Battle 2 | clip 확보 |
| 420638 | 두 손 내려찍어 지면 폭발 | Battle 19_02/04 + Battle 1 | 일부 확보 |
| 420640~420646 | 연속 카운터 내려찍기 | Battle 14 | clip 미확보 |

레이드 action이 참조하는 고유 공격 clip token은 92개다. 현재
`Client/Bin/Resources/Character/Valtan/anims`에는 그중 18개만 있고 74개가 없다.
현재 확보 묶음은 Battle `2, 4, 7, 19, 20`뿐이다. Battle `1, 5, 8~18, 21`의 필요한 부분은
추가 원본 animation package 추적·cook이 선행되어야 한다.

현재 `Data/Effects`와 runtime Effect pack에는 `FX_MN_RPBF_00_*`가 없다. action 문자열에서
particle 이름을 보았다는 사실만으로 Effect Tool asset이 준비됐다고 처리하지 않는다.

### 4.3 현재 코드와 데이터의 정확한 공백

| 계층 | 현재 상태 | 필요한 변화 |
|---|---|---|
| `ValtanEncounter.json` | state 7개, pattern 1개 | health pool, scripted mechanic, pattern pool, multi-stage hit shape |
| `CValtanBrain` | nearest target + chase + radial hit 1회 | HP crossing preemption, pattern selection, stage timeline, counter/grab/charge/world event |
| Shared snapshot | action/actionId/startTick/HP/phase | pattern instance, stage, target/facing, arena state revision |
| `CValtan` | generic 3 clip, 전부 loop 재생 | actionId별 ordered one-shot, startTick seek, 누락 presentation 격리 |
| World v2 | Client parse/save만 구현 | publisher/Server/destroyable/nav/replication 제품 admission |
| `.navblockers` | region count 0 | 실제 붕괴 구역 region과 condition 작성 |
| Valtan Level | static map + replication만 로드 | deploy runtime, encounter presentation, arena state consumer |
| sky phase | MapTool 수동 radio만 존재 | Server semantic state에 따른 제품 runtime fade/visibility |
| Balance Tool | pattern 숫자 편집은 가능 | phase/pool/stage/shape/world event 편집·진단 |

## 5. 실제 발탄 Gate 2 기준선

공식 문서는 8인·2관문·Normal 1415/Hard 1445와 에스더 역할은 확정하지만 세부 HP line을
공식 수치로 제공하지 않는다. 아래 HP line은 2022 Normal 8-player 공략과 영상을 교차한
`REFERENCE_ONLY`이며, JSON에는 `referenceBarApprox`와 프로젝트 `triggerHpRatio`를 분리한다.

| 구간 | 실제 관찰 기준 | 런타임 분류 |
|---|---|---|
| 시작~x130 | 방어 2중첩, target charge를 외벽에 유도, 충돌 후 약점 파괴 window | collision/reactive mechanic |
| x130 | 2회 지면 강타 전멸기 | HP scripted once |
| 약 x115~105 | 공중 이탈·복귀, 외곽 벽 정리, 4기둥+표적 cone | HP scripted once |
| x88 전후 | 한쪽 arena 붕괴, 착지·회전, 기둥·cone | HP scripted once + arena mutation |
| x64 전후 | 중앙 이동, 개인 표적 폭발, charge grab/counter | HP scripted once |
| x34 전후 | 반대쪽 arena 붕괴, 작은 platform만 유지 | HP scripted once + arena mutation |
| x15~16 | 공중/portal 진입, 4방향 강타·추적 폭발·안/밖·기둥 후 ghost | HP scripted once + health-pool transition |
| Ghost x40~0 | 중앙 고정, clone counter로 armor 제거, grab, edge clone rush | 별도 health pool + ghost pattern pool |

일반 pattern pool은 최소 다음 묶음으로 분리한다.

- 기본: 내려찍기, 지진 찍기, 앞뒤앞, 휠윈드, 점프 후 휠윈드, 휘두르기.
- 이동: 대쉬 돌진, 고공 점프, 워프 돌진, anchor rain.
- 반응형: 큰 베기 parry, counter, charge wall collision, grab.
- scripted: x130 wipe, 모든 지형물 파괴, x88/x34 붕괴, x15 ghost transition.

Inferno의 연속 카운터는 고정 HP가 아니라 약 x90~x30에서 보이거나 고 DPS에서 생략될 수 있다는
공식 안내가 있으므로 `scriptedMechanics`에 넣지 않고 조건부 pattern pool에 둔다. Solo/Extreme/
카제로스 변형 action은 `NORMAL_8P_2022` 프로필에 섞지 않는다.

## 6. 정본과 소유권

```text
원본 LPK/DB/영상
  -> Data/Animation/Reference/Valtan/*        읽기 전용 근거
  -> Data/Balance/Reference/Official/*       field provenance

Data/Balance/BossProfiles.json               기본 HP/AP/이동/충돌
Data/Balance/DamageProfiles.json             피해 rate
Data/Encounters/Valtan/ValtanEncounter.json  Server phase/pattern/stage/shape/worldEventId
Data/Animation/Authored/Valtan/
  Valtan.patternbindings.json                Client actionId -> ordered clip/effect cue
Data/Worlds/LV_LUT_HEARTRB_ED/
  Gameplay.world.json                        stable destroyable와 worldEvent binding
Data/Navigation/LV_LUT_HEARTRB_ED.navblockers
                                               arena condition -> blocked cells
Data/Maps/Authoring/LV_LUT_HEARTRB_ED/
  EncounterPresentation.json                 worldStateId -> sky/camera/fade presentation
```

Server는 `att_battle_*`, `FX_MN_*`, camera 이름을 읽지 않는다. Client는 HP threshold, hit shape,
damage, phase 완료를 판정하지 않는다. `worldEventId`는 두 계층이 공유하는 semantic stable ID이며
Client asset path나 deploy vector index가 아니다.

## 7. 데이터 계약 방향

### 7.1 Encounter schema v2

`ValtanEncounter.json`은 다음 다섯 층을 명시한다.

```json
{
  "schema": "lostark.encounter-profile",
  "formatVersion": 2,
  "encounterId": "ENCOUNTER_VALTAN",
  "referenceProfileId": "NORMAL_8P_2022",
  "healthPools": [
    { "id": "BODY_160", "displayBars": 160 },
    { "id": "GHOST_40", "displayBars": 40 }
  ],
  "phases": [
    { "id": "BODY_DESTRUCTION", "healthPoolId": "BODY_160", "patternPoolId": "BODY_DESTRUCTION_POOL" },
    { "id": "BODY_BROKEN_ARENA", "healthPoolId": "BODY_160", "patternPoolId": "BODY_NORMAL_POOL" },
    { "id": "GHOST", "healthPoolId": "GHOST_40", "patternPoolId": "GHOST_POOL" }
  ],
  "scriptedMechanics": [
    {
      "id": "ARENA_BREAK_LEFT_OR_RIGHT",
      "referenceBarApprox": 88,
      "triggerHpRatio": 0.55,
      "once": true,
      "patternId": "VALTAN_ARENA_BREAK_HALF"
    }
  ],
  "patternPools": [],
  "patterns": []
}
```

각 pattern은 `targetPolicy`, `facingPolicy`, `movementPolicy`, `cooldownTicks`, `repeatLimit`,
`stages[]`를 가진다. 각 stage는 tick duration과 필요한 `hitWindows[]`, `worldEvents[]`만 가진다.
hit window의 shape는 `CIRCLE`, `RING`, `CONE`, `BOX`, `CROSS` 중 실제 소비가 생긴 것만 admission한다.
각 hit은 damage profile, knockback vector/profile, crowd-control profile을 참조한다.

`referenceActionId=420621` 같은 LPK ID와 한글 action명은 provenance/reference에 남기되 Server
분기 키는 `VALTAN_JUMP_SPIN` 같은 stable pattern ID다.

### 7.2 World event binding

발탄 HP mechanic을 `triggerBox`로 흉내 내지 않는다. `Gameplay.world.json`에 area-level
`worldEventBindings`를 두고 Server encounter가 semantic event를 제출한다.

```json
{
  "eventId": "valtan.arena.break.half-a",
  "mutations": [
    { "type": "setDestroyableGroupState", "targetId": "valtan.arena.half-a", "value": "FRACTURED" },
    { "type": "setCondition", "targetId": "VALTAN_HALF_A_COLLAPSED", "value": true },
    { "type": "setArenaState", "targetId": "valtan.arena", "value": "HALF_A_COLLAPSED" }
  ]
}
```

같은 Server tick에서 모든 mutation을 validate/stage한 뒤 일괄 commit한다. 하나라도 실패하면
destroyable, navigation, arena state를 전부 기존 상태로 유지한다. 이 계약은 08-05 trigger/
destroyable 계획을 구현하기 전에 그 계획의 G01/G03에 반영한다.

### 7.3 Client presentation binding

`BossCatalog.json`에는 idle/chase/dead 기본 clip만 남긴다. pattern별 표현은 별도
`Valtan.patternbindings.json`이 소유한다.

```json
{
  "patternId": "VALTAN_JUMP_SPIN",
  "sourceActionId": 420663,
  "actions": [
    { "actionId": "valtan.jump-spin.takeoff", "clips": ["att_battle_20_01"], "loop": false },
    { "actionId": "valtan.jump-spin.spin", "clips": ["att_battle_20_02", "att_battle_20_03", "att_battle_20_04"], "loop": false }
  ]
}
```

Client는 `patternInstanceId + actionId + actionStartTick`이 바뀔 때만 one-shot을 시작한다.
late join 또는 packet 지연 시 `serverTick - actionStartTick`으로 현재 clip과 track position을 찾아
`CModel::Set_AnimTrackPosition`으로 seek한다. 지금처럼 모든 pattern clip을 loop로 재생하지 않는다.

## 8. Server authority 실행 흐름

```text
CGameRoom fixed 30 Hz
-> CValtanBrain::Update
   -> HP threshold crossing을 먼저 검사
   -> 미실행 scripted mechanic이 있으면 일반 pattern을 preempt
   -> 아니면 현재 phase pattern pool을 range/cooldown/repeat/condition으로 filter
   -> room seed + pattern instance sequence로 결정적 선택
   -> target/facing lock
   -> stage tick 진행
   -> hit window에서 Server shape query + damage/CC/knockback
   -> stage worldEventId를 CGameRoom::Apply_WorldEvent에 제출
-> world mutation과 path invalidation commit
-> snapshot broadcast
```

`CValtanBrain`을 범용 `CBossPatternManager`로 교체하지 않는다. 발탄 전용 runtime state는
`SERVER_VALTAN_RUNTIME_STATE`로 분리해 현재 `SERVER_WORLD_ENTITY`의 공통 transform/HP와 조합한다.
상태에는 current phase/health pool, pattern instance sequence, stage index/start tick, target/facing,
scripted mechanic consumed bitset, armor/stagger/counter window, deterministic RNG state만 둔다.

HP가 threshold를 한 tick에 여러 개 통과하면 낮은 threshold로 건너뛰지 않고 가장 먼저 지나친
미실행 mechanic 하나를 queue한다. mechanic 실행 중 HP lock/invulnerability 정책은 pattern별로
명시한다. animation이 끝났다는 Client 신호로 Server stage를 넘기지 않는다.

## 9. 벽 파괴와 arena 붕괴 적용 방식

### 9.1 시작 외벽과 armor break

1. Server charge pattern이 target과 locked direction을 확정한다.
2. charge swept shape가 `wallReceiverId`와 충돌하면 같은 tick에 charge를 종료한다.
3. `valtan.wall.<segment>.fracture` world event를 적용해 해당 stable destroyable group을 바꾼다.
4. 발탄을 groggy로 전이하고 weak-point window를 연다.
5. player weak-point command의 Server 판정만 armor stack을 감소시킨다.
6. snapshot은 boss armor/window와 destroyable state를 함께 보낸다.

`420654`와 `Par_D_RPBF_PartsDestruction_01`은 이 표현의 LPK 근거다. wall 충돌 판정 자체를
Client particle 또는 mesh collision 결과에 맡기지 않는다.

### 9.2 x88/x34 arena half collapse

- authoring에서 floor/edge를 `half-a`, `half-b`, `final-platform` stable group으로 확정한다.
- collapse landing tick에 visual state, Server nav condition, arena lethal-edge state를 한 번에 commit한다.
- path는 즉시 재계산하고 실패하면 현재 move goal을 취소한다.
- 플레이어는 click path로 낭떠러지 밖을 걸어 나가지 못한다. Server knockback displacement가 활성
  lethal edge를 넘어간 경우에만 낙사 판정한다.
- 늦게 입장한 Client도 snapshot의 destroyable/arena state로 즉시 동일 화면을 만든다.

현재 Deploy 85건은 시각 후보일 뿐 `half-a/b` 그룹 정본이 아니다. `TriggerMapData.loa`,
DeployData, Matinee와 기준 영상을 교차해 group을 확정하고, 임의의 각도/인덱스로 반을 나누지 않는다.

## 10. 공중 도약·하늘·카메라 연출 적용 방식

공중 이탈을 animation root motion 하나로 처리하지 않는다. Server action은 다음 semantic stage를
소유한다.

```text
TAKEOFF -> AIRBORNE_ASCEND -> OFFSTAGE_TARGETING
-> DESCEND -> LAND_IMPACT -> RECOVERY
```

- Server: targetable/invulnerable/collision, logical position, landing anchor, damage window, landing tick을 소유한다.
- Client `CValtan`: actionStartTick 기준으로 Start/Loop/End clip을 seek·재생하고 Server transform을 보간한다.
- Level presentation: semantic arena state를 sky/camera/effect cue로 번역한다.
- Camera cue 동안 gameplay input submission을 막고 종료 후 기존 follow/free 요청 상태로 복귀한다.
- F6 외 새 전역 기능키를 만들지 않는다.

원작 고공 점프 `420610`은 `Att_Battle_8_01_Start/Loop/End`를 사용하므로 Battle 8 추출 전에는
정확한 공중 animation 완료로 표시하지 않는다. `420621/420663`의 점프 후 휠윈드는 현재 Battle 20으로
먼저 검증할 수 있다.

하늘은 한 장의 붉은 sky texture가 아니다.

```text
sky_mirror_sm                 기본 배경
par_d_spacehole_03            청백색 중심/깊이
par_d_hugechaosgate_01        붉은 ring/cloud/electric
```

현재 runtime은 exact texture를 사용한 proxy layer 6개와 MapTool 수동 visibility까지만 있다.
`EncounterPresentation.json`은 `BASELINE -> SPACEHOLE -> CHAOS_GATE -> GHOST` state별 layer,
fade-in/out tick, camera cue, color/intensity를 저장한다. 활성 시점은 TriggerMapData/Matinee를 추가
복원하기 전까지 `VIDEO_TUNED`로 표시하고 `OFFICIAL_EXTRACTED`로 가장하지 않는다.

boss action이 참조하는 `FX_MN_RPBF_00_*` particle은 Effect Tool의 새 빈 asset으로 추측 생성하지
않는다. UPK export를 찾아 texture/material/emitter source를 추출하고, 현재 Effect Tool 문서로
authoring한 뒤 immutable resource pack에 배포한다.

## 11. Balance Tool·Animation Tool·MapTool 역할

| Tool | 편집하는 것 | 편집하지 않는 것 |
|---|---|---|
| Balance Tool | boss stats, phase threshold, pattern pool, tick/range/shape, damage/CC profile | clip, particle, camera asset |
| Animation Tool | actionId별 ordered clip, loop/one-shot, clip-local presentation cue | HP, damage, Server hit 판정 |
| Effect Tool | 추출된 particle/material을 표현 asset으로 저작·preview | phase 전이와 damage |
| MapTool | destroyable group, wall receiver, arena segment, world event binding, nav condition | runtime NetEntityId와 Server phase |

Balance Tool Save는 `Data` authoring과 provenance만 바꾸고 Validate한다. Publish 후 Server 재시작이
필수이며 hot reload는 추가하지 않는다. 오른쪽 Live Verification에는 최소 다음을 표시한다.

- Server tick, balance revision.
- health pool, phase, patternId, patternInstanceId, stage index/start tick.
- locked target/facing, invulnerability/counter/armor/stagger.
- arena state/world revision과 최근 world event.
- 최근 damage/knockback/ledge-death event.

## 12. G별 구현 순서

### G00. 원작 reference 추출기와 증거 고정

- `MN_RPBF_00`, `01-1`, `02-2`를 같은 parser로 action/stage/Anim/notify/particle/camera 문자열까지 추출한다.
- `Data/Animation/Reference/Valtan`에 `.clipmap`, `.clipseq`, `.animnotify`, `.animevents`, action receipt를 만든다.
- action object 수, stage 수 합, Anim notify 수, wmodel clip resolve 수가 일치하지 않으면 실패한다.
- `02-2`는 별도 profile로 격리하고 Normal 2022에 자동 merge하지 않는다.

종료 증거: 원본 hash 고정, 재실행 byte-identical, 420600 계열 action/clip/notify 누락 0.

### G01. 현재 asset으로 실제 이름의 1차 pattern pool

현재 있는 Battle 2/4/7/19/20만으로 아래 다섯 패턴을 구현한다.

```text
VALTAN_DOWN_SMASH          420602/420661
VALTAN_DASH_CHARGE         420604
VALTAN_EARTHQUAKE_SMASH    420605/420662
VALTAN_JUMP_SPIN           420621/420663
VALTAN_FRONT_BACK_FRONT    420637/420666
```

`VALTAN_BASIC_SWING`과 generic windup/active/recovery clip binding을 제거한다. G01은 벽·ghost 없이
pattern 선택, ordered clip, cone/ring shape, startTick seek, 두 Client 동일성까지 닫는다.

종료 증거: 다섯 semantic pattern이 Server에서 선택되고 Client 둘이 같은 stage/clip을 재생하며
hit/HP가 동일하다.

### G02. 누락 Valtan animation/effect resource pack

- Battle 1, 5, 8~18, 21의 실제 package/AnimSet을 추적해 `.wmodel` 통합 경로로 cook한다.
- `FX_MN_RPBF_00_*` 중 G03~G06이 소비하는 effect만 source exact로 추출한다.
- 기존 `Character/Valtan` asset을 한 immutable pack version으로 교체하고 manifest/lock을 갱신한다.

종료 증거: 92 action clip reference 중 선택한 Normal profile 필수 clip resolve 100%, Hydrate/Verify PASS.

### G03. Encounter schema v2와 publisher

- `BossProfiles`의 base stat과 `ValtanEncounter`의 phase/pattern을 분리한다.
- exact property, duplicate ID, bad reference, invalid tick/shape, empty stage, cycle, unreachable phase를 거부한다.
- publish는 gameplay/world/navigation bootstrap을 rollback set으로 검증한다.

종료 증거: 정상 publish, 잘못된 ID/version/path/중간 실패 rollback harness PASS.

### G04. Server pattern/phase authority와 Shared replication

- `CValtanBrain`에 scripted preemption, pool selection, stage timeline, hit shape, counter/grab/stagger/armor를 추가한다.
- snapshot에 pattern instance/stage/arena revision을 추가하고 protocol harness를 갱신한다.
- 진행 중 pattern은 시작한 balance revision을 끝까지 사용한다.

종료 증거: Server contract test에서 threshold once, repeat limit, target/facing lock, damage once,
counter success/fail, 두 Client snapshot 동일 PASS.

### G05. 외벽 충돌·destroyable·동적 navigation

- 08-05 trigger/destroyable 계획의 publisher/Server/Client vertical slice를 구현한다.
- player trigger box와 encounter world event를 같은 typed world mutation endpoint에 연결한다.
- 420654 charge wall collision, armor break window, stable destroyable group을 연결한다.

종료 증거: charge가 정확한 wall receiver에 충돌할 때만 fracture, armor window와 nav revision 동시 commit,
late join 정합, 실패 rollback PASS.

### G06. x88/x34 arena collapse와 낙사

- half-a/half-b/final-platform group과 navblocker/lethal edge를 작성한다.
- collapse landing tick에 visual/nav/lethal state를 atomic commit한다.
- Server knockback과 ledge death를 추가한다.

종료 증거: 걷기로 낙사하지 않음, Server knockback으로만 낙사, 두 Client 파괴 상태와 path 동일 PASS.

### G07. 고공 점프·모든 지형물 파괴·sky/camera/effect

- 420610 Start/Loop/End, 420629 Battle 12, 420658 Battle 16/Event Battle 5를 presentation binding에 연결한다.
- Level-owned encounter presentation이 SpaceHole/ChaosGate, camera cue, input block/follow 복귀를 수행한다.
- Server landing/world-event tick과 Client effect/camera cue가 같은 actionStartTick을 기준으로 한다.

종료 증거: 두 Client의 takeoff/landing/world mutation tick 동일, cinematic 종료 후 follow/F6 상태 복귀,
누락 effect가 gameplay state를 깨지 않고 해당 표현만 실패 보고.

### G08. x130~Ghost 전체 scripted timeline

- x130 wipe, x115~105 pillar, x88, x64, x34, x15 transition, Ghost 40 health pool을 순서대로 admission한다.
- Ghost model/clip이 별도 asset이면 실제 원본을 확보한 뒤에만 추가한다.
- clone counter/armor/grab/edge rush와 clear를 Server authority로 닫는다.

종료 증거: 8-player fixture와 두 실제 Client에서 전체 phase 순서, 중복 mechanic 0, late join, disconnect,
clear 전이 PASS.

### G09. Tool·provenance·regression

- Balance Tool, Animation Tool, MapTool, Effect Tool의 위 경계를 실제 consumer까지 연결한다.
- Normal 2022 reference와 `PROJECT_TUNED` field를 구분한다.
- Debug/Release 전체 build/regression, ProjectAudit `-DeepAssetHash`, Valtan full smoke를 실행한다.

## 13. 파일 영향 지도

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Data/Encounters/Valtan/ValtanEncounter.json` | phase/pattern Server 정본 |
| 수정 | `C:/Users/user/Desktop/LostArk/Data/Balance/BossProfiles.json` | 발탄 base stat만 소유 |
| 추가 | `C:/Users/user/Desktop/LostArk/Data/Animation/Reference/Valtan/` | LPK 추출 reference |
| 추가 | `C:/Users/user/Desktop/LostArk/Data/Animation/Authored/Valtan/Valtan.patternbindings.json` | Client animation/effect binding |
| 수정 | `C:/Users/user/Desktop/LostArk/Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json` | world event/destroyable group |
| 수정 | `C:/Users/user/Desktop/LostArk/Data/Navigation/LV_LUT_HEARTRB_ED.navblockers` | 붕괴 구역 condition |
| 추가 | `C:/Users/user/Desktop/LostArk/Data/Maps/Authoring/LV_LUT_HEARTRB_ED/EncounterPresentation.json` | sky/camera/fade binding |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | encounter v2 검증/publish |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/WorldPipeline/Publish-WorldGameplay.ps1` | world event/destroyable 검증 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/NavigationPipeline/Publish-ServerNavigation.ps1` | dynamic blocker cook |
| 수정 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketMessages.h` 외 writer/reader/harness | semantic runtime 복제 |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Public/ValtanBrain.h` | 발탄 Server public 실행 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Private/ValtanBrain.cpp` | pattern/phase/stage authority |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Public/ServerWorldEntity.h` | boss 공통 state와 발탄 state 연결 |
| 수정 | `C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp` | world event atomic commit/snapshot |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Valtan.h` | semantic presentation state |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Valtan.cpp` | ordered one-shot/seek/interpolation |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_ValtanArena.cpp` | deploy/arena presentation/camera owner |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/ClientReplication.cpp` | boss/destroyable/arena state consumer |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/BalanceTool.cpp` | phase/pattern live verification |

새 C++ 파일은 G별 전체 코드 계획에서 물리 폴더와 `.vcxproj`/`.vcxproj.filters` 등록을 함께 확정한다.
상위 방향 단계에서는 미래용 빈 Manager/placeholder를 먼저 만들지 않는다.

## 14. 검증 게이트

각 G는 필요한 하위 집합을 실행하고 G09에서 전체를 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate
Tools/NetworkProtocolHarness/Bin/Debug/NetworkProtocolHarness.exe
Server/Bin/Debug/Server.exe --contract-test
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -DeepAssetHash
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -DeepAssetHash
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1 -DeepAssetHash
git diff --check
```

수동 Valtan smoke는 다음을 별도 기록한다.

```text
Lobby Server 승인 -> Valtan Arena
두 Client 동일 pattern/stage/HP
외벽 charge collision -> fracture + armor window
x130 -> x115~105 -> x88 -> x64 -> x34 -> x15
공중 이탈/착지와 camera/input 복귀
half-a/half-b 파괴와 knockback 낙사
Ghost 40 health pool/counter/grab/clear
late join과 disconnect recovery
잔류 Client/Server process 및 7777 listener 없음
```

## 15. 출처와 증거 등급

### 공식

- [Lost Ark Academy - Valtan Legion Raid](https://www.playlostark.com/en-gb/news/articles/lost-ark-academy-valtan?language-picker=true)
- [Lost Ark Academy - Inferno Legion Raids](https://www.playlostark.com/en-us/news/articles/lost-ark-academy-inferno-legion-raids?language-picker=true)

### 패턴·HP line 교차검증

- [Icy Veins - Valtan Gate 2](https://www.icy-veins.com/lost-ark/valtan-legion-raid-gate-2)
- [Mobalytics - Valtan Phase 2](https://mobalytics.gg/blog/lost-ark/lostark-valtan-phase-2-legion-raid-guide/)
- [ATK - Valtan Gate 2 Guide](https://www.youtube.com/watch?v=K3LI4bYCdeI)

### 로컬 exact

- `.codex_tmp/data3_reextract_20260805/data3/EFGame_Extra/ClientData/XmlData/Action/MN_RPBF_00.loa`
- `.codex_tmp/data3_reextract_20260805/data3/EFGame_Extra/ClientData/XmlData/Action/MN_RPBF_01-1.loa`
- `.codex_tmp/data3_reextract_20260805/data3/EFGame_Extra/ClientData/XmlData/Action/MN_RPBF_02-2.loa`
- `Client/Bin/Resources/Character/Valtan/anims/*.wanim`
- `Data/Maps/Imported/LV_LUT_HEARTRB_ED/*.deployassets|*.deployplacements`

LPK action name·clip·notify·effect reference는 `OFFICIAL_EXTRACTED` 근거가 될 수 있다. community HP
line과 영상 측정 timing은 `REFERENCE_ONLY`, 이 프로젝트가 선택한 tick/shape/damage/fade는
`PROJECT_TUNED`로 기록한다.

