#include "Renderer.h"
#include "Render_OutputContract.h"
#include "Profiler.h"

#include <fstream>
#include <iomanip>
#include <typeinfo>

namespace
{
	void WriteRendererFailure(
		const char* stage,
		const HRESULT result,
		const char* objectType = nullptr)
	{
#ifdef _DEBUG
		std::ofstream output(
			"RendererExit.user.log",
			std::ios::binary | std::ios::app);
		if (!output)
			return;

		output << "stage=" << (nullptr == stage ? "unknown" : stage)
			<< " hr=0x" << std::hex << std::uppercase
			<< static_cast<unsigned long>(result) << std::dec;
		if (nullptr != objectType)
			output << " object=" << objectType;
		output << '\n';
#else
		UNREFERENCED_PARAMETER(stage);
		UNREFERENCED_PARAMETER(result);
		UNREFERENCED_PARAMETER(objectType);
#endif
	}
}
#include "GameInstance.h"
#include "Presentation_Manager.h"

#include <cmath>

namespace
{
	constexpr uint32_t DEFERRED_PASS_SCENE_RESOLVE =
		ETOUI(DEFERRED::SCENE_RESOLVE);
	constexpr uint32_t DEFERRED_PASS_RGB_NOISE =
		ETOUI(DEFERRED::PRESENTATION_RGB_NOISE);
	constexpr uint32_t DEFERRED_PASS_ZOOM_BLUR =
		ETOUI(DEFERRED::PRESENTATION_ZOOM_BLUR);
	constexpr uint32_t DEFERRED_PASS_FILM_NOISE =
		ETOUI(DEFERRED::PRESENTATION_FILM_NOISE);
	constexpr uint32_t DEFERRED_PASS_TEXTURED_OVERLAY =
		PRESENTATION_TEXTURED_OVERLAY_PASS_INDEX;

	static_assert(8u == DEFERRED_PASS_SCENE_RESOLVE);
	static_assert(9u == DEFERRED_PASS_RGB_NOISE);
	static_assert(10u == DEFERRED_PASS_ZOOM_BLUR);
	static_assert(11u == DEFERRED_PASS_FILM_NOISE);
	static_assert(14u == DEFERRED_PASS_TEXTURED_OVERLAY);
	static_assert(12u == ETOUI(DEFERRED::SSAO_RAW));
	static_assert(13u == ETOUI(DEFERRED::SSAO_BLUR));
	static_assert(14u == ETOUI(DEFERRED::END));

	bool_t IsFiniteInRange(const f32_t fValue, const f32_t fMinimum,
		const f32_t fMaximum)
	{
		return std::isfinite(fValue) &&
			fValue >= fMinimum && fValue <= fMaximum;
	}

	bool_t IsValidRenderQualitySettings(
		const RENDER_QUALITY_SETTINGS& Settings)
	{
		return
			IsFiniteInRange(Settings.fSSAORadius, 0.01f, 8.f) &&
			IsFiniteInRange(Settings.fSSAOBias, 0.f, 1.f) &&
			IsFiniteInRange(Settings.fSSAOIntensity, 0.f, 4.f) &&
			IsFiniteInRange(Settings.fSSAOPower, 0.1f, 8.f) &&
			IsFiniteInRange(Settings.fSSAODistanceFade, 1.f, 1000.f) &&
			Settings.fSSAOBias < Settings.fSSAORadius &&
			Settings.fSSAODistanceFade >= Settings.fSSAORadius &&
			IsFiniteInRange(Settings.fBloomThreshold, 0.f, 64.f) &&
			IsFiniteInRange(Settings.fBloomSoftKnee, 0.f, 1.f) &&
			IsFiniteInRange(Settings.fBloomIntensity, 0.f, 16.f) &&
			IsFiniteInRange(Settings.fBloomScatter, 0.25f, 4.f) &&
			IsFiniteInRange(Settings.fExposure, 0.01f, 32.f) &&
			IsFiniteInRange(Settings.fWhitePoint, 1.f, 64.f) &&
			IsFiniteInRange(Settings.fGamma, 1.f, 3.f) &&
			IsFiniteInRange(Settings.fFXAASubpixel, 0.f, 1.f) &&
			IsFiniteInRange(Settings.fFXAAEdgeThreshold, 0.0312f, 0.333f) &&
			IsFiniteInRange(Settings.fFXAAEdgeThresholdMin, 0.0156f, 0.0833f);
	}
}

CRenderer::CRenderer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{

}

CRenderer::~CRenderer()
{
}

