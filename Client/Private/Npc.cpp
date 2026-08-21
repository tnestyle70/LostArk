#include "Npc.h"

#include "ActorCatalog.h"
#include "Collider.h"
#include "DeferredMaterialRenderUtils.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Transform.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <span>

namespace
{
	constexpr f32_t HIT_FLASH_DURATION_SECONDS = 0.12f;
	constexpr f32_t HIT_FLASH_PEAK_INTENSITY = 4.f;
}

CNpc::CNpc(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject{ pDevice, pContext }
{
}

CNpc::~CNpc()
{
}

HRESULT CNpc::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNpc::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const NPC_DESC* pDesc = static_cast<const NPC_DESC*>(pArg);
	if (!std::isfinite(pDesc->fCollisionRadius) ||
		pDesc->fCollisionRadius < 0.f)
	{
		return E_INVALIDARG;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_strArchetypeId = pDesc->strArchetypeId;
	m_strAnimationEffectCueAssetId =
		pDesc->strAnimationEffectCueAssetId;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, pDesc->vPosition.z, 1.f));
	if (0.f != pDesc->fYawDegree)
		m_pTransformCom->Rotation(0.f, pDesc->fYawDegree, 0.f);
	if (nullptr != m_pColliderCom)
	{
		m_pColliderCom->Update(XMMatrixTranslationFromVector(
			m_pTransformCom->Get_State(STATE::POSITION)));
	}

	/* With no animation set the bone palette is never filled, so every vertex
	collapses onto the origin and the NPC simply vanishes -- a wrong clip name
	looks exactly like a failed load. Fall back to the model's first clip so a
	bad name is visible as a wrong pose instead of nothing at all. */
	if (nullptr == pDesc->pIdleClip ||
		!m_pModelCom->Set_Animation(pDesc->pIdleClip, pDesc->isLoop))
	{
		if (0 == m_pModelCom->Get_NumAnimations())
			return E_FAIL;
		m_pModelCom->Set_Animation(0u, pDesc->isLoop);
	}

	/* Effect cues are optional presentation. A missing/corrupt document or an
	invalid NPC-specific filter is isolated without rolling back the model and
	its authoritative animation path. */
	(void)Load_EffectCues();

	return S_OK;
}

bool_t CNpc::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == pClipName || nullptr == m_pModelCom)
		return false;
	return m_pModelCom->Set_Animation(pClipName, isLoop);
}

bool_t CNpc::Apply_NetworkState(
	const float3_t& position,
	const f32_t yawDegrees)
{
	if (nullptr == m_pTransformCom ||
		!std::isfinite(position.x) ||
		!std::isfinite(position.y) ||
		!std::isfinite(position.z) ||
		!std::isfinite(yawDegrees))
	{
		return false;
	}
	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSet(position.x, position.y, position.z, 1.f));
	m_pTransformCom->Rotation(0.f, yawDegrees, 0.f);
	if (nullptr != m_pColliderCom)
	{
		m_pColliderCom->Update(XMMatrixTranslationFromVector(
			m_pTransformCom->Get_State(STATE::POSITION)));
	}
	return true;
}

