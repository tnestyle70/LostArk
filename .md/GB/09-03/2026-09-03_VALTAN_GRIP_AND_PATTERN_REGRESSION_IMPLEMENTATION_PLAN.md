# 2026-09-03 Valtan 왼손 부착과 패턴 회귀 통합 구현 계획서

작성 기준 브랜치 `GB/valtan-bugfix-koukusaydon-pattern`, HEAD `de3cc494`, worktree clean.
이 문서는 구현 계획서다. 범위, 현재 실측, 변경 파일, 호출 흐름, G별 구현 범위와 검증만 소유한다.
G별 전체 코드 정본이 필요하면 같은 폴더에 `_DETAIL_PLAN.md`를 따로 만든다.

읽은 정본: `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, `.md/GB/gotchas.local.md`,
`.md/GB/계획서하네스규칙.local.md`, `.md/GB/local.md`, `.md/TEAM/README.md`.

## 0. 질문과 G 배치

| # | 사용자 질문 | 실측 결론 | 담당 G |
|---|---|---|---|
| Q1 | 추적 도끼 패턴에서 도끼 이펙트가 왜 한 개만 생성되나. 플레이어 수+위치 기준 생성과 랜덤 4개 3회 반복은 왜 사라졌나 | Server 런타임은 온전하고 `Valtan.gameplay.json` 한 블록이 커밋 `0e2e84b0`에서 축소됐다 | G03 |
| Q2 | 피자 패턴에 사자후 이펙트 group v2가 들어가 있다. 포효할 때 이펙트만 분리해서 Sequencer에서 조작할 수 있나 | 사자후는 presentation cue가 아니라 Effect V2 binding 문서에 있고, binding 제거/이동/추가는 이미 Workbench에서 가능하다. group 내부 자식 분할만 API가 없다 | G04 |
| Q3 | 워프 패턴에서 사거리 끝 소멸은 되는데 다시 시작할 때 플레이어 기준 뒤를 돌아본 상태로 시작해 일자 질주 연출이 안 된다 | 클립 대시(0~528ms)와 Server 이동(500~1300ms)이 엇박이고, leg 사이 재배치가 없으며, 포탈 큐가 원본의 준비 클립이 아니라 도착 시점에 있다 | G05 |
| Q4 | 왼손 부착이 아예 다른 위치에 붙는다. 현재 코드 전부 | world 단위 계약이 100배 틀렸고, grip offset이 잘못된 actionId로 조회된 뒤 실패가 조용히 0으로 대체된다 | G01, G02 |

G01과 G02가 사용자가 지금 겪는 증상의 직접 원인이므로 먼저 닫는다.
G03은 데이터 회귀 복구와 그 회귀를 다시 못 보게 만든 툴 구멍을 같이 닫는다.
G04와 G05는 저작 계약 변경이라 G01~G03이 끝난 뒤 별도 검증 단위로 진행한다.

## 1. 공통 실측 기준

### 1.1 Client world 단위는 미터이며 Server와 1:1이다

| 근거 | 위치 |
|---|---|
| 맵 에셋 pretransform `XMMatrixScaling(0.01f)` — cm 원본을 미터로 | `Client/Private/Loader.cpp:885` |
| 캐릭터 pretransform `0.01`(DimensionMaster, cm 원본) / `0.0001`(구형 팩, rig root x100) | `Client/Private/PlayableCharacterAssetService.cpp:200` |
| Valtan body `bodyModelPreScale 0.0001`, weapon `100.0` | `Data/Actors/BossCatalog.json` |
| `CModel::Get_BoneMatrix`가 `m_PreTransformMatrix`를 포함 | `Engine/Private/Bone.cpp:66`, `Engine/Private/Model.cpp:118` |
| Server snapshot 위치를 변환 없이 transform에 적용 | `Client/Private/Character.cpp:1132` |
| Valtan 텔레포트 임계 `NETWORK_TELEPORT_DISTANCE_SQ = 100.f`(10 단위) | `Client/Private/Valtan.cpp:71` |
| Server 충돌 반경 `collisionRadius 1.4`(m) | `Data/Balance/BossProfiles.json` |

Valtan 손뼈 combined 3x3은 약 `0.01`이고 그 translation은 이미 world 미터다.
`CPlayerHandGripTransform::WORLD_UNITS_PER_METRE = 100.f`만 world를 cm로 가정한다.

### 1.2 CAPTURE stage와 그 직후 분기

| 패턴 | CAPTURE stage | duration | hit offsets | 잡힘 직후 분기 |
|---|---|---|---|---|
| `VALTAN_TRASH` | `STEP_08` | 667 | 0,100,...,600 | `ANY_PLAYER_GRABBED -> ...catch-counter` |
| `VALTAN_TRASH` | `RETRY_RUSH_02` | 667 | 동일 | `...catch-counter` |
| `VALTAN_TRASH` | `RETRY_RUSH_03` | 667 | 동일 | `...catch-counter` |
| `VALTAN_TRASH_CATCH_IF` | `STEP_08` | 667 | 동일 | `...catch-counter` |
| `VALTAN_TRASH_CATCH_IF` | `RETRY_RUSH_02` | 667 | 동일 | `...catch-counter` |
| `VALTAN_TRASH_CATCH_IF` | `RETRY_RUSH_03` | 667 | 동일 | `...catch-counter` |
| `VALTAN_CATCH_BREATH` | `STEP_02` | 500 | 250 | `ANY_PLAYER_GRABBED -> ...step-03` |

7개 stage 모두 `gripLocalOffset = { forwardM 0.0, upM -0.9, rightM 0.0 }`이고 같은 값이다.
분기 대상은 전부 같은 패턴 안의 다른 stage이므로 패턴 단위 조회가 결정적으로 유일해진다.

### 1.3 프레임 순서

```text
CGameInstance::Update_Engine
  Object_Manager::Priority_Update
  Object_Manager::Update            <- CValtan::Update(본 갱신), CCharacter::Update_NetworkTransform
  Physics_Manager::Update
  Object_Manager::Post_Physics_Update
  Level_Manager::Update             <- CLevel_ValtanArena::Update -> CClientReplication::Update
                                       -> Update_PlayerAttachmentPresentations (부착 transform 확정)
  Object_Manager::Late_Update       <- CCharacter::Late_Update(BoneChains), CValtan::Late_Update
