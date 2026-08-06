#include "Loader.h"

#include "ActorCatalog.h"
#include "AnimationPreviewAssets.h"
#include "Camera_Free.h"
#include "Body_Valtan.h"
#include "Character.h"
#include "CharacterSelectionState.h"
#include "DeployPropCatalog.h"
#include "DeployPropObject.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "MapAssetCatalog.h"
#include "MapAssetObject.h"
#include "MapAssetPreview.h"
#include "MapNavigationContract.h"
#include "MapPlacementRuntime.h"
#include "MapStaticBatchObject.h"
#include "Navigation.h"
#include "NetworkManager.h"
#include "Part_Body.h"
#include "Part_Equipment.h"
#include "PlayableCharacterAssetService.h"
#include "RuntimeAssetRoot.h"
#include "Trigger_Box.h"
#include "Valtan.h"
#include "ValtanPresentationAssetService.h"

#ifdef _DEBUG
#include "MapEditorWorkspaceService.h"
#endif

#include <algorithm>
#include <array>
#include <exception>
#include <unordered_set>

namespace
{
	std::mutex g_ActiveStatusMutex;
	std::string g_ActiveStatus = "Loader has not started.";

	std::string ToUtf8(const tchar_t* pText)
	{
		if (nullptr == pText || L'\0' == *pText)
			return {};
		const int characterCount = static_cast<int>(wcslen(pText));
		const int byteCount = WideCharToMultiByte(
			CP_UTF8, 0, pText, characterCount, nullptr, 0, nullptr, nullptr);
		if (byteCount <= 0)
			return {};
		std::string result(static_cast<size_t>(byteCount), '\0');
		WideCharToMultiByte(
			CP_UTF8, 0, pText, characterCount, result.data(), byteCount,
			nullptr, nullptr);
		return result;
	}

	class CLevelResourceRollbackScope final
	{
	public:
		explicit CLevelResourceRollbackScope(const uint32_t levelIndex)
			: m_iLevelIndex { levelIndex }
		{
		}

		~CLevelResourceRollbackScope()
		{
			if (m_isCommitted)
				return;

			if (FAILED(CGameInstance::Get().Clear_Resources(m_iLevelIndex)))
			{
				OutputDebugStringA(
					"[Loader] Failed to roll back level resources.\n");
			}
		}

		CLevelResourceRollbackScope(
			const CLevelResourceRollbackScope&) = delete;
		CLevelResourceRollbackScope& operator=(
			const CLevelResourceRollbackScope&) = delete;

		void Commit()
		{
			m_isCommitted = true;
		}

	private:
		uint32_t m_iLevelIndex = {};
		bool_t m_isCommitted = false;
	};

	std::string ResolveAssetPath(const std::filesystem::path& relativePath)
	{
		return CRuntimeAssetRoot::Resolve(relativePath).string();
	}

	const tchar_t* Get_CharacterClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return TEXT("Lance Master");
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return TEXT("Gunslinger");
		case CHARACTER_CLASS_ID::SLAYER:
			return TEXT("Slayer");
		case CHARACTER_CLASS_ID::ARTIST:
			return TEXT("Artist");
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return TEXT("DimensionMaster");
		default:
			return TEXT("Unknown");
		}
	}
}

