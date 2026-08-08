# 발탄 아레나 World Destruction 저작 결과

대응 계획: `2026-08-07_VALTAN_WORLD_DESTRUCTION_PLAN.md`
최초 브랜치: `feature/maptool-world-destruction-bern-nav`
2026-08-08 간편 편집기 브랜치: `codex/maptool-wall-editor`

> 2026-08-08 문서 정합성 메모: 대응 PLAN은 현재 코드와 PhysX/Server 수직 슬라이스 기준으로 전면
> 개정됐다. 아래 0절은 2026-08-07 당시 “최초 PLAN보다 코드가 먼저 진행됐다”는 실제 결과 기록이며,
> 이번 계획서 개정이 C++ 구현 완료를 뜻하지는 않는다.

## 0. 계획과 실제 반영 범위가 다르다 — 먼저 읽을 것

PLAN 2절은 이번 G1을 **읽기 전용 진단 모드**로 못박고 "저장 경로, dirty 플래그, runtime 상태
변경을 만들지 않는다"고 적었다. PLAN 4절 파일 목록도 `CEncounterPatternReference` 한 클래스와
`MapTool.h` / `MapTool.cpp` 두 파일만 든다.

실제 반영된 코드는 그 경계를 넘었다.

```text
PLAN에 없는 것   Client/Public/WorldDestructionDocument.h
                 Client/Private/WorldDestructionDocument.cpp
                 CMapTool::Save_WorldDestruction / Apply_DestructionPreview / Try_PickDeployProp
                 Client/Public/DeployPropObject.h 의 Get_WorldBounds
PLAN 문자열 검사  grep -c "WorldDestructionDocument" PLAN = 0
```

그 결과 PLAN이 G1 종료 증거로 든 두 불변식은 현재 코드에서 성립하지 않는다.

| PLAN G1 불변식 | 현재 코드 |
|---|---|
| 저장 경로 0개 | `CMapTool::Save_WorldDestruction()` 존재 (`MapTool.cpp:3514`) |
| `Has_UnsavedAuthoring()` 이 이 모드 사용 후에도 false | `m_DestructionDocument.Is_Dirty()` 를 소비 (`MapTool.cpp:2234`) |

`Save_AllAuthoring()` 도 destruction 문서를 저장 대상에 넣는다 (`MapTool.cpp:580`).

따라서 이 RESULT는 **PLAN 5절을 코드 정본으로 취급하지 않는다.** 코드가 정본이고 PLAN에서
현재도 유효한 부분은 0절 실측과 1절 G 경계표뿐이다. G 경계표 기준으로 실제 반영 범위는
G1(진단) + G2(문서와 strict parser, atomic save) + G3(그룹 저작) + G4의 미리보기 일부다.
G5 이후는 착수하지 않았다.

PLAN 본문을 실제 코드에 맞춰 다시 쓰는 일은 이번 변경 단위에 넣지 않았다. 문서를 코드에
맞추는 교정은 다음 작업에서 별도로 한다.

## 1. 구현 완료

### 1.1 새 파일

| 파일 | 줄 수 | 역할 |
|---|---|---|
| `Client/Public/EncounterPatternReference.h` | 86 | Encounter pattern/stage 읽기 전용 참조 계약 |
| `Client/Private/EncounterPatternReference.cpp` | 314 | `Data/Encounters/Valtan/ValtanEncounter.json` strict parse |
| `Client/Public/WorldDestructionDocument.h` | 207 | 파괴 그룹·mutation·binding 저작 계약 |
| `Client/Private/WorldDestructionDocument.cpp` | 1084 | 위 계약의 parse / validate / save 구현 |

네 파일 모두 UTF-8 BOM 없음이고 non-ASCII 바이트가 0이다.

`CEncounterPatternReference`는 Save 함수를 갖지 않는다. 쓰기 정본은 Balance Tool이다.

`CWorldDestructionDocument`의 저작 대상은 `Data/Encounters/Valtan/ValtanWorldEvents.json`이고
schema는 `lostark.world-destruction-events`다 (`WorldDestructionDocument.cpp:17`).
저장하는 것은 안정 ID 관계뿐이다.

