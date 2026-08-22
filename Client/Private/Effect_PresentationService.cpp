#include "Effect_PresentationService.h"

#include "Character.h"
#include "Effect_Catalog.h"
#include "Effect_Object.h"
#include "Effect_ProductPrewarmQueue.h"
#include "Effect_ReconstructedExecution.h"
#include "Effect_ScreenOverlayPresentation.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Transform.h"
#include "Valtan.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    constexpr const wchar_t* EFFECT_LAYER = L"Layer_Effect";
	constexpr const char_t* RECONSTRUCTED_ARTIST_31470_ASSET_ID =
		"effect.artist.skill.31470";
	constexpr Client::EFFECT_RECONSTRUCTED_VISUAL_SCOPE
		RECONSTRUCTED_ARTIST_31470_VISUAL_SCOPE =
		Client::EFFECT_RECONSTRUCTED_VISUAL_SCOPE::CORE_RENDERERS;
	constexpr uint32_t RECONSTRUCTED_ARTIST_31470_ELEMENT_COUNT = 35u;
	constexpr uint32_t RECONSTRUCTED_ARTIST_31470_VISIBLE_ELEMENT_COUNT = 33u;
	constexpr size_t ARTIST_31470_ANCHOR_BINDING_COUNT = 5u;
	constexpr const char_t* ARTIST_31470_RUNTIME_ANCHOR_SLOT_ID =
		"WP_SDM_R_Battle";
	constexpr const char_t* ARTIST_31470_RUNTIME_BONE_NAME = "b_wp_1";
	constexpr const char_t* ARTIST_31470_ANIMATION_CLIP_NAME =
		"sdm_sk_onestroke";
	constexpr f64_t ARTIST_31470_FIXED_STEP_SECONDS_EXACT = 1.0 / 60.0;
	constexpr f32_t ARTIST_31470_TRACK_TOLERANCE_TICKS = 0.01f;
	constexpr size_t ARTIST_31470_MAX_HISTORY_SAMPLE_COUNT = 4096u;
	// The playable Artist prototype contributes a 0.0001 admission transform,
	// while the Artist rig's sdm root contributes a scale of 100. b_wp_1 is a
	// combined bone matrix, so its admitted effective basis is 0.01. Keep this
	// contract Artist-specific and fail closed instead of normalizing an
	// arbitrary observed scale.
	constexpr f32_t ARTIST_SOURCE_BONE_COMBINED_SCALE = 0.01f;
	constexpr f32_t ARTIST_SOURCE_BONE_COMBINED_SCALE_TOLERANCE = 0.00005f;
	constexpr f32_t ARTIST_SOURCE_BONE_UNIFORM_SCALE_TOLERANCE = 0.000005f;
	constexpr f32_t SOURCE_BONE_ORTHOGONAL_TOLERANCE = 0.001f;
	constexpr f32_t SOURCE_BONE_AFFINE_TOLERANCE = 0.00001f;
	constexpr f32_t SOURCE_BONE_MINIMUM_BASIS_LENGTH = 0.00000001f;
	constexpr f32_t SOURCE_BONE_MINIMUM_NORMALIZED_DETERMINANT = 0.0001f;

    struct SOURCE_ANCHOR_REQUEST final
    {
        std::string strRuntimeAnchorSlotId;
        std::string strRuntimeBoneName;
        Client::EFFECT_TRANSFORM_DESC SocketLocalTransform{};
		bool_t bNormalizeSourceImportScale = false;
    };

	struct ARTIST_31470_ANCHOR_HISTORY_SAMPLE final
	{
		f32_t fSampleTimeSeconds = 0.f;
		float4x4_t AnchorWorld{};
	};

	struct ARTIST_31470_TRANSFORM_HISTORY final
	{
		bool_t bEnabled = false;
		std::weak_ptr<Engine::CModel> pModel;
		uint32_t iAnimationIndex = UINT32_MAX;
		std::string strAnimationClipName;
		f32_t fAnimationTickRate = 0.f;
		f32_t fAnimationDurationTicks = 0.f;
		f32_t fAnimationDurationSeconds = 0.f;
		float4x4_t ActionStartRootWorld{};
		std::array<uint32_t, ARTIST_31470_ANCHOR_BINDING_COUNT> BoneIndices{};
		std::vector<ARTIST_31470_ANCHOR_HISTORY_SAMPLE> Samples;
	};

	struct EFFECT_OWNER_VIEW final
	{
		std::shared_ptr<Client::CCharacter> pCharacter;
		std::shared_ptr<Client::CValtan> pBoss;

		bool_t Is_Valid() const
		{
			return (nullptr != pCharacter) != (nullptr != pBoss);
		}

		bool_t Try_Get_PresentationRoot(float4x4_t& Out) const
		{
			if (nullptr != pCharacter)
			{
				const std::shared_ptr<Engine::CTransform> pTransform =
					pCharacter->Get_Transform();
				if (nullptr == pTransform)
					return false;
				Out = *pTransform->Get_WorldMatrixPtr();
				return true;
			}
			return nullptr != pBoss &&
				pBoss->Try_Get_PresentationRootMatrix(&Out);
		}

		std::shared_ptr<Engine::CModel> Get_Model() const
		{
			return nullptr != pCharacter ? pCharacter->Get_BodyModel() :
				(nullptr != pBoss ? pBoss->Get_BodyModel() : nullptr);
		}
	};

    struct ACTIVE_EFFECT final
    {
        std::shared_ptr<Client::CEffectObject> pObject;
        std::weak_ptr<Client::CCharacter> pOwner;
		std::weak_ptr<Client::CValtan> pBossOwner;
        uint32_t iLevelIndex = UINT32_MAX;
        std::string strEffectAssetId;
        std::string strAnchorSlotId;
        Client::EFFECT_TRANSFORM_DESC LocalTransform{};
        Client::EFFECT_FOLLOW_POLICY eFollowPolicy =
            Client::EFFECT_FOLLOW_POLICY::FOLLOW;
		Client::EFFECT_ORIENTATION_POLICY eOrientationPolicy =
			Client::EFFECT_ORIENTATION_POLICY::ANCHOR;
		f32_t fActionFacingYawDegrees = 0.f;
        Client::EFFECT_STOP_POLICY eStopPolicy =
            Client::EFFECT_STOP_POLICY::NATURAL;
        uint32_t iCueDurationMs = 0u;
        uint32_t iActionStartTick = 0u;
        uint32_t iCueStartMs = 0u;
		std::string strOccurrenceId;
        f32_t fPlaybackRate = 1.f;
        f32_t fElapsedCueTimeSeconds = 0.f;
        f32_t fPendingInitialSampleTimeSeconds = 0.f;
        bool_t bPendingInitialSeek = true;
		bool_t bFollowAnchorMissing = false;
		std::vector<SOURCE_ANCHOR_REQUEST> SourceAnchorRequests;
		std::unordered_map<std::string, float4x4_t> SourceAnchorWorldsScratch;
		ARTIST_31470_TRANSFORM_HISTORY Artist31470TransformHistory;
		Client::EFFECT_SCENE_BUDGET_COST AdmissionCost;
		uint64_t iWorldRootHandle = 0u;
		float4x4_t WorldRoot{};
    };

	EFFECT_OWNER_VIEW Resolve_Owner(
		const Client::EFFECT_SPAWN_DESC& Desc)
	{
		return { Desc.pOwner.lock(), Desc.pBossOwner.lock() };
	}

	EFFECT_OWNER_VIEW Resolve_Owner(const ACTIVE_EFFECT& Effect)
	{
		return { Effect.pOwner.lock(), Effect.pBossOwner.lock() };
	}

	bool_t Same_Owner(
		const EFFECT_OWNER_VIEW& Left,
		const EFFECT_OWNER_VIEW& Right)
	{
		return Left.pCharacter == Right.pCharacter && Left.pBoss == Right.pBoss;
	}

	struct RECONSTRUCTED_SOURCE_RUNTIME_CACHE final
	{
		std::shared_ptr<const Client::EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation;
		std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> pDocument;
		std::shared_ptr<const
			Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pVisualProjection;
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPrepared;
		Client::EFFECT_RECONSTRUCTED_VISUAL_SCOPE eVisualScope =
			Client::EFFECT_RECONSTRUCTED_VISUAL_SCOPE::END;
		uint64_t iGeneration = 0u;
		uint32_t iVisibleElementCount = 0u;
	};

	struct RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW final
	{
		std::shared_ptr<const Client::EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation;
		std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> pDocument;
		std::shared_ptr<const
			Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pVisualProjection;
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPrepared;
		Client::EFFECT_ARTIST_31470_CACHE_IDENTITY Identity;
	};

	struct PENDING_EFFECT_SPAWN final
	{
		Client::EFFECT_SPAWN_DESC Desc;
		uint32_t iLevelIndex = UINT32_MAX;
		Client::EFFECT_SCENE_BUDGET_COST AdmissionCost;
	};

	std::vector<ACTIVE_EFFECT> g_ActiveEffects;
	std::vector<PENDING_EFFECT_SPAWN> g_PendingEffectSpawns;
	Client::CEffectProductPrewarmQueue g_ProductPrewarmQueue;
	std::map<std::string, Client::EFFECT_SCENE_BUDGET_COST, std::less<>>
		g_ProductEffectBudgetCosts;
	std::map<std::string, f32_t, std::less<>>
		g_ProductEffectPlaybackDurations;
	std::map<std::string,
		std::shared_ptr<const Client::CEffectScreenOverlayPresentation>,
		std::less<>> g_ProductScreenOverlayTemplates;
	uint64_t g_iSceneBudgetRejectedSpawnCount = 0u;
	uint64_t g_iNextWorldRootHandle = 1u;
	RECONSTRUCTED_SOURCE_RUNTIME_CACHE g_ReconstructedArtist31470;
	uint64_t g_iReconstructedArtist31470CacheGeneration = 0u;
	Client::EFFECT_ARTIST_31470_CACHE_IDENTITY
		g_LastArtist31470ToolPreviewConsumption;
	Client::EFFECT_ARTIST_31470_CACHE_IDENTITY
		g_LastArtist31470GameplayConsumption;
	uint64_t g_iArtist31470ToolPreviewConsumeCount = 0u;
	uint64_t g_iArtist31470GameplayConsumeCount = 0u;
	std::string g_strStatus = "Effect presentation service is idle.";

	bool_t SourceModuleClassMatches(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strBaseClass)
	{
		std::string_view Class = Module.strClassName;
		if (Class.starts_with("efparticlemodule"))
			Class.remove_prefix(2u);
		if (Class.ends_with("_seeded"))
			Class.remove_suffix(7u);
		return Class == strBaseClass;
	}

	bool_t Try_ResolveProductPlaybackDuration(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const std::shared_ptr<const
			Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>& pProjection,
		f32_t& fOutDurationSeconds,
		std::string& strOutStatus)
	{
		fOutDurationSeconds = 0.f;
		std::unordered_set<std::string> SourceVisualTargetElementIds;
		if (nullptr != pProjection)
		{
			if (!pProjection->Is_Valid() ||
				pProjection->Get_EffectAssetId() != Document.strEffectAssetId ||
				pProjection->Get_DocumentShared().get() != &Document)
			{
				strOutStatus =
					"Product Effect duration projection identity diverged.";
				return false;
			}
			if (pProjection->Get_ProjectionKind() == Client::
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
			{
				for (const Client::EFFECT_VISUAL_PROGRAM_ROW& Row :
					pProjection->Get_AdmittedRows())
				{
					if (!Row.TargetIdentity.has_value() ||
						Row.TargetIdentity->strTargetElementId.empty() ||
						!SourceVisualTargetElementIds.emplace(
							Row.TargetIdentity->strTargetElementId).second)
					{
						strOutStatus =
							"Product Effect duration source target closure is invalid.";
						return false;
					}
				}
				for (const Client::EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT&
					Supplemental :
					pProjection->Get_AdmittedSupplementalElements())
				{
					if (Supplemental.TargetIdentity.strTargetElementId.empty() ||
						!SourceVisualTargetElementIds.emplace(
							Supplemental.TargetIdentity.strTargetElementId).second)
					{
						strOutStatus =
							"Product Effect duration supplemental target closure is invalid.";
						return false;
					}
				}
			}
			else if (pProjection->Get_ProjectionKind() != Client::
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
			{
				strOutStatus =
					"Product Effect duration projection kind is unsupported.";
				return false;
			}
		}

		f32_t fDurationSeconds = 0.f;
		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (!Element.bVisible)
				continue;
			const bool_t bSourceVisualActive =
				SourceVisualTargetElementIds.contains(Element.strElementId);
			const bool_t bSourceMeshOrSpriteParticle = bSourceVisualActive &&
				Element.SourceRecipe.bEnabled &&
				((Element.eKind == Client::EFFECT_ELEMENT_KIND::MESH &&
				  Element.SourceRecipe.strRendererShape == "mesh") ||
				 (Element.eKind == Client::EFFECT_ELEMENT_KIND::SPRITE &&
				  Element.SourceRecipe.strRendererShape == "sprite"));
			const bool_t bSourceDecalParticle = bSourceVisualActive &&
				Element.eKind == Client::EFFECT_ELEMENT_KIND::DECAL &&
				Element.SourceRecipe.bEnabled &&
				Element.SourceRecipe.strRendererShape == "decal" &&
				std::any_of(Element.SourceRecipe.Modules.begin(),
					Element.SourceRecipe.Modules.end(),
					[](const Client::EFFECT_SOURCE_MODULE_DESC& Module)
					{
						return SourceModuleClassMatches(
							Module, "particlemoduletypedatadecal");
					});
			f32_t fElementTailSeconds = 0.f;
			if (Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE ||
				bSourceMeshOrSpriteParticle || bSourceDecalParticle)
			{
				fElementTailSeconds = Element.Detail.Particle.vLifeTimeSeconds.y;
			}
			else if (Element.eKind == Client::EFFECT_ELEMENT_KIND::TRAIL)
			{
				fElementTailSeconds =
					Element.Detail.Trail.fPointLifeTimeSeconds;
			}

			f32_t fElementDurationSeconds =
				Element.Detail.Timing.fLifeTimeSeconds;
			if (Element.SourceRecipe.bEnabled &&
				Element.SourceRecipe.fEmitterDurationSeconds > 0.f &&
				0u != Element.SourceRecipe.iEmitterLoopCount)
			{
				fElementDurationSeconds =
					Element.SourceRecipe.fEmitterDurationSeconds *
					static_cast<f32_t>(Element.SourceRecipe.iEmitterLoopCount);
			}
			const f32_t fElementTotalSeconds =
				Element.Detail.Timing.fStartDelaySeconds +
				(Element.SourceRecipe.bEnabled ?
					Element.SourceRecipe.fEmitterDelaySeconds : 0.f) +
				fElementDurationSeconds +
				Element.Detail.Timing.fAfterImageSeconds +
				fElementTailSeconds;
			if (!std::isfinite(fElementTotalSeconds) ||
				fElementTotalSeconds < 0.f)
			{
				strOutStatus =
					"Product Effect prepared playback duration is invalid.";
				return false;
			}
			fDurationSeconds = (std::max)(
				fDurationSeconds, fElementTotalSeconds);
		}
		for (const Client::EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
		{
			if (!Cue.bVisible)
				continue;
			const f32_t fCueTotalSeconds =
				Cue.fStartDelaySeconds + Cue.fDurationSeconds;
			if (!std::isfinite(fCueTotalSeconds) || fCueTotalSeconds < 0.f)
			{
				strOutStatus =
					"Product Effect prepared model-cue duration is invalid.";
				return false;
			}
			fDurationSeconds = (std::max)(
				fDurationSeconds, fCueTotalSeconds);
		}
		fOutDurationSeconds = fDurationSeconds;
		strOutStatus.clear();
		return true;
	}

	bool_t Prepare_ProductScreenOverlayTemplate(
		ComPtr<ID3D11Device> pDevice,
		const std::string& EffectAssetId,
		std::shared_ptr<const Client::CEffectScreenOverlayPresentation>&
			OutTemplate,
		std::string& strOutStatus)
	{
		using namespace Client;
		OutTemplate.reset();
		const std::shared_ptr<const EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING>
			Binding = CEffectCatalog::Find_ScreenOverlayProductBinding(
				EffectAssetId);
		if (nullptr == Binding)
		{
			strOutStatus.clear();
			return true;
		}
		if (nullptr == pDevice || Binding->strEffectAssetId != EffectAssetId ||
			Binding->strPresentationId.empty() ||
			Binding->strUtf8Json.size() != Binding->iByteCount ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				Binding->strUtf8Json) != Binding->strSha256 ||
			Binding->Resources.empty())
		{
			strOutStatus =
				"Product screen-overlay catalog identity is invalid.";
			return false;
		}
		for (const EFFECT_SCREEN_OVERLAY_RESOURCE_IDENTITY& Resource :
			Binding->Resources)
		{
			const std::filesystem::path Path = CRuntimeAssetRoot::Resolve(
				std::filesystem::path(Resource.strAssetId));
			std::ifstream Input(Path, std::ios::binary);
			if (Path.empty() || !Input)
			{
				strOutStatus =
					"Product screen-overlay resource is missing: " +
					Resource.strAssetId;
				return false;
			}
			const std::string Bytes{
				std::istreambuf_iterator<char_t>(Input),
				std::istreambuf_iterator<char_t>() };
			if (Bytes.size() != Resource.iByteCount ||
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Bytes) !=
					Resource.strSha256)
			{
				strOutStatus =
					"Product screen-overlay resource identity changed: " +
					Resource.strAssetId;
				return false;
			}
		}

		std::shared_ptr<CEffectScreenOverlayPresentation> Staged =
			CEffectScreenOverlayPresentation::Create(std::move(pDevice));
		if (nullptr == Staged ||
			!Staged->Stage_AndCommit(Binding->strUtf8Json, strOutStatus) ||
			Staged->Get_CommittedGeneration() != 1u ||
			Staged->Get_PresentationId() != Binding->strPresentationId ||
			0u == Staged->Get_PreparedOverlayCount() ||
			!std::isfinite(Staged->Get_MaximumEndSeconds()) ||
			Staged->Get_MaximumEndSeconds() <= 0.f)
		{
			if (strOutStatus.empty())
			{
				strOutStatus =
					"Product screen-overlay typed staging failed.";
			}
			return false;
		}
		OutTemplate = std::move(Staged);
		strOutStatus.clear();
		return true;
	}

	bool_t Apply_ProductScreenOverlayReceipt(
		const std::shared_ptr<const Client::CEffectScreenOverlayPresentation>&
			pTemplate,
		Client::EFFECT_SCENE_BUDGET_COST& InOutBudget,
		f32_t& fInOutDurationSeconds,
		std::string& strOutStatus)
	{
		if (nullptr == pTemplate)
			return true;
		const uint64_t iOverlayCount = static_cast<uint64_t>(
			pTemplate->Get_PreparedOverlayCount());
		if (0u == iOverlayCount || iOverlayCount > 64u ||
			iOverlayCount >
				(std::numeric_limits<uint64_t>::max)() -
				InOutBudget.iScreenOverlays ||
			iOverlayCount >
				(std::numeric_limits<uint64_t>::max)() -
				InOutBudget.iEstimatedDrawSubmissions ||
			!std::isfinite(pTemplate->Get_MaximumEndSeconds()) ||
			pTemplate->Get_MaximumEndSeconds() <= 0.f)
		{
			strOutStatus =
				"Product screen-overlay budget or duration receipt is invalid.";
			return false;
		}
		InOutBudget.iScreenOverlays += iOverlayCount;
		InOutBudget.iEstimatedDrawSubmissions += iOverlayCount;
		fInOutDurationSeconds = (std::max)(
			fInOutDurationSeconds, pTemplate->Get_MaximumEndSeconds());
		strOutStatus.clear();
		return true;
	}

	bool_t Queue_ProductCueTargets(
		const std::vector<Client::ANIMATION_EFFECT_CUE>& Cues,
		const bool_t bPriority,
		std::vector<std::string>* pOutCurrentTargets,
		std::string& strOutStatus)
	{
		const uint64_t iCatalogRevision =
			Client::CEffectCatalog::Get_RuntimeRevision();
		if (0u == iCatalogRevision)
		{
			strOutStatus =
				"Animation Effect cue registration has no runtime catalog revision.";
			return false;
		}

		std::vector<std::string> CurrentTargets;
		CurrentTargets.reserve(Cues.size());
		std::set<std::string, std::less<>> CurrentTargetIds;
		for (const Client::ANIMATION_EFFECT_CUE& Cue : Cues)
		{
			if (Cue.strEffectAssetId.empty() ||
				!Client::CEffectCatalog::Contains(Cue.strEffectAssetId))
			{
				strOutStatus =
					"Animation Effect cue target is not admitted by the catalog: " +
					Cue.strEffectAssetId;
				return false;
			}
			if (CurrentTargetIds.insert(Cue.strEffectAssetId).second)
				CurrentTargets.push_back(Cue.strEffectAssetId);
		}

		std::string QueueStatus;
		bool_t bCurrentTargetsQueued = false;
		if (g_ProductPrewarmQueue.Get_CatalogRevision() != iCatalogRevision)
		{
			const std::vector<std::string> PreviousTargets(
				g_ProductPrewarmQueue.Get_Targets().begin(),
				g_ProductPrewarmQueue.Get_Targets().end());
			g_ProductPrewarmQueue.Reset_ForCatalogRevision(iCatalogRevision);
			g_ProductEffectBudgetCosts.clear();
			g_ProductEffectPlaybackDurations.clear();
			g_ProductScreenOverlayTemplates.clear();

			if (bPriority)
			{
				std::vector<std::string> ValidPreviousTargets;
				ValidPreviousTargets.reserve(PreviousTargets.size());
				for (const std::string& EffectId : PreviousTargets)
				{
					if (Client::CEffectCatalog::Contains(EffectId))
						ValidPreviousTargets.push_back(EffectId);
				}
				if (!g_ProductPrewarmQueue.Enqueue(
						ValidPreviousTargets, QueueStatus))
				{
					strOutStatus = QueueStatus;
					return false;
				}
			}
			else
			{
				std::vector<std::string> RebasedTargets = CurrentTargets;
				RebasedTargets.reserve(
					CurrentTargets.size() + PreviousTargets.size());
				for (const std::string& EffectId : PreviousTargets)
				{
					if (Client::CEffectCatalog::Contains(EffectId))
						RebasedTargets.push_back(EffectId);
				}
				if (!g_ProductPrewarmQueue.Enqueue(
						RebasedTargets, QueueStatus))
				{
					strOutStatus = QueueStatus;
					return false;
				}
				bCurrentTargetsQueued = true;
			}
		}

		const bool_t bQueued = bPriority ?
			g_ProductPrewarmQueue.Enqueue_Priority(
				CurrentTargets, QueueStatus) :
			(bCurrentTargetsQueued ||
			 g_ProductPrewarmQueue.Enqueue(CurrentTargets, QueueStatus));
		if (!bQueued)
		{
			strOutStatus = QueueStatus;
			return false;
		}

		if (nullptr != pOutCurrentTargets)
			*pOutCurrentTargets = std::move(CurrentTargets);
		strOutStatus = QueueStatus;
		return true;
	}

	bool_t Add_BudgetCost(
		Client::EFFECT_SCENE_BUDGET_COST& InOut,
		const Client::EFFECT_SCENE_BUDGET_COST& Value)
	{
		const auto Add = [](uint64_t& Target, const uint64_t Increment)
		{
			if (Increment > (std::numeric_limits<uint64_t>::max)() - Target)
				return false;
			Target += Increment;
			return true;
		};
		return Add(InOut.iEffects, Value.iEffects) &&
			Add(InOut.iParticles, Value.iParticles) &&
			Add(InOut.iMeshParticles, Value.iMeshParticles) &&
			Add(InOut.iTrailPoints, Value.iTrailPoints) &&
			Add(InOut.iAfterImages, Value.iAfterImages) &&
			Add(InOut.iLights, Value.iLights) &&
			Add(InOut.iScreenPosts, Value.iScreenPosts) &&
			Add(InOut.iScreenOverlays, Value.iScreenOverlays) &&
			Add(InOut.iEstimatedDrawSubmissions,
				Value.iEstimatedDrawSubmissions);
	}

	bool_t BudgetWithin(
		const Client::EFFECT_SCENE_BUDGET_COST& Value,
		const Client::EFFECT_SCENE_BUDGET_COST& Limit)
	{
		return Value.iEffects <= Limit.iEffects &&
			Value.iParticles <= Limit.iParticles &&
			Value.iMeshParticles <= Limit.iMeshParticles &&
			Value.iTrailPoints <= Limit.iTrailPoints &&
			Value.iAfterImages <= Limit.iAfterImages &&
			Value.iLights <= Limit.iLights &&
			Value.iScreenPosts <= Limit.iScreenPosts &&
			Value.iScreenOverlays <= Limit.iScreenOverlays &&
			Value.iEstimatedDrawSubmissions <=
				Limit.iEstimatedDrawSubmissions;
	}

	const Client::EFFECT_SCENE_BUDGET_COST SCENE_HARD_BUDGET =
		{ 128u, 16384u, 4096u, 12288u, 2048u, 32u, 16u, 64u, 6144u };
	/* Remote cosmetics stop before the hard ceiling so a local action and boss
	   telegraph retain deterministic headroom during a four-player burst. */
	const Client::EFFECT_SCENE_BUDGET_COST REMOTE_SCENE_SOFT_BUDGET =
		{ 96u, 12288u, 2814u, 8192u, 1536u, 24u, 4u, 24u, 4096u };
	const Client::EFFECT_SCENE_BUDGET_COST OWNER_BUDGET =
		{ 32u, 8192u, 2048u, 4096u, 1024u, 16u, 16u, 32u, 3072u };

	Client::EFFECT_SCENE_BUDGET_COST Current_ActiveBudget()
	{
		Client::EFFECT_SCENE_BUDGET_COST Result;
		for (const ACTIVE_EFFECT& Effect : g_ActiveEffects)
			Add_BudgetCost(Result, Effect.AdmissionCost);
		return Result;
	}

	Client::EFFECT_SCENE_BUDGET_COST Current_PendingBudget()
	{
		Client::EFFECT_SCENE_BUDGET_COST Result;
		for (const PENDING_EFFECT_SPAWN& Pending : g_PendingEffectSpawns)
			Add_BudgetCost(Result, Pending.AdmissionCost);
		return Result;
	}

	bool_t Can_AdmitBudget(
		const EFFECT_OWNER_VIEW& Owner,
		const Client::EFFECT_SCENE_BUDGET_COST& Candidate,
		const bool_t bIncludePending,
		std::string& strOutStatus)
	{
		Client::EFFECT_SCENE_BUDGET_COST Scene = Current_ActiveBudget();
		if (bIncludePending && !Add_BudgetCost(Scene, Current_PendingBudget()))
		{
			strOutStatus = "Effect scene budget accumulation overflowed.";
			return false;
		}
		Client::EFFECT_SCENE_BUDGET_COST OwnerTotal;
		for (const ACTIVE_EFFECT& Effect : g_ActiveEffects)
		{
			if (Same_Owner(Resolve_Owner(Effect), Owner))
				Add_BudgetCost(OwnerTotal, Effect.AdmissionCost);
		}
		if (bIncludePending)
		{
			for (const PENDING_EFFECT_SPAWN& Pending : g_PendingEffectSpawns)
			{
				if (Same_Owner(Resolve_Owner(Pending.Desc), Owner))
					Add_BudgetCost(OwnerTotal, Pending.AdmissionCost);
			}
		}
		if (!Add_BudgetCost(Scene, Candidate) ||
			!Add_BudgetCost(OwnerTotal, Candidate))
		{
			strOutStatus = "Effect scene budget accumulation overflowed.";
			return false;
		}
		const bool_t bRemoteCharacter = nullptr != Owner.pCharacter &&
			!Owner.pCharacter->Is_LocallyControlled();
		const Client::EFFECT_SCENE_BUDGET_COST& SceneLimit =
			bRemoteCharacter ? REMOTE_SCENE_SOFT_BUDGET : SCENE_HARD_BUDGET;
		if (!BudgetWithin(Scene, SceneLimit) ||
			!BudgetWithin(OwnerTotal, OWNER_BUDGET))
		{
			strOutStatus = bRemoteCharacter ?
				"Remote cosmetic Effect suppressed to preserve local/boss frame budget." :
				"Effect spawn rejected by the scene/owner frame budget.";
			return false;
		}
		strOutStatus.clear();
		return true;
	}

	bool_t Is_FiniteMatrix(const float4x4_t& Value)
	{
		const f32_t* const pValues = &Value._11;
		return std::all_of(pValues, pValues + 16u,
			[](const f32_t fValue)
			{
				return std::isfinite(fValue);
			});
	}

	f32_t Basis_Length(
		const f32_t X,
		const f32_t Y,
		const f32_t Z)
	{
		return std::sqrt(X * X + Y * Y + Z * Z);
	}

	f32_t Basis_Dot(
		const f32_t AX,
		const f32_t AY,
		const f32_t AZ,
		const f32_t BX,
		const f32_t BY,
		const f32_t BZ)
	{
		return AX * BX + AY * BY + AZ * BZ;
	}

	f32_t Basis_Determinant(const float4x4_t& Value)
	{
		return
			Value._11 * (Value._22 * Value._33 - Value._23 * Value._32) -
			Value._12 * (Value._21 * Value._33 - Value._23 * Value._31) +
			Value._13 * (Value._21 * Value._32 - Value._22 * Value._31);
	}

	bool_t Is_NonDegenerateAffineMatrix(const float4x4_t& Value)
	{
		if (!Is_FiniteMatrix(Value) ||
			std::abs(Value._14) > SOURCE_BONE_AFFINE_TOLERANCE ||
			std::abs(Value._24) > SOURCE_BONE_AFFINE_TOLERANCE ||
			std::abs(Value._34) > SOURCE_BONE_AFFINE_TOLERANCE ||
			std::abs(Value._44 - 1.f) > SOURCE_BONE_AFFINE_TOLERANCE)
		{
			return false;
		}

		const f32_t fScaleX = Basis_Length(
			Value._11, Value._12, Value._13);
		const f32_t fScaleY = Basis_Length(
			Value._21, Value._22, Value._23);
		const f32_t fScaleZ = Basis_Length(
			Value._31, Value._32, Value._33);
		if (!std::isfinite(fScaleX) || !std::isfinite(fScaleY) ||
			!std::isfinite(fScaleZ) ||
			fScaleX <= SOURCE_BONE_MINIMUM_BASIS_LENGTH ||
			fScaleY <= SOURCE_BONE_MINIMUM_BASIS_LENGTH ||
			fScaleZ <= SOURCE_BONE_MINIMUM_BASIS_LENGTH)
		{
			return false;
		}

		const f32_t fScaleProduct = fScaleX * fScaleY * fScaleZ;
		const f32_t fNormalizedDeterminant =
			std::abs(Basis_Determinant(Value)) / fScaleProduct;
		return std::isfinite(fNormalizedDeterminant) &&
			fNormalizedDeterminant >=
				SOURCE_BONE_MINIMUM_NORMALIZED_DETERMINANT;
	}

	bool_t Has_ExpectedArtistSourceBoneCombinedScale(const float4x4_t& RawBone)
	{
		if (!Is_NonDegenerateAffineMatrix(RawBone))
			return false;

		const f32_t fScaleX = Basis_Length(
			RawBone._11, RawBone._12, RawBone._13);
		const f32_t fScaleY = Basis_Length(
			RawBone._21, RawBone._22, RawBone._23);
		const f32_t fScaleZ = Basis_Length(
			RawBone._31, RawBone._32, RawBone._33);
		const f32_t fMinimumScale = (std::min)({ fScaleX, fScaleY, fScaleZ });
		const f32_t fMaximumScale = (std::max)({ fScaleX, fScaleY, fScaleZ });
		if (std::abs(fScaleX - ARTIST_SOURCE_BONE_COMBINED_SCALE) >
				ARTIST_SOURCE_BONE_COMBINED_SCALE_TOLERANCE ||
			std::abs(fScaleY - ARTIST_SOURCE_BONE_COMBINED_SCALE) >
				ARTIST_SOURCE_BONE_COMBINED_SCALE_TOLERANCE ||
			std::abs(fScaleZ - ARTIST_SOURCE_BONE_COMBINED_SCALE) >
				ARTIST_SOURCE_BONE_COMBINED_SCALE_TOLERANCE ||
			fMaximumScale - fMinimumScale >
				ARTIST_SOURCE_BONE_UNIFORM_SCALE_TOLERANCE)
		{
			return false;
		}

		const f32_t fXY = std::abs(Basis_Dot(
			RawBone._11, RawBone._12, RawBone._13,
			RawBone._21, RawBone._22, RawBone._23) /
			(fScaleX * fScaleY));
		const f32_t fXZ = std::abs(Basis_Dot(
			RawBone._11, RawBone._12, RawBone._13,
			RawBone._31, RawBone._32, RawBone._33) /
			(fScaleX * fScaleZ));
		const f32_t fYZ = std::abs(Basis_Dot(
			RawBone._21, RawBone._22, RawBone._23,
			RawBone._31, RawBone._32, RawBone._33) /
			(fScaleY * fScaleZ));
		return fXY <= SOURCE_BONE_ORTHOGONAL_TOLERANCE &&
			fXZ <= SOURCE_BONE_ORTHOGONAL_TOLERANCE &&
			fYZ <= SOURCE_BONE_ORTHOGONAL_TOLERANCE;
	}

	bool_t Prepare_TargetSet(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext,
        const std::set<std::string, std::less<>>& Targets,
        std::string& strOutStatus)
    {
        if (Targets.empty())
        {
            strOutStatus = "No admitted animation Effect targets require prewarm.";
            return true;
        }
		const std::shared_ptr<const Client::CEffectMaterialProgramRegistry>
			MaterialProgramRegistry =
				Client::CEffectCatalog::Acquire_MaterialProgramRegistry();
		if (nullptr == MaterialProgramRegistry ||
			0u == MaterialProgramRegistry->Get_CatalogRevision() ||
			Client::CEffectCatalog::Get_RuntimeRevision() !=
				MaterialProgramRegistry->Get_CatalogRevision())
		{
			strOutStatus =
				"Animation Effect prewarm has no immutable material-program generation.";
			return false;
		}
		std::vector<Client::EFFECT_RENDER_PREWARM_TARGET> PrewarmTargets;
		PrewarmTargets.reserve(Targets.size());
		std::map<std::string, Client::EFFECT_SCENE_BUDGET_COST, std::less<>>
			StagedBudgetCosts;
		std::map<std::string, f32_t, std::less<>>
			StagedPlaybackDurations;
		std::map<std::string, std::shared_ptr<const
			Client::CEffectScreenOverlayPresentation>, std::less<>>
			StagedScreenOverlayTemplates;
		for (const std::string& EffectId : Targets)
		{
            const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> Document =
                Client::CEffectCatalog::Find(EffectId);
            if (nullptr == Document)
            {
                strOutStatus =
                    "Animation Effect target is absent from the runtime catalog: " +
                    EffectId;
                return false;
            }
			const std::shared_ptr<const
				Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> Projection =
				Client::CEffectCatalog::Find_VisualProjection(EffectId);
			if (nullptr != Projection &&
				Projection->Get_DocumentShared().get() != Document.get())
			{
				strOutStatus =
					"Animation Effect visual projection/catalog document identity diverged: " +
					EffectId;
				return false;
			}
			std::shared_ptr<const Client::CEffectScreenOverlayPresentation>
				ScreenOverlayTemplate;
			if (!Prepare_ProductScreenOverlayTemplate(
					pDevice, EffectId, ScreenOverlayTemplate, strOutStatus))
			{
				strOutStatus =
					"Animation Effect screen-overlay preparation failed for " +
					EffectId + ": " + strOutStatus;
				return false;
			}
			Client::EFFECT_SCENE_BUDGET_COST BudgetCost;
			if (!Client::CEffectPresentationService::Estimate_DocumentBudget(
					*Document, BudgetCost, strOutStatus))
			{
				strOutStatus = "Animation Effect budget admission failed for " +
					EffectId + ": " + strOutStatus;
				return false;
			}
			f32_t fPlaybackDurationSeconds = 0.f;
			if (!Try_ResolveProductPlaybackDuration(
					*Document, Projection, fPlaybackDurationSeconds,
					strOutStatus))
			{
				strOutStatus =
					"Animation Effect duration preparation failed for " +
					EffectId + ": " + strOutStatus;
				return false;
			}
			if (!Apply_ProductScreenOverlayReceipt(
					ScreenOverlayTemplate, BudgetCost,
					fPlaybackDurationSeconds, strOutStatus) ||
				!BudgetWithin(BudgetCost, SCENE_HARD_BUDGET))
			{
				if (strOutStatus.empty())
					strOutStatus = "screen-overlay hard budget exceeded";
				strOutStatus =
					"Animation Effect screen-overlay receipt failed for " +
					EffectId + ": " + strOutStatus;
				return false;
			}
			if (nullptr != ScreenOverlayTemplate)
			{
				StagedScreenOverlayTemplates.emplace(
					EffectId, std::move(ScreenOverlayTemplate));
			}
			StagedBudgetCosts.emplace(EffectId, BudgetCost);
			StagedPlaybackDurations.emplace(
				EffectId, fPlaybackDurationSeconds);
			PrewarmTargets.push_back({ EffectId, Document, Projection,
				MaterialProgramRegistry });
		}
		if (!Client::CEffectDocumentRenderer::Prepare_VisualProgramCatalog(
			std::move(pDevice), std::move(pContext),
			MaterialProgramRegistry->Get_CatalogRevision(), PrewarmTargets,
			strOutStatus))
		{
			return false;
		}
		g_ProductEffectBudgetCosts = std::move(StagedBudgetCosts);
		g_ProductEffectPlaybackDurations =
			std::move(StagedPlaybackDurations);
		g_ProductScreenOverlayTemplates =
			std::move(StagedScreenOverlayTemplates);
		return true;
	}

	std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
		Find_ProductPrepared(
			const std::string& strEffectAssetId,
			const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		const std::shared_ptr<const Client::CEffectMaterialProgramRegistry>
			MaterialProgramRegistry =
				Client::CEffectCatalog::Acquire_MaterialProgramRegistry();
		if (nullptr == MaterialProgramRegistry)
			return nullptr;
		const std::shared_ptr<const
			Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> Projection =
			Client::CEffectCatalog::Find_VisualProjection(strEffectAssetId);
		return Client::CEffectDocumentRenderer::Find_Prepared(
			MaterialProgramRegistry->Get_CatalogRevision(),
			strEffectAssetId, Document, Projection, MaterialProgramRegistry);
	}

	bool_t Resolve_ProductScreenOverlayTemplate(
		const std::string& EffectAssetId,
		std::shared_ptr<const Client::CEffectScreenOverlayPresentation>& Out,
		std::string& strOutStatus)
	{
		Out.reset();
		const std::shared_ptr<const Client::EFFECT_SCREEN_OVERLAY_PRODUCT_BINDING>
			Binding = Client::CEffectCatalog::Find_ScreenOverlayProductBinding(
				EffectAssetId);
		const auto Prepared =
			g_ProductScreenOverlayTemplates.find(EffectAssetId);
		if (nullptr == Binding)
		{
			if (Prepared != g_ProductScreenOverlayTemplates.end())
			{
				strOutStatus =
					"Product screen-overlay cache has no catalog owner.";
				return false;
			}
			strOutStatus.clear();
			return true;
		}
		if (Prepared == g_ProductScreenOverlayTemplates.end() ||
			nullptr == Prepared->second ||
			Prepared->second->Get_PresentationId() !=
				Binding->strPresentationId ||
			Prepared->second->Get_PreparedOverlayCount() == 0u)
		{
			strOutStatus =
				"Product screen-overlay target was not atomically prewarmed.";
			return false;
		}
		Out = Prepared->second;
		strOutStatus.clear();
		return true;
	}

    bool Resolve_Anchor(
		const EFFECT_OWNER_VIEW& Owner,
        const std::string& strAnchorSlotId,
        float4x4_t& Out)
    {
		if (!Owner.Is_Valid() || !Owner.Try_Get_PresentationRoot(Out))
            return false;
		if ("skill_target" == strAnchorSlotId)
			return nullptr != Owner.pCharacter &&
				Owner.pCharacter->Try_Get_SkillTargetRoot(Out);
        if ("root" == strAnchorSlotId)
            return true;
        const std::shared_ptr<Engine::CModel> pModel =
			Owner.Get_Model();
        if (nullptr == pModel || !pModel->Has_Bone(strAnchorSlotId.c_str()))
            return false;
        XMStoreFloat4x4(&Out,
            pModel->Get_BoneMatrix(strAnchorSlotId.c_str()) *
            XMLoadFloat4x4(&Out));
        return true;
    }

    std::vector<SOURCE_ANCHOR_REQUEST> Collect_SourceAnchorRequests(
        const Client::EFFECT_DOCUMENT_DESC& Document)
    {
        std::vector<SOURCE_ANCHOR_REQUEST> Requests;
        const auto AddRequest = [&Requests](SOURCE_ANCHOR_REQUEST Request)
        {
            if (Request.strRuntimeAnchorSlotId.empty() ||
                Request.strRuntimeBoneName.empty())
            {
                return;
            }
            const auto Existing = std::find_if(
                Requests.begin(), Requests.end(),
                [&Request](const SOURCE_ANCHOR_REQUEST& Value)
                {
                    return Value.strRuntimeAnchorSlotId ==
                        Request.strRuntimeAnchorSlotId;
                });
            if (Existing == Requests.end())
                Requests.push_back(std::move(Request));
        };
        for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
        {
            if (Element.ActionCueAttachment.bEnabled &&
                Element.ActionCueAttachment.bFollow)
            {
                AddRequest({
                    Element.ActionCueAttachment.strRuntimeAnchorSlotId,
                    Element.ActionCueAttachment.strRuntimeBoneName,
                    Element.ActionCueAttachment.SocketLocalTransform });
            }
            for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
                Element.SourceRecipe.Modules)
            {
                if (Module.strClassName != "particlemodulelocationbonesocket")
                    continue;
                for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal :
                    Module.Literals)
                {
                    if (Client::EFFECT_SOURCE_LITERAL_KIND::STRING !=
                        Literal.eKind ||
                        !Literal.strPropertyPath.starts_with("sourcelocations[") ||
                        !Literal.strPropertyPath.ends_with("].bonesocketname") ||
                        Literal.strString.empty())
                    {
                        continue;
                    }
                    AddRequest({ Literal.strString, Literal.strString, {} });
                }
            }
        }
        std::sort(Requests.begin(), Requests.end(),
            [](const SOURCE_ANCHOR_REQUEST& Left,
                const SOURCE_ANCHOR_REQUEST& Right)
            {
                return Left.strRuntimeAnchorSlotId <
                    Right.strRuntimeAnchorSlotId;
            });
        return Requests;
    }

	std::vector<SOURCE_ANCHOR_REQUEST> Collect_ReconstructedAnchorRequests(
		const std::shared_ptr<const
			Client::EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation)
	{
		std::vector<SOURCE_ANCHOR_REQUEST> Requests;
		if (nullptr == pPreparation ||
			5u != pPreparation->Get_AnchorRequests().size())
		{
			return Requests;
		}
		Requests.reserve(pPreparation->Get_AnchorRequests().size());
		for (const Client::EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Binding :
			pPreparation->Get_AnchorRequests())
		{
			const Client::EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST& Source =
				Binding.Request;
			if (!Source.bFollow || Source.strRuntimeAnchorSlotId.empty() ||
				Source.strRuntimeBoneName.empty())
			{
				Requests.clear();
				return Requests;
			}
			SOURCE_ANCHOR_REQUEST Request;
			Request.strRuntimeAnchorSlotId = Source.strRuntimeAnchorSlotId;
			Request.strRuntimeBoneName = Source.strRuntimeBoneName;
			Request.SocketLocalTransform.vPosition = {
				static_cast<f32_t>(Source.SocketLocalTransform.vPosition[0]),
				static_cast<f32_t>(Source.SocketLocalTransform.vPosition[1]),
				static_cast<f32_t>(Source.SocketLocalTransform.vPosition[2]) };
			Request.SocketLocalTransform.vRotationDegrees = {
				static_cast<f32_t>(Source.SocketLocalTransform.vRotationDegrees[0]),
				static_cast<f32_t>(Source.SocketLocalTransform.vRotationDegrees[1]),
				static_cast<f32_t>(Source.SocketLocalTransform.vRotationDegrees[2]) };
			Request.SocketLocalTransform.vScale = {
				static_cast<f32_t>(Source.SocketLocalTransform.vScale[0]),
				static_cast<f32_t>(Source.SocketLocalTransform.vScale[1]),
				static_cast<f32_t>(Source.SocketLocalTransform.vScale[2]) };
			Request.bNormalizeSourceImportScale = true;
			Requests.push_back(std::move(Request));
		}
		return Requests;
	}

    void Resolve_SourceAnchors(
		const EFFECT_OWNER_VIEW& Owner,
        const std::vector<SOURCE_ANCHOR_REQUEST>& Requests,
		std::unordered_map<std::string, float4x4_t>& InOutResult)
    {
		InOutResult.clear();
		InOutResult.reserve(Requests.size());
		float4x4_t PresentationRoot{};
		if (!Owner.Is_Valid() ||
			!Owner.Try_Get_PresentationRoot(PresentationRoot))
            return;
		const std::shared_ptr<Engine::CModel> pModel = Owner.Get_Model();
        if (nullptr == pModel)
            return;
		const matrix_t OwnerWorld = XMLoadFloat4x4(&PresentationRoot);
        for (const SOURCE_ANCHOR_REQUEST& Request : Requests)
        {
            if (!pModel->Has_Bone(Request.strRuntimeBoneName.c_str()))
                continue;
			matrix_t BoneAnchorWorld;
			if (Request.bNormalizeSourceImportScale)
			{
				Client::EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
				XMStoreFloat4x4(&AnchorBuild.RawBone,
					pModel->Get_BoneMatrix(
						Request.strRuntimeBoneName.c_str()));
				XMStoreFloat4x4(&AnchorBuild.OwnerWorld, OwnerWorld);
				float4x4_t NormalizedBoneAnchorWorld{};
				if (!Client::CEffectPresentationService::
					Build_SourceBoneAnchorWorld(
						AnchorBuild, NormalizedBoneAnchorWorld))
				{
					continue;
				}
				BoneAnchorWorld =
					XMLoadFloat4x4(&NormalizedBoneAnchorWorld);
			}
			else
			{
				BoneAnchorWorld =
					pModel->Get_BoneMatrix(
						Request.strRuntimeBoneName.c_str()) *
					OwnerWorld;
			}
            const Client::EFFECT_TRANSFORM_DESC& Local =
                Request.SocketLocalTransform;
            const matrix_t SocketLocal = XMMatrixScaling(
                Local.vScale.x, Local.vScale.y, Local.vScale.z) *
                XMMatrixRotationRollPitchYaw(
                    XMConvertToRadians(Local.vRotationDegrees.x),
                    XMConvertToRadians(Local.vRotationDegrees.y),
                    XMConvertToRadians(Local.vRotationDegrees.z)) *
                XMMatrixTranslation(
                    Local.vPosition.x,
                    Local.vPosition.y,
                    Local.vPosition.z);
            float4x4_t World{};
            XMStoreFloat4x4(&World,
                SocketLocal *
				BoneAnchorWorld);
			if (Request.bNormalizeSourceImportScale &&
				!Is_NonDegenerateAffineMatrix(World))
				continue;
            InOutResult.emplace(Request.strRuntimeAnchorSlotId, World);
        }
    }

	bool_t Is_FiniteTransform(const Client::EFFECT_TRANSFORM_DESC& Value)
	{
		return std::isfinite(Value.vPosition.x) &&
			std::isfinite(Value.vPosition.y) &&
			std::isfinite(Value.vPosition.z) &&
			std::isfinite(Value.vRotationDegrees.x) &&
			std::isfinite(Value.vRotationDegrees.y) &&
			std::isfinite(Value.vRotationDegrees.z) &&
			std::isfinite(Value.vScale.x) &&
			std::isfinite(Value.vScale.y) &&
			std::isfinite(Value.vScale.z);
	}

	bool_t Is_ValidOrientationDescriptor(
		const Client::EFFECT_SPAWN_DESC& Desc)
	{
		if (Desc.eOrientationPolicy >=
			Client::EFFECT_ORIENTATION_POLICY::END ||
			(Desc.bUseWorldRoot &&
				(Client::EFFECT_ORIENTATION_POLICY::ANCHOR !=
					Desc.eOrientationPolicy ||
				 Desc.bHasActionFacingYaw)) ||
			(Desc.bHasActionFacingYaw &&
			 !std::isfinite(Desc.fActionFacingYawDegrees)))
		{
			return false;
		}
		if (Client::EFFECT_ORIENTATION_POLICY::ANCHOR ==
			Desc.eOrientationPolicy)
		{
			return true;
		}
		return "root" == Desc.strAnchorSlotId &&
			Desc.bHasActionFacingYaw &&
			std::isfinite(Desc.fActionFacingYawDegrees) &&
			0u != Desc.iActionStartTick;
	}

	bool_t Has_EqualTransform(
		const Client::EFFECT_TRANSFORM_DESC& Left,
		const Client::EFFECT_TRANSFORM_DESC& Right)
	{
		return Left.vPosition.x == Right.vPosition.x &&
			Left.vPosition.y == Right.vPosition.y &&
			Left.vPosition.z == Right.vPosition.z &&
			Left.vRotationDegrees.x == Right.vRotationDegrees.x &&
			Left.vRotationDegrees.y == Right.vRotationDegrees.y &&
			Left.vRotationDegrees.z == Right.vRotationDegrees.z &&
			Left.vScale.x == Right.vScale.x &&
			Left.vScale.y == Right.vScale.y &&
			Left.vScale.z == Right.vScale.z;
	}

	bool_t Has_ExactArtist31470AnchorContract(
		const std::vector<SOURCE_ANCHOR_REQUEST>& Requests)
	{
		if (ARTIST_31470_ANCHOR_BINDING_COUNT != Requests.size())
			return false;
		const SOURCE_ANCHOR_REQUEST& First = Requests.front();
		if (First.strRuntimeAnchorSlotId !=
				ARTIST_31470_RUNTIME_ANCHOR_SLOT_ID ||
			First.strRuntimeBoneName != ARTIST_31470_RUNTIME_BONE_NAME ||
			!First.bNormalizeSourceImportScale ||
			!Is_FiniteTransform(First.SocketLocalTransform))
		{
			return false;
		}
		return std::all_of(Requests.begin(), Requests.end(),
			[&First](const SOURCE_ANCHOR_REQUEST& Request)
			{
				return Request.strRuntimeAnchorSlotId ==
						ARTIST_31470_RUNTIME_ANCHOR_SLOT_ID &&
					Request.strRuntimeBoneName ==
						ARTIST_31470_RUNTIME_BONE_NAME &&
					Request.bNormalizeSourceImportScale &&
					Is_FiniteTransform(Request.SocketLocalTransform) &&
					Has_EqualTransform(
						Request.SocketLocalTransform,
						First.SocketLocalTransform);
			});
	}

	bool_t Sample_Artist31470AnchorWorld(
		const std::shared_ptr<Engine::CModel>& pModel,
		const ARTIST_31470_TRANSFORM_HISTORY& History,
		const std::vector<SOURCE_ANCHOR_REQUEST>& Requests,
		const f32_t fSampleTimeSeconds,
		float4x4_t& OutAnchorWorld,
		std::string& strOutError)
	{
		OutAnchorWorld = {};
		const char_t* pCurrentClip = nullptr == pModel ? nullptr :
			pModel->Get_AnimationName(pModel->Get_CurrentAnimIndex());
		if (nullptr == pModel || !History.bEnabled ||
			!Has_ExactArtist31470AnchorContract(Requests) ||
			pModel->Get_CurrentAnimIndex() != History.iAnimationIndex ||
			nullptr == pCurrentClip ||
			History.strAnimationClipName != pCurrentClip ||
			History.strAnimationClipName != ARTIST_31470_ANIMATION_CLIP_NAME ||
			!std::isfinite(fSampleTimeSeconds) || fSampleTimeSeconds < 0.f ||
			!std::isfinite(History.fAnimationDurationSeconds) ||
			History.fAnimationDurationSeconds <= 0.f ||
			!Is_NonDegenerateAffineMatrix(History.ActionStartRootWorld))
		{
			strOutError =
				"Artist 31470 historical model/clip identity is invalid.";
			return false;
		}

		const f32_t fAnimationTimeSeconds = std::clamp(
			fSampleTimeSeconds, 0.f, History.fAnimationDurationSeconds);
		const f32_t fTrackPositionTicks = (std::min)(
			History.fAnimationDurationTicks,
			fAnimationTimeSeconds * History.fAnimationTickRate);
		std::array<float4x4_t, ARTIST_31470_ANCHOR_BINDING_COUNT>
			RawBoneMatrices{};
		if (!std::isfinite(fTrackPositionTicks) ||
			!pModel->Sample_CurrentAnimationBoneCombinedMatricesAtBlendElapsed(
				History.iAnimationIndex, fTrackPositionTicks,
				fAnimationTimeSeconds,
				History.BoneIndices, RawBoneMatrices))
		{
			strOutError =
				"Artist 31470 side-effect-free historical bone sample failed.";
			return false;
		}

		float4x4_t FirstAnchorWorld{};
		for (size_t iBinding = 0u; iBinding < Requests.size(); ++iBinding)
		{
			if (0u != iBinding && 0 != std::memcmp(
				&RawBoneMatrices.front(), &RawBoneMatrices[iBinding],
				sizeof(float4x4_t)))
			{
				strOutError =
					"Artist 31470 equal bone bindings sampled different poses.";
				return false;
			}
			Client::EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
			AnchorBuild.RawBone = RawBoneMatrices[iBinding];
			AnchorBuild.OwnerWorld = History.ActionStartRootWorld;
			float4x4_t BoneWorld{};
			if (!Client::CEffectPresentationService::Build_SourceBoneAnchorWorld(
					AnchorBuild, BoneWorld))
			{
				strOutError =
					"Artist 31470 historical b_wp_1 import scale is invalid.";
				return false;
			}

			const Client::EFFECT_TRANSFORM_DESC& Local =
				Requests[iBinding].SocketLocalTransform;
			const matrix_t SocketLocal = XMMatrixScaling(
				Local.vScale.x, Local.vScale.y, Local.vScale.z) *
				XMMatrixRotationRollPitchYaw(
					XMConvertToRadians(Local.vRotationDegrees.x),
					XMConvertToRadians(Local.vRotationDegrees.y),
					XMConvertToRadians(Local.vRotationDegrees.z)) *
				XMMatrixTranslation(
					Local.vPosition.x,
					Local.vPosition.y,
					Local.vPosition.z);
			float4x4_t AnchorWorld{};
			XMStoreFloat4x4(&AnchorWorld,
				SocketLocal * XMLoadFloat4x4(&BoneWorld));
			if (!Is_NonDegenerateAffineMatrix(AnchorWorld) ||
				(0u != iBinding && 0 != std::memcmp(
					&FirstAnchorWorld, &AnchorWorld, sizeof(float4x4_t))))
			{
				strOutError =
					"Artist 31470 equal anchor bindings produced different worlds.";
				return false;
			}
			if (0u == iBinding)
				FirstAnchorWorld = AnchorWorld;
		}
		OutAnchorWorld = FirstAnchorWorld;
		strOutError.clear();
		return true;
	}

	bool_t Prepare_Artist31470TransformHistory(
		const std::shared_ptr<Client::CCharacter>& pOwner,
		const std::vector<SOURCE_ANCHOR_REQUEST>& Requests,
		const float4x4_t& ActionStartRootWorld,
		const f32_t fInitialSampleTimeSeconds,
		ARTIST_31470_TRANSFORM_HISTORY& OutHistory,
		std::string& strOutError)
	{
		OutHistory = {};
		const std::shared_ptr<Engine::CModel> pModel = nullptr == pOwner ?
			nullptr : pOwner->Get_BodyModel();
		if (nullptr == pModel ||
			!Has_ExactArtist31470AnchorContract(Requests) ||
			!Is_NonDegenerateAffineMatrix(ActionStartRootWorld) ||
			!std::isfinite(fInitialSampleTimeSeconds) ||
			fInitialSampleTimeSeconds < 0.f)
		{
			strOutError =
				"Artist 31470 action-start transform history input is invalid.";
			return false;
		}

		ARTIST_31470_TRANSFORM_HISTORY Staged;
		Staged.bEnabled = true;
		Staged.pModel = pModel;
		Staged.iAnimationIndex = pModel->Get_CurrentAnimIndex();
		const char_t* pClipName =
			pModel->Get_AnimationName(Staged.iAnimationIndex);
		if (Staged.iAnimationIndex >= pModel->Get_NumAnimations() ||
			nullptr == pClipName ||
			std::string_view(pClipName) != ARTIST_31470_ANIMATION_CLIP_NAME)
		{
			strOutError =
				"Artist 31470 current animation is not sdm_sk_onestroke.";
			return false;
		}
		Staged.strAnimationClipName = pClipName;
		f32_t fCurrentTrackTicks = 0.f;
		if (!pModel->Get_AnimationProgress(
				Staged.iAnimationIndex, fCurrentTrackTicks,
				Staged.fAnimationDurationTicks))
		{
			strOutError =
				"Artist 31470 current animation progress is unavailable.";
			return false;
		}
		Staged.fAnimationTickRate =
			pModel->Get_AnimationTickPerSecond(Staged.iAnimationIndex);
		Staged.fAnimationDurationSeconds =
			Staged.fAnimationDurationTicks / Staged.fAnimationTickRate;
		const f32_t fExpectedTrackTicks = (std::min)(
			Staged.fAnimationDurationTicks,
			fInitialSampleTimeSeconds * Staged.fAnimationTickRate);
		if (!std::isfinite(fCurrentTrackTicks) || fCurrentTrackTicks < 0.f ||
			!std::isfinite(Staged.fAnimationTickRate) ||
			Staged.fAnimationTickRate <= 0.f ||
			!std::isfinite(Staged.fAnimationDurationTicks) ||
			Staged.fAnimationDurationTicks <= 0.f ||
			!std::isfinite(Staged.fAnimationDurationSeconds) ||
			Staged.fAnimationDurationSeconds <= 0.f ||
			std::abs(fCurrentTrackTicks - fExpectedTrackTicks) >
				ARTIST_31470_TRACK_TOLERANCE_TICKS)
		{
			strOutError =
				"Artist 31470 action age and current animation cursor disagree.";
			return false;
		}

		int32_t iExpectedBoneIndex = -1;
		for (size_t iBinding = 0u; iBinding < Requests.size(); ++iBinding)
		{
			const int32_t iBoneIndex = pModel->Find_BoneIndex(
				Requests[iBinding].strRuntimeBoneName.c_str());
			if (iBoneIndex < 0 ||
				(0u != iBinding && iBoneIndex != iExpectedBoneIndex))
			{
				strOutError =
					"Artist 31470 five binding rows do not resolve to one bone.";
				return false;
			}
			if (0u == iBinding)
				iExpectedBoneIndex = iBoneIndex;
			Staged.BoneIndices[iBinding] =
				static_cast<uint32_t>(iBoneIndex);
		}
		/* PlayerSkills 31470 has movementDistance=0.  The provider therefore
		   keeps this authoritative action-start root for every fixed step. */
		Staged.ActionStartRootWorld = ActionStartRootWorld;
		const f64_t fFixedStepCountEstimate = std::floor(
			static_cast<f64_t>(Staged.fAnimationDurationSeconds) /
			ARTIST_31470_FIXED_STEP_SECONDS_EXACT);
		if (!std::isfinite(fFixedStepCountEstimate) ||
			fFixedStepCountEstimate < 0.0 ||
			fFixedStepCountEstimate + 3.0 >
				static_cast<f64_t>(ARTIST_31470_MAX_HISTORY_SAMPLE_COUNT))
		{
			strOutError =
				"Artist 31470 fixed-step transform history exceeds its instance bound.";
			return false;
		}
		Staged.Samples.reserve(
			static_cast<size_t>(fFixedStepCountEstimate) + 3u);
		const auto AppendSample = [&pModel, &Staged, &Requests, &strOutError](
			const f32_t fSampleTimeSeconds)
		{
			if (!std::isfinite(fSampleTimeSeconds) ||
				fSampleTimeSeconds < 0.f ||
				fSampleTimeSeconds > Staged.fAnimationDurationSeconds)
			{
				strOutError =
					"Artist 31470 prepared history time is invalid.";
				return false;
			}
			const auto Existing = std::lower_bound(
				Staged.Samples.begin(), Staged.Samples.end(),
				fSampleTimeSeconds,
				[](const ARTIST_31470_ANCHOR_HISTORY_SAMPLE& Sample,
					const f32_t fTimeSeconds)
				{
					return Sample.fSampleTimeSeconds < fTimeSeconds;
				});
			if (Existing != Staged.Samples.end() &&
				Existing->fSampleTimeSeconds == fSampleTimeSeconds)
				return true;
			if (Staged.Samples.size() >=
				ARTIST_31470_MAX_HISTORY_SAMPLE_COUNT)
			{
				strOutError =
					"Artist 31470 prepared history exceeded its instance bound.";
				return false;
			}
			ARTIST_31470_ANCHOR_HISTORY_SAMPLE Sample;
			Sample.fSampleTimeSeconds = fSampleTimeSeconds;
			if (!Sample_Artist31470AnchorWorld(
					pModel, Staged, Requests, fSampleTimeSeconds,
					Sample.AnchorWorld, strOutError))
			{
				return false;
			}
			Staged.Samples.insert(Existing, std::move(Sample));
			return true;
		};
		if (!AppendSample(0.f))
			return false;
		const uint64_t iFixedStepCount =
			static_cast<uint64_t>(fFixedStepCountEstimate);
		for (uint64_t iStep = 1u; iStep <= iFixedStepCount; ++iStep)
		{
			const f32_t fStepTimeSeconds = static_cast<f32_t>(
				static_cast<f64_t>(iStep) *
				ARTIST_31470_FIXED_STEP_SECONDS_EXACT);
			if (fStepTimeSeconds > Staged.fAnimationDurationSeconds)
				break;
			if (!AppendSample(fStepTimeSeconds))
				return false;
		}
		/* The exact end row is the immutable Natural-tail hold.  The arbitrary
		   authoritative action age is also cached because Seek requests it after
		   replaying all preceding fixed steps. */
		if (!AppendSample(Staged.fAnimationDurationSeconds) ||
			!AppendSample((std::min)(
				fInitialSampleTimeSeconds,
				Staged.fAnimationDurationSeconds)))
		{
			return false;
		}
		OutHistory = std::move(Staged);
		strOutError.clear();
		return true;
	}

	bool_t Build_Artist31470TransformSample(
		ACTIVE_EFFECT& Effect,
		const f32_t fSampleTimeSeconds,
		Client::EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
		std::string& strOutError)
	{
		OutSample = {};
		const ARTIST_31470_TRANSFORM_HISTORY& History =
			Effect.Artist31470TransformHistory;
		const std::shared_ptr<Client::CCharacter> pOwner = Effect.pOwner.lock();
		const std::shared_ptr<Engine::CModel> pModel = History.pModel.lock();
		if (!History.bEnabled || nullptr == pOwner || nullptr == pModel ||
			pOwner->Get_BodyModel() != pModel ||
			Effect.strEffectAssetId != RECONSTRUCTED_ARTIST_31470_ASSET_ID ||
			!Has_ExactArtist31470AnchorContract(Effect.SourceAnchorRequests) ||
			!std::isfinite(fSampleTimeSeconds) || fSampleTimeSeconds < 0.f ||
			!Is_NonDegenerateAffineMatrix(History.ActionStartRootWorld))
		{
			strOutError =
				"Artist 31470 instance-local transform history identity is invalid.";
			return false;
		}

		const f32_t fCanonicalTimeSeconds = (std::min)(
			fSampleTimeSeconds, History.fAnimationDurationSeconds);
		const auto Found = std::lower_bound(
			History.Samples.begin(), History.Samples.end(),
			fCanonicalTimeSeconds,
			[](const ARTIST_31470_ANCHOR_HISTORY_SAMPLE& Sample,
				const f32_t fTimeSeconds)
			{
				return Sample.fSampleTimeSeconds < fTimeSeconds;
			});
		if (Found == History.Samples.end() ||
			Found->fSampleTimeSeconds != fCanonicalTimeSeconds)
		{
			strOutError =
				"Artist 31470 cache-only provider received an unprepared history time.";
			return false;
		}
		if (!Is_NonDegenerateAffineMatrix(Found->AnchorWorld))
		{
			strOutError =
				"Artist 31470 cached historical anchor became invalid.";
			return false;
		}

		Client::EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Staged;
		Staged.RootWorld = History.ActionStartRootWorld;
		const auto [Iterator, bInserted] =
			Staged.SourceAnchorWorlds.emplace(
				ARTIST_31470_RUNTIME_ANCHOR_SLOT_ID, Found->AnchorWorld);
		if (!bInserted || Iterator->first !=
				ARTIST_31470_RUNTIME_ANCHOR_SLOT_ID ||
			Staged.SourceAnchorWorlds.size() != 1u)
		{
			strOutError =
				"Artist 31470 historical anchor map did not preserve one equal slot.";
			return false;
		}
		OutSample = std::move(Staged);
		strOutError.clear();
		return true;
	}

	bool_t Validate_Artist31470CoreDocument(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		uint32_t& iOutVisibleElementCount,
		std::string& strOutStatus)
	{
		iOutVisibleElementCount = 0u;
		if (Document.strEffectAssetId != RECONSTRUCTED_ARTIST_31470_ASSET_ID ||
			Document.Elements.size() != RECONSTRUCTED_ARTIST_31470_ELEMENT_COUNT)
		{
			strOutStatus =
				"Artist Core F cache document identity/count is invalid.";
			return false;
		}

		constexpr size_t RENDERER_COUNT = static_cast<size_t>(
			Client::EFFECT_RENDERER_TYPE::END);
		std::array<uint32_t, RENDERER_COUNT> VisibleByRenderer{};
		std::array<uint32_t, RENDERER_COUNT> HiddenByRenderer{};
		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			const size_t iRenderer = static_cast<size_t>(Element.Renderer.eType);
			if (iRenderer >= RENDERER_COUNT)
			{
				strOutStatus =
					"Artist Core F cache document has an invalid renderer family.";
				return false;
			}
			if (Element.bVisible)
			{
				++VisibleByRenderer[iRenderer];
				++iOutVisibleElementCount;
			}
			else
			{
				++HiddenByRenderer[iRenderer];
			}
		}

		const auto VisibleCount = [&VisibleByRenderer](
			const Client::EFFECT_RENDERER_TYPE eRenderer)
		{
			return VisibleByRenderer[static_cast<size_t>(eRenderer)];
		};
		const auto HiddenCount = [&HiddenByRenderer](
			const Client::EFFECT_RENDERER_TYPE eRenderer)
		{
			return HiddenByRenderer[static_cast<size_t>(eRenderer)];
		};
		const bool_t bExactCoreScope =
			iOutVisibleElementCount ==
				RECONSTRUCTED_ARTIST_31470_VISIBLE_ELEMENT_COUNT &&
			VisibleCount(Client::EFFECT_RENDERER_TYPE::MESH_PARTICLE) == 13u &&
			VisibleCount(Client::EFFECT_RENDERER_TYPE::SPRITE_PARTICLE) == 16u &&
			VisibleCount(Client::EFFECT_RENDERER_TYPE::DECAL_PARTICLE) == 3u &&
			VisibleCount(Client::EFFECT_RENDERER_TYPE::CASCADE_RIBBON) == 1u &&
			HiddenCount(Client::EFFECT_RENDERER_TYPE::LIGHT_PARTICLE) == 1u &&
			HiddenCount(Client::EFFECT_RENDERER_TYPE::SCREEN_POST) == 1u;
		uint32_t iVisibleCoreCount = 0u;
		uint32_t iHiddenDeferredCount = 0u;
		if (bExactCoreScope)
		{
			iVisibleCoreCount =
				VisibleCount(Client::EFFECT_RENDERER_TYPE::MESH_PARTICLE) +
				VisibleCount(Client::EFFECT_RENDERER_TYPE::SPRITE_PARTICLE) +
				VisibleCount(Client::EFFECT_RENDERER_TYPE::DECAL_PARTICLE) +
				VisibleCount(Client::EFFECT_RENDERER_TYPE::CASCADE_RIBBON);
			iHiddenDeferredCount =
				HiddenCount(Client::EFFECT_RENDERER_TYPE::LIGHT_PARTICLE) +
				HiddenCount(Client::EFFECT_RENDERER_TYPE::SCREEN_POST);
		}
		const uint32_t iHiddenElementCount = static_cast<uint32_t>(
			Document.Elements.size()) - iOutVisibleElementCount;
		if (!bExactCoreScope ||
			iVisibleCoreCount != iOutVisibleElementCount ||
			iHiddenDeferredCount != iHiddenElementCount)
		{
			strOutStatus =
				"Artist Core F cache document is not the exact 33 core/2 deferred scope.";
			return false;
		}
		return true;
	}

	uint64_t Next_Artist31470CacheGeneration()
	{
		if (g_iReconstructedArtist31470CacheGeneration ==
			(std::numeric_limits<uint64_t>::max)())
		{
			return 0u;
		}
		return ++g_iReconstructedArtist31470CacheGeneration;
	}

	bool_t Create_Artist31470VisualAdapterProjection(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const
			Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>& OutProjection,
		std::string& strOutStatus)
	{
		OutProjection.reset();
		const std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_CORPUS> pCorpus =
			Client::CEffectCatalog::Find_VisualProgramCorpus();
		const std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM> pProgram =
			Client::CEffectCatalog::Find_VisualProgram(
				std::string(RECONSTRUCTED_ARTIST_31470_ASSET_ID));
		if (nullptr == pCorpus || nullptr == pProgram ||
			pProgram->eProjectionKind !=
				Client::EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
			Document.strEffectAssetId != RECONSTRUCTED_ARTIST_31470_ASSET_ID)
		{
			strOutStatus =
				"Artist Core F visual adapter Program identity is unavailable.";
			return false;
		}
		if (!Client::CEffectVisualProgramCorpusCodec::Create_DocumentProjection(
				*pCorpus, Document, OutProjection, strOutStatus) ||
			nullptr == OutProjection || !OutProjection->Is_Valid() ||
			OutProjection->Get_ProjectionKind() !=
				Client::EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
			OutProjection->Get_EffectAssetId() !=
				RECONSTRUCTED_ARTIST_31470_ASSET_ID ||
			OutProjection->Get_ProgramSha256() != pProgram->strProgramSha256 ||
			OutProjection->Get_AdmittedRows().empty())
		{
			OutProjection.reset();
			if (strOutStatus.empty())
			{
				strOutStatus =
					"Artist Core F visual adapter projection identity is invalid.";
			}
			return false;
		}
		return true;
	}

	Client::EFFECT_ARTIST_31470_CACHE_IDENTITY
		Build_Artist31470CacheIdentity(
			const RECONSTRUCTED_SOURCE_RUNTIME_CACHE& Cache)
	{
		Client::EFFECT_ARTIST_31470_CACHE_IDENTITY Identity;
		Identity.iGeneration = Cache.iGeneration;
		Identity.iPreparationIdentity = reinterpret_cast<std::uintptr_t>(
			Cache.pPreparation.get());
		Identity.iDocumentIdentity = reinterpret_cast<std::uintptr_t>(
			Cache.pDocument.get());
		Identity.iPreparedDocumentIdentity = reinterpret_cast<std::uintptr_t>(
			Cache.pPrepared.get());
		Identity.eVisualScope = Cache.eVisualScope;
		Identity.iDocumentElementCount = nullptr == Cache.pDocument ? 0u :
			static_cast<uint32_t>(Cache.pDocument->Elements.size());
		Identity.iVisibleElementCount = Cache.iVisibleElementCount;
		if (nullptr != Cache.pPreparation &&
			nullptr != Cache.pPreparation->Get_CatalogEntry())
		{
			Identity.iCatalogRevision = Cache.pPreparation->Get_CatalogEntry()->
				Get_Identity().iCatalogRevision;
		}
		return Identity;
	}

	bool_t Resolve_Artist31470Cache(
		const std::shared_ptr<const
			Client::EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			pExpectedPreparation,
		RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW& OutCache,
		std::string& strOutStatus)
	{
		OutCache = {};
		const std::shared_ptr<const Client::EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
			pCurrentEntry = Client::CEffectCatalog::Find_RuntimeProgramEntry(
				std::string(RECONSTRUCTED_ARTIST_31470_ASSET_ID));
		const auto& Cache = g_ReconstructedArtist31470;
		const std::shared_ptr<const Client::EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
			pCachedEntry = nullptr == Cache.pPreparation ? nullptr :
				Cache.pPreparation->Get_CatalogEntry();
		const std::shared_ptr<const Client::EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
			pProgram = nullptr == Cache.pPreparation ? nullptr :
				Cache.pPreparation->Get_Program();
		const std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM>
			pVisualProgram = Client::CEffectCatalog::Find_VisualProgram(
				std::string(RECONSTRUCTED_ARTIST_31470_ASSET_ID));
		if (nullptr == pCurrentEntry || nullptr == Cache.pPreparation ||
			nullptr == Cache.pDocument || nullptr == Cache.pVisualProjection ||
			nullptr == Cache.pPrepared || nullptr == pVisualProgram ||
			nullptr == pCachedEntry || nullptr == pProgram ||
			pCachedEntry.get() != pCurrentEntry.get() ||
			pCachedEntry->Get_Program().get() != pProgram.get() ||
			pCachedEntry->Get_Identity().strEffectAssetId !=
				RECONSTRUCTED_ARTIST_31470_ASSET_ID ||
			pCachedEntry->Get_Identity().iCatalogRevision == 0u ||
			!Cache.pVisualProjection->Is_Valid() ||
			Cache.pVisualProjection->Get_ProjectionKind() !=
				Client::EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
			Cache.pVisualProjection->Get_EffectAssetId() !=
				RECONSTRUCTED_ARTIST_31470_ASSET_ID ||
			Cache.pVisualProjection->Get_ProgramSha256() !=
				pVisualProgram->strProgramSha256 ||
			Cache.pVisualProjection->Get_DocumentShared().get() !=
				Cache.pDocument.get() ||
			Cache.eVisualScope != RECONSTRUCTED_ARTIST_31470_VISUAL_SCOPE ||
			Cache.iGeneration == 0u ||
			(nullptr != pExpectedPreparation &&
			 pExpectedPreparation.get() != Cache.pPreparation.get()) ||
			pProgram->Admission.bRuntimeExecution || pProgram->Admission.bProduct)
		{
			strOutStatus =
				"Artist Core F shared cache identity/provenance is invalid.";
			return false;
		}

		uint32_t iVisibleElementCount = 0u;
		if (!Validate_Artist31470CoreDocument(
			*Cache.pDocument, iVisibleElementCount, strOutStatus))
		{
			return false;
		}
		if (iVisibleElementCount != Cache.iVisibleElementCount)
		{
			strOutStatus =
				"Artist Core F shared cache scope provenance changed.";
			return false;
		}

		const Client::EFFECT_ARTIST_31470_CACHE_IDENTITY Identity =
			Build_Artist31470CacheIdentity(Cache);
		if (Identity.iPreparationIdentity == 0u ||
			Identity.iDocumentIdentity == 0u ||
			Identity.iPreparedDocumentIdentity == 0u ||
			Identity.iDocumentElementCount !=
				RECONSTRUCTED_ARTIST_31470_ELEMENT_COUNT ||
			Identity.iVisibleElementCount !=
				RECONSTRUCTED_ARTIST_31470_VISIBLE_ELEMENT_COUNT)
		{
			strOutStatus =
				"Artist Core F shared cache pointer/count identity is invalid.";
			return false;
		}

		OutCache.pPreparation = Cache.pPreparation;
		OutCache.pDocument = Cache.pDocument;
		OutCache.pVisualProjection = Cache.pVisualProjection;
		OutCache.pPrepared = Cache.pPrepared;
		OutCache.Identity = Identity;
		return true;
	}

	void Record_Artist31470ToolPreviewConsumption(
		const Client::EFFECT_ARTIST_31470_CACHE_IDENTITY& Identity)
	{
		g_LastArtist31470ToolPreviewConsumption = Identity;
		if (g_iArtist31470ToolPreviewConsumeCount !=
			(std::numeric_limits<uint64_t>::max)())
		{
			++g_iArtist31470ToolPreviewConsumeCount;
		}
	}

	void Record_Artist31470GameplayConsumption(
		const Client::EFFECT_ARTIST_31470_CACHE_IDENTITY& Identity)
	{
		g_LastArtist31470GameplayConsumption = Identity;
		if (g_iArtist31470GameplayConsumeCount !=
			(std::numeric_limits<uint64_t>::max)())
		{
			++g_iArtist31470GameplayConsumeCount;
		}
	}

	bool_t Attach_Artist31470CoreCache(
		const std::shared_ptr<Client::CEffectObject>& pObject,
		const std::shared_ptr<const
			Client::EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			pExpectedPreparation,
		Client::EFFECT_ARTIST_31470_CACHE_IDENTITY& OutIdentity,
		std::string& strOutStatus)
	{
		OutIdentity = {};
		if (nullptr == pObject)
		{
			strOutStatus =
				"Artist Core F cache attach object is invalid.";
			return false;
		}
		RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW Cache;
		if (!Resolve_Artist31470Cache(
			pExpectedPreparation, Cache, strOutStatus))
		{
			return false;
		}
		if (!Cache.Identity.Is_ExactCoreScope())
		{
			strOutStatus =
				"Artist Core F cache attach scope identity is invalid.";
			return false;
		}
		if (!pObject->Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
			Cache.pVisualProjection, Cache.pPrepared,
			Cache.pPreparation, strOutStatus))
		{
			return false;
		}
		if (pObject->Get_ReconstructedRuntimePreparation().get() !=
			Cache.pPreparation.get())
		{
			strOutStatus =
				"Artist Core F object/cache preparation identity diverged.";
			return false;
		}
		OutIdentity = Cache.Identity;
		return true;
	}

    void Remove_At(const size_t iIndex)
    {
        ACTIVE_EFFECT& Effect = g_ActiveEffects[iIndex];
        if (nullptr != Effect.pObject &&
            Effect.iLevelIndex == CGameInstance::Get().Get_CurrentLevelID())
        {
            CGameInstance::Get().Remove_GameObject_from_Layer(
                Effect.iLevelIndex, EFFECT_LAYER, Effect.pObject);
        }
        g_ActiveEffects.erase(g_ActiveEffects.begin() + iIndex);
    }
}

