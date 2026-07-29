#include "LanceMaster.h"

#include "Body_LanceMaster.h"

CLanceMaster::CLanceMaster(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CContainerObject { pDevice, pContext }
{
}

CLanceMaster::~CLanceMaster()
{
}

HRESULT CLanceMaster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLanceMaster::Initialize(void* pArg)
{
	LANCEMASTER_DESC desc{};
	desc.fSpeedPerSec = 0.f;
	desc.fRotationPerSec = 0.f;
	if (nullptr != pArg)
		desc.vPosition = static_cast<LANCEMASTER_DESC*>(pArg)->vPosition;

	if (FAILED(__super::Initialize(&desc)))
		return E_FAIL;
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(desc.vPosition.x, desc.vPosition.y, desc.vPosition.z, 1.f));

	return Ready_PartObjects();
}

void CLanceMaster::Priority_Update(f32_t fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLanceMaster::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLanceMaster::Late_Update(f32_t fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLanceMaster::Render()
{
	return S_OK;
}

HRESULT CLanceMaster::Ready_PartObjects()
{
	CBody_LanceMaster::BODY_LANCEMASTER_DESC bodyDesc{};
	bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.pParentState = &m_iState;

	return __super::Add_PartObject(
		ETOUI(LEVEL::TEST_LEVEL2),
		TEXT("Prototype_GameObject_Body_LanceMaster"),
		TEXT("Part_Body"),
		&bodyDesc);
}

unique_ptr<CLanceMaster> CLanceMaster::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLanceMaster>(new CLanceMaster(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLanceMaster");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CLanceMaster::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CLanceMaster>(new CLanceMaster(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLanceMaster");
		return nullptr;
	}
	return pInstance;
}
