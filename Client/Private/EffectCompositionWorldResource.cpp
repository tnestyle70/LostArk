#include "EffectCompositionWorldResource.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <set>

namespace
{
using namespace Client;
constexpr std::uint32_t MAX_MS = CWorldSequenceDocument::MAX_DURATION_MS;

void Note(std::string& out, const std::string& text)
{ if (!out.empty()) out += " "; out += text; }

bool Text(const DATA_JSON_VALUE& value, const char* key, std::string& out)
{
    const auto* item = value.Find(key);
    if (!item || !item->Is_String()) return false;
    out = item->Get_String(); return true;
}

bool Number(const DATA_JSON_VALUE* value, float& out)
{
    if (!value || !value->Is_Number() || !std::isfinite(value->Get_Number()) || std::abs(value->Get_Number()) > 1000000.0) return false;
    out = static_cast<float>(value->Get_Number()); return true;
}

bool UInt(const DATA_JSON_VALUE& value, const char* key, std::uint32_t& out,
    std::uint32_t max = (std::numeric_limits<std::uint32_t>::max)())
{
    const auto* item = value.Find(key);
    if (!item || !item->Is_Number()) return false;
    const double number = item->Get_Number();
    if (!std::isfinite(number) || number < 0.0 || number > max || std::floor(number) != number) return false;
    out = static_cast<std::uint32_t>(number); return true;
}

bool Vector(const DATA_JSON_VALUE* value, float* out, const std::size_t count)
{
    if (!value || !value->Is_Array() || value->Get_Array().size() != count) return false;
    for (std::size_t i = 0; i < count; ++i)
        if (!Number(&value->Get_Array()[i], out[i])) return false;
    return true;
}

bool AssetPath(const std::string& value)
{
    if (value.empty() || value.find(':') != std::string::npos || value.front() == '/' || value.front() == '\\') return false;
    std::string normalized = value;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    std::size_t begin = 0;
    while (begin <= normalized.size())
    {
        const auto end = normalized.find('/', begin);
        const auto part = normalized.substr(begin, end == std::string::npos ? end : end - begin);
        if (part.empty() || part == "." || part == "..") return false;
        if (end == std::string::npos) break;
        begin = end + 1u;
    }
    return true;
}

void ReadTemplate(const DATA_JSON_VALUE& source, EFFECT_COMPOSITION_WORLD_MOTION& out)
{
    Text(source, "displayName", out.displayName);
    if (!UInt(source, "durationMs", out.durationMs, MAX_MS) || !out.durationMs)
        Note(out.error, "Invalid Motion duration.");
    if (const auto* motion = source.Find("objectMotion"))
    {
        auto& value = out.objectMotion;
        if (!Vector(motion->Find("velocity"), &value.velocity.x, 3) ||
            !Vector(motion->Find("acceleration"), &value.acceleration.x, 3) ||
            !Vector(motion->Find("angularVelocityDegrees"), &value.angularVelocityDegrees.x, 3) ||
            !Vector(motion->Find("revolutionDegreesPerSecond"), &value.revolutionDegreesPerSecond.x, 3) ||
            !Vector(motion->Find("revolutionOffset"), &value.revolutionOffset.x, 3) ||
            !UInt(*motion, "count", value.count, 128u) || !value.count ||
            !UInt(*motion, "intervalMs", value.intervalMs, MAX_MS) || !UInt(*motion, "seed", value.seed) ||
            !Number(motion->Find("spreadDegrees"), value.spreadDegrees) || value.spreadDegrees < 0.f || value.spreadDegrees > 180.f ||
            std::uint64_t(value.count - 1u) * value.intervalMs >= out.durationMs)
            Note(out.error, "Invalid saved physics/count/interval.");
    }
    const auto* tracks = source.Find("tracks");
    if (!tracks || !tracks->Is_Array() || tracks->Get_Array().size() > CWorldSequenceDocument::MAX_TRACK_COUNT)
        Note(out.error, "Invalid transform tracks.");
    else for (const auto& value : tracks->Get_Array())
    {
        std::string slot;
        if (!Text(value, "slotId", slot) || slot != out.slotId) continue;
        auto& track = out.tracks.emplace_back(); track.slotId = slot;
        const auto* keys = value.Find("keys");
        if (!keys || !keys->Is_Array() || keys->Get_Array().size() > CWorldSequenceDocument::MAX_KEY_COUNT)
        { Note(out.error, "Invalid transform keys."); continue; }
        for (const auto& keyValue : keys->Get_Array())
        {
            auto& key = track.keys.emplace_back();
            const auto* visible = keyValue.Find("visible");
            if (!UInt(keyValue, "timeMs", key.timeMs, MAX_MS) ||
                !Vector(keyValue.Find("positionOffset"), &key.positionOffset.x, 3) ||
                !Vector(keyValue.Find("rotationQuaternion"), &key.rotationQuaternion.x, 4) ||
                !Vector(keyValue.Find("scaleMultiplier"), &key.scaleMultiplier.x, 3) || !visible || !visible->Is_Boolean())
                Note(out.error, "Invalid transform key value.");
            else key.visible = visible->Get_Boolean();
        }
    }
    if (const auto* animations = source.Find("animationTracks"))
    {
        if (!animations->Is_Array() || animations->Get_Array().size() > CWorldSequenceDocument::MAX_TRACK_COUNT)
            Note(out.error, "Invalid animation tracks.");
        else for (const auto& value : animations->Get_Array())
        {
            std::string slot;
            if (!Text(value, "slotId", slot) || slot != out.slotId) continue;
            auto& clip = out.animationTracks.emplace_back(); clip.slotId = slot;
            Text(value, "displayName", clip.displayName);
            const auto* loop = value.Find("loop");
            const auto* hold = value.Find("holdLastFrame");
            if (!Text(value, "clipName", clip.clipName) || clip.clipName.empty() ||
                !UInt(value, "startMs", clip.startMs, MAX_MS) || !Number(value.Find("playbackRate"), clip.playbackRate) ||
                clip.playbackRate <= 0.f || !loop || !loop->Is_Boolean() || !hold || !hold->Is_Boolean())
                Note(out.error, "Invalid animation clip.");
            else { clip.loop = loop->Get_Boolean(); clip.holdLastFrame = hold->Get_Boolean(); }
        }
    }
    std::stable_sort(out.animationTracks.begin(), out.animationTracks.end(),
        [](const auto& a, const auto& b) { return a.startMs < b.startMs; });
}

bool Same3(const float3_t& a, const float3_t& b)
{ return std::abs(a.x-b.x) < .000001f && std::abs(a.y-b.y) < .000001f && std::abs(a.z-b.z) < .000001f; }

bool Zero(const float3_t& value) { return Same3(value, {}); }

float3_t Euler(const float4_t& q)
{
    float4x4_t matrix;
    XMStoreFloat4x4(&matrix, XMMatrixRotationQuaternion(XMQuaternionNormalize(XMLoadFloat4(&q))));
    const float pitch = std::asin((std::clamp)(-matrix._32, -1.f, 1.f));
    const float cosine = std::cos(pitch);
    const float yaw = std::abs(cosine) > .00001f ? std::atan2(matrix._31, matrix._33) : std::atan2(-matrix._13, matrix._11);
    const float roll = std::abs(cosine) > .00001f ? std::atan2(matrix._12, matrix._22) : 0.f;
    return {XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll)};
}
}

