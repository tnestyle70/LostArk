#include "ForkLift.h"
#include "GameInstance.h"

CForkLift::CForkLift(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CForkLift::~CForkLift()
{
	int a = 10;

}

HRESULT CForkLift::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CForkLift::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Rotation(0.f, CGameInstance::Get().Random(0.f, 360.f), 0.f);

	return S_OK;
}

void CForkLift::Priority_Update(f32_t fTimeDelta)
{
}

void CForkLift::Update(f32_t fTimeDelta)
{

	m_pNavigationCom->SetUp_OnNavigation(m_pTransformCom);

}

void CForkLift::Late_Update(f32_t fTimeDelta)
{
	if (CGameInstance::Get().isIn_Frustum_InWorldSpace(m_pTransformCom->Get_State(STATE::POSITION), 2.f))
	{
		CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND, static_pointer_cast<CGameObject>(shared_from_this()));
#ifdef _DEBUG

		CGameInstance::Get().Add_DebugComponent(m_pNavigationCom);
#endif
	}

	
}

HRESULT CForkLift::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	uint32_t		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (uint32_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			return E_FAIL;
	
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}



	

	return S_OK;
}

HRESULT CForkLift::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_ForkLift"),
		TEXT("Com_Model"), m_pModelCom)))
		return E_FAIL;

	/* For.Com_Navigation */
	CNavigation::NAVIGATION_DESC			NaviDesc{};

	int32_t		iCellIndex = (rand() % 20) + (rand() % 20) * 256;

	NaviDesc.iStartCellIndex = iCellIndex;
	NaviDesc.pTransformCom = m_pTransformCom;

	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_TerrainNavigation"),
		TEXT("Com_Navigation"), m_pNavigationCom, &NaviDesc)))
		return E_FAIL;



	return S_OK;
}

HRESULT CForkLift::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;

	return S_OK;
}

unique_ptr<CForkLift> CForkLift::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CForkLift>(new CForkLift(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CForkLift");

	return move(pInstance);
}

shared_ptr<CPrototype> CForkLift::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CForkLift>(new CForkLift(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CForkLift");

	return pInstance;
}