```

부착 transform은 캐릭터 자체 보간 뒤에 쓰이므로 덮이지 않는다.
다만 `CCharacter::Update`의 `m_pColliderCom->Update`와 `Late_Update`의 `m_BoneChains.Update`는
부착 전 transform과 네트워크 yaw를 쓰므로 한 프레임 어긋난다. 이 두 건은 G01/G02 범위 밖으로 둔다.

## 2. G01 — 왼손 grip 단위 계약 교정

### 2.1 목표와 종료 증거

`gripLocalOffset`의 미터 값이 world 미터 그대로 적용되어야 한다.
현재는 `-0.9 m`가 손바닥 아래 `90 m`로 적용된다.

종료 증거는 헤드리스 계약 하네스에서, 손뼈 basis가 `0.01` scale일 때
`{0, -0.9, 0}` 보정 결과 위치가 손뼈 world 위치에서 정확히 `0.9` 단위만 내려간 값이라는 검증이다.

### 2.2 수정 파일

| 파일 | 변경 |
|---|---|
| `Client/Public/PlayerHandGripTransform.h` | `WORLD_UNITS_PER_METRE = 100.f` 제거. Client world가 미터라는 사실을 주석과 함께 명시하고 보정 변위를 미터 그대로 사용 |

`.vcxproj` / `.filters` 신규 등록은 없다. 기존 헤더 수정이다.

### 2.3 변경 지점

`Compose_World(localOffset, handWorld, gripLocalOffset, outWorld)` 안의 변위 계산에서
`* WORLD_UNITS_PER_METRE` 세 개를 제거한다. 정규화된 손 축 세 개를 쓰는 계약과
`Is_ValidGripLocalOffset` 사전 검증, 영벡터 조기 반환은 그대로 둔다.

상수 자체는 삭제한다. 남겨 두면 다른 호출자가 다시 cm 가정을 되살릴 수 있다.
현재 이 상수의 유일한 소비자는 같은 헤더 안의 변위 계산 세 줄뿐이다.

`MAX_GRIP_OFFSET_COMPONENT_M = 10.f`는 유지한다.
`Publish-GameplayBalance.ps1:2185`의 `±10.0`, `BalanceTool.cpp:3939`의 draft 범위,
`ActionCompositionWorkbench.cpp:6400`의 드래그 clamp가 모두 이 상수를 참조하므로 값을 바꾸지 않는다.

### 2.4 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core
```

추가로 G01 전용 계약 검사를 `Server/Private/ServerGameplayContractTests.cpp`가 아니라
Client 헤더 전용 순수 수학이므로 기존 grip 계약 하네스에 붙인다.
현재 저장소에 `CPlayerHandGripTransform` 전용 실행형 하네스가 없으므로 G01에서 함께 추가한다.
검사 항목은 다음 네 가지다.

```text
1. 손뼈 basis scale 0.01, gripLocalOffset {0,-0.9,0} -> 결과 위치가 손 위치 -0.9 단위
2. 손뼈 basis scale 1.0 동일 입력 -> 같은 -0.9 단위 (basis scale 비의존)
3. gripLocalOffset {0,0,0} -> Compose_World 2인자 결과와 완전 동일
4. 비유한 / 범위 초과 성분 -> false 반환, out 미변경
```

## 3. G02 — 왼손 grip action identity 확정과 silent fallback 제거

### 3.1 목표와 종료 증거

`ANY_PLAYER_GRABBED` 분기가 같은 tick에 stage를 넘겨도 보정이 사라지지 않아야 한다.
보정을 못 찾으면 조용히 0으로 붙이지 않고 실패 이유를 남긴 채 Server fallback을 유지해야 한다.

종료 증거는 실제 Server + Client 실행에서 잡기 성공 시 사용자가 손 위치 부착을 확인하는 것이며,
자동 검증은 조회 실패 시 `bHasGripLocalOffset`이 false로 남고 진단 문자열이 남는 것까지다.

