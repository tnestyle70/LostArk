#include "UIObject.h"

#include "GameInstance.h"

CUIObject::CUIObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CGameObject { pDevice, pContext }
{
}

CUIObject::~CUIObject()
{
}

HRESULT CUIObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUIObject::Initialize(void* pArg)
{
    auto        pDesc = static_cast<UIOBJECT_DESC*>(pArg);

    m_fSizeX = pDesc->fSizeX;
    m_fSizeY = pDesc->fSizeY;

    m_fX = pDesc->fX;
    m_fY = pDesc->fY;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_pTransformCom->Scale(m_fSizeX, m_fSizeY);
    m_pTransformCom->Set_State(STATE::POSITION,
        XMVectorSet(
            m_fX - CGameInstance::Get().Get_ViewportSize().x * 0.5f, 
            -m_fY + CGameInstance::Get().Get_ViewportSize().y * 0.5f, 
            0.f, 1.f));

    /* View행렬 : 모든 객체를 카메라기준(원점)으로 재배치한다. -> 카메라가 바라본 장면을 투영해주기 위해서  */
    /* 근데 ui는? 카메라기준으로 표현하지 않는다 -> 뷰스페이스 변환행렬은 항등이면 충분. */
    XMStoreFloat4x4(&m_TransformMatrices[ETOUI(D3DTS::VIEW)], XMMatrixIdentity());


    /* 투영행렬 : 원근투영 -> 직교투영으로 찍어낸다. */
    /* 원근투영 : 화면에 보여질 영역을 카메라기준으로 다 셋팅을 한것이나 마찬가지. */
    /* 직교투영 : 인위적으로 보여질 영역(볼륨)을 뷰스페이스 내에서 설정한다. */
    XMStoreFloat4x4(&m_TransformMatrices[ETOUI(D3DTS::PROJ)],
        XMMatrixOrthographicLH(CGameInstance::Get().Get_ViewportSize().x, CGameInstance::Get().Get_ViewportSize().y, 0.f, 1.f));


       

    return S_OK;
}

void CUIObject::Priority_Update(f32_t fTimeDelta)
{
}

void CUIObject::Update(f32_t fTimeDelta)
{
}

void CUIObject::Late_Update(f32_t fTimeDelta)
{
}

HRESULT CUIObject::Render()
{
    return S_OK;
}

HRESULT CUIObject::Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
    return pShader->Bind_Matrix(pConstantName, &m_TransformMatrices[ETOUI(eType)]);    
}
