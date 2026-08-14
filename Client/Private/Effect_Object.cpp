#include "Effect_Object.h"

#include "Effect_DocumentRenderer.h"
#include "Effect_LightPresentation.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Presentation_Manager.h"

#include <cmath>
#include <cstdio>

namespace
{
	struct CONFIGURED_PRESENTATION_COUNTS final
	{
		uint64_t iLights = 0u;
		uint64_t iScreenPosts = 0u;
	};

	CONFIGURED_PRESENTATION_COUNTS Count_ConfiguredPresentation(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		CONFIGURED_PRESENTATION_COUNTS Counts;
		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (!Element.bVisible)
				continue;
			if (Client::EFFECT_ELEMENT_KIND::LIGHT == Element.eKind &&
				Element.Detail.Light.bEnabled)
			{
				++Counts.iLights;
			}
			else if (Client::EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind &&
				Element.Detail.ScreenPost.bEnabled)
			{
				++Counts.iScreenPosts;
			}
		}
		return Counts;
	}

	void Record_LocalSubmissionResult(
		PRESENTATION_CHANNEL_SUBMISSION_STATS& Stats,
		const HRESULT hResult)
	{
		++Stats.iAttempted;
		if (S_OK == hResult)
			++Stats.iAccepted;
		else if (S_FALSE == hResult)
			++Stats.iSuppressed;
		else
			++Stats.iFailed;
	}

	bool Is_CompleteLocalSubmission(
		const PRESENTATION_CHANNEL_SUBMISSION_STATS& Stats)
	{
		return Stats.iExpected <= Stats.iConfigured &&
			Stats.iAttempted == Stats.iExpected &&
			Stats.iAccepted + Stats.iSuppressed == Stats.iAttempted &&
			0u == Stats.iFailed;
	}

	std::string Format_Hresult(const HRESULT hResult)
	{
		char Buffer[16]{};
		std::snprintf(Buffer, sizeof(Buffer), "0x%08X",
			static_cast<unsigned int>(hResult));
		return Buffer;
	}
}

Client::CEffectObject::CEffectObject(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(pDevice, pContext),
	  m_pRenderer(make_unique<CEffectDocumentRenderer>(
		  std::move(pDevice), std::move(pContext)))
{
	XMStoreFloat4x4(&m_RootWorld, XMMatrixIdentity());
}

Client::CEffectObject::~CEffectObject() = default;

HRESULT Client::CEffectObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT Client::CEffectObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	if (FAILED(m_pRenderer->Initialize()))
		return E_FAIL;
	if (nullptr == pArg)
		return S_OK;
	const EFFECT_OBJECT_DESC& Desc =
		*static_cast<EFFECT_OBJECT_DESC*>(pArg);
	if (!std::isfinite(Desc.fPlaybackRate) ||
		Desc.fPlaybackRate <= 0.f || Desc.fPlaybackRate > 16.f)
	{
		return E_FAIL;
	}
	m_RootWorld = Desc.RootWorld;
	m_bPlaying = Desc.bAutoPlay;
	m_fPlaybackRate = Desc.fPlaybackRate;
	if (nullptr != Desc.pVisualProgramProjection)
	{
		if (nullptr != Desc.pDocument || nullptr == Desc.pPreparedResources)
		{
			return E_FAIL;
		}
		const bool_t bStaged =
			nullptr == Desc.pReconstructedRuntimePreparation ?
			Stage_PrevalidatedVisualProgramDocument(
				Desc.pVisualProgramProjection, Desc.pPreparedResources,
				m_strStatus) :
			Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
				Desc.pVisualProgramProjection, Desc.pPreparedResources,
				Desc.pReconstructedRuntimePreparation, m_strStatus);
		if (!bStaged)
			return E_FAIL;
		return S_OK;
	}
	if (nullptr != Desc.pReconstructedRuntimePreparation)
	{
		if (!Stage_ReconstructedRuntimeProgram(Desc, m_strStatus))
		{
			return E_FAIL;
		}
		return S_OK;
	}
	if (nullptr != Desc.pDocument && nullptr != Desc.pPreparedResources &&
		!Stage_PreparedDocument(*Desc.pDocument,
			Desc.pPreparedResources, m_strStatus))
	{
		return E_FAIL;
	}
	if (nullptr != Desc.pDocument && nullptr == Desc.pPreparedResources &&
		(Desc.bRequirePreparedResources ||
			!Stage_Document(*Desc.pDocument, m_strStatus)))
	{
		return E_FAIL;
	}
	if (nullptr == Desc.pDocument && Desc.bRequirePreparedResources)
		return E_FAIL;
	return S_OK;
}

