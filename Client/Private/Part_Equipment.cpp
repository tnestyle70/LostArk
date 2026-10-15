#include "Part_Equipment.h"
// 비소유 캐릭터의 월드 표현 숨김 상태를 읽는다.
#include "Character.h"
// surface family와 sourceCharacter program을 검사한다.
#include "BinaryAsset/ModelAssetData.h"
// 장비 재질의 반투명·양면 mask 분류를 재사용한다.
#include "SourceEquipmentMaterialPrograms.h"

// 프로필별 재질 입력·masked hair·hover 표시를 기존 경로로 그린다.
#include "DeferredMaterialRenderUtils.h"
// 렌더 그룹 등록과 카메라·그림자 행렬을 사용한다.
#include "GameInstance.h"
// source character용 forward 조명 상수를 전달한다.
#include "MapAssetRenderUtils.h"
// 소켓 offset 유한성·범위를 검사한다.
#include <cmath>
// 재질 pass와 mesh index의 고정 크기 정수 타입을 사용한다.
#include <cstdint>

namespace
{
	constexpr uint32_t SOURCE_TRANSLUCENT_TWO_SIDED_PASS = 9u;

	// SOURCE_CHARACTER의 지정 program과 장비 반투명 program을 forward pass로 분류한다.
	uint32_t Resolve_TranslucentSourcePass(const Engine::MODEL_SURFACE_PARAMETERS* surface)
	{
		if (nullptr == surface || surface->family != Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
			return 0u;
		const uint32_t program = surface->sourceCharacter.program;
		return 6u == program || 7u == program || 18u == program || 99u == program || SourceEquipmentMaterial::Is_Translucent(program) ?
			SOURCE_TRANSLUCENT_TWO_SIDED_PASS : 0u;
	}


}

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

// desc 값과 차용 포인터를 저장한 뒤 소켓·Component를 검사한다. 실패한 복제본은 Clone에서 정리한다.
HRESULT CPart_Equipment::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<PART_EQUIPMENT_DESC*>(pArg);
	m_iHiddenMeshMask = pDesc->iHiddenMeshMask;
    m_isWeaponPart = pDesc->isWeaponPart;
    m_isIdentityPart = pDesc->isIdentityPart;
	m_pSkeletonModelCom = pDesc->pSkeletonModel;
	m_strSocketBoneName = nullptr != pDesc->pSocketBoneName ?
		pDesc->pSocketBoneName : "";
	m_fSocketYawDegrees = pDesc->fSocketYawDegrees;
	m_pSocketRootMatrix = pDesc->pSocketRootMatrix;
	m_strMaterialProfileId = pDesc->strMaterialProfileId;
	m_pEmissiveOverride = pDesc->pEmissiveOverride;

	// 소켓은 본 하나, 스키닝은 현재 palette를 읽으므로 두 방식 모두 본체 Model이 필요하다.
	if (nullptr == m_pSkeletonModelCom)
		return E_FAIL;
	if (!m_strSocketBoneName.empty() &&
		!m_pSkeletonModelCom->Has_Bone(m_strSocketBoneName.c_str()))
	{
		return E_INVALIDARG;
	}

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	// 본체에 없는 장비 전용 본이 하나라도 있으면 자체 palette를 사용하는 방식으로 전환한다.
	if (m_strSocketBoneName.empty() && nullptr != m_pModelCom)
	{
		for (const string& strBone : m_pModelCom->Get_BoneNames())
		{
			if (m_pSkeletonModelCom->Has_Bone(strBone.c_str()))
				continue;
			// 이름이 _sk로 끝나는 mesh root는 추가 변형 본 검사에서 제외한다.
			if (strBone.size() > 3 && strBone.compare(strBone.size() - 3, 3, "_sk") == 0)
				continue;
			m_hasOwnBones = true;
			break;
		}
	}

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
		m_hasTranslucentMeshes |= 0u != Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));

	return S_OK;
}

void CPart_Equipment::Priority_Update(f32_t fTimeDelta)
{
}

bool_t CPart_Equipment::Set_SocketTransform(const float3_t& positionMeters, const float3_t& rotationDegrees)
{
	const auto finiteBounded = [](const float3_t& value, const float bound) {
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
			std::abs(value.x) <= bound && std::abs(value.y) <= bound && std::abs(value.z) <= bound;
	};
	if (m_strSocketBoneName.empty() || !finiteBounded(positionMeters, 10.f) ||
		!finiteBounded(rotationDegrees, 360.f)) return false;
	m_vSocketPositionMeters = positionMeters;
	m_vSocketRotationDegrees = rotationDegrees;
	return true;
}

