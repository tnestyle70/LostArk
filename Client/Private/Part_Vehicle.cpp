#include "Part_Vehicle.h"
#include "BinaryAsset/ModelAssetData.h"

#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "MapAssetRenderUtils.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr uint32_t SOURCE_TRANSLUCENT_TWO_SIDED_PASS = 9u;
	constexpr uint32_t SOURCE_TRANSLUCENT_ONE_SIDED_PASS = 10u;
	constexpr f32_t LOCOMOTION_BLEND_SECONDS = 0.12f;
	constexpr const char_t* ROOT_MOTION_BONE = "b_root";
	constexpr int32_t ROOT_MOTION_VERTICAL_AXIS = 2;

	uint32_t Resolve_TranslucentSourcePass(const Engine::MODEL_SURFACE_PARAMETERS* surface)
	{
		if (nullptr == surface || surface->family != Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
			return 0u;
		switch (surface->sourceCharacter.program)
		{
		case 18u: return SOURCE_TRANSLUCENT_TWO_SIDED_PASS;
		case 88u: return SOURCE_TRANSLUCENT_ONE_SIDED_PASS;
		default: return 0u;
		}
	}
}

CPart_Vehicle::CPart_Vehicle(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CPart_Vehicle::~CPart_Vehicle()
{
}

HRESULT CPart_Vehicle::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPart_Vehicle::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<PART_VEHICLE_DESC*>(pArg);
	m_strIdleClip = pDesc->strIdleClip;
	m_strRunClip = pDesc->strRunClip;
	m_strSeatBone = pDesc->strSeatBone;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(pDesc)) ||
		!m_pModelCom->Has_Bone(m_strSeatBone.c_str()) ||
		!m_pModelCom->Set_Animation(m_strIdleClip.c_str(), true))
	{
		return E_FAIL;
	}
	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
		m_hasTranslucentMeshes |= 0u != Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));
	if (m_pModelCom->Has_Bone(ROOT_MOTION_BONE))
		(void)m_pModelCom->Enable_RootMotionSuppression(ROOT_MOTION_BONE, ROOT_MOTION_VERTICAL_AXIS);
	return S_OK;
}

bool_t CPart_Vehicle::Set_Moving(const bool_t isMoving)
{
	if (nullptr == m_pModelCom)
		return false;
	if (m_isMoving == isMoving)
		return true;
	m_isMoving = isMoving;
	if (m_isPlayingSkill)
		return true;
	return m_pModelCom->Set_Animation(
		(isMoving ? m_strRunClip : m_strIdleClip).c_str(), true, LOCOMOTION_BLEND_SECONDS);
}

bool_t CPart_Vehicle::Seek_SkillChain(
	const std::vector<std::string>& clips,
	const f32_t actionAgeSeconds)
{
	if (nullptr == m_pModelCom || clips.empty() ||
		!std::isfinite(actionAgeSeconds) || actionAgeSeconds < 0.f)
	{
		return false;
	}
	f32_t remaining = actionAgeSeconds;
	for (std::size_t step = 0u; step < clips.size(); ++step)
	{
		uint32_t animation = UINT32_MAX;
		for (uint32_t index = 0u; index < m_pModelCom->Get_NumAnimations(); ++index)
		{
			const char_t* pName = m_pModelCom->Get_AnimationName(index);
			if (nullptr != pName && clips[step] == pName)
			{
				animation = index;
				break;
			}
		}
		f32_t position = 0.f;
		f32_t duration = 0.f;
		const f32_t ticksPerSecond = UINT32_MAX == animation ? 0.f :
			m_pModelCom->Get_AnimationTickPerSecond(animation);
		if (UINT32_MAX == animation ||
			!m_pModelCom->Get_AnimationProgress(animation, position, duration) ||
			!std::isfinite(duration) || duration <= 0.f ||
			!std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f)
		{
			return false;
		}
		const f32_t seconds = duration / ticksPerSecond;
		if (remaining < seconds || step + 1u == clips.size())
		{
			if (!m_isPlayingSkill || m_pModelCom->Get_CurrentAnimIndex() != animation)
			{
				if (!m_pModelCom->Set_Animation(clips[step].c_str(), false, LOCOMOTION_BLEND_SECONDS))
					return false;
			}
			m_isPlayingSkill = true;
			const f32_t local = (std::min)(remaining, (std::max)(0.f, seconds - 0.0001f));
			return m_pModelCom->Set_AnimTrackPosition(animation, local * ticksPerSecond);
		}
		remaining -= seconds;
	}
	return false;
}

