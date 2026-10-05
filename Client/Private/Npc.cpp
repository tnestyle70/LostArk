#include "Npc.h"
#include "Part_Equipment.h"
#include "MaharakaWaterpangPresentation.h"
#include "KoukuSaydonAnimationBlend.h"
#include "KoukuSaydonCompositionDocument.h"
#include "EffectV2_Runtime.h"
#include "Effect_PresentationService.h"
#include "AnimationTargetService.h"
#include "NpcPresentationAssetService.h"
#include "RuntimeAssetRoot.h"
#include "SoundCueCatalog.h"
#include "CombatHUDViewModel.h"
#include "MonsterPresentationAssetService.h"
#include "KoukuSaydonPresentationAssetService.h"

#include "Collider.h"
#include "HitAreaWire.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "BinaryAsset/ModelAssetData.h"
#include "MapAssetRenderUtils.h"
#include "Profiler.h"
#include "Shader.h"
#include "Transform.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <array>
#include <random>

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
	if (m_iKoukuHitReactionSound) CGameInstance::Get().Stop_SoundCue(m_iKoukuHitReactionSound);
	Release_ActionEffectCues();
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
    m_bAllowOffscreenAnimationCulling = pDesc->bAllowOffscreenAnimationCulling;

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

    if (m_bAllowOffscreenAnimationCulling)
    {
        // Town actors reuse identical cooked channel samples at the same exact time.
        // Their clocks, root suppression, blending and combined palettes stay local.
        m_pModelCom->Enable_AnimationSampleReuse();

        // Scan immutable clip keys during actor creation, not its first active update.
        f32_t envelopeRadius = 0.f;
        (void)Can_DeferAnimationPose(envelopeRadius);
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
    Require_ImmediateAnimationPose();
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
    Require_ImmediateAnimationPose();
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
    Resolve_PendingAnimationPose();
	m_iPendingHitReactionSoundClip = UINT32_MAX;
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
    Resolve_PendingAnimationPose();
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
    Resolve_PendingAnimationPose();
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
    Resolve_PendingAnimationPose();
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
    Resolve_PendingAnimationPose();
	m_iPendingHitReactionSoundClip = UINT32_MAX;
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
	Arm_HitReactionSound(pClipName);
	return true;
}

