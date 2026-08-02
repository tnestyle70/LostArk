#include "Loader.h"

#include "Player.h"
#include "Body_Player.h"
#include "Valtan.h"
#include "Body_Valtan.h"
#include "Part_Equipment.h"
#include "Part_Body.h"
#include "Character.h"
#include "Npc.h"
#include "MapAssetCatalog.h"
#include "MapNavigationContract.h"
#include "MapAssetObject.h"
#include "MapStaticBatchObject.h"

#include "MapAssetPreview.h"
#include "DeployPropCatalog.h"
#include "DeployPropObject.h"


#include "Sky.h"
#include "Snow.h"
#include "Effect.h"
#include "Weapon.h"
#include "Monster.h"
#include "Terrain.h"
#include "ForkLift.h"
#include "Explosion.h"
#include "BackGround.h"
#include "Camera_Free.h"

#include "GameInstance.h"
#include "Navigation.h"

namespace
{
    class CLevelResourceRollbackScope final
    {
    public:
        explicit CLevelResourceRollbackScope(uint32_t levelIndex)
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
                    "[Loader] Failed to roll back partially loaded level resources.\n");
            }
        }

        CLevelResourceRollbackScope(
            const CLevelResourceRollbackScope&) = delete;
        CLevelResourceRollbackScope& operator=(
            const CLevelResourceRollbackScope&) = delete;

        void Commit() { m_isCommitted = true; }

    private:
        uint32_t m_iLevelIndex = {};
        bool_t m_isCommitted = false;
    };
}

CLoader::CLoader(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : m_pDevice { pDevice } 
    , m_pContext { pContext }
{
}

CLoader::~CLoader()
{
    Free();
}

uint32_t APIENTRY ThreadMain(void* pArg)
{
    CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

    auto  pLoader = static_cast<CLoader*>(pArg);

    if (FAILED(pLoader->Start_Loading()))
        return 1;

    CoUninitialize();

    return 0;
}

HRESULT CLoader::Initialize(LEVEL eNextLevelID)
{
    m_eNextLevelID = eNextLevelID;

    InitializeCriticalSection(&m_CriticalSection);

    /* 스레드를 생성한다. */
    m_hThread = (HANDLE)_beginthreadex(nullptr, 0, ThreadMain, this, 0, nullptr);

    if (0 == m_hThread)
        return E_FAIL;    

    return S_OK;
}

HRESULT CLoader::Start_Loading()
{
    /*내 스레드가 임계영역에 일을 좀 할꺼야! */
    EnterCriticalSection(&m_CriticalSection);

    HRESULT hr = {};

    switch (m_eNextLevelID)
    {
    case LEVEL::LOGO:        
        hr = Ready_For_Level_Logo();
        break;
    case LEVEL::LOBBY:
        hr = Ready_For_Lobby();
        break;
    case LEVEL::BAREN:
        hr = Ready_For_Baren();
        break;
	case LEVEL::VALTAN_ARENA:
		hr = Ready_For_ValtanArena();
		break;
    case LEVEL::GAMEPLAY:
        hr = Ready_For_Level_GamePlay();
        break;
    case LEVEL::ASSET_TEST:
        hr = Ready_For_Level_AssetTest();
        break;
    case LEVEL::TEST_LEVEL2:
        hr = Ready_For_Test_Level2();
        break;
    }

    /* 내 스레드가 임계영역에 해야할 일을 끝냈어. */
    LeaveCriticalSection(&m_CriticalSection);

    if (FAILED(hr))
    {
        MessageBox(
            nullptr,
            m_szLoadingText,
            L"Level Loading Failed",
            MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    return S_OK;
}

#ifdef _DEBUG
void CLoader::Print_Text()
{
    SetWindowText(g_hWnd, m_szLoadingText);    
}
#endif

HRESULT CLoader::Ready_For_Level_Logo()
{
    lstrcpy(m_szLoadingText, TEXT("Texture Loading"));
    /* For.Prototype_Component_Texture_BackGround */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::LOGO), TEXT("Prototype_Component_Texture_BackGround"),
        CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Default%d.jpg"), 2))))
        return E_FAIL;


    lstrcpy(m_szLoadingText, TEXT("Model Loading"));


    lstrcpy(m_szLoadingText, TEXT("Shader Loading"));


    lstrcpy(m_szLoadingText, TEXT("Object Prototype Loading"));

    /* For.Prototype_GameObject_BackGround */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_GameObject_BackGround"),
        CBackGround::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    lstrcpy(m_szLoadingText, TEXT("Loading Complete"));

    m_isFinished = true;

    return S_OK;
}

HRESULT CLoader::Ready_For_Lobby()
{
    lstrcpy(m_szLoadingText, TEXT("Loading Lobby"));

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::LOBBY),
        TEXT("Prototype_GameObject_Camera_Free"),
        CCamera_Free::Create(m_pDevice, m_pContext))))
    {
        return E_FAIL;
    }

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::LOBBY),
        TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
        CShader::Create(
            m_pDevice,
            m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
            VTXANIMMESH::Elements,
            VTXANIMMESH::iNumElements))))
    {
        return E_FAIL;
    }

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::LOBBY),
        TEXT("Prototype_Component_Shader_VtxMeshBinary"),
        CShader::Create(
            m_pDevice,
            m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
            VTXMESH::Elements,
            VTXMESH::iNumElements))))
    {
        return E_FAIL;
    }

    if (FAILED(Ready_Character_Shared_Prototypes(
        ETOUI(LEVEL::LOBBY))))
    {
        return E_FAIL;
    }

    if (FAILED(Ready_LanceMaster_Prototypes(
        ETOUI(LEVEL::LOBBY))))
    {
        return E_FAIL;
    }

    lstrcpy(
        m_szLoadingText,
        TEXT("Lobby Loading Complete"));

    m_isFinished = true;

    return S_OK;
}

