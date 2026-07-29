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
	float3_t m_vWorldPreviewPosition = { 0.f, 2.f, 0.f };
	string m_strJsonPath = {
		"../Bin/Resources/LostArk/Effect/Effect_Tool/Editor/working.effect.json"
	};
	string m_strBinaryPath = {
		"../Bin/Resources/LostArk/Effect/Effect_Tool/Editor/working.weffect"
	};
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
};

NS_END

