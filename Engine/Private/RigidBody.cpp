#include "RigidBody.h"

#include "GameInstance.h"

namespace
{
	CPhysics_Manager* GetPhysicsManager()
	{
		return CGameInstance::Get().Get_PhysicsManager();
	}
}

CRigidBody::CRigidBody(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

CRigidBody::CRigidBody(const CRigidBody& Prototype)
	: CComponent { Prototype }
{
}

CRigidBody::~CRigidBody()
{
	Destroy_Actor();
}

HRESULT CRigidBody::Initialize_Prototype()
{
	return CComponent::Initialize_Prototype();
}

HRESULT CRigidBody::Initialize(void* pArg)
{
	if (FAILED(CComponent::Initialize(pArg)))
		return E_FAIL;
	if (nullptr == pArg)
		return S_OK;
	return Create_Actor(*static_cast<PHYSICS_RIGID_BODY_DESC*>(pArg));
}

HRESULT CRigidBody::Create_Actor(const PHYSICS_RIGID_BODY_DESC& Desc)
{
	if (Is_Valid())
		return E_FAIL;
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Create_Actor(Desc, m_Handle) : E_FAIL;
}

HRESULT CRigidBody::Destroy_Actor()
{
	if (!m_Handle.Is_Valid())
		return S_FALSE;

	CPhysics_Manager* pPhysics = GetPhysicsManager();
	const HRESULT result = nullptr != pPhysics ?
		pPhysics->Destroy_Actor(m_Handle) : S_FALSE;
	m_Handle.Invalidate();
	return result;
}

HRESULT CRigidBody::Reset_Actor(const PHYSICS_POSE& Pose)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Reset_Actor(m_Handle, Pose) : E_FAIL;
}

bool_t CRigidBody::Is_Valid() const
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics && pPhysics->Is_ActorValid(m_Handle);
}

HRESULT CRigidBody::Set_Pose(const PHYSICS_POSE& Pose)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_Pose(m_Handle, Pose) : E_FAIL;
}

HRESULT CRigidBody::Get_Pose(PHYSICS_POSE& outPose) const
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Get_Pose(m_Handle, outPose) : E_FAIL;
}

HRESULT CRigidBody::Set_LinearVelocity(const float3_t& vVelocity)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_LinearVelocity(m_Handle, vVelocity) : E_FAIL;
}

HRESULT CRigidBody::Get_LinearVelocity(float3_t& outVelocity) const
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Get_LinearVelocity(m_Handle, outVelocity) : E_FAIL;
}

HRESULT CRigidBody::Set_AngularVelocity(const float3_t& vVelocity)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_AngularVelocity(m_Handle, vVelocity) : E_FAIL;
}

HRESULT CRigidBody::Get_AngularVelocity(float3_t& outVelocity) const
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Get_AngularVelocity(m_Handle, outVelocity) : E_FAIL;
}

HRESULT CRigidBody::Add_Force(
	const float3_t& vForce,
	PHYSICS_FORCE_MODE eMode)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Add_Force(m_Handle, vForce, eMode) : E_FAIL;
}

HRESULT CRigidBody::Set_MotionMode(PHYSICS_MOTION_MODE eMode)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_MotionMode(m_Handle, eMode) : E_FAIL;
}

HRESULT CRigidBody::Set_GravityEnabled(bool_t isEnabled)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_GravityEnabled(m_Handle, isEnabled) : E_FAIL;
}

HRESULT CRigidBody::Set_GravityScale(f32_t fGravityScale)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_GravityScale(m_Handle, fGravityScale) : E_FAIL;
}

HRESULT CRigidBody::Set_ConstantAcceleration(
	const float3_t& vAcceleration)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_ConstantAcceleration(m_Handle, vAcceleration) : E_FAIL;
}

HRESULT CRigidBody::Set_Active(bool_t isActive)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_Active(m_Handle, isActive) : E_FAIL;
}

shared_ptr<CPrototype> CRigidBody::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CRigidBody>(new CRigidBody(*this));
	if (FAILED(pInstance->Initialize(pArg)))
		return nullptr;
	return pInstance;
}

unique_ptr<CRigidBody> CRigidBody::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CRigidBody>(
		new CRigidBody(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
		return nullptr;
	return pInstance;
}

shared_ptr<CRigidBody> CRigidBody::Create_Runtime(
	const PHYSICS_RIGID_BODY_DESC& Desc)
{
	auto pInstance = shared_ptr<CRigidBody>(new CRigidBody(nullptr, nullptr));
	if (FAILED(pInstance->Initialize(
		const_cast<PHYSICS_RIGID_BODY_DESC*>(&Desc))))
	{
		return nullptr;
	}
	return pInstance;
}
