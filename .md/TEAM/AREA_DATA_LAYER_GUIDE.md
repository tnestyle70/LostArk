# LostArk Area 데이터 레이어 가이드

## 1. 결론

Area별 visual, gameplay placement, navigation은 분리 저장된다. 어떤 entity placement가 없으면 해당 entity는 생성되지 않는다. 그러나 모든 레이어가 완전한 plug-in 구조인 것은 아니다. 일반 몬스터와 wave spawn은 Valtan Arena와 Character Select Arena의 명시된 SpawnGroups만 제품 계약이 있고, Area별 balance override는 아직 없다.

편집·추출 정본은 전부 repository root의 `Data` 아래에 둔다. `Data/Maps/Imported`는
재추출한 catalog와 shard 기준, `Data/Maps/Authoring`은 현재 visual placement,
`Data/Worlds`는 gameplay placement, `Data/Navigation`은 bake/paint/blocker를 소유한다.
`Client/Bin/DataFiles`와 `Server/Bin/DataFiles`는 publisher 생성물이므로 직접 수정하지 않는다.

```text
LevelCatalog scenario
  -> MapCatalog area
     -> visual asset admission / placement
     -> optional deploy asset / placement pair
     -> optional point-light presentation source / runtime pair
     -> Gameplay.world.json formatVersion 6 (actor/NPC behavior + gated trigger/destroyable authoring)
     -> navigation authoring -> runtime navgrid
     -> stable actor / encounter / balance ID 참조
```

## 2. 현재 Area 실측

| Area | Visual | Gameplay | Navigation | 추가 데이터 |
|---|---|---|---|---|
| `LV_BER_BERNCASTLE` | shard-set, 50,017 placements | 16 placements: class-neutral player spawn 4 + NPC 10 + triggerBox 1 + collisionBox 1 | 50×347 source/paint, Server navgrid + 1m deck-step policy | NPC behavior/trigger/collision authoring, boss 없음 |
| `LV_LUT_HEARTRB_ED` | 275 assets / 13,186 placements | player spawn 4 + `BOSS_VALTAN` 1 | 62×63, `Data/Navigation/LV_LUT_HEARTRB_ED.*` | deploy pair, source-exact outer towers, map point light 22, BossProfile, ValtanEncounter |
| `LV_DEV_TRAINING_GROUND` | RCArena 10 assets / 18 placements | class-neutral player spawn 4 | uniform 32×32 | NPC/boss/monster/trigger 없음 |
| `LV_LOBBY_CLASSSELECT_SL00` | 55 assets / 803 placements | class-neutral player spawn 4 | Server uniform 42×60 + MapTool source/paint bootstrap | Character Select Arena gameplay + monster/Lugaru SpawnGroups |
| `LV_SHS_RCARENA_D` | 302 assets / 7,856 placements | 없음 | 없음 | 원본 Training Map 편집 대상 |

수련장은 Lobby의 `Enter Training`에서 Server 승인을 받은 뒤 `LEVEL::DEVELOPMENT`로 진입한다. Debug/Release network smoke는 map load, player spawn, Q command, Server action 승인, cooldown HUD 반영까지 검사한다.

## 3. 레이어별 생략 규칙

| 레이어 | 없을 때 | 불완전할 때 |
|---|---|---|
| visual map | scenario가 Map domain을 요구하면 load 실패 | catalog/placement 참조 오류는 rollback |
| deploy | `.deployassets`와 `.deployplacements`가 모두 없으면 skip | 둘 중 하나만 있으면 오류 |
| point-light presentation | `sourceLights`와 `lights`가 모두 없으면 skip | 둘 중 하나만 있거나 문서 검증이 실패하면 publish/load 실패 |
| NPC/boss placement | 해당 kind 행이 없으면 spawn하지 않음 | unknown archetype/encounter는 publish 실패 |
| navigation | navigationRuntime을 선언하지 않은 world만 생략 가능 | Bern/Valtan/Training/Character Select Arena는 grid/policy 누락·손상 시 room 기동 실패 |
| balance definition | 사용하지 않는 actor/skill 정의는 runtime state를 만들지 않음 | placement/action이 없는 stable ID를 참조하면 publish 또는 Server load 실패 |

