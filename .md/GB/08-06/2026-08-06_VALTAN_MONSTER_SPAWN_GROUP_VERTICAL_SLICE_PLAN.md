# 2026-08-06 발탄 몬스터 Spawn Group 수직 슬라이스 구현 계획

## 1. 목표

발탄 Area의 진행 단계를 새 Level로 늘리지 않는다. 기존 `LEVEL::VALTAN_ARENA`와
`WORLD_ID::VALTAN_ARENA` 안에서 MapTool로 저작한 Trigger Box가 Server 권위의
Spawn Group을 활성화하고, 일반 몬스터와 통솔자 루가루가 순차 생성되어 Client에
표시·전투·사망·제거되는 한 개의 실행 계약을 완성한다.

최종 흐름은 다음 하나만 사용한다.

```text
MapTool
  -> Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json
  -> Data/Worlds/LV_LUT_HEARTRB_ED/SpawnGroups.world.json
  -> Publish-WorldGameplay.ps1
  -> Server bootstrap
  -> CGameRoom Trigger 진입 판정
  -> Spawn Group 활성화와 wave 진행
  -> Shared world entity packet/snapshot
  -> Client monster presentation
```

`Stage_1`, `Stage_2`, `Stage_3`은 Level 이름이 아니라 같은 발탄 월드 안의 stable
spawn group ID다. `Change Level`은 기존 Bern/Valtan Arena 전환에만 사용한다.

## 2. 저장 계약

### 2-1. Gameplay.world.json

Trigger Box는 정확히 하나의 action만 소유한다. 새 action은 다음 형태다.

```json
{
  "type": "activateSpawnGroup",
  "spawnGroupId": "spawn.valtan.stage01"
}
```

발탄 최종 보스는 별도 action으로 disabled boss placement를 활성화한다.

```json
{
  "type": "activateEncounter",
  "targetPlacementId": "boss.valtan.center"
}
```

### 2-2. SpawnGroups.world.json

정본은 `Data/Worlds/<AreaId>/SpawnGroups.world.json`이다. 이 문서는 다음을 소유한다.

- stable anchor ID와 위치
- stable spawn group ID
- 선행 group ID
- 동시 생존 수 제한
- wave 시작 지연
- wave별 archetype, 수량, anchor, 생성 간격
- 1회 실행과 전체 wave 처치 완료 정책

런타임 Prototype tag, 포인터, vector index는 저장하지 않는다.

## 3. 논리 monster ID

물리 리소스 폴더 이름은 원본 근거를 보존하고, 제품 계약에서는 다음 stable ID를 쓴다.

| 역할 | stable archetype ID | 물리 리소스 |
|---|---|---|
| 일반 몬스터 1 | `MONSTER_VALTAN_PADD_01` | `Character/Monster/NPC_480001_MN_PADD_01` |
| 일반 몬스터 2 | `MONSTER_VALTAN_SJFC_00_4` | `Character/Monster/NPC_480002_MN_SJFC_00_4` |
| 일반 몬스터 3 | `MONSTER_VALTAN_0019_05` | `Character/Monster/NPC_480003_MN_0019_05` |
| 중간 보스 | `MINIBOSS_LUGARU` | `Character/Monster/NPC_480005_MN_RPRS_02` |

## 4. 구현 그룹

### G1. Authoring 문서와 MapTool

- `CSpawnGroupDocument`를 추가한다.
- 로드는 `parse -> validate -> stage -> commit`을 지킨다.
- MapTool World Gameplay에 Spawn Anchor와 Spawn Group 편집 UI를 추가한다.
- Trigger action 목록에 `Activate Spawn Group`, `Activate Encounter`를 추가한다.
- Trigger action의 target은 free-form 추측값이 아니라 현재 문서의 stable group/placement ID에서 선택한다.
- 저장은 Gameplay와 SpawnGroups를 각각 authoring 정본에 기록한다.

### G2. Publisher와 bootstrap

- publisher가 Gameplay trigger target과 SpawnGroups 문서를 교차 검증한다.
- 잘못된 version, 중복 ID, 없는 anchor/archetype/prerequisite, 순환 prerequisite,
  최대 수량 초과, 활성 trigger의 없는 target을 거부한다.
