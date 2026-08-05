# Map Gameplay Trigger·Destroyable·NPC·동적 Navigation 데이터 확장 PLAN

> 2026-08-05 부분 구현: world authoring formatVersion 2 migration과
> `CWorldGameplayDocument` triggerBox/destroyable parse/validate/atomic save 구조까지 반영됐다.
> publisher/Server/dynamic nav/replication/Client presentation/MapTool widget은 아직 이 PLAN의 TARGET이며
> 제품 publisher가 신규 kind를 fail-closed로 거부한다. 실제 상태는
> `2026-08-05_BALANCE_PROVENANCE_IMGUI_MAP_DATA_RESULT.md`를 따른다.

작성일: 2026-08-05
상태: 구현 전 정본 (범위·계약 확정용 상위 계획. G별 구현 착수 시 이 문서를 기준으로 G별 전체 코드 PLAN을 분리 작성한다)
복원 대상 이전 작업:
- `.md/GB/07-31/2026-07-31_LOSTARK_RAID_AUTHORING_CAMERA_NAVIGATION_TRIGGER_PLAN.md` (condition 3층 소유권, `Set_Condition` public endpoint, `CNavigationConditionRuntime` 설계)
- `.md/GB/07-30/2026-07-30_LOSTARK_VALTAN_DYNAMIC_ENVIRONMENT_DEPLOY_RUNTIME_PLAN.md` / `_RESULT.md` (DeployProp catalog/placement, INTACT/FRACTURED/DESPAWNED, ANIM on/off)
- `.md/GB/07-30/2026-07-30_LOSTARK_VALTAN_DESTRUCTIBLE_ENVIRONMENT_SKY_AUDIT_RESULT.md` (필수 저장 계약, 모델 없는 상태 레코드 보존)
- `.md/GB/08-04/2026-08-04_DEVELOPMENT_MAP_EDITOR_WORKSPACE_PLAN.md` / `_RESULT.md` (editor workspace 경계, DeployProp authoring 유예, trigger 유예)

## G00. 실측 결론과 범위

### 사용자 요구 → 현재 상태 매핑

| 요구 | 현재 실측 | 이 계획의 처리 |
|---|---|---|
| player spawn 위치 지정 | 완료. MapTool `World Gameplay` 탭이 `playerSpawn` 배치·저장 지원 (`CMapTool::Try_PlaceWorldGameplay`, MapTool.cpp:676) | 변경 없음 |
| Valtan(boss) spawn 위치 지정 | 완료. 같은 탭의 `boss` kind + `BOSS_VALTAN`/`ENCOUNTER_VALTAN` (Data/Worlds/LV_LUT_HEARTRB_ED) | 변경 없음 |
| NPC 설치 | 절반. schema/publisher/Server spawn은 지원하나 `NpcCatalog.json`이 비어 있고 `CClientReplication::Apply_WorldEntitySpawn`이 BOSS 외 kind를 거부 (ClientReplication.cpp:326-330). `CNpc`는 인스턴스화 경로 없음 | G04 |
| 트리거 박스 설치 (ID 기반) | 없음. 세 층(publisher/Server/Client) kind가 `playerSpawn\|npc\|boss`로 닫힘 | G01+G03+G05 |
| destroyable placement id | 시각 계층만 존재. `.deployplacements`가 stable `runtimePlacementId(uint64)` 85건 소유, `CDeployPropObject` 상태 전환 실존. gameplay/Server 계약 없음, 현재 로드 호출자 0곳 | G01+G03+G04 |
| 트리거 발동 → 애니메이션 재생 | 조각만 존재. `CDeployPropObject::Set_State`가 ANIM kind의 "on"/"off" non-loop clip 재생을 이미 구현. 트리거·복제 경로 없음 | G03+G04 |
| 유동적 walkable cell 변화 | Client 조각만 존재. `.navblockers`(region id+condition id+cells) 편집과 `CNavGrid::Register_RuntimeBlocker/Set_RuntimeBlockerActive`는 실존하나 publisher가 `.navblockers`를 읽지 않고 Server `CServerNavigation`에는 동적 API가 전혀 없음 | G02+G03 |

### 복원하는 이전 결정 (그대로 유지)

1. **condition 3층 소유권** (07-31): 정적 walkable은 `.navgrid`, 동적 영역 정의는 `.navblockers`의 `NAV_RUNTIME_BLOCKER_REGION { id, conditionId, activateWhenConditionTrue, cells }`, 현재 상태는 runtime owner의 `conditionId → bool` 맵. cell이 파괴 오브젝트 포인터를 저장하지 않는다.
2. **파괴 사건의 이중 전달** (07-31): 시각 상태는 `CDeployPropObject`(INTACT/FRACTURED/DESPAWNED), 이동 가능 상태는 condition runtime이 각각 소유하고, 트리거는 두 owner에게 같은 사건을 별개 event로 전달한다. 자동 연동(파괴→condition 암묵 발화)은 만들지 않는다.
3. **public endpoint는 `Set_Condition(conditionId, bool)` 하나** (07-31): 보스 패턴·트리거·에디터 테스트가 같은 함수를 호출한다.
4. **파괴 에셋별 `CMapDestroyableAsset` 클래스를 만들지 않는다** (07-31 §1.2).
5. **필수 저장 계약** (07-30 audit): stable `placementId`/`deployActorId`, intact/fractured 또는 skeletal 모델 경로, transform, 초기 상태·파괴 후 상태·트리거 바인딩, `parse -> validate -> stage -> commit` + 전체 rollback, `CModel -> CMaterial` 단일 경로. 모델 없는 상태 레코드(Prop `375303`)는 빈 메시로 생성하지 않고 데이터로만 보존.
6. **editor workspace 경계** (08-04): MapTool 저장 대상은 `Data`뿐, runtime 산출물은 publisher만 교체, editor shell은 runtime blocker를 등록하지 않는다.

