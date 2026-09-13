#include "imgui.h"
#include "Effect_Tool_Internal.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Artist31470ShaderRegistry.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_DocumentRenderer.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Object.h"
#include "Effect_Playback.h"
#include "Effect_PresentationService.h"
#include "Effect_ReconstructedExecution.h"
#include "Effect_RuntimeAuthority.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Logic_DimensionMaster.h"
#include "MapEffectPresentationRuntime.h"
#include "Profiler.h"
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
#include "Model.h"
#include "Transform.h"

bool_t Client::CEffect_Tool::Try_StartArtist31470FullPreview()
{
	if (!Ensure_WorldPreviewObject())
	{
		Reset_ReconstructedSourceRuntimeTimeline();
		return false;
	}
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject)
	{
		Reset_ReconstructedSourceRuntimeTimeline();
		m_strPreviewStatus =
			"Artist Core F (33) EffectObject is unavailable.";
		return false;
	}
	const auto FailStart = [this, &pObject](std::string Reason)
	{
		pObject->Set_Playing(false);
		pObject->Set_Visible(false);
		Set_SynchronizedAnimationPaused(true);
		Reset_SynchronizedAnimationSequence();
		Reset_ReconstructedSourceRuntimeTimeline();
		m_bReconstructedDiagnosticActive = false;
		m_bReconstructedSourceRuntimeActive = false;
		m_pVisualPreviewProjection.reset();
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus = std::move(Reason);
		return false;
	};

	/* Resource preparation is synchronous and may consume several seconds.
	   Neither the previous Object nor its animation may absorb that wall time. */
	pObject->Set_Playing(false);
	Set_SynchronizedAnimationPaused(true);
	std::string Error;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> Preparation;
	if (!CEffectPresentationService::Acquire_ReconstructedArtist31470(
		m_pDevice, m_pContext, Preparation, Error))
	{
		return FailStart(
			"Artist Core F (33) cache failed: " + Error);
	}
	if (!Ensure_ArtistFSourceAuthoringOverlaySession())
	{
		return FailStart(m_strElementStatus.empty() ?
			"Artist F source-backed edit session could not be opened." :
			m_strElementStatus);
	}
	const bool_t bReusePreparedPreview =
		m_bReconstructedSourceRuntimeActive &&
		pObject->Is_ReconstructedSourceRuntimeActive() &&
		pObject->Get_ReconstructedRuntimePreparation().get() ==
			Preparation.get();
	/* A successful Tool start always obtains a fresh consumption receipt.
	   Reuse skips resource preparation, not the shared cache attach boundary. */
	if (!Synchronize_Artist31470FullPreview(Preparation))
	{
		return FailStart(m_strPreviewAnimationStatus);
	}
	if (!CEffectPresentationService::Stage_ReconstructedArtist31470Preview(
			pObject, Preparation, Error))
	{
		return FailStart(
			"Artist Core F (33) cache stage failed: " + Error);
	}
	EFFECT_ARTIST_31470_CACHE_PROBE CacheProbe;
	if (!CEffectPresentationService::Get_ReconstructedArtist31470CacheProbe(
		CacheProbe, Error))
	{
		return FailStart(
			"Artist Core F (33) cache receipt probe failed: " + Error);
	}
	const bool_t bToolReceiptMatches =
		CacheProbe.Current.Is_ExactCoreScope() &&
		CacheProbe.iToolPreviewConsumeCount > 0u &&
		CacheProbe.Current.Matches(
			CacheProbe.LastToolPreviewConsumption);
	const bool_t bGameplayReceiptExists =
		CacheProbe.iGameplayConsumeCount > 0u;
	if (!bToolReceiptMatches ||
		(bGameplayReceiptExists &&
		 !CacheProbe.Current.Matches(
			 CacheProbe.LastGameplayConsumption)))
	{
		return FailStart(
			"Artist Core F (33) Tool/gameplay cache identity diverged.");
	}
	m_bReconstructedDiagnosticActive = false;
	m_bReconstructedSourceRuntimeActive = true;
	m_pVisualPreviewProjection.reset();
	m_bPreviewPlaying = true;
	m_bPreviewVisibleRequested = true;
	m_fPreviewTimeSeconds = 0.f;
	Reset_ReconstructedSourceRuntimeTimeline();
	if (!Prepare_ReconstructedSourceRuntimeTransformHistory())
	{
		return FailStart(
			"Artist F historical anchor preparation failed: " +
			m_strPreviewAnimationStatus);
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		Preparation->Get_Program();
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		Preparation->Get_CatalogEntry();
	const std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT> pLegacyTuning =
		nullptr == pEntry ? nullptr : pEntry->Get_OccurrenceTuning();
	/* An empty source overlay is a true no-op only while the shared cache also
	   carries no legacy occurrence overrides.  If legacy overrides exist, an
	   intentionally empty overlay means reset-to-source and must restage. */
	const bool_t bRequiresSourceOverlayRestage =
		m_SourceAuthoringOverlayDocument.has_value() &&
		(!m_SourceAuthoringOverlayDocument->Entries.empty() ||
		 (nullptr != pLegacyTuning && !pLegacyTuning->Entries.empty()));
	if (nullptr != pProgram && bRequiresSourceOverlayRestage)
	{
		EFFECT_DOCUMENT_DESC SourceTransformDocument;
		std::string SourceTransformError;
		if (!CEffectReconstructedSourceRuntimeFactory::Build_Document(
				Preparation, SourceTransformDocument, SourceTransformError,
				EFFECT_RECONSTRUCTED_VISUAL_SCOPE::CORE_RENDERERS) ||
			!CEffectSourceAuthoringOverlayCodec::Apply_ToProjectedDocument(
				SourceTransformDocument, *pProgram,
				*m_SourceAuthoringOverlayDocument, SourceTransformError))
		{
			return FailStart(
				"Artist F source cue-local overlay validation failed: " +
				SourceTransformError);
		}
	}
	if (bRequiresSourceOverlayRestage && nullptr != pProgram &&
		m_SourceAuthoringOverlayDocument->strEffectAssetId ==
			pProgram->strRuntimeCatalogAssetId &&
		!CEffectPresentationService::
			Stage_ReconstructedSourceAuthoringOverlayPreview(
				m_pDevice, m_pContext, pObject, Preparation,
				*m_SourceAuthoringOverlayDocument, Error))
	{
		return FailStart(
			"Source-backed edit preview restage failed: " + Error);
	}
	else if (!m_SourceAuthoringOverlayDocument.has_value() &&
		m_OccurrenceTuningDocument.has_value() && nullptr != pProgram &&
		m_OccurrenceTuningDocument->strEffectAssetId ==
			pProgram->strRuntimeCatalogAssetId &&
		!CEffectPresentationService::Stage_ReconstructedOccurrenceTuningPreview(
			m_pDevice, m_pContext, pObject, Preparation,
			*m_OccurrenceTuningDocument, Error))
	{
		return FailStart(
			"Occurrence tuning preview restage failed: " + Error);
	}
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
		[this](const f32_t fSampleTimeSeconds,
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& strOutError)
		{
			return Build_ReconstructedSourceRuntimeTransformSample(
				fSampleTimeSeconds, OutSample, strOutError);
		};
	if (!pObject->Set_SampleTimeWithTransformHistory(
			0.f, TransformProvider, Error))
	{
		return FailStart(
			"Artist F historical zero-frame stage failed: " + Error);
	}
	m_bReconstructedSourceRuntimeStartPending = true;
	pObject->Set_Playing(false);
	/* Do not expose a zero-time frame whose bone palette predates the deferred
	   animation update.  The next Tool update refreshes the exact zero pose,
	   its normalized source anchors, and only then publishes visibility. */
	pObject->Set_Visible(false);
	m_strPreviewStatus =
		bReusePreparedPreview ?
		"Artist Core F (35 document / 33 visible) reused the shared CORE_RENDERERS cache" +
			std::string(bGameplayReceiptExists ?
				" and matches the latest gameplay F receipt; " :
				"; no gameplay F receipt exists yet; ") +
			"playback starts on the next update." :
		"Artist Core F (35 document / 33 visible) consumed the shared CORE_RENDERERS cache" +
			std::string(bGameplayReceiptExists ?
				" and matches the latest gameplay F receipt; " :
				"; no gameplay F receipt exists yet; ") +
			"playback starts on the next update.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ResetArtist31470PreviewIsolation()
{
	if (!m_bReconstructedSourceRuntimeActive &&
		!Try_StartArtist31470FullPreview())
	{
		return false;
	}
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_ReconstructedSourceRuntimeActive())
	{
		m_strPreviewStatus =
			"Artist Core F isolation requires the shared Core33 preview.";
		return false;
	}
	pObject->Reset_PreviewSubmissionIsolation();
	m_strPreviewStatus =
		"Artist Core F submission isolation: ALL 33 stable occurrences.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SetArtist31470PreviewFamilyIsolation(
	const EFFECT_GPU_RENDER_FAMILY eFamily)
{
	if (eFamily != EFFECT_GPU_RENDER_FAMILY::MESH &&
		eFamily != EFFECT_GPU_RENDER_FAMILY::SPRITE &&
		eFamily != EFFECT_GPU_RENDER_FAMILY::DECAL &&
		eFamily != EFFECT_GPU_RENDER_FAMILY::RIBBON)
	{
		m_strPreviewStatus = "Artist Core F isolation family is invalid.";
		return false;
	}
	if (!m_bReconstructedSourceRuntimeActive &&
		!Try_StartArtist31470FullPreview())
	{
		return false;
	}
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_ReconstructedSourceRuntimeActive())
	{
		m_strPreviewStatus =
			"Artist Core F isolation requires the shared Core33 preview.";
		return false;
	}
	EFFECT_PREVIEW_SUBMISSION_ISOLATION Isolation;
	Isolation.eKind = EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::FAMILY;
	Isolation.eFamily = eFamily;
	std::string Error;
	if (!pObject->Set_PreviewSubmissionIsolation(Isolation, Error))
	{
		m_strPreviewStatus = "Artist Core F family isolation failed: " + Error;
		return false;
	}
	m_strPreviewStatus = std::string("Artist Core F submission isolation: ") +
		ArtistCoreFamilyLabel(eFamily) + ".";
	return true;
}

void Client::CEffect_Tool::Reset_ArtistFPreparationFailureLatch()
{
	if (ARTIST_F_PREPARATION_STATE::FAILED ==
		m_eArtistFSourcePreparationState)
	{
		m_eArtistFSourcePreparationState =
			ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
		m_iArtistFSourcePreparationAttemptRevision = UINT64_MAX;
	}
	if (ARTIST_F_PREPARATION_STATE::FAILED ==
		m_eArtistFMaterialPreparationState)
	{
		m_eArtistFMaterialPreparationState =
			ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
		m_iArtistFMaterialPreparationAttemptRevision = UINT64_MAX;
	}
}

