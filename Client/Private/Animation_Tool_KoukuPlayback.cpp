#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "CharacterPreviewPanel.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "EffectV2_Runtime.h"
#include "Model.h"
#include "SoundCueCatalog.h"
#include <charconv>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <span>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>




void Client::CAnimation_Tool::Reset_KoukuSaydonActionDocumentState(
	const bool_t bClearProfile)
{
	m_KoukuSaydonActionReference = {};
	m_KoukuSaydonActionAuthored = {};
	m_KoukuSaydonAnimationPatterns = {};
	m_bKoukuSaydonActionLoadAttempted = false;
	m_bKoukuSaydonActionDirty = false;
	m_bKoukuSaydonPatternDirty = false;
	m_bKoukuSaydonActionReloadConfirmationRequested = false;
	m_iSelectedKoukuSaydonAction = -1;
	m_iRequestedKoukuSaydonSourceActionId = 0u;
	m_iSelectedKoukuSaydonStage = 0;
	m_iSelectedKoukuSaydonSlot = 0;
	m_iSelectedKoukuSaydonActionClip = 0;
	m_iSelectedKoukuSaydonPattern = -1;
	m_iSelectedKoukuSaydonPatternClip = 0;
	m_KoukuSaydonActionFilter[0] = '\0';
	m_KoukuSaydonPatternFilter[0] = '\0';
	m_strKoukuSaydonActionStatus.clear();
	m_strKoukuSaydonPatternStatus.clear();
	Reset_KoukuSaydonPatternPreviewState({});
	if (bClearProfile)
		m_strKoukuSaydonProfileId.clear();
}

bool_t Client::CAnimation_Tool::Open_KoukuSaydonProfile(
	const std::string& profileId)
{
	const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* pProfile =
		Find_KoukuSaydonActionProfile(profileId);
	const bool_t bClipOnlyDonor =
		"MN_RPCT_00" == profileId || "MN_RPCT_03" == profileId || "MN_RPCZ_00-1" == profileId;
	if (nullptr == pProfile && !bClipOnlyDonor)
	{
		m_Status = "KoukuSaydon action profile is not admitted: " + profileId;
		return false;
	}
	const char_t* pPreviewAsset = bClipOnlyDonor ?
		profileId.c_str() : pProfile->pPreviewAssetName;

	if (Is_AnyDocumentDirty() && !m_strKoukuSaydonProfileId.empty() &&
		m_strKoukuSaydonProfileId != profileId)
	{
		m_Status =
			"Save or discard the current Animation document before opening another KoukuSaydon profile.";
		return false;
	}
	if (m_bKoukuSaydonPatternPreviewPlaying)
	{
		const shared_ptr<Engine::CModel> PreviewModel =
			m_KoukuSaydonPatternPreviewModel.lock();
		if (nullptr != PreviewModel)
		{
			Stop_KoukuSaydonPatternPreview(
				PreviewModel,
				"KoukuSaydon Pattern preview stopped before the profile changed; idle restored.");
		}
		else
		{
			Reset_KoukuSaydonPatternPreviewState(
				"KoukuSaydon Pattern preview stopped before the profile changed.");
		}
	}
	if (!m_pPreviewPanel->Select_TargetAsset(pPreviewAsset))
	{
		m_Status = "KoukuSaydon profile selected, but its physical preview body could not open: " +
			m_pPreviewPanel->Get_Status();
		return false;
	}

	const bool_t bProfileChanged = m_strKoukuSaydonProfileId != profileId;
	if (bProfileChanged)
		Reset_KoukuSaydonActionDocumentState(true);
	m_bValtanDataWorkspaceRequested = false;
	m_strKoukuSaydonProfileId = profileId;
	m_Status = bClipOnlyDonor ?
		("Opened " + profileId +
		 " as a local clip donor preview; select its physical clips to inspect and play them.") :
		("Opened KoukuSaydon extracted action profile " + profileId +
		 " as a local REFERENCE_ONLY preview.");
	return true;
}

bool_t Client::CAnimation_Tool::Open_KoukuSaydonAction(
	const std::string& profileId,
	const std::uint32_t iSourceActionId)
{
	if (0u == iSourceActionId || !Open_KoukuSaydonProfile(profileId))
		return false;
	m_iRequestedKoukuSaydonSourceActionId = iSourceActionId;
	const auto Requested = std::find_if(
		m_KoukuSaydonActionReference.Actions.begin(),
		m_KoukuSaydonActionReference.Actions.end(),
		[iSourceActionId](const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& Action)
		{
			return Action.iSourceActionId == iSourceActionId;
		});
	if (Requested != m_KoukuSaydonActionReference.Actions.end())
	{
		m_iSelectedKoukuSaydonAction = static_cast<int32_t>(
			std::distance(m_KoukuSaydonActionReference.Actions.begin(), Requested));
		m_iSelectedKoukuSaydonStage = 0;
		m_iSelectedKoukuSaydonSlot = 0;
		m_iSelectedKoukuSaydonActionClip = 0;
		m_iRequestedKoukuSaydonSourceActionId = 0u;
	}
	return true;
}

bool_t Client::CAnimation_Tool::Preview_KoukuSaydonCompositionAnimation(
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
	std::string& strOutStatus)
{
	KOUKU_SAYDON_COMPOSITION_STAGE stage;
	stage.iDurationMs = occurrence.iPlayMs;
	stage.AnimationOccurrences.push_back(occurrence);
	stage.AnimationOccurrences.front().iStartOffsetMs = 0u;
	KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
	pattern.Stages.push_back(std::move(stage));
	return Preview_KoukuSaydonCompositionPattern(pattern, strOutStatus);
}

bool_t Client::CAnimation_Tool::Preview_KoukuSaydonCompositionPattern(
	const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
	std::string& strOutStatus,
	const std::uint32_t startClockMs,
	const bool_t startPaused)
{
	for (const auto& stage : pattern.Stages)
	{
		if (stage.AnimationOccurrences.empty()) continue;
		const auto& first = stage.AnimationOccurrences.front();
		const auto* profile = Find_KoukuSaydonActionProfile(first.strProfileId);
		if (nullptr != profile || first.strProfileId == "MN_RPCT_00")
			return Preview_CompositionResourcePattern(pattern,
				nullptr != profile ? profile->pPreviewAssetName : "MN_RPCT_00",
				strOutStatus, startClockMs, startPaused);
		strOutStatus = "KoukuSaydon composition preview has an unknown profile: " + first.strProfileId;
		m_Status = m_strKoukuSaydonPatternStatus = strOutStatus;
		return false;
	}
	strOutStatus = "Composition Pattern preview has no animation occurrence.";
	m_Status = m_strKoukuSaydonPatternStatus = strOutStatus;
	return false;
}

Client::KOUKU_SAYDON_ANIMATION_ACTION_BINDING*
Client::CAnimation_Tool::Find_KoukuSaydonActionBinding(
	const std::uint32_t iSourceActionId,
	const std::string& strStageId,
	const std::string& strSlotId)
{
	const auto Found = std::find_if(
		m_KoukuSaydonActionAuthored.Bindings.begin(),
		m_KoukuSaydonActionAuthored.Bindings.end(),
		[&](const KOUKU_SAYDON_ANIMATION_ACTION_BINDING& Binding)
		{
			return Binding.iSourceActionId == iSourceActionId &&
				Binding.strStageId == strStageId &&
				Binding.strSlotId == strSlotId;
		});
	return Found == m_KoukuSaydonActionAuthored.Bindings.end() ?
		nullptr : &*Found;
}

