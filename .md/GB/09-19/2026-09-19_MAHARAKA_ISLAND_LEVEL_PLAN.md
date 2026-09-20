# 마하라카 파라다이스 섬 레벨 추가 PLAN

작성일: 2026-09-19
브랜치: `feature/maharaka-island-level` (origin/main `8f15a3c3`에서 분기, 미커밋 작업본)

작업 중 다른 세션이 작업 폴더를 `main`으로 전환하면서 미커밋 변경을 `stash@{0}`("backup before switching to main d2d3563b (Maharaka/EOL uncommitted, 2026-09-19)")에 백업했다. 이 브랜치는 그 stash에서 마하라카 파일 19개의 변경분만 3-way로 다시 적용한 것이다. `Framework.sln` 변경은 이 작업의 것이 아니라 적용하지 않았고, stash는 백업으로 남겨 두었다.

## 목표와 종료 증거

로비의 Debug 스테이지 패널에 `Maharaka` 버튼을 추가한다. 버튼을 누르면 Server 승인(`WORLD_ID::MAHARAKA`)을 받아
새 `LEVEL::MAHARAKA`로 진입하고, 원본 2021 마하라카 파라다이스 섬(`LV_OCN_EventIS_MHP`)의 섬 배치 전부가
원본 배치 데이터 그대로 자동 배치되어 보인다.

종료 증거는 다음과 같이 구분한다.

- 자동: 도구 테스트, 데이터 Validate, 게시(Publish), 사용자 Product 빌드 성공
- 사용자: 로컬 Server+Client 실행 후 로비 `Maharaka` → 섬 진입 화면 확인. 에이전트는 Client를 실행하거나 화면을 판정하지 않는다.

## 확정한 설계

| 항목 | 결정 | 근거 |
|---|---|---|
| Area ID | `LV_OCN_EVENTIS_MHP` | 기존 Area 관례(대문자 레벨 접두사). 원본 존 57009, 레벨 `LV_OCN_EventIS_MHP_PS` |
| 섬 범위 | 원본 좌표 y > 50000cm | 같은 레벨에 레이싱 트랙(y 0~350m)이 함께 있다. 섬은 y 약 913~1071m |
| 포함 배치 | SL01 3,820 + LAND01 섬 쪽 2(중앙 수영장) + PS 하늘 돔 1 = 3,823 | PS 숨김 헬퍼 243개와 레이싱 LAND01 80개는 제외 |
| 제외 배치 | SL01 `bg_tot_movillage_decoprop07f_sm_artree` 3개 | 원본 glTF normal/tangent가 평행. 저장소에 native parallel 증거 생성기가 없어 geometry contract를 통과할 수 없다 |
| 지면 | LAND01 랜드스케이프 46조각 중 섬 16조각 | 섬 바닥은 랜드스케이프다. 16조각 높이 14~22m, 섬 바닥 20.48m |
| 랜드스케이프 설치 폴더 | `Map/LV_OCN_EVENTIS_MHP_LAND` | 변형 설치 폴더는 소유 영수증(CAS)이 영수증 밖 파일을 거부한다. Bern도 별도 `_T` 폴더를 쓴다 |
| Client Level | 기존 `CLevel_Development`를 `LEVEL` 매개변수로 재사용 | 새 .cpp/.h가 없어 vcxproj/filters 변경이 없다. Map Editor 분기는 DEVELOPMENT에서만 연다 |
| 렌더링 프로필 | `scene.development.neutral.v1` 재사용 | Bern 프로필은 Bern 좌표의 환경 영역 5개를 가진다 |
| World | `WORLD_ID::MAHARAKA = 6`, shared simulation | Bern과 같은 공유 월드 방식 |
| Protocol | 93 → **94** | 이전 월드 추가 3건은 모두 버전을 올렸다. main은 91(관문 진행 패킷)과 93(쿠크 레이드 준비·Sequence clock)까지 사용했다. 같은 번호를 다른 wire에 쓰지 않도록 main 최신 번호 다음을 쓴다 |
| Navigation | 160×160 균일 격자, 높이 20.48m | Client replication이 navgrid를 필수로 요구한다. MapTool bake는 후속 작업이다 |
| Spawn | 광장 20.48m 바닥의 원본 Spot 4곳 | 균일 격자는 spawn 높이 차 0.25m 이하를 요구한다 |

## G1. 맵 에셋 추출·cook (저장소 밖 작업 폴더)

작업 폴더는 `C:\LostArkExtract\LV_OCN_EVENTIS_MHP_20260919`이다. cook 출력은 경로 길이(260자) 때문에
`C:\LostArkExtract\MHPrt`로 분리한다.

1. 배치 추출: `extract_ue3_placements.py ... LV_OCN_EVENTIS_MHP_PS LV_OCN_EVENTIS_MHP_LAND01 LV_OCN_EVENTIS_MHP_SL01`
2. 섬 범위 승인 사본: `Placements_Admitted/` + 제외 기록 `admission.exclusions.json`
3. `build_map_material_variants.py inventory --expect-packages 3 --expect-source-meshes 303 --expect-variants 382 --expect-placements 3823 --expect-override-placements 2926`
4. `extract` → `hydrate-catalog` → `cook --expect-variants 382 --package-root <Packages> --force`
5. 랜드스케이프: `extract_ue3_landscape.py --area-id LV_OCN_EVENTIS_MHP_LANDSCAPE --pack-name LV_OCN_EVENTIS_MHP_LAND --expect-components 46 LV_OCN_EVENTIS_MHP_LAND01` 후 섬 16조각만 필터

