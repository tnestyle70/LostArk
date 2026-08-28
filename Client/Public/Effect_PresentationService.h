#pragma once

#include "AnimationEffectCueDocument.h"
#include "Client_Defines.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentRenderer.h"
#include "Effect_OccurrenceTuning.h"
#include "Effect_ProductPrewarmQueue.h"
#include "Effect_ReconstructedExecution.h"
#include "ValtanPatternEffectCueDocument.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CCharacter;
class CEffectLoadPreparationJob;
class CEffectObject;
class CEffectScreenOverlayPresentation;
class CValtan;

struct EFFECT_SPAWN_DESC final
{
    std::string strEffectAssetId;
	std::weak_ptr<CCharacter> pOwner;
	std::weak_ptr<CValtan> pBossOwner;
    std::string strAnchorSlotId = "root";
    EFFECT_TRANSFORM_DESC LocalTransform{};
    EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
	VALTAN_PATTERN_EFFECT_SCALE_POLICY eScalePolicy =
		VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
	float3_t vWorldScale{ 1.f, 1.f, 1.f };
	EFFECT_ORIENTATION_POLICY eOrientationPolicy =
		EFFECT_ORIENTATION_POLICY::ANCHOR;
	/* Valid only for ACTION_FACING and captured with iActionStartTick from the
	   same authoritative Player snapshot. */
	bool_t bHasActionFacingYaw = false;
	f32_t fActionFacingYawDegrees = 0.f;
    EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
    uint32_t iCueDurationMs = 0u;
    uint32_t iActionStartTick = 0u;
    uint32_t iCueStartMs = 0u;
	std::string strOccurrenceId;
    f32_t fPlaybackRate = 1.f;
    f32_t fInitialSampleTimeSeconds = 0.f;
	// Dedicated world-root entry points are the only callers that set these.
	// The boss owner remains the budget/lifetime owner; this matrix is the
	// independent network world anchor.
	bool_t bUseWorldRoot = false;
	uint64_t iWorldRootHandle = 0u;
	float4x4_t WorldRoot{};
	/* Set only by Spawn_LevelPlacement.  It is a real level-lifetime owner,
	   not a null/fake Character or Boss weak pointer. */
	bool_t bLevelOwned = false;
	uint32_t iLevelOwnerIndex = ETOUI(LEVEL::END);
	bool_t bExternallySampled = false;
	std::string strLevelPlacementId;
};

struct EFFECT_WORLD_ROOT_HANDLE final
{
	uint64_t iValue = 0u;
	bool_t Is_Valid() const { return 0u != iValue; }
};

struct EFFECT_WORLD_ROOT_SPAWN_DESC final
{
	std::string strEffectAssetId;
	std::weak_ptr<CValtan> pBossBudgetAndLifetimeOwner;
	float4x4_t RootWorld{};
	std::string strOccurrenceId;
	uint32_t iSpawnTick = 0u;
	f32_t fInitialSampleTimeSeconds = 0.f;
};

struct EFFECT_LEVEL_PLACEMENT_SPAWN_DESC final
{
	uint32_t iLevelIndex = ETOUI(LEVEL::END);
	std::string strPlacementId;
	std::string strEffectAssetId;
	float4x4_t RootWorld{};
	uint32_t iSpawnTick = 0u;
	f32_t fInitialSampleTimeSeconds = 0.f;
	bool_t bExternallySampled = false;
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
	uint64_t iScreenOverlays = 0u;
	uint64_t iEstimatedDrawSubmissions = 0u;

	bool operator==(const EFFECT_SCENE_BUDGET_COST&) const = default;
};

/* Immutable Loader-worker candidate for one Product target.  Catalog parsing,
   overlay resource validation, budget/duration derivation and D3D11
   device-object creation finish before this object crosses to the owner thread.
   Nothing in this bundle is globally published until the owner commits it. */
struct EFFECT_PRODUCT_LOADING_TARGET_STAGE final
{
	std::shared_ptr<const EFFECT_PRODUCT_LOAD_STAGE_RESULT> pCatalogStage;
	std::shared_ptr<CEffectDocumentRenderer::PRODUCT_TARGET_STAGE>
		pRendererStage;
	EFFECT_SCENE_BUDGET_COST BudgetCost;
	f32_t fPlaybackDurationSeconds = 0.f;
	std::shared_ptr<const CEffectScreenOverlayPresentation>
		pScreenOverlayTemplate;
};

