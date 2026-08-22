#include "Character.h"

#include "AnimationSkillBindingDocument.h"
#include "Collider.h"
#include "Effect_Catalog.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "HitAreaWire.h"
#include "Navigation.h"
#include "Part_Body.h"
#include "Part_Equipment.h"
#include "PlayerSkillCatalog.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
	constexpr f32_t CLIP_BLEND_SECONDS = 0.12f;
	/* The move-goal dead zone makes a held right-click arrive, idle for one
	resend interval, then run again at ~2Hz, so the server honestly reports
	IDLE gaps shorter than the clip blend. Holding RUN through a gap shorter
	than this swallows the flicker; a real stop lands this much later. See
	.md/JS/08-10/2026-08-10_LOSTARK_WARLORD_BONE_CHAIN_RESULT.md section 7. */
	constexpr f32_t LOCOMOTION_IDLE_DELAY_SECONDS = 0.15f;
	/* Fast enough that a deliberate turn onto a skill's aim still lands inside a
	quarter second, slow enough to swallow the per-cell steps of a grid path. */
	constexpr f32_t TURN_DEGREES_PER_SECOND = 720.f;
	/* On since the 08-10 solver rewrite. The old off-by-default spawn-frame
	crash never reproduced after the solve moved to Late_Update; if it returns,
	WER LocalDumps for Client.exe writes to C:\Users\95jus\CrashDumps. See
	.md/JS/08-10/2026-08-10_LOSTARK_CLOTH_SPRINGBONE_REWRITE_RESULT.md. */
	constexpr bool_t BONE_CHAINS_ENABLED = true;
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	constexpr f32_t INTERPOLATION_DELAY_TICKS = 2.f;
	constexpr f32_t PLAYBACK_SNAP_TICKS = 6.f;
	constexpr f32_t PLAYBACK_DRIFT_GAIN = 4.f;
	constexpr f32_t TELEPORT_DISTANCE_SQ = 100.f;
	constexpr f32_t ACTION_SEEK_EPSILON_SECONDS = 0.0001f;
	constexpr uint64_t MAX_EFFECT_CUE_OCCURRENCES_PER_UPDATE = 256u;
	constexpr const char_t* ROOT_MOTION_BONE = "b_root";
	constexpr int32_t ROOT_MOTION_VERTICAL_AXIS = 2;
}

CCharacter::CCharacter(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CContainerObject { pDevice, pContext }
{
}

CCharacter::CCharacter(const CCharacter& Prototype)
	: CContainerObject { Prototype }
{
}

CCharacter::~CCharacter()
{
}

HRESULT CCharacter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCharacter::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<CHARACTER_DESC*>(pArg);

	m_pSpec = pDesc->pSpec;
	m_iPrototypeLevelIndex = pDesc->iPrototypeLevelIndex;
	m_fMoveSpeed = pDesc->fSpeedPerSec > 0.f ?
		pDesc->fSpeedPerSec : 5.f;
	//nickname과 local control 여부 추가
	m_strNickName = pDesc->strNickName;
	m_isLocallyControlled = pDesc->isLocallyControlled;

	if (nullptr != pDesc->pNavigationPrototypeTag)
	{
		m_strNavigationPrototypeTag =
			pDesc->pNavigationPrototypeTag;
	}

	if (nullptr == m_pSpec)
		return E_FAIL;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSet(
			pDesc->vPosition.x,
			pDesc->vPosition.y,
			pDesc->vPosition.z,
			1.f));

	if (FAILED(Ready_Components()) ||
		FAILED(Ready_PartObjects()))
	{
		return E_FAIL;
	}

	/* A missing/corrupt presentation document must not prevent the Character,
	IDLE/RUN, or the Animation Tool repair target from existing. Approved skill
	actions remain valid network state even when their optional clip mapping is
	unavailable. */
	Load_ClipChains();
	Load_EffectCues();
	/* Secondary motion is optional per class: a spec with no chains simply never
	activates the solver. */
	m_BoneChains.Initialize(
		m_pBodyModel, m_pSpec->pBoneChains, m_pSpec->iNumBoneChains);

	// Remote Character는 local keyboard logic를 만들지 않는다.
	if (m_isLocallyControlled &&
		nullptr != m_pSpec->pCreateLogic)
	{
		m_pLogic = m_pSpec->pCreateLogic();
	}

	return S_OK;
}

/* Extraction references may seed authoring, but runtime consumes only the
validated authored document. Animation Tool Save is therefore the single
presentation path used by local and remote Characters. */
bool_t CCharacter::Load_ClipChains()
{
	if (nullptr == m_pSpec->pAssetName)
		return false;
	std::vector<std::string> availableClips;
	availableClips.reserve(m_pBodyModel->Get_NumAnimations());
	for (uint32_t index = 0; index < m_pBodyModel->Get_NumAnimations(); ++index)
	{
		const char_t* clipName = m_pBodyModel->Get_AnimationName(index);
		if (nullptr != clipName)
			availableClips.emplace_back(clipName);
	}

	ANIMATION_SKILL_BINDING_DOCUMENT document;
	std::string status;
	if (!CAnimationSkillBindingDocument::Load(
		m_pSpec->pAssetName,
		m_pSpec->eCharacterClass,
		CPlayerSkillCatalog::Get_Skills(),
		availableClips,
		document,
		status))
	{
		OutputDebugStringA(("Character skill animation load failed: " +
			status + "\n").c_str());
		return false;
	}

	std::vector<CLIP_CHAIN> stagedChains;
	stagedChains.reserve(document.Bindings.size());
	for (const ANIMATION_SKILL_BINDING& binding : document.Bindings)
	{
		const PLAYER_SKILL_DEFINITION* definition =
			CPlayerSkillCatalog::Find_ById(binding.iSkillId);
		if (nullptr == definition)
			return false;
		CLIP_CHAIN chain{};
		chain.iSkillId = static_cast<int32_t>(binding.iSkillId);
		const bool_t isHold =
			LostArk::Shared::PLAYER_SKILL_KIND::HOLD == definition->eSkillKind;
		chain.isServerStaged = isHold ||
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
				definition->eSkillKind ||
			LostArk::Shared::PLAYER_SKILL_KIND::COUNTER ==
				definition->eSkillKind;
		chain.stages.reserve(binding.Stages.size());
		for (const ANIMATION_SKILL_STAGE& stage : binding.Stages)
		{
			CLIP_STAGE stagedStage{};
			stagedStage.clips.reserve(stage.Clips.size());
			for (std::size_t clipIndex = 0; clipIndex < stage.Clips.size(); ++clipIndex)
			{
				const ANIMATION_SKILL_CLIP& clip = stage.Clips[clipIndex];
				/* The hold loop is the middle stage's last clip: stage two runs
				until the release the Server confirms. Earlier clips in the same
				stage are a charge-up chain -- each plays once and hands off at
				its playMs, same as any other multi-clip stage -- so only the
				clip actually held on may loop. */
				const bool_t isHoldLoop =
					isHold && 1u == chain.stages.size() &&
					3u == binding.Stages.size() &&
					clipIndex + 1u == stage.Clips.size();
				stagedStage.clips.push_back(
					{ clip.strClipName, clip.iPlayMs, clip.fPlayRate, isHoldLoop });
			}
			chain.stages.push_back(std::move(stagedStage));
		}
		stagedChains.push_back(std::move(chain));
	}
	if (stagedChains.empty())
		return false;
	if (nullptr != m_pChain)
		m_PendingChains = std::move(stagedChains);
	else
		m_Chains = std::move(stagedChains);
	return true;
}

bool_t CCharacter::Reload_SkillAnimationBindings()
{
	return Load_ClipChains();
}

bool_t CCharacter::Load_EffectCues()
{
	if (nullptr == m_pSpec || nullptr == m_pSpec->pAssetName ||
		nullptr == m_pBodyModel)
		return false;
	std::vector<std::string> clips;
	clips.reserve(m_pBodyModel->Get_NumAnimations());
	for (uint32_t index = 0u; index < m_pBodyModel->Get_NumAnimations(); ++index)
	{
		const char_t* pName = m_pBodyModel->Get_AnimationName(index);
		if (nullptr != pName)
			clips.emplace_back(pName);
	}
	ANIMATION_EFFECT_CUE_DOCUMENT staged;
	std::string status;
	if (!CAnimationEffectCueDocument::Load(
		m_pSpec->pAssetName, clips, staged, status))
	{
		OutputDebugStringA(("Character Effect cue load isolated: " +
			status + "\n").c_str());
		return false;
	}
	if (!staged.UnavailableEffectAssetIds.empty())
	{
		OutputDebugStringA(("Character Effect cue targets isolated: " +
			status + "\n").c_str());
	}

	for (const ANIMATION_EFFECT_CUE& cue : staged.Cues)
	{
		if ("root" != cue.strAnchorSlotId &&
			"skill_target" != cue.strAnchorSlotId &&
			!m_pBodyModel->Has_Bone(cue.strAnchorSlotId.c_str()))
		{
			OutputDebugStringA(("Character Effect cue anchor rejected: " +
				cue.strAnchorSlotId + "\n").c_str());
			return false;
		}
	}
	if (!CEffectPresentationService::Queue_ProductCues(
		staged.Cues, status))
	{
		OutputDebugStringA(("Character Effect cue registration isolated: " +
			status + "\n").c_str());
		return false;
	}
	m_EffectCueDocument = std::move(staged);
	return true;
}