bool Client::Read_EffectCompositionWorldResources(const std::string& areaId,
    std::vector<EFFECT_COMPOSITION_WORLD_RESOURCE>& resources,
    std::uint32_t& sourceRevision, std::string& status)
{
    if (!CWorldSequenceDocument::Is_ValidStableId(areaId) || areaId.find_first_of("/\\:") != std::string::npos)
    { status = "Invalid World Object Area ID; previous rows preserved."; return false; }
    const auto path = CProjectDataRoot::Resolve(std::filesystem::path("Maps/Authoring") / areaId / (areaId + ".worldsequences.json"));
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    if (error || !size || size > 16u * 1024u * 1024u)
    { status = "World Object source is unavailable; previous rows preserved."; return false; }
    std::ifstream input(path, std::ios::binary);
    std::string bytes(static_cast<std::size_t>(size), '\0');
    if (!input.read(bytes.data(), static_cast<std::streamsize>(bytes.size())))
    { status = "World Object source read failed; previous rows preserved."; return false; }
    DATA_JSON_VALUE root;
    if (!CDataJson::Parse(bytes, root, status)) return false;
    std::string schema, actualArea;
    std::uint32_t version = 0, revision = 0;
    const auto* objects = root.Find("objectResources");
    const auto* templates = root.Find("templates");
    const auto* instances = root.Find("instances");
    if (!Text(root, "schema", schema) || schema != "lostark.world-sequences" ||
        !Text(root, "areaId", actualArea) || actualArea != areaId || !UInt(root, "formatVersion", version, 3u) || version != 3u ||
        !UInt(root, "revision", revision) || !revision || !objects || !objects->Is_Array() || objects->Get_Array().size() > 4096u ||
        !templates || !templates->Is_Array() || templates->Get_Array().size() > CWorldSequenceDocument::MAX_TEMPLATE_COUNT ||
        !instances || !instances->Is_Array() || instances->Get_Array().size() > CWorldSequenceDocument::MAX_INSTANCE_COUNT)
    { status = "World Object source header is invalid; previous rows preserved."; return false; }

    std::vector<EFFECT_COMPOSITION_WORLD_RESOURCE> staged;
    std::map<std::string, std::size_t> objectIndex;
    for (const auto& value : objects->Get_Array())
    {
        auto& row = staged.emplace_back(); auto& object = row.resource;
        Text(value, "objectId", object.objectId); Text(value, "displayName", object.displayName);
        Text(value, "modelAssetId", object.modelAssetId); Text(value, "diffuseTextureAssetId", object.diffuseTextureAssetId);
        Text(value, "sequenceInstanceId", object.sequenceInstanceId); Text(value, "anchorKind", object.anchorKind);
        const auto* animated = value.Find("animated");
        if (!CWorldSequenceDocument::Is_ValidStableId(object.objectId) ||
            !Number(value.Find("modelPreScale"), object.modelPreScale) || object.modelPreScale <= 0.f ||
            !Vector(value.Find("scale"), &object.scale.x, 3) || object.scale.x <= 0.f || object.scale.y <= 0.f || object.scale.z <= 0.f ||
            !animated || !animated->Is_Boolean()) row.error = "Invalid saved Object identity, scale or animation flag.";
        else object.animated = animated->Get_Boolean();
        if ((!object.modelAssetId.empty() && !AssetPath(object.modelAssetId)) ||
            (!object.diffuseTextureAssetId.empty() && !AssetPath(object.diffuseTextureAssetId))) Note(row.error, "Invalid Resources-relative path.");
        if (!objectIndex.emplace(object.objectId, staged.size()-1u).second)
        { Note(row.error, "Duplicate saved Object ID."); Note(staged[objectIndex.at(object.objectId)].error, "Duplicate saved Object ID."); }
    }
    std::map<std::string, const DATA_JSON_VALUE*> templateIndex;
    std::set<std::string> duplicateTemplates, instanceIds;
    for (const auto& value : templates->Get_Array())
    {
        std::string id; Text(value, "sequenceId", id);
        if (!templateIndex.emplace(id, &value).second) duplicateTemplates.insert(id);
    }
    for (const auto& value : instances->Get_Array())
    {
        std::string instanceId; Text(value, "instanceId", instanceId);
        const bool duplicate = !instanceIds.insert(instanceId).second;
        const auto* bindings = value.Find("bindings");
        if (!bindings || !bindings->Is_Array()) continue;
        for (const auto& binding : bindings->Get_Array())
        {
            std::string kind, id, slot;
            Text(binding, "targetKind", kind);
            if (kind != "OBJECT_RESOURCE") continue;
            Text(binding, "targetId", id); Text(binding, "slotId", slot);
            auto found = objectIndex.find(id);
            if (found == objectIndex.end())
            {
                auto& missing = staged.emplace_back();
                missing.resource.objectId = id; missing.resource.displayName = id;
                missing.error = "Instance refers to a missing Object resource.";
                found = objectIndex.emplace(id, staged.size()-1u).first;
            }
            auto& motion = staged[found->second].motions.emplace_back();
            motion.objectId = id; motion.instanceId = instanceId; motion.slotId = slot;
            Text(value, "templateId", motion.templateId); Text(value, "anchorKind", motion.anchorKind);
            Text(value, "nextMotionId", motion.nextMotionId);
            if (const auto* position = value.Find("position"); position && !Vector(position, &motion.position.x, 3))
                Note(motion.error, "Invalid saved placement position.");
            std::string end;
            if (Text(value, "motionEnd", end) && !CWorldSequenceDocument::Try_ParseMotionEnd(end, motion.motionEnd))
                Note(motion.error, "Unknown Motion completion policy.");
            if (duplicate || !CWorldSequenceDocument::Is_ValidStableId(instanceId) || !CWorldSequenceDocument::Is_ValidStableId(slot) ||
                !UInt(value, "startDelayMs", motion.startDelayMs, MAX_MS) || !Number(value.Find("playbackSpeed"), motion.playbackSpeed) ||
                motion.playbackSpeed < .05f || motion.playbackSpeed > 16.f)
                Note(motion.error, "Invalid Motion identity, delay or playback speed.");
            const auto source = templateIndex.find(motion.templateId);
            if (source == templateIndex.end() || duplicateTemplates.contains(motion.templateId))
                Note(motion.error, "Missing or duplicate Motion template: " + motion.templateId);
            else ReadTemplate(*source->second, motion);
            if (motion.displayName.empty()) motion.displayName = motion.instanceId;
        }
    }
    resources = std::move(staged); sourceRevision = revision;
    std::size_t motions = 0u, errors = 0u;
    for (const auto& resource : resources)
    {
        motions += resource.motions.size(); errors += !resource.error.empty();
        for (const auto& motion : resource.motions) errors += !motion.error.empty();
    }
    status = "Saved World Objects rev " + std::to_string(revision) + ": " + std::to_string(resources.size()) +
        " objects / " + std::to_string(motions) + " motions / " + std::to_string(errors) + " isolated row errors. Source remains read-only.";
    return true;
}

