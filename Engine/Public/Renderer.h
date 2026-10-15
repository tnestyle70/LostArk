#pragma once

#include "GameObject.h"
#include "Engine_RenderTypes.h"
#include "Target_Manager.h"

/* 화면에 그려져야할 객체들을 그리는 순서에 따른 그룹별로 모아둔다. */
/* 모아둔 순서대로 객체들의 드로우콜을 수행해준다.*/

NS_BEGIN(Engine)

class CGpuSceneVisibility;
class CGpuVisibilityHiZ;
struct PRESENTATION_SCREEN_POST_DESC;
struct PRESENTATION_SCREEN_OVERLAY_DESC;

// 프레임 동안 제출된 GameObject를 렌더 그룹별로 보관하고 그림자·표면·조명·합성·후처리 순서로 그린다.
// CGameInstance가 이 객체를 소유하며, 내부 GPU 자원과 프레임 큐의 참조는 각각 ComPtr·shared_ptr가 유지한다.
class CRenderer final 
{
private:	
	CRenderer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CRenderer();

public:
    struct VIEWPORT_RESIZE_STATE final
    {
        uint32_t Width = 0u, Height = 0u;
        CTarget_Manager::RESIZE_STATE Targets;
        ComPtr<ID3D11DepthStencilView> BloomDSV, SSAODSV, SourceLightMaskDSV;
        ComPtr<ID3D11Texture2D> ScenePostTextures[2];
        ComPtr<ID3D11RenderTargetView> ScenePostRTVs[2], SceneBloomPostRTVs[2];
        ComPtr<ID3D11ShaderResourceView> ScenePostSRVs[2], SceneBloomPostSRVs[2];
    };
    HRESULT Stage_ViewportResize(uint32_t width, uint32_t height,
        const CTarget_Manager& targets, VIEWPORT_RESIZE_STATE& output) const;
    void Commit_ViewportResize(CTarget_Manager& targets, VIEWPORT_RESIZE_STATE& staged) noexcept;

