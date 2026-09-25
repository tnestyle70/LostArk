#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include <deque>
#include <memory>
#include <vector>

namespace Engine { class CModel; class CShader; }

namespace Client
{
// A bounded visual history of an already sampled actor. No animation playback,
// model clone, movement authority or collision is owned by this component.
class CSkeletalAfterimage final
{
public:
    struct MODEL_VIEW final
    {
        std::shared_ptr<Engine::CModel> model, paletteModel;
        std::shared_ptr<Engine::CShader> shader;
        float4x4_t world{};
        uint32_t hiddenMeshMask = 0u;
        bool socketed = false;
    };
    struct SETTINGS final
    {
        float sampleIntervalSeconds = .05f;
        float sampleLifetimeSeconds = .25f;
        uint32_t maxSamples = 6u;
        float4_t color{1.8f, 1.8f, 1.8f, .38f};
        bool capturePoseChanges = false;
        bool interpolateColor = false;
        float sourceColorIntensity = 0.f;
        float4_t endColor{1.f, 1.f, 1.f, 1.f};
        // Opt-in source TrailGhost channels. color/endColor hold the rim channel;
        // legacy callers retain their authored shader, opacity and reset policy.
        bool sourceChannels = false;
        float sourceInitialAlpha = 1.f;
        float userAlphaScale = 1.f;
        float sourceAlphaHoldSeconds = 0.f;
        float4_t ambientStart{0.f, 0.f, 0.f, 0.f};
        float4_t ambientEnd{0.f, 0.f, 0.f, 0.f};
    };
    bool Configure(const SETTINGS& settings);
    void Set_PresentationView(const MODEL_VIEW& view) { m_View = view; }
    bool Capture_Initial(const MODEL_VIEW& view, float ageSeconds);
    void Update(float deltaSeconds, bool emitting,
        const std::shared_ptr<Engine::CModel>& model, const float4x4_t& world);
    // Presentation-clock pulse: one full pose per period, then a fully hidden gap.
    void Sample_Pulse(float elapsedSeconds, const std::shared_ptr<Engine::CModel>& model,
        const float4x4_t& world);
    void Reset();
    bool Has_Samples() const { return !m_Samples.empty(); }
    HRESULT Render(const std::shared_ptr<Engine::CModel>& model,
        const std::shared_ptr<Engine::CShader>& shader, const float4x4_t& liveWorld);

private:
    struct SAMPLE final
    {
        float4x4_t world{};
        std::vector<std::vector<float4x4_t>> palettes;
        float ageSeconds = 0.f;
        uint32_t hiddenMeshMask = 0u;
        SETTINGS settings; // An emitted child retains its own lifetime and colors.
    };
    MODEL_VIEW m_View;
    SETTINGS m_Settings;
    std::deque<SAMPLE> m_Samples;
    std::weak_ptr<Engine::CModel> m_Model;
    float m_Accumulator = 0.f;
    bool m_SuppressedUntilDisabled = false;
    bool m_WasEmitting = false;
    float m_PulseClockSeconds = -1.f;
    float m_PulseBirthSeconds = -1.f;
    bool Capture_Sample(const std::shared_ptr<Engine::CModel>& model,
        const float4x4_t& world, SAMPLE& sample);
    void Suppress_FailedPresentation();
};
}
