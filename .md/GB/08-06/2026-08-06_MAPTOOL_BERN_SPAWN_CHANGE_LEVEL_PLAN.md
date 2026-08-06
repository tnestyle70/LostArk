# MapTool Bern Spawn / Change Level 구현 계획

## G0. 목표와 완료 경계

- MapTool에서 현재 선택 가능한 World Gameplay option과 실제 지원 범위를 UI와 결과 문서에 정리한다.
- Bern Area 선택 시 전체 배경 bounds가 아니라 `player.spawn.bern.entry`를 기준으로 카메라를 가까이 배치한다.
- Player Spawn, NPC, Boss, Trigger Box를 렌더된 첫 triangle surface picking 위치에 생성하고 position/yaw, Trigger Box half extents를 편집·저장한다.
- Bern의 player spawn은 `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`을 정본으로 사용하고 Server publisher/bootstrap을 거쳐 Server가 spawn transform을 확정한다.
- Trigger Box에 typed `changeLevel` action을 추가하고 Bern에서 Valtan Arena로 이동하는 경로를 Server authority로 끝까지 연결한다.
- 구현, publisher, contract harness, Debug/Release build와 ProjectAudit 결과를 같은 변경 단위에서 검증한다.

## G1. 조사된 현재 상태

### G1-1. MapTool option

| option | 현재 authoring | 현재 product runtime | 이번 변경 |
|---|---|---|---|
| Player Spawn | placement ID, surface pick position, yaw, enabled 저장 가능 | publisher가 Server bootstrap을 만들고 Server가 session별 spawn slot을 선택 | Bern entry를 카메라 기준과 Server spawn 정본으로 고정 |
| NPC | placement ID, archetype ID, position, yaw, enabled 저장 가능 | `NpcCatalog.json`이 비어 있고 Client replication presentation이 없어 제품 배치는 아직 미지원 | authoring option과 저장 경계는 유지하고 미지원 상태를 UI/RESULT에 명시 |
| Boss | placement ID, archetype/encounter ID, position, yaw, enabled 저장 가능 | Valtan만 Server/Client 수직 슬라이스 완료 | 기존 계약 유지 |
| Trigger Box | position, yaw, half extents, triggerOnce, movePlayer 저장 가능 | Server OBB 진입 판정과 movePlayer 실행 완료 | typed changeLevel 추가, Bern→Valtan 완료 |

### G1-2. 확인된 문제

- Bern editor camera가 distant backdrop을 포함한 map placement 통계로 framing되어 성 중심과 player spawn에서 멀다.
- placement picking은 depth target miss 시 Y=0 plane fallback을 사용해 렌더된 triangle이 아닌 위치도 저장한다.
- Trigger action publisher/bootstrap/runtime은 `movePlayer` 하나만 이해한다.
- 제품 level 전환은 Lobby 승인 진입만 소비하며 gameplay 중 Server 승인 world transfer를 공통 서비스가 소비하지 않는다.

## G2. 데이터와 public 계약

### G2-1. World gameplay JSON v3

- `WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL`을 추가한다.
- JSON action은 `{ "type": "changeLevel", "targetWorldId": "VALTAN_ARENA" }` exact shape만 허용한다.
- target은 현재 등록된 제품 world ID만 허용하고 동일 world target을 publisher/Server에서 거부한다.
- playerSpawn은 `archetypeId`와 `encounterId`를 모두 소유하지 않는다.
- 네 Area의 `Gameplay.world.json`을 formatVersion 3으로 함께 승격한다.

### G2-2. World bootstrap v4

- trigger action을 `actionType payloadFieldCount payload...` 형식으로 직렬화한다.
- `movePlayer 5 x y z duration arc`, `changeLevel 1 TARGET_WORLD`를 지원한다.
- Server parser는 전체 field 소비, payload count, target world, current world와의 중복을 stage 단계에서 검증하고 실패 시 기존 bootstrap을 유지한다.

### G2-3. Server-authority world transfer

1. Server TriggerSystem이 player OBB 진입을 판정한다.
2. `changeLevel` action이면 session/class/nickname/target world가 담긴 typed transfer request를 생성한다.
3. source GameRoom이 player를 `LEVEL_CHANGED`로 제거하고 CServerApp이 같은 session을 target GameRoom에 REGISTER/ENTER한다.
4. target GameRoom이 기존 `S2C_ENTER_ACCEPTED`, player spawn, snapshot 계약을 전송한다.
5. Client gameplay level은 accepted world를 소비한 뒤 `CLevelTransitionService`에만 전환 요청을 제출한다.

## G3. MapTool 구현

### G3-1. Bern camera

- `player.spawn.bern.entry`가 enabled이고 finite이면 그 위치를 focus로 하고 가까운 고정 editor radius를 사용한다.
- entry가 없을 때만 첫 enabled playerSpawn, 그것도 없으면 기존 bounds 계산으로 fallback한다.
- Valtan도 hard-coded editor focus 대신 playerSpawn을 우선 사용한다.

### G3-2. Picking과 편집

- `Target_PickPos`의 depth-tested 첫 visible triangle surface 위치만 허용한다.
- Y=0 plane fallback을 제거하고 miss는 배치 실패로 유지한다.
- World placement와 movePlayer target pick 양쪽이 같은 surface-only 함수를 사용한다.
- trigger target pick armed 상태도 gameplay mouse 소비에 포함한다.
- actor scale은 Server spawn 계약에 없으므로 저장하지 않는다. Trigger Box scale은 half extents로 편집한다.

