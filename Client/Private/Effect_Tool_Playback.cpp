#include "Effect_Tool_Internal.h"
#include "ActionPresentationTimeline.h"
#include "AnimationSkillBindingDocument.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_DocumentRenderer.h"
#include "ValtanPatternEffectCueDocument.h"
#include "Effect_Object.h"
#include "Effect_Playback.h"
#include "Effect_PresentationService.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Logic_DimensionMaster.h"
#include "MapEffectPresentationRuntime.h"
#include "Model.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Transform.h"
#include "CharacterPreviewPanel.h"
#include "EffectAuthoringSequencer.h"

void Client::CEffect_Tool::Reset_BufferedComboAudition()
{
	m_bBufferedComboAuditionActive = false;
	m_eBufferedComboAuditionClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	m_iBufferedComboAuditionSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	m_fBufferedComboAuditionDurationSeconds = 0.f;
	m_fBufferedComboAuditionOccurrenceOffsetSeconds = 0.f;
}

bool_t Client::CEffect_Tool::Try_BuildBufferedComboAnimationClips(
	const PLAYER_SKILL_DEFINITION& Skill,
	const std::vector<PLAYER_SKILL_DEFINITION>& Skills,
	std::vector<SYNCHRONIZED_ANIMATION_CLIP>& OutClips,
	std::vector<std::vector<f32_t>>& OutStageClipOffsetsSeconds,
	f32_t& fOutDurationSeconds,
	std::string& strOutError)
{
	OutClips.clear();
	OutStageClipOffsetsSeconds.clear();
	fOutDurationSeconds = 0.f;
	strOutError.clear();
	if (Skill.eSkillKind != LostArk::Shared::PLAYER_SKILL_KIND::COMBO ||
		Skill.ComboStages.size() < 2u ||
		Skill.ComboStages.size() != Skill.iComboStageCount)
	{
		strOutError =
			"the selected skill has no complete Server combo timing contract.";
		return false;
	}

	const char_t* pAnimationAsset = Animation_AssetName(Skill.eCharacterClass);
	if (nullptr == pAnimationAsset)
	{
		strOutError = "the combo class has no admitted animation asset.";
		return false;
	}
	if (CAnimationTargetService::Resolve_AssetName() != pAnimationAsset &&
		!m_pCharacterPreviewPanel->Select_TargetAsset(pAnimationAsset))
	{
		strOutError = "the combo class model could not be staged.";
		return false;
	}
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		strOutError = "the combo class model is unavailable.";
		return false;
	}

	ANIMATION_SKILL_BINDING_DOCUMENT Bindings;
	std::string BindingStatus;
	if (!CAnimationSkillBindingDocument::Load(
			pAnimationAsset, Skill.eCharacterClass, Skills,
			Collect_AnimationClipNames(pModel), Bindings, BindingStatus))
	{
		strOutError = "the skill binding could not be loaded: " + BindingStatus;
		return false;
	}
	const auto Binding = std::find_if(
		Bindings.Bindings.begin(), Bindings.Bindings.end(),
		[&Skill](const ANIMATION_SKILL_BINDING& Candidate)
		{
			return Candidate.iSkillId == Skill.iSkillId;
		});
	if (Binding == Bindings.Bindings.end() ||
		Binding->Stages.size() != Skill.ComboStages.size())
	{
		strOutError =
			"the skill binding stage count does not match Server comboStages.";
		return false;
	}

	std::vector<SYNCHRONIZED_ANIMATION_CLIP> StagedClips;
	std::vector<std::vector<f32_t>> StagedStageClipOffsetsSeconds;
	double fStagedDurationSeconds = 0.0;
	for (size_t iStage = 0u; iStage < Binding->Stages.size(); ++iStage)
	{
		const ANIMATION_SKILL_STAGE& Stage = Binding->Stages[iStage];
		const PLAYER_COMBO_STAGE_TIMING& ServerTiming =
			Skill.ComboStages[iStage];
		const bool_t bFinalStage = iStage + 1u == Binding->Stages.size();
		const uint32_t iBoundaryMs = bFinalStage ?
			ServerTiming.iActionDurationMs : ServerTiming.iComboAdvanceMs;
		if (Stage.Clips.empty() || 0u == iBoundaryMs ||
			iBoundaryMs > ServerTiming.iActionDurationMs)
		{
			strOutError = "combo stage " + std::to_string(iStage + 1u) +
				" has an invalid buffered boundary or no clips.";
			return false;
		}

		f32_t fRemainingWallSeconds =
			static_cast<f32_t>(iBoundaryMs) * 0.001f;
		bool_t bReachedBoundary = false;
		const size_t iFirstStagedClip = StagedClips.size();
		std::vector<f32_t> StageClipOffsetsSeconds;
		f32_t fStageElapsedSeconds = 0.f;
		constexpr f32_t BOUNDARY_EPSILON_SECONDS = 0.0005f;
		for (const ANIMATION_SKILL_CLIP& Clip : Stage.Clips)
		{
			uint32_t iAnimation = UINT32_MAX;
			for (uint32_t iCandidate = 0u;
				iCandidate < pModel->Get_NumAnimations(); ++iCandidate)
			{
				const char_t* pName = pModel->Get_AnimationName(iCandidate);
				if (nullptr == pName || Clip.strClipName != pName)
					continue;
				if (UINT32_MAX != iAnimation)
				{
					strOutError = "combo stage " +
						std::to_string(iStage + 1u) +
						" resolves an ambiguous duplicate model clip: " +
						Clip.strClipName;
					return false;
				}
				iAnimation = iCandidate;
			}
			if (UINT32_MAX == iAnimation)
			{
				strOutError = "combo stage " + std::to_string(iStage + 1u) +
					" is missing model clip: " + Clip.strClipName;
				return false;
			}

			f32_t fPositionTicks = 0.f;
			f32_t fDurationTicks = 0.f;
			const f32_t fTicksPerSecond =
				pModel->Get_AnimationTickPerSecond(iAnimation);
			if (!pModel->Get_AnimationProgress(
					iAnimation, fPositionTicks, fDurationTicks) ||
				!std::isfinite(fDurationTicks) || fDurationTicks <= 0.f ||
				!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f)
			{
				strOutError = "combo stage " + std::to_string(iStage + 1u) +
					" has no valid duration for model clip: " + Clip.strClipName;
				return false;
			}

			ACTION_PRESENTATION_CLIP_TIMING ClipTiming;
			ClipTiming.fModelSourceDurationSeconds =
				fDurationTicks / fTicksPerSecond;
			ClipTiming.iPlayMs = Clip.iPlayMs;
			ClipTiming.fPlayRate = Clip.fPlayRate;
			ClipTiming.fSourceStartSeconds =
				static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
			f32_t fSourceDurationSeconds = 0.f;
			f32_t fClipWallDurationSeconds = 0.f;
			if (!CActionPresentationTimeline::Resolve_ClipDuration(
					ClipTiming, fSourceDurationSeconds,
					fClipWallDurationSeconds))
			{
				strOutError = "combo stage " + std::to_string(iStage + 1u) +
					" has an invalid source window: " + Clip.strClipName;
				return false;
			}

			if (fRemainingWallSeconds + BOUNDARY_EPSILON_SECONDS >=
				fClipWallDurationSeconds)
			{
				StageClipOffsetsSeconds.push_back(static_cast<f32_t>(
					fStagedDurationSeconds) + fStageElapsedSeconds);
				StagedClips.emplace_back(Clip);
				fStageElapsedSeconds += fClipWallDurationSeconds;
				fRemainingWallSeconds = (std::max)(
					0.f, fRemainingWallSeconds - fClipWallDurationSeconds);
				if (fRemainingWallSeconds <= BOUNDARY_EPSILON_SECONDS)
				{
					bReachedBoundary = true;
					break;
				}
				continue;
			}

			const double fTrimSourceMilliseconds =
				static_cast<double>(fRemainingWallSeconds) * 1000.0 *
				static_cast<double>(Clip.fPlayRate);
			if (!std::isfinite(fTrimSourceMilliseconds) ||
				fTrimSourceMilliseconds <= 0.0 ||
				fTrimSourceMilliseconds >
					static_cast<double>((std::numeric_limits<uint32_t>::max)()))
			{
				strOutError = "combo stage " + std::to_string(iStage + 1u) +
					" cannot represent its buffered source cutoff.";
				return false;
			}
			SYNCHRONIZED_ANIMATION_CLIP Trimmed(Clip);
			Trimmed.iPlayMs = (std::max)(1u, static_cast<uint32_t>(
				std::llround(fTrimSourceMilliseconds)));
			if (0u != Clip.iPlayMs)
				Trimmed.iPlayMs = (std::min)(Trimmed.iPlayMs, Clip.iPlayMs);
			StageClipOffsetsSeconds.push_back(static_cast<f32_t>(
				fStagedDurationSeconds) + fStageElapsedSeconds);
			StagedClips.push_back(std::move(Trimmed));
			fRemainingWallSeconds = 0.f;
			bReachedBoundary = true;
			break;
		}
		if (!bReachedBoundary &&
			StagedClips.size() > iFirstStagedClip &&
			fRemainingWallSeconds > BOUNDARY_EPSILON_SECONDS)
		{
			/* The Server can keep a combo stage alive after its authored clip has
			   ended. Preserve that wall time as an end-pose hold before the next
			   input stage begins; do not stretch the clip or alter playRate. */
			StagedClips.back().fHoldAfterSeconds += fRemainingWallSeconds;
			fRemainingWallSeconds = 0.f;
			bReachedBoundary = true;
		}
		if (!bReachedBoundary)
		{
			strOutError = "combo stage " + std::to_string(iStage + 1u) +
				" could not stage its buffered Server boundary.";
			return false;
		}
		StagedStageClipOffsetsSeconds.push_back(
			std::move(StageClipOffsetsSeconds));
		fStagedDurationSeconds +=
			static_cast<double>(iBoundaryMs) * 0.001;
	}

	if (StagedClips.empty() || !std::isfinite(fStagedDurationSeconds) ||
		fStagedDurationSeconds <= 0.0 ||
		fStagedDurationSeconds >
			static_cast<double>((std::numeric_limits<f32_t>::max)()))
	{
		strOutError = "the buffered combo timeline has no valid duration.";
		return false;
	}
	OutClips = std::move(StagedClips);
	OutStageClipOffsetsSeconds =
		std::move(StagedStageClipOffsetsSeconds);
	fOutDurationSeconds = static_cast<f32_t>(fStagedDurationSeconds);
	return true;
}

