#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "GameInstance.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
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




bool_t Client::CAnimation_Tool::Is_ValtanDocumentDirty() const
{
	return m_bValtanPatternSoundCuesDirty ||
		m_bValtanCombatObjectSoundCuesDirty ||
		m_bValtanPatternAnimationBindingDirty;
}

bool_t Client::CAnimation_Tool::Open_ValtanWorkspace()
{
	if (Is_AnyDocumentDirty() && !m_AssetName.empty() &&
		"Valtan" != m_AssetName)
	{
		m_Status =
			"Save or discard the current Animation document before opening the Valtan data workspace.";
		return false;
	}

	m_bValtanDataWorkspaceRequested = true;
	m_bValtanWorkspaceTabInitialized = false;
	m_Status =
		"Opened the canonical Valtan Pattern workspace. Product data remains visible even when the local Model View is unavailable.";
	return true;
}

const char_t* Client::CAnimation_Tool::ValtanPatternMasterPathName(
	const VALTAN_PATTERN_PREVIEW_PATH ePath)
{
	switch (ePath)
	{
	case VALTAN_PATTERN_PREVIEW_PATH::NORMAL:
		return "Normal";
	case VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY:
		return "Counter Hit -> Groggy";
	case VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY:
		return "Wall -> Groggy -> Recovery";
	case VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK:
		return "Wall -> Groggy -> Part Break";
	default:
		return "Invalid";
	}
}

const char_t* Client::CAnimation_Tool::ValtanPatternMasterAdmissionLabel() const
{
	switch (m_eValtanPatternMasterAdmission)
	{
	case VALTAN_VIEW_ADMISSION::UNLOADED:
		return "NOT LOADED";
	case VALTAN_VIEW_ADMISSION::ADMITTED:
		return "READY";
	case VALTAN_VIEW_ADMISSION::STALE_PRESERVED:
		return "PREVIOUS DATA / LOAD FAILED";
	case VALTAN_VIEW_ADMISSION::REJECTED:
		return "LOAD FAILED";
	default:
		return "INVALID";
	}
}

std::vector<const Client::VALTAN_PATTERN_VIEW*>
Client::CAnimation_Tool::Collect_ValtanPatternMasterPatterns() const
{
	std::vector<const VALTAN_PATTERN_VIEW*> Patterns;
	Patterns.reserve(m_ValtanPatternMasterView.Get_PatternCount());
	std::unordered_set<std::string> Collected;
	const auto AppendEditablePatterns = [&Patterns, &Collected](
		const std::vector<VALTAN_PATTERN_VIEW>& Source)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Source)
		{
			if (Pattern.bAuthoringMasterManaged &&
				!Pattern.strPatternId.empty() && !Pattern.Stages.empty() &&
				Collected.insert(Pattern.strPatternId).second)
			{
				Patterns.push_back(&Pattern);
			}
		}
	};
	AppendEditablePatterns(m_ValtanPatternMasterView.Rotation);
	AppendEditablePatterns(m_ValtanPatternMasterView.Gimmicks);
	return Patterns;
}

bool_t Client::CAnimation_Tool::Reload_ValtanPatternMaster()
{
	const auto RejectReload = [this](std::string Diagnostic)
	{
		const bool_t bHasPreservedAdmission =
			!Collect_ValtanPatternMasterPatterns().empty();
		m_eValtanPatternMasterAdmission = bHasPreservedAdmission ?
			VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
			VALTAN_VIEW_ADMISSION::REJECTED;
		m_strValtanPatternMasterStatus = bHasPreservedAdmission ?
			"Pattern data could not be loaded; the previous read-only view and pose were preserved: " + Diagnostic :
			"Pattern data could not be loaded: " + Diagnostic;
		return false;
	};
	VALTAN_PATTERN_TREE_VIEW Staged;
	std::string Status;
	if (!CValtanPatternTree::Load(Staged, Status))
		return RejectReload(Status);

	size_t iEditablePatternCount = 0u;
	const auto CountEditable = [&iEditablePatternCount](
		const std::vector<VALTAN_PATTERN_VIEW>& Patterns)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Patterns)
		{
			if (Pattern.bAuthoringMasterManaged &&
				!Pattern.strPatternId.empty() && !Pattern.Stages.empty())
			{
				++iEditablePatternCount;
			}
		}
	};
	CountEditable(Staged.Rotation);
	CountEditable(Staged.Gimmicks);
	if (0u == iEditablePatternCount)
	{
		return RejectReload(
			"the loaded files contain no editable Pattern with a stable ID and Stage.");
	}

	m_ValtanPatternMasterView = std::move(Staged);
	m_bValtanCompositionDraftPreviewReady = false;
	m_ValtanCompositionDraftPreview = {};
	m_eValtanPatternMasterAdmission =
		VALTAN_VIEW_ADMISSION::ADMITTED;
	/* Sound is a fail-open presentation lane.  Its loader still stages the
	   complete document and only commits on a valid gameplay/animation join;
	   a rejected refresh preserves the last admitted lane while animation and
	   Server gameplay remain usable. */
	if (!m_bValtanPatternSoundCuesDirty)
		(void)Reload_ValtanPatternSoundCues();
	(void)Reload_ValtanPatternShakeCues();
	(void)Reload_ValtanCombatObjectSoundCues();
	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
		Collect_ValtanPatternMasterPatterns();
	m_iValtanPatternMasterSelected = std::clamp(
		m_iValtanPatternMasterSelected, 0,
		static_cast<int32_t>(Patterns.size() - 1u));
	m_strValtanPatternMasterStatus =
		"Loaded " + std::to_string(Patterns.size()) +
		" editable Valtan Patterns from "
		"Data/Valtan/Valtan.gameplay.json + Valtan.presentation.json. " +
		Status;
	return true;
}

