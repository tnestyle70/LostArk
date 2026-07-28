#pragma once

#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance abstract : public CVIBuffer
{
public:
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
	virtual HRESULT Initialize_Prototype(INSTANCE_DESC* pArg);
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Render() override;

public:
	virtual HRESULT Bind_Resources() override;


protected:
	D3D11_BUFFER_DESC				m_InstanceBufferDesc{};
	ComPtr<ID3D11Buffer>			m_pVBInstance = { nullptr };	

protected:
	uint32_t					m_iNumInstances = {};
	uint32_t					m_iNumIndexPerInstance = {};
	uint32_t					m_iVertexInstanceStride = {};		

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
};

NS_END