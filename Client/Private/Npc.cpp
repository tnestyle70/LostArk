#include "Npc.h"
#include "KoukuSaydonAnimationBlend.h"
#include "KoukuSaydonCompositionDocument.h"
#include "EffectV2_Runtime.h"
#include "Effect_PresentationService.h"
#include "AnimationTargetService.h"
#include "NpcPresentationAssetService.h"

#include "Collider.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Transform.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr f32_t HIT_FLASH_DURATION_SECONDS = 0.12f;
	constexpr f32_t HIT_FLASH_PEAK_INTENSITY = 4.f;
	constexpr const char_t* ROOT_MOTION_BONE = "b_root";
	constexpr int32_t ROOT_MOTION_VERTICAL_AXIS = 2;
	constexpr const char_t* PLAYER_LEFT_HAND_BONE = "bip001-l-hand";
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

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;
	if (FAILED(CNpcPresentationAssetService::Prepare_SaydonHat(m_pDevice, m_pContext, m_pModelCom, m_pSaydonHatModel)))
		OutputDebugStringA("[SaydonHat] NPC head prop unavailable; body preserved.\n");

	Apply_ImmediateTransform(pDesc->vPosition, pDesc->fYawDegree);

	m_fOutlineWidth = pDesc->fOutlineWidth;
	m_vOutlineColor = pDesc->vOutlineColor;
	m_bSuppressRootMotion = pDesc->bSuppressRootMotion;
	m_bInterpolateNetworkTransform = pDesc->bInterpolateNetworkTransform;

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
	const char_t* pResolvedIdle = m_pModelCom->Get_AnimationName(
		m_pModelCom->Get_CurrentAnimIndex());
	if (nullptr == pResolvedIdle || '\0' == pResolvedIdle[0])
		return E_FAIL;
	m_strDefaultIdleClip = pResolvedIdle;
	m_pModelCom->Set_AnimationSpeed(1.f);
	/* Authored town placements are Server-transform authoritative. Their clips
	may pose translated root keys but must not move presentation away from the
	replicated transform. Esther leaves suppression disabled because its existing
	action chains intentionally use authored root motion. */
	if (m_bSuppressRootMotion)
	{
		m_pModelCom->Enable_RootMotionSuppression(
			ROOT_MOTION_BONE, ROOT_MOTION_VERTICAL_AXIS);
	}

	return S_OK;
}

void CNpc::Synchronize_WeaponPose()
{
	CNpcPresentationAssetService::Synchronize_SaydonHammerPose(m_pModelCom, m_pWeaponModelCom, m_WeaponRestPose);
}

bool_t CNpc::Try_GetAnimationModelTarget(const ANIMATION_BONE_TARGET target,
	ANIMATION_MODEL_TARGET_VIEW& outView) const
{
	if (!m_pModelCom || !m_pTransformCom) return false;
	ANIMATION_MODEL_TARGET_VIEW staged;
	staged.TargetRoot = *m_pTransformCom->Get_WorldMatrixPtr();
	if (target == ANIMATION_BONE_TARGET::BODY)
	{
		staged.Model = m_pModelCom;
		staged.BoneRoot = staged.TargetRoot;
	}
	else if (target == ANIMATION_BONE_TARGET::WEAPON)
	{
		if (!m_pWeaponModelCom || !m_pModelCom->Has_Bone(m_strWeaponSocketBone.c_str())) return false;
		matrix_t weaponLocal = XMMatrixIdentity();
#ifdef _DEBUG
		weaponLocal = XMLoadFloat4x4(&m_DebugWeaponRotation) * XMMatrixScaling(m_fDebugWeaponScale, m_fDebugWeaponScale, m_fDebugWeaponScale);
#endif
		staged.Model = m_pWeaponModelCom;
		XMStoreFloat4x4(&staged.BoneRoot, weaponLocal *
			m_pModelCom->Get_BoneMatrix(m_strWeaponSocketBone.c_str()) * XMLoadFloat4x4(&staged.TargetRoot));
	}
	else return false;
	outView = std::move(staged);
	return true;
}

bool_t CNpc::Set_PlayerHandGripLocalOffset(const PLAYER_HAND_GRIP_LOCAL_OFFSET& offset)
{
    if (!m_pModelCom || !m_pModelCom->Has_Bone(PLAYER_LEFT_HAND_BONE) ||
        !CPlayerHandGripTransform::Is_ValidGripLocalOffset(offset)) return false;
    m_PlayerHandGripLocalOffset = offset;
    return true;
}

bool_t CNpc::Try_Get_PlayerHandGripLocalOffset(const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
    PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const
{
    if (slot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ||
        !m_PlayerHandGripLocalOffset ||
        !CPlayerHandGripTransform::Is_ValidGripLocalOffset(*m_PlayerHandGripLocalOffset)) return false;
    outOffset = *m_PlayerHandGripLocalOffset;
    return true;
}

bool_t CNpc::Try_Get_PlayerHandGripSocketView(const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
    PLAYER_HAND_GRIP_SOCKET_VIEW& outView) const
{
    if (slot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ||
        !m_PlayerHandGripLocalOffset || !m_pModelCom || !m_pTransformCom ||
        !m_pModelCom->Has_Bone(PLAYER_LEFT_HAND_BONE)) return false;
    PLAYER_HAND_GRIP_SOCKET_VIEW staged{};
    const auto* root = m_pTransformCom->Get_WorldMatrixPtr();
    XMStoreFloat4x4(&staged.SocketWorld,
        m_pModelCom->Get_BoneMatrix(PLAYER_LEFT_HAND_BONE) * XMLoadFloat4x4(root));
    staged.OwnerYawBasis = *root;
    // The same composition validates matrices and metre offsets for every owner.
    float3_t position{};
    if (!CPlayerHandGripTransform::Compose_WorldPosition(staged, *m_PlayerHandGripLocalOffset, position)) return false;
    outView = staged;
    return true;
}

bool_t CNpc::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == pClipName || nullptr == m_pModelCom)
		return false;
	m_bNetworkAnimationWindow = false;
	m_bNetworkAnimationTransition = false;
    m_NetworkAnimationBlendWindows.clear();
    m_iNetworkSemanticClip = UINT32_MAX;
    m_iNetworkSemanticAbsoluteStartMs = UINT32_MAX;
    m_fNetworkPatternAgeSeconds = 0.0;
	m_iNetworkAnimationSourceStartMs = m_iNetworkAnimationSourceEndMs = 0u;
	m_fNetworkAnimationHoldSeconds = 0.f;
	m_fNetworkAnimationAgeSeconds = 0.f;
	m_isNetworkAnimationLoop = isLoop;
	m_fNetworkAnimationStartOffsetSeconds = 0.f;
	m_pModelCom->Set_AnimationSpeed(1.f);
	if (!m_pModelCom->Set_Animation(pClipName, isLoop))
		return false;
	CEffectV2Runtime::Notify_Clip(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		pClipName);
	Arm_ActionEffectCues(pClipName);
	return true;
}