### 이 계획이 갱신하는 기존 경계

08-02 §0과 08-04 PLAN의 "monster/wave/**trigger**를 새로 만들지 않는다"는 *실제 요구와 수직 슬라이스 없이 placeholder만 추가하는 것*의 금지다. 본 계획은 AGENTS.md가 요구하는 형식 그대로 — schema, publisher, Server 판정, replication, Client presentation, MapTool authoring, harness를 한 묶음의 수직 슬라이스들로 — trigger/destroyable 계약을 추가하므로 이 경계의 정당한 해제 절차다. 일반 monster/wave/증분 spawn은 여전히 범위 밖이며, G06에서 AGENTS/CLAUDE/AREA_DATA_LAYER_GUIDE의 해당 문장을 실제 구현 상태로 갱신한다.

### 결정과 대안 (조사 근거 포함)

| 결정 | 채택 | 기각 대안과 이유 |
|---|---|---|
| trigger 저장 위치 | `Gameplay.world.json` formatVersion 2의 새 kind `triggerBox` | 별도 `Gameplay/Triggers.json`(08-02 §4 제안): placement 문서·publisher·bootstrap·MapTool 탭·stable ID 파이프라인 전체가 이미 Gameplay.world.json에 있어 kind 확장이 두 번째 placement 경로를 만들지 않는 최소 변경 |
| destroyable의 gameplay identity | 새 kind `destroyable` placement가 `deployPlacementId`(uint64)로 `.deployplacements` 레코드를 참조 | deploy placement 전체를 Server로 승격: Server가 모델 경로를 알게 되어 담당 경계 위반. 85건 중 gameplay 판정이 필요한 것만 gameplay 문서에 올리는 참조 방식이 계층 분리 유지 |
| entity 상태 복제 | `WORLD_ENTITY_SNAPSHOT`/`S2C_WORLD_ENTITY_SPAWNED`에 명시 필드 `iStateValue(u8)` 추가 (protocol 8) | 기존 `iPhase` 재활용: kind별 의미가 갈라지는 암묵 계약이 되어 "모르는 enum을 정상값처럼 처리하지 않는다" 원칙 위반 |
| condition 상태 복제 | 하지 않는다 | 제품 Client는 네트워크 상태에서 pathfinding을 하지 않고(`CCharacter::Request_Move`는 비네트워크 한정, Character.cpp:432-439) 이동은 snapshot을 따른다. 시각 변화는 destroyable 상태 복제로 충분. Server만 walkable 진실을 소유 |
| trigger box의 snapshot 노출 | 하지 않는다 (Server 내부 판정 전용) | trigger를 world entity로 복제: Client presentation이 없는 개념을 snapshot에 실을 이유가 없음. 디버그 표시는 MapTool overlay가 담당 |
| trigger 발화 의미 | `triggerOnce=true`는 room 수명 1회, `false`는 내부 player 전원 이탈 시 재장전(edge trigger) | once만 지원: 반복 트리거(재개폐 문 등)에 대한 저장 형식 재변경을 피하기 위해 필드는 지금 확정 |

### 이 계획의 범위 밖 (명시 유예)

- 일반 monster archetype/wave/증분 spawn, trigger의 `spawnEntity` 계열 event
- NPC/boss 애니메이션을 트리거로 직접 지정하는 event(`playAnimation` 타입) — destroyable 상태 전환의 on/off clip으로 1차 요구를 닫은 뒤 별도 슬라이스
- Deploy placement의 transform authoring UI(신규 배치·이동·삭제) — G05는 기존 85건을 **읽기 전용 바인딩 대상**으로만 노출. exact source/stage/save 편집 계약은 08-04 유예 그대로 별도 슬라이스
- TriggerMapData/Matinee 원작 타이밍 자동 재생, `ITR_02326_Ani` AnimSet 복구
- Server world entity despawn packet(`S2C_WORLD_ENTITY_DESPAWNED`) — DESPAWNED는 상태 값으로 표현하며 entity 제거는 이번 범위에서 하지 않는다

## G01. `lostark.world-gameplay` formatVersion 2 — triggerBox·destroyable kind

### 목표와 종료 증거

`Gameplay.world.json`이 5개 kind(`playerSpawn`, `npc`, `boss`, `triggerBox`, `destroyable`)를 저장하고, `Publish-WorldGameplay.ps1`이 kind별 exact-property·참조 검증 후 `.worldbootstrap` version 3을 원자 생성하며, `CWorldBootstrap::Load`가 v3을 파싱해 계약 위반을 전부 실패로 처리한다. 종료 증거: 4개 world 문서 v2 마이그레이션 + `-Mode Validate` 통과 + `Server.exe --contract-test`의 v3 파싱 섹션 PASS + 잘못된 참조/필드 주입 시 publish 실패.

### 수정 파일과 존재 이유

