#pragma once

#include "PresentationProvider.h"

#include <array>
#include <memory>

NS_BEGIN(Engine)
class CShader;
NS_END

NS_BEGIN(Client)

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
    f32_t fLocalTimeSeconds = 0.f;
};

bool Is_NativeScreenPostShaderProfile(uint32_t iProfile);

class CEffectNativeScreenPostMaterial final : public Engine::IPresentationScreenPostMaterial
{
public:
    explicit CEffectNativeScreenPostMaterial(EFFECT_NATIVE_SCREEN_POST_SNAPSHOT Snapshot);
    HRESULT Bind(const Engine::PRESENTATION_SCREEN_POST_MATERIAL_INPUT& Input) const override;

private:
    const EFFECT_NATIVE_SCREEN_POST_SNAPSHOT m_Snapshot;
};

NS_END
