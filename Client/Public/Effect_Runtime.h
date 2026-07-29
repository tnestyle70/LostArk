#pragma once

#include <filesystem>
#include <unordered_map>

#include "Client_Defines.h"
#include "Effect_ParticleSimulator.h"
#include "Effect_Types.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CCookedModel;
class CModel;
class CShader;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

/*
 * Game-world renderer for the data authored by CEffect_Tool.
 *
 * The editor publishes an immutable snapshot through Publish_Preview().
 * Instances consume that snapshot at the next Update(), rebuild their CPU
 * simulators only when the generation changes, and render through the normal
 * RENDERGROUP::BLEND path.  Consequently the editor never owns a D3D resource
 * and the runtime never owns ImGui state.
 */
class CEffect_Runtime final : public CGameObject
{
private:
	static constexpr size_t MAX_TEXTURE_CACHE_ENTRIES = 64;
	static constexpr size_t MAX_MODEL_CACHE_ENTRIES = 16;
	static constexpr uint32_t MAX_PARTICLES_PER_EMITTER = 65536;
	static constexpr uint32_t MAX_PARTICLES_PER_EFFECT = 262144;
	static constexpr uint32_t MAX_TRAIL_POINTS = 4096;
	static constexpr uint32_t MAX_PRIMITIVE_VERTICES =
		(MAX_TRAIL_POINTS - 1u) * 6u;

private:
	struct EFFECT_EMITTER_RUNTIME
	{
		EFFECT_EMITTER_DESC Desc;
		CEffect_ParticleSimulator Simulator;
		ComPtr<ID3D11ShaderResourceView> pTextureSRV;
		shared_ptr<CModel> pModel;
		shared_ptr<CCookedModel> pCookedModel;
		bool_t isSimulationReady = { false };
	};

private:
	CEffect_Runtime(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CEffect_Runtime();

public:
	/*
	 * Thread-safe editor/game bridge.  Publishing disabled state also advances
	 * the generation so an already running world object disappears promptly.
	 */
	static void Publish_Preview(const EFFECT_ASSET_DESC& AssetDesc,
		bool_t isEnabled,
		const float3_t& vWorldPosition);
	static uint64_t Get_PublishedGeneration();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_RenderResources();
	HRESULT Create_FallbackTexture();
	bool_t Consume_PublishedPreview();
	bool_t Rebuild(const EFFECT_ASSET_DESC& AssetDesc,
		const float3_t& vWorldPosition);

	const EFFECT_MODULE_DESC* Find_Module(
		const EFFECT_EMITTER_DESC& EmitterDesc,
		EFFECT_MODULE_TYPE eType) const;

	filesystem::path Resolve_AssetPath(const string& strAssetId) const;
	ComPtr<ID3D11ShaderResourceView> Load_Texture(
		const string& strTextureAssetId);
	shared_ptr<CModel> Load_StaticModel(const string& strMeshAssetId);
	shared_ptr<CCookedModel> Load_CookedModel(
		const EFFECT_REQUIRED_MODULE_DESC& RequiredDesc);

	HRESULT Render_SpriteEmitter(const EFFECT_EMITTER_RUNTIME& Runtime);
	HRESULT Render_MeshEmitter(const EFFECT_EMITTER_RUNTIME& Runtime);
	HRESULT Render_CookedMeshEmitter(
		const EFFECT_EMITTER_RUNTIME& Runtime);
	HRESULT Render_PrimitiveEmitter(const EFFECT_EMITTER_RUNTIME& Runtime);

	HRESULT Bind_CommonSpriteResources(
		const EFFECT_EMITTER_RUNTIME& Runtime);
	HRESULT Bind_CommonPrimitiveResources(
		const EFFECT_EMITTER_RUNTIME& Runtime,
		const float4_t& vTint,
		fmatrix_t WorldMatrix);
	HRESULT Ensure_PrimitiveBuffer(uint32_t iVertexCount);
	HRESULT Draw_PrimitiveVertices(
		const vector<VTXTEX>& Vertices,
		const EFFECT_EMITTER_RUNTIME& Runtime,
		const float4_t& vTint);

	float3_t Get_WorldOrigin() const;
	float3_t To_WorldPosition(const EFFECT_PARTICLE& Particle) const;
	float4_t Calculate_UVRect(const EFFECT_EMITTER_DESC& EmitterDesc,
		const EFFECT_PARTICLE& Particle) const;

public:
	static unique_ptr<CEffect_Runtime> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

private:
	EFFECT_ASSET_DESC m_AssetDesc;
	vector<EFFECT_EMITTER_RUNTIME> m_EmitterRuntimes;

	shared_ptr<CShader> m_pSpriteShader;
	shared_ptr<CShader> m_pPrimitiveShader;
	shared_ptr<CShader> m_pMeshShader;
	shared_ptr<CShader> m_pAnimMeshShader;
	shared_ptr<CVIBuffer_Rect> m_pRectBuffer;

	ComPtr<ID3D11ShaderResourceView> m_pFallbackTextureSRV;
	ComPtr<ID3D11Buffer> m_pPrimitiveVertexBuffer;
	uint32_t m_iPrimitiveVertexCapacity = {};

	unordered_map<wstring_t, ComPtr<ID3D11ShaderResourceView>>
		m_TextureCache;
	unordered_map<wstring_t, shared_ptr<CModel>>
		m_ModelCache;

	float3_t m_vWorldPosition = {};
	uint64_t m_iAppliedGeneration = {};
	bool_t m_isPreviewEnabled = { false };
};

NS_END