bool_t CNpc::Try_SampleNetworkAnimationTicks(const f32_t animationAgeSeconds,
    const uint32_t clip, const f32_t playRate, f32_t& outTicks) const
{
    if (!m_pModelCom || !std::isfinite(animationAgeSeconds) || animationAgeSeconds < 0.f) return false;
    float cursor = 0.f, duration = 0.f;
    const float ticksPerSecond = m_pModelCom->Get_AnimationTickPerSecond(clip);
    if (!m_pModelCom->Get_AnimationProgress(clip, cursor, duration) ||
        !std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f) return false;
    const float sampleAge = m_fNetworkAnimationHoldSeconds > 0.f ?
        (std::min)(animationAgeSeconds, m_fNetworkAnimationHoldSeconds) : animationAgeSeconds;
    double sourceMs = 0.0;
    if (!Client::CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(
        m_iNetworkAnimationSourceStartMs, m_iNetworkAnimationSourceEndMs,
        double(sampleAge) * 1000.0, playRate, double(duration) * 1000.0 / ticksPerSecond,
        m_isNetworkAnimationLoop, sourceMs)) return false;
    outTicks = (std::min)(duration, float(sourceMs * .001 * ticksPerSecond));
    return true;
}

bool_t CNpc::Set_NetworkAnimationWindow(const f32_t ageSeconds, const f32_t holdSeconds,
    const f32_t startOffsetSeconds, const uint32_t sourceStartMs, const uint32_t sourceEndMs)
{
    if (!m_pModelCom || !std::isfinite(ageSeconds) || ageSeconds < 0.f ||
        !std::isfinite(holdSeconds) || holdSeconds < 0.f || holdSeconds > 600.f ||
        !std::isfinite(startOffsetSeconds) || startOffsetSeconds < 0.f || startOffsetSeconds > 600.f) return false;
    const auto clip = m_pModelCom->Get_CurrentAnimIndex();
    float cursor = 0.f, duration = 0.f;
    const float ticksPerSecond = m_pModelCom->Get_AnimationTickPerSecond(clip);
    if (!m_pModelCom->Get_AnimationProgress(clip, cursor, duration) ||
        !std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f) return false;
    const float animationAge = (std::max)(0.f, ageSeconds - startOffsetSeconds);
    const float sampleAge = holdSeconds > 0.f ? (std::min)(animationAge, holdSeconds) : animationAge;
    double sourceMs = 0.0;
    if (!Client::CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(
        sourceStartMs, sourceEndMs, double(sampleAge) * 1000.0, m_fNetworkAnimationPlayRate,
        double(duration) * 1000.0 / ticksPerSecond, m_isNetworkAnimationLoop, sourceMs)) return false;
    const float ticks = (std::min)(duration, float(sourceMs * .001 * ticksPerSecond));
    // The source window owns wrapping. Disable the model's full-clip loop so
    // its endpoint cannot wrap behind the cropped-range sampler.
    if (!m_pModelCom->Start_Animation(clip, false) ||
        !m_pModelCom->Set_AnimTrackPosition(clip, ticks)) return false;
    m_fNetworkAnimationAgeSeconds = ageSeconds;
    m_fNetworkAnimationHoldSeconds = holdSeconds;
    m_fNetworkAnimationStartOffsetSeconds = startOffsetSeconds;
    m_iNetworkAnimationSourceStartMs = sourceStartMs;
    m_iNetworkAnimationSourceEndMs = sourceEndMs;
    m_bNetworkAnimationWindow = true;
    m_iNetworkSemanticClip = clip;
    m_pModelCom->Skip_Blend();
    m_pModelCom->Set_AnimPaused(true);
    m_pModelCom->Update_Animation(0.f);
    Synchronize_WeaponPose();
    return true;
}

