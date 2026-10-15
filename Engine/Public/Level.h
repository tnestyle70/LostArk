#pragma once

// DLL/namespace 선언과 D3D11 장치·context의 ComPtr, 생명주기 반환값/시간 타입을 사용한다.
#include "Engine_Defines.h"

/* 1. 클라이언트 개발자가 제작한 모든 레벨들의 부모가 되는 클래스타입. */
// Client의 구체 Level이 상속하는 생명주기 경계다. 활성 Level의 단독 소유자는 CLevel_Manager다.
// 이 기반 클래스는 장치 참조만 보유하며, 해당 Level의 객체와 콘텐츠는 파생 구현이 준비한다.
NS_BEGIN(Engine)

class ENGINE_DLL CLevel abstract 
{
protected:
	CLevel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CLevel();

public:
	// 기본 구현은 작업 없이 성공한다. 실제 로드 실패는 파생 Level의 Initialize가 반환한다.
	virtual HRESULT Initialize();
	// CGameInstance가 Level_Manager를 통해 호출하며 fTimeDelta의 단위는 초다.
	virtual void Update(f32_t fTimeDelta);
	// Level_Manager가 전달하는 Level별 추가 그리기다. 기본 구현은 성공하고 장치 context의 동시 접근은 조정하지 않는다.
	virtual HRESULT Render();

protected:
	// COM 참조로 장치 수명을 공유한다. Level을 교체해도 다른 소유자가 사용하는 장치는 유지된다.
	ComPtr<ID3D11Device>			m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>		m_pContext = { nullptr };

};

NS_END