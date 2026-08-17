# 2026-08-17 플레이어 ↔ 몬스터/보스 몸체 이동 차단 PLAN

작성자: JS · branch `feature/player-entity-body-collision` (넉백 브랜치 `985ecce` 위 상위집합)
선행: `2026-08-17_MONSTER_HIT_FLASH_KNOCKBACK_RESULT.md`

## 목표

플레이어가 우클릭 이동이나 스킬 루트모션으로 살아 있는 몬스터/보스 몸체(원) 안으로 들어가지 못하게
Server가 막는다. 넉백이 어색해 보이는 원인 하나가 "겹친 채로 맞음"이었다(사용자 관찰 2026-08-17).

## 실측

- 플레이어 이동 차단은 `CServerCollisionSystem::Resolve_PlayerMove`(ServerCollisionSystem.cpp:364) 하나이며
  정적 `collisionBox` OBB만 스윕한다. 호출자 둘: `CGameRoom::Update_Players`(GameRoom.cpp:3511, 우클릭 이동)와
  `CPlayerSkillSystem::Update`(PlayerSkillSystem.cpp:532, 루트모션·movementDistance). 둘 다 blocked면 그
  tick 이동을 접촉점에서 멈춘다(우클릭은 move goal도 해제).
- 플레이어 몸은 `Shared::WorldCollision::PLAYER_HALF_EXTENT_X = 0.45`. 몬스터 반경은 `SERVER_WORLD_ENTITY::
  fCollisionRadius`(profile), 보스는 `catalog.Find_Boss(archetype)->fCollisionRadius`(Valtan 3.0) —
  `PlayerSkillSystem`의 `targetBodyOf`와 같은 규칙. NPC는 반경 0.
- 몬스터→플레이어 파고듦은 `CMonsterBrain`이 `attackRange + collisionRadius`에서 멈춰 대체로 없다.
  그쪽은 몬스터 담당 레인이라 손대지 않는다.

## 계약

| 항목 | 값 |
|---|---|
| 차단 대상 | `m_WorldEntities` 중 살아 있고(`eAction != DEAD`, hp>0) 반경>0인 entity. MONSTER=`fCollisionRadius`, BOSS=boss profile 반경(없으면 entity 값). Esther summon 제외 |
| 갱신 시점 | 매 tick `Update_Players` 직전 `Refresh_PlayerBlockingBodies()`가 `CServerCollisionSystem::Set_BlockingBodies(vector<SERVER_BLOCKING_BODY{x,z,radius}>)`로 전달 (이전 tick의 entity 위치) |
| 판정 | XZ 스윕 원-원: 플레이어 반경 0.45 + 몸체 반경 = R. 시작이 이미 R 안(겹침)이면 **중심 쪽으로 가는 이동만 차단(ratio 0)**, 멀어지는 이동은 허용 → 보스 착지 등으로 끼어도 항상 빠져나올 수 있다 |
| 결과 | 기존 box 스윕과 같은 `earliestHit`/`CONTACT_MARGIN` 처리. 두 호출자 모두 자동 적용, Client 무변경 |
| 미끄러짐 (사용자 결정: 필수) | 몸체에 막히면 접촉점까지 간 뒤 **남은 이동 길이 그대로 접선 방향으로** 한 번 더 스윕(정면이면 결정적으로 한쪽). 원 둘레를 전속력으로 감아 돌며 우클릭 goal은 유지. 정적 collisionBox는 기존 정지 유지. 옆으로도 못 가는 경우만 `wasBlocked` |
| 미포함 | 스킬별 통과 플래그(원작 `sa/move` 조사 필요), 플레이어↔플레이어, 몬스터↔몬스터, 넉백 ease-out(원본 `EFTable_SkillEffect` PushMin/MaxRange·Time은 등속 기술, 곡선 없음) |

## 변경 파일

- `Server/Public/ServerCollisionSystem.h`: `struct SERVER_BLOCKING_BODY { float fX, fZ, fRadius; }`,
  `void Set_BlockingBodies(std::vector<SERVER_BLOCKING_BODY> bodies)`, private `static bool
  Sweep_PlayerAgainstBody(...)`, 멤버 `std::vector<SERVER_BLOCKING_BODY> m_BlockingBodies;`
- `Server/Private/ServerCollisionSystem.cpp`: `Resolve_PlayerMove`의 box 루프 뒤에 body 루프.
- `Server/Public/GameRoom.h` / `Server/Private/GameRoom.cpp`: `Refresh_PlayerBlockingBodies()`, tick에서
  `Update_Players` 직전 호출.
- `Server/Private/ServerGameplayContractTests.cpp`: 정면 접근 정지·측면 통과·겹침에서 탈출 허용·겹침에서
  중심 접근 차단 4건.

## 검증

Server 빌드, `Server.exe --contract-test` 0 fail, NetworkProtocolHarness 불변. 수동: Character Select에서
몬스터/Valtan 쪽으로 우클릭·루트모션 스킬 → 몸체 앞에서 멈춤, 소환 직후 겹쳐 있으면 빠져나올 수 있음.