### 3.2 근거

`Client/Private/ClientReplication.cpp:2207`

```cpp
attachment->second.GripLocalOffset = {};
(void)valtan->Try_Get_PlayerHandGripLocalOffset(
    valtan->Get_ServerActionId(),
    attachment->second.GripLocalOffset);
attachment->second.bHasGripLocalOffset = true;
```

반환값을 버리고 `true`를 무조건 세운다.
그리고 `Get_ServerActionId()`는 CAPTURE stage가 아니라 이미 분기한 stage의 actionId다.

### 3.3 수정 파일

| 파일 | 변경 |
|---|---|
| `Client/Public/Valtan.h` | grip 조회 키를 actionId에서 patternId로 바꾸는 접근자 선언 추가. 기존 actionId 접근자는 Tool 진단용으로 유지 |
| `Client/Private/Valtan.cpp` | `Reload_PlayerHandGripLocalOffsets_WhileAdmitted`가 patternId 기준 map을 함께 만들고, 한 패턴 안 CAPTURE stage들의 grip 값이 서로 다르면 admission 실패 |
| `Client/Private/ClientReplication.cpp` | `Update_PlayerAttachmentPresentations`가 boss의 replicated patternId로 조회하고 반환값을 그대로 `bHasGripLocalOffset`에 반영. 실패 시 진단 문자열 보존 |

### 3.4 계약

patternId 기준 map은 다음 규칙으로 만든다.

```text
CAPTURE stage가 하나도 없는 패턴 -> map에 넣지 않는다
CAPTURE stage가 여러 개이고 grip 값이 모두 같은 패턴 -> 그 값 하나를 넣는다
CAPTURE stage들의 grip 값이 서로 다른 패턴 -> reload 실패, 이전 admitted 상태 보존
```

현재 데이터에서 `VALTAN_TRASH` 3개, `VALTAN_TRASH_CATCH_IF` 3개, `VALTAN_CATCH_BREATH` 1개가
모두 `{0, -0.9, 0}`이므로 세 패턴 전부 결정적으로 하나의 값을 갖는다.
값을 stage별로 다르게 저작해야 하는 요구가 생기면 그때 Shared에
capture actionId를 복제하는 protocol 변경을 별도 수직 슬라이스로 연다.
이 계획서에서는 protocol version을 올리지 않는다.

Client가 참조하는 boss patternId는 이미 replicated world entity 상태에 있다.
`CValtan`이 `m_strServerPatternId`로 보관하므로 새 wire 필드가 필요 없다.