### G1 도구 수정 (`Tools/LevelPlacementExtractor/build_map_material_variants.py`)

이 섬에서 처음 드러난 결함 네 가지를 최소 수정한다.

- 부모 재질 경로: UModel은 패키지 루트의 부모를 이름만 적는다(`zzzbg_simple_opa_inst`). 점(.) 필수 검사를 비어 있지 않은 문자열 검사로 바꾸고, 식별은 기존 `contextual_candidate`의 정확히-하나 규칙에 맡긴다.
- 텍스처 수화 판정: 베이스 추출기는 역할 텍스처만 팩에 복사하므로 UModel이 내보낸 텍스처도 팩에 없을 수 있다. 수화 대상 판정을 "UModel이 내보냄"에서 "팩 `textures` 폴더에 실제로 있음"으로 바꾼다.
- `cook` 명령의 `--package-root`: 커밋 560741ac가 `args.package_root`를 넘기면서 인자 정의를 빠뜨렸다. 베이스 cook과 같은 선택 인자로 추가한다.
- 범위 밖 재질 덮어쓰기: UE3는 메시 요소 수만큼만 component override를 조회한다. 메시 슬롯 수를 넘는 항목은 적용하지 않고 receipt의 `ignoredOutOfRangeOverrides`에 남긴다.

테스트는 기존 기대값을 새 판정에 맞게 바꾸고 `test_exported_texture_absent_from_pack_is_a_hydration_input`를 추가한다.

## G2. Area 데이터

1. `build_map_material_variants.py install --runtime-manifest MHPrt/manifests/map_material_runtime_assets.json --runtime-root MHPrt/runtime --resources-root Client/Bin/Resources --area-id LV_OCN_EVENTIS_MHP --expect-variants 382 --allow-partial-material-preview`
2. `build_maptool_scene.py --area-id LV_OCN_EVENTIS_MHP --asset-manifest admitted.inventory.json --runtime-manifest ... --runtime-root MHPrt/runtime --placements-dir Placements_Admitted --catalog-output Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapassets --placement-output .../LV_OCN_EVENTIS_MHP.mapplacements --receipt-output .../LV_OCN_EVENTIS_MHP.build.receipt.json --allow-partial-material-preview`
3. `merge_maptool_landscape.py`로 섬 랜드스케이프 16조각을 같은 catalog/placement에 병합하고, 랜드스케이프 wmodel을 `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP_LAND`에 둔다.
4. `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapplacements` = 병합 placement
5. `Data/Maps/MapCatalog.json`에 single catalog 행을 추가한다(navigation/gameplay 문서 포함).
6. 게시: `Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Publish`

## G3. World·Navigation 데이터

- `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json`: playerSpawn 4개, 높이 20.48
- `Data/Navigation/LV_OCN_EVENTIS_MHP.navgrid.json`: 160×160, cell 1m, origin (0, -1072), 높이 20.48
- `Publish-WorldGameplay.ps1`: ALL 분기에 `LV_OCN_EVENTIS_MHP`/`MAHARAKA` 3줄
- `Publish-ServerNavigation.ps1`: AreaId ValidateSet와 `$gridFactories`에 균일 격자 항목