| 파일 | 변경 이유 |
|---|---|
| `Data/Worlds/*/Gameplay.world.json` (4개) | `formatVersion: 2`로 승격. 신규 kind는 필요한 Area(LV_LUT)에만 추가 |
| `Tools/WorldPipeline/Publish-WorldGameplay.ps1` | kind별 exact-property 검증, 참조 검증, bootstrap v3 행 생성 |
| `Client/Public/WorldGameplayDocument.h` / `Client/Private/WorldGameplayDocument.cpp` | `WORLD_PLACEMENT_KIND`에 `TRIGGER_BOX`, `DESTROYABLE` 추가, kind별 필드 struct, v2 read/write, `Is_Valid` kind별 검증 |
| `Server/Public/WorldBootstrap.h` / `Server/Private/WorldBootstrap.cpp` | `WORLD_BOOTSTRAP_KIND` 확장, v3 per-kind 행 파싱, staging 후 일괄 commit 유지 |
| `Server/Private/ServerGameplayContractTests.cpp` | v3 파싱 정상/오류 계약 섹션 추가 |

새 C++ 파일 없음 — `.vcxproj`/`.filters` 변경 없음.

### placement 스키마 v2

공통 7필드는 v1과 동일하다: `placementId`(stable ID `^[A-Za-z0-9_.-]{1,128}$`, 문서 내 유일), `kind`, `archetypeId`, `encounterId`, `position[3]`, `yawDegrees`, `enabled`. kind별로 아래 필드를 **추가로 정확히** 요구한다(그 외 property 존재 시 publish 실패, exact-property 검증은 kind별 집합으로 수행).

| kind | 추가 필드 | 계약 |
|---|---|---|
| `playerSpawn` | 없음 | v1과 동일. `archetypeId`/`encounterId`는 null |
| `npc` | 없음 | v1과 동일. `archetypeId`는 NpcCatalog 참조 필수 |
| `boss` | 없음 | v1과 동일 |
| `triggerBox` | `halfExtents[3]`, `triggerOnce`, `triggerEvents[]` | `archetypeId`/`encounterId`는 반드시 null. `halfExtents` 각 성분 `0 < v <= 1000`. `triggerEvents`는 1개 이상 16개 이하 |
| `destroyable` | `deployPlacementId`, `initialState` | `archetypeId`/`encounterId`는 반드시 null. `deployPlacementId`는 십진 uint64 문자열, 같은 Area의 authoring `.deployplacements`의 `runtimePlacementId`에 존재해야 하고 문서 내 중복 금지. `initialState`는 `INTACT\|FRACTURED\|DESPAWNED` |

`triggerEvents[]` 원소는 정확히 3 property다.

| 필드 | 계약 |
|---|---|
| `type` | `setCondition` \| `setDestroyableState` |
| `targetId` | `setCondition`: conditionId (stable ID, 같은 Area `.navblockers`의 어떤 region `conditionId`와 일치해야 함). `setDestroyableState`: 같은 문서 내 `destroyable` placement의 `placementId` |
| `value` | `setCondition`: bool. `setDestroyableState`: `INTACT\|FRACTURED\|DESPAWNED` 문자열 |

### v2 예시 전문 (`Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`에 추가될 형태)

```json
{
  "schema": "lostark.world-gameplay",
  "formatVersion": 2,
  "areaId": "LV_LUT_HEARTRB_ED",
  "revision": 3,
  "placements": [
    {
      "placementId": "destroyable.valtan.outer.pillar01",
      "kind": "destroyable",
      "archetypeId": null,
      "encounterId": null,
      "position": [163.006504, 23.04, -129.46252],
      "yawDegrees": 0.0,
      "enabled": true,
      "deployPlacementId": "9335938568718910930",
      "initialState": "INTACT"
    },
    {
      "placementId": "trigger.valtan.outer.collapse",
      "kind": "triggerBox",
      "archetypeId": null,
      "encounterId": null,
      "position": [151.25, 23.0, -121.75],
      "yawDegrees": 0.0,
      "enabled": true,
      "halfExtents": [4.0, 2.0, 4.0],
      "triggerOnce": true,
      "triggerEvents": [
        { "type": "setDestroyableState", "targetId": "destroyable.valtan.outer.pillar01", "value": "FRACTURED" },
        { "type": "setCondition", "targetId": "VALTAN_ARENA_DESTROYED", "value": true }
      ]
    }
  ]
}
```

(실제 문서에는 기존 playerSpawn 4개·boss 1개 placement가 그대로 함께 존재한다. 위 destroyable의 `position`은 참조하는 deploy placement transform의 사본이 아니라 **검증용 anchor**이며, 시각 transform 정본은 `.deployplacements`다. publisher는 두 위치 차이가 0.5를 넘으면 경고가 아니라 실패로 처리해 참조 실수를 조기에 잡는다.)

### publisher 검증 추가 (기존 검증에 더해)