```text
DESTRUCTION_GROUP     groupId, memberPlacementIds(.deployplacements 의 runtimePlacementId),
                      navigationRegionIds, nav polarity, 초기 상태
DESTRUCTION_MUTATION  mutationId, groupId, 목표 상태, iBreakingDurationMs
DESTRUCTION_BINDING   bindingId, mutationId, patternId, stageId, trigger kind,
                      iOffsetMs, receiverCollisionId, isEnabled(기본 false)
```

deploy transform은 `.deployplacements`, navigation cell은 `.navblockers`, stage timing은
encounter profile이 계속 소유한다. 이 문서는 runtime 값을 갖지 않는다.

`DESTRUCTION_NAV_POLARITY`가 그룹마다 따로 있는 이유는 publish된 Valtan navgrid가 deploy
footprint를 전부 walkable로 굽기 때문이다. 온전한 벽은 blocker를 **추가**해야 하고 아레나
바닥은 반대로 무너질 때 막아야 한다. bool 하나로는 두 방향을 표현할 수 없다.

### 1.2 수정 파일

| 파일 | 증감 | 내용 |
|---|---|---|
| `Client/Public/MapTool.h` | +51 | `TOOL_MODE::WORLD_DESTRUCTION`, descriptor의 `encounterReference` / `worldEventsDocument`, 패널 9개와 저작 함수 7개 선언, 저작 상태 멤버 |
| `Client/Private/MapTool.cpp` | +1206 / -1 | 모드 라디오, 패널 switch, 패널 구현, Valtan descriptor 확장, 저장·미리보기·피킹 |
| `Client/Public/DeployPropObject.h` | +8 | `Get_WorldBounds()` 선언 |
| `Client/Private/DeployPropObject.cpp` | +61 | 현재 렌더 중인 model의 local bounds를 배치 transform으로 회전·스케일해 world AABB로 환산 |
| `Client/Default/Client.vcxproj` | +4 | 새 `.h` 2개, `.cpp` 2개 등록 |
| `Client/Default/Client.vcxproj.filters` | +12 | 위 4개를 `03. Tools\00. Map` 필터에 등록 |

`Get_WorldBounds()`는 저작 도구가 prototype tag로 다시 clone하지 않고 prop을 hit-test하고
외곽선을 그리기 위한 경계다. baked local bounds가 없으면 `false`를 돌려 호출자가 0 크기
상자를 쓰지 않고 해당 prop을 건너뛰게 한다.

### 1.3 MapTool에 추가된 함수

```text
Load_EncounterReference        MapTool.cpp:1449
Load_WorldDestruction          MapTool.cpp:3480
Save_WorldDestruction          MapTool.cpp:3514
Try_PickDeployProp             MapTool.cpp:3534
Refresh_DestructionHighlight   MapTool.cpp:3576
Apply_DestructionPreview       MapTool.cpp:3646
Render_DestructionGroupEditor  MapTool.cpp:3675
Render_DestructionBindingEditor MapTool.cpp:3891
Render_DestructionTimeline     MapTool.cpp:4074
Render_WorldDestructionPanel   MapTool.cpp:4178
Render_DestructionEncounterSource MapTool.cpp:4212
Render_DestructionDeployList   MapTool.cpp:4340
Render_DestructionWorldRows    MapTool.cpp:4418
Render_DestructionNavigationRegions MapTool.cpp:4461
Render_DestructionDiagnostics  MapTool.cpp:4511
```

`worldEventsDocument`는 Valtan descriptor에만 설정된다 (`MapTool.cpp:2178`). 다른 Area는
경로가 비어 있어 `Get_WorldDestructionPath()`가 빈 경로를 돌려주고 패널이 저작 없음으로
표시한다.

## 2. 자동 검증 (실행함)