bool_t Client::CEffectPresentationService::Estimate_DocumentBudget(
	const EFFECT_DOCUMENT_DESC& Document,
	EFFECT_SCENE_BUDGET_COST& OutCost,
	std::string& strOutStatus)
{
	EFFECT_SCENE_BUDGET_COST Staged;
	Staged.iEffects = 1u;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Element.bVisible)
			continue;
		const EFFECT_DETAIL_DESC& Detail = Element.Detail;
		const bool_t bSourceParticle = Element.SourceRecipe.bEnabled &&
			(Element.SourceRecipe.strRendererShape == "mesh" ||
			 Element.SourceRecipe.strRendererShape == "sprite" ||
			 Element.SourceRecipe.strRendererShape == "decal");
		const bool_t bParticle =
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE || bSourceParticle;
		const bool_t bMeshParticle = bParticle &&
			(Element.Renderer.eType == EFFECT_RENDERER_TYPE::MESH_PARTICLE ||
			 Element.SourceRecipe.strRendererShape == "mesh");
		if (bParticle)
		{
			Staged.iParticles += Detail.Particle.iMaxParticles;
			if (bMeshParticle)
			{
				Staged.iMeshParticles += Detail.Particle.iMaxParticles;
				Staged.iEstimatedDrawSubmissions +=
					Detail.Particle.iMaxParticles;
			}
			else
			{
				++Staged.iEstimatedDrawSubmissions;
			}
		}
		else if (Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
			Element.eKind == EFFECT_ELEMENT_KIND::SPRITE ||
			Element.eKind == EFFECT_ELEMENT_KIND::DECAL)
		{
			++Staged.iEstimatedDrawSubmissions;
		}
		if (Element.eKind == EFFECT_ELEMENT_KIND::TRAIL)
		{
			Staged.iTrailPoints += Detail.Trail.iMaxPoints;
			++Staged.iEstimatedDrawSubmissions;
		}
		if (Detail.Timing.fAfterImageSeconds > 0.f &&
			Detail.AfterImage.iMaxCopies > 0u)
		{
			Staged.iAfterImages += Detail.AfterImage.iMaxCopies;
			Staged.iEstimatedDrawSubmissions +=
				Detail.AfterImage.iMaxCopies;
		}
		if (Element.eKind == EFFECT_ELEMENT_KIND::LIGHT &&
			Detail.Light.bEnabled)
		{
			++Staged.iLights;
			++Staged.iEstimatedDrawSubmissions;
		}
		if (Element.eKind == EFFECT_ELEMENT_KIND::SCREEN_POST &&
			Detail.ScreenPost.bEnabled)
		{
			++Staged.iScreenPosts;
			++Staged.iEstimatedDrawSubmissions;
		}
	}
	if (!BudgetWithin(Staged, SCENE_HARD_BUDGET))
	{
		strOutStatus =
			"One Effect document exceeds the scene hard budget.";
		return false;
	}
	OutCost = Staged;
	strOutStatus.clear();
	return true;
}