게시 명령:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Publish -AreaId LV_OCN_EVENTIS_MHP
```

## G4. Shared·Server

- `PacketType.h`: `WORLD_ID::MAHARAKA = 6`, `Is_Known_World_Id`, protocol 94
- `WorldBootstrap.cpp`, `SpawnGroupBootstrap.cpp`: `World_ToString`의 `MAHARAKA`
- `ServerApp.cpp`: `stageSharedSimulation(WORLD_ID::MAHARAKA)`와 시작 로그
- `GameRoom.cpp`: navigation 필수 월드 목록에 MAHARAKA

## G5. Client

- `Client_Defines.h`: `LEVEL::MAHARAKA` (DEVELOPMENT 뒤, END 앞)
- `LevelRegistry.cpp`: `CreateMaharaka`와 descriptor(배열 6→7)
- `Loader.h/.cpp`: `Ready_For_Maharaka` (맵 + 선택 class 캐릭터, NPC placement 서비스 초기화)
- `Level_Development.h/.cpp`: `m_eLevel` 멤버와 `Create(..., LEVEL)`; 모든 레벨 인덱스를 `m_eLevel`로, Map Editor는 DEVELOPMENT에서만
- `ClickMoveEffect.cpp`: 클릭 마커 허용 레벨에 MAHARAKA
- `LobbyCommandService.h`: `LOBBY_STAGE::MAHARAKA`
- `Level_Lobby.cpp`: `Resolve_Stage` case와 `Maharaka` 버튼(Debug 패널)
- `CharacterSelectionState.cpp`: MAHARAKA는 Valtan·Kouku와 같은 created-or-audition identity

범위 밖으로 둔 것: MainApp HUD 게이트(아이템 강화·저작 HUD·보스 게이지 표시 레벨 목록), Release Character Select의 스테이지 진입, Map Editor Area 목록, BGM, 미니맵, 로딩 화면 문구.

## 검증 순서

1. `python -B -m unittest discover -s Tools/LevelPlacementExtractor -p test_build_map_material_variants.py`
2. `Publish-WorldGameplay.ps1 -Mode Validate`, `Publish-ServerNavigation.ps1 -Mode Validate -AreaId LV_OCN_EVENTIS_MHP`
3. Visual Studio를 닫은 뒤 위 게시 명령과 `Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Publish`
4. 사용자 Product 빌드(Engine → Shared → Server → Client)
5. 로컬 Server(이 작업본으로 빌드한 protocol 94)와 Client로 로비 → `Maharaka` 진입. 팀 서버(main)는 MAHARAKA가 없고 protocol도 달라서 이 버튼으로 들어갈 수 없다.

## 전체 반영 코드 (코드 변경 전체 diff)

아래 diff가 이번 작업의 C++/Shared/Server/Tools 변경 전체다. 데이터 파일(G2/G3)은 생성물 또는 전문을 RESULT에 기록한다.

```diff
diff --git a/Client/Private/CharacterSelectionState.cpp b/Client/Private/CharacterSelectionState.cpp
index c96768e1..e080ddfb 100644
--- a/Client/Private/CharacterSelectionState.cpp
+++ b/Client/Private/CharacterSelectionState.cpp
@@ -140,6 +140,7 @@ bool_t Client::CCharacterSelectionState::Try_Resolve_ForWorld(
 
 	case WORLD_ID::VALTAN_ARENA:
 	case WORLD_ID::KAKULSAYDON_ARENA:
+	case WORLD_ID::MAHARAKA:
 		staged.eCharacterClass = g_SelectedClass.value_or(
 			CHARACTER_CLASS_ID::LANCE_MASTER);
 		if (g_CreatedNickname.has_value())
diff --git a/Client/Private/ClickMoveEffect.cpp b/Client/Private/ClickMoveEffect.cpp
index 343b71fc..92181e4b 100644
--- a/Client/Private/ClickMoveEffect.cpp
+++ b/Client/Private/ClickMoveEffect.cpp
@@ -18,7 +18,7 @@ bool_t Client::CClickMoveEffect::Uses_LevelMarkers(const LEVEL level)
 {
 	return LEVEL::CHARACTER_SELECT == level || LEVEL::BERN == level ||
 		LEVEL::VALTAN_ARENA == level || LEVEL::KAKULSAYDON_ARENA == level ||
-		LEVEL::DEVELOPMENT == level;
+		LEVEL::DEVELOPMENT == level || LEVEL::MAHARAKA == level;
 }
 
 std::vector<std::string> Client::CClickMoveEffect::Queue_LevelResources(const LEVEL level)
diff --git a/Client/Private/LevelRegistry.cpp b/Client/Private/LevelRegistry.cpp
index 21efb9c8..a3ea515e 100644
--- a/Client/Private/LevelRegistry.cpp
+++ b/Client/Private/LevelRegistry.cpp
@@ -100,12 +100,21 @@ namespace
 		return CLevel_Development::Create(pDevice, pContext);
 	}
 
