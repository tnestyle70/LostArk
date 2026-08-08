# Map Destruction PhysX·Mesh Debris 인계서

이 문서는 Valtan MapTool의 `World Destruction -> Destruction Model View`를 이어서 작업하는 담당자의
실행 입구다. Area별 파일 존재 여부와 publisher 지원 범위는 `AREA_DATA_LAYER_GUIDE.md`, 제품 권위
경계는 `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, 구현·검증 이력은
`../GB/08-07/2026-08-07_VALTAN_WORLD_DESTRUCTION_RESULT.md`가 정본이다.

## 1. 현재 완료 상태

현재 완료된 것은 **Debug MapTool authoring preview**다. 제품 레이드 파괴 권위는 아직 열지 않았다.

```text
Destruction Profile
└─ Wall Mesh Emitter (Deploy placement 1개)
   ├─ fragment.00 -> CModel proxy 1개 -> CRigidBody 1개
   ├─ fragment.01 -> CModel proxy 1개 -> CRigidBody 1개
   ├─ ...
   └─ fragment.11 -> CModel proxy 1개 -> CRigidBody 1개
```

현재 Valtan 검증 profile은 `DEPLOY_ITR_02306` 다섯 placement다. 따라서 All Fragments 재생 시 최대
`5 emitters * 12 fragments = 60`개의 presentation-only dynamic actor가 생성된다.

완료된 기능:

- Profile 전체 `Play All Fragments`
- Wall Emitter 단위 Solo
- `fragment.00`~`fragment.11` 단위 `Solo + Play`
- fragment별 stable ID, 사용 model asset, 상태, lifetime, position, velocity 표시
- Play/Pause/Restart/Loop, 정확히 1/60초 single step
- Reset 뒤 같은 fixed step을 다시 계산하는 deterministic seek
- direction, speed, spawn offset, gravity scale, lifetime, trigger local draft와 Apply/Revert
- trigger `IMMEDIATE`, `TIMELINE_TIME`, `COLLISION_IMPACT`의 MapTool audition
- tool close, mode/Area switch, stage 실패에서 actor와 source presentation 원상 복원

제품에서 아직 완료되지 않은 기능:

- `Gameplay.world.json kind=destroyable` publisher admission
- Server `INTACT -> BREAKING -> FRACTURED -> DESPAWNED` 권위 상태
- 실제 Valtan charge와 receiver collision 판정
- 파괴 commit tick의 collision/navigation 원자 전환
- Shared full/delta와 late join
- 제품 Client의 one-shot debris/effect cue
- 원본 source particle `FX_ITR_02315.Par_G_Fracture_Dust_02_01` 복구

## 2. 처음 5분 실행 절차

Client 시작 Level은 항상 Lobby고 `Test`의 Debug Map Editor 진입도 최초 Server 승인이 필요하다.
같은 PC에서는 Server를 `127.0.0.1:7777`로 먼저 실행한다.

```text
Lobby -> Test -> LEVEL::DEVELOPMENT
F1 -> Map Tool
Area -> Valtan
World Destruction
Destruction Model View
```

패널에서 다음 순서로 확인한다.

```text
1. profile destroyable.group.valtan.wall.3705102.preview 선택
2. Play All Fragments
3. Profile -> Wall Mesh Emitter 트리 확장
4. fragment.00~fragment.11의 WAITING -> ACTIVE -> EXPIRED 확인
5. 한 fragment의 Solo + Play
6. 오른쪽에서 model/state/life/position/velocity 확인
7. Emitter Authoring의 direction/speed/gravity/lifetime 변경 후 audition
```

`Stage Selected (Paused)`는 의도적으로 0초에서 멈춘다. 재생 확인은 `Play All Fragments`를 누르거나
Stage 뒤 `Play`를 누른다. 기본 `TIMELINE_TIME`은 0.25초이므로 첫 15 fixed step 동안 `WAITING`인 것이
정상이다.

## 3. 자산을 잘못 이해하면 안 되는 이유

### 3.1 `DEPLOY_ITR_02306`

`ITR_02306`은 한 모델 내부의 invisible 벽돌을 visible로 바꾸는 자산이 아니다. deploy catalog가
물리적으로 별도인 두 WModel을 가리킨다.

```text
.../DEPLOY_ITR_02306/intact/DEPLOY_ITR_02306_INTACT.wmodel
.../DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_FRACTURED.wmodel
```

`CDeployPropObject`는 `Com_Model_Intact`와 `Com_Model_Fractured`를 함께 보유하고 state에 따라 하나만
렌더한다. fractured WModel도 작은 brick fragment collection이 아니라 기둥 전체 static CModel이다.
그러므로 `visible=true`, submesh toggle, `Play_Animation()`은 해결책이 아니다.

### 3.2 `DEPLOY_ITR_02326`

`ITR_02326`은 `ITR_02306`의 숨은 debris가 아니라 별도 ANIM deploy다. `b_piece_00`~`b_piece_14`가
동일 skinned model의 bone influence geometry이고 `ao_off`는 약 1.9초 뒤 scale을 0으로 만든다. 현재
MapTool proxy recipe는 이 자산을 사용하지 않는다. 향후 이 15개를 정확한 rigid fragment로 쓰려면
offline split/cook 또는 bone별 fragment renderer가 별도 수직 슬라이스로 필요하다.

### 3.3 현재 보이는 작은 돌

원본 particle이 아직 복구되지 않았기 때문에 현재 작은 돌은 명시적인 `PROJECT_AUTHORED` proxy다.

```text
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel
```

MapTool은 네 모델을 기존 `CModel -> CMaterial` 경로로 한 batch에 admit한다. 일부만 성공하는 상태는
commit하지 않는다. asset pretransform 0.01 뒤 preview recipe scale 3.5를 적용해 아레나 카메라에서도
확인 가능한 크기로 만든다. 누락·손상 시 Valtan Area 전체가 아니라 debris audition만 unavailable로
격리된다.

## 4. 데이터와 stable ID

### 4.1 World event graph

정본:

```text
Data/Encounters/Valtan/ValtanWorldEvents.json
```

현재 seed:

```text
groupId    destroyable.group.valtan.wall.3705102
mutationId mutation.valtan.wall.3705102.break
bindingId  binding.valtan.wall.3705102.preview
patternId  VALTAN_ARMOR_BREAK_OPENING
stageId    WALL_CHARGE
trigger    STAGE_TIME + 250 ms
enabled    false
```

group member는 다섯 deploy runtime placement ID다. 실제 collisionBox가 아직 없기 때문에 receiver는
빈 문자열이고 binding은 disabled다. 존재하지 않는 receiver ID를 먼저 적어 두지 않는다.

### 4.2 Physics audition document

정본:

```text
Data/Maps/Authoring/LV_LUT_HEARTRB_ED/
  LV_LUT_HEARTRB_ED.destructionsimulation.json
