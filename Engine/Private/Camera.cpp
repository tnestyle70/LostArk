#include "Camera.h"

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

void CCamera::Update_PipeLine()
{
	matrix_t ViewMatrixInverse = XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	CGameInstance::Get().Set_Transform(D3DTS::VIEW, ViewMatrixInverse);
	CGameInstance::Get().Set_Transform(D3DTS::PROJ, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fFovy), m_fAspect, m_fNear, m_fFar));
}
