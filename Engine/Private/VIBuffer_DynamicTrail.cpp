// Client가 만든 trail geometry의 GPU 업로드와 draw 개수를 관리한다.
#include "VIBuffer_DynamicTrail.h"

// span의 연속 CPU 데이터를 Map된 GPU 쓰기 영역에 복사한다.
#include <cstring>

namespace
{
	// 원본 Ribbon의 제어점은 구간마다 최대 25단계로 세분화할 수 있다.
	// 아래는 렌더 버퍼의 점 상한이며, 저작/런타임 제어점 계약의
	// 최대 512개와 구분한다.
	constexpr uint32_t MAX_DYNAMIC_TRAIL_POINTS = 16384u;
}

NS_BEGIN(Engine)

CVIBuffer_DynamicTrail::CVIBuffer_DynamicTrail(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer(std::move(pDevice), std::move(pContext))
{
}

CVIBuffer_DynamicTrail::~CVIBuffer_DynamicTrail() = default;

// Create/Clone이 필요한 렌더 점 용량으로 호출한다. 생성 도중 실패한 자원의 정리는 호출 factory가 맡는다.
HRESULT CVIBuffer_DynamicTrail::Initialize_Prototype(
	const uint32_t iMaxPoints)
{
	if (iMaxPoints < 2u || iMaxPoints > MAX_DYNAMIC_TRAIL_POINTS)
	{
		return E_INVALIDARG;
	}

	// 폭의 양쪽 정점 두 개를 점마다 두고, 인접 점 사이 사각형을 두 삼각형(index6개)으로 만든다.
	m_iMaxVertices = iMaxPoints * 2u;
	m_iMaxIndices = (iMaxPoints - 1u) * 6u;
	m_iNumVertices = m_iMaxVertices;
	m_iVertexStride = sizeof(VTXEFFECT_TRAIL);
	m_iNumIndices = 0u;
	m_iIndexStride = sizeof(uint32_t);
	m_iNumVertexBuffers = 1u;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 매 업로드 때 WRITE_DISCARD로 교체할 dynamic vertex buffer를 할당한다. 아직 그릴 geometry는 없다.
	D3D11_BUFFER_DESC VertexDesc{};
	VertexDesc.ByteWidth = m_iVertexStride * m_iMaxVertices;
	VertexDesc.Usage = D3D11_USAGE_DYNAMIC;
	VertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_pDevice->CreateBuffer(&VertexDesc, nullptr, &m_pVB)))
	{
		return E_FAIL;
	}

	// index도 CPU에서 새 geometry에 맞춰 쓰므로 dynamic buffer로 준비한다.
	D3D11_BUFFER_DESC IndexDesc{};
	IndexDesc.ByteWidth = m_iIndexStride * m_iMaxIndices;
	IndexDesc.Usage = D3D11_USAGE_DYNAMIC;
	IndexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_pDevice->CreateBuffer(&IndexDesc, nullptr, &m_pIB)))
	{
		return E_FAIL;
	}

	return S_OK;
}

// 추가 자원 없이 기반 초기화에 위임한다.
HRESULT CVIBuffer_DynamicTrail::Initialize(void* pArg)
{
	return CVIBuffer::Initialize(pArg);
}

// 렌더 스레드가 현재 geometry를 차용 span으로 전달한다. 용량만 검사하고 실제 vertex/index 내용 검증은 상위에서 수행한다.
HRESULT CVIBuffer_DynamicTrail::Update_Geometry(
	const std::span<const VTXEFFECT_TRAIL> Vertices,
	const std::span<const uint32_t> Indices)
{
	if (Vertices.size() > m_iMaxVertices || Indices.size() > m_iMaxIndices)
	{
		return E_INVALIDARG;
	}
	if (Vertices.empty() || Indices.empty())
	{
		m_iActiveVertices = 0u;
		m_iNumIndices = 0u;
		return S_OK;
	}

	// vertex 쓰기부터 수행한다. 뒤 index Map이 실패해도 vertex 내용을 되돌리지는 않으며 draw 개수만 비운다.
	D3D11_MAPPED_SUBRESOURCE VertexMapped{};
	const HRESULT hVertexMapResult = m_pContext->Map(
		m_pVB.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &VertexMapped);
	if (FAILED(hVertexMapResult))
	{
		m_iActiveVertices = 0u;
		m_iNumIndices = 0u;
		return hVertexMapResult;
	}
	std::memcpy(VertexMapped.pData, Vertices.data(), Vertices.size_bytes());
	m_pContext->Unmap(m_pVB.Get(), 0u);

	// index까지 업로드에 성공해야 마지막 활성 개수를 새 geometry로 확정한다.
	D3D11_MAPPED_SUBRESOURCE IndexMapped{};
	const HRESULT hIndexMapResult = m_pContext->Map(
		m_pIB.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &IndexMapped);
	if (FAILED(hIndexMapResult))
	{
		m_iActiveVertices = 0u;
		m_iNumIndices = 0u;
		return hIndexMapResult;
	}
	std::memcpy(IndexMapped.pData, Indices.data(), Indices.size_bytes());
	m_pContext->Unmap(m_pIB.Get(), 0u);

	m_iActiveVertices = static_cast<uint32_t>(Vertices.size());
	m_iNumIndices = static_cast<uint32_t>(Indices.size());
	return S_OK;
}

// 빈 geometry는 제출하지 않는다. 기본 Render는 draw만 하므로 입력/shader 바인딩은 호출 순서에서 먼저 준비해야 한다.
HRESULT CVIBuffer_DynamicTrail::Render()
{
	if (0u == m_iNumIndices)
	{
		return S_FALSE;
	}
	return CVIBuffer::Render();
}

// 새 GPU buffer 두 개를 준비하고 완성된 소유 객체만 넘긴다.
unique_ptr<CVIBuffer_DynamicTrail>
CVIBuffer_DynamicTrail::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iMaxPoints)
{
	unique_ptr<CVIBuffer_DynamicTrail> Instance(
		new CVIBuffer_DynamicTrail(
			std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype(iMaxPoints)))
	{
		return nullptr;
	}
	return Instance;
}

// 같은 용량으로 새 vertex/index 자원을 만든다. 원본 geometry와 활성 개수는 복사하지 않는다.
shared_ptr<CPrototype> CVIBuffer_DynamicTrail::Clone(void* pArg)
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

NS_END