bool_t Client::CEffect_Tool::Try_PlayBufferedComboAudition(
	const EFFECT_SKILL_TREE_ENTRY& Entry)
{
	if (Entry.Skill.eSkillKind !=
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO ||
		Entry.ProductCues.empty())
	{
		m_strElementStatus =
			"Buffered combo audition requires a COMBO skill with Product cues.";
		return false;
	}

	std::string CatalogStatus;
	if (!Ensure_PlayerSkillCatalog(CatalogStatus))
	{
		m_strElementStatus =
			"Buffered combo audition could not refresh PlayerSkills: " +
			CatalogStatus;
		return false;
	}
	const std::vector<PLAYER_SKILL_DEFINITION>& Skills =
		CPlayerSkillCatalog::Get_Skills();
	const auto Skill = std::find_if(Skills.begin(), Skills.end(),
		[&Entry](const PLAYER_SKILL_DEFINITION& Candidate)
		{
			return Candidate.eCharacterClass == Entry.Skill.eCharacterClass &&
				Candidate.iSkillId == Entry.Skill.iSkillId;
		});
	if (Skill == Skills.end())
	{
		m_strElementStatus =
			"Buffered combo audition rejected a stale All Effects skill row.";
		return false;
	}

	/* Clip-duration validation may stage the combo class model. Keep the exact
	   Product preview transactional across builder, Product selection and first
	   clip start failures. Authoritative data reconstructs the prior sequence. */
	const std::string strPreviousTargetAsset =
		CAnimationTargetService::Resolve_AssetName();
	const optional<EFFECT_PRODUCT_PREVIEW> PreviousProductPreview =
		m_ProductPreview;
	const optional<VALTAN_PRODUCT_PREVIEW> PreviousValtanProductPreview =
		m_ValtanProductPreview;
	const optional<EFFECT_DOCUMENT_DESC> PreviousSourcePreviewDocument =
		m_SourcePreviewDocument;
	const bool_t bPreviousReconstructedSourceRuntimeActive =
		m_bReconstructedSourceRuntimeActive;
	const LostArk::Shared::CHARACTER_CLASS_ID ePreviousAllEffectsClass =
		m_eAllEffectsClass;
	const std::string strPreviousAuthoringDomainId =
		m_strSelectedAuthoringDomainId;
	const EFFECT_PREVIEW_FILTER ePreviousPreviewFilter = m_ePreviewFilter;
	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	const std::string strPreviousIsolationGroup =
		m_strPreviewIsolationGroupId;
	const shared_ptr<CEffectObject> pPreviousPreviewObject =
		m_pWorldPreviewObject.lock();
	const bool_t bPreviousPreviewObjectAvailable =
		nullptr != pPreviousPreviewObject;
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION
		PreviousPreviewSubmissionIsolation =
			bPreviousPreviewObjectAvailable ?
				pPreviousPreviewObject->Get_PreviewSubmissionIsolation() :
				EFFECT_PREVIEW_SUBMISSION_ISOLATION{};
	const bool_t bPreviousScreenPostEnabled = m_bPreviewScreenPostEnabled;
	const f32_t fPreviousPreviewTimeSeconds = m_fPreviewTimeSeconds;
	const f32_t fPreviousPreviewDurationSeconds =
		m_fPreviewDurationSeconds;
	const uint32_t iPreviousValtanWorldOwnerStageDurationMs =
		m_iValtanWorldOwnerStageDurationMs;
	const bool_t bPreviousPreviewPlaying = m_bPreviewPlaying;
	const bool_t bPreviousPreviewVisibleRequested =
		m_bPreviewVisibleRequested;
	const float4x4_t PreviousProductCueSnapshotRoot =
		m_ProductCueSnapshotRoot;
	const bool_t bPreviousProductCueSnapshotCaptured =
		m_bProductCueSnapshotCaptured;
	const f32_t fPreviousProductCueActionFacingYawDegrees =
		m_fProductCueActionFacingYawDegrees;
	const bool_t bPreviousProductCueActionFacingCaptured =
		m_bProductCueActionFacingCaptured;
	const bool_t bPreviousBufferedComboAuditionActive =
		m_bBufferedComboAuditionActive;
	const LostArk::Shared::CHARACTER_CLASS_ID
		ePreviousBufferedComboAuditionClass =
			m_eBufferedComboAuditionClass;
	const LostArk::Shared::SKILL_ID iPreviousBufferedComboAuditionSkillId =
		m_iBufferedComboAuditionSkillId;
	const f32_t fPreviousBufferedComboAuditionDurationSeconds =
		m_fBufferedComboAuditionDurationSeconds;
	const f32_t fPreviousBufferedComboAuditionOccurrenceOffsetSeconds =
		m_fBufferedComboAuditionOccurrenceOffsetSeconds;

	const auto RestoreBufferedComboRollback =
		[this, &strPreviousTargetAsset, &PreviousProductPreview,
		 &PreviousValtanProductPreview, &PreviousSourcePreviewDocument,
		 bPreviousReconstructedSourceRuntimeActive,
		 ePreviousAllEffectsClass,
		 &strPreviousAuthoringDomainId,
		 ePreviousPreviewFilter, bPreviousScreenPostEnabled,
		 &strPreviousIsolationElement, &strPreviousIsolationGroup,
		 bPreviousPreviewObjectAvailable,
		 &PreviousPreviewSubmissionIsolation,
		 fPreviousPreviewTimeSeconds, fPreviousPreviewDurationSeconds,
		 iPreviousValtanWorldOwnerStageDurationMs,
		 bPreviousPreviewPlaying, bPreviousPreviewVisibleRequested,
		 &PreviousProductCueSnapshotRoot,
		 bPreviousProductCueSnapshotCaptured,
		 fPreviousProductCueActionFacingYawDegrees,
		 bPreviousProductCueActionFacingCaptured,
		 bPreviousBufferedComboAuditionActive,
		 ePreviousBufferedComboAuditionClass,
		 iPreviousBufferedComboAuditionSkillId,
		 fPreviousBufferedComboAuditionDurationSeconds,
		 fPreviousBufferedComboAuditionOccurrenceOffsetSeconds](
			const bool_t bAuditionStateMutated,
			std::string& strOutError)
	{
		strOutError.clear();
		const std::string strCurrentTargetAsset =
			CAnimationTargetService::Resolve_AssetName();
		if (!bAuditionStateMutated &&
			strCurrentTargetAsset == strPreviousTargetAsset)
		{
			return true;
		}
		Reset_SynchronizedAnimationSequence();
		if (strCurrentTargetAsset != strPreviousTargetAsset)
		{
			if (strPreviousTargetAsset.empty())
				m_pCharacterPreviewPanel->Release(true);
			else if (!m_pCharacterPreviewPanel->Select_TargetAsset(
					strPreviousTargetAsset))
			{
				strOutError = "the previous target model could not be restored";
				return false;
			}
		}

		m_ProductPreview = PreviousProductPreview;
		m_ValtanProductPreview = PreviousValtanProductPreview;
		m_SourcePreviewDocument = PreviousSourcePreviewDocument;
		m_ePreviewFilter = ePreviousPreviewFilter;
		m_bPreviewScreenPostEnabled = bPreviousScreenPostEnabled;
		m_bBufferedComboAuditionActive =
			bPreviousBufferedComboAuditionActive;
		m_eBufferedComboAuditionClass =
			ePreviousBufferedComboAuditionClass;
		m_iBufferedComboAuditionSkillId =
			iPreviousBufferedComboAuditionSkillId;
		m_fBufferedComboAuditionDurationSeconds =
			fPreviousBufferedComboAuditionDurationSeconds;
		m_fBufferedComboAuditionOccurrenceOffsetSeconds =
			fPreviousBufferedComboAuditionOccurrenceOffsetSeconds;
		m_iValtanWorldOwnerStageDurationMs =
			iPreviousValtanWorldOwnerStageDurationMs;
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
		m_ProductCueSnapshotRoot = PreviousProductCueSnapshotRoot;
		m_bProductCueSnapshotCaptured =
			bPreviousProductCueSnapshotCaptured;
		m_fProductCueActionFacingYawDegrees =
			fPreviousProductCueActionFacingYawDegrees;
		m_bProductCueActionFacingCaptured =
			bPreviousProductCueActionFacingCaptured;
		m_fPreviewTimeSeconds = fPreviousPreviewTimeSeconds;
		m_fPreviewDurationSeconds = fPreviousPreviewDurationSeconds;
		m_bPreviewPlaying = bPreviousPreviewPlaying;
		m_bPreviewVisibleRequested = bPreviousPreviewVisibleRequested;
		const auto RestoreSubmissionIsolation =
			[this, bPreviousPreviewObjectAvailable,
			 &PreviousPreviewSubmissionIsolation](std::string& strError)
		{
			if (!bPreviousPreviewObjectAvailable)
				return true;
			const shared_ptr<CEffectObject> pObject =
				m_pWorldPreviewObject.lock();
			if (nullptr == pObject)
			{
				strError =
					"the previous preview isolation has no restored EffectObject";
				return false;
			}
			std::string IsolationError;
			if (!pObject->Set_PreviewSubmissionIsolation(
					PreviousPreviewSubmissionIsolation, IsolationError))
			{
				strError = "the previous preview isolation could not be restored: " +
					IsolationError;
				return false;
			}
			return true;
		};
		const auto RestoreCommonPreviewFields = [this,
			ePreviousAllEffectsClass, ePreviousPreviewFilter,
			&strPreviousAuthoringDomainId,
			bPreviousScreenPostEnabled, &strPreviousIsolationElement,
			&strPreviousIsolationGroup, &PreviousProductCueSnapshotRoot,
			bPreviousProductCueSnapshotCaptured,
			fPreviousProductCueActionFacingYawDegrees,
			bPreviousProductCueActionFacingCaptured]()
		{
			m_eAllEffectsClass = ePreviousAllEffectsClass;
			m_strSelectedAuthoringDomainId = strPreviousAuthoringDomainId;
			m_ePreviewFilter = ePreviousPreviewFilter;
			m_bPreviewScreenPostEnabled = bPreviousScreenPostEnabled;
			m_strPreviewIsolationElementId = strPreviousIsolationElement;
			m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
			m_ProductCueSnapshotRoot = PreviousProductCueSnapshotRoot;
			m_bProductCueSnapshotCaptured =
				bPreviousProductCueSnapshotCaptured;
			m_fProductCueActionFacingYawDegrees =
				fPreviousProductCueActionFacingYawDegrees;
			m_bProductCueActionFacingCaptured =
				bPreviousProductCueActionFacingCaptured;
		};

		if (bPreviousReconstructedSourceRuntimeActive)
		{
			if (!Try_StartArtist31470FullPreview())
			{
				strOutError =
					"the previous Artist F reconstructed preview could not restart: " +
					m_strPreviewStatus;
				return false;
			}
			if (!Seek_ReconstructedSourceRuntimeTimeline(
					fPreviousPreviewTimeSeconds))
			{
				strOutError =
					"the previous Artist F reconstructed preview time could not be restored: " +
					m_strPreviewStatus;
				return false;
			}
			m_fPreviewDurationSeconds = fPreviousPreviewDurationSeconds;
			m_bPreviewPlaying = bPreviousPreviewPlaying;
			m_bPreviewVisibleRequested = bPreviousPreviewVisibleRequested;
			Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
			if (const shared_ptr<CEffectObject> pObject =
					m_pWorldPreviewObject.lock())
			{
				pObject->Set_Playing(false);
				pObject->Set_Visible(m_bPreviewVisibleRequested);
			}
			RestoreCommonPreviewFields();
			return RestoreSubmissionIsolation(strOutError);
		}

		if (m_ValtanProductPreview.has_value())
		{
			const bool_t bRestored = Restore_ValtanProductPreviewPlayback(
				m_ValtanProductPreview,
				fPreviousPreviewTimeSeconds,
				fPreviousPreviewDurationSeconds,
				bPreviousPreviewPlaying,
				bPreviousPreviewVisibleRequested,
				PreviousProductCueSnapshotRoot,
				bPreviousProductCueSnapshotCaptured,
				strOutError);
			if (!bRestored)
				return false;
			RestoreCommonPreviewFields();
			return RestoreSubmissionIsolation(strOutError);
		}

		Synchronize_LoadedSkillPreview();
		const EFFECT_DOCUMENT_DESC* pRestoreDocument =
			m_ProductPreview.has_value() &&
			m_SourcePreviewDocument.has_value() ?
				&*m_SourcePreviewDocument :
				(m_ActiveDocument.has_value() ? &*m_ActiveDocument : nullptr);
		if (nullptr == pRestoreDocument)
		{
			Release_WorldPreview(true);
			RestoreCommonPreviewFields();
			return true;
		}
		if (!Stage_WorldPreview(
				*pRestoreDocument,
				m_ProductPreview.has_value() &&
					m_SourcePreviewDocument.has_value()))
		{
			strOutError = "the previous Effect preview could not be restored: " +
				m_strPreviewStatus;
			return false;
		}
		m_fPreviewTimeSeconds = fPreviousPreviewTimeSeconds;
		m_fPreviewDurationSeconds = fPreviousPreviewDurationSeconds;
		m_bPreviewPlaying = bPreviousPreviewPlaying;
		m_bPreviewVisibleRequested = bPreviousPreviewVisibleRequested;
		Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
		Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
		if (const shared_ptr<CEffectObject> pObject =
				m_pWorldPreviewObject.lock())
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(m_bPreviewVisibleRequested &&
				Is_ProductCueVisible(m_fPreviewTimeSeconds));
		}
		RestoreCommonPreviewFields();
		return RestoreSubmissionIsolation(strOutError);
	};
	std::vector<SYNCHRONIZED_ANIMATION_CLIP> StagedClips;
	std::vector<std::vector<f32_t>> StagedStageClipOffsetsSeconds;
	f32_t fStagedDurationSeconds = 0.f;
	std::string BuildError;
	if (!Try_BuildBufferedComboAnimationClips(
			*Skill, Skills, StagedClips, StagedStageClipOffsetsSeconds,
			fStagedDurationSeconds, BuildError))
	{
		std::string RollbackError;
		const bool_t bRestored =
			RestoreBufferedComboRollback(false, RollbackError);
		m_strElementStatus =
			"Buffered combo audition failed closed: " + BuildError +
			(bRestored ? std::string{} :
				" Rollback failed: " + RollbackError);
		return false;
	}

	const bool_t bCurrentProductMatches = m_ProductPreview.has_value() &&
		m_ProductPreview->eCharacterClass == Skill->eCharacterClass &&
		m_ProductPreview->iSkillId == Skill->iSkillId;
	if (!bCurrentProductMatches)
	{
		const auto FirstCue = std::min_element(
			Entry.ProductCues.begin(), Entry.ProductCues.end(),
			[](const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Left,
				const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Right)
			{
				return std::tie(Left.iStageIndex, Left.iStageClipIndex,
					Left.iBoundClipOrdinal, Left.Cue.iStartMs) <
					std::tie(Right.iStageIndex, Right.iStageClipIndex,
						Right.iBoundClipOrdinal, Right.Cue.iStartMs);
			});
		const size_t iFirstCueIndex = static_cast<size_t>(
			std::distance(Entry.ProductCues.begin(), FirstCue));
		if (!Try_SelectProductCue(Entry, iFirstCueIndex))
		{
			const std::string SelectError = m_strElementStatus;
			std::string RollbackError;
			const bool_t bRestored =
				RestoreBufferedComboRollback(true, RollbackError);
			m_strElementStatus = SelectError +
				(bRestored ?
					" Previous Product preview was restored." :
					" Rollback failed: " + RollbackError);
			return false;
		}
	}
	const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& SelectedOccurrence =
		m_ProductPreview->ProductCue;
	if (SelectedOccurrence.iStageIndex >=
			StagedStageClipOffsetsSeconds.size() ||
		SelectedOccurrence.iStageClipIndex >=
			StagedStageClipOffsetsSeconds[
				SelectedOccurrence.iStageIndex].size())
	{
		std::string RollbackError;
		const bool_t bRestored =
			RestoreBufferedComboRollback(true, RollbackError);
		m_strElementStatus =
			"Buffered combo audition rejected a Product occurrence outside its Server stage boundary." +
			(bRestored ?
				" Previous Product preview was restored." :
				" Rollback failed: " + RollbackError);
		return false;
	}
	const f32_t fOccurrenceOffsetSeconds =
		StagedStageClipOffsetsSeconds[SelectedOccurrence.iStageIndex]
			[SelectedOccurrence.iStageClipIndex];
	if (!std::isfinite(fOccurrenceOffsetSeconds) ||
		fOccurrenceOffsetSeconds < 0.f ||
		fOccurrenceOffsetSeconds > fStagedDurationSeconds)
	{
		std::string RollbackError;
		const bool_t bRestored =
			RestoreBufferedComboRollback(true, RollbackError);
		m_strElementStatus =
			"Buffered combo audition resolved an invalid Product occurrence offset." +
			(bRestored ?
				" Previous Product preview was restored." :
				" Rollback failed: " + RollbackError);
		return false;
	}

	m_bBufferedComboAuditionActive = true;
	m_eBufferedComboAuditionClass = Skill->eCharacterClass;
	m_iBufferedComboAuditionSkillId = Skill->iSkillId;
	m_fBufferedComboAuditionDurationSeconds = fStagedDurationSeconds;
	m_fBufferedComboAuditionOccurrenceOffsetSeconds =
		fOccurrenceOffsetSeconds;
	m_SynchronizedAnimationClips = std::move(StagedClips);
	m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
	m_iSynchronizedAnimationTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	if (!Start_SynchronizedAnimationClip(0u, false))
	{
		std::string RollbackError;
		const bool_t bRestored =
			RestoreBufferedComboRollback(true, RollbackError);
		m_strElementStatus = bRestored ?
			"Buffered combo audition failed to start; exact Product cue playback was restored." :
			"Buffered combo audition failed to start and rollback failed: " +
				RollbackError;
		return false;
	}

	Recalculate_PreviewDuration();
	Start_WorldPreviewFromBeginning();
	m_strPreviewAnimationStatus = "Buffered combo audition: " +
		Skill->strInputSlot + " | " + Skill->strDisplayName + " | " +
		std::to_string(Skill->ComboStages.size()) + " Server input stages | " +
		std::to_string(static_cast<uint64_t>(
			std::llround(static_cast<double>(
				fStagedDurationSeconds) * 1000.0))) + " ms";
	m_strElementStatus =
		"Playing buffered combo animation audition | " +
		Skill->strDisplayName +
		" | selected Product Effect remains occurrence-local.";
	return true;
}

bool_t Client::CEffect_Tool::Ensure_WorldPreviewObject()
{
    if (nullptr != m_pWorldPreviewObject.lock())
        return true;
    const uint32_t iLevel = CGameInstance::Get().Get_CurrentLevelID();
    CEffectObject::EFFECT_OBJECT_DESC Desc{};
    Desc.pDocument = nullptr;
    Desc.RootWorld = m_PreviewWorldRoot;
    Desc.bAutoPlay = false;
    shared_ptr<CGameObject> pGameObject;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
        ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectObject",
        iLevel, PREVIEW_LAYER, &Desc, &pGameObject)))
    {
        m_strPreviewStatus =
            "EffectObject prototype is not registered for world preview.";
        return false;
    }
    const shared_ptr<CEffectObject> pEffect =
        dynamic_pointer_cast<CEffectObject>(pGameObject);
    if (nullptr == pEffect)
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            iLevel, PREVIEW_LAYER, pGameObject);
        m_strPreviewStatus = "World preview clone returned the wrong type.";
        return false;
    }
    m_pWorldPreviewObject = pEffect;
    m_iWorldPreviewLevel = iLevel;
    return true;
}

bool_t Client::CEffect_Tool::Stage_WorldPreview()
{
	if (m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value())
		return Stage_WorldPreview(*m_SourcePreviewDocument, true);
    return m_ActiveDocument.has_value() ?
		Stage_WorldPreview(*m_ActiveDocument) : false;
}

bool_t Client::CEffect_Tool::Stage_WorldPreview(
    const EFFECT_DOCUMENT_DESC& Document)
{
	return Stage_WorldPreview(Document, false);
}