bool_t Client::CAnimation_Tool::Reload_ValtanPatternSoundCues()
{
	std::string LifecycleStatus;
	if (!Can_CommitValtanCompositionPatternSoundGeneration(LifecycleStatus))
	{
		m_strValtanPatternSoundCueStatus = std::move(LifecycleStatus);
		return false;
	}
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;
	std::string StagedBaselineSourceBytes;
	std::string Status;
	if (!CValtanPatternSoundCueDocument::Load_ForAuthoring(
		Staged, StagedBaselineSourceBytes, Status))
	{
		m_strValtanPatternSoundCueStatus =
			"Strict Pattern Sound authoring reload rejected; the previous admitted draft was preserved: " +
			Status;
		return false;
	}
	m_ValtanPatternSoundCues = std::move(Staged);
	++m_iValtanPatternSoundDraftGeneration;
	m_strValtanPatternSoundCueBaselineSourceBytes =
		std::move(StagedBaselineSourceBytes);
	m_bValtanPatternSoundCuesReady = true;
	m_bValtanPatternSoundCuesDirty = false;
	/* Source admission is data-only.  The Workbench may apply it to active
	   Arena/preview consumers only after the exact immutable Pattern revision is
	   confirmed Server-active. */
	m_bValtanPatternSoundRuntimeApplyReady = false;
	m_ValtanPatternSoundRuntimeAppliedRevision = {};
	m_strValtanPatternSoundCueStatus =
		"Strict authoring admission (active consumer apply pending): " + Status;
	return true;
}

bool_t Client::CAnimation_Tool::Reload_ValtanPatternShakeCues()
{
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT Staged;
	std::string Status;
	if (!CValtanPatternShakeCueDocument::Load_Source(Staged, Status))
	{
		m_strValtanPatternShakeCueStatus =
			"Camera/Shake lane reload rejected; the previous admitted lane was preserved: " +
			Status;
		return false;
	}
	m_ValtanPatternShakeCues = std::move(Staged);
	m_bValtanPatternShakeCuesReady = true;
	m_strValtanPatternShakeCueStatus =
		"Admitted " + std::to_string(m_ValtanPatternShakeCues.Cues.size()) +
		" Valtan camera-shake cue occurrences.";
	return true;
}

bool_t Client::CAnimation_Tool::Reload_ValtanCombatObjectSoundCues()
{
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT Staged;
	std::string Status;
	if (!CValtanCombatObjectSoundCueDocument::Load_Source(Staged, Status))
	{
		m_strValtanCombatObjectSoundCueStatus =
			"Server-hit Sound lane reload rejected; the previous admitted lane was preserved: " +
			Status;
		return false;
	}
	m_ValtanCombatObjectSoundCues = std::move(Staged);
	m_bValtanCombatObjectSoundCuesReady = true;
	m_strValtanCombatObjectSoundCueStatus =
		"Admitted " +
		std::to_string(m_ValtanCombatObjectSoundCues.Cues.size()) +
		" Server-hit-qualified Valtan Sound cue(s).";
	return true;
}

bool_t Client::CAnimation_Tool::Preview_ValtanSoundAsset(
	const std::string& strResourceAssetId)
{
	const std::filesystem::path SoundPath =
		CRuntimeAssetRoot::Resolve(strResourceAssetId);
	std::error_code Error;
	if (SoundPath.empty() ||
		!std::filesystem::is_regular_file(SoundPath, Error) || Error)
	{
		m_strValtanPatternSoundCueStatus =
			"Sound asset preview rejected: Resources-relative asset is missing or invalid: " +
			strResourceAssetId + ".";
		return false;
	}
	if (FAILED(CGameInstance::Get().Play_Sound(SoundPath.wstring(), 1.f)))
	{
		m_strValtanPatternSoundCueStatus =
			"Sound asset preview failed: " + strResourceAssetId + ".";
		return false;
	}
	m_strValtanPatternSoundCueStatus =
		"Previewing sound asset: " + strResourceAssetId + ".";
	return true;
}