bool_t Client::CEffect_Tool::Ensure_ArtistFSourceSnapshotForAuthoring()
{
	const uint64_t iRuntimeRevision = CEffectCatalog::Get_RuntimeRevision();
	if (m_iArtistFSourceSnapshotRevision != iRuntimeRevision &&
		m_iArtistFSourcePreparationAttemptRevision != iRuntimeRevision)
	{
		m_eArtistFSourcePreparationState =
			ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
		m_iArtistFSourcePreparationAttemptRevision = UINT64_MAX;
		m_pArtistFSourcePreparation.reset();
		m_pArtistFSourceProjection.reset();
		m_iArtistFSourceSnapshotRevision = UINT64_MAX;
		m_ArtistFMaterialExecutionSnapshots.clear();
		m_iArtistFMaterialExecutionSnapshotRevision = UINT64_MAX;
		m_eArtistFMaterialPreparationState =
			ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
		m_iArtistFMaterialPreparationAttemptRevision = UINT64_MAX;
	}
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		CEffectCatalog::Find_RuntimeProgramEntry(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == pEntry ? nullptr : pEntry->Get_Program();
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> pVisualProgram =
		CEffectCatalog::Find_VisualProgram(ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	if (iRuntimeRevision == m_iArtistFSourceSnapshotRevision &&
		nullptr != m_pArtistFSourcePreparation &&
		nullptr != m_pArtistFSourceProjection &&
		nullptr != pEntry && nullptr != pProgram && nullptr != pVisualProgram &&
		m_pArtistFSourcePreparation->Get_CatalogEntry().get() == pEntry.get() &&
		m_pArtistFSourcePreparation->Get_Program().get() == pProgram.get() &&
		m_pArtistFSourceProjection->Is_Valid() &&
		m_pArtistFSourceProjection->Get_EffectAssetId() ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
		m_pArtistFSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 &&
		m_pArtistFSourceProjection->Get_ProgramSha256() ==
			pVisualProgram->strProgramSha256)
	{
		m_eArtistFSourcePreparationState =
			ARTIST_F_PREPARATION_STATE::READY;
		m_iArtistFSourcePreparationAttemptRevision = iRuntimeRevision;
		return true;
	}
	if (ARTIST_F_PREPARATION_STATE::FAILED ==
			m_eArtistFSourcePreparationState &&
		m_iArtistFSourcePreparationAttemptRevision == iRuntimeRevision)
	{
		return false;
	}
	m_iArtistFSourcePreparationAttemptRevision = iRuntimeRevision;
	m_eArtistFSourcePreparationState = ARTIST_F_PREPARATION_STATE::FAILED;
	m_pArtistFSourcePreparation.reset();
	m_pArtistFSourceProjection.reset();
	m_iArtistFSourceSnapshotRevision = UINT64_MAX;
	m_ArtistFMaterialExecutionSnapshots.clear();
	m_iArtistFMaterialExecutionSnapshotRevision = UINT64_MAX;
	m_eArtistFMaterialPreparationState =
		ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
	m_iArtistFMaterialPreparationAttemptRevision = UINT64_MAX;
	Engine::CProfilerScope PreparationProfile(
		CGameInstance::Get().Get_Profiler(),
		"EffectTool.ArtistF.SourcePreparation");

	std::string Error;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		StagedPreparation;
	if (!CEffectCatalog::Prepare_ReconstructedRuntimeProgram(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID, StagedPreparation, Error) ||
		nullptr == StagedPreparation ||
		nullptr == StagedPreparation->Get_CatalogEntry() ||
		nullptr == StagedPreparation->Get_Program())
	{
		m_strArtistFSourceSnapshotStatus = Error.empty() ?
			"Artist F source recipe is unavailable." : Error;
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument;
	if (!CEffectReconstructedSourceRuntimeFactory::Build_Document(
			StagedPreparation, StagedDocument, Error,
			EFFECT_RECONSTRUCTED_VISUAL_SCOPE::CORE_RENDERERS))
	{
		m_strArtistFSourceSnapshotStatus = Error.empty() ?
			"Artist F source recipe could not be opened." : Error;
		return false;
	}
	const std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT> pTuning =
		StagedPreparation->Get_CatalogEntry()->Get_OccurrenceTuning();
	if (nullptr != pTuning &&
		!CEffectOccurrenceTuningCodec::Apply_ToProjectedDocument(
			StagedDocument, *StagedPreparation->Get_Program(), *pTuning, Error))
	{
		m_strArtistFSourceSnapshotStatus = Error.empty() ?
			"Artist F saved source adjustments are invalid." : Error;
		return false;
	}

	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_CORPUS> pCorpus =
		CEffectCatalog::Find_VisualProgramCorpus();
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		StagedProjection;
	if (nullptr == pCorpus ||
		!CEffectVisualProgramCorpusCodec::Create_DocumentProjection(
			*pCorpus, StagedDocument, StagedProjection, Error) ||
		nullptr == StagedProjection || !StagedProjection->Is_Valid() ||
		StagedProjection->Get_EffectAssetId() !=
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID ||
		StagedProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
		StagedProjection->Get_ProgramSha256() !=
			pVisualProgram->strProgramSha256)
	{
		m_strArtistFSourceSnapshotStatus = Error.empty() ?
			"Artist F source recipe identity changed while opening." : Error;
		return false;
	}

	static constexpr std::array<size_t, 4u> EXPECTED_COUNTS{
		13u, 16u, 3u, 1u };
	std::array<size_t, 4u> Counts{};
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter :
		StagedPreparation->Get_Program()->Emitters)
	{
		EFFECT_GPU_RENDER_FAMILY eFamily = EFFECT_GPU_RENDER_FAMILY::END;
		if (!Emitter.bVisible ||
			!Try_ResolveArtistCoreFamily(Emitter.eRenderer, eFamily))
		{
			continue;
		}
		const size_t iFamily = eFamily == EFFECT_GPU_RENDER_FAMILY::MESH ? 0u :
			eFamily == EFFECT_GPU_RENDER_FAMILY::SPRITE ? 1u :
			eFamily == EFFECT_GPU_RENDER_FAMILY::DECAL ? 2u : 3u;
		const auto Element = std::find_if(
			StagedProjection->Get_Document().Elements.begin(),
			StagedProjection->Get_Document().Elements.end(),
			[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
			{ return Candidate.strElementId == Emitter.strSourceElementId; });
		if (Element == StagedProjection->Get_Document().Elements.end() ||
			!Element->bVisible)
		{
			m_strArtistFSourceSnapshotStatus =
				"Artist F source recipe is missing a visible family Element.";
			return false;
		}
		++Counts[iFamily];
	}
	if (Counts != EXPECTED_COUNTS)
	{
		m_strArtistFSourceSnapshotStatus =
			"Artist F source recipe family counts changed; import was not opened.";
		return false;
	}

	m_pArtistFSourcePreparation = std::move(StagedPreparation);
	m_pArtistFSourceProjection = std::move(StagedProjection);
	m_iArtistFSourceSnapshotRevision = iRuntimeRevision;
	m_iArtistFSourcePreparationAttemptRevision = iRuntimeRevision;
	m_eArtistFSourcePreparationState = ARTIST_F_PREPARATION_STATE::READY;
	m_ArtistFMaterialExecutionSnapshots.clear();
	m_iArtistFMaterialExecutionSnapshotRevision = UINT64_MAX;
	m_strArtistFSourceSnapshotStatus =
		"Artist F source recipe ready: Mesh 13, Sprite 16, Decal 3, Ribbon 1.";
	return true;
}

bool_t Client::CEffect_Tool::Ensure_ArtistFMaterialExecutionSnapshots()
{
	const uint64_t iRuntimeRevision = CEffectCatalog::Get_RuntimeRevision();
	if (m_iArtistFMaterialExecutionSnapshotRevision != iRuntimeRevision &&
		m_iArtistFMaterialPreparationAttemptRevision != iRuntimeRevision)
	{
		m_eArtistFMaterialPreparationState =
			ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
		m_iArtistFMaterialPreparationAttemptRevision = UINT64_MAX;
		m_ArtistFMaterialExecutionSnapshots.clear();
		m_iArtistFMaterialExecutionSnapshotRevision = UINT64_MAX;
	}
	if (ARTIST_F_PREPARATION_STATE::FAILED ==
			m_eArtistFMaterialPreparationState &&
		m_iArtistFMaterialPreparationAttemptRevision == iRuntimeRevision)
	{
		return false;
	}
	if (!Ensure_ArtistFSourceSnapshotForAuthoring() ||
		nullptr == m_pArtistFSourcePreparation ||
		nullptr == m_pArtistFSourceProjection ||
		nullptr == m_pArtistFSourcePreparation->Get_Program())
	{
		m_eArtistFMaterialPreparationState =
			ARTIST_F_PREPARATION_STATE::FAILED;
		m_iArtistFMaterialPreparationAttemptRevision = iRuntimeRevision;
		return false;
	}
	if (m_iArtistFMaterialExecutionSnapshotRevision ==
			m_iArtistFSourceSnapshotRevision &&
		!m_ArtistFMaterialExecutionSnapshots.empty())
	{
		m_eArtistFMaterialPreparationState =
			ARTIST_F_PREPARATION_STATE::READY;
		m_iArtistFMaterialPreparationAttemptRevision = iRuntimeRevision;
		return true;
	}
	m_eArtistFMaterialPreparationState = ARTIST_F_PREPARATION_STATE::FAILED;
	m_iArtistFMaterialPreparationAttemptRevision = iRuntimeRevision;
	Engine::CProfilerScope PreparationProfile(
		CGameInstance::Get().Get_Profiler(),
		"EffectTool.ArtistF.MaterialPreparation");

	std::unordered_map<std::string, EFFECT_MATERIAL_EXECUTION_DESC> Staged;
	std::string Error;
	if (!CEffectDocumentRenderer::
		Bake_ReconstructedMaterialExecutionSnapshots(
			m_pDevice, m_pContext,
			m_pArtistFSourceProjection->Get_Document(),
			m_pArtistFSourcePreparation, Staged, Error))
	{
		m_strArtistFSourceSnapshotStatus = Error.empty() ?
			"Artist F Track A material execution snapshots could not be baked." :
			Error;
		return false;
	}

	size_t iCoreCount = 0u;
	size_t iEnabledCount = 0u;
	size_t iRuntimeMaterialCount = 0u;
	size_t iArtistVisualCount = 0u;
	size_t iLocalDecalCount = 0u;
	size_t iFiniteCommonCount = 0u;
	size_t iFailClosedCount = 0u;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter :
		m_pArtistFSourcePreparation->Get_Program()->Emitters)
	{
		EFFECT_GPU_RENDER_FAMILY eFamily = EFFECT_GPU_RENDER_FAMILY::END;
		if (!Emitter.bVisible ||
			!Try_ResolveArtistCoreFamily(Emitter.eRenderer, eFamily))
		{
			continue;
		}
		const auto Snapshot = Staged.find(Emitter.strSourceElementId);
		if (Snapshot == Staged.end())
		{
			m_strArtistFSourceSnapshotStatus =
				"Artist F Track A material bake lost a Core33 Element identity.";
			return false;
		}
		++iCoreCount;
		if (!Emitter.strMaterialOccurrenceId.has_value())
		{
			m_strArtistFSourceSnapshotStatus =
				"Artist F Track A material row lost its stable occurrence ID.";
			return false;
		}
		const auto Registry = Find_Artist31470ShaderRegistry(
			Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId);
		if (!Registry.has_value() ||
			!Validate_Artist31470ShaderRegistryEmitterIdentity(
				Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
				Emitter.strSourceElementId, Emitter.strSourceEmitterPath))
		{
			m_strArtistFSourceSnapshotStatus =
				"Artist F Track A shader-registry identity changed.";
			return false;
		}
		if (!Snapshot->second.bEnabled)
		{
			if (Registry->eBackend ==
					EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON &&
				Registry->eFidelity ==
					EFFECT_ARTIST31470_SHADER_FIDELITY::BOUNDED_EXPLICIT &&
				Registry->bDrawAdmitted && Emitter.Row.iOrder == 17u)
			{
				++iFiniteCommonCount;
			}
			else if (Registry->eBackend ==
					EFFECT_ARTIST31470_SHADER_BACKEND::NONE &&
				Registry->eFidelity ==
					EFFECT_ARTIST31470_SHADER_FIDELITY::UNRESOLVED_FAIL_CLOSED &&
				!Registry->bDrawAdmitted)
			{
				++iFailClosedCount;
			}
			else
			{
				m_strArtistFSourceSnapshotStatus =
					"Artist F disabled material row is neither FiniteCommon nor fail-closed.";
				return false;
			}
			continue;
		}
		++iEnabledCount;
		switch (Snapshot->second.eBackend)
		{
		case EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2:
			++iRuntimeMaterialCount;
			break;
		case EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4:
			++iArtistVisualCount;
			break;
		case EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL:
			++iLocalDecalCount;
			break;
		case EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1:
		case EFFECT_MATERIAL_EXECUTION_BACKEND::GENERIC:
		case EFFECT_MATERIAL_EXECUTION_BACKEND::END:
		default:
			m_strArtistFSourceSnapshotStatus =
				"Artist F Track A material bake enabled an unsupported backend.";
			return false;
		}
	}
	if (iCoreCount != 33u || iEnabledCount != 28u ||
		iRuntimeMaterialCount + iLocalDecalCount != 18u ||
		iArtistVisualCount != 10u || iLocalDecalCount != 2u ||
		iFiniteCommonCount != 1u || iFailClosedCount != 4u)
	{
		m_strArtistFSourceSnapshotStatus =
			"Artist F Track A material denominator changed; expected typed 28 (RuntimeMaterialV2 family 18 including LocalDecal 2, ArtistVisualV4 10), FiniteCommon 1, fail-closed 4.";
		return false;
	}

	m_ArtistFMaterialExecutionSnapshots = std::move(Staged);
	m_iArtistFMaterialExecutionSnapshotRevision =
		m_iArtistFSourceSnapshotRevision;
	m_iArtistFMaterialPreparationAttemptRevision = iRuntimeRevision;
	m_eArtistFMaterialPreparationState = ARTIST_F_PREPARATION_STATE::READY;
	m_strArtistFSourceSnapshotStatus =
		"Artist F Track A material snapshots ready: typed 28, FiniteCommon bounded 1, fail-closed 4.";
	return true;
}

bool_t Client::CEffect_Tool::Ensure_ArtistFSourceAuthoringOverlaySession()
{
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		CEffectCatalog::Find_RuntimeProgramEntry(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == pEntry ? nullptr : pEntry->Get_Program();
	if (nullptr == pEntry || nullptr == pProgram ||
		pProgram->strRuntimeCatalogAssetId != ARTIST_F_VISUAL_PROGRAM_ASSET_ID)
	{
		m_strElementStatus =
			"Artist F Track A source Program is unavailable.";
		return false;
	}
	if (m_SourceAuthoringOverlayDocument.has_value() &&
		m_SourceAuthoringOverlayDocument->strEffectAssetId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID)
	{
		std::string Error;
		if (!CEffectSourceAuthoringOverlayCodec::Validate_AgainstProgram(
				*m_SourceAuthoringOverlayDocument, *pProgram, Error))
		{
			m_strElementStatus = Error;
			return false;
		}
		if (m_SourceAuthoringOverlayPath.empty())
		{
			m_strElementStatus =
				"Artist F source-backed edit path is unavailable.";
			return false;
		}
		std::error_code FileError;
		const bool_t bExists = std::filesystem::is_regular_file(
			m_SourceAuthoringOverlayPath, FileError);
		if (FileError)
		{
			m_strElementStatus =
				"Artist F saved overlay path could not be inspected: " +
				FileError.message();
			return false;
		}
		if (!m_bOccurrenceTuningDirty &&
			!m_bOccurrenceTransformDraftDirty)
		{
			if (!bExists &&
				!m_strSourceAuthoringOverlayBaselineCanonical.empty())
			{
				m_strElementStatus =
					"Artist F saved overlay disappeared; use Reload Saved before editing.";
				return false;
			}
			if (bExists &&
				m_strSourceAuthoringOverlayBaselineCanonical.empty() &&
				m_bSourceAuthoringOverlayNeedsInitialSave)
			{
				m_strElementStatus =
					"Artist F saved overlay appeared; use Reload Saved before editing.";
				return false;
			}
			if (bExists)
			{
				EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Disk;
				if (!CEffectSourceAuthoringOverlayCodec::Load(
						m_SourceAuthoringOverlayPath, Disk, Error) ||
					!CEffectSourceAuthoringOverlayCodec::
						Validate_AgainstProgram(Disk, *pProgram, Error))
				{
					m_strElementStatus = Error.empty() ?
						"Artist F saved overlay could not be reloaded." : Error;
					return false;
				}
				const std::string DiskCanonical =
					CEffectSourceAuthoringOverlayCodec::Serialize(Disk);
				if (DiskCanonical !=
					m_strSourceAuthoringOverlayBaselineCanonical)
				{
					m_strElementStatus =
						"Artist F saved overlay changed on disk; use Reload Saved before editing.";
					return false;
				}
				m_bSourceAuthoringOverlayNeedsInitialSave = false;
			}
		}
		return true;
	}
	if (m_bOccurrenceTuningDirty || m_bOccurrenceTransformDraftDirty)
	{
		m_strElementStatus =
			"Save or Reload the current Effect Detail changes before loading Artist F.";
		return false;
	}

	const std::filesystem::path Path = CProjectDataRoot::Resolve(
		std::filesystem::path(ARTIST_F_SOURCE_AUTHORING_OVERLAY_PATH));
	if (Path.empty())
	{
		m_strElementStatus =
			"Artist F source-backed edit path escaped the project Data root.";
		return false;
	}

	EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Staged;
	Staged.strEffectAssetId = ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	Staged.strSourceProgramSha256 = pProgram->Identity.strProgramSha256;
	Staged.SupplementalDocument =
		CEffectSourceAuthoringOverlayCodec::Create_EmptySupplementalDocument(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	std::string Baseline;
	std::string Error;
	std::error_code FileError;
	const bool_t bExists = std::filesystem::is_regular_file(Path, FileError);
	if (FileError)
	{
		m_strElementStatus =
			"Artist F saved overlay path could not be inspected: " +
			FileError.message();
		return false;
	}
	if (bExists)
	{
		if (!CEffectSourceAuthoringOverlayCodec::Load(Path, Staged, Error) ||
			!CEffectSourceAuthoringOverlayCodec::Validate_AgainstProgram(
				Staged, *pProgram, Error))
		{
			m_strElementStatus = Error.empty() ?
				"Artist F saved overlay could not be loaded." : Error;
			return false;
		}
		Baseline = CEffectSourceAuthoringOverlayCodec::Serialize(Staged);
	}
	else
	{
		const std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT>
			pLegacyTuning = pEntry->Get_OccurrenceTuning();
		if (nullptr != pLegacyTuning)
		{
			if (!CEffectSourceAuthoringOverlayCodec::
					Migrate_FromOccurrenceTuning(
						*pProgram, *pLegacyTuning, Staged, Error))
			{
				m_strElementStatus = Error.empty() ?
					"Artist F legacy tuning migration failed." : Error;
				return false;
			}
		}
		else if (!CEffectSourceAuthoringOverlayCodec::Validate_AgainstProgram(
				Staged, *pProgram, Error))
		{
			m_strElementStatus = Error;
			return false;
		}
	}

	m_OccurrenceTuningDocument.reset();
	m_OccurrenceTuningPath.clear();
	m_strOccurrenceTuningBaselineCanonical.clear();
	m_SourceAuthoringOverlayDocument = std::move(Staged);
	m_SourceAuthoringOverlayPath = Path;
	m_strSourceAuthoringOverlayBaselineCanonical = std::move(Baseline);
	m_bSourceAuthoringOverlayNeedsInitialSave = !bExists;
	m_bOccurrenceTuningDirty = !bExists &&
		!m_SourceAuthoringOverlayDocument->Entries.empty();
	m_bOccurrenceTransformDraftDirty = false;
	return true;
}

bool_t Client::CEffect_Tool::Try_ApplyArtistFTrackASeedData(
	const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
	const EFFECT_ELEMENT_DESC& SourceElement,
	EFFECT_ELEMENT_DESC& InOutElement,
	std::string& strOutError) const
{
	EFFECT_GPU_RENDER_FAMILY eSourceFamily = EFFECT_GPU_RENDER_FAMILY::END;
	if (!Emitter.bVisible ||
		!Try_ResolveArtistCoreFamily(Emitter.eRenderer, eSourceFamily))
	{
		strOutError = "Artist F Track A seed is outside the visible Core33 set.";
		return false;
	}
	const EFFECT_AUTHORING_FAMILY eExpectedFamily =
		eSourceFamily == EFFECT_GPU_RENDER_FAMILY::MESH ?
			EFFECT_AUTHORING_FAMILY::MESH_PARTICLE :
		eSourceFamily == EFFECT_GPU_RENDER_FAMILY::SPRITE ?
			EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE :
		eSourceFamily == EFFECT_GPU_RENDER_FAMILY::DECAL ?
			EFFECT_AUTHORING_FAMILY::LOCAL_DECAL :
		eSourceFamily == EFFECT_GPU_RENDER_FAMILY::RIBBON ?
			EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON :
			EFFECT_AUTHORING_FAMILY::END;
	if (Resolve_AuthoringFamily(InOutElement) != eExpectedFamily ||
		SourceElement.strElementId != Emitter.strSourceElementId)
	{
		strOutError =
			"Artist F Track A seed no longer matches its authored Family/Element identity.";
		return false;
	}
	if (!Emitter.strMaterialOccurrenceId.has_value())
	{
		strOutError =
			"Artist F Track A seed has no stable material occurrence ID.";
		return false;
	}
	const auto Registry = Find_Artist31470ShaderRegistry(
		Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId);
	if (!Registry.has_value() ||
		!Validate_Artist31470ShaderRegistryEmitterIdentity(
			Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
			Emitter.strSourceElementId, Emitter.strSourceEmitterPath))
	{
		strOutError =
			"Artist F Track A seed no longer matches the shader registry.";
		return false;
	}

	if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind)
	{
		uint64_t iFixedBurstCount = 0u;
		for (const EFFECT_RUNTIME_PROGRAM_BURST& Burst : Emitter.Timing.Bursts)
		{
			if (!std::isfinite(Burst.fTimeSeconds) ||
				std::abs(Burst.fTimeSeconds) > 1.0e-9 ||
				Burst.iCountMinimum != Burst.iCountMaximum)
			{
				strOutError =
					"Artist F Track A burst is not a fixed t=0 burst representable by the authored Particle Detail.";
				return false;
			}
			iFixedBurstCount += Burst.iCountMaximum;
		}
		if (iFixedBurstCount > (std::numeric_limits<uint32_t>::max)())
		{
			strOutError = "Artist F Track A fixed burst count overflowed uint32.";
			return false;
		}
		InOutElement.Detail.Particle.iBurstCount =
			static_cast<uint32_t>(iFixedBurstCount);
		InOutElement.Detail.Particle.bLocalSpace = Emitter.bLocalSpace;
		InOutElement.Detail.Particle.iRandomSeed =
			Emitter.Random.iEmitterRandomSeed;
		InOutElement.Detail.Particle.iMaxParticles = (std::max)(
			InOutElement.Detail.Particle.iMaxParticles,
			Emitter.iOperationalMaxParticles);
	}
	if (EFFECT_AUTHORING_FAMILY::MESH_PARTICLE == eExpectedFamily)
	{
		const EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Geometry =
			SourceElement.SourceRecipe.GeometryBinding;
		const auto ModelBinding = std::find_if(
			InOutElement.ResourceBindings.begin(),
			InOutElement.ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			});
		constexpr f32_t ARTIST_F_MODEL_PRE_SCALE = 0.01f;
		if (!Geometry.bEnabled ||
			ModelBinding == InOutElement.ResourceBindings.end() ||
			ModelBinding->strAssetId != Geometry.strAssetId ||
			!std::isfinite(Geometry.fCarrierGeometryPreScale) ||
			std::abs(Geometry.fCarrierGeometryPreScale -
				ARTIST_F_MODEL_PRE_SCALE) > 1.0e-7f)
		{
			strOutError =
				"Artist F MeshParticle lost its WModel geometry pre-scale 0.01 contract.";
			return false;
		}
		InOutElement.Detail.Mesh.fModelPreScale =
			Geometry.fCarrierGeometryPreScale;
	}

	/* The current unified baseline already flattened root snapshot yaw and the
	   five follow parents into Detail.Transform. Re-enabling either attachment
	   here would apply the Artist import basis/weapon basis a second time. The
	   P0 authored result therefore keeps exactly one emit-start baked basis. */
	InOutElement.ActionCueAttachment = {};
	InOutElement.TransformInheritance = {};
	if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind)
	{
		uint32_t iRequiredMask = 0u;
		float4_t vSourceStart{};
		float4_t vSourceEnd{};
		switch (Emitter.Row.iOrder)
		{
		case 13u:
		case 14u:
			iRequiredMask = 0x0fu;
			vSourceStart = { 1.f, 0.f, 1.f, 1.f };
			vSourceEnd = { 1.f, 2.f, 1.f, 1.f };
			break;
		case 25u:
			iRequiredMask = 0x06u;
			vSourceStart = { 1.f, 1.f, 0.f, 0.f };
			vSourceEnd = { 1.f, 0.5f, 0.f, 0.f };
			break;
		case 29u:
			iRequiredMask = 0x06u;
			vSourceStart = { 1.f, 1.f, 1.f, 0.f };
			vSourceEnd = { 1.f, 0.5f, 1.f, 0.f };
			break;
		default:
			break;
		}
		f32_t* pStart =
			&InOutElement.Detail.Particle.vDynamicParameterStart.x;
		f32_t* pEnd = &InOutElement.Detail.Particle.vDynamicParameterEnd.x;
		const f32_t* pSourceStart = &vSourceStart.x;
		const f32_t* pSourceEnd = &vSourceEnd.x;
		for (uint32_t iComponent = 0u; iComponent < 4u; ++iComponent)
		{
			const uint32_t iBit = 1u << iComponent;
			if (0u == (iRequiredMask & iBit) ||
				0u != (InOutElement.Detail.Particle.
					iDynamicParameterComponentMask & iBit))
			{
				continue;
			}
			pStart[iComponent] = pSourceStart[iComponent];
			pEnd[iComponent] = pSourceEnd[iComponent];
		}
		InOutElement.Detail.Particle.iDynamicParameterComponentMask |=
			iRequiredMask;
	}

	const auto MaterialSnapshot = m_ArtistFMaterialExecutionSnapshots.find(
		SourceElement.strElementId);
	if (MaterialSnapshot == m_ArtistFMaterialExecutionSnapshots.end())
	{
		strOutError =
			"Artist F Track A material snapshot no longer matches its source Element identity.";
		return false;
	}
	if (MaterialSnapshot->second.bEnabled)
	{
		if ((Registry->eBackend !=
				EFFECT_ARTIST31470_SHADER_BACKEND::RUNTIME_V2 &&
			 Registry->eBackend !=
				EFFECT_ARTIST31470_SHADER_BACKEND::ARTIST_V4) ||
			!Registry->bDrawAdmitted)
		{
			strOutError =
				"Artist F typed Material snapshot disagrees with the shader registry.";
			return false;
		}
		EFFECT_MATERIAL_EXECUTION_DESC StagedExecution =
			MaterialSnapshot->second;
		/* Re-running Upgrade refreshes the Track A recipe, but an artist's DDS
		   override remains authoritative when its semantic lane identity still
		   exists.  Generic ResourceBindings are independent and stay untouched. */
		if (InOutElement.Material.Execution.bEnabled)
		{
			for (EFFECT_MATERIAL_TEXTURE_LANE_DESC& StagedLane :
				StagedExecution.TextureLanes)
			{
				const auto ExistingLane = std::find_if(
					InOutElement.Material.Execution.TextureLanes.begin(),
					InOutElement.Material.Execution.TextureLanes.end(),
					[&StagedLane](
						const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Candidate)
					{ return Candidate.strLaneId == StagedLane.strLaneId; });
				if (ExistingLane !=
						InOutElement.Material.Execution.TextureLanes.end() &&
					!ExistingLane->strAssetId.empty())
				{
					StagedLane.strAssetId = ExistingLane->strAssetId;
				}
			}
		}
		InOutElement.Material.Execution = std::move(StagedExecution);
		/* The typed snapshot owns t#/s#/channel execution.  Keep the source path
		   as provenance, but never execute the older SourceMaterial profile in
		   parallel. */
		InOutElement.Material.SourceMaterial = {};
		if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind)
		{
			const uint32_t iConsumedMask =
				InOutElement.Material.Execution.iDynamicConsumedMask & 0x0fu;
			f32_t* pStart =
				&InOutElement.Detail.Particle.vDynamicParameterStart.x;
			f32_t* pEnd =
				&InOutElement.Detail.Particle.vDynamicParameterEnd.x;
			for (uint32_t iComponent = 0u; iComponent < 4u; ++iComponent)
			{
				const uint32_t iBit = 1u << iComponent;
				if (0u == (iConsumedMask & iBit) ||
					0u != (InOutElement.Detail.Particle.
						iDynamicParameterComponentMask & iBit))
				{
					continue;
				}
				/* Track A has no action-cue DynamicParameter sample for these
				   rows. UE's missing-payload carrier is constant one; seed it once
				   and preserve any later artist-authored component values. */
				pStart[iComponent] = 1.f;
				pEnd[iComponent] = 1.f;
			}
			InOutElement.Detail.Particle.iDynamicParameterComponentMask |=
				iConsumedMask;
		}
	}
	else if (Registry->eBackend ==
			EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON &&
		Registry->eFidelity ==
			EFFECT_ARTIST31470_SHADER_FIDELITY::BOUNDED_EXPLICIT &&
		Registry->bDrawAdmitted && Emitter.Row.iOrder == 17u)
	{
		/* #17 is the one valid FiniteCommon row. Unlike the 28 typed snapshots,
		   its bounded missile-trail evaluator is the ordinary SourceMaterial
		   profile 13. Copy that self-contained authored recipe instead of
		   degrading it to generic profile 0, while preserving same-name DDS
		   overrides already made in the editor. */
		const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
			SourceElement.Material.SourceMaterial;
		if (!SourceMaterial.bEnabled ||
			SourceMaterial.strRuntimeShaderProfileId !=
				"effect.ue3.missiletrail-01.v1")
		{
			strOutError =
				"Artist F #17 lost its bounded FiniteCommon missile-trail profile.";
			return false;
		}
		EFFECT_MATERIAL_DESC StagedMaterial = SourceElement.Material;
		StagedMaterial.Execution = {};
		for (EFFECT_NAMED_TEXTURE_DESC& StagedTexture :
			StagedMaterial.SourceMaterial.Textures)
		{
			const auto ExistingTexture = std::find_if(
				InOutElement.Material.SourceMaterial.Textures.begin(),
				InOutElement.Material.SourceMaterial.Textures.end(),
				[&StagedTexture](const EFFECT_NAMED_TEXTURE_DESC& Candidate)
				{
					return Candidate.strName == StagedTexture.strName;
				});
			if (ExistingTexture !=
					InOutElement.Material.SourceMaterial.Textures.end() &&
				!ExistingTexture->strAssetId.empty())
			{
				StagedTexture.strAssetId = ExistingTexture->strAssetId;
			}
		}
		InOutElement.Material = std::move(StagedMaterial);
	}
	else if (Registry->eBackend ==
			EFFECT_ARTIST31470_SHADER_BACKEND::NONE &&
		Registry->eFidelity ==
			EFFECT_ARTIST31470_SHADER_FIDELITY::UNRESOLVED_FAIL_CLOSED &&
		!Registry->bDrawAdmitted &&
		(Emitter.Row.iOrder == 1u || Emitter.Row.iOrder == 16u ||
		 Emitter.Row.iOrder == 26u || Emitter.Row.iOrder == 33u))
	{
		/* SceneColor/depth/fog/aux-MRT owners must not silently become a generic
		   white or distortion draw. Keep their seed resources for later manual
		   reconstruction, but make the automatic Track A import fail closed. */
		InOutElement.Material.Execution = {};
		InOutElement.Material.Execution.bFailClosed = true;
		InOutElement.Material.SourceMaterial = {};
		InOutElement.bVisible = false;
	}
	else
	{
		strOutError =
			"Artist F disabled Material snapshot has no admitted FiniteCommon/fail-closed policy.";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool::Try_CreateArtistFUnifiedDraft()
{
	if (Has_UnsavedWork())
	{
		m_strElementStatus =
			"Save or discard the current Effect changes before creating the Artist F Unified draft.";
		return false;
	}
	const std::filesystem::path Path = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"Authored" /
		(std::filesystem::path(ARTIST_F_UNIFIED_EFFECT_ASSET_ID).wstring() +
		 L".effect.json"));
	if (Path.empty())
	{
		m_strElementStatus =
			"Artist F Unified draft path escaped Data/Effects/Authored.";
		return false;
	}
	if (std::filesystem::is_regular_file(Path))
	{
		m_strElementStatus =
			"Artist F Unified Effect already exists. Use Load Effect; migration never overwrites it.";
		return false;
	}
	if (!Ensure_ArtistFSourceSnapshotForAuthoring() ||
		!Ensure_ArtistFMaterialExecutionSnapshots() ||
		nullptr == m_pArtistFSourcePreparation ||
		nullptr == m_pArtistFSourceProjection ||
		nullptr == m_pArtistFSourcePreparation->Get_Program())
	{
		m_strElementStatus = m_strArtistFSourceSnapshotStatus.empty() ?
			"Artist F source recipe is unavailable." :
			m_strArtistFSourceSnapshotStatus;
		return false;
	}

	const EFFECT_DOCUMENT_DESC& SourceDocument =
		m_pArtistFSourceProjection->Get_Document();
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		m_pArtistFSourcePreparation->Get_Program();
	struct PENDING_ARTIST_F_UNIFIED_ELEMENT final
	{
		EFFECT_ELEMENT_DESC LoweredElement;
		EFFECT_GENERIC_AUTHORED_STARTING_BAKE_REQUEST BakeRequest;
		std::string strSourceElementId;
		std::string strFollowAnchorSlotId;
	};
	std::array<size_t, 4u> FamilyOrdinals{};
	std::set<std::string, std::less<>> TargetIds;
	std::vector<PENDING_ARTIST_F_UNIFIED_ELEMENT> PendingElements;
	PendingElements.reserve(33u);
	size_t iFollowElementCount = 0u;
	size_t iStrictPresetElementCount = 0u;
	std::string strFirstSourceElementId;
	std::string Error;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : pProgram->Emitters)
	{
		EFFECT_GPU_RENDER_FAMILY eFamily = EFFECT_GPU_RENDER_FAMILY::END;
		if (!Emitter.bVisible ||
			!Try_ResolveArtistCoreFamily(Emitter.eRenderer, eFamily))
		{
			continue;
		}
		const auto Source = std::find_if(
			SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
			[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
			{ return Candidate.strElementId == Emitter.strSourceElementId; });
		if (Source == SourceDocument.Elements.end() || !Source->bVisible)
		{
			m_strElementStatus =
				"Artist F source Element disappeared before starting-state capture: " +
				Emitter.strSourceElementId;
			return false;
		}

		PENDING_ARTIST_F_UNIFIED_ELEMENT Pending;
		Pending.strSourceElementId = Emitter.strSourceElementId;
		if (!Try_NarrowRuntimeFloat3(
				Emitter.CueLocalTransform.vPosition,
				Pending.BakeRequest.CueLocalTransform.vPosition) ||
			!Try_NarrowRuntimeFloat3(
				Emitter.CueLocalTransform.vRotationDegrees,
				Pending.BakeRequest.CueLocalTransform.vRotationDegrees) ||
			!Try_NarrowRuntimeFloat3(
				Emitter.CueLocalTransform.vScale,
				Pending.BakeRequest.CueLocalTransform.vScale) ||
			!Try_NarrowRuntimeFloat3(
				Emitter.DetailTransform.vPosition,
				Pending.BakeRequest.EmitterLocalTransform.vPosition) ||
			!Try_NarrowRuntimeFloat3(
				Emitter.DetailTransform.vRotationDegrees,
				Pending.BakeRequest.EmitterLocalTransform.vRotationDegrees) ||
			!Try_NarrowRuntimeFloat3(
				Emitter.DetailTransform.vScale,
				Pending.BakeRequest.EmitterLocalTransform.vScale) ||
			(Emitter.RendererRuntimeConfig.Mesh.has_value() &&
			 !Try_NarrowRuntimeFloat3(
				 Emitter.RendererRuntimeConfig.Mesh->
					 vSourceTypeDataRotationDegrees,
				 Pending.BakeRequest.vSourceTypeDataRotationDegrees)))
		{
			m_strElementStatus =
				"Artist F runtime emitter transform is not representable: " +
				Emitter.strSourceElementId;
			return false;
		}
		Pending.BakeRequest.fScheduleStartDelaySeconds =
			Source->Detail.Timing.fStartDelaySeconds;
		Pending.BakeRequest.fScheduleLifeTimeSeconds =
			Source->Detail.Timing.fLifeTimeSeconds;
		Pending.BakeRequest.fEmitterDelaySeconds =
			Source->SourceRecipe.fEmitterDelaySeconds;
		Pending.BakeRequest.fEmitterDurationSeconds =
			Source->SourceRecipe.fEmitterDurationSeconds;
		Pending.BakeRequest.iEmitterLoopCount =
			Source->SourceRecipe.iEmitterLoopCount;
		Pending.BakeRequest.bAttachmentEnabled =
			Source->ActionCueAttachment.bEnabled;
		Pending.BakeRequest.bFollowAttachment =
			Source->ActionCueAttachment.bFollow;
		Pending.BakeRequest.fSnapshotRootSourceBasisYawDegrees =
			Source->ActionCueAttachment.fSnapshotRootSourceBasisYawDegrees;
		Pending.BakeRequest.bTransformInheritanceEnabled =
			Source->TransformInheritance.bEnabled;
		if (Pending.BakeRequest.bFollowAttachment)
		{
			const auto Binding = std::find_if(
				m_pArtistFSourcePreparation->Get_AnchorRequests().begin(),
				m_pArtistFSourcePreparation->Get_AnchorRequests().end(),
				[&Emitter](const EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Candidate)
				{ return Candidate.strOwnerEmitterId == Emitter.Row.strId; });
			if (!Pending.BakeRequest.bAttachmentEnabled ||
				Source->ActionCueAttachment.strRuntimeAnchorSlotId.empty() ||
				Binding ==
					m_pArtistFSourcePreparation->Get_AnchorRequests().end() ||
				!Binding->Request.bFollow ||
				Binding->Request.strRuntimeAnchorSlotId !=
					Source->ActionCueAttachment.strRuntimeAnchorSlotId ||
				Binding->Request.strRuntimeBoneName !=
					Source->ActionCueAttachment.strRuntimeBoneName)
			{
				m_strElementStatus =
					"Artist F follow Element has no exact typed anchor binding: " +
					Emitter.strSourceElementId;
				return false;
			}
			Pending.strFollowAnchorSlotId =
				Binding->Request.strRuntimeAnchorSlotId;
			++iFollowElementCount;
		}

		EFFECT_DOCUMENT_DESC OneElement;
		const std::string strStrictOccurrenceId =
			Emitter.strMaterialOccurrenceId.value_or(std::string{});
		const EFFECT_VISUAL_PROGRAM_ROW* pStrictVisualRow =
			strStrictOccurrenceId.empty() ? nullptr :
			m_pArtistFSourceProjection->Find_RowByOccurrenceId(
				strStrictOccurrenceId);
		const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pStrictSupplemental =
			strStrictOccurrenceId.empty() ? nullptr :
			m_pArtistFSourceProjection->
				Find_SupplementalElementByOccurrenceId(strStrictOccurrenceId);
		const bool_t bRequiresStrictPreset =
			strStrictOccurrenceId == "source-active-003" ||
			strStrictOccurrenceId == "source-active-020" ||
			strStrictOccurrenceId == "source-active-021";
		if (bRequiresStrictPreset && nullptr == pStrictVisualRow &&
			nullptr == pStrictSupplemental)
		{
			m_strElementStatus =
				"Artist F typed Decal/Ribbon source row lost its admitted preset identity.";
			return false;
		}
		if (nullptr != pStrictVisualRow && nullptr != pStrictSupplemental)
		{
			m_strElementStatus =
				"Artist F source Element matched more than one admitted row.";
			return false;
		}
		if (nullptr == pStrictVisualRow && nullptr == pStrictSupplemental)
		{
			if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
					SourceDocument, Emitter.strSourceElementId,
					ARTIST_F_UNIFIED_EFFECT_ASSET_ID, OneElement, Error))
			{
				m_strElementStatus = Error.empty() ?
					"Artist F source Element could not be lowered." : Error;
				return false;
			}
		}
		else if (m_bActiveDocumentDrawable)
		{
			m_strDetailStatus =
				"Applied and saved Authored; the preview is ready but hidden. Use Play All or Restart Preview.";
		}
		else
		{
			if (nullptr != pStrictVisualRow &&
				!pStrictVisualRow->TargetIdentity.has_value())
			{
				m_strElementStatus =
					"Artist F admitted source Element has no target identity.";
				return false;
			}
			EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_REQUEST Request;
			Request.strEffectAssetId = ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
			Request.strOccurrenceId = strStrictOccurrenceId;
			Request.strRowSha256 = nullptr != pStrictVisualRow ?
				pStrictVisualRow->strRowSha256 : pStrictSupplemental->strRowSha256;
			Request.strTargetElementId = nullptr != pStrictVisualRow ?
				pStrictVisualRow->TargetIdentity->strTargetElementId :
				pStrictSupplemental->TargetIdentity.strTargetElementId;
			Request.strSourceRecordId = nullptr != pStrictVisualRow ?
				pStrictVisualRow->SourceIdentity.strSourceRecordId :
				pStrictSupplemental->strSourceRecordId;
			if (Request.strTargetElementId != Emitter.strSourceElementId)
			{
				m_strElementStatus =
					"Artist F admitted source row target no longer matches its Core Element.";
				return false;
			}
			EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_STAGE Stage;
			if (!CEffectVisualProgramCorpusCodec::Build_ElementAuthoringPresetStage(
					m_pArtistFSourceProjection, Request, Stage, Error))
			{
				m_strElementStatus = Error.empty() ?
					"Artist F admitted source Element could not be staged." : Error;
				return false;
			}
			OneElement.strEffectAssetId = ARTIST_F_UNIFIED_EFFECT_ASSET_ID;
			OneElement.strDisplayName = "Artist F Unified starting Element";
			OneElement.Elements.push_back(std::move(Stage.Element));
			++iStrictPresetElementCount;
		}
		if (OneElement.Elements.size() != 1u)
		{
			m_strElementStatus = Error.empty() ?
				"Artist F source Element could not be lowered." : Error;
			return false;
		}
		Pending.LoweredElement = std::move(OneElement.Elements.front());
		const std::string StableId = StableUnifiedElementId(
			eFamily, Emitter.Row.strId);
		if (StableId.empty() || !TargetIds.insert(StableId).second)
		{
			m_strElementStatus =
				"Artist F Unified Element stable-ID collision was rejected.";
			return false;
		}
		const size_t iFamily = eFamily == EFFECT_GPU_RENDER_FAMILY::MESH ? 0u :
			eFamily == EFFECT_GPU_RENDER_FAMILY::SPRITE ? 1u :
			eFamily == EFFECT_GPU_RENDER_FAMILY::DECAL ? 2u : 3u;
		const size_t iOrdinal = ++FamilyOrdinals[iFamily];
		Pending.LoweredElement.strElementId = StableId;
		Pending.LoweredElement.strGroupId = std::string("family.") +
			(eFamily == EFFECT_GPU_RENDER_FAMILY::MESH ? "mesh" :
			 eFamily == EFFECT_GPU_RENDER_FAMILY::SPRITE ? "sprite" :
			 eFamily == EFFECT_GPU_RENDER_FAMILY::DECAL ? "decal" : "ribbon");
		Pending.LoweredElement.strDisplayName =
			std::string(ArtistCoreFamilyLabel(eFamily)) +
			" " + (iOrdinal < 10u ? "0" : "") +
			std::to_string(iOrdinal) + " | " +
			PrimaryAuthoringResourceLeaf(Pending.LoweredElement);
		if (PendingElements.empty())
			strFirstSourceElementId = Emitter.strSourceElementId;
		PendingElements.push_back(std::move(Pending));
	}
	if (PendingElements.size() != 33u || iFollowElementCount != 5u ||
		iStrictPresetElementCount != 3u ||
		FamilyOrdinals != std::array<size_t, 4u>{ 13u, 16u, 3u, 1u })
	{
		m_strElementStatus =
			"Artist F Unified draft requires Mesh 13, Sprite 16, Decal 3, Ribbon 1, including five exact follow starts.";
		return false;
	}
	if (!Synchronize_Artist31470FullPreview(m_pArtistFSourcePreparation))
	{
		m_strElementStatus =
			"Artist F Unified starting-state animation preparation failed: " +
			m_strPreviewAnimationStatus;
		return false;
	}
	CAnimationHistoricalPoseBinding PoseBinding;
	f32_t fAnimationDurationSeconds = 0.f;
	if (!Prepare_Artist31470HistoricalPoseBinding(
			m_pArtistFSourcePreparation, PoseBinding,
			fAnimationDurationSeconds, Error))
	{
		m_strElementStatus =
			"Artist F Unified historical pose binding failed: " + Error;
		return false;
	}

	std::vector<EFFECT_ELEMENT_DESC> BakedElements;
	BakedElements.reserve(PendingElements.size());
	for (PENDING_ARTIST_F_UNIFIED_ELEMENT& Pending : PendingElements)
	{
		if (Pending.BakeRequest.bFollowAttachment)
		{
			const f32_t fEmitStartSeconds =
				Pending.BakeRequest.fScheduleStartDelaySeconds +
				Pending.BakeRequest.fEmitterDelaySeconds;
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Sample;
			if (!Build_Artist31470HistoricalTransformSample(
					m_pArtistFSourcePreparation, PoseBinding,
					fAnimationDurationSeconds, fEmitStartSeconds,
					Sample, Error))
			{
				m_strElementStatus =
					"Artist F Unified follow start sampling failed for " +
					Pending.strSourceElementId + ": " + Error;
				return false;
			}
			const auto Anchor = Sample.SourceAnchorWorlds.find(
				Pending.strFollowAnchorSlotId);
			if (Anchor == Sample.SourceAnchorWorlds.end())
			{
				m_strElementStatus =
					"Artist F Unified follow start lost its exact anchor: " +
					Pending.strSourceElementId;
				return false;
			}
			const matrix_t RootWorld = XMLoadFloat4x4(&Sample.RootWorld);
			vector_t vRootDeterminant = XMMatrixDeterminant(RootWorld);
			const f32_t fRootDeterminant = XMVectorGetX(vRootDeterminant);
			if (!std::isfinite(fRootDeterminant) ||
				std::abs(fRootDeterminant) <= 1.0e-8f)
			{
				m_strElementStatus =
					"Artist F Unified follow start has a singular root: " +
					Pending.strSourceElementId;
				return false;
			}
			const matrix_t ParentLocal =
				XMLoadFloat4x4(&Anchor->second) *
				XMMatrixInverse(&vRootDeterminant, RootWorld);
			XMStoreFloat4x4(
				&Pending.BakeRequest.FollowParentLocalTransform, ParentLocal);
			Pending.BakeRequest.bHasFollowParentLocalTransform = true;
		}

		EFFECT_ELEMENT_DESC BakedElement;
		if (!CEffectDocumentCodec::Bake_GenericAuthoredElementStartingState(
				Pending.LoweredElement, Pending.BakeRequest,
				BakedElement, Error))
		{
			m_strElementStatus =
				"Artist F Unified starting-state bake failed for " +
				Pending.strSourceElementId + ": " + Error;
			return false;
		}
		const auto ProgramEmitter = std::find_if(
			pProgram->Emitters.begin(), pProgram->Emitters.end(),
			[&Pending](const EFFECT_RUNTIME_PROGRAM_EMITTER& Candidate)
			{ return Candidate.strSourceElementId == Pending.strSourceElementId; });
		const auto SourceElement = std::find_if(
			SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
			[&Pending](const EFFECT_ELEMENT_DESC& Candidate)
			{ return Candidate.strElementId == Pending.strSourceElementId; });
		if (ProgramEmitter == pProgram->Emitters.end() ||
			SourceElement == SourceDocument.Elements.end() ||
			!Try_ApplyArtistFTrackASeedData(
				*ProgramEmitter, *SourceElement, BakedElement, Error))
		{
			m_strElementStatus = Error.empty() ?
				"Artist F Unified Track A particle/attachment seed could not be applied." :
				Error;
			return false;
		}
		BakedElements.push_back(std::move(BakedElement));
	}

	EFFECT_DOCUMENT_DESC Candidate;
	if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
			SourceDocument, strFirstSourceElementId,
			ARTIST_F_UNIFIED_EFFECT_ASSET_ID, Candidate, Error))
	{
		m_strElementStatus = Error;
		return false;
	}
	Candidate.strEffectAssetId = ARTIST_F_UNIFIED_EFFECT_ASSET_ID;
	Candidate.strDisplayName = "Artist F Unified Effect";
	Candidate.ModelCues.clear();
	Candidate.Elements.assign(1u, BakedElements.front());
	std::vector<EFFECT_ELEMENT_DESC> Remaining(
		std::next(BakedElements.begin()), BakedElements.end());
	EFFECT_DOCUMENT_DESC Merged;
	if (!CEffectDocumentCodec::Merge_GenericAuthoredElements(
			Candidate, Remaining, Merged, Error) ||
		Merged.Elements.size() != 33u ||
		!CEffectDocumentCodec::Validate_Drawable(Merged, Error))
	{
		m_strElementStatus = Error.empty() ?
			"Artist F Unified draft merge failed." : Error;
		return false;
	}

	const bool_t bPreviousPreviewIsSource =
		m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value();
	const optional<EFFECT_DOCUMENT_DESC> PreviousPreview =
		bPreviousPreviewIsSource ?
			m_SourcePreviewDocument : m_ActiveDocument;
	const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
	const f32_t fPreviousTime = m_fPreviewTimeSeconds;
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_fPreviewTimeSeconds = 0.f;
	if (!Stage_WorldPreview(Merged))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_fPreviewTimeSeconds = fPreviousTime;
		m_strElementStatus =
			"Artist F Unified draft preview preflight failed: " +
			m_strPreviewStatus;
		return false;
	}
	if (!CEffectDocumentCodec::Save_AtomicIfUnchanged(
			Path, Merged, std::string_view{}, Error))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_fPreviewTimeSeconds = fPreviousTime;
		if (PreviousPreview.has_value())
			Stage_WorldPreview(*PreviousPreview, bPreviousPreviewIsSource);
		else
			Hide_WorldPreview();
		m_strElementStatus = Error.empty() ?
			"Artist F Unified draft could not be saved." : Error;
		return false;
	}
	if (!Try_LoadDocumentPathStaged(Path, EFFECT_DOCUMENT_SOURCE::AUTHORED,
			ARTIST_F_UNIFIED_EFFECT_ASSET_ID, true))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_fPreviewTimeSeconds = fPreviousTime;
		if (PreviousPreview.has_value())
			Stage_WorldPreview(*PreviousPreview, bPreviousPreviewIsSource);
		else
			Hide_WorldPreview();
		m_strElementStatus =
			"Artist F Unified draft was saved, but could not be opened. Use Load Effect to retry.";
		return false;
	}
	m_SourceElementPresetSelection.reset();
	m_strElementStatus =
		"Created Artist F Unified draft with 33 editable Elements, 28 typed Track A Material recipes, fixed particle bursts/local-space, editable constant-one DynamicParameter fallbacks, root snapshots, and five exact emit-start follow poses. Five unsupported Material rows remain explicit bounded generic starters; product mapping was unchanged.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectRuntimeOccurrence(
	const std::string& strEffectAssetId,
	const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter)
{
	const bool_t bChangesSelection =
		EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE != m_eDetailSelection ||
		m_strSelectedRuntimeOccurrenceEffectId != strEffectAssetId ||
		m_strSelectedRuntimeOccurrenceId != Emitter.Row.strId;
	if (!bChangesSelection)
		return true;
	if (m_bDocumentDirty || m_bParticleSystemDraftDirty ||
		m_bDetailDraftDirty || m_bModelCueDraftDirty)
	{
		m_strElementStatus =
			"Save or discard the active authored Effect work before opening runtime occurrence tuning.";
		return false;
	}
	if (m_bOccurrenceTransformDraftDirty)
	{
		m_strElementStatus =
			"Apply or Revert the occurrence Transform draft before selecting another occurrence.";
		return false;
	}
	if ((m_OccurrenceTuningDocument.has_value() ||
		 m_SourceAuthoringOverlayDocument.has_value()) &&
		m_bOccurrenceTuningDirty)
	{
		m_strElementStatus =
			"Save or Reload the current Effect Detail changes before selecting another Effect.";
		return false;
	}

	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		CEffectCatalog::Find_RuntimeProgramEntry(strEffectAssetId);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == pEntry ? nullptr : pEntry->Get_Program();
	if (nullptr == pEntry || nullptr == pProgram)
	{
		m_strElementStatus =
			"The selected occurrence has no admitted reconstructed Program entry.";
		return false;
	}
	const auto ProgramEmitter = std::find_if(
		pProgram->Emitters.begin(), pProgram->Emitters.end(),
		[&Emitter](const EFFECT_RUNTIME_PROGRAM_EMITTER& Candidate)
		{
			return Candidate.Row.strId == Emitter.Row.strId;
		});
	if (ProgramEmitter == pProgram->Emitters.end() ||
		ProgramEmitter->Row.strRowSha256 != Emitter.Row.strRowSha256 ||
		ProgramEmitter->strSourceElementId != Emitter.strSourceElementId ||
		ProgramEmitter->strSourceEmitterPath != Emitter.strSourceEmitterPath)
	{
		m_strElementStatus =
			"The selected occurrence identity changed after the All Effects tree was built.";
		return false;
	}
	if (!Ensure_ArtistFSourceSnapshotForAuthoring() ||
		!Ensure_ArtistFSourceAuthoringOverlaySession())
		return false;
	if (nullptr == m_pArtistFSourceProjection)
	{
		m_strElementStatus =
			"Artist F source projection is unavailable while opening Effect Detail.";
		return false;
	}

	const auto ProjectedElement = std::find_if(
		m_pArtistFSourceProjection->Get_Document().Elements.begin(),
		m_pArtistFSourceProjection->Get_Document().Elements.end(),
		[&ProgramEmitter](const EFFECT_ELEMENT_DESC& Candidate)
		{
			return Candidate.strElementId == ProgramEmitter->strSourceElementId;
		});
	if (ProjectedElement ==
			m_pArtistFSourceProjection->Get_Document().Elements.end() ||
		!ProjectedElement->bVisible)
	{
		m_strElementStatus =
			"Artist F projected source Element is unavailable while opening Effect Detail.";
		return false;
	}

	EFFECT_OCCURRENCE_LOCAL_TRANSFORM CueLocalSourceTransform{};
	if (!Try_NarrowRuntimeFloat3(
			ProgramEmitter->CueLocalTransform.vPosition,
			CueLocalSourceTransform.vPosition) ||
		!Try_NarrowRuntimeFloat3(
			ProgramEmitter->CueLocalTransform.vRotationDegrees,
			CueLocalSourceTransform.vRotationDegrees) ||
		!Try_NarrowRuntimeFloat3(
			ProgramEmitter->CueLocalTransform.vScale,
			CueLocalSourceTransform.vScale))
	{
		m_strElementStatus =
			"Artist F source-local Transform is non-finite while opening Effect Detail.";
		return false;
	}

	if (!m_SourceAuthoringOverlayDocument.has_value() ||
		m_SourceAuthoringOverlayDocument->strEffectAssetId != strEffectAssetId)
	{
		m_strElementStatus =
			"Artist F source-backed edit session changed while opening the Element.";
		return false;
	}
	const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY* pTuned =
		CEffectSourceAuthoringOverlayCodec::Find_Entry(
			*m_SourceAuthoringOverlayDocument, Emitter.Row.strId);
	const EFFECT_OCCURRENCE_LOCAL_TRANSFORM StagedDraft = nullptr == pTuned ?
		CueLocalSourceTransform : pTuned->EffectiveLocalTransform;
	m_OccurrenceTransformDraft = StagedDraft;
	m_SelectedOccurrenceSourceTransform = CueLocalSourceTransform;
	m_pSelectedVisualSourceProjection.reset();
	m_strSelectedRuntimeOccurrenceEffectId = strEffectAssetId;
	m_strSelectedRuntimeOccurrenceId = Emitter.Row.strId;
	m_strSelectedRuntimeOccurrenceRowSha256 = Emitter.Row.strRowSha256;
	m_strSelectedRuntimeOccurrenceElementId = Emitter.strSourceElementId;
	m_strSelectedRuntimeOccurrenceEmitterPath = Emitter.strSourceEmitterPath;
	m_bOccurrenceTransformDraftDirty = false;
	m_bOccurrenceTuningDirty =
		CEffectSourceAuthoringOverlayCodec::Serialize(
			*m_SourceAuthoringOverlayDocument) !=
		m_strSourceAuthoringOverlayBaselineCanonical;
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE;
	m_strElementStatus =
		"Loaded one Artist F Element into Effect Detail.";
	m_strDetailStatus =
		"The Track A source renderer, material, DDS roles, attachment, and distributions remain active; Position, Rotation, and Scale are editable here.";
	return true;
}