1. `formatVersion == 2` 강제. v1 문서는 실패 처리(4개 문서를 같은 커밋에서 일괄 승격).
2. kind별 exact-property 집합 검증. 공통 7 + kind 추가 필드 외 property 발견 시 throw.
3. `triggerBox`: `setCondition`의 `targetId`가 같은 Area `.navblockers` region들의 `conditionId` 집합에 없으면 실패. `.navblockers` 자체가 없는 Area(navigationBlockers 미보유)에서 `setCondition` event가 있으면 실패. `setDestroyableState`의 `targetId`가 같은 문서의 `destroyable` placementId가 아니면 실패.
4. `destroyable`: Area에 deploy pair가 없으면 실패. `deployPlacementId`가 authoring `.deployplacements`에 없거나, 해당 레코드 `destructible == 0`인데 `FRACTURED`/`DESPAWNED`로 전이 가능한 event가 이를 참조하면 실패. anchor 위치와 deploy 레코드 위치의 성분별 차 0.5 초과 시 실패.
5. navigation 정합 검사(Publish-ServerNavigation.ps1:312-329)는 `playerSpawn`/`boss`만 유지 — triggerBox/destroyable은 walkable cell 위에 있을 필요가 없다.

### `.worldbootstrap` version 3 행 형식

헤더는 6필드 유지, version만 3: `LOSTARK_WORLD_BOOTSTRAP \t 3 \t <WorldId> \t <AreaId> \t <revision> \t <행수>`.
행은 kind별 필드 수가 다르며 각 kind에서 **정확히 N필드**를 요구한다(v2의 "비boss 패턴 자리 0 채움" 방식을 버리고 per-kind 명시 계약으로 전환).

| kind | 필드 수 | 레이아웃 |
|---|---|---|
| `playerSpawn` / `npc` | 9 | `placementId kind archetypeId encounterId px py pz yaw enabled` (`playerSpawn`의 archetypeId는 `-`) |
| `boss` | 17 | 공통 9 + `patternId actionId minRange maxRange telegraphMs activeMs recoveryMs damageProfileId` (v2와 동일 의미) |
| `destroyable` | 11 | 공통 9 + `deployPlacementId(uint64) initialState(INTACT\|FRACTURED\|DESPAWNED)` |
| `triggerBox` | 9 + 4 + 3×N | 공통 9 + `hx hy hz once(0\|1) eventCount` … 이후 event마다 `type targetId value` 3필드. `eventCount`는 1..16 |

`CWorldBootstrap::Load`는 kind 파싱 후 kind별 필드 수·값 검증(uint64 파싱, state 문자열, event type/target stable ID, halfExtents finite·양수)을 수행하고 위반 시 전체 실패한다. `WORLD_BOOTSTRAP_PLACEMENT`는 kind별 확장 필드(`iDeployPlacementId`, `eInitialState`, `vHalfExtents`, `isTriggerOnce`, `TriggerEvents(vector)`)를 갖는다. `WORLD_BOOTSTRAP_TRIGGER_EVENT { eType(SET_CONDITION|SET_DESTROYABLE_STATE), strTargetId, strValue }`.

### G01 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
Server\Bin\Debug\Server.exe --contract-test
```

기대: 4 world publish 성공, v3 헤더/행수 일치, contract test에 (a) LV_LUT v3 로드 성공 (b) 손상 주입 케이스(미지 kind, event 수 불일치, uint64 파싱 실패, 미지 state)가 로드 실패로 판정되는 섹션 PASS. 오류 주입은 테스트 전용 임시 파일을 `LOSTARK_SERVER_DATA_ROOT` 우회 경로로 읽게 해 실제 배포물을 오염시키지 않는다.

## G02. `.navblockers` cook과 Server 동적 walkable

### 목표와 종료 증거

`Publish-ServerNavigation.ps1`이 `.navblockers`를 검증·cook해 runtime `.navgrid` version 2(blocker region 포함)를 Client/Server 양쪽에 생성하고, `CServerNavigation`이 region 등록과 `Set_ConditionActive(conditionId, value)`를 지원하며, condition 변경이 이동·경로 탐색에 즉시 반영된다. 종료 증거: publisher 검증 실패 케이스, contract test에서 condition on/off에 따른 `Find_Path`/`Project_Point` 결과 변화, 경로 무효화 정책 검증.

### 수정 파일과 존재 이유

| 파일 | 변경 이유 |
|---|---|
| `Tools/NavigationPipeline/Publish-ServerNavigation.ps1` | `.navblockers` parse/검증(헤더-소스 정합, region id/conditionId/셀 검증), navgrid v2 직렬화 |
| `Engine/Public/NavGrid.h` / `Engine/Private/NavGrid.cpp` | `Load`가 v2(magic+version 헤더) 파싱. region은 데이터로 보관만 하고 자동 등록하지 않음(등록은 기존 `Register_RuntimeBlocker` 호출자 결정) |
| `Server/Public/ServerNavigation.h` / `Server/Private/ServerNavigation.cpp` | v2 로드, region 저장, `Set_ConditionActive`, `Is_Walkable`에 blocker overlay 반영, revision 카운터 |
| `Server/Private/GameRoom.cpp` / `Server/Public/GameRoom.h` | `m_WorldConditions` 소유, condition 변경 시 경로 무효화 정책 실행 |
| `Server/Private/ServerGameplayContractTests.cpp` | 동적 walkable 계약 섹션 |

Engine public header 변경이므로 `UpdateLib.bat` 후 Client 재빌드까지 검증한다. 새 C++ 파일 없음.

### runtime `.navgrid` version 2 형식

현행 v1은 magic 없이 `u32 width, u32 height, f32 cellSize, f32 originX, f32 originZ` + `u8 walkable[N]` + `f32 height[N]`이다. v2는 식별 가능한 헤더를 도입한다.

```text
char[8]  magic     "LANAVG2\0"
u32      width
u32      height
f32      cellSize
f32      originX
f32      originZ
u8       walkable[width*height]
f32      height[width*height]
u32      regionCount                  (0..256)
region × regionCount:
  u16    idLength        + UTF-8 id                (stable ID 검증)
  u16    conditionLength + UTF-8 conditionId
  u8     activateWhenConditionTrue    (0|1)
  u32    cellCount       + u32 cellIndex[cellCount] (row-major, 중복·범위 밖 거부)
