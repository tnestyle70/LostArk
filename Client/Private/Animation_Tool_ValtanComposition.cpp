#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "CharacterPreviewPanel.h"
#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"
#include "Character.h"
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




bool_t Client::CAnimation_Tool::Stage_ValtanCompositionPreview(
	std::string& strOutStatus)
{
	if (!Open_ValtanWorkspace())
	{
		strOutStatus = m_Status;
		m_strValtanPatternPreviewStatus = strOutStatus;
		return false;
	}
	if (nullptr == m_pPreviewPanel)
	{
		strOutStatus =
			"Valtan preview panel is unavailable; canonical data-only authoring remains available.";
		m_strValtanPatternPreviewStatus = strOutStatus;
		return false;
	}
	/* Workbench transport is a programmatic request for the same typed Valtan
	   target, not a user-driven target change.  Animation Tool already applies
	   this same-owner exception during arena auto-stage; apply it here as well
	   so an unsaved Valtan Sound/Binding draft cannot deadlock its own preview.
	   Locks owned by Effect/Equipment or another tool remain set and continue
	   to reject the selection transaction. */
	const bool_t bTemporarilyReleaseOwnLock =
		Is_AnyDocumentDirty() && Is_ValtanDocumentDirty();
	if (bTemporarilyReleaseOwnLock)
	{
		m_pPreviewPanel->Set_SessionLock(
			CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL, false, {});
	}
	const bool_t bSelected =
		m_pPreviewPanel->Select_TargetAsset("Valtan");
	if (bTemporarilyReleaseOwnLock)
	{
		m_pPreviewPanel->Set_SessionLock(
			CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL, true,
			"Save or discard Animation Events, Skill Bindings, Valtan Pattern Animation Bindings, Valtan Pattern Sound, KoukuSaydon Action Bindings/Patterns, and Workbench Sound bindings before changing target.");
	}
	if (!bSelected)
	{
		strOutStatus =
			"Valtan Model View could not be staged; canonical data-only authoring remains available: " +
			m_pPreviewPanel->Get_Status();
		m_strValtanPatternPreviewStatus = strOutStatus;
		return false;
	}
	const std::string strAssetName =
		CAnimationTargetService::Resolve_AssetName();
	const shared_ptr<CValtan> pBoss = CAnimationTargetService::Resolve_Boss();
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if ("Valtan" != strAssetName || nullptr == pBoss || nullptr == pModel ||
		pBoss->Get_BodyModel() != pModel)
	{
		strOutStatus =
			"The selected target is not the dedicated Valtan Model View; no unrelated scene model was substituted.";
		m_strValtanPatternPreviewStatus = strOutStatus;
		return false;
	}
	if (!Sync_AssetName())
	{
		strOutStatus = m_Status;
		m_strValtanPatternPreviewStatus = strOutStatus;
		return false;
	}
	m_iValtanAutoPreviewAttemptGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanAutoPreviewSuccessGeneration =
		m_iValtanAutoPreviewAttemptGeneration;
	m_bValtanAutoPreviewSuppressedForServerPlayback = false;
	strOutStatus = "Dedicated Valtan Model View is ready for Action Composition preview.";
	m_strValtanPatternPreviewStatus = strOutStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Play_ValtanCompositionPattern(
	const std::string& strPatternId,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	std::string& strOutStatus)
{
	if (strPatternId.empty())
	{
		strOutStatus = "Select one stable Valtan Pattern before preview.";
		return false;
	}
	if (!Stage_ValtanCompositionPreview(strOutStatus))
		return false;
	if (!Can_MutateValtanView(m_eValtanPatternMasterAdmission) &&
		!Reload_ValtanPatternMaster())
	{
		strOutStatus = m_strValtanPatternMasterStatus;
		return false;
	}
	const VALTAN_PATTERN_VIEW* const pPattern =
		Find_ValtanPatternMaster(m_ValtanPatternMasterView, strPatternId);
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pPattern || nullptr == pModel)
	{
		strOutStatus = nullptr == pPattern ?
			"The selected Pattern is absent from the admitted animation preview graph." :
			"The dedicated Valtan preview model disappeared before playback.";
		return false;
	}
	m_bValtanCompositionDraftPreviewReady = false;
	m_ValtanCompositionDraftPreview = {};
	m_eValtanPatternMasterPath = ePath;
	const bool_t bStarted = Start_ValtanPatternMasterPreview(
		pModel, *pPattern, ePath);
	strOutStatus = m_strValtanPatternMasterStatus;
	return bStarted;
}