HRESULT CLoader::Ready_For_Baren()
{
	CLevelResourceRollbackScope resourceRollback(
		ETOUI(LEVEL::BAREN));

	lstrcpy(m_szLoadingText, TEXT("BAREN: loading Bern Castle map"));
	if (FAILED(Ready_MapArea(
		ETOUI(LEVEL::BAREN), "LV_BER_BERNCASTLE")))
	{
		return E_FAIL;
	}

	lstrcpy(m_szLoadingText, TEXT("Bern Castle map loading complete"));
	resourceRollback.Commit();
	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Ready_For_ValtanArena()
{
	CLevelResourceRollbackScope resourceRollback(
		ETOUI(LEVEL::VALTAN_ARENA));

	lstrcpy(m_szLoadingText, TEXT("VALTAN_ARENA: loading Valtan map"));
	if (FAILED(Ready_MapArea(
		ETOUI(LEVEL::VALTAN_ARENA), "LV_LUT_HEARTRB_ED")))
	{
		return E_FAIL;
	}

	lstrcpy(m_szLoadingText, TEXT("Valtan arena map loading complete"));
	resourceRollback.Commit();
	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Ready_MapArea(
	uint32_t iLevelIndex,
	const std::string& areaId)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || areaId.empty())
		return E_INVALIDARG;

	lstrcpy(m_szLoadingText, TEXT("Map: VtxMeshBinary shader"));
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
			VTXMESH::Elements,
			VTXMESH::iNumElements))))
	{
		return E_FAIL;
	}

	lstrcpy(m_szLoadingText, TEXT("Map: instance shader"));
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
	lstrcpy(m_szLoadingText, TEXT("Map: fixed area catalog"));
	if (!mapCatalog.Load_Area(areaId))
	{
		OutputDebugStringA(("[Loader][Map] " +
			mapCatalog.Get_Status() + "\n").c_str());
		return E_FAIL;
	}

	const matrix_t mapAssetTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f);
	lstrcpy(m_szLoadingText, TEXT("Map: model prototypes"));
	for (const MAP_ASSET_ENTRY& entry : mapCatalog.Get_Entries())
	{
		const std::string modelPath = entry.resolvedModelPath.string();
		auto modelPrototype = CModel::Create(
			m_pDevice,
			m_pContext,
			MODEL::NONANIM,
			modelPath.c_str(),
			mapAssetTransform);
		if (nullptr == modelPrototype ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				entry.prototypeTag,
				std::move(modelPrototype))))
		{
			const std::wstring detail =
				L"[Loader][Map] Model prototype failed: " +
				entry.prototypeTag + L" / " +
				entry.resolvedModelPath.wstring() + L"\n";
			OutputDebugStringW(detail.c_str());
			return E_FAIL;
		}
	}

	MAP_NAVIGATION_CONTRACT navigationContract;
	std::string navigationStatus;
	lstrcpy(m_szLoadingText, TEXT("Map: navigation contract"));
	if (!CMapNavigationContract::Resolve_Area(
		areaId, navigationContract, navigationStatus))
	{
		OutputDebugStringA(("[Loader][Map] " +
			navigationStatus + "\n").c_str());
		return E_FAIL;
	}
	if (navigationContract.runtimeGridAvailable)
	{
		auto navigationPrototype = CNavigation::Create_NavGrid(
			m_pDevice,
			m_pContext,
			navigationContract.runtimePath.c_str());
		if (nullptr == navigationPrototype ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				navigationContract.prototypeTag,
				std::move(navigationPrototype))))
		{
			return E_FAIL;
		}
	}
	else
	{
		OutputDebugStringA(("[Loader][Map] " +
			navigationStatus + "\n").c_str());
	}

	lstrcpy(m_szLoadingText, TEXT("Map: object prototypes"));
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Camera_Free"),
		CCamera_Free::Create(m_pDevice, m_pContext))) ||
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

