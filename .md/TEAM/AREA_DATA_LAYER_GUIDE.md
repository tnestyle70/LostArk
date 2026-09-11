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
| `LV_LUT_HEARTRB_ED` | 279 assets / 13,184 placements | player spawn 4 + `BOSS_VALTAN` 1 | 392×312, 0.5m cells, `Data/Navigation/LV_LUT_HEARTRB_ED.*` | deploy pair, source-exact outer towers, map point light 22, source stone material/baked lighting 7 placements, BossProfile, ValtanEncounter |
| `LV_DEV_TRAINING_GROUND` | RCArena 10 assets / 18 placements | class-neutral player spawn 4 | uniform 32×32 | NPC/boss/monster/trigger 없음 |
| `LV_LOBBY_CLASSSELECT_SL00` | 55 assets / 803 placements | class-neutral player spawn 4 | Server uniform 42×60 + MapTool source/paint bootstrap | Character Select Arena gameplay + monster/Lugaru SpawnGroups |
| `LV_SHS_RCARENA_D` | 302 assets / 7,856 placements | 없음 | 없음 | 원본 Training Map 편집 대상 |

수련장은 Lobby의 `Enter Training`에서 Server 승인을 받은 뒤 `LEVEL::DEVELOPMENT`로 진입한다. Debug/Release network smoke는 map load, player spawn, Q command, Server action 승인, cooldown HUD 반영까지 검사한다.

## 3. 레이어별 생략 규칙

발탄 중앙 석재 7배치는 `MapCatalog.json`의 `sourceMaterials/materials` 쌍으로 선언한
`LV_LUT_HEARTRB_ED.mapmaterials.json` formatVersion 2를 소비한다. 선택 family
`bg_base_opa_overlay`는 기본/overlay D/N 네 입력과 배치별 원본 COLOR0, UV1 및 tangent
handedness가 필요하다. `placementLighting`은 sourcePlacementId와 variant assetId별
평균색·방향 lightmap의 atlas 좌표와 RGB 계수를 운반한다. 필수 geometry/texture가 빠지면
해당 Area stage가 실패하며 흰 정점색이나 UV0를 정상 입력으로 대체하지 않는다.

catalog는 `Data/Maps/Imported`, placement/materials는 `Data/Maps/Authoring`에서 관리하고
`Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED`로 함께 배포한다. 이 변경을 공유할
때는 Git 제외 `Resources/Map/LV_LUT_HEARTRB_ED/SourceStoneRestore/`와
`Resources/Map/Lighting/Valtan/`도 필요하다. 기존 Deploy·파괴·Server gameplay 계약은
이 표면 재질 문서에 포함되지 않는다.

| 레이어 | 없을 때 | 불완전할 때 |
|---|---|---|
| visual map | scenario가 Map domain을 요구하면 load 실패 | catalog/placement 참조 오류는 rollback |
| deploy | `.deployassets`와 `.deployplacements`가 모두 없으면 skip | 둘 중 하나만 있으면 오류 |
| point-light presentation | `sourceLights`와 `lights`가 모두 없으면 skip | 둘 중 하나만 있거나 문서 검증이 실패하면 publish/load 실패 |
| 선택적 map material | `sourceMaterials`와 `materials`가 모두 없고 미선언 저작 파일도 없으면 기존 계산 | 선언 누락·미등록 asset/material·지원하지 않는 family·필수 texture 오류는 publish/load 실패 |
| NPC/boss placement | 해당 kind 행이 없으면 spawn하지 않음 | unknown archetype/encounter는 publish 실패 |
| navigation | navigationRuntime을 선언하지 않은 world만 생략 가능 | Bern/Valtan/Training/Character Select Arena는 grid/policy 누락·손상 시 room 기동 실패 |
| balance definition | 사용하지 않는 actor/skill 정의는 runtime state를 만들지 않음 | placement/action이 없는 stable ID를 참조하면 publish 또는 Server load 실패 |

`Gameplay.world.json` 자체는 Server가 여는 world마다 필요하다. 접속 가능한 world는 최소 하나의 활성 `playerSpawn`이 필요하므로, 빈 placements 문서를 제품 world의 정상값으로 취급하지 않는다.

### 3.1 선택적 map material 입력

