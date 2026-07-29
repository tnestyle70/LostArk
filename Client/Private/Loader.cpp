#include "Loader.h"

#include "Player.h"
#include "Body_Player.h"
#include "Valtan.h"
#include "Body_Valtan.h"
#include "MapAssetCatalog.h"
#include "MapAssetObject.h"
#include "MapAssetPreview.h"


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
        return E_FAIL;

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
    lstrcpy(m_szLoadingText, TEXT("텍스쳐를 로딩중입니다."));
    /* For.Prototype_Component_Texture_BackGround */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::LOGO), TEXT("Prototype_Component_Texture_BackGround"),
        CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Default%d.jpg"), 2))))
        return E_FAIL;


    lstrcpy(m_szLoadingText, TEXT("모델을 로딩중입니다."));


    lstrcpy(m_szLoadingText, TEXT("셰이더를 로딩중입니다."));


    lstrcpy(m_szLoadingText, TEXT("객체원형을 로딩중입니다."));

    /* For.Prototype_GameObject_BackGround */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_GameObject_BackGround"),
        CBackGround::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

    m_isFinished = true;

    return S_OK;
}

HRESULT CLoader::Ready_For_Level_AssetTest()
{
    lstrcpy(m_szLoadingText, TEXT("바이너리 에셋 테스트 자원을 로딩중입니다."));

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

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Model_Valtan"),
        CModel::Create(m_pDevice, m_pContext,
            MODEL::ANIM,
            "../Bin/Resources/LostArk/Character/MN_RPBF_01/MN_RPBF_01.wmodel",
            lostArkAssetPreTransform))))
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

    lstrcpy(m_szLoadingText, TEXT("바이너리 에셋 테스트 로딩이 완료되었습니다."));

    m_isFinished = true;

    return S_OK;
}

HRESULT CLoader::Ready_For_Test_Level2()
{
    lstrcpy(m_szLoadingText, TEXT("Loading Test Level 2 resources."));

    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::TEST_LEVEL2),
        TEXT("Prototype_GameObject_Camera_Free"),
        CCamera_Free::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    lstrcpy(m_szLoadingText, TEXT("Test Level 2 loading complete."));
    m_isFinished = true;

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
