#pragma once

#include "VIBuffer_Instance.h"

/* 네모의 형태를 구성해주기위한 정점과 인덱스를 보관한다. */

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance_Rect final : public CVIBuffer_Instance
{
public:
	typedef struct tagInstanceParticleDesc : CVIBuffer_Instance::INSTANCE_DESC
	{
		float2_t			vSpeed;
		float2_t			vLifeTime; 
		bool_t				isLoop;
	}INSTANCE_PARTICLE_DESC;
private:
	CVIBuffer_Instance_Rect(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CVIBuffer_Instance_Rect();

public:
	virtual HRESULT Initialize_Prototype(INSTANCE_DESC* pArg);
	virtual HRESULT Initialize(void* pArg) override;

public:
	void Drop(f32_t fTimeDelta);

private:
	bool_t						m_isLoop = { false };
	shared_ptr<f32_t[]>			m_pSpeeds = { nullptr };
	shared_ptr<VTXINSTANCE_PARTICLE[]>		m_pInstances = { nullptr };
	// shared_ptr<float2_t[]>			m_pLifeTimes = { nullptr };

public:
	static unique_ptr<CVIBuffer_Instance_Rect> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, INSTANCE_DESC* pArg);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END