#include "MaharakaWaterpangPresentation.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")
#include "Camera_Free.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "EffectFailureDiagnostic.h"
#include "GameInstance.h"
#include "UILabelFont.h"
#include "Gameplay/MaharakaWaterpangContract.h"
#include <fstream>
#include <sstream>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>

using namespace Client;
namespace
{
constexpr uint64_t CAMERA_OWNER = 0x4d48505741544552ull;
constexpr const char* AREA = "LV_OCN_EVENTIS_MHP";
constexpr const char* INTRO = "cutscene.maharaka.waterpang.source.intro15";
// Intro instances hold the match pose; the attack instances borrow the same objects.
constexpr const char* MOKOMOKO_HOLD = "world.sequence.instance.maharaka.waterpang.source.intro15.mokomoko";
constexpr const char* CANNON_HOLD = "world.sequence.instance.maharaka.waterpang.source.intro15.cannon";
/* Stable instance IDs per Server phase. Everything these sequences show or
   play - clips, effects with their anchors and offsets, telegraphs, sounds - is
   authored data, edited in MapTool Camera under the attack preview cutscenes. */
constexpr const char* MOKOMOKO_ATTACK = "world.sequence.instance.maharaka.waterpang.attack.mokomoko";
constexpr const char* CANNON_START = "world.sequence.instance.maharaka.waterpang.attack.cannon.start";
constexpr const char* CANNON_LOOP_CW = "world.sequence.instance.maharaka.waterpang.attack.cannon.loop.cw";
constexpr const char* CANNON_LOOP_CCW = "world.sequence.instance.maharaka.waterpang.attack.cannon.loop.ccw";
constexpr const char* CANNON_END = "world.sequence.instance.maharaka.waterpang.attack.cannon.end";
constexpr const char* ATTACK_INSTANCES[] = { MOKOMOKO_ATTACK, CANNON_START, CANNON_LOOP_CW, CANNON_LOOP_CCW, CANNON_END };
// Heading of a direction mod 180: both branches of a line share one axis.
float Line_Axis(DirectX::FXMVECTOR direction)
{
    const float degrees=std::fmod(DirectX::XMConvertToDegrees(std::atan2(
        DirectX::XMVectorGetX(direction),DirectX::XMVectorGetZ(direction)))+360.f,180.f);
    return degrees<0.f ? degrees+180.f : degrees;
}
const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& row, const char* key)
{
    const auto* value = row.Find(key);
    if (!value) throw std::runtime_error(std::string("Missing camera field: ") + key);
    return *value;
}
std::string Text(const DATA_JSON_VALUE& row, const char* key)
{
    const auto& value = Field(row,key);
    if (!value.Is_String() || value.Get_String().empty()) throw std::runtime_error("Invalid camera string");
    return value.Get_String();
}
double Number(const DATA_JSON_VALUE& row, const char* key, double lo, double hi)
{
    const auto& value = Field(row,key);
    if (!value.Is_Number() || !std::isfinite(value.Get_Number()) || value.Get_Number()<lo || value.Get_Number()>hi)
        throw std::runtime_error(std::string("Invalid camera number: ") + key);
    return value.Get_Number();
}
uint32_t Milliseconds(const DATA_JSON_VALUE& row, const char* key)
{
    const double value = Number(row,key,0,600000);
    if (value != std::floor(value)) throw std::runtime_error("Non-integral camera clock");
    return static_cast<uint32_t>(value);
}
const DATA_JSON_VALUE::ARRAY& Array(const DATA_JSON_VALUE& row, const char* key)
{
    const auto& value = Field(row,key);
    if (!value.Is_Array()) throw std::runtime_error("Invalid camera array");
    return value.Get_Array();
}
float3_t Vector(const DATA_JSON_VALUE& row, const char* key)
{
    const auto& a = Array(row,key);
    if (a.size()!=3) throw std::runtime_error("Invalid camera vector");
    for (const auto& v:a) if (!v.Is_Number() || !std::isfinite(v.Get_Number()) || std::abs(v.Get_Number())>100000)
        throw std::runtime_error("Invalid camera coordinate");
    return {static_cast<float>(a[0].Get_Number()),static_cast<float>(a[1].Get_Number()),static_cast<float>(a[2].Get_Number())};
}
}