bool_t Client::CEffect_Tool::Stage_WorldPreview(
	const EFFECT_DOCUMENT_DESC& Document,
	const bool_t bAllowReadOnlySourceProjection)
{
    if (m_pAuthoringSequencer && m_pAuthoringSequencer->Is_Active()) return m_pAuthoringSequencer->Refresh_Effects();
	if (m_ValtanCombatObjectIndependentPreview.has_value())
	{
		m_strPreviewStatus =
			"Direct single-root EffectObject preview is suppressed while the Product combat-object lifecycle owns four world roots.";
		return false;
	}
	std::string FreshnessStatus;
	if (!Validate_ActiveRegistryBoundAuditionFreshness(FreshnessStatus))
	{
		/* Every preview entry point converges here, including the direct
		   Particle System and selected-Element audition buttons.  Once an
		   authoring-registry row or its pinned Product source changes, no
		   derivative of the previously opened audition document may stage. */
		Release_WorldPreview(true);
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus =
			"Preview staging rejected: registry-bound audition source freshness failed. " +
			FreshnessStatus;
		return false;
	}
	const EFFECT_DOCUMENT_DESC PreviewDocument =
		Build_PreviewDocument(Document);
	const bool_t bOwnerYawAttachment = Has_RequiredSourceFollowAttachments(PreviewDocument);
	if (bOwnerYawAttachment)
	{
		std::unordered_map<std::string, float4x4_t> PreflightAnchors;
		std::string PreflightError;
		if (!Resolve_ToolSourceAnchorWorlds(PreviewDocument,
				m_ValtanProductPreview.has_value() ? &m_ValtanProductPreview->Cue : nullptr,
				PreflightAnchors, PreflightError) ||
			(!m_SynchronizedAnimationClips.empty() &&
			 !Seek_WorldPreviewWithSourceAnchorHistory(nullptr, PreviewDocument,
				 Resolve_EffectSampleTime(m_fPreviewTimeSeconds), PreflightError, true)))
		{
			m_strPreviewStatus = "Hand attachment preflight failed; previous preview preserved: " + PreflightError;
			return false;
		}
	}
    if (!Ensure_WorldPreviewObject())
        return false;
    shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    std::string Error;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pCatalogProjection = CEffectCatalog::Find_VisualProjection(
			PreviewDocument.strEffectAssetId);
	const bool_t bReadOnlySourcePreview = bAllowReadOnlySourceProjection &&
		m_ProductPreview.has_value() &&
		m_SourcePreviewDocument.has_value() &&
		Document.strEffectAssetId ==
			m_SourcePreviewDocument->strEffectAssetId &&
		CEffectDocumentCodec::Serialize(Document) ==
			CEffectDocumentCodec::Serialize(*m_SourcePreviewDocument);
	if (bAllowReadOnlySourceProjection && !bReadOnlySourcePreview)
	{
		m_strPreviewStatus =
			"Read-only source preview identity changed before staging.";
		return false;
	}
	const bool_t bExactVisualProjection = nullptr != pCatalogProjection &&
		bReadOnlySourcePreview &&
		m_ePreviewFilter == EFFECT_PREVIEW_FILTER::COMPLETE &&
		CEffectDocumentCodec::Serialize(PreviewDocument) ==
			CEffectDocumentCodec::Serialize(pCatalogProjection->Get_Document());
	std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
		pVisualPrepared;
	const bool_t bVisualPrepared = !bExactVisualProjection ||
		CEffectDocumentRenderer::Prepare_VisualProgramDocument(
			m_pDevice, m_pContext, pCatalogProjection,
			pVisualPrepared, Error);
	bool_t bStaged = nullptr != pObject && bVisualPrepared &&
		(bExactVisualProjection ?
			pObject->Stage_PrevalidatedVisualProgramDocument(
				pCatalogProjection, pVisualPrepared, Error) :
			pObject->Stage_Document(PreviewDocument, Error));
	if (!bStaged)
    {
        m_strPreviewStatus = "Document is editable but not drawable yet: " + Error;
        return false;
    }
	// Follow/seek must consume exactly the filtered, draft-applied document
	// that was staged, rather than anchors belonging to excluded siblings.
	m_WorldPreviewDocument = PreviewDocument;
	m_bSkipNextWorldPreviewDelta = true;
	m_pVisualPreviewProjection = bExactVisualProjection ?
		pCatalogProjection : nullptr;
	m_bReconstructedDiagnosticActive = false;
	m_bReconstructedSourceRuntimeActive = false;
	Reset_ReconstructedSourceRuntimeTimeline();
	pObject->Set_Playing(false);
	const f32_t fEffectSampleSeconds =
		Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
	std::string HistoryError;
	const bool_t bHistorySampled =
		m_bValtanBossPatternTransformHistoryRequired ?
			Seek_ValtanBossPatternTransformHistory(
				pObject, fEffectSampleSeconds, HistoryError) :
			Seek_WorldPreviewWithSourceAnchorHistory(
				pObject, PreviewDocument, fEffectSampleSeconds, HistoryError);
	if (m_bValtanBossPatternTransformHistoryRequired && !bHistorySampled)
	{
		pObject->Set_Visible(false);
		m_strPreviewStatus =
			"Valtan 420633 exact source-anchor preview failed closed: " +
			HistoryError;
		return false;
	}
	if (!bHistorySampled)
		pObject->Set_SampleTime(fEffectSampleSeconds);
    switch (m_ePreviewFilter)
    {
    case EFFECT_PREVIEW_FILTER::COMPLETE:
        m_strPreviewStatus =
            "Complete Effect preview committed from the active Document.";
        break;
    case EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM:
        m_strPreviewStatus =
            "All Particle subtypes preview committed.";
        break;
	case EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_MESHES:
		m_strPreviewStatus =
			"Standalone Mesh-only preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS:
		m_strPreviewStatus =
			"Mesh Particle-only preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_SPRITES:
		m_strPreviewStatus =
			"Standalone Sprite-only preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_SPRITE_EMITTERS:
		m_strPreviewStatus =
			"Sprite Particle-only preview committed.";
		break;
    case EFFECT_PREVIEW_FILTER::SOLO_SELECTED:
        m_strPreviewStatus = "Selected Element Solo preview committed.";
        break;
	case EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUE:
		m_strPreviewStatus = "Selected Model / Summon Solo preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUES:
		m_strPreviewStatus = "Model / Summon Family preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_AUTHORING_FAMILY:
		m_strPreviewStatus = "Selected authoring Family preview committed.";
		break;
    case EFFECT_PREVIEW_FILTER::MUTE_SELECTED:
        m_strPreviewStatus = "Selected Element Mute preview committed.";
        break;
    case EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP:
        m_strPreviewStatus = "Selected Element Group Solo preview committed.";
        break;
    case EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP:
        m_strPreviewStatus = "Selected Element Group Mute preview committed.";
        break;
    case EFFECT_PREVIEW_FILTER::END:
	default:
		m_strPreviewStatus = "Effect preview committed.";
		break;
	}
	if (!HistoryError.empty())
	{
		m_strPreviewStatus +=
			" Source-anchor history used current-pose fallback: " + HistoryError;
	}
	return true;
}

bool Client::CEffect_Tool::Resolve_ElementsPreviewWindow(const EFFECT_DOCUMENT_DESC& document,
    const std::vector<std::string>& elementIds, uint32_t& startMs, uint32_t& endMs,
    std::string& label, std::string& error) const
{
    float startSeconds = 600.f, endSeconds = 0.f;
    for (const auto& element : document.Elements)
    {
        if (std::find(elementIds.begin(), elementIds.end(), element.strElementId) == elementIds.end() ||
            Is_EffectSimulationOnlyParticle(element)) continue;
        const float start = element.Detail.Timing.fStartDelaySeconds +
            (element.SourceRecipe.bEnabled ? element.SourceRecipe.fEmitterDelaySeconds : 0.f);
        const float end = Element_PreviewEndSeconds(element);
        if (!std::isfinite(start) || !std::isfinite(end) || start < 0.f || end <= start || end > 600.f)
        { error = "Selected elements need finite playback windows within 600 seconds."; return false; }
        startSeconds = (std::min)(startSeconds, start);
        endSeconds = (std::max)(endSeconds, end);
        label = element.strDisplayName;
    }
    if (endSeconds <= startSeconds)
    { error = "The selected group has no visible playback window."; return false; }
    if (elementIds.size() > 1u) label = std::to_string(elementIds.size()) + " selected elements";
    endMs = static_cast<uint32_t>(std::ceil(endSeconds * 1000.f));
    startMs = (std::min)(endMs - 1u, static_cast<uint32_t>(std::ceil(startSeconds * 1000.f)));
    return true;
}

bool Client::CEffect_Tool::Build_ElementsPreviewDocument(const EFFECT_DOCUMENT_DESC& document,
    const std::vector<std::string>& elementIds, EFFECT_DOCUMENT_DESC& preview, std::string& error) const
{
    if (elementIds.empty())
    { error = "Select at least one stable Element ID for preview."; return false; }
    if (!CEffectPlayback::Validate_SourceParticleProviders(document, error)) return false;
    std::set<std::string, std::less<>> included;
    bool hasDrawable = false;
    for (const auto& elementId : elementIds)
    {
        const auto selected = std::find_if(document.Elements.begin(), document.Elements.end(),
            [&elementId](const auto& element) { return element.strElementId == elementId; });
        if (elementId.empty() || selected == document.Elements.end())
        { error = "Selected Element preview rejected a missing stable Element ID: " + elementId; return false; }
        if (!Is_ElementPreviewAdmitted(*selected))
        { error = ElementPreviewAdmissionReason(*selected); return false; }
        hasDrawable = hasDrawable || !Is_EffectSimulationOnlyParticle(*selected);
        included.insert(elementId);
        for (const auto& module : selected->SourceRecipe.Modules)
        {
            if (module.strClassName != "particlemodulelocationemitter" &&
                module.strClassName != "efparticlemodulelocationemitter") continue;
            for (const auto& literal : module.Literals)
                if (literal.strPropertyPath == "runtime.providerelementid" && !literal.strString.empty())
                    included.insert(literal.strString);
        }
    }
    if (!hasDrawable)
    { error = "Source providers only simulate positions. Select a dependent drawable element for preview."; return false; }
    preview = document;
    std::erase_if(preview.Elements, [&included](const auto& element)
        { return !included.contains(element.strElementId); });
    std::erase_if(preview.ModelCues, [&preview](const auto& cue)
        { return std::none_of(preview.Elements.begin(), preview.Elements.end(),
            [&cue](const auto& element) { return element.ActionCueAttachment.strModelCueId == cue.strCueId; }); });
    // Dependencies keep their original order and clock; hidden model cues
    // supply anchors without adding unrelated summon pixels to the selection.
    for (auto& cue : preview.ModelCues) cue.bVisible = false;
    return true;
}

Client::EFFECT_DOCUMENT_DESC
Client::CEffect_Tool::Build_PreviewDocument(
	const EFFECT_DOCUMENT_DESC& Document) const
{
    EFFECT_DOCUMENT_DESC Preview = Document;
	const auto PreserveModelCueAnchors = [&Preview]()
	{
		std::erase_if(Preview.ModelCues,
			[&Preview](const EFFECT_MODEL_CUE_DESC& Cue)
			{
				return std::none_of(Preview.Elements.begin(), Preview.Elements.end(),
					[&Cue](const EFFECT_ELEMENT_DESC& Element)
					{ return Element.ActionCueAttachment.strModelCueId == Cue.strCueId; });
			});
		// Hidden model cues still supply their animated bones to child Elements.
		for (EFFECT_MODEL_CUE_DESC& Cue : Preview.ModelCues)
			Cue.bVisible = false;
	};
	if (!m_bPreviewScreenPostEnabled)
	{
		std::erase_if(Preview.Elements,
			[](const EFFECT_ELEMENT_DESC& Element)
			{
				return EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind;
			});
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUE == m_ePreviewFilter)
	{
		for (EFFECT_ELEMENT_DESC& Element : Preview.Elements)
			Element.bVisible = false;
		for (EFFECT_MODEL_CUE_DESC& Cue : Preview.ModelCues)
		{
			Cue.bVisible = Cue.bVisible &&
				Cue.strCueId == m_strPreviewIsolationModelCueId;
		}
		return Preview;
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUES == m_ePreviewFilter)
	{
		for (EFFECT_ELEMENT_DESC& Element : Preview.Elements)
			Element.bVisible = false;
		return Preview;
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_AUTHORING_FAMILY == m_ePreviewFilter)
	{
		std::erase_if(Preview.Elements,
			[this](const EFFECT_ELEMENT_DESC& Element)
			{
				return Resolve_AuthoringFamily(Element) !=
					m_ePreviewIsolationAuthoringFamily;
			});
		PreserveModelCueAnchors();
		return Preview;
	}
    if (EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM == m_ePreviewFilter)
    {
        std::erase_if(Preview.Elements,
            [](const EFFECT_ELEMENT_DESC& Element)
            {
                return EFFECT_ELEMENT_KIND::PARTICLE != Element.eKind;
            });
        PreserveModelCueAnchors();
        return Preview;
    }
	if (EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_MESHES == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_SPRITES == m_ePreviewFilter)
	{
		const EFFECT_ELEMENT_KIND eRequired =
			EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_MESHES == m_ePreviewFilter ?
				EFFECT_ELEMENT_KIND::MESH : EFFECT_ELEMENT_KIND::SPRITE;
		std::erase_if(Preview.Elements,
			[eRequired](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.eKind != eRequired;
			});
		PreserveModelCueAnchors();
		return Preview;
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::SOLO_SPRITE_EMITTERS == m_ePreviewFilter)
	{
		const CASCADE_RENDERER_KIND eRequired =
			EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS == m_ePreviewFilter ?
				CASCADE_RENDERER_KIND::MESH :
				CASCADE_RENDERER_KIND::SPRITE;
		std::erase_if(Preview.Elements,
			[eRequired](const EFFECT_ELEMENT_DESC& Element)
			{
				return EFFECT_ELEMENT_KIND::PARTICLE != Element.eKind ||
					Resolve_CascadeRendererKind(Element) != eRequired;
			});
		PreserveModelCueAnchors();
		return Preview;
	}
    if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter ||
        EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == m_ePreviewFilter)
    {
		if (m_strPreviewIsolationGroupId.empty())
            return Preview;
        const bool_t bGroupExists = std::any_of(
            Preview.Elements.begin(), Preview.Elements.end(),
			[this](const EFFECT_ELEMENT_DESC& Element)
            {
				return Element.strGroupId == m_strPreviewIsolationGroupId;
            });
        if (!bGroupExists)
            return Preview;
        std::erase_if(Preview.Elements,
            [this](const EFFECT_ELEMENT_DESC& Element)
            {
				const bool_t bSelectedGroup =
					Element.strGroupId == m_strPreviewIsolationGroupId;
                return EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP ==
                    m_ePreviewFilter ? !bSelectedGroup : bSelectedGroup;
            });
        if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter)
            PreserveModelCueAnchors();
        return Preview;
    }
    if (EFFECT_PREVIEW_FILTER::COMPLETE == m_ePreviewFilter ||
		m_strPreviewIsolationElementId.empty())
        return Preview;
    const bool_t bSelectionExists = std::any_of(
        Preview.Elements.begin(), Preview.Elements.end(),
		[this](const EFFECT_ELEMENT_DESC& Element)
        {
			return Element.strElementId == m_strPreviewIsolationElementId;
        });
    if (!bSelectionExists)
        return Preview;
    std::erase_if(Preview.Elements,
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
			const bool_t bSelected =
				Element.strElementId == m_strPreviewIsolationElementId;
            return EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ?
                !bSelected : bSelected;
        });
    if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter)
        PreserveModelCueAnchors();
    return Preview;
}

bool_t Client::CEffect_Tool::Stage_ParticleSystemDraftPreview()
{
    if (!m_ActiveDocument.has_value() ||
        !m_ParticleSystemDraft.has_value())
    {
        return false;
    }

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    if (!Apply_ParticleSystemDraft(Staged))
    {
        m_strPreviewStatus =
            "Live Particle System preview rejected: draft is missing.";
        return false;
    }
    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    Recalculate_PreviewDuration(Staged);
    if (!Stage_WorldPreview(Staged))
    {
        m_fPreviewDurationSeconds = fPreviousDuration;
        m_fPreviewTimeSeconds = fPreviousTime;
        return false;
    }
    m_strPreviewStatus =
        "Live Particle System draft staged; Apply commits it to the active Document.";
    return true;
}

bool_t Client::CEffect_Tool::Stage_DetailDraftPreview()
{
    if (!m_ActiveDocument.has_value() || !m_DetailDraft.has_value())
        return false;

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    if (!Apply_DetailDraft(Staged))
    {
        m_strPreviewStatus =
            "Live Detail preview rejected: selected Element is missing.";
        return false;
    }

    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    Recalculate_PreviewDuration(Staged);
    if (!Stage_WorldPreview(Staged))
    {
        m_fPreviewDurationSeconds = fPreviousDuration;
        m_fPreviewTimeSeconds = fPreviousTime;
        return false;
    }
    m_strPreviewStatus =
        "Live Detail draft staged; Apply Detail commits it to the active Document.";
	if (m_bDetailDraftPreviewRestartRequested)
	{
		m_bDetailDraftPreviewRestartRequested = false;
        // Refresh_Effects already rebuilt and sampled the selected element at
        // the Sequencer cursor. Keep that clock while editing its curves.
        if (!m_pAuthoringSequencer || !m_pAuthoringSequencer->Is_Active() ||
            !m_pAuthoringSequencer->Is_ElementPreview())
            Start_WorldPreviewFromBeginning();
	}
    return true;
}

bool_t Client::CEffect_Tool::Stage_ModelCueDraftPreview()
{
	if (!m_ActiveDocument.has_value() || !m_ModelCueDraft.has_value())
		return false;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	if (!Apply_ModelCueDraft(Staged))
	{
		m_strPreviewStatus =
			"Live Model Cue preview rejected: selected Cue is missing.";
		return false;
	}
	const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
	const f32_t fPreviousTime = m_fPreviewTimeSeconds;
	Recalculate_PreviewDuration(Staged);
	if (!Stage_WorldPreview(Staged))
	{
		m_fPreviewDurationSeconds = fPreviousDuration;
		m_fPreviewTimeSeconds = fPreviousTime;
		return false;
	}
	m_strPreviewStatus =
		"Live Model Cue draft staged; Apply commits it to the active Document.";
	return true;
}

bool_t Client::CEffect_Tool::Apply_ParticleSystemDraft(
    EFFECT_DOCUMENT_DESC& Document) const
{
    if (!m_ParticleSystemDraft.has_value())
        return false;
    Document.ParticleSystem = *m_ParticleSystemDraft;
    return true;
}

bool_t Client::CEffect_Tool::Apply_DetailDraft(
    EFFECT_DOCUMENT_DESC& Document) const
{
    if (!m_DetailDraft.has_value())
        return false;
    for (EFFECT_ELEMENT_DESC& Element : Document.Elements)
    {
        if (Element.strElementId != m_strDetailDraftElementId)
            continue;
        Apply_EffectElementDetailDraft(Element, *m_DetailDraft);
		if (m_bDetailDraftCapabilityDeferred ||
			!Is_EffectElementAuthoringExecutionTarget(Element))
			Element.bVisible = false;
        return true;
    }
    return false;
}