### 3.5 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core
```

사용자 수동 확인 경로를 함께 보고한다.

```text
Framework.slnLaunch 의 Server + Client profile 실행
Lobby -> Character Select 진입 -> Valtan 진입
Debug F1 -> Valtan 소환 -> VALTAN_TRASH 또는 VALTAN_CATCH_BREATH 발동
잡힌 캐릭터가 왼손 안쪽에 붙는지 사용자가 직접 판정
```

에이전트는 Client를 자율 실행하지 않으며 사용자의 서면 판정 전에는 visual PASS로 기록하지 않는다.

## 4. G03 — 추적 도끼 volley 복구와 툴 구멍 봉합

### 4.1 목표와 종료 증거

`VALTAN_HIGH_JUMP / AIRBORNE`이 살아있는 플레이어 수만큼의 추적 도끼와
아레나 랜덤 4개를 1333ms 간격 3회 반복으로 다시 만들어야 한다.
그리고 같은 회귀가 다시 조용히 들어오지 않도록 툴 표면을 닫는다.

### 4.2 회귀 근거

커밋 `0e2e84b0 valtan-pattern-bugfix`(2026-09-03 02:19)의
`Data/Valtan/Valtan.gameplay.json` hunk에서 다음이 축소됐다.

```text
spawnSchedule.count       3    -> 1
spawnSchedule.intervalMs  1333 -> 0
arenaRandom               RANDOM_NAVIGABLE_CIRCLE / BOSS_SPAWN_POSITION / 4 / 14.0 / 1.0 -> NONE
maximumTotalObjects       36   -> 4
```

그 이전 여덟 개 커밋(`4601f799`, `d8f73564`, `6d95a903`, `8ad9420f`, `3ea1088c`,
`fe0dab04`, `8769ebab`, `780fd0bd`)까지 값이 동일했다.

Server 실행 코드는 온전하다.

| 계약 | 위치 |
|---|---|
| 웨이브 반복과 한 stage 한 clock 검증 | `Server/Private/GameRoom.cpp:9689` |
| 살아있는 플레이어별 lock | `Server/Private/GameRoom.cpp:10061` |
| 아레나 랜덤 원점 결정적 생성 | `Server/Private/GameRoom.cpp:9189` |
| 호출 지점 | `Server/Private/GameRoom.cpp:10171` |

생성물에도 그대로 전파돼 있다. `Data/Encounters/Valtan/ValtanEncounter.json:491`.

### 4.3 G03-A 데이터 복구

`Data/Valtan/Valtan.gameplay.json:1086` 블록을 이전 계약으로 되돌린다.

```json
"spawnSchedule": {
  "kind": "INTERVAL",
  "count": 3,
  "firstOffsetMs": 0,
  "intervalMs": 1333
},
"arenaRandom": {
  "kind": "RANDOM_NAVIGABLE_CIRCLE",
  "anchor": "BOSS_SPAWN_POSITION",
  "count": 4,
  "radiusM": 14.0,
  "heightToleranceM": 1.0
},
"allowOverlap": false,
"maximumTotalObjects": 36
```

`AIRBORNE` duration은 8000ms이므로 마지막 웨이브 offset 2666ms가 stage 안쪽이다.
4인 기준 웨이브당 최대 8개, 3웨이브 누적 24개로
`MAX_COMBAT_OBJECTS_PER_SNAPSHOT = 128` 안쪽이다.

### 4.4 G03-B BalanceTool 모델을 파일에서 읽게 만든다

`Client/Public/BalanceTool.h:513` `VALTAN_AXE_VOLLEY_EDIT`는 다음 값을 C++ 기본값으로 갖는다.

```cpp
std::uint32_t maximumTotalObjects = 36u;
std::uint32_t spawnCount          = 3u;
std::uint32_t spawnIntervalMs     = 1333u;
std::string   arenaRandomKind     = "RANDOM_NAVIGABLE_CIRCLE";
std::string   arenaAnchor         = "BOSS_SPAWN_POSITION";
std::uint32_t arenaRandomCount    = 4u;
double        arenaRandomRadiusM  = 14.0;
double        arenaHeightToleranceM = 1.0;
```

`Client/Private/BalanceTool.cpp:4982`에서 기본 생성 후 `:5334`에서
`countPerResolvedTarget` 하나만 트리 값으로 덮는다. 나머지는 파일을 읽지 않는다.
그래서 툴 내부 모델과 실제 파일이 다르고,
도끼 개수를 한 번만 바꿔도 `:9643`의 `SET_AXE_VOLLEY` op이
사용자가 건드리지 않은 spawnSchedule과 arenaRandom까지 기본값으로 덮어쓴다.

변경 범위는 다음과 같다.

```text
VALTAN_COMBAT_OBJECT_EFFECT_VIEW 에 spawnCount / spawnIntervalMs /
arenaRandomKind / arenaAnchor / arenaRandomCount / arenaRandomRadiusM /
arenaHeightToleranceM 를 추가하고 ValtanPatternTree 파싱에서 채운다
BalanceTool Reload 가 이 뷰에서 m_valtanAxeVolley 전체를 채운다
VALTAN_AXE_VOLLEY_EDIT 의 하드코딩 기본값을 중립값으로 낮춰
파일을 못 읽었을 때 조용히 예전 계약을 흉내내지 않게 한다
```

`Client/Private/ValtanPatternTree.cpp:5098` 이하 volley 파서는 이미
`spawnSchedule`과 `arenaRandom`을 exact-property로 검증하며 읽고 있으므로
뷰 필드만 늘리면 된다. 새 파서를 만들지 않는다.

### 4.5 G03-C 저장 후 검증기의 하드코딩 상수 제거

`Client/Private/BalanceTool.cpp:6305` 와 `:6338` 에 다음 상수가 박혀 있다.

```cpp
3u != spawnCount || 1333u != spawnIntervalMs
arenaRandomKind != "RANDOM_NAVIGABLE_CIRCLE"
4u != arenaRandomCount || 14.0 != arenaRandomRadiusM || 1.0 != arenaHeightToleranceM
```

이는 특정 튜닝 값을 계약으로 고정한 것이라, 값이 바뀌면 Save 후 Reload가 거부된다.
지금은 `Intermediate/ValtanTuningAuthoring/current-authoring.json` 포인터가 없어
`RestoreValtanSavedAuthoring` 경로가 실행되지 않아 조용할 뿐이다.

파이프라인 검증기 `Tools/ValtanPipeline/valtan_tuning_pipeline.py:3005` 와 `:3035` 는
`count 1`과 `arenaRandom NONE`을 정상으로 통과시킨다.
두 계층의 계약이 서로 다르므로, Client 상수를 파이프라인과 같은 구조 검증으로 낮춘다.

```text
남길 검사   INTERVAL kind, count 1..8, interval/ count 정합, stage 안쪽, arenaRandom exact 필드, 범위
지울 검사   특정 숫자 3 / 1333 / 4 / 14.0 / 1.0 동등 비교
```

### 4.6 G03-D Workbench 타임라인 노출

`Client/Private/ActionCompositionWorkbench.cpp:4108` 라벨은 지금
`Server Combat Object (read-only) x1 @+0ms` 만 보여 준다.
`iSpawnValue`와 `iFirstSpawnOffsetMs`만 뷰에 있기 때문이다.
G03-B에서 늘린 필드를 써서 웨이브 수, 간격, 아레나 랜덤 수를 같은 라벨에 넣는다.
읽기 전용 진단 표시이며 이 G에서 편집 UI를 만들지 않는다.

### 4.7 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
python Tools/ValtanPipeline/test_valtan_pattern_master_v2.py
python Tools/ValtanPipeline/test_valtan_balance_tool_contract.py
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
```

