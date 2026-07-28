#include "VIBuffer_Instance.h"

CVIBuffer_Instance::CVIBuffer_Instance(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Instance::~CVIBuffer_Instance()
{
}

HRESULT CVIBuffer_Instance::Initialize_Prototype(INSTANCE_DESC* pArg)
{
	return S_OK;
}

HRESULT CVIBuffer_Instance::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CVIBuffer_Instance::Render()
{
	m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstances, 0, 0, 0);
	
	return S_OK;
}

HRESULT CVIBuffer_Instance::Bind_Resources()
{
	ID3D11Buffer* pVertexBuffers[] = {
		m_pVB.Get(),
		m_pVBInstance.Get()
	};

	uint32_t	iVertexStrides[] = {
		m_iVertexStride,
		m_iVertexInstanceStride
	};

	uint32_t	iOffsets[] = {
		0,
		0
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetIndexBuffer(m_pIB.Get(), m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	return S_OK;
}