void CNpc::Arm_ActionEffectCues(const char_t* pClipName)
{
	/* A new action edge is an explicit kill of the previous occurrence: its
	uncut sounds do not belong to this one. */
	Release_ActionEffectCues();
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
#ifdef _DEBUG
	m_pDebugEstherStrike = LostArk::Shared::EstherStrike::Find_ByArchetype(archetype.c_str());
	m_pDebugEstherGuard = LostArk::Shared::EstherStrike::Find_GuardByArchetype(archetype.c_str());
	m_pDebugEstherZone = LostArk::Shared::EstherStrike::Find_ZoneByArchetype(archetype.c_str());
	m_fDebugEstherStrikeAgeSeconds = 0.f;
#endif
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

void CNpc::Arm_HitReactionSound(const char_t* pClipName)
{
	m_iPendingHitReactionSoundClip = UINT32_MAX;
	m_iPendingHitReactionSoundEvent = UINT32_MAX;
	if (!pClipName || !m_pModelCom || m_isNetworkAnimationLoop) return;
	const std::string_view clip(pClipName);
	const auto monster = [&](const std::string_view archetype) {
		return m_strModelTag == CMonsterPresentationAssetService::Get_ModelPrototypeTag(archetype);
	};
	if ((clip == "rpcz00_dmg_idle_1" || clip == "rpcz00_dmg_idle_2") &&
		(m_strModelTag == CKoukuSaydonPresentationAssetService::Get_ModelPrototypeTag("BOSS_KAKULSAYDON_G1_KOUKU") ||
		 m_strModelTag == CKoukuSaydonPresentationAssetService::Get_ModelPrototypeTag("BOSS_KAKULSAYDON_G2_KOUKU")))
		m_iPendingHitReactionSoundEvent = 0u;
	else if (clip == "mn_padd_01_sk.ao_dmg_idle_1" && monster("MONSTER_VALTAN_PADD_01"))
		m_iPendingHitReactionSoundEvent = 1u;
	else if (clip == "mn_sjfc_00_sk.ao_dmg_idle_1" &&
		(monster("MONSTER_VALTAN_SJFC_00_4") || monster("MONSTER_VALTAN_SJFC_ELITE")))
		m_iPendingHitReactionSoundEvent = 2u;
	else if (clip == "mn_0019_05_sk.ao_dmg_idle_1" && monster("MONSTER_VALTAN_0019_05"))
		m_iPendingHitReactionSoundEvent = 3u;
	else if (clip == "dmg_idle_1" && monster("MONSTER_KOUKU_CLOWN_BOX"))
		m_iPendingHitReactionSoundEvent = 4u;
	if (m_iPendingHitReactionSoundEvent != UINT32_MAX)
		m_iPendingHitReactionSoundClip = m_pModelCom->Get_CurrentAnimIndex();
}

void CNpc::Update_HitReactionSound()
{
	const auto& listener = CCombatHUDViewModel::Get().Get_Player();
	const bool muteBoss = listener.isValid && !listener.isPreview && listener.iMarioStage != 0u;
	if (muteBoss && m_iKoukuHitReactionSound)
	{
		CGameInstance::Get().Stop_SoundCue(m_iKoukuHitReactionSound);
		m_iKoukuHitReactionSound = 0u;
	}
	if (m_iPendingHitReactionSoundClip == UINT32_MAX) return;
	struct SOUND_NOTIFY final { const char* soundClass; const char* event; uint32_t startMs; };
	static constexpr std::array<SOUND_NOTIFY, 5u> notifies = {{
		{"KoukuSaydon", "s_mob_g_kouku1.g_kouku1_damage1", 1u},
		{"Valtan", "Retch_Voice_Damage1", 10u},
		{"Valtan", "FlameKerberos_Damage1", 10u},
		{"Valtan", "Troll1_Damage1", 1u},
		{"KoukuSaydon", "s_mob_c01.clownbox1_damage1", 1u}
	}};
	const uint32_t clip = m_iPendingHitReactionSoundClip;
	const uint32_t event = m_iPendingHitReactionSoundEvent;
	if (muteBoss && event == 0u)
	{ m_iPendingHitReactionSoundClip = m_iPendingHitReactionSoundEvent = UINT32_MAX; return; }
	const auto cancel = [&]() { m_iPendingHitReactionSoundClip = UINT32_MAX;
		m_iPendingHitReactionSoundEvent = UINT32_MAX; };
	if (event >= notifies.size() || !m_pModelCom || !Is_PresentationVisible() ||
		m_pModelCom->Get_CurrentAnimIndex() != clip ||
		(m_bNetworkAnimationWindow && m_iNetworkAnimationSourceStartMs > notifies[event].startMs))
	{ cancel(); return; }
	float position = 0.f, duration = 0.f;
	const float ticksPerSecond = m_pModelCom->Get_AnimationTickPerSecond(clip);
	if (!std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f ||
		!m_pModelCom->Get_AnimationProgress(clip, position, duration) ||
		!std::isfinite(position) || !std::isfinite(duration) || duration <= 0.f)
	{ cancel(); return; }
	if (position / ticksPerSecond < notifies[event].startMs * .001f) return;
	// Consume before lookup/play: a missing resource never retries each frame.
	cancel();
	const auto& variants = CSoundCueCatalog::Find_Variants(notifies[event].soundClass, notifies[event].event);
	if (variants.empty()) return;
	// All five native event containers are global, equal-weight, avoid-repeat 1.
	// Asset IDs survive catalog reordering and shrinking without stale indices.
	static std::array<std::string, notifies.size()> previousAssets;
	auto& previousAsset = previousAssets[event];
	std::vector<size_t> eligible;
	for (size_t i = 0u; i < variants.size(); ++i)
		if (variants[i] != previousAsset) eligible.push_back(i);
	if (eligible.empty())
		for (size_t i = 0u; i < variants.size(); ++i) eligible.push_back(i);
	static std::mt19937 random(std::random_device{}());
	std::uniform_int_distribution<size_t> choose(0u, eligible.size() - 1u);
	const size_t variant = eligible[choose(random)];
	previousAsset = variants[variant];
	const auto path = CRuntimeAssetRoot::Resolve(variants[variant]);
	if (event == 0u)
	{
		if (m_iKoukuHitReactionSound) CGameInstance::Get().Stop_SoundCue(m_iKoukuHitReactionSound);
		m_iKoukuHitReactionSound = CGameInstance::Get().Play_SoundCue(path.wstring(), 1.f);
	}
	else (void)CGameInstance::Get().Play_Sound(path.wstring(), 1.f);
}

void CNpc::Release_ActionEffectCues()
{
	for (const NPC_ACTION_EFFECT_LIVE_CUE& live : m_NpcActionEffectState.LiveCues)
	{
		if (0u != live.iSoundHandle)
			CGameInstance::Get().Stop_SoundCue(live.iSoundHandle);
	}
	m_NpcActionEffectState.LiveCues.clear();
}

void CNpc::Play_ActionEffectCueSound(const NPC_ACTION_EFFECT_CUE& cue,
	const f32_t fDueSeconds, const f32_t fAgeSeconds,
	NPC_ACTION_EFFECT_LIVE_CUE& live)
{
	/* The source chooses between two events by the actor's DLChar. The model
	tag carries that identity here, so the alternate event wins only when the
	tag ends with the recorded suffix. Exactly one of the two ever plays. */
	const std::string* pEvent = &cue.strSoundEvent;
	if (!cue.strSoundEventAlternate.empty() &&
		!cue.strSoundAlternateModelTagSuffix.empty())
	{
		std::wstring wide;
		wide.reserve(cue.strSoundAlternateModelTagSuffix.size());
		for (const char_t character : cue.strSoundAlternateModelTagSuffix)
		{
			wide.push_back(static_cast<wchar_t>(
				static_cast<unsigned char>(character)));
		}
		if (m_strModelTag.size() >= wide.size() && !wide.empty() &&
			0 == m_strModelTag.compare(m_strModelTag.size() - wide.size(),
				wide.size(), wide))
		{
			pEvent = &cue.strSoundEventAlternate;
		}
	}
	const auto& variants =
		CSoundCueCatalog::Find_Variants(cue.strSoundClass, *pEvent);
	if (variants.empty())
		return;
	/* Equal weight, avoid repeat 1, keyed by asset id so catalog reordering
	never leaves a stale index behind. */
	std::vector<size_t> eligible;
	for (size_t i = 0u; i < variants.size(); ++i)
	{
		if (variants[i] != m_strLastActionEffectSoundAsset)
			eligible.push_back(i);
	}
	if (eligible.empty())
	{
		for (size_t i = 0u; i < variants.size(); ++i)
			eligible.push_back(i);
	}
	static std::mt19937 random(std::random_device{}());
	std::uniform_int_distribution<size_t> choose(0u, eligible.size() - 1u);
	const size_t variant = eligible[choose(random)];
	m_strLastActionEffectSoundAsset = variants[variant];
	/* A frame that overshot the notify starts the cue already aged, so a late
	join never replays a one-shot from its beginning. */
	const f32_t fClampedAge = std::clamp(fAgeSeconds, 0.f, 600.f);
	const uint32_t iAgeMs = static_cast<uint32_t>(fClampedAge * 1000.f);
	const auto path = CRuntimeAssetRoot::Resolve(variants[variant]);
	live.iSoundHandle =
		CGameInstance::Get().Play_SoundCue(path.wstring(), 1.f, iAgeMs);
	/* The source duration is min(media length, action remainder): a positive
	value cuts the sound at the action edge instead of letting it run on. */
	if (0u != live.iSoundHandle && 0u != cue.iDurationMs)
	{
		live.fSoundStopAtSeconds =
			fDueSeconds + static_cast<f32_t>(cue.iDurationMs) * 0.001f;
	}
}

void CNpc::Update_ActionEffectCues(const f32_t fTimeDelta)
{
	if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
		return;
	const bool_t bCursorActive =
		m_NpcActionEffectState.bActive && nullptr != m_pTransformCom;
	/* A sound cut age can fall after the last notify of the clip, so the clock
	keeps running while any stop is still pending even once the cursor is
	exhausted. Without this the final sound would never be cut. */
	bool_t bPendingStop = false;
	for (const NPC_ACTION_EFFECT_LIVE_CUE& live : m_NpcActionEffectState.LiveCues)
	{
		if (0u != live.iSoundHandle && live.fSoundStopAtSeconds > 0.f)
			bPendingStop = true;
	}
	if (!bCursorActive && !bPendingStop)
		return;
	m_NpcActionEffectState.fElapsedSeconds += fTimeDelta;
	for (NPC_ACTION_EFFECT_LIVE_CUE& live : m_NpcActionEffectState.LiveCues)
	{
		if (0u == live.iSoundHandle || live.fSoundStopAtSeconds <= 0.f ||
			m_NpcActionEffectState.fElapsedSeconds < live.fSoundStopAtSeconds)
		{
			continue;
		}
		CGameInstance::Get().Stop_SoundCue(live.iSoundHandle);
		live.iSoundHandle = 0u;
		live.fSoundStopAtSeconds = 0.f;
	}
	if (!bCursorActive)
		return;
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
		/* Catch a cue the frame overshot so a long frame still starts it at
		its authored phase instead of from zero. */
		const f32_t fAge = m_NpcActionEffectState.fElapsedSeconds - fDue;
		NPC_ACTION_EFFECT_LIVE_CUE live;
		if (!cue.strEffectAssetId.empty())
		{
			EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
			desc.iLevelIndex = iLevel;
			desc.strPlacementId = cue.strCueId + ":" +
				std::to_string(m_iNpcActionEffectOccurrence);
			desc.strEffectAssetId = cue.strEffectAssetId;
			desc.RootWorld = *m_pTransformCom->Get_WorldMatrixPtr();
			desc.pAnchorOwner = static_pointer_cast<CNpc>(shared_from_this());
			desc.fInitialSampleTimeSeconds = fAge;
			EFFECT_WORLD_ROOT_HANDLE handle;
			std::string status;
			if (!CEffectPresentationService::Spawn_LevelPlacement(
				desc, handle, status))
			{
				OutputDebugStringA(
					("[Npc] Action Effect cue isolated: " + cue.strCueId + " " +
					 status + "\n").c_str());
			}
		}
		if (!cue.strSoundEvent.empty())
			Play_ActionEffectCueSound(cue, fDue, fAge, live);
		/* Only a cue that actually started something is tracked, so teardown
		never stops a handle it does not own. */
		if (0u != live.iSoundHandle)
			m_NpcActionEffectState.LiveCues.push_back(live);
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
	m_HitFlash.vColor = float4_t(1.f, 0.72f, 0.08f, 1.f);
	m_HitFlash.fIntensity = HIT_FLASH_PEAK_INTENSITY;
	m_HitFlash.usesSurfaceDetailMask = true;
}

void CNpc::Priority_Update(f32_t fTimeDelta)
{
}

void CNpc::Update(f32_t fTimeDelta)
{
	Engine::CProfiler* profiler = CGameInstance::Get().Get_Profiler();
	Engine::CProfilerWorkScope workScope(profiler, Engine::EProfilerWork::NpcUpdate);
	// Authored/composition visibility only; this is not a camera-frustum result.
	if (profiler && !Is_PresentationVisible())
		profiler->Add_Counter(Engine::EProfilerCounter::NpcAuthoredHiddenUpdates);
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
    f32_t envelopeRadius = 0.f;
    if (Can_DeferAnimationPose(envelopeRadius))
    {
        // Advance only playback/blend clocks. The final camera decides whether
        // this frame needs channel sampling, bone combination and GPU palettes.
        (void)m_pModelCom->Advance_AnimationClock(frameDelta, m_bSuppressRootMotion);
        m_bPendingAnimationPose = true;
    }
    else if (m_bNetworkAnimationWindow)
    {
        Resolve_PendingAnimationPose();
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
        Resolve_PendingAnimationPose();
        m_pModelCom->Update_Animation(frameDelta);
    }
    else
    {
        Resolve_PendingAnimationPose();
        m_pModelCom->Play_Animation(frameDelta);
    }
    if (!m_bPendingAnimationPose) Synchronize_WeaponPose();
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
	Update_HitReactionSound();
	CEffectV2Runtime::Tick(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		m_pDevice, m_pContext);
	if (m_fHitFlashRemainingSeconds > 0.f)
	{
		m_fHitFlashRemainingSeconds -= fTimeDelta;
		if (m_fHitFlashRemainingSeconds <= 0.f)
		{
			m_fHitFlashRemainingSeconds = 0.f;
			m_HitFlash.isEnabled = false;
			m_HitFlash.fIntensity = 0.f;
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
    const float previewClockSeconds, const CSkeletalAfterimage::SETTINGS* sourceSettings)
{
    const bool externalClock = std::isfinite(previewClockSeconds) && previewClockSeconds >= 0.f;
    if (externalClock != m_ChargeAfterimageExternalClock ||
        (externalClock && previewClockSeconds < m_ChargeAfterimageClockSeconds))
        Reset_AfterimageHistory();
    m_ChargeAfterimageEnabled = enabled;
    m_ChargeAfterimageExternalClock = externalClock;
    if (externalClock) m_ChargeAfterimageClockSeconds = previewClockSeconds;
    // Keep the source lifetime/style while the last emitted pose fades out.
    if (enabled)
    {
        m_BackstepAfterimageStyle = backstep;
        m_HasSourceAfterimageSettings = sourceSettings && sourceSettings->sourceChannels;
        if (m_HasSourceAfterimageSettings) m_SourceAfterimageSettings = *sourceSettings;
    }
}

void CNpc::Reset_AfterimageHistory()
{
    m_ChargeAfterimageLastSampleSeconds = -1.f;
    m_SourceAfterimageOwnsHistory = false;
    m_BodyAfterimage.Reset();
    m_WeaponAfterimage.Reset();
    m_HatAfterimage.Reset();
}

void CNpc::Set_CounterAfterimageEnabled(const bool enabled, const float previewClockSeconds)
{
    const bool externalClock = std::isfinite(previewClockSeconds) && previewClockSeconds >= 0.f;
    if (enabled != m_CounterAfterimageEnabled || externalClock != m_CounterAfterimageExternalClock)
    {
        // Source TrailGhost children outlive their emission and counter flag.
        // A counter transition cannot erase a still-owned native child history.
        if (!m_SourceAfterimageOwnsHistory) Reset_AfterimageHistory();
        else m_HatAfterimage.Reset();
        m_CounterAfterimageClockSeconds = 0.f;
    }
    m_CounterAfterimageEnabled = enabled;
    m_CounterAfterimageExternalClock = externalClock;
    if (externalClock) m_CounterAfterimageClockSeconds = previewClockSeconds;
}

bool_t CNpc::Can_DeferAnimationPose(f32_t& radius) const
{
    if (!m_bAllowOffscreenAnimationCulling || m_bExternalPoseConsumer ||
        !m_pModelCom || !m_pTransformCom || !m_bNativeBinaryBasePass ||
        m_pWeaponModelCom || m_pSaydonHatModel || m_fOutlineWidth != 0.f ||
        m_bNetworkAnimationWindow || m_bNetworkAnimationTransition ||
        !m_NetworkAnimationBlendWindows.empty() || m_PlayerHandGripLocalOffset ||
        !m_strEffectV2BindingOwner.empty() || !m_strNpcActionEffectArchetype.empty() ||
        !m_strClipEndEffect.empty() || m_iPendingHitReactionSoundClip != UINT32_MAX ||
        m_ChargeAfterimageEnabled || m_CounterAfterimageEnabled ||
        m_BodyAfterimage.Has_Samples() || m_WeaponAfterimage.Has_Samples() ||
        m_HatAfterimage.Has_Samples()) return false;
    if (CEffectV2Runtime::Requires_CurrentPose(this)) return false;
    // The geometric envelope does not claim to enclose arbitrary native
    // shader vertex displacement. Bern's town models use the legacy skin VS.
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
        if (surface && surface->family != Engine::MODEL_SURFACE_FAMILY::LEGACY) return false;
    }
    return m_pModelCom->Try_GetAnimationEnvelopeRadius(radius);
}

void CNpc::Resolve_PendingAnimationPose()
{
    if (!m_bPendingAnimationPose || !m_pModelCom) return;
    m_bPendingAnimationPose = false;
    // The clock already contains every hidden frame. Evaluate once at that
    // time; never replay hidden frames or advance the clip a second time.
    (void)m_pModelCom->Play_Animation(0.f);
    Synchronize_WeaponPose();
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
        profiler->Add_Counter(Engine::EProfilerCounter::NpcDeferredPoseEvaluations);
}

void CNpc::Require_ImmediateAnimationPose() const
{
    // Socket/effect/tool consumers can need bones before the final camera.
    // Keep their actor on the original eager path for the rest of its lifetime.
    m_bExternalPoseConsumer = true;
    const_cast<CNpc*>(this)->Resolve_PendingAnimationPose();
}

bool_t CNpc::Is_AnimationEnvelopeOutsideCamera(const f32_t radius) const
{
    if (!m_pTransformCom || !std::isfinite(radius) || radius <= 0.f) return false;
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    if (!camera) return false;
    const auto& world = *m_pTransformCom->Get_WorldMatrixPtr();
    for (const auto& row : world.m)
        for (const float value : row)
            if (!std::isfinite(value)) return false;
    if (world._14 != 0.f || world._24 != 0.f || world._34 != 0.f || world._44 != 1.f) return false;
    // sqrt(||A A^T||_infinity) bounds the spectral norm, including shear.
    double normSquared = 0.0;
    for (size_t row = 0u; row < 3u; ++row)
    {
        double sum = 0.0;
        for (size_t other = 0u; other < 3u; ++other)
        {
            double dot = 0.0;
            for (size_t axis = 0u; axis < 3u; ++axis)
                dot += double(world.m[row][axis]) * world.m[other][axis];
            sum += std::abs(dot);
        }
        normSquared = (std::max)(normSquared, sum);
    }
    const double worldRadius = double(radius) * std::sqrt(normSquared) * 1.00001 + 0.001;
    if (!std::isfinite(worldRadius) || worldRadius <= 0.0 || worldRadius > 1.e6) return false;
    MAP_FRUSTUM_CULLING_POLICY policy;
    policy.baseMargin = 0.f;
    policy.largeObjectRadiusThreshold = 0.f;
    policy.largeObjectAbsoluteMargin = 0.f;
    policy.largeObjectRelativeMargin = 0.f;
    MAP_FRUSTUM_RUNTIME_STATE state;
    MAP_FRUSTUM_CULL_DECISION decision;
    const float3_t center(world._41, world._42, world._43);
    return CMapAssetRenderUtils::Evaluate_FrustumVisibility(policy, *camera,
        {}, {}, 0u, center, static_cast<f32_t>(worldRadius), state, decision) &&
        !decision.shouldRender;
}

void CNpc::Submit_FinalCamera()
{
    if (!m_bDeferredLateUpdate) return;
    m_bDeferredLateUpdate = false;
    auto* profiler = CGameInstance::Get().Get_Profiler();
    // The camera and transform are current here. Recheck admission because an
    // external action/tool may have changed this actor after normal Update.
    f32_t radius = 0.f;
    const bool_t admitted = Can_DeferAnimationPose(radius);
    if (admitted && !Is_PresentationVisible()) return;
    if (admitted && profiler)
        profiler->Add_Counter(Engine::EProfilerCounter::NpcCullingCandidates);
    if (admitted && Is_AnimationEnvelopeOutsideCamera(radius))
    {
        if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::NpcCulled);
        return;
    }
    Resolve_PendingAnimationPose();
    m_bFinalCameraSubmission = true;
    Late_Update(m_fDeferredLateDelta);
    m_bFinalCameraSubmission = false;
}

void CNpc::Late_Update(f32_t fTimeDelta)
{
	Engine::CProfilerWorkScope workScope(
		CGameInstance::Get().Get_Profiler(), Engine::EProfilerWork::NpcLateUpdate);
    // A failed/skipped render may leave the previous frame unsubmitted.
    // This frame's normal Late_Update owns a fresh submission decision.
    m_bDeferredLateUpdate = false;
    if (m_bPendingAnimationPose && !m_bFinalCameraSubmission)
    {
        m_bDeferredLateUpdate = true;
        m_fDeferredLateDelta = fTimeDelta;
        return;
    }
    if (!Is_PresentationVisible())
    {
        m_HitFlash.isCombatHovered = false;
        Reset_AfterimageHistory();
        return;
    }
    if (m_pWaterGun)
    {
        m_pWaterGun->Set_Visible(m_bWaterGunArmed);
        if (m_bWaterGunArmed)
        {
            Require_ImmediateAnimationPose();
            m_pWaterGun->Update(fTimeDelta);
            m_pWaterGun->Late_Update(fTimeDelta);
        }
    }
    if (m_bNativeBinaryBasePass)
    {
        const bool useSourceTrail = m_HasSourceAfterimageSettings &&
            (m_ChargeAfterimageEnabled || (m_SourceAfterimageOwnsHistory &&
                (m_BodyAfterimage.Has_Samples() || m_WeaponAfterimage.Has_Samples())));
        m_SourceAfterimageOwnsHistory = useSourceTrail;
        const bool useCounterPulse = m_CounterAfterimageEnabled && !useSourceTrail;
        CSkeletalAfterimage::SETTINGS settings;
        // Saydon's owner path uses translucent exposure; other model histories
        // retain their shared defaults and native material reflection inputs.
        settings.color = { .8f, .8f, .8f, .38f };
        settings.endColor = { .8f, .8f, .8f, 0.f };
        if (m_BackstepAfterimageStyle)
        {
            // Action 4219951/stage004 TrailGhost: source 5ms samples, 500ms tail.
            // White translucent exposure is requested authoring, not native rim ABI.
            settings.sampleIntervalSeconds = .005f;
            settings.sampleLifetimeSeconds = .5f;
            settings.maxSamples = 64u;
            settings.sourceColorIntensity = .5f;
            settings.capturePoseChanges = true;
            settings.color = { .5f, .5f, .5f, .19f };
            settings.endColor = { .5f, .5f, .5f, 0.f };
        }
        if (useSourceTrail) settings = m_SourceAfterimageSettings;
        if (useCounterPulse)
        {
            settings = {}; // Counter remains a separate PROJECT_AUTHORED pulse.
            // Project-authored counter cue; the live native materials stay intact.
            settings.sampleIntervalSeconds = .24f;
            settings.sampleLifetimeSeconds = .16f;
            settings.maxSamples = 1u;
            settings.color = { .05f, .3f, 1.f, .70f };
            settings.endColor = { .05f, .3f, 1.f, 0.f };
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
            if (useCounterPulse)
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
        if (useCounterPulse && m_pSaydonHatModel &&
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
    if (m_bNativeBinaryBasePass && m_HitFlash.isCombatHovered)
        CGameInstance::Get().Add_RenderObject(RENDERGROUP::DEFERRED_OVERLAY,
            static_pointer_cast<CGameObject>(shared_from_this()));
	if (m_isCombatColliderDebugVisible && nullptr != m_pColliderCom)
		CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
#ifdef _DEBUG
	Draw_EstherStrikeDebug(fTimeDelta);
#endif
}

#ifdef _DEBUG
void CNpc::Draw_EstherStrikeDebug(const f32_t fTimeDelta)
{
	if ((nullptr == m_pDebugEstherStrike && nullptr == m_pDebugEstherGuard && nullptr == m_pDebugEstherZone) ||
		nullptr == m_pTransformCom)
		return;
	m_fDebugEstherStrikeAgeSeconds += fTimeDelta;
	if (!m_isSkillHitAreaDebugVisible)
		return;
	constexpr uint32_t ESTHER_HIT_COLOR_RGBA = 255u | (70u << 8) | (60u << 16) | (255u << 24);
	constexpr uint32_t ESTHER_GUARD_COLOR_RGBA = 80u | (220u << 8) | (120u << 16) | (255u << 24);
	constexpr f32_t VISIBLE_WINDOW_MS = 300.f;
	constexpr f32_t GUARD_VISIBLE_WINDOW_MS = 1000.f;
	const f32_t fAgeMs = m_fDebugEstherStrikeAgeSeconds * 1000.f;
	const float4x4_t& World = *m_pTransformCom->Get_WorldMatrixPtr();
	if (nullptr != m_pDebugEstherGuard)
	{
		const f32_t fGrantMs = static_cast<f32_t>(m_pDebugEstherGuard->iGrantTimeMs);
		if (fAgeMs >= fGrantMs && fAgeMs <= fGrantMs + GUARD_VISIBLE_WINDOW_MS)
		{
			HIT_AREA_SHAPE Shape{};
			Shape.iAreaType = LostArk::Shared::EstherStrike::AREA_CIRCLE;
			Shape.iAreaRange = static_cast<int32_t>(std::lround(m_pDebugEstherGuard->fRadiusM * 100.f));
			Shape.iAreaOffsetX = static_cast<int32_t>(std::lround(m_pDebugEstherGuard->fOffsetForwardM * 100.f));
			CHitAreaWire::Draw(World, Shape, ESTHER_GUARD_COLOR_RGBA);
		}
	}
	if (nullptr != m_pDebugEstherZone)
	{
		const f32_t fStartMs = static_cast<f32_t>(m_pDebugEstherZone->iStartMs);
		if (fAgeMs >= fStartMs && fAgeMs <= fStartMs + static_cast<f32_t>(m_pDebugEstherZone->iDurationMs))
		{
			HIT_AREA_SHAPE Shape{};
			Shape.iAreaType = LostArk::Shared::EstherStrike::AREA_CIRCLE;
			Shape.iAreaRange = static_cast<int32_t>(std::lround(m_pDebugEstherZone->fRadiusM * 100.f));
			CHitAreaWire::Draw(World, Shape, ESTHER_GUARD_COLOR_RGBA);
		}
	}
	if (nullptr == m_pDebugEstherStrike)
		return;
	const vector_t vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f),
		XMVectorSetY(XMLoadFloat4x4(&World).r[2], 0.f)));
	for (size_t iHit = 0; iHit < m_pDebugEstherStrike->iHitCount; ++iHit)
	{
		const LostArk::Shared::EstherStrike::HIT& Hit = m_pDebugEstherStrike->pHits[iHit];
		const f32_t fStartMs = static_cast<f32_t>(Hit.iTimeMs);
		if (fAgeMs < fStartMs || fAgeMs > fStartMs + VISIBLE_WINDOW_MS)
			continue;
		HIT_AREA_SHAPE Shape{};
		Shape.iAreaType = Hit.iAreaType;
		Shape.iAreaRange = static_cast<int32_t>(std::lround(Hit.fRangeM * 100.f));
		Shape.iAreaAngle = static_cast<int32_t>(std::lround(Hit.fWidthM * 100.f));
		Shape.iAreaOffsetX = static_cast<int32_t>(std::lround(Hit.fOffsetForwardM * 100.f));
		float4x4_t Root = World;
		Root._41 += XMVectorGetX(vRight) * Hit.fOffsetRightM;
		Root._43 += XMVectorGetZ(vRight) * Hit.fOffsetRightM;
		CHitAreaWire::Draw(Root, Shape, ESTHER_HIT_COLOR_RGBA);
	}
}
#endif

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
	Engine::CProfilerWorkScope workScope(
		CGameInstance::Get().Get_Profiler(), Engine::EProfilerWork::NpcRender);
    Resolve_PendingAnimationPose();
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
    auto hatPresentation = m_HitFlash;
    hatPresentation.isCombatHovered = false;
	if (FAILED(CNpcPresentationAssetService::Render_SaydonHat(m_pModelCom, m_pSaydonHatModel,
		m_pShaderCom, *m_pTransformCom->Get_WorldMatrixPtr(), 0u, m_bNativeBinaryBasePass, false, &hatPresentation)))
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

bool_t CNpc::Try_PickPresentation(const float3_t& origin, const float3_t& direction, f32_t& distance) const
{
    // A picking request consumes the actual pose even when the preceding frame
    // was culled. It does not permanently turn a town actor into a bone owner.
    const_cast<CNpc*>(this)->Resolve_PendingAnimationPose();
    if (!Is_PresentationVisible() || !m_pTransformCom) return false;
    bool hit = false;
    float closest = (std::numeric_limits<float>::max)();
    const auto pick = [&](const shared_ptr<CModel>& model, const float4x4_t& world) {
        float candidate;
        if (model && model->Try_PickCurrentPose(world, origin, direction, candidate) && candidate < closest)
        { closest = candidate; hit = true; }
    };
    const auto& world = *m_pTransformCom->Get_WorldMatrixPtr();
    pick(m_pModelCom, world);
    float4x4_t hatWorld;
    if (!CNpcPresentationAssetService::Is_SaydonHatSuppressed(m_pModelCom) &&
        CNpcPresentationAssetService::Try_GetSaydonHatWorld(m_pModelCom, world, hatWorld))
        pick(m_pSaydonHatModel, hatWorld);
    ANIMATION_MODEL_TARGET_VIEW weapon;
    if (!CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_pModelCom) &&
        Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weapon)) pick(weapon.Model, weapon.BoneRoot);
    if (hit) distance = closest;
    return hit;
}

