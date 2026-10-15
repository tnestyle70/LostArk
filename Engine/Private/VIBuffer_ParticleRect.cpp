// sprite quad instance 버퍼의 준비·순환 업로드·draw를 구현한다.
#include "VIBuffer_ParticleRect.h"
// 불변 quad의 위치/UV 데이터에 VTXTEX layout을 사용한다.
#include "Engine_VertexTypes.h"
// frame profiler를 현재 엔진 인스턴스에서 조회한다.
#include "GameInstance.h"
// 제출 draw/instance/index 개수를 기록한다.
#include "Profiler.h"

// quad 정점 네 개와 index 여섯 개를 초기화 중 고정 배열에 둔다.
#include <array>
// instance span을 Map된 byte offset으로 복사한다.
#include <cstring>

NS_BEGIN(Engine)

CVIBuffer_ParticleRect::CVIBuffer_ParticleRect(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer(std::move(pDevice), std::move(pContext))
{
}

CVIBuffer_ParticleRect::~CVIBuffer_ParticleRect() = default;

// Create/Clone의 용량을 확인한 뒤 불변 quad와 동적 instance 자원을 준비한다. 이 함수 자체는 부분 실패를 rollback하지 않는다.
HRESULT CVIBuffer_ParticleRect::Initialize_Prototype(
	const uint32_t iCapacity)
{
	if (0u == iCapacity || iCapacity > 2048u)
	{
		return E_INVALIDARG;
	}

	m_iNumVertices = 4u;
	m_iVertexStride = sizeof(VTXTEX);
	m_iNumIndices = 6u;
	m_iIndexStride = sizeof(uint16_t);
	m_iNumVertexBuffers = 2u;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 중심 원점의 1×1 quad와 UV를 고정한다. 실제 particle 위치·크기·회전은 instance World가 전달한다.
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
	{
		return E_FAIL;
	}

	D3D11_BUFFER_DESC IndexDesc{};
	IndexDesc.ByteWidth = sizeof(Indices);
	IndexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IndexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA IndexData{};
	IndexData.pSysMem = Indices.data();
	if (FAILED(m_pDevice->CreateBuffer(&IndexDesc, &IndexData, &m_pIB)))
	{
		return E_FAIL;
	}

	// 변하는 부분만 dynamic instance stream으로 분리한다. 모든 자원 생성 성공 뒤 capacity와 커서를 초기화한다.
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

// 기반 클래스의 추가 작업 없는 clone 초기화로 전달한다.
HRESULT CVIBuffer_ParticleRect::Initialize(void* pArg)
{
	return CVIBuffer::Initialize(pArg);
}

// 이펙트 renderer가 계산한 목록을 GPU에 복사한다. 실패 유형에 따라 기존 상태 유지 또는 이번 draw 억제를 구분한다.
HRESULT CVIBuffer_ParticleRect::Update_Instances(
	const std::span<const VTXEFFECT_PARTICLE> Instances)
{
	if (Instances.size() > m_iCapacity)
	{
		return E_INVALIDARG;
	}
	if (Instances.empty())
	{
		m_iNumInstances = 0u;
		return S_OK;
	}

	// 여러 effect 제출이 같은 immediate-context buffer를 재사용한다.
	// frame이 바뀌어도 buffer의 커서를 유지하고 빈 공간에 NO_OVERWRITE하거나
	// DISCARD해 앞서 제출한 draw가 읽는 영역을 다시 쓰지 않는다.
	const bool bDiscard = 0u == m_iNextInstanceIndex ||
		Instances.size() > m_iCapacity - m_iNextInstanceIndex;
	const uint32_t iFirstInstance = bDiscard ? 0u : m_iNextInstanceIndex;
	// instance index에 record byte 크기를 곱해 Map 쓰기 위치와 IA 바인딩 위치를 함께 정한다.
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
	// 업로드 성공 뒤에만 draw 수·byte offset·다음 빈 instance 위치를 갱신한다.
	m_iNumInstances = static_cast<uint32_t>(Instances.size());
	m_iInstanceByteOffset = iByteOffset;
	m_iNextInstanceIndex = iFirstInstance + m_iNumInstances;
	return S_OK;
}

// 세 자원의 존재를 확인한 뒤 마지막 업로드 구간을 slot1의 시작 offset으로 바인딩한다.
HRESULT CVIBuffer_ParticleRect::Bind_Resources()
{
	if (nullptr == m_pVB || nullptr == m_pInstanceBuffer || nullptr == m_pIB)
	{
		return E_FAIL;
	}

	ID3D11Buffer* Buffers[] = { m_pVB.Get(), m_pInstanceBuffer.Get() };
	const uint32_t Strides[] = { sizeof(VTXTEX), sizeof(VTXEFFECT_PARTICLE) };
	const uint32_t Offsets[] = { 0u, m_iInstanceByteOffset };
	m_pContext->IASetVertexBuffers(0u, 2u, Buffers, Strides, Offsets);
	m_pContext->IASetIndexBuffer(m_pIB.Get(), m_eIndexFormat, 0u);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);
	return S_OK;
}

// shader는 호출자가 준비한다. 이번 업로드가 비었으면 제출하지 않고, 아니면 입력 바인딩 후 instance 수만큼 그린다.
HRESULT CVIBuffer_ParticleRect::Render()
{
	if (0u == m_iNumInstances)
	{
		return S_FALSE;
	}
	if (FAILED(Bind_Resources()))
	{
		return E_FAIL;
	}
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

// 생성 실패한 일부 COM 자원도 지역 소유 객체와 함께 정리한다.
unique_ptr<CVIBuffer_ParticleRect>
CVIBuffer_ParticleRect::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iCapacity)
{
	unique_ptr<CVIBuffer_ParticleRect> Instance(
		new CVIBuffer_ParticleRect(
			std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype(iCapacity)))
	{
		return nullptr;
	}
	return Instance;
}

// 별도 GPU 자원과 초기 cursor로 새 buffer를 만든다. 여러 clone의 쓰기 커서가 같은 자원을 가리키지 않게 한다.
shared_ptr<CPrototype> CVIBuffer_ParticleRect::Clone(void* pArg)
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

NS_END
