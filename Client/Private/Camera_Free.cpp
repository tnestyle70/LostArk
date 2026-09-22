#include <WinSock2.h>
#include <dinput.h>
#include "imgui.h"

#include "Camera_Free.h"
#include "CombatHUDViewModel.h"
#include "Effect_PresentationService.h"

#include "CameraShakeService.h"
#include "Transform.h"
#include "UIInputRouter.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr f32_t SHAKE_TRANSLATION_METERS_PER_UNIT = 0.01f;
	constexpr f32_t MIN_SHAKE_FOVY = 10.f;
	constexpr f32_t MAX_SHAKE_FOVY = 170.f;
	constexpr f32_t FOLLOW_TARGET_DISCONTINUITY_METERS = 12.f;
	constexpr std::uint32_t ANCIENT_SEA_VEHICLE_ID = 9523u;
	constexpr f32_t VEHICLE_ORBIT_RADIANS_PER_PIXEL = 0.0035f;
	constexpr f32_t VEHICLE_ORBIT_MIN_PITCH = -0.15f;
	constexpr f32_t VEHICLE_ORBIT_MAX_PITCH = 1.35f;
}

CCamera_Free::CCamera_Free(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CCamera { pDevice, pContext }
    
{
}

CCamera_Free::~CCamera_Free()
{
}

HRESULT CCamera_Free::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	auto pDesc = static_cast<CAMERA_FREE_DESC*>(pArg);
	if (!std::isfinite(pDesc->fSpeedPerSec) || pDesc->fSpeedPerSec <= 0.f ||
		!std::isfinite(pDesc->fFollowRollDegrees) ||
		!std::isfinite(pDesc->fFollowResponse) || pDesc->fFollowResponse < 0.f)
		return E_INVALIDARG;
	m_fInitialMoveSpeed = pDesc->fSpeedPerSec;
	m_fFreeMoveSpeed = pDesc->fSpeedPerSec;

	m_fMouseSensor = pDesc->fMouseSensor;
	m_pFollowTarget = pDesc->pFollowTarget;
	m_vPositionOffset = pDesc->vPositionOffset;
	m_vLookOffset = pDesc->vLookOffset;
	m_fFollowResponse = pDesc->fFollowResponse;
	m_fFollowRollDegrees = pDesc->fFollowRollDegrees;
	m_allowCapturedKeyboardInput = pDesc->allowCapturedKeyboardInput;
	m_bFollowRequested = pDesc->isFollowEnabled;
	m_bFollowEnabled =
		m_bFollowRequested &&
		nullptr != pDesc->pFollowTarget;
	m_vCurrentLookAt = pDesc->vAt;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (0.f != m_fFollowRollDegrees)
	{
		Apply_FollowRoll();
		__super::Update_PipeLine();
	}
	m_fBaseFovy = m_fFovy;
	m_vAppliedShakeOffset = {};
	CCameraShakeService::Clear();

	return S_OK;
}

void CCamera_Free::Priority_Update(f32_t fTimeDelta)
{
	Update_Shortcuts();

	if (!m_bFollowEnabled && !Is_PresentationOverrideActive())
	{
		Update_FreeCamera(fTimeDelta);
		__super::Update_PipeLine();
	}
}

void CCamera_Free::Update(f32_t fTimeDelta)
{
}

void CCamera_Free::Late_Update(f32_t fTimeDelta)
{
	CEffectPresentationService::Set_FrameCamera(static_pointer_cast<CCamera_Free>(shared_from_this()));
	Update_VehicleOrbitInput();
	if (Is_PresentationOverrideActive())
	{
		// The cinematic owns the visible pose; do not feed it into follow smoothing.
		m_bFollowInitialized = false;
		m_vAppliedShakeOffset = {};
		CAMERA_SHAKE_SAMPLE Unused;
		CCameraShakeService::Sample(fTimeDelta, Unused);
		return;
	}
	Remove_AppliedCameraShake();
	if (!m_bFollowEnabled)
	{
		CAMERA_SHAKE_SAMPLE Unused;
		CCameraShakeService::Sample(fTimeDelta, Unused);
		return;
	}

	Update_FollowCamera(fTimeDelta);
	Apply_CameraShake(fTimeDelta);
	if (m_bFollowEnabled)
		__super::Update_PipeLine();
}

