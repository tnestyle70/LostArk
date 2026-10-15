#pragma once

// 복제 가능한 Component의 장치·context 참조를 상속하고 GPU 버퍼를 ComPtr로 소유한다.
#include "Component.h"

NS_BEGIN(Engine)

// 정점/인덱스 버퍼를 IA 단계에 연결하고 indexed draw를 제출하는 공통 컴포넌트다.
// Shader의 input layout/pass 및 상수 바인딩은 호출자가 먼저 준비한다. 내부 context 동기화는 없다.
class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CVIBuffer();

public:
	// 기본 구현은 S_OK만 반환한다. 파생 prototype이 실제 버퍼와 개수/stride를 준비한다.
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;	
	// 현재 바인딩된 상태로 한 번 그린다. S_OK는 GPU 완료나 화면 표시 성공을 확인한 결과가 아니다.
	virtual HRESULT Render();
	// 같은 인덱스를 instanceCount만큼 제출하며 0이면 아무것도 하지 않는다. instance 데이터 업로드는 호출자의 책임이다.
	HRESULT Render_Instanced(uint32_t instanceCount);

public:
	// 기본 구현의 로컬 배열은 VB 한 개용이다. 둘 이상의 stream은 파생 Bind_Resources 구현이 필요하다.
	virtual HRESULT Bind_Resources();

protected:
	// ComPtr로 GPU 버퍼 참조를 소유한다. 기본 복사로 clone을 만들면 버퍼를 복제하지 않고 같은 자원을 공유한다.
	ComPtr<ID3D11Buffer>			m_pVB = { nullptr };
	ComPtr<ID3D11Buffer>			m_pIB = { nullptr };

protected:
	// 개수는 원소 수, stride는 byte 수다. 파생 prototype이 버퍼 내용과 format/topology를 서로 맞춰야 한다.
	uint32_t					m_iNumVertices = {};
	uint32_t					m_iVertexStride = {};
	uint32_t					m_iNumIndices = {};
	uint32_t					m_iIndexStride = {};
	uint32_t					m_iNumVertexBuffers = {};
	DXGI_FORMAT					m_eIndexFormat = {};
	D3D11_PRIMITIVE_TOPOLOGY	m_ePrimitiveTopology = {};
	// 필요한 파생 버퍼가 채우는 CPU 위치 배열이다. shared_ptr 복사도 같은 배열을 공유하며 기본 Rect는 채우지 않는다.
	shared_ptr<float3_t[]>		m_pVertexPositions = {nullptr};

public:
	// 실제 복제 정책과 인스턴스 초기화는 파생 버퍼가 구현한다. 공통 계층은 GPU 버퍼 깊은 복사 경로를 제공하지 않는다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;

};

NS_END