bool_t CNpc::Set_NetworkAnimationBlendWindows(
    const std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW>& windows,
    const std::string_view semanticOccurrenceId, const f32_t patternAgeSeconds)
{
    if (!m_pModelCom || !m_bNetworkAnimationWindow || !std::isfinite(patternAgeSeconds) || patternAgeSeconds < 0.f)
        return false;
    std::string status;
    if (!CKoukuSaydonAnimationBlend::Validate_ModelWindows(*m_pModelCom, windows, status)) return false;
    const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* semantic = nullptr;
    for (const auto& window : windows)
        for (const auto* row : {&window.Source, &window.Target})
            if (row->strOccurrenceId == semanticOccurrenceId)
            {
                if (semantic && *semantic != *row) return false;
                semantic = row;
            }
    const char* clipName = m_pModelCom->Get_AnimationName(m_iNetworkSemanticClip);
    if (!semantic || !clipName || semantic->strRuntimeClip != clipName) return false;
    CModel::ANIMATION_TRANSITION_POSE pose;
    bool active = false;
    if (!CKoukuSaydonAnimationBlend::Sample_Pose(*m_pModelCom, windows, double(patternAgeSeconds) * 1000.0,
        pose, active, status) || (active && !m_pModelCom->Set_AnimationTransitionPose(pose))) return false;
    const float semanticAge = (std::max)(0.f, patternAgeSeconds - semantic->iStartOffsetMs * .001f);
    if (!active)
    {
        float ticks = 0.f;
        if (!Try_SampleNetworkAnimationTicks(semanticAge, m_iNetworkSemanticClip, m_fNetworkAnimationPlayRate, ticks)) return false;
        if (m_bNetworkAnimationTransition)
        {
            pose.sourceIndex = m_iTransitionSourceIndex; pose.sourceTicks = m_fTransitionSourceTicks;
            pose.targetIndex = m_iNetworkSemanticClip; pose.targetTicks = ticks;
            pose.durationSeconds = m_fTransitionDurationSeconds; pose.elapsedSeconds = semanticAge;
            pose.playRate = m_fTransitionPlayRate;
            if (!m_pModelCom->Set_AnimationTransitionPose(pose)) return false;
        }
        else
        {
            m_pModelCom->Clear_AnimationTransitionPose();
            m_pModelCom->Set_Animation(m_iNetworkSemanticClip, false, 0.f);
            if (!m_pModelCom->Set_AnimTrackPosition(m_iNetworkSemanticClip, ticks)) return false;
            m_pModelCom->Skip_Blend();
            m_pModelCom->Update_Animation(0.f);
        }
    }
    m_NetworkAnimationBlendWindows = windows;
    m_iNetworkSemanticAbsoluteStartMs = semantic->iStartOffsetMs;
    m_fNetworkPatternAgeSeconds = patternAgeSeconds;
    m_pModelCom->Set_AnimPaused(true);
    Synchronize_WeaponPose();
    return true;
}

bool_t CNpc::Apply_NetworkAnimationTransition(const char_t* sourceClip, const f32_t sourceMs,
    const f32_t durationMs, const f32_t ageSeconds, const f32_t playRate)
{
    if (!m_pModelCom || !m_bNetworkAnimationWindow || !sourceClip ||
        !std::isfinite(sourceMs) || sourceMs < 0.f ||
        !std::isfinite(durationMs) || durationMs <= 0.f || durationMs > 1000.f ||
        !std::isfinite(ageSeconds) || ageSeconds < 0.f || !std::isfinite(playRate) || playRate <= 0.f) return false;
    uint32_t sourceIndex = UINT32_MAX;
    for (uint32_t i = 0; i < m_pModelCom->Get_NumAnimations(); ++i)
    {
        const char_t* name = m_pModelCom->Get_AnimationName(i);
        if (name && std::string_view(sourceClip) == name) { sourceIndex = i; break; }
    }
    if (sourceIndex == UINT32_MAX) return false;
    const uint32_t targetIndex = m_pModelCom->Get_CurrentAnimIndex();
    float cursor = 0.f, sourceEnd = 0.f;
    const float sourceTicksPerSecond = m_pModelCom->Get_AnimationTickPerSecond(sourceIndex);
    if (!m_pModelCom->Get_AnimationProgress(sourceIndex, cursor, sourceEnd) ||
        !std::isfinite(sourceTicksPerSecond) || sourceTicksPerSecond <= 0.f ||
        !std::isfinite(sourceEnd) || sourceEnd <= 0.f) return false;
    CModel::ANIMATION_TRANSITION_POSE pose;
    pose.sourceIndex = sourceIndex; pose.targetIndex = targetIndex;
    pose.sourceTicks = (std::min)(sourceEnd, sourceMs * .001f * sourceTicksPerSecond);
    if (!Try_SampleNetworkAnimationTicks(ageSeconds, targetIndex, playRate, pose.targetTicks)) return false;
    pose.durationSeconds = durationMs * .001f; pose.elapsedSeconds = ageSeconds; pose.playRate = playRate;
    const bool waitingForAnimation = m_fNetworkAnimationAgeSeconds < m_fNetworkAnimationStartOffsetSeconds;
    if (!waitingForAnimation && !m_pModelCom->Set_AnimationTransitionPose(pose)) return false;
    m_iTransitionSourceIndex = sourceIndex; m_fTransitionSourceTicks = pose.sourceTicks;
    m_fTransitionDurationSeconds = pose.durationSeconds; m_fTransitionAgeSeconds = ageSeconds;
    m_fTransitionPlayRate = playRate; m_bNetworkAnimationTransition = true;
    m_pModelCom->Set_AnimPaused(true);
    Synchronize_WeaponPose();
    return true;
}

bool_t CNpc::Play_NetworkAction(
	const char_t* pClipName,
	const bool_t isLoop,
	const f32_t fPlaybackRate,
	const f32_t fBlendSeconds,
	const f32_t fRootVerticalScale)
{
	m_strClipEndEffect.clear();
	m_fClipEndEffectRemaining = 0.f;
	m_bNetworkAnimationWindow = false;
	m_bNetworkAnimationTransition = false;
    m_NetworkAnimationBlendWindows.clear();
    m_iNetworkSemanticClip = UINT32_MAX;
    m_iNetworkSemanticAbsoluteStartMs = UINT32_MAX;
    m_fNetworkPatternAgeSeconds = 0.0;
	m_iNetworkAnimationSourceStartMs = m_iNetworkAnimationSourceEndMs = 0u;
	m_fNetworkAnimationHoldSeconds = 0.f;
	m_fNetworkAnimationAgeSeconds = 0.f;
	m_isNetworkAnimationLoop = isLoop;
	m_fNetworkAnimationStartOffsetSeconds = 0.f;
	m_fTransientActionRemainingSeconds = 0.f;
	m_strTransientReturnClip.clear();
	if (nullptr == pClipName || '\0' == pClipName[0] ||
		nullptr == m_pModelCom || !std::isfinite(fPlaybackRate) ||
		fPlaybackRate < 0.1f || fPlaybackRate > 4.f ||
		!std::isfinite(fBlendSeconds) ||
		fBlendSeconds < 0.f || fBlendSeconds > 2.f ||
		!std::isfinite(fRootVerticalScale) || fRootVerticalScale < 0.f || fRootVerticalScale > 1.f ||
		(fRootVerticalScale != 1.f && !m_bSuppressRootMotion) ||
		!m_pModelCom->Set_Animation(pClipName, isLoop, fBlendSeconds))
	{
		return false;
	}
	if (!m_pModelCom->Set_RootMotionVerticalScale(fRootVerticalScale)) return false;
	m_pModelCom->Set_AnimationSpeed(fPlaybackRate);
	m_fNetworkAnimationPlayRate = fPlaybackRate;
	if (!m_pModelCom->Start_Animation(
			m_pModelCom->Get_CurrentAnimIndex(), isLoop))
	{
		return false;
	}
	/* Keep the existing NPC effect/cutin hook on every semantic action edge,
	including a restart that resolves to the same clip name. */
	CEffectV2Runtime::Notify_Clip(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		pClipName);
	Arm_ActionEffectCues(pClipName);
	return true;
}