HRESULT CCamera_Free::Render()
{
    return S_OK;
}

void CCamera_Free::Set_FollowTarget(const shared_ptr<CTransform>& pFollowTarget)
{
	const bool_t targetChanged =
		m_pFollowTarget.lock() != pFollowTarget;
	if (targetChanged)
	{
		m_bFollowInitialized = false;
		m_isVehicleOrbitActive = false;
		m_isVehicleOrbitDragging = false;
	}

	m_pFollowTarget = pFollowTarget;
	m_bFollowEnabled =
		m_bFollowRequested && nullptr != pFollowTarget;
	if (targetChanged && m_bFollowEnabled && !Is_PresentationOverrideActive())
	{
		Update_FollowCamera(0.f);
		__super::Update_PipeLine();
	}
}

void CCamera_Free::Set_FollowEnabled(bool_t isEnabled)
{
	m_bFollowRequested = isEnabled;
	const bool_t nextEnabled =
		m_bFollowRequested && !m_pFollowTarget.expired();
	if (m_bFollowEnabled != nextEnabled)
		m_bFollowInitialized = false;

	m_bFollowEnabled = nextEnabled;
	if (!nextEnabled)
	{
		m_isVehicleOrbitActive = false;
		m_isVehicleOrbitDragging = false;
	}
	if (m_bFollowEnabled && !m_bFollowInitialized && !Is_PresentationOverrideActive())
	{
		Update_FollowCamera(0.f);
		__super::Update_PipeLine();
	}
}

bool_t CCamera_Free::Set_FreeMoveSpeed(const f32_t metersPerSecond)
{
	if (!std::isfinite(metersPerSecond) ||
		metersPerSecond < MIN_FREE_MOVE_SPEED ||
		metersPerSecond > MAX_FREE_MOVE_SPEED)
	{
		return false;
	}
	m_fFreeMoveSpeed = metersPerSecond;
	return true;
}

void CCamera_Free::Set_PositionOffset(
	const float3_t& vPositionOffset)
{
	if (!std::isfinite(vPositionOffset.x) ||
		!std::isfinite(vPositionOffset.y) ||
		!std::isfinite(vPositionOffset.z))
	{
		return;
	}

	m_vPositionOffset = vPositionOffset;
}

bool_t CCamera_Free::Set_FollowPose(
	const float3_t& vPositionOffset,
	const float3_t& vLookOffset,
	const f32_t rollDegrees,
	const f32_t fovYDegrees,
	const f32_t followResponse)
{
	if (nullptr == m_pTransformCom ||
		!std::isfinite(vPositionOffset.x) || !std::isfinite(vPositionOffset.y) ||
		!std::isfinite(vPositionOffset.z) || !std::isfinite(vLookOffset.x) ||
		!std::isfinite(vLookOffset.y) || !std::isfinite(vLookOffset.z) ||
		!std::isfinite(rollDegrees) || !std::isfinite(fovYDegrees) ||
		fovYDegrees <= 1.f || fovYDegrees >= 179.f ||
		!std::isfinite(followResponse) || followResponse < 0.f)
	{
		return false;
	}
	const vector_t direction = XMLoadFloat3(&vLookOffset) - XMLoadFloat3(&vPositionOffset);
	const f32_t lengthSquared = XMVectorGetX(XMVector3LengthSq(direction));
	const f32_t horizontalSquared =
		(vLookOffset.x - vPositionOffset.x) * (vLookOffset.x - vPositionOffset.x) +
		(vLookOffset.z - vPositionOffset.z) * (vLookOffset.z - vPositionOffset.z);
	if (!std::isfinite(lengthSquared) || lengthSquared <= 0.000001f ||
		horizontalSquared <= lengthSquared * 0.000001f)
	{
		return false;
	}

	// An active cinematic keeps the visible pose; the next follow frame uses
	// these settings without changing ownership or the user's F6 mode.
	if (!Is_PresentationOverrideActive())
		Remove_AppliedCameraShake();
	m_vPositionOffset = vPositionOffset;
	m_vLookOffset = vLookOffset;
	m_fFollowRollDegrees = rollDegrees;
	m_fFollowResponse = followResponse;
	m_fBaseFovy = fovYDegrees;
	m_bFollowInitialized = false;
	if (!Is_PresentationOverrideActive())
	{
		m_fFovy = fovYDegrees;
		if (m_bFollowEnabled)
			Update_FollowCamera(0.f);
		__super::Update_PipeLine();
	}
	return true;
}