bool_t Client::CAnimation_Tool::Build_ValtanPatternMasterTimeline(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	const shared_ptr<Engine::CModel>& pModel,
	std::vector<VALTAN_PATTERN_MASTER_PLAY_ITEM>& OutPlaylist,
	uint32_t& iOutDurationMs,
	std::string& strOutStatus) const
{
	OutPlaylist.clear();
	iOutDurationMs = 0u;
	strOutStatus.clear();
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		strOutStatus = "Animation target changed before the master timeline was staged.";
		return false;
	}
	if (Pattern.Stages.empty())
	{
		strOutStatus = "Selected pattern has no admitted stages in the split "
			"Valtan.gameplay.json + Valtan.presentation.json source.";
		return false;
	}

	std::vector<const VALTAN_STAGE_VIEW*> StagePath;
	if (!CValtanPatternTree::Build_PreviewStagePath(
		Pattern, ePath, StagePath, strOutStatus))
	{
		return false;
	}

	uint64_t iTimelineMs = 0u;
	for (const VALTAN_STAGE_VIEW* pStage : StagePath)
	{
		if (nullptr == pStage || 0u == pStage->iDurationMs ||
			pStage->strActionId.empty())
		{
			strOutStatus =
				"Master branch graph contains an incomplete animation stage.";
			return false;
		}
		const VALTAN_STAGE_VIEW& Stage = *pStage;
		LostArk::Shared::WORLD_ENTITY_ACTION eStageAction =
			LostArk::Shared::WORLD_ENTITY_ACTION::END;
		if (!Try_ResolveValtanArenaPatternAction(
				Stage.strStageKind, eStageAction) ||
			LostArk::Shared::WORLD_ENTITY_ACTION::END == eStageAction)
		{
			strOutStatus = "Master stage kind has no Arena snapshot action: " +
				Stage.strStageKind + ".";
			return false;
		}
		const uint64_t iStageTimelineStartMs = iTimelineMs;
		if (Stage.bSuppressAnimation)
		{
			if (!Stage.ClipOccurrences.empty())
			{
				strOutStatus = "NONE animation stage unexpectedly owns clips: " +
					Stage.strActionId + ".";
				return false;
			}
			VALTAN_PATTERN_MASTER_PLAY_ITEM Item;
			Item.strPatternId = Pattern.strPatternId;
			Item.strPatternDisplayName = Pattern.strDisplayName;
			Item.strStageId = Stage.strStageId;
			Item.strSequenceRole = Stage.strSequenceRole;
			Item.strStageKind = Stage.strStageKind;
			Item.strActionId = Stage.strActionId;
			Item.iAuthoringWallMs = Stage.iDurationMs;
			Item.iTimelineStartMs = static_cast<uint32_t>(iTimelineMs);
			Item.iStageTimelineStartMs =
				static_cast<uint32_t>(iStageTimelineStartMs);
			Item.iOccurrenceNumber = 1u;
			Item.iOccurrenceCount = 1u;
			Item.bSuppressAnimation = true;
			OutPlaylist.push_back(std::move(Item));
			iTimelineMs += Stage.iDurationMs;
			if (iTimelineMs > static_cast<uint64_t>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				strOutStatus = "Master authoring timeline duration overflowed.";
				return false;
			}
			continue;
		}
		if (Stage.ClipOccurrences.empty())
		{
			strOutStatus = "Master branch graph contains an unbound animation stage: " +
				Stage.strActionId + ".";
			return false;
		}
		const size_t iPlayableOccurrenceCount =
			Stage.iAuthoringRepeatCount > 1u ?
				static_cast<size_t>(Stage.iAuthoringRepeatCount) :
				Stage.ClipOccurrences.size();
		if (0u == iPlayableOccurrenceCount ||
			iPlayableOccurrenceCount != Stage.ClipOccurrences.size())
		{
			strOutStatus = "Master repeatCount does not own exactly its explicit occurrences for " +
				Stage.strActionId + ".";
			return false;
		}

		uint64_t iStageAnimationWallMs = 0u;
		for (size_t iClip = 0u; iClip < iPlayableOccurrenceCount; ++iClip)
		{
			const VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
				Stage.ClipOccurrences[iClip];
			if (Clip.strClipOccurrenceId.empty() || Clip.strClipName.empty() ||
				0u == Clip.iAuthoringWallMs ||
				!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f)
			{
				strOutStatus = "Master occurrence is incomplete: " +
					Clip.strClipOccurrenceId + ".";
				return false;
			}

			uint32_t iAnimationIndex = (std::numeric_limits<uint32_t>::max)();
			for (uint32_t iAnimation = 0u;
				iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
			{
				const char_t* pName = pModel->Get_AnimationName(iAnimation);
				if (nullptr != pName && Clip.strClipName == pName)
				{
					iAnimationIndex = iAnimation;
					break;
				}
			}
			if ((std::numeric_limits<uint32_t>::max)() == iAnimationIndex)
			{
				strOutStatus = "Scene Valtan model is missing master clip " +
					Clip.strClipName + ".";
				return false;
			}

			f32_t fTrackPosition = 0.f;
			f32_t fTrackDuration = 0.f;
			const f32_t fTickRate =
				pModel->Get_AnimationTickPerSecond(iAnimationIndex);
			if (!std::isfinite(fTickRate) || fTickRate <= 0.f ||
				!pModel->Get_AnimationProgress(
					iAnimationIndex, fTrackPosition, fTrackDuration) ||
				!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f)
			{
				strOutStatus = "Master clip has no valid model clock: " +
					Clip.strClipName + ".";
				return false;
			}
			const ACTION_PRESENTATION_CLIP_TIMING Timing{
				fTrackDuration / fTickRate,
				Clip.iPlayMs,
				Clip.fPlayRate,
				Clip.bLoop,
				static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f };
			f32_t fSourceDuration = 0.f;
			f32_t fSourceWallDuration = 0.f;
			if (!CActionPresentationTimeline::Resolve_ClipDuration(
				Timing, fSourceDuration, fSourceWallDuration))
			{
				strOutStatus = "Master source window is outside the model clip: " +
					Clip.strClipOccurrenceId + ".";
				return false;
			}

			VALTAN_PATTERN_MASTER_PLAY_ITEM Item;
			Item.strPatternId = Pattern.strPatternId;
			Item.strPatternDisplayName = Pattern.strDisplayName;
			Item.strStageId = Stage.strStageId;
			Item.strSequenceRole = Stage.strSequenceRole;
			Item.strStageKind = Stage.strStageKind;
			Item.strActionId = Stage.strActionId;
			Item.strClipOccurrenceId = Clip.strClipOccurrenceId;
			Item.strClipName = Clip.strClipName;
			Item.iSourceStartMs = Clip.iSourceStartMs;
			Item.iPlayMs = Clip.iPlayMs;
			Item.iAuthoringWallMs = Clip.iAuthoringWallMs;
			Item.iTimelineStartMs = static_cast<uint32_t>(iTimelineMs);
			Item.iStageTimelineStartMs =
				static_cast<uint32_t>(iStageTimelineStartMs);
			Item.iOccurrenceNumber = static_cast<uint32_t>(iClip + 1u);
			Item.iOccurrenceCount = static_cast<uint32_t>(
				iPlayableOccurrenceCount);
			Item.fPlayRate = Clip.fPlayRate;
			Item.bRepeatUntilStageEnd = Clip.bLoop;
			OutPlaylist.push_back(std::move(Item));

			iStageAnimationWallMs += Clip.iAuthoringWallMs;
			iTimelineMs += Clip.iAuthoringWallMs;
			if (iTimelineMs > static_cast<uint64_t>(
				(std::numeric_limits<uint32_t>::max)()))
			{
				strOutStatus = "Master authoring timeline duration overflowed.";
				return false;
			}
		}
		if (iStageAnimationWallMs > Stage.iDurationMs)
		{
			strOutStatus = "Master occurrences exceed Server stage " +
				Stage.strStageId + ".";
			return false;
		}
		if (iStageAnimationWallMs < Stage.iDurationMs)
		{
			if ("HOLD_LAST_POSE" != Stage.strAnimationEndPolicy)
			{
				strOutStatus =
					"Only HOLD_LAST_POSE may leave a trailing Server Stage gap: " +
					Stage.strStageId + ".";
				return false;
			}
			/* HOLD_LAST_POSE deliberately allows the finite source playlist to
			   finish before the Server Stage clock.  Keep one master item active
			   through that trailing gap so local pattern sampling, collider debug,
			   and the next Stage all remain on the exact Server clock. */
			const uint32_t iTrailingHoldMs = static_cast<uint32_t>(
				Stage.iDurationMs - iStageAnimationWallMs);
			VALTAN_PATTERN_MASTER_PLAY_ITEM& LastStageItem = OutPlaylist.back();
			if (LastStageItem.strStageId != Stage.strStageId ||
				LastStageItem.iAuthoringWallMs >
					(std::numeric_limits<uint32_t>::max)() - iTrailingHoldMs)
			{
				strOutStatus =
					"Master trailing pose hold lost its exact Stage owner clock.";
				return false;
			}
			LastStageItem.iAuthoringWallMs += iTrailingHoldMs;
			iStageAnimationWallMs = Stage.iDurationMs;
			iTimelineMs += iTrailingHoldMs;
			if (iTimelineMs > static_cast<uint64_t>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				strOutStatus = "Master authoring timeline duration overflowed.";
				return false;
			}
		}
	}
	if (OutPlaylist.empty() || 0u == iTimelineMs)
	{
		strOutStatus = "Master authoring timeline is empty.";
		return false;
	}
	iOutDurationMs = static_cast<uint32_t>(iTimelineMs);
	strOutStatus = "Admitted " + Pattern.strPatternId + " / " +
		ValtanPatternMasterPathName(ePath) + " / " +
		std::to_string(OutPlaylist.size()) + " presentation items / " +
		std::to_string(iOutDurationMs) + " ms.";
	return true;
}

bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath)
{
	if (!Can_MutateValtanView(m_eValtanPatternMasterAdmission))
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Offline play rejected: the preserved graph is display-only until a fresh canonical reload is ADMITTED.";
		return false;
	}
	std::vector<VALTAN_PATTERN_MASTER_PLAY_ITEM> StagedPlaylist;
	uint32_t iStagedDurationMs = 0u;
	std::string Status;
	if (!Build_ValtanPatternMasterTimeline(
		Pattern, ePath, pModel, StagedPlaylist, iStagedDurationMs, Status))
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Master play rejected; current model pose preserved: " +
			Status;
		return false;
	}

	const shared_ptr<CValtan> PreviewBoss =
		CAnimationTargetService::Resolve_Boss();
	if (nullptr == PreviewBoss || PreviewBoss->Get_BodyModel() != pModel)
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Master play rejected; the staged model is not the current local Valtan boss.";
		return false;
	}
	if (!PreviewBoss->Stage_LocalPatternAuthoringPreview(Pattern, Status))
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Master play rejected; effective draft preview staging failed: " +
			Status;
		return false;
	}

	const uint32_t iPreviousAnimation = pModel->Get_CurrentAnimIndex();
	f32_t fPreviousPosition = 0.f;
	f32_t fPreviousDuration = 0.f;
	const bool_t bPreviousTrack = pModel->Get_AnimationProgress(
		iPreviousAnimation, fPreviousPosition, fPreviousDuration);
	const bool_t bPreviousPaused = pModel->Is_AnimPaused();
	const bool_t bPreviousLoop = pModel->Is_AnimLoop();
	const f32_t fPreviousSpeed = m_bValtanPatternPreviewPlaying ?
		m_fValtanPatternPreviewSpeed : 1.f;
	if (m_bValtanPatternPreviewPlaying)
	{
		Reset_ValtanPatternPreviewState(
			"Source reference preview yielded to Valtan Pattern Master.");
	}

	m_ValtanPatternMasterPlaylist = std::move(StagedPlaylist);
	m_iValtanPatternMasterItem = 0u;
	m_fValtanPatternMasterItemElapsedSeconds = 0.f;
	m_iValtanPatternMasterDurationMs = iStagedDurationMs;
	m_eValtanPatternMasterPath = ePath;
	m_bValtanPatternMasterPlaying = true;
	m_bValtanPatternMasterPaused = false;
	m_ValtanPatternMasterModel = pModel;
	m_ValtanPatternMasterBoss = PreviewBoss;
	m_iValtanPatternMasterTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_strValtanPatternMasterStatus = Status;
	if (Activate_ValtanPatternMasterItem(pModel, 0u, 0.f))
	{
		Update_ValtanPatternMasterHitAreaPreview();
		return true;
	}

	Reset_ValtanPatternMasterPreviewState(
		"Valtan Pattern Master play failed; previous model pose restored.");
	if (bPreviousTrack &&
		pModel->Start_Animation(iPreviousAnimation, bPreviousLoop))
	{
		pModel->Set_AnimationSpeed(fPreviousSpeed);
		pModel->Set_AnimTrackPosition(iPreviousAnimation, fPreviousPosition);
		pModel->Set_AnimPaused(bPreviousPaused);
		pModel->Play_Animation(0.f);
	}
	return false;
}

bool_t Client::CAnimation_Tool::Apply_ValtanPatternMasterPose(
	const shared_ptr<Engine::CModel>& pModel,
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item,
	const f32_t fLocalWallSeconds,
	const bool_t bForceAnimationEdge) const
{
	if (nullptr == pModel || !std::isfinite(fLocalWallSeconds) ||
		fLocalWallSeconds < 0.f)
	{
		return false;
	}
	const shared_ptr<CValtan> Boss = m_ValtanPatternMasterBoss.lock();
	if (nullptr == Boss || Boss->Get_BodyModel() != pModel)
		return false;
	LostArk::Shared::WORLD_ENTITY_ACTION ePatternAction =
		LostArk::Shared::WORLD_ENTITY_ACTION::END;
	if (!Try_ResolveValtanArenaPatternAction(
			Item.strStageKind, ePatternAction))
	{
		return false;
	}
	const f32_t fStageWallSeconds =
		(static_cast<f32_t>(Item.iTimelineStartMs -
			Item.iStageTimelineStartMs) * 0.001f) +
		fLocalWallSeconds;
	if (!Boss->Apply_LocalPatternPresentationSample(
			ePatternAction,
			Item.strActionId,
			fStageWallSeconds,
			bForceAnimationEdge))
	{
		return false;
	}
	/* Animation Tool owns the wall clock and samples the Product presentation
	   pose explicitly. Pausing the model prevents a second local frame clock
	   from drifting between those samples. */
	pModel->Set_AnimPaused(true);
	return true;
}

