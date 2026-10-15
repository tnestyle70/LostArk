// UI 기반 객체의 수명·초기화·렌더 훅을 구현한다.
#include "UIObject.h"

// 현재 viewport 크기와 Transform/CShader 인터페이스를 기존 엔진 경로로 가져온다.
#include "GameInstance.h"

// 장치/context 수명은 CGameObject가 가진 COM 참조로 유지한다.
CUIObject::CUIObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CGameObject { pDevice, pContext }
{
}

CUIObject::~CUIObject()
{
}

// 공통 UI Prototype에는 추가 준비 단계가 없다. 파생 타입이 필요한 컴포넌트 원형을 등록한다.
HRESULT CUIObject::Initialize_Prototype()
{
    return S_OK;
}

// 파생 객체 초기화가 유효한 UIOBJECT_DESC를 전달한다. 입력 필드를 먼저 저장하므로 기본 객체 초기화 실패 때도 이 필드는 남는다.
HRESULT CUIObject::Initialize(void* pArg)
{
    auto        pDesc = static_cast<UIOBJECT_DESC*>(pArg);

    m_fSizeX = pDesc->fSizeX;
    m_fSizeY = pDesc->fSizeY;

    m_fX = pDesc->fX;
    m_fY = pDesc->fY;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    // 좌상단 원점·아래로 증가하는 화면 픽셀을 중심 원점·위로 증가하는 UI 좌표로 옮긴다.
    // 폭/높이로 사각형을 확대하며 이때 읽은 viewport에 맞춰 위치와 투영을 구성한다.
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
        XMMatrixOrthographicLH(
            CGameInstance::Get().Get_ViewportSize().x,
            CGameInstance::Get().Get_ViewportSize().y, 0.f, 1.f));

    return S_OK;
}

// 기본 프레임 훅은 상태를 변경하지 않는다. 파생 UI가 필요한 업데이트만 구현한다.
void CUIObject::Priority_Update(f32_t fTimeDelta)
{
}

void CUIObject::Update(f32_t fTimeDelta)
{
}

void CUIObject::Late_Update(f32_t fTimeDelta)
{
}

// 기본 UI 객체 자체는 그리지 않는다. 파생 Render의 공통 성공 반환을 제공한다.
HRESULT CUIObject::Render()
{
    return S_OK;
}

// VIEW/PROJ 등 요청한 저장 행렬을 그대로 전달한다. 월드 행렬은 필요 시 Transform의 별도 바인딩 경로를 사용한다.
HRESULT CUIObject::Bind_ShaderResource(
    shared_ptr<class CShader> pShader,
    const char_t* pConstantName, D3DTS eType)
{
    return pShader->Bind_Matrix(pConstantName, &m_TransformMatrices[ETOUI(eType)]);    
}