void CCamera_Free::Frame_Area(
	const float3_t& center,
	const f32_t radius)
{
	if (!std::isfinite(center.x) || !std::isfinite(center.y) ||
		!std::isfinite(center.z) || !std::isfinite(radius) || radius <= 0.f ||
		nullptr == m_pTransformCom)
	{
		return;
	}

	m_bFollowEnabled = false;
	m_bFollowRequested = false;
	m_bFollowInitialized = false;
	m_vAppliedShakeOffset = {};
	const vector_t lookAt = XMLoadFloat3(&center);
	const vector_t eye = XMVectorSet(
		center.x,
		center.y + radius * 0.65f,
		center.z - radius,
		1.f);
	m_pTransformCom->Set_State(STATE::POSITION, eye);
	m_pTransformCom->LookAt(lookAt);
	XMStoreFloat3(&m_vCurrentLookAt, lookAt);
	__super::Update_PipeLine();
}

void CCamera_Free::Update_Shortcuts()
{
	if (GetForegroundWindow() != g_hWnd)
		return;
	if (ImGui::GetIO().WantTextInput ||
		CUIInputRouter::Get().Is_TextInputActive())
		return;
	const bool_t useRawKeyboard = m_allowCapturedKeyboardInput;
	const auto keyPressed = [useRawKeyboard](const uint8_t keyCode)
	{
		return useRawKeyboard ?
			CGameInstance::Get().Get_DIKeyPressedRaw(keyCode) :
			CGameInstance::Get().Get_DIKeyPressed(keyCode);
	};

	if (keyPressed(DIK_F6) &&
		!m_pFollowTarget.expired())
	{
		Set_FollowEnabled(!m_bFollowRequested);
	}

	if (!m_bFollowEnabled &&
		keyPressed(DIK_TAB))
	{
		m_bMouseLookEnabled = !m_bMouseLookEnabled;
	}
}