std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
Client::CEffect_Tool::Resolve_VisualProgramProjectionForAuthoring(
	const std::string& strEffectAssetId)
{
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> pProgram =
		CEffectCatalog::Find_VisualProgram(strEffectAssetId);
	if (nullptr == pProgram)
		return nullptr;

	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProjection = CEffectCatalog::Find_VisualProjection(strEffectAssetId);
	if (strEffectAssetId == ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
		pProgram->eProjectionKind ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 &&
		Ensure_ArtistFSourceSnapshotForAuthoring())
	{
		pProjection = m_pArtistFSourceProjection;
	}
	if (nullptr == pProjection)
	{
		if (pProgram->eProjectionKind !=
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
		{
			return nullptr;
		}
		if (strEffectAssetId == ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
			Ensure_ArtistFSourceSnapshotForAuthoring())
		{
			pProjection = m_pArtistFSourceProjection;
		}
		if (nullptr != pProjection)
		{
			// CPU-only immutable source snapshot; Play/Solo still owns GPU staging.
		}
		else if (!m_bReconstructedSourceRuntimeActive)
		{
			return nullptr;
		}
		else
		{
		const shared_ptr<CEffectObject> pObject =
			m_pWorldPreviewObject.lock();
		const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
			pRuntimeProgram = nullptr == pObject ? nullptr :
				pObject->Get_ReconstructedRuntimeProgram();
		if (nullptr == pObject ||
			!pObject->Is_ReconstructedSourceRuntimeActive() ||
			!pObject->Is_SourceVisualProgramActive() ||
			nullptr == pRuntimeProgram ||
			pRuntimeProgram->strRuntimeCatalogAssetId != strEffectAssetId)
		{
			return nullptr;
		}
		pProjection = pObject->Get_SourceVisualProgramProjection();
		}
	}
	if (nullptr == pProjection || !pProjection->Is_Valid() ||
		pProjection->Get_EffectAssetId() != strEffectAssetId ||
		pProjection->Get_ProjectionKind() != pProgram->eProjectionKind ||
		pProjection->Get_ProgramSha256() != pProgram->strProgramSha256)
	{
		return nullptr;
	}
	return pProjection;
}

bool_t Client::CEffect_Tool::Try_OpenVisualProgramElementForAuthoring(
	const std::string& strEffectAssetId,
	const std::string& strOccurrenceId,
	const std::string& strRowSha256,
	const std::string& strTargetElementId,
	const std::string& strSourceRecordId)
{
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProjection =
			Resolve_VisualProgramProjectionForAuthoring(strEffectAssetId);
	EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_REQUEST Request;
	Request.strEffectAssetId = strEffectAssetId;
	Request.strOccurrenceId = strOccurrenceId;
	Request.strRowSha256 = strRowSha256;
	Request.strTargetElementId = strTargetElementId;
	Request.strSourceRecordId = strSourceRecordId;
	EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_STAGE Stage;
	std::string Error;
	if (!CEffectVisualProgramCorpusCodec::Build_ElementAuthoringPresetStage(
			pProjection, Request, Stage, Error))
	{
		m_strElementStatus = Error.empty() ?
			"The selected Element is no longer available." : Error;
		return false;
	}

	SOURCE_ELEMENT_PRESET_SELECTION Selection;
	Selection.pProjection = Stage.pProjection;
	Selection.GenericElement = std::move(Stage.Element);
	Selection.strSourceEffectAssetId = Stage.Identity.strEffectAssetId;
	Selection.strOccurrenceId = Stage.Identity.strOccurrenceId;
	Selection.strRowSha256 = Stage.Identity.strRowSha256;
	Selection.strTargetElementId = Stage.Identity.strTargetElementId;
	Selection.strSourceRecordId = Stage.Identity.strSourceRecordId;
	Selection.strSourceFamily = VisualProgramFamilyLabel(Stage.eSourceFamily);

	EFFECT_ELEMENT_DESC Preset = Selection.GenericElement;
	const EFFECT_AUTHORING_FAMILY eFamily = Resolve_AuthoringFamily(Preset);
	if (!AuthoringFamily_CanCreate(eFamily))
	{
		m_strElementStatus =
			"Presentation Light and Screen Post are edited or deleted in the active Effect; creating them from a source preset is not admitted.";
		return false;
	}
	Preset.strGroupId = "manual.hit1";
	Preset.strElementId.clear();
	Preset.strDisplayName = AuthoringFamily_Label(eFamily);
	Selection.GenericElement = Preset;
	m_eSelectedAuthoringFamily = eFamily;
	m_eSelectedEffectType = Preset.eKind;
	m_MeshAuthoringDraft = std::move(Preset);
	m_bMeshAuthoringDraftInitialized = true;
	m_SourceElementPresetSelection = std::move(Selection);
	m_NewElementId[0u] = '\0';
	const bool_t bRequiresMesh = AuthoringFamily_RequiresMesh(eFamily);
	m_strSelectedResourceSlotId = bRequiresMesh ?
		std::string(EFFECT_MESH_SHAPE_SLOT_ID) :
		std::string(EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
	m_strSelectedResourceAssetId.clear();
	m_eResourceLibraryFileKind = bRequiresMesh ?
		EFFECT_RESOURCE_FILE_KIND::MODEL : EFFECT_RESOURCE_FILE_KIND::TEXTURE;
	m_iResourceViewRevision = UINT64_MAX;
	m_strElementStatus = std::string("Loaded one ") +
		AuthoringFamily_Label(eFamily) +
		" Track A data as an Element seed. Current Effect and Effect Details were preserved; use Create Element, then Save Changes.";
	return true;
}

bool_t Client::CEffect_Tool::Try_OpenArtistFReconstructedElementForAuthoring(
	const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter)
{
	if (!Ensure_ArtistFSourceSnapshotForAuthoring())
	{
		m_strElementStatus = m_strArtistFSourceSnapshotStatus.empty() ?
			"Artist F source recipe is unavailable." :
			m_strArtistFSourceSnapshotStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProjection = Resolve_VisualProgramProjectionForAuthoring(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == m_pArtistFSourcePreparation ? nullptr :
			m_pArtistFSourcePreparation->Get_Program();
	if (nullptr == pProjection || nullptr == pProgram ||
		pProgram->strRuntimeCatalogAssetId != ARTIST_F_VISUAL_PROGRAM_ASSET_ID)
	{
		m_strElementStatus =
			"Artist F source recipe identity changed before import.";
		return false;
	}
	const std::string strVisualOccurrenceId =
		Emitter.strMaterialOccurrenceId.value_or(Emitter.Row.strId);
	if (nullptr != pProjection->Find_RowByOccurrenceId(strVisualOccurrenceId) ||
		nullptr != pProjection->Find_SupplementalElementByOccurrenceId(
			strVisualOccurrenceId))
	{
		m_strElementStatus =
			"This admitted Track A row must use its exact preset-stage identity.";
		return false;
	}

	const auto RuntimeEmitter = std::find_if(
		pProgram->Emitters.begin(), pProgram->Emitters.end(),
		[&Emitter](const EFFECT_RUNTIME_PROGRAM_EMITTER& Candidate)
		{
			return Candidate.Row.strId == Emitter.Row.strId;
		});
	EFFECT_GPU_RENDER_FAMILY eFamily = EFFECT_GPU_RENDER_FAMILY::END;
	if (RuntimeEmitter == pProgram->Emitters.end() ||
		RuntimeEmitter->Row.strRowSha256 != Emitter.Row.strRowSha256 ||
		RuntimeEmitter->strSourceElementId != Emitter.strSourceElementId ||
		RuntimeEmitter->strSourceEmitterPath != Emitter.strSourceEmitterPath ||
		!RuntimeEmitter->bVisible ||
		!Try_ResolveArtistCoreFamily(RuntimeEmitter->eRenderer, eFamily))
	{
		m_strElementStatus =
			"The selected Core33 occurrence identity changed after the tree was built.";
		return false;
	}
	const auto ProjectedElement = std::find_if(
		pProjection->Get_Document().Elements.begin(),
		pProjection->Get_Document().Elements.end(),
		[&Emitter](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == Emitter.strSourceElementId;
		});
	if (ProjectedElement == pProjection->Get_Document().Elements.end() ||
		!ProjectedElement->bVisible)
	{
		m_strElementStatus =
			"The immutable Core33 projection no longer contains this visible Element.";
		return false;
	}

	SOURCE_ELEMENT_PRESET_SELECTION Selection;
	Selection.pProjection = pProjection;
	Selection.strSourceEffectAssetId = ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	Selection.strOccurrenceId = Emitter.Row.strId;
	Selection.strRowSha256 = Emitter.Row.strRowSha256;
	Selection.strTargetElementId = Emitter.strSourceElementId;
	Selection.strSourceRecordId = Emitter.strSourceEmitterPath;
	Selection.strSourceFamily = ArtistCoreFamilyLabel(eFamily);
	if (!Try_StageElementAsAuthoringPreset(
			pProjection->Get_Document(), Emitter.strSourceElementId,
			std::move(Selection)))
	{
		return false;
	}
	m_strElementStatus = std::string("Loaded one ") +
		ArtistCoreFamilyLabel(eFamily) +
		" Element seed. Safe Detail values and DDS/WModel slots were copied; native source execution was not copied.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectVisualOccurrence(
	const std::string& strEffectAssetId,
	const std::string& strOccurrenceId,
	const std::string& strRowSha256,
	const std::string& strTargetElementId,
	const std::string& strSourceRecordId)
{
	const bool_t bChangesSelection =
		EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE != m_eDetailSelection ||
		m_strSelectedRuntimeOccurrenceEffectId != strEffectAssetId ||
		m_strSelectedRuntimeOccurrenceId != strOccurrenceId ||
		m_strSelectedRuntimeOccurrenceRowSha256 != strRowSha256 ||
		m_strSelectedRuntimeOccurrenceElementId != strTargetElementId ||
		m_strSelectedRuntimeOccurrenceEmitterPath != strSourceRecordId;
	if (!bChangesSelection)
		return true;
	if (m_bDocumentDirty || m_bParticleSystemDraftDirty ||
		m_bDetailDraftDirty || m_bModelCueDraftDirty)
	{
		m_strElementStatus =
			"Save or discard the active authored Effect work before opening visual occurrence tuning.";
		return false;
	}
	if (m_bOccurrenceTransformDraftDirty)
	{
		m_strElementStatus =
			"Apply or Revert the occurrence Transform draft before selecting another occurrence.";
		return false;
	}
	if (m_OccurrenceTuningDocument.has_value() &&
		m_OccurrenceTuningDocument->strEffectAssetId != strEffectAssetId &&
		m_bOccurrenceTuningDirty)
	{
		m_strElementStatus =
			"Save or Reload the current occurrence tuning artifact before selecting another Effect.";
		return false;
	}

	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProjection = CEffectCatalog::Find_VisualProjection(strEffectAssetId);
	const EFFECT_VISUAL_PROGRAM_ROW* pCatalogRow = nullptr == pProjection ?
		nullptr : pProjection->Find_RowByOccurrenceId(
			strOccurrenceId);
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pSupplemental =
		nullptr == pProjection ? nullptr :
			pProjection->Find_SupplementalElementByOccurrenceId(strOccurrenceId);
	const bool_t bVisualRowMatches = nullptr != pCatalogRow &&
		pCatalogRow->strRowSha256 == strRowSha256 &&
		pCatalogRow->bTuningEligibleTransform &&
		pCatalogRow->TargetIdentity.has_value() &&
		pCatalogRow->TargetIdentity->strTargetElementId == strTargetElementId;
	const bool_t bSupplementalMatches = nullptr != pSupplemental &&
		pSupplemental->strRowSha256 == strRowSha256 &&
		pSupplemental->bTuningEligibleTransform &&
		pSupplemental->TargetIdentity.strTargetElementId == strTargetElementId;
	if (nullptr == pProjection ||
		bVisualRowMatches == bSupplementalMatches)
	{
		m_strElementStatus =
			"The selected visual occurrence is not admitted for Transform tuning.";
		return false;
	}
	const shared_ptr<CEffectObject> pPreviewObject =
		m_pWorldPreviewObject.lock();
	if (nullptr == pPreviewObject ||
		!pPreviewObject->Is_SourceVisualProgramActive() ||
		nullptr == m_pVisualPreviewProjection ||
		m_pVisualPreviewProjection->Get_EffectAssetId() != strEffectAssetId)
	{
		m_strElementStatus =
			"Play this Effect cue before selecting one of its visual occurrences.";
		return false;
	}
	const auto SourceElement = std::find_if(
		pProjection->Get_Document().Elements.begin(),
		pProjection->Get_Document().Elements.end(),
		[&strTargetElementId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == strTargetElementId;
		});
	if (SourceElement == pProjection->Get_Document().Elements.end())
	{
		m_strElementStatus =
			"The selected visual occurrence target Element is missing.";
		return false;
	}

	EFFECT_OCCURRENCE_TUNING_DOCUMENT StagedTuning;
	std::filesystem::path StagedPath;
	std::string StagedBaseline;
	std::string Error;
	const bool_t bReuseSession = m_OccurrenceTuningDocument.has_value() &&
		m_OccurrenceTuningDocument->strEffectAssetId == strEffectAssetId;
	if (bReuseSession)
	{
		StagedTuning = *m_OccurrenceTuningDocument;
		StagedPath = m_OccurrenceTuningPath;
		StagedBaseline = m_strOccurrenceTuningBaselineCanonical;
	}
	else
	{
		StagedPath = CProjectDataRoot::Resolve(
			std::filesystem::path("Effects/AuthoredCorrections/VisualPrograms") /
				(strEffectAssetId + ".occurrence-tuning.json"));
		if (StagedPath.empty())
		{
			m_strElementStatus =
				"The visual occurrence tuning authoring path is invalid.";
			return false;
		}
		if (std::filesystem::is_regular_file(StagedPath))
		{
			if (!CEffectOccurrenceTuningCodec::Load(
					StagedPath, StagedTuning, Error) ||
				!CEffectOccurrenceTuningCodec::Validate_AgainstProjection(
					StagedTuning, *pProjection, Error))
			{
				m_strElementStatus = Error.empty() ?
					"Visual occurrence tuning authoring file is unavailable." :
					Error;
				return false;
			}
			StagedBaseline =
				CEffectOccurrenceTuningCodec::Serialize(StagedTuning);
		}
		else
		{
			StagedTuning.iFormatVersion =
				EFFECT_OCCURRENCE_TUNING_FORMAT_VERSION;
			StagedTuning.strEffectAssetId = strEffectAssetId;
			if (!CEffectOccurrenceTuningCodec::Validate_AgainstProjection(
					StagedTuning, *pProjection, Error))
			{
				m_strElementStatus = Error;
				return false;
			}
		}
	}
	if (!Stage_RuntimeOccurrenceTuningPreview(StagedTuning))
		return false;

	EFFECT_OCCURRENCE_LOCAL_TRANSFORM SourceTransform;
	SourceTransform.vPosition = SourceElement->Detail.Transform.vPosition;
	SourceTransform.vRotationDegrees =
		SourceElement->Detail.Transform.vRotationDegrees;
	SourceTransform.vScale = SourceElement->Detail.Transform.vScale;
	const EFFECT_OCCURRENCE_TUNING_ENTRY* pTuned =
		CEffectOccurrenceTuningCodec::Find_Entry(
			StagedTuning, strOccurrenceId);
	const EFFECT_OCCURRENCE_LOCAL_TRANSFORM StagedDraft = nullptr == pTuned ?
		SourceTransform : pTuned->EffectiveLocalTransform;
	const bool_t bStartedEmptyArtifact = StagedBaseline.empty();
	m_OccurrenceTuningDocument = std::move(StagedTuning);
	m_OccurrenceTuningPath = std::move(StagedPath);
	m_strOccurrenceTuningBaselineCanonical = std::move(StagedBaseline);
	m_OccurrenceTransformDraft = StagedDraft;
	m_SelectedOccurrenceSourceTransform = SourceTransform;
	m_pSelectedVisualSourceProjection = pProjection;
	m_strSelectedRuntimeOccurrenceEffectId = strEffectAssetId;
	m_strSelectedRuntimeOccurrenceId = strOccurrenceId;
	m_strSelectedRuntimeOccurrenceRowSha256 = strRowSha256;
	m_strSelectedRuntimeOccurrenceElementId = strTargetElementId;
	m_strSelectedRuntimeOccurrenceEmitterPath = strSourceRecordId;
	m_bOccurrenceTransformDraftDirty = false;
	m_bOccurrenceTuningDirty =
		CEffectOccurrenceTuningCodec::Serialize(*m_OccurrenceTuningDocument) !=
		m_strOccurrenceTuningBaselineCanonical;
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE;
	m_strElementStatus =
		"Selected stable visual occurrence; playback scope was preserved.";
	m_strDetailStatus = bReuseSession ?
		"Visual occurrence tuning session preserved." :
		(bStartedEmptyArtifact ?
			"Started an empty PROJECT_TUNED override set from immutable Source." :
			"Loaded and staged the saved visual occurrence tuning artifact.");
	return true;
}

bool_t Client::CEffect_Tool::Try_SetVisualPreviewOccurrenceIsolation(
	const std::string& strTargetElementId)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_SourceVisualProgramActive() ||
		strTargetElementId.empty())
	{
		m_strPreviewStatus =
			"Visual element Solo requires an active admitted visual-program preview.";
		return false;
	}
	EFFECT_PREVIEW_SUBMISSION_ISOLATION Isolation;
	Isolation.eKind = EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::OCCURRENCE;
	Isolation.strElementId = strTargetElementId;
	std::string Error;
	if (!pObject->Set_PreviewSubmissionIsolation(Isolation, Error))
	{
		m_strPreviewStatus = "Visual element Solo failed: " + Error;
		return false;
	}
	m_strPreviewIsolationElementId = strTargetElementId;
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewStatus = "Visual element Solo: " + strTargetElementId +
		". Click selection remains independent; Return to All restores playback.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SetVisualPreviewFamilyIsolation(
	const EFFECT_VISUAL_PROGRAM& Program,
	const EFFECT_VISUAL_PROGRAM_FAMILY eFamily)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_SourceVisualProgramActive() ||
		eFamily >= EFFECT_VISUAL_PROGRAM_FAMILY::END)
	{
		m_strPreviewStatus =
			"Visual Family playback requires an active admitted visual-program preview.";
		return false;
	}
	std::set<std::string, std::less<>> TargetIds;
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program.VisualRows)
	{
		if (Row.eFamily == eFamily && Row.eDisposition ==
				EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED &&
			Row.TargetIdentity.has_value())
		{
			TargetIds.insert(Row.TargetIdentity->strTargetElementId);
		}
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Row :
		Program.SupplementalElements)
	{
		if (Row.eFamily == eFamily && Row.eDisposition ==
				EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED &&
			!Row.TargetIdentity.strTargetElementId.empty())
		{
			TargetIds.insert(Row.TargetIdentity.strTargetElementId);
		}
	}
	if (TargetIds.empty())
	{
		m_strPreviewStatus =
			"Visual Family has no admitted runtime target Elements.";
		return false;
	}
	EFFECT_PREVIEW_SUBMISSION_ISOLATION Isolation;
	Isolation.eKind = EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET;
	Isolation.ElementIds.assign(TargetIds.begin(), TargetIds.end());
	std::string Error;
	if (!pObject->Set_PreviewSubmissionIsolation(Isolation, Error))
	{
		m_strPreviewStatus = "Visual Family playback failed: " + Error;
		return false;
	}
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId = VisualProgramFamilyLabel(eFamily);
	m_strPreviewStatus = std::string("Visual-program playback Family: ") +
		VisualProgramFamilyLabel(eFamily) + " (" +
		std::to_string(TargetIds.size()) + " stable targets).";
	return true;
}