void CCharacter::Reset_EffectCueCursor(
	const std::uint32_t iActionStartTick,
	const f32_t fActionFacingYawDegrees)
{
	m_fPreviousEffectCueStageWallSeconds = -1.f;
	m_iEffectActionStartTick = iActionStartTick;
	m_fEffectActionFacingYawDegrees = fActionFacingYawDegrees;
	m_bHasEffectActionFacingYaw = true;
}

f32_t CCharacter::Get_EffectPlaybackRate() const
{
	if (nullptr == m_pChain || m_iChainStage < 0 ||
		m_iChainStage >= static_cast<int32_t>(m_pChain->stages.size()))
	{
		return 1.f;
	}
	const std::vector<CLIP_STEP>& clips =
		m_pChain->stages[m_iChainStage].clips;
	if (m_iChainStep < 0 ||
		m_iChainStep >= static_cast<int32_t>(clips.size()))
	{
		return 1.f;
	}
	const f32_t fRate = clips[m_iChainStep].playRate;
	return std::isfinite(fRate) && fRate > 0.f ? fRate : 1.f;
}

void CCharacter::Spawn_FallbackEffect(
	const LostArk::Shared::SKILL_ID iSkillId)
{
	const PLAYER_SKILL_DEFINITION* pDefinition =
		CPlayerSkillCatalog::Find_ById(iSkillId);
	if (nullptr == pDefinition || pDefinition->strEffectId.empty() ||
		!CEffectCatalog::Contains(pDefinition->strEffectId) ||
		nullptr == m_pBodyModel)
		return;
	if (nullptr != m_pChain && m_iChainStage >= 0 &&
		m_iChainStage < static_cast<int32_t>(m_pChain->stages.size()))
	{
		const std::vector<CLIP_STEP>& Clips =
			m_pChain->stages[m_iChainStage].clips;
		const bool_t bHasAuthoredCue = std::any_of(
			m_EffectCueDocument.Cues.begin(), m_EffectCueDocument.Cues.end(),
			[&Clips](const ANIMATION_EFFECT_CUE& Cue)
			{
				return std::any_of(
					Clips.begin(), Clips.end(),
					[&Cue](const CLIP_STEP& Clip)
					{
						return Cue.strClipName == Clip.clip;
					});
			});
		if (bHasAuthoredCue)
			return;
	}
	EFFECT_SPAWN_DESC Desc;
	Desc.strEffectAssetId = pDefinition->strEffectId;
	Desc.pOwner = static_pointer_cast<CCharacter>(shared_from_this());
	const bool_t bGroundTarget =
		LostArk::Shared::SKILL_TARGET_INTENT_KIND::GROUND_POINT ==
			pDefinition->eTargetIntent;
	Desc.strAnchorSlotId = bGroundTarget ? "skill_target" : "root";
	Desc.eFollowPolicy = bGroundTarget ?
		EFFECT_FOLLOW_POLICY::SNAPSHOT : EFFECT_FOLLOW_POLICY::FOLLOW;
	Desc.eOrientationPolicy = EFFECT_ORIENTATION_POLICY::ANCHOR;
	Desc.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
	Desc.iActionStartTick = m_iEffectActionStartTick;
	Desc.iCueStartMs = 0u;
	Desc.strOccurrenceId = "fallback:skill:" + std::to_string(iSkillId);
	Desc.fPlaybackRate = Get_EffectPlaybackRate();
	std::string status;
	CEffectPresentationService::Spawn(Desc, status);
}

void CCharacter::Update_EffectCues()
{
	if (nullptr == m_pBodyModel || nullptr == m_pChain ||
		0u == m_iEffectActionStartTick || m_iChainStage < 0 ||
		m_iChainStage >= static_cast<int32_t>(m_pChain->stages.size()))
		return;
	std::vector<ACTION_PRESENTATION_CLIP_TIMING> Timings;
	if (!Build_ActiveStageTimeline(Timings))
		return;
	const std::vector<CLIP_STEP>& Clips =
		m_pChain->stages[m_iChainStage].clips;
	const f32_t fCurrentStageWallSeconds = (std::max)(
		0.f, m_fActionPresentationSeconds);
	const f32_t fPreviousStageWallSeconds =
		m_fPreviousEffectCueStageWallSeconds;
	const std::shared_ptr<CCharacter> Owner =
		static_pointer_cast<CCharacter>(shared_from_this());
	for (std::size_t iCue = 0u;
		iCue < m_EffectCueDocument.Cues.size(); ++iCue)
	{
		const ANIMATION_EFFECT_CUE& Cue = m_EffectCueDocument.Cues[iCue];
		for (std::size_t iClip = 0u; iClip < Clips.size(); ++iClip)
		{
			if (Cue.strClipName != Clips[iClip].clip)
				continue;
			f32_t fSourceDurationSeconds = 0.f;
			f32_t fWallDurationSeconds = 0.f;
			if (!CActionPresentationTimeline::Resolve_ClipDuration(
				Timings[iClip], fSourceDurationSeconds, fWallDurationSeconds))
			{
				continue;
			}
			const f32_t fCueSourceSeconds =
				static_cast<f32_t>(Cue.iStartMs) * 0.001f;
			f32_t fFirstOccurrenceWallSeconds = 0.f;
			if (!CActionPresentationTimeline::Resolve_CueWallOffset(
				Timings, iClip, fCueSourceSeconds, 0u,
				fFirstOccurrenceWallSeconds))
			{
				continue;
			}

			uint64_t iFirstEpoch = 0u;
			uint64_t iLastEpoch = 0u;
			if (Timings[iClip].bLoop)
			{
				if (fCurrentStageWallSeconds <
					fFirstOccurrenceWallSeconds)
				{
					continue;
				}
				if (fPreviousStageWallSeconds >=
					fFirstOccurrenceWallSeconds)
				{
					iFirstEpoch = static_cast<uint64_t>(std::floor(
						(fPreviousStageWallSeconds -
							fFirstOccurrenceWallSeconds) /
						fWallDurationSeconds)) + 1u;
				}
				iLastEpoch = static_cast<uint64_t>(std::floor((std::max)(
					0.f, fCurrentStageWallSeconds -
						fFirstOccurrenceWallSeconds) /
					fWallDurationSeconds));
				if (iFirstEpoch > iLastEpoch)
					continue;
				if (iLastEpoch - iFirstEpoch + 1u >
					MAX_EFFECT_CUE_OCCURRENCES_PER_UPDATE)
				{
					iFirstEpoch = iLastEpoch -
						MAX_EFFECT_CUE_OCCURRENCES_PER_UPDATE + 1u;
				}
			}

			for (uint64_t iEpoch = iFirstEpoch;
				iEpoch <= iLastEpoch; ++iEpoch)
			{
				f32_t fOccurrenceWallSeconds = 0.f;
				if (!CActionPresentationTimeline::Resolve_CueWallOffset(
					Timings, iClip, fCueSourceSeconds, iEpoch,
					fOccurrenceWallSeconds) ||
					fOccurrenceWallSeconds <=
						fPreviousStageWallSeconds ||
					fOccurrenceWallSeconds > fCurrentStageWallSeconds)
				{
					continue;
				}
				const f32_t fInitialSampleSeconds = (std::max)(0.f,
					(fCurrentStageWallSeconds - fOccurrenceWallSeconds) *
						Timings[iClip].fPlayRate);
				const uint32_t iCueDurationMs =
					Cue.iEndMs - Cue.iStartMs;
				if (EFFECT_STOP_POLICY::CUE_END == Cue.eStopPolicy &&
					fInitialSampleSeconds * 1000.f >=
						static_cast<f32_t>(iCueDurationMs))
				{
					continue;
				}

				EFFECT_SPAWN_DESC Desc;
				Desc.strEffectAssetId = Cue.strEffectAssetId;
				Desc.pOwner = Owner;
				Desc.strAnchorSlotId = Cue.strAnchorSlotId;
				Desc.LocalTransform = Cue.LocalTransform;
				Desc.eFollowPolicy = Cue.eFollowPolicy;
				Desc.eOrientationPolicy = Cue.eOrientationPolicy;
				if (EFFECT_ORIENTATION_POLICY::ACTION_FACING ==
					Cue.eOrientationPolicy)
				{
					Desc.bHasActionFacingYaw =
						m_bHasEffectActionFacingYaw;
					Desc.fActionFacingYawDegrees =
						m_fEffectActionFacingYawDegrees;
				}
				Desc.eStopPolicy = Cue.eStopPolicy;
				Desc.iCueDurationMs = iCueDurationMs;
				Desc.iActionStartTick = m_iEffectActionStartTick;
				Desc.iCueStartMs = Cue.iStartMs;
				Desc.strOccurrenceId = "stage:" +
					std::to_string(m_iChainStage) + "/clip:" +
					std::to_string(iClip) + "/cue:" +
					std::to_string(iCue) + "/loop:" +
					std::to_string(iEpoch);
				Desc.fPlaybackRate = Timings[iClip].fPlayRate;
				Desc.fInitialSampleTimeSeconds = fInitialSampleSeconds;
				std::string status;
				CEffectPresentationService::Spawn(Desc, status);

				if (iEpoch == (std::numeric_limits<uint64_t>::max)())
					break;
			}
		}
	}
	m_fPreviousEffectCueStageWallSeconds = fCurrentStageWallSeconds;
}