void CCamera_Free::Update_FollowCamera(f32_t fTimeDelta)
{
	const shared_ptr<CTransform> pFollowTarget =
		m_pFollowTarget.lock();
	if (nullptr == pFollowTarget)
	{
		m_bFollowEnabled = false;
		m_bFollowInitialized = false;
		return;
	}

	const vector_t vTargetPosition =
		pFollowTarget->Get_State(STATE::POSITION);
	if (XMVector3IsNaN(vTargetPosition) || XMVector3IsInfinite(vTargetPosition))
		return;
	const vector_t targetDelta = vTargetPosition - XMLoadFloat3(&m_vPreviousFollowTarget);
	const bool_t targetDiscontinuity = m_bFollowInitialized &&
		XMVectorGetX(XMVector3LengthSq(targetDelta)) >
			FOLLOW_TARGET_DISCONTINUITY_METERS * FOLLOW_TARGET_DISCONTINUITY_METERS;
	float3_t positionOffset = m_vPositionOffset;
	float3_t lookOffset = m_vLookOffset;
	Update_VehicleOrbitOffsets(fTimeDelta, pFollowTarget, positionOffset, lookOffset);
	const vector_t vDesiredEye = XMVectorSetW(
		vTargetPosition + XMLoadFloat3(&positionOffset),
		1.f);
	const vector_t vDesiredAt = XMVectorSetW(
		vTargetPosition + XMLoadFloat3(&lookOffset),
		1.f);

	if (!m_bFollowInitialized || targetDiscontinuity ||
		m_fFollowResponse <= 0.f)
	{
		m_pTransformCom->Set_State(STATE::POSITION, vDesiredEye);
		XMStoreFloat3(&m_vCurrentLookAt, vDesiredAt);
		m_vAppliedShakeOffset = {};
		m_bFollowInitialized = true;
	}
	else if (std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
	{
		// Exponential decay is stable across frame rates and does not delay input.
		const f32_t alpha = -std::expm1(-m_fFollowResponse * fTimeDelta);
		const vector_t vNextEye = XMVectorLerp(
			m_pTransformCom->Get_State(STATE::POSITION),
			vDesiredEye,
			alpha);
		const vector_t vNextAt = XMVectorLerp(
			XMLoadFloat3(&m_vCurrentLookAt),
			vDesiredAt,
			alpha);

		m_pTransformCom->Set_State(
			STATE::POSITION,
			XMVectorSetW(vNextEye, 1.f));
		XMStoreFloat3(&m_vCurrentLookAt, vNextAt);
	}

	XMStoreFloat3(&m_vPreviousFollowTarget, vTargetPosition);
	m_pTransformCom->LookAt(
		XMLoadFloat3(&m_vCurrentLookAt));
	if (0.f != m_fFollowRollDegrees)
		Apply_FollowRoll();
}

void CCamera_Free::Update_VehicleOrbitInput()
{
	const bool_t leftDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::LB) & 0x80);
	const bool_t leftPressed = leftDown && !m_wasVehicleOrbitLeftDown;
	m_wasVehicleOrbitLeftDown = leftDown;
	const auto& player = CCombatHUDViewModel::Get().Get_Player();
	const auto& ui = CUIInputRouter::Get();
	const bool_t allowed = m_isVehicleOrbitActive && m_bFollowEnabled &&
		player.isValid && player.iVehicleId == ANCIENT_SEA_VEHICLE_ID &&
		!Is_PresentationOverrideActive() && GetForegroundWindow() == g_hWnd &&
		!CGameInstance::Get().IsMouseInputBlocked() &&
		!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantTextInput &&
		!ui.Is_TextInputActive() && !ui.Is_MouseClaimedThisFrame() &&
		!ui.Was_MouseClaimedLastFrame();
	if (!allowed || !leftDown)
		m_isVehicleOrbitDragging = false;
	else if (leftPressed)
		m_isVehicleOrbitDragging = true;
	if (!m_isVehicleOrbitDragging)
		return;

	// DirectInput deltas already integrate mouse motion over this frame.
	// Multiplying by delta time would make the orbit frame-rate dependent.
	m_fVehicleOrbitYaw = std::remainder(m_fVehicleOrbitYaw -
		CGameInstance::Get().Get_DIMouseMove(DIMM::X) * VEHICLE_ORBIT_RADIANS_PER_PIXEL,
		XM_2PI);
	m_fVehicleOrbitPitch = std::clamp(m_fVehicleOrbitPitch +
		CGameInstance::Get().Get_DIMouseMove(DIMM::Y) * VEHICLE_ORBIT_RADIANS_PER_PIXEL,
		VEHICLE_ORBIT_MIN_PITCH, VEHICLE_ORBIT_MAX_PITCH);
}