bool_t Client::CAnimation_Tool::Activate_ValtanPatternMasterItem(
	const shared_ptr<Engine::CModel>& pModel,
	const std::size_t iItem,
	const f32_t fLocalWallSeconds)
{
	if (nullptr == pModel ||
		iItem >= m_ValtanPatternMasterPlaylist.size())
	{
		return false;
	}
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
		m_ValtanPatternMasterPlaylist[iItem];
	const f32_t fDurationSeconds =
		static_cast<f32_t>(Item.iAuthoringWallMs) * 0.001f;
	/* The master timeline drives the pose itself: it holds the model paused and
	writes the authored source time every frame.  A blend cannot finish on that
	path -- CModel::Play_Animation feeds Update_AnimBlend a zero delta while the
	model is paused, so m_fBlendElapsed never advances and every bone is pulled
	back to the captured blend-from pose at ratio zero.  Start the occurrence
	without a blend; the very next Apply_ValtanPatternMasterPose owns the pose. */
	if (!std::isfinite(fLocalWallSeconds) || fLocalWallSeconds < 0.f ||
		fLocalWallSeconds > fDurationSeconds + 0.000001f)
	{
		return false;
	}
	m_iValtanPatternMasterItem = iItem;
	m_fValtanPatternMasterItemElapsedSeconds =
		std::clamp(fLocalWallSeconds, 0.f, fDurationSeconds);
	if (!Apply_ValtanPatternMasterPose(
		pModel, Item, m_fValtanPatternMasterItemElapsedSeconds, true))
	{
		return false;
	}
	m_strValtanPatternMasterStatus = Item.bSuppressAnimation ?
		"Playing admitted NONE stage (boss pose hold) | " +
			Item.strStageId + " / " + Item.strSequenceRole + "." :
		"Playing admitted occurrence " + Item.strClipOccurrenceId + " | " +
			Item.strStageId + " / " + Item.strSequenceRole + ".";
	return true;
}

bool_t Client::CAnimation_Tool::Seek_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const f32_t fTimelineSeconds,
	const bool_t bPause,
	const bool_t bResetPresentationTransport)
{
	if (!m_bValtanPatternMasterPlaying || nullptr == pModel ||
		m_ValtanPatternMasterPlaylist.empty() ||
		!std::isfinite(fTimelineSeconds) || fTimelineSeconds < 0.f)
	{
		return false;
	}
	const f32_t fTimelineMs = std::clamp(
		fTimelineSeconds * 1000.f, 0.f,
		static_cast<f32_t>(m_iValtanPatternMasterDurationMs));
	for (size_t iItem = 0u;
		iItem < m_ValtanPatternMasterPlaylist.size(); ++iItem)
	{
		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
			m_ValtanPatternMasterPlaylist[iItem];
		const f32_t fEndMs = static_cast<f32_t>(
			Item.iTimelineStartMs + Item.iAuthoringWallMs);
		const bool_t bLast =
			iItem + 1u == m_ValtanPatternMasterPlaylist.size();
		if (fTimelineMs < fEndMs || bLast)
		{
			f32_t fLocalSeconds = std::clamp(
				(fTimelineMs - static_cast<f32_t>(Item.iTimelineStartMs)) *
					0.001f,
				0.f,
				static_cast<f32_t>(Item.iAuthoringWallMs) * 0.001f);
			/* Sampling an exact looping endpoint wraps to source time zero.
			   The timeline endpoint means the final visible pose, so only that
			   endpoint is moved to the previous representable wall time. */
			if (bLast && fTimelineMs >= fEndMs && fLocalSeconds > 0.f)
				fLocalSeconds = std::nextafter(fLocalSeconds, 0.f);
			if (bResetPresentationTransport)
			{
				const shared_ptr<CValtan> PreviewBoss =
					m_ValtanPatternMasterBoss.lock();
				if (nullptr == PreviewBoss || PreviewBoss->Get_BodyModel() != pModel)
					return false;
				PreviewBoss->Reset_LocalPatternPreviewTransport();
			}
			if (!Activate_ValtanPatternMasterItem(
				pModel, iItem, fLocalSeconds))
			{
				return false;
			}
			m_bValtanPatternMasterPaused = bPause;
			Update_ValtanPatternMasterHitAreaPreview();
			return true;
		}
	}
	return false;
}

void Client::CAnimation_Tool::Advance_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (!m_bValtanPatternMasterPlaying ||
		m_iValtanPatternMasterItem >= m_ValtanPatternMasterPlaylist.size())
	{
		return;
	}
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
		m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
	const f32_t fAbsoluteSeconds =
		static_cast<f32_t>(Item.iTimelineStartMs) * 0.001f +
		m_fValtanPatternMasterItemElapsedSeconds;
	if (fAbsoluteSeconds * 1000.f + 0.001f >=
		static_cast<f32_t>(m_iValtanPatternMasterDurationMs))
	{
		if (m_bValtanCompositionLoop &&
			Seek_ValtanPatternMasterPreview(pModel, 0.f, false, true))
		{
			m_strValtanPatternMasterStatus =
				"Valtan Pattern preview loop restarted from the admitted first occurrence.";
			return;
		}
		Stop_ValtanPatternMasterPreview(
			pModel,
			"Valtan Pattern Master timeline completed; idle restored.");
		return;
	}
	if (!Seek_ValtanPatternMasterPreview(
			pModel, fAbsoluteSeconds, false, false))
	{
		Stop_ValtanPatternMasterPreview(
			pModel,
			"Valtan Pattern Master timeline failed to advance; idle restored.");
	}
}

void Client::CAnimation_Tool::Stop_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const std::string& status)
{
	if (nullptr != pModel)
	{
		m_bLoop = true;
		pModel->Set_AnimationSpeed(1.f);
		if (!pModel->Start_Animation("mesh_idle_battle_1", true))
			pModel->Set_AnimPaused(true);
	}
	Reset_ValtanPatternMasterPreviewState(status);
}

