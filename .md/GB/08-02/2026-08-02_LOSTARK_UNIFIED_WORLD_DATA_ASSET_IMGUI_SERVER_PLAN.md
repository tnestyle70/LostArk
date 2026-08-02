# LostArk 월드 데이터·에셋·ImGui·서버 통합 계획

작성일: 2026-08-02  
대상: 맵, 플레이어 스폰, NPC/발탄, 캐릭터, 이펙트, 애니메이션, HUD, ImGui 편집, 서버 권한, 팀 데이터 공유  
문서 성격: 전체 방향을 고정하는 마스터 계획. 실제 구현은 아래 세로 조각 순서로 나눈다.

## 1. 먼저 내릴 결론

질문의 핵심에 대한 답은 다음과 같다.

1. 저장소 루트에 Data 폴더를 만들고 사람이 관리하는 정본 데이터를 분리하는 것이 맞다.
2. 단, 모든 데이터를 하나의 JSON에 넣지 않는다. Catalog, World, Spawn, Effect, Navigation처럼 책임별 문서로 나눈다.
3. 사람이 자주 고치는 작은 JSON은 일반 Git으로 관리한다. 그래야 diff, merge, review가 가능하다.
4. 추출로 생성되는 수만 건의 base placement JSON만 경로 한정 Git LFS로 관리한다. 모든 JSON을 LFS로 묶으면 안 된다.
5. WModel, DDS, animation, audio 같은 실제 에셋은 JSON이 대신할 수 없다. 전체 Resources를 올리는 대신 월드와 기능 단위의 선택적 asset pack으로 공유한다.
6. Client/Bin/DataFiles와 Server/Bin/DataFiles는 직접 편집하는 정본이 아니라 Data에서 검증·변환·배포한 런타임 산출물로 취급한다.
7. 서버는 플레이어, NPC, 보스의 실제 위치와 상태를 소유한다. ImGui는 데이터를 편집하고 명령을 전달할 뿐 서버 런타임의 권한을 빼앗지 않는다.
8. 현재 CMainApp이 Debug ToolHost 역할을 계속 맡는다. 별도의 두 번째 에디터 런타임을 만들지 않는다.

즉, 목표 구조는 다음 세 층이다.

| 층 | 정본 여부 | 저장 방식 | 내용 |
|---|---:|---|---|
| Data | 정본 | 일반 Git + 일부 생성물만 LFS | 사람이 리뷰하는 Catalog, World, Spawn, 편집 layer |
| Shared Asset Packs | 바이너리 정본 | 공용 저장소/Drive/선택적 동기화 | WModel, DDS, animation, audio, cooked effect |
| Bin/DataFiles | 파생물 | 빌드·배포 산출물, 필요한 대형 파일은 LFS | Client와 Server가 빠르게 읽는 런타임 문서 |

## 2. 현재 상태 체크포인트

### 2.1 이미 잘 되어 있는 부분

- HeartRB 맵은 정확 일치 에셋 260개를 확보했고 현재 base와 overlay를 합쳐 13,103 placement가 있다.
- Bern은 13개 shard, 총 50,017 placement를 로드하는 구조가 이미 있다.
- 좌표 변환은 UE3의 X,Y,Z cm에서 Client의 X,Z,-Y m로 검증됐다.
- Map은 catalog와 placement가 분리되어 있고 stable asset ID와 source placement ID 개념이 있다.
- Map load는 parse → validate → stage → commit 순서이며 실패 시 생성 객체를 rollback한다.
- MapTool은 매 프레임 파일을 다시 읽지 않고 활성 문서를 메모리에 유지한다.
- Shared와 Server 프로젝트가 있고 서버 Room이 플레이어의 런타임 상태를 소유하기 시작했다.
- 플레이어 spawn packet에는 서버가 선택한 실제 위치와 yaw가 포함된다.
- Effect는 authored 문서와 cooked 문서를 나누는 기반이 있고 이름을 지정한 저장 경로도 있다.
- DataFiles의 map runtime 문서는 이미 Git LFS 대상이다.

이 기반은 폐기하지 않는다. 통합 계획은 현재 경로를 확장하고 정본과 파생물의 경계를 바로잡는 작업이다.

### 2.2 현재 난잡함의 실제 원인

| 영역 | 현재 실측 | 문제 |
|---|---|---|
| Map 정본 | .mapassets, .mapplacements, .mapset과 외부 추출 JSON이 혼재 | 사람이 편집할 원본과 runtime 산출물이 분리되지 않았다 |
| Bern 편집 | 큰 shard 문서는 사실상 read-only | 팀원이 작은 변경을 저장할 overlay 경로가 없다 |
| placement ID | editor local 증가 숫자 사용 | 여러 브랜치에서 동시에 저장하면 충돌할 수 있다 |
| NPC | Loader에 모델 4개, Level_Test2에 위치 4개 하드코딩 | catalog와 spawn 문서가 없고 재사용할 수 없다 |
| NPC 품질 | pilot 4개 중 Beda 외 3개는 rest pose/retarget 문제가 남음 | 데이터화와 시각 품질 완료를 혼동하면 안 된다 |
| Player Character | 모델과 장비 경로는 Loader, class spec은 C++ switch | 시각 데이터와 행동 로직의 경계가 없다 |
| Server Spawn | 플레이어 수 × 2 좌표로 즉석 계산 | 월드가 정의한 spawn point를 사용하지 않는다 |
| Server Data | 파일 로더가 없음 | NPC/보스/스폰을 서버 권한으로 확장할 기반이 없다 |
| Replication | 클래스는 있으나 Bern level owner에 완전히 연결되지 않음 | 서버 spawn 결과가 실제 level 생명주기와 아직 한 흐름이 아니다 |
| Effect | 코드 경로 Effect/Effect_Tool과 실제 Effect_Tool 폴더가 다름 | authored/cooked/resource root 계약이 흔들린다 |
| Animation | DataFiles/Anim 파일 이름을 툴이 직접 조합 | catalog와 dependency 확인이 없다 |
| HUD | HUD_Layout.cfg가 Resources 아래에 저장 | 데이터가 바이너리 리소스 폴더에 섞였다 |
| AssetCatalog.json | 예전 wmesh 경로와 구형 발탄 항목을 가진 파일, 실사용 reader 없음 | 이름만 catalog이고 현재 CModel/WModel 경로의 정본이 아니다 |
| Resources | SourceData 4.255 GiB, Map 1.599 GiB, Character 0.237 GiB 등 | 원본 DB와 실행 에셋을 통째로 공유하려는 구조가 비효율적이다 |

Client/Bin/Resources/LostArk의 현재 주요 용량은 다음과 같다.

| 폴더 | 대략적 크기 | 통합 후 취급 |
|---|---:|---|
| SourceData | 4.255 GiB | runtime 및 팀 asset pack에서 제외 |
| Map | 1.599 GiB | 월드별 map pack으로 분리 |
| Character | 0.237 GiB | player, NPC, boss pack으로 분리 |
| Deploy | 0.058 GiB | 월드 deploy pack 또는 공용 prop pack |
| Effect_Tool | 0.008 GiB | authored data와 cooked asset으로 분리 |
| UI | 0.002 GiB | texture asset과 HUD layout data 분리 |

JSON만 공유하면 위치와 참조는 전달되지만 화면에 모델과 텍스처가 나오지 않는다. 따라서 Resources 전체 업로드를 없애는 해법은 바이너리를 버리는 것이 아니라 필요한 asset pack만 받게 만드는 것이다.

## 3. 목표 데이터 흐름

~~~mermaid
flowchart LR
    A["외부 추출 원본<br/>UModel·DB·placement JSON"] --> B["Data 정본<br/>Catalog·World·Spawn·Layer"]
    C["ImGui 명령<br/>선택·편집·저장"] --> B
    B --> D["검증기<br/>schema·ID·참조·좌표·dependency"]
    D --> E["Data Cooker"]
    E --> F["Client/Bin/DataFiles<br/>render·effect·UI·client navigation"]
    E --> G["Server/Bin/DataFiles<br/>spawn·gameplay·server navigation"]
    H["선택적 Asset Packs<br/>WModel·DDS·animation·audio"] --> I["Runtime Asset Resolver"]
    F --> I
    I --> J["Client CModel → CMaterial"]
    G --> K["Server Room<br/>플레이어·NPC·보스 권한"]
    K --> L["Network ID·archetype ID·transform·effect cue"]
    L --> J
~~~

이 흐름에서 중요한 금지 사항은 다음과 같다.

- Server는 WModel, DDS, Prototype tag, Renderer, ImGui를 알지 않는다.
- Data JSON은 C++ 포인터, vector index, Prototype tag를 저장하지 않는다.
- ImGui가 매 프레임 파일을 읽거나 모델을 다시 decode하지 않는다.
- Map, Deploy, NPC spawn, Effect anchor를 하나의 거대 placement 배열로 섞지 않는다.
- CCookedModel과 CBinaryAssetObject 레거시 경로를 신규 데이터 계약으로 확장하지 않는다.
- 실패한 load가 현재 활성 월드나 서버 Room 상태를 반쯤 바꾸지 않는다.

## 4. 정본 Data 폴더 구조

초기 목표 구조는 다음과 같다. 실제 파일은 해당 세로 조각을 구현할 때 추가한다.

~~~text
Data/
  Schemas/
    ProjectData.schema.json
    AssetPacks.schema.json
    CharacterCatalog.schema.json
    NpcCatalog.schema.json
    BossCatalog.schema.json
    EffectCatalog.schema.json
    World.schema.json
    PlacementLayer.schema.json
    PlayerSpawns.schema.json
    EntitySpawns.schema.json

  Catalogs/
    AssetPacks.json
    Characters.json
    Npcs.json
    Bosses.json
    Effects.json
    AnimationSets.json

  Worlds/
    LV_BER_BERNCASTLE/
      World.json
      GeneratedBase/
        BASE.json
        Landscape.json
        SL00.json
        ...
      Layers/
        world_dressing.json
        quest_props.json
        collision_review.json
      Gameplay/
        PlayerSpawns.json
        NpcSpawns.json
        EncounterSpawns.json
        Triggers.json
      Presentation/
        EffectAnchors.json
        CameraCues.json
      Navigation/
        NavManifest.json

    LV_LUT_HEARTRB_ED/
      World.json
      GeneratedBase/
      Layers/
      Gameplay/
      Presentation/
      Navigation/

  Receipts/
    LV_BER_BERNCASTLE.runtime-receipt.json
    LV_LUT_HEARTRB_ED.runtime-receipt.json
~~~

### 4.1 왜 하나의 JSON이 아닌가

한 파일에 map placement, NPC, player spawn, effect, trigger를 모두 넣으면 다음 문제가 생긴다.

- 팀원이 서로 다른 기능을 수정해도 같은 파일에서 충돌한다.
- base map 수만 건 때문에 사소한 spawn 변경도 거대한 diff가 된다.
- Server에 필요 없는 렌더 데이터까지 Server가 읽게 된다.
- 한 영역의 파싱 실패가 모든 데이터 갱신을 막는다.
- 권한과 검증 규칙을 문서 종류별로 나눌 수 없다.

World.json은 모든 내용을 직접 담는 파일이 아니라 해당 월드가 사용하는 문서와 asset pack을 연결하는 manifest 역할만 맡는다.

### 4.2 World.json의 책임

| 필드 | 의미 |
|---|---|
| schemaVersion | 문서 구조 버전 |
| dataVersion | 콘텐츠 배포 버전 |
| worldId | LV_BER_BERNCASTLE 같은 stable ID |
| coordinateContract | 좌표축, 단위, handedness 계약 |
| requiredAssetPacks | 이 월드를 볼 때 필요한 pack ID 목록 |
| renderDocuments | generated base와 editable layer 참조 |
| gameplayDocuments | player/NPC/encounter/trigger 문서 참조 |
| presentationDocuments | effect anchor와 camera cue 참조 |
| navigationDocuments | client/server navigation 문서 참조 |

문서 참조는 Data 루트 기준 상대 경로만 허용한다. 절대 경로와 상위 폴더 탈출은 validate 단계에서 거부한다.

## 5. ID와 저장 계약

### 5.1 안정 ID 종류

| ID | 소유 범위 | 예 | 사용처 |
|---|---|---|---|
| WorldId | 프로젝트 전체 | LV_BER_BERNCASTLE | Client, Server, Tool |
| AssetPackId | 프로젝트 전체 | map-bern-v1 | dependency 동기화 |
| AssetId | 프로젝트 전체 | MAP_BERN_WALL_001 | 렌더 에셋 정의 |
| CharacterClassId | 프로젝트 전체 | LANCE_MASTER | 플레이어 직업 |
| NpcArchetypeId | 프로젝트 전체 | NPC_101234 | NPC 게임/시각 연결 |
| BossArchetypeId | 프로젝트 전체 | BOSS_VALTAN | 보스 게임/시각 연결 |
| EffectAssetId | 프로젝트 전체 | EFFECT_VALTAN_CHARGE | effect cue 해석 |
| PlacementId | 월드 문서 전체 | editor:bern:GUID | 저장된 배치 인스턴스 |
| SpawnPointId | 월드 gameplay | player_spawn_north_01 | 서버 spawn 선택 |
| NetEntityId | 서버 Room runtime | 증가하는 정수 | 네트워크 생명주기 |

NetEntityId는 파일에 저장하지 않는다. AssetId와 archetype ID는 런타임 객체 주소가 아니며 Prototype tag로 대체하지 않는다.

