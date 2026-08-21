# 2026-08-20 발탄 보스 전투·패턴 그래프 전체 적용 계획

branch: `codex/valtan-boss-combat-runtime`

## 2026-08-20 G05-1 체크포인트

- 완료: G01~G04의 보스 피격 runtime, stage 결과 분기, 갑옷 내구도/피해 감소,
  무력화·보호막·카운터, Shared snapshot/event, Client 갑옷/HUD projection.
- 완료: G05-1의 room-owned combat object runtime, 하늘 도끼·이동 검기,
  Shared v27 lifecycle/snapshot, Client world-root effect projection.
- 빌드: Engine/Shared/전체 harness project/Server/Client Debug·Release 컴파일과 링크 성공.
- 다음 재개점: G05-6의 비석 차폐·제품 파괴와 침묵·감금·잡기 상태부터 시작한다.
- 아직 완료 아님: 3:1 패턴 cadence, typed phase commit/pool, 유령 발탄·분신,
  비석 차폐·상태 이상, 전체 시각 검증은 후속 범위다.

## 2026-08-21 G05-1 재감사 보완

완료 보고 뒤 계획 계약과 실제 runtime을 다시 비교해 다음 세 공백을 같은 G05-1 범위에서
보완한다.

- 같은 fixed tick에 due인 player projectile는 각각 commit하지 않고 하나의 transaction에
  모두 stage한다. capacity 또는 두 번째 definition 검증이 실패하면 live object, lifecycle,
  projectile mask를 전부 그대로 보존하며, 성공한 뒤에만 due mask를 일괄 commit한다.
- live combat object는 owner pattern sequence/pattern ID/stage action ID와 previous/current pose를
  지속 상태로 소유한다. player source에는 boss occurrence metadata를 넣지 않고, boss source는
  유효한 non-zero occurrence sequence를 필수로 한다.
- LOCK target이 TAKEOFF 뒤 사라지거나 DEAD/FALLING이 되어도 HIGH_JUMP는 다른 player로
  재지목하지 않는다. Brain이 마지막 유효 Server 위치를 보존하고 AIRBORNE ENTER가 그 위치로
  object를 생성해 첫 pulse까지 고정한다.

로컬 실행 정본도 `127.0.0.1:7777`로 통일한다. Client/Server 공용 debugger 기본값과 Server
listener 기본값을 loopback으로 맞추고, endpoint 동기화 도구는 `LOSTARK_SERVER_HOST` 한 행만
정규화해 `LOSTARK_RESOURCE_ROOT` 같은 다른 user 환경 변수를 보존한다.

현재 제품 발탄은 `CValtanBrain`의 Server fixed-tick 선택기와
`ValtanEncounter.json`의 선형 stage 목록으로 실행된다. 이 작업은 별도 행동트리나
Client 판정 경로를 만들지 않고, 현재 경로를 결과 분기형 encounter runtime으로 확장한다.

기존 미커밋 휠윈드 stage 분할, 갑옷 presentation 연결, 1~67 audition 후보 승격,
비석 runtime, 맵·렌더링 변경은 다른 작업의 입력으로 간주해 되돌리거나 전체 파일로
덮어쓰지 않는다. 변경은 실제 선언·함수 기준의 좁은 patch로만 적용한다.

## G00. 권위와 실행 경계

제품 실행 경로는 다음 하나를 유지한다.

```text
PlayerSkillSystem landed hit
-> CBossCombatRuntime
-> HP / shield / stagger / part durability / counter outcome
-> CValtanBrain pattern-stage branch
-> CGameRoom typed stage action and world transaction
-> WORLD_ENTITY_SNAPSHOT persistent state
-> CValtan / CombatHUD presentation
```

- Server는 stable boss, pattern, stage action, part ID만 소유한다.
- Client model path, animation clip, bone 이름은 Server 데이터에 들어가지 않는다.
- `Valtan.patternpreview.json`과 `Valtan.clipseq`는 계속 검토 자료이며 제품 실행 입력이 아니다.
- `ValtanDebugAudition.json`은 제품 패턴 ID를 강제로 순서대로 실행하는 Debug ledger다.
- 일반 패턴 선택, 단일 패턴 audition, 1~67 ordered audition을 서로 다른 실행 모드로 유지한다.
- 선배 영상의 체력줄 숫자는 구조 참고 자료다. 제품 트리거는 현재 1~67 정본의
  `159, 115, 109, 100, 84, 73, 62, 30, 14`를 유지한다.

