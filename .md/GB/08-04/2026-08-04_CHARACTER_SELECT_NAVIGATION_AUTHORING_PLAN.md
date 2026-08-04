# Character Select Navigation Authoring PLAN

작성일: 2026-08-04

상태: 하위 계획. 상위 workspace 계획은
`2026-08-04_DEVELOPMENT_MAP_EDITOR_WORKSPACE_PLAN.md`를 따른다. Character Select의
exact floor admission과 provenance v3는 workspace 구현 이후 후속으로 적용한다.

## 0. C1~C8 관점

- C1 기준계(핵심): UE3 placement world 좌표, WModel의 0.01 pre-scale, NavGrid XZ cell과 world Y를 같은 기준계로 유지한다.
- C2 이동>계산(핵심): triangle 높이, 경사, walkability는 MapTool bake에서 계산하고 authoring 문서에 고정한다.
- C3 공유는 비싸다: visual placement, navigation, gameplay placement를 한 문서에 합치지 않는다.
- C4 수명은 선언된다: Character Select는 socket 없는 preview Level이다. navigation은 Debug MapTool authoring 수명만 가진다.
- C5 이산화와 오차(핵심): 0.5 m cell 중심의 최고 표면 Y, slope 한계, manual blocked를 명시한다.
- C6 가지치기: CUL_BOX, decor, pillar를 이름 추측으로 처리하지 않고 stable numeric placement admission으로 bake 입력을 제한한다.
- C7 권위와 정합성(핵심): Data/Navigation이 정본이다. Character Select의 Client/Server runtime navgrid는 만들지 않는다.
- C8 검증이 병목: admission digest, bake cell 통계, source/paint 재로드 hash, rollback을 별도 증거로 남긴다.

## 1. 문제 해결 ①~⑤

① 문제·제약: LV_LOBBY_CLASSSELECT_SL00 visual 55 assets/803 placements는 있지만 navigation authoring이 없다. preview Character는 pNavigationPrototypeTag=null이며 제품 이동 소비자가 아니다.

② 단순 해법의 문제: 발탄 전용 Python의 CUL_BOX/Floor 상수를 복사하면 다른 placement를 가진 Character Select에서 틀린 결과가 나온다. 모든 visible static을 굽는 generic 경로도 겹친 장식의 평평한 윗면을 floor로 오인할 수 있다.

③ 해결 방식: 현재 CNavGridBaker를 유지하고 exact numeric placementId와 expected assetId admission을 추가한다. bake input provenance는 navsource v3에 기록하고 수동 block은 navpaint에 저장한다.

④ 비교: 발탄 Python의 CUL_BOX는 범위 계산만 했다. 새 계획에서도 CUL_BOX는 walkable surface가 아니며 흰 Nav Bounds가 범위 정본이다.

⑤ 대가: admission UI/parser와 navsource v3가 추가된다. 대신 같은 asset의 다른 placement 혼입, 장식 최고 Y 오염, 재베이크 입력 불명확성을 없앤다.

## 2. 현재 상태 실측

### 2.1 발탄 Python과 현재 MapTool은 같은 방식이 아니다

Tools/LevelPlacementExtractor/build_valtan_navgrid.py는 발탄 한 장면 전용 offline bootstrap이다.

    walkable triangle
      BG_RAD_VALTAN_FLOOR01_SM / FLOOR01A / FLOOR01B
      MAP_4A6CF4B84315_LV_LUT_HEARTRB_FLOOR01_SM exact placement 4개
      MAP_FBC80A02F72E_BG_LUT_WAGLOY_CIRCLEFLOOR01_SM_JJY exact placement 1개

    bounds only
      MAP_1E5F55FC0524_LV_COMMON_MESH_CUL_BOX_8 exact placement 1개

CUL_BOX 자체를 walkable로 굽지 않았다. floor triangle을 cell center에 투영하고 같은 XZ cell에서는 최고 Y를 선택해 navsource v1을 만들었다.

