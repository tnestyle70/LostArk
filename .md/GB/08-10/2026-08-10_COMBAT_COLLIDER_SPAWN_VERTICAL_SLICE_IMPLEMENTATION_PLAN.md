# Combat Collider / Character Select Spawn 수직 슬라이스 구현 계획

## 1. 목표

현재 `main`의 기존 제품 경로를 유지하면서 다음 한 단위를 끝까지 구현한다.

1. Shared에 Engine/PhysX 비의존 XZ combat collision 수학을 둔다.
2. Server fixed tick에서 플레이어 스킬, 일반 몬스터 공격, Valtan 공격을 같은 body-footprint 계약으로 판정한다.
3. 피해량은 기존 Data 정본(`PlayerSkills`, `DamageProfiles`, `MonsterProfiles`, `ValtanEncounter`)만 사용한다.
4. Character Select Server Arena에서 ImGui로 일반 몬스터, 중간 보스 Lugaru, Valtan을 선택해 Server 명령으로 즉시 생성한다.
5. Client는 Server 결과를 표현하고 collider를 Debug ImGui 토글로만 표시한다. Client/PhysX는 damage 권위가 아니다.
6. Debug/Release 빌드와 protocol/Server/Client harness, ProjectAudit를 통과한 뒤 PR을 만들고 병합한다.

이 문서는 08-09의 장기 Combat Capsule/F7 계획 중 이번 요청에 필요한 실행 단위만 대체한다. 다중 hit-volume authoring, weapon bone sweep, knockback/rigid-body, 전용 encounter editor, F7 public key 변경은 이번 단위에 포함하지 않는다.

## 2. 권위와 데이터 흐름

```text
Data authoring
  -> publisher/bootstrap
  -> Shared analytic collision contract
  -> Server fixed tick overlap + damage
  -> Shared snapshot/spawn/damage event
  -> Client presentation + Debug collider mirror
```

- 플레이어 body seed: 기존 `WorldCollisionContract`의 XZ half extent 0.45를 원형 footprint radius로 사용한다.
- 몬스터/중간 보스/Valtan body: `MonsterProfiles.collisionRadius`, `BossProfiles.collisionRadius`를 Server가 소유한다.
- 플레이어 스킬: 기존 `maximumRange + target body radius` 의미를 Shared circle overlap으로 보존한다.
- 몬스터 공격: monster root 기준 `collisionRadius + attackRange` 원과 player footprint를 겹침 검사한다.
- Valtan 공격: 기존 CIRCLE/RING/CONE/BOX/CROSS를 player footprint가 있는 도형 교차로 확장한다.
- Client collider는 같은 radius를 wire로 받아 그리는 read-only mirror다.

## 3. G별 구현

### G0. 격리와 기준선

- worktree: `C:/Users/user/Desktop/LostArk-combat-collider-spawn-slice`
- branch: `codex/combat-collider-spawn-slice`
- `origin/main` 기준 Character Select 단일 Server 흐름 커밋을 먼저 통합한다.
- 다른 Effect checkout의 파일, Client/Server 바이너리와 실행 PID는 수정·재빌드·종료하지 않는다.

### G1. Shared combat collision contract

신규:

- `Shared/Public/Gameplay/CombatCollisionContract.h`
- 필요 시 `Shared/Private/Gameplay/CombatCollisionContract.cpp`

계약:

- finite/positive body circle validation
- circle-circle tangent 포함 overlap
- circle-ring overlap
- circle-forward-box overlap
- circle-cross overlap
- circle-cone overlap
- 모든 함수는 pure/deterministic이며 DirectX/Engine/PhysX를 include하지 않는다.

프로젝트 파일에 신규 CPP/H를 실제 물리 폴더와 동일하게 등록한다.

### G2. Server fixed-tick 판정 연결

수정:

- `Server/Private/PlayerSkillSystem.cpp`
- `Server/Private/MonsterBrain.cpp`
- `Server/Private/ValtanBrain.cpp`

보존:

- Server만 target/damage를 확정한다.
- 기존 hit timing, target ordering, counter gate, defense, HP/death, `DAMAGE_EVENT` 흐름을 유지한다.
- 플레이어 스킬의 현재 once-per-stage 의미와 총 damage를 바꾸지 않는다.

변경:

- 각 시스템의 수작업 center-point/range 계산을 G1 함수로 연결한다.
- player target은 radius 0.45 body footprint로 취급한다.
- Valtan analytic shape는 tangent body까지 적중하며 높이/PhysX pose는 사용하지 않는다.

### G3. Character Select Server spawn 세트

신규 Data:

- `Data/Worlds/LV_LOBBY_CLASSSELECT_SL00/SpawnGroups.world.json`

