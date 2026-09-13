#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "ValtanBossTool.h"
#include "BalanceTool.h"
#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "Level_ValtanArena.h"
#include "Model.h"
#include "SoundCueCatalog.h"
#include "Valtan.h"
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




bool_t Client::CAnimation_Tool::Ensure_ValtanCompositionPatternSounds(
	std::string& strOutStatus)
{
	if (!m_bValtanPatternSoundCuesReady)
		(void)Reload_ValtanPatternSoundCues();
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return m_bValtanPatternSoundCuesReady;
}

bool_t Client::CAnimation_Tool::Reload_ValtanCompositionPatternSounds(
	std::string& strOutStatus)
{
	const bool_t bReloaded = Reload_ValtanPatternSoundCues();
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return bReloaded;
}

bool_t Client::CAnimation_Tool::
Discard_ValtanCompositionPatternSoundDraftAndReload(
	std::string& strOutStatus)
{
	const bool_t bDiscardedDraft = m_bValtanPatternSoundCuesDirty;
	if (!Reload_ValtanPatternSoundCues())
	{
		strOutStatus = m_strValtanPatternSoundCueStatus;
		return false;
	}
	m_strValtanPatternSoundCueStatus = bDiscardedDraft ?
		"Discarded the unsaved Pattern Sound composition draft and reloaded the physical owner." :
		"Reloaded the Pattern Sound composition owner; no draft required discard.";
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return true;
}

bool_t Client::CAnimation_Tool::
Can_CommitValtanCompositionPatternSoundGeneration(
	std::string& strOutStatus)
{
	if (nullptr == m_pValtanBossTool)
	{
		strOutStatus =
			"Pattern Sound Save/Load requires Valtan Boss Tool to be available.";
		return false;
	}
	return m_pValtanBossTool->Can_CommitPatternSoundGeneration(strOutStatus);
}

bool_t Client::CAnimation_Tool::Is_ValtanCompositionPatternSoundRuntimeReady(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus) const
{
	if (!ExpectedRevision.Is_Valid())
	{
		strOutStatus =
			"Pattern Sound runtime apply requires one valid Server-active Pattern revision.";
		return false;
	}
	if (m_bValtanPatternSoundCuesDirty)
	{
		strOutStatus =
			"Pattern Sound playback is blocked by an unsaved Sound owner draft.";
		return false;
	}
	if (!m_bValtanPatternSoundRuntimeApplyReady ||
		m_ValtanPatternSoundRuntimeAppliedRevision != ExpectedRevision)
	{
		strOutStatus = m_strValtanPatternSoundCueStatus.empty() ?
			"Pattern Sound active-consumer apply is pending for the exact Server-active Pattern revision." :
			m_strValtanPatternSoundCueStatus;
		return false;
	}
	strOutStatus =
		"Pattern Sound consumers are pinned to Server revision " +
		LostArk::Shared::Format_GameplayDataRevision(ExpectedRevision) + ".";
	return true;
}

bool_t Client::CAnimation_Tool::Apply_ValtanCompositionPatternSoundsToActiveConsumers(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus)
{
	if (!Can_CommitValtanCompositionPatternSoundGeneration(strOutStatus))
		return false;
	if (!ExpectedRevision.Is_Valid())
	{
		m_bValtanPatternSoundRuntimeApplyReady = false;
		m_ValtanPatternSoundRuntimeAppliedRevision = {};
		strOutStatus =
			"Pattern Sound consumer reload requires one valid revision receipt from Valtan Boss Tool.";
		return false;
	}
	std::string ArenaStatus =
		"No active Valtan Arena; Server Pattern playback admission cannot be established until the primary replicated Valtan consumer exists.";
	bool_t bArenaReloaded = false;
	if (CLevel_ValtanArena* const pArena = CLevel_ValtanArena::Get_Active())
	{
		bArenaReloaded =
			pArena->Reload_PrimaryValtanPresentationAuthoring(
				ExpectedRevision, ArenaStatus);
	}
	std::string PreviewStatus =
		"No Development preview Valtan is active.";
	bool_t bPreviewReloaded = true;
	if (const shared_ptr<CValtan> Boss =
			CAnimationTargetService::Resolve_Boss())
	{
		bPreviewReloaded =
			Boss->Reload_PatternPresentationAuthoring(PreviewStatus);
		if (!bPreviewReloaded)
		{
			PreviewStatus =
				"Development preview reload rejected: " + PreviewStatus;
		}
	}
	m_bValtanPatternSoundRuntimeApplyReady =
		bArenaReloaded && bPreviewReloaded;
	m_ValtanPatternSoundRuntimeAppliedRevision =
		m_bValtanPatternSoundRuntimeApplyReady ? ExpectedRevision :
			LostArk::Shared::GameplayDataRevision{};
	strOutStatus =
		(bArenaReloaded ? ArenaStatus :
			"ACTIVE ARENA RELOAD REJECTED: " + ArenaStatus) + " " +
		PreviewStatus +
		" Consumer reload receipt is provisional until Valtan Boss Tool revalidates the exact Server revision.";
	return m_bValtanPatternSoundRuntimeApplyReady;
}