현재 MapTool은 다음 흐름이다.

    white NAVGRID_BAKE_DESC
    -> bounds와 겹치는 visible static placement 수집
    -> LV_NAVIMESH / CUL_BOX 제외
    -> WModel decode
    -> placement world transform
    -> cell center 최고 Y + max slope
    -> navsource v2
    -> navpaint / navblockers
    -> runtime navgrid

generic 수집은 CUL_BOX만 제외하고 모든 visible deferred static을 읽는다. Character Select처럼 floor/decor/bridge가 겹치는 씬에서는 평평한 decor나 뒤집힌 면이 더 높은 Y로 선택될 수 있다. manual paint는 blocked bit만 바꾸므로 잘못 선택된 Y를 아래 floor Y로 복구하지 못한다.

### 2.2 Character Select Area

| 항목 | 실측 |
|---|---:|
| Area ID | LV_LOBBY_CLASSSELECT_SL00 |
| visual assets | 55 |
| visual placements | 803 |
| preview 중심 | (-772.017, -142.55, 197.538) |
| CUL_BOX_1 placements | 20 |
| CUL_BOX_4 placements | 4 |
| navigation authoring | 없음 |
| Client/Server runtime navigation | 없음 |
| gameplay world document | 없음 |

24개 CUL_BOX는 원형 둘레에 흩어져 있다. 발탄처럼 단일 CUL_BOX 하나를 bounds로 쓰는 구조가 아니다.

### 2.3 Data 저장 상태

| 정본 | 현재 보유 데이터 | 의미 |
|---|---|---|
| Data/Maps/Imported/AreaId | Character Select 재추출 catalog/baseline | 생성 가능한 visual과 원본 배치 증거 |
| Data/Maps/Authoring/AreaId | Character Select visual placement | 현재 보이는 정적 맵 |
| Data/Navigation/LV_LUT_HEARTRB_ED.* | 발탄 source/paint/blockers | 제품 Server navigation 입력 |
| Data/Navigation/LV_DEV_TRAINING_GROUND.navgrid.json | uniform 수련장 grid | 제품 Server navigation 입력 |
| Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json | playerSpawn 4 + boss 1 | 발탄 gameplay placement |
| Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json | playerSpawn 4 | 베른 gameplay placement |
| Data/Worlds/LV_DEV_TRAINING_GROUND/Gameplay.world.json | playerSpawn 4 | 수련장 gameplay placement |
| Data/Actors/BossCatalog.json | BOSS_VALTAN | boss stable archetype |
| Data/Actors/NpcCatalog.json | 0 rows | NPC 저장 형식은 있으나 제품 presentation 미완료 |
| Data/Encounters/Valtan/ValtanEncounter.json | 발탄 encounter | Character Select와 무관 |

Character Select에는 Gameplay.world.json을 만들지 않는다. preview transform은 CLevel_CharacterSelect presentation 상수이며 Server spawn이 아니다.

일반 Monster는 schema, catalog, Server brain, replication이 없다. npc나 boss로 위장하지 않고 이번 계획에서도 추가하지 않는다.

### 2.4 현재 피킹의 실제 의미

| 피킹 | 현재 코드 | 이번 판단 |
|---|---|---|
| navigation paint | resolved height면 blocked cell도 선택 | 유지. blocked를 다시 walkable로 지워야 함 |
| MapTool playerSpawn/NPC/boss | raw map pick | 현재 non-walkable gate 없음 |
| 제품 우클릭 | 현재 Character Y의 평면 ray pick | Server navigation이 최종 권위 |
| Character Select preview | 입력/navigation 없음 | 그대로 유지 |

non-walkable cell은 피킹하지 않는다는 규칙을 navigation paint에 적용하면 blocked cell을 해제할 수 없다. gameplay placement의 walkable gate는 필요한 기능이지만 Character Select에는 gameplay layer 자체가 없으므로 이번 수직 슬라이스와 분리한다.

