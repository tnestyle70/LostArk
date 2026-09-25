#include "SkeletalAfterimage.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include <algorithm>
#include <cmath>

namespace
{
    // The existing carrier serves authored cues and source TrailGhost parameters.
    // Native material/fade code is unavailable; source-channel rendering is an adapter.
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
        settings.sampleIntervalSeconds > 2.f || !std::isfinite(settings.sampleLifetimeSeconds) ||
        settings.sampleLifetimeSeconds < .005f || settings.sampleLifetimeSeconds > 2.f ||
        !settings.maxSamples || settings.maxSamples > 64u ||
        !std::isfinite(settings.color.x) || !std::isfinite(settings.color.y) ||
        !std::isfinite(settings.color.z) || !std::isfinite(settings.color.w) ||
        settings.color.x < 0.f || settings.color.y < 0.f || settings.color.z < 0.f ||
        settings.color.w < 0.f || settings.color.w > 1.f) return false;
    if (!std::isfinite(settings.sourceColorIntensity) || settings.sourceColorIntensity < 0.f || settings.sourceColorIntensity > 100.f) return false;
    for (const float value : {settings.endColor.x, settings.endColor.y, settings.endColor.z, settings.endColor.w})
        if (!std::isfinite(value) || value < 0.f) return false;
    if (!std::isfinite(settings.sourceInitialAlpha) || settings.sourceInitialAlpha < 0.f || settings.sourceInitialAlpha > 1.f ||
        !std::isfinite(settings.userAlphaScale) || settings.userAlphaScale < 0.f ||
        settings.sourceInitialAlpha * settings.userAlphaScale > 1.f ||
        !std::isfinite(settings.sourceAlphaHoldSeconds) || settings.sourceAlphaHoldSeconds < 0.f ||
        settings.sourceAlphaHoldSeconds >= settings.sampleLifetimeSeconds) return false;
    for (const auto& color : {settings.ambientStart, settings.ambientEnd})
        for (const float value : {color.x, color.y, color.z, color.w})
            if (!std::isfinite(value) || value < 0.f || value > 1.f) return false;
    const auto sameColor = [](const float4_t& a, const float4_t& b) {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    };
    const bool changed = settings.sampleIntervalSeconds != m_Settings.sampleIntervalSeconds ||
        settings.sampleLifetimeSeconds != m_Settings.sampleLifetimeSeconds || settings.maxSamples != m_Settings.maxSamples ||
        !sameColor(settings.color, m_Settings.color) || !sameColor(settings.endColor, m_Settings.endColor) ||
        settings.capturePoseChanges != m_Settings.capturePoseChanges || settings.interpolateColor != m_Settings.interpolateColor ||
        settings.sourceColorIntensity != m_Settings.sourceColorIntensity || settings.sourceChannels != m_Settings.sourceChannels ||
        settings.sourceInitialAlpha != m_Settings.sourceInitialAlpha || settings.userAlphaScale != m_Settings.userAlphaScale ||
        settings.sourceAlphaHoldSeconds != m_Settings.sourceAlphaHoldSeconds ||
        !sameColor(settings.ambientStart, m_Settings.ambientStart) || !sameColor(settings.ambientEnd, m_Settings.ambientEnd);
    if (changed)
    {
        // A new source notify must not recolor or prematurely expire its siblings.
        // Legacy style/counter transitions keep their existing isolation behavior.
        if (!settings.sourceChannels || !m_Settings.sourceChannels) Reset();
        else { m_Accumulator = 0.f; m_WasEmitting = false; }
        m_Settings = settings;
    }
    return true;
}

