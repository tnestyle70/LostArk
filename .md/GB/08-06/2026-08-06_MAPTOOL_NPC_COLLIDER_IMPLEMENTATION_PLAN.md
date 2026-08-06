# MapTool NPC 1종·Server 권위 Collision Box 구현 계획서

## 0. 목표와 완료 기준

이번 변경은 이미 닫힌 MapTool의 Bern 카메라, player spawn 저장, Server spawn,
Trigger Box `changeLevel`, Bern → Valtan 전환 위에 다음 수직 슬라이스를 추가한다.

1. `Npc_Beda.wmodel` 한 종을 `NpcCatalog.json`의 stable archetype으로 등록한다.
2. MapTool에서 NPC를 표면 피킹으로 배치하고 저장·선택·수정·삭제할 수 있게 한다.
3. Bern의 `Gameplay.world.json`에 저장된 NPC를 publisher와 Server bootstrap이 읽고,
   Server가 `WORLD_ENTITY_KIND::NPC`로 spawn한 뒤 Client가 `CNpc`로 표시한다.
4. MapTool에 `Collision Box` option을 추가하고 position, yaw, half extents, enabled를
   편집해 같은 gameplay 문서에 저장한다.
5. Server는 Collision Box를 정적 OBB로 stage하고 플레이어 이동을 플레이어 collider
   크기까지 확장한 swept OBB 판정으로 차단한다.
6. Trigger Box 진입도 더 이상 플레이어 중심점 하나가 아니라 같은 플레이어 collider
   계약으로 판정한다.
7. Client `CCharacter`에는 기존 Engine `CCollider::OBB` component를 붙여 Server가 쓰는
   크기와 동일한 presentation/debug shape를 유지한다. 실제 이동 판정 권위는 Server에만 둔다.

자동 완료 증거는 world publisher rollback, Server contract test, Debug/Release Server·Client
build, ProjectAudit, `git diff --check`다. MapTool GUI에서 직접 배치·저장·재로드하고 Bern 접속 후
NPC 표시, 벽 충돌, Trigger Box의 Valtan 전환을 확인하는 항목은 수동 검증으로 따로 기록한다.

## 1. 현재 실측과 범위 경계

### 1.1 NPC 현재 상태

- `Client/Bin/Resources/Character/NPC/Npc_Beda/Npc_Beda.wmodel`과 texture가 존재한다.
- 기존 `CNpc`는 skinned model 한 개와 idle clip을 받아 렌더링할 수 있다.
- 이전 실제 검증 계약은 model pre-transform `scale 0.0001`, Y축 `-90°`, clip
  `npc_idle_normal_1`이다.
- `Data/Actors/NpcCatalog.json`은 현재 비어 있다.
- `CActorCatalog`는 Character와 Boss만 parse한다.
- Server는 NPC placement를 world entity로 만들고 spawn packet을 보낼 수 있지만,
  `CClientReplication::Apply_WorldEntitySpawn`은 Boss만 허용한다.

따라서 새 NPC GameObject 계층을 만들지 않고 기존 `CNpc`, `CActorCatalog`,
`CClientReplication` 경로를 끝까지 연결한다.

### 1.2 충돌 현재 상태

- Engine에는 DirectX bounding 기반 `CCollider`의 Sphere/AABB/OBB가 있다.
- 저장소에는 `PxScene`, Physics Manager, Physics Tool이나 PhysX project registration이 없다.
- 제품 이동은 `CGameRoom::Update_Players`에서 Server가 확정하고 Client `CCharacter`는 snapshot을
  보간한다.
- 현재 Trigger Box는 Server에서 플레이어 position 한 점으로만 진입을 판정한다.

이번 작업에서 존재하지 않는 PhysX runtime을 병행 도입하지 않는다. 저작 데이터의 정적 OBB,
Server 권위 collision system, 기존 Engine `CCollider` Client component를 한 계약으로 연결한다.

### 1.3 Monster Spawn 경계