| 항목 | 결과 |
|---|---|
| Client x64 Debug 빌드 | 성공. MSBuild exit 0, `Client\Bin\Debug\Client.exe` 링크 |
| 새 파일 컴파일 | `EncounterPatternReference.cpp`, `WorldDestructionDocument.cpp` 둘 다 컴파일 로그에 나옴 |
| 신규 경고 | 없음. `C4819`(CP949 코드페이지)와 `LNK4099`(DirectXTK PDB)는 이번 변경 이전부터 있던 것 |
| 새 파일 인코딩 | 4개 모두 UTF-8 BOM 없음, non-ASCII 0바이트 |
| `git diff --check` | clean |
| `Publish-WorldGameplay.ps1 -Mode Validate` | 통과. BERN 7 / VALTAN_ARENA 12 / TRAINING_GROUND 4 / CHARACTER_SELECT_ARENA 5 placements, VALTAN_ARENA 3 spawn groups |
| `Publish-ServerNavigation.ps1 -Mode Validate` | 통과 |
| ProjectAudit `projects.data-source-visibility` | expected=545 project=545 filters=545 |
| ProjectAudit 전체 | **실패 1건.** 아래 2.1 |

### 2.1 남아 있는 ProjectAudit 실패는 사전 존재

```text
Project audit failed (1): effect.g09-authoring-world-runtime-boundary:
  paths=0 authoredUnexpected=0 intake=2 shaders=1 symbols=0 project=0
  entry=False detailPreview=True
```

Effect Tool 저작/런타임 경계 검사이며 이번 변경은 Effect 파일을 하나도 건드리지 않았다.
같은 실패가 `2026-08-07_MAPTOOL_NAVIGATION_BOOTSTRAP_GATE_RESULT.md` 4.1절에도 baseline
실패로 기록돼 있다. 별도 담당 작업으로 남긴다.

이 작업 전에는 `projects.data-source-visibility`도 함께 실패했다
(`expected=545 project=543 filters=543`). 원인은 Bern navigation 저작본 두 개가
`Client.vcxproj`에 등록되지 않은 것이었고, navigation 관문 커밋에서 `96.DataFiles\Navigation`
`None` 항목으로 등록해 해소했다.

## 3. 수동 검증 (미실행 — 사용자 확인 대기)

Client는 GUI이므로 아래는 실행해서 눈으로 확인해야 한다. **PASS로 기록하지 않는다.**

PLAN 7.3~7.5의 절차 전부가 여기 해당한다.

```text
1. Server + Client 실행 -> Lobby -> Test -> LEVEL::DEVELOPMENT -> F1 -> Map Tool
2. Valtan 선택 후 status commit 대기
3. 모드 바가 Map Assets | World Gameplay | World Destruction | Navigation | Camera 다섯 개인지
4. Reload Encounter Reference 로 31 pattern, Deploy 85행이 보이는지
5. 손상된 ValtanEncounter.json 에서 표가 유지되고 상태 문자열만 실패로 바뀌는지
```

PLAN이 다루지 않는 새 경로는 검증 절차 자체가 PLAN에 없다. 최소한 다음은 별도로 확인해야 한다.

```text
그룹 생성 -> 멤버 추가 -> Save -> Area 재진입 -> Load 왕복이 동일한지
잘못된 문서에서 기존 문서가 보존되는지
Apply_DestructionPreview 의 INTACT/FRACTURED 전환이 화면에 보이는지
Try_PickDeployProp 의 선택이 Get_WorldBounds 결과와 맞는지
Save 하지 않고 Area 를 바꿀 때 Has_UnsavedAuthoring 이 물어보는지
```

## 4. 남은 경계

### 4.1 첫 Save 프로젝트 등록 — 2026-08-08 해결

`Data/Encounters/Valtan/ValtanWorldEvents.json` 빈 정본을 추가했고 schema/version/Area/
Encounter/provenance와 빈 group/mutation/binding 배열을 저장했다.

같은 변경 단위에서 다음 `96.DataFiles\Encounters` 항목을 프로젝트와 filters에 등록했다.

```xml
<None Include="..\..\Data\Encounters\Valtan\ValtanWorldEvents.json" />
```

```xml
<None Include="..\..\Data\Encounters\Valtan\ValtanWorldEvents.json">
  <Filter>96.DataFiles\Encounters</Filter>
</None>
```

### 4.2 publisher / Server / Shared 미연결