bool_t Client::CEffect_Tool::Try_ResetVisualPreviewIsolation()
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_SourceVisualProgramActive())
	{
		m_strPreviewStatus =
			"Return to All requires an active admitted visual-program preview.";
		return false;
	}
	pObject->Reset_PreviewSubmissionIsolation();
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewStatus = "Visual-program playback scope: All.";
	return true;
}

void Client::CEffect_Tool::Render_RuntimeOccurrenceDetail()
{
	const bool_t bSourceBacked =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_strSelectedRuntimeOccurrenceEffectId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	if ((!bSourceBacked && !m_OccurrenceTuningDocument.has_value()) ||
		!m_OccurrenceTransformDraft.has_value() ||
		m_strSelectedRuntimeOccurrenceId.empty())
	{
		ImGui::TextDisabled(
			"The selected Element edit session is unavailable.");
		return;
	}
	ImGui::TextWrapped("Artist F | %s",
		m_strSelectedRuntimeOccurrenceElementId.c_str());
	if (bSourceBacked)
	{
		ImGui::TextDisabled(
			"Track A source-backed Element. Renderer, DDS roles, attachment, timing distributions, and material evaluator remain connected.");
		if (m_pArtistFSourceProjection)
		{
			const auto Element = std::find_if(
				m_pArtistFSourceProjection->Get_Document().Elements.begin(),
				m_pArtistFSourceProjection->Get_Document().Elements.end(),
				[this](const EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId ==
						m_strSelectedRuntimeOccurrenceElementId;
				});
			if (Element !=
				m_pArtistFSourceProjection->Get_Document().Elements.end())
			{
				ImGui::TextWrapped("Connected slots: %s",
					AuthoringElementResourceSlotSummary(*Element).c_str());
			}
		}
	}
	ImGui::Separator();
	ImGui::TextDisabled("Source local Transform (read-only)");
	ImGui::TextDisabled("Position: %.4f, %.4f, %.4f",
		m_SelectedOccurrenceSourceTransform.vPosition.x,
		m_SelectedOccurrenceSourceTransform.vPosition.y,
		m_SelectedOccurrenceSourceTransform.vPosition.z);
	ImGui::TextDisabled("Rotation: %.4f, %.4f, %.4f degrees",
		m_SelectedOccurrenceSourceTransform.vRotationDegrees.x,
		m_SelectedOccurrenceSourceTransform.vRotationDegrees.y,
		m_SelectedOccurrenceSourceTransform.vRotationDegrees.z);
	ImGui::TextDisabled("Scale: %.4f, %.4f, %.4f",
		m_SelectedOccurrenceSourceTransform.vScale.x,
		m_SelectedOccurrenceSourceTransform.vScale.y,
		m_SelectedOccurrenceSourceTransform.vScale.z);
	ImGui::Separator();
	const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY* pSourceOverride =
		bSourceBacked ? CEffectSourceAuthoringOverlayCodec::Find_Entry(
			*m_SourceAuthoringOverlayDocument,
			m_strSelectedRuntimeOccurrenceId) : nullptr;
	const EFFECT_OCCURRENCE_TUNING_ENTRY* pLegacyOverride =
		bSourceBacked ? nullptr : CEffectOccurrenceTuningCodec::Find_Entry(
			*m_OccurrenceTuningDocument,
			m_strSelectedRuntimeOccurrenceId);
	const bool_t bHasCommittedOverride =
		nullptr != pSourceOverride || nullptr != pLegacyOverride;
	ImGui::TextDisabled(bHasCommittedOverride ?
		"Saved in-memory values are active for this Element." :
		"This Element currently uses the Track A source Transform.");
	bool_t bChanged = false;
	bChanged |= DragFloat3("Position",
		m_OccurrenceTransformDraft->vPosition, 0.01f, -1000.f, 1000.f);
	bChanged |= DragFloat3("Rotation (Degrees)",
		m_OccurrenceTransformDraft->vRotationDegrees,
		0.25f, -360.f, 360.f);
	bChanged |= DragFloat3("Scale",
		m_OccurrenceTransformDraft->vScale, 0.01f, 0.001f, 100.f);
	if (bChanged)
	{
		m_bOccurrenceTransformDraftDirty = true;
		bool_t bPreviewStaged = false;
		if (bSourceBacked)
		{
			EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Preview =
				*m_SourceAuthoringOverlayDocument;
			Upsert_SourceAuthoringOverlayEntry(Preview,
				m_strSelectedRuntimeOccurrenceId,
				m_strSelectedRuntimeOccurrenceRowSha256,
				m_strSelectedRuntimeOccurrenceElementId,
				*m_OccurrenceTransformDraft);
			bPreviewStaged = Stage_SourceAuthoringOverlayPreview(Preview);
		}
		else
		{
			EFFECT_OCCURRENCE_TUNING_DOCUMENT Preview =
				*m_OccurrenceTuningDocument;
			Upsert_OccurrenceTuningEntry(Preview,
				m_strSelectedRuntimeOccurrenceId,
				m_strSelectedRuntimeOccurrenceRowSha256,
				*m_OccurrenceTransformDraft);
			bPreviewStaged = Stage_RuntimeOccurrenceTuningPreview(Preview);
		}
		if (bPreviewStaged)
		{
			m_strDetailStatus =
				"Live preview updated. Save Changes persists this Element.";
		}
	}
	ImGui::Separator();
	ImGui::TextDisabled("Effective local Transform (read-only preview)");
	const EFFECT_OCCURRENCE_LOCAL_TRANSFORM& Effective =
		*m_OccurrenceTransformDraft;
	ImGui::TextDisabled("Position: %.4f, %.4f, %.4f",
		Effective.vPosition.x, Effective.vPosition.y, Effective.vPosition.z);
	ImGui::TextDisabled("Rotation: %.4f, %.4f, %.4f degrees",
		Effective.vRotationDegrees.x, Effective.vRotationDegrees.y,
		Effective.vRotationDegrees.z);
	ImGui::TextDisabled("Scale: %.4f, %.4f, %.4f",
		Effective.vScale.x, Effective.vScale.y, Effective.vScale.z);

	ImGui::Separator();
	ImGui::BeginDisabled(!m_bOccurrenceTransformDraftDirty);
	if (ImGui::Button("Revert Draft"))
	{
		const EFFECT_OCCURRENCE_LOCAL_TRANSFORM Reverted =
			nullptr != pSourceOverride ?
				pSourceOverride->EffectiveLocalTransform :
			(nullptr != pLegacyOverride ?
				pLegacyOverride->EffectiveLocalTransform :
				m_SelectedOccurrenceSourceTransform);
		const bool_t bRestaged = bSourceBacked ?
			Stage_SourceAuthoringOverlayPreview(
				*m_SourceAuthoringOverlayDocument) :
			Stage_RuntimeOccurrenceTuningPreview(
				*m_OccurrenceTuningDocument);
		if (bRestaged)
		{
			m_OccurrenceTransformDraft = Reverted;
			m_bOccurrenceTransformDraftDirty = false;
			m_strDetailStatus =
				"Reverted the draft to the last applied Element values.";
		}
	}
	ImGui::EndDisabled();

	ImGui::BeginDisabled(!bHasCommittedOverride);
	if (ImGui::Button("Reset to Source"))
		Try_ResetRuntimeOccurrenceToSource();
	ImGui::EndDisabled();

	ImGui::Separator();
	ImGui::BeginDisabled(
		!m_bOccurrenceTuningDirty && !m_bOccurrenceTransformDraftDirty &&
		!m_bSourceAuthoringOverlayNeedsInitialSave);
	if (ImGui::Button("Save"))
	{
		const bool_t bApplied = !m_bOccurrenceTransformDraftDirty ||
			Try_ApplyRuntimeOccurrenceDraft();
		if (bApplied)
			Try_SaveRuntimeOccurrenceTuning();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(m_bOccurrenceTransformDraftDirty);
	if (ImGui::Button("Load Saved"))
		Try_ReloadRuntimeOccurrenceTuning();
	ImGui::EndDisabled();

	ImGui::TextWrapped("%s", m_strDetailStatus.empty() ?
		"Edit Position, Rotation, or Scale, then Save." :
		m_strDetailStatus.c_str());
	if (bSourceBacked)
	{
		ImGui::TextDisabled(
			"Save writes the Artist F edit overlay without changing its linked source mapping.");
	}
}

bool_t Client::CEffect_Tool::Stage_RuntimeOccurrenceTuningPreview(
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Tuning)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pSourceProjection = CEffectCatalog::Find_VisualProjection(
			Tuning.strEffectAssetId);
	if (nullptr != pSourceProjection)
	{
		if (nullptr == pObject)
		{
			m_strDetailStatus =
				"Visual occurrence tuning preview requires an active world preview.";
			return false;
		}
		EFFECT_DOCUMENT_DESC TunedDocument = pSourceProjection->Get_Document();
		std::string Error;
		if (!CEffectOccurrenceTuningCodec::Apply_ToProjectedDocument(
				TunedDocument, *pSourceProjection, Tuning, Error))
		{
			m_strDetailStatus = Error;
			return false;
		}
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pTunedProjection;
		if (!CEffectVisualProgramCorpusCodec::Derive_TransformTunedProjection(
				*pSourceProjection, TunedDocument, pTunedProjection, Error) ||
			nullptr == pTunedProjection)
		{
			m_strDetailStatus = Error.empty() ?
				"Visual Transform tuning projection could not be resealed." : Error;
			return false;
		}
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pTunedPrepared;
		if (!CEffectDocumentRenderer::Prepare_VisualProgramDocument(
				m_pDevice, m_pContext, pTunedProjection, pTunedPrepared, Error) ||
			nullptr == pTunedPrepared)
		{
			m_strDetailStatus = Error.empty() ?
				"Visual Transform tuning prewarm failed." : Error;
			return false;
		}
		const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pRollbackProjection = nullptr != m_pVisualPreviewProjection ?
				m_pVisualPreviewProjection : pSourceProjection;
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pRollbackPrepared;
		if (!CEffectDocumentRenderer::Prepare_VisualProgramDocument(
				m_pDevice, m_pContext, pRollbackProjection,
				pRollbackPrepared, Error) || nullptr == pRollbackPrepared)
		{
			m_strDetailStatus = Error.empty() ?
				"Visual Transform tuning rollback prewarm failed." : Error;
			return false;
		}
		const EFFECT_PREVIEW_SUBMISSION_ISOLATION PreviousIsolation =
			pObject->Get_PreviewSubmissionIsolation();
		const auto ApplyIsolation = [&pObject](
			const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
			std::string& strOutError)
		{
			if (Isolation.eKind ==
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL)
			{
				pObject->Reset_PreviewSubmissionIsolation();
				strOutError.clear();
				return true;
			}
			return pObject->Set_PreviewSubmissionIsolation(
				Isolation, strOutError);
		};
		const auto RestorePrior = [&]()
		{
			std::string RollbackError;
			const bool_t bRestaged =
				pObject->Stage_PrevalidatedVisualProgramDocument(
					pRollbackProjection, pRollbackPrepared, RollbackError);
			const bool_t bIsolation = bRestaged &&
				ApplyIsolation(PreviousIsolation, RollbackError);
			if (bRestaged && bIsolation)
			{
				pObject->Set_SampleTime(
					Resolve_EffectSampleTime(m_fPreviewTimeSeconds));
			}
			return std::pair<bool_t, std::string>{
				bRestaged && bIsolation, std::move(RollbackError) };
		};
		if (!pObject->Stage_PrevalidatedVisualProgramDocument(
				pTunedProjection, pTunedPrepared, Error) ||
			!ApplyIsolation(PreviousIsolation, Error))
		{
			const auto [bRestored, RollbackError] = RestorePrior();
			m_strDetailStatus = "Visual Transform preview failed: " + Error +
				(bRestored ? " Previous preview/scope restored." :
					" Rollback failed: " + RollbackError);
			return false;
		}
		pObject->Set_SampleTime(
			Resolve_EffectSampleTime(m_fPreviewTimeSeconds));
		m_pVisualPreviewProjection = std::move(pTunedProjection);
		m_strDetailStatus =
			"Visual Transform override staged; playback scope and sample time preserved.";
		return true;
	}
	const auto pPreparation = nullptr == pObject ? nullptr :
		pObject->Get_ReconstructedRuntimePreparation();
	if (nullptr == pObject || nullptr == pPreparation ||
		!m_bReconstructedSourceRuntimeActive)
	{
		m_strDetailStatus =
			"Occurrence tuning preview requires an active reconstructed runtime preview.";
		return false;
	}
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION PreviousIsolation =
		pObject->Get_PreviewSubmissionIsolation();
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		CEffectCatalog::Find_RuntimeProgramEntry(Tuning.strEffectAssetId);
	const std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT> pPublished =
		nullptr == pEntry ? nullptr : pEntry->Get_OccurrenceTuning();
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT* pRollback =
		m_OccurrenceTuningDocument.has_value() &&
		m_OccurrenceTuningDocument->strEffectAssetId == Tuning.strEffectAssetId ?
			&*m_OccurrenceTuningDocument : pPublished.get();
	const auto StageNext =
		[this, &pObject, &pPreparation, &Tuning](std::string& strOutError)
		{
			return CEffectPresentationService::
				Stage_ReconstructedOccurrenceTuningPreview(
					m_pDevice, m_pContext, pObject, pPreparation,
					Tuning, strOutError);
		};
	const auto StageRollback =
		[this, &pObject, &pPreparation, pRollback](std::string& strOutError)
		{
			if (nullptr == pRollback)
			{
				strOutError =
					"The prior occurrence tuning artifact is unavailable.";
				return false;
			}
			return CEffectPresentationService::
				Stage_ReconstructedOccurrenceTuningPreview(
					m_pDevice, m_pContext, pObject, pPreparation,
					*pRollback, strOutError);
		};
	const auto ApplyIsolation =
		[&pObject](
			const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
			std::string& strOutError)
		{
			if (Isolation.eKind ==
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL)
			{
				pObject->Reset_PreviewSubmissionIsolation();
				strOutError.clear();
				return true;
			}
			return pObject->Set_PreviewSubmissionIsolation(
				Isolation, strOutError);
		};
	const f32_t fPreviousTime = m_fPreviewTimeSeconds;
	std::string Error;
	if (!CEffectPresentationService::Restage_ObjectLocalOccurrencePreview(
			PreviousIsolation, StageNext, StageRollback, ApplyIsolation, Error))
	{
		m_strDetailStatus = "Occurrence tuning preview failed: " + Error;
		return false;
	}
	if (!Seek_ReconstructedSourceRuntimeTimeline(fPreviousTime))
	{
		std::string RollbackError;
		const bool_t bRollbackStaged =
			CEffectPresentationService::Restage_ObjectLocalOccurrencePreview(
				PreviousIsolation, StageRollback, StageRollback,
				ApplyIsolation, RollbackError);
		const bool_t bRollbackSeeked = bRollbackStaged &&
			Seek_ReconstructedSourceRuntimeTimeline(fPreviousTime);
		m_strDetailStatus = bRollbackSeeked ?
			"Occurrence tuning preview could not commit; the previous tuning, All/Family scope, and synchronized sample time were restored." :
			"Occurrence tuning preview failed and rollback could not restore its prior tuning, All/Family scope, and synchronized sample time: " +
				(RollbackError.empty() ? std::string("seek failed.") : RollbackError);
		return false;
	}
	m_strDetailStatus =
		"Occurrence tuning staged object-locally at the current synchronized sample time.";
	return true;
}

