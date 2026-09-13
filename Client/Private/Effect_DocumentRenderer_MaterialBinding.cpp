#include "Effect_DocumentRenderer_Internal.h"
#include "GameInstance.h"
#include "Presentation_Manager.h"
#include "Profiler.h"
#include "Shader.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
#include <d3d11sdklayers.h>
#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Model.h"
#include "Engine_RenderTypes.h"
#include "VIBuffer_Rect.h"

const Client::CEffectDocumentRenderer::ELEMENT_RESOURCE*
Client::CEffectDocumentRenderer::Find_Resource(
	const std::string& strElementId) const
{
	if (nullptr == m_pPreparedDocument)
		return nullptr;
	const auto Iterator =
		m_pPreparedDocument->ElementResources.find(strElementId);
	return Iterator == m_pPreparedDocument->ElementResources.end() ?
		nullptr : &Iterator->second;
}

uint32_t Client::CEffectDocumentRenderer::Select_Pass(
	const EFFECT_RENDER_PROFILE eProfile) const
{
	switch (eProfile)
	{
	case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE: return 0u;
	case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ: return 1u;
	case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ: return 2u;
	case EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ: return 3u;
	case EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ: return 4u;
	case EFFECT_RENDER_PROFILE::END:
	default: return UINT32_MAX;
	}
}

HRESULT Client::CEffectDocumentRenderer::Fail_RenderOperation(
	std::string strOperation,
	const HRESULT hResult,
	const bool_t bObjectLocal)
{
	if (m_strRenderFailureDetail.empty())
		m_strRenderFailureDetail = std::move(strOperation);
	m_bLastRenderFailureObjectLocal =
		m_bLastRenderFailureObjectLocal || bObjectLocal;
	return FAILED(hResult) ? hResult : E_FAIL;
}

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
void Client::CEffectDocumentRenderer::Record_TestMaterialBinding()
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	++m_pActiveOccurrenceStats->iMaterialBindCount;
	/* Bind_MaterialInputs commits five standard SRVs and seven source SRVs.
	   Missing optional lanes are deliberately bound to typed white/black
	   resources, so every successful call has the same complete shader seam. */
	m_pActiveOccurrenceStats->iTextureSrvBindCount += 12u;
}

void Client::CEffectDocumentRenderer::Record_TestSamplerBinding()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iSamplerBindCount;
}

void Client::CEffectDocumentRenderer::Record_TestShaderPassApplication()
{
	if (nullptr != m_pActiveOccurrenceStats)
	{
		++m_pActiveOccurrenceStats->iShaderPassApplyCount;
		/* CShader::Begin applies the selected Effect pass, including its compiled
		   LinearSampler state. RuntimeMaterialV2 may then replace that PS sampler;
		   its successful override is recorded independently below. */
		++m_pActiveOccurrenceStats->iSamplerBindCount;
	}
}

void Client::CEffectDocumentRenderer::Record_TestGeometryUpload()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iGeometryUploadCount;
}

void Client::CEffectDocumentRenderer::Record_TestTrailGeometryUpload(
	const std::span<const Engine::VTXEFFECT_TRAIL> Vertices)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	++m_pActiveOccurrenceStats->iGeometryUploadCount;
	m_pActiveOccurrenceStats->iFinalTrailUploadedVertexCount += Vertices.size();
	for (size_t iVertex = 0u; iVertex + 1u < Vertices.size(); iVertex += 2u)
	{
		const float3_t& Left = Vertices[iVertex].vPosition;
		const float3_t& Right = Vertices[iVertex + 1u].vPosition;
		const float3_t Center = {
			(Left.x + Right.x) * 0.5f,
			(Left.y + Right.y) * 0.5f,
			(Left.z + Right.z) * 0.5f };
		const f32_t DeltaX = Right.x - Left.x;
		const f32_t DeltaY = Right.y - Left.y;
		const f32_t DeltaZ = Right.z - Left.z;
		const f32_t Width = std::sqrt(
			DeltaX * DeltaX + DeltaY * DeltaY + DeltaZ * DeltaZ);
		if (!m_pActiveOccurrenceStats->bHasFinalTrailPairCenter)
		{
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin = Center;
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax = Center;
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMin = Width;
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMax = Width;
			m_pActiveOccurrenceStats->bHasFinalTrailPairCenter = true;
			continue;
		}
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.x = (std::min)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.x, Center.x);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.y = (std::min)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.y, Center.y);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.z = (std::min)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.z, Center.z);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.x = (std::max)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.x, Center.x);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.y = (std::max)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.y, Center.y);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.z = (std::max)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.z, Center.z);
		m_pActiveOccurrenceStats->fFinalTrailPairWidthMin = (std::min)(
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMin, Width);
		m_pActiveOccurrenceStats->fFinalTrailPairWidthMax = (std::max)(
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMax, Width);
	}
}

void Client::CEffectDocumentRenderer::Record_TestVIBufferBinding()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iVIBufferBindCount;
}

void Client::CEffectDocumentRenderer::Record_TestDrawSelection(
	const EFFECT_GPU_RENDER_CARRIER eCarrier,
	const uint32_t iSelectedPassIndex)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (0u == m_pActiveOccurrenceStats->iDrawSelectionCount)
	{
		m_pActiveOccurrenceStats->eCarrier = eCarrier;
		m_pActiveOccurrenceStats->iSelectedPassIndex = iSelectedPassIndex;
	}
	else if (m_pActiveOccurrenceStats->eCarrier != eCarrier ||
		m_pActiveOccurrenceStats->iSelectedPassIndex != iSelectedPassIndex)
	{
		m_pActiveOccurrenceStats->bDrawSelectionDiverged = true;
	}
	if (EFFECT_GPU_RENDER_CARRIER::END == eCarrier ||
		UINT32_MAX == iSelectedPassIndex)
	{
		m_pActiveOccurrenceStats->bDrawSelectionDiverged = true;
	}
	++m_pActiveOccurrenceStats->iDrawSelectionCount;
}

void Client::CEffectDocumentRenderer::
	Record_TestCompiledAdapterPipelineValidation()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iCompiledAdapterPipelineValidationCount;
}

namespace
{
	void Extend_SubmittedPosition(
		Client::EFFECT_GPU_RENDER_OCCURRENCE_STATS& Stats,
		const float3_t& Position)
	{
		if (!Stats.bHasSubmittedPosition)
		{
			Stats.vSubmittedPositionMin = Position;
			Stats.vSubmittedPositionMax = Position;
			Stats.bHasSubmittedPosition = true;
			return;
		}
		Stats.vSubmittedPositionMin.x =
			(std::min)(Stats.vSubmittedPositionMin.x, Position.x);
		Stats.vSubmittedPositionMin.y =
			(std::min)(Stats.vSubmittedPositionMin.y, Position.y);
		Stats.vSubmittedPositionMin.z =
			(std::min)(Stats.vSubmittedPositionMin.z, Position.z);
		Stats.vSubmittedPositionMax.x =
			(std::max)(Stats.vSubmittedPositionMax.x, Position.x);
		Stats.vSubmittedPositionMax.y =
			(std::max)(Stats.vSubmittedPositionMax.y, Position.y);
		Stats.vSubmittedPositionMax.z =
			(std::max)(Stats.vSubmittedPositionMax.z, Position.z);
	}
}