bool_t Client::CEffect_Tool::Apply_ModelCueDraft(
	EFFECT_DOCUMENT_DESC& Document) const
{
	if (!m_ModelCueDraft.has_value())
		return false;
	for (EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
	{
		if (Cue.strCueId != m_ModelCueDraft->strCueId)
			continue;
		Cue = *m_ModelCueDraft;
		return true;
	}
	return false;
}

bool_t Client::CEffect_Tool::Resolve_PreviewRoot(float4x4_t& OutRoot)
{
	const bool_t bPlayerSnapshot = m_ProductPreview.has_value() &&
		EFFECT_FOLLOW_POLICY::SNAPSHOT ==
			m_ProductPreview->ProductCue.Cue.eFollowPolicy;
	const bool_t bValtanSnapshot = m_ValtanProductPreview.has_value() &&
		EFFECT_FOLLOW_POLICY::SNAPSHOT ==
			m_ValtanProductPreview->Cue.eFollowPolicy;
	const bool_t bPlayerActionFacing = m_ProductPreview.has_value() &&
		EFFECT_ORIENTATION_POLICY::ACTION_FACING ==
			m_ProductPreview->ProductCue.Cue.eOrientationPolicy;
    if ((bPlayerSnapshot || bValtanSnapshot) &&
		m_bProductCueSnapshotCaptured)
    {
        OutRoot = m_ProductCueSnapshotRoot;
        return true;
    }

    float4x4_t Anchor{};
    bool_t bResolved = false;
    if (m_ProductPreview.has_value())
    {
        const std::string& strCueAnchor =
            m_ProductPreview->ProductCue.Cue.strAnchorSlotId;
        bResolved = "root" == strCueAnchor ?
            CAnimationTargetService::Resolve_RootTransform(&Anchor) :
            CAnimationTargetService::Resolve_AnchorTransform(
                strCueAnchor.c_str(), &Anchor);
    }
	else if (m_ValtanProductPreview.has_value())
	{
		const std::string& strCueAnchor =
			m_ValtanProductPreview->Cue.strAnchorSlotId;
		bResolved = "root" == strCueAnchor ?
			CAnimationTargetService::Resolve_RootTransform(&Anchor) :
			CAnimationTargetService::Resolve_AnchorTransform(
				strCueAnchor.c_str(), &Anchor);
	}
    else
    {
        switch (m_ePreviewPivotKind)
        {
        case EFFECT_PREVIEW_PIVOT_KIND::WORLD:
            Anchor = m_PreviewWorldRoot;
            bResolved = true;
            break;
        case EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT:
            bResolved = CAnimationTargetService::Resolve_RootTransform(&Anchor);
            break;
        case EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET:
        case EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE:
            bResolved = CAnimationTargetService::Resolve_AnchorTransform(
                m_strPreviewAnchorSlotId.c_str(), &Anchor);
            break;
        case EFFECT_PREVIEW_PIVOT_KIND::END:
        default:
            return false;
        }
    }
    if (!bResolved)
        return false;
	if (!Has_ProductCuePreview())
    {
        OutRoot = Anchor;
        return true;
    }

	if (m_ProductPreview.has_value())
	{
		const ANIMATION_EFFECT_CUE& Cue =
			m_ProductPreview->ProductCue.Cue;
		const bool_t bCueVisible =
			Is_ProductCueVisible(m_fPreviewTimeSeconds);
		if (bPlayerActionFacing && "root" != Cue.strAnchorSlotId)
			return false;
		f32_t fActionFacingYawDegrees =
			m_fProductCueActionFacingYawDegrees;
		if (bPlayerActionFacing && !m_bProductCueActionFacingCaptured)
		{
			if (!Try_ExtractPlanarYawDegrees(
				Anchor, fActionFacingYawDegrees))
			{
				return false;
			}
			/* A later combo occurrence must lock its action-facing yaw from the
			   pose at that occurrence, not from the hidden stage-one frame. */
			if (bCueVisible)
			{
				m_fProductCueActionFacingYawDegrees =
					fActionFacingYawDegrees;
				m_bProductCueActionFacingCaptured = true;
			}
		}
		if (!CAnimationEffectCueDocument::Try_ComposeRootTransform(
			Cue.LocalTransform, Anchor, Cue.eOrientationPolicy,
			fActionFacingYawDegrees, OutRoot))
		{
			return false;
		}
	}
	else
	{
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue =
			m_ValtanProductPreview->Cue;
		if (!CEffectPresentationService::Build_CueScalePolicyRoot(
				Cue.LocalTransform, Cue.eScalePolicy, Cue.vWorldScale,
				Anchor, OutRoot))
		{
			return false;
		}
	}
	if ((bPlayerSnapshot || bValtanSnapshot) &&
		Is_ProductCueVisible(m_fPreviewTimeSeconds))
    {
        m_ProductCueSnapshotRoot = OutRoot;
        m_bProductCueSnapshotCaptured = true;
    }
    return true;
}

bool_t Client::CEffect_Tool::Has_ProductCuePreview() const
{
	return m_ProductPreview.has_value() ||
		m_ValtanProductPreview.has_value();
}

f32_t Client::CEffect_Tool::Resolve_EffectSampleTime(
    const f32_t fTimelineSeconds) const
{
	if (m_ProductPreview.has_value())
	{
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
			m_ProductPreview->ProductCue;
		f32_t fProductTimelineSeconds = (std::max)(0.f, fTimelineSeconds);
		if (m_bBufferedComboAuditionActive &&
			m_eBufferedComboAuditionClass ==
				m_ProductPreview->eCharacterClass &&
			m_iBufferedComboAuditionSkillId == m_ProductPreview->iSkillId)
		{
			fProductTimelineSeconds = (std::max)(0.f,
				fProductTimelineSeconds -
					m_fBufferedComboAuditionOccurrenceOffsetSeconds);
		}
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Clip.iSourceStartMs) * 0.001f;
		Timing.fPlayRate = ProductCue.Clip.fPlayRate;
		Timing.fCueSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Cue.iStartMs) * 0.001f;
		Timing.fCueSourceEndSeconds = static_cast<f32_t>(
			ProductCue.Cue.iEndMs) * 0.001f;
		Timing.bHasCueSourceEnd = EFFECT_STOP_POLICY::CUE_END ==
			ProductCue.Cue.eStopPolicy;
		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
		return CActionPresentationTimeline::Resolve_CuePreviewSample(
			Timing, fProductTimelineSeconds, Sample) ?
			Sample.fEffectSampleSeconds : 0.f;
	}
	if (!m_ValtanProductPreview.has_value())
	{
		return (std::max)(0.f, fTimelineSeconds -
			static_cast<f32_t>(m_iValtanReferenceEffectStartMs) * 0.001f);
	}
	const f32_t fClipTimelineOffsetSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->iOwningClipTimelineOffsetMs) * 0.001f;
	if (fTimelineSeconds < fClipTimelineOffsetSeconds)
		return 0.f;

	ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
	Timing.fClipSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Clip.iSourceStartMs) * 0.001f;
	Timing.fPlayRate = m_ValtanProductPreview->Clip.fPlayRate;
	Timing.fCueSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceStartMs) * 0.001f;
	Timing.fCueSourceEndSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceEndMs) * 0.001f;
	Timing.bHasCueSourceEnd =
		m_ValtanProductPreview->Cue.bHasSourceEnd;
	ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
	return CActionPresentationTimeline::Resolve_CuePreviewSample(
		Timing, (std::max)(0.f,
			fTimelineSeconds - fClipTimelineOffsetSeconds), Sample) ?
		Sample.fEffectSampleSeconds : 0.f;
}

f32_t Client::CEffect_Tool::Resolve_EffectTimelineTime(
	const f32_t fEffectSampleSeconds) const
{
	const f32_t fClampedEffectSample =
		(std::max)(0.f, fEffectSampleSeconds);
	if (m_ProductPreview.has_value())
	{
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
			m_ProductPreview->ProductCue;
		const bool_t bBufferedOccurrence =
			m_bBufferedComboAuditionActive &&
			m_eBufferedComboAuditionClass ==
				m_ProductPreview->eCharacterClass &&
			m_iBufferedComboAuditionSkillId == m_ProductPreview->iSkillId;
		const f32_t fOccurrenceOffsetSeconds = bBufferedOccurrence ?
			m_fBufferedComboAuditionOccurrenceOffsetSeconds : 0.f;
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Clip.iSourceStartMs) * 0.001f;
		Timing.fPlayRate = ProductCue.Clip.fPlayRate;
		Timing.fCueSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Cue.iStartMs) * 0.001f;
		Timing.fCueSourceEndSeconds = static_cast<f32_t>(
			ProductCue.Cue.iEndMs) * 0.001f;
		Timing.bHasCueSourceEnd = EFFECT_STOP_POLICY::CUE_END ==
			ProductCue.Cue.eStopPolicy;
		f32_t fTimelineSeconds = 0.f;
		return CActionPresentationTimeline::Resolve_CuePreviewTimelineTime(
			Timing, fOccurrenceOffsetSeconds, fClampedEffectSample,
			fTimelineSeconds) ? fTimelineSeconds : fClampedEffectSample;
	}
	if (!m_ValtanProductPreview.has_value())
	{
		return static_cast<f32_t>(m_iValtanReferenceEffectStartMs) * 0.001f +
			fClampedEffectSample;
	}
	const f32_t fClipTimelineOffsetSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->iOwningClipTimelineOffsetMs) * 0.001f;

	ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
	Timing.fClipSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Clip.iSourceStartMs) * 0.001f;
	Timing.fPlayRate = m_ValtanProductPreview->Clip.fPlayRate;
	Timing.fCueSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceStartMs) * 0.001f;
	Timing.fCueSourceEndSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceEndMs) * 0.001f;
	Timing.bHasCueSourceEnd =
		m_ValtanProductPreview->Cue.bHasSourceEnd;
	f32_t fTimelineSeconds = 0.f;
	return CActionPresentationTimeline::Resolve_CuePreviewTimelineTime(
		Timing, fClipTimelineOffsetSeconds, fClampedEffectSample,
		fTimelineSeconds) ? fTimelineSeconds : fClampedEffectSample;
}