bool CMaharakaWaterpangPresentation::Load_Camera()
{
    try
    {
        const auto path = CMapAssetCatalog::Get_MapDataRoot()/L"LV_OCN_EVENTIS_MHP.camerashots.json";
        if (std::filesystem::file_size(path)>2097152u) throw std::runtime_error("Camera document exceeds limit");
        std::ifstream input(path, std::ios::binary); std::ostringstream bytes; bytes<<input.rdbuf();
        DATA_JSON_VALUE root; std::string error;
        if (!input || !CDataJson::Parse(bytes.str(),root,error)) throw std::runtime_error("Camera parse failed: "+error);
        if (Text(root,"schema")!="lostark.camera-shots" || Number(root,"formatVersion",1,1)!=1 || Text(root,"areaId")!=AREA)
            throw std::runtime_error("Wrong Waterpang camera document");
        const DATA_JSON_VALUE* scene = nullptr;
        std::set<std::string> seen;
        for (const auto& row:Array(root,"cutscenes"))
        {
            const auto id = Text(row,"cutsceneId");
            if (!seen.insert(id).second) throw std::runtime_error("Duplicate cutscene");
            if (id==INTRO) scene=&row;
        }
        if (!scene) throw std::runtime_error("Waterpang intro15 is missing");
        const uint32_t duration = Milliseconds(*scene,"durationMs");
        if (!duration) throw std::runtime_error("Empty Waterpang intro");
        std::vector<std::string> instances; seen.clear();
        for (const auto& id:Array(*scene,"worldInstanceIds"))
        {
            if (!id.Is_String() || !seen.insert(id.Get_String()).second || !m_World.Get_Document().Find_Instance(id.Get_String()))
                throw std::runtime_error("Missing/duplicate Waterpang world instance");
            instances.push_back(id.Get_String());
        }
        if (instances.empty()) throw std::runtime_error("Empty Waterpang cast");
        std::map<std::string,const DATA_JSON_VALUE*> shots;
        for (const auto& row:Array(root,"shots"))
            if (!shots.emplace(Text(row,"shotId"), &row).second) throw std::runtime_error("Duplicate camera shot");
        std::vector<CUT> cuts;
        for (const auto& row:Array(*scene,"cameraCuts"))
        {
            CUT cut; cut.startMs=Milliseconds(row,"startMs");
            if (cut.startMs>=duration || (!cuts.empty() && cut.startMs<=cuts.back().startMs)) throw std::runtime_error("Camera cut order invalid");
            auto shot=shots.find(Text(row,"shotId"));
            if (shot==shots.end()) throw std::runtime_error("Camera shot reference missing");
            const auto& track=Field(*shot->second,"cameraTrack");
            auto& cue=cut.cue; cue.strCueId=shot->first; cue.iDurationMs=Milliseconds(track,"durationMs");
            if (!cue.iDurationMs) throw std::runtime_error("Empty camera track");
            const auto interpolation=Text(track,"interpolation"), easing=Text(track,"easing");
            if (interpolation=="LINEAR") cue.eInterpolation=VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
            else if (interpolation=="CATMULL_ROM") cue.eInterpolation=VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
            else throw std::runtime_error("Unknown camera interpolation");
            if (easing=="LINEAR") cue.eEasing=VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
            else if (easing=="SMOOTHSTEP") cue.eEasing=VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
            else if (easing=="HOLD") cue.eEasing=VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
            else throw std::runtime_error("Unknown camera easing");
            for (const auto& k:Array(track,"keyframes"))
            {
                VALTAN_CINEMATIC_CAMERA_KEYFRAME key; key.strSceneId=Text(k,"sceneId"); key.iTimeMs=Milliseconds(k,"timeMs");
                if (key.iTimeMs>cue.iDurationMs || (!cue.Keyframes.empty() && key.iTimeMs<=cue.Keyframes.back().iTimeMs))
                    throw std::runtime_error("Invalid camera key clock");
                key.vEye=Vector(k,"eye"); key.vLookAt=Vector(k,"lookAt"); key.fFovYDegrees=static_cast<float>(Number(k,"fovYDegrees",1,179));
                if (k.Find("up")) { key.vUp=Vector(k,"up"); key.hasUp=true; }
                cue.Keyframes.push_back(key);
            }
            if (cue.Keyframes.empty() || cue.Keyframes.size()>512) throw std::runtime_error("Camera key count invalid");
            VALTAN_CINEMATIC_CAMERA_POSE pose;
            if (!CValtanCinematicCameraController::Sample_Cue(cue,0,pose)) throw std::runtime_error("Camera sampler rejected cue");
            cuts.push_back(std::move(cut));
        }
        if (cuts.empty() || cuts.front().startMs!=0) throw std::runtime_error("Missing initial camera cut");
        m_Cuts=std::move(cuts); m_Instances=std::move(instances); m_DurationMs=duration;
        return true;
    }
    catch (const std::exception& error) { m_Status=error.what(); return false; }
}