```

format v1에서 element 하나는 source Deploy placement 하나다. 저장 ID는 다음과 같다.

```text
profileId  destroyable.group.valtan.wall.3705102.preview
elementId  debris.<decimal sourceRuntimePlacementId>
fragmentId <elementId>.fragment.00 ... fragment.11
```

fragment ID와 12-piece recipe는 runtime이 stable element ID와 ordinal로 결정한다. fragment를 JSON의
중복 placement element로 저장하지 않는다. 이 규칙 덕분에 WorldEvents group member 집합과 simulation
element placement 집합은 계속 1:1로 cross-validate된다.

WorldEvents와 simulation은 pair transaction으로 저장한다. 두 번째 canonical 교체에 실패하면 첫 파일도
byte-exact 원본으로 rollback하고 sidecar를 정리한다.

## 5. 실제 호출 흐름

```text
ImGui button
  -> CMapTool이 Request_* 명령만 latch
  -> 다음 CMapTool::Update
  -> CDestructionSimulationController::Consume_Commands
  -> CDestructionSimulationRuntime::Stage_Profile / Advance_Timeline
  -> CDeployPropObject::Begin_PhysicsPreview(FRACTURED)
  -> CDeployPropObject::Begin_DebrisPreview(12 proxy CModel)
  -> CRigidBody::Create_Runtime(12 dynamic box actors)
  -> direction * speed + deterministic spread/up/angular velocity
  -> CPhysics_Manager::Simulate_DebugSteps(1)
  -> CDestructionSimulationRuntime::Post_Physics_Update
  -> CDeployPropObject::Apply_DebrisPreviewPose
  -> 기존 CModel/CMaterial render path