bool_t Client::CEffect_Tool::Seek_WorldPreviewWithSourceAnchorHistory(
	const std::shared_ptr<CEffectObject>& pObject,
	const EFFECT_DOCUMENT_DESC& Document,
	const f32_t fEffectSampleSeconds,
	std::string& strOutError,
	const bool_t bValidateOnly)
{
	strOutError.clear();
	if ((!bValidateOnly && nullptr == pObject) || !std::isfinite(fEffectSampleSeconds) ||
		fEffectSampleSeconds < 0.f)
	{
		strOutError = "Effect history seek received an invalid sample request.";
		return false;
	}
	const bool_t bStableRootPreview = m_ProductPreview.has_value() ?
		"root" == m_ProductPreview->ProductCue.Cue.strAnchorSlotId :
		m_ValtanProductPreview.has_value() ?
			"root" == m_ValtanProductPreview->Cue.strAnchorSlotId :
			(EFFECT_PREVIEW_PIVOT_KIND::WORLD == m_ePreviewPivotKind ||
			 EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind);
	if (!bStableRootPreview)
	{
		strOutError =
			"Source-anchor history currently requires a stable root Effect pivot.";
		return false;
	}

	float4x4_t EffectRoot{};
	if (!Resolve_PreviewRoot(EffectRoot))
	{
		strOutError = "Effect history seek could not resolve its preview root.";
		return false;
	}
	const std::vector<TOOL_SOURCE_ANCHOR_REQUEST> Requests =
		Collect_ToolSourceAnchorRequests(Document);
	if (Requests.empty())
	{
		if (bValidateOnly)
			return true;
		pObject->Set_SourceAnchorWorlds({});
		pObject->Set_RootWorld(EffectRoot);
		pObject->Set_SampleTime(fEffectSampleSeconds);
		return true;
	}

	const bool_t bCameraOnly = std::all_of(Requests.begin(), Requests.end(),
		[](const TOOL_SOURCE_ANCHOR_REQUEST& Request)
		{ return Request.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW; });
	if (bCameraOnly)
	{
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER CameraProvider =
			[&Requests, EffectRoot](const f32_t, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& Sample, std::string& Error)
			{
				Sample.RootWorld = EffectRoot;
				Sample.SourceAnchorWorlds.clear();
				for (const auto& Request : Requests)
				{
					float4x4_t World{};
					if (!Try_ResolveToolCameraAnchorWorld(Request.SocketLocalTransform, World))
					{ Error = "camera_view history requires the actual scene view."; return false; }
					Sample.SourceAnchorWorlds.emplace(Request.strRuntimeAnchorSlotId, World);
				}
				Error.clear();
				return true;
			};
		if (bValidateOnly)
		{
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Sample;
			return CameraProvider(fEffectSampleSeconds, Sample, strOutError);
		}
		return pObject->Set_SampleTimeWithTransformHistory(fEffectSampleSeconds, CameraProvider, strOutError);
	}
	float4x4_t ActualOwnerWorld{};
	const bool_t bNeedsOwnerYaw = std::any_of(Requests.begin(), Requests.end(),
		[](const TOOL_SOURCE_ANCHOR_REQUEST& Request)
		{ return Request.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW; });
	if (bNeedsOwnerYaw && !Try_ResolveToolAttachmentOwnerWorld(ActualOwnerWorld))
	{
		strOutError = "owner_yaw history requires the actual preview owner transform.";
		return false;
	}

	if (m_SynchronizedAnimationClips.empty() ||
		0u == m_iSynchronizedAnimationTargetGeneration ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration())
	{
		strOutError = "Source-anchor history requires a synchronized animation timeline.";
		return false;
	}
	const auto pModel = CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		strOutError = "Source-anchor history has no animation model.";
		return false;
	}
	std::vector<std::string> BoneNames;
	for (const TOOL_SOURCE_ANCHOR_REQUEST& Request : Requests)
		if (Request.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
			BoneNames.push_back(Request.strRuntimeBoneName);
	std::vector<CAnimationHistoricalPoseBinding> PoseBindings;
	std::vector<ACTION_PRESENTATION_CLIP_TIMING> ClipTimings;
	std::vector<float> WallBudgets;
	const bool_t bSingleClip = m_SynchronizedAnimationClips.size() == 1u;
	for (const SYNCHRONIZED_ANIMATION_CLIP& Clip : m_SynchronizedAnimationClips)
	{
		CAnimationHistoricalPoseBinding Binding;
		bool_t bPrepared = false;
		if (bSingleClip)
		{
			const uint32_t iAnimationIndex = pModel->Get_CurrentAnimIndex();
			const char* pCurrentClip = pModel->Get_AnimationName(iAnimationIndex);
			bPrepared = nullptr != pCurrentClip && Clip.strClipName == pCurrentClip &&
				CAnimationTargetService::Prepare_HistoricalPoseBinding(
					m_iSynchronizedAnimationTargetGeneration, iAnimationIndex,
					BoneNames, Binding);
		}
		else
		{
			bPrepared = CAnimationTargetService::Prepare_HistoricalClipPoseBinding(
				m_iSynchronizedAnimationTargetGeneration, Clip.strClipName,
				BoneNames, Binding);
		}
		if (!bPrepared || Binding.Get_BoneCount() != BoneNames.size())
		{
			strOutError = "Source-anchor historical clip binding failed: " + Clip.strClipName;
			return false;
		}
		ACTION_PRESENTATION_CLIP_TIMING Timing;
		Timing.fModelSourceDurationSeconds = Binding.Get_DurationSeconds();
		Timing.iPlayMs = Clip.iPlayMs;
		Timing.fPlayRate = Clip.fPlayRate;
		Timing.bLoop = Clip.bHasExplicitLoopPolicy && Clip.bLoop;
		Timing.fSourceStartSeconds = static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
		float SourceDuration = 0.f, WallDuration = 0.f;
		if (!std::isfinite(Clip.fHoldAfterSeconds) || Clip.fHoldAfterSeconds < 0.f ||
			!CActionPresentationTimeline::Resolve_ClipDuration(Timing, SourceDuration, WallDuration))
		{
			strOutError = "Source-anchor history has an invalid clip source/wall segment: " + Clip.strClipName;
			return false;
		}
		PoseBindings.push_back(std::move(Binding));
		ClipTimings.push_back(Timing);
		WallBudgets.push_back(0u == Clip.iAuthoringWallMs ?
			WallDuration + Clip.fHoldAfterSeconds : static_cast<f32_t>(Clip.iAuthoringWallMs) * 0.001f);
	}
	const bool_t bValtanScalePolicy = m_ValtanProductPreview.has_value();
	const VALTAN_PATTERN_EFFECT_SCALE_POLICY eValtanScalePolicy =
		bValtanScalePolicy ? m_ValtanProductPreview->Cue.eScalePolicy :
			VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
	const float3_t vValtanWorldScale = bValtanScalePolicy ?
		m_ValtanProductPreview->Cue.vWorldScale : float3_t{ 1.f, 1.f, 1.f };

	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
		[this, &PoseBindings, &ClipTimings, &WallBudgets, &Requests, &BoneNames, EffectRoot,
		 bSingleClip, bNeedsOwnerYaw, bValtanScalePolicy, eValtanScalePolicy,
		 vValtanWorldScale, ActualOwnerWorld](const f32_t fHistoryEffectSeconds,
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& strProviderError)
		{
			const f32_t fTimelineSeconds = Resolve_EffectTimelineTime(fHistoryEffectSeconds);
			ACTION_PRESENTATION_SAMPLE AnimationSample;
			const bool_t bMapped = bSingleClip && !bNeedsOwnerYaw ?
				CActionPresentationTimeline::Resolve_Sample(
					ClipTimings, fTimelineSeconds, AnimationSample) :
				CActionPresentationTimeline::Resolve_PreviewSequenceSample(
					ClipTimings, WallBudgets, fTimelineSeconds, AnimationSample);
			if (!bMapped || AnimationSample.iClipIndex >= PoseBindings.size())
			{
				strProviderError = "Source-anchor history could not resolve the authored clip timeline.";
				return false;
			}

			ANIMATION_HISTORICAL_POSE_SAMPLE PoseSample;
			if (!CAnimationTargetService::Sample_HistoricalPose(
					PoseBindings[AnimationSample.iClipIndex],
					AnimationSample.fClipSourceTimeSeconds, PoseSample) ||
				PoseSample.BoneCombinedMatrices.size() != BoneNames.size())
			{
				strProviderError =
					"Source-anchor historical bone sampling failed.";
				return false;
			}
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Staged;
			Staged.RootWorld = EffectRoot;
			Staged.SourceAnchorWorlds.reserve(Requests.size());
			size_t iBone = 0u;
			for (size_t iRequest = 0u; iRequest < Requests.size(); ++iRequest)
			{
				const TOOL_SOURCE_ANCHOR_REQUEST& Request = Requests[iRequest];
				if (Request.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
				{
					float4x4_t World{};
					if (!Try_ResolveToolCameraAnchorWorld(Request.SocketLocalTransform, World))
					{ strProviderError = "camera_view history requires the actual scene view."; return false; }
					Staged.SourceAnchorWorlds.emplace(Request.strRuntimeAnchorSlotId, World);
					continue;
				}
				const size_t iBoneSample = iBone++;
				float4x4_t BoneWorld{};
				if (bValtanScalePolicy ||
					Request.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW)
				{
					if (!Build_ToolValtanSourceAnchorWorld(
							PoseSample.BoneCombinedMatrices[iBoneSample],
							PoseSample.RootWorld, eValtanScalePolicy,
							vValtanWorldScale, ActualOwnerWorld, Request.eOrientation,
							BoneWorld))
					{
						strProviderError =
							"Source-anchor history could not normalize the Valtan source bone with its cue scale policy.";
						return false;
					}
				}
				else if (Request.bNormalizeSourceImportScale)
				{
					EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
					AnchorBuild.RawBone = PoseSample.BoneCombinedMatrices[iBoneSample];
					AnchorBuild.OwnerWorld = PoseSample.RootWorld;
					if (!CEffectPresentationService::Build_SourceBoneAnchorWorld(AnchorBuild, BoneWorld))
					{
						strProviderError = "Source-anchor history does not match its admitted import scale.";
						return false;
					}
				}
				else
				{
					XMStoreFloat4x4(&BoneWorld,
						XMLoadFloat4x4(
							&PoseSample.BoneCombinedMatrices[iBoneSample]) *
						XMLoadFloat4x4(&PoseSample.RootWorld));
				}
				const EFFECT_TRANSFORM_DESC& Local =
					Request.SocketLocalTransform;
				const matrix_t SocketLocal = XMMatrixScaling(
					Local.vScale.x, Local.vScale.y, Local.vScale.z) *
					XMMatrixRotationRollPitchYaw(
						XMConvertToRadians(Local.vRotationDegrees.x),
						XMConvertToRadians(Local.vRotationDegrees.y),
						XMConvertToRadians(Local.vRotationDegrees.z)) *
					XMMatrixTranslation(
						Local.vPosition.x, Local.vPosition.y, Local.vPosition.z);
				float4x4_t AnchorWorld{};
				XMStoreFloat4x4(&AnchorWorld,
					SocketLocal * XMLoadFloat4x4(&BoneWorld));
				Staged.SourceAnchorWorlds.emplace(
					Request.strRuntimeAnchorSlotId, AnchorWorld);
			}
			if (Staged.SourceAnchorWorlds.size() != Requests.size())
			{
				strProviderError =
					"Source-anchor history produced duplicate runtime slots.";
				return false;
			}
			OutSample = std::move(Staged);
			strProviderError.clear();
			return true;
		};
	if (bValidateOnly)
	{
		// Preflight every sample before Stage_Document replaces the existing preview.
		// Use the same exact fixed-step clock as CEffectPlayback history evaluation.
		const uint64_t iSteps = static_cast<uint64_t>(std::floor(
			static_cast<f64_t>(fEffectSampleSeconds) * 60.0 + 1.0e-9));
		for (uint64_t iStep = 0u; iStep <= iSteps; ++iStep)
		{
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Sample;
			if (!TransformProvider(static_cast<f32_t>(
					static_cast<f64_t>(iStep) / 60.0), Sample, strOutError))
				return false;
		}
		EFFECT_FIXED_STEP_TRANSFORM_SAMPLE FinalSample;
		return TransformProvider(fEffectSampleSeconds, FinalSample, strOutError);
	}
	return pObject->Set_SampleTimeWithTransformHistory(
		fEffectSampleSeconds, TransformProvider, strOutError);
}

bool_t Client::CEffect_Tool::Is_ProductCueVisible(
    const f32_t fTimelineSeconds) const
{
	if (m_ProductPreview.has_value())
	{
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
			m_ProductPreview->ProductCue;
		f32_t fProductTimelineSeconds = (std::max)(0.f, fTimelineSeconds);
		if (m_bBufferedComboAuditionActive &&
			m_eBufferedComboAuditionClass ==
				m_ProductPreview->eCharacterClass &&
			m_iBufferedComboAuditionSkillId == m_ProductPreview->iSkillId)
		{
			if (fProductTimelineSeconds + 0.0001f <
				m_fBufferedComboAuditionOccurrenceOffsetSeconds)
			{
				return false;
			}
			fProductTimelineSeconds = (std::max)(0.f,
				fProductTimelineSeconds -
					m_fBufferedComboAuditionOccurrenceOffsetSeconds);
		}
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Clip.iSourceStartMs) * 0.001f;
		Timing.fPlayRate = ProductCue.Clip.fPlayRate;
		Timing.fCueSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Cue.iStartMs) * 0.001f;
		Timing.fCueSourceEndSeconds = static_cast<f32_t>(
			ProductCue.Cue.iEndMs) * 0.001f;
		Timing.bHasCueSourceEnd = EFFECT_STOP_POLICY::CUE_END ==
			ProductCue.Cue.eStopPolicy;
		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
		return CActionPresentationTimeline::Resolve_CuePreviewSample(
			Timing, fProductTimelineSeconds, Sample) &&
			Sample.bVisible;
	}
	if (!m_ValtanProductPreview.has_value())
	{
		return fTimelineSeconds * 1000.f + 0.5f >=
			static_cast<f32_t>(m_iValtanReferenceEffectStartMs);
	}
	const f32_t fTimelineMs = fTimelineSeconds * 1000.f;
	const f32_t fStageStartMs = static_cast<f32_t>(
		m_ValtanProductPreview->iOwningStageTimelineOffsetMs);
	const f32_t fClipStartMs = static_cast<f32_t>(
		m_ValtanProductPreview->iOwningClipTimelineOffsetMs);
	if (0u == m_ValtanProductPreview->Cue.iStageDurationMs ||
		fTimelineMs + 0.5f < fStageStartMs ||
		fTimelineMs + 0.5f < fClipStartMs)
	{
		return false;
	}
	/* Product runtime retains NATURAL boss-action Effects when the Server
	   advances to the next stage.  Do not clamp those previews to their owner
	   stage: the Effect document's own lifetime may intentionally cover a
	   following semantic stage (the unified INNER -> OUTER donut does).  A
	   CUE_END source window remains bounded by Resolve_CuePreviewSample below. */

	ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
	Timing.fClipSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Clip.iSourceStartMs) * 0.001f;
	Timing.fPlayRate = m_ValtanProductPreview->Clip.fPlayRate;
	Timing.fCueSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceStartMs) * 0.001f;
	Timing.fCueSourceEndSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceEndMs) * 0.001f;
	Timing.bHasCueSourceEnd =
		m_ValtanProductPreview->Cue.bHasSourceEnd;
	ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
	return CActionPresentationTimeline::Resolve_CuePreviewSample(
		Timing, (std::max)(0.f,
			fTimelineSeconds - fClipStartMs * 0.001f), Sample) &&
		Sample.bVisible;
}

bool_t Client::CEffect_Tool::Restore_ValtanProductPreviewPlayback(
	const optional<VALTAN_PRODUCT_PREVIEW>& Preview,
	const f32_t fTimelineSeconds,
	const f32_t fDurationSeconds,
	const bool_t bPlaying,
	const bool_t bVisibleRequested,
	const float4x4_t& SnapshotRoot,
	const bool_t bSnapshotCaptured,
	std::string& strOutError)
{
	strOutError.clear();
	if (!Preview.has_value())
		return true;
	const EFFECT_DOCUMENT_DESC* pRestoreDocument = nullptr;
	if (m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId == Preview->Cue.strEffectAssetId)
	{
		pRestoreDocument = &*m_ActiveDocument;
	}
	else if (m_SourcePreviewDocument.has_value() &&
		m_SourcePreviewDocument->strEffectAssetId ==
			Preview->Cue.strEffectAssetId)
	{
		pRestoreDocument = &*m_SourcePreviewDocument;
	}
	if (nullptr == pRestoreDocument)
	{
		strOutError =
			"the exact Valtan Effect document is no longer active";
		return false;
	}

	const auto FailRestore = [this, &strOutError](std::string Reason)
	{
		const shared_ptr<CEffectObject> pObject =
			m_pWorldPreviewObject.lock();
		if (nullptr != pObject)
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
		}
		Set_SynchronizedAnimationPaused(true);
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		strOutError = std::move(Reason);
		m_strPreviewStatus =
			"Exact Valtan Product preview restore failed: " + strOutError;
		return false;
	};

	m_ProductPreview.reset();
	m_ValtanProductPreview = Preview;
	m_fPreviewDurationSeconds = (std::max)(0.f, fDurationSeconds);
	m_fPreviewTimeSeconds = (std::clamp)(
		fTimelineSeconds, 0.f, m_fPreviewDurationSeconds);
	m_bPreviewPlaying = bPlaying;
	m_bPreviewVisibleRequested = bVisibleRequested;
	m_ProductCueSnapshotRoot = SnapshotRoot;
	m_bProductCueSnapshotCaptured = bSnapshotCaptured;
	m_fProductCueActionFacingYawDegrees = 0.f;
	m_bProductCueActionFacingCaptured = false;
	/* Rebuild document-owned boss state first. This restores the exact 420633
	   transform-history preparation when the selected document owns it. The v2
	   occurrence then replaces the legacy clip with its exact source segment. */
	Synchronize_LoadedSkillPreview();
	if (!Play_ValtanStageSequence(Preview->TimelineClips))
	{
		return FailRestore(
			"the exact full animation timeline could not be staged");
	}
	Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
	if (m_bPreviewPlaying)
		Update_SynchronizedAnimationSequence();
	Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);

	if (!Stage_WorldPreview(*pRestoreDocument))
	{
		return FailRestore(
			"the exact Effect document could not be staged: " +
			m_strPreviewStatus);
	}
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject)
		return FailRestore("the restored EffectObject is unavailable");

	const f32_t fEffectSampleSeconds =
		Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
	if (m_bValtanBossPatternTransformHistoryRequired)
	{
		if (!m_bValtanBossPatternTransformHistoryActive)
		{
			return FailRestore(
				"the exact Valtan transform history is unavailable");
		}
		std::string TransformError;
		if (!Seek_ValtanBossPatternTransformHistory(
				pObject, fEffectSampleSeconds, TransformError))
		{
			return FailRestore(
				"the exact Valtan transform-history sample failed: " +
				TransformError);
		}
	}
	else
	{
		std::string HistoryError;
		if (!Seek_WorldPreviewWithSourceAnchorHistory(
				pObject, *pRestoreDocument,
				fEffectSampleSeconds, HistoryError))
		{
			return FailRestore(
				"the source-anchor history could not be restored: " +
				HistoryError);
		}
	}

	/* Effect Tool owns this wall clock, so the EffectObject remains autonomous
	   playback-off while Tool and animation play/pause state are restored. */
	pObject->Set_Playing(false);
	pObject->Set_Visible(m_bPreviewVisibleRequested &&
		Is_ProductCueVisible(m_fPreviewTimeSeconds));
	Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
	return true;
}

void Client::CEffect_Tool::Clear_ProductCuePreview()
{
	Reset_BufferedComboAudition();
    m_ProductPreview.reset();
	m_ValtanProductPreview.reset();
	m_iValtanWorldOwnerStageDurationMs = 0u;
	m_iValtanReferenceEffectStartMs = 0u;
	m_SourcePreviewDocument.reset();
	m_PlayerPreviewCueCandidates.clear();
	m_iPlayerPreviewCueCandidateIndex = 0u;
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
    Reset_ProductCueSnapshot();
}

void Client::CEffect_Tool::Select_PlayerPreviewCueCandidate(
	const size_t iCandidateIndex)
{
	if (!m_ProductPreview.has_value() || !m_ActiveDocument.has_value() ||
		iCandidateIndex >= m_PlayerPreviewCueCandidates.size())
	{
		return;
	}

	const ANIMATION_EFFECT_PREVIEW_CANDIDATE Candidate =
		m_PlayerPreviewCueCandidates[iCandidateIndex];
	Reset_BufferedComboAudition();
	EFFECT_PRODUCT_PREVIEW Preview = *m_ProductPreview;
	Preview.ProductCue.Cue = Candidate.Cue;
	Preview.ProductCue.Clip = Candidate.Clip;
	Preview.ProductCue.iBoundClipOrdinal = Candidate.iBoundClipOrdinal;
	Preview.ProductCue.iStageIndex = Candidate.iStageIndex;
	Preview.ProductCue.iStageClipIndex = Candidate.iStageClipIndex;
	m_ProductPreview = std::move(Preview);
	m_SourcePreviewDocument.reset();
	m_iPlayerPreviewCueCandidateIndex = iCandidateIndex;
	m_fPreviewTimeSeconds = 0.f;
	Reset_ProductCueSnapshot();
	Recalculate_PreviewDuration(*m_ActiveDocument);
	Synchronize_LoadedSkillPreview();
}

void Client::CEffect_Tool::Reset_ProductCueSnapshot()
{
    m_ProductCueSnapshotRoot = Identity_Matrix();
    m_bProductCueSnapshotCaptured = false;
	m_fProductCueActionFacingYawDegrees = 0.f;
	m_bProductCueActionFacingCaptured = false;
}