bool CMaharakaWaterpangPresentation::Initialize(const CWorldSequencePlayer::TARGET_SET& targets, std::shared_ptr<CCamera_Free> camera)
{
    m_Targets=targets; m_Camera=camera; m_Targets.objectPreparationOwner=&m_World;
    /* The loop sequences' effects (the bursts and the projectile jet tracks) sit
       on RotY(snapshot basis) * RotY(track yaw) * RotY(held object yaw). Turn only
       those effects (never the model: its head already spins in the clip) about
       the held pivot by the Server's turned angle, so they stay on the Server jet
       line. Their placement itself is the authored track data. */
    const auto previous=m_Targets.objectEffectPostTransform;
    m_Targets.objectEffectPostTransform=[previous,turn=m_CannonTurn](const std::string& instanceId,
        f32_t sourceMs, float4x4_t& out, std::string& status)
    {
        DirectX::XMStoreFloat4x4(&out,DirectX::XMMatrixIdentity());
        if (previous && !previous(instanceId,sourceMs,out,status)) return false;
        if (instanceId!=CANNON_LOOP_CW && instanceId!=CANNON_LOOP_CCW) return true;
        const DirectX::XMMATRIX spin=
            DirectX::XMMatrixTranslation(-turn->pivotX,0.f,-turn->pivotZ)*
            DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(turn->yawDegrees))*
            DirectX::XMMatrixTranslation(turn->pivotX,0.f,turn->pivotZ);
        DirectX::XMStoreFloat4x4(&out,DirectX::XMLoadFloat4x4(&out)*spin);
        return true;
    };
    if (!m_World.Load_PreparedArea(AREA,m_Targets)) { m_Status=m_World.Get_Status(); return false; }
    if (!Load_Camera()) return false;
    for (const auto& id:m_Instances)
        if (!m_World.Prepare_InstanceResources(id,m_Targets)) { m_Status=m_World.Get_Status(); return false; }
    // Attack presentation is optional: a missing instance keeps the intro, the
    // notice and the Server hazards working.
    const auto casts=[this](const char* id) { return std::find(m_Instances.begin(),m_Instances.end(),id)!=m_Instances.end(); };
    m_AttackDisabled=!casts(MOKOMOKO_HOLD) || !casts(CANNON_HOLD);
    for (const char* id:ATTACK_INSTANCES)
        if (!m_World.Get_Document().Find_Instance(id)) m_AttackDisabled=true;
    // Both loop templates must hold one pure-yaw cannon pose; the effect turn needs it.
    bool loopPoseRead=false;
    for (const char* id:{CANNON_LOOP_CW,CANNON_LOOP_CCW})
    {
        const auto* instance=m_World.Get_Document().Find_Instance(id);
        const auto* sequence=instance ? m_World.Get_Document().Find_Template(instance->templateId) : nullptr;
        if (!sequence || sequence->tracks.empty() || sequence->tracks.front().keys.empty()) { m_AttackDisabled=true; continue; }
        const auto& key=sequence->tracks.front().keys.front();
        const auto& q=key.rotationQuaternion;
        const float yaw=DirectX::XMConvertToDegrees(2.f*std::atan2(q.y,q.w));
        if (std::abs(q.x)>1.e-4f || std::abs(q.z)>1.e-4f ||
            (loopPoseRead && (std::abs(yaw-m_CannonObjectYawDegrees)>1.e-3f ||
             key.positionOffset.x!=m_CannonTurn->pivotX || key.positionOffset.z!=m_CannonTurn->pivotZ)))
        { m_AttackDisabled=true; continue; }
        m_CannonObjectYawDegrees=yaw; m_CannonTurn->pivotX=key.positionOffset.x; m_CannonTurn->pivotZ=key.positionOffset.z;
        loopPoseRead=true;
    }
    if (m_AttackDisabled) OutputDebugStringA("[Waterpang] attack presentation instances are missing\n");
    m_Ready=true; return true;
}

bool CMaharakaWaterpangPresentation::Show_Actor(size_t actor, const char* instanceId, float offsetMs, uint32_t occurrence)
{
    std::string& current=m_ActorInstances[actor];
    if (current==instanceId && m_ActorOccurrences[actor]==occurrence) return true;
    if (!current.empty()) m_World.Stop_Instance(current,m_Targets,false);
    current=instanceId; m_ActorOccurrences[actor]=occurrence;
    if (m_World.Play(instanceId,m_Targets) && m_World.Seek_InstanceToMs(instanceId,offsetMs,m_Targets,true)) return true;
    m_Status=m_World.Get_Status();
    OutputDebugStringA(("[Waterpang] "+m_Status+"\n").c_str());
    return false;
}

