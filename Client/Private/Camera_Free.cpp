#include "Camera_Free.h"

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

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Free::Priority_Update(f32_t fTimeDelta)
{
	if(CGameInstance::Get().Get_DIKeyState(DIK_W) & 0x80)
	{
		m_pTransformCom->Go_Straight(fTimeDelta);
	}
	if (CGameInstance::Get().Get_DIKeyState(DIK_S) & 0x80)
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}
	if (CGameInstance::Get().Get_DIKeyState(DIK_A) & 0x80)
	{
		m_pTransformCom->Go_Left(fTimeDelta);
	}
	if (CGameInstance::Get().Get_DIKeyState(DIK_D) & 0x80)
	{
		m_pTransformCom->Go_Right(fTimeDelta);
	}

	int32_t			iMouseMove = {};

	if (iMouseMove = CGameInstance::Get().Get_DIMouseMove(DIMM::X))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fMouseSensor * iMouseMove * fTimeDelta);
	}
	if (iMouseMove = CGameInstance::Get().Get_DIMouseMove(DIMM::Y))
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
