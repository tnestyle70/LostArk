#include "Client_Defines.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")
#include "Effect_NativeScreenPostMaterial.h"

#include "Shader.h"
#include "Effect_ArtistMaterial.h"
#include "GameInstance.h"

#include <cmath>
#include <utility>

namespace
{
    bool IsFinite(const float4_t& Value)
    {
        return std::isfinite(Value.x) && std::isfinite(Value.y) &&
            std::isfinite(Value.z) && std::isfinite(Value.w);
    }
}

HRESULT Client::EFFECT_SCENE_CAPTURE_STATE::Capture_Once(
    const Engine::PRESENTATION_SCREEN_POST_MATERIAL_INPUT& Input, const float2_t& destinationUV)
{
    if (pColor && pBloom) return hLastResult = S_OK;
    if (!pDevice || !pContext || !Input.pSceneColor || !Input.pSceneBloom) return hLastResult = E_INVALIDARG;
    const auto copy = [&](const ComPtr<ID3D11ShaderResourceView>& source,
        ComPtr<ID3D11ShaderResourceView>& target) -> HRESULT
    {
        ComPtr<ID3D11Resource> resource;
        ComPtr<ID3D11Texture2D> texture, frozen;
        source->GetResource(&resource);
        if (!resource || FAILED(resource.As(&texture))) return E_INVALIDARG;
        D3D11_TEXTURE2D_DESC desc{}; texture->GetDesc(&desc);
        if (!desc.Width || !desc.Height || desc.SampleDesc.Count != 1u || desc.ArraySize != 1u)
            return E_INVALIDARG;
        desc.Usage = D3D11_USAGE_DEFAULT; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0u; desc.MiscFlags = 0u;
        HRESULT result = pDevice->CreateTexture2D(&desc, nullptr, &frozen);
        if (SUCCEEDED(result)) result = pDevice->CreateShaderResourceView(frozen.Get(), nullptr, &target);
        if (FAILED(result)) return result;
        // Input is the resolved ping-pong source, never the bound destination.
        pContext->CopyResource(frozen.Get(), texture.Get());
        return S_OK;
    };
    ComPtr<ID3D11ShaderResourceView> color, bloom;
    HRESULT result = copy(Input.pSceneColor, color);
    if (SUCCEEDED(result)) result = copy(Input.pSceneBloom, bloom);
    if (FAILED(result)) return hLastResult = result;
    pColor = std::move(color); pBloom = std::move(bloom);
    vDestinationUV = destinationUV;
    return hLastResult = S_OK;
}

bool Client::Is_NativeScreenPostShaderProfile(const uint32_t iProfile)
{
    if (const auto* Program = Find_ArtistNativeCarrierProgram(iProfile))
        return Program->strRendererShape == "screenPost";
    return iProfile == 68u || iProfile == 76u || iProfile == 155u ||
        iProfile == 156u || iProfile == 415u || iProfile == 672u || iProfile == 1084u ||
        iProfile == 876u || iProfile == 894u || iProfile == 1619u ||
        iProfile == 1623u || iProfile == 1648u || iProfile == 659u ||
        iProfile == 1228u || iProfile == 1255u;
}

Client::CEffectNativeScreenPostMaterial::CEffectNativeScreenPostMaterial(
    EFFECT_NATIVE_SCREEN_POST_SNAPSHOT Snapshot)
    : m_Snapshot(std::move(Snapshot))
{
}