void Client::CAnimation_Tool::Reset_ValtanPatternMasterPreviewState(
	const std::string& status)
{
	const shared_ptr<Engine::CModel> PreviewModel =
		m_ValtanPatternMasterModel.lock();
	const shared_ptr<CValtan> PreviewBoss =
		m_ValtanPatternMasterBoss.lock();
	m_bValtanPatternMasterPlaying = false;
	m_bValtanPatternMasterPaused = false;
	m_iValtanPatternMasterItem = 0u;
	m_fValtanPatternMasterItemElapsedSeconds = 0.f;
	m_iValtanPatternMasterDurationMs = 0u;
	m_ValtanPatternMasterPlaylist.clear();
	m_strValtanPatternMasterStatus = status;
	m_ValtanPatternMasterModel.reset();
	m_ValtanPatternMasterBoss.reset();
	m_iValtanPatternMasterTargetGeneration = 0u;
	if (nullptr != PreviewBoss)
	{
		PreviewBoss->Reset_LocalPatternPresentationSample();
	}
	else if (nullptr != PreviewModel)
	{
		PreviewModel->Set_AnimationSpeed(1.f);
		PreviewModel->Set_AnimPaused(false);
	}
#ifdef _DEBUG
	if (nullptr != PreviewBoss)
	{
		PreviewBoss->Clear_PatternHitAreaPreview();
	}
#endif
}

void Client::CAnimation_Tool::Update_ValtanPatternMasterHitAreaPreview()
{
#ifdef _DEBUG
	const shared_ptr<CValtan> Boss = CAnimationTargetService::Resolve_Boss();
	if (nullptr == Boss)
		return;
	if (!m_bValtanPatternMasterPlaying ||
		m_iValtanPatternMasterItem >= m_ValtanPatternMasterPlaylist.size())
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
		m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
	const f32_t fStageSeconds =
		(static_cast<f32_t>(Item.iTimelineStartMs -
			Item.iStageTimelineStartMs) * 0.001f) +
		m_fValtanPatternMasterItemElapsedSeconds;
	Boss->Set_PatternHitAreaPreview(Item.strActionId, fStageSeconds);
#endif
}

bool_t Client::CAnimation_Tool::Load_ValtanAnimationBindingDraft(
	const shared_ptr<Engine::CModel>& pModel)
{
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT Staged;
	std::string Status;
	const std::vector<std::string> AvailableClips = Collect_ClipNames(pModel);
	if (!CValtanPatternAnimationBindingDocument::Load(
		"Valtan", "BOSS_VALTAN", AvailableClips, Staged, Status))
	{
		m_strValtanPatternAnimationBindingStatus =
			"Animation Product reload rejected; the current read-only projection was preserved: " +
			Status;
		return false;
	}
	m_ValtanPatternAnimationBindingDraft = std::move(Staged);
	m_strValtanPatternAnimationBindingBaselineSourceBytes.clear();
	m_bValtanPatternAnimationBindingReady = true;
	m_bValtanPatternAnimationBindingDirty = false;
	m_strValtanPatternAnimationBindingStatus = std::move(Status);
	if (AvailableClips.end() == std::find(
		AvailableClips.begin(), AvailableClips.end(),
		m_strValtanAnimationBindingNewClip))
	{
		m_strValtanAnimationBindingNewClip = AvailableClips.empty() ?
			std::string{} : AvailableClips.front();
	}
	return true;
}

bool_t Client::CAnimation_Tool::Start_ValtanSequencePreview(
	const shared_ptr<Engine::CModel>& pModel,
	const std::size_t iSequenceIndex)
{
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		m_strValtanPatternPreviewStatus =
			"Sequence preview start rejected because the animation target changed.";
		return false;
	}
	if (iSequenceIndex >= m_ClipSeqs.size())
		return false;
	const CLIP_SEQ& Seq = m_ClipSeqs[iSequenceIndex];
	if (Seq.clips.empty())
	{
		m_strValtanPatternPreviewStatus =
			"Sequence preview start rejected because the sequence has no clips.";
		return false;
	}

	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> staged;
	staged.reserve(Seq.clips.size());
	const uint32_t iStepCount = static_cast<uint32_t>(Seq.clips.size());
	std::string label = Seq.name.empty() ?
		std::to_string(Seq.iSkillId) : Seq.name;
	label += " seq" + std::to_string(Seq.iSeqIndex) +
		" [" + Seq.sMode + "]";
	const bool_t bHasCuts = Seq.cuts.size() == Seq.clips.size();
	for (uint32_t iStep = 0u; iStep < iStepCount; ++iStep)
	{
		VALTAN_PATTERN_PREVIEW_PLAY_ITEM item;
		item.strPatternLabel = label;
		item.iSourceActionId = static_cast<uint32_t>(Seq.iSkillId);
		item.iSequenceIndex = Seq.iSeqIndex;
		item.iSequenceRepeatNumber = 1u;
		item.iSequenceRepeatCount = 1u;
		item.iSourceStepNumber = iStep + 1u;
		item.iSourceStepCount = iStepCount;
		item.strSequenceName = Seq.name;
		item.strSequenceMode = Seq.sMode;
		item.strClipName = Seq.clips[iStep];
		item.iStepNumber = iStep + 1u;
		item.iStepCount = iStepCount;
		if (bHasCuts && std::isfinite(Seq.cuts[iStep]))
		{
			/* The source stage hands over here; a near-zero cut is a stage
			the original skips through instantly. */
			if (Seq.cuts[iStep] < 0.02f)
				continue;
			item.fAuthoredDurationSeconds = Seq.cuts[iStep];
		}
		else
		{
			const auto length = m_ClipLength.find(item.strClipName);
			if (m_ClipLength.end() != length && length->second > 0.f &&
				std::isfinite(length->second))
			{
				item.fAuthoredDurationSeconds = length->second;
			}
		}
		staged.push_back(std::move(item));
	}
	if (staged.empty())
	{
		m_strValtanPatternPreviewStatus =
			"Sequence preview start rejected because every step is skipped.";
		return false;
	}
	if (m_bValtanPatternMasterPlaying)
	{
		Reset_ValtanPatternMasterPreviewState(
			"Valtan Pattern Master yielded to the read-only source reference.");
	}

	m_ValtanPatternPreviewPlaylist = std::move(staged);
	m_iValtanPatternPreviewItem = 0u;
	m_bValtanPatternPreviewPlaying = true;
	m_bValtanPatternPreviewPaused = false;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_ValtanPatternPreviewModel = pModel;
	m_iValtanPatternPreviewTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanSequenceSelected = static_cast<int32_t>(iSequenceIndex);
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	m_strValtanPatternPreviewStatus =
		"Playing source sequence " + label + ".";
	return Activate_ValtanPatternPreviewItem(pModel);
}