사용자 수동 확인은 Valtan `VALTAN_HIGH_JUMP` 발동 시
공중 8초 동안 1333ms 간격으로 도끼 세트가 세 번 떨어지는지, 세트마다
플레이어 위치 추적 도끼와 아레나 랜덤 도끼가 함께 나오는지다.

## 5. G04 — 피자 패턴의 사자후 분리

### 5.1 현재 구조

`VALTAN_SIX_PIZZA_106`의 `Valtan.presentation.json` effectCues는 `STEP_01`에 하나뿐이다.
사자후는 `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`의 두 행이다.

| bindingId | resource | scope | clock |
|---|---|---|---|
| `binding.valtan.project-tuned.six-pizza.001.shout-loop` | GROUP `boss.valtan.shout` | `VALTAN_SIX_PIZZA_106 / STEP_06` | `CLIP_OCCURRENCE : 0 : EACH_LOOP`, clip `...step-06.clip-01` |
| `binding.valtan.project-tuned.six-pizza.002.shout-burst` | GROUP `boss.valtan.shout.burst` | `VALTAN_SIX_PIZZA_106 / STEP_07` | `CLIP_OCCURRENCE : 733 : ONCE`, clip `...step-07.clip-01` |

`STEP_06` duration 8000ms에 clip이 `LOOP_TO_STAGE_END`이고 binding이 `EACH_LOOP`이므로
`boss.valtan.shout` 자식 20개가 8초 내내 반복된다.
사용자가 원하는 포효 순간은 `STEP_07`의 `boss.valtan.shout.burst`다.

| group | 자식 | 성격 |
|---|---|---|
| `boss.valtan.shout` | 20개. `shout.comet_1` 9, `shout.comet_2` 9가 yaw 0/45/60/90/135/180/225/270/315로 startMs 200~1650 계단식, `shout.converge_1` 0ms, `blur_3` 0ms | 사방 확산 혜성 링 |
| `boss.valtan.shout.burst` | 6개. `shout.fog_1~4`, `shout.emit_1`, `blur_4` 전부 0ms | 포효 순간 충격 안개와 발광 |

### 5.2 Sequencer가 지금 할 수 있는 것

`Client/Public/EffectV2_Catalog.h:94` 이하 mutation API는 네 개다.

| 동작 | 가능 | 근거 |
|---|---|---|
| binding 제거 | 가능 | `Stage_RemoveBossValtanStageBinding`. key가 `bindingId`뿐이라 CLIP_OCCURRENCE 행도 대상 |
| 시작 시각 이동 | 가능 | 박스 드래그 -> `Stage_UpdateBossValtanStageBindingStart` |
| 복제 | 가능 | `Stage_DuplicateBossValtanStageBinding` |
| LEAF/GROUP 추가 | 가능 | Resources 탭의 `V2 Authored Effects` / `V2 Effect Groups` |
| group 자식 편집과 분할 | 불가 | group 문서 mutation API가 없다 |
| 추가 시 anchor/follow/rotation/localTransform/repeat 지정 | 불가 | `EffectV2_Catalog.cpp:1171`에서 고정값 |
| 추가 시 CLIP_OCCURRENCE clock 지정 | 불가 | 항상 STAGE clock |

추가가 강제로 박는 값은 다음과 같다.

```cpp
eClockBasis    = STAGE;              // clip 지정 불가
strClipOccurrenceId.clear();
eRepeatPolicy  = ONCE;               // EACH_LOOP 불가
strAnchorSlotId = "b_effectroot";
eFollowPolicy  = SNAPSHOT_AT_START;
eRotationBasis = TARGET_YAW;
LocalTransform = {};                 // yaw 45/90/... 재현 불가
eStopPolicy    = NATURAL;
```

`Validate_NoLeafGroupClockOverlap`(`EffectV2_Catalog.cpp:601`)은
같은 leaf가 같은 유효 clock에 중복될 때만 거부하므로
group 제거와 leaf 추가 조합은 걸리지 않는다.

Save는 Pattern, Sound, EffectV2를 한 트랜잭션으로 커밋한다
(`ActionCompositionWorkbench.cpp:4806`).

### 5.3 G04-A 코드 변경 없이 즉시 가능한 분리

Workbench 타임라인에서 `binding.valtan.project-tuned.six-pizza.001.shout-loop` 박스를 선택해 제거한다.
8초 반복 혜성 링이 사라지고 `STEP_07`의 포효 burst만 남는다.
burst 박스는 드래그로 733ms 앞뒤 조정이 된다.

이 경로는 저장 파일 하나(`BOSS_VALTAN.effectv2bindings.json`)만 바뀌고
`Valtan.presentation.json`과 gameplay는 건드리지 않는다.

### 5.4 G04-B 원하는 자식만 쓰는 전용 group

`boss.valtan.shout`의 자식 일부만 쓰고 싶으면
`Data/Effects/V2/Groups/boss.valtan.shout.roar-only.effectv2group.json`을 새로 만들고
필요한 자식을 옮겨 담은 뒤 Workbench의 `V2 Effect Groups` 트리에서 `STEP_07`에 추가한다.