+	unique_ptr<CLevel> CreateMaharaka(
+		ComPtr<ID3D11Device> pDevice,
+		ComPtr<ID3D11DeviceContext> pContext)
+	{
+		// The island reuses the Development shell: map, follow camera, Server
+		// replication and the player controller, without the Map Editor branch.
+		return CLevel_Development::Create(pDevice, pContext, LEVEL::MAHARAKA);
+	}
+
 }
 
 const CLIENT_LEVEL_DESCRIPTOR* CLevelRegistry::Find(
 	const LEVEL eLevel)
 {
-	static const std::array<CLIENT_LEVEL_DESCRIPTOR, 6> levels =
+	static const std::array<CLIENT_LEVEL_DESCRIPTOR, 7> levels =
 	{{
 		{
 			LEVEL::LOBBY,
@@ -171,6 +180,18 @@ const CLIENT_LEVEL_DESCRIPTOR* CLevelRegistry::Find(
 			{ true, false, -20.f, -20.f, 20.f, 20.f },
 			CreateDevelopment,
 			&CLoader::Ready_For_Development
+		},
+		{
+			LEVEL::MAHARAKA,
+			CLIENT_LEVEL_KIND::PRODUCT,
+			"world.maharaka",
+			"LV_OCN_EVENTIS_MHP",
+			// The Bern profile carries Bern-space environment regions; the neutral
+			// profile keeps the island free of them until it gets its own.
+			"scene.development.neutral.v1",
+			MakeFullMapScope(),
+			CreateMaharaka,
+			&CLoader::Ready_For_Maharaka
 		}
 	}};
 
diff --git a/Client/Private/Level_Development.cpp b/Client/Private/Level_Development.cpp
index cab26d51..b07480df 100644
--- a/Client/Private/Level_Development.cpp
+++ b/Client/Private/Level_Development.cpp
@@ -18,8 +18,10 @@
 
 CLevel_Development::CLevel_Development(
 	ComPtr<ID3D11Device> pDevice,
-	ComPtr<ID3D11DeviceContext> pContext)
+	ComPtr<ID3D11DeviceContext> pContext,
+	const LEVEL eLevel)
 	: CLevel{ pDevice, pContext }
+	, m_eLevel{ eLevel }
 {
 }
 
@@ -41,6 +43,7 @@ HRESULT CLevel_Development::Initialize()
 
 #ifdef _DEBUG
 	m_isMapEditorWorkspace =
+		LEVEL::DEVELOPMENT == m_eLevel &&
 		CMapEditorWorkspaceService::Is_Requested();
 	if (m_isMapEditorWorkspace)
 	{
@@ -63,10 +66,10 @@ HRESULT CLevel_Development::Initialize()
 #endif
 
 	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
-		CLevelRegistry::Find(LEVEL::DEVELOPMENT);
+		CLevelRegistry::Find(m_eLevel);
 	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
 		!m_MapRuntime.Load_Area(
-			ETOUI(LEVEL::DEVELOPMENT),
+			ETOUI(m_eLevel),
 			pEntry->pMapAreaId,
 			pEntry->MapLoadScope))
 	{
@@ -85,8 +88,8 @@ HRESULT CLevel_Development::Initialize()
 	CClientReplication::DESC replicationDesc{};
 	replicationDesc.pDevice = m_pDevice;
 	replicationDesc.pContext = m_pContext;
-	replicationDesc.iPrototypeLevelIndex = ETOUI(LEVEL::DEVELOPMENT);
-	replicationDesc.iLayerLevelIndex = ETOUI(LEVEL::DEVELOPMENT);
+	replicationDesc.iPrototypeLevelIndex = ETOUI(m_eLevel);
+	replicationDesc.iLayerLevelIndex = ETOUI(m_eLevel);
 	replicationDesc.strMapAreaId = pEntry->pMapAreaId;
 	replicationDesc.strPlayerLayerTag = TEXT("Layer_Player");
 	replicationDesc.strWorldEntityLayerTag = TEXT("Layer_WorldEntity");
@@ -99,12 +102,12 @@ HRESULT CLevel_Development::Initialize()
 	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
 	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
 	if (!m_PlayerController.Initialize_TargetingPreview(
-			ETOUI(LEVEL::DEVELOPMENT)))
+			ETOUI(m_eLevel)))
 	{
 		return E_FAIL;
 	}
 	if (!m_PlayerController.Initialize_ClickMoveEffect(
-			ETOUI(LEVEL::DEVELOPMENT)))
+			ETOUI(m_eLevel)))
 	{
 		return E_FAIL;
 	}
@@ -157,6 +160,8 @@ HRESULT CLevel_Development::Render()
 #ifdef _DEBUG
 	CMainApp::Update_DebugWindowTitleWithFps(m_isMapEditorWorkspace ?
 		TEXT("LostArk Map Editor Workspace") :
+		LEVEL::MAHARAKA == m_eLevel ?
+		TEXT("LostArk Maharaka Paradise") :
 		TEXT("LostArk Test Training Ground"));
 #endif
 	return S_OK;
@@ -187,9 +192,9 @@ HRESULT CLevel_Development::Ready_Camera(
 
 	shared_ptr<CGameObject> gameObject;
 	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
-		ETOUI(LEVEL::DEVELOPMENT),
+		ETOUI(m_eLevel),
 		TEXT("Prototype_GameObject_Camera_Free"),
-		ETOUI(LEVEL::DEVELOPMENT),
+		ETOUI(m_eLevel),
 		strLayerTag,
 		&cameraDesc,
 		&gameObject)))
@@ -202,7 +207,7 @@ HRESULT CLevel_Development::Ready_Camera(
 	if (nullptr == camera)
 	{
 		CGameInstance::Get().Remove_GameObject_from_Layer(
-			ETOUI(LEVEL::DEVELOPMENT),
+			ETOUI(m_eLevel),
 			strLayerTag,
 			gameObject);
 		return E_FAIL;
@@ -244,10 +249,11 @@ bool_t CLevel_Development::Bind_CameraToLocalCharacter()
 
 unique_ptr<CLevel_Development> CLevel_Development::Create(
 	ComPtr<ID3D11Device> pDevice,
-	ComPtr<ID3D11DeviceContext> pContext)
+	ComPtr<ID3D11DeviceContext> pContext,
+	const LEVEL eLevel)
 {
 	auto instance = unique_ptr<CLevel_Development>(
-		new CLevel_Development(pDevice, pContext));
+		new CLevel_Development(pDevice, pContext, eLevel));
 	if (FAILED(instance->Initialize()))
 		return nullptr;
 	return instance;
diff --git a/Client/Private/Level_Lobby.cpp b/Client/Private/Level_Lobby.cpp
index 1339d10c..2379fa5c 100644
--- a/Client/Private/Level_Lobby.cpp
+++ b/Client/Private/Level_Lobby.cpp
@@ -343,6 +343,10 @@ bool_t CLevel_Lobby::Resolve_Stage(
 		outWorldId = WORLD_ID::BERN;
 		outTargetLevel = LEVEL::BERN;
 		return true;
+	case LOBBY_STAGE::MAHARAKA:
+		outWorldId = WORLD_ID::MAHARAKA;
+		outTargetLevel = LEVEL::MAHARAKA;
+		return true;
 	default:
 		return false;
 	}
@@ -507,6 +511,9 @@ void CLevel_Lobby::Render_StagePanel()
 	ImGui::SameLine();
 	if (ImGui::Button("Bern"))
 		CLobbyCommandService::Request(LOBBY_STAGE::BERN);
+	ImGui::SameLine();
+	if (ImGui::Button("Maharaka"))
+		CLobbyCommandService::Request(LOBBY_STAGE::MAHARAKA);
 	ImGui::EndDisabled();
 
 	ImGui::TextWrapped("%s", m_strStatus.c_str());
diff --git a/Client/Private/Loader.cpp b/Client/Private/Loader.cpp
index d9b083c8..3c5e660b 100644
--- a/Client/Private/Loader.cpp
+++ b/Client/Private/Loader.cpp
@@ -868,6 +868,46 @@ HRESULT CLoader::Ready_For_Development()
 	return S_OK;
 }
 