`Gameplay.world.json` 자체는 Server가 여는 world마다 필요하다. 접속 가능한 world는 최소 하나의 활성 `playerSpawn`이 필요하므로, 빈 placements 문서를 제품 world의 정상값으로 취급하지 않는다.

## 4. MapTool이 지금 편집하는 것

Debug Lobby에서 `Test`를 누르면 기존 `TRAINING_GROUND` 서버 승인을 거친 뒤
`LEVEL::DEVELOPMENT`를 socket, player, replication이 없는 Map Editor Workspace shell로
연다. 진입 후 F1은 공통 Developer Tools만 토글하며, 그 안의 `Map Tool`에서 다음 네 Area
중 하나를 선택한다. F1이나 Map Tool 버튼 자체는 Level을 전환하지 않는다.

| 선택 | visual source | navigation | gameplay |
|---|---|---|---|
| Character Select | `LV_LOBBY_CLASSSELECT_SL00` | source/paint, Nav Bounds bootstrap 허용 | exact `gameplayDocument` 필수 |
| Bern | `LV_BER_BERNCASTLE` | source/paint Nav Bounds bootstrap 허용 | exact `gameplayDocument` 필수 |
| Valtan | `LV_LUT_HEARTRB_ED` | source/paint/blockers 필수 | exact `gameplayDocument` 필수 |
| Training Map | `LV_SHS_RCARENA_D` | disabled | disabled |

Training Map은 원본 302 assets / 7,856 placements인 `LV_SHS_RCARENA_D`다. Release 제품
Test가 사용하는 10 assets / 18 placements `LV_DEV_TRAINING_GROUND`와 다른 데이터다.

Area selector는 `Data/Maps/MapCatalog.json`의 exact `sourceCatalog`와
`sourcePlacements`를 읽는다. `Client/Bin/DataFiles/Map` fallback은 없다. Area 전환 전에
visual/gameplay/navigation dirty를 검사하고 `Save and Continue / Discard and Continue /
Cancel` 중 하나를 요구한다. 전환 stage가 실패하면 기존 Area 객체와 문서를 유지한다.

MapTool의 저장 대상은 Data 원본뿐이다.

- visual: active descriptor의 `Data/Maps/Authoring/...mapplacements`
- point-light presentation: catalog가 선언한 `Data/Maps/Authoring/...maplights.json`
- gameplay: Character Select/Bern/Valtan의 exact `Data/Worlds/.../Gameplay.world.json`
- navigation: 정책이 허용한 `Data/Navigation/*.navsource/.navpaint/.navblockers`

Bern은 `Place Nav Bounds`로 실제 렌더 바닥을 고른 뒤 Bottom Y와 Height from Bottom으로 세로 범위를
제한해 bake한다. 제품 runtime은 이미 활성화되어 있으므로 source/paint/policy가 누락되거나 손상되면
Server room admission이 실패한다. publisher는 실제 bake 결과, player spawn/trigger 연결성, cell
통계와 Area별 step policy를 함께 검증한다.

Navigation `Walkability` 브러시는 높이가 해석된 셀에 대해 `Block`, `Force Walkable`,
`Reset` 세 명령을 제공한다. `Block`과 `Force Walkable`은 bake 결과보다 우선하는 수동
override이며 `Reset`은 해당 셀의 walkability와 명시적 높이를 bake 결과 상속으로 되돌린다.
`.navpaint` version 3은 `x z BLOCKED|WALKABLE [height]` 또는 `x z HEIGHT height`를 저장한다.
명시적 높이는 bake가 surface를 해석한 셀에만 허용된다. 기존 version 2의
`x z BLOCKED|WALKABLE`과 version 1의 `x z`(`BLOCKED`)도 호환 로드한다. 높이가 없는
`NO_SURFACE` 셀은 강제로 이동 가능하게 만들 수 없다.

