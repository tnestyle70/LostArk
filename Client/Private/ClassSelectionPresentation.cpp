#include "ClassSelectionPresentation.h"

#include "Camera_Free.h"
#include "ProjectDataRoot.h"
#include "WorldSequenceObject.h"
#include "GameInstance.h"
#include "Effect_LightPresentation.h"
#include "Presentation_Manager.h"
#include "SourceCharacterMaterialParameters.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <set>
#include <tuple>

namespace Client
{
class CClassSelectionLightFrame final : public Engine::IPresentationProvider
{
public:
    std::vector<Engine::LIGHT_DESC> lights;
    HRESULT Submit_Presentation() override
    {
        auto& presentation = Engine::CPresentation_Manager::Get();
        presentation.Register_ProviderSubmissionExpectation(lights.size(), lights.size(), 0u, 0u);
        for (const auto& light : lights)
            if (FAILED(presentation.Add_TransientLight(light))) return E_FAIL;
        return S_OK;
    }
};
namespace
{
constexpr uint64_t CAMERA_OWNER = 0x434C41535353454Cull;
constexpr uint32_t MAX_MS = CWorldSequenceDocument::MAX_DURATION_MS;
// Presentation commands run on the main thread. A token identifies a user
// playback request across Level instances, but stays unchanged across camera loops.
uint64_t g_NextPlaybackToken = 0u;

bool Text(const DATA_JSON_VALUE& row, const char* key, std::string& out)
{
    const auto* value = row.Find(key);
    if (!value || !value->Is_String()) return false;
    out = value->Get_String();
    return !out.empty();
}

bool Number(const DATA_JSON_VALUE& row, const char* key, double& out)
{
    const auto* value = row.Find(key);
    if (!value || !value->Is_Number() || !std::isfinite(value->Get_Number())) return false;
    out = value->Get_Number();
    return true;
}

template<size_t N>
bool Vector(const DATA_JSON_VALUE& row, const char* key, std::array<float, N>& out)
{
    const auto* value = row.Find(key);
    if (!value || !value->Is_Array() || value->Get_Array().size() != N) return false;
    for (size_t i = 0u; i < N; ++i)
    {
        const auto& component = value->Get_Array()[i];
        if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
            std::abs(component.Get_Number()) > 1000000.) return false;
        out[i] = static_cast<float>(component.Get_Number());
    }
    return true;
}

bool KeyTime(const DATA_JSON_VALUE& value, uint32_t durationMs, uint32_t& out)
{
    double time = 0.;
    if (!Number(value, "timeMs", time) || time < 0. || time > durationMs || std::floor(time) != time)
        return false;
    out = static_cast<uint32_t>(time);
    return true;
}

double SampleClock(const std::vector<CClassSelectionPresentation::CLOCK_KEY>& keys, double timeMs)
{
    const auto right = std::upper_bound(keys.begin(), keys.end(), timeMs,
        [](double time, const auto& key) { return time < key.timeMs; });
    if (right == keys.begin()) return right->sourceMs;
    if (right == keys.end()) return keys.back().sourceMs;
    const auto& left = *(right - 1);
    const double fraction = (timeMs - left.timeMs) / (right->timeMs - left.timeMs);
    return left.sourceMs + (right->sourceMs - left.sourceMs) * fraction;
}

bool ParseEffects(const DATA_JSON_VALUE& value, CClassSelectionPresentation::PHASE& phase,
    std::string& error)
{
    const auto* effects = value.Find("effects");
    if (!effects) return true;
    if (!effects->Is_Array() || effects->Get_Array().size() > 32u)
    { error = "Invalid class selection Effect count."; return false; }
    std::set<std::string> ids;
    for (const auto& row : effects->Get_Array())
    {
        CClassSelectionPresentation::EFFECT_TRACK track;
        std::array<float, 16> matrix;
        double start = 0., end = 0.;
        const auto* keys = row.Find("clockKeys");
        if (!Text(row, "effectId", track.effectId) || !Text(row, "assetId", track.assetId) ||
            !CWorldSequenceDocument::Is_ValidStableId(track.effectId) || !ids.insert(track.effectId).second ||
            track.assetId.size() > 512u || track.assetId.find("..") != std::string::npos ||
            track.assetId.find(':') != std::string::npos || track.assetId.front() == '/' ||
            track.assetId.find('\\') != std::string::npos || !Vector(row, "rootWorld", matrix) ||
            !Number(row, "startMs", start) || !Number(row, "endMs", end) ||
            start < 0. || end <= start || end > phase.durationMs ||
            !Number(row, "sourceLoopEndMs", track.sourceLoopEndMs) ||
            track.sourceLoopEndMs < 0. || track.sourceLoopEndMs > MAX_MS ||
            !keys || !keys->Is_Array() || keys->Get_Array().size() < 2u || keys->Get_Array().size() > 4096u)
        { error = "Invalid class selection Effect identity, transform or lifetime."; return false; }
        std::copy(matrix.begin(), matrix.end(), &track.rootWorld.m[0][0]);
        const float determinant = XMVectorGetX(XMMatrixDeterminant(XMLoadFloat4x4(&track.rootWorld)));
        if (!std::isfinite(determinant) || std::abs(determinant) < 1.e-12f ||
            std::abs(matrix[3]) > 1.e-6f || std::abs(matrix[7]) > 1.e-6f ||
            std::abs(matrix[11]) > 1.e-6f || std::abs(matrix[15] - 1.f) > 1.e-6f)
        { error = "Class selection Effect root must be a non-degenerate affine WORLD transform."; return false; }
        track.startMs = start;
        track.endMs = end;
        for (const auto& key : keys->Get_Array())
        {
            CClassSelectionPresentation::CLOCK_KEY parsed;
            if (!Number(key, "timeMs", parsed.timeMs) || !Number(key, "sourceMs", parsed.sourceMs) ||
                parsed.timeMs < 0. || parsed.timeMs > phase.durationMs ||
                parsed.sourceMs < 0. || parsed.sourceMs > MAX_MS ||
                (!track.clockKeys.empty() && (parsed.timeMs <= track.clockKeys.back().timeMs ||
                    parsed.sourceMs < track.clockKeys.back().sourceMs)))
            { error = "Class selection Effect clock must be finite, bounded and monotonic."; return false; }
            track.clockKeys.push_back(parsed);
        }
        if (track.clockKeys.front().timeMs != 0. || track.clockKeys.back().timeMs != phase.durationMs ||
            (track.sourceLoopEndMs > 0. && track.sourceLoopEndMs < track.clockKeys.back().sourceMs))
        { error = "Class selection Effect clock does not cover its phase or source loop."; return false; }
        phase.effects.push_back(std::move(track));
    }
    return true;
}

bool ParseMaterialsAndLights(const DATA_JSON_VALUE& value, CClassSelectionPresentation::PHASE& phase,
    std::string& error)
{
    if (const auto* materials = value.Find("materialTracks"))
    {
        if (!materials->Is_Array() || materials->Get_Array().size() > 128u)
        { error = "Invalid class selection material track count."; return false; }
        std::set<std::tuple<std::string, std::string, std::string>> targets;
        for (const auto& row : materials->Get_Array())
        {
            CClassSelectionPresentation::MATERIAL_TRACK track;
            const auto* parameters = row.Find("parameters");
            const auto* curves = row.Find("curves");
            Engine::MODEL_SOURCE_CHARACTER_PARAMETERS native;
            if (!Text(row, "instanceId", track.instanceId) || !Text(row, "slotId", track.slotId) ||
                !Text(row, "materialName", track.materialName) || !Text(row, "family", track.family) ||
                std::find(phase.instanceIds.begin(), phase.instanceIds.end(), track.instanceId) == phase.instanceIds.end() ||
                !targets.emplace(track.instanceId, track.slotId, track.materialName).second ||
                !parameters || !SourceCharacterMaterial::Read(*parameters, track.parameters) ||
                !SourceCharacterMaterial::Configure(track.family, track.parameters, native) ||
                !curves || !curves->Is_Array() || curves->Get_Array().empty() || curves->Get_Array().size() > 64u)
            { error = "Class selection material native profile or target is invalid."; return false; }
            std::set<std::string> names;
            for (const auto& curve : curves->Get_Array())
            {
                CClassSelectionPresentation::MATERIAL_CURVE parsed;
                const auto* keys = curve.Find("keys");
                if (!Text(curve, "parameter", parsed.parameter) || !names.insert(parsed.parameter).second ||
                    !track.parameters.contains(parsed.parameter) || !keys || !keys->Is_Array() ||
                    keys->Get_Array().empty() || keys->Get_Array().size() > 4096u)
                { error = "Invalid class selection native material curve."; return false; }
                for (const auto& key : keys->Get_Array())
                {
                    CClassSelectionPresentation::MATERIAL_KEY sample;
                    if (!KeyTime(key, phase.durationMs, sample.timeMs) || !Vector(key, "value", sample.value) ||
                        (!parsed.keys.empty() && sample.timeMs <= parsed.keys.back().timeMs))
                    { error = "Invalid class selection material key."; return false; }
                    parsed.keys.push_back(sample);
                }
                if (parsed.keys.front().timeMs != 0u || parsed.keys.back().timeMs != phase.durationMs)
                { error = "Class selection material curve does not cover its phase."; return false; }
                track.curves.push_back(std::move(parsed));
            }
            phase.materialTracks.push_back(std::move(track));
        }
    }
    if (const auto* lights = value.Find("lights"))
    {
        if (!lights->Is_Array() || lights->Get_Array().size() > 32u)
        { error = "Invalid class selection point light count."; return false; }
        std::set<std::string> ids;
        for (const auto& row : lights->Get_Array())
        {
            CClassSelectionPresentation::LIGHT_TRACK track;
            double radius = 0., falloff = 0.;
            const auto* keys = row.Find("keys");
            if (!Text(row, "lightId", track.lightId) || !ids.insert(track.lightId).second ||
                !Number(row, "radiusMeters", radius) || radius <= 0. || radius > 100000. ||
                !Number(row, "falloffExponent", falloff) || falloff <= 0. || falloff > 1000. ||
                !keys || !keys->Is_Array() || keys->Get_Array().empty() || keys->Get_Array().size() > 4096u)
            { error = "Invalid class selection point light definition."; return false; }
            track.radiusMeters = static_cast<float>(radius);
            track.falloffExponent = static_cast<float>(falloff);
            for (const auto& key : keys->Get_Array())
            {
                CClassSelectionPresentation::LIGHT_KEY sample;
                std::array<float, 3> position, color;
                double brightness = 0.;
                const auto* enabled = key.Find("enabled");
                if (!KeyTime(key, phase.durationMs, sample.timeMs) || !Vector(key, "position", position) ||
                    !Vector(key, "color", color) || !Number(key, "brightness", brightness) ||
                    brightness < 0. || brightness > 100000. || !enabled || !enabled->Is_Boolean() ||
                    std::any_of(color.begin(), color.end(), [](float c) { return c < 0.f; }) ||
                    (!track.keys.empty() && sample.timeMs <= track.keys.back().timeMs))
                { error = "Invalid class selection point light key."; return false; }
                sample.position = {position[0], position[1], position[2]};
                sample.color = {color[0], color[1], color[2]};
                sample.brightness = static_cast<float>(brightness);
                sample.enabled = enabled->Get_Boolean();
                track.keys.push_back(sample);
            }
            if (track.keys.front().timeMs != 0u || track.keys.back().timeMs != phase.durationMs)
            { error = "Class selection point light keys do not cover their phase."; return false; }
            phase.lights.push_back(std::move(track));
        }
    }
    return true;
}

bool ParsePhase(const DATA_JSON_VALUE* value, CClassSelectionPresentation::PHASE& out,
    std::string& error)
{
    double duration = 0.;
    if (!value || !value->Is_Object() || !Number(*value, "durationMs", duration) ||
        duration < 1. || duration > MAX_MS || std::floor(duration) != duration)
    { error = "A class selection phase needs a bounded integer durationMs."; return false; }
    CClassSelectionPresentation::PHASE phase;
    phase.durationMs = static_cast<uint32_t>(duration);
    const auto* ids = value->Find("instanceIds");
    if (!ids || !ids->Is_Array() || ids->Get_Array().empty() || ids->Get_Array().size() > 128u)
    { error = "A class selection phase needs 1..128 world sequence instances."; return false; }
    std::set<std::string> unique;
    for (const auto& id : ids->Get_Array())
    {
        if (!id.Is_String() || !CWorldSequenceDocument::Is_ValidStableId(id.Get_String()) ||
            !unique.insert(id.Get_String()).second)
        { error = "Invalid or duplicated class selection instance ID."; return false; }
        phase.instanceIds.push_back(id.Get_String());
    }
    if (!CEffectRecoveryCamera::Parse(*value, true, phase.cameras, error)) return false;
    uint32_t end = 0u;
    std::sort(phase.cameras.begin(), phase.cameras.end(),
        [](const auto& a, const auto& b) { return a.startMs < b.startMs; });
    for (const auto& camera : phase.cameras)
    {
        if (camera.muted || camera.modelRelative || camera.startMs != end ||
            camera.cue.iDurationMs > phase.durationMs - end)
        { error = "Class selection WORLD cameras must cover their phase without gaps or overlap."; return false; }
        end += camera.cue.iDurationMs;
    }
    if (end != phase.durationMs)
    { error = "Class selection cameras do not cover the complete phase."; return false; }
    if (const auto* clock = value->Find("clockKeys"))
    {
        if (!clock->Is_Array() || clock->Get_Array().size() < 2u || clock->Get_Array().size() > 4096u)
        { error = "Class selection clockKeys must contain 2..4096 samples."; return false; }
        for (const auto& key : clock->Get_Array())
        {
            CClassSelectionPresentation::CLOCK_KEY parsed;
            if (!key.Is_Object() || !Number(key, "timeMs", parsed.timeMs) ||
                !Number(key, "sourceMs", parsed.sourceMs) || parsed.timeMs < 0. ||
                parsed.timeMs > MAX_MS || parsed.sourceMs < 0. || parsed.sourceMs > duration ||
                (!phase.clockKeys.empty() && (parsed.timeMs <= phase.clockKeys.back().timeMs ||
                    parsed.sourceMs < phase.clockKeys.back().sourceMs)))
            { error = "Class selection clock must be finite, bounded and monotonic."; return false; }
            phase.clockKeys.push_back(parsed);
        }
        if (phase.clockKeys.front().timeMs != 0. || phase.clockKeys.front().sourceMs != 0. ||
            phase.clockKeys.back().sourceMs != duration)
        { error = "Class selection clock must cover zero through the phase duration."; return false; }
    }
    if (!ParseMaterialsAndLights(*value, phase, error) || !ParseEffects(*value, phase, error)) return false;
    out = std::move(phase);
    return true;
}

bool LoadScenes(const std::string& areaId, std::vector<CClassSelectionPresentation::SCENE>& scenes,
    std::string& error)
{
    const auto path = CProjectDataRoot::Resolve(L"Camera/ClassSelection.cinematics.json");
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec || size == 0u || size > 16u * 1024u * 1024u)
    { error = "Class selection cinematics are missing or exceed 16 MiB."; return false; }
    std::ifstream file(path, std::ios::binary);
    if (!file) { error = "Cannot read class selection cinematics."; return false; }
    const std::string text(std::istreambuf_iterator<char>(file), {});
    DATA_JSON_VALUE root;
    DATA_JSON_PARSE_LIMITS limits;
    limits.iMaximumBytes = 16u * 1024u * 1024u;
    limits.iMaximumValues = 1000000u;
    return !file.bad() && CDataJson::Parse(text, root, error, limits) &&
        CClassSelectionPresentation::Parse(root, areaId, scenes, error);
}
}