bool_t Client::CAnimation_Tool::Retry_ValtanCompositionPatternSoundRuntimeApply(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus)
{
	if (!Can_CommitValtanCompositionPatternSoundGeneration(strOutStatus))
		return false;
	if (!ExpectedRevision.Is_Valid())
	{
		m_bValtanPatternSoundRuntimeApplyReady = false;
		m_ValtanPatternSoundRuntimeAppliedRevision = {};
		strOutStatus =
			"Pattern Sound runtime apply rejected because Valtan Boss Tool supplied no valid expected Pattern revision.";
		return false;
	}
	if (m_bValtanPatternSoundCuesDirty)
	{
		strOutStatus =
			"Pattern Sound runtime apply is blocked by an unsaved Sound owner draft.";
		return false;
	}
	if (!Reload_ValtanPatternSoundCues())
	{
		m_bValtanPatternSoundRuntimeApplyReady = false;
		m_ValtanPatternSoundRuntimeAppliedRevision = {};
		strOutStatus = m_strValtanPatternSoundCueStatus;
		return false;
	}
	std::string ApplyStatus;
	const bool_t bApplied =
		Apply_ValtanCompositionPatternSoundsToActiveConsumers(
			ExpectedRevision, ApplyStatus);
	m_strValtanPatternSoundCueStatus = bApplied ?
		"Pattern Sound source rejoined and applied to every active consumer at exact Server revision " +
			LostArk::Shared::Format_GameplayDataRevision(ExpectedRevision) + ". " +
			ApplyStatus :
		"Pattern Sound source rejoined, but one or more active consumers preserved their previous cache. Complete Play/Restart remain blocked until Retry Apply succeeds or the Arena is re-entered. " +
			ApplyStatus;
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return bApplied;
}

void Client::CAnimation_Tool::Invalidate_ValtanCompositionPatternSoundRuntimeApply(
	const std::string& strStatus)
{
	m_bValtanPatternSoundRuntimeApplyReady = false;
	m_ValtanPatternSoundRuntimeAppliedRevision = {};
	m_strValtanPatternSoundCueStatus = strStatus.empty() ?
		"Pattern Sound runtime receipt was invalidated before exact revision commit." :
		strStatus;
}

const Client::VALTAN_PATTERN_SOUND_CUE_DOCUMENT*
Client::CAnimation_Tool::Get_ValtanCompositionPatternSoundDraft(
	bool_t& bOutDirty,
	std::string& strOutStatus) const
{
	bOutDirty = m_bValtanPatternSoundCuesDirty;
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return m_bValtanPatternSoundCuesReady ?
		&m_ValtanPatternSoundCues : nullptr;
}

std::vector<std::string>
Client::CAnimation_Tool::Collect_ValtanCompositionPatternSoundEvents() const
{
	const std::vector<std::string> AllEvents =
		CSoundCueCatalog::Collect_EventNames("Valtan");
	std::vector<std::string> Admitted;
	Admitted.reserve(AllEvents.size());
	for (const std::string& Event : AllEvents)
	{
		if (IsValtanSoundAuthoringCandidate(Event))
			Admitted.push_back(Event);
	}
	return Admitted;
}