struct EFFECT_SCENE_BUDGET_PROBE final
{
	EFFECT_SCENE_BUDGET_COST Active;
	EFFECT_SCENE_BUDGET_COST Pending;
	uint64_t iRejectedSpawnCount = 0u;
};

struct EFFECT_BOSS_ACTION_STOP_RESULT final
{
	uint64_t iPendingStopped = 0u;
	uint64_t iActiveStopped = 0u;
	uint64_t iActiveRetainedNatural = 0u;

	bool operator==(const EFFECT_BOSS_ACTION_STOP_RESULT&) const = default;
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
	/* Effect Tool and product runtime must use the same owner-basis replacement
	   for Valtan cue-v3 scale policies.  These helpers validate the sampled
	   owner transform and never mutate it on failure. */
	static bool_t Build_CueScalePolicyAnchor(
		VALTAN_PATTERN_EFFECT_SCALE_POLICY eScalePolicy,
		const float3_t& WorldScale,
		const float4x4_t& SampledOwnerAnchor,
		float4x4_t& OutAnchor);
	static bool_t Build_CueScalePolicyRoot(
		const EFFECT_TRANSFORM_DESC& LocalTransform,
		VALTAN_PATTERN_EFFECT_SCALE_POLICY eScalePolicy,
		const float3_t& WorldScale,
		const float4x4_t& SampledOwnerAnchor,
		float4x4_t& OutRoot);
    static bool_t Queue_ProductCues(
        const std::vector<ANIMATION_EFFECT_CUE>& Cues,
        std::string& strOutStatus);
	static bool_t Queue_ProductCues_Priority(
		const std::vector<ANIMATION_EFFECT_CUE>& Cues,
		std::vector<std::string>& OutEffectAssetIds,
		std::string& strOutStatus);
	/* Boss and other action-qualified presentation contracts are not clip cue
	   documents.  This typed target-only entry point reuses the same priority
	   preparation queue without fabricating animation clip ownership. */
	static bool_t Queue_ProductTargets_Priority(
		const std::vector<std::string>& EffectAssetIds,
		std::vector<std::string>& OutEffectAssetIds,
		std::string& strOutStatus);
	static EFFECT_PRODUCT_PREWARM_TARGET_PROBE
		Get_ProductCuePreparationProbe(
			const std::vector<std::string>& EffectAssetIds);
	static bool_t Begin_LoadingProductCuePreparation(
		const std::shared_ptr<CEffectLoadPreparationJob>& pJob,
		uint64_t iJobEpoch,
		const std::vector<std::string>& EffectAssetIds,
		std::string& strOutStatus);
	/* Loader-worker-only stage.  The context argument is retained solely as the
	   immutable identity stored by prepared resources; this path never invokes
	   immediate-context methods or publishes catalog/renderer state. */
	static bool_t Stage_LoadingProductTarget(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContextIdentity,
		const EFFECT_PRODUCT_LOAD_STAGE_REQUEST& Request,
		std::shared_ptr<const EFFECT_PRODUCT_LOADING_TARGET_STAGE>& OutStage,
		std::string& strOutStatus);
	/* Main-thread non-blocking result consumer.  It commits at most one staged
	   worker result and never waits for the Loader producer. */
	static void Advance_LoadingProductCuePreparation(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const std::shared_ptr<CEffectLoadPreparationJob>& pJob,
		uint64_t iJobEpoch);
	static void Cancel_LoadingProductCuePreparation(
		const std::shared_ptr<CEffectLoadPreparationJob>& pJob,
		uint64_t iJobEpoch);
	/* Natural late-join scheduling consults only metadata committed with the
	   prepared Product target.  It never loads or stages an Effect from a boss
	   snapshot callback. */
	static bool_t Try_Get_PreparedProductDurationSeconds(
		const std::string& strEffectAssetId,
		f32_t& fOutDurationSeconds);
	/* Called once from the main thread.  It consumes at most one queued
	   document and performs no work on the registration frame. */
	static void Advance_ProductCuePreparation(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	/* Commits one already-staged catalog candidate into the Product GPU cache.
	   The catalog revision and every unrelated prepared target remain unchanged.
	   Active occurrences retain their old shared resources; the candidate and
	   its budget/duration receipts are used by subsequent spawns only. */
	static bool_t Replace_ProductPreparedTarget(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint64_t iCatalogRevision,
		const std::string& strEffectAssetId,
		std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pVisualProgramProjection,
		std::string& strOutStatus);
	/* Save-time Debug transaction for exactly one direct-authored Effect.  The
	   catalog stages and commits the new immutable document at the same runtime
	   revision, then the renderer/queue/budget/duration selected-target caches are
	   replaced.  Any preparation failure restores the old catalog pointers and
	   leaves the old prepared target available. */
	static bool_t Reload_SelectedProductEffect(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const std::string& strEffectAssetId,
		const std::filesystem::path& AuthoredPath,
		std::string& strOutStatus);
    static bool_t Reprepare_ProductTargets(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext,
        const std::vector<std::string>& AdditionalEffectAssetIds,
        std::string& strOutStatus);
    static bool_t Spawn(
        const EFFECT_SPAWN_DESC& Desc,
        std::string& strOutStatus);
	static bool_t Spawn_WorldRoot(
		const EFFECT_WORLD_ROOT_SPAWN_DESC& Desc,
		EFFECT_WORLD_ROOT_HANDLE& OutHandle,
		std::string& strOutStatus);
	// RootWorld already includes the cue local transform and scale policy.
	// Preserve the cue clock/stop policy while allocating through the same path.
	static bool_t Spawn_WorldRoot(
		const EFFECT_SPAWN_DESC& CueDesc,
		const float4x4_t& RootWorld,
		EFFECT_WORLD_ROOT_HANDLE& OutHandle,
		std::string& strOutStatus);
	static bool_t Spawn_LevelPlacement(
		const EFFECT_LEVEL_PLACEMENT_SPAWN_DESC& Desc,
		EFFECT_WORLD_ROOT_HANDLE& OutHandle,
		std::string& strOutStatus);
	static bool_t Update_WorldRoot(
		EFFECT_WORLD_ROOT_HANDLE Handle,
		const float4x4_t& RootWorld);
	static bool_t Seek_WorldRoot(
		EFFECT_WORLD_ROOT_HANDLE Handle,
		f32_t fSampleTimeSeconds);
	static void Stop_WorldRoot(EFFECT_WORLD_ROOT_HANDLE Handle);
	/* Product cue requests can originate while Object Manager is iterating its
	   layer map.  Commit them only after Update_Engine finishes. */
	static void Commit_PendingSpawns();
	/* Commits only the listed pending world-root spawns.  This is used by a
	   staged aggregate admission probe and deliberately leaves every unrelated
	   gameplay request queued for the normal post-update commit seam. */
	static void Commit_PendingWorldRootSpawns(
		const std::vector<EFFECT_WORLD_ROOT_HANDLE>& Handles);
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
	/* A replicated boss stage owns every queued cue and every active cue created
	   from its non-zero actionStartTick.  Stage replacement cancels queued work
	   and active CUE_END work; an already-active NATURAL cue keeps updating until
	   its document finishes.  Stop_BossOwner remains the unconditional death,
	   despawn and level-teardown boundary. */
	static EFFECT_BOSS_ACTION_STOP_RESULT Stop_BossAction(
		const std::shared_ptr<CValtan>& pOwner,
		uint32_t iActionStartTick);
	static void Stop_BossOwner(const std::shared_ptr<CValtan>& pOwner);
    static void Clear_Level(uint32_t iLevelIndex);
    static void Clear_All();
	/* Requires Clear_All while ObjectManager is alive and a quiescent Loading
	   worker.  This function releases only process-global prepared/cache state. */
    static void Release_PreparedResources();
    static const std::string& Get_Status();

private:
	static bool_t Spawn_Immediate(
		const EFFECT_SPAWN_DESC& Desc,
		std::string& strOutStatus);
};

NS_END