### 5.2 placement ID 마이그레이션

현재 imported placement의 sourcePlacementId는 최대한 보존한다. 이미 로드되는 ID를 일괄 rename하지 않는다.

신규 ImGui 배치는 local 증가 숫자 대신 GUID 기반 문자열 PlacementId를 사용한다. runtime의 빠른 조회를 위해 숫자가 필요하면 cooker가 문자열 ID를 결정적으로 숫자 ID로 변환하고 충돌을 검사한다. 숫자는 runtime 파생물이며 사람이 편집하는 정본 ID가 아니다.

### 5.3 좌표 계약

- 외부 UE3 source: X,Y,Z, cm.
- Client canonical transform: X,Z,-Y, m.
- quaternion과 signed scale을 보존한다.
- source transform과 변환 규칙을 문서 metadata에 남긴다.
- Server gameplay transform도 Client canonical 좌표를 사용한다.
- source 좌표를 다시 임의 해석하지 않고 import 단계에서 한 번만 canonical로 바꾼다.

## 6. Git, LFS, asset pack 운용

### 6.1 Git에 둘 것

일반 Git:

- Data/Schemas 아래 schema.
- Data/Catalogs 아래 사람이 관리하는 catalog.
- World.json.
- Gameplay의 player, NPC, encounter spawn.
- Layers 아래 작은 editable overlay.
- Effect, camera, trigger의 사람이 관리하는 authoring data.
- runtime receipt와 migration report.

경로 한정 Git LFS:

- Data/Worlds/*/GeneratedBase 아래 대형 생성 JSON.
- 현재 Client/Bin/DataFiles/Map의 .mapassets, .mapplacements, .mapset runtime 산출물.
- 필요한 대형 nav grid와 바이너리 data.

.gitattributes에는 전역 *.json 규칙을 추가하지 않는다. 다음 의미의 경로 규칙만 추가한다.

~~~text
/Data/Worlds/**/GeneratedBase/**/*.json  → Git LFS
일반 Data/**/*.json                     → 일반 Git
~~~

### 6.2 asset pack에 둘 것

~~~text
SharedAssetRoot/
  Packs/
    map-bern-v1/
    map-valtan-v1/
    deploy-bern-v1/
    character-player-core-v1/
    npc-bern-v1/
    boss-valtan-v1/
    effect-common-v1/
~~~

각 pack은 manifest를 가진다.

| manifest 항목 | 목적 |
|---|---|
| packId, version | 선택 동기화와 호환성 확인 |
| contentHash | 팀원 간 동일 pack 검증 |
| relativeRoot | Runtime Asset Root 아래 실제 위치 |
| assets | stable AssetId와 상대 파일 경로 |
| dependencies | 다른 pack 필요 여부 |
| platform/build | Debug/Release가 아니라 콘텐츠 호환 정보 |

HeartRB에 이미 있는 map_asset_runtime_manifest.json은 폐기하기 전에 현재 hash와 asset 목록을 읽어 새 pack manifest로 승격한다.

### 6.3 올리지 않을 것

- LostArk SourceData 전체.
- UModel 원본 export와 중간 분석 dump 전부.
- Client/Bin/Resources 전체 복사본.
- 개인 absolute path.
- 중복된 WModel/DDS.
- 빌드 산출물, EngineSDK, .vs, imgui.ini.

외부 추출 원본은 재현용 source workspace에 남기고, 저장소에는 import spec, stable ID mapping, 생성 결과, receipt만 둔다.

## 7. Catalog 역할 분리

### 7.1 물리 Asset Catalog

물리 catalog는 AssetId를 실제 에셋으로 해석한다.

| 필드 | 예 |
|---|---|
| assetId | BOSS_VALTAN_BODY |
| kind | model, texture, animation, audio, effect |
| packId | boss-valtan-v1 |
| relativePath | Character/Boss/Valtan/Body.wmodel |
| contentHash | 변조 및 stale 검출 |
| dependencies | material texture, animation pack |
| importScale/anchor | visual 보정 metadata |

Client만 relativePath를 해석한다. Server는 물리 catalog를 로드하지 않는다.

현재 Client/Bin/Resources/LostArk/AssetCatalog.json은 구형 wmesh 경로이며 코드 reader도 확인되지 않았다. 새 정본으로 재사용하지 않고 사용처 0건을 다시 확인한 뒤 deprecated 목록으로 옮긴다.

### 7.2 의미 Catalog

| Catalog | 저장 내용 | 저장하지 않는 내용 |
|---|---|---|
| Characters.json | class ID, body/weapon/equipment AssetId, animation set ID, logic ID | C++ factory pointer, Prototype tag |
| Npcs.json | archetype ID, visual AssetId, idle/interaction profile | test level 좌표 |
| Bosses.json | boss ID, visual AssetId, gameplay profile ID, effect set ID | 서버 runtime state |
| Effects.json | EffectAssetId, authored/cooked 문서, dependency | particle runtime pointer |
| AnimationSets.json | logical clip ID와 asset mapping, notify/timing 문서 | 툴의 absolute path |

행동 factory와 함수 포인터는 계속 C++이 소유한다. JSON의 logic ID를 C++ registry가 해석한다. 데이터화한다는 이유로 함수를 JSON에 흉내 내지 않는다.

### 7.3 Spawn 문서

NPC 정의와 NPC 배치를 분리한다.

- Npcs.json은 무엇을 만들 수 있는지 정의한다.
- NpcSpawns.json은 어느 월드의 어디에 어떤 archetype을 놓는지 정의한다.
- Server는 spawn 문서를 읽어 NetEntityId를 발급하고 상태를 소유한다.
- Client는 network spawn의 archetype ID를 visual catalog로 해석한다.

플레이어도 같은 원칙이다.

- Characters.json은 class의 시각 구성을 정의한다.
- PlayerSpawns.json은 서버가 선택할 월드 spawn point를 정의한다.
- 실제 플레이어 위치는 서버 packet이 최종 권한이다.

## 8. ImGui 통합 설계

### 8.1 소유권

CMainApp이 다음 요소의 유일한 Debug ToolHost다.

- CImGuiLayer
- CMapTool
- CEffect_Tool
- CAnimation_Tool
- CHUDLayoutTool
- 앞으로 추가할 World Data 상태와 편집 session

새 editor executable이나 두 번째 scene runtime을 만들지 않는다. 기존 Prototype → Clone → Layer → CModel 흐름 위에 편집 명령만 얹는다.

### 8.2 UI와 저장 로직의 경계

현재 CMapTool은 UI, active placement 배열, 파일 저장 명령을 직접 많이 소유한다. 한 번에 갈아엎지 않고 다음 경계를 추가한다.

~~~text
ImGui Panel
  → Editor Command
  → WorldEditorSession
  → Document Model
  → Validator
  → Atomic Save Service
  → Dirty/Undo/Receipt 갱신
~~~

ImGui가 담당하는 것:

- 월드, layer, asset, placement, spawn 선택.
- create, move, delete, save, reload 명령 제출.
- dirty, validation error, pack missing, server reload 필요 상태 표시.
- base/generated와 editable layer의 구분 표시.

ImGui가 담당하지 않는 것:

- 매 frame JSON parse.
- WModel decode.
- 직접 fopen과 부분 쓰기.
- server Room 상태를 임의 변경.
- validation을 우회한 강제 commit.

### 8.3 Base와 Overlay 분리

대규모 Bern base shard는 import 결과이므로 read-only를 유지한다. 사람이 편집하는 변경은 Layers의 작은 문서에 저장한다.

| Layer 종류 | 편집 | Git 방식 |
|---|---:|---|
| GeneratedBase | import 재생성만 | LFS |
| World Dressing | 가능 | 일반 Git |
| Quest Props | 가능 | 일반 Git |
| Gameplay Spawn | 가능 | 일반 Git |
| Effect Anchors | 가능 | 일반 Git |
| Personal Scratch | 로컬 시험, commit 전 정식 layer로 이동 | Git 제외 가능 |

이 방식이면 Bern의 50,017건을 매번 수정하지 않고 배치 몇 건만 일반 Git diff로 review할 수 있다.

### 8.4 팀 저장

- 한 사람이 한 base shard를 직접 수정하지 않는다.
- 기능별 layerId와 owner를 문서에 기록한다.
- 신규 PlacementId는 GUID로 생성한다.
- Save는 temp 파일 작성 → flush → validate → replace 순서의 atomic save다.
- Reload는 parse → validate → stage → commit을 지킨다.
- 실패 시 현재 화면과 서버 상태는 이전 성공본을 유지한다.
- Git branch와 PR이 최종 협업 경계다.
- ImGui에는 현재 layer, dirty, source revision, 마지막 receipt hash를 표시한다.
- 이후 필요하면 layer ownership 경고를 추가하되 중앙 잠금 서버를 첫 단계에 만들지 않는다.

## 9. Client와 Server 권한

| 데이터/상태 | Client | Server | 정본 |
|---|---:|---:|---|
| map visual placement | 로드/표시 | 불필요 | Data + Client runtime |
| player spawn 후보 | 표시 가능 | 선택 | Data gameplay |
| player 실제 transform | 보간/표시 | 권한 | Server Room |
| NPC archetype 정의 | visual 부분 | gameplay 부분 | 공통 stable ID로 분할 |
| NPC 실제 transform/state | 표시 | 권한 | Server Room |
| boss phase/HP/position | 표시 | 권한 | Server Room |
| effect definition | 로드/재생 | 불필요 | Client catalog/pack |
| effect cue 발생 | 재생 | 권한 판단 | packet의 EffectAssetId |
| camera/HUD | 권한 | 불필요 | Client data |
| navigation visual/debug | 사용 | 불필요 | Client data |
| server collision/trigger pack | 불필요 | 사용 | Server data |

Server에서 Client, Engine renderer, ImGui, Resource path를 include하지 않는다. Shared에는 중립 ID, gameplay data 구조, packet 계약만 둔다.

## 10. 구현 순서

한 번에 모든 포맷을 바꾸지 않는다. 가장 얇은 online 세로 조각을 먼저 완성한 뒤 기존 기능을 옮긴다.

### 단계 0. 동결과 inventory

목표:

- 현재 active area, mapset, NPC pilot, character, effect, animation, HUD 파일을 목록화한다.
- 모든 기존 runtime 파일의 hash와 사용처를 receipt에 기록한다.
- 구형 AssetCatalog.json, BG_RAD legacy, 중복 CPlayer/CCharacter 경로는 삭제하지 않고 deprecated 후보로만 표시한다.
- SourceData는 pack 대상이 아님을 명시한다.

완료 기준:

- 어떤 파일이 정본, 생성물, 레거시, 미사용 후보인지 표가 있다.
- 무근거 삭제나 대량 이동이 없다.
- 현재 HeartRB와 Bern load가 그대로 된다.

### 단계 1. 첫 세로 조각: World manifest + 서버 Player Spawn

이 단계가 첫 구현 대상이다. JSON, Shared data, Server load, packet 결과, Debug 표시가 한 번에 연결되는 최소 기능이다.

#### 완료 후 사용자가 보게 될 상태

- Data/Worlds/LV_BER_BERNCASTLE/World.json이 Bern gameplay 문서를 가리킨다.
- PlayerSpawns.json에 최소 2개의 stable SpawnPointId와 transform이 있다.
- Server 시작 시 해당 문서를 한 번 parse하고 validate한다.
- CGameRoom::Join은 현재 players.size × 2 하드코딩 대신 검증된 spawn point를 선택한다.
- Client는 기존 spawn packet의 transform을 그대로 사용한다.
- 접속 직후 Server는 resource path가 없는 S2C_WORLD_INFO로 world ID, data version, gameplay hash, spawn count를 알린다.
- Debug ImGui에서 server world ID, data version, spawn count, source hash, 마지막 오류를 read-only로 볼 수 있다.
- 잘못된 JSON이면 Server는 world를 commit하지 않고 Room을 시작하지 않는다.

#### 신규 파일 책임

##### Shared/Public/Data/ProjectDataIds.h

- 존재 이유: Client와 Server가 같은 stable ID 타입을 사용해야 한다.
- 한 줄 역할: WorldId, SpawnPointId, archetype ID의 중립 별칭과 공통 검증 한계를 선언한다.
- 소유: 문자열 길이와 빈 값 금지 같은 중립 규칙.
- 비소유: 파일 경로, JSON parser, renderer, ImGui.
- 호출자: Shared data 구조, Server Room, 이후 Client replication.
- 실패: ID 생성이 아니라 validate 결과로 보고한다.

##### Shared/Public/Data/WorldGameplayData.h

- 존재 이유: Server가 JSON 라이브러리 타입에 의존하지 않는 immutable gameplay snapshot이 필요하다.
- 한 줄 역할: WorldId, dataVersion, player spawn 목록을 담는 중립 구조를 선언한다.
- 소유: canonical transform과 spawn selection에 필요한 값.
- 비소유: WModel 경로, effect 파일, UI 표시 상태.
- 입력: 검증을 끝낸 parser 결과.
- 출력: ServerApp에서 GameRoom으로 전달할 읽기 전용 snapshot.

##### Shared/Public/Data/WorldGameplayDocument.h

- 존재 이유: JSON parsing과 domain validation 입구를 하나로 고정해야 한다.
- 한 줄 역할: 파일 또는 문자열을 parse해 staged WorldGameplayData를 만들고 진단을 반환한다.
- 공개 계약: JSON library 타입을 header에 노출하지 않는다.
- 성공: 완전히 검증된 snapshot을 반환한다.
- 실패: field path가 포함된 진단을 반환하고 출력 snapshot을 바꾸지 않는다.

