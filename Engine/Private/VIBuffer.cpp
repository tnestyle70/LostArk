// 공통 버퍼의 IA 바인딩과 indexed draw 제출 경계를 구현한다.
#include "VIBuffer.h"
// 현재 엔진이 소유하는 profiler를 조회한다. nullptr이면 계측 없이 draw를 제출한다.
#include "GameInstance.h"
// DrawCalls·Indices·Instances 등 제출량 카운터 종류와 누적 API를 사용한다.
#include "Profiler.h"

CVIBuffer::CVIBuffer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

CVIBuffer::~CVIBuffer()
{
}

// CComponent의 장치 참조 외에 이 기본 함수가 추가하는 상태는 없다. GPU 생성은 파생 구현이 담당한다.
HRESULT CVIBuffer::Initialize_Prototype()
{
	return S_OK;
}

// 기본 clone 초기화는 pArg를 사용하지 않고 공유 자원을 그대로 둔다.
HRESULT CVIBuffer::Initialize(void* pArg)
{
	return S_OK;
}

// UI/이펙트와 렌더 단계가 Bind_Resources와 shader 준비 뒤 호출한다. 카운터는 제출량이며 GPU 실행 결과가 아니다.
HRESULT CVIBuffer::Render()
{
	if (CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Add_Counter(EProfilerCounter::DrawCalls);
		pProfiler->Add_Counter(EProfilerCounter::Indices, m_iNumIndices);
	}

	m_pContext->DrawIndexed(m_iNumIndices, 0, 0);

	return S_OK;
}

// Light_Manager는 shader 배열에 light instance 값을 올린 뒤 이 함수를 사용한다. 별도 instance VB를 만드는 함수는 아니다.
HRESULT CVIBuffer::Render_Instanced(uint32_t instanceCount)
{
	if (instanceCount == 0u)
		return S_OK;

	if (CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Add_Counter(EProfilerCounter::DrawCalls);
		pProfiler->Add_Counter(EProfilerCounter::InstancedDrawCalls);
		pProfiler->Add_Counter(EProfilerCounter::Instances, instanceCount);
		pProfiler->Add_Counter(EProfilerCounter::Indices,
			static_cast<uint64_t>(m_iNumIndices) * instanceCount);
	}

	m_pContext->DrawIndexedInstanced(m_iNumIndices, instanceCount, 0u, 0, 0u);
	return S_OK;
}

// VB 한 stream, IB와 topology만 현재 immediate context에 연결한다. 아래 배열 크기상 m_iNumVertexBuffers는 1 이하가 전제다.
// InputLayout과 shader는 호출자가 설정한다. 여러 stream을 사용하는 VIBuffer_Instance는 이 함수를 재정의한다.
HRESULT CVIBuffer::Bind_Resources()
{
	ID3D11Buffer* pVertexBuffers[] = {
		m_pVB.Get(),		
	};

	uint32_t	iVertexStrides[] = {
		m_iVertexStride, 
	};

	uint32_t	iOffsets[] = {
		0, 
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetIndexBuffer(m_pIB.Get(), m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	// 아래는 과거 InputLayout 연결 예시를 보존한 비실행 코드다. 현재 경로의 layout은 호출자가 shader와 함께 준비한다.
	//ID3D11InputLayout* pInputLayout = { nullptr };

	//D3D11_INPUT_ELEMENT_DESC		ElementDesc[] = {
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },		
	//};

	//m_pDevice->CreateInputLayout(ElementDesc, 2, , , &pInputLayout);

	//m_pContext->IASetInputLayout(pInputLayout);

	return S_OK;
}