- 기존 world bootstrap과 spawn group bootstrap을 같은 stage/promote 트랜잭션으로 교체한다.
- 생성된 bootstrap은 직접 편집하지 않는다.

### G3. Shared protocol

- `WORLD_ENTITY_KIND::MONSTER`를 추가한다.
- 동적 몬스터 제거를 위한 `S2C_WORLD_ENTITY_DESPAWNED`를 추가한다.
- protocol version을 올리고 writer/reader/NetworkProtocolHarness를 함께 변경한다.

### G4. Server authority

- `CSpawnGroupBootstrap`과 `CSpawnGroupRuntime`을 추가한다.
- Trigger 진입 시 group activation 성공 여부가 확정된 경우에만 trigger once를 소비한다.
- wave, spawn interval, max alive, prerequisite, once 정책을 Server tick으로 진행한다.
- 새 몬스터는 ServerNavigation에 투영된 anchor에서 생성한다.
- `CMonsterBrain`은 가장 가까운 생존 플레이어를 추적하고 chase/windup/active/recovery/dead를 Server 권위로 진행한다.
- 공격 damage와 player skill 피격은 Server에서 처리한다.
- 사망 presentation 시간이 지난 엔티티는 despawn packet 후 제거한다.
- encounter activation은 disabled boss placement를 stable placement ID로 생성한다.

### G5. Client presentation

- `MonsterCatalog.json`에서 stable archetype ID를 Resources-relative model asset ID와
  semantic animation으로 변환한다.
- 기존 `CModel -> CMaterial` 경로만 사용한다.
- 새 제품 객체는 수업용 `CMonster`를 승격하지 않고 별도 monster presentation으로 둔다.
- spawn/snapshot/despawn 이벤트로 생성, action 갱신, 제거한다.
- Client는 AI, damage, wave 완료를 판정하지 않는다.

### G6. 발탄 데이터

- 기존 사용자가 배치한 Trigger 좌표와 Move Player action을 보존한다.
- `Stage_1`은 `spawn.valtan.stage01`을 활성화한다.
- `Stage_MiniBoss`는 `spawn.valtan.stage02.miniboss`를 활성화한다.
- `Stage_2`는 `spawn.valtan.stage03`을 활성화한다.
- `Stage_3`은 기존 절벽 이동 action으로 유지한다.
- `Stage_Boss`는 `boss.valtan.center` encounter를 활성화한다.
- 원본 수량 근거가 없는 동안 수량·전투 수치는 `PROJECT_TUNED` 검증값으로 시작하며
  MapTool에서 조정한다.

## 5. 실패·rollback 계약

- authoring parse/validate 실패 시 현재 MapTool 문서를 유지한다.
- publisher 실패 시 기존 Server runtime bootstrap을 유지한다.
- 없는 group이나 prerequisite 미완료 activation은 실패하고 trigger once를 소비하지 않는다.
- 일부 몬스터 prototype/presentation 로드 실패는 해당 archetype만 격리하고 Server 진행을
  Client 로컬 fallback으로 바꾸지 않는다.
- disconnect/room leave 시 해당 Client presentation을 전부 제거한다.

## 6. 검증

1. SpawnGroups parser 정상/잘못된 version/중복/없는 참조/순환/rollback harness
2. Publisher 정상 publish와 failure injection rollback
3. Shared Debug/Release + NetworkProtocolHarness
4. Server Debug/Release + contract test
5. trigger activation, prerequisite, wave, maxAlive, death/despawn Server harness
6. Client Debug/Release
7. MapTool 생성/저장/재로드/잘못된 문서에서 기존 상태 유지 smoke
8. Lobby -> Valtan Server 진입 후 Stage 1, mini boss, Stage 3, boss 수동 smoke
9. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
10. `git diff --check`

## 7. 완료 경계

UI에 항목만 보이거나 JSON만 저장되는 상태는 완료가 아니다. 실제 Server 접속 발탄 월드에서
Trigger Box 진입으로 몬스터가 생성되고, Server snapshot에 따라 Client 모델과 애니메이션이
표시되며, 전투·사망 후 제거되고, 재접속한 Client가 현재 생존 엔티티만 받는 것까지 확인해야
제품 수직 슬라이스 완료로 기록한다.