void CCharacter::Commit_PendingClipChains()
{
	if (nullptr != m_pChain || m_PendingChains.empty())
		return;
	m_Chains = std::move(m_PendingChains);
}

bool_t CCharacter::Start_Clip(const CLIP_STEP& Step)
{
	if (!Set_Animation(Step.clip.c_str(), Step.loop))
		return false;

	m_pBodyModel->Set_AnimTrackPosition(m_pBodyModel->Get_CurrentAnimIndex(), 0.f);
	m_pBodyModel->Set_AnimationSpeed(Step.playRate);
	return true;
}

bool_t CCharacter::Resolve_ClipTiming(
	const CLIP_STEP& Step,
	std::uint32_t& iOutAnimation,
	f32_t& fOutSourceDurationSeconds) const
{
	iOutAnimation = UINT32_MAX;
	fOutSourceDurationSeconds = 0.f;
	if (nullptr == m_pBodyModel || Step.clip.empty() ||
		!std::isfinite(Step.playRate) || Step.playRate <= 0.f)
	{
		return false;
	}
	for (std::uint32_t iAnimation = 0u;
		iAnimation < m_pBodyModel->Get_NumAnimations(); ++iAnimation)
	{
		const char_t* pName = m_pBodyModel->Get_AnimationName(iAnimation);
		if (nullptr != pName && Step.clip == pName)
		{
			iOutAnimation = iAnimation;
			break;
		}
	}
	if (UINT32_MAX == iOutAnimation)
		return false;

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTicksPerSecond =
		m_pBodyModel->Get_AnimationTickPerSecond(iOutAnimation);
	if (!m_pBodyModel->Get_AnimationProgress(
		iOutAnimation, fPosition, fDuration) ||
		!std::isfinite(fDuration) || fDuration <= 0.f ||
		!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f)
	{
		return false;
	}
	fOutSourceDurationSeconds = fDuration / fTicksPerSecond;
	return std::isfinite(fOutSourceDurationSeconds) &&
		fOutSourceDurationSeconds > 0.f;
}

bool_t CCharacter::Build_ActiveStageTimeline(
	std::vector<ACTION_PRESENTATION_CLIP_TIMING>& OutTimings,
	std::vector<std::uint32_t>* pOutAnimations) const
{
	OutTimings.clear();
	if (nullptr != pOutAnimations)
		pOutAnimations->clear();
	if (nullptr == m_pChain || m_iChainStage < 0 ||
		m_iChainStage >= static_cast<int32_t>(m_pChain->stages.size()))
	{
		return false;
	}
	const std::vector<CLIP_STEP>& Clips =
		m_pChain->stages[m_iChainStage].clips;
	OutTimings.reserve(Clips.size());
	if (nullptr != pOutAnimations)
		pOutAnimations->reserve(Clips.size());
	for (const CLIP_STEP& Step : Clips)
	{
		std::uint32_t iAnimation = UINT32_MAX;
		f32_t fModelSourceDurationSeconds = 0.f;
		if (!Resolve_ClipTiming(
			Step, iAnimation, fModelSourceDurationSeconds))
		{
			OutTimings.clear();
			if (nullptr != pOutAnimations)
				pOutAnimations->clear();
			return false;
		}
		OutTimings.push_back({ fModelSourceDurationSeconds,
			Step.playMs, Step.playRate, Step.loop });
		if (nullptr != pOutAnimations)
			pOutAnimations->push_back(iAnimation);
	}
	return !OutTimings.empty();
}

bool_t CCharacter::Seek_ActiveStageForward(const f32_t fActionAgeSeconds)
{
	if (nullptr == m_pChain || nullptr == m_pBodyModel ||
		m_iChainStage < 0 ||
		m_iChainStage >= static_cast<int32_t>(m_pChain->stages.size()) ||
		!std::isfinite(fActionAgeSeconds) || fActionAgeSeconds < 0.f)
	{
		return false;
	}
	const std::vector<CLIP_STEP>& Clips =
		m_pChain->stages[m_iChainStage].clips;
	if (Clips.empty())
		return false;

	std::vector<ACTION_PRESENTATION_CLIP_TIMING> Timings;
	std::vector<std::uint32_t> Animations;
	if (!Build_ActiveStageTimeline(Timings, &Animations))
		return false;
	ACTION_PRESENTATION_SAMPLE Sample;
	if (!CActionPresentationTimeline::Resolve_Sample(
		Timings, fActionAgeSeconds, Sample) ||
		Sample.iClipIndex >= Clips.size() ||
		Sample.iClipIndex >= Animations.size())
	{
		return false;
	}
	const CLIP_STEP& Step = Clips[Sample.iClipIndex];
	const std::uint32_t iAnimation = Animations[Sample.iClipIndex];
	if (!Start_Clip(Step))
		return false;
	const f32_t fTicksPerSecond =
		m_pBodyModel->Get_AnimationTickPerSecond(iAnimation);
	const f32_t fTrackPosition =
		Sample.fClipSourceTimeSeconds * fTicksPerSecond;
	if (!m_pBodyModel->Set_AnimTrackPosition(iAnimation, fTrackPosition))
		return false;
	m_pBodyModel->Play_Animation(0.f);
	m_iChainStep = static_cast<int32_t>(Sample.iClipIndex);
	return true;
}

bool_t CCharacter::Is_ClipFinished() const
{
	if (nullptr == m_pBodyModel)
		return false;

	const uint32_t iAnimation = m_pBodyModel->Get_CurrentAnimIndex();
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	if (!m_pBodyModel->Get_AnimationProgress(
		iAnimation, fPosition, fDuration))
		return false;
	if (fDuration <= 0.f)
		return false;

	f32_t fLimit = fDuration;
	if (nullptr != m_pChain && m_iChainStage >= 0 &&
		m_iChainStage < static_cast<int32_t>(m_pChain->stages.size()) &&
		m_iChainStep >= 0 &&
		m_iChainStep < static_cast<int32_t>(
			m_pChain->stages[m_iChainStage].clips.size()))
	{
		const uint32_t iPlayMs =
			m_pChain->stages[m_iChainStage].clips[m_iChainStep].playMs;
		const f32_t fTicksPerSecond =
			m_pBodyModel->Get_AnimationTickPerSecond(iAnimation);
		if (0u != iPlayMs && std::isfinite(fTicksPerSecond) &&
			fTicksPerSecond > 0.f)
		{
			fLimit = (std::min)(fDuration,
				static_cast<f32_t>(iPlayMs) * 0.001f * fTicksPerSecond);
		}
	}

	/* Same test CAnimation uses to report a non-looping clip as done; the track
	is left past the end rather than clamped. */
	return fPosition >= fLimit;
}

bool_t CCharacter::Play_Skill(
	int32_t iSkillId,
	bool_t isCombo)
{
	if (nullptr != m_pChain)
		return false;

	/* The document has exactly one row per PlayerSkills definition, so a kind
	mismatch is a stale/invalid presentation contract rather than a tripod fallback. */
	const CLIP_CHAIN* pPick = nullptr;
	for (const CLIP_CHAIN& chain : m_Chains)
	{
		if (chain.iSkillId != iSkillId || chain.isServerStaged != isCombo)
			continue;
		pPick = &chain;
		break;
	}

	if (nullptr == pPick || pPick->stages.empty() ||
		!Start_Clip(pPick->stages[0].clips[0]))
	{
		return false;
	}

	m_pChain = pPick;
	m_iChainStage = 0;
	m_iChainStep = 0;
	return true;
}

