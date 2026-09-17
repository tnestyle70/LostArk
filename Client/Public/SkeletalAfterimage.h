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
    struct SETTINGS final
    {
        float sampleIntervalSeconds = .05f;
        float sampleLifetimeSeconds = .25f;
        uint32_t maxSamples = 6u;
        float4_t color{1.8f, 1.8f, 1.8f, .38f};
        bool capturePoseChanges = false;
    };
    bool Configure(const SETTINGS& settings);
    void Update(float deltaSeconds, bool emitting,
        const std::shared_ptr<Engine::CModel>& model, const float4x4_t& world);
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
    };
    SETTINGS m_Settings;
    std::deque<SAMPLE> m_Samples;
    std::weak_ptr<Engine::CModel> m_Model;
    float m_Accumulator = 0.f;
    bool m_SuppressedUntilDisabled = false;
    void Suppress_FailedPresentation();
};
}