```

`Convert-NavigationAuthoringGrid`가 `.navblockers`를 함께 읽어 region을 직렬화한다. `.navblockers`가 없는 Area는 `regionCount 0`. Client `CNavGrid::Load`와 Server `CServerNavigation::Load`는 v1을 더 이상 받지 않는다(같은 커밋에서 publisher 재실행으로 전 Area v2 재생성 — pre-build publish가 강제하므로 stale v1은 빌드 단계에서 사라진다). trailing byte 거부와 walkable/height 전수 검증은 유지한다.

### Server 동적 walkable 계약

- `CServerNavigation`에 추가: `struct SERVER_NAV_REGION { std::string strRegionId; std::string strConditionId; bool activateWhenConditionTrue; std::vector<std::uint32_t> CellIndices; bool isActive; }`, 멤버 `std::vector<SERVER_NAV_REGION> m_Regions`, 셀별 `std::vector<std::uint16_t> m_BlockCounts`, `std::uint64_t m_iRevision`.
- `bool Set_ConditionActive(const std::string& conditionId, bool value)`: conditionId를 구독하는 모든 region의 목표 활성값(`value == activateWhenConditionTrue`)을 계산해 `m_BlockCounts`를 증감하고 revision을 올린다. 매칭 region이 0개면 false를 반환하고 상태를 바꾸지 않는다(미지 condition을 정상 처리하지 않는다).
- `Is_Walkable(cell)`은 `m_Walkable[cell] != 0 && m_BlockCounts[cell] == 0`. `Find_Path`/`Project_Point`/`Resolve_Cell`은 수정 없이 이 판정을 경유하므로 자동 반영된다.
- 로드 직후 초기 조건은 전부 false로 간주해 `activateWhenConditionTrue == false`인 region(Open after destruction)은 활성-차단 상태로 시작한다 — 07-31의 양방향 의미를 그대로 보존.

### 경로 무효화 정책 (GameRoom)

condition 변경 성공 시 GameRoom은 같은 tick 안에서:

1. `hasMoveGoal`인 모든 player의 `MovePath`를 `Find_Path`로 재계산한다. 실패하면 `hasMoveGoal = false`로 목표를 버리고 제자리 정지한다(무한 대기 금지).
2. boss entity의 `fLastPathGoalX/Z`를 NaN sentinel이 아닌 "재계산 강제" 값으로 리셋해 `CValtanBrain`이 다음 tick에 경로를 다시 뽑게 한다.
3. spawn 가용성 검사(`Find_AvailablePlayerSpawn`)는 base walkable 기준을 유지한다 — spawn cell을 blocker로 덮는 authoring은 publisher가 실패시킨다(`playerSpawn` cell이 어떤 region에도 포함되면 Validate 실패).

### G02 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Publish
UpdateLib.bat Debug
Server\Bin\Debug\Server.exe --contract-test
```

기대: LV_LUT navgrid v2에 region 직렬화(현재 regionCount 0 — MapTool로 실제 region 1개를 만들어 저장한 뒤 재검증), contract test에서 `Set_ConditionActive(true)` 후 차단 셀 경유 `Find_Path` 실패·우회 성공, `false` 복원 후 원경로 복구, 미지 conditionId 거부 PASS. Client/Server navgrid byte hash 일치.

## G03. Server trigger·destroyable 런타임과 protocol 8

### 목표와 종료 증거

GameRoom이 bootstrap v3의 `triggerBox`/`destroyable`을 소유 상태로 올리고, 30 Hz tick에서 player 진입 판정 → event 적용(`Set_ConditionActive` / destroyable 상태 전이)을 수행하며, destroyable 상태가 protocol 8 snapshot으로 복제된다. 종료 증거: NetworkProtocolHarness failures 0, contract test의 trigger 발화·1회성·재장전·상태 복제 섹션 PASS.

### 수정 파일과 존재 이유

| 파일 | 변경 이유 |
|---|---|
| `Shared/Public/Network/PacketType.h` | `NETWORK_PROTOCOL_VERSION 7 -> 8` |
| `Shared/Public/Network/PacketMessages.h` / `Shared/Private/Network/PacketMessages.cpp` | `WORLD_ENTITY_KIND`에 `DESTROYABLE`, `S2C_WORLD_ENTITY_SPAWNED`에 `iStateValue(u8)`·`iDeployPlacementId(u64)`, `WORLD_ENTITY_SNAPSHOT`에 `iStateValue(u8)` — writer/reader 대칭 |
| `Tools/NetworkProtocolHarness/...` | 신규 필드 round-trip과 경계값 검증 |
| `Server/Public/ServerWorldEntity.h` | destroyable용 상태 필드(`eDestroyableState`), `iDeployPlacementId` |
| `Server/Public/GameRoom.h` / `Server/Private/GameRoom.cpp` | `SERVER_TRIGGER` 소유, `Update_Triggers`, destroyable entity 생성·상태 전이, snapshot 직렬화 확장 |
| `Server/Private/ServerGameplayContractTests.cpp` | trigger/destroyable 계약 섹션 |