MapTool은 Client `.navgrid`/`.navpolicy`를 export하거나 제품 Navigation runtime blocker를 등록하지 않는다.
Visual runtime은 `Publish-MapAuthoring.ps1`, world bootstrap은
`Publish-WorldGameplay.ps1`, Server navigation은 `Publish-ServerNavigation.ps1`만 교체한다.
`ACTIVE.maparea`도 selector 변경 때 자동 저장하지 않는다.

MapCatalog의 `sourceLights`와 `lights`는 선택적인 한 쌍이다. source는
`Data/Maps/Authoring/<AreaId>/<AreaId>.maplights.json`, runtime은
`Client/Bin/DataFiles/Map/<AreaId>.maplights.json`만 허용한다. schema
`lostark.map-light-presentation` formatVersion 1은 stable `lightId`, source level/object ID,
world position, radius, falloff, RGBA와 brightness를 소유한다. publisher는 이 문서를 visual placement와
optional deploy pair와 같은 파일 집합 트랜잭션으로 교체하며 중간 실패 시 전부 rollback한다. MapTool은
source 문서를, 제품 Level은 runtime 문서를 읽고 둘 다 기존 `CPresentation_Manager`의 transient point-light
경로에 제출한다. 이 레이어는 Client 시각 표현 전용이며 Server gameplay 판정이나 광원 충돌을 만들지 않는다.
Valtan은 catalog가 이 pair를 선언하므로 누락·손상을 정상적인 생략으로 취급하지 않고 Area stage를 실패시킨다.

Valtan DeployProp은 Development MapTool에서 source catalog 12 asset / 151 placement를
`CDeployPropRuntime` 한 경로로 stage한다. Deploy asset catalog는 format version 2이며 각 asset의
`emissiveIntensity`와 `deferredEmissiveOverlay(0|1)`를 저장한다. placement 문서는 format version 1을
유지하므로 기존 stable placement ID와 Transform 계약은 바뀌지 않는다. 현재
`VALTAN_FLOOR_BRICK_A/B`만 각각 `0.35/1`을 사용하고 나머지 Deploy asset은 `1/0`이다.
Area 전환은 catalog가 요구하는 모든 Map/Deploy runtime
asset이 실제 `Client/Bin/Resources`에 있을 때만 commit하며, 하나라도 없으면 이전 Area를 보존하고
누락된 Resources-relative asset ID를 workspace status에 표시한다.

`VALTAN_FLOOR_BRICK_A/B`의 material index 1 균열 마스크는 일반 불투명 draw 뒤,
`MRT_GameObject`가 끝나기 직전 `DEFERRED_OVERLAY`에서 pass 15로 Target Emissive에만 기록한다.
이 전용 순서는 뒤의 Map draw가 발광 target을 0으로 덮는 문제를 막고, base pass의 같은 발광은
억제해 중복 합성을 피한다. depth는 read-only이고 작은 음수 bias만 사용하므로 바닥 Transform,
collision, navigation, Server 상태는 바꾸지 않는다. A/B가 붕괴 상태에서 `DESPAWNED`되면 같은
`CDeployPropObject`가 overlay queue에도 들어가지 않아 발광도 함께 사라진다. 사용 texture는
`bg_rad_valtan_crack_floor01_em_reconstruction.png`이며 원본 MIC에는 authored emissive가 없어서
`VIDEO_MATCH_RECONSTRUCTION`으로 관리한다.

Bern Area 전환은 모델 admission을 프레임 예산으로 나눠 수행한다. Area를 고르면 workspace status가
`Preparing Bern: N / 1003 model prototypes` 진행률을 표시하고 그동안 Area 콤보와 world 편집
입력은 잠긴다. 1003개 prototype 1.8 GB를 한 프레임에 올리던 정지 구간은 사라졌지만, admission이 끝난 뒤 실행되는
50,017 placement의 parse/stage/commit 트랜잭션은 여전히 한 프레임이므로 마지막에 짧은 멈춤이 남는다. 진행률이
끝나고 status가 commit될 때까지 기다린다.

