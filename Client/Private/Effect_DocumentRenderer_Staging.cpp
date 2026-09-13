#include "Effect_DocumentRenderer_Internal.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_RuntimeAuthority.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
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
#include "Engine_RenderTypes.h"
#include "VIBuffer_Rect.h"
#include "DirectXTK/DDSTextureLoader.h"

bool_t Client::CEffectDocumentRenderer::Stage_Prepared(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
	std::string& strOutError)
{
	return Stage_PreparedInternal(Document, std::move(pPrepared), strOutError);
}

void Client::CEffectDocumentRenderer::Preserve_StartingSceneCapture(
	const CEffectDocumentRenderer& Previous)
{
	if (nullptr != m_pStartingSceneCapture &&
		Get_StagedDocument().strEffectAssetId == Previous.Get_StagedDocument().strEffectAssetId &&
		nullptr != Previous.m_pStartingSceneCapture &&
		nullptr != Previous.m_pStartingSceneBloomCapture)
	{
		m_pStartingSceneCapture = Previous.m_pStartingSceneCapture;
		m_pStartingSceneBloomCapture = Previous.m_pStartingSceneBloomCapture;
	}
}

bool_t Client::CEffectDocumentRenderer::Capture_StartingSceneTarget(const wchar_t* targetTag,
	ComPtr<ID3D11ShaderResourceView>& OutCapture, std::string& strOutError) const
{
	// Stage runs before this occurrence is rendered. Keep the last completed
	// world frame, before the cinematic camera/overlays have drawn over it.
	// This is a project initial-view adapter, not the unexported UE capture view.
	const auto SourceView = CGameInstance::Get().Get_RT_SRV(targetTag);
	ComPtr<ID3D11Resource> SourceResource;
	ComPtr<ID3D11Texture2D> SourceTexture;
	if (nullptr != SourceView) SourceView->GetResource(SourceResource.GetAddressOf());
	if (nullptr == SourceResource || FAILED(SourceResource.As(&SourceTexture)))
	{ strOutError = "Starting scene capture requires the completed scene color and bloom targets."; return false; }
	D3D11_TEXTURE2D_DESC Desc{}; SourceTexture->GetDesc(&Desc);
	if (Desc.SampleDesc.Count != 1u || Desc.ArraySize != 1u || Desc.Width == 0u || Desc.Height == 0u)
	{ strOutError = "Starting scene capture target has an unsupported texture shape."; return false; }
	Desc.Usage = D3D11_USAGE_DEFAULT; Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Desc.CPUAccessFlags = 0u; Desc.MiscFlags = 0u;
	ComPtr<ID3D11Texture2D> CaptureTexture;
	ComPtr<ID3D11ShaderResourceView> CaptureView;
	if (FAILED(m_pDevice->CreateTexture2D(&Desc, nullptr, CaptureTexture.GetAddressOf())) ||
		FAILED(m_pDevice->CreateShaderResourceView(CaptureTexture.Get(), nullptr, CaptureView.GetAddressOf())))
	{ strOutError = "Starting scene capture texture allocation failed; previous playback preserved."; return false; }
	ID3D11RenderTargetView* Targets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	ID3D11DepthStencilView* Depth = nullptr;
	m_pContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, Targets, &Depth);
	m_pContext->OMSetRenderTargets(0u, nullptr, nullptr);
	const HRESULT Result = CGameInstance::Get().Copy_RT_Resource(targetTag, CaptureTexture);
	m_pContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, Targets, Depth);
	for (auto* Target : Targets) if (Target) Target->Release();
	if (Depth) Depth->Release();
	if (FAILED(Result))
	{ strOutError = "Starting scene capture copy failed; previous playback preserved."; return false; }
	OutCapture = std::move(CaptureView); strOutError.clear(); return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_PreparedInternal(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
	std::string& strOutError)
{
	const bool_t bCatalogPrepared = nullptr != pPrepared &&
		0u != pPrepared->iCatalogRevision;
	bool_t bCatalogIdentityCurrent = true;
	if (bCatalogPrepared)
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		const auto Current = g_PreparedEffectDocumentsByIdentity.find(&Document);
		bCatalogIdentityCurrent =
			g_iPreparedCatalogRevision == pPrepared->iCatalogRevision &&
			nullptr != g_pProductPrewarmSession &&
			g_pProductPrewarmSession->pMaterialProgramRegistry.get() ==
				pPrepared->pMaterialProgramRegistry.get() &&
			nullptr != pPrepared->pMaterialProgramRegistry &&
			pPrepared->pMaterialProgramRegistry->Get_CatalogRevision() ==
				pPrepared->iCatalogRevision &&
			pPrepared->pMaterialProgramRegistry->Get_GenerationId() ==
				pPrepared->iMaterialProgramRegistryGeneration &&
			Current != g_PreparedEffectDocumentsByIdentity.end() &&
			Current->second.get() == pPrepared.get();
	}
	const bool_t bIdentityMatches = bCatalogPrepared ?
		(pPrepared->pCatalogDocumentIdentity == &Document &&
		 pPrepared->pImmutableDocument.get() == &Document) :
		(nullptr != pPrepared &&
			pPrepared->iResourceSignature == Build_ResourceSignature(Document) &&
			Resource_SignatureMatches(
				pPrepared->ResourceDocument, Document));
	if (nullptr == pPrepared || !bCatalogIdentityCurrent || !bIdentityMatches ||
		pPrepared->strEffectAssetId != Document.strEffectAssetId ||
		(!bCatalogPrepared &&
			!CEffectDocumentCodec::Validate_Drawable(Document, strOutError)))
	{
		if (strOutError.empty())
			strOutError = "Prepared Effect resources do not match the Document.";
		return false;
	}
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>
		StagedModelCueResources;
	const std::shared_ptr<Engine::CVIBuffer_DynamicTrail> pStagedTrailBuffer =
		pPrepared->pTrailBuffer;
	if (!Validate_PreparedInstanceBuffers(
			Document, *pPrepared, strOutError))
		return false;
	if (!Clone_ModelCueResources(
		*pPrepared, StagedModelCueResources, strOutError))
	{
		return false;
	}
	ComPtr<ID3D11ShaderResourceView> StartingCapture, StartingBloomCapture;
	const bool_t bNeedsStartingCapture = Requires_StartingSceneCapture(Document);
	if (bNeedsStartingCapture &&
		(!Capture_StartingSceneTarget(TEXT("Target_SceneHDR"), StartingCapture, strOutError) ||
		 !Capture_StartingSceneTarget(TEXT("Target_SceneBloom"), StartingBloomCapture, strOutError)))
		return false;
	m_pStartingSceneCapture = std::move(StartingCapture);
	m_pStartingSceneBloomCapture = std::move(StartingBloomCapture);
	m_BloomIntensityOverride.reset();
	m_Document = bCatalogPrepared ? EFFECT_DOCUMENT_DESC{} : Document;
	m_pPreparedDocument = std::move(pPrepared);
	m_pTrailBuffer = pStagedTrailBuffer;
	m_pReconstructedDiagnostic.reset();
	m_ReconstructedRuntimeBoundary.Clear();
	m_bReconstructedSourceRuntimeActive = false;
	m_bSourceVisualProgramActive = false;
	Reset_PreviewSubmissionIsolation();
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus = "Prepared Effect Document resources attached.";
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iPreparedAttachCount;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_PrevalidatedVisualProgramDocument(
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pProjection,
	std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
	std::string& strOutError)
{
	if (nullptr == pProjection || !pProjection->Is_Valid() ||
		nullptr == pPrepared ||
		pPrepared->pVisualProgramProjection.get() != pProjection.get() ||
		pPrepared->strEffectAssetId != pProjection->Get_EffectAssetId())
	{
		strOutError =
			"Visual-program renderer resources or immutable token do not match.";
		return false;
	}
	const bool_t bCatalogPrepared = nullptr != pPrepared &&
		0u != pPrepared->iCatalogRevision;
	bool_t bCatalogIdentityCurrent = true;
	if (bCatalogPrepared)
	{
		const EFFECT_DOCUMENT_DESC& Document = pProjection->Get_Document();
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		const auto Current = g_PreparedEffectDocumentsByIdentity.find(&Document);
		bCatalogIdentityCurrent =
			g_iPreparedCatalogRevision == pPrepared->iCatalogRevision &&
			nullptr != g_pProductPrewarmSession &&
			g_pProductPrewarmSession->pMaterialProgramRegistry.get() ==
				pPrepared->pMaterialProgramRegistry.get() &&
			nullptr != pPrepared->pMaterialProgramRegistry &&
			pPrepared->pMaterialProgramRegistry->Get_CatalogRevision() ==
				pPrepared->iCatalogRevision &&
			pPrepared->pMaterialProgramRegistry->Get_GenerationId() ==
				pPrepared->iMaterialProgramRegistryGeneration &&
			Current != g_PreparedEffectDocumentsByIdentity.end() &&
			Current->second.get() == pPrepared.get();
	}
	const bool_t bDocumentIdentityMatches = bCatalogPrepared ?
		(nullptr != pPrepared->pImmutableDocument &&
		 pPrepared->pImmutableDocument.get() ==
			&pProjection->Get_Document() &&
		 pPrepared->pCatalogDocumentIdentity ==
			&pProjection->Get_Document()) :
		(nullptr != pPrepared &&
		 pPrepared->iResourceSignature ==
			Build_ResourceSignature(pProjection->Get_Document()) &&
		 Resource_SignatureMatches(
			pPrepared->ResourceDocument, pProjection->Get_Document()));
	if (!bCatalogIdentityCurrent || !bDocumentIdentityMatches)
	{
		strOutError =
			"Visual-program renderer resources or immutable token do not match.";
		return false;
	}
	std::unordered_map<std::string, MODEL_CUE_RESOURCE> StagedModelCueResources;
	const std::shared_ptr<Engine::CVIBuffer_DynamicTrail> pStagedTrailBuffer =
		pPrepared->pTrailBuffer;
	if (!Validate_PreparedInstanceBuffers(
			pProjection->Get_Document(), *pPrepared, strOutError) ||
		!Clone_ModelCueResources(
			*pPrepared, StagedModelCueResources, strOutError))
	{
		return false;
	}
	m_BloomIntensityOverride.reset();
	m_Document = bCatalogPrepared ? EFFECT_DOCUMENT_DESC{} :
		pProjection->Get_Document();
	m_pPreparedDocument = std::move(pPrepared);
	m_pTrailBuffer = pStagedTrailBuffer;
	m_pReconstructedDiagnostic.reset();
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_ReconstructedRuntimeBoundary.Clear();
	m_bReconstructedSourceRuntimeActive = false;
	/* Source-module execution is enabled only by an admitted overlay program.
	   Adapter packets (for example LocalDecal) reuse the base playback document
	   and admit renderer material/projector state only. */
	m_bSourceVisualProgramActive =
		pProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1;
	Reset_PreviewSubmissionIsolation();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus = "Admitted source visual-program renderer attached.";
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iPreparedAttachCount;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_Document(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iSynchronousDocumentStageCount;
	}
	if (!Validate_DimensionMasterProjectTunedDocumentExecution(
			Document, strOutError) ||
		!CEffectDocumentCodec::Validate_Drawable(Document, strOutError))
		return false;
	if (nullptr != m_pPreparedDocument &&
		0u == m_pPreparedDocument->iCatalogRevision &&
		Resource_SignatureMatches(m_Document, Document))
	{
		// Timing and transform edits reuse models, but still admit their clip window.
		if (!Validate_PreparedInstanceBuffers(Document, *m_pPreparedDocument, strOutError))
			return false;
		// Visibility edits can reuse the same prepared resources. A previously
		// hidden capture box still needs its occurrence snapshot on first use.
		if ((!m_pStartingSceneCapture || !m_pStartingSceneBloomCapture) &&
			Requires_StartingSceneCapture(Document))
		{
			ComPtr<ID3D11ShaderResourceView> color, bloom;
			if (!Capture_StartingSceneTarget(TEXT("Target_SceneHDR"), color, strOutError) ||
				!Capture_StartingSceneTarget(TEXT("Target_SceneBloom"), bloom, strOutError)) return false;
			m_pStartingSceneCapture = std::move(color);
			m_pStartingSceneBloomCapture = std::move(bloom);
		}
		m_BloomIntensityOverride.reset();
		m_Document = Document;
		m_pReconstructedDiagnostic.reset();
		m_ReconstructedRuntimeBoundary.Clear();
		m_bReconstructedSourceRuntimeActive = false;
		m_bSourceVisualProgramActive = false;
		Reset_PreviewSubmissionIsolation();
		m_strRenderFailureDetail.clear();
		m_bLastRenderFailureObjectLocal = false;
		m_strStatus = "Effect Document values committed; GPU resources reused.";
		strOutError.clear();
		return true;
	}
	PREWARM_ASSET_CACHE SharedAssets;
	std::shared_ptr<const PREPARED_DOCUMENT> Prepared;
	if (!Build_PreparedDocument(0u, Document.strEffectAssetId, Document,
		&SharedAssets, Prepared, strOutError))
	{
		return false;
	}
	return Stage_PreparedInternal(Document, std::move(Prepared), strOutError);
}

