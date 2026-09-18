#include "Renderer.h"
#include "BlendSortKey.h"
#pragma push_macro("new")
#undef new
#include "DirectXTK/DDSTextureLoader.h"
#pragma pop_macro("new")
#include "Engine_RenderTypes.h"
#include "Engine_VertexTypes.h"
#include "Material.h"
#include "Render_OutputContract.h"
#include "Profiler.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <typeinfo>
#include <filesystem>

namespace
{
	void WriteRendererFailure(
        const char* stage, const HRESULT result, const char* objectType = nullptr) noexcept
    {
        try
        {
            static unsigned int records = 0u;
            if (records >= 256u) return;
            ++records;
            wchar_t modulePath[32768]{};
            const DWORD length = GetModuleFileNameW(nullptr, modulePath, 32768u);
            std::filesystem::path path = "RendererExit.user.log";
            if (length > 0u && length < 32768u)
                path = std::filesystem::path(modulePath).parent_path().parent_path().parent_path() /
                    L"Default" / L"RendererExit.user.log";
            std::error_code error;
            const auto bytes = std::filesystem::file_size(path, error);
            const bool rotate = !error && bytes >= 1024u * 1024u;
            std::ofstream output(path, std::ios::binary | (rotate ? std::ios::trunc : std::ios::app));
            if (!output) return;
            output << "pid=" << GetCurrentProcessId() << " tick=" << GetTickCount64()
                << " stage=" << (stage ? stage : "unknown") << " hr=0x" << std::hex
                << std::uppercase << static_cast<unsigned long>(result) << std::dec;
            if (objectType) output << " object=" << objectType;
            output << '\n';
        }
        catch (...) { }
    }
}
#include "GameInstance.h"
#include "Presentation_Manager.h"
#include "Transform.h"

#include <algorithm>
#include <cfloat>
#include <cstring>
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
	constexpr uint32_t DEFERRED_PASS_CHROMATIC_ABERRATION =
		ETOUI(DEFERRED::PRESENTATION_CHROMATIC_ABERRATION);

	static_assert(8u == DEFERRED_PASS_SCENE_RESOLVE);
	static_assert(9u == DEFERRED_PASS_RGB_NOISE);
	static_assert(10u == DEFERRED_PASS_ZOOM_BLUR);
	static_assert(11u == DEFERRED_PASS_FILM_NOISE);
	static_assert(14u == DEFERRED_PASS_TEXTURED_OVERLAY);
	static_assert(12u == ETOUI(DEFERRED::SSAO_RAW));
	static_assert(13u == ETOUI(DEFERRED::SSAO_BLUR));
	static_assert(15u == DEFERRED_PASS_CHROMATIC_ABERRATION);
	static_assert(16u == ETOUI(DEFERRED::SPOT));
	static_assert(17u == ETOUI(DEFERRED::PRESENTATION_DISPLAY_OVERLAY));
	static_assert(18u == ETOUI(DEFERRED::SOURCE_LIGHT_MASK));
	static_assert(19u == ETOUI(DEFERRED::SOURCE_DIRECTIONAL));
	static_assert(20u == ETOUI(DEFERRED::SOURCE_POINT));
	static_assert(21u == ETOUI(DEFERRED::SOURCE_SPOT));
	static_assert(28u == ETOUI(DEFERRED::SOURCE_LUT_BAKE));
	static_assert(29u == ETOUI(DEFERRED::SOURCE_LUT_BAKE_NEUTRAL));
	static_assert(30u == ETOUI(DEFERRED::END));

	bool_t IsFiniteInRange(const f32_t fValue, const f32_t fMinimum,
		const f32_t fMaximum)
	{
		return std::isfinite(fValue) &&
			fValue >= fMinimum && fValue <= fMaximum;
	}


	bool_t IsValidSourcePostProcess(const SOURCE_POST_PROCESS_SETTINGS& source)
	{
		const auto vectorInRange = [](const float3_t& v, float minimum, float maximum)
		{ return IsFiniteInRange(v.x, minimum, maximum) && IsFiniteInRange(v.y, minimum, maximum) && IsFiniteInRange(v.z, minimum, maximum); };
		if (!IsFiniteInRange(source.fToneScale, 0.000001f, 64.f) ||
			!IsFiniteInRange(source.fToneRange, 0.000001f, 128.f) ||
			!IsFiniteInRange(source.fToneToe, 0.f, 1.f) ||
			!IsFiniteInRange(source.fDesaturation, 0.f, 1.f) ||
			!vectorInRange(source.vHighlights, 0.000001f, 64.f) ||
			!vectorInRange(source.vMidtones, 0.000001f, 64.f) ||
			!vectorInRange(source.vShadows, -64.f, 64.f) ||
			!vectorInRange(source.vColorize, 0.f, 64.f) || source.LutLayers.size() > 32u) return false;
		float weight = 0.f;
		for (const auto& layer : source.LutLayers)
		{
			if (!IsFiniteInRange(layer.fWeight, 0.f, 1.f)) return false;
			weight += layer.fWeight;
		}
		return source.LutLayers.empty() || fabsf(weight - 1.f) < 0.0001f;
	}

	bool_t SameSourceGrading(const SOURCE_POST_PROCESS_SETTINGS& a, const SOURCE_POST_PROCESS_SETTINGS& b)
	{
		const auto equal = [](const float3_t& x, const float3_t& y) { return x.x == y.x && x.y == y.y && x.z == y.z; };
		if (a.fDesaturation != b.fDesaturation || !equal(a.vHighlights,b.vHighlights) ||
			!equal(a.vMidtones,b.vMidtones) || !equal(a.vShadows,b.vShadows) ||
			!equal(a.vColorize,b.vColorize) || a.LutLayers.size() != b.LutLayers.size()) return false;
		for (size_t i = 0u; i < a.LutLayers.size(); ++i)
			if (a.LutLayers[i].pLut != b.LutLayers[i].pLut || a.LutLayers[i].fWeight != b.LutLayers[i].fWeight) return false;
		return true;
	}


	struct ScopedSourceLutState final
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targets[8]{};
		ComPtr<ID3D11DepthStencilView> depth;
		D3D11_VIEWPORT viewports[16]{}; UINT viewportCount=16u;
		ComPtr<ID3D11VertexShader> vs; ComPtr<ID3D11PixelShader> ps; ComPtr<ID3D11GeometryShader> gs;
		ComPtr<ID3D11InputLayout> layout; ComPtr<ID3D11Buffer> vertex,index;
		UINT stride=0u,offset=0u,indexOffset=0u; DXGI_FORMAT indexFormat=DXGI_FORMAT_UNKNOWN;
		D3D11_PRIMITIVE_TOPOLOGY topology=D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		ComPtr<ID3D11BlendState> blend; float blendFactor[4]{}; UINT sampleMask=0u;
		ComPtr<ID3D11DepthStencilState> depthState; UINT stencilRef=0u;
		ComPtr<ID3D11RasterizerState> raster;
		ID3D11ShaderResourceView* resources[128]{}; ID3D11SamplerState* samplers[16]{};
		ID3D11Buffer* psBuffers[14]{}; ID3D11Buffer* vsBuffers[14]{};
		explicit ScopedSourceLutState(ID3D11DeviceContext* value) : context(value)
		{
			context->OMGetRenderTargets(8u,targets,depth.GetAddressOf());
			context->RSGetViewports(&viewportCount,viewports);
			context->VSGetShader(vs.GetAddressOf(),nullptr,nullptr); context->PSGetShader(ps.GetAddressOf(),nullptr,nullptr);
			context->GSGetShader(gs.GetAddressOf(),nullptr,nullptr); context->IAGetInputLayout(layout.GetAddressOf());
			context->IAGetVertexBuffers(0u,1u,vertex.GetAddressOf(),&stride,&offset);
			context->IAGetIndexBuffer(index.GetAddressOf(),&indexFormat,&indexOffset); context->IAGetPrimitiveTopology(&topology);
			context->OMGetBlendState(blend.GetAddressOf(),blendFactor,&sampleMask);
			context->OMGetDepthStencilState(depthState.GetAddressOf(),&stencilRef); context->RSGetState(raster.GetAddressOf());
			context->PSGetShaderResources(0u,128u,resources); context->PSGetSamplers(0u,16u,samplers);
			context->PSGetConstantBuffers(0u,14u,psBuffers); context->VSGetConstantBuffers(0u,14u,vsBuffers);
		}
		~ScopedSourceLutState()
		{
			ID3D11ShaderResourceView* empty[128]{}; context->PSSetShaderResources(0u,128u,empty);
			context->OMSetRenderTargets(8u,targets,depth.Get()); context->RSSetViewports(viewportCount,viewports);
			context->VSSetShader(vs.Get(),nullptr,0u); context->PSSetShader(ps.Get(),nullptr,0u); context->GSSetShader(gs.Get(),nullptr,0u);
			context->IASetInputLayout(layout.Get()); ID3D11Buffer* vb=vertex.Get(); context->IASetVertexBuffers(0u,1u,&vb,&stride,&offset);
			context->IASetIndexBuffer(index.Get(),indexFormat,indexOffset); context->IASetPrimitiveTopology(topology);
			context->OMSetBlendState(blend.Get(),blendFactor,sampleMask); context->OMSetDepthStencilState(depthState.Get(),stencilRef);
			context->RSSetState(raster.Get()); context->PSSetShaderResources(0u,128u,resources); context->PSSetSamplers(0u,16u,samplers);
			context->PSSetConstantBuffers(0u,14u,psBuffers); context->VSSetConstantBuffers(0u,14u,vsBuffers);
			for(auto* p:targets) if(p)p->Release(); for(auto* p:resources) if(p)p->Release();
			for(auto* p:samplers) if(p)p->Release(); for(auto* p:psBuffers) if(p)p->Release(); for(auto* p:vsBuffers) if(p)p->Release();
		}
	};

	bool_t IsValidRenderQualitySettings(
		const RENDER_QUALITY_SETTINGS& Settings)
	{
		return
			IsValidSourcePostProcess(Settings.SourcePostProcess) &&
			IsFiniteInRange(Settings.vBloomTint.x, 0.f, 1.f) &&
			IsFiniteInRange(Settings.vBloomTint.y, 0.f, 1.f) &&
			IsFiniteInRange(Settings.vBloomTint.z, 0.f, 1.f) &&
			IsFiniteInRange(Settings.vBloomTint.w, 1.f, 1.f) &&
			IsFiniteInRange(Settings.fSceneDesaturation, 0.f, 1.f) &&
			Settings.iColorFilterType >= 0 && Settings.iColorFilterType <= 3 &&
			IsFiniteInRange(Settings.fColorFilterStrength, 0.f, 1.f) &&
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
		DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(1.f, 1.f, 1.f, 0.f))))
		return E_FAIL;

	/* For.Target_Specular */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Specular"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Material input RGB is separate from accumulated light specular. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_MaterialSpecular"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

    // Source character direct lighting needs UV and its full tangent basis.
    if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_CharacterSurface"), vViewportSize.x, vViewportSize.y,
        DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))) ||
        FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_CharacterGeometry"), vViewportSize.x, vViewportSize.y,
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

	// One shared full-resolution contribution target for all effects, never one per skill.
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SceneBloom"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Refractive effects read this snapshot while SceneHDR remains the output. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_EffectSceneColor"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_EffectSceneBloom"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Signed offsets: RG for ordinary effects, BA for receiver-isolated noise. */
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
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_MaterialSpecular"))))
		return E_FAIL;
    if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_CharacterSurface"))))
        return E_FAIL;
    if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_CharacterGeometry"))))
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
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SceneHDR"), TEXT("Target_SceneBloom"))))
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

	m_RenderObjects[ETOUI(eRenderGroupID)].push_back(std::move(pRenderObject));
    if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
    {
        switch (eRenderGroupID)
        {
        case RENDERGROUP::PRIORITY: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsPriority); break;
        case RENDERGROUP::SHADOW: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsShadow); break;
        case RENDERGROUP::NONBLEND: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsNonBlend); break;
        case RENDERGROUP::BLEND: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsBlend); break;
        default: break; // Other queues are not included in these four counters.
        }
    }

	return S_OK;
}