bool_t Client::CAnimation_Tool::Start_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const uint32_t iFirstPattern,
	const uint32_t iLastPattern)
{
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		m_strValtanPatternPreviewStatus =
			"Pattern preview start rejected because the animation target changed.";
		return false;
	}
	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> staged;
	std::string status;
	if (!CValtanPatternPreviewDocument::Build_Playlist(
			m_ValtanPatternPreviewDocument,
			iFirstPattern,
			iLastPattern,
			staged,
			status))
	{
		m_strValtanPatternPreviewStatus =
			"Pattern preview start rejected; current pose preserved: " + status;
		return false;
	}
	if (m_bValtanPatternMasterPlaying)
	{
		Reset_ValtanPatternMasterPreviewState(
			"Valtan Pattern Master yielded to the read-only source reference.");
	}

	m_ValtanPatternPreviewPlaylist = std::move(staged);
	m_iValtanPatternPreviewItem = 0u;
	m_bValtanPatternPreviewPlaying = true;
	m_bValtanPatternPreviewPaused = false;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_ValtanPatternPreviewModel = pModel;
	m_iValtanPatternPreviewTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanSequenceSelected = -1;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	m_strValtanPatternPreviewStatus = status;
	return Activate_ValtanPatternPreviewItem(pModel);
}

bool_t Client::CAnimation_Tool::Activate_ValtanPatternPreviewItem(
	const shared_ptr<Engine::CModel>& pModel)
{
	constexpr f32_t PATTERN_MARKER_DURATION_SECONDS = 0.45f;
	std::string skippedStatus;
	const auto RecordSkip = [&skippedStatus](const std::string& reason)
	{
		if (!skippedStatus.empty())
			skippedStatus += " ";
		skippedStatus += reason;
	};

	while (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size())
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		m_fValtanPatternPreviewElapsedSeconds = 0.f;
		m_fValtanPatternPreviewItemDurationSeconds = 0.f;
		m_bValtanPatternPreviewPaused = false;

		if (Item.bPatternMarker)
		{
			if (!pModel->Start_Animation("mesh_idle_battle_1", true))
			{
				RecordSkip(
					"Skipped pattern marker because the idle clip is unavailable.");
				++m_iValtanPatternPreviewItem;
				continue;
			}

			m_bLoop = true;
			m_fValtanPatternPreviewItemDurationSeconds =
				PATTERN_MARKER_DURATION_SECONDS;
			pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
			m_strValtanPatternPreviewStatus = skippedStatus;
			if (!m_strValtanPatternPreviewStatus.empty())
				m_strValtanPatternPreviewStatus += " ";
			m_strValtanPatternPreviewStatus +=
				"Pattern " + std::to_string(Item.iPatternNumber) +
				" step " + std::to_string(Item.iStepNumber) + "/" +
				std::to_string(Item.iStepCount) +
				" marker started for 450 ms.";
			return true;
		}

		if (!Start_PreviewClip(
			pModel, Item.strClipName.c_str(), false, m_fPreviewBlendSeconds))
		{
			RecordSkip(
				"Skipped unavailable clip without stopping Play All: " +
				Item.strClipName + ".");
			++m_iValtanPatternPreviewItem;
			continue;
		}

		const uint32_t AnimationIndex = pModel->Get_CurrentAnimIndex();
		f32_t Position = 0.f;
		f32_t DurationTicks = 0.f;
		const f32_t TickRate =
			pModel->Get_AnimationTickPerSecond(AnimationIndex);
		if (!std::isfinite(TickRate) || TickRate <= 0.f ||
			!pModel->Get_AnimationProgress(
				AnimationIndex, Position, DurationTicks) ||
			!std::isfinite(DurationTicks) || DurationTicks <= 0.f)
		{
			RecordSkip(
				"Skipped clip with invalid native duration without stopping Play All: " +
				Item.strClipName + ".");
			++m_iValtanPatternPreviewItem;
			continue;
		}

		const f32_t NativeDurationSeconds = DurationTicks / TickRate;
		if (!std::isfinite(NativeDurationSeconds) ||
			NativeDurationSeconds <= 0.f)
		{
			RecordSkip(
				"Skipped clip with invalid native duration without stopping Play All: " +
				Item.strClipName + ".");
			++m_iValtanPatternPreviewItem;
			continue;
		}

		f32_t fItemDurationSeconds = NativeDurationSeconds;
		bool_t bLoopClip = false;
		std::string durationNote = " started for its full native duration (";
		if (Item.fAuthoredDurationSeconds > 0.f)
		{
			fItemDurationSeconds = Item.fAuthoredDurationSeconds;
			bLoopClip =
				Item.fAuthoredDurationSeconds > NativeDurationSeconds + 0.001f;
			durationNote = bLoopClip ?
				" started looping for its source stage length (" :
				" started for its source stage length (";
			if (bLoopClip &&
				!Start_PreviewClip(
					pModel, Item.strClipName.c_str(), true,
					m_fPreviewBlendSeconds))
			{
				RecordSkip(
					"Skipped unavailable clip without stopping Play All: " +
					Item.strClipName + ".");
				++m_iValtanPatternPreviewItem;
				continue;
			}
		}

		m_bLoop = bLoopClip;
		m_fValtanPatternPreviewItemDurationSeconds = fItemDurationSeconds;
		pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
		m_strValtanPatternPreviewStatus = skippedStatus;
		if (!m_strValtanPatternPreviewStatus.empty())
			m_strValtanPatternPreviewStatus += " ";
		m_strValtanPatternPreviewStatus +=
			"Pattern " + std::to_string(Item.iPatternNumber) +
			" step " + std::to_string(Item.iStepNumber) + "/" +
			std::to_string(Item.iStepCount) +
			durationNote +
			std::to_string(fItemDurationSeconds) + " s).";
		return true;
	}

	if (!skippedStatus.empty())
		skippedStatus += " ";
	Stop_ValtanPatternPreview(
		pModel, skippedStatus + "Pattern preview completed; idle restored.");
	return false;
}