group 자식의 yaw와 startMs는 툴에서 만들 수 없으므로
사방 회전이 필요한 조합은 반드시 group 문서를 새로 저작해야 한다.
`formatVersion 2`, `groupId`가 파일명과 일치, 자식은 LEAF만(중첩 group 금지),
`Isolate_InvalidCrossReferences`가 leaf 실재를 검사한다.

### 5.5 G04-C 툴 안에서 끝내려면 필요한 확장

이 G에서 구현하지 않고 범위만 기록한다.

```text
CEffectV2Catalog 에 group 문서 stage/commit API
Stage_AppendBossValtanStageBinding 에 clock basis, anchor, follow, rotation,
localTransform, repeat 지정 경로
Workbench 에 group 자식 편집 패널
```

### 5.6 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1
```

사용자 수동 확인은 피자 패턴 `STEP_06` 8초 구간에서 혜성 링 반복이 사라지고
`STEP_07` 포효 시점에만 안개와 발광이 나오는지다.

## 6. G05 — 워프 leg 재정합

### 6.1 실측한 어긋남

| # | 원인 | 근거 |
|---|---|---|
| 1 | 클립 대시는 0~528ms에 7.2654m를 끝내고 이후 정지, Server 이동은 500~1300ms에 16m | `Data/Animation/RootMotion/Valtan.rootmotion.json` STEP_02~09, `Server/Private/ValtanBrain.cpp:3147` |
| 2 | 방향 전환을 의도적으로 500ms 지연하고 무보간 스냅 | `Server/Private/GameRoom.cpp:10530`, `Server/Private/ValtanBrain.cpp:1631` |
| 3 | leg 사이 텔레포트 없음. `PORTAL_TARGET_RUSH` 분기가 위치와 yaw를 쓰지 않고 조기 return | `Server/Private/ValtanBrain.cpp:3250` |
| 4 | stage 1800ms에 528ms 클립을 `LOOP_TO_STAGE_END` -> leg당 약 3.4회 재루프 | `Data/Valtan/Valtan.presentation.json` |
| 5 | 이동 16m 대 클립 7.2654m, 20m/s 대 클립 평균 13.8m/s | gameplay.json 대 rootmotion.json |
| 6 | 포탈 큐가 원본의 준비 클립 750/814ms가 아니라 각 leg 도착 1300ms에 있음 | `Data/Animation/Reference/Valtan/Valtan.animevents:482`, presentation.json |
| 7 | 원본 `420622`는 돌진 1회, 저작본은 8 leg | `Data/Animation/Reference/Valtan/Valtan.clipseq:98` |
| 8 | Client 회전 상한 720도/초 -> 180도 스냅이 250ms에 걸쳐 회전하며 그 사이 이동 | `Client/Private/Valtan.cpp:72` |
| 9 | `PORTAL_TARGET_RUSH`는 navigation을 우회하고 위치를 직접 대입 | `Server/Private/GameRoom.cpp:13526` |

`STEP_10`의 `RETURN_TO_ARENA_CENTER`는 정상 동작한다(`Server/Private/GameRoom.cpp:10482`).

### 6.2 결정이 필요한 선택지

이 G는 사용자 결정 없이 진행하지 않는다. 두 방향의 범위가 크게 다르다.

```text
A안 저작만 조정
   stage duration 을 클립 대시 길이에 맞추고 retargetDelayMs 를 0 으로
   distanceM 과 speedMps 를 클립 root motion 과 일치시키며
   LOOP_TO_STAGE_END 를 EXACT 로 바꾸고 포탈 큐를 준비 stage 로 옮긴다
   Server 코드 변경 없음. gameplay.json 과 presentation.json 만 바뀐다

B안 워프 재배치까지 구현
   A안에 더해 PORTAL_TARGET_RUSH 가 leg 시작에 아레나 반대편으로 텔레포트하고
   그 지점에서 타깃을 향해 정렬한 뒤 직선 질주하도록 Configure_PortalMotion 을 확장한다
   Server 모션 계약과 navigation 우회 정책, Client 텔레포트 스냅 임계까지 함께 닫아야 한다