bool_t Client::CEffectObject::Stage_Document(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	CEffectPlayback StagedPlayback;
	if (!StagedPlayback.Stage_Document(Document, strOutError))
		return false;
	if (!m_pRenderer->Stage_Document(Document, strOutError))
		return false;
	m_Playback = std::move(StagedPlayback);
	m_ReconstructedRuntimeBoundary.Clear();
	m_pReconstructedDiagnosticFrame.reset();
	m_eReconstructedDiagnosticSolo = RECONSTRUCTED_DIAGNOSTIC_SOLO::END;
	m_bReconstructedDiagnosticActive = false;
	m_bSourceVisualProgramActive = false;
	m_bReconstructedSourceRuntimeActive = false;
	const CONFIGURED_PRESENTATION_COUNTS Counts =
		Count_ConfiguredPresentation(Document);
	m_iConfiguredLightCount = Counts.iLights;
	m_iConfiguredScreenPostCount = Counts.iScreenPosts;
	m_LastPresentationSubmissionStats = {};
	m_Playback.Seek(0.f, m_RootWorld);
	Reset_RenderFailureIsolation();
	m_strStatus = "Effect Document staged.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectObject::Stage_PreparedDocument(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
		pPreparedResources,
	std::string& strOutError)
{
	CEffectPlayback StagedPlayback;
	const std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
		pPlaybackResources =
			CEffectDocumentRenderer::Get_PlaybackResources(pPreparedResources);
	if (!StagedPlayback.Stage_PrevalidatedDocument(
		Document, pPlaybackResources, strOutError))
		return false;
	if (!m_pRenderer->Stage_Prepared(
		Document, std::move(pPreparedResources), strOutError))
	{
		return false;
	}
	m_Playback = std::move(StagedPlayback);
	m_ReconstructedRuntimeBoundary.Clear();
	m_pReconstructedDiagnosticFrame.reset();
	m_eReconstructedDiagnosticSolo = RECONSTRUCTED_DIAGNOSTIC_SOLO::END;
	m_bReconstructedDiagnosticActive = false;
	m_bSourceVisualProgramActive = false;
	m_bReconstructedSourceRuntimeActive = false;
	const CONFIGURED_PRESENTATION_COUNTS Counts =
		Count_ConfiguredPresentation(Document);
	m_iConfiguredLightCount = Counts.iLights;
	m_iConfiguredScreenPostCount = Counts.iScreenPosts;
	m_LastPresentationSubmissionStats = {};
	Reset_RenderFailureIsolation();
	m_strStatus = "Prepared Effect Document staged.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectObject::Stage_PrevalidatedVisualProgramDocument(
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pProjection,
	std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
		pPreparedResources,
	std::string& strOutError)
{
	if (nullptr == pProjection || !pProjection->Is_Valid() ||
		nullptr == pPreparedResources)
	{
		strOutError =
			"Admitted source visual-program object inputs are missing.";
		return false;
	}
	const EFFECT_DOCUMENT_DESC& Document = pProjection->Get_Document();
	const std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
		pPlaybackResources =
			CEffectDocumentRenderer::Get_PlaybackResources(pPreparedResources);
	CEffectPlayback StagedPlayback;
	if (!StagedPlayback.Stage_PrevalidatedVisualProgramDocument(
			pProjection, pPlaybackResources, strOutError))
	{
		return false;
	}
	if (!m_pRenderer->Stage_PrevalidatedVisualProgramDocument(
			pProjection, std::move(pPreparedResources), strOutError))
	{
		return false;
	}
	const bool_t bSourceVisualProgramActive =
		StagedPlayback.Is_SourceVisualProgramActive();

	m_Playback = std::move(StagedPlayback);
	m_ReconstructedRuntimeBoundary.Clear();
	m_pReconstructedDiagnosticFrame.reset();
	m_eReconstructedDiagnosticSolo = RECONSTRUCTED_DIAGNOSTIC_SOLO::END;
	m_bReconstructedDiagnosticActive = false;
	m_bSourceVisualProgramActive = bSourceVisualProgramActive;
	m_bReconstructedSourceRuntimeActive = false;
	const CONFIGURED_PRESENTATION_COUNTS Counts =
		Count_ConfiguredPresentation(Document);
	m_iConfiguredLightCount = Counts.iLights;
	m_iConfiguredScreenPostCount = Counts.iScreenPosts;
	m_LastPresentationSubmissionStats = {};
	Reset_RenderFailureIsolation();
	m_strStatus = "Admitted source visual-program Effect Document staged.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectObject::Stage_ReconstructedRuntimeProgram(
	const EFFECT_OBJECT_DESC& Desc,
	std::string& strOutError)
{
	if (nullptr == Desc.pReconstructedRuntimePreparation ||
		nullptr != Desc.pDocument || nullptr != Desc.pPreparedResources ||
		nullptr != Desc.pVisualProgramProjection ||
		!std::isfinite(Desc.fPlaybackRate) ||
		Desc.fPlaybackRate <= 0.f || Desc.fPlaybackRate > 16.f)
	{
		strOutError =
			"Reconstructed Effect inspection descriptor is not exclusive.";
		return false;
	}
	if (!Stage_ReconstructedRuntimeEntry(
		Desc.pReconstructedRuntimePreparation, strOutError))
	{
		return false;
	}
	m_RootWorld = Desc.RootWorld;
	m_fPlaybackRate = Desc.fPlaybackRate;
	return true;
}