`MapCatalog.json`의 `sourceMaterials`/`materials` 쌍은
`Data/Maps/Authoring/<AreaId>/<AreaId>.mapmaterials.json`과
`Client/Bin/DataFiles/Map/<AreaId>.mapmaterials.json`을 선언한다.
`lostark.map-materials` formatVersion 1/2의 재질 key는 `assetId + materialName`이다.
`sourceMaterial`은 원본 근거이며 WModel의 material 이름과 별개다.
v1/2의 `diffuse-sampler` 행은 `assetId`, `materialName`, `sourceMaterial`, `family`,
`sourceTexture`, `addressU`만 저장하며 `addressU`는 `WRAP` 또는 `MIRROR`다.
원본 Texture2D의 주소 방식이 필요한 legacy diffuse 재질에만 사용하고 V축은 WRAP을 유지한다.
기존 diffuse 경로·UV·표면 수치·그림자 정책은 보존한다. 다른 surface family와 같은 material key로
중복 선언하지 않는다. `sourceTexture`는 원본 object 경로 근거이고 새 texture 교체 경로가 아니다.
v1의 `bg_seamless-specular_msk`, `bg_base_msk`를 유지하며, v2는
`bg_base_pbr_seamless_opa`, `bg_base_pbr_opa`의 원본 채널·계산을 지원한다.
PBR 입력에는 texture별 색 공간, optional `bakedLighting`/`environment`가 있다.
`placementLighting`은 stable `sourcePlacementId`와 일치하는 `assetId`에 atlas 좌표와 광량 계수를
연결한다. baked 입력을 선택한 모델은 실제 UV1이 있어야 하며 환경 cube와 BRDF 입력은 함께 요구한다.
필드·원본 근거·근사 경계는 해당 바닥 복구 PLAN/RESULT를 따른다. 임의의 모든 ORM 재질을
이 family로 대신 해석하지 않는다.

`Publish-MapAuthoring.ps1`은 요청 material 이름이 실제 WModel에 정확히 한 번 있는지,
texture 경로·finite 수치·색 공간과 source/runtime 경로 쌍을 검사하고 문서와 catalog를 함께 교체한다.
선택적 재질을 선언한 Area의 runtime mapassets header는 version 5이며 필수 재질 파일명을 가진다.
v4 row 형식은 유지하며 기존 v1~v4와 mapset v1도 읽는다. shard-set은 각 child header에 같은 참조를 둔다.
Loader와 MapTool은 같은 named override를 `CModel -> CMaterial`에 전달한다. 로드 실패는 기존 catalog를 보존한다.

MapTool은 이 문서를 읽지만 재질 수치 편집/저장 UI는 제공하지 않는다. 저작 JSON 변경 후 명시적 publish와
Client 재시작이 적용 기준이다. F1 Rendering Workbench의 Floor Materials은 session-only A/B와
기여 진단이며 Runtime Reload 또는 Quality Save를 재질 prototype 갱신으로 사용하지 않는다.

Character Select의 map light도 기존 `CMapLightPresentationRuntime`을 사용한다.
Lighting Workbench는 현재 Level/Area가 일치할 때 저작 preview와 published reload를 연결한다.
광원 로드 실패는 기존 runtime/preview를 유지한다. material prototype 재생성과 광원 reload는 별개다.

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
- world sequence: visual source와 같은 폴더의
  `Data/Maps/Authoring/<Area>/<AreaId>.worldsequences.json`
- animated prop placement: catalog가 `sourceDeployCatalog`/`sourceDeployPlacements`
  pair를 선언한 Area의 `Data/Maps/Authoring/<Area>/<AreaId>.deployplacements`
- point-light presentation: catalog가 선언한 `Data/Maps/Authoring/...maplights.json`
- gameplay: Character Select/Bern/Valtan의 exact `Data/Worlds/.../Gameplay.world.json`
- navigation: 정책이 허용한 `Data/Navigation/*.navsource/.navpaint/.navblockers`

한 Area 안에서 스테이지마다 필요한 정밀도가 다르면 세부 영역 격자를 쓴다.
`Data/Navigation/<AreaId>.navregions`에 `REGION "<regionId>" <stepHeight>` 행을 두면
MapTool의 Navigation 패널에서 그 영역을 골라 별도 Nav Bounds와 Cell Size로 Bake한다.
영역은 자기가 덮는 스테이지의 걷는 범위 전체를 덮어야 한다. 플레이어가 걸어서 영역
밖으로 나가는 지형에는 쓰지 않는다. 영역끼리 겹치면 publisher와 Server가 모두 거부하고,
영역에는 runtime blocker를 둘 수 없다. 매니페스트가 없으면 Area는 기본 격자 하나로
종전과 동일하게 동작한다.