```

ImGui Render 중 actor를 만들거나 world state를 바꾸지 않는다. controller가 debug-paused PhysX scene의
유일한 preview clock이며 Play, single step, seek가 모두 1/60초 `Step_Once()`를 통과한다.

activation은 all-or-nothing이다. 12 proxy clone 또는 12 actor 중 하나라도 실패하면 생성한 actor를 모두
제거하고 debris preview를 끝낸다. 성공하면 source는 제자리의 FRACTURED presentation으로 남고 proxy만
각 actor pose를 따라 날아간다. lifetime 뒤 proxy actor/visual만 만료되고 Reset/Clear에서 source state,
transform, animation track을 정확히 복원한다.

## 6. All/Solo 의미

| Scope | 입력 ID | 생성되는 actor |
|---|---|---|
| `ALL_DEBRIS` | 없음 | profile의 모든 emitter * 12 |
| `SOLO_SELECTED` | element ID | 선택 wall emitter의 12개 |
| `SOLO_FRAGMENT` | `<element>.fragment.NN` | 정확히 선택 fragment 1개 |

scope는 authoring 문서를 바꾸지 않는 preview projection이다. Solo를 저장하거나 element visibility를
변경하지 않는다. `DESTRUCTION_SIMULATION_FRAME`은 element 아래 fragment frame을 제공하며 각 frame은
model asset, state, normalized life, world pose, linear velocity를 가진다.

## 7. Trigger 연결 상태와 다음 호출 지점

### 7.1 `IMMEDIATE`

현재 구현됨. 다음 fixed timeline step에서 `Activate_Element()`가 실행된다. 실제 Render frame delta가
아니라 controller의 60 Hz clock을 사용한다.

### 7.2 `TIMELINE_TIME`

현재 구현됨. `trigger.timeSeconds` 이상에서 발화한다. 기본 profile과 disabled WorldEvents preview binding은
모두 0.25초를 사용한다. MapTool pattern timeline은 저작 playhead일 뿐 실제 `CValtan` clip이나 Server
pattern을 재생하지 않는다.

### 7.3 `COLLISION_IMPACT`

MapTool 수동 audition만 구현됐다.

```text
Fire Collision button
  -> CDestructionSimulationController::Request_Collision(receiverCollisionId)
  -> CDestructionSimulationRuntime::Notify_Collision(receiverCollisionId)
  -> ID가 같은 WAITING element Activate_Element
```

실제 Valtan 충돌로 연결하려면 다음을 한 수직 슬라이스로 닫는다.

1. `Gameplay.world.json`에 stable collisionBox를 저작한다.
2. `ValtanWorldEvents.json` binding의 `receiverCollisionId`를 그 ID로 바꾼다.
3. navigation blocker region과 enabled 외부 참조 검증을 통과한다.
4. Server가 forced charge와 swept boss shape/receiver OBB 교차를 판정한다.
5. Server persistent transition과 one-shot event를 같은 commit tick에 확정한다.
6. Shared full/delta와 Client presentation projection을 연결한다.

Client/MapTool이 boss transform을 보고 제품 충돌을 자체 판정하거나 binding을 임의 발화하면 안 된다.

### 7.4 Pattern/phase

`CEncounterPatternReference`와 Easy Wall Editor는 pattern/stage/actionId/duration을 읽고 offset을 저작한다.
현재 `VALTAN_ARMOR_BREAK_OPENING/WALL_CHARGE` 선택은 데이터 관계만 뜻한다. 실제 Server pattern start,
charge direction, impact, groggy/recovery 전이는 아직 `WD-G06` 이후 범위다.

## 8. Effect Tool과 연결하는 올바른 방법

현재 구현은 Effect Tool의 Complete/Solo/Detail/Timeline **UX 패턴만** 재사용한다. `CEffect_Tool` 인스턴스,
active document, global character preview service를 MapTool에 연결하지 않는다. 그렇게 하면 Effect clock과
PhysX clock이 따로 움직이고 Reset/Seek 소유권이 충돌한다.

원본 dust effect가 복구된 뒤에는 `CDestructionSimulationController`가 소유하는 preview session에
effect lane을 추가한다.

```text
Destruction controller sample time
  ├─ PhysX debris lane: reset -> fixed 1/60 resimulate
  └─ Effect lane: CEffectObject::Reset / Set_SampleTime(same sample time)