	HRESULT Initialize();
	HRESULT Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<CGameObject> pRenderObject);
	HRESULT Draw(bool staticOcclusionAllowed = false);
    // Level 소유 장면을 weak 참조하므로 등록 자체가 언로드를 막지 않는다.
    void Register_GpuSceneVisibility(const shared_ptr<CGpuSceneVisibility>& scene);
    void Unregister_GpuSceneVisibility(const CGpuSceneVisibility* scene);
    MAP_VISIBILITY_SETTINGS Get_MapVisibilitySettings() const { return m_MapVisibility; }
    HRESULT Apply_MapVisibilitySettings(const MAP_VISIBILITY_SETTINGS& settings);
    RENDER_OPTIMIZATION_SETTINGS Get_RenderOptimizationSettings() const { return m_RenderOptimization; }
    HRESULT Apply_RenderOptimizationSettings(const RENDER_OPTIMIZATION_SETTINGS& settings);

	/* world보다 먼저 같은 G-buffer·조명 경로로 portrait를 그린다. 호출자가 대상 texture·draw callback을 소유하고 Engine은 카메라·pass·resolve를 맡는다. */
	struct PORTRAIT_REQUEST final
	{
		/* tone mapping 결과를 목적 크기로 resample하는 target이다. caller가 Draw가 끝날 때까지 수명을 유지해야 한다. */
		ComPtr<ID3D11RenderTargetView>		pDestination = { nullptr };
		uint32_t							iWidth = 0u;
		uint32_t							iHeight = 0u;
		/* The portrait camera, already built by the caller. */
		float4x4_t							ViewMatrix{};
		float4x4_t							ProjMatrix{};
		/* G-buffer가 바인딩된 상태에서 대상을 그린다. 실패하면 이 요청만 건너뛴다. */
		function<HRESULT()>					DrawSubject;
		/* combine 뒤 대상의 forward pass를 재사용한다. 선택적 blend state는 offscreen UI의 coverage 누적만 바꾼다. */
		function<HRESULT(RENDERGROUP, ID3D11BlendState*)> DrawForward;
	};

	/* 다음 Draw가 소비·비우는 요청이다. 창이 요청을 멈추면 별도 teardown 없이 그리지 않는다. */
	HRESULT Request_Portrait(PORTRAIT_REQUEST Request);
	// A visible refractive occurrence requests the pre-translucency colour once.
	void Request_SceneColorSnapshot() { m_bSceneColorSnapshotRequested = true; }
	HRESULT Refresh_SceneColorSnapshot();
    void Request_SceneEnvironmentReplacement() { m_bSceneEnvironmentReplaced = true; }
    bool_t Is_SceneEnvironmentReplaced() const { return m_bSceneEnvironmentReplaced; }
	const RENDER_QUALITY_SETTINGS& Get_RenderQualitySettings() const { return m_RenderQualitySettings; }
	HRESULT Apply_RenderQualitySettings(const RENDER_QUALITY_SETTINGS& Settings);
	const MATERIAL_RENDER_SETTINGS& Get_MaterialRenderSettings() const { return m_MaterialRenderSettings; }
	HRESULT Apply_MaterialRenderSettings(const MATERIAL_RENDER_SETTINGS& settings);
	const HEIGHT_FOG_SETTINGS& Get_HeightFogSettings() const { return m_HeightFogSettings; }
	HRESULT Apply_HeightFog(const HEIGHT_FOG_SETTINGS& Settings);
	HRESULT Bind_HeightFog(class CShader* shader) const;
	// 최종 합성과 같은 현재 tone/LUT를 차용하며 저장 설정이나 LUT 수명을 바꾸지 않는다.
	HRESULT Bind_ActiveSceneColorTransform(class CShader* shader) const;
	HRESULT Stage_RenderEnvironment(const wstring_t& cubePath, const float4_t& color,
		const float4_t& rotationIntensity, RENDER_ENVIRONMENT_STATE& outState, bool_t forceReload = false) const;
	void Commit_RenderEnvironment(const RENDER_ENVIRONMENT_STATE& state);
	RENDER_ENVIRONMENT_STATE Get_RenderEnvironment() const;

	/* deferred fog의 시계는 Renderer가 소유해 각 화면 pass가 같은 시간을 사용한다. */
	void Advance_PresentationClock(f32_t fTimeDelta);

	HRESULT Add_DebugComponent(shared_ptr<CComponent> pDebugComponent);