Valtan 외곽의 기존 다섯 철탑은 새 조립물이 아니다. 13,186개 source baseline에 보존된 SL04 exact
반복 구조이며, 각 철탑 core는 같은 9개 asset의 106개 placement로 이루어진다. 다섯 station을 rigid
registration한 RMS는 약 0.000025~0.000058m이고 cooked core 외형은 약 10.09×44.97×8.16m다. 따라서
`MAP_0AEF815A33D8_BG_LUT_LUTOMB_STRUCTURE06_SM_OLD` 한 종류를 돌려 배치한 136개
`PROJECT_AUTHORED_RIM` overlay는 원본 철탑 복원이 아니며 제거된 상태를 유지한다.

후방 네 station의 상부 component는 같은 SL04 조립체의 하부·체인과 원본 transform에서 맞물린다.
SL00 floor와 SL04 floor의 높이 차이를 상부에만 적용하면 하부는 제자리에 남고 상부 47개씩이
`10.6108742m` 떠서 조립체가 분리된다. 따라서 정본
`Tools/LevelPlacementExtractor/heartrb_valtan_tower_phase_registration.json`은 후방 네 station의
component를 47개씩, 총 188개 stable source ID로 고정하되 registration을 비활성화하고 전방 우측
`pointlight_11` station을 불변 control로 소유한다.

`sync_valtan_tower_phase_registration.py`는 이 계약을 다음 네 authoring 입력에 한 transaction으로
동기화한다.

- 원본 SL04 source 188개는 provenance와 원본 transform을 보존하고 `visible=1`로 둔다.
- 잘못된 `VALTAN_TOWER_REGISTERED` overlay 188개와 해당 환경 hidden override 188개를 제거한다.
- `heartrb_valtan_core_overlay.json`에는 phase proxy 6개만, `heartrb_environment_runtime.json`에는
  원래의 visibility override 2개만 유지한다.
- 후방 light 4개도 source Y `24.734033m`로 복원하고 control station light와 나머지 light는 유지한다.

따라서 HeartRB의 현재 제품 정본은 source baseline `13,186` placements다. base scene을
재생성하거나 tower attachment manifest를 바꾼 뒤에는 source attachment sync를 먼저 실행하고,
그 다음 `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED`로 runtime 문서를 교체한다.
별도 조사 문서
`LV_LUT_HEARTRB_ED_LANDSCAPE.mapplacements`의 LAND01 component 6개는 메인 아레나 좌표계에
직접 병합하면 검은색·갈색 직사각형 판으로 입구 바닥을 덮으므로 제품 문서에 넣지 않는다.
`MapCatalog.json`의 수치가 다른 과거 결과서와 충돌하면 메인 authoring/runtime의 동일 hash와
`Publish-MapAuthoring.ps1` 검증 결과를 우선한다.

철탑 주변의 SL04 PointLight는 총 22개다. 높이 기준 상단 5개와
중·하단 17개이며, 공통 color는 RGB `(255,37,0)`, falloff는 source class 기본값 `2`다. radius는
`9m`, `10.24m`, `20.48m`, brightness는 `2.5`, `3`, `6`의 source 값을 행별로 보존한다.
색·반경·밝기와 위치는 source instance 값이다. 후방 tower light
`pointlight_106/21/104/102`와 `pointlight_11`은 모두 source Y `24.734033m`를 유지한다.
따라서 maplight 문서 provenance는 `SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED`다. falloff `2`는 source 행에 없으며
current-revision `Engine.Default__PointLightComponent`의 상속 기본값을 사용한 inference다.
22개를 특정 철탑 station에 묶는 것은 반복 geometry와 world position 정렬을 근거로 한 inference이고,
source가 parent/slot 연결을 직접 제공한 것은 아니다.