bool_t Client::CEffectObject::Stage_ReconstructedRuntimeEntry(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	std::string& strOutError)
{
	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::OBJECT, strOutError))
	{
		return false;
	}
	CEffectPlayback StagedPlayback;
	if (!StagedPlayback.Stage_ReconstructedRuntimeProgram(
		pPreparation, strOutError))
	{
		return false;
	}
	if (!m_pRenderer->Stage_ReconstructedRuntimeProgram(
		pPreparation, strOutError))
	{
		return false;
	}
	m_Playback = std::move(StagedPlayback);
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_pReconstructedDiagnosticFrame.reset();
	m_eReconstructedDiagnosticSolo = RECONSTRUCTED_DIAGNOSTIC_SOLO::END;
	m_bReconstructedDiagnosticActive = false;
	m_bSourceVisualProgramActive = false;
	m_bReconstructedSourceRuntimeActive = false;
	m_iConfiguredLightCount = 0u;
	m_iConfiguredScreenPostCount = 0u;
	m_LastPresentationSubmissionStats = {};
	m_bPlaying = false;
	m_bVisible = false;
	Reset_RenderFailureIsolation();
	m_strStatus =
		"Reconstructed Effect program prepared; Product execution remains blocked.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectObject::Stage_ReconstructedSourceRuntime(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
		pPreparedResources,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	std::string& strOutError)
{
	if (nullptr == pPreparedResources || nullptr == pPreparation ||
		nullptr == pPreparation->Get_Program() ||
		Document.strEffectAssetId !=
			pPreparation->Get_Program()->strRuntimeCatalogAssetId)
	{
		strOutError =
			"Reconstructed source runtime object identity is invalid.";
		return false;
	}
	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::OBJECT, strOutError))
	{
		return false;
	}
	const std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
		pPlaybackResources =
			CEffectDocumentRenderer::Get_PlaybackResources(pPreparedResources);
	CEffectPlayback StagedPlayback;
	if (!StagedPlayback.Stage_ReconstructedSourceRuntime(Document,
		pPlaybackResources, pPreparation, strOutError))
	{
		return false;
	}
	if (!m_pRenderer->Stage_ReconstructedSourceRuntime(Document,
		std::move(pPreparedResources), pPreparation, strOutError))
	{
		return false;
	}
	if (StagedPlayback.Get_ReconstructedRuntimePreparation().get() !=
			pPreparation.get() ||
		m_pRenderer->Get_ReconstructedRuntimePreparation().get() !=
			pPreparation.get())
	{
		strOutError =
			"Reconstructed source runtime seam identity diverged.";
		return false;
	}
	m_Playback = std::move(StagedPlayback);
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_pReconstructedDiagnosticFrame.reset();
	m_eReconstructedDiagnosticSolo = RECONSTRUCTED_DIAGNOSTIC_SOLO::END;
	m_bReconstructedDiagnosticActive = false;
	m_bSourceVisualProgramActive = true;
	m_bReconstructedSourceRuntimeActive = true;
	const CONFIGURED_PRESENTATION_COUNTS Counts =
		Count_ConfiguredPresentation(Document);
	m_iConfiguredLightCount = Counts.iLights;
	m_iConfiguredScreenPostCount = Counts.iScreenPosts;
	m_LastPresentationSubmissionStats = {};
	m_bVisible = true;
	m_Playback.Seek(0.f, m_RootWorld);
	Reset_RenderFailureIsolation();
	m_strStatus =
		"Artist F core renderer preview staged; Product remains blocked.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectObject::
	Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPreparedResources,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError)
{
	if (nullptr == pProjection || !pProjection->Is_Valid() ||
		pProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
		nullptr == pPreparedResources || nullptr == pPreparation ||
		nullptr == pPreparation->Get_Program() ||
		pProjection->Get_EffectAssetId() !=
			pPreparation->Get_Program()->strRuntimeCatalogAssetId)
	{
		strOutError =
			"Reconstructed source runtime visual adapter object identity is invalid.";
		return false;
	}

	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(pPreparation,
			EFFECT_RECONSTRUCTED_RUNTIME_SEAM::OBJECT, strOutError))
	{
		return false;
	}
	const std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
		pPlaybackResources =
			CEffectDocumentRenderer::Get_PlaybackResources(pPreparedResources);
	CEffectPlayback StagedPlayback;
	if (!StagedPlayback.Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
			pProjection, pPlaybackResources, pPreparation, strOutError) ||
		StagedPlayback.Get_ReconstructedRuntimePreparation().get() !=
			pPreparation.get() ||
		StagedPlayback.Get_SourceVisualProgramProjection().get() !=
			pProjection.get())
	{
		if (strOutError.empty())
		{
			strOutError =
				"Reconstructed source runtime visual adapter playback seam diverged.";
		}
		return false;
	}
	if (!m_pRenderer->Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
			pProjection, std::move(pPreparedResources), pPreparation, strOutError))
	{
		return false;
	}

	m_Playback = std::move(StagedPlayback);
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_pReconstructedDiagnosticFrame.reset();
	m_eReconstructedDiagnosticSolo = RECONSTRUCTED_DIAGNOSTIC_SOLO::END;
	m_bReconstructedDiagnosticActive = false;
	m_bSourceVisualProgramActive = true;
	m_bReconstructedSourceRuntimeActive = true;
	const CONFIGURED_PRESENTATION_COUNTS Counts =
		Count_ConfiguredPresentation(pProjection->Get_Document());
	m_iConfiguredLightCount = Counts.iLights;
	m_iConfiguredScreenPostCount = Counts.iScreenPosts;
	m_LastPresentationSubmissionStats = {};
	m_bVisible = true;
	m_Playback.Seek(0.f, m_RootWorld);
	Reset_RenderFailureIsolation();
	m_strStatus =
		"Reconstructed source runtime staged with immutable visual adapter.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectObject::Stage_ReconstructedDiagnostic(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME> pFrame,
	const RECONSTRUCTED_DIAGNOSTIC_SOLO eSolo,
	std::string& strOutError)
{
	if (nullptr == pFrame ||
		(RECONSTRUCTED_DIAGNOSTIC_SOLO::MESH != eSolo &&
		 RECONSTRUCTED_DIAGNOSTIC_SOLO::SPRITE != eSolo))
	{
		strOutError =
			"Reconstructed diagnostic requires a frame and one valid solo kind.";
		return false;
	}

	const std::shared_ptr<const
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>
		pSelectedPreparation = pFrame->Get_Preparation();
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pRuntimePreparation = nullptr == pSelectedPreparation ? nullptr :
			pSelectedPreparation->Get_RuntimePreparation();
	if (nullptr == pSelectedPreparation || nullptr == pRuntimePreparation ||
		pSelectedPreparation->Get_CatalogEntry().get() !=
			pRuntimePreparation->Get_CatalogEntry().get() ||
		pSelectedPreparation->Get_Program().get() !=
			pRuntimePreparation->Get_Program().get() ||
		pSelectedPreparation->Get_RenderResourceAuthority().get() !=
			pRuntimePreparation->Get_RenderResourceAuthority().get())
	{
		strOutError =
			"Reconstructed diagnostic frame/runtime preparation identity mismatch.";
		return false;
	}

	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(pRuntimePreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::OBJECT, strOutError))
	{
		return false;
	}
	CEffectPlayback StagedPlayback;
	if (!StagedPlayback.Stage_ReconstructedRuntimeProgram(
		pRuntimePreparation, strOutError))
	{
		return false;
	}
	if (StagedPlayback.Get_ReconstructedRuntimePreparation().get() !=
		pRuntimePreparation.get())
	{
		strOutError =
			"Reconstructed diagnostic playback preparation identity mismatch.";
		return false;
	}
	if (!m_pRenderer->Stage_ReconstructedDiagnostic(pFrame, strOutError))
		return false;
	if (m_pRenderer->Get_ReconstructedRuntimePreparation().get() !=
		pRuntimePreparation.get())
	{
		strOutError =
			"Reconstructed diagnostic renderer preparation identity mismatch.";
		return false;
	}

	m_Playback = std::move(StagedPlayback);
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_pReconstructedDiagnosticFrame = std::move(pFrame);
	m_eReconstructedDiagnosticSolo = eSolo;
	m_bReconstructedDiagnosticActive = true;
	m_bSourceVisualProgramActive = false;
	m_bReconstructedSourceRuntimeActive = false;
	m_iConfiguredLightCount = 0u;
	m_iConfiguredScreenPostCount = 0u;
	m_LastPresentationSubmissionStats = {};
	m_bPlaying = false;
	m_bVisible = true;
	Reset_RenderFailureIsolation();
	m_strStatus =
		"Reconstructed selected diagnostic staged; Product execution remains blocked.";
	strOutError.clear();
	return true;
}

