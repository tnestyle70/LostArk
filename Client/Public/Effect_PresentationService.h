#pragma once

#include "AnimationEffectCueDocument.h"
#include "Client_Defines.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentRenderer.h"
#include "Effect_OccurrenceTuning.h"
#include "Effect_ReconstructedExecution.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CCharacter;
class CEffectObject;
class CValtan;

struct EFFECT_SPAWN_DESC final
{
    std::string strEffectAssetId;
	std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION_TOKEN>
		pProductAdmissionToken;
	std::weak_ptr<CCharacter> pOwner;
	std::weak_ptr<CValtan> pBossOwner;
    std::string strAnchorSlotId = "root";
    EFFECT_TRANSFORM_DESC LocalTransform{};
    EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
    EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
    uint32_t iCueDurationMs = 0u;
    uint32_t iActionStartTick = 0u;
    uint32_t iCueStartMs = 0u;
	std::string strOccurrenceId;
    f32_t fPlaybackRate = 1.f;
    f32_t fInitialSampleTimeSeconds = 0.f;
};

struct EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC final
{
	float4x4_t RawBone{};
	float4x4_t OwnerWorld{};
};

/* Conservative admission cost computed once from an immutable prepared
	Document.  These are capacity/upper-bound units, not a claim about the exact
	GPU work of a particular frame. */
struct EFFECT_SCENE_BUDGET_COST final
{
	uint64_t iEffects = 0u;
	uint64_t iParticles = 0u;
	uint64_t iMeshParticles = 0u;
	uint64_t iTrailPoints = 0u;
	uint64_t iAfterImages = 0u;
	uint64_t iLights = 0u;
	uint64_t iScreenPosts = 0u;
	uint64_t iEstimatedDrawSubmissions = 0u;

	bool operator==(const EFFECT_SCENE_BUDGET_COST&) const = default;
};

struct EFFECT_SCENE_BUDGET_PROBE final
{
	EFFECT_SCENE_BUDGET_COST Active;
	EFFECT_SCENE_BUDGET_COST Pending;
	uint64_t iRejectedSpawnCount = 0u;
};

/* Opaque process-local identities for the one immutable Artist F Core33 cache.
   The Tool and authoritative gameplay paths publish successful consumption
   receipts so an executable harness can prove that neither path rebuilt or
   substituted the document/prepared-resource pair. */
struct EFFECT_ARTIST_31470_CACHE_IDENTITY final
{
	uint64_t iGeneration = 0u;
	uint64_t iCatalogRevision = 0u;
	std::uintptr_t iPreparationIdentity = 0u;
	std::uintptr_t iDocumentIdentity = 0u;
	std::uintptr_t iPreparedDocumentIdentity = 0u;
	EFFECT_RECONSTRUCTED_VISUAL_SCOPE eVisualScope =
		EFFECT_RECONSTRUCTED_VISUAL_SCOPE::END;
	uint32_t iDocumentElementCount = 0u;
	uint32_t iVisibleElementCount = 0u;

	bool_t Is_ExactCoreScope() const
	{
		return iGeneration != 0u && iCatalogRevision != 0u &&
			iPreparationIdentity != 0u && iDocumentIdentity != 0u &&
			iPreparedDocumentIdentity != 0u &&
			eVisualScope == EFFECT_RECONSTRUCTED_VISUAL_SCOPE::CORE_RENDERERS &&
			iDocumentElementCount == 35u && iVisibleElementCount == 33u;
	}

	bool_t Matches(const EFFECT_ARTIST_31470_CACHE_IDENTITY& Other) const
	{
		return iGeneration == Other.iGeneration &&
			iCatalogRevision == Other.iCatalogRevision &&
			iPreparationIdentity == Other.iPreparationIdentity &&
			iDocumentIdentity == Other.iDocumentIdentity &&
			iPreparedDocumentIdentity == Other.iPreparedDocumentIdentity &&
			eVisualScope == Other.eVisualScope &&
			iDocumentElementCount == Other.iDocumentElementCount &&
			iVisibleElementCount == Other.iVisibleElementCount;
	}
};