HRESULT CRenderer::Apply_MaterialRenderSettings(const MATERIAL_RENDER_SETTINGS& settings)
{
	if (static_cast<uint32_t>(settings.eDebugView) >= static_cast<uint32_t>(MATERIAL_DEBUG_VIEW::END))
		return E_INVALIDARG;
	m_MaterialRenderSettings = settings;
	return S_OK;
}


HRESULT CRenderer::Stage_SourceGradingLut(const SOURCE_POST_PROCESS_SETTINGS& source, f32_t gamma,
	ComPtr<ID3D11ShaderResourceView>& output)
{
	if (!m_pShader || !m_pVIBuffer) return E_FAIL;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width=256u; desc.Height=16u; desc.MipLevels=desc.ArraySize=desc.SampleDesc.Count=1u;
	desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.Usage=D3D11_USAGE_IMMUTABLE; desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
	std::array<ComPtr<ID3D11ShaderResourceView>,4u> inputs;
	ID3D11ShaderResourceView* inputViews[4]{};
	float4_t weights{};
	float neutralWeight=source.LutLayers.empty()?1.f:0.f;
	size_t inputIndex=0u;
	for (const auto& layer : source.LutLayers)
	{
		if (!layer.pLut) { neutralWeight+=layer.fWeight; continue; }
		if (inputIndex>=inputs.size()) return E_INVALIDARG;
		std::array<uint32_t,4096u> pixels{};
		for(size_t i=0u;i<pixels.size();++i)
		{
			const auto& p=layer.pLut->Pixels[i];
			if(!IsFiniteInRange(p.x,0.f,1.f)||!IsFiniteInRange(p.y,0.f,1.f)||!IsFiniteInRange(p.z,0.f,1.f)||!IsFiniteInRange(p.w,0.f,1.f))return E_INVALIDARG;
			const auto byte=[](float v){return static_cast<uint32_t>(std::nearbyintf(v*255.f));};
			pixels[i]=byte(p.z)|(byte(p.y)<<8u)|(byte(p.x)<<16u)|(byte(p.w)<<24u);
		}
		D3D11_SUBRESOURCE_DATA initial{}; initial.pSysMem=pixels.data(); initial.SysMemPitch=256u*sizeof(uint32_t);
		ComPtr<ID3D11Texture2D> texture;
		HRESULT result=m_pDevice->CreateTexture2D(&desc,&initial,texture.GetAddressOf());
		if(SUCCEEDED(result))result=m_pDevice->CreateShaderResourceView(texture.Get(),nullptr,inputs[inputIndex].GetAddressOf());
		if(FAILED(result))return result;
		inputViews[inputIndex]=inputs[inputIndex].Get();
		(&weights.x)[inputIndex]=layer.fWeight; ++inputIndex;
	}
	desc.Usage=D3D11_USAGE_DEFAULT; desc.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
	ComPtr<ID3D11Texture2D> target;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11ShaderResourceView> staged;
	HRESULT result=m_pDevice->CreateTexture2D(&desc,nullptr,target.GetAddressOf());
	if(SUCCEEDED(result))result=m_pDevice->CreateRenderTargetView(target.Get(),nullptr,rtv.GetAddressOf());
	if(SUCCEEDED(result))result=m_pDevice->CreateShaderResourceView(target.Get(),nullptr,staged.GetAddressOf());
	if(FAILED(result))return result;
	const float d=source.fDesaturation;
	const float4_t parameters[7]={
		{source.vShadows.x,source.vShadows.y,source.vShadows.z,1.f-d},
		{1.f/source.vHighlights.x,1.f/source.vHighlights.y,1.f/source.vHighlights.z,0.f},
		{source.vMidtones.x,source.vMidtones.y,source.vMidtones.z,0.f},
		{.3f*d,.59f*d,.11f*d,0.f},
		{source.vColorize.x,source.vColorize.y,source.vColorize.z,0.f},
		{1.f,1.f,1.f,1.f/(std::max)(gamma,.0001f)},
		{0.f,0.f,0.f,0.f}};
	{
		ScopedSourceLutState restore(m_pContext.Get());
		ID3D11RenderTargetView* destination=rtv.Get();
		m_pContext->OMSetRenderTargets(1u,&destination,nullptr);
		SetUp_ViewportDesc(256u,16u);
		if(FAILED(m_pShader->Bind_Textures("g_SourceLutInputs",inputViews,4u)) ||
			FAILED(m_pShader->Bind_RawValue("g_vSourceLutWeights",&weights,sizeof(weights))) ||
			FAILED(m_pShader->Bind_RawValue("g_fSourceNeutralLutWeight",&neutralWeight,sizeof(neutralWeight))) ||
			FAILED(m_pShader->Bind_RawValue("g_vSourceGradingParameters",parameters,sizeof(parameters))) ||
			FAILED(m_pShader->Begin(ETOUI(inputIndex == 0u ? DEFERRED::SOURCE_LUT_BAKE_NEUTRAL : DEFERRED::SOURCE_LUT_BAKE))) ||
			FAILED(m_pVIBuffer->Bind_Resources()) || FAILED(m_pVIBuffer->Render())) result=E_FAIL;
		else result=m_pDevice->GetDeviceRemovedReason();
		ID3D11ShaderResourceView* empty[4]{};
		if(FAILED(m_pShader->Bind_Textures("g_SourceLutInputs",empty,4u)))result=E_FAIL;
	}
	if(FAILED(result))return result;
	output=std::move(staged);
	return S_OK;
}