void CCharacter::Update_Chain()
{
	if (nullptr == m_pChain || !Is_ClipFinished())
		return;

	/* Clips inside one stage always run themselves out: a staged skill's single
	press is what buys that whole stage. */
	const std::vector<CLIP_STEP>& clips =
		m_pChain->stages[m_iChainStage].clips;
	if (m_iChainStep + 1 < static_cast<int32_t>(clips.size()))
	{
		++m_iChainStep;
		Start_Clip(clips[m_iChainStep]);
		return;
	}

	/* A combo holds on its stage's last clip until the server confirms the next
	stage. Every other mode keeps running to the end by itself. */
	if (m_pChain->isServerStaged)
		return;

	/* A user-authored presentation clip can be shorter than the Server action.
	Hold its final pose until the replicated action becomes NONE; otherwise the
	client would return to locomotion before the authoritative duration ends. */
	if (m_iChainStage + 1 >= static_cast<int32_t>(m_pChain->stages.size()))
		return;

	++m_iChainStage;
	m_iChainStep = 0;
	Start_Clip(m_pChain->stages[m_iChainStage].clips[0]);
}

void CCharacter::Update_KnockdownPresentation()
{
	if (KNOCKDOWN_STEP::NONE == m_eKnockdownStep ||
		KNOCKDOWN_STEP::DOWN == m_eKnockdownStep ||
		!Is_ClipFinished())
	{
		return;
	}
	if (KNOCKDOWN_STEP::FALLING == m_eKnockdownStep)
	{
		if (Set_Animation(CHARACTER_ANIM::KNOCKDOWN_LAND, false))
			m_eKnockdownStep = KNOCKDOWN_STEP::LANDING;
		else
			m_eKnockdownStep = KNOCKDOWN_STEP::DOWN;
		return;
	}
	if (KNOCKDOWN_STEP::LANDING == m_eKnockdownStep)
	{
		/* A class without a lying loop holds the land clip's final pose. */
		Set_Animation(CHARACTER_ANIM::DOWN_LOOP, true);
		m_eKnockdownStep = KNOCKDOWN_STEP::DOWN;
		return;
	}
	if (KNOCKDOWN_STEP::STANDUP == m_eKnockdownStep)
	{
		m_eKnockdownStep = KNOCKDOWN_STEP::NONE;
		Set_Animation(
			m_isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
			true);
	}
}

void CCharacter::Update_NetworkTransform(f32_t fTimeDelta)
{
	if (!m_hasNetworkState ||
		nullptr == m_pTransformCom ||
		0 == m_iNetworkSampleCount)
	{
		return;
	}

	const f32_t oldestTick =
		static_cast<f32_t>(m_NetworkSamples[0].iServerTick);
	const f32_t newestTick = static_cast<f32_t>(
		m_NetworkSamples[m_iNetworkSampleCount - 1].iServerTick);

	m_fPlaybackServerTick += fTimeDelta * SERVER_TICK_HZ;

	const f32_t targetTick = newestTick - INTERPOLATION_DELAY_TICKS;
	const f32_t drift = targetTick - m_fPlaybackServerTick;
	if (fabsf(drift) > PLAYBACK_SNAP_TICKS)
	{
		m_fPlaybackServerTick = targetTick;
	}
	else
	{
		m_fPlaybackServerTick +=
			drift * (std::min)(1.f, PLAYBACK_DRIFT_GAIN * fTimeDelta);
	}

	m_fPlaybackServerTick = (std::max)(
		oldestTick,
		(std::min)(newestTick, m_fPlaybackServerTick));

	size_t older = m_iNetworkSampleCount - 1;
	for (size_t i = 0; i + 1 < m_iNetworkSampleCount; ++i)
	{
		if (m_fPlaybackServerTick <=
			static_cast<f32_t>(m_NetworkSamples[i + 1].iServerTick))
		{
			older = i;
			break;
		}
	}

	const NETWORK_TRANSFORM_SAMPLE& from = m_NetworkSamples[older];
	const NETWORK_TRANSFORM_SAMPLE& to = m_NetworkSamples[
		(std::min)(older + 1, m_iNetworkSampleCount - 1)];

	vector_t next = XMVectorSet(
		to.vPosition.x,
		to.vPosition.y,
		to.vPosition.z,
		1.f);
	f32_t targetYawDegrees = to.fYawDegrees;

	if (to.iServerTick > from.iServerTick)
	{
		const f32_t ratio =
			(m_fPlaybackServerTick - static_cast<f32_t>(from.iServerTick)) /
			static_cast<f32_t>(to.iServerTick - from.iServerTick);

		const vector_t fromPosition = XMVectorSet(
			from.vPosition.x,
			from.vPosition.y,
			from.vPosition.z,
			1.f);
		next = XMVectorLerp(fromPosition, next, ratio);

		f32_t yawSpan = to.fYawDegrees - from.fYawDegrees;
		while (yawSpan > 180.f)
			yawSpan -= 360.f;
		while (yawSpan < -180.f)
			yawSpan += 360.f;
		targetYawDegrees = from.fYawDegrees + yawSpan * ratio;
	}

	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSetW(next, 1.f));

	/* The server recomputes yaw per path segment, and a grid path changes
	direction at every cell, so copying the latest value straight in reads as a
	series of snaps. Turning toward it at a fixed rate keeps the server's yaw
	authoritative and only spreads the change over a few frames. */
	f32_t difference = targetYawDegrees - m_fPresentationYawDegrees;
	while (difference > 180.f)
		difference -= 360.f;
	while (difference < -180.f)
		difference += 360.f;

	const f32_t step = TURN_DEGREES_PER_SECOND * fTimeDelta;
	if (fabsf(difference) <= step)
		m_fPresentationYawDegrees = targetYawDegrees;
	else
		m_fPresentationYawDegrees += difference > 0.f ? step : -step;

	m_pTransformCom->Rotation(
		0.f,
		m_fPresentationYawDegrees,
		0.f);
}

shared_ptr<CModel> CCharacter::Get_BodyModel() const
{
	return m_pBodyModel;
}

void CCharacter::Set_Position(fvector_t vPosition)
{
	m_pTransformCom->Set_State(STATE::POSITION, vPosition);
}

bool_t CCharacter::Apply_NetworkState(const float3_t& position, f32_t yawDegrees, bool_t isMoving, std::uint32_t iServerTick)
{
	//client replication이 서버 상태를 character 표현 상태로 전달하는 public 함수이다.
	//character는 S2C_WORLD_SNAPSHOT을 직접 받지 않는다.
	//transform 존재 검사 -> position / yaw 유한성 검사 -> network target position 저장
	//network target yaw 저장 -> m_hasNetworkstate = true -> set locomotion(ismoving)
	if (nullptr == m_pTransformCom ||
		!std::isfinite(position.x) ||
		!std::isfinite(position.y) ||
		!std::isfinite(position.z) ||
		!std::isfinite(yawDegrees))
	{
		return false;
	}
	//network state apply!
	/* The first snapshot is where the character is, not somewhere it turned to,
	so it takes the yaw outright; spawning would otherwise spin into place. */
	bool_t reset = !m_hasNetworkState;
	if (!reset && m_iNetworkSampleCount > 0)
	{
		const NETWORK_TRANSFORM_SAMPLE& newest =
			m_NetworkSamples[m_iNetworkSampleCount - 1];
		const f32_t dx = position.x - newest.vPosition.x;
		const f32_t dy = position.y - newest.vPosition.y;
		const f32_t dz = position.z - newest.vPosition.z;
		if (dx * dx + dy * dy + dz * dz > TELEPORT_DISTANCE_SQ)
			reset = true;
	}

	if (reset)
	{
		m_iNetworkSampleCount = 0;
		m_fPresentationYawDegrees = yawDegrees;
		m_fPlaybackServerTick =
			static_cast<f32_t>(iServerTick) - INTERPOLATION_DELAY_TICKS;
	}
	if (!m_hasNetworkState)
		m_BoneChains.Reset();

	if (m_iNetworkSampleCount > 0 &&
		m_NetworkSamples[m_iNetworkSampleCount - 1].iServerTick >= iServerTick)
	{
		m_NetworkSamples[m_iNetworkSampleCount - 1].vPosition = position;
		m_NetworkSamples[m_iNetworkSampleCount - 1].fYawDegrees = yawDegrees;
	}
	else
	{
		if (NETWORK_SAMPLE_CAPACITY == m_iNetworkSampleCount)
		{
			for (size_t i = 1; i < NETWORK_SAMPLE_CAPACITY; ++i)
				m_NetworkSamples[i - 1] = m_NetworkSamples[i];
			--m_iNetworkSampleCount;
		}
		m_NetworkSamples[m_iNetworkSampleCount].iServerTick = iServerTick;
		m_NetworkSamples[m_iNetworkSampleCount].vPosition = position;
		m_NetworkSamples[m_iNetworkSampleCount].fYawDegrees = yawDegrees;
		++m_iNetworkSampleCount;
	}
	m_hasNetworkState = true;

	Set_Locomotion(isMoving);
	return true;
}

