#pragma once

// 원형/복제본의 shared_ptr 기반 선언, D3D11 장치의 ComPtr와 DLL/공통 타입을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// GameObject와 Component가 공통으로 따르는 원형·복제 계약이다.
// Prototype_Manager는 등록 원형을 소유하고, Clone의 결과는 호출자에게 전달한다.
class ENGINE_DLL CPrototype abstract : public enable_shared_from_this<CPrototype>
{
protected:
	CPrototype(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	CPrototype(const CPrototype& Prototype);
public:
	virtual ~CPrototype();

public:
	// 원형의 공용 자원 준비와 복제 객체의 개별 상태 초기화를 구분한다.
	// 구체 타입이 두 단계를 구현하며 실패 결과는 생성·복제 호출자가 처리한다.
	virtual HRESULT Initialize_Prototype() = 0;
	virtual HRESULT Initialize(void* pArg) = 0;

protected:
	// COM 참조로 장치 수명을 공유한다. 복사 생성해도 장치·컨텍스트 자체를 복제하지 않는다.
	ComPtr<ID3D11Device>				m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>			m_pContext = { nullptr };
	// 기본 생성은 원형(false), 복사 생성은 복제본(true)으로 표시한다.
	bool_t								m_isCloned = { false };

public:
	// pArg의 실제 타입·사용 기간은 구체 클래스의 Initialize 계약을 따른다.
	// 반환된 객체의 수명은 shared_ptr 소유자들이 관리한다.
	// Prototype_Manager는 이 반환값을 그대로 전달한다. 파생 Clone이 새 객체 초기화에 실패하면 후보를 정리해 nullptr를 반환해야 한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
};

NS_END