HRESULT Client::CEffectNativeScreenPostMaterial::Bind(
    const Engine::PRESENTATION_SCREEN_POST_MATERIAL_INPUT& Input) const
{
    const auto& State = m_Snapshot;
    if (State.bSceneCollapse)
    {
        if (!State.pShader || !State.pCapture ||
            !std::isfinite(State.fCaptureProgress) || State.fCaptureProgress < 0.f || State.fCaptureProgress > 1.f ||
            !std::isfinite(State.vCaptureDestinationUV.x) || !std::isfinite(State.vCaptureDestinationUV.y) ||
            !std::isfinite(State.vCaptureDestinationOffsetUV.x) || !std::isfinite(State.vCaptureDestinationOffsetUV.y) ||
            std::abs(State.vCaptureDestinationOffsetUV.x) > 1.f || std::abs(State.vCaptureDestinationOffsetUV.y) > 1.f ||
            !std::isfinite(State.vCaptureDestinationSizeUV.x) || !std::isfinite(State.vCaptureDestinationSizeUV.y) ||
            State.vCaptureDestinationSizeUV.x < 0.f || State.vCaptureDestinationSizeUV.y < 0.f ||
            !IsFinite(State.vCaptureEdgeSpeed) ||
            State.vCaptureEdgeSpeed.x < .05f || State.vCaptureEdgeSpeed.x > 20.f ||
            State.vCaptureEdgeSpeed.y < .05f || State.vCaptureEdgeSpeed.y > 20.f ||
            State.vCaptureEdgeSpeed.z < .05f || State.vCaptureEdgeSpeed.z > 20.f ||
            State.vCaptureEdgeSpeed.w < .05f || State.vCaptureEdgeSpeed.w > 20.f ||
            !std::isfinite(State.fCaptureRotationDegrees) || std::abs(State.fCaptureRotationDegrees) > 180.f ||
            !std::isfinite(State.fCaptureBackgroundDim) || State.fCaptureBackgroundDim < 0.f || State.fCaptureBackgroundDim > 1.f)
            return E_INVALIDARG;
        const bool ready = State.pCapture->pColor && State.pCapture->pBloom;
        const bool priming = !ready && !State.bCaptureAllowed;
        HRESULT result = priming ? S_OK : State.pCapture->Capture_Once(Input, State.vCaptureDestinationUV);
        if (FAILED(result)) return result;
        auto& shader = *State.pShader;
        const int live = State.bCaptureOverLiveScene ? 1 : 0;
        const int square = State.bCaptureSquare ? 1 : 0;
        const float progress = priming ? 0.f : State.fCaptureProgress;
        auto destination = State.bCaptureOverLiveScene ?
            State.vCaptureDestinationUV : State.pCapture->vDestinationUV;
        // Freeze the capture anchor, not the current authored tuning. Apply/Save
        // preserves this occurrence's image while a changed offset takes effect now.
        destination.x += State.vCaptureDestinationOffsetUV.x;
        destination.y += State.vCaptureDestinationOffsetUV.y;
        result = shader.Bind_Matrix("g_WorldMatrix", &Input.World);
        if (SUCCEEDED(result)) result = shader.Bind_Matrix("g_ViewMatrix", &Input.View);
        if (SUCCEEDED(result)) result = shader.Bind_Matrix("g_ProjMatrix", &Input.Projection);
        if (SUCCEEDED(result)) result = shader.Bind_Texture("g_CapturedSceneColor", priming ? Input.pSceneColor : State.pCapture->pColor);
        if (SUCCEEDED(result)) result = shader.Bind_Texture("g_CapturedSceneBloom", priming ? Input.pSceneBloom : State.pCapture->pBloom);
        if (SUCCEEDED(result)) result = shader.Bind_Texture("g_EffectSceneColorTexture", Input.pSceneColor);
        if (SUCCEEDED(result)) result = shader.Bind_Texture("g_EffectSceneBloomTexture", Input.pSceneBloom);
        if (SUCCEEDED(result)) result = shader.Bind_RawValue("g_CaptureProgress", &progress, sizeof(float));
        if (SUCCEEDED(result)) result = shader.Bind_RawValue("g_CaptureDestinationUV", &destination, sizeof(float2_t));
        if (SUCCEEDED(result)) result = shader.Bind_RawValue("g_CaptureDestinationSizeUV", &State.vCaptureDestinationSizeUV, sizeof(float2_t));
        if (SUCCEEDED(result)) result = shader.Bind_RawValue("g_CaptureOverLiveScene", &live, sizeof(live));
        if (SUCCEEDED(result)) result = shader.Bind_RawValue("g_CaptureEdgeSpeed", &State.vCaptureEdgeSpeed, sizeof(float4_t));
        if (SUCCEEDED(result)) result = shader.Bind_RawValue("g_CaptureRotationDegrees", &State.fCaptureRotationDegrees, sizeof(float));
        if (SUCCEEDED(result)) result = shader.Bind_RawValue("g_CaptureBackgroundDim", &State.fCaptureBackgroundDim, sizeof(float));
        if (SUCCEEDED(result)) result = shader.Bind_RawValue("g_CaptureSquare", &square, sizeof(square));
        result = FAILED(result) ? result : shader.Begin(1u);
        State.pCapture->hLastResult = SUCCEEDED(result) && priming ? S_FALSE : result;
        return result;
    }
    if (!State.pShader || !Input.pSceneColor || !Input.pSceneBloom || !Input.pSceneDepth ||
        !Is_NativeScreenPostShaderProfile(State.iProfile) ||
        !IsFinite(State.vSourceColor) || !IsFinite(State.vDynamicParameter) ||
        !std::isfinite(State.fProjectionW) || !std::isfinite(State.fLocalTimeSeconds) ||
        State.fLocalTimeSeconds < 0.f || !std::isfinite(State.fBloomIntensity) ||
        State.fBloomIntensity < 0.f || State.fBloomIntensity > 16.f || (State.iTextureMask & ~0x3ffu) != 0u)
        return E_INVALIDARG;
    for (const auto& Value : State.Parameters)
        if (!IsFinite(Value)) return E_INVALIDARG;
    for (size_t Lane = 0u; Lane < State.SourceTextures.size(); ++Lane)
        if ((State.iTextureMask & (1u << Lane)) != 0u && !State.SourceTextures[Lane])
            return E_INVALIDARG;

    Engine::CShader& Shader = *State.pShader;
    HRESULT Result = S_OK;
    const auto BindRaw = [&](const char* Name, const auto& Value)
    {
        if (SUCCEEDED(Result))
            Result = Shader.Bind_RawValue(Name, &Value, static_cast<uint32_t>(sizeof(Value)));
    };
    const auto BindMatrix = [&](const char* Name, const float4x4_t& Value)
    {
        if (SUCCEEDED(Result)) Result = Shader.Bind_Matrix(Name, &Value);
    };
    const auto BindTexture = [&](const char* Name, const ComPtr<ID3D11ShaderResourceView>& Value)
    {
        if (SUCCEEDED(Result)) Result = Shader.Bind_Texture(Name, Value);
    };
    BindMatrix("g_WorldMatrix", Input.World);
    BindMatrix("g_ViewMatrix", Input.View);
    BindMatrix("g_ProjMatrix", Input.Projection);
    BindTexture("g_EffectSceneColorTexture", Input.pSceneColor);
    BindTexture("g_EffectSceneBloomTexture", Input.pSceneBloom);
    const auto Quality = Engine::CGameInstance::Get().Get_RenderQualitySettings();
    BindRaw("g_fEffectBloomIntensity", State.fBloomIntensity);
    BindRaw("g_fEffectBloomThreshold", Quality.fBloomThreshold);
    BindRaw("g_fEffectBloomSoftKnee", Quality.fBloomSoftKnee);
    BindTexture("g_EffectSceneDepthTexture", Input.pSceneDepth);
    static constexpr std::array<const char*, 10u> TextureNames = {{
        "g_SourceTexture0", "g_SourceTexture1", "g_SourceTexture2",
        "g_SourceTexture3", "g_SourceTexture4", "g_SourceTexture5",
        "g_SourceTexture6", "g_SourceTexture7", "g_SourceTexture8", "g_SourceTexture9"
    }};
    for (size_t Lane = 0u; Lane < TextureNames.size(); ++Lane)
        BindTexture(TextureNames[Lane], State.SourceTextures[Lane]);
    BindRaw("g_SourceMaterialProfile", State.iProfile);
    BindRaw("g_SourceTextureMask", State.iTextureMask);
    BindRaw("g_SourceTextureClampUMask", State.iClampUMask);
    BindRaw("g_SourceTextureClampVMask", State.iClampVMask);
    BindRaw("g_PostSourceColor", State.vSourceColor);
    BindRaw("g_PostSourceDynamicParameter", State.vDynamicParameter);
    BindRaw("g_PostSourceProjectionW", State.fProjectionW);
    BindRaw("g_PostSourceProjectionZ", State.fProjectionZ);
    if (State.iProfile == 4006u || State.iProfile == 4060u)
    {
        if (!std::isfinite(State.fProjectionZ) || std::abs(State.fProjectionZ) < 1.e-6f) return S_FALSE;
        for (const auto& Row : State.SourceLocalToWorld) if (!IsFinite(Row)) return E_INVALIDARG;
        for (const auto& Row : State.SourceWorldToView) if (!IsFinite(Row)) return E_INVALIDARG;
        const auto& Forward = State.SourceLocalToWorld[0];
        const auto& View0 = State.SourceWorldToView[0];
        const auto& View1 = State.SourceWorldToView[1];
        const auto& View2 = State.SourceWorldToView[2];
        const float ScreenX = -(Forward.x * View0.x + Forward.y * View1.x + Forward.z * View2.x);
        const float ScreenY = -(Forward.x * View0.y + Forward.y * View1.y + Forward.z * View2.y);
        // The source atan polynomial divides by max(abs(x), abs(y)).
        // A forward vector exactly along the camera has no screen direction.
        if (!std::isfinite(ScreenX) || !std::isfinite(ScreenY)) return E_INVALIDARG;
        if (std::max(std::abs(ScreenX), std::abs(ScreenY)) < 1.e-6f) return S_FALSE;
        BindRaw("g_ArtistSourceLocalToWorld", State.SourceLocalToWorld);
        BindRaw("g_ArtistSourceWorldToView", State.SourceWorldToView);
    }
    const float Emissive = 1.f;
    const float ColorClip = 0.f;
    BindRaw("g_EmissiveIntensity", Emissive);
    BindRaw("g_ColorClip", ColorClip);
    if (State.iProfile == 68u || State.iProfile == 76u)
    {
        BindRaw("g_VSourceMaterialParameters", State.Parameters);
        BindRaw("g_VSourceMaterialTime", State.fLocalTimeSeconds);
    }
    else if (State.iProfile == 155u || State.iProfile == 156u)
    {
        BindRaw("g_ALTVSourceMaterialParameters", State.Parameters);
        BindRaw("g_ALTVSourceMaterialTime", State.fLocalTimeSeconds);
    }
    else if (State.iProfile == 415u || State.iProfile == 672u || State.iProfile == 1084u)
    {
        BindRaw("g_WarlordSourceMaterialParameters", State.Parameters);
        BindRaw("g_WarlordSourceMaterialTime", State.fLocalTimeSeconds);
    }
    else if ((State.iProfile == 876u || State.iProfile == 894u ||
        State.iProfile == 1619u || State.iProfile == 1623u || State.iProfile == 1648u ||
        (nullptr != Find_ArtistNativeCarrierProgram(State.iProfile))))
    {
        BindRaw("g_ArtistSourceMaterialParameters", State.Parameters);
        BindRaw("g_ArtistSourceMaterialTime", State.fLocalTimeSeconds);
    }
    else if (State.iProfile == 659u || State.iProfile == 1228u || State.iProfile == 1255u)
    {
        BindRaw("g_LanceVASourceMaterialParameters", State.Parameters);
        BindRaw("g_LanceVASourceMaterialTime", State.fLocalTimeSeconds);
    }
    if (FAILED(Result)) return Result;
    // Engine retains the ping-pong targets and the one quad draw.
    return Shader.Begin(0u);
}
