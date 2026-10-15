// Component 소유, 원형/개별 초기화와 렌더/프레임 기본 동작을 구현한다.
#include "GameObject.h"

// 기존 엔진 접근 헤더의 포함 경계를 유지한다. 이 CPP 본문에는 GameInstance 직접 호출이 없고 원형 복제는 공개 템플릿에 있다.
#include "GameInstance.h"
// 표시/경계 변경 때 소속 Layer의 dirty 목록에 재계산 알림을 보낸다.
#include "Layer.h"

CGameObject::CGameObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CPrototype { pDevice, pContext }    
{
}

// map의 소유 참조를 먼저 해제한다. 전용 멤버와 외부 shared_ptr의 수명은 각각의 소유자에게 남는다.
CGameObject::~CGameObject()
{
    Free();
}

// Component tag를 조회해 소유권을 공유한다. 없는 tag는 nullptr다.
shared_ptr<CComponent> CGameObject::Get_Component(const wstring_t& strComponentTag)
{
    auto    iter = m_Components.find(strComponentTag);
    if (iter == m_Components.end())
        return nullptr;

    return iter->second;
}

// 기반 GameObject 원형에는 별도 자원 로드가 없다. 파생 원형이 필요한 자원을 준비한다.
HRESULT CGameObject::Initialize_Prototype()
{
    return S_OK;
}

// Clone의 구체 초기화가 기반 Transform 준비에 사용하는 경로다. 실패를 반환해도 이 객체의
// 이전 상태로 되돌리는 함수는 아니므로, 실패한 새 Clone을 버리는 것은 상위 생성 경로의 책임이다.
HRESULT CGameObject::Initialize(void* pArg)
{
    if (nullptr != pArg)
    {
        auto    pDesc = static_cast<CGameObject::GAMEOBJECT_DESC*>(pArg);

        /* 나에게 채워야할 데이터가 있다면 채워주자 .*/
        // 현재 기반 구현은 별도 GameObject 필드를 읽지 않고 아래 Transform 초기화에 같은 인자를 넘긴다.

    }

    // 기본 Transform의 생성·개별 초기화를 마친 뒤 map에 공유 참조를 등록한다. 실패 시 이 객체 내부의 중간 상태를 되돌리지는 않는다.
    m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
    if (nullptr == m_pTransformCom)
        return E_FAIL;

    if (FAILED(m_pTransformCom->Initialize(pArg)))
        return E_FAIL;

    m_Components.emplace(g_strTransformComTag, m_pTransformCom);

    return S_OK;
}

// 네 기본 프레임 callback은 아무 상태도 바꾸지 않는다. 파생 객체가 필요한 단계만 재정의한다.
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

// 등록된 Layer가 있을 때만 bounds/표시 정책 변경을 알린다. 등록 전 객체에는 알릴 대상이 없다.
void CGameObject::Invalidate_FinalCameraSpatialBounds()
{
    if (m_FinalCameraLayer.Owner)
        m_FinalCameraLayer.Owner->Invalidate_FinalCameraSpatialBounds(this);
}

// 기본 객체는 제출할 draw가 없어 성공한다. 실제 표시가 필요한 파생 클래스는 이 경계를 재정의한다.
HRESULT CGameObject::Render()
{
    return S_OK;
}

// 그룹별 렌더를 재정의하지 않은 객체는 기존 Render 한 곳으로 전달한다.
HRESULT CGameObject::Render_Group(RENDERGROUP group)
{
    UNREFERENCED_PARAMETER(group);
    return Render();
}

// 기본 객체는 자기 한 항목만 소비한다. 큐의 첫 객체가 자신인지 확인하고 결과를 Renderer에 돌려준다.
HRESULT CGameObject::Render_AdjacentNonBlend(
    std::span<const std::shared_ptr<CGameObject>> objects, size_t& consumed)
{
    // 입력 검사보다 먼저 소비 개수를 기록하는 기존 계약이다. Renderer는 실패 결과를 먼저 처리해야 한다.
    consumed = 1u;
    if (objects.empty() || objects.front().get() != this)
        return E_INVALIDARG;
    return Render_Group(RENDERGROUP::NONBLEND);
}

// 추가 overlay/그림자가 없는 기본 객체의 성공 경로다. 실제 그리기는 파생 클래스가 재정의한다.
HRESULT CGameObject::Render_DeferredOverlay()
{
    return S_OK;
}

HRESULT CGameObject::Render_Shadow()
{
    return S_OK;
}

// Add_Component 등 객체 내부 조회용 차용 포인터다. 조회가 소유권을 추가하지 않는다.
CComponent* CGameObject::Find_Component(const wstring_t& strComponentTag)
{
    auto    iter = m_Components.find(strComponentTag);
    if(iter == m_Components.end())
        return nullptr;

    return iter->second.get();
}

// Component map만 비운다. m_pTransformCom 같은 전용 소유 참조를 여기서 reset하지는 않는다.
void CGameObject::Free()
{
    m_Components.clear();
}