## G01. Data와 catalog 계약

### 수정·추가 파일

| 구분 | 경로 | 역할 |
|---|---|---|
| 수정 | `Data/Encounters/Valtan/ValtanEncounter.json` | v4 pattern policy, stage branch/action/motion |
| 추가 | `Data/Balance/ValtanBossParts.json` | stable logical part와 durability/reduction |
| 수정 | `Data/Balance/PlayerSkills.json` | landed hit의 stagger/part/counter power |
| 수정 | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | exact validation과 bootstrap row emit |
| 수정 | `Tools/GameplayPipeline/Export-OfficialBalanceReceipt.py` | 새 authored field provenance |
| 수정 | `Server/Public/GameplayCatalog.h` | compiled definition 공개 계약 |
| 수정 | `Server/Private/GameplayCatalog.cpp` | parse -> validate -> stage -> commit |
| 수정 | `Server/Private/ServerGameplayContractTests.cpp` | malformed/rollback와 runtime contract |
| 수정 | `Client/Default/Client.vcxproj(.filters)` | 새 Data 원본을 `96.DataFiles`의 `None`으로 등록 |

### enum과 compiled row

```text
BOSS_PATTERN_CATEGORY
  NORMAL / IMPORTANT / MECHANIC

BOSS_PATTERN_TARGET_POLICY
  NONE / NEAREST_EACH_TICK / LOCK_NEAREST_ON_START /
  LOCK_RANDOM_ALIVE_ON_START

BOSS_PATTERN_AIM_POLICY
  NONE / TRACK_TARGET_EACH_TICK / LOCK_FACING_ON_START /
  FACE_MOTION_ANCHOR

BOSS_PATTERN_STAGE_OUTCOME
  TIMEOUT / COUNTER_HIT / STAGGER_BROKEN / WALL_CONTACT /
  PART_DESTROYED / PROP_DESTROYED / SUMMON_DEAD /
  ALL_PLAYERS_GRABBED

BOSS_PATTERN_STAGE_ACTION_TRIGGER
  ENTER / EXIT

BOSS_PATTERN_STAGE_ACTION_KIND
  SET_BOSS_FLAG

BOSS_PATTERN_STAGE_MOTION_KIND
  NONE / FORWARD
```

기존 `stageId`와 `actionId` 중 런타임 분기 identity는 이미 제품 binding과 strict join되는
`actionId`를 사용한다. branch target은 같은 pattern 안의 action ID 또는 종료 sentinel만
허용한다. branch가 없는 stage는 publisher가 기존 선형 동작과 같은 TIMEOUT branch를
컴파일한다.

bootstrap 추가 row는 다음과 같다.

```text
PATTERNPOLICY
PATTERNSTAGEBRANCH
PATTERNSTAGEACTION
PATTERNSTAGEMOTION
BOSSPART
SKILLCOMBATTRAITS
```

publisher는 branch target, outcome 중복, TIMEOUT 경로, action order, part mask power-of-two,
mask/ID 중복, boss owner, total reduction < 100, skill owner와 수치 범위를 검증한다.
catalog load 실패는 기존 committed catalog 전체를 유지한다.

### `ValtanBossParts.json` 전체 코드

```json
{
  "schema": "lostark.valtan-boss-parts",
  "formatVersion": 1,
  "bossArchetypeId": "BOSS_VALTAN",
  "parts": [
    {
      "partId": "boss.part.valtan.shoulder-armor",
      "stateMask": 1,
      "maximumDurability": 1000,
      "damageReductionPercent": 15,
      "partDamageCondition": "GROGGY_ONLY"
    },
    {
      "partId": "boss.part.valtan.arm-armor",
      "stateMask": 2,
      "maximumDurability": 1000,
      "damageReductionPercent": 15,
      "partDamageCondition": "GROGGY_ONLY"
    }
  ]
}
```

위 내구도와 감소율은 공식 수치 주장이 아니라 `PROJECT_TUNED` 게임플레이 초기값이다.
Balance provenance receipt가 그 근거 종류를 명시한다.

## G02. Server boss combat runtime과 첫 수직 슬라이스

### 수정·추가 파일

