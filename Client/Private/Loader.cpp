#include "Loader.h"

#include "Player.h"
#include "Body_Player.h"
#include "Valtan.h"
#include "Body_Valtan.h"
#include "Part_Equipment.h"
#include "Part_Body.h"
#include "Character.h"
#include "MapAssetCatalog.h"
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
        MSG_BOX("Failed to Create Level!");
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
    lstrcpy(m_szLoadingText, TEXT("Loading Baren"));

    lstrcpy(m_szLoadingText, TEXT("Loading Baren Complete!"));

    m_isFinished = true;

    return S_OK;
}

HRESULT CLoader::Ready_For_Level_AssetTest()
{
    lstrcpy(m_szLoadingText, TEXT("Loading Binary Asset"));

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
        CShader::Create(m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
            VTXANIMMESH::Elements,
            VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Shader_VtxMeshBinary"),
        CShader::Create(m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
            VTXMESH::Elements,
            VTXMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(Ready_LanceMaster_Prototypes(
        ETOUI(LEVEL::ASSET_TEST))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Shader_VtxMeshMapInstance"),
        CShader::Create(m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl"),
            VTXMESHINSTANCE::Elements,
            VTXMESHINSTANCE::iNumElements))))
        return E_FAIL;

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

    if (!mapCatalog.Load_Default())
    {
        return E_FAIL;
    }

    for (const MAP_ASSET_ENTRY& entry : mapCatalog.Get_Entries())
    {
        const string modelPath = entry.resolvedModelPath.string();

        if (FAILED(CGameInstance::Get().Add_Prototype(
            ETOUI(LEVEL::ASSET_TEST), entry.prototypeTag,
            CModel::Create(m_pDevice, m_pContext,
                MODEL::NONANIM, modelPath.c_str(), mapAssetTransform))))
        {
            return E_FAIL;
        }
    }

    //Valtan Raid 맵 기준 동적 에셋 카탈로그
    CDeployPropCatalog deployCatalog;

    if (deployCatalog.Load_Default(mapCatalog.Get_AreaId()))
    {
        for (const DEPLOY_PROP_ASSET_ENTRY& entry : deployCatalog.Get_Assets())
        {
            const MODEL modelKind =
                entry.kind == DEPLOY_PROP_MODEL_KIND::ANIM ?
                MODEL::ANIM : MODEL::NONANIM;

            const string intactPath = entry.intactResolvedPath.string();

            if (FAILED(CGameInstance::Get().Add_Prototype(
                ETOUI(LEVEL::ASSET_TEST), entry.intactPrototypeTag,
                CModel::Create(m_pDevice, m_pContext,
                    modelKind, intactPath.c_str(), mapAssetTransform))))
                return E_FAIL;

            if (entry.kind == DEPLOY_PROP_MODEL_KIND::STATIC)
            {
                const string fracturedPath = entry.fracturedResolvedPath.string();
                if (FAILED(CGameInstance::Get().Add_Prototype(
                    ETOUI(LEVEL::ASSET_TEST), entry.fracturedPrototypeTag,
                    CModel::Create(m_pDevice, m_pContext,
                        MODEL::NONANIM, fracturedPath.c_str(), mapAssetTransform))))
                    return E_FAIL;
            }
        }
    }

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Model_Valtan"),
        CModel::Create(m_pDevice, m_pContext,
            MODEL::ANIM,
            "../Bin/Resources/LostArk/Character/Valtan/MN_RPBF_01.wmodel",
            lostArkAssetPreTransform))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Model_ValtanWeapon"),
        CModel::Create(m_pDevice, m_pContext,
            MODEL::NONANIM,
            "../Bin/Resources/LostArk/Character/Valtan/ValtanWeapon.wmodel",
            XMMatrixScaling(100.f, 100.f, 100.f)))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Navigation_ValtanArena"),
        CNavigation::Create_NavGrid(
            m_pDevice,
            m_pContext,
            TEXT("../DataFiles/Navigation/ValtanArena.navgrid")))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_Camera_Free"),
        CCamera_Free::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_Body_Valtan"),
        CBody_Valtan::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_Valtan"),
        CValtan::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_MapAsset"),
        CMapAssetObject::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_MapStaticBatch"),
        CMapStaticBatchObject::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_DeployProp"),
        CDeployPropObject::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    lstrcpy(m_szLoadingText, TEXT("Binary Asset Test Loading Complete"));

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

    if (FAILED(Ready_LanceMaster_Prototypes(
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
