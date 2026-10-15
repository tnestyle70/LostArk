// GameObject/Component의 기반 생성자가 공유하는 장치 참조와 원형/복제 표시를 구현한다.
#include "Prototype.h"

// 새 원형의 장치 COM 참조를 보관한다. 실제 메시·텍스처 준비는 파생 Initialize_Prototype에서 수행한다.
CPrototype::CPrototype(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_isCloned { false }
{
}

// 파생 클래스의 복제 생성자가 이 경로를 사용한다. 장치 참조는 공유하고 복제 표시만 바꾼다.
// 메시·텍스처 등의 공유 여부와 개별 실행 상태 복사는 각 파생 클래스가 결정한다.
CPrototype::CPrototype(const CPrototype& Prototype)
	: m_pDevice{ Prototype.m_pDevice }
	, m_pContext{ Prototype.m_pContext }
	, m_isCloned{ true }
{
}


CPrototype::~CPrototype()
{
}