```

### 6.3 검증

A안은 `Publish-GameplayBalance.ps1 -Mode Validate`와
`Tools/ValtanPipeline/test_valtan_portal_rush_tuning_contract.py`로 닫는다.
B안은 `Server.exe --contract-test`와 `Invoke-BuildAndRegression.ps1 -Profile FullDiagnostic`까지 필요하다.

두 안 모두 최종 연출 판정은 사용자가 직접 실행해서 한다.

## 7. 진행 순서와 커밋 단위

```text
G01 + G02   왼손 부착 한 커밋. 단위 교정과 identity 확정은 같은 증상의 두 원인이라 함께 검증한다
G03         도끼 volley 한 커밋. 데이터 복구와 툴 모델/검증기/타임라인을 같은 단위로 닫는다
G04         사자후 분리 한 커밋. G04-A 만으로 끝나면 binding 문서 한 개 변경이다
G05         사용자 결정 후 별도 커밋
```

각 커밋 전에 `git diff --check`, 변경 domain publisher `Validate`, 관련 harness,
정본 build/regression을 실행하고 실행한 것과 미실행한 것을 RESULT에 분리해 기록한다.

## 8. 이 계획서가 하지 않는 것

```text
Client 또는 UI 자율 실행과 화면 캡처
사용자 서면 판정 없는 visual PASS 기록
Shared protocol version 인상
Effect V2 group 자식 편집 UI 구현 (G04-C 로 범위만 기록)
CCharacter 콜라이더 한 프레임 지연과 BoneChains yaw 불일치 수정 (별도 슬라이스)
Character.cpp 의 도달 불가 두 번째 GRABBED 분기 제거 (요청 없는 죽은 코드)
```

## 9. 2026-09-03 실제 반영 요약

### 9.1 소스에 반영한 내용

| 구분 | 실제 반영 내용 |
|---|---|
| G01/G02 왼손 grip | `bone offset`을 100배 확대하던 단위 변환을 제거했다. Server가 복제한 `patternId`를 기준으로 CAPTURE grip을 조회하고, 같은 pattern 안의 상충하는 offset은 admission 단계에서 거부하며 stage 실패 시 기존 상태를 유지한다. |
| G03 도끼 낙하 | high-jump volley의 `spawnCount`, `spawnIntervalMs`, random navigable circle, 최대 투사체 수를 툴 모델과 저장 경로에 연결했다. 정본은 `3회 / 1333ms / boss spawn 4개 / 반경 14m / 허용오차 1m / 최대 36개`다. |
| G04 사자후 | `binding.valtan.project-tuned.six-pizza.001.shout-loop`를 제거하고 `STEP_07`의 one-shot burst만 유지했다. |
| 도넛 저장 abort | Ring 편집 Save가 graph reload 뒤의 무효 포인터를 같은 frame에서 다시 사용하던 use-after-free를 제거했다. Save가 reload를 수행하면 해당 UI frame을 즉시 끝내며 ImGui disabled scope도 균형 있게 닫는다. `inner=16`, `outer=24` 값 자체는 허용 범위다. |
| Flow term | 전역 `interStepPursuitMs`와 별도로 각 edge의 `transitionPursuitMs`를 저장ㆍpublishㆍServer 실행하도록 연결했다. 52개 pattern에는 51개 transition 값이 필요하다. 현재 두 번째 `VALTAN_FIST_IN_OUT` 직전 edge만 `100ms`, 나머지는 `1000ms`다. 전역 입력은 모든 edge를 일괄 변경하는 bulk edit임을 UI에 표시했다. |
| Server 실행 | bootstrap의 새 6번째 sequence field를 읽되 기존 5-field bootstrap도 허용한다. canonical debug flow는 Server가 실제로 로드한 edge별 wait를 사용하며, Valtan brain은 현재 step의 wait tick을 pattern 종료 뒤에 적용한다. Shared packet과 protocol version은 바꾸지 않았다. |
| 돌기둥 두 세트 | Six Pizza와 Struggling 두 세트의 visual-only rock pillar가 off-nav라는 이유로 Server entry 전체를 실패시키지 않도록 admission과 회귀 계약을 반영했다. |

`100ms`는 두 pattern의 **시작 시각 차이**가 아니라 앞 pattern이 끝난 뒤 다음 pattern으로 넘어가기 전 wait다.
따라서 앞 도넛 pattern 자체의 stage 시간이 있으면 실제 두 도넛 생성 시각 차이는 100ms보다 길 수 있다.

### 9.2 도넛 크기 identity 경계

현재 Flow의 두 occurrence는 모두 같은 stable pattern `VALTAN_FIST_IN_OUT`과 같은
combat object `combatobject.valtan.fist-in-out.donut`을 참조한다.
따라서 현재 정본의 `inner=16`, `outer=24`가 두 occurrence 모두에 적용되며,
첫 도넛 8–16m와 두 번째 도넛 16–24m처럼 서로 다른 크기로 실행되지는 않는다.

툴에는 같은 pattern occurrence가 반경을 공유한다는 경고를 추가했다.
서로 다른 두 크기를 제품 계약으로 만들려면 두 번째 stable pattern ID, 별도 combat-object ID,
BossCatalog projection, provenance receipt, Server hit 계약을 함께 추가하는 별도 수직 슬라이스가 필요하다.
특히 현재 Server 계약의 10m 대상 2회 hit 기대와 16–24m 단일 정의가 충돌할 수 있으므로,
단순히 occurrence별 숫자를 덧붙이는 방식으로는 완료 처리하지 않는다.

### 9.3 아직 반영하지 않은 항목

G05 워프 leg 재정합은 6.2의 A/B 선택에 따라 Server 모션 계약까지 달라지므로 이번 변경에 포함하지 않았다.
또한 PublishV2 Product, Server gameplay bootstrap, 빌드 산출물은 아래 사용자 실행 전까지 의도적으로
갱신되지 않은 상태다.

## 10. 반영 후 사용자 실행 순서

저장소 루트에서 아래 순서로 실행한다. 중간 단계가 실패하면 뒤 단계를 실행하기 전에 그 오류를 해결한다.

```powershell
# 1. Valtan split authoring 검증 및 Product 재생성
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2 -RepositoryRoot $PWD

