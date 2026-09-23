#include "Body_Valtan.h"

#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "MapAssetRenderUtils.h"
#include "Valtan.h"

namespace
{
	bool_t Is_SourceGhostSurface(const Engine::MODEL_SURFACE_PARAMETERS* surface)
	{
		return nullptr != surface &&
			surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
			surface->sourceCharacter.program == 84u;
	}
}

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
	m_strModelPrototypeTag = pDesc->strModelPrototypeTag;
	if (m_strModelPrototypeTag.empty())
		return E_INVALIDARG;
	m_pEmissiveOverride = pDesc->pEmissiveOverride;
    m_pChargeAfterimageEnabled = pDesc->pChargeAfterimageEnabled;
	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
		m_hasOpaqueGhostMeshes |= Is_SourceGhostSurface(m_pModelCom->Get_MaterialSurface(i));

	/* 발탄 원본 모델의 전방축을 Engine의 LOOK(+Z) 기준에 맞춘다. */
	m_pTransformCom->Rotation(0.f, -90.f, 0.f);

	if (!m_pModelCom->Set_Animation("mesh_idle_battle_1", true))
		m_pModelCom->Set_Animation(0u, true);
	return S_OK;
}

void CBody_Valtan::Priority_Update(f32_t fTimeDelta)
{
}

void CBody_Valtan::Update(f32_t fTimeDelta)
{
	// A server death owns a finite, non-loop clip; keep advancing its pose.
	if (nullptr != m_pModelCom)
		m_pModelCom->Update_Animation(fTimeDelta);

	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CBody_Valtan::Late_Update(f32_t fTimeDelta)
{
    m_ChargeAfterimage.Update(fTimeDelta, m_pChargeAfterimageEnabled && *m_pChargeAfterimageEnabled,
        m_pModelCom, m_CombinedWorldMatrix);
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
	// Forward opaque color belongs in SceneHDR before the sorted translucent queue.
    if (m_hasOpaqueGhostMeshes)
        CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONLIGHT,
            static_pointer_cast<CGameObject>(shared_from_this()));
	if (m_ChargeAfterimage.Has_Samples())
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::BLEND,
			static_pointer_cast<CGameObject>(shared_from_this()));
	}
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
		if (Is_SourceGhostSurface(m_pModelCom->Get_MaterialSurface(i)))
			continue;
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

HRESULT CBody_Valtan::Render_Group(RENDERGROUP group)
{
    if (RENDERGROUP::NONLIGHT == group) return Render_OpaqueGhost();
    if (RENDERGROUP::BLEND != group) return Render();
    m_ChargeAfterimage.Render(m_pModelCom, m_pShaderCom, m_CombinedWorldMatrix);
    return S_OK;
}

std::string CBody_Valtan::Get_RenderDiagnostic() const
{
    std::string status = m_hasOpaqueGhostMeshes ? "NONLIGHT opaque pass 16" : "NONBLEND pass 0";
    status += " | submitted opaque ghost meshes=" + std::to_string(m_iOpaqueGhostDrawCount);
    if (m_pModelCom)
        for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
        {
            const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
            status += " | " + m_pModelCom->Get_MaterialName(mesh) + ":program=" +
                std::to_string(surface ? surface->sourceCharacter.program : 0u);
        }
    if (!m_strOpaqueGhostRenderFailure.empty()) status += " | " + m_strOpaqueGhostRenderFailure;
    return status;
}

HRESULT CBody_Valtan::Render_OpaqueGhost()
{
    constexpr uint32_t SOURCE_GHOST_OPAQUE_PASS = 16u;
    m_iOpaqueGhostDrawCount = 0u;
    const auto checked = [this](HRESULT result, const std::string& stage)
    {
        if (FAILED(result))
            m_strOpaqueGhostRenderFailure = stage + " failed, HRESULT=" +
                std::to_string(static_cast<int32_t>(result));
        return result;
    };
    HRESULT result = checked(Bind_ShaderResources(), "world/view/projection binding");
    if (FAILED(result)) return result;
    result = checked(CMapAssetRenderUtils::Bind_SourceCharacterForwardLights(m_pShaderCom),
        "scene forward light/fog binding");
    if (FAILED(result)) return result;

    for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
    {
        if (!Is_SourceGhostSurface(m_pModelCom->Get_MaterialSurface(i))) continue;
        const std::string label = " mesh=" + std::to_string(i);
        const DEFERRED_MATERIAL_PROFILE profile = Resolve_DeferredMaterialProfile(
            "material.valtan.monster-base.v1", m_pModelCom->Get_MaterialName(i));
        result = checked(Bind_DeferredMaterialInputs(*m_pModelCom, m_pShaderCom, i,
            profile, m_pEmissiveOverride), "source base material" + label);
        if (FAILED(result)) return result;
        result = checked(m_pModelCom->Bind_SourceCharacterForwardLight(m_pShaderCom, i),
            "source light material" + label);
        if (FAILED(result)) return result;
        result = checked(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i),
            "bone palette" + label);
        if (FAILED(result)) return result;
        result = checked(m_pShaderCom->Begin(SOURCE_GHOST_OPAQUE_PASS),
            "native84 opaque variant/pass16" + label);
        if (FAILED(result)) return result;
        result = checked(m_pModelCom->Render(i), "mesh submission" + label);
        if (FAILED(result)) return result;
        ++m_iOpaqueGhostDrawCount;
    }
    m_strOpaqueGhostRenderFailure.clear();
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
			FAILED(m_pShaderCom->Begin(Is_SourceGhostSurface(m_pModelCom->Get_MaterialSurface(i)) ?
                17u : ANIMATED_SHADOW_PASS)) ||
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
		m_strModelPrototypeTag,
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