const Client::KOUKU_SAYDON_ANIMATION_ACTION_BINDING*
Client::CAnimation_Tool::Find_KoukuSaydonActionBinding(
	const std::uint32_t iSourceActionId,
	const std::string& strStageId,
	const std::string& strSlotId) const
{
	const auto Found = std::find_if(
		m_KoukuSaydonActionAuthored.Bindings.begin(),
		m_KoukuSaydonActionAuthored.Bindings.end(),
		[&](const KOUKU_SAYDON_ANIMATION_ACTION_BINDING& Binding)
		{
			return Binding.iSourceActionId == iSourceActionId &&
				Binding.strStageId == strStageId &&
				Binding.strSlotId == strSlotId;
		});
	return Found == m_KoukuSaydonActionAuthored.Bindings.end() ?
		nullptr : &*Found;
}

void Client::CAnimation_Tool::Upsert_KoukuSaydonActionBinding(
	const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& ReferenceSlot,
	const std::uint32_t iSourceActionId,
	const std::string& strStageId,
	const std::string& strRuntimeClip,
	const std::uint32_t iSourceStartMs,
	const std::uint32_t iPlayMs,
	const f32_t fPlayRate,
	const bool_t bLoop)
{
	KOUKU_SAYDON_ANIMATION_ACTION_BINDING* pBinding = Find_KoukuSaydonActionBinding(
		iSourceActionId, strStageId, ReferenceSlot.strSlotId);
	if (nullptr == pBinding)
	{
		KOUKU_SAYDON_ANIMATION_ACTION_BINDING Binding;
		Binding.iSourceActionId = iSourceActionId;
		Binding.strStageId = strStageId;
		Binding.strSlotId = ReferenceSlot.strSlotId;
		m_KoukuSaydonActionAuthored.Bindings.push_back(std::move(Binding));
		pBinding = &m_KoukuSaydonActionAuthored.Bindings.back();
	}
	pBinding->strRuntimeClip = strRuntimeClip;
	pBinding->iSourceStartMs = iSourceStartMs;
	pBinding->iPlayMs = iPlayMs;
	pBinding->fPlayRate = fPlayRate;
	pBinding->bLoop = bLoop;
	pBinding->strMappingBasis = "PROJECT_AUTHORED";
	pBinding->strAuthority = "REFERENCE_ONLY";
	std::sort(
		m_KoukuSaydonActionAuthored.Bindings.begin(),
		m_KoukuSaydonActionAuthored.Bindings.end(),
		[](const KOUKU_SAYDON_ANIMATION_ACTION_BINDING& Left,
			const KOUKU_SAYDON_ANIMATION_ACTION_BINDING& Right)
		{
			return std::tie(
				Left.iSourceActionId, Left.strStageId, Left.strSlotId) <
				std::tie(
					Right.iSourceActionId, Right.strStageId, Right.strSlotId);
		});
	m_bKoukuSaydonActionDirty = true;
	m_strKoukuSaydonActionStatus =
		"Staged one PROJECT_AUTHORED local slot override. Save commits only the sparse binding document.";
}

void Client::CAnimation_Tool::Remove_KoukuSaydonActionBinding(
	const std::uint32_t iSourceActionId,
	const std::string& strStageId,
	const std::string& strSlotId)
{
	const auto NewEnd = std::remove_if(
		m_KoukuSaydonActionAuthored.Bindings.begin(),
		m_KoukuSaydonActionAuthored.Bindings.end(),
		[&](const KOUKU_SAYDON_ANIMATION_ACTION_BINDING& Binding)
		{
			return Binding.iSourceActionId == iSourceActionId &&
				Binding.strStageId == strStageId &&
				Binding.strSlotId == strSlotId;
		});
	if (NewEnd == m_KoukuSaydonActionAuthored.Bindings.end())
		return;
	m_KoukuSaydonActionAuthored.Bindings.erase(
		NewEnd, m_KoukuSaydonActionAuthored.Bindings.end());
	m_bKoukuSaydonActionDirty = true;
	m_strKoukuSaydonActionStatus =
		"Removed the local override; this slot now resolves to its extracted default.";
}

bool_t Client::CAnimation_Tool::Load_KoukuSaydonActionBindings(
	const shared_ptr<Engine::CModel>& pModel)
{
	const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* pProfile =
		Find_KoukuSaydonActionProfile(m_strKoukuSaydonProfileId);
	if (nullptr == pProfile || nullptr == pModel)
	{
		m_strKoukuSaydonActionStatus =
			"No exact KoukuSaydon action profile and physical WModel are selected.";
		return false;
	}

	KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT StagedReference;
	KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT StagedAuthored;
	KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT StagedPatterns;
	std::string Status;
	const std::vector<std::string> AvailableClips = Collect_ClipNames(pModel);
	if (!CKoukuSaydonAnimationActionDocument::Load(
		pProfile->pProfileId,
		pProfile->pModelAssetId,
		AvailableClips,
		StagedReference,
		StagedAuthored,
		Status))
	{
		m_strKoukuSaydonActionStatus =
			"Load rejected; current KoukuSaydon action draft preserved: " + Status;
		return false;
	}
	if (!CKoukuSaydonAnimationPatternDocument::Load(
		pProfile->pProfileId,
		StagedReference,
		pProfile->pModelAssetId,
		AvailableClips,
		StagedPatterns,
		Status))
	{
		m_strKoukuSaydonActionStatus =
			"Load rejected; current KoukuSaydon action/pattern drafts preserved: " +
			Status;
		return false;
	}

	m_KoukuSaydonActionReference = std::move(StagedReference);
	m_KoukuSaydonActionAuthored = std::move(StagedAuthored);
	m_KoukuSaydonAnimationPatterns = std::move(StagedPatterns);
	m_iSelectedKoukuSaydonAction = -1;
	for (int32_t iAction = 0;
		iAction < static_cast<int32_t>(m_KoukuSaydonActionReference.Actions.size());
		++iAction)
	{
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& Action =
			m_KoukuSaydonActionReference.Actions[iAction];
		const bool_t bHasSlot = std::any_of(
			Action.Stages.begin(), Action.Stages.end(),
			[](const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& Stage)
			{
				return !Stage.Slots.empty();
			});
		if (bHasSlot && "REVIEW_CANDIDATE" == Action.strReviewStatus)
		{
			m_iSelectedKoukuSaydonAction = iAction;
			break;
		}
	}
	if (m_iSelectedKoukuSaydonAction < 0 &&
		!m_KoukuSaydonActionReference.Actions.empty())
	{
		m_iSelectedKoukuSaydonAction = 0;
	}
	if (0u != m_iRequestedKoukuSaydonSourceActionId)
	{
		const auto Requested = std::find_if(
			m_KoukuSaydonActionReference.Actions.begin(),
			m_KoukuSaydonActionReference.Actions.end(),
			[this](const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& Action)
			{
				return Action.iSourceActionId ==
					m_iRequestedKoukuSaydonSourceActionId;
			});
		if (Requested != m_KoukuSaydonActionReference.Actions.end())
		{
			m_iSelectedKoukuSaydonAction = static_cast<int32_t>(std::distance(
				m_KoukuSaydonActionReference.Actions.begin(), Requested));
		}
		m_iRequestedKoukuSaydonSourceActionId = 0u;
	}
	m_iSelectedKoukuSaydonStage = 0;
	m_iSelectedKoukuSaydonSlot = 0;
	m_iSelectedKoukuSaydonActionClip = 0;
	m_iSelectedKoukuSaydonPattern = m_KoukuSaydonAnimationPatterns.Patterns.empty() ? -1 : 0;
	m_iSelectedKoukuSaydonPatternClip = 0;
	if (m_iSelectedKoukuSaydonAction >= 0)
	{
		const auto& Stages = m_KoukuSaydonActionReference.Actions[
			m_iSelectedKoukuSaydonAction].Stages;
		for (int32_t iStage = 0;
			iStage < static_cast<int32_t>(Stages.size()); ++iStage)
		{
			if (!Stages[iStage].Slots.empty())
			{
				m_iSelectedKoukuSaydonStage = iStage;
				break;
			}
		}
	}
	m_bKoukuSaydonActionDirty = false;
	m_bKoukuSaydonPatternDirty = false;
	m_strKoukuSaydonActionStatus =
		"Loaded immutable KoukuSaydon action references, sparse overrides, and " +
		std::to_string(m_KoukuSaydonAnimationPatterns.Patterns.size()) +
		" local REFERENCE_ONLY pattern(s).";
	m_strKoukuSaydonPatternStatus = Status;
	return true;
}

