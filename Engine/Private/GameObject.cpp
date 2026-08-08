#include "GameObject.h"

#include "GameInstance.h"

CGameObject::CGameObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CPrototype { pDevice, pContext }    
{
}

CGameObject::~CGameObject()
{
    Free();
}

shared_ptr<CComponent> CGameObject::Get_Component(const wstring_t& strComponentTag)
{
    auto    iter = m_Components.find(strComponentTag);
    if (iter == m_Components.end())
        return nullptr;

    return iter->second;
}

HRESULT CGameObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CGameObject::Initialize(void* pArg)
{
    if (nullptr != pArg)
    {
        auto    pDesc = static_cast<CGameObject::GAMEOBJECT_DESC*>(pArg);

        /* 나에게 채워야할 데이터가 있다면 채워주자 .*/

    }

    m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
    if (nullptr == m_pTransformCom)
        return E_FAIL;

    if (FAILED(m_pTransformCom->Initialize(pArg)))
        return E_FAIL;

    m_Components.emplace(g_strTransformComTag, m_pTransformCom);

    return S_OK;
}

void CGameObject::Priority_Update(f32_t fTimeDelta)
{
}

void CGameObject::Update(f32_t fTimeDelta)
{
}

void CGameObject::Post_Physics_Update(f32_t fTimeDelta)
{
}

void CGameObject::Late_Update(f32_t fTimeDelta)
{
}

HRESULT CGameObject::Render()
{
    return S_OK;
}

HRESULT CGameObject::Render_Shadow()
{
    return S_OK;
}

CComponent* CGameObject::Find_Component(const wstring_t& strComponentTag)
{
    auto    iter = m_Components.find(strComponentTag);
    if(iter == m_Components.end())
        return nullptr;

    return iter->second.get();
}

//template<typename T>
//HRESULT CGameObject::Add_Component(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, const wstring_t& strComponentTag, shared_ptr<T>& pOut, void* pArg)
//{
//   
//}

void CGameObject::Free()
{
    m_Components.clear();
}
