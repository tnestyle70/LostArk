#pragma once

#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

enum class COOKED_TEXTURE_SLOT
{
	DIFFUSE,
	NORMAL,
	END
};

class ENGINE_DLL CCookedModel final
{
private:
	CCookedModel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
	~CCookedModel() = default;

public:
	uint32_t Get_NumMeshes() const { return static_cast<uint32_t>(m_Meshes.size()); }
	uint32_t Get_NumAnimations() const { return static_cast<uint32_t>(m_Animations.size()); }
	bool_t Is_Skinned() const { return m_IsSkinned; }
	bool_t Has_Skeleton() const { return !m_Skeleton.bones.empty(); }
	bool_t Has_Animations() const { return !m_Animations.empty(); }
	const string& Get_AnimationName(uint32_t index) const;

	bool_t Play_Animation(uint32_t index, bool_t loop = true);
	bool_t Play_Animation(const string& name, bool_t loop = true);
	void Stop_Animation();
	void Set_AnimationSpeed(f32_t speed) { m_AnimationSpeed = speed; }
	void Update_Animation(f32_t fTimeDelta);

	HRESULT Bind_Material(shared_ptr<class CShader> pShader,
		const char_t* pConstantName,
		uint32_t meshIndex,
		COOKED_TEXTURE_SLOT slot) const;
	HRESULT Bind_BoneMatrices(shared_ptr<class CShader> pShader,
		const char_t* pConstantName) const;
	HRESULT Render(uint32_t meshIndex) const;

private:
	struct RUNTIME_MESH
	{
		ComPtr<ID3D11Buffer> pVertexBuffer;
		ComPtr<ID3D11Buffer> pIndexBuffer;
		uint32_t vertexStride = { sizeof(VTXMESH) };
		uint32_t indexCount = {};
		uint32_t materialIndex = {};
	};

	struct RUNTIME_MATERIAL
	{
		ComPtr<ID3D11ShaderResourceView> pDiffuseSRV;
		ComPtr<ID3D11ShaderResourceView> pNormalSRV;
	};

private:
	HRESULT Initialize(const MODEL_ASSET_LOAD_DESC& desc);
	HRESULT Ready_Meshes(const MODEL_ASSET_DATA& assetData);
	HRESULT Ready_Materials(const MODEL_ASSET_DATA& assetData);
	HRESULT Ready_Animation(const MODEL_ASSET_DATA& assetData,
		const string& defaultAnimationName);
	void Evaluate_Pose();

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	vector<RUNTIME_MESH> m_Meshes;
	vector<RUNTIME_MATERIAL> m_Materials;
	bool_t m_IsSkinned = { false };

	MODEL_SKELETON_DATA m_Skeleton;
	vector<MODEL_ANIMATION_DATA> m_Animations;
	uint32_t m_CurrentAnimationIndex = { static_cast<uint32_t>(-1) };
	f32_t m_CurrentTimeTicks = {};
	f32_t m_AnimationSpeed = { 1.f };
	bool_t m_IsPlaying = { false };
	bool_t m_IsLoop = { true };
	vector<float4x4_t> m_LocalBoneMatrices;
	vector<float4x4_t> m_GlobalBoneMatrices;
	vector<float4x4_t> m_FinalBoneMatrices;

public:
	static shared_ptr<CCookedModel> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const MODEL_ASSET_LOAD_DESC& desc);
};

NS_END