bool_t Client::CAnimation_Tool::Save_KoukuSaydonActionBindings(
	const shared_ptr<Engine::CModel>& pModel)
{
	const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* pProfile =
		Find_KoukuSaydonActionProfile(m_strKoukuSaydonProfileId);
	if (nullptr == pProfile || nullptr == pModel ||
		m_KoukuSaydonActionReference.Actions.empty())
	{
		m_strKoukuSaydonActionStatus =
			"KoukuSaydon action Save requires a validated profile reference and physical WModel.";
		return false;
	}
	std::string Status;
	if (!CKoukuSaydonAnimationActionDocument::Save_Atomic(
		m_KoukuSaydonActionAuthored,
		m_KoukuSaydonActionReference,
		pProfile->pProfileId,
		pProfile->pModelAssetId,
		Collect_ClipNames(pModel),
		Status))
	{
		m_strKoukuSaydonActionStatus =
			"Save rejected; destination and current KoukuSaydon bindings preserved: " +
			Status;
		return false;
	}
	m_bKoukuSaydonActionDirty = false;
	m_strKoukuSaydonActionStatus = Status;
	return true;
}

bool_t Client::CAnimation_Tool::Save_KoukuSaydonAnimationPatterns(
	const shared_ptr<Engine::CModel>& pModel)
{
	const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* pProfile =
		Find_KoukuSaydonActionProfile(m_strKoukuSaydonProfileId);
	if (nullptr == pProfile || nullptr == pModel ||
		m_KoukuSaydonActionReference.Actions.empty())
	{
		m_strKoukuSaydonPatternStatus =
			"KoukuSaydon Pattern Save requires a validated profile reference and physical WModel.";
		return false;
	}

	std::string Status;
	if (!CKoukuSaydonAnimationPatternDocument::Save_Atomic(
		m_KoukuSaydonAnimationPatterns,
		m_KoukuSaydonActionReference,
		pProfile->pProfileId,
		pProfile->pModelAssetId,
		Collect_ClipNames(pModel),
		Status))
	{
		m_strKoukuSaydonPatternStatus =
			"Pattern Save rejected; destination and current draft preserved: " +
			Status;
		return false;
	}
	m_bKoukuSaydonPatternDirty = false;
	m_strKoukuSaydonPatternStatus = Status;
	return true;
}

bool_t Client::CAnimation_Tool::Build_KoukuSaydonPatternFromAction(
	const shared_ptr<Engine::CModel>& pModel,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& Action,
	const std::string& strPatternId,
	KOUKU_SAYDON_ANIMATION_PATTERN& outPattern,
	std::string& strOutStatus) const
{
	if (nullptr == pModel || strPatternId.empty())
	{
		strOutStatus =
			"KoukuSaydon Pattern materialization requires the exact physical WModel and a stable pattern ID.";
		return false;
	}

	KOUKU_SAYDON_ANIMATION_PATTERN Staged;
	Staged.strPatternId = strPatternId;
	Staged.strDisplayName = Action.strDisplayName;
	Staged.iSourceActionId = Action.iSourceActionId;
	std::uint32_t iOccurrenceOrdinal = 1u;

	for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& Stage : Action.Stages)
	{
		for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& Slot : Stage.Slots)
		{
			const KOUKU_SAYDON_ANIMATION_ACTION_BINDING* const pOverride =
				Find_KoukuSaydonActionBinding(
					Action.iSourceActionId, Stage.strStageId, Slot.strSlotId);
			const std::string& strRuntimeClip = nullptr != pOverride ?
				pOverride->strRuntimeClip : Slot.strRuntimeClip;
			const std::uint32_t iSourceStartMs = nullptr != pOverride ?
				pOverride->iSourceStartMs : Slot.iSourceStartMs;
			const std::uint32_t iPlayMs = nullptr != pOverride ?
				pOverride->iPlayMs : Slot.iPlayMs;
			const f32_t fPlayRate = nullptr != pOverride ?
				pOverride->fPlayRate : Slot.fPlayRate;
			const bool_t bLoop = nullptr != pOverride ?
				pOverride->bLoop : Slot.bLoop;

			std::uint32_t iAnimation = 0u;
			bool_t bFoundClip = false;
			for (; iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
			{
				const char_t* const pClipName =
					pModel->Get_AnimationName(iAnimation);
				if (nullptr != pClipName && strRuntimeClip == pClipName)
				{
					bFoundClip = true;
					break;
				}
			}
			f32_t fPositionTicks = 0.f;
			f32_t fDurationTicks = 0.f;
			const f32_t fTicksPerSecond = bFoundClip ?
				pModel->Get_AnimationTickPerSecond(iAnimation) : 0.f;
			if (!bFoundClip || !std::isfinite(fTicksPerSecond) ||
				fTicksPerSecond <= 0.f ||
				!pModel->Get_AnimationProgress(
					iAnimation, fPositionTicks, fDurationTicks) ||
				!std::isfinite(fDurationTicks) || fDurationTicks <= 0.f)
			{
				strOutStatus =
					"KoukuSaydon action materialization found an unavailable or malformed physical clip: " +
					strRuntimeClip + ".";
				return false;
			}

			const f64_t fNativeDurationMs =
				static_cast<f64_t>(fDurationTicks) /
				static_cast<f64_t>(fTicksPerSecond) * 1000.0;
			if (static_cast<f64_t>(iSourceStartMs) + 0.0001 >=
				fNativeDurationMs)
			{
				strOutStatus =
					"KoukuSaydon action materialization found a sourceStartMs outside the physical clip: " +
					strRuntimeClip + ".";
				return false;
			}
			const f64_t fSourceWindowEndMs =
				static_cast<f64_t>(iSourceStartMs) +
				static_cast<f64_t>(iPlayMs) * static_cast<f64_t>(fPlayRate);

			KOUKU_SAYDON_ANIMATION_PATTERN_CLIP Clip;
			Clip.strOccurrenceId = strPatternId + ".clip." +
				std::to_string(iOccurrenceOrdinal++);
			Clip.strStageId = Stage.strStageId;
			Clip.strSlotId = Slot.strSlotId;
			Clip.strRuntimeClip = strRuntimeClip;
			Clip.iSourceStartMs = iSourceStartMs;
			Clip.iPlayMs = iPlayMs;
			Clip.fPlayRate = fPlayRate;
			Clip.strEndPolicy = bLoop ? "LOOP_TO_WINDOW" :
				(fSourceWindowEndMs > fNativeDurationMs + 0.5 ?
					"HOLD_LAST_POSE" : "EXACT");
			Staged.Clips.push_back(std::move(Clip));
		}
	}

	if (Staged.Clips.empty())
	{
		strOutStatus = "The selected planner action has no exact physical clip occurrence; its HOLDOUT evidence remains read-only.";
		return false;
	}
	Staged.iNextOccurrenceOrdinal = iOccurrenceOrdinal;
	outPattern = std::move(Staged);
	strOutStatus = "Materialized " + std::to_string(outPattern.Clips.size()) +
		" ordered clip occurrence(s) from planner action " +
		std::to_string(Action.iSourceActionId) + ".";
	return true;
}