##### Shared/Private/Data/WorldGameplayDocument.cpp

- 존재 이유: third-party JSON dependency와 schema migration을 private 구현에 가둔다.
- 한 줄 역할: parse → schema version 확인 → field validate → cross-reference validate → stage를 수행한다.
- 의존: vendored nlohmann/json 또는 동등한 단일 JSON parser.
- 금지: ServerApp, CGameRoom, Client class include.

##### Shared/External/nlohmann

- 존재 이유: 현재 저장소에 C++ JSON parser가 없고 Server와 ImGui가 같은 정본 문서를 읽어야 한다.
- 사용 범위: Shared Private parser에만 노출한다.
- 프로젝트 조치: 라이선스를 함께 보관하고 public header에는 JSON 타입을 노출하지 않는다.
- 주의: 전체 프로젝트의 임의 JSON 접근을 허용하는 전역 편의 include로 만들지 않는다.

##### Server/Public/ServerDataRoot.h, Server/Private/ServerDataRoot.cpp

- 존재 이유: 실행 위치가 바뀌어도 Server/Bin/DataFiles를 한 규칙으로 찾아야 한다.
- 한 줄 역할: 실행 파일 기준 Server runtime data root를 resolve하고 root 탈출을 막는다.
- 소유: server data root와 상대 경로 결합.
- 비소유: JSON 의미 검증, Room 상태.
- 실패: missing root와 invalid relative path를 구분해 진단한다.

##### Server/Public/WorldBootstrap.h, Server/Private/WorldBootstrap.cpp

- 존재 이유: 파일 load 성공 여부와 Room 생성 시점을 분리해야 한다.
- 한 줄 역할: world gameplay 문서를 load하고 검증 성공 시 immutable snapshot을 ServerApp에 commit한다.
- 호출자: ServerApp 시작 경로.
- 피호출자: ServerDataRoot, WorldGameplayDocument.
- 실패: 기존 active snapshot을 바꾸지 않고 서버 시작 오류를 반환한다.
- 금지: global mutable singleton.

##### Server/Public/ServerApp.h, Server/Private/ServerApp.cpp 수정

- 존재 이유: 현재 server startup과 listener 시작 사이에 world bootstrap gate가 없다.
- 변경 역할: WorldBootstrap을 소유하고 active gameplay snapshot 준비가 끝난 뒤 GameRoom과 listener를 생성한다.
- 소유: startup 순서와 active world lifetime.
- 비소유: JSON field parsing, spawn 선택 정책.
- 실패: 초기화 실패를 Main에 반환하고 listener를 열지 않는다.

##### Server/Public/GameRoom.h, Server/Private/GameRoom.cpp 수정

- 존재 이유: 현재 Join의 players.size × 2 spawn을 제거해야 한다.
- 변경 역할: 생성 시 immutable WorldGameplayData를 받고 Join에서 SpawnPointId를 선택한다.
- 소유: player와 NetEntityId, 현재 spawn 점유/선택의 runtime 상태.
- 비소유: 파일 load와 asset path.
- 실패: 유효 spawn 후보가 없으면 player를 Room map에 넣기 전에 Join을 거부한다.

##### Shared/Public/Network/PacketType.h, PacketMessages.h와 Shared/Private/Network/PacketMessages.cpp 수정

- 존재 이유: Client가 server의 active world와 data version을 추측하지 않고 확인해야 한다.
- 변경 역할: S2C_WORLD_INFO message와 serialization을 추가한다.
- 필드: WorldId, dataVersion, gameplay source hash, enabled player spawn count.
- 금지: Data 절대 경로, WModel 경로, JSON 원문.
- 기존 S2C_PLAYER_SPAWNED transform 계약은 바꾸지 않는다.

##### Client/Public/NetworkManager.h, Client/Private/NetworkManager.cpp 수정

- 존재 이유: network thread의 world info를 ImGui가 안전하게 읽을 snapshot으로 전달해야 한다.
- 변경 역할: S2C_WORLD_INFO를 decode하고 main-thread 적용 queue를 통해 read-only status를 갱신한다.
- 소유: 연결 세션 동안 마지막으로 확인된 server world summary.
- 비소유: gameplay spawn 목록과 Server Room state.
- 실패: malformed packet을 폐기하고 기존 status를 유지한다.

##### Client/Public/WorldDataStatusPanel.h, Client/Private/WorldDataStatusPanel.cpp

- 존재 이유: 첫 slice부터 실제로 어느 world data가 활성화됐는지 Debug에서 확인할 창구가 필요하다.
- 한 줄 역할: world ID, data version, spawn count, source hash, pack/data 오류를 read-only로 표시한다.
- 입력: NetworkManager가 main thread에 공개한 server world summary와 현재 Client level ID.
- 비소유: JSON parse, save, Server reload, scene object 생성.
- 주의: 별도 ImGui context를 만들지 않고 CMainApp의 기존 CImGuiLayer에서 그린다.

##### Client/Public/MainApp.h, Client/Private/MainApp.cpp 수정

- 존재 이유: CMainApp을 유일한 Debug ToolHost로 유지하면서 새 status panel의 생명주기를 명시해야 한다.
- 변경 역할: WorldDataStatusPanel을 생성·tick·render·해제한다.
- 주의: 현재 MapTool open boolean으로 Effect/Animation/HUD까지 함께 닫히는 결합을 새 panel에 복제하지 않는다.
- 비소유: panel의 표시 데이터와 server world snapshot.

##### Client/Public/Level_Baren.h, Client/Private/Level_Baren.cpp 수정

- 존재 이유: 현재 CClientReplication 구현이 있어도 Bern level 생명주기에 완전히 연결되지 않았다.
- 변경 역할: Bern 진입 시 replication owner를 준비하고 퇴장 시 registry와 network object를 정리한다.
- 소유: 해당 level에서 보이는 replicated object 생명주기.
- 비소유: server spawn 선택과 data 파일 저장.
- 실패: 중간 단계 객체를 rollback하고 이미 활성인 static map은 훼손하지 않는다.

##### Data/Worlds/LV_BER_BERNCASTLE/World.json

- 존재 이유: Bern을 구성하는 runtime 문서의 manifest 정본이 필요하다.
- 첫 단계 범위: gameplay의 PlayerSpawns 참조와 world metadata만 활성화한다.
- 이후 확장: render, NPC, encounter, effect, navigation 참조.

##### Data/Worlds/LV_BER_BERNCASTLE/Gameplay/PlayerSpawns.json

- 존재 이유: 서버 spawn 하드코딩을 데이터로 이동한다.
- 소유: SpawnPointId, position, yaw, enabled, optional tags/priority.
- 비소유: 접속한 player ID, NetEntityId, 현재 점유 상태.
- 편집: 일반 Git, 향후 ImGui command를 통한 atomic save.

##### Tools/DataPipeline/validate_project_data.py

- 존재 이유: PR과 배포 전에 C++ 실행 없이 schema와 cross-reference 오류를 잡는다.
- 한 줄 역할: Data 전체 또는 선택 world를 검증하고 사람이 읽는 오류와 machine-readable report를 만든다.
- 주의: Python validator와 C++ parser의 규칙 목록을 테스트 fixture로 맞춘다.

##### Tools/DataPipeline/build_runtime_data.py

- 존재 이유: 정본 Data와 Bin/DataFiles의 경계를 자동화해야 한다.
- 첫 단계 역할: World와 PlayerSpawns를 Server/Bin/DataFiles의 정해진 위치로 복사·정규화하고 receipt를 만든다.
- 이후 역할: map runtime cook, client/server 분할, catalog index 생성.
- 실패: 부분 배포 폴더를 활성 위치로 바꾸지 않는다.

#### H 계약

WorldGameplayDocument:

- LoadFromFile은 staged output과 diagnostics를 받는다.
- schemaVersion이 지원 범위를 벗어나면 명시적으로 실패한다.
- worldId 불일치, 중복 SpawnPointId, 비정상 숫자, 비활성 spawn만 존재하는 경우 실패한다.
- 성공하기 전 caller가 보유한 active snapshot은 변하지 않는다.
- JSON parser 구현 타입을 public signature에 사용하지 않는다.

WorldBootstrap:

- Prepare는 root resolve와 parse/validate만 수행한다.
- Commit은 Prepare 성공 결과에만 허용한다.
- ServerApp은 Commit 성공 전 listener와 Room gameplay를 활성화하지 않는다.
- reload를 추가할 때도 새 snapshot을 먼저 완성하고 tick 경계에서 교체한다.

CGameRoom:

- 생성 시 검증된 WorldGameplayData를 명시적으로 받는다.
- Join은 enabled spawn 후보 중 결정적 정책으로 하나를 선택한다.
- 첫 정책은 SpawnPointId 정렬 후 현재 player 수에 따른 round-robin으로 충분하다.
- packet에는 선택된 실제 position과 yaw를 기록한다.
- 문서에 없는 fallback 원점 spawn을 조용히 만들지 않는다.

#### CPP 흐름

Server 시작:

~~~text
Main
  → ServerApp::Initialize
  → ServerDataRoot::Resolve
  → WorldBootstrap::Prepare
  → WorldGameplayDocument::LoadFromFile
  → parse
  → validate schema/ID/transform/reference
  → stage immutable WorldGameplayData
  → WorldBootstrap::Commit
  → GameRoom 생성
  → listener 시작
~~~

Client 접속 상태 확인:

~~~text
Server session ready
  → active WorldGameplayData 요약
  → S2C_WORLD_INFO
  → NetworkManager decode
  → main-thread status snapshot 교체
  → WorldDataStatusPanel read-only 표시
  → Client level ID 불일치 시 경고
~~~

Player join:

~~~text
ClientSession join command
  → GameRoom::Join
  → PlayerSpawns에서 후보 선택
  → SERVER_PLAYER 생성
  → NetEntityId 할당
  → Room map commit
  → S2C_PLAYER_SPAWNED 전송
  → Client replication이 packet transform으로 CCharacter 생성
~~~

실패:

~~~text
parse 또는 validate 실패
  → diagnostics 작성
  → staged snapshot 폐기
  → GameRoom과 listener 미시작
  → 이전 성공 runtime 파일과 active snapshot 보존
~~~

#### 의존 방향

허용:

~~~text
Shared Data ← Server
Shared Network ← Server
Shared Data ← Client
DataPipeline → Data와 Bin/DataFiles
~~~

금지:

~~~text
Shared → Server
Shared → Client
Server → Client
Server → Engine Renderer
Server → ImGui
Shared public header → nlohmann JSON type
~~~

#### 프로젝트 등록

새 C++ 파일은 물리 폴더를 기준으로 다음 프로젝트에 필요한 항목만 등록한다.

- Shared/Default/Shared.vcxproj
- Shared/Default/Shared.vcxproj.filters
- Server/Default/Server.vcxproj
- Server/Default/Server.vcxproj.filters
- Client C++ 파일을 추가하는 slice에서는 Client/Default/Client.vcxproj와 filters

기존 filters를 재배치하지 않는다. Shared의 third-party include 경로는 Shared project private compile 설정에만 추가한다. Data JSON은 Client filter에 억지로 넣지 않고 물리 Data 폴더를 정본으로 둔다.

#### 실제 작성 순서

사용자가 처음 작성할 순서는 다음과 같다.

1. Bern World.json과 PlayerSpawns.json, 성공/실패 fixture를 먼저 만든다.
2. ProjectDataIds와 WorldGameplayData로 JSON과 무관한 domain 계약을 고정한다.
3. WorldGameplayDocument parser와 validator를 구현한다.
4. Python validator가 같은 fixture를 통과하고 실패하게 만든다.
5. ServerDataRoot와 WorldBootstrap을 연결한다.
6. ServerApp startup gate를 추가한다.
7. S2C_WORLD_INFO와 Client status snapshot을 연결한다.
8. GameRoom Join의 하드코딩 spawn을 교체한다.
9. Bern level에 CClientReplication 생명주기를 연결한다.
10. WorldDataStatusPanel을 기존 CMainApp ImGui host에 붙인다.
11. 마지막으로 runtime copy와 receipt 생성을 자동화한다.

이 순서는 UI부터 만들어 저장 계약이 다시 흔들리는 것을 막는다. 첫 성공 지점은 ImGui 버튼이 아니라 Server가 JSON spawn을 선택해 두 Client가 같은 결과를 보는 시점이다.

#### 주요 중단점

| 위치 | 확인 값 |
|---|---|
| WorldGameplayDocument parse 직후 | schemaVersion, worldId |
| spawn validate loop | SpawnPointId, position의 finite 여부, duplicate set |
| WorldBootstrap commit 직전 | staged spawn count, source hash |
| ServerApp listener 시작 직전 | active world ID |
| GameRoom::Join | 선택된 SpawnPointId, player count |
| spawn packet 작성 | NetEntityId, class ID, position, yaw |
| S2C_WORLD_INFO 적용 | world ID, data version, source hash |
| Client spawn 처리 | packet transform, 생성된 CCharacter 등록 결과 |

#### 빌드와 실행 검증