bool_t Client::CEffectDocumentRenderer::Stage_ReconstructedRuntimeProgram(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	std::string& strOutError)
{
	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(std::move(pPreparation),
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
		return false;
	m_BloomIntensityOverride.reset();
	m_Document = {};
	m_pPreparedDocument.reset();
	m_pReconstructedDiagnostic.reset();
	m_ModelCueResources.clear();
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_bReconstructedSourceRuntimeActive = false;
	m_bSourceVisualProgramActive = false;
	Reset_PreviewSubmissionIsolation();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus =
		"Reconstructed Effect program prepared; renderer execution remains blocked.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_ReconstructedSourceRuntime(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	std::string& strOutError)
{
	if (nullptr == pPrepared || nullptr == pPreparation ||
		pPrepared->pReconstructedRuntimePreparation.get() != pPreparation.get() ||
		pPrepared->strEffectAssetId != Document.strEffectAssetId ||
		pPrepared->iResourceSignature != Build_ResourceSignature(Document) ||
		!Resource_SignatureMatches(pPrepared->ResourceDocument, Document))
	{
		strOutError =
			"Reconstructed source runtime resources or authority do not match.";
		return false;
	}
	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
	{
		return false;
	}
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>
		StagedModelCueResources;
	const std::shared_ptr<Engine::CVIBuffer_DynamicTrail> pStagedTrailBuffer =
		pPrepared->pTrailBuffer;
	if (!Validate_PreparedInstanceBuffers(
			Document, *pPrepared, strOutError) ||
		!Clone_ModelCueResources(*pPrepared,
			StagedModelCueResources, strOutError))
	{
		return false;
	}
	m_BloomIntensityOverride.reset();
	m_Document = Document;
	m_pPreparedDocument = std::move(pPrepared);
	m_pTrailBuffer = pStagedTrailBuffer;
	m_pReconstructedDiagnostic.reset();
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_bReconstructedSourceRuntimeActive = true;
	m_bSourceVisualProgramActive = true;
	Reset_PreviewSubmissionIsolation();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus =
		"Reconstructed Artist source runtime resources attached; Product remains blocked.";
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iPreparedAttachCount;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError)
{
	if (nullptr == pProjection || !pProjection->Is_Valid() ||
		pProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
		nullptr == pPrepared || nullptr == pPreparation ||
		pPrepared->pVisualProgramProjection.get() != pProjection.get() ||
		pPrepared->pReconstructedRuntimePreparation.get() !=
			pPreparation.get() ||
		pPrepared->strEffectAssetId != pProjection->Get_EffectAssetId() ||
		pPrepared->iResourceSignature !=
			Build_ResourceSignature(pProjection->Get_Document()) ||
		!Resource_SignatureMatches(pPrepared->ResourceDocument,
			pProjection->Get_Document()) ||
		0u == pPrepared->iVisualProgramAdapterCount)
	{
		strOutError =
			"Reconstructed visual-adapter renderer resources do not match.";
		return false;
	}
	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
	{
		return false;
	}
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>
		StagedModelCueResources;
	const std::shared_ptr<Engine::CVIBuffer_DynamicTrail> pStagedTrailBuffer =
		pPrepared->pTrailBuffer;
	if (!Validate_PreparedInstanceBuffers(
			pProjection->Get_Document(), *pPrepared, strOutError) ||
		!Clone_ModelCueResources(*pPrepared,
			StagedModelCueResources, strOutError))
	{
		return false;
	}
	m_BloomIntensityOverride.reset();
	m_Document = pProjection->Get_Document();
	m_pPreparedDocument = std::move(pPrepared);
	m_pTrailBuffer = pStagedTrailBuffer;
	m_pReconstructedDiagnostic.reset();
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_bReconstructedSourceRuntimeActive = true;
	/* Adapter packets add renderer state only.  The reconstructed preparation
	   continues to own source-module execution and its 35-row target closure. */
	m_bSourceVisualProgramActive = true;
	Reset_PreviewSubmissionIsolation();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus =
		"Reconstructed source runtime attached with immutable visual adapters.";
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iPreparedAttachCount;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_ReconstructedDiagnostic(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME> pFrame,
	std::string& strOutError)
{
	if (nullptr == pFrame || nullptr == pFrame->Get_Preparation() ||
		nullptr == pFrame->Get_Preparation()->Get_RuntimePreparation() ||
		nullptr == pFrame->Get_Preparation()->Get_Program() ||
		nullptr == pFrame->Get_Preparation()->Get_RenderResourceAuthority() ||
		pFrame->Get_Packets().size() != 2u)
	{
		strOutError =
			"Reconstructed diagnostic frame is incomplete or not exact-two.";
		return false;
	}
	if (nullptr == m_pMeshShader || nullptr == m_pParticleShader ||
		nullptr == m_pRect)
	{
		if (FAILED(Initialize()))
		{
			strOutError = "Reconstructed diagnostic renderer core is unavailable.";
			return false;
		}
	}

	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(
		pFrame->Get_Preparation()->Get_RuntimePreparation(),
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
	{
		return false;
	}

	auto Staged = std::make_unique<RECONSTRUCTED_DIAGNOSTIC_COMPOSITE>();
	Staged->pFrame = pFrame;
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Program =
		pFrame->Get_Preparation()->Get_Program();
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
		Authority = pFrame->Get_Preparation()->Get_RenderResourceAuthority();
	std::array<bool_t, 2u> SelectionSeen{};

	const auto StageState = [this, &Program, &Authority, &strOutError](
		const EFFECT_RECONSTRUCTED_SELECTED_STATE_BINDING& Binding,
		const EFFECT_RECONSTRUCTED_RENDER_STATE_KIND eKind,
		RECONSTRUCTED_DIAGNOSTIC_COMPOSITE::GPU_RESOURCE& Resource) -> bool_t
	{
		if (!Binding.SidecarDecision.has_value())
		{
			if (!Binding.ProgramPolicy.has_value())
			{
				strOutError =
					"Reconstructed diagnostic render state has no authority.";
				return false;
			}
			const auto BindingIterator = std::find_if(
				Program->MaterialRenderBindings.begin(),
				Program->MaterialRenderBindings.end(),
				[&Binding](const auto& Row)
				{
					return Row.Row.strId == Binding.ProgramBinding.strId &&
						Row.Row.strRowSha256 ==
							Binding.ProgramBinding.strRowSha256;
				});
			const auto PolicyIterator = std::find_if(
				Program->MaterialPolicies.begin(),
				Program->MaterialPolicies.end(),
				[&Binding](const auto& Row)
				{
					return Row.Row.strId == Binding.ProgramPolicy->strId &&
						Row.Row.strRowSha256 ==
							Binding.ProgramPolicy->strRowSha256;
				});
			if (BindingIterator == Program->MaterialRenderBindings.end() ||
				PolicyIterator == Program->MaterialPolicies.end() ||
				BindingIterator->strPolicyRowId != PolicyIterator->Row.strId ||
				PolicyIterator->eDomain !=
					EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::RENDER_STATE ||
				!PolicyIterator->D3dDescriptorOracle.has_value() ||
				PolicyIterator->D3dDescriptorOracle->strPolicyRowId !=
					PolicyIterator->Row.strId ||
				PolicyIterator->D3dDescriptorOracle->strDecision != "PASS")
			{
				strOutError =
					"Reconstructed diagnostic render policy changed.";
				return false;
			}
			const EFFECT_RUNTIME_PROGRAM_D3D_DESCRIPTOR& Descriptor =
				PolicyIterator->D3dDescriptorOracle->Actual;
			if (eKind == EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::RASTERIZER &&
				PolicyIterator->D3dDescriptorOracle->eKind ==
					EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::RASTERIZER &&
				Descriptor.Rasterizer.has_value())
			{
				const EFFECT_RUNTIME_PROGRAM_D3D_RASTERIZER& Source =
					*Descriptor.Rasterizer;
				D3D11_RASTERIZER_DESC StagedDescriptor{};
				StagedDescriptor.FillMode =
					static_cast<D3D11_FILL_MODE>(Source.iFillMode);
				StagedDescriptor.CullMode =
					static_cast<D3D11_CULL_MODE>(Source.iCullMode);
				StagedDescriptor.FrontCounterClockwise =
					Source.bFrontCounterClockwise;
				StagedDescriptor.DepthBias = Source.iDepthBias;
				StagedDescriptor.DepthBiasClamp =
					static_cast<f32_t>(Source.fDepthBiasClamp);
				StagedDescriptor.SlopeScaledDepthBias =
					static_cast<f32_t>(Source.fSlopeScaledDepthBias);
				StagedDescriptor.DepthClipEnable = Source.bDepthClipEnable;
				StagedDescriptor.ScissorEnable = Source.bScissorEnable;
				StagedDescriptor.MultisampleEnable = Source.bMultisampleEnable;
				StagedDescriptor.AntialiasedLineEnable =
					Source.bAntialiasedLineEnable;
				if (FAILED(m_pDevice->CreateRasterizerState(
					&StagedDescriptor, &Resource.pRasterizerState)))
				{
					strOutError =
						"Reconstructed diagnostic policy rasterizer-state creation failed.";
					return false;
				}
				Resource.RasterizerDescriptor = StagedDescriptor;
				Resource.bHasRasterizerDescriptor = true;
				return true;
			}
			if (eKind == EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::DEPTH_STENCIL &&
				PolicyIterator->D3dDescriptorOracle->eKind ==
					EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::DEPTH_STENCIL &&
				Descriptor.DepthStencil.has_value())
			{
				const auto ConvertFace = [](const
					EFFECT_RUNTIME_PROGRAM_D3D_STENCIL_FACE& Source)
				{
					D3D11_DEPTH_STENCILOP_DESC Result{};
					Result.StencilFailOp = static_cast<D3D11_STENCIL_OP>(
						Source.iStencilFailOp);
					Result.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(
						Source.iStencilDepthFailOp);
					Result.StencilPassOp = static_cast<D3D11_STENCIL_OP>(
						Source.iStencilPassOp);
					Result.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(
						Source.iStencilFunc);
					return Result;
				};
				const EFFECT_RUNTIME_PROGRAM_D3D_DEPTH_STENCIL& Source =
					*Descriptor.DepthStencil;
				D3D11_DEPTH_STENCIL_DESC StagedDescriptor{};
				StagedDescriptor.DepthEnable = Source.bDepthEnable;
				StagedDescriptor.DepthWriteMask =
					static_cast<D3D11_DEPTH_WRITE_MASK>(Source.iDepthWriteMask);
				StagedDescriptor.DepthFunc =
					static_cast<D3D11_COMPARISON_FUNC>(Source.iDepthFunc);
				StagedDescriptor.StencilEnable = Source.bStencilEnable;
				StagedDescriptor.StencilReadMask =
					static_cast<uint8_t>(Source.iStencilReadMask);
				StagedDescriptor.StencilWriteMask =
					static_cast<uint8_t>(Source.iStencilWriteMask);
				StagedDescriptor.FrontFace = ConvertFace(Source.FrontFace);
				StagedDescriptor.BackFace = ConvertFace(Source.BackFace);
				if (FAILED(m_pDevice->CreateDepthStencilState(
					&StagedDescriptor, &Resource.pDepthStencilState)))
				{
					strOutError =
						"Reconstructed diagnostic policy depth-state creation failed.";
					return false;
				}
				Resource.DepthStencilDescriptor = StagedDescriptor;
				Resource.bHasDepthStencilDescriptor = true;
				return true;
			}
			strOutError =
				"Reconstructed diagnostic render policy kind changed.";
			return false;
		}
		const auto Iterator = Authority->RenderStateDescriptorsById.find(
			Binding.SidecarDecision->strId);
		if (Iterator == Authority->RenderStateDescriptorsById.end() ||
			Iterator->second.strRowSha256 !=
				Binding.SidecarDecision->strRowSha256 ||
			Iterator->second.eKind != eKind ||
			Iterator->second.strRenderBindingId !=
				Binding.ProgramBinding.strId ||
			Iterator->second.strRenderBindingRowSha256 !=
				Binding.ProgramBinding.strRowSha256)
		{
			strOutError =
				"Reconstructed diagnostic render-state authority changed.";
			return false;
		}
		const EFFECT_RECONSTRUCTED_RENDER_STATE_DESCRIPTOR& Descriptor =
			Iterator->second;
		switch (eKind)
		{
		case EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::BLEND:
			if (FAILED(m_pDevice->CreateBlendState(
				&Descriptor.BlendDescriptor, &Resource.pBlendState)))
			{
				strOutError =
					"Reconstructed diagnostic blend-state creation failed.";
				return false;
			}
			Resource.BlendDescriptor = Descriptor.BlendDescriptor;
			Resource.bHasBlendDescriptor = true;
			return true;
		case EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::RASTERIZER:
			if (FAILED(m_pDevice->CreateRasterizerState(
				&Descriptor.RasterizerDescriptor,
				&Resource.pRasterizerState)))
			{
				strOutError =
					"Reconstructed diagnostic rasterizer-state creation failed.";
				return false;
			}
			Resource.RasterizerDescriptor = Descriptor.RasterizerDescriptor;
			Resource.bHasRasterizerDescriptor = true;
			return true;
		case EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::DEPTH_STENCIL:
			if (FAILED(m_pDevice->CreateDepthStencilState(
				&Descriptor.DepthStencilDescriptor,
				&Resource.pDepthStencilState)))
			{
				strOutError =
					"Reconstructed diagnostic depth-state creation failed.";
				return false;
			}
			Resource.DepthStencilDescriptor = Descriptor.DepthStencilDescriptor;
			Resource.bHasDepthStencilDescriptor = true;
			return true;
		case EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::END:
		default:
			strOutError = "Reconstructed diagnostic render-state kind is invalid.";
			return false;
		}
	};

	for (const EFFECT_RECONSTRUCTED_SELECTED_PACKET& Packet :
		pFrame->Get_Packets())
	{
		const uint32_t iSelection = Packet.Get_SelectionIndex();
		if (iSelection >= SelectionSeen.size() || SelectionSeen[iSelection])
		{
			strOutError =
				"Reconstructed diagnostic packet selection is duplicated.";
			return false;
		}
		SelectionSeen[iSelection] = true;
		const EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& Selection =
			pFrame->Get_Preparation()->Get_Request().Emitters[iSelection];
		if (Selection.eKind != Packet.Get_Kind())
		{
			strOutError = "Reconstructed diagnostic packet kind changed.";
			return false;
		}
		RECONSTRUCTED_DIAGNOSTIC_COMPOSITE::GPU_RESOURCE& Resource =
			Staged->Resources[iSelection];

		for (size_t iLane = 0u; iLane < Selection.Material.TextureLanes.size();
			++iLane)
		{
			const EFFECT_RECONSTRUCTED_SELECTED_TEXTURE_LANE& Lane =
				Selection.Material.TextureLanes[iLane];
			const auto BindingIterator = Authority->TextureBindingsById.find(
				Lane.SidecarTextureBinding.strId);
			const auto TextureIterator = Authority->TextureResourcesById.find(
				Lane.SidecarTextureResource.strId);
			if (BindingIterator == Authority->TextureBindingsById.end() ||
				TextureIterator == Authority->TextureResourcesById.end() ||
				BindingIterator->second.strRowSha256 !=
					Lane.SidecarTextureBinding.strRowSha256 ||
				TextureIterator->second.strRowSha256 !=
					Lane.SidecarTextureResource.strRowSha256 ||
				BindingIterator->second.strResourceAuthorityId !=
					Lane.SidecarTextureResource.strId ||
				BindingIterator->second.strRuntimeAssetId != Lane.strRuntimeAssetId ||
				TextureIterator->second.strRuntimeAssetId != Lane.strRuntimeAssetId ||
				BindingIterator->second.strActualDdsRawSha256 != Lane.strRawSha256 ||
				TextureIterator->second.strRawSha256 != Lane.strRawSha256)
			{
				strOutError =
					"Reconstructed diagnostic texture authority changed.";
				return false;
			}

			std::filesystem::path TexturePath;
			std::vector<uint8_t> TextureBytes;
			if (!Read_ReconstructedAssetBytes(Lane.strRuntimeAssetId,
				BindingIterator->second.iActualDdsByteCount,
				Lane.strRawSha256, TexturePath, TextureBytes, strOutError))
			{
				return false;
			}
			const DirectX::DDS_LOADER_FLAGS Flags =
				BindingIterator->second.ActualDdsSrv.strColorSpace == "SRGB" ?
					DirectX::DDS_LOADER_FORCE_SRGB :
					DirectX::DDS_LOADER_IGNORE_SRGB;
			ComPtr<ID3D11ShaderResourceView> Texture;
			if (FAILED(DirectX::CreateDDSTextureFromMemoryEx(
				m_pDevice.Get(), TextureBytes.data(), TextureBytes.size(), 0u,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0u, 0u,
				Flags, nullptr, &Texture)))
			{
				strOutError = "Reconstructed diagnostic DDS upload failed: " +
					Lane.strRuntimeAssetId;
				return false;
			}
			D3D11_SHADER_RESOURCE_VIEW_DESC SrvDescriptor{};
			Texture->GetDesc(&SrvDescriptor);
			const EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY& ExpectedSrv =
				BindingIterator->second.ActualDdsSrv;
			if (SrvDescriptor.Format != ExpectedSrv.eFormat ||
				SrvDescriptor.ViewDimension != ExpectedSrv.eViewDimension ||
				SrvDescriptor.Texture2D.MostDetailedMip !=
					ExpectedSrv.iMostDetailedMip ||
				SrvDescriptor.Texture2D.MipLevels != ExpectedSrv.iMipLevels)
			{
				strOutError =
					"Reconstructed diagnostic DDS SRV descriptor changed.";
				return false;
			}
			ComPtr<ID3D11SamplerState> Sampler;
			D3D11_SAMPLER_DESC RuntimeSamplerAuthority{};
			if (!Materialize_RuntimeSamplerDescriptor(
				BindingIterator->second.SamplerDescriptor,
				RuntimeSamplerAuthority))
			{
				strOutError =
					"Reconstructed diagnostic sampler descriptor is invalid.";
				return false;
			}
			if (FAILED(m_pDevice->CreateSamplerState(
				&RuntimeSamplerAuthority, &Sampler)))
			{
				strOutError =
					"Reconstructed diagnostic sampler-state creation failed.";
				return false;
			}
			D3D11_SAMPLER_DESC SamplerDescriptor{};
			Sampler->GetDesc(&SamplerDescriptor);
			if (!Same_RuntimeSamplerReadbackDescriptor(SamplerDescriptor,
				RuntimeSamplerAuthority,
				BindingIterator->second.SamplerDescriptor))
			{
				strOutError =
					"Reconstructed diagnostic sampler descriptor changed.";
				return false;
			}
			Resource.Textures[iLane] = std::move(Texture);
			Resource.Samplers[iLane] = std::move(Sampler);
			Resource.SamplerDescriptors[iLane] = SamplerDescriptor;
		}
		if (!Same_SamplerDescriptor(Resource.SamplerDescriptors[0u],
			Resource.SamplerDescriptors[1u]))
		{
			strOutError =
				"Reconstructed diagnostic texture lanes require different samplers.";
			return false;
		}

		if (!StageState(Selection.Material.BlendState,
				EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::BLEND, Resource) ||
			!StageState(Selection.Material.RasterizerState,
				EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::RASTERIZER, Resource) ||
			!StageState(Selection.Material.DepthStencilState,
				EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::DEPTH_STENCIL, Resource))
		{
			return false;
		}

		if (EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH ==
			Selection.eKind)
		{
			if (!Selection.Geometry.has_value() ||
				!Packet.Get_Values().vMeshDimensionlessScaleXzy.has_value())
			{
				strOutError =
					"Reconstructed diagnostic Mesh geometry is unavailable.";
				return false;
			}
			const EFFECT_RECONSTRUCTED_SELECTED_GEOMETRY_BINDING& Geometry =
				*Selection.Geometry;
			std::filesystem::path ModelPath;
			std::vector<uint8_t> ModelBytes;
			if (!Read_ReconstructedAssetBytes(Geometry.strRuntimeAssetId,
				Geometry.iCandidateResourceByteSize,
				Geometry.strCandidateResourceSha256,
				ModelPath, ModelBytes, strOutError))
			{
				return false;
			}
			const f32_t fPreScale = static_cast<f32_t>(Geometry.fGeometryPreScale);
			const std::string ModelPathString = ModelPath.string();
			unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
				m_pDevice, m_pContext, MODEL::NONANIM, ModelPathString.c_str(),
				XMMatrixScaling(fPreScale, fPreScale, fPreScale), true);
			if (nullptr == Model || Model->Get_NumMeshes() != Geometry.iSubmeshCount ||
				Sha256Hex(Model->Get_GeometryPayloadSha256()) !=
					Geometry.strPayloadSha256 ||
				Sha256Hex(Model->Get_GeometryMetadataIdentitySha256()) !=
					Geometry.strMetadataIdentitySha256)
			{
				strOutError =
					"Reconstructed diagnostic CModel identity changed.";
				return false;
			}
			Resource.pModel = std::move(Model);
		}
		else if (EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE ==
			Selection.eKind)
		{
			if (!Selection.SpriteSink.has_value() ||
				!Packet.Get_Values().vSpriteSignedWorldSizeXzy.has_value())
			{
				strOutError =
					"Reconstructed diagnostic Sprite sink is unavailable.";
				return false;
			}
		}
		else
		{
			strOutError = "Reconstructed diagnostic packet kind is unsupported.";
			return false;
		}

		D3D11_QUERY_DESC QueryDescriptor{};
		QueryDescriptor.Query = D3D11_QUERY_PIPELINE_STATISTICS;
		if (FAILED(m_pDevice->CreateQuery(
			&QueryDescriptor, &Resource.pPipelineStatisticsQuery)))
		{
			strOutError =
				"Reconstructed diagnostic pipeline-statistics query failed.";
			return false;
		}
	}

	unique_ptr<Engine::CVIBuffer_ParticleRect> StagedParticleBuffer;
	if (nullptr == m_pParticleBuffer)
	{
		StagedParticleBuffer = Engine::CVIBuffer_ParticleRect::Create(
			m_pDevice, m_pContext, 1u);
		if (nullptr == StagedParticleBuffer)
		{
			strOutError =
				"Reconstructed diagnostic particle instance buffer failed.";
			return false;
		}
	}
	if (nullptr != StagedParticleBuffer)
		m_pParticleBuffer = std::move(StagedParticleBuffer);
	m_BloomIntensityOverride.reset();
	m_Document = {};
	m_pPreparedDocument.reset();
	m_ModelCueResources.clear();
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_pReconstructedDiagnostic = std::move(Staged);
	m_bReconstructedSourceRuntimeActive = false;
	m_bSourceVisualProgramActive = false;
	Reset_PreviewSubmissionIsolation();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus =
		"Reconstructed diagnostic GPU resources committed; waiting for draw.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Select_OccurrenceElement(
    const std::string_view elementId, std::string& status)
{
    if (!m_pPreparedDocument || elementId.empty())
    { status = "An occurrence element requires a prepared document and stable ID."; return false; }
    const auto& elements = Get_StagedDocument().Elements;
    const auto found = std::find_if(elements.begin(), elements.end(),
        [elementId](const auto& element) { return element.strElementId == elementId; });
    if (found == elements.end() || !found->bVisible)
    { status = "The selected occurrence element is absent or disabled."; return false; }
    EFFECT_PREVIEW_SUBMISSION_ISOLATION selection;
    selection.eKind = EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET;
    selection.ElementIds.emplace_back(elementId);
    m_PreviewSubmissionIsolation = std::move(selection);
    m_bOccurrenceElementSelected = true;
    status.clear();
    return true;
}

bool_t Client::CEffectDocumentRenderer::Set_SubmissionElementSet(std::vector<std::string> elementIds, std::string& error)
{
    if (!m_pPreparedDocument || m_pReconstructedDiagnostic)
    { error = "Element submission visibility requires a prepared document."; return false; }
    const auto& elements = Get_StagedDocument().Elements;
    std::sort(elementIds.begin(), elementIds.end());
    if (std::adjacent_find(elementIds.begin(), elementIds.end()) != elementIds.end())
    { error = "Element submission visibility contains duplicate IDs."; return false; }
    for (const auto& id : elementIds)
        if (id.empty() || std::none_of(elements.begin(), elements.end(),
            [&id](const auto& element) { return element.strElementId == id; }))
        { error = "Element submission visibility names a missing row."; return false; }
    EFFECT_PREVIEW_SUBMISSION_ISOLATION selection;
    selection.eKind = EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET;
    selection.ElementIds = std::move(elementIds); // Empty means submit no Elements, never ALL.
    m_PreviewSubmissionIsolation = std::move(selection);
    error.clear(); return true;
}

bool_t Client::CEffectDocumentRenderer::Set_PreviewSubmissionIsolation(
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
	std::string& strOutError)
{
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Program =
		m_ReconstructedRuntimeBoundary.Get_Program();
	std::string DocumentError;
	const bool_t bReconstructedArtist =
		m_bReconstructedSourceRuntimeActive && nullptr != Program;
	const bool_t bGenericVisual = m_bSourceVisualProgramActive &&
		nullptr != m_pPreparedDocument &&
		nullptr != m_pPreparedDocument->pVisualProgramProjection;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	/* The focused GPU harness stages the ordinary product document directly so
	   Binding 0 and Binding 1 exercise the same renderer path.  Only the
	   test-only CEffectObject entry point can reach this allowance; the product
	   entry point still requires an admitted source visual program. */
	const bool_t bStagedOrdinaryTest = nullptr != m_pPreparedDocument &&
		!m_bReconstructedSourceRuntimeActive && !m_bSourceVisualProgramActive;
#else
	constexpr bool_t bStagedOrdinaryTest = false;
#endif
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	if ((!bReconstructedArtist && !bGenericVisual && !bStagedOrdinaryTest) ||
		(bReconstructedArtist &&
		 (Program->iSkillId != 31470u || Program->Admission.bRuntimeExecution ||
		  Program->Admission.bProduct ||
		  !CEffectDocumentCodec::Validate_Artist31470ReconstructedRuntimeDrawable(
			  Document, DocumentError))))
	{
		strOutError =
			"Preview submission isolation requires an admitted source visual program.";
		if (!DocumentError.empty())
			strOutError += " " + DocumentError;
		return false;
	}

	std::array<uint32_t,
		static_cast<size_t>(EFFECT_GPU_RENDER_FAMILY::END)> VisibleFamilies{};
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const EFFECT_GPU_RENDER_FAMILY eFamily =
			Resolve_GpuRenderFamily(Element);
		if (Element.bVisible)
		{
			if (EFFECT_GPU_RENDER_FAMILY::END == eFamily)
			{
				if (bReconstructedArtist)
				{
					strOutError =
						"Artist Core preview contains a visible Light/Post row.";
					return false;
				}
				continue;
			}
			++VisibleFamilies[static_cast<size_t>(eFamily)];
		}
	}
	if (bReconstructedArtist &&
		(VisibleFamilies[static_cast<size_t>(
			EFFECT_GPU_RENDER_FAMILY::MESH)] != 13u ||
		 VisibleFamilies[static_cast<size_t>(
			EFFECT_GPU_RENDER_FAMILY::SPRITE)] != 16u ||
		 VisibleFamilies[static_cast<size_t>(
			EFFECT_GPU_RENDER_FAMILY::DECAL)] != 3u ||
		 VisibleFamilies[static_cast<size_t>(
			EFFECT_GPU_RENDER_FAMILY::RIBBON)] != 1u))
	{
		strOutError =
			"Preview submission isolation requires the exact visible Core33 scope.";
		return false;
	}

	EFFECT_PREVIEW_SUBMISSION_ISOLATION Staged = Isolation;
	switch (Isolation.eKind)
	{
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL:
		if (Isolation.eFamily != EFFECT_GPU_RENDER_FAMILY::END ||
			!Isolation.strElementId.empty() || !Isolation.ElementIds.empty())
		{
			strOutError =
				"ALL preview isolation must not carry a family or occurrence ID.";
			return false;
		}
		break;
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::FAMILY:
		switch (Isolation.eFamily)
		{
		case EFFECT_GPU_RENDER_FAMILY::MESH:
		case EFFECT_GPU_RENDER_FAMILY::SPRITE:
		case EFFECT_GPU_RENDER_FAMILY::DECAL:
		case EFFECT_GPU_RENDER_FAMILY::RIBBON:
			break;
		case EFFECT_GPU_RENDER_FAMILY::END:
		default:
			strOutError =
				"FAMILY preview isolation requires one explicit Core family.";
			return false;
		}
		if (!Isolation.strElementId.empty() ||
			!Isolation.ElementIds.empty() ||
			0u == VisibleFamilies[static_cast<size_t>(Isolation.eFamily)])
		{
			strOutError =
				"FAMILY preview isolation carries an occurrence ID or empty family.";
			return false;
		}
		break;
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::OCCURRENCE:
	{
		if (Isolation.strElementId.empty() || !Isolation.ElementIds.empty())
		{
			strOutError =
				"OCCURRENCE preview isolation requires one stable Element ID.";
			return false;
		}
		const EFFECT_ELEMENT_DESC* pSelected = nullptr;
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.strElementId != Isolation.strElementId)
				continue;
			if (nullptr != pSelected)
			{
				strOutError =
					"OCCURRENCE preview isolation ID is duplicated.";
				return false;
			}
			pSelected = &Element;
		}
		const EFFECT_GPU_RENDER_FAMILY eSelectedFamily =
			nullptr == pSelected ? EFFECT_GPU_RENDER_FAMILY::END :
				Resolve_GpuRenderFamily(*pSelected);
		if (nullptr == pSelected || !pSelected->bVisible ||
			EFFECT_GPU_RENDER_FAMILY::END == eSelectedFamily ||
			(Isolation.eFamily != EFFECT_GPU_RENDER_FAMILY::END &&
			 Isolation.eFamily != eSelectedFamily))
		{
			strOutError =
				"OCCURRENCE preview isolation must name one visible Core GPU row.";
			return false;
		}
		Staged.eFamily = eSelectedFamily;
		break;
	}
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET:
	{
		if (Isolation.eFamily != EFFECT_GPU_RENDER_FAMILY::END ||
			!Isolation.strElementId.empty() || Isolation.ElementIds.empty())
		{
			strOutError =
				"ELEMENT_SET preview isolation requires only stable Element IDs.";
			return false;
		}
		std::unordered_set<std::string> Seen;
		for (const std::string& ElementId : Isolation.ElementIds)
		{
			if (ElementId.empty() || !Seen.emplace(ElementId).second)
			{
				strOutError =
					"ELEMENT_SET preview isolation ID is empty or duplicated.";
				return false;
			}
			const auto Element = std::find_if(Document.Elements.begin(),
				Document.Elements.end(), [&ElementId](const auto& Candidate)
				{
					return Candidate.strElementId == ElementId;
				});
			if (Element == Document.Elements.end() || !Element->bVisible)
			{
				strOutError =
					"ELEMENT_SET preview isolation names a missing or hidden row.";
				return false;
			}
		}
		std::sort(Staged.ElementIds.begin(), Staged.ElementIds.end());
		break;
	}
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::END:
	default:
		strOutError = "Preview submission isolation kind is invalid.";
		return false;
	}

	m_PreviewSubmissionIsolation = std::move(Staged);
	m_strStatus = "Source visual-program preview submission isolation staged.";
	strOutError.clear();
	return true;
}