double CClassSelectionPresentation::PHASE::WallDurationMs() const
{ return clockKeys.empty() ? durationMs : clockKeys.back().timeMs; }

double CClassSelectionPresentation::PHASE::SourceTimeMs(const double wallMs) const
{
    if (clockKeys.empty()) return (std::clamp)(wallMs, 0., static_cast<double>(durationMs));
    return SampleClock(clockKeys, wallMs);
}

double CClassSelectionPresentation::EFFECT_TRACK::SourceTimeMs(const double timeMs) const
{ return SampleClock(clockKeys, timeMs); }

bool CClassSelectionPresentation::Parse(const DATA_JSON_VALUE& root, const std::string& areaId,
    std::vector<SCENE>& out, std::string& error)
{
    std::string schema, area;
    double version = 0.;
    const auto* scenes = root.Find("scenes");
    if (!root.Is_Object() || !Text(root, "schema", schema) || schema != "lostark.class-selection-cinematics" ||
        !Number(root, "formatVersion", version) || version != 1. || !Text(root, "areaId", area) ||
        area != areaId || !scenes || !scenes->Is_Array() || scenes->Get_Array().size() > 32u)
    { error = "Invalid class selection cinematics header."; return false; }
    std::vector<SCENE> staged;
    std::set<std::string> ids, classes;
    for (const auto& value : scenes->Get_Array())
    {
        SCENE scene;
        if (!value.Is_Object() || !Text(value, "classId", scene.classId) ||
            !Text(value, "sceneId", scene.sceneId) ||
            !CWorldSequenceDocument::Is_ValidStableId(scene.sceneId) ||
            scene.classId.size() > 64u || !classes.insert(scene.classId).second ||
            !ids.insert(scene.sceneId).second ||
            !std::all_of(scene.classId.begin(), scene.classId.end(), [](unsigned char c)
                { return (c >= 'A' && c <= 'Z') || c == '_'; }))
        { error = "Invalid or duplicated class selection scene identity."; return false; }
        if (!ParsePhase(value.Find("intro"), scene.intro, error) ||
            !ParsePhase(value.Find("loop"), scene.loop, error)) return false;
        staged.push_back(std::move(scene));
    }
    out = std::move(staged);
    error.clear();
    return true;
}