HRESULT CRenderer::Initialize()
{
	float2_t		vViewportSize = CGameInstance::Get().Get_ViewportSize();
	m_vShadowTexelSize = float2_t(
		1.f / static_cast<f32_t>(m_iShadowMapSize),
		1.f / static_cast<f32_t>(m_iShadowMapSize));
	m_iBloomWidth = max(1u, static_cast<uint32_t>(vViewportSize.x) / 2u);
	m_iBloomHeight = max(1u, static_cast<uint32_t>(vViewportSize.y) / 2u);
	m_vBloomTexelSize = float2_t(
		1.f / static_cast<f32_t>(m_iBloomWidth),
		1.f / static_cast<f32_t>(m_iBloomHeight));
	m_iSSAOWidth = max(1u, static_cast<uint32_t>(vViewportSize.x) / 2u);
	m_iSSAOHeight = max(1u, static_cast<uint32_t>(vViewportSize.y) / 2u);
	m_vSSAOTexelSize = float2_t(
		1.f / static_cast<f32_t>(m_iSSAOWidth),
		1.f / static_cast<f32_t>(m_iSSAOHeight));

	/* For.Target_Diffuse */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Diffuse"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R8G8B8A8_UNORM, float4_t(1.f, 1.f, 1.f, 0.f))))
		return E_FAIL;

	/* For.Target_Normal */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Normal"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_UNORM, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* For.Target_Shade */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Shade"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Depth */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Depth"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* For.Target_Specular */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Specular"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_PickPos */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_PickPos"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Emissive */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Emissive"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_SceneHDR */
	/* Scene colour before tone mapping. FP16 so values above 1 survive. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SceneHDR"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Signed RG offsets written by distortion-capable effect shaders. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Distortion"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Half-resolution ambient occlusion: raw estimate plus bilateral resolve. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SSAORaw"), m_iSSAOWidth, m_iSSAOHeight,
		DXGI_FORMAT_R16_FLOAT, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SSAOBlur"), m_iSSAOWidth, m_iSSAOHeight,
		DXGI_FORMAT_R16_FLOAT, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* Half-resolution bloom chain. R11G11B10 preserves positive HDR energy. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_BloomExtract"), m_iBloomWidth, m_iBloomHeight,
		DXGI_FORMAT_R11G11B10_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_BloomPing"), m_iBloomWidth, m_iBloomHeight,
		DXGI_FORMAT_R11G11B10_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_BloomResult"), m_iBloomWidth, m_iBloomHeight,
		DXGI_FORMAT_R11G11B10_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(Ready_Shadow_Resources()))
		return E_FAIL;
	if (FAILED(Ready_Bloom_DSV()))
		return E_FAIL;
	if (FAILED(Ready_SSAO_DSV()))
		return E_FAIL;
	if (FAILED(Ready_ScenePostTargets(
		static_cast<uint32_t>(vViewportSize.x),
		static_cast<uint32_t>(vViewportSize.y))))
	{
		return E_FAIL;
	}



	/* MRT_GameObject */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Normal"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Depth"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_PickPos"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Emissive"))))
		return E_FAIL;

	/* MRT_LightAcc */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Shade"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Specular"))))
		return E_FAIL;

	/* MRT_SceneHDR */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SceneHDR"), TEXT("Target_SceneHDR"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SceneHDR"), TEXT("Target_Distortion"))))
		return E_FAIL;

	/* Half-resolution bloom ping-pong targets. */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_BloomExtract"), TEXT("Target_BloomExtract"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_BloomPing"), TEXT("Target_BloomPing"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_BloomResult"), TEXT("Target_BloomResult"))))
		return E_FAIL;

	/* Half-resolution SSAO raw and bilateral targets. */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SSAORaw"), TEXT("Target_SSAORaw"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SSAOBlur"), TEXT("Target_SSAOBlur"))))
		return E_FAIL;

	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;
	
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(vViewportSize.x, vViewportSize.y, 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix,
		XMMatrixOrthographicLH(vViewportSize.x, vViewportSize.y, 0.f, 1.f));

#ifdef _DEBUG
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Diffuse"), 150.f, 150.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Normal"), 150.f, 450.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Shade"), 450.f, 150.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Specular"), 450.f, 450.f, 300.f, 300.f)))
	//	return E_FAIL;
#endif

	return S_OK;
}

HRESULT CRenderer::Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<CGameObject> pRenderObject)
{
	if (nullptr == pRenderObject ||
		eRenderGroupID >= RENDERGROUP::END)
		return E_FAIL;

	m_RenderObjects[ETOUI(eRenderGroupID)].push_back(pRenderObject);	

	return S_OK;
}

HRESULT CRenderer::Apply_RenderQualitySettings(
	const RENDER_QUALITY_SETTINGS& Settings)
{
	if (!IsValidRenderQualitySettings(Settings))
		return E_INVALIDARG;

	m_RenderQualitySettings = Settings;
	return S_OK;
}