기존 최초 요청의 `monsterSpawn`은 NPC 배치로 대체하지 않는다. 현재 제품 계약에는 Monster catalog,
Server archetype/profile, replication, Client presentation이 없고 팀 규칙도 수업용 `CMonster`를
placeholder로 승격하는 것을 금지한다. 이번 결과에는 NPC·Collision Box와 기존 `changeLevel`의 완료를
증명하고, Monster Spawn은 별도 Monster 수직 슬라이스로 명시한다.

## 2. 데이터와 호출 흐름

```text
MapTool surface pick
  → CWorldGameplayDocument formatVersion 4
  → Data/Worlds/<AreaId>/Gameplay.world.json
  → Publish-WorldGameplay.ps1 validate/stage/atomic publish
  → Server/Bin/DataFiles/World/<World>.worldbootstrap version 5
  → CWorldBootstrap parse/validate/stage/commit
  ├─ NPC → CGameRoom world entity → S2C_WORLD_ENTITY_SPAWNED/SNAPSHOT
  │       → CClientReplication → NpcCatalog → CNpc(Npc_Beda)
  └─ collisionBox → CServerCollisionSystem static OBB
                    → player swept movement resolve
                    → authoritative PLAYER_SNAPSHOT
                    → CCharacter transform + matching debug CCollider
```

저장 ID는 `placementId`, NPC 정의 ID는 `NPC_BEDA`, model은 Resources-relative
`Character/NPC/Npc_Beda/Npc_Beda.wmodel`을 사용한다. Prototype tag, pointer, vector index는
데이터 계약에 저장하지 않는다.

## G00. World gameplay formatVersion 4와 Collision Box 저작 계약

수정 파일:

- `Client/Public/WorldGameplayDocument.h`
- `Client/Private/WorldGameplayDocument.cpp`
- `Client/Public/MapTool.h`
- `Client/Private/MapTool.cpp`
- `Client/Public/Trigger_Box.h`
- `Client/Private/Trigger_Box.cpp`
- `Data/Worlds/*/Gameplay.world.json`

`WORLD_PLACEMENT_KIND::COLLISION_BOX`와 JSON kind `collisionBox`를 추가한다. Collision Box는
`placementId`, `position`, `yawDegrees`, `enabled`, `halfExtents`만 소유한다. archetype, encounter,
trigger action은 소유하지 않는다. 문서는 exact-field validation과 positive finite half extents를 적용하고
formatVersion을 4로 올린다.

MapTool option은 Player Spawn, NPC, Boss, Trigger Box, Collision Box를 제공한다. Collision Box와
Trigger Box는 같은 wire OBB authoring renderer를 재사용하되 종류별 색을 구분한다. 표면 피킹 실패 시
Y=0 fallback을 만들지 않는다. 목록 선택 후 position/yaw/halfExtents/enabled 변경과 delete가 기존
document stage/rollback 경로를 그대로 통과해야 한다.

Player Spawn의 표면 피킹 결과는 초기 기준 위치다. 선택 Inspector에는 `Position Delta`와 적용 명령을
제공해 예를 들어 `[+50, 0, 0]`을 입력하면 현재 position에 합산한 최종 transform을 문서에 저장한다.
offset 자체를 별도 runtime 상태로 숨기지 않고 최종 position을 정본으로 만들며, publisher와 Server는
그 저장값을 그대로 소비한다.

MapTool의 기존 CP949 `MapTool.h/.cpp`는 작업 중 UTF-8로 임시 변환해 semantic patch를 적용한 뒤
원래 CP949로 복원하고 decoding을 검증한다.

## G01. Publisher와 Server bootstrap version 5

수정 파일:

- `Tools/WorldPipeline/Publish-WorldGameplay.ps1`
- `Server/Public/WorldBootstrap.h`
- `Server/Private/WorldBootstrap.cpp`
- 생성되는 `Server/Bin/DataFiles/World/*.worldbootstrap`

publisher는 formatVersion 4만 허용하고 collisionBox exact fields, stable ID, finite transform,
positive extents를 검증한다. 출력 row는 actor/trigger와 구분되는 고정 필드 수를 사용하고 bootstrap
header version을 5로 올린다.

