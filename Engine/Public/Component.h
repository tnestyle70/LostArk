#pragma once

// Component가 상속하는 원형 초기화·개별 초기화·Clone과 장치 참조의 공통 기반이다.
#include "Prototype.h"

NS_BEGIN(Engine)

// GameObject가 조합해 사용하는 기능의 공통 기반이다. 생성·복제 계약은 CPrototype을 따른다.
class ENGINE_DLL CComponent abstract : public CPrototype
{
protected:
	CComponent(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CComponent();

public:
	// 기반 구현은 자원을 만들지 않고 S_OK다. 파생 타입이 원형에 공유할 자원을 준비한다.
	virtual HRESULT Initialize_Prototype() override;
	// 기반 구현은 pArg를 읽거나 보관하지 않고 S_OK다. 실제 입력 타입과 복제 상태는 파생 구현이 정한다.
	virtual HRESULT Initialize(void* pArg) override;

public:
	// 그리기 동작이 없는 Component도 존재하므로 기본 구현은 작업 없이 성공한다.
	virtual HRESULT Render()
	{
		return S_OK;
	}

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
};

NS_END