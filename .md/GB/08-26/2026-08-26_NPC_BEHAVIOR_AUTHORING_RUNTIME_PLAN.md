# NPC 행동·순찰·일괄 배치 수직 슬라이스 PLAN

## 1. 목표와 현재 기준

통합 전 target 제품 경로는 `NpcCatalog.json -> Gameplay.world.json formatVersion 5 ->
Publish-WorldGameplay.ps1 -> Server world entity -> Shared snapshot -> CNpc`였다.
Map Tool은 NPC archetype, position, yaw, enabled와 placement별 idle clip 하나만 저작하며 일반
NPC는 Server fixed tick에서 갱신되지 않았다. 현재 통합 대상 catalog는
`runtimeStatus=supported` NPC 75종이고, Bern authoring은 전체 placement 16개 중 NPC 10개다.
기존 placement와 연출을 임의로 바꾸지 않기 위해 이 10개 NPC는 모두 `behavior: null`로 v6에
이관하며, 특정 Aylara/Beda 행동 샘플을 초기값으로 전제하지 않는다.

이번 변경은 기존 경로를 교체하지 않고 다음 계약을 한 번에 닫는다.

1. Map Tool에서 NPC별 제자리 생활 행동, 순찰, 배회와 실제 clip을 편집한다.
2. 순찰 지점, 대기시간, 도착 방향, 반복 방식과 결정적 seed를 저장한다.
3. 여러 NPC archetype pool을 지정 영역에 최소 간격으로 일괄 배치한다.
4. publisher는 의미 행동·경로만 Server에, clip 표현만 Client에 분리한다.
5. Server가 navigation 위에서 위치·방향·행동을 결정한다.
6. Client는 기존 snapshot을 보간하고 `(actionId, actionStartTick)` edge로 clip을 재생한다.
7. 누락 clip은 해당 행동만 기본 idle로 격리하고 world replication을 실패시키지 않는다.

## 2. 저장 계약

`Gameplay.world.json`은 formatVersion 6으로 올린다. NPC placement의 기존 `idleClip`은 유지하고
optional `behavior`를 추가한다. `behavior: null`은 기존 정적 idle NPC와 완전히 같은 의미다.

`behavior`는 다음 필드를 소유한다.

| 필드 | 의미 |
|---|---|
| `mode` | `stationary`, `patrol`, `wander` |
| `routeMode` | `loop`, `pingPong`, `once` |
| `actionSelection` | `sequence`, `weighted` |
| `walkClip` | Client 표현용 실제 clip. Server 산출물에는 포함하지 않는다. |
| `moveSpeed` | Server 이동속도 m/s |
| `wanderRadius` | spawn 기준 배회 XZ 반경. patrol/stationary에서는 0 |
| `randomSeed` | placement가 소유하는 1 이상의 결정적 seed |
| `startDelayMs` | room 시작 뒤 최초 행동 지연 |
| `idleMinMs`, `idleMaxMs` | 행동 사이 Server 대기 범위 |
| `lookTargetPlacementId` | optional 같은 Area NPC placement 참조 |
| `waypoints` | stable waypoint ID, world position, wait ms, optional 도착 yaw |
| `actions` | stable action ID, clip, loop, duration/wait/weight/playback/blend |

검증 상한은 waypoint 64개, action 32개, move speed `0.1..10`, wander radius
`0.5..100`, action duration `1..600000ms`, wait `0..600000ms`, playback `0.1..4`, blend `0..2s`다.
patrol은 waypoint 2개 이상과 walk clip을 요구하고 wander는 양수 반경과 walk clip을 요구한다.
stationary는 waypoint와 wander radius를 가지지 않는다.

## 3. publish 분리

`Publish-WorldGameplay.ps1`은 Gameplay v6을 strict validate하고 같은 transaction에서 다음을 만든다.

- Server `*.worldbootstrap` v7: mode, 경로, 속도, seed, timing, semantic action ID만 포함
- Client `*.npcpresentation.json` v2: placement idle/walk와 action ID별 clip, loop,
  playback rate, blend seconds만 포함

publisher는 NPC placement/action/waypoint/partner 중복과 cross-reference, world entity 256 상한을
검사한다. clip 이름과 model asset 경로는 Server 산출물에 기록하지 않는다.

## 4. Server 계약

`WORLD_BOOTSTRAP_PLACEMENT`는 optional NPC 행동 descriptor를 소유한다.
`SERVER_WORLD_ENTITY`는 authored descriptor 복사본이 아니라 현재 mode, waypoint index, 왕복 방향,
현재 action index, wait 종료 tick, deterministic RNG state를 묶은 NPC runtime state를 소유한다.

새 `CNpcBehaviorRuntime`은 일반 NPC에 대해서만 다음 fixed-tick 순서를 수행한다.

```text
start delay
-> stationary: idle/ambient 선택
-> patrol: 현재 waypoint Find_Path -> path advance -> arrival wait/action
-> wander: spawn 반경 안의 walkable 목적지 선택 -> Find_Path -> wait/action
-> semantic actionId/actionStartTick/position/yaw commit
```

`CMonsterBrain`의 추격·공격 상태는 재사용하지 않는다. `CServerNavigation::Project_Point`,
`Find_Path`와 기존 `SERVER_WORLD_ENTITY::MovePath`만 재사용한다. 필수 waypoint가 nav 밖이거나
구간 도달 불가능하면 room admission을 실패시킨다.

