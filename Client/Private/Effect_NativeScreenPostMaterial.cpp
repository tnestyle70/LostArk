#include "Effect_NativeScreenPostMaterial.h"

#include "Shader.h"

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

bool Client::Is_NativeScreenPostShaderProfile(const uint32_t iProfile)
{
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
    if (!State.pShader || !Input.pSceneColor || !Input.pSceneDepth ||
        !Is_NativeScreenPostShaderProfile(State.iProfile) ||
        !IsFinite(State.vSourceColor) || !IsFinite(State.vDynamicParameter) ||
        !std::isfinite(State.fProjectionW) || !std::isfinite(State.fLocalTimeSeconds) ||
        State.fLocalTimeSeconds < 0.f || (State.iTextureMask & ~0x1ffu) != 0u)
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
    BindTexture("g_EffectSceneDepthTexture", Input.pSceneDepth);
    static constexpr std::array<const char*, 9u> TextureNames = {{
        "g_SourceTexture0", "g_SourceTexture1", "g_SourceTexture2",
        "g_SourceTexture3", "g_SourceTexture4", "g_SourceTexture5",
        "g_SourceTexture6", "g_SourceTexture7", "g_SourceTexture8"
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
        State.iProfile == 1619u || State.iProfile == 1623u || State.iProfile == 1648u))
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