PointLight는 주변 조명만 만든다. 원작 화면의 visible fire/sprite 후보
`BFX_LOW_02.fire.par_c_fire_r_001`과 밝게 보이는 slot surface/material은 현재
`Client/Bin/Resources` 및 runtime presentation에서 복구되지 않았다. 따라서 붉은 주변광 복구를
발광 불꽃이나 slot geometry 복구와 동일시하지 않으며, 해당 표현이 실제 추출·cook·runtime smoke를
통과하기 전에는 원작 철탑 연출 완성으로 판정하지 않는다.

`NpcCatalog.json`은 `runtimeStatus=supported` archetype을 모두 지원하며 현재 75종이다.
Area의 enabled NPC placement는 Server world entity로 생성되고 `CClientReplication`이
catalog → on-demand model prototype/animation set → `CNpc` 경로로 표현한다.
catalog에 없거나 `runtimeStatus`가 `supported`가 아닌 archetype은 publisher가 거부한다.
NPC placement는 Gameplay formatVersion 6의 optional `behavior`로 stationary/patrol/wander,
waypoint, timing과 semantic action을 소유한다. publisher는 이를 Server `worldbootstrap` v7의
이동·행동 의미와 Client `npcpresentation` v2의 실제 clip binding으로 분리한다.

현재 Bern authoring은 전체 placement 16개 중 NPC 10개이며, 기존 placement/revision/transform을
보존한 초기 v6 이관에서는 모두 `behavior: null`이다. 이는 기존 정적 idle과 같은 의미이며 특정
Aylara/Beda 행동 샘플이 미리 들어 있다는 뜻이 아니다. 행동 변경은 Map Tool에서
`Apply NPC Behavior -> Save Gameplay -> Publish-WorldGameplay -> Server restart` 순서로 반영한다.
배치·쿠킹·상호작용 작업 절차는 `NPC_OWNER_HANDOFF.md`가 정본이다.

Valtan 일반 몬스터와 spawn wave는 `Data/Worlds/LV_LUT_HEARTRB_ED/SpawnGroups.world.json`이 정본이다. Character Select의 즉시 audition은 `Data/Worlds/LV_LOBBY_CLASSSELECT_SL00/SpawnGroups.world.json`에 일반 몬스터와 `MINIBOSS_LUGARU`를 각각 zero-delay 단일 wave로 저장한다. Character Select Debug ImGui는 이 stable group ID 또는 disabled Valtan placement ID만 Server에 제출하며 transform/archetype을 보내거나 Client object를 직접 만들지 않는다. `triggerBox`는
world formatVersion 6의 strict parse/save 구조와 Debug Development MapTool 배치·선택·크기·목적지
편집·저장/재로드 UI를 제공한다. typed action이 없는 draft는 `enabled: false`, `events: []`로만 저장한다.
제품 publisher/runtime가 admission하는 action은 정확히 하나의 `movePlayer`, `changeLevel`, `activateSpawnGroup`, `activateEncounter`다. movePlayer는
`targetPosition[3]`, `durationSeconds(0.05..10)`, `arcHeight(0..1000)`를 소유하며 Server가 yaw가 적용된
OBB 진입과 포물선 이동을 판정하고 player snapshot으로 복제한다. `triggerOnce=true`는 room 전체에서
첫 성공 진입 한 번, `false`는 player가 완전히 나간 뒤 다시 들어오는 edge마다 재실행한다. 저작용 wire
box는 판정 권위가 아니다. `activateSpawnGroup`은 Area spawn group stable ID만, `activateEncounter`는 disabled boss placement ID만 참조한다. `destroyable`, `setCondition`, `setDestroyableState`, 파티 대기, 컷신은 계속 publisher가 fail-closed로 거부한다. 수업용 `CMonster`를 재사용하거나 `npc`/`boss`로 위장하지 않는다.