Client::CAnimation_Tool::KOUKU_COMPOSITION_PREVIEW_STATE
Client::CAnimation_Tool::Get_KoukuCompositionPreviewState() const
{
	KOUKU_COMPOSITION_PREVIEW_STATE state;
	state.strPatternId = m_strKoukuSaydonPatternPreviewId;
	state.strStatus = m_strKoukuSaydonPatternStatus;
	state.bPlaying = m_bKoukuCompositionTimelinePlaying;
	state.bPaused = m_bKoukuCompositionTimelinePlaying &&
		m_bKoukuSaydonPatternPreviewPaused;
	state.iDurationMs = m_iKoukuCompositionPreviewDurationMs;
	state.iClockMs = static_cast<std::uint32_t>(std::clamp(
		m_fKoukuCompositionPreviewClockMs, 0.0,
		static_cast<double>(m_iKoukuCompositionPreviewDurationMs)));
	return state;
}

bool_t Client::CAnimation_Tool::Set_KoukuCompositionPreviewPaused(
	const bool_t bPaused,
	std::string& strOutStatus)
{
	const auto model = m_KoukuSaydonPatternPreviewModel.lock();
	if (!m_bKoukuCompositionTimelinePlaying || nullptr == model ||
		CAnimationTargetService::Resolve_Model() != model ||
		m_iKoukuSaydonPatternPreviewTargetGeneration != CAnimationTargetService::Resolve_TargetGeneration())
	{
		strOutStatus = "No current KoukuSaydon composition preview is staged.";
		m_Status = m_strKoukuSaydonPatternStatus = strOutStatus;
		return false;
	}
	if (!bPaused && m_fKoukuCompositionPreviewClockMs >= m_iKoukuCompositionPreviewDurationMs)
		m_fKoukuCompositionPreviewClockMs = 0.0;
	m_bKoukuSaydonPatternPreviewPaused = bPaused;
	Sample_KoukuSaydonCompositionPreview(model);
	strOutStatus = bPaused ?
		"Composition preview paused." : "Composition preview resumed.";
	m_Status = m_strKoukuSaydonPatternStatus = strOutStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Seek_KoukuCompositionPreview(
	const std::uint32_t iClockMs,
	std::string& strOutStatus)
{
	const auto pModel = m_KoukuSaydonPatternPreviewModel.lock();
	if (!m_bKoukuCompositionTimelinePlaying || nullptr == pModel ||
		0u == m_iKoukuCompositionPreviewDurationMs ||
		CAnimationTargetService::Resolve_Model() != pModel ||
		m_iKoukuSaydonPatternPreviewTargetGeneration != CAnimationTargetService::Resolve_TargetGeneration())
	{
		strOutStatus = "No current KoukuSaydon composition preview is staged.";
		m_Status = m_strKoukuSaydonPatternStatus = strOutStatus;
		return false;
	}
	Reset_KoukuCompositionEffects();
	m_fKoukuCompositionPreviewClockMs = static_cast<double>((std::min)(
		iClockMs, m_iKoukuCompositionPreviewDurationMs));
	if (m_fKoukuCompositionPreviewClockMs >= m_iKoukuCompositionPreviewDurationMs)
		m_bKoukuSaydonPatternPreviewPaused = true;
	Sample_KoukuSaydonCompositionPreview(pModel);
	strOutStatus = "Composition preview seeked to " + std::to_string(
		static_cast<std::uint32_t>(m_fKoukuCompositionPreviewClockMs)) + " ms.";
	m_Status = m_strKoukuSaydonPatternStatus = strOutStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Stop_KoukuCompositionPreview(
	std::string& strOutStatus)
{
	const bool_t hadPending = m_bKoukuSaydonCompositionAnimationPreviewPending ||
		m_bKoukuSaydonCompositionPatternPreviewPending;
	m_bKoukuSaydonCompositionAnimationPreviewPending = false;
	m_bKoukuSaydonCompositionPatternPreviewPending = false;
	m_PendingKoukuSaydonCompositionAnimationPreview = {};
	m_PendingKoukuSaydonCompositionPatternPreview = {};
	if (!m_bKoukuCompositionTimelinePlaying)
	{
		strOutStatus = hadPending ? "Composition preview request cancelled." :
			"No KoukuSaydon composition preview is playing.";
		m_Status = m_strKoukuSaydonPatternStatus = strOutStatus;
		return hadPending;
	}
	Stop_KoukuSaydonPatternPreview(
		m_KoukuSaydonPatternPreviewModel.lock(), "Composition preview stopped.");
	strOutStatus = m_strKoukuSaydonPatternStatus;
	m_Status = strOutStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Start_PendingKoukuSaydonCompositionPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (!m_bKoukuSaydonCompositionAnimationPreviewPending && !m_bKoukuSaydonCompositionPatternPreviewPending)
		return true;
	const std::string targetAssetName = m_strPendingCompositionPreviewTargetAssetName;
	if (nullptr == pModel || !Is_CompositionAnimationTargetAsset(targetAssetName) ||
		CAnimationTargetService::Resolve_Model() != pModel ||
		CAnimationTargetService::Resolve_AssetName() != targetAssetName)
	{
		m_bKoukuSaydonCompositionAnimationPreviewPending = false;
		m_bKoukuSaydonCompositionPatternPreviewPending = false;
		m_Status = m_strKoukuSaydonPatternStatus = "Composition preview target is unavailable.";
		return false;
	}

	const std::string previewPatternId = m_bKoukuSaydonCompositionPatternPreviewPending ?
		m_PendingKoukuSaydonCompositionPatternPreview.strPatternId : std::string{};
	const std::uint32_t startClockMs = m_iPendingKoukuCompositionStartClockMs;
	const bool_t startPaused = m_bPendingKoukuCompositionStartPaused;
	std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE> rows;
	std::uint32_t durationMs = 0u;
	std::string diagnostics;
	const auto append = [&](KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE row,
		const std::uint32_t stageStart, const std::uint32_t stageDuration, const std::uint32_t poseStart = 0u)
	{
		std::uint32_t index = 0u;
		for (; index < pModel->Get_NumAnimations(); ++index)
		{
			const char* name = pModel->Get_AnimationName(index);
			if (nullptr != name && row.strRuntimeClip == name) break;
		}
		f32_t position = 0.f, ticks = 0.f;
		const f32_t tps = index < pModel->Get_NumAnimations() ? pModel->Get_AnimationTickPerSecond(index) : 0.f;
		const auto* rowProfile = Find_KoukuSaydonActionProfile(row.strProfileId);
		const std::string_view rowTarget = nullptr != rowProfile ?
			std::string_view{ rowProfile->pPreviewAssetName } : std::string_view{ row.strProfileId };
		double sourceStartSampleMs = 0.0;
		const bool valid = rowTarget == targetAssetName &&
			row.iPlayMs > 0u && std::isfinite(row.fPlayRate) && row.fPlayRate >= 0.01f && row.fPlayRate <= 16.f &&
			static_cast<std::uint64_t>(row.iStartOffsetMs) + row.iPlayMs <= stageDuration &&
			(row.strEndPolicy == "EXACT" || row.strEndPolicy == "HOLD_LAST_POSE" || row.strEndPolicy == "LOOP_TO_WINDOW") &&
			std::isfinite(tps) && tps > 0.f && pModel->Get_AnimationProgress(index, position, ticks) &&
			std::isfinite(ticks) && ticks > 0.f &&
			(row.strEndPolicy == "HOLD_LAST_POSE" || row.iSourceStartMs * tps * 0.001 < ticks) &&
            CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(row.iSourceStartMs, row.iSourceEndMs,
                0.0, row.fPlayRate, ticks * 1000.0 / tps, row.strEndPolicy == "LOOP_TO_WINDOW", sourceStartSampleMs);
		if (!valid)
		{
			diagnostics += "Skipped " + row.strRuntimeClip + ": wrong body, missing clip, or invalid timing. ";
			return;
		}
		/* An EXACT window that outruns the native clip is still the box the
		   designer drew: Sample clamps the source clock, so the remainder holds
		   the last pose and the Details panel reports the same overrun. */
		const double sourceEndMs = row.iSourceStartMs + static_cast<double>(row.iPlayMs) * row.fPlayRate;
		if (row.strEndPolicy == "EXACT" && sourceEndMs > ticks / tps * 1000.0 + 1.0)
			diagnostics += row.strRuntimeClip + " holds its last pose past the native clip end. ";
		row.iPoseStartMs = poseStart;
		row.iStartOffsetMs += stageStart;
		rows.push_back(std::move(row));
	};
	if (m_bKoukuSaydonCompositionAnimationPreviewPending)
	{
		auto row = m_PendingKoukuSaydonCompositionAnimationPreview;
		row.iStartOffsetMs = 0u;
		durationMs = row.iPlayMs;
		append(std::move(row), 0u, durationMs);
	}
	else
	{
		for (const auto& stage : m_PendingKoukuSaydonCompositionPatternPreview.Stages)
		{
			if (stage.iDurationMs > 600000u || durationMs > 600000u - stage.iDurationMs)
			{
				diagnostics += "Pattern duration exceeds 600 seconds. ";
				rows.clear();
				break;
			}
			const auto first = std::min_element(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
                [](const auto& a, const auto& b) { return a.iStartOffsetMs != b.iStartOffsetMs ?
                    a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; });
            for (const auto& row : stage.AnimationOccurrences)
                append(row, durationMs, stage.iDurationMs, durationMs + (&row == &*first ? 0u : row.iStartOffsetMs));
			durationMs += stage.iDurationMs;
		}
	}
	m_bKoukuSaydonCompositionAnimationPreviewPending = false;
	m_bKoukuSaydonCompositionPatternPreviewPending = false;
	if (rows.empty())
	{
		m_Status = m_strKoukuSaydonPatternStatus = "No playable Animation rows. " + diagnostics;
		return false;
	}
	std::sort(rows.begin(), rows.end(), [](const auto& left, const auto& right) {
		if (left.iStartOffsetMs != right.iStartOffsetMs) return left.iStartOffsetMs < right.iStartOffsetMs;
		return left.strOccurrenceId < right.strOccurrenceId;
	});
	Reset_ValtanPatternPreviewState({});
	Reset_ValtanPatternMasterPreviewState({});
	Reset_KoukuSaydonPatternPreviewState({});
	m_KoukuCompositionPreviewRows = std::move(rows);
	std::unordered_map<std::string, KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT> scaleReferences;
	for (const auto& row : m_KoukuCompositionPreviewRows)
	{
		f32_t scale = 1.f;
		if (row.strSourceStageId == "RAW")
			scale = Resolve_LargeNamePreviewScale(row.strRuntimeClip);
		else if (nullptr != Find_KoukuSaydonActionProfile(row.strProfileId))
		{
			auto [entry, inserted] = scaleReferences.try_emplace(row.strProfileId);
			if (inserted)
			{
				std::string bytes, status;
				const auto path = CKoukuSaydonAnimationActionDocument::Resolve_ReferencePath(row.strProfileId);
				if (!Read_BoundedFile(path, 16u * 1024u * 1024u, bytes, status) ||
					!CKoukuSaydonAnimationActionDocument::Parse_ReferenceText(bytes, entry->second, status, true) ||
					entry->second.strProfileId != row.strProfileId)
				{
					entry->second = {};
					diagnostics += "Preview scale reference unavailable for " + row.strProfileId + ": " + status + ". ";
				}
			}
			scale = Resolve_ActionPreviewScale(entry->second, row.iSourceActionId);
		}
		m_KoukuCompositionPreviewScales.push_back(scale);
	}
	if (std::any_of(m_KoukuCompositionPreviewScales.begin(), m_KoukuCompositionPreviewScales.end(),
			[](const f32_t scale) { return scale != 1.f; }))
	{
		char scaleNote[160]{};
		sprintf_s(scaleNote,
			"Large-named actions preview at the catalog big Saydon scale (x%.2f of the Saydon preview body). ",
			Large_SaydonPreviewMultiplier());
		diagnostics += scaleNote;
	}
	m_strKoukuSaydonPatternPreviewId = previewPatternId;
	m_iKoukuSaydonPatternPreviewClip = m_KoukuCompositionPreviewRows.size();
	m_iKoukuCompositionPreviewDurationMs = durationMs;
	m_iKoukuCompositionInitialAnimation = pModel->Get_CurrentAnimIndex();
	f32_t initialDuration = 0.f;
	m_fKoukuCompositionInitialTrackTicks = 0.f;
	(void)pModel->Get_AnimationProgress(m_iKoukuCompositionInitialAnimation,
		m_fKoukuCompositionInitialTrackTicks, initialDuration);
	m_fKoukuCompositionPreviewClockMs = (std::min)(startClockMs, durationMs);
	m_bKoukuSaydonPatternPreviewPaused = startPaused || startClockMs >= durationMs;
	m_KoukuSaydonPatternPreviewModel = pModel;
	m_iKoukuSaydonPatternPreviewTargetGeneration = CAnimationTargetService::Resolve_TargetGeneration();
	m_bKoukuSaydonPatternPreviewPlaying = true;
	m_bKoukuCompositionTimelinePlaying = true;
	m_Status = m_strKoukuSaydonPatternStatus =
		(previewPatternId.empty() ? "Animation row/resource preview. " : "Animation family preview. ") + diagnostics;
	if (!diagnostics.empty()) OutputDebugStringA(("[KoukuPreview] " + diagnostics + "\n").c_str());
	Sample_KoukuSaydonCompositionPreview(pModel);
	return true;
}

void Client::CAnimation_Tool::Apply_KoukuSaydonPreviewScale(
	const shared_ptr<Engine::CModel>& pModel, const f32_t multiplier)
{
	const std::string asset = CAnimationTargetService::Resolve_AssetName();
	if (nullptr != pModel && nullptr != m_pPreviewPanel &&
		(asset == "MN_RPCZ_00" || asset == "MN_RPCZ_00-1" || asset == "MN_RPCT_00" || asset == "MN_RPCT_03" ||
		 asset == "MN_RPCT_05" || asset == "MN_RPCT_06") &&
		m_pPreviewPanel->Set_PreviewScaleMultiplier(pModel, multiplier) && multiplier != 1.f)
		m_KoukuScaledPreviewModel = pModel;
	else if (multiplier == 1.f)
		m_KoukuScaledPreviewModel.reset();
}

void Client::CAnimation_Tool::Reset_KoukuCompositionEffects()
{
	CEffectV2Runtime::Reset_LocalPreviewTarget(m_KoukuCompositionEffectTarget);
	m_KoukuCompositionEffectTarget.Reset();
	m_strKoukuCompositionEffectOccurrence.clear();
	m_fKoukuCompositionEffectSourceSeconds = -1.f;
}

void Client::CAnimation_Tool::Sample_KoukuCompositionEffects(
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& row,
	const f32_t fSourceSeconds)
{
	const EFFECT_V2_TARGET target = nullptr != m_pPreviewPanel ?
		m_pPreviewPanel->Get_PreviewEffectTarget() : EFFECT_V2_TARGET{};
	if (!target.Is_Valid())
	{
		Reset_KoukuCompositionEffects();
		return;
	}
	if (target.pKey != m_KoukuCompositionEffectTarget.pKey ||
		row.strOccurrenceId != m_strKoukuCompositionEffectOccurrence ||
		fSourceSeconds + 0.00001f < m_fKoukuCompositionEffectSourceSeconds)
	{
		Reset_KoukuCompositionEffects();
		m_KoukuCompositionEffectTarget = target;
		m_strKoukuCompositionEffectOccurrence = row.strOccurrenceId;
		CEffectV2Runtime::Notify_Clip(target, row.strRuntimeClip.c_str());
	}
	CEffectV2Runtime::Sample_LocalClipPreview(target,
		m_bKoukuSaydonPatternPreviewPaused, row.fPlayRate,
		m_pPreviewDevice, m_pPreviewContext);
	m_fKoukuCompositionEffectSourceSeconds = fSourceSeconds;
}

void Client::CAnimation_Tool::Sample_KoukuSaydonCompositionPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (nullptr == pModel)
		return;
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* selected = nullptr;
	const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* previous = nullptr;
	double previousEndMs = 0.0;
	for (const auto& row : m_KoukuCompositionPreviewRows)
	{
		const double endMs = static_cast<double>(row.iStartOffsetMs) + row.iPlayMs;
		if (m_fKoukuCompositionPreviewClockMs >= row.iPoseStartMs &&
			m_fKoukuCompositionPreviewClockMs < endMs)
			selected = &row;
		if (endMs <= m_fKoukuCompositionPreviewClockMs && endMs >= previousEndMs)
		{
			previous = &row;
			previousEndMs = endMs;
		}
	}
	const bool_t hasActiveOccurrence = nullptr != selected;
	if (!hasActiveOccurrence) Reset_KoukuCompositionEffects();
	// Reconstruct a gap from its preceding row, rather than retaining whatever
	// later pose happened to be visible before a backward seek.
	double sampleClockMs = m_fKoukuCompositionPreviewClockMs;
	if (nullptr == selected)
	{
		selected = previous;
		sampleClockMs = previousEndMs;
	}
	if (nullptr == selected && !m_KoukuCompositionPreviewRows.empty())
	{
		// Hold the scheduled clip's first pose while pattern-relative Effects can begin.
		selected = &m_KoukuCompositionPreviewRows.front();
		sampleClockMs = selected->iStartOffsetMs;
	}
	if (nullptr == selected)
	{
		// An empty timeline preserves the pose captured when this preview began.
		Apply_KoukuSaydonPreviewScale(pModel, 1.f);
		if (m_iKoukuCompositionInitialAnimation < pModel->Get_NumAnimations())
		{
			if (pModel->Get_CurrentAnimIndex() != m_iKoukuCompositionInitialAnimation || pModel->Is_AnimLoop())
				(void)pModel->Start_Animation(m_iKoukuCompositionInitialAnimation, false);
			pModel->Set_AnimPaused(true);
			(void)pModel->Set_AnimTrackPosition(
				m_iKoukuCompositionInitialAnimation, m_fKoukuCompositionInitialTrackTicks);
			(void)pModel->Play_Animation(0.f);
		}
		m_iKoukuSaydonPatternPreviewClip = m_KoukuCompositionPreviewRows.size();
		if (nullptr != m_pPreviewPanel) m_pPreviewPanel->Synchronize_PreviewWeapon();
		return;
	}
	std::uint32_t index = 0u;
	for (; index < pModel->Get_NumAnimations(); ++index)
	{
		const char* name = pModel->Get_AnimationName(index);
		if (nullptr != name && selected->strRuntimeClip == name) break;
	}
	if (index >= pModel->Get_NumAnimations()) return;
	f32_t position = 0.f, duration = 0.f;
	const f32_t tps = pModel->Get_AnimationTickPerSecond(index);
	if (!pModel->Get_AnimationProgress(index, position, duration) ||
		!std::isfinite(tps) || tps <= 0.f || !std::isfinite(duration) || duration <= 0.f)
		return;
	const std::size_t rowIndex = static_cast<std::size_t>(selected - m_KoukuCompositionPreviewRows.data());
	Apply_KoukuSaydonPreviewScale(pModel, rowIndex < m_KoukuCompositionPreviewScales.size() ?
		m_KoukuCompositionPreviewScales[rowIndex] : 1.f);
	if (m_iKoukuSaydonPatternPreviewClip != rowIndex || pModel->Get_CurrentAnimIndex() != index)
	{
		if (!Start_PreviewClip(pModel, selected->strRuntimeClip.c_str(), false, 0.f)) return;
		m_iKoukuSaydonPatternPreviewClip = rowIndex;
	}
    double sourceMs = 0.0;
    const double ageMs = (std::max)(0.0, sampleClockMs - selected->iStartOffsetMs);
    if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(selected->iSourceStartMs,
        selected->iSourceEndMs, (std::min)(ageMs, double(selected->iPlayMs)), selected->fPlayRate,
        duration * 1000.0 / tps, selected->strEndPolicy == "LOOP_TO_WINDOW", sourceMs))
    {
        m_strKoukuSaydonPatternStatus = "Animation source range is outside the native clip.";
        Stop_KoukuSaydonPatternPreview(pModel, m_strKoukuSaydonPatternStatus);
        return;
    }
    const double sourceTicks = sourceMs * tps * .001;
    pModel->Clear_AnimationTransitionPose();
    pModel->Set_AnimPaused(true);
    (void)pModel->Set_AnimTrackPosition(index, static_cast<f32_t>(sourceTicks));
    (void)pModel->Play_Animation(0.f);
    if (rowIndex > 0u && selected->iBlendInMs && ageMs < selected->iBlendInMs)
    {
        const auto& blendFrom = m_KoukuCompositionPreviewRows[rowIndex - 1u];
        CModel::ANIMATION_TRANSITION_POSE pose;
        for (std::uint32_t i = 0u; i < pModel->Get_NumAnimations(); ++i)
            if (const auto* name = pModel->Get_AnimationName(i); name && blendFrom.strRuntimeClip == name)
            { pose.sourceIndex = i; break; }
        float previousCursor = 0.f, previousDuration = 0.f;
        if (pose.sourceIndex != UINT32_MAX && pModel->Get_AnimationProgress(pose.sourceIndex, previousCursor, previousDuration))
        {
            const float previousTps = pModel->Get_AnimationTickPerSecond(pose.sourceIndex);
            double previousMs = 0.0;
            if (CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(blendFrom.iSourceStartMs,
                blendFrom.iSourceEndMs, blendFrom.iPlayMs, blendFrom.fPlayRate,
                previousDuration * 1000.0 / previousTps, blendFrom.strEndPolicy == "LOOP_TO_WINDOW", previousMs))
            {
                pose.sourceTicks = float(previousMs * previousTps * .001);
                pose.targetIndex = index; pose.targetTicks = float(sourceTicks);
                pose.durationSeconds = selected->iBlendInMs * .001f;
                pose.elapsedSeconds = float(ageMs * .001); pose.playRate = selected->fPlayRate;
                (void)pModel->Set_AnimationTransitionPose(pose);
            }
        }
    }
	if (nullptr != m_pPreviewPanel) m_pPreviewPanel->Synchronize_PreviewWeapon();
	if (hasActiveOccurrence)
		Sample_KoukuCompositionEffects(*selected, static_cast<f32_t>(sourceTicks / tps));
}