## 3. 독립 비평과 최종 반영

| 우선순위 | 비평 | 최종 계획 |
|---|---|---|
| P0 | Character Select client navgrid는 소비자가 없고 MapTool 재진입 시 Layer_Player/Com_Navigation 탐색도 실패 | authoring-only. Client/Server navgrid 모두 금지 |
| P0 | asset ID admission은 같은 asset의 다른 placement를 섞음 | stable numeric placementId + expected assetId exact pair |
| P0 | stale transform을 탐지할 provenance가 없음 | sorted placementId + assetId + transform digest를 navsource에 기록 |
| P0 | missing Gameplay.world.json을 empty로 열고 저장 가능 | typed World policy가 NONE이면 panel/load/save/create/edit 전부 차단 |
| P1 | MapCatalog를 Client가 소비하지 않음 | LevelRegistry typed policy를 Client가 소비하고 ProjectAudit가 MapCatalog와 parity 검사 |
| P1 | declared admission invalid 시 all-visible fallback 위험 | Character Select는 hard fail. legacy Area만 기존 fallback 유지 |
| P1 | 범용 publisher와 runtime transaction은 범위가 큼 | 이번 계획에서 제거. Valtan/Training publisher는 변경하지 않음 |
| P2 | highest Y + abs(normalY)는 admitted model 내부 장식까지 완전히 제거하지 못함 | down-facing/겹침 harness와 overlay를 완료 gate에 추가 |
| 확인 | Lobby Test → editor → F1 → Map Tool 진입 가능 여부 | 실제 코드와 일치. F1은 tool-only이고 Debug 전용임을 인계에 명시 |

초안에서 폐기한 항목:

- Character Select navigationTargets client
- Character Select navigationRuntime
- Character Select runtime prototype
- client-only publisher 분기
- asset ID 단위 admission
- CUL_BOX 자동 bounds
- 이번 변경에서 범용 gameplay placement projection 수정

## 4. 최종 목표와 비목표

### 4.1 목표

    Debug Character Select Level
    -> F1 Developer Tools
    -> Map Tool
    -> AUTHORING_ONLY navigation 선언 확인
    -> placement별 bake admission 작성/로드
    -> white Nav Bounds 배치
    -> admitted placement triangle만 bake
    -> source/cell overlay
    -> walkability paint
    -> source/paint 저장
    -> Level 재진입
    -> same admission digest / source hash / cell statistics

생성할 정본:

    Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navbake
    Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navsource
    Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navpaint

없어야 하는 파일:

    Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navblockers
    Client/Bin/DataFiles/Navigation/LV_LOBBY_CLASSSELECT_SL00.navgrid
    Server/Bin/DataFiles/Navigation/LV_LOBBY_CLASSSELECT_SL00.navgrid
    Data/Worlds/LV_LOBBY_CLASSSELECT_SL00/Gameplay.world.json

### 4.2 비목표

- Character Select preview click-to-move
- preview Character의 pNavigationPrototypeTag 변경
- CUL_BOX surface bake
- 새 LEVEL
- 발탄/수련장 publisher 개편
- 일반 Monster placeholder
- NPC product presentation
- 이름에 FLOOR가 포함되면 자동 승인하는 heuristic
- 모든 Area의 gameplay placement walkable gate

## 5. 자료구조와 포맷

### 5.1 typed Area policy

추가 enum:

    enum class CLIENT_MAP_NAVIGATION_POLICY : uint8_t
    {
        NONE,
        AUTHORING_ONLY,
        RUNTIME_REQUIRED,
    };

    enum class CLIENT_WORLD_GAMEPLAY_POLICY : uint8_t
    {
        NONE,
        AUTHORING_REQUIRED,
    };

