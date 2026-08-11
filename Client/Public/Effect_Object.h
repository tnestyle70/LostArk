#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentRenderer.h"
#include "Effect_Playback.h"
#include "GameObject.h"
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
	virtual HRESULT Submit_Presentation() override;

	bool_t Stage_Document(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	bool_t Stage_PreparedDocument(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPreparedResources,
		std::string& strOutError);
	bool_t Stage_ReconstructedRuntimeProgram(
		const EFFECT_OBJECT_DESC& Desc,
		std::string& strOutError);
	bool_t Stage_ReconstructedDiagnostic(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME> pFrame,
		RECONSTRUCTED_DIAGNOSTIC_SOLO eSolo,
		std::string& strOutError);
	void Set_ReconstructedDiagnosticSolo(
		RECONSTRUCTED_DIAGNOSTIC_SOLO eSolo);
	bool_t Is_ReconstructedDiagnosticActive() const
	{
		return m_bReconstructedDiagnosticActive;
	}
	void Set_RootWorld(const float4x4_t& RootWorld);
	void Set_SourceAnchorWorlds(
		const std::unordered_map<std::string, float4x4_t>& SourceAnchorWorlds);
	void Set_SampleTime(f32_t fSampleTimeSeconds);
	void Advance_Preview(f32_t fTimeDelta);
	void Advance_Preview(
		f32_t fTimeDelta,
		const float4x4_t& RootWorld);
	void Set_Playing(bool_t bPlaying) { m_bPlaying = bPlaying; }
	void Set_Visible(bool_t bVisible) { m_bVisible = bVisible; }
	void Reset();
	bool_t Is_Finished() const { return m_Playback.Is_Finished(); }
	bool_t Query_ParticleRuntimeProbe(
		std::string_view strElementId,
		EFFECT_PARTICLE_RUNTIME_PROBE& OutProbe) const
	{
		return m_Playback.Query_ParticleRuntimeProbe(strElementId, OutProbe);
	}
	const std::string& Get_Status() const { return m_strStatus; }
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
	bool_t m_bPlaying = true;
	bool_t m_bVisible = true;
	f32_t m_fPlaybackRate = 1.f;
	std::string m_strStatus;

public:
	static unique_ptr<CEffectObject> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
