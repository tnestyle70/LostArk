#pragma once

#include "VIBuffer.h"

/* 네모의 형태를 구성해주기위한 정점과 인덱스를 보관한다. */

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Cube final : public CVIBuffer
{
private:
	CVIBuffer_Cube(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CVIBuffer_Cube();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;



public:
	static unique_ptr<CVIBuffer_Cube> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END