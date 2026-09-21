#pragma once

#include "PresentationProvider.h"

#include <array>
#include <memory>

NS_BEGIN(Engine)
class CShader;
NS_END

NS_BEGIN(Client)

// The occurrence owns one frozen HDR/bloom pair. Frame packets share it across seeks.
struct EFFECT_SCENE_CAPTURE_STATE final
{
    ComPtr<ID3D11Device> pDevice;
    ComPtr<ID3D11DeviceContext> pContext;
    ComPtr<ID3D11ShaderResourceView> pColor;
    ComPtr<ID3D11ShaderResourceView> pBloom;
    float2_t vDestinationUV = {.5f, .5f};
    HRESULT hLastResult = S_FALSE;
    f32_t fStartSeconds = 0.f;
    f32_t fDurationSeconds = 0.f;
    HRESULT Capture_Once(const Engine::PRESENTATION_SCREEN_POST_MATERIAL_INPUT& Input,
        const float2_t& destinationUV);
};

// One queued frame owns its source inputs even if the authoring document changes.
struct EFFECT_NATIVE_SCREEN_POST_SNAPSHOT final
{
    std::shared_ptr<Engine::CShader> pShader;
    std::array<ComPtr<ID3D11ShaderResourceView>, 10u> SourceTextures;
    std::array<float4_t, 32u> Parameters{};
    uint32_t iProfile = 0u;
    uint32_t iTextureMask = 0u;
    uint32_t iClampUMask = 0u;
    uint32_t iClampVMask = 0u;
    float4_t vSourceColor = { 1.f, 1.f, 1.f, 1.f };
    float4_t vDynamicParameter = { 1.f, 1.f, 1.f, 1.f };
    f32_t fProjectionW = 0.f;
    f32_t fProjectionZ = 0.f;
    std::array<float4_t, 3u> SourceLocalToWorld{};
    std::array<float4_t, 3u> SourceWorldToView{};
    f32_t fLocalTimeSeconds = 0.f;
    f32_t fBloomIntensity = 1.3f;
    // Authored screen transition; it does not impersonate a native material ID.
    bool bSceneCollapse = false;
    f32_t fCaptureProgress = 0.f;
    float2_t vCaptureDestinationUV = {.5f, .5f};
    bool bCaptureAllowed = true;
    bool bCaptureOverLiveScene = false;
    float2_t vCaptureDestinationSizeUV = {0.f, 0.f};
    float2_t vCaptureDestinationOffsetUV = {0.f, 0.f};
    float4_t vCaptureEdgeSpeed = {1.f, 1.f, 1.f, 1.f};
    f32_t fCaptureRotationDegrees = 0.f;
    f32_t fCaptureBackgroundDim = 0.f;
    bool bCaptureSquare = false;
    std::shared_ptr<EFFECT_SCENE_CAPTURE_STATE> pCapture;
};

bool Is_NativeScreenPostShaderProfile(uint32_t iProfile);

class CEffectNativeScreenPostMaterial final : public Engine::IPresentationScreenPostMaterial
{
public:
    explicit CEffectNativeScreenPostMaterial(EFFECT_NATIVE_SCREEN_POST_SNAPSHOT Snapshot);
    HRESULT Bind(const Engine::PRESENTATION_SCREEN_POST_MATERIAL_INPUT& Input) const override;
    // Capture and compose after scene distortion has resolved, before product UI.

private:
    const EFFECT_NATIVE_SCREEN_POST_SNAPSHOT m_Snapshot;
};

NS_END