void Client::CEffectObject::Set_ReconstructedDiagnosticSolo(
	const RECONSTRUCTED_DIAGNOSTIC_SOLO eSolo)
{
	if (!m_bReconstructedDiagnosticActive ||
		(RECONSTRUCTED_DIAGNOSTIC_SOLO::MESH != eSolo &&
		 RECONSTRUCTED_DIAGNOSTIC_SOLO::SPRITE != eSolo))
	{
		return;
	}
	m_eReconstructedDiagnosticSolo = eSolo;
	m_strStatus = RECONSTRUCTED_DIAGNOSTIC_SOLO::MESH == eSolo ?
		"Reconstructed Mesh diagnostic solo active; Product execution remains blocked." :
		"Reconstructed Sprite diagnostic solo active; Product execution remains blocked.";
}

bool_t Client::CEffectObject::Set_PreviewSubmissionIsolation(
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
	std::string& strOutError)
{
	if ((!m_bReconstructedSourceRuntimeActive &&
		!m_bSourceVisualProgramActive) ||
		m_bReconstructedDiagnosticActive || nullptr == m_pRenderer)
	{
		strOutError =
			"Preview submission isolation requires an active admitted source visual program.";
		return false;
	}
	if (!m_pRenderer->Set_PreviewSubmissionIsolation(Isolation, strOutError))
		return false;
	m_strStatus = m_pRenderer->Get_Status();
	return true;
}