std::string CMaharakaWaterpangPresentation::Describe_EffectTracks(const std::string& instanceId, bool turned) const
{
    /* Each authored effect track of the instance: its line axis (mod 180) from the
       track yaw, the held object yaw when inherited, the effect turn when applied
       and the document's snapshot basis, i.e. the composition the World player
       and Effect_Playback use. Head followers ride the clip bone instead. */
    const auto* instance=m_World.Get_Document().Find_Instance(instanceId);
    const auto* sequence=instance ? m_World.Get_Document().Find_Template(instance->templateId) : nullptr;
    if (!sequence) return "none";
    // The held object's authored yaw, which tracks inheriting its rotation take.
    float objectYaw=0.f;
    if (!sequence->tracks.empty() && !sequence->tracks.front().keys.empty())
    {
        const auto& q=sequence->tracks.front().keys.front().rotationQuaternion;
        objectYaw=DirectX::XMConvertToDegrees(2.f*std::atan2(q.y,q.w));
    }
    std::string text;
    for (const auto& track:sequence->effectTracks)
    {
        const auto document=CEffectCatalog::Find_Loaded(track.resourceId);
        bool root=false, head=false;
        float basis=0.f;
        if (document)
            for (const auto& element:document->Elements)
            {
                const auto& attachment=element.ActionCueAttachment;
                if (attachment.bEnabled && attachment.bFollow) head=true;
                else if (!root) { root=true; basis=attachment.bEnabled ? attachment.fSnapshotRootSourceBasisYawDegrees : 0.f; }
            }
        char row[160];
        if (!document) std::snprintf(row,sizeof(row),"%s=unprepared ",track.effectTrackId.c_str());
        else if (!root) std::snprintf(row,sizeof(row),"%s=head ",track.effectTrackId.c_str());
        else
        {
            const float yaw=basis+track.rotationDegrees.y+(track.inheritObjectRotation ? objectYaw : 0.f)+
                (turned ? m_CannonTurn->yawDegrees : 0.f);
            std::snprintf(row,sizeof(row),"%s=%.2f%s ",track.effectTrackId.c_str(),
                Line_Axis(DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.f,0.f,1.f,0.f),
                    DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(yaw)))),head ? "+head" : "");
        }
        text+=row;
    }
    return text.empty() ? "none" : text;
}

void CMaharakaWaterpangPresentation::Trace_JetAxes(const LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event)
{
    /* Once a second while firing: the Server line and every loop effect track's
       line axis from the authored data in use, plus whether the body turns. */
    m_LastAxisTraceMs=m_ClockMs;
    const DirectX::XMVECTOR localZ=DirectX::XMVectorSet(0.f,0.f,1.f,0.f);
    const bool loop=m_ActorInstances[1]==CANNON_LOOP_CW || m_ActorInstances[1]==CANNON_LOOP_CCW;
    float model=-1.f;
    float4x4_t pivot{};
    // The effect root is the held template pose (pure yaw); the model sample only
    // shows whether the cannon body itself turns.
    if (loop && m_World.Try_GetObjectPivot(m_ActorInstances[1],pivot))
        model=Line_Axis(DirectX::XMVector3TransformNormal(localZ,DirectX::XMLoadFloat4x4(&pivot)));
    // The head bone spins by its clip: att_battle_3_02 clockwise, 3_03 counter-clockwise.
    const char* headSpin=m_ActorInstances[1]==CANNON_LOOP_CW ? "cw" : (m_ActorInstances[1]==CANNON_LOOP_CCW ? "ccw" : "none");
    const std::string tracks=loop ? Describe_EffectTracks(m_ActorInstances[1],true) : std::string("none");
    char line[512];
    std::snprintf(line,sizeof(line),
        "occurrence=%u server=%.2f tracks=[%s] modelFacing=%.2f effectTurn=%.2f serverSpin=%s headSpin=%s instance=%s",
        event.iOccurrence,Line_Axis(DirectX::XMVector3TransformNormal(localZ,DirectX::XMMatrixRotationY(
            DirectX::XMConvertToRadians(event.fCannonYawDegrees)))),tracks.c_str(),model,m_CannonTurn->yawDegrees,
        event.bClockwise ? "cw" : "ccw",headSpin,loop ? "loop" : "none");
    Write_EffectFailureDiagnostic("maharaka.waterpang.axis",line);
}