CLIENT_LEVEL_DESCRIPTOR 전체 계약:

    struct CLIENT_LEVEL_DESCRIPTOR final
    {
        using CREATE_FUNCTION = unique_ptr<Engine::CLevel>(*)(
            ComPtr<ID3D11Device>,
            ComPtr<ID3D11DeviceContext>);
        using LOAD_FUNCTION = HRESULT (CLoader::*)();

        LEVEL eLevel = LEVEL::END;
        CLIENT_LEVEL_KIND eKind = CLIENT_LEVEL_KIND::PRODUCT;
        const char_t* pStableId = nullptr;
        const char_t* pMapAreaId = nullptr;
        MAP_LOAD_SCOPE MapLoadScope{};
        CLIENT_MAP_NAVIGATION_POLICY eNavigationPolicy =
            CLIENT_MAP_NAVIGATION_POLICY::NONE;
        CLIENT_WORLD_GAMEPLAY_POLICY eWorldGameplayPolicy =
            CLIENT_WORLD_GAMEPLAY_POLICY::NONE;
        CREATE_FUNCTION pCreate = nullptr;
        LOAD_FUNCTION pLoad = nullptr;
    };

등록값:

| Level | Navigation | World gameplay |
|---|---|---|
| Lobby | NONE | NONE |
| Character Select | AUTHORING_ONLY | NONE |
| Bern | NONE | AUTHORING_REQUIRED |
| Valtan | RUNTIME_REQUIRED | AUTHORING_REQUIRED |
| Development training | RUNTIME_REQUIRED | AUTHORING_REQUIRED |

Loader:

- NONE과 AUTHORING_ONLY는 제품 runtime navigation prototype을 만들지 않는다.
- RUNTIME_REQUIRED는 runtime navgrid가 없거나 decode 실패하면 Level load를 실패시킨다.

MapTool:

- AUTHORING_ONLY와 RUNTIME_REQUIRED만 Navigation panel을 연다.
- AUTHORING_ONLY는 Destruction Area, runtime export, runtime blocker registration을 닫는다.
- World policy NONE은 World panel/load/save/create/edit를 전부 닫는다.

### 5.2 exact bake admission

    struct NAVGRID_BAKE_ADMISSION_ENTRY final
    {
        uint64_t placementId = {};
        std::string expectedAssetId;
    };

저장 포맷:

    LOSTARK_NAVGRID_BAKE_ADMISSION 1 "LV_LOBBY_CLASSSELECT_SL00" <pairCount>
    <placementId> "<expectedAssetId>"

불변식:

- pairCount 1..현재 loaded placement count
- placementId 중복/0 금지
- 현재 placement 문서에 placementId가 정확히 한 번 존재
- 현재 placement assetId와 expectedAssetId exact match
- 다른 Area, invisible, non-batch, LV_NAVIMESH, CUL_BOX 거부
- admitted placement 하나라도 누락되면 전체 bake 실패
- admission에 없는 placement는 같은 asset이어도 bake 입력이 아님

MapTool UI:

- Hierarchy에서 placement를 선택한다.
- Inspector의 Use for Navigation Bake checkbox로 numeric placementId를 admission set에 넣고 뺀다.
- Save Bake Admission은 temporary write, staged reload, atomic replace를 수행한다.
- Reload Bake Admission 실패 시 현재 live admission을 유지한다.
- Character Select에서 admission이 없거나 invalid하면 Bake 버튼을 disable한다. all-visible fallback은 없다.

현재 catalog에서 floor/bridge 16 asset type과 첫 bounds가 겹치는 placement 후보는 134개다. 이것은 최종 승인 개수가 아니라 첫 UI 검토 집합이다. 최종 navbake는 렌더된 floor를 확인한 numeric placement pair만 저장한다.

### 5.3 stale input digest

digest 입력:

    placementId
    assetId
    position xyz
    rotation quaternion xyzw
    signed scale xyz
    visible