bool_t Client::CEffectObject::Set_PreviewElementIsolation(
	std::vector<std::string> ElementIds,
	std::string& strOutError)
{
	EFFECT_PREVIEW_SUBMISSION_ISOLATION Isolation;
	Isolation.eKind = EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET;
	Isolation.ElementIds = std::move(ElementIds);
	return Set_PreviewSubmissionIsolation(Isolation, strOutError);
}

void Client::CEffectObject::Reset_PreviewSubmissionIsolation()
{
	if (nullptr != m_pRenderer)
		m_pRenderer->Reset_PreviewSubmissionIsolation();
}

bool_t Client::CEffectObject::Should_SubmitPreviewElement(
	const EFFECT_ELEMENT_DESC* const pElement) const
{
	if (nullptr == m_pRenderer)
		return true;
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation =
		m_pRenderer->Get_PreviewSubmissionIsolation();
	return Isolation.eKind !=
			EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET ||
		(nullptr != pElement && std::binary_search(Isolation.ElementIds.begin(),
			Isolation.ElementIds.end(), pElement->strElementId));
}

void Client::CEffectObject::Set_RootWorld(const float4x4_t& RootWorld)
{
	m_RootWorld = RootWorld;
	if (!m_bReconstructedDiagnosticActive)
		m_Playback.Update(0.f, m_RootWorld);
}

void Client::CEffectObject::Set_SourceAnchorWorlds(
	const std::unordered_map<std::string, float4x4_t>& SourceAnchorWorlds)
{
	m_Playback.Set_SourceAnchorWorlds(SourceAnchorWorlds);
}

void Client::CEffectObject::Set_SourceAnchorWorlds(
	std::unordered_map<std::string, float4x4_t>&& SourceAnchorWorlds)
{
	m_Playback.Set_SourceAnchorWorlds(std::move(SourceAnchorWorlds));
}

void Client::CEffectObject::Set_SampleTime(const f32_t fSampleTimeSeconds)
{
	if (m_bReconstructedDiagnosticActive)
		return;
	m_bPlaying = false;
	m_Playback.Seek(fSampleTimeSeconds, m_RootWorld);
}

void Client::CEffectObject::Advance_Preview(const f32_t fTimeDelta)
{
	if (m_bReconstructedDiagnosticActive)
		return;
	m_Playback.Update((std::max)(0.f, fTimeDelta), m_RootWorld);
}

void Client::CEffectObject::Advance_Preview(
	const f32_t fTimeDelta,
	const float4x4_t& RootWorld)
{
	m_RootWorld = RootWorld;
	if (m_bReconstructedDiagnosticActive)
		return;
	m_Playback.Update((std::max)(0.f, fTimeDelta), m_RootWorld);
}

