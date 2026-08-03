#include "Loader.h"

#include "ActorCatalog.h"
#include "Camera_Free.h"
#include "Body_Valtan.h"
#include "Character.h"
#include "ClientLaunchOptions.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelCatalog.h"
#include "MapAssetCatalog.h"
#include "MapAssetObject.h"
#include "MapNavigationContract.h"
#include "MapPlacementRuntime.h"
#include "MapStaticBatchObject.h"
#include "Navigation.h"
#include "Part_Body.h"
#include "Part_Equipment.h"
#include "RuntimeAssetRoot.h"
#include "Valtan.h"

#include <algorithm>
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
	Set_Status(TEXT("LOBBY: core UI and camera"));

	if (FAILED(Ready_Camera_Prototype(ETOUI(LEVEL::LOBBY))))
		return E_FAIL;

	Set_Status(TEXT("Lobby loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_Bern()
{
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::BERN));
	Set_Status(TEXT("BERN: world catalog and placements"));

	const LEVEL_CATALOG_ENTRY* pEntry =
		CLevelCatalog::Find(CLIENT_SCENARIO::WORLD_BERN);
	if (nullptr == pEntry || pEntry->eLevel != LEVEL::BERN ||
		pEntry->strMapAreaId.empty() ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::BERN),
			pEntry->strMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("BERN: session character bundle"));
	if (FAILED(Ready_Character_Rendering(ETOUI(LEVEL::BERN))))
		return E_FAIL;

	Set_Status(TEXT("Bern loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_ValtanArena()
{
	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::VALTAN_ARENA));
	Set_Status(TEXT("VALTAN: arena map"));

	const LEVEL_CATALOG_ENTRY* pEntry =
		CLevelCatalog::Find(
			CLIENT_SCENARIO::RAID_VALTAN_ARENA);
	if (nullptr == pEntry ||
		pEntry->eLevel != LEVEL::VALTAN_ARENA ||
		pEntry->strMapAreaId.empty() ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::VALTAN_ARENA),
			pEntry->strMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: network player rendering"));
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::VALTAN_ARENA))))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: server-authoritative boss presentation"));
	if (FAILED(Ready_ValtanPresentation(
		ETOUI(LEVEL::VALTAN_ARENA))))
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
	const CLIENT_SCENARIO scenario =
		CClientLaunchOptions::Get().eScenario;
	const LEVEL_CATALOG_ENTRY* pEntry =
		CLevelCatalog::Find(scenario);
	if (nullptr == pEntry ||
		pEntry->eLevel != LEVEL::DEVELOPMENT)
	{
		return E_INVALIDARG;
	}

	const auto HasDomain = [pEntry](const std::string_view domain)
	{
		return pEntry->AssetDomains.end() !=
			std::find(
				pEntry->AssetDomains.begin(),
				pEntry->AssetDomains.end(),
				domain);
	};

	if (!pEntry->strMapAreaId.empty())
	{
		Set_Status(TEXT("DEV: explicit map scenario"));
		if (!HasDomain("Map") ||
			FAILED(Ready_MapArea(
				ETOUI(LEVEL::DEVELOPMENT),
				pEntry->strMapAreaId,
				pEntry->MapLoadScope)))
		{
			return E_FAIL;
		}
		if (HasDomain("Character"))
		{
			Set_Status(TEXT("DEV: network character rendering"));
			if (FAILED(Ready_Character_Rendering(
				ETOUI(LEVEL::DEVELOPMENT))))
			{
				return E_FAIL;
			}
		}
	}
	else if (HasDomain("Character"))
	{
		Set_Status(TEXT("DEV: LanceMaster scenario"));
		if (FAILED(Ready_Camera_Prototype(
			ETOUI(LEVEL::DEVELOPMENT))) ||
			FAILED(Ready_StaticMeshShader(
				ETOUI(LEVEL::DEVELOPMENT))) ||
			FAILED(Ready_Character_Rendering(
				ETOUI(LEVEL::DEVELOPMENT))))
		{
			return E_FAIL;
		}
	}
	else if (HasDomain("Effect") || HasDomain("UI"))
	{
		Set_Status(TEXT("DEV: minimal render scenario"));
		if (FAILED(Ready_Camera_Prototype(
			ETOUI(LEVEL::DEVELOPMENT))))
		{
			return E_FAIL;
		}
	}
	else
	{
		Set_Status(TEXT("DEV: unsupported scenario"));
		return E_INVALIDARG;
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

	if (FAILED(Ready_StaticMeshShader(iLevelIndex)))
		return E_FAIL;

	Set_Status(TEXT("Map: instance shader"));
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshMapInstance"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl"),
			VTXMESHINSTANCE::Elements,
			VTXMESHINSTANCE::iNumElements))))
	{
		return E_FAIL;
	}

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

	Set_Status(TEXT("Map: object prototypes"));
	if (FAILED(Ready_Camera_Prototype(iLevelIndex)) ||
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

HRESULT CLoader::Ready_Character_Rendering(
	const uint32_t iLevelIndex)
{
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
			VTXANIMMESH::Elements,
			VTXANIMMESH::iNumElements))) ||
		FAILED(Ready_Character_Shared_Prototypes(iLevelIndex)) ||
		FAILED(Ready_LanceMaster_Prototypes(iLevelIndex)))
	{
		return E_FAIL;
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

HRESULT CLoader::Ready_LanceMaster_Prototypes(
	const uint32_t iLevelIndex)
{
	const CHARACTER_ACTOR_ENTRY* pActor =
		CActorCatalog::Find_Character(
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER);
	if (nullptr == pActor || pActor->runtimeStatus != "supported")
		return E_FAIL;

	const matrix_t characterTransform =
		XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) *
		XMMatrixRotationY(XMConvertToRadians(-90.f));

	const std::string bodyPath = ResolveAssetPath(
		pActor->bodyModel);
	if (bodyPath.empty())
		return E_FAIL;

	auto pBodyModel = CModel::Create(
		m_pDevice,
		m_pContext,
		MODEL::ANIM,
		bodyPath.c_str(),
		characterTransform);
	if (nullptr == pBodyModel ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_Component_Model_LanceMaster"),
			std::move(pBodyModel))))
	{
		return E_FAIL;
	}

	static const tchar_t* EQUIPMENT_PROTOTYPE_TAGS[] =
	{
		TEXT("Prototype_Component_Model_LanceMaster_Upper"),
		TEXT("Prototype_Component_Model_LanceMaster_Lower"),
		TEXT("Prototype_Component_Model_LanceMaster_Arm"),
		TEXT("Prototype_Component_Model_LanceMaster_Shoulder"),
		TEXT("Prototype_Component_Model_LanceMaster_Helmet")
	};

	if (pActor->equipmentModels.size() !=
		std::size(EQUIPMENT_PROTOTYPE_TAGS))
	{
		return E_FAIL;
	}

	for (size_t index = 0;
		index < pActor->equipmentModels.size(); ++index)
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);

		const std::string modelPath =
			ResolveAssetPath(pActor->equipmentModels[index]);
		if (modelPath.empty() ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				EQUIPMENT_PROTOTYPE_TAGS[index],
				CModel::Create(
					m_pDevice,
					m_pContext,
					MODEL::ANIM,
					modelPath.c_str(),
					characterTransform))))
		{
			return E_FAIL;
		}
	}

	const std::string weaponPath = ResolveAssetPath(
		pActor->weaponModel);
	if (weaponPath.empty() ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_Component_Model_LanceMaster_Weapon"),
			CModel::Create(
				m_pDevice,
				m_pContext,
				MODEL::NONANIM,
				weaponPath.c_str(),
				XMMatrixScaling(100.f, 100.f, 100.f)))))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CLoader::Ready_ValtanPresentation(const uint32_t iLevelIndex)
{
	const BOSS_ACTOR_ENTRY* pActor =
		CActorCatalog::Find_Boss("BOSS_VALTAN");
	if (nullptr == pActor ||
		pActor->clientPresentationId != "boss.valtan.client.v1")
	{
		return E_FAIL;
	}

	const std::string bodyPath = ResolveAssetPath(
		pActor->bodyModel);
	const std::string weaponPath = ResolveAssetPath(
		pActor->weaponModel);
	if (bodyPath.empty() || weaponPath.empty())
		return E_FAIL;

	auto bodyModel = CModel::Create(
		m_pDevice,
		m_pContext,
		MODEL::ANIM,
		bodyPath.c_str(),
		XMMatrixScaling(0.0001f, 0.0001f, 0.0001f));
	auto weaponModel = CModel::Create(
		m_pDevice,
		m_pContext,
		MODEL::NONANIM,
		weaponPath.c_str(),
		XMMatrixScaling(100.f, 100.f, 100.f));
	if (nullptr == bodyModel || nullptr == weaponModel)
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Model_Valtan"),
		std::move(bodyModel))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Model_ValtanWeapon"),
		std::move(weaponModel))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Body_Valtan"),
		CBody_Valtan::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Valtan"),
		CValtan::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}
	return S_OK;
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