1. Shared x64 Debug와 Release.
2. Server x64 Debug와 Release.
3. Engine x64 Debug와 Release.
4. UpdateLib.bat Debug와 Release.
5. Client x64 Debug와 Release.
6. Data validator로 Bern world 성공 확인.
7. Server 1개와 Client 2개를 실행해 두 player가 JSON spawn 위치에 생성되는지 확인.
8. ImGui status의 world ID, data version, hash가 Server startup log와 같은지 확인.
9. Client가 다른 level에 있을 때 status panel이 world mismatch를 경고하는지 확인.
10. duplicate SpawnPointId fixture가 배포 전에 실패하는지 확인.
11. JSON을 의도적으로 손상했을 때 Server가 Room을 시작하지 않고 정확한 field path를 출력하는지 확인.
12. Server 배포 폴더에서 Client Resources를 제거해도 Server가 시작되는지 확인.
13. Client.exe가 link output을 점유하면 종료 후 다시 link.

### 단계 2. Asset pack과 Character/NPC/Boss catalog

목표:

- AssetPacks.json과 pack manifest를 도입한다.
- Loader의 하드코딩된 모델 경로를 catalog resolver로 이동한다.
- Character visual spec, NPC archetype, Boss archetype을 data-driven으로 만든다.
- 행동 factory는 logic ID 기반 C++ registry로 유지한다.
- 발탄은 BOSS_VALTAN archetype과 boss-valtan pack으로 정의한다.

핵심 순서:

1. 현재 WModel과 texture dependency를 pack 단위로 inventory한다.
2. asset ID와 content hash를 확정한다.
3. Client resolver가 pack missing을 명확히 보고하게 한다.
4. Loader는 catalog를 읽어 Prototype을 기존 경로에 등록한다.
5. Level_Test2 하드코딩 NPC 위치를 NpcSpawns overlay로 옮긴다.
6. pilot NPC 4개의 데이터 이관과 시각 품질 검증을 분리한다.

완료 기준:

- 신규 NPC/보스 배치 JSON에 모델 상대 경로가 없다.
- Server에 boss WModel 경로가 없다.
- 필요한 pack만 동기화한 PC에서 해당 월드가 보인다.
- pack이 없으면 placeholder 또는 명시적 load failure가 나오며 crash하지 않는다.
- Beda 외 3개 NPC의 retarget 문제는 별도 asset-quality gate로 추적한다.

### 단계 3. World Tool layer 편집과 transaction

목표:

- CMapTool을 유지하면서 WorldEditorSession과 command 경계를 추가한다.
- generated base는 read-only, overlay는 편집 가능하게 한다.
- placement, player spawn, NPC spawn, effect anchor를 각 문서 타입에 저장한다.
- dirty, undo/redo, validate, atomic save, reload를 공통화한다.

완료 기준:

- Bern base 50,017건을 수정하지 않고 overlay 1건을 저장할 수 있다.
- 두 브랜치에서 서로 다른 layer를 편집하면 대형 LFS 충돌이 없다.
- 같은 layer의 충돌은 일반 JSON diff로 review할 수 있다.
- 저장 실패 후 기존 scene과 문서가 유지된다.
- ImGui를 닫아도 다른 tool이 MapTool open boolean에 잘못 종속되지 않는다.

### 단계 4. 서버 권한 NPC와 발탄

목표:

- Server가 NpcSpawns와 EncounterSpawns를 load한다.
- Room이 NetEntityId, archetype ID, transform, HP/state를 소유한다.
- Client는 network archetype ID를 visual catalog로 해석한다.
- 발탄 encounter의 spawn, phase, despawn, effect cue를 서버 이벤트로 연결한다.

packet 원칙:

- spawn: NetEntityId, archetype ID, transform, 필요한 gameplay summary.
- state: NetEntityId와 최소 상태 delta.
- effect: stable EffectAssetId와 transform/target 정보.
- 금지: WModel 경로, texture 경로, C++ pointer, Prototype tag.

완료 기준:

- Client 단독 코드가 NPC/발탄 실제 위치를 최종 결정하지 않는다.
- 중간 접속한 Client가 Room snapshot으로 동일 상태를 복원한다.
- despawn 후 registry와 scene object가 함께 정리된다.
- Bern level이 CClientReplication 생명주기를 명시적으로 소유한다.

### 단계 5. Effect, Animation, HUD, Camera 통합

Effect:

- 코드와 실제 폴더의 Effect/Effect_Tool 경로 불일치를 먼저 해소한다.
- authored effect는 Data의 정본 문서로, cooked .weffect와 texture는 effect pack으로 나눈다.
- 현재 custom .effect를 즉시 폐기하지 않는다. JSON wrapper/catalog로 연결한 뒤 converter 필요성을 판단한다.
- CEffect_Runtime의 신규 로드는 CModel → CMaterial 통합 경로만 확장한다.

Animation:

- 현재 DataFiles/Anim의 .animevents, .animnotify, .clipmap, .clipseq, .skilltiming을 AnimationSets catalog로 index한다.
- CAnimation_Tool이 파일 이름을 직접 조합하지 않고 선택된 animation set 문서를 사용한다.

HUD:

- Resources/UI/HUD/HUD_Layout.cfg를 Data의 client presentation 문서로 이동한다.
- texture는 UI asset pack에 남긴다.

Camera:

- 월드 camera cue와 editor camera preset을 분리한다.
- gameplay camera cue는 stable cue ID를 사용하고 개인 editor view는 commit 대상에서 제외한다.

완료 기준:

- Resources 아래에 사람이 저장하는 HUD/layout 파일이 없다.
- Effect와 Animation tool이 공통 data root와 save transaction을 사용한다.
- Effect runtime cue가 파일 경로가 아니라 EffectAssetId로 재생된다.

### 단계 6. 레거시 정리

정리 후보:

- 사용처 없는 구형 AssetCatalog.json.
- BG_RAD와 기타 legacy map runtime.
- Level_Test2의 NPC 위치 하드코딩.
- Loader의 모델 상대 경로 하드코딩.
- CGameRoom의 players.size × 2 spawn.
- Effect_Tool 경로 중복.
- Resources 아래 HUD authoring data.
- 중복 CPlayer와 CCharacter 경로.
- CCookedModel과 CBinaryAssetObject에 남은 신규 기능 오용.

삭제 조건:

1. 새 경로가 Debug/Release와 runtime test를 통과한다.
2. rg와 project reference로 소비자가 0건임을 확인한다.
3. 기존 파일 hash와 migration receipt가 남아 있다.
4. 팀 PR에서 삭제 범위를 review한다.

CPlayer와 CCharacter는 즉시 하나를 지우지 않는다. network와 level consumer를 CCharacter로 옮기고 입력, 장비, 애니메이션 기능 차이를 비교한 후 최종 통합 계획을 별도로 작성한다.

## 11. 데이터 검증 규칙

모든 문서가 공통으로 지켜야 할 규칙:

- 지원 schemaVersion.
- non-empty stable ID.
- 문서 범위 안의 ID 중복 금지.
- 상대 경로만 허용하고 root 탈출 금지.
- float는 finite 값만 허용.
- quaternion 정규화 가능 범위 확인.
- scale 0 금지, signed scale 보존.
- catalog와 placement/spawn cross-reference 확인.
- required pack 존재 및 manifest hash 확인.
- Client 전용 필드가 Server output에 들어가지 않는지 확인.
- Server 필수 gameplay 문서가 누락되지 않았는지 확인.
- source hash와 runtime receipt의 stale 여부 확인.

load 공통 상태:

~~~text
Unloaded
  → Parsing
  → Validating
  → Staged
  → Committed

어느 단계에서든 실패
  → diagnostics
  → staged 객체 rollback
  → 이전 Committed 상태 보존
~~~

## 12. 마이그레이션 매핑

