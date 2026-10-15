// actor handle의 생성/해제 책임을 Component 수명에 연결한다.
#include "RigidBody.h"

// 각 호출 시 현재 Physics_Manager를 조회한다. manager의 raw pointer를 멤버에 저장하지 않는다.
#include "GameInstance.h"

namespace
{
	// GameInstance가 소유한 manager를 호출 중만 차용한다. 종료 순서상 manager가 없으면 각 wrapper가 실패/빈 결과로 처리한다.
	CPhysics_Manager* GetPhysicsManager()
	{
		return CGameInstance::Get().Get_PhysicsManager();
	}
}

// 기반 Component의 장치 참조를 준비한다. actor는 Initialize에서 명세를 받은 경우에만 생성한다.
CRigidBody::CRigidBody(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

// 원형의 기반 상태만 복제한다. m_Handle은 기본 무효값이므로 원형 actor와 해제 책임을 공유하지 않는다.
CRigidBody::CRigidBody(const CRigidBody& Prototype)
	: CComponent { Prototype }
{
}

// 명시적으로 제거하지 않은 actor를 정리한다. 소멸자는 HRESULT를 외부로 반환하지 않는다.
CRigidBody::~CRigidBody()
{
	Destroy_Actor();
}

// 원형은 물리 actor를 보유할 필요가 없어 기반 초기화 결과만 전달한다.
HRESULT CRigidBody::Initialize_Prototype()
{
	return CComponent::Initialize_Prototype();
}

// Clone과 Create_Runtime이 호출한다. pArg의 명세를 복사해 보관하지 않고 manager 생성에 동기적으로 전달한다.
HRESULT CRigidBody::Initialize(void* pArg)
{
	if (FAILED(CComponent::Initialize(pArg)))
		return E_FAIL;
	if (nullptr == pArg)
		return S_OK;
	return Create_Actor(*static_cast<PHYSICS_RIGID_BODY_DESC*>(pArg));
}

// 이미 유효한 actor가 있으면 E_FAIL이다. manager의 생성 실패는 이 객체의 handle을 무효화한다.
HRESULT CRigidBody::Create_Actor(const PHYSICS_RIGID_BODY_DESC& Desc)
{
	if (Is_Valid())
		return E_FAIL;
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Create_Actor(Desc, m_Handle) : E_FAIL;
}

// actor 제거를 요청한 뒤 반환 HRESULT와 무관하게 로컬 handle을 무효화한다. 이미 빈 handle은 S_FALSE다.
HRESULT CRigidBody::Destroy_Actor()
{
	if (!m_Handle.Is_Valid())
		return S_FALSE;

	CPhysics_Manager* pPhysics = GetPhysicsManager();
	const HRESULT result = nullptr != pPhysics ?
		pPhysics->Destroy_Actor(m_Handle) : S_FALSE;
	// manager가 없거나 이미 삭제됐더라도 이 소유자는 더 이상 해당 actor를 추적하지 않는다.
	m_Handle.Invalidate();
	return result;
}

// pose 변경과 dynamic 속도0·기본 eFORCE 누적 초기화를 요청한다. active/kinematic 추가 검사는 없으며 충격량 누적은 별도로 비우지 않는다.
HRESULT CRigidBody::Reset_Actor(const PHYSICS_POSE& Pose)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Reset_Actor(m_Handle, Pose) : E_FAIL;
}

// Level Clear나 외부 Destroy 이후에도 로컬 handle 값은 남을 수 있으므로 manager의 세대 검사까지 통과해야 한다.
bool_t CRigidBody::Is_Valid() const
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics && pPhysics->Is_ActorValid(m_Handle);
}

// kinematic은 다음 step의 목표 pose를, 그 외 actor는 즉시 global pose를 설정한다. CTransform에는 직접 쓰지 않는다.
HRESULT CRigidBody::Set_Pose(const PHYSICS_POSE& Pose)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_Pose(m_Handle, Pose) : E_FAIL;
}

