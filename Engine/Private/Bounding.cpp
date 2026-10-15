// 형상 공통 계층의 장치 참조와 기본 초기화 경계를 구현한다.
#include "Bounding.h"

// 장치는 ComPtr로 소유한다. 현재 생성자는 pContext를 멤버에 보관하지 않으며 파생 Render는 전달받은 Batch를 사용한다.
CBounding::CBounding(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : m_pDevice { pDevice }
{
}

CBounding::~CBounding()
{
}

// 기본 계층에는 공통 초기화/검증 동작이 없다. 파생 Initialize가 형상별 입력을 해석한다.
HRESULT CBounding::Initialize(const CBounding::BOUNDING_DESC* pArg)
{
    return S_OK;
}
