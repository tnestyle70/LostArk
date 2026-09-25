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

bool IsBackgroundAreaId(const std::string& value)
{
    return value.size() <= 64u && value != "." && value != ".." &&
        CWorldSequenceDocument::Is_ValidStableId(value);
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

bool ParameterVector(const DATA_JSON_VALUE& row, const char* key, const uint32_t count,
    std::array<float, 3>& out)
{
    if (count == 3u) return Vector(row, key, out);
    std::array<float, 1> scalar;
    if (count != 1u || !Vector(row, key, scalar)) return false;
    out = {scalar[0], 0.f, 0.f};
    return true;
}

bool ParseParameterTracks(const DATA_JSON_VALUE& row, CClassSelectionPresentation::EFFECT_TRACK& track,
    const uint32_t durationMs, std::string& error)
{
    const auto* curves = row.Find("parameterTracks");
    if (!curves) return true;
    if (!curves->Is_Array() || curves->Get_Array().size() > 64u)
    { error = "Invalid class selection particle parameter track count."; return false; }
    std::vector<EFFECT_PARAMETER_INPUT> identities;
    for (const auto& curve : curves->Get_Array())
    {
        CClassSelectionPresentation::EFFECT_PARAMETER_TRACK parsed;
        double count = 0.;
        const auto* keys = curve.Find("keys");
        if (!Text(curve, "parameterName", parsed.parameterName) ||
            !Number(curve, "componentCount", count) || (count != 1. && count != 3.) ||
            !keys || !keys->Is_Array() || keys->Get_Array().empty() || keys->Get_Array().size() > 65536u)
        { error = "Invalid class selection particle parameter name, type or keys."; return false; }
        parsed.componentCount = static_cast<uint32_t>(count);
        EFFECT_PARAMETER_INPUT identity;
        identity.strName = parsed.parameterName;
        identity.eKind = parsed.componentCount == 1u ? EFFECT_PARAMETER_VALUE_KIND::SCALAR :
            EFFECT_PARAMETER_VALUE_KIND::VECTOR3;
        identities.push_back(std::move(identity));
        if (!CEffectDistribution::Validate_ParameterInputs(identities, error)) return false;
        for (const auto& key : keys->Get_Array())
        {
            CClassSelectionPresentation::EFFECT_PARAMETER_KEY value;
            std::string interpolation;
            if (!Number(key, "timeMs", value.timeMs) || value.timeMs < 0. || value.timeMs > durationMs ||
                (!parsed.keys.empty() && value.timeMs <= parsed.keys.back().timeMs) ||
                !ParameterVector(key, "value", parsed.componentCount, value.value) ||
                !ParameterVector(key, "arriveTangent", parsed.componentCount, value.arriveTangent) ||
                !ParameterVector(key, "leaveTangent", parsed.componentCount, value.leaveTangent) ||
                !Text(key, "interpolation", interpolation))
            { error = "Class selection particle parameter keys must be finite, typed and increasing."; return false; }
            if (interpolation == "CONSTANT") value.interpolation = EFFECT_DISTRIBUTION_INTERPOLATION::CONSTANT;
            else if (interpolation == "LINEAR") value.interpolation = EFFECT_DISTRIBUTION_INTERPOLATION::LINEAR;
            else if (interpolation == "CUBIC") value.interpolation = EFFECT_DISTRIBUTION_INTERPOLATION::CUBIC;
            else { error = "Unsupported class selection particle parameter interpolation."; return false; }
            parsed.keys.push_back(value);
        }
        track.parameterTracks.push_back(std::move(parsed));
    }
    return true;
}

bool ParseEffects(const DATA_JSON_VALUE& value, CClassSelectionPresentation::PHASE& phase,
    std::string& error)
{
    const auto* effects = value.Find("effects");
    if (!effects) return true;
    if (!effects->Is_Array() || effects->Get_Array().size() > 128u)
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
        const double ageSpan = track.clockKeys.back().sourceMs - track.clockKeys.front().sourceMs;
        track.loopAgeDeltaMs = ageSpan;
        if (row.Find("loopAgeDeltaMs") &&
            (!Number(row, "loopAgeDeltaMs", track.loopAgeDeltaMs) || track.loopAgeDeltaMs < 0. ||
                track.loopAgeDeltaMs > MAX_MS || (track.loopAgeDeltaMs > 0. &&
                    std::abs(track.loopAgeDeltaMs - ageSpan) > 0.001)))
        { error = "Class selection Effect loop age delta must be zero or match its clock span."; return false; }
        // Canonicalize a continuous delta so accepted rounding cannot accumulate gaps.
        if (track.loopAgeDeltaMs > 0.) track.loopAgeDeltaMs = ageSpan;
        if (const auto* roots = row.Find("rootKeys"))
        {
            if (!roots->Is_Array() || roots->Get_Array().size() < 2u || roots->Get_Array().size() > 65536u)
            { error = "Invalid class selection Effect root key count."; return false; }
            for (const auto& root : roots->Get_Array())
            {
                CClassSelectionPresentation::EFFECT_ROOT_KEY parsed;
                std::array<float, 3> position, scale;
                std::array<float, 4> quaternion;
                if (!Number(root, "timeMs", parsed.timeMs) || parsed.timeMs < 0. ||
                    parsed.timeMs > phase.durationMs ||
                    (!track.rootKeys.empty() && parsed.timeMs <= track.rootKeys.back().timeMs) ||
                    !Vector(root, "position", position) || !Vector(root, "scale", scale) ||
                    !Vector(root, "rotationQuaternion", quaternion))
                { error = "Class selection Effect root keys must be finite and strictly monotonic."; return false; }
                parsed.position = {position[0], position[1], position[2]};
                parsed.scale = {scale[0], scale[1], scale[2]};
                parsed.rotationQuaternion = {quaternion[0], quaternion[1], quaternion[2], quaternion[3]};
                const auto rotation = XMLoadFloat4(&parsed.rotationQuaternion);
                const float lengthSquared = XMVectorGetX(XMQuaternionLengthSq(rotation));
                const double determinant = static_cast<double>(scale[0]) * scale[1] * scale[2];
                if (!std::isfinite(lengthSquared) || lengthSquared < 1.e-12f ||
                    std::abs(determinant) < 1.e-12 ||
                    (!track.rootKeys.empty() &&
                        (scale[0] * track.rootKeys.back().scale.x <= 0.f ||
                         scale[1] * track.rootKeys.back().scale.y <= 0.f ||
                         scale[2] * track.rootKeys.back().scale.z <= 0.f)))
                { error = "Class selection Effect root interpolation must stay non-degenerate."; return false; }
                XMStoreFloat4(&parsed.rotationQuaternion, XMQuaternionNormalize(rotation));
                track.rootKeys.push_back(parsed);
            }
            if (track.rootKeys.front().timeMs != 0. || track.rootKeys.back().timeMs != phase.durationMs)
            { error = "Class selection Effect roots must cover the whole phase."; return false; }
        }
        if (!ParseParameterTracks(row, track, phase.durationMs, error)) return false;
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
                double brightness = 0., keyRadius = track.radiusMeters;
                const auto* enabled = key.Find("enabled");
                if (!KeyTime(key, phase.durationMs, sample.timeMs) || !Vector(key, "position", position) ||
                    !Vector(key, "color", color) || !Number(key, "brightness", brightness) ||
                    brightness < 0. || brightness > 100000. || !enabled || !enabled->Is_Boolean() ||
                    (key.Find("radiusMeters") && (!Number(key, "radiusMeters", keyRadius) ||
                        keyRadius <= 0. || keyRadius > 100000.)) ||
                    std::any_of(color.begin(), color.end(), [](float c) { return c < 0.f; }) ||
                    (!track.keys.empty() && sample.timeMs <= track.keys.back().timeMs))
                { error = "Invalid class selection point light key."; return false; }
                sample.position = {position[0], position[1], position[2]};
                sample.color = {color[0], color[1], color[2]};
                sample.brightness = static_cast<float>(brightness);
                sample.radiusMeters = static_cast<float>(keyRadius);
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


double CClassSelectionPresentation::PHASE::MovieTimeMs(const double sourceMs) const
{
    const double source = (std::clamp)(sourceMs, 0., static_cast<double>(durationMs));
    if (clockKeys.empty()) return source;
    const auto right = std::lower_bound(clockKeys.begin(), clockKeys.end(), source,
        [](const auto& key, double time) { return key.sourceMs < time; });
    if (right == clockKeys.begin()) return right->timeMs;
    if (right == clockKeys.end()) return clockKeys.back().timeMs;
    const auto& left = *(right - 1);
    return left.timeMs + (right->timeMs - left.timeMs) *
        (source - left.sourceMs) / (right->sourceMs - left.sourceMs);
}

bool CClassSelectionPresentation::Set_PlaybackRate(const double rate)
{
    if (!std::isfinite(rate) || rate < .05 || rate > 2.) return false;
    m_PlaybackRate = rate;
    return true;
}

double CClassSelectionPresentation::Get_SourceClockMs() const
{ return m_Scene ? (m_Looping ? m_Scene->loop : m_Scene->intro).SourceTimeMs(m_ElapsedMs) : 0.; }

double CClassSelectionPresentation::Get_SourceRate() const
{
    if (!m_Scene) return 1.;
    const auto& phase = m_Looping ? m_Scene->loop : m_Scene->intro;
    if (phase.clockKeys.empty()) return 1.;
    auto right = std::upper_bound(phase.clockKeys.begin(), phase.clockKeys.end(), m_ElapsedMs,
        [](double time, const auto& key) { return time < key.timeMs; });
    if (right == phase.clockKeys.begin()) ++right;
    if (right == phase.clockKeys.end()) --right;
    const auto& left = *(right - 1);
    return (right->sourceMs - left.sourceMs) / (right->timeMs - left.timeMs);
}

std::shared_ptr<const CLASS_MOVIE_TIMELINE> CClassSelectionPresentation::Get_Timeline(
    const std::string& classId, const bool loop) const
{
    const auto key = std::make_pair(classId, loop);
    if (const auto found = m_Timelines.find(key); found != m_Timelines.end()) return found->second;
    const auto& scenes = m_Authoring ? m_Authoring->scenes : m_Scenes;
    const auto scene = std::find_if(scenes.begin(), scenes.end(),
        [&classId](const auto& value) { return value.classId == classId; });
    if (scene == scenes.end()) return {};
    const auto& phase = loop ? scene->loop : scene->intro;
    const auto& document = m_Authoring ? m_Authoring->document : m_Resources.Get_Document();
    auto timeline = std::make_shared<CLASS_MOVIE_TIMELINE>();
    timeline->classId = classId; timeline->loop = loop;
    timeline->movieDurationMs = phase.WallDurationMs(); timeline->sourceDurationMs = phase.durationMs;
    auto box = [&phase](std::string id, std::string label, std::string resource, double start, double end) {
        CLASS_MOVIE_TIMELINE_BOX value;
        value.id = std::move(id); value.label = std::move(label); value.resource = std::move(resource);
        value.sourceStartMs = start; value.sourceEndMs = end;
        value.movieStartMs = phase.MovieTimeMs(start); value.movieEndMs = phase.MovieTimeMs(end);
        return value;
    };
    auto add = [&timeline](const char* kind, std::string id, std::string label, CLASS_MOVIE_TIMELINE_BOX&& value) {
        timeline->rows.push_back({kind, std::move(id), std::move(label), {std::move(value)}});
    };
    add("World Model", "background", "Background", box("background", scene->backgroundAreaId,
        scene->backgroundAreaId, 0., phase.durationMs));
    for (const auto& id : phase.instanceIds)
    {
        const auto* instance = document.Find_Instance(id);
        const auto* sequence = instance ? document.Find_Template(instance->templateId) : nullptr;
        if (!sequence) continue;
        // The movie owner seeks every instance in phase-source milliseconds.
        for (const auto& track : sequence->tracks)
        {
            const auto binding = std::find_if(instance->bindings.begin(), instance->bindings.end(),
                [&track](const auto& value) { return value.slotId == track.slotId; });
            const auto* resource = binding == instance->bindings.end() ? nullptr : document.Find_ObjectResource(binding->targetId);
            auto value = box(id + ".transform." + track.slotId, resource ? resource->displayName : track.slotId,
                resource ? resource->modelAssetId : (binding == instance->bindings.end() ? id : binding->targetId),
                0., sequence->durationMs);
            for (const auto& frame : track.keys) value.keyMovieTimes.push_back(phase.MovieTimeMs(frame.timeMs));
            add("World Model", value.id, value.label, std::move(value));
        }
        for (size_t i = 0; i < sequence->animationTracks.size(); ++i)
        {
            const auto& track = sequence->animationTracks[i];
            double end = sequence->durationMs;
            for (const auto& next : sequence->animationTracks)
                if (next.slotId == track.slotId && next.startMs > track.startMs) end = (std::min)(end, double(next.startMs));
            auto value = box(id + ".animation." + std::to_string(i), track.displayName.empty() ? track.clipName : track.displayName,
                track.clipName, track.startMs, end);
            value.playbackRate = track.playbackRate; value.sourceOffsetMs = track.sourceStartMs;
            add("Animation", value.id, sequence->displayName + " / " + track.slotId, std::move(value));
        }
        for (const auto& track : sequence->soundTracks)
            add("Sound", id + "." + track.soundTrackId, sequence->displayName,
                box(id + "." + track.soundTrackId, track.soundTrackId, track.assetId,
                    track.startMs, track.startMs + track.durationMs));
    }
    CLASS_MOVIE_TIMELINE_ROW cameras{"Camera", "camera", "Director", {}};
    for (const auto& track : phase.cameras)
    {
        auto value = box(track.id, track.label, track.source, track.startMs, track.startMs + track.cue.iDurationMs);
        value.camera = track;
        for (const auto& frame : track.cue.Keyframes)
            value.keyMovieTimes.push_back(phase.MovieTimeMs(track.startMs + frame.iTimeMs));
        cameras.boxes.push_back(std::move(value));
    }
    timeline->rows.push_back(std::move(cameras));
    for (const auto& track : phase.effects)
        add("Effect", track.effectId, track.effectId,
            box(track.effectId, track.effectId, track.assetId, track.startMs, track.endMs));
    for (const auto& track : phase.materialTracks)
        for (const auto& curve : track.curves)
        {
            auto value = box(track.instanceId + "." + track.slotId + "." + curve.parameter,
                curve.parameter, track.materialName, 0., phase.durationMs);
            for (const auto& frame : curve.keys) value.keyMovieTimes.push_back(phase.MovieTimeMs(frame.timeMs));
            add("Material", value.id, track.materialName + " / " + curve.parameter, std::move(value));
        }
    for (const auto& track : phase.lights)
    {
        auto value = box(track.lightId, track.lightId, "Point light", 0., phase.durationMs);
        for (const auto& frame : track.keys) value.keyMovieTimes.push_back(phase.MovieTimeMs(frame.timeMs));
        add("Light", value.id, value.label, std::move(value));
    }
    auto clock = box("movie-clock", "Source time dilation", "Movie clock -> source clock", 0., phase.durationMs);
    for (const auto& frame : phase.clockKeys) clock.keyMovieTimes.push_back(frame.timeMs);
    add("Time Control", "movie-clock", "Source time dilation", std::move(clock));
    m_Timelines.emplace(key, timeline);
    return timeline;
}

double CClassSelectionPresentation::EFFECT_TRACK::SourceTimeMs(const double timeMs) const
{ return SampleClock(clockKeys, timeMs); }

double CClassSelectionPresentation::EFFECT_TRACK::PhaseTimeMs(double ageMs) const
{
    // Float particle seconds can round an exact clock boundary in either direction.
    const double tolerance = (std::max)(0.001, std::abs(ageMs) * 2.4e-7);
    auto right = std::lower_bound(clockKeys.begin(), clockKeys.end(), ageMs,
        [](const auto& key, double age) { return key.sourceMs < age; });
    if (right != clockKeys.end() && std::abs(right->sourceMs - ageMs) <= tolerance)
        ageMs = right->sourceMs;
    else if (right != clockKeys.begin() && std::abs((right - 1)->sourceMs - ageMs) <= tolerance)
        ageMs = (right - 1)->sourceMs;
    right = std::lower_bound(clockKeys.begin(), clockKeys.end(), ageMs,
        [](const auto& key, double age) { return key.sourceMs < age; });
    if (right == clockKeys.begin()) return right->timeMs;
    if (right == clockKeys.end()) return clockKeys.back().timeMs;
    if (right->sourceMs == ageMs) return right->timeMs; // Earliest arrival on a plateau.
    const auto& left = *(right - 1);
    return left.timeMs + (right->timeMs - left.timeMs) *
        (ageMs - left.sourceMs) / (right->sourceMs - left.sourceMs);
}

float4x4_t CClassSelectionPresentation::EFFECT_TRACK::RootWorldAt(const double timeMs) const
{
    if (rootKeys.empty()) return rootWorld;
    auto right = std::upper_bound(rootKeys.begin(), rootKeys.end(), timeMs,
        [](double time, const auto& key) { return time < key.timeMs; });
    if (right == rootKeys.end()) right = rootKeys.end() - 1;
    const auto left = right == rootKeys.begin() ? right : right - 1;
    const float fraction = right == left ? 0.f : static_cast<float>(std::clamp(
        (timeMs - left->timeMs) / (right->timeMs - left->timeMs), 0., 1.));
    const auto position = XMVectorLerp(XMLoadFloat3(&left->position), XMLoadFloat3(&right->position), fraction);
    const auto scale = XMVectorLerp(XMLoadFloat3(&left->scale), XMLoadFloat3(&right->scale), fraction);
    const auto rotation = XMQuaternionNormalize(XMQuaternionSlerp(XMLoadFloat4(&left->rotationQuaternion),
        XMLoadFloat4(&right->rotationQuaternion), fraction));
    float4x4_t result;
    XMStoreFloat4x4(&result, XMMatrixAffineTransformation(scale, XMVectorZero(), rotation, position));
    return result;
}

const CClassSelectionPresentation::EFFECT_TRACK& CClassSelectionPresentation::EFFECT_TRACK::HistoryTrackAt(
    const double ageMs, const EFFECT_TRACK* intro, const uint64_t loopCycle, double& phaseMs) const
{
    const double firstAge = clockKeys.front().sourceMs;
    const double span = clockKeys.back().sourceMs - firstAge;
    const double tolerance = (std::max)(0.001, std::abs(ageMs) * 2.4e-7);
    const bool resetEpoch = loopAgeDeltaMs == 0. && span > 0.;
    if (intro && (!resetEpoch || loopCycle == 0u) &&
        std::abs(intro->clockKeys.back().sourceMs - firstAge) <= 0.001 && ageMs <= firstAge + tolerance)
    {
        phaseMs = intro->PhaseTimeMs(ageMs);
        return *intro;
    }
    double historyCycle = 0.;
    if (loopAgeDeltaMs > 0. && ageMs > firstAge + tolerance)
    {
        // Boundary particles belong to the segment that just ended.
        historyCycle = std::clamp(std::ceil((ageMs - firstAge - tolerance) / loopAgeDeltaMs) - 1.,
            0., static_cast<double>(loopCycle));
    }
    phaseMs = PhaseTimeMs(ageMs - historyCycle * loopAgeDeltaMs);
    return *this;
}

float4x4_t CClassSelectionPresentation::EFFECT_TRACK::HistoryRootWorld(const double ageMs,
    const EFFECT_TRACK* intro, const uint64_t loopCycle) const
{
    double phaseMs = 0.;
    const auto& history = HistoryTrackAt(ageMs, intro, loopCycle, phaseMs);
    return history.RootWorldAt(phaseMs);
}

std::vector<EFFECT_PARAMETER_INPUT> CClassSelectionPresentation::EFFECT_TRACK::ParametersAt(
    const double phaseMs) const
{
    std::vector<EFFECT_PARAMETER_INPUT> result;
    result.reserve(parameterTracks.size());
    for (const auto& track : parameterTracks)
    {
        auto right = std::upper_bound(track.keys.begin(), track.keys.end(), phaseMs,
            [](double time, const auto& key) { return time < key.timeMs; });
        if (right == track.keys.end()) right = track.keys.end() - 1;
        const auto left = right == track.keys.begin() ? right : right - 1;
        const double fraction = right == left ? 0. : std::clamp(
            (phaseMs - left->timeMs) / (right->timeMs - left->timeMs), 0., 1.);
        auto sampled = left->value;
        for (uint32_t lane = 0u; lane < track.componentCount; ++lane)
        {
            // At and after an exact key, use that key even for a held segment.
            if (phaseMs >= right->timeMs) sampled[lane] = right->value[lane];
            else if (left->interpolation == EFFECT_DISTRIBUTION_INTERPOLATION::LINEAR)
                sampled[lane] = static_cast<float>(left->value[lane] +
                    fraction * (right->value[lane] - left->value[lane]));
            else if (left->interpolation == EFFECT_DISTRIBUTION_INTERPOLATION::CUBIC)
            {
                const double squared = fraction * fraction, cubed = squared * fraction;
                const double seconds = (right->timeMs - left->timeMs) * .001;
                sampled[lane] = static_cast<float>((2. * cubed - 3. * squared + 1.) * left->value[lane] +
                    (cubed - 2. * squared + fraction) * seconds * left->leaveTangent[lane] +
                    (-2. * cubed + 3. * squared) * right->value[lane] +
                    (cubed - squared) * seconds * right->arriveTangent[lane]);
            }
        }
        EFFECT_PARAMETER_INPUT input;
        input.strName = track.parameterName;
        input.eKind = track.componentCount == 1u ? EFFECT_PARAMETER_VALUE_KIND::SCALAR :
            EFFECT_PARAMETER_VALUE_KIND::VECTOR3;
        if (track.componentCount == 1u) input.fScalarValue = sampled[0];
        else input.vVectorValue = {sampled[0], sampled[1], sampled[2]};
        result.push_back(std::move(input));
    }
    return result;
}

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
        if (const auto* background = value.Find("backgroundAreaId"))
        {
            if (!background->Is_String() || !IsBackgroundAreaId(background->Get_String()))
            { error = "Invalid class selection background Area for " + scene.classId + "."; return false; }
            scene.backgroundAreaId = background->Get_String();
        }
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

bool CClassSelectionPresentation::Load_BackgroundAreas(const std::string& areaId,
    const std::string& fallbackAreaId, std::vector<std::string>& outAreaIds, std::string& outStatus)
{
    std::vector<SCENE> scenes;
    if (!LoadScenes(areaId, scenes, outStatus)) return false;
    std::vector<std::string> staged;
    std::set<std::string> seen;
    for (const auto& scene : scenes)
    {
        const auto& background = scene.backgroundAreaId.empty() ? fallbackAreaId : scene.backgroundAreaId;
        if (!IsBackgroundAreaId(background))
        { outStatus = "Class selection background Area is not configured for " + scene.classId + "."; return false; }
        if (seen.insert(background).second) staged.push_back(background);
    }
    outAreaIds = std::move(staged);
    outStatus.clear();
    return true;
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

const std::string& CClassSelectionPresentation::Get_BackgroundAreaId(const std::string& classId) const
{
    static const std::string empty;
    const auto scene = std::find_if(m_Scenes.begin(), m_Scenes.end(),
        [&classId](const SCENE& value) { return value.classId == classId; });
    return scene == m_Scenes.end() ? empty : scene->backgroundAreaId;
}

bool CClassSelectionPresentation::Play(const std::string& classId)
{
    // Opening the editor only reads a source draft. Explicit Play admits that
    // already-read generation; it never reloads files or discards unsaved edits.
    if (m_Authoring && !m_Authoring->appliedToPreview)
        if (!Prepare_Authoring(m_Authoring->scenes, m_Authoring->document, m_Status) ||
            !Commit_Authoring(m_Authoring->scenes, m_Authoring->document, m_Status)) return false;
    const auto scene = std::find_if(m_Scenes.begin(), m_Scenes.end(),
        [&classId](const SCENE& value) { return value.classId == classId; });
    if (scene == m_Scenes.end())
    { m_Status = "No admitted class selection cinematic for " + classId + "."; return false; }
    const bool started = Start_Phase(*scene, false, 0., 0u, false);
    if (started)
    { m_DeferAdvance = true; m_PlaybackToken = ++g_NextPlaybackToken; }
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
    const double sample = (std::min)(wallMs, std::nextafter(
        (loop ? scene->loop : scene->intro).WallDurationMs(), 0.));
    if (!Start_Phase(*scene, loop, sample, 0u, true, true)) return false;
    m_PlaybackToken = ++g_NextPlaybackToken;
    m_DeferAdvance = true;
    return true;
}

bool CClassSelectionPresentation::Start_Phase(const SCENE& scene, const bool loop,
    const double elapsedMs, const uint64_t loopCycle, const bool desiredPaused, const bool rebuildEffects)
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
    staged->Set_Paused(true);
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
    if (rebuildEffects || !loop || m_Scene != &scene) Stop_Effects();
    if (m_Active) m_Active->Stop_All(m_Targets, true);
    m_Active = std::move(staged);
    m_Scene = &scene;
    m_Looping = loop;
    m_ElapsedMs = elapsedMs;
    m_LoopCycle = loopCycle;
    if (!Sample_Frame()) { Fail(m_Status); return false; }
    Set_Paused(desiredPaused);
    m_Status = loop ? "Class selection cinematic looping." : "Class selection cinematic intro.";
    return true;
}

void CClassSelectionPresentation::Set_Paused(const bool paused)
{
    m_Paused = paused;
    if (m_Active) m_Active->Set_Paused(paused);
}

bool CClassSelectionPresentation::Sample_Frame()
{
    if (!m_Active || !m_Scene) return false;
    const auto camera = m_Camera.lock();
    if (!camera) { m_Status = "Class selection camera was released."; return false; }
    const auto& phase = m_Looping ? m_Scene->loop : m_Scene->intro;
    const float sampleMs = (std::min)(static_cast<float>(phase.SourceTimeMs(m_ElapsedMs)),
        std::nextafter(static_cast<float>(phase.durationMs), 0.f));
    m_Active->Set_ExternalSoundClockRate(static_cast<float>(m_PlaybackRate * Get_SourceRate()));
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
    m_CameraSample = {true, row->id, m_ElapsedMs, static_cast<double>(timeMs), camera->Get_AspectRatio(), pose, up};
    return Sample_MaterialsAndLights(phase, sampleMs) && Sample_Effects(phase, sampleMs);
}

bool CClassSelectionPresentation::Sample_Effects(const PHASE& phase, const float sampleMs)
{
    std::set<std::string> visible;
    for (const auto& track : phase.effects)
    {
        if (sampleMs < track.startMs || sampleMs >= track.endMs) continue;
        visible.insert(track.effectId);
        const double ageOffset = m_Looping ? m_LoopCycle * track.loopAgeDeltaMs : 0.;
        const auto currentRoot = track.RootWorldAt(sampleMs);
        const auto currentParameters = track.ParametersAt(sampleMs);
        const EFFECT_TRACK* intro = nullptr;
        if (m_Looping)
        {
            const auto found = std::find_if(m_Scene->intro.effects.begin(), m_Scene->intro.effects.end(),
                [&track](const auto& value) { return value.effectId == track.effectId && value.assetId == track.assetId; });
            if (found != m_Scene->intro.effects.end()) intro = &*found;
        }
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
            desc.RootWorld = currentRoot;
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
        const bool resetEpoch = m_Looping && track.loopAgeDeltaMs == 0. &&
            track.clockKeys.back().sourceMs > track.clockKeys.front().sourceMs;
        const bool carryIntro = intro &&
            std::abs(intro->clockKeys.back().sourceMs - track.clockKeys.front().sourceMs) <= 0.001;
        const bool rebuildHistory = m_Looping &&
            ((!active->second.sampledInLoop && (!carryIntro || (resetEpoch && m_LoopCycle != 0u))) ||
                (resetEpoch && active->second.sampledInLoop && active->second.loopCycle != m_LoopCycle));
        const auto provider = [track = &track, intro, loop = m_Looping, cycle = m_LoopCycle]
            (float particleAge, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& out, std::string& error)
        {
            const double ageMs = static_cast<double>(particleAge) * 1000.;
            double historyMs = track->PhaseTimeMs(ageMs);
            const auto& history = loop ? track->HistoryTrackAt(ageMs, intro, cycle, historyMs) : *track;
            out.RootWorld = history.RootWorldAt(historyMs);
            out.ParticleParameters = history.ParametersAt(historyMs);
            out.SourceAnchorWorlds.clear();
            error.clear();
            return true;
        };
        if (!CEffectPresentationService::Update_WorldRoot(active->second.handle, currentRoot) ||
            !CEffectPresentationService::Seek_WorldRoot(active->second.handle, age, provider,
                rebuildHistory, 0.f, &currentRoot, &currentParameters))
        { m_Status = "Class selection Effect sampling failed: " + track.effectId; return false; }
        active->second.sampledInLoop = m_Looping;
        active->second.loopCycle = m_LoopCycle;
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
        evaluated.fRange = lerp(left->radiusMeters, right->radiusMeters);
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
    Poll_AuthoringPublish();
    if (!m_Active || !m_Scene) return;
    if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.f)
    { Fail("Invalid class selection frame time."); return; }
    // This is a visual preview clock. Loading/debugger stalls pause it instead of
    // replaying hours of particle history from zero in one frame. Server time is unaffected.
    const double deltaMs = (m_Paused || m_DeferAdvance) ? 0. :
        (std::min)(static_cast<double>(deltaSeconds), .25) * 1000. * m_PlaybackRate;
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
        else if (!Start_Phase(*m_Scene, true, remainder, cycles, m_Paused)) Fail(m_Status);
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
    m_CameraSample = {};
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
    m_Authoring.reset();
    m_Resources.Clear();
    m_Targets = {};
    m_Camera.reset();
    m_LightFrame.reset();
    m_Scenes.clear();
    m_Timelines.clear();
    m_PlaybackRate = 1.;
}
}