Server parser는 `WORLD_BOOTSTRAP_KIND::COLLISION_BOX`를 추가한다. collisionBox는 archetype,
encounter, pattern, trigger action을 모두 거부하고 OBB shape만 commit한다. malformed version,
truncated row, duplicate ID, bad extent, trailing field가 하나라도 있으면 이전 상태를 보존하고 load를
실패시킨다.

## G02. NPC_BEDA catalog와 Client presentation

수정·추가 파일:

- `Data/Actors/NpcCatalog.json`
- `Client/Public/ActorCatalog.h`
- `Client/Private/ActorCatalog.cpp`
- `Client/Public/NpcPresentationAssetService.h` 신규
- `Client/Private/NpcPresentationAssetService.cpp` 신규
- `Client/Public/Npc.h`
- `Client/Private/Npc.cpp`
- `Client/Public/ClientReplication.h`
- `Client/Private/ClientReplication.cpp`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`

NpcCatalog의 한 row는 `NPC_BEDA`, client presentation ID, Resources-relative model ID,
idle clip, runtime status를 소유한다. `CActorCatalog`는 Character/Boss/NPC 세 문서를 모두
parse→validate→stage한 뒤 한 번에 commit하며 NPC 문서만 실패해도 기존 전역 catalog를 부분 갱신하지
않는다.

`CNpcPresentationAssetService`는 실제 Beda model과 기존 `CNpc` prototype을 level 단위 batch로
admit한다. Server나 world data에는 model path, clip, Prototype tag가 노출되지 않는다.

`CClientReplication`은 NPC spawn에서 catalog row를 resolve하고 `CNpc`를 world entity layer에
clone한다. snapshot은 NPC의 Server position/yaw를 `CNpc::Apply_NetworkState`로 반영한다. duplicate
spawn은 kind/archetype/실제 live presentation이 모두 같을 때만 no-op이며, clone 또는 state 적용 실패는
생성한 object를 layer에서 제거한다. Reset은 Valtan과 NPC를 모두 제거한다.

## G03. Server 권위 player collider와 static OBB collision

신규·수정 파일:

- `Shared/Public/Gameplay/WorldCollisionContract.h` 신규
- `Shared/Default/Shared.vcxproj`
- `Shared/Default/Shared.vcxproj.filters`
- `Server/Public/ServerCollisionSystem.h` 신규
- `Server/Private/ServerCollisionSystem.cpp` 신규
- `Server/Public/GameRoom.h`
- `Server/Private/GameRoom.cpp`
- `Server/Public/ServerTriggerSystem.h`
- `Server/Private/ServerTriggerSystem.cpp`
- `Server/Default/Server.vcxproj`
- `Server/Default/Server.vcxproj.filters`

공유 contract는 플레이어 OBB half extents와 center offset만 소유한다. gameplay result나 collision
판정 상태를 공유하지 않는다.

`CServerCollisionSystem`은 enabled collisionBox만 staged vector로 소유한다. GameRoom 초기화에서
world bootstrap 이후 stage하고, player spawn이 OBB와 겹치면 world readiness를 실패시킨다.

이동에서는 현재 position→다음 position segment를 collisionBox local yaw 공간으로 변환하고,
player half extents로 확장한 box에 slab sweep을 수행한다. 충돌 시 penetration 이전의 안전 위치에서
정지하고 move goal/path를 지운다. 목표점만 검사해 얇은 box를 통과하는 tunneling을 허용하지 않는다.
Trigger Move는 scripted Server action이므로 이번 정적 보행 충돌의 적용 대상에서 제외한다.

Trigger Box `Contains`도 같은 플레이어 half extents로 trigger OBB를 확장해 collider가 처음 닿은 tick에
진입으로 처리한다.

## G04. Client CCharacter collider component

수정 파일:

- `Client/Public/Character.h`
- `Client/Private/Character.cpp`
- `Client/Private/Loader.cpp`

`Ready_Character_Shared_Prototypes`가 level별 OBB collider component prototype을 등록한다.
`CCharacter::Ready_Components`는 navigation 유무와 관계없이 shared contract 크기의 collider를 clone한다.
`Update`가 authoritative snapshot 보간 뒤 world matrix로 collider를 갱신한다.

Client collider는 hit/movement 판정을 하지 않는다. Debug에서 기존 navigation 표시가 켜진 경우 같은
renderer debug component 경로로 player collider를 확인할 수 있게 한다. 제품 transform은 계속 Server
snapshot만 소비한다.

## G05. Bern 데이터와 자동 검증

수정 파일:

- `Data/Worlds/LV_BERN_CASTLE_CITY_Z010/Gameplay.world.json`
- `Server/Private/ServerGameplayContractTests.cpp`
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` 또는 기존 audit data check
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`
- `AGENTS.md`
- 대응 `2026-08-06_MAPTOOL_NPC_COLLIDER_RESULT.md`

Bern에는 현재 문서에 authored된 네 player spawn과 Valtan changeLevel Trigger Box를 보존한다. 네 좌표는
코드 고정값이 아니라 MapTool에서 자유롭게 이동하고 Save할 수 있는 초기 데이터다. 그 주변의 실제 지면 위치에
`NPC_BEDA` 한 개와 수동 검증용 collisionBox 한 개를 추가하되 spawn이나 trigger를 가로막지 않는다.

Server contract test는 다음을 검증한다.

- Bern bootstrap에서 네 spawn, changeLevel trigger, NPC_BEDA, collisionBox를 읽는다.
- NPC world entity가 stable archetype과 Server transform으로 초기화된다.
- player spawn은 collisionBox와 겹치지 않는다.
- 빠른 단일 tick 이동도 collisionBox를 관통하지 않는다.
- box 밖 평행 이동은 보존된다.
- 플레이어 collider가 Trigger Box 가장자리에 닿으면 진입 action이 한 번만 발생한다.
- malformed collisionBox/version/extent는 parser가 거부하고 staged state를 commit하지 않는다.

검증 순서:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
msbuild Server/Default/Server.vcxproj /m /p:Configuration=Debug /p:Platform=x64
Server/Bin/Debug/Server.exe --contract-test
msbuild Client/Default/Client.vcxproj /m /p:Configuration=Debug /p:Platform=x64
msbuild Server/Default/Server.vcxproj /m /p:Configuration=Release /p:Platform=x64
Server/Bin/Release/Server.exe --contract-test
msbuild Client/Default/Client.vcxproj /m /p:Configuration=Release /p:Platform=x64
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

수동 검증은 Client 작업 디렉터리 `Client/Default`에서 한다.

1. F1 Developer Tools → Map Tool → Bern을 연다.
2. NPC와 Collision Box를 surface pick으로 만들고 목록에서 선택·수정·삭제·Save·Reload한다.
3. Player Spawn을 선택해 `Position Delta`에 `[+50, 0, 0]`을 적용하고 Save·Reload한 뒤 최종 X가
   50 증가한 값으로 유지되는지 확인한다.
4. Bern Server 입장 시 선택 class player가 변경해 저장한 spawn slot 위치에 생기는지 확인한다.
5. `NPC_BEDA`가 저장 위치와 yaw로 idle animation을 재생하는지 확인한다.
6. 플레이어가 collisionBox를 통과하지 않고, 우회 이동은 가능한지 확인한다.
7. 기존 changeLevel Trigger Box에 닿아 Server room transfer 후 Valtan으로 전환되는지 확인한다.

## 3. 구현 불변식

- MapTool만 `Data/Worlds/<AreaId>/Gameplay.world.json`을 authoring하고 publisher만 runtime bootstrap을
  교체한다.
- playerSpawn은 archetype을 소유하지 않고 class는 인증된 session selection이 소유한다.
- NPC model path와 animation clip은 Client catalog만 소유하며 Server에는 stable archetype만 보낸다.
- Collision Box와 player collision result는 Server authority다. Client collider는 presentation/debug다.
- load와 prototype admission은 parse→validate→stage→commit이며 중간 실패 시 부분 object와 prototype을
  rollback한다.
- 기존 Bern 카메라, 현재 authored된 네 spawn slot, Trigger Box changeLevel, 다른 세션의 Effect Tool 변경을
  보존한다. spawn 좌표는 코드 상수가 아니며 MapTool 저장 transform이 정본이다.
- Monster Spawn은 빈 enum/catalog/placeholder로 완료 처리하지 않는다.