Valtan MapTool의 파괴 물리 audition은 제품 publisher 입력과 분리된 Debug authoring 계약이다.
`Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.destructionsimulation.json`이
파괴 그룹별 debris element의 spawn offset, world direction, speed, gravity scale, lifetime,
`IMMEDIATE`/`TIMELINE_TIME`/`COLLISION_IMPACT` 조건과 suppress-only alias placement ID를 저장한다.
`Destruction Model View`는 실제
Development world의 기존 `CDeployPropRuntime -> CDeployPropObject -> CModel` 인스턴스에 PhysX actor를
연결한다. Profile 아래 source placement를 Wall Mesh Emitter로, 각 Emitter 아래 runtime-generated
`fragment.00`~`fragment.11`을 표시하고 All Fragments/Solo Emitter/Solo Fragment,
play/pause/restart, 1/60 step과 reset 후 고정-step seek를 제공한다. fragment는 runtime sample이며
format v2 JSON은 source emitter와 `suppressionAliasPlacementIds`의 합집합이
WorldEvents group member와 정확히 일치해야 한다. alias는 source와 함께 숨길 뿐 debris actor를 추가
생성하지 않는다. format v1은 자동 추측 이관 없이 fail-closed하므로 MapTool에서 v2로 다시 authoring해야 한다.
이 문서는 `Publish-WorldGameplay.ps1`의 입력이 아니므로 tool audition 자체는 위 destroyable admission에
거부되지 않는다. 반대로 이 preview가 Server 파괴 상태, 동적 collision/navigation, Shared replication이나
제품 Valtan presentation을 활성화했다는 뜻은 아니며, 그 제품 gate는 계속 fail-closed다.

`DEPLOY_ITR_02306`의 fractured WModel은 작은 벽돌 submesh를 숨겨 둔 자산이 아니라 기둥 전체를
표현하는 static CModel이다. MapTool의 작은 파편 audition은 정확한 source particle
`FX_ITR_02315.Par_G_Fracture_Dust_02_01`이 복구되기 전까지 다음 Resources-relative Valtan stone
WModel을 명시적인 `PROJECT_AUTHORED` proxy로 사용한다.

```text
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel
```

source placement 하나에서 proxy stone 12개만 각각 PhysX actor로 날아간다. activation부터 fragment
lifetime 만료 뒤까지 source와 suppress-only alias는 숨김을 유지하고 Reset/Clear에서 이전 상태로
복원한다. fragment ID는 `<elementId>.fragment.00`~`.11`이고 model/state/life/pose/velocity를
read-only로 확인할 수 있다. prototype의 0.01 pretransform 뒤 preview scale 3.5와 deterministic
0.8~1.2 piece scale을 적용한다. generic proxy admission은 원자적이며 누락·손상 시 파편 preview만
unavailable 상태로 격리한다.

`DEPLOY_ITR_02316`에는 source fractured mesh의 17,731 triangle/material을 정확히 한 번 보존해 4x3으로
분할한 12개 `PROJECT_AUTHORED` exact-geometry macro-shard lane이 추가되어 있다. 원본 chunk/physics graph를
복구한 것은 아니다. 저작 receipt는
`Data/Maps/Authoring/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316.debrisrecipe.json`, Git 제외 runtime 입력은
`Client/Bin/Resources/Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_00.wmodel`
부터 `_11.wmodel`까지다. 12개 exact prototype은 optional atomic batch이며 하나라도 준비되지 않으면
부분 exact 등록 대신 위 generic stone 12-instance 표현으로 fallback한다.

이 group은 인접한 `DEPLOY_ITR_02316` 벽 두 칸을 함께 날린다. 각 벽은 자기 source emitter 1개와
`suppressionAliasPlacementIds` 1개로 나뉘며, 네 placement ID가 모두 같은 WorldEvents group member다.