bool_t CCharacter::Advance_ComboStage(const std::uint8_t comboStage)
{
	if (nullptr == m_pChain || 0u == comboStage)
		return false;
	const int32_t stage = static_cast<int32_t>(comboStage) - 1;
	if (stage >= static_cast<int32_t>(m_pChain->stages.size()))
		return false;
	m_iChainStage = stage;
	m_iChainStep = 0;
	return Start_Clip(m_pChain->stages[stage].clips[0]);
}

bool_t CCharacter::Apply_NetworkAction(
	const LostArk::Shared::PLAYER_ACTION_STATE action,
	const LostArk::Shared::SKILL_ID skillId,
	const std::uint32_t serverTick,
	const std::uint32_t actionStartTick,
	const f32_t actionFacingYawDegrees,
	const std::uint8_t comboStage,
	const bool_t hasSkillTarget,
	const float3_t& skillTarget)
{
	using namespace LostArk::Shared;
	if (0u == serverTick || !std::isfinite(actionFacingYawDegrees) ||
		static_cast<std::uint8_t>(action) >=
		static_cast<std::uint8_t>(PLAYER_ACTION_STATE::END))
	{
		return false;
	}
	const PLAYER_SKILL_DEFINITION* targetDefinition =
		PLAYER_ACTION_STATE::SKILL == action ?
		CPlayerSkillCatalog::Find_ById(skillId) : nullptr;
	const bool_t expectsGroundTarget = nullptr != targetDefinition &&
		SKILL_TARGET_INTENT_KIND::GROUND_POINT ==
			targetDefinition->eTargetIntent;
	if (expectsGroundTarget != hasSkillTarget ||
		(hasSkillTarget && (!std::isfinite(skillTarget.x) ||
			!std::isfinite(skillTarget.y) || !std::isfinite(skillTarget.z))))
	{
		return false;
	}
	if (hasSkillTarget && m_hasNetworkSkillTarget &&
		m_eNetworkAction == action &&
		m_iLastNetworkActionStartTick == actionStartTick &&
		(std::abs(m_NetworkSkillTarget.x - skillTarget.x) > 0.001f ||
		 std::abs(m_NetworkSkillTarget.y - skillTarget.y) > 0.001f ||
		 std::abs(m_NetworkSkillTarget.z - skillTarget.z) > 0.001f))
	{
		return false;
	}
	if (PLAYER_ACTION_STATE::SKILL == action)
	{
		if (INVALID_SKILL_ID == skillId || 0u == actionStartTick)
			return false;
		f32_t fActionAgeSeconds = 0.f;
		if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
			serverTick, actionStartTick, SERVER_TICK_HZ,
			fActionAgeSeconds))
			return false;
		const bool_t bSameActionEdge = m_eNetworkAction == action &&
			m_iLastNetworkActionStartTick == actionStartTick;
		if (bSameActionEdge)
		{
			if (m_iCurrentEffectSkillId != skillId)
				return false;
			const int32_t iExpectedStage = 0u == comboStage ?
				0 : static_cast<int32_t>(comboStage) - 1;
			/* A staged Server transition owns a new start tick. Never restart a
				running stage when a repeated edge arrives. */
			if (nullptr != m_pChain && m_iChainStage != iExpectedStage)
				return false;
			if (nullptr != m_pChain &&
				fActionAgeSeconds > m_fActionPresentationSeconds +
					ACTION_SEEK_EPSILON_SECONDS)
			{
				if (!Seek_ActiveStageForward(fActionAgeSeconds))
					return false;
				m_fActionPresentationSeconds = fActionAgeSeconds;
			}
			return true;
		}
		/* A newer authoritative edge replaces an ACTIVE action even when the NONE
		snapshot between them was dropped. Clearing before the pending document
		commit also makes Animation Tool live reload pointer-safe. */
		m_pChain = nullptr;
		m_iChainStage = 0;
		m_iChainStep = 0;
		m_eKnockdownStep = KNOCKDOWN_STEP::NONE;
		Commit_PendingClipChains();
		m_iCurrentEffectSkillId = skillId;
		Reset_EffectCueCursor(actionStartTick, actionFacingYawDegrees);
		/* A presentation fallback may resolve its anchor synchronously below.
		Commit the already-validated Server target before that spawn so even a
		missing clip stays rooted at the authoritative point, never the caster. */
		m_hasNetworkSkillTarget = hasSkillTarget;
		m_NetworkSkillTarget = hasSkillTarget ? skillTarget : float3_t{};

		bool_t presented = Play_Skill(
			static_cast<int32_t>(skillId),
			comboStage > 0u);
		if (presented && comboStage > 1u)
			presented = Advance_ComboStage(comboStage);
		if (presented)
			presented = Seek_ActiveStageForward(fActionAgeSeconds);
		m_fActionPresentationSeconds = presented ?
			fActionAgeSeconds : 0.f;
		if (!presented)
		{
			/* Presentation data is not network authority. Record this edge once and
			keep transform/HUD/other entities consuming the snapshot. */
			OutputDebugStringA((
				"Character skill presentation unavailable for skill " +
				std::to_string(skillId) + " at action tick " +
				std::to_string(actionStartTick) + "\n").c_str());
			m_pChain = nullptr;
			m_iChainStage = 0;
			m_iChainStep = 0;
			m_fActionPresentationSeconds = 0.f;
			Set_Animation(
				m_isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
				true);
		}
		Spawn_FallbackEffect(skillId);
		m_iLastNetworkActionStartTick = actionStartTick;
	}
	else if (PLAYER_ACTION_STATE::TRIGGER_MOVE == action)
	{
		if (INVALID_SKILL_ID != skillId || 0u == actionStartTick)
			return false;
		if (m_eNetworkAction == action &&
			m_iLastNetworkActionStartTick == actionStartTick)
		{
			return true;
		}
		if (PLAYER_ACTION_STATE::SKILL == m_eNetworkAction ||
			PLAYER_ACTION_STATE::KNOCKDOWN == m_eNetworkAction)
		{
			m_pChain = nullptr;
			m_iChainStage = 0;
			m_iChainStep = 0;
			m_eKnockdownStep = KNOCKDOWN_STEP::NONE;
			m_fActionPresentationSeconds = 0.f;
			Commit_PendingClipChains();
			Set_Animation(CHARACTER_ANIM::RUN, true);
		}
		m_iLastNetworkActionStartTick = actionStartTick;
	}
	else if (PLAYER_ACTION_STATE::FALLING == action)
	{
		if (INVALID_SKILL_ID != skillId || 0u == actionStartTick)
			return false;
		if (m_eNetworkAction == action)
			return true;
		m_pChain = nullptr;
		m_iChainStage = 0;
		m_iChainStep = 0;
		m_fActionPresentationSeconds = 0.f;
		Commit_PendingClipChains();
		/* No class owns a falling clip, so the damaged-idle loop plays while the
		Server drives the body down. The descent itself is the replicated Y, not
		an animation. */
		Set_Animation(CHARACTER_ANIM::HIT, true);
		m_iCurrentEffectSkillId = INVALID_SKILL_ID;
		m_iEffectActionStartTick = 0u;
		m_bHasEffectActionFacingYaw = false;
		m_fEffectActionFacingYawDegrees = 0.f;
	}
	else if (PLAYER_ACTION_STATE::KNOCKDOWN == action)
	{
		if (INVALID_SKILL_ID != skillId || 0u == actionStartTick)
			return false;
		if (m_eNetworkAction == action &&
			m_iLastNetworkActionStartTick == actionStartTick)
		{
			return true;
		}
		m_pChain = nullptr;
		m_iChainStage = 0;
		m_iChainStep = 0;
		m_fActionPresentationSeconds = 0.f;
		Commit_PendingClipChains();
		if (!Set_Animation(CHARACTER_ANIM::KNOCKDOWN, false))
			Set_Animation(CHARACTER_ANIM::HIT, false);
		m_eKnockdownStep = KNOCKDOWN_STEP::FALLING;
		m_iCurrentEffectSkillId = INVALID_SKILL_ID;
		m_iEffectActionStartTick = 0u;
		m_bHasEffectActionFacingYaw = false;
		m_fEffectActionFacingYawDegrees = 0.f;
		m_iLastNetworkActionStartTick = actionStartTick;
	}
	else if (PLAYER_ACTION_STATE::DEAD == action)
	{
		m_pChain = nullptr;
		m_iChainStage = 0;
		m_iChainStep = 0;
		m_eKnockdownStep = KNOCKDOWN_STEP::NONE;
		m_fActionPresentationSeconds = 0.f;
		Commit_PendingClipChains();
		Set_Animation(CHARACTER_ANIM::DEAD, false);
		m_iCurrentEffectSkillId = INVALID_SKILL_ID;
		m_iEffectActionStartTick = 0u;
		m_bHasEffectActionFacingYaw = false;
		m_fEffectActionFacingYawDegrees = 0.f;
	}
	else if (PLAYER_ACTION_STATE::SKILL == m_eNetworkAction)
	{
		m_pChain = nullptr;
		m_iChainStage = 0;
		m_iChainStep = 0;
		m_fActionPresentationSeconds = 0.f;
		Commit_PendingClipChains();
		Set_Animation(
			m_isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
			true);
		m_iCurrentEffectSkillId = INVALID_SKILL_ID;
		m_iEffectActionStartTick = 0u;
		m_bHasEffectActionFacingYaw = false;
		m_fEffectActionFacingYawDegrees = 0.f;
	}
	else if (PLAYER_ACTION_STATE::KNOCKDOWN == m_eNetworkAction)
	{
		m_fActionPresentationSeconds = 0.f;
		if (Set_Animation(CHARACTER_ANIM::STANDUP, false))
		{
			m_eKnockdownStep = KNOCKDOWN_STEP::STANDUP;
		}
		else
		{
			m_eKnockdownStep = KNOCKDOWN_STEP::NONE;
			Set_Animation(
				m_isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
				true);
		}
	}
	if (PLAYER_ACTION_STATE::SKILL != action)
	{
		m_bHasEffectActionFacingYaw = false;
		m_fEffectActionFacingYawDegrees = 0.f;
	}
	Update_ActionEmissiveOverride(action, skillId);
	m_hasNetworkSkillTarget = hasSkillTarget;
	m_NetworkSkillTarget = hasSkillTarget ? skillTarget : float3_t{};
	m_eNetworkAction = action;
	return true;
}