CLoader::CLoader(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

CLoader::~CLoader()
{
	Free();
}

uint32_t APIENTRY ThreadMain(void* pArgument)
{
	const HRESULT comResult =
		CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	CLoader* pLoader = static_cast<CLoader*>(pArgument);
	const HRESULT loadResult =
		nullptr == pLoader ? E_POINTER : pLoader->Start_Loading();

	if (SUCCEEDED(comResult))
		CoUninitialize();

	return FAILED(loadResult) ? 1u : 0u;
}

HRESULT CLoader::Initialize(const LEVEL eNextLevelID)
{
	if (nullptr == CLevelRegistry::Find(eNextLevelID))
		return E_INVALIDARG;

	m_eNextLevelID = eNextLevelID;
	m_iResult.store(S_FALSE, std::memory_order_release);
	m_eState.store(STATE::RUNNING, std::memory_order_release);

	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(
		nullptr,
		0,
		ThreadMain,
		this,
		0,
		nullptr));
	if (nullptr == m_hThread)
	{
		m_iResult.store(E_FAIL, std::memory_order_release);
		m_eState.store(STATE::FAILED, std::memory_order_release);
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CLoader::Start_Loading()
{
	const HRESULT result =
		CLevelRegistry::Execute_Load(m_eNextLevelID, *this);
	m_iResult.store(result, std::memory_order_release);
	m_eState.store(
		SUCCEEDED(result) ? STATE::SUCCEEDED : STATE::FAILED,
		std::memory_order_release);
	if (FAILED(result))
		OutputDebugStringW(L"[Loader] Level load failed.\n");
	return result;
}

void CLoader::Set_Status(const tchar_t* pStatus)
{
	{
		lock_guard<mutex> lock(m_StatusMutex);
		if (nullptr == pStatus)
			m_szLoadingText[0] = L'\0';
		else
			wcsncpy_s(m_szLoadingText, pStatus, _TRUNCATE);
	}

	lock_guard<mutex> activeLock(g_ActiveStatusMutex);
	g_ActiveStatus = ToUtf8(pStatus);
}

std::string CLoader::Get_ActiveStatus()
{
	lock_guard<mutex> lock(g_ActiveStatusMutex);
	return g_ActiveStatus;
}

void CLoader::Copy_Status(
	tchar_t* pOutput,
	const size_t outputCount) const
{
	if (nullptr == pOutput || 0u == outputCount)
		return;
	lock_guard<mutex> lock(m_StatusMutex);
	wcsncpy_s(pOutput, outputCount, m_szLoadingText, _TRUNCATE);
}

#ifdef _DEBUG
void CLoader::Print_Text()
{
	tchar_t status[MAX_PATH]{};
	Copy_Status(status, size(status));
	SetWindowText(g_hWnd, status);
}
#endif

HRESULT CLoader::Ready_For_Lobby()
{
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::LOBBY));
	Set_Status(TEXT("LOBBY: stage selection UI"));
	Set_Status(TEXT("Lobby loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_CharacterSelect()
{
	CValtanPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::CHARACTER_SELECT));
	using LostArk::Shared::CHARACTER_CLASS_ID;
	CHARACTER_CLASS_ID initialClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	if (!CCharacterSelectionState::Try_Get_SelectedClass(initialClass) ||
		!LostArk::Shared::Is_Supported_Playable_Character_Class(initialClass))
	{
		initialClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	}

	const std::array characterClasses = { initialClass };

	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::CHARACTER_SELECT));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::CHARACTER_SELECT);

	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId)
		return E_INVALIDARG;

	Set_Status(TEXT("CHARACTER SELECT: visual map"));
	if (FAILED(Ready_MapArea(
		ETOUI(LEVEL::CHARACTER_SELECT),
		pEntry->pMapAreaId,
		pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("CHARACTER SELECT: playable classes"));
	if (FAILED(Ready_Character_Rendering(
			ETOUI(LEVEL::CHARACTER_SELECT),
			characterClasses)) ||
		FAILED(Ready_AnimationPreviewModels(
			ETOUI(LEVEL::CHARACTER_SELECT))))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Character Select loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_Bern()
{
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::BERN));
	Set_Status(TEXT("BERN: world catalog and placements"));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::BERN);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::BERN),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("BERN: session character bundle"));
	const std::array selectedClass =
	{
		CNetworkManager::Get().Get_LocalCharacterClass()
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::BERN),
		selectedClass)))
		return E_FAIL;

	Set_Status(TEXT("Bern loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_ValtanArena()
{
	CValtanPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::VALTAN_ARENA));
	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::VALTAN_ARENA));
	Set_Status(TEXT("VALTAN: arena map"));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::VALTAN_ARENA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::VALTAN_ARENA),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: network player rendering"));
	const std::array selectedClass =
	{
		CNetworkManager::Get().Get_LocalCharacterClass()
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::VALTAN_ARENA),
		selectedClass)))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: server-authoritative boss presentation"));
	if (FAILED(Ready_ValtanPresentation(
		ETOUI(LEVEL::VALTAN_ARENA))))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: deploy environment prototypes"));
	if (FAILED(Ready_DeployPropArea(
		ETOUI(LEVEL::VALTAN_ARENA),
		pEntry->pMapAreaId)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Valtan arena loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_Development()
{
	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::DEVELOPMENT));

#ifdef _DEBUG
	if (CMapEditorWorkspaceService::Is_Requested())
	{
		Set_Status(TEXT("MAP EDITOR: core rendering resources"));
		if (FAILED(Ready_MapAuthoringCore(ETOUI(LEVEL::DEVELOPMENT))))
		{
			CMapEditorWorkspaceService::Cancel();
			return E_FAIL;
		}
		if (FAILED(Ready_AnimatedMeshShader(ETOUI(LEVEL::DEVELOPMENT))) ||
			FAILED(Ready_DeployPropCore(ETOUI(LEVEL::DEVELOPMENT))))
		{
			CMapEditorWorkspaceService::Cancel();
			return E_FAIL;
		}

		Set_Status(TEXT("Map Editor workspace loading complete"));
		rollback.Commit();
		return S_OK;
	}
#endif

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::DEVELOPMENT);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId)
		return E_INVALIDARG;

	Set_Status(TEXT("TEST: training map"));
	if (FAILED(Ready_MapArea(
		ETOUI(LEVEL::DEVELOPMENT),
		pEntry->pMapAreaId,
		pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	const std::array selectedClass =
	{
		CNetworkManager::Get().Get_LocalCharacterClass()
	};
	Set_Status(TEXT("TEST: server-approved character rendering"));
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::DEVELOPMENT),
		selectedClass)))
	{
		return E_FAIL;
	}
	if (FAILED(Ready_AnimationPreviewModels(
		ETOUI(LEVEL::DEVELOPMENT))))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Development scenario loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_MapArea(
	const uint32_t iLevelIndex,
	const std::string& areaId,
	const MAP_LOAD_SCOPE& loadScope)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || areaId.empty())
		return E_INVALIDARG;

	if (FAILED(Ready_MapAuthoringCore(iLevelIndex)))
		return E_FAIL;

	CMapAssetCatalog mapCatalog;
	Set_Status(TEXT("Map: explicit area catalog"));
	if (!mapCatalog.Load_Area(areaId))
	{
		OutputDebugStringA((
			"[Loader][Map] " +
			mapCatalog.Get_Status() +
			"\n").c_str());
		return E_FAIL;
	}

	std::unordered_set<std::string> requiredAssetIds;
	std::vector<MAP_PLACEMENT_RECORD> scopedPlacements;
	if (loadScope.isEnabled)
	{
		std::string placementStatus;
		Set_Status(TEXT("Map: product load scope"));
		if (!CMapPlacementRuntime::Read_Placements(
			mapCatalog, scopedPlacements, placementStatus))
		{
			OutputDebugStringA(("[Loader][Map] " + placementStatus + "\n").c_str());
			return E_FAIL;
		}
		CMapPlacementRuntime::Apply_LoadScope(
			mapCatalog, loadScope, scopedPlacements);
		for (const MAP_PLACEMENT_RECORD& record : scopedPlacements)
			requiredAssetIds.insert(record.assetId);
		if (requiredAssetIds.empty())
			return E_FAIL;
		CMapPlacementRuntime::Cache_LoadStage(
			areaId,
			loadScope,
			mapCatalog,
			scopedPlacements);
	}

	const matrix_t mapAssetTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f);
	const size_t requiredModelCount = loadScope.isEnabled ?
		requiredAssetIds.size() : mapCatalog.Get_Entries().size();
	size_t loadedModelCount = {};
	for (const MAP_ASSET_ENTRY& entry : mapCatalog.Get_Entries())
	{
		if (loadScope.isEnabled &&
			requiredAssetIds.end() == requiredAssetIds.find(entry.id))
		{
			continue;
		}
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);

		tchar_t progress[128]{};
		_snwprintf_s(
			progress,
			std::size(progress),
			_TRUNCATE,
			TEXT("Map: model prototypes %zu/%zu"),
			loadedModelCount,
			requiredModelCount);
		Set_Status(progress);

		const std::string modelPath =
			entry.resolvedModelPath.string();
		auto pModel = CModel::Create(
			m_pDevice,
			m_pContext,
			MODEL::NONANIM,
			modelPath.c_str(),
			mapAssetTransform);
		if (nullptr == pModel ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				entry.prototypeTag,
				std::move(pModel))))
		{
			const std::wstring detail =
				L"[Loader][Map] Model failed: " +
				entry.prototypeTag +
				L" / " +
				entry.resolvedModelPath.wstring() +
				L"\n";
			OutputDebugStringW(detail.c_str());
			return E_FAIL;
		}
		++loadedModelCount;
	}
	if (loadedModelCount != requiredModelCount)
		return E_FAIL;

	MAP_NAVIGATION_CONTRACT navigationContract;
	std::string navigationStatus;
	Set_Status(TEXT("Map: navigation contract"));
	if (!CMapNavigationContract::Resolve_Area(
		areaId,
		navigationContract,
		navigationStatus))
	{
		OutputDebugStringA((
			"[Loader][Map] " +
			navigationStatus +
			"\n").c_str());
		return E_FAIL;
	}

	if (navigationContract.runtimeGridAvailable)
	{
		auto pNavigation = CNavigation::Create_NavGrid(
			m_pDevice,
			m_pContext,
			navigationContract.runtimePath.c_str());
		if (nullptr == pNavigation ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				navigationContract.prototypeTag,
				std::move(pNavigation))))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLoader::Ready_MapAuthoringCore(const uint32_t iLevelIndex)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) ||
		FAILED(Ready_StaticMeshShader(iLevelIndex)) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			CMapAssetPreview::SHADER_PROTOTYPE_TAG,
			CShader::Create(
				m_pDevice,
				m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl"),
				VTXMESH::Elements,
				VTXMESH::iNumElements))))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Map: editor core prototypes"));
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshMapInstance"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl"),
			VTXMESHINSTANCE::Elements,
			VTXMESHINSTANCE::iNumElements))) ||
		FAILED(Ready_Camera_Prototype(iLevelIndex)) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_MapAsset"),
			CMapAssetObject::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_MapStaticBatch"),
			CMapStaticBatchObject::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

