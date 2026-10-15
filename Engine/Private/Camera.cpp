// 카메라 Transform과 연출 owner/복귀 자세를 관리하고 기존 GameInstance 경계로 View/Projection을 제출한다.
#include "Camera.h"

// 연출 위치·FOV·basis 길이와 viewport 크기의 유한성을 검사한다.
#include <cmath>

CCamera::CCamera(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{

}

CCamera::~CCamera()
{
}

// 공통 카메라 prototype에는 별도 초기화가 없다. 파생 카메라가 자신의 초기화를 추가한다.
HRESULT CCamera::Initialize_Prototype()
{
	return S_OK;
}

// descriptor 값과 viewport 비율을 읽고 CGameObject가 Transform을 준비한 뒤 최초 View/Projection을 제출한다.
// descriptor null, 높이 0과 잘못된 clip 범위를 여기서 거르지 않으며 부모 초기화 실패 시 앞서 쓴 멤버를 되돌리지 않는다.
HRESULT CCamera::Initialize(void* pArg)
{
	auto		pDesc = static_cast<CAMERA_DESC*>(pArg);

	m_fFovy = pDesc->fFovy;
	m_fNear = pDesc->fNear;
	m_fFar = pDesc->fFar;
	m_fAspect = CGameInstance::Get().Get_ViewportSize().x / CGameInstance::Get().Get_ViewportSize().y;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vEye), 1.f));
	m_pTransformCom->LookAt(XMLoadFloat3(&pDesc->vAt));

	Update_PipeLine();

	return S_OK;
}

// 기본 프레임 callback은 비어 있다. 실제 이동 입력과 최종 카메라 제출 시점은 파생 카메라가 정한다.
void CCamera::Priority_Update(f32_t fTimeDelta)
{
}

void CCamera::Update(f32_t fTimeDelta)
{
}

void CCamera::Late_Update(f32_t fTimeDelta)
{
}

// 카메라는 이 기본 함수에서 geometry를 그리지 않는다. 행렬 제출은 Update_PipeLine이 수행한다.
HRESULT CCamera::Render()
{
	return S_OK;
}

// CameraTool/제품 연출/관찰 카메라가 owner와 우선순위로 제어권을 요청한다. 내부 mutex 없이 같은 갱신 흐름에서 사용한다.
// 활성 상태의 우선권 인계는 아래 0 ID 검사보다 먼저 실행되므로 호출자가 항상 nonzero owner를 전달해야 한다.
bool_t CCamera::Begin_PresentationOverride(
	const uint64_t iOwnerId,
	const PRESENTATION_PRIORITY ePriority)
{
	if (m_bPresentationOverrideActive)
	{
		if (m_iPresentationOverrideOwnerId == iOwnerId)
			return true;
		if (static_cast<uint32_t>(ePriority) <=
			static_cast<uint32_t>(m_ePresentationOverridePriority))
		{
			return false;
		}
		// 높은 우선순위의 owner는 최초 Saved 자세를 그대로 인계받는다.
		// 중간 preview 자세를 복귀점으로 덮어쓰지 않으므로 일반 End가 최초 자세를 복원한다.
		m_iPresentationOverrideOwnerId = iOwnerId;
		m_ePresentationOverridePriority = ePriority;
		return true;
	}
	if (0u == iOwnerId || nullptr == m_pTransformCom)
		return false;
	// 새 연출의 최초 자세를 Saved와 Applied에 함께 기록한다. 이후 높은 우선순위 인계는 이 복귀점을 덮어쓰지 않는다.
	m_PresentationSavedWorld = *m_pTransformCom->Get_WorldMatrixPtr();
	m_PresentationAppliedWorld = m_PresentationSavedWorld;
	m_fPresentationSavedFovy = m_fFovy;
	m_fPresentationAppliedFovy = m_fFovy;
	m_bPresentationOverrideActive = true;
	m_iPresentationOverrideOwnerId = iOwnerId;
	m_ePresentationOverridePriority = ePriority;
	return true;
}