### Server 상태 모델

- `struct SERVER_TRIGGER { std::string strPlacementId; float fCenterX, fCenterY, fCenterZ; float fHalfX, fHalfY, fHalfZ; float fYawDegrees; bool isTriggerOnce; bool hasFired; bool hasPlayerInside; std::vector<WORLD_BOOTSTRAP_TRIGGER_EVENT> Events; bool isEnabled; }` — GameRoom 멤버 `std::vector<SERVER_TRIGGER> m_Triggers`. trigger는 `SERVER_WORLD_ENTITY`가 아니며 NetEntityId를 소비하지 않는다.
- `destroyable`은 `SERVER_WORLD_ENTITY`로 생성한다(`eKind == DESTROYABLE`, HP/패턴 없음, `eDestroyableState = initialState`, `iDeployPlacementId` 보유). NetEntityId는 기존 `m_iNextNetEntityId` 발급 규칙 공유. `Initialize_WorldEntities`에서 boss 프로파일 검증 분기와 분리된 destroyable 분기를 추가하고, navigation `Project_Point` 스냅은 적용하지 않는다(파괴물은 비walkable 위에 있을 수 있다).
- `Update_Triggers(updateTick)` — `Tick()`의 `Update_Players` 뒤, `Update_WorldEntities` 앞에서 실행:
  1. 각 enabled trigger에 대해 생존 player 중심점의 yaw-회전 OBB 내부 판정(2D XZ + Y 반높이)을 수행한다.
  2. `triggerOnce == true`: `hasFired`면 skip, 최초 진입 시 event 적용 후 `hasFired = true`.
  3. `triggerOnce == false`: `hasPlayerInside`가 false→true 전이하는 tick에만 발화(edge), 전원 이탈 시 재장전.
  4. event 적용: `SET_CONDITION` → `CServerNavigation::Set_ConditionActive` + 경로 무효화(G02 정책). 반환 false(미지 condition)는 room 상태 오류로 기록하되 bootstrap v3 검증이 선차단하므로 정상 경로에서 발생하지 않는다. `SET_DESTROYABLE_STATE` → 대상 placementId의 destroyable entity 탐색 후 상태 전이. INTACT→FRACTURED→DESPAWNED 전방 전이만 허용하고 역전이는 거부(상태 오류 기록).
- snapshot: `Broadcast_WorldSnapshot`이 entity마다 `iStateValue`를 채운다 — BOSS/NPC는 0 고정, DESTROYABLE은 `eDestroyableState` 값. `To_NetworkKind`에 `DESTROYABLE` 매핑 추가. spawn 메시지에는 현재 상태와 `iDeployPlacementId`를 실어 late-join Client가 즉시 정합한다.

### G03 검증

```powershell
msbuild Shared/Default/Shared.vcxproj /m /t:Build /p:Configuration=Debug /p:Platform=x64
msbuild Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj /m /t:Build /p:Configuration=Debug /p:Platform=x64
Tools\NetworkProtocolHarness\Bin\Debug\NetworkProtocolHarness.exe
msbuild Server/Default/Server.vcxproj /m /t:Build /p:Configuration=Debug /p:Platform=x64
Server\Bin\Debug\Server.exe --contract-test
```

기대: harness failures 0 (신규 필드 round-trip, `iStateValue > 2` 거부, `DESTROYABLE` kind spawn/snapshot 인코딩), contract test에서 (a) trigger 진입 tick에 destroyable FRACTURED + condition true 동시 적용 (b) once 재진입 무발화 (c) edge trigger 재장전 (d) DESPAWNED 역전이 거부 (e) snapshot의 `iStateValue` 반영 PASS.

## G04. Client replication·presentation — destroyable·NPC

### 목표와 종료 증거

Valtan 제품 Level이 deploy prop을 로드하고, `CClientReplication`이 BOSS 하드코딩을 kind별 분기로 확장해 DESTROYABLE snapshot 상태를 `CDeployPropObject::Set_State`로, NPC spawn을 `CNpc` 생성으로 번역한다. 종료 증거: Server+Client 런타임에서 trigger 발화 시 해당 프롭이 FRACTURED 시각/ANIM "off" one-shot으로 전환되고, NPC 배치가 idle clip으로 보인다.

### 수정 파일과 존재 이유

