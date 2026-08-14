#pragma once

#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCamera abstract : public CGameObject
{
public:
	typedef struct tagCameraDesc : public CGameObject::GAMEOBJECT_DESC
	{
		float3_t	vEye, vAt;
		f32_t		fFovy, fNear, fFar;
	}CAMERA_DESC;

protected:
	CCamera(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CCamera();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	bool_t Begin_PresentationOverride(uint64_t iOwnerId);
	bool_t Apply_PresentationPose(
		uint64_t iOwnerId,
		const float3_t& vEye,
		const float3_t& vLookAt,
		f32_t fFovYDegrees);
	bool_t End_PresentationOverride(uint64_t iOwnerId);
	bool_t Is_PresentationOverrideActive() const
	{
		return m_bPresentationOverrideActive;
	}

protected:
	f32_t				m_fFovy = {}, m_fAspect = {}, m_fNear = {}, m_fFar = {};
	bool_t				m_bPresentationOverrideActive = false;
	float4x4_t			m_PresentationSavedWorld = {};
	float4x4_t			m_PresentationAppliedWorld = {};
	f32_t				m_fPresentationSavedFovy = 60.f;
	f32_t				m_fPresentationAppliedFovy = 60.f;
	uint64_t			m_iPresentationOverrideOwnerId = 0u;
	
protected:
	void Update_PipeLine();


public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
};

NS_END
