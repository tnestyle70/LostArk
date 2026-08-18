#include "Body_Valtan.h"

#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "Valtan.h"

CBody_Valtan::CBody_Valtan(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CBody_Valtan::~CBody_Valtan()
{
}

HRESULT CBody_Valtan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBody_Valtan::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<BODY_VALTAN_DESC*>(pArg);
	m_pParentState = pDesc->pParentState;
	m_iPrototypeLevelIndex = pDesc->iPrototypeLevelIndex;
	m_pEmissiveOverride = pDesc->pEmissiveOverride;
	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components()))
		return E_FAIL;

	/* 발탄 원본 모델의 전방축을 Engine의 LOOK(+Z) 기준에 맞춘다. */
	m_pTransformCom->Rotation(0.f, -90.f, 0.f);

	if (!m_pModelCom->Set_Animation("idle_battle_1", true))
		m_pModelCom->Set_Animation(0u, true);
	return S_OK;
}

void CBody_Valtan::Priority_Update(f32_t fTimeDelta)
{
}

void CBody_Valtan::Update(f32_t fTimeDelta)
{
	if (nullptr != m_pParentState &&
		*m_pParentState != CValtan::VALTAN_STATE::DEAD)
		m_pModelCom->Update_Animation(fTimeDelta);

	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CBody_Valtan::Late_Update(f32_t fTimeDelta)
{
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

HRESULT CBody_Valtan::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		const DEFERRED_MATERIAL_PROFILE Profile =
			Resolve_DeferredMaterialProfile(
				"material.valtan.monster-base.v1",
				m_pModelCom->Get_MaterialName(i));
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, Profile,
				m_pEmissiveOverride)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CBody_Valtan::Render_Shadow()
{
	constexpr uint32_t ANIMATED_SHADOW_PASS = 1u;
	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		const DEFERRED_MATERIAL_PROFILE Profile =
			Resolve_DeferredMaterialProfile(
				"material.valtan.monster-base.v1",
				m_pModelCom->Get_MaterialName(i));
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, Profile)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(ANIMATED_SHADOW_PASS)) ||
			FAILED(m_pModelCom->Render(i)))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CBody_Valtan::Ready_Components()
{
	if (FAILED(__super::Add_Component(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		TEXT("Com_Shader"),
		m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_Component_Model_Valtan"),
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBody_Valtan::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

HRESULT CBody_Valtan::Bind_ShadowShaderResources()
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

unique_ptr<CBody_Valtan> CBody_Valtan::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CBody_Valtan>(new CBody_Valtan(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][ValtanBody] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CBody_Valtan::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CBody_Valtan>(new CBody_Valtan(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][ValtanBody] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