+HRESULT CLoader::Ready_For_Maharaka()
+{
+	CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::MAHARAKA));
+	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::MAHARAKA));
+	if (FAILED(CNpcPlacementPresentationService::Load(
+		ETOUI(LEVEL::MAHARAKA), "MAHARAKA")))
+	{
+		OutputDebugStringA(("[Loader][NpcPresentation] " +
+			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
+	}
+	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::MAHARAKA));
+	Declare_Phases(6u);
+	Set_Status(TEXT("MAHARAKA: island catalog and placements"));
+
+	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
+		CLevelRegistry::Find(LEVEL::MAHARAKA);
+	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
+		FAILED(Ready_MapArea(
+			ETOUI(LEVEL::MAHARAKA),
+			pEntry->pMapAreaId,
+			pEntry->MapLoadScope)))
+	{
+		return E_FAIL;
+	}
+
+	Set_Status(TEXT("MAHARAKA: session character bundle"));
+	const std::array selectedClass =
+	{
+		m_ePreparedCharacterClass
+	};
+	if (FAILED(Ready_Character_Rendering(
+		ETOUI(LEVEL::MAHARAKA),
+		selectedClass)))
+		return E_FAIL;
+
+	Set_Status(TEXT("Maharaka loading complete"));
+	rollback.Commit();
+	return S_OK;
+}
+
 HRESULT CLoader::Ready_MapArea(
 	const uint32_t iLevelIndex,
 	const std::string& areaId,
diff --git a/Client/Public/Client_Defines.h b/Client/Public/Client_Defines.h
index 69119494..cd45783f 100644
--- a/Client/Public/Client_Defines.h
+++ b/Client/Public/Client_Defines.h
@@ -23,6 +23,7 @@ namespace Client
 		VALTAN_ARENA,
 		KAKULSAYDON_ARENA,
 		DEVELOPMENT,
+		MAHARAKA,
 		END
 	};
 }
diff --git a/Client/Public/Level_Development.h b/Client/Public/Level_Development.h
index bc32eff8..d76bb63a 100644
--- a/Client/Public/Level_Development.h
+++ b/Client/Public/Level_Development.h
@@ -17,7 +17,8 @@ class CLevel_Development final : public CLevel
 private:
 	CLevel_Development(
 		ComPtr<ID3D11Device> pDevice,
-		ComPtr<ID3D11DeviceContext> pContext);
+		ComPtr<ID3D11DeviceContext> pContext,
+		LEVEL eLevel);
 
 public:
 	virtual ~CLevel_Development();
@@ -33,6 +34,8 @@ private:
 	bool_t Bind_CameraToLocalCharacter();
 
 private:
+	// Registry entry this instance plays; only DEVELOPMENT may open the Map Editor.
+	LEVEL m_eLevel = LEVEL::DEVELOPMENT;
 	CMapPlacementRuntime m_MapRuntime;
 	bool_t m_isMapEditorWorkspace = false;
 	weak_ptr<CCamera_Free> m_pCamera;
@@ -44,7 +47,8 @@ private:
 public:
 	static unique_ptr<CLevel_Development> Create(
 		ComPtr<ID3D11Device> pDevice,
-		ComPtr<ID3D11DeviceContext> pContext);
+		ComPtr<ID3D11DeviceContext> pContext,
+		LEVEL eLevel = LEVEL::DEVELOPMENT);
 };
 
 NS_END
diff --git a/Client/Public/Loader.h b/Client/Public/Loader.h
index 57c1219a..3293510d 100644
--- a/Client/Public/Loader.h
+++ b/Client/Public/Loader.h
@@ -91,6 +91,7 @@ private:
 	HRESULT Ready_For_ValtanArena();
 	HRESULT Ready_For_KakulSaydonArena();
 	HRESULT Ready_For_Development();
+	HRESULT Ready_For_Maharaka();
 
 	HRESULT Ready_MapArea(
 		uint32_t iLevelIndex,
diff --git a/Client/Public/LobbyCommandService.h b/Client/Public/LobbyCommandService.h
index 8647b60b..b7812e55 100644
--- a/Client/Public/LobbyCommandService.h
+++ b/Client/Public/LobbyCommandService.h
@@ -14,6 +14,7 @@ enum class LOBBY_STAGE
 	VALTAN,
 	BERN,
 	KOUKU_SAYDON,
+	MAHARAKA,
 	END
 };
 
