#include "Effect_DocumentRenderer_Internal.h"
#include "Effect_Catalog.h"
#include "Model.h"
#include <d3d11sdklayers.h>
#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Shader.h"
#include "Engine_RenderTypes.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
#include "VIBuffer_Rect.h"

bool_t Client::CEffectDocumentRenderer::Clone_ModelCueResources(
	const PREPARED_DOCUMENT& Prepared,
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>& OutResources,
	std::string& strOutError) const
{
	std::unordered_map<std::string, MODEL_CUE_RESOURCE> Staged;
	for (const auto& [CueId, Prototype] : Prepared.ModelCuePrototypes)
	{
		if (nullptr == Prototype.pModel)
		{
			strOutError = "Prepared animated Model Cue prototype is missing: " +
				CueId;
			return false;
		}
		const std::shared_ptr<CPrototype> CloneBase =
			Prototype.pModel->Clone(nullptr);
		const std::shared_ptr<Engine::CModel> Model =
			std::dynamic_pointer_cast<Engine::CModel>(CloneBase);
		if (nullptr == Model)
		{
			strOutError = "Prepared animated Model Cue clone failed: " + CueId;
			return false;
		}
		Model->Set_Animation(Prototype.iAnimationIndex, false);
		if (!Model->Set_AnimTrackPosition(
			Prototype.iAnimationIndex, 0.f))
		{
			strOutError = "Prepared animated Model Cue reset failed: " + CueId;
			return false;
		}
		// Play_Animation reports whether a non-looping clip finished.  At the
		// reset position a valid clip therefore returns false; that is not a
		// staging failure.
		Model->Play_Animation(0.f);
		MODEL_CUE_RESOURCE Resource;
		Resource.pModel = Model;
		Resource.pMaterialResource = Prototype.pMaterialResource;
		Resource.iAnimationIndex = Prototype.iAnimationIndex;
		Resource.fTicksPerSecond = Prototype.fTicksPerSecond;
		Resource.fDurationSeconds = Prototype.fDurationSeconds;
		Staged.emplace(CueId, std::move(Resource));
	}
	OutResources = std::move(Staged);
	return true;
}

bool_t Client::CEffectDocumentRenderer::Validate_PreparedInstanceBuffers(
	const EFFECT_DOCUMENT_DESC& Document,
	const PREPARED_DOCUMENT& Prepared,
	std::string& strOutError) const
{
	for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
	{
		const auto Resource = Prepared.ModelCuePrototypes.find(Cue.strCueId);
		if (Resource == Prepared.ModelCuePrototypes.end() ||
			!std::isfinite(Resource->second.fDurationSeconds) ||
			Resource->second.fDurationSeconds <= 0.f ||
			(!Cue.bLoop && !Cue.bHoldLastFrame &&
			 Cue.fDurationSeconds > Resource->second.fDurationSeconds + 0.001f))
		{
			strOutError = "Prepared Model Cue timing is invalid: " + Cue.strCueId;
			return false;
		}
	}
	uint32_t iRequiredTrailPoints = 0u;
	if (!Try_ResolveTrailBufferPointCapacity(
			Document, iRequiredTrailPoints, strOutError))
	{
		return false;
	}
	if ((Needs_ParticleInstanceBuffer(Document) &&
			nullptr == m_pParticleBuffer) ||
		(0u != iRequiredTrailPoints && nullptr == Prepared.pTrailBuffer) ||
		(0u == iRequiredTrailPoints && nullptr != Prepared.pTrailBuffer))
	{
		strOutError =
			"Prepared Effect mutable buffer contract does not match the Document.";
		return false;
	}
	return true;
}

bool_t Client::CEffectDocumentRenderer::Prepare_Catalog(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint64_t iCatalogRevision,
	const std::vector<std::pair<std::string,
		std::shared_ptr<const EFFECT_DOCUMENT_DESC>>>& Documents,
	std::string& strOutError)
{
	const std::shared_ptr<const CEffectMaterialProgramRegistry> Registry =
		CEffectCatalog::Acquire_MaterialProgramRegistry();
	if (nullptr == Registry ||
		Registry->Get_CatalogRevision() != iCatalogRevision)
	{
		strOutError =
			"Effect catalog preparation has no matching immutable "
			"material-program generation.";
		return false;
	}
	std::vector<EFFECT_RENDER_PREWARM_TARGET> Targets;
	Targets.reserve(Documents.size());
	for (const auto& [EffectId, Document] : Documents)
		Targets.push_back({ EffectId, Document, nullptr, Registry });
	return Prepare_VisualProgramCatalog(std::move(pDevice),
		std::move(pContext), iCatalogRevision, Targets, strOutError);
}