bool_t CCharacter::Try_Get_SkillTargetRoot(float4x4_t& outWorld) const
{
	if (!m_hasNetworkSkillTarget ||
		LostArk::Shared::PLAYER_ACTION_STATE::SKILL != m_eNetworkAction ||
		!std::isfinite(m_NetworkSkillTarget.x) ||
		!std::isfinite(m_NetworkSkillTarget.y) ||
		!std::isfinite(m_NetworkSkillTarget.z))
	{
		return false;
	}
	const f32_t yawDegrees = m_bHasEffectActionFacingYaw ?
		m_fEffectActionFacingYawDegrees : m_fPresentationYawDegrees;
	XMStoreFloat4x4(&outWorld,
		XMMatrixRotationY(XMConvertToRadians(yawDegrees)) *
		XMMatrixTranslation(
			m_NetworkSkillTarget.x,
			m_NetworkSkillTarget.y,
			m_NetworkSkillTarget.z));
	return true;
}

void CCharacter::Update_ActionEmissiveOverride(
	const LostArk::Shared::PLAYER_ACTION_STATE action,
	const LostArk::Shared::SKILL_ID skillId)
{
	m_ActionEmissiveOverride = {};
	if (LostArk::Shared::PLAYER_ACTION_STATE::SKILL != action ||
		nullptr == m_pSpec || nullptr == m_pSpec->pSkillSurfaceEmissives)
	{
		return;
	}

	for (uint32_t index = 0u;
		index < m_pSpec->iNumSkillSurfaceEmissives; ++index)
	{
		const SKILL_SURFACE_EMISSIVE_SPEC& spec =
			m_pSpec->pSkillSurfaceEmissives[index];
		if (spec.iSkillId != skillId || !std::isfinite(spec.fIntensity) ||
			spec.fIntensity <= 0.f)
		{
			continue;
		}
		m_ActionEmissiveOverride.isEnabled = true;
		m_ActionEmissiveOverride.vColor = spec.vColor;
		m_ActionEmissiveOverride.fIntensity = spec.fIntensity;
		return;
	}
}

void CCharacter::Apply_NetworkStance(const LostArk::Shared::PLAYER_STANCE_ID stance)
{
	const bool_t hasChanged = m_eStance != stance;
	m_eStance = stance;
	/* A stance that owns its own idle and run has to take over the pose the
	character is already holding; Set_Locomotion only fires on a move edge. A
	skill keeps its clip and picks the new stance up when it ends. */
	if (hasChanged && !Is_PlayingSkill())
	{
		Set_Animation(
			m_isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
			true);
	}
	for (uint32_t i = 0; i < m_pSpec->iNumWeapons; ++i)
	{
		const WEAPON_PART_SPEC& weapon = m_pSpec->pWeapons[i];
		Set_PartVisible(
			weapon.pPartTag,
			LostArk::Shared::PLAYER_STANCE_ID::NONE == weapon.eRequiredStance ||
				weapon.eRequiredStance == stance);
	}
}

void CCharacter::Set_PartVisible(const tchar_t* pPartTag, const bool_t isVisible)
{
	const auto pPart = dynamic_cast<CPart_Equipment*>(
		__super::Find_PartObject(pPartTag));
	if (nullptr != pPart)
		pPart->Set_Visible(isVisible);
}

const char_t* CCharacter::Resolve_LocomotionClip(const CHARACTER_ANIM eAnim) const
{
	const char_t* pClipName = m_pSpec->AnimationClips[ETOUI(eAnim)];
	if (CHARACTER_ANIM::IDLE != eAnim && CHARACTER_ANIM::RUN != eAnim)
		return pClipName;
	for (uint32_t i = 0; i < m_pSpec->iNumStanceLocomotion; ++i)
	{
		const STANCE_LOCOMOTION_SPEC& stance = m_pSpec->pStanceLocomotion[i];
		if (stance.eStance != m_eStance)
			continue;
		const char_t* pOverride = CHARACTER_ANIM::IDLE == eAnim ?
			stance.pIdleClip : stance.pRunClip;
		if (nullptr != pOverride)
			return pOverride;
	}
	return pClipName;
}

bool_t CCharacter::Set_Animation(CHARACTER_ANIM eAnim, bool_t isLoop)
{
	if (eAnim >= CHARACTER_ANIM::END)
		return false;
	return Set_Animation(Resolve_LocomotionClip(eAnim), isLoop);
}

bool_t CCharacter::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == m_pBodyModel || nullptr == pClipName)
		return false;
	m_pBodyModel->Set_AnimationSpeed(1.f);
	return m_pBodyModel->Set_Animation(
		pClipName,
		isLoop,
		CLIP_BLEND_SECONDS);
}

bool_t CCharacter::Try_SampleTargetGround(
	const f32_t x,
	const f32_t z,
	float3_t& outPosition) const
{
	if (nullptr == m_pNavigationCom || !std::isfinite(x) || !std::isfinite(z))
		return false;
	return m_pNavigationCom->Try_SampleWalkablePoint(
		XMVectorSet(x, 0.f, z, 1.f), outPosition);
}

PATH_RESULT_CODE CCharacter::Request_Move(fvector_t vGoalPosition)
{
	//유효하지 않은 Grid return 예외처리
	if (nullptr == m_pNavigationCom || nullptr == m_pTransformCom)
		return PATH_RESULT_CODE::INVALID_GRID;
	//PathFollower 객체에게 Path 요청
	const PATH_RESULT_CODE eResult = m_PathFollower.Request_Path(
		m_pNavigationCom,
		m_pTransformCom->Get_State(STATE::POSITION),
		vGoalPosition);

	if (PATH_RESULT_CODE::SUCCESS == eResult)
		Set_Locomotion(m_PathFollower.Has_Path());

	return eResult;
}

void CCharacter::Cancel_Move()
{
	m_PathFollower.Cancel();
	Set_Locomotion(false);
}