| 파일 | 변경 이유 |
|---|---|
| `Client/Public/DeployPropRuntime.h` / `Client/Private/DeployPropRuntime.cpp` (**새 파일**) | 제품 소유 deploy 로드 경로. `CDeployPropCatalog` parse→validate→prototype 등록→clone stage→commit(실패 시 전량 rollback)과 `runtimePlacementId → weak_ptr<CDeployPropObject>` 조회를 소유. MapTool의 호출자 없는 `Load_DeployProps`를 대체하는 단일 경로(두 번째 로더 금지 원칙에 따라 MapTool도 G05에서 이 클래스를 사용) |
| `Client/Default/Client.vcxproj` / `.filters` | 위 새 파일 2개 등록 (`ClInclude`/`ClCompile`, 필터 `03. Tools\00. Map`이 아닌 기존 runtime 소속 필터 — `MapPlacementRuntime`과 같은 위치) |
| `Client/Private/Loader.cpp` | `Ready_For_ValtanArena`에서 deploy model prototype 등록(`Prototype_GameObject_DeployProp` GameObject prototype 포함 — 현재 저장소에 등록 코드가 전혀 없음) |
| `Client/Public/DeployPropObject.h` / `Client/Private/DeployPropObject.cpp` | 컴포넌트 clone level index 하드코딩(`ETOUI(LEVEL::DEVELOPMENT)`, Ready_Components) 제거 — desc로 level index 주입 |
| `Client/Public/ClientReplication.h` / `Client/Private/ClientReplication.cpp` | `WORLD_ENTITY_PRESENTATION`의 `weak_ptr<CValtan>` 고정 해제 → kind별 variant(BOSS: `CValtan`, NPC: `CNpc`, DESTROYABLE: `CDeployPropObject` 참조). `Apply_WorldEntitySpawn` kind 분기, `Apply_WorldSnapshot`에서 `iStateValue` → `Set_State` (변화 시에만 호출) |
| `Data/Actors/NpcCatalog.json` | NPC archetype 스키마 확정: `archetypeId`, `clientPresentationId`, `modelAssetId`(Resources 상대), `idleClip`, `runtimeStatus` |
| `Client/Private/ActorCatalog.cpp` / `Client/Public/ActorCatalog.h` | `Find_Npc` 파싱 추가 (BossCatalog `presentationClips` 파싱과 같은 required 검증 스타일) |
| `Client/Private/Level_ValtanArena.cpp` 등 | Level이 `CDeployPropRuntime`를 소유하고 replication DESC로 조회 인터페이스 전달 |

### 흐름

```text
Level_ValtanArena::Initialize
-> CDeployPropRuntime::Load(levelIndex, areaId)   (deploy pair 없으면 no-op 성공)
-> CClientReplication DESC에 deploy 조회 인터페이스 전달

S2C_WORLD_ENTITY_SPAWNED(kind==DESTROYABLE)
-> iDeployPlacementId로 CDeployPropRuntime 조회 실패 시 spawn 거부(실패 이유 보존)
-> 성공 시 presentation 등록 + iStateValue로 초기 상태 적용

S2C_WORLD_SNAPSHOT entity(iStateValue 변화)
-> CDeployPropObject::Set_State(FRACTURED)  → STATIC은 모델 스왑, ANIM은 "off" non-loop 재생
-> DESPAWNED → 렌더 제외(기존 Late_Update 경로)

S2C_WORLD_ENTITY_SPAWNED(kind==NPC)
-> CActorCatalog::Find_Npc(archetypeId) → clientPresentationId 검증
-> 최초 spawn 시 model prototype 1회 admission(CPlayableCharacterAssetService와 같은 first-spawn 경계, 두 번째 로더 금지)
-> CNpc clone + pIdleClip 적용
```

NPC 슬라이스의 전제: 실제 town NPC `.wmodel` 1종을 리소스 팩 규칙에 따라 준비해야 런타임 검증이 닫힌다. 에셋 선정·추출은 이 계획의 입력이며, 준비 전까지 NpcCatalog는 비어 있고(현행 유지) 계약 코드만으로 완료 처리하지 않는다.

### G04 검증

```powershell
msbuild Client/Default/Client.vcxproj /m /t:Build /p:Configuration=Debug /p:Platform=x64
```

runtime smoke (Framework.slnLaunch `Server + Client`):

```text
Lobby -> Valtan 진입
deploy prop 85건 INTACT 표시 (파괴 대상 pillar 포함)
트리거 박스 영역 진입 -> 해당 프롭 FRACTURED 전환 + Server 경로가 붕괴 영역 우회
두 번째 Client 진입(late join) -> FRACTURED 상태로 즉시 정합
NPC placement 추가된 Area에서 idle clip 재생
disconnect -> Lobby 복귀 시 replicated/deploy 상태 정리
```

## G05. MapTool 확장 — trigger·destroyable authoring

### 목표와 종료 증거

World Gameplay 탭에서 `Trigger Box`/`Destroyable` kind를 배치·편집·저장하고, trigger box를 3D wire box로 표시하며, event 목록을 UI로 편집한다. destroyable은 로드된 deploy placement 목록에서 `deployPlacementId`를 선택해 바인딩한다. 종료 증거: v2 문서 저장→재로드 왕복 무손실, publish Validate 통과, workspace 테스트 명령으로 로컬 preview 상태 전환.

### 수정 파일과 존재 이유

| 파일 | 변경 이유 |
|---|---|
| `Client/Public/MapTool.h` / `Client/Private/MapTool.cpp` | kind 라디오 확장, trigger 편집 상태(입력 버퍼·선택 event 인덱스), box overlay 렌더, destroyable 바인딩 콤보, workspace 테스트 명령 |
| `Client/Public/WorldGameplayDocument.h` / `.cpp` | (G01에서 이미 v2 대응) MapTool이 사용하는 kind별 편집 API |
| `Client/Private/MapTool.cpp`의 deploy 경로 | 호출자 없는 `Load_DeployProps` 제거 → `CDeployPropRuntime` 사용으로 교체. Valtan Area 전환 시 deploy pair를 **읽기 전용**으로 stage(전환 transaction의 기존 단계에 추가, 실패 시 rollback 규칙 동일). transform 편집 UI는 추가하지 않음 |