bool_t Client::CEffectDocumentRenderer::Prepare_VisualProgramCatalog(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint64_t iCatalogRevision,
	const std::vector<EFFECT_RENDER_PREWARM_TARGET>& Targets,
	std::string& strOutError)
{
	const std::shared_ptr<const CEffectMaterialProgramRegistry> Registry =
		Targets.empty() ? nullptr : Targets.front().pMaterialProgramRegistry;
	if (nullptr == pDevice || nullptr == pContext || 0u == iCatalogRevision ||
		Targets.empty() || nullptr == Registry ||
		Registry->Get_CatalogRevision() != iCatalogRevision ||
		Registry->Get_BindingCount() > UINT32_MAX ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError = "Effect product prewarm arguments or renderer core are invalid.";
		return false;
	}

	std::map<PREPARED_KEY, std::shared_ptr<const PREPARED_DOCUMENT>> Existing;
	std::shared_ptr<PRODUCT_PREWARM_SESSION> ExistingSession;
	uint64_t iStagedFromGeneration = 0u;
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		iStagedFromGeneration = g_iPreparedCatalogGeneration;
		if (g_pPreparedDevice == pDevice.Get() &&
			g_iPreparedCatalogRevision == iCatalogRevision)
		{
			Existing = g_PreparedEffectDocuments;
			if (nullptr != g_pProductPrewarmSession &&
				g_pProductPrewarmSession->pDevice == pDevice.Get() &&
				g_pProductPrewarmSession->pContext == pContext.Get() &&
				g_pProductPrewarmSession->iCatalogRevision == iCatalogRevision &&
				g_pProductPrewarmSession->pMaterialProgramRegistry.get() ==
					Registry.get())
			{
				ExistingSession = g_pProductPrewarmSession;
			}
		}
	}
	std::map<PREPARED_KEY, std::shared_ptr<const PREPARED_DOCUMENT>> Staged;
	std::unordered_map<const EFFECT_DOCUMENT_DESC*,
		std::shared_ptr<const PREPARED_DOCUMENT>> StagedByIdentity;
	PREWARM_ASSET_CACHE SharedAssets = nullptr == ExistingSession ?
		PREWARM_ASSET_CACHE{} : ExistingSession->SharedAssets;
	std::unordered_set<std::string> EffectIds;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	for (const EFFECT_RENDER_PREWARM_TARGET& Target : Targets)
	{
		const std::string& EffectId = Target.strEffectAssetId;
		const std::shared_ptr<const EFFECT_DOCUMENT_DESC>& Document =
			Target.pDocument;
		const std::shared_ptr<const
			EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>& Projection =
			Target.pVisualProgramProjection;
		if (EffectId.empty() || nullptr == Document ||
			Target.pMaterialProgramRegistry.get() != Registry.get() ||
			EffectId != Document->strEffectAssetId ||
			(nullptr != Projection &&
			 (!Projection->Is_Valid() ||
			  Projection->Get_EffectAssetId() != EffectId ||
			  Projection->Get_DocumentShared().get() != Document.get())) ||
			!EffectIds.insert(EffectId).second)
		{
			strOutError = "Effect product prewarm contains an invalid or duplicate target.";
			return false;
		}
		const PREPARED_KEY Key{
			iCatalogRevision, EffectId, Build_ResourceSignature(*Document),
			nullptr == Projection ? std::string{} :
				Projection->Get_AdmissionTokenSha256() };
		const auto Reusable = Existing.find(Key);
		if (Reusable != Existing.end() && nullptr != Reusable->second &&
			Reusable->second->pCatalogDocumentIdentity == Document.get() &&
			Reusable->second->pVisualProgramProjection.get() == Projection.get() &&
			Reusable->second->pMaterialProgramRegistry.get() == Registry.get() &&
			Reusable->second->pImmutableDocument.get() == Document.get())
		{
			Staged.emplace(Key, Reusable->second);
			if (!StagedByIdentity.emplace(
					Document.get(), Reusable->second).second)
			{
				strOutError =
					"Effect product prewarm reused one document identity twice.";
				return false;
			}
			continue;
		}
		std::shared_ptr<const PREPARED_DOCUMENT> Prepared;
		if (!Loader.Build_PreparedDocument(
			iCatalogRevision, EffectId, *Document, &SharedAssets,
			Prepared, strOutError, nullptr, Projection, Document, Registry))
		{
			return false;
		}
		if (!StagedByIdentity.emplace(Document.get(), Prepared).second)
		{
			strOutError =
				"Effect product prewarm produced one document identity twice.";
			return false;
		}
		Staged.emplace(Key, std::move(Prepared));
	}

	auto StagedSession = std::make_shared<PRODUCT_PREWARM_SESSION>();
	StagedSession->pDevice = pDevice.Get();
	StagedSession->pContext = pContext.Get();
	StagedSession->iCatalogRevision = iCatalogRevision;
	StagedSession->pMaterialProgramRegistry = Registry;
	StagedSession->SharedAssets = std::move(SharedAssets);
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		if (g_iPreparedCatalogGeneration != iStagedFromGeneration)
		{
			strOutError =
				"Effect product prewarm cache changed during batch staging.";
			return false;
		}
		g_PreparedEffectDocuments = std::move(Staged);
		g_PreparedEffectDocumentsByIdentity = std::move(StagedByIdentity);
		g_pProductPrewarmSession = std::move(StagedSession);
		g_pPreparedDevice = pDevice.Get();
		g_iPreparedCatalogRevision = iCatalogRevision;
		++g_iPreparedCatalogGeneration;
		++g_EffectRenderPrewarmProbe.iCatalogCommitCount;
		g_EffectRenderPrewarmProbe.iCatalogRevision = iCatalogRevision;
		g_EffectRenderPrewarmProbe.iMaterialProgramRegistryGeneration =
			Registry->Get_GenerationId();
		g_EffectRenderPrewarmProbe.iMaterialProgramBindingCount =
			static_cast<uint32_t>(Registry->Get_BindingCount());
		g_EffectRenderPrewarmProbe.iPreparedDocumentCount =
			static_cast<uint32_t>(g_PreparedEffectDocuments.size());
		g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount = 0u;
		for (const auto& [Key, Prepared] : g_PreparedEffectDocuments)
		{
			(void)Key;
			if (nullptr != Prepared)
			{
				g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount +=
					Prepared->iMaterialProgramResolvedElementCount;
			}
		}
	}
	strOutError = "Prepared " + std::to_string(Targets.size()) +
		" admitted animation Effect targets for catalog revision " +
		std::to_string(iCatalogRevision) + ".";
	return true;
}

bool_t Client::CEffectDocumentRenderer::Prepare_VisualProgramTarget(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint64_t iCatalogRevision,
	const EFFECT_RENDER_PREWARM_TARGET& Target,
	std::string& strOutError)
{
	std::shared_ptr<PRODUCT_TARGET_STAGE> Stage;
	if (!Stage_VisualProgramTarget(
			std::move(pDevice), std::move(pContext), iCatalogRevision, Target,
			Stage, strOutError))
	{
		return false;
	}
	return Commit_VisualProgramTargetStage(Stage, strOutError);
}