void CNpc::Arm_ActionEffectCues(const char_t* pClipName)
{
	m_NpcActionEffectState.Reset();
	m_strNpcActionEffectArchetype.clear();
	if (nullptr == pClipName || '\0' == pClipName[0])
		return;
	/* One identity for both presentation documents: an NPC without an explicit
	binding owner resolves its archetype from the model tag, exactly as the
	existing binding path does. */
	const std::string archetype = CEffectV2Runtime::Resolve_ArchetypeId(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())));
	if (archetype.empty())
		return;
	std::string status;
	if (!CNpcActionEffectCueDocument::Load(archetype, status))
	{
		OutputDebugStringA(
			("[Npc] Action Effect cues rejected for " + archetype + ": " +
			 status + "\n").c_str());
		return;
	}
	if (!CNpcActionEffectCueDocument::Has_Clip(archetype, pClipName))
		return;
	/* Product spawn only consumes prepared targets, so register this
	archetype's documents once. Preparation settles on later frame seams;
	a cue whose target is not ready yet is isolated by Spawn itself. */
	if (!m_bNpcActionEffectTargetsQueued)
	{
		m_bNpcActionEffectTargetsQueued = true;
		std::vector<std::string> targets, admitted;
		for (const NPC_ACTION_EFFECT_CUE& cue :
			CNpcActionEffectCueDocument::Get_Cues(archetype))
		{
			if (std::find(targets.begin(), targets.end(), cue.strEffectAssetId) ==
				targets.end())
			{
				targets.push_back(cue.strEffectAssetId);
			}
		}
		if (!targets.empty() &&
			!CEffectPresentationService::Queue_ProductTargets_Priority(
				targets, admitted, status))
		{
			OutputDebugStringA(
				("[Npc] Action Effect targets rejected for " + archetype + ": " +
				 status + "\n").c_str());
		}
	}
	m_strNpcActionEffectArchetype = archetype;
	m_NpcActionEffectState.strClip = pClipName;
	m_NpcActionEffectState.bActive = true;
	++m_iNpcActionEffectOccurrence;
}

