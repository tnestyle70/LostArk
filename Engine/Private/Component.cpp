// 원형 경로에 장치 참조를 연결하고 기능 없는 기반 Component의 기본 성공 동작을 정의한다.
#include "Component.h"

CComponent::CComponent(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CPrototype { pDevice, pContext }
{
}

CComponent::~CComponent()
{
}

// 기반 Component는 준비할 자원이 없다. 필요한 원형 자원은 파생 클래스가 준비한다.
HRESULT CComponent::Initialize_Prototype()
{
	return S_OK;
}

// 기본 구현은 인자를 보관하지 않는다. 개별 초기화가 필요한 타입이 이 단계를 재정의한다.
HRESULT CComponent::Initialize(void* pArg)
{
	return S_OK; 
}