`Publish-WorldGameplay.ps1:401-402`는 placement kind를
`playerSpawn, npc, boss, triggerBox, collisionBox`로만 허용하고 그 외를 던진다.
`destroyable`은 여전히 fail-closed다. publisher는 `ValtanWorldEvents.json`을 읽지도 않는다.

따라서 지금 저작한 내용은 Server 런타임에 아무 영향을 주지 않는다. G5(publisher 검증),
G6(Server 상태와 collision receiver), G7(Shared 복제), G8(Client `Set_State` 서버 연결)이
닫히기 전에는 "벽이 실제로 부서진다"고 판단하지 않는다.

### 4.3 착수하지 않았거나 일부만 열린 항목

```text
G9   데이터 시간축 Play/Pause/Restart/현재 시점 복사는 구현. 실제 boss clip 동기 재생은 미구현
G10  돌진 충돌 판정과 스킬 이동 sweep 보강
G11  아레나 바닥 붕괴와 낙사 hazard
G12  파괴 Effect / Camera / Audio
G13  나머지 그룹 확장
```

### 4.4 팀 문서

PLAN 8절대로 G1 단계에서는 팀 문서를 갱신하지 않는다. publisher가 `destroyable`을
admission하는 G5에서 `AREA_DATA_LAYER_GUIDE.md`,
`UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md`, `BALANCE_TOOL_OWNER_HANDOFF.md` 세 곳을 같은
변경 단위에서 교정한다.

## 5. 다음 작업에서 먼저 할 것

```text
1. 아래 6.4절 수동 검증을 실행하고 결과를 이 문서에 채운다
2. navigation blocker region을 실제 벽 그룹에 저작한다
3. G5 publisher 검증부터 순서대로 열어 Server 수직 슬라이스를 닫는다
```

## 6. 2026-08-08 벽 중심 간편 편집기 결과

### 6.1 구현 완료

- `World Destruction`의 기본 화면을 `Easy Wall Editor`로 연결하고 기존 원본 그래프 UI는
  `Advanced Graph Editor`로 보존했다.
- 월드 클릭과 Deploy 목록 클릭을 `Select_DestructionWall()` 하나로 통합했다. 선택 성공 시
  owner group 역조회, 기존 binding draft 동기화, 선택 벽/그룹 와이어 강조가 함께 갱신된다.
- pick 실패는 렌더 표면 없음과 DeployProp 소유권 없음으로 나눠 상태 문자열을 남긴다.
- 선택 모드의 LMB는 gameplay 입력으로 전달하지 않고 Escape/모드 전환에서 해제한다.
- 선택 벽 Inspector에 asset, runtime placement ID, 위치, stateOff/trigger 원본 근거, 모델 종류,
  fractured mesh 유무, 그룹과 함께 무너지는 벽 수를 표시한다.
- INTACT/FRACTURED/DESPAWNED 프리뷰 버튼은 실제 적용 범위를 Wall/Group으로 표시한다.
- pattern/stage/actionId/kind/duration을 선택하고 stage 시작/시간/종료/Collision Box 충돌 중 하나를
  고를 수 있다. 정확한 actionId별 clip이 없다는 경고를 같은 화면에 표시한다.
- 30Hz encounter 데이터 시간축에 Play/Pause/Restart/Loop/slider/Use Current Time을 추가했다.
  실제 CValtan 모델 clip 재생이 아니라 파괴 offset 저작용 playhead다.
- 간편 Apply는 document copy에 전부 stage한 뒤 commit한다. group identity 기반 deterministic
  mutation/binding ID, semantic 중복 방지, hash 충돌 fail-closed를 적용했다.
- Save 전에 모든 group member DeployProp, navigation region, pattern/stage/offset, collision receiver
  외부 참조를 다시 검증한다. 새 binding은 기본 disabled다.
- 여러 setting이 있는 그룹은 Existing/New을 명시하기 전까지 Apply할 수 없고, 기존 setting을
  편집하면 같은 binding ID를 갱신한다. Easy Editor는 FRACTURED setting만 다룬다.
- 비파괴 DeployProp은 벽 목록·월드 선택·그룹 추가에서 거부한다. Preview는 원래 상태를 기억해
  다른 벽 선택, 모드 전환, F1 닫기와 ImGui 창의 `X` 닫기에서 자동 복원한다.