```

권장 연결:

1. 추출 근거와 dependency를 복구해 정상 `EFFECT_DOCUMENT_DESC`로 만든다.
2. MapTool이 기존 `CEffectObject` prototype을 Development preview layer에 stage한다.
3. `bAutoPlay=false`, `Set_Playing(false)`로 Effect 자체 clock을 끈다.
4. controller의 Restart/Seek/Step 뒤 `CEffectObject::Set_SampleTime(snapshot time)`을 호출한다.
5. effect root는 source wall 또는 impact origin world matrix를 사용한다.
6. stage 실패 시 기존 physics preview와 effect object를 함께 rollback한다.

제품 재생은 MapTool 경로가 아니라 `CEffectCatalog -> CEffectPresentationService -> CEffectObject`를 사용한다.
Server는 effect asset path를 보내지 않고 stable cue/profile ID와 impact origin/direction/seed만 보낸다.

## 9. 다른 mesh debris로 확장하는 순서

현재 네 proxy model과 12-piece count는 project-authored Valtan recipe다. 다른 자산까지 넓힐 때 hardcode를
계속 복사하지 않는다.

1. source Deploy asset별로 실제 fractured/piece/effect 근거를 먼저 조사한다.
2. exact fragment asset이 있으면 stable fragment asset ID와 pivot/bounds를 admit한다.
3. 없으면 `PROJECT_AUTHORED`임을 표시한 recipe/profile을 만든다.
4. schema v2 또는 별도 presentation catalog에 `recipeId`, piece model IDs/count, scale, spread/up/angular
   정책을 저장한다.
5. element는 계속 source placement identity를 소유하고 runtime이 recipe의 fragment를 파생한다.
6. All/Solo/clock/actor lifecycle은 현재 runtime을 재사용한다.

작은 먼지·불꽃은 Effect particle, 충돌하는 큰 돌은 PhysX mesh debris로 분리한다. Effect particle을 Server
collision truth로 쓰거나 작은 dust particle마다 rigid actor를 만들지 않는다.

## 10. 파일별 작업 지점

| 파일 | 책임 |
|---|---|
| `Engine/Public/Physics_Manager.h` | PhysX 타입을 숨긴 actor/shape/pose/velocity/gravity/fixed-step facade |
| `Engine/Public/RigidBody.h` | runtime actor component wrapper |
| `Client/Public/DestructionSimulationDocument.h` | tool-only profile/element/trigger 저장 계약 |
| `Client/Public/DestructionSimulationRuntime.h` | All/Emitter/Fragment scope와 evaluated frame |
| `Client/Private/DestructionSimulationRuntime.cpp` | 12-piece recipe, actor lifecycle, trigger, pose pull |
| `Client/Public/DestructionSimulationController.h` | Render-safe command latch와 단일 fixed clock |
| `Client/Private/DestructionSimulationController.cpp` | Stage/Play/Reset/Seek/Collision command 순서 |
| `Client/Public/DeployPropObject.h` | source physics preview와 multi-proxy presentation seam |
| `Client/Private/DeployPropObject.cpp` | admitted CModel clone, proxy render, exact restore |
| `Client/Public/MapTool.h` | editor selection/draft/status ownership |
| `Client/Private/MapTool.cpp` | prototype batch admission, Emitter tree, Timeline/Detail UI |
| `Data/Encounters/Valtan/ValtanWorldEvents.json` | group/mutation/binding stable 관계 |
| `Data/Maps/Authoring/...destructionsimulation.json` | MapTool physics tuning source |

## 11. 흔한 실패와 확인 순서

### Valtan Area가 열리지 않음

1. `Client/Bin/Resources/Map/LV_LUT_HEARTRB_ED`의 catalog 요구 파일이 실제로 있는지 확인한다.
2. `ValtanWorldEvents.json`이 빈 group 파일로 덮이지 않았는지 확인한다.
3. simulation `profile.groupId`가 WorldEvents group과 같은지 확인한다.
4. 존재하지 않는 collision receiver를 binding이 참조하지 않는지 확인한다.
5. 현재 실행한 EXE, working directory, `LOSTARK_RESOURCE_ROOT`를 확인한다.

`git pull`은 Git 제외 대상인 `Client/Bin/Resources`를 채우지 않는다.

### 패널은 보이지만 파편이 재생되지 않음

1. `Stage Selected (Paused)`만 누르지 않았는지 확인한다.
2. `Play All Fragments` 또는 `Play`를 누른다.
3. element가 `TIMELINE_TIME`이면 0.25초를 지난다.
4. `COLLISION_IMPACT`이면 `Fire Collision`을 누른다.
5. scope가 `SOLO_FRAGMENT`이고 다른 fragment가 `FILTERED`인 것은 정상이다.
6. 패널의 debris prototype status가 ready인지 확인한다.
7. 오른쪽 fragment state가 `ACTIVE`인지 확인한다.

### 다른 worktree에서 외형·Effect가 회귀해 보임

미커밋 변경은 worktree 사이에 공유되지 않는다. EXE는 자신의 worktree `Data`, `DataFiles`, ShaderFiles를
읽는다. Resources만 junction으로 공유해도 코드·저작 catalog·shader가 다른 실행본이면 결과가 다르다.
실행 전 EXE 절대 경로와 working directory를 기록한다.

## 12. 담당자 완료 체크리스트

```text
[ ] 최신 main을 기능 branch에 병합하고 conflict/unmerged path 0 확인
[ ] 네 proxy Resources-relative asset 실제 존재 확인
[ ] Lobby -> Test -> Valtan Area 진입
[ ] Play All Fragments에서 fragment 60개 admission/활성 상태 확인
[ ] 한 Wall Emitter Solo에서 12 actor 확인
[ ] 한 Fragment Solo에서 actor 1개, 나머지 FILTERED 확인
[ ] Restart/Seek 결과가 같은 pose/velocity seed를 재현하는지 확인
[ ] tool close/Area switch에서 source와 actor가 원상 복원되는지 확인
[ ] Collision trigger manual fire 확인
[ ] Save/Reload와 pair rollback 확인
[ ] PhysicsContractHarness, Server contract test, Client build, ProjectAudit 실행
[ ] 제품 Server gate를 열었다면 publisher/Shared/late join까지 같은 PR에서 검증
```
