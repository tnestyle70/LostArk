#include "VIBuffer_Rect.h"

CVIBuffer_Rect::CVIBuffer_Rect(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Rect::~CVIBuffer_Rect()
{
}

HRESULT CVIBuffer_Rect::Initialize_Prototype()
{
#pragma region PUBLIC_DATA
	m_iVertexStride = sizeof(VTXTEX);
	m_iNumVertices = 4;
	m_iIndexStride = 2;
	m_iNumIndices = 6;
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		 
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
	unique_ptr<VTXTEX[]> pVertices = make_unique<VTXTEX[]>(m_iNumVertices);
		
	pVertices[0].vPosition = float3_t(-0.5f, 0.5f, 0.f);
	pVertices[0].vTexcoord = float2_t(0.f, 0.f);

	pVertices[1].vPosition = float3_t(0.5f, 0.5f, 0.f);
	pVertices[1].vTexcoord = float2_t(1.f, 0.f);

	pVertices[2].vPosition = float3_t(0.5f, -0.5f, 0.f);
	pVertices[2].vTexcoord = float2_t(1.f, 1.f);

	pVertices[3].vPosition = float3_t(-0.5f, -0.5f, 0.f);
	pVertices[3].vTexcoord = float2_t(0.f, 1.f);

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
	pIndices[4] = 2;
	pIndices[5] = 3;

	D3D11_SUBRESOURCE_DATA			IndexInitialData{};
	IndexInitialData.pSysMem = pIndices.get();

	if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, &m_pIB)))
		return E_FAIL;

#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Rect::Initialize(void* pArg)
{
	return S_OK;
}

unique_ptr<CVIBuffer_Rect> CVIBuffer_Rect::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CVIBuffer_Rect>(new CVIBuffer_Rect(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Rect");
		return nullptr;
	}

	return pInstance;
}


shared_ptr<CPrototype> CVIBuffer_Rect::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CVIBuffer_Rect>(new CVIBuffer_Rect(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CVIBuffer_Rect");
		return nullptr;
	}

	return pInstance;
}

