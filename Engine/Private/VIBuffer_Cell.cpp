#include "VIBuffer_Cell.h"

CVIBuffer_Cell::CVIBuffer_Cell(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Cell::~CVIBuffer_Cell()
{
}

HRESULT CVIBuffer_Cell::Initialize_Prototype(const float3_t* pPoints)
{
#pragma region PUBLIC_DATA
	m_iVertexStride = sizeof(VTXPOS);
	m_iNumVertices = 3;
	m_iIndexStride = 2;
	m_iNumIndices = 4;
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		 
#pragma endregion

#pragma region VERTEX_BUFFER
	D3D11_BUFFER_DESC		VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iVertexStride;	


	// unique_ptr<VTXTEX[]> pVertices = unique_ptr<VTXTEX[]>(new VTXTEX[m_iNumVertices]());
	unique_ptr<VTXPOS[]> pVertices = make_unique<VTXPOS[]>(m_iNumVertices);
		
	memcpy(pVertices.get(), pPoints, sizeof(VTXPOS) * 3);

	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;

#pragma endregion

#pragma region INDEX_BUFFER
	D3D11_BUFFER_DESC		IndexBufferDesc{};
	IndexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = m_iIndexStride;

	unique_ptr<uint16_t[]>		pIndices = make_unique<uint16_t[]>(m_iNumIndices);
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;
	pIndices[3] = 0;
	
	D3D11_SUBRESOURCE_DATA			IndexInitialData{};
	IndexInitialData.pSysMem = pIndices.get();

	if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, &m_pIB)))
		return E_FAIL;

#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Cell::Initialize(void* pArg)
{
	return S_OK;
}

unique_ptr<CVIBuffer_Cell> CVIBuffer_Cell::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const float3_t* pPoints)
{
	auto pInstance = unique_ptr<CVIBuffer_Cell>(new CVIBuffer_Cell(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pPoints)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Cell");
		return nullptr;
	}

	return pInstance;
}


shared_ptr<CPrototype> CVIBuffer_Cell::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CVIBuffer_Cell>(new CVIBuffer_Cell(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CVIBuffer_Cell");
		return nullptr;
	}

	return pInstance;
}