```text
벽 1  source 17150846598057876717  alias 10426387515393336411  (156.480996, 23.04, -127.7017)
벽 2  source 9863801195242004116   alias 12598937882346321836  (155.489756, 23.04, -127.5063)
```

각 쌍의 alias는 source와 Z로 0.000137m만 떨어진 co-located sibling이라 **절대 두 번째 emitter로
만들지 않는다.** 그러면 같은 벽 하나에서 24조각이 나온다. 반면 1.01m 떨어진 별개의 벽은 자기
emitter를 갖는 것이 정상이며, 그래서 이 profile의 All Fragments는 `2 * 12 = 24`개다. exact recipe는
placement가 아니라 `sourceDeployAssetId`로 조회되므로 두 벽이 같은 12-piece recipe를 재사용하고
새 cook이나 C++ 등록은 필요하지 않다.
optional proxy 누락 때문에 Valtan Area, DeployProp 편집, World Destruction 문서 편집 전체를 막아서는
안 된다. Map 담당자에게는 위 네 asset ID와 물리 `Client/Bin/Resources/Effect/Valtan/Meshes/FX_SM_00`
폴더, 그리고 02316 CHUNK 12개가 있는 `Client/Bin/Resources/Deploy/.../fractured` 물리 폴더를 함께
인계한다.

맵 담당자의 실행 순서, 세 trigger 층의 차이, Effect cue와 제품 Server를 붙이는 호출 지점은
`MAP_DESTRUCTION_PHYSX_HANDOFF.md`가 정본이다. `Stage Selected (Paused)`는 actor tree를 stage할 뿐
재생하지 않으므로 가시성 검증은 `Play All Fragments` 또는 fragment 행의 `Solo + Play`를 사용한다.

### Valtan MapTool Area 진입 실패 점검

`git pull`은 Git 제외 대상인 `Client/Bin/Resources`를 복구하지 않는다. Valtan Area catalog는
275 map asset / 13,186 map placement와 위 Deploy 9/85를 strict validation하므로, 팀 runtime pack의
`Map/LV_LUT_HEARTRB_ED` 파일을 물리 Resources 폴더에 준비해야 한다. 서로 다른 worktree의 EXE를
실행할 때는 그 EXE가 읽는 `Data`, `ShaderFiles`, `Resources` 루트가 달라질 수 있으므로 실행 경로를
먼저 확인하고, 공유 Resources를 쓸 때는 process-local `LOSTARK_RESOURCE_ROOT`로 명시한다.

파괴 audition은 `ValtanWorldEvents.json`의 non-empty group과
`LV_LUT_HEARTRB_ED.destructionsimulation.json`의 `profile.groupId`가 일치해야 한다. 실제
`collisionBox`가 아직 저작되지 않은 preview binding은 `STAGE_TIME`과 빈 `receiverCollisionId`를
사용한다. 존재하지 않는 collision ID를 `COLLISION_IMPACT`로 참조하거나 빈 WorldEvents 파일을
병합하면 Area 전환이 fail-closed된다. 실제 receiver collisionBox를 저장한 뒤에만
`COLLISION_IMPACT`로 바꾼다.

Valtan `SpawnGroups.world.json`은 anchor transform, prerequisite group, maxAlive, wave 순서, archetype/count/delay를 소유한다. MapTool은 gameplay와 spawn group을 별도 dirty/save로 관리하고 publisher는 두 문서를 한 transaction으로 `VALTAN_ARENA.worldbootstrap`과 `VALTAN_ARENA.spawngroupsbootstrap`에 publish한다. Server가 wave·AI·damage·despawn을 확정하고 Client는 Shared spawn/snapshot/despawn만 표현한다.

`collisionBox`는 transform, half extents, enabled만 저장한다. MapTool의 파란 wire box는 authoring
표시이고 Server가 player OBB 크기를 포함한 swept OBB 판정으로 일반 보행을 차단한다. Client
`CCharacter`의 같은 크기 `CCollider::OBB`는 presentation/debug이며 이동 정답을 만들지 않는다.

