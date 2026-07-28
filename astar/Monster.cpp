#include "Monster.h"

#include "GameInstance.h"
#include "PartObject.h"
#include "ContainerObject.h"


CMonster::CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject { pDevice, pContext }
{
}

CMonster::CMonster(const CMonster& Prototype)
    : CGameObject { Prototype }
{
}

HRESULT CMonster::Initialize_Prototype()
{
    
    return S_OK;
}

HRESULT CMonster::Initialize(void* pArg)
{
    CGameObject::GAMEOBJECT_DESC        Desc{};
    Desc.fSpeedPerSec = 1.f;
    Desc.fRotationPerSec = 0.f;

    if (FAILED(__super::Initialize(&Desc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(m_pNavigationCom->Get_CellPos(), 1.f));  

    m_pModelCom->Set_Animation(rand() % 20, true);

    return S_OK;
}

void CMonster::Update_Priority(_float fTimeDelta)
{
    int a = 10;
}

void CMonster::Update(_float fTimeDelta)
{     
    m_pTransformCom->Go_Astar(fTimeDelta/*0.00016f*/, m_pNavigationCom, 10);

    m_pModelCom->Play_Animation(fTimeDelta);
   
    if (true == m_pModelCom->is_AnimFinished())
        int a = 10;

    for (size_t i = 0; i < ENUM_TO_UINT(COLLIDER::END); i++)
    {
        m_pColliderCom[i]->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    }

    Collision_ToPlayer();

    
        
}

void CMonster::Update_Late(_float fTimeDelta)
{
#ifdef _DEBUG
    for (size_t i = 0; i < ENUM_TO_UINT(COLLIDER::END); i++)
        m_pGameInstance->Add_DebugComponent(m_pColliderCom[i]);
#endif

    m_pGameInstance->Add_RenderObject(RENDERGROUP::NONBLEND, this);
}

HRESULT CMonster::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;    

    _uint   iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
            return E_FAIL;
        /*m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);*/

        if (FAILED(m_pModelCom->Bind_Bones(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        m_pModelCom->Render(i);
    }



    return S_OK;
}


HRESULT CMonster::Ready_Components()
{
    /* For.Com_Model */
    if (FAILED(__super::Add_Component(ENUM_TO_UINT(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fiona"),
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
        return E_FAIL;

    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(ENUM_TO_UINT(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    /* For.Com_Navigation */
    CNavigation::NAVIGATION_DESC        NavigationDesc{};
    NavigationDesc.iCurrentCellIndex = rand() % 25000;

    if (FAILED(__super::Add_Component(ENUM_TO_UINT(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Navigation"),
        TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom), &NavigationDesc)))
        return E_FAIL;

    /* For.Com_Collider_AABB */
    CBounding_AABB::BOUNDING_AABB_DESC      AABBDesc{};
    AABBDesc.vExtents = _float3(0.4f, 0.6f, 0.4f);
    AABBDesc.vCenter = _float3(0.f, AABBDesc.vExtents.y, 0.f);

    if (FAILED(__super::Add_Component(ENUM_TO_UINT(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_AABB"),
        TEXT("Com_Collider_AABB"), reinterpret_cast<CComponent**>(&m_pColliderCom[ENUM_TO_UINT(COLLIDER::AABB)]), &AABBDesc)))
        return E_FAIL;

    /* For.Com_Collider_Sphere */
    CBounding_Sphere::BOUNDING_SPHERE_DESC      SphereDesc{};
    SphereDesc.fRadius = 0.5f;
    SphereDesc.vCenter = _float3(0.f, SphereDesc.fRadius, 0.f);

    if (FAILED(__super::Add_Component(ENUM_TO_UINT(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_Sphere"),
        TEXT("Com_Collider_Sphere"), reinterpret_cast<CComponent**>(&m_pColliderCom[ENUM_TO_UINT(COLLIDER::SPHERE)]), &SphereDesc)))
        return E_FAIL;

    /* For.Com_Collider_OBB */
    CBounding_OBB::BOUNDING_OBB_DESC      OBBDesc{};
    OBBDesc.vAngles = _float3(0.0f, 45.0F, 0.0f);
    OBBDesc.vExtents = _float3(0.4f, 0.4f, 0.4f);
    OBBDesc.vCenter = _float3(0.f, OBBDesc.vExtents.y, 0.f);

    if (FAILED(__super::Add_Component(ENUM_TO_UINT(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_OBB"),
        TEXT("Com_Collider_OBB"), reinterpret_cast<CComponent**>(&m_pColliderCom[ENUM_TO_UINT(COLLIDER::OBB)]), &OBBDesc)))
        return E_FAIL;



    return S_OK;
}

HRESULT CMonster::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Bind_PipeLineMatrix(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Bind_PipeLineMatrix(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;
  
    return S_OK;
}

void CMonster::Collision_ToPlayer()
{
    //CGameObject*        pPlayer = m_pGameInstance->Find_GameObject(ENUM_TO_UINT(LEVEL::GAMEPLAY), TEXT("Layer_Player"));
    //if (nullptr == pPlayer)
    //    return;

    //CCollider*          pTargetCollider = static_cast<CCollider*>(pPlayer->Get_Component(TEXT("Com_Collider_AABB")));
    //if (nullptr == pTargetCollider)
    //    return;

    //m_pColliderCom[ENUM_TO_UINT(COLLIDER::AABB)]->Intersect(pTargetCollider);

    CGameObject* pPlayer = m_pGameInstance->Find_GameObject(ENUM_TO_UINT(LEVEL::GAMEPLAY), TEXT("Layer_Player"));
    if (nullptr == pPlayer)
        return;

    CGameObject* pTargetObject = dynamic_cast<CContainerObject*>(pPlayer)->Find_PartObject(TEXT("Part_Weapon"));
    if (nullptr == pTargetObject)
        return;
    CCollider* pTargetCollider = static_cast<CCollider*>(pTargetObject->Get_Component(TEXT("Com_Collider_OBB")));
    if (nullptr == pTargetCollider)
        return;

    m_pColliderCom[ENUM_TO_UINT(COLLIDER::OBB)]->Intersect(pTargetCollider);





}

CMonster* CMonster::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMonster* pInstance = new CMonster(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CMonster");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CMonster::Clone(void* pArg)
{
    CMonster* pInstance = new CMonster(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CMonster");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMonster::Free() 
{
    __super::Free();

    for (auto& pCollider : m_pColliderCom)
        Safe_Release(pCollider);

    Safe_Release(m_pNavigationCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