HRESULT CLoader::Ready_For_Level_AssetTest()
{
    // The current level is LOADING, and ASSET_TEST objects have not been
    // created yet. A failed load can therefore clear the target level safely.
    CLevelResourceRollbackScope resourceRollback(
        ETOUI(LEVEL::ASSET_TEST));

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: VtxAnimMeshBinary shader"));

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
        CShader::Create(m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
            VTXANIMMESH::Elements,
            VTXANIMMESH::iNumElements))))
        return E_FAIL;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: VtxMeshBinary shader"));
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Shader_VtxMeshBinary"),
        CShader::Create(m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
            VTXMESH::Elements,
            VTXMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(Ready_Character_Shared_Prototypes(
        ETOUI(LEVEL::ASSET_TEST))))
        return E_FAIL;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: LanceMaster prototypes"));
    if (FAILED(Ready_LanceMaster_Prototypes(
        ETOUI(LEVEL::ASSET_TEST))))
        return E_FAIL;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: map instance shader"));
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Shader_VtxMeshMapInstance"),
        CShader::Create(m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl"),
            VTXMESHINSTANCE::Elements,
            VTXMESHINSTANCE::iNumElements))))
        return E_FAIL;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: map preview shader"));
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        CMapAssetPreview::SHADER_PROTOTYPE_TAG,
        CShader::Create(m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl"),
            VTXMESH::Elements,
            VTXMESH::iNumElements))))
        return E_FAIL;

    const matrix_t lostArkAssetPreTransform =
        XMMatrixScaling(0.0001f, 0.0001f, 0.0001f);

    const matrix_t mapAssetTransform =
        XMMatrixScaling(0.01f, 0.01f, 0.01f);

    //Valtan Raid 맵 기준 정적 에셋 카탈로그
    CMapAssetCatalog mapCatalog;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: map catalog"));
    if (!mapCatalog.Load_Default())
    {
        return E_FAIL;
    }

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: map model prototypes"));
    for (const MAP_ASSET_ENTRY& entry : mapCatalog.Get_Entries())
    {
        const string modelPath = entry.resolvedModelPath.string();

        const wstring assetId(entry.id.begin(), entry.id.end());
        const auto reportModelFailure =
            [this, &entry, &assetId](const wchar_t* failureStage)
            {
                const wstring loadingDetail =
                    L"Map model failed [" + wstring(failureStage) +
                    L"] id=" + assetId +
                    L" tag=" + entry.prototypeTag +
                    L" path=" + entry.resolvedModelPath.wstring();
                const wstring loadingSummary =
                    L"Map model failed [" + wstring(failureStage) +
                    L"] id=" + assetId + L" (see debugger)";
                const wstring& loadingText =
                    loadingDetail.size() < std::size(m_szLoadingText) ?
                    loadingDetail : loadingSummary;
                lstrcpynW(
                    m_szLoadingText,
                    loadingText.c_str(),
                    static_cast<int>(std::size(m_szLoadingText)));

                const wstring debugDetail =
                    L"[Loader][ASSET_TEST] Map model prototype failure\n"
                    L"stage=" + wstring(failureStage) +
                    L"\nasset_id=" + assetId +
                    L"\nprototype_tag=" + entry.prototypeTag +
                    L"\nresolved_path=" +
                    entry.resolvedModelPath.wstring() + L"\n";
                OutputDebugStringW(debugDetail.c_str());
            };

        auto modelPrototype = CModel::Create(
            m_pDevice,
            m_pContext,
            MODEL::NONANIM,
            modelPath.c_str(),
            mapAssetTransform);
        if (nullptr == modelPrototype)
        {
            reportModelFailure(L"CModel::Create");
            return E_FAIL;
        }

        if (FAILED(CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::ASSET_TEST),
            entry.prototypeTag,
            std::move(modelPrototype))))
        {
            reportModelFailure(L"Add_Prototype");
            return E_FAIL;
        }
    }

    //Valtan Raid 맵 기준 동적 에셋 카탈로그
    CDeployPropCatalog deployCatalog;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: deploy catalog"));
    if (deployCatalog.Load_Default(mapCatalog.Get_AreaId()))
    {
        for (const DEPLOY_PROP_ASSET_ENTRY& entry : deployCatalog.Get_Assets())
        {
            const MODEL modelKind =
                entry.kind == DEPLOY_PROP_MODEL_KIND::ANIM ?
                MODEL::ANIM : MODEL::NONANIM;

            const string intactPath = entry.intactResolvedPath.string();

            lstrcpy(
                m_szLoadingText,
                TEXT("ASSET_TEST: deploy intact prototypes"));
            if (FAILED(CGameInstance::Get().Add_Prototype(
                ETOUI(LEVEL::ASSET_TEST), entry.intactPrototypeTag,
                CModel::Create(m_pDevice, m_pContext,
                    modelKind, intactPath.c_str(), mapAssetTransform))))
                return E_FAIL;

            if (entry.kind == DEPLOY_PROP_MODEL_KIND::STATIC)
            {
                const string fracturedPath = entry.fracturedResolvedPath.string();
                lstrcpy(
                    m_szLoadingText,
                    TEXT("ASSET_TEST: deploy fractured prototypes"));
                if (FAILED(CGameInstance::Get().Add_Prototype(
                    ETOUI(LEVEL::ASSET_TEST), entry.fracturedPrototypeTag,
                    CModel::Create(m_pDevice, m_pContext,
                        MODEL::NONANIM, fracturedPath.c_str(), mapAssetTransform))))
                    return E_FAIL;
            }
        }
    }

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: Valtan model"));
    auto valtanModel = CModel::Create(
        m_pDevice,
        m_pContext,
        MODEL::ANIM,
        "../Bin/Resources/LostArk/Character/Valtan/MN_RPBF_01.wmodel",
        lostArkAssetPreTransform);
    if (nullptr == valtanModel)
    {
        lstrcpy(m_szLoadingText, TEXT("Valtan model failed after map load"));
        OutputDebugStringA(
            "[Loader][ASSET_TEST] Valtan CModel::Create failed after map load.\n");
        return E_FAIL;
    }
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Model_Valtan"),
        std::move(valtanModel))))
    {
        lstrcpy(m_szLoadingText, TEXT("Valtan prototype registration failed"));
        return E_FAIL;
    }

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: Valtan weapon model"));
    auto valtanWeaponModel = CModel::Create(
        m_pDevice,
        m_pContext,
        MODEL::NONANIM,
        "../Bin/Resources/LostArk/Character/Valtan/ValtanWeapon.wmodel",
        XMMatrixScaling(100.f, 100.f, 100.f));
    if (nullptr == valtanWeaponModel)
    {
        lstrcpy(m_szLoadingText, TEXT("Valtan weapon model failed after map load"));
        OutputDebugStringA(
            "[Loader][ASSET_TEST] Valtan weapon CModel::Create failed after map load.\n");
        return E_FAIL;
    }
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Model_ValtanWeapon"),
        std::move(valtanWeaponModel))))
    {
        lstrcpy(m_szLoadingText, TEXT("Valtan weapon prototype registration failed"));
        return E_FAIL;
    }

    MAP_NAVIGATION_CONTRACT navigationContract;
    string navigationStatus;
    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: resolve navigation"));
    if (!CMapNavigationContract::Resolve_Area(
        mapCatalog.Get_AreaId(), navigationContract, navigationStatus))
        return E_FAIL;

    if (navigationContract.runtimeGridAvailable)
    {
        lstrcpy(
            m_szLoadingText,
            TEXT("ASSET_TEST: navigation prototype"));
        auto navigationPrototype = CNavigation::Create_NavGrid(
            m_pDevice,
            m_pContext,
            navigationContract.runtimePath.c_str());
        if (nullptr == navigationPrototype ||
            FAILED(CGameInstance::Get().Add_Prototype(
                ETOUI(LEVEL::ASSET_TEST),
                navigationContract.prototypeTag,
                std::move(navigationPrototype))))
        {
            return E_FAIL;
        }
    }

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: Camera prototype"));
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_Camera_Free"),
        CCamera_Free::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: Body_Valtan prototype"));
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_Body_Valtan"),
        CBody_Valtan::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: Valtan object prototype"));
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_Valtan"),
        CValtan::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: MapAsset prototype"));
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_MapAsset"),
        CMapAssetObject::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: MapStaticBatch prototype"));
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_MapStaticBatch"),
        CMapStaticBatchObject::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    lstrcpy(
        m_szLoadingText,
        TEXT("ASSET_TEST: DeployProp prototype"));
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_DeployProp"),
        CDeployPropObject::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    lstrcpy(m_szLoadingText, TEXT("Binary Asset Test Loading Complete"));

    resourceRollback.Commit();
    m_isFinished = true;

    return S_OK;
}