void Client::CEffectDocumentRenderer::Record_TestIssuedDraw(
	const float4x4_t& World)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (UINT64_MAX == m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal)
		m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal =
			m_iTestIssuedDrawOrdinal;
	++m_iTestIssuedDrawOrdinal;
	++m_pActiveOccurrenceStats->iVIBufferDrawCount;
	++m_pActiveOccurrenceStats->iIssuedDrawCallCount;
	if (!m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld)
	{
		m_pActiveOccurrenceStats->FirstSubmittedParticleWorld = World;
		m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld = true;
	}
	Extend_SubmittedPosition(*m_pActiveOccurrenceStats,
		{ World._41, World._42, World._43 });
}

void Client::CEffectDocumentRenderer::Record_TestIssuedDraw(
	const std::span<const Engine::VTXEFFECT_PARTICLE> Instances)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (UINT64_MAX == m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal)
		m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal =
			m_iTestIssuedDrawOrdinal;
	++m_iTestIssuedDrawOrdinal;
	++m_pActiveOccurrenceStats->iVIBufferDrawCount;
	++m_pActiveOccurrenceStats->iIssuedDrawCallCount;
	for (const Engine::VTXEFFECT_PARTICLE& Instance : Instances)
	{
		if (!m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld)
		{
			m_pActiveOccurrenceStats->FirstSubmittedParticleWorld = Instance.World;
			m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld = true;
		}
		Extend_SubmittedPosition(*m_pActiveOccurrenceStats,
			{ Instance.World._41, Instance.World._42, Instance.World._43 });
	}
}

void Client::CEffectDocumentRenderer::Record_TestIssuedDraw(
	const std::span<const Engine::VTXEFFECT_TRAIL> Vertices)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (UINT64_MAX == m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal)
		m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal =
			m_iTestIssuedDrawOrdinal;
	++m_iTestIssuedDrawOrdinal;
	++m_pActiveOccurrenceStats->iVIBufferDrawCount;
	++m_pActiveOccurrenceStats->iIssuedDrawCallCount;
	for (const Engine::VTXEFFECT_TRAIL& Vertex : Vertices)
		Extend_SubmittedPosition(*m_pActiveOccurrenceStats, Vertex.vPosition);
}
#endif

HRESULT Client::CEffectDocumentRenderer::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader,
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const EFFECT_MATERIAL_DESC* pMaterialOverride,
    const EFFECT_SHADER_PROGRAM_DESC* pShaderProgram)
{
	return Bind_Common(pShader, *Element.pElement, Element.Color,
		Element.fLocalTimeSeconds, Element.fNormalizedLife,
		Resource, fAlphaScale, pMaterialOverride, pShaderProgram);
}

HRESULT Client::CEffectDocumentRenderer::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader,
	const EFFECT_ELEMENT_DESC& Element,
	const EFFECT_COLOR_DESC& Color,
	const f32_t fLocalTimeSeconds,
	const f32_t fNormalizedLife,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const EFFECT_MATERIAL_DESC* pMaterialOverride,
    const EFFECT_SHADER_PROGRAM_DESC* pShaderProgram)
{
	if (nullptr == pShader)
		return Fail_RenderOperation(
			"Common shader bind failed: shader is null.", E_POINTER);
	HRESULT hResult = pShader->Bind_Matrix("g_ViewMatrix",
		CGameInstance::Get().Get_Transform(D3DTS::VIEW));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Common shader bind failed: g_ViewMatrix.", hResult);
	hResult = pShader->Bind_Matrix("g_ProjMatrix",
		CGameInstance::Get().Get_Transform(D3DTS::PROJ));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Common shader bind failed: g_ProjMatrix.", hResult);
	if ((nullptr != pShaderProgram && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE))
	{
		const float4_t CameraPosition = *CGameInstance::Get().Get_CamPosition();
		hResult = pShader->Bind_RawValue("g_CameraPosition",
			&CameraPosition, sizeof(CameraPosition));
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Particle camera-position shader binding failed.", hResult);
	}
	return Bind_MaterialInputs(pShader, Element, Color,
		fLocalTimeSeconds, fNormalizedLife, Resource, fAlphaScale, pMaterialOverride, pShaderProgram);
}