| 구분 | 경로 | 역할 |
|---|---|---|
| 추가 | `Server/Public/BossCombatRuntime.h` | 보스 persistent combat state와 hit/outcome 계약 |
| 추가 | `Server/Private/BossCombatRuntime.cpp` | 피해·갑옷·shield·stagger·counter 계산 |
| 수정 | `Server/Default/Server.vcxproj(.filters)` | 새 H/CPP 물리 등록 |
| 수정 | `Server/Public/ServerWorldEntity.h` | boss combat state, stage motion과 target lock |
| 수정 | `Server/Private/PlayerSkillSystem.cpp` | boss landed hit를 combat runtime에 위임 |
| 수정 | `Server/Public/ValtanBrain.h` | generic stage motion/outcome API |
| 수정 | `Server/Private/ValtanBrain.cpp` | policy, branch, stage action, forward motion |
| 수정 | `Server/Public/GameRoom.h` | stage action과 boss event tick state |
| 수정 | `Server/Private/GameRoom.cpp` | 초기화, wall outcome, snapshot/event commit |

`BOSS_INCOMING_HIT`은 source player/skill, HP damage, stagger, part, counter power,
server tick과 source XZ를 소유한다. `BOSS_HIT_RESULT`는 실제 HP/shield/stagger/part
변화와 발생한 outcome을 반환한다.

`SERVER_BOSS_PATTERN_OUTCOME_SIGNAL`은 `patternId + actionId + patternSequence`에
귀속한다. Brain은 현재 action의 external outcome을 TIMEOUT보다 먼저 소비한다.
이전 pattern/sequence 신호는 다음 stage에 전파하지 않는다.

갑옷 opening은 다음 그래프로 실행한다.

```text
PREPARE -> AIM_LOCK -> WALL_CHARGE
WALL_CHARGE: WALL_CONTACT -> GROGGY
WALL_CHARGE: TIMEOUT -> FINISH
GROGGY ENTER: boss.flag.groggy = true
GROGGY: PART_DESTROYED or TIMEOUT -> RECOVERY
GROGGY EXIT: boss.flag.groggy = false
```

일반 timeout이 GROGGY로 들어가던 기존 하드코딩을 제거한다. 충돌 transaction이 실제
commit된 뒤에만 `WALL_CONTACT`를 publish한다. GROGGY 중 landed part damage만 내구도를
줄이고 0에서 alive mask를 제거한다.

## G03. Shared persistent state와 Client projection

### 수정 파일

```text
Shared/Public/Network/PacketType.h
Shared/Public/Network/PacketMessages.h
Shared/Private/Network/PacketMessages.cpp
Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp
Client/Public/ClientReplication.h
Client/Private/ClientReplication.cpp
Client/Public/Valtan.h
Client/Private/Valtan.cpp
Client/Public/CombatHUDViewModel.h
Client/Private/CombatHUDViewModel.cpp
Client/Private/MainApp.cpp
```

protocol은 현재 25에서 26으로 올린다. 기존 top-level `iPhase`는 호환을 위해 유지하고
boss combat state의 gameplay phase와 동일함을 검증한다.

```text
BOSS_COMBAT_SNAPSHOT
  stateRevision
  alivePartMask
  flags: INVULNERABLE / SHIELDED / COUNTERABLE / GROGGY
  staggerCurrent / staggerMaximum
  shieldCurrent / shieldMaximum
  gameplayPhase

BOSS_COMBAT_EVENT
  eventSequence
  eventTick
  bossNetEntityId
  PART_BROKEN
  partMask
```

snapshot은 late join의 최종 상태를 복구하고 event는 live edge를 중복 없이 소비한다.
Client는 성공한 attach 순서로 vector를 압축하지 않고 wire bit별 고정 part tag를 보존한다.
part load 실패는 해당 presentation만 격리하고 Server gameplay를 막지 않는다.
HUD는 기존 stagger slot에 live current/max만 투영하며 새 가짜 image slot을 만들지 않는다.

## G04. 무력화·보호막·카운터

- `SET_BOSS_FLAG`에 더해 실제 소비자가 있는 `SET_STAGGER_GAUGE`, `SET_SHIELD` action을
  같은 typed action 계약으로 추가한다.
- `SET_STAGGER_GAUGE(value)`는 current를 0으로 초기화하고 maximum을 value로 설정한다.
  value 0은 gauge를 clear한다.