HRESULT CRenderer::Draw()
{
	CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
	CProfilerScope drawScope(pProfiler, "Renderer.Draw");
	const auto profileStage = [pProfiler](
		const char* pName, auto&& operation) -> HRESULT
	{
		CProfilerScope scope(pProfiler, pName);
		return operation();
	};
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	auto FailFrame = [this, &Presentation](
		const char* stage, const HRESULT hResult) -> HRESULT
	{
		WriteRendererFailure(stage, hResult);
		Presentation.Clear_Frame();
		for (auto& RenderGroup : m_RenderObjects)
			RenderGroup.clear();
		return FAILED(hResult) ? hResult : E_FAIL;
	};
	HRESULT hResult = profileStage("Renderer.SubmitProviders",
		[&Presentation]() { return Presentation.Submit_FrameProviders(); });
	if (FAILED(hResult))
		return FailFrame("Submit_FrameProviders", hResult);
	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	if (vViewportSize.x <= 0.f || vViewportSize.y <= 0.f)
	{
		return FailFrame("Viewport", E_INVALIDARG);
	}
	hResult = profileStage("Renderer.ReadyTargets", [this, &vViewportSize]()
		{
			return Ready_ScenePostTargets(
				static_cast<uint32_t>(vViewportSize.x),
				static_cast<uint32_t>(vViewportSize.y));
		});
	if (FAILED(hResult))
		return FailFrame("Ready_ScenePostTargets", hResult);
	hResult = profileStage("Renderer.Shadow",
		[this]() { return Render_Shadow(); });
	if (FAILED(hResult))
		return FailFrame("Render_Shadow", hResult);
	hResult = profileStage("Renderer.GBuffer",
		[this]() { return Render_NonBlend(); });
	if (FAILED(hResult))
		return FailFrame("Render_NonBlend", hResult);
	if (m_RenderQualitySettings.bSSAOEnabled)
	{
		hResult = profileStage("Renderer.SSAO",
			[this]() { return Render_SSAO(); });
		if (FAILED(hResult))
			return FailFrame("Render_SSAO", hResult);
	}
	hResult = profileStage("Renderer.Lights",
		[this]() { return Render_Lights(); });
	if (FAILED(hResult))
		return FailFrame("Render_Lights", hResult);

	/* Scene colour is accumulated in FP16 so lighting and effect values above 1 */
	/* survive until tone mapping. Sky/background join the same target, or the   */
	/* final blit would overwrite them with black.                               */
	hResult = CGameInstance::Get().Begin_MRT(TEXT("MRT_SceneHDR"));
	if (FAILED(hResult))
		return FailFrame("Begin_MRT_SceneHDR", hResult);
	HRESULT hSceneResult = S_OK;
	HRESULT hEndSceneResult = S_OK;
	{
		CRenderOutputContractScope SceneOutputScope(
			RENDER_OUTPUT_CONTRACT::
			SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION,
			m_pContext.Get());
		hSceneResult = profileStage("Renderer.Priority",
			[this]() { return Render_Priority(); });
		if (SUCCEEDED(hSceneResult))
			hSceneResult = profileStage("Renderer.Combined",
				[this]() { return Render_Combined(); });
		if (SUCCEEDED(hSceneResult))
			hSceneResult = profileStage("Renderer.NonLight",
				[this]() { return Render_NonLight(); });
		if (SUCCEEDED(hSceneResult))
			hSceneResult = profileStage("Renderer.Blend",
				[this]() { return Render_Blend(); });

		/* Always restore the back-buffer/DSV pair after entering the HDR MRT. */
		hEndSceneResult = CGameInstance::Get().End_MRT();
	}
	if (FAILED(hSceneResult) || FAILED(hEndSceneResult))
		return FailFrame(
			FAILED(hSceneResult) ? "Render_Scene" : "End_MRT_SceneHDR",
			FAILED(hSceneResult) ? hSceneResult : hEndSceneResult);

	hResult = profileStage("Renderer.ScreenPosts",
		[this]() { return Render_ScreenPosts(); });
	if (FAILED(hResult))
		return FailFrame("Render_ScreenPosts", hResult);

	if (m_RenderQualitySettings.bBloomEnabled)
	{
		hResult = profileStage("Renderer.Bloom",
			[this]() { return Render_Bloom(); });
		if (FAILED(hResult))
			return FailFrame("Render_Bloom", hResult);
	}

	/* The one and only place tone mapping and gamma are applied. */
	hResult = profileStage("Renderer.Final",
		[this]() { return Render_Final(); });
	if (FAILED(hResult))
		return FailFrame("Render_Final", hResult);

	/* UI is authored in display space, so it stays out of the HDR target. */
	hResult = profileStage("Renderer.UI",
		[this]() { return Render_UI(); });
	if (FAILED(hResult))
		return FailFrame("Render_UI", hResult);

#ifdef _DEBUG
	hResult = profileStage("Renderer.Debug",
		[this]() { return Render_Debug(); });
	if (FAILED(hResult))
		return FailFrame("Render_Debug", hResult);
#endif

	Presentation.Clear_Frame();
	return S_OK;
}

#ifdef _DEBUG

HRESULT CRenderer::Add_DebugComponent(shared_ptr<CComponent> pDebugComponent)
{
	m_DebugComponent.push_back(pDebugComponent);
	return S_OK;
}

#endif