bool_t Client::CEffectDocumentRenderer::Stage_VisualProgramTarget(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContextIdentity,
	const uint64_t iCatalogRevision,
	const EFFECT_RENDER_PREWARM_TARGET& Target,
	std::shared_ptr<PRODUCT_TARGET_STAGE>& OutStage,
	std::string& strOutError)
{
	OutStage.reset();
	const CTargetPreparationTimer PreparationTimer;
	const std::string& EffectId = Target.strEffectAssetId;
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC>& Document =
		Target.pDocument;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
		Projection = Target.pVisualProgramProjection;
	const std::shared_ptr<const CEffectMaterialProgramRegistry>& Registry =
		Target.pMaterialProgramRegistry;
	if (nullptr == pDevice || nullptr == pContextIdentity ||
		0u == iCatalogRevision ||
		EffectId.empty() || nullptr == Document ||
		nullptr == Registry ||
		Registry->Get_CatalogRevision() != iCatalogRevision ||
		Registry->Get_BindingCount() > UINT32_MAX ||
		EffectId != Document->strEffectAssetId ||
		(nullptr != Projection &&
			(!Projection->Is_Valid() ||
			 Projection->Get_EffectAssetId() != EffectId ||
			 Projection->Get_DocumentShared().get() != Document.get())))
	{
		strOutError =
			"Effect incremental Product prewarm target is invalid.";
		return false;
	}
	const std::shared_ptr<EFFECT_RENDERER_CORE> RendererCore =
		Acquire_RendererCore(pDevice, pContextIdentity);
	if (nullptr == RendererCore)
	{
		strOutError =
			"Effect incremental Product prewarm renderer core is unavailable.";
		return false;
	}

	auto Stage = std::make_shared<PRODUCT_TARGET_STAGE>();
	Stage->pDevice = pDevice;
	Stage->pContextIdentity = pContextIdentity;
	Stage->iCatalogRevision = iCatalogRevision;
	Stage->strEffectAssetId = EffectId;
	Stage->pDocument = Document;
	Stage->pVisualProgramProjection = Projection;
	Stage->pMaterialProgramRegistry = Registry;
	Stage->pRendererCoreIdentity = RendererCore;
	Stage->Key = PREPARED_KEY{
		iCatalogRevision, EffectId, Build_ResourceSignature(*Document),
		nullptr == Projection ? std::string{} :
			Projection->Get_AdmissionTokenSha256() };
	PREWARM_ASSET_CACHE StagedSharedAssets;
	bool_t bMergeExisting = false;
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		const auto CurrentRendererCore =
			g_EffectRendererCores.find(pDevice.Get());
		if (CurrentRendererCore == g_EffectRendererCores.end() ||
			CurrentRendererCore->second.get() != RendererCore.get())
		{
			strOutError =
				"Effect renderer core identity changed before target staging.";
			return false;
		}
		Stage->iStagedFromGeneration = g_iPreparedCatalogGeneration;
		Stage->pStagedFromDevice = g_pPreparedDevice;
		Stage->iStagedFromRevision = g_iPreparedCatalogRevision;
		bMergeExisting = g_pPreparedDevice == pDevice.Get() &&
			g_iPreparedCatalogRevision == iCatalogRevision;
		if (bMergeExisting && nullptr != g_pProductPrewarmSession &&
			(g_pProductPrewarmSession->pDevice != pDevice.Get() ||
			 g_pProductPrewarmSession->pContext != pContextIdentity.Get() ||
			 g_pProductPrewarmSession->iCatalogRevision != iCatalogRevision ||
			 g_pProductPrewarmSession->pMaterialProgramRegistry.get() !=
				Registry.get()))
		{
			strOutError =
				"Effect incremental Product prewarm session identity changed.";
			return false;
		}
		if (bMergeExisting)
		{
			const auto Existing = g_PreparedEffectDocuments.find(Stage->Key);
			if (Existing != g_PreparedEffectDocuments.end())
			{
				if (nullptr == Existing->second ||
					Existing->second->pCatalogDocumentIdentity != Document.get() ||
					Existing->second->pVisualProgramProjection.get() !=
						Projection.get() ||
					Existing->second->pMaterialProgramRegistry.get() !=
						Registry.get() ||
					Existing->second->pImmutableDocument.get() != Document.get())
				{
					strOutError =
						"Effect incremental Product prewarm duplicate identity diverged.";
					return false;
				}
				Stage->pPrepared = Existing->second;
				Stage->bAlreadyPrepared = true;
				Stage->strCommitSuccessStatus =
					"Product Effect target is already prepared.";
				std::string StageStatus =
					"Product Effect target is already prepared.";
				OutStage = std::move(Stage);
				strOutError.swap(StageStatus);
				return true;
			}
			const auto SameId = std::find_if(
				g_PreparedEffectDocuments.begin(),
				g_PreparedEffectDocuments.end(),
				[&EffectId, iCatalogRevision](const auto& Entry)
				{
					return Entry.first.iCatalogRevision == iCatalogRevision &&
						Entry.first.strEffectAssetId == EffectId;
				});
			if (SameId != g_PreparedEffectDocuments.end())
			{
				strOutError =
					"Effect incremental Product prewarm found a stale target identity.";
				return false;
			}
			if (nullptr != g_pProductPrewarmSession)
				StagedSharedAssets = g_pProductPrewarmSession->SharedAssets;
			Stage->CandidateDocuments = g_PreparedEffectDocuments;
			Stage->CandidateDocumentsByIdentity =
				g_PreparedEffectDocumentsByIdentity;
		}
	}

	CEffectDocumentRenderer Loader(pDevice, pContextIdentity);
	std::shared_ptr<const PREPARED_DOCUMENT> Prepared;
	if (!Loader.Build_PreparedDocument(
		iCatalogRevision, EffectId, *Document, &StagedSharedAssets,
		Prepared, strOutError, nullptr, Projection, Document, Registry))
	{
		return false;
	}
	Stage->pPrepared = Prepared;
	Stage->pCandidateSession = std::make_shared<PRODUCT_PREWARM_SESSION>();
	Stage->pCandidateSession->pDevice = pDevice.Get();
	Stage->pCandidateSession->pContext = pContextIdentity.Get();
	Stage->pCandidateSession->iCatalogRevision = iCatalogRevision;
	Stage->pCandidateSession->pMaterialProgramRegistry = Registry;
	Stage->pCandidateSession->SharedAssets = std::move(StagedSharedAssets);
	if (!Stage->CandidateDocuments.emplace(Stage->Key, Prepared).second ||
		!Stage->CandidateDocumentsByIdentity.emplace(
			Document.get(), Prepared).second)
	{
		strOutError =
			"Effect incremental Product prewarm merge identity is duplicate.";
		return false;
	}
	uint64_t iResolvedElementCount = 0u;
	for (const auto& [PreparedKey, PreparedEntry] : Stage->CandidateDocuments)
	{
		(void)PreparedKey;
		if (nullptr != PreparedEntry)
		{
			iResolvedElementCount +=
				PreparedEntry->iMaterialProgramResolvedElementCount;
		}
	}
	if (iResolvedElementCount > UINT32_MAX ||
		Stage->CandidateDocuments.size() > UINT32_MAX)
	{
		strOutError =
			"Effect incremental Product prewarm candidate exceeds probe bounds.";
		return false;
	}
	Stage->iResolvedElementCount =
		static_cast<uint32_t>(iResolvedElementCount);
	Stage->strCommitSuccessStatus =
		"Incrementally committed Product Effect target " + EffectId +
		" for catalog revision " + std::to_string(iCatalogRevision) + ".";
	std::string StageStatus = "Staged Product Effect target " + EffectId +
		" for owner-thread commit.";
	OutStage = std::move(Stage);
	strOutError.swap(StageStatus);
	return true;
}