// 장비 local→선택적 소켓 본→선택적 본체 local→부모 순으로 합성한다. 본체가 먼저 갱신되어야 한다.
void CPart_Equipment::Update(f32_t fTimeDelta)
{
	matrix_t ChildMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	// 소켓 장비만 본 행렬을 곱한다. 스키닝 변형은 Render의 palette가 맡고 이 함수는 최종 배치 행렬을 만든다.
	if (!m_strSocketBoneName.empty())
	{
		ChildMatrix =
			XMMatrixRotationY(XMConvertToRadians(m_fSocketYawDegrees)) *
			ChildMatrix;
		const matrix_t socket = m_pSkeletonModelCom->Get_BoneMatrix(m_strSocketBoneName.c_str());
		if (m_vSocketRotationDegrees.x != 0.f || m_vSocketRotationDegrees.y != 0.f || m_vSocketRotationDegrees.z != 0.f)
			ChildMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_vSocketRotationDegrees.x),
				XMConvertToRadians(m_vSocketRotationDegrees.y), XMConvertToRadians(m_vSocketRotationDegrees.z)) * ChildMatrix;
		ChildMatrix = ChildMatrix * socket;
		// 본 행렬에는 model preScale이 이미 포함되어 있으므로 meter 이동에는 정규화한 축을 쓴다.
		// 축 길이가 유효하지 않으면 그 프레임의 추가 위치 offset은 적용하지 않는다.
		if (m_vSocketPositionMeters.x != 0.f || m_vSocketPositionMeters.y != 0.f || m_vSocketPositionMeters.z != 0.f)
		{
			matrix_t axes = socket;
			bool_t valid = true;
			for (size_t i = 0u; i < 3u; ++i)
			{
				const float length = XMVectorGetX(XMVector3Length(axes.r[i]));
				if (!std::isfinite(length) || length <= 0.00000001f) { valid = false; break; }
				axes.r[i] = XMVectorSetW(axes.r[i] / length, 0.f);
			}
			if (valid) ChildMatrix.r[3] += XMVector3TransformNormal(XMLoadFloat3(&m_vSocketPositionMeters), axes);
		}
	}

	if (nullptr != m_pSocketRootMatrix)
	{
		ChildMatrix = ChildMatrix * XMLoadFloat4x4(m_pSocketRootMatrix);
	}

	__super::Update_CombinedWorldMatrix(ChildMatrix);
}

bool_t CPart_Equipment::Is_CharacterPresentationHidden() const
{
	const auto owner = m_pCharacterPresentationOwner.lock();
	return owner && owner->Is_WorldPresentationHidden();
}

void CPart_Equipment::Late_Update(f32_t fTimeDelta)
{
	if (!Is_Visible())
		return;
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

HRESULT CPart_Equipment::Render()
{
	return Render_Pass(0u, 0u);
}

HRESULT CPart_Equipment::Render_Group(RENDERGROUP group)
{
	return RENDERGROUP::BLEND == group ? Render_Translucent() : Render();
}

// 현재 palette와 forward 조명을 바인딩해 반투명으로 분류된 mesh만 그린다.
HRESULT CPart_Equipment::Render_Translucent(ID3D11BlendState* pCoverageBlend)
{
	if (!Is_Visible())
		return S_OK;
	if (FAILED(Bind_ShaderResources()) ||
		FAILED(CMapAssetRenderUtils::Bind_SourceCharacterForwardLights(m_pShaderCom)))
		return E_FAIL;

	if (m_strSocketBoneName.empty() && !m_hasOwnBones &&
		FAILED(m_pSkeletonModelCom->Bind_BoneMatrices(
			m_pShaderCom, "g_BoneMatrices", 0)))
		return E_FAIL;
	if (m_strSocketBoneName.empty() && m_hasOwnBones)
		m_pModelCom->Pose_BonesFrom(*m_pSkeletonModelCom);

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;
		const uint32_t pass = Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));
		if (0u == pass)
			continue;
		if (m_strSocketBoneName.empty() && m_hasOwnBones &&
			FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		const DEFERRED_MATERIAL_PROFILE Profile =
			Resolve_DeferredMaterialProfile(
				m_strMaterialProfileId,
				m_pModelCom->Get_MaterialName(i));
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, Profile,
				m_pEmissiveOverride)) ||
			FAILED(m_pModelCom->Bind_SourceCharacterForwardLight(m_pShaderCom, i)) ||
			FAILED(m_pShaderCom->Begin(m_strSocketBoneName.empty() ? pass : 24u)))
			return E_FAIL;
		if (pCoverageBlend)
			m_pContext->OMSetBlendState(pCoverageBlend, nullptr, 0xffffffffu);
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Equipment::Render_Pass(
	uint32_t iSkinnedPassIndex, uint32_t iSocketedPassIndex)
{
    if (!Is_Visible()) return S_OK;
	const uint32_t iPassIndex =
		m_strSocketBoneName.empty() ? iSkinnedPassIndex : iSocketedPassIndex;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	// 추가 본이 없으면 본체 palette를 한 번 바인딩한다. 추가 본이 있으면 자체 자세를 동기화하고 mesh별로 바인딩한다.
	const bool_t isSkinned = m_strSocketBoneName.empty();
	if (isSkinned && !m_hasOwnBones &&
		FAILED(m_pSkeletonModelCom->Bind_BoneMatrices(
			m_pShaderCom, "g_BoneMatrices", 0)))
		return E_FAIL;
	if (isSkinned && m_hasOwnBones)
		m_pModelCom->Pose_BonesFrom(*m_pSkeletonModelCom);

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;
		if (isSkinned && m_hasOwnBones &&
			FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		uint32_t materialPass = iPassIndex;
		const auto* surface = m_pModelCom->Get_MaterialSurface(i);
		// 기본 pass의 반투명 스키닝 mesh는 masked 부분을 그린다. 반투명 색상은 별도 BLEND 단계가 맡는다.
		if (iPassIndex == 0u && 0u != Resolve_TranslucentSourcePass(surface))
		{
			if (!isSkinned) continue;
			const DEFERRED_MATERIAL_PROFILE MaskedProfile =
				Resolve_DeferredMaterialProfile(
					m_strMaterialProfileId, m_pModelCom->Get_MaterialName(i));
			const HRESULT masked = Render_SourceHairMaskedMesh(
				*m_pModelCom, m_pShaderCom, i, MaskedProfile, m_pEmissiveOverride);
			if (FAILED(masked)) return masked;
			continue;
		}
		if (m_strSocketBoneName.empty() && iPassIndex == 0u && surface &&
			surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
			(surface->sourceCharacter.program == 6u || surface->sourceCharacter.program == 7u ||
             surface->sourceCharacter.program == 19u ||
             surface->sourceCharacter.program == 20u || SourceEquipmentMaterial::Is_TwoSidedMasked(surface->sourceCharacter.program)))
		{
			materialPass = 6u;
		}

		const DEFERRED_MATERIAL_PROFILE Profile =
			Resolve_DeferredMaterialProfile(
				m_strMaterialProfileId,
				m_pModelCom->Get_MaterialName(i));
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, Profile,
				m_pEmissiveOverride)) ||
			FAILED(m_pShaderCom->Begin(materialPass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
		if (iPassIndex == 0u)
			(void)Render_CombatHoverMesh(*m_pModelCom, m_pShaderCom, i, m_pEmissiveOverride, isSkinned);
	}
	return S_OK;
}