void CNpc::Update_ActionEffectCues(const f32_t fTimeDelta)
{
	if (!m_NpcActionEffectState.bActive || nullptr == m_pTransformCom ||
		!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
	{
		return;
	}
	m_NpcActionEffectState.fElapsedSeconds += fTimeDelta;
	const auto& cues =
		CNpcActionEffectCueDocument::Get_Cues(m_strNpcActionEffectArchetype);
	const uint32_t iLevel = CGameInstance::Get().Get_CurrentLevelID();
	while (m_NpcActionEffectState.iNextCue < cues.size())
	{
		const NPC_ACTION_EFFECT_CUE& cue = cues[m_NpcActionEffectState.iNextCue];
		if (cue.strClip != m_NpcActionEffectState.strClip)
		{
			++m_NpcActionEffectState.iNextCue;
			continue;
		}
		const f32_t fDue = static_cast<f32_t>(cue.iStartMs) * 0.001f;
		if (m_NpcActionEffectState.fElapsedSeconds < fDue)
			break;
		++m_NpcActionEffectState.iNextCue;
		EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
		desc.iLevelIndex = iLevel;
		desc.strPlacementId = cue.strCueId + ":" +
			std::to_string(m_iNpcActionEffectOccurrence);
		desc.strEffectAssetId = cue.strEffectAssetId;
		desc.RootWorld = *m_pTransformCom->Get_WorldMatrixPtr();
		desc.pAnchorOwner = static_pointer_cast<CNpc>(shared_from_this());
		/* Catch a cue the frame overshot so a long frame still starts it at
		its authored phase instead of from zero. */
		desc.fInitialSampleTimeSeconds =
			m_NpcActionEffectState.fElapsedSeconds - fDue;
		EFFECT_WORLD_ROOT_HANDLE handle;
		std::string status;
		if (!CEffectPresentationService::Spawn_LevelPlacement(desc, handle, status))
		{
			OutputDebugStringA(
				("[Npc] Action Effect cue isolated: " + cue.strCueId + " " +
				 status + "\n").c_str());
		}
	}
	if (m_NpcActionEffectState.iNextCue >= cues.size())
		m_NpcActionEffectState.bActive = false;
}

bool_t CNpc::Schedule_ClipEndEffect(const std::string& effectAssetId,
	const uint32_t levelIndex, const std::string& occurrenceId)
{
	if (effectAssetId.empty() || occurrenceId.empty() || levelIndex >= ETOUI(LEVEL::END) ||
		!m_pModelCom || m_isNetworkAnimationLoop || !CEffectCatalog::Contains(effectAssetId)) return false;
	const uint32_t clip = m_pModelCom->Get_CurrentAnimIndex();
	f32_t position = 0.f, duration = 0.f;
	const f32_t rate = m_pModelCom->Get_AnimationTickPerSecond(clip) * m_fNetworkAnimationPlayRate;
	if (rate <= 0.f || !m_pModelCom->Get_AnimationProgress(clip, position, duration) || duration <= 0.f) return false;
	m_strClipEndEffect = effectAssetId;
	m_strClipEndEffectOccurrence = occurrenceId;
	m_iClipEndEffectLevel = levelIndex;
	m_fClipEndEffectRemaining = (std::max)(0.f, duration - position) / rate;
	return true;
}

bool_t CNpc::Play_TransientNetworkAction(
	const char_t* pClipName,
	const f32_t fPlaybackRate,
	const f32_t fDurationSeconds,
	const char_t* pReturnClip,
	const bool_t isReturnLoop,
	const f32_t fReturnPlaybackRate,
	const f32_t fBlendSeconds)
{
	if (nullptr == pReturnClip || '\0' == pReturnClip[0] ||
		!std::isfinite(fDurationSeconds) || fDurationSeconds <= 0.f ||
		fDurationSeconds > 5.f || !std::isfinite(fReturnPlaybackRate) ||
		fReturnPlaybackRate < 0.1f || fReturnPlaybackRate > 4.f ||
		!Play_NetworkAction(
			pClipName, false, fPlaybackRate, fBlendSeconds))
	{
		return false;
	}
	m_fTransientActionRemainingSeconds = fDurationSeconds;
	m_strTransientReturnClip = pReturnClip;
	m_fTransientReturnPlaybackRate = fReturnPlaybackRate;
	m_isTransientReturnLoop = isReturnLoop;
	return true;
}

bool_t CNpc::Play_DefaultIdle(const f32_t fBlendSeconds)
{
	return !m_strDefaultIdleClip.empty() && Play_NetworkAction(
		m_strDefaultIdleClip.c_str(), true, 1.f, fBlendSeconds);
}

bool_t CNpc::Apply_NetworkState(
	const float3_t& position,
	const f32_t yawDegrees,
	const std::uint32_t iServerTick,
	const bool_t snapToSnapshot)
{
	if (nullptr == m_pTransformCom ||
		!std::isfinite(position.x) ||
		!std::isfinite(position.y) ||
		!std::isfinite(position.z) ||
		!std::isfinite(yawDegrees))
	{
		return false;
	}
	if (!m_bInterpolateNetworkTransform)
	{
		Apply_ImmediateTransform(position, yawDegrees);
		return true;
	}
	/* Spawn messages carry no simulation tick. They place the object exactly;
	interpolation begins with the first world snapshot that has a tick. */
	if (0u == iServerTick)
	{
		m_NetworkTransformInterpolator.Reset();
		Apply_ImmediateTransform(position, yawDegrees);
		return true;
	}
	// A new Server pattern can commit a discontinuous position and facing.
	// Seed both the rendered root and interpolation history before its first cue.
	if (snapToSnapshot)
		m_NetworkTransformInterpolator.Reset();
	if (!m_NetworkTransformInterpolator.Push(position, yawDegrees, iServerTick))
		return false;
	if (snapToSnapshot)
		Apply_ImmediateTransform(position, yawDegrees);
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
	if (m_bInterpolateNetworkTransform)
		Update_NetworkTransform(fTimeDelta);
	if (m_fTransientActionRemainingSeconds > 0.f)
	{
		m_fTransientActionRemainingSeconds -= fTimeDelta;
		if (m_fTransientActionRemainingSeconds <= 0.f)
		{
			const std::string returnClip = m_strTransientReturnClip;
			const f32_t returnRate = m_fTransientReturnPlaybackRate;
			const bool_t returnLoop = m_isTransientReturnLoop;
			m_fTransientActionRemainingSeconds = 0.f;
			m_strTransientReturnClip.clear();
			if (!returnClip.empty())
			{
				(void)Play_NetworkAction(
					returnClip.c_str(), returnLoop, returnRate, 0.05f);
			}
		}
	}
    const float frameDelta = (std::max)(0.f, fTimeDelta);
    m_fNetworkAnimationAgeSeconds += frameDelta;
    m_fNetworkPatternAgeSeconds += frameDelta;
    const float animationAge = m_bNetworkAnimationWindow && m_iNetworkSemanticAbsoluteStartMs != UINT32_MAX ?
        float((std::max)(0.0, m_fNetworkPatternAgeSeconds - double(m_iNetworkSemanticAbsoluteStartMs) * .001)) :
        (std::max)(0.f, m_fNetworkAnimationAgeSeconds - m_fNetworkAnimationStartOffsetSeconds);
    if (m_bNetworkAnimationWindow)
    {
        const auto clip = m_iNetworkSemanticClip;
        float ticks = 0.f;
        bool sampled = Try_SampleNetworkAnimationTicks(animationAge, clip,
            m_fNetworkAnimationPlayRate, ticks);
        CModel::ANIMATION_TRANSITION_POSE logicPose;
        bool logicActive = false;
        std::string blendStatus;
        if (sampled) sampled = CKoukuSaydonAnimationBlend::Sample_Pose(*m_pModelCom,
            m_NetworkAnimationBlendWindows, m_fNetworkPatternAgeSeconds * 1000.0, logicPose, logicActive, blendStatus);
        if (sampled && logicActive) sampled = m_pModelCom->Set_AnimationTransitionPose(logicPose);
        else if (sampled && m_bNetworkAnimationTransition &&
            m_fNetworkAnimationAgeSeconds >= m_fNetworkAnimationStartOffsetSeconds)
        {
            CModel::ANIMATION_TRANSITION_POSE pose;
            pose.sourceIndex = m_iTransitionSourceIndex; pose.sourceTicks = m_fTransitionSourceTicks;
            pose.targetIndex = clip; pose.targetTicks = ticks;
            pose.durationSeconds = m_fTransitionDurationSeconds; pose.elapsedSeconds = animationAge;
            pose.playRate = m_fTransitionPlayRate;
            sampled = m_pModelCom->Set_AnimationTransitionPose(pose);
            m_fTransitionAgeSeconds = animationAge;
        }
        else if (sampled)
        {
            m_pModelCom->Clear_AnimationTransitionPose();
            m_pModelCom->Set_Animation(clip, false, 0.f);
            sampled = m_pModelCom->Set_AnimTrackPosition(clip, ticks);
            if (sampled) { m_pModelCom->Skip_Blend(); m_pModelCom->Update_Animation(0.f); }
        }
        if (!sampled)
        {
            // A failed source sample must hold the last valid palette rather
            // than silently resume the uncropped full clip.
            m_pModelCom->Set_AnimPaused(true);
            m_bNetworkAnimationWindow = false; m_bNetworkAnimationTransition = false;
            OutputDebugStringA("[Npc] Network animation source window could not be sampled.\n");
        }
    }
    else if (m_bSuppressRootMotion)
    {
        m_pModelCom->Update_Animation(frameDelta);
    }
    else
    {
        m_pModelCom->Play_Animation(frameDelta);
    }
	Synchronize_WeaponPose();
	Update_CombatCollider();
	if (!m_strClipEndEffect.empty())
	{
		m_fClipEndEffectRemaining -= frameDelta;
		if (m_fClipEndEffectRemaining <= 0.f)
		{
			EFFECT_LEVEL_PLACEMENT_SPAWN_DESC cue;
			cue.iLevelIndex = m_iClipEndEffectLevel;
			cue.strPlacementId = m_strClipEndEffectOccurrence;
			cue.strEffectAssetId = m_strClipEndEffect;
			cue.RootWorld = *m_pTransformCom->Get_WorldMatrixPtr();
			cue.fInitialSampleTimeSeconds = -m_fClipEndEffectRemaining;
			EFFECT_WORLD_ROOT_HANDLE handle;
			std::string status;
			if (!CEffectPresentationService::Spawn_LevelPlacement(cue, handle, status))
				OutputDebugStringA(("[Npc] Clip-end Effect isolated: " + status + "\n").c_str());
			m_strClipEndEffect.clear();
		}
	}
	Update_ActionEffectCues(frameDelta);
	CEffectV2Runtime::Tick(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		m_pDevice, m_pContext);
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

void CNpc::Apply_ImmediateTransform(
	const float3_t& position,
	const f32_t yawDegrees)
{
	float3_t drawn = position;
	f32_t drawnYaw = yawDegrees;
#ifdef _DEBUG
	m_vDebugUnadjustedPosition = position;
	m_fDebugUnadjustedYawDegrees = yawDegrees;
	drawn.x += m_vDebugPresentationOffset.x;
	drawn.y += m_vDebugPresentationOffset.y;
	drawn.z += m_vDebugPresentationOffset.z;
	drawnYaw += m_fDebugPresentationYawOffset;
#endif
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(drawn.x, drawn.y, drawn.z, 1.f));
	// Rotation keeps the transform's current scale, so a Debug scale set once
	// through Set_DebugPresentationScale survives every replicated pose.
	m_pTransformCom->Rotation(0.f, drawnYaw, 0.f);
	Update_CombatCollider();
}

void CNpc::Update_NetworkTransform(const f32_t fTimeDelta)
{
	if (nullptr == m_pTransformCom)
		return;
	NPC_NETWORK_TRANSFORM_FRAME frame{};
	if (!m_NetworkTransformInterpolator.Advance(fTimeDelta, frame))
		return;
	float3_t drawn = frame.vPosition;
	f32_t drawnYaw = frame.fYawDegrees;
#ifdef _DEBUG
	m_vDebugUnadjustedPosition = frame.vPosition;
	m_fDebugUnadjustedYawDegrees = frame.fYawDegrees;
	drawn.x += m_vDebugPresentationOffset.x;
	drawn.y += m_vDebugPresentationOffset.y;
	drawn.z += m_vDebugPresentationOffset.z;
	drawnYaw += m_fDebugPresentationYawOffset;
#endif
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(
		drawn.x,
		drawn.y,
		drawn.z,
		1.f));
	m_pTransformCom->Rotation(0.f, drawnYaw, 0.f);
}