쿠크 2관문의 `BOSS_KAKULSAYDON_G2_BIG_SAYDON`은 저장한 높이에서 서는 보스다.
Navigation publisher도 Server `Build_WorldEntity`와 같은 해당 Area/archetype의 높이 정책을 사용하며,
유한한 좌표·영역 안의 XZ·walkable 검사는 그대로 적용한다. 다른 보스와 playerSpawn은 지면 높이를 검사한다.

### F1 Sequence Viewer

Debug Client의 모든 Level에서 `F1 > Sequence Viewer`를 열면 `KoukuSaydon / 쿠크세이튼`와
`Valtan / 발탄` 탭을 볼 수 있다. 현재 Area와 관계없이 트리거, 맵 시퀀스 인스턴스, 기존
Boss Tool의 서버 패턴을 조회하며, 한국어 이름·구역·동작·원본 stable ID로 검색한다.
`Data/Maps/SequenceViewer.labels.json`은 표시 이름만 소유한다. `kind`는
`trigger/sequence/pattern`, `targetId`는 각 기존 ID, `displayName/location`은 한글을 포함한
표시 문자열이다. 이 파일을 바꾸고 Refresh하면 이름만 바뀌고 실행 연결은 바뀌지 않는다.

Test의 Play는 MapTool의 현재 문서와 기존 WorldSequencePlayer/카메라를 사용한다.
이동·소환·전투와 공동 재생은 해당 Server-approved 아레나에서 실행한다. 다른 Level에서는
Enter Arena로 기존 입장 절차를 요청한다. Go To는 Test에서 카메라를, Arena에서 요청한
플레이어만 이동시킨다. Arena에서 트리거에 도착하면 그 트리거가 발동할 수 있다.
Replay는 한 번 실행한 트리거도 같은 authored action으로 다시 요청한다. Stop은 선택 연출의
표현만 정리하며 피해·소환·보스 전투나 다른 플레이어의 진행을 롤백하지 않는다.
보스 패턴의 세부 제어는 Open Editor가 여는 기존 Boss Tool에 유지된다.

`Publish-WorldGameplay.ps1`은 worldbootstrap v9에 활성 시퀀스 ID 목록을 함께 저장한다.
Server는 이 목록으로 직접 재생/정지 명령을 확인한다. Client용
`DataFiles/World/<Area>.viewer.world.json` 및 `SequenceViewer.labels.json`은 원본 checkout이
없는 Debug 배포본의 읽기 전용 목록이다. 맵 연출 자체는 기존 Map publisher의
worldsequences.json이므로 맵 연출을 변경했다면 Map과 World를 모두 publish해야 한다.
Shared protocol은 66이며 Server와 모든 Client를 함께 빌드·재시작해야 한다.
서버 승인과 broadcast는 화면 성공 판정이 아니다. 연출 화면과 4인 동시 확인은 사용자가 한다.

`World Sequence`는 map placement, Deploy ANIM과 생성형 World Object의 상대 위치·회전·크기·표시
상태를 시간축으로 편집하는 재사용 저작 레이어다. `templates`는 이름을 가진 상태와 동작 정의,
`instances`는 template slot과 stable target ID의 Area별 연결을 소유한다. MapTool의
`Save`는 visual placement와 world sequence를 백업·사후 재검증·rollback이 있는 연결 저장으로
처리하며, 중단 marker가 남으면 다음 Area load 전에 원본 pair를 복구한다. Background render
mode처럼 카메라가 transform을 소유하는 placement는 sequence target으로 거부한다.
같은 Area의 load/save는 exclusive sidecar lock으로 직렬화하고, Reload 때 읽은 두 원본의 byte
baseline이 Save 직전과 다르면 stale editor 저장을 거부한다. sequence JSON은 parse 전에 16 MiB
한도를 적용하며 저장 후에는 단순 유효성뿐 아니라 의도한 map/sequence 내용과 같은지도 비교한다.

WorldSequence JSON의 저장 버전은 `formatVersion: 3`이며 `CWorldSequenceDocument`는 기존 v1/v2도
읽는다. v3의 `objectResources`는 stable `objectId`, `displayName`, Resources-relative `.wmodel`
`modelAssetId`, optional `diffuseTextureAssetId`, `modelPreScale`(기본 0.01), `animated`, `scale`을
소유한다. 기존 배치 연출의 별칭은 `modelAssetId`를 비우고 `sequenceInstanceId`로 기존 instance를
참조한다. 커튼·룰렛의 stable instance ID와 기존 key는 그대로 유지한다. 생성형 모델의 binding은
`targetKind=OBJECT_RESOURCE`, `targetId=objectId`이고, 기존 `MAP_PLACEMENT`/`DEPLOY_PLACEMENT`
binding도 유지한다. 모델 path와 별칭 instance를 동시에 지정하거나 별칭을 모델 binding으로 쓰는
저장은 거부한다.