`changeLevel`은 Bern과 Valtan Arena 사이에서만 허용한다. Server가 OBB enter edge에서 source room leave와
target room enter를 처리하고 Client는 새 `S2C_ENTER_ACCEPTED`를 소비한 뒤에만 level transition을 요청한다.

## 5. Balance 소유권

Balance는 현재 Area별 파일이 아니라 전역 stable definition이다.

| 정본 | 소유 값 |
|---|---|
| `Data/Balance/PlayerProfiles.json` | class HP/resource/move speed |
| `Data/Balance/PlayerSkills.json` | skill slot/timing/cost/range/damage ID |
| `Data/Balance/DamageProfiles.json` | Server damage |
| `Data/Balance/BossProfiles.json` | boss HP/range/speed/phase threshold |
| `Data/Encounters/<Boss>/...json` | stage/encounter state와 pattern timeline |

Area는 수치를 복사하지 않고 stable actor/encounter ID를 참조한다. Debug F1 `Balance Tool`에서 JSON
저작 → provenance 동기화 → Validate/Publish를 수행하고 Server를 재시작해 적용한다. 레벨별 override가
필요하면 별도 schema와 우선순위/rollback 계약부터 정해야 한다.

## 6. 여섯 캐릭터 roster 상태

Lobby와 Character Select는 Lance Master, Gunslinger, Slayer, Artist, DimensionMaster, Warlord 여섯 class를 선택할 수 있다. 실제 world 진입과 Character Select class 변경은 `Is_Supported_Playable_Character_Class`가 승인한 class만 가능하다.

| Class | Lobby 선택 | Resource pack | Client Loader/Spec | Server profile | World 진입 |
|---|---:|---:|---:|---:|---:|
| Lance Master | 가능 | 있음 | 완료 | 완료 | 가능 |
| Gunslinger | 가능 | 있음 | 완료 | training baseline | 가능 |
| Slayer | 가능 | 있음 | 완료 | training baseline | 가능 |
| Artist | 가능 | 있음 | 완료 | training baseline | 가능 |
| DimensionMaster | 가능 | `.3`에는 없음, 로컬만 | 완료 | training baseline | 가능(로컬 payload 필요) |
| Warlord | 가능 | 있음 | 완료 | 완료 | 가능 |

기존 resource pack class는 body·equipment·weapon `.wmodel`과 texture를 6-root resource pack에 admission한다. DimensionMaster는 combined body `Character/DimensionMaster/DimensionMaster_Character.wmodel`과 `Character/WP_WSWP_M_06`의 L/S/P/E 네 정적 기본 무기 파츠를 로컬 payload로 사용한다. 여섯 class 모두 `CharacterCatalog`, `CCharacterCatalog`, Loader prototype, Server `PlayerProfiles`, class parser, spawn/remote presentation까지 연결한다. LanceMaster로 조용히 대체하는 identity fallback은 금지한다.

Area 진입 시 여섯 class binary를 모두 선로드하지 않는다. Lobby가 승인한 class만 먼저 준비하고 다른 class는 Character Select의 class-change 요청 전 또는 실제 remote snapshot에서 최초 한 번만 같은 `CPlayableCharacterAssetService`로 admission한다. 이 규칙은 mixed-class 표현을 유지하면서 불필요한 Level 로딩 증가를 막는 고정 경계다.

## 7. 새 Area 추가 체크리스트

1. `LevelCatalog.json` stable scenario와 기존 Engine Level 매핑
2. `MapCatalog.json` Area 및 실제 사용하는 visual admission만 등록
3. authoring placement → 원자 publish → runtime placement
4. class-neutral player spawn과 필요한 NPC/boss placement
5. navigation authoring, spawn/boss cell·height 검증
6. 필요한 actor/encounter/balance stable ID 연결
7. Loader/registry/publisher 연결과 해당 domain Validate/실행형 harness 등록
8. Debug/Release scenario smoke와 process cleanup