Client::EFFECT_SCENE_BUDGET_PROBE
Client::CEffectPresentationService::Get_SceneBudgetProbe()
{
	EFFECT_SCENE_BUDGET_PROBE Probe;
	Probe.Active = Current_ActiveBudget();
	Probe.Pending = Current_PendingBudget();
	Probe.iRejectedSpawnCount = g_iSceneBudgetRejectedSpawnCount;
	return Probe;
}

bool_t Client::CEffectPresentationService::Queue_ProductCues(
    const std::vector<ANIMATION_EFFECT_CUE>& Cues,
    std::string& strOutStatus)
{
	if (!Queue_ProductCueTargets(Cues, false, nullptr, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Queue_ProductCues_Priority(
	const std::vector<ANIMATION_EFFECT_CUE>& Cues,
	std::vector<std::string>& OutEffectAssetIds,
	std::string& strOutStatus)
{
	OutEffectAssetIds.clear();
	if (!Queue_ProductCueTargets(
			Cues, true, &OutEffectAssetIds, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Queue_ProductTargets_Priority(
	const std::vector<std::string>& EffectAssetIds,
	std::vector<std::string>& OutEffectAssetIds,
	std::string& strOutStatus)
{
	std::vector<ANIMATION_EFFECT_CUE> Targets;
	Targets.reserve(EffectAssetIds.size());
	for (const std::string& EffectAssetId : EffectAssetIds)
	{
		Targets.emplace_back();
		Targets.back().strEffectAssetId = EffectAssetId;
	}
	return Queue_ProductCues_Priority(
		Targets, OutEffectAssetIds, strOutStatus);
}

Client::EFFECT_PRODUCT_PREWARM_TARGET_PROBE
Client::CEffectPresentationService::Get_ProductCuePreparationProbe(
	const std::vector<std::string>& EffectAssetIds)
{
	return g_ProductPrewarmQueue.Get_TargetProbe(
		EffectAssetIds, CEffectCatalog::Get_RuntimeRevision());
}

bool_t Client::CEffectPresentationService::
Try_Get_PreparedProductDurationSeconds(
	const std::string& strEffectAssetId,
	f32_t& fOutDurationSeconds)
{
	fOutDurationSeconds = 0.f;
	const uint64_t iCatalogRevision = CEffectCatalog::Get_RuntimeRevision();
	if (strEffectAssetId.empty() || 0u == iCatalogRevision ||
		g_ProductPrewarmQueue.Get_CatalogRevision() != iCatalogRevision ||
		!g_ProductPrewarmQueue.Is_Prepared(strEffectAssetId))
	{
		return false;
	}
	const auto Duration =
		g_ProductEffectPlaybackDurations.find(strEffectAssetId);
	if (g_ProductEffectPlaybackDurations.end() == Duration ||
		!std::isfinite(Duration->second) || Duration->second < 0.f)
	{
		return false;
	}
	fOutDurationSeconds = Duration->second;
	return true;
}

void Client::CEffectPresentationService::Advance_ProductCuePreparation(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	const std::shared_ptr<const CEffectMaterialProgramRegistry>
		MaterialProgramRegistry =
			CEffectCatalog::Acquire_MaterialProgramRegistry();
	const uint64_t iCatalogRevision = nullptr == MaterialProgramRegistry ? 0u :
		MaterialProgramRegistry->Get_CatalogRevision();
	if (nullptr == pDevice || nullptr == pContext || 0u == iCatalogRevision)
	{
		g_strStatus =
			"Product Effect incremental prewarm has invalid frame arguments.";
		return;
	}
	if (g_ProductPrewarmQueue.Get_CatalogRevision() != iCatalogRevision)
	{
		const std::vector<std::string> PreviousTargets(
			g_ProductPrewarmQueue.Get_Targets().begin(),
			g_ProductPrewarmQueue.Get_Targets().end());
		std::vector<std::string> CurrentTargets;
		CurrentTargets.reserve(PreviousTargets.size());
		for (const std::string& EffectId : PreviousTargets)
		{
			if (CEffectCatalog::Contains(EffectId))
				CurrentTargets.push_back(EffectId);
		}
		g_ProductPrewarmQueue.Reset_ForCatalogRevision(iCatalogRevision);
		g_ProductEffectBudgetCosts.clear();
		g_ProductEffectPlaybackDurations.clear();
		g_ProductScreenOverlayTemplates.clear();
		std::string QueueStatus;
		if (!g_ProductPrewarmQueue.Enqueue(CurrentTargets, QueueStatus))
		{
			g_strStatus = QueueStatus;
			return;
		}
	}

	std::string EffectId;
	const EFFECT_PRODUCT_PREWARM_STEP_RESULT Step =
		g_ProductPrewarmQueue.Begin_Frame(EffectId);
	if (EFFECT_PRODUCT_PREWARM_STEP_RESULT::IDLE == Step ||
		EFFECT_PRODUCT_PREWARM_STEP_RESULT::YIELDED == Step)
	{
		return;
	}
	if (EFFECT_PRODUCT_PREWARM_STEP_RESULT::READY != Step || EffectId.empty())
	{
		g_strStatus = "Product Effect incremental prewarm queue is invalid.";
		return;
	}

	std::string PrepareStatus;
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC> Document =
		CEffectCatalog::Find(EffectId);
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		Projection = nullptr == Document ? nullptr :
			CEffectCatalog::Find_VisualProjection(EffectId);
	EFFECT_SCENE_BUDGET_COST BudgetCost;
	f32_t fPlaybackDurationSeconds = 0.f;
	std::shared_ptr<const CEffectScreenOverlayPresentation>
		ScreenOverlayTemplate;
	bool_t bPrepared = nullptr != Document;
	if (nullptr == Document)
	{
		PrepareStatus =
			"Product Effect incremental document load failed for " + EffectId +
			": " + CEffectCatalog::Get_Status();
	}
	else if (nullptr != Projection &&
		Projection->Get_DocumentShared().get() != Document.get())
	{
		PrepareStatus =
			"Product Effect incremental document/projection identity diverged: " +
			EffectId;
		bPrepared = false;
	}
	if (bPrepared && !Prepare_ProductScreenOverlayTemplate(
			pDevice, EffectId, ScreenOverlayTemplate, PrepareStatus))
	{
		PrepareStatus =
			"Product Effect incremental screen-overlay preparation failed for " +
			EffectId + ": " + PrepareStatus;
		bPrepared = false;
	}
	if (bPrepared && !Estimate_DocumentBudget(
			*Document, BudgetCost, PrepareStatus))
	{
		PrepareStatus = "Product Effect incremental budget admission failed for " +
			EffectId + ": " + PrepareStatus;
		bPrepared = false;
	}
	if (bPrepared && !Try_ResolveProductPlaybackDuration(
			*Document, Projection, fPlaybackDurationSeconds, PrepareStatus))
	{
		PrepareStatus =
			"Product Effect incremental duration preparation failed for " +
			EffectId + ": " + PrepareStatus;
		bPrepared = false;
	}
	if (bPrepared &&
		(!Apply_ProductScreenOverlayReceipt(
			ScreenOverlayTemplate, BudgetCost,
			fPlaybackDurationSeconds, PrepareStatus) ||
		 !BudgetWithin(BudgetCost, SCENE_HARD_BUDGET)))
	{
		if (PrepareStatus.empty())
			PrepareStatus = "screen-overlay hard budget exceeded";
		PrepareStatus =
			"Product Effect incremental screen-overlay receipt failed for " +
			EffectId + ": " + PrepareStatus;
		bPrepared = false;
	}
	if (bPrepared)
	{
		bPrepared = CEffectDocumentRenderer::Prepare_VisualProgramTarget(
			std::move(pDevice), std::move(pContext), iCatalogRevision,
			{ EffectId, Document, Projection, MaterialProgramRegistry },
			PrepareStatus);
	}

	std::string CompletionStatus;
	if (!g_ProductPrewarmQueue.Complete_Front(
			EffectId, bPrepared, CompletionStatus))
	{
		g_strStatus = CompletionStatus;
		OutputDebugStringA(("[Client][EffectPresentation] " +
			g_strStatus + "\n").c_str());
		return;
	}
	if (bPrepared)
	{
		g_ProductEffectBudgetCosts[EffectId] = BudgetCost;
		g_ProductEffectPlaybackDurations[EffectId] =
			fPlaybackDurationSeconds;
		if (nullptr != ScreenOverlayTemplate)
		{
			g_ProductScreenOverlayTemplates[EffectId] =
				std::move(ScreenOverlayTemplate);
		}
		else
		{
			g_ProductScreenOverlayTemplates.erase(EffectId);
		}
		g_strStatus = PrepareStatus;
		return;
	}
	g_strStatus =
		"Product Effect incremental prewarm failed closed for " + EffectId +
		": " + PrepareStatus;
	OutputDebugStringA(("[Client][EffectPresentation] " +
			g_strStatus + "\n").c_str());
}

bool_t Client::CEffectPresentationService::Replace_ProductPreparedTarget(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint64_t iCatalogRevision,
	const std::string& strEffectAssetId,
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProgramProjection,
	std::string& strOutStatus)
{
	const std::shared_ptr<const CEffectMaterialProgramRegistry>
		MaterialProgramRegistry =
			CEffectCatalog::Acquire_MaterialProgramRegistry();
	if (nullptr == pDevice || nullptr == pContext ||
		0u == iCatalogRevision || strEffectAssetId.empty() ||
		nullptr == pDocument ||
		nullptr == MaterialProgramRegistry ||
		MaterialProgramRegistry->Get_CatalogRevision() != iCatalogRevision ||
		pDocument->strEffectAssetId != strEffectAssetId ||
		CEffectCatalog::Get_RuntimeRevision() != iCatalogRevision ||
		g_ProductPrewarmQueue.Get_CatalogRevision() != iCatalogRevision ||
		(nullptr != pVisualProgramProjection &&
			(!pVisualProgramProjection->Is_Valid() ||
			 pVisualProgramProjection->Get_EffectAssetId() != strEffectAssetId ||
			 pVisualProgramProjection->Get_DocumentShared().get() !=
				pDocument.get())))
	{
		strOutStatus =
			"Selected Effect prepared replacement arguments are invalid.";
		g_strStatus = strOutStatus;
		return false;
	}

	std::shared_ptr<const CEffectScreenOverlayPresentation>
		CandidateScreenOverlayTemplate;
	if (!Prepare_ProductScreenOverlayTemplate(
			pDevice, strEffectAssetId, CandidateScreenOverlayTemplate,
			strOutStatus))
	{
		strOutStatus =
			"Selected Effect screen-overlay preparation failed for " +
			strEffectAssetId + ": " + strOutStatus;
		g_strStatus = strOutStatus;
		return false;
	}
	EFFECT_SCENE_BUDGET_COST CandidateBudget;
	if (!Estimate_DocumentBudget(*pDocument, CandidateBudget, strOutStatus))
	{
		strOutStatus = "Selected Effect budget admission failed for " +
			strEffectAssetId + ": " + strOutStatus;
		g_strStatus = strOutStatus;
		return false;
	}
	f32_t fCandidatePlaybackDurationSeconds = 0.f;
	if (!Try_ResolveProductPlaybackDuration(
			*pDocument, pVisualProgramProjection,
			fCandidatePlaybackDurationSeconds, strOutStatus))
	{
		strOutStatus = "Selected Effect duration preparation failed for " +
			strEffectAssetId + ": " + strOutStatus;
		g_strStatus = strOutStatus;
		return false;
	}
	if (!Apply_ProductScreenOverlayReceipt(
			CandidateScreenOverlayTemplate, CandidateBudget,
			fCandidatePlaybackDurationSeconds, strOutStatus) ||
		!BudgetWithin(CandidateBudget, SCENE_HARD_BUDGET))
	{
		if (strOutStatus.empty())
			strOutStatus = "screen-overlay hard budget exceeded";
		strOutStatus = "Selected Effect screen-overlay receipt failed for " +
			strEffectAssetId + ": " + strOutStatus;
		g_strStatus = strOutStatus;
		return false;
	}
	CEffectProductPrewarmQueue StagedPrewarmQueue = g_ProductPrewarmQueue;
	std::string QueueStatus;
	if (!StagedPrewarmQueue.Commit_HotReloadPrepared(
			strEffectAssetId, QueueStatus))
	{
		strOutStatus = QueueStatus;
		g_strStatus = strOutStatus;
		return false;
	}
	auto StagedBudgetCosts = g_ProductEffectBudgetCosts;
	StagedBudgetCosts.insert_or_assign(strEffectAssetId, CandidateBudget);
	auto StagedPlaybackDurations = g_ProductEffectPlaybackDurations;
	StagedPlaybackDurations.insert_or_assign(
		strEffectAssetId, fCandidatePlaybackDurationSeconds);
	auto StagedScreenOverlayTemplates = g_ProductScreenOverlayTemplates;
	if (nullptr != CandidateScreenOverlayTemplate)
	{
		StagedScreenOverlayTemplates.insert_or_assign(
			strEffectAssetId, std::move(CandidateScreenOverlayTemplate));
	}
	else
	{
		StagedScreenOverlayTemplates.erase(strEffectAssetId);
	}
	if (!CEffectDocumentRenderer::Replace_VisualProgramTarget(
			std::move(pDevice), std::move(pContext), iCatalogRevision,
			{ strEffectAssetId, std::move(pDocument),
				std::move(pVisualProgramProjection), MaterialProgramRegistry },
			strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}

	g_ProductPrewarmQueue = std::move(StagedPrewarmQueue);
	g_ProductEffectBudgetCosts = std::move(StagedBudgetCosts);
	g_ProductEffectPlaybackDurations = std::move(StagedPlaybackDurations);
	g_ProductScreenOverlayTemplates =
		std::move(StagedScreenOverlayTemplates);
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Reload_SelectedProductEffect(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const std::string& strEffectAssetId,
	const std::filesystem::path& AuthoredPath,
	std::string& strOutStatus)
{
	std::shared_ptr<const EFFECT_DEBUG_DIRECT_AUTHORED_REPLACEMENT> Candidate;
	if (!CEffectCatalog::Stage_DebugDirectAuthoredReplacement(
			strEffectAssetId, AuthoredPath, Candidate, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	if (nullptr == Candidate ||
		Candidate->Get_EffectAssetId() != strEffectAssetId ||
		Candidate->Get_RuntimeRevision() == 0u ||
		nullptr == Candidate->Get_DocumentShared() ||
		Candidate->Get_DocumentShared()->strEffectAssetId != strEffectAssetId)
	{
		strOutStatus =
			"Selected Effect catalog replacement candidate is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	if (!CEffectCatalog::Commit_DebugDirectAuthoredReplacement(
			Candidate, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}

	std::string ReplaceStatus;
	if (Replace_ProductPreparedTarget(
			std::move(pDevice), std::move(pContext),
			Candidate->Get_RuntimeRevision(), Candidate->Get_EffectAssetId(),
			Candidate->Get_DocumentShared(), Candidate->Get_VisualProjection(),
			ReplaceStatus))
	{
		strOutStatus = "Hot reloaded selected Effect for subsequent spawns: " +
			strEffectAssetId + ". " + ReplaceStatus;
		g_strStatus = strOutStatus;
		return true;
	}

	std::string RestoreStatus;
	const bool_t bRestored =
		CEffectCatalog::Restore_DebugDirectAuthoredReplacement(
			Candidate, RestoreStatus);
	strOutStatus = "Selected Effect GPU replacement failed: " + ReplaceStatus;
	if (!bRestored)
	{
		strOutStatus += " Catalog rollback also failed: " + RestoreStatus;
	}
	else
	{
		strOutStatus += " Previous catalog document and prepared target were preserved.";
	}
	g_strStatus = strOutStatus;
	return false;
}

bool_t Client::CEffectPresentationService::Build_SourceBoneAnchorWorld(
	const EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC& Desc,
	float4x4_t& OutWorld)
{
	OutWorld = {};
	if (!Has_ExpectedArtistSourceBoneCombinedScale(Desc.RawBone) ||
		!Is_NonDegenerateAffineMatrix(Desc.OwnerWorld))
	{
		return false;
	}

	float4x4_t BoneWithoutImportScale = Desc.RawBone;
	constexpr f32_t COMBINED_SCALE_RECIPROCAL =
		1.f / ARTIST_SOURCE_BONE_COMBINED_SCALE;
	BoneWithoutImportScale._11 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._12 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._13 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._21 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._22 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._23 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._31 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._32 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._33 *= COMBINED_SCALE_RECIPROCAL;
	XMStoreFloat4x4(&OutWorld,
		XMLoadFloat4x4(&BoneWithoutImportScale) *
		XMLoadFloat4x4(&Desc.OwnerWorld));
	if (!Is_NonDegenerateAffineMatrix(OutWorld))
	{
		OutWorld = {};
		return false;
	}
	return true;
}

bool_t Client::CEffectPresentationService::Prepare_ReconstructedRuntimeProgram(
	const std::string& strEffectAssetId,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
		OutPreparation,
	std::string& strOutStatus)
{
	if (!CEffectReconstructedRuntimeBoundary::Prepare_Presentation(
		strEffectAssetId, OutPreparation, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	strOutStatus = "Prepared reconstructed Effect program for inspection.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Prepare_ReconstructedArtist31470(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	std::string& strOutStatus)
{
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pPreparation;
	if (!CEffectReconstructedRuntimeBoundary::Prepare_Presentation(
		std::string(RECONSTRUCTED_ARTIST_31470_ASSET_ID),
		pPreparation, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		pPreparation->Get_CatalogEntry();
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		pPreparation->Get_Program();
	if (nullptr == pEntry || nullptr == pProgram ||
		pEntry->Get_Program().get() != pProgram.get() ||
		pEntry->Get_Identity().strEffectAssetId !=
			RECONSTRUCTED_ARTIST_31470_ASSET_ID ||
		pProgram->Admission.bRuntimeExecution || pProgram->Admission.bProduct)
	{
		strOutStatus =
			"Artist 31470 reconstructed preparation identity is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW Cached;
	std::string strCacheStatus;
	if (Resolve_Artist31470Cache(nullptr, Cached, strCacheStatus) &&
		Cached.pPreparation->Get_CatalogEntry().get() == pEntry.get())
	{
		strOutStatus =
			"Artist Core F (33) shared document/prepared cache is already prepared.";
		g_strStatus = strOutStatus;
		return true;
	}

	EFFECT_DOCUMENT_DESC Document;
	if (!CEffectReconstructedSourceRuntimeFactory::Build_Document(
		pPreparation, Document, strOutStatus,
		RECONSTRUCTED_ARTIST_31470_VISUAL_SCOPE))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	if (const std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT> pTuning =
			pEntry->Get_OccurrenceTuning();
		nullptr != pTuning &&
		!CEffectOccurrenceTuningCodec::Apply_ToProjectedDocument(
			Document, *pProgram, *pTuning, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	uint32_t iVisibleElementCount = 0u;
	if (!Validate_Artist31470CoreDocument(
		Document, iVisibleElementCount, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProjection;
	if (!Create_Artist31470VisualAdapterProjection(
			Document, pVisualProjection, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument =
		pVisualProjection->Get_DocumentShared();
	std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT> pPrepared;
	if (!CEffectDocumentRenderer::
		Prepare_ReconstructedSourceRuntimeWithVisualProgramAdapter(
			std::move(pDevice), std::move(pContext), pPreparation,
			pVisualProjection, pPrepared, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	if (nullptr == pPrepared)
	{
		strOutStatus =
			"Artist 31470 reconstructed resource preparation returned no result.";
		g_strStatus = strOutStatus;
		return false;
	}
	RECONSTRUCTED_SOURCE_RUNTIME_CACHE Staged;
	Staged.pPreparation = std::move(pPreparation);
	Staged.pDocument = pDocument;
	Staged.pVisualProjection = std::move(pVisualProjection);
	Staged.pPrepared = std::move(pPrepared);
	Staged.eVisualScope = RECONSTRUCTED_ARTIST_31470_VISUAL_SCOPE;
	Staged.iGeneration = Next_Artist31470CacheGeneration();
	Staged.iVisibleElementCount = iVisibleElementCount;
	if (0u == Staged.iGeneration)
	{
		strOutStatus = "Artist Core F shared cache generation is exhausted.";
		g_strStatus = strOutStatus;
		return false;
	}
	g_ReconstructedArtist31470 = std::move(Staged);
	g_LastArtist31470ToolPreviewConsumption = {};
	g_LastArtist31470GameplayConsumption = {};
	g_iArtist31470ToolPreviewConsumeCount = 0u;
	g_iArtist31470GameplayConsumeCount = 0u;
	strOutStatus =
		"Artist Core F (33) shared cache prepared: MeshParticle 13, SpriteParticle 16, DecalParticle 3, CascadeRibbon 1; PointLight/ScreenPost stay deferred and Product remains false.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Acquire_ReconstructedArtist31470(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
		OutPreparation,
	std::string& strOutStatus)
{
	OutPreparation.reset();
	if (!Prepare_ReconstructedArtist31470(
		std::move(pDevice), std::move(pContext), strOutStatus))
	{
		return false;
	}
	RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW Cache;
	if (!Resolve_Artist31470Cache(nullptr, Cache, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	OutPreparation = Cache.pPreparation;
	strOutStatus =
		"Artist Core F (33) shared cache acquired; Product remains false.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::
	Stage_ReconstructedArtist31470Preview(
		const std::shared_ptr<CEffectObject>& pObject,
		const std::shared_ptr<const
			EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pExpectedPreparation,
		std::string& strOutStatus)
{
	if (nullptr == pObject || nullptr == pExpectedPreparation)
	{
		strOutStatus =
			"Artist Core F preview object/expected cache identity is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	EFFECT_ARTIST_31470_CACHE_IDENTITY ConsumedIdentity;
	if (!Attach_Artist31470CoreCache(
		pObject, pExpectedPreparation,
		ConsumedIdentity, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	Record_Artist31470ToolPreviewConsumption(ConsumedIdentity);
	strOutStatus =
		"Artist Core F (33) Tool preview consumed the shared document/prepared cache.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Get_ReconstructedOccurrenceInfo(
	const std::string& strEffectAssetId,
	const std::string& strOccurrenceId,
	EFFECT_RECONSTRUCTED_OCCURRENCE_INFO& OutInfo,
	std::string& strOutStatus)
{
	OutInfo = {};
	RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW Cache;
	if (!Resolve_Artist31470Cache(nullptr, Cache, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		Cache.pPreparation->Get_Program();
	if (nullptr == pProgram ||
		pProgram->strRuntimeCatalogAssetId != strEffectAssetId)
	{
		strOutStatus =
			"Reconstructed occurrence does not belong to the prepared Effect.";
		g_strStatus = strOutStatus;
		return false;
	}
	const auto Emitter = std::find_if(
		pProgram->Emitters.begin(), pProgram->Emitters.end(),
		[&strOccurrenceId](const EFFECT_RUNTIME_PROGRAM_EMITTER& Candidate)
		{
			return Candidate.Row.strId == strOccurrenceId;
		});
	if (Emitter == pProgram->Emitters.end())
	{
		strOutStatus = "Reconstructed occurrence ID is not in the prepared Program.";
		g_strStatus = strOutStatus;
		return false;
	}
	const auto Element = std::find_if(
		Cache.pDocument->Elements.begin(), Cache.pDocument->Elements.end(),
		[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
		{
			return Candidate.strElementId == Emitter->strSourceElementId;
		});
	if (Element == Cache.pDocument->Elements.end())
	{
		strOutStatus = "Reconstructed occurrence projected Element is missing.";
		g_strStatus = strOutStatus;
		return false;
	}
	const auto Assign = [](const std::array<double, 3u>& Source,
		float3_t& Target)
	{
		if (std::any_of(Source.begin(), Source.end(), [](const double Value)
			{
				return !std::isfinite(Value) ||
					Value < -static_cast<double>(FLT_MAX) ||
					Value > static_cast<double>(FLT_MAX);
			}))
		{
			return false;
		}
		Target = { static_cast<float>(Source[0]), static_cast<float>(Source[1]),
			static_cast<float>(Source[2]) };
		return true;
	};
	EFFECT_RECONSTRUCTED_OCCURRENCE_INFO Staged;
	Staged.strEffectAssetId = strEffectAssetId;
	Staged.strOccurrenceId = Emitter->Row.strId;
	Staged.strSourceOccurrenceRowSha256 = Emitter->Row.strRowSha256;
	Staged.strSourceElementId = Emitter->strSourceElementId;
	Staged.strSourceEmitterPath = Emitter->strSourceEmitterPath;
	Staged.eRenderer = Emitter->eRenderer;
	if (!Assign(Emitter->CueLocalTransform.vPosition,
			Staged.SourceLocalTransform.vPosition) ||
		!Assign(Emitter->CueLocalTransform.vRotationDegrees,
			Staged.SourceLocalTransform.vRotationDegrees) ||
		!Assign(Emitter->CueLocalTransform.vScale,
			Staged.SourceLocalTransform.vScale))
	{
		strOutStatus = "Reconstructed occurrence source transform is non-finite.";
		g_strStatus = strOutStatus;
		return false;
	}
	Staged.EffectiveLocalTransform.vPosition = Element->Detail.Transform.vPosition;
	Staged.EffectiveLocalTransform.vRotationDegrees =
		Element->Detail.Transform.vRotationDegrees;
	Staged.EffectiveLocalTransform.vScale = Element->Detail.Transform.vScale;
	OutInfo = std::move(Staged);
	strOutStatus.clear();
	return true;
}

bool_t Client::CEffectPresentationService::
	Stage_ReconstructedOccurrenceTuningPreview(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const std::shared_ptr<CEffectObject>& pObject,
		const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			pExpectedPreparation,
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Tuning,
		std::string& strOutStatus)
{
	if (nullptr == pDevice || nullptr == pContext ||
		nullptr == pObject || nullptr == pExpectedPreparation)
	{
		strOutStatus = "Occurrence tuning preview object/preparation is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW Cache;
	if (!Resolve_Artist31470Cache(
		pExpectedPreparation, Cache, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		Cache.pPreparation->Get_Program();
	if (nullptr == pProgram)
	{
		strOutStatus = "Occurrence tuning preview Program is unavailable.";
		g_strStatus = strOutStatus;
		return false;
	}
	EFFECT_DOCUMENT_DESC StagedDocument = *Cache.pDocument;
	if (!CEffectOccurrenceTuningCodec::Apply_ToProjectedDocument(
			StagedDocument, *pProgram, Tuning, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	uint32_t iVisibleElementCount = 0u;
	if (!Validate_Artist31470CoreDocument(
			StagedDocument, iVisibleElementCount, strOutStatus) ||
		iVisibleElementCount != Cache.Identity.iVisibleElementCount)
	{
		if (strOutStatus.empty())
			strOutStatus = "Occurrence tuning changed the prepared visual scope.";
		g_strStatus = strOutStatus;
		return false;
	}
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProjection;
	if (!Create_Artist31470VisualAdapterProjection(
			StagedDocument, pVisualProjection, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT> pPrepared;
	if (!CEffectDocumentRenderer::
			Prepare_ReconstructedSourceRuntimeWithVisualProgramAdapter(
				std::move(pDevice), std::move(pContext), Cache.pPreparation,
				pVisualProjection, pPrepared, strOutStatus) ||
		nullptr == pPrepared)
	{
		if (strOutStatus.empty())
		{
			strOutStatus =
				"Occurrence tuning visual adapter prewarm returned no result.";
		}
		g_strStatus = strOutStatus;
		return false;
	}
	if (!pObject->Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
			pVisualProjection, pPrepared, Cache.pPreparation, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	Record_Artist31470ToolPreviewConsumption(Cache.Identity);
	strOutStatus =
		"Occurrence tuning staged object-locally; shared Product cache is unchanged.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::
	Stage_ReconstructedSourceAuthoringOverlayPreview(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const std::shared_ptr<CEffectObject>& pObject,
		const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			pExpectedPreparation,
		const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Overlay,
		std::string& strOutStatus)
{
	if (nullptr == pDevice || nullptr == pContext ||
		nullptr == pObject || nullptr == pExpectedPreparation)
	{
		strOutStatus =
			"Source authoring overlay preview object/preparation is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW Cache;
	if (!Resolve_Artist31470Cache(
		pExpectedPreparation, Cache, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		Cache.pPreparation->Get_Program();
	if (nullptr == pProgram)
	{
		strOutStatus =
			"Source authoring overlay preview Program is unavailable.";
		g_strStatus = strOutStatus;
		return false;
	}

	/* The overlay is applied to an object-local document copy.  Its codec has no
	   representation for Renderer, SourceRecipe, attachment, preparation,
	   module or distribution fields, so those admitted source authorities are
	   carried unchanged into the same visual-adapter prewarm transaction. */
	EFFECT_DOCUMENT_DESC StagedDocument = *Cache.pDocument;
	if (!CEffectSourceAuthoringOverlayCodec::Apply_ToProjectedDocument(
			StagedDocument, *pProgram, Overlay, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	uint32_t iVisibleElementCount = 0u;
	if (!Validate_Artist31470CoreDocument(
			StagedDocument, iVisibleElementCount, strOutStatus) ||
		iVisibleElementCount != Cache.Identity.iVisibleElementCount)
	{
		if (strOutStatus.empty())
		{
			strOutStatus =
				"Source authoring overlay changed the prepared visual scope.";
		}
		g_strStatus = strOutStatus;
		return false;
	}
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProjection;
	if (!Create_Artist31470VisualAdapterProjection(
			StagedDocument, pVisualProjection, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT> pPrepared;
	if (!CEffectDocumentRenderer::
			Prepare_ReconstructedSourceRuntimeWithVisualProgramAdapter(
				std::move(pDevice), std::move(pContext), Cache.pPreparation,
				pVisualProjection, pPrepared, strOutStatus) ||
		nullptr == pPrepared)
	{
		if (strOutStatus.empty())
		{
			strOutStatus =
				"Source authoring overlay prewarm returned no result.";
		}
		g_strStatus = strOutStatus;
		return false;
	}
	if (!pObject->Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
			pVisualProjection, pPrepared, Cache.pPreparation, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	Record_Artist31470ToolPreviewConsumption(Cache.Identity);
	strOutStatus =
		"Source authoring overlay staged object-locally; immutable source preparation and shared Product cache are unchanged.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Spawn_ReconstructedArtist31470(
	const EFFECT_SPAWN_DESC& Desc,
	std::string& strOutStatus)
{
	const std::shared_ptr<CCharacter> pOwner = Desc.pOwner.lock();
	if (Desc.strEffectAssetId != RECONSTRUCTED_ARTIST_31470_ASSET_ID ||
		nullptr == pOwner || nullptr == pOwner->Get_BodyModel() ||
		Desc.strAnchorSlotId != "root" || Desc.strOccurrenceId.empty() ||
		0u == Desc.iActionStartTick ||
		!std::isfinite(Desc.fPlaybackRate) ||
		std::abs(Desc.fPlaybackRate - 1.f) > 1.0e-6f ||
		!std::isfinite(Desc.fInitialSampleTimeSeconds) ||
		Desc.fInitialSampleTimeSeconds < 0.f ||
		EFFECT_FOLLOW_POLICY::END == Desc.eFollowPolicy ||
		!Is_ValidOrientationDescriptor(Desc) ||
		EFFECT_STOP_POLICY::NATURAL != Desc.eStopPolicy)
	{
		strOutStatus =
			"Artist Core F gameplay spawn descriptor is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW Cache;
	if (!Resolve_Artist31470Cache(nullptr, Cache, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	EFFECT_SCENE_BUDGET_COST AdmissionCost;
	const EFFECT_OWNER_VIEW Owner{ pOwner, nullptr };
	if (nullptr == Cache.pDocument ||
		!Estimate_DocumentBudget(*Cache.pDocument, AdmissionCost, strOutStatus) ||
		!Can_AdmitBudget(Owner, AdmissionCost, true, strOutStatus))
	{
		++g_iSceneBudgetRejectedSpawnCount;
		g_strStatus = strOutStatus;
		return false;
	}
	if (std::any_of(g_ActiveEffects.begin(), g_ActiveEffects.end(),
		[&Desc, &pOwner](const ACTIVE_EFFECT& Effect)
		{
			return Effect.pOwner.lock() == pOwner &&
				Effect.iActionStartTick == Desc.iActionStartTick &&
				Effect.strOccurrenceId == Desc.strOccurrenceId;
		}))
	{
		strOutStatus = "Duplicate Artist 31470 action edge ignored.";
		g_strStatus = strOutStatus;
		return false;
	}

	float4x4_t Anchor{};
	if (!Resolve_Anchor(
			Owner,
			Desc.strAnchorSlotId, Anchor))
	{
		strOutStatus = "Artist 31470 root anchor is missing.";
		g_strStatus = strOutStatus;
		return false;
	}
	std::vector<SOURCE_ANCHOR_REQUEST> SourceAnchorRequests =
		Collect_ReconstructedAnchorRequests(Cache.pPreparation);
	if (!Has_ExactArtist31470AnchorContract(SourceAnchorRequests))
	{
		strOutStatus =
			"Artist 31470 five equal WP_SDM_R_Battle binding rows are unavailable.";
		g_strStatus = strOutStatus;
		return false;
	}
	float4x4_t Root{};
	if (!CAnimationEffectCueDocument::Try_ComposeRootTransform(
		Desc.LocalTransform, Anchor, Desc.eOrientationPolicy,
		Desc.fActionFacingYawDegrees, Root))
	{
		strOutStatus = "Artist 31470 root transform is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	ARTIST_31470_TRANSFORM_HISTORY ArtistTransformHistory;
	if (!Prepare_Artist31470TransformHistory(
			pOwner, SourceAnchorRequests, Root,
			Desc.fInitialSampleTimeSeconds,
			ArtistTransformHistory, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	CEffectObject::EFFECT_OBJECT_DESC ObjectDesc{};
	ObjectDesc.RootWorld = Root;
	ObjectDesc.bAutoPlay = false;
	ObjectDesc.fPlaybackRate = Desc.fPlaybackRate;
	const uint32_t iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
	const EFFECT_RENDER_PREWARM_PROBE ProbeBefore =
		CEffectDocumentRenderer::Get_PrewarmProbe();
	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectObject",
		iLevelIndex, EFFECT_LAYER, &ObjectDesc, &pGameObject)))
	{
		strOutStatus = "Artist 31470 EffectObject clone failed.";
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<CEffectObject> pEffect =
		std::dynamic_pointer_cast<CEffectObject>(pGameObject);
	EFFECT_ARTIST_31470_CACHE_IDENTITY ConsumedIdentity;
	if (nullptr == pEffect ||
		!Attach_Artist31470CoreCache(
			pEffect, Cache.pPreparation,
			ConsumedIdentity, strOutStatus))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			iLevelIndex, EFFECT_LAYER, pGameObject);
		if (nullptr == pEffect)
			strOutStatus = "Artist 31470 EffectObject prototype type mismatch.";
		g_strStatus = strOutStatus;
		return false;
	}
	const EFFECT_RENDER_PREWARM_PROBE ProbeAfter =
		CEffectDocumentRenderer::Get_PrewarmProbe();
	if (ProbeAfter.iCoreBuildCount != ProbeBefore.iCoreBuildCount ||
		ProbeAfter.iModelDiskLoadCount != ProbeBefore.iModelDiskLoadCount ||
		ProbeAfter.iTextureDiskLoadCount != ProbeBefore.iTextureDiskLoadCount ||
		ProbeAfter.iVectorFieldDiskLoadCount !=
			ProbeBefore.iVectorFieldDiskLoadCount ||
		ProbeAfter.iMutableInstanceBufferBuildCount !=
			ProbeBefore.iMutableInstanceBufferBuildCount ||
		ProbeAfter.iSynchronousDocumentStageCount !=
			ProbeBefore.iSynchronousDocumentStageCount ||
		ProbeAfter.iPreparedAttachCount !=
			ProbeBefore.iPreparedAttachCount + 1u)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			iLevelIndex, EFFECT_LAYER, pGameObject);
		strOutStatus =
			"Artist 31470 action edge violated the prepared no-I/O/no-GPU-allocation contract.";
		g_strStatus = strOutStatus;
		return false;
	}
	ACTIVE_EFFECT Active;
	Active.pObject = pEffect;
	Active.pOwner = pOwner;
	Active.iLevelIndex = iLevelIndex;
	Active.strEffectAssetId = Desc.strEffectAssetId;
	Active.strAnchorSlotId = Desc.strAnchorSlotId;
	Active.LocalTransform = Desc.LocalTransform;
	Active.eFollowPolicy = Desc.eFollowPolicy;
	Active.eOrientationPolicy = Desc.eOrientationPolicy;
	Active.fActionFacingYawDegrees = Desc.fActionFacingYawDegrees;
	Active.eStopPolicy = Desc.eStopPolicy;
	Active.iCueDurationMs = Desc.iCueDurationMs;
	Active.iActionStartTick = Desc.iActionStartTick;
	Active.iCueStartMs = Desc.iCueStartMs;
	Active.strOccurrenceId = Desc.strOccurrenceId;
	Active.fPlaybackRate = Desc.fPlaybackRate;
	Active.fElapsedCueTimeSeconds = Desc.fInitialSampleTimeSeconds;
	Active.fPendingInitialSampleTimeSeconds =
		Desc.fInitialSampleTimeSeconds;
	Active.SourceAnchorRequests = std::move(SourceAnchorRequests);
	Active.Artist31470TransformHistory =
		std::move(ArtistTransformHistory);
	Active.AdmissionCost = AdmissionCost;
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
		[&Active](const f32_t fSampleTimeSeconds,
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& strOutError)
		{
			return Build_Artist31470TransformSample(
				Active, fSampleTimeSeconds, OutSample, strOutError);
		};
	std::string strHistoryError;
	if (!pEffect->Set_SampleTimeWithTransformHistory(
			Desc.fInitialSampleTimeSeconds,
			TransformProvider, strHistoryError))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			iLevelIndex, EFFECT_LAYER, pGameObject);
		strOutStatus =
			"Artist 31470 initial transform-history seek failed: " +
			strHistoryError;
		g_strStatus = strOutStatus;
		return false;
	}
	const f64_t fCommittedHistoryClock =
		pEffect->Get_PreviewFixedStepClockSeconds();
	if (!std::isfinite(fCommittedHistoryClock) ||
		fCommittedHistoryClock < 0.0 ||
		fCommittedHistoryClock >
			static_cast<f64_t>(Desc.fInitialSampleTimeSeconds) + 1.0e-5)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			iLevelIndex, EFFECT_LAYER, pGameObject);
		strOutStatus =
			"Artist 31470 initial transform-history clock is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	Active.fElapsedCueTimeSeconds =
		static_cast<f32_t>(fCommittedHistoryClock);
	Active.bPendingInitialSeek = false;
	g_ActiveEffects.push_back(std::move(Active));
	Record_Artist31470GameplayConsumption(ConsumedIdentity);
	strOutStatus =
		"Artist Core F (33) gameplay consumed the shared cache with an instance-local fixed-step anchor history.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::
	Get_ReconstructedArtist31470CacheProbe(
		EFFECT_ARTIST_31470_CACHE_PROBE& OutProbe,
		std::string& strOutStatus)
{
	OutProbe = {};
	RECONSTRUCTED_SOURCE_RUNTIME_CACHE_VIEW Cache;
	if (!Resolve_Artist31470Cache(nullptr, Cache, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	OutProbe.Current = Cache.Identity;
	OutProbe.LastToolPreviewConsumption =
		g_LastArtist31470ToolPreviewConsumption;
	OutProbe.LastGameplayConsumption = g_LastArtist31470GameplayConsumption;
	OutProbe.iToolPreviewConsumeCount =
		g_iArtist31470ToolPreviewConsumeCount;
	OutProbe.iGameplayConsumeCount = g_iArtist31470GameplayConsumeCount;
	strOutStatus =
		"Artist Core F (33) shared-cache identity/provenance probe resolved.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Spawn(
    const EFFECT_SPAWN_DESC& Desc,
    std::string& strOutStatus)
{
	if (!CEffectReconstructedRuntimeBoundary::Admit_ProductSpawn(
		Desc.strEffectAssetId, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const EFFECT_OWNER_VIEW Owner = Resolve_Owner(Desc);
	const bool_t bDescriptorValid = Owner.Is_Valid() &&
		!Desc.strAnchorSlotId.empty() && !Desc.strOccurrenceId.empty() &&
		std::isfinite(Desc.fPlaybackRate) && Desc.fPlaybackRate > 0.f &&
		Desc.fPlaybackRate <= 16.f &&
		std::isfinite(Desc.fInitialSampleTimeSeconds) &&
		Desc.fInitialSampleTimeSeconds >= 0.f &&
		EFFECT_FOLLOW_POLICY::END != Desc.eFollowPolicy &&
		Is_ValidOrientationDescriptor(Desc) &&
		EFFECT_STOP_POLICY::END != Desc.eStopPolicy &&
		(EFFECT_STOP_POLICY::CUE_END != Desc.eStopPolicy ||
			0u != Desc.iCueDurationMs) &&
		(Desc.bUseWorldRoot ?
			(0u != Desc.iWorldRootHandle &&
			 Is_NonDegenerateAffineMatrix(Desc.WorldRoot)) :
			0u == Desc.iWorldRootHandle);
	if (!bDescriptorValid)
	{
		strOutStatus = "Effect spawn descriptor is invalid or not admitted.";
		g_strStatus = strOutStatus;
		return false;
	}
	if (!g_ProductPrewarmQueue.Is_Prepared(Desc.strEffectAssetId))
	{
		strOutStatus =
			"Effect spawn rejected because its Product target is not prepared.";
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument =
		CEffectCatalog::Find_Loaded(Desc.strEffectAssetId);
	if (nullptr == pDocument)
	{
		strOutStatus =
			"Effect spawn rejected because its prepared catalog document is absent.";
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
		pPrepared = Find_ProductPrepared(Desc.strEffectAssetId, *pDocument);
	if (nullptr == pPrepared)
	{
		strOutStatus =
			"Effect spawn rejected because its admitted animation target was not prewarmed.";
		g_strStatus = strOutStatus;
		return false;
	}
	std::shared_ptr<const CEffectScreenOverlayPresentation>
		ScreenOverlayTemplate;
	if (!Resolve_ProductScreenOverlayTemplate(
			Desc.strEffectAssetId, ScreenOverlayTemplate, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const auto Budget = g_ProductEffectBudgetCosts.find(
		Desc.strEffectAssetId);
	if (Budget == g_ProductEffectBudgetCosts.end() ||
		!Can_AdmitBudget(Owner,
			Budget == g_ProductEffectBudgetCosts.end() ?
				EFFECT_SCENE_BUDGET_COST{} : Budget->second,
			true, strOutStatus))
	{
		++g_iSceneBudgetRejectedSpawnCount;
		if (Budget == g_ProductEffectBudgetCosts.end())
			strOutStatus = "Effect spawn has no prepared scene-budget receipt.";
		g_strStatus = strOutStatus;
		return false;
	}
	const auto SameEdge = [&Desc, &Owner](const auto& Effect)
	{
		return Same_Owner(Resolve_Owner(Effect.Desc), Owner) &&
			Effect.Desc.iActionStartTick == Desc.iActionStartTick &&
			Effect.Desc.strOccurrenceId == Desc.strOccurrenceId;
	};
	const bool_t bActiveDuplicate = std::any_of(
		g_ActiveEffects.begin(), g_ActiveEffects.end(),
		[&Desc, &Owner](const ACTIVE_EFFECT& Effect)
		{
			return Same_Owner(Resolve_Owner(Effect), Owner) &&
				Effect.iActionStartTick == Desc.iActionStartTick &&
				Effect.strOccurrenceId == Desc.strOccurrenceId;
		});
	if (bActiveDuplicate ||
		std::any_of(g_PendingEffectSpawns.begin(),
			g_PendingEffectSpawns.end(), SameEdge))
	{
		strOutStatus = "Duplicate Effect cue edge ignored.";
		g_strStatus = strOutStatus;
		return false;
	}

	PENDING_EFFECT_SPAWN Pending;
	Pending.Desc = Desc;
	Pending.iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
	Pending.AdmissionCost = Budget->second;
	g_PendingEffectSpawns.push_back(std::move(Pending));
	strOutStatus = "Queued admitted Effect for post-update layer commit: " +
		Desc.strEffectAssetId;
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Spawn_WorldRoot(
	const EFFECT_WORLD_ROOT_SPAWN_DESC& Desc,
	EFFECT_WORLD_ROOT_HANDLE& OutHandle,
	std::string& strOutStatus)
{
	OutHandle = {};
	if (Desc.strEffectAssetId.empty() || Desc.strOccurrenceId.empty() ||
		0u == Desc.iSpawnTick ||
		nullptr == Desc.pBossBudgetAndLifetimeOwner.lock() ||
		!Is_NonDegenerateAffineMatrix(Desc.RootWorld) ||
		!std::isfinite(Desc.fInitialSampleTimeSeconds) ||
		Desc.fInitialSampleTimeSeconds < 0.f)
	{
		strOutStatus = "World-root Effect spawn descriptor is invalid.";
		return false;
	}

	EFFECT_SPAWN_DESC SpawnDesc;
	SpawnDesc.strEffectAssetId = Desc.strEffectAssetId;
	SpawnDesc.pBossOwner = Desc.pBossBudgetAndLifetimeOwner;
	SpawnDesc.strAnchorSlotId = "root";
	SpawnDesc.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
	SpawnDesc.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
	SpawnDesc.iActionStartTick = Desc.iSpawnTick;
	SpawnDesc.strOccurrenceId = Desc.strOccurrenceId;
	SpawnDesc.fInitialSampleTimeSeconds = Desc.fInitialSampleTimeSeconds;
	SpawnDesc.bUseWorldRoot = true;
	SpawnDesc.iWorldRootHandle = g_iNextWorldRootHandle;
	SpawnDesc.WorldRoot = Desc.RootWorld;
	if (!Spawn(SpawnDesc, strOutStatus))
		return false;

	OutHandle.iValue = g_iNextWorldRootHandle++;
	if (0u == g_iNextWorldRootHandle)
		g_iNextWorldRootHandle = 1u;
	return true;
}

bool_t Client::CEffectPresentationService::Update_WorldRoot(
	const EFFECT_WORLD_ROOT_HANDLE Handle,
	const float4x4_t& RootWorld)
{
	if (!Handle.Is_Valid() || !Is_NonDegenerateAffineMatrix(RootWorld))
		return false;
	for (PENDING_EFFECT_SPAWN& Pending : g_PendingEffectSpawns)
	{
		if (Pending.Desc.iWorldRootHandle == Handle.iValue)
		{
			Pending.Desc.WorldRoot = RootWorld;
			return true;
		}
	}
	for (ACTIVE_EFFECT& Effect : g_ActiveEffects)
	{
		if (Effect.iWorldRootHandle == Handle.iValue &&
			nullptr != Effect.pObject)
		{
			Effect.WorldRoot = RootWorld;
			Effect.pObject->Set_RootWorldForNextUpdate(RootWorld);
			return true;
		}
	}
	return false;
}

void Client::CEffectPresentationService::Stop_WorldRoot(
	const EFFECT_WORLD_ROOT_HANDLE Handle)
{
	if (!Handle.Is_Valid())
		return;
	g_PendingEffectSpawns.erase(std::remove_if(
		g_PendingEffectSpawns.begin(), g_PendingEffectSpawns.end(),
		[Handle](const PENDING_EFFECT_SPAWN& Pending)
		{
			return Pending.Desc.iWorldRootHandle == Handle.iValue;
		}), g_PendingEffectSpawns.end());
	for (size_t index = g_ActiveEffects.size(); index-- > 0u;)
	{
		if (g_ActiveEffects[index].iWorldRootHandle == Handle.iValue)
			Remove_At(index);
	}
}

void Client::CEffectPresentationService::Commit_PendingSpawns()
{
	if (g_PendingEffectSpawns.empty())
		return;

	std::vector<PENDING_EFFECT_SPAWN> Pending =
		std::move(g_PendingEffectSpawns);
	g_PendingEffectSpawns.clear();
	const uint32_t iCurrentLevel = CGameInstance::Get().Get_CurrentLevelID();
	for (PENDING_EFFECT_SPAWN& Request : Pending)
	{
		if (Request.iLevelIndex != iCurrentLevel)
		{
			g_strStatus =
				"Discarded queued Effect because its source level changed.";
			continue;
		}
		std::string Status;
		if (!Spawn_Immediate(Request.Desc, Status))
		{
			OutputDebugStringA((
				"Deferred Effect spawn failed after Object Manager update: " +
				Status + "\n").c_str());
		}
	}
}

bool_t Client::CEffectPresentationService::Spawn_Immediate(
    const EFFECT_SPAWN_DESC& Desc,
    std::string& strOutStatus)
{
	if (!CEffectReconstructedRuntimeBoundary::Admit_ProductSpawn(
		Desc.strEffectAssetId, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const EFFECT_OWNER_VIEW Owner = Resolve_Owner(Desc);
	if (!Owner.Is_Valid() ||
        Desc.strAnchorSlotId.empty() ||
		Desc.strOccurrenceId.empty() ||
		!std::isfinite(Desc.fPlaybackRate) ||
		Desc.fPlaybackRate <= 0.f || Desc.fPlaybackRate > 16.f ||
        !std::isfinite(Desc.fInitialSampleTimeSeconds) ||
        Desc.fInitialSampleTimeSeconds < 0.f ||
        EFFECT_FOLLOW_POLICY::END == Desc.eFollowPolicy ||
		!Is_ValidOrientationDescriptor(Desc) ||
        EFFECT_STOP_POLICY::END == Desc.eStopPolicy ||
        (EFFECT_STOP_POLICY::CUE_END == Desc.eStopPolicy &&
            0u == Desc.iCueDurationMs) ||
		(Desc.bUseWorldRoot ?
			(0u == Desc.iWorldRootHandle ||
			 !Is_NonDegenerateAffineMatrix(Desc.WorldRoot)) :
			0u != Desc.iWorldRootHandle))
    {
        strOutStatus = "Effect spawn descriptor is invalid or not admitted.";
        g_strStatus = strOutStatus;
        return false;
    }
	if (!g_ProductPrewarmQueue.Is_Prepared(Desc.strEffectAssetId))
	{
		strOutStatus =
			"Effect spawn rejected because its Product target is not prepared.";
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument =
		CEffectCatalog::Find_Loaded(Desc.strEffectAssetId);
	if (nullptr == pDocument)
	{
		strOutStatus =
			"Effect spawn rejected because its prepared catalog document is absent.";
		g_strStatus = strOutStatus;
		return false;
	}
    const std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
		pPrepared = Find_ProductPrepared(Desc.strEffectAssetId, *pDocument);
    if (nullptr == pPrepared)
    {
        strOutStatus =
            "Effect spawn rejected because its admitted animation target was not prewarmed.";
        g_strStatus = strOutStatus;
        return false;
    }
	std::shared_ptr<const CEffectScreenOverlayPresentation>
		ScreenOverlayTemplate;
	if (!Resolve_ProductScreenOverlayTemplate(
			Desc.strEffectAssetId, ScreenOverlayTemplate, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const auto Budget = g_ProductEffectBudgetCosts.find(
		Desc.strEffectAssetId);
	if (Budget == g_ProductEffectBudgetCosts.end() ||
		!Can_AdmitBudget(Owner,
			Budget == g_ProductEffectBudgetCosts.end() ?
				EFFECT_SCENE_BUDGET_COST{} : Budget->second,
			false, strOutStatus))
	{
		++g_iSceneBudgetRejectedSpawnCount;
		if (Budget == g_ProductEffectBudgetCosts.end())
			strOutStatus = "Effect spawn has no prepared scene-budget receipt.";
		g_strStatus = strOutStatus;
		return false;
	}
    const bool Duplicate = std::any_of(
        g_ActiveEffects.begin(), g_ActiveEffects.end(),
		[&Desc, &Owner](const ACTIVE_EFFECT& Effect)
        {
			return Same_Owner(Resolve_Owner(Effect), Owner) &&
                Effect.iActionStartTick == Desc.iActionStartTick &&
				Effect.strOccurrenceId == Desc.strOccurrenceId;
        });
    if (Duplicate)
    {
        strOutStatus = "Duplicate Effect cue edge ignored.";
        g_strStatus = strOutStatus;
        return false;
    }

	float4x4_t Root{};
	if (Desc.bUseWorldRoot)
	{
		Root = Desc.WorldRoot;
	}
	else
	{
		float4x4_t Anchor{};
		if (!Resolve_Anchor(Owner, Desc.strAnchorSlotId, Anchor))
		{
			strOutStatus = "Effect anchor is missing on the owner skeleton.";
			g_strStatus = strOutStatus;
			return false;
		}
		if (!CAnimationEffectCueDocument::Try_ComposeRootTransform(
			Desc.LocalTransform, Anchor, Desc.eOrientationPolicy,
			Desc.fActionFacingYawDegrees, Root))
		{
			strOutStatus = "Effect cue root transform is invalid.";
			g_strStatus = strOutStatus;
			return false;
		}
	}
    std::vector<SOURCE_ANCHOR_REQUEST> SourceAnchorRequests =
        Collect_SourceAnchorRequests(*pDocument);
    CEffectObject::EFFECT_OBJECT_DESC ObjectDesc{};
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProjection = CEffectCatalog::Find_VisualProjection(
			Desc.strEffectAssetId);
	ObjectDesc.pDocument = nullptr == pVisualProjection ? pDocument.get() : nullptr;
	ObjectDesc.pPreparedResources = pPrepared;
	ObjectDesc.pVisualProgramProjection = pVisualProjection;
	ObjectDesc.pScreenOverlayPresentationTemplate = ScreenOverlayTemplate;
    ObjectDesc.RootWorld = Root;
    ObjectDesc.bAutoPlay = false;
    ObjectDesc.bRequirePreparedResources = true;
	ObjectDesc.fPlaybackRate = Desc.fPlaybackRate;
    const uint32_t iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
    const EFFECT_RENDER_PREWARM_PROBE ProbeBefore =
        CEffectDocumentRenderer::Get_PrewarmProbe();
    std::shared_ptr<CGameObject> pGameObject;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
        ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectObject",
        iLevelIndex, EFFECT_LAYER, &ObjectDesc, &pGameObject)))
    {
        strOutStatus = "EffectObject clone failed.";
        g_strStatus = strOutStatus;
        return false;
    }
    const std::shared_ptr<CEffectObject> pEffect =
        std::dynamic_pointer_cast<CEffectObject>(pGameObject);
    if (nullptr == pEffect)
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            iLevelIndex, EFFECT_LAYER, pGameObject);
        strOutStatus = "EffectObject prototype returned the wrong type.";
        g_strStatus = strOutStatus;
        return false;
    }
    const EFFECT_RENDER_PREWARM_PROBE ProbeAfter =
        CEffectDocumentRenderer::Get_PrewarmProbe();
    if (ProbeAfter.iCoreBuildCount != ProbeBefore.iCoreBuildCount ||
        ProbeAfter.iModelDiskLoadCount != ProbeBefore.iModelDiskLoadCount ||
        ProbeAfter.iTextureDiskLoadCount != ProbeBefore.iTextureDiskLoadCount ||
		ProbeAfter.iVectorFieldDiskLoadCount !=
			ProbeBefore.iVectorFieldDiskLoadCount ||
		ProbeAfter.iMutableInstanceBufferBuildCount !=
			ProbeBefore.iMutableInstanceBufferBuildCount ||
        ProbeAfter.iSynchronousDocumentStageCount !=
            ProbeBefore.iSynchronousDocumentStageCount ||
        ProbeAfter.iPreparedAttachCount !=
            ProbeBefore.iPreparedAttachCount + 1u)
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            iLevelIndex, EFFECT_LAYER, pGameObject);
		strOutStatus =
			"Effect spawn violated the prepared no-I/O/no-GPU-allocation resource contract.";
        g_strStatus = strOutStatus;
        return false;
    }

    ACTIVE_EFFECT Active;
    Active.pObject = pEffect;
	Active.pOwner = Owner.pCharacter;
	Active.pBossOwner = Owner.pBoss;
    Active.iLevelIndex = iLevelIndex;
    Active.strEffectAssetId = Desc.strEffectAssetId;
    Active.strAnchorSlotId = Desc.strAnchorSlotId;
    Active.LocalTransform = Desc.LocalTransform;
    Active.eFollowPolicy = Desc.eFollowPolicy;
	Active.eOrientationPolicy = Desc.eOrientationPolicy;
	Active.fActionFacingYawDegrees = Desc.fActionFacingYawDegrees;
    Active.eStopPolicy = Desc.eStopPolicy;
    Active.iCueDurationMs = Desc.iCueDurationMs;
    Active.iActionStartTick = Desc.iActionStartTick;
    Active.iCueStartMs = Desc.iCueStartMs;
	Active.strOccurrenceId = Desc.strOccurrenceId;
    Active.fPlaybackRate = Desc.fPlaybackRate;
    Active.fElapsedCueTimeSeconds = Desc.fInitialSampleTimeSeconds;
    Active.fPendingInitialSampleTimeSeconds =
        Desc.fInitialSampleTimeSeconds;
	Active.SourceAnchorRequests = std::move(SourceAnchorRequests);
	Active.AdmissionCost = Budget->second;
	Active.iWorldRootHandle = Desc.iWorldRootHandle;
	Active.WorldRoot = Desc.WorldRoot;
    g_ActiveEffects.push_back(std::move(Active));
    strOutStatus = "Spawned admitted Effect: " + Desc.strEffectAssetId;
    g_strStatus = strOutStatus;
    return true;
}

void Client::CEffectPresentationService::Update(const f32_t fTimeDelta)
{
    const uint32_t iCurrentLevel = CGameInstance::Get().Get_CurrentLevelID();
    for (size_t iEffect = g_ActiveEffects.size(); iEffect-- > 0u;)
    {
        ACTIVE_EFFECT& Effect = g_ActiveEffects[iEffect];
		const EFFECT_OWNER_VIEW Owner = Resolve_Owner(Effect);
		const std::shared_ptr<CCharacter>& pCharacterOwner = Owner.pCharacter;
		if (nullptr != Effect.pObject &&
			Effect.pObject->Is_RenderFailureIsolated())
		{
			g_strStatus = Effect.pObject->Get_Status();
			OutputDebugStringA(("[Client][EffectPresentation] " +
				g_strStatus + "\n").c_str());
			Remove_At(iEffect);
			continue;
		}
		if (nullptr != pCharacterOwner && nullptr != Effect.pObject &&
			!Effect.bFollowAnchorMissing &&
			Effect.Artist31470TransformHistory.bEnabled)
		{
			const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
				[&Effect](const f32_t fSampleTimeSeconds,
					EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
					std::string& strOutError)
				{
					return Build_Artist31470TransformSample(
						Effect, fSampleTimeSeconds, OutSample, strOutError);
				};
			std::string strHistoryError;
			bool_t bHistoryCommitted = false;
			if (Effect.bPendingInitialSeek)
			{
				bHistoryCommitted =
					Effect.pObject->Set_SampleTimeWithTransformHistory(
						Effect.fPendingInitialSampleTimeSeconds,
						TransformProvider, strHistoryError);
			}
			else
			{
				const f32_t fCueDelta =
					(std::max)(0.f, fTimeDelta) * Effect.fPlaybackRate;
				bHistoryCommitted =
					Effect.pObject->Advance_PreviewWithTransformHistory(
						fCueDelta, TransformProvider, strHistoryError);
			}
			if (!bHistoryCommitted)
			{
				Effect.pObject->Set_Visible(false);
				g_strStatus =
					"Artist 31470 transform-history playback failed closed: " +
					strHistoryError;
				OutputDebugStringA(("[Client][EffectPresentation] " +
					g_strStatus + "\n").c_str());
				Remove_At(iEffect);
				continue;
			}
			const f64_t fCommittedClock =
				Effect.pObject->Get_PreviewFixedStepClockSeconds();
			if (!std::isfinite(fCommittedClock) || fCommittedClock < 0.0)
			{
				Effect.pObject->Set_Visible(false);
				g_strStatus =
					"Artist 31470 transform-history clock failed closed.";
				Remove_At(iEffect);
				continue;
			}
			Effect.fElapsedCueTimeSeconds =
				static_cast<f32_t>(fCommittedClock);
			Effect.bPendingInitialSeek = false;
		}
		else if (Effect.bPendingInitialSeek && nullptr != Effect.pObject &&
			!Effect.bFollowAnchorMissing)
		{
			Effect.pObject->Set_SampleTime(
				Effect.fPendingInitialSampleTimeSeconds);
			Effect.bPendingInitialSeek = false;
		}
		else if (nullptr != Effect.pObject && !Effect.bFollowAnchorMissing)
		{
			const f32_t fCueDelta =
				(std::max)(0.f, fTimeDelta) * Effect.fPlaybackRate;
			Effect.pObject->Advance_Preview(fCueDelta);
			Effect.fElapsedCueTimeSeconds += fCueDelta;
		}
        const bool bCueEnded =
            EFFECT_STOP_POLICY::CUE_END == Effect.eStopPolicy &&
            Effect.fElapsedCueTimeSeconds * 1000.f >= Effect.iCueDurationMs;
        if (!Owner.Is_Valid() || nullptr == Effect.pObject ||
            Effect.iLevelIndex != iCurrentLevel || bCueEnded ||
            Effect.pObject->Is_Finished() || Effect.bFollowAnchorMissing)
        {
            Remove_At(iEffect);
            continue;
        }
    }
}

void Client::CEffectPresentationService::Synchronize_FollowAnchors()
{
    const uint32_t iCurrentLevel = CGameInstance::Get().Get_CurrentLevelID();
    for (ACTIVE_EFFECT& Effect : g_ActiveEffects)
    {
		const EFFECT_OWNER_VIEW Owner = Resolve_Owner(Effect);
        if (!Owner.Is_Valid() || nullptr == Effect.pObject ||
            Effect.iLevelIndex != iCurrentLevel)
        {
            Effect.bFollowAnchorMissing = true;
            if (nullptr != Effect.pObject)
                Effect.pObject->Set_Visible(false);
            continue;
        }
		if (Effect.Artist31470TransformHistory.bEnabled)
		{
			const std::shared_ptr<Engine::CModel> pHistoryModel =
				Effect.Artist31470TransformHistory.pModel.lock();
			if (nullptr == Owner.pCharacter || nullptr == pHistoryModel ||
				Owner.pCharacter->Get_BodyModel() != pHistoryModel ||
				!Is_NonDegenerateAffineMatrix(
					Effect.Artist31470TransformHistory.ActionStartRootWorld))
			{
				Effect.bFollowAnchorMissing = true;
				Effect.pObject->Set_Visible(false);
			}
			/* Artist 31470 has no gameplay movement.  Its typed history provider
			   owns both the captured root and WP_SDM_R_Battle anchor, so the live
			   current-pose synchronization below must not overwrite either one. */
			continue;
		}
		if (0u != Effect.iWorldRootHandle)
		{
			Effect.pObject->Set_RootWorldForNextUpdate(Effect.WorldRoot);
			continue;
		}
		Resolve_SourceAnchors(
			Owner, Effect.SourceAnchorRequests,
			Effect.SourceAnchorWorldsScratch);
		Effect.pObject->Set_SourceAnchorWorlds(
			std::move(Effect.SourceAnchorWorldsScratch));
        if (EFFECT_FOLLOW_POLICY::FOLLOW != Effect.eFollowPolicy &&
            !Effect.bPendingInitialSeek)
            continue;
        float4x4_t Anchor{};
        if (!Resolve_Anchor(Owner, Effect.strAnchorSlotId, Anchor))
        {
            Effect.bFollowAnchorMissing = true;
            Effect.pObject->Set_Visible(false);
            continue;
        }
		float4x4_t Root{};
		if (!CAnimationEffectCueDocument::Try_ComposeRootTransform(
			Effect.LocalTransform, Anchor, Effect.eOrientationPolicy,
			Effect.fActionFacingYawDegrees, Root))
		{
			Effect.bFollowAnchorMissing = true;
			Effect.pObject->Set_Visible(false);
			continue;
		}
		Effect.pObject->Set_RootWorldForNextUpdate(Root);
    }
}

void Client::CEffectPresentationService::Stop_Owner(
    const std::shared_ptr<CCharacter>& pOwner)
{
	g_PendingEffectSpawns.erase(std::remove_if(
		g_PendingEffectSpawns.begin(), g_PendingEffectSpawns.end(),
		[&pOwner](const PENDING_EFFECT_SPAWN& Pending)
		{
			return Pending.Desc.pOwner.lock() == pOwner;
		}), g_PendingEffectSpawns.end());
    for (size_t iEffect = g_ActiveEffects.size(); iEffect-- > 0u;)
    {
        if (g_ActiveEffects[iEffect].pOwner.lock() == pOwner)
            Remove_At(iEffect);
    }
}

Client::EFFECT_BOSS_ACTION_STOP_RESULT
Client::CEffectPresentationService::Stop_BossAction(
	const std::shared_ptr<CValtan>& pOwner,
	const uint32_t iActionStartTick)
{
	EFFECT_BOSS_ACTION_STOP_RESULT Result;
	if (nullptr == pOwner || 0u == iActionStartTick)
		return Result;

	const size_t iPendingBefore = g_PendingEffectSpawns.size();
	g_PendingEffectSpawns.erase(std::remove_if(
		g_PendingEffectSpawns.begin(), g_PendingEffectSpawns.end(),
		[&pOwner, iActionStartTick](const PENDING_EFFECT_SPAWN& Pending)
		{
			return 0u == Pending.Desc.iWorldRootHandle &&
				Pending.Desc.pBossOwner.lock() == pOwner &&
				Pending.Desc.iActionStartTick == iActionStartTick;
		}), g_PendingEffectSpawns.end());
	Result.iPendingStopped = static_cast<uint64_t>(
		iPendingBefore - g_PendingEffectSpawns.size());

	for (size_t iEffect = g_ActiveEffects.size(); iEffect-- > 0u;)
	{
		const ACTIVE_EFFECT& Effect = g_ActiveEffects[iEffect];
		if (0u != Effect.iWorldRootHandle ||
			Effect.pBossOwner.lock() != pOwner ||
			Effect.iActionStartTick != iActionStartTick)
		{
			continue;
		}
		Remove_At(iEffect);
		++Result.iActiveStopped;
	}
	return Result;
}

void Client::CEffectPresentationService::Stop_BossOwner(
	const std::shared_ptr<CValtan>& pOwner)
{
	g_PendingEffectSpawns.erase(std::remove_if(
		g_PendingEffectSpawns.begin(), g_PendingEffectSpawns.end(),
		[&pOwner](const PENDING_EFFECT_SPAWN& Pending)
		{
			return Pending.Desc.pBossOwner.lock() == pOwner;
		}), g_PendingEffectSpawns.end());
	for (size_t iEffect = g_ActiveEffects.size(); iEffect-- > 0u;)
	{
		if (g_ActiveEffects[iEffect].pBossOwner.lock() == pOwner)
			Remove_At(iEffect);
	}
}

void Client::CEffectPresentationService::Clear_Level(
    const uint32_t iLevelIndex)
{
	g_PendingEffectSpawns.erase(std::remove_if(
		g_PendingEffectSpawns.begin(), g_PendingEffectSpawns.end(),
		[iLevelIndex](const PENDING_EFFECT_SPAWN& Pending)
		{
			return Pending.iLevelIndex == iLevelIndex;
		}), g_PendingEffectSpawns.end());
    for (size_t iEffect = g_ActiveEffects.size(); iEffect-- > 0u;)
    {
        if (g_ActiveEffects[iEffect].iLevelIndex == iLevelIndex)
            Remove_At(iEffect);
    }
}

void Client::CEffectPresentationService::Clear_All()
{
	g_PendingEffectSpawns.clear();
    while (!g_ActiveEffects.empty())
        Remove_At(g_ActiveEffects.size() - 1u);
    g_strStatus = "All runtime Effect instances cleared.";
}

void Client::CEffectPresentationService::Release_PreparedResources()
{
    Clear_All();
	g_ProductPrewarmQueue.Clear();
	g_ProductEffectBudgetCosts.clear();
	g_ProductEffectPlaybackDurations.clear();
	g_ProductScreenOverlayTemplates.clear();
	g_iSceneBudgetRejectedSpawnCount = 0u;
	g_ReconstructedArtist31470 = {};
	g_LastArtist31470ToolPreviewConsumption = {};
	g_LastArtist31470GameplayConsumption = {};
	g_iArtist31470ToolPreviewConsumeCount = 0u;
	g_iArtist31470GameplayConsumeCount = 0u;
    CEffectDocumentRenderer::Clear_Prepared_Catalog();
    g_strStatus = "Effect product prewarm resources released.";
}

const std::string& Client::CEffectPresentationService::Get_Status()
{
    return g_strStatus;
}