- `SET_SHIELD(value)`는 shield current/maximum과 SHIELDED flag를 한 transaction으로
  설정한다. value 0은 gauge와 flag를 함께 clear한다.
- `VALTAN_MAGIC_ORB_STAGGER_76`의 실제 73줄 패턴은 SHIELD/WINDOW에서 HP 무적과
  stagger gauge를 활성화한다. `STAGGER_BROKEN -> GROGGY`, `TIMEOUT -> WIPE`로 분기하고
  종료 시 flag와 gauge를 모두 clear한다.
- PARRY는 STANCE 중 stagger를 받는다. `STAGGER_BROKEN -> COUNTER_SLASH`,
  `TIMEOUT -> NORMAL_SLASH`로 나눠 선배 구현의 광역 반격과 일반 공격을 구분한다.
- outgoing player hit의 `counterPower`만 boss `COUNTERABLE` window를 닫는다.
- 기존 `CPlayerSkillSystem::Try_Counter`의 player defensive counter와 boss counter 판정을
  합치지 않는다.
- CENTER_GRAB_COUNTER와 TRIPLE_COUNTER의 counter window는 ENTER/EXIT에서
  COUNTERABLE을 켜고 끈다. `COUNTER_HIT`은 성공 stage로, TIMEOUT은 기존 fail attack으로
  이동한다.
- 새 stagger GROGGY/WIPE와 parry NORMAL_SLASH action ID는 기존 의미가 같은 제품 clip을
  Client `patternbindings`에서 재사용하되 candidate presentation임을 자동 검증과 수동
  육안 판정에서 구분한다.

## G05. Combat object, prop cover와 player status

### G05-1. 첫 수직 슬라이스의 종료 범위

- 플레이어별 projectile 코드를 복사하지 않고 `CGameRoom`이 하나만 소유하는
  `CCombatObjectRuntime`으로 실제 이관한다. `SERVER_PLAYER::Projectiles`와
  `CPlayerSkillSystem::Update_Projectiles`는 이관 뒤 제거한다.
- 첫 제품 소비자는 `VALTAN_HIGH_JUMP`의 대상 추적 하늘 도끼와
  `VALTAN_RED_BLADE_WAVE`의 이동 검기다. 추적 폭발, portal/ghost rush는 같은 정의와
  runtime을 후속 데이터에서 사용한다.
- 기존 HIGH_JUMP LAND와 RED_BLADE PROJECTILE stage의 보스 중심 hit는 `NONE`으로
  바꾸고 combat object hit만 피해 권위를 가져 이중 피해를 금지한다.
- Shared reliable spawn/despawn과 30 Hz full live transform snapshot, Client visual-only
  projection을 같은 변경 단위에 넣는다. Client collider나 damage 판정은 추가하지 않는다.

### G05-2. Data와 compiled catalog

새 정본 `Data/Encounters/Valtan/ValtanCombatObjects.json`은 gameplay stable ID와 수치만
소유한다. asset path와 clip 이름은 넣지 않는다.

```text
combatobject.valtan.high-jump.target-axe
  owner: VALTAN_HIGH_JUMP / valtan.attack.high-jump.airborne
  kind: FIXED_AREA
  target: LOCKED_TARGET_UNTIL_FIRST_PULSE
  life: 1900 ms
  timed pulse: 1200 ms, CIRCLE radius 10
  기존 high-jump damage/push/knockdown 수치를 그대로 이전

combatobject.valtan.red-blade-wave.projectile
  owner: VALTAN_RED_BLADE_WAVE / valtan.attack.red-blade-wave.active
  kind: MISSILE
  origin: BOSS_POSITION
  direction: PATTERN_FACING_AT_SPAWN
  forward offset: 3 m
  speed: 24.444445 m/s, max distance: 22 m, life: 900 ms
  swept CONTACT CIRCLE radius 2, 대상별 1회
```

`BOSS_PATTERN_STAGE_ACTION_KIND`에는 실제 소비자가 함께 들어오는
`SPAWN_COMBAT_OBJECT`만 추가한다. action은 ENTER, value 1, duration 0이며 정의와 action은
양방향 1:1 exact join이다. generated gameplay bootstrap은 v13의 다음 두 row family를
추가하고 직접 편집하지 않는다.