bool_t Client::CAnimation_Tool::Start_KoukuSaydonPatternPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const KOUKU_SAYDON_ANIMATION_PATTERN& Pattern,
	const std::string& strLabel)
{
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		m_strKoukuSaydonPatternStatus =
			"KoukuSaydon Pattern preview rejected because the animation target changed.";
		return false;
	}
	const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* const pProfile =
		Find_KoukuSaydonActionProfile(m_strKoukuSaydonProfileId);
	if (nullptr == pProfile)
	{
		m_strKoukuSaydonPatternStatus =
			"KoukuSaydon Pattern preview requires an exact admitted action profile.";
		return false;
	}

	KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT Candidate = m_KoukuSaydonAnimationPatterns;
	const auto Found = std::find_if(
		Candidate.Patterns.begin(), Candidate.Patterns.end(),
		[&Pattern](const KOUKU_SAYDON_ANIMATION_PATTERN& Existing)
		{
			return Existing.strPatternId == Pattern.strPatternId;
		});
	if (Found == Candidate.Patterns.end())
	{
		Candidate.Patterns.push_back(Pattern);
		++Candidate.iNextPatternOrdinal;
	}
	else
	{
		*Found = Pattern;
	}
	std::string Status;
	if (!CKoukuSaydonAnimationPatternDocument::Validate(
		Candidate,
		m_KoukuSaydonActionReference,
		pProfile->pProfileId,
		pProfile->pModelAssetId,
		Collect_ClipNames(pModel),
		Status))
	{
		m_strKoukuSaydonPatternStatus =
			"KoukuSaydon Pattern preview rejected before pose commit: " + Status;
		return false;
	}

	if (m_bValtanPatternPreviewPlaying)
		Reset_ValtanPatternPreviewState(
			"Valtan source preview yielded to KoukuSaydon Pattern preview.");
	if (m_bValtanPatternMasterPlaying)
		Reset_ValtanPatternMasterPreviewState(
			"Valtan Pattern Master yielded to KoukuSaydon Pattern preview.");

	m_KoukuSaydonPatternPreviewClips = Pattern.Clips;
	m_fKoukuSaydonPatternPreviewScale = Resolve_ActionPreviewScale(m_KoukuSaydonActionReference, Pattern.iSourceActionId);
	m_strKoukuSaydonPatternPreviewId = Pattern.strPatternId;
	m_strKoukuSaydonPatternPreviewLabel = strLabel;
	m_KoukuSaydonPatternPreviewModel = pModel;
	m_iKoukuSaydonPatternPreviewTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iKoukuSaydonPatternPreviewClip = 0u;
	m_fKoukuSaydonPatternPreviewElapsedSeconds = 0.f;
	m_fKoukuSaydonPatternPreviewClipDurationSeconds = 0.f;
	m_bKoukuSaydonPatternPreviewPlaying = true;
	m_bKoukuSaydonPatternPreviewPaused = false;
	m_strKoukuSaydonPatternStatus = "Staged " +
		std::to_string(Pattern.Clips.size()) +
		" validated occurrence(s) for local pattern preview.";
	return Activate_KoukuSaydonPatternPreviewClip(pModel);
}