bool_t Client::CAnimation_Tool::Play_ValtanCompositionDraftPattern(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	std::string& strOutStatus)
{
	if (Pattern.strPatternId.empty() || Pattern.Stages.empty())
	{
		strOutStatus =
			"The effective authoring draft has no stable Pattern/stage timeline.";
		return false;
	}
	if (!Stage_ValtanCompositionPreview(strOutStatus))
		return false;
	if (!Can_MutateValtanView(m_eValtanPatternMasterAdmission) &&
		!Reload_ValtanPatternMaster())
	{
		strOutStatus = m_strValtanPatternMasterStatus;
		return false;
	}
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel)
	{
		strOutStatus =
			"The dedicated Valtan preview model disappeared before draft playback.";
		return false;
	}
	m_ValtanCompositionDraftPreview = Pattern;
	m_bValtanCompositionDraftPreviewReady = true;
	m_eValtanPatternMasterPath = ePath;
	const bool_t bStarted = Start_ValtanPatternMasterPreview(
		pModel, m_ValtanCompositionDraftPreview, ePath);
	if (!bStarted)
	{
		m_bValtanCompositionDraftPreviewReady = false;
		m_ValtanCompositionDraftPreview = {};
	}
	strOutStatus = m_strValtanPatternMasterStatus;
	return bStarted;
}

bool_t Client::CAnimation_Tool::Seek_ValtanCompositionPattern(
	const std::string& strPatternId,
	const uint32_t iPositionMs,
	const bool_t bPause,
	std::string& strOutStatus)
{
	const bool_t bSamePattern = m_bValtanPatternMasterPlaying &&
		!m_ValtanPatternMasterPlaylist.empty() &&
		m_ValtanPatternMasterPlaylist.front().strPatternId == strPatternId;
	if (!bSamePattern)
	{
		const bool_t bDraftMatches = m_bValtanCompositionDraftPreviewReady &&
			m_ValtanCompositionDraftPreview.strPatternId == strPatternId;
		const VALTAN_PATTERN_VIEW DraftCopy = bDraftMatches ?
			m_ValtanCompositionDraftPreview : VALTAN_PATTERN_VIEW{};
		const bool_t bStarted = bDraftMatches ?
			Play_ValtanCompositionDraftPattern(
				DraftCopy,
				VALTAN_PATTERN_PREVIEW_PATH::NORMAL, strOutStatus) :
			Play_ValtanCompositionPattern(
				strPatternId, VALTAN_PATTERN_PREVIEW_PATH::NORMAL, strOutStatus);
		if (!bStarted)
			return false;
	}
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel || !Seek_ValtanPatternMasterPreview(
			pModel, static_cast<f32_t>(iPositionMs) * 0.001f, bPause, true))
	{
		strOutStatus = m_strValtanPatternMasterStatus.empty() ?
			"Valtan composition preview seek was rejected." :
			m_strValtanPatternMasterStatus;
		return false;
	}
	strOutStatus = m_strValtanPatternMasterStatus;
	return true;
}

void Client::CAnimation_Tool::Stop_ValtanCompositionPattern(
	std::string& strOutStatus)
{
	if (const shared_ptr<Engine::CModel> pModel = Resolve_Model())
	{
		Stop_ValtanPatternMasterPreview(
			pModel, "Action Composition preview stopped.");
	}
	else
	{
		Reset_ValtanPatternMasterPreviewState(
			"Action Composition preview stopped after its Model View disappeared.");
	}
	strOutStatus = m_strValtanPatternMasterStatus;
}

void Client::CAnimation_Tool::Release_ValtanCompositionPreviewForServerPlayback()
{
	m_bValtanAutoPreviewSuppressedForServerPlayback = true;
	if (nullptr == m_pPreviewPanel || !m_pPreviewPanel->Is_PreviewActive() ||
		"Valtan" != CAnimationTargetService::Resolve_AssetName() ||
		nullptr == CAnimationTargetService::Resolve_Boss())
	{
		return;
	}
	std::string Status;
	Stop_ValtanCompositionPattern(Status);
	if (m_bValtanPatternPreviewPlaying)
		Stop_ValtanPatternPreview(Resolve_Model(), "Local Valtan clip preview released for Server playback.");
	m_pPreviewPanel->Release(true);
	m_strValtanPatternMasterStatus =
		"Local Valtan preview released for Server playback. Local Play opens it again.";
}

