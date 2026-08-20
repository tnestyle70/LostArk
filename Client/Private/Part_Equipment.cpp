#include "Part_Equipment.h"

#include "DeferredMaterialRenderUtils.h"
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
	m_iHiddenMeshMask = pDesc->iHiddenMeshMask;
	m_pSkeletonModelCom = pDesc->pSkeletonModel;
	m_pSocketBoneName = pDesc->pSocketBoneName;
	m_fSocketYawDegrees = pDesc->fSocketYawDegrees;
	m_pSocketRootMatrix = pDesc->pSocketRootMatrix;
	m_strMaterialProfileId = pDesc->strMaterialProfileId;
	m_pEmissiveOverride = pDesc->pEmissiveOverride;

	/* Both kinds need the body's model: a socketed piece reads one bone from it,
	a skinned piece borrows its whole palette. */
	if (nullptr == m_pSkeletonModelCom)
		return E_FAIL;
	if (nullptr != m_pSocketBoneName &&
		!m_pSkeletonModelCom->Has_Bone(m_pSocketBoneName))
	{
		return E_INVALIDARG;
	}

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
	body's pre-transform. A skinned piece adds no bone of its own: its deformation
	lives entirely in the bone palette bound at render time. Either way the piece
	still has to land in the frame the body model is drawn in. */
	if (nullptr != m_pSocketBoneName)
	{
		ChildMatrix =
			XMMatrixRotationY(XMConvertToRadians(m_fSocketYawDegrees)) *
			ChildMatrix;
		ChildMatrix = ChildMatrix * m_pSkeletonModelCom->Get_BoneMatrix(m_pSocketBoneName);
	}

	if (nullptr != m_pSocketRootMatrix)
	{
		ChildMatrix = ChildMatrix * XMLoadFloat4x4(m_pSocketRootMatrix);
	}

	__super::Update_CombinedWorldMatrix(ChildMatrix);
}

void CPart_Equipment::Late_Update(f32_t fTimeDelta)
{
	if (!m_isVisible)
		return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
	if (CGameInstance::Get().Is_ShadowLightEnabled())
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::SHADOW,
			static_pointer_cast<CGameObject>(shared_from_this()));
	}
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
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;

		const DEFERRED_MATERIAL_PROFILE Profile =
			Resolve_DeferredMaterialProfile(
				m_strMaterialProfileId,
				m_pModelCom->Get_MaterialName(i));
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, Profile,
				m_pEmissiveOverride)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Equipment::Render_Shadow()
{
	constexpr uint32_t ANIMATED_SHADOW_PASS = 1u;
	constexpr uint32_t STATIC_SHADOW_PASS = 12u;
	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	if (nullptr == m_pSocketBoneName &&
		FAILED(m_pSkeletonModelCom->Bind_BoneMatrices(
			m_pShaderCom, "g_BoneMatrices", 0)))
	{
		return E_FAIL;
	}

	const uint32_t iShadowPass = nullptr == m_pSocketBoneName ?
		ANIMATED_SHADOW_PASS : STATIC_SHADOW_PASS;
	if (nullptr != m_pSocketBoneName)
	{
		const float2_t vUVScale(1.f, 1.f);
		const float2_t vUVOffset(0.f, 0.f);
		const float4_t vColorTint(1.f, 1.f, 1.f, 1.f);
		if (FAILED(m_pShaderCom->Bind_RawValue(
				"g_UVScale", &vUVScale, sizeof(vUVScale))) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_UVOffset", &vUVOffset, sizeof(vUVOffset))) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_ColorTint", &vColorTint, sizeof(vColorTint))))
		{
			return E_FAIL;
		}
	}
	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;

		const DEFERRED_MATERIAL_PROFILE Profile =
			Resolve_DeferredMaterialProfile(
				m_strMaterialProfileId,
				m_pModelCom->Get_MaterialName(i));
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, Profile)) ||
			FAILED(m_pShaderCom->Begin(iShadowPass)) ||
			FAILED(m_pModelCom->Render(i)))
		{
			return E_FAIL;
		}
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
	if (nullptr != m_pSocketBoneName)
	{
		matrix_t World = XMLoadFloat4x4(&m_CombinedWorldMatrix);
		World.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		const matrix_t InverseTranspose =
			XMMatrixTranspose(XMMatrixInverse(nullptr, World));
		float4x4_t Stored{};
		XMStoreFloat4x4(&Stored, InverseTranspose);
		if (FAILED(m_pShaderCom->Bind_Matrix(
			"g_WorldInvTransposeMatrix", &Stored)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Equipment::Bind_ShadowShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	return S_OK;
}

unique_ptr<CPart_Equipment> CPart_Equipment::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CPart_Equipment>(new CPart_Equipment(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][PartEquipment] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CPart_Equipment::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CPart_Equipment>(new CPart_Equipment(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][PartEquipment] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