HRESULT CRenderer::Render_Priority()
{
	for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
	}

	m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_Shadow()
{
	auto& ShadowObjects =
		m_RenderObjects[ETOUI(RENDERGROUP::SHADOW)];
	if (nullptr == m_pShadowDSV || nullptr == m_pShadowSRV ||
		0u == m_iShadowMapSize)
	{
		ShadowObjects.clear();
		return E_FAIL;
	}

	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);

	if (FAILED(CGameInstance::Get().Begin_DepthOnly(m_pShadowDSV)))
	{
		ShadowObjects.clear();
		return E_FAIL;
	}

	HRESULT hRenderResult = S_OK;
	if (CGameInstance::Get().Is_ShadowLightEnabled())
	{
		SetUp_ViewportDesc(m_iShadowMapSize, m_iShadowMapSize);
		for (auto& pRenderObject : ShadowObjects)
		{
			if (nullptr != pRenderObject &&
				FAILED(pRenderObject->Render_Shadow()))
			{
				WriteRendererFailure(
					"Render_Shadow_Object",
					E_FAIL,
					typeid(*pRenderObject).name());
				hRenderResult = E_FAIL;
				break;
			}
		}
	}
	ShadowObjects.clear();

	const HRESULT hEndResult =
		CGameInstance::Get().End_DepthOnly();
	if (0u < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0u, nullptr);

	return FAILED(hRenderResult) || FAILED(hEndResult) ? E_FAIL : S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
	/* Diffuse + Normal */
	if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_GameObject"))))
		return E_FAIL;

	auto& NonBlendObjects =
		m_RenderObjects[ETOUI(RENDERGROUP::NONBLEND)];
	auto& DeferredOverlayObjects =
		m_RenderObjects[ETOUI(RENDERGROUP::DEFERRED_OVERLAY)];
	for (auto& pRenderObject : NonBlendObjects)
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
	}

	/* Deferred overlays must run after every opaque object while the complete
	   game-object MRT, including Target_Emissive, is still bound. */
	for (auto& pRenderObject : DeferredOverlayObjects)
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_DeferredOverlay();
	}

	NonBlendObjects.clear();
	DeferredOverlayObjects.clear();

	if (FAILED(CGameInstance::Get().End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_SSAO()
{
	if (nullptr == m_pSSAODSV || 0u == m_iSSAOWidth || 0u == m_iSSAOHeight)
		return E_FAIL;

	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);
	SetUp_ViewportDesc(m_iSSAOWidth, m_iSSAOHeight);

	HRESULT hResult = Render_SSAOPass(
		TEXT("MRT_SSAORaw"), DEFERRED::SSAO_RAW);
	if (SUCCEEDED(hResult))
	{
		hResult = Render_SSAOPass(
			TEXT("MRT_SSAOBlur"), DEFERRED::SSAO_BLUR);
	}

	if (0u < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0u, nullptr);

	return hResult;
}

HRESULT CRenderer::Render_SSAOPass(
	const wstring_t& strMRTTag, const DEFERRED ePass)
{
	if (DEFERRED::SSAO_RAW != ePass && DEFERRED::SSAO_BLUR != ePass)
		return E_INVALIDARG;
	if (FAILED(CGameInstance::Get().Begin_MRT(strMRTTag, m_pSSAODSV)))
		return E_FAIL;

	HRESULT hResult = S_OK;
	HRESULT hBindAO = S_OK;
	if (DEFERRED::SSAO_BLUR == ePass)
	{
		hBindAO = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_SSAORaw"), m_pShader, "g_SSAOTexture");
	}

	if (FAILED(hBindAO) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Normal"), m_pShader, "g_NormalTexture")) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vSSAOTexelSize", &m_vSSAOTexelSize,
			sizeof(m_vSSAOTexelSize))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAORadius", &m_RenderQualitySettings.fSSAORadius,
			sizeof(m_RenderQualitySettings.fSSAORadius))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAOBias", &m_RenderQualitySettings.fSSAOBias,
			sizeof(m_RenderQualitySettings.fSSAOBias))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAOIntensity", &m_RenderQualitySettings.fSSAOIntensity,
			sizeof(m_RenderQualitySettings.fSSAOIntensity))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAOPower", &m_RenderQualitySettings.fSSAOPower,
			sizeof(m_RenderQualitySettings.fSSAOPower))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAODistanceFade", &m_RenderQualitySettings.fSSAODistanceFade,
			sizeof(m_RenderQualitySettings.fSSAODistanceFade))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_CameraViewMatrix",
			CGameInstance::Get().Get_Transform(D3DTS::VIEW))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_CameraProjMatrix",
			CGameInstance::Get().Get_Transform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ProjMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(ETOUI(ePass))) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		hResult = E_FAIL;
	}

	/* Balance Begin_MRT even when a shader bind or draw fails. */
	if (FAILED(CGameInstance::Get().End_MRT()))
		hResult = E_FAIL;

	return hResult;
}

HRESULT CRenderer::Render_Lights()
{
	/* Shade */
	if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_LightAcc"))))
		return E_FAIL;

	HRESULT hResult = S_OK;
	const bool_t bShadowEnabled =
		CGameInstance::Get().Is_ShadowLightEnabled();
	const uint32_t iSSAOEnabled =
		m_RenderQualitySettings.bSSAOEnabled ? 1u : 0u;
	HRESULT hBindAO = S_OK;
	if (0u != iSSAOEnabled)
	{
		hBindAO = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_SSAOBlur"), m_pShader, "g_SSAOTexture");
	}
	if (FAILED(hBindAO) ||
		FAILED(m_pShader->Bind_Texture(
			"g_LightDepthTexture", m_pShadowSRV)) ||
		FAILED(CGameInstance::Get().
			Bind_ShadowLight_LightingResources(m_pShader)) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vShadowTexelSize", &m_vShadowTexelSize,
			sizeof(m_vShadowTexelSize))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iSSAOEnabled", &iSSAOEnabled, sizeof(iSSAOEnabled))) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInverse", CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInverse", CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_RawValue("g_vCamPosition", CGameInstance::Get().Get_CamPosition(), sizeof(float4_t))) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(CGameInstance::Get().Render_Lights(
			m_pShader, m_pVIBuffer, bShadowEnabled)))
	{
		hResult = E_FAIL;
	}

	/* Always restore the back buffer even when a light bind or draw fails. */
	if (FAILED(CGameInstance::Get().End_MRT()))
		hResult = E_FAIL;

	return hResult;
}

HRESULT CRenderer::Render_Combined()
{
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Emissive"), m_pShader, "g_EmissiveTexture")))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Begin(ETOUI(DEFERRED::COMBINED))))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_NonLight()
{
	HRESULT hFirstFailure = S_OK;
	for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)])
	{
		if (nullptr != pRenderObject)
		{
			const HRESULT hResult = pRenderObject->Render();
			if (FAILED(hResult) && SUCCEEDED(hFirstFailure))
			{
				WriteRendererFailure(
					"Render_NonLight_Object",
					hResult,
					typeid(*pRenderObject).name());
				hFirstFailure = hResult;
			}
		}
	}

	m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)].clear();

	return hFirstFailure;
}

