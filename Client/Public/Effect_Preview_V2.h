#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CEffectPreviewV2 final : public CGameObject
{
public:
	enum class SHAPE : int32_t
	{
		MESH,
		SPRITE,
		END
	};

	enum class BLEND_MODE : int32_t
	{
		ALPHA,
		ADDITIVE,
		SOLID,
		END
	};

	enum class TEXTURE_INPUT : int32_t
	{
		BASE,
		NOISE,
		MASK,
		EMISSIVE,
		DISSOLVE,
		END
	};

	enum class COLOR_CLIP_CHANNEL : int32_t
	{
		RGB,
		ALPHA,
		END
	};

	struct LERP_FLOAT3 final
	{
		float3_t vStart = { 0.f, 0.f, 0.f };
		float3_t vEnd = { 0.f, 0.f, 0.f };
		bool_t bLerp = false;
		float3_t Evaluate(f32_t fLifeRatio) const;
	};

	struct PARAMS final
	{
		LERP_FLOAT3 Position;
		LERP_FLOAT3 Rotation;
		LERP_FLOAT3 Scale = { { 1.f, 1.f, 1.f }, { 1.f, 1.f, 1.f }, false };
		LERP_FLOAT3 Velocity;
		float4_t vColorOffset = { 0.f, 0.f, 0.f, 0.f };
		float4_t vColorMul = { 1.f, 1.f, 1.f, 1.f };
		COLOR_CLIP_CHANNEL eColorClipChannel = COLOR_CLIP_CHANNEL::ALPHA;
		f32_t fColorClip = 0.f;
		float4_t vRimColor = { 1.f, 1.f, 1.f, 1.f };
		f32_t fRimPower = 3.f;
		f32_t fRimIntensity = 0.f;
		f32_t fGhostAlpha = 0.f;
		f32_t fBloomIntensity = 1.f;
		f32_t fDistortionIntensity = 0.f;
		float2_t vUVStart = { 0.f, 0.f };
		float2_t vUVSpeed = { 0.f, 0.f };
		float2_t vUVTileCount = { 1.f, 1.f };
		f32_t fNoiseStrength = 0.f;
		f32_t fNoiseScale = 1.f;
		float2_t vNoisePan = { 0.f, 0.f };
		f32_t fDissolveStart = 0.f;
		f32_t fDissolveSoftness = 0.1f;
		BLEND_MODE eBlend = BLEND_MODE::ADDITIVE;
		bool_t bBillboard = true;
		bool_t bDepthTest = true;
		f32_t fLifetime = 0.f;
		bool_t bLoop = true;
		f32_t fPlayRate = 1.f;
		f32_t fMeshPreScale = 0.01f;
		uint32_t iAnimationIndex = 0u;
		bool_t bAnimationLoop = true;
	};

	struct PART final
	{
		bool_t bVisible = true;
		std::string strBaseAssetId;
		ComPtr<ID3D11ShaderResourceView> pBaseView;
	};

	struct DESC final : public GAMEOBJECT_DESC
	{
		SHAPE eShape = SHAPE::SPRITE;
		std::string strMeshAssetId;
		std::array<std::string, static_cast<size_t>(TEXTURE_INPUT::END)> TextureAssetIds;
		float4x4_t PivotWorld = {
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f };
		PARAMS Params;
		bool_t bParamsAuthored = false;
	};

private:
	CEffectPreviewV2(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CEffectPreviewV2();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	PARAMS& Params() { return m_Params; }
	const DESC& Creation_Desc() const { return m_CreationDesc; }
	float4x4_t& PivotWorld() { return m_PivotWorld; }
	const std::string& Status() const { return m_strStatus; }
	SHAPE Shape() const { return m_eShape; }
	f32_t Time() const { return m_fTime; }
	f32_t Life_Ratio() const;
	f32_t Dissolve_Amount() const;
	bool_t Has_Texture(const TEXTURE_INPUT eInput) const
	{
		return nullptr != m_Textures[static_cast<size_t>(eInput)];
	}
	uint32_t Part_Count() const { return static_cast<uint32_t>(m_Parts.size()); }
	const std::string& Part_Name(uint32_t iIndex) const;
	bool_t& Part_Visible(const uint32_t iIndex) { return m_Parts[iIndex].bVisible; }
	const std::string& Part_BaseAssetId(const uint32_t iIndex) const
	{
		return m_Parts[iIndex].strBaseAssetId;
	}
	HRESULT Set_PartBase(uint32_t iIndex, const std::string& strAssetId);
	bool_t Is_Skinned() const { return m_bSkinned; }
	uint32_t Animation_Count() const;
	const char_t* Animation_Name(uint32_t iIndex) const;
	f32_t Animation_DurationSeconds(uint32_t iIndex) const;
	bool_t Animation_Progress(f32_t& fOutSeconds, f32_t& fOutDurationSeconds) const;
	bool_t Is_Finished() const { return m_bFinished; }
	bool_t Is_Hidden() const { return m_bHidden; }
	void Set_Hidden(const bool_t bHidden) { m_bHidden = bHidden; }
	void Restart();
	static const std::string& Last_Error() { return s_strLastError; }

private:
	HRESULT Load_Texture(
		const std::string& strAssetId, ComPtr<ID3D11ShaderResourceView>& OutView);
	void Apply_Transform();
	void Sync_Animation(bool_t bRestart);
	HRESULT Bind_Common(const shared_ptr<Engine::CShader>& pShader);

private:
	SHAPE m_eShape = SHAPE::SPRITE;
	PARAMS m_Params;
	DESC m_CreationDesc;
	float4x4_t m_PivotWorld;
	float3_t m_vDisplacement = { 0.f, 0.f, 0.f };
	uint32_t m_iAppliedAnimationIndex = UINT32_MAX;
	std::vector<PART> m_Parts;
	f32_t m_fTime = 0.f;
	bool_t m_bFinished = false;
	bool_t m_bHidden = false;
	bool_t m_bSkinned = false;
	std::string m_strStatus;

	shared_ptr<Engine::CShader> m_pShader;
	shared_ptr<Engine::CModel> m_pModel;
	shared_ptr<Engine::CVIBuffer_Rect> m_pRect;
	std::array<ComPtr<ID3D11ShaderResourceView>,
		static_cast<size_t>(TEXTURE_INPUT::END)> m_Textures;
	static std::string s_strLastError;

public:
	static unique_ptr<CEffectPreviewV2> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
