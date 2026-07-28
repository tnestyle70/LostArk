#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CVIBuffer();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;	
	virtual HRESULT Render();

public:
	virtual HRESULT Bind_Resources();


protected:
	ComPtr<ID3D11Buffer>			m_pVB = { nullptr };
	ComPtr<ID3D11Buffer>			m_pIB = { nullptr };

protected:
	uint32_t					m_iNumVertices = {};
	uint32_t					m_iVertexStride = {};
	uint32_t					m_iNumIndices = {};
	uint32_t					m_iIndexStride = {};
	uint32_t					m_iNumVertexBuffers = {};
	DXGI_FORMAT					m_eIndexFormat = {};
	D3D11_PRIMITIVE_TOPOLOGY	m_ePrimitiveTopology = {};
	shared_ptr<float3_t[]>		m_pVertexPositions = {nullptr};

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;

};

NS_END