bool_t Client::CAnimation_Tool::Activate_KoukuSaydonPatternPreviewClip(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (nullptr == pModel || !m_bKoukuSaydonPatternPreviewPlaying ||
		m_iKoukuSaydonPatternPreviewClip >= m_KoukuSaydonPatternPreviewClips.size())
	{
		return false;
	}
	const KOUKU_SAYDON_ANIMATION_PATTERN_CLIP& Clip =
		m_KoukuSaydonPatternPreviewClips[m_iKoukuSaydonPatternPreviewClip];
	std::uint32_t iAnimation = 0u;
	for (; iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
	{
		const char_t* const pName = pModel->Get_AnimationName(iAnimation);
		if (nullptr != pName && Clip.strRuntimeClip == pName)
			break;
	}
	if (iAnimation >= pModel->Get_NumAnimations())
	{
		Stop_KoukuSaydonPatternPreview(
			pModel, "KoukuSaydon Pattern preview stopped: a staged clip disappeared from the physical WModel.");
		return false;
	}

	f32_t fPositionTicks = 0.f;
	f32_t fDurationTicks = 0.f;
	const f32_t fTicksPerSecond =
		pModel->Get_AnimationTickPerSecond(iAnimation);
	if (!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
		!pModel->Get_AnimationProgress(
			iAnimation, fPositionTicks, fDurationTicks) ||
		!std::isfinite(fDurationTicks) || fDurationTicks <= 0.f)
	{
		Stop_KoukuSaydonPatternPreview(
			pModel, "KoukuSaydon Pattern preview stopped: native clip timing is malformed.");
		return false;
	}
	const f32_t fSourceStartTicks =
		static_cast<f32_t>(Clip.iSourceStartMs) * fTicksPerSecond * 0.001f;
	if (!std::isfinite(fSourceStartTicks) ||
		fSourceStartTicks + 0.0001f >= fDurationTicks)
	{
		Stop_KoukuSaydonPatternPreview(
			pModel, "KoukuSaydon Pattern preview stopped: sourceStartMs is outside the native clip.");
		return false;
	}
	const f64_t fNativeDurationMs =
		static_cast<f64_t>(fDurationTicks) /
		static_cast<f64_t>(fTicksPerSecond) * 1000.0;
	const f64_t fSourceWindowEndMs =
		static_cast<f64_t>(Clip.iSourceStartMs) +
		static_cast<f64_t>(Clip.iPlayMs) *
		static_cast<f64_t>(Clip.fPlayRate);
	if (("EXACT" == Clip.strEndPolicy &&
		 fSourceWindowEndMs > fNativeDurationMs + 0.5) ||
		("HOLD_LAST_POSE" == Clip.strEndPolicy &&
		 fSourceWindowEndMs <= fNativeDurationMs + 0.5))
	{
		Stop_KoukuSaydonPatternPreview(
			pModel, "KoukuSaydon Pattern preview stopped: endPolicy disagrees with the native source window.");
		return false;
	}

	const bool_t bLoop = "LOOP_TO_WINDOW" == Clip.strEndPolicy;
	if (!Start_PreviewClip(
			pModel, Clip.strRuntimeClip.c_str(), bLoop,
			m_fPreviewBlendSeconds) ||
		!pModel->Set_AnimTrackPosition(iAnimation, fSourceStartTicks))
	{
		Stop_KoukuSaydonPatternPreview(
			pModel, "KoukuSaydon Pattern preview stopped because the next clip could not start.");
		return false;
	}
	pModel->Set_AnimationSpeed(Clip.fPlayRate);
	Apply_KoukuSaydonPreviewScale(pModel, m_fKoukuSaydonPatternPreviewScale);
	pModel->Set_AnimPaused(false);
	m_bLoop = bLoop;
	m_bKoukuSaydonPatternPreviewPaused = false;
	m_fKoukuSaydonPatternPreviewElapsedSeconds = 0.f;
	m_fKoukuSaydonPatternPreviewClipDurationSeconds =
		static_cast<f32_t>(Clip.iPlayMs) * 0.001f;
	m_strKoukuSaydonPatternStatus = "Playing " + m_strKoukuSaydonPatternPreviewLabel +
		" clip " + std::to_string(m_iKoukuSaydonPatternPreviewClip + 1u) + "/" +
		std::to_string(m_KoukuSaydonPatternPreviewClips.size()) + ": " +
		Clip.strRuntimeClip + " [" + Clip.strEndPolicy + "].";
	return true;
}

void Client::CAnimation_Tool::Advance_KoukuSaydonPatternPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (!m_bKoukuSaydonPatternPreviewPlaying)
		return;
	++m_iKoukuSaydonPatternPreviewClip;
	if (m_iKoukuSaydonPatternPreviewClip >= m_KoukuSaydonPatternPreviewClips.size())
	{
		Stop_KoukuSaydonPatternPreview(
			pModel, "KoukuSaydon Pattern preview completed; idle restored.");
		return;
	}
	(void)Activate_KoukuSaydonPatternPreviewClip(pModel);
}