HRESULT CLoader::Ready_For_Test_Level2()
{
    lstrcpy(m_szLoadingText, TEXT("Loading Test Level 2 resources."));

    /*
     * The HDR regression only needs a camera.  Loading the full LanceMaster
     * character makes the renderer test depend on optional Drive-managed
     * models and currently blocks at CModel::Create when they are absent.
     * Normal TEST_LEVEL2 startup still follows the complete path below.
     */
    const wchar_t* pCommandLine = GetCommandLineW();
    const bool_t isHDRReadbackRequested =
        nullptr != pCommandLine &&
        nullptr != wcsstr(pCommandLine, L"--hdr-readback");
    if (isHDRReadbackRequested)
    {
        if (FAILED(CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::TEST_LEVEL2),
            TEXT("Prototype_GameObject_Camera_Free"),
            CCamera_Free::Create(m_pDevice, m_pContext))))
        {
            return E_FAIL;
        }

        lstrcpy(m_szLoadingText, TEXT("HDR validation loading complete."));
        m_isFinished = true;
        return S_OK;
    }

#pragma region CAMERA
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::TEST_LEVEL2),
        TEXT("Prototype_GameObject_Camera_Free"),
        CCamera_Free::Create(m_pDevice, m_pContext))))
        return E_FAIL;
#pragma endregion

#pragma region SHADER
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::TEST_LEVEL2),
        TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
        CShader::Create(m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
            VTXANIMMESH::Elements,
            VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::TEST_LEVEL2),
        TEXT("Prototype_Component_Shader_VtxMeshBinary"),
        CShader::Create(m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
            VTXMESH::Elements,
            VTXMESH::iNumElements))))
        return E_FAIL;
#pragma endregion

    if (FAILED(Ready_Character_Shared_Prototypes(
        ETOUI(LEVEL::TEST_LEVEL2))))
        return E_FAIL;

    /* One class per run -- keep this in step with the spec CLevel_Test2 spawns.
    Registering two full classes here decodes both bodies and every clip, which
    is what exhausted the Debug heap when LanceMaster and GunSlinger shared the
    level. */
    //if (FAILED(Ready_LanceMaster_Prototypes(
    //    ETOUI(LEVEL::TEST_LEVEL2))))
    //    return E_FAIL;

    //if (FAILED(Ready_GunSlinger_Prototypes(
    //    ETOUI(LEVEL::TEST_LEVEL2))))
    //    return E_FAIL;

    //if (FAILED(Ready_Artist_Prototypes(
    //    ETOUI(LEVEL::TEST_LEVEL2))))
    //    return E_FAIL;

    if (FAILED(Ready_Slayer_Prototypes(
        ETOUI(LEVEL::TEST_LEVEL2))))
        return E_FAIL;

    if (FAILED(Ready_Npc_Prototypes(
        ETOUI(LEVEL::TEST_LEVEL2))))
        return E_FAIL;

    lstrcpy(m_szLoadingText, TEXT("Test Level 2 loading complete."));
    m_isFinished = true;

    return S_OK;
}