diff --git a/Server/Private/GameRoom.cpp b/Server/Private/GameRoom.cpp
index dc5f2d82..d47a4db4 100644
--- a/Server/Private/GameRoom.cpp
+++ b/Server/Private/GameRoom.cpp
@@ -83,6 +83,7 @@ LostArk::Server::CGameRoom::CGameRoom(
 		LostArk::Shared::WORLD_ID::TRAINING_GROUND == worldId ||
 		LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId ||
 		LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == worldId ||
+		LostArk::Shared::WORLD_ID::MAHARAKA == worldId ||
 		LostArk::Shared::WORLD_ID::BERN == worldId) &&
 		!m_ServerNavigation.Load(m_WorldBootstrap.Get_AreaId()))
 	{
diff --git a/Server/Private/ServerApp.cpp b/Server/Private/ServerApp.cpp
index d7bf588f..9160778a 100644
--- a/Server/Private/ServerApp.cpp
+++ b/Server/Private/ServerApp.cpp
@@ -2510,7 +2510,8 @@ int LostArk::Server::CServerApp::Run(
 	if (!stageSharedSimulation(WORLD_ID::BERN) ||
 		!stageSharedSimulation(WORLD_ID::VALTAN_ARENA) ||
 		!stageSharedSimulation(WORLD_ID::TRAINING_GROUND) ||
-		!stageSharedSimulation(WORLD_ID::KAKULSAYDON_ARENA))
+		!stageSharedSimulation(WORLD_ID::KAKULSAYDON_ARENA) ||
+		!stageSharedSimulation(WORLD_ID::MAHARAKA))
 	{
 		return 1;
 	}
@@ -2572,7 +2573,7 @@ int LostArk::Server::CServerApp::Run(
 		0 == ::_isatty(::_fileno(stdin));
 	std::cout << "Listening on " << bindAddress << ':' << port
 		<< " with shared BERN, VALTAN_ARENA, TRAINING_GROUND, "
-		<< "KAKULSAYDON_ARENA and "
+		<< "KAKULSAYDON_ARENA, MAHARAKA and "
 		<< "session-private CHARACTER_SELECT_ARENA simulations.";
 	if (0u == automaticShutdownMilliseconds && useHeadlessMode)
 	{
diff --git a/Server/Private/SpawnGroupBootstrap.cpp b/Server/Private/SpawnGroupBootstrap.cpp
index bc494d68..0beca978 100644
--- a/Server/Private/SpawnGroupBootstrap.cpp
+++ b/Server/Private/SpawnGroupBootstrap.cpp
@@ -31,6 +31,7 @@ namespace
 		case WORLD_ID::TRAINING_GROUND: return "TRAINING_GROUND";
 		case WORLD_ID::CHARACTER_SELECT_ARENA: return "CHARACTER_SELECT_ARENA";
 		case WORLD_ID::KAKULSAYDON_ARENA: return "KAKULSAYDON_ARENA";
+		case WORLD_ID::MAHARAKA: return "MAHARAKA";
 		default: return {};
 		}
 	}
diff --git a/Server/Private/WorldBootstrap.cpp b/Server/Private/WorldBootstrap.cpp
index 00b7ee4e..f2a36dcc 100644
--- a/Server/Private/WorldBootstrap.cpp
+++ b/Server/Private/WorldBootstrap.cpp
@@ -27,6 +27,7 @@ namespace
 			return "CHARACTER_SELECT_ARENA";
 		case WORLD_ID::KAKULSAYDON_ARENA:
 			return "KAKULSAYDON_ARENA";
+		case WORLD_ID::MAHARAKA: return "MAHARAKA";
 		default: return {};
 		}
 	}
diff --git a/Shared/Public/Network/PacketType.h b/Shared/Public/Network/PacketType.h
index 2e180ee7..fc69d128 100644
--- a/Shared/Public/Network/PacketType.h
+++ b/Shared/Public/Network/PacketType.h
@@ -84,7 +84,8 @@ namespace LostArk::Shared
 	// 89 adds the worn honor title to player snapshots and the title change request/verdict.
 	// 93 combines main gate-progress votes with the fixed-roster Kouku raid
 	// preparation and shared Sequence clock. Earlier peers are incompatible.
-	inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 93;
+	// 94 admits the Maharaka Paradise island as a Server-owned shared world.
+	inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 94;
 
 	enum class WORLD_ID : std::uint16_t
 	{
@@ -93,6 +94,7 @@ namespace LostArk::Shared
 		TRAINING_GROUND = 3,
 		CHARACTER_SELECT_ARENA = 4,
 		KAKULSAYDON_ARENA = 5,
+		MAHARAKA = 6,
 		END
 	};
 
@@ -103,7 +105,8 @@ namespace LostArk::Shared
 			WORLD_ID::VALTAN_ARENA == worldId ||
 			WORLD_ID::TRAINING_GROUND == worldId ||
 			WORLD_ID::CHARACTER_SELECT_ARENA == worldId ||
-			WORLD_ID::KAKULSAYDON_ARENA == worldId;
+			WORLD_ID::KAKULSAYDON_ARENA == worldId ||
+			WORLD_ID::MAHARAKA == worldId;
 	}
 
 	enum class CHARACTER_CLASS_ID : std::uint8_t