# 2. Server gameplay bootstrap 재생성
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish

# 3. Effect V2 binding 정합성 검증
python Tools/EffectToolV2/validate_effect_v2.py --repository-root . --resource-root Client/Bin/Resources
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1 -RepositoryRoot $PWD -ResourceRoot Client/Bin/Resources

# 4. 제품 및 변경 domain 광역 회귀
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic

# 5. 최종 whitespace 검사
git diff --check
```

Effect V2 binding JSON은 자체가 runtime source이므로 별도의 Effect V2 publish 명령은 없다.
2~4단계가 끝나면 기존 Server 프로세스를 종료하고 새 Server를 시작한 뒤 Client를 실행해야
새 bootstrap과 새 binary가 같은 revision으로 동작한다. Client 화면과 이펙트 최종 판정은 사용자가 수행한다.

## 11. 실행 도구 오류 원인 분류

| 관찰되는 오류 | 원인 종류 | 확인 및 조치 |
|---|---|---|
| `transitionPursuitMs must contain ...` 또는 transition count invalid | source/schema | pattern N개에는 edge wait N-1개가 필요하다. 현재 기대값은 52/51이다. 배열 누락ㆍ중복 편집 여부를 확인한다. |
| transition pursuit milliseconds invalid | source/value | edge wait가 정수 `100..10000ms` 범위를 벗어났거나 숫자가 아니다. 해당 edge의 `Wait before next`를 고친다. |
| `split authoring Product drift` 또는 pattern rotation Product parity drift | generated Product | source를 저장한 뒤 PublishV2가 아직 실행되지 않았거나 중간 실패했다. Product JSON을 손으로 고치지 말고 1단계를 다시 실행한다. |
| high-jump spawn schedule/arena/random-circle validation 실패 | source/projection | gameplay와 Boss Tool projection의 volley 필드가 서로 다르다. `3/1333`, random boss-spawn `4`, `14m`, `1m`, max `36`인지 확인한다. |
| Effect V2 header/row, unknown group/leaf/resource 오류 | Effect V2 source/resource | binding JSON 형식이 깨졌거나 참조 leaf/group/resource가 없다. 제거한 shout-loop를 임의 복구하지 말고 STEP_07 burst 참조와 Resources 상대 경로를 확인한다. |
| `Gameplay.bootstrap` version/row count 또는 sequence transition pursuit invalid | Server bootstrap | 2단계 publish가 실패했거나 이전 bootstrap과 새 Server 코드가 섞였다. gameplay publish 후 Server를 다시 빌드ㆍ재시작한다. |
| C2660/C2511 등 함수 인자ㆍ선언 불일치 | source sync/compile | header와 cpp 중 일부만 반영된 상태다. 현재 브랜치 변경 전체를 동기화한 뒤 clean rebuild가 아니라 정상 순서의 FullDiagnostic을 다시 실행한다. |
| LNK1104/LNK1168 또는 exe/pdb access denied | process/file lock | 실행 중인 Client/Server가 출력물을 점유한다. 해당 프로세스를 정상 종료한 뒤 4단계를 다시 실행한다. |
| `pattern flow does not match the Server-active canonical scriptedSequence` | runtime revision drift | Client Product와 Server bootstrap/실행 프로세스 revision이 다르다. 1→2→4 순서로 다시 만들고 Server를 재시작한다. |
| `observed a disconnected server session`, socket 10054, Server entry failed | downstream disconnect | 이 문구는 원인이 아니라 Server 종료/연결 단절의 결과다. 같은 시각보다 앞선 Server assertionㆍcontractㆍbootstrap 로그를 먼저 본다. 이번 Flow 변경은 Shared packet/protocol을 바꾸지 않았으므로 우선 stale bootstrap/binary 또는 별도 Server runtime fault를 확인한다. |
| Ring Save에서 `abort() has been called` 재발 | Client assertion/runtime | 이번에 제거한 graph reload 후 use-after-free가 아닌 새 경로일 수 있다. `16/24`를 범위 오류로 단정하지 말고 최초 assertion과 call stack을 보존한다. |
| 빌드/검증은 통과하지만 도넛 간격ㆍ사자후ㆍ기둥이 기대와 다름 | manual presentation | 자동 검증은 시각 품질을 확정하지 않는다. Valtan에서 사용자가 직접 두 FIST의 종료 후 wait, STEP_07 one-shot burst, Six Pizza/Struggling 기둥 두 세트를 확인한다. |

## 12. 이번 세션의 검증 상태

변경 JSON parse와 `git diff --check`만 통과시켰다.
사용자 요청에 따라 PublishV2, gameplay publish, Effect validator, build/regression, Client 실행은 수행하지 않았다.
따라서 10절의 도구 실행과 사용자 수동 화면 확인 전에는 Product/runtime/visual PASS로 기록하지 않는다.