// 현재 actor root의 월드 pose를 읽는다. manager/handle 오류 시 출력은 보존된다.
HRESULT CRigidBody::Get_Pose(PHYSICS_POSE& outPose) const
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Get_Pose(m_Handle, outPose) : E_FAIL;
}

// simulated dynamic actor의 질량 중심 선속도를 월드 거리/초 단위로 지정한다. static/kinematic은 거절된다.
HRESULT CRigidBody::Set_LinearVelocity(const float3_t& vVelocity)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_LinearVelocity(m_Handle, vVelocity) : E_FAIL;
}

// dynamic actor의 현재 선속도를 읽는다. 읽기 실패면 출력 값을 보존한다.
HRESULT CRigidBody::Get_LinearVelocity(float3_t& outVelocity) const
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Get_LinearVelocity(m_Handle, outVelocity) : E_FAIL;
}

// simulated dynamic actor의 월드 각속도(rad/s)를 지정한다. 값의 유한성과 actor 종류는 manager가 검사한다.
HRESULT CRigidBody::Set_AngularVelocity(const float3_t& vVelocity)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_AngularVelocity(m_Handle, vVelocity) : E_FAIL;
}

// dynamic actor의 각속도(rad/s)를 읽는다. 읽기 실패면 출력 값을 보존한다.
HRESULT CRigidBody::Get_AngularVelocity(float3_t& outVelocity) const
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Get_AngularVelocity(m_Handle, outVelocity) : E_FAIL;
}

// 활성 simulated dynamic actor에 지정 모드의 힘/충격량/속도 변화/가속도를 전달한다. 질량 적용은 모드에 따라 다르다.
HRESULT CRigidBody::Add_Force(
	const float3_t& vForce,
	PHYSICS_FORCE_MODE eMode)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Add_Force(m_Handle, vForce, eMode) : E_FAIL;
}

// dynamic actor를 simulated/kinematic으로 전환한다. static actor는 거절되며 활성 simulated 전환은 깨운다.
HRESULT CRigidBody::Set_MotionMode(PHYSICS_MOTION_MODE eMode)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_MotionMode(m_Handle, eMode) : E_FAIL;
}

// manager가 step마다 적용할 개별 중력의 사용 여부를 바꾼다. dynamic actor만 허용한다.
HRESULT CRigidBody::Set_GravityEnabled(bool_t isEnabled)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_GravityEnabled(m_Handle, isEnabled) : E_FAIL;
}

// 개별 중력에 곱할 유한한 0 이상 배율을 설정한다. 일정 가속도는 이 배율과 별도로 적용된다.
HRESULT CRigidBody::Set_GravityScale(f32_t fGravityScale)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_GravityScale(m_Handle, fGravityScale) : E_FAIL;
}

// 매 step 적용할 월드 가속도(거리/초²)를 저장한다. 즉시 pose를 이동시키지는 않는다.
HRESULT CRigidBody::Set_ConstantAcceleration(
	const float3_t& vAcceleration)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_ConstantAcceleration(m_Handle, vAcceleration) : E_FAIL;
}

// actor를 삭제하지 않고 simulation 참여 flag를 바꾼다. handle과 소유 관계는 유지한다.
HRESULT CRigidBody::Set_Active(bool_t isActive)
{
	CPhysics_Manager* pPhysics = GetPhysicsManager();
	return nullptr != pPhysics ?
		pPhysics->Set_Active(m_Handle, isActive) : E_FAIL;
}

// 새 Component가 초기화에 실패하면 지역 shared_ptr 소멸로 actor의 해제 책임도 마무리한다.
shared_ptr<CPrototype> CRigidBody::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CRigidBody>(new CRigidBody(*this));
	if (FAILED(pInstance->Initialize(pArg)))
		return nullptr;
	return pInstance;
}

// 원형 준비 실패는 nullptr다. 할당 예외를 HRESULT로 바꾸는 별도 처리는 없다.
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

// GameObject 없는 도구/파괴 연출에서 사용하는 경로다. const_cast는 기존 void* 초기화 경계에 맞추며 명세를 수정하지 않는다.
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