HRESULT CLoader::Ready_LanceMaster_Prototypes(uint32_t iLevelIndex)
{
    const matrix_t preTransform =
        XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) *
        XMMatrixRotationY(
            XMConvertToRadians(-90.f));;

    auto pLanceMasterModel = CModel::Create(
        m_pDevice,
        m_pContext,
        MODEL::ANIM,
        "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster.wmodel",
        preTransform);

    if (nullptr == pLanceMasterModel)
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_Component_Model_LanceMaster"),
        move(pLanceMasterModel))))
        return E_FAIL;

    static const struct
    {
        const tchar_t* pTag;
        const char_t* pPath;
    } EquipmentModels[] =
    {
        { TEXT("Prototype_Component_Model_LanceMaster_Upper"),
          "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster_Upper.wmodel" },
        { TEXT("Prototype_Component_Model_LanceMaster_Lower"),
          "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster_Lower.wmodel" },
        { TEXT("Prototype_Component_Model_LanceMaster_Arm"),
          "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster_Arm.wmodel" },
        { TEXT("Prototype_Component_Model_LanceMaster_Shoulder"),
          "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster_Shoulder.wmodel" },
        { TEXT("Prototype_Component_Model_LanceMaster_Helmet"),
          "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster_Helmet.wmodel" },
    };

    for (const auto& equipmentModel : EquipmentModels)
    {
        if (FAILED(CGameInstance::Get().Add_Prototype(
            iLevelIndex,
            equipmentModel.pTag,
            CModel::Create(
                m_pDevice,
                m_pContext,
                MODEL::ANIM,
                equipmentModel.pPath,
                preTransform))))
            return E_FAIL;
    }

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_Component_Model_LanceMaster_Weapon"),
        CModel::Create(
            m_pDevice,
            m_pContext,
            MODEL::NONANIM,
            "../Bin/Resources/LostArk/Character/WP_WFLM_00L/WP_WFLM_00L.wmodel",
            XMMatrixScaling(100.f, 100.f, 100.f)))))
        return E_FAIL;

    return S_OK;
}

/* Class-agnostic: CCharacter and its part classes are driven entirely by the
CHARACTER_SPEC handed to them, so one registration serves every class on a
level. Split out of Ready_LanceMaster_Prototypes when GunSlinger arrived. */
HRESULT CLoader::Ready_Character_Shared_Prototypes(uint32_t iLevelIndex)
{
    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_GameObject_Part_Equipment"),
        CPart_Equipment::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_GameObject_Part_Body"),
        CPart_Body::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_GameObject_Character"),
        CCharacter::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    return S_OK;
}