void CCharacter::Priority_Update(f32_t fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CCharacter::Update(f32_t fTimeDelta)
{
	//network state -> apply snapshot, !networkstate -> pathfinding
	if (m_hasNetworkState)
	{
		Update_NetworkTransform(fTimeDelta);
	}
	else
	{
		m_PathFollower.Update(
			m_pTransformCom,
			m_fMoveSpeed,
			fTimeDelta);

		Set_Locomotion(m_PathFollower.Has_Path());
	}

	if (m_fPendingIdleSeconds >= 0.f)
	{
		m_fPendingIdleSeconds -= fTimeDelta;
		if (m_fPendingIdleSeconds < 0.f)
			Commit_Locomotion(false);
	}

	//Set_Locomotion(m_PathFollower.Has_Path());

	/* A running chain owns the clip until it ends, so it advances before the logic
	gets a say and Is_PlayingSkill() is already correct when the logic reads it. */
	Update_Chain();
	Update_KnockdownPresentation();

	/* Class code may only update presentation. Input and gameplay commands are
	owned by PlayerController and its command sink. */
	if (m_isLocallyControlled && nullptr != m_pLogic)
		m_pLogic->Update_Presentation(*this, fTimeDelta);

	__super::Update(fTimeDelta);
	if (LostArk::Shared::PLAYER_ACTION_STATE::SKILL == m_eNetworkAction &&
		nullptr != m_pChain && std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
	{
		m_fActionPresentationSeconds += fTimeDelta;
	}
	if (nullptr != m_pColliderCom)
		m_pColliderCom->Update(XMLoadFloat4x4(
			m_pTransformCom->Get_WorldMatrixPtr()));
	Update_EffectCues();
}

void CCharacter::Late_Update(f32_t fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	/* The chains solve here and not in Update because snapshot application
	runs in the level update, between the two: a skill seek replays the clip
	and rebuilds every bone, which erased an Update-time solve on each
	snapshot frame and strobed the cloth at 30Hz. After Late_Update nothing
	re-poses the skeleton before the render pass binds it. */
	if (BONE_CHAINS_ENABLED && m_BoneChains.Is_Active() &&
		nullptr != m_pTransformCom && nullptr != m_pBodyModel)
	{
		m_BoneChains.Update(m_pBodyModel, fTimeDelta,
			m_pTransformCom->Get_State(STATE::POSITION),
			m_fPresentationYawDegrees);
	}

#ifdef _DEBUG
	if (m_isNavigationDebugVisible && nullptr != m_pNavigationCom)
		CGameInstance::Get().Add_DebugComponent(m_pNavigationCom);
	if (m_isCombatColliderDebugVisible && nullptr != m_pColliderCom)
		CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
	if (m_isSkillHitAreaDebugVisible)
	{
		Update_SkillProjectileDebug(fTimeDelta);
		Draw_SkillHitAreaDebug();
	}
	else
	{
		m_DebugProjectiles.clear();
	}
#endif
}

#ifdef _DEBUG
void CCharacter::Update_SkillProjectileDebug(const f32_t fTimeDelta)
{
	if (nullptr == m_pBodyModel || nullptr == m_pTransformCom ||
		m_EffectCueDocument.Projectiles.empty())
	{
		m_DebugProjectiles.clear();
		return;
	}
	const uint32_t iAnimation = m_pBodyModel->Get_CurrentAnimIndex();
	const char_t* pClipName = m_pBodyModel->Get_AnimationName(iAnimation);
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTicksPerSecond =
		m_pBodyModel->Get_AnimationTickPerSecond(iAnimation);
	if (nullptr != pClipName && fTicksPerSecond > 0.f &&
		m_pBodyModel->Get_AnimationProgress(iAnimation, fPosition, fDuration))
	{
		const f32_t fNowMs = fPosition * 1000.f / fTicksPerSecond;
		/* A spawn is the clip clock crossing the cue's start; a clip change or a
		seek backwards restarts the watch without firing. */
		const bool_t bSameClip = m_strDebugProjectileClip == pClipName;
		const f32_t fPreviousMs = bSameClip ? m_fDebugProjectileClipMs : -1.f;
		if (bSameClip && fNowMs > fPreviousMs)
		{
			const matrix_t WorldRoot =
				XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
			const vector_t vPosition = WorldRoot.r[3];
			const vector_t vLook =
				XMVector3Normalize(XMVectorSetY(WorldRoot.r[2], 0.f));
			for (size_t iCue = 0; iCue < m_EffectCueDocument.Projectiles.size(); ++iCue)
			{
				const ANIMATION_PROJECTILE_CUE& Cue =
					m_EffectCueDocument.Projectiles[iCue];
				const f32_t fStartMs = static_cast<f32_t>(Cue.iStartMs);
				if (Cue.strClipName != pClipName ||
					fPreviousMs >= fStartMs || fNowMs < fStartMs)
				{
					continue;
				}
				DEBUG_PROJECTILE_WIRE Wire{};
				Wire.iCue = iCue;
				Wire.fDirectionX = XMVectorGetX(vLook);
				Wire.fDirectionZ = XMVectorGetZ(vLook);
				Wire.fX = XMVectorGetX(vPosition);
				Wire.fY = XMVectorGetY(vPosition);
				Wire.fZ = XMVectorGetZ(vPosition);
				Wire.fRemainingSeconds = static_cast<f32_t>(Cue.iLifeMs) * 0.001f;
				if (1u == Cue.iOrigin && 1u == Cue.iKind)
				{
					/* The Server places an AIM area where the aim points; the
					client has no aim here and shows the far end of its reach. */
					Wire.fX += Wire.fDirectionX * Cue.fMaxDistance;
					Wire.fZ += Wire.fDirectionZ * Cue.fMaxDistance;
				}
				else
				{
					const f32_t fRightX = Wire.fDirectionZ;
					const f32_t fRightZ = -Wire.fDirectionX;
					Wire.fX += Wire.fDirectionX * Cue.fOffsetForward +
						fRightX * Cue.fOffsetRight;
					Wire.fZ += Wire.fDirectionZ * Cue.fOffsetForward +
						fRightZ * Cue.fOffsetRight;
				}
				if (1u == Cue.iKind)
					Wire.fRemainingDistance = 0.f;
				else
					Wire.fRemainingDistance =
						Cue.fMaxDistance > 0.f ? Cue.fMaxDistance : -1.f;
				m_DebugProjectiles.push_back(Wire);
			}
		}
		m_strDebugProjectileClip = pClipName;
		m_fDebugProjectileClipMs = fNowMs;
	}
	for (size_t i = 0; i < m_DebugProjectiles.size();)
	{
		DEBUG_PROJECTILE_WIRE& Wire = m_DebugProjectiles[i];
		const ANIMATION_PROJECTILE_CUE& Cue =
			m_EffectCueDocument.Projectiles[Wire.iCue];
		Wire.fRemainingSeconds -= fTimeDelta;
		if (Cue.fSpeed > 0.f)
		{
			f32_t fStep = Cue.fSpeed * fTimeDelta;
			if (Wire.fRemainingDistance >= 0.f)
			{
				fStep = (std::min)(fStep, Wire.fRemainingDistance);
				Wire.fRemainingDistance -= fStep;
			}
			Wire.fX += Wire.fDirectionX * fStep;
			Wire.fZ += Wire.fDirectionZ * fStep;
		}
		const bool_t bExpired = Wire.fRemainingSeconds <= 0.f ||
			(Cue.fSpeed > 0.f && 0.f == Wire.fRemainingDistance);
		if (bExpired)
			m_DebugProjectiles.erase(m_DebugProjectiles.begin() + i);
		else
			++i;
	}
}

void CCharacter::Draw_SkillHitAreaDebug() const
{
	if (nullptr == m_pBodyModel || nullptr == m_pTransformCom)
		return;
	constexpr uint32_t PROJECTILE_COLOR_RGBA =
		255u | (170u << 8) | (30u << 16) | (255u << 24);
	for (const DEBUG_PROJECTILE_WIRE& Wire : m_DebugProjectiles)
	{
		const vector_t vLook = XMVectorSet(Wire.fDirectionX, 0.f, Wire.fDirectionZ, 0.f);
		const vector_t vRight = XMVector3Normalize(
			XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
		float4x4_t Root{};
		XMStoreFloat4x4(&Root, XMMatrixSet(
			XMVectorGetX(vRight), 0.f, XMVectorGetZ(vRight), 0.f,
			0.f, 1.f, 0.f, 0.f,
			Wire.fDirectionX, 0.f, Wire.fDirectionZ, 0.f,
			Wire.fX, Wire.fY, Wire.fZ, 1.f));
		for (const HIT_AREA_SHAPE& Shape :
			m_EffectCueDocument.Projectiles[Wire.iCue].Shapes)
		{
			CHitAreaWire::Draw(Root, Shape, PROJECTILE_COLOR_RGBA);
		}
	}
	if (m_EffectCueDocument.Hits.empty())
		return;
	const uint32_t iAnimation = m_pBodyModel->Get_CurrentAnimIndex();
	const char_t* pClipName = m_pBodyModel->Get_AnimationName(iAnimation);
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTicksPerSecond =
		m_pBodyModel->Get_AnimationTickPerSecond(iAnimation);
	if (nullptr == pClipName || fTicksPerSecond <= 0.f ||
		!m_pBodyModel->Get_AnimationProgress(iAnimation, fPosition, fDuration))
		return;
	const f32_t fNowMs = fPosition * 1000.f / fTicksPerSecond;
	constexpr uint32_t ACTIVE_HIT_COLOR_RGBA =
		255u | (70u << 8) | (60u << 16) | (255u << 24);
	constexpr f32_t MIN_VISIBLE_HIT_WINDOW_MS = 300.f;
	for (const ANIMATION_HIT_CUE& Hit : m_EffectCueDocument.Hits)
	{
		if (Hit.Shape.iAreaType <= 0 || Hit.strClipName != pClipName)
			continue;
		const f32_t fWidthMs = (std::max)(
			static_cast<f32_t>(Hit.iEndMs - Hit.iStartMs),
			MIN_VISIBLE_HIT_WINDOW_MS);
		for (uint32_t iTick = 0u; iTick < Hit.iRepeatCount; ++iTick)
		{
			const f32_t fTickMs =
				static_cast<f32_t>(Hit.iStartMs + Hit.iRepeatMs * iTick);
			if (fNowMs >= fTickMs && fNowMs <= fTickMs + fWidthMs)
			{
				CHitAreaWire::Draw(*m_pTransformCom->Get_WorldMatrixPtr(),
					Hit.Shape, ACTIVE_HIT_COLOR_RGBA);
				break;
			}
		}
	}
}
#endif

HRESULT CCharacter::Render()
{
	return S_OK;
}

HRESULT CCharacter::Ready_Components()
{
	using namespace LostArk::Shared::WorldCollision;
	CBounding_OBB::BOUNDING_OBB_DESC colliderDesc{};
	colliderDesc.vCenter = float3_t(0.f, PLAYER_CENTER_OFFSET_Y, 0.f);
	colliderDesc.vSize = float3_t(
		PLAYER_HALF_EXTENT_X * 2.f,
		PLAYER_HALF_EXTENT_Y * 2.f,
		PLAYER_HALF_EXTENT_Z * 2.f);
	if (FAILED(__super::Add_Component(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_Component_Collider_Player"),
		TEXT("Com_Collider"),
		m_pColliderCom,
		&colliderDesc)))
	{
		return E_FAIL;
	}
	m_pColliderCom->Update(XMLoadFloat4x4(
		m_pTransformCom->Get_WorldMatrixPtr()));

	if (m_strNavigationPrototypeTag.empty())
		return S_OK;

	return __super::Add_Component(
		m_iPrototypeLevelIndex,
		m_strNavigationPrototypeTag,
		TEXT("Com_Navigation"),
		m_pNavigationCom);
}

HRESULT CCharacter::Ready_PartObjects()
{
	/* Part tags are ordered because CContainerObject keeps parts in a std::map and
	updates them in tag order. The body has to run first: socketed parts read its
	bone matrices during their own Update. */
	CPart_Body::PART_BODY_DESC bodyDesc{};
	bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
	bodyDesc.strModelTag = m_pSpec->pBodyModelTag;
	bodyDesc.strShaderTag = m_pSpec->pShaderTag;
	bodyDesc.iHiddenMeshMask = m_pSpec->iBodyHiddenMeshMask;
	bodyDesc.pInitialAnimation = m_pSpec->AnimationClips[ETOUI(CHARACTER_ANIM::IDLE)];
	bodyDesc.pEmissiveOverride = &m_ActionEmissiveOverride;

	if (FAILED(__super::Add_PartObject(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Part_Body"),
		TEXT("Part_00_Body"),
		&bodyDesc)))
		return E_FAIL;

	m_pBodyModel = dynamic_pointer_cast<CModel>(
		__super::Get_Component(TEXT("Part_00_Body"), TEXT("Com_Model")));
	if (nullptr == m_pBodyModel)
		return E_FAIL;

	m_pBodyModel->Enable_RootMotionSuppression(
		ROOT_MOTION_BONE, ROOT_MOTION_VERTICAL_AXIS);

	/* An AVATAR_HEAD slot covers the class's DEFAULT_HELMET piece. An
	AVATAR_ARMOR slot covers every plain DEFAULT piece but not the helmet --
	that stays governed by AVATAR_HEAD alone, so armor and head toggle
	independently. A class declares its parts; this is the only place that
	derives who hides whom. */
	bool_t hasAvatarHead = false;
	bool_t hasAvatarArmor = false;
	for (uint32_t i = 0; i < m_pSpec->iNumEquipment; ++i)
	{
		if (EQUIPMENT_SLOT_KIND::AVATAR_HEAD == m_pSpec->pEquipment[i].eSlotKind)
			hasAvatarHead = true;
		else if (EQUIPMENT_SLOT_KIND::AVATAR_ARMOR == m_pSpec->pEquipment[i].eSlotKind)
			hasAvatarArmor = true;
	}

	/* Skinned equipment: no socket, it borrows the body's bone palette. */
	for (uint32_t i = 0; i < m_pSpec->iNumEquipment; ++i)
	{
		CPart_Equipment::PART_EQUIPMENT_DESC equipmentDesc{};
		equipmentDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		equipmentDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		equipmentDesc.strModelTag = m_pSpec->pEquipment[i].pModelTag;
		equipmentDesc.strShaderTag = m_pSpec->pShaderTag;
		equipmentDesc.iHiddenMeshMask = m_pSpec->pEquipment[i].iHiddenMeshMask;
		equipmentDesc.pSkeletonModel = m_pBodyModel;
		equipmentDesc.pEmissiveOverride = &m_ActionEmissiveOverride;

		if (FAILED(__super::Add_PartObject(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Part_Equipment"),
			m_pSpec->pEquipment[i].pPartTag,
			&equipmentDesc)))
			return E_FAIL;

		const EQUIPMENT_SLOT_KIND eKind = m_pSpec->pEquipment[i].eSlotKind;
		bool_t isHidden = m_pSpec->pEquipment[i].isHidden;
		if (EQUIPMENT_SLOT_KIND::DEFAULT_HELMET == eKind && hasAvatarHead)
			isHidden = true;
		else if (EQUIPMENT_SLOT_KIND::DEFAULT == eKind && hasAvatarArmor)
			isHidden = true;
		if (isHidden)
			Set_PartVisible(m_pSpec->pEquipment[i].pPartTag, false);
	}

	/* The weapon is the same part class in socket mode. It gets its own class once
	hit windows and trails need somewhere to live. */
	for (uint32_t i = 0; i < m_pSpec->iNumWeapons; ++i)
	{
		CPart_Equipment::PART_EQUIPMENT_DESC weaponDesc{};
		weaponDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		weaponDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		weaponDesc.strModelTag = m_pSpec->pWeapons[i].pModelTag;
		weaponDesc.strShaderTag = m_pSpec->pWeaponShaderTag;
		weaponDesc.pSkeletonModel = m_pBodyModel;
		weaponDesc.pSocketBoneName = m_pSpec->pWeapons[i].pSocketBone;
		weaponDesc.fSocketYawDegrees = m_pSpec->pWeapons[i].fSocketYawDegrees;
		weaponDesc.pEmissiveOverride = &m_ActionEmissiveOverride;

		if (FAILED(__super::Add_PartObject(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Part_Equipment"),
			m_pSpec->pWeapons[i].pPartTag,
			&weaponDesc)))
			return E_FAIL;
	}

	return S_OK;
}

void CCharacter::Set_Locomotion(bool_t isMoving)
{
	if (isMoving)
		m_fPendingIdleSeconds = -1.f;

	if (m_isMoving == isMoving)
		return;

	if (!isMoving)
	{
		if (m_fPendingIdleSeconds < 0.f)
			m_fPendingIdleSeconds = LOCOMOTION_IDLE_DELAY_SECONDS;
		return;
	}

	Commit_Locomotion(isMoving);
}

void CCharacter::Commit_Locomotion(bool_t isMoving)
{
	m_isMoving = isMoving;
	if (Is_PlayingSkill())
		return;

	Set_Animation(
		isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
		true);
}

unique_ptr<CCharacter> CCharacter::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CCharacter>(new CCharacter(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][Character] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CCharacter::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CCharacter>(new CCharacter(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][Character] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