새 상태도 기존 `templates`/`instances`에 저장한다. optional `objectMotion`은 `velocity`(m/s),
`acceleration`(m/s²), `angularVelocityDegrees`·`revolutionDegreesPerSecond`(각 축 deg/s),
`revolutionOffset`(m), `count`(1..128), `intervalMs`, `spreadDegrees`(0..180), `seed`를 가진다.
생략하면 1개·추가 이동/회전 없음이다. 마지막 생성 시각 `(count-1)*intervalMs`는 template의
`durationMs`보다 작아야 하며, 생성된 오브젝트는 같은 상태 수명 안에서 재생한다.
optional `emissions`는 seed 분산을 대신하는 저작 사본 목록이다. 각 행은 `positionOffset`(±100000),
`yawDegrees`(-36000..36000), `startDelayMs`(0..600000)를 정확히 갖고 1..128행이다. 행이 있으면
`count`는 행 수와 같고 `intervalMs`·`spreadDegrees`는 0이어야 하며, 마지막 생성 시각은 최대
`startDelayMs`다. 행 yaw는 그 사본의 로컬 이동과 공전 offset을 함께 돌리므로 offset을
`R(yaw)*revolutionOffset`로 두면 모든 행이 저작 위치를 중심으로 한 원을 돈다. 행이 없는 문서는
기존 동작을 그대로 유지한다. 생성형 instance의
optional `anchorKind`는 고정 `WORLD` 또는 살아 있는 복제 플레이어 각각을 따르는 `PLAYER`,
`position`은 해당 anchor의 상대 위치다. 기본값은 `WORLD`와 `[0,0,0]`이며 기존 배치 binding에는
플레이어 anchor나 추가 instance 위치를 적용하지 않는다. ANIM resource는 기존 `animationTracks`를
사용하고 실제 clip 존재 여부는 모델 admission에서 검증한다.

Animation track의 optional `displayName`은 빈 값 또는 최대 128 UTF-8 byte의 표시 이름이다.
실제 WModel lookup은 기존 `clipName`만 사용한다. Object Tool의 이름 편집과 Timeline은
`displayName`을 사용하며 비어 있으면 `clipName`을 표시한다. 표시 이름 변경이 재생 연결을 바꾸지 않는다.
Physics의 `Apply Vertical Arc`는 높이 H와 Lifetime T로 기존 velocity Y=4H/T,
acceleration Y=-8H/T²를 저장한다. 생성 개수·간격을 보존하고 첫 생성의 높이 곡선을 Timeline에
표시한다. 이후 생성도 전체 상태 종료시각을 공유한다. 별도 PhysX simulation이나 저장 곡선 schema는 없다.

쿠크 마리오의 줄무늬 공은 `world.sequence.instance.mario.striped_ball.bounce`의 기존
MAP_PLACEMENT track을 Level이 순환 Seek하는 순수 외형이다. Server-replicated `iMarioStage`
1~4일 때만 같은 Server tick 시계로 재생하며 마리오 밖에서는 원래 배치를 복원한다.
각 track의 상대 Y 키가 높이와 위상을 소유한다. 이는 일반 MAP_PLACEMENT의 `motionEnd=LOOP`
지원이나 이동 collider/피해 판정이 아니다. 같은 placement에 다른 활성 시퀀스를 동시에 적용하지
않으며, 활성 시퀀스의 기존 Preview/Reload guard를 풀려면 마리오 밖으로 나간다.

원본 마리오 색 공은 `source:37081:npc:<actorId>` provenance와 imported placement ID로 저장한다.
117개 후보는 기본 hidden이며 `world.sequence.instance.marioN.source.layoutC` 12개 시퀀스에서
선택된 배치만 보여준다. MAP_PLACEMENT의 STOP은 마지막 가시성을 유지하며 명시적 Stop(true)가
hidden baseline을 복구한다. 일반 공 13개의 bounce와 고정 해골 폭탄 7개는 별도 공통 배치다.
MapTool의 World Sequence에서 원본 배치 이름을 검색해 특정 Case를 미리 볼 수 있다.
placement와 시퀀스는 Map publisher, 몬스터 anchor/group은 WorldGameplay publisher로 내보낸다.
선택·수명·프로토콜은 팀 사용서 4.3을 따른다.

