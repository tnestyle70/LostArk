#pragma once

#include "Prototype.h"

NS_BEGIN(Engine)

class ENGINE_DLL CComponent abstract : public CPrototype
{
protected:
	CComponent(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CComponent();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

#ifdef _DEBUG
public:
	virtual HRESULT Render() { return S_OK; }

#endif

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
	
};

NS_END