private:
	ComPtr<ID3D11Device>					m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>				m_pContext = { nullptr };
	ComPtr<ID3D11Texture2D>				m_pShadowDepthTexture = { nullptr };
	ComPtr<ID3D11DepthStencilView>			m_pShadowDSV = { nullptr };
	ComPtr<ID3D11ShaderResourceView>		m_pShadowSRV = { nullptr };
	/* Request_Portrait가 채우고 Draw가 비우는 프레임 queue다. */
	vector<PORTRAIT_REQUEST>				m_PortraitRequests;
	ComPtr<ID3D11BlendState> m_pPortraitBlendState;
    struct STATIC_SHADOW_CASTER final
    {
        weak_ptr<CGameObject> Owner;
        const CGameObject* Identity = nullptr;
        uint64_t Revision = 0u;
    };
    // weak 참조로 cache가 언로드된 Level의 수명을 늘리지 않게 한다.
    ComPtr<ID3D11Texture2D> m_pStaticShadowDepthTexture;
    ComPtr<ID3D11ShaderResourceView> m_pStaticShadowSRV;
    vector<STATIC_SHADOW_CASTER> m_StaticShadowCasters;
    vector<STATIC_SHADOW_CASTER> m_CandidateStaticShadowCasters;
    vector<uint8_t> m_StaticShadowFlags;
    float4x4_t m_StaticShadowView{}, m_StaticShadowProjection{};
    bool_t m_bStaticShadowCacheValid = false;
    bool_t m_bStaticShadowSourceMaterials = false;
	ComPtr<ID3D11DepthStencilView>			m_pBloomDSV = { nullptr };
	ComPtr<ID3D11DepthStencilView>			m_pSSAODSV = { nullptr };
	ComPtr<ID3D11DepthStencilView> m_pSourceLightMaskDSV;
	uint32_t m_iSourceLightMaskWidth = 0u, m_iSourceLightMaskHeight = 0u;
	uint32_t m_iSourceLightMaskFailedWidth = 0u, m_iSourceLightMaskFailedHeight = 0u;
	// 명시적으로 선택한 후반 pass가 opaque pick을 지우지 않고 picking target을 갱신한다.
	ComPtr<ID3D11Texture2D> m_pPickingTexture;
	ComPtr<ID3D11RenderTargetView> m_pPickingRTV;
	ComPtr<ID3D11Texture2D> m_pPickingDepthTexture;
	ComPtr<ID3D11DepthStencilView> m_pPickingDSV;
	D3D11_TEXTURE2D_DESC m_PickingDepthDesc{};
	D3D11_DEPTH_STENCIL_VIEW_DESC m_PickingDSVDesc{};
	bool_t m_bPickingDepthCaptured = false;
	vector<shared_ptr<CGameObject>>		m_RenderObjects[ETOUI(RENDERGROUP::END)];

    std::vector<weak_ptr<CGpuSceneVisibility>> m_GpuScenes;
    std::unique_ptr<CGpuVisibilityHiZ> m_GpuHiZ;
    uint64_t m_GpuVisibilityFrame = 0u;
    bool m_GpuHiZUnavailable = false;
    MAP_VISIBILITY_SETTINGS m_MapVisibility{};
    RENDER_OPTIMIZATION_SETTINGS m_RenderOptimization{};
    std::unique_ptr<class COcclusionCuller> m_OcclusionCuller;
    struct OCCLUSION_CANDIDATE final
    {
        size_t Index = 0u;
        CGameObject::STATIC_OCCLUSION_DESC Desc{};
        float Score = 0.f;
    };
    vector<OCCLUSION_CANDIDATE> m_OcclusionCandidates;
    vector<size_t> m_OccluderOrder;
    vector<uint8_t> m_OcclusionHidden;
    vector<shared_ptr<CGameObject>> m_OcclusionVisibleQueue;
    struct OCCLUSION_CACHE_ROW final
    {
        weak_ptr<CGameObject> Owner;
        CGameObject::STATIC_OCCLUSION_DESC Desc{};
        bool Hidden = false;
    };
    vector<OCCLUSION_CACHE_ROW> m_OcclusionCache;
    float4x4_t m_OcclusionView{}, m_OcclusionProjection{};
    float2_t m_OcclusionViewport{};
    uint64_t m_OcclusionSettingsRevision = 0u;
    bool m_OcclusionCacheValid = false;
	shared_ptr<class CVIBuffer_Rect>		m_pVIBuffer = { nullptr };
	shared_ptr<class CShader>				m_pShader = { nullptr };
    shared_ptr<class CShader> m_pScreenSpaceLightingShader;
    ComPtr<ID3D11Texture2D> m_pSSGIHalfTexture;
    ComPtr<ID3D11RenderTargetView> m_pSSGIHalfRTV;
    ComPtr<ID3D11ShaderResourceView> m_pSSGIHalfSRV;
    uint32_t m_iSSGIHalfWidth = 0u, m_iSSGIHalfHeight = 0u;

	float4x4_t								m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	uint32_t								m_iShadowMapSize = 2048u;
	float2_t								m_vShadowTexelSize = {};
	uint32_t								m_iBloomWidth = {};
	uint32_t								m_iBloomHeight = {};
	float2_t								m_vBloomTexelSize = {};
	uint32_t								m_iSSAOWidth = {};
	uint32_t								m_iSSAOHeight = {};
	float2_t								m_vSSAOTexelSize = {};
	ComPtr<ID3D11Texture2D>				m_pScenePostTextures[2];
	ComPtr<ID3D11RenderTargetView>		m_pScenePostRTVs[2];
	ComPtr<ID3D11ShaderResourceView>	m_pScenePostSRVs[2];
	ComPtr<ID3D11RenderTargetView> m_pSceneBloomPostRTVs[2];
	ComPtr<ID3D11ShaderResourceView> m_pSceneBloomPostSRVs[2];
	uint32_t								m_iScenePostWidth = {};
	uint32_t								m_iScenePostHeight = {};
	uint32_t								m_iScenePostFinalTarget = {};
	struct SOURCE_GRADING_CACHE_ENTRY final
	{
		SOURCE_POST_PROCESS_SETTINGS Settings;
		f32_t fGamma = 2.2f;
		ComPtr<ID3D11ShaderResourceView> pTexture;
	};
	vector<SOURCE_GRADING_CACHE_ENTRY> m_SourceGradingCache;
	ComPtr<ID3D11ShaderResourceView> m_pSourceGradingLut;
	RENDER_QUALITY_SETTINGS				m_RenderQualitySettings = {};
	MATERIAL_RENDER_SETTINGS m_MaterialRenderSettings = {};
	RENDER_ENVIRONMENT_STATE m_RenderEnvironment;
	HEIGHT_FOG_SETTINGS				m_HeightFogSettings = {};
	bool_t m_bSceneColorSnapshotRequested = false;
    bool_t m_bSceneEnvironmentReplaced = false;
	f32_t							m_fPresentationClock = 0.f;

	list<shared_ptr<CComponent>>			m_DebugComponent;