void Client::CEffectObject::Set_RootWorldForNextUpdate(
	const float4x4_t& RootWorld)
{
	m_RootWorld = RootWorld;
}

bool_t Client::CEffectObject::Set_SampleTimeWithTransformHistory(
	const f32_t fSampleTimeSeconds,
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
	std::string& strOutError)
{
	if (m_bReconstructedDiagnosticActive ||
		(!m_bReconstructedSourceRuntimeActive &&
		 !m_bSourceVisualProgramActive))
	{
		strOutError =
			"Effect object transform-history seek requires an admitted source visual runtime.";
		return false;
	}
	if (!m_Playback.Seek_WithTransformHistory(
			fSampleTimeSeconds, TransformProvider, strOutError))
	{
		return false;
	}
	m_RootWorld = m_Playback.Get_Frame().RootWorld;
	m_bPlaying = false;
	return true;
}

bool_t Client::CEffectObject::Advance_PreviewWithTransformHistory(
	const f32_t fTimeDelta,
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
	std::string& strOutError)
{
	if (m_bReconstructedDiagnosticActive ||
		(!m_bReconstructedSourceRuntimeActive &&
		 !m_bSourceVisualProgramActive))
	{
		strOutError =
			"Effect object transform-history update requires an admitted source visual runtime.";
		return false;
	}
	if (!m_Playback.Update_WithTransformHistory(
			fTimeDelta, TransformProvider, strOutError))
	{
		return false;
	}
	m_RootWorld = m_Playback.Get_Frame().RootWorld;
	return true;
}

void Client::CEffectObject::Reset()
{
	if (m_bReconstructedDiagnosticActive)
		return;
	m_Playback.Seek(0.f, m_RootWorld);
}

void Client::CEffectObject::Update(const f32_t fTimeDelta)
{
	if (m_bRenderFailureIsolated)
		return;
	if (m_bReconstructedDiagnosticActive)
		return;
	std::string GateStatus;
	if (!m_bReconstructedSourceRuntimeActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Execution(GateStatus))
	{
		m_strStatus = std::move(GateStatus);
		return;
	}
	if (m_bPlaying)
		m_Playback.Update(fTimeDelta * m_fPlaybackRate, m_RootWorld);
}

void Client::CEffectObject::Late_Update(const f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (m_bRenderFailureIsolated)
		return;
	if (m_bReconstructedDiagnosticActive)
	{
		if (!m_bVisible)
			return;
		const shared_ptr<CEffectObject> Self =
			static_pointer_cast<CEffectObject>(shared_from_this());
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::BLEND,
			static_pointer_cast<CGameObject>(Self));
		return;
	}
	std::string GateStatus;
	if (!m_bReconstructedSourceRuntimeActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Execution(GateStatus))
	{
		m_strStatus = std::move(GateStatus);
		return;
	}
	if (!m_bVisible)
		return;
	const shared_ptr<CEffectObject> Self =
		static_pointer_cast<CEffectObject>(shared_from_this());
	const HRESULT hProviderResult =
		CPresentation_Manager::Get().Add_FrameProvider(
			static_pointer_cast<IPresentationProvider>(Self));
	if (FAILED(hProviderResult))
	{
		m_strStatus = "Effect presentation provider budget exceeded.";
		return;
	}
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::BLEND,
		static_pointer_cast<CGameObject>(Self));
}

void Client::CEffectObject::Begin_PresentationSubmission()
{
	m_bPresentationFailureIsolated = false;
	m_ePresentationFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
	m_LastPresentationSubmissionStats = {};
	m_LastPresentationSubmissionStats.Lights.iConfigured =
		m_iConfiguredLightCount;
	m_LastPresentationSubmissionStats.ScreenPosts.iConfigured =
		m_iConfiguredScreenPostCount;
}