```text
BOSSCOMBATOBJECT
BOSSCOMBATOBJECTHIT
```

publisher와 `CGameplayCatalog`는 owner encounter/pattern/action, target/aim policy,
damage profile, contiguous hit index/count, finite motion, life 안의 pulse, max travel,
inline stage hit 부재와 stable ID를 parse -> validate -> stage -> commit으로 검사한다.
malformed reload는 기존 catalog를 보존한다.

Client 표현 join은 `BossCatalog.json`의 boss별
`combatObjectVisuals[{combatObjectArchetypeId,visualId,effectAssetId}]`가 소유한다.
Server bootstrap에는 `clientVisualId`만 들어가고 effect/model 경로는 들어가지 않는다.

### G05-3. 하나의 Server runtime

새 `Server/Public|Private/CombatObjectRuntime.h/.cpp`를 Server project/filter에 등록한다.
runtime live object는 vector index나 definition pointer가 아니라 0을 예약한 uint64 instance ID,
source/target side, source entity/player, locked target, owner pattern sequence/action,
spawn tick, 이전/현재 pose, motion, life, resolved immutable hit pulse와 contact ledger를 소유한다.

`CPlayerSkillSystem`은 PlayerSkills projectile를 resolved spawn request로 바꾸는 adapter만 맡고,
boss stage ENTER도 GameplayCatalog definition을 같은 request로 바꾼다. 두 source 모두
`Prepare_SpawnBatch -> Commit` 한 transaction을 사용한다. 실패하면 spawned mask, boss state,
live vector와 wire event가 모두 바뀌지 않는다.

공용 fixed-tick 순서는 다음과 같다.

```text
Update_Players: direct hit와 player combat-object spawn commit
-> CCombatObjectRuntime::Update: motion -> tracking -> hit -> expiry
-> trigger / spawn group / Esther
-> Update_WorldEntities: boss brain과 stage transition, boss object spawn commit
-> prop / world destruction commit
-> reliable lifecycle flush
-> full world snapshot
```

spawn tick에는 이동/공격하지 않는다. 하늘 도끼는 `iPatternTargetEntityId`로 잠근 대상만
첫 pulse 전까지 추적하고 대상이 사라지면 마지막 유효 XZ를 유지한다. 첫 pulse 뒤에는 위치를
고정한다. 붉은 검기는 segment-vs-expanded-circle swept 판정을 사용해 한 tick에 대상을
건너뛰어도 접촉을 놓치지 않는다.

새 `ServerCombatHitRuntime`은 기존 player direct/projectile, monster, Valtan hit의 방어·HP·
사망·넉백·boss combat hook을 한 typed resolver로 묶는다. combat object runtime은 source별
피격 규칙을 복사하지 않고 이 resolver를 소비한다.

### G05-4. Shared v27과 Client projection

Shared에는 generic `COMBAT_OBJECT_ID`와 최대 128개의 replicated object를 둔다.

```text
S2C_COMBAT_OBJECT_SPAWNED
  instanceId, sourceNetEntityId, spawnTick,
  combatObjectArchetypeId, clientVisualId, initial pose/yaw

COMBAT_OBJECT_SNAPSHOT
  instanceId, sourceNetEntityId, pose/yaw

S2C_COMBAT_OBJECT_DESPAWNED
  instanceId

S2C_WORLD_SNAPSHOT::CombatObjects
  현재 살아 있는 replicated object 전체 집합, instanceId 오름차순/unique
```

player object는 첫 slice에서 기존과 같이 replication NONE이고 boss object만 TRANSFORM으로
복제하지만 둘은 Server의 같은 live vector와 update kernel을 사용한다. join은 boss spawn 뒤
live object spawn을 ID 순으로 보내고 첫 snapshot을 보낸다. lifecycle frame은 snapshot
coalescing의 barrier이며 duplicate spawn은 전체 identity가 같을 때만 idempotent다.

Client의 새 `CCombatObjectProjectionRuntime`은 logical lifecycle과 effect handle만 소유한다.
실제 렌더는 두 번째 projectile GameObject 경로를 만들지 않고 기존
`CEffectPresentationService -> CEffectObject`에 world-root handle API를 추가해 재사용한다.
snapshot batch가 unknown/missing/conflicting ID를 포함하면 어떤 root도 갱신하지 않는다.
effect clone 실패는 해당 visual handle만 0으로 격리하고 logical object와 이후 despawn 소비는
유지한다.

