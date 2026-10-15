// 파생 Level이 공유하는 장치 참조와 기본 생명주기 함수의 구현을 제공한다.
#include "Level.h"

// 파생 Level 생성자가 장치 참조를 전달한다. 별도 장치나 컨텍스트를 새로 만들지 않는다.
CLevel::CLevel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{

}

CLevel::~CLevel()
{

}

// 공통 기반에는 로드할 콘텐츠가 없으므로 구체 Level이 초기화 동작을 재정의한다.
HRESULT CLevel::Initialize()
{
	return S_OK;
}

// 기본 Level 갱신에는 상태 변화가 없다. Debug의 지역 값은 전달된 시간 확인 용도다.
void CLevel::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
	[[maybe_unused]] volatile f32_t fDebugTimeDelta = fTimeDelta;
#endif
}

// Level별 추가 렌더가 없을 때의 기본 성공 경로다. 객체 그리기는 별도 Renderer 경로가 담당한다.
HRESULT CLevel::Render()
{
	return S_OK;
}
