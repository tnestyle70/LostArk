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
		f32_t		fMouseSensor = 0.1f;
		shared_ptr<CTransform> pFollowTarget = { nullptr };
		float3_t vPositionOffset = { 0.4f, 7.5f, 4.5f };
		float3_t vLookOffset = { 0.f, 1.2f, 0.f };
		f32_t fFollowResponse = 18.f;
		bool_t isFollowEnabled = { false };
		bool_t allowCapturedKeyboardInput = { false };
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

public:
	void Set_FollowTarget(const shared_ptr<CTransform>& pFollowTarget);

	void Set_FollowEnabled(bool_t isEnabled);

	bool_t Is_FollowEnabled() const
	{
		return m_bFollowEnabled;
	}
	bool_t Is_FollowRequested() const
	{
		return m_bFollowRequested;
	}
	shared_ptr<CTransform> Get_FollowTarget() const
	{
		return m_pFollowTarget.lock();
	}
	bool_t Is_MouseLookEnabled() const
	{
		return m_bMouseLookEnabled;
	}
	void Set_PositionOffset(const float3_t& vPositionOffset);
	void Frame_Area(const float3_t& center, f32_t radius);
	const float3_t& Get_PositionOffset() const
	{
		return m_vPositionOffset;
	}

private:
	void Update_Shortcuts();
	void Update_FollowCamera(f32_t fTimeDelta);
	void Update_FreeCamera(f32_t fTimeDelta);
	void Remove_AppliedCameraShake();
	void Apply_CameraShake(f32_t fTimeDelta);

private:
	f32_t				m_fMouseSensor = 0.1f;
	bool_t				m_bFollowEnabled = false;
	bool_t				m_bFollowRequested = false;
	bool_t				m_bFollowInitialized = false;
	bool_t				m_bMouseLookEnabled = true;
	// 추적 대상의 수명은 Layer_Player가 소유하므로 카메라는 약한 참조만 유지한다.
	weak_ptr<CTransform>	m_pFollowTarget;
	float3_t			m_vPositionOffset = { 0.4f, 7.5f, 4.5f };
	float3_t			m_vLookOffset = { 0.f, 1.2f, 0.f };
	float3_t			m_vCurrentLookAt = {};
	f32_t				m_fFollowResponse = 18.f;
	bool_t				m_allowCapturedKeyboardInput = false;
	float3_t			m_vAppliedShakeOffset = {};
	f32_t				m_fBaseFovy = 60.f;

public:
	static unique_ptr<CCamera_Free> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END
