# MapTool NPC 1종·Server 권위 Collision Box 구현 결과

## 0. 완료 상태

구현 계획서의 NPC_BEDA와 Collision Box 수직 슬라이스를 반영했다.

- MapTool World Gameplay option은 Player Spawn, NPC, Boss, Trigger Box, Collision Box다.
- 각 placement는 목록에서 선택해 transform/shape/enabled/action을 수정하고 delete할 수 있다.
- Player Spawn은 선택한 position에 `Position Delta`를 합산해 최종 좌표로 확정할 수 있다.
- Save Gameplay는 `Data/Worlds/<AreaId>/Gameplay.world.json` formatVersion 4에 저장한다.
- publisher는 bootstrap version 5를 원자적으로 생성한다.
- Bern runtime은 player spawn 4개, NPC_BEDA 1개, collisionBox 1개,
  Valtan changeLevel trigger 1개를 갖는다.
- Server가 NPC entity spawn, player spawn slot, trigger 진입, collision sweep의 권위를 갖는다.
- Client는 NPC_BEDA를 기존 `CNpc`로 표시하고 player snapshot을 보간한다.
- Client `CCharacter`의 OBB collider는 Server와 같은 크기의 presentation/debug shape이며
  이동 판정을 하지 않는다.

## 1. 실제 데이터

### 1.1 NPC catalog

`Data/Actors/NpcCatalog.json`에 다음 한 archetype을 등록했다.

```text
archetypeId: NPC_BEDA
clientPresentationId: npc.beda.client.v1
modelAssetId: Character/NPC/Npc_Beda/Npc_Beda.wmodel
idleClip: npc_idle_normal_1
runtimeStatus: supported
```

model admission은 과거 실제 검증된 `scale 0.0001`, Y축 `-90°` pre-transform을 사용한다.

### 1.2 Bern placements

```text
player.spawn.bern.entry    [144.8, 42.7, -70.3]
player.spawn.bern.party02  [145.8, 42.7, -70.3]
player.spawn.bern.party03  [143.8, 42.7, -70.3]
player.spawn.bern.party04  [144.8, 42.7, -71.3]
npc.bern.beda.guide        [149.0, 42.7, -66.0], yaw 225
collision.bern.editor-proof center [140.8, 43.7, -65.3], half [0.5, 1.0, 3.0]
trigger.bern.to-valtan     [144.8, 42.7, -60.3], half [2.0, 2.0, 2.0]
```

위 좌표는 코드에 고정된 spawn 값이 아니라 현재 `Gameplay.world.json`에 저장된 초기 authored 값이다.
MapTool에서 한 slot을 선택해 position을 직접 편집하거나 `Position Delta`에 `[+50, 0, 0]`을 적용하면
최종 X가 50 증가한 transform으로 저장된다. publisher와 Server room은 이 최종 저장값을 그대로 읽는다.
Server는 아직 사용되지 않은 enabled slot을 문서 순서대로 할당하며 실제 character class는 session
selection이 소유한다.

## 2. 구현 결과

### 2.1 World document와 MapTool

`WORLD_PLACEMENT_KIND::COLLISION_BOX`와 JSON kind `collisionBox`를 추가했다. collisionBox는
placement ID, position, yaw, enabled, half extents만 소유하며 archetype/action을 거부한다.

MapTool은 기존 surface pick을 그대로 사용한다. miss 때 Y=0 fallback을 만들지 않는다. Trigger Box와
Collision Box는 같은 staged wire OBB presentation을 사용하고, Collision Box는 파란색으로 구분한다.
편집이나 delete 뒤 wire box stage가 실패하면 이전 document와 presentation을 유지한다.
NPC option은 현재 단일 지원값 `NPC_BEDA (Npc_Beda)`를 자동 설정해 빈 archetype 입력을 만들지 않는다.
Player Spawn 선택 Inspector의 `Position Delta`는 현재 position에 상대 이동량을 합산한다. 적용 전에는
document를 dirty로 만들지 않고, 적용 후 validation과 stage가 성공한 경우에만 최종 position을 commit한다.
offset은 runtime에 별도로 남기지 않으므로 Server가 숨은 하드코딩을 더하지 않는다.

MapTool의 기존 CP949 파일은 semantic patch 뒤 CP949로 복원했고 strict round-trip decode를 확인했다.

### 2.2 NPC Client presentation

`CActorCatalog`가 NpcCatalog를 읽고 `NPC_BEDA`를 resolve한다. `CNpcPresentationAssetService`가
level 단위로 Beda model과 기존 `CNpc` prototype을 batch admission한다.