HRESULT CRenderer::Apply_RenderQualitySettings(
	const RENDER_QUALITY_SETTINGS& Settings)
{
	if (!IsValidRenderQualitySettings(Settings))
		return E_INVALIDARG;

	ComPtr<ID3D11ShaderResourceView> stagedLut;
	if (Settings.SourcePostProcess.bEnabled)
	{
		for (auto entry = m_SourceGradingCache.begin(); entry != m_SourceGradingCache.end(); ++entry)
			if (entry->fGamma == Settings.fGamma && SameSourceGrading(entry->Settings, Settings.SourcePostProcess))
			{
				stagedLut = entry->pTexture;
				auto recent = *entry;
				m_SourceGradingCache.erase(entry);
				m_SourceGradingCache.push_back(std::move(recent));
				break;
			}
		if (!stagedLut)
		{
			const HRESULT result = Stage_SourceGradingLut(Settings.SourcePostProcess,
				Settings.fGamma, stagedLut);
			if (FAILED(result)) return result;
			// Keep the prior resource available for profile/region rollback. This
			// bounded cache retains immutable source pixels along with each key.
			m_SourceGradingCache.push_back({Settings.SourcePostProcess, Settings.fGamma, stagedLut});
			if (m_SourceGradingCache.size() > 8u) m_SourceGradingCache.erase(m_SourceGradingCache.begin());
		}
	}
	m_pSourceGradingLut = stagedLut;
	m_RenderQualitySettings = Settings;
	return S_OK;
}

namespace
{
	bool_t IsValidHeightFogSettings(const HEIGHT_FOG_SETTINGS& Settings)
	{
		const auto finite = [](const f32_t value) { return std::isfinite(value); };
		return finite(Settings.vColor.x) && finite(Settings.vColor.y) &&
			finite(Settings.vColor.z) && finite(Settings.vColor.w) &&
			Settings.vColor.x >= 0.f && Settings.vColor.y >= 0.f &&
			Settings.vColor.z >= 0.f &&
			finite(Settings.fDensity) && Settings.fDensity >= 0.f &&
			finite(Settings.fHeightFalloff) && Settings.fHeightFalloff > 0.f &&
			finite(Settings.fTopHeight) &&
			finite(Settings.fStartDistance) && Settings.fStartDistance >= 0.f &&
			finite(Settings.fMaximumOpacity) &&
			Settings.fMaximumOpacity >= 0.f && Settings.fMaximumOpacity <= 1.f &&
			finite(Settings.fDriftSpeed) && Settings.fDriftSpeed >= 0.f &&
			finite(Settings.fDriftHeightAmplitude) &&
			Settings.fDriftHeightAmplitude >= 0.f &&
			finite(Settings.fDriftDensityAmplitude) &&
			Settings.fDriftDensityAmplitude >= 0.f &&
			finite(Settings.fCoveragePercent) &&
			Settings.fCoveragePercent >= 0.f &&
			Settings.fCoveragePercent <= 1.f &&
			finite(Settings.fWindDirectionX) &&
			finite(Settings.fWindDirectionZ) &&
			finite(Settings.fWindSpeed) && Settings.fWindSpeed >= 0.f &&
			finite(Settings.fPatchScale) && Settings.fPatchScale > 0.f &&
			finite(Settings.fPatchSoftness) &&
			Settings.fPatchSoftness > 0.f &&
			Settings.fPatchSoftness <= 0.5f &&
            finite(Settings.vInscatteringColor.x) && Settings.vInscatteringColor.x >= 0.f &&
            finite(Settings.vInscatteringColor.y) && Settings.vInscatteringColor.y >= 0.f &&
            finite(Settings.vInscatteringColor.z) && Settings.vInscatteringColor.z >= 0.f &&
            finite(Settings.vFogLightDirection.x) && finite(Settings.vFogLightDirection.y) &&
            finite(Settings.vFogLightDirection.z) && finite(Settings.vFogLightDirection.w) &&
            Settings.vFogLightDirection.w >= -1.f && Settings.vFogLightDirection.w <= 1.f &&
            (!Settings.bSourceExponential ||
                Settings.vFogLightDirection.x * Settings.vFogLightDirection.x +
                Settings.vFogLightDirection.y * Settings.vFogLightDirection.y +
                Settings.vFogLightDirection.z * Settings.vFogLightDirection.z > 0.000001f);
	}
}

HRESULT CRenderer::Stage_RenderEnvironment(const wstring_t& cubePath,
    const float4_t& color, const float4_t& rotationIntensity,
    RENDER_ENVIRONMENT_STATE& outState, bool_t forceReload) const
{
    const float values[] = {color.x, color.y, color.z, color.w,
        rotationIntensity.x, rotationIntensity.y, rotationIntensity.z, rotationIntensity.w};
    for (float value : values) if (!std::isfinite(value)) return E_INVALIDARG;
    if (color.x < 0.f || color.y < 0.f || color.z < 0.f || color.w < 0.f ||
        color.x > 64.f || color.y > 64.f || color.z > 64.f || color.w > 64.f ||
        rotationIntensity.z < 0.f || rotationIntensity.z > 64.f ||
        rotationIntensity.w != 0.f ||
        std::abs(rotationIntensity.x * rotationIntensity.x +
            rotationIntensity.y * rotationIntensity.y - 1.f) > .001f) return E_INVALIDARG;
    RENDER_ENVIRONMENT_STATE staged;
    staged.strCubePath = cubePath;
    staged.vColor = color;
    staged.vRotationIntensity = rotationIntensity;
    if (!cubePath.empty() && !forceReload && cubePath == m_RenderEnvironment.strCubePath &&
        m_RenderEnvironment.pCube)
    {
        // Quality edits and mood changes in the same scene retain its staged cube.
        staged.pCube = m_RenderEnvironment.pCube;
    }
    else if (!cubePath.empty())
    {
        if (FAILED(CreateDDSTextureFromFileEx(m_pDevice.Get(), cubePath.c_str(), 0,
            D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
            DDS_LOADER_DEFAULT, nullptr, staged.pCube.GetAddressOf()))) return E_FAIL;
        D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
        staged.pCube->GetDesc(&desc);
        if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE ||
            desc.TextureCube.MipLevels == 0u ||
            (desc.Format != DXGI_FORMAT_BC1_UNORM && desc.Format != DXGI_FORMAT_BC2_UNORM &&
             desc.Format != DXGI_FORMAT_BC3_UNORM && desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM &&
             desc.Format != DXGI_FORMAT_B8G8R8A8_UNORM))
            return E_INVALIDARG;
    }
    outState = std::move(staged);
    return S_OK;
}

void CRenderer::Commit_RenderEnvironment(const RENDER_ENVIRONMENT_STATE& state)
{
    m_RenderEnvironment = state;
}

RENDER_ENVIRONMENT_STATE CRenderer::Get_RenderEnvironment() const
{
    return m_RenderEnvironment;
}

HRESULT CRenderer::Bind_HeightFog(CShader* shader) const
{
    if (!shader) return E_INVALIDARG;
	const uint32_t iFogEnabled = m_HeightFogSettings.bEnabled ? 1u : 0u;
	const float2_t vFogWind(
		m_HeightFogSettings.fWindDirectionX,
		m_HeightFogSettings.fWindDirectionZ);
    const uint32_t sourceModel = m_HeightFogSettings.bSourceExponential ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_iSourceExponentialFog", &sourceModel, sizeof(sourceModel))) ||
        FAILED(shader->Bind_RawValue("g_vFogInscatteringColor", &m_HeightFogSettings.vInscatteringColor, sizeof(float4_t))) ||
        FAILED(shader->Bind_RawValue("g_vFogLightDirection", &m_HeightFogSettings.vFogLightDirection, sizeof(float4_t))) ||
		FAILED(shader->Bind_RawValue("g_iHeightFogEnabled",
			&iFogEnabled, sizeof(iFogEnabled))) ||
		FAILED(shader->Bind_RawValue("g_vHeightFogColor",
			&m_HeightFogSettings.vColor, sizeof(float4_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDensity",
			&m_HeightFogSettings.fDensity, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogFalloff",
			&m_HeightFogSettings.fHeightFalloff, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogTopHeight",
			&m_HeightFogSettings.fTopHeight, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogStartDistance",
			&m_HeightFogSettings.fStartDistance, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogMaximumOpacity",
			&m_HeightFogSettings.fMaximumOpacity, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDriftSpeed",
			&m_HeightFogSettings.fDriftSpeed, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDriftHeight",
			&m_HeightFogSettings.fDriftHeightAmplitude, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDriftDensity",
			&m_HeightFogSettings.fDriftDensityAmplitude, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fFogCoverage",
			&m_HeightFogSettings.fCoveragePercent, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_vFogWindDirection",
			&vFogWind, sizeof(vFogWind))) ||
		FAILED(shader->Bind_RawValue("g_fFogWindSpeed",
			&m_HeightFogSettings.fWindSpeed, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fFogPatchScale",
			&m_HeightFogSettings.fPatchScale, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fFogPatchSoftness",
			&m_HeightFogSettings.fPatchSoftness, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fPresentationClock",
			&m_fPresentationClock, sizeof(m_fPresentationClock))))
	{
		return E_FAIL;
	}
    return S_OK;
}

