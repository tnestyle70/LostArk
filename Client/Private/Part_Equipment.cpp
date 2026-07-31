#include "Part_Equipment.h"

#include "GameInstance.h"

CPart_Equipment::CPart_Equipment(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CPart_Equipment::~CPart_Equipment()
{
}

HRESULT CPart_Equipment::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPart_Equipment::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<PART_EQUIPMENT_DESC*>(pArg);
	m_pSkeletonModelCom = pDesc->pSkeletonModel;
	m_pSocketBoneName = pDesc->pSocketBoneName;
	m_pSocketRootMatrix = pDesc->pSocketRootMatrix;

	/* Both kinds need the body's model: a socketed piece reads one bone from it,
	a skinned piece borrows its whole palette. */
	if (nullptr == m_pSkeletonModelCom)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	return S_OK;
}

void CPart_Equipment::Priority_Update(f32_t fTimeDelta)
{
}

void CPart_Equipment::Update(f32_t fTimeDelta)
{
	matrix_t ChildMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	/* A socketed piece rides one bone, and that bone's matrix already carries the
	body's pre-transform. A skinned piece adds nothing here: its deformation lives
	entirely in the bone palette bound at render time. */
	if (nullptr != m_pSocketBoneName)
	{
		ChildMatrix = ChildMatrix * m_pSkeletonModelCom->Get_BoneMatrix(m_pSocketBoneName);

		if (nullptr != m_pSocketRootMatrix)
		{
			ChildMatrix = ChildMatrix * XMLoadFloat4x4(m_pSocketRootMatrix);
		}
	}

	__super::Update_CombinedWorldMatrix(ChildMatrix);
}

void CPart_Equipment::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CPart_Equipment::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* The cooked path gives every skinned mesh the same skeleton-wide palette, so
	one bind covers all of this piece's meshes and any body mesh index produces it. */
	if (nullptr == m_pSocketBoneName &&
		FAILED(m_pSkeletonModelCom->Bind_BoneMatrices(
			m_pShaderCom, "g_BoneMatrices", 0)))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		const uint32_t hasNormalTexture =
			m_pModelCom->Has_MaterialTexture(i, aiTextureType_NORMALS) ? 1u : 0u;
		if (FAILED(m_pModelCom->Bind_Material(
			m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_HasNormalTexture", &hasNormalTexture, sizeof(hasNormalTexture))) ||
			(0 != hasNormalTexture && FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0))) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Equipment::Ready_Components(const PART_EQUIPMENT_DESC* pDesc)
{
	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strShaderTag,
		TEXT("Com_Shader"),
		m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strModelTag,
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPart_Equipment::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

unique_ptr<CPart_Equipment> CPart_Equipment::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CPart_Equipment>(new CPart_Equipment(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPart_Equipment");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CPart_Equipment::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CPart_Equipment>(new CPart_Equipment(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPart_Equipment");
		return nullptr;
	}
	return pInstance;
}