HRESULT Client::CEffectObject::Submit_Presentation()
{
	const auto Complete = [this](const HRESULT hResult)
	{
		m_LastPresentationSubmissionStats.bCompleted = true;
		m_LastPresentationSubmissionStats.bCommitted = false;
		if (FAILED(hResult))
			++m_LastPresentationSubmissionStats.iProviderFailures;
		return FAILED(hResult) ?
			Complete_PresentationResult(hResult, m_strStatus) : hResult;
	};
	if (m_bReconstructedDiagnosticActive)
		return Complete(S_OK);
	std::string GateStatus;
	if (!m_bReconstructedSourceRuntimeActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Submit(GateStatus))
	{
		m_ePresentationFailureScope =
			PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
		m_strStatus = std::move(GateStatus);
		return Complete(E_FAIL);
	}
	if (!m_bVisible)
		return Complete(S_OK);
	const EFFECT_EVALUATED_FRAME& Frame = m_Playback.Get_Frame();
	const uint64_t iVisibleLightCount = static_cast<uint64_t>(std::count_if(
		Frame.Lights.begin(), Frame.Lights.end(),
		[this](const EFFECT_EVALUATED_LIGHT& Value)
		{
			return Should_SubmitPreviewElement(Value.pElement);
		}));
	const uint64_t iVisibleScreenPostCount = static_cast<uint64_t>(std::count_if(
		Frame.ScreenPosts.begin(), Frame.ScreenPosts.end(),
		[this](const EFFECT_EVALUATED_SCREEN_POST& Value)
		{
			return Should_SubmitPreviewElement(Value.pElement);
		}));
	m_LastPresentationSubmissionStats.Lights.iExpected = iVisibleLightCount;
	m_LastPresentationSubmissionStats.ScreenPosts.iExpected =
		iVisibleScreenPostCount;
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	const bool_t bElementSetIsolation = nullptr != m_pRenderer &&
		m_pRenderer->Get_PreviewSubmissionIsolation().eKind ==
			EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET;
	Presentation.Register_ProviderSubmissionExpectation(
		!bElementSetIsolation ? m_iConfiguredLightCount :
			iVisibleLightCount,
		iVisibleLightCount,
		!bElementSetIsolation ? m_iConfiguredScreenPostCount :
			iVisibleScreenPostCount,
		iVisibleScreenPostCount);
	bool_t bSuppressed = false;
	for (const EFFECT_EVALUATED_LIGHT& Evaluated : Frame.Lights)
	{
		if (!Should_SubmitPreviewElement(Evaluated.pElement))
			continue;
		LIGHT_DESC Light{};
		if (!Try_BuildEffectPointLightDesc(Evaluated, Light))
		{
			Record_LocalSubmissionResult(
				m_LastPresentationSubmissionStats.Lights, E_INVALIDARG);
			Presentation.Record_TransientLightValidationFailure();
			m_ePresentationFailureScope =
				PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
			m_strStatus = "Effect point-light presentation validation failed.";
			return Complete(E_INVALIDARG);
		}
		const HRESULT hResult = Presentation.Add_TransientLight(Light);
		Record_LocalSubmissionResult(
			m_LastPresentationSubmissionStats.Lights, hResult);
		if (FAILED(hResult))
		{
			m_ePresentationFailureScope = Presentation.Get_LastFailureScope();
			m_strStatus = "Effect point-light presentation submission failed.";
			return Complete(hResult);
		}
		bSuppressed = bSuppressed || S_FALSE == hResult;
	}

	for (const EFFECT_EVALUATED_SCREEN_POST& Evaluated : Frame.ScreenPosts)
	{
		if (!Should_SubmitPreviewElement(Evaluated.pElement))
			continue;
		PRESENTATION_SCREEN_POST_DESC Post;
		switch (Evaluated.eProfile)
		{
		case EFFECT_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED_V1:
			Post.eProfile =
				PRESENTATION_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED;
			break;
		case EFFECT_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED_V1:
			Post.eProfile =
				PRESENTATION_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED;
			break;
		case EFFECT_SCREEN_POST_PROFILE::FILM_NOISE_RECONSTRUCTED_V1:
			Post.eProfile =
				PRESENTATION_SCREEN_POST_PROFILE::FILM_NOISE_RECONSTRUCTED;
			break;
		default:
			Record_LocalSubmissionResult(
				m_LastPresentationSubmissionStats.ScreenPosts, E_INVALIDARG);
			Presentation.Record_ScreenPostValidationFailure();
			m_ePresentationFailureScope =
				PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
			m_strStatus = "Effect screen-post profile validation failed.";
			return Complete(E_INVALIDARG);
		}
		Post.iSourceOrder = Evaluated.iSourceOrder;
		Post.iRandomSeed = Evaluated.iRandomSeed;
		Post.fSampleTimeSeconds = Evaluated.fSampleTimeSeconds;
		Post.fIntensity = Evaluated.fIntensity;
		Post.fSecondaryIntensity = Evaluated.fSecondaryIntensity;
		Post.fFrequency = Evaluated.fFrequency;
		Post.vTint = Evaluated.vTint;
		const HRESULT hResult = Presentation.Add_ScreenPost(Post);
		Record_LocalSubmissionResult(
			m_LastPresentationSubmissionStats.ScreenPosts, hResult);
		if (FAILED(hResult))
		{
			m_ePresentationFailureScope = Presentation.Get_LastFailureScope();
			m_strStatus = "Effect screen-post presentation submission failed.";
			return Complete(hResult);
		}
		bSuppressed = bSuppressed || S_FALSE == hResult;
	}
	if (!Is_CompleteLocalSubmission(
			m_LastPresentationSubmissionStats.Lights) ||
		!Is_CompleteLocalSubmission(
			m_LastPresentationSubmissionStats.ScreenPosts))
	{
		m_ePresentationFailureScope =
			PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
		m_strStatus =
			"Effect typed presentation denominator mismatch.";
		return Complete(E_FAIL);
	}
	if (bSuppressed)
	{
		m_strStatus =
			"Effect typed presentation was suppressed by runtime channel controls.";
		return Complete(S_FALSE);
	}
	return Complete(S_OK);
}