void Client::CAnimation_Tool::Advance_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (!m_bValtanPatternPreviewPlaying)
		return;
	const bool_t bHadItem =
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size();
	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM Finished = bHadItem ?
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem] :
		VALTAN_PATTERN_PREVIEW_PLAY_ITEM{};
	const f32_t fFinishedDurationSeconds =
		m_fValtanPatternPreviewItemDurationSeconds;
	++m_iValtanPatternPreviewItem;
	if (m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		Stop_ValtanPatternPreview(
			pModel, "Pattern preview completed; idle restored.");
		return;
	}
	Activate_ValtanPatternPreviewItem(pModel);
	/* The pattern hit clock keeps counting only while the next started item
	   continues the same source sequence pass; anything else restarts it. */
	if (!m_bValtanPatternPreviewPlaying ||
		m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		m_fValtanPatternHitTimelineBaseSeconds = 0.f;
		return;
	}
	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Current =
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
	const bool_t bSameTimeline = bHadItem &&
		!Finished.bPatternMarker && !Current.bPatternMarker &&
		Finished.iPatternNumber == Current.iPatternNumber &&
		Finished.iSourceActionId == Current.iSourceActionId &&
		Finished.iSequenceIndex == Current.iSequenceIndex &&
		Finished.iSequenceRepeatNumber == Current.iSequenceRepeatNumber;
	if (bSameTimeline)
		m_fValtanPatternHitTimelineBaseSeconds += fFinishedDurationSeconds;
	else
		m_fValtanPatternHitTimelineBaseSeconds = 0.f;
}

void Client::CAnimation_Tool::Stop_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const std::string& status)
{
	if (nullptr != pModel)
	{
		m_bLoop = true;
		pModel->Set_AnimationSpeed(1.f);
		/* Each authoring body names its own idle; an unknown target keeps the
		   product clip so the existing Valtan paths are unchanged. */
		const CUSTOM_CHAIN_PROFILE* pProfile =
			Find_CustomChainProfile(m_AssetName);
		const char_t* pIdleClip = nullptr != pProfile ?
			pProfile->pIdleClip : "mesh_idle_battle_1";
		if (!pModel->Start_Animation(pIdleClip, true))
			pModel->Set_AnimPaused(true);
	}
	Reset_ValtanPatternPreviewState(status);
}

void Client::CAnimation_Tool::Reset_ValtanPatternPreviewState(
	const std::string& status)
{
	m_bValtanPatternPreviewPlaying = false;
	m_bValtanPatternPreviewPaused = false;
	m_iValtanPatternPreviewItem = 0u;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_fValtanPatternPreviewItemDurationSeconds = 0.f;
	m_ValtanPatternPreviewPlaylist.clear();
	m_strValtanPatternPreviewStatus = status;
	m_bLoop = true;
	m_ValtanPatternPreviewModel.reset();
	m_iValtanPatternPreviewTargetGeneration = 0u;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
#ifdef _DEBUG
	if (const shared_ptr<CValtan> Boss =
		CAnimationTargetService::Resolve_Boss())
	{
		Boss->Clear_PatternHitAreaPreview();
	}
#endif
}

void Client::CAnimation_Tool::Update_ValtanPatternHitAreaPreview()
{
#ifdef _DEBUG
	const shared_ptr<CValtan> Boss = CAnimationTargetService::Resolve_Boss();
	if (nullptr == Boss)
		return;
	if (!m_bValtanPatternPreviewPlaying ||
		m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}
	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
	if (Item.bPatternMarker || 0u == Item.iSourceActionId)
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}

	if (!m_bValtanEncounterReferenceLoadAttempted)
	{
		m_bValtanEncounterReferenceLoadAttempted = true;
		std::string status;
		if (!m_ValtanEncounterReference.Load(CProjectDataRoot::Resolve(
				std::filesystem::path(L"Encounters") / L"Valtan" /
				L"ValtanEncounter.json"), status))
		{
			m_ValtanEncounterReference.Clear();
		}
	}
	if (!m_ValtanEncounterReference.Is_Ready())
		return;

	const ENCOUNTER_PATTERN_REFERENCE* pPattern = nullptr;
	for (const ENCOUNTER_PATTERN_REFERENCE& pattern :
		m_ValtanEncounterReference.Get_Patterns())
	{
		if (std::find(pattern.sourceActionIds.begin(),
				pattern.sourceActionIds.end(),
				Item.iSourceActionId) != pattern.sourceActionIds.end())
		{
			pPattern = &pattern;
			break;
		}
	}
	if (nullptr == pPattern)
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}

	/* The stage lookup runs on the whole-pattern clock: seconds of finished
	   same-sequence items plus the current item's local elapsed. */
	const f32_t fTimelineMs = (m_fValtanPatternHitTimelineBaseSeconds +
		m_fValtanPatternPreviewElapsedSeconds) * 1000.f;
	const ENCOUNTER_STAGE_REFERENCE* pStage = nullptr;
	for (const ENCOUNTER_STAGE_REFERENCE& stage : pPattern->stages)
	{
		const f32_t fStartMs = static_cast<f32_t>(stage.iStartOffsetMs);
		if (fTimelineMs >= fStartMs &&
			fTimelineMs < fStartMs + static_cast<f32_t>(stage.iDurationMs))
		{
			pStage = &stage;
			break;
		}
	}
	if (nullptr == pStage)
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}
	Boss->Set_PatternHitAreaPreview(
		pStage->actionId,
		(fTimelineMs - static_cast<f32_t>(pStage->iStartOffsetMs)) / 1000.f);
#endif
}