HRESULT CRenderer::Render_Blend()
{
	HRESULT hFirstFailure = S_OK;
	for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERGROUP::BLEND)])
	{
		if (nullptr != pRenderObject)
		{
			const HRESULT hResult = pRenderObject->Render();
			if (FAILED(hResult) && SUCCEEDED(hFirstFailure))
			{
				WriteRendererFailure(
					"Render_Blend_Object",
					hResult,
					typeid(*pRenderObject).name());
				hFirstFailure = hResult;
			}
		}
	}

	m_RenderObjects[ETOUI(RENDERGROUP::BLEND)].clear();

	return hFirstFailure;
}

HRESULT CRenderer::Render_ScreenPostPass(
	ComPtr<ID3D11ShaderResourceView> pSourceSRV,
	ComPtr<ID3D11RenderTargetView> pDestinationRTV,
	const uint32_t iPassIndex,
	const PRESENTATION_SCREEN_POST_DESC* pPostDesc)
{
	if (nullptr == pDestinationRTV)
		return E_FAIL;
	if (nullptr != pSourceSRV)
	{
		ComPtr<ID3D11Resource> pSourceResource;
		ComPtr<ID3D11Resource> pDestinationResource;
		pSourceSRV->GetResource(&pSourceResource);
		pDestinationRTV->GetResource(&pDestinationResource);
		if (nullptr == pSourceResource || nullptr == pDestinationResource ||
			pSourceResource.Get() == pDestinationResource.Get())
		{
			return E_FAIL;
		}
	}

	ID3D11ShaderResourceView* pNullSRVs[
		D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(
		0u, _countof(pNullSRVs), pNullSRVs);
	ID3D11RenderTargetView* pDestination = pDestinationRTV.Get();
	m_pContext->OMSetRenderTargets(1u, &pDestination, nullptr);
	const float4_t vClear{};
	m_pContext->ClearRenderTargetView(pDestination, &vClear.x);

	if (nullptr == pPostDesc)
	{
		if (FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_SceneHDR"), m_pShader,
				"g_SceneHDRTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_Distortion"), m_pShader,
				"g_DistortionTexture")))
		{
			return E_FAIL;
		}
	}
	else
	{
		if (nullptr == pSourceSRV ||
			FAILED(m_pShader->Bind_Texture(
				"g_PostProcessTexture", pSourceSRV)) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationTime", &pPostDesc->fSampleTimeSeconds,
				sizeof(pPostDesc->fSampleTimeSeconds))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationIntensity", &pPostDesc->fIntensity,
				sizeof(pPostDesc->fIntensity))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationSecondaryIntensity",
				&pPostDesc->fSecondaryIntensity,
				sizeof(pPostDesc->fSecondaryIntensity))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationFrequency", &pPostDesc->fFrequency,
				sizeof(pPostDesc->fFrequency))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_iPresentationSeed", &pPostDesc->iRandomSeed,
				sizeof(pPostDesc->iRandomSeed))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_vPresentationTint", &pPostDesc->vTint,
				sizeof(pPostDesc->vTint))))
		{
			return E_FAIL;
		}
	}

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(iPassIndex)) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CRenderer::Render_ScreenPosts()
{
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	ComPtr<ID3D11RenderTargetView> pOriginalRTV;
	ComPtr<ID3D11DepthStencilView> pOriginalDSV;
	m_pContext->OMGetRenderTargets(
		1u, pOriginalRTV.GetAddressOf(), pOriginalDSV.GetAddressOf());
	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);
	SetUp_ViewportDesc(m_iScenePostWidth, m_iScenePostHeight);

	HRESULT hResult = Render_ScreenPostPass(
		nullptr, m_pScenePostRTVs[0], DEFERRED_PASS_SCENE_RESOLVE);
	const vector<PRESENTATION_SCREEN_POST_DESC>& ScreenPosts =
		Presentation.Get_ScreenPosts();
	for (size_t iPost = 0u;
		SUCCEEDED(hResult) && iPost < ScreenPosts.size(); ++iPost)
	{
		const PRESENTATION_SCREEN_POST_DESC& Post = ScreenPosts[iPost];
		const PRESENTATION_SCREEN_POST_PLAN_STEP Step =
			Build_PresentationScreenPostPlanStep(iPost);
		uint32_t iPassIndex = {};
		switch (Post.eProfile)
		{
		case PRESENTATION_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_RGB_NOISE;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_ZOOM_BLUR;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::FILM_NOISE_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_FILM_NOISE;
			break;
		default:
			hResult = E_FAIL;
			continue;
		}
		hResult = Render_ScreenPostPass(
			m_pScenePostSRVs[Step.iSourceTarget],
			m_pScenePostRTVs[Step.iDestinationTarget],
			iPassIndex, &Post);
	}
	const vector<PRESENTATION_SCREEN_OVERLAY_DESC>& ScreenOverlays =
		Presentation.Get_ScreenOverlays();
	for (size_t iOverlay = 0u;
		SUCCEEDED(hResult) && iOverlay < ScreenOverlays.size(); ++iOverlay)
	{
		const PRESENTATION_SCREEN_OVERLAY_DESC& Overlay =
			ScreenOverlays[iOverlay];
		const PRESENTATION_SCREEN_POST_PLAN_STEP Step =
			Build_PresentationScreenOverlayPlanStep(
				ScreenPosts.size(), iOverlay);
		ComPtr<ID3D11ShaderResourceView> pSourceSRV =
			m_pScenePostSRVs[Step.iSourceTarget];
		ComPtr<ID3D11RenderTargetView> pDestinationRTV =
			m_pScenePostRTVs[Step.iDestinationTarget];
		if (nullptr == pSourceSRV || nullptr == pDestinationRTV ||
			nullptr == Overlay.pTexture)
		{
			hResult = E_FAIL;
			break;
		}
		ComPtr<ID3D11Resource> pSourceResource;
		ComPtr<ID3D11Resource> pDestinationResource;
		ComPtr<ID3D11Resource> pOverlayResource;
		pSourceSRV->GetResource(&pSourceResource);
		pDestinationRTV->GetResource(&pDestinationResource);
		Overlay.pTexture->GetResource(&pOverlayResource);
		if (nullptr == pSourceResource || nullptr == pDestinationResource ||
			nullptr == pOverlayResource ||
			pSourceResource.Get() == pDestinationResource.Get() ||
			pOverlayResource.Get() == pDestinationResource.Get())
		{
			hResult = E_FAIL;
			break;
		}

		ID3D11ShaderResourceView* pNullSRVs[
			D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
		m_pContext->PSSetShaderResources(
			0u, _countof(pNullSRVs), pNullSRVs);
		ID3D11RenderTargetView* pDestination = pDestinationRTV.Get();
		m_pContext->OMSetRenderTargets(1u, &pDestination, nullptr);
		const float4_t vClear{};
		m_pContext->ClearRenderTargetView(pDestination, &vClear.x);
		const uint32_t iCoverageChannel =
			static_cast<uint32_t>(Overlay.eCoverageChannel);
		const uint32_t iFilter = static_cast<uint32_t>(Overlay.eFilter);
		const uint32_t iAddress = static_cast<uint32_t>(Overlay.eAddress);
		if (FAILED(m_pShader->Bind_Texture(
				"g_PostProcessTexture", pSourceSRV)) ||
			FAILED(m_pShader->Bind_Texture(
				"g_PresentationOverlayTexture", Overlay.pTexture)) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationTime", &Overlay.fSampleTimeSeconds,
				sizeof(Overlay.fSampleTimeSeconds))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_vPresentationOverlayPosition", &Overlay.vPosition,
				sizeof(Overlay.vPosition))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_vPresentationOverlayScale", &Overlay.vScale,
				sizeof(Overlay.vScale))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationOverlayRotationDegrees",
				&Overlay.fRotationDegrees,
				sizeof(Overlay.fRotationDegrees))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationOverlayAngularVelocityDegreesPerSecond",
				&Overlay.fAngularVelocityDegreesPerSecond,
				sizeof(Overlay.fAngularVelocityDegreesPerSecond))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_vPresentationOverlayUvDriftPerSecond",
				&Overlay.vUvDriftPerSecond,
				sizeof(Overlay.vUvDriftPerSecond))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_vPresentationOverlayTint", &Overlay.vTint,
				sizeof(Overlay.vTint))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationOverlayAlpha", &Overlay.fAlpha,
				sizeof(Overlay.fAlpha))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_iPresentationOverlayCoverageChannel",
				&iCoverageChannel, sizeof(iCoverageChannel))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_iPresentationOverlayFilter", &iFilter,
				sizeof(iFilter))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_iPresentationOverlayAddress", &iAddress,
				sizeof(iAddress))) ||
			FAILED(m_pShader->Bind_Matrix(
				"g_WorldMatrix", &m_WorldMatrix)) ||
			FAILED(m_pShader->Bind_Matrix(
				"g_ViewMatrix", &m_ViewMatrix)) ||
			FAILED(m_pShader->Bind_Matrix(
				"g_ProjMatrix", &m_ProjMatrix)) ||
			FAILED(m_pShader->Begin(DEFERRED_PASS_TEXTURED_OVERLAY)) ||
			FAILED(m_pVIBuffer->Bind_Resources()) ||
			FAILED(m_pVIBuffer->Render()))
		{
			hResult = E_FAIL;
		}
	}
	m_iScenePostFinalTarget = PresentationScreenCompositionFinalTarget(
		ScreenPosts.size(), ScreenOverlays.size());

	ID3D11ShaderResourceView* pNullSRVs[
		D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(
		0u, _countof(pNullSRVs), pNullSRVs);
	ID3D11RenderTargetView* pOriginal = pOriginalRTV.Get();
	m_pContext->OMSetRenderTargets(
		nullptr == pOriginal ? 0u : 1u,
		nullptr == pOriginal ? nullptr : &pOriginal,
		pOriginalDSV.Get());
	if (0u < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0u, nullptr);
	Presentation.Clear_ScreenPosts();
	Presentation.Clear_ScreenOverlays();
	return hResult;
}