stable request IDs:

- `spawn.character-select.monster`
- `spawn.character-select.miniboss`
- 기존 `boss.valtan.character-select.lazy`

규칙:

- 일반 몬스터는 기존 supported archetype 한 개를 zero-delay 단일 wave로 생성한다.
- 중간 보스는 기존 `MINIBOSS_LUGARU` resource/profile을 같은 제품 monster runtime으로 생성한다.
- Valtan은 기존 disabled boss placement를 Server가 활성화한다.
- Character Select Arena 외 world, unknown ID, duplicate activation은 거부하거나 idempotent 결과를 반환한다.
- 즉시 audition group은 실제 entity 생성 callback이 성공한 뒤에만 `ACTIVATED`를 commit하며 실패 시 DORMANT를 유지한다.
- 마지막 플레이어가 나가면 Character Select 동적 entity와 SpawnGroup runtime을 bootstrap 상태로 되돌린다.
- Client는 archetype/transform/damage를 보내지 않고 stable ID만 보낸다.

Shared protocol은 기존 spawn request packet의 stable ID payload를 일반화하고 group activation 성공을 표현한다. protocol version, codec, roundtrip harness를 같은 변경에서 갱신한다.

Server `CGameRoom`은 placement 또는 SpawnGroup ID를 현재 room bootstrap에서 resolve한다. SpawnGroup activation은 Server tick의 기존 `CSpawnGroupRuntime`으로 들어가며 zero-delay 문서는 같은 tick에 생성된다.

### G4. Client Character Select ImGui와 presentation

수정:

- `Client/Private/Level_CharacterSelect.cpp`
- `Client/Public/Level_CharacterSelect.h`
- typed command sink/network 경계

UI:

- Monster / Mid Boss (Lugaru) / Valtan 세 항목을 선택한다.
- `Spawn Selected` 한 버튼만 Server request를 보낸다.
- pending/result/timeout을 stable ID별로 관리한다.
- local preview spawn이나 local `vector<CMonster>`를 만들지 않는다.
- server-only Character Select 흐름을 유지한다.

일반 몬스터와 Lugaru는 기존 `CMonsterPresentationAssetService -> CNpc`, Valtan은 기존 `CValtan` lazy presentation 경로를 소비한다.

### G5. Client combat collider Debug mirror

wire spawn row에 finite positive `collisionRadius`를 추가한다. Server는 runtime profile 값을 전송한다.

- `CCharacter`: 기존 root OBB를 재사용한다.
- `CNpc`: replicated radius sphere collider를 추가한다.
- `CValtan`: replicated radius sphere collider를 추가한다.
- `CClientReplication`이 Debug visibility를 보관하고 현재/신규 presentation에 전달한다.
- Character Select ImGui에 `Show Combat Colliders` checkbox를 둔다.
- F7을 추가하지 않으며 Release gameplay와 damage에는 영향이 없다.

### G6. 검증과 문서

자동 검증:

- Shared geometry: tangent/miss, ring inner/outer, rotated cone/box/cross
- protocol: request/result/radius roundtrip, invalid enum/radius, truncated/trailing payload, destination rollback
- Server: Character Select monster/miniboss/Valtan request, wrong-world/unknown/duplicate, fixed-tick spawn, damage/death
- Client: selection IDs, pending/send-failure/result, collider visibility propagation
- JSON/XML strict parse
- `git diff --check`
- ProjectAudit

빌드 순서:

1. Engine Debug/Release
2. UpdateLib Debug/Release
3. Shared + NetworkProtocolHarness Debug/Release 및 실행
4. Server Debug/Release 및 `--contract-test`
5. Client Debug/Release
6. 격리 worktree의 Character Select Server Arena smoke
7. ProjectAudit

RESULT에는 실제 실행한 명령, PASS/FAIL, 수동 검증 미실행 항목을 분리 기록한다.

## 4. 완료 조건

- 세 spawn 선택이 모두 Client local object 생성 없이 Server 명령으로 동작한다.
- Lugaru가 중간 보스 resource/profile을 사용해 기존 Server monster AI(IDLE/CHASE/ATTACK/DEAD)와 replication을 탄다.
- player skill/monster/Valtan damage가 Shared collider 계약을 거쳐 Server tick에서만 결정된다.
- collider Debug 표시가 Server 적용 radius와 일치하고 gameplay 결과를 변경하지 않는다.
- 빌드/하네스/audit가 통과한 하나의 commit을 push하고 PR을 생성한다.
- 격리 worktree의 한 검증 단위로 commit/push하고 PR에서 `main`과 충돌 여부를 확인한 뒤 병합한다.