`CClientReplication`은 Server의 NPC spawn에서 catalog를 확인하고 world entity layer에 `CNpc`를
생성한다. snapshot position/yaw를 `CNpc::Apply_NetworkState`로 반영한다. duplicate spawn 계약이
다르거나 presentation이 죽었으면 실패하며, clone/state 적용 중 실패한 object는 layer에서 제거한다.
world reset은 Valtan과 NPC presentation을 모두 제거한다.

### 2.3 Server collision

`CServerCollisionSystem`은 enabled collisionBox만 bootstrap에서 stage한다. 모든 enabled player spawn이
player OBB를 포함해 box와 겹치지 않는지 room readiness에서 확인한다.

일반 보행은 현재 position에서 다음 tick position까지 segment를 box yaw local space로 변환하고 player
half extents만큼 확장한 OBB에 slab sweep을 수행한다. hit 시 contact margin 전 위치에서 정지하고 move
goal/path를 지운다. 얇은 box를 한 tick에 건너뛰는 tunneling을 허용하지 않는다. Server scripted
Trigger Move는 이 일반 보행 차단 대상이 아니다.

Trigger Box도 player 중심점 대신 같은 player OBB 크기로 확장해 첫 contact tick을 진입으로 판정한다.

### 2.4 Client player collider

공유 contract의 player half extents는 `[0.45, 0.90, 0.45]`, center Y offset은 `0.90`이다.
Loader가 level별 `CCollider::OBB` prototype을 등록하고 모든 `CCharacter`가 navigation 유무와 관계없이
clone한다. snapshot 보간 후 world matrix로 갱신하며 Debug navigation 표시 때 같은 debug renderer에
추가한다.

## 3. 자동 검증

실행 결과:

```text
World JSON/NpcCatalog JSON parse: PASS
Publish-WorldGameplay.ps1 -Mode Validate: PASS (Bern 7 / Valtan 5 / Training 4 / Character Select 5)
Publish-WorldGameplay.ps1 -Mode Publish: PASS, bootstrap v5
Server x64 Debug build: PASS, warnings 0 / errors 0
Server Debug --contract-test: PASS, failures 0
Client x64 Debug build: PASS
Server x64 Release build: PASS
Server Release --contract-test: PASS, failures 0
Client x64 Release build: PASS
ProjectAudit: PASS, 70 checks
world publish generation rollback: PASS through ProjectAudit
git diff --check: PASS
```

Server contract에서 추가로 증명한 항목:

- Bern의 네 spawn, NPC_BEDA, collisionBox, trigger parse
- spawn과 collisionBox 비중첩
- 빠른 단일 tick movement sweep 차단
- box 밖 평행 이동 보존
- bootstrap v5 trigger/collisionBox parse와 malformed trigger rollback
- player collider가 Trigger Box 가장자리에 닿을 때 진입

Client build에는 기존 저장소의 C4819/LNK4099 warning이 남아 있지만 신규 compile/link 오류는 0이다.

## 4. 수동 검증 상태

GUI와 실제 socket을 사용하는 다음 항목은 이번 자동 실행에서 수행하지 않았다. 사용자가 직접 확인할
검증 경계다.

1. F1 Developer Tools → Map Tool → Bern에서 NPC/Collision Box 표면 배치
2. 목록에서 Player Spawn을 선택하고 `Position Delta` `[+50, 0, 0]` 적용, Save Gameplay, Reload Gameplay
3. Reload 뒤 최종 X가 50 증가한 값인지 확인하고 Server와 Client를 실행해 그 위치에 입장
4. NPC_BEDA model과 `npc_idle_normal_1` 표시
5. `collision.bern.editor-proof` 통과 차단과 바깥 우회 이동
6. `trigger.bern.to-valtan` 진입 후 Server room transfer와 Valtan 전환

## 5. 남은 경계

일반 Monster Spawn은 완료 처리하지 않았다. 실제 MonsterCatalog, Server profile/action, replication,
Client presentation이 없으므로 NPC_BEDA로 위장하거나 빈 enum/catalog를 추가하지 않았다. 첫 실제 Monster
요구에서는 model과 gameplay profile을 포함한 별도 수직 슬라이스가 필요하다.

현재 worktree에는 이 작업 전부터 다른 Effect Tool/Animation/asset 변경이 대량으로 존재한다. 이번 작업은
그 변경을 되돌리거나 자동 stage/commit하지 않았다.
