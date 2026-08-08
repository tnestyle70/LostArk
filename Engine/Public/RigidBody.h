#pragma once

#include "Component.h"
#include "Physics_Manager.h"

NS_BEGIN(Engine)

/* Optional component-shaped owner for one generation-checked actor handle.
   It does not own a CTransform. GameObjects push kinematic poses during
   Update and pull simulated poses during Post_Physics_Update explicitly.
   Tool controllers may use Create_Runtime without creating a GameObject. */
class ENGINE_DLL CRigidBody final : public CComponent
{
private:
	CRigidBody(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	CRigidBody(const CRigidBody& Prototype);

public:
	virtual ~CRigidBody();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	HRESULT Create_Actor(const PHYSICS_RIGID_BODY_DESC& Desc);
	HRESULT Destroy_Actor();
	HRESULT Reset_Actor(const PHYSICS_POSE& Pose);
	bool_t Is_Valid() const;
	PHYSICS_ACTOR_HANDLE Get_Handle() const { return m_Handle; }

	HRESULT Set_Pose(const PHYSICS_POSE& Pose);
	HRESULT Get_Pose(PHYSICS_POSE& outPose) const;
	HRESULT Set_LinearVelocity(const float3_t& vVelocity);
	HRESULT Get_LinearVelocity(float3_t& outVelocity) const;
	HRESULT Set_AngularVelocity(const float3_t& vVelocity);
	HRESULT Get_AngularVelocity(float3_t& outVelocity) const;
	HRESULT Add_Force(
		const float3_t& vForce,
		PHYSICS_FORCE_MODE eMode = PHYSICS_FORCE_MODE::FORCE);
	HRESULT Apply_Impulse(const float3_t& vImpulse)
	{
		return Add_Force(vImpulse, PHYSICS_FORCE_MODE::IMPULSE);
	}

	HRESULT Set_MotionMode(PHYSICS_MOTION_MODE eMode);
	HRESULT Set_GravityEnabled(bool_t isEnabled);
	HRESULT Set_GravityScale(f32_t fGravityScale);
	HRESULT Set_ConstantAcceleration(const float3_t& vAcceleration);
	HRESULT Set_Active(bool_t isActive);

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	static unique_ptr<CRigidBody> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	static shared_ptr<CRigidBody> Create_Runtime(
		const PHYSICS_RIGID_BODY_DESC& Desc);

private:
	PHYSICS_ACTOR_HANDLE m_Handle = {};
};

NS_END