bool_t Client::CAnimation_Tool::Resolve_ValtanCompositionPatternSoundWindow(
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	uint32_t& iOutMinimumStartMs,
	uint32_t& iOutMaximumStartMs,
	bool_t& bOutLoop,
	std::string& strOutStatus) const
{
	const auto Clip = std::find_if(
		Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
		[&strClipOccurrenceId](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
		{
			return Candidate.strClipOccurrenceId == strClipOccurrenceId;
		});
	if (Stage.ClipOccurrences.end() == Clip || 0u == Stage.iDurationMs)
	{
		strOutStatus =
			"Pattern Sound occurrence does not resolve one admitted clip/stage wall.";
		return false;
	}

	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel)
	{
		m_ValtanPatternSoundDurationModel.reset();
		m_ValtanPatternSoundClipDurations.clear();
	}
	else if (m_ValtanPatternSoundDurationModel.lock() != pModel)
	{
		m_ValtanPatternSoundClipDurations =
			CollectModelClipSourceDurationSeconds(pModel);
		m_ValtanPatternSoundDurationModel = pModel;
	}
	const std::unordered_map<std::string, f32_t>& Durations =
		m_ValtanPatternSoundClipDurations;
	if (Durations.empty())
	{
		strOutStatus =
			"Pattern Sound timing edit requires the admitted Valtan model source durations.";
		return false;
	}

	std::vector<ACTION_PRESENTATION_CLIP_TIMING> Timings;
	Timings.reserve(Stage.ClipOccurrences.size());
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
		Stage.ClipOccurrences)
	{
		const auto Duration = Durations.find(Occurrence.strClipName);
		if (Durations.end() == Duration)
		{
			strOutStatus = "Pattern Sound timing edit is missing model clip: " +
				Occurrence.strClipName + ".";
			return false;
		}
		ACTION_PRESENTATION_CLIP_TIMING Timing{
			Duration->second,
			Occurrence.iPlayMs,
			Occurrence.fPlayRate,
			Occurrence.bLoop,
			static_cast<f32_t>(Occurrence.iSourceStartMs) * 0.001f };
		f32_t fSourceDurationSeconds = 0.f;
		f32_t fWallDurationSeconds = 0.f;
		if (!CActionPresentationTimeline::Resolve_ClipDuration(
				Timing, fSourceDurationSeconds, fWallDurationSeconds))
		{
			strOutStatus = "Pattern Sound timing edit rejected invalid clip timing: " +
				Occurrence.strClipOccurrenceId + ".";
			return false;
		}
		Timings.push_back(Timing);
	}

	const std::size_t iClipIndex = static_cast<std::size_t>(
		Clip - Stage.ClipOccurrences.begin());
	f32_t fResolvedSourceDurationSeconds = 0.f;
	f32_t fResolvedWallDurationSeconds = 0.f;
	f32_t fClipStageWallStartSeconds = 0.f;
	if (!CActionPresentationTimeline::Resolve_ClipDuration(
			Timings[iClipIndex], fResolvedSourceDurationSeconds,
			fResolvedWallDurationSeconds) ||
		!CActionPresentationTimeline::Resolve_CueWallOffset(
			Timings, iClipIndex,
			Timings[iClipIndex].fSourceStartSeconds,
			0u, fClipStageWallStartSeconds))
	{
		strOutStatus =
			"Pattern Sound timing edit could not resolve the clip on the Stage wall.";
		return false;
	}

	const f64_t fRemainingStageWallSeconds = (std::max)(0.0,
		static_cast<f64_t>(Stage.iDurationMs) * 0.001 -
		static_cast<f64_t>(fClipStageWallStartSeconds));
	const f64_t fResolvedSourceEndSeconds =
		static_cast<f64_t>(Clip->iSourceStartMs) * 0.001 +
		static_cast<f64_t>(fResolvedSourceDurationSeconds);
	const f64_t fStageSourceEndSeconds =
		static_cast<f64_t>(Clip->iSourceStartMs) * 0.001 +
		fRemainingStageWallSeconds * static_cast<f64_t>(Clip->fPlayRate);
	const f64_t fEffectiveSourceEndMilliseconds = 1000.0 *
		(std::min)(fResolvedSourceEndSeconds, fStageSourceEndSeconds);
	if (!std::isfinite(fEffectiveSourceEndMilliseconds) ||
		fEffectiveSourceEndMilliseconds <=
			static_cast<f64_t>(Clip->iSourceStartMs))
	{
		strOutStatus =
			"Pattern Sound timing edit has no source sample inside the Stage wall.";
		return false;
	}
	const std::uint64_t iSourceEndExclusiveMs =
		static_cast<std::uint64_t>(std::ceil(fEffectiveSourceEndMilliseconds));
	if (iSourceEndExclusiveMs <= Clip->iSourceStartMs)
	{
		strOutStatus =
			"Pattern Sound timing edit resolved an empty source window.";
		return false;
	}

	iOutMinimumStartMs = Clip->iSourceStartMs;
	iOutMaximumStartMs = static_cast<uint32_t>((std::min)(
		iSourceEndExclusiveMs - 1u,
		static_cast<std::uint64_t>(
			(std::numeric_limits<uint32_t>::max)())));
	bOutLoop = Clip->bLoop;
	strOutStatus = "Resolved Pattern Sound source window " +
		std::to_string(iOutMinimumStartMs) + ".." +
		std::to_string(iOutMaximumStartMs) + " ms.";
	return true;
}

