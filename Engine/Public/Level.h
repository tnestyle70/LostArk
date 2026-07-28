#pragma once

#include "Engine_Defines.h"

/* 1. 클라이언트 개발자가 제작한 모든 레벨들의 부모가 되는 클래스타입. */

NS_BEGIN(Engine)

class ENGINE_DLL CLevel abstract 
{
protected:
	CLevel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CLevel();

public:
	virtual HRESULT Initialize();
	virtual void Update(f32_t fTimeDelta);
	virtual HRESULT Render();

protected:
	ComPtr<ID3D11Device>			m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>		m_pContext = { nullptr };

};

NS_END