void CNpc::Update_CombatCollider()
{
	if (nullptr == m_pColliderCom || nullptr == m_pTransformCom)
		return;
	matrix_t world = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
#ifdef _DEBUG
	// Presentation tuning must not move or resize the Server-radius mirror.
	if (m_fDebugPresentationScale != 1.f ||
		m_vDebugPresentationOffset.x != 0.f || m_vDebugPresentationOffset.y != 0.f ||
		m_vDebugPresentationOffset.z != 0.f)
	{
		for (size_t axis = 0u; axis < 3u; ++axis)
			world.r[axis] = XMVector3Normalize(world.r[axis]);
		world.r[3] = XMVectorSet(m_vDebugUnadjustedPosition.x,
			m_vDebugUnadjustedPosition.y, m_vDebugUnadjustedPosition.z, 1.f);
	}
#endif
	m_pColliderCom->Update(world);
}

#ifdef _DEBUG
void CNpc::Set_DebugWeaponScale(const f32_t fScale)
{
	if (std::isfinite(fScale) && fScale > 0.f && fScale <= 10000.f)
		m_fDebugWeaponScale = fScale;
}

void CNpc::Set_DebugWeaponRotation(
	const float3_t& vCatalogDegrees, const float3_t& vTargetDegrees)
{
	if (!std::isfinite(vCatalogDegrees.x) || !std::isfinite(vCatalogDegrees.y) ||
		!std::isfinite(vCatalogDegrees.z) || !std::isfinite(vTargetDegrees.x) ||
		!std::isfinite(vTargetDegrees.y) || !std::isfinite(vTargetDegrees.z))
	{
		return;
	}
	const matrix_t catalogRotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(vCatalogDegrees.x), XMConvertToRadians(vCatalogDegrees.y),
		XMConvertToRadians(vCatalogDegrees.z));
	const matrix_t targetRotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(vTargetDegrees.x), XMConvertToRadians(vTargetDegrees.y),
		XMConvertToRadians(vTargetDegrees.z));
	// A pure rotation's transpose is its inverse. R(catalog) * delta then
	// equals R(saved Euler), including mixed axes and Save/Reload in one run.
	XMStoreFloat4x4(&m_DebugWeaponRotation,
		XMMatrixTranspose(catalogRotation) * targetRotation);
}

void CNpc::Set_DebugPresentationScale(const f32_t fScale)
{
	if (!std::isfinite(fScale) || fScale <= 0.f || nullptr == m_pTransformCom)
		return;
	m_fDebugPresentationScale = fScale;
	m_pTransformCom->Scale(fScale, fScale, fScale);
}