- enabled setting은 실제 벽 멤버, 단일 소유 navigation region과 1개 이상의 cell, 활성 Collision
  Box를 요구한다. 미저장 World Gameplay/Navigation을 참조한 단독 Save도 차단한다.
- Reload와 Area 전환은 Deploy/Gameplay/Navigation/Encounter와의 cross-document 검증을 먼저
  통과한 뒤 staged 문서를 commit한다.
- `Reload Authoring Data`는 Encounter와 WorldEvents를 함께 stage/validate/commit한다. Encounter
  단독 Reload도 기존 world-events가 계속 유효할 때만 교체하므로 두 정본이 반쪽만 갱신되지 않는다.
- enabled setting은 `BOSS_VALTAN`과 `ENCOUNTER_VALTAN`이 정확히 연결된 boss placement가 하나일
  때만 저장한다.
- `Save All`, 개별 Gameplay 저장/재로드, Navigation 저장/재로드도 기존 world-events 참조를
  깨뜨리는 Collision Box 삭제·blocker 삭제·0-cell 변경을 파일 쓰기 전에 거부한다.
- 빈 `ValtanWorldEvents.json` 정본과 `Client.vcxproj/.filters` 데이터 노출 항목을 함께 추가했다.

### 6.2 자동 검증

| 항목 | 결과 |
|---|---|
| JSON / vcxproj / filters parse | PASS |
| `git diff --check` | PASS |
| Client x64 Debug | PASS, MSBuild exit 0, `Client/Bin/Debug/Client.exe` 링크 |
| Client x64 Release | PASS, MSBuild exit 0, `Client/Bin/Release/Client.exe` 링크 |
| `Publish-WorldGameplay.ps1 -Mode Validate` | PASS, 4 world + Valtan spawn groups |
| `Publish-ServerNavigation.ps1 -Mode Validate` | PASS, Valtan/Training/Character Select |
| ProjectAudit | 기존 `effect.g09-authoring-world-runtime-boundary` 1건만 FAIL. 이번 파일 관련 신규 실패 없음 |

ProjectAudit 첫 실행의 `python` 미탐지는 Codex 번들 Python 경로를 연결해 재실행했고 제거됐다.
남은 Effect Tool 실패는 이 작업 전 RESULT 2.1절에도 기록된 baseline이다.

### 6.3 아직 제품 런타임이 아닌 것

이번 변경은 사용자가 파괴 데이터를 쉽게 저작하는 화면과 정본 저장까지다.
`Publish-WorldGameplay.ps1`은 여전히 destroyable을 admission하지 않으며 Server/Shared/Client
replication, BREAKING→FRACTURED commit, 동적 collision/nav, late join, 돌진 receiver, 파괴 Effect/
Camera/Audio는 연결되지 않았다. 따라서 Easy Editor에서 저장했다고 실제 레이드 벽이 바로
무너지는 것은 아니다.

### 6.4 수동 검증 대기

```text
Lobby -> Test -> Map Editor -> Valtan -> F1 -> World Destruction
Easy Wall Editor가 기본으로 보이는지
Pick Wall In Viewport와 왼쪽 목록이 같은 벽/그룹을 강조하는지
Original/Broken/Hidden 프리뷰와 Group 범위 문구가 맞는지
pattern/stage/time을 고른 뒤 Apply And Save가 JSON을 만들고 Reload 후 동일한지
Play/Pause/Restart/Use Current Time이 stage와 offset을 올바르게 갱신하는지
잘못된 Collision Box/외부 참조에서 기존 파일을 보존하고 저장을 거부하는지
```

GUI 수동 검증은 이 세션에서 실행하지 않았으므로 PASS로 기록하지 않는다.

---

## 7. 2026-08-08 MapTool authoring PhysX slice 결과

작업 브랜치: `codex/valtan-maptool-physics-sim`

이 절은 PLAN 13절의 tool-only slice 실제 결과다. 제품 `destroyable` admission은 열지 않았고,
MapTool에서 맵 담당자가 방향·속도·중력·수명·조건을 직접 audition할 수 있는 물리 경로만 닫았다.