// 같은 본체/자체 palette와 숨김 mask를 사용해 스키닝 또는 static 그림자 pass로 그린다.
HRESULT CPart_Equipment::Render_Shadow()
{
    if (!Is_Visible()) return S_OK;
	constexpr uint32_t ANIMATED_SHADOW_PASS = 1u;
	constexpr uint32_t STATIC_SHADOW_PASS = 12u;
	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	if (m_strSocketBoneName.empty())
	{
		if (m_hasOwnBones)
			m_pModelCom->Pose_BonesFrom(*m_pSkeletonModelCom);
		else if (FAILED(m_pSkeletonModelCom->Bind_BoneMatrices(
			m_pShaderCom, "g_BoneMatrices", 0)))
		{
			return E_FAIL;
		}
	}

	const uint32_t iShadowPass = m_strSocketBoneName.empty() ?
		ANIMATED_SHADOW_PASS : STATIC_SHADOW_PASS;
	if (!m_strSocketBoneName.empty())
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

		if (m_strSocketBoneName.empty() && m_hasOwnBones &&
			FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
		{
			return E_FAIL;
		}
		const DEFERRED_MATERIAL_PROFILE Profile =
			Resolve_DeferredMaterialProfile(
				m_strMaterialProfileId,
				m_pModelCom->Get_MaterialName(i));
		// 소켓의 static 셰이더는 재질 분기를 유지하고 스키닝 그림자는 diffuse alpha를 사용한다.
		const HRESULT materialResult = m_strSocketBoneName.empty() ?
			m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture",
				i, aiTextureType_DIFFUSE, 0) :
			Bind_DeferredMaterialInputs(*m_pModelCom, m_pShaderCom, i, Profile);
		if (FAILED(materialResult) ||
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

bool CPart_Equipment::Get_AfterimageView(CSkeletalAfterimage::MODEL_VIEW& view)
{
    if (!Is_Visible() || !m_pModelCom || !m_pShaderCom) return false;
    if (m_hasOwnBones && m_strSocketBoneName.empty()) m_pModelCom->Pose_BonesFrom(*m_pSkeletonModelCom);
    // Update 이후 도구의 자세 변경도 반영하도록 소켓·root 합성을 다시 계산한다.
    Update(0.f);
    view.model = m_pModelCom; view.shader = m_pShaderCom; view.world = m_CombinedWorldMatrix;
    view.socketed = !m_strSocketBoneName.empty(); view.hiddenMeshMask = m_iHiddenMeshMask;
    view.paletteModel = m_hasOwnBones ? m_pModelCom : m_pSkeletonModelCom;
    return true;
}

// 합성 World·카메라 행렬을 전달하고 소켓 static mesh에는 법선용 역전치 행렬을 추가한다.
HRESULT CPart_Equipment::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	if (!m_strSocketBoneName.empty())
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

// World와 그림자 광원의 View/Projection만 바인딩한다.
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