bool_t Client::CAnimation_Tool::
Validate_ValtanCompositionPatternSoundStageDependencies(
	const VALTAN_PATTERN_VIEW& BaselinePattern,
	const VALTAN_STAGE_VIEW& BaselineStage,
	const VALTAN_STAGE_VIEW& CandidateStage,
	std::string& strOutStatus) const
{
	if (!m_bValtanPatternSoundCuesReady)
	{
		strOutStatus =
			"Pattern/Animation mutation requires the admitted Pattern Sound source before dependency validation.";
		return false;
	}
	if (BaselinePattern.strPatternId.empty() ||
		BaselineStage.strStageId.empty() ||
		BaselineStage.strStageId != CandidateStage.strStageId)
	{
		strOutStatus =
			"Pattern Sound dependency admission requires one exact baseline/candidate Pattern and Stage identity.";
		return false;
	}

	std::vector<const VALTAN_PATTERN_SOUND_CUE*> Rows;
	for (const VALTAN_PATTERN_SOUND_CUE& Cue : m_ValtanPatternSoundCues.Cues)
	{
		if (Cue.strPatternId == BaselinePattern.strPatternId &&
			Cue.strStageId == BaselineStage.strStageId)
		{
			Rows.push_back(&Cue);
		}
	}
	/* Do not manufacture a model requirement for a Stage that owns no Sound
	   dependency.  The loaded source inventory above is still mandatory so an
	   unavailable owner cannot be mistaken for an empty Stage. */
	if (Rows.empty())
	{
		strOutStatus = "No Pattern Sound dependency is attached to this Stage.";
		return true;
	}
	const bool_t bValidateTimingWindow =
		BaselineStage.strActionId != CandidateStage.strActionId ||
		BaselineStage.iDurationMs != CandidateStage.iDurationMs ||
		BaselineStage.ClipOccurrences.size() !=
			CandidateStage.ClipOccurrences.size() ||
		!std::equal(
			BaselineStage.ClipOccurrences.begin(),
			BaselineStage.ClipOccurrences.end(),
			CandidateStage.ClipOccurrences.begin(),
			[](const VALTAN_CLIP_OCCURRENCE_VIEW& Baseline,
				const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
			{
				return Baseline.strClipOccurrenceId ==
						Candidate.strClipOccurrenceId &&
					Baseline.strClipName == Candidate.strClipName &&
					Baseline.strMappingBasis == Candidate.strMappingBasis &&
					Baseline.iSourceStartMs == Candidate.iSourceStartMs &&
					Baseline.iPlayMs == Candidate.iPlayMs &&
					Baseline.fPlayRate == Candidate.fPlayRate &&
					Baseline.bLoop == Candidate.bLoop;
			});

	for (const VALTAN_PATTERN_SOUND_CUE* const pCue : Rows)
	{
		const VALTAN_PATTERN_SOUND_CUE& Cue = *pCue;
		if (Cue.strActionId != BaselineStage.strActionId ||
			Cue.strActionId != CandidateStage.strActionId)
		{
			strOutStatus =
				"Pattern Sound row no longer resolves the exact Pattern/Stage/action tuple: " +
				Cue.strOccurrenceId + ".";
			return false;
		}
		const std::size_t iBaselineClipCount = static_cast<std::size_t>(
			std::count_if(
				BaselineStage.ClipOccurrences.begin(),
				BaselineStage.ClipOccurrences.end(),
				[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
				{
					return Clip.strClipOccurrenceId == Cue.strClipOccurrenceId;
				}));
		const std::size_t iCandidateClipCount = static_cast<std::size_t>(
			std::count_if(
				CandidateStage.ClipOccurrences.begin(),
				CandidateStage.ClipOccurrences.end(),
				[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
				{
					return Clip.strClipOccurrenceId == Cue.strClipOccurrenceId;
				}));
		const auto BaselineClip = std::find_if(
			BaselineStage.ClipOccurrences.begin(),
			BaselineStage.ClipOccurrences.end(),
			[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId == Cue.strClipOccurrenceId;
			});
		const auto CandidateClip = std::find_if(
			CandidateStage.ClipOccurrences.begin(),
			CandidateStage.ClipOccurrences.end(),
			[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId == Cue.strClipOccurrenceId;
			});
		if (1u != iBaselineClipCount || 1u != iCandidateClipCount ||
			BaselineStage.ClipOccurrences.end() == BaselineClip ||
			CandidateStage.ClipOccurrences.end() == CandidateClip)
		{
			strOutStatus =
				"Pattern Sound row would dangle or become ambiguous because its clipOccurrenceId does not resolve exactly once: " +
				Cue.strClipOccurrenceId + ".";
			return false;
		}
		if (BaselineClip->strClipName != CandidateClip->strClipName &&
			"PROJECT_AUTHORED" != CandidateClip->strMappingBasis)
		{
			strOutStatus =
				"Pattern Sound-qualified clipOccurrenceId can change its resource only through an explicit PROJECT_AUTHORED slot replacement: " +
					Cue.strClipOccurrenceId + ".";
			return false;
		}

		/* Canonical Save must preserve exact owner/action/occurrence identity for
		   every Sound row, including legacy rows.  A pre-existing timing debt in
		   an otherwise unchanged Stage is not re-admitted here: strict source
		   window admission is required only when this save changes a field that
		   can affect the Sound wall clock.  Direct Stage mutations that alter
		   those fields therefore cannot create or extend that debt. */
		if (!bValidateTimingWindow)
			continue;

		uint32_t iMinimumStartMs = 0u;
		uint32_t iMaximumStartMs = 0u;
		bool_t bLoop = false;
		if (!Resolve_ValtanCompositionPatternSoundWindow(
				CandidateStage, Cue.strClipOccurrenceId,
				iMinimumStartMs, iMaximumStartMs, bLoop, strOutStatus))
		{
			strOutStatus =
				"Pattern Sound dependency timing rejected " +
				Cue.strOccurrenceId + ": " + strOutStatus;
			return false;
		}
		if (Cue.iStartMs < iMinimumStartMs ||
			Cue.iStartMs > iMaximumStartMs ||
			(VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				Cue.eRepeatPolicy && !bLoop))
		{
			strOutStatus =
				"Pattern Sound row is outside the candidate clip/Stage window or keeps each_loop on a non-loop clip: " +
				Cue.strOccurrenceId + ".";
			return false;
		}
	}
	strOutStatus = "Validated " + std::to_string(Rows.size()) +
		" Pattern Sound dependency row(s) against the candidate Stage" +
		(bValidateTimingWindow ? " with strict timing admission." :
			"; unchanged timing debt was not expanded.");
	return true;
}

bool_t Client::CAnimation_Tool::
Validate_ValtanCompositionPatternSoundGraphDependencies(
	const std::vector<VALTAN_PATTERN_VIEW>& BaselinePatterns,
	const std::vector<VALTAN_PATTERN_VIEW>& CandidatePatterns,
	std::string& strOutStatus) const
{
	if (!m_bValtanPatternSoundCuesReady)
	{
		strOutStatus =
			"Canonical Save requires the admitted Pattern Sound source before dependency validation.";
		return false;
	}
	if (m_ValtanPatternSoundCues.Cues.empty())
	{
		strOutStatus = "The admitted Pattern Sound source has no dependency rows.";
		return true;
	}

	const auto FindUniquePattern = [](
		const std::vector<VALTAN_PATTERN_VIEW>& Patterns,
		const std::string& strPatternId,
		const VALTAN_PATTERN_VIEW*& pOutPattern)
	{
		pOutPattern = nullptr;
		for (const VALTAN_PATTERN_VIEW& Pattern : Patterns)
		{
			if (Pattern.strPatternId != strPatternId)
				continue;
			if (nullptr != pOutPattern)
				return false;
			pOutPattern = &Pattern;
		}
		return nullptr != pOutPattern;
	};
	const auto FindUniqueStage = [](
		const VALTAN_PATTERN_VIEW& Pattern,
		const std::string& strStageId,
		const VALTAN_STAGE_VIEW*& pOutStage)
	{
		pOutStage = nullptr;
		for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
		{
			if (Stage.strStageId != strStageId)
				continue;
			if (nullptr != pOutStage)
				return false;
			pOutStage = &Stage;
		}
		return nullptr != pOutStage;
	};

	std::unordered_set<std::string> ValidatedStages;
	for (const VALTAN_PATTERN_SOUND_CUE& Cue : m_ValtanPatternSoundCues.Cues)
	{
		const std::string StageKey = Cue.strPatternId + "/" + Cue.strStageId;
		if (!ValidatedStages.insert(StageKey).second)
			continue;

		const VALTAN_PATTERN_VIEW* pBaselinePattern = nullptr;
		const VALTAN_PATTERN_VIEW* pCandidatePattern = nullptr;
		const VALTAN_STAGE_VIEW* pBaselineStage = nullptr;
		const VALTAN_STAGE_VIEW* pCandidateStage = nullptr;
		if (!FindUniquePattern(
				BaselinePatterns, Cue.strPatternId, pBaselinePattern) ||
			!FindUniquePattern(
				CandidatePatterns, Cue.strPatternId, pCandidatePattern) ||
			!FindUniqueStage(*pBaselinePattern, Cue.strStageId, pBaselineStage) ||
			!FindUniqueStage(*pCandidatePattern, Cue.strStageId, pCandidateStage))
		{
			strOutStatus =
				"Pattern Sound dependency does not resolve exactly one candidate Pattern/Stage: " +
				StageKey + ".";
			return false;
		}
		if (!Validate_ValtanCompositionPatternSoundStageDependencies(
				*pBaselinePattern, *pBaselineStage, *pCandidateStage,
				strOutStatus))
		{
			return false;
		}
	}
	strOutStatus = "Validated all " +
		std::to_string(m_ValtanPatternSoundCues.Cues.size()) +
		" Pattern Sound row(s) against the complete candidate Pattern graph.";
	return true;
}

bool_t Client::CAnimation_Tool::Patch_ValtanCompositionPatternSound(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strOccurrenceId,
	const std::string& strSoundEvent,
	const uint32_t iStartMs,
	const VALTAN_PATTERN_SOUND_REPEAT_POLICY eRepeatPolicy,
	std::string& strOutStatus)
{
	const std::string StableOccurrenceId = strOccurrenceId;
	const std::string StableSoundEvent = strSoundEvent;
	std::string AuthoringRevision;
	std::string AuthoringStatus;
	bool_t bCanonicalDraftDirty = false;
	if (Is_ValtanCompositionPatternTransactionActive() ||
		nullptr == m_pBalanceTool ||
		!m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bCanonicalDraftDirty, AuthoringStatus))
	{
		strOutStatus = Is_ValtanCompositionPatternTransactionActive() ?
			"Pattern Sound patch is blocked while Create New Pattern owns the Pattern dependency transaction." :
			"Pattern Sound patch requires an admitted Pattern/Animation source generation: " +
				AuthoringStatus;
		return false;
	}
	if (!m_bValtanPatternSoundCuesReady || StableOccurrenceId.empty() ||
		(eRepeatPolicy != VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE &&
		 eRepeatPolicy != VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP))
	{
		strOutStatus =
			"Pattern Sound patch requires one admitted stable occurrence.";
		return false;
	}
	const std::vector<std::string> Events =
		Collect_ValtanCompositionPatternSoundEvents();
	if (Events.end() == std::find(
			Events.begin(), Events.end(), StableSoundEvent))
	{
		strOutStatus =
			"Pattern Sound patch rejected an event without admitted Valtan assets.";
		return false;
	}

	uint32_t iMinimumStartMs = 0u;
	uint32_t iMaximumStartMs = 0u;
	bool_t bLoop = false;
	const auto Current = std::find_if(
		m_ValtanPatternSoundCues.Cues.begin(),
		m_ValtanPatternSoundCues.Cues.end(),
		[&](const VALTAN_PATTERN_SOUND_CUE& Cue)
		{
			return Cue.strOccurrenceId == StableOccurrenceId &&
				Cue.strPatternId == Pattern.strPatternId &&
				Cue.strStageId == Stage.strStageId &&
				Cue.strActionId == Stage.strActionId;
		});
	if (m_ValtanPatternSoundCues.Cues.end() == Current ||
		!Resolve_ValtanCompositionPatternSoundWindow(
			Stage, Current->strClipOccurrenceId,
			iMinimumStartMs, iMaximumStartMs, bLoop, strOutStatus))
	{
		if (m_ValtanPatternSoundCues.Cues.end() == Current)
			strOutStatus =
				"Pattern Sound patch did not resolve the exact selected tuple.";
		return false;
	}
	if (iStartMs < iMinimumStartMs || iStartMs > iMaximumStartMs ||
		(VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP == eRepeatPolicy &&
		 !bLoop))
	{
		strOutStatus =
			"Pattern Sound patch is outside the model/Stage source window or requests each_loop on a non-loop clip.";
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged = m_ValtanPatternSoundCues;
	VALTAN_PATTERN_SOUND_CUE& Candidate = Staged.Cues[
		static_cast<std::size_t>(Current - m_ValtanPatternSoundCues.Cues.begin())];
	Candidate.strSoundEvent = StableSoundEvent;
	Candidate.strSoundBank = std::string(
		ValtanSoundBankForEvent(StableSoundEvent));
	Candidate.iStartMs = iStartMs;
	Candidate.eRepeatPolicy = eRepeatPolicy;
	if (Candidate == *Current)
	{
		strOutStatus = "Pattern Sound draft is unchanged.";
		return true;
	}
	m_ValtanPatternSoundCues = std::move(Staged);
	m_bValtanPatternSoundCuesDirty = true;
	++m_iValtanPatternSoundDraftGeneration;
	m_strValtanPatternSoundCueStatus =
		"UNSAVED Pattern Sound occurrence: " + StableOccurrenceId + ".";
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Add_ValtanCompositionPatternSound(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	const std::string& strSoundEvent,
	const uint32_t iStartMs,
	const VALTAN_PATTERN_SOUND_REPEAT_POLICY eRepeatPolicy,
	VALTAN_PATTERN_SOUND_CUE_ROW_ID& OutCreatedRowId,
	std::string& strOutStatus)
{
	std::string AuthoringRevision;
	std::string AuthoringStatus;
	bool_t bCanonicalDraftDirty = false;
	if (Is_ValtanCompositionPatternTransactionActive() ||
		nullptr == m_pBalanceTool ||
		!m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bCanonicalDraftDirty, AuthoringStatus))
	{
		strOutStatus = Is_ValtanCompositionPatternTransactionActive() ?
			"Pattern Sound Add is blocked while Create New Pattern owns the Pattern dependency transaction." :
			"Pattern Sound Add requires an admitted Pattern/Animation source generation: " +
				AuthoringStatus;
		return false;
	}
	if (!m_bValtanPatternSoundCuesReady ||
		Pattern.strPatternId.empty() || Stage.strStageId.empty() ||
		Stage.strActionId.empty() || strClipOccurrenceId.empty() ||
		(eRepeatPolicy != VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE &&
		 eRepeatPolicy != VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP))
	{
		strOutStatus =
			"Pattern Sound Add requires one admitted Pattern/Stage/clip occurrence.";
		return false;
	}
	const std::vector<std::string> Events =
		Collect_ValtanCompositionPatternSoundEvents();
	if (Events.end() == std::find(
			Events.begin(), Events.end(), strSoundEvent))
	{
		strOutStatus =
			"Pattern Sound Add rejected an event without admitted Valtan assets.";
		return false;
	}

	uint32_t iMinimumStartMs = 0u;
	uint32_t iMaximumStartMs = 0u;
	bool_t bLoop = false;
	if (!Resolve_ValtanCompositionPatternSoundWindow(
			Stage, strClipOccurrenceId, iMinimumStartMs, iMaximumStartMs,
			bLoop, strOutStatus))
	{
		return false;
	}
	if (iStartMs < iMinimumStartMs || iStartMs > iMaximumStartMs ||
		(VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP == eRepeatPolicy &&
		 !bLoop))
	{
		strOutStatus =
			"Pattern Sound Add is outside the selected clip source window or requests each_loop on a non-loop clip.";
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_ADD_ROW Row;
	Row.strPatternId = Pattern.strPatternId;
	Row.strStageId = Stage.strStageId;
	Row.strActionId = Stage.strActionId;
	Row.strClipOccurrenceId = strClipOccurrenceId;
	Row.strSoundEvent = strSoundEvent;
	Row.strSoundBank = std::string(ValtanSoundBankForEvent(strSoundEvent));
	Row.eRepeatPolicy = eRepeatPolicy;
	Row.iStartMs = iStartMs;
	const std::unordered_map<std::string, f32_t> Durations =
		CollectModelClipSourceDurationSeconds(Resolve_Model());
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged = m_ValtanPatternSoundCues;
	VALTAN_PATTERN_SOUND_CUE_ROW_ID CreatedRowId;
	std::string AddStatus;
	if (!CValtanPatternSoundCueDocument::Add_AuthoringRow(
			Staged, Row, Durations, CreatedRowId, AddStatus))
	{
		strOutStatus =
			"Pattern Sound Add rejected; admitted draft preserved: " + AddStatus;
		return false;
	}
	m_ValtanPatternSoundCues = std::move(Staged);
	m_bValtanPatternSoundCuesDirty = true;
	++m_iValtanPatternSoundDraftGeneration;
	OutCreatedRowId = CreatedRowId;
	m_strValtanPatternSoundCueStatus =
		"UNSAVED Pattern Sound row added: " + CreatedRowId.strBindingId +
		" / " + CreatedRowId.strOccurrenceId + ". " + AddStatus;
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Remove_ValtanCompositionPatternSound(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_PATTERN_SOUND_CUE_ROW_ID& RowId,
	std::string& strOutStatus)
{
	std::string AuthoringRevision;
	std::string AuthoringStatus;
	bool_t bCanonicalDraftDirty = false;
	if (Is_ValtanCompositionPatternTransactionActive() ||
		nullptr == m_pBalanceTool ||
		!m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bCanonicalDraftDirty, AuthoringStatus))
	{
		strOutStatus = Is_ValtanCompositionPatternTransactionActive() ?
			"Pattern Sound Remove is blocked while Create New Pattern owns the Pattern dependency transaction." :
			"Pattern Sound Remove requires an admitted Pattern/Animation source generation: " +
				AuthoringStatus;
		return false;
	}
	if (!m_bValtanPatternSoundCuesReady)
	{
		strOutStatus =
			"Pattern Sound Remove requires the admitted typed source draft.";
		return false;
	}
	const auto Exact = std::find_if(
		m_ValtanPatternSoundCues.Cues.begin(),
		m_ValtanPatternSoundCues.Cues.end(),
		[&](const VALTAN_PATTERN_SOUND_CUE& Cue)
		{
			return Cue.strBindingId == RowId.strBindingId &&
				Cue.strOccurrenceId == RowId.strOccurrenceId &&
				Cue.strPatternId == Pattern.strPatternId &&
				Cue.strStageId == Stage.strStageId &&
				Cue.strActionId == Stage.strActionId;
		});
	if (m_ValtanPatternSoundCues.Cues.end() == Exact)
	{
		strOutStatus =
			"Pattern Sound Remove did not resolve the exact selected Pattern/Stage row.";
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged = m_ValtanPatternSoundCues;
	std::string RemoveStatus;
	if (!CValtanPatternSoundCueDocument::Remove_AuthoringRow(
			Staged, RowId, RemoveStatus))
	{
		strOutStatus =
			"Pattern Sound Remove rejected; admitted draft preserved: " +
			RemoveStatus;
		return false;
	}
	m_ValtanPatternSoundCues = std::move(Staged);
	m_bValtanPatternSoundCuesDirty = true;
	++m_iValtanPatternSoundDraftGeneration;
	m_strValtanPatternSoundCueStatus =
		"UNSAVED Pattern Sound row removed: " + RowId.strBindingId +
		" / " + RowId.strOccurrenceId + ". " + RemoveStatus;
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return true;
}

bool_t Client::CAnimation_Tool::
Stage_ValtanCompositionPatternSoundCascadeForAnimationDelete(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT& OutPreviousDraft,
	bool_t& bOutPreviousDirty,
	uint64_t& iOutMutationGeneration,
	std::size_t& iOutRemovedRowCount,
	std::string& strOutStatus)
{
	OutPreviousDraft = {};
	bOutPreviousDirty = false;
	iOutMutationGeneration = m_iValtanPatternSoundDraftGeneration;
	iOutRemovedRowCount = 0u;
	if (Is_ValtanCompositionPatternTransactionActive() ||
		!m_bValtanPatternSoundCuesReady ||
		Pattern.strPatternId.empty() || Stage.strStageId.empty() ||
		Stage.strActionId.empty() || strClipOccurrenceId.empty())
	{
		strOutStatus =
			"Animation Delete Sound cascade requires one admitted Pattern/Stage/occurrence and an idle Create transaction.";
		return false;
	}

	OutPreviousDraft = m_ValtanPatternSoundCues;
	bOutPreviousDirty = m_bValtanPatternSoundCuesDirty;
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged = m_ValtanPatternSoundCues;
	std::vector<VALTAN_PATTERN_SOUND_CUE_ROW_ID> Rows;
	for (const VALTAN_PATTERN_SOUND_CUE& Cue : Staged.Cues)
	{
		if (Cue.strPatternId != Pattern.strPatternId ||
			Cue.strStageId != Stage.strStageId ||
			Cue.strActionId != Stage.strActionId ||
			Cue.strClipOccurrenceId != strClipOccurrenceId)
		{
			continue;
		}
		Rows.push_back({ Cue.strBindingId, Cue.strOccurrenceId });
	}
	if (Rows.empty())
	{
		strOutStatus =
			"The selected Animation occurrence has no exact Pattern Sound dependency.";
		return true;
	}

	for (const VALTAN_PATTERN_SOUND_CUE_ROW_ID& Row : Rows)
	{
		std::string RemoveStatus;
		if (!CValtanPatternSoundCueDocument::Remove_AuthoringRow(
				Staged, Row, RemoveStatus))
		{
			strOutStatus =
				"Animation Delete Sound cascade was rejected before either owner changed: " +
				RemoveStatus;
			return false;
		}
	}

	m_ValtanPatternSoundCues = std::move(Staged);
	m_bValtanPatternSoundCuesDirty = true;
	++m_iValtanPatternSoundDraftGeneration;
	iOutMutationGeneration = m_iValtanPatternSoundDraftGeneration;
	iOutRemovedRowCount = Rows.size();
	m_strValtanPatternSoundCueStatus =
		"UNSAVED cascade: removed " + std::to_string(Rows.size()) +
		" Pattern Sound row(s) linked to Animation occurrence " +
		strClipOccurrenceId + ".";
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return true;
}

bool_t Client::CAnimation_Tool::
Restore_ValtanCompositionPatternSoundCascade(
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& PreviousDraft,
	const bool_t bPreviousDirty,
	const uint64_t iExpectedMutationGeneration,
	std::string& strOutStatus)
{
	if (m_iValtanPatternSoundDraftGeneration !=
		iExpectedMutationGeneration)
	{
		strOutStatus =
			"Pattern Sound cascade rollback rejected a stale draft generation; reload before editing again.";
		return false;
	}
	m_ValtanPatternSoundCues = PreviousDraft;
	m_bValtanPatternSoundCuesDirty = bPreviousDirty;
	++m_iValtanPatternSoundDraftGeneration;
	m_strValtanPatternSoundCueStatus =
		"Pattern Sound cascade rolled back with the rejected Animation Delete.";
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Prepare_ValtanCompositionPatternSoundSave(
	std::string& strOutBaselineBytes,
	std::string& strOutCandidateBytes,
	uint64_t& iOutDraftGeneration,
	bool_t& bOutDirty,
	std::string& strOutStatus) const
{
	strOutBaselineBytes.clear();
	strOutCandidateBytes.clear();
	iOutDraftGeneration = m_iValtanPatternSoundDraftGeneration;
	bOutDirty = m_bValtanPatternSoundCuesDirty;
	if (!m_bValtanPatternSoundCuesReady)
	{
		strOutStatus = "Pattern Sound data is not loaded.";
		return false;
	}
	if (!m_bValtanPatternSoundCuesDirty)
	{
		strOutStatus = "Pattern Sound owner has no staged Composition changes.";
		return true;
	}
	if (m_strValtanPatternSoundCueBaselineSourceBytes.empty())
	{
		strOutStatus = "Pattern Sound draft has no exact source baseline.";
		return false;
	}
	std::string DiskBytes;
	std::string ReadStatus;
	if (!Read_BoundedFile(
			CValtanPatternSoundCueDocument::Resolve_Path(), 512u * 1024u,
			DiskBytes, ReadStatus) ||
		DiskBytes != m_strValtanPatternSoundCueBaselineSourceBytes)
	{
		strOutStatus =
			"Pattern Sound source changed after this Composition draft began; Load before saving. " +
			ReadStatus;
		return false;
	}
	if (!CValtanPatternSoundCueDocument::Serialize_TransactionCandidate(
			m_ValtanPatternSoundCues, strOutCandidateBytes, strOutStatus))
	{
		return false;
	}
	strOutBaselineBytes = m_strValtanPatternSoundCueBaselineSourceBytes;
	strOutStatus =
		"Prepared Pattern Sound for the rollback-safe Composition Save transaction.";
	return true;
}

bool_t Client::CAnimation_Tool::Accept_ValtanCompositionPatternSoundSave(
	const uint64_t iExpectedDraftGeneration,
	const std::string& strExpectedCandidateBytes,
	std::string& strOutStatus)
{
	if (!m_bValtanPatternSoundCuesDirty ||
		m_iValtanPatternSoundDraftGeneration != iExpectedDraftGeneration)
	{
		strOutStatus =
			"Pattern Sound draft changed while the Composition Save transaction was running.";
		return false;
	}
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Reopened;
	std::string ReopenedBytes;
	if (!CValtanPatternSoundCueDocument::Load_ForAuthoring(
			Reopened, ReopenedBytes, strOutStatus) ||
		ReopenedBytes != strExpectedCandidateBytes ||
		Reopened != m_ValtanPatternSoundCues)
	{
		strOutStatus =
			"The Composition transaction completed, but Pattern Sound did not reopen as the exact committed draft: " +
			strOutStatus;
		return false;
	}
	m_ValtanPatternSoundCues = std::move(Reopened);
	m_strValtanPatternSoundCueBaselineSourceBytes = std::move(ReopenedBytes);
	m_bValtanPatternSoundCuesDirty = false;
	++m_iValtanPatternSoundDraftGeneration;
	m_bValtanPatternSoundRuntimeApplyReady = false;
	m_ValtanPatternSoundRuntimeAppliedRevision = {};
	m_strValtanPatternSoundCueStatus =
		"Pattern Sound committed and reopened by the Composition Save transaction.";
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Save_ValtanCompositionPatternSounds(
	std::string& strOutStatus)
{
	if (!Can_CommitValtanCompositionPatternSoundGeneration(strOutStatus))
		return false;
	std::string AuthoringRevision;
	std::string AuthoringStatus;
	bool_t bCanonicalDraftDirty = false;
	if (Is_ValtanCompositionPatternTransactionActive() ||
		nullptr == m_pBalanceTool ||
		!m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bCanonicalDraftDirty, AuthoringStatus) ||
		bCanonicalDraftDirty)
	{
		strOutStatus = Is_ValtanCompositionPatternTransactionActive() ?
			"Finish Create New Pattern before saving Sound changes." :
			"Save could not prepare the Pattern/Animation data: " +
				AuthoringStatus;
		return false;
	}
	if (!m_bValtanPatternSoundCuesReady)
	{
		strOutStatus =
			"Pattern Sound data is not loaded. Use Load and try again.";
		return false;
	}
	if (!m_bValtanPatternSoundCuesDirty)
	{
		strOutStatus = "Pattern Sound source has no staged changes.";
		return true;
	}
	const std::unordered_map<std::string, f32_t> Durations =
		CollectModelClipSourceDurationSeconds(Resolve_Model());
	std::string SaveStatus;
	if (!CValtanPatternSoundCueDocument::Save_Atomic(
			m_ValtanPatternSoundCues, Durations,
			m_strValtanPatternSoundCueBaselineSourceBytes, SaveStatus))
	{
		m_strValtanPatternSoundCueStatus =
			"Pattern Sound Save rejected; source and admitted draft were preserved: " +
			SaveStatus;
		strOutStatus = m_strValtanPatternSoundCueStatus;
		return false;
	}

	m_bValtanPatternSoundCuesDirty = false;
	const bool_t bDraftReloaded = Reload_ValtanPatternSoundCues();
	const std::string DraftStatus = m_strValtanPatternSoundCueStatus;
	if (!bDraftReloaded)
	{
		m_bValtanPatternSoundRuntimeApplyReady = false;
		m_ValtanPatternSoundRuntimeAppliedRevision = {};
	}
	m_strValtanPatternSoundCueStatus =
		"Pattern Sound saved and loaded. " + SaveStatus + " " +
		(bDraftReloaded ? DraftStatus :
			"The file was saved, but Workbench Load failed: " + DraftStatus);
	strOutStatus = m_strValtanPatternSoundCueStatus;
	return bDraftReloaded;
}
