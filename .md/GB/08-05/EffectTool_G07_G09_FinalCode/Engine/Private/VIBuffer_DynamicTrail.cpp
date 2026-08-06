#include "VIBuffer_DynamicTrail.h"

#include <cstring>

Engine::CVIBuffer_DynamicTrail::CVIBuffer_DynamicTrail(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer(std::move(pDevice), std::move(pContext))
{
}

Engine::CVIBuffer_DynamicTrail::~CVIBuffer_DynamicTrail() = default;

HRESULT Engine::CVIBuffer_DynamicTrail::Initialize_Prototype(
	const uint32_t iMaxPoints)
{
	if (iMaxPoints < 2u || iMaxPoints > 256u)
		return E_INVALIDARG;

	m_iMaxVertices = iMaxPoints * 2u;
	m_iMaxIndices = (iMaxPoints - 1u) * 6u;
	m_iNumVertices = m_iMaxVertices;
	m_iVertexStride = sizeof(VTXEFFECT_TRAIL);
	m_iNumIndices = 0u;
	m_iIndexStride = sizeof(uint32_t);
	m_iNumVertexBuffers = 1u;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_BUFFER_DESC VertexDesc{};
	VertexDesc.ByteWidth = m_iVertexStride * m_iMaxVertices;
	VertexDesc.Usage = D3D11_USAGE_DYNAMIC;
	VertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_pDevice->CreateBuffer(&VertexDesc, nullptr, &m_pVB)))
		return E_FAIL;

	D3D11_BUFFER_DESC IndexDesc{};
	IndexDesc.ByteWidth = m_iIndexStride * m_iMaxIndices;
	IndexDesc.Usage = D3D11_USAGE_DYNAMIC;
	IndexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_pDevice->CreateBuffer(&IndexDesc, nullptr, &m_pIB)))
		return E_FAIL;

	return S_OK;
}

HRESULT Engine::CVIBuffer_DynamicTrail::Initialize(void* pArg)
{
	return CVIBuffer::Initialize(pArg);
}

HRESULT Engine::CVIBuffer_DynamicTrail::Update_Geometry(
	const std::span<const VTXEFFECT_TRAIL> Vertices,
	const std::span<const uint32_t> Indices)
{
	if (Vertices.size() > m_iMaxVertices || Indices.size() > m_iMaxIndices)
		return E_INVALIDARG;
	if (Vertices.empty() || Indices.empty())
	{
		m_iActiveVertices = 0u;
		m_iNumIndices = 0u;
		return S_OK;
	}

	D3D11_MAPPED_SUBRESOURCE VertexMapped{};
	if (FAILED(m_pContext->Map(
		m_pVB.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &VertexMapped)))
	{
		m_iActiveVertices = 0u;
		m_iNumIndices = 0u;
		return E_FAIL;
	}
	std::memcpy(VertexMapped.pData, Vertices.data(), Vertices.size_bytes());
	m_pContext->Unmap(m_pVB.Get(), 0u);

	D3D11_MAPPED_SUBRESOURCE IndexMapped{};
	if (FAILED(m_pContext->Map(
		m_pIB.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &IndexMapped)))
	{
		m_iActiveVertices = 0u;
		m_iNumIndices = 0u;
		return E_FAIL;
	}
	std::memcpy(IndexMapped.pData, Indices.data(), Indices.size_bytes());
	m_pContext->Unmap(m_pIB.Get(), 0u);

	m_iActiveVertices = static_cast<uint32_t>(Vertices.size());
	m_iNumIndices = static_cast<uint32_t>(Indices.size());
	return S_OK;
}

HRESULT Engine::CVIBuffer_DynamicTrail::Render()
{
	if (0u == m_iNumIndices)
		return S_FALSE;
	return CVIBuffer::Render();
}

unique_ptr<Engine::CVIBuffer_DynamicTrail>
Engine::CVIBuffer_DynamicTrail::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iMaxPoints)
{
	unique_ptr<CVIBuffer_DynamicTrail> Instance(
		new CVIBuffer_DynamicTrail(
			std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype(iMaxPoints)))
		return nullptr;
	return Instance;
}

shared_ptr<Engine::CPrototype> Engine::CVIBuffer_DynamicTrail::Clone(void* pArg)
{
	shared_ptr<CVIBuffer_DynamicTrail> Instance(
		new CVIBuffer_DynamicTrail(m_pDevice, m_pContext));
	if (FAILED(Instance->Initialize_Prototype(m_iMaxVertices / 2u)) ||
		FAILED(Instance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CVIBuffer_DynamicTrail");
		return nullptr;
	}
	return Instance;
}