bool_t CNpc::Load_EffectCues()
{
	m_EffectCueDocument = {};
	if (m_strAnimationEffectCueAssetId.empty())
		return true;
	if (nullptr == m_pModelCom || m_strArchetypeId.empty())
		return false;

	std::vector<std::string> AvailableClips;
	AvailableClips.reserve(m_pModelCom->Get_NumAnimations());
	for (uint32_t iAnimation = 0u;
		iAnimation < m_pModelCom->Get_NumAnimations(); ++iAnimation)
	{
		const char_t* pClipName =
			m_pModelCom->Get_AnimationName(iAnimation);
		if (nullptr != pClipName && '\0' != *pClipName)
			AvailableClips.emplace_back(pClipName);
	}

	ANIMATION_EFFECT_CUE_DOCUMENT Staged;
	std::string Status;
	if (!CAnimationEffectCueDocument::Load(
			m_strAnimationEffectCueAssetId,
			AvailableClips,
			Staged,
			Status,
			true))
	{
		OutputDebugStringA((
			"[Client][Npc] animation Effect cues isolated for " +
			m_strArchetypeId + ": " + Status + "\n").c_str());
		return false;
	}

	const NPC_ACTOR_ENTRY* pActor =
		CActorCatalog::Find_Npc(m_strArchetypeId);
	if (nullptr == pActor)
		return false;
	for (const ANIMATION_EFFECT_CUE& Cue : Staged.Cues)
	{
		if ("root" != Cue.strAnchorSlotId &&
			!m_pModelCom->Has_Bone(Cue.strAnchorSlotId.c_str()))
		{
			OutputDebugStringA((
				"[Client][Npc] animation Effect cue anchor rejected for " +
				m_strArchetypeId + ": " + Cue.strAnchorSlotId + "\n").c_str());
			return false;
		}

		uint32_t iMatchingActionClipCount = 0u;
		for (const auto& [ActionId, Clips] : pActor->actionClips)
		{
			(void)ActionId;
			iMatchingActionClipCount += static_cast<uint32_t>(std::count(
				Clips.begin(), Clips.end(), Cue.strClipName));
		}
		if (1u != iMatchingActionClipCount)
		{
			OutputDebugStringA((
				"[Client][Npc] animation Effect cue clip/action join is not unique for " +
				m_strArchetypeId + ": " + Cue.strClipName + "\n").c_str());
			return false;
		}

		uint32_t iAnimationIndex = UINT32_MAX;
		for (uint32_t iAnimation = 0u;
			iAnimation < m_pModelCom->Get_NumAnimations(); ++iAnimation)
		{
			const char_t* pName =
				m_pModelCom->Get_AnimationName(iAnimation);
			if (nullptr != pName && Cue.strClipName == pName)
			{
				iAnimationIndex = iAnimation;
				break;
			}
		}
		if (UINT32_MAX == iAnimationIndex)
			return false;
		f32_t fTrackPosition = 0.f;
		f32_t fTrackDuration = 0.f;
		const f32_t fTicksPerSecond =
			m_pModelCom->Get_AnimationTickPerSecond(iAnimationIndex);
		if (!m_pModelCom->Get_AnimationProgress(
				iAnimationIndex, fTrackPosition, fTrackDuration) ||
			!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
			!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f)
		{
			return false;
		}
		const f32_t fClipDurationMs =
			fTrackDuration * 1000.f / fTicksPerSecond;
		if (static_cast<f32_t>(Cue.iStartMs) > fClipDurationMs + 0.01f ||
			static_cast<f32_t>(Cue.iEndMs) > fClipDurationMs + 0.01f)
		{
			OutputDebugStringA((
				"[Client][Npc] animation Effect cue exceeds its source clip for " +
				m_strArchetypeId + ": " + Cue.strClipName + "\n").c_str());
			return false;
		}
	}

	m_EffectCueDocument = std::move(Staged);
	return true;
}