HRESULT CRenderer::Render_Bloom()
{
	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);

	SetUp_ViewportDesc(m_iBloomWidth, m_iBloomHeight);

	HRESULT hResult = Render_BloomPass(
		TEXT("MRT_BloomExtract"),
		m_pScenePostSRVs[m_iScenePostFinalTarget],
		DEFERRED::BLOOM_EXTRACT);
	if (SUCCEEDED(hResult))
	{
		hResult = Render_BloomPass(
			TEXT("MRT_BloomPing"), nullptr,
			DEFERRED::BLOOM_BLUR_H);
	}
	if (SUCCEEDED(hResult))
	{
		hResult = Render_BloomPass(
			TEXT("MRT_BloomResult"), nullptr,
			DEFERRED::BLOOM_BLUR_V);
	}

	/* Restore every viewport exactly as it was before the half-resolution pass. */
	if (0 < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0, nullptr);

	return hResult;
}

HRESULT CRenderer::Render_BloomPass(const wstring_t& strMRTTag,
	ComPtr<ID3D11ShaderResourceView> pSourceSRV, DEFERRED ePass)
{
	if (FAILED(CGameInstance::Get().Begin_MRT(strMRTTag, m_pBloomDSV)))
		return E_FAIL;

	HRESULT hResult = S_OK;
	HRESULT hBindSource = S_OK;
	if (nullptr != pSourceSRV)
		hBindSource = m_pShader->Bind_Texture(
			"g_PostProcessTexture", pSourceSRV);
	else if (DEFERRED::BLOOM_BLUR_H == ePass)
		hBindSource = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_BloomExtract"), m_pShader,
			"g_PostProcessTexture");
	else if (DEFERRED::BLOOM_BLUR_V == ePass)
		hBindSource = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_BloomPing"), m_pShader,
			"g_PostProcessTexture");
	else
		hBindSource = E_FAIL;
	if (FAILED(hBindSource) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vBloomTexelSize", &m_vBloomTexelSize,
			sizeof(m_vBloomTexelSize))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fBloomThreshold", &m_RenderQualitySettings.fBloomThreshold,
			sizeof(m_RenderQualitySettings.fBloomThreshold))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fBloomSoftKnee", &m_RenderQualitySettings.fBloomSoftKnee,
			sizeof(m_RenderQualitySettings.fBloomSoftKnee))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fBloomScatter", &m_RenderQualitySettings.fBloomScatter,
			sizeof(m_RenderQualitySettings.fBloomScatter))) ||
		FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(ETOUI(ePass))) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		hResult = E_FAIL;
	}

	/* Keep render-target state balanced even if binding or drawing failed. */
	if (FAILED(CGameInstance::Get().End_MRT()))
		hResult = E_FAIL;

	return hResult;
}

