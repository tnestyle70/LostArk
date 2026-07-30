#include "Camera_Free.h"

#include "Transform.h"

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
	auto	pDesc = static_cast<CAMERA_FREE_DESC*>(pArg);

	m_fMouseSensor = pDesc->fMouseSensor;
	m_pFollowTarget = pDesc->pFollowTarget;
	m_vFollowOffset = pDesc->vFollowOffset;
	m_vLookOffset = pDesc->vLookOffset;
	m_bFollowEnabled = pDesc->isFollowEnabled;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Free::Priority_Update(f32_t fTimeDelta)
{
	const bool_t isF6Down =
		0 != (CGameInstance::Get().Get_DIKeyState(DIK_F6) & 0x80);
	if (isF6Down && false == m_bF6Down && false == m_pFollowTarget.expired())
		m_bFollowEnabled = !m_bFollowEnabled;
	m_bF6Down = isF6Down;

	if (m_bFollowEnabled)
	{
		shared_ptr<CTransform> pFollowTarget = m_pFollowTarget.lock();
		if (nullptr == pFollowTarget)
		{
			m_bFollowEnabled = false;
		}
		else
		{
		const vector_t vTargetPosition =
			pFollowTarget->Get_State(STATE::POSITION);
		const vector_t vEye = XMVectorSetW(
			vTargetPosition + XMLoadFloat3(&m_vFollowOffset),
			1.f);
		const vector_t vAt = XMVectorSetW(
			vTargetPosition + XMLoadFloat3(&m_vLookOffset),
			1.f);

		m_pTransformCom->Set_State(STATE::POSITION, vEye);
		m_pTransformCom->LookAt(vAt);
		__super::Update_PipeLine();
		return;
		}
	}

	const bool_t bTabDown =
		0 != (CGameInstance::Get().Get_DIKeyState(DIK_TAB) & 0x80);
	if (bTabDown && !m_bTabDown)
		m_bMovementLocked = !m_bMovementLocked;
	m_bTabDown = bTabDown;

	if (!m_bMovementLocked &&
		(CGameInstance::Get().Get_DIKeyState(DIK_W) & 0x80))
	{
		m_pTransformCom->Go_Straight(fTimeDelta);
	}
	if (!m_bMovementLocked &&
		(CGameInstance::Get().Get_DIKeyState(DIK_S) & 0x80))
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}
	if (!m_bMovementLocked &&
		(CGameInstance::Get().Get_DIKeyState(DIK_A) & 0x80))
	{
		m_pTransformCom->Go_Left(fTimeDelta);
	}
	if (!m_bMovementLocked &&
		(CGameInstance::Get().Get_DIKeyState(DIK_D) & 0x80))
	{
		m_pTransformCom->Go_Right(fTimeDelta);
	}

	int32_t			iMouseMove = {};

	if (!m_bMovementLocked &&
		(iMouseMove = CGameInstance::Get().Get_DIMouseMove(DIMM::X)))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fMouseSensor * iMouseMove * fTimeDelta);
	}
	if (!m_bMovementLocked &&
		(iMouseMove = CGameInstance::Get().Get_DIMouseMove(DIMM::Y)))
	{
		m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::RIGHT), m_fMouseSensor * iMouseMove * fTimeDelta);
	}


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