// 검사에서 false를 반환할 때는 기존 적용 자세를 유지한다. 눈과 목표를 잇는 방향으로 Transform::LookAt을 호출한다.
// 이 경로는 방향 연산의 overflow와 고정 +Y에 평행한 시선을 별도로 검사하지 않는다. 명시 up 경로와 검증 범위가 다르다.
bool_t CCamera::Apply_PresentationPose(
	const uint64_t iOwnerId,
	const float3_t& vEye,
	const float3_t& vLookAt,
	const f32_t fFovYDegrees)
{
	if (!m_bPresentationOverrideActive ||
		m_iPresentationOverrideOwnerId != iOwnerId ||
		nullptr == m_pTransformCom ||
		!std::isfinite(vEye.x) || !std::isfinite(vEye.y) ||
		!std::isfinite(vEye.z) || !std::isfinite(vLookAt.x) ||
		!std::isfinite(vLookAt.y) || !std::isfinite(vLookAt.z) ||
		!std::isfinite(fFovYDegrees) || fFovYDegrees <= 1.f ||
		fFovYDegrees >= 179.f)
	{
		return false;
	}
	const vector_t eye = XMLoadFloat3(&vEye);
	const vector_t lookAt = XMLoadFloat3(&vLookAt);
	if (XMVectorGetX(XMVector3LengthSq(lookAt - eye)) <= 0.000001f)
		return false;
	// 소유권·입력·최소 시선 길이 검사가 끝난 뒤 실제 자세와 마지막 연출 자세를 함께 갱신한다.
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(eye, 1.f));
	m_pTransformCom->LookAt(lookAt);
	m_fFovy = fFovYDegrees;
	m_PresentationAppliedWorld = *m_pTransformCom->Get_WorldMatrixPtr();
	m_fPresentationAppliedFovy = m_fFovy;
	Update_PipeLine();
	return true;
}

// 저작된 up을 기준으로 roll을 포함한 직교 basis를 만든다. 반환 false인 검사는 Transform을 쓰기 전에 끝낸다.
bool_t CCamera::Apply_PresentationPoseWithUp(
	const uint64_t iOwnerId,
	const float3_t& vEye,
	const float3_t& vLookAt,
	const float3_t& vUp,
	const f32_t fFovYDegrees)
{
	if (!Is_PresentationOverrideOwnedBy(iOwnerId) || nullptr == m_pTransformCom ||
		!std::isfinite(vEye.x) || !std::isfinite(vEye.y) || !std::isfinite(vEye.z) ||
		!std::isfinite(vLookAt.x) || !std::isfinite(vLookAt.y) || !std::isfinite(vLookAt.z) ||
		!std::isfinite(vUp.x) || !std::isfinite(vUp.y) || !std::isfinite(vUp.z) ||
		!std::isfinite(fFovYDegrees) || fFovYDegrees <= 1.f || fFovYDegrees >= 179.f)
		return false;
	const vector_t eye = XMLoadFloat3(&vEye);
	const vector_t direction = XMLoadFloat3(&vLookAt) - eye;
	const f32_t lengthSq = XMVectorGetX(XMVector3LengthSq(direction));
	const f32_t upLengthSq = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&vUp)));
	if (!std::isfinite(lengthSq) || lengthSq <= 0.000001f ||
		!std::isfinite(upLengthSq) || upLengthSq <= 0.000001f)
		return false;
	// look은 eye에서 target 방향이다. up과 look의 외적 길이가 너무 작으면 서로 평행해 오른쪽 축을 만들 수 없다.
	const vector_t look = XMVector3Normalize(direction);
	const vector_t cross = XMVector3Cross(XMVector3Normalize(XMLoadFloat3(&vUp)), look);
	const f32_t crossLengthSq = XMVectorGetX(XMVector3LengthSq(cross));
	if (!std::isfinite(crossLengthSq) || crossLengthSq <= 0.000001f)
		return false;
	// 오른쪽과 위쪽 축을 다시 직교화한다. basis w=0, 위치 w=1로 world 행렬에 저장한다.
	const vector_t right = XMVector3Normalize(cross);
	const vector_t up = XMVector3Cross(look, right);
	m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSetW(right, 0.f));
	m_pTransformCom->Set_State(STATE::UP, XMVectorSetW(up, 0.f));
	m_pTransformCom->Set_State(STATE::LOOK, XMVectorSetW(look, 0.f));
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(eye, 1.f));
	m_fFovy = fFovYDegrees;
	m_PresentationAppliedWorld = *m_pTransformCom->Get_WorldMatrixPtr();
	m_fPresentationAppliedFovy = m_fFovy;
	Update_PipeLine();
	return true;
}