HRESULT CRenderer::Render_Final()
{
	const uint32_t iBloomEnabled =
		m_RenderQualitySettings.bBloomEnabled ? 1u : 0u;
	const uint32_t iFXAAEnabled =
		m_RenderQualitySettings.bFXAAEnabled ? 1u : 0u;
	const float2_t vInverseSceneSize = {
		1.f / static_cast<f32_t>((max)(1u, m_iScenePostWidth)),
		1.f / static_cast<f32_t>((max)(1u, m_iScenePostHeight)) };

	if (FAILED(m_pShader->Bind_Texture(
		"g_SceneHDRTexture",
		m_pScenePostSRVs[m_iScenePostFinalTarget])))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_BloomResult"), m_pShader, "g_BloomTexture")))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue(
			"g_iBloomEnabled", &iBloomEnabled, sizeof(iBloomEnabled))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fBloomIntensity", &m_RenderQualitySettings.fBloomIntensity,
			sizeof(m_RenderQualitySettings.fBloomIntensity))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fToneMapExposure", &m_RenderQualitySettings.fExposure,
			sizeof(m_RenderQualitySettings.fExposure))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fToneMapWhitePoint", &m_RenderQualitySettings.fWhitePoint,
			sizeof(m_RenderQualitySettings.fWhitePoint))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fToneMapGamma", &m_RenderQualitySettings.fGamma,
			sizeof(m_RenderQualitySettings.fGamma))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iFXAAEnabled", &iFXAAEnabled, sizeof(iFXAAEnabled))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fFXAASubpixel", &m_RenderQualitySettings.fFXAASubpixel,
			sizeof(m_RenderQualitySettings.fFXAASubpixel))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fFXAAEdgeThreshold", &m_RenderQualitySettings.fFXAAEdgeThreshold,
			sizeof(m_RenderQualitySettings.fFXAAEdgeThreshold))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fFXAAEdgeThresholdMin", &m_RenderQualitySettings.fFXAAEdgeThresholdMin,
			sizeof(m_RenderQualitySettings.fFXAAEdgeThresholdMin))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vInverseSceneSize", &vInverseSceneSize,
			sizeof(vInverseSceneSize))))
	{
		return E_FAIL;
	}

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Begin(ETOUI(DEFERRED::FINAL))))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
	for (auto& pRenderObject : m_RenderObjects[ETOUI(RENDERGROUP::UI)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
	}

	m_RenderObjects[ETOUI(RENDERGROUP::UI)].clear();

	return S_OK;
}

HRESULT CRenderer::Ready_Shadow_Resources()
{
	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = m_iShadowMapSize;
	TextureDesc.Height = m_iShadowMapSize;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags =
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> pStagedTexture;
	if (FAILED(m_pDevice->CreateTexture2D(
		&TextureDesc, nullptr, pStagedTexture.GetAddressOf())))
		return E_FAIL;

	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc{};
	DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Texture2D.MipSlice = 0;
	ComPtr<ID3D11DepthStencilView> pStagedDSV;
	if (FAILED(m_pDevice->CreateDepthStencilView(
		pStagedTexture.Get(), &DSVDesc, pStagedDSV.GetAddressOf())))
		return E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;
	ComPtr<ID3D11ShaderResourceView> pStagedSRV;
	if (FAILED(m_pDevice->CreateShaderResourceView(
		pStagedTexture.Get(), &SRVDesc, pStagedSRV.GetAddressOf())))
	{
		return E_FAIL;
	}

	m_pShadowDepthTexture = std::move(pStagedTexture);
	m_pShadowDSV = std::move(pStagedDSV);
	m_pShadowSRV = std::move(pStagedSRV);
	return S_OK;
}