#ifdef _DEBUG
	if (iLevelIndex == ETOUI(LEVEL::DEVELOPMENT) &&
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_TriggerBox"),
			CTrigger_Box::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}
#endif

	return S_OK;
}

HRESULT CLoader::Ready_Camera_Prototype(
	const uint32_t iLevelIndex)
{
	return CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Camera_Free"),
		CCamera_Free::Create(m_pDevice, m_pContext));
}

HRESULT CLoader::Ready_StaticMeshShader(
	const uint32_t iLevelIndex)
{
	return CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
			VTXMESH::Elements,
			VTXMESH::iNumElements));
}

HRESULT CLoader::Ready_AnimatedMeshShader(
	const uint32_t iLevelIndex)
{
	return CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
			VTXANIMMESH::Elements,
			VTXANIMMESH::iNumElements));
}

HRESULT CLoader::Ready_DeployPropCore(const uint32_t iLevelIndex)
{
	if (iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;
	return CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_DeployProp"),
		CDeployPropObject::Create(m_pDevice, m_pContext));
}

HRESULT CLoader::Ready_DeployPropArea(
	const uint32_t iLevelIndex,
	const std::string& areaId)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || areaId.empty())
		return E_INVALIDARG;

	CDeployPropCatalog catalog;
	if (!catalog.Load_Default(areaId))
	{
		OutputDebugStringA(("[Loader][DeployProp] " +
			catalog.Get_Status() + "\n").c_str());
		return E_FAIL;
	}

	const matrix_t modelTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f);
	for (const DEPLOY_PROP_ASSET_ENTRY& asset : catalog.Get_Assets())
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);

		const MODEL modelKind =
			DEPLOY_PROP_MODEL_KIND::ANIM == asset.kind ?
			MODEL::ANIM : MODEL::NONANIM;
		auto intactModel = CModel::Create(
			m_pDevice,
			m_pContext,
			modelKind,
			asset.intactResolvedPath.string().c_str(),
			modelTransform);
		if (nullptr == intactModel ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				asset.intactPrototypeTag,
				std::move(intactModel))))
		{
			return E_FAIL;
		}

		if (DEPLOY_PROP_MODEL_KIND::STATIC != asset.kind)
			continue;
		auto fracturedModel = CModel::Create(
			m_pDevice,
			m_pContext,
			MODEL::NONANIM,
			asset.fracturedResolvedPath.string().c_str(),
			modelTransform);
		if (nullptr == fracturedModel ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				asset.fracturedPrototypeTag,
				std::move(fracturedModel))))
		{
			return E_FAIL;
		}
	}
	return Ready_DeployPropCore(iLevelIndex);
}

