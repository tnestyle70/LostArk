#include "Camera.h"

#include <cmath>

CCamera::CCamera(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{

}

CCamera::~CCamera()
{
}

HRESULT CCamera::Initialize_Prototype()
{
	return S_OK;
}

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

void CCamera::Priority_Update(f32_t fTimeDelta)
{
}

void CCamera::Update(f32_t fTimeDelta)
{
}

void CCamera::Late_Update(f32_t fTimeDelta)
{
}

HRESULT CCamera::Render()
{
	return S_OK;
}

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
		/* A higher-priority product cinematic inherits the original saved pose,
		   not the authoring preview pose it displaced. Its End therefore still
		   restores the exact camera state from before either override. */
		m_iPresentationOverrideOwnerId = iOwnerId;
		m_ePresentationOverridePriority = ePriority;
		return true;
	}
	if (0u == iOwnerId || nullptr == m_pTransformCom)
		return false;
	m_PresentationSavedWorld = *m_pTransformCom->Get_WorldMatrixPtr();
	m_PresentationAppliedWorld = m_PresentationSavedWorld;
	m_fPresentationSavedFovy = m_fFovy;
	m_fPresentationAppliedFovy = m_fFovy;
	m_bPresentationOverrideActive = true;
	m_iPresentationOverrideOwnerId = iOwnerId;
	m_ePresentationOverridePriority = ePriority;
	return true;
}

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
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(eye, 1.f));
	m_pTransformCom->LookAt(lookAt);
	m_fFovy = fFovYDegrees;
	m_PresentationAppliedWorld = *m_pTransformCom->Get_WorldMatrixPtr();
	m_fPresentationAppliedFovy = m_fFovy;
	Update_PipeLine();
	return true;
}

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

void CCamera::Update_PipeLine()
{
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