void CMaharakaWaterpangPresentation::Update_Attacks()
{
    using namespace LostArk::Shared;
    // The intro owns both actors until its HOLD pose; the first event starts later.
    if (m_AttackDisabled || m_ClockMs<static_cast<float>(m_DurationMs)) return;
    if (!m_AttackPrepared)
    {
        // Instance resources include their V1 effect tracks, which prepare asynchronously.
        m_AttackPrepared=true;
        for (const char* id:ATTACK_INSTANCES)
            if (!m_World.Prepare_InstanceResources(id,m_Targets)) { m_AttackPrepared=false; break; }
    }
    MAHARAKA_WATERPANG_EVENT_SAMPLE event{};
    const bool live=Sample_Now(event);
    const bool waterfall=live && MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL==event.eKind;
    const bool cannonEvent=live && !waterfall;
    const float tickMs=1000.f/static_cast<float>(MAHARAKA_WATERPANG_TICK_HZ);
    const float eventMs=static_cast<float>(event.iElapsedTicks)*tickMs;
    const float telegraphMs=static_cast<float>(MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS)*tickMs;
    const float fireMs=cannonEvent ? static_cast<float>(event.iDurationTicks-MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS-
        MAHARAKA_WATERPANG_CANNON_END_TICKS)*tickMs : 0.f;
    const float holdMs=static_cast<float>(m_DurationMs);
    const char* mokoko=MOKOMOKO_HOLD; float mokokoMs=holdMs;
    const char* cannon=CANNON_HOLD; float cannonMs=holdMs;
    if (m_AttackPrepared)
    {
        if (waterfall) { mokoko=MOKOMOKO_ATTACK; mokokoMs=eventMs; }
        else if (cannonEvent && eventMs<telegraphMs) { cannon=CANNON_START; cannonMs=eventMs; }
        else if (cannonEvent && eventMs<telegraphMs+fireMs)
        {
            cannon=event.bClockwise ? CANNON_LOOP_CW : CANNON_LOOP_CCW; cannonMs=eventMs-telegraphMs;
        }
        else if (cannonEvent) { cannon=CANNON_END; cannonMs=eventMs-telegraphMs-fireMs; }
    }
    const uint32_t mokokoOccurrence=mokoko==MOKOMOKO_HOLD ? UINT32_MAX : event.iOccurrence;
    const uint32_t cannonOccurrence=cannon==CANNON_HOLD ? UINT32_MAX : event.iOccurrence;
    if (!Show_Actor(0,mokoko,mokokoMs,mokokoOccurrence) || !Show_Actor(1,cannon,cannonMs,cannonOccurrence))
    {
        // Fall back to the held intro pose and stop trying this session.
        m_AttackDisabled=true;
        (void)Show_Actor(0,MOKOMOKO_HOLD,holdMs,UINT32_MAX);
        (void)Show_Actor(1,CANNON_HOLD,holdMs,UINT32_MAX);
        return;
    }
    if (cannonEvent && event.bFiring && m_ClockMs-m_LastAxisTraceMs>=1000.f)
        Trace_JetAxes(event);
    if (waterfall && event.iOccurrence!=m_WaterfallTraceOccurrence &&
        event.iElapsedTicks>=MAHARAKA_WATERPANG_WATERFALL_HIT_TICK)
        Trace_Waterfall(event);
}