HRESULT CLoader::Ready_Character_Rendering(
	const uint32_t iLevelIndex,
	const std::span<const LostArk::Shared::CHARACTER_CLASS_ID>
		characterClasses)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || characterClasses.empty())
	{
		return E_INVALIDARG;
	}
	for (const auto characterClass : characterClasses)
	{
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
			characterClass))
		{
			return E_INVALIDARG;
		}
	}

	CPlayableCharacterAssetService::Begin_LevelLoad(iLevelIndex);

	if (FAILED(Ready_AnimatedMeshShader(iLevelIndex)) ||
		FAILED(Ready_Character_Shared_Prototypes(iLevelIndex)))
	{
		return E_FAIL;
	}

	for (size_t classIndex = 0;
		classIndex < characterClasses.size();
		++classIndex)
	{
		const auto characterClass = characterClasses[classIndex];
		const auto progress = [
			this,
			classIndex,
			classCount = characterClasses.size(),
			characterClass](
			const size_t completedModelCount,
			const size_t totalModelCount,
			const std::string& assetId)
		{
			const std::wstring fileName =
				std::filesystem::path(assetId).filename().wstring();
			tchar_t status[MAX_PATH]{};
			_snwprintf_s(
				status,
				std::size(status),
				_TRUNCATE,
				TEXT("Character %zu/%zu | %s models %zu/%zu | %s"),
				classIndex + 1u,
				classCount,
				Get_CharacterClassName(characterClass),
				completedModelCount,
				totalModelCount,
				fileName.c_str());
			Set_Status(status);
		};
		if (FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			iLevelIndex,
			characterClass,
			&m_isCancellationRequested,
			progress)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLoader::Ready_Character_Shared_Prototypes(
	const uint32_t iLevelIndex)
{
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Part_Equipment"),
		CPart_Equipment::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_Part_Body"),
			CPart_Body::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_Character"),
			CCharacter::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CLoader::Ready_AnimationPreviewModels(
	const uint32_t iLevelIndex)
{
#ifdef _DEBUG
	if (iLevelIndex != ETOUI(LEVEL::CHARACTER_SELECT) &&
		iLevelIndex != ETOUI(LEVEL::DEVELOPMENT))
	{
		return E_INVALIDARG;
	}
	using LostArk::Shared::CHARACTER_CLASS_ID;
	if (!CPlayableCharacterAssetService::Is_Ready(
			iLevelIndex, CHARACTER_CLASS_ID::DIMENSIONMASTER))
	{
		Set_Status(TEXT("Animation preview: DimensionMaster character"));
		if (FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			iLevelIndex,
			CHARACTER_CLASS_ID::DIMENSIONMASTER,
			&m_isCancellationRequested)))
		{
			return E_FAIL;
		}
	}

	// Core and summon use the same ActorX Blender unit contract as the playable
	// DimensionMaster body, not the older UModel character-pack scale.
	const matrix_t previewTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f) *
		XMMatrixRotationY(XMConvertToRadians(-90.f));
	for (const ANIMATION_PREVIEW_ASSET& asset :
		ANIMATION_PREVIEW_ASSETS)
	{
		// The main body is the playable DimensionMaster prototype admitted above.
		// Loading it again under a debug-only tag doubled a 154-clip decode.
		if (0 == wcscmp(
			asset.pPrototypeTag,
			TEXT("Prototype_Component_Model_DimensionMaster")))
		{
			continue;
		}
		const filesystem::path path =
			CRuntimeAssetRoot::Resolve(asset.pModelAssetId);
		if (path.empty() || !filesystem::is_regular_file(path))
			continue;

		unique_ptr<CPrototype> model = CModel::Create(
			m_pDevice,
			m_pContext,
			MODEL::ANIM,
			path.string().c_str(),
			previewTransform);
		if (nullptr == model ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				asset.pPrototypeTag,
				move(model))))
		{
			return E_FAIL;
		}
	}