bool_t CPart_Vehicle::Try_Get_SkillClipWindow(
	const std::vector<std::string>& clips,
	const std::size_t clipIndex,
	f32_t& outStartSeconds,
	f32_t& outDurationSeconds) const
{
	if (nullptr == m_pModelCom || clipIndex >= clips.size())
		return false;
	f32_t start = 0.f;
	for (std::size_t step = 0u; step <= clipIndex; ++step)
	{
		uint32_t animation = UINT32_MAX;
		for (uint32_t index = 0u; index < m_pModelCom->Get_NumAnimations(); ++index)
		{
			const char_t* pName = m_pModelCom->Get_AnimationName(index);
			if (nullptr != pName && clips[step] == pName)
			{
				animation = index;
				break;
			}
		}
		f32_t position = 0.f;
		f32_t duration = 0.f;
		const f32_t ticksPerSecond = UINT32_MAX == animation ? 0.f :
			m_pModelCom->Get_AnimationTickPerSecond(animation);
		if (UINT32_MAX == animation ||
			!m_pModelCom->Get_AnimationProgress(animation, position, duration) ||
			!std::isfinite(duration) || duration <= 0.f ||
			!std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f)
		{
			return false;
		}
		if (step == clipIndex)
		{
			outStartSeconds = start;
			outDurationSeconds = duration / ticksPerSecond;
			return true;
		}
		start += duration / ticksPerSecond;
	}
	return false;
}

bool_t CPart_Vehicle::Try_Get_LocomotionClipTime(
	std::string& outClip,
	f32_t& outSeconds,
	f32_t& outDurationSeconds) const
{
	if (nullptr == m_pModelCom || m_isPlayingSkill)
		return false;
	const std::string& clip = m_isMoving ? m_strRunClip : m_strIdleClip;
	const uint32_t animation = m_pModelCom->Get_CurrentAnimIndex();
	const char_t* pName = m_pModelCom->Get_AnimationName(animation);
	/* A locomotion switch blends for a moment; until the new clip owns the
	track its position belongs to the previous one. */
	if (nullptr == pName || clip != pName)
		return false;
	f32_t position = 0.f;
	f32_t duration = 0.f;
	const f32_t ticksPerSecond = m_pModelCom->Get_AnimationTickPerSecond(animation);
	if (!m_pModelCom->Get_AnimationProgress(animation, position, duration) ||
		!std::isfinite(position) || position < 0.f ||
		!std::isfinite(duration) || duration <= 0.f ||
		!std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f)
	{
		return false;
	}
	outClip = clip;
	outSeconds = position / ticksPerSecond;
	outDurationSeconds = duration / ticksPerSecond;
	return true;
}

void CPart_Vehicle::Resume_Locomotion()
{
	if (nullptr == m_pModelCom || !m_isPlayingSkill)
		return;
	m_isPlayingSkill = false;
	(void)m_pModelCom->Set_Animation(
		(m_isMoving ? m_strRunClip : m_strIdleClip).c_str(), true, LOCOMOTION_BLEND_SECONDS);
}

bool_t CPart_Vehicle::Try_Get_SeatWorldPosition(float3_t& outPosition) const
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom ||
		nullptr == m_pParentMatrix || !m_pModelCom->Has_Bone(m_strSeatBone.c_str()))
	{
		return false;
	}
	const matrix_t seat = m_pModelCom->Get_BoneMatrix(m_strSeatBone.c_str()) *
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) *
		XMLoadFloat4x4(m_pParentMatrix);
	float3_t staged{};
	XMStoreFloat3(&staged, seat.r[3]);
	if (!std::isfinite(staged.x) || !std::isfinite(staged.y) || !std::isfinite(staged.z))
		return false;
	outPosition = staged;
	return true;
}

void CPart_Vehicle::Priority_Update(f32_t fTimeDelta)
{
}

void CPart_Vehicle::Update(f32_t fTimeDelta)
{
	m_pModelCom->Update_Animation(fTimeDelta);

	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CPart_Vehicle::Late_Update(f32_t fTimeDelta)
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

HRESULT CPart_Vehicle::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		uint32_t materialPass = 0u;
		const auto* surface = m_pModelCom->Get_MaterialSurface(i);
		if (0u != Resolve_TranslucentSourcePass(surface))
			continue;
		if (surface &&
			surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
			(surface->sourceCharacter.program == 6u || surface->sourceCharacter.program == 7u ||
			 surface->sourceCharacter.program == 19u ||
			 surface->sourceCharacter.program == 20u))
			materialPass = 6u;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, nullptr)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(materialPass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Vehicle::Render_Group(RENDERGROUP group)
{
	return RENDERGROUP::BLEND == group ? Render_Translucent() : Render();
}

HRESULT CPart_Vehicle::Render_Translucent()
{
	if (FAILED(Bind_ShaderResources()) ||
		FAILED(CMapAssetRenderUtils::Bind_SourceCharacterForwardLights(m_pShaderCom)))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		const uint32_t pass = Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));
		if (0u == pass)
			continue;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, nullptr)) ||
			FAILED(m_pModelCom->Bind_SourceCharacterForwardLight(m_pShaderCom, i)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(pass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Vehicle::Render_Shadow()
{
	constexpr uint32_t ANIMATED_SHADOW_PASS = 1u;
	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
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
	return S_OK;
}

HRESULT CPart_Vehicle::Ready_Components(const PART_VEHICLE_DESC* pDesc)
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

HRESULT CPart_Vehicle::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

HRESULT CPart_Vehicle::Bind_ShadowShaderResources()
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

unique_ptr<CPart_Vehicle> CPart_Vehicle::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CPart_Vehicle>(new CPart_Vehicle(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][PartVehicle] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CPart_Vehicle::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CPart_Vehicle>(new CPart_Vehicle(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][PartVehicle] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