std::string Client::CAnimation_Tool::Resolve_KoukuSaydonIdleClip() const
{
	const auto Action = std::find_if(
		m_KoukuSaydonActionReference.Actions.begin(),
		m_KoukuSaydonActionReference.Actions.end(),
		[](const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& Candidate)
		{
			return 0u == Candidate.iSourceActionId;
		});
	if (Action == m_KoukuSaydonActionReference.Actions.end())
		return {};
	for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& Stage : Action->Stages)
	{
		for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& Slot : Stage.Slots)
		{
			const KOUKU_SAYDON_ANIMATION_ACTION_BINDING* const pOverride =
				Find_KoukuSaydonActionBinding(
					Action->iSourceActionId, Stage.strStageId, Slot.strSlotId);
			return nullptr != pOverride ?
				pOverride->strRuntimeClip : Slot.strRuntimeClip;
		}
	}
	return {};
}

void Client::CAnimation_Tool::Stop_KoukuSaydonPatternPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const std::string& strStatus)
{
	if (nullptr != pModel)
	{
		pModel->Set_AnimationSpeed(1.f);
		if (m_bKoukuCompositionTimelinePlaying)
		{
			if (m_iKoukuCompositionInitialAnimation < pModel->Get_NumAnimations())
			{
				(void)pModel->Start_Animation(m_iKoukuCompositionInitialAnimation, false);
				(void)pModel->Set_AnimTrackPosition(
					m_iKoukuCompositionInitialAnimation, m_fKoukuCompositionInitialTrackTicks);
			}
			pModel->Set_AnimPaused(true);
			(void)pModel->Play_Animation(0.f);
		}
		else
		{
			const std::string strIdleClip = Resolve_KoukuSaydonIdleClip();
			if (strIdleClip.empty() || !pModel->Start_Animation(strIdleClip.c_str(), true))
				pModel->Set_AnimPaused(true);
		}
	}
	m_bLoop = true;
	Reset_KoukuSaydonPatternPreviewState(strStatus);
}