void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()
{
    // Resource preparation in the UI frame must not consume a short Solo
    // effect's lifetime before its first playback update.
    m_bSkipNextWorldPreviewDelta = true;
    if (m_pAuthoringSequencer && m_ActiveDocument && !m_ProductPreview &&
        (m_ActiveDocument->strEffectAssetId.ends_with(".restore") ||
         Is_SceneAnchoredEffectAssetId(m_ActiveDocument->strEffectAssetId)) &&
        m_ePreviewFilter == EFFECT_PREVIEW_FILTER::SOLO_SELECTED)
    {
        if (Try_PreviewElementTimeline(m_strPreviewIsolationElementId)) m_pAuthoringSequencer->Pause(false);
        return;
    }
    if (m_ActiveDocument && !m_ProductPreview &&
        (m_ActiveDocument->strEffectAssetId.ends_with(".restore") ||
         Is_SceneAnchoredEffectAssetId(m_ActiveDocument->strEffectAssetId)) &&
        m_ePreviewFilter == EFFECT_PREVIEW_FILTER::COMPLETE)
    {
        (void)Try_PlayRecoveryEffect();
        return;
    }
	std::string FreshnessStatus;
	if (!Validate_ActiveRegistryBoundAuditionFreshness(FreshnessStatus))
	{
		Release_WorldPreview(true);
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus =
			"Play rejected: registry-bound audition source freshness failed. " +
			FreshnessStatus;
		return;
	}
	if (m_ValtanCombatObjectIndependentPreview.has_value())
	{
		m_fPreviewTimeSeconds = 0.f;
		if (!Prepare_ValtanStandaloneEffectTarget() ||
			!Sync_ValtanCombatObjectIndependentPreview(true))
		{
			const std::string Failure = m_strPreviewStatus.empty() ?
				m_strPreviewAnimationStatus : m_strPreviewStatus;
			Release_WorldPreview(true);
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus = Failure.empty() ?
				"Independent combat-object restart lost its locked IDLE target." :
				Failure;
		}
		else
		{
			m_bPreviewVisibleRequested = true;
			m_bPreviewPlaying = true;
		}
		return;
	}
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT ==
			m_eActiveDocumentPreviewIntent &&
		!Prepare_ActiveValtanPatternDraftTimeline(false))
	{
		Release_WorldPreview(true);
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus = m_strPreviewAnimationStatus;
		return;
	}
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT ==
			m_eActiveDocumentPreviewIntent &&
		!Prepare_ValtanStandaloneEffectTarget())
	{
		/* Restart is an explicit target boundary. Refuse to reuse a scene-player
		   root if the dedicated static Valtan target cannot be restored. */
		Release_WorldPreview(true);
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus = m_strPreviewAnimationStatus;
		return;
	}
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT ==
			m_eActiveDocumentPreviewIntent &&
		!Update_StaticAreaPreviewRoot())
	{
		Release_WorldPreview(true);
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus =
			"Static Area placement restart refused a missing typed transform.";
		return;
	}
    m_fPreviewTimeSeconds = 0.f;
    Reset_ProductCueSnapshot();
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT !=
		m_eActiveDocumentPreviewIntent)
	{
		Restart_SynchronizedAnimationSequence();
	}
    shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (nullptr == pObject &&
		(m_SourcePreviewDocument.has_value() || m_ActiveDocument.has_value()) &&
		Stage_WorldPreview())
    {
        pObject = m_pWorldPreviewObject.lock();
    }
    if (nullptr == pObject)
    {
		if (m_bReconstructedSourceRuntimeActive)
		{
			Set_SynchronizedAnimationPaused(true);
			Reset_SynchronizedAnimationSequence();
			m_bReconstructedSourceRuntimeActive = false;
			Reset_ReconstructedSourceRuntimeTimeline();
		}
        m_bPreviewPlaying = false;
        return;
    }
	if (m_bReconstructedSourceRuntimeActive)
	{
		Set_SynchronizedAnimationPaused(true);
		Reset_ReconstructedSourceRuntimeTimeline();
		if (!Prepare_ReconstructedSourceRuntimeTransformHistory())
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			Reset_SynchronizedAnimationSequence();
			m_bReconstructedSourceRuntimeActive = false;
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus =
				"Artist F restart historical anchor preparation failed: " +
				m_strPreviewAnimationStatus;
			return;
		}
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
			[this](const f32_t fSampleTimeSeconds,
				EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
				std::string& strOutError)
			{
				return Build_ReconstructedSourceRuntimeTransformSample(
					fSampleTimeSeconds, OutSample, strOutError);
			};
		std::string Error;
		if (!pObject->Set_SampleTimeWithTransformHistory(
				0.f, TransformProvider, Error))
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			Set_SynchronizedAnimationPaused(true);
			Reset_SynchronizedAnimationSequence();
			m_bReconstructedSourceRuntimeActive = false;
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus =
				"Artist F restart historical zero-frame failed: " + Error;
			return;
		}
		m_bReconstructedSourceRuntimeStartPending = true;
		m_bPreviewVisibleRequested = true;
		m_bPreviewPlaying = true;
		pObject->Set_Playing(false);
		pObject->Set_Visible(false);
		m_strPreviewStatus =
			"Artist Core F (33) restart prepared at synchronized time zero; playback starts on the next update.";
		return;
	}
	if (m_bValtanBossPatternTransformHistoryRequired)
	{
		if (!m_bValtanBossPatternTransformHistoryActive)
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			Set_SynchronizedAnimationPaused(true);
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus =
				"Valtan 420633 preview refused a missing exact b_effectroot history binding.";
			return;
		}
		std::string TransformError;
		pObject->Reset();
		if (!Seek_ValtanBossPatternTransformHistory(
				pObject, 0.f, TransformError))
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			Set_SynchronizedAnimationPaused(true);
			m_bValtanBossPatternTransformHistoryActive = false;
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus =
				"Valtan 420633 zero-frame anchor history failed: " +
				TransformError;
			return;
		}
		m_bPreviewVisibleRequested = true;
		m_bPreviewPlaying = true;
		pObject->Set_Visible(true);
		m_strPreviewStatus =
			"Valtan 420633 restart prepared with exact B_EffectRoot / b_effectroot history.";
		return;
	}
    float4x4_t TargetRoot{};
    const bool_t bRootResolved = Resolve_PreviewRoot(TargetRoot);
    if (bRootResolved)
        pObject->Set_RootWorld(TargetRoot);
    pObject->Set_Visible(
        bRootResolved && Is_ProductCueVisible(m_fPreviewTimeSeconds));
    pObject->Reset();
    pObject->Set_SampleTime(
        Resolve_EffectSampleTime(m_fPreviewTimeSeconds));
	m_bPreviewVisibleRequested = true;
    m_bPreviewPlaying = true;
}

void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()
{
	if (m_ValtanCombatObjectIndependentPreview.has_value())
	{
		m_strPreviewAnimationStatus =
			"Valtan IDLE remains paused; combat-object lifecycle playback owns the local preview clock.";
		return;
	}
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT ==
		m_eActiveDocumentPreviewIntent)
	{
		if (!Prepare_ActiveValtanPatternDraftTimeline(true))
		{
			Release_WorldPreview(true);
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus = m_strPreviewAnimationStatus;
		}
		return;
	}
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT ==
		m_eActiveDocumentPreviewIntent)
	{
		/* Editing, Save, Reload, and preview-filter paths may all request a
		   resynchronization. A standalone Valtan Effect remains animation-free
		   for the complete lifetime of the active document. */
		(void)Prepare_ValtanStandaloneEffectTarget();
		return;
	}
	if (EFFECT_DOCUMENT_PREVIEW_INTENT::STATIC_AREA_PLACEMENT ==
		m_eActiveDocumentPreviewIntent)
	{
		Reset_SynchronizedAnimationSequence();
		Reset_ValtanBossPatternTransformHistory();
		m_PlayerPreviewCueCandidates.clear();
		m_iPlayerPreviewCueCandidateIndex = 0u;
		m_strPreviewAnimationStatus =
			"Static Area placement preview is world-root owned and has no actor animation.";
		(void)Update_StaticAreaPreviewRoot();
		return;
	}
    Reset_SynchronizedAnimationSequence();
	Reset_ValtanBossPatternTransformHistory();
	m_PlayerPreviewCueCandidates.clear();
	m_iPlayerPreviewCueCandidateIndex = 0u;
    m_strPreviewAnimationStatus.clear();
	const EFFECT_DOCUMENT_DESC* pPreviewDocument =
		m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value() ?
			&*m_SourcePreviewDocument :
			(m_ActiveDocument.has_value() ? &*m_ActiveDocument : nullptr);
	if (nullptr == pPreviewDocument)
        return;

	/* Boss pattern Effects are action-owned rather than PlayerSkills-owned.
	   Resolve them before the playable catalog join so Model View can stage the
	   real Valtan model and exact authored action clip without a hard-coded
	   effect-to-clip switch. */
	BOSS_PATTERN_EFFECT_BINDING_DOCUMENT BossEffectBindings;
	std::string BossEffectStatus;
	const std::filesystem::path BossEffectPath =
		CValtanPatternEffectBindingDocument::Resolve_Path("Valtan");
	std::ifstream BossEffectInput(BossEffectPath, std::ios::binary);
	if (BossEffectInput)
	{
		const std::string BossEffectText{
			std::istreambuf_iterator<char>(BossEffectInput),
			std::istreambuf_iterator<char>() };
		if (CValtanPatternEffectBindingDocument::Parse_Text(
				BossEffectText, BossEffectBindings, BossEffectStatus))
		{
			const auto BossBinding = std::find_if(
				BossEffectBindings.Bindings.begin(),
				BossEffectBindings.Bindings.end(),
				[pPreviewDocument](
					const BOSS_PATTERN_EFFECT_BINDING& Candidate)
				{
					return Candidate.strEffectAssetId ==
							pPreviewDocument->strEffectAssetId ||
						Matches_ValtanExactHistoryBinding(
							Candidate.strBindingId,
							Candidate.strEffectAssetId,
							pPreviewDocument->strEffectAssetId);
				});
			if (BossBinding != BossEffectBindings.Bindings.end())
			{
				const bool_t bRequiresExactTransformHistory =
					Matches_ValtanExactHistoryBinding(
						BossBinding->strBindingId,
						BossBinding->strEffectAssetId,
						pPreviewDocument->strEffectAssetId);
				if (bRequiresExactTransformHistory)
				{
					m_bValtanBossPatternTransformHistoryRequired = true;
					m_strValtanBossPatternPreviewEffectAssetId =
						pPreviewDocument->strEffectAssetId;
				}
				constexpr const char_t* BOSS_PREVIEW_ASSET = VALTAN_ANIMATION_ASSET_NAME;
				if (CAnimationTargetService::Resolve_AssetName() !=
						BOSS_PREVIEW_ASSET &&
					!m_pCharacterPreviewPanel->Select_TargetAsset(
						BOSS_PREVIEW_ASSET))
				{
					m_strPreviewAnimationStatus =
						"Valtan Effect is loaded, but the boss model could not be staged.";
					return;
				}
				const shared_ptr<Engine::CModel> pBossModel =
					CAnimationTargetService::Resolve_Model();
				if (nullptr == pBossModel ||
					!CValtanPatternEffectBindingDocument::Validate(
						BossEffectBindings, "BOSS_VALTAN",
						Collect_AnimationClipNames(pBossModel), BossEffectStatus))
				{
					m_strPreviewAnimationStatus =
						"Valtan Effect binding was not applied: " + BossEffectStatus;
					return;
				}
				BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT AnimationBindings;
				std::string AnimationStatus;
				if (!CValtanPatternAnimationBindingDocument::Load(
						"Valtan", "BOSS_VALTAN",
						Collect_AnimationClipNames(pBossModel), AnimationBindings,
						AnimationStatus))
				{
					m_strPreviewAnimationStatus =
						"Valtan Effect animation was not applied: " +
						AnimationStatus;
					return;
				}
				const auto AnimationBinding = std::find_if(
					AnimationBindings.Bindings.begin(),
					AnimationBindings.Bindings.end(),
					[&BossBinding](
						const BOSS_PATTERN_ANIMATION_BINDING& Candidate)
					{
						return Candidate.strActionId ==
							BossBinding->strActionId;
					});
				if (AnimationBinding == AnimationBindings.Bindings.end() ||
					AnimationBinding->Clips.end() == std::find(
						AnimationBinding->Clips.begin(),
						AnimationBinding->Clips.end(),
						BossBinding->strRuntimeClipName))
				{
					m_strPreviewAnimationStatus =
						"Valtan Effect action/clip binding drifted; preview failed closed.";
					return;
				}
				m_SynchronizedAnimationClips = {
					{ BossBinding->strRuntimeClipName, 0u, 1.f } };
				m_iSynchronizedAnimationClipIndex = 0u;
				m_iSynchronizedAnimationLoopEpoch = 0u;
				m_iSynchronizedAnimationTargetGeneration =
					CAnimationTargetService::Resolve_TargetGeneration();
				if (!pBossModel->Start_Animation(
						BossBinding->strRuntimeClipName.c_str(), m_bPreviewLoop))
				{
					Reset_SynchronizedAnimationSequence();
					m_strPreviewAnimationStatus =
						"Valtan Effect clip could not be started: " +
						BossBinding->strRuntimeClipName;
					return;
				}
				pBossModel->Set_AnimationSpeed(1.f);
				pBossModel->Set_AnimPaused(false);
				if (bRequiresExactTransformHistory)
				{
					std::string TransformHistoryError;
					if (!Prepare_ValtanBossPatternTransformHistory(
							*BossBinding, *pPreviewDocument,
							TransformHistoryError))
					{
						pBossModel->Set_AnimPaused(true);
						m_strPreviewAnimationStatus =
							"Valtan Effect exact follow anchor was not staged: " +
							TransformHistoryError;
						return;
					}
				}
				m_strPreviewAnimationStatus =
					"Boss pattern animation synced: " +
					BossBinding->strPatternId + " / " +
					BossBinding->strActionId + " -> " +
					BossBinding->strRuntimeClipName + " | " +
					BossBinding->strProductAdmissionStatus;
				return;
			}
		}
	}

    std::string CatalogStatus;
    const bool_t bCatalogAvailable =
        Ensure_PlayerSkillCatalog(CatalogStatus);
    const vector<PLAYER_SKILL_DEFINITION>& Skills =
        CPlayerSkillCatalog::Get_Skills();
    auto Skill = m_ProductPreview.has_value() ?
        std::find_if(
            Skills.begin(), Skills.end(),
            [this](const PLAYER_SKILL_DEFINITION& Candidate)
            {
                return Candidate.eCharacterClass ==
                        m_ProductPreview->eCharacterClass &&
                    Candidate.iSkillId == m_ProductPreview->iSkillId;
            }) :
        std::find_if(
            Skills.begin(), Skills.end(),
            [pPreviewDocument](const PLAYER_SKILL_DEFINITION& Candidate)
            {
                return Candidate.strEffectId ==
                    pPreviewDocument->strEffectAssetId;
            });
    if (!m_ProductPreview.has_value() && Skill == Skills.end() &&
        (pPreviewDocument->strEffectAssetId == ARTIST_F_UNIFIED_EFFECT_ASSET_ID ||
         pPreviewDocument->strEffectAssetId ==
            DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID))
    {
        const bool_t bArtistFUnified =
            pPreviewDocument->strEffectAssetId == ARTIST_F_UNIFIED_EFFECT_ASSET_ID;
        const LostArk::Shared::CHARACTER_CLASS_ID eUnifiedClass = bArtistFUnified ?
            LostArk::Shared::CHARACTER_CLASS_ID::ARTIST :
            LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER;
        const uint32_t iUnifiedSkillId = bArtistFUnified ?
            ARTIST_F_CORE_SKILL_ID : DIMENSION_MASTER_T_SKILL_ID;
        Skill = std::find_if(
            Skills.begin(), Skills.end(),
            [eUnifiedClass, iUnifiedSkillId](
                const PLAYER_SKILL_DEFINITION& Candidate)
            {
                return Candidate.eCharacterClass == eUnifiedClass &&
                    Candidate.iSkillId == iUnifiedSkillId;
            });
    }
	if (!m_ProductPreview.has_value() && Skill == Skills.end())
	{
		const std::string& DocumentAssetId =
			pPreviewDocument->strEffectAssetId;
		const auto OwnsDocument = [&DocumentAssetId](
			const std::string_view ProductAssetId)
		{
			return !ProductAssetId.empty() &&
				(ProductAssetId == DocumentAssetId ||
				 Unified_CandidateAssetId(ProductAssetId) == DocumentAssetId);
		};
		const auto Owner = std::find_if(m_AllEffects.begin(), m_AllEffects.end(),
			[&OwnsDocument](const EFFECT_SKILL_TREE_ENTRY& Entry)
			{
				if (OwnsDocument(Entry.Skill.strEffectId))
					return true;
				return std::any_of(Entry.ProductCues.begin(),
					Entry.ProductCues.end(), [&OwnsDocument](const auto& Cue)
					{
						return OwnsDocument(Cue.Cue.strEffectAssetId);
					});
			});
		if (Owner != m_AllEffects.end())
		{
			Skill = std::find_if(Skills.begin(), Skills.end(),
				[&Owner](const PLAYER_SKILL_DEFINITION& Candidate)
				{
					return Candidate.eCharacterClass ==
							Owner->Skill.eCharacterClass &&
						Candidate.iSkillId == Owner->Skill.iSkillId;
			});
		}
	}
	if (!m_ProductPreview.has_value() && Skill == Skills.end())
	{
		const std::string& DocumentAssetId =
			pPreviewDocument->strEffectAssetId;
		const auto CandidateOwner = std::find_if(
			m_UnifiedCandidateBindings.begin(),
			m_UnifiedCandidateBindings.end(),
			[&DocumentAssetId](
				const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding)
			{
				return Binding.strEffectAssetId == DocumentAssetId;
			});
		if (CandidateOwner != m_UnifiedCandidateBindings.end())
		{
			Skill = std::find_if(Skills.begin(), Skills.end(),
				[&CandidateOwner](const PLAYER_SKILL_DEFINITION& Candidate)
				{
					return Candidate.eCharacterClass ==
							CandidateOwner->eCharacterClass &&
						Candidate.iSkillId == CandidateOwner->iSkillId;
				});
		}
	}
    if (!m_ProductPreview.has_value() && Skill == Skills.end() &&
        std::string::npos != pPreviewDocument->strEffectAssetId.find(
            "restoration-candidate"))
    {
        Skill = std::find_if(
            Skills.begin(), Skills.end(),
            [pPreviewDocument](const PLAYER_SKILL_DEFINITION& Candidate)
            {
                return !Candidate.strEffectId.empty() &&
                    pPreviewDocument->strEffectAssetId.starts_with(
                        Candidate.strEffectId + ".");
            });
    }
    if (Skill == Skills.end())
    {
        m_strPreviewAnimationStatus = bCatalogAvailable ?
            "No PlayerSkills row owns this Effect; animation was left unchanged." :
            "PlayerSkills refresh failed; animation was left unchanged: " +
                CatalogStatus;
        return;
    }
    if (m_ProductPreview.has_value() &&
		pPreviewDocument->strEffectAssetId !=
            m_ProductPreview->ProductCue.Cue.strEffectAssetId)
    {
        m_strPreviewAnimationStatus =
            "Product animation sync rejected a stale cue/document pairing.";
        return;
    }

    m_eAllEffectsClass = Skill->eCharacterClass;
    Select_AuthoringDomainForClass(Skill->eCharacterClass);
    const char* pAnimationAsset = Animation_AssetName(Skill->eCharacterClass);
    if (nullptr == pAnimationAsset)
    {
        m_strPreviewAnimationStatus =
            "The loaded Effect has no admitted playable class target.";
        return;
    }

    const std::string CurrentAsset =
        CAnimationTargetService::Resolve_AssetName();
    if (CurrentAsset != pAnimationAsset &&
        !m_pCharacterPreviewPanel->Select_TargetAsset(pAnimationAsset))
    {
        m_strPreviewAnimationStatus =
            "Effect is playing on the current target; the matching class model "
            "could not be staged.";
        return;
    }

    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr == pModel)
    {
        m_strPreviewAnimationStatus =
            "Effect is loaded, but no animation model target is available.";
        return;
    }

    ANIMATION_SKILL_BINDING_DOCUMENT Bindings;
    std::string BindingStatus;
	const std::vector<std::string> AvailableClips =
		Collect_AnimationClipNames(pModel);
    if (!CAnimationSkillBindingDocument::Load(
        pAnimationAsset,
        Skill->eCharacterClass,
        Skills,
        AvailableClips,
        Bindings,
        BindingStatus))
    {
        m_strPreviewAnimationStatus =
            "Effect is playing; skill animation binding was not applied: " +
            BindingStatus;
        return;
    }

    const auto Binding = std::find_if(
        Bindings.Bindings.begin(), Bindings.Bindings.end(),
        [&Skill](const ANIMATION_SKILL_BINDING& Candidate)
        {
            return Candidate.iSkillId == Skill->iSkillId;
        });
    if (Binding == Bindings.Bindings.end() || Binding->Stages.empty())
    {
        m_strPreviewAnimationStatus =
            "Effect is playing; its first bound animation clip is unavailable.";
        return;
    }

	const bool_t bSavedPlayerDirectAuthored =
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource &&
		m_ActiveDocument.has_value() &&
		pPreviewDocument == &*m_ActiveDocument;
	if (bSavedPlayerDirectAuthored)
	{
		const std::filesystem::path EventPath = CProjectDataRoot::Resolve(
			std::filesystem::path(L"Animation") / L"Authored" /
			std::filesystem::path(pAnimationAsset) /
			(std::filesystem::path(pAnimationAsset).wstring() +
				L".animevents"));
		std::string EventText;
		std::string CandidateStatus;
		ANIMATION_EFFECT_CUE_DOCUMENT CueDocument;
		if (!Read_TextFile(EventPath, EventText, CandidateStatus) ||
			!CAnimationEffectCueDocument::Load_FromText(
				pAnimationAsset, EventText, AvailableClips, CueDocument,
				CandidateStatus, true) ||
			!CAnimationEffectCueDocument::Resolve_PreviewCandidates(
				*Binding, CueDocument.Cues,
				pPreviewDocument->strEffectAssetId,
				m_PlayerPreviewCueCandidates, CandidateStatus))
		{
			m_ProductPreview.reset();
			m_SourcePreviewDocument.reset();
			Recalculate_PreviewDuration(*m_ActiveDocument);
			m_strPreviewAnimationStatus =
				"Saved Player Effect has no exact Product cue mapping; "
				"full skill-chain preview was refused: " + CandidateStatus;
			return;
		}

		size_t iSelectedCandidate = 0u;
		if (m_ProductPreview.has_value())
		{
			const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Selected =
				m_ProductPreview->ProductCue;
			const auto Match = std::find_if(
				m_PlayerPreviewCueCandidates.begin(),
				m_PlayerPreviewCueCandidates.end(),
				[&Selected](
					const ANIMATION_EFFECT_PREVIEW_CANDIDATE& Candidate)
				{
					return Candidate.iStageIndex == Selected.iStageIndex &&
						Candidate.iStageClipIndex == Selected.iStageClipIndex &&
						Candidate.Cue.strClipName == Selected.Cue.strClipName &&
						Candidate.Cue.iStartMs == Selected.Cue.iStartMs &&
						Candidate.Cue.strEffectAssetId ==
							Selected.Cue.strEffectAssetId;
				});
			if (Match != m_PlayerPreviewCueCandidates.end())
			{
				iSelectedCandidate = static_cast<size_t>(
					std::distance(m_PlayerPreviewCueCandidates.begin(), Match));
			}
		}
		m_iPlayerPreviewCueCandidateIndex = iSelectedCandidate;
		const ANIMATION_EFFECT_PREVIEW_CANDIDATE& Candidate =
			m_PlayerPreviewCueCandidates[iSelectedCandidate];
		EFFECT_PRODUCT_PREVIEW Preview;
		Preview.eCharacterClass = Skill->eCharacterClass;
		Preview.iSkillId = Skill->iSkillId;
		Preview.ProductCue.Cue = Candidate.Cue;
		Preview.ProductCue.Clip = Candidate.Clip;
		Preview.ProductCue.iBoundClipOrdinal = Candidate.iBoundClipOrdinal;
		Preview.ProductCue.iStageIndex = Candidate.iStageIndex;
		Preview.ProductCue.iStageClipIndex = Candidate.iStageClipIndex;
		m_ProductPreview = std::move(Preview);
		m_SourcePreviewDocument.reset();
		Recalculate_PreviewDuration(*m_ActiveDocument);
	}
	m_SynchronizedAnimationClips.clear();
	if (m_bBufferedComboAuditionActive)
	{
		if (!m_ProductPreview.has_value() ||
			m_eBufferedComboAuditionClass != Skill->eCharacterClass ||
			m_iBufferedComboAuditionSkillId != Skill->iSkillId)
		{
			Reset_BufferedComboAudition();
			m_strPreviewAnimationStatus =
				"Buffered combo audition rejected stale Product preview ownership.";
			return;
		}
		f32_t fBufferedDurationSeconds = 0.f;
		std::vector<std::vector<f32_t>> BufferedStageClipOffsetsSeconds;
		std::string BufferedError;
		if (!Try_BuildBufferedComboAnimationClips(
				*Skill, Skills, m_SynchronizedAnimationClips,
				BufferedStageClipOffsetsSeconds,
				fBufferedDurationSeconds, BufferedError))
		{
			Reset_BufferedComboAudition();
			m_SynchronizedAnimationClips.clear();
			m_strPreviewAnimationStatus =
				"Buffered combo audition failed closed: " + BufferedError;
			return;
		}
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& SelectedOccurrence =
			m_ProductPreview->ProductCue;
		if (SelectedOccurrence.iStageIndex >=
				BufferedStageClipOffsetsSeconds.size() ||
			SelectedOccurrence.iStageClipIndex >=
				BufferedStageClipOffsetsSeconds[
					SelectedOccurrence.iStageIndex].size())
		{
			Reset_BufferedComboAudition();
			m_SynchronizedAnimationClips.clear();
			m_strPreviewAnimationStatus =
				"Buffered combo audition rejected a Product occurrence outside its Server stage boundary.";
			return;
		}
		m_fBufferedComboAuditionDurationSeconds =
			fBufferedDurationSeconds;
		m_fBufferedComboAuditionOccurrenceOffsetSeconds =
			BufferedStageClipOffsetsSeconds[SelectedOccurrence.iStageIndex]
				[SelectedOccurrence.iStageClipIndex];
	}
    else if (m_ProductPreview.has_value())
    {
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Selected =
			m_ProductPreview->ProductCue;
		if (Selected.iStageIndex < Binding->Stages.size())
		{
			const ANIMATION_SKILL_STAGE& Stage =
				Binding->Stages[Selected.iStageIndex];
			if (Selected.iStageClipIndex < Stage.Clips.size())
			{
				const ANIMATION_SKILL_CLIP& Clip =
					Stage.Clips[Selected.iStageClipIndex];
				if (Clip == Selected.Clip &&
					Clip.strClipName == Selected.Cue.strClipName &&
					CAnimationEffectCueDocument::Is_CueStartInClipWindow(
						Clip, Selected.Cue.iStartMs))
				{
					m_SynchronizedAnimationClips.push_back(Clip);
				}
			}
		}
    }
    else
    {
        /* Generic Data File preview retains the full authored chain. Product
        Play above intentionally owns only the exact clip named by its cue. */
        for (const ANIMATION_SKILL_STAGE& Stage : Binding->Stages)
        {
            m_SynchronizedAnimationClips.insert(
                m_SynchronizedAnimationClips.end(),
                Stage.Clips.begin(), Stage.Clips.end());
        }
    }
    if (m_SynchronizedAnimationClips.empty())
    {
        m_strPreviewAnimationStatus =
            "Effect is playing; its first bound animation clip is unavailable.";
        return;
    }
    m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
    m_iSynchronizedAnimationTargetGeneration =
        CAnimationTargetService::Resolve_TargetGeneration();
	const SYNCHRONIZED_ANIMATION_CLIP& FirstClip =
        m_SynchronizedAnimationClips.front();
	if (!Start_SynchronizedAnimationClip(0u, false))
    {
		if (m_bBufferedComboAuditionActive)
			Reset_BufferedComboAudition();
        Reset_SynchronizedAnimationSequence();
        m_strPreviewAnimationStatus =
            "Effect is playing; its first bound animation clip is unavailable.";
        return;
    }
	m_strPreviewAnimationStatus = m_bBufferedComboAuditionActive ?
		"Buffered combo audition synced: " :
		(m_ProductPreview.has_value() ?
			"Product cue animation synced: " : "Skill animation synced: ");
    m_strPreviewAnimationStatus +=
        Skill->strInputSlot + " | " + Skill->strDisplayName + " -> " +
        FirstClip.strClipName;
	if (m_bBufferedComboAuditionActive)
	{
		m_strPreviewAnimationStatus += " | " +
			std::to_string(Skill->ComboStages.size()) +
			" Server input stages";
	}
    else if (m_ProductPreview.has_value())
    {
        m_strPreviewAnimationStatus += " @ " + std::to_string(
            m_ProductPreview->ProductCue.Cue.iStartMs) + " ms | anchor=" +
            m_ProductPreview->ProductCue.Cue.strAnchorSlotId;
    }
	if (1u != m_SynchronizedAnimationClips.size())
    {
        m_strPreviewAnimationStatus += " (sequence 1/" +
            std::to_string(m_SynchronizedAnimationClips.size()) + ")";
    }
	/* Product authoring must retain the complete selected animation window even
	   when this occurrence's Effect tail is shorter.  Recalculate only after
	   the exact class model and clip window have been staged. */
	if (m_ProductPreview.has_value())
		Recalculate_PreviewDuration(*pPreviewDocument);
}