Client::CAnimation_Tool::COMPOSITION_PREVIEW_STATE
Client::CAnimation_Tool::Get_ValtanCompositionPreviewState() const
{
	COMPOSITION_PREVIEW_STATE State;
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	const shared_ptr<CValtan> pBoss = CAnimationTargetService::Resolve_Boss();
	State.bModelReady = nullptr != pModel && nullptr != pBoss &&
		pBoss->Get_BodyModel() == pModel &&
		"Valtan" == CAnimationTargetService::Resolve_AssetName();
	State.bPlaying = m_bValtanPatternMasterPlaying;
	State.bPaused = m_bValtanPatternMasterPaused;
	State.bSourceSequencePlaying = m_bValtanPatternPreviewPlaying;
	State.iDurationMs = m_iValtanPatternMasterDurationMs;
	if (!m_ValtanPatternMasterPlaylist.empty())
		State.strPatternId = m_ValtanPatternMasterPlaylist.front().strPatternId;
	if (m_bValtanPatternMasterPlaying &&
		m_iValtanPatternMasterItem < m_ValtanPatternMasterPlaylist.size())
	{
		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
			m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
		const uint64_t iPositionMs =
			static_cast<uint64_t>(Item.iTimelineStartMs) +
			static_cast<uint64_t>(std::llround(
				static_cast<double>(m_fValtanPatternMasterItemElapsedSeconds) *
				1000.0));
		State.iPositionMs = static_cast<uint32_t>((std::min)(
			iPositionMs, static_cast<uint64_t>(State.iDurationMs)));
	}
	State.strStatus = m_strValtanPatternMasterStatus;
	State.strSourceSequenceStatus = m_strValtanPatternPreviewStatus;
	if (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size())
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		const char_t* const pCurrentClip = nullptr == pModel ? nullptr :
			pModel->Get_AnimationName(pModel->Get_CurrentAnimIndex());
		State.strSourceSequenceStatus = "Target=ARENA CLONE | Action=" +
			std::to_string(Item.iSourceActionId) + " | Sequence=" +
			std::to_string(Item.iSequenceIndex) + " | Clip=" +
			(nullptr == pCurrentClip ? Item.strClipName :
				std::string{ pCurrentClip }) + " | Step=" +
			std::to_string(Item.iStepNumber) + "/" +
			std::to_string(Item.iStepCount) +
			" | Server Valtan=UNCHANGED.";
	}
	return State;
}

bool_t Client::CAnimation_Tool::Get_ValtanCompositionSequences(
	std::vector<COMPOSITION_SEQUENCE_VIEW>& OutSequences,
	std::string& strOutStatus)
{
	OutSequences.clear();
	return Load_ValtanCompositionSequenceLibrary(
		OutSequences, strOutStatus);
}

bool_t Client::CAnimation_Tool::Ensure_ValtanCompositionNativeResources(
	std::string& strOutStatus) const
{
	if (!m_bCompositionResourcesReadAttempted)
	{
		std::vector<COMPOSITION_ANIMATION_RESOURCE> resources;
		(void)Read_CompositionAnimationResources(resources, strOutStatus);
	}
	if (std::none_of(m_CompositionAnimationResources.begin(),
		m_CompositionAnimationResources.end(), [](const auto& resource) {
			return resource.strTargetAssetName == "Valtan";
		}))
	{
		strOutStatus = "Valtan physical Animation metadata is unavailable. " +
			m_strCompositionAnimationResourceStatus;
		return false;
	}
	return true;
}

bool_t Client::CAnimation_Tool::Resolve_ValtanCompositionNativeClipDurationMs(
	const std::string& strClipName,
	uint32_t& iOutRoundedDurationMs,
	std::string& strOutStatus) const
{
	iOutRoundedDurationMs = 0u;
	if (!Ensure_ValtanCompositionNativeResources(strOutStatus))
		return false;
	VALTAN_NATIVE_CLIP_INVENTORY Inventory;
	if (!BuildStrictValtanNativeClipInventory(
			m_CompositionAnimationResources, Inventory, strOutStatus))
	{
		return false;
	}
	const auto Found = Inventory.find(strClipName);
	if (Found == Inventory.end())
	{
		strOutStatus =
			"The selected Sequence clip is absent from the physical Valtan Animation catalog: " +
			strClipName + ".";
		return false;
	}
	if (!CActionPresentationTimeline::Validate_AuthoredSourceWindow(
			Found->second.fDurationTicks,
			Found->second.fTicksPerSecond,
			0u, 0u, 1.f, iOutRoundedDurationMs) ||
		0u == iOutRoundedDurationMs)
	{
		strOutStatus =
			"The selected Sequence clip has no usable native duration: " +
			strClipName + ".";
		iOutRoundedDurationMs = 0u;
		return false;
	}
	strOutStatus = "Resolved native Valtan clip duration.";
	return true;
}