struct EFFECT_ARTIST_31470_CACHE_PROBE final
{
	EFFECT_ARTIST_31470_CACHE_IDENTITY Current;
	EFFECT_ARTIST_31470_CACHE_IDENTITY LastToolPreviewConsumption;
	EFFECT_ARTIST_31470_CACHE_IDENTITY LastGameplayConsumption;
	uint64_t iToolPreviewConsumeCount = 0u;
	uint64_t iGameplayConsumeCount = 0u;
};

struct EFFECT_RECONSTRUCTED_OCCURRENCE_INFO final
{
	std::string strEffectAssetId;
	std::string strOccurrenceId;
	std::string strSourceOccurrenceRowSha256;
	std::string strSourceElementId;
	std::string strSourceEmitterPath;
	EFFECT_RUNTIME_RENDERER_KIND eRenderer =
		EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE;
	EFFECT_OCCURRENCE_LOCAL_TRANSFORM SourceLocalTransform;
	EFFECT_OCCURRENCE_LOCAL_TRANSFORM EffectiveLocalTransform;
};

class CEffectPresentationService final
{
public:
	static bool_t Estimate_DocumentBudget(
		const EFFECT_DOCUMENT_DESC& Document,
		EFFECT_SCENE_BUDGET_COST& OutCost,
		std::string& strOutStatus);
	static EFFECT_SCENE_BUDGET_PROBE Get_SceneBudgetProbe();
	static bool_t Build_SourceBoneAnchorWorld(
		const EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC& Desc,
		float4x4_t& OutWorld);
    static bool_t Queue_ProductCues(
        const std::vector<ANIMATION_EFFECT_CUE>& Cues,
        std::string& strOutStatus);
	/* Called once from the main thread.  It consumes at most one queued
	   document and performs no work on the registration frame. */
	static void Advance_ProductCuePreparation(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
    static bool_t Reprepare_ProductTargets(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext,
        const std::vector<std::string>& AdditionalEffectAssetIds,
        std::string& strOutStatus);
    static bool_t Spawn(
        const EFFECT_SPAWN_DESC& Desc,
        std::string& strOutStatus);
	/* Product cue requests can originate while Object Manager is iterating its
	   layer map.  Commit them only after Update_Engine finishes. */
	static void Commit_PendingSpawns();
	static bool_t Prepare_ReconstructedRuntimeProgram(
		const std::string& strEffectAssetId,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			OutPreparation,
		std::string& strOutStatus);
	/* Debug/non-Product Artist F keeps its source runtime separate from Product
	   cue admission.  Preparation performs all document/resource work before an
	   authoritative action edge; Spawn only attaches the immutable result. */
	static bool_t Prepare_ReconstructedArtist31470(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::string& strOutStatus);
	static bool_t Acquire_ReconstructedArtist31470(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			OutPreparation,
		std::string& strOutStatus);
	static bool_t Stage_ReconstructedArtist31470Preview(
		const std::shared_ptr<CEffectObject>& pObject,
		const std::shared_ptr<const
			EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pExpectedPreparation,
		std::string& strOutStatus);
	static bool_t Get_ReconstructedOccurrenceInfo(
		const std::string& strEffectAssetId,
		const std::string& strOccurrenceId,
		EFFECT_RECONSTRUCTED_OCCURRENCE_INFO& OutInfo,
		std::string& strOutStatus);
	static bool_t Stage_ReconstructedOccurrenceTuningPreview(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const std::shared_ptr<CEffectObject>& pObject,
		const std::shared_ptr<const
			EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pExpectedPreparation,
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Tuning,
		std::string& strOutStatus);
	static bool_t Stage_ReconstructedSourceAuthoringOverlayPreview(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const std::shared_ptr<CEffectObject>& pObject,
		const std::shared_ptr<const
			EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pExpectedPreparation,
		const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Overlay,
		std::string& strOutStatus);
	/* Details selection is independent from submission isolation. A restage
	   resets submission to ALL, so preserve the prior explicit All, carrier
	   Family, stable occurrence Solo, or canonical visual-family ElementSet
	   transactionally. */
	static EFFECT_PREVIEW_SUBMISSION_ISOLATION
		Normalize_OccurrencePreviewIsolation(
			const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Previous)
	{
		EFFECT_PREVIEW_SUBMISSION_ISOLATION Normalized;
		if (Previous.eKind ==
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL)
		{
			return Normalized;
		}
		if (Previous.eKind ==
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::FAMILY &&
			Previous.strElementId.empty() && Previous.ElementIds.empty() &&
			(Previous.eFamily == EFFECT_GPU_RENDER_FAMILY::MESH ||
			 Previous.eFamily == EFFECT_GPU_RENDER_FAMILY::SPRITE ||
			 Previous.eFamily == EFFECT_GPU_RENDER_FAMILY::DECAL ||
			 Previous.eFamily == EFFECT_GPU_RENDER_FAMILY::RIBBON))
		{
			Normalized.eKind =
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::FAMILY;
			Normalized.eFamily = Previous.eFamily;
			return Normalized;
		}
		if (Previous.eKind ==
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::OCCURRENCE &&
			Previous.eFamily == EFFECT_GPU_RENDER_FAMILY::END &&
			!Previous.strElementId.empty() && Previous.ElementIds.empty())
		{
			Normalized.eKind =
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::OCCURRENCE;
			Normalized.strElementId = Previous.strElementId;
			return Normalized;
		}
		if (Previous.eKind ==
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET &&
			Previous.eFamily == EFFECT_GPU_RENDER_FAMILY::END &&
			Previous.strElementId.empty() && !Previous.ElementIds.empty())
		{
			Normalized.eKind =
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET;
			Normalized.ElementIds = Previous.ElementIds;
			return Normalized;
		}
		return Normalized;
	}

	template <typename StageNextFn, typename StageRollbackFn,
		typename ApplyIsolationFn>
	static bool_t Restage_ObjectLocalOccurrencePreview(
		const EFFECT_PREVIEW_SUBMISSION_ISOLATION& PreviousIsolation,
		StageNextFn&& StageNext,
		StageRollbackFn&& StageRollback,
		ApplyIsolationFn&& ApplyIsolation,
		std::string& strOutStatus)
	{
		const EFFECT_PREVIEW_SUBMISSION_ISOLATION Normalized =
			Normalize_OccurrencePreviewIsolation(PreviousIsolation);
		std::string StageError;
		const bool_t bStaged = StageNext(StageError);
		std::string IsolationError;
		const bool_t bIsolationRestored = bStaged &&
			ApplyIsolation(Normalized, IsolationError);
		if (bStaged && bIsolationRestored)
		{
			strOutStatus.clear();
			return true;
		}

		const std::string PrimaryError = bStaged ?
			(IsolationError.empty() ?
				"Occurrence preview isolation restore failed." : IsolationError) :
			(StageError.empty() ?
				"Occurrence preview restage failed." : StageError);
		std::string RollbackStageError;
		const bool_t bRollbackStaged = StageRollback(RollbackStageError);
		std::string RollbackIsolationError;
		const bool_t bRollbackIsolationRestored =
			ApplyIsolation(Normalized, RollbackIsolationError);
		strOutStatus = PrimaryError;
		if (!bRollbackStaged || !bRollbackIsolationRestored)
		{
			strOutStatus += " Rollback failed:";
			if (!bRollbackStaged)
			{
				strOutStatus += " " + (RollbackStageError.empty() ?
					std::string("previous tuning restage failed.") :
					RollbackStageError);
			}
			if (!bRollbackIsolationRestored)
			{
				strOutStatus += " " + (RollbackIsolationError.empty() ?
					std::string("previous isolation restore failed.") :
					RollbackIsolationError);
			}
		}
		return false;
	}
	static bool_t Spawn_ReconstructedArtist31470(
		const EFFECT_SPAWN_DESC& Desc,
		std::string& strOutStatus);
	static bool_t Get_ReconstructedArtist31470CacheProbe(
		EFFECT_ARTIST_31470_CACHE_PROBE& OutProbe,
		std::string& strOutStatus);
    static void Update(f32_t fTimeDelta);
    static void Synchronize_FollowAnchors();
    static void Stop_Owner(const std::shared_ptr<CCharacter>& pOwner);
	static void Stop_BossOwner(const std::shared_ptr<CValtan>& pOwner);
    static void Clear_Level(uint32_t iLevelIndex);
    static void Clear_All();
    static void Release_PreparedResources();
    static const std::string& Get_Status();

private:
	static bool_t Spawn_Immediate(
		const EFFECT_SPAWN_DESC& Desc,
		std::string& strOutStatus);
};

NS_END