CClassSelectionPresentation::~CClassSelectionPresentation() { Clear(); }

bool CClassSelectionPresentation::Is_Configured()
{
    std::error_code error;
    return std::filesystem::is_regular_file(
        CProjectDataRoot::Resolve(L"Camera/ClassSelection.cinematics.json"), error);
}

bool CClassSelectionPresentation::Load_EffectTargets(const std::string& areaId,
    std::vector<std::string>& out, std::string& error)
{
    std::vector<SCENE> scenes;
    if (!LoadScenes(areaId, scenes, error)) return false;
    std::set<std::string> ids;
    for (const auto& scene : scenes)
        for (const auto* phase : {&scene.intro, &scene.loop})
            for (const auto& effect : phase->effects) ids.insert(effect.assetId);
    out.assign(ids.begin(), ids.end());
    return true;
}

bool CClassSelectionPresentation::Initialize(const std::string& areaId,
    const CWorldSequencePlayer::TARGET_SET& targets, const std::shared_ptr<CCamera_Free>& camera)
{
    if (Is_Active())
    { m_Status = "Stop the class selection cinematic before reloading it."; return false; }
    if (!targets.Is_Complete() || !camera)
    { m_Status = "Class selection cinematic targets are unavailable."; return false; }
    std::vector<SCENE> scenes;
    if (!LoadScenes(areaId, scenes, m_Status)) return false;
    if (!m_Resources.Load_PreparedArea(areaId, targets))
    { m_Status = m_Resources.Get_Status(); return false; }
    const auto& document = m_Resources.Get_Document();
    for (const auto& scene : scenes)
        for (const auto* phase : { &scene.intro, &scene.loop })
            for (const auto& id : phase->instanceIds)
            {
                const auto* instance = document.Find_Instance(id);
                const auto* sequence = instance ? document.Find_Template(instance->templateId) : nullptr;
                if (!instance || !instance->enabled || !sequence || instance->startDelayMs != 0u ||
                    instance->playbackSpeed != 1.f || sequence->durationMs != phase->durationMs ||
                    instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
                { m_Status = "Class selection phase does not match its admitted world sequence: " + id; return false; }
            }
    auto& game = CGameInstance::Get();
    if (FAILED(game.Add_Prototype(targets.levelIndex, CWorldSequenceObject::PROTOTYPE_TAG,
        CWorldSequenceObject::Create(targets.device, targets.context))))
    { m_Status = "Cannot prepare the class selection WorldSequence prototype."; return false; }
    // Both sides of the intro-to-loop transaction can be alive simultaneously.
    // Prepare reusable clones before a Play request, never at each camera loop.
    for (const auto& scene : scenes)
        for (const auto* phase : {&scene.intro, &scene.loop})
            for (const auto& id : phase->instanceIds)
                if (!m_Resources.Prewarm_ObjectInstances(id, 2u, targets))
                { m_Status = m_Resources.Get_Status(); return false; }
    m_Targets = targets;
    m_Targets.objectPreparationOwner = &m_Resources;
    m_Camera = camera;
    m_Scenes = std::move(scenes);
    m_Status = "Class selection cinematics ready.";
    return true;
}

bool CClassSelectionPresentation::Has_Class(const std::string& classId) const
{
    return std::any_of(m_Scenes.begin(), m_Scenes.end(),
        [&classId](const SCENE& scene) { return scene.classId == classId; });
}

const std::string& CClassSelectionPresentation::Get_ActiveClass() const
{
    static const std::string empty;
    return m_Scene ? m_Scene->classId : empty;
}

bool CClassSelectionPresentation::Play(const std::string& classId)
{
    const auto scene = std::find_if(m_Scenes.begin(), m_Scenes.end(),
        [&classId](const SCENE& value) { return value.classId == classId; });
    if (scene == m_Scenes.end())
    { m_Status = "No admitted class selection cinematic for " + classId + "."; return false; }
    const bool started = Start_Phase(*scene, false, 0.);
    if (started)
    { m_Paused = false; m_DeferAdvance = true; m_PlaybackToken = ++g_NextPlaybackToken; }
    return started;
}

double CClassSelectionPresentation::Get_DurationMs() const
{ return m_Scene ? (m_Looping ? m_Scene->loop : m_Scene->intro).WallDurationMs() : 0.; }

double CClassSelectionPresentation::Get_PhaseDurationMs(const std::string& classId, bool loop) const
{
    const auto scene = std::find_if(m_Scenes.begin(), m_Scenes.end(),
        [&classId](const auto& value) { return value.classId == classId; });
    return scene == m_Scenes.end() ? 0. : (loop ? scene->loop : scene->intro).WallDurationMs();
}

bool CClassSelectionPresentation::Seek(const std::string& classId, const bool loop, const double wallMs)
{
    const auto scene = std::find_if(m_Scenes.begin(), m_Scenes.end(),
        [&classId](const auto& value) { return value.classId == classId; });
    if (scene == m_Scenes.end() || !std::isfinite(wallMs) || wallMs < 0. ||
        wallMs > (loop ? scene->loop : scene->intro).WallDurationMs())
    { m_Status = "Class selection seek is outside the selected phase."; return false; }
    // Scrubbing is explicitly discontinuous; rebuild only this scene's PSC history.
    // Normal forward camera loops below retain both actors and Effect handles.
    Stop_Effects();
    const double sample = (std::min)(wallMs, std::nextafter(
        (loop ? scene->loop : scene->intro).WallDurationMs(), 0.));
    if (!Start_Phase(*scene, loop, sample)) return false;
    m_PlaybackToken = ++g_NextPlaybackToken;
    m_Paused = true;
    m_DeferAdvance = true;
    return true;
}

bool CClassSelectionPresentation::Start_Phase(const SCENE& scene, const bool loop,
    const double elapsedMs, const uint64_t loopCycle)
{
    const auto camera = m_Camera.lock();
    if (!camera) { m_Status = "Class selection camera is unavailable."; return false; }
    if (!camera->Begin_PresentationOverride(CAMERA_OWNER))
    { m_Status = "Another presentation owns the camera."; return false; }
    const bool acquiredCamera = !m_OwnsCamera;
    m_OwnsCamera = true;
    const auto& phase = loop ? scene.loop : scene.intro;
    auto staged = std::make_unique<CWorldSequencePlayer>();
    if (!staged->Set_Document(m_Resources.Get_Document(), m_Targets, m_Status))
    {
        if (acquiredCamera) { camera->End_PresentationOverride(CAMERA_OWNER); m_OwnsCamera = false; }
        return false;
    }
    const float initialSourceMs = (std::min)(static_cast<float>(phase.SourceTimeMs(elapsedMs)),
        std::nextafter(static_cast<float>(phase.durationMs), 0.f));
    for (const auto& id : phase.instanceIds)
    {
        if (!m_Resources.Prepare_InstanceResources(id, m_Targets) ||
            !staged->Play(id, m_Targets) ||
            !staged->Seek_InstanceToMs(id, initialSourceMs, m_Targets))
        {
            m_Status = "Class selection start failed for " + id + ": " + staged->Get_Status() +
                "; preparation: " + m_Resources.Get_Status();
            staged->Stop_All(m_Targets, true);
            if (acquiredCamera) { camera->End_PresentationOverride(CAMERA_OWNER); m_OwnsCamera = false; }
            return false;
        }
    }
    if (!loop || m_Scene != &scene) Stop_Effects();
    if (m_Active) m_Active->Stop_All(m_Targets, true);
    m_Active = std::move(staged);
    m_Scene = &scene;
    m_Looping = loop;
    m_ElapsedMs = elapsedMs;
    m_LoopCycle = loopCycle;
    if (!Sample_Frame()) { Fail(m_Status); return false; }
    m_Status = loop ? "Class selection cinematic looping." : "Class selection cinematic intro.";
    return true;
}

bool CClassSelectionPresentation::Sample_Frame()
{
    if (!m_Active || !m_Scene) return false;
    const auto camera = m_Camera.lock();
    if (!camera) { m_Status = "Class selection camera was released."; return false; }
    const auto& phase = m_Looping ? m_Scene->loop : m_Scene->intro;
    const float sampleMs = (std::min)(static_cast<float>(phase.SourceTimeMs(m_ElapsedMs)),
        std::nextafter(static_cast<float>(phase.durationMs), 0.f));
    for (const auto& id : phase.instanceIds)
        if (!m_Active->Seek_InstanceToMs(id, static_cast<float>(sampleMs), m_Targets, false))
        { m_Status = "Class selection world sample failed: " + id + "; " + m_Active->Get_Status(); return false; }
    const uint32_t timeMs = static_cast<uint32_t>(sampleMs);
    const auto row = std::find_if(phase.cameras.begin(), phase.cameras.end(),
        [timeMs](const EFFECT_CAMERA_ROW& value)
        { return timeMs >= value.startMs && timeMs < value.startMs + value.cue.iDurationMs; });
    float4x4_t identity;
    XMStoreFloat4x4(&identity, XMMatrixIdentity());
    VALTAN_CINEMATIC_CAMERA_POSE pose;
    float3_t up{};
    if (row == phase.cameras.end() || !CEffectRecoveryCamera::Sample(*row, timeMs, identity,
        camera->Get_AspectRatio(), pose, up, m_Status)) return false;
    if (!camera->Apply_PresentationPoseWithUp(CAMERA_OWNER, pose.vEye, pose.vLookAt,
        up, pose.fFovYDegrees))
    { m_Status = "Class selection camera ownership was lost."; return false; }
    return Sample_MaterialsAndLights(phase, sampleMs) && Sample_Effects(phase, sampleMs);
}

bool CClassSelectionPresentation::Sample_Effects(const PHASE& phase, const float sampleMs)
{
    std::set<std::string> visible;
    for (const auto& track : phase.effects)
    {
        if (sampleMs < track.startMs || sampleMs >= track.endMs) continue;
        visible.insert(track.effectId);
        const double ageOffset = m_Looping ? m_LoopCycle *
            (track.clockKeys.back().sourceMs - track.clockKeys.front().sourceMs) : 0.;
        const float age = static_cast<float>((ageOffset + track.SourceTimeMs(sampleMs)) * .001);
        // Stop owns this continuous occurrence. Keep a full next-cycle margin so
        // float rounding at a camera boundary cannot kill a Lifetime=0 burst.
        const float end = track.sourceLoopEndMs > 0. ?
            static_cast<float>((ageOffset + track.sourceLoopEndMs +
                (std::max)(1000., track.clockKeys.back().sourceMs - track.clockKeys.front().sourceMs)) * .001) : 0.f;
        auto active = m_Effects.find(track.effectId);
        if (active != m_Effects.end() && active->second.assetId != track.assetId)
        { m_Status = "Class selection Effect identity changed across phases: " + track.effectId; return false; }
        if (active == m_Effects.end())
        {
            EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
            desc.iLevelIndex = m_Targets.levelIndex;
            desc.strPlacementId = m_Scene->sceneId + "." + track.effectId;
            desc.strEffectAssetId = track.assetId;
            desc.RootWorld = track.rootWorld;
            desc.fInitialSampleTimeSeconds = age;
            desc.bExternallySampled = true;
            desc.fSourceLoopEndSeconds = end;
            ACTIVE_EFFECT value;
            value.assetId = track.assetId;
            if (!CEffectPresentationService::Spawn_LevelPlacement(desc, value.handle, m_Status)) return false;
            active = m_Effects.emplace(track.effectId, std::move(value)).first;
        }
        else if (end > 0.f && !CEffectPresentationService::Set_WorldRootSourceLoopEndSeconds(
            active->second.handle, end, m_Status)) return false;
        const auto provider = [world = track.rootWorld](float, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& out,
            std::string& error)
        {
            out.RootWorld = world;
            out.SourceAnchorWorlds.clear();
            error.clear();
            return true;
        };
        if (!CEffectPresentationService::Update_WorldRoot(active->second.handle, track.rootWorld) ||
            !CEffectPresentationService::Seek_WorldRoot(active->second.handle, age, provider))
        { m_Status = "Class selection Effect sampling failed: " + track.effectId; return false; }
    }
    for (auto it = m_Effects.begin(); it != m_Effects.end(); )
    {
        if (visible.contains(it->first)) { ++it; continue; }
        CEffectPresentationService::Stop_WorldRoot(it->second.handle);
        it = m_Effects.erase(it);
    }
    return true;
}

void CClassSelectionPresentation::Stop_Effects()
{
    for (const auto& [id, effect] : m_Effects) CEffectPresentationService::Stop_WorldRoot(effect.handle);
    m_Effects.clear();
}

bool CClassSelectionPresentation::Sample_MaterialsAndLights(const PHASE& phase, const float sampleMs)
{
    const auto interval = [sampleMs](const auto& keys, float& fraction)
    {
        auto right = std::upper_bound(keys.begin(), keys.end(), sampleMs,
            [](float time, const auto& key) { return time < key.timeMs; });
        if (right == keys.end()) right = keys.end() - 1;
        const auto left = right == keys.begin() ? right : right - 1;
        fraction = right == left ? 0.f : (sampleMs - left->timeMs) / (right->timeMs - left->timeMs);
        return std::pair(left, right);
    };
    for (const auto& track : phase.materialTracks)
    {
        auto values = track.parameters;
        for (const auto& curve : track.curves)
        {
            float f = 0.f;
            const auto [left, right] = interval(curve.keys, f);
            auto& value = values.at(curve.parameter);
            for (size_t i = 0u; i < value.size(); ++i)
                value[i] = left->value[i] + (right->value[i] - left->value[i]) * f;
        }
        Engine::MODEL_SOURCE_CHARACTER_PARAMETERS native;
        if (!SourceCharacterMaterial::Configure(track.family, values, native) ||
            !m_Active->Set_ObjectMaterialConstants(track.instanceId, track.slotId, track.materialName, native))
        { m_Status = "Class selection material sampling failed: " + track.materialName + "; " + m_Active->Get_Status(); return false; }
    }
    if (!m_LightFrame) m_LightFrame = std::make_shared<CClassSelectionLightFrame>();
    std::vector<Engine::LIGHT_DESC> lights;
    for (const auto& track : phase.lights)
    {
        float f = 0.f;
        const auto [left, right] = interval(track.keys, f);
        if (!left->enabled) continue;
        const auto lerp = [f](float a, float b) { return a + (b - a) * f; };
        EFFECT_EVALUATED_LIGHT evaluated;
        evaluated.vWorldPosition = {lerp(left->position.x, right->position.x),
            lerp(left->position.y, right->position.y), lerp(left->position.z, right->position.z)};
        evaluated.vColor = {lerp(left->color.x, right->color.x), lerp(left->color.y, right->color.y),
            lerp(left->color.z, right->color.z), 1.f};
        evaluated.vAmbient = {0.f, 0.f, 0.f, 0.f};
        evaluated.fIntensity = lerp(left->brightness, right->brightness);
        evaluated.fRange = track.radiusMeters;
        evaluated.fFalloffExponent = track.falloffExponent;
        if (evaluated.fIntensity == 0.f) continue;
        Engine::LIGHT_DESC light;
        if (!Try_BuildEffectPointLightDesc(evaluated, light))
        { m_Status = "Class selection point light mapping failed: " + track.lightId; return false; }
        lights.push_back(light);
    }
    m_LightFrame->lights = std::move(lights);
    if (!m_LightFrame->lights.empty() &&
        FAILED(Engine::CPresentation_Manager::Get().Add_FrameProvider(m_LightFrame)))
    { m_Status = "Class selection light submission failed."; return false; }
    return true;
}

void CClassSelectionPresentation::Update(const float deltaSeconds)
{
    if (!m_Active || !m_Scene) return;
    if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.f)
    { Fail("Invalid class selection frame time."); return; }
    // This is a visual preview clock. Loading/debugger stalls pause it instead of
    // replaying hours of particle history from zero in one frame. Server time is unaffected.
    const double deltaMs = (m_Paused || m_DeferAdvance) ? 0. :
        (std::min)(static_cast<double>(deltaSeconds), .25) * 1000.;
    m_DeferAdvance = false;
    m_Active->Update_SoundTails(static_cast<float>(deltaMs * .001));
    m_ElapsedMs += deltaMs;
    const auto& phase = m_Looping ? m_Scene->loop : m_Scene->intro;
    if (m_ElapsedMs >= phase.WallDurationMs())
    {
        const double overflow = m_ElapsedMs - phase.WallDurationMs();
        const double loopDuration = m_Scene->loop.WallDurationMs();
        const uint64_t cycles = (m_Looping ? m_LoopCycle + 1u : 0u) +
            static_cast<uint64_t>(std::floor(overflow / loopDuration));
        const double remainder = std::fmod(overflow, loopDuration);
        if (m_Looping)
        {
            m_LoopCycle = cycles;
            m_ElapsedMs = remainder;
            if (!Sample_Frame()) Fail(m_Status);
        }
        else if (!Start_Phase(*m_Scene, true, remainder, cycles)) Fail(m_Status);
        return;
    }
    if (!Sample_Frame()) Fail(m_Status);
}