### 7.1 구현 완료

- PhysX 5.6.1(`107.3-physx-5.6.1`) CPU SDK를 구성별 lib/DLL과 함께
  `Engine/ThirdPartyLib/PhysX`에 고정했다. public H에는 PhysX 타입을 노출하지 않는다.
- `CPhysics_Manager`와 `CRigidBody`가 generation actor handle, static/dynamic actor, box/sphere/capsule,
  pose/velocity/force, gravity policy, fixed 1/60 step, paused deterministic steps와 level clear를 제공한다.
- Engine frame에 `Post_Physics_Update`를 추가해 simulation 뒤 visible transform을 pull한다.
- `CDestructionSimulationDocument`, `Runtime`, `Controller`를 추가했다. controller가 Play, Pause,
  Restart, Loop, 1/60 Step, reset 후 fixed-step Seek, All/Solo와 collision preview의 단일 clock이다.
- 기존 `CDeployPropRuntime -> CDeployPropObject -> CModel`을 그대로 사용한다. actor root와 visible root를
  공유하고 scaled local bounds center는 `ShapeLocalPose`로 분리해 collider 중심 관통과 world-AABB
  이중 회전을 제거했다.
- ANIM DeployProp은 preview 동안 실제 frame delta를 소비하지 않고 physics fixed step과 같은 tick에서
  logical `off` clip을 전진한다. Reset/Seek는 clip 0부터 다시 계산하고 종료 시 기존 clip/track을 복원한다.
- MapTool 실제 Development world 위 `Destruction Model View`에 Stage, Play/Pause, Restart, Loop,
  timeline/marker, 1/60 Step, All Debris, Solo Selected, manual collision fire를 추가했다.
- Detail local draft는 spawn offset, normalized direction, speed, derived velocity, gravity scale, lifetime,
  Immediate/Timeline/Collision trigger를 live audition하고 Apply에서만 document를 dirty로 만든다.
- `ValtanWorldEvents.json`의 group `3705102` 다섯 placement와 simulation 다섯 element를 정확히 맞췄다.
  제품 collision receiver가 아직 없으므로 초기 binding은 disabled `STAGE_TIME` 250ms로 두고,
  실제 Collision Box가 저작된 뒤에만 `COLLISION_IMPACT`로 전환한다.
- WorldEvents와 simulation은 cross-validation 뒤 하나의 pair transaction으로 저장한다. 두 번째 canonical
  replace가 실패하면 첫 파일을 byte-exact 원본으로 rollback하고 sidecar를 정리한다.

### 7.2 자동 검증

| 검증 | 결과 |
|---|---|
| Engine x64 Debug / Release | PASS |
| `UpdateLib.bat Debug / Release` | PASS, PhysX/PhysXCommon/PhysXFoundation DLL 배포 |
| PhysicsContractHarness Debug / Release | PASS, 180 fixed steps, local shape pose, settle, clear invalidation |
| destruction document 정상/negative/cross-ref/단일-file rollback | PASS |
| WorldEvents + simulation pair commit/두 번째 파일 잠금 rollback | PASS, 두 원본 byte-exact 보존·sidecar 0 |
| Shared + NetworkProtocolHarness Debug / Release | PASS, failures 0 |
| Server Debug / Release + `--contract-test` | PASS, failures 0 |
| Client x64 Debug / Release | PASS, `Client.exe` 링크 |
| ProjectAudit | PASS, 81 checks |
| JSON/XML parse, MapTool CP949+CRLF, `git diff --check` | PASS |

빌드 로그의 `C4819`와 Release DirectXTK `LNK4099`는 기존 인코딩/PDB 경고이며 새 컴파일 오류는 없다.

### 7.3 수동 인게임 검증 — visible mesh emitter 확인, 나머지 진행 중

자동 검증 뒤 통합 Server/Client를 실행했다. 사용자는 Valtan Area와 새 Destruction Model View에서
project-authored stone mesh가 PhysX로 날아가는 결과를 눈으로 확인했다. 따라서 visible mesh emitter
재생은 PASS다. 아래 Save/Reload, collision manual fire와 모든 lifecycle 조합은 아직 각각 PASS로
확정하지 않는다.