정렬과 포맷:

    placementId 오름차순
    IEEE-754 bit pattern을 little-endian byte로 사용
    문자열은 UTF-8 + NUL
    FNV-1a 64-bit
    16자리 lowercase hex

MapTool은 현재 admission + current placement로 digest를 계산한다. navsource의 count/digest가 다르면 Source stale 상태로 표시하고 Save/paint를 막고 Rebake만 허용한다.

### 5.4 navsource v3 provenance

v1/v2 read compatibility는 유지한다. 신규 Character Select bake는 v3만 쓴다.

    LOSTARK_NAVGRID_SOURCE 3 "AreaId" width height cellSize originX originZ
    boundsPositionX boundsPositionY boundsPositionZ
    boundsSizeX boundsSizeY boundsSizeZ boundsYaw maxSlope
    admittedCount "admissionDigest" cellCount
    PLACEMENT placementId "assetId"
    CELL cellX cellZ surfaceResolved baseWalkable height

실제 header는 한 줄이다. 그 뒤 admittedCount개의 PLACEMENT row, cellCount개의 CELL row가 온다.

load 검증:

- header count와 실제 row count exact
- placement ID 오름차순/중복 금지
- PLACEMENT rows의 pair set이 navbake와 exact match
- current placement transform으로 재계산한 digest와 header digest match
- CELL 좌표 중복/누락 금지
- baseWalkable은 surfaceResolved보다 클 수 없음
- non-finite height 금지
- 실패 시 기존 document 유지

### 5.5 bake와 save

    struct NAVGRID_BAKE_PLACEMENT final
    {
        uint64_t placementId = {};
        std::string assetId;
        std::filesystem::path modelPath;
        float4x4_t world = {};
    };

    struct NAVGRID_BAKE_RESULT final
    {
        NAVGRID_AUTHORING_DESC desc;
        std::vector<NAV_SOURCE_CELL> cells;
        std::vector<NAVGRID_BAKE_ADMISSION_ENTRY> admittedPlacements;
        uint64_t admissionDigest = {};
        uint32_t placementCount = {};
        uint64_t triangleCount = {};
        uint32_t resolvedCellCount = {};
    };

Bake 흐름:

    navbake parse/validate
    -> exact placement collect
    -> CNavGridBaker::Build
    -> existing source/paint backup
    -> new source temporary write
    -> incompatible paint reset
    -> source/paint staged reload
    -> 실패 시 backup byte restore
    -> 성공 시 live document commit

Character Select AUTHORING_ONLY에서는 blocker/runtime을 쓰지 않는다. Walkability Save는 navpaint의 기존 temporary atomic replace를 사용한다. 범용 multi-target publisher나 Valtan runtime transaction은 수정하지 않는다.

### 5.6 Bounds와 raster

첫 검토 시작값:

    position: (-772.017, -143.0, 197.538)
    size:     (31.0, 4.0, 31.0)
    yaw:      0.0 degrees
    cellSize: 0.5
    maxSlope: 50.0 degrees

이 값은 완료 증거가 아니다. RESULT에 다음 실측을 기록한다.

- 최종 admitted placement count
- 제외한 floor candidate와 이유
- resolved/walkable/blocked/no-surface cell count
- resolved Y min/max
- preview 중심 cell state/Y
- 원형 둘레 8방향 sample cell state
- down-facing flat triangle count 또는 대응 harness 결과
- admission digest
- navbake/source/paint SHA-256

CUL_BOX는 white bounds가 원형 경계와 맞는지 보는 overlay 참고다. 자동 bounds source나 triangle 입력으로 사용하지 않는다.

