// 두 stream 입력 바인딩과 instance draw의 공통 경로를 구현한다.
#include "VIBuffer_Instance.h"
// 현재 frame profiler를 조회한다.
#include "GameInstance.h"
// draw/instance/제출 index 수를 기록한다. GPU 처리 완료를 뜻하는 counter는 아니다.
#include "Profiler.h"

CVIBuffer_Instance::CVIBuffer_Instance(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Instance::~CVIBuffer_Instance()
{
}

// 이 기반 구현은 pArg를 읽지 않는다. 실제 파생 Prototype이 buffer와 수치를 구성해야 한다.
HRESULT CVIBuffer_Instance::Initialize_Prototype(INSTANCE_DESC* pArg)
{
	return S_OK;
}

// 기본 clone 초기화는 상태를 바꾸지 않는다.
HRESULT CVIBuffer_Instance::Initialize(void* pArg)
{
	return S_OK;
}

// 파생 소비자가 shader와 입력을 먼저 바인딩한다. 통계 기록 후 DrawIndexedInstanced를 제출하고 S_OK를 돌려준다.
HRESULT CVIBuffer_Instance::Render()
{
	if (CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Add_Counter(EProfilerCounter::DrawCalls);
		pProfiler->Add_Counter(EProfilerCounter::InstancedDrawCalls);
		pProfiler->Add_Counter(EProfilerCounter::Instances, m_iNumInstances);
		pProfiler->Add_Counter(
			EProfilerCounter::Indices,
			static_cast<uint64_t>(m_iNumIndexPerInstance) * m_iNumInstances);
	}

	m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstances, 0, 0, 0);
	
	return S_OK;
}

// slot0의 mesh vertex와 slot1의 instance stream을 같은 호출로 바인딩한다. 배열 길이/자원 유효성은 파생 초기화가 보장해야 한다.
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

	// buffer별 stride는 byte 단위다. index format과 topology도 파생 타입이 준비한 값을 그대로 적용한다.
	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetIndexBuffer(m_pIB.Get(), m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	return S_OK;
}