### G3-3. Trigger option

- 선택 Trigger Box inspector에 `None / Move Player / Change Level` typed action selector를 둔다.
- Change Level target은 현재 지원 world combo로 선택하며 Bern→Valtan을 저장할 수 있게 한다.
- action이 정확히 하나이고 유효할 때만 enabled를 허용한다.
- `monsterSpawn`은 Monster catalog/profile/AI/replication/presentation/spawn group이 없는 상태에서 enum placeholder를 만들지 않는다. 별도 승인된 수직 슬라이스에서 같은 typed action 확장점에 추가한다.

## G4. 변경 파일

- `Client/Public/MapTool.h`, `Client/Private/MapTool.cpp`
- `Client/Public/WorldGameplayDocument.h`, `Client/Private/WorldGameplayDocument.cpp`
- `Client/Public/LevelTransitionService.h`, `Client/Private/LevelTransitionService.cpp`
- `Client/Private/Level_Bern.cpp`, `Client/Private/Level_ValtanArena.cpp`
- `Tools/WorldPipeline/Publish-WorldGameplay.ps1`
- `Server/Public/WorldBootstrap.h`, `Server/Private/WorldBootstrap.cpp`
- `Server/Public/ServerTriggerSystem.h`, `Server/Private/ServerTriggerSystem.cpp`
- `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp`
- `Server/Public/ServerApp.h`, `Server/Private/ServerApp.cpp`
- `Server/Private/ServerGameplayContractTests.cpp`
- `Data/Worlds/*/Gameplay.world.json`, generated `Server/Bin/DataFiles/World/*.worldbootstrap`
- 대응 RESULT와 public 계약 변경이 있는 팀 문서

새 C++ 파일은 추가하지 않으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다.

## G5. 실패와 rollback

- malformed/unknown action, payload count mismatch, unknown/same target world는 publisher와 Server bootstrap stage에서 거부한다.
- bootstrap load 실패는 기존 world definition을 유지한다.
- target GameRoom enqueue 실패 시 session을 닫아 source/target 어느 room에도 유령 player를 남기지 않는다.
- Client가 malformed accepted target을 받거나 전환 요청을 제출할 수 없으면 socket을 정리하고 Lobby recovery를 요청한다.
- MapTool gameplay edit는 기존 document/runtime trigger box를 복사해 stage하고 실패 시 이전 상태를 복원한다.

## G6. 검증

1. JSON parse와 `Publish-WorldGameplay.ps1 -Mode Validate`.
2. publish rollback injection과 실제 Publish.
3. Server gameplay contract test: bootstrap v4 movePlayer/changeLevel 정상, malformed payload/target 실패 후 기존 상태 유지, trigger transfer request 1회 발생.
4. `Server.exe --contract-test`와 관련 harness.
5. Client x64 Debug/Release build, Server x64 Debug/Release build.
6. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`과 `git diff --check`.
7. 수동 smoke는 사용자가 MapTool Bern camera, surface pick, playerSpawn 저장, Trigger Box changeLevel 저장, Server 실행 상태에서 Bern→Valtan 진입을 확인한다.

## G7. 구현 후 기록 원칙

- 실제 완료, 자동 검증, 수동 검증, 후속 Monster/NPC 제품 presentation 경계를 RESULT에서 분리한다.
- 현재 dirty worktree의 Effect Tool 등 무관 변경은 되돌리거나 stage하지 않는다.

## G8. Debug Bern changeLevel Trigger Box 프리뷰

### G8-1. 목표

- Debug Client로 Bern에 진입하면 저장된 `enabled + triggerBox + changeLevel` placement를 월드 와이어 OBB로 렌더링한다.
- 프리뷰 좌표와 크기는 별도 하드코딩하지 않고 `Data/Worlds/<AreaId>/Gameplay.world.json`의 position, yawDegrees, halfExtents를 그대로 소비한다.
- Release Client에는 Trigger Box prototype 등록, authoring 문서 로드, Debug object 생성 경로를 포함하지 않는다.

### G8-2. 변경 경계

- `Client/Private/Loader.cpp`: `_DEBUG` Bern level에 기존 `CTrigger_Box` prototype을 등록한다.
- `Client/Public/Level_Bern.h`, `Client/Private/Level_Bern.cpp`: gameplay 문서를 parse/validate한 뒤 changeLevel Trigger Box를 임시 vector에 전부 stage하고, 성공 시에만 Debug layer와 owner vector를 유지한다.
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`: Bern 정본 데이터와 Debug-only prototype/staging 경로가 함께 존재하는지 검사한다.
- 기존 `CTrigger_Box`, MapTool 저장 형식, Server bootstrap/TriggerSystem에는 새 런타임 경로를 추가하지 않는다.

### G8-3. 실패와 검증

- 문서가 없거나 잘못됐거나 clone/type cast가 실패하면 이번 stage에서 만든 object를 모두 layer에서 제거한다.
- Debug 시각화 실패는 Server-authoritative Bern gameplay 진입을 막지 않고 Debug 출력으로 이유를 남긴다.
- Debug/Release Client build, World publisher validate, ProjectAudit, `git diff --check`를 실행한다.