마리오 플레이어의 광대 외형은 replication의 `iMarioStage` 1~4에서만
`CCharacter::Apply_MarioPresentation`이 body part local scale을 적용한다. 기본 대기 자세의
머리 장식 포함 높이 1.5m가 기준이며 자세에 따른 높이 변화는 유지한다. stage를 벗어나면
광대 body scale 1로 복구하고 일반 클래스, 공통 모델 prototype과 Server 충돌 크기는 변경하지 않는다.

마리오 2~4의 `Mario2_Boom`, `Mario2_Boom_1/2`, `Mario3_Boom`, `Mario3_Boom_1/2`,
`Mario4_Boom`은 밟는 action 없이 disabled triggerBox로 저장하는 발사 위치 marker다.
World Gameplay에서 위치를 저장한 뒤 `Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish`로
내보내고 Client에 재진입한다. 제품은 published `World/LV_LUT_MIDNIGHTC_ED.viewer.world.json`의
marker와 기존 마리오 진행선 끝점을 읽으며 source 직접 fallback을 사용하지 않는다.
해당 Server `iMarioStage`의 살아 있는 로컬 플레이어가 있을 때만 4초 간격·약 3m/s로
기존 `world.object.mario.clown_face_ball`을 WorldSequenceObject/CModel 경로에 생성한다.
marker에서 같은 진행선의 먼 끝점까지 수평 직진하고 도착·퇴장·사망 때 제거한다.
광대 얼굴 공의 모델 +X 앞축을 실제 비행 방향으로 회전시키며 world-up을 유지한다.
비대칭 모델의 바닥 중심 보정도 함께 회전한다. 카메라 추적 billboard는 아니다.
발사마다 stable marker/Server 시각 해시로 기본 바닥 정렬 높이에 +0.05m 또는 +0.90m를 더한다.
box halfExtents는 모델 scale이 아니며 모델은 기존 resource scale을 유지한다.
이는 피해 없는 Client 비행 표현이고 충돌·폭발·Server combat-object spawn은 포함하지 않는다.
위치/라인 데이터는 진입 시 읽고, 속도·간격·두 높이는 현재 Level WorldObjects의 MARIO_BOMB 상수다.

F1 `World Object Tool`은 이 Area source와 부모의 optional `defaultMotionInstanceId`를 편집·저장한다.
명시한 기본 상태는 enabled인 같은 Object의 단일 binding이어야 한다. 기존 Map alias는 원래 sequence ID를 사용한다.
Action Workbench는 부모 Object를 선택해 기본 상태로 Append하며, WORLD definition의 optional
`objectResourceId`와 `sequenceInstanceId`가 부모 및 Append 당시 초기 상태를 저장한다. 이후 기본 상태를
변경해도 기존 박스의 초기 상태는 바뀌지 않는다. Map 모델 occurrence의 optional `placement`는 절대
`position`, degree `rotationDegrees`, Object 기본 크기의 배수 `scale`을 소유한다. 기존 placement 없는 문서는
원래 동작을 유지한다. 이 TRS는 projector → Server → Shared protocol 69 → 기존 Client player로 전달된다.
owner 정상 완료의 `FINISH_OWNER`는 이미 시작한 WORLD의 저작 수명을 보존하며, 취소·실패의
`STOP_OWNER`는 즉시 정리한다. protocol 68 실행 파일과 혼용하지 않고 Server와 Client를 함께 갱신·재시작한다.
Object Tool Save가 기존 `Publish-MapAuthoring.ps1 -Scope WorldSequences`를 비동기 실행하며 이 scope는
해당 Area worldsequences만 원자 배포한다. 조명·카메라 등 다른 Area 파일은 갱신하지 않는다.
이 scope의 MAP/DEPLOY binding은 이미 설치된 runtime catalog·placement와 조인한다.
존재하지 않는 placement/asset, Sky map, STATIC Deploy 대상이면 기존 runtime을 유지하고
배포를 거절한다. 새 맵 배치까지 추가한 작업은 `-Scope Area`로 함께 배포해야 한다.
Area scope는 같은 transaction에서 stage한 catalog·placement를 검증 기준으로 사용한다. 생성과 상태 sampling·수명·정리는 기존 `CWorldSequencePlayer`가 소유하며, 렌더 객체는
`CWorldSequenceObject -> CModel -> CMaterial` 경로를 사용한다. 별도 Effect asset이나 두 번째
오브젝트 재생 runtime을 만들지 않는다.

