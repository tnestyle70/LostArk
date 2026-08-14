#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentRenderer.h"
#include "Effect_Playback.h"
#include "GameObject.h"
#include "Presentation_Manager.h"
#include "PresentationProvider.h"

#include <memory>
#include <string>
#include <unordered_map>

NS_BEGIN(Client)

class CEffectObject final : public CGameObject, public IPresentationProvider
{
public:
	struct EFFECT_OBJECT_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		const EFFECT_DOCUMENT_DESC* pDocument = nullptr;
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPreparedResources;
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pVisualProgramProjection;
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pReconstructedRuntimePreparation;
		float4x4_t RootWorld{};
		bool_t bAutoPlay = true;
		bool_t bRequirePreparedResources = false;
		f32_t fPlaybackRate = 1.f;
	};

private:
	CEffectObject(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CEffectObject();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Begin_PresentationSubmission() override;
	virtual HRESULT Submit_Presentation() override;
	virtual bool_t Is_PresentationFailureIsolated() const override
	{
		return m_bPresentationFailureIsolated;
	}
	virtual PRESENTATION_FAILURE_SCOPE Get_PresentationFailureScope() const override
	{
		return m_ePresentationFailureScope;
	}
	virtual void Finalize_PresentationSubmission(bool_t bCommitted) override;

	bool_t Stage_Document(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	bool_t Stage_PreparedDocument(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPreparedResources,
		std::string& strOutError);
	bool_t Stage_PrevalidatedVisualProgramDocument(
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPreparedResources,
		std::string& strOutError);
	bool_t Stage_ReconstructedRuntimeProgram(
		const EFFECT_OBJECT_DESC& Desc,
		std::string& strOutError);
	bool_t Stage_ReconstructedSourceRuntime(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPreparedResources,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	bool_t Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPreparedResources,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	bool_t Stage_ReconstructedDiagnostic(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME> pFrame,
		RECONSTRUCTED_DIAGNOSTIC_SOLO eSolo,
		std::string& strOutError);
	void Set_ReconstructedDiagnosticSolo(
		RECONSTRUCTED_DIAGNOSTIC_SOLO eSolo);
	bool_t Set_PreviewSubmissionIsolation(
		const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
		std::string& strOutError);
	bool_t Set_PreviewElementIsolation(
		std::vector<std::string> ElementIds,
		std::string& strOutError);
	void Reset_PreviewSubmissionIsolation();
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION&
		Get_PreviewSubmissionIsolation() const
	{
		return m_pRenderer->Get_PreviewSubmissionIsolation();
	}
	bool_t Is_ReconstructedDiagnosticActive() const
	{
		return m_bReconstructedDiagnosticActive;
	}
	bool_t Is_ReconstructedSourceRuntimeActive() const
	{
		return m_bReconstructedSourceRuntimeActive &&
			!m_bRenderFailureIsolated;
	}
	bool_t Is_SourceVisualProgramActive() const
	{
		return m_bSourceVisualProgramActive &&
			!m_bRenderFailureIsolated;
	}
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
		Get_SourceVisualProgramProjection() const
	{
		return m_Playback.Get_SourceVisualProgramProjection();
	}
	bool_t Is_RenderFailureIsolated() const
	{
		return m_bRenderFailureIsolated;
	}
	HRESULT Get_IsolatedRenderFailure() const { return m_hRenderFailure; }
	void Set_RootWorld(const float4x4_t& RootWorld);
	/* PresentationService calls this immediately before its single seek/update.
	   It changes only the pending root so FOLLOW does not rebuild twice. */
	void Set_RootWorldForNextUpdate(const float4x4_t& RootWorld);
	void Set_SourceAnchorWorlds(
		const std::unordered_map<std::string, float4x4_t>& SourceAnchorWorlds);
	void Set_SourceAnchorWorlds(
		std::unordered_map<std::string, float4x4_t>&& SourceAnchorWorlds);
	void Set_SampleTime(f32_t fSampleTimeSeconds);
	void Advance_Preview(f32_t fTimeDelta);
	void Advance_Preview(
		f32_t fTimeDelta,
		const float4x4_t& RootWorld);
	bool_t Set_SampleTimeWithTransformHistory(
		f32_t fSampleTimeSeconds,
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
		std::string& strOutError);
	bool_t Advance_PreviewWithTransformHistory(
		f32_t fTimeDelta,
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
		std::string& strOutError);
	f64_t Get_PreviewFixedStepClockSeconds() const
	{
		return m_Playback.Get_FixedStepClockSeconds();
	}
	void Set_Playing(bool_t bPlaying)
	{
		if (!m_bRenderFailureIsolated || !bPlaying)
			m_bPlaying = bPlaying;
	}
	void Set_Visible(bool_t bVisible)
	{
		if (!m_bRenderFailureIsolated || !bVisible)
			m_bVisible = bVisible;
	}
	void Reset();
	bool_t Is_Finished() const { return m_Playback.Is_Finished(); }
	bool_t Query_ParticleRuntimeProbe(
		std::string_view strElementId,
		EFFECT_PARTICLE_RUNTIME_PROBE& OutProbe) const
	{
		return m_Playback.Query_ParticleRuntimeProbe(strElementId, OutProbe);
	}
	const std::string& Get_Status() const { return m_strStatus; }
	const PRESENTATION_SUBMISSION_STATS&
		Get_LastPresentationSubmissionStats() const
	{
		return m_LastPresentationSubmissionStats;
	}
	const EFFECT_GPU_RENDER_SUBMISSION_STATS&
		Get_LastRenderSubmissionStats() const
	{
		return m_pRenderer->Get_LastRenderSubmissionStats();
	}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	const EFFECT_EVALUATED_FRAME& Get_ReconstructedTestFrame() const
	{
		return m_Playback.Get_Frame();
	}
#endif
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		Get_ReconstructedRuntimePreparation() const
	{
		return m_ReconstructedRuntimeBoundary.Get_Preparation();
	}
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		Get_ReconstructedRuntimeEntry() const
	{
		return m_ReconstructedRuntimeBoundary.Get_CatalogEntry();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_ReconstructedRuntimeProgram() const
	{
		return m_ReconstructedRuntimeBoundary.Get_Program();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_PlaybackReconstructedRuntimeProgram() const
	{
		return m_Playback.Get_ReconstructedRuntimeProgram();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_RendererReconstructedRuntimeProgram() const
	{
		return m_pRenderer->Get_ReconstructedRuntimeProgram();
	}

private:
	bool_t Stage_ReconstructedRuntimeEntry(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	bool_t Should_SubmitPreviewElement(const EFFECT_ELEMENT_DESC* pElement) const;
	HRESULT Complete_RenderResult(
		HRESULT hResult,
		std::string strFailureContext);
	HRESULT Complete_PresentationResult(
		HRESULT hResult,
		std::string strFailureContext);
	HRESULT Complete_LocalEffectFailure(
		HRESULT hResult,
		std::string strFailureContext,
		const char* pFailureChannel,
		bool_t bPreserveFailedResult);
	void Reset_RenderFailureIsolation();

private:
	unique_ptr<CEffectDocumentRenderer> m_pRenderer;
	CEffectReconstructedRuntimeBoundary m_ReconstructedRuntimeBoundary;
	CEffectPlayback m_Playback;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME>
		m_pReconstructedDiagnosticFrame;
	float4x4_t m_RootWorld{};
	RECONSTRUCTED_DIAGNOSTIC_SOLO m_eReconstructedDiagnosticSolo =
		RECONSTRUCTED_DIAGNOSTIC_SOLO::END;
	bool_t m_bReconstructedDiagnosticActive = false;
	bool_t m_bSourceVisualProgramActive = false;
	bool_t m_bReconstructedSourceRuntimeActive = false;
	bool_t m_bRenderFailureIsolated = false;
	bool_t m_bPresentationFailureIsolated = false;
	PRESENTATION_FAILURE_SCOPE m_ePresentationFailureScope =
		PRESENTATION_FAILURE_SCOPE::NONE;
	HRESULT m_hRenderFailure = S_OK;
	bool_t m_bPlaying = true;
	bool_t m_bVisible = true;
	f32_t m_fPlaybackRate = 1.f;
	uint64_t m_iConfiguredLightCount = 0u;
	uint64_t m_iConfiguredScreenPostCount = 0u;
	PRESENTATION_SUBMISSION_STATS m_LastPresentationSubmissionStats;
	std::string m_strStatus;

public:
	static unique_ptr<CEffectObject> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