새 C++ 파일 없음(G04의 `CDeployPropRuntime` 재사용).

### UI 계약

- kind 라디오: `Player Spawn / NPC / Boss / Trigger Box / Destroyable` 5종. Trigger Box 선택 시 추가 입력 — `HalfExtents` DragFloat3(0.1~1000 clamp), `Trigger Once` checkbox, event 테이블(행: type 콤보 + target 콤보 + value). target 콤보의 소스는 type별로: `setCondition` → `m_RuntimeBlockerDocument` region들의 conditionId 집합(중복 제거), `setDestroyableState` → 현재 문서의 destroyable placementId 목록. 자유 문자열 입력은 두지 않아 publish 실패를 authoring 단계에서 차단한다.
- Destroyable 선택 시: deploy placement 콤보(`runtimePlacementId` + assetId 라벨), `Initial State` 콤보. 배치 클릭 시 position은 선택한 deploy 레코드 위치로 자동 채움(anchor 정합 검증 통과 보장).
- overlay: `Render_NavigationBoundsOverlay`와 같은 `PrimitiveBatch` wire box로 모든 enabled triggerBox를 표시(선택된 것은 강조색). destroyable placement는 참조 프롭 위치에 marker.
- 테스트: `Test` CollapsingHeader에 `Fire Trigger`(선택 trigger의 event를 workspace 로컬 preview에 적용 — deploy prop `Set_State` + 기존 `Set_NavigationCondition` preview 경로). editor shell이 runtime blocker를 등록하지 않는 08-04 경계는 유지하며, preview는 시각 확인 전용임을 status 문자열로 명시.
- dirty/save: 기존 `m_bWorldGameplayDirty` + `Save_WorldGameplay` 경로 그대로. Area 전환 dirty gate(`Has_UnsavedAuthoring`)에 자동 포함된다.

### G05 검증

```text
Debug Client 실행 -> Lobby Test -> F1 -> Map Tool -> Valtan
World Gameplay 탭: trigger 배치, halfExtents/event 편집, 저장
재로드(Area 재전환) 후 왕복 무손실 확인
Destroyable 배치: deploy 콤보 바인딩, 저장, Publish-WorldGameplay Validate 통과
Fire Trigger -> 프롭 FRACTURED preview + Destruction Area 셀 상태 preview
잘못된 편집(빈 event, 미존재 target)이 UI 단계에서 차단되는지 확인
```

## G06. 하네스·감사·문서·전체 검증

### 수정 파일

| 파일 | 변경 이유 |
|---|---|
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | world 문서 formatVersion 2 검사, navgrid v2 magic 검사, `NETWORK_PROTOCOL_VERSION` ↔ harness 버전 문자열 일치 검사 |
| `AGENTS.md` / `CLAUDE.md` | "World Gameplay kind는 playerSpawn/npc/boss뿐", "trigger 계약 없음" 문장을 구현 상태로 갱신 |
| `.md/TEAM/AREA_DATA_LAYER_GUIDE.md` / `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | kind 표, trigger/destroyable 데이터 정본, Server 권위 경계 갱신 |
| 본 계획 `_RESULT.md` | 구현/자동 검증/수동 검증/미완 분리 기록 |

### 전체 검증 순서

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -DeepAssetHash
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -DeepAssetHash
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1 -DeepAssetHash
git diff --check
```

foreground smoke: G04 시나리오 + Bern/Training/Character Select 무영향 회귀(신규 kind가 없는 world의 기존 흐름 불변), 잔류 process/7777 listener 없음.

### 비평 에이전트 검토 요청 항목

```text
kind 확장이 세 층(publisher/Server/Client) 어디에서든 silent fallback으로 새 kind를 삼키지 않는가
trigger event의 대상 검증이 publish와 bootstrap 로드 양쪽에서 이중으로 닫히는가
destroyable 상태의 진실이 Server 한 곳인가 (Client Set_State 직접 호출 경로 잔존 여부)
condition 변경 시 이동 중 player/boss 경로 무효화가 실제로 검증되는가
deploy 로드가 CDeployPropRuntime 단일 경로인가 (MapTool 구경로 잔존 여부)
navgrid v1 잔존 파일이 로드되는 경로가 없는가
protocol 8 필드가 harness에서 경계값까지 검증되는가
NPC 슬라이스가 에셋 없이 완료 처리되지 않는가
```

지적은 실제 코드/데이터로 재현한 뒤 반영하고, 재현되지 않은 의견은 RESULT에 복사하지 않는다.

## 구현 순서와 의존성

```text
G01 (데이터 정본)  ← 선행 없음
G02 (nav cook·Server 동적 walkable)  ← G01과 독립, 병행 가능
G03 (Server 런타임·protocol 8)  ← G01, G02 필요
G04 (Client replication·presentation)  ← G03 필요
G05 (MapTool authoring)  ← G01 필요 (G03/G04 전이라도 저장·Validate까지 검증 가능)
G06 (하네스·문서·전체 검증)  ← 전체 완료 후
```

각 G는 독립 커밋 단위이며, G 착수 시 이 문서를 기준으로 전체 코드 G-PLAN을 `.md/GB/<MM-DD>/`에 분리 작성한다.
