#pragma once

// 정점/index 자원과 장치/context의 수명을 기반 buffer가 소유한다.
#include "VIBuffer.h"

NS_BEGIN(Engine)

// slot 0 정점과 slot 1 instance의 두 stream을 묶는 공통 기반이다. 각 파생 타입이 실제 buffer와 draw 수를 준비해야 한다.
class ENGINE_DLL CVIBuffer_Instance abstract : public CVIBuffer
{
public:
	// 파생 타입 초기화에 전달할 instance 수·크기·범위·중심 값이다. 이 기반 클래스는 값을 읽거나 저장하지 않는다.
	typedef struct tagInstanceDesc
	{
		uint32_t		iNumInstance;
		float2_t		vSize;
		float3_t		vRange;
		float3_t		vCenter;
	}INSTANCE_DESC;
protected:
	CVIBuffer_Instance(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CVIBuffer_Instance();

public:
	// 기본 Prototype/clone 초기화는 입력을 사용하지 않는 S_OK다. 파생 구현의 자원 준비를 대신하지 않는다.
	virtual HRESULT Initialize_Prototype(INSTANCE_DESC* pArg);
	virtual HRESULT Initialize(void* pArg) override;
	// 저장된 instance당 index 수와 instance 수로 draw한다. 0개 여부나 buffer 유효성을 별도로 검사하지 않는다.
	virtual HRESULT Render() override;

public:
	// 두 buffer의 stride와 offset0을 바인딩한다. 파생 타입은 m_iNumVertexBuffers를 실제 두 원소 배열 범위에 맞춰야 한다.
	virtual HRESULT Bind_Resources() override;


protected:
	// 파생 구현이 준비한 instance 설명과 강한 COM 참조다. 기본 클래스는 생성/Map을 직접 수행하지 않는다.
	D3D11_BUFFER_DESC				m_InstanceBufferDesc{};
	ComPtr<ID3D11Buffer>			m_pVBInstance = { nullptr };	

protected:
	// 이번 draw의 instance 개수, instance당 index 개수, instance stream의 byte stride다.
	uint32_t					m_iNumInstances = {};
	uint32_t					m_iNumIndexPerInstance = {};
	uint32_t					m_iVertexInstanceStride = {};		

public:
	// 쓰기 자원 공유 여부와 clone 초기화 책임은 파생 타입이 명시적으로 구현한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
};

NS_END