bool_t Client::CAnimation_Tool::
Validate_ValtanCompositionAnimationStageMutation(
	const VALTAN_STAGE_VIEW& BaselineStage,
	const VALTAN_STAGE_VIEW& CandidateStage,
	std::string& strOutStatus) const
{
	if (SameValtanAnimationAuthoringSignature(
			BaselineStage, CandidateStage))
	{
		strOutStatus = "Native Animation source windows are unchanged.";
		return true;
	}
	VALTAN_NATIVE_CLIP_INVENTORY Inventory;
	if (!CandidateStage.bSuppressAnimation &&
		"WAIT" != CandidateStage.strSequenceRole)
	{
		if (!Ensure_ValtanCompositionNativeResources(strOutStatus) ||
			!BuildStrictValtanNativeClipInventory(
				m_CompositionAnimationResources, Inventory, strOutStatus))
		{
			return false;
		}
	}
	if (!ValidateValtanStageNativeAnimationWindows(
			CandidateStage, Inventory, strOutStatus))
	{
		return false;
	}
	strOutStatus = "Native Animation Stage mutation admitted.";
	return true;
}

bool_t Client::CAnimation_Tool::
Validate_ValtanCompositionAnimationGraphMutations(
	const std::vector<VALTAN_PATTERN_VIEW>& BaselinePatterns,
	const std::vector<VALTAN_PATTERN_VIEW>& CandidatePatterns,
	std::string& strOutStatus) const
{
	VALTAN_NATIVE_CLIP_INVENTORY Inventory;
	bool_t bInventoryReady = false;
	for (const VALTAN_PATTERN_VIEW& CandidatePattern : CandidatePatterns)
	{
		const auto BaselinePattern = std::find_if(
			BaselinePatterns.begin(), BaselinePatterns.end(),
			[&CandidatePattern](const VALTAN_PATTERN_VIEW& Pattern)
			{
				return Pattern.strPatternId == CandidatePattern.strPatternId;
			});
		for (const VALTAN_STAGE_VIEW& CandidateStage : CandidatePattern.Stages)
		{
			const VALTAN_STAGE_VIEW* pBaselineStage = nullptr;
			if (BaselinePattern != BaselinePatterns.end())
			{
				const auto FoundStage = std::find_if(
					BaselinePattern->Stages.begin(), BaselinePattern->Stages.end(),
					[&CandidateStage](const VALTAN_STAGE_VIEW& Stage)
					{
						return Stage.strStageId == CandidateStage.strStageId;
					});
				if (FoundStage != BaselinePattern->Stages.end())
					pBaselineStage = &*FoundStage;
			}
			if (nullptr != pBaselineStage &&
				SameValtanAnimationAuthoringSignature(
					*pBaselineStage, CandidateStage))
			{
				continue;
			}
			if (!CandidateStage.bSuppressAnimation &&
				"WAIT" != CandidateStage.strSequenceRole &&
				!bInventoryReady)
			{
				if (!Ensure_ValtanCompositionNativeResources(strOutStatus) ||
					!BuildStrictValtanNativeClipInventory(
						m_CompositionAnimationResources, Inventory, strOutStatus))
				{
					return false;
				}
				bInventoryReady = true;
			}
			if (!ValidateValtanStageNativeAnimationWindows(
					CandidateStage, Inventory, strOutStatus))
			{
				strOutStatus = CandidatePattern.strPatternId + "/" +
					CandidateStage.strStageId + ": " + strOutStatus;
				return false;
			}
		}
	}
	strOutStatus = "All changed native Animation source windows are admitted.";
	return true;
}

bool_t Client::CAnimation_Tool::Preview_ValtanCompositionSequence(
	const int32_t iSkillId,
	const int32_t iSequenceIndex,
	std::string& strOutStatus)
{
	if (!Stage_ValtanCompositionPreview(strOutStatus))
		return false;
	if (!m_bClipSeqLoadAttempted)
	{
		m_bClipSeqLoadAttempted = true;
		(void)Load_ClipSeq();
	}
	const auto Found = std::find_if(
		m_ClipSeqs.begin(), m_ClipSeqs.end(),
		[iSkillId, iSequenceIndex](const CLIP_SEQ& Sequence)
		{
			return Sequence.iSkillId == iSkillId &&
				Sequence.iSeqIndex == iSequenceIndex;
		});
	if (Found == m_ClipSeqs.end())
	{
		strOutStatus = "The selected extracted Animation Sequence no longer exists.";
		return false;
	}
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel || !Start_ValtanSequencePreview(
			pModel, static_cast<std::size_t>(Found - m_ClipSeqs.begin())))
	{
		strOutStatus = "Could not preview the selected extracted Animation Sequence.";
		return false;
	}
	const char_t* const pCurrentClip = pModel->Get_AnimationName(
		pModel->Get_CurrentAnimIndex());
	strOutStatus = "Target=ARENA CLONE | Action=" +
		std::to_string(iSkillId) + " | Sequence=" +
		std::to_string(iSequenceIndex) + " | Clip=" +
		(nullptr == pCurrentClip ? std::string{ "UNKNOWN" } :
			std::string{ pCurrentClip }) +
		" | Server Valtan=UNCHANGED.";
	m_strValtanPatternPreviewStatus = strOutStatus;
	return true;
}