void CClassSelectionPresentation::Stop()
{
    const bool wasActive = Is_Active();
    Stop_Effects();
    // A provider may already be queued for this frame; clear its payload in place.
    if (m_LightFrame) m_LightFrame->lights.clear();
    if (m_Active) m_Active->Stop_All(m_Targets, true);
    m_Active.reset();
    if (m_OwnsCamera)
        if (const auto camera = m_Camera.lock()) camera->End_PresentationOverride(CAMERA_OWNER);
    m_OwnsCamera = false;
    m_Scene = nullptr;
    m_ElapsedMs = 0.;
    m_LoopCycle = 0u;
    m_Looping = false;
    m_Paused = false;
    m_DeferAdvance = false;
    m_PlaybackToken = 0u;
    if (wasActive) m_Status = "Class selection cinematic stopped.";
}

void CClassSelectionPresentation::Fail(const std::string& reason)
{
    const auto retained = reason;
    Stop();
    m_Status = retained;
    OutputDebugStringA(("[ClassSelectionPresentation] " + m_Status + "\n").c_str());
}

void CClassSelectionPresentation::Clear()
{
    Stop();
    m_Resources.Clear();
    m_Targets = {};
    m_Camera.reset();
    m_LightFrame.reset();
    m_Scenes.clear();
}
}