void CCamera_Free::Update_VehicleOrbitOffsets(const f32_t fTimeDelta,
	const shared_ptr<CTransform>& target, float3_t& positionOffset, float3_t& lookOffset)
{
	const auto& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || player.iVehicleId != ANCIENT_SEA_VEHICLE_ID)
	{
		m_isVehicleOrbitActive = false;
		m_isVehicleOrbitDragging = false;
		return;
	}
	const vector_t look = target->Get_State(STATE::LOOK);
	const f32_t x = XMVectorGetX(look), z = XMVectorGetZ(look);
	if (!std::isfinite(x) || !std::isfinite(z) || x * x + z * z < 0.000001f)
		return;
	const f32_t heading = std::atan2(x, z);
	if (!m_isVehicleOrbitActive)
	{
		const vector_t offset = XMLoadFloat3(&positionOffset) - XMLoadFloat3(&lookOffset);
		const f32_t radius = XMVectorGetX(XMVector3Length(offset));
		if (!std::isfinite(radius) || radius < 0.001f)
			return;
		m_fVehicleOrbitRadius = (std::max)(radius, 12.f);
		m_fVehicleOrbitYaw = std::atan2(XMVectorGetX(offset), XMVectorGetZ(offset));
		m_fVehicleOrbitPitch = std::clamp(std::asin(std::clamp(
			XMVectorGetY(offset) / radius, -1.f, 1.f)),
			VEHICLE_ORBIT_MIN_PITCH, VEHICLE_ORBIT_MAX_PITCH);
		m_fVehiclePreviousHeading = heading;
		m_isVehicleOrbitActive = true;
	}
	else if (!m_isVehicleOrbitDragging && std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
	{
		// Keep the user's viewing side while following the dragon's shortest turn.
		m_fVehicleOrbitYaw = std::remainder(m_fVehicleOrbitYaw +
			std::remainder(heading - m_fVehiclePreviousHeading, XM_2PI), XM_2PI);
	}
	m_fVehiclePreviousHeading = heading;
	lookOffset.y = (std::max)(lookOffset.y, 2.4f);
	const f32_t horizontal = m_fVehicleOrbitRadius * std::cos(m_fVehicleOrbitPitch);
	positionOffset = {
		lookOffset.x + horizontal * std::sin(m_fVehicleOrbitYaw),
		lookOffset.y + m_fVehicleOrbitRadius * std::sin(m_fVehicleOrbitPitch),
		lookOffset.z + horizontal * std::cos(m_fVehicleOrbitYaw)
	};
}

void CCamera_Free::Apply_FollowRoll()
{
	const matrix_t rotation = XMMatrixRotationAxis(
		XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)),
		XMConvertToRadians(m_fFollowRollDegrees));
	m_pTransformCom->Set_State(STATE::RIGHT, XMVector3TransformNormal(
		m_pTransformCom->Get_State(STATE::RIGHT), rotation));
	m_pTransformCom->Set_State(STATE::UP, XMVector3TransformNormal(
		m_pTransformCom->Get_State(STATE::UP), rotation));
}