void CNpc::Set_DebugPresentationOffset(const float3_t& vOffset)
{
	if (!std::isfinite(vOffset.x) || !std::isfinite(vOffset.y) ||
		!std::isfinite(vOffset.z))
	{
		return;
	}
	m_vDebugPresentationOffset = vOffset;
	if (nullptr != m_pTransformCom)
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(
			m_vDebugUnadjustedPosition.x + vOffset.x,
			m_vDebugUnadjustedPosition.y + vOffset.y,
			m_vDebugUnadjustedPosition.z + vOffset.z, 1.f));
	Update_CombatCollider();
}

void CNpc::Set_DebugPresentationYawOffset(const f32_t fYawOffsetDegrees)
{
	if (!std::isfinite(fYawOffsetDegrees))
		return;
	m_fDebugPresentationYawOffset = fYawOffsetDegrees;
	// Rotation keeps the current scale; the collider re-reads the Server yaw.
	if (nullptr != m_pTransformCom)
		m_pTransformCom->Rotation(0.f,
			m_fDebugUnadjustedYawDegrees + fYawOffsetDegrees, 0.f);
	Update_CombatCollider();
}
#endif

void CNpc::Set_ChargeAfterimageEnabled(const bool enabled, const bool backstep,
    const float previewClockSeconds)
{
    const bool externalClock = std::isfinite(previewClockSeconds) && previewClockSeconds >= 0.f;
    if (externalClock != m_ChargeAfterimageExternalClock ||
        (externalClock && previewClockSeconds < m_ChargeAfterimageClockSeconds))
        Reset_AfterimageHistory();
    m_ChargeAfterimageEnabled = enabled;
    m_ChargeAfterimageExternalClock = externalClock;
    if (externalClock) m_ChargeAfterimageClockSeconds = previewClockSeconds;
    // Keep the source lifetime/style while the last emitted pose fades out.
    if (enabled) m_BackstepAfterimageStyle = backstep;
}

void CNpc::Reset_AfterimageHistory()
{
    m_ChargeAfterimageLastSampleSeconds = -1.f;
    m_BodyAfterimage.Reset();
    m_WeaponAfterimage.Reset();
    m_HatAfterimage.Reset();
}

void CNpc::Set_CounterAfterimageEnabled(const bool enabled, const float previewClockSeconds)
{
    const bool externalClock = std::isfinite(previewClockSeconds) && previewClockSeconds >= 0.f;
    if (enabled != m_CounterAfterimageEnabled || externalClock != m_CounterAfterimageExternalClock)
    {
        Reset_AfterimageHistory();
        m_CounterAfterimageClockSeconds = 0.f;
    }
    m_CounterAfterimageEnabled = enabled;
    m_CounterAfterimageExternalClock = externalClock;
    if (externalClock) m_CounterAfterimageClockSeconds = previewClockSeconds;
}

void CNpc::Late_Update(f32_t fTimeDelta)
{
    if (!Is_PresentationVisible())
    {
        Reset_AfterimageHistory();
        return;
    }
    if (m_bNativeBinaryBasePass)
    {
        CSkeletalAfterimage::SETTINGS settings;
        if (m_BackstepAfterimageStyle)
        {
            // Action 4219951/stage004 TrailGhost: source 5ms samples, 500ms tail.
            // White translucent exposure is requested authoring, not native rim ABI.
            settings.sampleIntervalSeconds = .005f;
            settings.sampleLifetimeSeconds = .5f;
            settings.maxSamples = 64u;
            settings.sourceColorIntensity = 1.f;
            settings.capturePoseChanges = true;
            settings.color = { 1.f, 1.f, 1.f, .38f };
        }
        if (m_CounterAfterimageEnabled)
        {
            // Project-authored counter cue; the live native materials stay intact.
            settings.sampleIntervalSeconds = .24f;
            settings.sampleLifetimeSeconds = .16f;
            settings.maxSamples = 1u;
            settings.color = { .12f, .7f, 2.4f, .7f };
            settings.sourceColorIntensity = 0.f;
        }
        (void)m_BodyAfterimage.Configure(settings);
        (void)m_WeaponAfterimage.Configure(settings);
        const auto& bodyWorld = *m_pTransformCom->Get_WorldMatrixPtr();
        const bool initialPreviewPose = m_ChargeAfterimageExternalClock &&
            m_ChargeAfterimageLastSampleSeconds < 0.f;
        const float afterimageDelta = m_ChargeAfterimageExternalClock ?
            (initialPreviewPose ? 0.f : (std::max)(0.f,
                m_ChargeAfterimageClockSeconds - m_ChargeAfterimageLastSampleSeconds)) : fTimeDelta;
        if (m_ChargeAfterimageExternalClock)
            m_ChargeAfterimageLastSampleSeconds = m_ChargeAfterimageClockSeconds;
        const auto sample = [&](CSkeletalAfterimage& afterimage,
            const std::shared_ptr<CModel>& model, const float4x4_t& world) {
            if (m_CounterAfterimageEnabled)
                afterimage.Sample_Pulse(m_CounterAfterimageClockSeconds, model, world);
            else if (initialPreviewPose && m_ChargeAfterimageEnabled && model && model->Is_Skinned())
            {
                // Capture only the already sampled pose, including time zero.
                // A paused clock neither ages it nor invents missed history.
                CSkeletalAfterimage::MODEL_VIEW view;
                view.model = model; view.world = world;
                (void)afterimage.Capture_Initial(view, 0.f);
            }
            else afterimage.Update(afterimageDelta, m_ChargeAfterimageEnabled, model, world);
        };
        sample(m_BodyAfterimage, m_pModelCom, bodyWorld);
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        if (m_pWeaponModelCom && !CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_pModelCom) &&
            Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView))
            sample(m_WeaponAfterimage, m_pWeaponModelCom, weaponView.BoneRoot);
        else m_WeaponAfterimage.Reset();
        float4x4_t hatWorld;
        if (m_CounterAfterimageEnabled && m_pSaydonHatModel &&
            CNpcPresentationAssetService::Try_GetSaydonHatWorld(m_pModelCom, bodyWorld, hatWorld))
        {
            (void)m_HatAfterimage.Configure(settings);
            sample(m_HatAfterimage, m_pSaydonHatModel, hatWorld);
        }
        else m_HatAfterimage.Reset();
        if (m_CounterAfterimageEnabled && !m_CounterAfterimageExternalClock &&
            std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
            m_CounterAfterimageClockSeconds += fTimeDelta;
        if (m_BodyAfterimage.Has_Samples() || m_WeaponAfterimage.Has_Samples() || m_HatAfterimage.Has_Samples())
            CGameInstance::Get().Add_RenderObject(RENDERGROUP::BLEND,
                static_pointer_cast<CGameObject>(shared_from_this()));
    }
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
#ifdef _DEBUG
	if (m_isCombatColliderDebugVisible && nullptr != m_pColliderCom)
		CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