bool_t CNpc::Build_ActionTimeline(
	const std::string_view actionId,
	std::vector<std::string>& OutClips,
	std::vector<uint32_t>& OutAnimationIndices,
	std::vector<ACTION_PRESENTATION_CLIP_TIMING>& OutTimings) const
{
	OutClips.clear();
	OutAnimationIndices.clear();
	OutTimings.clear();
	if (actionId.empty() || nullptr == m_pModelCom ||
		m_strArchetypeId.empty())
	{
		return false;
	}
	const NPC_ACTOR_ENTRY* pActor =
		CActorCatalog::Find_Npc(m_strArchetypeId);
	if (nullptr == pActor)
		return false;
	const auto Action = pActor->actionClips.find(actionId);
	if (pActor->actionClips.end() == Action || Action->second.empty())
		return false;

	OutClips = Action->second;
	OutAnimationIndices.reserve(OutClips.size());
	OutTimings.reserve(OutClips.size());
	for (const std::string& ClipName : OutClips)
	{
		uint32_t iAnimationIndex = UINT32_MAX;
		for (uint32_t iAnimation = 0u;
			iAnimation < m_pModelCom->Get_NumAnimations(); ++iAnimation)
		{
			const char_t* pName =
				m_pModelCom->Get_AnimationName(iAnimation);
			if (nullptr != pName && ClipName == pName)
			{
				iAnimationIndex = iAnimation;
				break;
			}
		}
		if (UINT32_MAX == iAnimationIndex)
			return false;

		f32_t fTrackPosition = 0.f;
		f32_t fTrackDuration = 0.f;
		const f32_t fTicksPerSecond =
			m_pModelCom->Get_AnimationTickPerSecond(iAnimationIndex);
		if (!m_pModelCom->Get_AnimationProgress(
				iAnimationIndex, fTrackPosition, fTrackDuration) ||
			!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
			!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f)
		{
			return false;
		}
		OutAnimationIndices.push_back(iAnimationIndex);
		OutTimings.push_back(
			{ fTrackDuration / fTicksPerSecond, 0u, 1.f, false });
	}
	return true;
}

void CNpc::Spawn_DueEffectCues(
	const std::string_view actionId,
	const std::vector<std::string>& Clips,
	const std::vector<ACTION_PRESENTATION_CLIP_TIMING>& Timings,
	const f32_t fActionAgeSeconds)
{
	if (actionId.empty() || Clips.empty() || Clips.size() != Timings.size() ||
		m_EffectCueDocument.Cues.empty() ||
		0u == m_iServerActionStartTick ||
		!std::isfinite(fActionAgeSeconds) || fActionAgeSeconds < 0.f)
	{
		return;
	}

	const std::shared_ptr<CNpc> Owner =
		std::static_pointer_cast<CNpc>(shared_from_this());
	for (size_t iCue = 0u; iCue < m_EffectCueDocument.Cues.size(); ++iCue)
	{
		const ANIMATION_EFFECT_CUE& Cue = m_EffectCueDocument.Cues[iCue];
		for (size_t iClip = 0u; iClip < Clips.size(); ++iClip)
		{
			if (Cue.strClipName != Clips[iClip])
				continue;
			f32_t fCueWallSeconds = 0.f;
			if (!CActionPresentationTimeline::Resolve_CueWallOffset(
					std::span<const ACTION_PRESENTATION_CLIP_TIMING>(
						Timings.data(), Timings.size()),
					iClip,
					static_cast<f32_t>(Cue.iStartMs) * 0.001f,
					0u,
					fCueWallSeconds) ||
				fActionAgeSeconds + 0.00001f < fCueWallSeconds)
			{
				continue;
			}

			const std::string OccurrenceKey =
				"clip:" + std::to_string(iClip) +
				"/cue:" + std::to_string(iCue);
			if (!m_SpawnedEffectCueOccurrences.insert(OccurrenceKey).second)
				continue;

			const f32_t fInitialSampleSeconds = (std::max)(
				0.f,
				(fActionAgeSeconds - fCueWallSeconds) *
					Timings[iClip].fPlayRate);
			const uint32_t iCueDurationMs =
				EFFECT_STOP_POLICY::CUE_END == Cue.eStopPolicy ?
				Cue.iEndMs - Cue.iStartMs : 0u;
			if (EFFECT_STOP_POLICY::CUE_END == Cue.eStopPolicy &&
				fInitialSampleSeconds * 1000.f >=
					static_cast<f32_t>(iCueDurationMs))
			{
				continue;
			}

			EFFECT_SPAWN_DESC Desc;
			Desc.strEffectAssetId = Cue.strEffectAssetId;
			Desc.pNpcOwner = Owner;
			Desc.strAnchorSlotId = Cue.strAnchorSlotId;
			Desc.LocalTransform = Cue.LocalTransform;
			Desc.eFollowPolicy = Cue.eFollowPolicy;
			Desc.eStopPolicy = Cue.eStopPolicy;
			Desc.iCueDurationMs = iCueDurationMs;
			Desc.iActionStartTick = m_iServerActionStartTick;
			Desc.iCueStartMs = Cue.iStartMs;
			Desc.strOccurrenceId = "npc:" + m_strArchetypeId +
				"/action:" + std::string(actionId) + "/" + OccurrenceKey;
			Desc.fPlaybackRate = Timings[iClip].fPlayRate;
			Desc.fInitialSampleTimeSeconds = fInitialSampleSeconds;
			std::string Status;
			if (!CEffectPresentationService::Spawn(Desc, Status))
			{
				OutputDebugStringA((
					"[Client][Npc] action Effect spawn isolated for " +
					m_strArchetypeId + ": " + Status + "\n").c_str());
			}
		}
	}
}