#endif
	return S_OK;
}

HRESULT CLoader::Ready_ValtanPresentation(const uint32_t iLevelIndex)
{
	return CValtanPresentationAssetService::Ensure_Prototypes(
		m_pDevice,
		m_pContext,
		iLevelIndex);
}

unique_ptr<CLoader> CLoader::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eNextLevelID)
{
	auto pInstance = unique_ptr<CLoader>(
		new CLoader(pDevice, pContext));
	if (FAILED(pInstance->Initialize(eNextLevelID)))
		return nullptr;
	return pInstance;
}

void CLoader::Free()
{
	if (nullptr != m_hThread)
	{
		m_isCancellationRequested.store(true, std::memory_order_release);
		DWORD waitResult = WaitForSingleObject(m_hThread, 5000);
		if (WAIT_TIMEOUT == waitResult)
		{
			CancelSynchronousIo(m_hThread);
			waitResult = WaitForSingleObject(m_hThread, 5000);
		}
		if (WAIT_TIMEOUT == waitResult)
		{
			OutputDebugStringA(
				"[Loader] Worker exceeded shutdown deadline; terminating the process to preserve loader invariants.\n");
			if (!TerminateProcess(GetCurrentProcess(), ERROR_TIMEOUT))
				std::terminate();
			__assume(0);
		}
		CloseHandle(m_hThread);
		m_hThread = nullptr;
	}
}