// 최초 진입 때 저장한 world/FOV를 되돌린다. priority가 높은 연출이 인계받았어도 중간 preview가 복귀 지점이 되지 않는다.
bool_t CCamera::End_PresentationOverride(const uint64_t iOwnerId)
{
	if (!m_bPresentationOverrideActive)
		return true;
	if (m_iPresentationOverrideOwnerId != iOwnerId ||
		nullptr == m_pTransformCom)
	{
		return false;
	}
	m_bPresentationOverrideActive = false;
	m_iPresentationOverrideOwnerId = 0u;
	m_ePresentationOverridePriority = PRESENTATION_PRIORITY::DEFAULT;
	const matrix_t savedWorld = XMLoadFloat4x4(&m_PresentationSavedWorld);
	m_pTransformCom->Set_State(STATE::RIGHT, savedWorld.r[0]);
	m_pTransformCom->Set_State(STATE::UP, savedWorld.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, savedWorld.r[2]);
	m_pTransformCom->Set_State(STATE::POSITION, savedWorld.r[3]);
	m_fFovy = m_fPresentationSavedFovy;
	Update_PipeLine();
	return true;
}

// 마지막 Applied 자세를 새 복귀점으로 바꿔 해제한다. 이후 외부에서 임시 변경한 Transform 값을 캡처하는 함수는 아니다.
bool_t CCamera::End_PresentationOverrideAtCurrentPose(const uint64_t iOwnerId)
{
	if (!Is_PresentationOverrideOwnedBy(iOwnerId) || nullptr == m_pTransformCom)
		return false;
	m_PresentationSavedWorld = m_PresentationAppliedWorld;
	m_fPresentationSavedFovy = m_fPresentationAppliedFovy;
	return End_PresentationOverride(iOwnerId);
}

// 명시한 게임플레이 자세를 먼저 검사/적용하고 성공한 경우 그 자세를 복귀점으로 삼아 제어권을 해제한다.
bool_t CCamera::End_PresentationOverrideToPose(const uint64_t iOwnerId,
	const float3_t& vEye, const float3_t& vLookAt, const f32_t fFovYDegrees)
{
	if (!Apply_PresentationPose(iOwnerId, vEye, vLookAt, fFovYDegrees))
		return false;
	m_PresentationSavedWorld = m_PresentationAppliedWorld;
	m_fPresentationSavedFovy = m_fPresentationAppliedFovy;
	return End_PresentationOverride(iOwnerId);
}

// 유한한 양수 viewport이면 aspect를 갱신한다. override 중에는 저장한 Applied 자세로 외부 Transform 변경을 덮어쓴다.
// world 역행렬이 View다. FOV를 radian으로 바꾼 LH 투영과 함께 제출하며 특이 행렬/near/far 검증은 이 함수에 없다.
void CCamera::Update_PipeLine()
{
	const auto viewport = CGameInstance::Get().Get_ViewportSize();
	if (std::isfinite(viewport.x) && std::isfinite(viewport.y) &&
		viewport.x > 0.f && viewport.y > 0.f)
		m_fAspect = viewport.x / viewport.y;
	if (m_bPresentationOverrideActive && nullptr != m_pTransformCom)
	{
		const matrix_t appliedWorld =
			XMLoadFloat4x4(&m_PresentationAppliedWorld);
		m_pTransformCom->Set_State(STATE::RIGHT, appliedWorld.r[0]);
		m_pTransformCom->Set_State(STATE::UP, appliedWorld.r[1]);
		m_pTransformCom->Set_State(STATE::LOOK, appliedWorld.r[2]);
		m_pTransformCom->Set_State(STATE::POSITION, appliedWorld.r[3]);
		m_fFovy = m_fPresentationAppliedFovy;
	}
	matrix_t ViewMatrixInverse = XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	CGameInstance::Get().Set_Transform(D3DTS::VIEW, ViewMatrixInverse);
	CGameInstance::Get().Set_Transform(D3DTS::PROJ, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fFovy), m_fAspect, m_fNear, m_fFar));
	CGameInstance::Get().Refresh_CameraState();
}