| 현재 위치/구조 | 목표 정본 | 목표 runtime/pack |
|---|---|---|
| 외부 Specs/AreaId.json | Data/Worlds/AreaId/World.json | Client/Server runtime manifest |
| 외부 placement source JSON | GeneratedBase 또는 import workspace + receipt | .mapplacements shard |
| Client/Bin/DataFiles/Map | Data에서 생성되는 파생물 | 현 위치 유지 후 cooker 소유 |
| MapTool editor placement | Data/Worlds/Area/Layers/*.json | overlay runtime placement |
| Level_Test2 NPC 위치 | Gameplay/NpcSpawns.json | Server gameplay data |
| Loader NPC/character 경로 | Catalogs와 AssetPacks | 선택적 asset pack |
| CGameRoom spawn 계산 | Gameplay/PlayerSpawns.json | Server/Bin/DataFiles |
| Effect_Tool authored file | Data/Effects 및 Effects catalog | cooked effect pack |
| DataFiles/Anim 개별 파일 | AnimationSets catalog가 참조 | Client/Bin/DataFiles/Anim |
| Resources UI HUD cfg | Data client presentation | Client/Bin/DataFiles/UI |
| 구형 AssetCatalog.json | 새 catalog로 수동 검증 이관 | 사용처 0 확인 후 제거 |

초기 migration은 move가 아니라 copy → validate → consumer 전환 → 사용처 0 확인 → 제거 순서로 한다. 기존 팀원의 미커밋 파일과 무관한 리소스를 정리하지 않는다.

## 13. 작업 분할과 PR 경계

권장 PR 단위:

1. Data root, schema, validator, Bern World/PlayerSpawns fixture.
2. Shared JSON parser와 immutable WorldGameplayData.
3. Server bootstrap과 data-driven player spawn.
4. Client Debug world data status와 Bern replication owner 연결.
5. Asset pack manifest와 resolver.
6. Character/NPC/Boss catalog 및 Loader migration.
7. WorldEditorSession, overlay placement save.
8. Spawn/Effect editor command와 atomic save.
9. Server NPC/Valtan replication.
10. Effect/Animation/HUD 경로 통합.
11. 검증을 끝낸 legacy 제거.

각 PR은 unrelated formatting, filters 재배치, 대량 resource 이동을 포함하지 않는다.

## 14. 전체 완료 조건

다음이 모두 성립해야 통합 완료로 본다.

- 새 팀원이 저장소와 필요한 asset pack만 받아 Bern 또는 Valtan 목표 월드를 실행할 수 있다.
- SourceData와 Resources 전체를 받을 필요가 없다.
- Map base, editable layer, player spawn, NPC spawn, effect anchor가 서로 다른 정본 문서로 관리된다.
- 사람이 수정하는 JSON은 일반 Git diff와 review가 가능하다.
- 대형 generated base만 LFS를 사용한다.
- Client와 Server가 같은 stable ID 및 canonical 좌표 계약을 사용한다.
- Server가 player, NPC, boss의 실제 transform과 state를 소유한다.
- Client packet에는 resource path 대신 stable archetype/effect ID가 온다.
- ImGui는 command, dirty, validation, save/reload 상태를 제공하고 매 frame 파일을 읽지 않는다.
- save와 reload가 parse → validate → stage → commit, 실패 rollback을 지킨다.
- Loader, Level_Test2, GameRoom의 핵심 하드코딩이 catalog/spawn 문서로 이동한다.
- Effect, Animation, HUD가 공통 data root 규칙을 따른다.
- CModel → CMaterial 통합 경로가 신규 asset의 유일한 runtime 경로다.
- Debug/Release build와 두 Client 접속, 저장, 재로드, 손상 파일 실패 보존까지 검증된다.

## 15. 바로 시작할 범위

첫 구현에서는 욕심내서 NPC, 발탄, Effect editor까지 동시에 바꾸지 않는다. 다음 범위만 먼저 끝낸다.

1. Data 폴더와 Bern World/PlayerSpawns 정본 추가.
2. Shared private JSON parser와 WorldGameplayData 계약 추가.
3. Data validator와 Server runtime 배포 receipt 추가.
4. Server bootstrap 추가.
5. CGameRoom player spawn 하드코딩 제거.
6. Client 두 개 접속으로 서버 권한 spawn 검증.
7. Debug ImGui에는 read-only world data 상태만 표시.

이 세로 조각이 성공하면 같은 계약을 NPC spawn, 발탄 encounter, asset pack, effect cue에 반복 적용한다. 이 순서가 현재 맵 작업을 깨뜨리지 않으면서 데이터와 서버를 실제로 한 줄로 연결하는 가장 작은 출발점이다.

## 16. 에셋 데이터와 툴 범용화의 핵심 그림

이 절은 Data, binary asset, runtime object, ImGui Tool의 관계를 한 번에 이해하기 위한 기준이다.

가장 먼저 다섯 가지를 서로 다른 것으로 본다.

| 개념 | 질문 | 예 |
|---|---|---|
| Asset Definition | 무엇을 로드할 수 있는가 | BOSS_VALTAN_BODY → MN_RPBF_01.wmodel |
| Binary Asset | 실제 형상과 pose 데이터는 무엇인가 | WModel, DDS, WAV |
| Authoring Document | 사람이 무엇을 편집하는가 | animation event, effect, placement, HUD |
| Runtime Document | 실행기가 무엇을 빠르게 읽는가 | mapplacements, animevents, weffect, navgrid |
| Runtime Instance | 지금 화면과 서버에 존재하는 것은 무엇인가 | CValtan clone, CCharacter clone, map object |

이 다섯 종류를 한 폴더나 한 JSON으로 합치지 않는다.

~~~mermaid
flowchart LR
    A["Asset Definition<br/>stable AssetId·packId·relative path"] --> B["Binary Asset Pack<br/>WModel·DDS·WAV"]
    B --> C["CModel → CMesh·CMaterial·CBone·CAnimation"]
    C --> D["Prototype → Clone → Layer"]
    D --> E["Runtime Instance<br/>Player·NPC·Valtan·PreviewActor"]

    F["Authoring Data<br/>JSON·effect·event·placement·HUD"] --> G["Validator·Cooker"]
    G --> H["Client Runtime Data<br/>DataFiles"]
    G --> I["Server Runtime Data<br/>DataFiles"]

    J["ImGui Tool"] --> K["Document Session<br/>draft·dirty·undo"]
    J --> L["Editor Target Registry<br/>선택된 runtime instance"]
    K --> F
    L --> E
    H --> E
    I --> M["Server Room"]
~~~

툴이 binary를 직접 수정하는 것이 아니다.

- Animation Tool은 WModel 안의 clip을 재생하지만 hit event는 Data 문서에 저장한다.
- Effect Tool은 authored effect 문서를 편집하고 cooker가 .weffect를 만든다.
- Map Tool은 WModel을 움직이는 것이 아니라 AssetId와 Transform을 placement 문서에 저장한다.
- HUD Tool은 texture를 수정하지 않고 texture AssetId와 layout을 저장한다.
- NPC Tool은 NPC WModel을 수정하지 않고 NpcArchetypeId와 spawn을 저장한다.

## 17. 현재 물리 파일 전수 분류

### 17.1 Client/Bin/DataFiles

현재 DataFiles에는 세 성격이 섞여 있다.

| 영역 | 현재 파일 | 개수/크기 | 실제 성격 |
|---|---|---:|---|
| Anim | animevents, animnotify, clipmap, clipseq, skilltiming | 20개, 약 2.34 MB | authored와 imported reference 혼재 |
| Map | maparea, mapset, mapassets, mapplacements, deploy 문서, receipt | 35개, 약 17.19 MB | 대부분 generated runtime |
| Navigation | navsource, navpaint, navgrid, navblockers | 4개, 약 87 KB | authoring과 runtime 혼재 |
| Legacy Navigation | Navigation.dat, NavigationNeighbors.dat, TerrainNavigation.dat | 3개, 약 1.50 MB | 레거시 runtime |

파일별 목표는 다음과 같다.

| 확장자 | 현재 역할 | 목표 위치와 역할 |
|---|---|---|
| .animevents | Animation Tool authored event | Data/Animation/Authored의 정본에서 생성되는 runtime 문서 |
| .animnotify | 원본 Action notify 추출 | Data/Animation/Imported의 reference |
| .clipmap | clip과 skill 이름 연결 | Data/Animation/Imported의 reference |
| .clipseq | skill clip chain 추출 | Data/Animation/Imported의 reference, 필요한 부분은 runtime cook |
| .skilltiming | 추출 combat timing | Data/Animation/Imported의 reference |
| .mapassets | AssetId와 WModel runtime catalog | Data catalog/base에서 생성되는 Client runtime |
| .mapplacements | runtime placement | generated base와 editable layer를 합친 Client runtime |
| .mapset | shard manifest | World.json에서 생성되는 Client runtime |
| .deployassets/.deployplacements | deploy visual runtime | 별도 Deploy layer에서 생성되는 Client runtime |
| .navsource | bake source | Data/Worlds의 authoring |
| .navpaint | 사람의 walkable 수정 | Data/Worlds의 authoring |
| .navgrid | 탐색 runtime | Client/Server runtime |
| .navblockers | runtime condition blocker | Data gameplay에서 생성되는 runtime |
| .dat | 구형 navigation | 새 navgrid 소비자 전환 후 제거 후보 |

Bin/DataFiles는 최종적으로 read-only 배포 폴더가 된다. Debug Tool도 이 폴더를 정본처럼 직접 수정하지 않는다.

### 17.2 Client/Bin/Resources/LostArk

SourceData를 제외한 현재 주요 확장자 실측:

| 확장자 | 개수 | 대략적 크기 | 목표 |
|---|---:|---:|---|
| .wmodel | 722 | 266.57 MiB | runtime model asset pack |
| .dds | 2,131 | 1,499.15 MiB | runtime texture asset pack |
| .png | 198 | 60.91 MiB | runtime 또는 authoring preview, 용도 표시 필요 |
| .tga | 72 | 85.24 MiB | runtime texture asset pack |
| .wanim | 55 | 19.58 MiB | WModel build input, runtime 중복 배포 금지 |
| .wmesh | 22 | 10.25 MiB | WModel build input |
| .wmat | 22 | 약 0.04 MiB | WModel build input |
| .wskel | 3 | 약 0.05 MiB | WModel build input |
| .cfg | 4 | 약 0.03 MiB | Resources에서 Data로 이동 |
| .csv/.txt | 검색 catalog와 UModel list | 약 7.9 MiB | tool import index 또는 local cache |
| .json | 구형 catalog, intake manifest, runtime manifest | 소량 | 역할별 Data, pack manifest, receipt로 분리 |

현재 Resources가 난잡해 보이는 이유는 실행에 필요한 파일과 제작 중간 파일이 같은 위치에 있기 때문이다.

목표 규칙:

- runtime pack에는 .wmodel과 실제 texture/audio만 둔다.
- .wmesh, .wmat, .wskel, .wanim은 AssetBuild 입력 또는 외부 build workspace로 옮긴다.
- HUD cfg, authored effect, source catalog CSV는 Resources에 두지 않는다.
- SourceData와 UModel 대형 export는 project runtime root에 두지 않는다.

## 18. WModel binary의 정확한 역할

### 18.1 생성 과정

~~~text
외부 원본
  FBX / glTF / UModel export
  또는 WMesh + WMaterial + WSkeleton + WAnimation

        ↓ ModelAssetConverter

배포 모델
  MyAsset.wmodel
  textures/*.dds 또는 png/tga
~~~

WModel은 여러 binary section을 한 파일로 묶는 container다.

~~~mermaid
flowchart TB
    W["WMOD package header<br/>section count·animation count"]
    W --> M["MESH section<br/>submesh·vertex·index·bone weight·bounds"]
    W --> T["MATERIAL section<br/>material index·texture 상대 경로"]
    W --> S["SKELETON section<br/>bone hierarchy·rest transform·socket"]
    W --> A1["ANIMATION section 0<br/>clip name·duration·TPS·keys·event"]
    W --> A2["ANIMATION section 1"]
    W --> AN["ANIMATION section N"]
    T --> X["외부 texture 파일<br/>DDS·PNG·TGA"]
~~~

### 18.2 section별 내용

| Section | 포함 | 포함하지 않음 |
|---|---|---|
| Mesh | 정점, 인덱스, submesh, material index, bone weight/index, bounds | placement transform |
| Material WMA2 | diffuse/base, normal, specular, emissive, opacity, ORM 등 상대 경로 | texture pixel 자체 |
| Skeleton | bone name/hash, parent, rest transform, global inverse root, socket | gameplay state |
| Animation | clip name, duration tick, TPS, loop, bone channel key, 선택적 embedded event | boss pattern, damage tuning |

현재 CModel 경로:

~~~text
AssetId
  → Asset Catalog의 packId + WModel 상대 경로
  → CRuntimeAssetRoot
  → CModel::Create
  → CWModelDecoder
  → MODEL_ASSET_DATA
  → CMesh + CMaterial + CBone + CAnimation
  → Prototype 등록
  → Clone마다 독립 animation time
~~~

이 경로가 신규 모델의 유일한 경로다. CCookedModel과 CBinaryAssetObject는 이 구조의 두 번째 정식 runtime이 아니다.

### 18.3 WModel과 animation event의 경계

WModel의 WANM section에는 event 구조를 담을 자리가 있지만, 현재 발탄의 공격 clip 18개는 eventCount가 전부 0이다.

따라서 다음을 구분한다.

- WModel clip key: 뼈가 어떻게 움직이는가.
- Imported notify: 원본 데이터에 어떤 cue 또는 window가 있었는가.
- Authored animation event: 우리 게임에서 언제 hit, sound, effect를 발생시키는가.
- Boss pattern: 어떤 clip을 어떤 순서와 조건으로 실행하는가.

Animation Tool이 hit event를 저장할 때 WModel을 다시 pack하지 않는다. authored event 문서만 저장하고 cooker가 runtime event DB를 만든다.

## 19. 최종 폴더 구조

### 19.1 저장소의 정본

~~~text
Data/
  Project.json

  Schemas/

  Catalogs/
    AssetPacks.json
    Models.json
    AnimationSets.json
    Characters.json
    Npcs.json
    Bosses.json
    Effects.json

  Animation/
    Imported/
      ANIM_LANCE_MASTER/
        clipmap
        animnotify
        clipseq
        skilltiming
      ANIM_MN_RPBF_01/
        clipmap
        animnotify
        clipseq
        skilltiming
    Authored/
      ANIM_LANCE_MASTER.events.json
      ANIM_MN_RPBF_01.events.json

  Effects/
    Authored/
      EFFECT_*.effect
    Imported/
      source catalog index

  UI/
    HUD/
      HUD_Layout.json
    Screen/
      ScreenUI.json

  Worlds/
    <WorldId>/
      World.json
      GeneratedBase/
      Layers/
      Gameplay/
      Presentation/
      Navigation/
        NavSource.json
        NavPaint.json
        RuntimeBlockers.json

  Receipts/
~~~

현재 custom text 포맷을 처음부터 모두 JSON으로 다시 쓰지 않는다.

- 먼저 물리 위치와 정본 여부를 바로잡는다.
- catalog와 새 world/spawn 문서는 JSON으로 만든다.
- 기존 animevents/effect/nav 문서는 기존 parser를 살려 migration한다.
- typed parser와 test가 준비된 문서부터 JSON 정본으로 전환한다.
- cooker가 기존 runtime 포맷을 계속 생성하므로 소비자를 한 번에 바꾸지 않는다.

### 19.2 외부 AssetBuild workspace

~~~text
AssetBuild/
  RawExtract/
  ModelInputs/
    <AssetId>/
      source FBX 또는 glTF
      .wmesh
      .wmat
      .wskel
      anims/*.wanim
  EffectInputs/
  ImportSpecs/
  BuildCache/
~~~

이 영역은 대형 source 공유소 또는 개인 build workspace다. 저장소와 runtime pack에 전부 넣지 않는다. 재현에 필요한 import spec과 content hash만 Data/Receipts에 남긴다.

### 19.3 선택적으로 동기화하는 runtime asset pack

~~~text
SharedAssetRoot/
  Packs/
    boss-valtan-v1/
      manifest.json
      Character/Valtan/MN_RPBF_01.wmodel
      Character/Valtan/ValtanWeapon.wmodel
      Character/Valtan/textures/*

    character-lancemaster-v1/
    npc-bern-v1/
    map-bern-v1/
    map-valtan-v1/
    effect-common-v1/
~~~

초기 migration에서는 현재 Resources/LostArk 상대 경로를 유지할 수 있다. pack manifest가 그 경로를 소유하게 만든 뒤 물리 pack mount를 정리한다.

### 19.4 runtime data

~~~text
Client/Bin/DataFiles/
  Catalogs/
  Worlds/
  Animation/
  Effects/
  UI/
  Navigation/
  Receipts/

Server/Bin/DataFiles/
  Catalogs/
  Worlds/
    Gameplay/
  Navigation/
  Receipts/
~~~

Release Client와 Server는 저장소 Data 경로를 읽지 않는다. 배포된 Bin/DataFiles만 읽는다.

## 20. 세 종류의 Root

현재는 각 Tool이 ../Bin/DataFiles 또는 ../Bin/Resources를 직접 조합한다. 이를 다음 세 root로 고정한다.

| Root | 예 | 읽기/쓰기 | 소유자 |
|---|---|---|---|
| Project Data Root | 저장소/Data | Debug Tool 읽기·쓰기 | CEditorDataRoot |
| Runtime Data Root | Client/Bin/DataFiles | runtime read-only | CRuntimeDataRoot |
| Runtime Asset Root | SharedAssetRoot 또는 Resources/LostArk | runtime read-only | 기존 CRuntimeAssetRoot |

규칙:

- EditorDataRoot는 Debug에서만 존재한다.
- RuntimeDataRoot는 Client와 Server가 각각 실행 파일 기준으로 resolve한다.
- RuntimeAssetRoot는 LOSTARK_SHARED_ASSET_ROOT를 우선한다.
- 모든 Resolve는 상대 경로만 받고 root 탈출을 막는다.
- Tool이 literal ../Bin 경로를 만들지 않는다.
- Release code가 Project Data Root에 의존하지 않는다.

## 21. 툴 범용화에서 공유할 것과 공유하지 않을 것

모든 툴을 하나의 거대한 GenericTool 클래스로 합치지 않는다. 공통 기반 네 가지와 각 domain editor를 분리한다.

### 21.1 공통 서비스

~~~mermaid
flowchart TB
    H["CMainApp<br/>유일한 Debug ToolHost"]
    H --> R["Project Asset Registry<br/>AssetId → catalog definition"]
    H --> T["Editor Target Registry<br/>현재 runtime target 목록"]
    H --> D["Editor Document Store<br/>open·draft·validate·atomic save"]
    H --> C["Editor Command Stack<br/>dirty·undo·redo"]

    R --> AT["Animation Tool"]
    R --> MT["Map Tool"]
    R --> ET["Effect Tool"]
    R --> HT["HUD Tool"]
    T --> AT
    T --> ET
    D --> AT
    D --> MT
    D --> ET
    D --> HT
    C --> AT
    C --> MT
    C --> ET
    C --> HT
~~~

#### Project Asset Registry

- Models, AnimationSets, Characters, Npcs, Bosses, Effects catalog를 load한다.
- AssetId를 packId와 상대 경로로 해석한다.
- catalog cross-reference와 pack availability를 검사한다.
- Prototype tag를 정본으로 저장하지 않는다.
- 실제 CModel 생성은 기존 Loader/Prototype 흐름에 위임한다.

#### Editor Target Registry

- 현재 level에 존재하는 Player, NPC, Valtan, PreviewActor를 weak reference로 보관한다.
- 각 target은 runtime 전용 ToolTargetId, 표시 이름, archetype ID, model AssetId, animationSetId, capability를 제공한다.
- level 전환과 despawn 시 만료 target을 제거한다.
- ToolTargetId와 pointer는 파일에 저장하지 않는다.
- Animation Tool은 level/layer/part 문자열을 직접 알지 않는다.

#### Editor Document Store

- AssetId 또는 DocumentId로 typed document를 연다.
- active snapshot과 edit draft를 분리한다.
- save는 validate → temp write → reparse → replace 순서다.
- reload는 parse → validate → stage → commit 순서다.
- 문서 타입별 parser가 domain validation을 담당한다.
- UI에 raw JSON object를 노출하지 않는다.

#### Editor Command Stack

- UI가 직접 vector를 수정하는 대신 AddEvent, MovePlacement, ChangeEmitter 같은 command를 제출한다.
- dirty와 undo/redo가 document 단위로 동작한다.
- save 성공 시 clean revision을 갱신한다.
- target preview 조작과 document 변경을 구분한다.

### 21.2 각 툴이 계속 따로 소유할 것

| Tool | 고유 책임 |
|---|---|
| Animation Tool | clip, time scrub, loop, event timeline, sequence reference |
| Map Tool | asset placement, transform, layer, navigation bake |
| Effect Tool | emitter, module, timeline, texture preview, cook |
| HUD Tool | canvas, slot, anchor, class visibility |
| NPC/Boss Tool | archetype, spawn, encounter profile |

공통 서비스가 domain 규칙까지 흡수하지 않는다.

## 22. 범용 Animation Tool의 완성 구조

### 22.1 AnimationSet이 중심이다

현재 Animation Tool은 CCharacter의 pAssetName을 중심으로 파일명을 조합한다. 범용화 후에는 AnimationSetId가 중심이다.

AnimationSets catalog의 한 정의가 소유할 정보:

| 항목 | 의미 |
|---|---|
| animationSetId | ANIM_MN_RPBF_01 같은 stable ID |
| modelAssetId | clip을 포함한 WModel AssetId |
| expectedSkeletonHash | 다른 skeleton clip 오사용 방지 |
| defaultClipId | 최초 preview clip |
| authoredEventDocument | 사람이 편집하는 event 정본 |
| importedClipMap | 선택적 원본 skill mapping |
| importedNotify | 선택적 원본 notify |
| importedClipSequence | 선택적 chain reference |
| importedSkillTiming | 선택적 timing reference |
| previewProfile | 기본 scale, camera distance, optional equipment |

clip 이름 전체를 catalog에 중복 저장하지 않는다. 실제 clip 목록은 WModel의 CModel에서 읽고 catalog/reference 문서와 일치하는지 validate한다.

### 22.2 Target과 Asset Preview를 둘 다 지원한다

Animation Tool에는 두 mode가 필요하다.

| Mode | 대상 | 목적 |
|---|---|---|
| Live Target | 현재 scene의 Player/NPC/Valtan | 실제 장비, state, socket과 함께 확인 |
| Asset Preview | catalog에서 고른 AnimationSet | 게임 객체가 없어도 clip 제작 |

Asset Preview는 두 번째 runtime이 아니다.

~~~text
AnimationSet 선택
  → modelAssetId resolve
  → AssetTest level에 model Prototype 준비
  → 기존 Prototype/Clone/Layer로 CAnimationPreviewActor 생성
  → CModel → CMaterial로 렌더
  → Editor Target Registry에 preview target 등록
~~~

CAnimationPreviewActor는 generic skinned model preview만 담당한다. Valtan, Player, NPC별 gameplay 클래스를 복제하지 않는다.

### 22.3 Animation Target 계약

Animation Tool이 받는 target view:

| 필드/기능 | 역할 |
|---|---|
| toolTargetId | 현재 session에서 선택할 임시 ID |
| displayName | Valtan, Player 1, Beda |
| animationSetId | 열 authoring/reference 문서 |
| model | weak CModel, clip 재생과 scrub |
| owner | weak CGameObject, 생명주기 확인 |
| capability | animation, socket, equipment preview 가능 여부 |
| preview override | gameplay가 animation을 덮어쓰지 못하게 하는 lease |

툴은 CCharacter, CNpc, CValtan을 include하거나 dynamic cast하지 않는다.

target adapter 또는 등록 시점이 각 구체 클래스의 model을 target view로 번역한다.

### 22.4 Preview Override Lease

Live Target의 gameplay와 Tool이 같은 CModel을 동시에 제어하면 마지막 Set_Animation 호출이 이긴다.

다음 lease 계약으로 해결한다.

~~~text
Tool이 Preview 시작 요청
  → target controller가 override 허용 여부 판단
  → 성공하면 lease 발급
  → gameplay state는 계속 계산하지만 animation switch만 보류
  → Tool이 clip/time/loop 제어
  → target 변경·창 닫기·level 전환 시 lease 자동 반환
  → 현재 gameplay state의 animation 재적용
~~~

강제 pause pointer를 Tool이 직접 잡지 않는다. lease가 만료된 target에는 명령을 보내지 않는다.

### 22.5 Document Session

~~~mermaid
flowchart LR
    A["AnimationSet 선택"] --> B["WModel clip introspection"]
    A --> C["Imported reference load"]
    A --> D["Authored event load"]
    B --> E["Animation Edit Session"]
    C --> E
    D --> E
    E --> F["ImGui timeline·clip list"]
    F --> G["Command Stack"]
    G --> E
    E --> H["Save authoring Data"]
    H --> I["Animation Data Cooker"]
    I --> J["Client/Bin/DataFiles/Animation"]
~~~

문서 전환 규칙:

- dirty이면 target 또는 AnimationSet을 조용히 전환하지 않는다.
- Save, Discard, Cancel 중 하나가 결정되어야 한다.
- target이 despawn해도 draft는 유지하고 preview만 unavailable 상태가 된다.
- reference 문서가 없어도 clip playback과 authored event 편집은 가능하다.
- WModel이 없어도 문서는 열 수 있지만 preview unavailable을 표시한다.
- skeleton hash가 다르면 clip 재생과 save apply를 막는다.

### 22.6 발탄에 적용하면

현재 MN_RPBF_01.wmodel은 skeleton과 animation 27개를 가지고 있다.

범용화 후 흐름:

~~~text
Asset Browser에서 ANIM_MN_RPBF_01 선택
  → BOSS_VALTAN_BODY WModel resolve
  → 27 clip 표시
  → att_battle_2/4/7/19/20 preview
  → ANIM_MN_RPBF_01.events.json에 hit/effect/sound 작성
  → cooker가 runtime animevents 생성
  → Boss pattern data는 clip ID와 event profile을 참조
~~~

Live Target mode에서는 ASSET_TEST의 CValtan target을 선택하고 override lease를 얻는다. Tool에는 Layer_Valtan, Part_Body 문자열이 존재하지 않는다.

## 23. 다른 툴에 같은 기반을 적용하는 방법

### 23.1 Map Tool

현재:

- CMapTool이 catalog, placement vector, dirty, file save, UI를 직접 소유한다.

목표:

- Project Asset Registry에서 Map AssetId 목록을 받는다.
- World document session이 base와 overlay layer를 연다.
- Editor Target Registry는 scene selection과 preview object만 제공한다.
- MovePlacement command가 draft를 바꾼다.
- Save는 editable layer 정본만 기록한다.
- runtime mapplacements는 cooker가 만든다.

### 23.2 Effect Tool

현재:

- authored .effect가 Runtime Asset Root 아래 있다.
- working.effect와 working.weffect, named asset이 같은 Resources 계층에 있다.
- source catalog CSV와 UModel list도 Resources에 있다.
- 파일 browser가 legacy ../Bin/Resources path를 저장할 수 있다.

목표:

- authored .effect는 Data/Effects/Authored.
- cooked .weffect는 Client/Bin/DataFiles/Effects.
- texture/model/audio는 Effect Asset Pack.
- source catalog는 Data/Effects/Imported 또는 local import cache.
- effect 내부에는 absolute path가 아니라 AssetId를 저장한다.
- attachment preview가 필요하면 Editor Target Registry에서 socket capability target을 고른다.

### 23.3 HUD Tool

현재:

- HUD cfg와 texture가 모두 Resources 아래 있다.

목표:

- layout은 Data/UI.
- texture는 UI pack.
- HUD Tool은 UI document session과 AssetId picker를 사용한다.
- class 선택은 CharacterCatalog의 CharacterClassId를 사용한다.

### 23.4 Navigation Tool

현재:

- navsource, navpaint, navgrid가 DataFiles에 섞여 있다.

목표:

- source와 paint는 World Navigation authoring.
- navgrid는 cooker output.
- runtime blockers는 gameplay document에서 Client와 Server용으로 cook한다.
- Map Tool은 active WorldId를 통해 navigation document를 연다.

### 23.5 NPC/Boss Tool

- Npc/Boss catalog에서 archetype을 선택한다.
- visual preview는 model/animation/effect catalog를 조합한다.
- spawn 편집은 World Gameplay document에 저장한다.
- live server entity의 NetEntityId는 관찰용이며 저장하지 않는다.

## 24. 공통 파일 책임 제안

### Client/Public/Data/RuntimeDataRoot.h, Client/Private/Data/RuntimeDataRoot.cpp

- 실행 파일 기준 Client/Bin/DataFiles를 resolve한다.
- runtime read-only다.
- authoring save를 제공하지 않는다.

### Client/Public/Editor/EditorDataRoot.h, Client/Private/Editor/EditorDataRoot.cpp

- Debug에서 저장소 Data root를 resolve한다.
- LOSTARK_PROJECT_DATA_ROOT와 명시 command line을 우선한다.
- Release dependency가 되지 않는다.

### Client/Public/Asset/ProjectAssetRegistry.h, Client/Private/Asset/ProjectAssetRegistry.cpp

- typed catalog를 stage하고 cross-reference를 검증한다.
- Model AssetId를 pack과 WModel 상대 경로로 해석한다.
- runtime pointer와 Prototype tag를 저장하지 않는다.

### Client/Public/Editor/EditorTargetRegistry.h, Client/Private/Editor/EditorTargetRegistry.cpp

- weak runtime target과 capability를 등록·조회·제거한다.
- level generation을 확인해 이전 level target을 자동 무효화한다.
- 파일 저장 기능이 없다.

### Client/Public/Editor/EditorDocumentStore.h, Client/Private/Editor/EditorDocumentStore.cpp

- DocumentId로 typed session을 연다.
- atomic save와 reload transaction을 조정한다.
- 각 domain parser를 대체하지 않는다.

### Client/Public/Editor/EditorCommandStack.h, Client/Private/Editor/EditorCommandStack.cpp

- document command의 apply/revert와 dirty revision을 소유한다.
- runtime gameplay command와 섞지 않는다.

### Client/Public/Editor/AnimationPreviewActor.h, Client/Private/Editor/AnimationPreviewActor.cpp

- AssetTest에서 generic animated model을 기존 CModel 경로로 preview한다.
- AI, HP, navigation, combat을 소유하지 않는다.
- optional equipment/socket preview descriptor만 받는다.

### Client/Public/Animation/AnimationAssetCatalog.h, Client/Private/Animation/AnimationAssetCatalog.cpp

- AnimationSetId와 model/event/reference 문서 관계를 해석한다.
- WModel clip key를 복제 보관하지 않는다.

### Client/Public/Animation/AnimationEditSession.h, Client/Private/Animation/AnimationEditSession.cpp

- selected AnimationSet, clip introspection, imported reference, authored draft를 묶는다.
- UI code와 파일 system 호출을 분리한다.

새 C++ 파일은 물리 폴더 구조 그대로 Client.vcxproj와 Client.vcxproj.filters에 필요한 항목만 추가한다. 기존 filter를 이동하지 않는다. 공통 data/target/editor 계층은 Client가 소유하며 Engine에 ImGui 또는 LostArk AssetId를 추가하지 않는다.

## 25. 첫 범용화 세로 조각

통합 전체를 시작하기 전에 Animation Tool로 공통 구조를 검증한다.

### 사용자에게 보일 결과

- Animation Tool에 Live Target과 Asset Preview mode가 있다.
- target 목록에 Player, Valtan, NPC, PreviewActor가 capability에 따라 나타난다.
- 발탄을 spawn하지 않아도 ANIM_MN_RPBF_01을 선택해 27개 clip을 볼 수 있다.
- live Valtan을 선택하면 Preview Override 동안 AI가 clip을 덮어쓰지 않는다.
- target/asset을 바꿔도 dirty event가 조용히 사라지지 않는다.
- Save는 Data/Animation/Authored에 쓴다.
- Cook 후 Client/Bin/DataFiles/Animation에 runtime event가 생성된다.
- WModel과 texture는 수정되지 않는다.

### 구현 순서

1. 세 root 계약을 추가하고 기존 Asset Root는 유지한다.
2. Models.json과 AnimationSets.json에 LanceMaster와 MN_RPBF_01 두 fixture를 등록한다.
3. ProjectAssetRegistry와 AnimationAssetCatalog를 추가한다.
4. EditorTargetRegistry를 CMainApp 소유로 추가한다.
5. AnimationPreviewActor를 AssetTest Prototype/Clone/Layer에 연결한다.
6. AnimationEditSession을 만들고 현재 Animation_Tool의 event/reference 상태를 이동한다.
7. Animation_Tool의 TEST_LEVEL2, Layer_Player, Part_00_Body 하드코딩을 제거한다.
8. Player와 Valtan live target adapter를 등록한다.
9. Preview Override lease를 연결한다.
10. authored save를 Project Data Root로 바꾼다.
11. cooker가 현재 animevents v3 runtime을 생성하게 한다.
12. 성공 후 NPC target adapter를 추가한다.

### 이 단계에서 하지 않을 것

- WModel format 변경.
- Engine animation API 재작성.
- 발탄 pattern/AI 구현.
- 모든 기존 anim reference 문서 JSON 변환.
- Map/Effect/HUD Tool 동시 대규모 개편.
- 두 번째 editor executable 또는 별도 renderer 생성.

### 검증

1. ModelAssetConverter info로 LanceMaster와 Valtan section/clip 수를 확인한다.
2. PreviewActor에서 Valtan 27개 clip을 순서대로 선택한다.
3. Pause, frame step, scrub, loop를 확인한다.
4. live Valtan override 중 IDLE/CHASE 전환이 선택 clip을 덮어쓰지 않는지 확인한다.
5. override 반환 시 현재 state의 idle/run으로 복귀하는지 확인한다.
6. event 저장 후 WModel hash가 바뀌지 않았는지 확인한다.
7. dirty 상태 target 전환에서 Save/Discard/Cancel이 동작하는지 확인한다.
8. target despawn 후 dangling pointer 없이 preview unavailable이 되는지 확인한다.
9. 잘못된 skeleton hash catalog가 stage에서 실패하는지 확인한다.
10. Project Data가 없어도 Release Client가 runtime DataFiles로 실행되는지 확인한다.
11. Engine Debug/Release → UpdateLib Debug/Release → Client Debug/Release 순서를 검증한다.

## 26. 구조를 판단하는 한 문장 규칙

새 파일이나 기능을 어디에 둘지 헷갈릴 때 다음 질문을 순서대로 적용한다.

1. 사람이 diff하고 고치는가? 그러면 Data의 authoring 정본이다.
2. 외부 원본에서 자동 생성되는가? 그러면 Imported 또는 GeneratedBase다.
3. 모델·texture·audio의 큰 payload인가? 그러면 선택적 Asset Pack이다.
4. 실행 속도를 위해 만든 결과인가? 그러면 Bin/DataFiles runtime 산출물이다.
5. 지금 scene에만 존재하는 객체나 pointer인가? 그러면 Runtime Instance이며 저장하지 않는다.
6. Tool이 특정 클래스 이름을 알아야 하는가? capability/target adapter가 빠졌다는 신호다.
7. Tool이 ../Bin 경로를 직접 쓰는가? Root/Document Store 경계가 빠졌다는 신호다.
8. JSON에 Prototype tag나 vector index가 있는가? stable ID 계약이 깨진 것이다.

이 규칙을 지키면 Player, NPC, Valtan, Map, Effect가 서로 다른 기능이어도 같은 데이터·툴 기반 위에서 작업할 수 있다.

## 27. Player·NPC·Monster·Boss 공통 Entity의 핵심 그림

공통 Entity를 이해할 때 Client 클래스 상속부터 보면 혼란이 커진다. 하나의 Entity가 네 공간에서 서로 다른 모습으로 존재한다고 본다.

~~~mermaid
flowchart LR
    A["Data Archetype<br/>무엇인가<br/>PLAYER_LANCE·NPC_BEDA·BOSS_VALTAN"]
    B["Server Entity<br/>실제 상태의 권한<br/>Transform·HP·Action·Phase"]
    C["Network DTO<br/>필요한 상태만 전달<br/>NetEntityId·ArchetypeId·Snapshot"]
    D["Client Presentation<br/>어떻게 보이는가<br/>CCharacter·CNpc·CValtan"]

    A --> B
    B --> C
    C --> D
    A --> D
~~~

각 공간의 질문은 다르다.

| 공간 | 질문 | 저장/수명 |
|---|---|---|
| Data Archetype | 이 종류는 무엇인가 | Git 정본, stable ArchetypeId |
| Server Entity | 지금 실제로 어떤 상태인가 | Room runtime, NetEntityId |
| Network DTO | Client가 이번에 무엇을 알아야 하는가 | packet 일시 데이터 |
| Client Presentation | 어떤 모델과 animation으로 보여줄 것인가 | Level/Layer GameObject |

Server Entity와 Client CGameObject는 같은 객체가 아니다. 서로 NetEntityId와 ArchetypeId로 대응할 뿐이다.

## 28. 현재 구현 체크포인트

### 28.1 Client

| 클래스 | 현재 형태 | 현재 책임 | 판단 |
|---|---|---|---|
| CCharacter | CContainerObject | class spec, body/equipment, animation chain, navigation | 신규 Player 표현 기반 |
| CNpc | CGameObject | 단일 skinned model, idle clip | 시각 pilot, network 미연결 |
| CValtan | CContainerObject | body/weapon, local chase, idle/run | 보스 시각 pilot, 권한이 Client에 있음 |
| CMonster | CGameObject | 구형 random animation, collider/navigation | 공통 기반으로 승격하지 않는 legacy |
| CPlayer | CContainerObject | 구형 직접 input/state/part 조립 | CCharacter 이관 후 정리할 legacy |

이 클래스들을 모두 CActorBase 하나 아래로 다시 상속시키지 않는다.

- Player는 여러 part와 class logic이 필요하다.
- NPC는 단일 merged model일 수 있다.
- Boss는 body, weapon, encounter presentation이 필요하다.
- Monster는 data-driven combat presentation으로 다시 설계될 수 있다.

공통점은 렌더 클래스의 형태가 아니라 Entity identity, transform 적용, animation target, spawn/despawn 생명주기다.

### 28.2 Server

현재 Server는 다음만 가진다.

- SERVER_PLAYER.
- CGameRoom의 m_Players.
- SessionId → PlayerId, NetEntityId → PlayerId index.
- Player join/spawn/despawn.
- Room fixed tick과 command queue.

Monster, NPC, Boss entity registry와 simulation component는 아직 없다.

### 28.3 Network와 Client replication

현재:

- S2C_PLAYER_SPAWNED와 S2C_PLAYER_DESPAWNED만 구현됐다.
- C2S_MOVE와 S2C_WORLD_SNAPSHOT enum은 있으나 message 계약과 실제 처리 흐름은 아직 없다.
- CClientReplication은 CCharacter 생성만 안다.
- CNetObjectRegistry는 NET_PLAYER_RECORD와 weak CCharacter만 저장한다.
- Bern Level이 replication lifetime과 Layer_Player를 소유하도록 연결 중이다.

즉, 지금 Player 세로 조각은 좋은 기준선이지만 generic entity layer는 아직 아니다.

현재 Player spawn 흐름:

~~~text
C2S_ENTER_WORLD
  → ServerApp가 RoomCommand enqueue
  → CGameRoom::Join
  → SERVER_PLAYER 생성
  → S2C_PLAYER_SPAWNED
  → NetworkManager의 main-thread event queue
  → CLevel_Baren의 CClientReplication
  → CharacterCatalog
  → Prototype_GameObject_Character clone
  → Layer_Player
  → CNetObjectRegistry
~~~

이 흐름을 디버거로 끝까지 설명할 수 있는 것이 첫 학습 목표다.

## 29. 절대로 섞지 않을 ID

| ID | 의미 | 저장 여부 | 예 |
|---|---|---:|---|
| ArchetypeId | 어떤 종류인가 | Data 정본 | BOSS_VALTAN |
| RuntimeArchetypeId | packet용 compact ID | cooker 생성 | uint32 |
| PlacementId | 월드 어디에 놓였는가 | Data 정본 | boss_spawn_center |
| NetEntityId | 이번 Room의 실제 인스턴스 | 저장 금지 | 104 |
| PlayerId | Room의 Player identity | runtime | 2 |
| SessionId | socket session | 저장 금지 | 17 |
| ObjectHandle | Client local slot+generation | 저장/전송 금지 | slot 3 gen 2 |
| ModelAssetId | 어떤 WModel을 쓰는가 | Client catalog | BOSS_VALTAN_BODY |
| AnimationSetId | 어떤 clip/event 집합인가 | Client catalog | ANIM_MN_RPBF_01 |

Data의 문자열 ArchetypeId는 cooker가 RuntimeArchetypeId로 변환한다. 변환 표와 collision 검사를 receipt에 남긴다.

NetEntityId는 ArchetypeId를 대체하지 않는다.

- BOSS_VALTAN은 정의 하나다.
- NetEntityId 104는 이번 Room에 생성된 발탄 한 마리다.
- 다음 Room의 같은 발탄은 다른 NetEntityId를 갖는다.

## 30. 권한 분리

### Server가 결정할 것

- spawn과 despawn.
- 실제 transform과 이동 결과.
- HP, stagger, status.
- 현재 gameplay action과 action 시작 server tick.
- target 선택과 AI 결정.
- boss phase와 encounter condition.
- NPC interaction 가능 여부.
- damage와 hit 결과.

### Client가 결정할 것

- model, material, equipment 조립.
- transform interpolation.
- action ID를 clip으로 표시.
- animation local playback time 보정.
- effect, sound, camera, HUD.
- selection outline과 Debug Tool preview.

### Client가 Server에 보내는 것

Client는 결과 transform을 보내지 않고 intent를 보낸다.

~~~text
Local input
  → MoveIntent 또는 SkillIntent
  → Server validate
  → Server simulation
  → authoritative state
  → Snapshot/Event
  → Client presentation
~~~

Boss와 Monster AI도 최종적으로 같은 intent 입구를 사용할 수 있다.

~~~text
Player Session → Player Intent Source ┐
Monster AI     → AI Intent Source     ├→ Entity Intent → World Simulation
Boss Script    → Boss Intent Source   ┘
~~~

같은 명령을 생성한다고 해서 Player와 Boss가 같은 C++ 클래스를 상속할 필요는 없다.

## 31. Server Entity는 상속보다 component 조합

### 31.1 Entity Core

모든 server entity가 공통으로 가지는 최소 상태:

| 필드 | 의미 |
|---|---|
| NetEntityId | Room 인스턴스 identity |
| EntityKind | Player, NPC, Monster, Boss |
| RuntimeArchetypeId | data definition 연결 |
| Transform | position, yaw, optional velocity |
| LifecycleState | Spawning, Alive, Dying, Despawning |
| CapabilityMask | 어떤 component가 있는지 |
| ReplicationRevision | 변경 추적 |

Core에 nickname, boss phase, AI target을 모두 넣지 않는다.

### 31.2 Capability component

| Entity | 조합 |
|---|---|
| Player | Core + PlayerOwner + Movement + Combat + Action |
| Town NPC | Core + Interaction + Action |
| Monster | Core + Movement + Combat + Action + AI |
| Boss | Core + Movement + Combat + Action + AI + BossEncounter |
| Client-only Map Prop | Server Entity가 아님 |

component 역할:

| Component | 소유 상태 |
|---|---|
| PlayerOwner | SessionId, PlayerId, class, nickname |
| Movement | speed, goal, velocity, navigation state |
| Combat | HP, max HP, stagger, target, status |
| Action | ActionId, action instance, start tick, phase |
| AI | brain profile, decision timer, aggro target |
| Interaction | talk/quest/shop profile, enabled |
| BossEncounter | encounter ID, phase, mechanic state |

첫 구현부터 완전한 ECS framework를 만들지 않는다. CServerEntityRegistry가 Core map과 필요한 component map을 소유하는 작은 data-oriented 구조면 충분하다.

### 31.3 Server 시스템

~~~mermaid
flowchart TB
    R["CGameRoom<br/>session·command·fixed tick owner"]
    R --> W["CServerWorld"]
    W --> ER["Entity Registry"]
    W --> MS["Movement System"]
    W --> CS["Combat System"]
    W --> AS["Action System"]
    W --> AIS["AI System"]
    W --> BS["Boss Encounter System"]
    W --> RS["Replication System"]

    ER --> MS
    ER --> CS
    ER --> AS
    ER --> AIS
    ER --> BS
    ER --> RS
~~~

CGameRoom은 socket을 직접 다루지 않고 현재처럼 session weak reference와 send/broadcast 경계를 유지한다. CServerWorld는 Client, Renderer, ImGui를 모른다.

## 32. Network 계약

최종 packet은 공통 envelope와 필요한 typed payload로 나눈다.

### Spawn

공통:

- NetEntityId.
- EntityKind.
- RuntimeArchetypeId.
- initial transform.
- capability mask.

typed payload:

- Player면 PlayerId, class, nickname, local owner 정보.
- NPC면 interaction baseline.
- Monster면 combat baseline.
- Boss면 combat와 encounter baseline.

packet에 넣지 않는 것:

- WModel path.
- Prototype tag.
- animation clip name.
- C++ class name.
- pointer와 ObjectHandle.

### Snapshot

- 여러 Entity의 transform/action/combat delta를 batch한다.
- Server tick과 entity revision을 포함한다.
- Client는 오래된 revision을 적용하지 않는다.
- 매 snapshot에 모든 catalog와 nickname을 반복하지 않는다.

### Action

Server는 clip 이름 대신 gameplay ActionId와 시작 tick을 보낸다.

~~~text
Server: ACTION_VALTAN_AXE_COMBO_2 시작
  → Client Presentation Catalog
  → ANIM_MN_RPBF_01
  → att_battle_2_01, 02, 03
  → effect/sound event 재생
~~~

Server는 발탄 WModel에 어떤 clip 이름이 있는지 모른다.

### Despawn

- NetEntityId.
- generic despawn reason.
- 필요한 경우 death/level change/disconnect의 typed reason.

Player 전용 disconnect와 일반 entity death를 하나의 PLAYER_DESPAWN_REASON으로 계속 확장하지 않는다.

## 33. Client는 Presentation Adapter로 일반화

Client GameObject 상속을 다시 만들지 않고 spawn adapter를 둔다.

~~~mermaid
flowchart LR
    P["S2C_ENTITY_SPAWNED"] --> Q["Main-thread Replication Queue"]
    Q --> F["Client Entity Factory"]
    F --> A1["Player Adapter → CCharacter"]
    F --> A2["NPC Adapter → CNpc"]
    F --> A3["Monster Adapter → 신규/정리된 Monster View"]
    F --> A4["Boss Adapter → CValtan"]
    A1 --> R["Generic Net Entity Registry"]
    A2 --> R
    A3 --> R
    A4 --> R
~~~

### Generic Net Entity Registry

현재 CNetObjectRegistry의 최종 형태:

- NET_ENTITY_RECORD를 저장한다.
- weak CGameObject를 저장한다.
- NetEntityId → ObjectHandle을 제공한다.
- ObjectHandle generation으로 stale reference를 막는다.
- EntityKind와 RuntimeArchetypeId를 record에 가진다.
- 특정 CCharacter를 강하게 소유하지 않는다.

Local Player가 필요하면:

~~~text
Local NetEntityId
  → Generic Registry
  → weak CGameObject resolve
  → CCharacter typed cast
~~~

### Client Entity Factory와 Adapter

Adapter 계약:

- 해당 EntityKind/Archetype을 만들 수 있는지 판단.
- spawn DTO를 typed Client descriptor로 변환.
- 기존 Prototype/Clone/Layer로 stage 생성.
- snapshot을 해당 presentation에 적용.
- despawn 시 Layer 제거.
- 실패 시 staged object rollback.

Player Adapter만 먼저 구현한 뒤 NPC Adapter를 두 번째로 추가한다.

### Presentation capability

Animation Tool과 Effect Tool이 보는 capability는 network interface와 분리한다.

- AnimationTarget capability.
- SocketTarget capability.
- TransformTarget capability.
- EffectAttachment capability.

Server entity component를 ImGui가 직접 include하지 않는다. Client adapter가 생성 성공 후 Editor Target Registry에 필요한 capability를 등록한다.

## 34. Archetype Data 분리

모든 entity catalog는 공통 header를 가지되 domain 문서는 분리한다.

공통 header:

- ArchetypeId.
- EntityKind.
- display name.
- gameplay profile ID.
- presentation profile ID.
- capability 선언.

domain:

| 문서 | 전용 정보 |
|---|---|
| Characters | class, skill deck, player defaults |
| Npcs | interaction, talk, quest/shop profile |
| Monsters | AI, aggro, combat, loot |
| Bosses | encounter, phase, stagger, mechanic profile |

Client presentation profile:

- ModelAssetId.
- AnimationSetId.
- equipment set.
- shader/render profile.
- effect set.

Server gameplay profile:

- movement/combat/action/AI/encounter 값.
- resource path 없음.

cooker가 같은 ArchetypeId에서 Client와 Server runtime catalog를 각각 만든다.

## 35. Entity 전체 생명주기

~~~text
Data Spawn Document
  → Server world bootstrap
  → Archetype validate
  → ServerEntityRegistry stage create
  → NetEntityId 발급
  → component attach
  → registry commit
  → S2C_ENTITY_SPAWNED
  → Client main-thread queue
  → ClientEntityFactory stage create
  → Prototype/Clone/Layer
  → Generic NetEntityRegistry commit
  → EditorTargetRegistry capability 등록

Server fixed tick
  → player/AI/boss intent
  → movement/action/combat system
  → dirty revision
  → snapshot/event
  → Client interpolation/animation/effect

Server despawn
  → S2C_ENTITY_DESPAWNED
  → Client Layer 제거
  → NetEntityRegistry unregister
  → EditorTargetRegistry unregister
  → ObjectHandle generation 증가
~~~

모든 create는 stage가 완성된 뒤 registry에 commit한다. Client GameObject 생성 실패 시 NetEntityId만 등록된 반쪽 상태를 남기지 않는다.

## 36. 지금 당장 해야 할 일

현재는 generic Boss framework부터 만들지 않는다.

### 1단계: 작업 중인 Player 세로 조각 완성

먼저 현재 변경을 독립적으로 완료하고 검증한다.

- Server 1개와 Client 2개.
- join 시 두 CCharacter spawn.
- 기존 Client가 신규 Player를 받음.
- disconnect 시 다른 Client에서 despawn.
- reconnect와 중복 spawn.
- Bern level 전환 시 registry/layer/camera 정리.
- Registry는 weak reference만 소유.

이 상태를 commit 가능한 기준선으로 만든다. Player slice가 불안한 상태에서 generic refactor를 시작하지 않는다.

### 2단계: 이름과 소유권만 문서로 고정

코드 전에 다음 표를 한 장으로 고정한다.

| 질문 | 답 |
|---|---|
| Entity runtime owner | Server CGameRoom/CServerWorld |
| Client GameObject owner | Level Layer |
| Client registry ownership | weak reference |
| 정의 owner | Data Archetype Catalog |
| model owner | Asset Pack |
| network instance identity | NetEntityId |
| tool selection identity | local ToolTargetId |

### 3단계: Player 동작을 바꾸지 않는 generic rename/refactor

- ENTITY_KIND와 RuntimeArchetypeId 계약 추가.
- NET_ENTITY_RECORD 추가.
- CNetObjectRegistry를 weak CGameObject 기반으로 일반화.
- Player Adapter 추가.
- 기존 Player spawn 결과가 완전히 동일한지 확인.

이 단계에서는 NPC packet을 아직 추가하지 않는다.

### 4단계: 정지 NPC 한 명을 두 번째 타입으로 연결

NPC가 가장 좋은 abstraction test다.

- Server에 Core + Interaction만 가진 NPC를 spawn.
- S2C_ENTITY_SPAWNED.
- Client NPC Adapter가 CNpc를 생성.
- 같은 generic registry에 등록.
- AI, combat, navigation은 넣지 않는다.

Player와 NPC가 같은 registry/lifecycle을 쓰면 공통 abstraction이 실제로 검증된다.

### 5단계: Monster

- Core + Movement + Combat + Action + AI.
- AI가 MoveIntent/ActionIntent 생성.
- Server simulation이 transform과 combat 결과를 결정.
- Client는 snapshot과 action을 표시.

기존 CMonster를 억지로 확장하지 말고 소비자와 asset을 확인한 뒤 presentation adapter 안에서 재사용 또는 교체를 결정한다.

### 6단계: Valtan Boss

- Monster 공통 component를 재사용.
- BossEncounter component와 system만 추가.
- Server가 phase/action/HP/stagger 권한을 가진다.
- Client CValtan은 presentation이 되고 local chase AI를 제거하거나 Debug-only preview로 제한한다.
- ActionId를 AnimationSet과 EffectSet으로 표현한다.

## 37. 먼저 이해해야 할 여섯 가지

### 1. Definition과 Instance

- BOSS_VALTAN은 Data definition.
- NetEntityId 104는 현재 Room의 instance.
- CValtan은 Client presentation.

이 셋을 같은 객체라고 생각하지 않는다.

### 2. Owner와 Observer

- Server Registry는 gameplay state owner.
- Client Layer는 visual object owner.
- Client Net Registry는 weak observer.
- Tool Registry도 weak observer.

### 3. Intent와 Result

- Client와 AI는 의도를 만든다.
- Server simulation이 결과를 만든다.
- Client는 결과를 표현한다.

### 4. Component와 Inheritance

- 공통 상태는 server component.
- 서로 다른 렌더 조립은 Client class.
- 공통 툴 조작은 capability.

한 부모 클래스가 세 문제를 모두 해결하지 않는다.

### 5. Stable ID와 Runtime Handle

- ArchetypeId는 저장한다.
- NetEntityId는 Room 동안만 쓴다.
- ObjectHandle은 Client 내부에서만 쓴다.
- pointer는 저장하거나 전송하지 않는다.

### 6. Spawn Transaction

parse → validate → stage → commit → broadcast 순서를 이해한다. 실패하면 아직 broadcast하지 않은 staged 상태를 rollback한다.

## 38. 디버거로 따라갈 학습 순서

첫날에는 코드를 새로 만들기보다 현재 Player 한 명을 따라간다.

1. C2S_ENTER_WORLD가 packet parser를 통과하는 지점.
2. ROOM_COMMAND가 enqueue되는 지점.
3. CGameRoom::Join에서 PlayerId와 NetEntityId가 발급되는 지점.
4. S2C_PLAYER_SPAWNED가 serialize되는 지점.
5. Client NetworkManager가 replication event를 만드는 지점.
6. CLevel_Baren이 CClientReplication::Update를 호출하는 지점.
7. CharacterCatalog가 CHARACTER_SPEC을 고르는 지점.
8. CCharacter clone이 Layer_Player에 들어가는 지점.
9. CNetObjectRegistry가 weak reference를 등록하는 지점.
10. disconnect 후 Layer 제거와 generation 증가 지점.

각 중단점에서 다음 값만 적는다.

- SessionId.
- PlayerId.
- NetEntityId.
- CharacterClassId 또는 ArchetypeId.
- LevelId와 Layer tag.
- ObjectHandle slot/generation.
- shared/weak reference owner.

이 흐름을 종이에 다시 그린 뒤 NPC에 필요한 차이만 표시한다. 그때부터 abstraction을 구현한다.

## 39. 제안 파일 책임

### Shared/Public/Entity/EntityIds.h

- ArchetypeId runtime type, EntityKind, invalid 값.
- Client/Server 중립.
- resource path 없음.

### Shared/Public/Entity/EntityState.h

- neutral transform, lifecycle, capability mask, action baseline.
- DirectX 타입과 Engine include 없음.

### Shared/Public/Network/EntityMessages.h, Shared/Private/Network/EntityMessages.cpp

- generic spawn, snapshot, action, despawn DTO와 serialization.
- 기존 Player packet은 첫 migration 동안 adapter로 유지 가능.

### Server/Public/Entity/ServerEntityRegistry.h, Server/Private/Entity/ServerEntityRegistry.cpp

- Core와 component map, NetEntityId 발급, stage/commit/remove.
- socket/session을 직접 다루지 않는다.

### Server/Public/World/ServerWorld.h, Server/Private/World/ServerWorld.cpp

- fixed tick에서 intent와 system 순서를 조정한다.
- CGameRoom이 소유한다.

### Server/Public/Entity/EntityIntent.h

- Move, Face, Action 같은 server 내부 intent.
- Client가 다른 entity를 조종하는 packet DTO로 그대로 노출하지 않는다.

### Server/Public/Replication/ServerReplication.h, Server/Private/Replication/ServerReplication.cpp

- registry revision을 packet으로 번역한다.
- Client resource와 animation clip을 모른다.

### Client/Public/Replication/NetEntityRecord.h

- generic Client-side network record.
- EntityKind, RuntimeArchetypeId, latest revision.

### Client/Public/Replication/ClientEntityFactory.h, Client/Private/Replication/ClientEntityFactory.cpp

- archetype과 kind에 맞는 adapter를 고른다.
- Prototype/Clone/Layer stage 생성.

### Client/Public/Replication/ClientEntityAdapter.h

- create, apply snapshot, destroy 계약.
- Player/NPC/Monster/Boss adapter가 구현한다.

### 기존 CNetObjectRegistry 수정

- weak CCharacter를 weak CGameObject로 일반화.
- typed Resolve helper는 호출자 쪽에 둔다.
- ObjectHandle generation 계약은 보존한다.

### 기존 CClientReplication 수정

- Player 전용 switch를 generic entity event와 factory로 교체한다.
- Level은 replication lifetime과 대상 layer policy를 계속 소유한다.

모든 신규 Shared/Server/Client C++ 파일은 각 vcxproj와 filters에 물리 폴더대로 등록한다. Shared public header를 바꾼 뒤 Shared와 Server를 검증하고, Engine → UpdateLib → Client Debug/Release 순서를 지킨다.

## 40. 하지 말아야 할 것

- CActorBase에 Player, NPC, Monster, Boss의 모든 필드를 넣기.
- Server에서 CCharacter, CValtan, CModel include.
- packet으로 Prototype tag와 model path 보내기.
- Client가 Monster/Boss 실제 transform과 HP를 독자 결정하기.
- animation clip 이름을 Server gameplay truth로 만들기.
- CGameRoom 안에 AI, combat, packet serialization을 계속 직접 추가하기.
- current Player slice가 검증되기 전에 generic packet으로 전부 교체하기.
- CPlayer와 CMonster legacy를 공통 기반이라고 가정하기.

지금의 가장 중요한 산출물은 완벽한 클래스 다이어그램이 아니라 Player spawn 한 줄을 확실히 이해하고, 같은 registry와 lifecycle로 정지 NPC 한 명을 추가하는 것이다. 그 두 타입이 통과한 뒤 Monster와 Boss의 공통 component를 설계하면 추상화가 실제 요구에서 나온다.