bool_t Client::CEffectDocumentRenderer::Commit_VisualProgramTargetStage(
	const std::shared_ptr<PRODUCT_TARGET_STAGE>& pStage,
	std::string& strOutError)
{
	const auto CommitStarted = std::chrono::steady_clock::now();
	if (nullptr == pStage || nullptr == pStage->pDevice ||
		nullptr == pStage->pContextIdentity ||
		0u == pStage->iCatalogRevision ||
		pStage->strEffectAssetId.empty() || nullptr == pStage->pDocument ||
		nullptr == pStage->pMaterialProgramRegistry ||
		nullptr == pStage->pRendererCoreIdentity ||
		pStage->pMaterialProgramRegistry->Get_CatalogRevision() !=
			pStage->iCatalogRevision ||
		pStage->strEffectAssetId != pStage->pDocument->strEffectAssetId)
	{
		strOutError =
			"Effect incremental Product prewarm staged target is invalid.";
		return false;
	}
	const std::scoped_lock Lock(g_EffectRenderCacheMutex);
	if (pStage->bConsumed)
	{
		strOutError =
			"Effect incremental Product prewarm staged target was already consumed.";
		return false;
	}
	if (pStage->strCommitSuccessStatus.empty())
	{
		strOutError =
			"Effect incremental Product prewarm staged target has no commit status.";
		return false;
	}
	if (g_iPreparedCatalogGeneration != pStage->iStagedFromGeneration ||
		g_pPreparedDevice != pStage->pStagedFromDevice ||
		g_iPreparedCatalogRevision != pStage->iStagedFromRevision)
	{
		strOutError =
			"Effect incremental Product prewarm cache changed during staging.";
		return false;
	}
	const auto CurrentRendererCore =
		g_EffectRendererCores.find(pStage->pDevice.Get());
	if (CurrentRendererCore == g_EffectRendererCores.end() ||
		CurrentRendererCore->second.get() != pStage->pRendererCoreIdentity.get())
	{
		strOutError =
			"Effect renderer core identity changed before target commit.";
		return false;
	}
	if (pStage->bAlreadyPrepared)
	{
		const auto Existing = g_PreparedEffectDocuments.find(pStage->Key);
		if (Existing == g_PreparedEffectDocuments.end() ||
			nullptr == Existing->second ||
			Existing->second.get() != pStage->pPrepared.get() ||
			Existing->second->pCatalogDocumentIdentity != pStage->pDocument.get() ||
			Existing->second->pVisualProgramProjection.get() !=
				pStage->pVisualProgramProjection.get() ||
			Existing->second->pMaterialProgramRegistry.get() !=
				pStage->pMaterialProgramRegistry.get())
		{
			strOutError =
				"Effect incremental Product prewarm prepared identity changed before commit.";
			return false;
		}
		pStage->bConsumed = true;
		++g_EffectRenderPrewarmProbe.iTargetCommitCount;
		g_EffectRenderPrewarmProbe.iTargetCommitMaximumMicroseconds =
			(std::max)(
				g_EffectRenderPrewarmProbe.iTargetCommitMaximumMicroseconds,
				static_cast<uint64_t>(std::chrono::duration_cast<
					std::chrono::microseconds>(
						std::chrono::steady_clock::now() - CommitStarted).count()));
		strOutError.swap(pStage->strCommitSuccessStatus);
		return true;
	}
	if (nullptr == pStage->pPrepared || nullptr == pStage->pCandidateSession ||
		pStage->pCandidateSession->pDevice != pStage->pDevice.Get() ||
		pStage->pCandidateSession->pContext != pStage->pContextIdentity.Get() ||
		pStage->pCandidateSession->iCatalogRevision !=
			pStage->iCatalogRevision ||
		pStage->pCandidateSession->pMaterialProgramRegistry.get() !=
			pStage->pMaterialProgramRegistry.get())
	{
		strOutError =
			"Effect incremental Product prewarm candidate session is invalid.";
		return false;
	}
	const auto Candidate = pStage->CandidateDocuments.find(pStage->Key);
	const auto CandidateByIdentity =
		pStage->CandidateDocumentsByIdentity.find(pStage->pDocument.get());
	if (Candidate == pStage->CandidateDocuments.end() ||
		CandidateByIdentity == pStage->CandidateDocumentsByIdentity.end() ||
		Candidate->second.get() != pStage->pPrepared.get() ||
		CandidateByIdentity->second.get() != pStage->pPrepared.get())
	{
		strOutError =
			"Effect incremental Product prewarm candidate identity is incomplete.";
		return false;
	}

	g_PreparedEffectDocuments.swap(pStage->CandidateDocuments);
	g_PreparedEffectDocumentsByIdentity.swap(
		pStage->CandidateDocumentsByIdentity);
	g_pProductPrewarmSession.swap(pStage->pCandidateSession);
	g_pPreparedDevice = pStage->pDevice.Get();
	g_iPreparedCatalogRevision = pStage->iCatalogRevision;
	++g_iPreparedCatalogGeneration;
	++g_EffectRenderPrewarmProbe.iCatalogCommitCount;
	g_EffectRenderPrewarmProbe.iCatalogRevision = pStage->iCatalogRevision;
	g_EffectRenderPrewarmProbe.iMaterialProgramRegistryGeneration =
		pStage->pMaterialProgramRegistry->Get_GenerationId();
	g_EffectRenderPrewarmProbe.iMaterialProgramBindingCount =
		static_cast<uint32_t>(
			pStage->pMaterialProgramRegistry->Get_BindingCount());
	g_EffectRenderPrewarmProbe.iPreparedDocumentCount =
		static_cast<uint32_t>(g_PreparedEffectDocuments.size());
	g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount =
		pStage->iResolvedElementCount;
	pStage->bConsumed = true;
	++g_EffectRenderPrewarmProbe.iTargetCommitCount;
	g_EffectRenderPrewarmProbe.iTargetCommitMaximumMicroseconds = (std::max)(
		g_EffectRenderPrewarmProbe.iTargetCommitMaximumMicroseconds,
		static_cast<uint64_t>(std::chrono::duration_cast<
			std::chrono::microseconds>(
				std::chrono::steady_clock::now() - CommitStarted).count()));
	strOutError.swap(pStage->strCommitSuccessStatus);
	return true;
}