bool_t Client::CEffect_Tool::Resolve_SynchronizedAnimationClipStart(
	const std::shared_ptr<Engine::CModel>& pModel,
	const SYNCHRONIZED_ANIMATION_CLIP& Clip,
	uint32_t& iOutAnimationIndex,
	f32_t& fOutSourceStartTicks) const
{
	if (nullptr == pModel || Clip.strClipName.empty() ||
		Clip.strClipName.find('\0') != std::string::npos ||
		!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f ||
		!std::isfinite(Clip.fHoldAfterSeconds) || Clip.fHoldAfterSeconds < 0.f)
	{
		return false;
	}

	uint32_t iAnimationIndex = UINT32_MAX;
	for (uint32_t iCandidate = 0u;
		iCandidate < pModel->Get_NumAnimations(); ++iCandidate)
	{
		const char_t* pName = pModel->Get_AnimationName(iCandidate);
		if (nullptr == pName || Clip.strClipName != pName)
			continue;
		if (UINT32_MAX != iAnimationIndex)
			return false;
		iAnimationIndex = iCandidate;
	}
	if (UINT32_MAX == iAnimationIndex)
		return false;

	f32_t fPositionTicks = 0.f;
	f32_t fDurationTicks = 0.f;
	const f32_t fTicksPerSecond =
		pModel->Get_AnimationTickPerSecond(iAnimationIndex);
	if (!pModel->Get_AnimationProgress(
			iAnimationIndex, fPositionTicks, fDurationTicks) ||
		!std::isfinite(fDurationTicks) || fDurationTicks <= 0.f ||
		!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f)
	{
		return false;
	}

	ACTION_PRESENTATION_CLIP_TIMING Timing;
	Timing.fModelSourceDurationSeconds = fDurationTicks / fTicksPerSecond;
	Timing.iPlayMs = Clip.iPlayMs;
	Timing.fPlayRate = Clip.fPlayRate;
	Timing.bLoop = Clip.bHasExplicitLoopPolicy && Clip.bLoop;
	Timing.fSourceStartSeconds =
		static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
	f32_t fSourceDurationSeconds = 0.f;
	f32_t fWallDurationSeconds = 0.f;
	if (!std::isfinite(Timing.fModelSourceDurationSeconds) ||
		Timing.fModelSourceDurationSeconds <= 0.f ||
		!CActionPresentationTimeline::Resolve_ClipDuration(
			Timing, fSourceDurationSeconds, fWallDurationSeconds))
	{
		return false;
	}
	const f32_t fSourceStartTicks =
		Timing.fSourceStartSeconds * fTicksPerSecond;
	if (!std::isfinite(fSourceStartTicks) || fSourceStartTicks < 0.f ||
		fSourceStartTicks >= fDurationTicks)
	{
		return false;
	}

	iOutAnimationIndex = iAnimationIndex;
	fOutSourceStartTicks = fSourceStartTicks;
	return true;
}

bool_t Client::CEffect_Tool::Start_SynchronizedAnimationClip(
	const size_t iClipIndex,
	const bool_t bPaused)
{
	if (iClipIndex >= m_SynchronizedAnimationClips.size())
		return false;
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
		return false;
	const SYNCHRONIZED_ANIMATION_CLIP& Clip =
		m_SynchronizedAnimationClips[iClipIndex];
	uint32_t iAnimationIndex = UINT32_MAX;
	f32_t fSourceStartTicks = 0.f;
	if (!Resolve_SynchronizedAnimationClipStart(
			pModel, Clip, iAnimationIndex, fSourceStartTicks))
	{
		return false;
	}
	const bool_t bHasSourceWindow =
		0u != Clip.iSourceStartMs || 0u != Clip.iPlayMs;
	const bool_t bEngineLoop = !Clip.bHasExplicitLoopPolicy &&
		!bHasSourceWindow &&
		1u == m_SynchronizedAnimationClips.size() && m_bPreviewLoop;
	if (!pModel->Start_Animation(iAnimationIndex, bEngineLoop))
		return false;

	pModel->Set_AnimationSpeed(Clip.fPlayRate);
	if (0u != Clip.iSourceStartMs)
	{
		pModel->Set_AnimTrackPosition(iAnimationIndex, fSourceStartTicks);
		pModel->Play_Animation(0.f);
	}
	pModel->Set_AnimPaused(bPaused);
	return true;
}

void Client::CEffect_Tool::Restart_SynchronizedAnimationSequence()
{
    if (m_SynchronizedAnimationClips.empty() ||
        m_iSynchronizedAnimationTargetGeneration !=
            CAnimationTargetService::Resolve_TargetGeneration())
    {
        return;
    }
	m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
	if (!Start_SynchronizedAnimationClip(0u, false))
	{
		m_strPreviewAnimationStatus =
			"Skill animation restart failed: " +
			m_SynchronizedAnimationClips.front().strClipName;
		return;
	}
}