diff --git a/Tools/LevelPlacementExtractor/build_map_material_variants.py b/Tools/LevelPlacementExtractor/build_map_material_variants.py
index 761283bc..7be30634 100644
--- a/Tools/LevelPlacementExtractor/build_map_material_variants.py
+++ b/Tools/LevelPlacementExtractor/build_map_material_variants.py
@@ -545,9 +545,11 @@ def resolve_material_contracts(
         document = parse_exact_material_document(props_path)
         parent = document.get("parent")
         if parent:
-            if not isinstance(parent, str) or "." not in parent:
+            # UModel writes a package-root parent as its bare object name; the
+            # exactly-one rule in contextual_candidate still decides identity.
+            if not isinstance(parent, str) or not parent.strip():
                 raise VariantError(
-                    f"material parent is not a full canonical objectPath: {parent!r}"
+                    f"material parent is not a valid objectPath: {parent!r}"
                 )
             inherited = visit(parent, None, canonical_object_path)
             contract = {
@@ -765,17 +767,24 @@ def missing_loaded_texture_sources(source_roots: Sequence[Path]) -> list[dict[st
             if not log_path.is_file():
                 continue
             text = log_path.read_text(encoding="utf-8", errors="replace")
-            exported = {
-                row["objectName"].casefold()
-                for row in parse_umodel_export_log(log_path)
-                if row["class"] == "Texture2D"
-            }
+            # A base mesh pack keeps only role textures, so a texture UModel
+            # exported may still be absent from the pack a catalog indexes.
+            texture_directory = pack / "textures"
+            present = (
+                {
+                    path.stem.casefold()
+                    for path in texture_directory.iterdir()
+                    if path.is_file()
+                }
+                if texture_directory.is_dir()
+                else set()
+            )
             for line in text.splitlines():
                 match = UMODEL_LOADED_TEXTURE_PATTERN.match(line.strip())
                 if match is None:
                     continue
                 object_name, physical_package = match.groups()
-                if object_name.casefold() in exported:
+                if object_name.casefold() in present:
                     continue
                 key = (physical_package.casefold(), object_name.casefold())
                 missing.setdefault(
@@ -1675,11 +1684,18 @@ def cook_variant(
                 f"default material/glTF slot count mismatch for {asset['fullPath']}: "
                 f"{0 if defaults is None else len(defaults)} != {len(slot_names)}"
             )
-        overrides = asset.get("materialSlots", [])
-        if len(overrides) > len(defaults):
-            raise VariantError(
-                f"override array exceeds glTF material slots: {asset_id}"
-            )
+        # UE3 asks a component override only for the mesh's own elements, so an
+        # authored entry past the last element never renders. Keep it in the
+        # receipt instead of rejecting the placement.
+        authored_overrides = asset.get("materialSlots", [])
+        ignored_overrides = [
+            dict(slot) for slot in authored_overrides
+            if int(slot["slot"]) >= len(defaults)
+        ]
+        overrides = [
+            slot for slot in authored_overrides
+            if int(slot["slot"]) < len(defaults)
+        ]
 
         effective = [dict(row) for row in defaults]
         for slot in overrides:
@@ -1905,6 +1921,8 @@ def cook_variant(
             },
             "outputs": outputs,
         }
+        if ignored_overrides:
+            receipt["ignoredOutOfRangeOverrides"] = ignored_overrides
         base.atomic_write_json(pack / "runtime.receipt.json", receipt)
         try:
             base.commit_directory(pack, destination, output_root, force)
@@ -2300,6 +2318,8 @@ def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
     cook.add_argument("--expect-variants", type=int, default=292)
     cook.add_argument("--asset-id", action="append")
     cook.add_argument("--force", action="store_true")
+    cook.add_argument("--package-root", type=Path,
+                      help="Resolve physical packages recorded by older source receipts")
 
     install = commands.add_parser("install")
     install.add_argument("--runtime-manifest", type=Path, required=True)
diff --git a/Tools/LevelPlacementExtractor/test_build_map_material_variants.py b/Tools/LevelPlacementExtractor/test_build_map_material_variants.py
index 9615d34b..eecfa0c9 100644
--- a/Tools/LevelPlacementExtractor/test_build_map_material_variants.py
+++ b/Tools/LevelPlacementExtractor/test_build_map_material_variants.py
@@ -416,11 +416,30 @@ class ExactMaterialResolutionTests(unittest.TestCase):
                 "Exporting Texture2D exported to C:/work/umodel/PKG/tex/exported.dds\n",
                 encoding="utf-8",
             )
+            (pack / "textures").mkdir()
+            (pack / "textures" / "exported.dds").write_bytes(b"DDS ")
             self.assertEqual(
                 [{"physicalPackage": "B.upk", "objectName": "missing"}],
                 tool.missing_loaded_texture_sources([pack.parent]),
             )
 
+    def test_exported_texture_absent_from_pack_is_a_hydration_input(self) -> None:
+        with tempfile.TemporaryDirectory() as temporary:
+            pack = Path(temporary) / "source" / "PACK"
+            (pack / "textures").mkdir(parents=True)
+            (pack / "textures" / "diffuse_d.dds").write_bytes(b"DDS ")
+            (pack / "umodel.log.txt").write_text(
+                "Loading Texture2D diffuse_d from package A.upk\n"
+                "Loading Texture2D mask_mk from package A.upk\n"
+                "Exporting Texture2D diffuse_d to C:/work/umodel/PKG/tex/diffuse_d.dds\n"
+                "Exporting Texture2D mask_mk to C:/work/umodel/PKG/tex/mask_mk.dds\n",
+                encoding="utf-8",
+            )
+            self.assertEqual(
+                [{"physicalPackage": "A.upk", "objectName": "mask_mk"}],
+                tool.missing_loaded_texture_sources([pack.parent]),
+            )
+
     def test_source_only_null_mesh_default_remains_explicit(self) -> None:
         defaults = tool.exact_mesh_defaults(
             [
diff --git a/Tools/NavigationPipeline/Publish-ServerNavigation.ps1 b/Tools/NavigationPipeline/Publish-ServerNavigation.ps1
index 8e57d558..d7a41371 100644
--- a/Tools/NavigationPipeline/Publish-ServerNavigation.ps1
+++ b/Tools/NavigationPipeline/Publish-ServerNavigation.ps1
@@ -7,7 +7,8 @@ param(
     # at a scratch folder stages a publish without touching live outputs.
     [string]$ClientOutputRoot = 'Client/Bin/DataFiles/Navigation',
     [ValidateSet('', 'LV_LUT_HEARTRB_ED', 'LV_LUT_MIDNIGHTC_ED',
-        'LV_DEV_TRAINING_GROUND', 'LV_LOBBY_CLASSSELECT_SL00', 'LV_BER_BERNCASTLE')]
+        'LV_DEV_TRAINING_GROUND', 'LV_LOBBY_CLASSSELECT_SL00', 'LV_BER_BERNCASTLE',
+        'LV_OCN_EVENTIS_MHP')]
     [string]$AreaId = ''
 )
 
@@ -1022,6 +1023,11 @@ $gridFactories = [ordered]@{
         -RelativeSourcePath 'Data/Navigation/LV_BER_BERNCASTLE.navsource' `
         -RelativePaintPath 'Data/Navigation/LV_BER_BERNCASTLE.navpaint' `
         -RuntimeMaximumStepHeight 1.0 }
+    # Maharaka starts on a flat grid at the plaza floor (20.48 m, the height of
+    # most source spots) until the island gets a MapTool bake.
+    'LV_OCN_EVENTIS_MHP' = { New-UniformNavigationGrid `
+        -RelativeAuthoringPath 'Data/Navigation/LV_OCN_EVENTIS_MHP.navgrid.json' `
+        -RuntimeMaximumStepHeight 0.6 }
 }
 $grids = @(foreach ($factory in $gridFactories.GetEnumerator()) {
     if (-not $AreaId -or $factory.Key -eq $AreaId) { & $factory.Value }
diff --git a/Tools/WorldPipeline/Publish-WorldGameplay.ps1 b/Tools/WorldPipeline/Publish-WorldGameplay.ps1
index 78c3c425..a1d6e5e0 100644
--- a/Tools/WorldPipeline/Publish-WorldGameplay.ps1
+++ b/Tools/WorldPipeline/Publish-WorldGameplay.ps1
@@ -1430,7 +1430,8 @@ $spawnDocuments = @(
     (Convert-SpawnGroupsDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
     (Convert-SpawnGroupsDocument -AreaId 'LV_LUT_MIDNIGHTC_ED' -WorldId 'KAKULSAYDON_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
     (Convert-SpawnGroupsDocument -AreaId 'LV_DEV_TRAINING_GROUND' -WorldId 'TRAINING_GROUND' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
-    (Convert-SpawnGroupsDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles)
+    (Convert-SpawnGroupsDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
+    (Convert-SpawnGroupsDocument -AreaId 'LV_OCN_EVENTIS_MHP' -WorldId 'MAHARAKA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles)
 )
 $spawnByWorld = @{}
 foreach ($spawn in $spawnDocuments) { $spawnByWorld[$spawn.WorldId] = $spawn }
@@ -1439,14 +1440,16 @@ $encounterPropDocuments = @(
     (Convert-EncounterPropsDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA'),
     (Convert-EncounterPropsDocument -AreaId 'LV_LUT_MIDNIGHTC_ED' -WorldId 'KAKULSAYDON_ARENA'),
     (Convert-EncounterPropsDocument -AreaId 'LV_DEV_TRAINING_GROUND' -WorldId 'TRAINING_GROUND'),
-    (Convert-EncounterPropsDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA')
+    (Convert-EncounterPropsDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA'),
+    (Convert-EncounterPropsDocument -AreaId 'LV_OCN_EVENTIS_MHP' -WorldId 'MAHARAKA')
 )
 $worlds = @(
     (Convert-WorldDocument -AreaId 'LV_BER_BERNCASTLE' -WorldId 'BERN' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.BERN.GroupIds),
     (Convert-WorldDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.VALTAN_ARENA.GroupIds),
     (Convert-WorldDocument -AreaId 'LV_LUT_MIDNIGHTC_ED' -WorldId 'KAKULSAYDON_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.KAKULSAYDON_ARENA.GroupIds),
     (Convert-WorldDocument -AreaId 'LV_DEV_TRAINING_GROUND' -WorldId 'TRAINING_GROUND' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.TRAINING_GROUND.GroupIds),
-    (Convert-WorldDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.CHARACTER_SELECT_ARENA.GroupIds)
+    (Convert-WorldDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.CHARACTER_SELECT_ARENA.GroupIds),
+    (Convert-WorldDocument -AreaId 'LV_OCN_EVENTIS_MHP' -WorldId 'MAHARAKA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.MAHARAKA.GroupIds)
 )
 }
 
```