bool_t Client::CEffectDocumentRenderer::Replace_VisualProgramTarget(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint64_t iCatalogRevision,
	const EFFECT_RENDER_PREWARM_TARGET& Target,
	std::string& strOutError)
{
	const std::string& EffectId = Target.strEffectAssetId;
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC>& Document =
		Target.pDocument;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
		Projection = Target.pVisualProgramProjection;
	const std::shared_ptr<const CEffectMaterialProgramRegistry>& Registry =
		Target.pMaterialProgramRegistry;
	if (nullptr == pDevice || nullptr == pContext || 0u == iCatalogRevision ||
		EffectId.empty() || nullptr == Document ||
		nullptr == Registry ||
		Registry->Get_CatalogRevision() != iCatalogRevision ||
		EffectId != Document->strEffectAssetId ||
		(nullptr != Projection &&
			(!Projection->Is_Valid() ||
			 Projection->Get_EffectAssetId() != EffectId ||
			 Projection->Get_DocumentShared().get() != Document.get())) ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError = "Effect Product replacement target is invalid.";
		return false;
	}

	const PREPARED_KEY CandidateKey{
		iCatalogRevision, EffectId, Build_ResourceSignature(*Document),
		nullptr == Projection ? std::string{} :
			Projection->Get_AdmissionTokenSha256() };
	PREWARM_ASSET_CACHE StagedSharedAssets;
	uint64_t iStagedFromGeneration = 0u;
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		iStagedFromGeneration = g_iPreparedCatalogGeneration;
		if (g_pPreparedDevice != pDevice.Get() ||
			g_iPreparedCatalogRevision != iCatalogRevision ||
			nullptr == g_pProductPrewarmSession ||
			g_pProductPrewarmSession->pDevice != pDevice.Get() ||
			g_pProductPrewarmSession->pContext != pContext.Get() ||
			g_pProductPrewarmSession->iCatalogRevision != iCatalogRevision ||
			g_pProductPrewarmSession->pMaterialProgramRegistry.get() !=
				Registry.get())
		{
			strOutError =
				"Effect Product replacement has no matching prepared session.";
			return false;
		}
		StagedSharedAssets = g_pProductPrewarmSession->SharedAssets;
	}

	CEffectDocumentRenderer Loader(pDevice, pContext);
	std::shared_ptr<const PREPARED_DOCUMENT> Candidate;
	if (!Loader.Build_PreparedDocument(
		iCatalogRevision, EffectId, *Document, &StagedSharedAssets,
		Candidate, strOutError, nullptr, Projection, Document, Registry))
	{
		return false;
	}
	auto StagedSession = std::make_shared<PRODUCT_PREWARM_SESSION>();
	StagedSession->pDevice = pDevice.Get();
	StagedSession->pContext = pContext.Get();
	StagedSession->iCatalogRevision = iCatalogRevision;
	StagedSession->pMaterialProgramRegistry = Registry;
	StagedSession->SharedAssets = std::move(StagedSharedAssets);

	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		if (g_iPreparedCatalogGeneration != iStagedFromGeneration ||
			g_pPreparedDevice != pDevice.Get() ||
			g_iPreparedCatalogRevision != iCatalogRevision ||
			nullptr == g_pProductPrewarmSession ||
			g_pProductPrewarmSession->pDevice != pDevice.Get() ||
			g_pProductPrewarmSession->pContext != pContext.Get() ||
			g_pProductPrewarmSession->iCatalogRevision != iCatalogRevision ||
			g_pProductPrewarmSession->pMaterialProgramRegistry.get() !=
				Registry.get())
		{
			strOutError =
				"Effect Product prepared cache changed during replacement staging.";
			return false;
		}

		auto StagedDocuments = g_PreparedEffectDocuments;
		auto StagedByIdentity = g_PreparedEffectDocumentsByIdentity;
		for (auto Iterator = StagedDocuments.begin();
			Iterator != StagedDocuments.end();)
		{
			if (Iterator->first.iCatalogRevision == iCatalogRevision &&
				Iterator->first.strEffectAssetId == EffectId)
			{
				Iterator = StagedDocuments.erase(Iterator);
			}
			else
			{
				++Iterator;
			}
		}
		for (auto Iterator = StagedByIdentity.begin();
			Iterator != StagedByIdentity.end();)
		{
			const std::shared_ptr<const PREPARED_DOCUMENT>& Prepared =
				Iterator->second;
			if (nullptr != Prepared &&
				Prepared->iCatalogRevision == iCatalogRevision &&
				Prepared->strEffectAssetId == EffectId)
			{
				Iterator = StagedByIdentity.erase(Iterator);
			}
			else
			{
				++Iterator;
			}
		}
		if (!StagedDocuments.emplace(CandidateKey, Candidate).second ||
			!StagedByIdentity.emplace(Document.get(), Candidate).second)
		{
			strOutError =
				"Effect Product replacement candidate identity is duplicate.";
			return false;
		}

		g_PreparedEffectDocuments = std::move(StagedDocuments);
		g_PreparedEffectDocumentsByIdentity = std::move(StagedByIdentity);
		g_pProductPrewarmSession = std::move(StagedSession);
		++g_iPreparedCatalogGeneration;
		++g_EffectRenderPrewarmProbe.iCatalogCommitCount;
		g_EffectRenderPrewarmProbe.iCatalogRevision = iCatalogRevision;
		g_EffectRenderPrewarmProbe.iMaterialProgramRegistryGeneration =
			Registry->Get_GenerationId();
		g_EffectRenderPrewarmProbe.iMaterialProgramBindingCount =
			static_cast<uint32_t>(Registry->Get_BindingCount());
		g_EffectRenderPrewarmProbe.iPreparedDocumentCount =
			static_cast<uint32_t>(g_PreparedEffectDocuments.size());
		g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount = 0u;
		for (const auto& [PreparedKey, PreparedEntry] :
			g_PreparedEffectDocuments)
		{
			(void)PreparedKey;
			if (nullptr != PreparedEntry)
			{
				g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount +=
					PreparedEntry->iMaterialProgramResolvedElementCount;
			}
		}
	}
	strOutError = "Replaced prepared Product Effect target " + EffectId +
		" for catalog revision " + std::to_string(iCatalogRevision) + ".";
	return true;
}

