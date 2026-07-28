#include "Monster.h"
#include "GameInstance.h"

CMonster::CMonster(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CMonster::~CMonster()
{
	int a = 10;

}

HRESULT CMonster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	
	m_pModelCom->Set_Animation(rand() % 20, true);


	return S_OK;
}

void CMonster::Priority_Update(f32_t fTimeDelta)
{
}

void CMonster::Update(f32_t fTimeDelta)
{
	if (true == m_pModelCom->Play_Animation(fTimeDelta))
		int a = 10;

	m_pNavigationCom->SetUp_OnNavigation(m_pTransformCom);

	for (auto& pColliderCom : m_ColliderCom)
		pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	Collision_ToPlayer();
}

void CMonster::Late_Update(f32_t fTimeDelta)
{

	CGameInstance::Get().Add_RenderObject(RENDERGROUP::SHADOW, static_pointer_cast<CGameObject>(shared_from_this()));
	CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND, static_pointer_cast<CGameObject>(shared_from_this()));	

#ifdef _DEBUG
	for (auto& pColliderCom : m_ColliderCom)
		CGameInstance::Get().Add_DebugComponent(pColliderCom);
	
	CGameInstance::Get().Add_DebugComponent(m_pNavigationCom);
#endif
}

HRESULT CMonster::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	uint32_t		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (uint32_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;
	
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}



	

	return S_OK;
}

HRESULT CMonster::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;



	uint32_t		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (uint32_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;


}

HRESULT CMonster::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Com_Shader"), m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fiona"),
		TEXT("Com_Model"), m_pModelCom)))
		return E_FAIL;

	/* For.Com_Navigation */
	CNavigation::NAVIGATION_DESC			NaviDesc{};

	int32_t		iCellIndex = (rand() % 40 + 20) + (rand() % 30 + 10) * 256;

	NaviDesc.iStartCellIndex = iCellIndex;
	NaviDesc.pTransformCom = m_pTransformCom;

	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_TerrainNavigation"),
		TEXT("Com_Navigation"), m_pNavigationCom, &NaviDesc)))
		return E_FAIL;


	/* For.Com_Collider_AABB */
	CBounding_AABB::BOUNDING_AABB_DESC			AABBDesc{};
	AABBDesc.vSize = float3_t(0.8f, 1.2f, 0.8f);
	AABBDesc.vCenter = float3_t(0.f, AABBDesc.vSize.y * 0.5f, 0.f);

	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider_AABB"), m_ColliderCom[COLLIDER::AABB], &AABBDesc)))
		return E_FAIL;

	/* For.Com_Collider_Sphere */
	CBounding_Sphere::BOUNDING_SPHERE_DESC		SphereDesc{};
	SphereDesc.fRadius = 0.5f;
	SphereDesc.vCenter = float3_t(0.f, SphereDesc.fRadius, 0.f);

	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider_Sphere"), m_ColliderCom[COLLIDER::SPHERE], &SphereDesc)))
		return E_FAIL;


	/* For.Com_Collider_OBB */
	CBounding_OBB::BOUNDING_OBB_DESC		OBBDesc{};
	OBBDesc.vSize = float3_t(0.8f, 0.8f, 0.8f);
	OBBDesc.vDegree = float3_t(0.f, 45.f, 0.f);
	OBBDesc.vCenter = float3_t(0.f, OBBDesc.vSize.y * 0.5f, 0.f);

	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider_OBB"), m_ColliderCom[COLLIDER::OBB], &OBBDesc)))
		return E_FAIL;



	return S_OK;
}

HRESULT CMonster::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;

	return S_OK;
}

void CMonster::Collision_ToPlayer()
{
	// auto	pTargetCollider = static_pointer_cast<CCollider>(CGameInstance::Get().Get_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Layer_Player"), TEXT("Com_Collider_AABB"), 0));
	// auto	pTargetCollider = static_pointer_cast<CCollider>(CGameInstance::Get().Get_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Layer_Player"), TEXT("Part_Body"), TEXT("Com_Collider_Sphere"), 0));
	auto	pTargetCollider = static_pointer_cast<CCollider>(CGameInstance::Get().Get_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Layer_Player"), TEXT("Part_Weapon"), TEXT("Com_Collider_OBB"), 0));

	m_ColliderCom[COLLIDER::OBB]->Intersect(pTargetCollider);

}

unique_ptr<CMonster> CMonster::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CMonster>(new CMonster(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CMonster");

	return move(pInstance);
}

shared_ptr<CPrototype> CMonster::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CMonster>(new CMonster(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CMonster");

	return pInstance;
}
