#include "BoneAnimationDocument.h"
#include "Animation.h"
#include "DataJson.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <io.h>
#include <iterator>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace Client
{
namespace
{
using Json = DATA_JSON_VALUE;
bool Stable(const std::string& text)
{
    return !text.empty() && text.size() < 128u && text != "." && text != ".." &&
        std::all_of(text.begin(), text.end(), [](unsigned char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-'; });
}
const Json& Required(const Json& value, const char* field)
{
    const auto* found = value.Find(field);
    if (!found) throw std::runtime_error(std::string("Missing field: ") + field);
    return *found;
}
std::string Text(const Json& value, const char* field)
{
    const auto& found = Required(value, field);
    if (!found.Is_String()) throw std::runtime_error(std::string("Invalid text: ") + field);
    return found.Get_String();
}
double Number(const Json& value, const char* field, double minimum, double maximum)
{
    const auto& found = Required(value, field);
    if (!found.Is_Number() || !std::isfinite(found.Get_Number()) ||
        found.Get_Number() < minimum || found.Get_Number() > maximum)
        throw std::runtime_error(std::string("Invalid number: ") + field);
    return found.Get_Number();
}
uint32_t Milliseconds(const Json& value, const char* field, uint32_t minimum = 0u)
{
    const auto number = Number(value, field, minimum, 60000u);
    if (std::floor(number) != number) throw std::runtime_error("Time must use whole milliseconds");
    return static_cast<uint32_t>(number);
}
const Json::ARRAY& Array(const Json& value, const char* field, size_t maximum)
{
    const auto& found = Required(value, field);
    if (!found.Is_Array() || found.Get_Array().size() > maximum)
        throw std::runtime_error(std::string("Invalid array: ") + field);
    return found.Get_Array();
}
void Vector(const Json& value, const char* field, float* output, size_t count)
{
    const auto& values = Array(value, field, count);
    if (values.size() != count) throw std::runtime_error("Invalid vector length");
    for (size_t i = 0u; i < count; ++i)
    {
        if (!values[i].Is_Number() || !std::isfinite(values[i].Get_Number()) || std::abs(values[i].Get_Number()) > 100000.0)
            throw std::runtime_error("Invalid vector component");
        output[i] = static_cast<float>(values[i].Get_Number());
    }
}
bool Read(const std::filesystem::path& path, std::string& text, bool& exists)
{
    std::error_code error;
    exists = std::filesystem::exists(path, error);
    if (error) return false;
    if (!exists) { text.clear(); return true; }
    if (std::filesystem::file_size(path, error) > 8u * 1024u * 1024u || error) return false;
    std::ifstream input(path, std::ios::binary);
    if (!input) return false;
    text.assign(std::istreambuf_iterator<char>(input), {});
    return !input.bad();
}
float Duration(const Engine::CModel& model, const std::string& name)
{
    float result = -1.f;
    for (uint32_t i = 0u; i < model.Get_NumAnimations(); ++i)
        if (name == model.Get_AnimationName(i))
        {
            float position = 0.f, ticks = 0.f;
            const auto rate = model.Get_AnimationTickPerSecond(i);
            if (result >= 0.f || !model.Get_AnimationProgress(i, position, ticks) || rate <= 0.f)
                throw std::runtime_error("Ambiguous or invalid source clip: " + name);
            result = ticks / rate * 1000.f;
        }
    if (result < 0.f) throw std::runtime_error("Source clip is missing: " + name);
    return result;
}
void Normalize(BONE_ANIMATION_KEY& key)
{
    for (float component : {key.position.x, key.position.y, key.position.z,
        key.rotation.x, key.rotation.y, key.rotation.z, key.rotation.w,
        key.scale.x, key.scale.y, key.scale.z})
        if (!std::isfinite(component) || std::abs(component) > 100000.f) throw std::runtime_error("Nonfinite or excessive bone key");
    if (key.scale.x <= .00001f || key.scale.y <= .00001f || key.scale.z <= .00001f)
        throw std::runtime_error("Bone scale must be positive");
    const auto rotation = XMLoadFloat4(&key.rotation);
    if (XMVectorGetX(XMVector4LengthSq(rotation)) < .000001f) throw std::runtime_error("Zero bone quaternion");
    XMStoreFloat4(&key.rotation, XMQuaternionNormalize(rotation));
}
BONE_ANIMATION_KEY Interpolate(const BONE_ANIMATION_TRACK& track, float time)
{
    const auto upper = std::upper_bound(track.keys.begin(), track.keys.end(), time,
        [](float t, const auto& key) { return t < key.timeMs; });
    if (upper == track.keys.begin()) return track.keys.front();
    if (upper == track.keys.end()) return track.keys.back();
    auto a = *(upper - 1), b = *upper;
    Normalize(a); Normalize(b);
    const auto weight = (time - a.timeMs) / static_cast<float>(b.timeMs - a.timeMs);
    BONE_ANIMATION_KEY result;
    XMStoreFloat3(&result.position, XMVectorLerp(XMLoadFloat3(&a.position), XMLoadFloat3(&b.position), weight));
    XMStoreFloat3(&result.scale, XMVectorLerp(XMLoadFloat3(&a.scale), XMLoadFloat3(&b.scale), weight));
    XMStoreFloat4(&result.rotation, XMQuaternionNormalize(XMQuaternionSlerp(XMLoadFloat4(&a.rotation), XMLoadFloat4(&b.rotation), weight)));
    return result;
}
void RequireNewClip(const CBoneAnimationDocument& document, const Engine::CModel& model, const std::string& name)
{
    if (!Stable(name) || !name.starts_with("authored.")) throw std::runtime_error("New clip needs a stable authored. name");
    if (std::any_of(document.clips.begin(), document.clips.end(), [&](const auto& clip) { return clip.name == name; }))
        throw std::runtime_error("Clip already exists; existing keys were preserved: " + name);
    for (uint32_t i = 0u; i < model.Get_NumAnimations(); ++i)
        if (name == model.Get_AnimationName(i)) throw std::runtime_error("Clip name is already installed: " + name);
}
float SourceDuration(const CBoneAnimationDocument& document, const Engine::CModel& model, const std::string& name)
{
    const auto found = std::find_if(document.clips.begin(), document.clips.end(), [&](const auto& clip) { return clip.name == name; });
    if (found != document.clips.end()) return static_cast<float>(found->durationMs);
    if (name.starts_with("authored.")) throw std::runtime_error("Source authored clip is missing: " + name);
    return Duration(model, name);
}
void AppendRestRelativePose(const Engine::CModel& model, BONE_ANIMATION_CLIP& clip,
    const std::vector<float4x4_t>& pose, uint32_t timeMs)
{
    const auto bones = model.Get_BoneNames();
    if (pose.size() != bones.size()) throw std::runtime_error("Source pose does not match the installed skeleton");
    if (clip.tracks.empty())
        for (const auto& bone : bones) { BONE_ANIMATION_TRACK track; track.bone = bone; clip.tracks.push_back(std::move(track)); }
    for (uint32_t bone = 0u; bone < bones.size(); ++bone)
    {
        matrix_t rest;
        if (!model.Get_BoneRestLocalMatrix(bone, rest)) throw std::runtime_error("Source bone has no rest transform");
        vector_t determinant;
        const auto inverseRest = XMMatrixInverse(&determinant, rest);
        if (!std::isfinite(XMVectorGetX(determinant)) || std::abs(XMVectorGetX(determinant)) < 1.e-12f)
            throw std::runtime_error("Source bone has a singular rest transform");
        const auto delta = XMLoadFloat4x4(&pose[bone]) * inverseRest;
        vector_t scale, rotation, position;
        if (!XMMatrixDecompose(&scale, &rotation, &position, delta)) throw std::runtime_error("Source pose cannot be represented by bone keys");
        BONE_ANIMATION_KEY key; key.timeMs = timeMs;
        XMStoreFloat3(&key.scale, scale); XMStoreFloat4(&key.rotation, rotation); XMStoreFloat3(&key.position, position);
        Normalize(key);
        const auto rebuilt = XMMatrixAffineTransformation(XMLoadFloat3(&key.scale), XMVectorZero(),
            XMLoadFloat4(&key.rotation), XMVectorSetW(XMLoadFloat3(&key.position), 1.f)) * rest;
        float4x4_t check; XMStoreFloat4x4(&check, rebuilt);
        for (size_t component = 0u; component < 16u; ++component)
        {
            const float expected = (&pose[bone]._11)[component], actual = (&check._11)[component];
            if (!std::isfinite(expected) || !std::isfinite(actual) || std::abs(expected - actual) > .001f * (std::max)(1.f, std::abs(expected)))
                throw std::runtime_error("Source pose contains a non-TRS rest-relative transform");
        }
        clip.tracks[bone].keys.push_back(key);
    }
}
void BlendLocalPose(std::vector<float4x4_t>& pose, const std::vector<float4x4_t>& target, float weight)
{
    if (pose.size() != target.size()) throw std::runtime_error("Loop poses have different skeletons");
    if (weight >= 1.f) { pose = target; return; }
    for (size_t bone = 0u; bone < pose.size(); ++bone)
    {
        vector_t aScale, aRotation, aPosition, bScale, bRotation, bPosition;
        if (!XMMatrixDecompose(&aScale, &aRotation, &aPosition, XMLoadFloat4x4(&pose[bone])) ||
            !XMMatrixDecompose(&bScale, &bRotation, &bPosition, XMLoadFloat4x4(&target[bone])))
            throw std::runtime_error("Loop pose cannot decompose");
        XMStoreFloat4x4(&pose[bone], XMMatrixAffineTransformation(XMVectorLerp(aScale, bScale, weight), XMVectorZero(),
            XMQuaternionNormalize(XMQuaternionSlerp(XMQuaternionNormalize(aRotation), XMQuaternionNormalize(bRotation), weight)),
            XMVectorLerp(aPosition, bPosition, weight)));
    }
}
}

std::filesystem::path CBoneAnimationDocument::Path(const std::string& asset)
{
    return Stable(asset) ? CProjectDataRoot::Resolve(std::filesystem::path("Animation/Authored") / asset / (asset + ".boneclips.json")) : std::filesystem::path{};
}
bool CBoneAnimationDocument::Parse(const std::string& text, std::string& status)
{
    try
    {
        Json root;
        if (!CDataJson::Parse(text, root, status)) return false;
        if (Text(root, "schema") != "lostark.authored-bone-clips" || Number(root, "formatVersion", 1, 1) != 1)
            throw std::runtime_error("Invalid authored animation schema");
        CBoneAnimationDocument candidate;
        candidate.asset = Text(root, "animationAssetId");
        const auto hash = Text(root, "skeletonHash");
        if (hash.size() != 16u || !std::all_of(hash.begin(), hash.end(), [](unsigned char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }))
            throw std::runtime_error("Skeleton hash must contain exactly 16 hexadecimal digits");
        size_t end = 0u;
        candidate.skeletonHash = std::stoull(hash, &end, 16);
        if (!Stable(candidate.asset) || end != hash.size() || !candidate.skeletonHash)
            throw std::runtime_error("Invalid animation/skeleton identity");
        for (const auto& value : Array(root, "clips", 32u))
        {
            BONE_ANIMATION_CLIP clip; clip.name = Text(value, "name"); clip.durationMs = Milliseconds(value, "durationMs", 1u);
            for (const auto& source : Array(value, "segments", 128u))
            {
                BONE_ANIMATION_SEGMENT segment;
                segment.id = Text(source, "id"); segment.sourceClip = Text(source, "sourceClip");
                segment.durationMs = Milliseconds(source, "durationMs", 1u);
                segment.sourceStartMs = Milliseconds(source, "sourceStartMs");
                segment.playRate = static_cast<float>(Number(source, "playRate", .01, 16.0));
                const auto& loop = Required(source, "loop");
                if (!loop.Is_Boolean()) throw std::runtime_error("Invalid segment loop");
                segment.loop = loop.Get_Boolean(); clip.segments.push_back(std::move(segment));
            }
            for (const auto& valueTrack : Array(value, "tracks", 512u))
            {
                BONE_ANIMATION_TRACK track; track.bone = Text(valueTrack, "bone");
                for (const auto& valueKey : Array(valueTrack, "keys", 4096u))
                {
                    BONE_ANIMATION_KEY key; key.timeMs = Milliseconds(valueKey, "timeMs");
                    Vector(valueKey, "position", &key.position.x, 3u);
                    Vector(valueKey, "rotation", &key.rotation.x, 4u);
                    Vector(valueKey, "scale", &key.scale.x, 3u);
                    Normalize(key); track.keys.push_back(key);
                }
                clip.tracks.push_back(std::move(track));
            }
            candidate.clips.push_back(std::move(clip));
        }
        asset = std::move(candidate.asset); skeletonHash = candidate.skeletonHash; clips = std::move(candidate.clips);
        return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}
bool CBoneAnimationDocument::Load(const std::string& owner, const Engine::CModel& model, std::string& status)
{
    const auto path = Path(owner);
    if (path.empty()) { status = "Invalid animation asset identity"; return false; }
    std::string bytes; bool exists = false;
    if (!Read(path, bytes, exists)) { status = "Cannot read authored bone clips: " + path.string(); return false; }
    CBoneAnimationDocument candidate;
    if (exists && !candidate.Parse(bytes, status)) return false;
    if (!exists) { candidate.asset = owner; candidate.skeletonHash = model.Get_SkeletonHash(); }
    if (candidate.asset != owner || candidate.skeletonHash != model.Get_SkeletonHash())
    { status = "Authored animation skeleton/asset mismatch"; return false; }
    std::vector<Engine::MODEL_ANIMATION_DATA> compiled;
    if (!candidate.Compile(model, compiled, status)) return false;
    candidate.m_Baseline = std::move(bytes); candidate.m_Existed = exists;
    *this = std::move(candidate); return true;
}
bool CBoneAnimationDocument::Sample(const Engine::CModel& model, const std::string& name,
    float timeMs, std::vector<float4x4_t>& output, std::string& status) const
{
    try
    {
        if (!std::isfinite(timeMs) || timeMs < 0.f) throw std::runtime_error("Invalid sample time");
        std::set<std::string> visiting;
        std::function<std::vector<float4x4_t>(const std::string&, float)> evaluate;
        evaluate = [&](const std::string& current, float time)
        {
            const auto found = std::find_if(clips.begin(), clips.end(), [&](const auto& c) { return c.name == current; });
            std::vector<float4x4_t> pose;
            if (found == clips.end())
            {
                if (current.starts_with("authored.")) throw std::runtime_error("Missing authored dependency: " + current);
                if (!model.Sample_AnimationLocalTransforms(current.c_str(), time * .001f, pose)) throw std::runtime_error("Source sampling failed: " + current);
                return pose;
            }
            if (!visiting.insert(current).second) throw std::runtime_error("Authored clip dependency cycle");
            time = (std::min)(time, static_cast<float>(found->durationMs));
            const auto names = model.Get_BoneNames(); pose.resize(names.size());
            if (found->segments.empty())
                for (uint32_t bone = 0u; bone < names.size(); ++bone)
                {
                    matrix_t matrix;
                    if (!model.Get_BoneRestLocalMatrix(bone, matrix)) throw std::runtime_error("Missing rest pose");
                    XMStoreFloat4x4(&pose[bone], matrix);
                }
            else
            {
                float start = 0.f;
                for (size_t index = 0u; index < found->segments.size(); ++index)
                {
                    const auto& segment = found->segments[index];
                    if (time < start + segment.durationMs || index + 1u == found->segments.size())
                    {
                        const auto dependency = std::find_if(clips.begin(), clips.end(), [&](const auto& c) { return c.name == segment.sourceClip; });
                        const float duration = dependency == clips.end() ? Duration(model, segment.sourceClip) : static_cast<float>(dependency->durationMs);
                        const float window = duration - segment.sourceStartMs;
                        if (window <= 0.f) throw std::runtime_error("Source start is outside clip: " + segment.sourceClip);
                        const float age = (std::max)(0.f, time - start) * segment.playRate;
                        const float sourceTime = segment.sourceStartMs + (segment.loop ? std::fmod(age, window) : (std::min)(age, window));
                        pose = evaluate(segment.sourceClip, sourceTime); break;
                    }
                    start += segment.durationMs;
                }
            }
            for (const auto& track : found->tracks)
            {
                const auto bone = model.Find_BoneIndex(track.bone.c_str());
                if (bone < 0 || track.keys.empty()) throw std::runtime_error("Missing keyed bone: " + track.bone);
                auto key = Interpolate(track, time); Normalize(key);
                const auto delta = XMMatrixAffineTransformation(XMLoadFloat3(&key.scale), XMVectorZero(),
                    XMLoadFloat4(&key.rotation), XMVectorSetW(XMLoadFloat3(&key.position), 1.f));
                XMStoreFloat4x4(&pose[bone], delta * XMLoadFloat4x4(&pose[bone]));
            }
            visiting.erase(current); return pose;
        };
        auto staged = evaluate(name, timeMs);
        for (const auto& matrix : staged)
            for (size_t component = 0u; component < 16u; ++component)
                if (!std::isfinite((&matrix._11)[component])) throw std::runtime_error("Authored pose contains a nonfinite matrix");
        output = std::move(staged); return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}
bool CBoneAnimationDocument::Compile(const Engine::CModel& model,
    std::vector<Engine::MODEL_ANIMATION_DATA>& output, std::string& status) const
{
    try
    {
        const auto bones = model.Get_BoneNames();
        if (!Stable(asset) || !skeletonHash || skeletonHash != model.Get_SkeletonHash() || bones.empty() || bones.size() > 512u || clips.size() > 32u)
            throw std::runtime_error("Authored animation owner, skeleton or count is invalid");
        std::set<std::string> ids;
        for (const auto& clip : clips)
        {
            if (!Stable(clip.name) || !clip.name.starts_with("authored.") || !ids.insert(clip.name).second ||
                !clip.durationMs || clip.durationMs > 60000u || clip.segments.size() > 128u || clip.tracks.size() > bones.size())
                throw std::runtime_error("Invalid or duplicate authored clip");
            std::set<std::string> segments, keyedBones;
            uint64_t duration = 0u;
            for (const auto& segment : clip.segments)
            {
                if (!Stable(segment.id) || !segments.insert(segment.id).second || segment.sourceClip.empty() || segment.sourceClip.size() >= 128u ||
                    !segment.durationMs || segment.durationMs > 60000u || segment.sourceStartMs > 60000u || !std::isfinite(segment.playRate) || segment.playRate < .01f || segment.playRate > 16.f)
                    throw std::runtime_error("Invalid or duplicate source segment");
                duration += segment.durationMs;
            }
            if (!clip.segments.empty() && duration != clip.durationMs) throw std::runtime_error("Stage durations must cover the clip duration exactly");
            for (const auto& track : clip.tracks)
            {
                if (!keyedBones.insert(track.bone).second || model.Find_BoneIndex(track.bone.c_str()) < 0 || track.keys.empty() || track.keys.size() > 4096u)
                    throw std::runtime_error("Invalid or duplicate bone track");
                uint32_t previous = 0u; bool first = true;
                for (auto key : track.keys)
                {
                    if (key.timeMs > clip.durationMs || (!first && key.timeMs <= previous)) throw std::runtime_error("Bone key times must increase inside the clip");
                    Normalize(key); previous = key.timeMs; first = false;
                }
            }
        }
        // Visit every dependency, including a source segment never sampled at a grid point.
        std::set<std::string> visiting, complete;
        std::function<void(const BONE_ANIMATION_CLIP&)> visit = [&](const auto& clip)
        {
            if (complete.contains(clip.name)) return;
            if (!visiting.insert(clip.name).second) throw std::runtime_error("Authored animation dependency cycle");
            for (const auto& segment : clip.segments)
            {
                const auto source = std::find_if(clips.begin(), clips.end(), [&](const auto& c) { return c.name == segment.sourceClip; });
                if (source != clips.end()) { visit(*source); if (segment.sourceStartMs >= source->durationMs) throw std::runtime_error("Source start exceeds authored clip"); }
                else if (segment.sourceClip.starts_with("authored.") || segment.sourceStartMs >= Duration(model, segment.sourceClip))
                    throw std::runtime_error("Source clip reference is unavailable");
            }
            visiting.erase(clip.name); complete.insert(clip.name);
        };
        for (const auto& clip : clips) visit(clip);
        std::vector<Engine::MODEL_ANIMATION_DATA> staged;
        size_t sampleCount = 0u;
        for (const auto& clip : clips)
        {
            std::set<uint32_t> times{0u, clip.durationMs};
            for (uint32_t frame = 1u; frame * 1000u / 30u < clip.durationMs; ++frame) times.insert(frame * 1000u / 30u);
            uint32_t boundary = 0u;
            for (const auto& segment : clip.segments) { boundary += segment.durationMs; times.insert(boundary); if (boundary) times.insert(boundary - 1u); }
            for (const auto& track : clip.tracks) for (const auto& key : track.keys) times.insert(key.timeMs);
            sampleCount += times.size() * bones.size();
            if (sampleCount > 2000000u) throw std::runtime_error("Authored clips exceed the two-million sampled-bone limit");
            Engine::MODEL_ANIMATION_DATA compiled;
            compiled.name = clip.name; compiled.skeletonHash = skeletonHash;
            compiled.durationTicks = clip.durationMs * .001f * Engine::CAnimation::COOKED_TICK_RATE;
            compiled.ticksPerSecond = Engine::CAnimation::COOKED_TICK_RATE;
            compiled.channels.resize(bones.size());
            for (uint32_t bone = 0u; bone < bones.size(); ++bone) compiled.channels[bone].resolvedBoneIndex = static_cast<int32_t>(bone);
            for (const auto time : times)
            {
                std::vector<float4x4_t> pose;
                if (!Sample(model, clip.name, static_cast<float>(time), pose, status)) return false;
                for (size_t bone = 0u; bone < bones.size(); ++bone)
                {
                    vector_t scale, rotation, translation;
                    if (!XMMatrixDecompose(&scale, &rotation, &translation, XMLoadFloat4x4(&pose[bone]))) throw std::runtime_error("Authored bone matrix cannot decompose");
                    auto& channel = compiled.channels[bone];
                    const auto ticks = time * .001f * Engine::CAnimation::COOKED_TICK_RATE;
                    Engine::MODEL_VECTOR_KEY_DATA p{ticks}, s{ticks}; Engine::MODEL_QUAT_KEY_DATA r{ticks};
                    XMStoreFloat3(&p.value, translation); XMStoreFloat3(&s.value, scale); XMStoreFloat4(&r.value, XMQuaternionNormalize(rotation));
                    channel.positionKeys.push_back(p); channel.rotationKeys.push_back(r); channel.scaleKeys.push_back(s);
                }
            }
            staged.push_back(std::move(compiled));
        }
        output = std::move(staged); return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}
bool CBoneAnimationDocument::Install(Engine::CModel& model, std::string& status) const
{
    std::vector<Engine::MODEL_ANIMATION_DATA> compiled;
    return Compile(model, compiled, status) && model.Install_AuthoredAnimations(compiled, status);
}
bool CBoneAnimationDocument::Create_FromSourceRange(const Engine::CModel& model, const std::string& name,
    const std::string& source, uint32_t sourceStartMs, uint32_t sourceEndMs, float playRate, std::string& status)
{
    try
    {
        RequireNewClip(*this, model, name);
        const float sourceDuration = SourceDuration(*this, model, source);
        if (sourceStartMs >= sourceEndMs || sourceEndMs > 60000u || sourceEndMs > sourceDuration + .001f ||
            !std::isfinite(playRate) || playRate < .01f || playRate > 16.f)
            throw std::runtime_error("Select an increasing source range inside the clip and a rate from 0.01 to 16");
        const double span = static_cast<double>(sourceEndMs - sourceStartMs);
        const double roundedDuration = std::round(span / playRate);
        if (roundedDuration < 1.0 || roundedDuration > 60000.0) throw std::runtime_error("New clip duration must be 1 to 60000 milliseconds");
        BONE_ANIMATION_CLIP clip; clip.name = name; clip.durationMs = static_cast<uint32_t>(roundedDuration);
        BONE_ANIMATION_SEGMENT segment; segment.id = "source.range"; segment.sourceClip = source;
        segment.sourceStartMs = sourceStartMs; segment.durationMs = clip.durationMs;
        // Integer millisecond storage slightly adjusts rate so the requested end pose remains exact.
        segment.playRate = static_cast<float>(span / roundedDuration); segment.loop = false;
        clip.segments.push_back(std::move(segment));
        auto candidate = *this; candidate.clips.push_back(std::move(clip));
        std::vector<Engine::MODEL_ANIMATION_DATA> compiled;
        if (!candidate.Compile(model, compiled, status)) return false;
        *this = std::move(candidate); status = "Source range added to the draft; save to keep it"; return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}
bool CBoneAnimationDocument::Create_HeldPose(const Engine::CModel& model, const std::string& name,
    const std::string& source, uint32_t sourceTimeMs, uint32_t durationMs, std::string& status)
{
    try
    {
        RequireNewClip(*this, model, name);
        if (!durationMs || durationMs > 60000u || sourceTimeMs > 60000u || sourceTimeMs > SourceDuration(*this, model, source) + .001f)
            throw std::runtime_error("Hold time must be inside the source clip and duration must be 1 to 60000 milliseconds");
        std::vector<float4x4_t> pose;
        if (!Sample(model, source, static_cast<float>(sourceTimeMs), pose, status)) return false;
        BONE_ANIMATION_CLIP clip; clip.name = name; clip.durationMs = durationMs;
        AppendRestRelativePose(model, clip, pose, 0u); AppendRestRelativePose(model, clip, pose, durationMs);
        auto candidate = *this; candidate.clips.push_back(std::move(clip));
        std::vector<Engine::MODEL_ANIMATION_DATA> compiled;
        if (!candidate.Compile(model, compiled, status)) return false;
        *this = std::move(candidate); status = "Held pose added to the draft; save to keep it"; return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}
bool CBoneAnimationDocument::Create_AncientSeaFlightStudies(const Engine::CModel& model, std::string& status)
{
    try
    {
        if (asset != "Vehicle_9523" || skeletonHash != 0x4bd78391eec5de7aULL ||
            model.Get_SkeletonHash() != skeletonHash || model.Get_BoneNames().size() != 91u)
            throw std::runtime_error("Dragon studies require the installed Ancient Sea skeleton");
        RequireNewClip(*this, model, "authored.dragon.glide"); RequireNewClip(*this, model, "authored.dragon.ascent");
        if (std::abs(SourceDuration(*this, model, "npc_sk_look") - 160000.f / 30.f) > .01f)
            throw std::runtime_error("Ancient Sea flight source timing changed; studies were not added");
        const auto root = model.Find_BoneIndex("bip001"); matrix_t rest;
        if (root < 0 || !model.Get_BoneRestLocalMatrix(root, rest)) throw std::runtime_error("Ancient Sea vertical root bone is missing");
        const float restZ = XMVectorGetZ(rest.r[3]);
        const auto sample = [&](float sourceTime, std::vector<float4x4_t>& pose)
        {
            if (!Sample(model, "npc_sk_look", sourceTime, pose, status)) throw std::runtime_error(status);
            // This imported skeleton maps bip001 local -Z to up; Server owns flight height.
            pose[root]._43 = restZ;
        };
        BONE_ANIMATION_CLIP glide; glide.name = "authored.dragon.glide"; glide.durationMs = 2000u;
        BONE_ANIMATION_CLIP ascent; ascent.name = "authored.dragon.ascent"; ascent.durationMs = 967u;
        std::vector<float4x4_t> held, first; sample(2500.f, held); sample(1800.f, first);
        for (auto* clip : {&glide, &ascent})
        {
            std::set<uint32_t> times{0u, clip->durationMs};
            for (uint32_t frame = 1u; frame * 1000u / 30u < clip->durationMs; ++frame) times.insert(frame * 1000u / 30u);
            if (clip == &ascent) times.insert(ascent.durationMs - 120u);
            for (const auto time : times)
            {
                auto pose = held;
                if (clip == &ascent)
                {
                    sample(1800.f + (83000.f / 30.f - 1800.f) * (static_cast<float>(time) / ascent.durationMs), pose);
                    if (time > ascent.durationMs - 120u) BlendLocalPose(pose, first, (time - (ascent.durationMs - 120u)) / 120.f);
                }
                AppendRestRelativePose(model, *clip, pose, time);
            }
        }
        auto candidate = *this; candidate.clips.push_back(std::move(glide)); candidate.clips.push_back(std::move(ascent));
        std::vector<Engine::MODEL_ANIMATION_DATA> compiled;
        if (!candidate.Compile(model, compiled, status)) return false;
        *this = std::move(candidate); status = "Glide and ascent studies added to the draft; visual review and save are pending"; return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}
bool CBoneAnimationDocument::Retime_Clip(const Engine::CModel& model, const std::string& name,
    uint32_t durationMs, std::string& status)
{
    try
    {
        if (!durationMs || durationMs > 60000u) throw std::runtime_error("Clip duration must be 1 to 60000 milliseconds");
        const auto found = std::find_if(clips.begin(), clips.end(), [&](const auto& clip) { return clip.name == name; });
        if (found == clips.end()) throw std::runtime_error("The clip to retime is missing");
        if (!found->segments.empty()) throw std::runtime_error("Source segment clips use source range/segment trim; retime is for held or baked bone clips");
        if (!found->durationMs || found->durationMs > 60000u) throw std::runtime_error("Existing clip duration is invalid");
        auto candidate = *this;
        auto& retimed = candidate.clips[static_cast<size_t>(found - clips.begin())];
        retimed.durationMs = durationMs;
        for (size_t trackIndex = 0u; trackIndex < found->tracks.size(); ++trackIndex)
        {
            const auto& source = found->tracks[trackIndex];
            if (source.keys.empty()) throw std::runtime_error("Cannot retime an empty bone track");
            std::set<uint32_t> times{0u, durationMs};
            for (uint32_t frame = 1u; frame * 1000u / 30u < durationMs; ++frame) times.insert(frame * 1000u / 30u);
            uint32_t previous = 0u; bool first = true;
            for (const auto& key : source.keys)
            {
                if (key.timeMs > found->durationMs || (!first && key.timeMs <= previous))
                    throw std::runtime_error("Cannot retime invalid bone key times");
                // Preserve authored breakpoints as closely as the millisecond schema permits.
                const auto mapped = (static_cast<uint64_t>(key.timeMs) * durationMs + found->durationMs / 2u) / found->durationMs;
                times.insert(static_cast<uint32_t>(mapped)); previous = key.timeMs; first = false;
            }
            auto& keys = retimed.tracks[trackIndex].keys; keys.clear(); keys.reserve(times.size());
            for (const auto time : times)
            {
                auto key = time == 0u ? source.keys.front() : time == durationMs ? source.keys.back() :
                    Interpolate(source, static_cast<float>(static_cast<double>(time) * found->durationMs / durationMs));
                key.timeMs = time; keys.push_back(key);
            }
        }
        std::vector<Engine::MODEL_ANIMATION_DATA> compiled;
        if (!candidate.Compile(model, compiled, status)) return false;
        *this = std::move(candidate); status = "Bone clip retimed in the draft; save to keep it"; return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}
bool CBoneAnimationDocument::Load_IntoModel(Engine::CModel& model, const std::string& owner, std::string& status)
{
    CBoneAnimationDocument document;
    return document.Load(owner, model, status) && document.Install(model, status);
}
bool CBoneAnimationDocument::Load_PublishedValtan(Engine::CModel& model, std::string& status)
{
    const auto path = CProjectDataRoot::Resolve("Valtan/Published/Valtan.boneclips.json");
    std::string bytes; bool exists = false;
    if (!Read(path, bytes, exists)) { status = "Cannot read published Valtan bone clips"; return false; }
    if (!exists) return true; // Existing native-only products need no optional document.
    CBoneAnimationDocument candidate;
    if (!candidate.Parse(bytes, status) || candidate.asset != "Valtan")
    { if (status.empty()) status = "Published bone clip owner mismatch"; return false; }
    return candidate.Install(model, status);
}
std::string CBoneAnimationDocument::Serialize() const
{
    std::ostringstream out; out.imbue(std::locale::classic()); out << std::setprecision(9);
    const auto quote = [&](const std::string& value) { out << '"' << CDataJson::Escape(value) << '"'; };
    const auto vector = [&](const float* value, size_t count) { out << '['; for (size_t i = 0u; i < count; ++i) out << (i ? ", " : "") << value[i]; out << ']'; };
    out << "{\n  \"schema\": \"lostark.authored-bone-clips\",\n  \"formatVersion\": 1,\n  \"animationAssetId\": "; quote(asset);
    out << ",\n  \"skeletonHash\": \"" << std::hex << std::setw(16) << std::setfill('0') << skeletonHash << std::dec << std::setfill(' ') << "\",\n  \"clips\": [";
    for (size_t i = 0u; i < clips.size(); ++i)
    {
        const auto& clip = clips[i]; out << (i ? ",\n" : "\n") << "    {\"name\": "; quote(clip.name);
        out << ", \"durationMs\": " << clip.durationMs << ", \"segments\": [";
        for (size_t j = 0u; j < clip.segments.size(); ++j)
        {
            const auto& segment = clip.segments[j]; out << (j ? ", " : "") << "{\"id\": "; quote(segment.id);
            out << ", \"sourceClip\": "; quote(segment.sourceClip);
            out << ", \"durationMs\": " << segment.durationMs << ", \"sourceStartMs\": " << segment.sourceStartMs
                << ", \"playRate\": " << segment.playRate << ", \"loop\": " << (segment.loop ? "true" : "false") << '}';
        }
        out << "], \"tracks\": [";
        for (size_t j = 0u; j < clip.tracks.size(); ++j)
        {
            const auto& track = clip.tracks[j]; out << (j ? ", " : "") << "{\"bone\": "; quote(track.bone); out << ", \"keys\": [";
            for (size_t k = 0u; k < track.keys.size(); ++k)
            {
                auto key = track.keys[k]; Normalize(key);
                out << (k ? ", " : "") << "{\"timeMs\": " << key.timeMs << ", \"position\": "; vector(&key.position.x, 3u);
                out << ", \"rotation\": "; vector(&key.rotation.x, 4u); out << ", \"scale\": "; vector(&key.scale.x, 3u); out << '}';
            }
            out << "]}";
        }
        out << "]}";
    }
    out << "\n  ]\n}\n"; return out.str();
}
bool CBoneAnimationDocument::Save(Engine::CModel& model, std::string& status)
{
    std::vector<Engine::MODEL_ANIMATION_DATA> compiled;
    if (!Compile(model, compiled, status)) return false;
    const auto path = Path(asset);
    const auto text = Serialize(); CBoneAnimationDocument reparsed;
    if (text.size() > 8u * 1024u * 1024u || !reparsed.Parse(text, status) || !reparsed.Compile(model, compiled, status))
    { status = "Authored clip serialization did not round-trip; draft preserved"; return false; }
    std::error_code error; std::filesystem::create_directories(path.parent_path(), error);
    if (error) { status = "Cannot create authored animation directory"; return false; }
    const auto lockPath = path.wstring() + L".writer.lock";
    const HANDLE lock = CreateFileW(lockPath.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (lock == INVALID_HANDLE_VALUE) { status = "Another writer owns this animation document; draft preserved"; return false; }
    struct Lock { HANDLE handle; ~Lock() { CloseHandle(handle); } } guard{lock};
    std::string current; bool exists = false;
    if (!Read(path, current, exists) || exists != m_Existed || current != m_Baseline)
    { status = "Bone clips changed externally; saved file and draft preserved"; return false; }
    const auto temporary = path.wstring() + L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(GetTickCount64());
    FILE* file = nullptr;
    if (_wfopen_s(&file, temporary.c_str(), L"wb") || !file) { status = "Cannot stage bone animation file"; return false; }
    const bool wrote = fwrite(text.data(), 1u, text.size(), file) == text.size();
    const bool flushed = fflush(file) == 0 && _commit(_fileno(file)) == 0;
    const bool closed = fclose(file) == 0;
    const auto cleanup = [&]() { std::filesystem::remove(temporary, error); };
    std::string staged; bool stagedExists = false;
    if (!wrote || !flushed || !closed || !Read(temporary, staged, stagedExists) || staged != text ||
        !Read(path, current, exists) || exists != m_Existed || current != m_Baseline)
    { cleanup(); status = "Bone animation write/freshness verification failed; previous file preserved"; return false; }
    const auto backup = path.wstring() + L".previous";
    if (exists && !CopyFileW(path.c_str(), backup.c_str(), FALSE))
    { cleanup(); status = "Cannot preserve previous bone animation bytes"; return false; }
    if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    { cleanup(); status = "Atomic animation replacement failed; draft preserved"; return false; }
    // Compilation succeeded before replacement. Install itself stages all clips
    // and commits once; a native-name collision cannot partially mutate the model.
    if (!model.Install_AuthoredAnimations(compiled, status))
    {
        std::string observed; bool present = false;
        if (Read(path, observed, present) && present && observed == text)
        {
            if (m_Existed) MoveFileExW(backup.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
            else std::filesystem::remove(path, error);
        }
        return false;
    }
    m_Baseline = text; m_Existed = true;
    status = "Saved and installed authored clips; native animation and Server gameplay are unchanged"; return true;
}
}