bool Client::Apply_WorldMotionToEffect(const EFFECT_COMPOSITION_WORLD_RESOURCE& source,
    const EFFECT_COMPOSITION_WORLD_MOTION& motion, EFFECT_V2_DOCUMENT& effect,
    EFFECT_V2_GROUP_CHILD& child, std::string& status)
{
    const auto& object = source.resource;
    if (!source.error.empty() || !motion.error.empty() || object.objectId != motion.objectId ||
        !AssetPath(object.modelAssetId) || !motion.durationMs || motion.durationMs > MAX_MS ||
        !std::isfinite(motion.playbackSpeed) || motion.playbackSpeed <= 0.f || motion.playbackSpeed > 16.f ||
        !motion.objectMotion.count || motion.objectMotion.count > 128u ||
        std::uint64_t(motion.objectMotion.count - 1u)*motion.objectMotion.intervalMs >= motion.durationMs)
    { status = "Object Motion not applied; Effect draft preserved. " + source.error + " " + motion.error;
      if (object.modelAssetId.empty()) status += " This resource is a placed sequence alias without a model."; return false; }
    auto staged = effect; auto stagedChild = child;
    auto& params = staged.Desc.Params;
    const auto& physics = motion.objectMotion;
    const float life = float(motion.durationMs) * .001f;
    staged.Desc.strMeshAssetId = object.modelAssetId;
    staged.Desc.TextureAssetIds[static_cast<std::size_t>(CEffectV2Object::TEXTURE_INPUT::BASE)] = object.diffuseTextureAssetId;
    staged.Parts.clear();
    params.fMeshPreScale = object.modelPreScale;
    params.fPlayRate = motion.playbackSpeed;
    params.bLoop = false;
    params.bBillboard = false;
    params.Velocity = {};
    params.Position = {};
    params.Rotation = {};
    params.Scale = {{1.f,1.f,1.f},{1.f,1.f,1.f},false};
    staged.Desc.bParamsAuthored = true;
    stagedChild.iStartMs = motion.startDelayMs;
    stagedChild.eStop = EFFECT_V2_CHILD_STOP::DEACTIVATE;
    status = "Applied saved model + Motion to this Effect draft only. ";
    WORLD_SEQUENCE_TRANSFORM_KEY fixed;
    bool constant = true, hasKey = false;
    for (const auto& track : motion.tracks)
        for (const auto& key : track.keys)
        {
            if (!hasKey) { fixed = key; hasKey = true; }
            if (!Same3(fixed.positionOffset,key.positionOffset) || !Same3(fixed.scaleMultiplier,key.scaleMultiplier) ||
                !Same3({fixed.rotationQuaternion.x,fixed.rotationQuaternion.y,fixed.rotationQuaternion.z},
                    {key.rotationQuaternion.x,key.rotationQuaternion.y,key.rotationQuaternion.z}) ||
                std::abs(fixed.rotationQuaternion.w-key.rotationQuaternion.w) > .000001f || fixed.visible != key.visible)
                constant = false;
        }
    if (!constant)
    { fixed = {}; Note(status, "Animated transform/visibility keys are not imported; physics is applied without those keys."); }
    if (!fixed.visible) { fixed = {}; Note(status, "Hidden source pose is not imported."); }
    const float qLength = fixed.rotationQuaternion.x*fixed.rotationQuaternion.x + fixed.rotationQuaternion.y*fixed.rotationQuaternion.y +
        fixed.rotationQuaternion.z*fixed.rotationQuaternion.z + fixed.rotationQuaternion.w*fixed.rotationQuaternion.w;
    if (!std::isfinite(qLength) || qLength < .000001f || fixed.scaleMultiplier.x <= 0.f || fixed.scaleMultiplier.y <= 0.f || fixed.scaleMultiplier.z <= 0.f)
    { status = "Invalid fixed transform; Effect draft preserved."; return false; }
    const auto rotation = Euler(fixed.rotationQuaternion);
    const float3_t scale = {object.scale.x*fixed.scaleMultiplier.x,object.scale.y*fixed.scaleMultiplier.y,object.scale.z*fixed.scaleMultiplier.z};
    params.Position = {fixed.positionOffset,fixed.positionOffset,false};
    const bool native = object.animated && !motion.animationTracks.empty();
    if (!native)
    {
        staged.eType = EFFECT_V2_TYPE::PARTICLE;
        staged.Desc.eShape = CEffectV2Object::SHAPE::PARTICLE;
        staged.strAnimationClip.clear();
        auto& particle = params.Particle;
        particle.eSpawnShape = CEffectV2Object::PARTICLE_SPAWN_SHAPE::POINT;
        particle.eVelocityMode = CEffectV2Object::PARTICLE_VELOCITY_MODE::FIXED;
        particle.vVelocityMin = particle.vVelocityMax = physics.velocity;
        particle.vAcceleration = physics.acceleration;
        particle.fDrag = 0.f;
        particle.vLifetime = {life,life};
        particle.iRandomSeed = physics.seed;
        particle.bLocalSpace = false;
        particle.vSizeStart = particle.vSizeEnd = {scale.x,scale.x};
        particle.vMeshRotationMin = particle.vMeshRotationMax = rotation;
        particle.vMeshSpinMin = particle.vMeshSpinMax = physics.angularVelocityDegrees;
        particle.iMaxParticles = (std::max)(particle.iMaxParticles, physics.count);
        particle.iBurstCount = physics.intervalMs && physics.count > 1u ? 1u : physics.count;
        particle.fSpawnRate = physics.intervalMs && physics.count > 1u ? 1000.f/float(physics.intervalMs) : 0.f;
        const double lastBirthMs = double(physics.count-1u)*physics.intervalMs;
        // Include the final birth, then stop before another interval can fire.
        const double emissionMs = particle.fSpawnRate > 0.f ? lastBirthMs + (std::min)(1.0,double(physics.intervalMs)*.5) : 1.0;
        params.fLifetime = static_cast<float>(emissionMs*.001);
        stagedChild.iDurationMs = static_cast<std::uint32_t>((std::max)(1.0,std::ceil(emissionMs/motion.playbackSpeed)));
        Note(status, "Particle lifetime=" + std::to_string(motion.durationMs) + "ms, count=" + std::to_string(physics.count) +
            ", interval=" + std::to_string(physics.intervalMs) + "ms, rate=" + std::to_string(motion.playbackSpeed) +
            ". Finite emission drains each particle; later births do not share the World sequence end.");
        Note(status, object.animated ? "No saved native clip in this Motion; imported mesh particles." : "Static model has no native animation clip; saved velocity/acceleration drive the motion.");
        if (!Same3(scale,{scale.x,scale.x,scale.x})) Note(status, "Mesh particles support uniform size: X scale applied; nonuniform Y/Z scale is unsupported.");
        if (physics.spreadDegrees != 0.f) Note(status, "World yaw/pitch velocity spread is not equivalent to Effect cone spread and was not imported; tune velocity/rotation ranges explicitly.");
        Note(status, "Saved spin becomes equal mesh-spin Min/Max; widen mesh rotation/spin ranges to randomize copies.");
    }
    else
    {
        staged.eType = EFFECT_V2_TYPE::MESH;
        staged.Desc.eShape = CEffectV2Object::SHAPE::MESH;
        const auto& clip = motion.animationTracks.front();
        staged.strAnimationClip = clip.clipName;
        params.iAnimationIndex = 0u;
        params.bAnimationLoop = clip.loop;
        params.fPlayRate *= clip.playbackRate;
        params.fLifetime = life*clip.playbackRate;
        params.Scale = {scale,scale,false};
        params.Rotation = {rotation,rotation,false};
        const float3_t velocity = {physics.velocity.x/clip.playbackRate,physics.velocity.y/clip.playbackRate,physics.velocity.z/clip.playbackRate};
        params.Velocity = {velocity,velocity,false};
        if (!Zero(physics.acceleration)) Note(status, "Native mesh acceleration is not imported; only constant velocity is applied. Use a Particle element for ballistic motion.");
        if (!Zero(physics.angularVelocityDegrees)) Note(status, "Native mesh angular physics is not imported; use a Particle element for independent spin.");
        stagedChild.iDurationMs = static_cast<std::uint32_t>((std::max)(1.0,std::ceil(double(motion.durationMs)/motion.playbackSpeed)));
        if (motion.animationTracks.size() > 1u || clip.startMs != 0u)
            Note(status, "Imported the first native clip only; ordered clip switching/start offsets remain in World Object Motion.");
        if (physics.count != 1u || physics.intervalMs != 0u || physics.spreadDegrees != 0.f)
            Note(status, "Native Mesh imports one actor; emission count/interval/spread require a Particle element.");
        Note(status, "Native clip=" + staged.strAnimationClip + ". Physical clip availability is checked when Preview is prepared.");
    }
    if (!Zero(physics.revolutionDegreesPerSecond) || !Zero(physics.revolutionOffset)) Note(status, "Revolution/orbit is unsupported and was not imported.");
    if (motion.motionEnd != WORLD_SEQUENCE_MOTION_END::STOP)
        Note(status, "World Motion HOLD/LOOP/NEXT completion is not imported; this Effect plays one finite occurrence.");
    if (object.diffuseTextureAssetId.empty()) Note(status, "Source has no explicit base texture; bind an Effect Base slot before saving if required by this renderer.");
    Note(status, "World placement position/anchor are not copied; use the Effect occurrence anchor and offset.");
    effect = std::move(staged); child = std::move(stagedChild);
    return true;
}
