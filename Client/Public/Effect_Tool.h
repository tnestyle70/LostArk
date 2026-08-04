#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_Types.h"
#include "Effect_ParticleSimulator.h"
#include "Effect_ResourceCatalog.h"

struct ImDrawList;
struct ImVec2;

NS_BEGIN(Client)

class CEffect_Runtime;

class CEffect_Tool final
{
public:
	explicit CEffect_Tool(ComPtr<ID3D11Device> pDevice);
	~CEffect_Tool();

	void Render();

private:
	struct TEXTURE_PREVIEW_RESOURCE
	{
		string strAssetId;
		ComPtr<ID3D11ShaderResourceView> pShaderResourceView = { nullptr };
		string strStatus = "Not loaded";
		bool_t isLoadAttempted = { false };
	};

	void Create_DefaultAsset();
	void Rebuild_Simulators();
	// A loaded asset brings its own element ids. Without this the counter
	// would restart at 1 and hand out ids that already exist.
	void Refresh_NextElementId();
	void Adopt_LoadedAsset(EFFECT_ASSET_DESC&& Loaded);
	filesystem::path Resolve_AuthoredPath(const string& strAssetId) const;
	void Refresh_AuthoredList();
	// 사용자가 명시적으로 요청했을 때 SceneHDR과 Bloom을 제한된 프레임 동안 검사한다.
	void Capture_SceneHDR_Readback();
	// Zoom that makes the current asset fill the preview canvas.
	f32_t Estimate_PreviewZoom() const;
	void Restart_Preview();
	void Render_Toolbar();
	void Render_EmitterPanel();
	void Render_ModulePanel();
	void Render_PropertyPanel();
	void Render_PreviewPanel();
	void Render_SourceCatalogPanel();
	void Update_WorldPreview(f32_t fTimeDelta);
	void Reload_SourceCatalog();
	void Refresh_SourceCatalogResults();
	void Draw_Particles(ImDrawList* pDrawList,
		const ImVec2& vOrigin, const ImVec2& vSize, f32_t fScale);
	ID3D11ShaderResourceView* Resolve_TextureResource(
		const EFFECT_EMITTER_DESC& Emitter);
	const string& Get_TextureResourceStatus(
		const EFFECT_EMITTER_DESC& Emitter) const;
	void Reload_TextureResource(const EFFECT_EMITTER_DESC& Emitter);

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	shared_ptr<CEffect_Runtime> m_pWorldPreview = { nullptr };
	EFFECT_ASSET_DESC m_Asset;
	vector<unique_ptr<CEffect_ParticleSimulator>> m_Simulators;
	unordered_map<uint64_t, TEXTURE_PREVIEW_RESOURCE> m_TextureResources;
	CEffect_ResourceCatalog m_SourceCatalog;
	vector<size_t> m_SourceCatalogResults;
	EFFECT_PARTICLE_COUNT_VALIDATION_RESULT m_Phase2Validation;
	int32_t m_iSelectedEmitter = {};
	int32_t m_iSelectedModule = {};
	uint64_t m_iNextElementId = { 1 };
	f32_t m_fPreviewTime = {};
	f32_t m_fTimeScale = { 1.f };
	f32_t m_fPreviewDistance = {};
	// Pixels per world metre in the 2D preview. Imported effects are authored
	// in centimetres and convert down to fractions of a metre, so the canvas
	// needs to zoom instead of the asset being scaled up.
	f32_t m_fPreviewZoom = { 18.f };
	float3_t m_vWorldPreviewPosition = { 0.f, 2.f, 0.f };
	// A cue that will ride a weapon socket cannot be judged at a fixed point in
	// space. When following, the world preview is placed on the named anchor of
	// whatever character the shared preview panel currently publishes, so the
	// effect is seen on the actual bone at the actual animation frame.
	char_t m_AnchorSlot[64] = "b_wp_swm_m_1";
	bool_t m_isAnchorFollow = { false };
	string m_strAnchorStatus;
	string m_strAuthoringPath;
	string m_strBinaryPath;
	string m_strSaveAsId;
	vector<string> m_AuthoredAssets;
	int32_t m_iSelectedAuthoredAsset = { -1 };
	string m_strFileStatus;
	string m_strSourceCatalogGroup = "Bard";
	string m_strSourceCatalogFilter;
	string m_strSourceCatalogStatus;
	int32_t m_iSelectedSourceCatalogResult = { -1 };
	bool_t m_isPlaying = { true };
	bool_t m_isLooping = { true };
	bool_t m_isWorldPreviewEnabled = { false };
	bool_t m_isWorldPreviewDirty = { true };
	bool_t m_isSourceCatalogOpen = { true };
	bool_t m_isFocusRequested = { false };
	ComPtr<ID3D11Texture2D> m_pSceneHDRStaging = { nullptr };
	ComPtr<ID3D11Texture2D> m_pDistortionStaging = { nullptr };
	uint32_t m_iHDRReadbackFrame = {};
	bool_t m_isHDRReadbackRequested = { false };
	bool_t m_isHDRReadbackDone = { false };
	ComPtr<ID3D11Texture2D> m_pBloomResultStaging = { nullptr };
};

NS_END
