// 부모 행렬의 차용과 자식×부모 합성 결과의 저장/셰이더 전달을 구현한다.
#include "PartObject.h"

// 기존 간접 include 경계를 유지한다. 이 본문은 GameInstance를 직접 호출하지 않고 Shader의 Bind_Matrix를 사용한다.
#include "GameInstance.h"

CPartObject::CPartObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CPartObject::~CPartObject()
{
}

// 기반 Part의 원형에는 추가 자원이 없어 성공한다.
HRESULT CPartObject::Initialize_Prototype()
{
	return S_OK;
}

// 구체 Part의 Clone 초기화가 부모 행렬 주소를 넘긴다. 주소를 저장하므로 descriptor 자체는 임시여도
// 그 descriptor가 가리킨 부모 행렬은 이후 Update_CombinedWorldMatrix 호출까지 살아 있어야 한다.
HRESULT CPartObject::Initialize(void* pArg)
{
	auto		pDesc = static_cast<PARTOBJECT_DESC*>(pArg);
	m_pParentMatrix = pDesc->pParentMatrix;

	// 부모 행렬 주소를 먼저 저장한 상태에서 GameObject 초기화를 호출한다. 실패해도 이미 저장한 주소를 되돌리지 않는다.
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

// 기본 Part의 프레임 콜백은 비어 있다. 장비/무기 등 구체 Part가 부착과 제출 동작을 구현한다.
void CPartObject::Priority_Update(f32_t fTimeDelta)
{
}

void CPartObject::Update(f32_t fTimeDelta)
{
}

void CPartObject::Late_Update(f32_t fTimeDelta)
{
}

// 기반 클래스에는 실제 draw가 없어 성공한다.
HRESULT CPartObject::Render()
{
	return S_OK;
}

// 구체 Part가 local/소켓 행렬을 전달한다. 부모까지 합성한 월드 행렬은 다음 shader 바인딩이 소비한다.
HRESULT CPartObject::Update_CombinedWorldMatrix(fmatrix_t ChildMatrix)
{
	XMStoreFloat4x4(&m_CombinedWorldMatrix, ChildMatrix * XMLoadFloat4x4(m_pParentMatrix));

	return S_OK;
}

// 합성 상태를 다시 계산하지 않고 현재 저장값을 전달한다. 실패 HRESULT는 호출자에게 그대로 반환한다.
HRESULT CPartObject::Bind_WorldMatrix(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	return pShader->Bind_Matrix(pConstantName, &m_CombinedWorldMatrix);	
}