void CCamera_Free::Update_FreeCamera(f32_t fTimeDelta)
{
	// DirectInput ��ġ�� BACKGROUND ����̹Ƿ� �ٸ� â�� ������ ���� ī�޶� �Է��� �����Ѵ�.
	if (GetForegroundWindow() != g_hWnd)
		return;
	const bool_t textInputActive = ImGui::GetIO().WantTextInput ||
		CUIInputRouter::Get().Is_TextInputActive();
	const bool_t useRawKeyboard =
		m_allowCapturedKeyboardInput && !textInputActive;
	const auto keyState = [useRawKeyboard](const uint8_t keyCode)
	{
		return useRawKeyboard ?
			CGameInstance::Get().Get_DIKeyStateRaw(keyCode) :
			CGameInstance::Get().Get_DIKeyState(keyCode);
	};

	const bool_t sprintHeld = !textInputActive &&
		(0 != (keyState(DIK_LSHIFT) & 0x80) ||
		 0 != (keyState(DIK_RSHIFT) & 0x80));
	// Scale only translation; the Transform keeps its initialized speed and
	// mouse rotation and follow-camera response retain their original time step.
	const f32_t fMoveDelta = fTimeDelta *
		(m_fFreeMoveSpeed / m_fInitialMoveSpeed) *
		(sprintHeld ? FREE_MOVE_SPRINT_MULTIPLIER : 1.f);

	if (!textInputActive && keyState(DIK_W) & 0x80)
		m_pTransformCom->Go_Straight(fMoveDelta);
	if (!textInputActive && keyState(DIK_S) & 0x80)
		m_pTransformCom->Go_Backward(fMoveDelta);
	if (!textInputActive && keyState(DIK_A) & 0x80)
		m_pTransformCom->Go_Left(fMoveDelta);
	if (!textInputActive && keyState(DIK_D) & 0x80)
		m_pTransformCom->Go_Right(fMoveDelta);

	if (!m_bMouseLookEnabled)
		return;

	// Free Camera������ ���� ���콺 ��ư ���� DI ��� �̵������� �ٷ� ȸ���Ѵ�.
	const int32_t mouseMoveX =
		CGameInstance::Get().Get_DIMouseMove(DIMM::X);
	const int32_t mouseMoveY =
		CGameInstance::Get().Get_DIMouseMove(DIMM::Y);

	if (0 != mouseMoveX)
	{
		m_pTransformCom->Turn(
			XMVectorSet(0.f, 1.f, 0.f, 0.f),
			m_fMouseSensor * mouseMoveX * fTimeDelta);
	}
	if (0 != mouseMoveY)
	{
		m_pTransformCom->Turn(
			m_pTransformCom->Get_State(STATE::RIGHT),
			m_fMouseSensor * mouseMoveY * fTimeDelta);
	}
}

void CCamera_Free::Remove_AppliedCameraShake()
{
	if (0.f != m_vAppliedShakeOffset.x ||
		0.f != m_vAppliedShakeOffset.y ||
		0.f != m_vAppliedShakeOffset.z)
	{
		m_pTransformCom->Set_State(
			STATE::POSITION,
			XMVectorSetW(
				m_pTransformCom->Get_State(STATE::POSITION) -
					XMLoadFloat3(&m_vAppliedShakeOffset),
				1.f));
		m_vAppliedShakeOffset = {};
	}
	m_fFovy = m_fBaseFovy;
}

void CCamera_Free::Apply_CameraShake(f32_t fTimeDelta)
{
	CAMERA_SHAKE_SAMPLE Sample;
	const bool_t bActive = CCameraShakeService::Sample(fTimeDelta, Sample);
	if (!bActive || !m_bFollowEnabled || Is_PresentationOverrideActive())
		return;

	const vector_t vLook =
		XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	const vector_t vRight =
		XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT));
	const vector_t vUp =
		XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP));
	const vector_t vOffset =
		(vLook * Sample.fForward + vRight * Sample.fRight + vUp * Sample.fUp) *
		SHAKE_TRANSLATION_METERS_PER_UNIT;
	XMStoreFloat3(&m_vAppliedShakeOffset, vOffset);
	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSetW(
			m_pTransformCom->Get_State(STATE::POSITION) + vOffset,
			1.f));
	m_fFovy = std::clamp(
		m_fBaseFovy + Sample.fFovDeltaDegrees,
		MIN_SHAKE_FOVY, MAX_SHAKE_FOVY);
}

unique_ptr<CCamera_Free> CCamera_Free::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CCamera_Free>(new CCamera_Free(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		OutputDebugStringA("[Client][Camera] Create failed.\n");

	return move(pInstance);
}

shared_ptr<CPrototype> CCamera_Free::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CCamera_Free>(new CCamera_Free(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		OutputDebugStringA("[Client][Camera] Clone failed.\n");

	return pInstance;
}