bool_t Client::CAnimation_Tool::Stage_ValtanCompositionIntakeSequence(
	const int32_t iSkillId,
	const int32_t iSequenceIndex,
	const std::string& strTargetPatternId,
	const std::string& strTargetStageId,
	std::string& strOutStatus)
{
	std::vector<COMPOSITION_SEQUENCE_VIEW> Sequences;
	if (!Get_ValtanCompositionSequences(Sequences, strOutStatus))
		return false;
	const auto Found = std::find_if(
		Sequences.begin(), Sequences.end(),
		[iSkillId, iSequenceIndex](const COMPOSITION_SEQUENCE_VIEW& Sequence)
		{
			return Sequence.iSkillId == iSkillId &&
				Sequence.iSequenceIndex == iSequenceIndex;
		});
	if (Found == Sequences.end() || Found->Clips.empty())
	{
		strOutStatus = "The selected Animation Sequence cannot stage an empty Intake.";
		return false;
	}
	m_CustomChainSteps.clear();
	m_CustomChainSteps.reserve(Found->Clips.size());
	for (const COMPOSITION_SEQUENCE_CLIP_VIEW& Clip : Found->Clips)
	{
		CUSTOM_CHAIN_STEP Step;
		Step.clipName = Clip.strClipName;
		Step.fDurationSeconds = static_cast<f32_t>(Clip.iDurationMs) * 0.001f;
		m_CustomChainSteps.push_back(std::move(Step));
	}
	const std::string strChainId = "sequence." + std::to_string(iSkillId) +
		"." + std::to_string(iSequenceIndex);
	snprintf(m_CustomChainId, sizeof(m_CustomChainId), "%s", strChainId.c_str());
	snprintf(m_CustomChainTargetPatternId,
		sizeof(m_CustomChainTargetPatternId), "%s", strTargetPatternId.c_str());
	snprintf(m_CustomChainTargetStageId,
		sizeof(m_CustomChainTargetStageId), "%s", strTargetStageId.c_str());
	m_iValtanPatternCreateSourceKind = 0;
	m_bValtanPatternCreateExactSourceSelection = true;
	m_iValtanPatternCreateSourceActionId = iSkillId;
	m_iValtanPatternCreateSourceSequenceIndex = iSequenceIndex;
	m_strValtanPatternCreateValidatedRequestSha256.clear();
	strOutStatus = "Staged " + std::to_string(m_CustomChainSteps.size()) +
		" Sequence clips in Create New Pattern Intake. Review the Pattern ID and Apply transaction below.";
	return true;
}

void Client::CAnimation_Tool::Set_ValtanCompositionLoop(const bool_t bLoop)
{
	m_bValtanCompositionLoop = bLoop;
}

bool_t Client::CAnimation_Tool::Consume_ValtanCompositionPatternCreated(
	std::string& strOutPatternId)
{
	if (!m_bValtanCompositionPatternCreatedPending)
		return false;
	strOutPatternId = std::move(m_strValtanCompositionPatternCreatedId);
	m_strValtanCompositionPatternCreatedId.clear();
	m_bValtanCompositionPatternCreatedPending = false;
	return !strOutPatternId.empty();
}

bool_t Client::CAnimation_Tool::Is_ValtanCompositionPatternTransactionActive() const
{
	return nullptr != m_hValtanPatternCreateProcess;
}

void Client::CAnimation_Tool::Render_ValtanCompositionPatternCreator()
{
	if (!m_bCustomChainLibraryLoadAttempted)
	{
		m_bCustomChainLibraryLoadAttempted = true;
		(void)Load_CustomChainLibrary();
	}
	if (!m_bValtanPatternMasterLoadAttempted)
	{
		/* This tab is rendered every frame.  Latch the command edge before the
		   file-backed canonical reload so a rejected load is not retried at the
		   render rate. */
		m_bValtanPatternMasterLoadAttempted = true;
		(void)Reload_ValtanPatternMaster();
	}
	Render_ValtanPatternCreatePanel();
}