bool_t CNpc::Apply_NetworkAction(
	const std::string_view actionId,
	const uint32_t iServerTick,
	const uint32_t iActionStartTick)
{
	if (nullptr == m_pModelCom || m_strArchetypeId.empty() ||
		0u == iServerTick)
	{
		return false;
	}
	if (0u != m_iLastServerTick)
	{
		if (iServerTick == m_iLastServerTick)
			return true;
		if (!CActionPresentationTimeline::Is_ForwardTick(
				iServerTick, m_iLastServerTick))
		{
			return false;
		}
	}

	std::vector<std::string> Clips;
	std::vector<uint32_t> AnimationIndices;
	std::vector<ACTION_PRESENTATION_CLIP_TIMING> Timings;
	if (!Build_ActionTimeline(
			actionId, Clips, AnimationIndices, Timings))
	{
		const NPC_ACTOR_ENTRY* pActor =
			CActorCatalog::Find_Npc(m_strArchetypeId);
		if (nullptr == pActor)
			return false;
		const uint32_t iPreviousActionStartTick =
			m_iServerActionStartTick;
		if (!m_strServerActionId.empty() &&
			!m_pModelCom->Set_Animation(pActor->idleClip.c_str(), true))
		{
			return false;
		}
		if (0u != iPreviousActionStartTick)
		{
			CEffectPresentationService::Stop_NpcAction(
				std::static_pointer_cast<CNpc>(shared_from_this()),
				iPreviousActionStartTick);
		}
		m_strServerActionId.clear();
		m_iServerActionStartTick = 0u;
		m_fServerActionAgeSeconds = 0.f;
		m_SpawnedEffectCueOccurrences.clear();
		m_iLastServerTick = iServerTick;
		return true;
	}

	f32_t fActionAgeSeconds = 0.f;
	if (0u == iActionStartTick ||
		!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
			iServerTick, iActionStartTick, 30.f, fActionAgeSeconds))
	{
		return false;
	}
	const bool_t bActionEdge =
		m_strServerActionId != actionId ||
		m_iServerActionStartTick != iActionStartTick;
	if (0u != m_iServerActionStartTick && bActionEdge &&
		!CActionPresentationTimeline::Is_ForwardTick(
			iActionStartTick, m_iServerActionStartTick))
	{
		return false;
	}

	ACTION_PRESENTATION_SAMPLE Sample;
	if (!CActionPresentationTimeline::Resolve_Sample(
			std::span<const ACTION_PRESENTATION_CLIP_TIMING>(
				Timings.data(), Timings.size()),
			fActionAgeSeconds,
			Sample) ||
		Sample.iClipIndex >= AnimationIndices.size())
	{
		return false;
	}
	const uint32_t iTargetAnimation =
		AnimationIndices[Sample.iClipIndex];
	const bool_t bNeedsSeek = bActionEdge ||
		m_pModelCom->Get_CurrentAnimIndex() != iTargetAnimation ||
		fActionAgeSeconds > m_fServerActionAgeSeconds + 0.001f;
	if (bNeedsSeek)
	{
		if (bActionEdge ||
			m_pModelCom->Get_CurrentAnimIndex() != iTargetAnimation)
		{
			if (!m_pModelCom->Start_Animation(
					Clips[Sample.iClipIndex].c_str(), false))
			{
				return false;
			}
		}
		else
		{
			m_pModelCom->Set_Animation(iTargetAnimation, false);
		}
		const f32_t fTicksPerSecond =
			m_pModelCom->Get_AnimationTickPerSecond(iTargetAnimation);
		if (!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
			!m_pModelCom->Set_AnimTrackPosition(
				iTargetAnimation,
				Sample.fClipSourceTimeSeconds * fTicksPerSecond))
		{
			return false;
		}
		m_pModelCom->Play_Animation(0.f);
	}

	const uint32_t iPreviousActionStartTick = m_iServerActionStartTick;
	if (bActionEdge && 0u != iPreviousActionStartTick)
	{
		CEffectPresentationService::Stop_NpcAction(
			std::static_pointer_cast<CNpc>(shared_from_this()),
			iPreviousActionStartTick);
	}
	if (bActionEdge)
		m_SpawnedEffectCueOccurrences.clear();
	m_strServerActionId.assign(actionId);
	m_iLastServerTick = iServerTick;
	m_iServerActionStartTick = iActionStartTick;
	m_fServerActionAgeSeconds = fActionAgeSeconds;
	Spawn_DueEffectCues(actionId, Clips, Timings, fActionAgeSeconds);
	return true;
}