HRESULT CRenderer::Apply_HeightFog(const HEIGHT_FOG_SETTINGS& Settings)
{
	if (!IsValidHeightFogSettings(Settings))
		return E_INVALIDARG;

	m_HeightFogSettings = Settings;
	return S_OK;
}

void CRenderer::Advance_PresentationClock(f32_t fTimeDelta)
{
	if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
		return;

	/* Wrapping keeps the drift phase exact after long sessions instead of
	   letting float precision quantise the sine input. */
	constexpr f32_t CLOCK_WRAP_SECONDS = 3600.f;
	m_fPresentationClock = fmodf(
		m_fPresentationClock + fTimeDelta, CLOCK_WRAP_SECONDS);
}

HRESULT CRenderer::Draw()
{
    // Priority sky and later forward water share this frame clock.
    CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	auto FailFrame = [this, &Presentation](
		const char* stage, const HRESULT hResult) -> HRESULT
	{
		WriteRendererFailure(stage, hResult);
        CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);
		m_bSceneColorSnapshotRequested = false;
        m_bSceneEnvironmentReplaced = false;
		Presentation.Clear_Frame();
		for (auto& RenderGroup : m_RenderObjects)
			RenderGroup.clear();
		return FAILED(hResult) ? hResult : E_FAIL;
	};
	CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
	CProfilerScope drawScope(pProfiler, "Render.Draw");
	CProfilerGpuScope gpuDrawScope(pProfiler, "Render.Draw");
	HRESULT hResult = S_OK;
	{
		CProfilerScope scope(pProfiler, "Render.SubmitFrameProviders");
		hResult = Presentation.Submit_FrameProviders();
	}
	if (FAILED(hResult))
		return FailFrame("Submit_FrameProviders", hResult);
	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	if (vViewportSize.x <= 0.f || vViewportSize.y <= 0.f)
	{
		return FailFrame("Viewport", E_INVALIDARG);
	}
	hResult = Ready_ScenePostTargets(
		static_cast<uint32_t>(vViewportSize.x),
		static_cast<uint32_t>(vViewportSize.y));
	if (FAILED(hResult))
		return FailFrame("Ready_ScenePostTargets", hResult);
	{
		CProfilerScope scope(pProfiler, "Render.Shadow");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Shadow", true);
		hResult = Render_Shadow();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Shadow", hResult);
	{
		CProfilerScope scope(pProfiler, "Render.NonBlend");
		CProfilerGpuScope gpuScope(pProfiler, "Render.NonBlend", true);
		hResult = Render_NonBlend();
	}
	if (FAILED(hResult))
		return FailFrame("Render_NonBlend", hResult);
	if (m_RenderQualitySettings.bSSAOEnabled)
	{
		CProfilerScope scope(pProfiler, "Render.SSAO");
		CProfilerGpuScope gpuScope(pProfiler, "Render.SSAO", true);
		hResult = Render_SSAO();
		if (FAILED(hResult))
			return FailFrame("Render_SSAO", hResult);
	}
	{
		CProfilerScope scope(pProfiler, "Render.Lights");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Lights", true);
		hResult = Render_Lights();
	}
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
		CProfilerScope scope(pProfiler, "Render.SceneHDR");
		CProfilerGpuScope gpuScope(pProfiler, "Render.SceneHDR");
		hSceneResult = Render_Priority();
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_Combined();
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_NonLight();
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_SceneReplacements();
		if (SUCCEEDED(hSceneResult) && m_bSceneColorSnapshotRequested)
		{
			hSceneResult = Capture_SceneColorSnapshot();
		}
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_Blend();

		/* Always restore the back-buffer/DSV pair after entering the HDR MRT. */
		hEndSceneResult = CGameInstance::Get().End_MRT();
	}
	if (FAILED(hSceneResult) || FAILED(hEndSceneResult))
		return FailFrame(
			FAILED(hSceneResult) ? "Render_Scene" : "End_MRT_SceneHDR",
			FAILED(hSceneResult) ? hSceneResult : hEndSceneResult);

	{
		CProfilerScope scope(pProfiler, "Render.ScreenPosts");
		CProfilerGpuScope gpuScope(pProfiler, "Render.ScreenPosts");
		hResult = Render_ScreenPosts();
	}
	if (FAILED(hResult))
		return FailFrame("Render_ScreenPosts", hResult);

	if (m_RenderQualitySettings.bBloomEnabled)
	{
		CProfilerScope scope(pProfiler, "Render.Bloom");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Bloom");
		hResult = Render_Bloom();
		if (FAILED(hResult))
			return FailFrame("Render_Bloom", hResult);
	}

	/* The one and only place tone mapping and gamma are applied. */
	{
		CProfilerScope scope(pProfiler, "Render.Final");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Final");
		hResult = Render_Final();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Final", hResult);

	{
		CProfilerScope scope(pProfiler, "Render.DisplayOverlays");
		CProfilerGpuScope gpuScope(pProfiler, "Render.DisplayOverlays");
		hResult = Render_DisplayOverlays();
	}
	if (FAILED(hResult))
		return FailFrame("Render_DisplayOverlays", hResult);

	/* UI is authored in display space, so it stays out of the HDR target. */
	{
		CProfilerScope scope(pProfiler, "Render.UI");
		CProfilerGpuScope gpuScope(pProfiler, "Render.UI", true);
		hResult = Render_UI();
	}
	if (FAILED(hResult))
		return FailFrame("Render_UI", hResult);

#ifdef _DEBUG
	{
		CProfilerScope scope(pProfiler, "Render.Debug");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Debug");
		hResult = Render_Debug();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Debug", hResult);
#endif

	m_bSceneColorSnapshotRequested = false;
    m_bSceneEnvironmentReplaced = false;
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
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.Priority");
    CProfilerGpuScope gpuScope(profiler, "Render.Priority");
	// Index iteration permits callbacks to append without retaining vector
	// iterators or element references; the queue still owns every object.
	for (size_t renderIndex = 0; renderIndex < m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)].size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)][renderIndex].get();
		if (nullptr != pRenderObject)
			pRenderObject->Render_Group(RENDERGROUP::PRIORITY);
	}

	m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_Shadow()
{
    auto& objects = m_RenderObjects[ETOUI(RENDERGROUP::SHADOW)];
    if (!m_pShadowDSV || !m_pShadowSRV || !m_pShadowDepthTexture || !m_iShadowMapSize)
    {
        m_bStaticShadowCacheValid = false;
        objects.clear();
        return E_FAIL;
    }

    auto& game = CGameInstance::Get();
    const auto* view = game.Get_ShadowLightTransform(D3DTS::VIEW);
    const auto* projection = game.Get_ShadowLightTransform(D3DTS::PROJ);
    const bool enabled = game.Is_ShadowLightEnabled();
    const bool sourceMaterials = m_MaterialRenderSettings.bUseSourceMaterials;
    const bool allowCache = enabled && view && projection &&
        m_pStaticShadowDepthTexture && !m_bSceneEnvironmentReplaced;
    m_CandidateStaticShadowCasters.clear();
    m_StaticShadowFlags.assign(objects.size(), 0u);
    if (allowCache)
    {
        CProfilerScope scope(game.Get_Profiler(), "Render.Shadow.CacheAdmission");
        for (size_t i = 0; i < objects.size(); ++i)
        {
            uint64_t revision = 0u;
            if (objects[i] && objects[i]->Try_GetStaticShadowRevision(revision) && revision)
            {
                m_StaticShadowFlags[i] = 1u;
                m_CandidateStaticShadowCasters.push_back({objects[i], objects[i].get(), revision});
            }
        }
    }
    const bool hasStatic = !m_CandidateStaticShadowCasters.empty();
    bool reuse = hasStatic && m_bStaticShadowCacheValid &&
        sourceMaterials == m_bStaticShadowSourceMaterials &&
        0 == std::memcmp(view, &m_StaticShadowView, sizeof(*view)) &&
        0 == std::memcmp(projection, &m_StaticShadowProjection, sizeof(*projection)) &&
        m_CandidateStaticShadowCasters.size() == m_StaticShadowCasters.size();
    for (size_t i = 0; reuse && i < m_StaticShadowCasters.size(); ++i)
    {
        const auto& previous = m_StaticShadowCasters[i];
        const auto& current = m_CandidateStaticShadowCasters[i];
        reuse = previous.Identity == current.Identity && previous.Revision == current.Revision &&
            !previous.Owner.owner_before(current.Owner) && !current.Owner.owner_before(previous.Owner);
    }
    if (!reuse) m_bStaticShadowCacheValid = false;
    if (!hasStatic) m_StaticShadowCasters.clear();

    D3D11_VIEWPORT originalViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
    UINT viewportCount = _countof(originalViewports);
    m_pContext->RSGetViewports(&viewportCount, originalViewports);
    if (FAILED(game.Begin_DepthOnly(m_pShadowDSV)))
    {
        m_bStaticShadowCacheValid = false;
        objects.clear();
        return E_FAIL;
    }

    // Keep the existing target manager's saved output pair. Only unbind the
    // depth target for CopyResource; beginning another pass would clear it.
    const auto copyDepth = [&](ID3D11Texture2D* destination, ID3D11Texture2D* source)
    {
        CProfilerScope scope(game.Get_Profiler(), "Render.Shadow.CacheCopy");
        m_pContext->OMSetRenderTargets(0u, nullptr, nullptr);
        m_pContext->CopyResource(destination, source);
        m_pContext->OMSetRenderTargets(0u, nullptr, m_pShadowDSV.Get());
    };
    const auto drawObjects = [&](bool staticPass) -> HRESULT
    {
        for (size_t i = 0; i < objects.size(); ++i)
        {
            const bool isStatic = i < m_StaticShadowFlags.size() && m_StaticShadowFlags[i] != 0u;
            if (isStatic != staticPass) continue;
            // Hold this object across a callback that may append to the queue.
            const auto object = objects[i];
            if (object && FAILED(object->Render_Shadow()))
            {
                WriteRendererFailure("Render_Shadow_Object", E_FAIL, typeid(*object).name());
                return E_FAIL;
            }
        }
        return S_OK;
    };

    HRESULT result = S_OK;
    if (enabled)
    {
        SetUp_ViewportDesc(m_iShadowMapSize, m_iShadowMapSize);
        if (reuse)
            copyDepth(m_pShadowDepthTexture.Get(), m_pStaticShadowDepthTexture.Get());
        else if (hasStatic)
        {
            CProfilerScope scope(game.Get_Profiler(), "Render.Shadow.StaticBuild");
            CProfilerGpuScope gpuScope(game.Get_Profiler(), "Render.Shadow.StaticBuild");
            result = drawObjects(true);
            if (SUCCEEDED(result))
            {
                copyDepth(m_pStaticShadowDepthTexture.Get(), m_pShadowDepthTexture.Get());
                m_StaticShadowCasters.swap(m_CandidateStaticShadowCasters);
                m_StaticShadowView = *view;
                m_StaticShadowProjection = *projection;
                m_bStaticShadowSourceMaterials = sourceMaterials;
                m_bStaticShadowCacheValid = true;
            }
        }
        if (SUCCEEDED(result))
        {
            CProfilerScope scope(game.Get_Profiler(), "Render.Shadow.Dynamic");
            CProfilerGpuScope gpuScope(game.Get_Profiler(), "Render.Shadow.Dynamic");
            result = drawObjects(false);
        }
    }
    objects.clear();
    const HRESULT endResult = game.End_DepthOnly();
    m_pContext->RSSetViewports(viewportCount, viewportCount ? originalViewports : nullptr);
    if (FAILED(result) || FAILED(endResult))
    {
        m_bStaticShadowCacheValid = false;
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
    CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);
	/* Diffuse + Normal */
	if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_GameObject"))))
		return E_FAIL;

	auto& NonBlendObjects =
		m_RenderObjects[ETOUI(RENDERGROUP::NONBLEND)];
	auto& DeferredOverlayObjects =
		m_RenderObjects[ETOUI(RENDERGROUP::DEFERRED_OVERLAY)];
	for (size_t renderIndex = 0; renderIndex < NonBlendObjects.size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = NonBlendObjects[renderIndex].get();
		if (nullptr != pRenderObject)
			pRenderObject->Render_Group(RENDERGROUP::NONBLEND);
	}

	/* Deferred overlays must run after every opaque object while the complete
	   game-object MRT, including Target_Emissive, is still bound. */
	for (size_t renderIndex = 0; renderIndex < DeferredOverlayObjects.size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = DeferredOverlayObjects[renderIndex].get();
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
        FAILED(CGameInstance::Get().Bind_RT_SRV(
            TEXT("Target_CharacterGeometry"), m_pShader, "g_CharacterGeometryTexture")) ||
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
    const uint32_t noSourceCharacter = 0u;
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
    if (FAILED(m_pShader->Bind_RawValue("g_SourceCharacterProgram", &noSourceCharacter, sizeof(noSourceCharacter))) ||
        FAILED(m_pShader->Bind_RawValue("g_SourceCharacterRow", &noSourceCharacter, sizeof(noSourceCharacter))) ||
        FAILED(m_pShader->Bind_Matrix("g_SourceCharacterViewMatrix", CGameInstance::Get().Get_Transform(D3DTS::VIEW))) ||
        FAILED(m_pShader->Bind_Matrix("g_SourceCharacterProjMatrix", CGameInstance::Get().Get_Transform(D3DTS::PROJ))) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Emissive"), m_pShader, "g_EmissiveTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterSurface"), m_pShader, "g_CharacterSurfaceTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterGeometry"), m_pShader, "g_CharacterGeometryTexture")) ||
        FAILED(hBindAO) ||
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
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_MaterialSpecular"), m_pShader, "g_MaterialSpecularTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_PickPos"), m_pShader, "g_GeometricNormalTexture")) ||
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

    // The ordinary pass skips marker-5 pixels. Only materials submitted by
    // visible mesh draws get a source light pass; prototypes never enter here.
    const uint32_t sourceCount = CMaterial::Get_SourceCharacterFrameCount();
    if (SUCCEEDED(hResult) && sourceCount != 0u)
    {
        // Effect outlines own the scene stencil. Only this separate DSV receives
        // the marker-5 mask, and the original light targets are restored on failure.
        ID3D11RenderTargetView* rawTargets[2]{};
        ComPtr<ID3D11DepthStencilView> originalDepth;
        m_pContext->OMGetRenderTargets(2u, rawTargets, originalDepth.GetAddressOf());
        ComPtr<ID3D11RenderTargetView> targets[2];
        for (uint32_t index = 0u; index < 2u; ++index) targets[index].Attach(rawTargets[index]);
        D3D11_TEXTURE2D_DESC targetDesc{};
        if (targets[0])
        {
            ComPtr<ID3D11Resource> resource;
            ComPtr<ID3D11Texture2D> texture;
            targets[0]->GetResource(resource.GetAddressOf());
            if (resource && SUCCEEDED(resource.As(&texture))) texture->GetDesc(&targetDesc);
        }
        bool_t useSourceMask = false;
        if (targetDesc.SampleDesc.Count == 1u &&
            S_OK == Ready_SourceLightMask(targetDesc.Width, targetDesc.Height))
        {
            m_pContext->ClearDepthStencilView(m_pSourceLightMaskDSV.Get(), D3D11_CLEAR_STENCIL, 1.f, 0u);
            m_pContext->OMSetRenderTargets(0u, nullptr, m_pSourceLightMaskDSV.Get());
            HRESULT maskResult = m_pShader->Begin(ETOUI(DEFERRED::SOURCE_LIGHT_MASK));
            if (SUCCEEDED(maskResult)) maskResult = m_pVIBuffer->Render();
            useSourceMask = SUCCEEDED(maskResult);
            if (!useSourceMask)
            {
                m_iSourceLightMaskFailedWidth = targetDesc.Width;
                m_iSourceLightMaskFailedHeight = targetDesc.Height;
                WriteRendererFailure("SourceLightMask_DrawFallback", maskResult);
            }
            m_pContext->OMSetRenderTargets(2u, rawTargets,
                useSourceMask ? m_pSourceLightMaskDSV.Get() : originalDepth.Get());
        }
        for (uint32_t index = 0u; SUCCEEDED(hResult) && index < sourceCount; ++index)
        {
            if (FAILED(CMaterial::Bind_SourceCharacterLight(m_pShader, index)) ||
                FAILED(CGameInstance::Get().Render_Lights(m_pShader, m_pVIBuffer, bShadowEnabled,
                    LIGHT_RECEIVER::SOURCE_CHARACTER, useSourceMask))) hResult = E_FAIL;
        }
        m_pContext->OMSetRenderTargets(2u, rawTargets, originalDepth.Get());
    }
    if (FAILED(m_pShader->Bind_RawValue("g_SourceCharacterProgram", &noSourceCharacter, sizeof(noSourceCharacter))) ||
        FAILED(m_pShader->Bind_RawValue("g_SourceCharacterRow", &noSourceCharacter, sizeof(noSourceCharacter))))
        hResult = E_FAIL;
    CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);

	/* Always restore the back buffer even when a light bind or draw fails. */
	if (FAILED(CGameInstance::Get().End_MRT()))
		hResult = E_FAIL;

	return hResult;
}

HRESULT CRenderer::Render_Combined()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.Combined");
    CProfilerGpuScope gpuScope(profiler, "Render.Combined");
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

	/* Height fog reuses the world position the combine step can rebuild from
	   the depth target, so it needs the same inverse matrices and camera the
	   lighting pass already binds. */
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_RawValue("g_vCamPosition",
			CGameInstance::Get().Get_CamPosition(), sizeof(float4_t))) ||
		FAILED(Bind_HeightFog(m_pShader.get()))) return E_FAIL;

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
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.NonLight");
    CProfilerGpuScope gpuScope(profiler, "Render.NonLight");
	HRESULT hFirstFailure = S_OK;
	for (size_t renderIndex = 0; renderIndex < m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)].size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)][renderIndex].get();
		if (nullptr != pRenderObject)
		{
			const HRESULT hResult = pRenderObject->Render_Group(RENDERGROUP::NONLIGHT);
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

HRESULT CRenderer::Refresh_SceneColorSnapshot()
{
	if (CRenderOutputContract::Get_Active() !=
			RENDER_OUTPUT_CONTRACT::SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION ||
		!CRenderOutputContract::Matches_ActiveRenderTargets(m_pContext.Get()))
		return E_INVALIDARG;
	return Capture_SceneColorSnapshot();
}

HRESULT CRenderer::Capture_SceneColorSnapshot()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope snapshotScope(profiler, "Render.SceneColorSnapshot");
	const auto sourceSRV = CGameInstance::Get().Get_RT_SRV(TEXT("Target_SceneHDR"));
	const auto snapshotSRV = CGameInstance::Get().Get_RT_SRV(TEXT("Target_EffectSceneColor"));
	if (!sourceSRV || !snapshotSRV)
		return E_FAIL;
	ComPtr<ID3D11Resource> sourceResource, snapshotResource;
	sourceSRV->GetResource(sourceResource.GetAddressOf());
	snapshotSRV->GetResource(snapshotResource.GetAddressOf());
	ComPtr<ID3D11Texture2D> sourceTexture, snapshotTexture;
	if (!sourceResource || !snapshotResource || sourceResource.Get() == snapshotResource.Get() ||
		FAILED(sourceResource.As(&sourceTexture)) || FAILED(snapshotResource.As(&snapshotTexture)))
		return E_FAIL;
	const auto bloomSnapshotSRV = CGameInstance::Get().Get_RT_SRV(TEXT("Target_EffectSceneBloom"));
	ComPtr<ID3D11Resource> bloomSnapshotResource;
	ComPtr<ID3D11Texture2D> bloomSnapshotTexture;
	if (!bloomSnapshotSRV) return E_FAIL;
	bloomSnapshotSRV->GetResource(bloomSnapshotResource.GetAddressOf());
	if (!bloomSnapshotResource || FAILED(bloomSnapshotResource.As(&bloomSnapshotTexture))) return E_FAIL;
	D3D11_TEXTURE2D_DESC sourceDesc{}, snapshotDesc{};
	sourceTexture->GetDesc(&sourceDesc);
	snapshotTexture->GetDesc(&snapshotDesc);
	if (sourceDesc.Width != snapshotDesc.Width || sourceDesc.Height != snapshotDesc.Height ||
		sourceDesc.Format != snapshotDesc.Format || sourceDesc.MipLevels != snapshotDesc.MipLevels ||
		sourceDesc.ArraySize != snapshotDesc.ArraySize ||
		sourceDesc.SampleDesc.Count != snapshotDesc.SampleDesc.Count ||
		sourceDesc.SampleDesc.Quality != snapshotDesc.SampleDesc.Quality)
		return E_INVALIDARG;

	// Copy outside the output binding, then restore all MRTs and the same DSV.
	// Begin_MRT would clear the accumulated scene, so it must not be used here.
	ID3D11RenderTargetView* outputs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	ComPtr<ID3D11DepthStencilView> depth;
	m_pContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		outputs, depth.GetAddressOf());
	ComPtr<ID3D11Resource> activeSceneResource;
	if (outputs[0]) outputs[0]->GetResource(activeSceneResource.GetAddressOf());
	if (activeSceneResource.Get() != sourceResource.Get())
	{
		for (auto* output : outputs)
			if (output) output->Release();
		return E_INVALIDARG;
	}
	m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
	ID3D11ShaderResourceView* emptySRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, emptySRVs);
    HRESULT result = E_FAIL;
    {
        CProfilerGpuScope copyScope(profiler, "Render.SceneColorCopy");
        result = CGameInstance::Get().Copy_RT_Resource(
            TEXT("Target_SceneHDR"), snapshotTexture);
        if (SUCCEEDED(result))
            result = CGameInstance::Get().Copy_RT_Resource(TEXT("Target_SceneBloom"), bloomSnapshotTexture);
    }
    if (SUCCEEDED(result) && profiler)
    {
        profiler->Add_Counter(EProfilerCounter::SceneColorCopies, 2u);
        // Both named targets are created as RGBA16F. Count the copied logical
        // payload, not read+write bus traffic or driver allocation overhead.
        uint64_t pixels = 0;
        uint32_t width = sourceDesc.Width;
        uint32_t height = sourceDesc.Height;
        for (uint32_t mip = 0; mip < sourceDesc.MipLevels; ++mip)
        {
            pixels += static_cast<uint64_t>(width) * height;
            width = (std::max)(1u, width / 2);
            height = (std::max)(1u, height / 2);
        }
        profiler->Add_Counter(EProfilerCounter::SceneColorCopyBytes,
            pixels * 16u * sourceDesc.ArraySize * sourceDesc.SampleDesc.Count);
    }
	m_pContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		outputs, depth.Get());
	for (auto* output : outputs)
		if (output) output->Release();
	return result;
}