void Client::CAnimation_Tool::Reset_KoukuSaydonPatternPreviewState(
	const std::string& strStatus)
{
	Reset_KoukuCompositionEffects();
	Apply_KoukuSaydonPreviewScale(m_KoukuScaledPreviewModel.lock(), 1.f);
	m_fKoukuSaydonPatternPreviewScale = 1.f;
	m_KoukuCompositionPreviewScales.clear();
	m_bKoukuCompositionTimelinePlaying = false;
	m_strPendingCompositionPreviewTargetAssetName.clear();
	m_bKoukuSaydonCompositionAnimationPreviewPending = false;
	m_bKoukuSaydonCompositionPatternPreviewPending = false;
	m_KoukuCompositionPreviewRows.clear();
	m_fKoukuCompositionPreviewClockMs = 0.0;
	m_iKoukuCompositionPreviewDurationMs = 0u;
	m_KoukuSaydonPatternPreviewClips.clear();
	m_strKoukuSaydonPatternPreviewId.clear();
	m_strKoukuSaydonPatternPreviewLabel.clear();
	m_KoukuSaydonPatternPreviewModel.reset();
	m_iKoukuSaydonPatternPreviewTargetGeneration = 0u;
	m_iKoukuSaydonPatternPreviewClip = 0u;
	m_fKoukuSaydonPatternPreviewElapsedSeconds = 0.f;
	m_fKoukuSaydonPatternPreviewClipDurationSeconds = 0.f;
	m_bKoukuSaydonPatternPreviewPlaying = false;
	m_bKoukuSaydonPatternPreviewPaused = false;
	if (!strStatus.empty())
		m_strKoukuSaydonPatternStatus = strStatus;
}
