#include "VIBuffer_ParticleRect.h"
#include "GameInstance.h"
#include "Profiler.h"

#include <array>
#include <cstring>

Engine::CVIBuffer_ParticleRect::CVIBuffer_ParticleRect(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer(std::move(pDevice), std::move(pContext))
{
}

Engine::CVIBuffer_ParticleRect::~CVIBuffer_ParticleRect() = default;

HRESULT Engine::CVIBuffer_ParticleRect::Initialize_Prototype(
	const uint32_t iCapacity)
{
	if (0u == iCapacity || iCapacity > 2048u)
		return E_INVALIDARG;

	m_iNumVertices = 4u;
	m_iVertexStride = sizeof(VTXTEX);
	m_iNumIndices = 6u;
	m_iIndexStride = sizeof(uint16_t);
	m_iNumVertexBuffers = 2u;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	const std::array<VTXTEX, 4> Vertices =
	{
		VTXTEX{ { -0.5f, 0.5f, 0.f }, { 0.f, 0.f } },
		VTXTEX{ { 0.5f, 0.5f, 0.f }, { 1.f, 0.f } },
		VTXTEX{ { 0.5f, -0.5f, 0.f }, { 1.f, 1.f } },
		VTXTEX{ { -0.5f, -0.5f, 0.f }, { 0.f, 1.f } }
	};
	const std::array<uint16_t, 6> Indices = { 0u, 1u, 2u, 0u, 2u, 3u };

	D3D11_BUFFER_DESC VertexDesc{};
	VertexDesc.ByteWidth = sizeof(Vertices);
	VertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	VertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA VertexData{};
	VertexData.pSysMem = Vertices.data();
	if (FAILED(m_pDevice->CreateBuffer(&VertexDesc, &VertexData, &m_pVB)))
		return E_FAIL;

	D3D11_BUFFER_DESC IndexDesc{};
	IndexDesc.ByteWidth = sizeof(Indices);
	IndexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IndexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA IndexData{};
	IndexData.pSysMem = Indices.data();
	if (FAILED(m_pDevice->CreateBuffer(&IndexDesc, &IndexData, &m_pIB)))
		return E_FAIL;

	D3D11_BUFFER_DESC InstanceDesc{};
	InstanceDesc.ByteWidth = sizeof(VTXEFFECT_PARTICLE) * iCapacity;
	InstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
	InstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	InstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_pDevice->CreateBuffer(
		&InstanceDesc, nullptr, &m_pInstanceBuffer)))
	{
		return E_FAIL;
	}

	m_iCapacity = iCapacity;
	m_iNumInstances = 0u;
	m_iNextInstanceIndex = 0u;
	m_iInstanceByteOffset = 0u;
	return S_OK;
}

HRESULT Engine::CVIBuffer_ParticleRect::Initialize(void* pArg)
{
	return CVIBuffer::Initialize(pArg);
}

HRESULT Engine::CVIBuffer_ParticleRect::Update_Instances(
	const std::span<const VTXEFFECT_PARTICLE> Instances)
{
	if (Instances.size() > m_iCapacity)
		return E_INVALIDARG;
	if (Instances.empty())
	{
		m_iNumInstances = 0u;
		return S_OK;
	}

	// Every effect shares this immediate-context buffer. Keep the cursor on
	// the buffer, including across frames, so pending draws are never overwritten.
	const bool bDiscard = 0u == m_iNextInstanceIndex ||
		Instances.size() > m_iCapacity - m_iNextInstanceIndex;
	const uint32_t iFirstInstance = bDiscard ? 0u : m_iNextInstanceIndex;
	const uint32_t iByteOffset =
		static_cast<uint32_t>(sizeof(VTXEFFECT_PARTICLE)) * iFirstInstance;
	D3D11_MAPPED_SUBRESOURCE Mapped{};
	const HRESULT hMapResult = m_pContext->Map(
		m_pInstanceBuffer.Get(), 0u,
		bDiscard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE,
		0u, &Mapped);
	if (FAILED(hMapResult))
	{
		m_iNumInstances = 0u;
		return hMapResult;
	}
	std::memcpy(
		static_cast<uint8_t*>(Mapped.pData) + iByteOffset,
		Instances.data(),
		Instances.size_bytes());
	m_pContext->Unmap(m_pInstanceBuffer.Get(), 0u);
	m_iNumInstances = static_cast<uint32_t>(Instances.size());
	m_iInstanceByteOffset = iByteOffset;
	m_iNextInstanceIndex = iFirstInstance + m_iNumInstances;
	return S_OK;
}

HRESULT Engine::CVIBuffer_ParticleRect::Bind_Resources()
{
	if (nullptr == m_pVB || nullptr == m_pInstanceBuffer || nullptr == m_pIB)
		return E_FAIL;

	ID3D11Buffer* Buffers[] = { m_pVB.Get(), m_pInstanceBuffer.Get() };
	const uint32_t Strides[] = { sizeof(VTXTEX), sizeof(VTXEFFECT_PARTICLE) };
	const uint32_t Offsets[] = { 0u, m_iInstanceByteOffset };
	m_pContext->IASetVertexBuffers(0u, 2u, Buffers, Strides, Offsets);
	m_pContext->IASetIndexBuffer(m_pIB.Get(), m_eIndexFormat, 0u);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);
	return S_OK;
}

HRESULT Engine::CVIBuffer_ParticleRect::Render()
{
	if (0u == m_iNumInstances)
		return S_FALSE;
	if (FAILED(Bind_Resources()))
		return E_FAIL;
	if (CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Add_Counter(EProfilerCounter::DrawCalls);
		pProfiler->Add_Counter(EProfilerCounter::InstancedDrawCalls);
		pProfiler->Add_Counter(EProfilerCounter::Instances, m_iNumInstances);
		pProfiler->Add_Counter(EProfilerCounter::Indices,
			static_cast<uint64_t>(m_iNumIndices) * m_iNumInstances);
	}
	m_pContext->DrawIndexedInstanced(
		m_iNumIndices, m_iNumInstances, 0u, 0, 0u);
	return S_OK;
}

unique_ptr<Engine::CVIBuffer_ParticleRect>
Engine::CVIBuffer_ParticleRect::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iCapacity)
{
	unique_ptr<CVIBuffer_ParticleRect> Instance(
		new CVIBuffer_ParticleRect(
			std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype(iCapacity)))
		return nullptr;
	return Instance;
}

shared_ptr<Engine::CPrototype> Engine::CVIBuffer_ParticleRect::Clone(void* pArg)
{
	shared_ptr<CVIBuffer_ParticleRect> Instance(
		new CVIBuffer_ParticleRect(m_pDevice, m_pContext));
	if (FAILED(Instance->Initialize_Prototype(m_iCapacity)) ||
		FAILED(Instance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CVIBuffer_ParticleRect");
		return nullptr;
	}
	return Instance;
}
