#include "SkeletalAfterimage.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include <algorithm>
#include <cmath>

namespace
{
    // Project-authored presentation values. The source TrailGhost notify is
    // known; its native material/fade ABI has not been recovered.
    constexpr uint32_t AFTERIMAGE_PASS = 14u;

    bool Finite(const float4x4_t& value)
    {
        for (const auto& row : value.m)
            for (const float component : row)
                if (!std::isfinite(component)) return false;
        return true;
    }

    float DistanceSquared(const float4x4_t& a, const float4x4_t& b)
    {
        const float x = a._41 - b._41, y = a._42 - b._42, z = a._43 - b._43;
        return x * x + y * y + z * z;
    }
}

bool Client::CSkeletalAfterimage::Configure(const SETTINGS& settings)
{
    if (!std::isfinite(settings.sampleIntervalSeconds) || settings.sampleIntervalSeconds < .005f ||
        settings.sampleIntervalSeconds > .5f || !std::isfinite(settings.sampleLifetimeSeconds) ||
        settings.sampleLifetimeSeconds < settings.sampleIntervalSeconds || settings.sampleLifetimeSeconds > 2.f ||
        !settings.maxSamples || settings.maxSamples > 16u ||
        !std::isfinite(settings.color.x) || !std::isfinite(settings.color.y) ||
        !std::isfinite(settings.color.z) || !std::isfinite(settings.color.w) ||
        settings.color.x < 0.f || settings.color.y < 0.f || settings.color.z < 0.f ||
        settings.color.w < 0.f || settings.color.w > 1.f) return false;
    if (settings.sampleIntervalSeconds != m_Settings.sampleIntervalSeconds ||
        settings.sampleLifetimeSeconds != m_Settings.sampleLifetimeSeconds || settings.maxSamples != m_Settings.maxSamples ||
        settings.color.x != m_Settings.color.x || settings.color.y != m_Settings.color.y ||
        settings.color.z != m_Settings.color.z || settings.color.w != m_Settings.color.w ||
        settings.capturePoseChanges != m_Settings.capturePoseChanges)
    {
        Reset();
        m_Settings = settings;
    }
    return true;
}

void Client::CSkeletalAfterimage::Reset()
{
    m_Samples.clear();
    m_Model.reset();
    m_Accumulator = 0.f;
    m_SuppressedUntilDisabled = false;
}

void Client::CSkeletalAfterimage::Suppress_FailedPresentation()
{
    m_Samples.clear();
    m_Accumulator = 0.f;
    if (!m_SuppressedUntilDisabled)
        OutputDebugStringA("[Client][Afterimage] Pose history isolated; live actor presentation is preserved.\n");
    m_SuppressedUntilDisabled = true;
}

void Client::CSkeletalAfterimage::Update(const float deltaSeconds, const bool emitting,
    const std::shared_ptr<Engine::CModel>& model, const float4x4_t& world)
{
    if (!model || !model->Is_Skinned() || !Finite(world) ||
        !std::isfinite(deltaSeconds) || deltaSeconds < 0.f)
    {
        Reset();
        return;
    }
    if (m_Model.lock() != model)
    {
        Reset();
        m_Model = model;
    }
    // A paused owner cannot age or manufacture samples. A discontinuity cannot
    // leave silhouettes spanning a teleport or a suspended process.
    if (deltaSeconds == 0.f) return;
    if (deltaSeconds > m_Settings.sampleLifetimeSeconds || (!m_Samples.empty() &&
        DistanceSquared(m_Samples.back().world, world) > 100.f))
    {
        m_Samples.clear();
        m_Accumulator = 0.f;
    }
    for (auto& sample : m_Samples) sample.ageSeconds += deltaSeconds;
    while (!m_Samples.empty() && m_Samples.front().ageSeconds >= m_Settings.sampleLifetimeSeconds)
        m_Samples.pop_front();
    if (!emitting)
    {
        m_Accumulator = 0.f;
        m_SuppressedUntilDisabled = false;
        return;
    }
    if (m_SuppressedUntilDisabled) return;
    m_Accumulator += deltaSeconds;
    if (m_Accumulator < m_Settings.sampleIntervalSeconds) return;
    m_Accumulator = std::fmod(m_Accumulator, m_Settings.sampleIntervalSeconds);
    // Save one actual pose per presented frame, never duplicate it to fabricate
    // missed sub-frame motion. Stationary actors do not accumulate a bright stack.
    const bool stationary = !m_Samples.empty() && DistanceSquared(m_Samples.back().world, world) < .0016f;
    if (stationary && !m_Settings.capturePoseChanges) return;
    SAMPLE staged;
    staged.world = world;
    staged.palettes.resize(model->Get_NumMeshes());
    for (uint32_t mesh = 0u; mesh < model->Get_NumMeshes(); ++mesh)
    {
        if (!model->Capture_BoneMatrices(mesh, staged.palettes[mesh]))
        {
            Suppress_FailedPresentation();
            return;
        }
    }
    if (staged.palettes.empty()) return;
    if (stationary && m_Settings.capturePoseChanges)
    {
        bool changed = staged.palettes.size() != m_Samples.back().palettes.size();
        for (size_t mesh = 0u; !changed && mesh < staged.palettes.size(); ++mesh)
        {
            const auto& before = m_Samples.back().palettes[mesh];
            const auto& after = staged.palettes[mesh];
            changed = before.size() != after.size();
            for (size_t bone = 0u; !changed && bone < after.size(); ++bone)
                for (size_t row = 0u; !changed && row < 4u; ++row)
                    for (size_t column = 0u; column < 4u; ++column)
                        if (std::abs(before[bone].m[row][column] - after[bone].m[row][column]) > 1e-5f)
                        { changed = true; break; }
        }
        if (!changed) return;
    }
    m_Samples.push_back(std::move(staged));
    while (m_Samples.size() > m_Settings.maxSamples) m_Samples.pop_front();
}