HRESULT CRenderer::Ready_Bloom_DSV()
{
	ComPtr<ID3D11Texture2D> pDepthStencilTexture = { nullptr };

	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = m_iBloomWidth;
	TextureDesc.Height = m_iBloomHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(m_pDevice->CreateTexture2D(
		&TextureDesc, nullptr, pDepthStencilTexture.GetAddressOf())))
	{
		return E_FAIL;
	}

	if (FAILED(m_pDevice->CreateDepthStencilView(
		pDepthStencilTexture.Get(), nullptr, m_pBloomDSV.GetAddressOf())))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CRenderer::Ready_SSAO_DSV()
{
	ComPtr<ID3D11Texture2D> pDepthStencilTexture = { nullptr };

	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = m_iSSAOWidth;
	TextureDesc.Height = m_iSSAOHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(m_pDevice->CreateTexture2D(
		&TextureDesc, nullptr, pDepthStencilTexture.GetAddressOf())))
	{
		return E_FAIL;
	}

	if (FAILED(m_pDevice->CreateDepthStencilView(
		pDepthStencilTexture.Get(), nullptr, m_pSSAODSV.GetAddressOf())))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CRenderer::Ready_ScenePostTargets(
	const uint32_t iWidth, const uint32_t iHeight)
{
	if (0u == iWidth || 0u == iHeight)
		return E_FAIL;
	if (m_iScenePostWidth == iWidth && m_iScenePostHeight == iHeight &&
		nullptr != m_pScenePostTextures[0] &&
		nullptr != m_pScenePostTextures[1] &&
		nullptr != m_pScenePostRTVs[0] &&
		nullptr != m_pScenePostRTVs[1] &&
		nullptr != m_pScenePostSRVs[0] &&
		nullptr != m_pScenePostSRVs[1])
	{
		return S_OK;
	}

	ComPtr<ID3D11Texture2D> StagedTextures[2];
	ComPtr<ID3D11RenderTargetView> StagedRTVs[2];
	ComPtr<ID3D11ShaderResourceView> StagedSRVs[2];
	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1u;
	TextureDesc.ArraySize = 1u;
	TextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	TextureDesc.SampleDesc.Count = 1u;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags =
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	for (size_t iTarget = 0u; iTarget < 2u; ++iTarget)
	{
		if (FAILED(m_pDevice->CreateTexture2D(
				&TextureDesc, nullptr,
				StagedTextures[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateRenderTargetView(
				StagedTextures[iTarget].Get(), nullptr,
				StagedRTVs[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateShaderResourceView(
				StagedTextures[iTarget].Get(), nullptr,
				StagedSRVs[iTarget].GetAddressOf())))
		{
			return E_FAIL;
		}
	}

	for (size_t iTarget = 0u; iTarget < 2u; ++iTarget)
	{
		m_pScenePostTextures[iTarget] = std::move(StagedTextures[iTarget]);
		m_pScenePostRTVs[iTarget] = std::move(StagedRTVs[iTarget]);
		m_pScenePostSRVs[iTarget] = std::move(StagedSRVs[iTarget]);
	}
	m_iScenePostWidth = iWidth;
	m_iScenePostHeight = iHeight;
	m_iScenePostFinalTarget = 0u;
	XMStoreFloat4x4(&m_WorldMatrix,
		XMMatrixScaling(static_cast<f32_t>(iWidth),
			static_cast<f32_t>(iHeight), 1.f));
	XMStoreFloat4x4(&m_ProjMatrix,
		XMMatrixOrthographicLH(static_cast<f32_t>(iWidth),
			static_cast<f32_t>(iHeight), 0.f, 1.f));
	return S_OK;
}

void CRenderer::SetUp_ViewportDesc(uint32_t iWidth, uint32_t iHeight)
{
	D3D11_VIEWPORT			ViewPortDesc;
	ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
	ViewPortDesc.TopLeftX = 0;
	ViewPortDesc.TopLeftY = 0;
	ViewPortDesc.Width = static_cast<f32_t>(iWidth);
	ViewPortDesc.Height = static_cast<f32_t>(iHeight);
	ViewPortDesc.MinDepth = 0.f;
	ViewPortDesc.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &ViewPortDesc);
}

#ifdef _DEBUG

HRESULT CRenderer::Render_Debug()
{
	for (auto& pDebugComponent : m_DebugComponent)
	{
		if (nullptr != pDebugComponent)
			pDebugComponent->Render();
	}
	m_DebugComponent.clear();

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	//if (FAILED(CGameInstance::Get().Render_MRT(TEXT("MRT_GameObject"), m_pShader, m_pVIBuffer)))
	//	return E_FAIL;

	//if (FAILED(CGameInstance::Get().Render_MRT(TEXT("MRT_LightAcc"), m_pShader, m_pVIBuffer)))
	//	return E_FAIL;

	return S_OK;
}

#endif

unique_ptr<CRenderer> CRenderer::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CRenderer>(new CRenderer(pDevice, pContext));

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CRenderer");
		return nullptr;
	}

	return pInstance;
}