HRESULT Client::CEffectDocumentRenderer::Bind_MaterialInputs(
	const shared_ptr<Engine::CShader>& pShader,
	const EFFECT_ELEMENT_DESC& Element,
	const EFFECT_COLOR_DESC& Color,
	const f32_t fLocalTimeSeconds,
	const f32_t fNormalizedLife,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const EFFECT_MATERIAL_DESC* pMaterialOverride,
    const EFFECT_SHADER_PROGRAM_DESC* pShaderProgram)
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Material.Bind");
	if (FAILED(Bind_BloomInputs(pShader))) return E_FAIL;
	const EFFECT_MATERIAL_DESC& Material = nullptr != pMaterialOverride ?
		*pMaterialOverride : Element.Material;
	if (nullptr == pShader)
		return Fail_RenderOperation(
			"Material bind failed: shader is null.", E_INVALIDARG);
	HRESULT hFirstBindFailure = S_OK;
	const auto BindFailed = [&hFirstBindFailure](const HRESULT hResult)
	{
		if (SUCCEEDED(hResult))
			return false;
		if (SUCCEEDED(hFirstBindFailure))
			hFirstBindFailure = hResult;
		return true;
	};
    // Fixed decal/trail carriers share the admitted native parameter packet.
    const bool bKoukuFixedNative = nullptr == pShaderProgram &&
        Resource.iSourceMaterialProfile >= 2304u && Resource.iSourceMaterialProfile <= 3711u &&
        (pShader == m_pDecalShader || pShader == m_pTrailShader);
    if (bKoukuFixedNative)
    {
        if (BindFailed(pShader->Bind_RawValue("g_ArtistSourceMaterialParameters",
                Resource.ArtistSourceMaterialParameters.data(), sizeof(Resource.ArtistSourceMaterialParameters))) ||
            BindFailed(pShader->Bind_RawValue("g_ArtistSourceMaterialTime", &fLocalTimeSeconds, sizeof(fLocalTimeSeconds))))
            return Fail_RenderOperation("Kouku native fixed-carrier parameter binding failed.", hFirstBindFailure);
        if (Resource.bSourceRequiresSceneDepth &&
            BindFailed(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), pShader, "g_EffectSceneDepthTexture")))
            return Fail_RenderOperation("Kouku fixed-carrier source depth is unavailable.", hFirstBindFailure);
        if (Resource.bSourceRequiresSceneColor &&
            (BindFailed(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_EffectSceneColor"), pShader, "g_EffectSceneColorTexture")) ||
             BindFailed(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_EffectSceneBloom"), pShader, "g_EffectSceneBloomTexture"))))
            return Fail_RenderOperation("Kouku fixed-carrier source scene color is unavailable.", hFirstBindFailure);
        if (pShader == m_pTrailShader)
        {
            // Native ribbon distortion projects source world centimeters once.
            const matrix_t SourceToClient = XMMatrixSet(.01f,0.f,0.f,0.f,
                0.f,0.f,-.01f,0.f, 0.f,.01f,0.f,0.f, 0.f,0.f,0.f,1.f);
            float4x4_t SourceProjection;
            XMStoreFloat4x4(&SourceProjection, SourceToClient *
                XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW)) *
                XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
            for (auto& Row : SourceProjection.m) for (auto& Value : Row) Value *= 100.f;
            if (BindFailed(pShader->Bind_Matrix("g_KoukuSourceProjection", &SourceProjection)))
                return Fail_RenderOperation("Kouku ribbon source projection binding failed.", hFirstBindFailure);
        }
        if (pShader == m_pDecalShader)
        {
            // Match the existing native model/mesh adapter: committed scene ambient;
            // UE skylight hemispheres have no scene owner and remain disabled.
            float4_t Ambient{0.f, 0.f, 0.f, 1.f};
            for (const auto& Light : CGameInstance::Get().Get_SceneLights())
                if (Light.eType == LIGHT::DIRECTIONAL)
                { Ambient.x += Light.vAmbient.x; Ambient.y += Light.vAmbient.y; Ambient.z += Light.vAmbient.z; }
            if (BindFailed(pShader->Bind_RawValue("g_KoukuDecalAmbient", &Ambient, sizeof(Ambient))))
                return Fail_RenderOperation("Kouku decal ambient binding failed.", hFirstBindFailure);
        }
    }
    const bool_t bGenericCarrierShader = nullptr != pShaderProgram &&
        pShaderProgram->eFamily == EFFECT_SHADER_FAMILY::GENERIC;
    // The native ARTIST particle programs consume their source packet and
    // shared UV/color inputs, but never the generic material textures/effects.
    // Match the selected executable, not an authored native-profile flag: mesh
    // and fixed carriers continue to receive the complete common packet.
    const bool_t bNativeArtistParticle = nullptr != pShaderProgram &&
        pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE &&
        pShaderProgram->eFamily == EFFECT_SHADER_FAMILY::ARTIST &&
        pShaderProgram == Get_EffectShaderProgram(Resource.iShaderProgramIndex) &&
        Resource.iShaderProgramIndex < m_ShaderPrograms.size() &&
        pShader == m_ShaderPrograms[Resource.iShaderProgramIndex] &&
        Resource.iSourceMaterialProfile >= pShaderProgram->iFirstProfile &&
        Resource.iSourceMaterialProfile <= pShaderProgram->iLastProfile;
	if ((nullptr != pShaderProgram && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::MESH) || (nullptr != pShaderProgram && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE))
	{
        const auto BindNativePacket = [&](const char* pParameters, const char* pTime,
            const auto& Parameters)
        {
            return BindFailed(pShader->Bind_RawValue(pParameters, Parameters.data(), static_cast<uint32_t>(sizeof(Parameters)))) ||
                BindFailed(pShader->Bind_RawValue(pTime, &fLocalTimeSeconds, sizeof(fLocalTimeSeconds)));
        };
        bool NativeBindFailed = false;
        switch (pShaderProgram->eFamily)
        {
        case EFFECT_SHADER_FAMILY::GENERIC: break;
        case EFFECT_SHADER_FAMILY::DIMENSIONMASTER_Q:
            NativeBindFailed = BindNativePacket("g_QSourceMaterialParameters", "g_QSourceMaterialTime", Resource.QSourceMaterialParameters); break;
        case EFFECT_SHADER_FAMILY::DIMENSIONMASTER_V:
        case EFFECT_SHADER_FAMILY::DIMENSIONMASTER_WR:
        case EFFECT_SHADER_FAMILY::DIMENSIONMASTER_SD:
            NativeBindFailed = BindNativePacket("g_VSourceMaterialParameters", "g_VSourceMaterialTime", Resource.VSourceMaterialParameters); break;
        case EFFECT_SHADER_FAMILY::DIMENSIONMASTER_ALTV:
            NativeBindFailed = BindNativePacket("g_ALTVSourceMaterialParameters", "g_ALTVSourceMaterialTime", Resource.ALTVSourceMaterialParameters); break;
        case EFFECT_SHADER_FAMILY::ARTIST:
        {
            NativeBindFailed = BindNativePacket("g_ArtistSourceMaterialParameters", "g_ArtistSourceMaterialTime", Resource.ArtistSourceMaterialParameters);
            if (Resource.iSourceMaterialProfile >= 2304u && Resource.iSourceMaterialProfile <= 3711u)
            {
                float4_t SceneAmbient{0.f, 0.f, 0.f, 1.f};
                for (const auto& Light : CGameInstance::Get().Get_SceneLights())
                    if (Light.eType == LIGHT::DIRECTIONAL)
                    { SceneAmbient.x += Light.vAmbient.x; SceneAmbient.y += Light.vAmbient.y; SceneAmbient.z += Light.vAmbient.z; }
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_KoukuSourceAmbient", &SceneAmbient, sizeof(SceneAmbient))) || NativeBindFailed;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_KoukuSourceActorPosition", &m_vSourceActorPosition, sizeof(m_vSourceActorPosition))) || NativeBindFailed;
                // UE source world is centimeters in X,Z,-Y. Source clip W is
                // likewise centimeters, matching the native depth adapters.
                const matrix_t SourceToClient = XMMatrixSet(.01f,0.f,0.f,0.f,
                    0.f,0.f,-.01f,0.f, 0.f,.01f,0.f,0.f, 0.f,0.f,0.f,1.f);
                float4x4_t SourceProjection;
                XMStoreFloat4x4(&SourceProjection, SourceToClient *
                    XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW)) *
                    XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
                for (auto& Row : SourceProjection.m) for (auto& Value : Row) Value *= 100.f;
                NativeBindFailed = BindFailed(pShader->Bind_Matrix("g_KoukuSourceProjection", &SourceProjection)) || NativeBindFailed;
            }
            break;
        }
        case EFFECT_SHADER_FAMILY::LANCE_MASTER:
            NativeBindFailed = BindNativePacket("g_LanceVASourceMaterialParameters", "g_LanceVASourceMaterialTime", Resource.LanceVASourceMaterialParameters); break;
        case EFFECT_SHADER_FAMILY::WARLORD:
        {
            NativeBindFailed = BindNativePacket("g_WarlordSourceMaterialParameters", "g_WarlordSourceMaterialTime", Resource.VSourceMaterialParameters);
            float4_t SceneAmbient{0.f, 0.f, 0.f, 1.f};
            const float4_t NoHemisphere{};
            for (const auto& Light : CGameInstance::Get().Get_SceneLights())
                if (Light.eType == LIGHT::DIRECTIONAL)
                { SceneAmbient.x += Light.vAmbient.x; SceneAmbient.y += Light.vAmbient.y; SceneAmbient.z += Light.vAmbient.z; }
            NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordSkyUpper", &NoHemisphere, sizeof(NoHemisphere))) || NativeBindFailed;
            NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordSkyLower", &NoHemisphere, sizeof(NoHemisphere))) || NativeBindFailed;
            NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordAmbient", &SceneAmbient, sizeof(SceneAmbient))) || NativeBindFailed;
            if (Resource.iSourceMaterialProfile == 1122u || Resource.iSourceMaterialProfile == 1123u)
            {
                // Current committed contracts: at most 16 scene + 64 transient lights.
                // Preserve the same scene-then-transient order as CLight_Manager::Render_Lights.
                std::array<float4_t, 80> Directions{};
                std::array<float4_t, 80> Positions{};
                std::array<float4_t, 80> Colors{};
                std::array<float4_t, 80> SpotCones{};
                uint32_t iLightCount = 0u;
                const auto AppendLight = [&](const LIGHT_DESC& Light) -> HRESULT
                {
                    if (iLightCount == Directions.size())
                        return Fail_RenderOperation("Guardian PBR light packet exceeds the 16 scene + 64 transient contract.", E_INVALIDARG);
                    float4_t Direction{};
                    if (Light.eType == LIGHT::DIRECTIONAL || Light.eType == LIGHT::SPOT)
                    {
                        const float fLengthSquared = Light.vDirection.x * Light.vDirection.x +
                            Light.vDirection.y * Light.vDirection.y + Light.vDirection.z * Light.vDirection.z;
                        if (!std::isfinite(fLengthSquared) || fLengthSquared <= 1.e-12f)
                            return Fail_RenderOperation("Guardian PBR light has an invalid direction.", E_INVALIDARG);
                        const float fInverseLength = 1.f / std::sqrt(fLengthSquared);
                        const float fSign = Light.eType == LIGHT::DIRECTIONAL ? -1.f : 1.f;
                        Direction = float4_t(Light.vDirection.x * fInverseLength * fSign,
                            Light.vDirection.y * fInverseLength * fSign, Light.vDirection.z * fInverseLength * fSign, 0.f);
                    }
                    else if (Light.eType != LIGHT::POINT)
                        return Fail_RenderOperation("Guardian PBR light has an unsupported type.", E_INVALIDARG);
                    Direction.w = Light.eType == LIGHT::DIRECTIONAL ? 0.f : (Light.eType == LIGHT::POINT ? 1.f : 2.f);
                    Directions[iLightCount] = Direction;
                    Positions[iLightCount] = float4_t(Light.vPosition.x, Light.vPosition.y, Light.vPosition.z, Light.fRange);
                    Colors[iLightCount] = float4_t(Light.vDiffuse.x, Light.vDiffuse.y, Light.vDiffuse.z, Light.fFalloffExponent);
                    SpotCones[iLightCount] = float4_t(Light.fSpotInnerCos, Light.fSpotOuterCos, 0.f, 0.f);
                    ++iLightCount;
                    return S_OK;
                };
                for (const auto& Light : CGameInstance::Get().Get_SceneLights())
                    if (FAILED(AppendLight(Light)))
                        return E_INVALIDARG;
                for (const auto& Light : CPresentation_Manager::Get().Get_TransientLights())
                    if (FAILED(AppendLight(Light)))
                        return E_INVALIDARG;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordGuardianLightCount", &iLightCount, sizeof(iLightCount))) || NativeBindFailed;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordGuardianDirections", Directions.data(), static_cast<uint32_t>(sizeof(Directions)))) || NativeBindFailed;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordGuardianPositions", Positions.data(), static_cast<uint32_t>(sizeof(Positions)))) || NativeBindFailed;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordGuardianColors", Colors.data(), static_cast<uint32_t>(sizeof(Colors)))) || NativeBindFailed;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordGuardianSpotCones", SpotCones.data(), static_cast<uint32_t>(sizeof(SpotCones)))) || NativeBindFailed;
            }
            break;
        }
        default: return Fail_RenderOperation("Native shader family is invalid.", E_INVALIDARG, true);
        }
        if (NativeBindFailed)
            return Fail_RenderOperation("Selected native family material parameter binding failed.", hFirstBindFailure);
        if (bGenericCarrierShader)
        {
		if (BindFailed(pShader->Bind_RawValue(
			"g_ReconstructedMaterialEvaluatorEnabled",
			&Resource.iReconstructedMaterialEvaluatorEnabled,
			sizeof(Resource.iReconstructedMaterialEvaluatorEnabled))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_ReconstructedMaterialFeatureMask",
				&Resource.iReconstructedMaterialFeatureMask,
				sizeof(Resource.iReconstructedMaterialFeatureMask))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedUVScale",
				&Resource.vReconstructedUVScale,
				sizeof(Resource.vReconstructedUVScale))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedPanRotationAux",
				&Resource.vReconstructedPanRotationAux,
				sizeof(Resource.vReconstructedPanRotationAux))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedColor",
				&Resource.vReconstructedColor,
				sizeof(Resource.vReconstructedColor))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedParams0",
				&Resource.vReconstructedParams0,
				sizeof(Resource.vReconstructedParams0))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedParams1",
				&Resource.vReconstructedParams1,
				sizeof(Resource.vReconstructedParams1))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4Opcode",
				&Resource.iArtistVisualV4Opcode,
				sizeof(Resource.iArtistVisualV4Opcode))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4TextureMask",
				&Resource.iArtistVisualV4TextureMask,
				sizeof(Resource.iArtistVisualV4TextureMask))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4Params",
				Resource.ArtistVisualV4Params.data(),
				sizeof(Resource.ArtistVisualV4Params))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4Colors",
				Resource.ArtistVisualV4Colors.data(),
				sizeof(Resource.ArtistVisualV4Colors))))
		{
			return Fail_RenderOperation(
				"Material bind failed: reconstructed evaluator block.",
				hFirstBindFailure);
		}
        }
	}
	const bool_t bRuntimeMaterialV2Shader = bGenericCarrierShader || pShader == m_pDecalShader ||
		pShader == m_pTrailShader || pShader == m_pRectShader;
	if (bRuntimeMaterialV2Shader)
	{
		if (
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2Enabled",
				&Resource.iRuntimeMaterialV2Enabled,
				sizeof(Resource.iRuntimeMaterialV2Enabled))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2Opcode",
				&Resource.iRuntimeMaterialV2Opcode,
				sizeof(Resource.iRuntimeMaterialV2Opcode))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2TextureLaneCount",
				&Resource.iRuntimeMaterialV2TextureLaneCount,
				sizeof(Resource.iRuntimeMaterialV2TextureLaneCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2TextureMask",
				&Resource.iRuntimeMaterialV2TextureMask,
				sizeof(Resource.iRuntimeMaterialV2TextureMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2DynamicConsumedMask",
				&Resource.iRuntimeMaterialV2DynamicConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2DynamicConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2DynamicSuppressedMask",
				&Resource.iRuntimeMaterialV2DynamicSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2DynamicSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2ParticleColorPolicy",
				&Resource.iRuntimeMaterialV2ParticleColorPolicy,
				sizeof(Resource.iRuntimeMaterialV2ParticleColorPolicy))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2ParticleColorConsumedMask",
				&Resource.iRuntimeMaterialV2ParticleColorConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2ParticleColorConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2ParticleColorSuppressedMask",
				&Resource.iRuntimeMaterialV2ParticleColorSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2ParticleColorSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2ScalarCount",
				&Resource.iRuntimeMaterialV2ScalarCount,
				sizeof(Resource.iRuntimeMaterialV2ScalarCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2VectorCount",
				&Resource.iRuntimeMaterialV2VectorCount,
				sizeof(Resource.iRuntimeMaterialV2VectorCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2InputCount",
				&Resource.iRuntimeMaterialV2InputCount,
				sizeof(Resource.iRuntimeMaterialV2InputCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2InputConsumedMask",
				Resource.RuntimeMaterialV2InputConsumedMask.data(),
				sizeof(Resource.RuntimeMaterialV2InputConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2InputSuppressedMask",
				Resource.RuntimeMaterialV2InputSuppressedMask.data(),
				sizeof(Resource.RuntimeMaterialV2InputSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticInputCount",
				&Resource.iRuntimeMaterialV2StaticInputCount,
				sizeof(Resource.iRuntimeMaterialV2StaticInputCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticSelectedMask",
				&Resource.iRuntimeMaterialV2StaticSelectedMask,
				sizeof(Resource.iRuntimeMaterialV2StaticSelectedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticConsumedMask",
				&Resource.iRuntimeMaterialV2StaticConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2StaticConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticSuppressedMask",
				&Resource.iRuntimeMaterialV2StaticSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2StaticSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2RenderInputCount",
				&Resource.iRuntimeMaterialV2RenderInputCount,
				sizeof(Resource.iRuntimeMaterialV2RenderInputCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2RenderConsumedMask",
				&Resource.iRuntimeMaterialV2RenderConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2RenderConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2RenderSuppressedMask",
				&Resource.iRuntimeMaterialV2RenderSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2RenderSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2ScalarBlocks",
				Resource.RuntimeMaterialV2ScalarBlocks.data(),
				sizeof(Resource.RuntimeMaterialV2ScalarBlocks))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2VectorComponentConsumedMask",
				Resource.RuntimeMaterialV2VectorComponentConsumedMask.data(),
				sizeof(Resource.RuntimeMaterialV2VectorComponentConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2VectorComponentSuppressedMask",
				Resource.RuntimeMaterialV2VectorComponentSuppressedMask.data(),
				sizeof(Resource.RuntimeMaterialV2VectorComponentSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2NormalizedLife",
				&fNormalizedLife, sizeof(fNormalizedLife))) ||
			BindFailed(pShader->Bind_RawValue("g_EffectLocalTime",
				&fLocalTimeSeconds, sizeof(fLocalTimeSeconds))))
		{
			return Fail_RenderOperation(
				"Material bind failed: RuntimeMaterialV2 scalar block.",
				hFirstBindFailure);
		}
	}
	if (bRuntimeMaterialV2Shader &&
		BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2Vectors",
			Resource.RuntimeMaterialV2Vectors.data(),
			sizeof(Resource.RuntimeMaterialV2Vectors))))
	{
		return Fail_RenderOperation(
			"Material bind failed: RuntimeMaterialV2 vector block.",
			hFirstBindFailure);
	}
	const bool_t bStandardColorV1Shader = bGenericCarrierShader ||
		pShader == m_pDecalShader || pShader == m_pTrailShader;
	if (bStandardColorV1Shader &&
		(BindFailed(pShader->Bind_RawValue("g_StandardColorV1Enabled",
			&Resource.iStandardColorV1Enabled,
			sizeof(Resource.iStandardColorV1Enabled))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Header",
			Resource.StandardColorV1Header.data(),
			sizeof(Resource.StandardColorV1Header))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1BaseCoverage",
			Resource.StandardColorV1BaseCoverage.data(),
			sizeof(Resource.StandardColorV1BaseCoverage))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Dissolve",
			Resource.StandardColorV1Dissolve.data(),
			sizeof(Resource.StandardColorV1Dissolve))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Policies",
			Resource.StandardColorV1Policies.data(),
			sizeof(Resource.StandardColorV1Policies))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Scalars",
			&Resource.vStandardColorV1Scalars,
			sizeof(Resource.vStandardColorV1Scalars)))))
	{
		return Fail_RenderOperation(
			"Material bind failed: StandardColorV1 packet.",
			hFirstBindFailure);
	}
	const std::string_view strSourceSubUVMode = SourceLiteralString(
		Element, "interpolationmethod");
	const uint32_t iSourceSubUVColumns = static_cast<uint32_t>((std::max)(
		1.f, SourceLiteralNumber(Element, "subimages_horizontal", 1.f)));
	const uint32_t iSourceSubUVRows = static_cast<uint32_t>((std::max)(
		1.f, SourceLiteralNumber(Element, "subimages_vertical", 1.f)));
	const uint64_t iSourceSubUVFrameCount =
		static_cast<uint64_t>(iSourceSubUVColumns) * iSourceSubUVRows;
	const bool_t bSourceSubUV = Element.SourceRecipe.bEnabled &&
		iSourceSubUVFrameCount > 1u && iSourceSubUVFrameCount <= UINT32_MAX &&
		!strSourceSubUVMode.empty() && strSourceSubUVMode != "none" &&
		strSourceSubUVMode != "psuvim_none";
	const bool_t bParticleLifeSubUV = (nullptr != pShaderProgram && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE) &&
		!Element.SourceRecipe.bEnabled && Element.Detail.Particle.bSubUVOverLife;
	const bool_t bParticleOwnsAtlas = bSourceSubUV || bParticleLifeSubUV;

	float2_t UVOffset(
		Element.Detail.UV.vStart.x + Element.Detail.UV.vSpeed.x * fLocalTimeSeconds,
		Element.Detail.UV.vStart.y + Element.Detail.UV.vSpeed.y * fLocalTimeSeconds);
	if (Element.Detail.UV.bWave)
	{
		const f32_t Wave = std::sin(
			XM_2PI * Element.Detail.UV.fWaveFrequency * fLocalTimeSeconds);
		UVOffset.x += Element.Detail.UV.vWaveAmplitude.x * Wave;
		UVOffset.y += Element.Detail.UV.vWaveAmplitude.y * Wave;
	}
	int32_t iTileIndex = Element.Detail.UV.iTileIndex;
	if (!bParticleOwnsAtlas && Element.Detail.UV.bSequence)
	{
		const int32_t iTileCount = Element.Detail.UV.iTileColumns *
			Element.Detail.UV.iTileRows;
		const int32_t iFrame = static_cast<int32_t>(
			fLocalTimeSeconds / Element.Detail.UV.fSequenceTerm) +
			Element.Detail.UV.iTileIndex;
		iTileIndex = Element.Detail.UV.bLoop ?
			((iFrame % iTileCount) + iTileCount) % iTileCount :
			std::clamp(iFrame, 0, iTileCount - 1);
	}
	const float2_t UVScale = bParticleOwnsAtlas ? float2_t(1.f, 1.f) : float2_t(
		1.f / Element.Detail.UV.iTileColumns,
		1.f / Element.Detail.UV.iTileRows);
	if (!bParticleOwnsAtlas)
	{
		UVOffset.x += static_cast<f32_t>(
			iTileIndex % Element.Detail.UV.iTileColumns) * UVScale.x;
		UVOffset.y += static_cast<f32_t>(
			iTileIndex / Element.Detail.UV.iTileColumns) * UVScale.y;
	}
	const f32_t Dissolve = bNativeArtistParticle || Element.Detail.Timing.fDissolveStartNormalized >= 1.f ?
		0.f : std::clamp(
			(fNormalizedLife - Element.Detail.Timing.fDissolveStartNormalized) /
			(1.f - Element.Detail.Timing.fDissolveStartNormalized), 0.f, 1.f);

	const float4_t AuthoredColorMultiply = bNativeArtistParticle ? float4_t{} :
		Evaluate_CommonColor(Element, fNormalizedLife).vColorMultiply;
	float4_t ColorMultiply = Color.vColorMultiply;
	ColorMultiply.w *= fAlphaScale;
	const uint32_t iHasNoise = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasMask = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::MASK_TEXTURE) ? 1u : 0u;
	const uint32_t iHasEmissive = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasDissolve = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasBase2 = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::BASE2_TEXTURE) ? 1u : 0u;
	const uint32_t iHasMask2 = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::MASK2_TEXTURE) ? 1u : 0u;
	const uint32_t iHasNoise2 = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE2_TEXTURE) ? 1u : 0u;
	const uint32_t iDistortionOnBase =
		Element.Detail.Color.bDistortionOnBaseMaterial ? 1u : 0u;
	if ((bGenericCarrierShader || pShader == m_pTrailShader) &&
		(BindFailed(pShader->Bind_RawValue("g_SourceMaterialProfile",
			&Resource.iSourceMaterialProfile,
			sizeof(Resource.iSourceMaterialProfile))) ||
		BindFailed(pShader->Bind_RawValue("g_AuthoredColorMultiply",
			&AuthoredColorMultiply, sizeof(AuthoredColorMultiply))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceScalars0",
			&Resource.vSourceScalars0,
			sizeof(Resource.vSourceScalars0))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceScalars1",
			&Resource.vSourceScalars1,
			sizeof(Resource.vSourceScalars1))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceVector0",
			&Resource.vSourceVector0,
			sizeof(Resource.vSourceVector0))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceVector1",
			&Resource.vSourceVector1,
			sizeof(Resource.vSourceVector1))) ||
		BindFailed(pShader->Bind_RawValue("g_TypedTrailParameters",
			Resource.TypedTrailParameters.data(),
			sizeof(Resource.TypedTrailParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceTextureMask",
			&Resource.iSourceTextureMask,
			sizeof(Resource.iSourceTextureMask))) ||
		BindFailed(pShader->Bind_RawValue("g_LinearFlowParameters",
			Resource.LinearFlowParameters.data(),
			sizeof(Resource.LinearFlowParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_LinearFlowMaskAColor",
			&Resource.vLinearFlowMaskAColor,
			sizeof(Resource.vLinearFlowMaskAColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LinearFlowMaskBColor",
			&Resource.vLinearFlowMaskBColor,
			sizeof(Resource.vLinearFlowMaskBColor))) ||
		BindFailed(pShader->Bind_RawValue("g_BlacklineParameters",
			Resource.BlacklineParameters.data(),
			sizeof(Resource.BlacklineParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_BlacklineDiffuseColor",
			&Resource.vBlacklineDiffuseColor,
			sizeof(Resource.vBlacklineDiffuseColor))) ||
		BindFailed(pShader->Bind_RawValue("g_BlacklineMaskColor",
			&Resource.vBlacklineMaskColor,
			sizeof(Resource.vBlacklineMaskColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackParameters",
			Resource.LocalCrackParameters.data(),
			sizeof(Resource.LocalCrackParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackOutColor",
			&Resource.vLocalCrackOutColor,
			sizeof(Resource.vLocalCrackOutColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackInColor",
			&Resource.vLocalCrackInColor,
			sizeof(Resource.vLocalCrackInColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackReflectionColor",
			&Resource.vLocalCrackReflectionColor,
			sizeof(Resource.vLocalCrackReflectionColor))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceTextureClampUMask",
			&Resource.iSourceTextureClampUMask,
			sizeof(Resource.iSourceTextureClampUMask))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceTextureClampVMask",
			&Resource.iSourceTextureClampVMask,
			sizeof(Resource.iSourceTextureClampVMask))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedUVScalePan",
			&Resource.GroupedConstants.vUVScalePan,
			sizeof(Resource.GroupedConstants.vUVScalePan))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedAlphaEmissive",
			&Resource.GroupedConstants.vAlphaEmissive,
			sizeof(Resource.GroupedConstants.vAlphaEmissive))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedNoiseDissolve",
			&Resource.GroupedConstants.vNoiseDissolve,
			sizeof(Resource.GroupedConstants.vNoiseDissolve))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedTint",
			&Resource.GroupedConstants.vTint,
			sizeof(Resource.GroupedConstants.vTint))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedMaterialFlags",
			&Resource.GroupedConstants.iFlags,
			sizeof(Resource.GroupedConstants.iFlags))) ||
		BindFailed(pShader->Bind_RawValue("g_EffectLocalTime",
			&fLocalTimeSeconds, sizeof(fLocalTimeSeconds))) ||
		BindFailed(pShader->Bind_RawValue("g_DynamicParameterSemantics",
			Resource.DynamicParameterSemantics.data(),
			sizeof(Resource.DynamicParameterSemantics)))))
	{
		return Fail_RenderOperation(
			"Material bind failed: source-profile block.",
			hFirstBindFailure);
	}
    if (((nullptr != pShaderProgram && !bGenericCarrierShader) || pShader == m_pDecalShader) &&
        (BindFailed(pShader->Bind_RawValue("g_SourceMaterialProfile",
            &Resource.iSourceMaterialProfile, sizeof(Resource.iSourceMaterialProfile))) ||
         BindFailed(pShader->Bind_RawValue("g_SourceTextureMask",
            &Resource.iSourceTextureMask, sizeof(Resource.iSourceTextureMask))) ||
         BindFailed(pShader->Bind_RawValue("g_SourceTextureClampUMask",
            &Resource.iSourceTextureClampUMask, sizeof(Resource.iSourceTextureClampUMask))) ||
         BindFailed(pShader->Bind_RawValue("g_SourceTextureClampVMask",
            &Resource.iSourceTextureClampVMask, sizeof(Resource.iSourceTextureClampVMask)))))
    {
        return Fail_RenderOperation(
            "Material bind failed: native source texture profile block.", hFirstBindFailure);
    }
	if (pShader == m_pRectShader && 0u != Resource.iRuntimeMaterialV2Enabled &&
		BindFailed(pShader->Bind_RawValue("g_SourceTextureMask",
			&Resource.iSourceTextureMask,
			sizeof(Resource.iSourceTextureMask))))
	{
		return Fail_RenderOperation(
			"Material bind failed: typed sprite rect source texture mask.",
			hFirstBindFailure);
	}
	if (bGenericCarrierShader && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::MESH)
	{
		uint32_t iRingFillEnabled = 0u;
		uint32_t iRingFillDirection = 0u;
		uint32_t iRingFillInvert = 0u;
		f32_t fRingFillProgress = 1.f;
		f32_t fRingFillFeather = 0.f;
		const EFFECT_MESH_RING_FILL_DESC& RingFill =
			Element.Detail.Mesh.RingFill;
		const bool_t bGenericManualMeshParticle =
			RingFill.bEnabled &&
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			nullptr != Find_Binding(
				Element, EFFECT_RESOURCE_SLOT::MESH_MODEL) &&
			nullptr != Resource.pModel &&
			!Element.SourceRecipe.bEnabled &&
			Material.strTemplateId ==
				EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE &&
			!Material.SourceMaterial.bEnabled &&
			!Material.Execution.bEnabled &&
			0u == Resource.iSourceMaterialProfile &&
			0u == Resource.iReconstructedMaterialEvaluatorEnabled &&
			0u == Resource.iArtistVisualV4Opcode &&
			0u == Resource.iRuntimeMaterialV2Enabled &&
			0u == Resource.iStandardColorV1Enabled;
		if (bGenericManualMeshParticle)
		{
			iRingFillEnabled = 1u;
			iRingFillDirection = static_cast<uint32_t>(RingFill.eDirection);
			iRingFillInvert = RingFill.bInvert ? 1u : 0u;
			fRingFillProgress = RingFill.fProgress;
			const EFFECT_LINEAR_LERP_DESC& LinearLerp =
				Element.Detail.LinearLerp;
			if (LinearLerp.bRingFillProgress)
			{
				/* Ring Fill is an Element timeline, not an implicit particle fade.
				   Fixed bursts are born on the first simulation step, so particle
				   age cannot reach one at the authored completion-wave boundary. */
				const f32_t fLifeT = Element.Detail.Timing.fLifeTimeSeconds > 0.f ?
					std::clamp(fLocalTimeSeconds /
						Element.Detail.Timing.fLifeTimeSeconds, 0.f, 1.f) :
					std::clamp(fNormalizedLife, 0.f, 1.f);
				fRingFillProgress +=
					(LinearLerp.fEndRingFillProgress - fRingFillProgress) *
					fLifeT;
			}
			fRingFillProgress = std::clamp(fRingFillProgress, 0.f, 1.f);
			fRingFillFeather = std::clamp(RingFill.fFeather, 0.f, 0.5f);
		}
		if (BindFailed(pShader->Bind_RawValue("g_RingFillEnabled",
				&iRingFillEnabled, sizeof(iRingFillEnabled))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillDirection",
				&iRingFillDirection, sizeof(iRingFillDirection))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillInvert",
				&iRingFillInvert, sizeof(iRingFillInvert))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillProgress",
				&fRingFillProgress, sizeof(fRingFillProgress))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillFeather",
				&fRingFillFeather, sizeof(fRingFillFeather))))
		{
			return Fail_RenderOperation(
				"Material bind failed: generic mesh Ring Fill block.",
				hFirstBindFailure);
		}
	}
	if ((bGenericCarrierShader && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE) || pShader == m_pRectShader)
	{
		uint32_t iLinearRevealEnabled = 0u;
		uint32_t iLinearRevealAxis = 1u;
		uint32_t iLinearRevealInvert = 1u;
		f32_t fLinearRevealStartSeconds = 0.f;
		f32_t fLinearRevealDurationSeconds = 0.55f;
		f32_t fLinearRevealEdgeWidth = 0.045f;
		f32_t fLinearRevealSoftness = 0.03f;
		float4_t vLinearRevealEdgeColor = { 1.f, 1.f, 1.f, 1.f };
		f32_t fLinearRevealEdgeEmissive = 7.f;
		const EFFECT_LINEAR_REVEAL_DESC& LinearReveal =
			Element.Detail.Sprite.LinearReveal;
		const bool_t bDirectHandAuthored = Element.strSourceNode.empty() ||
			Element.strSourceNode.starts_with("authored-copy:");
		const bool_t bExpectedSpriteCarrier =
			((nullptr != pShaderProgram && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE) &&
			 Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			 nullptr == Find_Binding(Element, EFFECT_RESOURCE_SLOT::MESH_MODEL) &&
			 nullptr == Resource.pModel) ||
			(pShader == m_pRectShader &&
			 Element.eKind == EFFECT_ELEMENT_KIND::SPRITE);
		const bool_t bGenericManualSprite = LinearReveal.bEnabled &&
			bExpectedSpriteCarrier && bDirectHandAuthored &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::END &&
			!Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			Material.strTemplateId ==
				EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Material.strSourceMaterialPath.empty() &&
			!Material.SourceMaterial.bEnabled &&
			!Material.Execution.bEnabled &&
			!Material.Execution.bFailClosed &&
			!Material.Execution.bAuthoringApproximate &&
			Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE &&
			0u == Resource.iSourceMaterialProfile &&
			0u == Resource.iReconstructedMaterialEvaluatorEnabled &&
			0u == Resource.iArtistVisualV4Opcode &&
			0u == Resource.iRuntimeMaterialV2Enabled &&
			0u == Resource.iStandardColorV1Enabled;
		if (bGenericManualSprite)
		{
			iLinearRevealEnabled = 1u;
			iLinearRevealAxis = static_cast<uint32_t>(LinearReveal.eAxis);
			iLinearRevealInvert = LinearReveal.bInvert ? 1u : 0u;
			fLinearRevealStartSeconds = std::clamp(
				LinearReveal.fStartSeconds, 0.f, 30.f);
			fLinearRevealDurationSeconds = std::clamp(
				LinearReveal.fDurationSeconds, 0.0001f, 30.f);
			fLinearRevealEdgeWidth = std::clamp(
				LinearReveal.fEdgeWidth, 0.f, 0.5f);
			fLinearRevealSoftness = std::clamp(
				LinearReveal.fSoftness, 0.f,
				(std::max)(0.f, 0.5f - fLinearRevealEdgeWidth));
			vLinearRevealEdgeColor =
			{
				std::clamp(LinearReveal.vEdgeColor.x, 0.f, 1.f),
				std::clamp(LinearReveal.vEdgeColor.y, 0.f, 1.f),
				std::clamp(LinearReveal.vEdgeColor.z, 0.f, 1.f),
				std::clamp(LinearReveal.vEdgeColor.w, 0.f, 1.f)
			};
			fLinearRevealEdgeEmissive = std::clamp(
				LinearReveal.fEdgeEmissive, 0.f, 100.f);
		}
		if (BindFailed(pShader->Bind_RawValue("g_LinearRevealEnabled",
				&iLinearRevealEnabled, sizeof(iLinearRevealEnabled))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealAxis",
				&iLinearRevealAxis, sizeof(iLinearRevealAxis))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealInvert",
				&iLinearRevealInvert, sizeof(iLinearRevealInvert))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealStartSeconds",
				&fLinearRevealStartSeconds,
				sizeof(fLinearRevealStartSeconds))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealDurationSeconds",
				&fLinearRevealDurationSeconds,
				sizeof(fLinearRevealDurationSeconds))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealEdgeWidth",
				&fLinearRevealEdgeWidth, sizeof(fLinearRevealEdgeWidth))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealSoftness",
				&fLinearRevealSoftness, sizeof(fLinearRevealSoftness))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealEdgeColor",
				&vLinearRevealEdgeColor, sizeof(vLinearRevealEdgeColor))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealEdgeEmissive",
				&fLinearRevealEdgeEmissive,
				sizeof(fLinearRevealEdgeEmissive))))
		{
			return Fail_RenderOperation(
				"Material bind failed: generic sprite Linear Reveal block.",
				hFirstBindFailure);
		}
	}

	const bool_t bBindFailed =
		BindFailed(pShader->Bind_RawValue("g_UVScale", &UVScale, sizeof(UVScale))) ||
		BindFailed(pShader->Bind_RawValue("g_UVOffset", &UVOffset, sizeof(UVOffset))) ||
		BindFailed(pShader->Bind_RawValue("g_ColorOffset", &Color.vColorOffset, sizeof(Color.vColorOffset))) ||
		BindFailed(pShader->Bind_RawValue("g_ColorMultiply", &ColorMultiply, sizeof(ColorMultiply))) ||
		BindFailed(pShader->Bind_RawValue("g_ColorClip", &Color.fColorClip, sizeof(Color.fColorClip))) ||
		BindFailed(pShader->Bind_RawValue("g_EmissiveIntensity", &Color.fEmissiveIntensity, sizeof(Color.fEmissiveIntensity))) ||
		(!bNativeArtistParticle && (
		BindFailed(pShader->Bind_RawValue("g_DistortionIntensity", &Color.fDistortionIntensity, sizeof(Color.fDistortionIntensity))) ||
		BindFailed(pShader->Bind_RawValue("g_DistortionOnBaseMaterial", &iDistortionOnBase, sizeof(iDistortionOnBase))) ||
		BindFailed(pShader->Bind_RawValue("g_RadialTime", &Color.fRadialTime, sizeof(Color.fRadialTime))) ||
		BindFailed(pShader->Bind_RawValue("g_RadialIntensity", &Color.fRadialIntensity, sizeof(Color.fRadialIntensity))) ||
		BindFailed(pShader->Bind_RawValue("g_DissolveAmount", &Dissolve, sizeof(Dissolve))) ||
		BindFailed(pShader->Bind_RawValue("g_HasNoise", &iHasNoise, sizeof(iHasNoise))) ||
		BindFailed(pShader->Bind_RawValue("g_HasMask", &iHasMask, sizeof(iHasMask))) ||
		BindFailed(pShader->Bind_RawValue("g_HasEmissive", &iHasEmissive, sizeof(iHasEmissive))) ||
		BindFailed(pShader->Bind_RawValue("g_HasDissolve", &iHasDissolve, sizeof(iHasDissolve))) ||
		BindFailed(pShader->Bind_RawValue("g_HasBase2", &iHasBase2, sizeof(iHasBase2))) ||
		BindFailed(pShader->Bind_RawValue("g_HasMask2", &iHasMask2, sizeof(iHasMask2))) ||
		BindFailed(pShader->Bind_RawValue("g_HasNoise2", &iHasNoise2, sizeof(iHasNoise2))) ||
		BindFailed(pShader->Bind_Texture("g_Base2Texture", iHasBase2 ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE2_TEXTURE) : m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_Mask2Texture", iHasMask2 ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::MASK2_TEXTURE) : m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_Noise2Texture", iHasNoise2 ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE2_TEXTURE) : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_BaseTexture",
			nullptr != Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) ?
				Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) :
				m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_NoiseTexture", iHasNoise ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE_TEXTURE) : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_MaskTexture", iHasMask ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::MASK_TEXTURE) : m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_EmissiveTexture", iHasEmissive ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE) : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_DissolveTexture", iHasDissolve ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE) : m_pBlackTexture)))) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture0",
			Resource.SourceTextures[0] ? Resource.SourceTextures[0] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture1",
			Resource.SourceTextures[1] ? Resource.SourceTextures[1] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture2",
			Resource.SourceTextures[2] ? Resource.SourceTextures[2] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture3",
			Resource.SourceTextures[3] ? Resource.SourceTextures[3] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture4",
			Resource.SourceTextures[4] ? Resource.SourceTextures[4] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture5",
			Resource.SourceTextures[5] ? Resource.SourceTextures[5] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture6",
			Resource.SourceTextures[6] ? Resource.SourceTextures[6] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture7",
			Resource.SourceTextures[7] ? Resource.SourceTextures[7] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture8",
			Resource.SourceTextures[8] ? Resource.SourceTextures[8] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture9",
			Resource.SourceTextures[9] ? Resource.SourceTextures[9] : m_pBlackTexture));
	if (bBindFailed)
	{
		return Fail_RenderOperation(
			"Material bind failed: common constants/textures block.",
			hFirstBindFailure);
	}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestMaterialBinding();
#endif
	return S_OK;
}
