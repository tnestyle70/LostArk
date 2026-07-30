#pragma once

#include "Client_Defines.h"
#include "Camera.h"

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free final : public CCamera
{
public:
	typedef struct tagCameraFreeDesc : public CCamera::CAMERA_DESC
	{
		f32_t		fMouseSensor;
		shared_ptr<CTransform> pFollowTarget = { nullptr };
		float3_t vFollowOffset = { -12.f, 16.f, -12.f };
		float3_t vLookOffset = { 0.f, 1.2f, 0.f };
		bool_t isFollowEnabled = { false };
	}CAMERA_FREE_DESC;

private:
	CCamera_Free(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CCamera_Free();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	f32_t				m_fMouseSensor = {};
	bool_t				m_bMovementLocked = false;
	bool_t				m_bTabDown = false;
	bool_t				m_bFollowEnabled = false;
	bool_t				m_bF6Down = false;
	weak_ptr<CTransform>	m_pFollowTarget;
	float3_t			m_vFollowOffset = {};
	float3_t			m_vLookOffset = {};

public:
	static unique_ptr<CCamera_Free> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END