HRESULT CLoader::Ready_Npc_Prototypes(uint32_t iLevelIndex)
{
    /* Same cook units as the playable classes: centimetre out of the converter,
    metres in the world, facing -Z until rotated. */
    const matrix_t preTransform =
        XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) *
        XMMatrixRotationY(
            XMConvertToRadians(-90.f));

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_GameObject_Npc"),
        CNpc::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* The cook merges each NPC's body and head into one model, so one tag is one
    NPC. Heads are shared upstream in the source data, not here. */
    static const struct
    {
        const tchar_t* pTag;
        const char_t* pPath;
    } NpcModels[] =
    {
        { TEXT("Prototype_Component_Model_Npc_Aylara"),
          "../Bin/Resources/LostArk/Character/NPC/Npc_Aylara/Npc_Aylara.wmodel" },
        { TEXT("Prototype_Component_Model_Npc_Forman"),
          "../Bin/Resources/LostArk/Character/NPC/Npc_Forman/Npc_Forman.wmodel" },
        { TEXT("Prototype_Component_Model_Npc_Schmidt"),
          "../Bin/Resources/LostArk/Character/NPC/Npc_Schmidt/Npc_Schmidt.wmodel" },
        { TEXT("Prototype_Component_Model_Npc_Beda"),
          "../Bin/Resources/LostArk/Character/NPC/Npc_Beda/Npc_Beda.wmodel" },
    };

    for (const auto& npcModel : NpcModels)
    {
        if (FAILED(CGameInstance::Get().Add_Prototype(
            iLevelIndex,
            npcModel.pTag,
            CModel::Create(
                m_pDevice,
                m_pContext,
                MODEL::ANIM,
                npcModel.pPath,
                preTransform))))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CLoader::Ready_GunSlinger_Prototypes(uint32_t iLevelIndex)
{
    /* Same contract as LanceMaster: the cook is centimetre, the loader brings it
    back to metres, and the model faces -Z until rotated. */
    const matrix_t preTransform =
        XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) *
        XMMatrixRotationY(
            XMConvertToRadians(-90.f));

    auto pGunSlingerModel = CModel::Create(
        m_pDevice,
        m_pContext,
        MODEL::ANIM,
        "../Bin/Resources/LostArk/Character/GunSlinger/GunSlinger.wmodel",
        preTransform);

    if (nullptr == pGunSlingerModel)
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_Component_Model_GunSlinger"),
        move(pGunSlingerModel))))
        return E_FAIL;

    static const struct
    {
        const tchar_t* pTag;
        const char_t* pPath;
    } EquipmentModels[] =
    {
        { TEXT("Prototype_Component_Model_GunSlinger_Upper"),
          "../Bin/Resources/LostArk/Character/GunSlinger/GunSlinger_Upper.wmodel" },
        { TEXT("Prototype_Component_Model_GunSlinger_Lower"),
          "../Bin/Resources/LostArk/Character/GunSlinger/GunSlinger_Lower.wmodel" },
        { TEXT("Prototype_Component_Model_GunSlinger_Arm"),
          "../Bin/Resources/LostArk/Character/GunSlinger/GunSlinger_Arm.wmodel" },
        { TEXT("Prototype_Component_Model_GunSlinger_Shoulder"),
          "../Bin/Resources/LostArk/Character/GunSlinger/GunSlinger_Shoulder.wmodel" },
        { TEXT("Prototype_Component_Model_GunSlinger_Helmet"),
          "../Bin/Resources/LostArk/Character/GunSlinger/GunSlinger_Helmet.wmodel" },
    };

    for (const auto& equipmentModel : EquipmentModels)
    {
        if (FAILED(CGameInstance::Get().Add_Prototype(
            iLevelIndex,
            equipmentModel.pTag,
            CModel::Create(
                m_pDevice,
                m_pContext,
                MODEL::ANIM,
                equipmentModel.pPath,
                preTransform))))
            return E_FAIL;
    }

    /* WP_WGDH_02 is a three-gun set; the handgun is the one the spec sockets.

    No scaling here, unlike LanceMaster's weapon. The socket already carries the
    body's own pre-transform, so what matters is the weapon's size relative to
    the body in raw model units -- and this gun came out of the same psk export
    as the body, so it is already in the body's units (gun 30.4 against a body
    of 127.6). LanceMaster's lance measures 2.3 because it was cooked from a
    different source unit, which is why it needs the x100 and this does not. */
    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_Component_Model_GunSlinger_Weapon"),
        CModel::Create(
            m_pDevice,
            m_pContext,
            MODEL::NONANIM,
            "../Bin/Resources/LostArk/Character/WP_WGDH_02H/WP_WGDH_02H.wmodel",
            XMMatrixIdentity()))))
        return E_FAIL;

    return S_OK;
}

HRESULT CLoader::Ready_Artist_Prototypes(uint32_t iLevelIndex)
{
    /* Same contract as the other classes: the cook is centimetre, the loader
    brings it back to metres, and the model faces -Z until rotated. */
    const matrix_t preTransform =
        XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) *
        XMMatrixRotationY(
            XMConvertToRadians(-90.f));

    auto pArtistModel = CModel::Create(
        m_pDevice,
        m_pContext,
        MODEL::ANIM,
        "../Bin/Resources/LostArk/Character/Artist/Artist.wmodel",
        preTransform);

    if (nullptr == pArtistModel)
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_Component_Model_Artist"),
        move(pArtistModel))))
        return E_FAIL;

    static const struct
    {
        const tchar_t* pTag;
        const char_t* pPath;
    } EquipmentModels[] =
    {
        { TEXT("Prototype_Component_Model_Artist_Upper"),
          "../Bin/Resources/LostArk/Character/Artist/Artist_Upper.wmodel" },
        { TEXT("Prototype_Component_Model_Artist_Lower"),
          "../Bin/Resources/LostArk/Character/Artist/Artist_Lower.wmodel" },
        { TEXT("Prototype_Component_Model_Artist_Arm"),
          "../Bin/Resources/LostArk/Character/Artist/Artist_Arm.wmodel" },
        { TEXT("Prototype_Component_Model_Artist_Shoulder"),
          "../Bin/Resources/LostArk/Character/Artist/Artist_Shoulder.wmodel" },
        { TEXT("Prototype_Component_Model_Artist_Helmet"),
          "../Bin/Resources/LostArk/Character/Artist/Artist_Helmet.wmodel" },
    };

    for (const auto& equipmentModel : EquipmentModels)
    {
        if (FAILED(CGameInstance::Get().Add_Prototype(
            iLevelIndex,
            equipmentModel.pTag,
            CModel::Create(
                m_pDevice,
                m_pContext,
                MODEL::ANIM,
                equipmentModel.pPath,
                preTransform))))
            return E_FAIL;
    }

    /* No scaling, for the same reason GunSlinger's gun needs none: the brush was
    exported from the same psk set as the body, so it is already in the body's
    units (brush 100.4 long against a body 102.6 tall) and the socket carries the
    body's pre-transform. LanceMaster's lance measures 2.3 because it came from a
    different source unit, which is why that one needs the x100. */
    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_Component_Model_Artist_Weapon"),
        CModel::Create(
            m_pDevice,
            m_pContext,
            MODEL::NONANIM,
            "../Bin/Resources/LostArk/Character/WP_WSDM_09/WP_WSDM_09.wmodel",
            XMMatrixIdentity()))))
        return E_FAIL;

    return S_OK;
}