## 6. 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | C:/Users/user/Desktop/LostArk/Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navbake | placement admission |
| 생성 | C:/Users/user/Desktop/LostArk/Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navsource | v3 bake/provenance |
| 생성 | C:/Users/user/Desktop/LostArk/Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navpaint | static blocked cells |
| 수정 | C:/Users/user/Desktop/LostArk/Data/Maps/MapCatalog.json | authoring-only tool/audit manifest |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Public/LevelRegistry.h | typed navigation/world policy |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Private/LevelRegistry.cpp | Level별 policy |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Public/NavGridPaintDocument.h | v3 provenance load |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Private/NavGridPaintDocument.cpp | v1/v2/v3 staged validation |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Public/NavGridBaker.h | placement provenance |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Private/NavGridBaker.cpp | v3 serialize/digest |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h | admission/policy state |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp | admission UI, exact collect, policy gate |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Public/Loader.h | Ready_MapArea policy parameter |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp | runtime-required only load |
| 수정 | C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1 | authoring-only/absence/provenance audit |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj | 세 Data 원본 None 등록 |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters | 96.DataFiles/Navigation 등록 |
| 수정 | C:/Users/user/Desktop/LostArk/.md/TEAM/AREA_DATA_LAYER_GUIDE.md | Character Select 상태 |
| 수정 | C:/Users/user/Desktop/LostArk/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md | paint/gameplay pick 의미 |
| 추가 | C:/Users/user/Desktop/LostArk/.md/GB/08-04/2026-08-04_CHARACTER_SELECT_NAVIGATION_AUTHORING_RESULT.md | 승인 후 결과 |

새 C++ 파일은 없다. ClInclude/ClCompile 등록 변경은 없다.

## 7. 파일별 핵심 반영 코드

계획 승인 후 적용할 public 계약이다. 현재 작업 트리에는 반영하지 않는다.

### 7.1 Character Select descriptor

    {
        LEVEL::CHARACTER_SELECT,
        CLIENT_LEVEL_KIND::PRODUCT,
        "front.character-select",
        "LV_LOBBY_CLASSSELECT_SL00",
        { true, true, -792.f, 158.f, -750.f, 218.f },
        CLIENT_MAP_NAVIGATION_POLICY::AUTHORING_ONLY,
        CLIENT_WORLD_GAMEPLAY_POLICY::NONE,
        CreateCharacterSelect,
        &CLoader::Ready_For_CharacterSelect
    },

### 7.2 Loader signature

    HRESULT Ready_MapArea(
        uint32_t iLevelIndex,
        const std::string& areaId,
        const MAP_LOAD_SCOPE& loadScope,
        CLIENT_MAP_NAVIGATION_POLICY navigationPolicy);

Loader의 navigation prototype block은 RUNTIME_REQUIRED일 때만 실행한다. NONE/AUTHORING_ONLY에서 stray runtime 파일이 있어도 prototype을 등록하지 않는다.

### 7.3 MapTool policy gate

Handle_LevelTransition이 descriptor policy를 live tool state에 commit한다.

    m_eNavigationPolicy = pDescriptor->eNavigationPolicy;
    m_eWorldGameplayPolicy = pDescriptor->eWorldGameplayPolicy;

    if (CLIENT_WORLD_GAMEPLAY_POLICY::AUTHORING_REQUIRED ==
        m_eWorldGameplayPolicy)
    {
        Load_WorldGameplay();
    }
    else
    {
        m_WorldGameplayDocument = CWorldGameplayDocument{};
        m_bWorldGameplayDirty = false;
        m_bWorldGameplayPlacementArmed = false;
        m_SelectedWorldPlacementId.clear();
        m_WorldGameplayStatus =
            "World gameplay layer is not declared for this Area";
    }

Load_WorldGameplay, Save_WorldGameplay, Try_PlaceWorldGameplay도 UI와 독립적으로 policy를 검사한다. Character Select에서는 missing file을 empty-success로 해석하지 않는다.

### 7.4 exact placement collect

    const auto admitted =
        m_NavigationBakeAdmission.find(
            entry.record.placementId);
    if (m_NavigationBakeAdmission.end() == admitted ||
        admitted->second != entry.record.assetId)
    {
        continue;
    }