이동은 player와 같은 Server collision primitive를 일반화해 collisionBox와 살아 있는 entity body를
통과하지 않게 한다. body는 stable NetEntityId로 자기 자신을 제외하고 vertical span이 겹치는 같은
층에서만 충돌한다. wander admission과 runtime fallback은 spawn 원 내부로 expansion을 제한한 같은
deterministic BFS를 사용한다. Map Tool도 base navigation에 초기 active runtime blocker를 합친 동일
graph로 waypoint/loop/wander를 사전검사한다.

`WORLD_ENTITY_SNAPSHOT`의 기존 `strActionId`, `iActionStartTick`, position, yaw를 재사용한다.
일반 NPC는 combat `eAction`과 무관하게 semantic action ID를 항상 snapshot에 기록한다.

## 5. Client와 Tool 계약

`CNpcPlacementPresentationService`는 v2 문서를 parse/validate/stage/commit하고 placement별 idle,
walk, action binding을 제공한다. `CClientReplication`은 `(actionId, actionStartTick)`이 바뀐 경우
같은 clip이라도 다시 시작한다. clip 실패는 해당 NPC만 idle로 복구하고 replication 전체 실패로
전파하지 않는다.

`CNpc`는 기존 `CModel`을 유지하면서 root motion을 억제하고, clip loop/playback/blend를 적용한다.
기존 `CCharacter`의 server-tick transform interpolation 의미를 공통 helper 또는 CNpc 전용 상태로
재사용하여 30Hz 위치를 frame presentation으로 보간한다.

Map Tool 선택 NPC inspector에는 다음을 추가한다.

- behavior mode, speed, route/action selection, seed, start/idle timing
- waypoint add/pick/delete/reorder, wait/yaw, route wire preview
- 현재 model이 실제 보유한 clip만 보여 주는 idle/walk/action dropdown
- action add/delete/reorder와 loop/duration/wait/weight/playback/blend
- preview play/pause/restart와 Apply/Revert

일괄 배치는 archetype pool, count, 반경, 최소 간격, seed, random yaw를 입력받아 ghost 결과를
전부 stage한다. navigation, collision box, 기존/신규 NPC 간격, stable placement ID와 256 entity
예산 검증이 모두 성공할 때만 Gameplay document와 preview를 함께 commit한다.
pending ghost가 있으면 Save, Reload와 placement selection을 거부하고 Confirm/Discard만 허용한다.

## 6. 구현 순서

1. G01: Gameplay v6 struct/parser/save/validator와 publisher v7/v2
2. G02: Server bootstrap parser와 `CNpcBehaviorRuntime`, room admission/업데이트
3. G03: Client placement presentation v2, action edge, interpolation, fallback
4. G04: Map Tool 단일 NPC behavior/action/waypoint 편집과 preview
5. G05: Map Tool archetype pool 기반 일괄 배치와 all-or-nothing rollback
6. G06: contract/protocol/client harness, publisher Validate, Debug/Release 회귀
7. G07: 반복 저작을 줄이는 Quick NPC Brush와 one-click animation preset

G07은 Gameplay v6이나 Server/Client runtime 계약을 늘리지 않는다. Map Tool의 기존 placement와
behavior 필드만 조합하며 다음 authoring 편의 기능을 소유한다.

- `Start Continuous NPC Brush` 한 번으로 배치를 무장하고, 이후 world click마다 NPC를 계속 배치한다.
- placement ID는 `npc.<area>.<archetype>.<ordinal>` 형식의 비어 있는 ID를 Tool이 자동 할당한다.
- 선택 NPC를 brush preset으로 캡처하면 idle clip, behavior, ambient action/loop 설정을 스냅샷으로
  보존한다. 새 NPC 선택으로 preset source가 바뀌지 않는다.
- copied look target은 제거하고 random seed는 새 placement ID로부터 결정적으로 다시 만든다.
- patrol preset은 새 spawn delta만큼 waypoint를 평행이동한 뒤 walkable navigation 높이에 다시
  투영한다. 한 waypoint라도 유효하지 않으면 그 click 전체를 rollback한다.
- 실제 model preview에서 고른 clip을 `Stationary Idle Loop` 또는 `Wander Walk`로 한 번에 적용한다.
  clip 이름을 추측하거나 catalog에 없는 animation을 자동 선택하지 않는다.
- Quick Brush도 기존 `parse -> validate -> stage -> commit` presentation transaction과 256 entity
  예산을 그대로 통과해야 한다. 실패한 click은 문서와 preview를 모두 유지한다.
- Save는 여러 click 뒤 사용자가 한 번 명시적으로 수행한다. Tool이 click마다 원본 JSON을 자동
  덮어쓰지는 않는다.

## 7. 완료 증거

- Gameplay JSON/XML parse와 `git diff --check`
- `Publish-WorldGameplay.ps1 -Mode Validate`
- Server contract test: static, patrol loop/ping-pong/once, wait/yaw, wander bounds,
  deterministic seed, invalid nav/ref rollback
- Client focused harness: 같은 clip 재시작, missing clip idle 격리, interpolation, v2 rollback
- NetworkProtocolHarness failures 0
- Engine/Shared/Server/Client Debug·Release 정본 빌드
- 사용자가 Development Map Tool에서 저장/reload와 Server+Client Bern 화면을 직접 확인

책·삽·음료처럼 별도 소품이 필요한 clip은 이번 행동 재생 계약으로 선택할 수 있지만 소품을 자동
생성하지 않는다. prop socket/anchor와 실제 상점·대화 command는 별도 gameplay 수직 슬라이스다.