void CNpc::Trigger_HitFlash()
{
	m_fHitFlashRemainingSeconds = HIT_FLASH_DURATION_SECONDS;
	m_HitFlash.isEnabled = true;
	m_HitFlash.vColor = float4_t(1.f, 1.f, 1.f, 1.f);
	m_HitFlash.fIntensity = HIT_FLASH_PEAK_INTENSITY;
	m_HitFlash.usesSurfaceDetailMask = true;
}

void CNpc::Priority_Update(f32_t fTimeDelta)
{
}

void CNpc::Update(f32_t fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);
	if (m_fHitFlashRemainingSeconds > 0.f)
	{
		m_fHitFlashRemainingSeconds -= fTimeDelta;
		if (m_fHitFlashRemainingSeconds <= 0.f)
		{
			m_fHitFlashRemainingSeconds = 0.f;
			m_HitFlash = {};
		}
		else
		{
			m_HitFlash.fIntensity = HIT_FLASH_PEAK_INTENSITY *
				(m_fHitFlashRemainingSeconds / HIT_FLASH_DURATION_SECONDS);
		}
	}
}

void CNpc::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
#ifdef _DEBUG
	if (m_isCombatColliderDebugVisible && nullptr != m_pColliderCom)
		CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
#endif
}

HRESULT CNpc::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const uint32_t iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (uint32_t i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, &m_HitFlash)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CNpc::Ready_Components(const NPC_DESC* pDesc)
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

	if (pDesc->fCollisionRadius > 0.f)
	{
		Engine::CBounding_Sphere::BOUNDING_SPHERE_DESC colliderDesc{};
		colliderDesc.vCenter = float3_t(
			0.f, pDesc->fCollisionRadius, 0.f);
		colliderDesc.fRadius = pDesc->fCollisionRadius;
		if (FAILED(__super::Add_Component(
			pDesc->iPrototypeLevelIndex,
			TEXT("Prototype_Component_Collider_WorldEntity"),
			TEXT("Com_CombatCollider"),
			m_pColliderCom,
			&colliderDesc)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CNpc::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

unique_ptr<CNpc> CNpc::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CNpc>(new CNpc(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CNpc");

	return move(pInstance);
}

shared_ptr<CPrototype> CNpc::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CNpc>(new CNpc(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CNpc");

	return pInstance;
}
