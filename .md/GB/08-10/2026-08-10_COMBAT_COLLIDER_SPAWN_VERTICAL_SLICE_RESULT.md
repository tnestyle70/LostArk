# Combat Collider / Character Select Spawn 수직 슬라이스 결과

## 1. 결과

요청한 최소 수직 슬라이스를 `Data -> Shared -> Server -> Client -> harness` 한 경로로 구현했다.

- Shared에 Engine/PhysX 비의존 XZ combat collision primitive를 추가했다.
- player skill, 일반 몬스터/Lugaru, Valtan 공격은 Server fixed tick에서 body footprint를 포함해 판정한다.
- damage, defense, counter, HP/death 순서는 기존 Data와 Server authority를 유지한다.
- Character Select Server Arena에 `Monster / Mid Boss (Lugaru) / Valtan` 선택과 `Spawn Selected` Debug ImGui를 추가했다.
- 일반 몬스터와 Lugaru는 Character Select `SpawnGroups.world.json`, Valtan은 기존 disabled placement를 사용한다.
- 즉시 SpawnGroup은 실제 entity 생성 성공 뒤에만 `ACTIVATED`가 되며 실패하면 DORMANT를 유지한다.
- 중복 활성화는 멱등 결과를 반환하고, 마지막 플레이어 퇴장 시 동적 audition entity와 SpawnGroup runtime을 초기화한다.
- spawn packet이 Server collision radius를 전달하고 Client character/NPC/Valtan collider는 Debug wire mirror로만 표시된다.
- F7은 추가하지 않았고 Character Select의 `Show Combat Colliders` checkbox만 사용한다.

## 2. 주요 구현 파일

### Data / Shared

- `Data/Worlds/LV_LOBBY_CLASSSELECT_SL00/SpawnGroups.world.json`
- `Shared/Public/Gameplay/CombatCollisionContract.h`
- `Shared/Private/Gameplay/CombatCollisionContract.cpp`
- `Shared/Public/Network/PacketType.h`
- `Shared/Public/Network/PacketMessages.h`
- `Shared/Private/Network/PacketMessages.cpp`

### Server

- `Server/Private/PlayerSkillSystem.cpp`
- `Server/Private/MonsterBrain.cpp`
- `Server/Private/ValtanBrain.cpp`
- `Server/Private/SpawnGroupRuntime.cpp`
- `Server/Private/GameRoom.cpp`
- `Server/Private/ServerGameplayContractTests.cpp`

### Client

- `Client/Private/Level_CharacterSelect.cpp`
- `Client/Private/ClientReplication.cpp`
- `Client/Private/Character.cpp`
- `Client/Private/Npc.cpp`
- `Client/Private/Valtan.cpp`
- `Client/Private/Loader.cpp`

## 3. 자동 검증

### PASS

- Debug 전체 build: Engine, UpdateLib, Shared, NetworkProtocolHarness, ClientFrontendHarness, Server, Client compile/link PASS.
- Release 전체 build: Engine, UpdateLib, Shared, NetworkProtocolHarness, ClientFrontendHarness, Server, Client compile/link PASS.
- NetworkProtocolHarness Debug/Release: `failures : 0`.
- `Server.exe --contract-test` Debug/Release: `failures : 0`.
  - circle tangent/miss
  - ring inner/outer boundary
  - rotated box/cone/cross
  - combat-ready target admission
  - Character Select two SpawnGroups parse/reference/zero-delay contract
  - immediate spawn callback rollback and commit
  - last-player room reset and next-generation reactivation
- `Publish-WorldGameplay.ps1 -Mode Validate`: 4 world + Valtan 3 groups + Character Select 2 groups PASS.
- `Publish-ServerNavigation.ps1 -Mode Validate`: Valtan/Training/Character Select PASS.
- changed JSON/XML parse PASS.
- 이번 변경에 추가·갱신한 ProjectAudit 항목 PASS:
  - Character Select Server-only three-choice spawn contract
  - Character Select SpawnGroups/Shared collider/project registration
  - protocol version 14
- `git diff --check` PASS.

### 현재 브랜치 밖 선행 Effect 실패

정본 회귀를 끝까지 실행했으며 다음 실패는 이 수직 슬라이스 파일과 무관한 현재 Effect Data/Resources 상태에서 동일하게 발생했다.

- ClientFrontendHarness: Effect authored/imported resource join 7 failures.
- ProjectAudit: Effect WFX component assembly, representative authored readiness, four-class authored clip product 3 checks.
- 대표 원인: `Artist/31210` manifest/binding stage count mismatch와 진행 중 Effect resource contract 불일치.

이 결과를 숨기거나 PASS로 기록하지 않았고 Collider/Spawn 구현에서 Effect 문서나 Resources를 수정하지 않았다.

## 4. 수동 검증

격리 작업 중 `C:/Users/user/Desktop/LostArk`의 별도 Effect 검증 Client/Server PID와 바이너리를 보존해야 해서 Character Select live smoke는 이 worktree에서 실행하지 않았다. 자동 protocol, Server room/runtime, Debug/Release build 검증으로 이번 변경 경계를 확인했다.

## 5. 범위 밖

- animation/effect bone 기반 hit volume authoring
- multi-hit window와 weapon sweep
- knockback/forced motion/PhysX presentation
- F7 public shortcut

위 항목은 이번 최소 collider/damage/spawn 수직 슬라이스의 완료 조건에 포함하지 않았다.