HRESULT CNpc::Render_DeferredOverlay()
{
    Resolve_PendingAnimationPose();
    if (!Is_PresentationVisible() || !m_bNativeBinaryBasePass || !m_HitFlash.isCombatHovered) return S_OK;
    if (FAILED(Bind_ShaderResources())) return E_FAIL;
    const auto& world = *m_pTransformCom->Get_WorldMatrixPtr();
    float4x4_t hatWorld;
    const bool hatVisible = m_pSaydonHatModel && !CNpcPresentationAssetService::Is_SaydonHatSuppressed(m_pModelCom) &&
        CNpcPresentationAssetService::Try_GetSaydonHatWorld(m_pModelCom, world, hatWorld);
    ANIMATION_MODEL_TARGET_VIEW weapon;
    const bool weaponVisible = m_pWeaponModelCom && !CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_pModelCom) &&
        Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weapon);
    HRESULT result = S_OK;
    // Stamp the complete actor before any dilation, then return bit 0x80. The
    // lower seven stencil bits, scene depth and original body pixels stay intact.
    for (const auto phase : {COMBAT_HOVER_PHASE::MASK, COMBAT_HOVER_PHASE::OUTLINE, COMBAT_HOVER_PHASE::CLEAR})
    {
        const auto render = [&](const shared_ptr<CModel>& model, const float4x4_t& root) {
            if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &root))) { result = E_FAIL; return; }
            for (uint32_t mesh = 0; mesh < model->Get_NumMeshes(); ++mesh)
                if (FAILED(Bind_DeferredMaterialInputs(*model, m_pShaderCom, mesh, {}, &m_HitFlash, nullptr, true)) ||
                    FAILED(model->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", mesh)) ||
                    FAILED(Render_CombatHoverSilhouetteMesh(*model, m_pShaderCom, mesh, phase))) result = E_FAIL;
        };
        render(m_pModelCom, world);
        if (hatVisible) render(m_pSaydonHatModel, hatWorld);
        if (weaponVisible) render(weapon.Model, weapon.BoneRoot);
    }
    (void)m_pShaderCom->Bind_Matrix("g_WorldMatrix", &world);
    return result;
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