HRESULT CRenderer::Render_Blend()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.Blend");
    CProfilerGpuScope gpuScope(profiler, "Render.Blend", true);
	HRESULT hFirstFailure = S_OK;

	/* Translucent surfaces have to be drawn far to near or they overwrite each
	   other in submission order, which is what made overlapping water sheets
	   and their props flip depending on which placement happened to be added
	   first. The queue itself is left alone; only the draw order is sorted, so
	   nothing else that walks the render group sees a different sequence.

	   An object may request an earlier slot through Get_BlendSortPriority():
	   lower priorities draw first and equal priorities keep far-to-near order.
	   Without a camera every distance ties, so only the priority reorders and
	   stable_sort keeps the submission order.

	   The scratch buffer is a function-local static because rendering runs on
	   one thread and the alternative is a heap allocation every frame. */
	struct BLEND_ENTRY final
	{
		Engine::BLEND_SORT_KEY key;
		CGameObject* pObject;
	};
	static vector<BLEND_ENTRY> SortedBlendObjects;
	SortedBlendObjects.clear();
	auto& BlendObjects = m_RenderObjects[ETOUI(RENDERGROUP::BLEND)];
	SortedBlendObjects.reserve(BlendObjects.size());
	const float4_t* pCamPosition = CGameInstance::Get().Get_CamPosition();
	const vector_t vCamera =
		nullptr != pCamPosition ? XMLoadFloat4(pCamPosition) : XMVectorZero();
	for (size_t renderIndex = 0; renderIndex < BlendObjects.size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = BlendObjects[renderIndex].get();
		if (nullptr == pRenderObject)
			continue;
		/* No transform is not an error here: the object still has to draw,
		   it just cannot be placed in the ordering, so it goes first. */
		f32_t fDistanceSquared = FLT_MAX;
		if (nullptr != pCamPosition)
		{
			const shared_ptr<CTransform> pTransform =
				dynamic_pointer_cast<CTransform>(
					pRenderObject->Get_Component(g_strTransformComTag));
			if (nullptr != pTransform)
			{
				const vector_t vDelta =
					pTransform->Get_State(STATE::POSITION) - vCamera;
				fDistanceSquared =
					XMVectorGetX(XMVector3LengthSq(vDelta));
				if (!std::isfinite(fDistanceSquared))
					fDistanceSquared = FLT_MAX;
			}
		}
		SortedBlendObjects.push_back(
			{ { pRenderObject->Get_BlendSortPriority(), fDistanceSquared }, pRenderObject });
	}
	std::stable_sort(
		SortedBlendObjects.begin(), SortedBlendObjects.end(),
		[](const BLEND_ENTRY& lhs, const BLEND_ENTRY& rhs)
		{
			return Engine::BlendSortBefore(lhs.key, rhs.key);
		});
	for (const BLEND_ENTRY& Entry : SortedBlendObjects)
	{
		const HRESULT hResult = Entry.pObject->Render_Group(RENDERGROUP::BLEND);
		if (FAILED(hResult) && SUCCEEDED(hFirstFailure))
		{
			WriteRendererFailure(
				"Render_Blend_Object",
				hResult,
				typeid(*Entry.pObject).name());
			hFirstFailure = hResult;
		}
	}
	BlendObjects.clear();
	SortedBlendObjects.clear();
	return hFirstFailure;
}

