#include "imgui.h"

#include "Camera_Free.h"

#include "Transform.h"

#include <algorithm>
#include <cmath>

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

	m_fMouseSensor = pDesc->fMouseSensor;
	m_pFollowTarget = pDesc->pFollowTarget;
	m_vPositionOffset = pDesc->vPositionOffset;
	m_vLookOffset = pDesc->vLookOffset;
	m_fFollowResponse = pDesc->fFollowResponse;
	m_allowCapturedKeyboardInput = pDesc->allowCapturedKeyboardInput;
	m_bFollowRequested = pDesc->isFollowEnabled;
	m_bFollowEnabled =
		m_bFollowRequested &&
		nullptr != pDesc->pFollowTarget;
	m_vCurrentLookAt = pDesc->vAt;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Free::Priority_Update(f32_t fTimeDelta)
{
	Update_Shortcuts();

	if (m_bFollowEnabled)
		Update_FollowCamera(fTimeDelta);
	else
		Update_FreeCamera(fTimeDelta);

	__super::Update_PipeLine();
}

void CCamera_Free::Update(f32_t fTimeDelta)
{
}

void CCamera_Free::Late_Update(f32_t fTimeDelta)
{
}

HRESULT CCamera_Free::Render()
{
    return S_OK;
}

void CCamera_Free::Set_FollowTarget(const shared_ptr<CTransform>& pFollowTarget)
{
	if (m_pFollowTarget.lock() != pFollowTarget)
		m_bFollowInitialized = false;

	m_pFollowTarget = pFollowTarget;
	m_bFollowEnabled =
		m_bFollowRequested && nullptr != pFollowTarget;
}

void CCamera_Free::Set_FollowEnabled(bool_t isEnabled)
{
	m_bFollowRequested = isEnabled;
	const bool_t nextEnabled =
		m_bFollowRequested && !m_pFollowTarget.expired();
	if (m_bFollowEnabled != nextEnabled)
		m_bFollowInitialized = false;

	m_bFollowEnabled = nextEnabled;
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
	m_bFollowInitialized = false;
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
	const vector_t lookAt = XMLoadFloat3(&center);
	const vector_t eye = XMVectorSet(
		center.x,
		center.y + radius * 0.65f,
		center.z - radius,
		1.f);
	m_pTransformCom->Set_State(STATE::POSITION, eye);
	m_pTransformCom->LookAt(lookAt);
	XMStoreFloat3(&m_vCurrentLookAt, lookAt);
}

void CCamera_Free::Update_Shortcuts()
{
	if (GetForegroundWindow() != g_hWnd)
		return;
	if (ImGui::GetIO().WantTextInput)
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
	const vector_t vDesiredEye = XMVectorSetW(
		vTargetPosition + XMLoadFloat3(&m_vPositionOffset),
		1.f);
	const vector_t vDesiredAt = XMVectorSetW(
		vTargetPosition + XMLoadFloat3(&m_vLookOffset),
		1.f);

	if (!m_bFollowInitialized ||
		fTimeDelta <= 0.f ||
		m_fFollowResponse <= 0.f)
	{
		m_pTransformCom->Set_State(STATE::POSITION, vDesiredEye);
		XMStoreFloat3(&m_vCurrentLookAt, vDesiredAt);
		m_bFollowInitialized = true;
	}
	else
	{
		const f32_t clampedDelta =
			(std::min)(fTimeDelta, 0.05f);
		const f32_t alpha =
			1.f - std::exp(-m_fFollowResponse * clampedDelta);
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

	m_pTransformCom->LookAt(
		XMLoadFloat3(&m_vCurrentLookAt));
}

void CCamera_Free::Update_FreeCamera(f32_t fTimeDelta)
{
	// DirectInput 장치가 BACKGROUND 모드이므로 다른 창을 조작할 때는 카메라 입력을 무시한다.
	if (GetForegroundWindow() != g_hWnd)
		return;
	const bool_t textInputActive = ImGui::GetIO().WantTextInput;
	const bool_t useRawKeyboard =
		m_allowCapturedKeyboardInput && !textInputActive;
	const auto keyState = [useRawKeyboard](const uint8_t keyCode)
	{
		return useRawKeyboard ?
			CGameInstance::Get().Get_DIKeyStateRaw(keyCode) :
			CGameInstance::Get().Get_DIKeyState(keyCode);
	};

	if (!textInputActive && keyState(DIK_W) & 0x80)
		m_pTransformCom->Go_Straight(fTimeDelta);
	if (!textInputActive && keyState(DIK_S) & 0x80)
		m_pTransformCom->Go_Backward(fTimeDelta);
	if (!textInputActive && keyState(DIK_A) & 0x80)
		m_pTransformCom->Go_Left(fTimeDelta);
	if (!textInputActive && keyState(DIK_D) & 0x80)
		m_pTransformCom->Go_Right(fTimeDelta);

	if (!m_bMouseLookEnabled)
		return;

	// Free Camera에서는 별도 마우스 버튼 없이 DI 상대 이동량으로 바로 회전한다.
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