bool_t Client::CEffect_Tool::Stage_SourceAuthoringOverlayPreview(
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Overlay)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	const auto pPreparation = nullptr == pObject ? nullptr :
		pObject->Get_ReconstructedRuntimePreparation();
	if (nullptr == pObject || nullptr == pPreparation ||
		!m_bReconstructedSourceRuntimeActive)
	{
		m_strDetailStatus =
			"Artist F editing requires an active Track A source preview.";
		return false;
	}
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION PreviousIsolation =
		pObject->Get_PreviewSubmissionIsolation();
	const bool_t bPreviousPlaying = m_bPreviewPlaying;
	const bool_t bPreviousVisible = m_bPreviewVisibleRequested;
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT* pRollback =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_SourceAuthoringOverlayDocument->strEffectAssetId ==
			Overlay.strEffectAssetId ?
			&*m_SourceAuthoringOverlayDocument : nullptr;
	const auto StageNext =
		[this, &pObject, &pPreparation, &Overlay](std::string& strOutError)
		{
			return CEffectPresentationService::
				Stage_ReconstructedSourceAuthoringOverlayPreview(
					m_pDevice, m_pContext, pObject, pPreparation,
					Overlay, strOutError);
		};
	const auto StageRollback =
		[this, &pObject, &pPreparation, pRollback](std::string& strOutError)
		{
			if (nullptr == pRollback)
			{
				strOutError =
					"The prior Artist F edit overlay is unavailable.";
				return false;
			}
			return CEffectPresentationService::
				Stage_ReconstructedSourceAuthoringOverlayPreview(
					m_pDevice, m_pContext, pObject, pPreparation,
					*pRollback, strOutError);
		};
	const auto ApplyIsolation =
		[&pObject](const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
			std::string& strOutError)
		{
			if (Isolation.eKind ==
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL)
			{
				pObject->Reset_PreviewSubmissionIsolation();
				strOutError.clear();
				return true;
			}
			return pObject->Set_PreviewSubmissionIsolation(
				Isolation, strOutError);
		};
	const f32_t fPreviousTime = m_fPreviewTimeSeconds;
	std::string Error;
	if (!CEffectPresentationService::Restage_ObjectLocalOccurrencePreview(
			PreviousIsolation, StageNext, StageRollback, ApplyIsolation, Error))
	{
		const bool_t bRollbackFailed =
			std::string::npos != Error.find("Rollback failed:");
		m_bPreviewPlaying = !bRollbackFailed && bPreviousPlaying;
		m_bPreviewVisibleRequested = !bRollbackFailed && bPreviousVisible;
		pObject->Set_Playing(false);
		pObject->Set_Visible(m_bPreviewVisibleRequested);
		Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
		m_strDetailStatus = "Artist F edit preview failed: " + Error;
		return false;
	}
	if (!Seek_ReconstructedSourceRuntimeTimeline(fPreviousTime))
	{
		std::string RollbackError;
		const bool_t bRollbackStaged =
			CEffectPresentationService::Restage_ObjectLocalOccurrencePreview(
				PreviousIsolation, StageRollback, StageRollback,
				ApplyIsolation, RollbackError);
		const bool_t bRollbackSeeked = bRollbackStaged &&
			Seek_ReconstructedSourceRuntimeTimeline(fPreviousTime);
		m_bPreviewPlaying = bRollbackSeeked && bPreviousPlaying;
		m_bPreviewVisibleRequested = bRollbackSeeked && bPreviousVisible;
		pObject->Set_Playing(false);
		pObject->Set_Visible(m_bPreviewVisibleRequested);
		Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
		m_strDetailStatus = bRollbackSeeked ?
			"Artist F edit preview could not commit; the previous values, playback scope, and sample time were restored." :
			"Artist F edit preview failed and rollback could not restore the prior session: " +
				(RollbackError.empty() ? std::string("seek failed.") :
					RollbackError);
		return false;
	}
	m_bPreviewPlaying = bPreviousPlaying;
	m_bPreviewVisibleRequested = bPreviousVisible;
	pObject->Set_Playing(false);
	pObject->Set_Visible(bPreviousVisible);
	Set_SynchronizedAnimationPaused(!bPreviousPlaying);
	m_strDetailStatus =
		"Artist F source-backed preview updated at the current sample time.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ApplyRuntimeOccurrenceDraft()
{
	const bool_t bSourceBacked =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_strSelectedRuntimeOccurrenceEffectId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	if ((!bSourceBacked && !m_OccurrenceTuningDocument.has_value()) ||
		!m_OccurrenceTransformDraft.has_value() ||
		!m_bOccurrenceTransformDraftDirty)
		return false;
	if (bSourceBacked)
	{
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Staged =
			*m_SourceAuthoringOverlayDocument;
		Upsert_SourceAuthoringOverlayEntry(Staged,
			m_strSelectedRuntimeOccurrenceId,
			m_strSelectedRuntimeOccurrenceRowSha256,
			m_strSelectedRuntimeOccurrenceElementId,
			*m_OccurrenceTransformDraft);
		if (!Stage_SourceAuthoringOverlayPreview(Staged))
			return false;
		m_SourceAuthoringOverlayDocument = std::move(Staged);
		m_bOccurrenceTransformDraftDirty = false;
		m_bOccurrenceTuningDirty =
			CEffectSourceAuthoringOverlayCodec::Serialize(
				*m_SourceAuthoringOverlayDocument) !=
			m_strSourceAuthoringOverlayBaselineCanonical;
		m_strDetailStatus =
			"Applied this Element to the in-memory Artist F edit overlay; Save Changes is required to persist.";
		return true;
	}
	EFFECT_OCCURRENCE_TUNING_DOCUMENT Staged = *m_OccurrenceTuningDocument;
	Upsert_OccurrenceTuningEntry(Staged,
		m_strSelectedRuntimeOccurrenceId,
		m_strSelectedRuntimeOccurrenceRowSha256,
		*m_OccurrenceTransformDraft);
	if (!Stage_RuntimeOccurrenceTuningPreview(Staged))
		return false;
	m_OccurrenceTuningDocument = std::move(Staged);
	m_bOccurrenceTransformDraftDirty = false;
	m_bOccurrenceTuningDirty =
		CEffectOccurrenceTuningCodec::Serialize(*m_OccurrenceTuningDocument) !=
		m_strOccurrenceTuningBaselineCanonical;
	m_strDetailStatus =
		"Applied to the in-memory occurrence tuning artifact; Save is required to persist.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ResetRuntimeOccurrenceToSource()
{
	const bool_t bSourceBacked =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_strSelectedRuntimeOccurrenceEffectId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	if (!bSourceBacked && !m_OccurrenceTuningDocument.has_value())
		return false;
	if (bSourceBacked)
	{
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Staged =
			*m_SourceAuthoringOverlayDocument;
		Remove_SourceAuthoringOverlayEntry(
			Staged, m_strSelectedRuntimeOccurrenceId);
		if (!Stage_SourceAuthoringOverlayPreview(Staged))
			return false;
		m_SourceAuthoringOverlayDocument = std::move(Staged);
		m_OccurrenceTransformDraft = m_SelectedOccurrenceSourceTransform;
		m_bOccurrenceTransformDraftDirty = false;
		m_bOccurrenceTuningDirty =
			CEffectSourceAuthoringOverlayCodec::Serialize(
				*m_SourceAuthoringOverlayDocument) !=
			m_strSourceAuthoringOverlayBaselineCanonical;
		m_strDetailStatus =
			"Restored the immutable Track A source Transform; Save Changes is required to persist.";
		return true;
	}
	EFFECT_OCCURRENCE_TUNING_DOCUMENT Staged = *m_OccurrenceTuningDocument;
	Remove_OccurrenceTuningEntry(Staged, m_strSelectedRuntimeOccurrenceId);
	if (!Stage_RuntimeOccurrenceTuningPreview(Staged))
		return false;
	m_OccurrenceTuningDocument = std::move(Staged);
	m_OccurrenceTransformDraft = m_SelectedOccurrenceSourceTransform;
	m_bOccurrenceTransformDraftDirty = false;
	m_bOccurrenceTuningDirty =
		CEffectOccurrenceTuningCodec::Serialize(*m_OccurrenceTuningDocument) !=
		m_strOccurrenceTuningBaselineCanonical;
	m_strDetailStatus =
		"Removed this PROJECT_TUNED entry and restored the immutable source Transform; Save is required to persist.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SaveRuntimeOccurrenceTuning()
{
	const bool_t bSourceBacked =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_strSelectedRuntimeOccurrenceEffectId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	if (bSourceBacked)
	{
		if (m_SourceAuthoringOverlayPath.empty())
		{
			m_strDetailStatus =
				"There is no Artist F edit overlay path to save.";
			return false;
		}
		if (m_bOccurrenceTransformDraftDirty)
		{
			m_strDetailStatus =
				"Apply or Revert the Element Transform draft before saving.";
			return false;
		}
		const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
			pProgram = CEffectCatalog::Find_ReconstructedRuntimeProgram(
				m_SourceAuthoringOverlayDocument->strEffectAssetId);
		std::string Error;
		if (nullptr == pProgram ||
			!CEffectSourceAuthoringOverlayCodec::Save_AtomicIfUnchanged(
				m_SourceAuthoringOverlayPath,
				*m_SourceAuthoringOverlayDocument, *pProgram,
				m_strSourceAuthoringOverlayBaselineCanonical, Error))
		{
			m_strDetailStatus = nullptr == pProgram ?
				"Artist F Track A source Program is no longer admitted." : Error;
			return false;
		}
		m_strSourceAuthoringOverlayBaselineCanonical =
			CEffectSourceAuthoringOverlayCodec::Serialize(
				*m_SourceAuthoringOverlayDocument);
		m_bOccurrenceTuningDirty = false;
		m_bSourceAuthoringOverlayNeedsInitialSave = false;
		m_strDetailStatus =
			"Saved Artist F Element changes atomically. Track A source data and Product mapping were not modified.";
		return true;
	}
	if (!m_OccurrenceTuningDocument.has_value() ||
		m_OccurrenceTuningPath.empty())
	{
		m_strDetailStatus = "There is no occurrence tuning artifact to save.";
		return false;
	}
	if (m_bOccurrenceTransformDraftDirty)
	{
		m_strDetailStatus =
			"Apply or Revert the occurrence Transform draft before saving.";
		return false;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		CEffectCatalog::Find_ReconstructedRuntimeProgram(
			m_OccurrenceTuningDocument->strEffectAssetId);
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProjection = CEffectCatalog::Find_VisualProjection(
			m_OccurrenceTuningDocument->strEffectAssetId);
	std::string Error;
	const bool_t bSaved = nullptr != pVisualProjection ?
		CEffectOccurrenceTuningCodec::Save_AtomicIfUnchanged(
			m_OccurrenceTuningPath, *m_OccurrenceTuningDocument,
			*pVisualProjection, m_strOccurrenceTuningBaselineCanonical, Error) :
		(nullptr != pProgram &&
		 CEffectOccurrenceTuningCodec::Save_AtomicIfUnchanged(
			m_OccurrenceTuningPath, *m_OccurrenceTuningDocument, *pProgram,
			m_strOccurrenceTuningBaselineCanonical, Error));
	if (!bSaved)
	{
		m_strDetailStatus = nullptr == pProgram && nullptr == pVisualProjection ?
			"Occurrence tuning Program is no longer admitted by the catalog." : Error;
		return false;
	}
	m_strOccurrenceTuningBaselineCanonical =
		CEffectOccurrenceTuningCodec::Serialize(*m_OccurrenceTuningDocument);
	m_bOccurrenceTuningDirty = false;
	m_strDetailStatus =
		"Saved the reconstructed reference tuning source; direct-authored playback does not consume this lane.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ReloadRuntimeOccurrenceTuning()
{
	const bool_t bSourceBacked =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_strSelectedRuntimeOccurrenceEffectId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	if (bSourceBacked)
	{
		if (m_SourceAuthoringOverlayPath.empty() ||
			m_bOccurrenceTransformDraftDirty)
		{
			m_strDetailStatus =
				"Apply or Revert the current Element draft before Reload Saved.";
			return false;
		}
		const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
			pProgram = CEffectCatalog::Find_ReconstructedRuntimeProgram(
				m_SourceAuthoringOverlayDocument->strEffectAssetId);
		if (nullptr == pProgram)
		{
			m_strDetailStatus =
				"Artist F Track A source Program is no longer admitted.";
			return false;
		}
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Staged;
		Staged.strEffectAssetId = ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
		Staged.strSourceProgramSha256 = pProgram->Identity.strProgramSha256;
		Staged.SupplementalDocument =
			CEffectSourceAuthoringOverlayCodec::Create_EmptySupplementalDocument(
				ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
		std::string Error;
		std::error_code FileError;
		const bool_t bExists = std::filesystem::is_regular_file(
			m_SourceAuthoringOverlayPath, FileError);
		if (FileError ||
			(bExists && !CEffectSourceAuthoringOverlayCodec::Load(
				m_SourceAuthoringOverlayPath, Staged, Error)) ||
			!CEffectSourceAuthoringOverlayCodec::Validate_AgainstProgram(
				Staged, *pProgram, Error) ||
			!Stage_SourceAuthoringOverlayPreview(Staged))
		{
			if (!Error.empty())
				m_strDetailStatus = Error;
			else if (FileError)
				m_strDetailStatus = FileError.message();
			return false;
		}
		const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY* pEntry =
			CEffectSourceAuthoringOverlayCodec::Find_Entry(
				Staged, m_strSelectedRuntimeOccurrenceId);
		m_OccurrenceTransformDraft = nullptr == pEntry ?
			m_SelectedOccurrenceSourceTransform :
			pEntry->EffectiveLocalTransform;
		m_strSourceAuthoringOverlayBaselineCanonical = bExists ?
			CEffectSourceAuthoringOverlayCodec::Serialize(Staged) :
			std::string{};
		m_SourceAuthoringOverlayDocument = std::move(Staged);
		m_bOccurrenceTuningDirty = false;
		m_bOccurrenceTransformDraftDirty = false;
		m_bSourceAuthoringOverlayNeedsInitialSave = !bExists;
		m_strDetailStatus =
			"Reloaded and staged the saved Artist F Element changes transactionally.";
		return true;
	}
	if (!m_OccurrenceTuningDocument.has_value() ||
		m_OccurrenceTuningPath.empty() || m_bOccurrenceTuningDirty ||
		m_bOccurrenceTransformDraftDirty)
	{
		m_strDetailStatus =
			"Reload requires a clean occurrence tuning session.";
		return false;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		CEffectCatalog::Find_ReconstructedRuntimeProgram(
			m_OccurrenceTuningDocument->strEffectAssetId);
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProjection = CEffectCatalog::Find_VisualProjection(
			m_OccurrenceTuningDocument->strEffectAssetId);
	EFFECT_OCCURRENCE_TUNING_DOCUMENT Staged;
	std::string Error;
	const bool_t bLoaded =
		CEffectOccurrenceTuningCodec::Load(
			m_OccurrenceTuningPath, Staged, Error);
	const bool_t bValidated = bLoaded &&
		(nullptr != pVisualProjection ?
			CEffectOccurrenceTuningCodec::Validate_AgainstProjection(
				Staged, *pVisualProjection, Error) :
			(nullptr != pProgram &&
			 CEffectOccurrenceTuningCodec::Validate_AgainstProgram(
				Staged, *pProgram, Error)));
	if (!bValidated ||
		!Stage_RuntimeOccurrenceTuningPreview(Staged))
	{
		if (!Error.empty())
			m_strDetailStatus = Error;
		return false;
	}
	const EFFECT_OCCURRENCE_TUNING_ENTRY* pEntry =
		CEffectOccurrenceTuningCodec::Find_Entry(
			Staged, m_strSelectedRuntimeOccurrenceId);
	m_OccurrenceTransformDraft = nullptr == pEntry ?
		m_SelectedOccurrenceSourceTransform : pEntry->EffectiveLocalTransform;
	m_strOccurrenceTuningBaselineCanonical =
		CEffectOccurrenceTuningCodec::Serialize(Staged);
	m_OccurrenceTuningDocument = std::move(Staged);
	m_bOccurrenceTuningDirty = false;
	m_bOccurrenceTransformDraftDirty = false;
	m_bSourceAuthoringOverlayNeedsInitialSave = false;
	m_strDetailStatus =
		"Reloaded and staged the saved source occurrence tuning artifact transactionally.";
	return true;
}

void Client::CEffect_Tool::Reset_RuntimeOccurrenceTuningSession()
{
	m_OccurrenceTuningDocument.reset();
	m_SourceAuthoringOverlayDocument.reset();
	m_OccurrenceTransformDraft.reset();
	m_OccurrenceTuningPath.clear();
	m_SourceAuthoringOverlayPath.clear();
	m_strOccurrenceTuningBaselineCanonical.clear();
	m_strSourceAuthoringOverlayBaselineCanonical.clear();
	m_strSelectedRuntimeOccurrenceEffectId.clear();
	m_strSelectedRuntimeOccurrenceId.clear();
	m_strSelectedRuntimeOccurrenceRowSha256.clear();
	m_strSelectedRuntimeOccurrenceElementId.clear();
	m_strSelectedRuntimeOccurrenceEmitterPath.clear();
	m_SelectedOccurrenceSourceTransform = {};
	m_pSelectedVisualSourceProjection.reset();
	m_bOccurrenceTuningDirty = false;
	m_bOccurrenceTransformDraftDirty = false;
	m_bSourceAuthoringOverlayNeedsInitialSave = false;
	if (EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE == m_eDetailSelection)
		m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
}