Object Tool의 부모 선택은 공통 모델·텍스처·크기·Anchor와 연결된 Motion 목록만 표시하고,
자식 선택은 해당 Motion의 Detail과 Sequencer를 표시한다. `Create Object`는 부모만 만들고
모델을 지정한 뒤 `Create Motion`으로 자식을 추가한다. `Append Clip`은 자식 선택에서만 가능하다.
저장 모션의 호출 ID는 기존 `instances[].instanceId`이며 native `clipName`이나 표시 이름이 아니다.
instance의 optional `motionEnd`는 `STOP`(기본), `HOLD`, `LOOP`, `NEXT`다. `NEXT`의
`nextMotionId`는 같은 Object·slot의 enabled 단일 생성 모션만 가리키며 순환과 32개를 넘는
연결을 거부한다. `HOLD`는 마지막 자세, `LOOP`는 같은 모션 반복, `NEXT`는 같은 객체에서
다음 모션 재생이다. 독립 WORLD의 `STOP`은 기존 수명 종료를 유지한다. Result로 이미 생성된
객체에 적용한 `STOP`은 마지막 자세에서 모션만 정지하며 target WORLD cue가 객체 수명을 소유한다.
적용 모션의 초기 지연에는 기본 모션을, 다음 모션의 지연에는 직전 마지막 자세를 유지한다.

쿠크 `OBJECT_OVERLAP`은 저작 Collider와 고정 카드 원의 겹침을 Server에서 판정한다.
`targetWorldInstanceId`와 미터 단위 `targetRadiusM`를 저장하며 native mesh/bone 자동 collider가 아니다.
target은 같은 패턴에서 판정 창 전체를 포함하는 단일 WORLD occurrence, WORLD anchor, Count 1,
고정 Transform과 추가 물리 이동 없는 Object여야 한다. 움직이는 source Collider는 지원되는
WorldTrack의 변환을 사용한다. Result의 `PLAY_WORLD_OBJECT_MOTION`은 같은 target과 같은 부모의
`motionInstanceId`를 참조한다. Client는 기존 target 객체의 모델과 배치를 유지하고 모션만 바꾼다.
이 판정의 target 기본 모션은 `LOOP/HOLD`로 계속 표시해야 한다. Result와 NEXT 후속 모션도
enabled·같은 slot·고정 Transform을 유지하고 `LOOP/HOLD`에 도달해야 한다. 비활성 모션이나
판정 창 도중 숨겨지거나 다른 위치로 움직이는 연결은 publisher가 구체적인 이유와 함께 거부한다.
이는 동적 navigation이나 Client mesh에 의한 damage 판정을 추가하지 않는다.

`World Sequence` 모드의 `Animated Props`는 그 sequence가 binding할 Deploy ANIM 배치를
만드는 곳이다. Area catalog가 `sourceDeployCatalog`/`sourceDeployPlacements` pair를 선언한
경우에만 열리고, catalog에 등록된 조리 완료 `.wmodel` ANIM asset만 목록에 나온다. 원본
glTF/PSK/PSA bundle은 catalog가 파일 실재를 검사해 거부하므로 조리 전 자산이 런타임 자산으로
위장 등록되지 않는다. 배치는 뷰포트 depth pick 위치에 생성하고 위치·회전·크기를 편집·삭제할
수 있으며, 이때 저장되는 행은 `provenance=PROJECT_AUTHORED`다.

placement 문서는 format version 2이며 마지막 열이 provenance token이다. 추출 원본 행은
`SOURCE_EXACT`로 남고 읽기 전용이다. project 행은 `deployActorId`, `propDefinitionId`,
`stateOffActionId`, `triggerBinaryOccurrenceCount`를 모두 `0`으로 유지해 추출 증거를 사칭할 수
없고, `runtimePlacementId`는 map placement와 같은 editor ID domain 규칙을 따라
`0x7fffffffffffffff` 이하만 사용한다. 저장은 temp 파일 + `MoveFileExW` 원자 교체 뒤 같은 pair를
다시 읽어 의도한 행과 정확히 같은지 확인한 다음에만 dirty를 해제한다. 편집·삭제는 catalog를
복사해 검증한 뒤 Deploy runtime 전체를 다시 stage하는 방식이므로, sequence preview와 destruction
preview seam을 먼저 반납하고 실패 시 이전 배치를 유지한다. 이미 world sequence animation track이
binding한 배치를 지우려 하면 sequence validator가 거부하고 Deploy runtime을 이전 상태로 되돌린다.
`Save All`은 sequence가 stable Deploy placement ID를 참조하므로 Deploy 문서를 먼저 저장하고,
성공한 뒤에만 map placement + world sequence 연결 트랜잭션을 실행한다.