- red blade는 기존 `effect.valtan.red-blade-wave.active`를 이동 root에서 재사용하고 기존
  boss-root active cue 한 행만 제거한다.
- sky axe는 `Character/Valtan/ValtanWeapon.wmodel`을 mesh carrier로 쓰는
  `effect.valtan.sky-axe.active` v13 문서를 추가한다. effect codec/publisher는 meshModel slot의
  safe `Character/*.wmodel`만 허용하고 texture/path escape에는 계속 허용하지 않는다.
- Loading/Character Select prewarm은 Valtan pattern cue와 combat-object visual target을
  sort/unique해 함께 준비한다.

### G05-5. 첫 slice 자동 검증

```text
Gameplay publisher Validate / Publish, bootstrap v13
GameplayCatalog definition/action 양방향 join과 malformed rollback
기존 player projectile의 spawn tick/damage split/fixed-area/retire 무회귀
sky axe target lock/no retarget/pulse 뒤 freeze/no double hit
red blade swept crossing/behind miss/target별 1회
source leave/death/reset cleanup와 transactional cap failure
Shared Debug/Release, NetworkProtocolHarness Debug/Release failures 0
Client projection late join/dedupe/batch rollback/reset fake-sink harness
Effect publisher, exact BossCatalog visual join과 prewarm set
Server/Client Debug/Release build
```

### G05-6. 다음 slice 경계

- 비석 slot은 stable XZ/반경을 소유하고 attack cover policy는
  `RESPECT_ACTIVE_PROP / IGNORE_COVER / BREAK_PROP`로 닫는다.
- `SILENCED / IMPRISONED / GRABBED`는 `PLAYER_ACTION_STATE`가 아닌 status flags와
  expiry/source/attachment metadata로 복제한다.
- grab은 Server logical `BOSS_LEFT_HAND`만 소유하고 Client catalog가 verified bone/offset으로
  표현한다.

## G06. phase scheduler, motion과 ghost phase

- HP 감소는 phase 값을 직접 바꾸지 않는다. 현재 1~67 정본에서 109줄은 추가 외벽
  붕괴이고 100줄이 2페이즈 전환이므로, `VALTAN_FOUR_PILLARS_105`의 실제 100줄 terminal
  action에서 `COMMIT_PHASE(2)`를 실행한다. 선배 영상의 109 전환 수치를 제품 정본에
  덮어쓰지 않는다.
- scheduler priority는 death, current outcome, running pattern, forced mechanic,
  important quota, weighted normal, chase 순서다.
- phase 2의 일반 pattern 정상 완료 3회마다 IMPORTANT 1회를 실행한다.
- forced HP mechanic은 quota를 증가시키거나 초기화하지 않는다.
- motion은 `MOVE_TO_ANCHOR`, `CHARGE_UNTIL_IMPACT`, `TELEPORT_TO_TARGET_OFFSET`,
  `ROTATE_HEADING`, `FIXED_ANCHOR`를 실제 Server transform path로 확장한다.
- phase 3은 같은 boss entity의 gameplay phase와 presentation variant를 우선 사용한다.
  별도 HP/lifecycle이 필요할 때만 두 번째 boss archetype을 사용한다.
- ghost clone은 owner boss를 가진 dependent encounter actor다. 10초 ghost rush는 boss의
  exclusive pattern이 아니라 room-owned parallel phase timer다.

## G07. 검증과 완료 조건

```text
Gameplay publisher Validate / Publish
GameplayCatalog malformed and rollback contract
Server Debug / Release build
Server contract: armor reduction, wall outcome, timeout, part break, stagger,
counter, target lock, phase pool, 3:1 quota, prop cover, status, ghost timer
Shared Debug / Release build
NetworkProtocolHarness Debug / Release failures 0
Client Debug / Release build
ClientFrontendHarness state/part/HUD/late-join/reset
JSON/XML parse
git diff --check
```

자동 검증은 Server 권위, 순서, 분기, 상태 복구를 증명한다. 애니메이션, 갑옷 파괴 이펙트,
카메라, 비석과 유령의 visual fidelity는 사용자가 Server와 Client를 직접 실행해 판정한다.