bool_t CNpc::Prepare_WaterGun(const uint32_t prototypeLevelIndex)
{
    if (m_pWaterGun) return true;
    if (!m_pModelCom || !m_pTransformCom || !m_pModelCom->Has_Bone("bip001-r-hand")) return false;
    CPart_Equipment::PART_EQUIPMENT_DESC desc{};
    desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    desc.iPrototypeLevelIndex = prototypeLevelIndex;
    desc.strModelTag = CMaharakaWaterpangPresentation::WATER_GUN_PROTOTYPE_TAG;
    desc.strShaderTag = TEXT("Prototype_Component_Shader_VtxMeshBinary");
    desc.pSkeletonModel = m_pModelCom;
    desc.pSocketBoneName = "bip001-r-hand";
    desc.iHiddenMeshMask = 1u << 1; // Same unrestored translucent tank as player water guns.
    auto part = std::dynamic_pointer_cast<CPart_Equipment>(CGameInstance::Get().Clone_Prototype(
        prototypeLevelIndex, TEXT("Prototype_GameObject_Part_Equipment"), &desc));
    // Project attachment measured from installed Guardian watergun_idle: prop3 * inverse(right hand).
    // The eight NPC rigs have a .01 metre bone basis after their respective model pre-scales.
    if (!part || !part->Set_SocketTransform({ .112383735f, .040912395f, -.019941085f },
        { -20.648280f, 41.948665f, 11.908303f })) return false;
    part->Set_Visible(false);
    m_pWaterGun = std::move(part);
    return true;
}