HRESULT Client::CSkeletalAfterimage::Render(const std::shared_ptr<Engine::CModel>& model,
    const std::shared_ptr<Engine::CShader>& shader, const float4x4_t& liveWorld)
{
    if (m_Samples.empty()) return S_FALSE;
    if (!model || !shader || m_Model.lock() != model || !Finite(liveWorld))
    {
        Reset();
        return S_FALSE;
    }
    auto& game = Engine::CGameInstance::Get();
    HRESULT result = game.Bind_Transform(shader, "g_ViewMatrix", Engine::D3DTS::VIEW);
    if (SUCCEEDED(result)) result = game.Bind_Transform(shader, "g_ProjMatrix", Engine::D3DTS::PROJ);
    const auto* camera = game.Get_CamPosition();
    if (SUCCEEDED(result)) result = camera ?
        shader->Bind_RawValue("g_vCamPosition", camera, sizeof(float4_t)) : E_FAIL;
    for (const auto& sample : m_Samples)
    {
        if (FAILED(result)) break;
        if (sample.palettes.size() != model->Get_NumMeshes()) { result = E_FAIL; break; }
        const float life = std::clamp(1.f - sample.ageSeconds / m_Settings.sampleLifetimeSeconds, 0.f, 1.f);
        const float4_t color{m_Settings.color.x, m_Settings.color.y, m_Settings.color.z,
            m_Settings.color.w * life * life};
        result = shader->Bind_Matrix("g_WorldMatrix", &sample.world);
        if (SUCCEEDED(result)) result = shader->Bind_RawValue("g_ChargeAfterimageColor", &color, sizeof(color));
        for (uint32_t mesh = 0u; SUCCEEDED(result) && mesh < model->Get_NumMeshes(); ++mesh)
        {
            const auto& palette = sample.palettes[mesh];
            result = shader->Bind_Matrices("g_BoneMatrices", palette.data(), static_cast<uint32_t>(palette.size()));
            if (SUCCEEDED(result)) result = shader->Begin(AFTERIMAGE_PASS);
            if (SUCCEEDED(result)) result = model->Render(mesh);
        }
    }
    // Bind history only to the GPU. The actor's pose/cache/cursor never changed.
    // Restore this shader's live draw inputs even if a history draw failed.
    const HRESULT worldResult = shader->Bind_Matrix("g_WorldMatrix", &liveWorld);
    const HRESULT poseResult = model->Get_NumMeshes() > 0u ?
        model->Bind_BoneMatrices(shader, "g_BoneMatrices", model->Get_NumMeshes() - 1u) : E_FAIL;
    if (FAILED(result) || FAILED(worldResult) || FAILED(poseResult))
    {
        Suppress_FailedPresentation();
        return FAILED(result) ? result : E_FAIL;
    }
    return S_OK;
}
