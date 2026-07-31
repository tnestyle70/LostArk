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
	m_bFollowEnabled =
		pDesc->isFollowEnabled &&
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
	if (nullptr == pFollowTarget)
		m_bFollowEnabled = false;
}

void CCamera_Free::Set_FollowEnabled(bool_t isEnabled)
{
	const bool_t nextEnabled =
		isEnabled && !m_pFollowTarget.expired();
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

void CCamera_Free::Update_Shortcuts()
{
	if (GetForegroundWindow() != g_hWnd)
		return;

	if (CGameInstance::Get().Get_DIKeyPressed(DIK_F6))
		Set_FollowEnabled(!m_bFollowEnabled);

	if (!m_bFollowEnabled &&
		CGameInstance::Get().Get_DIKeyPressed(DIK_TAB))
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

	if (CGameInstance::Get().Get_DIKeyState(DIK_W) & 0x80)
		m_pTransformCom->Go_Straight(fTimeDelta);
	if (CGameInstance::Get().Get_DIKeyState(DIK_S) & 0x80)
		m_pTransformCom->Go_Backward(fTimeDelta);
	if (CGameInstance::Get().Get_DIKeyState(DIK_A) & 0x80)
		m_pTransformCom->Go_Left(fTimeDelta);
	if (CGameInstance::Get().Get_DIKeyState(DIK_D) & 0x80)
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
		MSG_BOX("Failed to Created : CCamera_Free");

	return move(pInstance);
}

shared_ptr<CPrototype> CCamera_Free::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CCamera_Free>(new CCamera_Free(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CCamera_Free");

	return pInstance;
}
