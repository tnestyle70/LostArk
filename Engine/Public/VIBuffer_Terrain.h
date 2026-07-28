#pragma once

#include "VIBuffer.h"

/* 네모의 형태를 구성해주기위한 정점과 인덱스를 보관한다. */

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Terrain final : public CVIBuffer
{
private:
	CVIBuffer_Terrain(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CVIBuffer_Terrain();

public:
	virtual HRESULT Initialize_Prototype(const tchar_t* pHeightMapFilePath);
	virtual HRESULT Initialize(void* pArg) override;

public:
	void Culling(fmatrix_t WorldMatrix);

private:
	uint32_t			m_iNumVerticesX = {};
	uint32_t			m_iNumVerticesZ = {};



public:
	static unique_ptr<CVIBuffer_Terrain> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pHeightMapFilePath);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END