```text
1. Client/Bin/Debug/Client.exe를 Client/Default 작업 디렉터리에서 실행
2. Lobby -> Test -> LEVEL::DEVELOPMENT
3. F1 -> Map Tool -> Valtan -> World Destruction
4. group destroyable.group.valtan.wall.3705102 선택
5. Destruction Model View -> Play All Fragments
6. All Fragments / Solo Emitter / Solo Fragment, Pause, 1/60 Step, Restart, timeline Seek 확인
7. Detail에서 direction/speed/gravity/lifetime을 바꾸고 live audition 확인
8. Apply Detail 뒤 Save All, Area 재진입 후 값 유지 확인
9. Collision trigger element에서는 Fire Collision로 발화 확인
10. tool close/Area switch에서 벽 pose와 physics actor가 원상복귀하는지 확인
```

### 7.4 의도적으로 미완료인 제품 경계

`Publish-WorldGameplay.ps1`은 `Gameplay.world.json kind=destroyable`을 계속 fail-closed로 거부한다.
이번 simulation JSON은 publisher 입력이 아니므로 MapTool audition은 그 거부 대상이 아니다. Server
persistent state, dynamic collision/navigation, Valtan 실제 charge impact, Shared full/delta와 제품 Client
debris presentation은 PLAN의 `WD-G05`~`WD-G08`에서 함께 구현하기 전까지 완료로 판단하지 않는다.

### 7.5 Git 상태

`codex/valtan-brick-debris-preview`는 작업 시작 시 공개 `origin/main` `52dd7e8`로 fast-forward한 뒤
MapTool/PhysX 변경만 다시 적용했다. Primary Effect dirty worktree나 다른 세션의 미커밋 파일은 이 branch에
복사하지 않는다. main 직접 push 대신 이 검증 단위를 commit/push하고 PR로 동기화한다.

### 7.6 Valtan Area 진입 회귀 원인과 방지

- Git 제외 대상인 `Client/Bin/Resources/Map/LV_LUT_HEARTRB_ED`에서 catalog 요구 파일 654개
  (`.wmodel` 235, `.dds` 407, 기타 12)가 빠져 strict MapAsset stage가 첫 누락 모델에서 rollback됐다.
- 팀 runtime pack에서 **없는 파일만** 보충해 기존 파일을 덮지 않았고, catalog 요구 누락을 0으로 만들었다.
- 상대 작업의 빈 `ValtanWorldEvents.json`은 simulation `groupId`를 해석하지 못한다. 반대로 존재하지 않는
  collision ID를 binding에 남기면 external-reference validation에서 실패한다. 통합 데이터는 5-member group과
  `STAGE_TIME` 250ms / 빈 receiver를 사용한다.
- MapTool workspace bar에 Area stage 실패 status를 항상 표시해 다음 담당자가 조용한 실패로 오인하지 않게 했다.
- 잘못된 별도 worktree EXE를 실행하면 Primary의 미커밋 Effect/Data/Shader를 읽지 않아 외형·성능이 회귀해
  보일 수 있다. 통합 smoke는 실행 파일 경로, 작업 디렉터리, `LOSTARK_RESOURCE_ROOT`를 함께 고정한다.

## 8. 2026-08-09 project-authored Mesh Debris Emitter 결과

### 8.1 해결한 문제

이전 preview는 simulation element 하나를 Deploy placement 전체 actor 하나로 처리했기 때문에
`ITR_02306` 기둥 5개가 통째로 움직였다. `ITR_02306_FRACTURED.wmodel` 내부에는 visible toggle로 꺼진
작은 벽돌이 없으므로 해당 모델의 visibility나 animation을 바꿔서는 원하는 결과가 나오지 않는다.

해결 구조는 source placement 하나를 하나의 Wall Mesh Emitter로 유지하고 그 아래 12개의
project-authored proxy fragment를 파생하는 것이다.

```text
Profile 1개
  -> Wall Mesh Emitter 5개
  -> Emitter당 fragment 12개
  -> 최대 CModel proxy 60개 + PhysX actor 60개
```