void CMaharakaWaterpangPresentation::Trace_Waterfall(const LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event)
{
    /* Once per cast at the first Server wave tick: the whole-deck hit volume, the
       held mokoko object's facing (its +X carries the water columns) and the source
       wave origin 3.2 m ahead of it, every authored effect track (the whole-deck
       telegraph and the ground splash stations) and the authored sound rows. */
    using namespace LostArk::Shared;
    m_WaterfallTraceOccurrence=event.iOccurrence;
    const auto heading=[](DirectX::FXMVECTOR direction)
    { return DirectX::XMConvertToDegrees(std::atan2(DirectX::XMVectorGetX(direction),DirectX::XMVectorGetZ(direction))); };
    const float sectorDegrees=DirectX::XMConvertToDegrees(std::atan2(
        MAHARAKA_WATERPANG_CANNON_X-MAHARAKA_WATERPANG_MOKOMOKO_X,MAHARAKA_WATERPANG_CANNON_Z-MAHARAKA_WATERPANG_MOKOMOKO_Z));
    float facing=0.f, originDistance=-1.f, originAngle=0.f;
    float4x4_t pivot{};
    const bool posed=m_World.Try_GetObjectPivot(MOKOMOKO_ATTACK,pivot);
    if (posed)
    {
        const DirectX::XMVECTOR forward=DirectX::XMVector3Normalize(DirectX::XMVectorSet(pivot._11,0.f,pivot._13,0.f));
        facing=heading(forward);
        // Source wave origin (4225612 AreaOffsetX 320 cm) along the object +X.
        const float x=pivot._41+DirectX::XMVectorGetX(forward)*MAHARAKA_WATERPANG_WATERFALL_ORIGIN_FORWARD_M-MAHARAKA_WATERPANG_MOKOMOKO_X;
        const float z=pivot._43+DirectX::XMVectorGetZ(forward)*MAHARAKA_WATERPANG_WATERFALL_ORIGIN_FORWARD_M-MAHARAKA_WATERPANG_MOKOMOKO_Z;
        originDistance=std::sqrt(x*x+z*z);
        originAngle=DirectX::XMConvertToDegrees(std::atan2(x,z))-sectorDegrees;
        originAngle=std::fmod(originAngle+540.f,360.f)-180.f;
    }
    std::string sounds;
    const auto* instance=m_World.Get_Document().Find_Instance(MOKOMOKO_ATTACK);
    if (const auto* sequence=instance ? m_World.Get_Document().Find_Template(instance->templateId) : nullptr)
        for (const auto& row:sequence->soundTracks)
            sounds+=row.soundTrackId+"@"+std::to_string(row.startMs)+"+"+std::to_string(row.durationMs)+"ms:"+
                row.assetId.substr(row.assetId.find_last_of('/')+1u)+" ";
    const std::string tracks=Describe_EffectTracks(MOKOMOKO_ATTACK,false);
    char line[768];
    std::snprintf(line,sizeof(line),
        "occurrence=%u tick=%u hit=deck(r<=%.1fm,waves@%u/%u) facingToCentre=%.2f tracks=[%s] modelFacing=%s%.2f waveOrigin=%.2fm/%+.1f sounds=[%s] instance=%s",
        event.iOccurrence,event.iElapsedTicks,MAHARAKA_WATERPANG_DECK_RADIUS_M,MAHARAKA_WATERPANG_WATERFALL_HIT_TICK,
        MAHARAKA_WATERPANG_WATERFALL_SECOND_HIT_TICK,sectorDegrees,tracks.c_str(),posed ? "" : "unposed:",facing,
        originDistance,originAngle,sounds.empty() ? "none" : sounds.c_str(),m_ActorInstances[0].c_str());
    Write_EffectFailureDiagnostic("maharaka.waterpang.waterfall",line);
}

void CMaharakaWaterpangPresentation::Accept(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play)
{
    using namespace LostArk::Shared;
    MAHARAKA_WATERPANG_EVENT_KIND forcedKind{};
    if (m_Ready && play.eOperation==WORLD_SEQUENCE_OPERATION::PLAY && play.iStartTick &&
        Find_MaharakaWaterpangDebugKind(play.strSequenceInstanceId,forcedKind))
    {
        // Debug F1 forced event: a newer press replaces the running one (new occurrence).
        m_Forced=true; m_ForcedKind=forcedKind; m_ForcedStartTick=play.iStartTick;
        if (!m_LastTick || static_cast<int32_t>(play.iServerTick-m_LastTick)>0) m_LastTick=play.iServerTick;
        return;
    }
    if (!m_Ready || play.strSequenceInstanceId!=MAHARAKA_WATERPANG_INTRO_INSTANCE || play.eOperation!=WORLD_SEQUENCE_OPERATION::PLAY || !play.iStartTick) return;
    if (m_Scheduled) return; // Same room reservation must not restart on duplicate delivery.
    // The reservation ends a Debug forced event on the Server; the intro poses the cast afresh.
    m_Forced=false;
    End_ForcedOnly();
    m_StartTick=play.iStartTick; m_LastTick=play.iServerTick;
    m_ClockMs=static_cast<float>(static_cast<int32_t>(play.iServerTick-m_StartTick))*1000.f/MAHARAKA_WATERPANG_TICK_HZ;
    m_Scheduled=true;
}