void Client::CSkeletalAfterimage::Reset()
{
    m_Samples.clear();
    m_Model.reset();
    m_Accumulator = 0.f;
    m_PulseClockSeconds = m_PulseBirthSeconds = -1.f;
    m_SuppressedUntilDisabled = false;
    m_WasEmitting = false;
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
    if (!model || (!model->Is_Skinned() && !m_View.socketed) || !Finite(world) ||
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
    float longestLifetime = m_Settings.sampleLifetimeSeconds;
    for (const auto& sample : m_Samples) longestLifetime = (std::max)(longestLifetime, sample.settings.sampleLifetimeSeconds);
    if (deltaSeconds > longestLifetime || (!m_Samples.empty() &&
        DistanceSquared(m_Samples.back().world, world) > 100.f))
    {
        m_Samples.clear();
        m_Accumulator = 0.f;
        m_WasEmitting = false;
    }
    for (auto& sample : m_Samples) sample.ageSeconds += deltaSeconds;
    std::erase_if(m_Samples, [](const SAMPLE& sample) {
        return sample.ageSeconds >= sample.settings.sampleLifetimeSeconds;
    });
    if (!emitting)
    {
        m_Accumulator = 0.f;
        m_SuppressedUntilDisabled = false;
        m_WasEmitting = false;
        return;
    }
    if (m_SuppressedUntilDisabled) return;
    const bool initialSourcePose = m_Settings.sourceChannels && !m_WasEmitting;
    m_WasEmitting = true;
    m_Accumulator += deltaSeconds;
    if (!initialSourcePose && m_Accumulator < m_Settings.sampleIntervalSeconds) return;
    m_Accumulator = initialSourcePose ? 0.f : std::fmod(m_Accumulator, m_Settings.sampleIntervalSeconds);
    // Save one actual pose per presented frame, never duplicate it to fabricate
    // missed sub-frame motion. Stationary actors do not accumulate a bright stack.
    const bool stationary = !initialSourcePose && !m_Samples.empty() && DistanceSquared(m_Samples.back().world, world) < .0016f;
    if (stationary && !m_Settings.capturePoseChanges) return;
    SAMPLE staged;
    if (!Capture_Sample(model, world, staged)) return;
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

bool Client::CSkeletalAfterimage::Capture_Sample(const std::shared_ptr<Engine::CModel>& model,
    const float4x4_t& world, SAMPLE& sample)
{
    sample.world = world;
    sample.settings = m_Settings;
    const bool supplied = m_View.model == model;
    sample.hiddenMeshMask = supplied ? m_View.hiddenMeshMask : 0u;
    sample.palettes.resize(model->Get_NumMeshes());
    if (supplied && m_View.socketed) return !sample.palettes.empty();
    const auto palette = supplied && m_View.paletteModel ? m_View.paletteModel : model;
    for (uint32_t mesh = 0u; mesh < model->Get_NumMeshes(); ++mesh)
        if (!palette->Capture_BoneMatrices(palette == model ? mesh : 0u, sample.palettes[mesh]))
        { Suppress_FailedPresentation(); return false; }
    return !sample.palettes.empty();
}

bool Client::CSkeletalAfterimage::Capture_Initial(const MODEL_VIEW& view, const float ageSeconds)
{
    if (!view.model || !Finite(view.world) || !std::isfinite(ageSeconds) || ageSeconds < 0.f ||
        ageSeconds >= m_Settings.sampleLifetimeSeconds) return false;
    if (m_Model.lock() != view.model) Reset();
    m_View = view; m_Model = view.model;
    SAMPLE sample;
    if (!Capture_Sample(view.model, view.world, sample)) return false;
    sample.ageSeconds = ageSeconds; m_Samples.push_back(std::move(sample));
    m_WasEmitting = true;
    while (m_Samples.size() > m_Settings.maxSamples) m_Samples.pop_front();
    return true;
}

void Client::CSkeletalAfterimage::Sample_Pulse(const float elapsedSeconds,
    const std::shared_ptr<Engine::CModel>& model, const float4x4_t& world)
{
    if (!model || !model->Is_Skinned() || !Finite(world) ||
        !std::isfinite(elapsedSeconds) || elapsedSeconds < 0.f)
    { Reset(); return; }
    if (m_Model.lock() != model || elapsedSeconds < m_PulseClockSeconds ||
        (!m_Samples.empty() && DistanceSquared(m_Samples.front().world, world) > 100.f))
    { Reset(); m_Model = model; }
    m_PulseClockSeconds = elapsedSeconds;
    if (m_SuppressedUntilDisabled) return;
    const float phase = std::fmod(elapsedSeconds, m_Settings.sampleIntervalSeconds);
    const float birth = elapsedSeconds - phase;
    if (phase >= m_Settings.sampleLifetimeSeconds)
    { m_Samples.clear(); return; }
    if (m_Samples.empty() || std::abs(birth - m_PulseBirthSeconds) > .0001f)
    {
        SAMPLE sample;
        if (!Capture_Sample(model, world, sample)) return;
        m_Samples.clear();
        m_Samples.push_back(std::move(sample));
        m_PulseBirthSeconds = birth;
    }
    // Repeated paused samples change neither the captured pose nor the pulse phase.
    m_Samples.front().ageSeconds = phase;
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
    const bool socketed = m_View.model == model && m_View.socketed;
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
        const auto& settings = sample.settings;
        const float life = std::clamp(1.f - sample.ageSeconds / settings.sampleLifetimeSeconds, 0.f, 1.f);
        const float blend = settings.sourceChannels || settings.interpolateColor ? 1.f - life : 0.f;
        // PROJECT_RECONSTRUCTED: keep the decoded initial-alpha plateau, then
        // retain this carrier's squared fade over the remaining child lifetime.
        const float heldLife = std::clamp(1.f - (std::max)(0.f, sample.ageSeconds - settings.sourceAlphaHoldSeconds) /
            (settings.sampleLifetimeSeconds - settings.sourceAlphaHoldSeconds), 0.f, 1.f);
        const float opacity = settings.sourceChannels ? settings.sourceInitialAlpha * settings.userAlphaScale * heldLife * heldLife :
            std::lerp(settings.color.w, settings.endColor.w, blend) * life * life;
        const float4_t color{
            std::lerp(settings.color.x, settings.endColor.x, blend),
            std::lerp(settings.color.y, settings.endColor.y, blend),
            std::lerp(settings.color.z, settings.endColor.z, blend), opacity};
        const float4_t ambient{
            std::lerp(settings.ambientStart.x, settings.ambientEnd.x, blend),
            std::lerp(settings.ambientStart.y, settings.ambientEnd.y, blend),
            std::lerp(settings.ambientStart.z, settings.ambientEnd.z, blend),
            std::lerp(settings.ambientStart.w, settings.ambientEnd.w, blend)};
        result = shader->Bind_Matrix("g_WorldMatrix", &sample.world);
        if (SUCCEEDED(result)) result = shader->Bind_RawValue("g_ChargeAfterimageColor", &color, sizeof(color));
        if (!socketed && SUCCEEDED(result))
        {
            const int sourceChannels = settings.sourceChannels ? 1 : 0;
            result = shader->Bind_RawValue("g_ChargeAfterimageSourceChannels", &sourceChannels, sizeof(sourceChannels));
            if (SUCCEEDED(result)) result = shader->Bind_RawValue("g_ChargeAfterimageAmbientColor", &ambient, sizeof(ambient));
        }
        for (uint32_t mesh = 0u; SUCCEEDED(result) && mesh < model->Get_NumMeshes(); ++mesh)
        {
            if (mesh < 32u && (sample.hiddenMeshMask & (1u << mesh))) continue;
            const auto& palette = sample.palettes[mesh];
            if (!socketed) result = shader->Bind_Matrices("g_BoneMatrices", palette.data(), static_cast<uint32_t>(palette.size()));
            if (socketed)
            {
                auto normal = XMLoadFloat4x4(&sample.world); normal.r[3] = XMVectorSet(0,0,0,1);
                float4x4_t inverse; XMStoreFloat4x4(&inverse, XMMatrixTranspose(XMMatrixInverse(nullptr, normal)));
                result = shader->Bind_Matrix("g_WorldInvTransposeMatrix", &inverse);
            }
            if (SUCCEEDED(result)) result = shader->Bind_RawValue("g_ChargeAfterimageSourceIntensity", &settings.sourceColorIntensity, sizeof(float));
            if (SUCCEEDED(result) && settings.sourceColorIntensity > 0.f)
                result = model->Bind_Material(shader, "g_DiffuseTexture", mesh, aiTextureType_DIFFUSE, 0);
            if (SUCCEEDED(result)) result = shader->Begin(socketed ? 23u : AFTERIMAGE_PASS);
            if (SUCCEEDED(result)) result = model->Render(mesh);
        }
    }
    // Bind history only to the GPU. The actor's pose/cache/cursor never changed.
    // Restore this shader's live draw inputs even if a history draw failed.
    const HRESULT worldResult = shader->Bind_Matrix("g_WorldMatrix", &liveWorld);
    const auto livePalette = m_View.model == model && m_View.paletteModel ? m_View.paletteModel : model;
    HRESULT poseResult = !socketed && livePalette->Get_NumMeshes() > 0u ?
        livePalette->Bind_BoneMatrices(shader, "g_BoneMatrices", livePalette == model ? model->Get_NumMeshes() - 1u : 0u) : S_OK;
    if (socketed)
    {
        auto normal = XMLoadFloat4x4(&liveWorld); normal.r[3] = XMVectorSet(0,0,0,1);
        float4x4_t inverse; XMStoreFloat4x4(&inverse, XMMatrixTranspose(XMMatrixInverse(nullptr, normal)));
        poseResult = shader->Bind_Matrix("g_WorldInvTransposeMatrix", &inverse);
    }
    if (FAILED(result) || FAILED(worldResult) || FAILED(poseResult))
    {
        Suppress_FailedPresentation();
        return FAILED(result) ? result : E_FAIL;
    }
    return S_OK;
}
