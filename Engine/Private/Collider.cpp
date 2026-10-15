// 원형이 공유하는 debug 자원과 clone별 형상의 생성·갱신·교차·표시를 구현한다.
#include "Collider.h"
// clone들이 공유하는 debug 도형 Batch를 생성하고 Begin/End로 제출 범위를 감싼다.
#include "DirectXTK/PrimitiveBatch.h"
// 색상 정점의 입력 요소와 그 개수로 D3D11 InputLayout을 만든다.
#include "DirectXTK/VertexTypes.h"
// BasicEffect를 생성하고 world/view/projection 및 정점 색상 사용을 설정한다.
#include "DirectXTK/Effects.h"

// 현재 카메라 View/Projection을 조회해 월드 형상의 debug 표시와 맞춘다.
#include "GameInstance.h"

CCollider::CCollider(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CComponent { pDevice, pContext }
{
}

CCollider::~CCollider()
{
}

// Loader가 OBB/구 prototype을 등록할 때 호출한다. InputLayout 실패는 Create의 후보 해제로 정리된다.
HRESULT CCollider::Initialize_Prototype(COLLIDER eType)
{
	m_eType = eType;

	m_pBatch = make_shared<PrimitiveBatch<VertexPositionColor>>(m_pContext.Get());
	m_pEffect = make_shared<BasicEffect>(m_pDevice.Get());
	m_pEffect->SetVertexColorEnabled(true);

	const void* pVertexShaderByteCode = { nullptr };
	size_t iLength = {};

	m_pEffect->GetVertexShaderBytecode(&pVertexShaderByteCode, &iLength);

	if (FAILED(m_pDevice->CreateInputLayout(
		VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		pVertexShaderByteCode, iLength, &m_pInputLayout)))
		return E_FAIL;

    return S_OK;
}

// Clone의 pArg는 선택한 형상에 맞는 descriptor여야 한다. switch의 END/잘못된 enum도 현재 S_OK를 반환한다.
// 하위 형상의 null/수치 검사를 여기서 추가하지 않으므로 S_OK를 모든 입력의 유효성 보장으로 해석하지 않는다.
HRESULT CCollider::Initialize(void* pArg)
{
	auto		pDesc = static_cast<CBounding::BOUNDING_DESC*>(pArg);

	switch (m_eType)
	{
	case COLLIDER::AABB:
		m_pBounding = CBounding_AABB::Create(m_pDevice, m_pContext, pDesc);
		break;
	case COLLIDER::OBB:
		m_pBounding = CBounding_OBB::Create(m_pDevice, m_pContext, pDesc);
		break;
	case COLLIDER::SPHERE:
		m_pBounding = CBounding_Sphere::Create(m_pDevice, m_pContext, pDesc);
		break;

	}

    return S_OK;
}

// 매번 로컬 원본에서 월드 형상을 계산한다. 이전 프레임 결과에 행렬을 누적 적용하지 않는다.
void CCollider::Update(fmatrix_t WorldMatrix)
{
	m_pBounding->Update(WorldMatrix);
}

// 자기 bounding의 마지막 결과를 갱신한다. 여러 대상을 검사하면 마지막 호출 결과가 debug 색으로 남는다.
bool_t CCollider::Intersect(shared_ptr<CCollider> pTargetCollider)
{

	return m_isColl = m_pBounding->Intersect(pTargetCollider->m_eType, pTargetCollider->m_pBounding);
}

// bounding이 이미 월드 좌표이므로 World는 항등 행렬이다. 공유 Effect/Batch와 렌더 context를 순서대로 사용한다.
HRESULT CCollider::Render()
{
	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	m_pEffect->SetProjection(XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(m_pInputLayout.Get());

	m_pEffect->Apply(m_pContext.Get());

	m_pBatch->Begin();

	// 하위 Render의 HRESULT를 별도로 전달하지 않고 Batch를 끝낸 뒤 S_OK를 반환한다.
	m_pBounding->Render(m_pBatch);

	m_pBatch->End();

	return S_OK;
}

// Loader가 형상 종류별 원형을 만든다. 부분 생성한 debug 자원은 실패 후보 소멸 시 공유/COM 참조로 정리된다.
unique_ptr<CCollider> CCollider::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, COLLIDER eType)
{
	auto pInstance = unique_ptr<CCollider>(new CCollider(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(eType)))
	{
		MSG_BOX("Failed to Created : CCollider");
		return nullptr;
	}

	return pInstance;
}

// CComponent 복사로 GPU 참조를 공유하고 Initialize가 clone 전용 형상을 만든다. 원본 prototype의 형상 정의는 바꾸지 않는다.
shared_ptr<CPrototype> CCollider::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CCollider>(new CCollider(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CCollider");
		return nullptr;
	}

	return pInstance;
}