수집 종료 후 found placementId 수와 admission 수가 다르면 전체 bake를 실패시킨다. visible/non-batch/CUL 검증도 수집 전에 수행한다.

### 7.5 navigation paint pick

Try_PickNavigationCell의 마지막 조건은 유지한다.

    return m_NavigationDocument.Has_ResolvedHeight(
        m_NavigationDocument.To_Index(
            outCellX,
            outCellZ));

Get_CellState == WALKABLE 조건을 추가하지 않는다. blocked cell을 Erase로 다시 walkable로 바꾸기 위해서다.

### 7.6 MapCatalog Character Select row

    {
      "id": "LV_LOBBY_CLASSSELECT_SL00",
      "kind": "product",
      "catalogType": "single",
      "sourceCatalog": "Data/Maps/Imported/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapassets",
      "sourcePlacements": "Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements",
      "catalog": "Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapassets",
      "placements": "Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements",
      "navigationMode": "authoring-only",
      "navigationBakeAdmission": "Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navbake",
      "navigationSource": "Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navsource",
      "navigationPaint": "Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navpaint",
      "placementCount": 803,
      "assetCount": 55,
      "runtimeAssetRoot": "Map/CHARACTERSELECTMAP"
    }

MapCatalog는 tool/audit manifest다. 제품 Client가 repository Data/Maps/MapCatalog.json을 읽는 새 runtime 경로는 만들지 않는다. ProjectAudit가 이 row와 LevelRegistry typed policy 및 convention-derived Data/Navigation 경로를 맞춘다.

navigationBlockers, navigationRuntime, gameplayDocument는 넣지 않는다.

## 8. MapTool 담당자 인계

### 8.1 Character Select bake 순서

1. Debug Client에서 Lobby → Test로 Map Editor Workspace에 진입한다.
2. F1 → Map Tool → Navigation을 연다.
3. Area LV_LOBBY_CLASSSELECT_SL00, policy AUTHORING_ONLY를 확인한다.
4. Hierarchy에서 실제 floor/bridge placement를 선택하고 Use for Navigation Bake를 켠다.
5. Save Bake Admission 후 numeric placement count와 digest를 확인한다.
6. Place Nav Bounds로 중앙을 찍고 5.6의 시작값을 넣는다.
7. Bake한다.
8. 중앙/연결 바닥 green, 외곽/no-surface/급경사 yellow를 확인한다.
9. 잘못된 상층 Y가 보이면 paint로 덮지 말고 해당 placement admission을 제거하고 다시 bake한다.
10. 실제 이동 금지인 resolved floor만 paint로 block한다.
11. Save Navigation으로 navpaint를 저장한다.
12. Character Select를 나갔다 재진입해 admission digest, source/paint hash, cell 통계와 overlay가 같은지 확인한다.

### 8.2 데이터 역할

- visual: Data/Maps/Authoring/AreaId
- walkable surface/Y: Data/Navigation/AreaId.navsource
- static block: Data/Navigation/AreaId.navpaint
- playerSpawn/NPC/boss: Data/Worlds/AreaId/Gameplay.world.json
- boss definition: Data/Actors/BossCatalog.json
- boss balance: Data/Balance/BossProfiles.json
- encounter: Data/Encounters/Boss

Character Select는 visual과 authoring navigation만 갖는다. World Gameplay panel은 닫힌다.

Valtan의 playerSpawn/boss는 publisher가 navigation walkable과 Y 오차를 검사한다. NPC는 저장 kind가 있으나 catalog와 product presentation이 미완료다. Monster는 저장 kind부터 없으며 별도 수직 슬라이스 전에는 추가하지 않는다.

### 8.3 후속 독립 작업

MapTool gameplay placement를 non-walkable cell에서 거부하는 기능은 별도 PLAN으로 분리한다.

그 후속 작업은 다음을 함께 닫아야 한다.

