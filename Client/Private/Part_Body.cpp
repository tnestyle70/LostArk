#include "Part_Body.h"
// weak 소유자의 월드 표시 숨김 상태를 읽는다.
#include "Character.h"
// mesh별 surface family와 sourceCharacter program을 확인한다.
#include "BinaryAsset/ModelAssetData.h"
// 장비 재질의 반투명·양면 mask program을 분류한다.
#include "SourceEquipmentMaterialPrograms.h"

// mesh별 재질 입력과 차용 Emissive override를 바인딩한다.
#include "DeferredMaterialRenderUtils.h"
// Component 복제의 기반 타입과 렌더 그룹·카메라·그림자 서비스를 사용한다.
#include "GameInstance.h"
// source character의 forward 조명 입력을 바인딩한다.
#include "MapAssetRenderUtils.h"
// 사이던 모자 준비·렌더와 망치 표현 억제 여부를 재사용한다.
#include "NpcPresentationAssetService.h"

namespace
{
	constexpr uint32_t SOURCE_TRANSLUCENT_TWO_SIDED_PASS = 9u;
	constexpr uint32_t SOURCE_GHOST_OPAQUE_PASS = 16u;

	// SOURCE_CHARACTER의 지정 program과 장비 반투명 program을 forward pass로 분류한다.
	// 조명 뒤 blend로 표시해 deferred dither의 점무늬를 피한다.
	uint32_t Resolve_ForwardSourcePass(const Engine::MODEL_SURFACE_PARAMETERS* surface)
	{
		if (nullptr == surface || surface->family != Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
			return 0u;
		const uint32_t program = surface->sourceCharacter.program;
		// program 84의 ghost는 별도 불투명 forward pass를 사용한다.
		if (84u == program)
			return SOURCE_GHOST_OPAQUE_PASS;
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

// descriptor의 차용 상태를 저장한 뒤 기반·Component를 준비한다. 실패한 Clone은 호출 factory에서 폐기한다.
HRESULT CPart_Body::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<PART_BODY_DESC*>(pArg);
	m_iHiddenMeshMask = pDesc->iHiddenMeshMask;
	m_pEmissiveOverride = pDesc->pEmissiveOverride;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(pDesc)))
		return E_FAIL;
	// 모자 준비 실패는 진단만 남기고 본체 초기화는 계속한다.
	if (FAILED(CNpcPresentationAssetService::Prepare_SaydonHat(m_pDevice, m_pContext, m_pModelCom, m_pSaydonHatModel)))
		OutputDebugStringA("[SaydonHat] Preview head prop unavailable; body preserved.\n");

	// 지정한 초기 clip이 없거나 적용에 실패하면 0번 clip 재생을 시도한다.
	if (nullptr == pDesc->pInitialAnimation ||
		!m_pModelCom->Set_Animation(pDesc->pInitialAnimation, true))
		m_pModelCom->Set_Animation(0u, true);

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
    {
        const uint32_t pass = Resolve_ForwardSourcePass(m_pModelCom->Get_MaterialSurface(i));
        m_hasOpaqueGhostMeshes |= pass == SOURCE_GHOST_OPAQUE_PASS;
        m_hasTranslucentMeshes |= pass != 0u && pass != SOURCE_GHOST_OPAQUE_PASS;
    }

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
	// 본체가 animation clock을 진행한다. 소켓 장비가 이 프레임 본을 읽으므로 소유자의 본체 선행 갱신이 필요하다.
	m_pModelCom->Update_Animation(fTimeDelta);

	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

bool_t CPart_Body::Is_CharacterPresentationHidden() const
{
	const auto owner = m_pCharacterPresentationOwner.lock();
	return owner && owner->Is_WorldPresentationHidden();
}

void CPart_Body::Late_Update(f32_t fTimeDelta)
{
	if (Is_CharacterPresentationHidden()) return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
    if (m_hasOpaqueGhostMeshes)
        CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONLIGHT,
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
	if (RENDERGROUP::NONLIGHT == group) return Render_ForwardSource(true);
    return RENDERGROUP::BLEND == group ? Render_ForwardSource(false) : Render();
}

// forward 재질 중 ghost/반투명 분류에 맞는 mesh만 그린다. bind/draw 실패는 E_FAIL이다.
HRESULT CPart_Body::Render_ForwardSource(bool opaqueGhost, ID3D11BlendState* pCoverageBlend)
{
	if (Is_CharacterPresentationHidden()) return S_OK;
	if (Client::CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_WeaponReplacementBody.lock())) return S_OK;
	if (FAILED(Bind_ShaderResources()) ||
		FAILED(CMapAssetRenderUtils::Bind_SourceCharacterForwardLights(m_pShaderCom)))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;
		const uint32_t pass = Resolve_ForwardSourcePass(m_pModelCom->Get_MaterialSurface(i));
		if (0u == pass || (pass == SOURCE_GHOST_OPAQUE_PASS) != opaqueGhost)
			continue;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {},
				m_pEmissiveOverride)) ||
			FAILED(m_pModelCom->Bind_SourceCharacterForwardLight(m_pShaderCom, i)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(pass)))
			return E_FAIL;
		if (pCoverageBlend && !opaqueGhost)
			m_pContext->OMSetBlendState(pCoverageBlend, nullptr, 0xffffffffu);
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

// 숨김 mask와 재질별 pass를 적용해 본체를 그리고 선택적 모자를 같은 본체 기준으로 그린다.
HRESULT CPart_Body::Render_Pass(uint32_t iPassIndex)
{
	if (Is_CharacterPresentationHidden()) return S_OK;
	if (Client::CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_WeaponReplacementBody.lock())) return S_OK;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;

        uint32_t materialPass = iPassIndex;
        const auto* surface = m_pModelCom->Get_MaterialSurface(i);
        // 기본 pass에서 forward 재질을 건너뛰어 NONLIGHT/BLEND 단계와 중복하지 않는다.
        if (iPassIndex == 0u && 0u != Resolve_ForwardSourcePass(surface))
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

// 같은 숨김 mask와 현재 본 palette를 사용하되 그림자 광원의 행렬로 그린다.
HRESULT CPart_Body::Render_Shadow()
{
	if (Is_CharacterPresentationHidden()) return S_OK;
	if (Client::CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_WeaponReplacementBody.lock())) return S_OK;
	constexpr uint32_t ANIMATED_SHADOW_PASS = 1u;
	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;

		// ghost는 고형 그림자 pass, 나머지는 diffuse alpha를 사용하는 스키닝 그림자 pass다.
		if (FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(Resolve_ForwardSourcePass(m_pModelCom->Get_MaterialSurface(i)) ==
                SOURCE_GHOST_OPAQUE_PASS ? 17u : ANIMATED_SHADOW_PASS)) ||
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

// Update에서 저장한 합성 World와 현재 카메라 View/Projection을 전달한다.
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

// 합성 World는 유지하고 View/Projection은 그림자 광원에서 가져온다.
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