HRESULT CLoader::Ready_Slayer_Prototypes(uint32_t iLevelIndex)
{
    /* Same contract as the other classes: the cook is centimetre, the loader
    brings it back to metres, and the model faces -Z until rotated. */
    const matrix_t preTransform =
        XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) *
        XMMatrixRotationY(
            XMConvertToRadians(-90.f));

    auto pSlayerModel = CModel::Create(
        m_pDevice,
        m_pContext,
        MODEL::ANIM,
        "../Bin/Resources/LostArk/Character/Slayer/Slayer.wmodel",
        preTransform);

    if (nullptr == pSlayerModel)
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_Component_Model_Slayer"),
        move(pSlayerModel))))
        return E_FAIL;

    static const struct
    {
        const tchar_t* pTag;
        const char_t* pPath;
    } EquipmentModels[] =
    {
        { TEXT("Prototype_Component_Model_Slayer_Upper"),
          "../Bin/Resources/LostArk/Character/Slayer/Slayer_Upper.wmodel" },
        { TEXT("Prototype_Component_Model_Slayer_Lower"),
          "../Bin/Resources/LostArk/Character/Slayer/Slayer_Lower.wmodel" },
        { TEXT("Prototype_Component_Model_Slayer_Arm"),
          "../Bin/Resources/LostArk/Character/Slayer/Slayer_Arm.wmodel" },
        { TEXT("Prototype_Component_Model_Slayer_Shoulder"),
          "../Bin/Resources/LostArk/Character/Slayer/Slayer_Shoulder.wmodel" },
        { TEXT("Prototype_Component_Model_Slayer_Helmet"),
          "../Bin/Resources/LostArk/Character/Slayer/Slayer_Helmet.wmodel" },
    };

    for (const auto& equipmentModel : EquipmentModels)
    {
        if (FAILED(CGameInstance::Get().Add_Prototype(
            iLevelIndex,
            equipmentModel.pTag,
            CModel::Create(
                m_pDevice,
                m_pContext,
                MODEL::ANIM,
                equipmentModel.pPath,
                preTransform))))
            return E_FAIL;
    }

    /* No scaling, same reasoning as the gun and the brush: the greatsword came
    out of the same psk export as the body, so it is already in the body's units.
    It is the longest weapon so far though -- 209.1 against a body 129.6 tall --
    so this is the one to look at first if the proportions read wrong on screen. */
    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_Component_Model_Slayer_Weapon"),
        CModel::Create(
            m_pDevice,
            m_pContext,
            MODEL::NONANIM,
            "../Bin/Resources/LostArk/Character/WP_WWBK_03/WP_WWBK_03.wmodel",
            XMMatrixIdentity()))))
        return E_FAIL;

    return S_OK;
}