- MapCatalog/LevelRegistry의 runtime-navigation declared 여부
- create뿐 아니라 load, DragFloat3 edit, enable, save validation
- declared-but-unavailable hard reject
- no-navigation Bern만 raw surface pick 허용
- nav Y snap
- Valtan/Training world publisher 회귀

Character Select에는 world layer가 없으므로 이 후속 작업을 먼저 섞지 않는다.

## 9. 적용 순서

1. LevelRegistry에 typed navigation/world policy를 추가한다.
2. Loader에서 RUNTIME_REQUIRED만 runtime prototype을 등록한다.
3. MapTool World/Navigation panel을 policy로 gate한다.
4. placement admission UI/parser를 추가한다.
5. baker/source를 numeric placement provenance 포함 v3로 확장한다.
6. Character Select navbake를 MapTool로 작성한다.
7. first bake 후 navsource/navpaint를 저장한다.
8. MapCatalog, project/filter, ProjectAudit, 팀 문서를 갱신한다.
9. 실제 검증은 승인 후 RESULT에만 기록한다.

## 10. 검증 계획

### 10.1 admission/source

- placementId 0, duplicate, unknown, other Area 거부
- expected asset mismatch 거부
- invisible/non-batch/CUL admission 거부
- same asset의 admitted/non-admitted 겹침에서 admitted placement만 사용
- admitted placement transform 변경 시 stale digest
- navsource v1 Valtan load 호환
- navsource v2 기존 MapTool load 호환
- v3 count/digest/pair/cell 변조 거부

### 10.2 bake/rollback

- source temporary write 실패
- incompatible paint reset 실패
- staged source/paint reload 실패
- backup restore 실패를 별도 오류로 보존
- 각 정상 rollback에서 기존 source/paint byte hash 유지
- stage/backup 잔여물 0

### 10.3 geometry 잔여 위험

- admitted lower floor와 non-admitted higher decor가 같은 cell에 겹치는 fixture
- 같은 asset의 admitted/non-admitted placement가 겹치는 fixture
- down-facing flat triangle fixture
- 최고 Y 선택 결과와 slope 결과 기록

### 10.4 ProjectAudit

- navigationMode authoring-only
- navbake/source/paint 존재와 Area ID 일치
- MapCatalog ↔ LevelRegistry policy parity
- Character Select navblockers/client navgrid/server navgrid 부재
- Character Select Gameplay.world.json 부재
- preview pNavigationPrototypeTag=null
- Valtan/Training publisher 파일과 script 미변경

### 10.5 build/runtime

    powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
    powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
    powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1 -DeepAssetHash
    git diff --check

Debug 수동 smoke:

    Character Select -> F1 -> Map Tool
    AUTHORING_ONLY 표시
    World Gameplay panel disabled
    admission count/digest 표시
    Bake -> overlay
    blocked cell Apply -> Erase 가능
    Save -> 재진입 -> same digest/hash/cell stats
    Client/Server Character Select navgrid 없음
    Character preview 다섯 class 전환 유지

Release에서는 Debug MapTool smoke를 PASS로 기록하지 않는다. 제품 Character Select 진입과 preview 회귀만 확인한다.

## 11. 승인 전 검토 포인트

1. Character Select는 authoring-only navigation이며 Client/Server runtime을 만들지 않는다.
2. admission은 asset ID가 아니라 stable numeric placementId + expectedAssetId pair다.
3. CUL_BOX는 surface도 자동 bounds 정본도 아니다.
4. Character Select World Gameplay panel은 완전히 비활성화한다.
5. navigation paint는 blocked cell 선택을 허용한다.
6. gameplay placement walkable gate는 후속 독립 작업으로 분리한다.
7. 제품 우클릭 client-side non-walkable 차단은 이번 범위 밖이다.
8. Monster/NPC product 확장은 이번 범위 밖이다.

이 여덟 항목을 사용자가 승인한 뒤에만 구현과 실제 bake를 시작한다.
