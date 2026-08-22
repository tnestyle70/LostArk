#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

#include <array>
#include <cstdint>
#include <string>

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

	struct PARAMS final
	{
		float4_t vTint = { 1.f, 1.f, 1.f, 1.f };
		BLEND_MODE eBlend = BLEND_MODE::ADDITIVE;
		bool_t bBillboard = true;
		bool_t bDepthTest = true;
		float2_t vUVScale = { 1.f, 1.f };
		float2_t vBasePan = { 0.f, 0.f };
		f32_t fNoiseStrength = 0.f;
		f32_t fNoiseScale = 1.f;
		float2_t vNoisePan = { 0.f, 0.f };
		f32_t fEmissiveIntensity = 1.f;
		f32_t fDissolveAmount = 0.f;
		f32_t fDissolveSoftness = 0.1f;
		f32_t fLifetime = 0.f;
		bool_t bLoop = true;
		f32_t fPlayRate = 1.f;
		f32_t fMeshPreScale = 0.01f;
	};

	struct DESC final : public GAMEOBJECT_DESC
	{
		SHAPE eShape = SHAPE::SPRITE;
		std::string strMeshAssetId;
		std::array<std::string, static_cast<size_t>(TEXTURE_INPUT::END)> TextureAssetIds;
		float3_t vPosition = { 0.f, 0.f, 0.f };
		float3_t vRotationDegrees = { 0.f, 0.f, 0.f };
		float3_t vScale = { 1.f, 1.f, 1.f };
		PARAMS Params;
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
	float3_t& Position() { return m_vPosition; }
	float3_t& RotationDegrees() { return m_vRotationDegrees; }
	float3_t& Scale() { return m_vScale; }
	const std::string& Status() const { return m_strStatus; }
	SHAPE Shape() const { return m_eShape; }
	f32_t Time() const { return m_fTime; }
	bool_t Is_Finished() const { return m_bFinished; }
	bool_t Is_Hidden() const { return m_bHidden; }
	void Set_Hidden(const bool_t bHidden) { m_bHidden = bHidden; }
	void Restart();
	static const std::string& Last_Error() { return s_strLastError; }

private:
	HRESULT Load_Texture(
		const std::string& strAssetId, ComPtr<ID3D11ShaderResourceView>& OutView);
	void Apply_Transform();
	HRESULT Bind_Common(const shared_ptr<Engine::CShader>& pShader);

private:
	SHAPE m_eShape = SHAPE::SPRITE;
	PARAMS m_Params;
	float3_t m_vPosition = { 0.f, 0.f, 0.f };
	float3_t m_vRotationDegrees = { 0.f, 0.f, 0.f };
	float3_t m_vScale = { 1.f, 1.f, 1.f };
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
