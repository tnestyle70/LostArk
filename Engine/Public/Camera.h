#pragma once

// 카메라도 기존 GameObject의 Transform과 복제·갱신 수명에 참여하며 기기/문맥의 COM 참조를 공유한다.
#include "GameObject.h"

NS_BEGIN(Engine)

// Client의 free/follow 카메라와 연출이 공유하는 View/Projection 제출 경계다.
// Transform과 현재 연출 자세는 이 객체가 관리하며 입력 처리와 연출 시간 진행은 호출자가 담당한다.
class ENGINE_DLL CCamera abstract : public CGameObject
{
public:
	// 다른 owner가 사용 중이면 더 큰 값만 인계받는다. 같은 owner의 재요청은 우선순위를 바꾸지 않는다.
	enum class PRESENTATION_PRIORITY : uint32_t
	{
		DEFAULT = 0u,
		AUTHORING_PREVIEW = 10u,
		REFERENCE_AUDITION = 20u,
		SERVER_CINEMATIC = 100u,
		OBSERVER_HOLD = 200u
	};

	// vEye/vAt은 world 좌표, fFovy는 수직 시야각 degree다. near/far는 같은 world 길이 단위를 쓴다.
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
	// 기반 카메라에는 원형 단계의 추가 자원이 없다. 파생 클래스가 실제 생성 준비를 담당한다.
	virtual HRESULT Initialize_Prototype() override;
	// 파생 카메라가 유효한 CAMERA_DESC와 viewport를 전달한다. null/near/far/FOV 전 범위 검증이나 실패 rollback은 없다.
	virtual HRESULT Initialize(void* pArg) override;
	// 기반 세 callback은 비어 있다. Client 카메라가 free/follow/연출에 맞춰 자세 계산과 Update_PipeLine 호출 시점을 정한다.
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	// 기반 카메라는 geometry를 그리지 않고 성공을 반환한다.
	virtual HRESULT Render() override;

public:
	// 최초 자세/FOV를 한 번 저장하고 owner에게 연출 제어권을 준다. iOwnerId는 호출자가 0이 아닌 값으로 유지해야 한다.
	bool_t Begin_PresentationOverride(
		uint64_t iOwnerId,
		PRESENTATION_PRIORITY ePriority = PRESENTATION_PRIORITY::DEFAULT);
	// 현재 owner가 world eye/target 및 degree FOV를 적용한다. Transform::LookAt의 고정 +Y 기준을 쓰며 roll을 보존하지 않는다.
	bool_t Apply_PresentationPose(
		uint64_t iOwnerId,
		const float3_t& vEye,
		const float3_t& vLookAt,
		f32_t fFovYDegrees);
	// 명시 up는 저작된 roll 기준을 전달한다. 검증 실패 시 기존 적용 자세를 유지한다.
	bool_t Apply_PresentationPoseWithUp(
		uint64_t iOwnerId,
		const float3_t& vEye,
		const float3_t& vLookAt,
		const float3_t& vUp,
		f32_t fFovYDegrees);
	// 마지막으로 저장한 viewport 폭/높이 비율이다. 조회 자체가 창 크기나 투영 행렬을 갱신하지 않는다.
	f32_t Get_AspectRatio() const
	{
		return m_fAspect;
	}
	// 활성 owner만 최초 저장 자세로 복귀시킨다. 이미 비활성이면 owner와 무관하게 true다.
	bool_t End_PresentationOverride(uint64_t iOwnerId);
	// 마지막 적용 자세의 roll/FOV를 유지한 채 제어권만 해제한다.
	bool_t End_PresentationOverrideAtCurrentPose(uint64_t iOwnerId);
	// 명시한 gameplay 자세를 새 복귀점으로 삼아 해제한다. 일반 End는 최초 저장 자세로 돌아간다.
	bool_t End_PresentationOverrideToPose(uint64_t iOwnerId,
		const float3_t& vEye, const float3_t& vLookAt, f32_t fFovYDegrees);
	// 연출 제어권의 활성 여부만 반환한다. 자세나 시간을 진행시키지 않는다.
	bool_t Is_PresentationOverrideActive() const
	{
		return m_bPresentationOverrideActive;
	}
	// 현재 활성 owner와 유효한 nonzero ID가 일치하는지 확인한다. 단순 활성 조회와 구분한다.
	bool_t Is_PresentationOverrideOwnedBy(uint64_t iOwnerId) const
	{
		return m_bPresentationOverrideActive &&
			0u != iOwnerId && m_iPresentationOverrideOwnerId == iOwnerId;
	}

protected:
	// FOV는 degree, aspect는 viewport 폭/높이다. Saved는 최초 복귀 자세, Applied는 마지막 연출 적용 자세다.
	f32_t				m_fFovy = {}, m_fAspect = {}, m_fNear = {}, m_fFar = {};
	bool_t				m_bPresentationOverrideActive = false;
	float4x4_t			m_PresentationSavedWorld = {};
	float4x4_t			m_PresentationAppliedWorld = {};
	f32_t				m_fPresentationSavedFovy = 60.f;
	f32_t				m_fPresentationAppliedFovy = 60.f;
	uint64_t			m_iPresentationOverrideOwnerId = 0u;
	PRESENTATION_PRIORITY m_ePresentationOverridePriority =
		PRESENTATION_PRIORITY::DEFAULT;
	
protected:
	// 현재/연출 world 행렬의 역행렬과 LH 투영을 제출한 뒤 PipeLine 파생값과 월드 Frustum을 갱신한다.
	void Update_PipeLine();


public:
	// Prototype_Manager의 기존 복제 진입점이다. 구체 카메라가 자기 타입의 인스턴스 생성과 초기화 실패 정리를 구현한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
};

NS_END
