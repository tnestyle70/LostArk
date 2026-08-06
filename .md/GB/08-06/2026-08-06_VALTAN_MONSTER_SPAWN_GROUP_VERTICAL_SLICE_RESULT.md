# 2026-08-06 발탄 몬스터 Spawn Group 수직 슬라이스 구현 결과

## 1. 완료 상태

발탄 맵에 배치한 Trigger Box가 Server 권위로 일반 몬스터, 통솔자 루가루, 발탄을 순서대로 활성화하는 제품 경로를 구현했다.

이번 변경은 스테이지를 새 Level이나 `Change Level` 대상으로 만들지 않는다. Bern/Valtan Arena 전환 계약은 그대로 두고, 하나의 `VALTAN_ARENA` 월드 안에서 Trigger action이 stable ID를 가진 Spawn Group 또는 disabled Boss placement를 활성화한다.

```text
MapTool Trigger/Spawn Group 저작
-> Gameplay.world.json + SpawnGroups.world.json
-> Publish-WorldGameplay.ps1
-> Server world/spawn bootstrap
-> Trigger OBB 진입 판정
-> Server wave spawn, AI, 전투, 사망, despawn
-> Shared spawn/snapshot/despawn protocol
-> Client CModel 기반 몬스터 presentation
```

수업용 `CMonster`를 제품 경로로 승격하지 않았고, 기존 `CModel -> CMaterial`과 Server world entity 경계를 확장했다.

## 2. MapTool과 authoring 데이터

### 2.1 Trigger action

Trigger Box는 기존 `movePlayer`, `changeLevel` 외에 다음 두 action을 지원한다.

- `activateSpawnGroup`: `spawnGroupId`로 일반 몬스터/미니보스 그룹 활성화
- `activateEncounter`: disabled Boss placement의 `targetPlacementId`로 발탄 활성화

활성 Trigger Box는 정확히 하나의 지원 action만 소유한다. 존재하지 않는 Spawn Group이나 enabled Boss를 target으로 저장할 수 없도록 교차 검증한다.

### 2.2 Spawn Group 편집

MapTool World Gameplay 화면에 다음 authoring 항목을 추가했다.

- Spawn Anchor 생성, 선택, 위치·회전 편집, 맵에서 좌표 선택
- Spawn Group 생성과 prerequisite, max alive 편집
- Wave 생성과 start delay, next-wave 정책 편집
- Entry별 monster archetype, 수량, anchor, 최초 지연, 생성 간격 편집
- `Save Spawn Groups`, reload, dirty 상태 분리

정본은 `Data/Worlds/<AreaId>/SpawnGroups.world.json`이며 로드는 `parse -> validate -> stage -> commit`, 저장은 임시 파일 후 원자 교체를 사용한다.

## 3. 발탄 데이터 연결

### 3.1 Monster catalog와 profile

다음 stable archetype 네 개를 등록했다.

| 역할 | Archetype ID | 물리 리소스 |
|---|---|---|
| 일반 1 | `MONSTER_VALTAN_PADD_01` | `Character/Monster/NPC_480001_MN_PADD_01` |
| 일반 2 | `MONSTER_VALTAN_SJFC_00_4` | `Character/Monster/NPC_480002_MN_SJFC_00_4` |
| 일반 3 | `MONSTER_VALTAN_0019_05` | `Character/Monster/NPC_480003_MN_0019_05` |
| 미니보스 | `MINIBOSS_LUGARU` | `Character/Monster/NPC_480005_MN_RPRS_02` |

`MonsterCatalog.json`은 model과 idle/chase/attack/dead semantic clip을, `MonsterProfiles.json`은 HP, 공격력, 방어력, 이동 속도, 인식·공격 거리, 공격 단계 시간, 사망 제거 시간을 소유한다.

### 3.2 현재 Stage 연결

기존 사용자가 배치한 Trigger 좌표와 이동 action은 보존했다.

| Trigger placement | 연결 action | 현재 내용 |
|---|---|---|
| `Stage_1` | `spawn.valtan.stage01` | 2 wave, 일반 몬스터 15마리 |
| `Stage_MiniBoss` | `spawn.valtan.stage02.miniboss` | 일반 7마리 전멸 후 루가루 1마리 |
| `Stage_2` | `spawn.valtan.stage03` | 2 wave, 일반 몬스터 19마리 |
| `Stage_3` | 기존 `movePlayer` | 절벽 이동용으로 유지 |
| `Stage_Boss` | `boss.valtan.center` | disabled 발탄 encounter 활성화 |

그룹 prerequisite는 `stage01 -> stage02.miniboss -> stage03` 순서다. 앞 그룹이 모든 wave와 생존 몬스터를 정리하기 전에는 다음 그룹 활성화를 Server가 거부한다.

현재 수량과 profile 값은 원본의 확정 수치가 아니라 실제 플레이 검증을 위한 `PROJECT_TUNED` 시작값이다. MapTool과 JSON 정본에서 조정해야 하며 Server 생성물을 직접 편집하지 않는다.

## 4. Publisher와 Server authority

`Publish-WorldGameplay.ps1`은 Gameplay 문서와 Spawn Group 문서를 함께 검증하고 같은 transaction으로 다음 생성물을 교체한다.

- `VALTAN_ARENA.worldbootstrap`
- `VALTAN_ARENA.spawngroupsbootstrap`