#endif
}

HRESULT CNpc::Render_Group(const RENDERGROUP group)
{
    if (group != RENDERGROUP::BLEND) return Render();
    if (!Is_PresentationVisible()) return S_OK;
    m_BodyAfterimage.Render(m_pModelCom, m_pShaderCom, *m_pTransformCom->Get_WorldMatrixPtr());
    ANIMATION_MODEL_TARGET_VIEW weaponView;
    if (m_pWeaponModelCom && Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView))
        m_WeaponAfterimage.Render(m_pWeaponModelCom, m_pShaderCom, weaponView.BoneRoot);
    float4x4_t hatWorld;
    if (m_pSaydonHatModel && CNpcPresentationAssetService::Try_GetSaydonHatWorld(
        m_pModelCom, *m_pTransformCom->Get_WorldMatrixPtr(), hatWorld))
        m_HatAfterimage.Render(m_pSaydonHatModel, m_pShaderCom, hatWorld);
    return S_OK; // A failed auxiliary history never suppresses the live actor.
}

HRESULT CNpc::Render()
{
    if (!Is_PresentationVisible()) return S_OK;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const uint32_t iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (uint32_t i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, &m_HitFlash,
				nullptr, m_bNativeBinaryBasePass)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	if (FAILED(CNpcPresentationAssetService::Render_SaydonHat(m_pModelCom, m_pSaydonHatModel,
		m_pShaderCom, *m_pTransformCom->Get_WorldMatrixPtr(), 0u, m_bNativeBinaryBasePass)))
		return E_FAIL;
	if (nullptr != m_pWeaponModelCom && !CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_pModelCom))
	{
		ANIMATION_MODEL_TARGET_VIEW weaponView;
		if (!Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView) ||
			FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &weaponView.BoneRoot)))
			return E_FAIL;
		const uint32_t iNumWeaponMeshes = m_pWeaponModelCom->Get_NumMeshes();
		for (uint32_t i = 0; i < iNumWeaponMeshes; ++i)
		{
			if (FAILED(Bind_DeferredMaterialInputs(
					*m_pWeaponModelCom, m_pShaderCom, i, {}, &m_HitFlash,
					nullptr, m_bNativeBinaryBasePass)) ||
				FAILED(m_pWeaponModelCom->Bind_BoneMatrices(
					m_pShaderCom, "g_BoneMatrices", i)) ||
				FAILED(m_pShaderCom->Begin(0)) ||
				FAILED(m_pWeaponModelCom->Render(i)))
				return E_FAIL;
		}
		if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;
	}
	if (m_fOutlineWidth > 0.f)
	{
		/* Pass 3 of the esther shader: front-culled hull pushed along the
		skinned normal, stencil-tested against the body drawn just above. */
		if (FAILED(m_pShaderCom->Bind_RawValue(
				"g_OutlineWidth", &m_fOutlineWidth, sizeof(m_fOutlineWidth))) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_OutlineColor", &m_vOutlineColor, sizeof(m_vOutlineColor))))
			return E_FAIL;
		for (uint32_t i = 0; i < iNumMeshes; ++i)
		{
			if (FAILED(Bind_DeferredMaterialInputs(
					*m_pModelCom, m_pShaderCom, i, {}, &m_HitFlash)) ||
				FAILED(m_pModelCom->Bind_BoneMatrices(
					m_pShaderCom, "g_BoneMatrices", i)) ||
				FAILED(m_pShaderCom->Begin(3)) ||
				FAILED(m_pModelCom->Render(i)))
				return E_FAIL;
		}
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
	m_bNativeBinaryBasePass =
		pDesc->strShaderTag == TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");

	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strModelTag,
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;
	m_strModelTag = pDesc->strModelTag;
	m_strEffectV2BindingOwner = pDesc->strEffectV2BindingOwner;

	if (!pDesc->strWeaponModelTag.empty() || nullptr != pDesc->pWeaponSocketBone)
	{
		if (pDesc->strWeaponModelTag.empty() ||
			nullptr == pDesc->pWeaponSocketBone ||
			'\0' == pDesc->pWeaponSocketBone[0] ||
			!m_pModelCom->Has_Bone(pDesc->pWeaponSocketBone))
		{
			return E_INVALIDARG;
		}
		if (FAILED(__super::Add_Component(
			pDesc->iPrototypeLevelIndex,
			pDesc->strWeaponModelTag,
			TEXT("Com_WeaponModel"),
			m_pWeaponModelCom)))
			return E_FAIL;
		m_strWeaponSocketBone = pDesc->pWeaponSocketBone;
		// Keep the actual loaded rest pose for body clips without a hammer counterpart.
		matrix_t local;
		for (uint32_t i = 0u; m_pWeaponModelCom->Get_BoneRestLocalMatrix(i, local); ++i)
		{
			float4x4_t stored;
			XMStoreFloat4x4(&stored, local);
			m_WeaponRestPose.push_back(stored);
		}
		m_pWeaponModelCom->Set_AnimPaused(true);
		m_pWeaponModelCom->Refresh_BoneCombinedMatrices();
	}

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