bool CMaharakaWaterpangPresentation::Start_World(float elapsedMs)
{
    // Admission can arrive before async NPC presentation is ready. Do not start
    // half a cast; wait for every exact replacement's owner before taking poses.
    for (const auto& id:m_Instances)
        for (const auto& binding:m_World.Get_Document().Find_Instance(id)->bindings)
            if (!binding.previewNpcPlacementId.empty() && (!m_Targets.previewNpc || !m_Targets.previewNpc(binding.previewNpcPlacementId))) return false;
    for (const auto& id:m_Instances)
    {
        if (!m_World.Play(id,m_Targets) || !m_World.Seek_InstanceToMs(id,elapsedMs,m_Targets,true))
        {
            m_Status=m_World.Get_Status(); m_World.Stop_All(m_Targets,true); m_Failed=true;
            OutputDebugStringA(("[Waterpang] "+m_Status+"\n").c_str()); return false;
        }
    }
    m_WorldClockMs=elapsedMs; m_Started=true;
    m_ActorInstances[0]=MOKOMOKO_HOLD; m_ActorInstances[1]=CANNON_HOLD;
    m_ActorOccurrences[0]=m_ActorOccurrences[1]=UINT32_MAX;
    return true;
}

bool CMaharakaWaterpangPresentation::Sample_Now(LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event) const
{
    using namespace LostArk::Shared;
    if (m_Forced && Sample_MaharakaWaterpangDebugEvent(m_ForcedKind,m_ForcedStartTick,m_LastTick,event)) return true;
    return m_Scheduled && Sample_MaharakaWaterpangEvent(static_cast<int32_t>(m_LastTick-m_StartTick),m_StartTick,event);
}

void CMaharakaWaterpangPresentation::End_ForcedOnly()
{
    if (!m_ForcedOnly) return;
    m_ForcedOnly=false;
    if (m_Started) m_World.Stop_All(m_Targets,true);
    m_Started=false; m_Notice=-1; m_ReadinessWait=0.f;
    m_ActorInstances[0].clear(); m_ActorInstances[1].clear();
    m_ActorOccurrences[0]=m_ActorOccurrences[1]=UINT32_MAX;
}

void CMaharakaWaterpangPresentation::Update(float delta, uint32_t serverTick, bool editing)
{
    using namespace LostArk::Shared;
    if (!m_Ready || m_Failed || (!m_Scheduled && !m_Forced)) return;
    if (static_cast<int32_t>(serverTick-m_LastTick)>0) m_LastTick=serverTick;
    // A forced event past its end hands the cast back to the schedule (or to nothing).
    MAHARAKA_WATERPANG_EVENT_SAMPLE forcedSample{};
    if (m_Forced && static_cast<int32_t>(m_LastTick-m_ForcedStartTick)>=0 &&
        !Sample_MaharakaWaterpangDebugEvent(m_ForcedKind,m_ForcedStartTick,m_LastTick,forcedSample))
        m_Forced=false;
    if (m_Scheduled)
        m_ClockMs=static_cast<float>(static_cast<int32_t>(m_LastTick-m_StartTick))*1000.f/MAHARAKA_WATERPANG_TICK_HZ;
    else
    {
        // No match: a forced event poses the held cast without countdown, intro camera or schedule.
        if (!m_Forced) { End_ForcedOnly(); return; }
        if (!m_ForcedOnly) { m_ForcedOnly=true; m_ForcedOnlyBaseTick=m_LastTick; }
        m_ClockMs=static_cast<float>(m_DurationMs)+
            static_cast<float>(static_cast<int32_t>(m_LastTick-m_ForcedOnlyBaseTick))*1000.f/MAHARAKA_WATERPANG_TICK_HZ;
    }
    // A frozen/disconnected Server cannot locally advance the match to its start.
    m_Countdown=m_ClockMs<0 ? static_cast<int>(std::ceil(-m_ClockMs/1000.f)) : 0;
    // The Server applies the same shared schedule; the notice follows its clock.
    MAHARAKA_WATERPANG_EVENT_SAMPLE event{};
    const bool live=Sample_Now(event);
    m_Notice=live ? static_cast<int>(event.eKind) : -1;
    // Before the world samples this frame: RotY(-90) * RotY(object) * RotY(turn) == RotY(Server jet yaw).
    if (live && MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL!=event.eKind)
        m_CannonTurn->yawDegrees=event.fCannonYawDegrees+90.f-m_CannonObjectYawDegrees;
    const auto camera=m_Camera.lock();
    if (editing) { Suspend_ForAuthoring(); return; }
    if (m_ClockMs<0) return;
    if (!m_Started && !Start_World(m_ClockMs))
    {
        m_ReadinessWait += delta;
        if (!m_Failed && m_ReadinessWait >= 5.f && !m_Scheduled)
        {
            // A forced event alone never disables the match presentation; drop just it.
            m_Forced=false; m_ReadinessWait=0.f;
            m_Status="Waterpang forced event dropped: NPC presentation not ready";
            OutputDebugStringA(("[Waterpang] "+m_Status+"\n").c_str());
            return;
        }
        if (!m_Failed && m_ReadinessWait >= 5.f)
        {
            m_Failed=true; m_Status="Waterpang NPC presentation readiness timed out";
            OutputDebugStringA(("[Waterpang] "+m_Status+"\n").c_str());
        }
        return;
    }
    m_World.Update((std::max)(0.f,m_ClockMs-m_WorldClockMs)*.001f,m_Targets);
    m_WorldClockMs=m_ClockMs;
    Update_Attacks();
    if (!camera) return;
    if (m_ClockMs>=m_DurationMs) { camera->End_PresentationOverride(CAMERA_OWNER); return; }
    const CUT* cut=&m_Cuts.front();
    for (const auto& row:m_Cuts) if (row.startMs<=m_ClockMs) cut=&row;
    VALTAN_CINEMATIC_CAMERA_POSE pose;
    if (CValtanCinematicCameraController::Sample_Cue(cut->cue,(m_ClockMs-cut->startMs)*.001f,pose) &&
        camera->Begin_PresentationOverride(CAMERA_OWNER,Engine::CCamera::PRESENTATION_PRIORITY::SERVER_CINEMATIC))
    {
        if (pose.hasUp) camera->Apply_PresentationPoseWithUp(CAMERA_OWNER,pose.vEye,pose.vLookAt,pose.vUp,pose.fFovYDegrees);
        else camera->Apply_PresentationPose(CAMERA_OWNER,pose.vEye,pose.vLookAt,pose.fFovYDegrees);
    }
}