HRESULT CRenderer::Render_ScreenPostPass(
	ComPtr<ID3D11ShaderResourceView> pSourceSRV,
	ComPtr<ID3D11RenderTargetView> pDestinationRTV,
	const uint32_t iPassIndex,
	ComPtr<ID3D11ShaderResourceView> pBloomSourceSRV,
	ComPtr<ID3D11RenderTargetView> pBloomDestinationRTV,
	const PRESENTATION_SCREEN_POST_DESC* pPostDesc)
{
	if (nullptr == pDestinationRTV || nullptr == pBloomDestinationRTV)
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fSceneBloomIntensity", &m_RenderQualitySettings.fBloomIntensity, sizeof(float))) ||
		FAILED(m_pShader->Bind_RawValue("g_fEffectBloomThreshold", &m_RenderQualitySettings.fBloomThreshold, sizeof(float))) ||
		FAILED(m_pShader->Bind_RawValue("g_fEffectBloomSoftKnee", &m_RenderQualitySettings.fBloomSoftKnee, sizeof(float))))
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
	ID3D11RenderTargetView* destinations[3] = {
		pDestinationRTV.Get(), nullptr, pBloomDestinationRTV.Get() };
	m_pContext->OMSetRenderTargets(3u, destinations, nullptr);
	const float4_t vClear{};
	m_pContext->ClearRenderTargetView(destinations[0], &vClear.x);
	m_pContext->ClearRenderTargetView(destinations[2], &vClear.x);

	if (nullptr != pPostDesc && nullptr != pPostDesc->pMaterial)
	{
		PRESENTATION_SCREEN_POST_MATERIAL_INPUT Input;
		Input.pSceneColor = pSourceSRV;
		Input.pSceneBloom = pBloomSourceSRV;
		Input.pSceneDepth = CGameInstance::Get().Get_RT_SRV(TEXT("Target_Depth"));
		Input.World = m_WorldMatrix;
		Input.View = m_ViewMatrix;
		Input.Projection = m_ProjMatrix;
		if (nullptr == Input.pSceneColor || nullptr == Input.pSceneBloom || nullptr == Input.pSceneDepth ||
			FAILED(pPostDesc->pMaterial->Bind(Input)) ||
			FAILED(m_pVIBuffer->Bind_Resources()) || FAILED(m_pVIBuffer->Render()))
			return E_FAIL;
		return S_OK;
	}
	if (nullptr == pPostDesc)
	{
		if (FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_SceneHDR"), m_pShader,
				"g_SceneHDRTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_Distortion"), m_pShader,
				"g_DistortionTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_PickPos"), m_pShader, "g_GeometricNormalTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_SceneBloom"), m_pShader, "g_SceneBloomTexture")))
		{
			return E_FAIL;
		}
	}
	else
	{
		if (nullptr == pSourceSRV || nullptr == pBloomSourceSRV ||
			FAILED(m_pShader->Bind_Texture("g_PostBloomTexture", pBloomSourceSRV)) ||
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

HRESULT CRenderer::Render_SceneReplacements()
{
    const auto& posts = CPresentation_Manager::Get().Get_ScreenPosts();
    const bool hasReplacement = std::any_of(posts.begin(), posts.end(), [](const auto& post) {
        return post.pMaterial && post.pMaterial->Replaces_SceneBeforeBlend();
    });
    if (!hasReplacement) return S_OK;
    auto color = CGameInstance::Get().Get_RT_SRV(TEXT("Target_SceneHDR"));
    auto bloom = CGameInstance::Get().Get_RT_SRV(TEXT("Target_SceneBloom"));
    if (!color || !bloom) return E_FAIL;
    ID3D11RenderTargetView* saved[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
    ID3D11DepthStencilView* depth = nullptr;
    m_pContext->OMGetRenderTargets(_countof(saved), saved, &depth);
    HRESULT result = S_OK;
    for (const auto& post : posts)
    {
        if (!post.pMaterial || !post.pMaterial->Replaces_SceneBeforeBlend()) continue;
        result = Render_ScreenPostPass(color, m_pScenePostRTVs[0], 0u,
            bloom, m_pSceneBloomPostRTVs[0], &post);
        if (FAILED(result)) break;
        ID3D11ShaderResourceView* clear[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
        m_pContext->PSSetShaderResources(0u, _countof(clear), clear);
        m_pContext->OMSetRenderTargets(0u, nullptr, nullptr);
        const auto copy = [this](ID3D11ShaderResourceView* source, ID3D11ShaderResourceView* destination) {
            ComPtr<ID3D11Resource> sourceResource, destinationResource;
            source->GetResource(&sourceResource); destination->GetResource(&destinationResource);
            ComPtr<ID3D11Texture2D> sourceTexture, destinationTexture;
            if (!sourceResource || !destinationResource || sourceResource.Get() == destinationResource.Get() ||
                FAILED(sourceResource.As(&sourceTexture)) || FAILED(destinationResource.As(&destinationTexture))) return E_FAIL;
            D3D11_TEXTURE2D_DESC a{}, b{};
            sourceTexture->GetDesc(&a); destinationTexture->GetDesc(&b);
            if (a.Width != b.Width || a.Height != b.Height || a.Format != b.Format ||
                a.ArraySize != b.ArraySize || a.MipLevels != b.MipLevels ||
                a.SampleDesc.Count != b.SampleDesc.Count || a.SampleDesc.Quality != b.SampleDesc.Quality) return E_FAIL;
            m_pContext->CopyResource(destinationResource.Get(), sourceResource.Get());
            return S_OK;
        };
        result = copy(m_pScenePostSRVs[0].Get(), color.Get());
        if (SUCCEEDED(result)) result = copy(m_pSceneBloomPostSRVs[0].Get(), bloom.Get());
        if (FAILED(result)) break;
    }
    ID3D11ShaderResourceView* clear[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    m_pContext->PSSetShaderResources(0u, _countof(clear), clear);
    m_pContext->OMSetRenderTargets(_countof(saved), saved, depth);
    for (auto* target : saved) if (target) target->Release();
    if (depth) depth->Release();
    return result;
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
		nullptr, m_pScenePostRTVs[0], DEFERRED_PASS_SCENE_RESOLVE,
		nullptr, m_pSceneBloomPostRTVs[0]);
	const vector<PRESENTATION_SCREEN_POST_DESC>& ScreenPosts =
		Presentation.Get_ScreenPosts();
	size_t composedPostCount = 0u;
	for (size_t iPost = 0u;
		SUCCEEDED(hResult) && iPost < ScreenPosts.size(); ++iPost)
	{
		const PRESENTATION_SCREEN_POST_DESC& Post = ScreenPosts[iPost];
		if (Post.pMaterial && Post.pMaterial->Replaces_SceneBeforeBlend()) continue;
		const PRESENTATION_SCREEN_POST_PLAN_STEP Step =
			Build_PresentationScreenPostPlanStep(composedPostCount++);
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
		case PRESENTATION_SCREEN_POST_PROFILE::CHROMATIC_ABERRATION_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_CHROMATIC_ABERRATION;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::PREPARED_MATERIAL:
			iPassIndex = 0u;
			break;
		default:
			hResult = E_FAIL;
			continue;
		}
		hResult = Render_ScreenPostPass(
			m_pScenePostSRVs[Step.iSourceTarget],
			m_pScenePostRTVs[Step.iDestinationTarget],
			iPassIndex, m_pSceneBloomPostSRVs[Step.iSourceTarget],
			m_pSceneBloomPostRTVs[Step.iDestinationTarget], &Post);
	}
	const vector<PRESENTATION_SCREEN_OVERLAY_DESC>& ScreenOverlays =
		Presentation.Get_ScreenOverlays();
	size_t hdrOverlayCount = 0u;
	for (size_t iOverlay = 0u;
		SUCCEEDED(hResult) && iOverlay < ScreenOverlays.size(); ++iOverlay)
	{
		const PRESENTATION_SCREEN_OVERLAY_DESC& Overlay =
			ScreenOverlays[iOverlay];
		if (Overlay.bDisplaySpace)
			continue;
		const PRESENTATION_SCREEN_POST_PLAN_STEP Step =
			Build_PresentationScreenOverlayPlanStep(
				composedPostCount, hdrOverlayCount++);
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
		ID3D11RenderTargetView* destinations[3] = { pDestinationRTV.Get(),
			nullptr, m_pSceneBloomPostRTVs[Step.iDestinationTarget].Get() };
		m_pContext->OMSetRenderTargets(3u, destinations, nullptr);
		const float4_t vClear{};
		m_pContext->ClearRenderTargetView(destinations[0], &vClear.x);
		m_pContext->ClearRenderTargetView(destinations[2], &vClear.x);
		if (FAILED(m_pShader->Bind_Texture("g_PostProcessTexture", pSourceSRV)) ||
			FAILED(m_pShader->Bind_Texture("g_PostBloomTexture", m_pSceneBloomPostSRVs[Step.iSourceTarget])) ||
			FAILED(Render_ScreenOverlay(Overlay)))
			hResult = E_FAIL;
	}
	m_iScenePostFinalTarget = PresentationScreenCompositionFinalTarget(
		composedPostCount, hdrOverlayCount);

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
	return hResult;
}

HRESULT CRenderer::Render_ScreenOverlay(
	const PRESENTATION_SCREEN_OVERLAY_DESC& Overlay)
{
	const uint32_t iCoverageChannel =
		static_cast<uint32_t>(Overlay.eCoverageChannel);
	const uint32_t iFilter = static_cast<uint32_t>(Overlay.eFilter);
	const uint32_t iAddress = static_cast<uint32_t>(Overlay.eAddress);
	if (FAILED(m_pShader->Bind_RawValue("g_fEffectBloomIntensity", &Overlay.fBloomIntensity, sizeof(Overlay.fBloomIntensity))) ||
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
		FAILED(m_pShader->Begin(Overlay.bDisplaySpace ?
			ETOUI(DEFERRED::PRESENTATION_DISPLAY_OVERLAY) :
			DEFERRED_PASS_TEXTURED_OVERLAY)) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CRenderer::Render_DisplayOverlays()
{
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	HRESULT result = S_OK;
	for (const auto& overlay : Presentation.Get_ScreenOverlays())
	{
		if (!overlay.bDisplaySpace)
			continue;
		// No scene SRV is sampled: alpha blend directly over the final display.
		if (FAILED(m_pShader->Bind_Texture("g_PostProcessTexture", nullptr)) ||
			FAILED(Render_ScreenOverlay(overlay)))
		{
			result = E_FAIL;
			break;
		}
	}
	ID3D11ShaderResourceView* nullSrvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(0u, _countof(nullSrvs), nullSrvs);
	Presentation.Clear_ScreenOverlays();
	return result;
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
		m_pSceneBloomPostSRVs[m_iScenePostFinalTarget],
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
	const uint32_t materialView = static_cast<uint32_t>(m_MaterialRenderSettings.eDebugView);
	if (FAILED(m_pShader->Bind_RawValue("g_MaterialDebugView", &materialView, sizeof(materialView))) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterSurface"), m_pShader, "g_CharacterSurfaceTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterGeometry"), m_pShader, "g_CharacterGeometryTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return E_FAIL;
	const uint32_t iBloomEnabled =
		m_RenderQualitySettings.bBloomEnabled ? 1u : 0u;
	const uint32_t iFXAAEnabled =
		m_RenderQualitySettings.bFXAAEnabled ? 1u : 0u;
	const float2_t vInverseSceneSize = {
		1.f / static_cast<f32_t>((max)(1u, m_iScenePostWidth)),
		1.f / static_cast<f32_t>((max)(1u, m_iScenePostHeight)) };

	const auto& source = m_RenderQualitySettings.SourcePostProcess;
	const int32_t sourceEnabled = source.bEnabled ? 1 : 0;
	const float scale = (std::max)(0.000001f, source.fToneScale);
	const float inverseScale = 1.f / scale;
	const float curveA = .22f * inverseScale;
	const float curveB = 1.f / (source.fToneRange / (source.fToneRange + curveA));
	const float4_t sourceCurve(curveA, curveB, sqrtf(curveB * curveA * inverseScale) - curveA, scale);
	if (FAILED(m_pShader->Bind_RawValue("g_iSourcePostProcessEnabled", &sourceEnabled, sizeof(sourceEnabled))) ||
		FAILED(m_pShader->Bind_RawValue("g_vSourceToneCurve", &sourceCurve, sizeof(sourceCurve))) ||
		FAILED(m_pShader->Bind_RawValue("g_fSourceToneToe", &source.fToneToe, sizeof(source.fToneToe))) ||
		FAILED(m_pShader->Bind_Texture("g_SourceGradingLut", m_pSourceGradingLut))) return E_FAIL;

	if (FAILED(m_pShader->Bind_Texture(
		"g_SceneHDRTexture",
		m_pScenePostSRVs[m_iScenePostFinalTarget])))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_BloomResult"), m_pShader, "g_BloomTexture")))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue(
			"g_iBloomEnabled", &iBloomEnabled, sizeof(iBloomEnabled))) ||
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
			"g_vSceneBloomTint", &m_RenderQualitySettings.vBloomTint,
			sizeof(m_RenderQualitySettings.vBloomTint))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSceneDesaturation", &m_RenderQualitySettings.fSceneDesaturation,
			sizeof(m_RenderQualitySettings.fSceneDesaturation))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iColorFilterType", &m_RenderQualitySettings.iColorFilterType,
			sizeof(m_RenderQualitySettings.iColorFilterType))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fColorFilterStrength", &m_RenderQualitySettings.fColorFilterStrength,
			sizeof(m_RenderQualitySettings.fColorFilterStrength))) ||
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
	for (size_t renderIndex = 0; renderIndex < m_RenderObjects[ETOUI(RENDERGROUP::UI)].size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = m_RenderObjects[ETOUI(RENDERGROUP::UI)][renderIndex].get();
		if (nullptr != pRenderObject)
			pRenderObject->Render_Group(RENDERGROUP::UI);
	}

	m_RenderObjects[ETOUI(RENDERGROUP::UI)].clear();

	return S_OK;
}