void Client::CEffectDocumentRenderer::Reset_PreviewSubmissionIsolation()
{
	m_PreviewSubmissionIsolation = {};
	m_bOccurrenceElementSelected = false;
}

bool_t Client::CEffectDocumentRenderer::Should_SubmitPreviewOccurrence(
	const EFFECT_ELEMENT_DESC& Element,
	const EFFECT_GPU_RENDER_FAMILY eFamily) const
{
	switch (m_PreviewSubmissionIsolation.eKind)
	{
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL:
		return true;
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::FAMILY:
		return eFamily == m_PreviewSubmissionIsolation.eFamily;
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::OCCURRENCE:
		return Element.strElementId ==
			m_PreviewSubmissionIsolation.strElementId;
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET:
		return std::binary_search(m_PreviewSubmissionIsolation.ElementIds.begin(),
			m_PreviewSubmissionIsolation.ElementIds.end(), Element.strElementId);
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::END:
	default:
		return false;
	}
}

bool_t Client::CEffectDocumentRenderer::Set_BloomIntensity(const f32_t value, std::string& error)
{
    if (!Is_ValidEffectBloomIntensity(value))
    { error = "Effect bloomIntensity must be finite and between 0 and 16."; return false; }
    m_BloomIntensityOverride = value;
    error.clear();
    return true;
}