private:
	HRESULT Render_Priority();
	HRESULT Render_Shadow();
	HRESULT Render_NonBlend(bool staticOcclusionAllowed);
    void Cull_StaticOcclusion();
    bool Prepare_GpuSceneVisibility(bool staticOcclusionAllowed);
	HRESULT Render_SSAO();
	HRESULT Render_SSAOPass(const wstring_t& strMRTTag, DEFERRED ePass);
	HRESULT Render_Lights();
	HRESULT Render_Portraits();
	HRESULT Render_Combined(bool_t bPortrait = false);
    HRESULT Render_ScreenSpaceLighting();
    HRESULT Ready_SSGIHalfTarget(uint32_t width, uint32_t height);
	HRESULT Render_NonLight();
	HRESULT Render_Blend();
	HRESULT Capture_SceneColorSnapshot();
	HRESULT Render_SceneReplacements();
	HRESULT Render_ScreenPosts();
	HRESULT Render_DisplayOverlays();
	HRESULT Render_ScreenOverlay(const PRESENTATION_SCREEN_OVERLAY_DESC& Overlay);
	HRESULT Render_ScreenPostPass(
		ComPtr<ID3D11ShaderResourceView> pSourceSRV,
		ComPtr<ID3D11RenderTargetView> pDestinationRTV,
		uint32_t iPassIndex,
		ComPtr<ID3D11ShaderResourceView> pBloomSourceSRV,
		ComPtr<ID3D11RenderTargetView> pBloomDestinationRTV,
		const PRESENTATION_SCREEN_POST_DESC* pPostDesc = nullptr);
	HRESULT Render_Bloom();
	HRESULT Render_BloomPass(const wstring_t& strMRTTag,
		ComPtr<ID3D11ShaderResourceView> pSourceSRV, DEFERRED ePass);
	HRESULT Render_Final(bool_t bPortrait = false);
	HRESULT Render_UI();
	HRESULT Capture_PickingDepth();
	HRESULT Render_Picking();

private:
	HRESULT Stage_SourceGradingLut(const SOURCE_POST_PROCESS_SETTINGS& source, f32_t gamma,
		ComPtr<ID3D11ShaderResourceView>& output);
	HRESULT Ready_Shadow_Resources();
	HRESULT Ready_SourceLightMask(uint32_t width, uint32_t height);
	HRESULT Ready_Bloom_DSV();
	HRESULT Ready_SSAO_DSV();
	HRESULT Ready_ScenePostTargets(uint32_t iWidth, uint32_t iHeight);
	void SetUp_ViewportDesc(uint32_t iWidth, uint32_t iHeight);

private:
	HRESULT Render_Debug();



public:
	static unique_ptr<CRenderer> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

};

NS_END