void CMaharakaWaterpangPresentation::Render() const
{
    const auto viewport=Engine::CGameInstance::Get().Get_ViewportSize();
    const float scale=(std::min)(viewport.x/1280.f,viewport.y/720.f);
    // EFTable_GameMsg se_announce_52 / 53 / 51, in MAHARAKA_WATERPANG_EVENT_KIND order.
    static const wchar_t* const NOTICES[]={
        L"\uBAA8\uCF54\uBAA8\uCF54 \uC6CC\uD130\uCE90\uB17C\uC774 \uD68C\uC804\uD569\uB2C8\uB2E4.",
        L"\uBAA8\uCF54\uBAA8\uCF54 \uC6CC\uD130\uCE90\uB17C\uC774 \uBE60\uB974\uAC8C \uD68C\uC804\uD569\uB2C8\uB2E4.",
        L"\uBAA8\uCF54\uBAA8\uCF54 \uC5B4\uD2B8\uB809\uC158\uC5D0\uC11C \uBB3C\uBCBC\uB77D\uC774 \uC3DF\uC544\uC9D1\uB2C8\uB2E4."};
    if (m_Notice>=0 && m_Notice<3)
        UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"),NOTICES[m_Notice],viewport.x*.5f,viewport.y*.18f,22.f*scale,DirectX::Colors::White);
    if (!m_Countdown || m_Failed) return;
    const std::wstring text=L"\uC6CC\uD130\uD321 \uC544\uB808\uB098\uAC00 "+std::to_wstring(m_Countdown)+L"\uCD08 \uB4A4\uC5D0 \uC2DC\uC791\uD569\uB2C8\uB2E4";
    UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"),text.c_str(),viewport.x*.5f,viewport.y*.25f,26.f*scale,DirectX::Colors::Yellow);
}

void CMaharakaWaterpangPresentation::Stop()
{
    if (const auto camera=m_Camera.lock()) camera->End_PresentationOverride(CAMERA_OWNER);
    m_World.Stop_All(m_Targets,true); m_Scheduled=false; m_Started=false; m_Countdown=0; m_Notice=-1;
    m_ActorInstances[0].clear(); m_ActorInstances[1].clear();
    m_ActorOccurrences[0]=m_ActorOccurrences[1]=UINT32_MAX;
    m_Forced=m_ForcedOnly=false;
}
void CMaharakaWaterpangPresentation::Suspend_ForAuthoring()
{
    if (const auto camera=m_Camera.lock()) camera->End_PresentationOverride(CAMERA_OWNER);
    if (m_Started) m_World.Stop_All(m_Targets,true);
    m_ActorInstances[0].clear(); m_ActorInstances[1].clear();
    m_ActorOccurrences[0]=m_ActorOccurrences[1]=UINT32_MAX;
    m_Started=false; m_Countdown=0; m_ReadinessWait=0;
    // Keep the Server reservation. Closing the tool seeks to the current room
    // time (including HOLD after the intro), rather than replaying the event.
}
CMaharakaWaterpangPresentation::~CMaharakaWaterpangPresentation() { Stop(); }