f32_t Client::CEffectDocumentRenderer::Get_BloomIntensity() const
{
    return m_BloomIntensityOverride.value_or(Get_StagedDocument().fBloomIntensity);
}

HRESULT Client::CEffectDocumentRenderer::Bind_BloomInputs(const std::shared_ptr<Engine::CShader>& shader)
{
    if (!shader) return E_POINTER;
    const f32_t intensity = Get_BloomIntensity();
    const auto quality = CGameInstance::Get().Get_RenderQualitySettings();
    if (FAILED(shader->Bind_RawValue("g_fEffectBloomThreshold", &quality.fBloomThreshold, sizeof(quality.fBloomThreshold))) ||
        FAILED(shader->Bind_RawValue("g_fEffectBloomSoftKnee", &quality.fBloomSoftKnee, sizeof(quality.fBloomSoftKnee))) ||
        FAILED(shader->Bind_RawValue("g_fEffectBloomIntensity", &intensity, sizeof(intensity))))
        return Fail_RenderOperation("Effect bloom shader binding failed.", E_FAIL);
    return S_OK;
}

void Client::CEffectDocumentRenderer::Clear()
{
    m_BloomIntensityOverride.reset();
    m_pStartingSceneCapture.Reset();
    m_pStartingSceneBloomCapture.Reset();
	m_Document = {};
	m_pPreparedDocument.reset();
	m_pReconstructedDiagnostic.reset();
	m_ReconstructedRuntimeBoundary.Clear();
	m_bReconstructedSourceRuntimeActive = false;
	m_bSourceVisualProgramActive = false;
	Reset_PreviewSubmissionIsolation();
	m_ModelCueResources.clear();
	m_LastRenderSubmissionStats = {};
	m_bWorldMarkSubmissionPending = false;
	m_iWorldMarkSubmissionSerial = 0u;
	m_iStatusEvaluated = (std::numeric_limits<uint64_t>::max)();
	m_iStatusActive = (std::numeric_limits<uint64_t>::max)();
	m_iStatusSubmitted = (std::numeric_limits<uint64_t>::max)();
	m_iStatusSuppressed = (std::numeric_limits<uint64_t>::max)();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus = "No Effect Document staged.";
}
