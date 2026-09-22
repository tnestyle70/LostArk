#include "Part_Body.h"
#include "BinaryAsset/ModelAssetData.h"
#include "SourceEquipmentMaterialPrograms.h"

#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "MapAssetRenderUtils.h"
#include "NpcPresentationAssetService.h"

namespace
{
	constexpr uint32_t SOURCE_TRANSLUCENT_TWO_SIDED_PASS = 9u;
	constexpr uint32_t SOURCE_TRANSLUCENT_ONE_SIDED_PASS = 10u;

	/* Programs 7 and 18 are the source two-sided translucent hair and program 6
	the eyelash/eye-AO shell. The deferred two-sided pass resolves their coverage
	as a dither, so they read as stipple dots; the forward pass blends them after
	scene lighting instead. */
	uint32_t Resolve_TranslucentSourcePass(const Engine::MODEL_SURFACE_PARAMETERS* surface)
	{
		if (nullptr == surface || surface->family != Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
			return 0u;
		const uint32_t program = surface->sourceCharacter.program;
		// The generic ghost preview uses the same forward material pass as CBody_Valtan.
		if (84u == program)
			return SOURCE_TRANSLUCENT_ONE_SIDED_PASS;
		return 6u == program || 7u == program || 18u == program || 99u == program || SourceEquipmentMaterial::Is_Translucent(program) ?
			SOURCE_TRANSLUCENT_TWO_SIDED_PASS : 0u;
	}
}

CPart_Body::CPart_Body(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CPart_Body::~CPart_Body()
{
}

HRESULT CPart_Body::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPart_Body::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<PART_BODY_DESC*>(pArg);
	m_iHiddenMeshMask = pDesc->iHiddenMeshMask;
	m_pEmissiveOverride = pDesc->pEmissiveOverride;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(pDesc)))
		return E_FAIL;
	if (FAILED(CNpcPresentationAssetService::Prepare_SaydonHat(m_pDevice, m_pContext, m_pModelCom, m_pSaydonHatModel)))
		OutputDebugStringA("[SaydonHat] Preview head prop unavailable; body preserved.\n");

	/* Fall back to the first clip so a class with a mistyped name still animates
	instead of standing in its bind pose. */
	if (nullptr == pDesc->pInitialAnimation ||
		!m_pModelCom->Set_Animation(pDesc->pInitialAnimation, true))
		m_pModelCom->Set_Animation(0u, true);

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
		m_hasTranslucentMeshes |= 0u != Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));

	return S_OK;
}

bool_t CPart_Body::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == pClipName)
		return false;
	return m_pModelCom->Set_Animation(pClipName, isLoop);
}

bool_t CPart_Body::Try_Get_PresentationRootMatrix(float4x4_t* pOutWorld) const
{
	if (nullptr == pOutWorld || nullptr == m_pTransformCom || nullptr == m_pParentMatrix)
		return false;
	XMStoreFloat4x4(pOutWorld,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * XMLoadFloat4x4(m_pParentMatrix));
	return true;
}

void CPart_Body::Priority_Update(f32_t fTimeDelta)
{
}

void CPart_Body::Update(f32_t fTimeDelta)
{
	/* The body drives the clock every frame; the logic only picks the clip. Parts
	that borrow this palette read it at render time, so they need no ordering. */
	m_pModelCom->Update_Animation(fTimeDelta);

	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CPart_Body::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
	if (m_hasTranslucentMeshes)
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

HRESULT CPart_Body::Render()
{
	return Render_Pass(0u);
}

HRESULT CPart_Body::Render_Group(RENDERGROUP group)
{
	return RENDERGROUP::BLEND == group ? Render_Translucent() : Render();
}

HRESULT CPart_Body::Render_Translucent()
{
	if (Client::CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_WeaponReplacementBody.lock())) return S_OK;
	if (FAILED(Bind_ShaderResources()) ||
		FAILED(CMapAssetRenderUtils::Bind_SourceCharacterForwardLights(m_pShaderCom)))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;
		const uint32_t pass = Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));
		if (0u == pass)
			continue;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {},
				m_pEmissiveOverride)) ||
			FAILED(m_pModelCom->Bind_SourceCharacterForwardLight(m_pShaderCom, i)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(pass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Body::Render_Pass(uint32_t iPassIndex)
{
	if (Client::CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_WeaponReplacementBody.lock())) return S_OK;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;

        uint32_t materialPass = iPassIndex;
        const auto* surface = m_pModelCom->Get_MaterialSurface(i);
        /* The BLEND group draws these forward; the portrait's explicit passes
        still take every mesh so the second draw keeps its own look. */
        if (iPassIndex == 0u && 0u != Resolve_TranslucentSourcePass(surface))
            continue;
        if (iPassIndex == 0u && surface &&
            surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
            (surface->sourceCharacter.program == 19u ||
             surface->sourceCharacter.program == 20u || SourceEquipmentMaterial::Is_TwoSidedMasked(surface->sourceCharacter.program)))
            materialPass = 6u;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {},
				m_pEmissiveOverride)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(materialPass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	float4x4_t bodyWorld;
	if (m_pSaydonHatModel && (!Try_Get_PresentationRootMatrix(&bodyWorld) ||
		FAILED(CNpcPresentationAssetService::Render_SaydonHat(m_pModelCom, m_pSaydonHatModel,
			m_pShaderCom, bodyWorld, iPassIndex)))) return E_FAIL;
	return S_OK;
}

HRESULT CPart_Body::Render_Shadow()
{
	if (Client::CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_WeaponReplacementBody.lock())) return S_OK;
	constexpr uint32_t ANIMATED_SHADOW_PASS = 1u;
	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;

		// Animated shadow consumes diffuse alpha only; keep the exact material override path.
		if (FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(ANIMATED_SHADOW_PASS)) ||
			FAILED(m_pModelCom->Render(i)))
		{
			return E_FAIL;
		}
	}

	float4x4_t bodyWorld;
	if (m_pSaydonHatModel && (!Try_Get_PresentationRootMatrix(&bodyWorld) ||
		FAILED(CNpcPresentationAssetService::Render_SaydonHat(m_pModelCom, m_pSaydonHatModel,
			m_pShaderCom, bodyWorld, ANIMATED_SHADOW_PASS, false, true)))) return E_FAIL;
	return S_OK;
}

HRESULT CPart_Body::Ready_Components(const PART_BODY_DESC* pDesc)
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

HRESULT CPart_Body::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

HRESULT CPart_Body::Bind_ShadowShaderResources()
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

unique_ptr<CPart_Body> CPart_Body::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CPart_Body>(new CPart_Body(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][PartBody] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CPart_Body::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CPart_Body>(new CPart_Body(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][PartBody] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
