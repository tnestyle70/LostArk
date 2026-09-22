#pragma once

#include "EffectRecoveryCamera.h"
#include "WorldSequencePlayer.h"
#include "Effect_PresentationService.h"

#include <memory>
#include <map>
#include <string>
#include <vector>

namespace Client
{
class CCamera_Free;
class CClassSelectionLightFrame;

// Owns the class-selection presentation clock, not the replicated player.
// Models use WorldSequence; particles use the ordinary world-root Effect service.
class CClassSelectionPresentation final
{
public:
    struct CLOCK_KEY final
    {
        double timeMs = 0., sourceMs = 0.;
    };
    struct MATERIAL_KEY final
    {
        uint32_t timeMs = 0u;
        std::array<float, 4> value{};
    };
    struct MATERIAL_CURVE final
    {
        std::string parameter;
        std::vector<MATERIAL_KEY> keys;
    };
    struct MATERIAL_TRACK final
    {
        std::string instanceId, slotId, materialName, family;
        std::map<std::string, std::array<float, 4>> parameters;
        std::vector<MATERIAL_CURVE> curves;
    };
    struct LIGHT_KEY final
    {
        uint32_t timeMs = 0u;
        float3_t position{}, color{1.f, 1.f, 1.f};
        float brightness = 1.f;
        bool enabled = true;
    };
    struct LIGHT_TRACK final
    {
        std::string lightId;
        float radiusMeters = 1.f, falloffExponent = 1.f;
        std::vector<LIGHT_KEY> keys;
    };
    struct EFFECT_TRACK final
    {
        std::string effectId, assetId;
        float4x4_t rootWorld{};
        double startMs = 0., endMs = 0.;
        double sourceLoopEndMs = 0.;
        std::vector<CLOCK_KEY> clockKeys;
        double SourceTimeMs(double timeMs) const;
    };
    struct PHASE final
    {
        uint32_t durationMs = 0u;
        std::vector<std::string> instanceIds;
        std::vector<EFFECT_CAMERA_ROW> cameras;
        std::vector<CLOCK_KEY> clockKeys;
        std::vector<MATERIAL_TRACK> materialTracks;
        std::vector<LIGHT_TRACK> lights;
        std::vector<EFFECT_TRACK> effects;
        double WallDurationMs() const;
        double SourceTimeMs(double wallMs) const;
    };
    struct SCENE final
    {
        std::string classId, sceneId;
        PHASE intro, loop;
    };

    CClassSelectionPresentation() = default;
    ~CClassSelectionPresentation();
    CClassSelectionPresentation(const CClassSelectionPresentation&) = delete;
    CClassSelectionPresentation& operator=(const CClassSelectionPresentation&) = delete;

    // Optional presentation failure must not reject Server-approved arena entry.
    bool Initialize(const std::string& areaId, const CWorldSequencePlayer::TARGET_SET& targets,
        const std::shared_ptr<CCamera_Free>& camera);
    bool Play(const std::string& classId);
    void Stop();
    void Clear();
    void Update(float deltaSeconds);
    void Set_Paused(bool paused) { m_Paused = paused; }
    bool Seek(const std::string& classId, bool loop, double wallMs);
    bool Is_Paused() const { return m_Paused; }
    uint64_t Get_LoopCycle() const { return m_LoopCycle; }
    uint64_t Get_PlaybackToken() const { return m_PlaybackToken; }
    double Get_ClockMs() const { return m_ElapsedMs; }
    double Get_DurationMs() const;
    double Get_PhaseDurationMs(const std::string& classId, bool loop) const;
    bool Is_Active() const { return m_Active != nullptr; }
    bool Is_Looping() const { return Is_Active() && m_Looping; }
    bool Has_Class(const std::string& classId) const;
    const std::string& Get_Status() const { return m_Status; }
    const std::string& Get_ActiveClass() const;

    // Parser is shared with non-UI contract checks; failed parsing preserves out.
    static bool Parse(const DATA_JSON_VALUE& root, const std::string& areaId,
        std::vector<SCENE>& out, std::string& error);
    static bool Load_EffectTargets(const std::string& areaId,
        std::vector<std::string>& out, std::string& error);
    static bool Is_Configured();

private:
    bool Start_Phase(const SCENE& scene, bool loop, double elapsedMs, uint64_t loopCycle = 0u);
    bool Sample_Frame();
    bool Sample_MaterialsAndLights(const PHASE& phase, float sampleMs);
    bool Sample_Effects(const PHASE& phase, float sampleMs);
    void Stop_Effects();
    void Fail(const std::string& reason);

    CWorldSequencePlayer m_Resources;
    std::unique_ptr<CWorldSequencePlayer> m_Active;
    CWorldSequencePlayer::TARGET_SET m_Targets;
    std::weak_ptr<CCamera_Free> m_Camera;
    std::shared_ptr<CClassSelectionLightFrame> m_LightFrame;
    std::vector<SCENE> m_Scenes;
    const SCENE* m_Scene = nullptr;
    double m_ElapsedMs = 0.;
    uint64_t m_LoopCycle = 0u;
    uint64_t m_PlaybackToken = 0u;
    struct ACTIVE_EFFECT final
    {
        std::string assetId;
        EFFECT_WORLD_ROOT_HANDLE handle;
    };
    std::map<std::string, ACTIVE_EFFECT> m_Effects;
    bool m_Looping = false;
    bool m_Paused = false;
    bool m_DeferAdvance = false;
    bool m_OwnsCamera = false;
    std::string m_Status = "Class selection cinematics are not loaded.";
};
}