검증 항목은 schema/version, stable ID 중복, anchor/archetype/prerequisite 누락, prerequisite 순환, 수량 상한, trigger target, disabled Boss target이다. 실패하면 기존 Server bootstrap을 유지한다.

Server는 다음을 소유한다.

- Trigger OBB 진입과 trigger-once 소비
- Spawn Group prerequisite, wave, 지연, 간격, max alive
- navigation 위 spawn 좌표 투영
- 가장 가까운 플레이어 추적, chase, attack windup/active/recovery
- 몬스터 공격과 플레이어 skill 피격 damage
- 사망 상태와 authored 지연 후 despawn
- disabled 발탄 placement 활성화

Client는 wave 완료, AI, damage를 결정하지 않는다.

## 5. Shared와 Client presentation

Shared protocol version을 11로 올리고 다음 계약을 추가했다.

- `WORLD_ENTITY_KIND::MONSTER`
- `S2C_WORLD_ENTITY_DESPAWNED`

Client는 Server spawn packet의 archetype ID를 `MonsterCatalog.json`으로 resolve하고 level별 CModel prototype을 lazy load한다. Snapshot action을 idle/chase/attack/dead clip으로 표시하며 despawn packet을 받으면 해당 GameObject를 layer와 replication registry에서 제거한다.

몬스터 presentation shell은 기존 animated `CNpc`의 범용 CModel 재생 기능을 재사용하지만 NPC 제품 archetype 계약과 Monster gameplay 계약은 catalog와 network kind로 분리돼 있다.

## 6. 자동 검증 결과

다음 검증을 실행했다.

- `Publish-WorldGameplay.ps1 -Mode Validate`: PASS
  - Bern 7 placements
  - Valtan 12 placements
  - Training 4 placements
  - Character Select 5 placements
  - Valtan 3 spawn groups
- Shared + NetworkProtocolHarness Debug: PASS, failures 0
- Shared + NetworkProtocolHarness Release: PASS, failures 0
- Server x64 Debug build: PASS
- Server x64 Debug `--contract-test`: PASS, failures 0
- Server x64 Release build: PASS
- Server x64 Release `--contract-test`: PASS, failures 0
- Client x64 Debug build: PASS
- Client x64 Release build: PASS
- 네 JSON 문서 parse: PASS
- Client/Server vcxproj와 filters XML parse: PASS
- `git diff --check`: PASS

Server contract test는 다음을 직접 검증한다.

- `activateSpawnGroup` typed target 전달과 trigger-once
- `activateEncounter` typed target 전달
- 3개 Valtan Spawn Group load
- Stage 1 완료 전 루가루 그룹 활성화 거부
- Stage 1 전체 wave 예약과 전멸 완료
- Stage 1 완료 뒤 루가루 그룹 해금

`ProjectAudit`에서 이번 변경의 다음 항목은 PASS다.

- `world.publish-contract`
- `world.authoring-format-v4`
- `world.publish-generation-rollback`
- `world.monster-spawn-group-contract`
- `world.publish-cleanup`

전체 ProjectAudit는 이번 범위 밖의 기존 문제 네 건 때문에 FAIL 상태다.

1. Data UI JSON 두 개의 vcxproj/filters 노출 누락
2. Effect G09 authoring/runtime boundary 검사
3. DimensionMaster body WModel 누락
4. DimensionMaster runtime animation 검사

이 항목들을 몬스터 구현을 통과시키기 위해 임의 수정하지 않았다.

## 7. 수동 실행 확인 절차

제품 동작은 Server 권위이므로 MapTool Test 화면만으로 몬스터를 생성할 수 없다.

```text
1. Server Debug 실행
2. Client Debug 실행 (작업 디렉터리 Client/Default)
3. Lobby -> Valtan 진입
4. Stage_1 Trigger Box 진입
5. 일반 몬스터 wave 생성, 이동, 공격, 피격, 사망, 제거 확인
6. Stage_MiniBoss 진입 후 일반 wave와 루가루 생성 확인
7. Stage_2 진입 후 마지막 일반 wave 확인
8. Stage_Boss 진입 후 발탄 생성 확인
```

MapTool에서 anchor, 수량, 시간 값을 바꾼 뒤에는 `Save Gameplay`와 `Save Spawn Groups`를 각각 저장하고 Server를 다시 빌드·재시작해야 runtime bootstrap에 반영된다.

## 8. 남은 수동 검증 경계

자동 빌드와 계약 검증은 완료했지만 실제 Client를 조작해 Trigger를 차례로 밟는 육안 smoke는 사용자 입력이 필요하므로 남아 있다. 수동 smoke에서 우선 확인할 항목은 다음과 같다.

- anchor 위치가 지면과 겹치거나 절벽 밖으로 투영되지 않는지
- model scale/yaw와 idle/chase/attack/dead clip이 올바른지
- 실제 전투 속도와 몬스터 수량이 과하지 않은지
- 여러 플레이어가 동시에 Trigger에 진입해도 한 번만 활성화되는지
- 마지막 몬스터 despawn 뒤 다음 prerequisite가 해제되는지

이번 작업은 `Client/Bin/Resources`의 몬스터 리소스 payload를 변경하지 않았다. 기존에 추출·배포된 `Character/Monster/NPC_480001...480005` 폴더를 그대로 소비한다.