source wall은 원래 위치에서 FRACTURED presentation을 유지한다. 네 Valtan stone WModel을 기존
`CModel -> CMaterial` 경로로 clone하고 각각 별도 `CRigidBody` dynamic box actor를 연결한다. 기본
diffuse/emissive가 없는 binary material은 `CMaterial`의 solid gray diffuse fallback을 사용한다.

### 8.2 UI와 runtime 계약

- `Profile -> Wall Mesh Emitter -> fragment.00~fragment.11` 트리를 추가했다.
- `Play All Fragments`, `Solo Emitter`, fragment별 `Solo + Play`를 추가했다.
- fragment stable ID는 `<elementId>.fragment.NN`이며 vector index나 Prototype tag를 저장하지 않는다.
- fragment frame은 model asset, WAITING/ACTIVE/EXPIRED/FILTERED, life, pose, velocity를 제공한다.
- `SOLO_FRAGMENT`는 선택 actor 하나만 만들고 다른 11개는 `FILTERED`로 유지한다.
- 네 proxy prototype은 0.01 asset pretransform 뒤 3.5 preview scale로 admit한다.
- piece별 0.8~1.2 scale, spread/up/angular velocity는 stable seed에서 결정돼 Restart/Seek가 재현된다.
- fragment sample은 read-only이고 v1 JSON에는 emitter 공통 direction/speed/gravity/lifetime/trigger만 저장한다.

### 8.3 Trigger와 Effect 경계

`IMMEDIATE`와 `TIMELINE_TIME`은 controller fixed timeline이 발화한다. `COLLISION_IMPACT`는 현재
MapTool의 `Request_Collision -> Notify_Collision` 수동 audition만 구현됐다. `ValtanWorldEvents.json`의
preview binding은 실제 collisionBox가 없으므로 disabled `STAGE_TIME` 250ms와 빈 receiver를 유지한다.

원본 `FX_ITR_02315.Par_G_Fracture_Dust_02_01`은 아직 미복구다. 현재 돌은 Effect particle이 아니라
static CModel + PhysX actor다. exact dust가 복구되면 Effect Tool private runtime을 가져오지 않고
controller sample time으로 `CEffectObject::Reset/Set_SampleTime`을 구동하는 별도 effect lane을 붙인다.
제품에서는 Server live event가 stable cue ID, impact origin/direction과 seed만 확정한다.

### 8.4 이번 변경에서 실행한 검증

| 검증 | 결과 |
|---|---|
| 공개 main 동기화 | `HEAD == origin/main == 52dd7e8`에서 변경 적용 |
| Engine x64 Debug | PASS |
| `UpdateLib.bat Debug` | PASS, PhysX DLL 배포 |
| Server x64 Debug + `--contract-test` | PASS, failures 0 |
| Client x64 Debug | PASS, MapTool/Runtime/Controller/DeployProp 실제 컴파일·링크 |
| PhysicsContractHarness Debug | PASS, physics-and-destruction.contract |
| ProjectAudit | PASS, 81 checks |
| JSON/XML parse, `git diff --check` | PASS |
| 사용자 GUI | Valtan에서 visible mesh debris playback 확인 |

이번 follow-up에서는 Release를 다시 빌드하지 않았다. 7.2절의 이전 Release 증거와 이번 Debug/main
integration 증거를 구분한다.

### 8.5 맵 담당자 다음 순서

1. 실제 collisionBox와 nav blocker region을 저작한다.
2. simulation receiver와 WorldEvents receiver를 같은 stable ID로 연결한다.
3. `COLLISION_IMPACT`를 MapTool manual fire로 먼저 검증한다.
4. 원본 dust Effect와 dependency를 복구해 controller-owned effect lane에 stage한다.
5. 다른 destroy asset은 source 근거를 조사한 뒤 exact 또는 `PROJECT_AUTHORED` recipe로 추가한다.
6. publisher/Server/Shared/제품 Client를 한 수직 슬라이스로 닫은 뒤에만 destroyable gate를 연다.

상세 호출 지점과 금지 경계는 `.md/TEAM/MAP_DESTRUCTION_PHYSX_HANDOFF.md`를 따른다.
