#pragma once

#include "EffectRecoveryCamera.h"
#include "ClassSelectionTimeline.h"
#include "ClassMovieInspection.h"
#include "WorldSequencePlayer.h"
#include "Effect_PresentationService.h"

#include <memory>
#include <optional>
#include <map>
#include <set>
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
        float brightness = 1.f, radiusMeters = 1.f;
        bool enabled = true;
    };
    struct LIGHT_TRACK final
    {
        std::string lightId;
        float radiusMeters = 1.f, falloffExponent = 1.f;
        std::vector<LIGHT_KEY> keys;
    };
    struct EFFECT_ROOT_KEY final
    {
        double timeMs = 0.;
        float3_t position{}, scale{1.f, 1.f, 1.f};
        float4_t rotationQuaternion{0.f, 0.f, 0.f, 1.f};
    };
    struct EFFECT_PARAMETER_KEY final
    {
        double timeMs = 0.;
        std::array<float, 3> value{}, arriveTangent{}, leaveTangent{};
        EFFECT_DISTRIBUTION_INTERPOLATION interpolation = EFFECT_DISTRIBUTION_INTERPOLATION::LINEAR;
    };
    struct EFFECT_PARAMETER_TRACK final
    {
        std::string parameterName;
        uint32_t componentCount = 1u;
        std::vector<EFFECT_PARAMETER_KEY> keys;
    };
    struct EFFECT_TRACK final
    {
        std::string effectId, assetId;
        float4x4_t rootWorld{};
        double startMs = 0., endMs = 0.;
        double sourceLoopEndMs = 0., loopAgeDeltaMs = 0.;
        std::vector<CLOCK_KEY> clockKeys;
        std::vector<EFFECT_ROOT_KEY> rootKeys;
        std::vector<EFFECT_PARAMETER_TRACK> parameterTracks;
        double SourceTimeMs(double timeMs) const;
        double PhaseTimeMs(double ageMs) const;
        float4x4_t RootWorldAt(double timeMs) const;
        const EFFECT_TRACK& HistoryTrackAt(double ageMs, const EFFECT_TRACK* intro,
            uint64_t loopCycle, double& phaseMs) const;
        float4x4_t HistoryRootWorld(double ageMs, const EFFECT_TRACK* intro, uint64_t loopCycle) const;
        std::vector<EFFECT_PARAMETER_INPUT> ParametersAt(double phaseMs) const;
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
        double MovieTimeMs(double sourceMs) const;
    };
    struct SCENE final
    {
        std::string classId, sceneId, backgroundAreaId;
        std::vector<std::string> excludedWorldObjectIds;
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
    void Set_Paused(bool paused);
    // Camera inspection never stops the Movie clock, actors or Effect handles.
    bool Set_InspectionFreeCamera(bool free, std::string& status);
    bool Is_InspectionFreeCamera() const { return m_InspectionFreeCamera; }
    CLASS_MOVIE_INSPECTION_STATE Get_WorldInspection(const std::string& classId, bool loop) const;
    bool Inspect_World(const std::string& classId, bool loop,
        const CLASS_MOVIE_INSPECTION_COMMAND& command, std::string& status);
    bool Pick_WorldInspection(const float3_t& origin, const float3_t& direction, std::string& status);
    void Update_InspectionPicking(bool productPointerHovered);
    bool Is_InspectionPickArmed() const { return m_InspectionPickArmed; }
    bool Is_InspectionBackgroundVisible() const;
    bool Seek(const std::string& classId, bool loop, double wallMs);
    bool Set_PlaybackRate(double rate);
    double Get_PlaybackRate() const { return m_PlaybackRate; }
    double Get_SourceClockMs() const;
    double Get_SourceRate() const;
    const CLASS_MOVIE_CAMERA_SAMPLE& Get_CameraSample() const { return m_CameraSample; }
    std::shared_ptr<const CLASS_MOVIE_TIMELINE> Get_Timeline(const std::string& classId, bool loop) const;
    double Map_TimelineTime(const std::string& classId, bool loop, double timeMs, bool toSource) const;
    // Instance-local V1 editor draft; retained across Play, Stop, Seek and phase transitions.
    bool Preview_EffectDocument(const EFFECT_DOCUMENT_DESC& document, std::string& status,
        const std::vector<std::string>* drawElementIds = nullptr);
    bool Clear_EffectPreviews(std::string& status);
    bool Play_EffectSelection(const std::string& classId, bool loop,
        const EFFECT_DOCUMENT_DESC& full, const EFFECT_DOCUMENT_DESC& selected,
        const std::vector<std::string>& drawElementIds, double startAgeMs, double endAgeMs, bool repeat, std::string& status);
    bool Preview_EffectSelection(const EFFECT_DOCUMENT_DESC& full,
        const EFFECT_DOCUMENT_DESC& selected, const std::vector<std::string>& drawElementIds, double startAgeMs, double endAgeMs, std::string& status);
    bool Is_SelectionActive() const { return m_Selection.has_value(); }
    bool Is_SelectionRepeating() const { return m_Selection && m_Selection->bounded && m_Selection->repeat; }

    bool Begin_Authoring(std::string& status);
    bool Get_AuthoringBox(const std::string& classId, bool loop, const std::string& kind,
        const std::string& boxId, CLASS_MOVIE_AUTHORING_BOX& out, std::string& status);
    bool Apply_AuthoringBox(const CLASS_MOVIE_AUTHORING_BOX& before,
        const DATA_JSON_VALUE& replacement, std::string& status);
    bool Save_Authoring(std::string& status, bool publish = true);
    bool Publish_Authoring(std::string& status);
    bool Edit_AuthoringTiming(const CLASS_MOVIE_AUTHORING_BOX& before,
        double sourceStartMs, double sourceEndMs, CLASS_MOVIE_TIMING_EDIT gesture, std::string& status);
    bool Reload_Authoring(std::string& status);
    bool Has_AuthoringChanges() const { return m_Authoring && m_Authoring->dirty; }
    uint64_t Get_AuthoringGeneration() const { return m_Authoring ? m_Authoring->generation : 0u; }
    bool Is_AuthoringPublishPending() const { return m_Authoring && m_Authoring->publishProcess; }
    const std::string& Get_AuthoringStatus() const { return m_Authoring ? m_Authoring->status : m_Status; }
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
    const std::string& Get_BackgroundAreaId(const std::string& classId) const;

    // Parser is shared with non-UI contract checks; failed parsing preserves out.
    static bool Parse(const DATA_JSON_VALUE& root, const std::string& areaId,
        std::vector<SCENE>& out, std::string& error);
    static bool Load_EffectTargets(const std::string& areaId,
        std::vector<std::string>& out, std::string& error);
    // Missing per-scene background uses the registry's legacy presentation Area.
    // Validate the whole manifest before publishing the unique Area list.
    static bool Load_BackgroundAreas(const std::string& areaId, const std::string& fallbackAreaId,
        std::vector<std::string>& outAreaIds, std::string& outStatus);
    static bool Is_Configured();

private:
    static bool Validate_WorldExclusions(const std::vector<SCENE>& scenes,
        const CWorldSequenceDocument& document, std::string& status);
    bool Set_WorldExcluded(const std::string& classId, const std::string& objectId,
        bool excluded, std::string& status);
    void Apply_WorldInspection();
    bool Start_Phase(const SCENE& scene, bool loop, double elapsedMs, uint64_t loopCycle = 0u,
        bool desiredPaused = false, bool rebuildEffects = false);
    bool Sample_Frame();
    bool Sample_Camera(const PHASE& phase, float sampleMs);
    bool Sample_MaterialsAndLights(const PHASE& phase, float sampleMs);
    bool Sample_Effects(const PHASE& phase, float sampleMs);
    void Stop_Effects();
    bool Prepare_SelectionTargets(const EFFECT_DOCUMENT_DESC& full, const EFFECT_DOCUMENT_DESC& selected,
        const std::vector<std::string>& drawElementIds,
        std::shared_ptr<const EFFECT_WORLD_PREVIEW_TARGET>& fullTarget,
        std::shared_ptr<const EFFECT_WORLD_PREVIEW_TARGET>& selectedTarget, std::string& status);

    void Fail(const std::string& reason);

    struct AUTHORING_STATE final
    {
        DATA_JSON_VALUE manifest, world, manifestBase, worldBase;
        uint64_t generation = 1u;
        bool dirty = false, needsPublish = false, appliedToPreview = false;
        std::vector<SCENE> scenes;
        CWorldSequenceDocument document;
        HANDLE publishProcess = nullptr;
        std::filesystem::path publishLog;
        std::string status;
        ~AUTHORING_STATE() { if (publishProcess) CloseHandle(publishProcess); }
    };
    bool Validate_Authoring(const DATA_JSON_VALUE& manifest, const DATA_JSON_VALUE& world,
        std::vector<SCENE>& scenes, CWorldSequenceDocument& document, std::string& status) const;
    bool Prepare_Authoring(const std::vector<SCENE>& scenes, const CWorldSequenceDocument& document,
        std::string& status);
    bool Commit_Authoring(std::vector<SCENE> scenes, const CWorldSequenceDocument& document,
        std::string& status);
    void Start_AuthoringPublish();
    void Poll_AuthoringPublish();
    std::unique_ptr<AUTHORING_STATE> m_Authoring;
    CWorldSequencePlayer m_Resources;
    std::unique_ptr<CWorldSequencePlayer> m_Active;
    CWorldSequencePlayer::TARGET_SET m_Targets;
    std::weak_ptr<CCamera_Free> m_Camera;
    std::shared_ptr<CClassSelectionLightFrame> m_LightFrame;
    std::vector<SCENE> m_Scenes;
    const SCENE* m_Scene = nullptr;
    double m_ElapsedMs = 0.;
    double m_PlaybackRate = 1.;
    CLASS_MOVIE_CAMERA_SAMPLE m_CameraSample;
    mutable std::map<std::pair<std::string, bool>, std::shared_ptr<const CLASS_MOVIE_TIMELINE>> m_Timelines;
    mutable std::map<std::string, std::map<std::string, double>> m_AnimationDurations;
    uint64_t m_LoopCycle = 0u;
    uint64_t m_PlaybackToken = 0u;
    struct ACTIVE_EFFECT final
    {
        std::string assetId;
        EFFECT_WORLD_ROOT_HANDLE handle;
        bool sampledInLoop = false;
        uint64_t loopCycle = 0u;
    };
    std::map<std::string, ACTIVE_EFFECT> m_Effects;
    std::map<std::string, std::shared_ptr<const EFFECT_WORLD_PREVIEW_TARGET>> m_EffectPreviews;
    struct EFFECT_SELECTION final
    {
        std::string classId, assetId, occurrenceId;
        bool loop = false, repeat = false, bounded = true;
        double startMs = 0., endMs = 0.;
        std::shared_ptr<const EFFECT_WORLD_PREVIEW_TARGET> target;
    };
    std::optional<EFFECT_SELECTION> m_Selection;
    bool m_Looping = false;
    bool m_Paused = false;
    bool m_DeferAdvance = false;
    bool m_OwnsCamera = false;
    bool m_InspectionFreeCamera = false;
    std::string m_InspectionClass, m_InspectionSelectedObject, m_InspectionSoloObject, m_InspectionStatus;
    std::set<std::string> m_InspectionMutedObjects;
    uint32_t m_InspectionPickedMesh = UINT32_MAX;
    bool m_InspectionShowBackground = true, m_InspectionShowEffects = true;
    bool m_InspectionPickArmed = false, m_InspectionPickReleased = false;
    std::string m_Status = "Class selection cinematics are not loaded.";
};
}