쿠크세이튼(`LV_LUT_MIDNIGHTC_ED`)의 현재 ANIM asset은 `DEPLOY_ITR_02283`(레버, clip
`go_on`/`go_off`/`on`/`off`)와 `DEPLOY_BG_RAD_KOUKUSATON_PAPERSTAGE`(종이 펼침, clip
`evt2_paperstage_open01`, 실제 길이 3066.667ms) 둘이다. 종이 펼침은 정적 메시의 rigid transform이
아니라 skinned clip이므로 sequence의 transform track이 아니라 animation track으로 재생한다.

WorldSequence의 제품 재생은 Server가 보낸 stable instance cue를 Client의 기존 player가 소비하는
presentation 경로다. World Object의 transform과 mesh는 Client 표현이며 damage 판정이나 동적
collision/navigation 개방을 추가하지 않는다. 새로운 gameplay 상호작용은 해당 Server 권위와
Shared 상태 계약을 별도로 연결해야 한다.

Bern은 `Place Nav Bounds`로 실제 렌더 바닥을 고른 뒤 Bottom Y와 Height from Bottom으로 세로 범위를
제한해 bake한다. 제품 runtime은 이미 활성화되어 있으므로 source/paint/policy가 누락되거나 손상되면
Server room admission이 실패한다. publisher는 실제 bake 결과, player spawn/trigger 연결성, cell
통계와 Area별 step policy를 함께 검증한다.

Navigation `Walkability` 브러시는 `Block`, `Force Walkable`, `Reset`을 제공한다.
`Force Walkable`은 선택한 grid 범위 안의 실제 렌더 표면을 피킹해, bake가 놓친 빈 셀에도
명시적 높이를 가진 통행 셀을 추가할 수 있다. 기존 높이가 있는 셀은 기본적으로 그 높이를 유지한다.
`Use Picked Height`를 명시적으로 켜면 기존 셀도 클릭한 표면 높이로 교체한다. 브러시 범위에 같은
높이가 적용되므로 겹친 층이나 경사진 곳은 Brush 0부터 확인한다. 피킹 실패/선택 grid 밖/잘못된
높이는 상태 문구로 알리고 셀을 바꾸지 않는다. Walkability에서는 live 셀을 표시하고 미저장 Bake
Preview는 Bake 모드에서만 표시한다. Client 제품 아레나는 열람용이며 편집은 Lobby → Test에서 한다.

수동 override는 bake보다 우선하며 `Reset`은 walkability와 명시적 높이를 원래 bake 상태로 되돌린다.
`.navpaint` version 3은 `x z BLOCKED|WALKABLE [height]` 또는 `x z HEIGHT height`를 저장한다.
`WALKABLE height`는 원래 `NO_SURFACE`였던 셀도 명시적인 바닥으로 만들 수 있다. 기존 version 2의
`x z BLOCKED|WALKABLE`과 version 1의 `x z`(`BLOCKED`)도 호환 로드한다. 저장 후 제품 반영에는
Navigation publisher 실행과 Server 재시작이 필요하다. 재베이크 없이 paint만 저장·배포할 수 있다.

MapTool은 Client `.navgrid`/`.navpolicy`를 export하거나 제품 Navigation runtime blocker를 등록하지 않는다.
Visual runtime은 `Publish-MapAuthoring.ps1`, world bootstrap은
`Publish-WorldGameplay.ps1`, Server navigation은 `Publish-ServerNavigation.ps1`만 교체한다.
`ACTIVE.maparea`도 selector 변경 때 자동 저장하지 않는다.

Visual publisher는 `-AreaId <AreaId> -Mode Validate|Check|Publish`를 지원한다.
Validate는 source를 검증하고, Check는 같은 예상 파일과 현재 runtime의 byte 일치만 검사한다.
두 모드는 runtime을 쓰지 않으며 Publish만 기존 파일 집합 transaction을 실행한다. 기본값은
기존 호출과 같은 Publish다. 현재 Area가 선언한 파일만 대상으로 하며 다른 Area나 미참조 파일을
자동 삭제하지 않는다. 상세 명령은 [Map pipeline 사용서](../../Tools/MapPipeline/README.md)를 따른다.
`Client/Bin/DataFiles/Map/*.mapassets`와 `*.mapplacements`는 Git LFS 추적 생성물이다.
맵 담당자는 아래 명령으로 Area 출력을 생성·확인한 뒤 Data 원본과 변경된 runtime
catalog·placement·시퀀스 및 관련 출력을 같은 PR에 포함한다. 받는 PC는 같은 commit과
LFS 실파일을 받아 실행하며, 맵 배치가 바뀌면 열린 Client를 다시 시작한다.
받은 데이터의 일치는 `-Mode Check`로 확인할 수 있다. 로컬 저작본을 수정하지 않았다면
맵을 전달받기 위해 별도 Publish를 반복할 필요가 없다. `Resources` 모델/텍스처는
기존 Drive 계약으로 전달한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Check
```

명시적 전체 쿠크 배포 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`도
`map.kakulsaydon`을 포함한다. 기존 BuildDomains의 해당 Area 출력·필수 출력은
mapassets, mapplacements, deployassets, deployplacements, maplights, mapmaterials,
worldsequences, camerashots의8개이며, 누락된 출력은 기존 receipt로 생략하지 않는다.
일반 Client 컴파일은 계속 자동 배포를 하지 않는다. 컴파일 성공과 실행용 데이터 준비는
별도로 확인한다. 생성물은 직접 편집하거나 `git add -f`로 우회하지 않고 기존 LFS
규칙과 일반 `git add`로 추적한다. 다른 domain의 Git 제외 정책은 변경하지 않는다.