void Client::CEffect_Tool::Seek_SynchronizedAnimationSequence(
    const f32_t fTimeSeconds)
{
	if (m_SynchronizedAnimationClips.empty() || !std::isfinite(fTimeSeconds) ||
		m_iSynchronizedAnimationTargetGeneration != CAnimationTargetService::Resolve_TargetGeneration())
		return;
	const auto pModel = CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
		return;
	std::vector<ACTION_PRESENTATION_CLIP_TIMING> Timings;
	std::vector<float> WallBudgets;
	std::vector<uint32_t> AnimationIndices;
	std::vector<float> TickRates;
	for (const SYNCHRONIZED_ANIMATION_CLIP& Clip : m_SynchronizedAnimationClips)
	{
		uint32_t iAnimation = UINT32_MAX;
		for (uint32_t iCandidate = 0u; iCandidate < pModel->Get_NumAnimations(); ++iCandidate)
		{
			const char_t* pName = pModel->Get_AnimationName(iCandidate);
			if (nullptr != pName && Clip.strClipName == pName)
			{
				iAnimation = iCandidate;
				break;
			}
		}
		float Position = 0.f, Duration = 0.f;
		const float TickRate = iAnimation == UINT32_MAX ? 0.f :
			pModel->Get_AnimationTickPerSecond(iAnimation);
		if (iAnimation == UINT32_MAX || !std::isfinite(TickRate) || TickRate <= 0.f ||
			!pModel->Get_AnimationProgress(iAnimation, Position, Duration) ||
			!std::isfinite(Clip.fHoldAfterSeconds) || Clip.fHoldAfterSeconds < 0.f)
		{
			m_strPreviewAnimationStatus = "Skill animation seek has an invalid source clip: " + Clip.strClipName;
			return;
		}
		ACTION_PRESENTATION_CLIP_TIMING Timing;
		Timing.fModelSourceDurationSeconds = Duration / TickRate;
		Timing.iPlayMs = Clip.iPlayMs;
		Timing.fPlayRate = Clip.fPlayRate;
		Timing.bLoop = Clip.bHasExplicitLoopPolicy && Clip.bLoop;
		Timing.fSourceStartSeconds = static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
		float SourceDuration = 0.f, WallDuration = 0.f;
		if (!CActionPresentationTimeline::Resolve_ClipDuration(Timing, SourceDuration, WallDuration))
		{
			m_strPreviewAnimationStatus = "Skill animation seek has an invalid source segment: " + Clip.strClipName;
			return;
		}
		Timings.push_back(Timing);
		WallBudgets.push_back(0u == Clip.iAuthoringWallMs ? WallDuration + Clip.fHoldAfterSeconds :
			static_cast<f32_t>(Clip.iAuthoringWallMs) * 0.001f);
		AnimationIndices.push_back(iAnimation);
		TickRates.push_back(TickRate);
	}
	ACTION_PRESENTATION_SAMPLE Sample;
	if (!CActionPresentationTimeline::Resolve_PreviewSequenceSample(
			Timings, WallBudgets, (std::max)(0.f, fTimeSeconds), Sample))
	{
		m_strPreviewAnimationStatus = "Skill animation seek could not resolve the finite authoring timeline.";
		return;
	}
	const auto& Clip = m_SynchronizedAnimationClips[Sample.iClipIndex];
	float SourceDuration = 0.f, WallDuration = 0.f;
	if (!CActionPresentationTimeline::Resolve_ClipDuration(Timings[Sample.iClipIndex], SourceDuration, WallDuration))
		return;
	const bool_t bHoldingEndPose = !(Clip.bHasExplicitLoopPolicy && Clip.bLoop) &&
		Sample.fClipSourceTimeSeconds >= Timings[Sample.iClipIndex].fSourceStartSeconds + SourceDuration;
	m_iSynchronizedAnimationLoopEpoch = Sample.iLoopEpoch;
	m_iSynchronizedAnimationClipIndex = Sample.iClipIndex;
	if (!Start_SynchronizedAnimationClip(Sample.iClipIndex, bHoldingEndPose || !m_bPreviewPlaying))
	{
		m_strPreviewAnimationStatus = "Skill animation seek failed: " + Clip.strClipName;
		return;
	}
	pModel->Set_AnimTrackPosition(AnimationIndices[Sample.iClipIndex],
		Sample.fClipSourceTimeSeconds * TickRates[Sample.iClipIndex]);
	pModel->Play_Animation(0.f);
	pModel->Set_AnimPaused(bHoldingEndPose || !m_bPreviewPlaying);
}

void Client::CEffect_Tool::Set_SynchronizedAnimationPaused(
    const bool_t bPaused)
{
    if (m_SynchronizedAnimationClips.empty() ||
        m_iSynchronizedAnimationTargetGeneration !=
            CAnimationTargetService::Resolve_TargetGeneration())
    {
        return;
    }
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr != pModel)
        pModel->Set_AnimPaused(bPaused);
}

bool_t Client::CEffect_Tool::Try_ResolveSynchronizedAnimationTime(
    f32_t& fOutTimeSeconds) const
{
    fOutTimeSeconds = 0.f;
    if (m_SynchronizedAnimationClips.empty() ||
        m_iSynchronizedAnimationTargetGeneration !=
            CAnimationTargetService::Resolve_TargetGeneration())
    {
        return false;
    }
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr == pModel ||
        m_iSynchronizedAnimationClipIndex >=
            m_SynchronizedAnimationClips.size())
    {
        return false;
    }

    for (size_t iClip = 0u;
        iClip <= m_iSynchronizedAnimationClipIndex; ++iClip)
    {
		const SYNCHRONIZED_ANIMATION_CLIP& Clip =
            m_SynchronizedAnimationClips[iClip];
        uint32_t iAnimation = UINT32_MAX;
        for (uint32_t iCandidate = 0u;
            iCandidate < pModel->Get_NumAnimations(); ++iCandidate)
        {
            const char_t* pName = pModel->Get_AnimationName(iCandidate);
            if (nullptr != pName && Clip.strClipName == pName)
            {
                iAnimation = iCandidate;
                break;
            }
        }
        f32_t fPosition = 0.f;
        f32_t fDuration = 0.f;
        const f32_t fTicksPerSecond = UINT32_MAX == iAnimation ? 0.f :
            pModel->Get_AnimationTickPerSecond(iAnimation);
        if (UINT32_MAX == iAnimation ||
            !pModel->Get_AnimationProgress(
                iAnimation, fPosition, fDuration) ||
            !std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
            !std::isfinite(fDuration) || fDuration <= 0.f)
        {
            return false;
        }

		const f32_t fSourceStartSeconds =
			static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
		f32_t fSourceDurationSeconds =
			fDuration / fTicksPerSecond - fSourceStartSeconds;
		if (!std::isfinite(fSourceDurationSeconds) ||
			fSourceDurationSeconds <= 0.f ||
			!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f)
		{
			return false;
		}
		if (0u != Clip.iPlayMs)
        {
            fSourceDurationSeconds = (std::min)(
                fSourceDurationSeconds,
                static_cast<f32_t>(Clip.iPlayMs) * 0.001f);
		}
		if (iClip < m_iSynchronizedAnimationClipIndex)
		{
			if (!std::isfinite(Clip.fHoldAfterSeconds) ||
				Clip.fHoldAfterSeconds < 0.f)
			{
				return false;
			}
			const f32_t fDefaultSegmentWallSeconds =
				fSourceDurationSeconds / Clip.fPlayRate +
				Clip.fHoldAfterSeconds;
			fOutTimeSeconds += 0u == Clip.iAuthoringWallMs ?
				fDefaultSegmentWallSeconds :
				static_cast<f32_t>(Clip.iAuthoringWallMs) * 0.001f;
			continue;
        }

        const char_t* pCurrentName = pModel->Get_AnimationName(
            pModel->Get_CurrentAnimIndex());
        if (nullptr == pCurrentName || Clip.strClipName != pCurrentName)
            return false;
		const f32_t fWallDurationSeconds =
			fSourceDurationSeconds / Clip.fPlayRate;
		const f32_t fTimelineClipWallDurationSeconds =
			0u == Clip.iAuthoringWallMs ? fWallDurationSeconds :
				static_cast<f32_t>(Clip.iAuthoringWallMs) * 0.001f;
		const f32_t fCurrentSourceSeconds = (std::min)(
			(std::max)(0.f,
				fPosition / fTicksPerSecond - fSourceStartSeconds),
			fSourceDurationSeconds);
		const bool_t bBufferedFinalClip =
			m_bBufferedComboAuditionActive &&
			iClip + 1u == m_SynchronizedAnimationClips.size();
		if (m_bBufferedComboAuditionActive &&
			pModel->Is_AnimPaused() &&
			(Clip.fHoldAfterSeconds > 0.f || bBufferedFinalClip) &&
			fCurrentSourceSeconds + 0.0001f >= fSourceDurationSeconds)
		{
			/* A Server-owned stage hold has no advancing animation clock. The
			   Effect Tool wall clock advances it until Seek selects the next row. */
			return false;
		}
		const bool_t bHasSourceWindow =
			0u != Clip.iSourceStartMs || 0u != Clip.iPlayMs;
		const bool_t bAuthoredEndPoseHold =
			0u != Clip.iAuthoringWallMs &&
			fTimelineClipWallDurationSeconds >
				fWallDurationSeconds + 0.0001f;
		/* Use the model clock plus completed epochs, not the Tool's previous
		   frame time, to recognize a finite loop's held final boundary. */
		const f32_t fCurrentWallSeconds =
			static_cast<f32_t>(m_iSynchronizedAnimationLoopEpoch) *
				fWallDurationSeconds +
			fCurrentSourceSeconds / Clip.fPlayRate;
		if (CActionPresentationTimeline::
			Should_ReleaseCompletedAnimationClock(
				Clip.bHasExplicitLoopPolicy || bHasSourceWindow,
				Clip.bHasExplicitLoopPolicy ? Clip.bLoop : m_bPreviewLoop,
				iClip + 1u == m_SynchronizedAnimationClips.size(),
				bAuthoredEndPoseHold,
				pModel->Is_AnimPaused(), fCurrentSourceSeconds,
				fSourceDurationSeconds, fCurrentWallSeconds,
				0u == Clip.iAuthoringWallMs ? 0.f :
					fTimelineClipWallDurationSeconds))
		{
			/* Keep the model paused on its final pose, but let the Effect Tool
			   wall clock finish an authored hold or final natural Effect tail. */
			return false;
		}
		fOutTimeSeconds += (std::min)(
			fCurrentWallSeconds, fTimelineClipWallDurationSeconds);
    }
    return std::isfinite(fOutTimeSeconds);
}

void Client::CEffect_Tool::Update_SynchronizedAnimationSequence()
{
    if (m_SynchronizedAnimationClips.empty())
        return;
    if (m_iSynchronizedAnimationTargetGeneration !=
        CAnimationTargetService::Resolve_TargetGeneration())
    {
        Reset_SynchronizedAnimationSequence();
        return;
    }
	if (m_SynchronizedAnimationClips.size() <= 1u &&
		!m_SynchronizedAnimationClips.front().bHasExplicitLoopPolicy &&
		0u == m_SynchronizedAnimationClips.front().iSourceStartMs &&
		0u == m_SynchronizedAnimationClips.front().iPlayMs &&
		0u == m_SynchronizedAnimationClips.front().iAuthoringWallMs)
		return;
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr == pModel ||
        m_iSynchronizedAnimationClipIndex >=
            m_SynchronizedAnimationClips.size())
    {
        Reset_SynchronizedAnimationSequence();
        return;
    }
    const uint32_t iAnimation = pModel->Get_CurrentAnimIndex();
    const char_t* pCurrentName = pModel->Get_AnimationName(iAnimation);
	const SYNCHRONIZED_ANIMATION_CLIP& CurrentClip =
        m_SynchronizedAnimationClips[m_iSynchronizedAnimationClipIndex];
    if (nullptr == pCurrentName || CurrentClip.strClipName != pCurrentName)
    {
        Reset_SynchronizedAnimationSequence();
        return;
    }
	if (pModel->Is_AnimPaused())
	{
		if (m_bBufferedComboAuditionActive && m_bPreviewPlaying)
		{
			/* During an authored end-pose hold the animation clock is paused, so
			   the Effect Tool wall clock owns the transition to the next input
			   stage. Seek only while paused; active clips keep natural playback. */
			Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
		}
		return;
	}

    f32_t fPosition = 0.f;
    f32_t fDuration = 0.f;
    if (!pModel->Get_AnimationProgress(
        iAnimation, fPosition, fDuration) || fDuration <= 0.f)
    {
        return;
    }
	const f32_t fTicksPerSecond =
		pModel->Get_AnimationTickPerSecond(iAnimation);
	if (!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f)
		return;
	const f32_t fSourceStart =
		static_cast<f32_t>(CurrentClip.iSourceStartMs) * 0.001f *
		fTicksPerSecond;
	if (!std::isfinite(fSourceStart) || fSourceStart < 0.f ||
		fSourceStart >= fDuration)
	{
		Reset_SynchronizedAnimationSequence();
		return;
	}
	f32_t fLimit = fDuration;
	if (0u != CurrentClip.iPlayMs && std::isfinite(fTicksPerSecond) &&
		fTicksPerSecond > 0.f)
	{
		fLimit = (std::min)(
			fDuration,
			fSourceStart +
				static_cast<f32_t>(CurrentClip.iPlayMs) * 0.001f *
				fTicksPerSecond);
    }
	const f32_t fSourceSegmentSeconds =
		(fLimit - fSourceStart) / fTicksPerSecond;
	const f32_t fSourceSegmentWallSeconds =
		fSourceSegmentSeconds / CurrentClip.fPlayRate;
	const f32_t fTimelineClipWallSeconds =
		0u == CurrentClip.iAuthoringWallMs ? fSourceSegmentWallSeconds :
			static_cast<f32_t>(CurrentClip.iAuthoringWallMs) * 0.001f;
	const f32_t fEpochWallStart = static_cast<f32_t>(
		m_iSynchronizedAnimationLoopEpoch) * fSourceSegmentWallSeconds;
	if (CurrentClip.bHasExplicitLoopPolicy && CurrentClip.bLoop &&
		0u != CurrentClip.iAuthoringWallMs)
	{
		const f32_t fRemainingTimelineWall = (std::max)(
			0.f, fTimelineClipWallSeconds - fEpochWallStart);
		const f32_t fAllowedWall = (std::min)(
			fSourceSegmentWallSeconds, fRemainingTimelineWall);
		fLimit = fSourceStart + fAllowedWall * CurrentClip.fPlayRate *
			fTicksPerSecond;
	}
	else if (0u != CurrentClip.iAuthoringWallMs)
	{
		fLimit = (std::min)(fLimit,
			fSourceStart + fTimelineClipWallSeconds *
				CurrentClip.fPlayRate * fTicksPerSecond);
	}
	if (fPosition + 0.0001f < fLimit)
    {
        return;
    }
	if (!(CurrentClip.bHasExplicitLoopPolicy && CurrentClip.bLoop) &&
		0u != CurrentClip.iAuthoringWallMs &&
		fTimelineClipWallSeconds > fSourceSegmentWallSeconds + 0.0001f)
	{
		f32_t fOccurrenceStartSeconds = 0.f;
		for (size_t i = 0u; i < m_iSynchronizedAnimationClipIndex; ++i)
		{
			const SYNCHRONIZED_ANIMATION_CLIP& Previous =
				m_SynchronizedAnimationClips[i];
			fOccurrenceStartSeconds += static_cast<f32_t>(
				Previous.iAuthoringWallMs) * 0.001f;
		}
		const f32_t fTimelineLocalSeconds = (std::max)(
			0.f, m_fPreviewTimeSeconds - fOccurrenceStartSeconds);
		if (fTimelineLocalSeconds + 0.0001f <
			fTimelineClipWallSeconds)
		{
			/* Preserve the final source pose until the master-owned semantic
			   stage wall expires. The Effect Tool wall clock will seek into the
			   next occurrence at the exact boundary. */
			pModel->Set_AnimPaused(true);
			return;
		}
	}

	const bool_t bLastClip =
		m_iSynchronizedAnimationClipIndex + 1u ==
			m_SynchronizedAnimationClips.size();
	if (m_bBufferedComboAuditionActive &&
		CurrentClip.fHoldAfterSeconds > 0.f)
	{
		pModel->Set_AnimPaused(true);
		return;
	}
	if (m_bBufferedComboAuditionActive && bLastClip)
	{
		/* A buffered audition runs one finite Server input chain. The global
		   Tool Loop applies to the complete Effect timeline, never to its final
		   animation clip while an occurrence-local Effect tail is still alive. */
		pModel->Set_AnimPaused(true);
		return;
	}
	const bool_t bRepeatLoop =
		CurrentClip.bHasExplicitLoopPolicy && CurrentClip.bLoop &&
		(0u == CurrentClip.iAuthoringWallMs ||
		 fEpochWallStart + fSourceSegmentWallSeconds + 0.0001f <
			fTimelineClipWallSeconds);
	if (bRepeatLoop)
	{
		++m_iSynchronizedAnimationLoopEpoch;
		if (!Start_SynchronizedAnimationClip(
				m_iSynchronizedAnimationClipIndex, false))
		{
			Reset_SynchronizedAnimationSequence();
		}
		return;
	}
	if (bLastClip &&
		(CurrentClip.bHasExplicitLoopPolicy || !m_bPreviewLoop))
	{
		pModel->Set_AnimPaused(true);
		return;
	}
	m_iSynchronizedAnimationClipIndex = bLastClip ?
		0u : m_iSynchronizedAnimationClipIndex + 1u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
	const SYNCHRONIZED_ANIMATION_CLIP& NextClip =
		m_SynchronizedAnimationClips[m_iSynchronizedAnimationClipIndex];
	if (!Start_SynchronizedAnimationClip(
			m_iSynchronizedAnimationClipIndex, false))
	{
        m_strPreviewAnimationStatus =
            "Skill animation sequence stopped; next clip is unavailable: " +
            NextClip.strClipName;
        Reset_SynchronizedAnimationSequence();
        return;
    }
	m_strPreviewAnimationStatus = "Skill animation sequence: " +
        NextClip.strClipName +
        " (" + std::to_string(m_iSynchronizedAnimationClipIndex + 1u) +
        "/" + std::to_string(m_SynchronizedAnimationClips.size()) + ")";
}

void Client::CEffect_Tool::Reset_SynchronizedAnimationSequence()
{
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr != pModel)
        pModel->Set_AnimationSpeed(1.f);
	m_SynchronizedAnimationClips.clear();
	m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
    m_iSynchronizedAnimationTargetGeneration = 0u;
	m_ReconstructedSourceRuntimePoseBinding = {};
}