HRESULT CLoader::Ready_For_Level_GamePlay()
{
    lstrcpy(m_szLoadingText, TEXT("텍스쳐를 로딩중입니다."));
    /* For.Prototype_Component_Texture_Terrain */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Terrain"),
        CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Tile%d.dds"), 2))))
        return E_FAIL;

    /* For.Prototype_Component_Texture_Terrain_Mask */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Terrain_Mask"),
        CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/MyMask.dds"), 1))))
        return E_FAIL;

    /* For.Prototype_Component_Texture_Sky */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Sky"),
        CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/SkyBox/Sky_%d.dds"), 4))))
        return E_FAIL;

    /* For.Prototype_Component_Texture_Snow */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Snow"),
        CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Snow/Snow.png"), 1))))
        return E_FAIL;


    /* For.Prototype_Component_Texture_Effect */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Effect"),
        CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Explosion/Explosion%d.png"), 90))))
        return E_FAIL;



    lstrcpy(m_szLoadingText, TEXT("모델을 로딩중입니다."));
    /* For.Prototype_Component_VIBuffer_Terrain */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_VIBuffer_Terrain"),
        CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Height.bmp")))))
        return E_FAIL;

    /* For.Prototype_Component_VIBuffer_Cube */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_VIBuffer_Cube"),
        CVIBuffer_Cube::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    matrix_t            PreTransformMatrix = XMMatrixIdentity();
   
    /* For.Prototype_Component_Model_Fiona */
    PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f));
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fiona"),
        CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Fiona/Fiona.fbx", PreTransformMatrix))))
        return E_FAIL;

    /* For.Prototype_Component_Model_ForkLift */
    PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.f));
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_ForkLift"),
        CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/ForkLift/ForkLift.fbx", PreTransformMatrix))))
        return E_FAIL;

    /* For.Prototype_Component_VIBuffer_Instance_Point_Explosion */
    CVIBuffer_Instance_Point::INSTANCE_PARTICLE_DESC     ExploDesc{};
    ExploDesc.iNumInstance = 300;
    ExploDesc.vSize = float2_t(0.05f, 0.1f);
    ExploDesc.vCenter = float3_t(0.f, 0.f, 0.f);
    ExploDesc.vPivot = float3_t(0.f, 0.f, 0.f);
    ExploDesc.vRange = float3_t(0.1f, 0.1f, 0.1f);
    ExploDesc.vSpeed = float2_t(1.f, 2.f);
    ExploDesc.vLifeTime = float2_t(0.3f, 0.6f);
    ExploDesc.isLoop = true;

    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_VIBuffer_Instance_Point_Explosion"),
        CVIBuffer_Instance_Point::Create(m_pDevice, m_pContext, &ExploDesc))))
        return E_FAIL;


    /* For.Prototype_Component_VIBuffer_Instance_Rect_Snow */
    CVIBuffer_Instance_Rect::INSTANCE_PARTICLE_DESC     SnowDesc{};
    SnowDesc.iNumInstance = 3000;
    SnowDesc.vSize = float2_t(0.2f, 0.5f);
    SnowDesc.vCenter = float3_t(0.f, 0.f, 0.f);
    SnowDesc.vRange = float3_t(129.f, 1.f, 129.f);
    SnowDesc.vSpeed = float2_t(1.f, 4.f);
    SnowDesc.vLifeTime = float2_t(3.f, 5.f);
    SnowDesc.isLoop = true;

    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_VIBuffer_Instance_Rect_Snow"),
        CVIBuffer_Instance_Rect::Create(m_pDevice, m_pContext, &SnowDesc))))
        return E_FAIL;


    lstrcpy(m_szLoadingText, TEXT("셰이더를 로딩중입니다."));
    /* For.Prototype_Component_Shader_VtxNorTex */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxNorTex"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl"), VTXNORTEX::Elements, VTXNORTEX::iNumElements))))
        return E_FAIL;

    /* For.Prototype_Component_Shader_VtxMesh */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
        return E_FAIL;

    /* For.Prototype_Component_Shader_VtxAnimMesh */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;

    /* For.Prototype_Component_Shader_VtxCube */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxCube"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxCube.hlsl"), VTXCUBE::Elements, VTXCUBE::iNumElements))))
        return E_FAIL;

    /* For.Prototype_Component_Shader_VtxInstanceParticleRect */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxInstanceParticleRect"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstanceParticleRect.hlsl"), VTXINSTANCE_PARTICLE_RECT::Elements, VTXINSTANCE_PARTICLE_RECT::iNumElements))))
        return E_FAIL;

    /* For.Prototype_Component_Shader_VtxInstanceParticlePoint */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxInstanceParticlePoint"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstanceParticlePoint.hlsl"), VTXINSTANCE_PARTICLE_POINT::Elements, VTXINSTANCE_PARTICLE_POINT::iNumElements))))
        return E_FAIL;


    lstrcpy(m_szLoadingText, TEXT("네이게이션을 로딩중입니다."));
    /* For.Prototype_Component_Navigation */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Navigation"),
        CNavigation::Create(m_pDevice, m_pContext, TEXT("../Bin/DataFiles/Navigation.dat")))))
        return E_FAIL;

    /* For.Prototype_Component_TerrainNavigation */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_TerrainNavigation"),
        CNavigation::Create(m_pDevice, m_pContext, TEXT("../Bin/DataFiles/TerrainNavigation.dat"), TEXT("../Bin/DataFiles/NavigationNeighbors.dat")))))
        return E_FAIL;

    lstrcpy(m_szLoadingText, TEXT("충돌체를 로딩중입니다."));
    /* For.Prototype_Component_Collider_AABB */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_AABB"),
        CCollider::Create(m_pDevice, m_pContext, COLLIDER::AABB))))
        return E_FAIL;

    /* For.Prototype_Component_Collider_OBB */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_OBB"),
        CCollider::Create(m_pDevice, m_pContext, COLLIDER::OBB))))
        return E_FAIL;

    /* For.Prototype_Component_Collider_Sphere */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_Sphere"),
        CCollider::Create(m_pDevice, m_pContext, COLLIDER::SPHERE))))
        return E_FAIL;



    lstrcpy(m_szLoadingText, TEXT("객체원형을 로딩중입니다."));
    /* For.Prototype_GameObject_Terrain */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Terrain"),
        CTerrain::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_Camera_Free */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Camera_Free"),
        CCamera_Free::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_Monster */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Monster"),
        CMonster::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_Body_Player */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Body_Player"),
        CBody_Player::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_Weapon */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Weapon"),
        CWeapon::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_Player */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Player"),
        CPlayer::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_Sky */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Sky"),
        CSky::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_Snow */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Snow"),
        CSnow::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_Explosion */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Explosion"),
        CExplosion::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_ForkLift */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_ForkLift"),
        CForkLift::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_Effect */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Effect"),
        CEffect::Create(m_pDevice, m_pContext))))
        return E_FAIL;



    lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

    m_isFinished = true;
    return S_OK;
}

unique_ptr<CLoader> CLoader::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, LEVEL eNextLevelID)
{
    auto pInstance = unique_ptr<CLoader>(new CLoader(pDevice, pContext));

    if (FAILED(pInstance->Initialize(eNextLevelID)))
    {
        MSG_BOX("Failed to Created : CLoader");
        return nullptr;
    }

    return pInstance;
}

void CLoader::Free()
{
    WaitForSingleObject(m_hThread, INFINITE);

    CloseHandle(m_hThread);

    DeleteCriticalSection(&m_CriticalSection);
}