Valtan 파괴 벽의 `navblockers`는 각 source collisionBox의 실제 XZ OBB와 base-walkable 셀, 해당 셀 높이에서
시작하는 Server body 수직 범위가 겹치는 곳만 소유한다. 위층 벽으로 아래층 길을 막거나 nearest cell을
대신 배정하지 않는다. 겹치는 벽의 blocker refcount는 각각 유지한다. 99개 벽 중 98개에 wall region이 있으며
바닥 붕괴 6개 region의 `BLOCK_WHILE_FRACTURED` 계약과 기본 navgrid는 별도다.
벽 collision 또는 body 높이를 바꾸면 `Split-ValtanIndependentWallGroups.ps1 -Mode RebuildNavigation`으로
authoring을 갱신하고 `-Mode CheckNavigation`, `Publish-ServerNavigation.ps1 -Mode Validate`,
`Publish-ValtanWorldDestruction.ps1 -Mode ContractTest`로 검사한 뒤 각 publisher의 Publish를 실행한다.
파괴의 최종 commit은 같은 mutation이 source와 charge receiver collision, 해당 nav region을 함께 해제한다.

MapCatalog의 `sourceLights`와 `lights`는 선택적인 한 쌍이다. source는
`Data/Maps/Authoring/<AreaId>/<AreaId>.maplights.json`, runtime은
`Client/Bin/DataFiles/Map/<AreaId>.maplights.json`만 허용한다. schema
`lostark.map-light-presentation` formatVersion 1은 stable `lightId`, source level/object ID,
world position, radius, falloff, RGBA와 brightness를 소유한다. publisher는 이 문서를 visual placement와
optional deploy pair와 같은 파일 집합 트랜잭션으로 교체하며 중간 실패 시 전부 rollback한다. MapTool은
source 문서를, 제품 Level은 runtime 문서를 읽고 둘 다 기존 `CPresentation_Manager`의 transient point-light
경로에 제출한다. 이 레이어는 Client 시각 표현 전용이며 Server gameplay 판정이나 광원 충돌을 만들지 않는다.
Valtan은 catalog가 이 pair를 선언하므로 누락·손상을 정상적인 생략으로 취급하지 않고 Area stage를 실패시킨다.

쿠크 `LV_LUT_MIDNIGHTC_ED`도 같은 pair를 선언한다. formatVersion 2는 `PROJECT_AUTHORED` 문서이며
0~64개의 Directional/Point/Spot에 stable `lightId`, `displayName`, `groupId`, `enabled`, 위치·회전·range·falloff·cone·RGBA·brightness를 저장한다.
Rendering Workbench의 Map 목록은 player 위치를 기준으로 point/spot을 생성하고, 변경값을 Area source에 저장한다.
Publish는 기존 Map publisher, 제품 로드는 `CMapLightPresentationRuntime`을 사용한다. 잘못된 새 source/preview는 이전 문서를 보존한다.
같은 v2 map light는 Composition Light 탭에 읽기 전용 정의로 표시된다. Append는 stable lightId를 참조하며
조명 값을 LightResources.json에 복제하지 않는다. Composition에서 사용하는 map light의 삭제는 참조 해제 전 거부한다.
Default Directional Light는 Scene Profile의 기존 방향광을 편집하는 목록 행이다. maplights에 별도 기본광을 추가하지 않는다.
활성 RenderingProfiles의 optional `mapLightIntensityMultiplier`(0~4, 기본 1)는 실제 Map light 제출 때 brightness에 곱한다.
이 배율은 기존 v1/v2 맵 배치와 그 저작 preview에 적용하며 원본 brightness를 바꾸지 않는다. 패턴의 LIGHT occurrence에는 적용하지 않는다.

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
