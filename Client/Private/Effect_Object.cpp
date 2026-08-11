#include "Effect_Object.h"

#include "Effect_DocumentRenderer.h"
#include "Effect_LightPresentation.h"
#include "GameInstance.h"
#include "Presentation_Manager.h"

#include <cmath>

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
	if (nullptr == pArg)
		return m_pRenderer->Initialize();
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
	if (nullptr != Desc.pReconstructedRuntimePreparation)
	{
		if (!Stage_ReconstructedRuntimeProgram(Desc, m_strStatus))
		{
			return E_FAIL;
		}
		return S_OK;
	}
	if (FAILED(m_pRenderer->Initialize()))
		return E_FAIL;
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
	m_Playback.Seek(0.f, m_RootWorld);
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
	m_strStatus = "Prepared Effect Document staged.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectObject::Stage_ReconstructedRuntimeProgram(
	const EFFECT_OBJECT_DESC& Desc,
	std::string& strOutError)
{
	if (nullptr == Desc.pReconstructedRuntimePreparation ||
		nullptr != Desc.pDocument || nullptr != Desc.pPreparedResources ||
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
	m_bPlaying = false;
	m_bVisible = false;
	m_strStatus =
		"Reconstructed Effect program prepared; Product execution remains blocked.";
	strOutError.clear();
	return true;
}

void Client::CEffectObject::Set_RootWorld(const float4x4_t& RootWorld)
{
	m_RootWorld = RootWorld;
	m_Playback.Update(0.f, m_RootWorld);
}

void Client::CEffectObject::Set_SourceAnchorWorlds(
	const std::unordered_map<std::string, float4x4_t>& SourceAnchorWorlds)
{
	m_Playback.Set_SourceAnchorWorlds(SourceAnchorWorlds);
}

void Client::CEffectObject::Set_SampleTime(const f32_t fSampleTimeSeconds)
{
	m_bPlaying = false;
	m_Playback.Seek(fSampleTimeSeconds, m_RootWorld);
}

void Client::CEffectObject::Advance_Preview(const f32_t fTimeDelta)
{
	m_Playback.Update((std::max)(0.f, fTimeDelta), m_RootWorld);
}

void Client::CEffectObject::Advance_Preview(
	const f32_t fTimeDelta,
	const float4x4_t& RootWorld)
{
	m_RootWorld = RootWorld;
	m_Playback.Update((std::max)(0.f, fTimeDelta), m_RootWorld);
}

void Client::CEffectObject::Reset()
{
	m_Playback.Seek(0.f, m_RootWorld);
}

void Client::CEffectObject::Update(const f32_t fTimeDelta)
{
	std::string GateStatus;
	if (!m_ReconstructedRuntimeBoundary.Admit_Execution(GateStatus))
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
	std::string GateStatus;
	if (!m_ReconstructedRuntimeBoundary.Admit_Execution(GateStatus))
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

HRESULT Client::CEffectObject::Submit_Presentation()
{
	std::string GateStatus;
	if (!m_ReconstructedRuntimeBoundary.Admit_Submit(GateStatus))
	{
		m_strStatus = std::move(GateStatus);
		return E_FAIL;
	}
	if (!m_bVisible)
		return S_OK;
	const EFFECT_EVALUATED_FRAME& Frame = m_Playback.Get_Frame();
	for (const EFFECT_EVALUATED_LIGHT& Evaluated : Frame.Lights)
	{
		LIGHT_DESC Light{};
		if (!Try_BuildEffectPointLightDesc(Evaluated, Light) ||
			FAILED(CPresentation_Manager::Get().Add_TransientLight(Light)))
		{
			return E_FAIL;
		}
	}

	for (const EFFECT_EVALUATED_SCREEN_POST& Evaluated : Frame.ScreenPosts)
	{
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
			return E_FAIL;
		}
		Post.iSourceOrder = Evaluated.iSourceOrder;
		Post.iRandomSeed = Evaluated.iRandomSeed;
		Post.fSampleTimeSeconds = Evaluated.fSampleTimeSeconds;
		Post.fIntensity = Evaluated.fIntensity;
		Post.fSecondaryIntensity = Evaluated.fSecondaryIntensity;
		Post.fFrequency = Evaluated.fFrequency;
		Post.vTint = Evaluated.vTint;
		if (FAILED(CPresentation_Manager::Get().Add_ScreenPost(Post)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CEffectObject::Render()
{
	std::string GateStatus;
	if (!m_ReconstructedRuntimeBoundary.Admit_Render(GateStatus))
	{
		m_strStatus = std::move(GateStatus);
		return E_FAIL;
	}
	if (!m_bVisible)
		return S_FALSE;
	const HRESULT Result = m_pRenderer->Render(m_Playback.Get_Frame());
	if (FAILED(Result))
		m_strStatus = m_pRenderer->Get_Status();
	return Result;
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