HRESULT CRenderer::Ready_SourceLightMask(uint32_t width, uint32_t height)
{
    if (width == 0u || height == 0u) return S_FALSE;
    if (width == m_iSourceLightMaskFailedWidth && height == m_iSourceLightMaskFailedHeight)
        return S_FALSE;
    if (m_pSourceLightMaskDSV && width == m_iSourceLightMaskWidth && height == m_iSourceLightMaskHeight)
        return S_OK;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = desc.ArraySize = 1u;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1u;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11DepthStencilView> depth;
    HRESULT result = m_pDevice->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
    if (SUCCEEDED(result)) result = m_pDevice->CreateDepthStencilView(texture.Get(), nullptr, depth.GetAddressOf());
    if (FAILED(result))
    {
        m_iSourceLightMaskFailedWidth = width;
        m_iSourceLightMaskFailedHeight = height;
        WriteRendererFailure("SourceLightMask_ResourceFallback", result);
        return S_FALSE;
    }
    m_pSourceLightMaskDSV = std::move(depth);
    m_iSourceLightMaskWidth = width;
    m_iSourceLightMaskHeight = height;
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
    m_bStaticShadowCacheValid = false;
    m_StaticShadowCasters.clear();
    m_CandidateStaticShadowCasters.clear();
    m_pStaticShadowDepthTexture.Reset();
    // The optional copy-only texture uses the identical depth format and size.
    // Allocation failure preserves the ordinary uncached shadow path.
    TextureDesc.BindFlags = 0u;
    (void)m_pDevice->CreateTexture2D(&TextureDesc, nullptr,
        m_pStaticShadowDepthTexture.GetAddressOf());
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
		nullptr != m_pScenePostSRVs[1] &&
		nullptr != m_pSceneBloomPostSRVs[0] && nullptr != m_pSceneBloomPostSRVs[1])
	{
		return S_OK;
	}

	ComPtr<ID3D11Texture2D> StagedTextures[2];
	ComPtr<ID3D11RenderTargetView> StagedRTVs[2];
	ComPtr<ID3D11ShaderResourceView> StagedSRVs[2];
	ComPtr<ID3D11Texture2D> StagedBloomTextures[2];
	ComPtr<ID3D11RenderTargetView> StagedBloomRTVs[2];
	ComPtr<ID3D11ShaderResourceView> StagedBloomSRVs[2];
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
				StagedSRVs[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, StagedBloomTextures[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateRenderTargetView(StagedBloomTextures[iTarget].Get(), nullptr, StagedBloomRTVs[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateShaderResourceView(StagedBloomTextures[iTarget].Get(), nullptr, StagedBloomSRVs[iTarget].GetAddressOf())))
		{
			return E_FAIL;
		}
	}

	for (size_t iTarget = 0u; iTarget < 2u; ++iTarget)
	{
		m_pScenePostTextures[iTarget] = std::move(StagedTextures[iTarget]);
		m_pScenePostRTVs[iTarget] = std::move(StagedRTVs[iTarget]);
		m_pScenePostSRVs[iTarget] = std::move(StagedSRVs[iTarget]);
		m_pSceneBloomPostRTVs[iTarget] = std::move(StagedBloomRTVs[iTarget]);
		m_pSceneBloomPostSRVs[iTarget] = std::move(StagedBloomSRVs[iTarget]);
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