bool_t Client::CEffectDocumentRenderer::Prepare_ReconstructedSourceRuntime(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
	std::string& strOutError)
{
	OutPrepared.reset();
	if (nullptr == pDevice || nullptr == pContext || nullptr == pPreparation ||
		nullptr == pPreparation->Get_Program() ||
		Document.strEffectAssetId !=
			pPreparation->Get_Program()->strRuntimeCatalogAssetId ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError =
			"Reconstructed source runtime prewarm identity is invalid.";
		return false;
	}
	CEffectReconstructedRuntimeBoundary Boundary;
	if (!Boundary.Stage(pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
	{
		return false;
	}
	PREWARM_ASSET_CACHE SharedAssets;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	if (!Loader.Build_PreparedDocument(0u, Document.strEffectAssetId,
		Document, &SharedAssets, OutPrepared, strOutError, pPreparation))
	{
		return false;
	}
	if (nullptr == OutPrepared ||
		OutPrepared->pReconstructedRuntimePreparation.get() !=
			pPreparation.get() ||
		OutPrepared->iReconstructedNeutralBaseCount != 0u ||
		OutPrepared->iReconstructedOneLayerCount != 1u ||
		OutPrepared->iReconstructedMaterialEvaluatorCount != 28u ||
		OutPrepared->iRuntimeMaterialV2Count != 18u ||
		OutPrepared->iArtistVisualV4Count != 10u ||
		OutPrepared->iArtistVisualV4UnsupportedCount != 0u ||
		OutPrepared->iLegacyOccurrenceVisualSuppressedCount != 4u)
	{
		const uint32_t iNeutralCount = nullptr == OutPrepared ? 0u :
			OutPrepared->iReconstructedNeutralBaseCount;
		const uint32_t iOneLayerCount = nullptr == OutPrepared ? 0u :
			OutPrepared->iReconstructedOneLayerCount;
		const uint32_t iEvaluatorCount = nullptr == OutPrepared ? 0u :
			OutPrepared->iReconstructedMaterialEvaluatorCount;
		const uint32_t iRuntimeV2Count = nullptr == OutPrepared ? 0u :
			OutPrepared->iRuntimeMaterialV2Count;
		const uint32_t iArtistVisualV4Count = nullptr == OutPrepared ? 0u :
			OutPrepared->iArtistVisualV4Count;
		const uint32_t iArtistVisualV4UnsupportedCount =
			nullptr == OutPrepared ? 0u :
			OutPrepared->iArtistVisualV4UnsupportedCount;
		const uint32_t iLegacyOccurrenceVisualSuppressedCount =
			nullptr == OutPrepared ? 0u :
			OutPrepared->iLegacyOccurrenceVisualSuppressedCount;
		OutPrepared.reset();
		strOutError =
			"Reconstructed source runtime prewarm lost its immutable authority "
			"or exact material evaluator denominator: neutral=" +
			std::to_string(iNeutralCount) + ", oneLayer=" +
			std::to_string(iOneLayerCount) + ", evaluator=" +
			std::to_string(iEvaluatorCount) + ", runtimeV2=" +
			std::to_string(iRuntimeV2Count) + ", artistVisualV4=" +
			std::to_string(iArtistVisualV4Count) + ", artistVisualV4Unsupported=" +
			std::to_string(iArtistVisualV4UnsupportedCount) +
			", legacyVisualSuppressed=" +
			std::to_string(iLegacyOccurrenceVisualSuppressedCount) + ".";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Bake_ReconstructedMaterialExecutionSnapshots(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::unordered_map<std::string, EFFECT_MATERIAL_EXECUTION_DESC>&
			OutSnapshots,
		std::string& strOutError)
{
	OutSnapshots.clear();
	std::shared_ptr<const PREPARED_DOCUMENT> Prepared;
	if (!Prepare_ReconstructedSourceRuntime(pDevice, pContext, pPreparation,
		SourceDocument, Prepared, strOutError) || nullptr == Prepared)
	{
		return false;
	}

	CEffectDocumentRenderer Compiler(std::move(pDevice), std::move(pContext));
	std::unordered_map<std::string, EFFECT_MATERIAL_EXECUTION_DESC> Staged;
	Staged.reserve(SourceDocument.Elements.size());
	uint32_t iRuntimeMaterialV2Count = 0u;
	uint32_t iArtistVisualV4Count = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : SourceDocument.Elements)
	{
		const auto ResourceIt = Prepared->ElementResources.find(
			Element.strElementId);
		if (Element.strElementId.empty() ||
			ResourceIt == Prepared->ElementResources.end())
		{
			strOutError =
				"Prepared reconstructed material has no source Element resource: " +
				Element.strElementId;
			return false;
		}

		EFFECT_MATERIAL_EXECUTION_DESC Snapshot;
		std::string strElementError;
		const bool_t bTyped = Compiler.Build_MaterialExecutionSnapshot(
			Element, ResourceIt->second, Snapshot, strElementError);
		if (!bTyped && !strElementError.empty())
		{
			strOutError = std::move(strElementError);
			return false;
		}
		if (!bTyped)
			Snapshot = {};
		else if (Snapshot.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4)
		{
			++iArtistVisualV4Count;
		}
		else if (Snapshot.eBackend ==
				EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 ||
			 Snapshot.eBackend ==
				EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL)
		{
			++iRuntimeMaterialV2Count;
		}
		else
		{
			strOutError =
				"Prepared reconstructed material selected an unsupported authored "
				"backend: " + Element.strElementId;
			return false;
		}
		if (!Staged.emplace(Element.strElementId, std::move(Snapshot)).second)
		{
			strOutError =
				"Reconstructed material bake contains a duplicate Element ID: " +
				Element.strElementId;
			return false;
		}
	}
	if (Staged.size() != SourceDocument.Elements.size() ||
		iRuntimeMaterialV2Count != 18u || iArtistVisualV4Count != 10u)
	{
		strOutError =
			"Reconstructed material bake lost its typed denominator: elements=" +
			std::to_string(Staged.size()) + ", runtimeMaterialV2=" +
			std::to_string(iRuntimeMaterialV2Count) + ", artistVisualV4=" +
			std::to_string(iArtistVisualV4Count) + ".";
		return false;
	}
	OutSnapshots = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Prepare_VisualProgramDocument(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pProjection,
	std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
	std::string& strOutError)
{
	OutPrepared.reset();
	if (nullptr == pDevice || nullptr == pContext || nullptr == pProjection ||
		!pProjection->Is_Valid() ||
		pProjection->Get_EffectAssetId() !=
			pProjection->Get_Document().strEffectAssetId ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError = "Visual-program renderer prewarm identity is invalid.";
		return false;
	}
	PREWARM_ASSET_CACHE SharedAssets;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	if (!Loader.Build_PreparedDocument(0u,
		pProjection->Get_EffectAssetId(), pProjection->Get_Document(),
		&SharedAssets, OutPrepared, strOutError, nullptr, pProjection))
	{
		return false;
	}
	if (nullptr == OutPrepared ||
		OutPrepared->pVisualProgramProjection.get() != pProjection.get())
	{
		OutPrepared.reset();
		strOutError = "Visual-program renderer prewarm lost its immutable token.";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Prepare_ReconstructedSourceRuntimeWithVisualProgramAdapter(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
		std::string& strOutError)
{
	OutPrepared.reset();
	if (nullptr == pDevice || nullptr == pContext || nullptr == pPreparation ||
		nullptr == pPreparation->Get_Program() || nullptr == pProjection ||
		!pProjection->Is_Valid() ||
		pProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
		pProjection->Get_EffectAssetId() !=
			pPreparation->Get_Program()->strRuntimeCatalogAssetId ||
		pProjection->Get_Document().strEffectAssetId !=
			pProjection->Get_EffectAssetId() ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError =
			"Reconstructed visual-adapter renderer prewarm identity is invalid.";
		return false;
	}
	CEffectReconstructedRuntimeBoundary Boundary;
	if (!Boundary.Stage(pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
	{
		return false;
	}
	PREWARM_ASSET_CACHE SharedAssets;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	if (!Loader.Build_PreparedDocument(0u,
		pProjection->Get_EffectAssetId(), pProjection->Get_Document(),
		&SharedAssets, OutPrepared, strOutError, pPreparation, pProjection))
	{
		return false;
	}
	if (nullptr == OutPrepared ||
		OutPrepared->pReconstructedRuntimePreparation.get() !=
			pPreparation.get() ||
		OutPrepared->pVisualProgramProjection.get() != pProjection.get() ||
		0u == OutPrepared->iVisualProgramAdapterCount)
	{
		OutPrepared.reset();
		strOutError =
			"Reconstructed visual-adapter prewarm lost its immutable authorities.";
		return false;
	}
	strOutError.clear();
	return true;
}

std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
Client::CEffectDocumentRenderer::Find_Prepared(
	const uint64_t iCatalogRevision,
	const std::string& strEffectAssetId,
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProgramProjection,
	std::shared_ptr<const CEffectMaterialProgramRegistry>
		pMaterialProgramRegistry)
{
	const std::scoped_lock Lock(g_EffectRenderCacheMutex);
	++g_EffectRenderPrewarmProbe.iPreparedIdentityLookupCount;
	const auto Iterator = g_PreparedEffectDocumentsByIdentity.find(&Document);
	if (g_iPreparedCatalogRevision != iCatalogRevision ||
		Iterator == g_PreparedEffectDocumentsByIdentity.end() ||
		nullptr == Iterator->second ||
		Iterator->second->iCatalogRevision != iCatalogRevision ||
		Iterator->second->strEffectAssetId != strEffectAssetId ||
		Iterator->second->pCatalogDocumentIdentity != &Document ||
		Iterator->second->pImmutableDocument.get() != &Document ||
		Iterator->second->pMaterialProgramRegistry.get() !=
			pMaterialProgramRegistry.get() ||
		(nullptr != pMaterialProgramRegistry &&
			(pMaterialProgramRegistry->Get_CatalogRevision() != iCatalogRevision ||
			 Iterator->second->iMaterialProgramRegistryGeneration !=
				pMaterialProgramRegistry->Get_GenerationId())) ||
		Iterator->second->pVisualProgramProjection.get() !=
			pVisualProgramProjection.get())
	{
		++g_EffectRenderPrewarmProbe.iPreparedLookupMissCount;
		return nullptr;
	}
	return Iterator->second;
}

std::shared_ptr<const Client::CEffectPlayback::PREPARED_RESOURCES>
Client::CEffectDocumentRenderer::Get_PlaybackResources(
	const std::shared_ptr<const PREPARED_DOCUMENT>& pPrepared)
{
	return nullptr == pPrepared ? nullptr : pPrepared->pPlaybackResources;
}

Client::EFFECT_RENDER_PREWARM_PROBE
Client::CEffectDocumentRenderer::Get_PrewarmProbe()
{
	const std::scoped_lock Lock(g_EffectRenderCacheMutex);
	EFFECT_RENDER_PREWARM_PROBE Probe = g_EffectRenderPrewarmProbe;
	Probe.iVectorFieldDiskLoadCount =
		CEffectPlayback::Get_VectorFieldDiskLoadCount();
	return Probe;
}

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
bool_t Client::CEffectDocumentRenderer::Read_AuthoredTextureFormatsForTests(
	const std::string_view strElementId,
	std::array<DXGI_FORMAT, 8u>& OutFormats,
	std::string& strOutError) const
{
	const ELEMENT_RESOURCE* pResource = Find_Resource(std::string(strElementId));
	if (nullptr == pResource)
	{
		strOutError = "Authored color-space fixture has no prepared Element.";
		return false;
	}
	std::array<DXGI_FORMAT, 8u> Formats{};
	for (size_t i = 0u; i < Formats.size(); ++i)
	{
		if (nullptr == pResource->Textures[i])
			continue;
		D3D11_SHADER_RESOURCE_VIEW_DESC Desc{};
		pResource->Textures[i]->GetDesc(&Desc);
		Formats[i] = Desc.Format;
	}
	OutFormats = Formats;
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Prepare_UnboundMaterialProgramDocumentForTests(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pVisualProgramProjection,
		std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
		std::string& strOutError)
{
	OutPrepared.reset();
	if (nullptr == pDevice || nullptr == pContext || nullptr == pDocument ||
		(nullptr != pVisualProgramProjection &&
			(!pVisualProgramProjection->Is_Valid() ||
			 pVisualProgramProjection->Get_EffectAssetId() !=
				pDocument->strEffectAssetId ||
			 pVisualProgramProjection->Get_DocumentShared().get() !=
				pDocument.get())) ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError = "Binding0 comparison preparation identity is invalid.";
		return false;
	}
	PREWARM_ASSET_CACHE SharedAssets;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	if (!Loader.Build_PreparedDocument(
			0u, pDocument->strEffectAssetId, *pDocument, &SharedAssets,
			OutPrepared, strOutError, nullptr, pVisualProgramProjection) ||
		nullptr == OutPrepared || 0u != OutPrepared->iCatalogRevision ||
		0u != OutPrepared->iMaterialProgramRegistryGeneration ||
		nullptr != OutPrepared->pMaterialProgramRegistry ||
		0u != OutPrepared->iMaterialProgramResolvedElementCount)
	{
		OutPrepared.reset();
		if (strOutError.empty())
			strOutError = "Binding0 comparison preparation admitted a registry Binding.";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Validate_MaterialProgramPreparedComparisonForTests(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const std::shared_ptr<const PREPARED_DOCUMENT>& pUnboundPrepared,
		const std::shared_ptr<const PREPARED_DOCUMENT>& pBoundPrepared,
		const std::string_view strElementId,
		std::string& strOutError)
{
	if (nullptr == pDevice || nullptr == pContext ||
		nullptr == pUnboundPrepared || nullptr == pBoundPrepared ||
		strElementId.empty() ||
		pUnboundPrepared->strEffectAssetId != pBoundPrepared->strEffectAssetId ||
		pUnboundPrepared->iResourceSignature != pBoundPrepared->iResourceSignature ||
		nullptr != pUnboundPrepared->pMaterialProgramRegistry ||
		0u != pUnboundPrepared->iMaterialProgramResolvedElementCount ||
		nullptr == pBoundPrepared->pMaterialProgramRegistry ||
		pBoundPrepared->iMaterialProgramResolvedElementCount == 0u)
	{
		strOutError = "Binding0/Binding1 prepared comparison identity is invalid.";
		return false;
	}
	const EFFECT_DOCUMENT_DESC& UnboundDocument =
		nullptr == pUnboundPrepared->pImmutableDocument ?
			pUnboundPrepared->ResourceDocument :
			*pUnboundPrepared->pImmutableDocument;
	const EFFECT_DOCUMENT_DESC& BoundDocument =
		nullptr == pBoundPrepared->pImmutableDocument ?
			pBoundPrepared->ResourceDocument : *pBoundPrepared->pImmutableDocument;
	const auto FindElement = [strElementId](const EFFECT_DOCUMENT_DESC& Document)
	{
		return std::find_if(Document.Elements.begin(), Document.Elements.end(),
			[strElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			});
	};
	const auto UnboundElement = FindElement(UnboundDocument);
	const auto BoundElement = FindElement(BoundDocument);
	const auto UnboundResource = pUnboundPrepared->ElementResources.find(
		std::string(strElementId));
	const auto BoundResource = pBoundPrepared->ElementResources.find(
		std::string(strElementId));
	if (UnboundElement == UnboundDocument.Elements.end() ||
		BoundElement == BoundDocument.Elements.end() ||
		UnboundResource == pUnboundPrepared->ElementResources.end() ||
		BoundResource == pBoundPrepared->ElementResources.end() ||
		nullptr != UnboundResource->second.pMaterialProgramBinding ||
		nullptr == BoundResource->second.pMaterialProgramBinding)
	{
		strOutError = "Binding0/Binding1 occurrence resource closure is invalid.";
		return false;
	}
	CEffectDocumentRenderer Compiler(std::move(pDevice), std::move(pContext));
	EFFECT_MATERIAL_EXECUTION_DESC UnboundSnapshot;
	EFFECT_MATERIAL_EXECUTION_DESC BoundSnapshot;
	if (!Compiler.Build_MaterialExecutionSnapshot(
			*UnboundElement, UnboundResource->second, UnboundSnapshot,
			strOutError) ||
		!Compiler.Build_MaterialExecutionSnapshot(
			*BoundElement, BoundResource->second, BoundSnapshot, strOutError) ||
		!CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
			UnboundElement->Material.Execution,
			BoundElement->Material.Execution) ||
		!CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
			UnboundSnapshot, BoundSnapshot) ||
		!CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
			BoundResource->second.pMaterialProgramBinding->Execution,
			BoundSnapshot))
	{
		if (strOutError.empty())
		{
			strOutError =
				"Binding0/Binding1 execution packet or prepared snapshot diverged.";
		}
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Probe_ModelCueCloneAnimationForTests(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const std::shared_ptr<const PREPARED_DOCUMENT>& pPrepared,
		const std::string_view strCueId,
		const std::string_view strWitnessBoneName,
		EFFECT_MODEL_CUE_ANIMATION_PROBE& OutProbe,
		std::string& strOutError)
{
	OutProbe = {};
	if (nullptr == pDevice || nullptr == pContext || nullptr == pPrepared ||
		strCueId.empty() || strWitnessBoneName.empty())
	{
		strOutError = "Model Cue clone animation probe identity is invalid.";
		return false;
	}
	const EFFECT_DOCUMENT_DESC& Document =
		nullptr == pPrepared->pImmutableDocument ?
			pPrepared->ResourceDocument : *pPrepared->pImmutableDocument;
	const auto Cue = std::find_if(Document.ModelCues.begin(),
		Document.ModelCues.end(), [strCueId](const EFFECT_MODEL_CUE_DESC& Candidate)
		{
			return Candidate.strCueId == strCueId;
		});
	const std::string CueId(strCueId);
	const auto Prototype = pPrepared->ModelCuePrototypes.find(CueId);
	if (Cue == Document.ModelCues.end() || !Cue->bHoldLastFrame ||
		Prototype == pPrepared->ModelCuePrototypes.end() ||
		nullptr == Prototype->second.pModel)
	{
		strOutError =
			"Hold-last Model Cue prototype is unavailable for the animation probe: " +
			CueId;
		return false;
	}

	CEffectDocumentRenderer Renderer(std::move(pDevice), std::move(pContext));
	std::unordered_map<std::string, MODEL_CUE_RESOURCE> Clones;
	if (!Renderer.Clone_ModelCueResources(*pPrepared, Clones, strOutError))
		return false;
	const auto Clone = Clones.find(CueId);
	if (Clone == Clones.end() || nullptr == Clone->second.pModel)
	{
		strOutError = "Model Cue clone is unavailable for the animation probe: " +
			CueId;
		return false;
	}

	const MODEL_CUE_RESOURCE& PrototypeResource = Prototype->second;
	const MODEL_CUE_RESOURCE& CloneResource = Clone->second;
	OutProbe.fPrototypeDurationSeconds = PrototypeResource.fDurationSeconds;
	OutProbe.fCloneDurationSeconds = CloneResource.fDurationSeconds;
	if (!std::isfinite(PrototypeResource.fDurationSeconds) ||
		PrototypeResource.fDurationSeconds <= 0.f ||
		!std::isfinite(CloneResource.fDurationSeconds) ||
		CloneResource.fDurationSeconds <= 0.f ||
		std::fabs(PrototypeResource.fDurationSeconds -
			CloneResource.fDurationSeconds) > 0.00001f ||
		!std::isfinite(CloneResource.fTicksPerSecond) ||
		CloneResource.fTicksPerSecond <= 0.f)
	{
		strOutError = "Model Cue clone did not preserve finite clip timing: " +
			CueId;
		return false;
	}

	Engine::CModel& Model = *CloneResource.pModel;
	const std::string WitnessBoneName(strWitnessBoneName);
	if (!Model.Has_Bone(WitnessBoneName.c_str()))
	{
		strOutError = "Model Cue animation witness bone is unavailable: " +
			WitnessBoneName;
		return false;
	}
	if (!Model.Set_AnimTrackPosition(CloneResource.iAnimationIndex, 0.f))
	{
		strOutError = "Model Cue animation probe could not reset its track: " +
			CueId;
		return false;
	}
	Model.Play_Animation(0.f);
	float4x4_t StartBone{};
	XMStoreFloat4x4(&StartBone, Model.Get_BoneMatrix(WitnessBoneName.c_str()));

	const f32_t fMidAnimationSeconds = 0.5f * (std::min)(
		Cue->fDurationSeconds, CloneResource.fDurationSeconds);
	const f32_t fExpectedMidTrackTicks =
		fMidAnimationSeconds * CloneResource.fTicksPerSecond;
	if (!std::isfinite(fExpectedMidTrackTicks) ||
		fExpectedMidTrackTicks <= 0.f ||
		!Model.Set_AnimTrackPosition(
			CloneResource.iAnimationIndex, fExpectedMidTrackTicks))
	{
		strOutError = "Model Cue animation probe could not advance its track: " +
			CueId;
		return false;
	}
	Model.Play_Animation(0.f);
	f32_t fMidTrackPosition = 0.f;
	f32_t fTrackDuration = 0.f;
	if (!Model.Get_AnimationProgress(CloneResource.iAnimationIndex,
			fMidTrackPosition, fTrackDuration))
	{
		strOutError = "Model Cue animation probe could not read its track: " +
			CueId;
		return false;
	}
	if (!std::isfinite(fMidTrackPosition) ||
		!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f)
	{
		strOutError = "Model Cue animation probe read non-finite track timing: " +
			CueId;
		return false;
	}
	OutProbe.fMidTrackPositionTicks = fMidTrackPosition;
	OutProbe.fTrackDurationTicks = fTrackDuration;
	float4x4_t MidBone{};
	XMStoreFloat4x4(&MidBone, Model.Get_BoneMatrix(WitnessBoneName.c_str()));
	for (size_t iRow = 0u; iRow < 4u; ++iRow)
	{
		for (size_t iColumn = 0u; iColumn < 4u; ++iColumn)
		{
			if (!std::isfinite(StartBone.m[iRow][iColumn]) ||
				!std::isfinite(MidBone.m[iRow][iColumn]))
			{
				strOutError =
					"Model Cue animation witness matrix is non-finite: " +
					WitnessBoneName;
				return false;
			}
			OutProbe.fWitnessBoneMaximumDelta = (std::max)(
				OutProbe.fWitnessBoneMaximumDelta,
				std::fabs(StartBone.m[iRow][iColumn] -
					MidBone.m[iRow][iColumn]));
		}
	}

	const f32_t fExpectedTailTrackTicks = (std::min)(
		Cue->fDurationSeconds, CloneResource.fDurationSeconds) *
		CloneResource.fTicksPerSecond;
	if (!Model.Set_AnimTrackPosition(
			CloneResource.iAnimationIndex, fExpectedTailTrackTicks))
	{
		strOutError = "Model Cue animation probe could not clamp its tail: " +
			CueId;
		return false;
	}
	Model.Play_Animation(0.f);
	if (!Model.Get_AnimationProgress(CloneResource.iAnimationIndex,
			OutProbe.fTailTrackPositionTicks, fTrackDuration) ||
		!std::isfinite(OutProbe.fTailTrackPositionTicks) ||
		!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f ||
		std::fabs(OutProbe.fMidTrackPositionTicks -
			fExpectedMidTrackTicks) > 0.001f ||
		std::fabs(OutProbe.fTailTrackPositionTicks -
			fExpectedTailTrackTicks) > 0.001f ||
		std::fabs(OutProbe.fTrackDurationTicks - fTrackDuration) > 0.001f ||
		OutProbe.fWitnessBoneMaximumDelta <= 0.0001f)
	{
		strOutError =
			"Model Cue clone animation did not advance or clamp as authored: " +
			CueId;
		return false;
	}
	strOutError.clear();
	return true;
}

#endif

void Client::CEffectDocumentRenderer::Clear_Prepared_Catalog()
{
	const std::scoped_lock Lock(g_EffectRenderCacheMutex);
	g_PreparedEffectDocuments.clear();
	g_PreparedEffectDocumentsByIdentity.clear();
	g_pProductPrewarmSession.reset();
	g_EffectRendererCores.clear();
	g_EffectRendererCoreFailures.clear();
	g_pPreparedDevice = nullptr;
	g_iPreparedCatalogRevision = 0u;
	++g_iPreparedCatalogGeneration;
	g_EffectRenderPrewarmProbe.iCatalogRevision = 0u;
	g_EffectRenderPrewarmProbe.iMaterialProgramRegistryGeneration = 0u;
	g_EffectRenderPrewarmProbe.iPreparedDocumentCount = 0u;
	g_EffectRenderPrewarmProbe.iMaterialProgramBindingCount = 0u;
	g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount = 0u;
}
