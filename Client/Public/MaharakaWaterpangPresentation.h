#pragma once
#include "Client_Defines.h"
#include "WorldSequencePlayer.h"
#include "ValtanCinematicCameraController.h"
#include "Network/PacketMessages.h"
#include "Gameplay/MaharakaWaterpangContract.h"

namespace Client
{
class CCamera_Free;
// Level-owned presentation only. A Server reservation is the sole start authority.
class CMaharakaWaterpangPresentation final
{
public:
    static constexpr const wchar_t* WATER_GUN_PROTOTYPE_TAG = L"Prototype_Component_Model_MaharakaWaterGun";
    static HRESULT Ensure_WaterGunPrototype(ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context, uint32_t levelIndex, std::string& status);
    bool Initialize(const CWorldSequencePlayer::TARGET_SET& targets, std::shared_ptr<CCamera_Free> camera);
    void Accept(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play);
    void Update(float delta, uint32_t serverTick, bool editing);
    void Render() const;
    void Stop();
    void Suspend_ForAuthoring();
    ~CMaharakaWaterpangPresentation();
    const std::string& Get_Status() const { return m_Status; }
private:
    struct CUT { uint32_t startMs = 0; VALTAN_CINEMATIC_CAMERA_CUE cue; };
    bool Load_Camera();
    bool Start_World(float elapsedMs);
    // Match hazards: the authored attack sequence of each Server phase.
    void Update_Attacks();
    bool Show_Actor(size_t actor, const char* instanceId, float offsetMs, uint32_t occurrence);
    // The running Debug forced event first, else the match schedule (the Server samples the same way).
    bool Sample_Now(LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event) const;
    // Returns the cast to its placements after a forced event that ran without a match.
    void End_ForcedOnly();
    std::string Describe_EffectTracks(const std::string& instanceId, bool turned) const;
    void Trace_JetAxes(const LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event);
    void Trace_Waterfall(const LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event);
    CWorldSequencePlayer::TARGET_SET m_Targets;
    CWorldSequencePlayer m_World;
    std::weak_ptr<CCamera_Free> m_Camera;
    std::vector<CUT> m_Cuts;
    std::vector<std::string> m_Instances;
    uint32_t m_DurationMs = 0, m_StartTick = 0, m_LastTick = 0;
    float m_ClockMs = 0.f;
    float m_ReadinessWait = 0.f;
    float m_WorldClockMs = 0.f;
    int m_Countdown = 0;
    int m_Notice = -1; // MAHARAKA_WATERPANG_EVENT_KIND of the running match event
    std::string m_ActorInstances[2]; // instance currently posing the mokoko / cannon
    uint32_t m_ActorOccurrences[2] = { UINT32_MAX, UINT32_MAX }; // event occurrence each instance plays
    // Debug F1 forced event from the Server; m_ForcedOnly = the cast was posed for it without a match.
    bool m_Forced = false, m_ForcedOnly = false;
    LostArk::Shared::MAHARAKA_WATERPANG_EVENT_KIND m_ForcedKind = LostArk::Shared::MAHARAKA_WATERPANG_EVENT_KIND::CANNON;
    uint32_t m_ForcedStartTick = 0, m_ForcedOnlyBaseTick = 0;
    bool m_AttackPrepared = false, m_AttackDisabled = false;
    // Effect-only turn that puts the loop instance's bursts and jet on the Server
    // jet line, about the held cannon pivot. Shared so effect providers never
    // borrow this object.
    struct CANNON_TURN { float yawDegrees = 0.f; float pivotX = 0.f, pivotZ = 0.f; };
    std::shared_ptr<CANNON_TURN> m_CannonTurn = std::make_shared<CANNON_TURN>();
    float m_CannonObjectYawDegrees = 0.f; // held cannon object yaw of the loop templates
    float m_LastAxisTraceMs = -1.e9f;
    uint32_t m_WaterfallTraceOccurrence = UINT32_MAX;
    bool m_Ready = false, m_Scheduled = false, m_Started = false, m_Failed = false;
    std::string m_Status;
};
}