void Client::CEffectObject::Finalize_PresentationSubmission(
	const bool_t bCommitted)
{
	if (!m_LastPresentationSubmissionStats.bCompleted)
		return;
	m_LastPresentationSubmissionStats.bCommitted = bCommitted;
}

HRESULT Client::CEffectObject::Render()
{
	if (m_bRenderFailureIsolated)
		return S_FALSE;
	if (m_bReconstructedDiagnosticActive)
	{
		if (!m_bVisible)
			return S_FALSE;
		const HRESULT Result = m_pRenderer->Render_ReconstructedDiagnostic(
			m_RootWorld, m_eReconstructedDiagnosticSolo);
		m_strStatus = m_pRenderer->Get_Status();
		return Complete_RenderResult(Result, m_strStatus);
	}
	std::string GateStatus;
	if (!m_bReconstructedSourceRuntimeActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Render(GateStatus))
	{
		m_strStatus = std::move(GateStatus);
		return Complete_LocalEffectFailure(
			E_FAIL, m_strStatus, "render", false);
	}
	if (!m_bVisible)
		return S_FALSE;
	const HRESULT Result = m_pRenderer->Render(m_Playback.Get_Frame());
	m_strStatus = m_pRenderer->Get_Status();
	return Complete_RenderResult(Result, m_strStatus);
}

HRESULT Client::CEffectObject::Complete_RenderResult(
	const HRESULT hResult,
	std::string strFailureContext)
{
	if (FAILED(hResult) && !m_pRenderer->Is_LastRenderFailureObjectLocal())
		return hResult;
	return Complete_LocalEffectFailure(hResult, std::move(strFailureContext),
		"render", false);
}

HRESULT Client::CEffectObject::Complete_PresentationResult(
	const HRESULT hResult,
	std::string strFailureContext)
{
	if (FAILED(hResult) &&
		m_ePresentationFailureScope !=
			PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT)
	{
		return hResult;
	}
	return Complete_LocalEffectFailure(hResult, std::move(strFailureContext),
		"presentation", true);
}

HRESULT Client::CEffectObject::Complete_LocalEffectFailure(
	const HRESULT hResult,
	std::string strFailureContext,
	const char* const pFailureChannel,
	const bool_t bPreserveFailedResult)
{
	if (SUCCEEDED(hResult))
		return hResult;
	// Callers reach this helper only after the renderer or presentation
	// provider has explicitly classified the failure as object-local.  Do not
	// infer ownership from a shared HRESULT value such as E_FAIL/E_INVALIDARG.

	if (!m_bRenderFailureIsolated)
	{
		m_bRenderFailureIsolated = true;
		m_bPresentationFailureIsolated = bPreserveFailedResult;
		m_hRenderFailure = hResult;
		m_bVisible = false;
		m_bPlaying = false;
		if (strFailureContext.empty())
			strFailureContext = "Effect renderer returned no failure detail.";
		m_strStatus = std::string("Effect ") + pFailureChannel +
			" failure isolated [" +
			Format_Hresult(hResult) + "]: " + strFailureContext;
		OutputDebugStringA(("[Client][Effect] " + m_strStatus + "\n").c_str());
	}
	return bPreserveFailedResult ? hResult : S_FALSE;
}

void Client::CEffectObject::Reset_RenderFailureIsolation()
{
	m_bRenderFailureIsolated = false;
	m_bPresentationFailureIsolated = false;
	m_ePresentationFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
	m_hRenderFailure = S_OK;
}

unique_ptr<Client::CEffectObject> Client::CEffectObject::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	unique_ptr<CEffectObject> Instance(new CEffectObject(
		std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype()))
		return nullptr;
	return Instance;
}

shared_ptr<CPrototype> Client::CEffectObject::Clone(void* pArg)
{
	shared_ptr<CEffectObject> Instance(new CEffectObject(
		m_pDevice, m_pContext));
	if (FAILED(Instance->Initialize(pArg)))
		return nullptr;
	return Instance;
}
