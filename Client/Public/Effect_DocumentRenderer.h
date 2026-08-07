#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Playback.h"

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
class CVIBuffer_Rect;
class CVIBuffer_ParticleRect;
class CVIBuffer_DynamicTrail;
NS_END

NS_BEGIN(Client)

class CEffectDocumentRenderer final
{
private:
	struct ELEMENT_RESOURCE final
	{
		shared_ptr<Engine::CModel> pModel;
		std::array<ComPtr<ID3D11ShaderResourceView>, 5> Textures;
		std::array<ComPtr<ID3D11ShaderResourceView>, 7> SourceTextures;
		uint32_t iSourceTextureMask = 0u;
		uint32_t iSourceMaterialProfile = 0u;
		float4_t vSourceScalars0{};
		float4_t vSourceScalars1{};
		float4_t vSourceVector0{};
		float4_t vSourceVector1{};
		std::array<float4_t, 16u> LinearFlowParameters{};
		float4_t vLinearFlowMaskAColor{ 1.f, 1.f, 1.f, 1.f };
		float4_t vLinearFlowMaskBColor{ 1.f, 1.f, 1.f, 1.f };
		std::array<uint32_t, 4u> DynamicParameterSemantics{};
		EFFECT_GROUPED_TRANSLUCENT_CONSTANTS GroupedConstants;
		bool_t bSourceMaterialFallbackBlocked = false;
	};
	struct MODEL_CUE_RESOURCE final
	{
		shared_ptr<Engine::CModel> pModel;
		uint32_t iAnimationIndex = 0u;
		f32_t fTicksPerSecond = 0.f;
	};

public:
	CEffectDocumentRenderer(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CEffectDocumentRenderer();

	HRESULT Initialize();
	bool_t Stage_Document(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	HRESULT Render(const EFFECT_EVALUATED_FRAME& Frame);
	void Clear();
	const std::string& Get_Status() const { return m_strStatus; }

private:
	HRESULT Stage_ElementResource(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_RESOURCE& OutResource,
		std::string& strOutError) const;
	HRESULT Stage_ModelCueResource(
		const EFFECT_MODEL_CUE_DESC& Cue,
		MODEL_CUE_RESOURCE& OutResource,
		std::string& strOutError) const;
	HRESULT Load_Texture(
		const std::string& strAssetId,
		ComPtr<ID3D11ShaderResourceView>& OutSRV) const;
	HRESULT Create_FallbackTextures();
	HRESULT Bind_Common(
		const shared_ptr<Engine::CShader>& pShader,
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f) const;
	HRESULT Bind_Common(
		const shared_ptr<Engine::CShader>& pShader,
		const EFFECT_ELEMENT_DESC& Element,
		const EFFECT_COLOR_DESC& Color,
		f32_t fLocalTimeSeconds,
		f32_t fNormalizedLife,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f) const;
	HRESULT Bind_MaterialInputs(
		const shared_ptr<Engine::CShader>& pShader,
		const EFFECT_ELEMENT_DESC& Element,
		const EFFECT_COLOR_DESC& Color,
		f32_t fLocalTimeSeconds,
		f32_t fNormalizedLife,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f) const;
	HRESULT Render_Element(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource);
	HRESULT Render_Mesh(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f,
		const float4x4_t* pWorldOverride = nullptr,
		const float4_t* pDynamicParameter = nullptr);
	HRESULT Render_Rect(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f,
		const float4x4_t* pWorldOverride = nullptr);
	HRESULT Render_Decal(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource);
	HRESULT Render_Particles(
		const EFFECT_EVALUATED_FRAME& Frame,
		const std::string& strElementId);
	HRESULT Render_Trails(
		const EFFECT_EVALUATED_FRAME& Frame,
		const std::string& strElementId);
	HRESULT Render_AfterImages(
		const EFFECT_EVALUATED_FRAME& Frame,
		const std::string& strElementId);
	HRESULT Render_ModelCues(const EFFECT_EVALUATED_FRAME& Frame);
	uint32_t Select_Pass(EFFECT_RENDER_PROFILE eProfile) const;
	const ELEMENT_RESOURCE* Find_Resource(const std::string& strElementId) const;

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	EFFECT_DOCUMENT_DESC m_Document;
	std::unordered_map<std::string, ELEMENT_RESOURCE> m_Resources;
	std::unordered_map<std::string, MODEL_CUE_RESOURCE> m_ModelCueResources;
	shared_ptr<Engine::CShader> m_pMeshShader;
	shared_ptr<Engine::CShader> m_pAnimatedModelShader;
	shared_ptr<Engine::CShader> m_pRectShader;
	shared_ptr<Engine::CShader> m_pParticleShader;
	shared_ptr<Engine::CShader> m_pTrailShader;
	shared_ptr<Engine::CShader> m_pDecalShader;
	shared_ptr<Engine::CVIBuffer_Rect> m_pRect;
	shared_ptr<Engine::CVIBuffer_ParticleRect> m_pParticleBuffer;
	shared_ptr<Engine::CVIBuffer_DynamicTrail> m_pTrailBuffer;
	ComPtr<ID3D11ShaderResourceView> m_pWhiteTexture;
	ComPtr<ID3D11ShaderResourceView> m_pBlackTexture;
	std::string m_strStatus;
};

NS_END

